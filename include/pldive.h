#ifndef PLDIVE_H
#define PLDIVE_H

#include "player.h"

void diveDefault(Player *p, s32 msg, s32 a, s32 b);
void diveScrewSound(Player *p);
void diveYaeBegin(Player *p);
void diveYaeDiveIn(Player *p, s32 msg, s32 a, s32 b);
void diveYaeDiveInEnd(Player *p, s32 msg, s32 a, s32 b);
void diveYaeSurface(Player *p, s32 msg, s32 a, s32 b);
void diveSurfaceEnd(Player *p, s32 msg, s32 a, s32 b);
void diveYaeSwim(Player *p, s32 msg, s32 a, s32 b);
void diveYaeSwimStop(Player *p, s32 msg, s32 a, s32 b);
void diveYaeIdle(Player *p, s32 msg, s32 a, s32 b);
void diveYaeSwimStart(Player *p, s32 msg, s32 a, s32 b);
void diveYaeDash(Player *p, s32 msg, s32 a, s32 b);
void diveYaeDash2(Player *p, s32 msg, s32 a, s32 b);
void diveYaeDash3(Player *p, s32 msg, s32 a, s32 b);
void diveYaeDashEnd(Player *p, s32 msg, s32 a, s32 b);
void diveYaeHit(Player *p, s32 msg, s32 a, s32 b);
void diveYaeEat(Player *p, s32 msg, s32 a, s32 b);
void diveYaeBazooka(Player *p, s32 msg, s32 a, s32 b);
void diveYaeSwimBazooka(Player *p, s32 msg, s32 a, s32 b);
s32 diveIsAttackHeld(Player *p);
s32 diveYaeNextAction(Player *p);
s32 diveSasNextAction(Player *p);
s32 diveIsAHeld(Player *p);
void diveStickSwim(Player *p);
f32 diveDepthFactor(Player *p);
void diveUnusedStub1(void);
void diveLean(Player *p);
void diveUnusedStub2(void);
void diveSasBegin(Player *p);
void diveSasDiveIn(Player *p, s32 msg, s32 a, s32 b);
void diveSasSurface(Player *p, s32 msg, s32 a, s32 b);
void diveSasSwim(Player *p, s32 msg, s32 a, s32 b);
void diveSasIdle(Player *p, s32 msg, s32 a, s32 b);
void diveSasAttack(Player *p, s32 msg, s32 a, s32 b);
void diveSasAttackLv0(Player *p, s32 msg, s32 a, s32 b);
void diveSasAttackLv1(Player *p, s32 msg, s32 a, s32 b);
void diveSasAttackLv2(Player *p, s32 msg, s32 a, s32 b);
void diveSasHit(Player *p, s32 msg, s32 a, s32 b);
void diveSasEat(Player *p, s32 msg, s32 a, s32 b);
s32 sasScrew(Obj *obj, s16 msg, void *arg);
void diveScrewSetPos(Player *p, Obj *screw, s32 unused);
f32 diveScrewSpin(Obj *obj);
void __diveStart(Player *p);
void diveNonDiverWait(Player *p, s32 msg, s32 a, s32 b);

#endif
