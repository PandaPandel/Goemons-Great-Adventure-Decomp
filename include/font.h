#ifndef FONT_H
#define FONT_H

#include "object.h"

typedef struct TextFrame {
    /* 0x00 */ Frame frame;
    /* 0x14 */ f32 x;
    /* 0x18 */ f32 y;
    /* 0x1C */ s16 w; /* the cell */
    /* 0x1E */ s16 h;
    /* 0x20 */ s16 xgap;
    /* 0x22 */ s16 ygap;
    /* 0x24 */ u32 base; /* the file at 0x08000000 when made (fileMappedAddr) */
    /* 0x28 */ u32 text;
    /* 0x2C */ u8 flags; /* 1: bilinear filtering */
    /* 0x2D */ u8 r;
    /* 0x2E */ u8 g;
    /* 0x2F */ u8 b;
    /* 0x30 */ u8 a;
} TextFrame; /* size 0x34 */

void frmFontSetCell(TextFrame *f, s32 w, s32 h, s32 xgap, s32 ygap);
Gfx *frmFontDrawGlyph(Gfx *gfx, f32 x, f32 y, s32 w, s32 h, u8 *glyph);
s32 frmFontHexDigit(u8 c);
Gfx *frmFontDrawText(Gfx *gfx, TextFrame *f);
Gfx *frmFontDraw(Gfx *gfx, TextFrame *f);
TextFrame *frmFontCreate(Frame *parent, s16 x, s16 y, char *text);
Frame *frmFontDestroy(TextFrame *f);
s32 frmFontNewKind(void);
void frmFontDeleteKind(void);
s32 frmFontUnusedStub(TextFrame *f, s32 a, s32 b);
s32 frmFontGetWidth(TextFrame *f);
s32 frmFontGetHeight(TextFrame *f);

#endif
