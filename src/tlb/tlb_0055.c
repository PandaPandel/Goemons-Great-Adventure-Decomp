#include "tlb.h"

/* The water and its enemies: the scrolling water surfaces, "water", "serimaster", "serio",
 * "seria" and "ryutoge". */

extern u8 waterDrawScroll32Tex[];
extern u8 waterDrawFallTex[];
extern Gfx waterModel1Dl[];
extern Gfx waterModel2Dl[];
extern Gfx waterModel3Dl[];
extern Gfx waterModel4Dl[];
extern Gfx waterModel5Dl[];
extern Gfx waterModel6Dl[];
extern u8 seriRide[]; /* the seri's ride data (kinds 0x78/0x79's file set 0x60): the same address, another file */
extern Gfx waterModel7Dl[];
extern Gfx waterModel8Dl[];
extern Gfx waterModel9Dl[];
extern Gfx waterModel10Dl[];
extern Gfx waterModel11Dl[];
extern Gfx waterModel12Dl[];
extern u8 waterModel1Ride[];
extern u8 waterModel3Ride[];
extern u8 waterModel4Ride[];
extern u8 waterModel10Ride[];
extern u8 waterModel22Ride[];
extern u8 waterModel23Ride[];

/* A "water" surface's work. */
typedef struct WaterWork {
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
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ u32 scroll;
    /* 0x40 */ s32 step;
    /* 0x44 */ Gfx *(*draw)(Gfx *gfx, u16 scroll);
} WaterWork;

