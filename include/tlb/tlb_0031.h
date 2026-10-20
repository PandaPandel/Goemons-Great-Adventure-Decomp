#ifndef TLB_TLB_0031_H
#define TLB_TLB_0031_H

#include "common.h"
#include "object.h"

s32 demo22Proc(Obj *obj, s16 msg, void *arg);
Obj *demo22GoeCreate(Obj *obj);
Obj *demo22EbiCreate(Obj *obj);
Obj *demo22SasCreate(Obj *obj);
Obj *demo22YaeCreate(Obj *obj);
Obj *demo22SuzakuCreate(Obj *obj);
Obj *demo22DochukiCreate(Obj *obj);
Obj *demo22BismaruCreate(Obj *obj);
Obj *demo22GateCreate(Obj *obj);
void demo22TorchCreate(Obj *obj, f32 x, f32 y, f32 z, u8 pr, u8 pg, u8 pb, u8 er, u8 eg, u8 eb, f32 s, u8 n);
Obj *demo22HyotanCreate(Obj *obj);

#endif
