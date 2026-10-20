#include "player.h"
#include "managerpl.h"
#include "mathutil.h"
#include "model.h"
#include "ombupl.h"
#include "plbox.h"
#include "plmodel.h"
#include "plonigiri.h"
#include "plpick.h"
#include "plshot.h"
#include "plsound.h"
#include "plutil.h"
#include "tmanager.h"

/* The piggyback ("ombupl", overlay 9). One player rides on another's back: the rider's actions
 * ("OMBU_U") and the carrier's ("OMBU_D"). They talk by message 4 (plActSend sends it to
 * the partner, plMsgReply answers): a = 1 asks to get off, 2 checks the partner is still
 * there, 3 moves the rider onto the carrier's back, 4 ends it. The rider's actions are the
 * rider's own numbered from the partner's character (ombuUAction). A pair's special attack
 * comes from the carrier's character (ombuDSpecial). */

/* The file's dead prints are in the two base handlers. IDO emits strings in source order even
 * inside a switch, and each handler's "invincible." comes before
 * its timeout's, so it was printed in case 0 (flag 0x400 set), not in the damage case. */

/* A bounce's work (Player.work, as plonigiri.c's). */
typedef struct PlHop {
    /* 0x0 */ f32 vx;
    /* 0x4 */ f32 height;
    /* 0x8 */ f32 bounce;
} PlHop;

/* What a special's pieces are made with. */
typedef struct OmbuInit {
    /* 0x00 */ Player *p;
    /* 0x04 */ Vec3f *pos;
    /* 0x08 */ Vec3f *data;
    /* 0x0C */ s32 a;
    /* 0x10 */ s32 b;
} OmbuInit;

typedef struct OmbuInitA {
    /* 0x00 */ Player *p;
    /* 0x04 */ Vec3f *pos;
    /* 0x08 */ Vec3f *data;
    /* 0x0C */ s32 a;
    /* 0x10 */ s32 b;
    /* 0x14 */ f32 angle;
} OmbuInitA;

typedef struct PlStat {
    /* 0x0 */ u8 pad0[3];
    /* 0x3 */ s8 wpLv;
    /* 0x4 */ u8 pad4[6];
} PlStat;

extern PlStat gPlayerStats[];

/* ombupl.c's .data (0x801A73E0-0x801A7640). */
u16 gOmbuStartSndTable[4] = { 0x0393, 0x0372, 0x03B2, 0x03CF }; /* per character: a sound */
Vec3f gOmbuSpecialPosTable[16] = { /* the places of the specials, 4 per character */
    { -4.0f, 4.0f, 4.0f }, { -3.0f, 8.0f, -3.0f }, { 0.0f, 6.0f, -10.0f }, { 0.0f, 0.0f, 0.0f },
    { -5.0f, 4.0f, 5.0f }, { -6.0f, 8.0f, -2.0f }, { -2.0f, 5.0f, -10.0f }, { 0.0f, 0.0f, 0.0f },
    { 0.0f, 4.0f, -1.0f }, { 0.0f, 4.0f, -1.0f }, { 0.0f, 4.0f, -1.0f }, { 0.0f, 0.0f, 0.0f },
    { -5.5f, 7.0f, 4.0f }, { -3.0f, 13.0f, -4.0f }, { 4.5f, 8.0f, -12.0f }, { 0.0f, 0.0f, 0.0f },
};
f32 gOmbuSpecialFrameTable[4][4] = { /* the frame each special starts at */
    { 5.0f, 4.7f, 5.5f, 0.0f },
    { 3.0f, 2.5f, 3.0f, 0.0f },
    { 10.0f, 10.0f, 10.0f, 0.0f },
    { 9.3f, 8.7f, 8.2f, 0.0f },
};
Vec3f gOmbuKobanVelTable[3] = { { 0.0f, 0.0f, 5.5f }, { 0.0f, 5.5f, 0.0f }, { 0.0f, 0.0f, -5.5f } };
Vec3f gOmbuKobanSpreadTable[3] = { { 0.0f, 5.0f, 0.0f }, { 0.0f, 0.0f, 5.0f }, { 0.0f, 5.0f, 0.0f } };
Vec3f gOmbuShuriVelTable[9] = {
    { 0.0f, 0.0f, 5.5f }, { 0.0f, 5.5f, 0.0f }, { 0.0f, 0.0f, -5.5f },
    { 0.0f, 5.0f, 0.0f }, { 0.0f, 0.0f, 5.0f }, { 0.0f, 5.0f, 0.0f },
    { 0.0f, 5.0f, 0.0f }, { 0.0f, 0.0f, 5.0f }, { 0.0f, 5.0f, 0.0f },
};
Vec3f gOmbuBazookaVelTable[6] = {
    { 0.0f, 0.0f, 5.5f }, { 0.0f, 5.5f, 0.0f }, { 0.0f, 0.0f, -5.5f },
    { 0.0f, 5.0f, 0.0f }, { 0.0f, 0.0f, 5.0f }, { 0.0f, 5.0f, 0.0f },
};
void (*gOmbuSpecialFuncTable[4])(Player *p, s32 i) = { /* the special of each carrier */
    ombuSpecialCh0, ombuSpecialCh1, ombuSpecialCh2, ombuSpecialCh3,
};
s32 gOmbuAttackByLvTable[6][3] = { /* actions by weapon level */
    { 0xB6, 0xB9, 0xBC }, { 0xB6, 0xB9, 0xBC }, { 0xB6, 0xB9, 0xBC },
    { 0xB7, 0xBA, 0xBD }, { 0xB7, 0xBA, 0xBD }, { 0xB8, 0xBB, 0xBE },
};

