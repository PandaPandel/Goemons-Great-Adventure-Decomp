#ifndef PLBOX_H
#define PLBOX_H

#include "player.h"

void plBoxSetKind(Player *p, s32 n);
void plBoxDelete(Player *p);
void plBoxAttackBase(Player *p, s32 msg, s32 a, s32 b);

#endif
