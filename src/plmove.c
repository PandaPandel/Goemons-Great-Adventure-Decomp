#include "player.h"
#include "diveef.h"
#include "fade.h"
#include "floorhit.h"
#include "joypad.h"
#include "mathutil.h"
#include "person.h"
#include "plmain.h"
#include "plmove.h"
#include "plutil.h"
#include "ride.h"
#include "shadow.h"
#include "tmanager.h"
#include "vram.h"

/* The player's controls and movement (overlay 9): the buttons through the player's own mapping,
 * the stick, and each step's move along the course, with the ground under the player, the walls
 * and slopes ahead, the water it stands in, and the other players (who may not walk off the
 * screen). The course is a rail: a place along it (Player+0x50), a height and a place across it
 * (0x58); vramCoursePointXZ turns a place into x and z, vramCourseHeading gives the rail's heading. */

typedef struct PlRay {
    /* 0x00 */ Vec3f from;
    /* 0x0C */ Vec3f dir;
} PlRay;

/* Message 6's argument: a place on the course, moved by the object. */
typedef struct PlPlace {
    /* 0x0 */ s16 zone;
    /* 0x4 */ f32 place;
    /* 0x8 */ f32 side;
    /* 0xC */ f32 height;
} PlPlace;

extern s32 gGameFrameCount; /* the frame counter */
/* A button mapping: the player's button for each of the pad's. */
typedef struct PlButtons {
    /* 0x00 */ u16 b[16];
} PlButtons;

/* plmove.c's .data (0x801A84B0-0x801A8520). */
PlButtons gPlButtonMap = { { /* the mapping of the player (plMoveResetButtonMap resets it to the defaults) */
    0x0004, 0x0001, 0x0002, 0x0008, 0x0400, 0x0100, 0x0200, 0x0800,
    0x1000, 0x2000, 0x0010, 0x2020, 0x4000, 0x8000, 0x0000, 0x0000,
} };
PlButtons gPlButtonMapDefault = { { /* the defaults: each button itself, L also Z */
    R_CBUTTONS, L_CBUTTONS, D_CBUTTONS, U_CBUTTONS, R_TRIG, L_TRIG | Z_TRIG, 0, 0,
    R_JPAD, L_JPAD, D_JPAD, U_JPAD, START_BUTTON, Z_TRIG, B_BUTTON, A_BUTTON,
} };
PlFloorBuf gPlFloorBufTemplate = { { { 10.0f } } }; /* one sphere of radius 10, at the point */
extern Obj *gEneManager;

s32 ridePointIn(Vec3f *p, Vec3f *push, s32 a, Vec3f *b); /* FAKE: in no header: used here as this file declares it (The number of arguments doesn't agree with the number in the declaration.); ride.h has `s32 ridePointIn(Vec3f *p, Vec3f *push, s32 skip)` (src/plmove.c) */

CollTri *plMoveFloorBelow(Player *p, PlProbe *pr, PlHit *hit);
CollTri *plMoveFloorBehind(Player *p, PlProbe *pr, PlHit *hit);
CollTri *plMoveFloorAhead(Player *p, PlProbe *pr, PlHit *hit);
CollTri *plMoveCastRay(PlRay *ray, PlHit *hit);
f32 plMoveTriSlope(Player *p, CollTri *t, f32 heading);
CollTri *plMoveFloorQuery(Player *p, Vec3f *from, Vec3f *dir, PlFloor *out);

/* The pad's buttons through the player's mapping (reset to the defaults first). */
u16 plMoveMapButtons(u16 pad) {
    s32 pad2;
    u16 out = 0;
    s32 i;

    plMoveResetButtonMap();
    for (i = 0; i < 16; i++) {
        if (pad & (1 << i)) {
            out |= gPlButtonMap.b[i];
        }
    }
    return out;
}

u16 plMoveGetButtons(s32 port, s32 back) {
    return plMoveMapButtons(joyGetButton(port, back));
}

u16 plMoveHeldTime(s32 port, s32 mask) {
    return joyGetHeldTime(port, plMoveMapButtons(mask));
}

/* The stick across; the D-pad's left and right push it all the way. */
s32 plMoveStickX(s32 port, s32 back) {
    PadHist *h = joyGetPadHist(port, back);

    if (h->button & 0x200) {
        return -80;
    }
    if (h->button & 0x100) {
        return 80;
    }
    return h->x;
}

/* The stick up; the D-pad's up and down. */
s32 plMoveStickY(s32 port, s32 back) {
    PadHist *h = joyGetPadHist(port, back);

    if (h->button & 0x800) {
        return 80;
    }
    if (h->button & 0x400) {
        return -80;
    }
    return h->y;
}

/* Resets the player's mapping to the defaults. (A do-while: IDO's unrolled copy then keeps the four
 * halves in order, the last in the delay slot; a for loop puts the first there.) */
void plMoveResetButtonMap(void) {
    s32 i = 0;

    do {
        gPlButtonMap.b[i] = gPlButtonMapDefault.b[i];
        i++;
    } while (i < 16);
}

/* This step's buttons, and those newly pressed. */
void plMoveReadButtons(Player *p) {
    u16 old = p->buttons;

    p->buttons = plMoveGetButtons(p->index, 0);
    p->held = p->buttons ^ (old & p->buttons);
}

/* A step's move: carried or swimming, on a swing, or on foot; then the model follows. */
void plMoveStep(Player *p) {
    s32 i;

    if (p->flags & 0x410000) {
        plMoveStepFlyDive(p);
    } else if (p->flags & 0x40000) {
        plMoveStepLetGo(p);
    } else {
        plMoveStepWalk(p);
    }
    p->model->rot[0] = p->lean + p->leanOffset;
    p->model->rot[1] = vramCourseHeading(p->zone, p->place) + (p->facing + p->tilt);
    p->model->rot[2] = p->roll + p->rollOffset;
    p->model->pos[0] += p->modelOffset[0];
    p->model->pos[1] += p->modelOffset[1];
    p->model->pos[2] += p->modelOffset[2];
    if (p->shadow != NULL) {
        shadowPut(p->shadow, p->model->pos[0], p->model->pos[1], p->model->pos[2], 1.0f, 0);
    }
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
}

