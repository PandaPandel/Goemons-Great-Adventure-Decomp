#ifndef SNDTRACK_H
#define SNDTRACK_H

#include "snd.h"

void trkDriverTick(void);
void trkUpdate(void);
void trkNextNote(void);
void trkMusicTick(void);
void trkCheckFadeVol(void);
void trkSeTick(void);
void trkSetPitch(void);
void trkSetVolume(void);
s32 trkScaleTicks(s32 v, u8 scale, s8 x);
void trkEnvAttack(void);
void trkEnvDecay(void);
void trkEnvSustain(void);
void trkEnvRelease(void);
void trkEnvReleaseNow(void);
void trkEnvTick(void);

#endif
