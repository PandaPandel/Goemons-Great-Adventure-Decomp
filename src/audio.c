#include "common.h"
#include "scheduler.h"
#include "snd.h"
#include "audio.h"
#include "dma.h"
#include "impact.h"
#include "romdecode.h"
#include "sndtrack.h"

/* The audio manager: the N64 SDK's sample audiomgr.c, in its later form (an amConfig, and ROM
 * samples read with osEPiStartDma). The audio thread wakes on each retrace, builds an audio task
 * with alAudioFrame and sends it to the scheduler (main.c). Samples come through a pool of DMA
 * buffers, each returned to the free list FRAME_LAG frames after its last use. Konami's sound
 * driver follows it in this file. */

#define NUM_ACMD_LISTS 2
#define NUM_OUTPUT_BUFFERS 3
#define NUM_DMA_BUFFERS 39
#define DMA_BUFFER_LENGTH 0xA00
#define MAX_MESGS 64
#define FRAME_LAG 3
#define EXTRA_SAMPLES 240
#define AUDIO_RETRACE_MSG 1 /* the scheduler's messages (main.c) */
#define AUDIO_PRENMI_MSG 3
#define AUDIO_QUIT_MSG 0x20


typedef struct AMAudioMgr {
    /* 0x000 */ Acmd *ACMDList[NUM_ACMD_LISTS];
    /* 0x008 */ AudioInfo *audioInfo[NUM_OUTPUT_BUFFERS];
    /* 0x018 */ OSThread thread;
    /* 0x1C8 */ OSMesgQueue audioFrameMsgQ;
    /* 0x1E0 */ OSMesg audioFrameMsgBuf[MAX_MESGS];
    /* 0x2E0 */ OSMesgQueue audioReplyMsgQ;
    /* 0x2F8 */ OSMesg audioReplyMsgBuf[MAX_MESGS];
    /* 0x3F8 */ ALGlobals g;
} AMAudioMgr;

extern AMAudioMgr __am;             /* __am */
extern OSMesgQueue *sched_cmdQ;           /* the scheduler's command queue */
extern AMDMAState dmaState;             /* dmaState; the audio thread's stack ends here */
extern AMDMABuffer dmaBuffs[NUM_DMA_BUFFERS]; /* dmaBuffs */
extern u32 minFrameSize;                    /* minFrameSize */
extern u32 frameSize;                    /* frameSize */
extern u32 maxFrameSize;                    /* maxFrameSize */
extern u32 maxRSPCmds;                    /* amConfig.maxACMDSize */
extern OSIoMesg audDMAIOMesgBuf[];             /* audDMAIOMesgBuf */
extern OSMesgQueue audDMAMessageQ;            /* audDMAMessageQ */
extern OSMesg audDMAMessageBuf[MAX_MESGS];      /* audDMAMessageBuf */
extern OSPiHandle *gAmCartHandle;            /* the cartridge handle the samples are read with */
extern u32 audFrameCt;                    /* audFrameCt */
extern u32 nextDMA;                    /* nextDMA */
extern u32 curAcmdList;                    /* curAcmdList */


void audioNop(void) {
}

/* Nothing calls this one. */
void audioUnusedStub1(void) {
}

/* amCreateAudioMgr, which also starts the audio thread. */
void amCreateAudioMgr(ALSynConfig *c, OSPri pri, AmConfig *amc) {
    u32 i;
    f32 fsize;

    c->dmaproc = __amDmaNew;
    c->outputRate = osAiSetFrequency(amc->outputRate);
    i = 16;
    do {
        i--;
    } while (i != 0);
    fsize = (f32) amc->framesPerField * c->outputRate / 60;
    frameSize = (s32) fsize;
    if (frameSize < fsize) {
        frameSize++;
    }
    if (frameSize & 0xF) {
        frameSize = (frameSize & ~0xF) + 0x10;
    }
    minFrameSize = frameSize - 16;
    maxFrameSize = frameSize + EXTRA_SAMPLES + 16;
    alInit(&__am.g, c);
    dmaBuffs[0].node.prev = NULL;
    dmaBuffs[0].node.next = NULL;
    for (i = 0; i < NUM_DMA_BUFFERS - 1; i++) {
        alLink(&dmaBuffs[i + 1].node, &dmaBuffs[i].node);
        dmaBuffs[i].ptr = alHeapDBAlloc(NULL, 0, c->heap, 1, DMA_BUFFER_LENGTH);
    }
    dmaBuffs[i].ptr = alHeapDBAlloc(NULL, 0, c->heap, 1, DMA_BUFFER_LENGTH);
    for (i = 0; i < NUM_ACMD_LISTS; i++) {
        __am.ACMDList[i] = alHeapDBAlloc(NULL, 0, c->heap, 1, amc->maxACMDSize * sizeof(Acmd));
    }
    maxRSPCmds = amc->maxACMDSize;
    for (i = 0; i < NUM_OUTPUT_BUFFERS; i++) {
        __am.audioInfo[i] = alHeapDBAlloc(NULL, 0, c->heap, 1, sizeof(AudioInfo));
        __am.audioInfo[i]->msg.done.type = OS_SC_DONE_MSG;
        __am.audioInfo[i]->msg.done.info = __am.audioInfo[i];
        __am.audioInfo[i]->data = alHeapDBAlloc(NULL, 0, c->heap, 1, 4 * maxFrameSize);
    }
    osCreateMesgQueue(&__am.audioReplyMsgQ, __am.audioReplyMsgBuf, MAX_MESGS);
    osCreateMesgQueue(&__am.audioFrameMsgQ, __am.audioFrameMsgBuf, MAX_MESGS);
    osCreateMesgQueue(&audDMAMessageQ, audDMAMessageBuf, MAX_MESGS);
    osCreateThread(&__am.thread, 3, __amMain, NULL, &dmaState, pri);
    osStartThread(&__am.thread);
    gAmCartHandle = osCartRomInit();
}

/* __amMain: the audio thread. `pad` is a local the original has and never uses (its frame is
 * 8 bytes larger). */
void __amMain(void *arg) {
    s32 pad;
    u8 done = FALSE;
    AudioMsg *msg;
    AudioInfo *lastInfo = NULL;
    OSScClient client;

    mainSchedAddClient(&gMainSched, &client, &__am.audioFrameMsgQ);
    sched_cmdQ = mainSchedGetAudioQ(&gMainSched);
    while (!done) {
        osRecvMesg(&__am.audioFrameMsgQ, (OSMesg *) &msg, OS_MESG_BLOCK);
        switch (msg->gen.type) {
            case AUDIO_RETRACE_MSG:
                if (__amHandleFrameMsg(__am.audioInfo[audFrameCt % NUM_OUTPUT_BUFFERS], lastInfo)) {
                    osRecvMesg(&__am.audioReplyMsgQ, (OSMesg *) &msg, OS_MESG_BLOCK);
                    __amHandleDoneMsg(msg->done.info);
                    lastInfo = msg->done.info;
                }
                break;
            case AUDIO_PRENMI_MSG:
                break;
            case AUDIO_QUIT_MSG:
                done = TRUE;
                break;
        }
    }
    alClose(&__am.g);
}

/* __amHandleFrameMsg: play the last frame's buffer, and build this frame's task. Returns 0 if
 * there was nothing to do. */
s32 __amHandleFrameMsg(AudioInfo *info, AudioInfo *lastInfo) {
    s16 *audioPtr;
    Acmd *cmdp;
    s32 cmdLen;
    s32 samplesLeft = 0;
    OSScTask *t;

    __clearAudioDMA();
    audioPtr = (s16 *) osVirtualToPhysical(info->data);
    if (lastInfo) {
        osAiSetNextBuffer(lastInfo->data, lastInfo->frameSamples << 2);
    }
    samplesLeft = osAiGetLength() >> 2;
    info->frameSamples = (16 + (frameSize - samplesLeft + EXTRA_SAMPLES)) & ~0xF;
    if (info->frameSamples < minFrameSize) {
        info->frameSamples = minFrameSize;
    }
    cmdp = alAudioFrame(__am.ACMDList[curAcmdList], &cmdLen, audioPtr, info->frameSamples);
    if (cmdLen == 0) {
        return 0;
    }
    t = &info->task;
    t->next = NULL;
    t->msgQ = &__am.audioReplyMsgQ;
    t->msg = (OSMesg) &info->msg;
    t->list.t.data_ptr = (u64 *) __am.ACMDList[curAcmdList];
    t->list.t.data_size = (cmdp - __am.ACMDList[curAcmdList]) * sizeof(Acmd);
    t->list.t.type = M_AUDTASK;
    t->list.t.ucode_boot = (u64 *) rspbootTextStart;
    t->list.t.ucode_boot_size = (s32) rspbootTextEnd - (s32) rspbootTextStart;
    t->list.t.flags = 0;
    t->list.t.ucode = (u64 *) aspMainTextStart;
    t->list.t.ucode_data = (u64 *) aspMainDataStart;
    t->list.t.ucode_data_size = SP_UCODE_DATA_SIZE;
    t->list.t.yield_data_ptr = NULL;
    t->list.t.yield_data_size = 0;
    osSendMesg(sched_cmdQ, (OSMesg) t, OS_MESG_BLOCK);
    curAcmdList ^= 1;
    return 1;
}

