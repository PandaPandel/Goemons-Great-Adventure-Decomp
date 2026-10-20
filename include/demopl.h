#ifndef DEMOPL_H
#define DEMOPL_H

#include "player.h"

void pldemoRide1Base(Player *p, s32 msg, s32 a, s32 b);
void pldemoRide1FallCheck(Player *p, s32 msg, s32 a, s32 b);
void pldemoRide1IdlePa(Player *p, s32 msg, s32 a, s32 b);
void pldemoRide1StickPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoRide1WeaponPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoRide1HoldPa(Player *p, s32 msg, s32 a, s32 b);
f32 pldemoStickX(Player *p);
f32 pldemoStickY(Player *p);
f32 pldemoStickDeadZone(s32 v);
void pldemoRide2Base(Player *p, s32 msg, s32 a, s32 b);
void pldemoRide2FallCheck(Player *p, s32 msg, s32 a, s32 b);
void pldemoRide2IdlePa(Player *p, s32 msg, s32 a, s32 b);
void pldemoRide2StickPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoRide2WeaponPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoRideAlong(Player *p, Obj *ride);
void pldemoRide3Base(Player *p, s32 msg, s32 a, s32 b);
void pldemoRide3FallCheck(Player *p, s32 msg, s32 a, s32 b);
void pldemoRide3IdlePa(Player *p, s32 msg, s32 a, s32 b);
void pldemoRide3StickPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoRide3WeaponPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoRideJumpPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoWalkBase(Player *p, s32 msg, s32 a, s32 b);
s32 pldemoWalkForSpeed(Player *p);
s32 pldemoWalkOrFall(Player *p);
s32 pldemoWalkToGoal(Player *p);
s32 pldemoGoalStep(Player *p);
void pldemoPickWalkPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoStandPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoWalkSlowPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoWalkPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoWalkFastPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoRunPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoWalkDefaultPa(Player *p, s32 msg, s32 a, s32 b);
s32 pldemoLandingKind(Player *p);
s32 pldemojumpfallsub(Player *p);
void pldemoJumpPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoFallPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoAirPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoLandPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoLandHardPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoFreezePa(Player *p, s32 msg, s32 a, s32 b);
void pldemoStillPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoDiveWalkBase(Player *p, s32 msg, s32 a, s32 b);
void pldemoNopPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoAnim17Pa(Player *p, s32 msg, s32 a, s32 b);
void pldemoAnim68Pa(Player *p, s32 msg, s32 a, s32 b);
void pldemoAnim69Pa(Player *p, s32 msg, s32 a, s32 b);
void pldemoAnim6APa(Player *p, s32 msg, s32 a, s32 b);
void pldemoAnim6BPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoStruggle(Player *p, s32 msg, s32 a, s32 b);
void pldemoStruggleStillPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoStruggleSlowPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoStruggleFastPa(Player *p, s32 msg, s32 a, s32 b);
void pldemo3dGoStraitPa(Player *p, s32 msg, s32 a, s32 b);
void pldemoNopE4Pa(Player *p, s32 msg, s32 a, s32 b);
void pldemoNopE5Pa(Player *p, s32 msg, s32 a, s32 b);
void pldemoNop139Pa(Player *p, s32 msg, s32 a, s32 b);
void pldemoNop13APa(Player *p, s32 msg, s32 a, s32 b);

#endif
