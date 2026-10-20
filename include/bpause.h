#ifndef BPAUSE_H
#define BPAUSE_H

#include "common.h"
#include "object.h"
#include "game.h"

Gfx *bpauseDrawIconRgba(Gfx *gfx, u8 *tex, s32 n, s32 x, s32 y);
Gfx *bpauseDrawIconI4(Gfx *gfx, u8 *tex, s32 n, s32 x, s32 y);
s32 bpauseProc(Obj *obj, s16 msg, void *arg);

#endif
