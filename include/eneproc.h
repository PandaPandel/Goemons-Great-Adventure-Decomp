#ifndef ENEPROC_H
#define ENEPROC_H

#include "enemy.h"
#include "game.h"
#include "vram.h"

s32 eneHitValidCheck(Obj *o);
s32 eneAfterProc(Obj *o);
s32 eneSetOnEarth(Obj *o);
CollTri *eneLandGround(Obj *o, f32 h);
s32 eneAtamaCheck(Obj *o, f32 h);
void eneEdgeCheck(Obj *o, f32 h, f32 dx, f32 down);
void eneWallCheck(Obj *o, f32 dx, f32 dy);
s32 eneBlockCheck(Obj *o, f32 d, f32 dx, f32 dy);
s32 eneCamNearCheck(Obj *o, f32 r);
s32 eneNearestPlInX(Obj *o, f32 r);
s32 eneNearestPlInR(Obj *o, f32 r);
s32 enePlInRangeCheck(Obj *o, f32 r, s32 n);
s32 eneNearestPlWay(Obj *o);
s32 eneNearestPl(Obj *o);
f32 enePlDist(Obj *o, s32 n, s32 mask);
s32 enePlDir(Obj *o, s32 n);
s32 enePlDirX(Obj *o, s32 n);
s32 enePlDirY(Obj *o, s32 n);
f32 eneCourseAngle(Obj *o, s32 way);
s32 eneFaceWay(Obj *o);
s32 eneTurnRotYTo(Obj *o, f32 a, f32 step);
AnimeFrame *eneAnimeModelMake(Obj *o, u32 anim, Gfx *dl, s32 second);
AnimeFrame *eneAnimeModelAlpha(Obj *o, u32 anim, s32 alpha);
PolFrame *enePolModelUnder(Obj *o, Gfx *dl2, Frame *parent);
PolFrame *enePolModelMake(Obj *o, Gfx *dl2, Gfx *dl, s32 second);
PolFrame *enePolModelAlpha(Obj *o, Gfx *dl2, s32 alpha);
PolFrame *enePolModelAlphaNext(Obj *o, Gfx *dl2, s32 alpha);
void eneSetSd(Obj *o, u16 se);
void eneSetSd2(Obj *o, u16 se, u16 se2, u8 mode);
s32 eneAnimeStepTo(Obj *o, AnimeFrame *f, f32 step, f32 end);
s32 eneAnimeLoop(Obj *o, AnimeFrame *f, f32 step);
s32 eneAnimeLoopLen(Obj *o, AnimeFrame *f, f32 step, f32 len);
void eneAtkSwitch(Obj *o, ACollFrame *atk, s32 on);
void eneBlowOff(Obj *o, ACollFrame *box);
void eneHitSpark(Obj *o, ACollFrame *box);
void eneDeathAct(Obj *o, s32 kind);
void eneKillReward(Obj *o, s32 r, s32 many);
Gfx *eneDlEnvColor(Obj *o, s32 a, s32 r, s32 g, s32 b);
Gfx *eneDlEnvColorLit(Obj *o, s32 a, s32 r, s32 g, s32 b, s32 lit);
Gfx *eneDlEnvPrimColor(Obj *o, s32 r, s32 g, s32 b, s32 pr, s32 pg, s32 pb, s32 a);
s32 eneMoveToPos(Obj *o, f32 tx, f32 tz, f32 speed);
s32 eneMoveToPos3D(Obj *o, f32 tx, f32 ty, f32 tz, f32 speed);
s32 eneTurnToPoint(Obj *o, f32 tx, f32 tz, f32 deg);
s32 eneTurnToAngle(Obj *o, f32 deg, f32 stepdeg);
void eneSetAngle(Obj *o, f32 deg);
s32 eneNearerPl12(Obj *o, f32 r);
s32 eneMoveToX(Obj *o, f32 x, f32 speed);
s32 eneMoveToXZ(Obj *o, f32 x, f32 z, f32 sx, f32 sz);
void eneFaceMoveDir(Obj *o);
void eneFaceAwayMove(Obj *o);

#endif
