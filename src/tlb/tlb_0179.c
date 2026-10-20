#include "tlb.h"

/* The "area3kappa" enemy kind (archive file 179) and its company: "a3kp_g", a generator that puts
 * out an area3kappa (kind 0x238) every 91 steps, passing on its own placement value; "a3kpfire",
 * a yellow flame; and "area3beard" (its death print says area3backbeard). An area3kappa's
 * placement type (the value's low byte) picks its step: 0 (a3kpLeapStep) hides till a player is
 * within 100, leaps (by the value's high half) and bites when level with the player; 1
 * (a3kpWalkStep) then walks after the nearest player, leaping off edges, and bites within 20;
 * 2 (a3kpSpinStep) is drawn as a "mani" aligned to the ground and spins; 3 (a3kpTurnStep)
 * turns in place and is knocked up by each hit. */

#pragma intrinsic(fabs)

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(ACollFrame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(ACollFrame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

/* An animation table's entry: the animation, its speed at 60 steps a second, and whether it
 * loops. */
typedef struct A3KpAnim {
    /* 0x00 */ u32 anim;
    /* 0x04 */ f32 speed;
    /* 0x08 */ u8 loop;
} A3KpAnim;

/* An area3kappa's (and an area3beard's, an a3kp_g's) work, at its task + 0x20: an enemy's
 * (EneObj) up to 0x40, then its own. */
typedef struct A3Kappa {
    /* 0x00 */ u8 state;
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
    /* 0x40 */ f32 animSpeed; /* its animation's, per frame */
    /* 0x44 */ f32 value; /* its placement value's high half */
    /* 0x48 */ u8 type; /* its placement value's low byte */
    /* 0x49 */ u8 anim; /* the entry of its type's animation table: its state */
    /* 0x4A */ u8 life;
    /* 0x4B */ s8 gravity; /* hundredths */
    /* 0x4C */ s8 way; /* 0 or 1: the side of the course it faces */
    /* 0x4D */ s8 target; /* the player it goes for (-1: none) */
    /* 0x4E */ u16 time;
    /* 0x50 */ union {
        f32 floor; /* type 0: the height below which it is gone */
        u8 landed; /* types 1 and 2: on the ground last step */
        MAniFrame *mani; /* type 2: its ground-aligned model */
        f32 sway; /* type 3 and area3beard: an angle */
    } u;
} A3Kappa;

#define A3KP(obj) ((A3Kappa *) FX(obj))

/* An a3kpfire's work, at its task + 0x20. */
typedef struct A3KpFire {
    /* 0x00 */ PolFrame *pol;
    /* 0x04 */ Anim anim; /* its texture's */
    /* 0x14 */ s32 alpha;
} A3KpFire;

#define A3KPFIRE(obj) ((A3KpFire *) FX(obj))

A3KpAnim a3kpLeapAnims[] = {
    { 0x00000000, 0.0f, 0 },
    { 0x00000000, 0.0f, 0 },
    { 0x03000528, 0.0f, 0 },
    { 0x03000DC4, 3.0f, 0 },
    { 0x03000528, 6.0f, 1 },
    { 0x03001038, 2.0f, 0 },
    { 0x00000000, 0.0f, 0 },
};
A3KpAnim a3kpWalkAnims[] = {
    { 0x00000000, 0.0f, 0 },
    { 0x00000000, 0.0f, 0 },
    { 0x03000528, 3.0f, 1 },
    { 0x03000528, 6.0f, 1 },
    { 0x03000DC4, 3.0f, 0 },
    { 0x03001038, 2.0f, 0 },
    { 0x00000000, 0.0f, 0 },
};
A3KpAnim a3kpSpinAnims[] = {
    { 0x00000000, 0.0f, 0 },
    { 0x00000000, 0.0f, 0 },
    { 0x03000804, 2.0f, 1 },
    { 0x03001038, 2.0f, 0 },
    { 0x00000000, 0.0f, 0 },
};
A3KpAnim a3kpTurnAnims[] = {
    { 0x00000000, 0.0f, 0 },
    { 0x00000000, 0.0f, 0 },
    { 0x03000804, 1.0f, 1 },
    { 0x03000804, 6.0f, 1 },
    { 0x03001038, 2.0f, 0 },
    { 0x00000000, 0.0f, 0 },
};
A3KpAnim a3kpBeardAnims[] = {
    { 0x00000000, 0.0f, 0 },
    { 0x00000000, 0.0f, 0 },
    { 0x03000374, 1.0f, 1 },
    { 0x03000374, 6.0f, 1 },
    { 0x03000374, 2.0f, 0 },
    { 0x00000000, 0.0f, 0 },
    { 0x00000000, 0.0f, 0 },
};
extern u8 a3kpFireDl[];
extern u8 a3kpHalf0Dl[];
extern u8 a3kpHalf1Dl[];

s32 a3kpFallLimit(Obj *o);
s32 a3kpLeapStep(Obj *o);
s32 a3kpWalkStep(Obj *o);
s32 a3kpSpinStep(Obj *o);
s32 a3kpTurnStep(Obj *o);
void a3kpSplash(Obj *o);
void a3kpMtxStand(f32 m[4][4], f32 x, f32 y, f32 z);
void a3kpZAlign(MAniFrame *m, f32 x, f32 y, f32 z, f32 scale);
s32 a3kpBeardStep(Obj *o);

/* Sets animation n of the table tab, from its first frame. */
void a3kpSetAnim(Obj *o, u8 n, A3KpAnim *tab) {
    A3Kappa *w = A3KP(o);
    A3KpAnim *a;

    w->anim = n;
    /* a u32 sum: "&tab[n]" gives the add's operands the other way round */
    a = (A3KpAnim *) ((u32) tab + n * sizeof(A3KpAnim));
    w->animSpeed = a->speed / gRetracesPerFrame;
    w->model->anim = a->anim;
    w->model->frame = 0.0f;
}

/* Three empty functions. */
void a3kpEmpty0(void) {
}

void a3kpEmpty1(void) {
}

void a3kpEmpty2(void) {
}

/* Puts it back on its course and returns its heading there: mode 0 the heading less ang, 1 the
 * heading turned by ang and a half turn, -1 its own rotation, else the heading. */
f32 a3kpCourseHeading(Obj *o, f32 ang, s8 mode) {
    A3Kappa *w = A3KP(o);
    f32 h;
    f32 x;
    f32 y;
    f32 z;
    f32 r;

    w->x = vramCoursePointXZ(A3KP(o)->zone, A3KP(o)->x, A3KP(o)->z, &x, &z);
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
f32 a3kpPlDist(Obj *o, s32 i) {
    s32 pad[2];
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
        return sqrtf((x - pos.x) * (x - pos.x) + (y - pos.y) * (y - pos.y) + (z - pos.z) * (z - pos.z));
    }
    return 0.0f;
}

/* The nearest player in play (-1: none). */
s32 a3kpNearestPl(Obj *o) {
    s32 pad[3];
    f32 x;
    f32 y;
    f32 z;
    s32 i;
    f32 d;
    s32 best;
    Vec3f pos;
    f32 min;

    min = 65536.0f;
    best = -1;
    eneWorldPos(o, &pos);
    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plGetModelPos(gGameWork.players[i], &x, &y, &z);
            d = a3kpPlDist(o, i);
            if (d < min) {
                min = d;
                best = i;
            }
        }
    }
    return best;
}

