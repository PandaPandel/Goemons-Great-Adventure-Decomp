#include "tlb.h"

/* The "monoeye" enemy kind (archive file 177, kind 0x232) and the three kinds it makes: a one-eyed
 * cloud that floats along its course with six "monocloud" puffs (kind 0x234) circling it, and
 * weeps "monotear" drops (kind 0x233) on the players, each splashing into "monosmoke" (kind
 * 0x235). Type 0 hunts the nearest player (states 2-5: closes in, weeps above it, dashes past),
 * goes home when it strays more than 200 from where it was placed or leaves its course
 * (state 6); type 1 stays put and weeps every 80 steps. Each takes one hit. Their animations are
 * tables, and the state is the animation's number. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

#pragma intrinsic(fabs)

/* An animation of a table: the state with the same number plays it. */
typedef struct MonoAnim {
    /* 0x00 */ u32 anim;
    /* 0x04 */ f32 speed; /* at one retrace a frame */
    /* 0x08 */ u8 loop; /* 1: loops (eneAnimLoop), else plays once */
} MonoAnim; /* size 0x0C */

/* A monoeye's or a monotear's work, at its task + 0x20: an enemy's (EneObj) up to 0x40, then
 * its own. */
typedef struct Monoeye {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ MPolFrame *shadow;
    /* 0x40 */ f32 animSpeed; /* its animation's step at this frame rate */
    /* 0x44 */ f32 heading; /* state 5's dash */
    /* 0x48 */ f32 speedRatio; /* its speed over its top speed (the puffs circle faster) */
    /* 0x4C */ f32 homeX; /* where it was placed, along its course (eneTeritoryCheck) */
    /* 0x50 */ u8 homeZone; /* and its course */
    /* 0x51 */ u8 type; /* its placement's type, from eneSetup */
    /* 0x52 */ u8 anim; /* its animation in the table, and its state */
    /* 0x53 */ u8 life; /* hits it takes */
    /* 0x54 */ s8 gravity; /* a tear's fall, in hundredths */
    /* 0x55 */ s8 dir; /* 0: the nearest player is on its way 1, 1: on the other side */
    /* 0x56 */ s8 target; /* the nearest player (-1: none); a tear: it hit something */
    /* 0x57 */ s8 flash; /* steps left of the hit flash */
    /* 0x58 */ u8 shot; /* this weep's tear is made */
    /* 0x59 */ u8 wait; /* type 1: steps to the next weep */
    /* 0x5A */ u8 splashed; /* a tear: it has hit the water */
    /* 0x5B */ u8 near; /* steps it has hovered above its player */
} Monoeye;

/* A monocloud's work: one of the puffs circling a monoeye. */
typedef struct Monocloud {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z; /* made with its number, 0-5 */
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ Obj *parent; /* its monoeye (eneSetup's result) */
    /* 0x38 */ u8 primR;
    /* 0x39 */ u8 primG;
    /* 0x3A */ u8 primB;
    /* 0x3B */ u8 envR;
    /* 0x3C */ u8 envG;
    /* 0x3D */ u8 envB;
    /* 0x3E */ u8 pad3E[2];
    /* 0x40 */ f32 angle; /* around the monoeye */
    /* 0x44 */ f32 speed; /* its turn a step */
    /* 0x48 */ f32 radius;
    /* 0x4C */ u8 state;
    /* 0x4D */ u8 alpha;
    /* 0x4E */ u8 index; /* its number, 0-5: where on the circle it starts */
} Monocloud;

/* A monosmoke's work: a tear's splash. */
typedef struct Monosmoke {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u8 primR;
    /* 0x35 */ u8 primG;
    /* 0x36 */ u8 primB;
    /* 0x37 */ u8 envR;
    /* 0x38 */ u8 envG;
    /* 0x39 */ u8 envB;
    /* 0x3A */ u8 pad3A[2];
    /* 0x3C */ ACollFrame *atk;
    /* 0x40 */ Anim anim; /* its puff texture's */
    /* 0x50 */ u8 state;
    /* 0x51 */ u8 alpha;
    /* 0x52 */ u8 delay; /* steps before it shows (eneSetup's result) */
} Monosmoke;

#define MONO(obj) ((Monoeye *) FX(obj))
#define CLOUD(obj) ((Monocloud *) FX(obj))
#define SMOKE(obj) ((Monosmoke *) FX(obj))

