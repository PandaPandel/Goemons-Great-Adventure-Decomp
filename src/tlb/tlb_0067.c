#include "tlb.h"

/* happa: falling leaves. "happa master" drops one every 16 steps over the first player in play;
 * each (its task named "hotaru", as tlb_0066's fireflies) swings down like a pendulum until it
 * lands or 120 steps pass, then fades out. */

#define sinf (*sinf)
#define cosf (*cosf)

extern Gfx happaDl[];

/* A leaf's work, at its task + 0x20: an enemy's head (EneObj) up to 0x34. */
typedef struct HappaWork {
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
    /* 0x34 */ s32 (*step)(Obj *, struct HappaWork *); /* falling, then fading */
    /* 0x38 */ PolFrame *pol;
    /* 0x3C */ f32 pivot[3]; /* the swing's centre */
    /* 0x48 */ f32 angle; /* the swing's phase */
    /* 0x4C */ f32 speed; /* its step */
    /* 0x50 */ f32 radius;
    /* 0x54 */ s32 alpha;
    /* 0x58 */ u16 time;
    /* 0x5A */ u8 mode; /* 0 choosing a side, 1 and 2 starting on the right or left, 3 swinging */
} HappaWork;

/* happa master's work, at its task + 0x20. */
typedef struct HappaMasterWork {
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
    /* 0x34 */ s32 count;
} HappaMasterWork;

s32 happaFall(Obj *o, HappaWork *w);
s32 happaFade(Obj *o, HappaWork *w);

s32 happaProc(Obj *o, s16 msg, void *arg) {
    HappaWork *w = (HappaWork *) FX(o);

    switch (msg) {
        case 2:
            o->name = "hotaru";
            eneSetup(o, arg);
            w->pol = eneNewPol(o, happaDl);
            if (w->pol == NULL) {
                return -1;
            }
            w->step = happaFall;
            w->alpha = 0xFF;
            break;
        case 1:
            frmDestroy((Frame *) w->pol);
            break;
        case 0:
            w->time++;
            if (w->step(o, w)) {
                return -1;
            }
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombineLERP(gGfxDlHead++, SHADE, 0, TEXEL0, 0, TEXEL0, 0, ENVIRONMENT, 0, SHADE, 0, TEXEL0, 0, TEXEL0,
                              0, ENVIRONMENT, 0);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Falling: a pendulum swing about a centre that sinks a unit a step, the leaf tipping with it,
 * until the ground (a ray from where it was) stops it or 120 steps pass. */
s32 happaFall(Obj *o, HappaWork *w) {
    s32 r;
    Vec3f from;
    Vec3f dir;
    Vec3f at;
    f32 t;

    switch (w->mode) {
        case 0:
            r = fadeRand(20) + 20;
            w->radius = r;
            w->speed = 6.2831855f / (r * 2 + 40);
            if (r & 1) {
                w->mode = 1;
            } else {
                w->mode = 2;
            }
            break;
        case 1:
            w->pivot[0] = w->x - cosf(-0.5235988f) * w->radius;
            w->pivot[1] = w->y - sinf(-0.5235988f) * w->radius;
            w->pivot[2] = w->z;
            w->angle = 1.5707964f;
            w->mode = 3;
            break;
        case 2:
            w->pivot[0] = w->x - cosf(-2.6179938f) * w->radius;
            w->pivot[1] = w->y - sinf(-2.6179938f) * w->radius;
            w->pivot[2] = w->z;
            w->angle = -1.5707964f;
            w->mode = 3;
            break;
        case 3:
            w->pivot[1] -= 1.0f;
            w->angle += w->speed;
            w->rot[2] = sinf(w->angle) * 1.0471976f - 1.5707964f;
            w->x = w->pivot[0] + cosf(w->rot[2]) * w->radius;
            w->y = w->pivot[1] + sinf(w->rot[2]) * w->radius;
            w->rot[2] += 0.7853982f;
            break;
    }
    w->rot[0] += 0.06981317f;
    w->rot[1] = vramCourseHeading(w->zone, w->x) - 1.5707964f;
    from.x = w->pol->pos[0];
    from.y = w->pol->pos[1] - 2.0f;
    from.z = w->pol->pos[2];
    eneSetModelOnCourse(o, (Frame *) w->pol);
    dir.x = w->pol->pos[0] - from.x;
    dir.y = w->pol->pos[1] - from.y - 2.0f;
    dir.z = w->pol->pos[2] - from.z;
    if (vramRayGround(&from, &dir, &at, &t) != NULL) {
        w->pol->pos[0] = at.x;
        w->pol->pos[1] = at.y + 2.0f;
        w->pol->pos[2] = at.z;
        w->time = 0;
        w->step = happaFade;
        frmMove((Frame *) w->pol, gFrmLayer8);
    }
    if (w->time > 120) {
        w->time = 0;
        w->step = happaFade;
        frmMove((Frame *) w->pol, gFrmLayer8);
    }
    return 0;
}

/* Landed: after 20 steps it fades out, 8 a step; 1 when it is gone. */
s32 happaFade(Obj *o, HappaWork *w) {
    if (w->time > 20) {
        w->alpha -= 8;
        if (w->alpha < 0) {
            return 1;
        }
    }
    return 0;
}

/* The first player in play: its course (*zone), place and height; 0 if there is none. */
s32 happaFirstPlayerPos(s32 *zone, f32 *place, f32 *height) {
    s32 i;
    f32 side;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plAskPos(gGameWork.players[i], place, height, &side, zone);
            return 1;
        }
    }
    return 0;
}

/* happa master: every 16 steps, a leaf (kind 0x91) 100 above the first player in play, ahead of
 * it by the camera's pan, if that is on the course. */
s32 happaMasterProc(Obj *o, s16 msg, void *arg) {
    HappaMasterWork *w = (HappaMasterWork *) FX(o);
    s32 pad[2];
    s32 zone;
    f32 place;
    f32 height;
    f32 z;
    f32 pan[3];

    switch (msg) {
        case 2:
            o->name = "happa master";
            eneSetup(o, arg);
            break;
        case 1:
            break;
        case 0:
            w->count++;
            if ((w->count & 0xF) == 1 && happaFirstPlayerPos(&zone, &place, &height)) {
                camAiGetPan(pan);
                place += (fadeRand(200) - 100) + pan[0] * 5.0f;
                height += 100.0f;
                z = fadeRand(40) - 20;
                if (place < 0.0f) {
                    break;
                }
                if (vramGetCourseLength(zone) < place) {
                    break;
                }
                eneMakeEx(o, ENE_HAPPA, zone, place, height, z, 2, NULL);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
