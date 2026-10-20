#include "player.h"
#include "demopl.h"
#include "ombupl.h"
#include "plact.h"
#include "plattack.h"
#include "plbase.h"
#include "plbox.h"
#include "plchain.h"
#include "plchara.h"
#include "pldefault.h"
#include "pldive.h"
#include "plhoni.h"
#include "plonigiri.h"
#include "plpick.h"
#include "plpush.h"
#include "plshot.h"
#include "plsound.h"
#include "plswim.h"
#include "plswing.h"
#include "pltalk.h"
#include "plutil.h"

/* The player's general actions (overlay 9): standing, the four walk speeds, jumping, falling,
 * landing, turning back, the weapon's three attacks, the sub-weapon, the charged move, being
 * picked up, and the idle demo. plActDefault is their default. */

typedef struct PlStat {
    /* 0x0 */ u8 pad0[3];
    /* 0x3 */ s8 wpLv;
    /* 0x4 */ u8 pad4[6];
} PlStat;

extern PlStat gPlayerStats[];
/* An action: msg 0 starts it, 1 steps it (plutil.c plActSend calls it through the table). */
typedef void PlActFunc(Player *p, s32 msg, s32 a, s32 b);

/* plchara.c */
PlActFunc plCharaJump, plChara0JumpFall, plChara1BounceFall;
/* plswing.c */
PlActFunc plSwingHang, plSwingAct0A, plSwingAct0B,
    plSwingLetGoBack, plSwingLetGoFwd;
/* plchain.c */
PlActFunc plChainStart5C, plChainWait5E, plChainEnd5D,
    plChainStart5F, plChainWait61, plChainEnd60, plChainStart62,
    plChainWait64, plChainEnd63;
/* plonigiri.c */
PlActFunc plActDamage, plActDamageEnd, plActNull,
    plActDamageWater, plActDown, plActDownWater, plActDownShrink,
    plActDownFall, plActDown2F, plActCharaSpecial, plActToStand;
/* demopl.c */
PlActFunc pldemoRide1IdlePa, pldemoRide1StickPa, pldemoRide1WeaponPa,
    pldemoRide1HoldPa, pldemoRide2IdlePa, pldemoRide2StickPa, pldemoRide2WeaponPa,
    pldemoRide3IdlePa, pldemoRide3StickPa, pldemoRide3WeaponPa, pldemoRideJumpPa,
    pldemoPickWalkPa, pldemoStandPa, pldemoWalkSlowPa, pldemoWalkPa,
    pldemoWalkFastPa, pldemoRunPa, pldemoWalkDefaultPa, pldemoJumpPa,
    pldemoFallPa, pldemoAirPa, pldemoLandPa, pldemoLandHardPa,
    pldemoFreezePa, pldemoStillPa, pldemoAnim17Pa, pldemoAnim68Pa,
    pldemoAnim69Pa, pldemoAnim6APa, pldemoAnim6BPa, pldemoStruggleStillPa,
    pldemoStruggleSlowPa, pldemoStruggleFastPa, pldemo3dGoStraitPa, pldemoNopE4Pa,
    pldemoNopE5Pa, pldemoNop139Pa, pldemoNop13APa;
/* pldive.c */
PlActFunc diveYaeDiveIn, diveYaeDiveInEnd, diveYaeSurface,
    diveSurfaceEnd, diveYaeSwim, diveYaeSwimStop, diveYaeIdle,
    diveYaeSwimStart, diveYaeDash, diveYaeDash2, diveYaeDash3,
    diveYaeDashEnd, diveYaeHit, diveYaeEat, diveYaeBazooka,
    diveYaeSwimBazooka, diveSasDiveIn, diveSasSurface, diveSasSwim,
    diveSasIdle, diveSasAttackLv0, diveSasAttackLv1, diveSasAttackLv2,
    diveSasHit, diveSasEat, diveNonDiverWait;
/* plhoni.c */
PlActFunc plHoniAct, plHoniActStand, plHoniActSwim;
/* ombupl.c */
PlActFunc ombuUStandCh0, ombuUStandCh1, ombuUStandCh2,
    ombuUStandCh3, ombuUWalk1Ch0, ombuUWalk1Ch1, ombuUWalk1Ch2,
    ombuUWalk1Ch3, ombuUWalk2Ch0, ombuUWalk2Ch1, ombuUWalk2Ch2,
    ombuUWalk2Ch3, ombuUWalk3Ch0, ombuUWalk3Ch1, ombuUWalk3Ch2,
    ombuUWalk3Ch3, ombuUWalk4Ch0, ombuUWalk4Ch1, ombuUWalk4Ch2,
    ombuUWalk4Ch3, ombuUHitCh0, ombuUHitCh1, ombuUHitCh2,
    ombuUHitCh3, ombuUJumpCh0, ombuUJumpCh1, ombuUJumpCh2,
    ombuUJumpCh3, ombuUJumpTopCh0, ombuUJumpTopCh1, ombuUJumpTopCh2,
    ombuUJumpTopCh3, ombuUFallCh0, ombuUFallCh1, ombuUFallCh2,
    ombuUFallCh3, ombuDStand, ombuDWalk1, ombuDWalk2,
    ombuDWalk3, ombuDWalk4, ombuDActB2, ombuDActB3,
    ombuDActB4, ombuDHit, ombuDAttackLv0, ombuDAttackLv1,
    ombuDAttackLv2, ombuDAttackLv0Fwd, ombuDAttackLv1Fwd, ombuDAttackLv2Fwd,
    ombuDAttackLv0Back, ombuDAttackLv1Back, ombuDAttackLv2Back, ombuDSpecialNeutral,
    ombuDSpecialFwd, ombuDSpecialBack;
