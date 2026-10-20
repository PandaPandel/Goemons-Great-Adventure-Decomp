#ifndef SNDLOC_H
#define SNDLOC_H

#include "object.h"

/* A sound event of the score data: its sound and whether it fired. */
typedef struct ScoreEvent {
    /* 0x0 */ u16 id;
    /* 0x4 */ s32 hit;
} ScoreEvent;

typedef struct PosFrame {
    /* 0x00 */ Frame f;
    /* 0x14 */ f32 pos[3];
} PosFrame;

void soundPlayAfterOne(u16 id);
s16 soundPanGet(CamView *view, f32 x, f32 z, f32 fov);
s32 soundInFovCheck(CamView *view, f32 x, f32 y, f32 z);
s32 actpan_proc(Obj *obj, s16 msg, void *arg);
void soundActpanFlagSet(Obj *obj, s32 on);
Obj *soundActpanCreateCam(Obj *parent, u16 id);
Obj *soundActpanCreate(Obj *parent, u16 id, CamView *view, f32 near, f32 far, f32 fov);
s32 suion_proc(Obj *obj, s16 msg, void *arg);
void soundSuionStopSet();
Obj *soundSuionCreateCam(Obj *parent, u16 id, f32 x, f32 y, f32 z);
Obj *soundSuionCreate(Obj *parent, u16 id, CamView *view, f32 x, f32 y, f32 z, f32 near, f32 far);
void soundLocationCam(u16 id, f32 x, f32 y, f32 z);
void SoundLocation2(u16 id, CamView *view, f32 x, f32 y, f32 z, f32 near, f32 far, f32 fov);
s32 soundIncamCam(u16 id, f32 x, f32 y, f32 z);
s32 SoundIncam2(u16 id, CamView *view, f32 x, f32 y, f32 z, f32 near, f32 far);
s32 soundScorePlay(u32 addr, u16 file, f32 frame, f32 step, s32 loop, u16 id0, u16 id1, u16 id2, u16 id3, PosFrame *pos);
s32 soundScoreCheck(s32 addr, u16 file, f32 frame, f32 step, s32 loop, AnimeFrame *unused);
void soundScoreRead(u32 addr, u16 file, ScoreEvent *ev, f32 frame, f32 step, s32 loop);
s32 soundScoreKeyCheck(u16 track, f32 frame, f32 step, s32 loop, f32 frames);
s32 soundNoPlPoweredDown(void);

#endif
