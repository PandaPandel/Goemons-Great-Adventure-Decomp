#include "enemy.h"
#include "game.h"
#include "camera.h"
#include "effect.h"
#include "enecommon.h"
#include "eneproc.h"
#include "file.h"
#include "gfxmgr.h"
#include "itex.h"
#include "mathutil.h"
#include "plmain.h"
#include "polygon.h"
#include "shadow.h"
#include "sndloc.h"
#include "tmanager.h"
#include "vram.h"

/* The enemies' step and death routines (overlay 9): eneAfterProc (eneAfterProc, what the
 * game does with an enemy after its own step: hits, deaths, gravity, the ground, the walls,
 * its model and shadow), eneSetOnEarth, eneAtamaCheck (the ceiling), the wall and edge
 * checks, the players around it, eneSetSd2 (its sounds), eneDeathAct (how it dies: the
 * pieces), and the model makers. The debug prints are compiled out; their strings are in
 * gEneprocDebugStr. */

#pragma intrinsic(fabs)

/* Degrees to radians, as the game has it: the double 0x3F91DF46AAA2FD9A, a little over
 * pi / 180. */
#define DEG2RAD 0.0174532930038800785

/* FAKE: the enemy's velocity read as a Vec3f (vel.x, vel.y, vel.z). EneObj declares it f32
 * vel[3]; with a constant-index array element IDO orders the operands of `y + vel[1]` and of
 * the product around it the other way (IDO_NOTES 66), and the moves to a point
 * (eneMoveToPos, eneMoveToPos3D) only match with named fields. The game's struct likely has a
 * Vec3f here. */
#define VEL(e) ((Vec3f *) (e)->vel)

extern u32 gGameFrameCount; /* the frame counter */
extern Frame *gFrmLayers[]; /* the frame pools (gFrmLayer2 is [2]) */
extern Frame *gFrmModelLayer;
extern Frame *gFrmLayer2;
extern Frame *gFrmLayer8;
extern s16 gFrmKindPol; /* "pol" */
extern s16 gFrmKindAnime; /* "anime" */
extern Gfx *gGfxDlHead; /* the display-list buffer */
extern Gfx gDrawSetupEnvTintPrimShadeDl[];
extern Gfx gDrawSetupEnvTintTexDl[];
extern Gfx gDrawSetupEnvTintTexShadeDl[];
extern Gfx gDrawSetupPrimEnvTexBlendDl[];

/* Whether a hit counts this step (-1: not): not with 0x1000000, nor with 0x2000000 but on
 * every tenth frame. Both are cleared first, so it always counts (sic). */
s32 eneHitValidCheck(Obj *o) {
    EneObj *e = ENE(o);

    e->behave &= ~0x01000000;
    e->behave &= ~0x02000000;
    if (e->behave & 0x01000000) {
        return -1;
    }
    if ((e->behave & 0x02000000) && (gGameFrameCount % 10)) {
        return -1;
    }
    return 0;
}

/* eneAfterProc: what the game does with an enemy after its own step; -1 when it is gone.
 * - A hit this step (its damage box hit by another attack kind than last): hit 0x10, and unless
 *   behave 1 its hp drops by 3 (kinds 3, 6, 8, 20, 31), 2 (2, 5, 12, 16, 19, 30) or 1. Below 0
 *   it dies by that kind (eneDeathAct); else, with behave 2, the hit's sound (0x220 for 4-6 and
 *   23; 0x22E and a spark for 18-20; else 0x22C).
 * - Deaths by behave: 0x80000 by the last hit's kind, 0x200000 as kind 1, 0x4000000 a burst
 *   (sound 0x405), 0x100000 and 0x10000000 two other effects, 0x40000 at once. With 0x400000
 *   the placement is counted as beaten (eneReleaseSpawn).
 * - Hit 8 if one of its attack boxes hit (that box's target gets message 7).
 * - Gravity 0.5 (0x800000) or 0.3 (0x800), then its speed; below y -600 it is gone.
 * - It lands unless it has walls (0x7C) or 0x1000 (keeping its fall with 0x2000); ceilings
 *   stop it (0x20000000); the head check sets hit 6 (4).
 * - Walls (0x78) and edges (0x780) stop it (8, 0x20, 0x80, 0x200) or bounce it (0x10, 0x40,
 *   0x100, 0x400).
 * - It faces along its course (0x4000); its model follows it; its shadow is sized by its
 *   damage box.
 * (The shadow's scale is read as a named field, x: as the array element scale[0], IDO loads it
 * before converting the radius, which the game does first; the eneproc lesson of
 * eneAtamaCheck.) */
