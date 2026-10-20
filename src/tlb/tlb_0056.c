#include "tlb.h"

/* The raft ("ikada"), which ferries the players along the course, and "pkokesi", a pole that
 * rises out of the floor and sinks back on a 210-step cycle. */

/* plCountPlayers returns tskCountChildren's count (main declares it void). */
typedef s32 (*CountFunc)(void);

/* acollAtkAddYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);

#define cosf (*cosf)

extern Frame *gFrmLayer3;
extern Gfx ikadaDl[];
extern u8 ikadaRide[];

/* The raft's work ("ikada"). */
typedef struct IkadaWork {
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
    /* 0x3C */ f32 base;  /* the middle of its run */
    /* 0x40 */ f32 range; /* half its run (its kind's value's low half) */
    /* 0x44 */ f32 ofs;   /* where it is from base */
    /* 0x48 */ f32 speed;
    /* 0x4C */ u8 mode;   /* 0, 1 waiting at the near end, 2 going, 3, 4 waiting at the far end, 5 coming back */
    /* 0x4D */ u8 riders; /* this step's (message 6) */
} IkadaWork;

extern Gfx pkokesiDl[];
extern Gfx pkokesiShadowDl[];
extern u8 pkokesiRide[];

/* The rising pole's work ("pkokesi"). */
typedef struct PkokesiWork {
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
    /* 0x40 */ PolFrame *shadow;
    /* 0x44 */ f32 base;   /* its place */
    /* 0x48 */ f32 rise;   /* how far up it is */
    /* 0x4C */ f32 height; /* how far it rises (50) */
    /* 0x50 */ u32 timer;  /* its step in the 210-step cycle */
} PkokesiWork;

/* The place along the course of the first player in play on course zone, or 0. */
f32 ikadaPlayerPlace(s32 zone) {
    s32 i;
    f32 place;
    f32 height;
    f32 side;
    s32 z;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plAskPos(gGameWork.players[i], &place, &height, &side, &z);
            if (z == zone) {
                return place;
            }
        }
    }
    return 0.0f;
}

/* "ikada": a raft that waits at one end of its run until every player is on it (or one has
 * gone past it), carries them to the other end at 1 a step, and waits there likewise. Riders
 * send message 6 each step and are moved with it. */
s32 ikadaProc(Obj *o, s16 msg, void *arg) {
    IkadaWork *w = (IkadaWork *) FX(o);
    s32 pad;
    s32 i;
    s32 pad2;

    switch (msg) {
        case 2:
            o->name = "ikada";
            i = eneSetup(o, arg);
            w->pol = eneNewPol(o, ikadaDl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayer3);
            w->ride = rideCreate(gRideTree1, (s32) o, &w->pol->f, (RideData *) eneFileAddr(o, ikadaRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            w->base = w->x;
            w->range = i & 0xFFFF;
            if (w->base < ikadaPlayerPlace(w->zone)) {
                w->mode = 4;
                w->ofs = w->range;
            } else {
                w->mode = 1;
                w->ofs = -w->range;
            }
            break;
        case 1:
            frmDestroy(&w->ride->f);
            frmDestroy(&w->pol->f);
            break;
        case 6:
            if (arg != NULL) {
                w->riders++;
                ((f32 *) arg)[1] += w->speed;
            }
            break;
        case 0:
            switch (w->mode) {
                case 0:
                    w->speed = 0.0f;
                    if (((CountFunc) plCountPlayers)() != w->riders) {
                        w->mode = 1;
                    }
                    break;
                case 1:
                    w->speed = 0.0f;
                    if (((CountFunc) plCountPlayers)() == w->riders) {
                        w->mode = 2;
                    }
                    if (w->base + w->range < ikadaPlayerPlace(w->zone)) {
                        w->mode = 2;
                    }
                    break;
                case 2:
                    w->speed = 1.0f;
                    if (w->range < w->ofs) {
                        w->mode = 3;
                    }
                    break;
                case 3:
                    w->speed = 0.0f;
                    if (((CountFunc) plCountPlayers)() != w->riders) {
                        w->mode = 4;
                    }
                    break;
                case 4:
                    w->speed = 0.0f;
                    if (((CountFunc) plCountPlayers)() == w->riders) {
                        w->mode = 5;
                    }
                    if (ikadaPlayerPlace(w->zone) < w->base - w->range) {
                        w->mode = 5;
                    }
                    break;
                case 5:
                    w->speed = -1.0f;
                    if (w->ofs < -w->range) {
                        w->mode = 0;
                    }
                    break;
            }
            w->riders = 0;
            w->ofs += w->speed;
            w->x = w->base + w->ofs;
            w->rot[1] = vramCourseHeading(w->zone, w->x) - 1.570796371f;
            eneSetModelOnCourse(o, &w->pol->f);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "pkokesi": a pole on a 210-step cycle from the frame count (its kind's value's low byte
 * staggers it by a quarter): down 30 steps, up 50 over 130, up 30, then down on a quarter
 * cosine over 20 with a sound; it is a ride and hurts. Its shadow shrinks as it rises. */
s32 pkokesiProc(Obj *o, s16 msg, void *arg) {
    PkokesiWork *w = (PkokesiWork *) FX(o);
    Obj *by;
    s32 i;
    f32 s;
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "pkokesi";
            i = eneSetup(o, arg);
            w->pol = eneNewPol(o, pkokesiDl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayer3);
            w->ride = rideCreate(gRideTree1, (s32) o, &w->pol->f, (RideData *) eneFileAddr(o, pkokesiRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            w->box = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->pol->f, 200, 250, 0, 0);
            if (w->box == NULL) {
                return -1;
            }
            w->box->kind = 125;
            w->box->flags |= 0x10;
            w->shadow = (PolFrame *) frmCreate(gFrmLayer4, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            w->shadow->dl = gShadowSetupDl;
            w->shadow->dl2 = pkokesiShadowDl;
            floadGetSetRam((void **) w->shadow->segs, 0x120);
            w->shadow->rot[0] = -1.570796371f;
            w->shadow->scale[2] = 1.0f;
            w->shadow->scale[0] = w->shadow->scale[1] = w->shadow->scale[2];
            w->height = 50.0f;
            w->rot[1] = (i >> 16) * 3.141592741f / 360;
            w->base = w->y;
            w->timer = gGameFrameCount;
            switch (i & 0xFF) {
                case 0:
                    break;
                case 1:
                    w->timer += 52;
                    break;
                case 2:
                    w->timer += 105;
                    break;
                case 3:
                    w->timer += 157;
                    break;
            }
            break;
        case 1:
            frmDestroy(&w->box->frame);
            frmDestroy(&w->ride->f);
            frmDestroy(&w->pol->f);
            frmDestroy(&w->shadow->f);
            break;
        case 0:
            w->timer++;
            w->timer %= 210;
            if (w->timer < 30) {
                w->rise = 0.0f;
            } else if (w->timer < 160) {
                w->rise = w->height * (w->timer - 30) / 130;
            } else if (w->timer < 190) {
                w->rise = w->height;
            } else {
                w->rise = cosf((w->timer - 190) * 1.570796371f / 20) * w->height;
                if (w->timer == 209) {
                    soundIncamCam(0x296, w->pol->pos[0], w->pol->pos[1], w->pol->pos[2]);
                }
            }
            w->y = w->base + w->rise;
            s = 1.0f - w->rise / 100;
            if (s < 0.0f) {
                w->shadow->f.flags &= ~2;
            } else {
                w->shadow->f.flags |= 2;
                w->shadow->scale[0] = w->shadow->scale[1] = w->shadow->scale[2] = s * 0.6f;
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
