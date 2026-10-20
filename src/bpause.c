#include "common.h"
#include "object.h"
#include "game.h"
#include "audio.h"
#include "bpause.h"
#include "contpak.h"
#include "fade.h"
#include "fileload.h"
#include "gfxmgr.h"
#include "joypad.h"
#include "scenario.h"
#include "tmanager.h"

/* "pause" (overlay 10): the boss battle's pause. The screen dims; five icons slide in from the
 * right and the move list (技表) shows, sixteen button icons in four rows; A, B or Start on a
 * controller in play slides them out and the battle goes on. The frame count is held still and
 * the controllers' rumble is stopped meanwhile. */

/* One sliding icon. */
typedef struct PauseKey {
    /* 0x0 */ PolFrame *frame;
    /* 0x4 */ f32 step; /* x per frame while count runs */
    /* 0x8 */ s16 count;
} PauseKey; /* size 0xC */

typedef struct BPauseWork {
    /* 0x00 */ s32 (*proc)(Obj *obj, struct BPauseWork *w);
    /* 0x04 */ CamView *view; /* an orthographic one */
    /* 0x08 */ DLFrame *dl; /* the dimming and the move list */
    /* 0x0C */ u16 timer; /* frames in this proc */
    /* 0x10 */ PolFrame *title;
    /* 0x14 */ u8 list; /* 1 while the move list shows */
    /* 0x18 */ PauseKey keys[5];
} BPauseWork;

#define PAUSE(task) ((BPauseWork *) ((u32) (task) + sizeof(Obj)))

extern Gfx *gGfxDlHead; /* the display-list buffer */
extern Frame *gFramePool10;
extern s16 gFrmKindPol; /* the "pol" kind */
extern s16 gFrmKindCamera; /* a view's */
extern s16 gFrmKindDlist; /* a display list's */
extern Gfx gDrawSetupLitTexEdgeDl[];
extern Gfx bpauseIconDl[]; /* an icon list, in each icon file set (segment 3) */

/* the 2D setup */
Gfx gBpause2dSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                          G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_DECALRGBA),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetTextureFilter(G_TF_POINT),
    gsSPEndDisplayList(),
};

/* the title setup (w->title->dl) */
Gfx gBpauseTitleDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                          G_TEXTURE_GEN_LINEAR | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_CULL_BACK),
    gsDPSetRenderMode(G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2),
    gsDPSetCombineLERP(PRIMITIVE, 0, 0, 0, 0, 0, 0, PRIMITIVE, PRIMITIVE, 0, 0, 0, 0, 0, 0, PRIMITIVE),
    gsDPPipeSync(),
    gsDPSetBlendColor(0, 0, 0, 1),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsSPEndDisplayList(),
};

s32 gBpauseIconSetTable[] = { 0x110, 0x112, 0x13E, 0x13F, 0x140 }; /* the icons file sets */
Gfx *gBpauseIconDlTable[] = { bpauseIconDl, bpauseIconDl, bpauseIconDl, bpauseIconDl, bpauseIconDl }; /* and their lists */
extern u8 bpauseTitleDl[];
extern u8 bpauseButtonTex0[]; /* the button icons, in file set 0xF2 */
extern u8 bpauseButtonTex1[];
extern u8 bpauseButtonTex2[];
extern u8 bpauseButtonTex3[];
extern u8 bpauseButtonTex4[];
extern u8 bpauseButtonI4Tex[];


s32 bpauseOpen(Obj *obj, BPauseWork *w);
s32 bpauseWait(Obj *obj, BPauseWork *w);
s32 bpauseClose(Obj *obj, BPauseWork *w);

/* A 16x16 RGBA16 icon (tex in file set n), drawn 24x24 at (x, y). Returns the list's end, its
 * gSPEndDisplayList left to be overwritten. */
Gfx *bpauseDrawIconRgba(Gfx *gfx, u8 *tex, s32 n, s32 x, s32 y) {
    gDPPipeSync(gfx++);
    gDPLoadTextureBlock(gfx++, floadSegToRam((u32) tex, n, 0), G_IM_FMT_RGBA, G_IM_SIZ_16b, 16, 16, 0,
                        G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK,
                        G_TX_NOLOD, G_TX_NOLOD);
    gSPTextureRectangle(gfx++, x << 2, y << 2, (x + 24) << 2, (y + 24) << 2, G_TX_RENDERTILE, 0, 0, 1 << 10,
                        1 << 10);
    gSPEndDisplayList(gfx);
    return gfx;
}

