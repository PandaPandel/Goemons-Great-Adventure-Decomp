#ifndef PLBASE_H
#define PLBASE_H

#include "player.h"

void plBase(Player *p, s32 msg, s32 a, s32 b);
void plBaseStickSpeed(Player *p);
s32 plBaseStandNext(Player *p);
s32 plBaseSlowDown(Player *p);
s32 plBaseWalkAction(Player *p);

#endif
