#include "tlb.h"

/* The "saburo" enemy kind (archive file 176): a one-hit enemy built on the monoeye's helpers
 * (file 177). Type 0 follows the nearest player along the course once one comes within 80,
 * flying at it a little above, and goes home when it strays more than 200 from where it was
 * placed or leaves its course (state 3); type 1 waits for a player within 80, then dashes at it
 * once and is gone when it lands, meets a wall or its attack hits, leaving a kind 0x236 behind.
 * Their animations are tables, and the state is the animation's number. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

#pragma intrinsic(fabs)

/* An animation of a table: the state with the same number plays it. */
typedef struct SaburoAnim {
    /* 0x00 */ u32 anim;
    /* 0x04 */ f32 speed; /* at one retrace a frame */
    /* 0x08 */ u8 loop; /* 1: loops (eneAnimLoop), else plays once */
} SaburoAnim; /* size 0x0C */

/* A saburo's work, at its task + 0x20: an enemy's (EneObj) up to 0x40, then its own. */
typedef struct Saburo {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ MPolFrame *shadow;
    /* 0x40 */ f32 animSpeed; /* its animation's step at this frame rate */
    /* 0x44 */ f32 aimX; /* its player's place along the course (plAskPos) */
    /* 0x48 */ f32 aimY; /* and height */
    /* 0x4C */ f32 heading; /* type 1's dash */
    /* 0x50 */ f32 homeX; /* where it was placed, along its course (eneTeritoryCheck) */
    /* 0x54 */ u8 homeZone; /* and its course */
    /* 0x55 */ u8 type; /* its placement's type, from eneSetup */
    /* 0x56 */ u8 anim; /* its animation in the table, and its state */
    /* 0x57 */ u8 life; /* hits it takes */
    /* 0x58 */ s8 dir; /* 0: the nearest player is on its way 1, 1: on the other side */
    /* 0x59 */ s8 target; /* the nearest player (-1: none) */
    /* 0x5A */ u8 struck; /* its attack has hit something */
} Saburo;

#define SABURO(obj) ((Saburo *) FX(obj))

SaburoAnim saburoAnimTable0[] = {
    { 0x03000288, 0.0f, 0 },
    { 0x03000288, 0.0f, 0 },
    { 0x03000288, 2.0f, 1 },
    { 0x03000288, 1.0f, 1 },
    { 0x03000288, 0.0f, 0 },
    { 0x00000000, 0.0f, 0 },
}; /* type 0's animations */
SaburoAnim saburoAnimTable1[] = {
    { 0x03000288, 0.0f, 0 },
    { 0x03000288, 1.0f, 1 },
    { 0x03000288, 2.0f, 1 },
    { 0x03000288, 0.0f, 0 },
    { 0x03000288, 0.0f, 0 },
    { 0x00000000, 0.0f, 0 },
}; /* type 1's */
extern u8 saburoHalf0Dl[];
extern u8 saburoHalf1Dl[];

s32 saburoMain0(Obj *o);
s32 saburoMain1(Obj *o);

/* Sets animation n of the table: its step, scaled to the frame rate, from its first frame. */
void saburoSetAnim(Obj *o, u8 n, SaburoAnim *tab) {
    Saburo *w = SABURO(o);

    w->anim = n;
    w->animSpeed = tab[n].speed / gRetracesPerFrame;
    w->model->anim = tab[n].anim;
    w->model->frame = 0.0f;
}

void saburoNop1(void) {
}

void saburoNop2(void) {
}

void saburoNop3(void) {
}

/* Puts it back on its course and returns its heading there: mode 0 the heading less ang, 1 the
 * heading turned by ang and a half turn, -1 its own rotation, else the heading. */
f32 saburoCourseAngle(Obj *o, f32 ang, s8 mode) {
    Saburo *w = SABURO(o);
    f32 h;
    f32 x;
    f32 y;
    f32 z;
    f32 r;

    w->x = vramCoursePointXZ(w->zone, w->x, w->z, &x, &z);
    r = h = vramCourseHeading(w->zone, w->x);
    if (mode == 0) {
        r = h - ang;
    } else if (mode == 1) {
        r += ang + 3.141592741f;
    } else if (mode == -1) {
        r = w->rot[1];
    }
    return r;
}