/* The first words of this file's .data (0x80037C20, in ROM order; the rest is at the file's
 * end), defined here so that firstTime, __amHandleDoneMsg's static, follows them (IDO_NOTES 367). */
u32 audFrameCt = 0;
u32 nextDMA = 0;
u32 curAcmdList = 0;

/* __amHandleDoneMsg (its message printed nothing in the release build): the SDK's, with its
 * function-local `static s32 firstTime = 1;` (0x80037C2C). As one extern read and written, IDO
 * kept its address in a register (IDO_NOTES 38); as the static, a fresh lui for each access, as
 * the game has (a kept FAKE, measured). */
void __amHandleDoneMsg(AudioInfo *info) {
    s32 samplesLeft;
    static s32 firstTime = 1;

    samplesLeft = osAiGetLength() >> 2;
    if (samplesLeft == 0 && !firstTime) {
        firstTime = 0;
    }
}

/* __amDMA: a ROM address's samples, from a buffer that holds them or from a new DMA. */
s32 __amDMA(s32 addr, s32 len, void *state) {
    void *foundBuffer;
    s32 delta;
    s32 addrEnd;
    s32 buffEnd;
    AMDMABuffer *dmaPtr;
    AMDMABuffer *lastDmaPtr;

    lastDmaPtr = NULL;
    dmaPtr = dmaState.firstUsed;
    addrEnd = addr + len;
    while (dmaPtr) {
        buffEnd = dmaPtr->startAddr + DMA_BUFFER_LENGTH;
        if (dmaPtr->startAddr > (u32) addr) {
            break;
        } else if (addrEnd <= buffEnd) {
            dmaPtr->lastFrame = audFrameCt;
            foundBuffer = dmaPtr->ptr + addr - dmaPtr->startAddr;
            return (s32) osVirtualToPhysical(foundBuffer);
        }
        lastDmaPtr = dmaPtr;
        dmaPtr = (AMDMABuffer *) dmaPtr->node.next;
    }
    dmaPtr = dmaState.firstFree;
    dmaState.firstFree = (AMDMABuffer *) dmaPtr->node.next;
    alUnlink(&dmaPtr->node);
    if (lastDmaPtr) {
        alLink(&dmaPtr->node, &lastDmaPtr->node);
    } else if (dmaState.firstUsed) {
        lastDmaPtr = dmaState.firstUsed;
        dmaState.firstUsed = dmaPtr;
        dmaPtr->node.next = &lastDmaPtr->node;
        dmaPtr->node.prev = NULL;
        lastDmaPtr->node.prev = &dmaPtr->node;
    } else {
        dmaState.firstUsed = dmaPtr;
        dmaPtr->node.next = NULL;
        dmaPtr->node.prev = NULL;
    }
    foundBuffer = dmaPtr->ptr;
    delta = addr & 1;
    addr -= delta;
    dmaPtr->startAddr = addr;
    dmaPtr->lastFrame = audFrameCt;
    audDMAIOMesgBuf[nextDMA].hdr.pri = OS_MESG_PRI_NORMAL;
    audDMAIOMesgBuf[nextDMA].hdr.retQueue = &audDMAMessageQ;
    audDMAIOMesgBuf[nextDMA].dramAddr = foundBuffer;
    audDMAIOMesgBuf[nextDMA].devAddr = addr;
    audDMAIOMesgBuf[nextDMA].size = DMA_BUFFER_LENGTH;
    osEPiStartDma(gAmCartHandle, &audDMAIOMesgBuf[nextDMA++], OS_READ);
    return (s32) osVirtualToPhysical(foundBuffer) + delta;
}

/* __amDmaNew. */
ALDMAproc __amDmaNew(AMDMAState **state) {
    if (!dmaState.initialized) {
        dmaState.firstUsed = NULL;
        dmaState.firstFree = &dmaBuffs[0];
        dmaState.initialized = 1;
    }
    *state = &dmaState;
    return __amDMA;
}

/* __clearAudioDMA: collect the DMAs' messages, and free the buffers FRAME_LAG frames unused. */
void __clearAudioDMA(void) {
    u32 i;
    OSIoMesg *iomsg;
    AMDMABuffer *dmaPtr;
    AMDMABuffer *nextPtr;

    for (i = 0; i < nextDMA; i++) {
        osRecvMesg(&audDMAMessageQ, (OSMesg *) &iomsg, OS_MESG_NOBLOCK);
    }
    dmaPtr = dmaState.firstUsed;
    while (dmaPtr) {
        nextPtr = (AMDMABuffer *) dmaPtr->node.next;
        if (dmaPtr->lastFrame + FRAME_LAG < audFrameCt) {
            if (dmaState.firstUsed == dmaPtr) {
                dmaState.firstUsed = (AMDMABuffer *) dmaPtr->node.next;
            }
            alUnlink(&dmaPtr->node);
            if (dmaState.firstFree) {
                alLink(&dmaPtr->node, &dmaState.firstFree->node);
            } else {
                dmaState.firstFree = dmaPtr;
                dmaPtr->node.next = NULL;
                dmaPtr->node.prev = NULL;
            }
        }
        dmaPtr = nextPtr;
    }
    nextDMA = 0;
    audFrameCt++;
}

/* Konami's sound driver (include/snd.h): sounds are asked for through a queue of requests. */

typedef struct SndParam {
    /* 0x0 */ u16 id;
    /* 0x2 */ s16 value;
    /* 0x4 */ u8 kind;
} SndParam; /* size 0x6 */

typedef struct SndRange {
    /* 0x0 */ s16 min;
    /* 0x2 */ s16 max;
} SndRange;

typedef struct SndSe {
    /* 0x0 */ u8 prio;  /* low 5 bits: priority */
    /* 0x1 */ u8 flags; /* high nibble: track - 15 */
    /* 0x2 */ u8 pad2[2];
    /* 0x4 */ u8 *data;
} SndSe; /* size 0x8 */

typedef struct SndCmd {
    /* 0x0 */ u16 id;
    /* 0x2 */ u8 kind;
    /* 0x3 */ u8 ticks;
    /* 0x4 */ s16 target;
} SndCmd; /* size 0x6 */

typedef struct SndJingle {
    /* 0x0 */ u8 count;
    /* 0x1 */ u8 bank;
    /* 0x2 */ u8 pad2[2];
    /* 0x4 */ u8 **channels;
} SndJingle; /* size 0x8 */

typedef struct SndSeq {
    /* 0x0 */ u8 count; /* channels */
    /* 0x1 */ u8 firstTrack;
    /* 0x2 */ u8 prio;
    /* 0x3 */ u8 bank;
    /* 0x04 */ u8 startDelay;
    /* 0x08 */ u8 **channels;
    /* 0x0C */ s16 param0[3];
    /* 0x12 */ s16 slideTarget[3];
    /* 0x18 */ s16 slide[3];
    /* 0x1E */ s16 slideStep[3];
    /* 0x24 */ u8 slideTicks[3];
} SndSeq;

extern u32 gSndBankRomStart[SND_BANKS];           /* the banks' ROM starts */
extern u32 gSndBankRomEnd[SND_BANKS];           /* and ends */
extern u8 *gSndWaveTableRom[SND_BANKS];           /* their wave tables' ROM addresses */
extern s32 gSndReverbParams[];                    /* the reverb's parameters (AL_FX_CUSTOM) */
extern ALPlayer gSndPlayer;                 /* the driver's player */
extern ALHeap gAudioHeap;                   /* the audio heap */
extern ALBankFile *gSndBankFiles[SND_BANKS];   /* the banks, loaded */
extern u8 gAudioHeapBuf[];                     /* the audio heap's memory (0x35000 bytes) */
extern u8 gSndBankRam[];                          /* the banks' RAM */
extern ALVoice gSndVoices[SND_TRACKS];      /* a voice per track */
extern ALFxRef gSndFxRef;
extern u8 gSndStartDelay;                       /* ticks until the driver starts */
extern u8 gSndRequestLock;                       /* the request queue is being written */
extern u16 gSndTicks;                      /* ticks */
extern u8 gSndMusicPaused;
extern s8 gSndAllFadeMusicStep;
extern u8 gSndAllFadeMusicLevel;
extern s8 gSndAllFadeSeStep;
extern u8 gSndAllFadeSeLevel;
extern u8 gSndMono;                       /* muted */
extern u32 gSndRequestQueue[SND_REQUESTS];        /* the requests: id | pan << 16 | vol << 24 */
extern u16 gSndSlotSounds[4];                   /* the slots' sounds */
extern u16 gSndSpeedUp;
extern u8 gSndSpeedUpTicks;
extern s8 gSndMusicFadeStep;
extern u8 gSndMusicFadeLevel;
extern u8 gSndReverbSend[2];
extern u32 gSndCurRequest;                      /* the request being served */
extern u16 gSndRampRatesByCmd[];
extern SndSeq gSndNewSeq;                   /* the music playing */
extern SndParam gSndParamQueue[SND_REQUESTS];   /* parameter changes queued */
extern u8 gSndParamCount;                       /* how many */
extern u8 gSndLoadedMusic;
extern u8 gSndRequestCount;                       /* requests queued */
extern SndRange gSndParamRanges[3];              /* each kind's range */
extern SndFade gSndFades[2];
extern u16 gSndPendingMusic;
extern u8 gSndLoadDelay;
extern u8 *gSndSeqChannels[];                    /* its channels' data */