s32 eneAfterProc(Obj *o) {
    EneObj *e = ENE(o);
    u8 kind;
    ACollFrame *box;
    f32 size;
    Obj *target;
    s32 pad;
    CollTri *ground;

    e->hit &= ~0x10;
    if ((*acollDmgWasHit)(e->dmg) == 1) {
        kind = e->dmg->hitKind;
        PRINTF("eneAfterProc(): 攻撃くらい kind = %d\n", kind);
        if (e->hitKind != kind) {
            e->hit |= 0x10;
        }
    } else {
        kind = 0;
    }
    e->hitKind = kind;
    if (e->hit & 0x10) {
        if (!(e->behave & 1)) {
            switch (kind) {
                case 3:
                case 6:
                case 8:
                case 20:
                case 31:
                    e->hp -= 3;
                    break;
                case 2:
                case 5:
                case 12:
                case 16:
                case 19:
                case 30:
                    e->hp -= 2;
                    break;
                default:
                    e->hp -= 1;
                    break;
            }
        }
        if (e->hp < 0) {
            (*eneDeathAct)(o, e->dmg->hitKind);
            if (e->behave & 0x400000) {
                (*eneReleaseSpawn)(e->spawn);
            }
            return -1;
        }
        if (e->behave & 2) {
            switch (e->dmg->hitKind) {
                case 4:
                case 5:
                case 6:
                case 23:
                    (*eneSetSd2)(o, 0x220, 0, 0);
                    break;
                case 18:
                case 19:
                case 20:
                    (*eneHitSpark)(o, e->dmg);
                    (*eneSetSd2)(o, 0x22E, 0, 0);
                    break;
                case 14:
                case 15:
                case 21:
                    break;
                case 1:
                default:
                    (*eneSetSd2)(o, 0x22C, 0, 0);
                    break;
            }
        }
    }
    if (e->behave & 0x80000) {
        (*eneDeathAct)(o, e->dmg->hitKind);
        if (e->behave & 0x400000) {
            (*eneReleaseSpawn)(e->spawn);
        }
        return -1;
    }
    if (e->behave & 0x200000) {
        (*eneDeathAct)(o, 1);
        if (e->behave & 0x400000) {
            (*eneReleaseSpawn)(e->spawn);
        }
        return -1;
    }
    if (e->behave & 0x04000000) {
        (*eneSetSd2)(o, 0x405, 0, 0);
        (*itexPuffYellow)(((AnimeFrame *) e->model)->pos[0], ((AnimeFrame *) e->model)->pos[1],
                                  ((AnimeFrame *) e->model)->pos[2], 0.075f);
        return -1;
    }
    if (e->behave & 0x100000) {
        (*itexPuff)(((AnimeFrame *) e->model)->pos[0], ((AnimeFrame *) e->model)->pos[1] + 5.0f,
                                  ((AnimeFrame *) e->model)->pos[2], 0.075f);
        if (e->behave & 0x400000) {
            (*eneReleaseSpawn)(e->spawn);
        }
        return -1;
    }
    if (e->behave & 0x10000000) {
        (*itexPuffWhite)(((AnimeFrame *) e->model)->pos[0], ((AnimeFrame *) e->model)->pos[1],
                                  ((AnimeFrame *) e->model)->pos[2], 0.04f);
        if (e->behave & 0x400000) {
            (*eneReleaseSpawn)(e->spawn);
        }
        return -1;
    }
    if (e->behave & 0x40000) {
        if (e->behave & 0x400000) {
            (*eneReleaseSpawn)(e->spawn);
        }
        return -1;
    }
    box = e->atk;
    e->hit &= ~8;
    if (box != NULL) {
        do {
            if ((*acollAtkWasHit)(box) == 1) {
                PRINTF("eneAfterProc(): 攻撃ヒット\n");
                e->hit |= 8;
                target = (Obj *) box->hitPower;
                if (target != NULL) {
                    (*tskSendMsg)(target, 7, o);
                }
                break;
            }
            if (box == e->atk) {
                if (box->frame.child != NULL) {
                    box = (ACollFrame *) box->frame.child;
                } else {
                    box = NULL;
                }
            } else {
                box = (box->frame.next != NULL) ? (ACollFrame *) box->frame.next : NULL;
            }
        } while (box != NULL);
    }
    if (e->behave & 0x800000) {
        eneFall(o, 0.5f);
    } else if (e->behave & 0x800) {
        eneFall(o, 0.3f);
    }
    eneMove(e);
    if (e->y < -600.0f) {
        PRINTF("eneAfterProc: 落ちすぎ\n");
        return -1;
    }
    ground = NULL;
    e->hit &= ~1;
    if (e->vel[1] < 0.0f && !(e->behave & 0x7C) && !(e->behave & 0x1000)) {
        ground = eneLandGround(o, 8.0f);
        if ((e->hit & 1) && !(e->behave & 0x2000)) {
            e->vel[1] = 0.0f;
        }
    }
    if ((e->behave & 0x20000000) && (*eneAtamaCheck)(o, 10.0f)) {
        e->vel[1] = 0.0f;
    }
    e->hit &= ~6;
    e->hit &= ~6; /* sic: twice */
    if ((e->behave & 4) && eneBlockCheck(o, 4.0f, 0.0f, 0.0f)) {
        e->hit |= 6;
    }
    if (e->behave & 0x78) {
        eneWallCheck(o, 10.0f, 5.0f);
        if (e->hit & 2) {
            if (e->behave & 8) {
                e->x -= e->vel[0];
                e->vel[0] = 0.0f;
            } else if (e->behave & 0x10) {
                e->vel[0] *= -1.0f;
            }
        }
        if (e->hit & 4) {
            if (e->behave & 0x20) {
                e->y -= e->vel[1];
                e->vel[1] = 0.0f;
            } else if (e->behave & 0x40) {
                e->vel[1] *= -1.0f;
            }
        }
    }
    if (e->behave & 0x780) {
        eneEdgeCheck(o, 10.0f, 8.0f, 4.0f);
        if (e->hit & 2) {
            if (e->behave & 0x80) {
                e->x -= e->vel[0];
                e->vel[0] = 0.0f;
            } else if (e->behave & 0x100) {
                e->vel[0] *= -1.0f;
            }
        }
        if (e->hit & 4) {
            if (e->behave & 0x200) {
                e->x -= e->vel[0];
                e->vel[0] = 0.0f;
            } else if (e->behave & 0x400) {
                e->vel[0] *= -1.0f;
            }
        }
    }
    if (e->behave & 0x4000) {
        eneTurnAlongCourse(o, 8.0f);
    }
    if (e->model != NULL) {
        eneSetModelOnCourse(o, e->model);
    }
    if (e->shadow != NULL) {
        if (e->dmg != NULL) {
            size = e->dmg->radius * ((Vec3f *) ((AnimeFrame *) e->model)->scale)->x / 6.0f;
            (*shadowPut)(e->shadow, ((AnimeFrame *) e->model)->pos[0],
                                      ((AnimeFrame *) e->model)->pos[1], ((AnimeFrame *) e->model)->pos[2], size,
                                      (s32) ground);
        } else {
            (*shadowPut)(e->shadow, ((AnimeFrame *) e->model)->pos[0],
                                      ((AnimeFrame *) e->model)->pos[1], ((AnimeFrame *) e->model)->pos[2], 1.0f,
                                      (s32) ground);
        }
    }
    return 0;
}

/* eneSetOnEarth: puts the enemy on the ground within 40 under it, taking its course; -1 if
 * there is none. */
s32 eneSetOnEarth(Obj *o) {
    EneObj *e = ENE(o);
    s32 pad;
    Vec3f from;
    Vec3f dir;
    Vec3f at;
    CollTri *tri;
    f32 t;

    dir.x = 0.0f;
    dir.z = 0.0f;
    dir.y = -40.0f;
    if (e->zone != 0) {
        vramCoursePointXZ(e->zone, e->x, e->z, &from.x, &from.z);
    } else {
        from.x = e->x;
        from.z = e->z;
    }
    from.y = e->y;
    tri = vramRayGround(&from, &dir, &at, &t);
    if (tri != NULL) {
        e->y = at.y;
        if (tri->zone != 0 && e->zone != 0 && e->zone != tri->zone) {
            e->zone = tri->zone;
            e->x = vramNearestPlaceXZ(e->zone, from.x, from.z, &e->z);
        }
        if (e->model != NULL) {
            eneSetModelOnCourse(o, e->model);
        }
        return 0;
    }
    PRINTF("eneSetOnEarth(): 地面にセット失敗\n");
    return -1;
}

/* Lands a falling enemy on the ground under it (h above its feet), taking the ground's course
 * (and z 0 on it); the ground, or NULL. */
CollTri *eneLandGround(Obj *o, f32 h) {
    EneObj *e = ENE(o);
    s32 pad;
    Vec3f from;
    Vec3f dir;
    Vec3f at;
    CollTri *tri = NULL;
    f32 t;

    if (e->vel[1] <= 0.0f) {
        dir.x = 0.0f;
        dir.y = e->vel[1] - h;
        dir.z = 0.0f;
        if (e->zone != 0) {
            vramCoursePointXZ(e->zone, e->x, e->z, &from.x, &from.z);
        } else {
            from.x = e->x;
            from.z = e->z;
        }
        from.y = e->y - e->vel[1] + h;
        tri = vramRayGround(&from, &dir, &at, &t);
        if (tri != NULL) {
            e->y = at.y;
            if (tri->zone != 0 && e->zone != 0 && e->zone != tri->zone) {
                e->zone = tri->zone;
                e->x = vramNearestPlaceXZ(e->zone, from.x, from.z, &e->z);
                e->z = 0.0f;
            }
            e->hit |= 1;
        }
    }
    return tri;
}

/* eneAtamaCheck: whether a rising enemy hits a ceiling within h above it (it stops h under).
 * dir.y reads the speed as VEL(e)->y: with e->vel[1] (a constant-index array element) IDO
 * loads the speed first and swaps f16/f18 (IDO_NOTES 66, the FAKE note on VEL above). */
s32 eneAtamaCheck(Obj *o, f32 h) {
    EneObj *e = ENE(o);
    s32 pad;
    Vec3f from;
    Vec3f dir;
    Vec3f at;
    f32 t;
    f32 k;

    if (e->vel[1] > 0.0f) {
        if (e->zone != 0) {
            vramCoursePointXZ(e->zone, e->x, e->z, &from.x, &from.z);
        } else {
            from.x = e->x;
            from.z = e->z;
        }
        from.y = e->y - e->vel[1] + h / 5.0f;
        dir.x = 0.0f;
        dir.z = 0.0f;
        dir.y = h - h / 5.0f + VEL(e)->y;
        if (vramRayGround(&from, &dir, &at, &t) != NULL) {
            PRINTF("eneAtamaCheck(): あたま打ち\n");
            e->y = at.y - h;
            return 1;
        }
    }
    return 0;
}

/* The edge check: a ray from h above the enemy down to down below it, |dx| ahead the way it
 * moves. Hit 2: a wall in the ray's first half; 4: no ground there (an edge). */
