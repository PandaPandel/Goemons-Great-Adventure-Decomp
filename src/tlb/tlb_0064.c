#include "tlb.h"

#define sinf (*sinf)
#define cosf (*cosf)

extern Frame *gFrmLayer3;

extern u8 trapBelconDrawScrollTex[];
extern Gfx trapBelconData1Dl[];
extern Gfx trapBelconData2Dl[];
extern Gfx trapBelconData3Dl[];
extern u8 trapBelconData1Anim[];
extern u8 trapBelconData2Anim[];
extern u8 trapBelconData3Anim[];
extern u8 trapBelconData1Ride[];
extern u8 trapBelconData2Ride[];
extern u8 trapBelconData3Ride[];
extern u8 trapBpropAnim[];
extern u8 trapBpropRide[];
extern u8 trapGuppaAnim[];
extern u8 trapGuppaAtkChain[];
extern Gfx trapFireballDl[];
extern Gfx trapBtogeDl[];

s32 trapFireballProc(Obj *o, s16 msg, void *arg);

/* A message 6's argument: where the player steps to, on the course (plmove.c). */
typedef struct PlPlace {
    /* 0x0 */ s16 zone;
    /* 0x4 */ f32 place;
    /* 0x8 */ f32 side;
    /* 0xC */ f32 height;
} PlPlace;

/* belcon: a conveyor belt, a ride whose texture scrolls; work at its task + 0x20. */
typedef struct BelconWork {
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
    /* 0x38 */ AnimeFrame *anim;
    /* 0x3C */ RideFrame *ride;
    /* 0x40 */ f32 speed;
    /* 0x44 */ s32 scroll;
} BelconWork;

/* bprop: a platform bobbing up and down; work at its task + 0x20. */
typedef struct BpropWork {
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
    /* 0x34 */ AnimeFrame *anim;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ f32 phase;
    /* 0x40 */ f32 step;
    /* 0x44 */ f32 baseY;
    /* 0x48 */ f32 amp;
} BpropWork;

/* guppa: a trap that snaps shut on a cycle (its attack box live while it closes); work at its
 * task + 0x20. */
typedef struct GuppaWork {
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
    /* 0x34 */ AnimeFrame *anim;
    /* 0x38 */ ACollFrame *atk;
    /* 0x3C */ u8 phase; /* 0 open, 1 closing, 2 shut, 3 opening */
    /* 0x3E */ u16 count;
} GuppaWork;

/* fireball: a ball of fire the firebell spits, rising and growing until it fades; work at its
 * task + 0x20. */
typedef struct FireballWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ Anim anim; /* its texture's (gItexTimedYellowAnim) */
    /* 0x14 */ s32 alpha;
    /* 0x18 */ ACollFrame *atk;
    /* 0x1C */ s32 life; /* steps before it fades */
    /* 0x20 */ f32 vel[3];
} FireballWork;

/* firebell: a bell that spits fireballs for 90 steps, then rests for 90; work at its task +
 * 0x20. */
typedef struct FirebellWork {
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
    /* 0x34 */ u8 mode; /* 0 spitting, 1 resting */
    /* 0x38 */ u32 count;
    /* 0x3C */ f32 dir[3]; /* a fireball's velocity */
} FirebellWork;

/* btoge: a spike, its attack box on a pol model; work at its task + 0x20. */
typedef struct BtogeWork {
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
    /* 0x38 */ ACollFrame *atk;
} BtogeWork;

/* acoll.c defines the box makers void; they return the box. */
typedef ACollFrame *(*AtkAddYZ)(Frame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*AtkAddXYZ)(Frame *parent, Frame *model, u16 radius, s16 ox, s16 oy, s16 oz, s32 a);

/* The belt's texture, scrolled down by `scroll`. */
Gfx *trapBelconDrawScroll(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetTextureImage(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, trapBelconDrawScrollTex);
    gDPSetTile(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP, 5, G_TX_NOLOD, G_TX_MIRROR,
               5, G_TX_NOLOD);
    gDPLoadSync(gfx++);
    gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, 1023, 256);
    gDPPipeSync(gfx++);
    gDPSetTile(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0, G_TX_RENDERTILE, 0, G_TX_WRAP, 5, G_TX_NOLOD, G_TX_MIRROR,
               5, G_TX_NOLOD);
    gDPSetTileSize(gfx++, G_TX_RENDERTILE, 0, scroll, 31 << 2, 31 << 2);
    gSPEndDisplayList(gfx++);
    return gfx;
}

