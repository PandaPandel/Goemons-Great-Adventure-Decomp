#include "player.h"
#include "demopl.h"
#include "enemy.h"
#include "mathutil.h"
#include "plact.h"
#include "pldive.h"
#include "plpick.h"
#include "plsound.h"
#include "plutil.h"
#include "vram.h"

/* The demo player ("demopl.c", overlay 9): the player's actions while a demo drives it. The
 * demo sends message 6 with a command (0x1010-0x1017) and each action answers it through
 * plMsgReply and picks the next action. Also the walking and jumping helpers the demo
 * uses: walking to a place, falling to death, and struggling free of Konaki-jiji. */

/* What the player stands on (its task's work). */
typedef struct PlRide {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ u8 pad02[0xE];
    /* 0x10 */ f32 place;
    /* 0x14 */ f32 height;
    /* 0x18 */ f32 side;
} PlRide;

/* Struggling free (Player.work). */
typedef struct PlStruggle {
    /* 0x0 */ s32 dir; /* the last shake's side */
    /* 0x4 */ s32 t; /* steps since the last shake */
    /* 0x8 */ s32 n; /* shakes */
} PlStruggle;

/* Walking straight on (Player.work). */
typedef struct PlStraight {
    /* 0x0 */ f32 vx;
    /* 0x4 */ f32 vz;
} PlStraight;

#define FX(obj) ((void *) ((u32) (obj) + sizeof(Obj)))

#pragma intrinsic (fabs)


void pldemoRide1Base(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            p->flags |= 2;
            p->flags2 |= 2;
            break;
        case 1:
            break;
        case 2:
            p->flags &= ~2;
            p->flags2 &= ~2;
            break;
        case 3:
            return;
    }
    plBaseDefault(p, msg, a, b);
}

/* Falls to death 100 below the rail. */
void pldemoRide1FallCheck(Player *p, s32 msg, s32 a, s32 b) {
    f32 x;
    f32 y;

    switch (msg) {
        case 0:
            break;
        case 1:
            vramCoursePoint(p->zone, p->place, p->height, &x, &y, &x);
            if (p->height < y - 100.0f) {
                PRINTF("demopl.c : 落ち死に\n");
                plActChange(p, 0x2E);
                p->ride = NULL;
            }
            break;
        case 2:
            break;
        case 3:
            return;
    }
    pldemoRide1Base(p, msg, a, b);
}

void pldemoRide1IdlePa(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 6:
            switch (*(s32 *) a) {
                case 0x1010:
                    if (p->held & B_BUTTON) {
                        plActChange(p, 0xCD);
                        plMsgReply(b, 3);
                        return;
                    }
                    if (pldemoStickX(p) != 0.0f) {
                        plActChange(p, 0xCC);
                        plMsgReply(b, 2);
                        p->tilt = 0.0f;
                        return;
                    }
                    if ((p->held & A_BUTTON) && pldemoStickY(p) > 0.0f) {
                        plActChange(p, 0xD5);
                        plMsgReply(b, 7);
                        return;
                    }
                    break;
                case 0x1011:
                    plActChange(p, 0xCC);
                    plMsgReply(b, 2);
                    return;
                case 0x1015:
                    plActChange(p, 0xCE);
                    plMsgReply(b, 5);
                    return;
                case 0x1017:
                    plInvincibleSet(p, 0x4B);
                    plActChange(p, 0xD5);
                    plMsgReply(b, 6);
                    return;
            }
            plMsgReply(b, 1);
            break;
        case 1:
            if (p->ride != NULL) {
                pldemoRideAlong(p, p->ride);
            }
            break;
    }
    pldemoRide1FallCheck(p, msg, a, b);
}

