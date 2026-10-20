#ifndef MATHUTIL_H
#define MATHUTIL_H

#include "object.h"

f32 mathSinCos(u16 angle, f32 *c);
f32 mathSin(f32 rad);
f32 mathCos(f32 rad);
void mathMtxTransRotScale(f32 m[4][4], f32 tx, f32 ty, f32 tz, f32 rx, f32 ry, f32 rz, f32 sx, f32 sy, f32 sz);
void mathMtxFToL(Mtx *m, f32 mf[4][4]);
void mathMtxTransRotScaleL(Mtx *mtx, f32 tx, f32 ty, f32 tz, f32 rx, f32 ry, f32 rz, f32 sx, f32 sy, f32 sz);
void mathMtxLookTransScale(f32 m[4][4], f32 x, f32 y, f32 z, f32 tx, f32 ty, f32 tz, f32 sx, f32 sy, f32 sz);
void mathMtxLookTransScaleL(Mtx *mtx, f32 x, f32 y, f32 z, f32 tx, f32 ty, f32 tz, f32 sx, f32 sy, f32 sz);
void mathMtxTransRotScaleInv(f32 m[4][4], f32 tx, f32 ty, f32 tz, f32 rx, f32 ry, f32 rz, f32 sx, f32 sy, f32 sz);
void mathMtxRotInv(f32 m[4][4], f32 rx, f32 ry, f32 rz);
f32 fmtxInv(f32 m[4][4]);
void mathRotAltEuler(u16 *x, u16 *y, u16 *z);
void mathMtxIdent(f32 m[4][4]);
void mathMtxPreRotX(f32 m[4][4], f32 a);
void mathMtxPreRotY(f32 m[4][4], f32 a);
void mathMtxPreRotZ(f32 m[4][4], f32 a);
void mathMtxPostRotX(f32 m[4][4], f32 a);
void mathMtxPostRotY(f32 m[4][4], f32 a);
void mathMtxPostRotZ(f32 m[4][4], f32 a);
void mathMtxPreTrans(f32 m[4][4], f32 x, f32 y, f32 z);
void mathMtxPostTrans(f32 m[4][4], f32 x, f32 y, f32 z);
void mathMtxScaleRotX(f32 m[4][4], f32 s);
void mathMtxScaleRotY(f32 m[4][4], f32 s);
void mathMtxScaleRotZ(f32 m[4][4], f32 s);
void mathMtxScaleRot(f32 m[4][4], f32 s);
void mathMtxScaleX(f32 m[4][4], f32 s);
void mathMtxScaleY(f32 m[4][4], f32 s);
void mathMtxScaleZ(f32 m[4][4], f32 s);
void mathMtxScale(f32 m[4][4], f32 s);
void mathMtxLookDir(f32 m[4][4], f32 x, f32 y, f32 z);
void mathVecNormalize(Vec3f *v);
void mathVecNormalizeB(Vec3f *v);
void mathVecScale(Vec3f *v, f32 s);
f32 mathAtan(f32 x);
f32 mathAtan2(f32 y, f32 x);
void mathVecRotate(f32 rx, f32 ry, f32 rz, f32 *x, f32 *y, f32 *z);
void mathVecRotateInv(f32 rx, f32 ry, f32 rz, f32 *x, f32 *y, f32 *z);
void mathVecRotateY(f32 a, f32 *x, f32 *y, f32 *z);
void mathXZRotateY(f32 a, f32 x, f32 z, f32 *outX, f32 *outZ);
void mathCardinalSpline(Vec3f *out, Vec3f *p, f32 tension, f32 t);
void mathSplinePoint(Vec3f *out, Spline *path, f32 t);
f32 mathLuDecomp3(f32 a[3][3], s32 p[3]);
f32 mathMatInv3(f32 a[3][3], f32 inv[3][3]);
f32 mathLuDecomp4(f32 a[4][4], s32 p[4]);
f32 mathMatInv4(f32 a[4][4], f32 inv[4][4]);

#endif
