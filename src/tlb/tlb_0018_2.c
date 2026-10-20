#include "diary.h"
#include "tlb/tlb_0018.h"

/* The Controller Pak error window's handlers: each reports its result to the window's parent
 * and returns -1, which ends the window. */

s32 diaryErrCancel(Obj *obj, DerrWork *w) {
    diarySetResult(obj->parent, -2);
    return -1;
}

s32 diaryErrRumbleOk(Obj *obj, DerrWork *w) {
    diarySetResult(obj->parent, -1);
    return -1;
}

s32 diaryErrNoSave(Obj *obj, DerrWork *w) {
    diarySetResult(obj->parent, -5);
    return -1;
}

s32 diaryErrRetry(Obj *obj, DerrWork *w) {
    diarySetResult(obj->parent, -4);
    return -1;
}

/* "Repair": probes the Pak in port 0, repairs its file system, and reports as a retry. */
s32 diaryErrRepair(Obj *obj, DerrWork *w) {
    contPakProbe(0);
    contPakInitPfsRepair(0);
    diarySetResult(obj->parent, -4);
    return -1;
}

/* "No room": the window closes itself through diaryErrQuit. */
s32 diaryErrNoRoomQuit(Obj *obj, DerrWork *w) {
    w->timer = 0;
    w->proc = diaryErrQuit;
    return 0;
}
