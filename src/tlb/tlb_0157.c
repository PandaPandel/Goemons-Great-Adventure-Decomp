#include "tlb.h"

/* File 157: "eyefrog_main" (a frog that sits until a player comes near, then hops at it; its
 * placement's type picks how it starts: on the ground, small and growing, in the air, or bobbing). */

#define sinf (*sinf)

/* acoll.c's definitions are void but return the box they add: called through these casts. */
#define acollAtkAddYZ_r ((ACollFrame * (*) (Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define acollDmgAddYZ_r ((ACollFrame * (*) (Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 eyefrogSitAnim[]; /* its sit */
extern u8 eyefrogHopAnim[]; /* its hop */
extern u8 eyefrogHop2Anim[];
extern u8 eyefrogHitAnim[]; /* its hit */
extern u8 eyefrogHit2Anim[];
extern u8 eyefrogHit3Anim[];
extern u8 eyefrogHalf0Dl[];
extern u8 eyefrogHalf1Dl[];

/* The animation steps are float locals ("f32 speed = 0.5f;"): IDO propagates the constant into the
 * calls but converts it to double at run time in "frame <= 7.0 + speed" (a literal 0.5f folds).
 * The same holds for the turn step (neg.s at run time) and the hop's f64 x speed (cvt.s.d). */

/* The eyefrog's work, at its task + 0x20: an enemy's (EneObj) up to 0x4C. */
typedef struct EyeFrog {
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
    /* 0x40 */ u32 halves[2];
    /* 0x48 */ u32 behave;
    /* 0x4C */ s16 baseY; /* type 4: the height it bobs about */
    /* 0x4E */ u8 type; /* its placement's type */
    /* 0x4F */ u8 way; /* eneNearestPlWay's */
    /* 0x50 */ u8 flash; /* steps of the flash after a hit */
} EyeFrog;

#define EYEFROG(obj) ((EyeFrog *) FX(obj))

void eyefrogWait(Obj *o);
void eyefrogHopStart(Obj *o);
void eyefrogHop(Obj *o);
void eyefrogSit(Obj *o);
void eyefrogCroak(Obj *o);
void eyefrogHideWait(Obj *o);
void eyefrogHopReady(Obj *o);
void eyefrogHopMove(Obj *o);
void eyefrogNone(Obj *o);
void eyefrogLurk(Obj *o);
void eyefrogLeap(Obj *o);
void eyefrogLanded(Obj *o);
void eyefrogFloat(Obj *o);
void eyefrogHit(Obj *o);
void eyefrogRecover(Obj *o);