void eneEdgeCheck(Obj *o, f32 h, f32 dx, f32 down) {
    EneObj *e = ENE(o);
    s32 s;
    s32 pad;
    f32 top;
    s32 pad2[2];
    f32 bottom;
    f32 x;
    f32 z;
    s32 pad3[5];
    Vec3f from;
    Vec3f to;
    Vec3f dir;
    Vec3f at;
    f32 t;
    s32 pad4[2];

    dx = fabsf(dx);
    s = (e->vel[0] < 0.0f) ? -1 : 1;
    dx *= s;
    x = e->x;
    top = e->y + h;
    z = e->z;
    vramCoursePointXZ(e->zone, x, z, &from.x, &from.z);
    from.y = top;
    x = e->x + dx;
    bottom = e->y - down;
    z = e->z;
    vramCoursePointXZ(e->zone, x, z, &to.x, &to.z);
    dir.x = to.x - from.x;
    dir.y = bottom - from.y;
    dir.z = to.z - from.z;
    to.y = bottom;
    at.x = at.y = at.z = 0.0f;
    if (vramRayGround(&from, &dir, &at, &t) != NULL) {
        if (t < 0.5) {
            e->hit |= 2;
        }
    } else {
        e->hit |= 4;
    }
}

/* The wall checks: a ray |dx| along the course the way the enemy moves (a wall there stops it
 * dx short: hit 2), then one |dy| up or down the way it moves (hit 4, dy short). The first
 * ray aims at the course's middle line (z 0), not at the enemy's z (sic?). */
void eneWallCheck(Obj *o, f32 dx, f32 dy) {
    EneObj *e = ENE(o);
    s32 pad[2];
    f32 y;
    f32 x;
    f32 z;
    f32 step;
    s32 pad2;
    Vec3f pos;
    Vec3f from;
    Vec3f dir;
    Vec3f at;
    f32 t;
    s32 pad3[2];

    x = e->x;
    y = e->y;
    z = e->z;
    vramCoursePointXZ(e->zone, x, z, &from.x, &from.z);
    from.y = y;
    if (fabs(e->vel[0]) > 0.0) {
        step = (e->vel[0] > 0.0f) ? dx : -dx;
        vramCoursePointXZ(e->zone, x + step, 0.0f, &dir.x, &dir.z);
        dir.x -= from.x;
        dir.z -= from.z;
        dir.y = 0.0f;
        if (vramRayGround(&from, &dir, &at, &t) != NULL) {
            pos.x = at.x;
            pos.y = at.y;
            pos.z = at.z;
            pos.x = vramNearestPlaceXZ(e->zone, at.x, at.z, &pos.z);
            e->x = pos.x - ((e->vel[0] < 0.0f) ? -1 : 1) * dx;
            e->hit |= 2;
        }
    }
    if (fabs(e->vel[1]) > 0.0) {
        dir.x = 0.0f;
        dir.y = (e->vel[1] > 0.0f) ? dy : -dy;
        dir.z = 0.0f;
        if (vramRayGround(&from, &dir, &at, &t) != NULL) {
            pos.y = at.y;
            e->y = pos.y - ((e->vel[1] < 0.0f) ? -1 : 1) * dy;
            e->hit |= 4;
        }
    }
}

/* Whether the way the enemy moves is blocked within d: a ray from (dx, dy) off it, d long split
 * between along and up by its speed's slope. */
s32 eneBlockCheck(Obj *o, f32 d, f32 dx, f32 dy) {
    EneObj *e = ENE(o);
    s32 pad;
    f32 z;
    f32 y;
    f32 x;
    f32 kx;
    f32 ky;
    s32 pad2[4];
    Vec3f from;
    Vec3f dir;
    Vec3f at;
    f32 t;

    x = e->x + dx;
    y = e->y + dy;
    z = e->z;
    vramCoursePointXZ(e->zone, x, z, &from.x, &from.z);
    from.y = y;
    if (fabs(e->vel[0]) > fabs(e->vel[1])) {
        kx = d;
        ky = fabs(e->vel[1]) / fabs(e->vel[0]) * d;
    } else if (fabs(e->vel[0]) < fabs(e->vel[1])) {
        ky = d;
        kx = fabs(e->vel[0]) / fabs(e->vel[1]) * d;
    } else {
        kx = d;
        ky = d;
    }
    kx *= (e->vel[0] < 0.0f) ? -1 : 1;
    ky *= (e->vel[1] < 0.0f) ? -1 : 1;
    vramCoursePointXZ(e->zone, x + kx, z, &dir.x, &dir.z);
    dir.x -= from.x;
    dir.z -= from.z;
    dir.y = ky;
    if (vramRayGround(&from, &dir, &at, &t) != NULL) {
        return 1;
    }
    return 0;
}

/* Whether the camera is within r of the enemy (flat). */
s32 eneCamNearCheck(Obj *o, f32 r) {
    EneObj *e = ENE(o);
    s32 pad2;
    f32 cx;
    f32 cy;
    f32 cz;
    f32 x;
    f32 z;
    s32 pad;
    s32 near = 0;

    if (camAiGetView() != NULL) {
        camAiGetAt(&cx, &cy, &cz);
        vramCoursePointXZ(e->zone, e->x, e->z, &x, &z);
        if (sqrtf((cx - x) * (cx - x) + (cz - z) * (cz - z)) <= r) {
            near = 1;
        }
    }
    return near;
}

/* The nearest player (1-4) if within r of the enemy along the course, else 0. */
s32 eneNearestPlInX(Obj *o, f32 r) {
    s32 pad[2];
    f32 x;
    f32 y;
    f32 z;
    s32 zone;
    s32 n = eneNearestPl(o);
    s32 pad2;

    if (n != 0) {
        plAskPos(gGameWork.players[n - 1], &x, &y, &z, &zone);
        if (r < ((ENE(o)->x < x) ? x - ENE(o)->x : -(x - ENE(o)->x))) {
            n = 0;
        }
    }
    return n;
}

/* The nearest player (1-4) if within r of the enemy (flat), else 0. */
s32 eneNearestPlInR(Obj *o, f32 r) {
    EneObj *e = ENE(o);
    s32 pad;
    f32 x;
    f32 y;
    f32 z;
    s32 zone;
    s32 n = eneNearestPl(o);

    if (n != 0) {
        plAskPos(gGameWork.players[n - 1], &x, &y, &z, &zone);
        if (r < sqrtf((x - e->x) * (x - e->x) + (z - e->z) * (z - e->z))) {
            n = 0;
        }
    }
    return n;
}

/* Whether the players in play on the enemy's course (or player n; 0: all) are within r of it
 * (flat): 1 all of them, 0 none, 2 some. which is unsigned: the -1 it is tested against is not
 * the -1 stored for a player out of play (two registers in the game). */
s32 enePlInRangeCheck(Obj *o, f32 r, s32 n) {
    EneObj *e = ENE(o);
    s32 pad;
    f32 x;
    f32 y;
    f32 z;
    s32 pad2;
    s32 zone;
    s32 in[4];
    s32 i;
    u32 which;

    for (i = 0; i < 4; i++) {
        in[i] = 0;
    }
    for (i = 0, which = n - 1; i < 4; i++) {
        if (i == which || which == -1) {
            if (plIsInPlay(i)) {
                plAskPos(gGameWork.players[i], &x, &y, &z, &zone);
                if (zone == e->zone) {
                    if (sqrtf((x - e->x) * (x - e->x) + (z - e->z) * (z - e->z)) <= r) {
                        in[i] = 1;
                    }
                }
            } else {
                in[i] = -1;
            }
        }
    }
    if (in[0] >= 0 && in[1] >= 0 && in[2] >= 0 && in[3] >= 0) {
        return 1;
    }
    if (in[0] <= 0 && in[1] <= 0 && in[2] <= 0 && in[3] <= 0) {
        return 0;
    }
    return 2;
}

/* Which way the nearest player is along the course: 1 ahead, 4 behind, 0 none. */
s32 eneNearestPlWay(Obj *o) {
    s32 pad[2];
    f32 x;
    f32 y;
    f32 z;
    s32 zone;
    s32 n;
    s32 way = 0;

    n = eneNearestPl(o);
    if (n != 0) {
        plAskPos(gGameWork.players[n - 1], &x, &y, &z, &zone);
        way = (x <= ENE(o)->x) ? 4 : 1;
    }
    return way;
}

