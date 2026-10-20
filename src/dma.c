#include "scheduler.h"
#include "dma.h"
#include "sys.h"

/* The frame's display list (begin and submit), and the game's PI DMA: the cartridge handle, the
 * SRAM handle, and one synchronous DMA routine every ROM load goes through (LoadFile included). */

extern OSPiHandle *gCartHandle; /* the cartridge ROM */
extern OSPiHandle *gSramHandlePtr; /* SRAM */
extern OSIoMesg gDmaIoMesg;    /* the one DMA request */
extern OSMesgQueue gDmaMsgQ; /* its reply queue */
extern OSPiHandle gSramHandle;  /* the SRAM handle itself */

extern Gfx gDisplayListBufs[3][0x2000]; /* the display lists, one per framebuffer, 64 KB each */
extern Gfx *gDisplayListStart;           /* the start of this frame's list */
extern Gfx *gDisplayListHead;           /* the display-list head */
extern OSScTask gGfxTasks[3];    /* the graphics task, one per framebuffer */
extern u64 gRdpFifo[];          /* the RDP output FIFO, 16 KB */
extern u64 gRspDramStack[];          /* the RSP's DRAM stack */
extern u64 gRspYieldBuf[];          /* the yield buffer */
extern u16 gFramebuffers[3][320 * 240];   /* the framebuffers (file 5's RAM) */
extern s16 gGfxTaskDoneMsgs[3];         /* each task's done message */
extern OSMesgQueue gBootSchedMsgQ;    /* thread 5's scheduler messages */
#ifndef AVOID_UB
void gfxStartBuffer(void); /* FAKE: gfxbuf.h is not included here: gfxStartBuffer: used here as this file declares it (The number of arguments doesn't agree with the number in the declaration.); gfxbuf.h has `void gfxStartBuffer(s32 i)` */
#else
#include "gfxbuf.h"
#endif

/* The three graphics tasks, one per framebuffer: F3DEX2 through the FIFO, done message 2. */
void dmaInitGfxTasks(void) {
    u8 i;
    OSScTask *task;

    for (i = 0; i < 3; i++) {
        gGfxTaskDoneMsgs[i] = 2;
        task = &gGfxTasks[i];
        task->next = NULL;
        task->state = 0;
        task->flags = OS_SC_SWAPBUFFER;
        task->framebuffer = gFramebuffers[i];
        task->list.t.type = M_GFXTASK;
        task->list.t.flags = 0;
        task->list.t.ucode_boot = (u64 *) rspbootTextStart;
        task->list.t.ucode_boot_size = 0;
        task->list.t.ucode = (u64 *) gspF3DEX2_fifoTextStart;
        task->list.t.ucode_size = SP_UCODE_SIZE;
        task->list.t.ucode_data = (u64 *) gspF3DEX2_fifoDataStart;
        task->list.t.ucode_data_size = SP_UCODE_DATA_SIZE;
        task->list.t.dram_stack = gRspDramStack;
        task->list.t.dram_stack_size = SP_DRAM_STACK_SIZE8;
        task->list.t.output_buff = gRdpFifo;
        task->list.t.output_buff_size = 0;
        task->list.t.data_ptr = NULL;
        task->list.t.data_size = 0;
        task->list.t.yield_data_ptr = gRspYieldBuf;
        task->list.t.yield_data_size = OS_YIELD_DATA_SIZE;
        task->msgQ = &gBootSchedMsgQ;
        task->msg = &gGfxTaskDoneMsgs[i];
    }
}

void dmaBeginDisplayList(s32 fbIndex) {
    gDisplayListHead = gDisplayListBufs[fbIndex];
    gDisplayListStart = gDisplayListHead;
    gSPSegment(gDisplayListHead++, 0, 0);
}