s32 trapBelconProc(Obj *o, s16 msg, void *arg) {
    BelconWork *w = (BelconWork *) FX(o);
    s32 data;
    s32 pad;
    u32 anim;
    Gfx *pol;
    u32 ride;
    s32 pad2;
    f32 r;

    switch (msg) {
        case 2:
            o->name = "belcon";
            data = eneSetup(o, arg);
            switch (data & 0xFF) {
                case 1:
                    anim = (u32) trapBelconData1Anim; ride = (u32) trapBelconData1Ride;
                    pol = trapBelconData1Dl;
                    w->rot[1] = (data >> 16) * 3.1415927f / 512;
                    break;
                case 2:
                    anim = (u32) trapBelconData2Anim; ride = (u32) trapBelconData2Ride;
                    pol = trapBelconData2Dl;
                    w->rot[1] = (data >> 16) * 3.1415927f / 512;
                    break;
                case 3:
                    anim = (u32) trapBelconData3Anim; ride = (u32) trapBelconData3Ride;
                    pol = trapBelconData3Dl;
                    w->rot[0] = (data >> 16) * 3.1415927f / 512;
                    w->rot[1] = 3.926991f;
                    break;
            }
            w->speed = (s8) ((data >> 8) & 0xFF);
            if (pol != NULL) {
                w->pol = eneNewPol(o, pol);
            }
            if (anim != 0) {
                w->anim = eneNewAnime(o, anim);
            }
            if (w->pol == NULL || w->anim == NULL) {
                return -1;
            }
            frmMove((Frame *) w->pol, gFrmLayer3);
            frmMove((Frame *) w->anim, gFrmLayer3);
            if (ride != 0) {
                w->ride = rideCreate(gRideTree1, (s32) o, (Frame *) w->pol, (RideData *) eneFileAddr(o, ride, 0));
            }
            if (w->ride == NULL) {
                return -1;
            }
            break;
        case 1:
            frmDestroy((Frame *) w->ride);
            frmDestroy((Frame *) w->anim);
            frmDestroy((Frame *) w->pol);
            break;
        case 7:
            if (arg != NULL) {
                *(s32 *) arg = 3;
            }
            break;
        case 6:
            if (arg != NULL) {
                r = w->speed * 0.32f;
                ((PlPlace *) arg)->place += r * mathCos(w->rot[0]);
                ((PlPlace *) arg)->height += r * mathSin(w->rot[0]);
            }
            break;
        case 0:
            w->scroll += w->speed * 2;
            eneAnimLoop(o, w->anim, w->speed * 0.1f);
            eneSetModelOnCourse(o, (Frame *) w->pol);
            eneSetModelOnCourse(o, (Frame *) w->anim);
            eneStepHookPost(o);
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gGfxDlHead = trapBelconDrawScroll(gGfxDlHead, w->scroll);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

s32 trapBpropProc(Obj *o, s16 msg, void *arg) {
    BpropWork *w = (BpropWork *) FX(o);
    s32 pad;
    s32 data;
    f32 deg = 180.0f;
    s32 n = 180;
    s32 d;
    s32 pad2;

    switch (msg) {
        case 2:
            o->name = "bprop";
            data = eneSetup(o, arg);
            w->anim = eneNewAnime(o, (u32) trapBpropAnim);
            if (w->anim == NULL) {
                return -1;
            }
            frmMove((Frame *) w->anim, gFrmLayer3);
            w->ride = rideCreate(gRideTree1, (s32) o, (Frame *) w->anim, (RideData *) eneFileAddr(o, trapBpropRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            w->step = 6.2831855f / deg;
            w->baseY = w->y;
            w->amp = data & 0xFF;
            w->rot[1] = (data >> 16) * 3.1415927f / 512;
            d = (data >> 8) & 0xFF;
            switch (d) {
                case 0:
                default:
                    d = 0;
                    break;
                case 1:
                    d = 45;
                    break;
                case 2:
                    d = 90;
                    break;
                case 3:
                    d = 135;
                    break;
            }
            w->phase = ((d + gGameFrameCount) % n * 2) * 3.1415927f / n;
            break;
        case 1:
            frmDestroy((Frame *) w->ride);
            frmDestroy((Frame *) w->anim);
            break;
        case 6:
            if (arg != NULL) {
                ((PlPlace *) arg)->height += cosf(w->phase) * w->step * w->amp;
            }
            break;
        case 0:
            w->phase += w->step;
            w->y = sinf(w->phase) * w->amp + w->baseY;
            eneAnimLoop(o, w->anim, -0.4f);
            eneSetModelOnCourse(o, (Frame *) w->anim);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

s32 trapGuppaProc(Obj *o, s16 msg, void *arg) {
    GuppaWork *w = (GuppaWork *) FX(o);
    Obj *by;
    s32 data;

    switch (msg) {
        case 2:
            o->name = "guppa";
            data = eneSetup(o, arg);
            w->anim = eneNewAnime(o, (u32) trapGuppaAnim);
            if (w->anim == NULL) {
                return -1;
            }
            w->atk = aniAtkCreate((Frame *) ene_atk_root_frame, (Frame *) w->anim, (u32) trapGuppaAtkChain,
                                  eneGetFile(o, 0), 0);
            if (w->atk == NULL) {
                PRINTF("攻撃フレームが足りません\n");
                return -1;
            }
            w->atk->kind = 0x78;
            w->rot[1] = (data >> 16) * 3.1415927f / 512;
            break;
        case 1:
            if (w->atk != NULL) {
                aniAtkDelete(w->atk);
            }
            frmDestroy((Frame *) w->anim);
            break;
        case 0:
            w->count++;
            switch (w->phase) {
                case 0:
                    if (w->atk != NULL) {
                        w->atk->flags &= ~2;
                    }
                    if (w->count > 60) {
                        w->count = 0;
                        w->phase = 1;
                    }
                    break;
                case 1:
                    if (w->atk != NULL) {
                        w->atk->flags |= 2;
                    }
                    if (eneAnimPlayOnce(o, w->anim, 0.5f)) {
                        w->count = 0;
                        w->phase = 2;
                    }
                    break;
                case 2:
                    if (w->atk != NULL) {
                        w->atk->flags &= ~2;
                    }
                    if (w->count > 30) {
                        w->count = 0;
                        w->phase = 3;
                    }
                    break;
                case 3:
                    if (w->atk != NULL) {
                        w->atk->flags &= ~2;
                    }
                    if (eneAnimPlayOnce(o, w->anim, -0.2f)) {
                        w->count = 0;
                        w->phase = 0;
                    }
                    break;
            }
            eneSetModelOnCourse(o, (Frame *) w->anim);
            if (w->atk != NULL) {
                ((AniAtkFrame *) w->atk->model)->time = w->anim->frame;
                if (acollAtkWasHit(w->atk) == 1) {
                    by = (Obj *) w->atk->hitPower;
                    if (by != NULL) {
                        tskSendMsg(by, 7, o);
                    }
                }
            }
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "fireball": the twin of tlb_0062's kpaFireballProc, with a life for its time.
 * Matched 2026-10-12; it was parked for the life's decrement (its registers and one stack slot). */
s32 trapFireballProc(Obj *o, s16 msg, void *arg) {
    FireballWork *w = (FireballWork *) FX(o);
    s32 n; /* the texture file's index in case 2, the life left in case 0 */

    switch (msg) {
        case 2:
            o->name = "fireball";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = trapFireballDl;
            floadGetSetRam((void **) &w->frame->segs[0], 0x125);
            n = floadGetSetFile(0x125, 0);
            fadeAnimStart(&w->anim, gItexTimedYellowAnim, 0, &w->frame->segs[3], n);
            w->frame->flags |= 7;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 0.1f;
            w->alpha = 0xFF;
            w->atk = ((AtkAddYZ) acollAtkAddYZ)((Frame *) ene_atk_root_frame, (Frame *) w->frame, 20, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x78;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            frmDestroy((Frame *) w->atk);
            break;
        case 0:
            fadeAnimStep(&w->anim);
            w->vel[1] += 0.01;
            w->frame->pos[0] += w->vel[0];
            w->frame->pos[1] += w->vel[1];
            w->frame->pos[2] += w->vel[2];
            w->frame->scale[0] *= 1.01f;
            w->frame->scale[1] *= 1.01f;
            w->frame->scale[2] *= 1.01f;
            n = --w->life;
            if (n < 0) {
                w->alpha -= 16;
                if (w->alpha < 0) {
                    return -1;
                }
            }
            if (w->atk != NULL && acollAtkWasHit(w->atk) == 1 && (Obj *) w->atk->hitPower != NULL) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, o);
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, ENVIRONMENT, 0, TEXEL0, 0,
                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, ENVIRONMENT, 0, TEXEL0, 0);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 128, 0);
            gDPSetEnvColor(gGfxDlHead++, 255, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Spits a fireball from parent at (x, y, z) moving by (vx, vy, vz), for life steps. */
Obj *trapFireballCreate(Obj *parent, f32 x, f32 y, f32 z, f32 vx, f32 vy, f32 vz, f32 life) {
    Obj *o = tskCreate(parent, trapFireballProc, parent->file, NULL);

    if (o != NULL) {
        ((FireballWork *) FX(o))->frame->pos[0] = x;
        ((FireballWork *) FX(o))->frame->pos[1] = y;
        ((FireballWork *) FX(o))->frame->pos[2] = z;
        ((FireballWork *) FX(o))->vel[0] = vx;
        ((FireballWork *) FX(o))->vel[1] = vy;
        ((FireballWork *) FX(o))->vel[2] = vz;
        ((FireballWork *) FX(o))->life = life;
    }
    return o;
}

s32 trapFirebellProc(Obj *o, s16 msg, void *arg) {
    FirebellWork *w = (FirebellWork *) FX(o);
    s32 pad;
    s32 pad2;
    f32 x;
    f32 z;
    CamView view;

    switch (msg) {
        case 2:
            o->name = "firebell";
            eneSetup(o, arg);
            if (w->zone) {
                w->rot[1] = vramCourseHeading(w->zone, w->x);
                vramCoursePointXZ(w->zone, w->x, w->z, &x, &z);
                w->zone = 0;
                w->x = x;
                w->z = z;
            }
            w->dir[0] = 0.0f;
            w->dir[1] = 0.0f;
            w->dir[2] = 2.0f;
            mathVecRotateY(w->rot[1] - 1.5707964f, &w->dir[0], &w->dir[1], &w->dir[2]);
            break;
        case 1:
            break;
        case 0:
            w->count++;
            switch (w->mode) {
                case 0:
                    if (w->count == 1 && gGameWork.view != NULL) {
                        view = *gGameWork.view;
                        view.fovy = 1.0471976f;
                        if (soundInFovCheck(&view, w->x, w->y, w->z)) {
                            sndPlay(0x29E);
                        }
                    }
                    if (w->count > 90) {
                        w->mode = 1;
                        w->count = 0;
                    }
                    if ((w->count & 7) == 1) {
                        trapFireballCreate(o, w->x, w->y, w->z, w->dir[0], w->dir[1], w->dir[2], 32.0f);
                    }
                    break;
                case 1:
                    if (w->count > 90) {
                        w->mode = 0;
                        w->count = 0;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

s32 trapBtogeProc(Obj *o, s16 msg, void *arg) {
    BtogeWork *w = (BtogeWork *) FX(o);
    s32 data;
    Obj *by;
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "btoge";
            data = eneSetup(o, arg);
            w->rot[0] = (data & 0xFFFF) * 3.1415927f / 512;
            w->rot[1] = (data >> 16) * 3.1415927f / 512;
            w->pol = eneNewPol(o, trapBtogeDl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove((Frame *) w->pol, gFrmLayer3);
            w->atk = ((AtkAddXYZ) acollAtkAddXYZ)((Frame *) ene_atk_root_frame, (Frame *) w->pol, 100, 0, 64, -108, 0);
            if (w->atk == NULL) {
                return 0;
            }
            w->atk->kind = 0x78;
            break;
        case 1:
            frmDestroy((Frame *) w->atk);
            frmDestroy((Frame *) w->pol);
            break;
        case 0:
            if (acollAtkWasHit(w->atk) == 1) {
                by = (Obj *) w->atk->hitPower;
                if (by != NULL) {
                    tskSendMsg(by, 7, o);
                }
            }
            eneSetModelOnCourse(o, (Frame *) w->pol);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
