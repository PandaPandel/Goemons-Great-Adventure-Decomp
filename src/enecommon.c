#include "player.h"
#include "game.h"
#include "contpak.h"
#include "effect.h"
#include "enecommon.h"
#include "enemy.h"
#include "eneproc.h"
#include "fade.h"
#include "file.h"
#include "gfxmgr.h"
#include "itex.h"
#include "mathutil.h"
#include "plmain.h"
#include "polygon.h"
#include "sndloc.h"
#include "tmanager.h"
#include "vram.h"

/* The game's debug print as this file was built: compiled to nothing but its arguments, so the
 * strings stay in .rodata and the arguments are still evaluated (as pakfiler.c). */
#define DPRINTF

/* The enemies' shared routines (overlay 9): moving and turning an enemy (its work, EneObj, at
 * task + 0x20), placing its model at it, the ground and walls, the players around it, the
 * sound and knockback of a hit on its damage box, the stage's counters (0x8008830A), and the
 * pieces it breaks into: "kill_hahen" (they fall and burst) and "futtobi_hahen" (blown off,
 * they spin and shatter). */

/* For the thrown and blown attacks, a box's three words are the hit's direction. */
#define HITDIR(box) ((f32 *) &(box)->hitInfo)

extern Frame *gFrmModelLayer;
extern Frame *gFrmLayer2;
extern Frame *gFrmLayers[]; /* the frame pools (gFrmLayer2 is [2]) */
extern s16 gFrmKindPol; /* "pol" */
extern s16 gFrmKindAnime; /* "anime" */
extern Gfx *gGfxDlHead; /* the display-list buffer */

/* The enemies' step hooks (empty). */
void eneStepHookPre(Obj *o) {
}

void eneStepHookPost(Obj *o) {
}

/* Counter n's reward: none at 0, else two enemies of kind 0x1FF (at 1) or 0x200 at the two
 * places. */
s32 eneCounterReward(s32 n, s32 zone0, f32 x0, f32 y0, f32 z0, s32 zone1, f32 x1, f32 y1, f32 z1) {
    s32 c = eneCounterGet(n);

    if (c == 0) {
        return c;
    }
    if (c == 1) {
        (*eneMakeEx)(NULL, ENE_ETSK_NEKO_MAIN_FLAGS_3, zone0, x0, y0, z0, 2, (void *) 1);
        (*eneMakeEx)(NULL, ENE_ETSK_NEKO_MAIN_FLAGS_3, zone1, x1, y1, z1, 2, (void *) 1);
    } else {
        (*eneMakeEx)(NULL, ENE_ETSK_GOLDNEKO_MAIN_FLAGS_3, zone0, x0, y0, z0, 2, (void *) 1);
        (*eneMakeEx)(NULL, ENE_ETSK_GOLDNEKO_MAIN_FLAGS_3, zone1, x1, y1, z1, 2, (void *) 1);
    }
    return c;
}

/* Every player in play: plEndPowerDown. */
void eneEndPlayersPowerDown(void) {
    s32 i;

    for (i = 0; i < 4; i++) {
        if ((*plIsInPlay)(i)) {
            plEndPowerDown(gGameWork.players[i]);
        }
    }
}

/* Rumbles every player in play. */
void eneRumbleAll(u32 pattern, s32 time) {
    s32 i;

    for (i = 0; i < 4; i++) {
        if ((*plIsInPlay)(i)) {
            contPakRumbleStart(i, pattern, time);
        }
    }
}

/* And stops it. */
void eneRumbleStopAll(void) {
    s32 i;

    for (i = 0; i < 4; i++) {
        contPakRumbleRequestStop(i);
    }
}

/* Stage counter n (0x8008830A + n): the low four bits count to 15; 0x80 a flag. */
void eneCounterSetFlag(s32 n) {
    gSaveData.stageFlags[0x58 + n] |= 0x80;
}

void eneCounterClearFlag(s32 n) {
    gSaveData.stageFlags[0x58 + n] &= ~0x80;
}

s32 eneCounterIncrement(s32 n) {
    s32 c = gSaveData.stageFlags[0x58 + n] & 0xF;
    s32 hi = gSaveData.stageFlags[0x58 + n] & 0x80;

    if (c < 15) {
        c++;
    }
    gSaveData.stageFlags[0x58 + n] = c + hi;
    return c;
}

s32 eneCounterGet(s32 n) {
    u8 v = gSaveData.stageFlags[0x58 + n];

    return v & 0xF;
}

/* Moves the enemy towards (tx, ty, tz) by |sx|, |sy|, |sz|; how many axes it is not at. */
s32 eneMoveToward(Obj *o, f32 tx, f32 ty, f32 tz, f32 sx, f32 sy, f32 sz) {
    EneObj *e = ENE(o);
    s32 n = 0;

    if (tx != e->x) {
        n = 1;
        if (e->x < tx) {
            e->x += fabsf(sx);
            if (tx <= e->x) {
                tx = e->x;
            }
        }
        if (tx < e->x) {
            e->x -= fabsf(sx);
        }
    }
    if (ty != e->y) {
        n++;
        if (e->y < ty) {
            e->y += fabsf(sy);
            if (ty <= e->y) {
                ty = e->y;
            }
        }
        if (ty < e->y) {
            e->y -= fabsf(sy);
        }
    }
    if (tz != e->z) {
        n++;
        if (e->z < tz) {
            e->z += fabsf(sz);
            if (tz <= e->z) {
                tz = e->z;
            }
        }
        if (tz < e->z) {
            e->z -= fabsf(sz);
        }
    }
    return n;
}

/* A hit's sound, by the attack's kind. */
void eneHitSound(ACollFrame *box) {
    eneHitSoundByKind(box, 0);
}

void eneHitSoundHeavy(ACollFrame *box) {
    eneHitSoundByKind(box, 1);
}

void eneHitSoundAlt(ACollFrame *box) {
    eneHitSoundByKind(box, 0);
}

void eneHitSoundByKind(ACollFrame *box, s32 heavy) {
    switch (box->hitKind) {
        case 1:
        case 27:
        default:
            soundIncamCam(0x22C, box->hitPos[0], box->hitPos[1], box->hitPos[2]);
            break;
        case 4:
        case 5:
        case 6:
        case 23:
            if (heavy == 1) {
                soundIncamCam(0x220, box->hitPos[0], box->hitPos[1], box->hitPos[2]);
            } else {
                soundIncamCam(0x22C, box->hitPos[0], box->hitPos[1], box->hitPos[2]);
            }
            break;
        case 18:
        case 19:
        case 20:
            soundIncamCam(0x22E, box->hitPos[0], box->hitPos[1], box->hitPos[2]);
            break;
        case 10:
        case 11:
        case 14:
        case 15:
        case 21:
            break;
    }
}

