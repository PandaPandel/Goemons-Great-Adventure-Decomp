#include "object.h"
#include "fileload.h"
#include "gfxmgr.h"
#include "mathutil.h"
#include "shadow.h"
#include "vram.h"

/* Drop shadows: an "mpol" frame (polygon.c) drawing a round spot (file 0x120) flat on the
 * ground under a point, scaled down the higher the point is. */

extern Frame *gFrmLayer4; /* the frame shadows go under (gFrmLayers[4]) */
extern s16 gFrmKindMpol; /* "mpol" */
/* shadow.c's .data (0x8010C6A0-0x8010C6B0), in ROM order. */
Gfx gShadowSetupDl[] = {
    gsDPSetPrimColor(0, 0, 0x00, 0x00, 0x00, 0xC0),
    gsSPEndDisplayList(),
};
extern Gfx shadowSpotDl[]; /* the spot, in file 0x120 */

/* A shadow. */
MPolFrame *shadowCreate(void) {
    MPolFrame *f = (MPolFrame *) frmCreate(gFrmLayer4, gFrmKindMpol);

    if (f != NULL) {
        f->dl = gShadowSetupDl;
        f->dl2 = shadowSpotDl;
        floadGetSetRam((void **) &f->segs[0], 0x120);
    }
    return f;
}

/* Puts a shadow of a size under (x, y, z): on the floor given, or on the ground found 20
 * sizes below (smaller the farther down), or hides it if there is none. size is scaled in
 * place, "size *= 0.4f" in both branches (the original reloads it from its home, multiplies
 * into f2 and stores it back). from.z is set before dir.y: the float temps rotate f6, f8, f10,
 * f18, f4, so z's load takes f10 ahead of the ray's product. */
void shadowPut(MPolFrame *f, f32 x, f32 y, f32 z, f32 size, CollTri *floor) {
    Vec3f from;
    Vec3f dir;
    Vec3f at;
    f32 t;

    if (f != NULL) {
        if (floor != NULL) {
            shadowMtxOnSurface(f->mtx.m, -floor->n.x, -floor->n.y, -floor->n.z);
            mathMtxPostTrans(f->mtx.m, x, y + 0.1, z);
            size *= 0.4f;
            mathMtxScaleRot(f->mtx.m, size);
        } else {
            from.y = y + 1.0f;
            from.x = x;
            from.z = z;
            dir.y = -fabsf(size) * 20.0f;
            dir.x = 0.0f;
            dir.z = 0.0f;
            floor = vramRayGround(&from, &dir, &at, &t);
            if (floor == NULL) {
                f->f.flags &= ~2;
                return;
            }
            shadowMtxOnSurface(f->mtx.m, -floor->n.x, -floor->n.y, -floor->n.z);
            mathMtxPostTrans(f->mtx.m, at.x, at.y + 0.1, at.z);
            size *= 0.4f;
            mathMtxScaleRot(f->mtx.m, (1.0f - t) * size);
        }
        f->f.flags |= 2;
    }
}

/* A matrix lying on a surface whose normal is (x, y, z). */
void shadowMtxOnSurface(f32 m[4][4], f32 x, f32 y, f32 z) {
    mathMtxLookDir(m, x, -z, y);
    mathMtxPostRotX(m, 1.5707964f);
}