/* This file's .bss (0x800C3380-0x800C33FB). Defined here, as the game's were: as1 shares one
 * `lui at` between accesses to a symbol only when the file defines it. IDO lays .bss out by
 * each variable's first declaration (snd.h, then the externs above), so those stay in address
 * order. */
SndRamp gSndSlotRamps[4];
u16 gSndSlotSounds[4];
u16 gSndSpeedUp;
u8 gSndSpeedUpTicks;
s8 gSndMusicFadeStep;
u8 gSndMusicFadeLevel;
u8 gSndReverbSend[2];
SndSeq gSndNewSeq;
SndParam gSndParamQueue[SND_REQUESTS];
u8 gSndParamCount;
u8 gSndLoadedMusic;
u8 gSndRequestCount;
extern u8 gSndMusicBuf[];                          /* the music file loaded */
extern u8 gSndMusicInfoTable[][4];              /* per music id - 0x10: channels, ... */
extern SndSe gSndSe500Table[];              /* per sound effect id - 0x500 */
extern u8 gSndMusicGroupParts[];                 /* per music group - 1: parts */
extern u8 gSndMusicGroupIds[][6];              /* and their music ids */
extern u8 *gSndMusicParts[];                    /* the parts, loaded */
extern SndJingle gSndJingleTable[];          /* per id - 0xD0 */
extern SndCmd gSndEventQueue[];                 /* slide commands queued */
extern void (*gSndCmdTable[])(void);          /* the commands, ids 1-15 */
extern u8 gFileLoaderBusy;                       /* the file loader is busy */
extern u8 gSndLoadedGroup;
extern u32 gSndMusicRomStart[];                    /* per music id - 0x10: ROM start */
extern u32 gSndMusicRomEnd[];                    /* and end */
extern s32 gSndCheckFailCount;                      /* checksum failures */
extern u8 gSndSpawnCount;                       /* music changes queued */
extern SndSpawn gSndSpawnQueue[4];
extern u8 gSndEventCount;
extern u8 gSndGroupMusicId;
extern u8 gSndGroupMusicChanged;

extern SndSe *gSndSeTablesByPage[]; /* per 0x100 ids (music below 0x200, sounds 0x200-0x4FF) */

/* Start audio: load the sound banks, create the audio manager (44.1 kHz, the custom reverb), and
 * add the driver's player. Both ends are an empty count to 0x100000 in steps of 4, written
 * `while (TRUE)` with a break: as a for, while or do/while IDO unrolls it by 4 (steps of 16). The
 * banks are cleared through gSndBankRam itself, which lets IDO keep the ROM start in a3. */
void sndInit(void) {
    ALSynConfig c;
    AmConfig amc;
    u32 i;
    s32 b;
    u32 size;
    u32 offset;

    i = 0;
    while (TRUE) {
        i += 4;
        if (i == 0x100000) {
            break;
        }
    }
    alHeapInit(&gAudioHeap, gAudioHeapBuf, 0x35000);
    offset = 0;
    for (b = 0; b < SND_BANKS; b++) {
        size = gSndBankRomEnd[b] - gSndBankRomStart[b];
        gSndBankFiles[b] = (ALBankFile *) &gSndBankRam[offset];
        for (i = 0; i < size; i++) {
            gSndBankRam[offset + i] = 0;
        }
        dmaReadRom(gSndBankRomStart[b], gSndBankFiles[b], size);
        offset += size;
    }
    for (b = 0; b < SND_BANKS; b++) {
        alBnkfNew(gSndBankFiles[b], gSndWaveTableRom[b]);
    }
    c.maxVVoices = 16;
    c.maxPVoices = 16;
    c.maxUpdates = 160;
    c.dmaproc = NULL;
    c.outputRate = 0;
    c.heap = &gAudioHeap;
    c.fxType = AL_FX_CUSTOM;
    c.params = gSndReverbParams;
    amc.outputRate = 44100;
    amc.framesPerField = 1;
    amc.maxACMDSize = 0x1000;
    amCreateAudioMgr(&c, 12, &amc);
    sndInitPlayer(&gSndPlayer);
    i = 0;
    while (TRUE) {
        i += 4;
        if (i == 0x100000) {
            break;
        }
    }
}

/* Reset the driver's state and add its player. */
void sndInitPlayer(ALPlayer *player) {
    gSndStartDelay = 8;
    gSndTicks = gSndRequestCount = gSndMusicPaused = gSndMono = gSndLoadedMusic = 0;
    gSndAllFadeMusicLevel = gSndAllFadeMusicStep = 0;
    gSndAllFadeSeLevel = gSndAllFadeSeStep = 0;
    player->next = NULL;
    player->handler = sndPlayerProc;
    player->clientData = player;
    alSynAddPlayer(&alGlobals->drvr, player);
}

/* The player: after 8 ticks of start-up (the tracks are set up on the first), service the
 * driver. Called again in 5000 us. */
ALMicroTime sndPlayerProc(void *node) {
    if (gSndStartDelay != 0) {
        if (gSndStartDelay == 8) {
            sndInitVoices();
        }
        gSndStartDelay--;
    } else {
        if (gSndPendingMusic != 0) {
            sndLoadMusic();
        }
        if (gSndSpawnCount != 0) {
            sndServeMusicChanges();
        }
        if (gSndEventCount != 0) {
            sndServeCommands();
        }
        if (gSndRequestLock == 0) {
            sndServeRequests();
        }
        trkDriverTick();
        gSndTicks++;
        if (gSndGroupMusicChanged != 0) {
            sndPlayGroupMusic(1, gSndGroupMusicId, 0);
        }
    }
    return 5000;
}

/* Give each track a voice, started (silent) on its instrument's first sound. The bank, the
 * instrument and the sound are locals (they hold t0-t2, and the temporaries go round the rest);
 * `pad` is a fourth local the original has and never uses (its frame has the slot). */
void sndInitVoices(void) {
    ALVoiceConfig config;
    s32 i;
    s32 pad;
    ALBank *bank;
    ALInstrument *inst;
    ALSound *sound;

    for (i = 0; i < SND_TRACKS; i++) {
        config.priority = 10;
        config.fxBus = 0;
        config.unityPitch = 0;
        alSynAllocVoice(&alGlobals->drvr, &gSndVoices[i], &config);
        gSndCurTrack = &gSndTracks[i];
        gSndCurTrack->bank = gSndCurTrack->inst = 0;
        bank = gSndBankFiles[gSndCurTrack->bank]->bankArray[0];
        inst = bank->instArray[gSndCurTrack->inst];
        sound = inst->soundArray[0];
        gSndCurTrack->wave = sound->wavetable;
        gSndCurTrack->pitch = 1.0f;
        gSndCurTrack->voiceVol = 0;
        gSndCurTrack->voiceVolTicks = 0;
        gSndCurTrack->voiceOn = 1;
        alSynStartVoiceParams(&alGlobals->drvr, &gSndVoices[i], gSndCurTrack->wave, 1.0f, 0, 64, 0, 0);
    }
    sndStopAll();
    gSndFxRef = alSynGetFXRef(&alGlobals->drvr, 0, 0);
}

void sndPlay(u16 id) {
    sndRequest(id, 0, 0);
}

void sndCommand(u16 id) {
    sndRequest(id, 0, 0);
}

void sndPlayPan(u16 id, u8 pan) {
    sndRequest(id, pan, 0);
}

void sndPlayVol(u16 id, u8 vol) {
    sndRequest(id, 0, vol);
}

/* Queue a sound request (ignored if the same sound is already queued, or the queue is full): the
 * id in bits 0-15, a pan of 1-0x7F in bits 16-23, a volume in bits 24-31. */
void sndRequest(u16 id, u8 pan, u8 vol) {
    u32 req;
    s32 i;

    if (id) {
        gSndRequestLock = 0xFF;
        if (!pan || pan >= 0x80) {
            req = id;
        } else {
            req = id + (pan << 16);
        }
        if (vol != 0) {
            req |= vol << 24;
        }
        if (gSndRequestCount < SND_REQUESTS) {
            if (gSndRequestCount != 0) {
                for (i = 0; i < gSndRequestCount; i++) {
                    if (id == (gSndRequestQueue[i] & 0xFFFF)) {
                        gSndRequestLock = 0;
                        return;
                    }
                }
            }
            gSndRequestQueue[gSndRequestCount] = req;
            gSndRequestCount++;
        }
        gSndRequestLock = 0;
    }
}

/* Queue a parameter change for the tracks playing a sound: kind 0-2, applied by
 * sndApplyParam. */
void sndSetParam(u16 id, u8 kind, s16 value) {
    if (gSndParamCount < SND_REQUESTS && id != 0 && kind < 3) {
        gSndParamQueue[gSndParamCount].id = id;
        gSndParamQueue[gSndParamCount].value = value;
        gSndParamQueue[gSndParamCount].kind = kind;
        gSndParamCount++;
        gSndRequestLock = 0;
    }
}