/* The distance from it to player i (0 when i is out of range or not in play). */
f32 saburoPlDist(Obj *o, s32 i) {
    s32 pad[2];
    f32 x;
    f32 y;
    f32 z;
    Vec3f e;

    if (i < 0 || i >= 4) {
        return 0.0f;
    }
    eneWorldPos(o, &e);
    if (plIsInPlay(i)) {
        plGetModelPos(gGameWork.players[i], &x, &y, &z);
        return sqrtf((x - e.x) * (x - e.x) + (y - e.y) * (y - e.y) + (z - e.z) * (z - e.z));
    }
    return 0.0f;
}

/* The nearest player in play (-1: none). */
s32 saburoNearestPl(Obj *o) {
    s32 i;
    f32 min;
    f32 d;
    f32 x;
    f32 y;
    f32 z;
    s32 pad[2];
    s32 nearest;
    Vec3f e;

    min = 65536.0f;
    nearest = -1;
    eneWorldPos(o, &e);
    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plGetModelPos(gGameWork.players[i], &x, &y, &z);
            d = saburoPlDist(o, i);
            if (d < min) {
                min = d;
                nearest = i;
            }
        }
    }
    return nearest;
}

void saburoNop4(void) {
}

/* Speeds it up by accel toward ang (in the course's plane: x along it, y up), the y part (scale
 * below 1) or the x part (else) scaled by scale, and holds its speed to max; returns the speed. */
f32 saburoAccel(Obj *o, f32 ang, f32 accel, f32 max, f32 scale) {
    Saburo *w = SABURO(o);
    s32 pad;
    s32 pad2;
    f32 ax = mathCos(ang) * accel;
    f32 ay = mathSin(ang) * accel;
    f32 speed;
    f32 k;

    if (scale < 1.0f) {
        ay *= scale;
    } else {
        ax *= scale;
    }
    w->vel[0] += ax;
    w->vel[1] += ay;
    speed = sqrtf(w->vel[1] * w->vel[1] + w->vel[0] * w->vel[0]);
    if (max < speed) {
        k = max / speed;
        w->vel[0] *= k;
        w->vel[1] *= k;
        return max;
    }
    return speed;
}

/* "eneFallLimit": 1 once it is below -600 or above 1000. */
s32 saburoFallLimit(Obj *o) {
    Saburo *w = SABURO(o);

    if (w->y < -600.0f) {
        PRINTF("eneFallLimit(-600):\n");
        return 1;
    }
    if (1000.0f < w->y) {
        PRINTF("eneFallLimit(+1000):\n");
        return 1;
    }
    return 0;
}

/* "eneTeritoryCheck": 1 if it is on another course than *zone (which it then takes, with its
 * place in *x) or farther than range from *x along it. */
s32 saburoTeritoryCheck(Obj *o, f32 *x, u8 *zone, f32 range) {
    Saburo *w = SABURO(o);

    if (w->zone != *zone) {
        PRINTF("eneTeritoryCheck: new course\n");
        *x = w->x;
        *zone = w->zone;
        return 1;
    }
    if (fabs(w->x - *x) > range) {
        PRINTF("eneTeritoryCheck: out of range\n");
        return 1;
    }
    return 0;
}

/* "saburo": the kind's proc. */
s32 saburoProc(Obj *o, s16 msg, void *arg) {
    Saburo *w = SABURO(o);
    s32 pad;
    s32 r;

    switch (msg) {
        case 2:
            o->name = "saburo";
            w->type = eneSetup(o, arg);
            w->life = 1;
            w->struck = 0;
            w->homeX = w->x;
            w->homeZone = w->zone;
            if ((w->model = eneNewAnime(o, 0)) == NULL) {
                return -1;
            }
            if ((w->shadow = shadowCreate()) == NULL) {
                return -1;
            }
            if ((w->atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, &w->model->f, 0, 0, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, &w->model->f, 0, 0, 0, 0, 0)) == NULL) {
                return -1;
            }
            break;
        case 0:
            if (saburoFallLimit(o)) {
                return -1;
            }
            if (acollAtkWasHit(w->atk) == 1 && (Obj *) w->atk->hitPower != NULL) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, o);
                w->struck = 1;
            }
            switch (w->type) {
                case 0:
                    r = saburoMain0(o);
                    break;
                case 1:
                    r = saburoMain1(o);
                    break;
            }
            if (r != 0) {
                return r;
            }
            break;
        case 1:
            if (w->model != NULL) { frmDestroy(&w->model->f); }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        default:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Type 0's step: moves, lands, animates, takes its hit (state 4: dies), goes home when it strays
 * (state 3), then runs its state: 0 sets up, 1 waits for a player within 80, 2 follows it. */
