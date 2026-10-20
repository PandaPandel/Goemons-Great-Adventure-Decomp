#ifndef PLCHARA_H
#define PLCHARA_H

#include "player.h"

void plChara0Jump(Player *p, s32 msg, s32 a, s32 b);
void plChara1Bounce(Player *p, s32 msg, s32 a, s32 b);
void plSetPhase(Player *p, s32 phase);
void plResetScale(Player *p);
s32 plSquashStretch(Player *p, s32 step, f32 dir, s32 mark);
void plCharaJump(Player *p, s32 msg, s32 a, s32 b);
void plChara0JumpFall(Player *p, s32 msg, s32 a, s32 b);
void plChara1BounceFall(Player *p, s32 msg, s32 a, s32 b);
s32 plCanCharaJump(Player *p);

#endif
