#ifndef SCHEDULER_H
#define SCHEDULER_H

/* The game's scheduler (src/main.c). It is built on the N64 SDK's sample scheduler: OSScTask and
 * OSScClient are the SDK's own (PR/sched.h; with _FINALROM, OSScTask is 0x58 bytes), but the
 * Sched struct is Konami's. */

#include "common.h"
#include <PR/sched.h>

#define SCHED_RETRACE_MSG 666 /* the event values the scheduler receives */
#define SCHED_SP_MSG 667
#define SCHED_DP_MSG 668
#define SCHED_PRENMI_MSG 669

typedef struct Sched {
    /* 0x000 */ s16 retraceMsg;  /* broadcast to every client on a retrace */
    /* 0x002 */ s16 prenmiMsg;   /* broadcast on PRE-NMI */
    /* 0x004 */ u8 pad4[0x4];
    /* 0x008 */ OSMesgQueue audioQ; /* audio tasks */
    /* 0x020 */ OSMesg audioBuf[8];
    /* 0x040 */ OSMesgQueue gfxQ; /* graphics tasks */
    /* 0x058 */ OSMesg gfxBuf[8];
    /* 0x078 */ OSMesgQueue swapQ; /* framebuffer swaps, read by mainSchedSwapThread */
    /* 0x090 */ OSMesg swapBuf[8];
    /* 0x0B0 */ OSMesgQueue eventQ; /* retrace / PRE-NMI events */
    /* 0x0C8 */ OSMesg eventBuf[8];
    /* 0x0E8 */ OSMesgQueue spQ; /* SP task done */
    /* 0x100 */ OSMesg spBuf[8];
    /* 0x120 */ OSMesgQueue dpQ; /* DP done */
    /* 0x138 */ OSMesg dpBuf[8];
    /* 0x158 */ u8 pad158[0x38];
    /* 0x190 */ OSMesgQueue gfxWaitQ;
    /* 0x1A8 */ OSMesg gfxWaitBuf[8];
    /* 0x1C8 */ OSMesgQueue waitQ;
    /* 0x1E0 */ OSMesg waitBuf[8];
    /* 0x200 */ OSThread eventThread; /* mainSchedEventThread */
    /* 0x3B0 */ OSThread thread3B0;   /* mainSchedAudioThread */
    /* 0x560 */ OSThread thread560;   /* mainSchedGfxThread */
    /* 0x710 */ OSThread swapThread;  /* mainSchedSwapThread */
    /* 0x8C0 */ OSScClient *clients;
    /* 0x8C4 */ OSScTask *curGfxTask;
    /* 0x8C8 */ OSScTask *curAudioTask;
    /* 0x8CC */ OSScTask *pendingGfxTask; /* a graphics task waiting for the audio task */
    /* 0x8D0 */ u32 bufferIndex; /* 0, 1, 2 */
    /* 0x8D4 */ u32 pendingSwaps; /* swaps outstanding (compared unsigned by thread 6) */
    /* 0x8D8 */ u32 viBlack;
    /* 0x8DC */ u32 firstSwap;
    /* 0x8E0 */ u64 unused8E0; /* 64-bit: zeroed as hi/lo word pairs (OSTime-like); never read */
    /* 0x8E8 */ u64 unused8E8;
    /* 0x8F0 */ u64 unused8F0;
    /* 0x8F8 */ u64 unused8F8;
} Sched;

extern Sched gMainSched;        /* the scheduler */
extern OSMesgQueue *gSchedGfxQ; /* its graphics-task queue */

void mainSchedCreate(Sched *sc, u8 viMode, u8 retraceCount);
OSMesgQueue *mainSchedGetAudioQ(Sched *sc);
OSMesgQueue *mainSchedGetGfxQ(Sched *sc);
s32 mainSchedGetBufferIndex(Sched *sc);
void mainSchedAddClient(Sched *sc, OSScClient *client, OSMesgQueue *msgQ);
void mainSchedRemoveClient(Sched *sc, OSScClient *client);
u32 mainSchedGetPending(Sched *sc);
void mainSchedSendTask(Sched *sc, OSMesgQueue *mq, OSScTask *task);

#endif
