#ifndef BMENU_H
#define BMENU_H

#include "common.h"
#include "object.h"
#include "game.h"
#include "ascii.h"

typedef struct BMenu {
    /* 0x00 */ s32 (*proc)(Obj *obj, struct BMenu *w);
    /* 0x04 */ Frame *title;
    /* 0x08 */ Frame *list; /* a line per boss */
    /* 0x0C */ AsciiFrame *cursor; /* "$", a child of the chosen line */
    /* 0x10 */ u8 state;
} BMenu;

s32 Bmenu(Obj *obj, s16 msg, void *arg);
s32 bmenuProcChosen(Obj *obj, BMenu *w);
s32 bmenuProcSelect(Obj *obj, BMenu *w);

#endif
