#include "common.h"
#include "stubs.h"

/* stubs.c's .data (0x80107790-0x80107798), in ROM order. */
Gfx gStubsEmptyDl[] = { 0xDF000000, 0x00000000 }; /* an empty list; nothing names it */

/* Overlay 6's first stubs: calls that do nothing or pass straight through, likely debug hooks
 * compiled out (only overlay 10 calls one of them). */

#ifndef AVOID_UB
void tskDefaultProc(s32 a, s16 b); /* FAKE: tmanager.h is not included here: tskDefaultProc: used here as this file declares it (The number of arguments doesn't agree with the number in the declaration.); tmanager.h has `s32 tskDefaultProc(Obj *obj, s16 msg, void *arg)` */

//! @bug A task handler (tlb_0018_4 makes it the game's next task) that passes two of its three
//! arguments on: tskDefaultProc gets arg from a2 untouched, and its result is left in v0.
void stubCallDefault(s32 a, s16 b) {
    tskDefaultProc(a, b);
}
#else
#include "tmanager.h"

s32 stubCallDefault(Obj *obj, s16 msg, void *arg) {
    return tskDefaultProc(obj, msg, arg);
}
#endif

s32 stubReturnZero(s32 a, s32 b, s32 c) {
    return 0;
}

void stubNop(s32 a) {
}