/* plact.c */
PlActFunc plActStand, plActWalk1, plActWalk2,
    plActWalk3, plActWalk4, plActJump, plActFallNoRide,
    plActFall, plActAttack34, plActAttack39, plActAttack3E,
    plActAirAttack35, plActAirAttack3A, plActAirAttack3F, plActBaseAttack36,
    plActBaseAttack3B, plActBaseAttack40, plActSubWeapon37, plActSubWeapon3C,
    plActSubWeapon41, plActAirSubWeapon38, plActAirAttack3D, plActAirSubWeapon42,
    plActLandHard, plActLand, plActTurnBack, plActBaseStand,
    plActBaseWalk, plActChargeEnd, plActLetClimbOnC2, plActLetClimbOnC3,
    plActLetClimbOnC4, plActLetClimbOnC5, plActLetClimbOnC6, plActIdle70,
    plActIdle71, plActIdle72, plActIdle73, plActIdle74,
    plActIdle75, plActIdle76, plActIdle77, plActIdle78,
    plActIdle79, plActDefault6C, plActDefault6D, plActDefault6E,
    plActDefault6F, plActSlope7A, plActSlope7D, plActLastDefault7B,
    plActLastDefault7C;
/* plpush.c */
PlActFunc plActPush, plActPushStand, plActPushEnd,
    plActPushToPull, plActPullToPush, plActPull, plActPullEnd;
/* pldefault.c */
PlActFunc plActBaseDefault;
/* plswim.c */
PlActFunc plSwimPa, plSwimIdlePa, plSwimStartPa,
    plSwimStopPa, plSwimAttack0Pa, plSwimAttack1Pa, plSwimAttack2Pa,
    plSwimUpAttack0Pa, plSwimUpAttack1Pa, plSwimUpAttack2Pa, plSwimSubWeaponPa,
    plSwimUpSubWeaponPa;
/* pltalk.c */
PlActFunc plTalkProc;
/* plattack.c */
PlActFunc plAttackShot, plAttackShotAir, plAttackShotStand,
    plAttackShotFwd, plAttackShotFwdAir, plAttackCharged, plAttackChargedAir,
    plAttackChargedStand, plAttackChargedFwd, plAttackChargedFwdAir;

