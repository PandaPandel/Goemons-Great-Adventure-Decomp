#include "enemy.h"
#include "game.h"
#include "effect.h"
#include "enecommon.h"
#include "eneproc.h"
#include "plmain.h"
#include "shim.h"
#include "sndloc.h"
#include "tmanager.h"
#include "vram.h"

/* More enemy helpers (overlay 9; the prints name shimEneChangeCtrl and shimAtkKindCheck, and
 * a second eneDeathAct): swinging back and forth between two limits, the players around an
 * enemy, sounds at a model, a death by the damage box that took the hit. The debug prints are
 * compiled out; their strings are in gShimPrintStrs. */

extern s16 gFrmKindPol; /* "pol" */
extern s16 gFrmKindAnime; /* "anime" */

/* Swings the enemy back and forth along its course between min and max, accelerating by accel
 * up to maxspeed; 1 while it is outside them (turning back). ("return 1" is shared at the end
 * and the three cases in between share one "return 0", as bmove.c's patrols.) */
s32 shimEneSwingX(Obj *o, f32 min, f32 max, f32 accel, f32 maxspeed) {
    EneObj *e = ENE(o);

    if (maxspeed < e->vel[0]) {
        e->vel[0] = maxspeed;
    } else if (e->vel[0] < -maxspeed) {
        e->vel[0] = -maxspeed;
    }
    if (e->x < min) {
        e->vel[0] += accel;
    } else if (max < e->x) {
        e->vel[0] -= accel;
    } else {
        if (e->vel[0] > 0.0f) {
            e->vel[0] += accel;
        } else if (e->vel[0] < 0.0f) {
            e->vel[0] -= accel;
        } else {
            e->vel[0] = -accel;
        }
        return 0;
    }
    return 1;
}

/* The same up and down. */
s32 shimEneSwingY(Obj *o, f32 min, f32 max, f32 accel, f32 maxspeed) {
    EneObj *e = ENE(o);

    if (maxspeed < e->vel[1]) {
        e->vel[1] = maxspeed;
    } else if (e->vel[1] < -maxspeed) {
        e->vel[1] = -maxspeed;
    }
    if (e->y < min) {
        e->vel[1] += accel;
    } else if (max < e->y) {
        e->vel[1] -= accel;
    } else {
        if (e->vel[1] > 0.0f) {
            e->vel[1] += accel;
        } else if (e->vel[1] < 0.0f) {
            e->vel[1] -= accel;
        } else {
            e->vel[1] = -accel;
        }
        return 0;
    }
    return 1;
}

/* The nearest player (0-3, flat, on the enemy's course's terms); players not in play are 10000
 * away. (Both calls through their pointers, as the game makes them; e taken in the loop; two
 * pad words below the locals.) */
s32 shimNearestPl(Obj *o) {
    s32 i;
    s32 n;
    f32 x;
    f32 y;
    f32 z;
    f32 d[4];
    f32 min;
    s32 zone;
    EneObj *e;
    s32 pad[2];

    for (i = 0; i < 4; i++) {
        if ((*plIsInPlay)(i)) {
            e = ENE(o);
            (*plAskPos)(gGameWork.players[i], &x, &y, &z, &zone);
            d[i] = sqrtf((x - e->x) * (x - e->x) + (z - e->z) * (z - e->z));
        } else {
            d[i] = 10000.0f;
        }
    }
    n = 0;
    min = d[0];
    for (i = 1; i < 4; i++) {
        if (d[i] < min) {
            min = d[i];
            n = i;
        }
    }
    return n;
}

/* Whether the players in play are within r of the enemy (a box) on its course: 0 none, 1 some,
 * 2 all. A player off the courses goes to shimPlNearCheckWorld. (bit initialised first and all declared
 * below zone: the game's li order and slots. The enemy's coordinate in one reused local c, set as
 * its own statement, and the distance in one reused d: c takes f0 and d f2, as the game; with the
 * coordinate read in place they swap. e set at its declaration, two pad words.) */