s32 saburoMain0(Obj *o) {
    Saburo *w = SABURO(o);
    f32 a;
    s32 land;
    s32 wall;
    s32 pad;
    s32 zone;
    f32 z;
    s32 pad2;
    s32 pad3;

    eneMove((EneObj *) w);
    land = eneLandOff((EneObj *) w, 10.0f, -10.0f);
    wall = eneWallCheckNoPush(o, 10.0f, 7.0f, 0.0f, -10.0f);
    eneSetModelOnCourse(o, &w->model->f);
    if (saburoAnimTable0[w->anim].loop == 1) {
        eneAnimLoop(o, w->model, w->animSpeed);
    } else {
        eneAnimPlayOnce(o, w->model, w->animSpeed);
    }
    if (w->shadow != NULL) {
        shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.299999952f, NULL);
    }
    if (acollDmgWasHit(w->dmg) == 1) {
        if (--w->life == 0) {
            w->dmg->flags &= ~2;
            w->atk->flags &= ~2;
            saburoSetAnim(o, 4, saburoAnimTable0);
        }
    }
    if (w->anim != 3 && saburoTeritoryCheck(o, &w->homeX, &w->homeZone, 200.0f)) {
        w->vel[0] = w->vel[1] = 0.0f;
        saburoSetAnim(o, 3, saburoAnimTable0);
    }
    switch (w->anim) {
        case 0:
            w->atk->radius = 70;
            w->atk->kind = 120;
            w->dmg->radius = 75;
            w->dmg->height = 200;
            w->dmg->ofs[1] = -100;
            w->model->scale[2] = 0.07999999821f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            if (eneNearestPlWay(o) == 1) {
                w->dir = 0;
            } else {
                w->dir = 1;
            }
            w->rot[1] = saburoCourseAngle(o, 0.87266463f, w->dir);
            saburoSetAnim(o, 1, saburoAnimTable0);
            break;
        case 1:
            w->target = saburoNearestPl(o);
            if (w->target >= 0 && saburoPlDist(o, w->target) < 80.0f) {
                saburoSetAnim(o, 2, saburoAnimTable0);
            }
            break;
        case 2:
            w->target = saburoNearestPl(o);
            if (w->target >= 0) {
                if (eneNearestPlWay(o) == 1) {
                    w->dir = 0;
                } else {
                    w->dir = 1;
                }
                eneTurnToward(o, saburoCourseAngle(o, 0.87266463f, w->dir), gRetracesPerFrame * 0.06981316954f);
                plAskPos(gGameWork.players[w->target], &w->aimX, &w->aimY, &z, &zone);
                saburoAccel(o, mathAtan2(w->aimY - w->y + 30.0f, w->aimX - w->x), 0.1f, (f32) gRetracesPerFrame, 0.5f);
                if (land) {
                    w->vel[1] *= -1.0f;
                }
                if (wall == -1) {
                    w->vel[0] *= -1.0f;
                }
                if (fabs(w->vel[0]) < 0.5) {
                    w->rot[1] += (w->dir == 0) ? 1.04719758f : -1.04719758f;
                }
                w->animSpeed = (1.0 - fabs(w->vel[0]) / (f32) gRetracesPerFrame) * (0.8000000119f * gRetracesPerFrame) + 0.5f * gRetracesPerFrame;
            }
            break;
        case 3:
            if (land) {
                w->vel[1] = 0;
            }
            if (w->x < w->homeX) {
                w->dir = 0;
            } else {
                w->dir = 1;
            }
            eneTurnToward(o, saburoCourseAngle(o, 0.87266463f, w->dir), gRetracesPerFrame * 0.06981316954f);
            if (w->dir == 0) {
                w->vel[0] = gRetracesPerFrame * 0.5;
            } else if (w->dir == 1) {
                w->vel[0] = gRetracesPerFrame * -0.5;
            }
            if (fabs(w->x - w->homeX) <= 0.5) {
                w->vel[0] = 0;
                saburoSetAnim(o, 1, saburoAnimTable0);
            }
            break;
        case 4:
            shimEneDeathAct(o, w->dmg, (u32) saburoHalf0Dl, (u32) saburoHalf1Dl);
            eneReleaseSpawn(w->spawn);
            PRINTF("*** saburo(type %d):dead ***\n", w->type);
            return -1;
    }
    return 0;
}

