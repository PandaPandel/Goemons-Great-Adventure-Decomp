#include "tlb.h"

/* File 71: the bloodw (a blood-water surface), the nami (a wave) and the nami master. */

/* The bloodw's work: an enemy's up to 0x34. */
typedef struct BloodwWork {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ Frame *ride; /* destroyed with it; never made here */
    /* 0x3C */ s32 scroll;
    /* 0x40 */ s32 speed; /* added to scroll each draw */
    /* 0x44 */ Gfx *(*draw)(Gfx *gfx, u16 t); /* its texture's display list */
} BloodwWork;

/* A wave's work, at its task + 0x20 (made by the nami master, not a placed enemy). */
typedef struct NamiWork {
    /* 0x00 */ f32 period; /* the fades' and the height change's length, in steps */
    /* 0x04 */ PolFrame *pol;
    /* 0x08 */ RideFrame *ride;
    /* 0x0C */ u16 scroll[2]; /* the texture's (t, s) */
    /* 0x10 */ s32 speed; /* added to scroll[1] each step */
    /* 0x14 */ s32 timer; /* its life; it fades out over the last period steps */
    /* 0x18 */ f32 vel[3];
    /* 0x24 */ f32 height; /* the model's y scale */
    /* 0x28 */ f32 target;
    /* 0x2C */ f32 step;
    /* 0x30 */ s32 alpha;
    /* 0x34 */ s32 alphaStep;
} NamiWork;

/* The nami master's work: an enemy's up to 0x34. */
typedef struct NamiMasterWork {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ s32 count; /* waves in a row at the start */
    /* 0x38 */ u16 timer;
    /* 0x3C */ s32 phase; /* the next height, 0-3 */
} NamiMasterWork;

/* Message 6's argument (plmove.c): a place on the course, moved by the object ridden. */
typedef struct PlPlace {
    /* 0x0 */ s16 zone;
    /* 0x4 */ f32 place;
    /* 0x8 */ f32 side;
    /* 0xC */ f32 height;
} PlPlace;

extern u8 bloodwWaterTex[];
extern u8 bloodwWaveTex[];
extern Gfx bloodwData1Dl[];
extern Gfx bloodwData2Dl[];
extern Gfx bloodwData3Dl[];
extern Gfx bloodwData4Dl[];
extern Gfx namiDl[];
extern u8 namiRide[];
f32 namiHeights[4] = { 0.1f, 0.04f, 0.02f, 0.04f }; /* the waves' heights, in turn */

Gfx *bloodwWaveGfx(Gfx *gfx, u16 t);
Obj *namiMake(Obj *o, f32 x, f32 y, f32 z, f32 rotY, f32 dist, f32 height, s32 timer, s32 alpha);

/* The water's display list: one 32x32 CI4 texture (bloodwWaterTex) in two tiles, scrolled by t
 * (tile 0 along both axes, tile 1 the other way), blended through the environment colour. */
