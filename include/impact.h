#ifndef IMPACT_H
#define IMPACT_H

#include "common.h"
#include "object.h"
#include "player.h"
#include "game.h"
#include "boss.h"
#include "camera.h"

s32 Valcan(Obj *obj, s16 msg, void *arg);
s32 Jab(Obj *obj, s16 msg, void *arg);
s32 ComboJab(Obj *obj, s16 msg, void *arg);
s32 str_proc(Obj *obj, s16 msg, void *arg);
s32 CStraight(Obj *obj, s16 msg, void *arg);
s32 Ovalcan(Obj *obj, s16 msg, void *arg);
s32 kick_proc(Obj *obj, s16 msg, void *arg);
s32 kick_proc2(Obj *obj, s16 msg, void *arg);
s32 impGuardArm(Obj *obj, s16 msg, void *arg);
s32 Guard(Obj *obj, s16 msg, void *arg);
s32 Pgage(Obj *obj, s16 msg, void *arg);
s32 money(Obj *obj, s16 msg, void *arg);
s32 fuel(Obj *obj, s16 msg, void *arg);
s32 bosslife(Obj *obj, s16 msg, void *arg);
void impStormCreate(s32 kind);
s32 Storm(Obj *obj, s16 msg, void *arg);
s32 Astorm(Obj *obj, s16 msg, void *arg);
s32 Bbg(Obj *obj, s16 msg, void *arg);
s32 shoulder(Obj *obj, s16 msg, void *arg);
s32 Baton(Obj *obj, s16 msg, void *arg);
s32 MEGA(Obj *obj, s16 msg, void *arg);
s32 vc(Obj *obj, s16 msg, void *arg);
s32 BbgBoss5(Obj *obj, s16 msg, void *arg);
s32 Bappcam3(Obj *obj, s16 msg, void *arg);
s32 Bword(Obj *obj, s16 msg, void *arg);
void clearWordCreate(f32 x, f32 y, f32 z, s32 n);
s32 BwordEffect(Obj *obj, s16 msg, void *arg);
void clearWordEffectCreate(f32 a, f32 b, f32 c, f32 d, f32 e, f32 f, s32 n);
s32 Bclear(Obj *obj, s16 msg, void *arg);
void clearPackCreate(f32 x, f32 y, f32 z, s32 n);

#endif
