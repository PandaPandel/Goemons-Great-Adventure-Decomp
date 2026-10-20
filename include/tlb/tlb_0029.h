#ifndef TLB_TLB_0029_H
#define TLB_TLB_0029_H

#include "common.h"
#include "object.h"

s32 mach2DemoProc(Obj *obj, s16 msg, void *arg);
void mach2DemoScenarioStart(s32 n);
Obj *mach2DemoGoeCreate(Obj *obj);
Obj *mach2DemoEbiCreate(Obj *obj);
Obj *mach2DemoSasCreate(Obj *obj);
Obj *mach2DemoYaeCreate(Obj *obj);
Obj *mach2DemoMachineCreate(Obj *obj);
void mach2DemoEffectsStep(void);

#endif