/* Carried or swimming: the speed is held under 2.7 (6.8 in action 0x49). */
void plMoveStepFlyDive(Player *p) {
    Vec3f vel;
    PlGround g;
    PlGround g2;
    f32 speed;
    f32 max;
    f32 fr;

    plMoveWater(p);
    p->velX += p->pushX;
    p->velZ += p->pushZ;
    p->velY += p->pushY;
    if (!(p->flags & 0x400000)) {
        p->velY += p->gravity + p->jumpPush;
    }
    fr = p->drag;
    p->velX *= fr;
    p->velY *= (p->flags & 0x400000) ? fr : p->dragY;
    p->velZ *= fr;
    speed = plSpeedLen(p);
    max = (p->action != 0x49) ? 2.7f : 6.8f;
    if (max < speed) {
        speed = max / speed;
        p->velX *= speed;
        p->velY *= speed;
        p->velZ *= speed;
    }
    vel.x = p->velX;
    vel.y = p->velY;
    vel.z = p->velZ;
    plMoveObjPush(p, &vel);
    plMoveWaterCarry(p, &vel);
    g2.ground = 0;
    if (p->flags & 0x400000) {
        plMoveRideCollide(p, &vel, &g2);
    }
    plMoveCollideFlyDive(p, &vel, &g);
    p->ceilingKind = g.above;
    p->ground = g.ground;
    p->groundSurface = g.soundKind;
    p->edgeFlags = g.edgeFlags;
    p->slope = g.angle;
    p->wallKind = g.ahead;
    if (g.ground == 0 && g2.ground != 0) {
        p->ground = g2.ground;
        p->groundSurface = g2.soundKind;
        p->edgeFlags = 0;
        p->slope = 0.0f;
    }
    if (plMoveTiedXZ(p, &vel)) {
        vel.x = vel.z = 0.0f;
    }
    if (plMoveTiedY(p, vel.y, 0.725f, 1)) {
        vel.y = 0.0f;
    }
    p->place += vel.x;
    p->height += vel.y;
    vramCoursePointXZ(p->zone, p->place, p->side, &p->model->pos[0], &p->model->pos[2]);
    p->model->pos[1] = p->height;
}

/* On a swing (flag 0x40000). */
void plMoveStepLetGo(Player *p) {
    Vec3f vel;
    PlGround g;
    PlGround g2;
    s32 pad[2];

    plMoveWater(p);
    p->velX += p->pushX;
    p->velY += p->pushY + p->gravity + p->jumpPush;
    p->velZ += p->pushZ;
    p->velX *= p->drag;
    p->velY *= p->dragY;
    p->velZ *= p->drag;
    plMoveClampSpeed(p);
    vel.x = p->velX;
    vel.y = p->velY;
    vel.z = p->velZ;
    plMoveWaterCarry(p, &vel);
    g2.ground = 0;
    if (!(p->flags & 2)) {
        plMoveRideCollide(p, &vel, &g2);
    }
    plMoveCollideLetGo(p, &vel, &g);
    p->velX = vel.x;
    p->velY = vel.y;
    p->velZ = vel.z;
    p->ceilingKind = g.above;
    p->ground = g.ground;
    p->groundSurface = g.soundKind;
    p->edgeFlags = g.edgeFlags;
    p->slope = g.angle;
    p->wallKind = g.ahead;
    if (g.ground == 0 && g2.ground != 0) {
        p->ground = g2.ground;
        p->groundSurface = g2.soundKind;
        p->edgeFlags = 0;
        p->slope = 0.0f;
    }
    if (plMoveTiedXZ(p, &vel)) {
        vel.x = vel.z = 0.0f;
    }
    if (plMoveTiedY(p, vel.y, 1.2f, 0)) {
        vel.y = 0.0f;
    }
    p->place += vel.x;
    p->height += vel.y;
    vramCoursePointXZ(p->zone, p->place, p->side, &p->model->pos[0], &p->model->pos[2]);
    p->model->pos[1] = p->height;
    plWaterSplash(p);
}

/* On foot. A slope steeper than pi/4 against the way the player moves slides it back. */
void plMoveStepWalk(Player *p) {
    s32 pad0[2];
    s16 zone;
    Vec3f vel;
    PlGround g;
    PlGround g2;
    s32 pad;
    f32 side;
    f32 k;

    if (!(p->flags & 4)) {
        plMoveWater(p);
        p->velX += p->pushX;
        p->velY += p->pushY + p->gravity + p->jumpPush;
        p->velZ += p->pushZ;
        if (((p->velX > 0.0f) ? 1 : -1) * p->slope >= 0.7853982f) {
            p->velX *= 0.95f;
            p->velZ *= 0.95f;
        } else {
            p->velX *= p->drag;
            p->velY *= p->dragY;
            p->velZ *= p->drag;
        }
        k = mathSin(p->slope) * -p->velX * 0.4f;
        if (k < 0.0f) {
            p->velY += k;
        }
        plMoveClampSpeed(p);
        vel.x = p->velX;
        vel.y = p->velY;
        vel.z = p->velZ;
        if (p->flags & 0x08000000) {
            plVecScaleSelf(&vel, 0.3f);
        }
        if (plMoveTiedY(p, vel.y, 0.8f, 0)) {
            vel.y = 0.0f;
        }
        plMoveObjPush(p, &vel);
        plMoveWaterCarry(p, &vel);
        g2.ground = 0;
        if (!(p->flags & 2)) {
            plMoveRideCollide(p, &vel, &g2);
        }
        if (!(p->flags & 1)) {
            plMoveProbeGround(p, &vel, &zone, &g);
            if (g.ground != 0) {
                p->ground = g.ground;
                p->groundSurface = g.soundKind;
                p->edgeFlags = g.edgeFlags;
                p->slope = g.angle;
            } else if (g2.ground != 0) {
                p->ground = g2.ground;
                p->groundSurface = g2.soundKind;
                p->edgeFlags = 0;
                p->slope = 0.0f;
            } else {
                p->ground = 0;
                p->edgeFlags = 0;
                p->slope = 0.0f;
            }
            p->wallKind = g.ahead;
            p->ceilingKind = g.above;
        } else {
            zone = p->zone;
        }
        if (p->flags & 0x08000000) {
            p->edgeFlags = 0;
        }
        if (plMoveTiedXZ(p, &vel)) {
            vel.x = vel.z = 0.0f;
        }
        if (plMoveTiedY(p, vel.y, 1.0f, 0)) {
            vel.y = 0.0f;
        }
        p->place += vel.x;
        p->height += vel.y;
        if (p->ground != 0 && p->velY < 0.0f) {
            p->velY = 0.0f;
        }
    } else {
        zone = p->zone;
    }
    vramCoursePointXZ(p->zone, p->place, p->side, &p->model->pos[0], &p->model->pos[2]);
    p->model->pos[1] = p->height;
    if (zone != p->zone && zone != 0) {
        p->zone = zone;
        p->place = vramNearestPlace(zone, p->model->pos[0], p->model->pos[1], p->model->pos[2], &side);
    }
    plWaterSplash(p);
}