/* plact.c's .data (0x801A7670-0x801A7CF0). */
PlActFunc *gPlActFuncTable[315] = { /* the actions, by number (0x70-0x79: the idle demo) */
    plActStand, plActWalk1, plActWalk2, plActWalk3,
    plActWalk4, plActBaseWalk, plActTurnBack, plActChargeEnd,
    plActBaseStand, plSwingHang, plSwingAct0A, plSwingAct0B,
    plSwingLetGoBack, plSwingLetGoFwd, plActJump, plActFall,
    plActFallNoRide, plActLandHard, plActLand, plCharaJump,
    NULL, plChara0JumpFall, plChara1BounceFall, pldemoAnim17Pa,
    plSwimPa, plSwimIdlePa, plSwimStartPa, plSwimStopPa,
    plSwimAttack0Pa, plSwimAttack1Pa, plSwimAttack2Pa, plSwimUpAttack0Pa,
    plSwimUpAttack1Pa, plSwimUpAttack2Pa, plSwimSubWeaponPa, plSwimUpSubWeaponPa,
    plActPush, plActPushStand, plActPushEnd, plActPushToPull,
    plActPullToPush, plActPull, plActPullEnd, plActDown,
    plActDownWater, plActDownShrink, plActDownFall, plActDown2F,
    plActDamage, plActDamageEnd, plActNull, plActDamageWater,
    plActAttack34, plActAirAttack35, plActBaseAttack36, plActSubWeapon37,
    plActAirSubWeapon38, plActAttack39, plActAirAttack3A, plActBaseAttack3B,
    plActSubWeapon3C, plActAirAttack3D, plActAttack3E, plActAirAttack3F,
    plActBaseAttack40, plActSubWeapon41, plActAirSubWeapon42, diveYaeDiveIn,
    diveYaeDiveInEnd, diveYaeSwim, diveYaeSwimStop, diveYaeIdle,
    diveYaeSwimStart, diveYaeDash, diveYaeDash2, diveYaeDash3,
    diveYaeDashEnd, diveYaeHit, diveYaeEat, diveYaeBazooka,
    diveYaeSwimBazooka, diveYaeSurface, diveSurfaceEnd, diveSasDiveIn,
    diveSasSurface, diveSasSwim, diveSasIdle, diveSasAttackLv0,
    diveSasAttackLv1, diveSasAttackLv2, diveSasHit, diveSasEat,
    plChainStart5C, plChainEnd5D, plChainWait5E, plChainStart5F,
    plChainEnd60, plChainWait61, plChainStart62, plChainEnd63,
    plChainWait64, plHoniAct, plHoniActStand, plHoniActSwim,
    pldemoAnim68Pa, pldemoAnim69Pa, pldemoAnim6APa, pldemoAnim6BPa,
    plActDefault6C, plActDefault6D, plActDefault6E, plActDefault6F,
    plActIdle70, plActIdle71, plActIdle72, plActIdle73,
    plActIdle74, plActIdle75, plActIdle76, plActIdle77,
    plActIdle78, plActIdle79, plActSlope7A, plActLastDefault7B,
    plActLastDefault7C, plActSlope7D, plAttackShot, plAttackShotAir,
    plAttackShotStand, plAttackShotFwd, plAttackShotFwdAir, plAttackCharged,
    plAttackChargedAir, plAttackChargedStand, plAttackChargedFwd, plAttackChargedFwdAir,
    plActBaseDefault, ombuUStandCh0, ombuUStandCh1, ombuUStandCh2,
    ombuUStandCh3, ombuUWalk1Ch0, ombuUWalk1Ch1, ombuUWalk1Ch2,
    ombuUWalk1Ch3, ombuUWalk2Ch0, ombuUWalk2Ch1, ombuUWalk2Ch2,
    ombuUWalk2Ch3, ombuUWalk3Ch0, ombuUWalk3Ch1, ombuUWalk3Ch2,
    ombuUWalk3Ch3, ombuUWalk4Ch0, ombuUWalk4Ch1, ombuUWalk4Ch2,
    ombuUWalk4Ch3, ombuUJumpCh0, ombuUJumpCh1, ombuUJumpCh2,
    ombuUJumpCh3, ombuUJumpTopCh0, ombuUJumpTopCh1, ombuUJumpTopCh2,
    ombuUJumpTopCh3, ombuUFallCh0, ombuUFallCh1, ombuUFallCh2,
    ombuUFallCh3, ombuUHitCh0, ombuUHitCh1, ombuUHitCh2,
    ombuUHitCh3, ombuDStand, ombuDWalk1, ombuDWalk2,
    ombuDWalk3, ombuDWalk4, ombuDActB2, ombuDActB3,
    ombuDActB4, ombuDHit, ombuDAttackLv0, ombuDAttackLv0Fwd,
    ombuDAttackLv0Back, ombuDAttackLv1, ombuDAttackLv1Fwd, ombuDAttackLv1Back,
    ombuDAttackLv2, ombuDAttackLv2Fwd, ombuDAttackLv2Back, ombuDSpecialNeutral,
    ombuDSpecialFwd, ombuDSpecialBack, plActLetClimbOnC2, plActLetClimbOnC3,
    plActLetClimbOnC4, plActLetClimbOnC5, plActLetClimbOnC6, pldemoStruggleStillPa,
    pldemoStruggleSlowPa, pldemoStruggleFastPa, plTalkProc, pldemoRide1IdlePa,
    pldemoRide1StickPa, pldemoRide1WeaponPa, pldemoRide1HoldPa, pldemoRide2IdlePa,
    pldemoRide2StickPa, pldemoRide2WeaponPa, pldemoRide3IdlePa, pldemoRide3StickPa,
    pldemoRide3WeaponPa, pldemoRideJumpPa, pldemoPickWalkPa, pldemoStandPa,
    pldemoWalkDefaultPa, pldemoWalkSlowPa, pldemoWalkPa, pldemoWalkFastPa,
    pldemoRunPa, pldemoJumpPa, pldemoFallPa, pldemoAirPa,
    pldemoLandHardPa, pldemoLandPa, pldemoFreezePa, pldemoStillPa,
    pldemoNopE4Pa, pldemoNopE5Pa, diveNonDiverWait, NULL, /* to 0xE7 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, /* to 0x121 */
    pldemo3dGoStraitPa, plActCharaSpecial, plActToStand, /* 0x122-0x124 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, /* to 0x138 */
    pldemoNop139Pa, pldemoNop13APa, /* 0x139, 0x13A */
};
s32 gPlAttackKindByLevel[3] = { 0x00, 0x0B, 0x16 }; /* the attack animations by weapon level */
Vec3f gPlSasukeThrow2Pos = { 0.0f, 6.0f, 6.0f }; /* the second throw of Sasuke: where it starts */
Vec3f gPlSasukeThrow2Vel = { 0.0f, 0.0f, 6.5f }; /* and its speed */
s32 gPlAirAttackKindByLevel[3] = { 0x01, 0x0C, 0x17 };
s32 gPlBaseAttackKindByLevel[3] = { 0x02, 0x0D, 0x18 };
s32 gPlSubWeaponKindByLevel[3] = { 0x03, 0x0E, 0x19 };
s32 gPlAirSubWeaponKindByLevel[3] = { 0x04, 0x0F, 0x1A };
/* The idle demo, per character and idle action (0x70-0x79): standing still, the frames to wait
 * (top byte) and the action after them; on moving, the action (0 the usual one, -1 none). */
