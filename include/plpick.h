#ifndef PLPICK_H
#define PLPICK_H

#include "player.h"

s32 plPickAttack(Player *p);
s32 plPickGround(Player *p);
s32 plSetGroundAction(Player *p);
s32 plPickGroundWrap(Player *p);
s32 plPickAttackNoFwd(Player *p);
s32 plPickMoveOrAttack(Player *p);
s32 plPickGroundNoStand(Player *p);
s32 plPickWalk(Player *p);
s32 plPickTurnBack(Player *p);
s32 plWantsTurnBack(Player *p);
s32 plNoGroundAhead(Player *p);
s32 plPickJump(Player *p);
s32 plWantsJump(Player *p);
s32 plPickStandIfGrounded(Player *p);
s32 plIsOnSlope(Player *p);
s32 plPickSlope(Player *p);
s32 plWantsChargedMove(Player *p);
s32 plIsCharged(Player *p);
s32 plWantsOwnMove(Player *p);
s32 plPickOwnMove(Player *p);
s32 plCanDive(Player *p);
s32 plCanSurface(Player *p);
s32 plPickStandAtAnimEnd(Player *p);
s32 plPickLanding(Player *p);
s32 __plIdleDemoPa(Player *p);
Player *plFindRideTarget(Player *p);
void plTurn2(Player *p, f32 a);
s32 plTurnTo(Player *p, f32 a);
void plCopyAnimeFrame(AnimeFrame *dst, AnimeFrame *src);
void plPutFrameAt(s32 index, AnimeFrame *f);

#endif
