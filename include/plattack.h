#ifndef PLATTACK_H
#define PLATTACK_H

#include "player.h"

void plAttackInit(Player *p, s32 msg, s32 a, s32 b);
void plAttackDefault(Player *p, s32 msg, s32 a, s32 b);
s32 plAttackFireShot(Player *p, f32 *frames);
void plAttackShot(Player *p, s32 msg, s32 a, s32 b);
void plAttackShotAir(Player *p, s32 msg, s32 a, s32 b);
void plAttackShotStand(Player *p, s32 msg, s32 a, s32 b);
void plAttackShotFwd(Player *p, s32 msg, s32 a, s32 b);
void plAttackShotFwdAir(Player *p, s32 msg, s32 a, s32 b);
s32 plAttackForm(Player *p);
s32 plAttackCanEnd(Player *p);
s32 plAttackChargedForm(Player *p);
s32 plAttackAtChargedFrame(Player *p);
void plAttackFireCharged(Player *p);
void plAttackCharged(Player *p, s32 msg, s32 a, s32 b);
void plAttackChargedAir(Player *p, s32 msg, s32 a, s32 b);
void plAttackChargedStand(Player *p, s32 msg, s32 a, s32 b);
void plAttackChargedFwd(Player *p, s32 msg, s32 a, s32 b);
void plAttackChargedFwdAir(Player *p, s32 msg, s32 a, s32 b);

#endif
