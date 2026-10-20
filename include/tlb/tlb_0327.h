#ifndef TLB_TLB_0327_H
#define TLB_TLB_0327_H

#include "common.h"
#include "object.h"

s32 Bdev_proc(Obj *obj, s16 msg, void *arg);
void onibiCreate(f32 x, f32 y, f32 z, s32 type);
void pentaCreate(f32 x, f32 y, f32 z, s32 type);
void ironballCreate(f32 x, f32 y, f32 z, s32 type);
void pentaniCreate(f32 x, f32 y, f32 z, s32 type);
void kesaEffectCreate(f32 x, f32 y, f32 z, s32 type);
void pentaniDemoCreate(f32 x, f32 y, f32 z, s32 type);
void curseballRCreate(f32 x, f32 y, f32 z, s32 type);
void impactBulletCreate(f32 x, f32 y, f32 z, s32 type);
void bdevCuBalDCreate(f32 x, f32 y, f32 z, s32 type);
s32 bdevBappcamProc(Obj *obj, s16 msg, void *arg);
void bdevBisuGCreate(f32 x, f32 y, f32 z);
void bdevBisuG2Create(f32 x, f32 y, f32 z);

#endif