Gfx *bloodwWaterGfx(Gfx *gfx, u16 t) {
    gDPPipeSync(gfx++);
    gDPSetCombineLERP(gfx++, TEXEL0, TEXEL1, ENVIRONMENT, TEXEL1, 0, 0, 0, ENVIRONMENT, 0, 0, 0, COMBINED, 0, 0, 0,
                      COMBINED);
    gDPSetEnvColor(gfx++, 128, 128, 128, 128);
    gDPSetTextureImage(gfx++, G_IM_FMT_CI, G_IM_SIZ_16b, 1, bloodwWaterTex);
    gDPSetTile(gfx++, G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD,
               G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD);
    gDPLoadSync(gfx++);
    gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, ((32 * 32 + 3) >> 2) - 1, CALC_DXT_4b(32));
    gDPPipeSync(gfx++);
    gDPSetTile(gfx++, G_IM_FMT_CI, G_IM_SIZ_4b, ((32 >> 1) + 7) >> 3, 0, G_TX_RENDERTILE, 0,
               G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD);
    gDPSetTileSize(gfx++, G_TX_RENDERTILE, t, t, (32 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC);
    gDPSetTextureImage(gfx++, G_IM_FMT_CI, G_IM_SIZ_16b, 1, bloodwWaterTex);
    gDPSetTile(gfx++, G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0x80, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD,
               G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD);
    gDPLoadSync(gfx++);
    gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, ((32 * 32 + 3) >> 2) - 1, CALC_DXT_4b(32));
    gDPPipeSync(gfx++); gDPSetTile(gfx++, G_IM_FMT_CI, G_IM_SIZ_4b, ((32 >> 1) + 7) >> 3, 0x80, 1, 0, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD); gDPSetTileSize(gfx++, 1, t + 10, -t, (32 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC);
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* The same water with no environment colour set, tile 0 scrolled by (s, t), tile 1 by
 * (s + 10, -t). */
Gfx *namiWaterGfx(Gfx *gfx, u16 t, u16 s) {
    gDPPipeSync(gfx++);
    gDPSetCombineLERP(gfx++, TEXEL0, TEXEL1, ENVIRONMENT, TEXEL1, 0, 0, 0, ENVIRONMENT, 0, 0, 0, COMBINED, 0, 0, 0,
                      COMBINED);
    gDPSetTextureImage(gfx++, G_IM_FMT_CI, G_IM_SIZ_16b, 1, bloodwWaterTex);
    gDPSetTile(gfx++, G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD,
               G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD);
    gDPLoadSync(gfx++);
    gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, ((32 * 32 + 3) >> 2) - 1, CALC_DXT_4b(32));
    gDPPipeSync(gfx++);
    gDPSetTile(gfx++, G_IM_FMT_CI, G_IM_SIZ_4b, ((32 >> 1) + 7) >> 3, 0, G_TX_RENDERTILE, 0,
               G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD);
    gDPSetTileSize(gfx++, G_TX_RENDERTILE, s, t, (32 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC);
    gDPSetTextureImage(gfx++, G_IM_FMT_CI, G_IM_SIZ_16b, 1, bloodwWaterTex);
    gDPSetTile(gfx++, G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0x80, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD,
               G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD);
    gDPLoadSync(gfx++);
    gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, ((32 * 32 + 3) >> 2) - 1, CALC_DXT_4b(32));
    gDPPipeSync(gfx++); gDPSetTile(gfx++, G_IM_FMT_CI, G_IM_SIZ_4b, ((32 >> 1) + 7) >> 3, 0x80, 1, 0, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD); gDPSetTileSize(gfx++, 1, s + 10, -t, (32 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC);
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* The wave's display list: one 32x64 I8 texture (bloodwWaveTex) in two tiles, tile 1 scrolling twice
 * as fast as tile 0, coloured between the primitive red and the environment colour.
 * The first load (image, load tile, sync, block) is on one line: IDO schedules by source line,
 * and the game built it as one macro's expansion. As at the end, the second render tile, its
 * size and the pipe sync before them are one line too. */
Gfx *bloodwWaveGfx(Gfx *gfx, u16 t) {
    gDPPipeSync(gfx++);
    gDPSetCombineLERP(gfx++, 0, 0, 0, 1, TEXEL0, TEXEL1, ENVIRONMENT, TEXEL1, PRIMITIVE, ENVIRONMENT, COMBINED_ALPHA,
                      ENVIRONMENT, COMBINED, 0, PRIMITIVE, 0);
    gDPSetPrimColor(gfx++, 0, 0, 255, 128, 128, 255);
    gDPSetEnvColor(gfx++, 128, 0, 0, 128);
    gDPSetTextureImage(gfx++, G_IM_FMT_I, G_IM_SIZ_16b, 1, bloodwWaveTex); gDPSetTile(gfx++, G_IM_FMT_I, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, 6, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD); gDPLoadSync(gfx++); gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, ((32 * 64 + 1) >> 1) - 1, CALC_DXT(32, G_IM_SIZ_8b_BYTES));
    gDPPipeSync(gfx++);
    gDPSetTile(gfx++, G_IM_FMT_I, G_IM_SIZ_8b, ((32 * G_IM_SIZ_8b_LINE_BYTES) + 7) >> 3, 0, G_TX_RENDERTILE, 0,
               G_TX_NOMIRROR | G_TX_WRAP, 6, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD);
    gDPSetTileSize(gfx++, G_TX_RENDERTILE, 0, t, (32 - 1) << G_TEXTURE_IMAGE_FRAC, (64 - 1) << G_TEXTURE_IMAGE_FRAC);
    gDPSetTextureImage(gfx++, G_IM_FMT_I, G_IM_SIZ_16b, 1, bloodwWaveTex);
    gDPSetTile(gfx++, G_IM_FMT_I, G_IM_SIZ_16b, 0, 0x100, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, 6, G_TX_NOLOD,
               G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD);
    gDPLoadSync(gfx++);
    gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, ((32 * 64 + 1) >> 1) - 1, CALC_DXT(32, G_IM_SIZ_8b_BYTES));
    gDPPipeSync(gfx++); gDPSetTile(gfx++, G_IM_FMT_I, G_IM_SIZ_8b, ((32 * G_IM_SIZ_8b_LINE_BYTES) + 7) >> 3, 0x100, 1, 0, G_TX_NOMIRROR | G_TX_WRAP, 6, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD); gDPSetTileSize(gfx++, 1, 10, t * 2, (32 - 1) << G_TEXTURE_IMAGE_FRAC, (64 - 1) << G_TEXTURE_IMAGE_FRAC);
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* The bloodw: a sheet of blood water (one of four models, data's low byte; its tilt in the
 * second byte, its heading in the high half) whose texture scrolls; a player standing at its
 * side gets its damage box's hit kind set to 120 and message 7.
 * Matched 2026-10-04; it was parked for registers: arg's address taken (the FAKE empty
 * "if (&arg) {}" below, as OoT keeps "if (&this->subCamEye) {}") stops IDO from giving it a
 * register, so it reloads from its home as the game does; case 6's plIsInPlay address needed
 * the same reasoning as the twin namiProc's matched case 6. */
