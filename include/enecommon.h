#ifndef ENECOMMON_H
#define ENECOMMON_H

#include "player.h"
#include "enemy.h"
#include "game.h"

void eneStepHookPre(Obj *o);
void eneStepHookPost(Obj *o);
s32 eneCounterReward(s32 n, s32 zone0, f32 x0, f32 y0, f32 z0, s32 zone1, f32 x1, f32 y1, f32 z1);
void eneEndPlayersPowerDown(void);
void eneRumbleAll(u32 pattern, s32 time);
void eneRumbleStopAll(void);
void eneCounterSetFlag(s32 n);
void eneCounterClearFlag(s32 n);
s32 eneCounterIncrement(s32 n);
s32 eneCounterGet(s32 n);
s32 eneMoveToward(Obj *o, f32 tx, f32 ty, f32 tz, f32 sx, f32 sy, f32 sz);
void eneHitSound(ACollFrame *box);
void eneHitSoundHeavy(ACollFrame *box);
void eneHitSoundAlt(ACollFrame *box);
void eneHitSoundByKind(ACollFrame *box, s32 heavy);
s32 eneHeadingSide(f32 a, f32 b);
f32 eneKnockbackSpeed(Obj *o, ACollFrame *box, f32 speed);
s32 eneHitStrength(ACollFrame *box);
void eneSetModelOnCourse(Obj *o, Frame *f);
void eneSetModel(Obj *o, Frame *f);
u8 eneGetCoursePos(Obj *o, Vec3f *out);
void eneWorldPos(Obj *o, Vec3f *out);
void eneGetWorldPos(Obj *o, Vec3f *out);
Obj *eneRandomPlayer(void);
u8 eneRandomPlayerIndex(void);
u8 eneCountPlayersUp(void);
void eneSetVelToward(Obj *o, f32 speed, f32 x, f32 y, f32 z);
void eneSetVelTowardXZ(Obj *o, f32 speed, f32 x, f32 z);
f32 eneDistTo(Obj *o, f32 x, f32 y, f32 z);
void eneFromModelOnCourse(Obj *o, Frame *f);
void eneFromModel(Obj *o, Frame *f);
s32 eneAnimPlayOnce(Obj *o, AnimeFrame *m, f32 step);
s32 eneAnimLoop(Obj *o, AnimeFrame *m, f32 step);
void eneTurnAlongCourse(Obj *o, f32 n);
s32 eneTurnToward(Obj *o, f32 a, f32 step);
void eneFall(Obj *o, f32 g);
void eneAccelAxis(f32 target, f32 max, f32 accel, f32 *pos, f32 *vel);
void eneMoveObj(Obj *o);
void eneMove(EneObj *e);
void eneAccelMove(EneObj *e, f32 ax, f32 ay, f32 az);
s32 eneLand(EneObj *e, f32 h);
s32 eneLandOff(EneObj *e, f32 h, f32 off);
s32 eneWallCheckPush(Obj *o, f32 h, f32 r, f32 a, f32 b);
s32 eneWallCheckNoPush(Obj *o, f32 h, f32 r, f32 a, f32 b);
s32 eneWallCheckEx(Obj *o, f32 h, f32 dx, f32 dz, f32 down, u8 push);
s32 eneRayHitPush(Obj *o, f32 h, f32 x, f32 y, f32 z);
s32 eneRayHitEx(Obj *o, f32 h, f32 x, f32 y, f32 z, u8 push);
void eneMoveAndCollide(Obj *o, f32 h, f32 r, f32 a, f32 b);
f32 eneNearestPlayerDist(Obj *o);
void enePlayersCenter(f32 *x, f32 *y, f32 *z);
void enePlayersCenterOnCourse(f32 *place, f32 *height, f32 *side);
void eneMakeKillHahen(Obj *o, ACollFrame *box, u32 dl, u32 dl2);
s32 kill_hahen(Obj *obj, s16 msg, void *arg);
Frame *eneKillHahenModel(Obj *o, EneObj *from, u32 dl, s32 life, f32 up, f32 vx, f32 vy, f32 vz, f32 scale, u8 lit);
void eneMakeFuttobiHahen(Obj *o, ACollFrame *box, u32 dl, u8 turn, u8 second);
void eneMakeFuttobiHahenAnime(Obj *o, ACollFrame *box, u32 anim, f32 frame, u8 turn, u8 second);
s32 futtobi_hahen(Obj *obj, s16 msg, void *arg);
Frame *eneFuttobiHahenModel(Obj *o, EneObj *from, u32 dl, f32 h, f32 vx, f32 vy, f32 vz, f32 scale, u8 lit, u8 turn, u8 second);
Frame *eneFuttobiHahenAnime(Obj *o, EneObj *from, u32 anim, f32 frame, f32 h, f32 vx, f32 vy, f32 vz, f32 scale, u8 lit, u8 turn, u8 second);
void eneMakeHitSpark(s32 zone, f32 x, f32 y, f32 z, s16 a, f32 angle);

#endif
