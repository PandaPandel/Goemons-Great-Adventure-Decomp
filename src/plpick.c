#include "player.h"
#include "fade.h"
#include "impact.h"
#include "mathutil.h"
#include "plmain.h"
#include "plmove.h"
#include "plpick.h"
#include "plutil.h"
#include "swwork.h"
#include "tmanager.h"
#include "vram.h"

/* The player's next action from the buttons and the ground (overlay 9), and its turning. Each
 * picker returns an action, or -1 to keep the one on. B (0x4000) is the weapon, Z (0x2000) the
 * sub-weapon, R (0x10) the character's own move, A (0x8000) the jump. */

/* A player's state in main (10 bytes each). */
typedef struct PlStat {
    /* 0x0 */ s8 pips;
    /* 0x1 */ s8 armor; /* the second pip row (game.h's Unk88282.armor) */
    /* 0x2 */ s16 money; /* compared with each character's cost */
    /* 0x4 */ u8 pad4[6];
} PlStat;

/* Character 1's honi out, a bit each (plhoni.c). */
#define PL_HONI(p) ((u16 *) &(p)->chain)

extern PlStat gSavePlayerPips[];
/* plpick.c's .data (0x801A7640-0x801A7670). */
f32 gPlFallSpeedTable[4] = { 2.0f, 2.0f, 4.0f, 2.0f }; /* per character: a fall speed */
s32 gPlSubWeaponCostTable[4] = { 1, 1, 2, 2 }; /* per character: what Z costs */
s32 gPlChargeCostTable[4] = { 3, 3, 5, 5 }; /* per character: what the charged move costs */


/* The weapon (B), the sub-weapon (Z), the charged move, or the character's own. */
s32 plPickAttack(Player *p) {
    s32 next = -1;

    if (plWantsWeapon(p)) {
        if (plStickForward(p)) {
            next = plWeaponAction(p, 3);
        } else {
            next = plWeaponAction(p, 0);
        }
    } else if (plWantsSubWeapon(p)) {
        if (plStickForward(p)) {
            next = 0x81;
        } else {
            next = 0x7E;
        }
    } else if (plWantsChargedMove(p)) {
        if (plStickForward(p)) {
            next = 0x86;
        } else {
            next = 0x83;
        }
    } else if (plWantsOwnMove(p)) {
        next = plPickOwnMove(p);
    }
    return next;
}

/* On the ground. */
s32 plPickGround(Player *p) {
    s32 next = -1;
    s32 v;

    if (plIsInWater(p)) {
        next = 0x19;
    } else if (!plIsOnGround(p)) {
        next = 0xF;
    } else if (plWantsJump(p)) {
        next = 0xE;
    } else if ((v = plPickAttack(p)) != -1) {
        next = v;
    } else if (plWantsTurnBack(p)) {
        next = 6;
    } else if (plIsOnSlope(p)) {
        next = plPickSlope(p);
    }
    return next;
}

s32 plPickGroundOrWalk(Player *p) {
    s32 next = plPickGround(p);

    if (next == -1) {
        next = plPickWalk(p);
    }
    return next;
}

s32 plSetGroundAction(Player *p) {
    s32 next;
    s32 changed;

    plStickSpeed(p);
    next = plPickGroundWrap(p);
    if (next == 0x19) {
        changed = plActChangeAfter(p, next, 3);
    } else {
        changed = plActChange(p, next);
    }
    return changed;
}

s32 plPickGroundWrap(Player *p) {
    return plPickGroundOrWalk(p);
}

/* The same, in the air. */
s32 plPickAirAttack(Player *p) {
    s32 next = -1;

    if (plWantsWeapon(p)) {
        if (plStickForward(p)) {
            next = plWeaponAction(p, 4);
        } else {
            next = plWeaponAction(p, 1);
        }
    } else if (plWantsSubWeapon(p)) {
        if (plStickForward(p)) {
            next = 0x82;
        } else {
            next = 0x7F;
        }
    } else if (plWantsChargedMove(p)) {
        if (plStickForward(p)) {
            next = 0x87;
        } else {
            next = 0x84;
        }
    } else if (plWantsOwnMove(p)) {
        next = plPickOwnMove(p);
    }
    return next;
}