s32 gPlIdleDemoTable[4][20] = { /* [character][(action - 0x70) * 2 + moved] */
    {
        0x05000071, 0, 0x01000072, 0, 0, -1, 0, -1, 0, -1,
        0, -1, 0x01000077, 0, 0x01000077, 0, 0x01000078, 0, 0x01000079, 0,
    },
    {
        0x08000071, 0, 0x01000072, 0x01000075, 0x01000073, 0x01000074, 0x01000073, 0x01000074, 0, -1,
        0, -1, 0x01000077, 0, 0x01000077, 0, 0x01000078, 0, 0x01000079, 0,
    },
    {
        0x03000071, 0, 0x01000073, 0x01000072, 0, -1, 0x01000074, 0x01000072, 0x01000074, 0x01000075,
        0, -1, 0, -1, 0, -1, 0, -1, 0x01000079, 0,
    },
    {
        0, 0, 0, 0, 0, -1, 0, -1, 0, -1,
        0, -1, 0x01000077, 0, 0x01000077, 0, 0x01000078, 0, 0x01000079, 0,
    },
};



/* Standing (tilted pi/6 to the facing). */
void plActStand(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;

    switch (msg) {
        case 0:
            if (p->targetFacing == 0.0f) {
                p->tilt = -0.5235988f;
            } else {
                p->tilt = 0.5235988f;
            }
            p->facing = p->targetFacing;
            break;
        case 1:
            plStickSpeed(p);
            next = plPickGroundOrWalk(p);
            if (next == -1) {
                next = __plIdleDemoPa(p);
            }
            plBaseDefault(p, msg, a, b);
            if (!(p->flags & 0x10)) {
                if (p->targetFacing == 0.0f) {
                    p->tilt = -0.5235988f;
                } else {
                    p->tilt = 0.5235988f;
                }
            }
            plActChange(p, next);
            return;
        case 2:
            p->tilt = 0.0f;
            break;
    }
    plActDefault(p, msg, a, b);
}

/* The walks: the animation's speed follows the walking speed. */
void plActWalk1(Player *p, s32 msg, s32 a, s32 b) {
    if (msg == 1) {
        if (plSetGroundAction(p)) {
            return;
        }
        p->animStepAdd = plPushSize(p);
    }
    plActDefault(p, msg, a, b);
}

void plActWalk2(Player *p, s32 msg, s32 a, s32 b) {
    if (msg == 1) {
        if (plSetGroundAction(p)) {
            return;
        }
        p->animStepAdd = (plPushSize(p) - 0.7f) * 0.6f;
    }
    plActDefault(p, msg, a, b);
}

void plActWalk3(Player *p, s32 msg, s32 a, s32 b) {
    if (msg == 1) {
        if (plSetGroundAction(p)) {
            return;
        }
        p->animStepAdd = (plPushSize(p) - 1.2f) * 0.7f;
    }
    plActDefault(p, msg, a, b);
}

void plActWalk4(Player *p, s32 msg, s32 a, s32 b) {
    if (msg == 1) {
        if (plSetGroundAction(p)) {
            return;
        }
        p->animStepAdd = (plPushSize(p) - 1.7f) * 0.4f;
        if ((p->flags & 0x10) && (p->buttons & 0x4000)) {
            p->animStepAdd = 2.0f * p->animStepAdd;
        }
    }
    plActDefault(p, msg, a, b);
}

/* The jump: A held keeps it going up. */
void plActJump(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;

    switch (msg) {
        case 0:
            p->flags |= 0x4000;
            p->jumpPush = p->jumpSpeed - p->velY;
            plLandSound(p);
            break;
        case 1:
            switch (p->state) {
                case 1:
                    break;
                case 0:
                    if (!(p->buttons & A_BUTTON)) {
                        p->state++;
                        p->jumpPush = -(p->velY + -1.0714285f) - -1.0714285f * p->jumpCut;
                    }
                    if ((f32) p->phase == p->jumpHoldSteps - 1.0f) {
                        p->state++;
                    }
                    break;
            }
            if (plCanCharaJump(p)) {
                next = 0x13;
            } else {
                next = plPickAir(p);
            }
            p->phase++;
            if (plActChange(p, next)) {
                return;
            }
            break;
        case 2:
            p->flags &= ~0x4000;
            break;
    }
    plActDefault(p, msg, a, b);
}

