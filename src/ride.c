#include "object.h"
#include "camlight.h"
#include "gfxbuf.h"
#include "gfxmgr.h"
#include "mathutil.h"
#include "ride.h"
#include "tmanager.h"

/* The "Ride" task: boxes that move with a model and can be stood on or pushed against.
 * - A "ride" frame hangs off a model's frame ("pol", "anime" or "mpol") and carries boxes,
 *   each with a half-size, a place and a rotation in the model, and an inverse matrix.
 * - Each step (rideUpdateTree) every ride frame takes its model's place and a reach (its
 *   radius times the model's largest scale), and makes its inverse matrix.
 * - A point inside a box is pushed out through the nearest open face (rideBoxPushOut), 0.1
 *   beyond it. A moving point is stopped at the first face it crosses (rideBoxStop), or
 *   placed on it (rideBoxOnto). Faces flagged 1, 2 or 4 (x, y or z) are left open.
 * - The results come back turned by the model's rotation, and a ride's id tells the caller
 *   what it met. Two trees of ride frames (gRideTree1, gRideTree2) hold them. */



/* The nearest hit of a moving point. */
typedef struct RideHit {
    /* 0x00 */ f32 t; /* 2: none */
    /* 0x04 */ Vec3f move;
    /* 0x10 */ RideFrame *frame;
} RideHit;

/* The model frames a ride can follow (polygon.c). */
typedef struct RPolFrame {
    /* 0x00 */ Frame f;
    /* 0x14 */ f32 pos[3];
    /* 0x20 */ f32 rot[3];
    /* 0x2C */ f32 scale[3];
} RPolFrame;

typedef struct RMPolFrame {
    /* 0x00 */ Frame f;
    /* 0x14 */ MtxF mtx;
    /* 0x54 */ MtxF world;
} RMPolFrame;

typedef s32 (*RideTest)(RideBox *b, Vec3f *from, Vec3f *to, Vec3f *move, f32 *t);