s32 shimPlNearCheck(Obj *o, f32 r) {
    s32 bit = 1;
    s32 in = 0;
    f32 x;
    f32 y;
    f32 z;
    s32 zone;
    s32 all = 0xF;
    s32 i;
    EneObj *e = ENE(o);
    f32 c;
    f32 d;
    s32 pad[2];

    for (i = 0; i < 4; i++, bit *= 2) {
        if ((*plIsInPlay)(i)) {
            (*plAskPos)(gGameWork.players[i], &x, &y, &z, &zone);
            if (zone == 0) {
                return shimPlNearCheckWorld(o, r);
            }
            if (zone == e->zone) {
                c = e->x;
                if (c < x) {
                    d = x - c;
                } else {
                    d = -(x - c);
                }
                if (d <= r) {
                    c = e->y;
                    if (c < y) {
                        d = y - c;
                    } else {
                        d = -(y - c);
                    }
                    if (d <= r) {
                        c = e->z;
                        if (c < z) {
                            d = z - c;
                        } else {
                            d = -(z - c);
                        }
                        if (d <= r) {
                            in |= bit;
                        }
                    }
                }
            }
        } else {
            all &= ~bit;
        }
    }
    if (in == 0) {
        return 0;
    }
    if (in == all) {
        return 2;
    }
    return 1;
}

/* The same in the world. Every player counts as the first (sic: bit 1 each time). (in and all
 * declared first, four pad words below, plIsInPlay through its pointer.) */
s32 shimPlNearCheckWorld(Obj *o, f32 r) {
    s32 in = 0;
    s32 all = 0xF;
    f32 x;
    f32 y;
    f32 z;
    f32 ex;
    f32 ey;
    f32 ez;
    s32 i;
    s32 pad[4];

    vramCoursePointXZ(ENE(o)->zone, ENE(o)->x, ENE(o)->z, &ex, &ez);
    ey = ENE(o)->y;
    for (i = 0; i < 4; i++) {
        if ((*plIsInPlay)(i)) {
            plGetModelPos(gGameWork.players[i], &x, &y, &z);
            if (((ex < x) ? x - ex : -(x - ex)) <= r) {
                if (((ey < y) ? y - ey : -(y - ey)) <= r) {
                    if (((ez < z) ? z - ez : -(z - ez)) <= r) {
                        in |= 1;
                    }
                }
            }
        } else {
            all &= ~1;
        }
    }
    if (in == 0) {
        return 0;
    }
    if (in == all) {
        return 2;
    }
    return 1;
}


/* Which way player n is along the enemy's course: 1 ahead, 4 behind (0: not in play). */
u8 shimPlSideCheck(Obj *o, s32 n) {
    s32 pad[2];
    f32 x;
    f32 y;
    f32 z;
    s32 zone;
    u8 way = 0;

    if ((*plIsInPlay)(n)) {
        (*plAskPos)(gGameWork.players[n], &x, &y, &z, &zone);
        way = (x <= ENE(o)->x) ? 4 : 1;
    }
    return way;
}

/* A sound at a model (SoundLocation2: near 0, far 300). */
void shimSdAtModel(Frame *f, u16 se) {
    if (f->kind == gFrmKindAnime) {
        (*SoundIncam2)(se, gGameWork.view, ((AnimeFrame *) f)->pos[0], ((AnimeFrame *) f)->pos[1],
                                  ((AnimeFrame *) f)->pos[2], 0.0f, 300.0f);
        return;
    }
    if (f->kind == gFrmKindPol) {
        (*SoundIncam2)(se, gGameWork.view, ((PolFrame *) f)->pos[0], ((PolFrame *) f)->pos[1],
                                  ((PolFrame *) f)->pos[2], 0.0f, 300.0f);
    }
}

/* eneDeathAct by a damage box that took a hit: cut in two ("kill_hahen", with both lists; sound
 * 0x21F at the hit), blown off (sound 0x22D), or the smoke; then the reward for the player
 * whose attack it was. The calls through pointers (all but soundIncamCam) and r, i declared
 * before reward give the game's calls and frame; the attacker read once into a local "by" (as
 * eneproc.c's eneDeathAct) keeps hitPower in a1 and reward in v1, as the game. */
