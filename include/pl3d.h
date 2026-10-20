#ifndef PL3D_H
#define PL3D_H

#include "player.h"

void pl3dStep(Player *p);
s32 pl3dCollide(Player *p, Vec3f *vel);
void pl3dUnusedStub(void);
void pl3dAddVel(Player *p, Vec3f *vel);
void pl3dSetModelPos(Player *p);
void pl3dCapSpeed(Player *p);
void pl3dFreeMove(Player *p);
void pl3dApplyPush(Player *p);
void pl3dTurnModel(Player *p, f32 x, f32 z);
f32 pl3dGetMoveAngle(Player *p);
void pl3dRideFloors(Player *p, Vec3f *vel);

#endif