/* Which side of the course a heading a is, against the course's heading b: 1 or -1. */
s32 eneHeadingSide(f32 a, f32 b) {
    if ((s32) ((a - b + 1.5707964f) * 0.31830987f + 256.0f) & 1) {
        return -1;
    }
    return 1;
}

/* Which way along its course a hit knocks the enemy, times |speed|: by the attacker's heading,
 * by the hit's direction, or (no direction) by which side of it the hit landed. */
f32 eneKnockbackSpeed(Obj *o, ACollFrame *box, f32 speed) {
    EneObj *e = ENE(o);
    f32 x;
    f32 y;
    f32 z;
    s32 pad[3];
    f32 side;
    s32 pad2;
    f32 side2;

    speed = fabsf(speed);
    switch (box->hitKind) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 17:
        case 18:
        case 19:
        case 20:
        case 22:
        case 24:
        case 25:
        case 26:
        case 27:
        case 32:
            return eneHeadingSide(box->hitRotY, vramCourseHeading(e->zone, e->x)) * speed;
        case 7:
        case 8:
        case 9:
        case 11:
        case 12:
        case 15:
        case 16:
        case 23:
        case 33:
            x = HITDIR(box)[0];
            y = HITDIR(box)[1];
            z = HITDIR(box)[2];
            if (x < 0.0001 && y < 0.0001) {
                return ((e->x < (*vramNearestPlaceXZ)(e->zone, box->hitPos[0], box->hitPos[2], &side)) ? -1 : 1) *
                       speed;
            }
            return eneHeadingSide(mathAtan2(x, z), vramCourseHeading(e->zone, e->x)) *
                   speed;
        case 0:
        default:
            return ((e->x < (*vramNearestPlaceXZ)(e->zone, box->hitPos[0], box->hitPos[2], &side2)) ? -1 : 1) *
                   speed;
    }
}

/* How hard a hit is (0-3), by the attack's kind. */
s32 eneHitStrength(ACollFrame *box) {
    switch (box->hitKind) {
        case 0:
            return 0;
        default:
            return 1;
        case 2:
        case 5:
        case 12:
        case 16:
        case 19:
        case 30:
            return 2;
        case 3:
        case 6:
        case 8:
        case 20:
        case 31:
            return 3;
    }
}

/* Puts a model (an "anime" or "pol" frame) at the enemy, on its course. */
void eneSetModelOnCourse(Obj *o, Frame *f) {
    s32 pad[3];
    EneObj *e = ENE(o);

    if (f->kind == gFrmKindAnime) {
        vramCoursePointXZ(e->zone, e->x, e->z, &((AnimeFrame *) f)->pos[0], &((AnimeFrame *) f)->pos[2]);
        ((AnimeFrame *) f)->pos[1] = e->y;
        ((AnimeFrame *) f)->rot[0] = e->rot[0];
        ((AnimeFrame *) f)->rot[1] = e->rot[1];
        ((AnimeFrame *) f)->rot[2] = e->rot[2];
        return;
    }
    if (f->kind == gFrmKindPol) {
        vramCoursePointXZ(e->zone, e->x, e->z, &((PolFrame *) f)->pos[0], &((PolFrame *) f)->pos[2]);
        ((PolFrame *) f)->pos[1] = e->y;
        ((PolFrame *) f)->rot[0] = e->rot[0];
        ((PolFrame *) f)->rot[1] = e->rot[1];
        ((PolFrame *) f)->rot[2] = e->rot[2];
    }
}

/* The same, off any course. */
void eneSetModel(Obj *o, Frame *f) {
    EneObj *e = ENE(o);

    if (f->kind == gFrmKindAnime) {
        ((AnimeFrame *) f)->pos[0] = e->x;
        ((AnimeFrame *) f)->pos[1] = e->y;
        ((AnimeFrame *) f)->pos[2] = e->z;
        ((AnimeFrame *) f)->rot[0] = e->rot[0];
        ((AnimeFrame *) f)->rot[1] = e->rot[1];
        ((AnimeFrame *) f)->rot[2] = e->rot[2];
        return;
    }
    if (f->kind == gFrmKindPol) {
        ((PolFrame *) f)->pos[0] = e->x;
        ((PolFrame *) f)->pos[1] = e->y;
        ((PolFrame *) f)->pos[2] = e->z;
        ((PolFrame *) f)->rot[0] = e->rot[0];
        ((PolFrame *) f)->rot[1] = e->rot[1];
        ((PolFrame *) f)->rot[2] = e->rot[2];
    }
}

/* The enemy's place as it keeps it, and its course. */
u8 eneGetCoursePos(Obj *o, Vec3f *out) {
    out->x = ENE(o)->x;
    out->y = ENE(o)->y;
    out->z = ENE(o)->z;
    return ENE(o)->zone;
}

/* The enemy's place in the world. */
void eneWorldPos(Obj *o, Vec3f *out) {
    vramCoursePointXZ(ENE(o)->zone, ENE(o)->x, ENE(o)->z, &out->x, &out->z);
    out->y = ENE(o)->y;
}

void eneGetWorldPos(Obj *o, Vec3f *out) {
    eneWorldPos(o, out);
}

/* A random player in play. */
Obj *eneRandomPlayer(void) {
    u8 i;
    u8 n;
    Obj *list[4];

    for (i = 0, n = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            list[n] = gGameWork.players[i];
            n++;
        }
    }
    i = fadeRand(n);
    return list[i];
}

/* A random player in play, by number. */
u8 eneRandomPlayerIndex(void) {
    u8 i;
    u8 n;
    u8 list[4];

    for (i = 0, n = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            list[n] = i;
            n++;
        }
    }
    return list[(u8) fadeRand(n)];
}

/* How many players are in play and not plIsPoweredDown. */
u8 eneCountPlayersUp(void) {
    u8 i;
    u8 n;

    for (i = 0, n = 0; i < 4; i++) {
        if (plIsInPlay(i) &&!(*plIsPoweredDown)(i)) {
            n++;
        }
    }
    return n;
}

