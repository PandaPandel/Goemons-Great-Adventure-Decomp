#ifndef DMENU_H
#define DMENU_H

#include "common.h"
#include "object.h"
#include "game.h"
#include "ascii.h"

typedef struct DMenu {
    /* 0x00 */ s32 (*proc)(Obj *obj, struct DMenu *w);
    /* 0x04 */ Frame *title;
    /* 0x08 */ Frame *list; /* a line per demo */
    /* 0x0C */ AsciiFrame *cursor; /* "$", a child of the chosen line */
    /* 0x10 */ void *ram; /* where the demo's file loads */
    /* 0x14 */ u8 state;
} DMenu;

s32 Dmenu(Obj *obj, s16 msg, void *arg);
s32 dmenuProcDone(Obj *obj, DMenu *w);
s32 dmenuProcSelect(Obj *obj, DMenu *w);
void dmenuQueueDemo1(void);
void dmenuQueueDemo2(void);
void dmenuGoStage9A(void);
void dmenuGoStageB6(void);
void dmenuGoStageC9(void);
void dmenuQueueDemo(s32 demo);

#endif