/* Type 1's step: as type 0's without the homing; it waits for a player within 80 (state 1),
 * aims at it, dashes (state 2) and is gone (state 4) when it lands, meets a wall or its attack
 * hits, leaving a kind 0x236 behind; its one hit kills it (state 3). */
s32 saburoMain1(Obj *o) {
    Saburo *w = SABURO(o);
    f32 a;
    s32 land;
    s32 wall;
    s32 pad;
    s32 zone;
    f32 z;

    eneMove((EneObj *) w);
    land = eneLandOff((EneObj *) w, 10.0f, -10.0f);
    wall = eneWallCheckNoPush(o, 10.0f, 7.0f, 0.0f, -10.0f);
    eneSetModelOnCourse(o, &w->model->f);
    if (saburoAnimTable1[w->anim].loop == 1) {
        eneAnimLoop(o, w->model, w->animSpeed);
    } else {
        eneAnimPlayOnce(o, w->model, w->animSpeed);
    }
    if (w->shadow != NULL) {
        shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.299999952f, NULL);
    }
    if (acollDmgWasHit(w->dmg) == 1) {
        if (--w->life == 0) {
            w->dmg->flags &= ~2;
            w->atk->flags &= ~2;
            saburoSetAnim(o, 3, saburoAnimTable1);
        }
    }
    switch (w->anim) {
        case 0:
            w->atk->radius = 70;
            w->atk->kind = 120;
            w->dmg->radius = 75;
            w->dmg->height = 200;
            w->dmg->ofs[1] = -100;
            w->model->scale[2] = 0.07999999821f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            if (eneNearestPlWay(o) == 1) {
                w->dir = 0;
            } else {
                w->dir = 1;
            }
            w->rot[1] = saburoCourseAngle(o, 0.87266463f, w->dir);
            saburoSetAnim(o, 1, saburoAnimTable1);
            break;
        case 1:
            w->target = saburoNearestPl(o);
            if (w->target >= 0 && saburoPlDist(o, w->target) < 80.0f) {
                if (eneNearestPlWay(o) == 1) {
                    w->dir = 0;
                } else {
                    w->dir = 1;
                }
                eneTurnToward(o, saburoCourseAngle(o, 0.87266463f, w->dir), gRetracesPerFrame * 0.06981316954f);
                plAskPos(gGameWork.players[w->target], &w->aimX, &w->aimY, &z, &zone);
                a = mathAtan2(w->aimY - w->y, ((w->dir != 0) ? 10.0f : -10.0f) + (w->aimX - w->x));
                if (1.570796371f < a && a < 4.712388992f) {
                    w->rot[0] = a - 3.141592741f;
                } else {
                    w->rot[0] = -a;
                }
                w->heading = a;
                saburoSetAnim(o, 2, saburoAnimTable1);
            }
            break;
        case 2:
            if (land || wall == -1 || w->struck) {
                w->dmg->flags &= ~2;
                w->atk->flags &= ~2;
                saburoSetAnim(o, 4, saburoAnimTable1);
                eneMakeEx(NULL, ENE_FIREBALL, w->zone, w->x, w->y - 5.0f, w->z, 2, NULL);
                break;
            }
            saburoAccel(o, w->heading, 0.15f, gRetracesPerFrame * 2.0f, 1.0f);
            w->animSpeed = (1.0 - fabs(w->vel[0]) / (gRetracesPerFrame * 2.0f)) * (0.8000000119f * gRetracesPerFrame) + 0.5f * gRetracesPerFrame;
            break;
        case 3:
            shimEneDeathAct(o, w->dmg, (u32) saburoHalf0Dl, (u32) saburoHalf1Dl);
        case 4:
            eneReleaseSpawn(w->spawn);
            PRINTF("*** saburo(type %d):dead ***\n", w->type);
            return -1;
    }
    return 0;
}