void plActDefaultUnused(Player *p, s32 msg, s32 a, s32 b) {
    plActDefault(p, msg, a, b);
}

/* Falling. */
void plActFallNoRide(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;

    switch (msg) {
        case 0:
            p->flags |= 0x4000;
            break;
        case 1:
            if (plCanCharaJump(p)) {
                next = 0x13;
            } else {
                next = plPickAir(p);
            }
            if (plActChange(p, next)) {
                return;
            }
            break;
        case 2:
            p->flags &= ~0x4000;
            break;
    }
    plActDefault(p, msg, a, b);
}

/* Falling past another player: asks to ride on its back. */
void plActFall(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;
    Player *q;

    switch (msg) {
        case 1:
            next = plPickAir(p);
            if (next == 0xF || next == -1) {
                if (plCanCharaJump(p)) {
                    next = 0x13;
                } else if (!(p->flags & 0x10)) {
                    if ((q = plFindRideTarget(p)) != NULL) {
                        plActSend(q, 4, 0, p);
                        if (plMsgReplyTake(p)) {
                            next = ombuDStart(p, q);
                        }
                    }
                }
            }
            if (plActChange(p, next)) {
                return;
            }
            break;
        case 0:
            p->flags |= 0x4000;
            break;
        case 2:
            p->flags &= ~0x4000;
            break;
    }
    plActDefault(p, msg, a, b);
}

/* The weapon's first attack (by weapon level). */
void plActAttack(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;
    s32 t;

    switch (msg) {
        case 0:
            plAttackSound(p);
            plBoxSetKind(p, gPlAttackKindByLevel[gPlayerStats[p->index].wpLv]);
            break;
        case 1:
            next = -1;
            plStickSpeed(p);
            if (plPushSize(p) >= 0.05f) {
                if (p->pushX >= 0.0f) {
                    p->targetFacing = 0.0f;
                } else {
                    p->targetFacing = 3.1415927f;
                }
            }
            plFaceMoveDir(p, 1.5707964f);
            t = plAnimPercent(p);
            if (p->state == 0) {
                p->state = plActSasukeSecondThrow(p, t);
            }
            if (t >= 60) {
                next = plPickMoveOrAttack(p);
                if (next == -1 && t == 100) {
                    next = 0;
                }
            }
            plActSwitch(p, next);
            p->pushZ = 0.0f;
            p->pushX = 0.0f;
            return;
        case 2:
            plBoxDelete(p);
            break;
    }
    plBoxAttackBase(p, msg, a, b);
}

void plActAttack34(Player *p, s32 msg, s32 a, s32 b) {
    plActAttack(p, msg, a, b);
}

void plActAttack39(Player *p, s32 msg, s32 a, s32 b) {
    plActAttack(p, msg, a, b);
}

void plActAttack3E(Player *p, s32 msg, s32 a, s32 b) {
    plActAttack(p, msg, a, b);
}

/* Sasuke's level-2 weapon throws a second time after 25 frames. */
s32 plActSasukeSecondThrow(Player *p, s32 t) {
    if (p->chara == 2 && gPlayerStats[p->index].wpLv == 2 && p->state == 0 && t >= 25 &&
        !(p->flags & 0x10)) {
        plKunaiFire(p, &gPlSasukeThrow2Pos, &gPlSasukeThrow2Vel);
        return TRUE;
    }
    return FALSE;
}

/* The attack in the air. */
void plActAirAttack(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;
    s32 t;

    switch (msg) {
        case 0:
            p->flags |= 0x4000;
            plAttackSound(p);
            plBoxSetKind(p, gPlAirAttackKindByLevel[gPlayerStats[p->index].wpLv]);
            break;
        case 1:
            next = -1;
            switch (p->state) {
                case 0:
                    plStickSpeed(p);
                    if (plPushSize(p) >= 0.05f) {
                        if (p->pushX >= 0.0f) {
                            p->targetFacing = 0.0f;
                        } else {
                            p->targetFacing = 3.1415927f;
                        }
                        if (p->targetFacing != p->facing) {
                            plFaceMoveDir(p, 1.5707964f);
                        }
                    }
                    if (p->ground != 0) {
                        p->state = 1;
                        p->phase = 2;
                    } else {
                        t = plAnimPercent(p);
                        if (p->state == 0) {
                            p->state = plActSasukeSecondThrow(p, t);
                        }
                        if (t >= 100) {
                            if (p->ground != 0) {
                                next = 0;
                            } else {
                                next = 0xF;
                            }
                        }
                    }
                    break;
                case 1:
                    if (--p->phase <= 0) {
                        next = 0x12;
                    }
                    break;
            }
            if (plActChange(p, next)) {
                return;
            }
            break;
        case 2:
            p->flags &= ~0x4000;
            plBoxDelete(p);
            break;
    }
    plBoxAttackBase(p, msg, a, b);
}

