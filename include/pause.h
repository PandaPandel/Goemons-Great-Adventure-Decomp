#ifndef PAUSE_H
#define PAUSE_H

#include "object.h"
#include "game.h"

Gfx *pauseDrawCount(Gfx *gfx);
void pauseUnusedStub(void);
s32 pause_proc(Obj *obj, s16 msg, void *arg);

#endif