void pldemoRide1StickPa(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 6:
            switch (*(s32 *) a) {
                case 0x1011:
                    if (p->held & B_BUTTON) {
                        plActChange(p, 0xCD);
                        plMsgReply(b, 3);
                        return;
                    }
                    if (pldemoStickX(p) == 0.0f) {
                        plActChange(p, 0xCB);
                        plMsgReply(b, 1);
                        return;
                    }
                    if ((p->held & A_BUTTON) && pldemoStickY(p) > 0.0f) {
                        plActChange(p, 0xD5);
                        plMsgReply(b, 7);
                        return;
                    }
                    break;
                case 0x1010:
                    plActChange(p, 0xCB);
                    plMsgReply(b, 1);
                    return;
                case 0x1015:
                    plActChange(p, 0xCE);
                    plMsgReply(b, 5);
                    return;
                case 0x1017:
                    plInvincibleSet(p, 0x4B);
                    plActChange(p, 0xD5);
                    plMsgReply(b, 6);
                    return;
            }
            plMsgReply(b, 2);
            break;
        case 1:
            if (p->ride != NULL) {
                pldemoRideAlong(p, p->ride);
            }
            break;
    }
    pldemoRide1FallCheck(p, msg, a, b);
}

void pldemoRide1WeaponPa(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 6:
            switch (*(s32 *) a) {
                case 0x1013:
                    plActChange(p, 0xCB);
                    return;
                case 0x1014:
                    plActChange(p, 0xCB);
                    plMsgReply(b, 1);
                    return;
                case 0x1017:
                    plInvincibleSet(p, 0x4B);
                    plActChange(p, 0xD5);
                    plMsgReply(b, 6);
                    return;
                default:
                case 0x1012:
                    plMsgReply(b, 3);
                    break;
            }
            break;
        case 1:
            if (p->ride != NULL) {
                pldemoRideAlong(p, p->ride);
            }
            break;
    }
    pldemoRide1FallCheck(p, msg, a, b);
}

void pldemoRide1HoldPa(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 1:
            break;
        case 6:
            if (*(s32 *) a == 0x1016) {
                plActChange(p, 0xCB);
                plMsgReply(b, 1);
                return;
            }
            plMsgReply(b, 5);
            break;
    }
    pldemoRide1FallCheck(p, msg, a, b);
}

/* The stick across, with a dead zone. */
f32 pldemoStickX(Player *p) {
    return pldemoStickDeadZone(plMoveStickX(p->index, 0));
}

/* The stick up, with a dead zone. */
f32 pldemoStickY(Player *p) {
    return pldemoStickDeadZone(plMoveStickY(p->index, 0));
}

/* A stick reading: nothing under 20, then 0.03 a step. */
f32 pldemoStickDeadZone(s32 v) {
    if (fabs(v) < 20.0) {
        v = 0;
    }
    return v * 0.03f;
}

void pldemoRide2Base(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            p->flags |= 0xA2;
            p->flags2 |= 2;
            break;
        case 1:
            break;
        case 2:
            p->flags &= ~0xA2;
            p->flags2 &= ~2;
            break;
        case 3:
            return;
    }
    plBaseDefault(p, msg, a, b);
}

void pldemoRide2FallCheck(Player *p, s32 msg, s32 a, s32 b) {
    f32 x;
    f32 y;

    switch (msg) {
        case 0:
            break;
        case 1:
            vramCoursePoint(p->zone, p->place, p->height, &x, &y, &x);
            if (p->height < y - 100.0f) {
                PRINTF("demopl.c : 落ち死に\n");
                plActChange(p, 0x2E);
                p->ride = NULL;
            }
            break;
        case 2:
            break;
        case 3:
            return;
    }
    pldemoRide2Base(p, msg, a, b);
}