/* Sets the enemy's speed towards (x, y, z), at speed. */
void eneSetVelToward(Obj *o, f32 speed, f32 x, f32 y, f32 z) {
    s32 pad[3];
    f32 v[3];
    f32 d;
    EneObj *e = ENE(o);

    v[0] = x - e->x;
    v[1] = y - e->y;
    v[2] = z - e->z;
    d = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    if (d == 0.0f) {
        d = 1.0f;
    }
    e->vel[0] = v[0] / d * speed;
    e->vel[1] = v[1] / d * speed;
    e->vel[2] = v[2] / d * speed;
}

/* And flat, towards (x, z). */
void eneSetVelTowardXZ(Obj *o, f32 speed, f32 x, f32 z) {
    s32 pad[3];
    f32 v[2];
    f32 d;
    EneObj *e = ENE(o);

    v[0] = x - e->x;
    v[1] = z - e->z;
    d = sqrtf(v[0] * v[0] + v[1] * v[1]);
    if (d == 0.0f) {
        d = 1.0f;
    }
    e->vel[0] = v[0] / d * speed;
    e->vel[2] = v[1] / d * speed;
}

/* How far (x, y, z) is from the enemy. */
f32 eneDistTo(Obj *o, f32 x, f32 y, f32 z) {
    s32 pad[3];
    f32 v[3];

    v[0] = x - ENE(o)->x;
    v[1] = y - ENE(o)->y;
    v[2] = z - ENE(o)->z;
    return sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

/* The enemy's place from a model's (the reverse of eneSetModelOnCourse). */
void eneFromModelOnCourse(Obj *o, Frame *f) {
    s32 pad[3];
    EneObj *e = ENE(o);

    if (f != NULL) {
        if (f->kind == gFrmKindAnime) {
            e->x = vramNearestPlaceXZ(e->zone, ((AnimeFrame *) f)->pos[0], ((AnimeFrame *) f)->pos[2], &e->z);
            e->y = ((AnimeFrame *) f)->pos[1];
            e->rot[0] = ((AnimeFrame *) f)->rot[0];
            e->rot[1] = ((AnimeFrame *) f)->rot[1];
            e->rot[2] = ((AnimeFrame *) f)->rot[2];
            return;
        }
        if (f->kind == gFrmKindPol) {
            e->x = vramNearestPlaceXZ(e->zone, ((PolFrame *) f)->pos[0], ((PolFrame *) f)->pos[2], &e->z);
            e->y = ((PolFrame *) f)->pos[1];
            e->rot[0] = ((PolFrame *) f)->rot[0];
            e->rot[1] = ((PolFrame *) f)->rot[1];
            e->rot[2] = ((PolFrame *) f)->rot[2];
        }
    }
}

/* And off any course. */
void eneFromModel(Obj *o, Frame *f) {
    EneObj *e = ENE(o);

    if (f != NULL) {
        if (f->kind == gFrmKindAnime) {
            e->x = ((AnimeFrame *) f)->pos[0];
            e->y = ((AnimeFrame *) f)->pos[1];
            e->z = ((AnimeFrame *) f)->pos[2];
            e->rot[0] = ((AnimeFrame *) f)->rot[0];
            e->rot[1] = ((AnimeFrame *) f)->rot[1];
            e->rot[2] = ((AnimeFrame *) f)->rot[2];
            return;
        }
        if (f->kind == gFrmKindPol) {
            e->x = ((PolFrame *) f)->pos[0];
            e->y = ((PolFrame *) f)->pos[1];
            e->z = ((PolFrame *) f)->pos[2];
            e->rot[0] = ((PolFrame *) f)->rot[0];
            e->rot[1] = ((PolFrame *) f)->rot[1];
            e->rot[2] = ((PolFrame *) f)->rot[2];
        }
    }
}

/* Runs a model's animation by step, stopping at its ends; whether it reached one. */
s32 eneAnimPlayOnce(Obj *o, AnimeFrame *m, f32 step) {
    s32 n;

    m->frame += step;
    if (step > 0.0f) {
        n = polAnimeLastFrame(m);
        if (n < m->frame) {
            m->frame = n;
            return 1;
        }
    } else if (m->frame < 0.0f) {
        m->frame = 0.0f;
        return 1;
    }
    return 0;
}

/* The same, looping; whether it wrapped. */
s32 eneAnimLoop(Obj *o, AnimeFrame *m, f32 step) {
    m->frame += step;
    if (step > 0.0f) {
        if (polAnimeLastFrame(m) + 1 <= m->frame) {
            m->frame -= polAnimeLastFrame(m) + 1;
            return 1;
        }
    } else if (m->frame < 0.0f) {
        m->frame += polAnimeLastFrame(m) + 1;
        return 1;
    }
    return 0;
}

/* Turns a moving enemy to face along its course, the way it goes, by pi / n a step. */
void eneTurnAlongCourse(Obj *o, f32 n) {
    EneObj *e = ENE(o);
    f32 a;
    f32 d;
    f32 step;
    s32 pad;

    if (fabsf(e->vel[0]) > 0.0001) {
        a = vramCourseHeading(e->zone, e->x);
        if (e->vel[0] < 0.0f) {
            a += 3.1415927f;
        }
        if (a - e->rot[1] > 3.1415927f) {
            e->rot[1] += 6.2831855f;
        } else if (a - e->rot[1] < -3.1415927f) {
            e->rot[1] -= 6.2831855f;
        }
        d = (a - e->rot[1] > 0.0f) ? a - e->rot[1] : -(a - e->rot[1]);
        step = 3.1415927f / n;
        if (d < step) {
            e->rot[1] = a;
            return;
        }
        if (a < e->rot[1]) {
            e->rot[1] -= step;
        } else {
            e->rot[1] += step;
        }
    }
}

/* Turns the enemy's heading towards a by step; whether it got there. */
s32 eneTurnToward(Obj *o, f32 a, f32 step) {
    EneObj *e = ENE(o);

    if (a - e->rot[1] > 3.1415927f) {
        e->rot[1] += 6.2831855f;
    } else if (a - e->rot[1] < -3.1415927f) {
        e->rot[1] -= 6.2831855f;
    }
    if (fabsf(a - e->rot[1]) < step) {
        e->rot[1] = a;
        return 1;
    }
    if (a < e->rot[1]) {
        e->rot[1] -= step;
    } else {
        e->rot[1] += step;
    }
    return 0;
}

/* Falls by g a step, 8 at most. */
void eneFall(Obj *o, f32 g) {
    ENE(o)->vel[1] -= g;
    if (ENE(o)->vel[1] <= -8.0f) {
        ENE(o)->vel[1] = -8.0f;
    }
}

/* Speeds *vel up by |accel| towards target, up to max either way, and moves *pos by it. */
void eneAccelAxis(f32 target, f32 max, f32 accel, f32 *pos, f32 *vel) {
    f32 d = target - *pos;

    if (d >= 0.0f) {
        *vel += (accel > 0.0f) ? accel : -accel;
        if (max <= *vel) {
            *vel = max;
        }
    } else {
        *vel -= (accel > 0.0f) ? accel : -accel;
        if (*vel <= -max) {
            *vel = -max;
        }
    }
    *pos += *vel;
}

void eneMoveObj(Obj *o) {
    eneMove(ENE(o));
}

/* Moves by the speed. */
void eneMove(EneObj *e) {
    e->x += e->vel[0];
    e->y += e->vel[1];
    e->z += e->vel[2];
}

/* Speeds up by (ax, ay, az) and moves (the game adds the speed across to y, not z). */
void eneAccelMove(EneObj *e, f32 ax, f32 ay, f32 az) {
    e->vel[0] += ax;
    e->vel[1] += ay;
    e->vel[2] += az;
    e->x += e->vel[0];
    e->y += e->vel[1];
    e->y += e->vel[2]; /* sic: the game adds the z speed to y */
}

s32 eneLand(EneObj *e, f32 h) {
    return eneLandOff(e, h, 0.0f);
}

/* Lands a falling enemy on the ground under it (h above its feet, off below them): 1 on the
 * ground, -1 on a ground of another course (it moves onto that course), 0 in the air. */
s32 eneLandOff(EneObj *e, f32 h, f32 off) {
    Vec3f from;
    Vec3f dir;
    Vec3f at;
    CollTri *tri;
    f32 t;

    if (e->vel[1] <= 0.0f) {
        dir.x = 0.0f;
        dir.y = e->vel[1] - h;
        dir.z = 0.0f;
        vramCoursePointXZ(e->zone, e->x, e->z, &from.x, &from.z);
        from.y = e->y + off + h;
        tri = vramRayGround(&from, &dir, &at, &t);
        if (tri != NULL) {
            PRINTF("HITHIT t=%f\n", t);
            e->y = at.y - off;
            if (e->zone != 0 && tri->zone != 0 && e->zone != tri->zone) {
                e->zone = tri->zone;
                e->x = vramNearestPlaceXZ(e->zone, from.x, from.z, &e->z);
                return -1;
            }
            return 1;
        }
    }
    return 0;
}

s32 eneWallCheckPush(Obj *o, f32 h, f32 r, f32 a, f32 b) {
    return eneWallCheckEx(o, h, r, a, b, 1);
}

s32 eneWallCheckNoPush(Obj *o, f32 h, f32 r, f32 a, f32 b) {
    return eneWallCheckEx(o, h, r, a, b, 0);
}

/* Whether the enemy runs into a wall: a ray from h up, (dx, dz) ahead the way it is moving and
 * down below its feet. 1: a wall past half the ray; -1: a nearer one (with push, the enemy
 * backs off it by half the ray); 0: nothing. */
s32 eneWallCheckEx(Obj *o, f32 h, f32 dx, f32 dz, f32 down, u8 push) {
    EneObj *e = ENE(o);
    s32 pad;
    Vec3f to;
    Vec3f from;
    Vec3f dir;
    Vec3f at;
    f32 t;
    f32 k;
    s32 pad2[2];
    f32 side;

    dx = fabsf(dx);
    if (e->vel[0] < 0.0f) {
        dx *= -1.0f;
    }
    dz = fabsf(dz);
    if (e->vel[2] < 0.0f) {
        dz *= -1.0f;
    }
    h = fabsf(h);
    down = fabsf(down);
    vramCoursePointXZ(e->zone, e->x + dx, e->z + dz, &to.x, &to.z);
    vramCoursePointXZ(e->zone, e->x, e->z, &from.x, &from.z);
    from.y = e->y + h;
    dir.x = to.x - from.x;
    dir.z = to.z - from.z;
    dir.y = -(h + down);
    k = fabsf(dir.y) / h;
    dir.x *= k;
    dir.z *= k;
    if (vramRayGround(&from, &dir, &at, &t) != NULL) {
        if (t >= 0.5) {
            return 1;
        }
        if (push) {
            e->x = vramNearestPlaceXZ(e->zone, at.x, at.z, &side) - k * (dx * 0.5);
            e->z = side - k * (dz * 0.5);
        }
        return -1;
    }
    return 0;
}

s32 eneRayHitPush(Obj *o, f32 h, f32 x, f32 y, f32 z) {
    return eneRayHitEx(o, h, x, y, z, 1);
}

/* Whether the ray (x, y, z) from the enemy (h up) hits a wall; with push, the enemy moves back
 * from the hit by the ray.
 * The game's frame: `e` (set at its declaration, so `o` stays in its home, rule 132) and the
 * pad are the two unused words above `from`, and the hit triangle `tri`, declared last, is the
 * word under `t` (as eneLandOff keeps it). */
s32 eneRayHitEx(Obj *o, f32 h, f32 x, f32 y, f32 z, u8 push) {
    EneObj *e = ENE(o);
    s32 pad;
    Vec3f from;
    Vec3f dir;
    Vec3f at;
    f32 t;
    CollTri *tri;

    from.x = e->x;
    from.y = e->y + h;
    from.z = e->z;
    dir.x = x;
    dir.y = y;
    dir.z = z;
    tri = vramRayGround(&from, &dir, &at, &t);
    if (tri != NULL) {
        if (push) {
            e->x = at.x - x;
            e->y = at.y - y;
            e->z = at.z - z;
        }
        return 1;
    }
    return 0;
}

/* Moves the enemy by its speed, then lands it and checks its walls: its hit flags. */
void eneMoveAndCollide(Obj *o, f32 h, f32 r, f32 a, f32 b) {
    EneObj *e = ENE(o);
    s32 wall;

    eneMove(e);
    e->hit &= ~7;
    if (eneLand(e, h)) {
        e->hit |= 1;
    }
    wall = eneWallCheckPush(o, h, r, a, b);
    if (wall < 0) {
        e->hit |= 2;
    } else if (wall == 0) {
        e->hit |= 4;
    }
}

/* How far the nearest player in play is from the enemy (-1: none). */
f32 eneNearestPlayerDist(Obj *o) {
    EneObj *e = ENE(o);
    f32 min;
    Vec3f pos;
    Vec3f p;
    f32 d;
    s32 i;
    s32 pad;

    min = -1.0f;
    if (e->zone != 0) {
        vramCoursePointXZ(e->zone, e->x, e->z, &pos.x, &pos.z);
    } else {
        pos.x = e->x;
        pos.z = e->z;
    }
    pos.y = e->y;
    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plGetModelPos(gGameWork.players[i], &p.x, &p.y, &p.z);
            d = sqrtf((p.x - pos.x) * (p.x - pos.x) + (p.y - pos.y) * (p.y - pos.y) +
                      (p.z - pos.z) * (p.z - pos.z));
            if (min < 0.0f) {
                min = d;
            }
            if (d < min) {
                min = d;
            }
        }
    }
    return min;
}

