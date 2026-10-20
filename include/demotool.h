#ifndef DEMOTOOL_H
#define DEMOTOOL_H

#include "common.h"
#include "object.h"
#include "player.h"
#include "game.h"
#include "camera.h"

typedef struct Anim {
    /* 0x00 */ u32 file;
    /* 0x04 */ s16 tick;
    /* 0x06 */ s16 index;
    /* 0x08 */ AnimFrame *frames;
    /* 0x0C */ u32 *out;
} Anim;

Gfx *demoColorDL(s32 r, s32 g, s32 b, s32 er, s32 eg, s32 eb, s32 ea);
Gfx *demoShadeDL(void);
Gfx *demoModulateDL(void);
void demoTexAnimK0(Anim *a, s32 which, u32 *out);
void demoTexAnimK1(Anim *a, s32 which, u32 *out);
void demoTexAnimK2(Anim *a, s32 which, u32 *out);
void demoTexAnimK3(Anim *a, s32 which, u32 *out);
void demoPolSetDL(PolFrame *f, Gfx *dl, s32 n);
void demoNop(void);
void demoAnimeSet(AnimeFrame *f, u32 anim, s32 n, f32 at);
s32 demoAnimeStep(AnimeFrame *f, f32 speed, s32 mode);
void gfxReplace(AnimeFrame *f, u32 *from, u32 file1, u32 *to, u32 file2);
f32 demoTurnToward(f32 x, f32 z, f32 tx, f32 tz, f32 rot, f32 step);
s32 modelProc(Obj *obj, s16 msg, void *arg);
s32 modelChk(Obj *obj, s16 msg, void *arg);
void demoModelCreate(Obj *parent, f32 x, f32 y, f32 z, f32 sx, f32 sy, f32 sz, f32 rx, f32 ry, f32 rz, s32 set, Gfx *dl, s32 which, s32 kind);
s32 CAMchk(Obj *obj, s16 msg, void *arg);
void demoCamChkOpen(f32 fov);
s32 JOYchk(Obj *obj, s16 msg, void *arg);
void demoJoyChkOpen(void);
s32 OBJMOVE(Obj *obj, s16 msg, void *arg);
void demoObjMoveOpen(void);
s32 tunagi_anime(AnimeFrame *f, s32 step, s32 mode1, s32 mode2, f32 speed1, f32 speed2, s32 set1, u32 anim1, s32 set2, u32 anim2);
void demoTexAnim(s32 kind, Anim *a, s32 which, u32 *out);
s32 demoPlTunagiAnime(Obj *obj, s32 which, s32 step, s32 mode1, s32 mode2, f32 speed1, f32 speed2, s32 action1, s32 action2);
s32 democam(Obj *obj, s16 msg, void *arg);
s32 demoWalkTo(AnimeFrame *f, f32 animSpeed, f32 speed, f32 tx, f32 tz);
void demoFadeStart(f32 speed, f32 level, f32 target, u8 r, u8 g, u8 b);
void demoFadeForce(f32 speed, f32 level, f32 target, u8 r, u8 g, u8 b);
void demoRumbleAll(u32 pattern, s32 time);

#endif
