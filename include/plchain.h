#ifndef PLCHAIN_H
#define PLCHAIN_H

#include "player.h"

Obj *plChainThrow(Player *p);
void plChainDefault(Player *p, s32 msg, s32 a, s32 b);
void plChainStart5C(Player *p, s32 msg, s32 a, s32 b);
void plChainWait5E(Player *p, s32 msg, s32 a, s32 b);
void plChainEnd5D(Player *p, s32 msg, s32 a, s32 b);
void plChainStart5F(Player *p, s32 msg, s32 a, s32 b);
void plChainWait61(Player *p, s32 msg, s32 a, s32 b);
void plChainEnd60(Player *p, s32 msg, s32 a, s32 b);
void plChainStart62(Player *p, s32 msg, s32 a, s32 b);
void plChainWait64(Player *p, s32 msg, s32 a, s32 b);
void plChainEnd63(Player *p, s32 msg, s32 a, s32 b);
s32 plChainProc(Obj *obj, s16 msg, void *arg);
void *plChainCastRay(Vec3f *from, Vec3f *dir, Vec3f *at, f32 *arg);
void plChainEmpty(void);

#endif
