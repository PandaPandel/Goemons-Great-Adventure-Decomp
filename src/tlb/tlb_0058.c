#include "tlb.h"

/* "ndram": a turning drum to ride, with eight spikes round it. */

/* acollAtkAddYZ / acollAtkAddXYZ return their box (main declares them void). */
typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy,
                                     s16 oz, s32 a);

extern Frame *gFrmLayer3;
extern Gfx ndramDl[];
extern u8 ndramRide[];
Vec3f ndramSpikePos[8] = { { 3e+01f, 283.0f, 76.0f }, { 3e+01f, -76.0f, 283.0f }, { 3e+01f, -283.0f, -76.0f }, { 3e+01f, 76.0f, -283.0f }, { -3e+01f, 283.0f, 76.0f }, { -3e+01f, -76.0f, 283.0f }, { -3e+01f, -283.0f, -76.0f }, { -3e+01f, 76.0f, -283.0f } }; /* the spikes, from the drum's middle */

/* The drum's work ("ndram"). */
typedef struct NdramWork {
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
    /* 0x3C */ ACollFrame *box; /* the spikes' root */
    /* 0x40 */ f32 spin;        /* its turn per step: a degree, either way */
} NdramWork;

/* Message 6's argument: a rider's place (on its course). */
typedef struct RideTouch {
    /* 0x00 */ s16 zone;
    /* 0x04 */ f32 x; /* along the course */
    /* 0x08 */ f32 z; /* across it */
    /* 0x0C */ f32 y;
} RideTouch;

/* "ndram": its kind's value 0 turns it one way, else the other; a rider is carried round. */
s32 ndramProc(Obj *o, s16 msg, void *arg) {
    NdramWork *w = (NdramWork *) FX(o);
    s32 i;
    ACollFrame *box;
    s32 data;
    s32 pad; /* the game's frame word */
    f32 dx;
    f32 dy;
    f32 dz;
    Obj *by;

    switch (msg) {
        case 2:
            o->name = "ndram";
            data = eneSetup(o, arg);
            w->pol = eneNewPol(o, ndramDl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayer3);
            w->pol->rot[1] = vramCourseHeading(w->zone, w->x) - 3.141592741f;
            w->ride = rideCreate(gRideTree1, (s32) o, &w->pol->f, (RideData *) eneFileAddr(o, ndramRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            w->box = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->pol->f, 1, 0, 0, 0);
            if (w->box == NULL) {
                return -1;
            }
            for (i = 0; i < 8; i++) {
                box = ((AtkAddXYZFunc) acollAtkAddXYZ)(w->box, &w->pol->f, 25, ndramSpikePos[i].x,
                                                        ndramSpikePos[i].y, ndramSpikePos[i].z, 0);
                if (box != NULL) {
                    box->kind = 120;
                }
            }
            if (data == 0) {
                w->spin = 0.01745329238f;
            } else {
                w->spin = -0.01745329238f;
            }
            break;
        case 1:
            frmDestroy(&w->ride->f);
            frmDestroy(&w->box->frame);
            frmDestroy(&w->pol->f);
            break;
        case 6:
            if (arg != NULL) {
                vramCoursePointXZ(((RideTouch *) arg)->zone, ((RideTouch *) arg)->x, 0.0f, &dx, &dz);
                dx = dx - w->pol->pos[0];
                dz = dz - w->pol->pos[2];
                dy = ((RideTouch *) arg)->y - w->pol->pos[1];
                mathVecRotateY(-(vramCourseHeading(((RideTouch *) arg)->zone, ((RideTouch *) arg)->x) - 1.570796371f),
                               &dx, &dy, &dz);
                ((RideTouch *) arg)->x = ((RideTouch *) arg)->x - dy * w->spin;
                ((RideTouch *) arg)->y = ((RideTouch *) arg)->y + dx * w->spin;
            }
            break;
        case 0:
            w->pol->rot[0] += w->spin;
            for (box = (ACollFrame *) w->box->frame.child; box != NULL; box = (ACollFrame *) box->frame.next) {
                if (acollAtkWasHit(box) == 1) {
                    by = (Obj *) box->hitPower;
                    if (by != NULL) {
                        tskSendMsg(by, 7, o);
                    }
                }
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
