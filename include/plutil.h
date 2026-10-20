#ifndef PLUTIL_H
#define PLUTIL_H

#include "player.h"
#include "game.h"

s32 plActChangeAfter(Player *p, s32 next, s32 n);
s32 plActSwitch(Player *p, s32 next);
s32 plAlwaysTrue(Player *p);
void plActFromQueue(Player *p);
s32 plActSetDefault(Player *p, s32 action);
s32 plActEnqueue(Player *p, s32 action);
s32 plActEnqueueFront(Player *p, s32 action);
s32 plActDequeue(Player *p);
void plActSend(Player *p, s32 msg, s32 a, s32 b);
s32 plMsgReplyTake(Player *p);
f32 plSpeedLen(Player *p);
f32 plPushLen(Player *p);
s32 plIsZero(s32 a);
Player *plOtherPlayer(Player *p);
Player *plPartnerGet(Player *p);
void plBlinkSet(Player *p, s32 n);
void plFramePosCopy(AnimeFrame *a, AnimeFrame *b);
void plModelPosCopy(Player *a, Player *b);
void plModelOffsetSet(Player *p, f32 x, f32 y, f32 z);
void plModelOffsetZStartSide(Player *p);
f32 plJumpSpeed(f32 h);
void plFrameScaleSet(AnimeFrame *f, f32 s);
void plFrameScaleMul(AnimeFrame *f, f32 s);
void plFramePosAdd(AnimeFrame *f, Vec3f *v);
void plVecScaleSelf(Vec3f *v, f32 s);
void plChargeCount(Player *p);
void plChargeRelease(Player *p);
s32 plMoneyAdd(Player *p, s32 n);
Vec3f *plVecTurnByCourse(Player *p, f32 a, Vec3f *v, Vec3f *out);
void *plCourseRayCast(s32 zone, f32 *place, Vec3f *d);
f32 plVecDist(Vec3f *a, Vec3f *b);
f32 plVecLen(Vec3f *v);
f32 plLen2D(f32 a, f32 b);
void plVecClamp(Vec3f *v, f32 max);
f32 plVecDot(Vec3f *a, Vec3f *b);
Vec3f *plVecNormalize(Vec3f *out, Vec3f *v);
Vec3f *plVecAdd(Vec3f *out, Vec3f *a, Vec3f *b);
Vec3f *plVecSub(Vec3f *out, Vec3f *a, Vec3f *b);
Vec3f *plVecScale(Vec3f *out, Vec3f *v, f32 s);
s32 plDamage(Player *p, s32 hit, s32 wp);
void plRevive(Player *p);
void plFormSizeGet(Player *p, s32 form, Vec3f *out);
void plFormDmgBoxSet(Player *p, s32 form);

#endif