/* The ground under a step: straight down, else a step behind (0x8000) or ahead (0x4000); then
 * the wall ahead at the feet and at the waist, whichever is nearer. (The signs compare with an
 * int 0: against 0.0f, the compare's zero register is shared with the sign's 0.0f.) */
void plMoveProbeGround(Player *p, Vec3f *vel, s16 *zone, PlGround *g) {
    f32 heading;
    PlProbe pr;
    Vec3f base;
    PlHit hitA;
    PlHit hitD;
    PlHit hitE;
    PlHit hitF;
    PlHit hitB;
    PlHit hitC;
    PlHit *s;
    PlHit *h;
    s32 pad[3];
    PlProbe pr2;
    PlProbe pr3;
    Vec3f a;
    Vec3f b;
    Vec3f at;
    f32 t;

    heading = vramCourseHeading(p->zone, p->place);
    *zone = p->zone;
    pr.place = p->place;
    pr.y = p->height - p->probeDrop;
    pr.side = p->side;
    pr.d = *vel;
    pr.sign.x = (pr.d.x == 0) ? 0.0f : ((pr.d.x >= 0) ? 1 : -1);
    pr.sign.y = (pr.d.y == 0) ? 0.0f : ((pr.d.y >= 0) ? 1 : -1);
    pr.sign.z = (pr.d.z == 0) ? 0.0f : ((pr.d.z >= 0) ? 1 : -1);
    pr.heading = heading;
    vramCoursePointXZ(p->zone, pr.place, pr.side, &base.x, &base.z);
    base.y = pr.y;
    g->edgeFlags = 0;
    if (plMoveFloorBelow(p, &pr, &hitA) != NULL) {
        h = &hitA;
    } else if (plMoveFloorBehind(p, &pr, &hitB) != NULL) {
        g->edgeFlags = 0x8000;
        h = &hitB;
    } else if (plMoveFloorAhead(p, &pr, &hitC) != NULL) {
        g->edgeFlags = 0x4000;
        h = &hitC;
    } else {
        h = &hitA;
    }
    g->ground = h->kind;
    g->soundKind = h->soundKind;
    g->angle = h->slope;
    if (g->ground != 0) {
        pr2 = pr;
        pr2.d.y = h->pos.y - base.y;
        plMoveWallFeet(p, &pr2, &hitD, g);
        plMoveWallHead(p, &pr, &hitE);
        s = (hitD.t < hitE.t) ? &hitD : &hitE;
        vel->x = s->pos.x - base.x;
        vel->y = h->pos.y - base.y;
        vel->z = s->pos.z - base.z;
        g->above = 0;
        *zone = h->zone;
    } else {
        plMoveCeiling(p, &pr, &hitF);
        plMoveWallFeet(p, &pr, &hitD, g);
        if (hitF.t < 1.0f) {
            pr3 = pr;
            pr3.d.y = hitF.pos.y - p->bodyHeight - base.y;
            plMoveWallWaist(p, &pr3, &hitE);
        } else {
            plMoveWallWaist(p, &pr, &hitE);
        }
        s = (hitD.t < hitE.t) ? &hitD : &hitE;
        vel->x = s->pos.x - base.x;
        vel->z = s->pos.z - base.z;
        if (hitF.t > 1.0f) {
            vel->y = hitA.pos.y - base.y;
        } else {
            vel->y = hitF.pos.y - p->bodyHeight - base.y;
        }
        g->above = hitF.kind;
    }
    mathVecRotateY(-(heading - 1.5707963f), &vel->x, &vel->y, &vel->z);
    g->ahead = s->kind;
    if (vel->x * pr.d.x < 0.0f) {
        a.x = base.x;
        a.z = base.z;
        a.y = plMoveFeetHeight(p) + pr.y;
        b.x = vel->x - p->width * 0.5f * pr.sign.x;
        b.y = vel->y;
        b.z = 0.0f;
        mathVecRotateY(pr.heading - 1.5707963f, &b.x, &b.y, &b.z);
        if (vramRayGround(&a, &b, &at, &t) != NULL) {
            vel->x = 0.0f;
            vel->z = 0.0f;
        }
    }
}

/* The ground straight down from the step's end. */
CollTri *plMoveFloorBelow(Player *p, PlProbe *pr, PlHit *hit) {
    CollTri *t;
    s32 pad0[4];
    PlRay ray;
    s32 pad1[7];
    Vec3f d;

    vramCoursePointXZ(p->zone, pr->place, pr->side, &ray.from.x, &ray.from.z);
    ray.from.y = plMoveFeetHeight(p) + pr->y;
    hit->zone = p->zone;
    ray.dir.x = 0.0f;
    ray.dir.y = pr->d.y - plMoveFeetHeight(p);
    ray.dir.z = 0.0f;
    t = plMoveCastRay(&ray, hit);
    if (t != NULL) {
        hit->kind = t->kind;
        hit->soundKind = t->soundKind;
        hit->zone = t->zone;
        hit->slope = plMoveTriSlope(p, t, pr->heading);
    } else {
        hit->pos.y = pr->y + pr->d.y;
        hit->kind = 0;
        hit->soundKind = 0;
    }
    d = pr->d;
    mathVecRotateY(pr->heading - 1.5707963f, &d.x, &d.y, &d.z);
    hit->pos.x = ray.from.x + d.x;
    hit->pos.z = ray.from.z + d.z;
    return t;
}

