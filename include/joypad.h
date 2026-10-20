#ifndef JOYPAD_H
#define JOYPAD_H

#include "object.h"

/* One frame of one controller. */
typedef struct PadHist {
    /* 0x0 */ u16 button;
    /* 0x2 */ s8 x;
    /* 0x3 */ s8 y;
} PadHist;

/* A controller's state. */
typedef struct PadState {
    /* 0x00 */ u8 flags; /* 1 connected, 2 its stick works the D-pad */
    /* 0x01 */ u8 unused1;
    /* 0x02 */ u16 held[14]; /* frames held: U, D, L, R, C-U, C-D, C-L, C-R, A, B, Z, L, R, Start */
} PadState; /* size 0x1E */

void joyInitControllers(void);
void joyReadControllers(void);
void joyUpdateHeld(void);
s32 joyProc(Obj *obj, s16 msg, void *arg);
u16 joyGetButton(s32 port, s32 back);
s8 joyGetStickX(s32 port, s32 back);
s8 joyGetStickY(s32 port, s32 back);
PadHist *joyGetPadHist(s32 port, s32 back);
PadState *joyGetPadState(s32 port);
void joySetStickDpad(s32 port, s32 on);
u16 joyGetHeldTime(s32 port, u16 mask);
void joyDemoEnd(void);
s8 joyDemoGetMode(void);
void joyDemoRecordFrame(void);
void joyDemoPlayFrame(void);
void joyDemoReset(void);
void joyDemoStartRecord(s32 n);
void joyDemoStartPlay(u32 *data, u32 file);

#endif