/* The same for a 16x16 I4 icon. */
Gfx *bpauseDrawIconI4(Gfx *gfx, u8 *tex, s32 n, s32 x, s32 y) {
    gDPPipeSync(gfx++);
    gDPLoadTextureBlock_4b(gfx++, floadSegToRam((u32) tex, n, 0), G_IM_FMT_I, 16, 16, 0,
                           G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                           G_TX_NOLOD, G_TX_NOLOD);
    gSPTextureRectangle(gfx++, x << 2, y << 2, (x + 24) << 2, (y + 24) << 2, G_TX_RENDERTILE, 0, 0, 1 << 10,
                        1 << 10);
    gSPEndDisplayList(gfx);
    return gfx;
}

/* The pause's handler. */
s32 bpauseProc(Obj *obj, s16 msg, void *arg) {
    BPauseWork *w = PAUSE(obj);
    s32 pad;
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "pause";
            tskDestroy(fadeCreate(gGameWork.gameProcTask));
            fadeCreate(gGameWork.gameProcTask);
            w->view = (CamView *) frmCreate(gFramePool10, gFrmKindCamera);
            if (w->view == NULL) {
                return -1;
            }
            w->view->ortho |= 1;
            w->view->zNear = -200.0f;
            w->view->zFar = 200.0f;
            w->dl = (DLFrame *) frmCreate(&w->view->f, gFrmKindDlist);
            if (w->dl == NULL) {
                return -1;
            }
            for (i = 0; i < 5; i++) {
                w->keys[i].frame = (PolFrame *) frmCreate(&w->view->f, gFrmKindPol);
                if (w->keys[i].frame == NULL) {
                    return -1;
                }
            }
            w->title = (PolFrame *) frmCreate(&w->view->f, gFrmKindPol);
            if (w->title == NULL) {
                return -1;
            }
            w->title->dl = gBpauseTitleDl;
            w->title->dl2 = (Gfx *) bpauseTitleDl;
            floadGetSetRam((void **) w->title->segs, 0xF2);
            w->title->rot[0] = 3.1415927f;
            w->title->pos[0] = 160.0f;
            w->title->pos[1] = 150.0f;
            w->title->f.flags &= ~2;
            sndPlay(3);
            for (i = 0; i < 4; i++) {
                contPakInitRumble(i);
                contPakRumbleRequestStop(i);
            }
            contPakRumbleSuspend();
            w->proc = bpauseOpen;
            break;
        case 1:
            contPakRumbleResume();
            sndPlay(4);
            for (i = 0; i < 5; i++) {
                frmDestroy(&w->keys[i].frame->f);
            }
            frmDestroy(&w->dl->f);
            frmDestroy(&w->title->f);
            frmDestroy(&w->view->f);
            fadeSetLevel(0.0f);
            break;
        case 0:
            w->timer++;
            for (i = 0; i < 4; i++) {
                contPakRumbleRequestStop(i);
            }
            if (w->proc != NULL && w->proc(obj, w) != 0) {
                return -1;
            }
            for (i = 0; i < 5; i++) {
                if (w->keys[i].frame != NULL && w->keys[i].count != 0) {
                    w->keys[i].frame->pos[0] += w->keys[i].step;
                    w->keys[i].count--;
                }
            }
            gGameState.frameCount--;
            break;
        case 3:
            if (w->dl != NULL) {
                w->dl->dl = gGfxDlHead;
                if (w->list != 0) {
                    gSPDisplayList(gGfxDlHead++, gBpause2dSetupDl);
                    gGfxDlHead = bpauseDrawIconRgba(gGfxDlHead, bpauseButtonTex0, 0xF2, 170, 90);
                    gGfxDlHead = bpauseDrawIconI4(gGfxDlHead, bpauseButtonI4Tex, 0xF2, 190, 90);
                    gGfxDlHead = bpauseDrawIconRgba(gGfxDlHead, bpauseButtonTex1, 0xF2, 210, 90);
                    gGfxDlHead = bpauseDrawIconRgba(gGfxDlHead, bpauseButtonTex3, 0xF2, 170, 115);
                    gGfxDlHead = bpauseDrawIconRgba(gGfxDlHead, bpauseButtonTex3, 0xF2, 190, 115);
                    gGfxDlHead = bpauseDrawIconRgba(gGfxDlHead, bpauseButtonTex0, 0xF2, 210, 115);
                    gGfxDlHead = bpauseDrawIconRgba(gGfxDlHead, bpauseButtonTex1, 0xF2, 170, 145);
                    gGfxDlHead = bpauseDrawIconRgba(gGfxDlHead, bpauseButtonTex1, 0xF2, 190, 145);
                    gGfxDlHead = bpauseDrawIconRgba(gGfxDlHead, bpauseButtonTex1, 0xF2, 210, 145);
                    gGfxDlHead = bpauseDrawIconRgba(gGfxDlHead, bpauseButtonTex0, 0xF2, 230, 145);
                    gGfxDlHead = bpauseDrawIconRgba(gGfxDlHead, bpauseButtonTex4, 0xF2, 170, 172);
                    gGfxDlHead = bpauseDrawIconRgba(gGfxDlHead, bpauseButtonTex3, 0xF2, 190, 172);
                    gGfxDlHead = bpauseDrawIconRgba(gGfxDlHead, bpauseButtonTex4, 0xF2, 210, 172);
                    gGfxDlHead = bpauseDrawIconRgba(gGfxDlHead, bpauseButtonTex0, 0xF2, 230, 172);
                    gGfxDlHead = bpauseDrawIconRgba(gGfxDlHead, bpauseButtonTex3, 0xF2, 170, 200);
                    gGfxDlHead = bpauseDrawIconRgba(gGfxDlHead, bpauseButtonTex3, 0xF2, 190, 200);
                    gGfxDlHead = bpauseDrawIconRgba(gGfxDlHead, bpauseButtonTex2, 0xF2, 210, 200);
                }
                gSPDisplayList(gGfxDlHead++, gBpause2dSetupDl);
                gDPPipeSync(gGfxDlHead++);
                gDPSetCombine(gGfxDlHead++, 0xFFFFFF, 0xFFFEFB7D);
                gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 0);
                gDPFillRectangle(gGfxDlHead++, 16, 8, 304, 232);
                gSPEndDisplayList(gGfxDlHead++);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Opening: the dimming fades in; at 16 the five icons are set to the right and start sliding
 * in; once the last has stopped, the move list shows and the scenario 0x15 runs. */
s32 bpauseOpen(Obj *obj, BPauseWork *w) {
    s32 i;

    if (w->timer == 1) {
        fadeSetAll(16.0f, 0.0f, 192.0f, 0, 0, 0);
    } else if (w->timer == 16) {
        for (i = 0; i < 5; i++) {
            w->keys[i].frame->dl = gDrawSetupLitTexEdgeDl;
            w->keys[i].frame->dl2 = gBpauseIconDlTable[i];
            w->keys[i].frame->pos[0] = 352 + i * 64;
            w->keys[i].frame->pos[1] = 40.0f;
            w->keys[i].frame->rot[0] = 3.1415927f;
            w->keys[i].frame->scale[2] = 0.5f;
            w->keys[i].frame->scale[0] = w->keys[i].frame->scale[1] = w->keys[i].frame->scale[2];
            floadGetSetRam((void **) w->keys[i].frame->segs, gBpauseIconSetTable[i]);
            w->keys[i].step = -16.0f;
            w->keys[i].count = (w->keys[i].frame->pos[0] - (96 + i * 32)) / fabsf(w->keys[i].step);
        }
    }
    if (w->timer >= 17 && w->keys[4].count == 0) {
        w->list = 1;
        w->title->f.flags |= 2;
        PRINTF("技表文字を表示します\n");
        (*scRunStart)(0x15, 0, 0);
        w->timer = 0;
        w->proc = bpauseWait;
    }
    return 0;
}

/* Waiting: A, B or Start on a controller in play closes it. */
s32 bpauseWait(Obj *obj, BPauseWork *w) {
    s32 i;

    for (i = 0; i < 4; i++) {
        if (gSaveData.players[i].chara >= 0 &&
            joyGetHeldTime(i, A_BUTTON | B_BUTTON | START_BUTTON) == 1) {
            w->timer = 0;
            w->proc = bpauseClose;
            return 0;
        }
    }
    return 0;
}

/* Closing: the move list goes and the scenario ends; the icons slide out in turn (frames 1, 6,
 * 10, 14, 18); at 30 they and the dimming are freed and the fade goes; at 54 the pause ends. */
s32 bpauseClose(Obj *obj, BPauseWork *w) {
    s32 i;

    if (w->timer == 1) {
        w->title = (PolFrame *) frmDestroy(&w->title->f);
        scRunEnd();
        w->list = 0;
        PRINTF("技表文字を消します\n");
        w->keys[0].count = 30;
        w->keys[0].step = -16.0f;
    } else if (w->timer == 6) {
        w->keys[1].count = 30;
        w->keys[1].step = -16.0f;
    } else if (w->timer == 10) {
        w->keys[2].count = 30;
        w->keys[2].step = -16.0f;
    } else if (w->timer == 14) {
        w->keys[3].count = 30;
        w->keys[3].step = -16.0f;
    } else if (w->timer == 18) {
        w->keys[4].count = 30;
        w->keys[4].step = -16.0f;
    } else if (w->timer == 30) {
        i = 0;
        do {
            w->keys[i].frame = (PolFrame *) frmDestroy(&w->keys[i].frame->f);
        } while (++i != 3);
        w->dl = (DLFrame *) frmDestroy(&w->dl->f);
        fadeSetAll(16.0f, 160.0f, 0.0f, 0, 0, 0);
    } else if (w->timer == 54) {
        return 1;
    }
    return 0;
}
