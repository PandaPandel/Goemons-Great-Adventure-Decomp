#ifndef TLB_TLB_0323_H
#define TLB_TLB_0323_H

#include "common.h"
#include "object.h"

s32 Bbisu_proc(Obj *obj, s16 msg, void *arg);
s32 bisuHbeamProc(Obj *obj, s16 msg, void *arg);
void bisuNeedleCreate(f32 x, f32 y, f32 z, f32 v);
void bisuRoseCreate(f32 x, f32 y, f32 z, f32 scale);
void bisuHpunchCreate(f32 x, f32 y, f32 z, s32 type);
s32 bisuLbarriProc(Obj *obj, s16 msg, void *arg);
s32 bisuPlateProc(Obj *obj, s16 msg, void *arg);
s32 bisuBVaniProc(Obj *obj, s16 msg, void *arg);
void bisuHpunch2Create(f32 x, f32 y, f32 z, s32 type);
void bisuBboostCreate(f32 x, f32 y, f32 z, f32 v);
s32 bisuBVDemoProc(Obj *obj, s16 msg, void *arg);
s32 bisuBplateDProc(Obj *obj, s16 msg, void *arg);
s32 bisuBroseDProc(Obj *obj, s16 msg, void *arg);
s32 bisuBappcamProc(Obj *obj, s16 msg, void *arg);
void bisuHeffectCreate(f32 x, f32 y, f32 z);
void bisuHringCreate(f32 x, f32 y, f32 z);

#endif