/* A wall at the feet, half the player's width ahead. (The signs are named fields: as an array,
 * sign[0], IDO multiplies the sign by the half width, not the half width by the sign.) */
void plMoveWallFeet(Player *p, PlProbe *pr, PlHit *hit, PlGround *g) {
    CollTri *t;
    s32 pad[4];
    PlRay ray;
    Vec3f n;
    Vec3f n2;
    Vec3f d;

    vramCoursePointXZ(p->zone, pr->place, pr->side, &ray.from.x, &ray.from.z);
    ray.from.y = plMoveFeetHeight(p) + pr->y;
    hit->zone = p->zone;
    ray.dir.x = pr->d.x + p->width * 0.5f * pr->sign.x;
    ray.dir.y = pr->d.y;
    ray.dir.z = 0.0f;
    mathVecRotateY(pr->heading - 1.5707963f, &ray.dir.x, &ray.dir.y, &ray.dir.z);
    t = plMoveCastRay(&ray, hit);
    if (t != NULL) {
        plMoveNormalizeXZ(&ray.dir, &n);
        hit->pos.x -= p->width * 0.5f * n.x;
        hit->pos.y -= p->width * 0.5f * n.y;
        hit->pos.z -= p->width * 0.5f * n.z;
        hit->kind = t->kind;
        hit->soundKind = t->soundKind;
        hit->zone = t->zone;
        return;
    }
    if (g->ground == 0 && p->velY < -1.0714285f) {
        ray.from.y = pr->y;
    } else {
        ray.from.y = pr->y + p->bodyHeight * 0.5f;
    }
    t = plMoveCastRay(&ray, hit);
    if (t != NULL) {
        plMoveNormalizeXZ(&ray.dir, &n2);
        hit->pos.x -= p->width * 0.5f * n2.x;
        hit->pos.y -= p->width * 0.5f * n2.y;
        hit->pos.z -= p->width * 0.5f * n2.z;
        hit->kind = t->kind;
        hit->soundKind = t->soundKind;
        hit->zone = t->zone;
        return;
    }
    d = pr->d;
    mathVecRotateY(pr->heading - 1.5707963f, &d.x, &d.y, &d.z);
    hit->pos.x = ray.from.x + d.x;
    hit->pos.y = ray.from.y + d.y;
    hit->pos.z = ray.from.z + d.z;
    hit->kind = 0;
    hit->soundKind = 0;
}

/* A wall at the head (0.8 of the height up). */
void plMoveWallHead(Player *p, PlProbe *pr, PlHit *hit) {
    CollTri *t;
    s32 pad[4];
    PlRay ray;
    Vec3f n;
    Vec3f d;

    vramCoursePointXZ(p->zone, pr->place, pr->side, &ray.from.x, &ray.from.z);
    ray.from.y = pr->y + p->bodyHeight * 0.8f;
    hit->zone = p->zone;
    ray.dir.x = pr->d.x + p->width * 0.5f * pr->sign.x;
    ray.dir.y = pr->d.y + p->bodyHeight * 0.19999999f;
    ray.dir.z = 0.0f;
    mathVecRotateY(pr->heading - 1.5707963f, &ray.dir.x, &ray.dir.y, &ray.dir.z);
    t = plMoveCastRay(&ray, hit);
    if (t != NULL) {
        plMoveNormalizeXZ(&ray.dir, &n);
        hit->pos.x -= p->width * 0.5f * n.x;
        hit->pos.y -= p->width * 0.5f * n.y;
        hit->pos.z -= p->width * 0.5f * n.z;
        hit->kind = t->kind;
        hit->soundKind = t->soundKind;
        hit->zone = t->zone;
        return;
    }
    d = pr->d;
    mathVecRotateY(pr->heading - 1.5707963f, &d.x, &d.y, &d.z);
    hit->pos.x = ray.from.x + d.x;
    hit->pos.y = ray.from.y + d.y;
    hit->pos.z = ray.from.z + d.z;
    hit->kind = 0;
    hit->soundKind = 0;
}

/* A wall at the waist. (The step's half height is "/ 2.0f": IDO turns it into a multiply by 0.5
 * with its own load; "* 0.5f" shares the 0.5 already in f0.) */
void plMoveWallWaist(Player *p, PlProbe *pr, PlHit *hit) {
    CollTri *t;
    s32 pad[4];
    PlRay ray;
    Vec3f n;
    Vec3f d;

    vramCoursePointXZ(p->zone, pr->place, pr->side, &ray.from.x, &ray.from.z);
    ray.from.y = pr->y + p->bodyHeight * 0.5f;
    hit->zone = p->zone;
    ray.dir.x = pr->d.x + p->width * 0.5f * pr->sign.x;
    ray.dir.y = pr->d.y + p->bodyHeight / 2.0f;
    ray.dir.z = 0.0f;
    mathVecRotateY(pr->heading - 1.5707963f, &ray.dir.x, &ray.dir.y, &ray.dir.z);
    t = plMoveCastRay(&ray, hit);
    if (t != NULL) {
        plMoveNormalizeXZ(&ray.dir, &n);
        hit->pos.x -= p->width * 0.5f * n.x;
        hit->pos.y -= p->width * 0.5f * n.y;
        hit->pos.z -= p->width * 0.5f * n.z;
        hit->kind = t->kind;
        hit->soundKind = t->soundKind;
        hit->zone = t->zone;
        return;
    }
    d = pr->d;
    mathVecRotateY(pr->heading - 1.5707963f, &d.x, &d.y, &d.z);
    hit->pos.x = ray.from.x + d.x;
    hit->pos.y = ray.from.y + d.y;
    hit->pos.z = ray.from.z + d.z;
    hit->kind = 0;
    hit->soundKind = 0;
}

