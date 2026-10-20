#include "object.h"
#include "fade.h"
#include "file.h"
#include "gfxbuf.h"
#include "gfxmgr.h"
#include "heap.h"
#include "mathutil.h"
#include "stage.h"
#include "tmanager.h"
#include "vram.h"

/* The "VRAM" task: a stage's courses (the rails the camera follows) and its ground collision.
 * - A course is a cardinal spline in x-z (with y for the 3D forms), numbered 1-11 from the
 *   stage data. Its length is measured by 8 steps a point. A place on it is a distance along
 *   it; a point at a place can be pushed sideways off it.
 * - The collision is a grid of cells over the stage, each with a list of triangles (a corner,
 *   two edges, and a normal made when first needed). A ray tests the triangles of the cells
 *   it crosses, each once, and keeps the nearest hit. A triangle's +0x25 is its course.
 * - "Vdisp1" draws the collision triangles as a model, coloured by their normals (debug). */

/* A course (0x10 bytes). */
typedef struct Rail {
    /* 0x0 */ Spline spline;
    /* 0xC */ f32 length;
} Rail;

/* The collision data, at the start of its file's data. */
typedef struct Coll {
    /* 0x00 */ f32 origin[3];
    /* 0x0C */ s16 dims[3]; /* cells */
    /* 0x12 */ u8 pad12[2];
    /* 0x14 */ f32 cell[3]; /* a cell's size */
    /* 0x20 */ s32 file;
    /* 0x24 */ s32 count; /* triangles */
    /* 0x28 */ u16 *grid; /* a list's start for each cell (0xFFFF: none) */
    /* 0x2C */ u16 *lists; /* triangle numbers, each list ending in 0 */
    /* 0x30 */ CollTri *tris;
} Coll;

/* The task's work. */
typedef struct CourseWork {
    /* 0x00 */ Coll *coll;
    /* 0x04 */ u8 *tested; /* one byte a triangle, cleared for each ray */
    /* 0x08 */ u8 pad8[4];
    /* 0x0C */ Rail rails[12]; /* 1-11 */
} CourseWork;

/* "Vdisp1"'s work. */
typedef struct VdispWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad4[4];
    /* 0x08 */ Coll *coll;
} VdispWork;

/* vram.c's .data (0x801094C0-0x80109508), in ROM order. */
Obj *gVramTask = NULL; /* the VRAM task */
u32 gVramCollFileRam = 0x00000000; /* the collision file's RAM */
Gfx gVdispSetupDl[] = {
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_CULL_BACK),
    gsSPSetGeometryMode(G_LIGHTING),
    gsDPSetCombineLERP(PRIMITIVE, 0, SHADE, 0, 0, 0, 0, 1, PRIMITIVE, 0, SHADE, 0, 0, 0, 0, 1),
    gsDPSetPrimColor(0, 0, 0xBF, 0x00, 0x00, 0xFF),
    gsSPEndDisplayList(),
};
Gfx gVdispTriDl[] = {
    gsSPVertex(0x00000000, 3, 0),
    gsSP1Triangle(2, 1, 0, 0),
};
extern Frame *gFrmModelLayer; /* the second frame pool */
extern s16 gFrmKindPol; /* the "pol" kind */

Coll *vramSetupColl(u32 addr, s32 file);

#define COURSE(obj) ((CourseWork *) ((u32) (obj) + sizeof(Obj)))

/* Makes the VRAM task under parent, once. */
void vramCreate(Obj *parent) {
    if (gVramTask == NULL) {
        gVramTask = tskCreateBlocks(parent, vram_proc, 2, 0, NULL);
    }
}

