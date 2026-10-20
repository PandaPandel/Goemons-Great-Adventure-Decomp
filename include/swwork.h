#ifndef SWWORK_H
#define SWWORK_H

#include "game.h"

/* Where a flag is: its byte in a bank, and its bit. */
typedef struct SwRef {
    /* 0x0 */ u16 index;
    /* 0x2 */ u8 mask;
    /* 0x4 */ u8 *base;
} SwRef;

void swWorkSet(s32 flag);
void swWorkReset(s32 flag);
u8 swWorkTest(s32 flag);
void swWorkClearGame(void);
void swWorkClear800(void);
void swWorkClear808(void);
SwRef swWorkCheck(s32 flag);

#endif