void plActAirAttack35(Player *p, s32 msg, s32 a, s32 b) {
    plActAirAttack(p, msg, a, b);
}

void plActAirAttack3A(Player *p, s32 msg, s32 a, s32 b) {
    plActAirAttack(p, msg, a, b);
}

void plActAirAttack3F(Player *p, s32 msg, s32 a, s32 b) {
    plActAirAttack(p, msg, a, b);
}

/* The attack from standing (flag 0x100000). */
void plActBaseAttack(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    switch (msg) {
        case 0:
            p->flags |= 0x100000;
            plAttackSound(p);
            plBoxSetKind(p, gPlBaseAttackKindByLevel[gPlayerStats[p->index].wpLv]);
            plChargeRelease(p);
            break;
        case 1:
            if (plAnimEnded(p)) {
                next = 8;
            } else if (plAnimPercent(p) >= 50 && plBaseStandNext(p)) {
                return;
            }
            if (plActChange(p, next)) {
                return;
            }
            break;
        case 2:
            plBoxDelete(p);
            break;
    }
    plBase(p, msg, a, b);
}

void plActBaseAttack36(Player *p, s32 msg, s32 a, s32 b) {
    plActBaseAttack(p, msg, a, b);
}

void plActBaseAttack3B(Player *p, s32 msg, s32 a, s32 b) {
    plActBaseAttack(p, msg, a, b);
}

void plActBaseAttack40(Player *p, s32 msg, s32 a, s32 b) {
    plActBaseAttack(p, msg, a, b);
}

/* The sub-weapon. */
void plActSubWeapon(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;
    s32 t;

    switch (msg) {
        case 0:
            plAttackSound(p);
            plBoxSetKind(p, gPlSubWeaponKindByLevel[gPlayerStats[p->index].wpLv]);
            break;
        case 1:
            next = -1;
            plStickSpeed(p);
            if (plPushSize(p) >= 0.05f) {
                if (p->pushX >= 0.0f) {
                    p->targetFacing = 0.0f;
                } else {
                    p->targetFacing = 3.1415927f;
                }
                if (p->targetFacing != p->facing) {
                    plFaceMoveDir(p, 1.5707964f);
                }
            }
            t = plAnimPercent(p);
            if (t >= 60) {
                next = plPickGroundWrap(p);
                if (next == 0) {
                    next = -1;
                }
                if (next == -1 && t == 100) {
                    next = 0;
                }
            } else {
                p->pushX = 0.0f;
                p->pushZ = 0.0f;
            }
            plActSwitch(p, next);
            return;
        case 2:
            plBoxDelete(p);
            break;
    }
    plBoxAttackBase(p, msg, a, b);
}

void plActSubWeapon37(Player *p, s32 msg, s32 a, s32 b) {
    plActSubWeapon(p, msg, a, b);
}

void plActSubWeapon3C(Player *p, s32 msg, s32 a, s32 b) {
    plActSubWeapon(p, msg, a, b);
}

void plActSubWeapon41(Player *p, s32 msg, s32 a, s32 b) {
    plActSubWeapon(p, msg, a, b);
}

/* The sub-weapon in the air. */
void plActAirSubWeapon(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;

    switch (msg) {
        case 0:
            p->flags |= 0x4000;
            plAttackSound(p);
            plBoxSetKind(p, gPlAirSubWeaponKindByLevel[gPlayerStats[p->index].wpLv]);
            break;
        case 1:
            next = -1;
            plStickSpeed(p);
            if (plPushSize(p) >= 0.05f) {
                if (p->pushX >= 0.0f) {
                    p->targetFacing = 0.0f;
                } else {
                    p->targetFacing = 3.1415927f;
                }
                if (p->targetFacing != p->facing) {
                    plFaceMoveDir(p, 1.5707964f);
                }
            }
            if (p->ground != 0) {
                next = plPickLanding(p);
            } else if (plAnimEnded(p)) {
                if (p->ground != 0) {
                    next = 0;
                } else {
                    next = 0xF;
                }
            }
            if (plActChange(p, next)) {
                return;
            }
            break;
        case 2:
            p->flags &= ~0x4000;
            plBoxDelete(p);
            break;
    }
    plBoxAttackBase(p, msg, a, b);
}

void plActAirSubWeapon38(Player *p, s32 msg, s32 a, s32 b) {
    plActAirSubWeapon(p, msg, a, b);
}