extern u8 monoeyeTable0Anim[];
MonoAnim monoeyeAnimTable0[] = {
    { (u32) monoeyeTable0Anim, 0.0f, 0 },
    { (u32) monoeyeTable0Anim, 0.0f, 0 },
    { (u32) monoeyeTable0Anim, 2.0f, 1 },
    { 0x03000310, 5.0f, 0 },
    { 0x03000310, -5.0f, 0 },
    { (u32) monoeyeTable0Anim, 2.0f, 1 },
    { (u32) monoeyeTable0Anim, 1.0f, 1 },
    { (u32) monoeyeTable0Anim, 0.0f, 0 },
    { (u32) monoeyeTable0Anim, 0.0f, 0 },
    { 0x00000000, 0.0f, 0 },
}; /* type 0's animations */
MonoAnim monoeyeTearAnimTable[] = {
    { 0x03000364, 0.0f, 0 },
    { 0x03000364, 1.5f, 0 },
    { 0x03000364, 0.0f, 0 },
    { 0x03000364, 0.0f, 0 },
}; /* a tear's */
MonoAnim monoeyeAnimTable1[] = {
    { (u32) monoeyeTable0Anim, 0.0f, 0 },
    { (u32) monoeyeTable0Anim, 0.0f, 0 },
    { 0x03000310, 5.0f, 0 },
    { 0x03000310, -5.0f, 0 },
    { (u32) monoeyeTable0Anim, 0.0f, 0 },
    { (u32) monoeyeTable0Anim, 0.0f, 0 },
    { 0x00000000, 0.0f, 0 },
}; /* type 1's */
extern u8 monoeyeHalf0Dl[];
extern u8 monoeyeHalf1Dl[];
extern Gfx monoeyeCloudDl[];
extern Gfx monoeyeSmokeDl[];

s32 monoeyeMain0(Obj *o);
void monoeyeAimNearestPl(Obj *o, f32 *x, f32 *y, f32 *z);
s32 monoeyeMain1(Obj *o);

/* Sets animation n of the table: its step, scaled to the frame rate, from its first frame. */
void monoeyeSetAnim(Obj *o, u8 n, MonoAnim *tab) {
    Monoeye *w = MONO(o);

    w->anim = n;
    w->animSpeed = tab[n].speed / gRetracesPerFrame;
    w->model->anim = tab[n].anim;
    w->model->frame = 0.0f;
}

void monoeyeNop1(void) {
}

/* Sets animation n of the table, at its last frame. */
void monoeyeSetAnimEnd(Obj *o, u8 n, MonoAnim *tab) {
    Monoeye *w = MONO(o);
    s32 pad;

    monoeyeSetAnim(o, n, tab);
    w->model->frame = polAnimeLastFrame(w->model);
}

void monoeyeNop2(void) {
}

/* Puts it back on its course and returns its heading there: mode 0 the heading less ang, 1 the
 * heading turned by ang and a half turn, -1 its own rotation, else the heading. */
f32 monoeyeCourseAngle(Obj *o, f32 ang, s8 mode) {
    Monoeye *w = MONO(o);
    f32 h;
    f32 x;
    f32 y;
    f32 z;
    f32 r;

    w->x = vramCoursePointXZ(w->zone, w->x, w->z, &x, &z);
    r = h = vramCourseHeading(w->zone, w->x);
    if (mode == 0) {
        r = h - ang;
    } else if (mode == 1) {
        r += ang + 3.141592741f;
    } else if (mode == -1) {
        r = w->rot[1];
    }
    return r;
}

/* The distance from it to player i (0 if i is not 0-3 or not in play). */
f32 monoeyePlDist(Obj *o, s32 i) {
    f32 dx;
    f32 dy;
    f32 x;
    f32 y;
    f32 z;
    Vec3f pos;

    if (i < 0 || i >= 4) {
        return 0.0f;
    }
    eneWorldPos(o, &pos);
    if (plIsInPlay(i)) {
        plGetModelPos(gGameWork.players[i], &x, &y, &z);
        dx = x - pos.x;
        dy = y - pos.y;
        return sqrtf(dx * dx + dy * dy + (z - pos.z) * (z - pos.z));
    }
    return 0.0f;
}

/* The nearest player in play (-1: none). */
s32 monoeyeNearestPl(Obj *o) {
    s32 pad[3];
    f32 x;
    f32 y;
    f32 z;
    s32 i;
    f32 d;
    s32 near;
    Vec3f pos;
    f32 min;

    min = 65536.0f;
    near = -1;
    eneWorldPos(o, &pos);
    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plGetModelPos(gGameWork.players[i], &x, &y, &z);
            d = monoeyePlDist(o, i);
            if (d < min) {
                min = d;
                near = i;
            }
        }
    }
    return near;
}

