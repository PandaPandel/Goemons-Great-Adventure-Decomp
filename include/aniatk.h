#ifndef ANIATK_H
#define ANIATK_H

#include "acoll.h"

s32 aniAtkInit(void);
void aniAtkSyncLinkTime(AniAtkFrame *f, s32 link);
ACollFrame *aniAtkCreate(Frame *parent, Frame *model, u8 *anim, s32 file, s32 base);
s32 aniAtkDelete(ACollFrame *box);
ACollFrame *aniAtkGetBox(ACollFrame *box);
s32 aniAtkJointCount(AniAtkFrame *f);
s32 aniAtkFrameCount(AniAtkFrame *f);
s32 aniAtkBoxFrameCount(ACollFrame *box);
u8 *aniAtkTrackStream(AniAtkFrame *f, u32 base);
u16 *aniAtkJoints(AniAtkFrame *f, u32 base);
void aniAtkLinkTransform(AniAtkFrame *f, u8 link, f32 time, f32 *x, f32 *y, f32 *z, f32 *rx, f32 *ry, f32 *rz, f32 *sx, f32 *sy, f32 *sz);
void aniAtkReadJoint(u16 *joint);
void aniAtkPlaceJoint(f32 x, f32 y, f32 z, f32 sx, f32 sy, f32 sz, f32 rx, f32 ry, f32 rz);
u16 aniAtkTrackValueU(u16 key);
s16 aniAtkTrackValueS(u16 key);
u16 aniAtkWalkTrack(u16 pos, s32 i1, u16 *out1, s32 i2);

#endif
