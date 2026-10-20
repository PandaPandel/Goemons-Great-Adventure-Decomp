#include "player.h"
#include "diveef.h"
#include "floorhit.h"
#include "mathutil.h"
#include "person.h"
#include "pl3d.h"
#include "plmove.h"
#include "shadow.h"
#include "vram.h"

#pragma intrinsic (fabs)

/* The player's movement in a 3-D area (overlay 9; flag 0x10, plmain.c calls it instead of
 * plmove.c's). Player+0x50/0x54/0x58 are x, y and z here, and the speeds (0x5C, 0x70, 0x84:
 * x, y, z) turn with the camera. Each step probes the moving floors, the ceiling, a wall ahead
 * and the ground under (and, falling, at 5 ahead and behind). */

/* pl3d.c's .data (0x801B44D0-0x801B4500): the player's two spheres for the moving floors. */
PlFloorBuf gPl3dFloorSpheres = { {
    { 5.0f, { 0.0f, 6.0f, 0.0f } },
    { 5.0f, { 0.0f, 10.0f, 0.0f } },
    { 0.0f, { 0.0f, 0.0f, 0.0f } },
} };

/* A ray to the ground or a wall. */
typedef struct Pl3dRay {
    /* 0x00 */ CollTri *hit;
    /* 0x04 */ Vec3f from;
    /* 0x10 */ Vec3f dir;
    /* 0x1C */ Vec3f at;
    /* 0x28 */ f32 t;
} Pl3dRay;

extern Obj *gEneManager;

CollTri *pl3dRayWall(Player *p, Pl3dRay *r, Vec3f *vel);
CollTri *pl3dRayCeiling(Player *p, Pl3dRay *r, Vec3f *vel);
CollTri *pl3dRayGround(Player *p, Pl3dRay *r, Vec3f *vel);
CollTri *pl3dRayGroundAhead(Player *p, Pl3dRay *r, Vec3f *vel);
CollTri *pl3dRayGroundBehind(Player *p, Pl3dRay *r, Vec3f *vel);
CollTri *pl3dRayFloors(Player *p, PlFloor *out, Vec3f *vel);

/* A step in a 3-D area. */
void pl3dStep(Player *p) {
    s32 i;
    Vec3f vel;

    plMoveWater(p);
    pl3dApplyPush(p);
    pl3dCapSpeed(p);
    vel.x = PL_HIST(p)[0][0];
    vel.y = PL_HIST(p)[1][0];
    vel.z = PL_HIST(p)[2][0];
    pl3dRideFloors(p, &vel);
    if (!(p->flags & 1)) {
        pl3dCollide(p, &vel);
    }
    if (plMoveTiedXZ(p, &vel)) {
        vel.z = 0.0f;
        vel.x = 0.0f;
    }
    pl3dAddVel(p, &vel);
    pl3dSetModelPos(p);
    pl3dTurnModel(p, p->velZ, p->velX);
    shadowPut(p->shadow, p->model->pos[0], p->model->pos[1], p->model->pos[2], 1.0f, 0);
    for (i = 4; i != 0; i--) {
        PL_HIST(p)[0][i] = PL_HIST(p)[0][i - 1];
        PL_HIST(p)[1][i] = PL_HIST(p)[1][i - 1];
        PL_HIST(p)[2][i] = PL_HIST(p)[2][i - 1];
        PL_HIST(p)[3][i] = PL_HIST(p)[3][i - 1];
        PL_HIST(p)[4][i] = PL_HIST(p)[4][i - 1];
        PL_HIST(p)[5][i] = PL_HIST(p)[5][i - 1];
    }
    p->pushZ = 0.0f;
    p->pushY = 0.0f;
    p->pushX = 0.0f;
    p->jumpPush = 0.0f;
    plWaterSplash(p);
}

/* The ground and walls for a step's speed vel: moving floors push it, the ceiling stops it, a
 * wall ahead turns it along the wall, the ground under (or, falling, 5 ahead or behind, if flat
 * enough) sets the height and the ground's kind. The ground's slope sets the grip (0.95 steep,
 * 0.6 flat), and a slope pushes down as it is walked up.
 * The normal is read as a Vec3f (named fields keep the loads in source order; n[2]/n[0] are
 * loaded first), d is a named local in f0 at the wall too, and the int 0 stored to unk070 keeps
 * that store out of the shared 0.0f register (the game's fresh mtc1 there and at the wall). */
