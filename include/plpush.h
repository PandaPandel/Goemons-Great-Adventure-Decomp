#ifndef PLPUSH_H
#define PLPUSH_H

#include "player.h"

void plPushDefault(Player *p, s32 msg, s32 a, s32 b);
void plPushSendPush(Player *p, f32 dir);
void plPushSendPull(Player *p, f32 dir);
void plActPush(Player *p, s32 msg, s32 a, s32 b);
void plActPushStand(Player *p, s32 msg, s32 a, s32 b);
void plActPushEnd(Player *p, s32 msg, s32 a, s32 b);
void plActPushToPull(Player *p, s32 msg, s32 a, s32 b);
void plActPullToPush(Player *p, s32 msg, s32 a, s32 b);
void plActPull(Player *p, s32 msg, s32 a, s32 b);
void plActPullEnd(Player *p, s32 msg, s32 a, s32 b);

#endif