void plActAirAttack3D(Player *p, s32 msg, s32 a, s32 b) {
    plActAirAttack(p, msg, a, b);
}

void plActAirSubWeapon42(Player *p, s32 msg, s32 a, s32 b) {
    plActAirSubWeapon(p, msg, a, b);
}

/* Landing hard: a moment before moving on. */
void plActLandHard(Player *p, s32 msg, s32 a, s32 b) {
    s32 t;
    s32 next;

    switch (msg) {
        case 0:
            plRequestLandSound(p);
            break;
        case 1:
            if ((t = plAnimPercent(p)) >= 35) {
                plStickSpeed(p);
                next = plPickMoveOrAttack(p);
                if (t >= 99 && next == -1) {
                    next = 0;
                }
                if (plActChange(p, next)) {
                    return;
                }
            }
            break;
    }
    plActDefault(p, msg, a, b);
}

/* Landing. */
void plActLand(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;

    switch (msg) {
        case 0:
            p->state = 0;
            break;
        case 1:
            plStickSpeed(p);
            next = plPickMoveOrAttack(p);
            if (p->state == 0) {
                if (next == -1) {
                    plRequestLandSound(p);
                } else {
                    p->eventBits |= 0x20;
                }
                p->state = 1;
            }
            if (next == -1 && plAnimEnded(p)) {
                next = 0;
            }
            if (plActChange(p, next)) {
                return;
            }
            break;
    }
    plActDefault(p, msg, a, b);
}

/* Turning back. */
void plActTurnBack(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;

    if (msg == 1) {
        if (plAnimEnded(p)) {
            next = 8;
        } else if (plAnimPercent(p) >= 25) {
            next = plPickAttackNoFwd(p);
        } else {
            next = plPickAttack(p);
        }
        if (plActChange(p, next)) {
            return;
        }
    }
    plBase(p, msg, a, b);
}

void plActBaseStand(Player *p, s32 msg, s32 a, s32 b) {
    if (msg != 1 || !plBaseStandNext(p)) {
        plBase(p, msg, a, b);
    }
}

void plActBaseWalk(Player *p, s32 msg, s32 a, s32 b) {
    if (msg != 1 || !plBaseStandNext(p)) {
        plBase(p, msg, a, b);
    }
}

/* The charged move's end. */
void plActChargeEnd(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;
    s32 t;

    if (msg == 1) {
        if (plAnimEnded(p)) {
            next = 0;
        } else if ((t = plAnimPercent(p)) >= 50) {
            plStickSpeed(p);
            next = plPickMoveOrAttack(p);
        } else if (t >= 25) {
            next = plPickMoveOrAttack(p);
        } else {
            next = plPickAttackNoFwd(p);
        }
        if (plActChange(p, next)) {
            return;
        }
    }
    plActDefault(p, msg, a, b);
}

/* Standing so another player can climb on (flag 0x80000): message 4 with a = 0 lets it. */
void plActLetClimbOn(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            p->flags |= 0x80000;
            break;
        case 1:
            if (plBaseSlowDown(p)) {
                return;
            }
            break;
        case 2:
            p->flags &= ~0x80000;
            break;
        case 4:
            if (a == 0) {
                plMsgReply(b, 1);
                plActChange(p, ombuUStart(p, b));
                return;
            }
            break;
    }
    plActDefault(p, msg, a, b);
}

void plActLetClimbOnC2(Player *p, s32 msg, s32 a, s32 b) {
    plActLetClimbOn(p, msg, a, b);
}

void plActLetClimbOnC3(Player *p, s32 msg, s32 a, s32 b) {
    plActLetClimbOn(p, msg, a, b);
}

void plActLetClimbOnC4(Player *p, s32 msg, s32 a, s32 b) {
    plActLetClimbOn(p, msg, a, b);
}

void plActLetClimbOnC5(Player *p, s32 msg, s32 a, s32 b) {
    plActLetClimbOn(p, msg, a, b);
}

void plActLetClimbOnC6(Player *p, s32 msg, s32 a, s32 b) {
    plActLetClimbOn(p, msg, a, b);
}

void plActEmpty(void) {
}

/* The idle demo: each step of it holds for a count (the entry's top byte) and names the next
 * (its low 24 bits); moving the stick or a button leaves it.
 * The inner switch has no default: a default: label is laid out right after the compares, while
 * the original jumps over the two cases to the shared code, so the cases goto past it. The table
 * is indexed [chara][action * 2 + moved] (a [][10][2] index numbers the temporaries one apart). */
