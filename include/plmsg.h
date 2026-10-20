#ifndef PLMSG_H
#define PLMSG_H

#include "player.h"
#include "game.h"

void plHitOrItem(Player *p);
void plGroundKnockback(Player *p);
void plAddHudPips(Player *p, s32 n);

#endif
