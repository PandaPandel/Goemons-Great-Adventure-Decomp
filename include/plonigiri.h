#ifndef PLONIGIRI_H
#define PLONIGIRI_H

#include "player.h"

void plActNoneDefault(Player *p, s32 msg, s32 a, s32 b);
void plActDamage(Player *p, s32 msg, s32 a, s32 b);
void plActDamageEnd(Player *p, s32 msg, s32 a, s32 b);
void plActNull(Player *p, s32 msg, s32 a, s32 b);
void plActDamageWater(Player *p, s32 msg, s32 a, s32 b);
void plActDownDefault(Player *p, s32 msg, s32 a, s32 b);
void plActDown(Player *p, s32 msg, s32 a, s32 b);
void plActDownWater(Player *p, s32 msg, s32 a, s32 b);
void plActDownShrink(Player *p, s32 msg, s32 a, s32 b);
void plActDownFall(Player *p, s32 msg, s32 a, s32 b);
void plActDown2F(Player *p, s32 msg, s32 a, s32 b);
void plVoiceDamage(Player *p);
Obj *plOnigiriCreate(Player *p);
s32 onigiriProc(Obj *obj, s16 msg, void *arg);
void __plOnigiri(void);
void plActCharaSpecial(Player *p, s32 msg, s32 a, s32 b);
void plActToStand(Player *p, s32 msg, s32 a, s32 b);
void plItexPuff(f32 x, f32 y, f32 z, Vec3f *v, f32 size);
s32 plYaeAwa(Obj *obj, s16 msg, void *arg);
Obj *plYaeAwaCreate(Obj *task);
s32 plGoeKumo(Obj *obj, s16 msg, void *arg);
Obj *plGoeKumoCreate(Obj *task);

#endif
