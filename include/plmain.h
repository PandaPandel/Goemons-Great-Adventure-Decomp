#ifndef PLMAIN_H
#define PLMAIN_H

#include "player.h"
#include "game.h"

Obj *plCreate(Obj *parent, PlInit *init);
PlInit *plMakeInit(PlInit *init, s32 index, s32 chara);
void plDestroy(s32 i);
s32 plMain(Obj *task, s16 msg, void *arg);
s32 plMain_init(Obj *task, PlInit *init);
void plStep(Player *p);
void plInitJump(Player *p);
void plAskPos(Obj *task, f32 *place, f32 *height, f32 *side, s32 *zone);
void plGetModelPos(Obj *task, f32 *x, f32 *y, f32 *z);
f32 plGetModelYaw(Obj *task);
s32 plIsInPlay(s32 i);
s32 plIsPoweredDown(s32 i);
s32 plIsSwimming(s32 i);
s32 plIsDiving(s32 i);
ACollFrame *plGetDamageBox(Obj *task);
void plExtDamage(Obj *task, s32 a, s32 kind);
s32 plGetIndex(Obj *task);
s32 plGetChara(Obj *task);
void plGetLastPush(Obj *task, Vec3f *out);
void plGetSpeed(Obj *task, Vec3f *out);
void plCountPlayers(void);
s32 plGetAction(Obj *task);
void plHalt(Obj *task);
void plContinue(Obj *task);
void plUpdateBlink(Player *p);
void plCountInvincible(Player *p);
s32 plEnterDemo(Obj *task);
void plDemoWalkTo(Obj *task, Vec3f goal, f32 speed);
void plDemoWalkToPlace(Obj *task, f32 place, f32 speed);
void plSetPos(Obj *task, s32 zone, Vec3f *pos);
s32 plIsDemoWalking(Obj *task);
s32 plSetAction(Obj *task, s32 next);
void plSlowDown(s32 i);
s32 plEndPowerDown(Obj *task);
void *plGetBuffAddr(Player *p, s32 kind);
void plFreeBuff(void *buf);

#endif
