#include "tlb.h"

/* libultra's sinf and cosf (gu.h declares them), called far like tlb.h's. */
#define sinf (*sinf)
#define cosf (*cosf)

/* Message 6's argument (plmove.c's PlPlace): a player's place on the course, pushed by a ride. */
typedef struct PlPlace {
    /* 0x0 */ s16 zone;
    /* 0x4 */ f32 place;
    /* 0x8 */ f32 side;
    /* 0xC */ f32 height;
} PlPlace;

/* "wcylinder": a rolling cylinder the player rides (an enemy's work up to 0x34). */
typedef struct WcylObj {
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
    /* 0x3C */ f32 spin; /* its roll each frame (radians) */
} WcylObj;

extern Gfx wcylinderDl[];
extern u8 wcylinderRide[];

s32 wcylinderProc(Obj *obj, s16 msg, void *arg) {
    WcylObj *w = (WcylObj *) FX(obj);
    s32 pad[4];
    f32 x;
    f32 y;
    f32 z;
    s32 data;

    switch (msg) {
        case 2:
            obj->name = "wcylinder";
            data = eneSetup(obj, (EneArg *) arg);
            if (data & 0xFF) {
                w->spin = -0.01745329238f;
            } else {
                w->spin = 0.01745329238f;
            }
            w->rot[1] = (f32) (data >> 16) * 3.141592741f / 512;
            w->pol = eneNewPol(obj, wcylinderDl);
            if (w->pol == NULL) {
                return -1;
            }
            w->ride = rideCreate(gRideTree1, (s32) obj, &w->pol->f, (RideData *) eneFileAddr(obj, wcylinderRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            break;
        case 1:
            frmDestroy(&w->ride->f);
            frmDestroy(&w->pol->f);
            break;
        case 7:
            if (arg != NULL) {
                *(s32 *) arg = 3;
            }
            break;
        case 6:
            if (arg != NULL) {
                vramCoursePointXZ(((PlPlace *) arg)->zone, ((PlPlace *) arg)->place, 0.0f, &x, &z);
                x -= w->pol->pos[0];
                z -= w->pol->pos[2];
                y = ((PlPlace *) arg)->height - w->pol->pos[1];
                mathVecRotateY(-(vramCourseHeading(((PlPlace *) arg)->zone, ((PlPlace *) arg)->place) - 1.570796371f),
                               &x, &y, &z);
                ((PlPlace *) arg)->place = ((PlPlace *) arg)->place - y * w->spin;
                ((PlPlace *) arg)->height = ((PlPlace *) arg)->height + x * w->spin;
            }
            break;
        case 0:
            w->pol->rot[1] = w->rot[1];
            w->pol->rot[0] += w->spin;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "wboard": a board swinging on its axle, with two kind 0xA6 enemies at its ends. */
typedef struct WboardObj {
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
    /* 0x1C */ f32 rot[3]; /* [0] its swing */
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ f32 swing; /* the swing's step each frame (radians) */
    /* 0x40 */ s32 timer; /* frames to its next creak */
} WboardObj;

extern Gfx wboardDl[];
extern u8 wboardRide[];

s32 wboardProc(Obj *obj, s16 msg, void *arg) {
    WboardObj *w = (WboardObj *) FX(obj);
    s32 pad;
    s32 data;
    f32 x;
    f32 y;
    f32 z;
    s32 pad2[2];

    switch (msg) {
        case 2:
            obj->name = "wboard";
            data = eneSetup(obj, (EneArg *) arg);
            if (data & 0xFF) {
                w->swing = -0.01745329238f;
            } else {
                w->swing = 0.01745329238f;
            }
            w->rot[1] = (f32) (data >> 16) * 3.141592741f / 512;
            if ((w->pol = eneNewPol(obj, wboardDl)) == NULL) {
                return -1;
            }
            if ((w->ride = rideCreate(gRideTree1, (s32) obj, &w->pol->f,
                                      (RideData *) eneFileAddr(obj, wboardRide, 0))) == NULL) {
                return -1;
            }
            mathXZRotateY(w->rot[1], 0.0f, 45.0f, &x, &z);
            eneMakeEx(obj, ENE_WCYLINDER, 0, w->pol->pos[0] + x, w->pol->pos[1], w->pol->pos[2] + z, 2,
                      (u32 (*)(void *, void *, void *)) data);
            eneMakeEx(obj, ENE_WCYLINDER, 0, w->pol->pos[0] - x, w->pol->pos[1], w->pol->pos[2] - z, 2,
                      (u32 (*)(void *, void *, void *)) data);
            break;
        case 1:
            frmDestroy(&w->ride->f);
            frmDestroy(&w->pol->f);
            break;
        case 7:
            if (arg != NULL) {
                *(s32 *) arg = 3;
            }
            break;
        case 6:
            if (arg != NULL) {
                ((PlPlace *) arg)->place -= 45.0f * w->swing * mathCos(w->rot[0]);
                ((PlPlace *) arg)->height -= 45.0f * w->swing * mathSin(w->rot[0]);
            }
            break;
        case 0:
            w->timer++;
            if (w->timer >= 28) {
                w->timer -= 27;
                if (sndGetVolume(0x2EA, 0) != 0) {
                    soundIncamCam(0x2EA, w->x, w->y, w->z);
                } else {
                    soundIncamCam(0x2EB, w->x, w->y, w->z);
                }
            }
            w->rot[0] += w->swing;
            mathCos(w->rot[0]);
            mathXZRotateY(w->rot[1], 0.0f, sinf(w->rot[0]) * 45.0f, &x, &z);
            w->pol->rot[1] = w->rot[1];
            w->pol->pos[0] = w->x + x;
            w->pol->pos[2] = w->z + z;
            w->pol->pos[1] = cosf(w->rot[0]) * 45.0f + w->y;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "fusha": a water wheel, two models (the second turned half round) the player can ride. */
typedef struct FushaObj {
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
    /* 0x34 */ PolFrame *pol[2];
    /* 0x3C */ RideFrame *ride[2];
    /* 0x44 */ f32 spin; /* its turn each frame (radians) */
} FushaObj;

extern Frame *gFrmLayer3;
extern Gfx fushaType1Dl[];
extern Gfx fushaType0Dl[];
extern u8 fushaRide[];

/* "fusha": two models, the second turned half a turn, each a ride, spinning by spin each frame
 * (eneSetup's 0 one way, else the other); a rider is carried by its distance times the spin.
 * Message 7 answers 3.
 * Matched 2026-10-11; it was parked for its compiler temps' stack slots (the pad below z). */
s32 fushaProc(Obj *obj, s16 msg, void *arg) {
    FushaObj *w = (FushaObj *) FX(obj);
    s32 pad[2];
    Gfx *dl;
    f32 x;
    f32 z;
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "fusha";
            if (eneSetup(obj, (EneArg *) arg) == 0) {
                dl = fushaType0Dl;
                w->spin = 0.008726646192f;
            } else {
                dl = fushaType1Dl;
                w->spin = -0.008726646192f;
            }
            w->pol[0] = eneNewPol(obj, dl);
            w->pol[1] = eneNewPol(obj, dl);
            if (w->pol[0] == NULL || w->pol[1] == NULL) {
                return -1;
            }
            frmMove(&w->pol[0]->f, gFrmLayer3);
            frmMove(&w->pol[1]->f, gFrmLayer3);
            w->ride[0] = rideCreate(gRideTree1, (s32) obj, &w->pol[0]->f,
                                    (RideData *) eneFileAddr(obj, fushaRide, 0));
            w->ride[1] = rideCreate(gRideTree1, (s32) obj, &w->pol[1]->f,
                                    (RideData *) eneFileAddr(obj, fushaRide, 0));
            if (w->ride[0] == NULL || w->ride[1] == NULL) {
                return -1;
            }
            break;
        case 1:
            frmDestroy(&w->ride[0]->f);
            frmDestroy(&w->ride[1]->f);
            frmDestroy(&w->pol[0]->f);
            frmDestroy(&w->pol[1]->f);
            break;
        case 7:
            if (arg != NULL) {
                *(s32 *) arg = 3;
            }
            break;
        case 6:
            if (arg != NULL) {
                vramCoursePointXZ(((PlPlace *) arg)->zone, ((PlPlace *) arg)->place, 0.0f, &x, &z);
                ((PlPlace *) arg)->place += sqrtf((w->x - x) * (w->x - x) + (w->z - z) * (w->z - z)) * w->spin;
            }
            break;
        case 0:
            w->rot[1] += w->spin;
            eneSetModelOnCourse(obj, &w->pol[0]->f);
            eneSetModelOnCourse(obj, &w->pol[1]->f);
            w->pol[1]->rot[1] += 3.141592741f;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