/* The players' average place (0, 0, 0 with none in play). */
void enePlayersCenter(f32 *x, f32 *y, f32 *z) {
    f32 sx;
    f32 sy;
    f32 sz;
    s32 n;
    s32 i;
    f32 px;
    f32 py;
    f32 pz;

    n = 0;
    sx = sy = sz = 0.0f;
    for (i = 0; i < 4; i++) {
        if ((*plIsInPlay)(i)) {
            (*plGetModelPos)(gGameWork.players[i], &px, &py, &pz);
            n++;
            sx += px;
            sy += py;
            sz += pz;
        }
    }
    if (n == 0) {
        *x = 0.0f;
        *y = 0.0f;
        *z = 0.0f;
    } else {
        *x = sx / n;
        *y = sy / n;
        *z = sz / n;
    }
}

/* The players' average place along their courses (0, 0, 0 with none in play). */
void enePlayersCenterOnCourse(f32 *place, f32 *height, f32 *side) {
    f32 sx;
    f32 sy;
    f32 sz;
    s32 n;
    s32 i;
    f32 px;
    f32 py;
    f32 pz;
    s32 zone;

    n = 0;
    sx = sy = sz = 0.0f;
    for (i = 0; i < 4; i++) {
        if ((*plIsInPlay)(i)) {
            (*plAskPos)(gGameWork.players[i], &px, &py, &pz, &zone);
            n++;
            sx += px;
            sy += py;
            sz += pz;
        }
    }
    if (n == 0) {
        *place = 0.0f;
        *height = 0.0f;
        *side = 0.0f;
    } else {
        *place = sx / n;
        *height = sy / n;
        *side = sz / n;
    }
}