/* The nearest player in play, 1-4 (0: none within 3000). */
s32 eneNearestPl(Obj *o) {
    EneObj *e = ENE(o);
    s32 pad;
    f32 x;
    f32 y;
    f32 z;
    f32 min = 3000.0f;
    s32 n = 0;
    s32 zone;
    s32 i;
    f32 d;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plAskPos(gGameWork.players[i], &x, &y, &z, &zone);
            d = sqrtf((x - e->x) * (x - e->x) + (y - e->y) * (y - e->y) + (z - e->z) * (z - e->z));
            if (d < min) {
                n = i + 1;
                min = d;
            }
        }
    }
    return n;
}

/* How far the nearest player in play (or player n; 0: all) is, over the axes in mask (1 along,
 * 2 up, 4 across); -1 if none. */
f32 enePlDist(Obj *o, s32 n, s32 mask) {
    EneObj *e = ENE(o);
    s32 pad;
    f32 x;
    f32 y;
    f32 z;
    f32 sx;
    f32 sy;
    f32 sz;
    f32 min;
    f32 d;
    s32 zone;
    s32 i;
    s32 which;

    sz = 0.0f;
    sx = sy = 0.0f;
    min = -1.0f;
    for (i = 0, which = n - 1; i < 4; i++) {
        if ((i == which || which == -1) && plIsInPlay(i)) {
            plAskPos(gGameWork.players[i], &x, &y, &z, &zone);
            if (mask & 1) {
                d = x - e->x;
                sx = d * d;
            }
            if (mask & 2) {
                d = y - e->y;
                sy = d * d;
            }
            if (mask & 4) {
                d = z - e->z;
                sz = d * d;
            }
            d = sqrtf(sx + sy + sz);
            if (min == -1.0f) {
                min = d;
            } else if (d < min) {
                min = d;
            }
        }
    }
    return min;
}

/* Where the nearest player in play (or player n; 0: all) is from the enemy: 1 ahead, 4
 * behind, 0x10 above, 0x20 below, 8 and 2 to either side. Unset if none (sic). */
s32 enePlDir(Obj *o, s32 n) {
    EneObj *e = ENE(o);
    f32 min = 3000.0f;
    f32 x;
    f32 y;
    f32 z;
    s32 i;
    s32 which;
    s32 zone;
    f32 d;
    s32 dirs;
#ifdef AVOID_UB
    dirs = 0;
#endif

    //! @bug dirs is never set when no player is within 3000.
    for (i = 0, which = n - 1; i < 4; i++) {
        if ((i == which || which == -1) && plIsInPlay(i)) {
            plAskPos(gGameWork.players[i], &x, &y, &z, &zone);
            d = sqrtf((x - e->x) * (x - e->x) + (y - e->y) * (y - e->y) + (z - e->z) * (z - e->z));
            if (d < min) {
                dirs = 0;
                dirs |= (e->x < x) ? 1 : 0;
                dirs |= (x < e->x) ? 4 : 0;
                dirs |= (e->y < y) ? 0x10 : 0;
                dirs |= (y < e->y) ? 0x20 : 0;
                dirs |= (e->z < z) ? 8 : 0;
                dirs |= (z < e->z) ? 2 : 0;
                min = d;
            }
        }
    }
    return dirs;
}

/* The same along the course only: 1 ahead, 4 behind. */
s32 enePlDirX(Obj *o, s32 n) {
    EneObj *e = ENE(o);
    f32 min = 3000.0f;
    f32 x;
    f32 y;
    f32 z;
    s32 i;
    s32 which;
    s32 zone;
    f32 d;
    s32 dirs;
#ifdef AVOID_UB
    dirs = 0;
#endif

    //! @bug dirs is never set when no player is within 3000.
    for (i = 0, which = n - 1; i < 4; i++) {
        if ((i == which || which == -1) && plIsInPlay(i)) {
            plAskPos(gGameWork.players[i], &x, &y, &z, &zone);
            d = sqrtf((x - e->x) * (x - e->x) + (y - e->y) * (y - e->y) + (z - e->z) * (z - e->z));
            if (d < min) {
                dirs = 0;
                dirs |= (e->x < x) ? 1 : 0;
                dirs |= (x < e->x) ? 4 : 0;
                min = d;
            }
        }
    }
    return dirs;
}

/* The same up and down only: 0x10 above, 0x20 below. */
s32 enePlDirY(Obj *o, s32 n) {
    EneObj *e = ENE(o);
    f32 min = 3000.0f;
    f32 x;
    f32 y;
    f32 z;
    s32 i;
    s32 which;
    s32 zone;
    f32 d;
    s32 dirs;
#ifdef AVOID_UB
    dirs = 0;
#endif

    //! @bug dirs is never set when no player is within 3000.
    for (i = 0, which = n - 1; i < 4; i++) {
        if ((i == which || which == -1) && plIsInPlay(i)) {
            plAskPos(gGameWork.players[i], &x, &y, &z, &zone);
            d = sqrtf((x - e->x) * (x - e->x) + (y - e->y) * (y - e->y) + (z - e->z) * (z - e->z));
            if (d < min) {
                dirs = 0;
                dirs |= (e->y < y) ? 0x10 : 0;
                dirs |= (y < e->y) ? 0x20 : 0;
                min = d;
            }
        }
    }
    return dirs;
}

/* The course's heading at the enemy, turned by way (1: not, 2: a quarter, 4: a half, 8: three
 * quarters), in -pi..pi. Another way gives an unset value (sic). */
f32 eneCourseAngle(Obj *o, s32 way) {
    EneObj *e = ENE(o);
    s32 pad;
    f32 a;
#ifdef AVOID_UB
    a = 0.0f;
#endif

    switch (way) {
        case 1:
            a = (*vramCourseHeading)(e->zone, e->x);
            break;
        case 2:
            a = (*vramCourseHeading)(e->zone, e->x) + 3.1415927f * 0.5;
            break;
        case 4:
            a = (*vramCourseHeading)(e->zone, e->x) + 3.1415927f;
            break;
        case 8:
            a = (*vramCourseHeading)(e->zone, e->x) + 3.1415927f * 1.5;
            break;
    }
    //! @bug a is never set for any other way, and callers pass eneNearestPlWay's result, which is
    //! 0 when no player is near (tlb_0078, tlb_0081, tlb_0082). a is then whatever was on the
    //! stack, and for an infinite or very large value this loop never ends.
    while (fabs(a) > 3.1415927f) {
        a -= (a > 0.0f) ? 6.2831855f : -6.2831855f;
    }
    return a;
}

/* Which quarter the enemy faces in against its course's heading: 1 along, 2, 4, 8. */
s32 eneFaceWay(Obj *o) {
    EneObj *e = ENE(o);
    s32 pad;
    f32 t;
    s32 way;

    t = (e->rot[1] - (*vramCourseHeading)(e->zone, e->x) + 0.7853982f) * 0.63661975f;
    t += 256.0f;
    switch ((s32) t & 3) {
        case 0:
            way = 1;
            break;
        case 1:
            way = 2;
            break;
        case 2:
            way = 4;
            break;
        case 3:
            way = 8;
            break;
    }
    return way;
}

/* Turns the enemy's heading towards a (brought into -pi..pi) by step; 0 once it is there.
 * e is set at its declaration: assigned after the first loop it was folded into o + 0x40 and
 * pi, 0 and +-2pi left fs0-fs3 (IDO_NOTES 182). */
s32 eneTurnRotYTo(Obj *o, f32 a, f32 step) {
    EneObj *e = ENE(o);

    while (fabs(a) > 3.1415927f) {
        a -= (a > 0.0f) ? 6.2831855f : -6.2831855f;
    }
    if (a - fabs(step) < e->rot[1] && e->rot[1] < fabs(step) + a) {
        e->rot[1] = a;
        return 0;
    }
    e->rot[1] += step;
    while (fabs(e->rot[1]) > 3.1415927f) {
        e->rot[1] -= (e->rot[1] > 0.0f) ? 6.2831855f : -6.2831855f;
    }
    return 1;
}

/* The enemy's model makers: a frame at the enemy, a tenth its size, in its kind's files.
 * pos[1] is set before the scales: set after them, e->y (an f32 like the scales) was loaded
 * after their stores and the flags' ori moved up one instruction. */

