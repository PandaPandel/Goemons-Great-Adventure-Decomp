#ifndef SHADOW_H
#define SHADOW_H

#include "object.h"
#include "polygon.h"
#include "vram.h"

MPolFrame *shadowCreate(void);
void shadowPut(MPolFrame *f, f32 x, f32 y, f32 z, f32 size, CollTri *floor);
void shadowMtxOnSurface(f32 m[4][4], f32 x, f32 y, f32 z);

#endif