s32 plPickAttackNoFwd(Player *p) {
    s32 next = -1;

    if (plWantsWeapon(p)) {
        next = plWeaponAction(p, 2);
    } else if (plWantsSubWeapon(p)) {
        next = 0x80;
    } else if (plWantsChargedMove(p)) {
        next = 0x85;
    } else if (plWantsOwnMove(p)) {
        next = plPickOwnMove(p);
    }
    return next;
}

/* In the air: landing, the air moves, falling. */
s32 plPickAir(Player *p) {
    s32 next = -1;
    s32 v;
    f32 vy;
    f32 fall;

    plStickSpeed(p);
    if (p->action != 0xE && plIsInWater(p)) {
        next = 0x19;
    } else if (plIsOnGround(p)) {
        if (p->action == 0xE) {
            if (!(p->edgeFlags & 0xC000) || p->actionSteps >= 2) {
                next = plPickLanding(p);
            }
        } else {
            next = plPickLanding(p);
        }
    } else if ((v = plPickAirAttack(p)) != -1) {
        next = v;
    } else {
        vy = p->velY;
        fall = gPlFallSpeedTable[p->chara];
        if (vy < -fall) {
            next = 0xF;
        } else if (p->action == 0xE && fabsf(vy) < fall) {
            next = 0x10;
        }
    }
    return next;
}

s32 plPickMoveOrAttack(Player *p) {
    s32 v;
    s32 next = -1;

    if (plWantsJump(p)) {
        next = 0xE;
    } else if (plWantsWeapon(p)) {
        if (plStickForward(p)) {
            next = plWeaponAction(p, 3);
        } else {
            next = plWeaponAction(p, 0);
        }
    } else if (plWantsSubWeapon(p)) {
        if (plStickForward(p)) {
            next = 0x81;
        } else {
            next = 0x7E;
        }
    } else if (plWantsChargedMove(p)) {
        if (plStickForward(p)) {
            next = 0x86;
        } else {
            next = 0x83;
        }
    } else if (plWantsOwnMove(p)) {
        next = plPickOwnMove(p);
    } else if (plWantsTurnBack(p)) {
        next = 6;
    } else if ((v = plPickWalk(p)) != 0) {
        next = v;
    }
    return next;
}

s32 plPickGroundNoStand(Player *p) {
    s32 next = plPickGroundOrWalk(p);

    if (next == 0) {
        next = -1;
    }
    return next;
}

/* The stick sets the speed: across and up in 3D (flag 0x10), else across. */
void plStickSpeed(Player *p) {
    f32 v;

    if (p->flags & 0x10) {
        p->pushZ = plStickSpeedX(p);
        v = plStickSpeedY(p);
    } else {
        v = plStickSpeedX(p);
    }
    p->pushX = v;
}

/* The walk by speed (0.05, 0.7, 1.2, 1.7: actions 0-4). */
s32 plPickWalk(Player *p) {
    f32 speed = sqrtf(p->pushZ * p->pushZ + p->pushX * p->pushX);
    s32 next;

    if (speed < 0.05f) {
        next = 0;
    } else if (speed < 0.7f) {
        next = 1;
    } else if (speed < 1.2f) {
        next = 2;
    } else if (speed < 1.7f) {
        next = 3;
    } else {
        next = 4;
    }
    return (next == p->action) ? -1 : next;
}

s32 plPickTurnBack(Player *p) {
    return plWantsTurnBack(p) ? 6 : -1;
}

