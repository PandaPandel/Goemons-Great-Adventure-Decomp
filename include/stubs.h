#ifndef STUBS_H
#define STUBS_H

#include "common.h"

#ifndef AVOID_UB
void stubCallDefault(s32 a, s16 b);
#else
#include "object.h"

s32 stubCallDefault(Obj *obj, s16 msg, void *arg);
#endif
s32 stubReturnZero(s32 a, s32 b, s32 c);
void stubNop(s32 a);

#endif
