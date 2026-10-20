#include "tlb.h"

/* "buranko": a swing, a seat on ropes that swings along the course and carries the player. Its
 * value from eneSetup: the period in frames (low 16 bits) and which quarter of it it starts in
 * (bits 16-17). */

/* An enemy's work (EneObj) to 0x34, then its own. */
typedef struct BurankoWork {
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
    /* 0x34 */ PolFrame *seat; /* swings */
    /* 0x38 */ PolFrame *frame; /* stays */
    /* 0x3C */ RideFrame *ride;
    /* 0x40 */ f32 speed; /* 2 pi / period */
    /* 0x44 */ s32 time;
    /* 0x48 */ s32 period;
} BurankoWork;

/* Message 6's argument (plmove.c's PlPlace): a place on the course, moved by the object. */
typedef struct PlPlace {
    /* 0x0 */ s16 zone;
    /* 0x4 */ f32 place;
    /* 0x8 */ f32 side;
    /* 0xC */ f32 height;
} PlPlace;

/* libultra's, through their addresses (code at 0x08000000 cannot jal them). */
#define sinf (*sinf)
#define cosf (*cosf)

extern Gfx burankoSeatDl[];
extern Gfx burankoDl[];
extern u8 burankoRide[];

s32 burankoProc(Obj *obj, s16 msg, void *arg) {
    BurankoWork *w = (BurankoWork *) FX(obj);
    s32 pad;
    s32 v;
    f32 r;
    f32 k;
    f32 dx;
    f32 dy;
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "buranko";
            v = eneSetup(obj, arg);
            w->seat = eneNewPol(obj, burankoSeatDl);
            if (w->seat == NULL) {
                return -1;
            }
            w->seat->rot[1] = vramCourseHeading(w->zone, w->x) - 3.141592741f;
            w->frame = eneNewPol(obj, burankoDl);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->rot[1] = w->seat->rot[1];
            w->ride = rideCreate(gRideTree1, (s32) obj, &w->seat->f,
                                 (RideData *) eneFileAddr(obj, burankoRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            w->period = v & 0xFFFF;
            w->time = gGameFrameCount;
            w->time += w->period / 4 * ((v >> 16) & 3);
            w->speed = 6.283185482f / w->period;
            break;
        case 1:
            frmDestroy(&w->ride->f);
            frmDestroy(&w->seat->f);
            frmDestroy(&w->frame->f);
            break;
        case 7:
            if (arg != NULL) {
                *(s32 *) arg = 3;
            }
            break;
        case 6:
            dx = ((PlPlace *) arg)->place - w->x;
            dy = ((PlPlace *) arg)->height - w->y;
            r = sqrtf(dx * dx + dy * dy);
            k = cosf(w->speed * (w->time - 1)) * (r * 0.3926990926f * w->speed);
            ((PlPlace *) arg)->place -= k * (dy / r);
            ((PlPlace *) arg)->height += k * (dx / r);
            break;
        case 0:
            w->seat->rot[0] = sinf(w->speed * w->time) * 0.3926990926f;
            w->time++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