/* Breaks the enemy into two "kill_hahen" pieces flying off away from the hit: dl (3 bounces)
 * and dl2 (1 bounce, lit). */
void eneMakeKillHahen(Obj *o, ACollFrame *box, u32 dl, u32 dl2) {
    EneObj *e = ENE(o);
    s32 pad;
    Obj *p;
    Obj *p2;
    s32 pad2[2];
    f32 v[2];
    f32 d;

    if (e->model != NULL) {
        v[0] = ((PolFrame *) e->model)->pos[0] - box->hitPos[0];
        v[1] = ((PolFrame *) e->model)->pos[2] - box->hitPos[2];
        d = sqrtf(v[0] * v[0] + v[1] * v[1]);
        v[0] /= d;
        v[1] /= d;
        p = eneMakeBaseTask(1, kill_hahen);
        if (p != NULL) {
            PRINTF("UP PARTS SET\n");
            if (eneKillHahenModel(p, e, dl, 3, 5.0f, v[0], 5.0f, v[1], ((PolFrame *) e->model)->scale[1], 0) ==
                NULL) {
                tskDestroy(p);
            }
        }
        p2 = eneMakeBaseTask(1, kill_hahen);
        if (p2 != NULL) {
            PRINTF("DOWN PARTS SET\n");
            if (eneKillHahenModel(p2, e, dl2, 1, 0.0f, v[0], 3.0f, v[1], ((PolFrame *) e->model)->scale[1], 1) ==
                NULL) {
                tskDestroy(p2);
            }
        }
    }
}

/* A "kill_hahen" piece: it falls, bounces off the ground at half its speed until its bounces
 * run out (or lies still once slow), and bursts when they or its time run out. */