void monoeyeNop3(void) {
}

/* Speeds it up by accel toward ang (in the course's plane: x along it, y up), the y part (scale
 * below 1) or the x part (else) scaled by scale, and holds its speed to max; returns the speed. */
f32 monoeyeAccel(Obj *o, f32 ang, f32 accel, f32 max, f32 scale) {
    Monoeye *w = MONO(o);
    s32 pad;
    s32 pad2;
    f32 ax = mathCos(ang) * accel;
    f32 ay = mathSin(ang) * accel;
    f32 speed;
    f32 k;

    if (scale < 1.0f) {
        ay *= scale;
    } else {
        ax *= scale;
    }
    w->vel[0] += ax;
    w->vel[1] += ay;
    speed = sqrtf(w->vel[1] * w->vel[1] + w->vel[0] * w->vel[0]);
    if (max < speed) {
        k = max / speed;
        w->vel[0] *= k;
        w->vel[1] *= k;
        return max;
    }
    return speed;
}

/* "eneFallLimit": 1 once it is below -600 or above 1000. */
s32 monoeyeFallLimit(Obj *o) {
    Monoeye *w = MONO(o);

    if (w->y < -600.0f) {
        PRINTF("eneFallLimit(-600):\n");
        return 1;
    }
    if (1000.0f < w->y) {
        PRINTF("eneFallLimit(+1000):\n");
        return 1;
    }
    return 0;
}

/* "eneTeritoryCheck": 1 if it is on another course than *zone (which it then takes, with its
 * place in *x) or farther than range from *x along it. */
s32 monoeyeTeritoryCheck(Obj *o, f32 *x, u8 *zone, f32 range) {
    Monoeye *w = MONO(o);

    if (w->zone != *zone) {
        PRINTF("eneTeritoryCheck: new course\n");
        *x = w->x;
        *zone = w->zone;
        return 1;
    }
    if (fabs(w->x - *x) > range) {
        PRINTF("eneTeritoryCheck: out of range\n");
        return 1;
    }
    return 0;
}

/* "monoeye": the kind's proc. */
s32 monoeyeProc(Obj *o, s16 msg, void *arg) {
    Monoeye *w = MONO(o);
    s32 pad;
    s32 r;

    switch (msg) {
        case 2:
            o->name = "monoeye";
            w->type = eneSetup(o, arg);
            w->life = 1;
            w->homeX = w->x;
            w->homeZone = w->zone;
            if ((w->model = eneNewAnime(o, 0)) == NULL) {
                return -1;
            }
            if ((w->shadow = shadowCreate()) == NULL) {
                return -1;
            }
            if ((w->atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, &w->model->f, 0, 0, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, &w->model->f, 0, 0, 0, 0, 0)) == NULL) {
                return -1;
            }
            break;
        case 0:
            if (monoeyeFallLimit(o)) {
                return -1;
            }
            if (acollAtkWasHit(w->atk) == 1 && (Obj *) w->atk->hitPower != NULL) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, o);
            }
            switch (w->type) {
                case 0:
                    r = monoeyeMain0(o);
                    break;
                case 1:
                    r = monoeyeMain1(o);
                    break;
            }
            if (r != 0) {
                return r;
            }
            break;
        case 1:
            /* the first test and its call on one line: as1 then fills the branch's delay slot
             * with the work pointer's addiu, as the game has it */
            if (w->model != NULL) { frmDestroy(&w->model->f); }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        default:
            /* an empty block in w's range: IDO then keeps w in v1, saved around each call, not
             * in s0 (as the game does) */
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Type 0's step: moves, lands, animates, takes its hit (state 8: dies), measures its speed for
 * its puffs, goes home when it strays (state 6), then runs its state: 0 sets up and makes its
 * six puffs, 1 waits for a player within 80, 2 closes in above it, 3 weeps, 4 picks a dash
 * past it, 5 dashes. */
/* Matched 2026-10-12; it was parked for its stack frame and the order of mathAtan2's sums.
 * r and dx are block locals of case 4's if, and the scale chain, case 6's angle and the second
 * random each go through a local (size, a, rx), as the game's frame has them. */