void pldemoRide2IdlePa(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 6:
            switch (*(s32 *) a) {
                case 0x1010:
                    if (p->held & B_BUTTON) {
                        plActChange(p, 0xD1);
                        plMsgReply(b, 3);
                        p->tilt = 0.0f;
                        return;
                    }
                    if ((p->held & A_BUTTON) && pldemoStickY(p) > 0.0f) {
                        plActChange(p, 0xD5);
                        plMsgReply(b, 7);
                        p->tilt = 0.0f;
                        return;
                    }
                    break;
                case 0x1011:
                    plActChange(p, 0xD0);
                    plMsgReply(b, 2);
                    p->tilt = 0.0f;
                    return;
                case 0x1015:
                    p->tilt = 0.0f;
                    return;
                case 0x1017:
                    plInvincibleSet(p, 0x4B);
                    plActChange(p, 0xD5);
                    plMsgReply(b, 6);
                    p->tilt = 0.0f;
                    return;
            }
            plMsgReply(b, 1);
            break;
        case 1:
            if (p->ride != NULL) {
                pldemoRideAlong(p, p->ride);
            }
            break;
    }
    pldemoRide2FallCheck(p, msg, a, b);
    if (p->targetFacing == 0.0f) {
        p->tilt = -0.5235988f;
    } else {
        p->tilt = 0.5235988f;
    }
}

void pldemoRide2StickPa(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 6:
            switch (*(s32 *) a) {
                case 0x1011:
                    if (p->held & B_BUTTON) {
                        plActChange(p, 0xD1);
                        plMsgReply(b, 3);
                        return;
                    }
                    if ((p->held & A_BUTTON) && pldemoStickY(p) > 0.0f) {
                        plActChange(p, 0xD5);
                        plMsgReply(b, 7);
                        return;
                    }
                    break;
                case 0x1010:
                    plActChange(p, 0xCF);
                    plMsgReply(b, 1);
                    return;
                case 0x1015:
                    return;
                case 0x1017:
                    plInvincibleSet(p, 0x4B);
                    plActChange(p, 0xD5);
                    plMsgReply(b, 6);
                    return;
            }
            plMsgReply(b, 2);
            break;
        case 1:
            if (p->ride != NULL) {
                pldemoRideAlong(p, p->ride);
            }
            break;
    }
    pldemoRide2FallCheck(p, msg, a, b);
}

void pldemoRide2WeaponPa(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 6:
            switch (*(s32 *) a) {
                case 0x1014:
                    plActChange(p, 0xCF);
                    plMsgReply(b, 1);
                    return;
                case 0x1017:
                    plInvincibleSet(p, 0x4B);
                    plActChange(p, 0xD5);
                    plMsgReply(b, 6);
                    return;
                default:
                case 0x1012:
                case 0x1013:
                    plMsgReply(b, 3);
                    break;
            }
            break;
        case 1:
            if (p->ride != NULL) {
                pldemoRideAlong(p, p->ride);
            }
            break;
    }
    pldemoRide2FallCheck(p, msg, a, b);
}

/* Rides along with what the player stands on, and turns to the stick. */
void pldemoRideAlong(Player *p, Obj *ride) {
    PlRide *w = FX(ride);
    f32 sx = pldemoStickX(p);

    p->zone = w->zone;
    p->place = w->place;
    p->height = w->height;
    p->side = w->side;
    if (fabsf(sx) > 0.05f) {
        p->targetFacing = (sx >= 0.0f) ? 0.0 : 3.141592653589793;
    }
    if (p->targetFacing != p->facing) {
        plTurn2(p, 0.5235988f);
    }
}

void pldemoRide3Base(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            p->flags |= 0x82;
            p->flags2 |= 2;
            break;
        case 1:
            break;
        case 2:
            p->flags &= ~0x82;
            p->flags2 &= ~2;
            break;
        case 3:
            return;
    }
    plBaseDefault(p, msg, a, b);
}

void pldemoRide3FallCheck(Player *p, s32 msg, s32 a, s32 b) {
    f32 x;
    f32 y;

    switch (msg) {
        case 0:
            break;
        case 1:
            vramCoursePoint(p->zone, p->place, p->height, &x, &y, &x);
            if (p->height < y - 100.0f) {
                PRINTF("demopl.c : 落ち死に\n");
                plActChange(p, 0x2E);
                p->ride = NULL;
            }
            break;
        case 2:
            break;
        case 3:
            return;
    }
    pldemoRide3Base(p, msg, a, b);
}

