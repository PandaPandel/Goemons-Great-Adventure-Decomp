#ifndef BAPPCAM_H
#define BAPPCAM_H

#include "common.h"
#include "object.h"
#include "camera.h"
#include "boss.h"

s32 change(Obj *obj, s16 msg, void *arg);
s32 bappcamCannon(Obj *obj, s16 msg, void *arg);
s32 bappcamHundredPunch(Obj *obj, s16 msg, void *arg);

#endif
