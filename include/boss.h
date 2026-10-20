#ifndef BOSS_H
#define BOSS_H
/* The boss battles (overlay 10): the Goemon Impact against a boss. The battle's state is a set
 * of separate variables in the overlay's .bss; fields are named where their use is known.
 * bmove.c's bossClearState clears them all at the start of a battle. */
#include "common.h"
#include "object.h"
#include "camera.h"

/* A battle camera's work ("change" and the "Bappcam"s, bappcam.c and impact.c). */
typedef struct BCam {
    /* 0x00 */ Frame *frame;
    /* 0x04 */ u8 pad4[0x20];
    /* 0x24 */ Camera *cam;
    /* 0x28 */ Frame *frames[3];
    /* 0x34 */ u8 pad34[8];
    /* 0x3C */ s32 timer;
    /* 0x40 */ u8 on;
    /* 0x41 */ u8 unused41;
    /* 0x42 */ u8 step;
    /* 0x43 */ u8 pad43[9];
    /* 0x4C */ f32 side; /* the boss's x when the camera began */
} BCam;

#define BCAM(task) ((BCam *) ((u32) (task) + sizeof(Obj)))

extern s32 gBattlePlayer1Chara; /* the first player's character, 0-3 (cockpit.c) */
extern s32 gBattlePlayer2Chara; /* the second's (the partner's with one player) */
extern s32 gCockpitPilotChara; /* the cockpit's pilot's character (the voice) */
extern s32 gBattleUnused2;
extern s32 gBossDownCount; /* a counter (bossAddDownCount) */
extern s32 gBossFlinchCount; /* a counter (bossAddFlinchCount) */
extern f32 gMegaShotPosX; /* the cannon's position (bappcam.c) */
extern f32 gMegaShotPosY;
extern f32 gMegaShotPosZ;
extern f32 gMegaBurstY;
extern s32 gBattleBossId; /* the boss, 1-5 (bmenu.c's order) */
extern u32 gBattleFlags; /* flags: 1 hit, 2 the battle is over */
extern u32 gBrotherFlags;
extern u32 gBossFlags;
extern u32 gBattleSndToggles;
extern u32 gBattleFlags2;
extern f32 gCockpitLookAtX; /* the Impact's position */
extern f32 gCockpitLookAtY;
extern f32 gCockpitLookAtZ;
extern s32 gImpactPower; /* a gauge, 0-100 (99 at most from bossAddPower) */
extern s32 gBrotherMoveTime;
extern s32 gBrotherMove;
extern s32 gBattleUnused3;
extern s32 gImpactMoney; /* a gauge, 0-999 (500 at first) */
extern s32 gImpactArmor; /* two lives, 0-100 */
extern s32 gImpactArmor2;
extern f32 gImpactPosX;
extern f32 gImpactPosY;
extern f32 gImpactPosZ;
extern f32 gBrotherPosX;
extern f32 gBrotherPosY;
extern f32 gBrotherPosZ;
extern f32 gBrotherYaw;
extern f32 gCockpitRotX;
extern f32 gCockpitRotY;
extern f32 gCockpitRotZ;
extern s32 gImpactHitDelay;
extern s32 gBrotherDownTime;
extern s32 gBossLifeMax;
extern s32 gBossLife; /* the boss's life */
extern s32 gBattleUnused5;
extern s32 gImpactHitDamage;
extern f32 gBossPosX; /* the boss's position */
extern f32 gBossPosY;
extern f32 gBossPosZ;
extern f32 gBossYaw;
extern f32 gCockpitAimX;
extern f32 gCockpitAimY;
extern f32 gCockpitAimZ;
extern f32 gCockpitHeadPitch;
extern f32 gCockpitHeadYaw;
extern f32 gBrotherAimY;
extern s32 gImpactMoveTime;
extern s32 gBattleUnused10;
extern s32 gBattleSeqDone; /* 1 once a button sequence is done (bossCheckButtonSeq) */
extern s32 gBossActionArg;
extern s32 gBossPhase;
extern s32 gBossActionCount;
extern s32 gBossFrameCount;
extern u8 gValcanSide;
extern u8 gOvalcanSide;
extern u8 gBattleUnused11;
extern u8 gCockpitShake;
extern u8 gBattleUnused12;
extern u16 gCockpitPort; /* the two pilots' controllers */
extern u16 gCockpitBtnStraight; /* buttons that drop the guard */
extern u16 gCockpitBtnJab;
extern u16 gCockpitBtnShoot; /* the cockpit's shoot button (vulcan, cannon) */
extern u16 gCockpitBtnBaton; /* and its throw (the baton) */
extern u16 gCockpitBtnDUp; /* the D-pad's buttons (up, right, down, left) */
extern u16 gCockpitBtnDRight;
extern u16 gCockpitBtnDDown;
extern u16 gCockpitBtnDLeft;
extern u16 gBrotherPort; /* the second Impact's ("Brother") controller */
extern u16 gBrotherBtnJump; /* and its buttons: jump, */
extern u16 gBrotherBtnPunch; /* punch, */
extern u16 gBrotherBtnShoot; /* shoot, */
extern u16 gBrotherBtnShout; /* shout */
extern s32 gBrotherPilotChara; /* its pilot's character, 0-3 (the voice) */

#endif