/* Parameter change i's value for the current track: scaled by the track's own factor (in tenths)
 * and added to its base, if it has one.
 * The scale is read from the track twice, not kept in a local, and the sum is built in `base`. */
s32 sndParamValue(u8 i, u8 kind) {
    s32 base;

    if (gSndCurTrack->scale[kind] != 0) {
        if (kind == 0) {
            base = gSndCurTrack->base[kind] << 8;
        } else {
            base = gSndCurTrack->base[kind];
        }
        base += gSndParamQueue[i].value * gSndCurTrack->scale[kind] / 10;
        return base;
    }
    return gSndParamQueue[i].value;
}

/* Apply parameter change i to every track playing its sound, clamped to the kind's range. The
 * value is clamped untruncated (`v` an s32) and truncated only to compare it with the track's. */
void sndApplyParam(s32 n) {
    SndParam *p = &gSndParamQueue[n];
    s32 kind = p->kind;
    s32 i;
    SndRange *r;
    s32 v;

    for (i = 0; i < SND_TRACKS; i++) {
        gSndCurTrack = &gSndTracks[i];
        if (p->id == (gSndCurTrack->id & 0x7FFF)) {
            r = &gSndParamRanges[kind];
            v = sndParamValue(n, kind);
            if (v < r->min) {
                v = r->min;
            } else if (r->max < v) {
                v = r->max;
            }
            if ((s16) v != gSndCurTrack->param[kind]) {
                gSndCurTrack->param[kind] = v;
            }
        }
    }
}

void sndSetReverb(u8 which, u8 v) {
    if (v >= 13) {
        v = 12;
    }
    if (which < 2) {
        gSndReverbSend[which] = v;
        return;
    }
    gSndReverbSend[0] = v;
    gSndReverbSend[1] = v;
}

void sndClearReverb(void) {
    gSndReverbSend[1] = 0;
    gSndReverbSend[0] = 0;
}

s32 sndRampLevel(void) {
    s32 n = gSndMusicRamp.level + gSndAllRamp.level;

    if (n == 0 && gSndTracks[0].id != 0 && gSndTracks[0].id < 0x100) {
        n = -1;
    }
    return n;
}

/* For the first track playing a sound: its volume (which 0) or pitch, never 0; or 0. */
s32 sndGetVolume(u16 id, s32 which) {
    s32 i;
    s32 v;

    for (i = 0; i < SND_TRACKS; i++) {
        if (id == (gSndTracks[i].id & 0x7FFF)) {
            if (which == 0) {
                v = gSndCurTrack[i].voiceVol >> 7;
            } else {
                v = gSndCurTrack[i].volume >> 8;
            }
            if (v == 0) {
                v++;
            }
            return v;
        }
    }
    return 0;
}

/* The first track playing one of the sounds below 0x100, or 0. */
u16 sndGetMusic(void) {
    s32 i;

    for (i = 0; i < SND_TRACKS; i++) {
        if (gSndTracks[i].id != 0 && gSndTracks[i].id < 0x100) {
            return gSndTracks[i].id;
        }
    }
    return 0;
}

/* Start fade i: out (from full, if at 0), or in (from 0x800). */
void sndStartFade(s32 i, s32 in) {
    if (in == 0) {
        if (gSndFades[i].level == 0) {
            gSndFades[i].level = 0x7FFF;
        }
        gSndFades[i].rate = 17;
        gSndFades[i].rate = -gSndFades[i].rate;
        return;
    }
    if (gSndFades[i].level == 0) {
        gSndFades[i].level = 0x800;
    }
    gSndFades[i].rate = 20;
}

/* Crossfade: fade 0 in (1) or out (0), fade 1 the other way. */
void sndCrossfade(s32 in) {
    if (in >= 2) {
        in = 1;
    }
    sndStartFade(0, in);
    sndStartFade(1, 1 - in);
}

/* Serve the requests queued since the last tick: ids below 0x10 are commands (a table of
 * functions), below 0x100 sound effects, below 0x8000 music, and the rest music slots. Then apply
 * the queued parameter changes. */
void sndServeRequests(void) {
    s32 i = 0;
    u16 id;

    while (gSndRequestCount != 0) {
        gSndCurRequest = gSndRequestQueue[i++];
        gSndRequestCount--;
        id = gSndCurRequest & 0xFFFF;
        if (id < 0x100) {
            if (id < 0x10) {
                gSndCmdTable[id - 1]();
            } else {
                sndStartMusic();
            }
        } else if (id < 0x8000) {
            sndStartRequest(0);
        } else {
            sndAddSlot();
        }
    }
    i = 0;
    while (gSndParamCount != 0) {
        sndApplyParam(i);
        i++;
        gSndParamCount--;
    }
}

/* Serve the queued music changes, the last first: each holds three parameters packed above
 * three sign bits, every one halved until it is within its limit (0: cleared), and the three
 * slides to start; a request for music (0x100 and up) is then served too. The first limit is
 * held in `i` (an s16, so `(s16) (max << 8)`), the counter of the slide loop below. */
void sndServeMusicChanges(void) {
    SndLimit *lim;
    u16 flags;
    s16 a;
    s16 p0;
    s16 p1;
    s16 p2;
    s32 max;
    s16 i;

    while (gSndSpawnCount != 0) {
        gSndSpawnCount--;
        flags = gSndSpawnQueue[gSndSpawnCount].r;
        lim = gSndSpawnQueue[gSndSpawnCount].lim;
        a = flags >> 3;
        p0 = a & 0x3FFF;
        a >>= 3;
        p1 = a & 0xFF;
        a >>= 3;
        p2 = a & 0x7F;
        if (lim->max[0] != 0) {
            i = lim->max[0] << 8;
            while (i < p0) {
                p0 >>= 1;
            }
        } else {
            p0 = 0;
        }
        if (lim->max[1] != 0) {
            max = lim->max[1];
            while (max < p1) {
                p1 >>= 1;
            }
        } else {
            p1 = 0;
        }
        if (lim->max[2] != 0) {
            max = lim->max[2];
            while (max < p2) {
                p2 >>= 1;
            }
        } else {
            p2 = 0;
        }
        if (flags & 1) {
            p0 = -p0;
        }
        if (flags & 2) {
            p1 = -p1;
        }
        if (flags & 4) {
            p2 = -p2;
        }
        gSndCurRequest = (gSndSpawnQueue[gSndSpawnCount].sound << 16) + lim->id;
        gSndNewSeq.param0[0] = p0;
        gSndNewSeq.param0[1] = p1;
        gSndNewSeq.param0[2] = p2;
        for (i = 0; i < 3; i++) {
            gSndNewSeq.slideTarget[i] = gSndSpawnQueue[gSndSpawnCount].slideTarget[i];
            gSndNewSeq.slide[i] = gSndSpawnQueue[gSndSpawnCount].slide[i];
            gSndNewSeq.slideStep[i] = gSndSpawnQueue[gSndSpawnCount].slideStep[i];
            gSndNewSeq.slideTicks[i] = gSndSpawnQueue[gSndSpawnCount].slideTicks[i];
        }
        if ((gSndCurRequest & 0x7FFF) >= 0x100) {
            sndStartRequest(1);
        }
    }
}

/* Serve the queued commands, at most 8, the last first: by kind, 0 a request (0x10-0xFF a
 * sound effect, 0x100 and up music), 1 a stop (over the command's ticks), 2-4 a slide of
 * that kind. */
void sndServeCommands(void) {
    u32 id;

    if (gSndEventCount > 8) {
        gSndEventCount = 8;
    }
    while (gSndEventCount) {
        gSndEventCount--;
        id = gSndEventQueue[gSndEventCount].id;
        switch (gSndEventQueue[gSndEventCount].kind) {
            case 0:
                gSndCurRequest = id;
                if (id < 0x100) {
                    if (id >= 0x10) {
                        sndStartMusic();
                    }
                } else {
                    sndStartRequest(0);
                }
                break;
            case 1:
                sndStopSound(id);
                break;
            case 2:
                sndStartSlide(0, id);
                break;
            case 3:
                sndStartSlide(1, id);
                break;
            case 4:
                sndStartSlide(2, id);
                break;
        }
    }
}

/* Stop the tracks playing a sound (over 18 tracks): over the command's ticks, as a fade of
 * 0x100/ticks a step (at least 1), or at once. A sound effect (below 0x100) stops everything,
 * if the first track is playing it. */
void sndStopSound(u32 id) {
    s32 i;

    if (gSndEventQueue[gSndEventCount].ticks != 0) {
        for (i = 0; i < 18; i++) {
            gSndCurTrack = &gSndTracks[i];
            if (id == (gSndCurTrack->id & 0x7FFF)) {
                gSndCurTrack->stopFadeLevel = 0xFF;
                gSndCurTrack->stopFadeStep = 0x100 / gSndEventQueue[gSndEventCount].ticks;
                if (gSndCurTrack->stopFadeStep == 0) {
                    gSndCurTrack->stopFadeStep = 1;
                }
            }
        }
        return;
    }
    if (id < 0x100) {
        if (id == (gSndTracks[0].id & 0x7FFF)) {
            sndStopMusic();
        }
    } else {
        for (i = 0; i < 18; i++) {
            if (id == (gSndTracks[i].id & 0x7FFF)) {
                sndStopTrack(i);
            }
        }
    }
}