void pldemoRide3IdlePa(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 6:
            switch (*(s32 *) a) {
                case 0x1010:
                    if (p->held & B_BUTTON) {
                        plActChange(p, 0xD4);
                        plMsgReply(b, 3);
                        p->tilt = 0.0f;
                        return;
                    }
                    if (pldemoStickX(p) != 0.0f) {
                        plActChange(p, 0xD3);
                        plMsgReply(b, 2);
                        p->tilt = 0.0f;
                        return;
                    }
                    if (p->held & A_BUTTON) {
                        if (pldemoStickY(p) > 0.0f) {
                            plActChange(p, 0xD5);
                            plMsgReply(b, 7);
                            p->tilt = 0.0f;
                            return;
                        }
                        plMsgReply(b, 4);
                        return;
                    }
                    break;
                case 0x1015:
                    break;
                case 0x1011:
                    plActChange(p, 0xD3);
                    plMsgReply(b, 2);
                    p->tilt = 0.0f;
                    return;
                case 0x1017:
                    plInvincibleSet(p, 0x4B);
                    plActChange(p, 0xD5);
                    plMsgReply(b, 6);
                    p->tilt = 0.0f;
                    return;
            }
            plMsgReply(b, 1);
            break;
        case 1:
            if (p->ride != NULL) {
                pldemoRideAlong(p, p->ride);
            }
            break;
    }
    pldemoRide3FallCheck(p, msg, a, b);
    if (p->targetFacing == 0.0f) {
        p->tilt = -0.5235988f;
    } else {
        p->tilt = 0.5235988f;
    }
}

void pldemoRide3StickPa(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 6:
            switch (*(s32 *) a) {
                case 0x1011:
                    if (p->held & B_BUTTON) {
                        plActChange(p, 0xD4);
                        plMsgReply(b, 3);
                        return;
                    }
                    if (pldemoStickX(p) == 0.0f) {
                        plActChange(p, 0xD2);
                        plMsgReply(b, 1);
                        return;
                    }
                    if (p->held & A_BUTTON) {
                        if (pldemoStickY(p) > 0.0f) {
                            plActChange(p, 0xD5);
                            plMsgReply(b, 7);
                            return;
                        }
                        plMsgReply(b, 4);
                        return;
                    }
                    break;
                case 0x1015:
                    break;
                case 0x1010:
                    plActChange(p, 0xD2);
                    plMsgReply(b, 1);
                    return;
                case 0x1017:
                    plInvincibleSet(p, 0x4B);
                    plActChange(p, 0xD5);
                    plMsgReply(b, 6);
                    return;
            }
            plMsgReply(b, 2);
            break;
        case 1:
            if (p->ride != NULL) {
                pldemoRideAlong(p, p->ride);
            }
            break;
    }
    pldemoRide3FallCheck(p, msg, a, b);
}

void pldemoRide3WeaponPa(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 6:
            switch (*(s32 *) a) {
                case 0x1012:
                case 0x1013:
                    if (p->held & A_BUTTON) {
                        plMsgReply(b, 4);
                        return;
                    }
                    break;
                case 0x1014:
                    plActChange(p, 0xD2);
                    plMsgReply(b, 1);
                    return;
                case 0x1017:
                    plInvincibleSet(p, 0x4B);
                    plActChange(p, 0xD5);
                    plMsgReply(b, 6);
                    return;
            }
            plMsgReply(b, 3);
            break;
        case 1:
            if (p->ride != NULL) {
                pldemoRideAlong(p, p->ride);
            }
            break;
    }
    pldemoRide3FallCheck(p, msg, a, b);
}

void pldemoRideJumpPa(Player *p, s32 msg, s32 a, s32 b) {
    if (msg == 0) {
        p->flags |= 0x4000;
        p->jumpPush = p->jumpSpeed - p->velY;
        plActDefault(p, msg, a, b);
        return;
    }
    plActJump(p, msg, a, b);
}

/* The walking actions' default. */
void pldemoWalkBase(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            p->flags |= 0x20;
            plInvincibleSet(p, -1);
            break;
        case 1:
            plFaceMoveDir(p, 0.5235988f);
            return;
        case 2:
            p->flags &= ~0x20;
            plInvincibleSet(p, 0);
            break;
    }
    plBaseDefault(p, msg, a, b);
}

