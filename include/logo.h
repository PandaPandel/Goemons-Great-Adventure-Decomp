#ifndef LOGO_H
#define LOGO_H

#include "object.h"

/* A texture a sprite list draws from. */
typedef struct LogoTex {
    /* 0x0 */ u16 w;
    /* 0x2 */ u16 h;
    /* 0x4 */ u8 pad4[4]; /* { 9, 0, 0, 0 } in every entry; never read */
    /* 0x8 */ u8 *addr;
} LogoTex; /* size 0xC */

/* A rectangle of a sprite list. */
typedef struct LogoRect {
    /* 0x0 */ u8 flags; /* 0x10: the next texture; 0x80: the last */
    /* 0x1 */ u8 w;
    /* 0x2 */ u8 h;
    /* 0x3 */ u8 unused3;
    /* 0x4 */ s16 x;
    /* 0x6 */ s16 y;
    /* 0x8 */ u16 s;
    /* 0xA */ u16 t;
} LogoRect; /* size 0xC */

s32 logoProc(Obj *obj, s16 msg, void *arg);
s32 logoIsDone(s32 unused);
void logoUpdateFirst(void);
void logoUpdateSecond(void);
Gfx *logoDraw(Gfx *gfx);
void logoDrawSprites(LogoTex *tex, u16 *tlut, LogoRect *rect, u8 alpha);

#endif
