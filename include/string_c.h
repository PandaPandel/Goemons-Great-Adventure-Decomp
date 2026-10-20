#ifndef STRING_C_H
#define STRING_C_H

#include "object.h"

/* A character: its frame, a blink timer and flags (1, 2, 4: from variables 0x08000054,
 * 0x08000040 and 0x08000058; 4 blinks). */
typedef struct StrChar {
    /* 0x0 */ struct FontFrame *frame;
    /* 0x4 */ u16 timer;
    /* 0x6 */ u8 flags;
} StrChar; /* size 0x8 */

/* A character's frame (the "afont" kind). */
typedef struct FontFrame {
    /* 0x00 */ Frame frame;
    /* 0x14 */ s32 x;
    /* 0x18 */ s32 y;
    /* 0x1C */ s32 size;
    /* 0x20 */ void *glyph;
    /* 0x24 */ Color3 color;
    /* 0x27 */ u8 flags; /* 1: hidden (a blink) */
} FontFrame;

typedef struct StrPos {
    s16 x, y;
} StrPos;

s32 strRun_proc(Obj *obj, s16 msg, void *arg);
#ifdef AVOID_UB
/* In the matching build its one caller, scenario.c, declares it with no parameters (see there). */
Obj *strCreate(Obj *parent);
#endif
Obj *strDestroy(Obj *obj);
FontFrame *_strInit(Frame *parent, void *glyph, s32 no, Color3 color, f32 s, f32 x, f32 y);
void _strSet(Obj *obj, s32 c);
void _strAdd(Obj *obj, s32 c);
s32 strAddByTimer(Obj *obj, s32 c);
void _strClear(Obj *obj);
StrPos strGetPos(Obj *obj);
void strSetColor(Obj *obj, s32 c);
void strSetSize(Obj *obj, s32 size, s32 xgap, s32 ygap);
void strSetAlpha(Obj *obj, s32 alpha);
void strSetCharFlags(Obj *obj);
void strSetBlinkTimer(Obj *obj, s32 timer);
void strBlinkAll(Obj *obj);
void strBlinkChar(StrChar *c);

#endif