/* ride.c's .data (0x80109510-0x80109788), in ROM order. */
Frame *gRideTree1 = NULL; /* the two trees of ride frames */
Frame *gRideTree2 = NULL;
s16 gFrmKindRide = 0; /* the "ride" kind */
Obj *gRideTask = NULL; /* the Ride task */
RideData gRideDefaultData = { /* the boxes when a model gives none */
    1, 86.60254f, { 0x00, 0x00, 0x00, 0x00 },
    {
        {
            0x0000, { 50.0f, 50.0f, 50.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f },
            {
                {
                    { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f },
                    { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f },
                },
            },
        },
    },
};
Vtx gRideBoxVtx[24] = { /* the corners of the box, by face (named only by gRideBoxDl) */
    { { { -50, -50, -50 }, 0x0000, { 0, 0 }, { 0x81, 0x00, 0x00, 0xFF } } },
    { { { -50, 50, 50 }, 0x0000, { 0, 0 }, { 0x81, 0x00, 0x00, 0xFF } } },
    { { { -50, -50, 50 }, 0x0000, { 0, 0 }, { 0x81, 0x00, 0x00, 0xFF } } },
    { { { -50, 50, -50 }, 0x0000, { 0, 0 }, { 0x81, 0x00, 0x00, 0xFF } } },
    { { { -50, -50, 50 }, 0x0000, { 0, 0 }, { 0x00, 0x00, 0x7F, 0xFF } } },
    { { { 50, 50, 50 }, 0x0000, { 0, 0 }, { 0x00, 0x00, 0x7F, 0xFF } } },
    { { { 50, -50, 50 }, 0x0000, { 0, 0 }, { 0x00, 0x00, 0x7F, 0xFF } } },
    { { { -50, 50, 50 }, 0x0000, { 0, 0 }, { 0x00, 0x00, 0x7F, 0xFF } } },
    { { { 50, -50, 50 }, 0x0000, { 0, 0 }, { 0x7F, 0x00, 0x00, 0xFF } } },
    { { { 50, 50, -50 }, 0x0000, { 0, 0 }, { 0x7F, 0x00, 0x00, 0xFF } } },
    { { { 50, -50, -50 }, 0x0000, { 0, 0 }, { 0x7F, 0x00, 0x00, 0xFF } } },
    { { { 50, 50, 50 }, 0x0000, { 0, 0 }, { 0x7F, 0x00, 0x00, 0xFF } } },
    { { { 50, -50, -50 }, 0x0000, { 0, 0 }, { 0x00, 0x00, 0x81, 0xFF } } },
    { { { -50, 50, -50 }, 0x0000, { 0, 0 }, { 0x00, 0x00, 0x81, 0xFF } } },
    { { { -50, -50, -50 }, 0x0000, { 0, 0 }, { 0x00, 0x00, 0x81, 0xFF } } },
    { { { 50, 50, -50 }, 0x0000, { 0, 0 }, { 0x00, 0x00, 0x81, 0xFF } } },
    { { { 50, -50, -50 }, 0x0000, { 0, 0 }, { 0x00, 0x81, 0x00, 0xFF } } },
    { { { -50, -50, 50 }, 0x0000, { 0, 0 }, { 0x00, 0x81, 0x00, 0xFF } } },
    { { { 50, -50, 50 }, 0x0000, { 0, 0 }, { 0x00, 0x81, 0x00, 0xFF } } },
    { { { -50, -50, -50 }, 0x0000, { 0, 0 }, { 0x00, 0x81, 0x00, 0xFF } } },
    { { { -50, 50, -50 }, 0x0000, { 0, 0 }, { 0x00, 0x7F, 0x00, 0xFF } } },
    { { { 50, 50, 50 }, 0x0000, { 0, 0 }, { 0x00, 0x7F, 0x00, 0xFF } } },
    { { { -50, 50, 50 }, 0x0000, { 0, 0 }, { 0x00, 0x7F, 0x00, 0xFF } } },
    { { { 50, 50, -50 }, 0x0000, { 0, 0 }, { 0x00, 0x7F, 0x00, 0xFF } } },
};
Gfx gRideBoxDl[] = {
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR),
    gsSPSetGeometryMode(G_CULL_BACK | G_LIGHTING),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_OFF),
    gsDPSetCombineLERP(PRIMITIVE, 0, SHADE, 0, 0, 0, 0, 1, PRIMITIVE, 0, SHADE, 0, 0, 0, 0, 1),
    gsDPSetPrimColor(0, 0, 0xBF, 0x00, 0x00, 0xFF),
    gsSPVertex(gRideBoxVtx, 24, 0),
    gsSP2Triangles(2, 1, 0, 0, 1, 3, 0, 0),
    gsSP2Triangles(6, 5, 4, 0, 5, 7, 4, 0),
    gsSP2Triangles(10, 9, 8, 0, 9, 11, 8, 0),
    gsSP2Triangles(14, 13, 12, 0, 13, 15, 12, 0),
    gsSP2Triangles(18, 17, 16, 0, 17, 19, 16, 0),
    gsSP2Triangles(22, 21, 20, 0, 21, 23, 20, 0),
    gsSPEndDisplayList(),
};
extern s16 gFrmKindCore;
extern s16 gFrmKindMpol; /* "mpol" */
extern s16 gFrmKindPol; /* "pol" */
extern s16 gFrmKindAnime; /* "anime" */
extern f32 gWorldOriginX; /* the world's origin */
extern f32 gWorldOriginY;
extern f32 gWorldOriginZ;
extern f32 gFrmDrawOriginX;
extern f32 gFrmDrawOriginY;
extern f32 gFrmDrawOriginZ;
extern RideTest gRideBoxTest;
extern RideHit gRideHit;


#define MAX3(a, b, c) ((a) > (b) ? ((a) > (c) ? (a) : (c)) : ((b) > (c) ? (b) : (c)))

