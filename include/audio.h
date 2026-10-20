#ifndef AUDIO_H
#define AUDIO_H

#include "common.h"
#include "scheduler.h"
#include "snd.h"

typedef struct AmConfig {
    /* 0x0 */ u32 outputRate;
    /* 0x4 */ u32 framesPerField;
    /* 0x8 */ u32 maxACMDSize;
} AmConfig;

typedef union AudioMsg {
    struct {
        s16 type;
    } gen;
    struct {
        s16 type;
        struct AudioInfo *info;
    } done;
} AudioMsg;

typedef struct AudioInfo {
    /* 0x00 */ s16 *data;        /* the output buffer */
    /* 0x04 */ s16 frameSamples; /* samples made this frame */
    /* 0x08 */ OSScTask task;
    /* 0x60 */ AudioMsg msg;     /* the done message */
} AudioInfo; /* size 0x68 */

typedef struct AMDMABuffer {
    /* 0x00 */ ALLink node;
    /* 0x08 */ u32 startAddr;
    /* 0x0C */ u32 lastFrame;
    /* 0x10 */ char *ptr;
} AMDMABuffer; /* size 0x14 */

typedef struct AMDMAState {
    /* 0x0 */ u8 initialized;
    /* 0x4 */ AMDMABuffer *firstUsed;
    /* 0x8 */ AMDMABuffer *firstFree;
} AMDMAState;

void audioNop(void);
void audioUnusedStub1(void);
void amCreateAudioMgr(ALSynConfig *c, OSPri pri, AmConfig *amc);
void __amMain(void *arg);
s32 __amHandleFrameMsg(AudioInfo *info, AudioInfo *lastInfo);
void __amHandleDoneMsg(AudioInfo *info);
s32 __amDMA(s32 addr, s32 len, void *state);
ALDMAproc __amDmaNew(AMDMAState **state);
void __clearAudioDMA(void);
void sndInit(void);
void sndInitPlayer(ALPlayer *player);
ALMicroTime sndPlayerProc(void *node);
void sndInitVoices(void);
void sndPlay(u16 id);
void sndCommand(u16 id);
void sndPlayPan(u16 id, u8 pan);
void sndPlayVol(u16 id, u8 vol);
void sndRequest(u16 id, u8 pan, u8 vol);
void sndSetParam(u16 id, u8 kind, s16 value);
s32 sndParamValue(u8 i, u8 kind);
void sndApplyParam(s32 n);
void sndSetReverb(u8 which, u8 v);
void sndClearReverb(void);
s32 sndRampLevel(void);
s32 sndGetVolume(u16 id, s32 which);
u16 sndGetMusic(void);
void sndStartFade(s32 i, s32 in);
void sndCrossfade(s32 in);
void sndServeRequests(void);
void sndServeMusicChanges(void);
void sndServeCommands(void);
void sndStopSound(u32 id);
void sndStartSlide(s32 kind, u32 id);
void sndStartRequest(s32 mode);
u8 sndFindSideTrack(void);
void sndStartSe(void);
void audioUnusedStub2(void);
u8 sndLowestPrioTrack(u8 group);
void sndStartMusic(void);
s32 sndCheckBlock(u8 n, u8 *table);
void sndLoadMusic(void);
void sndPlayMusic(u16 id, u8 delay);
void sndLoadMusicParts(void);
void sndPlayGroupMusic(u8 mode, u16 id, u8 delay);
void sndStartSequence(s32 mode);
void sndStopAll(void);
void sndStopMusic(void);
void sndStopTrack(s32 i);
void sndFadeOutAll(void);
void sndFadeInAll(void);
void sndSetStereo(void);
void sndSetMono(void);
void sndSetPan(u8 i);
void sndStartRamp(void);
void sndSpeedUpStart(void);
void sndSpeedUpStop(void);
void sndMusicFadeOut(void);
void sndMusicFadeIn(void);
void sndCmdNop(void);
void sndAddSlot(void);
void sndFreeSlot(u16 n);

#endif
