#include "tlb.h"

/* Impact's ball and cube ("impact ball", "impact cube"): things on the course the player can
 * push (and pull, the cube), that roll or tip over the ground and the rides, carry the player
 * (message 6) and hit both ways. */

/* The ball: an enemy's work (EneObj) to 0x34, then its own. */
typedef struct ImpactBallWork {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit; /* 1 on the ground */
    /* 0x0B */ s8 hp;
    /* 0x0C */ Frame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ MPolFrame *ball; /* the ball drawn, turned by roll */
    /* 0x38 */ MPolFrame *shadow;
    /* 0x3C */ PolFrame *pol; /* where it is: the ride and the boxes hang on it */
    /* 0x40 */ RideFrame *ride;
    /* 0x44 */ ACollFrame *atk;
    /* 0x48 */ ACollFrame *dmg;
    /* 0x4C */ CollTri *floor;
    /* 0x50 */ u8 rolling; /* its rolling sound is on */
    /* 0x54 */ f32 dustRoll; /* roll at the last dust puff */
    /* 0x58 */ f32 roll;
    /* 0x5C */ f32 moved; /* along the course this step */
} ImpactBallWork;

/* The cube. */
typedef struct ImpactCubeWork {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit; /* 1 on the ground */
    /* 0x0B */ s8 hp;
    /* 0x0C */ Frame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ ACollFrame *atk;
    /* 0x40 */ ACollFrame *dmg;
    /* 0x44 */ f32 moved; /* along the course this step */
    /* 0x48 */ f32 dustX; /* x at the last scrape */
    /* 0x4C */ u8 sliding; /* its scrape sounded */
} ImpactCubeWork;

/* Message 6's argument (plmove.c's PlPlace): a place on the course, moved by the object. */
typedef struct PlPlace {
    /* 0x0 */ s16 zone;
    /* 0x4 */ f32 place;
    /* 0x8 */ f32 side;
    /* 0xC */ f32 height;
} PlPlace;

/* acollAtkAddYZ and acollDmgAddXYZ return their box (main declares them void). */
typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz,
                                    s32 a);
typedef ACollFrame *(*DmgAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height,
                                     s16 ox, s16 oy, s16 oz, s32 a);

#define guMtxXFMF (*guMtxXFMF)

Gfx impObjBallDl[] = {
    gsDPPipeSync(),
    gsDPSetEnvColor(0x00, 0x00, 0x00, 0xFF),
    gsSPEndDisplayList(),
    gsDPNoOp(),
};
extern Gfx impObjBallModelDl[];
extern u8 impObjBallRide[];
extern Gfx impObjCubeDl[];
extern u8 impObjCubeRide[];

CollTri *impObjLand(Obj *obj, f32 x, f32 y, f32 z, f32 h, f32 *vy);
f32 impObjBallMove(Obj *obj, f32 r);
s32 impObjCubeMove(Obj *obj, ImpactCubeWork *w);

/* A dark grey sparkle shrinking a little (the dust they raise). */
void impObjDust(f32 x, f32 y, f32 z, f32 size) {
    ItexInit init;

    init = gItexTemplate;
    init.file = 0x123;
    init.frames = gItexSparkleAnim;
    init.flags = 3;
    init.env.r = init.env.g = init.env.b = 0x80;
    init.prim.r = init.prim.g = init.prim.b = 0x40;
    init.pos[0] = x;
    init.pos[1] = y;
    init.grow.x = init.grow.y = init.grow.z = 0.9900000095f;
    init.pos[2] = z;
    init.scale[2] = size;
    init.scale[1] = size;
    init.scale[0] = size;
    tskCreate(gEffectTask, itex, 0, &init);
}

