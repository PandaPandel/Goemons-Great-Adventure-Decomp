#ifndef TLB_TLB_0020_H
#define TLB_TLB_0020_H

#include "common.h"
#include "object.h"

s32 chademoProc(Obj *obj, s16 msg, void *arg);
Obj *chademoGoeCreate(Obj *obj);
Obj *chademoEbiCreate(Obj *obj);
Obj *chademoSasCreate(Obj *obj);
void chademoChimneySmoke(void);
void chademoScenarioStep(void);
void chademoBabaMusumeCreate(Obj *obj);

#endif
