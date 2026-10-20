#ifndef TLB_TLB_0324_H
#define TLB_TLB_0324_H

#include "common.h"
#include "object.h"

s32 btaiProc(Obj *obj, s16 msg, void *arg);
void btaiHit(Obj *obj);
s32 btaiNormal(Obj *obj);
s32 btaiNormalSwayLong(Obj *obj);
s32 btaiNormalSwayMid(Obj *obj);
s32 btaiNormalSwayShort(Obj *obj);
s32 btaiNeutral(Obj *obj);
s32 btaiMarbleShot(Obj *obj);
s32 btaiMarbleBarrage(Obj *obj);
s32 btaiBiribiriRod(Obj *obj);
s32 btaiRushFeint(Obj *obj);
s32 btaiWhirlpoolAttack(Obj *obj);
s32 btaiTorpedoAttack(Obj *obj);
s32 btaiHeadbutt(Obj *obj);
s32 btaiMoveBack(Obj *obj);
s32 btaiMoveToCockpit(Obj *obj);
s32 btaiMoveToBase(Obj *obj);
s32 btaiMoveToEdge(Obj *obj);
s32 btaiStraightHit(Obj *obj);
s32 btaiJabHitNegX(Obj *obj);
s32 btaiJabHitPosX(Obj *obj);
s32 btaiHundredPunchHit(Obj *obj);
s32 btaiKickHit(Obj *obj);
s32 btaiCannonKnockback(Obj *obj);
s32 btaiDizzyKnockdown(Obj *obj);
s32 btaiCannonHit(Obj *obj);
s32 btaiImpactChange(Obj *obj);
s32 btaiKnockedBack(Obj *obj);
s32 btaiBarrierAttack(Obj *obj);
s32 btaiExternalImpactHit(Obj *obj);
s32 btaiStep(Obj *o);
s32 btaiEntranceDemo(Obj *obj);
s32 btaiDeathDemo(Obj *obj);
s32 btaiWinDemo(Obj *obj);
void btaiMoveLimit(Obj *o);
s32 btaiBmoveX(Obj *o, f32 x, f32 y, f32 z, f32 accel);
s32 btaiBmoveXFast(Obj *o, f32 x, f32 y, f32 z, f32 accel);
s32 btaiTurnTo(Obj *o, f32 x, f32 z, f32 step);
void btaiBank(Obj *o, f32 k);
void btaiEaseVel(Obj *o);
void btaiRumble(u32 pattern, s32 time);
void btaiTMarbleCreate(Obj *o, f32 x, f32 y, f32 z);
void btaiDdmybombCreate(Obj *obj, f32 x, f32 y, f32 z, f32 v, s32 a, s32 b, s32 c);
void btaiIkaBombCreate(Obj *obj, f32 x, f32 y, f32 z, f32 angle);
void sunaEfMake(f32 x, f32 y, f32 z, f32 dx, f32 dz, f32 scale, s32 time);
Obj *taisambaCreateTask(void);
void btaiSunaEf2Make(f32 x, f32 y, f32 z, f32 dx, f32 dz, f32 scale, s32 time);
void btaiKokeEfMake(f32 x, f32 y, f32 z);
void btaiUzuSnaMake(void);
void btaiTbubbleMake(Obj *parent, f32 x, f32 y, f32 z, f32 vx, f32 vz, f32 scale);
void btaiTbubbleMakeRand(Obj *parent);
void btaiT3bubbleMake(f32 x, f32 y, f32 z);
void btaiTsbubbleMake(f32 x, f32 y, f32 z);
void btaiU3bubbleMake(f32 x, f32 y, f32 z);
Obj *btaiUzu1Make(void);
Obj *btaiUzuInMake(void);
void btaiUzu2Make(void);
void btaiHagataMake(Obj *parent, f32 x, f32 y, f32 z);
void btaiIkasumiMake(f32 x, f32 y, f32 z, f32 scale, s32 time);
void btaiTzanzouMake(Obj *parent, f32 x, f32 y, f32 z, f32 rx, f32 ry, f32 rz, f32 frame);
void btaiTzanzou2Make(Obj *parent, f32 x, f32 y, f32 z, f32 rx, f32 ry, f32 rz, f32 frame, s32 n);
void btaiKiranMake(f32 x, f32 y, f32 z);
void btaiBarriMake(void);
s32 btaiUtsuboTakeSlot(void);
void btaiUtsuboCreate(s32 side);
s16 btaiRand(s16 n);
s16 btaiRandSigned(s16 n);

#endif