s32 monoeyeMain0(Obj *o) {
    Monoeye *w = MONO(o);
    f32 a;
    s32 land;
    s32 wall;
    s32 end;
    s32 i;
    f32 size;
    f32 x2;
    f32 y2;
    f32 z2;
    f32 x3;
    f32 y3;
    f32 z3;
    f32 rx;
    f32 x4;
    f32 y4;
    f32 z4;
    f32 x5;
    f32 y5;
    f32 z5;
    f32 speed;

    eneMove((EneObj *) w);
    land = eneLandOff((EneObj *) w, 10.0f, -20.0f);
    wall = eneWallCheckNoPush(o, 10.0f, 15.0f, 0.0f, -20.0f);
    eneSetModelOnCourse(o, &w->model->f);
    if (monoeyeAnimTable0[w->anim].loop == 1) {
        end = eneAnimLoop(o, w->model, w->animSpeed);
    } else {
        end = eneAnimPlayOnce(o, w->model, w->animSpeed);
    }
    if (w->shadow != NULL) {
        shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 4.0f, NULL);
    }
    if (acollDmgWasHit(w->dmg) == 1) {
        if (--w->life == 0) {
            w->dmg->flags &= ~2;
            w->atk->flags &= ~2;
            monoeyeSetAnim(o, 8, monoeyeAnimTable0);
        } else {
            w->flash = 24 / gRetracesPerFrame;
            if (eneNearestPlWay(o) == 1) {
                w->vel[0] = gRetracesPerFrame * -1.0;
            }
            if (eneNearestPlWay(o) == 4) {
                w->vel[0] = gRetracesPerFrame * 1.0;
            }
            monoeyeSetAnim(o, 7, monoeyeAnimTable0);
        }
    }
    speed = sqrtf(w->vel[1] * w->vel[1] + w->vel[0] * w->vel[0]);
    w->speedRatio = speed / sqrtf((gRetracesPerFrame * 2.0f) * (gRetracesPerFrame * 2.0f) + (0.75f * gRetracesPerFrame) * (0.75f * gRetracesPerFrame));
    w->target = monoeyeNearestPl(o);
    if (w->anim != 6 && monoeyeTeritoryCheck(o, &w->homeX, &w->homeZone, 200.0f)) {
        PRINTF("**** monoeye:go home\n");
        w->vel[0] = w->vel[1] = 0.0f;
        monoeyeSetAnim(o, 6, monoeyeAnimTable0);
    }
    switch (w->anim) {
        case 0:
            w->atk->radius = 150;
            w->atk->kind = 120;
            w->dmg->radius = 150;
            w->dmg->height = 300;
            w->dmg->ofs[1] = -150;
            w->model->scale[2] = 0.05999999866f;
            size = w->model->scale[2];
            w->model->scale[1] = size;
            w->model->scale[0] = size;
            if (eneNearestPlWay(o) == 1) {
                w->dir = 0;
            } else {
                w->dir = 1;
            }
            w->rot[1] = monoeyeCourseAngle(o, 0.7853981853f, w->dir);
            w->rot[0] = 0.5235987902f;
            w->shot = 0;
            w->near = 0;
            for (i = 0; i < 6; i++) {
                eneMakeEx(o, ENE_MONOEYE_CLOUD, w->zone, w->x, w->y, i, 2, (u32 (*)(void *, void *, void *)) o);
            }
            monoeyeSetAnim(o, 1, monoeyeAnimTable0);
            break;
        case 1:
            if (w->target >= 0 && monoeyePlDist(o, w->target) < 80.0f) {
                monoeyeSetAnim(o, 2, monoeyeAnimTable0);
            }
            break;
        case 2:
            monoeyeAimNearestPl(o, &x2, &y2, &z2);
            if (w->dir == 0) {
                a = 0.0f;
            } else {
                a = 3.141592741f;
            }
            monoeyeAccel(o, a, 0.3f, gRetracesPerFrame * 2.0f, 1.0f);
            if (y2 + 20.0f + 30.0f <= w->y) {
                a = 4.712388992f;
            } else {
                a = 1.570796371f;
            }
            monoeyeAccel(o, a, 0.15f, gRetracesPerFrame * 0.75f, 1.0f);
            if (land) {
                w->vel[1] *= -1.0f;
            }
            if (wall == -1) {
                w->vel[0] *= -1.0f;
            }
            if (fabs(x2 - w->x) < 5.0 && fabs(y2 - w->y) < 55.0) {
                if (++w->near >= 40) {
                    monoeyeSetAnim(o, 3, monoeyeAnimTable0);
                }
            } else {
                w->near = 0;
            }
            break;
        case 3:
            w->vel[0] *= 0.8000000119f;
            w->vel[1] *= 0.8000000119f;
            monoeyeAimNearestPl(o, &x3, &y3, &z3);
            if (w->shot == 0 && w->model->frame >= 8.0f) {
                eneMakeEx(NULL, ENE_MONOEYE_TEAR, w->zone, w->x, w->y - 5.0f, w->z, 2,
                          (u32 (*)(void *, void *, void *)) (((s16) (w->rot[1] * 57.29577637f) << 16) & 0xFFFF0000));
                w->shot = 1;
            }
            if (end) {
                w->shot = 0;
                monoeyeSetAnimEnd(o, 4, monoeyeAnimTable0);
            }
            break;
        case 4:
            w->vel[0] *= 0.9499999881f;
            w->vel[1] *= 0.9499999881f;
            if (end) {
                f32 dx;
                f32 r;

                monoeyeAimNearestPl(o, &x4, &y4, &z4);
                r = fadeRandRange(-5, 5);
                if ((gGameFrameCount & 1) == 0) {
                    dx = -40.0f;
                } else {
                    dx = 40.0f;
                }
                rx = fadeRandRange(-5, 5);
                /* FAKE: the (f32) casts (no-ops) put each random first in its sum, as the game adds */
                w->heading = mathAtan2((f32) r + (y4 + 20.0f + 25.0f - w->y), (f32) rx + (x4 + dx - w->x));
                monoeyeSetAnim(o, 5, monoeyeAnimTable0);
            }
            break;
        case 5:
            monoeyeAimNearestPl(o, &x5, &y5, &z5);
            monoeyeAccel(o, w->heading, 0.05f, gRetracesPerFrame * 0.75f, 1.0f);
            if (land) {
                w->vel[1] *= -1.0f;
            }
            if (wall == -1) {
                w->vel[0] *= -1.0f;
            }
            if (fabs(x5 - w->x) > 50.0) {
                monoeyeSetAnim(o, 2, monoeyeAnimTable0);
            }
            break;
        case 6:
            if (land) {
                w->vel[1] = 0;
            }
            if (w->x < w->homeX) {
                w->dir = 0;
            } else {
                w->dir = 1;
            }
            a = monoeyeCourseAngle(o, 0.7853981853f, w->dir);
            eneTurnToward(o, a, gRetracesPerFrame * 0.104719758f);
            if (w->dir == 0) {
                w->vel[0] = gRetracesPerFrame * 0.5;
            } else if (w->dir == 1) {
                w->vel[0] = gRetracesPerFrame * -0.5;
            }
            if (fabs(w->x - w->homeX) <= 0.5) {
                w->vel[0] = 0;
                monoeyeSetAnim(o, 1, monoeyeAnimTable0);
            }
            break;
        case 7:
            monoeyeSetAnim(o, 2, monoeyeAnimTable0);
            break;
        case 8:
            shimEneDeathAct(o, w->dmg, (u32) monoeyeHalf0Dl, (u32) monoeyeHalf1Dl);
            eneReleaseSpawn(w->spawn);
            PRINTF("*** monoeye(type %d):dead ***\n", w->type);
            return -1;
    }
    return 0;
}

