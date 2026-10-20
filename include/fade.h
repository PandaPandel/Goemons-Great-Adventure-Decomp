#ifndef FADE_H
#define FADE_H

#include "object.h"
#include "game.h"
#include "demotool.h"

s32 fadeProc(Obj *obj, s16 msg, void *arg);
Obj *fadeCreate(Obj *parent);
void fadeSetSpeed(f32 speed);
void fadeSetLevel(f32 level);
void fadeSetColor(u8 r, u8 g, u8 b);
void fadeSetTarget(f32 target);
void fadeSetLetterbox(s32 width, s32 top, s32 bottomLeft, s32 bottom);
s32 fadeCloseLetterbox(s32 n);
s32 fadeOpenLetterbox(void);
s32 fadeMoveLetterbox(s32 top, s32 bottom, s32 speed);
void fadeSetParent(s32 arg);
void fadeSetAll(f32 speed, f32 level, f32 target, u8 r, u8 g, u8 b);
void fadeGetColor(u8 *r, u8 *g, u8 *b);
s32 fadeRand(s32 n);
f32 fadeRandRange(s32 lo, s32 hi);
u32 fadeSegToRam(u32 addr, u32 file);
void fadeAnimStart(Anim *a, AnimFrame *frames, s32 index, u32 *out, u32 file);
s32 fadeAnimStep(Anim *a);
u32 fadeAnimFrameAt(Anim *a, s32 t);
void fadeLightNormalize(LightFrame *f);
f32 fadeViewWidth(CamView *v);
s32 fadeCountPlayers(void);
s32 fadeStartPressed(void);
void fadeRumbleAll(u32 pattern, s32 time);
s32 fadeToViewSpace(Vec3f *v, Vec3f *out, Vec3f *d);
f32 fadeProjectView(CamView *view, f32 x, f32 y, f32 z, f32 *sx, f32 *sy);
f32 fadeProjectScreen(f32 x, f32 y, f32 z, f32 *sx, f32 *sy);
void fadeViewMatrix(f32 out[4][4], CamView *view);
Obj *fadeCreateRetry(Obj *parent, s32 (*handler)(Obj *, s16, void *), s32 prio, s32 a, void *arg);
void fadeNewSave(Unk88208 *save);
s32 fadePakInit(void);
s32 fadeReadSave(Unk88208 *save, s32 slot);
s32 fadeWriteSave(Unk88208 *save, s32 slot);
void fadeClearSettings(void);
s32 fadeReadSettings(void);
s32 fadeWriteSettings(void);

#endif