/* The file's own eneFallLimit: 1 when it is below -600 or above 1000. */
s32 a3kpFallLimit(Obj *o) {
    A3Kappa *w = A3KP(o);

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

/* "a3kp_g": puts out an area3kappa (kind 0x238) every 91 steps, with its own placement value. */
s32 a3kpGenProc(Obj *o, s16 msg, void *arg) {
    A3Kappa *w = A3KP(o);
    s32 pad;
    s32 v;

    switch (msg) {
        case 2:
            o->name = "a3kp_g";
            v = eneSetup(o, arg);
            w->type = v & 0xFF;
            w->value = v >> 16;
            w->time = 90;
            break;
        case 0:
            if (a3kpFallLimit(o)) {
                return -1;
            }
            w->time++;
            if (w->time > 90) {
                /* its placement value again: the type low, the high half high */
                eneMakeEx(NULL, ENE_A3KP, w->zone, w->x, w->y, w->z, 2,
                          (u32 (*)(void *, void *, void *)) ((w->type & 0xFFFF) |
                                                              (((s16) w->value << 16) & 0xFFFF0000)));
                w->time = 0;
            }
            break;
        case 1:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "area3kappa": its type's step (a3kpLeapStep on) runs each frame; a nonzero result ends it. */
s32 a3kpProc(Obj *o, s16 msg, void *arg) {
    A3Kappa *w = A3KP(o);
    s32 pad;
    s32 v;
    s32 r;

    switch (msg) {
        case 2:
            o->name = "area3kappa";
            v = eneSetup(o, arg);
            w->type = v & 0xFF;
            w->value = v >> 16;
            w->life = 1;
            w->gravity = 0;
            w->u.floor = w->y - 10.0f;
            if ((w->model = eneNewAnime(o, 0)) == NULL) {
                return -1;
            }
            if (w->type != 0) {
                if ((w->shadow = shadowCreate()) == NULL) {
                    return -1;
                }
            }
            if ((w->atk = ATK_ADD_YZ(ene_atk_root_frame, &w->model->f, 0, 0, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->dmg = DMG_ADD_YZ(ene_dmg_root_frame, &w->model->f, 0, 0, 0, 0, 0)) == NULL) {
                return -1;
            }
            break;
        case 0:
            if (a3kpFallLimit(o)) {
                return -1;
            }
            if (acollAtkWasHit(w->atk) == 1) {
                if ((Obj *) w->atk->hitPower != NULL) {
                    tskSendMsg((Obj *) w->atk->hitPower, 7, o);
                }
            }
            switch (w->type) {
                case 0:
                    r = a3kpLeapStep(o);
                    break;
                case 1:
                    r = a3kpWalkStep(o);
                    break;
                case 2:
                    r = a3kpSpinStep(o);
                    break;
                case 3:
                    r = a3kpTurnStep(o);
                    break;
            }
            /* the game's: r is never set for a type above 3 (it reads its stack word) */
            if (r != 0) {
                return r;
            }
            break;
        case 1:
            /* the first test and its call on one line (IDO schedules by line; tlb_0144's form) */
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
            if (w->type == 2 && w->u.mani != NULL) {
                frmDestroy(&w->u.mani->f);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* How far player i is above the height y (65536 if i is not 0-3 or not in play). */
f32 a3kpPlAbove(f32 y, s32 i) {
    f32 px;
    f32 py;
    f32 pz;

    if (i < 0 || i >= 4) {
        return 65536.0f;
    }
    if (plIsInPlay(i)) {
        plGetModelPos(gGameWork.players[i], &px, &py, &pz);
        return py - y;
    }
    return 65536.0f;
}

/* Type 0: hidden till a player comes within 100, then it leaps up (by its placement value) and
 * bites when level with the player; gone 240 frames after no one came, or below its floor. */
s32 a3kpLeapStep(Obj *o) {
    A3Kappa *w = A3KP(o);
    s32 pad;
    s32 done;

    eneFall(o, w->gravity * 0.01f * gRetracesPerFrame);
    eneMove((EneObj *) w);
    eneSetModelOnCourse(o, &w->model->f);
    if (a3kpLeapAnims[w->anim].loop == 1) {
        done = eneAnimLoop(o, w->model, w->animSpeed);
    } else {
        done = eneAnimPlayOnce(o, w->model, w->animSpeed);
    }
    if (acollDmgWasHit(w->dmg) == 1) {
        if (--w->life == 0) {
            w->dmg->flags &= ~2;
            w->atk->flags &= ~2;
            w->vel[1] = 0.0f;
            a3kpSetAnim(o, 5, a3kpLeapAnims);
        }
    }
    if (w->y <= w->u.floor) {
        eneReleaseSpawn(w->spawn);
        return -1;
    }
    switch (w->anim) {
        case 0:
            w->atk->radius = 50;
            w->atk->ofs[1] = 40;
            w->atk->kind = 120;
            w->dmg->radius = 65;
            w->dmg->height = 130;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 0.1f;
            if (eneNearestPlWay(o) == 1) {
                w->way = 0;
            } else {
                w->way = 1;
            }
            w->rot[1] = a3kpCourseHeading(o, 0.4363323152f, w->way);
            a3kpSetAnim(o, 1, a3kpLeapAnims);
            return 0;
        case 1:
            w->target = a3kpNearestPl(o);
            if (w->target >= 0) {
                if (a3kpPlDist(o, w->target) < 100.0f) {
                    w->gravity = 20;
                    w->vel[1] = w->value;
                    SoundLocation2(0x219, gGameWork.view, w->model->pos[0], w->model->pos[1], w->model->pos[2],
                                   0, 0, 1.570796371f);
                    a3kpSetAnim(o, 2, a3kpLeapAnims);
                    return 0;
                }
                w->time++;
                if (w->time > 240 / gRetracesPerFrame) {
                    eneReleaseSpawn(w->spawn);
                    return -1;
                }
            }
            break;
        case 2:
            if (fabs(a3kpPlAbove(w->y, w->target)) < 20.0) {
                a3kpSetAnim(o, 3, a3kpLeapAnims);
            }
            break;
        case 3:
            if (polAnimeLastFrame(w->model) * 0.5f <= w->model->frame) {
                w->atk->radius = 100;
                w->atk->ofs[2] = 50;
            }
            if (11.0f <= w->model->frame && w->model->frame < 11.0f + w->animSpeed) {
                eneSetSd(o, 0x226);
            }
            if (done) {
                w->atk->radius = 50;
                w->atk->ofs[2] = 0;
                a3kpSetAnim(o, 4, a3kpLeapAnims);
            }
            break;
        case 5:
            PRINTF("*** area3kappa(type %d):dead ***\n", w->type);
            shimEneDeathAct(o, w->dmg, (u32) a3kpHalf0Dl, (u32) a3kpHalf1Dl);
            eneReleaseSpawn(w->spawn);
            return -1;
    }
    return 0;
}

/* Type 1: leaps at a player within 100 and, once down, walks after the nearest one, biting when
 * within 20 (for 120 frames at a time). */
s32 a3kpWalkStep(Obj *o) {
    A3Kappa *w = A3KP(o);
    s32 pad1;
    s32 land;
    s32 pad2;
    s32 done;

    eneFall(o, w->gravity * 0.01f * gRetracesPerFrame);
    eneMove((EneObj *) w);
    land = eneLandOff((EneObj *) w, 10.0f, 0.0f);
    eneWallCheckNoPush(o, 10.0f, 7.0f, 0.0f, -10.0f);
    eneSetModelOnCourse(o, &w->model->f);
    if (a3kpWalkAnims[w->anim].loop == 1) {
        done = eneAnimLoop(o, w->model, w->animSpeed);
    } else {
        done = eneAnimPlayOnce(o, w->model, w->animSpeed);
    }
    if (w->shadow != NULL) {
        shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.3f, NULL);
    }
    if (acollDmgWasHit(w->dmg) == 1) {
        if (--w->life == 0) {
            w->dmg->flags &= ~2;
            w->atk->flags &= ~2;
            w->vel[1] = 0.0f;
            a3kpSetAnim(o, 5, a3kpWalkAnims);
        }
    }
    switch (w->anim) {
        case 0:
            w->atk->radius = 50;
            w->atk->ofs[1] = 40;
            w->atk->kind = 120;
            w->dmg->radius = 65;
            w->dmg->height = 130;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 0.1f;
            if (eneNearestPlWay(o) == 1) {
                w->way = 0;
            } else {
                w->way = 1;
            }
            w->rot[1] = a3kpCourseHeading(o, 0.4363323152f, w->way);
            w->gravity = 20;
            w->time = 0;
            a3kpSetAnim(o, 1, a3kpWalkAnims);
            break;
        case 1:
            w->target = a3kpNearestPl(o);
            if (w->target >= 0) {
                if (a3kpPlDist(o, w->target) < 100.0f) {
                    a3kpSetAnim(o, 2, a3kpWalkAnims);
                }
            }
            break;
        case 2:
            if (land) {
                w->vel[1] = 0;
            } else if (!land && w->u.landed) {
                if (eneNearestPlWay(o) == 1) {
                    w->vel[0] = gRetracesPerFrame * 0.9;
                }
                if (eneNearestPlWay(o) == 4) {
                    w->vel[0] = gRetracesPerFrame * -0.9;
                }
                w->vel[1] = w->value;
                SoundLocation2(0x219, gGameWork.view, w->model->pos[0], w->model->pos[1], w->model->pos[2], 0, 0,
                               1.570796371f);
                a3kpSetAnim(o, 3, a3kpWalkAnims);
                break;
            }
            if (eneNearestPlWay(o) == 1) {
                w->way = 0;
            } else {
                w->way = 1;
            }
            eneTurnToward(o, a3kpCourseHeading(o, 0.7853981853f, w->way), gRetracesPerFrame * 0.104719758f);
            if (w->way == 0) {
                w->vel[0] = gRetracesPerFrame * 0.4;
            } else if (w->way == 1) {
                w->vel[0] = gRetracesPerFrame * -0.4;
            }
            if (a3kpPlDist(o, w->target) < 20.0f) {
                w->time++;
                if (w->time < 120 / gRetracesPerFrame) {
                    w->atk->radius = 75;
                    w->atk->ofs[2] = 35;
                    a3kpSetAnim(o, 4, a3kpWalkAnims);
                }
            } else {
                w->time = 0;
            }
            break;
        case 3:
            if (land) {
                a3kpSetAnim(o, 2, a3kpWalkAnims);
            }
            break;
        case 4:
            if (11.0f <= w->model->frame && w->model->frame < 11.0f + w->animSpeed) {
                eneSetSd(o, 0x226);
            }
            if (eneNearestPlWay(o) == 1) {
                w->way = 0;
            } else {
                w->way = 1;
            }
            eneTurnToward(o, a3kpCourseHeading(o, 0.7853981853f, w->way), gRetracesPerFrame * 0.104719758f);
            if (done) {
                w->time = 0;
                w->atk->radius = 50;
                w->atk->ofs[2] = 0;
                a3kpSetAnim(o, 2, a3kpWalkAnims);
            }
            break;
        case 5:
            PRINTF("*** area3kappa(type %d):dead ***\n", w->type);
            shimEneDeathAct(o, w->dmg, (u32) a3kpHalf0Dl, (u32) a3kpHalf1Dl);
            eneReleaseSpawn(w->spawn);
            return -1;
    }
    w->u.landed = land;
    return 0;
}

/* Type 2: drawn as a "mani" model aligned to the ground under it; once a player is within 100 it
 * leaps and spins. */
s32 a3kpSpinStep(Obj *o) {
    A3Kappa *w = A3KP(o);
    s32 pad;
    s32 land;
    s32 done;

    eneFall(o, w->gravity * 0.01f * gRetracesPerFrame);
    eneMove((EneObj *) w);
    land = eneLandOff((EneObj *) w, 20.0f, 0.0f);
    eneSetModelOnCourse(o, &w->model->f);
    if (a3kpSpinAnims[w->anim].loop == 1) {
        done = eneAnimLoop(o, w->model, w->animSpeed);
    } else {
        done = eneAnimPlayOnce(o, w->model, w->animSpeed);
    }
    if (w->shadow != NULL) {
        shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.3f, NULL);
    }
    if (acollDmgWasHit(w->dmg) == 1) {
        if (--w->life == 0) {
            w->dmg->flags &= ~2;
            w->atk->flags &= ~2;
            w->vel[1] = 0.0f;
            a3kpSetAnim(o, 3, a3kpSpinAnims);
        }
    }
    if (w->u.mani != NULL) {
        PRINTF("*** call align\n");
        a3kpZAlign(w->u.mani, w->x, w->y, w->z, w->model->scale[0]);
        PRINTF("drive\n");
    }
    switch (w->anim) {
        case 0:
            if ((w->u.mani = (MAniFrame *) frmCreate(gFrmModelLayer, gFrmKindMani)) == NULL) {
                return -1;
            }
            w->model->flags &= ~2;
            w->u.mani->dl = w->model->dl;
            w->u.mani->anim = w->model->anim;
            w->u.mani->segs[0] = w->model->segs[0];
            w->atk->radius = 50;
            w->atk->ofs[1] = 40;
            w->atk->kind = 120;
            w->dmg->radius = 65;
            w->dmg->height = 130;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 0.15f;
            if (eneNearestPlWay(o) == 1) {
                w->way = 0;
            } else {
                w->way = 1;
            }
            w->rot[1] = a3kpCourseHeading(o, 0.4363323152f, w->way);
            w->gravity = 20;
            a3kpSetAnim(o, 1, a3kpSpinAnims);
            break;
        case 1:
            w->target = a3kpNearestPl(o);
            if (w->target >= 0) {
                if (a3kpPlDist(o, w->target) < 100.0f) {
                    a3kpSetAnim(o, 2, a3kpSpinAnims);
                }
            }
            break;
        case 2:
            if (land) {
                w->vel[1] = 0.0f;
            }
            w->rot[1] += 0.13962633907794952;
            break;
        case 3:
            PRINTF("*** area3kappa(type %d):dead ***\n", w->type);
            shimEneDeathAct(o, w->dmg, (u32) a3kpHalf0Dl, (u32) a3kpHalf1Dl);
            eneReleaseSpawn(w->spawn);
            return -1;
    }
    w->u.landed = land;
    w->model->flags &= ~2;
    return 0;
}

/* Type 3: rises and turns in place; each hit knocks it up with a spark, and it splashes down and
 * swings back a full turn while it cannot be hit (25 frames). */
s32 a3kpTurnStep(Obj *o) {
    A3Kappa *w = A3KP(o);
    s32 pad;
    s32 land;
    s32 pad2[2];

    eneFall(o, w->gravity * 0.01f * gRetracesPerFrame);
    eneMove((EneObj *) w);
    land = eneLandOff((EneObj *) w, 20.0f, 0.0f);
    eneSetModelOnCourse(o, &w->model->f);
    if (a3kpTurnAnims[w->anim].loop == 1) {
        eneAnimLoop(o, w->model, w->animSpeed);
    } else {
        eneAnimPlayOnce(o, w->model, w->animSpeed);
    }
    if (w->shadow != NULL) {
        shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.3f, NULL);
    }
    if (acollDmgWasHit(w->dmg) == 1) {
        eneHitSoundHeavy(w->dmg);
        eneSetSd(o, 0x23A);
        w->dmg->flags &= ~2;
        w->time = 0;
        w->vel[1] = 5.0f;
        land = 0;
        if (6.283185482f <= w->u.sway) {
            w->u.sway = 0.2617993951f;
        }
        eneMakeHitSpark(w->zone, w->x, w->y + 10.0f, w->z, 2, w->rot[1]);
        a3kpSetAnim(o, 3, a3kpTurnAnims);
    }
    switch (w->anim) {
        case 0:
            w->atk->radius = 60;
            w->atk->ofs[1] = 60;
            w->atk->kind = 120;
            w->dmg->radius = 65;
            w->dmg->height = 130;
            w->dmg->ofs[1] = -40;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 0.25f;
            if (eneNearestPlWay(o) == 1) {
                w->way = 0;
            } else {
                w->way = 1;
            }
            w->rot[1] = a3kpCourseHeading(o, 0.3490658402f, w->way);
            w->gravity = 15;
            w->u.sway = 0.0f;
            a3kpSetAnim(o, 1, a3kpTurnAnims);
            break;
        case 1:
            a3kpSetAnim(o, 2, a3kpTurnAnims);
            break;
        case 2:
            break;
        case 3:
            if (land) {
                w->vel[1] = 0.0f;
                a3kpSplash(o);
                eneSetSd(o, 0x2EE);
                a3kpSetAnim(o, 2, a3kpTurnAnims);
            }
            if (w->time + 1 > 25 / gRetracesPerFrame) {
                w->dmg->flags |= 2;
            } else {
                w->time++;
            }
            if (w->u.sway < 6.283185482f) {
                if (6.283185482f <= w->u.sway + 0.2617993951f) {
                    w->rot[1] += 0.2617993951f - (w->u.sway - 6.283185482f);
                    w->u.sway = 6.283185482f;
                } else {
                    w->u.sway += 0.2617993951f;
                    w->rot[1] += 0.2617993951f;
                }
            }
            break;
        case 4:
            PRINTF("*** area3kappa(type %d):dead ***\n", w->type);
            shimEneDeathAct(o, w->dmg, (u32) a3kpHalf0Dl, (u32) a3kpHalf1Dl);
            eneReleaseSpawn(w->spawn);
            return -1;
    }
    return 0;
}

/* Two grey puffs on its course, 10 to each side of it: its splash. */
void a3kpSplash(Obj *o) {
    A3Kappa *w = A3KP(o);
    s32 pad;
    f32 x;
    f32 y;
    f32 z;

    vramCoursePointXZ(A3KP(o)->zone, A3KP(o)->x + 10.0f, A3KP(o)->z, &x, &z);
    itexPuffGrey(x, w->y, z, 0.07f);
    vramCoursePointXZ(w->zone, w->x - 10.0f, w->z, &x, &z);
    itexPuffGrey(x, w->y, z, 0.07f);
}

/* "a3kpfire": a yellow flame (file 0x125's timed animation) that grows and fades out. */
s32 a3kpFireProc(Obj *o, s16 msg, void *arg) {
    A3KpFire *w = A3KPFIRE(o);

    switch (msg) {
        case 2:
            o->name = "a3kpfire";
            if ((w->pol = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                return -1;
            }
            w->pol->dl2 = (Gfx *) a3kpFireDl;
            floadGetSetRam((void **) w->pol->segs, 0x125);
            fadeAnimStart(&w->anim, gItexTimedYellowAnim, 0, &w->pol->segs[3], floadGetSetFile(0x125, 0));
            w->pol->flags |= 7;
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2] = 0.1f;
            w->alpha = 255;
            break;
        case 1:
            frmDestroy(&w->pol->f);
            break;
        case 0:
            fadeAnimStep(&w->anim);
            w->pol->scale[0] += 0.01f;
            w->pol->scale[1] = w->pol->scale[2] = w->pol->scale[0];
            w->alpha -= 8;
            if (w->alpha < 0) {
                return -1;
            }
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombine(gGfxDlHead++, 0x30D261, 0x55A6FF7F);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 0xFF, 0xFF, 0x80, 0x00);
            gDPSetEnvColor(gGfxDlHead++, 0xFF, 0x00, 0x00, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void a3kpEmpty3(void) {
}

/* A matrix turned to stand on a ground of normal (x, y, z). */
void a3kpMtxStand(f32 m[4][4], f32 x, f32 y, f32 z) {
    mathMtxLookDir(m, x, -z, y);
    mathMtxPostRotX(m, 1.570796371f);
}

/* Aligns the mani m to the ground below (x, y, z): stood on it, at its height, at half size
 * (scale is not used). */
void a3kpZAlign(MAniFrame *m, f32 x, f32 y, f32 z, f32 scale) {
    Vec3f from;
    Vec3f dir;
    Vec3f at;
    f32 t;
    CollTri *tri;

    from.x = x; from.y = y; from.z = z;
    dir.x = 0.0f;
    dir.y = -10.0f;
    dir.z = 0.0f;
    tri = vramRayGround(&from, &dir, &at, &t);
    if (tri == NULL) {
        PRINTF("*** z_align NULL\n");
    } else {
        PRINTF("*** z_align OK\n");
        a3kpMtxStand(m->mtx.m, -tri->n.x, -tri->n.y, -tri->n.z);
        mathMtxPostTrans(m->mtx.m, at.x, at.y, at.z);
        mathMtxScaleRot(m->mtx.m, 0.5f);
    }
}

/* "area3beard": the kappa's work and its handler's shape; its step is a3kpBeardStep. */
s32 a3kpBeardProc(Obj *o, s16 msg, void *arg) {
    A3Kappa *w = A3KP(o);
    s32 v;
    s32 r;

    switch (msg) {
        case 2:
            o->name = "area3beard";
            v = eneSetup(o, arg);
            w->type = v & 0xFF;
            w->value = v >> 16;
            w->life = 255;
            w->gravity = 0;
            w->u.floor = w->y - 10.0f;
            if ((w->model = eneNewAnime(o, 0)) == NULL) {
                return -1;
            }
            if ((w->shadow = shadowCreate()) == NULL) {
                return -1;
            }
            if ((w->atk = ATK_ADD_YZ(ene_atk_root_frame, &w->model->f, 0, 0, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->dmg = DMG_ADD_YZ(ene_dmg_root_frame, &w->model->f, 0, 0, 0, 0, 0)) == NULL) {
                return -1;
            }
            break;
        case 0:
            if (a3kpFallLimit(o)) {
                return -1;
            }
            if (acollAtkWasHit(w->atk) == 1) {
                if ((Obj *) w->atk->hitPower != NULL) {
                    tskSendMsg((Obj *) w->atk->hitPower, 7, o);
                }
            }
            if ((r = a3kpBeardStep(o)) != 0) {
                return r;
            }
            break;
        case 1:
            /* the first test and its call on one line (IDO schedules by line; tlb_0144's form) */
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
            if (w->type == 2 && w->u.mani != NULL) {
                frmDestroy(&w->u.mani->f);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The area3beard's step: as type 3's (a3kpTurnStep), bigger, and it is gone when beaten. */
s32 a3kpBeardStep(Obj *o) {
    A3Kappa *w = A3KP(o);
    s32 pad;
    s32 land;

    eneFall(o, w->gravity * 0.01f * gRetracesPerFrame);
    eneMove((EneObj *) w);
    land = eneLandOff((EneObj *) w, 20.0f, 0.0f);
    eneSetModelOnCourse(o, &w->model->f);
    if (a3kpBeardAnims[w->anim].loop == 1) {
        eneAnimLoop(o, w->model, w->animSpeed);
    } else {
        eneAnimPlayOnce(o, w->model, w->animSpeed);
    }
    if (w->shadow != NULL) {
        shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.3f, NULL);
    }
    if (acollDmgWasHit(w->dmg) == 1) {
        eneHitSoundHeavy(w->dmg);
        eneSetSd(o, 0x14F);
        w->dmg->flags &= ~2;
        w->time = 0;
        w->vel[1] = 5.0f;
        land = 0;
        if (6.283185482f <= w->u.sway) {
            w->u.sway = 0.2617993951f;
        }
        eneMakeHitSpark(w->zone, w->x, w->y + 10.0f, w->z, 2, w->rot[1]);
        a3kpSetAnim(o, 3, a3kpBeardAnims);
    }
    switch (w->anim) {
        case 0:
            w->atk->radius = 180;
            w->atk->ofs[1] = 180;
            w->atk->kind = 120;
            w->dmg->radius = 220;
            w->dmg->height = 500;
            w->dmg->ofs[1] = -100;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 0.075f;
            if (eneNearestPlWay(o) == 1) {
                w->way = 0;
            } else {
                w->way = 1;
            }
            w->rot[1] = a3kpCourseHeading(o, 0.3490658402f, w->way);
            w->gravity = 15;
            w->u.sway = 0.0f;
            a3kpSetAnim(o, 1, a3kpBeardAnims);
            break;
        case 1:
            a3kpSetAnim(o, 2, a3kpBeardAnims);
            break;
        case 2:
            if (eneNearestPlWay(o) == 1) {
                w->way = 0;
            } else {
                w->way = 1;
            }
            eneTurnToward(o, a3kpCourseHeading(o, 0.4363323152f, w->way), gRetracesPerFrame * 0.104719758f);
            break;
        case 3:
            if (land) {
                w->vel[1] = 0.0f;
                a3kpSplash(o);
                SoundLocation2(0x10A, gGameWork.view, w->model->pos[0], w->model->pos[1], w->model->pos[2], 0.0f,
                               600.0f, 1.570796371f);
                a3kpSetAnim(o, 2, a3kpBeardAnims);
            }
            if (w->time + 1 > 25 / gRetracesPerFrame) {
                w->dmg->flags |= 2;
            } else {
                w->time++;
            }
            if (w->u.sway < 6.283185482f) {
                if (6.283185482f <= w->u.sway + 0.2617993951f) {
                    w->rot[1] += 0.2617993951f - (w->u.sway - 6.283185482f);
                    w->u.sway = 6.283185482f;
                } else {
                    w->u.sway += 0.2617993951f;
                    w->rot[1] += 0.2617993951f;
                }
            }
            break;
        case 4:
            PRINTF("*** area3backbeard:dead ***\n");
            eneReleaseSpawn(w->spawn);
            return -1;
    }
    return 0;
}
