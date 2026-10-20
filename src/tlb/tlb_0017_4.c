#include "tlb.h"
#include "tlb/tlb_0017.h"

s32 openingIdleProc(Obj *obj, s16 msg, void *arg) {
    return tskDefaultProc(obj, msg, arg);
}

s32 openingIdleProc(Obj *obj, s16 msg, void *arg);

Obj *openingIdleMake(Obj *obj) {
    return tskCreate(obj, openingIdleProc, obj->file, NULL);
}
