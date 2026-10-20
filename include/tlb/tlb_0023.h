#ifndef TLB_TLB_0023_H
#define TLB_TLB_0023_H

#include "common.h"
#include "object.h"

s32 omiDemoProc(Obj *obj, s16 msg, void *arg);
s32 omiDemoShortProc(Obj *obj, s16 msg, void *arg);
Obj *omiDemoGoeCreate(Obj *obj);
Obj *omiDemoEbiCreate(Obj *obj);
Obj *omiDemoSasCreate(Obj *obj);
Obj *omiDemoJijiCreate(Obj *obj);
Obj *omiDemoShortJijiCreate(Obj *obj);
Obj *omiDemoKuroCreate(Obj *obj);
Obj *omiDemoShortKuroCreate(Obj *obj);
Obj *omiDemoOmiCreate(Obj *obj);
Obj *omiDemoShortOmiCreate(Obj *obj);

#endif