void eneGetWorldPos(s32 obj, f32 *x); /* FAKE: enecommon.h is not included here: eneGetWorldPos: the game's code calls it as this file declares it; enecommon.h has `void eneGetWorldPos(Obj *o, Vec3f *out)` */


/* Climbing on: the rider's first action. */
s32 ombuUStart(Player *p, s32 partner) {
    p->partner = partner;
    return ombuUAction(p, 0x89);
}

/* Taking a rider on: the carrier's first action. */
s32 ombuDStart(Player *p, s32 partner) {
    plDeleteLater(p);
    p->partner = partner;
    plSound(p, gOmbuStartSndTable[p->chara]);
    return 0xAD;
}

/* The rider's default ("OMBU_U_BASE"). */
void OMBU_U_BASE(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            PRINTF("invincible.\n");
            p->flags |= 0x400;
            break;
        case 1:
            PRINTF("ombupl: %d: Damage (1)\n", p->index);
            if (p->flags & 0x400) {
                if (ombuURideCheck(p) || !plActChange(p, 0)) {
                    PRINTF("OMBU_U_BASE: partner is dead\n");
                    plFaceMoveDir(p, 0.5235988f);
                    plActSend(plPartnerGet(p), 4, 3, p);
                }
                PRINTF("OMBU_U_BASE: timed out\n");
                return;
            }
            break;
        case 2:
            p->rideHold = 0;
            p->flags &= ~0x400;
            break;
        case 4:
            switch (a) {
                case 1:
                    plMsgReply(b, 1);
                    plActChange(p, 0);
                    return;
                case 2:
                default:
                    plMsgReply(b, 0);
                    return;
            }
        case 3:
            if (plIsInvincible(p) && p->hitKind >= 0x76) {
                return;
            }
            switch (p->hitKind) {
                case 0x65:
                case 0x6A:
                    return;
                case 0x78:
                case 0x7B:
                    p->damage = 1;
                    plActChange(p, ombuUAction(p, 0xA9));
                    return;
                case 0x7C:
                case 0x7E:
                    p->damage = -1;
                    p->weaponDamage = -1;
                    plActChange(p, ombuUAction(p, 0xA9));
                    return;
            }
            break;
    }
    plBaseDefault(p, msg, a, b);
}

void ombuUStandStep(Player *p, s32 msg, s32 a, s32 b) {
    if (msg == 1) {
        plStickSpeed(p);
        plActChange(p, ombuUNextAction(p));
    }
    OMBU_U_BASE(p, msg, a, b);
}

void ombuUStandCh0(Player *p, s32 msg, s32 a, s32 b) {
    ombuUStandStep(p, msg, a, b);
}

