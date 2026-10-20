#ifndef MODEL_H
#define MODEL_H

#include "object.h"

s32 mdlLoadPart(u16 file, u32 *table, s32 n, u8 *dest);
s32 mdlSwapDLs(u32 addr, u8 *base, u32 *from, u32 *to);
void mdlAnimNodePos(u32 addr, f32 frame, u8 *base, f32 *x, f32 *y, f32 *z);
void mdlAnimNodeRot(u32 addr, f32 frame, u8 *base, f32 *rx, f32 *ry, f32 *rz);
void mdlAnimNodeMtx(u32 addr, f32 frame, u8 **base, f32 out[4][4]);
s32 mdlListCount(u32 *list);
s32 mdlNodeSwapDLs(ModelNode *node, u32 *from, u32 *to);
ModelNode *mdlFirstDrawnNode(u32 addr, f32 unused, u8 *base);

#endif