/* Begin the frame's display list, then let the game draw into it. */
void dmaBeginFrame(s32 fbIndex) {
#ifndef AVOID_UB
    //! @bug gfxStartBuffer takes the buffer; the original passes none, and a0 still holds fbIndex.
    gfxStartBuffer();
#else
    gfxStartBuffer(fbIndex);
#endif
    gDisplayListHead = gDisplayListBufs[fbIndex];
    gDisplayListStart = gDisplayListHead;
    gSPSegment(gDisplayListHead++, 0, 0);
    gDisplayListHead = sysClearZBuffer(gDisplayListHead);
}

/* End the display list and send it to the scheduler as this framebuffer's graphics task. */
void dmaSubmitFrame(void) {
    OSScTask *task;

    gDPFullSync(gDisplayListHead++);
    gSPEndDisplayList(gDisplayListHead++);
    osWritebackDCacheAll();
    task = &gGfxTasks[mainSchedGetBufferIndex(&gMainSched)];
    task->list.t.data_ptr = (u64 *) gDisplayListStart;
    task->list.t.data_size = (gDisplayListHead - gDisplayListStart) * sizeof(Gfx);
    task->list.t.ucode_boot_size = (u8 *) rspbootTextEnd - (u8 *) rspbootTextStart;
    task->list.t.output_buff = gRdpFifo;
    task->list.t.output_buff_size = (u64 *) ((u32) gRdpFifo + 0x4000);
    task->flags = OS_SC_SWAPBUFFER;
    mainSchedSendTask(&gMainSched, gSchedGfxQ, task);
}

void dmaInitHandles(void) {
    gCartHandle = osCartRomInit();
    gSramHandlePtr = dmaSramInit();
}

/* The SRAM handle, as the SDK's osSramInit-style setup builds it: linked once. */
OSPiHandle *dmaSramInit(void) {
    if (gSramHandle.baseAddress == PHYS_TO_K1(0x08000000)) {
        return &gSramHandle;
    }
    gSramHandle.type = DEVICE_TYPE_SRAM;
    gSramHandle.baseAddress = PHYS_TO_K1(0x08000000);
    gSramHandle.latency = 5;
    gSramHandle.pulse = 12;
    gSramHandle.pageSize = 13;
    gSramHandle.relDuration = 2;
    gSramHandle.domain = PI_DOMAIN2;
    gSramHandle.speed = 0;
    bzero(&gSramHandle.transferInfo, sizeof(gSramHandle.transferInfo));
    osEPiLinkHandle(&gSramHandle);
    return &gSramHandle;
}

/* One synchronous PI DMA. A read also invalidates both caches over its target, since a read may
 * bring in code (the overlays). */
void dmaTransfer(OSPiHandle *handle, s32 direction, void *dram, u32 devAddr, s32 size) {
    if (direction == OS_READ) {
        osWritebackDCache(dram, size);
        osInvalDCache(dram, size);
        osInvalICache(dram, size);
    } else {
        osWritebackDCache(dram, size);
    }
    gDmaIoMesg.hdr.pri = OS_MESG_PRI_NORMAL;
    gDmaIoMesg.hdr.retQueue = &gDmaMsgQ;
    gDmaIoMesg.dramAddr = dram;
    gDmaIoMesg.devAddr = devAddr;
    gDmaIoMesg.size = size;
    osEPiStartDma(handle, &gDmaIoMesg, direction);
    osRecvMesg(&gDmaMsgQ, NULL, OS_MESG_BLOCK);
}

/* DmaRead: ROM to RAM. */
void dmaReadRom(u32 rom, void *ram, s32 size) {
    dmaTransfer(gCartHandle, OS_READ, ram, rom, size);
}

/* SRAM to RAM. */
void dmaReadSram(u32 sram, void *ram, s32 size) {
    dmaTransfer(gSramHandlePtr, OS_READ, ram, sram, size);
}

/* RAM to SRAM. */
void dmaWriteSram(void *ram, u32 sram, s32 size) {
    dmaTransfer(gSramHandlePtr, OS_WRITE, ram, sram, size);
}