/* Start a slide of kind 0-2 on every track playing a sound: toward the target in the command
 * being served (gSndEventQueue[gSndEventCount]), over its number of ticks. The step is worked out in one
 * variable (`step = difference; step /= ticks;`) before the stores. */
void sndStartSlide(s32 kind, u32 id) {
    u8 ticks = gSndEventQueue[gSndEventCount].ticks;
    s16 target = gSndEventQueue[gSndEventCount].target;
    s32 i;
    s32 step;

    for (i = 0; i < SND_TRACKS; i++) {
        gSndCurTrack = &gSndTracks[i];
        if (id == (gSndCurTrack->id & 0x7FFF) && target != gSndCurTrack->slide[kind]) {
            step = target - gSndCurTrack->slide[kind];
            step /= ticks;
            gSndCurTrack->slideTicks[kind] = ticks;
            gSndCurTrack->slideTarget[kind] = target;
            gSndCurTrack->slideStep[kind] = step;
        }
    }
}

/* Start the request being served (gSndCurRequest), if its priority is at least its track's: 0x500
 * and up through sndStartSe; 0x200-0x4FF a sound on its own track (0: on tracks 10-15, the
 * one already playing it, or else the free-est of group 1; 0x360-0x3FF one of their kind on its
 * side); below 0x200 music, of one or two channels (two: tracks 14/12/10, or 0x1C2-0x1FF on its
 * side, and the second track's priority too). The entry is read as `tbl[id & 0xFF]` each time
 * (no pointer to it), and `track` also holds the priority byte and the flags on their way. */
void sndStartRequest(s32 mode) {
    s32 id = gSndCurRequest & 0x7FFF;
    SndSe *tbl;
    u8 track;
    s32 i;
    s32 pad[2];

    if (id >= 0x500) {
        sndStartSe();
        return;
    }
    if (id >= 0x200) {
        if (id >= 0x500) {
            return;
        }
        tbl = gSndSeTablesByPage[id >> 8];
        gSndNewSeq.prio = tbl[id & 0xFF].prio;
        gSndNewSeq.prio &= 0x1F;
        track = tbl[id & 0xFF].flags;
        gSndNewSeq.bank = track & 0xF;
        track = track >> 4;
        if (id >= 0x360 && id < 0x400) {
            track = sndFindSideTrack();
            if (track == 0) {
                track = sndLowestPrioTrack(1);
            }
        } else if (track == 0) {
            for (i = 10; i < 16; i++) {
                if (gSndNewSeq.bank & 8) {
                    if (gSndCurRequest == gSndTracks[i].id) {
                        track = i;
                        break;
                    }
                } else if (id == (gSndTracks[i].id & 0x7FFF)) {
                    track = i;
                    break;
                }
            }
            if (track == 0) {
                track = sndLowestPrioTrack(1);
            }
        }
        if (gSndNewSeq.prio < gSndTracks[track].prio) {
            return;
        }
        gSndNewSeq.firstTrack = track;
        gSndNewSeq.count = 1;
        gSndNewSeq.channels = gSndSeqChannels;
        gSndSeqChannels[0] = tbl[id & 0xFF].data;
    } else if (id < 0x200) {
        tbl = gSndSeTablesByPage[id >> 8];
        track = tbl[id & 0xFF].prio;
        gSndNewSeq.prio = track & 0x1F;
        gSndNewSeq.count = (track >> 5) + 1;
        track = tbl[id & 0xFF].flags;
        gSndNewSeq.bank = track & 0xF;
        track = track >> 4;
        if (gSndNewSeq.count == 2) {
            if (id >= 0x1C2 && id < 0x200) {
                track = sndFindSideTrack();
                if (track == 0) {
                    track = sndLowestPrioTrack(2);
                }
            } else if (track == 0) {
                for (i = 14; i > 8; i -= 2) {
                    if (gSndNewSeq.bank & 8) {
                        if (gSndCurRequest == gSndTracks[i].id) {
                            track = i;
                            break;
                        }
                    } else if (id == (gSndTracks[i].id & 0x7FFF)) {
                        track = i;
                        break;
                    }
                }
                if (track == 0) {
                    track = sndLowestPrioTrack(2);
                }
            }
            if (gSndNewSeq.prio < gSndTracks[track].prio) {
                return;
            }
            if (gSndNewSeq.prio < gSndTracks[track + 1].prio) {
                return;
            }
        } else if (gSndNewSeq.prio < gSndTracks[track].prio) {
            return;
        }
        gSndNewSeq.firstTrack = track;
        gSndNewSeq.channels = (u8 **) tbl[id & 0xFF].data;
    } else {
        return;
    }
    gSndNewSeq.bank &= 7;
    gSndNewSeq.startDelay = 6;
    sndStartSequence(mode);
}

/* A track (8-15) already playing a sound in 0x360-0x3FF or 0x1C2-0x1FF on the current sound's
 * side (bit 16), or 0. `tside` holds the track's whole id, then only its side; `id` is a u16. */
u8 sndFindSideTrack(void) {
    u8 i;
    u16 id;
    u32 side = gSndCurRequest & 0x10000;
    u32 tside;

    for (i = 8; i < SND_TRACKS; i++) {
        tside = gSndTracks[i].id;
        id = tside & 0x7FFF;
        tside &= 0x10000;
        if (id >= 0x360 && id < 0x400 && side == tside) {
            return i;
        }
        if (id >= 0x1C2 && id < 0x200 && id < 0x200 && side == tside) {
            return i;
        }
    }
    return 0;
}

/* Start the sound effect asked for (ids 0x500-0x566), on the track its entry names (tracks 15 and
 * up mean: the one of tracks 16-17 already playing it, or else the free-est of group 0), if its
 * priority is at least that track's. */
void sndStartSe(void) {
    u16 id = gSndCurRequest & 0x7FFF;
    SndSe *se;
    s32 i;
    u8 track;

    if (id < 0x567) {
        se = &gSndSe500Table[id - 0x500];
        track = se->flags >> 4;
        track += 15;
        gSndNewSeq.prio = se->prio;
        gSndNewSeq.prio &= 0x1F;
        if (track == 15) {
            for (i = 16; i < 18; i++) {
                if (id == (gSndTracks[i].id & 0x7FFF)) {
                    track = i;
                    break;
                }
            }
            if (track == 15) {
                track = sndLowestPrioTrack(0);
            }
        }
        if (gSndNewSeq.prio >= gSndTracks[track].prio) {
            gSndNewSeq.firstTrack = track;
            gSndNewSeq.count = 1;
            gSndNewSeq.channels = gSndSeqChannels;
            gSndSeqChannels[0] = se->data;
            gSndNewSeq.bank = 0;
            gSndNewSeq.startDelay = 0;
            sndStartSequence(0);
        }
    }
}

/* Nothing calls this one. */
void audioUnusedStub2(void) {
}

/* The track in a group whose sound has the lowest priority (the last of equals): group 1 is
 * tracks 10-15, group 0 tracks 16-17; any other group compares the pairs 14/15, 12/13, 10/11 by
 * the higher of each. */
u8 sndLowestPrioTrack(u8 group) {
    u8 best;
    u8 min;
    u8 i;
    u8 j;

    if (group == 1) {
        best = 10;
        min = gSndTracks[best].prio;
        for (i = 11; i < 16; i++) {
            if (gSndTracks[i].prio <= min) {
                best = i;
                min = gSndTracks[i].prio;
            }
        }
    } else if (!group) {
        best = 16;
        min = gSndTracks[best].prio;
        for (i = 17; i < 18; i++) {
            if (gSndTracks[i].prio <= min) {
                best = i;
                min = gSndTracks[i].prio;
            }
        }
    } else {
        best = 14;
        min = 0xFF;
        for (i = 14; i >= 10; i -= 2) {
            j = i;
            if (gSndTracks[i].prio < gSndTracks[i + 1].prio) {
                j++;
            }
            if (gSndTracks[j].prio <= min) {
                best = i;
                min = gSndTracks[j].prio;
            }
        }
    }
    return best;
}

/* Start music gSndCurRequest (0x10-0xFF). Unless it belongs to the group playing, stop what plays (or
 * fade it, if the second ramp runs). Music already loaded starts at once; otherwise it is queued
 * for sndLoadMusic with a countdown of 4. Ids 0xC3-0xCF do nothing; the path for ids from 0xD0
 * (a table in overlay 6) is guarded by `id < 0xC1` and so never runs. */
void sndStartMusic(void) {
    u16 id = gSndCurRequest & 0x7FFF;
    u8 group = gSndMusicInfoTable[id - 0x10][2];

    if (group == 0 || group != gSndLoadedGroup) {
        if (gSndAllRamp.level != 0) {
            sndStopAll();
        } else {
            sndStopMusic();
        }
    }
    if (id < 0xD0) {
        if (id < 0xC3) {
            if (gSndMusicInfoTable[id - 0x10][2] != 0) {
                if (gSndLoadedGroup != gSndMusicInfoTable[id - 0x10][2]) {
                    gSndPendingMusic = id;
                    gSndLoadDelay = 4;
                } else {
                    sndPlayGroupMusic(0, id, 4);
                }
            } else if (id != gSndLoadedMusic) {
                gSndPendingMusic = id;
                gSndLoadDelay = 4;
            } else {
                sndPlayMusic(id, 6);
            }
        }
    } else if (id < 0xC1) {
        id -= 0xD0;
        gSndNewSeq.count = gSndJingleTable[id].count;
        gSndNewSeq.bank = gSndJingleTable[id].bank;
        gSndNewSeq.channels = gSndJingleTable[id].channels;
        gSndNewSeq.firstTrack = 0;
        gSndNewSeq.prio = 0x1F;
        gSndNewSeq.startDelay = 6;
        sndStartSequence(0);
    }
}