void plActIdleDemo(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;
    s32 moved = FALSE;
    s32 v;

    switch (msg) {
        case 0:
            p->flags |= 0x200000;
            if (p->targetFacing == 0.0f) {
                p->tilt = -0.5235988f;
            } else {
                p->tilt = 0.5235988f;
            }
            break;
        case 1:
            if (p->action == 0x79 && plWantsChargedMove(p)) {
                plActChange(p, 0x83);
                return;
            }
            plStickSpeed(p);
            if (plPushSize(p) > 0.05f ||
                plStickToSpeed(p, plMoveStickY(p->index, 0)) < -1.2f ||
                (p->held & 0xE010)) {
                moved = TRUE;
            }
            p->pushZ = 0.0f;
            p->pushX = 0.0f;
            v = gPlIdleDemoTable[p->chara][(p->action - 0x70) * 2 + moved];
            if (!moved) {
                if (p->idle == 0) {
                    next = v & 0xFFFFFF;
                    p->idle = v >> 24;
                }
                if (p->animStatus & 2) {
                    if (p->idle != 0) {
                        p->idle--;
                    }
                }
            } else {
                switch (v) {
                    case -1:
                        next = 0;
                        goto done;
                    case 0:
                        plStickSpeed(p);
                        next = plPickGroundOrWalk(p);
                        goto done;
                }
                next = v & 0xFFFFFF;
                p->idle = v >> 24;
            }
        done:
            plBaseDefault(p, msg, a, b);
            if (p->targetFacing == 0.0f) {
                p->tilt = -0.5235988f;
            } else {
                p->tilt = 0.5235988f;
            }
            if (next == 0x73 && p->chara == 1) {
                plFormSizeSet(p, 1);
            } else {
                plFormSizeSet(p, 0);
            }
            plActChange(p, next);
            return;
        case 2:
            p->flags &= ~0x200000;
            p->tilt = 0.0f;
            break;
    }
    plActDefault(p, msg, a, b);
}

void plActIdleDemoOnce(Player *p, s32 msg, s32 a, s32 b) {
    if (msg == 0) {
        p->idle = 1;
    }
    plActIdleDemo(p, msg, a, b);
}

void plActIdle70(Player *p, s32 msg, s32 a, s32 b) {
    plActIdleDemoOnce(p, msg, a, b);
}

void plActIdle71(Player *p, s32 msg, s32 a, s32 b) {
    if (p->chara == 3 && msg == 0) {
        p->idle = 1;
    }
    plActIdleDemo(p, msg, a, b);
}

void plActIdle72(Player *p, s32 msg, s32 a, s32 b) {
    plActIdleDemo(p, msg, a, b);
}

void plActIdle73(Player *p, s32 msg, s32 a, s32 b) {
    plActIdleDemo(p, msg, a, b);
}

void plActIdle74(Player *p, s32 msg, s32 a, s32 b) {
    plActIdleDemo(p, msg, a, b);
}

void plActIdle75(Player *p, s32 msg, s32 a, s32 b) {
    plActIdleDemo(p, msg, a, b);
}

void plActIdle76(Player *p, s32 msg, s32 a, s32 b) {
    plActIdleDemoOnce(p, msg, a, b);
}

void plActIdle77(Player *p, s32 msg, s32 a, s32 b) {
    plActIdleDemo(p, msg, a, b);
}

void plActIdle78(Player *p, s32 msg, s32 a, s32 b) {
    plActIdleDemoOnce(p, msg, a, b);
}

void plActIdle79(Player *p, s32 msg, s32 a, s32 b) {
    if (msg == 0) {
        if (p->targetFacing == 0.0f) {
            p->tilt = -0.5235988f;
        } else {
            p->tilt = 0.5235988f;
        }
        p->idle = 1;
        return;
    }
    plActIdleDemo(p, msg, a, b);
}

void plActDefault6C(Player *p, s32 msg, s32 a, s32 b) {
    plActDefault(p, msg, a, b);
}

void plActDefault6D(Player *p, s32 msg, s32 a, s32 b) {
    plActDefault(p, msg, a, b);
}

void plActDefault6E(Player *p, s32 msg, s32 a, s32 b) {
    plActDefault(p, msg, a, b);
}

void plActDefault6F(Player *p, s32 msg, s32 a, s32 b) {
    plActDefault(p, msg, a, b);
}

s32 plActPickNext(Player *p) {
    plStickSpeed(p);
    return plActChange(p, plPickGroundOrWalk(p));
}

void plActSlope7A(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            plActStartVoice(p);
            break;
        case 1:
            if (plActPickNext(p)) {
                return;
            }
            break;
    }
    plBaseDefault(p, msg, a, b);
}

void plActSlope7D(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            plActStartVoice(p);
            break;
        case 1:
            if (plActPickNext(p)) {
                return;
            }
            break;
    }
    plBaseDefault(p, msg, a, b);
}

void plActLastDefault7B(Player *p, s32 msg, s32 a, s32 b) {
    plBaseDefault(p, msg, a, b);
}

void plActLastDefault7C(Player *p, s32 msg, s32 a, s32 b) {
    plBaseDefault(p, msg, a, b);
}