s32 pl3dCollide(Player *p, Vec3f *vel) {
    Pl3dRay up;
    Pl3dRay down;
    Pl3dRay front;
    Pl3dRay back;
    Pl3dRay wall;
    PlFloor floor;
    CollTri *tri;
    f32 d;

    p->ground = 0;
    p->wallKind = 0;
    tri = NULL;
    if (pl3dRayFloors(p, &floor, vel)) {
        vel->z = floor.push.z;
        vel->x = floor.push.x;
        p->wallKind = floor.tri->kind;
    }
    if (pl3dRayCeiling(p, &up, vel)) {
        vel->y = up.at.y - (p->height + 20.0f);
    }
    if (pl3dRayWall(p, &wall, vel)) {
        tri = wall.hit;
        d = p->velZ * tri->n.z + p->velX * tri->n.x;
        if (d < 0.0f) {
            vel->x = wall.at.x - wall.from.x;
            vel->z = wall.at.z - wall.from.z;
            p->wallKind = wall.hit->kind;
        }
    }
    if (pl3dRayGround(p, &down, vel)) {
        tri = down.hit;
        if (tri->n.y > 0.0f) {
            vel->y = down.at.y - p->height;
            p->ground = down.hit->kind;
            p->groundSurface = down.hit->soundKind;
        }
    } else if (vel->y < 0.0f && pl3dRayGroundAhead(p, &front, vel)) {
        tri = front.hit;
        if (tri->n.y > 0.7f) {
            vel->y = front.at.y - p->height;
            p->ground = front.hit->kind;
            p->groundSurface = front.hit->soundKind;
        }
    } else if (vel->y < 0.0f && pl3dRayGroundBehind(p, &back, vel)) {
        tri = back.hit;
        if (tri->n.y > 0.7f) {
            vel->y = back.at.y - p->height;
            p->ground = back.hit->kind;
            p->groundSurface = back.hit->soundKind;
        }
    }
    if (tri != NULL) {
        if (tri->n.y < 0.5f) {
            p->ground = 0;
        }
        if (tri->n.y < 0.7f) {
            p->dragY = p->drag = 0.95f;
        } else {
            p->dragY = p->drag = 0.6f;
        }
    }
    if (p->ground && p->velY < 0.0f) {
        p->velY = 0;
    }
    if (tri != NULL) {
        d = p->velZ * tri->n.z + p->velX * tri->n.x;
        if (d > 0.0f) {
            p->velY -= d * 0.8f;
        }
    }
    return 0;
}

/* The wall ahead: from 10 up, along the step flat and 3 more; the point is pulled back by
 * the 3. */
CollTri *pl3dRayWall(Player *p, Pl3dRay *r, Vec3f *vel) {
    Vec3f d;

    r->from.x = p->place;
    r->from.y = p->height + 10.0f;
    r->from.z = p->side;
    d = *vel;
    d.y = 0.0f;
    r->dir = d;
    mathVecNormalizeB(&d);
    mathVecScale(&d, 3.0f);
    r->dir.x += d.x;
    r->dir.z += d.z;
    r->hit = vramRayGround(&r->from, &r->dir, &r->at, &r->t);
    if (r->hit != NULL) {
        r->at.x -= d.x;
        r->at.z -= d.z;
    }
    return r->hit;
}

/* The ceiling: from 2 up, up by the step and 18. */
CollTri *pl3dRayCeiling(Player *p, Pl3dRay *r, Vec3f *vel) {
    r->from.x = p->place;
    r->from.y = p->height + 2.0f;
    r->from.z = p->side;
    r->dir.x = 0.0f;
    r->dir.y = vel->y + 18.0f;
    r->dir.z = 0.0f;
    return r->hit = vramRayGround(&r->from, &r->dir, &r->at, &r->t);
}

/* The ground under: from 10 up, down by the step and 10. */
CollTri *pl3dRayGround(Player *p, Pl3dRay *r, Vec3f *vel) {
    r->from.x = p->place;
    r->from.y = p->height + 10.0f;
    r->from.z = p->side;
    r->dir.x = 0.0f;
    r->dir.y = vel->y - 10.0f;
    r->dir.z = 0.0f;
    return r->hit = vramRayGround(&r->from, &r->dir, &r->at, &r->t);
}

/* The ground 5 ahead (the way the player moves). */
CollTri *pl3dRayGroundAhead(Player *p, Pl3dRay *r, Vec3f *vel) {
    f32 a = pl3dGetMoveAngle(p);

    r->from.x = mathSin(a) * 5.0f + p->place;
    r->from.y = p->height + 10.0f;
    r->from.z = mathCos(a) * 5.0f + p->side;
    r->dir.x = 0.0f;
    r->dir.y = vel->y - 10.0f;
    r->dir.z = 0.0f;
    return r->hit = vramRayGround(&r->from, &r->dir, &r->at, &r->t);
}

/* And 5 behind. */
CollTri *pl3dRayGroundBehind(Player *p, Pl3dRay *r, Vec3f *vel) {
    f32 a = pl3dGetMoveAngle(p) + 3.1415927f;

    r->from.x = mathSin(a) * 5.0f + p->place;
    r->from.y = p->height + 10.0f;
    r->from.z = mathCos(a) * 5.0f + p->side;
    r->dir.x = 0.0f;
    r->dir.y = vel->y - 10.0f;
    r->dir.z = 0.0f;
    return r->hit = vramRayGround(&r->from, &r->dir, &r->at, &r->t);
}

