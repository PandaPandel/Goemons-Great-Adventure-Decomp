#ifndef DIARY_H
#define DIARY_H

#include "tlb.h"

/* The journal screen's Controller Pak error window, "derr" (tlb_0018_3), and its handlers
 * (tlb_0018_2): each reports a result to the window's parent, the journal task (diarySetResult,
 * tlb_0018_0), and ends the window. */

typedef struct DerrWork DerrWork;
typedef s32 (*DerrFunc)(Obj *obj, DerrWork *w);

struct DerrWork {
    /* 0x00 */ DLFrame *frame;
    /* 0x04 */ TextFrame *text;
    /* 0x08 */ u32 timer;
    /* 0x0C */ s32 alpha; /* the highlight's, swinging between 32 and 128 */
    /* 0x10 */ s32 alphaStep;
    /* 0x14 */ s32 x; /* the box */
    /* 0x18 */ s32 y;
    /* 0x1C */ s32 w;
    /* 0x20 */ s32 h;
    /* 0x24 */ s32 count; /* the lines to choose from */
    /* 0x28 */ s32 cursor;
    /* 0x2C */ DerrFunc choose[4]; /* run by A on each line */
    /* 0x3C */ DerrFunc cancel; /* run by B */
    /* 0x40 */ DerrFunc proc; /* run each frame */
    /* 0x44 */ u8 lines[4]; /* the text lines each choice spans */
};

void diarySetResult(Obj *obj, s32 result);
s32 diaryErrCancel(Obj *obj, DerrWork *w);
s32 diaryErrRumbleOk(Obj *obj, DerrWork *w);
s32 diaryErrNoSave(Obj *obj, DerrWork *w);
s32 diaryErrRetry(Obj *obj, DerrWork *w);
s32 diaryErrRepair(Obj *obj, DerrWork *w);
s32 diaryErrNoRoomQuit(Obj *obj, DerrWork *w);
s32 diaryErrInput(Obj *obj, DerrWork *w);
s32 diaryErrQuit(Obj *obj, DerrWork *w);

#endif