/* A ceiling: straight up from the head. */
void plMoveCeiling(Player *p, PlProbe *pr, PlHit *hit) {
    CollTri *t;
    s32 pad[4];
    PlRay ray;
    Vec3f d;

    vramCoursePointXZ(p->zone, pr->place, pr->side, &ray.from.x, &ray.from.z);
    ray.from.y = pr->y + 0.8f * p->bodyHeight;
    hit->zone = p->zone;
    ray.dir.x = 0.0f;
    ray.dir.y = pr->d.y + 0.19999999f * p->bodyHeight;
    if (p->gravity < 0.0f) {
        ray.dir.y -= p->gravity;
    }
    ray.dir.z = 0.0f;
    t = plMoveCastRay(&ray, hit);
    if (t != NULL) {
        hit->kind = t->kind;
        hit->soundKind = t->soundKind;
        hit->zone = t->zone;
    } else {
        hit->pos.y = pr->y + pr->d.y;
        hit->kind = 0;
        hit->soundKind = 0;
    }
    d = pr->d;
    mathVecRotateY(pr->heading - 1.5707963f, &d.x, &d.y, &d.z);
    hit->pos.x = ray.from.x + d.x;
    hit->pos.z = ray.from.z + d.z;
}

/* The ground 3 behind the step (the way the player faces), unless it is rising fast. (The 1s
 * of the ray are int literals: "1.0f" shares s's constant, which IDO then colours after the
 * zero; the zero is f12 because vramCoursePointXZ returns a float in f0.) */
CollTri *plMoveFloorBehind(Player *p, PlProbe *pr, PlHit *hit) {
    CollTri *t;
    s32 pad0[4];
    PlRay ray;
    s32 pad1[7];
    Vec3f d;
    f32 s;
    PlHit hit2;
    PlProbe pr2;
    f32 k;

    if (p->facing < 1.5707963f) {
        s = 1.0f;
    } else {
        s = -1.0f;
    }
    if (p->pushY + p->jumpPush > 4.0f) {
        return NULL;
    }
    vramCoursePointXZ(p->zone, pr->place - 3.0 * s, pr->side, &ray.from.x, &ray.from.z);
    ray.from.y = pr->y + 1;
    hit->zone = p->zone;
    ray.dir.x = 0.0f;
    ray.dir.y = pr->d.y - 1;
    ray.dir.z = 0.0f;
    t = plMoveCastRay(&ray, hit);
    if (t != NULL) {
        hit->kind = t->kind;
        hit->soundKind = t->soundKind;
        hit->zone = t->zone;
        hit->slope = plMoveTriSlope(p, t, pr->heading);
        k = mathSin(-hit->slope) * (s * p->width * 0.5f);
        pr2 = *pr;
        pr2.d.y += k;
        if (plMoveFloorBelow(p, &pr2, &hit2) != NULL) {
            hit->pos.y += k;
        }
    } else {
        hit->pos.y = pr->y + pr->d.y;
        hit->kind = 0;
        hit->soundKind = 0;
    }
    d.x = pr->d.x - 3.0f * s;
    d.y = pr->d.y;
    d.z = pr->d.z;
    mathVecRotateY(pr->heading - 1.5707963f, &d.x, &d.y, &d.z);
    hit->pos.x = ray.from.x + d.x;
    hit->pos.z = ray.from.z + d.z;
    return t;
}

/* The same, 3 ahead. (3.0f * s is written twice, not kept in a local: IDO's common temp spills
 * just under k, where a declared local spilled one word lower.) */
CollTri *plMoveFloorAhead(Player *p, PlProbe *pr, PlHit *hit) {
    CollTri *t;
    s32 pad0[4];
    PlRay ray;
    s32 pad1[7];
    Vec3f d;
    f32 s;
    PlHit hit2;
    PlProbe pr2;
    f32 k;

    if (p->facing < 1.5707963f) {
        s = 1.0f;
    } else {
        s = -1.0f;
    }
    if (p->pushY + p->jumpPush > 4.0f) {
        return NULL;
    }
    vramCoursePointXZ(p->zone, pr->place + 3.0f * s, pr->side, &ray.from.x, &ray.from.z);
    ray.from.y = pr->y + 1;
    hit->zone = p->zone;
    ray.dir.x = 0.0f;
    ray.dir.y = pr->d.y - 1;
    ray.dir.z = 0.0f;
    t = plMoveCastRay(&ray, hit);
    if (t != NULL) {
        hit->kind = t->kind;
        hit->soundKind = t->soundKind;
        hit->zone = t->zone;
        hit->slope = plMoveTriSlope(p, t, pr->heading);
        k = mathSin(-hit->slope) * (-s * p->width * 0.5f);
        pr2 = *pr;
        pr2.d.y += k;
        if (plMoveFloorBelow(p, &pr2, &hit2) != NULL) {
            hit->pos.y += k;
        }
    } else {
        hit->pos.y = pr->y + pr->d.y;
        hit->kind = 0;
        hit->soundKind = 0;
    }
    d.x = pr->d.x + 3.0f * s;
    d.y = pr->d.y;
    d.z = pr->d.z;
    mathVecRotateY(pr->heading - 1.5707963f, &d.x, &d.y, &d.z);
    hit->pos.x = ray.from.x + d.x;
    hit->pos.z = ray.from.z + d.z;
    return t;
}

/* A ray against the course; a triangle it meets from behind does not count. (The normal is a
 * Vec3f here: as an array, n[2], IDO loads its z before the ray's and adds that term first.) */
CollTri *plMoveCastRay(PlRay *ray, PlHit *hit) {
    CollTri *t = vramRayGround(&ray->from, &ray->dir, &hit->pos, &hit->t);

    if (t != NULL && ray->dir.x * t->n.x + ray->dir.y * t->n.y + ray->dir.z * t->n.z < 0.0f) {
        return t;
    }
    hit->t = 2.0f;
    return NULL;
}

/* Scales v so its x and z make 1 (the length is reused for its reciprocal). */
void plMoveNormalizeXZ(Vec3f *v, Vec3f *out) {
    f32 x = v->x;
    f32 z = v->z;
    f32 len = sqrtf(x * x + z * z);

    if (len == 0.0f) {
        out->x = out->y = out->z = 0.0f;
        return;
    }
    len = 1.0f / len;
    out->x = x * len;
    out->y = v->y * len;
    out->z = v->z * len;
}

/* How high the feet probe from. */
f32 plMoveFeetHeight(Player *p) {
    return 6.0f;
}

/* A triangle's slope along the course. */
f32 plMoveTriSlope(Player *p, CollTri *t, f32 heading) {
    f32 x;
    f32 y;
    f32 z;

    x = t->n.x;
    z = t->n.z;
    mathVecRotateY(-(heading - 1.5707963f), &x, &y, &z);
    return x;
}

