#include "tlb.h"

/* The "manji" object kind (archive file 181): a swastika-shaped block that can be ridden (stood
 * on), falls and slides along the course; a hit of attack kind 0x19 or 0x1A tells its attacker
 * "卍fx"; told "expl" it bursts into six "卍pi" pieces (kind 0x23E, its second task here). */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define DMG_ADD_XYZ ((ACollFrame *(*)(ACollFrame *, Frame *, u16, u16, s16, s16, s16, s32)) acollDmgAddXYZ)

extern Gfx manjiDl[];
extern u8 manjiPieceDl0[];
extern u8 manjiPieceDl1[];
RideData manjiRideData[] = { { 1, 424.26407f, { 0 }, { { 0x0000, { 50.0f, 50.0f, 50.0f }, { 0.0f, 50.0f, 0.0f } } } } }; /* its ride's box (file 0xB5's copy) */
Gfx *manjiPieceDls[] = { manjiPieceDl0, manjiPieceDl1, NULL }; /* the pieces' two display lists */

/* A manji's work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct ManjiWork {
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
    /* 0x3C */ ACollFrame *dmg;
    /* 0x40 */ s32 landed;
} ManjiWork;

/* A "卍pi" piece's work, at its task + 0x20. */
typedef struct ManjiPieceWork {
    /* 0x00 */ u8 unused0;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad9[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ u8 pad28[0xC]; /* EneObj's vel (eneSetup writes it); this work keeps its own vel at 0x40 */
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ s32 time;
    /* 0x3C */ s32 alpha;
    /* 0x40 */ f32 vel[3];
    /* 0x4C */ f32 spin[3];
} ManjiPieceWork;

#define MANJI(obj) ((ManjiWork *) FX(obj))
#define MANJI_PIECE(obj) ((ManjiPieceWork *) FX(obj))

f32 manjiLandAt(Obj *obj, f32 x, f32 y, f32 z, f32 h, f32 vy);
f32 manjiMove(Obj *obj, f32 r);

/* The "manji" task. */
s32 manjiProc(Obj *obj, s16 msg, void *arg) {
    ManjiWork *w = MANJI(obj);
    s32 pad[2];
    PolFrame *p;
    Vec3f *out;
    PolFrame *pol;
    s32 i;
    s32 pad3[2];

    switch (msg) {
        case 2:
            obj->name = "manji";
            w->rot[1] = (s32) eneSetup(obj, arg) * 3.141592741f / 512;
            p = w->pol = eneNewPol(obj, manjiDl);
            if (p == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmModelLayer);
            p->scale[0] = p->scale[1] = p->scale[2] = 0.200000003f;
            w->ride = rideCreate(gRideTree1, (s32) obj, &w->pol->f,
                                 (RideData *) fileRamAddrAt(0xB5, (u32) manjiRideData));
            if (w->ride == NULL) {
                return -1;
            }
            w->dmg = DMG_ADD_XYZ(ene_dmg_root_frame, &w->pol->f, 50, 100, 0, 0, 0, 0);
            w->rot[1] = vramCourseHeading(w->zone, w->x) - 1.570796371f;
            w->landed = 0;
            break;
        case 1:
            frmDestroy((Frame *) w->dmg);
            frmDestroy((Frame *) w->ride);
            frmDestroy((Frame *) w->pol);
            break;
        case 0:
            if (w->dmg != NULL && acollDmgWasHit(w->dmg) == 1 &&
                (w->dmg->hitKind == 0x19 || w->dmg->hitKind == 0x1A)) {
                plSendTagMsg((Obj *) w->dmg->hitPower, 0xD2C46678, (s32) obj, 0);
            }
            w->vel[1] += -0.4;
            manjiMove(obj, 10.0f);
            if (w->hit & 1) {
                w->vel[0] *= 0.5f;
                if (w->landed == 0 && w->vel[1] < -3.0f) {
                    SoundLocation2(0x10A, gGameWork.view, w->pol->pos[0], w->pol->pos[1], w->pol->pos[2], 0.0f,
                                   700.0f, 1.570796251f);
                }
                w->landed = 1;
                w->vel[1] = 0.0f;
            } else {
                w->landed = 0;
            }
            eneSetModelOnCourse(obj, &w->pol->f);
            eneStepHookPost(obj);
            break;
        case 9:
            switch (((s32 *) arg)[0]) {
                case 0x706F7369:
                    out = ((Vec3f **) arg)[1];
                    pol = w->pol;
                    out->x = pol->pos[0];
                    out->y = pol->pos[1] + 100.0f * ((Vec3f *) pol->scale)->y * 0.5f;
                    out->z = pol->pos[2];
                    break;
                case 0x6578706C:
                    for (i = 0; i < 6; i++) {
                        if (eneMake(ENE_MANJI_PIECE, 0, w->pol->pos[0], w->pol->pos[1], w->pol->pos[2]) == NULL) {
                            break;
                        }
                    }
                    plSoundAtPoint(w->pol->pos[0], w->pol->pos[1], w->pol->pos[2], 0x2E4, 0);
                    return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Whether a point h over (x, y, z), falling by vy, lands on a ride or the ground: if so, vy is
 * cut to land on it and the work is marked on the ground; the ground's triangle may move it to
 * another course. Returns vy (file 44's impObjLand by value). */
f32 manjiLandAt(Obj *obj, f32 x, f32 y, f32 z, f32 h, f32 vy) {
    ManjiWork *w = MANJI(obj);
    s32 pad;
    Vec3f dir;
    Vec3f from;
    Vec3f out;
    f32 t;
    CollTri *tri;

    dir.x = 0.0f;
    dir.y = vy + -h;
    dir.z = 0.0f;
    from.x = x;
    from.y = y + h;
    from.z = z;
    if (rideMoveStop(&from, &dir, &out, (s32) obj)) {
        w->hit |= 1;
        vy = out.y + h;
    }
    dir.y = vy + -h;
    tri = vramRayGround(&from, &dir, &out, &t);
    if (tri != NULL) {
        w->hit |= 1;
        vy = out.y - (from.y - h);
        if (w->zone != 0 && w->zone != tri->zone) {
            w->zone = tri->zone;
            w->x = vramNearestPlaceXZ(w->zone, out.x, out.z, &t);
        }
    }
    return vy;
}

/* The move: its speed turned to the course's heading, stopped by rides and the ground ahead,
 * then dropped onto them at three points across it; returns the distance moved along the
 * course (file 44's impObjBallMove, points 10 apart). */
f32 manjiMove(Obj *obj, f32 r) {
    ManjiWork *w = MANJI(obj);
    s32 pad0;
    Vec3f vel;
    Vec3f pos;
    Vec3f from;
    Vec3f dir;
    Vec3f d;
    s32 pad[9];
    Vec3f out;
    f32 t;
    s32 pad2[3];
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
        vel.y = manjiLandAt(obj, cx, pos.y, cz, r, vel.y);
        vel.y = manjiLandAt(obj, 10.0f * c + cx, pos.y, 10.0f * s + cz, r, vel.y);
        vel.y = manjiLandAt(obj, cx - 10.0f * c, pos.y, cz - 10.0f * s, r, vel.y);
    }
    mathVecRotateY(-w->rot[1], &vel.x, &vel.y, &vel.z);
    w->x += vel.x;
    w->y += vel.y;
    return vel.x;
}

/* A "卍pi" piece (kind 0x23E): one of the two display lists at random, a random size, thrown
 * up and out spinning, falling and fading for a second. */
s32 manjiPieceProc(Obj *obj, s16 msg, void *arg) {
    ManjiPieceWork *w = MANJI_PIECE(obj);
    s32 pad;
    PolFrame *p;

    switch (msg) {
        case 2:
            obj->name = "\xD2\xC4pi"; /* "卍pi" in EUC-JP */
            eneSetup(obj, arg);
            p = w->pol = eneNewPol(obj, manjiPieceDls[fadeRand(-1) & 1]);
            if (p == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayer8);
            p->scale[0] = p->scale[1] = p->scale[2] = fadeRand(100) * 0.009999999776f + 0.200000003f;
            w->alpha = 0xFF;
            w->vel[0] = (fadeRand(30) - 15) * 0.05000000075f;
            w->vel[1] = (fadeRand(6) + 6) * 0.200000003f;
            w->vel[2] = (fadeRand(30) - 15) * 0.05000000075f;
            w->spin[0] = (fadeRand(39) - 19.63495445f) * 0.004999999888f;
            w->spin[1] = (fadeRand(39) - 19.63495445f) * 0.004999999888f;
            w->spin[2] = (fadeRand(39) - 19.63495445f) * 0.004999999888f;
            w->time = 0;
            break;
        case 1:
            frmDestroy((Frame *) w->pol);
            break;
        case 0:
            p = w->pol;
            p->pos[0] += w->vel[0];
            p->pos[1] += w->vel[1];
            p->pos[2] += w->vel[2];
            p->rot[0] += w->spin[0];
            p->rot[1] += w->spin[1];
            p->rot[2] += w->spin[2];
            w->vel[1] -= 0.3000000119f;
            w->time++;
            if (w->time > 60) {
                return -1;
            }
            if (w->alpha != 0) {
                w->alpha -= 10;
                if (w->alpha < 0) {
                    w->alpha = 0;
                }
            }
            break;
        case 3:
            if (w->pol != NULL) {
                w->pol->dl = gGfxDlHead;
                gDPPipeSync(gGfxDlHead++);
                gDPSetCycleType(gGfxDlHead++, G_CYC_1CYCLE);
                gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
                gSPEndDisplayList(gGfxDlHead++);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
