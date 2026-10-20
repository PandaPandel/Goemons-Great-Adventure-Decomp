#ifndef DEMO_H
#define DEMO_H

#include "enemy.h"
#include "game.h"

void demoSetSpeedPlRun(f32 x1, f32 y1, f32 z1, f32 speed1, f32 x2, f32 y2, f32 z2, f32 speed2);
void demoSetSpeedPlRun4P(f32 x1, f32 y1, f32 z1, f32 speed1, f32 x2, f32 y2, f32 z2, f32 speed2, f32 x3, f32 y3, f32 z3, f32 speed3, f32 x4, f32 y4, f32 z4, f32 speed4);
void demoSetTimePlRun(f32 x1, f32 y1, f32 z1, f32 time1, f32 x2, f32 y2, f32 z2, f32 time2);
void demoSetTimePlRun4P(f32 x1, f32 y1, f32 z1, f32 time1, f32 x2, f32 y2, f32 z2, f32 time2, f32 x3, f32 y3, f32 z3, f32 time3, f32 x4, f32 y4, f32 z4, f32 time4);
s32 demoPlRunEndCheck(void);
void demoGetPlPos(f32 *x1, f32 *y1, f32 *z1, s32 *zone1, f32 *x2, f32 *y2, f32 *z2, s32 *zone2);
void demoGetPlPos4P(f32 *x1, f32 *y1, f32 *z1, s32 *zone1, f32 *x2, f32 *y2, f32 *z2, s32 *zone2, f32 *x3, f32 *y3, f32 *z3, s32 *zone3, f32 *x4, f32 *y4, f32 *z4, s32 *zone4);
void demoSetPlPos(s32 zone1, f32 x1, f32 y1, f32 z1, f32 r1, s32 zone2, f32 x2, f32 y2, f32 z2, f32 r2, s32 zone3, f32 x3, f32 y3, f32 z3, f32 r3, s32 zone4, f32 x4, f32 y4, f32 z4, f32 r4);
void demoPlStandAll(void);
void demoPlFreezeAll(void);
void demoPlReleaseAll(void);
s32 demoFindPlByChara(s32 chara);
void demoTalkInit(Obj *o, u32 data);
s32 demoPassMessage(Obj *o);
s32 demoTalk(Obj *o, PersonTalk *t);
void demoFinish(Obj *o);
s32 demoTalkEndCheck(Obj *o);
void demoSetBoxKind69(Obj *o);
s32 demoBoxHitCheck(Obj *o);
s32 demoTalkerMake(Obj *o, u8 r);
void demoTalkerFree(Obj *o);
void demoEneDeathAct(Obj *o, ACollFrame *box, u32 dl, u32 dl2, s32 reward);
void demoAreaFlagSave(void);
void demoAreaFlagRestore(void);
void demoAreaFlagSetBit7(s32 n);
void demoGetEneModelPos(Obj *o, Vec3f *out);
s32 demoEneState2Check(Obj *o);

#endif