void shimEneDeathAct(Obj *o, ACollFrame *box, u32 dl, u32 dl2) {
    s32 r;
    s32 i;
    s32 reward = 0;
    Obj *by;

    if ((*acollDmgWasHit)(box) == 1) {
        PRINTF("eneDeathAct(): %d 人斬り\n", gGameWork.kills);
        gGameWork.kills++;
        switch (box->hitKind) {
            case 4:
            case 5:
            case 6:
                if (dl != 0 && dl2 != 0) {
                    soundIncamCam(0x21F, box->hitPos[0], box->hitPos[1], box->hitPos[2]);
                    (*eneMakeKillHahen)(o, box, dl, dl2);
                }
                reward = 1;
                break;
            case 18:
            case 19:
            case 20:
                soundIncamCam(0x22D, box->hitPos[0], box->hitPos[1], box->hitPos[2]);
                (*eneBlowOff)(o, box);
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
                (*effectBurst)(((AnimeFrame *) ENE(o)->model)->pos[0],
                                          ((AnimeFrame *) ENE(o)->model)->pos[1] + 5.0f,
                                          ((AnimeFrame *) ENE(o)->model)->pos[2]);
                break;
        }
    }
    by = (Obj *) box->hitPower;
    if (by != NULL) {
        r = -1;
        if (reward) {
            for (i = 0; i < 4; i++) {
                if (by == gGameWork.players[i]) {
                    PRINTF("plD %d = 0x%x, atkD = 0x%x\n", i, gGameWork.players[i], by);
                    r = PL(gGameWork.players[i])->index;
                }
            }
            (*eneKillReward)(o, r, 0);
            PRINTF("plnum %d", r);
        }
    }
}

/* shimAtkKindCheck: how hard a hit is (0-3) by its attack's kind; an unknown kind is 1. */
s32 shimAtkKindCheck(ACollFrame *box) {
    switch (box->hitKind) {
        case 0:
            return 0;
        case 1:
        case 4:
        case 18:
        case 28:
        case 29:
            return 1;
        case 2:
        case 5:
        case 12:
        case 16:
        case 19:
        case 30:
            return 2;
        case 3:
        case 6:
        case 8:
        case 20:
        case 31:
            return 3;
        default:
            return 1;
    }
}

/* Sets the enemy's two low flag bits. */
void shimEneSetLowFlags(Obj *o, u8 n) {
    ENE(o)->flags &= ~3;
    ENE(o)->flags |= n;
}

/* Whether n is one of 120-126. */
s32 shimIsKind120To126(s32 n) {
    switch (n) {
        case 120:
        case 121:
        case 122:
        case 123:
        case 124:
        case 125:
        case 126:
            return 1;
        case 100:
        default:
            return 0;
    }
}

/* shimEneChangeCtrl: message 9 to player p, with ctrl. */
void shimEneChangeCtrl(s32 ctrl, s32 p) {
    s32 arg[3];
    s32 pad[2];

    PRINTF("shimEneChangeCtrl : ctrl [%x]\n", ctrl);
    arg[0] = ctrl;
    arg[1] = 0;
    arg[2] = 0;
    (*tskSendMsg)(gGameWork.players[p], 9, arg);
}

s32 shimPlGetState(Player *p) {
    return p->state;
}

s32 shimPlGetHitKind(Player *p) {
    return p->hitKind;
}

/* Its print names shimAtkKindCheck, but its string follows shimEneChangeCtrl's in the game's
 * .rodata, and IDO emits strings in source order: it was printed here, after the control
 * change, not in shimAtkKindCheck. */
void shimPlSetHitKind(Player *p, s32 v) {
    PRINTF("shimAtkKindCheck: unknown kind[%d]\n", v);
    p->hitKind = v;
}

/* Moves the enemy by its speed, lands it and checks its walls (eneWallCheckNoPush): its hit flags.
 * As enecommon.c's eneMoveAndCollide. */
void shimEneMove(Obj *o, f32 h, f32 r, f32 a, f32 b) {
    EneObj *e = ENE(o);
    s32 wall;

    eneMove(e);
    e->hit &= ~7;
    if ((*eneLand)(e, h)) {
        e->hit |= 1;
    }
    wall = eneWallCheckNoPush(o, h, r, a, b);
    if (wall < 0) {
        e->hit |= 2;
    } else if (wall == 0) {
        e->hit |= 4;
    }
}