/* The fastest along the course (keeping the direction) and the fastest fall; 12 up at most.
 * (The speed is fabsf of the field itself, not of v.) */
void plMoveClampSpeed(Player *p) {
    f32 k;
    f32 v = p->velX;
    f32 max = p->maxSpeed;
    f32 speed = fabsf(p->velX);

    if (max < speed) {
        k = max / speed;
        p->velX = v * k;
        p->velZ *= k;
    }
    if (p->velY < p->maxFall) {
        p->velY = p->maxFall;
    } else if (p->velY > 12.0f) {
        p->velY = 12.0f;
    }
}

/* Carried or swimming: the step against the course and the moving floors. */
void plMoveCollideFlyDive(Player *p, Vec3f *vel, PlGround *g) {
    PlFloor q;
    f32 heading;
    Vec3f pos;
    Vec3f d;
    Vec3f at;
    f32 t;

    heading = vramCourseHeading(p->zone, p->place);
    vramCoursePointXZ(p->zone, p->place, p->side, &pos.x, &pos.z);
    pos.y = p->height;
    d.x = vel->x;
    d.y = vel->y;
    d.z = vel->z;
    mathVecRotateY(heading - 1.5707963f, &d.x, &d.y, &d.z);
    if (vramRayGround(&pos, &d, &at, &t) != NULL) {
        d.x *= t;
        d.y *= t;
        d.z *= t;
    }
    if (plMoveFloorQuery(p, &pos, &d, &q)) {
        d.x = q.push.x;
        d.y += q.push.y;
        d.z = q.push.z;
        mathVecRotateY(-(heading - 1.5707963f), &d.x, &d.y, &d.z);
        vel->x = d.x;
        vel->y = d.y;
        vel->z = d.z;
        g->ahead = q.tri->kind;
    } else {
        g->ahead = 0;
    }
    g->edgeFlags = 0;
    g->soundKind = 0;
    g->above = 0;
    g->ground = g->ahead;
    g->angle = 0.0f;
}

CollTri *plMoveFloorQuery(Player *p, Vec3f *from, Vec3f *dir, PlFloor *out) {
    PlFloorBuf buf = gPlFloorBufTemplate;
    PlFloor q;

    q.from.x = from->x;
    q.from.y = from->y;
    q.from.z = from->z;
    q.dir.x = dir->x;
    q.dir.y = dir->y;
    q.dir.z = dir->z;
    q.buf = buf.s;
    q.tri = fhSweepSpheres(&q);
    *out = q;
    return out->tri;
}

/* On a swing: the step against the course and the moving floors. (Statement order sets the
 * float registers here: pos.x before d.y's zero, and the push in x, y, z order.) */
void plMoveCollideLetGo(Player *p, Vec3f *vel, PlGround *g) {
    PlFloor q;
    CollTri *tri;
    f32 heading;
    Vec3f pos;
    Vec3f d;
    Vec3f at;
    f32 t;
    s32 pad;
    s32 up;

    up = 0;
    heading = vramCourseHeading(p->zone, p->place);
    vramCoursePointXZ(p->zone, p->place, p->side, &pos.x, &pos.z);
    pos.y = p->height;
    d.x = vel->x;
    d.y = vel->y;
    d.z = vel->z;
    mathVecRotateY(heading - 1.5707963f, &d.x, &d.y, &d.z);
    g->ground = 0;
    g->above = 0;
    tri = vramRayGround(&pos, &d, &at, &t);
    if (tri != NULL) {
        plVecScaleSelf(&d, t);
        if (d.y > 0.0f) {
            p->place += d.y;
            pos.x = p->place;
            d.y = 0.0f;
            g->above = tri->kind;
        } else {
            g->ground = tri->kind;
        }
        vel->x = d.x;
        vel->y = d.y;
        vel->z = d.z;
    }
    if (plMoveFloorQuery(p, &pos, &d, &q)) {
        if (d.y > 0.0f) {
            up = 1;
        }
        d.x = q.push.x;
        d.y += q.push.y;
        d.z = q.push.z;
        mathVecRotateY(-(heading - 1.5707963f), &d.x, &d.y, &d.z);
        vel->x = d.x;
        vel->y = d.y;
        vel->z = d.z;
        g->ahead = q.tri->kind;
        if (up == 0) {
            g->ground = g->ahead;
        }
    } else {
        g->ahead = 0;
    }
    g->edgeFlags = 0;
    g->soundKind = 0;
    g->angle = 0.0f;
}

/* The walls: the step is pushed out of whatever it walks into, and an object it walks onto
 * is told (message 6). (n is declared up with d: IDO keeps 8 bytes of its own under msg, so a
 * last-declared n grew the frame to 0xB0.) */
void plMoveRideCollide(Player *p, Vec3f *vel, PlGround *g) {
    Vec3f to;
    Vec3f from;
    Vec3f d;
    s32 n;
    s32 pad[2];
    PlWall c;
    s32 hit;
    Vec3f r;
    PlPlace msg;

    g->ground = 0;
    p->wallObj = 0;
    vramCourseDirection(p->zone, p->place, &c.n);
    plMoveNormalizeXZ(&c.n, &c.n);
    c.g = g;
    c.p = p;
    c.obj = 0;
    vramCoursePointXZ(p->zone, p->place, p->side, &from.x, &from.z);
    from.y = p->height;
    vramCoursePointXZ(p->zone, p->place + vel->x, p->side + vel->z, &to.x, &to.z);
    to.y = p->height + vel->y;
    d.x = to.x - from.x;
    d.y = to.y - from.y;
    d.z = to.z - from.z;
    hit = plMoveRideAlong(&c, &to, &d);
    if (p->ground == 0) {
        hit |= plMoveRideAlongDown(&c, &to, &d);
    }
    hit |= plMoveRideHead(&c, &to);
    hit |= plMoveRideWallAhead(&c, &to, vel->x);
    hit |= plMoveRideWallBehind(&c, &to, vel->x);
    if (d.y < 0.0f) {
        n = plMoveRideGround(&c, &to, vel->x, 0.0f);
        hit |= n;
        if (n == 0) {
            n = plMoveRideGround(&c, &to, vel->x, -1.0f);
            hit |= n;
        }
        if (n == 0) {
            hit |= plMoveRideGround(&c, &to, vel->x, 1.0f);
        }
    }
    hit |= plMoveRidePushOut(&c, &to);
    if (hit) {
        r.x = to.x - from.x;
        r.y = to.y - from.y;
        r.z = to.z - from.z;
        mathVecRotateY(-(mathAtan2(c.n.x, c.n.z) - 1.5707964f), &r.x, &r.y, &r.z);
        vel->x = r.x;
        vel->y = r.y;
        if (c.obj != 0) {
            msg.zone = p->zone;
            msg.place = p->place + vel->x;
            msg.side = p->side + vel->z;
            msg.height = p->height + vel->y;
            tskSendMsg((Obj *) c.obj, 6, &msg);
            vel->x = msg.place - p->place;
            vel->z = msg.side - p->side;
            vel->y = msg.height - p->height;
        }
    }
}

