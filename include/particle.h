#ifndef PARTICLE_H
#define PARTICLE_H

#include "object.h"

s32 particle(Obj *obj, s16 msg, void *arg);
void ptclSpawnSparks(f32 x, f32 y, f32 z);
void ptclSpawnSparksFall(f32 x, f32 y, f32 z);
void ptclSpawnSparksWhite(f32 x, f32 y, f32 z);
void ptclSpawnSparksColor(f32 x, f32 y, f32 z, u8 r, u8 g, u8 b, f32 sx, f32 sy);
s32 fire_ko(Obj *obj, s16 msg, void *arg);
void ptclSpawnFire(f32 x, f32 y, f32 z, u8 time);
s32 ame(Obj *obj, s16 msg, void *arg);
void ptclDrawRainStreak(u8 *buf, u8 x, u8 y, u8 len, f32 dx, f32 dy, u8 bright);
void ptclSpawnRain(f32 x, f32 y, f32 z, f32 speed, f32 fall, f32 c, u8 len, u8 bright, u8 count, u8 time);
s32 bhanabi(Obj *obj, s16 msg, void *arg);
void ptclSpawnHanabi(f32 x, f32 y, f32 z, f32 size, s32 r, s32 g, s32 b);
void ptclSpawnHanabiSmall(f32 x, f32 y, f32 z, f32 size, s32 r, s32 g, s32 b);
void ptclSpawnHanabiRed(f32 x, f32 y, f32 z, f32 size);

#endif
