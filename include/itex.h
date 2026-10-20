#ifndef ITEX_H
#define ITEX_H

#include "object.h"
#include "demotool.h"

s32 itexAnimLength(AnimFrame *f);
s32 itex(Obj *obj, s16 msg, void *arg);
void itexSparkle(f32 x, f32 y, f32 z, f32 size);
void itexPuff(f32 x, f32 y, f32 z, f32 size);
void itexPuffGrey(f32 x, f32 y, f32 z, f32 size);
void itexPuffDark(f32 x, f32 y, f32 z, f32 size);
void itexPuffYellow(f32 x, f32 y, f32 z, f32 size);
void itexGlowBlue(f32 x, f32 y, f32 z, f32 size);
void itexSparkleYellow(f32 x, f32 y, f32 z, f32 size);
void itexSparkleBlue(f32 x, f32 y, f32 z, f32 size);
void itexFlash(f32 x, f32 y, f32 z, f32 size);
void itexTimedYellow(f32 x, f32 y, f32 z, f32 size, s32 life);
void itexPuffWhite(f32 x, f32 y, f32 z, f32 size);

#endif
