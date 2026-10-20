#ifndef PIPGAUGE_H
#define PIPGAUGE_H

#include "common.h"
#include "object.h"

s32 pipGauge_proc(Obj *obj, s16 msg, void *arg);
Gfx *pipDraw(Gfx *gfx, s32 x, s32 y, s32 i);
void pipGaugeOn(Obj *obj);
void pipGaugeOff(Obj *obj);
void pipGaugeSetPos(Obj *obj, s32 x, s32 y);
void pipGaugeSetTarget(Obj *obj, s32 n);

#endif
