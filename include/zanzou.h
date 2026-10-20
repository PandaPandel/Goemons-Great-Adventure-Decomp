#ifndef ZANZOU_H
#define ZANZOU_H

#include "object.h"

/* A player's task, as this file reads it. */
typedef struct ZPlayer {
    /* 0x000 */ Obj obj;
    /* 0x020 */ s32 index;
    /* 0x024 */ s32 weapon;
    /* 0x028 */ s32 action; /* the player's "PA" */
    /* 0x02C */ u8 pad2C[0xF0]; /* Player's queue .. lean (player.h) */
    /* 0x11C */ f32 yaw;
    /* 0x120 */ u8 pad120[8]; /* Player's roll, leanOffset */
    /* 0x128 */ f32 yaw2;
} ZPlayer;

/* What an afterimage is made with. */
typedef struct ZanzouInit {
    /* 0x00 */ u32 anim; /* the swing, a segmented address in the file */
    /* 0x04 */ u32 file; /* the file's RAM */
    /* 0x08 */ f32 frames;
    /* 0x0C */ u8 player;
} ZanzouInit;

/* What the player overlay passes in: an action's PlActAnim entry (plmodel.c). */
typedef struct ZanzouArg {
    /* 0x00 */ u8 pad00[0xC]; /* its anim, attach, events */
    /* 0x0C */ u32 anim;
    /* 0x10 */ f32 frames;
} ZanzouArg;

void zanzouStart(s32 player, u32 file, ZanzouArg *arg);
s32 zanzou(Obj *obj, s16 msg, void *arg);
s32 zanzouSetup(Obj *obj, ZanzouInit *init);
s32 zanzouAllocBuffers(Obj *obj, s32 knots, s32 verts);
s32 zanzouStep(Obj *obj);
void zanzouDrawStrip(Vtx *vtx, s32 n, u8 *tex);
f32 zanzouPlayerYaw(ZPlayer *p);
s32 zanzouPlayerWeapon(ZPlayer *p);
s32 zanzouPlayerAction(ZPlayer *p);
void zanzouEmptyStub(void);
s8 zanzouPlayerWpLv(ZPlayer *p);
s32 zanzouLayTrack(Obj *obj, f32 *xk, f32 *yk, f32 *zk, s32 side, s32 len, f32 turn);
s32 zanzouSplineCoeffs(f32 *x, f32 *y, f32 *y2, s32 n);
f32 zanzouSplineEval(f32 x, f32 *xa, f32 *ya, f32 *y2a, s32 n);
s32 zanzouSplineMake(f32 *ta, f32 *xa, f32 *ya, f32 *za, f32 *x2a, f32 *y2a, f32 *z2a, s32 n);
void zanzouSplinePoint(f32 t, f32 *x, f32 *y, f32 *z, f32 *ta, f32 *xa, f32 *ya, f32 *za, f32 *x2a, f32 *y2a, f32 *z2a, s32 n);
s32 z_cp(Obj *obj, s16 msg, void *arg);
void zanzouLeaveCopy(Obj *obj);

#endif
