#ifndef WINDOW_H
#define WINDOW_H

#include "object.h"

typedef struct WinPos {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
} WinPos;

s32 winRun_proc(Obj *obj, s16 msg, void *arg);
Obj *winCreate(Obj *parent, s32 kind, u8 method);
s32 winClose(Obj *win, u8 method);
void winLoadFiles(Obj *win);
void winFreeFiles(Obj *win);
void winLoadAllFiles(void);
void winFreeAllFiles(void);
u8 winGetState(Obj *win);
void winSetState();
void winSetMethod();
void winSetKind();
WinPos winGetPos(Obj *win);
s32 winGetPlayer(Obj *win);
void winPromptOn(Obj *win);
void winPromptOff(Obj *win);
s32 win_init(Obj *obj);

#endif
