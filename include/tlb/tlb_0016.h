#ifndef TLB_TLB_0016_H
#define TLB_TLB_0016_H

#include "common.h"
#include "object.h"
#include "boss.h"

void mapPlayersHide(void);
void mapPlayersShow(void);
s32 mapAtlasProc(Obj *obj, s16 msg, void *arg);
void mapSetHold(Obj *obj, s32 on);
void mapSetCamHold(Obj *obj, s32 on);
Camera *mapGetCamera(Obj *obj);
Gfx *mapSeaDL(Gfx *gfx, u16 scroll);
Gfx *mapTaki2DL(Gfx *gfx, u16 scroll);
void mapImpactLeaveMake(Obj *parent, s32 dir, f32 x, f32 y, f32 z);
void mapImpactArriveMake(Obj *parent, s32 dir, f32 x, f32 y, f32 z);
void mapSetupSeaFalls(Obj *obj);
void mapSetupLava(Obj *obj);
void mapSetupSea(Obj *obj);
void mapSetupObisu2(Obj *obj);

#endif