s32 kill_hahen(Obj *obj, s16 msg, void *arg) {
    EneObj *e = ENE(obj);

    switch (msg) {
        case 2:
            obj->name = "kill_hahen";
            ENE(obj)->time = 30;
            PRINTF("PARTS SET\n");
            break;
        case 1:
            if (e->model != NULL) {
                frmDestroy(e->model);
            }
            if (e->lit) {
                effectFspiritCreate(e->x, e->y, e->z, 0.1f, 0xE0, 0xE0, 0xE0, 0x10, 0x10, 0x10);
            }
            break;
        case 0:
            eneStepHookPre(obj);
            eneMove(e);
            eneFall(obj, 0.8f);
            if (--e->time == 0) {
                if (e->lit) {
                    eneSetSd(obj, 0x522);
                } else {
                    eneSetSd(obj, 0x523);
                }
                itexPuffYellow(e->x, e->y, e->z, 0.06f);
                return -1;
            }
            if (eneLandOff(e, 20.0f, 0.0f)) {
                if (fabsf(e->vel[1]) < (f64) 0.8f) {
                    e->vel[1] = 0.0f;
                } else {
                    if (--e->count < 0) {
                        if (e->lit) {
                            eneSetSd(obj, 0x522);
                        } else {
                            eneSetSd(obj, 0x523);
                        }
                        itexPuffYellow(e->x, e->y, e->z, 0.06f);
                        return -1;
                    }
                    e->vel[1] *= -0.5;
                }
            }
            if ((*eneAtamaCheck)(obj, 20.0f)) {
                e->vel[1] = 0.0f;
            }
            eneSetModel(obj, e->model);
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Makes a piece's model (a "pol" frame drawing dl from from's files) at from, up above it,
 * moving at (vx, vy, vz), for life bounces. */
Frame *eneKillHahenModel(Obj *o, EneObj *from, u32 dl, s32 life, f32 up, f32 vx, f32 vy, f32 vz, f32 scale,
                             u8 lit) {
    EneObj *e = ENE(o);
    PolFrame *f;
    EneKind *kind = from->kind;
    u8 zone;
    Vec3f pos;

    f = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
    if (f != NULL) {
        e->count = life;
        e->zone = 0;
        if (from->zone) {
            vramCoursePointXZ(from->zone, from->x, from->z, &pos.x, &pos.z);
        } else {
            pos.x = from->x;
            pos.z = from->z;
        }
        f->pos[0] = pos.x;
        e->x = f->pos[0];
        e->y = f->pos[1] = from->y + up;
        f->pos[1] = pos.z; /* sic: pos[2] is never set */
        e->z = f->pos[1];
        e->rot[0] = 0.0f;
        e->rot[1] = from->rot[1];
        e->rot[2] = 0.0f;
        e->vel[0] = vx;
        e->vel[1] = vy;
        e->vel[2] = vz;
        e->lit = lit;
        f->scale[0] = f->scale[1] = f->scale[2] = scale;
        f->dl2 = (Gfx *) dl;
        f->flags |= 0x20;
        f->segs[0] = fileRamAddr(kind->files.file[0]);
        f->segs[1] = fileRamAddr(kind->files.file[1]);
        f->segs[2] = fileRamAddr(kind->files.file[2]);
        e->model = (Frame *) f;
        eneSetModel(o, (Frame *) f);
    }
    return (Frame *) f;
}

/* Blows the enemy off as a "futtobi_hahen" piece (a "pol" frame, dl), sideways to its course,
 * on the side away from the attacker's heading. */
void eneMakeFuttobiHahen(Obj *o, ACollFrame *box, u32 dl, u8 turn, u8 second) {
    EneObj *e = ENE(o);
    s32 pad;
    Obj *p;
    s32 pad2;
    f32 rotY;
    f32 a;
    f32 vx;
    f32 vz;

    if (e->model != NULL) {
        rotY = box->hitRotY;
        a = (*vramCourseHeading)(e->zone, e->x) - (f64) 1.5707964f;
        if (a < 0.0f) {
            a += 6.2831855f;
        }
        if (rotY - a > 3.1415927f) {
            a += 6.2831855f;
        } else if (rotY - a < -3.1415927f) {
            a -= 6.2831855f;
        }
        if (a < rotY) {
            a += 3.1415927f;
        }
        vx = sinf(a) * 5.0f;
        vz = cosf(a) * 5.0f;
        p = eneMakeBaseTask(1, futtobi_hahen);
        if (p != NULL) {
            if (eneFuttobiHahenModel(p, e, dl, 5.0f, vx, 5.0f, vz, ((PolFrame *) e->model)->scale[1], 1, turn,
                                       second) == NULL) {
                tskDestroy(p);
            }
        }
    }
}

/* The same with an animated model ("anime" frame, anim from frame on). */
void eneMakeFuttobiHahenAnime(Obj *o, ACollFrame *box, u32 anim, f32 frame, u8 turn, u8 second) {
    EneObj *e = ENE(o);
    s32 pad;
    Obj *p;
    s32 pad2;
    f32 rotY;
    f32 a;
    f32 vx;
    f32 vz;

    if (e->model != NULL) {
        rotY = box->hitRotY;
        a = (*vramCourseHeading)(e->zone, e->x) - (f64) 1.5707964f;
        if (a < 0.0f) {
            a += 6.2831855f;
        }
        if (rotY - a > 3.1415927f) {
            a += 6.2831855f;
        } else if (rotY - a < -3.1415927f) {
            a -= 6.2831855f;
        }
        if (a < rotY) {
            a += 3.1415927f;
        }
        vx = sinf(a) * 5.0f;
        vz = cosf(a) * 5.0f;
        p = eneMakeBaseTask(1, futtobi_hahen);
        if (p != NULL) {
            if (eneFuttobiHahenAnime(p, e, anim, frame, 5.0f, vx, 5.0f, vz, ((PolFrame *) e->model)->scale[1], 1,
                                       turn, second) == NULL) {
                tskDestroy(p);
            }
        }
    }
}

/* A "futtobi_hahen" piece: it flies until it hits a wall (its model squashes, 1.3 wide and
 * half deep, with the sound 0x2F9 if in view), shakes there for 8 frames and bursts (with a
 * flash if lit); it vanishes if its 40 frames run out first. Message 3 gives its model a
 * black environment colour (or, under the second root, eneDlEnvColorLit's list).
 * How it matched (2026-10-03, from 273 lines): case 0's second test is an "else if" on
 * e->model->kind itself, pf set inside it (the game compares the one loaded kind twice: 276,
 * mix distance 2); case 3 tests e->model->kind and copies e->model into f at the head of each
 * branch (the game's "or s0,v1": 26, the game's length); the shake's sum written delta first,
 * "(f32) (count-- & 1) - 0.5f + e->x" (24); the second root read as gFrmLayers[2], an element
 * of the frame pools, which loads it after the parent as the game does: a match.
 * Earlier, while parked (451 lines, mostly stack offsets): the original's frame is 0x70 and it never
 * keeps obj in a register (it reloads it from its home in every case). 2026-10-02: e set at its
 * declaration (rule 132) now keeps obj in its home, and a FAKE pad[8] makes the frame 0x68.
 * The game's slots: view spilled at 0x44, af 0x40, pf 0x3C (the last declared), f in s0. Here
 * f is spilled (0x60) and view gets s0; a block split after "af/pf = f" changed nothing.
 * The burst's speeds are "* 2" (int): the game multiplies by a 2.0f held in f0, IDO turns
 * "* 2.0f" into adds (451 to 360 lines). Still: the frame 8 short and f spilled where the game
 * spills af and pf.
 * 2026-10-03 (360 to 276): case 0 reads e->model straight into af and pf (no f): with f, IDO
 * merged "af = f" into f and spilled f's home; now af and pf spill at the game's 0x40/0x3C.
 * (Case 3 with its own pointer instead: 324.) Left: the frame (0x68 to the game's 0x70, so every
 * reload of obj's home is 8 off: pad[9] gives 0x70 but moves view/af/pf 4 down, 288; pad[10]
 * keeps them but makes 0x78, 312); case 3's f, which the game holds in v1 and copies to s0 only
 * on the path that calls eneDlEnvColorLit (here s0 throughout); the model pointer in v0, not t0.
 * 276 to 273: pf set before its kind test (the else's own if). Lead, from the permuter (probe
 * only, not kept: it reads pf before it is set): an "if (pf->pos) {}" at the head of that else,
 * which makes pf live from the function's entry, gives 96 lines; case 3's f then still wants
 * v1, copied to s0 on the call paths (af/pf as copies there: no change, IDO merges them). A
 * natural source where pf (or the frame pointer) is live from entry is what to find. Not it:
 * case 0's else on case 3's f, declared last in pf's place (357). With the probe, case 3 is all
 * that is left: the game reads the model as obj + 0x2C (e folded, s0 free there) into v1 and
 * copies it to s0 only on the two paths that call eneDlEnvColorLit; af/pf as copies in case 3
 * leave it at 96 (no change). An empty argument-only DPRINTF or an "if (0)" PRINTF of pf
 * after the chain: no change (273), both are gone before allocation.
 * Sweep B's find (2026-10-03, score 2125 → 2096): "two = 2;" at state 0's head (declared after
 * view) indexing the four scale[2]/pos[2]: 197, but two addiu more than the game (the C's length
 * 304 for 303), so not kept; one more lead that something live across state 0's calls gives the
 * game's registers. Sweep B2's (score 2175 → 1960): state 0's two calls on the case line, 273
 * alone; with a FAKE do-while round case 3's first eneDlEnvColorLit store, 271 but mix distance 5
 * from 3 (not kept).
 * FAKE review, round 2 (2026-10-04): the pad stays. What the frame proves (IDO_NOTES 182 and
 * 275, and "a pad declared after the last homed local takes no slot"): the game declares
 * exactly ten words above view (0x48-0x6C), then view, af and pf (homed at 0x44/0x40/0x3C), and
 * nothing homed below pf (0x38 is alignment). The four gbi block locals of case 3 (the _g of
 * each gDPSetEnvColor and gSPEndDisplayList) take a word each only when view, af and pf are
 * declared after them. Probes, with view/af/pf declared inside the "if (eneRayHitPush ...)"
 * block: e, f and pad[4] gives a MATCH; the four macros taken out gives a 0x50 frame with view
 * still at 0x44, so the four words between f and view are the _g. Case 3 written on typed
 * block pointers ("AnimeFrame *af = (AnimeFrame *) e->model;" at each branch head, as case 0
 * does, and no f) with pad[3] also gives a MATCH, the code identical. So the original declared,
 * before view in source order, more words than this C can name: four with a shared f, three
 * with typed case 3 pointers (eight between f and view if view/af/pf sat at function scope, as
 * here). They are locals that live only in registers or are never read. Not found as natural
 * C: the ray as three f32 locals ("vx = e->vel[0] * 2;" and so on, passed to eneRayHitPush)
 * gives the exact frame with the typed pointers, but uopt colours them (f0/f2/f12, the 2.0 in
 * f14) where the game multiplies in ugen temps: 124 lines. A block "EneObj *e" in each case
 * moves obj from its home into s0/s1: 340. f as case 3's test variable ("f = e->model;" before
 * the kind test) with typed copies: 274 (301 instructions). Named scale factors would keep the
 * "* 1.0f" multiply (213; not tried). decomp-permuter (natural passes only, on the typed form)
 * fills the three words with single-use temps nobody writes (a gbi constant, pf->pos, the
 * callee's address: score 20). uopt's colouring trace does not apply: the pad moves frame
 * offsets, not registers. */
s32 futtobi_hahen(Obj *obj, s16 msg, void *arg) {
    EneObj *e = ENE(obj);
    Frame *f;
    s32 pad[8]; /* FAKE: the game's eight unused words above view */
    CamView *view;
    AnimeFrame *af;
    PolFrame *pf;

    switch (msg) {
        case 2:
            obj->name = "futtobi_hahen";
            e->count = 40;
            break;
        case 1:
            if (e->model != NULL) {
                frmDestroy(e->model);
            }
            break;
        case 3:
            if (e->model->kind == gFrmKindAnime) {
                f = e->model;
                if (f->parent == gFrmLayers[2]) {
                    ((AnimeFrame *) f)->dl = (*eneDlEnvColorLit)(obj, 0, 0xFF, 0xFF, 0xFF, 1);
                } else {
                    ((AnimeFrame *) f)->dl = gGfxDlHead;
                    gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 255);
                    gSPEndDisplayList(gGfxDlHead++);
                }
            } else if (e->model->kind == gFrmKindPol) {
                f = e->model;
                if (f->parent == gFrmLayers[2]) {
                    ((PolFrame *) f)->dl = (*eneDlEnvColorLit)(obj, 0, 0xFF, 0xFF, 0xFF, 1);
                } else {
                    ((PolFrame *) f)->dl = gGfxDlHead;
                    gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 255);
                    gSPEndDisplayList(gGfxDlHead++);
                }
            }
            break;
        case 0:
            switch (e->state) {
                case 0:
                    eneMove(e);
                    eneFall(obj, 0.4f);
                    if (eneRayHitPush(obj, e->h, e->vel[0] * 2, e->vel[1] * 2, e->vel[2] * 2)) {
                        view = gGameWork.view;
                        if (e->model->kind == gFrmKindAnime) {
                            af = (AnimeFrame *) e->model;
                            af->scale[0] *= 1.3f;
                            af->scale[1] *= 1.0f;
                            af->scale[2] *= 0.5f;
                            if (soundInFovCheck(view, af->pos[0], af->pos[1], af->pos[2])) {
                                SoundLocation2(0x2F9, view, af->pos[0], af->pos[1], af->pos[2], 100.0f, 800.0f,
                                                       view->fovy);
                            }
                        } else if (e->model->kind == gFrmKindPol) {
                            pf = (PolFrame *) e->model;
                            pf->scale[0] *= 1.3f;
                            pf->scale[1] *= 1.0f;
                            pf->scale[2] *= 0.5f;
                            if (soundInFovCheck(view, pf->pos[0], pf->pos[1], pf->pos[2])) {
                                SoundLocation2(0x2F9, view, pf->pos[0], pf->pos[1], pf->pos[2], 100.0f,
                                                       800.0f, view->fovy);
                            }
                        }
                        e->count = 8;
                        e->state = 1;
                        e->y -= e->h;
                    }
                    if (--e->count == 0) {
                        e->lit2 = 0;
                        return -1;
                    }
                    break;
                case 1:
                    e->x = (f32) (e->count-- & 1) - 0.5f + e->x;
                    if (e->count == 0) {
                        eneSetSd(obj, 0x405);
                        itexPuffYellow(e->x, e->y, e->z, 0.08f);
                        if (e->lit2) {
                            effectFspiritCreate(e->x, e->y, e->z, 0.1f, 0xE0, 0xE0, 0xE0, 0x10, 0x10, 0x10);
                        }
                        return -1;
                    }
                    break;
            }
            eneSetModel(obj, e->model);
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Makes a "futtobi_hahen" piece's model (a "pol" frame drawing dl from from's files; under
 * the second root with second) at from, moving at (vx, vy, vz), facing along that (turn 1:
 * sideways; 2: backwards and flagged 7; else backwards), with the sound 0x21A. */
