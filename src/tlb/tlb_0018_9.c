#include "tlb.h"
#include "tlb/tlb_0018.h"

extern u8 ntegataIconTex[]; /* file 0x141's 24 x 24 icon */
extern u8 ntegataDigitsTex[]; /* file 0x10E's 8 x 176 digits */

/* Draws a gauge's icon (24 x 24, file 0x141) with its top left at (x - 32, y - 12), then n in
 * two digits of 8 x 16 (file 0x10E) at (x + 8, y - 8). The icon's size comes through w and h
 * (the game does that load block's count & 0xFFF and the dxt divide at run time). The
 * rectangle's corners are shifted (<< 2), as the game computes (left + 24) << 2. */
Gfx *ntegataDrawGauge(Gfx *gfx, s32 x, s32 y, s32 n) {
    s32 left = x - 32;
    s32 top = y - 12;
    s32 w = 24;
    s32 h = 24;

    gSPDisplayList(gfx++, gGaugeSetupDl);
    gDPPipeSync(gfx++);
    gDPLoadTextureBlock(gfx++, floadSegToRam((u32) ntegataIconTex, 0x141, 0), G_IM_FMT_RGBA, G_IM_SIZ_16b, w, h, 0,
                        G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    gSPTextureRectangle(gfx++, left << 2, top << 2, (left + w) << 2, (top + h) << 2, G_TX_RENDERTILE, 0, 0,
                        1 << 10, 1 << 10);
    gSPDisplayList(gfx++, gGaugeModulatePrimDl);
    gDPPipeSync(gfx++);
    gDPLoadTextureBlock(gfx++, floadSegToRam((u32) ntegataDigitsTex, 0x10E, 0), G_IM_FMT_IA, G_IM_SIZ_8b, 8, 176, 0,
                        G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    return gaugeDrawNumber(gfx, x + 8, y - 8, 8, 16, 2, n, 2);
}

/* The "ntegata" task (the entry passes, "Number of entry passes"; its proc ntegataProc) runs
 * one step function a frame from its work. */
typedef struct NtegataWork NtegataWork;
typedef s32 (*NtegataFunc)(Obj *obj, NtegataWork *w);

struct NtegataWork {
    /* 0x00 */ NtegataFunc step;
    /* 0x04 */ CamView *view; /* orthographic, with its lights */
    /* 0x08 */ PolFrame *board; /* the board behind (file 0xF2) */
    /* 0x0C */ TextFrame *title;
    /* 0x10 */ TextFrame *left; /* the "left:" label */
    /* 0x14 */ DLFrame *list;
    /* 0x18 */ u16 timer;
    /* 0x1A */ u8 pad1A[0x2];
    /* 0x1C */ s32 count; /* the passes */
    /* 0x20 */ s32 counted; /* counted up so far, four steps a pass */
    /* 0x24 */ u8 shown;
};

s32 ntegataStepWait(Obj *obj, NtegataWork *w);
extern Gfx ntegataBoardDl[]; /* the board */

/* "ntegata", the entry passes: loads sets 0xF2, 0x10E and 0x141, fades in from black, and puts up
 * its board at (160, 120) and "Number of entry passes" at (160, 76); its pass count is the six
 * tegata summed. Each frame it counts its timer and runs its step (ntegataStepWait first),
 * ending when a step returns nonzero. Drawn: the gauge with the passes counted (a quarter of the
 * steps) and, once shown, the 44 less the count.
 * FAKE: message 1's first frmFontDestroy is called as returning s32 (IDO_NOTES 369; found by
 * match_function.py results): called as void, its address took v0, where the game has v1. */
s32 ntegataProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    NtegataWork *w = (NtegataWork *) FX(obj);
    LightFrame *light;
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "ntegata";
            gRetracesPerFrame = 1;
            floadLoadSet(0xF2);
            floadLoadSet(0x10E);
            floadLoadSet(0x141);
            frmFontNewKind();
            fadeCreate(gGameWork.gameProcTask);
            fadeSetParent(gFrmTextRoot);
            fadeSetColor(0, 0, 0);
            fadeSetLevel(255.0f);
            fadeSetSpeed(32.0f);
            fadeSetTarget(0.0f);
            if ((w->view = (CamView *) frmCreate(gFrmLayers[0], gFrmKindCamera)) == NULL) {
                return -1;
            }
            w->view->ortho |= 1;
            w->view->zNear = -200.0f;
            w->view->zFar = 200.0f;
            frmCreate(&w->view->f, gFrmKindAmbi);
            frmCreate(&w->view->f, gFrmKindLight);
            light = (LightFrame *) frmCreate(&w->view->f, gFrmKindLight);
            light->color[0] = 0;
            light->color[1] = 0;
            light->color[2] = 0;
            if ((w->board = (PolFrame *) frmCreate(&w->view->f, gFrmKindPol)) == NULL) {
                return -1;
            }
            w->board->dl = gDrawSetupLitTexEdgeDl;
            w->board->dl2 = ntegataBoardDl;
            floadGetSetRam((void **) w->board->segs, 0xF2);
            w->board->rot[0] = 3.1415927f;
            w->board->pos[0] = 160.0f;
            w->board->pos[1] = 120.0f;
            w->board->scale[2] = 0.8f;
            w->board->scale[0] = w->board->scale[1] = w->board->scale[2];
            if ((w->list = (DLFrame *) frmCreate(&w->view->f, gFrmKindDlist)) == NULL) {
                return -1;
            }
            w->title = frmFontCreate((Frame *) gFramePool10, 0, 0, NULL);
            diaryTextCenter(w->title, 160.0f, 76.0f, 8, 12, "Number of entry passes");
            w->count = 0;
            /* on one line, for the game's place of the unrolled loop's 2 (IDO_NOTES 12) */
            for (i = 0; i < 6; i++) { w->count += gSaveData.tegata[i]; }
            w->step = ntegataStepWait;
            break;
        case 1:
            ((s32 (*)(TextFrame *f)) frmFontDestroy)(w->title);
            frmFontDestroy(w->left);
            frmDestroy(&w->view->f);
            frmFontDeleteKind();
            break;
        case 0:
            w->timer++;
            if (w->step != NULL && w->step(obj, w)) {
                return -1;
            }
            break;
        case 3:
            if (w->list != NULL) {
                w->list->dl = gGfxDlHead;
                gGfxDlHead = ntegataDrawGauge(gGfxDlHead, 160, 104, w->counted / 4);
                if (w->shown) {
                    gGfxDlHead = gaugeDrawNumber(gGfxDlHead, 172, 142, 8, 16, 2, 44 - w->count, 10);
                }
                gSPEndDisplayList(gGfxDlHead++);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 ntegataStepCount(Obj *obj, NtegataWork *w);

/* After 33 frames, the "left:" step. */
s32 ntegataStepWait(Obj *obj, NtegataWork *w) {
    if (w->timer >= 33) {
        w->timer = 0;
        w->step = ntegataStepCount;
    }
    return 0;
}

s32 ntegataStepDone(Obj *obj, NtegataWork *w);

/* Counts up four steps a pass; when all are counted, puts up "left:" at 124, 144 and goes on to
 * ntegataStepDone. */
s32 ntegataStepCount(Obj *obj, NtegataWork *w) {
    if (w->count * 4 == w->counted) {
        w->left = frmFontCreate((Frame *) gFramePool10, 124, 144, "left:");
        w->shown = 1;
        w->timer = 0;
        w->step = ntegataStepDone;
        return 0;
    }
    w->counted++;
    return 0;
}

s32 ntegataStepDone(Obj *obj, NtegataWork *w) {
    return 0;
}
