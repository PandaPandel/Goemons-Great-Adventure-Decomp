#ifndef PAKFILER_H
#define PAKFILER_H

#include "object.h"
#include "contpak.h"

/* The cursor's work. */
typedef struct PakCursor {
    /* 0x00 */ s32 state; /* 1: moving; 2: chosen; 4: held (a message is up) */
    /* 0x04 */ s32 timer; /* since it last moved */
    /* 0x08 */ s32 row;
    /* 0x0C */ DLFrame *frame;
    /* 0x10 */ s32 shown;
} PakCursor;

typedef struct PakCursorObj {
    /* 0x00 */ Obj obj;
    /* 0x20 */ PakCursor w;
} PakCursorObj;

/* The list task's work (at 0x20 in the task). */
typedef struct PakList {
    /* 0x00 */ s32 state; /* 0 read the Pak; 1 browse; 2 confirm a delete; 3 cancelled; 4-5 a
                           * damaged Pak (Z repairs); 6 delete; 7-8 reload (A); 9 an error;
                           * 10 done */
    /* 0x04 */ s32 port;
    /* 0x08 */ s32 error; /* a PFS error, or 0x100 no Pak, 0x101 no controller, 0x200 a Rumble
                           * Pak */
    /* 0x0C */ s32 unused0C; /* cleared when the list starts reading, never read */
    /* 0x10 */ u16 used[16]; /* 1: the note is there */
    /* 0x30 */ PakCursorObj *cursor;
    /* 0x34 */ Frame *text; /* the 28 lines */
    /* 0x38 */ Frame *text2;
} PakList;

typedef struct PakListObj {
    /* 0x00 */ Obj obj;
    /* 0x20 */ PakList w;
} PakListObj;

s32 pakFiler_proc(Obj *obj, s16 msg, void *arg);
s32 pakFilerIsRunning(s32 unused);
s32 bg_proc(Obj *obj, s16 msg, void *arg);
Obj *pakFilerStartBg(Obj *obj);
s32 list_proc(Obj *obj, s16 msg, void *arg);
Obj *pakFilerStartList(Obj *obj);
s32 pakFilerListRead(PakListObj *list);
s32 pakFilerListIsReading(PakListObj *list);
void pakFilerUnusedStub(void);
void pakFilerListAskReload(PakListObj *list);
void pakFilerListMakeLines(PakListObj *list);
s32 pakFilerListShowError(PakListObj *list);
s32 pakFilerCursorProc(Obj *obj, s16 msg, void *arg);
Obj *pakFilerStartCursor(Obj *obj);
s32 pakFilerCursorIsChosen(PakCursorObj *cursor);
void pakFilerCursorHold(PakCursorObj *cursor);
void pakFilerCursorRelease(PakCursorObj *cursor);
void pakFilerCursorShow(PakCursorObj *cursor);
void pakFilerCursorHide(PakCursorObj *cursor);
void pakFilerFillRect(s32 ulx, s32 uly, s32 lrx, s32 lry, s32 r, s32 g, s32 b, s32 a);
Gfx *pakFilerDrawI4Tiles(Gfx *gfx, f32 x, f32 y, f32 scale, s32 w, s32 h, u32 tex);
s32 pakFilerIsPlug(u8 *pattern);
u8 pakFilerFontChar(u8 c);
u8 *pakFilerStrCopy(u8 *dst, u8 *src);
void pakFilerStrCenter(u8 *line, s32 width, u8 *s);
u8 *pakFilerNum3(u8 *dst, s32 n);

#endif
