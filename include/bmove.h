#ifndef BMOVE_H
#define BMOVE_H

#include "common.h"
#include "object.h"
#include "player.h"
#include "acoll.h"
#include "game.h"
#include "boss.h"
#include "polygon.h"

s32 BmoveX(Obj *obj, f32 x, f32 y, f32 z, f32 speed);
s32 BmoveX2(Obj *obj, f32 x, f32 y, f32 z, f32 speed);
s32 bossCheckButtonSeq(s32 port, u16 *seq, s32 back, s32 mode);
s32 bossDampVel(Obj *obj, f32 k);
void bossSetPolDL(PolFrame *f, Gfx *dl, s32 n);
void bossSetAnim(AnimeFrame *f, u32 anim, s32 atEnd);
void bossSetAnimAt(AnimeFrame *f, void *anim, s32 n, f32 at);
s32 bossStepAnim(AnimeFrame *f, f32 speed, s32 mode);
s32 bossStepAnimAtk(AnimeFrame *f, ACollFrame *g, f32 speed, s32 mode);
void bossApplyVel(Obj *obj);
void bossApplyVel2(Obj *obj);
void bossApplyVelMpol(Obj *obj);
void bossTiltByVel(Obj *obj, f32 k);
void bossTiltByVel2(Obj *obj, f32 k);
void bossNop(Obj *obj, s32 a);
s32 bossTurnToward(AnimeFrame *f, f32 x, f32 z, f32 step);
s32 bossTurnToward2(AnimeFrame *f, f32 x, f32 z, f32 step);
void bossHeadingOffset(f32 angle, f32 side, f32 dist, f32 *x, f32 *z);
f32 bossDistSqXZ(f32 x1, f32 z1, f32 x2, f32 z2);
f32 bossDistSq(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2);
s32 bossGravity(Obj *obj, f32 g);
s32 bossGravity2(Obj *obj, f32 g);
void bossStopFall(Obj *obj);
void bossImpactHit(s32 who, s32 a, s32 fill);
void bossImpactDamage(s32 damage, s32 a, s32 fill);
void bossDamage(s32 damage, s32 a, s32 fill);
void bossAddMoney(s32 n);
void bossAddPower(s32 n);
void bossBobAccel(Obj *obj, f32 amp, f32 speed);
void bossMoveLimitXZ(Obj *obj, f32 xmin, f32 xmax, f32 zmin, f32 zmax);
void bossMoveLimitY(Obj *obj, f32 ymin, f32 ymax);
void bossAddFlinchCount(s32 n);
s32 bossFlinchCountOver(s32 n);
void bossStartAction(Obj *obj, s32 a, s32 action);
void bossClearAction(Obj *obj);
void bossSetFrameFlags(AnimeFrame *f, s32 flags);
void bossSetFrameFlags2(AnimeFrame *f, s32 flags);
void bossSetMpolFlags(MPolFrame *f, s32 flags);
Gfx *bossPrimEnvColorDL(s32 r, s32 g, s32 b, s32 er, s32 eg, s32 eb, s32 ea);
Gfx *bossEnvColorDL(s32 r, s32 g, s32 b, s32 a);
Gfx *bossXluEnvColorDL(s32 r, s32 g, s32 b, s32 a);
Gfx *bossXluPrimEnvColorDL(s32 r, s32 g, s32 b, s32 a);
Gfx *bossLitTexEnvColorDL(s32 r, s32 g, s32 b, s32 a);
Gfx *bossCombineEnvColorDL(s32 r, s32 g, s32 b, s32 a);
Gfx *bossFogEnvColorDL(s32 r, s32 g, s32 b, s32 a, s32 fr, s32 fg, s32 fb, s32 fa, s32 near);
void bossBobVel(Obj *obj, f32 amp, f32 speed);
void bossBobFrame(AnimeFrame *f, f32 amp, f32 speed);
s32 bossFlyToward(Obj *obj, f32 x, f32 y, f32 z, f32 accel);
void bossStartActionIfFlag(Obj *obj, s32 action);
void bossClearState(void);
void vramCheck(AnimeFrame *f, f32 x, f32 y, f32 z, f32 down, f32 above);
Gfx *bossFogTexEnvColorDL(s32 r, s32 g, s32 b, s32 a);
s32 vramCheckGround(AnimeFrame *f, f32 x, f32 y, f32 z, f32 down, f32 above);
s32 bossTurnToDir(AnimeFrame *f, f32 dx, f32 dz, f32 step);
void bossMoveLimitClampXZ(Obj *obj, f32 xmin, f32 xmax, f32 ymin, f32 ymax, f32 zmin, f32 zmax);
void bossStartActionKind();
s32 bossFlyTowardXZ(Obj *obj, f32 x, f32 z, f32 accel, f32 max);
s32 bossPatrolX(Obj *obj, f32 min, f32 max, f32 accel);
s32 bossPatrolY(Obj *obj, f32 min, f32 max, f32 accel);
s32 bossPatrolZ(Obj *obj, f32 min, f32 max, f32 accel);
void bossAddDownCount(s32 n);
s32 bossDownCountOver(s32 n);
void bossBattleStart(u8 boss);
void bossBattleWin(u8 boss);
void bossBattleLose(s32 a);
void bossRumble(s32 who, u32 pattern, s32 time);

#endif