s32 plMoveRidePushOut(PlWall *c, Vec3f *pos) {
    Vec3f from;
    Vec3f push;
    s32 pad[2];

    from.x = pos->x;
    from.y = pos->y + c->p->bodyHeight * 0.3f;
    from.z = pos->z;
    if (ridePointIn(&from, &push, 0, pos)) {
        pos->x += push.x;
        pos->y += push.y;
        pos->z += push.z;
        return 1;
    }
    return 0;
}

/* The ground's objects under the step (side -1 and 1 look to either side). The multiplier 10 is
 * an int (one register for x and z) and the height's 10.0f a float (loaded again). */
s32 plMoveRideGround(PlWall *c, Vec3f *pos, f32 vx, f32 side) {
    Vec3f from;
    Vec3f dir;
    Vec3f out;
    s32 obj;

    from.x = pos->x + side * c->n.x * 0.3f * 10;
    from.y = pos->y + 10.0f;
    from.z = pos->z + side * c->n.z * 0.3f * 10;
    dir.x = 0.0f;
    dir.z = 0.0f;
    dir.y = -10.0f - fabsf(vx);
    obj = rideMoveStop(&from, &dir, &out, 0);
    if (obj != 0) {
        c->obj = obj;
        c->g->ground = 1;
        c->g->soundKind = rideAskMsg7(obj);
        pos->y = from.y + out.y;
        return 1;
    }
    return 0;
}

/* The wall ahead (half the width along the wall's normal). off is a zero margin the game keeps in
 * a local: its subtraction survives (a literal "- 0.0f" folds away). */
s32 plMoveRideWallAhead(PlWall *c, Vec3f *pos, f32 vx) {
    Vec3f from;
    Vec3f dir;
    Vec3f out;
    s32 obj;
    f32 off;
    s32 pad[3];

    off = 0.0f;
    from.x = pos->x;
    from.y = pos->y + 10.0f;
    from.z = pos->z;
    dir.x = c->n.x * c->p->width * 0.5f;
    dir.y = 0.0f;
    dir.z = c->n.z * c->p->width * 0.5f;
    obj = rideMoveStop(&from, &dir, &out, 0);
    if (obj != 0) {
        if (vx > 0.0f) {
            c->p->wallObj = obj;
        }
        pos->x = from.x - dir.x + out.x - off;
        pos->z = from.z - dir.z + out.z - off;
        return 1;
    }
    return 0;
}

/* The wall behind (the same, against the normal). */
s32 plMoveRideWallBehind(PlWall *c, Vec3f *pos, f32 vx) {
    Vec3f from;
    Vec3f dir;
    Vec3f out;
    s32 obj;
    f32 off;
    s32 pad[3];

    off = 0.0f;
    from.x = pos->x;
    from.y = pos->y + 10.0f;
    from.z = pos->z;
    dir.x = -c->n.x * c->p->width * 0.5f;
    dir.y = 0.0f;
    dir.z = -c->n.z * c->p->width * 0.5f;
    obj = rideMoveStop(&from, &dir, &out, 0);
    if (obj != 0) {
        if (vx < 0.0f) {
            c->p->wallObj = obj;
        }
        pos->x = from.x - dir.x + out.x - off;
        pos->z = from.z - dir.z + out.z - off;
        return 1;
    }
    return 0;
}

s32 plMoveRideHead(PlWall *c, Vec3f *pos) {
    Vec3f from;
    Vec3f dir;
    Vec3f out;
    f32 half = c->p->bodyHeight * 0.5f;
    s32 pad[2];

    from.x = pos->x;
    from.y = pos->y + half;
    from.z = pos->z;
    dir.x = 0.0f;
    dir.y = c->p->bodyHeight - half;
    dir.z = 0.0f;
    if (rideMoveStop(&from, &dir, &out, 0)) {
        pos->x = from.x + out.x;
        pos->y = from.y + out.y - c->p->bodyHeight;
        pos->z = from.z + out.z;
        return 1;
    }
    return 0;
}

s32 plMoveRideAlong(PlWall *c, Vec3f *pos, Vec3f *d) {
    Vec3f from;
    Vec3f dir;
    Vec3f out;
    s32 pad;
    f32 half;
    s32 obj;
    s32 pad2[2];

    half = c->p->bodyHeight * 0.5f;
    from.x = pos->x;
    from.y = pos->y + half;
    from.z = pos->z;
    plMoveNormalizeXZ(d, &dir);
    dir.x *= c->p->width * 0.5f * 0.8f;
    dir.y = (c->p->bodyHeight - half) * 0.8f;
    dir.z *= c->p->width * 0.5f * 0.8f;
    obj = rideMoveOnto(&from, &dir, &out, 0);
    if (obj != 0) {
        c->p->wallObj = obj;
        pos->x = from.x + out.x - dir.x;
        pos->y = from.y + out.y - c->p->bodyHeight;
        pos->z = from.z + out.z - dir.z;
        return 1;
    }
    return 0;
}

s32 plMoveRideAlongDown(PlWall *c, Vec3f *pos, Vec3f *d) {
    Vec3f from;
    Vec3f dir;
    Vec3f out;
    s32 pad;
    f32 half;
    s32 pad2[3];

    half = c->p->bodyHeight * 0.5f;
    from.x = pos->x;
    from.y = pos->y + half;
    from.z = pos->z;
    plMoveNormalizeXZ(d, &dir);
    dir.x *= c->p->width * 0.5f * 0.8f;
    dir.y = -half * 0.8f * 0.8f;
    dir.z *= c->p->width * 0.5f * 0.8f;
    if (rideMoveOnto(&from, &dir, &out, 0)) {
        pos->x = from.x + out.x - dir.x;
        pos->z = from.z + out.z - dir.z;
        return 1;
    }
    return 0;
}

