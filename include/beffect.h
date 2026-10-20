#ifndef BEFFECT_H
#define BEFFECT_H

#include "common.h"
#include "object.h"
#include "player.h"
#include "game.h"
#include "boss.h"

/* "Bspace"'s work: a raster effect, the battle's (or a demo's). */
typedef struct BspaceWork {
    /* 0x00 */ DLFrame *frame;
    /* 0x04 */ s32 time;
    /* 0x08 */ s32 demo;
    /* 0x0C */ s32 unusedC; /* set to 0 when made, never read */
} BspaceWork;

s32 beBeffectProc(Obj *obj, s16 msg, void *arg);
s32 beFilterProc(Obj *obj, s16 msg, void *arg);
s32 beEfilterProc(Obj *obj, s16 msg, void *arg);
s32 bePfilterProc(Obj *obj, s16 msg, void *arg);
void beHitringCreate(f32 x, f32 y, f32 z, f32 scale, s32 flip);
s32 beHitringProc(Obj *obj, s16 msg, void *arg);
void beHitballCreate(f32 x, f32 y, f32 z, f32 scale);
s32 beHitballProc(Obj *obj, s16 msg, void *arg);
void beHitmarkCreate(f32 x, f32 y, f32 z, f32 scale);
s32 beHitmarkProc(Obj *obj, s16 msg, void *arg);
void beATfieldCreate(f32 x, f32 y, f32 z, f32 scale);
s32 ATfield_proc(Obj *obj, s16 msg, void *arg);
void beSmoke6Create(f32 x, f32 y, f32 z, f32 size, f32 dist, s32 kind);
s32 beSmoke6Proc(Obj *obj, s16 msg, void *arg);
void beNeffectCreate(f32 x, f32 y, f32 z, f32 scale);
s32 beNeffectProc(Obj *obj, s16 msg, void *arg);
void beMeffectCreate(f32 x, f32 y, f32 z);
s32 beMeffectProc(Obj *obj, s16 msg, void *arg);
void beMeffect2Create(f32 x, f32 y, f32 z);
s32 beMeffect2Proc(Obj *obj, s16 msg, void *arg);
void beM3effectCreate(f32 x, f32 y, f32 z);
s32 beM3effectProc(Obj *obj, s16 msg, void *arg);
void beM3effectTintCreate(f32 x, f32 y, f32 z, f32 rot);
s32 beM3effectTintProc(Obj *obj, s16 msg, void *arg);
s32 beFlashFProc(Obj *obj, s16 msg, void *arg);
void beFlashFCreate(s32 r, s32 g, s32 b, s32 time);
void beMegaballCreate(f32 x, f32 y, f32 z, f32 scale, s32 time);
s32 beMegaballProc(Obj *obj, s16 msg, void *arg);
s32 beBeltProc(Obj *obj, s16 msg, void *arg);
void beBeltCreate(s32 x1, s32 y1, s32 x2, s32 y2, s32 time);
void beFlashPCreate(f32 x, f32 y, f32 z, s32 count);
s32 beFlashPProc(Obj *obj, s16 msg, void *arg);
void beM5effectCreate(f32 x, f32 y, f32 z);
s32 beM5effectProc(Obj *obj, s16 msg, void *arg);
void beM6effectCreate(f32 x, f32 y, f32 z, f32 scale, f32 dist);
s32 beM6effectProc(Obj *obj, s16 msg, void *arg);
void beM7effectCreate(f32 x, f32 y, f32 z, f32 scale, f32 dist);
s32 beM7effectProc(Obj *obj, s16 msg, void *arg);
void bePunchECreate(f32 x, f32 y, f32 z, f32 scale, f32 dist);
s32 bePunchEProc(Obj *obj, s16 msg, void *arg);
s32 beFadeProc(Obj *obj, s16 msg, void *arg);
void beFadeCreate(s32 y1, s32 y2, s32 time, s32 keep);
void beStarCreate(f32 x, f32 y, f32 z);
s32 beStarProc(Obj *obj, s16 msg, void *arg);
void beRedStarCreate(f32 x, f32 y, f32 z);
s32 beRedStarProc(Obj *obj, s16 msg, void *arg);
void beEffectPCreate(f32 x, f32 y, f32 z, f32 scale, s32 kind);
s32 beEffectPProc(Obj *obj, s16 msg, void *arg);
void beReversalCreate(f32 x, f32 y, f32 z, f32 scale);
s32 beReversalProc(Obj *obj, s16 msg, void *arg);
s32 beBpieceProc(Obj *obj, s16 msg, void *arg);
void beBpieceCreate(f32 x, f32 y, f32 z, f32 scale, s32 spread);
s32 bePsmokeProc(Obj *obj, s16 msg, void *arg);
void bePsmokeCreate(f32 x, f32 y, f32 z, f32 scale, s32 kind);
s32 beReversalBigProc(Obj *obj, s16 msg, void *arg);
void beReversalBigCreate(f32 x, f32 y, f32 z);
s32 bePsmokeGreyProc(Obj *obj, s16 msg, void *arg);
void bePsmokeGreyCreate(f32 x, f32 y, f32 z, f32 scale, s32 kind);
void beNullattackCreate(f32 x, f32 y, f32 z, s32 kind);
s32 beNullattackProc(Obj *obj, s16 msg, void *arg);
s32 beRyouProc(Obj *obj, s16 msg, void *arg);
void ryouCreate(f32 x, f32 y, f32 z, f32 scale);
void beLballCreate(f32 x, f32 y, f32 z, s32 unused);
s32 beLballProc(Obj *obj, s16 msg, void *arg);
s32 beSpeffectProc(Obj *obj, s16 msg, void *arg);
void beSpeffectCreate(f32 x, f32 y, f32 z, s32 kind);
s32 beFakeImpactProc(Obj *obj, s16 msg, void *arg);
void fakeImpactCreate(f32 x, f32 y, f32 z, s32 n);
s32 beBspaceProc(Obj *obj, s16 msg, void *arg);
Gfx *beDrawRasterBand(Gfx *gfx, s32 y, s32 h, s32 skew);
Gfx *beBspaceDrawRaster(Gfx *gfx, Obj *obj, BspaceWork *w);
s32 beMegaRefProc(Obj *obj, s16 msg, void *arg);
void beMegaRefCreate(f32 x, f32 y, f32 z);
s32 beBrownFProc(Obj *obj, s16 msg, void *arg);
void beBrownFCreate(f32 x, f32 y, f32 z, s32 unused);
s32 beShibukiProc(Obj *obj, s16 msg, void *arg);
void beShibukiCreate(f32 x, f32 y, f32 z, s32 kind);

#endif