void ombuUStandCh1(Player *p, s32 msg, s32 a, s32 b) {
    ombuUStandStep(p, msg, a, b);
}

void ombuUStandCh2(Player *p, s32 msg, s32 a, s32 b) {
    ombuUStandStep(p, msg, a, b);
}

void ombuUStandCh3(Player *p, s32 msg, s32 a, s32 b) {
    ombuUStandStep(p, msg, a, b);
}

void ombuUWalkStep(Player *p, s32 msg, s32 a, s32 b) {
    if (msg == 1) {
        plStickSpeed(p);
        plActChange(p, ombuUNextAction(p));
    }
    OMBU_U_BASE(p, msg, a, b);
}

void ombuUWalk1Ch0(Player *p, s32 msg, s32 a, s32 b) {
    ombuUWalkStep(p, msg, a, b);
}

void ombuUWalk1Ch1(Player *p, s32 msg, s32 a, s32 b) {
    ombuUWalkStep(p, msg, a, b);
}

void ombuUWalk1Ch2(Player *p, s32 msg, s32 a, s32 b) {
    ombuUWalkStep(p, msg, a, b);
}

void ombuUWalk1Ch3(Player *p, s32 msg, s32 a, s32 b) {
    ombuUWalkStep(p, msg, a, b);
}

void ombuUWalk2Ch0(Player *p, s32 msg, s32 a, s32 b) {
    ombuUWalkStep(p, msg, a, b);
}

void ombuUWalk2Ch1(Player *p, s32 msg, s32 a, s32 b) {
    ombuUWalkStep(p, msg, a, b);
}

void ombuUWalk2Ch2(Player *p, s32 msg, s32 a, s32 b) {
    ombuUWalkStep(p, msg, a, b);
}

void ombuUWalk2Ch3(Player *p, s32 msg, s32 a, s32 b) {
    ombuUWalkStep(p, msg, a, b);
}

void ombuUWalk3Ch0(Player *p, s32 msg, s32 a, s32 b) {
    ombuUWalkStep(p, msg, a, b);
}

void ombuUWalk3Ch1(Player *p, s32 msg, s32 a, s32 b) {
    ombuUWalkStep(p, msg, a, b);
}

void ombuUWalk3Ch2(Player *p, s32 msg, s32 a, s32 b) {
    ombuUWalkStep(p, msg, a, b);
}

void ombuUWalk3Ch3(Player *p, s32 msg, s32 a, s32 b) {
    ombuUWalkStep(p, msg, a, b);
}

void ombuUWalk4Ch0(Player *p, s32 msg, s32 a, s32 b) {
    ombuUWalkStep(p, msg, a, b);
}

void ombuUWalk4Ch1(Player *p, s32 msg, s32 a, s32 b) {
    ombuUWalkStep(p, msg, a, b);
}

void ombuUWalk4Ch2(Player *p, s32 msg, s32 a, s32 b) {
    ombuUWalkStep(p, msg, a, b);
}

void ombuUWalk4Ch3(Player *p, s32 msg, s32 a, s32 b) {
    ombuUWalkStep(p, msg, a, b);
}

/* The rider's bounce: 10 high, across by 40 from what hit it. */
void ombuUHit(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;
    PlHop *w = (PlHop *) p->work;
    Player *partner;
    s32 pad[2];
    f32 x;

    switch (msg) {
        case 0:
            if (plDamage(p, p->damage, p->weaponDamage)) {
                if (plIsInWater(p)) {
                    next = 0x2C;
                } else {
                    next = 0x2B;
                }
                if ((partner = plPartnerGet(p)) != NULL) {
                    plActSend(partner, 4, 4, p);
                }
                plActChange(p, next);
                return;
            }
            w->height = 10.0f;
            w->bounce = 1.0f;
            if (p->hitBy != 0) {
                eneGetWorldPos(p->hitBy, &x);
                p->pushX = w->vx = ((p->model->pos[0] <= x) ? -40.0f : 40.0f) - p->velX;
            } else {
                w->vx = 0.0f;
            }
            p->jumpPush = plJumpSpeed(w->height) - p->velY;
            plVoiceDamage(p);
            plInvincibleSet(p, 0x4B);
            plBlinkSet(p, 0x4B);
            break;
        case 1:
            plActChange(p, ombuUNextAction(p));
            if (!(p->flags & 0x400)) {
                return;
            }
            break;
        case 2:
            break;
    }
    OMBU_U_BASE(p, msg, a, b);
}

