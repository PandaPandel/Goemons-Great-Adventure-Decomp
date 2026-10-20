#include "common.h"
#include "object.h"
#include "bstub.h"

/* A task handler that only answers 1 (overlay 10's file of one function). */
s32 bstubNullTask(Obj *obj, s16 msg, void *arg) {
    return 1;
}