Frame *eneFuttobiHahenModel(Obj *o, EneObj *from, u32 dl, f32 h, f32 vx, f32 vy, f32 vz, f32 scale, u8 lit,
                             u8 turn, u8 second) {
    EneObj *e = ENE(o);
    PolFrame *f;
    EneKind *kind = from->kind;
    s32 pad;
    Vec3f pos;

    if (from->zone) {
        vramCoursePointXZ(from->zone, from->x, from->z, &pos.x, &pos.z);
    } /* sic: off a course, pos is never set */
    f = (PolFrame *) frmCreate(second ? gFrmLayer2 : gFrmModelLayer, gFrmKindPol);
    if (f != NULL) {
        e->h = h;
        e->zone = 0;
        f->pos[0] = pos.x;
        e->x = f->pos[0];
        e->y = f->pos[1] = from->y;
        f->pos[1] = pos.z; /* sic: pos[2] is never set */
        e->z = f->pos[1];
        e->vel[0] = vx;
        e->vel[1] = vy;
        e->vel[2] = vz;
        if (turn == 1) {
            e->rot[0] = 0.0f;
            e->rot[1] = mathAtan2(e->vel[0], e->vel[2]) + 1.5707964f;
            e->rot[2] = 0.0f;
        } else if (turn == 2) {
            e->rot[0] = 0.0f;
            e->rot[1] = mathAtan2(e->vel[0], e->vel[2]) + 3.1415927f;
            e->rot[2] = 0.0f;
            f->flags |= 7;
        } else {
            e->rot[0] = 0.0f;
            e->rot[1] = mathAtan2(e->vel[0], e->vel[2]) + 3.1415927f;
            e->rot[2] = 0.0f;
        }
        e->lit2 = lit;
        f->scale[0] = f->scale[1] = f->scale[2] = scale;
        f->dl2 = (Gfx *) dl;
        f->flags |= 0x20;
        f->segs[0] = fileRamAddr(kind->files.file[0]);
        f->segs[1] = fileRamAddr(kind->files.file[1]);
        f->segs[2] = fileRamAddr(kind->files.file[2]);
        e->model = (Frame *) f;
        soundIncamCam(0x21A, e->x, e->y, e->z);
        eneSetModel(o, e->model);
    }
    return (Frame *) f;
}

