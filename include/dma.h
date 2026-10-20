#ifndef DMA_H
#define DMA_H

#include "scheduler.h"

void dmaInitGfxTasks(void);
void dmaBeginDisplayList(s32 fbIndex);
void dmaBeginFrame(s32 fbIndex);
void dmaSubmitFrame(void);
void dmaInitHandles(void);
OSPiHandle *dmaSramInit(void);
void dmaTransfer(OSPiHandle *handle, s32 direction, void *dram, u32 devAddr, s32 size);
void dmaReadRom(u32 rom, void *ram, s32 size);
void dmaReadSram(u32 sram, void *ram, s32 size);
void dmaWriteSram(void *ram, u32 sram, s32 size);

#endif
