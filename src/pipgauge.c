#include "common.h"
#include "object.h"
#include "file.h"
#include "fileload.h"
#include "gfxmgr.h"
#include "pipgauge.h"
#include "tmanager.h"

/* The "gauge" task (overlay 9): a panel of up to 49 pips (two rows of 25) at (x, y), counting
 * one pip a frame towards its target; drawn only while on. */

typedef struct PipGauge {
    /* 0x00 */ DLFrame *frame;
    /* 0x04 */ u32 tex; /* file 0x23's first file, as a K0 address: segment 3 */
    /* 0x08 */ s32 target;
    /* 0x0C */ s32 shown;
    /* 0x10 */ s32 x;
    /* 0x14 */ s32 y;
    /* 0x18 */ u8 on;
} PipGauge;

#define GAUGE(obj) ((PipGauge *) ((u32) (obj) + sizeof(Obj)))

extern Gfx *gGfxDlHead; /* the display-list buffer */
extern Frame *gFramePool10;
extern s16 gFrmKindDlist; /* the "dlist" frame kind */
/* pipgauge.c's .data (0x801B4520-0x801B45C0): the gauge's display list, a 32x32 RGBA16
 * texture from segment 3, decal, translucent. */
Gfx gPipGaugeDl[] = {
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
    gsDPPipeSync(),
    gsDPLoadTextureBlock(0x03000000, G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 32, 0,
                         G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK,
                         G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD),
    gsSPEndDisplayList(),
};



/* The gauge's handler. */
s32 pipGauge_proc(Obj *obj, s16 msg, void *arg) {
    PipGauge *w = GAUGE(obj);
    s32 i;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "gauge";
            w->frame = (DLFrame *) frmCreatePrev(gFramePool10, gFrmKindDlist);
            if (w->frame == NULL) {
                return -1;
            }
            floadLoadSet(0x23);
            w->tex = fileRamAddr(floadGetSetFile(0x23, 0)) + 0x80000000;
            w->on = 0;
            w->target = 0;
            w->shown = 0;
            w->x = 73;
            w->y = 23;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 0:
            if (w->on) {
                w->frame->f.flags |= 2;
                if (w->target < w->shown) {
                    w->shown--;
                }
                if (w->shown < w->target) {
                    w->shown++;
                }
            } else {
                w->frame->f.flags &= ~2;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gSPSegment(gGfxDlHead++, 3, w->tex);
            gSPDisplayList(gGfxDlHead++, gPipGaugeDl);
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombine(gGfxDlHead++, 0xFFFFFF, 0xFFFEFB7D);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 255, 64);
            gDPFillRectangle(gGfxDlHead++, w->x - 2, w->y - 1, w->x + 173, w->y + 15);
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombine(gGfxDlHead++, 0xFFFFFF, 0xFFFCF279);
            gSPTextureRectangle(gGfxDlHead++, w->x << 2, (w->y + 1) << 2, (w->x + 20) << 2, (w->y + 13) << 2, 0, 0, 0,
                                0x400, 0x400);
            for (i = 0; i < w->shown; i++) {
                gGfxDlHead = pipDraw(gGfxDlHead, w->x, w->y, i);
            }
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Pip i (to 48; from 25 on the second row) of a gauge at (x, y). */
Gfx *pipDraw(Gfx *gfx, s32 x, s32 y, s32 i) {
    s32 dy = 0;
    s32 px;
    s32 py;

    if (i >= 50) {
        i = 49;
    }
    if (i >= 25) {
        i -= 25;
        dy = 8;
    }
    px = x; /* starts at the gauge, then steps over i pips: one statement gives the add the other way round */
    px += i * 6;
    py = y + dy;
    gSPTextureRectangle(gfx++, (px + 23) << 2, py << 2, (px + 27) << 2, (py + 6) << 2, 0, 0x2A0, 0, 0x400, 0x400);
    return gfx;
}

/* Shows the gauge. */
void pipGaugeOn(Obj *obj) {
    if (obj != NULL) {
        GAUGE(obj)->on = 1;
    }
}

/* Hides it, empty. */
void pipGaugeOff(Obj *obj) {
    if (obj != NULL) {
        GAUGE(obj)->on = 0;
        GAUGE(obj)->target = 0;
        GAUGE(obj)->shown = 0;
    }
}

/* Puts it at (x, y). */
void pipGaugeSetPos(Obj *obj, s32 x, s32 y) {
    if (obj != NULL) {
        GAUGE(obj)->x = x;
        GAUGE(obj)->y = y;
    }
}

/* Its target count. */
void pipGaugeSetTarget(Obj *obj, s32 n) {
    if (obj != NULL) {
        GAUGE(obj)->target = n;
    }
}
