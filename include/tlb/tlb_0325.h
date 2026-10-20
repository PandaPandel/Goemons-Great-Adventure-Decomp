#ifndef TLB_TLB_0325_H
#define TLB_TLB_0325_H

#include "common.h"
#include "object.h"

s32 bfriProc(Obj *obj, s16 msg, void *arg);
void bfriHit(Obj *obj);
s32 bfriNormal(Obj *obj);
s32 bfriNormalSwayLong(Obj *obj);
s32 bfriNormalSwayMid(Obj *obj);
s32 bfriNormalSwayShort(Obj *obj);
s32 bfriNeutral(Obj *obj);
s32 bfriElectricShot(Obj *obj);
s32 bfriBarrierAttack(Obj *obj);
s32 bfriBachiAtari(Obj *obj);
s32 bfriSuperHeadbutt(Obj *obj);
s32 bfriSenpuuCrash(Obj *obj);
s32 bfriBitBodySlam(Obj *obj);
s32 bfriBachiRanbu(Obj *obj);
s32 bfriNormalHeadbutt(Obj *obj);
s32 bfriNormalHeadbuttDeco(Obj *obj);
s32 bfriBitSuperHeadbutt(Obj *obj);
s32 bfriMove1(Obj *obj);
s32 bfriMoveForward(Obj *obj);
s32 bfriMoveToBase(Obj *obj);
s32 bfriMoveToBase2(Obj *obj);
s32 bfriMoveForward2(Obj *obj);
s32 bfriFloatMove(Obj *obj);
s32 bfriStraightHit(Obj *obj);
s32 bfriJabHitNegX(Obj *obj);
s32 bfriJabHitPosX(Obj *obj);
s32 bfriHundredPunchHit(Obj *obj);
s32 bfriDizzyKnockdown(Obj *obj);
s32 bfriKickHit(Obj *obj);
s32 bfriCannonHit(Obj *obj);
s32 bfriBrotherAttackHit(Obj *obj);
s32 bfriKnockedBack(Obj *obj);
s32 bfriImpactChange(Obj *obj);
s32 bfriStep(Obj *o);
s32 bfriEntranceDemo(Obj *obj);
s32 bfriDeathDemo(Obj *obj);
s32 bfriWinDemo(Obj *obj);
void bfriMoveLimit(Obj *o);
s32 bfriBmoveX(Obj *o, f32 x, f32 y, f32 z, f32 accel);
s32 bfriBmoveXFar(Obj *o, f32 x, f32 y, f32 z, f32 accel);
s32 bfriTurnTo(Obj *o, f32 x, f32 z, f32 step);
void bfriBank(Obj *o, f32 step);
void bfriRumble(u32 pattern, s32 time);
void bfriDdmybombCreate(Obj *obj, f32 x, f32 y, f32 z, f32 v, s32 a, s32 b, s32 c);
void bfriElecBombCreate(Obj *obj, f32 x, f32 y, f32 z);
void bfriPuroBombCreate(void);
void bfriSunaEfMake(f32 x, f32 y, f32 z, f32 dx, f32 dz, f32 scale, s32 time);
void bfriKoke2EfMake(f32 x, f32 y, f32 z);
void bfriCloudsMake(f32 x, f32 y, f32 z);
Obj *bfriTatTestMake(void);
void bfriPuroperaMake(void);
void bfriBariaMake(void);
void bfriFzanzouMake(Obj *obj, f32 x, f32 y, f32 z, f32 a, f32 b, f32 c, f32 d, s32 n);
Obj *bfriSkyMake(void);
void bfriThunderMake(s32 value);
void bfriPlasmaMake(f32 x, f32 y, f32 z, s32 type);
Obj *bfriBitPaCreate(Obj *obj);
s16 bfriRand(s16 n);
s16 bfriRandSigned(s16 n);

#endif