/* An "anime" model playing anim, drawing dl (under the second root with second). */
AnimeFrame *eneAnimeModelMake(Obj *o, u32 anim, Gfx *dl, s32 second) {
    EneObj *e = ENE(o);
    AnimeFrame *f;
    EneKind *kind = ENE(o)->kind;
    s32 pad[3];

    if (second) {
        f = (AnimeFrame *) frmCreate(gFrmLayer8, gFrmKindAnime);
    } else {
        f = (AnimeFrame *) frmCreate(gFrmLayer2, gFrmKindAnime);
    }
    if (f != NULL) {
        vramCoursePointXZ(e->zone, e->x, e->z, &f->pos[0], &f->pos[2]);
        f->pos[1] = e->y;
        f->scale[0] = 0.1f;
        f->scale[1] = 0.1f;
        f->scale[2] = 0.1f;
        f->dl = dl;
        f->anim = anim;
        f->flags |= 0x20;
        f->segs[0] = fileRamAddr(kind->files.file[0]);
        f->segs[1] = fileRamAddr(kind->files.file[1]);
        f->segs[2] = fileRamAddr(kind->files.file[2]);
    }
    return f;
}

/* An "anime" model under the second root, drawn with its environment alpha. */
AnimeFrame *eneAnimeModelAlpha(Obj *o, u32 anim, s32 alpha) {
    EneObj *e = ENE(o);
    AnimeFrame *f;
    EneKind *kind = ENE(o)->kind;
    s32 pad[3];

    f = (AnimeFrame *) frmCreate(gFrmLayer8, gFrmKindAnime);
    if (f != NULL) {
        vramCoursePointXZ(e->zone, e->x, e->z, &f->pos[0], &f->pos[2]);
        f->pos[1] = e->y;
        f->scale[0] = 0.1f;
        f->scale[1] = 0.1f;
        f->scale[2] = 0.1f;
        f->anim = anim;
        f->flags |= 0x20;
        f->dl = gGfxDlHead;
        gDPPipeSync(gGfxDlHead++);
        gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, alpha);
        gSPEndDisplayList(gGfxDlHead++);
        f->segs[0] = fileRamAddr(kind->files.file[0]);
        f->segs[1] = fileRamAddr(kind->files.file[1]);
        f->segs[2] = fileRamAddr(kind->files.file[2]);
    }
    return f;
}

/* A "pol" model drawing dl2, under parent. */
PolFrame *enePolModelUnder(Obj *o, Gfx *dl2, Frame *parent) {
    EneObj *e = ENE(o);
    PolFrame *f;
    EneKind *kind = ENE(o)->kind;
    s32 pad[2];

    f = (PolFrame *) frmCreate(parent, gFrmKindPol);
    if (f != NULL) {
        vramCoursePointXZ(e->zone, e->x, e->z, &f->pos[0], &f->pos[2]);
        f->pos[1] = e->y;
        f->scale[0] = 0.1f;
        f->scale[1] = 0.1f;
        f->scale[2] = 0.1f;
        f->dl2 = dl2;
        f->flags |= 0x20;
        f->segs[0] = fileRamAddr(kind->files.file[0]);
        f->segs[1] = fileRamAddr(kind->files.file[1]);
        f->segs[2] = fileRamAddr(kind->files.file[2]);
    }
    return f;
}

/* A "pol" model drawing dl then dl2 (under the second root with second). */
PolFrame *enePolModelMake(Obj *o, Gfx *dl2, Gfx *dl, s32 second) {
    EneObj *e = ENE(o);
    PolFrame *f;
    EneKind *kind = ENE(o)->kind;
    s32 pad[3];

    if (second) {
        f = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
    } else {
        f = (PolFrame *) frmCreate(gFrmLayer2, gFrmKindPol);
    }
    if (f != NULL) {
        vramCoursePointXZ(e->zone, e->x, e->z, &f->pos[0], &f->pos[2]);
        f->pos[1] = e->y;
        f->scale[0] = 0.1f;
        f->scale[1] = 0.1f;
        f->scale[2] = 0.1f;
        f->dl = dl;
        f->dl2 = dl2;
        f->flags |= 0x20;
        f->segs[0] = fileRamAddr(kind->files.file[0]);
        f->segs[1] = fileRamAddr(kind->files.file[1]);
        f->segs[2] = fileRamAddr(kind->files.file[2]);
    }
    return f;
}

/* A "pol" model under the second root, drawn with its environment alpha. */
PolFrame *enePolModelAlpha(Obj *o, Gfx *dl2, s32 alpha) {
    EneObj *e = ENE(o);
    PolFrame *f;
    EneKind *kind = ENE(o)->kind;
    s32 pad[3];

    f = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
    if (f != NULL) {
        vramCoursePointXZ(e->zone, e->x, e->z, &f->pos[0], &f->pos[2]);
        f->pos[1] = e->y;
        f->scale[0] = 0.1f;
        f->scale[1] = 0.1f;
        f->scale[2] = 0.1f;
        f->dl2 = dl2;
        f->flags |= 0x20;
        f->dl = gGfxDlHead;
        gDPPipeSync(gGfxDlHead++);
        gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, alpha);
        gSPEndDisplayList(gGfxDlHead++);
        f->segs[0] = fileRamAddr(kind->files.file[0]);
        f->segs[1] = fileRamAddr(kind->files.file[1]);
        f->segs[2] = fileRamAddr(kind->files.file[2]);
    }
    return f;
}

/* The same, made through frmCreateNext. */
PolFrame *enePolModelAlphaNext(Obj *o, Gfx *dl2, s32 alpha) {
    EneObj *e = ENE(o);
    PolFrame *f;
    EneKind *kind = ENE(o)->kind;
    s32 pad[3];

    f = (PolFrame *) frmCreateNext(gFrmLayer8, gFrmKindPol);
    if (f != NULL) {
        vramCoursePointXZ(e->zone, e->x, e->z, &f->pos[0], &f->pos[2]);
        f->pos[1] = e->y;
        f->scale[0] = 0.1f;
        f->scale[1] = 0.1f;
        f->scale[2] = 0.1f;
        f->dl2 = dl2;
        f->flags |= 0x20;
        f->dl = gGfxDlHead;
        gDPPipeSync(gGfxDlHead++);
        gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, alpha);
        gSPEndDisplayList(gGfxDlHead++);
        f->segs[0] = fileRamAddr(kind->files.file[0]);
        f->segs[1] = fileRamAddr(kind->files.file[1]);
        f->segs[2] = fileRamAddr(kind->files.file[2]);
    }
    return f;
}

/* A sound at the enemy's model. */
void eneSetSd(Obj *o, u16 se) {
    eneSetSd2(o, se, 0, 0);
}

/* eneSetSd2: the sound se (se2 instead, if there is one, after an odd number of kills) at the enemy's model: mode 0 SoundLocation2 (near 0, far 300), 1 SoundIncam2 (near 0,
 * far 300), 2 the same to 550. The first test reads the model through o, the rest through e
 * (the game loads the model again after it). */
void eneSetSd2(Obj *o, u16 se, u16 se2, u8 mode) {
    EneObj *e = ENE(o);
    s32 pad;
    f32 x;
    f32 y;
    f32 z;
    u16 id;

    if (ENE(o)->model->kind == gFrmKindAnime) {
        x = ((AnimeFrame *) e->model)->pos[0];
        y = ((AnimeFrame *) e->model)->pos[1];
        z = ((AnimeFrame *) e->model)->pos[2];
    } else if (e->model->kind == gFrmKindPol) {
        x = ((PolFrame *) e->model)->pos[0];
        y = ((PolFrame *) e->model)->pos[1];
        z = ((PolFrame *) e->model)->pos[2];
    }
    id = se;
    if (se2 != 0 && (gGameWork.kills & 1)) {
        id = se2;
    }
    switch (mode) {
        case 0:
            PRINTF("eneSetSd2(): use SoundIncam2(), near 0, far 300, code %d \n", id);
            (*SoundIncam2)(id, gGameWork.view, x, y, z, 0.0f, 300.0f);
            break;
        case 1:
            PRINTF("eneSetSd2(): use SoundLocation2(), near 0, far 300, code %d \n", id);
            (*SoundLocation2)(id, gGameWork.view, x, y, z, 0.0f, 300.0f, 1.5707964f);
            break;
        case 2:
            PRINTF("eneSetSd2(): use SoundLocation2(), near 0, far 550, code %d \n", id);
            (*SoundLocation2)(id, gGameWork.view, x, y, z, 0.0f, 550.0f, 1.5707964f);
            break;
    }
}