/* Check block n of a table: two big-endian u16s (length, sum), then the bytes they cover.
 * Counts failures in gSndCheckFailCount. */
s32 sndCheckBlock(u8 n, u8 *table) {
    u16 len;
    u16 check;
    u16 sum;
    u16 i;

    table += n * 2;
    len = table[0];
    len <<= 8;
    len += table[1];
    check = table[2];
    check <<= 8;
    check += table[3];
    table += 4;
    sum = 0;
    for (i = 0; i < len; i++) {
        sum += *table++;
    }
    if (check == sum) {
        gSndCheckFailCount = 0;
        return 1;
    }
    gSndCheckFailCount++;
    return 0;
}

/* Load the music asked for (gSndPendingMusic) once its countdown ends: decode it from ROM (Konami LZ)
 * to 0x80363FF8, check its sum, and start it. While the file loader is busy, or the decode fails,
 * try again next tick; after 5 bad sums, give up. Music with its third table byte set is loaded by
 * sndLoadMusicParts instead. */
void sndLoadMusic(void) {
    u8 *info;
    u16 i;

    if (--gSndLoadDelay == 0) {
        if (gFileLoaderBusy != 0) {
            gSndLoadDelay = 1;
            return;
        }
        i = gSndPendingMusic - 0x10;
        info = gSndMusicInfoTable[i];
        gSndLoadedMusic = gSndPendingMusic;
        gSndLoadedGroup = info[2];
        if (gSndLoadedGroup != 0) {
            sndLoadMusicParts();
            return;
        }
        if (romDecode(gSndMusicRomStart[i], gSndMusicBuf, gSndMusicRomEnd[i] - gSndMusicRomStart[i]) == NULL) {
            gSndLoadDelay = 1;
            return;
        }
        if (!sndCheckBlock(info[0], gSndMusicBuf)) {
            if (gSndCheckFailCount >= 5) {
                gSndPendingMusic = 0;
                gSndLoadedMusic = 0;
                gSndLoadedGroup = 0;
                return;
            }
            gSndLoadDelay = 1;
            return;
        }
        sndPlayMusic(gSndPendingMusic, 5);
        gSndPendingMusic = 0;
    }
}

/* Start music: its channel count comes from overlay 6's table, and each channel's data from the
 * offsets at the start of the loaded file. */
void sndPlayMusic(u16 id, u8 delay) {
    u16 *src = (u16 *) gSndMusicBuf;
    s32 i;

    gSndNewSeq.count = gSndMusicInfoTable[(u16) (id - 0x10)][0];
    gSndNewSeq.bank = gSndMusicInfoTable[(u16) (id - 0x10)][1];
    gSndNewSeq.channels = gSndSeqChannels;
    for (i = 0; i < gSndNewSeq.count; i++) {
        gSndSeqChannels[i] = (u8 *) ((s32) gSndMusicBuf + src[i]);
    }
    gSndCurRequest = id;
    gSndNewSeq.firstTrack = 0;
    gSndNewSeq.prio = 0x1F;
    gSndNewSeq.startDelay = delay;
    sndStartSequence(0);
}

/* Load music in parts: group gSndLoadedGroup (from 1) lists up to 6 music ids in overlay 6. Each part
 * is decoded after the last (16-byte aligned) from 0x80363FF8 and checked, as sndLoadMusic does
 * for one; the parts' addresses go to gSndMusicParts. Then play them. The size is computed in its
 * own statement first: that puts the end table's address in s6 and the start table's in s7. */
void sndLoadMusicParts(void) {
    s32 g = gSndLoadedGroup - 1;
    u8 *dst = gSndMusicBuf;
    u8 *end;
    s32 k;
    u16 i;
    s32 size;

    for (k = 0; k < gSndMusicGroupParts[g]; k++) {
        i = gSndMusicGroupIds[g][k] - 0x10;
        gSndMusicParts[k] = dst;
        size = gSndMusicRomEnd[i] - gSndMusicRomStart[i];
        end = romDecode(gSndMusicRomStart[i], dst, size);
        if (end == NULL) {
            gSndLoadDelay = 1;
            return;
        }
        if (!sndCheckBlock(gSndMusicInfoTable[i][0], dst)) {
            if (gSndCheckFailCount >= 5) {
                gSndPendingMusic = 0;
                gSndLoadedMusic = 0;
                gSndLoadedGroup = 0;
                return;
            }
            gSndLoadDelay = 1;
            return;
        }
        dst = end - ((u32) end & 0xF) + 0x10;
    }
    sndPlayGroupMusic(0, gSndPendingMusic, 4);
    gSndPendingMusic = 0;
}

/* Play grouped music from its loaded part (the entry's fourth byte). Asked for with mode 0, it is
 * remembered in gSndGroupMusicId and waits while track 0 plays; otherwise it needs a group.
 * Matched 2026-10-02 by the permuter's FAKE index on `count` (it schedules `info[3]` and `count`
 * above the `channels` store, as the original does). Natural forms measured before: `part =
 * info[3];` first (32), reusing `i` (42), `base` before `channels` (20), direct table reads (25),
 * a block split after the loop (22). */
void sndPlayGroupMusic(u8 mode, u16 id, u8 delay) {
    s32 pad[2];
    u8 *info;
    u8 *base;
    s32 i;

    gSndGroupMusicChanged = 0;
    if (mode != 0) {
        if (gSndMusicInfoTable[(u16) (id - 0x10)][2] == 0) {
            return;
        }
    } else {
        gSndGroupMusicId = id;
        if (gSndTracks[0].id != 0) {
            return;
        }
    }
    if (gSndGroupMusicId != 0) {
        info = gSndMusicInfoTable[(u16) (id - 0x10)];
        sndStopMusic();
        /* FAKE (permuter): "* 0" re-reads the entry's byte 2 to schedule the loads */
        gSndNewSeq.count = info[(gSndMusicInfoTable[(u16) (id - 0x10)][2] == 0) * 0];
        gSndNewSeq.bank = info[1];
        gSndNewSeq.channels = gSndSeqChannels;
        base = gSndMusicParts[info[3]];
        for (i = 0; i < gSndNewSeq.count; i++) {
            gSndSeqChannels[i] = base + ((u16 *) base)[i];
        }
        gSndCurRequest = id;
        gSndNewSeq.firstTrack = 0;
        gSndNewSeq.prio = 0x1F;
        gSndNewSeq.startDelay = delay;
        sndStartSequence(0);
    }
}

/* Start the sequence set up in gSndNewSeq on its tracks, one per channel from track unk1: each
 * gets the sound's id, priority, bank and channel data, the effect mix of its side (music or
 * sounds), and fresh state. Mode 0 starts clean; otherwise the sequence's values and slides carry
 * over. Then the music slot holding this sound, if any, is freed.
 * The u8 chains (unkF = unkE, unk1D = unk47) each spend a temp register, and the two chains on
 * one line come out in the game's store order. The slots' check at the end is a loop, unrolled
 * into the game's absolute loads. */
void sndStartSequence(s32 mode) {
    s32 pad[4];
    s32 side;
    s32 k;
    s32 n;
    s32 pad2;
    u8 **ch;

    if (mode == 0) {
        gSndNewSeq.param0[0] = 0;
        gSndNewSeq.param0[1] = 0;
        gSndNewSeq.param0[2] = 0;
    }
    if ((gSndCurRequest & 0x7FFF) < 0x100) {
        side = 0;
    } else {
        side = 1;
    }
    ch = gSndNewSeq.channels;
    for (k = 0; k < gSndNewSeq.count; k++) {
        SndTrack *t;
        SndTrack *p;

        n = gSndNewSeq.firstTrack + k;
        p = t = &gSndTracks[n];
        if (n < SND_TRACKS) {
            if (t->fxmix != gSndReverbSend[side]) {
                t->fxmix = gSndReverbSend[side];
                alSynSetFXMix(&alGlobals->drvr, &gSndVoices[n], t->fxmix);
            }
            if (side != 0 && t->voiceOn != 0) {
                alSynSetVol(&alGlobals->drvr, &gSndVoices[n], 0, 5000);
                t->voiceStopTicks = 2;
            }
        } else {
            t->voiceStopTicks = t->voiceOn = 0;
        }
        t->startDelay = gSndNewSeq.startDelay;
        t->id = gSndCurRequest;
        t->prio = gSndNewSeq.prio;
        t->bank = gSndNewSeq.bank;
        t->data = ch[k];
        t->tempo = 0x8000;
        t->tempoAcc = 0x80;
        t->waitTicks = t->gate = 1;
        t->transpose = t->detune = 0;
        t->loopDetune = t->loop2Detune = 0;
        t->loopVelocity = t->loop2Velocity = 0;
        p->portaTicks = p->bendAmount = 0;
        p->wobbleSpeed = 0;
        p->wobbleOffset = 0;
        p->tempoSlideTicks = p->volSlideTicks = 0; p->vibratoDepth = p->vibratoRiseTicks = 0;
        p->panSlideTicks = 0;
        p->pan = 0;
        for (n = 0; n < 3; n++) {
            p->param[n] = p->param0[n] = gSndNewSeq.param0[n];
        }
        for (n = 0; n < 3; n++) {
            p->scale[n] = 0;
            p->base[n] = 0;
        }
        p->stopFadeLevel = 0;
        for (n = 0; n < 3; n++) {
            if (mode == 0) {
                p->slide[n] = 0;
                p->slideTicks[n] = 0;
            } else {
                p->slideTarget[n] = gSndNewSeq.slideTarget[n];
                p->slide[n] = gSndNewSeq.slide[n];
                p->slideStep[n] = gSndNewSeq.slideStep[n];
                p->slideTicks[n] = gSndNewSeq.slideTicks[n];
            }
        }
        p->stateMuted = 0;
        p->fadeLevel = 0;
        p->noCrossfade = 0;
    }
    for (n = 0; n < 4; n++) {
        if ((gSndCurRequest & 0x7FFF) == gSndSlotSounds[n]) {
            gSndSlotSounds[n] = 0;
            gSndSlotRamps[n].level = 0;
        }
    }
}

