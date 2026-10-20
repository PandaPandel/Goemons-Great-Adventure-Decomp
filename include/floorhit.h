#ifndef FLOORHIT_H
#define FLOORHIT_H

#include "object.h"
#include "vram.h"

/* A sphere, around a point off the query's (a list of them ends with a 0 radius). */
typedef struct PlSphere {
    /* 0x00 */ f32 r;
    /* 0x04 */ Vec3f off;
} PlSphere;

/* A player's spheres for the moving floors (pl3d.c, plmove.c), copied by value into a query. */
typedef struct PlFloorBuf {
    /* 0x00 */ PlSphere s[3];
} PlFloorBuf;

/* The moving floors' query (fhSweepSpheres). */
typedef struct PlFloor {
    /* 0x00 */ Vec3f from;
    /* 0x0C */ Vec3f dir;
    /* 0x18 */ PlSphere *buf;
    /* 0x1C */ Vec3f push;
    /* 0x28 */ CollTri *tri;
} PlFloor;

CollTri *fhSweepSpheres(PlFloor *q);
CollTri *fhSweepSphere(Vec3f *pos, f32 r, Vec3f *dir, Vec3f *out);
s32 fhSphereVsTri(CollTri *t, Vec3f *pos, Vec3f *dir, f32 r, Vec3f *to, Vec3f *push);
f32 _dist_point_to_line(f32 px, f32 py, f32 pz, f32 dx, f32 dy, f32 dz, f32 ox, f32 oy, f32 oz, Vec3f *out);

#endif
