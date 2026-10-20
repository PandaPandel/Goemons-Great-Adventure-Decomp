#ifndef DIVEEF_H
#define DIVEEF_H

#include "player.h"

s32 plkira(Obj *obj, s16 msg, void *arg);
s32 plSuichuAwa(Obj *obj, s16 msg, void *arg);
Obj *plSuichuAwaCreate(Obj *task);
s32 plMoya(Obj *obj, s16 msg, void *arg);
Obj *plMoyaCreate(Obj *task);
void plMoyaPlace(Obj *moya, Player *p);
s32 diveef(Obj *obj, s16 msg, void *arg);
void plDiveefCreate(Player *p, f32 height);
void plWaterSplash(Player *p);
void plWaterSoundLater(Player *p, u16 se, s16 wait);

#endif
