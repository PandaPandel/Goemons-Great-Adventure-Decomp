#include "tlb.h"

extern u8 idashiDl[];
extern u8 idashiRide[];

/* An "idashi" platform's work (task + 0x20): an enemy's (EneObj) up to 0x34. */
typedef struct IdashiObj {
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
    /* 0x34 */ PolFrame *frame;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ f32 base;  /* where it starts */
    /* 0x40 */ f32 range; /* how far it goes (eneSetup's data, low half) */
    /* 0x44 */ f32 ofs;   /* how far it is (the data's high half at first) */
    /* 0x48 */ u8 move;   /* 0 going out, 1 waiting there, 2 coming back, 3 waiting */
    /* 0x4A */ s16 timer; /* steps in this move */
} IdashiObj;

/* "idashi": a platform (a ride) that rises by its range at half a unit a step, waits 31 steps,
 * sinks back and waits again; message 6 moves what stands on it (arg) with it. */
s32 idashiProc(Obj *obj, s16 msg, void *arg) {
    IdashiObj *w = (IdashiObj *) ((u32) obj + sizeof(Obj));
    s32 pad;
    s32 data;
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "idashi";
            data = eneSetup(obj, arg);
            w->frame = eneNewPol(obj, (Gfx *) idashiDl);
            if (w->frame == NULL) {
                return -1;
            }
            w->ride = rideCreate(gRideTree1, (s32) obj, &w->frame->f,
                                 (RideData *) eneFileAddr(obj, idashiRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            w->rot[1] = vramCourseHeading(w->zone, w->x) - PI / 2;
            w->base = w->y;
            w->range = data & 0xFFFF;
            w->ofs = data >> 16;
            break;
        case 1:
            frmDestroy(&w->ride->f);
            frmDestroy(&w->frame->f);
            break;
        case 6:
            if (arg != NULL) {
                switch (w->move) {
                    case 0:
                        ((f32 *) arg)[3] += 0.5f;
                        break;
                    case 2:
                        ((f32 *) arg)[3] -= 0.5f;
                        break;
                }
            }
            break;
        case 0:
            w->timer++;
            switch (w->move) {
                case 0:
                    w->ofs += 0.5f;
                    if (w->ofs >= w->range) {
                        w->timer = 0;
                        w->move = 1;
                    }
                    break;
                case 1:
                    if (w->timer >= 31) {
                        w->move = 2;
                    }
                    break;
                case 2:
                    w->ofs -= 0.5f;
                    if (w->ofs <= 0.0f) {
                        w->timer = 0;
                        w->move = 3;
                    }
                    break;
                case 3:
                    if (w->timer >= 31) {
                        w->move = 0;
                    }
                    break;
            }
            w->y = w->base + w->ofs;
            eneSetModelOnCourse(obj, &w->frame->f);
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "idashi" along the course: the same, moving out along x. */
s32 idashiSlideProc(Obj *obj, s16 msg, void *arg) {
    IdashiObj *w = (IdashiObj *) ((u32) obj + sizeof(Obj));
    s32 pad;
    s32 data;
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "idashi";
            data = eneSetup(obj, arg);
            w->frame = eneNewPol(obj, (Gfx *) idashiDl);
            if (w->frame == NULL) {
                return -1;
            }
            w->ride = rideCreate(gRideTree1, (s32) obj, &w->frame->f,
                                 (RideData *) eneFileAddr(obj, idashiRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            w->rot[1] = vramCourseHeading(w->zone, w->x) - PI / 2;
            w->base = w->x;
            w->range = data & 0xFFFF;
            w->ofs = data >> 16;
            break;
        case 1:
            frmDestroy(&w->ride->f);
            frmDestroy(&w->frame->f);
            break;
        case 6:
            if (arg != NULL) {
                switch (w->move) {
                    case 0:
                        ((f32 *) arg)[1] += 0.5f;
                        break;
                    case 2:
                        ((f32 *) arg)[1] -= 0.5f;
                        break;
                }
            }
            break;
        case 0:
            w->timer++;
            switch (w->move) {
                case 0:
                    w->ofs += 0.5f;
                    if (w->ofs >= w->range) {
                        w->timer = 0;
                        w->move = 1;
                    }
                    break;
                case 1:
                    if (w->timer >= 31) {
                        w->move = 2;
                    }
                    break;
                case 2:
                    w->ofs -= 0.5f;
                    if (w->ofs <= 0.0f) {
                        w->timer = 0;
                        w->move = 3;
                    }
                    break;
                case 3:
                    if (w->timer >= 31) {
                        w->move = 0;
                    }
                    break;
            }
            w->x = w->base + w->ofs;
            eneSetModelOnCourse(obj, &w->frame->f);
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
