#ifndef PLTALK_H
#define PLTALK_H

#include "player.h"

void plTalkBaseProc(Player *p, s32 msg, s32 a, s32 b);
void plTalkProc(Player *p, s32 msg, s32 a, s32 b);
void plTalkSetOthersHalt(Player *p, s32 halt);
void plTalkHaltOthers(Player *p);
void plTalkReleaseOthers(Player *p);

#endif
