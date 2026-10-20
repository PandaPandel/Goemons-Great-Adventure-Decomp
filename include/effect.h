#ifndef EFFECT_H
#define EFFECT_H

#include "object.h"
#include "demotool.h"

/* A puff's work (also what it is made with). */
typedef struct FxWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ Anim anim; /* its texture animation */
    /* 0x14 */ f32 pos[3];
    /* 0x20 */ f32 vel[3];
    /* 0x2C */ f32 scale;
    /* 0x30 */ f32 grow;
    /* 0x34 */ s32 alpha;
    /* 0x38 */ s32 fade;
    /* 0x3C */ u8 r;
    /* 0x3D */ u8 g;
    /* 0x3E */ u8 b;
    /* 0x3F */ u8 kind; /* 2: additive */
    /* 0x40 */ u8 unused40; /* copied from the init, never read */
} FxWork; /* size 0x44 */

s32 effect(Obj *obj, s16 msg, void *arg);
s32 effectPuffProc(Obj *obj, s16 msg, void *arg);
void effectPuff(f32 x, f32 y, f32 z, u8 kind);
void effectPuffRise(f32 x, f32 y, f32 z, f32 scale, f32 grow, f32 vy, u8 kind);
void effectPuffMake(FxWork init);
void effectPuffDefaults(FxWork *init);
void effectBurst(f32 x, f32 y, f32 z);
s32 kiran(Obj *obj, s16 msg, void *arg);
void effectKiranCreate(f32 x, f32 y, f32 z);
void effectKiranCreateSized(f32 x, f32 y, f32 z, f32 size, s32 rise, s32 fade);
s32 Fspirit(Obj *obj, s16 msg, void *arg);
void effectFspiritCreate(f32 x, f32 y, f32 z, f32 scale, u8 r, u8 g, u8 b, u8 r2, u8 g2, u8 b2);
s32 pompom(Obj *obj, s16 msg, void *arg);
Obj *effectPompomCreate(f32 x, f32 y, f32 z, f32 sx, f32 sy, f32 sz, s32 kind, s32 life, s32 interval, f32 size, f32 var);
s32 effectPompomSetPos(Obj *obj, f32 x, f32 y, f32 z);
void effectStarCreate(f32 x, f32 y, f32 z);
s32 effectStarProc(Obj *obj, s16 msg, void *arg);
s32 sibuki(Obj *obj, s16 msg, void *arg);
void effectSibukiCreateWhite(f32 x, f32 y, f32 z, f32 size, f32 grow, f32 a, f32 b);
Obj *effectSibukiCreate(f32 x, f32 y, f32 z, f32 size, f32 grow, f32 a, f32 b, f32 c, f32 d, u8 r, u8 g, u8 bl, u8 r2, u8 g2, u8 b2);

#endif
