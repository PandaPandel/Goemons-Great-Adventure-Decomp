#ifndef PLSOUND_H
#define PLSOUND_H

#include "player.h"

void plSoundAtModel(AnimeFrame *model, u16 se, s32 index);
void plSoundAtPoint(f32 x, f32 y, f32 z, u16 se, s32 index);
void plSoundLocation(u16 se, CamView *view, f32 x, f32 y, f32 z, f32 near, f32 far, f32 fov, s32 index);
s16 plSoundPan(CamView *view, f32 x, f32 y, f32 z, f32 near, f32 far, f32 fov, s32 index);
u16 plFootstepSe(Player *p, u16 se);
void plAttackSound(Player *p);
void plAttackVoice(Player *p);
void plLandSound(Player *p);
void plLandGroundSound(Player *p);
void plRequestLandSound(Player *p);
void plActStartVoice(Player *p);
void plEatVoice(Player *p);
void plPushSound(Player *p);
void plRestartSdSet(Player *p);
void plJoinSdSet(Player *p);
void plHumStart(Player *p);
void plHumStop(Player *p);
void plAnimEventSounds(Player *p);

#endif