void ombuUHitCh0(Player *p, s32 msg, s32 a, s32 b) {
    ombuUHit(p, msg, a, b);
}

void ombuUHitCh1(Player *p, s32 msg, s32 a, s32 b) {
    ombuUHit(p, msg, a, b);
}

void ombuUHitCh2(Player *p, s32 msg, s32 a, s32 b) {
    ombuUHit(p, msg, a, b);
}

void ombuUHitCh3(Player *p, s32 msg, s32 a, s32 b) {
    ombuUHit(p, msg, a, b);
}

/* The rider's jump: A held keeps it going up. */
void ombuUJump(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            p->jumpPush = p->jumpSpeed;
            plLandSound(p);
            break;
        case 1:
            switch (p->state) {
                case 0:
                    if (!(p->buttons & A_BUTTON)) {
                        p->state++;
                        p->jumpPush = -(p->velY + -1.0714285f) - -1.0714285f * p->jumpCut;
                    }
                    if ((f32) p->phase == p->jumpHoldSteps - 1.0f) {
                        p->state++;
                    }
                    break;
                case 1:
                    break;
            }
            p->phase++;
            plActChange(p, ombuUJumpNext(p));
            break;
    }
    OMBU_U_BASE(p, msg, a, b);
}

void ombuUJumpCh0(Player *p, s32 msg, s32 a, s32 b) {
    ombuUJump(p, msg, a, b);
}

void ombuUJumpCh1(Player *p, s32 msg, s32 a, s32 b) {
    ombuUJump(p, msg, a, b);
}

void ombuUJumpCh2(Player *p, s32 msg, s32 a, s32 b) {
    ombuUJump(p, msg, a, b);
}

void ombuUJumpCh3(Player *p, s32 msg, s32 a, s32 b) {
    ombuUJump(p, msg, a, b);
}

void ombuUJumpTop(Player *p, s32 msg, s32 a, s32 b) {
    if (msg == 1) {
        plStickSpeed(p);
        plActChange(p, ombuUNextAction(p));
    }
    OMBU_U_BASE(p, msg, a, b);
}

void ombuUJumpTopCh0(Player *p, s32 msg, s32 a, s32 b) {
    ombuUJumpTop(p, msg, a, b);
}

void ombuUJumpTopCh1(Player *p, s32 msg, s32 a, s32 b) {
    ombuUJumpTop(p, msg, a, b);
}

void ombuUJumpTopCh2(Player *p, s32 msg, s32 a, s32 b) {
    ombuUJumpTop(p, msg, a, b);
}

void ombuUJumpTopCh3(Player *p, s32 msg, s32 a, s32 b) {
    ombuUJumpTop(p, msg, a, b);
}

void ombuUFall(Player *p, s32 msg, s32 a, s32 b) {
    if (msg == 1) {
        plStickSpeed(p);
        plActChange(p, ombuUNextAction(p));
    }
    OMBU_U_BASE(p, msg, a, b);
}

void ombuUFallCh0(Player *p, s32 msg, s32 a, s32 b) {
    ombuUFall(p, msg, a, b);
}

void ombuUFallCh1(Player *p, s32 msg, s32 a, s32 b) {
    ombuUFall(p, msg, a, b);
}

void ombuUFallCh2(Player *p, s32 msg, s32 a, s32 b) {
    ombuUFall(p, msg, a, b);
}

void ombuUFallCh3(Player *p, s32 msg, s32 a, s32 b) {
    ombuUFall(p, msg, a, b);
}

/* The carrier's default ("OMBU_D_BASE"). The rider sits on the carrier's model, at its
 * animation's attachment point. */