/* Finds the nearest player, puts its place in *x, *y and *z and turns toward it (sharper when
 * it is within 15 along the course). */
void monoeyeAimNearestPl(Obj *o, f32 *x, f32 *y, f32 *z) {
    Monoeye *w = MONO(o);
    s32 pad;
    s32 zone;

    w->target = monoeyeNearestPl(o);
    if (w->target >= 0) {
        plAskPos(gGameWork.players[w->target], x, y, z, &zone);
        if (eneNearestPlWay(o) == 1) {
            w->dir = 0;
        } else {
            w->dir = 1;
        }
        if (fabs(*x - w->x) < 15.0) {
            eneTurnToward(o, monoeyeCourseAngle(o, 1.570796371f, w->dir), gRetracesPerFrame * 0.104719758f);
        } else {
            eneTurnToward(o, monoeyeCourseAngle(o, 0.7853981853f, w->dir), gRetracesPerFrame * 0.104719758f);
        }
    }
}

/* "monotear": a drop a monoeye weeps (its placement value's high half is its heading in
 * degrees). It falls, splashes once if it enters water, and on the ground or on a hit stops and
 * bursts into three monosmoke puffs, then shrinks away. */
/* Matched 2026-10-12: one local (ret) for eneSetup's result and eneLandOff's, and the burst
 * loop as "i < 3; i++" passing "(i * 6)", give the game's stack frame. "w->type = (u16) ret"
 * spends the game's register; the vel[1] < 0 test is against an int 0. */
