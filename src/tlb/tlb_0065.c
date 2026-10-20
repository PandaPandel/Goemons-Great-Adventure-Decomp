#include "tlb.h"

/* aqua: the underwater shimmer, a raster effect like main's "Bspace" (beffect.c): bands of the
 * framebuffer drawn back over it, every other one squeezed. */

/* The raster's set-up list. */
Gfx aquaRasterDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_DISABLE),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetTextureFilter(G_TF_POINT),
    gsDPSetCombineLERP(0, 0, 0, TEXEL0, 0, 0, 0, 1, 0, 0, 0, TEXEL0, 0, 0, 0, 1),
    gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsSPEndDisplayList(),
};

extern u16 gFramebuffers[3][SCREEN_WIDTH * SCREEN_HEIGHT]; /* the framebuffers */

/* aqua's work, at its task + 0x20: an enemy's head (EneObj) up to 0x34. */
typedef struct AquaWork {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ Frame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ DLFrame *frame;
    /* 0x38 */ s32 time;
} AquaWork;

Gfx *aquaDrawRaster(Gfx *gfx, Obj *obj, AquaWork *w);

s32 aquaProc(Obj *obj, s16 msg, void *arg) {
    AquaWork *w = (AquaWork *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "aqua";
            eneSetup(obj, arg);
            w->frame = (DLFrame *) frmCreateNext(gFrmEffectLayer, gFrmKindDlist);
            if (w->frame == NULL) {
                PRINTF("");
                return -1;
            }
            PRINTF("");
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gGfxDlHead = aquaDrawRaster(gGfxDlHead, obj, w);
            gSPEndDisplayList(gGfxDlHead++);
            w->time++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Draws rows y to y + h of the framebuffer being drawn back over it, stretched by skew rows at
 * each end (squeezed if skew is negative), in ten strips 32 wide (beffect.c's beDrawRasterBand). */
Gfx *aquaDrawBand(Gfx *gfx, s32 y, s32 h, s32 skew) {
    u16 *img = gFramebuffers[gfxGetFbIndex()];
    s32 top = y;
    s32 bottom;
    s32 y0;
    s32 y1;
    s32 dy;
    s32 i;
    s32 left;
    s32 right;

    if (y < gGameState.scissor.uly) {
        top = gGameState.scissor.uly;
    } else if (y >= gGameState.scissor.lry) {
        return gfx;
    }
    bottom = y + h;
    if (y + h < gGameState.scissor.uly) {
        return gfx;
    }
    y0 = top - skew;
    if (bottom >= gGameState.scissor.lry) {
        bottom = gGameState.scissor.lry - 1;
    }
    y1 = bottom + skew;
    if (y1 - y0 <= 0) {
        return gfx;
    }
    if (bottom - top <= 0) {
        bottom = top + 1;
    }
    dy = bottom - top;
    for (i = 0; i < 10; i++) {
        left = i * 32;
        right = (i + 1) * 32;
        gDPPipeSync(gfx++);
        gDPLoadTextureTile(gfx++, img, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, SCREEN_HEIGHT, left,
                           top, right - 1, bottom - 1, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK,
                           G_TX_NOLOD, G_TX_NOLOD);
        gSPTextureRectangle(gfx++, left << 2, y0 << 2, right << 2, y1 << 2, G_TX_RENDERTILE, left << 5, top << 5,
                            1 << 10, (dy << 10) / (y1 - y0));
    }
    return gfx;
}

/* The shimmer: the screen in bands 40 rows high, scrolling down a row a frame, every other band
 * squeezed by 3 rows at each end and the rest stretched by 3. */
Gfx *aquaDrawRaster(Gfx *gfx, Obj *obj, AquaWork *w) {
    s32 pad;
    s32 base;
    s32 odd;
    s32 i;

    gSPDisplayList(gfx++, fadeSegToRam(aquaRasterDl, obj->file));
    base = w->time % 40 - 40;
    odd = (w->time / 40) & 1;
    for (i = 0; i < 4; i++) {
        gfx = aquaDrawBand(gfx, base + i * 80 + odd * 40, 40, -3);
    }
    for (i = 0; i < 4; i++) {
        gfx = aquaDrawBand(gfx, base + i * 80 + (odd ^ 1) * 40, 40, 3);
    }
    return gfx;
}