/* Steps a model's animation by step up to end (or down to 0); 1 when it gets there. */
s32 eneAnimeStepTo(Obj *o, AnimeFrame *f, f32 step, f32 end) {
    f->frame += step;
    if (step > 0.0f) {
        if (end <= f->frame) {
            f->frame = end;
            return 1;
        }
    } else if (f->frame <= 0.0f) {
        f->frame = 0.0f;
        return 1;
    }
    return 0;
}

/* Steps a model's animation by step, looping over its length; 1 when it wraps. */
s32 eneAnimeLoop(Obj *o, AnimeFrame *f, f32 step) {
    return eneAnimeLoopLen(o, f, step, polAnimeLastFrame(f));
}

/* The same over len (0: the animation's length + 1). */
s32 eneAnimeLoopLen(Obj *o, AnimeFrame *f, f32 step, f32 len) {
    if (len == 0.0f) {
        len = polAnimeLastFrame(f) + 1;
    }
    f->frame += step;
    if (step > 0.0f) {
        if (len <= f->frame) {
            f->frame -= len;
            return 1;
        }
    } else if (f->frame < 0.0f) {
        f->frame += len;
        return 1;
    }
    return 0;
}

/* Turns the enemy's attack boxes (a box and its children) on (1) or off (0). */
void eneAtkSwitch(Obj *o, ACollFrame *atk, s32 on) {
    ACollFrame *box = atk;

    if (atk != NULL) {
        do {
            if (on == 1) {
                box->flags |= 2;
            } else if (on == 0) {
                box->flags &= ~2;
            }
            if (box == atk) {
                if (box->frame.child != NULL) {
                    box = (ACollFrame *) box->frame.child;
                } else {
                    box = NULL;
                }
            } else {
                box = (box->frame.next != NULL) ? (ACollFrame *) box->frame.next : NULL;
            }
        } while (box != NULL);
    }
}

/* Blows the enemy off as a "futtobi_hahen" piece of its model, away from the hit (with the
 * hit's spark). Like eneSetSd2, the first test reads the model through o and the rest
 * through e; gFrmLayer2 is read as the frame pool gFrmLayers[2] (as a plain global, IDO puts
 * it first in the compare's registers). */
void eneBlowOff(Obj *o, ACollFrame *box) {
    EneObj *e = ENE(o);
    s32 pad;
    AnimeFrame *a;
    PolFrame *p;

    if (ENE(o)->model->kind == gFrmKindAnime) {
        a = (AnimeFrame *) e->model;
        eneHitSpark(o, box);
        eneMakeFuttobiHahenAnime(o, box, a->anim, a->frame, 0, (a->f.parent == gFrmLayers[2]) ? 1 : 0);
    } else if (e->model->kind == gFrmKindPol) {
        p = (PolFrame *) e->model;
        eneHitSpark(o, box);
        eneMakeFuttobiHahen(o, box, (u32) p->dl2, 0, (p->f.parent == gFrmLayers[2]) ? 1 : 0);
    }
}

/* A hit's spark (eneMakeHitSpark) halfway between the hit and the enemy's model, across the
 * attacker's heading. The two model pointers are two locals (one register each). */
void eneHitSpark(Obj *o, ACollFrame *box) {
    EneObj *e = ENE(o);
    AnimeFrame *a;
    f32 x;
    f32 y;
    f32 z;
    PolFrame *p;

    if (ENE(o)->model->kind == gFrmKindAnime) {
        a = (AnimeFrame *) e->model;
        x = box->hitPos[0] + (a->pos[0] - box->hitPos[0]) * 0.5f;
        y = box->hitPos[1];
        z = box->hitPos[2] + (a->pos[2] - box->hitPos[2]) * 0.5f;
    } else if (e->model->kind == gFrmKindPol) {
        p = (PolFrame *) e->model;
        x = box->hitPos[0] + (p->pos[0] - box->hitPos[0]) * 0.5f;
        y = box->hitPos[1];
        z = box->hitPos[2] + (p->pos[2] - box->hitPos[2]) * 0.5f;
    }
    PRINTF("!!!! tx = %f, ty = %f, tz = %f \n", x, y, z);
    eneMakeHitSpark(0, x, y, z, 2, box->hitRotY - 1.5707964f);
}

/* eneDeathAct: how a beaten enemy goes, by the attack's kind, counting the kill:
 * - 4-6 (cuts): cut in two ("kill_hahen", its two halves' lists; sound 0x21F), or with 0x8000
 *   a puff of smoke (sound 0x229, or 0x22A every other kill);
 * - 18-20: blown off ("futtobi_hahen"; sound 0x22D), or with 0x10000 the smoke;
 * - else the smoke.
 * Unless 0x20000, those but the plain smoke (for 1-3, 17, 24 and 28 too) drop a reward for
 * the player whose attack it was (eneKillReward).
 * e is set at its declaration (IDO makes it in each case, o stays at its home), and the
 * attacker is looked for in a loop over the four players (IDO unrolls it). */
void eneDeathAct(Obj *o, s32 kind) {
    EneObj *e = ENE(o);
    s32 i;
    s32 reward = 0;
    s32 r;
    Obj *by;

    PRINTF("eneDeathAct(): %d 人斬り\n", gGameWork.kills);
    gGameWork.kills++;
    switch (kind) {
        case 4:
        case 5:
        case 6:
            if (!(e->behave & 0x8000)) {
                (*eneSetSd2)(o, 0x21F, 0, 0);
                (*eneMakeKillHahen)(o, e->dmg, e->halves[0], e->halves[1]);
            } else {
                (*eneSetSd2)(o, 0x229, 0x22A, 0);
                (*effectBurst)(((AnimeFrame *) e->model)->pos[0], ((AnimeFrame *) e->model)->pos[1] + 5.0f,
                                          ((AnimeFrame *) e->model)->pos[2]);
            }
            reward = 1;
            break;
        case 18:
        case 19:
        case 20:
            if (!(e->behave & 0x10000)) {
                (*eneSetSd2)(o, 0x22D, 0, 0);
                (*eneBlowOff)(o, e->dmg);
            } else {
                (*eneSetSd2)(o, 0x229, 0x22A, 0);
                (*effectBurst)(((AnimeFrame *) e->model)->pos[0], ((AnimeFrame *) e->model)->pos[1] + 5.0f,
                                          ((AnimeFrame *) e->model)->pos[2]);
            }
            reward = 1;
            break;
        case 1:
        case 2:
        case 3:
        case 17:
        case 24:
        case 28:
            reward = 1;
        default:
            (*eneSetSd2)(o, 0x229, 0x22A, 0);
            (*effectBurst)(((AnimeFrame *) e->model)->pos[0], ((AnimeFrame *) e->model)->pos[1] + 5.0f,
                                      ((AnimeFrame *) e->model)->pos[2]);
            break;
    }
    if (reward == 1 && !(e->behave & 0x20000)) {
        r = -1;
        by = (Obj *) e->dmg->hitPower;
        if (by != NULL) {
            PRINTF("atkD = 0x%x, plD %d = 0x%x\n", by, 0, gGameWork.players[0]);
            for (i = 0; i < 4; i++) {
                if (by == gGameWork.players[i]) {
                    r = PL(gGameWork.players[i])->index;
                }
            }
        }
        PRINTF("plnum %d\n", r);
        (*eneKillReward)(o, r, e->behave & 0x08000000);
    }
}