/* The same with an animated model (an "anime" frame playing anim from frame, or from its last
 * frame if frame is past it). Its two prints are argument-only (as `if (0)` PRINTFs they move
 * every temporary after them). */
Frame *eneFuttobiHahenAnime(Obj *o, EneObj *from, u32 anim, f32 frame, f32 h, f32 vx, f32 vy, f32 vz,
                             f32 scale, u8 lit, u8 turn, u8 second) {
    EneObj *e = ENE(o);
    AnimeFrame *f;
    EneKind *kind = from->kind;
    s32 pad;
    Vec3f pos;
    s32 len;
    s32 pad2[2];

    DPRINTF("ANIME FRAME MAKE START\n");
    if (from->zone) {
        vramCoursePointXZ(from->zone, from->x, from->z, &pos.x, &pos.z);
    } /* sic: off a course, pos is never set */
    f = (AnimeFrame *) frmCreate(second ? gFrmLayer2 : gFrmModelLayer, gFrmKindAnime);
    if (f != NULL) {
        e->h = h;
        e->zone = 0;
        f->pos[0] = pos.x;
        e->x = f->pos[0];
        e->y = f->pos[1] = from->y;
        f->pos[1] = pos.z; /* sic: pos[2] is never set */
        e->z = f->pos[1];
        e->vel[0] = vx;
        e->vel[1] = vy;
        e->vel[2] = vz;
        if (turn == 1) {
            e->rot[0] = 0.0f;
            e->rot[1] = mathAtan2(e->vel[0], e->vel[2]) + 1.5707964f;
            e->rot[2] = 0.0f;
        } else if (turn == 2) {
            e->rot[0] = 0.0f;
            e->rot[1] = mathAtan2(e->vel[0], e->vel[2]) + 3.1415927f;
            e->rot[2] = 0.0f;
            f->flags |= 7;
        } else {
            e->rot[0] = 0.0f;
            e->rot[1] = mathAtan2(e->vel[0], e->vel[2]) + 3.1415927f;
            e->rot[2] = 0.0f;
        }
        e->lit2 = lit;
        f->scale[0] = f->scale[1] = f->scale[2] = scale;
        f->anim = anim;
        f->flags |= 0x20;
        f->segs[0] = fileRamAddr(kind->files.file[0]);
        f->segs[1] = fileRamAddr(kind->files.file[1]);
        f->segs[2] = fileRamAddr(kind->files.file[2]);
        len = polAnimeLastFrame(f);
        if (len < frame) {
            f->frame = len;
        } else {
            f->frame = frame;
        }
        DPRINTF("F->time = %f, time = %f, max_time = %f,ANIME FRAME MAKE END\n", f->frame, frame, (f32) len);
        e->model = (Frame *) f;
        soundIncamCam(0x21A, e->x, e->y, e->z);
        eneSetModel(o, e->model);
    }
    return (Frame *) f;
}

/* Makes kind 0x237 at a place, with a and the angle (in 512ths of pi) packed as its callback
 * word. */
void eneMakeHitSpark(s32 zone, f32 x, f32 y, f32 z, s16 a, f32 angle) {
    eneMakeEx(NULL, ENE_BASHI, zone, x, y, z, 2,
                           (void *) ((a & 0xFFFF) | (((s16) (angle / 3.1415927f * 512.0f) << 16) & 0xFFFF0000)));
}