s32 eyefrogMain(Obj *o, s16 msg, void *arg) {
    EyeFrog *w = EYEFROG(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "eyefrog_main";
            w->type = eneSetup(o, arg);
            if ((w->model = eneAnimeModelMake(o, (u32) eyefrogSitAnim, NULL, 0)) == NULL) {
                return -1;
            }
            if ((w->atk = acollAtkAddYZ_r((Frame *) ene_atk_root_frame, (Frame *) w->model, 120, 200, 20, 0)) ==
                NULL) {
                return -1;
            }
            w->atk->kind = 120;
            if ((w->dmg = acollDmgAddYZ_r((Frame *) ene_dmg_root_frame, (Frame *) w->model, 140, 405, 0, -60, 0)) ==
                NULL) {
                return -1;
            }
            if ((w->shadow = shadowCreate()) == NULL) {
                return -1;
            }
            w->halves[0] = (u32) eyefrogHalf0Dl;
            w->halves[1] = (u32) eyefrogHalf1Dl;
            if (w->type == 1) {
                if (eneSetOnEarth(o) == -1) {
                    return -1;
                }
                w->behave = 0x400002;
                w->hp = 2;
                w->rot[1] = eneCourseAngle(o, 8);
                w->state = 0;
            } else if (w->type == 2) {
                if (eneSetOnEarth(o) == -1) {
                    return -1;
                }
                /* As a pointer (the anim field is a void *): written (u32), IDO keeps the model's
                 * address from eneAnimeModelMake in a spilled temp; the game builds it again here. */
                w->model->anim = eyefrogSitAnim;
                w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 0.05f;
                w->behave = 0x404000;
                w->rot[1] = eneCourseAngle(o, 8);
                w->state = 5;
            } else if (w->type == 3) {
                w->model->anim = eyefrogHopAnim;
                w->model->frame = 8.0f;
                w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 0.05f;
                w->behave = 0x400000;
                w->y -= 80.0f;
                w->rot[1] = eneCourseAngle(o, 8);
                w->state = 9;
            } else if (w->type == 4) {
                w->model->anim = eyefrogHit3Anim;
                w->model->frame = 0.0f;
                w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 0.075f;
                w->behave = 0x400000;
                w->rot[0] = -0.7853981853f;
                w->rot[1] = eneCourseAngle(o, 8);
                w->baseY = w->y;
                w->state = 12;
            }
            w->flash = 0;
            w->way = eneNearestPlWay(o);
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->state) {
                case 0:
                    (*eyefrogWait)(o);
                    break;
                case 1:
                    (*eyefrogHopStart)(o);
                    break;
                case 2:
                    (*eyefrogHop)(o);
                    break;
                case 3:
                    (*eyefrogSit)(o);
                    break;
                case 4:
                    (*eyefrogCroak)(o);
                    break;
                case 5:
                    (*eyefrogHideWait)(o);
                    break;
                case 6:
                    (*eyefrogHopReady)(o);
                    break;
                case 7:
                    (*eyefrogHopMove)(o);
                    break;
                case 8:
                    (*eyefrogNone)(o);
                    break;
                case 9:
                    (*eyefrogLurk)(o);
                    break;
                case 10:
                    (*eyefrogLeap)(o);
                    break;
                case 11:
                    (*eyefrogLanded)(o);
                    break;
                case 12:
                    (*eyefrogFloat)(o);
                    break;
                case 13:
                    (*eyefrogHit)(o);
                    break;
                case 14:
                    (*eyefrogRecover)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_eyefrog_main: 死亡\n");
                return -1;
            }
            if (w->hit & 0x10) {
                w->flash = 16;
                w->model->anim = eyefrogHitAnim;
                w->model->frame = 0.0f;
                w->vel[1] = 0.0f;
                if (w->way == 4) {
                    w->rot[1] = eneCourseAngle(o, 4) + 0.3926990926f;
                    PRINTF("front %f, ry %f", w->rot[1], w->rot[1]);
                    w->rot[1] = eneCourseAngle(o, 4);
                } else {
                    w->rot[1] = eneCourseAngle(o, 1) - 0.3926990926f;
                    w->rot[1] = eneCourseAngle(o, 1);
                }
                w->state = 13;
            }
            if (w->flash - 1 > 0) {
                w->flash = w->flash - 1;
                w->dmg->flags &= ~2;
            } else {
                w->flash = 0;
                w->dmg->flags |= 2;
            }
        case 3:
            if (w->flash != 0) {
                w->model->dl = eneDlEnvColorLit(o, (w->flash & 2) ? 0x80 : 0, 0xFF, 0xFF, 0xFF,
                                                (w->flash & 2) ? 0 : 1);
            } else {
                w->model->dl = eneDlEnvColorLit(o, 0, 0xFF, 0xFF, 0xFF, 1);
            }
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy((Frame *) w->model);
            }
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            if (w->atk != NULL) {
                frmDestroy((Frame *) w->atk);
            }
            if (w->dmg != NULL) {
                frmDestroy((Frame *) w->dmg);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Sits until a player is within 70: then hops. */
void eyefrogWait(Obj *o) {
    EyeFrog *w = EYEFROG(o);

    if (enePlInRangeCheck(o, 70.0f, 0)) {
        w->model->anim = eyefrogHopAnim;
        w->model->frame = 0.0f;
        w->state = 1;
    }
}

void eyefrogHopStart(Obj *o) {
    EYEFROG(o)->state = 2;
}

/* A hop: the sound at frame 7 (and up, if on the ground), turning to the player through 7-11. */
void eyefrogHop(Obj *o) {
    EyeFrog *w = EYEFROG(o);
    s32 pad[2];
    f32 speed = 0.5f;
    f32 turn = 0.294524312f;
    f32 ang;

    if (eneAnimLoop(o, w->model, speed)) {
        w->model->anim = eyefrogSitAnim;
        w->model->frame = 0.0f;
        w->state = 3;
    }
    if (w->model->frame > 7.0 && w->model->frame <= 7.0 + speed) {
        eneSetSd(o, 0x25B);
        if (w->hit & 1) {
            w->vel[1] = 4.0f;
        }
    }
    if (w->model->frame > 7.0 && w->model->frame <= 11.0) {
        ang = eneCourseAngle(o, w->way);
        if (w->way == 4) {
            eneTurnRotYTo(o, ang + 0.3926990926f, -turn);
        } else {
            eneTurnRotYTo(o, ang - 0.3926990926f, turn);
        }
    }
    if (w->model->frame > 11.0 && !(w->hit & 1)) {
        w->model->frame = 11.0f;
    }
    eneFall(o, 8.0 / (5.0 / speed));
}

void eyefrogSit(Obj *o) {
    EyeFrog *w = EYEFROG(o);

    if (eneAnimPlayOnce(o, w->model, 0.5f)) {
        w->model->anim = eyefrogHop2Anim;
        w->model->frame = 0.0f;
        w->state = 4;
    }
}

void eyefrogCroak(Obj *o) {
    EyeFrog *w = EYEFROG(o);
    f32 speed = 0.5f;

    if (eneAnimPlayOnce(o, w->model, speed)) {
        w->model->anim = eyefrogSitAnim;
        w->model->frame = 0.0f;
        w->state = 3;
    }
    if (w->model->frame >= 3.5 && w->model->frame < 3.5 + speed) {
        eneSetSd(o, 0x25C);
    }
}

void eyefrogHideWait(Obj *o) {
    EyeFrog *w = EYEFROG(o);

    eneAnimLoop(o, w->model, 0.5f);
    if (eneCamNearCheck(o, 80.0f)) {
        w->way = eneNearestPlWay(o);
        w->state = 6;
    }
}

void eyefrogHopReady(Obj *o) {
    EyeFrog *w = EYEFROG(o);

    if (eneAnimLoop(o, w->model, 0.5f)) {
        w->model->anim = eyefrogHopAnim;
        w->model->frame = 0.0f;
        w->state = 7;
    }
}

void eyefrogHopMove(Obj *o) {
    EyeFrog *w = EYEFROG(o);
    f32 speed = 0.5f;
    f64 vx;

    eneAnimLoop(o, w->model, speed);
    if (w->model->frame > 7.0 && w->model->frame <= 7.0 + speed) {
        eneSetSd(o, 0x25B);
        if (w->hit & 1) {
            w->vel[1] = 4.0f;
        }
    }
    if (w->model->frame > 7.0 && w->model->frame <= 11.0) {
        if (w->way == 4) {
            vx = -1.0;
            w->vel[0] = vx;
        } else {
            vx = 1.0;
            w->vel[0] = vx;
        }
    } else {
        w->vel[0] = 0.0f;
    }
    if (w->model->frame > 11.0 && !(w->hit & 1)) {
        w->model->frame = 11.0f;
    }
    eneFall(o, 8.0 / (5.0 / speed));
}

void eyefrogNone(Obj *o) {
}

void eyefrogLurk(Obj *o) {
    EyeFrog *w = EYEFROG(o);
    s32 pad;

    if (eneNearestPlInX(o, 85.0f)) {
        SoundLocation2(0x219, gGameWork.view, w->model->pos[0], w->model->pos[1], w->model->pos[2], 0.0f, 0.0f,
                       1.570796371f);
        w->behave |= 0x800;
        w->state = 10;
        w->vel[1] = 7.5f;
        w->vel[0] = -1.5f;
    }
}

void eyefrogLeap(Obj *o) {
    EyeFrog *w = EYEFROG(o);
    f32 f;
    f32 ang;
    s32 pad;

    w->rot[1] += 0.5;
    if (w->rot[1] > 3.141592741f) {
        w->rot[1] -= 6.283185482f;
    }
    if (w->hit & 1) {
        w->vel[0] = 0.0f;
        f = w->model->frame + 0.349999994f;
        if (f < 10.0) {
            w->model->frame = f;
        } else {
            w->model->frame = 10.0f;
            ang = eneCourseAngle(o, 8);
            if (ang <= w->rot[1] && w->rot[1] < ang + 0.5) {
                w->rot[1] = ang;
                w->behave |= 0x4000;
                w->state = 11;
            }
        }
    }
}

void eyefrogLanded(Obj *o) {
    EyeFrog *w = EYEFROG(o);

    if (eneAnimPlayOnce(o, w->model, 0.5f)) {
        w->way = eneNearestPlWay(o);
        w->state = 7;
    }
}

void eyefrogFloat(Obj *o) {
    EyeFrog *w = EYEFROG(o);
    f32 speed = 0.5f;
    s32 pad;

    eneAnimLoop(o, w->model, speed);
    w->y = sinf((gGameFrameCount + w->x) / 20.0f) * 20.0f + w->baseY;
}

void eyefrogHit(Obj *o) {
    EyeFrog *w = EYEFROG(o);
    f32 speed = 0.8f;

    if (eneAnimPlayOnce(o, w->model, speed)) {
        w->model->anim = eyefrogHit2Anim;
        w->model->frame = 0.0f;
        w->state = 14;
    }
    if (w->model->frame >= 4.0 && w->model->frame < 4.0 + speed) {
        eneSetSd2(o, 0x247, 0, 0);
    }
    eneFall(o, 0.5f);
}

void eyefrogRecover(Obj *o) {
    EyeFrog *w = EYEFROG(o);

    if (eneAnimPlayOnce(o, w->model, 0.6f)) {
        w->model->anim = eyefrogSitAnim;
        w->model->frame = 0.0f;
        w->state = 3;
    }
}