/* The reward for a kill, by the player r who made it (-1: none): item 0x1FC every 13 or 26
 * kills while its first row of pips is at 1 or 2, item 0x1FD every 15 kills for faces -1 to 1,
 * else coins (0x1FB): at night (19:00-5:00) kinds 11-16 (with many) or 9 and 10, by day kinds
 * 6-8 (with many) or 0. The game calls the maker through a register; e is set at its
 * declaration (IDO makes it from o, kept in s3, in each branch). */
void eneKillReward(Obj *o, s32 r, s32 many) {
    EneObj *e = ENE(o);
    s8 pips = -1;
    s32 face = -1;
    s32 hour = gGameWork.time / 0x10000;
    s32 i;

    if (r >= 0 && r < 4) {
        pips = gSaveData.players[r].life;
        face = gSaveData.players[r].weaponLevel + 1;
    }
    if (pips > 0 && pips < 3 && gGameWork.kills % (pips * 13) == 0) {
        (*eneMakeEx)(NULL, ENE_ETSK_DANGO_MAIN, e->zone, e->x, e->y, 0.0f, 0, NULL);
        return;
    }
    if (face >= 0 && face < 3 && gGameWork.kills % 15 == 0) {
        (*eneMakeEx)(NULL, ENE_ETSK_NEKO_MAIN_FLAGS_0, e->zone, e->x, e->y, 0.0f, 0, NULL);
        return;
    }
    if (hour >= 19 || hour < 5) {
        if (many) {
            for (i = 11; i < 17; i++) {
                (*eneMakeEx)(NULL, ENE_ETSK_KOBAN_MAIN, e->zone, e->x, e->y, 0.0f, 2, (void *) i);
            }
            return;
        }
        (*eneMakeEx)(NULL, ENE_ETSK_KOBAN_MAIN, e->zone, e->x, e->y, 0.0f, 2, (void *) 9);
        (*eneMakeEx)(NULL, ENE_ETSK_KOBAN_MAIN, e->zone, e->x, e->y, 0.0f, 2, (void *) 10);
        return;
    }
    if (many) {
        for (i = 6; i < 9; i++) {
            (*eneMakeEx)(NULL, ENE_ETSK_KOBAN_MAIN, e->zone, e->x, e->y, 0.0f, 2, (void *) i);
        }
        return;
    }
    (*eneMakeEx)(NULL, ENE_ETSK_KOBAN_MAIN, e->zone, e->x, e->y, 0.0f, 2, (void *) 0);
}

/* A display list for a model: gDrawSetupEnvTintPrimShadeDl, then the environment colour (r, g, b, a); where it
 * starts. */
Gfx *eneDlEnvColor(Obj *o, s32 a, s32 r, s32 g, s32 b) {
    Gfx *start = gGfxDlHead;

    gSPDisplayList(gGfxDlHead++, gDrawSetupEnvTintPrimShadeDl);
    gDPSetEnvColor(gGfxDlHead++, r, g, b, a);
    gSPEndDisplayList(gGfxDlHead++);
    return start;
}

/* The same after gDrawSetupEnvTintTexShadeDl (lit 1) or gDrawSetupEnvTintTexDl; where it starts. */
Gfx *eneDlEnvColorLit(Obj *o, s32 a, s32 r, s32 g, s32 b, s32 lit) {
    Gfx *start = gGfxDlHead;

    if (lit == 1) {
        gSPDisplayList(gGfxDlHead++, gDrawSetupEnvTintTexShadeDl);
    } else {
        gSPDisplayList(gGfxDlHead++, gDrawSetupEnvTintTexDl);
    }
    gDPSetEnvColor(gGfxDlHead++, r, g, b, a);
    gSPEndDisplayList(gGfxDlHead++);
    return start;
}

/* The same after gDrawSetupPrimEnvTexBlendDl (either way), with a primitive colour (pr, pg, pb) too. */
Gfx *eneDlEnvPrimColor(Obj *o, s32 r, s32 g, s32 b, s32 pr, s32 pg, s32 pb, s32 a) {
    Gfx *start = gGfxDlHead;

    if (a < 0xFF) {
        gSPDisplayList(gGfxDlHead++, gDrawSetupPrimEnvTexBlendDl);
    } else {
        gSPDisplayList(gGfxDlHead++, gDrawSetupPrimEnvTexBlendDl);
    }
    gDPSetEnvColor(gGfxDlHead++, r, g, b, a);
    gDPSetPrimColor(gGfxDlHead++, 0, 0, pr, pg, pb, 0);
    gSPEndDisplayList(gGfxDlHead++);
    return start;
}

/* The moves and turns to a point (the four below). The differences to the point are written
 * out each time (IDO keeps each in one register; named locals spill), and the velocity is read
 * through VEL. A model of neither kind prints "フレーム がありません" in DF20 and E3A0 (the
 * dead print's else is what keeps x, z and h in registers there). */

/* Heads the enemy's model at (tx, tz) (flat) at speed / 5; 1 (and stopped) once it is there or
 * would pass it. The game's frame has two more words below z (pad2). */
s32 eneMoveToPos(Obj *o, f32 tx, f32 tz, f32 speed) {
    EneObj *e = ENE(o);
    s32 pad;
    f32 d;
    f32 x;
    f32 z;
    s32 pad2[2];

    if (e->model->kind == gFrmKindAnime) {
        x = ((AnimeFrame *) e->model)->pos[0];
        z = ((AnimeFrame *) e->model)->pos[2];
    } else if (e->model->kind == gFrmKindPol) {
        x = ((PolFrame *) e->model)->pos[0];
        z = ((PolFrame *) e->model)->pos[2];
    } else {
        PRINTF("フレーム がありません\n");
    }
    d = sqrtf((tx - x) * (tx - x) + (tz - z) * (tz - z));
    if (d == 0.0f) {
        VEL(e)->x = 0.0f;
        VEL(e)->z = 0.0f;
        return 1;
    }
    VEL(e)->x = (tx - x) / d * speed / 5.0f;
    VEL(e)->z = (tz - z) / d * speed / 5.0f;
    if ((tx - x) * (tx - (x + VEL(e)->x)) < 0.0f || (tz - z) * (tz - (z + VEL(e)->z)) < 0.0f) {
        VEL(e)->x = 0.0f;
        VEL(e)->z = 0.0f;
        return 1;
    }
    return 0;
}

/* The same in 3-D at speed, from the "anime" model's place (a "pol" enemy's own place);
 * 1 once it would pass the point. The game divides by 1 (an int literal keeps the div.s). */
s32 eneMoveToPos3D(Obj *o, f32 tx, f32 ty, f32 tz, f32 speed) {
    EneObj *e = ENE(o);
    s32 pad;
    f32 d;
    f32 x;
    f32 y;
    f32 z;

    if (e->model->kind == gFrmKindAnime) {
        x = ((AnimeFrame *) e->model)->pos[0];
        y = ((AnimeFrame *) e->model)->pos[1];
        z = ((AnimeFrame *) e->model)->pos[2];
    } else if (e->model->kind == gFrmKindPol) {
        x = e->x;
        y = e->y;
        z = e->z;
    } else {
        PRINTF("フレーム がありません\n");
    }
    d = sqrtf((tx - x) * (tx - x) + (ty - y) * (ty - y) + (tz - z) * (tz - z));
    if (d == 0.0f) {
        VEL(e)->x = 0.0f;
        VEL(e)->y = 0.0f;
        VEL(e)->z = 0.0f;
        return 1;
    }
    VEL(e)->x = (tx - x) / d * speed / 1;
    VEL(e)->y = (ty - y) / d * speed / 1;
    VEL(e)->z = (tz - z) / d * speed / 1;
    if ((tx - x) * (tx - (x + VEL(e)->x)) < 0.0f || (ty - y) * (ty - (y + VEL(e)->y)) < 0.0f ||
        (tz - z) * (tz - (z + VEL(e)->z)) < 0.0f) {
        return 1;
    }
    return 0;
}

