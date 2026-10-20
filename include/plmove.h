#ifndef PLMOVE_H
#define PLMOVE_H

#include "player.h"

/* The ground, as each step finds it (Player+0x19C-0x1AB). */
typedef struct PlGround {
    /* 0x0 */ s16 ahead; /* ahead */
    /* 0x2 */ s16 ground; /* under */
    /* 0x4 */ s16 above; /* above */
    /* 0x6 */ u8 soundKind;
    /* 0x8 */ u16 edgeFlags; /* 0x8000 a step behind, 0x4000 ahead */
    /* 0xC */ f32 angle; /* the slope */
} PlGround;

/* A step to probe, along the course. */
typedef struct PlProbe {
    /* 0x00 */ f32 place;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 side;
    /* 0x0C */ Vec3f d; /* the step */
    /* 0x18 */ Vec3f sign; /* its signs (0 for none) */
    /* 0x24 */ f32 heading; /* the rail's */
} PlProbe;

/* What a ray found. */
typedef struct PlHit {
    /* 0x00 */ Vec3f pos;
    /* 0x0C */ f32 t; /* along the ray; 2 for nothing */
    /* 0x10 */ s16 kind; /* the triangle's */
    /* 0x12 */ u8 soundKind;
    /* 0x14 */ s16 zone;
    /* 0x18 */ f32 slope;
} PlHit;

/* The walls' query. */
typedef struct PlWall {
    /* 0x00 */ Player *p;
    /* 0x04 */ PlGround *g;
    /* 0x08 */ Vec3f n; /* the rail's normal, flat */
    /* 0x14 */ s32 obj; /* the object walked onto */
} PlWall;

u16 plMoveMapButtons(u16 pad);
u16 plMoveGetButtons(s32 port, s32 back);
u16 plMoveHeldTime(s32 port, s32 mask);
void plMoveResetButtonMap(void);
void plMoveReadButtons(Player *p);
void plMoveStep(Player *p);
void plMoveStepFlyDive(Player *p);
void plMoveStepLetGo(Player *p);
void plMoveStepWalk(Player *p);
void plMoveProbeGround(Player *p, Vec3f *vel, s16 *zone, PlGround *g);
void plMoveWallFeet(Player *p, PlProbe *pr, PlHit *hit, PlGround *g);
void plMoveWallHead(Player *p, PlProbe *pr, PlHit *hit);
void plMoveWallWaist(Player *p, PlProbe *pr, PlHit *hit);
void plMoveCeiling(Player *p, PlProbe *pr, PlHit *hit);
void plMoveNormalizeXZ(Vec3f *v, Vec3f *out);
f32 plMoveFeetHeight(Player *p);
void plMoveClampSpeed(Player *p);
void plMoveCollideFlyDive(Player *p, Vec3f *vel, PlGround *g);
void plMoveCollideLetGo(Player *p, Vec3f *vel, PlGround *g);
void plMoveRideCollide(Player *p, Vec3f *vel, PlGround *g);
s32 plMoveRidePushOut(PlWall *c, Vec3f *pos);
s32 plMoveRideGround(PlWall *c, Vec3f *pos, f32 vx, f32 side);
s32 plMoveRideWallAhead(PlWall *c, Vec3f *pos, f32 vx);
s32 plMoveRideWallBehind(PlWall *c, Vec3f *pos, f32 vx);
s32 plMoveRideHead(PlWall *c, Vec3f *pos);
s32 plMoveRideAlong(PlWall *c, Vec3f *pos, Vec3f *d);
s32 plMoveRideAlongDown(PlWall *c, Vec3f *pos, Vec3f *d);
void plMoveWaterCarry(Player *p, Vec3f *vel);
void plMoveWater(Player *p);
s32 plMoveTiedXZ(Player *p, Vec3f *vel);
f32 plMoveDistXZ(AnimeFrame *a, AnimeFrame *b, f32 dx, f32 dz);
s32 plMoveTiedY(Player *p, f32 dy, f32 scale, s32 absolute);
void plMoveObjPush(Player *p, Vec3f *vel);

#endif
