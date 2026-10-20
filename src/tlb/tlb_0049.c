#include "tlb.h"

/* "suisha": a water wheel that turns (one way or the other, by its value from eneSetup), carries
 * the player round on it and splashes now and then. */

/* An enemy's work (EneObj) to 0x34, then its own. */
typedef struct SuishaWork {
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
    /* 0x3C */ f32 spin; /* a step's turn */
    /* 0x40 */ s32 timer; /* to the next splash sound */
} SuishaWork;

/* Message 6's argument (plmove.c's PlPlace): a place on the course, moved by the object. */
typedef struct PlPlace {
    /* 0x0 */ s16 zone;
    /* 0x4 */ f32 place;
    /* 0x8 */ f32 side;
    /* 0xC */ f32 height;
} PlPlace;

extern Gfx suishaDl[];
extern u8 suishaRide[];

s32 suishaProc(Obj *obj, s16 msg, void *arg) {
    SuishaWork *w = (SuishaWork *) FX(obj);
    s32 pad;
    s32 v;
    f32 x;
    f32 y;
    f32 z;
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "suisha";
            v = eneSetup(obj, arg);
            w->pol = eneNewPol(obj, suishaDl);
            if (w->pol == NULL) {
                return -1;
            }
            w->pol->rot[1] = vramCourseHeading(w->zone, w->x) - 3.141592741f;
            w->pol->scale[2] = w->pol->scale[1] = 0.07999999821f;
            w->ride = rideCreate(gRideTree1, (s32) obj, &w->pol->f,
                                 (RideData *) eneFileAddr(obj, suishaRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            if (v != 0) {
                w->spin = 0.007853982039f;
            } else {
                w->spin = -0.007853982039f;
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
            vramCoursePointXZ(((PlPlace *) arg)->zone, ((PlPlace *) arg)->place, 0.0f, &x, &z);
            x -= w->pol->pos[0];
            z -= w->pol->pos[2];
            y = ((PlPlace *) arg)->height - w->pol->pos[1];
            mathVecRotateY(-(vramCourseHeading(((PlPlace *) arg)->zone, ((PlPlace *) arg)->place) - 1.570796371f),
                           &x, &y, &z);
            ((PlPlace *) arg)->place -= y * w->spin;
            ((PlPlace *) arg)->height += x * w->spin;
            break;
        case 0:
            w->pol->rot[0] += w->spin;
            if (w->timer >= 63) {
                if (soundSuionCreateCam(obj, 0x27A, w->pol->pos[0], w->pol->pos[1], w->pol->pos[2]) == NULL) {
                    soundSuionCreateCam(obj, 0x27B, w->pol->pos[0], w->pol->pos[1], w->pol->pos[2]);
                }
                w->timer = 0;
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