/* Fade everything: while the second ramp runs, every track but sound 0x45F's is set to sound 0x10,
 * otherwise all 18 are; then stop everything, and clear the slots, their ramps and the second.
 * The slots' and ramps' stores share their luis because this file defines them. */
void sndStopAll(void) {
    s32 i;
    s32 j;

    gSndSpawnCount = 0;
    gSndEventCount = 0;
    gSndParamCount = 0;
    gSndReverbSend[0] = gSndReverbSend[1] = 0;
    if (gSndAllRamp.level != 0) {
        for (i = 0; i < 18; i++) {
            if ((gSndTracks[i].id & 0x7FFF) != 0x45F) {
                gSndTracks[i].id = 0x10;
            }
        }
    } else {
        for (i = 0; i < 18; i++) {
            gSndTracks[i].id = 0x10;
        }
    }
    sndStopMusic();
    for (i = 0; i < 4; i++) {
        gSndSlotSounds[i] = 0;
    }
    j = 0;
    do {
        gSndSlotRamps[j].level = 0;
    } while (++j < 4);
    gSndAllRamp.level = 0;
    gSndAllFadeMusicLevel = gSndAllFadeMusicStep = 0;
    gSndAllFadeSeLevel = gSndAllFadeSeStep = 0;
}

/* Stop everything: the fades, ramps and flags cleared, and every track playing a sound in
 * 0x10-0xFF stopped (over 18 tracks). */
void sndStopMusic(void) {
    s32 i;
    u32 id;

    gSndGroupMusicChanged = 0;
    gSndGroupMusicId = 0;
    gSndFades[1].level = 0;
    gSndFades[0].level = 0;
    gSndPendingMusic = 0;
    gSndLoadDelay = 0;
    gSndMusicRamp.level = 0;
    gSndSpeedUpTicks = gSndSpeedUp = 0;
    gSndMusicFadeStep = gSndMusicFadeLevel = 0;
    for (i = 0; i < 18; i++) {
        id = gSndTracks[i].id & 0x7FFF;
        if (id >= 0x10 && id < 0x100) {
            sndStopTrack(i);
        }
    }
}

/* Stop track i, its voice faded out over 5 ms. */
void sndStopTrack(s32 i) {
    gSndCurTrack = &gSndTracks[i];
    gSndCurTrack->id = 0;
    gSndCurTrack->prio = 0;
    if (i < SND_TRACKS && gSndCurTrack->voiceOn != 0) {
        gSndCurTrack->voiceStopTicks = 2;
        gSndCurTrack->voiceVol = 0;
        gSndCurTrack->voiceVolTicks = 0;
        alSynSetVol(&alGlobals->drvr, &gSndVoices[i], 0, 5000);
    }
}

void sndFadeOutAll(void) {
    if (gSndAllFadeMusicLevel != 0xB0) {
        gSndAllFadeMusicStep = -4;
    }
    if (gSndAllFadeSeLevel != 0x10) {
        gSndAllFadeSeStep = -4;
    }
}

void sndFadeInAll(void) {
    if (gSndAllFadeMusicLevel != 0) {
        gSndAllFadeMusicStep = 4;
    }
    if (gSndAllFadeSeLevel != 0) {
        gSndAllFadeSeStep = 4;
    }
}

/* Unmute: re-pan every track that is playing. */
void sndSetStereo(void) {
    s32 i;

    if (gSndMono != 0) {
        gSndMono = 0;
        for (i = 0; i < SND_TRACKS; i++) {
            gSndCurTrack = &gSndTracks[i];
            if (gSndCurTrack->id != 0) {
                sndSetPan(i);
            }
        }
    }
}

/* Mute: every track playing is panned to the centre. */
void sndSetMono(void) {
    s32 i;

    if (gSndMono == 0) {
        gSndMono = 1;
        for (i = 0; i < SND_TRACKS; i++) {
            gSndCurTrack = &gSndTracks[i];
            if (gSndCurTrack->id != 0) {
                sndSetPan(i);
            }
        }
    }
}

/* Set track i's pan (the current track's): centred while muted, else from its fields, 0-127. */
void sndSetPan(u8 i) {
    s16 pan;

    if (i < SND_TRACKS) {
        gSndCurTrack->flags |= 1;
        if (gSndMono != 0) {
            pan = 64;
        } else {
            pan = (gSndCurTrack->pan >> 8) & 0x7F;
            pan += gSndCurTrack->param0[2];
            if (gSndCurTrack->slide[2] != 0) {
                pan += gSndCurTrack->slide[2] / 256;
            }
            if (pan < 0) {
                pan = 0;
            } else if (pan >= 0x80) {
                pan = 0x7F;
            }
        }
        alSynSetPan(&alGlobals->drvr, &gSndVoices[i], pan);
    }
}

/* Start a level ramp from the table: the first ramp, or the second for entry 10. */
void sndStartRamp(void) {
    if (gSndCurRequest != 10) {
        if (gSndMusicRamp.level == 0) {
            gSndMusicRamp.level = 0xFFFF;
        }
        gSndMusicRamp.rate = gSndRampRatesByCmd[gSndCurRequest];
        return;
    }
    if (gSndAllRamp.level == 0) {
        gSndAllRamp.level = 0xFFFF;
    }
    gSndAllRamp.rate = gSndRampRatesByCmd[gSndCurRequest];
}

void sndSpeedUpStart(void) {
    if (gSndSpeedUp < 0x100) {
        gSndSpeedUpTicks = 16;
    }
}

void sndSpeedUpStop(void) {
    gSndSpeedUpTicks = gSndSpeedUp = 0;
}

void sndMusicFadeOut(void) {
    if (gSndMusicFadeLevel == 0) {
        gSndMusicFadeStep = -4;
    }
}

void sndMusicFadeIn(void) {
    if (gSndMusicFadeLevel != 0) {
        gSndMusicFadeStep = 4;
    }
}

/* Nothing calls this one. */
void sndCmdNop(void) {
}

/* Give the current sound a slot of its own (unless it has one), with a level ramp. The second
 * loop tests its count at the bottom with a break, which IDO does not unroll (a `for` with the
 * test in its header is unrolled completely). */
void sndAddSlot(void) {
    s32 i;

    gSndCurRequest &= 0x7FFF;
    for (i = 0; i < 4; i++) {
        if (gSndCurRequest == gSndSlotSounds[i]) {
            return;
        }
    }
    for (i = 0; TRUE; ) {
        if (gSndSlotSounds[i] == 0) {
            gSndSlotSounds[i] = gSndCurRequest;
            gSndSlotRamps[i].level = 0xFFFF;
            gSndSlotRamps[i].rate = 0x4CC;
            return;
        }
        i++;
        if (i >= 4) {
            break;
        }
    }
}

/* Free slot n, stopping its tracks (the loop runs over 18 tracks). The chained assignment makes
 * the original's &gSndSlotRamps[n] with the store at +2. */
void sndFreeSlot(u16 n) {
    s32 i;
    SndRamp *fade;

    for (i = 0; i < 18; i++) {
        if (gSndSlotSounds[n] == (gSndTracks[i].id & 0x7FFF)) {
            sndStopTrack(i);
        }
    }
    fade = gSndSlotRamps;
    gSndSlotSounds[n] = fade[n].level = 0;
}

