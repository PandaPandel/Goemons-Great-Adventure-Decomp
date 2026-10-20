#ifndef TLB_TLB_0326_H
#define TLB_TLB_0326_H

#include "common.h"
#include "object.h"

s32 Bkas_proc(Obj *obj, s16 msg, void *arg);
void bkasOhudaCreate(f32 x, f32 y, f32 z, s32 type);
void bkasSensuCreate(f32 x, f32 y, f32 z, s32 type);
void bkasHandCreate(f32 x, f32 y, f32 z, s32 type);
s32 bkasWaterProc(Obj *obj, s16 msg, void *arg);
s32 bkasTakiProc(Obj *obj, s16 msg, void *arg);
void bkasKabukiBCreate(f32 x, f32 y, f32 z, s32 type);
void wakemiCreate(f32 x, f32 y, f32 z, s32 type);
void demoStageCreate(f32 x, f32 y, f32 z);
void demoLightCreate(f32 x, f32 y, f32 z, s32 type);
void demoPaperCreate(f32 x, f32 y, f32 z);
void bkasSensuRCreate(f32 x, f32 y, f32 z, s32 type);
s32 bkasBappcamProc(Obj *obj, s16 msg, void *arg);

#endif