/* The handler. */
s32 vram_proc(Obj *obj, s16 msg, void *arg) {
    switch (msg) {
        case 0:
            break;
        case 2:
            obj->name = "VRAM";
            break;
        case 1:
            if (COURSE(obj)->tested != NULL) {
                heapFree(COURSE(obj)->tested);
            }
            gVramTask = NULL;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}


/* The spline evaluators. The weighted ones copy the tension into a local, keep three weights in
 * locals and write p[3]'s weight out in each line, its local left unused: the shape of
 * GoldenEye's coord3dCubicSplineInterp. The copy decides which of t and s gets f12 and keeps
 * the weights in registers; the unused local sets the frame and the spill slots. */
/* A point on the cardinal spline through p[0..3], between p[1] and p[2], at t (0-1). */
void vramCardinalPoint(Vec3f *out, Vec3f *p, f32 tension, f32 t) {
    f32 t2;
    f32 s;
    f32 t3;
    f32 h0;
    f32 h1;
    f32 h2;
    f32 h3; /* unused: p[3]'s weight is written out */

    t2 = t * t;
    s = tension;
    t3 = t2 * t;
    h0 = (2.0f * t2 - t3 - t) * s;
    h1 = (2.0f - s) * t3 + (s - 3.0f) * t2 + 1.0f;
    h2 = (s - 2.0f) * t3 + (3.0f - 2.0f * s) * t2 + s * t;
    out->x = p[0].x * h0 + p[1].x * h1 + p[2].x * h2 + p[3].x * ((t3 - t2) * tension);
    out->y = p[0].y * h0 + p[1].y * h1 + p[2].y * h2 + p[3].y * ((t3 - t2) * tension);
    out->z = p[0].z * h0 + p[1].z * h1 + p[2].z * h2 + p[3].z * ((t3 - t2) * tension);
}

/* The same in x-z (y is 0). */
void vramCardinalPointXZ(Vec3f *out, Vec3f *p, f32 tension, f32 t) {
    f32 t2;
    f32 s;
    f32 t3;
    f32 h0;
    f32 h1;
    f32 h2;
    f32 h3; /* unused: p[3]'s weight is written out */

    t2 = t * t;
    s = tension;
    t3 = t2 * t;
    h0 = (2.0f * t2 - t3 - t) * s;
    h1 = (2.0f - s) * t3 + (s - 3.0f) * t2 + 1.0f;
    h2 = (s - 2.0f) * t3 + (3.0f - 2.0f * s) * t2 + s * t;
    out->x = p[0].x * h0 + p[1].x * h1 + p[2].x * h2 + p[3].x * ((t3 - t2) * tension);
    out->y = 0.0f;
    out->z = p[0].z * h0 + p[1].z * h1 + p[2].z * h2 + p[3].z * ((t3 - t2) * tension);
}

/* A point along a spline at u (0-1): the segment u falls in, and the place within it. */
void vramSplinePoint(Vec3f *out, Spline *sp, f32 u) {
    s16 i;
    Vec3f *p;

    if (u < 0.0f || u > 1.0f) {
        return;
    }
    if (u < 1.0f) {
        u *= sp->n - 1;
        i = u;
        u -= i;
        vramCardinalPoint(out, &sp->points[i], sp->tension, u);
    } else {
        i = sp->n - 1;
        p = &sp->points[i + 1];
        *out = *p;
    }
}

/* The same in x-z. */
void vramSplinePointXZ(Vec3f *out, Spline *sp, f32 u) {
    s16 i;
    Vec3f *p;

    if (u < 0.0f || u > 1.0f) {
        return;
    }
    if (u < 1.0f) {
        u *= sp->n - 1;
        i = u;
        u -= i;
        vramCardinalPointXZ(out, &sp->points[i], sp->tension, u);
    } else {
        i = sp->n - 1;
        p = &sp->points[i + 1];
        *out = *p;
    }
}

/* The spline's direction (its derivative) through p[0..3] at t. */
void vramCardinalDeriv(Vec3f *out, Vec3f *p, f32 tension, f32 t) {
    f32 t2;
    f32 s;
    f32 d0;
    f32 d1;
    f32 d2;
    f32 d3; /* unused: p[3]'s weight is written out */

    t2 = t * t;
    s = tension;
    d0 = (-3.0f * t2 + 4.0f * t - 1.0f) * s;
    d1 = (2.0f - s) * 3.0f * t2 + 2.0f * (s - 3.0f) * t;
    d2 = (s - 2.0f) * 3.0f * t2 + 2.0f * (3.0f - 2.0f * s) * t + s;
    out->x = p[0].x * d0 + p[1].x * d1 + p[2].x * d2 + p[3].x * ((3.0f * t2 - 2.0f * t) * s);
    out->y = p[0].y * d0 + p[1].y * d1 + p[2].y * d2 + p[3].y * ((3.0f * t2 - 2.0f * t) * s);
    out->z = p[0].z * d0 + p[1].z * d1 + p[2].z * d2 + p[3].z * ((3.0f * t2 - 2.0f * t) * s);
}

/* The same in x-z. */
void vramCardinalDerivXZ(Vec3f *out, Vec3f *p, f32 tension, f32 t) {
    f32 t2;
    f32 s;
    f32 d0;
    f32 d1;
    f32 d2;
    f32 d3; /* unused: p[3]'s weight is written out */

    t2 = t * t;
    s = tension;
    d0 = (-3.0f * t2 + 4.0f * t - 1.0f) * s;
    d1 = (2.0f - s) * 3.0f * t2 + 2.0f * (s - 3.0f) * t;
    d2 = (s - 2.0f) * 3.0f * t2 + 2.0f * (3.0f - 2.0f * s) * t + s;
    out->x = p[0].x * d0 + p[1].x * d1 + p[2].x * d2 + p[3].x * ((3.0f * t2 - 2.0f * t) * s);
    out->y = 0.0f;
    out->z = p[0].z * d0 + p[1].z * d1 + p[2].z * d2 + p[3].z * ((3.0f * t2 - 2.0f * t) * s);
}

/* A spline's direction at u (0-1). */
void vramSplineDeriv(Vec3f *out, Spline *sp, f32 u) {
    s16 i;
    Vec3f *p;

    if (u < 0.0f || u > 1.0f) {
        return;
    }
    if (u < 1.0f) {
        u *= sp->n - 1;
        i = u;
        u -= i;
        vramCardinalDeriv(out, &sp->points[i], sp->tension, u);
    } else {
        i = sp->n - 1;
        p = &sp->points[i + 1];
        *out = *p;
    }
}

/* The same in x-z. */
void vramSplineDerivXZ(Vec3f *out, Spline *sp, f32 u) {
    s16 i;
    Vec3f *p;

    if (u < 0.0f || u > 1.0f) {
        return;
    }
    if (u < 1.0f) {
        u *= sp->n - 1;
        i = u;
        u -= i;
        vramCardinalDerivXZ(out, &sp->points[i], sp->tension, u);
    } else {
        i = sp->n - 1;
        p = &sp->points[i + 1];
        *out = *p;
    }
}

/* A spline's length in x-z, measured by 8 straight steps a point. */
f32 vramSplineLengthXZ(Spline *sp) {
    Vec3f a;
    Vec3f b;
    f32 len = 0.0f;
    s32 steps = sp->n * 8;
    s32 i;

    vramSplinePointXZ(&a, sp, 0.0f);
    for (i = 1; i < steps + 1; i++) {
        vramSplinePointXZ(&b, sp, (f32) i / (f32) steps);
        len += sqrtf((b.x - a.x) * (b.x - a.x) + (b.z - a.z) * (b.z - a.z));
        a = b;
    }
    return len;
}

/* Sets up stage n's courses (1-11) from its stage data. It reads up to 11 entries, one past
 * the stage data's 10, unless one of them ends the list. */
void vramLoadCourses(s32 n) {
    StageData d;
    s32 i;
    s32 file;
    s32 kind;

    stageReadData(&d, n);
    file = d.file;
#ifndef AVOID_UB
    //! @bug reads d.rails[10], past the stage data's 10 entries (vramSetCourse refuses course 11).
    for (i = 0; i < 11; i++) {
#else
    for (i = 0; i < 10; i++) {
#endif
        kind = d.rails[i].n;
        if (kind == 0) {
            break;
        }
        vramSetCourse(i + 1, kind,
                               (Vec3f *) fadeSegToRam(d.rails[i].points, file));
    }
}

/* Sets course n (a spline of `kind` points, tension 0.5) and measures it. Course 11 is
 * refused, though vramLoadCourses offers it. */
void vramSetCourse(s32 n, s32 kind, Vec3f *points) {
    CourseWork *w = COURSE(gVramTask);

    if (gVramTask != NULL && (u32) n < 11 && n >= 0) {
        w->rails[n].spline.kind = 1;
        w->rails[n].spline.n = kind;
        w->rails[n].spline.points = points;
        w->rails[n].spline.tension = 0.5f;
        w->rails[n].length = vramSplineLengthXZ(&w->rails[n].spline);
    }
}

/* Course n's length (0: no such course). Its course error is the first of the file's seven
 * in .rodata (until the fakes pass of 2026-10-03 a FAKE print in vramCourseDirection stood for it;
 * the seven are the same bytes and no code reads them, so only their count shows). */
f32 vramGetCourseLength(s32 n) {
    CourseWork *w = COURSE(gVramTask);

    if (gVramTask == NULL || n <= 0 || n >= 12 || w->rails[n].spline.n == 0) {
        PRINTF("Error: セットされていないコースナンバーが指定されました courseNo=%d \n", n);
        return 0.0f;
    }
    return w->rails[n].length;
}

/* The x-z point at place `dist` along course n, pushed `off` to its side. Returns the place,
 * kept within the course. With no course, the point is (dist, off). */
f32 vramCoursePointXZ(s32 n, f32 dist, f32 off, f32 *x, f32 *z) {
    CourseWork *w = COURSE(gVramTask);
    Vec3f v;
    f32 nx;
    f32 len;
    f32 nz;

    if (n == 0 || gVramTask == NULL) {
        *x = dist;
        *z = off;
        return dist;
    }
    if (n < 0 || n >= 12 || w->rails[n].spline.n == 0) {
        PRINTF("Error: セットされていないコースナンバーが指定されました courseNo=%d \n", n);
        *x = dist;
        *z = off;
        return dist;
    }
    if (dist < 0.0f) {
        dist = 0.0f;
    } else if (dist > w->rails[n].length) {
        dist = w->rails[n].length;
    }
    vramSplinePointXZ(&v, &w->rails[n].spline, dist / w->rails[n].length);
    *x = v.x;
    *z = v.z;
    vramSplineDerivXZ(&v, &w->rails[n].spline, dist / w->rails[n].length);
    nx = -v.z;
    nz = v.x;
    len = sqrtf(nx * nx + nz * nz);
    *x += nx * off / len;
    *z += nz * off / len;
    return dist;
}

/* The same in 3D. */
f32 vramCoursePoint(s32 n, f32 dist, f32 off, f32 *x, f32 *y, f32 *z) {
    CourseWork *w = COURSE(gVramTask);
    Vec3f v;
    f32 nx;
    f32 len;
    f32 nz;

    if (n == 0 || gVramTask == NULL) {
        *x = dist;
        *y = 0.0f;
        *z = off;
        return dist;
    }
    if (n < 0 || n >= 12 || w->rails[n].spline.n == 0) {
        PRINTF("Error: セットされていないコースナンバーが指定されました courseNo=%d \n", n);
        *x = dist;
        *y = 0.0f;
        *z = off;
        return dist;
    }
    if (dist < 0.0f) {
        dist = 0.0f;
    } else if (dist > w->rails[n].length) {
        dist = w->rails[n].length;
    }
    vramSplinePoint(&v, &w->rails[n].spline, dist / w->rails[n].length);
    *x = v.x;
    *y = v.y;
    *z = v.z;
    vramSplineDeriv(&v, &w->rails[n].spline, dist / w->rails[n].length);
    nx = -v.z;
    nz = v.x;
    len = sqrtf(nx * nx + nz * nz);
    *x += nx * off / len;
    *z += nz * off / len;
    return dist;
}

/* Course n's heading at place `dist` (0: no course). */
f32 vramCourseHeading(s32 n, f32 dist) {
    CourseWork *w = COURSE(gVramTask);
    Vec3f d;

    if (n == 0 || gVramTask == NULL) {
        return 0.0f;
    }
    if (n < 0 || n >= 12 || w->rails[n].spline.n == 0) {
        PRINTF("Error: セットされていないコースナンバーが指定されました courseNo=%d \n", n);
        return 0.0f;
    }
    if (dist < 0.0f) {
        dist = 0.0f;
    } else if (dist > w->rails[n].length) {
        dist = w->rails[n].length;
    }
    vramSplineDerivXZ(&d, &w->rails[n].spline, dist / w->rails[n].length);
    return mathAtan2(d.x, d.z);
}

/* Course n's direction at place `dist` (no course: (1, 0, 0), and 0). */
s32 vramCourseDirection(s32 n, f32 dist, Vec3f *dir) {
    CourseWork *w = COURSE(gVramTask);

    if (n == 0 || gVramTask == NULL) {
        dir->z = 0.0f;
        dir->y = 0.0f;
        dir->x = 1.0f;
        return 0;
    }
    if (n < 0 || n >= 12 || w->rails[n].spline.n == 0) {
        PRINTF("Error: セットされていないコースナンバーが指定されました courseNo=%d \n", n);
        dir->z = 0.0f;
        dir->y = 0.0f;
        dir->x = 1.0f;
        return 0;
    }
    if (dist < 0.0f) {
        dist = 0.0f;
    } else if (dist > w->rails[n].length) {
        dist = w->rails[n].length;
    }
    vramSplineDeriv(dir, &w->rails[n].spline, dist / w->rails[n].length);
    return 1;
}

/* The nearest place on a course.
 * Matched 2026-10-04; it was parked for register colouring: uopt ends a basic block after about
 * 20 variable loads, and the far point's join used to fall after t (two more loads than the
 * committed join after s), which split ex/ez/nx's live ranges and let the far-point temps
 * outrank them. Reading the far points through "i = best;" (p[i + 1], p[i]) keeps the join one
 * block through s, giving every register and the frame the game's, with no FAKE. */
/* The place on course n nearest (x, z), found through its nearest points, and the side
 * distance in *side. With no course, x is the place and z the side. */
f32 vramNearestPlaceXZ(s32 n, f32 x, f32 z, f32 *side) {
    CourseWork *w = COURSE(gVramTask);
    Vec3f *p;
    s32 cnt;
    s32 best;
    s32 i;
    f32 bestd;
    f32 d;
    f32 ex;
    f32 ez;
    f32 inv;
    f32 nx;
    f32 nz;
    f32 pad0;
    f32 t;
    f32 s;
    s32 pad_x; /* an even count of locals, no alignment word */

    if (n == 0 || gVramTask == NULL) {
        *side = z;
        return x;
    }
    if (n < 0 || n >= 12 || w->rails[n].spline.n == 0) {
        PRINTF("Error: セットされていないコースナンバーが指定されました courseNo=%d \n", n);
        *side = z;
        return x;
    }
    cnt = w->rails[n].spline.n;
    p = w->rails[n].spline.points;
    best = 1;
    bestd = (p[1].x - x) * (p[1].x - x) + (p[1].z - z) * (p[1].z - z);
    for (i = 1; i < cnt; i++) {
        d = (p[i + 1].x - x) * (p[i + 1].x - x) + (p[i + 1].z - z) * (p[i + 1].z - z);
        if (d < bestd) {
            bestd = d;
            best = i + 1;
        }
    }
    if (best == 1) {
    } else if (best == cnt) {
        best--;
    } else if ((p[best - 1].x - x) * (p[best - 1].x - x) + (p[best - 1].z - z) * (p[best - 1].z - z) <
               (p[best + 1].x - x) * (p[best + 1].x - x) + (p[best + 1].z - z) * (p[best + 1].z - z)) {
        best--;
    }
    i = best;
    ex = p[i + 1].x - p[i].x;
    ez = p[i + 1].z - p[i].z;
    nx = -ex;
    inv = 1.0f / sqrtf(ez * ez + nx * nx);
    nx *= inv;
    nz = ez * inv;
    inv = 1.0f / (ez * nz - ex * nx);
    t = (nx * (p[i].x - x) - (p[i].z - z) * nz) * inv;
    s = (ez * (p[i].x - x) - (p[i].z - z) * ex) * inv;
    *side = s;
    t = ((f32) best + t - 1.0f) / (f32) (cnt - 1);
    return w->rails[n].length * t;
}

/* Matched 2026-10-04; it was parked for registers: only bx's home differed (0x44 here for the
 * game's 0x34, a uopt temp slot no declaration reaches, assigned before "best--" in the true
 * branch as a variable is). Writing the third case's near-point distance z term first, as the
 * twin vramNearestPlaceXZ does, gave it; the far point is no variable either (the twin's PRE
 * chain, the join reading the points through "i = best;"), with no FAKE. */
/* The same with y: the nearest point in 3D, the side distance in x-z. */
f32 vramNearestPlace(s32 n, f32 x, f32 y, f32 z, f32 *side) {
    CourseWork *w = COURSE(gVramTask);
    Vec3f *p;
    s32 cnt;
    s32 best;
    s32 i;
    f32 bestd;
    f32 d;
    f32 ex;
    f32 ez;
    f32 inv;
    f32 nx;
    f32 nz;
    f32 pad0;
    f32 t;
    f32 s;
    s32 pad_x;

    if (n == 0 || gVramTask == NULL) {
        *side = z;
        return x;
    }
    if (n < 0 || n >= 12 || w->rails[n].spline.n == 0) {
        PRINTF("Error: セットされていないコースナンバーが指定されました courseNo=%d \n", n);
        *side = z;
        return x;
    }
    cnt = w->rails[n].spline.n;
    p = w->rails[n].spline.points;
    best = 1;
    bestd = (p[1].x - x) * (p[1].x - x) + (p[1].y - y) * (p[1].y - y) +
            (p[1].z - z) * (p[1].z - z);
    for (i = 1; i < cnt; i++) {
        d = (p[i + 1].x - x) * (p[i + 1].x - x) + (p[i + 1].y - y) * (p[i + 1].y - y) +
            (p[i + 1].z - z) * (p[i + 1].z - z);
        if (d < bestd) {
            bestd = d;
            best = i + 1;
        }
    }
    if (best == 1) {
    } else if (best == cnt) {
        best--;
    } else if ((p[best - 1].x - x) * (p[best - 1].x - x) + (p[best - 1].z - z) * (p[best - 1].z - z) <
               (p[best + 1].x - x) * (p[best + 1].x - x) + (p[best + 1].z - z) * (p[best + 1].z - z)) {
        best--;
    }
    i = best;
    ex = p[i + 1].x - p[i].x;
    ez = p[i + 1].z - p[i].z;
    nx = -ex;
    inv = 1.0f / sqrtf(ez * ez + nx * nx);
    nx *= inv;
    nz = ez * inv;
    inv = 1.0f / (ez * nz - ex * nx);
    t = (nx * (p[i].x - x) - (p[i].z - z) * nz) * inv;
    s = (ez * (p[i].x - x) - (p[i].z - z) * ex) * inv;
    *side = s;
    t = ((f32) best + t - 1.0f) / (f32) (cnt - 1);
    return w->rails[n].length * t;
}

/* Makes a triangle's normal (edge 2 x edge 1, normalised) and its plane's offset. Each product
 * in the order the game multiplies it (a field keeps the source's order: IDO_NOTES 198). */
void vramMakeTriNormal(CollTri *t) {
    t->n.x = t->e2.y * t->e1.z - t->e2.z * t->e1.y;
    t->n.y = t->e2.z * t->e1.x - t->e2.x * t->e1.z;
    t->n.z = t->e2.x * t->e1.y - t->e2.y * t->e1.x;
    guNormalize(&t->n.x, &t->n.y, &t->n.z);
    t->d = t->n.x * t->v.x;
    t->d += t->n.y * t->v.y;
    t->d += t->n.z * t->v.z;
}

/* A ray (from, from + dir) against a triangle: 0 a hit (the point in *hit, the place along
 * the ray in *tp), 1 parallel, 2 the plane is off the ray, 3 the point is outside. Once
 * stored, t is reused for the second of the point's two edge coordinates (as the game's
 * registers show: one f2 for both). */
s32 vramRayTriangle(CollTri *tri, Vec3f *from, Vec3f *dir, Vec3f *hit, f32 *tp) {
    f32 dot;
    f32 t;
    f32 px;
    f32 py;
    f32 pz;
    f32 u;
    f32 inv;
    s32 det;

    if (tri->n.x > 10.0f) {
        vramMakeTriNormal(tri);
    }
    dot = tri->n.x * dir->x + tri->n.y * dir->y + tri->n.z * dir->z;
    if (fabsf(dot) < 0.0001f) {
        return 1;
    }
    t = (tri->d - tri->n.x * from->x - tri->n.y * from->y - tri->n.z * from->z) / dot;
    *tp = t;
    if (t < 0.0f || t > 1.0f) {
        return 2;
    }
    hit->x = dir->x * t + from->x;
    hit->y = dir->y * t + from->y;
    hit->z = dir->z * t + from->z;
    px = hit->x - tri->v.x;
    py = hit->y - tri->v.y;
    pz = hit->z - tri->v.z;
    if (px * px + py * py + pz * pz < 9.999999e-09f) { /* 0x322BCC76, one ulp under 1e-8f */
        return 0;
    }
    if ((det = tri->e1.x * tri->e2.z - tri->e2.x * tri->e1.z) != 0) {
        inv = 1.0f / det;
        u = (tri->e2.z * px - tri->e2.x * pz) * inv;
        t = (-tri->e1.z * px + tri->e1.x * pz) * inv;
    } else if ((det = tri->e1.z * tri->e2.y - tri->e2.z * tri->e1.y) != 0) {
        inv = 1.0f / det;
        u = (tri->e2.y * pz - tri->e2.z * py) * inv;
        t = (-tri->e1.y * pz + tri->e1.z * py) * inv;
    } else if ((det = tri->e1.x * tri->e2.y - tri->e2.x * tri->e1.y) != 0) {
        inv = 1.0f / det;
        u = (tri->e2.y * px - tri->e2.x * py) * inv;
        t = (-tri->e1.y * px + tri->e1.x * py) * inv;
    } else {
        return 3;
    }
    if (u < 0.0f || t < 0.0f || u + t > 1.0f) {
        return 3;
    }
    return 0;
}

/* Matched 2026-10-02: "if (list == 0xFFFF) continue;" gives the game's loops (floorhit.c
 * fhSweepSphere, the sphere twin, matched with it), a FAKE block boundary after the
 * initialisers gives from/dir/at their saved registers (the splits sweep: 305 to 95; without
 * it best takes s8 and at goes to its home), and best is set before the copy into at (the game
 * starts its multiply first). The initialisers as statements: 301; bestT set after the early
 * return: 307.
 * Fakes pass (2026-10-03), the boundary kept. uopt's trace (-zdbug:5)
 * shows what it does: best and at are tied for the last saved register (best 909.27, its
 * loop store and two outer references; at 909.09, its one loop read); the boundary adds two
 * head blocks to both ranges and only best's priority drops (833.5), so at takes fp and best
 * a temp saved around the calls, as the game. Natural forms measured without it, none a
 * match: "return best;" on the early path 301 (best is a constant there); two early-return
 * ifs 307; the sphere twin's "(c = w->coll) == NULL" and "c = w->coll;" after the clear
 * (c in a pad word) 337. A dead PRINTF would be the natural boundary, but every string of
 * the file is taken.
 * Fakes pass round 2 (2026-10-04), the boundary kept. The arithmetic, from uopt (f_compute_save
 * in the recompiled uopt.c): a range scores its references, weighted 10 per loop level, over
 * (N - 2) / 4 + 2, N its blocks. best has 10002 (the loop store 10000, the initialiser and the
 * return 1 each) and at 10000 (its one loop read); best spans two blocks more than at (the
 * loop exit and the return). With no boundary N is 41 and 39, both divisors 11, and best wins
 * by 2 / 11; the boundary adds two blocks to both (43 and 41, divisors 12 and 11). So the
 * original had one or two more basic blocks than this C where both live, with no code, or one
 * or more after the loops. Measured (tr.sh prints the
 * ranges, cmp.sh compares cc -S with the committed form): one "if (0) {}" in place of the
 * do-while: check MATCH; one after the loops, before the store to tp: check MATCH; one after
 * the clear, after z1, after the swaps or at the top of the list loop: cc -S the same; an
 * empty "if (best != NULL) {}" (or "== NULL") after the loops: cc -S the same (two blocks for
 * best alone). Three or four blocks at the head fail (k becomes a register range and takes
 * s6; at four best and at tie on 12). One right after the early return moves a temp. A dead
 * PRINTF at the head gives the same code, but its string would be an eighth in .rodata.
 * Natural forms with no extra block, none of which changes any N (cc -S 239 to 280 lines off):
 * the cell test as nested ifs, or negated with continue (that one adds two blocks but in the
 * loop, where k and the corners then lose their registers); the hit test nested or as one &&
 * chain; whiles for the grid loops or the list loop; the early test negated, with !, with the
 * rest in an else, or with w assigned inside it; the sphere twin form with a Coll pointer c
 * (36 forms: c set in the test, used for the dims, the clear or the cell maths, reloaded after
 * the clear or not, w and best as statements); the swaps as a do-while(0) or with a block
 * temp (a block temp frees k, which then takes s6, though three of them give the 0xB8 frame
 * with no pad). What the original must have had: a compiled-out statement with no string and
 * no code, most likely an if on the result after the loops whose body (debug output) was
 * removed by the preprocessor, or an empty debug macro (a do-while(0) or if (0)) at the head;
 * every string of .rodata already has its print. */
/* A ray (from, from + dir) against the ground: the nearest triangle hit (or NULL), the point
 * in *at and the place along the ray in *tp (1.1: none). k is also the swaps' temporary. The
 * pad stands for 8 bytes of locals the original has below the others. */
CollTri *vramRayGround(Vec3f *from, Vec3f *dir, Vec3f *at, f32 *tp) {
    CourseWork *w = COURSE(gVramTask);
    s16 x0;
    s16 y0;
    s16 z0;
    s16 x1;
    s16 y1;
    s16 z1;
    s16 i;
    s16 j;
    s16 k;
    s16 dimX;
    s16 dimY;
    s16 dimZ;
    CollTri *best = NULL;
    Vec3f hit;
    f32 t;
    f32 bestT = 1.1f;
    u16 *l;
    u16 list;
    s32 pad[2];

    if (1) {} /* FAKE: OoT's `if (1) {}`, a block boundary after the initialisers (IDO_NOTES 228) */
    if (gVramTask == NULL || w->coll == NULL) {
        return NULL;
    }
    dimX = w->coll->dims[0];
    dimY = w->coll->dims[1];
    dimZ = w->coll->dims[2];
    heapMemClear(w->tested, w->coll->count);
    x0 = (from->x - w->coll->origin[0]) / w->coll->cell[0];
    y0 = (from->y - w->coll->origin[1]) / w->coll->cell[1];
    z0 = (from->z - w->coll->origin[2]) / w->coll->cell[2];
    x1 = (from->x + dir->x - w->coll->origin[0]) / w->coll->cell[0];
    y1 = (from->y + dir->y - w->coll->origin[1]) / w->coll->cell[1];
    z1 = (from->z + dir->z - w->coll->origin[2]) / w->coll->cell[2];
    if (x1 < x0) {
        k = x0; x0 = x1; x1 = k;
    }
    if (y1 < y0) {
        k = y0; y0 = y1; y1 = k;
    }
    if (z1 < z0) {
        k = z0; z0 = z1; z1 = k;
    }
    for (i = x0; i <= x1; i++) {
        for (j = y0; j <= y1; j++) {
            for (k = z0; k <= z1; k++) {
                if (i >= 0 && i < dimX && j >= 0 && j < dimY && k >= 0 && k < dimZ) {
                    list = w->coll->grid[i * dimY * dimZ + j * dimZ + k];
                    if (list == 0xFFFF) {
                        continue;
                    }
                    for (l = &w->coll->lists[list]; *l != 0; l++) {
                        if (w->tested[*l] == 0) {
                            if (vramRayTriangle(&w->coll->tris[*l], from, dir, &hit, &t) == 0 &&
                                t < bestT) {
                                best = &w->coll->tris[*l];
                                *at = hit;
                                bestT = t;
                            }
                        }
                        w->tested[*l] = 1;
                    }
                }
            }
        }
    }
    *tp = bestT;
    return best;
}

/* Loads the stage's collision (at addr in file) and its tested flags. */
void vramLoadColl(u32 addr, s32 file) {
    CourseWork *w = COURSE(gVramTask);

    if (gVramTask != NULL) {
        w->coll = vramSetupColl(addr, file);
        if (w->coll->count == 0) {
            PRINTF("Error: 当たり三角形の個数が０個になっています \n");
        } else {
            w->tested = heapMalloc(w->coll->count);
            if (w->tested == NULL) {
                PRINTF("Error: メモリが足りません in vram_proc !!\n");
            }
        }
    }
}

/* A segmented address in the collision file, in RAM (0: NULL). */
void *vramCollToRam(u32 addr) {
    if (addr != 0) {
        return (void *) ((addr & 0xFFFFFF) + gVramCollFileRam);
    }
    return NULL;
}

/* The collision at addr in file, its tables made RAM addresses and its normals marked unmade. */
Coll *vramSetupColl(u32 addr, s32 file) {
    Coll *c;
    s32 i;

    gVramCollFileRam = fileRamAddr(file);
    c = vramCollToRam(addr);
    c->grid = vramCollToRam((u32) c->grid);
    c->lists = vramCollToRam((u32) c->lists);
    c->tris = vramCollToRam((u32) c->tris);
    c->file = file;
    for (i = 0; i < c->count; i++) {
        c->tris[i].n.x = 100.0f;
    }
    return c;
}

/* "Vdisp1": the collision triangles as a "pol" model, coloured by their normals (as vertex
 * normals). Its list is one gSPVertex and one triangle for each, the vertices after it. */
s32 vram_disp_proc(Obj *obj, s16 msg, void *arg) {
    VdispWork *w = (VdispWork *) ((u32) obj + sizeof(Obj));
    Gfx *g;
    Vtx *v;
    CollTri *t;
    s32 i;
    s32 j;

    switch (msg) {
        case 0:
            break;
        case 2:
            obj->name = "Vdisp1";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            w->coll = arg;
            w->frame->dl = gVdispSetupDl;
            w->frame->dl2 = heapMalloc(w->coll->count * 64 + 8);
            if (w->frame->dl2 == NULL) {
                PRINTF("vram_disp_proc : モデル用のメモリが確保出来ませんでした\n");
                return -1;
            }
            g = w->frame->dl2;
            v = (Vtx *) (w->frame->dl2 + w->coll->count * 2 + 1);
            for (i = 0; i < w->coll->count; i++) {
                gSPVertex(g++, &v[i * 3], 3, 0);
                gSP1Triangle(g++, 2, 1, 0, 0);
            }
            gSPEndDisplayList(g++);
            for (i = 0; i < w->coll->count; i++) {
                t = &w->coll->tris[i];
                v[i * 3 + 0].n.ob[0] = t->v.x;
                v[i * 3 + 0].n.ob[1] = t->v.y;
                v[i * 3 + 0].n.ob[2] = t->v.z;
                v[i * 3 + 1].n.ob[0] = t->e1.x + t->v.x;
                v[i * 3 + 1].n.ob[1] = t->e1.y + t->v.y;
                v[i * 3 + 1].n.ob[2] = t->e1.z + t->v.z;
                v[i * 3 + 2].n.ob[0] = t->e2.x + t->v.x;
                v[i * 3 + 2].n.ob[1] = t->e2.y + t->v.y;
                v[i * 3 + 2].n.ob[2] = t->e2.z + t->v.z;
                for (j = 0; j < 3; j++) {
                    v[i * 3 + j].n.tc[0] = 0;
                    v[i * 3 + j].n.tc[1] = 0;
                    v[i * 3 + j].n.n[0] = t->n.x * 127.0f;
                    v[i * 3 + j].n.n[1] = t->n.y * 127.0f;
                    v[i * 3 + j].n.n[2] = t->n.z * 127.0f;
                    v[i * 3 + j].n.a = 0xFF;
                }
            }
            break;
        case 1:
            if (w->frame != NULL) {
                while (gfxGetPendingSwaps() != 0) {
                }
                if (w->frame->dl2 != NULL) {
                    heapFree(w->frame->dl2);
                }
                frmDestroy((Frame *) w->frame);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Shows the collision (debug): a "Vdisp1" task under VRAM. */
void vramShowColl(void) {
    CourseWork *w = COURSE(gVramTask);

    if (gVramTask != NULL && w->coll != NULL) {
        tskCreate(gVramTask, vram_disp_proc, 0, w->coll);
    }
}

/* Hides it: deletes VRAM's child tasks. */
void vramHideColl(void) {
    if (gVramTask != NULL) {
        tskDestroyChildren(gVramTask);
    }
}