void OMBU_D_BASE(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;
    f32 x, y, z;
    s32 pad;
    f32 rot;

    switch (msg) {
        case 0:
            PRINTF("invincible.\n");
            p->flags |= 0x803;
            plFormSizeSet(p, 3);
            p->side = plPartnerGet(p)->side;
            return;
        case 1:
            if (p->flags & 0x800) {
                if (p->held & A_BUTTON) {
                    plActSend(plPartnerGet(p), 4, 1, p);
                    if (plMsgReplyTake(p)) {
                        next = 0xE;
                    }
                }
                if (plIsInWater(p)) {
                    plActSend(plPartnerGet(p), 4, 1, p);
                    if (plMsgReplyTake(p)) {
                        next = 0x19;
                    }
                }
                if ((plPartnerGet(p)->flags & 0x200) && plActChange(p, 0)) {
                    return;
                }
                if (p->rideHold == 0) {
                    if (plActChange(p, 0)) {
                        PRINTF("OMBU_D_BASE: timed out\n");
                        return;
                    }
                } else {
                    if (p->rideHold >= 4) {
                        p->rideHold = 3;
                    }
                    p->rideHold--;
                }
                plActChange(p, next);
                return;
            }
            break;
        case 2:
            p->flags &= ~0x803;
            plFormSizeSet(p, 0);
            p->rideHold = 1;
            p->side = p->startSide;
            return;
        case 4:
            switch (a) {
                case 2:
                    p->rideHold = 3;
                    plMsgReply(b, 1);
                    return;
                case 3:
                    mdlAnimNodePos(plActAnimeGet((Player *) b)->attach,
                                           ((Player *) b)->model->frame,
                                           ((Player *) b)->model->segs[0], &x, &y, &z);
                    p->place = ((Player *) b)->place;
                    p->height = ((Player *) b)->height;
                    p->side = ((Player *) b)->side;
                    p->velX = ((Player *) b)->velX;
                    p->velY = ((Player *) b)->velY;
                    p->velZ = ((Player *) b)->velZ;
                    p->pushX = ((Player *) b)->pushX;
                    p->pushY = ((Player *) b)->pushY;
                    p->velZ = ((Player *) b)->pushZ;
                    p->lean = ((Player *) b)->lean;
                    p->facing = ((Player *) b)->facing;
                    p->roll = ((Player *) b)->roll;
                    p->targetFacing = ((Player *) b)->targetFacing;
                    p->leanOffset = ((Player *) b)->leanOffset;
                    p->tilt = ((Player *) b)->tilt;
                    p->rollOffset = ((Player *) b)->rollOffset;
                    p->zone = ((Player *) b)->zone;
                    if (p->flags & 0x10) {
                        rot = ((Player *) b)->model->rot[1];
                        p->place -= x * 0.1f * cosf(rot) - z * 0.1f * sinf(rot);
                        p->side += x * 0.1f * sinf(rot) + z * 0.1f * cosf(rot);
                    } else {
                        p->place += z * 0.1f * cosf(p->facing) - x * 0.1f * sinf(p->facing);
                        p->side += z * 0.1f * sinf(p->facing) + x * 0.1f * cosf(p->facing);
                    }
                    p->height += y * 0.1f;
                    plMsgReply(b, 1);
                    return;
                case 4:
                    plActChange(p, 0);
                    return;
            }
            return;
        case 3:
            if (plIsInvincible(p) && p->hitKind >= 0x76) {
                return;
            }
            if (p->hitKind == 0x65 || p->hitKind == 0x6A) {
                return;
            }
            break;
    }
    plBaseDefault(p, msg, a, b);
}

void ombuDMoveStep(Player *p, s32 msg, s32 a, s32 b) {
    if (msg == 1) {
        plActChange(p, ombuDNextAction(p));
    }
    OMBU_D_BASE(p, msg, a, b);
}

void ombuDStand(Player *p, s32 msg, s32 a, s32 b) {
    ombuDMoveStep(p, msg, a, b);
}

void ombuDWalk1(Player *p, s32 msg, s32 a, s32 b) {
    ombuDMoveStep(p, msg, a, b);
}

void ombuDWalk2(Player *p, s32 msg, s32 a, s32 b) {
    ombuDMoveStep(p, msg, a, b);
}

void ombuDWalk3(Player *p, s32 msg, s32 a, s32 b) {
    ombuDMoveStep(p, msg, a, b);
}

