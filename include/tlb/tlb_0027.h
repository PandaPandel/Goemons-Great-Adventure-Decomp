#ifndef TLB_TLB_0027_H
#define TLB_TLB_0027_H

#include "common.h"
#include "object.h"

s32 dochukiDemoProc(Obj *obj, s16 msg, void *arg);
Obj *dochukiDemoGoeCreate(Obj *obj);
Obj *dochukiDemoEbiCreate(Obj *obj);
Obj *dochukiDemoSasCreate(Obj *obj);
Obj *dochukiDemoYaeCreate(Obj *obj);
Obj *dochukiDemoBisuCreate(Obj *obj);
Obj *dochukiDemoMach12Create(Obj *obj);
Obj *dochukiDemoSmokeCreate(Obj *obj);
void dochukiDemoMeffectCreate(Obj *obj, f32 x, f32 y, f32 z);
void dochukiDemoHitballCreate(Obj *obj, f32 x, f32 y, f32 z, f32 v);
void dochukiDemoEffectsStep(void);

#endif
