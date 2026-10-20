#ifndef GFXBUF_H
#define GFXBUF_H

#include "object.h"
#include "game.h"
#include "scheduler.h"

void gfxStartBuffer(s32 i);
s32 gfxGetFreeBytes(void);
void gfxWritebackBuffer(s32 i);
Mtx *gfxAllocMtx(s32 n);
Vtx *gfxAllocVtx(s32 n);
LookAt *gfxAllocLookAt(s32 n);
Vp *gfxAllocVp(s32 n);
Lights7 *gfxAllocLights7(s32 n);
Gfx *gfxCopyDisplayList(Gfx *dst, Gfx *src);
u32 gfxGetPendingSwaps(void);
s32 gfxGetFbIndex(void);
u16 *gfxGetPrevFramebuffer(void);
u32 gfxSegmentToRam(u32 addr, u32 *segs);

#endif