/* The walk for the speed: standing (under 0.05), 0xD9, 0xDA, 0xDB, running (1.7 and up).
 * -1 if it is the current action. */
s32 pldemoWalkForSpeed(Player *p) {
    f32 d = sqrtf(p->pushZ * p->pushZ + p->pushX * p->pushX);
    s32 next;

    if (d < 0.05f) {
        next = 0xD7;
    } else if (d < 0.7f) {
        next = 0xD9;
    } else if (d < 1.2f) {
        next = 0xDA;
    } else if (d < 1.7f) {
        next = 0xDB;
    } else {
        next = 0xDC;
    }
    return (next == p->action) ? -1 : next;
}

s32 pldemoWalkOrFall(Player *p) {
    s32 next = pldemoWalkForSpeed(p);

    if (!plIsOnGround(p)) {
        next = 0xDE;
    }
    return next;
}

/* Walks toward the goal: in 3D (flag 0x10) at the distance's speed, else along the rail at
 * the goal's speed. TRUE within 5. */
s32 pldemoWalkToGoal(Player *p) {
    f32 d;
    f32 dx;
    f32 dz;
    f32 ang;

    if (!(p->goal & 1)) {
        return TRUE;
    }
    if (p->flags & 0x10) {
        dx = p->goalX - p->place;
        dz = p->goalZ - p->side;
        d = plLen2D(dx, dz);
        if (fabsf(d) < 5.0f) {
            return TRUE;
        }
        ang = mathAtan2(dx, dz);
        p->pushX = mathCos(ang) * d;
        p->pushZ = mathSin(ang) * d;
    } else {
        d = p->goalX - p->place;
        if (fabsf(d) < 5.0f) {
            return TRUE;
        }
        if (d > 0.0f) {
            p->pushX = p->goalSpeed;
        } else {
            p->pushX = -p->goalSpeed;
        }
    }
    return FALSE;
}

s32 pldemoGoalStep(Player *p) {
    if ((p->goal & 1) && pldemoWalkToGoal(p)) {
        p->goal &= ~1;
        return TRUE;
    }
    plActChange(p, pldemoWalkOrFall(p));
    return FALSE;
}

void pldemoPickWalkPa(Player *p, s32 msg, s32 a, s32 b) {
    if (msg == 1 && plActChange(p, pldemoWalkOrFall(p))) {
        return;
    }
    pldemoWalkBase(p, msg, a, b);
}

void pldemoStandPa(Player *p, s32 msg, s32 a, s32 b) {
    if (msg == 1) {
        pldemoGoalStep(p);
        plBaseDefault(p, msg, a, b);
        return;
    }
    pldemoWalkBase(p, msg, a, b);
}

void pldemoWalkSlowPa(Player *p, s32 msg, s32 a, s32 b) {
    if (msg == 1 && pldemoGoalStep(p)) {
        return;
    }
    pldemoWalkBase(p, msg, a, b);
}

void pldemoWalkPa(Player *p, s32 msg, s32 a, s32 b) {
    if (msg == 1 && pldemoGoalStep(p)) {
        return;
    }
    pldemoWalkBase(p, msg, a, b);
}

void pldemoWalkFastPa(Player *p, s32 msg, s32 a, s32 b) {
    if (msg == 1 && pldemoGoalStep(p)) {
        return;
    }
    pldemoWalkBase(p, msg, a, b);
}

void pldemoRunPa(Player *p, s32 msg, s32 a, s32 b) {
    if (msg == 1 && pldemoGoalStep(p)) {
        return;
    }
    pldemoWalkBase(p, msg, a, b);
}

void pldemoWalkDefaultPa(Player *p, s32 msg, s32 a, s32 b) {
    pldemoWalkBase(p, msg, a, b);
}

s32 pldemoLandingKind(Player *p) {
    return (p->action == 0xDE && p->actionSteps >= 14) ? 0xE0 : 0xE1;
}