/* Turns the enemy towards (tx, tz) from its model by deg degrees a step; 1 once it faces it. */
s32 eneTurnToPoint(Obj *o, f32 tx, f32 tz, f32 deg) {
    EneObj *e = ENE(o);
    f32 a;
    s32 pad;
    f32 x;
    f32 z;
    f32 h;

    if (e->model->kind == gFrmKindAnime) {
        x = ((AnimeFrame *) e->model)->pos[0];
        z = ((AnimeFrame *) e->model)->pos[2];
        h = ((AnimeFrame *) e->model)->rot[1];
    } else if (e->model->kind == gFrmKindPol) {
        x = ((PolFrame *) e->model)->pos[0];
        z = ((PolFrame *) e->model)->pos[2];
        h = ((PolFrame *) e->model)->rot[1];
    } else {
        PRINTF("フレーム がありません\n");
    }
    a = mathAtan2(tx - x, tz - z);
    if (a - h > 3.1415927f) {
        h += 6.2831855f;
    } else if (a - h < -3.1415927f) {
        h -= 6.2831855f;
    }
    if (fabsf(a - h) < deg * DEG2RAD) {
        e->rot[1] = a;
        return 1;
    }
    if (a < h) {
        h -= deg * DEG2RAD;
    } else {
        h += deg * DEG2RAD;
    }
    e->rot[1] = h;
    return 0;
}

/* Turns the enemy towards the heading deg (degrees) by step degrees; 1 once it faces it.
 * a is made first in the source (IDO moves it after the model tests; written there, a and h
 * swap $f2 and $f14). */
s32 eneTurnToAngle(Obj *o, f32 deg, f32 stepdeg) {
    EneObj *e = ENE(o);
    f32 a;
    f32 h;

    a = deg * DEG2RAD;
    if (e->model->kind == gFrmKindAnime) {
        h = ((AnimeFrame *) e->model)->rot[1];
    } else if (e->model->kind == gFrmKindPol) {
        h = ((PolFrame *) e->model)->rot[1];
    } else {
        PRINTF("フレーム がありません\n");
    }
    if (a - h > 3.1415927f) {
        h += 6.2831855f;
    } else if (a - h < -3.1415927f) {
        h -= 6.2831855f;
    }
    if (fabsf(a - h) < stepdeg * DEG2RAD) {
        e->rot[1] = a;
        return 1;
    }
    if (a < h) {
        h -= stepdeg * DEG2RAD;
    } else {
        h += stepdeg * DEG2RAD;
    }
    e->rot[1] = h;
    return 0;
}

/* Sets the enemy's heading, in degrees. */
void eneSetAngle(Obj *o, f32 deg) {
    ENE(o)->rot[1] = deg * DEG2RAD;
}

/* The five below read the velocity through VEL (named fields). The last two "フレーム がありません"
 * strings (gEneprocDebugStr + 0x1F4 and + 0x20C) are likely the dead prints of E844 and E968's model
 * tests, but an else with the print there moves their registers (v0 and v1 swap); they match without. */

/* Which of the first two players is nearest the enemy's model, within r: 1, 2 (0: neither).
 * The distances add the squares z first, as bmove.c's BmoveX and plshot.c write a distance from
 * a frame's place (IDO evaluates them x, y, z, as the game does). */
s32 eneNearerPl12(Obj *o, f32 r) {
    EneObj *e = ENE(o);
    s32 pad;
    f32 x;
    f32 y;
    f32 z;
    f32 d0;
    f32 d;
    s32 n = 0;

    d0 = 0.0f;
    if (plIsInPlay(0)) {
        plGetModelPos(gGameWork.players[0], &x, &y, &z);
        d0 = sqrtf((z - ((AnimeFrame *) e->model)->pos[2]) * (z - ((AnimeFrame *) e->model)->pos[2]) +
                   ((y - ((AnimeFrame *) e->model)->pos[1]) * (y - ((AnimeFrame *) e->model)->pos[1]) +
                    (x - ((AnimeFrame *) e->model)->pos[0]) * (x - ((AnimeFrame *) e->model)->pos[0])));
        if (d0 <= r) {
            n = 1;
        }
    }
    if (plIsInPlay(1)) {
        plGetModelPos(gGameWork.players[1], &x, &y, &z);
        d = sqrtf((z - ((AnimeFrame *) e->model)->pos[2]) * (z - ((AnimeFrame *) e->model)->pos[2]) +
                  ((y - ((AnimeFrame *) e->model)->pos[1]) * (y - ((AnimeFrame *) e->model)->pos[1]) +
                   (x - ((AnimeFrame *) e->model)->pos[0]) * (x - ((AnimeFrame *) e->model)->pos[0])));
        if (n == 0 || d < d0) {
            if (d <= r) {
                n = 2;
            }
        }
    }
    return n;
}

/* Moves the enemy along its course towards x at speed; 1 (and stopped) once there. */
s32 eneMoveToX(Obj *o, f32 x, f32 speed) {
    EneObj *e = ENE(o);

    if (e->x < x) {
        VEL(e)->x = speed;
    } else if (x < e->x) {
        VEL(e)->x = speed * -1.0f;
    } else {
        VEL(e)->x = 0.0f;
    }
    if ((x - e->x) * (x - (e->x + VEL(e)->x)) <= 0.0f) {
        VEL(e)->x = 0.0f;
        VEL(e)->z = 0.0f;
        return 1;
    }
    return 0;
}

/* The same towards (x, z) at (sx, sz); 1 once there. */
s32 eneMoveToXZ(Obj *o, f32 x, f32 z, f32 sx, f32 sz) {
    EneObj *e = ENE(o);

    if (e->x < x) {
        VEL(e)->x = sx;
    } else if (x < e->x) {
        VEL(e)->x = sx * -1.0f;
    } else {
        VEL(e)->x = 0.0f;
    }
    if (e->z < z) {
        VEL(e)->z = sz;
    } else if (z < e->z) {
        VEL(e)->z = sz * -1.0f;
    } else {
        VEL(e)->z = 0.0f;
    }
    if ((x - e->x) * (x - (e->x + VEL(e)->x)) <= 0.0f) {
        VEL(e)->x = 0.0f;
    }
    if ((z - e->z) * (z - (e->z + VEL(e)->z)) <= 0.0f) {
        VEL(e)->z = 0.0f;
    }
    if (VEL(e)->x == 0.0f && VEL(e)->z == 0.0f) {
        return 1;
    }
    return 0;
}

/* Turns a moving enemy (8 degrees a step) to face the way it moves. */
void eneFaceMoveDir(Obj *o) {
    EneObj *e = ENE(o);
    s32 pad;
    f32 mx;
    f32 mz;
    f32 x;
    f32 z;

    if (VEL(e)->x != 0.0f || VEL(e)->z != 0.0f) {
        if (e->model->kind == gFrmKindAnime) {
            mx = ((AnimeFrame *) e->model)->pos[0];
            mz = ((AnimeFrame *) e->model)->pos[2];
        } else if (e->model->kind == gFrmKindPol) {
            mx = ((PolFrame *) e->model)->pos[0];
            mz = ((PolFrame *) e->model)->pos[2];
        }
        (*vramCoursePointXZ)(e->zone, e->x + VEL(e)->x, e->z + VEL(e)->z, &x, &z);
        eneTurnToAngle(o, (*mathAtan2)(x - mx, z - mz) * 180.0f / 3.1415927f, 8.0f);
    }
}

/* Faces a moving enemy away from the way it moves, at once. */
void eneFaceAwayMove(Obj *o) {
    EneObj *e = ENE(o);
    s32 pad;
    f32 mx;
    f32 mz;
    f32 x;
    f32 z;
    s32 pad2;

    if (VEL(e)->x != 0.0f || VEL(e)->z != 0.0f) {
        if (e->model->kind == gFrmKindAnime) {
            mx = ((AnimeFrame *) e->model)->pos[0];
            mz = ((AnimeFrame *) e->model)->pos[2];
        } else if (e->model->kind == gFrmKindPol) {
            mx = ((PolFrame *) e->model)->pos[0];
            mz = ((PolFrame *) e->model)->pos[2];
        }
        (*vramCoursePointXZ)(e->zone, e->x + VEL(e)->x, e->z + VEL(e)->z, &x, &z);
        e->rot[1] = (*mathAtan2)((x - mx) * -1.0f, (z - mz) * -1.0f);
    }
}
