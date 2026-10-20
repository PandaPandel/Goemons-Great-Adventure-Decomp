#ifndef VRAM_H
#define VRAM_H

#include "object.h"

/* A corner or an edge of a collision triangle. */
typedef struct CollVec {
    s16 x;
    s16 y;
    s16 z;
} CollVec;

/* A collision triangle (0x28 bytes). */
typedef struct CollTri {
    /* 0x00 */ CollVec v; /* a corner */
    /* 0x06 */ CollVec e1; /* and the two edges from it */
    /* 0x0C */ CollVec e2;
    /* 0x12 */ u8 pad12[2];
    /* 0x14 */ Vec3f n; /* the normal; x above 10: not made yet */
    /* 0x20 */ f32 d; /* the plane's offset */
    /* 0x24 */ u8 kind; /* the ground's kind */
    /* 0x25 */ u8 zone; /* the course the ground belongs to (0: none) */
    /* 0x26 */ u8 soundKind;
    /* 0x27 */ u8 unused27;
} CollTri;

void vramCreate(Obj *parent);
s32 vram_proc(Obj *obj, s16 msg, void *arg);
void vramCardinalPoint(Vec3f *out, Vec3f *p, f32 tension, f32 t);
void vramCardinalPointXZ(Vec3f *out, Vec3f *p, f32 tension, f32 t);
void vramSplinePoint(Vec3f *out, Spline *sp, f32 u);
void vramSplinePointXZ(Vec3f *out, Spline *sp, f32 u);
void vramCardinalDeriv(Vec3f *out, Vec3f *p, f32 tension, f32 t);
void vramCardinalDerivXZ(Vec3f *out, Vec3f *p, f32 tension, f32 t);
void vramSplineDeriv(Vec3f *out, Spline *sp, f32 u);
void vramSplineDerivXZ(Vec3f *out, Spline *sp, f32 u);
f32 vramSplineLengthXZ(Spline *sp);
void vramLoadCourses(s32 n);
void vramSetCourse(s32 n, s32 kind, Vec3f *points);
f32 vramGetCourseLength(s32 n);
f32 vramCoursePointXZ(s32 n, f32 dist, f32 off, f32 *x, f32 *z);
f32 vramCoursePoint(s32 n, f32 dist, f32 off, f32 *x, f32 *y, f32 *z);
f32 vramCourseHeading(s32 n, f32 dist);
s32 vramCourseDirection(s32 n, f32 dist, Vec3f *dir);
f32 vramNearestPlaceXZ(s32 n, f32 x, f32 z, f32 *side);
f32 vramNearestPlace(s32 n, f32 x, f32 y, f32 z, f32 *side);
void vramMakeTriNormal(CollTri *t);
s32 vramRayTriangle(CollTri *tri, Vec3f *from, Vec3f *dir, Vec3f *hit, f32 *tp);
CollTri *vramRayGround(Vec3f *from, Vec3f *dir, Vec3f *at, f32 *tp);
void vramLoadColl(u32 addr, s32 file);
void *vramCollToRam(u32 addr);
s32 vram_disp_proc(Obj *obj, s16 msg, void *arg);
void vramShowColl(void);
void vramHideColl(void);

#endif