/* The lifts' sound master's work ("serimaster"). */
typedef struct SeriMasterWork {
    /* 0x00 */ u8 unused0;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad9[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u16 timer;
    /* 0x36 */ u16 snd;     /* the lift sound, kept playing while a lift asks */
    /* 0x38 */ u16 sndStep; /* every 8 steps after, and when the master goes */
    /* 0x3A */ s16 rest;    /* the level with no request (-255) */
    /* 0x3C */ s16 level;   /* this step's loudest request (the sound's parameter 1) */
    /* 0x3E */ s16 param2;  /* its parameter 2 */
} SeriMasterWork;

/* A lift's work ("serio"). */
typedef struct SerioWork {
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
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ ACollFrame *box;
    /* 0x40 */ f32 base; /* its place */
    /* 0x44 */ f32 ofs;  /* how far up it is (message 9) */
} SerioWork;

/* A riding lift's work ("seria"). */
typedef struct SeriaWork {
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
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ ACollFrame *box;
    /* 0x40 */ Obj *master; /* its sound master ("serimaster") */
    /* 0x44 */ u8 mode;     /* 0 waiting for a rider, 1 rising, 2 at the top, 3 sinking, 4 at the bottom */
    /* 0x45 */ u8 riders;   /* this step's (message 6) */
    /* 0x46 */ u16 timer;
    /* 0x48 */ f32 base;    /* its place */
    /* 0x4C */ f32 ofs;     /* how far up it is */
    /* 0x50 */ f32 speed;
    /* 0x54 */ f32 top;
} SeriaWork;

/* The turning spiked floor's work ("ryutoge"). */
typedef struct RyutogeWork {
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
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ ACollFrame *box[4]; /* its spikes */
    /* 0x4C */ f32 spin;           /* its roll per step (pi / 32 either way, or 0) */
    /* 0x50 */ f32 turn;           /* this step's */
    /* 0x54 */ u8 mode;            /* 0 waiting, 1 rolling over, 2 waiting upside down, 3 rolling back */
    /* 0x56 */ s16 timer;
} RyutogeWork;

/* acollAtkAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy,
                                     s16 oz, s32 a);

extern Gfx ryutogeDl[];
extern u8 ryutogeRide[];

/* acollAtkAddYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);

extern Gfx seriDl[];

s32 seriMasterProc(Obj *o, s16 msg, void *arg);
Gfx *waterDrawScroll32(Gfx *gfx, u16 scroll);
Gfx *waterDrawFall(Gfx *gfx, u16 scroll);
Gfx *waterDrawScroll64(Gfx *gfx, u16 scroll);

/* A 32 x h I8 texture loaded at tmem and drawn through tile rtile from (uls, ult): one macro,
 * its seven commands on one source line (as tlb_0053's RGBA16 one). */
#define gDPLoadScrollBlockI8(pkt, timg, h, maskt, tmem, rtile, uls, ult)                                  \
    {                                                                                                     \
        gDPSetTextureImage(pkt, G_IM_FMT_I, G_IM_SIZ_16b, 1, timg);                                       \
        gDPSetTile(pkt, G_IM_FMT_I, G_IM_SIZ_16b, 0, tmem, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR,    \
                   maskt, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD);                          \
        gDPLoadSync(pkt);                                                                                 \
        gDPLoadBlock(pkt, G_TX_LOADTILE, 0, 0, 32 * (h) / 2 - 1, CALC_DXT(32, G_IM_SIZ_8b_BYTES));        \
        gDPPipeSync(pkt);                                                                                 \
        gDPSetTile(pkt, G_IM_FMT_I, G_IM_SIZ_8b, 4, tmem, rtile, 0, G_TX_WRAP | G_TX_NOMIRROR, maskt,      \
                   G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD);                                 \
        gDPSetTileSize(pkt, rtile, uls, ult, 31 << G_TEXTURE_IMAGE_FRAC, ((h) - 1) << G_TEXTURE_IMAGE_FRAC); \
    }

void waterDummy(void) {
}

Gfx *waterDrawScroll32(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetCombineLERP(gfx++, TEXEL0, TEXEL1, ENV_ALPHA, TEXEL1, 0, 0, 0, PRIMITIVE, PRIMITIVE, ENVIRONMENT,
                      COMBINED, ENVIRONMENT, 0, 0, 0, COMBINED);
    gDPSetPrimColor(gfx++, 0, 0, 0xFF, 0xFF, 0xFF, 0xA0);
    gDPSetEnvColor(gfx++, 0x10, 0x40, 0xFF, 0x80);
    gDPLoadScrollBlockI8(gfx++, waterDrawScroll32Tex, 32, 5, 0, G_TX_RENDERTILE, scroll, scroll);
    gDPLoadScrollBlockI8(gfx++, waterDrawScroll32Tex, 32, 5, 0x100, 1, scroll + 10, -scroll);
    gSPEndDisplayList(gfx++);
    return gfx;
}

Gfx *waterDrawFall(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetCombineLERP(gfx++, 0, 0, 0, 1, TEXEL0, TEXEL1, ENVIRONMENT, TEXEL1, PRIMITIVE, ENVIRONMENT,
                      COMBINED_ALPHA, ENVIRONMENT, COMBINED, 0, PRIMITIVE, 0);
    gDPSetPrimColor(gfx++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    gDPSetEnvColor(gfx++, 0x80, 0xA0, 0xFF, 0x80);
    gDPLoadScrollBlockI8(gfx++, waterDrawFallTex, 64, 6, 0, G_TX_RENDERTILE, 0, scroll);
    gDPLoadScrollBlockI8(gfx++, waterDrawFallTex, 64, 6, 0x100, 1, 10, scroll * 2);
    gSPEndDisplayList(gfx++);
    return gfx;
}

Gfx *waterDrawScroll64(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetCombineLERP(gfx++, TEXEL0, TEXEL1, ENV_ALPHA, TEXEL1, 0, 0, 0, PRIMITIVE, PRIMITIVE, ENVIRONMENT,
                      COMBINED, ENVIRONMENT, 0, 0, 0, COMBINED);
    gDPSetPrimColor(gfx++, 0, 0, 0xFF, 0xFF, 0xFF, 0xA0);
    gDPSetEnvColor(gfx++, 0x10, 0x40, 0xFF, 0x80);
    gDPLoadScrollBlockI8(gfx++, waterDrawFallTex, 64, 6, 0, G_TX_RENDERTILE, 0, scroll);
    gDPLoadScrollBlockI8(gfx++, waterDrawFallTex, 64, 6, 0x100, 1, 10, scroll * 2);
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* "water": a scrolling water surface (its kind's value: the model in the low byte, the heading
 * in 512ths of a half turn in the high half); some carry a ride (a floor to stand on). */
s32 waterProc(Obj *o, s16 msg, void *arg) {
    WaterWork *w = (WaterWork *) FX(o);
    s32 pad[2];
    s32 data;

    switch (msg) {
        case 0:
            break;
        case 2: {
            Gfx *dl;
            u32 ride;
            s32 model;

            o->name = "water";
            data = eneSetup(o, arg);
            w->draw = waterDrawScroll32;
            w->step = -1;
            dl = NULL;
            ride = 0;
            model = data & 0xFF;
            switch (model) {
                /* each model's settings on its own line(s): IDO schedules by line */
                case 1:
                    dl = waterModel1Dl; ride = (u32) waterModel1Ride;
                    w->draw = waterDrawScroll64; w->step = -6;
                    break;
                case 2:
                    dl = waterModel2Dl;
                    break;
                case 3:
                    dl = waterModel3Dl; ride = (u32) waterModel3Ride;
                    break;
                case 4:
                    dl = waterModel4Dl; ride = (u32) waterModel4Ride;
                    break;
                case 5:
                    dl = waterModel5Dl; w->draw = waterDrawFall; w->step = -6;
                    break;
                case 6:
                    dl = waterModel6Dl; w->draw = waterDrawFall; w->step = -6;
                    break;
                case 7:
                    dl = waterModel7Dl; w->draw = waterDrawFall; w->step = -6;
                    break;
                case 8:
                    dl = waterModel8Dl; w->draw = waterDrawFall; w->step = -6;
                    break;
                case 9:
                    dl = waterModel9Dl; w->draw = waterDrawFall; w->step = -6;
                    break;
                case 10:
                    dl = waterModel10Dl; ride = (u32) waterModel10Ride;
                    break;
                case 11:
                    dl = waterModel11Dl; w->draw = waterDrawFall; w->step = -6;
                    break;
                case 12:
                    dl = waterModel12Dl; w->draw = waterDrawFall; w->step = -6;
                    break;
                case 22:
                    ride = (u32) waterModel22Ride;
                    break;
                case 23:
                    ride = (u32) waterModel23Ride;
                    break;
            }
            w->pol = eneNewPol(o, dl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayers[6]);
            w->pol->scale[2] = 0.1f;
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
            w->rot[1] = (data >> 16) * 3.141592741f / 512;
            if (ride != 0) {
                w->ride = rideCreate(gRideTree2, (s32) o, &w->pol->f, (RideData *) eneFileAddr(o, ride, 0));
            }
            break;
        }
        case 1:
            if (w->ride != NULL) {
                frmDestroy(&w->ride->f);
            }
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            break;
        case 4:
            if (arg != NULL) {
                ((f32 *) arg)[0] = w->pol->pos[0];
                ((f32 *) arg)[1] = w->pol->pos[1];
                ((f32 *) arg)[2] = w->pol->pos[2];
            }
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gGfxDlHead = w->draw(gGfxDlHead, w->scroll);
            w->scroll += w->step;
            eneSetModelOnCourse(o, &w->pol->f);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "serimaster": the lifts' sound. Each lift asks for a level every step (seriMasterRequest); the
 * loudest wins: the lift sound is kept playing at it, and with no request the step sound plays
 * every 8 steps while the lift sound fades out. Message 10 finds the master of a sound. */
s32 seriMasterProc(Obj *o, s16 msg, void *arg) {
    SeriMasterWork *w = (SeriMasterWork *) FX(o);
    u32 *data;
    s32 pad[5]; /* the game's frame: 0x48 */

    switch (msg) {
        case 2:
            o->name = "serimaster";
            data = (u32 *) eneSetup(o, arg);
            w->snd = data[1];
            w->sndStep = data[2];
            w->rest = -255;
            w->level = w->rest;
            w->param2 = 0;
            break;
        case 1:
            sndPlay(w->sndStep);
            break;
        case 10:
            if ((u32) arg == w->snd) {
                return (s32) o;
            }
            break;
        case 0:
            w->timer++;
            if (w->rest >= w->level) {
                if (sndGetVolume(w->snd, 0) != 0 && (w->timer & 7) == 1) {
                    sndPlay(w->sndStep);
                }
            } else {
                if (sndGetVolume(w->snd, 0) == 0) {
                    sndPlay(w->snd);
                }
                sndSetParam(w->snd, 1, w->level);
                sndSetParam(w->snd, 2, w->param2);
            }
            w->level = w->rest;
            w->param2 = 0;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* A lift's request to its sound master: the loudest this step wins. */
void seriMasterRequest(Obj *master, s16 level, s16 param2) {
    SeriMasterWork *w = (SeriMasterWork *) FX(master);

    if (master->proc == seriMasterProc) {
        if (level >= w->level) {
            w->level = level;
            w->param2 = param2;
        }
    }
}

/* "serio": a lift, its ride and an attack box under it; message 9 sets how far up it is (the
 * negated argument), its kind's value is its heading in 512ths of a half turn. */
s32 serioProc(Obj *o, s16 msg, void *arg) {
    SerioWork *w = (SerioWork *) FX(o);
    Obj *by;
    s32 i;

    switch (msg) {
        case 2:
            o->name = "serio";
            i = eneSetup(o, arg);
            w->pol = eneNewPol(o, seriDl);
            if (w->pol == NULL) {
                return -1;
            }
            w->ride = rideCreate(gRideTree1, (s32) o, &w->pol->f, (RideData *) eneFileAddr(o, seriRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            w->box = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->pol->f, 125, -225, 0, 0);
            if (w->box == NULL) {
                return -1;
            }
            w->box->kind = 125;
            w->box->flags |= 0x10;
            w->rot[1] = i * 3.141592741f / 512;
            w->base = w->y;
            break;
        case 1:
            frmDestroy(&w->box->frame);
            frmDestroy(&w->ride->f);
            frmDestroy(&w->pol->f);
            break;
        case 9:
            w->ofs = -(s32) arg;
            break;
        case 0:
            if (w->box != NULL && acollAtkWasHit(w->box) == 1) {
                by = (Obj *) w->box->hitPower;
                if (by != NULL) {
                    tskSendMsg(by, 7, o);
                }
            }
            w->y = w->base + w->ofs;
            eneSetModelOnCourse(o, &w->pol->f);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "seria": a lift that rises while ridden (message 6 each step from a rider), waits 30 steps at
 * its top, sinks back and waits again; it carries its child "serio" (message 9: how far up)
 * and asks its sound master for the lift sound while rising. Its kind's value: the heading in
 * the high half, its height in bits 8-15, bit 7 to make the child, bit 0 the second sound. */
s32 seriaProc(Obj *o, s16 msg, void *arg) {
    SeriaWork *w = (SeriaWork *) FX(o);
    Obj *by;
    s32 i;
    u16 snd;
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "seria";
            i = eneSetup(o, arg);
            w->pol = eneNewPol(o, seriDl);
            if (w->pol == NULL) {
                return -1;
            }
            w->ride = rideCreate(gRideTree1, (s32) o, &w->pol->f, (RideData *) eneFileAddr(o, seriRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            w->box = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->pol->f, 120, 225, 0, 0);
            if (w->box == NULL) {
                return -1;
            }
            w->box->kind = 125;
            w->rot[1] = (i >> 16) * 3.141592741f / 512;
            w->base = w->y;
            w->top = (i >> 8) & 0xFF;
            if (i & 0x80) {
                eneMakeEx(o, ENE_SERIO, w->zone, w->x, w->y + 75.0f + w->top * 2, w->z, 2,
                          (u32 (*)(void *, void *, void *)) (i >> 16));
            }
            if (i & 1) {
                snd = 0x236;
            } else {
                snd = 0x235;
            }
            w->master = (Obj *) tskSendMsg(gEneManager, 10, (void *) snd);
            break;
        case 1:
            frmDestroy(&w->box->frame);
            frmDestroy(&w->ride->f);
            frmDestroy(&w->pol->f);
            break;
        case 6:
            w->riders++;
            ((f32 *) arg)[3] += w->speed;
            break;
        case 0:
            w->timer++;
            switch (w->mode) {
                case 0:
                    w->speed = 0.0f;
                    if (w->riders != 0) {
                        w->mode = 1;
                    }
                    break;
                case 1:
                    if (w->master != NULL) {
                        seriMasterRequest(w->master, -w->ofs / 4 + 10.0f,
                                               soundPanGet(gGameWork.view, w->pol->pos[0], w->pol->pos[2],
                                                           1.570796371f) - 64);
                    }
                    w->speed = 2.0f;
                    if (w->top < w->ofs + w->speed) {
                        if (o->child != NULL) {
                            soundIncamCam(0x23E, w->pol->pos[0], w->pol->pos[1], w->pol->pos[2]);
                        }
                        w->ofs = w->top;
                        w->timer = 0;
                        w->mode = 2;
                        w->speed = 0.0f;
                    }
                    break;
                case 2:
                    w->speed = 0.0f;
                    if (w->timer >= 31) {
                        w->mode = 3;
                    }
                    break;
                case 3:
                    w->speed = -1.0f;
                    if (w->ofs + w->speed < 0.0f) {
                        w->speed = 0.0f;
                        w->ofs = 0.0f;
                        w->timer = 0;
                        w->mode = 4;
                    }
                    break;
                case 4:
                    w->speed = 0.0f;
                    if (w->timer >= 31) {
                        w->mode = 0;
                    }
                    break;
            }
            w->riders = 0;
            w->ofs += w->speed;
            w->y = w->base + w->ofs;
            if (o->child != NULL) {
                tskSendMsg(o->child, 9, (void *) (s32) w->ofs);
            }
            if (w->box != NULL && acollAtkWasHit(w->box) == 1) {
                by = (Obj *) w->box->hitPower;
                if (by != NULL) {
                    tskSendMsg(by, 7, o);
                }
            }
            eneSetModelOnCourse(o, &w->pol->f);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* An attack box of radius 40 at (x, y, z) from a model, of kind 120 (the ryutoge's spikes). */
ACollFrame *ryutogeAtkMake(Frame *model, f32 x, f32 y, f32 z) {
    ACollFrame *box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, model, 40, x, y, z, 0);

    if (box == NULL) {
        return NULL;
    }
    box->kind = 120;
    return box;
}

/* "ryutoge": a spiked floor that turns over: it waits 90 steps, rolls half a turn (its kind's
 * value: 1 one way, 2 the other, 0 not at all), waits 90 steps and rolls back. Its four spikes
 * hurt; message 7 answers 3. */
s32 ryutogeProc(Obj *o, s16 msg, void *arg) {
    RyutogeWork *w = (RyutogeWork *) FX(o);
    Obj *by;
    s32 i;
    s32 data;

    switch (msg) {
        case 2:
            o->name = "ryutoge";
            data = eneSetup(o, arg);
            w->pol = eneNewPol(o, ryutogeDl);
            if (w->pol == NULL) {
                return -1;
            }
            w->ride = rideCreate(gRideTree1, (s32) o, &w->pol->f, (RideData *) eneFileAddr(o, ryutogeRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            w->box[0] = ryutogeAtkMake(&w->pol->f, -75.0f, 60.0f, -50.0f);
            w->box[1] = ryutogeAtkMake(&w->pol->f, -75.0f, 60.0f, 50.0f);
            w->box[2] = ryutogeAtkMake(&w->pol->f, 75.0f, 60.0f, -50.0f);
            w->box[3] = ryutogeAtkMake(&w->pol->f, 75.0f, 60.0f, 50.0f);
            if (w->zone) {
                w->rot[1] = vramCourseHeading(w->zone, w->x) - 3.141592741f;
            } else {
                w->rot[1] = (data >> 16) * 3.141592741f / 512;
            }
            switch (data) {
                case 0:
                    w->spin = 0.0f;
                    break;
                case 1:
                    w->spin = 0.09817477316f;
                    break;
                case 2:
                    w->spin = -0.09817477316f;
                    break;
            }
            break;
        case 1:
            for (i = 0; i < 4; i++) {
                frmDestroy(&w->box[i]->frame);
            }
            frmDestroy(&w->ride->f);
            frmDestroy(&w->pol->f);
            break;
        case 7:
            if (arg != NULL) {
                *(s32 *) arg = 3;
            }
            break;
        case 0:
            w->timer++;
            switch (w->mode) {
                case 0:
                    w->turn = 0.0f;
                    w->rot[0] = 0.0f;
                    if (w->timer >= 91) {
                        if (w->spin != 0.0f) {
                            soundIncamCam(0x460, w->pol->pos[0], w->pol->pos[1], w->pol->pos[2]);
                        }
                        w->timer = 0;
                        w->mode = 1;
                    }
                    break;
                case 1:
                    w->turn = w->spin;
                    w->rot[0] += w->turn;
                    if (3.141592741f <= fabsf(w->rot[0])) {
                        soundIncamCam(0x2DB, w->pol->pos[0], w->pol->pos[1], w->pol->pos[2]);
                        w->timer = 0;
                        w->mode = 2;
                    }
                    break;
                case 2:
                    w->turn = 0.0f;
                    w->rot[0] = 3.141592741f;
                    if (w->timer >= 91) {
                        soundIncamCam(0x460, w->pol->pos[0], w->pol->pos[1], w->pol->pos[2]);
                        w->timer = 0;
                        w->mode = 3;
                    }
                    break;
                case 3:
                    w->turn = w->spin;
                    w->rot[0] += w->turn;
                    if (w->rot[0] <= 0.0f || 6.283185482f <= w->rot[0]) {
                        soundIncamCam(0x2DB, w->pol->pos[0], w->pol->pos[1], w->pol->pos[2]);
                        w->timer = 0;
                        w->mode = 0;
                    }
                    break;
            }
            for (i = 0; i < 4; i++) {
                if (w->box[i] != NULL && acollAtkWasHit(w->box[i]) == 1) {
                    by = (Obj *) w->box[i]->hitPower;
                    if (by != NULL) {
                        tskSendMsg(by, 7, o);
                    }
                }
            }
            eneSetModelOnCourse(o, &w->pol->f);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
