#ifndef PLSWIM_H
#define PLSWIM_H

#include "player.h"
#include "game.h"

void plSwimBase(Player *p, s32 msg, s32 a, s32 b);
void plSwimPa(Player *p, s32 msg, s32 a, s32 b);
void plSwimIdlePa(Player *p, s32 msg, s32 a, s32 b);
void plSwimStartPa(Player *p, s32 msg, s32 a, s32 b);
void plSwimStopPa(Player *p, s32 msg, s32 a, s32 b);
void plSwimAttack(Player *p, s32 msg, s32 a, s32 b, s32 n);
void plSwimAttack0Pa(Player *p, s32 msg, s32 a, s32 b);
void plSwimAttack1Pa(Player *p, s32 msg, s32 a, s32 b);
void plSwimAttack2Pa(Player *p, s32 msg, s32 a, s32 b);
void plSwimUpAttack0Pa(Player *p, s32 msg, s32 a, s32 b);
void plSwimUpAttack1Pa(Player *p, s32 msg, s32 a, s32 b);
void plSwimUpAttack2Pa(Player *p, s32 msg, s32 a, s32 b);
void plSwimShoot(Player *p, f32 *frames, s32 kind);
s32 plSwimShotEnded(Player *p, s32 kind);
s32 plSwimShotNext(Player *p, s32 kind, f32 *frames);
void plSwimSubWeaponPa(Player *p, s32 msg, s32 a, s32 b);
void plSwimUpSubWeaponPa(Player *p, s32 msg, s32 a, s32 b);
s32 plSwimPick(Player *p);
s32 plSwimPushing(Player *p);

#endif