/* Turning back: the stick hard against the facing, on the ground (or Z in 3D). */
s32 plWantsTurnBack(Player *p) {
    f32 v = plStickToSpeed(p, plMoveStickY(p->index, 0));

    if (p->flags & 0x10) {
        if (p->buttons & 0x2000) {
            return TRUE;
        }
    } else if (v < -1.2f && plIsOnGround(p)) {
        return TRUE;
    }
    return FALSE;
}

/* Whether there is no ground 8 below the next step. */
s32 plNoGroundAhead(Player *p) {
    Vec3f from;
    Vec3f dir;
    Vec3f v;
    Vec3f at;
    f32 t;

    vramCoursePointXZ(p->zone, p->place, p->side, &from.x, &from.z);
    from.y = p->height + 8.0f;
    plFormSizeGet(p, 0, &v);
    dir.y = v.y - 8.0f;
    dir.x = dir.z = 0.0f;
    return (vramRayGround(&from, &dir, &at, &t) == NULL) ? TRUE : FALSE;
}

s32 plPickJump(Player *p) {
    return plWantsJump(p) ? 0xE : -1;
}

/* A jump: A pressed, on the ground or in water, and not busy. */
s32 plWantsJump(Player *p) {
    if (!(p->flags & 0x2020) && (p->held & A_BUTTON) &&
        (plIsOnGround(p) || plIsInWater(p)) &&
        (u32) gGameWork.frames >= 4) {
        return TRUE;
    }
    return FALSE;
}

s32 plPickStandIfGrounded(Player *p) {
    return plIsOnGround(p) ? 0 : -1;
}

s32 plIsOnGround(Player *p) {
    if (p->ground != 0) {
        return TRUE;
    }
    return FALSE;
}

/* In water, off the ground. */
s32 plIsInWater(Player *p) {
    return (p->waterObj != 0 && !plIsOnGround(p)) ? TRUE : FALSE;
}

/* Standing or on a slope, with the slope flags set. The tail is an if/else of two returns: the
 * else's TRUE goes above the test and the then-arm's FALSE into the delay slot of a `b` to the
 * very next instruction. */
s32 plIsOnSlope(Player *p) {
    if (p->action != 0 && p->action != 0x7A && p->action != 0x7D) {
        return FALSE;
    }
    if (!plIsOnGround(p)) {
        return FALSE;
    }
    if (!(p->edgeFlags & 0xC000)) {
        return FALSE;
    } else {
        return TRUE;
    }
}

s32 plPickSlope(Player *p) {
    return (p->edgeFlags & 0x8000) ? 0x7A : 0x7D;
}

/* B, not busy. */
s32 plWantsWeapon(Player *p) {
    if (!(p->flags & 0x3020) && (p->held & 0x4000)) {
        return TRUE;
    }
    return FALSE;
}

/* Z, if the player can pay for it (or flag 0x800). */
s32 plWantsSubWeapon(Player *p) {
    s32 flags = p->flags;

    if (!(flags & 0x3030) && (p->held & 0x2000) &&
        (gSavePlayerPips[p->index].money >= gPlSubWeaponCostTable[p->chara] || (flags & 0x800))) {
        return TRUE;
    }
    return FALSE;
}

/* The charged move: Z held since the count reached 31, and paid for. */
s32 plWantsChargedMove(Player *p) {
    if (!(p->flags & 0x3030) && plIsCharged(p) &&
        (plMoveGetButtons(p->index, 1) & 0x2000) && !(p->buttons & 0x2000) &&
        gSavePlayerPips[p->index].money >= gPlChargeCostTable[p->chara]) {
        return TRUE;
    }
    return FALSE;
}

s32 plIsCharged(Player *p) {
    return (p->charge >= 31) ? TRUE : FALSE;
}

/* The stick pushed forward hard (in 2D). The test, both results and the return on one line
 * (IDO schedules by line). */
s32 plStickForward(Player *p) {
    s32 fwd;
    f32 v = plStickToSpeed(p, plMoveStickY(p->index, 0));

    if (v > 1.4f && !(p->flags & 0x10)) { fwd = TRUE; } else { fwd = FALSE; } return fwd;
}