s32 monoeyeTearProc(Obj *o, s16 msg, void *arg) {
    Monoeye *w = MONO(o);
    s32 pad;
    s32 ret;
    s32 pad2[2];
    Vec3f p;
    Vec3f push;
    s32 i;

    switch (msg) {
        case 2:
            o->name = "monotear";
            ret = eneSetup(o, arg);
            w->type = (u16) ret; /* the placement value's low half */
            w->rot[1] = (ret >> 16) * 0.01745329238f;
            w->anim = 0;
            w->gravity = 20;
            w->target = 0;
            if ((w->model = eneNewAnime(o, 0)) == NULL) {
                return -1;
            }
            if ((w->atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, &w->model->f, 0, 0, 0, 0)) == NULL) {
                return -1;
            }
            break;
        case 0:
            if (monoeyeFallLimit(o)) {
                return -1;
            }
            w->target = 0;
            if (acollAtkWasHit(w->atk) == 1 && (Obj *) w->atk->hitPower != NULL) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, o);
                w->target = 1;
            }
            eneFall(o, w->gravity * 0.009999999776f * gRetracesPerFrame);
            eneMove((EneObj *) w);
            ret = eneLandOff((EneObj *) w, 5.0f, -1.0f);
            eneSetModelOnCourse(o, &w->model->f);
            if (monoeyeTearAnimTable[w->anim].loop == 1) {
                eneAnimLoop(o, w->model, w->animSpeed);
            } else {
                eneAnimPlayOnce(o, w->model, w->animSpeed);
            }
            switch (w->anim) {
                case 0:
                    w->atk->radius = 50;
                    w->atk->kind = 120;
                    w->model->scale[2] = 0.05999999866f;
                    w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                    if (eneNearestPlWay(o) == 1) {
                        w->dir = 0;
                    } else {
                        w->dir = 1;
                    }
                    eneSetSd(o, 0x2D2);
                    monoeyeSetAnim(o, 1, monoeyeTearAnimTable);
                    break;
                case 1:
                    if (w->vel[1] < 0 && w->splashed == 0) {
                        p.x = w->model->pos[0];
                        p.y = w->model->pos[1];
                        p.z = w->model->pos[2];
                        if (rideTree2PointIn(&p, &push)) {
                            eneSetSd(o, 0x298);
                            effectSibukiCreateWhite(p.x, p.y, p.z, 0.009999999776f, 0.009999999776f,
                                                    0.004999999888f, 0.004999999888f);
                            w->splashed = 1;
                        }
                    }
                    if (ret || w->target) {
                        w->gravity = 0;
                        w->vel[2] = w->vel[1] = w->vel[0] = 0.0f;
                        w->atk->flags &= ~2;
                        for (i = 0; i < 3; i++) {
                            eneMakeEx(NULL, ENE_MONOEYE_SMOKE, w->zone, w->x, w->y + 4.0f, w->z, 2,
                                      (u32 (*)(void *, void *, void *)) (i * 6));
                        }
                        eneSetSd(o, 0x2D4);
                        monoeyeSetAnim(o, 2, monoeyeTearAnimTable);
                    }
                    break;
                case 2:
                    if (w->model->scale[0] > 0.003000000026f) {
                        w->model->scale[0] -= 0.003000000026f;
                    } else {
                        w->model->scale[0] = 0.0f;
                        monoeyeSetAnim(o, 3, monoeyeTearAnimTable);
                    }
                    w->model->scale[1] -= 0.003000000026f;
                    w->model->scale[2] = w->model->scale[0];
                    break;
                case 3:
                    return -1;
            }
            break;
        case 1:
            if (w->model != NULL) { frmDestroy(&w->model->f); }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "monocloud": one of the six puffs circling a monoeye (its placement's z is its number). It
 * circles faster the faster its monoeye moves, tilted with it, and is gone with it. */
s32 monoeyeCloudProc(Obj *o, s16 msg, void *arg) {
    Monocloud *w = CLOUD(o);
    s32 pad;
    Monoeye *p;
    f32 k;
    f32 a;
    f32 dx;
    f32 dy;
    f32 dz;

    switch (msg) {
        case 2:
            o->name = "monocloud";
            w->parent = (Obj *) eneSetup(o, arg);
            w->index = w->z;
            if ((w->model = eneNewPol(o, NULL)) == NULL) {
                return -1;
            }
            frmMove(&w->model->f, gFrmEffectLayer);
            break;
        case 0:
            eneMove((EneObj *) w);
            eneSetModelOnCourse(o, &w->model->f);
            switch (w->state) {
                case 0:
                    w->model->dl2 = monoeyeCloudDl;
                    w->model->flags |= 7;
                    w->primB = 0xFF;
                    w->primG = 0xFF;
                    w->primR = 0xFF;
                    w->envB = 0xFF;
                    w->envG = 0xFF;
                    w->envR = 0xFF;
                    w->angle = w->index * 1.04719758f;
                    w->speed = fadeRandRange(15, 20) * 0.01745329238f;
                    w->radius = fadeRandRange(9, 12);
                    w->state = 1;
                    break;
                case 1:
                    p = MONO(w->parent);
                    if (p == NULL || !eneIsEnemy(o)) {
                        w->state = 3;
                        break;
                    }
                    k = p->speedRatio;
                    w->angle += w->speed * (0.3000000119f + 0.6999999881f * k);
                    a = w->angle;
                    dx = mathCos(a) * w->radius;
                    dy = (mathSin(a) * w->radius - 5.0f) * 0.8500000238f;
                    dz = -5.0f;
                    mathVecRotate(p->rot[0], p->rot[1], p->rot[2], &dx, &dy, &dz);
                    w->x = p->x + dx;
                    w->y = p->y + dy;
                    w->z = p->z + dz;
                    w->model->scale[0] = mathSin(a) * 0.009599998593f + 0.03840000182f;
                    w->model->scale[1] = w->model->scale[2] = w->model->scale[0];
                    w->alpha = mathSin(a + 1.04719758f) * 0 + 144.0f;
                    break;
                case 2:
                    break;
                case 3:
                    return -1;
            }
            break;
        case 3:
            if (w->alpha >= 0x100) { w->alpha = 0xFF; }
            w->model->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gSPDisplayList(gGfxDlHead++, gDrawSetupCldSurfDl);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0,
                              ENVIRONMENT, 0, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0,
                              ENVIRONMENT, 0);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->primR, w->primG, w->primB, 0);
            gDPSetEnvColor(gGfxDlHead++, w->envR, w->envG, w->envB, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "monosmoke": a puff a monotear bursts into (its placement value is its delay in steps). It
 * hurts while it is thick, grows and fades. */
s32 monoeyeSmokeProc(Obj *o, s16 msg, void *arg) {
    Monosmoke *w = SMOKE(o);

    switch (msg) {
        case 2:
            o->name = "monosmoke";
            w->delay = eneSetup(o, arg);
            if ((w->model = eneNewPol(o, NULL)) == NULL) {
                return -1;
            }
            frmMove(&w->model->f, gFrmEffectLayer);
            if ((w->atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, &w->model->f, 0, 0, 0, 0)) == NULL) {
                return -1;
            }
            break;
        case 0:
            if (monoeyeFallLimit(o)) {
                return -1;
            }
            if (acollAtkWasHit(w->atk) == 1 && (Obj *) w->atk->hitPower != NULL) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, o);
            }
            eneMove((EneObj *) w);
            eneSetModelOnCourse(o, &w->model->f);
            switch (w->state) {
                case 0:
                    if (w->delay-- != 0) {
                        break;
                    }
                    w->model->dl2 = monoeyeSmokeDl;
                    w->model->flags |= 3;
                    fadeAnimStart(&w->anim, gItexPuffAnim, 0, &w->model->segs[3], floadGetSetFile(0x123, 0));
                    w->primR = 0xFF;
                    w->primB = 0xFF;
                    w->primG = 0;
                    w->envR = 0xFF;
                    w->envB = 0xFF;
                    w->envG = 0;
                    w->model->scale[2] = 0.25f;
                    w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                    w->alpha = 0xC0;
                    w->atk->radius = 30;
                    w->atk->kind = 120;
                    w->vel[1] = 0.400000006f;
                    w->state = 1;
                    break;
                case 1:
                    if (w->alpha - 6 < 6) {
                        w->alpha = 0;
                        w->state = 2;
                    } else {
                        w->alpha -= 6;
                    }
                    w->model->scale[0] += 0.009999999776f;
                    w->model->scale[1] = w->model->scale[2] = w->model->scale[0];
                    fadeAnimStep(&w->anim);
                    if (w->alpha < 0x71) {
                        w->atk->flags &= ~2;
                    }
                    break;
                case 2:
                    return -1;
            }
            break;
        case 3:
            if (w->alpha >= 0x100) { w->alpha = 0xFF; }
            w->model->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gSPDisplayList(gGfxDlHead++, gDrawSetupCldSurfDl);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0,
                              ENVIRONMENT, 0, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0,
                              ENVIRONMENT, 0);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->primR, w->primG, w->primB, 0);
            gDPSetEnvColor(gGfxDlHead++, w->envR, w->envG, w->envB, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            if (w->model != NULL) { frmDestroy(&w->model->f); }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Type 1's step: as type 0's without the hunt; it weeps at the nearest player every 80 steps
 * (state 2), taking its one hit (state 5: dies). */
s32 monoeyeMain1(Obj *o) {
    Monoeye *w = MONO(o);
    s32 pad;
    s32 end;
    f32 x;
    f32 y;
    f32 z;
    s32 i;

    eneMove((EneObj *) w);
    eneSetModelOnCourse(o, &w->model->f);
    if (monoeyeAnimTable1[w->anim].loop == 1) {
        end = eneAnimLoop(o, w->model, w->animSpeed);
    } else {
        end = eneAnimPlayOnce(o, w->model, w->animSpeed);
    }
    if (w->shadow != NULL) {
        shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 4.0f, NULL);
    }
    if (acollDmgWasHit(w->dmg) == 1) {
        if (--w->life == 0) {
            w->dmg->flags &= ~2;
            w->atk->flags &= ~2;
            monoeyeSetAnim(o, 5, monoeyeAnimTable1);
        } else {
            w->flash = 24 / gRetracesPerFrame;
            if (eneNearestPlWay(o) == 1) {
                w->vel[0] = gRetracesPerFrame * -1.0;
            }
            if (eneNearestPlWay(o) == 4) {
                w->vel[0] = gRetracesPerFrame * 1.0;
            }
            monoeyeSetAnim(o, 4, monoeyeAnimTable1);
        }
    }
    switch (w->anim) {
        case 0:
            w->atk->radius = 150;
            w->atk->kind = 120;
            w->dmg->radius = 180;
            w->dmg->height = 300;
            w->dmg->ofs[1] = -150;
            w->model->scale[2] = 0.05999999866f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            if (eneNearestPlWay(o) == 1) {
                w->dir = 0;
            } else {
                w->dir = 1;
            }
            w->rot[1] = monoeyeCourseAngle(o, 0.7853981853f, w->dir);
            w->rot[0] = 0.5235987902f;
            w->shot = 0;
            for (i = 0; i < 6; i++) {
                eneMakeEx(o, ENE_MONOEYE_CLOUD, w->zone, w->x, w->y, i, 2, (u32 (*)(void *, void *, void *)) o);
            }
            monoeyeSetAnim(o, 1, monoeyeAnimTable1);
            break;
        case 1:
            w->target = monoeyeNearestPl(o);
            if (w->target >= 0) {
                monoeyeAimNearestPl(o, &x, &y, &z);
                if (w->wait-- == 0) {
                    monoeyeSetAnim(o, 2, monoeyeAnimTable1);
                }
            }
            break;
        case 2:
            monoeyeAimNearestPl(o, &x, &y, &z);
            if (w->shot == 0 && w->model->frame >= 8.0f) {
                eneMakeEx(NULL, ENE_MONOEYE_TEAR, w->zone, w->x, w->y - 5.0f, w->z, 2,
                          (u32 (*)(void *, void *, void *)) (((s16) (w->rot[1] * 57.29577637f) << 16) & 0xFFFF0000));
                w->shot = 1;
            }
            if (end) {
                w->shot = 0;
                monoeyeSetAnimEnd(o, 3, monoeyeAnimTable1);
            }
            break;
        case 3:
            if (end) {
                w->wait = 80;
                monoeyeSetAnim(o, 1, monoeyeAnimTable1);
            }
            break;
        case 4:
            monoeyeSetAnim(o, 1, monoeyeAnimTable1);
            break;
        case 5:
            shimEneDeathAct(o, w->dmg, (u32) monoeyeHalf0Dl, (u32) monoeyeHalf1Dl);
            eneReleaseSpawn(w->spawn);
            PRINTF("*** monoeye(type %d):dead ***\n", w->type);
            return -1;
    }
    return 0;
}
