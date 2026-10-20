#ifndef ANIM_H
#define ANIM_H

#include "object.h"

void animSeek(u32 addr, f32 frame, u32 *segs);
u16 animGetScale(u16 v);
s16 animGetPos(u16 v);
void animGetRot(ModelNode *node, u16 *rot);
void animDecodeKeys(u16 pos, s32 k0, s32 k1, u16 *out0, u16 *out1);

#endif