/* "impact ball" */
s32 impObjBall(Obj *obj, s16 msg, void *arg) {
    ImpactBallWork *w = (ImpactBallWork *) FX(obj);
    s32 pad[3];

    switch (msg) {
        case 2:
            obj->name = "impact ball";
            eneSetup(obj, arg);
            w->pol = eneNewPol(obj, NULL);
            w->ball = (MPolFrame *) frmCreate(gFrmModelLayer, gFrmKindMpol);
            if (w->ball == NULL || w->pol == NULL) {
                return -1;
            }
            w->ball->dl2 = impObjBallModelDl;
            w->ball->dl = (Gfx *) fadeSegToRam(impObjBallDl, obj->file);
            eneGetSegs(obj, w->ball->segs);
            w->ride = rideCreate(gRideTree1, (s32) obj, &w->pol->f,
                                 (RideData *) eneFileAddr(obj, impObjBallRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            w->shadow = shadowCreate();
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(pl_atk_root_frame, &w->pol->f, 100, 110, 0, 0);
            if (w->atk != NULL) {
                w->atk->kind = 0x7D;
            }
            w->dmg = ((DmgAddXYZFunc) acollDmgAddXYZ)(uda_dmg_root_frame, &w->pol->f, 100, 200, 0, 0, 0, 0);
            w->dustRoll = w->roll = 0.0f;
            break;
        case 1:
            sndPlay(0x51C);
            frmDestroy(&w->shadow->f);
            frmDestroy(&w->dmg->frame);
            frmDestroy(&w->atk->frame);
            frmDestroy(&w->ride->f);
            frmDestroy(&w->ball->f);
            frmDestroy(&w->pol->f);
            break;
        case 0:
            w->rot[1] = vramCourseHeading(w->zone, w->x) - 1.570796371f;
            eneFall(obj, 0.4f);
            w->moved = impObjBallMove(obj, 12.5f);
            w->roll -= w->moved / 12.5f;
            if (w->hit & 1) {
                if (fabsf(w->vel[1]) > 3.0f) {
                    soundLocationCam(0x23E, w->pol->pos[0], w->pol->pos[1], w->pol->pos[2]);
                }
                w->vel[0] /= 2;
                w->vel[1] = 0.0f;
            }
            eneSetModelOnCourse(obj, &w->pol->f);
            if (w->hit & 1) {
                if (fabsf(w->roll - w->dustRoll) > 0.979199951171875083 ||
                    (w->rolling == 0 && fabsf(w->vel[0]) >= 0.009999999776f)) {
                    if (w->rolling == 0) {
                        sndPlay(0x2B1);
                        w->rolling = 1;
                    }
                    w->dustRoll = w->roll;
                    impObjDust(w->pol->pos[0], w->pol->pos[1], w->pol->pos[2], 0.05f);
                }
            }
            if (fabsf(w->vel[0]) < 0.009999999776f) {
                if (w->rolling == 1) {
                    sndPlay(0x51C);
                    w->rolling = 0;
                }
            }
            mathMtxIdent(w->ball->mtx.m);
            mathMtxPostRotZ(w->ball->mtx.m, w->roll);
            mathMtxPostRotY(w->ball->mtx.m, w->pol->rot[1]);
            mathMtxPostTrans(w->ball->mtx.m, w->pol->pos[0], w->pol->pos[1] + 12.5f, w->pol->pos[2]);
            mathMtxScaleRotX(w->ball->mtx.m, w->pol->scale[0]);
            mathMtxScaleRotY(w->ball->mtx.m, w->pol->scale[1]);
            mathMtxScaleRotZ(w->ball->mtx.m, w->pol->scale[2]);
            if (w->shadow != NULL) {
                shadowPut(w->shadow, w->pol->pos[0], w->pol->pos[1], w->pol->pos[2], 3.0f, w->floor);
            }
            if (w->atk != NULL) {
                if (w->vel[1] * w->vel[1] + w->vel[0] * w->vel[0] > 0.0f) {
                    w->atk->flags |= 2;
                } else {
                    w->atk->flags &= ~2;
                }
                if (acollAtkWasHit(w->atk) == 1 && (Obj *) w->atk->hitPower != NULL) {
                    tskSendMsg((Obj *) w->atk->hitPower, 7, obj);
                }
            }
            break;
        case 6:
            if (arg != NULL) {
                ((PlPlace *) arg)->place += w->moved * 2;
            }
            break;
        case 9:
            switch (((s32 *) arg)[0]) {
                case 0x70757368:
                    w->vel[0] = ((s32 *) arg)[1] * 0.009999999776f * 0.3000000119f;
                    return 0x70757368;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Whether a point h over (x, y, z), falling by *vy, lands on a ride or the ground: if so, *vy is
 * cut to land on it and the work is marked on the ground; the ground's triangle may move it to
 * another course. Returns the ground's triangle. */
CollTri *impObjLand(Obj *obj, f32 x, f32 y, f32 z, f32 h, f32 *vy) {
    ImpactBallWork *w = (ImpactBallWork *) FX(obj);
    s32 pad;
    Vec3f dir;
    Vec3f from;
    Vec3f out;
    f32 t;
    CollTri *tri;

    dir.x = 0.0f;
    dir.y = *vy + -h;
    dir.z = 0.0f;
    from.x = x;
    from.y = y + h;
    from.z = z;
    if (rideMoveStop(&from, &dir, &out, (s32) obj)) {
        w->hit |= 1;
        *vy = out.y + h;
    }
    dir.y = *vy + -h;
    tri = vramRayGround(&from, &dir, &out, &t);
    if (tri != NULL) {
        w->hit |= 1;
        *vy = out.y - (from.y - h);
        if (w->zone != 0 && tri->zone != 0 && w->zone != tri->zone) {
            w->zone = tri->zone;
            w->x = vramNearestPlaceXZ(w->zone, out.x, out.z, &t);
        }
    }
    return tri;
}

/* The ball's move: its speed turned to the course's heading, stopped by rides and the ground
 * ahead, then dropped onto them at three points; returns the distance moved along the course. */
f32 impObjBallMove(Obj *obj, f32 r) {
    ImpactBallWork *w = (ImpactBallWork *) FX(obj);
    CollTri *f;
    Vec3f vel;
    Vec3f pos;
    Vec3f from;
    Vec3f dir;
    Vec3f d;
    s32 pad[9];
    Vec3f out;
    f32 t;
    s32 pad2[4];
    f32 cx;
    f32 cz;
    f32 s;
    f32 c;

    w->hit &= ~1;
    vramCoursePointXZ(w->zone, w->x, w->z, &pos.x, &pos.z);
    pos.y = w->y;
    vel = *(Vec3f *) w->vel;
    mathVecRotateY(w->rot[1], &vel.x, &vel.y, &vel.z);
    dir = vel;
    dir.y = 0.0f;
    mathVecNormalizeB(&dir);
    d.x = dir.x * r + vel.x;
    d.y = 0.0f;
    d.z = dir.z * r + vel.z;
    from = pos;
    from.y += r;
    if (rideMoveStop(&from, &d, &out, (s32) obj)) {
        vel.x = out.x - (d.x - vel.x);
        vel.z = out.z - (d.z - vel.z);
        w->vel[0] = 0.0f;
    }
    d.x = dir.x * r + vel.x;
    d.y = 0.0f;
    d.z = dir.z * r + vel.z;
    from = pos;
    from.y += r;
    if (vramRayGround(&from, &d, &out, &t)) {
        vel.x = out.x - (from.x + d.x - vel.x);
        vel.z = out.z - (from.z + d.z - vel.z);
        w->vel[0] = 0.0f;
    }
    if (vel.y <= 0.0f) {
        cx = pos.x + vel.x;
        cz = pos.z + vel.z;
        s = mathSinCos((s16) (w->rot[1] / 3.141592741f * 512.0f), &c);
        w->floor = NULL;
        if ((f = impObjLand(obj, 6.0f * c + cx, pos.y + 1.0f, 6.0f * s + cz, r, &vel.y)) != NULL) {
            w->floor = f;
        }
        if ((f = impObjLand(obj, cx - 6.0f * c, pos.y + 1.0f, cz - 6.0f * s, r, &vel.y)) != NULL) {
            w->floor = f;
        }
        if ((f = impObjLand(obj, cx, pos.y, cz, r, &vel.y)) != NULL) {
            w->floor = f;
        }
    }
    mathVecRotateY(-w->rot[1], &vel.x, &vel.y, &vel.z);
    w->x += vel.x;
    w->y += vel.y;
    return vel.x;
}

/* "impact cube" */
s32 impObjCube(Obj *obj, s16 msg, void *arg) {
    ImpactCubeWork *w = (ImpactCubeWork *) FX(obj);
    s32 pad[2];
    f32 sx;
    f32 sz;
    f32 x;
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "impact cube";
            eneSetup(obj, arg);
            w->pol = eneNewPol(obj, impObjCubeDl);
            w->ride = rideCreate(gRideTree1, (s32) obj, &w->pol->f,
                                 (RideData *) eneFileAddr(obj, impObjCubeRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(pl_atk_root_frame, &w->pol->f, 60, 0, 0, 0);
            if (w->atk != NULL) {
                w->atk->kind = 0x7D;
            }
            w->dmg = ((DmgAddXYZFunc) acollDmgAddXYZ)(uda_dmg_root_frame, &w->pol->f, 100, 200, 0, 0, 0, 0);
            w->dustX = w->x;
            break;
        case 1:
            frmDestroy(&w->dmg->frame);
            frmDestroy(&w->atk->frame);
            frmDestroy(&w->pol->f);
            frmDestroy(&w->ride->f);
            break;
        case 6:
            if (arg != NULL) {
                ((PlPlace *) arg)->place += w->moved;
            }
            break;
        case 9:
            switch (((s32 *) arg)[0]) {
                case 0x70757368:
                    w->vel[0] = ((s32 *) arg)[1] * 0.009999999776f * 0.3000000119f;
                    return 0x70757368;
                case 0x70756C6C:
                    w->vel[0] = ((s32 *) arg)[1] * 0.009999999776f * 0.3000000119f;
                    return 0x70756C6C;
            }
            break;
        case 0:
            if (w->hit & 1) {
                if (fabsf(w->x - w->dustX) > 15.29999924f || (w->sliding == 0 && w->vel[0] != 0.0f)) {
                    soundIncamCam(0x234, w->pol->pos[0], w->pol->pos[1], w->pol->pos[2]);
                    w->sliding = 1;
                    w->dustX = w->x;
                    if (w->vel[0] < 0.0f) {
                        x = w->x + 9.0f;
                    } else if (w->vel[0] > 0.0f) {
                        x = w->x - 9.0f;
                    } else {
                        x = w->x;
                    }
                    vramCoursePointXZ(w->zone, x, w->z, &sx, &sz);
                    impObjDust(sx, w->pol->pos[1] - 10.0f, sz, 0.05f);
                }
            }
            if (w->vel[0] == 0.0f) {
                w->sliding = 0;
            }
            if (w->atk != NULL) {
                if (w->vel[1] * w->vel[1] + w->vel[0] * w->vel[0] > 0.0f) {
                    w->atk->flags |= 2;
                } else {
                    w->atk->flags &= ~2;
                }
                if (acollAtkWasHit(w->atk) == 1 && (Obj *) w->atk->hitPower != NULL) {
                    tskSendMsg((Obj *) w->atk->hitPower, 7, obj);
                }
            }
            w->rot[1] = vramCourseHeading(w->zone, w->x);
            w->vel[1] += -0.4;
            w->moved = w->x;
            impObjCubeMove(obj, w);
            w->moved = w->x - w->moved;
            if (w->hit & 1) {
                w->vel[1] = w->vel[0] = 0.0f;
            }
            eneSetModelOnCourse(obj, &w->pol->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The cube's move: as the ball's, then it settles on three of its corners and tips over toward
 * the ground (a quarter turn at a time), sliding back by what the tip takes. */
s32 impObjCubeMove(Obj *obj, ImpactCubeWork *w) {
    Vec3f vel;
    Vec3f pos;
    Vec3f from;
    Vec3f dir;
    Vec3f d;
    Vec3f c[3];
    Vec3f out;
    f32 m[4][4];
    f32 t;
    s32 n;
    s32 i;
    u8 hit[3];
    f32 snap;
    f32 old;
    f32 y;

    vramCoursePointXZ(w->zone, w->x, w->z, &pos.x, &pos.z);
    pos.y = w->y;
    vel = *(Vec3f *) w->vel;
    mathVecRotateY(w->rot[1] - 1.570796371f, &vel.x, &vel.y, &vel.z);
    dir = vel;
    dir.y = 0.0f;
    mathVecNormalizeB(&dir);
    d.x = dir.x * 10.1 + vel.x;
    d.y = 0.0f;
    d.z = dir.z * 10.1 + vel.z;
    from = pos;
    from.y -= 8.0f;
    if (rideMoveStop(&from, &d, &out, (s32) obj)) {
        vel.x = out.x - (d.x - vel.x);
        vel.z = out.z - (d.z - vel.z);
    }
    d.x = dir.x * 10.10000038f + vel.x;
    d.z = dir.z * 10.10000038f + vel.z;
    from = pos;
    from.y += 8.0f;
    if (rideMoveStop(&from, &d, &out, (s32) obj)) {
        vel.x = out.x - (d.x - vel.x);
        vel.z = out.z - (d.z - vel.z);
    }
    d.x = dir.x * 10.10000038f + vel.x;
    d.y = -8.0f;
    d.z = dir.z * 10.10000038f + vel.z;
    from = pos;
    if (vramRayGround(&from, &d, &out, &t)) {
        vel.x = out.x - (from.x + d.x - vel.x);
        vel.z = out.z - (from.z + d.z - vel.z);
    }
    if (w->rot[0] < 0.0f) {
        w->rot[0] += 6.283185482f;
    }
    if (w->rot[0] > 5.497787476f) {
        snap = 0.0f;
    } else if (w->rot[0] > 3.926990986f) {
        w->rot[0] -= 4.712388992f;
        snap = 4.712388992f;
    } else if (w->rot[0] > 2.356194496f) {
        w->rot[0] -= 3.141592741f;
        snap = 3.141592741f;
    } else if (w->rot[0] > 0.7853981853f) {
        w->rot[0] -= 1.570796371f;
        snap = 1.570796371f;
    } else {
        snap = 0.0f;
    }
    mathMtxIdent(m);
    mathMtxPostRotX(m, -w->rot[0]);
    mathMtxPostRotY(m, w->rot[1]);
    mathMtxPostTrans(m, pos.x, pos.y, pos.z);
    guMtxXFMF(m, 0.0f, -10.0f, 10.0f, &c[0].x, &c[0].y, &c[0].z);
    guMtxXFMF(m, 0.0f, -10.0f, -10.0f, &c[1].x, &c[1].y, &c[1].z);
    guMtxXFMF(m, 0.0f, -10.0f, 0.0f, &c[2].x, &c[2].y, &c[2].z);
    n = 0;
    for (i = 0; i < 3; i++) {
        y = vel.y;
        impObjLand(obj, c[i].x, c[i].y, c[i].z, 10.0f, &y);
        if (y != vel.y) {
            n++;
            hit[i] = 1;
        } else {
            hit[i] = 0;
        }
        c[i].y += y;
    }
    if (n >= 2) {
        w->hit |= 1;
    } else {
        w->hit &= ~1;
    }
    mathMtxIdent(m);
    mathMtxPostTrans(m, -pos.x, 0.0f, -pos.z);
    mathMtxPostRotY(m, -w->rot[1]);
    guMtxXFMF(m, c[0].x, c[0].y, c[0].z, &c[0].x, &c[0].y, &c[0].z);
    guMtxXFMF(m, c[1].x, c[1].y, c[1].z, &c[1].x, &c[1].y, &c[1].z);
    guMtxXFMF(m, c[2].x, c[2].y, c[2].z, &c[2].x, &c[2].y, &c[2].z);
    old = w->rot[0];
    if (n == 2 && hit[2] != 0) {
        if (hit[0] != 0) {
            w->rot[0] = -mathAtan2(c[0].y - c[2].y, c[0].z - c[2].z);
        } else {
            w->rot[0] = -mathAtan2(c[2].y - c[1].y, c[2].z - c[1].z);
        }
    } else {
        w->rot[0] = -mathAtan2(c[0].y - c[1].y, c[0].z - c[1].z);
    }
    old -= w->rot[0];
    if (old > 3.141592741f) {
        old -= 6.283185482f;
    } else if (old < -3.141592741f) {
        old += 6.283185482f;
    }
    w->rot[0] += snap;
    mathVecRotateY(-(w->rot[1] - 1.570796371f), &vel.x, &vel.y, &vel.z);
    w->x += vel.x;
    w->x -= old * fabsf(c[0].z - c[1].z) / 2;
    if (n == 2 && hit[0] != 0 && hit[1] != 0) {
        w->y = ((c[0].y + c[1].y) + fabsf(c[0].z - c[1].z)) / 2;
    } else {
        w->y = c[2].y + fabsf(c[0].z - c[1].z) / 2;
    }
    return 0;
}