/* This file's .data (subsegment 0x25140), in ROM order. */
extern SndSe gSndSe1xxTable[];
extern SndSe gSndSe2xxTable[];
extern SndSe gSndSe300Table[];
extern SndSe gSndSe4Table[];
u32 gSndBankRomStart[SND_BANKS] = {
    0x0003F910, 0x00048D30, 0x00051320, 0x0005A650,
};
u32 gSndBankRomEnd[SND_BANKS] = {
    0x00048D30, 0x00051320, 0x0005A650, 0x00060890,
};
u8 *gSndWaveTableRom[SND_BANKS] = {
    (u8 *) 0x00060890,
    (u8 *) 0x00285020,
    (u8 *) 0x0035AC00,
    (u8 *) 0x00419BF0,
};
/* Per 0x100 ids from 0x100 (gSndSeTablesByPage, the name the code uses, is this table - 1). */
SndSe *gSndSeTables[4] = {
    gSndSe1xxTable, gSndSe2xxTable,
    gSndSe300Table, gSndSe4Table,
};
u8 gSndLoadedGroup = 0;
/* AL_FX_CUSTOM: 1 section, its length, then input, output, fbcoef, ffcoef, gain, chorus
 * rate and depth, lowpass. sndcmd.c reads [3] and [4] on (gSndFxDefaultOutput, gSndFxDefaultFbcoef). */
s32 gSndReverbParams[10] = {
    1, 12000, 0, 10200, 12800, 0, 32767, 0, 0, 0,
};
u32 gSndMusicRomStart[179] = {
    0x00531370, 0x005316F0, 0x00531790, 0x005318E0, 0x00531A20, 0x00531B60, 0x00531CA0, 0x00532120,
    0x00532910, 0x005337B0, 0x00534270, 0x00535250, 0x00535640, 0x00535AD0, 0x005362A0, 0x00536450,
    0x00536B00, 0x00537520, 0x005377A0, 0x00538AE0, 0x005394A0, 0x00539F70, 0x0053A0C0, 0x0053A870,
    0x0053B140, 0x0053B2B0, 0x0053B660, 0x0053BC30, 0x0053C220, 0x0053CA10, 0x0053D3E0, 0x0053D780,
    0x0053DE60, 0x0053E060, 0x0053E300, 0x0053F420, 0x0053FA80, 0x0053FC40, 0x005401D0, 0x00540D50,
    0x00541150, 0x005412F0, 0x00541870, 0x00542700, 0x00542E80, 0x00542FC0, 0x00543580, 0x00544260,
    0x005447E0, 0x00544A80, 0x00544F90, 0x005454E0, 0x00545A30, 0x005470A0, 0x00547450, 0x005475F0,
    0x00547B40, 0x005483A0, 0x00548C10, 0x00549A30, 0x0054A840, 0x0054B3E0, 0x0054B8C0, 0x0054BB00,
    0x0054BF70, 0x0054CC20, 0x0054D220, 0x0054D490, 0x0054D900, 0x0054E670, 0x0054EC70, 0x0054EE80,
    0x0054F330, 0x0054FC00, 0x005504D0, 0x005510A0, 0x00551830, 0x00551A30, 0x00551FA0, 0x00552660,
    0x00552960, 0x00552B00, 0x00553080, 0x00553C70, 0x005540A0, 0x005542F0, 0x00554890, 0x00555440,
    0x00555760, 0x005558F0, 0x00555FD0, 0x00556A80, 0x00556F70, 0x00557210, 0x00557930, 0x005588D0,
    0x00558D80, 0x00558FA0, 0x00559680, 0x00559EB0, 0x0055A790, 0x0055AEE0, 0x0055BF60, 0x0055C900,
    0x0055CE30, 0x0055D320, 0x0055D550, 0x0055D890, 0x0055EC50, 0x0055FA60, 0x00560A30, 0x005616E0,
    0x00562950, 0x00562CE0, 0x00563630, 0x00563A50, 0x005640F0, 0x005642D0, 0x005645F0, 0x00564EF0,
    0x005653D0, 0x005659D0, 0x00565CA0, 0x00565FF0, 0x00566930, 0x00567350, 0x00567490, 0x00567C30,
    0x005683A0, 0x00568F50, 0x0056A1F0, 0x0056A770, 0x0056B260, 0x0056B740, 0x0056BCE0, 0x0056BF50,
    0x0056C1C0, 0x0056CD50, 0x0056D0F0, 0x0056D550, 0x0056D9A0, 0x0056DD60, 0x0056E140, 0x0056E550,
    0x0056E910, 0x0056ECD0, 0x0056EF90, 0x00570160, 0x005711B0, 0x00571380, 0x005724D0, 0x00572580,
    0x00573420, 0x00574070, 0x00574B10, 0x00575560, 0x005761E0, 0x00576DA0, 0x005772C0, 0x00578BD0,
    0x0057A220, 0x0057BFA0, 0x0057C370, 0x0057E350, 0x0057E790, 0x0057EB30, 0x0057EE00, 0x0057EF70,
    0x0057F150, 0x0057F2F0, 0x0057F340, 0x0057F9E0, 0x005805D0, 0x00580B60, 0x005810C0, 0x00581270,
    0x00581780, 0x00581E00, 0x00582090,
};
u32 gSndMusicRomEnd[179] = {
    0x005316F0, 0x00531790, 0x005318E0, 0x00531A20, 0x00531B60, 0x00531CA0, 0x00532120, 0x00532910,
    0x005337B0, 0x00534270, 0x00535250, 0x00535640, 0x00535AD0, 0x005362A0, 0x00536450, 0x00536B00,
    0x00537520, 0x005377A0, 0x00538AE0, 0x005394A0, 0x00539F70, 0x0053A0C0, 0x0053A870, 0x0053B140,
    0x0053B2B0, 0x0053B660, 0x0053BC30, 0x0053C220, 0x0053CA10, 0x0053D3E0, 0x0053D780, 0x0053DE60,
    0x0053E060, 0x0053E300, 0x0053F420, 0x0053FA80, 0x0053FC40, 0x005401D0, 0x00540D50, 0x00541150,
    0x005412F0, 0x00541870, 0x00542700, 0x00542E80, 0x00542FC0, 0x00543580, 0x00544260, 0x005447E0,
    0x00544A80, 0x00544F90, 0x005454E0, 0x00545A30, 0x005470A0, 0x00547450, 0x005475F0, 0x00547B40,
    0x005483A0, 0x00548C10, 0x00549A30, 0x0054A840, 0x0054B3E0, 0x0054B8C0, 0x0054BB00, 0x0054BF70,
    0x0054CC20, 0x0054D220, 0x0054D490, 0x0054D900, 0x0054E670, 0x0054EC70, 0x0054EE80, 0x0054F330,
    0x0054FC00, 0x005504D0, 0x005510A0, 0x00551830, 0x00551A30, 0x00551FA0, 0x00552660, 0x00552960,
    0x00552B00, 0x00553080, 0x00553C70, 0x005540A0, 0x005542F0, 0x00554890, 0x00555440, 0x00555760,
    0x005558F0, 0x00555FD0, 0x00556A80, 0x00556F70, 0x00557210, 0x00557930, 0x005588D0, 0x00558D80,
    0x00558FA0, 0x00559680, 0x00559EB0, 0x0055A790, 0x0055AEE0, 0x0055BF60, 0x0055C900, 0x0055CE30,
    0x0055D320, 0x0055D550, 0x0055D890, 0x0055EC50, 0x0055FA60, 0x00560A30, 0x005616E0, 0x00562950,
    0x00562CE0, 0x00563630, 0x00563A50, 0x005640F0, 0x005642D0, 0x005645F0, 0x00564EF0, 0x005653D0,
    0x005659D0, 0x00565CA0, 0x00565FF0, 0x00566930, 0x00567350, 0x00567490, 0x00567C30, 0x005683A0,
    0x00568F50, 0x0056A1F0, 0x0056A770, 0x0056B260, 0x0056B740, 0x0056BCE0, 0x0056BF50, 0x0056C1C0,
    0x0056CD50, 0x0056D0F0, 0x0056D550, 0x0056D9A0, 0x0056DD60, 0x0056E140, 0x0056E550, 0x0056E910,
    0x0056ECD0, 0x0056EF90, 0x00570160, 0x005711B0, 0x00571380, 0x005724D0, 0x00572580, 0x00573420,
    0x00574070, 0x00574B10, 0x00575560, 0x005761E0, 0x00576DA0, 0x005772C0, 0x00578BD0, 0x0057A220,
    0x0057BFA0, 0x0057C370, 0x0057E350, 0x0057E790, 0x0057EB30, 0x0057EE00, 0x0057EF70, 0x0057F150,
    0x0057F2F0, 0x0057F340, 0x0057F9E0, 0x005805D0, 0x00580B60, 0x005810C0, 0x00581270, 0x00581780,
    0x00581E00, 0x00582090, 0x00582530,
};
SndRange gSndParamRanges[3] = {
    { -3072, 3072 },
    { -256, 256 },
    { -127, 127 },
};
void (*gSndCmdTable[15])(void) = {
    sndStopAll, sndStopMusic, sndFadeOutAll, sndFadeInAll,
    sndSetStereo, sndSetMono, sndStartRamp, sndStartRamp,
    sndStartRamp, sndStartRamp, sndSpeedUpStart, sndSpeedUpStop,
    sndMusicFadeOut, sndMusicFadeIn, sndCmdNop,
};
s32 gSndCheckFailCount = 0;
/* Ramp rates for commands 7-10 (gSndRampRatesByCmd, the name the code uses, is this table - 7). */
u16 gSndRampRates[4] = {
    0x600, 0x216, 0x99, 0x333,
};
