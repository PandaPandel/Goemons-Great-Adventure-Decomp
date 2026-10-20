#ifndef TLB_TLB_0033_H
#define TLB_TLB_0033_H

#include "common.h"
#include "object.h"
#include "player.h"

s32 utaDemoProc(Obj *obj, s16 msg, void *arg);
s32 utaAnimeStep(AnimeFrame *a, f32 speed, s32 mode);
Obj *utaGoeCreate(Obj *obj);
Obj *utaEbiCreate(Obj *obj);
Obj *utaSasCreate(Obj *obj);
Obj *utaYaeCreate(Obj *obj);
Obj *utaOmiCreate(Obj *obj);
Obj *utaKamomeCreate(Obj *obj);
Obj *utaMach2ShrinkCreate(Obj *obj);
Obj *utaHaikeiCreate(Obj *obj);
Obj *utaJimenScrollCreate(Obj *obj);
Obj *utaBisuBigCreate(Obj *obj);
Obj *utaMach2TrioCreate(Obj *obj);
Obj *utaDou2Create(Obj *obj);
Obj *utaCutcutCreate(Obj *obj);
Obj *utaBisuCreate(Obj *obj);
Obj *utaBaraCreate(Obj *obj);
Obj *utaFballCreate(Obj *obj);
Obj *utaZako4Create(Obj *obj);
Obj *utaWaterCreate(Obj *obj);
Obj *utaZako10Create(Obj *obj);
Obj *utaBoss3Create(Obj *obj);
Obj *utaFilterCreate(Obj *obj);
Obj *utaZako14Create(Obj *obj);
Obj *utaGhostCreate(Obj *obj, s32 v);
Obj *utaJimenShadeCreate(Obj *obj);
Obj *utaCKiranCreate(Obj *obj);
Obj *utaCLballCreate(Obj *obj);
Obj *utaCFireCreate(Obj *obj);
Obj *utaCBKiranCreate(Obj *obj);
Obj *utaCCldmanCreate(Obj *obj);
Obj *utaCCldmanCreateFar(Obj *obj);
s32 utaSongProc(Obj *obj, s16 msg, void *arg);

#endif