/* The character's own move (R). */
s32 plWantsOwnMove(Player *p) {
    switch (p->chara) {
        case 0:
            if ((p->held & 0x10) && !(p->flags & 0x10)) {
                return TRUE;
            }
            break;
        case 1:
            if ((p->held & 0x10) && !(p->flags & 0x10) && PL_HONI(p)[0] == 0) {
                return TRUE;
            }
            break;
        case 2:
        case 3:
            return plCanDive(p);
    }
    return FALSE;
}

s32 plPickOwnMove(Player *p) {
    switch (p->chara) {
        case 0:
            if (swWorkTest(0xC2) && (p->held & 0x10)) {
                if (p->flags & 0x4000) {
                    return 0x5F;
                }
                if ((p->flags & 0x100000) ||
                    (p->action == 6 && plAnimPercent(p) >= 31)) {
                    return 0x62;
                }
                return 0x5C;
            }
            break;
        case 1:
            if (swWorkTest(0xC6) && (p->held & 0x10) && PL_HONI(p)[0] == 0) {
                if (p->flags & 0x8000) {
                    return 0x67;
                }
                if (p->flags & 0x100000) {
                    return 0x66;
                }
                return 0x65;
            }
            break;
        case 2:
            if (plCanDive(p)) {
                return 0x53;
            }
            break;
        case 3:
            if (plCanDive(p)) {
                return 0x43;
            }
            break;
    }
    return -1;
}

/* Diving (R in water; Sasuke also needs item 0xCB). */
s32 plCanDive(Player *p) {
    if ((p->held & 0x10) && !(p->flags & 0x20000010) && plIsInWater(p) &&
        (u32) gGameWork.frames >= 6) {
        if (p->chara == 2) {
            if (swWorkTest(0xCB)) {
                return TRUE;
            }
        } else {
            return TRUE;
        }
    }
    return FALSE;
}

/* Coming up out of a dive (R, not too deep). */
s32 plCanSurface(Player *p) {
    if (p->chara != 0 && p->chara != 1 && (p->chara == 2 || p->chara == 3) &&
        (p->held & 0x10) && (p->flags & 0x10000) && (u32) gGameWork.frames >= 6 &&
        (p->waterObj == 0 || p->waterDepth < 26.0f)) {
        return TRUE;
    }
    return FALSE;
}

s32 plPickStandAtAnimEnd(Player *p) {
    return plAnimEnded(p) ? 0 : -1;
}

/* Landing: hard after 14 steps of falling. */
s32 plPickLanding(Player *p) {
    return (p->action == 0xF && p->actionSteps >= 14) ? 0x11 : 0x12;
}

/* The idle demo, after 300 steps of standing ("__plIdleDemoPa"). */
s32 __plIdleDemoPa(Player *p) {
    if (gSavePlayerPips[p->index].pips == 1 && p->actionSteps >= 15) {
        return 0x79;
    }
    if (p->actionSteps < 300 || p->charge != 0) {
        return -1;
    }
    if (gGameWork.area == 2 && p->chara != 2 && !(p->flags & 0x10)) {
        PRINTF("__plIdleDemoPa: cold\n");
        return 0x76;
    }
    if (gGameWork.tokageSet != 0 && p->chara != 2) {
        return 0x78;
    }
    if (p->chara == 3 && fadeRand(10000) >= 5001) {
        return 0x71;
    }
    return 0x70;
}

/* Another player the player can ride on: standing or walking, within 10 across and under 15
 * below.
 * The game hoists the loop's constants into s3-s6 as 4, 5, 8, 0x80000 (the bound first), after
 * the base in s2. Skipping the player himself with a continue, after q, puts the bound there:
 * with one test (q != p in the condition) the bound ranked last, and with a continue for the
 * in-play test as well it ranked above the base. */
