#include "tlb.h"

/* hotaru: fireflies; hmaster sends them out at night near the first player in play. */

extern Gfx hotaruGlowDl[];

/* A firefly's work, at its task + 0x20: an enemy's head (EneObj) up to 0x34. */
typedef struct HotaruWork {
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
    /* 0x34 */ PolFrame *glow;
    /* 0x38 */ PolFrame *body;
    /* 0x3C */ f32 phase;
    /* 0x40 */ s16 bright;
    /* 0x42 */ s16 life;
    /* 0x44 */ s16 time; /* steps in the move */
    /* 0x46 */ s16 limit; /* the move's length */
    /* 0x48 */ u8 move; /* 0 waiting, 1 rising, 2 choosing a turn, 3 and 4 turning */
} HotaruWork;

/* hmaster's work, at its task + 0x20. */
typedef struct HmasterWork {
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
    /* 0x34 */ u32 count; /* steps since the last firefly */
} HmasterWork;

s32 hotaruMove(HotaruWork *w);

/* Puts a new firefly 5 above the ground under it; 0 if there is no ground there, or it is in
 * water. */
s32 hotaruPutOnGround(Obj *o) {
    HotaruWork *w = (HotaruWork *) FX(o);
    s32 pad;
    Vec3f from;
    Vec3f dir;
    Vec3f at;
    f32 t;

    vramCoursePointXZ(w->zone, w->x, w->z, &from.x, &from.z);
    from.y = w->y;
    dir.x = 0.0f;
    dir.z = 0.0f;
    dir.y = -100.0f;
    if (vramRayGround(&from, &dir, &at, &t) != NULL) {
        from.y = at.y;
        if (rideTree2PointIn(&from, &dir) == 0) {
            w->y = from.y + 5.0f;
            return 1;
        }
    }
    return 0;
}

s32 hotaruProc(Obj *o, s16 msg, void *arg) {
    HotaruWork *w = (HotaruWork *) FX(o);
    s32 pad;
    s32 data;

    switch (msg) {
        case 2:
            o->name = "hotaru";
            data = eneSetup(o, arg);
            w->glow = eneNewPol(o, hotaruGlowDl);
            if (w->glow == NULL) {
                return -1;
            }
            frmMove((Frame *) w->glow, gFrmLayer8);
            w->glow->flags |= 7;
            w->glow->scale[0] = w->glow->scale[1] = w->glow->scale[2] = 0.2f;
            w->body = eneNewPol(o, hotaruGlowDl);
            if (w->body == NULL) {
                return -1;
            }
            frmMove((Frame *) w->body, gFrmLayer8);
            w->body->flags |= 7;
            w->body->scale[0] = w->body->scale[1] = w->body->scale[2] = 0.08f;
            w->phase = -1.5707964f;
            w->life = data;
            w->limit = fadeRand(w->life * 3);
            break;
        case 1:
            frmDestroy((Frame *) w->glow);
            frmDestroy((Frame *) w->body);
            break;
        case 0:
            w->time++;
            w->life--;
            if (w->life < 0 && w->bright < 4) {
                return -1;
            }
            hotaruMove(w);
            w->bright = mathSin(w->phase) * 127.9 + 128.0;
            w->phase += 0.05235988f;
            eneMove((EneObj *) w);
            eneSetModelOnCourse(o, (Frame *) w->body);
            *(Vec3f *) w->glow->pos = *(Vec3f *) w->body->pos;
            break;
        case 3:
            w->glow->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0,
                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 128, 0);
            gDPSetEnvColor(gGfxDlHead++, 255, 255, 0, w->bright / 4);
            gSPEndDisplayList(gGfxDlHead++);
            w->body->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0,
                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 255, 0);
            gDPSetEnvColor(gGfxDlHead++, 255, 255, 0, w->bright);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Whether a player in play is on the firefly's course within 30 of it and moving faster than 1. */
s32 hotaruPlayerNear(HotaruWork *w) {
    f32 place;
    f32 height;
    f32 side;
    s32 zone;
    s32 i;
    Vec3f speed;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plAskPos(gGameWork.players[i], &place, &height, &side, &zone);
            plGetSpeed(gGameWork.players[i], &speed);
            if (zone == w->zone && fabsf(place - w->x) < 30.0f && sqrtf(speed.x * speed.x + speed.y * speed.y) > 1.0) {
                return 1;
            }
        }
    }
    return 0;
}

/* A firefly's moves: it waits until its time is up or a player runs near, rises a while, then
 * circles left or right. */
s32 hotaruMove(HotaruWork *w) {
    switch (w->move) {
        case 0:
            if (w->limit < w->time || hotaruPlayerNear(w) == 1) {
                w->time = 0;
                w->move = 1;
            }
            break;
        case 1:
            w->vel[1] = 1.0f;
            if (fadeRand(10) + 5 < w->time) {
                w->time = 0;
                w->move = 2;
            }
            break;
        case 2:
            w->limit = fadeRand(20) + 5;
            w->time = 0;
            if (w->limit & 1) {
                w->move = 3;
            } else {
                w->move = 4;
            }
            break;
        case 3:
            mathVecRotateY(0.034906585f, &w->vel[1], &w->vel[2], &w->vel[0]);
            if (w->limit < w->time) {
                w->time = 0;
                w->move = 2;
            }
            break;
        case 4:
            mathVecRotateY(-0.034906585f, &w->vel[1], &w->vel[2], &w->vel[0]);
            if (w->limit < w->time) {
                w->time = 0;
                w->move = 2;
            }
            break;
    }
    return 0;
}

/* The first player in play: its course (*zone), place and height; 0 if there is none. */
s32 hotaruFirstPlayerPos(s32 *zone, f32 *place, f32 *height) {
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

/* hmaster: by day it waits for its fireflies to go and then ends; at night, while it has fewer
 * than ten, it makes another (kind 0x8F, 300 steps) near the first player in play once the
 * count passes their number squared. */
s32 hotaruMasterProc(Obj *o, s16 msg, void *arg) {
    HmasterWork *w = (HmasterWork *) FX(o);
    s32 n;
    s32 zone;
    s32 pad;
    f32 place;
    f32 height;
    s32 pad2;
    f32 pan[3];
    Obj *child;

    switch (msg) {
        case 2:
            o->name = "hmaster";
            eneSetup(o, arg);
            break;
        case 1:
            break;
        case 0:
            w->count++;
            n = tskCountChildren(o);
            if (gGameWork.time >= 0x30000 && gGameWork.time < 0x140000) {
                if (n == 0) {
                    eneReleaseSpawn(w->spawn);
                    eneClearSpawnWait(w->spawn);
                    return -1;
                }
            } else if (n < 10 && n * n < w->count) {
                if (hotaruFirstPlayerPos(&zone, &place, &height)) {
                    camAiGetPan(pan);
                    child = eneMakeEx(o, ENE_HOTARU, zone, pan[0] * 5.0f + (fadeRand(200) + place - 100.0f), height + 20.0f,
                                      fadeRand(20) - 10, 2, (u32 (*)(void *, void *, void *)) 300);
                    if (child != NULL && hotaruPutOnGround(child) == 0) {
                        tskDestroy(child);
                    }
                    w->count = 0;
                }
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