/* A demo jump or fall: the landing, or the fall. */
s32 pldemojumpfallsub(Player *p) {
    s32 next = -1;

    pldemoWalkToGoal(p);
    if (plIsOnGround(p)) {
        PRINTF("pldemojumpfallsub 1\n");
        if (p->action == 0xDD) {
            if (!(p->edgeFlags & 0xC000) || p->actionSteps >= 2) {
                next = pldemoLandingKind(p);
            }
        } else {
            next = pldemoLandingKind(p);
        }
    } else {
        PRINTF("pldemojumpfallsub 2n");
        if (p->velY < -2.0f) {
            next = 0xDE;
        } else if (p->action == 0xDD && fabsf(p->velY) < 2.0f) {
            next = 0xDF;
        }
    }
    return next;
}

void pldemoJumpPa(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;

    switch (msg) {
        case 0:
            p->jumpPush = p->jumpSpeed - p->velY;
            plLandSound(p);
            break;
        case 1:
            switch (p->state) {
                case 0:
                    if ((f32) p->phase == p->jumpHoldSteps - 1.0f) {
                        p->state++;
                    }
                    break;
                case 1:
                    break;
            }
            next = pldemojumpfallsub(p);
            p->phase++;
            if (plActChange(p, next)) {
                return;
            }
            break;
    }
    pldemoWalkBase(p, msg, a, b);
}

void pldemoFallPa(Player *p, s32 msg, s32 a, s32 b) {
    if (msg == 1 && plActChange(p, pldemojumpfallsub(p))) {
        return;
    }
    pldemoWalkBase(p, msg, a, b);
}

void pldemoAirPa(Player *p, s32 msg, s32 a, s32 b) {
    if (msg == 1 && plActChange(p, plPickAir(p))) {
        return;
    }
    pldemoWalkBase(p, msg, a, b);
}

void pldemoLandPa(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    switch (msg) {
        case 0:
            plLandGroundSound(p);
            break;
        case 1:
            if (plAnimEnded(p)) {
                next = 0xD7;
            }
            plActChange(p, next);
            break;
    }
    pldemoWalkBase(p, msg, a, b);
}

void pldemoLandHardPa(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    switch (msg) {
        case 0:
            plLandGroundSound(p);
            break;
        case 1:
            if (plAnimEnded(p)) {
                next = 0xD7;
            }
            plActChange(p, next);
            break;
    }
    pldemoWalkBase(p, msg, a, b);
}

void pldemoFreezePa(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            p->flags |= 0x40;
            return;
        case 2:
            p->flags &= ~0x40;
            return;
    }
}

void pldemoStillPa(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 1:
            return;
        case 0:
            p->flags |= 0x20;
            plInvincibleSet(p, -1);
            break;
        case 2:
            p->flags &= ~0x20;
            plInvincibleSet(p, 0);
            break;
    }
    plBaseDefault(p, msg, a, b);
}

void pldemoDiveWalkBase(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            p->flags |= 0x20;
            plInvincibleSet(p, -1);
            break;
        case 1:
            plFaceMoveDir(p, 0.5235988f);
            return;
        case 2:
            p->flags &= ~0x20;
            plInvincibleSet(p, 0);
            break;
    }
    diveDefault(p, msg, a, b);
}

void pldemoNopPa(Player *p, s32 msg, s32 a, s32 b) {
}

void pldemoAnim17Pa(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    if (msg == 1) {
        if (plAnimEnded(p)) {
            next = 0xD7;
        }
        plActChange(p, next);
    }
    pldemoWalkBase(p, msg, a, b);
}

void pldemoAnim68Pa(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    if (msg == 1) {
        if (plAnimEnded(p)) {
            next = 0xD7;
        }
        plActChange(p, next);
    }
    pldemoWalkBase(p, msg, a, b);
}

void pldemoAnim69Pa(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    if (msg == 1) {
        if (plAnimEnded(p)) {
            next = 0x6A;
        }
        plActChange(p, next);
    }
    pldemoWalkBase(p, msg, a, b);
}

void pldemoAnim6APa(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    if (msg == 1) {
        if (plAnimEnded(p)) {
            next = 0x6B;
        }
        plActChange(p, next);
    }
    pldemoWalkBase(p, msg, a, b);
}

