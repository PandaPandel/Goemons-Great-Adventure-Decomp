#ifndef TLB_TLB_0018_H
#define TLB_TLB_0018_H

#include "common.h"
#include "object.h"
#include "tlb.h"
#include "diary.h"

s32 diaryProc(Obj *obj, s16 msg, void *arg);
void diarySetResult(Obj *obj, s32 result);
void diaryTextCenter(TextFrame *t, f32 x, f32 y, s16 w, s16 h, u32 text);
void diarySegLoad(u32 addr, s32 n);
Gfx *diaryDrawTile4b(Gfx *gfx, f32 x, f32 y, s32 w, s32 h, s32 s, s32 t);
Obj *diarySioriMake(Obj *obj, void *arg, f32 x, f32 y, f32 scale);
void diarySioriScaleTo(Obj *obj, f32 s);
void diarySioriMoveXTo(Obj *obj, f32 x);
void diarySioriGetLeft(Obj *obj, f32 *x, f32 *y);
void diarySioriSetGreen(Obj *obj);
void diarySioriSetBlue(Obj *obj);
void diarySioriSetKind(Obj *o, s32 v);
Unk88208 *diarySioriGetSave(Obj *obj);
void diarySioriClearSave(Obj *obj);
s32 diarySioriWriteSave(Obj *obj);
s32 diarySioriGetResult(Obj *obj);
Obj *diaryBeshiMake(Obj *obj, f32 x, f32 y, f32 scale);
Obj *diaryDelcopyMake(Obj *obj, s32 v, f32 x, f32 y, f32 scale);
Obj *diaryDcursorMake(Obj *obj, f32 x, f32 y, f32 scale);
void diaryDcursorScaleTo(Obj *obj, f32 s);
void diaryDtexSetPos(Obj *obj, f32 x, f32 y);
s32 diaryErrCancel(Obj *obj, DerrWork *w);
s32 diaryErrRumbleOk(Obj *obj, DerrWork *w);
s32 diaryErrNoSave(Obj *obj, DerrWork *w);
s32 diaryErrRetry(Obj *obj, DerrWork *w);
s32 diaryErrRepair(Obj *obj, DerrWork *w);
s32 diaryErrNoRoomQuit(Obj *obj, DerrWork *w);
s32 diaryErrQuit(Obj *obj, DerrWork *w);
void diaryErrOpen(Obj *obj, s32 code);
s32 titleProc(Obj *obj, s16 msg, void *arg);
void titleSetNextTask(Obj *obj, s32 v);
Obj *titleGetTback(Obj *obj);
u8 titleGetPlayers(Obj *obj);
Gfx *titleDrawRgba16(Gfx *gfx, u32 file, f32 x, f32 y, f32 scale, s32 alpha, s32 width, s32 height, u32 tex);
Gfx *titleSetTint(Gfx *gfx, u32 file, s32 r, s32 g, s32 b, s32 a);
Gfx *titleDrawImage4b(Gfx *gfx, f32 x, f32 y, f32 scale, s32 width, s32 height, u32 tex);
Gfx *titleDrawImageCI4(Gfx *gfx, f32 x, f32 y, f32 scale, s32 width, s32 height, u32 tex);
Obj *titleTbackMake(Obj *obj);
void titleTbackFadeInSecond(Obj *obj, s32 steps);
void titleTbackTintIn(Obj *obj, s32 steps);
void titleTbackTintOut(Obj *obj, s32 steps);
void titleTbackSetSteps(Obj *obj, s32 steps);
void titleTbackTintTo(Obj *obj, s32 r, s32 g, s32 b, s32 envR, s32 envG, s32 envB);
Obj *titleTlogoMake(Obj *parent, s32 x, s32 y, s32 scale);
void titleTlogoFadeIn(Obj *obj, s32 steps);
void titleTlogoFadeOut(Obj *obj, s32 steps);
Obj *titleCopyrightMake(Obj *obj);
void titleCopyrightFadeIn(Obj *obj, s32 steps);
void titleCopyrightFadeOut(Obj *obj, s32 steps);
Obj *titleTbuttonMake(Obj *parent, s32 kind, f32 x, f32 y);
void titleTbuttonFadeIn(Obj *obj, s32 steps);
void titleTbuttonFadeOut(Obj *obj, s32 steps);
void titleTbuttonSetBlink(Obj *obj, s32 on);
s32 pseleProc(Obj *obj, s16 msg, void *arg);
u16 pseleGetTaker(Obj *o, s32 i);
void pseleSetTaker(Obj *o, s32 i, u16 v);
Obj *pseleTateMake(Obj *parent, s32 chara, f32 x, f32 y);
void pseleTateSetSteps(Obj *obj, s32 steps);
void pseleTateShow(Obj *obj);
void pseleTateHide(Obj *obj);
void pseleTateMoveTo(Obj *obj, f32 x, f32 y);
Obj *pselePscurMake(Obj *parent, s32 port, s32 count);
s32 subtProc(Obj *obj, s16 msg, void *arg);
s32 ntegataProc(Obj *obj, s16 msg, void *arg);

#endif