Player *plFindRideTarget(Player *p) {
    s32 i;
    Player *q;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            q = PL(gGameWork.players[i]);
            if (q == p) {
                continue;
            }
            if ((q->action == 5 || q->action == 8 || (q->flags & 0x80000)) &&
                fabsf(q->place - p->place) < 10.0f && p->height - q->height < 15.0f) {
                return q;
            }
        }
    }
    return NULL;
}

/* Faces the way the player moves, then turns to it by a a step. */
void plFaceMoveDir(Player *p, f32 a) {
    f32 v = p->velX;

    if (fabsf(v) > 0.05f) {
        if (v >= 0.0f) {
            p->targetFacing = 0.0f;
        } else {
            p->targetFacing = 3.1415927f;
        }
    }
    if (p->targetFacing != p->facing) {
        plTurn2(p, a);
    }
}

/* Turns the facing toward 0 or pi by a ("plTurn2"). */
void plTurn2(Player *p, f32 a) {
    s32 pad;
    f32 s;
    f32 c;

    if (p->facing != p->targetFacing) {
        if (p->facing < 0.0f) {
            s = mathSin(p->facing);
            c = mathCos(p->facing);
        } else {
            s = mathSinCos(p->facing * 0.31830990f * 512.0f, &c);
        }
        PRINTF("plTurn2(): %.2f, %.2f\n", s, c);
        if (p->facing != p->targetFacing) {
            if (fabsf(p->targetFacing) < 0.7853982f) {
                p->facing += a;
                if (s >= 0.0f && c >= 0.0f) {
                    p->facing = 0.0f;
                }
            } else {
                p->facing -= a;
                if (s >= 0.0f && c < 0.0f) {
                    p->facing = 3.1415927f;
                }
            }
        }
    }
}

/* Turns the facing toward unk0F0 by a; TRUE once there. */
s32 plTurnTo(Player *p, f32 a) {
    if (sinf(p->facing - p->targetFacing) >= 0.0f) {
        p->facing -= a;
        if (sinf(p->facing - p->targetFacing) <= 0.0f) {
            p->facing = p->targetFacing;
            return TRUE;
        }
    } else {
        p->facing += a;
        if (sinf(p->facing - p->targetFacing) >= 0.0f) {
            p->facing = p->targetFacing;
            return TRUE;
        }
    }
    return FALSE;
}

/* Leans toward unk0F4 by a. */
void plLeanTo(Player *p, f32 a) {
    if (sinf(p->lean - p->targetLean) >= 0.0f) {
        p->lean -= a;
        if (sinf(p->lean - p->targetLean) <= 0.0f) {
            p->lean = p->targetLean;
        }
    } else {
        p->lean += a;
        if (sinf(p->lean - p->targetLean) >= 0.0f) {
            p->lean = p->targetLean;
        }
    }
}

/* Sends obj message 9 with a tag and two words, and returns its answer. */
s32 plSendTagMsg(Obj *obj, s32 tag, s32 a, s32 b) {
    s32 msg[3];

    msg[0] = tag;
    msg[1] = a;
    msg[2] = b;
    return tskSendMsg(obj, 9, msg);
}

/* Copies a frame's place, turn and scale. */
void plCopyAnimeFrame(AnimeFrame *dst, AnimeFrame *src) {
    if (dst != NULL && src != NULL) {
        dst->pos[0] = src->pos[0];
        dst->pos[1] = src->pos[1];
        dst->pos[2] = src->pos[2];
        dst->rot[0] = src->rot[0];
        dst->rot[1] = src->rot[1];
        dst->rot[2] = src->rot[2];
        dst->scale[0] = src->scale[0];
        dst->scale[1] = src->scale[1];
        dst->scale[2] = src->scale[2];
    }
}

/* Puts a frame at a player. */
void plPutFrameAt(s32 index, AnimeFrame *f) {
    if (plIsInPlay(index)) {
        plGetModelPos(gGameWork.players[index], &f->pos[0], &f->pos[1], &f->pos[2]);
    }
}