void ombuDWalk4(Player *p, s32 msg, s32 a, s32 b) {
    ombuDMoveStep(p, msg, a, b);
}

void ombuDActB2(Player *p, s32 msg, s32 a, s32 b) {
    ombuDMoveStep(p, msg, a, b);
}

void ombuDActB3(Player *p, s32 msg, s32 a, s32 b) {
    ombuDMoveStep(p, msg, a, b);
}

void ombuDActB4(Player *p, s32 msg, s32 a, s32 b) {
    ombuDMoveStep(p, msg, a, b);
}

/* The carrier hit. */
void ombuDHit(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            if (plDamage(p, p->damage, p->weaponDamage)) {
                plActChange(p, plIsInWater(p) ? 0x2C : 0x2B);
                return;
            }
            plVoiceDamage(p);
            plInvincibleSet(p, 100);
            plBlinkSet(p, 0x4B);
            break;
        case 1:
            plActChange(p, ombuDNextAction(p));
            break;
        case 2:
            plInvincibleSet(p, 100);
            break;
    }
    OMBU_D_BASE(p, msg, a, b);
}

/* The carrier's attack by weapon level (n: its animation). */
void ombuDAttack(Player *p, s32 msg, s32 a, s32 b, s32 n) {
    s32 next;
    s32 t;

    switch (msg) {
        case 0:
            p->flags |= 0x800;
            plAttackSound(p);
            plBoxSetKind(p, n);
            break;
        case 1:
            next = -1;
            if ((t = plAnimPercent(p)) >= 60) {
                next = ombuDNextAttack(p);
                if (next == -1 && t == 100) {
                    next = ombuDWalkBySpeed(p);
                }
            }
            plActSwitch(p, next);
            return;
        case 2:
            plBoxDelete(p);
            break;
    }
    OMBU_D_BASE(p, msg, a, b);
}

void ombuDAttackLv0(Player *p, s32 msg, s32 a, s32 b) {
    ombuDAttack(p, msg, a, b, 5);
}

void ombuDAttackLv1(Player *p, s32 msg, s32 a, s32 b) {
    ombuDAttack(p, msg, a, b, 0x10);
}

void ombuDAttackLv2(Player *p, s32 msg, s32 a, s32 b) {
    ombuDAttack(p, msg, a, b, 0x1B);
}

void ombuDAttackLv0Fwd(Player *p, s32 msg, s32 a, s32 b) {
    ombuDAttack(p, msg, a, b, 6);
}

void ombuDAttackLv1Fwd(Player *p, s32 msg, s32 a, s32 b) {
    ombuDAttack(p, msg, a, b, 0x11);
}

void ombuDAttackLv2Fwd(Player *p, s32 msg, s32 a, s32 b) {
    ombuDAttack(p, msg, a, b, 0x1C);
}

void ombuDAttackLv0Back(Player *p, s32 msg, s32 a, s32 b) {
    ombuDAttack(p, msg, a, b, 7);
}

void ombuDAttackLv1Back(Player *p, s32 msg, s32 a, s32 b) {
    ombuDAttack(p, msg, a, b, 0x12);
}

void ombuDAttackLv2Back(Player *p, s32 msg, s32 a, s32 b) {
    ombuDAttack(p, msg, a, b, 0x1D);
}

/* Whether special i's frame has come. */
s32 ombuSpecialFrameCheck(Player *p, s32 i) {
    return (gOmbuSpecialFrameTable[p->chara][i] <= p->model->frame) ? TRUE : FALSE;
}

/* Character 0's special: two pieces, either side of the place. */
void ombuSpecialCh0(Player *p, s32 i) {
    s32 pad;
    OmbuInit init;
    Vec3f b;
    Vec3f a;

    init.p = p;
    init.data = &gOmbuKobanVelTable[i];
    init.a = 1;
    init.b = i;
    a = gOmbuSpecialPosTable[i];
    b = gOmbuSpecialPosTable[i];
    init.pos = &a;
    a.x += gOmbuKobanSpreadTable[i].x * 0.5f;
    a.y += gOmbuKobanSpreadTable[i].y * 0.5f;
    a.z += gOmbuKobanSpreadTable[i].z * 0.5f;
    tskCreateBlocks(gGameWork.players[p->index], koban, 2, 0x1A0,
                           &init);
    init.pos = &b;
    b.x -= gOmbuKobanSpreadTable[i].x * 0.5f;
    b.y -= gOmbuKobanSpreadTable[i].y * 0.5f;
    b.z -= gOmbuKobanSpreadTable[i].z * 0.5f;
    tskCreateBlocks(gGameWork.players[p->index], koban, 2, 0x1A0,
                           &init);
    plSound(p, 0x134);
    p->state = 1;
}