/* The moving floors, from 1 up along the step, flat. */
CollTri *pl3dRayFloors(Player *p, PlFloor *out, Vec3f *vel) {
    PlFloorBuf buf;
    PlFloor q;

    buf = gPl3dFloorSpheres;
    q.from.x = p->place;
    q.from.y = p->height + 1.0f;
    q.from.z = p->side;
    q.dir.x = vel->x;
    q.dir.y = 0.0f;
    q.dir.z = vel->z;
    q.buf = buf.s;
    q.tri = fhSweepSpheres(&q);
    *out = q;
    return out->tri;
}

void pl3dUnusedStub(void) {
}

void pl3dAddVel(Player *p, Vec3f *vel) {
    p->place += vel->x;
    p->height += vel->y;
    p->side += vel->z;
}

void pl3dSetModelPos(Player *p) {
    p->model->pos[0] = p->place;
    p->model->pos[1] = p->height;
    p->model->pos[2] = p->side;
}

/* The speed caps (none while button 0x4000 is held): across at 0x1E8, falling at 0x1EC, rising
 * at 12. The speed s is reused for the scale: a separate scale local swaps f14 and f16. */
void pl3dCapSpeed(Player *p) {
    f32 s = sqrtf(p->velZ * p->velZ + p->velX * p->velX);

    if (!(p->buttons & 0x4000)) {
        if (s > p->maxSpeed) {
            s = p->maxSpeed / s;
            p->velX *= s;
            p->velZ *= s;
        }
        if (p->velY < p->maxFall) {
            p->velY = p->maxFall;
            return;
        }
        if (p->velY > 12.0f) {
            p->velY = 12.0f;
        }
    }
}

/* Moves by the stick, turned with the camera (a free move). The camera's two differences are
 * locals: inline in the call, IDO subtracted the z pair first and the x pair in the delay slot. */
void pl3dFreeMove(Player *p) {
    CamView *view = gGameWork.view;
    s32 index = p->index;
    s32 sx;
    s32 sy;
    s32 pad[3];
    f32 a;
    f32 vx;
    f32 vz;
    f32 s;
    s32 pad2;
    f32 dx;
    f32 dz;

    sx = plMoveStickX(index, 0);
    sy = plMoveStickY(index, 0);
    if (view != NULL) {
        if (fabs(sx) < 5.0) {
            sx = 0;
        }
        if (fabs(sy) < 5.0) {
            sy = 0;
        }
        p->velZ = sx * 0.0375f;
        p->velX = sy * 0.0375f;
        dx = view->at[0] - view->eye[0];
        dz = view->at[2] - view->eye[2];
        a = mathAtan2(dx, dz);
        vx = p->velZ;
        vz = p->velX;
        s = mathSin(a);
        p->velZ = mathCos(a) * vz + vx * s;
        s = mathCos(a);
        p->velX = mathSin(a) * vz + -vx * s;
        p->pushX = p->pushY = p->pushZ = p->velY = 0.0f;
    }
}

/* The pushes (0x98 and 0xC0), turned with the camera (or not, with flag 0x40000000), added to
 * the speeds; the gravity and lifts to the speed up; then the grip. */
void pl3dApplyPush(Player *p) {
    s32 pad[5];
    f32 a;
    f32 vx;
    f32 vz;
    s32 pad2;
    f32 s;

    if (gGameWork.view != NULL) {
        if (p->flags & 0x40000000) {
            p->velZ += p->pushZ;
            p->velX += p->pushX;
        } else {
            a = mathAtan2(gGameWork.view->at[0] - gGameWork.view->eye[0],
                                       gGameWork.view->at[2] - gGameWork.view->eye[2]);
            vx = p->pushZ;
            vz = p->pushX;
            s = mathCos(a);
            p->velZ += vx * mathSin(a) + vz * s;
            s = mathSin(a);
            p->velX += -vx * mathCos(a) + vz * s;
        }
        p->velY += p->pushY + p->gravity + p->jumpPush;
        p->velX *= p->drag;
        p->velZ *= p->drag;
        p->velY *= p->dragY;
    }
}

/* Turns the model the way it moves, when it moves. */
void pl3dTurnModel(Player *p, f32 x, f32 z) {
    if (sqrtf(x * x + z * z) > 0.05f) {
        p->model->rot[1] = mathAtan2(z, x);
    }
}

/* The way the player moves (its facing, standing still). */
f32 pl3dGetMoveAngle(Player *p) {
    f32 x = p->velZ;

    if (fabs(x) < 1e-5f && fabs(p->velX) < 1e-5f) {
        return p->model->rot[1];
    }
    return mathAtan2(p->velX, x);
}

/* Lets the moving floors carry the player. */
void pl3dRideFloors(Player *p, Vec3f *vel) {
    Obj *o;

    for (o = gEneManager->child->next->next->next->child; o != NULL; o = o->next) {
        personPlCollCheck(o, p, vel);
    }
}
