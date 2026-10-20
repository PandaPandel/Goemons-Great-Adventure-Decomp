#ifndef SYS_H
#define SYS_H

#include "game.h"

Gfx *sysClearFrame(Gfx *gfx);
Gfx *sysClearZBuffer(Gfx *gfx);
void sysInitGame(void);
void *sysArenaAllocBottom(s32 size);
void *sysArenaCallocBottom(s32 size);
void *sysArenaAllocTop(s32 size);
void *sysArenaCallocTop(s32 size);
void sysArenaSetBottom(u32 bottom);
void sysArenaResetBottom(void);
void sysArenaSetTop(u32 top);
void sysArenaSet(void *bottom, void *top);
void sysArenaReset(u32 top);

#endif