/* Character 1's special. */
void ombuSpecialCh1(Player *p, s32 i) {
    s32 pad;
    OmbuInit init;

    init.p = p;
    init.data = &gOmbuShuriVelTable[i];
    init.a = 0;
    init.b = 0;
    init.pos = &gOmbuSpecialPosTable[4 + i];
    tskCreate(gGameWork.players[p->index], hshuri, 0x1A5, &init);
    plSound(p, 0x210);
    plVoice(p, 0x531);
    p->state = 1;
}

/* Character 2's special: eight pieces in a fan, pi/6 apart. */
void ombuSpecialCh2(Player *p, s32 i) {
    OmbuInitA init;
    Vec3f v;
    s32 k;
    f32 angle;

    angle = -0.5235988f;
    init.p = p;
    init.a = 1;
    init.b = 0;
    init.pos = &gOmbuSpecialPosTable[8 + i];
    init.data = &v;
    for (k = 0; k < 8; k++) {
        init.angle = angle;
        v.x = 0.0f;
        v.y = mathSin(angle) * 5.5f;
        v.z = mathCos(angle) * 5.5f;
        tskCreate(gGameWork.players[p->index], kunai, 0x1AA,
                               &init);
        angle += 0.5235988f;
    }
    plSound(p, 0x21C);
    p->state = 1;
}

/* Character 3's special. */
void ombuSpecialCh3(Player *p, s32 i) {
    s32 pad;
    OmbuInit init;

    init.p = p;
    init.data = &gOmbuBazookaVelTable[i];
    init.a = 1;
    init.b = 0;
    init.pos = &gOmbuSpecialPosTable[12 + i];
    tskCreate(gGameWork.players[p->index], bazooka, 0x1AF, &init);
    plVoice(p, 0x556);
    p->state = 1;
}

/* The pair's special (i: its step), by the carrier's character. */
void ombuDSpecial(Player *p, s32 msg, s32 a, s32 b, s32 i) {
    s32 next = -1;
    void (*special)(Player *p, s32 i);

    if (msg == 1) {
        if (p->state == 0 && ombuSpecialFrameCheck(p, i)) {
            special = gOmbuSpecialFuncTable[p->chara];
            if (special != NULL) {
                special(p, i);
            }
        }
        if (plAnimEnded(p)) {
            next = ombuDWalkBySpeed(p);
        }
        plActChange(p, next);
    }
    OMBU_D_BASE(p, msg, a, b);
}

void ombuDSpecialNeutral(Player *p, s32 msg, s32 a, s32 b) {
    ombuDSpecial(p, msg, a, b, 0);
}

void ombuDSpecialFwd(Player *p, s32 msg, s32 a, s32 b) {
    ombuDSpecial(p, msg, a, b, 1);
}

void ombuDSpecialBack(Player *p, s32 msg, s32 a, s32 b) {
    ombuDSpecial(p, msg, a, b, 2);
}

/* The rider's walk by speed. */
s32 ombuUWalkBySpeed(Player *p) {
    f32 speed = plPushSize(p);
    s32 next;

    if (speed < 0.05f) {
        next = ombuUAction(p, 0x89);
    } else if (speed < 0.7f) {
        next = ombuUAction(p, 0x8D);
    } else if (speed < 1.2f) {
        next = ombuUAction(p, 0x91);
    } else if (speed < 1.7f) {
        next = ombuUAction(p, 0x95);
    } else {
        next = ombuUAction(p, 0x99);
    }
    return (next == p->action) ? -1 : next;
}

