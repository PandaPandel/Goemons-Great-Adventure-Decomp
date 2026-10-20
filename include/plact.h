#ifndef PLACT_H
#define PLACT_H

#include "player.h"

void plActStand(Player *p, s32 msg, s32 a, s32 b);
void plActWalk1(Player *p, s32 msg, s32 a, s32 b);
void plActWalk2(Player *p, s32 msg, s32 a, s32 b);
void plActWalk3(Player *p, s32 msg, s32 a, s32 b);
void plActWalk4(Player *p, s32 msg, s32 a, s32 b);
void plActJump(Player *p, s32 msg, s32 a, s32 b);
void plActDefaultUnused(Player *p, s32 msg, s32 a, s32 b);
void plActFallNoRide(Player *p, s32 msg, s32 a, s32 b);
void plActFall(Player *p, s32 msg, s32 a, s32 b);
void plActAttack(Player *p, s32 msg, s32 a, s32 b);
void plActAttack34(Player *p, s32 msg, s32 a, s32 b);
void plActAttack39(Player *p, s32 msg, s32 a, s32 b);
void plActAttack3E(Player *p, s32 msg, s32 a, s32 b);
s32 plActSasukeSecondThrow(Player *p, s32 t);
void plActAirAttack(Player *p, s32 msg, s32 a, s32 b);
void plActAirAttack35(Player *p, s32 msg, s32 a, s32 b);
void plActAirAttack3A(Player *p, s32 msg, s32 a, s32 b);
void plActAirAttack3F(Player *p, s32 msg, s32 a, s32 b);
void plActBaseAttack(Player *p, s32 msg, s32 a, s32 b);
void plActBaseAttack36(Player *p, s32 msg, s32 a, s32 b);
void plActBaseAttack3B(Player *p, s32 msg, s32 a, s32 b);
void plActBaseAttack40(Player *p, s32 msg, s32 a, s32 b);
void plActSubWeapon(Player *p, s32 msg, s32 a, s32 b);
void plActSubWeapon37(Player *p, s32 msg, s32 a, s32 b);
void plActSubWeapon3C(Player *p, s32 msg, s32 a, s32 b);
void plActSubWeapon41(Player *p, s32 msg, s32 a, s32 b);
void plActAirSubWeapon(Player *p, s32 msg, s32 a, s32 b);
void plActAirSubWeapon38(Player *p, s32 msg, s32 a, s32 b);
void plActAirAttack3D(Player *p, s32 msg, s32 a, s32 b);
void plActAirSubWeapon42(Player *p, s32 msg, s32 a, s32 b);
void plActLandHard(Player *p, s32 msg, s32 a, s32 b);
void plActLand(Player *p, s32 msg, s32 a, s32 b);
void plActTurnBack(Player *p, s32 msg, s32 a, s32 b);
void plActBaseStand(Player *p, s32 msg, s32 a, s32 b);
void plActBaseWalk(Player *p, s32 msg, s32 a, s32 b);
void plActChargeEnd(Player *p, s32 msg, s32 a, s32 b);
void plActLetClimbOn(Player *p, s32 msg, s32 a, s32 b);
void plActLetClimbOnC2(Player *p, s32 msg, s32 a, s32 b);
void plActLetClimbOnC3(Player *p, s32 msg, s32 a, s32 b);
void plActLetClimbOnC4(Player *p, s32 msg, s32 a, s32 b);
void plActLetClimbOnC5(Player *p, s32 msg, s32 a, s32 b);
void plActLetClimbOnC6(Player *p, s32 msg, s32 a, s32 b);
void plActEmpty(void);
void plActIdleDemo(Player *p, s32 msg, s32 a, s32 b);
void plActIdleDemoOnce(Player *p, s32 msg, s32 a, s32 b);
void plActIdle70(Player *p, s32 msg, s32 a, s32 b);
void plActIdle71(Player *p, s32 msg, s32 a, s32 b);
void plActIdle72(Player *p, s32 msg, s32 a, s32 b);
void plActIdle73(Player *p, s32 msg, s32 a, s32 b);
void plActIdle74(Player *p, s32 msg, s32 a, s32 b);
void plActIdle75(Player *p, s32 msg, s32 a, s32 b);
void plActIdle76(Player *p, s32 msg, s32 a, s32 b);
void plActIdle77(Player *p, s32 msg, s32 a, s32 b);
void plActIdle78(Player *p, s32 msg, s32 a, s32 b);
void plActIdle79(Player *p, s32 msg, s32 a, s32 b);
void plActDefault6C(Player *p, s32 msg, s32 a, s32 b);
void plActDefault6D(Player *p, s32 msg, s32 a, s32 b);
void plActDefault6E(Player *p, s32 msg, s32 a, s32 b);
void plActDefault6F(Player *p, s32 msg, s32 a, s32 b);
s32 plActPickNext(Player *p);
void plActSlope7A(Player *p, s32 msg, s32 a, s32 b);
void plActSlope7D(Player *p, s32 msg, s32 a, s32 b);
void plActLastDefault7B(Player *p, s32 msg, s32 a, s32 b);
void plActLastDefault7C(Player *p, s32 msg, s32 a, s32 b);

#endif
