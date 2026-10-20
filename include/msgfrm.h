#ifndef MSGFRM_H
#define MSGFRM_H

#include "object.h"
#include "file.h"

/* A character frame. */
typedef struct AFontFrame {
    /* 0x00 */ Frame frame;
    /* 0x14 */ s32 x;
    /* 0x18 */ s32 y;
    /* 0x1C */ s32 size;
    /* 0x20 */ u8 *glyph;
    /* 0x24 */ Color3 color;
    /* 0x27 */ u8 flags;
} AFontFrame; /* size 0x28 */

/* A cursor or prompt frame. */
typedef struct MarkFrame {
    /* 0x00 */ Frame frame;
    /* 0x14 */ s16 x;
    /* 0x16 */ s16 y;
    /* 0x18 */ u8 blink;
    /* 0x19 */ u8 flags;
} MarkFrame; /* size 0x1C */

/* A window border or face frame (the "win" and "face" kinds). */
typedef struct PicFrame {
    /* 0x00 */ Frame frame;
    /* 0x14 */ void *tex;
    /* 0x18 */ s16 x;
    /* 0x1A */ s16 y;
    /* 0x1C */ u8 flags;
} PicFrame; /* size 0x20 */

Gfx *frmAFontSetup(Gfx *gfx, Color3 color);
Gfx *frmAFontDrawGlyph(Gfx *gfx, s32 x, s32 y, s32 size, u8 *glyph);
Gfx *frmAFontDraw(Gfx *gfx, AFontFrame *f);
AFontFrame *afontCreate(Frame *parent);
void frmAFontDestroy(AFontFrame *f);
void frmAFontShow(AFontFrame *f);
void frmAFontHide(AFontFrame *f);
s32 frmAFontKindCreate(void);
void frmAFontKindDelete(void);
void frmMsgFilesLoad(void);
void frmMsgFilesFree(void);
Gfx *frmCursorDrawAt(Gfx *gfx, s32 x, s32 y, u8 blink);
Gfx *frmCursorDraw(Gfx *gfx, MarkFrame *f);
MarkFrame *cursorCreate(Frame *parent);
void frmCursorDestroy(MarkFrame *f);
s32 frmCursorKindCreate(void);
void frmCursorKindDelete(void);
Gfx *frmPromptDrawAt(Gfx *gfx, s32 x, s32 y, u8 blink);
Gfx *frmPromptDraw(Gfx *gfx, MarkFrame *f);
MarkFrame *promptCreate(Frame *parent);
void frmPromptDestroy(MarkFrame *f);
s32 frmPromptKindCreate(void);
void frmPromptKindDelete(void);
Gfx *frmWindowDrawAt(Gfx *gfx, void *tex, s32 x, s32 y);
Gfx *frmWindowDraw(Gfx *gfx, PicFrame *f);
PicFrame *winFrmCreate(Frame *parent);
void frmWindowDestroy(PicFrame *f);
s32 frmWindowKindCreate(void);
void frmWindowKindDelete(void);
Gfx *frmFaceDrawAt(Gfx *gfx, void *tex, s32 x, s32 y);
Gfx *frmFaceDraw(Gfx *gfx, PicFrame *f);
PicFrame *faceFrmCreate(Frame *parent);
void frmFaceDestroy(PicFrame *f);
s32 frmFaceKindCreate(void);
void frmFaceKindDelete(void);

#endif
