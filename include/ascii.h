#ifndef ASCII_H
#define ASCII_H

#include "object.h"

typedef struct AsciiFrame {
    /* 0x00 */ Frame f;
    /* 0x14 */ s16 x;
    /* 0x16 */ s16 y;
    /* 0x18 */ char *str;
} AsciiFrame; /* size 0x1C */

Gfx *asciiDraw(Gfx *gfx, AsciiFrame *f);
AsciiFrame *asciiNew(Frame *parent, s16 x, s16 y, char *str);
void asciiNewKind(s32 count);

#endif