s32 ombuUNextAction(Player *p) {
    s32 next = ombuUWalkBySpeed(p);

    if (!plIsOnGround(p)) {
        next = ombuUAction(p, 0xA5);
    } else if (plWantsJump(p)) {
        next = ombuUAction(p, 0x9D);
    }
    return next;
}

/* Whether the ride goes on: the partner answered, or the hold (at most 3 steps) has not run
 * out. */
s32 ombuURideCheck(Player *p) {
    plActSend(plPartnerGet(p), 4, 2, p);
    if (!plMsgReplyTake(p)) {
        if (p->rideHold == 0) {
            return FALSE;
        }
        if (p->rideHold >= 4) {
            p->rideHold = 3;
        }
        p->rideHold--;
    }
    return TRUE;
}

s32 ombuUJumpNext(Player *p) {
    s32 next = -1;

    plStickSpeed(p);
    if (plIsOnGround(p)) {
        if (p->action >= 0x9D && p->action < 0xA1) {
            if (!(p->edgeFlags & 0xC000) || p->actionSteps >= 2) {
                next = ombuUAction(p, 0x89);
            }
        } else {
            next = ombuUAction(p, 0x89);
            plLandGroundSound(p);
        }
    } else if (p->velY < -2.0f) {
        next = ombuUAction(p, 0xA5);
    } else if (p->action >= 0x9D && p->action < 0xA1 && fabsf(p->velY) < 2.0f) {
        next = ombuUAction(p, 0xA1);
    }
    return next;
}

/* The carrier's walk by speed (0xAD-0xB1). */
s32 ombuDWalkBySpeed(Player *p) {
    f32 speed = plPushSize(p);
    s32 next;

    if (speed < 0.05f) {
        next = 0xAD;
    } else if (speed < 0.7f) {
        next = 0xAE;
    } else if (speed < 1.2f) {
        next = 0xAF;
    } else if (speed < 1.7f) {
        next = 0xB0;
    } else {
        next = 0xB1;
    }
    return (next == p->action) ? -1 : next;
}

s32 ombuWeaponCheck(Player *p) {
    return plWantsWeapon(p);
}

s32 ombuSubWeaponCheck(Player *p) {
    return plWantsSubWeapon(p);
}

void ombuDummy(void) {
}

s32 ombuStickFwdCheck(Player *p) {
    return plStickForward(p);
}

/* Whether the stick points back hard (under -1.2, the way the player faces). The else and the
 * return on one line (IDO schedules by line). */
s32 ombuStickBackCheck(Player *p) {
    f32 v = plStickToSpeed(p, plMoveStickX(p->index, 0));
    s32 back;
    f32 k;

    if (p->targetFacing > 1.5707963f) {
        k = -1.0f;
    } else {
        k = 1.0f;
    }
    if (v * k < -1.2f) {
        back = TRUE;
    } else { back = FALSE; } return back;
}

s32 ombuDAttackByLv(Player *p, s32 i) {
    return gOmbuAttackByLvTable[i][gPlayerStats[p->index].wpLv];
}

/* The carrier's next action from the buttons. */
s32 ombuDNextAction(Player *p) {
    s32 next = ombuDWalkBySpeed(p);

    if (ombuWeaponCheck(p)) {
        next = ombuDAttackByLv(
            p, ombuStickFwdCheck(p) ? 3 : (ombuStickBackCheck(p) ? 5 : 0));
    } else if (ombuSubWeaponCheck(p)) {
        next = ombuStickFwdCheck(p) ? 0xC0 : (ombuStickBackCheck(p) ? 0xC1 : 0xBF);
    }
    return (next == p->action) ? -1 : next;
}

s32 ombuDNextAttack(Player *p) {
    s32 next = -1;

    if (ombuWeaponCheck(p)) {
        next = ombuDAttackByLv(
            p, ombuStickFwdCheck(p) ? 3 : (ombuStickBackCheck(p) ? 5 : 0));
    } else if (ombuSubWeaponCheck(p)) {
        next = ombuWeaponCheck(p) ? 0xC0 : (ombuStickBackCheck(p) ? 0xC1 : 0xBF);
    }
    return next;
}

/* The rider's action base plus the partner's character. */
s32 ombuUAction(Player *p, s32 base) {
    return plPartnerGet(p)->chara + base;
}
