#ifndef TLB_TLB_0019_H
#define TLB_TLB_0019_H

#include "common.h"
#include "object.h"

s32 gdemoProc(Obj *obj, s16 msg, void *arg);
Obj *gdemoGenoimpCreate(Obj *obj);
s32 gdemoGenoKuroProc(Obj *obj, s16 msg, void *arg);
void gdemoGenoTrioCreate(Obj *obj);
void gdemoKemuriCreate(f32 x, f32 y, f32 z);
void gdemoLightningA(Obj *obj);
void gdemoLightningB(Obj *obj);
void gdemoShadowCreate(Obj *obj, f32 x, f32 y, f32 z, f32 a, f32 b, f32 c, f32 d, f32 e, f32 f, s32 m, Gfx *dl);
void gdemoScenarioStep(void);
Obj *gdemoWaterCreate(Obj *obj);

#endif