s32 bloodwProc(Obj *o, s16 msg, void *arg) {
    BloodwWork *w = (BloodwWork *) FX(o);
    s32 data;
    s32 i;
    Gfx *dl;
    s32 pad[4];
    s32 zone;
    f32 pos[3];
    s32 pad2;

    switch (msg) {
        case 2:
            o->name = "bloodw";
            data = eneSetup(o, arg);
            /* the "& 0xFF" spends the game's extra temp */
            w->rot[0] = (s8) ((data >> 8) & 0xFF) * 3.141592741f / 180;
            w->rot[1] = (data >> 16) * 3.141592741f / 180;
            w->draw = bloodwWaterGfx;
            dl = NULL;
            w->speed = -1;
            switch (data & 0xFF) {
                case 1:
                    dl = bloodwData1Dl;
                    break;
                case 2:
                    dl = bloodwData2Dl;
                    break;
                case 3:
                    dl = bloodwData3Dl;
                    break;
                case 4:
                    dl = bloodwData4Dl;
                    w->draw = bloodwWaveGfx;
                    w->speed = -6;
                    break;
            }
            if (dl != NULL) {
                w->pol = eneNewPol(o, dl);
            }
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayers[6]);
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2] = 0.1f;
            break;
        case 1:
            if (w->ride != NULL) {
                frmDestroy(w->ride);
            }
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            break;
        case 6:
            if (&arg) {} /* FAKE: arg's address taken keeps it in its home, as the game has */
            if (arg != NULL) {
                for (i = 0; i < 4; i++) {
                    if (plIsInPlay(i)) {
                        plAskPos(gGameWork.players[i], &pos[0], &pos[1], &pos[2], &zone);
                        if (fabsf(((PlPlace *) arg)->side - pos[2]) < 1.0f) {
                            plGetDamageBox(gGameWork.players[i])->hitKind = 120;
                            tskSendMsg(gGameWork.players[i], 7, o);
                        }
                    }
                }
            }
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gGfxDlHead = w->draw(gGfxDlHead, w->scroll);
            w->scroll += w->speed;
            eneSetModelOnCourse(o, &w->pol->f);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* A wave's height (scale y) heads for target, reached in period steps. */
void namiSetHeight(Obj *o, f32 target) {
    ((NamiWork *) FX(o))->target = target;
    ((NamiWork *) FX(o))->step = (target - ((NamiWork *) FX(o))->height) / ((NamiWork *) FX(o))->period;
}

/* The nami: one wave (a ride, file 0x6F's model) moving by vel each step, fading in over its
 * period and out over its last period steps while its height drops to 0.001; it is deleted when
 * its timer runs out. A player at its side gets its damage box's hit kind set to 120 and
 * message 7. */