/* The water's object moves the step (message 6). */
void plMoveWaterCarry(Player *p, Vec3f *vel) {
    Vec3f pos;
    s32 pad;
    PlPlace msg;

    pos.x = p->model->pos[0];
    pos.y = p->model->pos[1];
    pos.z = p->model->pos[2];
    p->waterObj = rideTree2PointIn(&pos, (Vec3f *) p->toSurface);
    if (p->waterObj != 0) {
        msg.zone = p->zone;
        msg.place = p->place + vel->x;
        msg.side = p->side + vel->z;
        msg.height = p->height + vel->y;
        tskSendMsg((Obj *) p->waterObj, 6, &msg);
        vel->x = msg.place - p->place;
        vel->z = msg.side - p->side;
        vel->y = msg.height - p->height;
    }
}

/* In water: the pull down, the drag and how deep (the surface bobs with the frame count).
 * The drag is set once and copied (p->dragY = p->drag): a chained assignment costs the
 * frame an 8-byte temporary. */
void plMoveWater(Player *p) {
    Vec3f pos;
    f32 k;
    f32 bob;
    f32 c;

    pos.x = p->model->pos[0];
    pos.y = p->model->pos[1];
    pos.z = p->model->pos[2];
    p->prevWaterObj = p->waterObj;
    p->waterObj = rideTree2PointIn(&pos, (Vec3f *) p->toSurface);
    if (p->waterObj != 0) {
        if (p->swimDepth < p->toSurface[1]) {
            k = 1.0f;
        } else {
            k = p->toSurface[1] / p->swimDepth;
        }
        bob = mathSinCos((((s32) p->place & 0x3FF) + gGameFrameCount) * 24, &c) * 0.2f;
        p->prevWaterDepth = p->waterDepth;
        p->waterDepth = p->toSurface[1];
        p->surfacePos[0] = p->toSurface[0] + pos.x;
        p->surfacePos[1] = p->toSurface[1] + pos.y;
        p->surfacePos[2] = p->toSurface[2] + pos.z;
        if (p->flags & 0x10000) {
            if (k > 0.5f) {
                p->gravity = k * bob * 0.25f;
            } else {
                p->gravity = (bob * 0.25f + 0.5769231f) * k + (1.0f - k) * p->baseGravity;
            }
            if (!(p->flags & 8)) {
                p->drag = k * 0.93f + (1.0f - k) * p->baseDrag;
                p->dragY = p->drag;
            }
        } else {
            p->gravity = k * 0.5769231f + (1.0f - k) * (p->baseGravity + bob);
            if (!(p->flags & 8)) {
                p->drag = k * 0.3f + (1.0f - k) * p->baseDrag;
                p->dragY = k * 0.725f + (1.0f - k);
            }
        }
    } else {
        p->gravity = p->baseGravity;
        if (!(p->flags & 8)) {
            p->dragY = 1.0f;
            p->drag = p->baseDrag;
        }
        p->waterDepth = 0.0f;
    }
    if (p->flags2 & 2) {
        p->gravity = 0.0f;
    }
}

/* The players are tied: a step that takes this one further than the camera's reach from
 * another is refused. */
s32 plMoveTiedXZ(Player *p, Vec3f *vel) {
    f32 reach = fadeViewWidth(gGameWork.view);
    f32 heading = vramCourseHeading(p->zone, p->place);
    s32 i;
    Player *o;
    f32 dx;
    f32 dz;
    f32 d0;
    f32 d1;

    if (p->flags2 & 4) {
        return 0;
    }
    dx = vel->x;
    dz = vel->z;
    if (!(p->flags & 0x10)) {
        mathVecRotateY(heading - 1.5707963f, &dx, NULL, &dz);
        if (p->flags & 0x820) {
            return 0;
        }
    }
    for (i = 0; i < 4; i++) {
        if (i != p->index && plIsInPlay(i)) {
            o = PL(gGameWork.players[i]);
            if (!(o->flags2 & 4)) {
                d0 = plMoveDistXZ(o->model, p->model, 0.0f, 0.0f);
                d1 = plMoveDistXZ(o->model, p->model, dx, dz);
                if (d0 < d1 && reach <= d1) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

/* How far apart two models are, across the ground, with b moved by (dx, dz). (The z term is
 * written first: IDO computes the sum's right operand, x, first.) */
f32 plMoveDistXZ(AnimeFrame *a, AnimeFrame *b, f32 dx, f32 dz) {
    return sqrtf((a->pos[2] - (b->pos[2] + dz)) * (a->pos[2] - (b->pos[2] + dz)) +
                 (a->pos[0] - (b->pos[0] + dx)) * (a->pos[0] - (b->pos[0] + dx)));
}

/* The same up and down (absolute: either way). (The skip is a continue: written as an if
 * around the body, IDO swaps the saved registers of the constants 1 and 4.) */
s32 plMoveTiedY(Player *p, f32 dy, f32 scale, s32 absolute) {
    s32 i;
    f32 reach = fadeViewWidth(gGameWork.view) * scale;
    Player *o;
    f32 d0;
    f32 d1;

    for (i = 0; i < 4; i++) {
        if (i == p->index || !plIsInPlay(i)) {
            continue;
        }
        o = PL(gGameWork.players[i]);
        if (!(o->flags2 & 4)) {
            d0 = p->height - o->height;
            d1 = p->height + dy - o->height;
            if (absolute == 1) {
                d0 = fabsf(d0);
                d1 = fabsf(d1);
            }
            if (d0 < d1 && reach <= d1) {
                return 1;
            }
        }
    }
    return 0;
}

/* The stage's objects push the step. */
void plMoveObjPush(Player *p, Vec3f *vel) {
    Obj *o;

    if (gEneManager != NULL) {
        for (o = gEneManager->child->next->next->next->child; o != NULL; o = o->next) {
            personPlCollCheck(o, p, vel);
        }
    }
}