void pldemoAnim6BPa(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    if (msg == 1) {
        if (plAnimEnded(p)) {
            next = 0xD7;
        }
        plActChange(p, next);
    }
    pldemoWalkBase(p, msg, a, b);
}

/* Struggling free of Konaki-jiji (its "子泣" message): five shakes of the stick, each the
 * other way and harder than 1.02, with no pause of 16 steps between them. */
void pldemoStruggle(Player *p, s32 msg, s32 a, s32 b) {
    PlStruggle *w = (PlStruggle *) p->work;
    s32 anim;
    f32 s;

    switch (msg) {
        case 0:
            w->t = 0;
            w->n = 0;
            w->dir = -1;
            break;
        case 1:
            plStickSpeed(p);
            s = plPushSize(p);
            if (s < 0.05f) {
                anim = 0xC7;
            } else if (s < 1.2f) {
                anim = 0xC8;
            } else {
                anim = 0xC9;
            }
            p->pushX *= 0.6f;
            p->animStepAdd = plPushSize(p) * 0.5f;
            if (fabsf(p->pushX) > 1.0200001f && p->pushX * w->dir < 0.0f) {
                w->dir = p->pushX;
                w->t = 0;
                if (++w->n >= 5) {
                    if (p->clinger != NULL && eneIsEnemy(p->clinger)) {
                        plSendTagMsg(p->clinger, 0xCEA5C3A6 /* 離脱 */, 0, 0);
                    }
                    plActChange(p, 0);
                    break;
                }
            } else if (w->t >= 16) {
                w->n = 0;
            }
            w->t++;
            plFaceMoveDir(p, 0.5235988f);
            if (anim != p->action) {
                plActAssign(p, anim);
            }
            return;
        case 2:
            p->clinger = NULL;
            break;
        case 6:
            if (*(s32 *) a == 0xBBD2B5E3 /* 子泣 */) {
                plMsgReply((s32) p, 1);
                return;
            }
            break;
    }
    plBaseDefault(p, msg, a, b);
}

void pldemoStruggleStillPa(Player *p, s32 msg, s32 a, s32 b) {
    pldemoStruggle(p, msg, a, b);
}

void pldemoStruggleSlowPa(Player *p, s32 msg, s32 a, s32 b) {
    pldemoStruggle(p, msg, a, b);
}

void pldemoStruggleFastPa(Player *p, s32 msg, s32 a, s32 b) {
    pldemoStruggle(p, msg, a, b);
}

/* Walking straight on the way the model faces, 1.7 a step, for 7 steps ("3d_go_strait"). */
void pldemo3dGoStraitPa(Player *p, s32 msg, s32 a, s32 b) {
    PlStraight *w = (PlStraight *) p->work;
    s32 pad;

    switch (msg) {
        case 0:
            PRINTF("3d_go_strait: init ry:%.2f\n", p->model->rot[1]);
            w->vx = mathSin(p->model->rot[1]) * 1.7f;
            w->vz = mathCos(p->model->rot[1]) * 1.7f;
            p->flags |= 0x40000000;
            break;
        case 1:
            PRINTF("3d_go_strait: %.2f\n", p->model->rot[1]);
            p->pushZ = w->vz;
            p->pushX = w->vx;
            if (p->actionSteps >= 7) {
                plActChange(p, 0);
            }
            break;
        case 2:
            p->flags &= ~0x40000000;
            p->pushZ = 0.0f;
            p->velZ = 0.0f;
            p->pushX = 0.0f;
            p->velX = 0.0f;
            break;
    }
    plBaseDefault(p, msg, a, b);
}

void pldemoNopE4Pa(Player *p, s32 msg, s32 a, s32 b) {
}

void pldemoNopE5Pa(Player *p, s32 msg, s32 a, s32 b) {
}

void pldemoNop139Pa(Player *p, s32 msg, s32 a, s32 b) {
}

void pldemoNop13APa(Player *p, s32 msg, s32 a, s32 b) {
}
