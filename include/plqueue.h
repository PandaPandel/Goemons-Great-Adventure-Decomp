#ifndef PLQUEUE_H
#define PLQUEUE_H

#include "player.h"

void plQueueClear(Player *p);
s32 plQueuePushBack(Player *p, s32 action);
s32 plQueuePushFront(Player *p, s32 action);
s32 plQueuePop(Player *p);

#endif