/* The handler: it makes the two trees and the "ride" kind, and updates the rides each step. */
s32 Ride(Obj *obj, s16 msg, void *arg) {
    switch (msg) {
        case 2:
            if (gRideTask != NULL) {
                return -1;
            }
            gRideTask = obj;
            obj->name = "Ride";
            if (gRideTree1 == NULL) {
                gRideTree1 = frmCreateRoot(gFrmKindCore);
            }
            if (gRideTree2 == NULL) {
                gRideTree2 = frmCreateRoot(gFrmKindCore);
            }
            if (gFrmKindRide == 0) {
                gFrmKindRide =
                    frmNewKind("ride", (Gfx * (*) (Gfx *, Frame *)) rideDraw,
                                           0x78, 0x28, NULL);
            }
            break;
        case 1:
            if (gRideTree1 != NULL) {
                frmDestroy(gRideTree1);
                gRideTree1 = NULL;
            }
            if (gRideTree2 != NULL) {
                frmDestroy(gRideTree2);
                gRideTree2 = NULL;
            }
            if (gFrmKindRide != 0) {
                frmDeleteKind(gFrmKindRide);
                gFrmKindRide = 0;
            }
            gRideTask = NULL;
            break;
        case 0:
            rideUpdateTree(gRideTree1);
            rideUpdateTree(gRideTree2);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A ride follows its model: its centre, its reach and its inverse matrix. For an "mpol" the
 * scale comes from the world matrix's rows 1-3 (the last is its place): as the game has it.
 * Each kind has its own pointer (one shared by "pol" and "anime" moves the float registers),
 * and the largest scale is an if/else into a local (MAX3's ternaries add temps). */
void rideFollowModel(RideFrame *r) {
    RPolFrame *p;
    RPolFrame *q;
    f32 max;
    RMPolFrame *mp;
    MtxF m;
    f32 sx;
    f32 sy;
    f32 sz;

    if (r == NULL || r->target == NULL) {
        return;
    }
    if (r->target->kind == gFrmKindPol) {
        p = (RPolFrame *) r->target;
        r->center[0] = p->pos[0];
        r->center[1] = p->pos[1];
        r->center[2] = p->pos[2];
        if (p->scale[0] > p->scale[1]) {
            if (p->scale[0] > p->scale[2]) {
                max = p->scale[0];
            } else {
                max = p->scale[2];
            }
        } else {
            if (p->scale[1] > p->scale[2]) {
                max = p->scale[1];
            } else {
                max = p->scale[2];
            }
        }
        r->reach = r->radius * max;
        mathMtxTransRotScaleInv(r->inv.m, p->pos[0], p->pos[1], p->pos[2], p->rot[0], p->rot[1],
                               p->rot[2], p->scale[0], p->scale[1], p->scale[2]);
    } else if (r->target->kind == gFrmKindAnime) {
        q = (RPolFrame *) r->target;
        r->center[0] = q->pos[0];
        r->center[1] = q->pos[1];
        r->center[2] = q->pos[2];
        if (q->scale[0] > q->scale[1]) {
            if (q->scale[0] > q->scale[2]) {
                max = q->scale[0];
            } else {
                max = q->scale[2];
            }
        } else {
            if (q->scale[1] > q->scale[2]) {
                max = q->scale[1];
            } else {
                max = q->scale[2];
            }
        }
        r->reach = r->radius * max;
        mathMtxTransRotScaleInv(r->inv.m, q->pos[0], q->pos[1], q->pos[2], q->rot[0], q->rot[1],
                               q->rot[2], q->scale[0], q->scale[1], q->scale[2]);
    } else if (r->target->kind == gFrmKindMpol) {
        mp = (RMPolFrame *) r->target;
        m = mp->world;
        mathMtxPostTrans(m.m, gFrmDrawOriginX, gFrmDrawOriginY, gFrmDrawOriginZ);
        r->center[0] = m.m[3][0];
        r->center[1] = m.m[3][1];
        r->center[2] = m.m[3][2];
        sx = sqrtf(mp->world.m[1][0] * mp->world.m[1][0] + mp->world.m[1][1] * mp->world.m[1][1] +
                   mp->world.m[1][2] * mp->world.m[1][2]);
        sy = sqrtf(mp->world.m[2][0] * mp->world.m[2][0] + mp->world.m[2][1] * mp->world.m[2][1] +
                   mp->world.m[2][2] * mp->world.m[2][2]);
        sz = sqrtf(mp->world.m[3][0] * mp->world.m[3][0] + mp->world.m[3][1] * mp->world.m[3][1] +
                   mp->world.m[3][2] * mp->world.m[3][2]);
        if (sx > sy) {
            if (sx > sz) {
                max = sx;
            } else {
                max = sz;
            }
        } else {
            if (sy > sz) {
                max = sy;
            } else {
                max = sz;
            }
        }
        r->reach = r->radius * max;
        mathMatInv4(m.m, r->inv.m);
    }
}

/* Updates every ride frame under f. */
void rideUpdateTree(Frame *f) {
    Frame *c;

    if (f->kind == gFrmKindRide) {
        rideFollowModel((RideFrame *) f);
    }
    for (c = f->child; c != NULL; c = c->next) {
        rideUpdateTree(c);
    }
}

/* Makes each box's inverse matrix. */
void rideInitBoxMtx(RideData *d) {
    RideBox *b = d->boxes;
    s32 i;

    for (i = 0; i < d->count; i++) {
        mathMtxTransRotScaleInv(b[i].inv.m, b[i].pos[0], b[i].pos[1], b[i].pos[2], b[i].rot[0],
                               b[i].rot[1], b[i].rot[2], 1.0f, 1.0f, 1.0f);
    }
}

/* A point moving from `from` to `to` against a box: stopped at the first face it crosses (y,
 * then x, then z). The move to there comes back turned into the world in *move. */
s32 rideBoxStop(RideBox *b, Vec3f *from, Vec3f *to, Vec3f *move, f32 *tp) {
    Vec3f p;
    Vec3f d;
    f32 t;

    guMtxXFMF(b->inv.m, from->x, from->y, from->z, &p.x, &p.y, &p.z);
    guMtxXFMF(b->inv.m, to->x, to->y, to->z, &d.x, &d.y, &d.z);
    d.x -= p.x;
    d.y -= p.y;
    d.z -= p.z;
    t = !(b->flags & 2) ? rideFaceStopT(p.y, p.x, p.z, d.y, d.x, d.z, b->half[1],
                                                 b->half[0], b->half[2])
                        : 1.0f;
    if (t == 1.0f) {
        t = !(b->flags & 1) ? rideFaceStopT(p.x, p.y, p.z, d.x, d.y, d.z, b->half[0],
                                                     b->half[1], b->half[2])
                            : 1.0f;
        if (t == 1.0f) {
            t = !(b->flags & 4) ? rideFaceStopT(p.z, p.y, p.x, d.z, d.y, d.x,
                                                         b->half[2], b->half[1], b->half[0])
                                : 1.0f;
            if (t == 1.0f) {
                return 0;
            }
        }
    }
    move->x = t * d.x;
    move->y = t * d.y;
    move->z = t * d.z;
    mathVecRotate(b->rot[0], b->rot[1], b->rot[2], &move->x, &move->y, &move->z);
    *tp = t;
    return 1;
}

/* Where a move (from a, by da) meets the face at +-ha of a box (half-sizes hb, hc across):
 * the part of the move, or 1. */
f32 rideFaceStopT(f32 a, f32 b, f32 c, f32 da, f32 db, f32 dc, f32 ha, f32 hb, f32 hc) {
    f32 t;

    if (fabsf(da) < 0.0001f) {
        return 1.0f;
    }
    if (da < 0.0f) {
        t = (ha - a) / da;
    } else {
        t = (-ha - a) / da;
    }
    if (t > 1.0f || t < 0.0f) {
        return 1.0f;
    }
    b = t * db + b;
    c = t * dc + c;
    if (-hb < b && b < hb && -hc < c && c < hc) {
        return t;
    }
    return 1.0f;
}

/* A point inside a box is pushed out through the nearest open face, 0.1 beyond it; the push
 * comes back turned into the world. Each case pushes by its own depth, modified in place (the
 * game writes the sum back into dx's register), and the default comes first in the switch (its
 * return sits right after the compares, and the call follows case 3). */
s32 rideBoxPushOut(RideBox *b, Vec3f *p, Vec3f *push) {
    Vec3f q;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 min;
    s32 axis;

    guMtxXFMF(b->inv.m, p->x, p->y, p->z, &q.x, &q.y, &q.z);
    if ((dx = b->half[0] - fabsf(q.x)) > 0.0f && (dy = b->half[1] - fabsf(q.y)) > 0.0f &&
        (dz = b->half[2] - fabsf(q.z)) > 0.0f) {
        axis = 0;
        min = 10000.0f;
        if (!(b->flags & 1) && dx < min) {
            min = dx;
            axis = 1;
        }
        if (!(b->flags & 2) && dy < min) {
            min = dy;
            axis = 2;
        }
        if (!(b->flags & 4) && dz < min) {
            axis = 3;
        }
        switch (axis) {
            default:
                return 0;
            case 1:
                dx += 0.1;
                if (q.x < 0.0f) {
                    dx *= -1.0f;
                }
                push->x = dx;
                push->y = 0.0f;
                push->z = 0.0f;
                break;
            case 2:
                dy += 0.1;
                if (q.y < 0.0f) {
                    dy *= -1.0f;
                }
                push->x = 0.0f;
                push->y = dy;
                push->z = 0.0f;
                break;
            case 3:
                dz += 0.1;
                if (q.z < 0.0f) {
                    dz *= -1.0f;
                }
                push->x = 0.0f;
                push->y = 0.0f;
                push->z = dz;
                break;
        }
        mathVecRotate(b->rot[0], b->rot[1], b->rot[2], &push->x, &push->y, &push->z);
        return 1;
    }
    return 0;
}

/* A point moving from `from` to `to` against a box: put onto the first face it crosses (x,
 * then y, then z), just outside. The move to there comes back turned into the world. */
s32 rideBoxOnto(RideBox *b, Vec3f *from, Vec3f *to, Vec3f *move, f32 *tp) {
    Vec3f p;
    Vec3f q;
    Vec3f d;
    f32 f;
    s32 hit = FALSE;

    guMtxXFMF(b->inv.m, from->x, from->y, from->z, &p.x, &p.y, &p.z);
    guMtxXFMF(b->inv.m, to->x, to->y, to->z, &q.x, &q.y, &q.z);
    d.x = q.x - p.x;
    d.y = q.y - p.y;
    d.z = q.z - p.z;
    f = !(b->flags & 1) ? rideFaceOntoPos(p.x, p.y, p.z, d.x, d.y, d.z, b->half[0],
                                                 b->half[1], b->half[2], tp)
                        : 0.0f;
    if (f != 0.0f) {
        q.x = f;
        hit = TRUE;
    } else {
        f = !(b->flags & 2) ? rideFaceOntoPos(p.y, p.x, p.z, d.y, d.x, d.z, b->half[1],
                                                     b->half[0], b->half[2], tp)
                            : 0.0f;
        if (f != 0.0f) {
            q.y = f;
            hit = TRUE;
        } else {
            f = !(b->flags & 4) ? rideFaceOntoPos(p.z, p.y, p.x, d.z, d.y, d.x,
                                                         b->half[2], b->half[1], b->half[0], tp)
                                : 0.0f;
            if (f != 0.0f) {
                q.z = f;
                hit = TRUE;
            }
        }
    }
    if (hit) {
        move->x = q.x - p.x;
        move->y = q.y - p.y;
        move->z = q.z - p.z;
        mathVecRotate(b->rot[0], b->rot[1], b->rot[2], &move->x, &move->y, &move->z);
    }
    return hit;
}

/* Where a move meets the face at +-ha: that face's place, 1 outside it (0: no meeting). */
f32 rideFaceOntoPos(f32 a, f32 b, f32 c, f32 da, f32 db, f32 dc, f32 ha, f32 hb, f32 hc,
                           f32 *tp) {
    f32 t;
    f32 s;
    f32 face;

    if (fabsf(da) < 0.0001f) {
        return 0.0f;
    }
    if (da < 0.0f) {
        face = ha;
        s = 1.0f;
    } else {
        face = -ha;
        s = -1.0f;
    }
    t = (face - a) / da;
    *tp = t;
    if (t > 1.0f || t < 0.0f) {
        return 0.0f;
    }
    b = t * db + b;
    c = t * dc + c;
    if (-hb < b && b < hb && -hc < c && c < hc) {
        return face + s;
    }
    return 0.0f;
}

/* Turns a vector from a model's frame into the world (rotation and scale). (The matrix is the
 * first local, at the frame's top, with three pad words under p; its translation is cleared by
 * one chain.) */
s32 rideToWorld(Frame *t, Vec3f *v) {
    MtxF m;
    RPolFrame *p;
    s32 pad[3];

    if (t->kind == gFrmKindPol) {
        p = (RPolFrame *) t;
        mathVecRotate(p->rot[0], p->rot[1], p->rot[2], &v->x, &v->y, &v->z);
        v->x *= p->scale[0];
        v->y *= p->scale[1];
        v->z *= p->scale[2];
    } else if (t->kind == gFrmKindAnime) {
        p = (RPolFrame *) t;
        mathVecRotate(p->rot[0], p->rot[1], p->rot[2], &v->x, &v->y, &v->z);
        v->x *= p->scale[0];
        v->y *= p->scale[1];
        v->z *= p->scale[2];
    } else if (t->kind == gFrmKindMpol) {
        m = ((RMPolFrame *) t)->world;
        m.m[3][0] = m.m[3][1] = m.m[3][2] = 0.0f;
        guMtxXFMF(m.m, v->x, v->y, v->z, &v->x, &v->y, &v->z);
    } else {
        return 0;
    }
    return 1;
}

/* The tree walks. Each takes a tree's Frame (the callers and the recursion pass Frames) and,
 * once the kind and id tests pass, casts it into a RideFrame local: IDO folds that local back
 * into the parameter everywhere but the loop's bottom test of the count, which reads through it
 * from a second register (s4 here, slot 0xA4 in rideTreeMoveHit). The locals are declared in
 * the game's stack order, and rideTreeMoveHit stores the frame through its own symbol
 * (gRideHitFrame, below). */
/* The first ride under f (other than id `skip`) whose boxes hold p: its id, and the push
 * out in *push. */
s32 rideTreePointIn(Frame *f, Vec3f *p, Vec3f *push, s32 skip) {
    f32 dx;
    f32 dy;
    f32 dz;
    Vec3f q;
    Frame *c;
    f32 x;
    f32 y;
    f32 z;
    s32 id;

    if (f->kind == gFrmKindRide && skip != ((RideFrame *) f)->id) {
        RideFrame *r = (RideFrame *) f;
        s32 i;

        x = p->x;
        dx = x - r->center[0];
        y = p->y;
        dy = y - r->center[1];
        z = p->z;
        dz = z - r->center[2];
        if (sqrtf(dx * dx + dy * dy + dz * dz) < r->reach) {
            guMtxXFMF(r->inv.m, x, y, z, &q.x, &q.y, &q.z);
            for (i = 0; i < r->count; i++) {
                if (rideBoxPushOut(&r->boxes[i], &q, push) != 0) {
                    rideToWorld(r->target, push);
                    return r->id;
                }
            }
        }
    }
    for (c = f->child; c != NULL; c = c->next) {
        if ((id = rideTreePointIn(c, p, push, skip)) != 0) {
            return id;
        }
    }
    return 0;
}

/* Clears the nearest hit. */
void rideClearHit(void) {
    gRideHit.t = 2.0f;
    gRideHit.move.x = gRideHit.move.y = gRideHit.move.z = 0.0f;
    gRideHit.frame = NULL;
}

/* gRideHit.frame under its own name: the game stores it absolutely (lui at), not through the
 * register holding &gRideHit. */
extern RideFrame *gRideHitFrame;

/* Keeps the nearest hit (by gRideBoxTest's test) of a move from `from` to `to`, of length
 * len, against the rides under f (other than id `skip`). */
void rideTreeMoveHit(Frame *f, Vec3f *from, Vec3f *to, f32 len, s32 skip) {
    RideFrame *r;
    f32 dx;
    f32 dy;
    Vec3f p;
    Vec3f q;
    Vec3f move;
    f32 dz;
    f32 t;
    f32 x;
    f32 y;
    f32 z;
    Frame *c;

    if (f->kind == gFrmKindRide && skip != ((RideFrame *) f)->id) {
        s32 i;

        r = (RideFrame *) f;
        x = from->x; dx = x - r->center[0];
        y = from->y; dy = y - r->center[1];
        z = from->z;
        dz = z - r->center[2];
        if (sqrtf(dx * dx + dy * dy + dz * dz) < r->reach + len) {
            guMtxXFMF(r->inv.m, x, y, z, &p.x, &p.y, &p.z);
            guMtxXFMF(r->inv.m, to->x, to->y, to->z, &q.x, &q.y, &q.z);
            for (i = 0; i < r->count; i++) {
                if (gRideBoxTest(&r->boxes[i], &p, &q, &move, &t) != 0 && t < gRideHit.t) {
                    rideToWorld(r->target, &move);
                    gRideHit.move = move;
                    gRideHit.t = t;
                    gRideHitFrame = r;
                }
            }
        }
    }
    for (c = f->child; c != NULL; c = c->next) {
        rideTreeMoveHit(c, from, to, len, skip);
    }
}

/* Sends message 7 to obj's tasks with a flag of 1, and returns the flag. */
s32 rideAskMsg7(Obj *obj) {
    s32 flag;

    if (gRideTask == NULL) {
        return 0;
    }
    flag = 1;
    tskSendMsg(obj, 7, &flag);
    return flag;
}

/* The ride of the second tree holding p (its id, the push in *push). */
s32 rideTree2PointIn(Vec3f *p, Vec3f *push) {
    if (gRideTask == NULL) {
        return 0;
    }
    return rideTreePointIn(gRideTree2, p, push, 0);
}

/* The same in the first tree, leaving out ride `skip`. */
s32 ridePointIn(Vec3f *p, Vec3f *push, s32 skip) {
    if (gRideTask == NULL) {
        return 0;
    }
    return rideTreePointIn(gRideTree1, p, push, skip);
}

/* The same, with the push turned by angle - pi/2. */
s32 ridePointInTurned(Vec3f *p, Vec3f *push, f32 angle, s32 skip) {
    s32 id;

    if (gRideTask == NULL) {
        return 0;
    }
    id = rideTreePointIn(gRideTree1, p, push, skip);
    if (id != 0) {
        mathVecRotateY(angle - 1.5707964f, &push->x, &push->y, &push->z);
    }
    return id;
}

/* A move from p by d onto the rides of the first tree: the move to the nearest face in *out,
 * and that ride's id. */
s32 rideMoveOnto(Vec3f *p, Vec3f *d, Vec3f *out, s32 skip) {
    Vec3f to;

    if (gRideTask == NULL) {
        return 0;
    }
    rideClearHit();
    gRideBoxTest = rideBoxOnto;
    to.x = p->x + d->x;
    to.y = p->y + d->y;
    to.z = p->z + d->z;
    rideTreeMoveHit(gRideTree1, p, &to,
                           sqrtf(d->x * d->x + d->y * d->y + d->z * d->z), skip);
    *out = gRideHit.move;
    if (gRideHit.frame != NULL) {
        return gRideHit.frame->id;
    }
    return 0;
}

/* The same, stopped at the face rather than put on it. */
s32 rideMoveStop(Vec3f *p, Vec3f *d, Vec3f *out, s32 skip) {
    Vec3f to;

    if (gRideTask == NULL) {
        return 0;
    }
    rideClearHit();
    gRideBoxTest = rideBoxStop;
    to.x = p->x + d->x;
    to.y = p->y + d->y;
    to.z = p->z + d->z;
    rideTreeMoveHit(gRideTree1, p, &to,
                           sqrtf(d->x * d->x + d->y * d->y + d->z * d->z), skip);
    *out = gRideHit.move;
    if (gRideHit.frame != NULL) {
        return gRideHit.frame->id;
    }
    return 0;
}

/* The same as rideMoveOnto, with the move turned by angle - pi/2. */
s32 rideMoveOntoTurned(Vec3f *p, Vec3f *d, Vec3f *out, f32 angle, s32 skip) {
    s32 pad;
    Vec3f to;

    if (gRideTask == NULL) {
        return 0;
    }
    rideClearHit();
    gRideBoxTest = rideBoxOnto;
    to.x = p->x + d->x;
    to.y = p->y + d->y;
    to.z = p->z + d->z;
    rideTreeMoveHit(gRideTree1, p, &to,
                           sqrtf(d->x * d->x + d->y * d->y + d->z * d->z), skip);
    *out = gRideHit.move;
    if (gRideHit.frame != NULL) {
        mathVecRotateY(angle - 1.5707964f, &out->x, &out->y, &out->z);
        return gRideHit.frame->id;
    }
    return 0;
}

/* A ride frame under parent, with id, following target, with the boxes of data (or the
 * default set). */
RideFrame *rideCreate(Frame *parent, s32 id, Frame *target, RideData *data) {
    RideFrame *r;

    if (gRideTask == NULL) {
        return NULL;
    }
    r = (RideFrame *) frmCreate(parent, gFrmKindRide);
    if (r != NULL) {
        if (data == NULL) {
            data = &gRideDefaultData;
        }
        r->id = id;
        r->target = target;
        r->count = data->count;
        r->radius = data->radius;
        r->boxes = data->boxes;
        rideInitBoxMtx(data);
        PRINTF("rideCreate: size(%f,%f,%f)\n", data->boxes[0].half[0], data->boxes[0].half[1],
               data->boxes[0].half[2]);
    }
    return r;
}

/* A model's matrix, less the world's origin. */
void rideModelMtx(Frame *t, Mtx *m) {
    RPolFrame *p = (RPolFrame *) t;

    if (t->kind == gFrmKindPol) {
        mathMtxTransRotScaleL(m, p->pos[0] - gWorldOriginX, p->pos[1] - gWorldOriginY,
                               p->pos[2] - gWorldOriginZ, p->rot[0], p->rot[1], p->rot[2],
                               p->scale[0], p->scale[1], p->scale[2]);
    } else if (t->kind == gFrmKindAnime) {
        mathMtxTransRotScaleL(m, p->pos[0] - gWorldOriginX, p->pos[1] - gWorldOriginY,
                               p->pos[2] - gWorldOriginZ, p->rot[0], p->rot[1], p->rot[2],
                               p->scale[0], p->scale[1], p->scale[2]);
    } else if (t->kind == gFrmKindMpol) {
        mathMtxFToL(m, ((RMPolFrame *) t)->world.m);
    }
}

/* The "ride" kind's draw: each box as a unit box scaled to it (debug). (The frame comes in as a
 * Frame and is cast into a RideFrame local, as gfxmgr.c's draw handlers: the game keeps both,
 * the parameter in its home and the local at 0x64.) */
Gfx *rideDraw(Gfx *gfx, Frame *frame) {
    RideFrame *r = (RideFrame *) frame;
    RideBox *b;
    Mtx *m;
    s32 i;

    gfx = camlightLoadLights(gfx);
    m = gfxAllocMtx(1);
    rideModelMtx(r->target, m);
    gSPMatrix(gfx++, K0_TO_PHYS(m), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_PUSH);
    for (i = 0; i < r->count; i++) {
        b = &r->boxes[i];
        m = gfxAllocMtx(1);
        mathMtxTransRotScaleL(m, b->pos[0], b->pos[1], b->pos[2], b->rot[0], b->rot[1],
                               b->rot[2], b->half[0] / 50.0f, b->half[1] / 50.0f,
                               b->half[2] / 50.0f);
        gSPMatrix(gfx++, K0_TO_PHYS(m), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
        gSPDisplayList(gfx++, gRideBoxDl);
        gSPPopMatrix(gfx++, G_MTX_MODELVIEW);
    }
    gSPPopMatrix(gfx++, G_MTX_MODELVIEW);
    return gfx;
}