s32 namiProc(Obj *o, s16 msg, void *arg) {
    NamiWork *w = (NamiWork *) FX(o);
    s32 i;
    s32 pad[3];
    s32 zone;
    f32 pos[3];
    f32 period;

    switch (msg) {
        case 2:
            o->name = "nami";
            w->pol = (PolFrame *) frmCreate(gFrmLayers[6], gFrmKindPol);
            if (w->pol == NULL) {
                return -1;
            }
            w->pol->dl2 = namiDl;
            floadGetSetRam((void **) w->pol->segs, 0x6F);
            w->ride = rideCreate(gRideTree2, (s32) o, &w->pol->f, (RideData *) floadSegToRam((u32) namiRide, 0x6F, 0));
            w->alpha = 0;
            period = 32;
            w->period = period;
            w->alphaStep = 128.0f / period;
            break;
        case 1:
            if (w->ride != NULL) {
                frmDestroy(&w->ride->f);
            }
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            break;
        case 6:
            if (arg != NULL) {
                for (i = 0; i < 4; i++) {
                    if (plIsInPlay(i)) {
                        plAskPos(gGameWork.players[i], &pos[0], &pos[1], &pos[2], &zone);
                        if (fabsf(((PlPlace *) arg)->side - pos[2]) < 1.0f) {
                            plGetDamageBox(gGameWork.players[i])->hitKind = 120;
                            tskSendMsg(gGameWork.players[i], 7, o);
                        }
                    }
                }
            }
            break;
        case 0:
            w->timer--;
            if (w->timer == w->period) {
                w->alphaStep = -w->alpha / w->period;
                namiSetHeight(o, 0.001f);
            }
            if (w->timer < 0) {
                return -1;
            }
            w->alpha += w->alphaStep;
            if (w->alpha < 0) {
                w->alpha = 0;
            } else if (w->alpha > 128) {
                w->alpha = 128;
            }
            if (w->step != 0.0f) {
                if (fabsf(w->height - w->target) <= fabsf(w->step)) {
                    w->height = w->target;
                    w->step = 0.0f;
                } else {
                    w->height += w->step;
                }
            }
            w->pol->pos[0] += w->vel[0];
            w->pol->pos[1] += w->vel[1];
            w->pol->pos[2] += w->vel[2];
            w->pol->scale[1] = w->height;
            w->scroll[1] += w->speed;
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gDPSetEnvColor(gGfxDlHead++, 128, 128, 128, w->alpha);
            gGfxDlHead = namiWaterGfx(gGfxDlHead, w->scroll[0], w->scroll[1]);
            w->scroll[0]--;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Makes a wave under o at (x, y, z) heading rotY, moving dist a step that way; it heads for
 * height, lives timer steps and starts at alpha. The work pointer is set just before the
 * mathXZRotateY call, which still takes the work's address afresh: the game reloads t after the
 * call and builds w once for the last five stores. "height = vel[1] = 0" stores vel[1] first,
 * as the game does. */
Obj *namiMake(Obj *o, f32 x, f32 y, f32 z, f32 rotY, f32 dist, f32 height, s32 timer, s32 alpha) {
    Obj *t = tskCreate(o, namiProc, o->file, NULL);
    NamiWork *w;

    if (t != NULL) {
        ((NamiWork *) FX(t))->pol->pos[0] = x;
        ((NamiWork *) FX(t))->pol->pos[1] = y;
        ((NamiWork *) FX(t))->pol->pos[2] = z;
        ((NamiWork *) FX(t))->pol->rot[1] = rotY;
        ((NamiWork *) FX(t))->pol->scale[0] = ((NamiWork *) FX(t))->pol->scale[2] = 0.1002999991f;
        w = (NamiWork *) FX(t);
        mathXZRotateY(rotY, -dist, 0.0f, &((NamiWork *) FX(t))->vel[0], &((NamiWork *) FX(t))->vel[2]);
        w->height = w->vel[1] = 0.0f;
        w->speed = (s32) dist * 4;
        w->timer = timer;
        w->alpha = alpha;
        namiSetHeight(t, height);
    }
    return t;
}

/* The nami master: sends waves (data's low byte of them at once at the start, then one each
 * 160.88 steps), each wave's height the next of four (namiHeights), with its sound. */
s32 namiMasterProc(Obj *o, s16 msg, void *arg) {
    NamiMasterWork *w = (NamiMasterWork *) FX(o);
    s32 data;
    s32 i;
    s32 pad;
    Vec3f d;

    switch (msg) {
        case 2:
            o->name = "nami master";
            data = eneSetup(o, arg);
            w->count = data & 0xFF;
            w->rot[1] = (data >> 16) * 3.141592741f / 180;
            for (i = 0; i < w->count; i++) {
                mathXZRotateY(w->rot[1], i * -160.8811951f, 0.0f, &d.x, &d.z);
                namiMake(o, w->x + d.x, w->y, w->z + d.z, w->rot[1], 1.0f,
                                       namiHeights[w->phase++], (w->count - i) * 160.8811951f, 128);
                w->phase &= 3;
            }
            break;
        case 1:
            sndPlay(0x82CC);
            break;
        case 0:
            if (sndGetVolume(0x2CC, 0) == 0) {
                sndPlay(0x2CC);
            }
            if (160.8811951f <= ++w->timer) {
                namiMake(o, w->x, w->y, w->z, w->rot[1], 1.0f, namiHeights[w->phase++],
                                       w->count * 160.8811951f, 0);
                w->timer = 0;
                w->phase &= 3;
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
