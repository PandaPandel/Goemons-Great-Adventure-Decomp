#include "tlb.h"

/* File 96: "mouja" (a dead man's ghost walking the course: it backs off, paces, turns to the
 * nearest player and throws an axe), "ono" (the axe it throws, kind 0xEB) and "koz_m". */

#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

/* An animation step: "* 2.0 * 1.0f" (add.d, then a mul.d by 1.0), as in file 78. */
#define ANIM_STEP(x) ((x) * 2.0 * 1.0f)

#define ABS(x) ((x) > 0.0f ? (x) : -(x))

extern u8 moujaWalkModelAnim[]; /* the mouja's walk */
extern u8 moujaKozMWaitAnim[]; /* koz_m's wait */
extern u8 moujaThrowAnim[]; /* the mouja's model and throw */
extern u8 moujaHalf0Dl[];
extern u8 moujaHalf1Dl[];

/* The mouja's work, at its task + 0x20: an enemy's (EneObj) up to 0x3C. */
typedef struct Mouja {
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
    /* 0x3C */ s32 timer; /* steps in this step */
    /* 0x40 */ f32 animSpeed;
    /* 0x44 */ f32 homeX; /* where it was placed: it paces 20 either side */
    /* 0x48 */ f32 range; /* a player this near (in x) makes it throw */
    /* 0x4C */ u8 type; /* its placement's type: the step it starts in and goes back to */
    /* 0x4D */ u8 step;
    /* 0x4E */ u8 throwing; /* 1 turned to the player, 2 the axe is out */
    /* 0x4F */ u8 side; /* shimPlSideCheck's */
    /* 0x50 */ u8 axeType; /* its placement's high half: the axe's type */
    /* 0x51 */ u8 phase; /* koz_m: 0-3 */
    /* 0x52 */ u8 pad52[2];
    /* 0x54 */ MPolFrame *shadow;
} Mouja;

#define MOUJA(obj) ((Mouja *) FX(obj))
AnimFrame moujaEmptyAnim[] = {
    { 0, 0 },
    { 0, 0 },
};

void moujaStepStick(Obj *o);
void moujaStepWalk(Obj *o);
void moujaStepPace(Obj *o);
void moujaStepThrow(Obj *o);
void moujaWalkAnim(Obj *o, AnimeFrame *m);

/* The mouja's handler. */
s32 moujaProc(Obj *o, s16 msg, void *arg) {
    Mouja *w = MOUJA(o);
    u32 r;

    switch (msg) {
        case 2:
            o->name = "mouja";
            r = eneSetup(o, arg);
            /* The halves through u16 (the casts spend the game's four t-registers: measured). */
            w->type = (u16) (r & 0xFFFF);
            w->axeType = (u16) ((r >> 16) & 0xFFFF);
            w->model = eneNewAnime(o, (u32) moujaThrowAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->model->flags |= 0x20;
            w->atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, (Frame *) w->model, 60, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 120;
            w->dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, (Frame *) w->model, 60, 180, 0, 10, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                PRINTF("mouja: SET MISS\n");
                return -1;
            }
            w->shadow = shadowCreate();
            if (w->shadow == NULL) {
                return -1;
            }
            w->step = w->type;
            w->timer = 0;
            w->homeX = w->x;
            w->rot[1] = eneCourseAngle(o, 4);
            PRINTF("mouja step [%d]\n", w->step);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy((Frame *) w->model);
            }
            if (w->atk != NULL) {
                frmDestroy((Frame *) w->atk);
            }
            if (w->dmg != NULL) {
                frmDestroy((Frame *) w->dmg);
            }
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            PRINTF("mouja DESTROY\n");
            break;
        case 3:
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->step) {
                case 0:
                    moujaStepWalk(o);
                    break;
                case 1:
                    moujaStepPace(o);
                    break;
                case 2:
                    moujaStepThrow(o);
                    break;
                case 3:
                    moujaStepStick(o);
                    break;
                default:
                    PRINTF("mouja step error!\n");
                    break;
            }
            eneFall(o, 0.4f);
            shimEneMove(o, 10.0f, 2.0f, w->z, 4.0f);
            if (w->hit & 1) {
                w->vel[1] = 0.0f;
            }
            if (w->hit & 6) {
                w->vel[0] *= -1.0;
                w->vel[2] = 0.0f;
            }
            if (acollAtkWasHit(w->atk) == 1 && (Obj *) w->atk->hitPower != NULL) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, o);
            }
            if (acollDmgWasHit(w->dmg) == 1) {
                shimEneDeathAct(o, w->dmg, (u32) moujaHalf0Dl, (u32) moujaHalf1Dl);
                eneReleaseSpawn(w->spawn);
                return -1;
            }
            if (w->y < -600.0f) {
                PRINTF("mouja: 落ちすぎ\n");
                return -1;
            }
            if (w->hit & 1) {
                w->vel[1] = 0.0f;
            }
            PRINTF("mouja ON_WALL | ON_CLIFF\n");
            eneTurnAlongCourse(o, 10.0f);
            eneSetModelOnCourse(o, (Frame *) w->model);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Step 3: driven by controller 2's stick (a debug step). */
void moujaStepStick(Obj *o) {
    Mouja *w = MOUJA(o);
    s32 pad;

    if (w->timer == 0) {
        w->animSpeed = 0.5f;
    }
    w->timer++;
    w->vel[0] = joyGetStickX(1, 0) / 24;
    w->vel[1] = joyGetStickY(1, 0) / 24;
    eneAnimLoop(o, w->model, ANIM_STEP(w->animSpeed));
}

/* Step 0: walks away from the nearest player until one comes within 100. */
void moujaStepWalk(Obj *o) {
    Mouja *w = MOUJA(o);

    if (w->timer == 0) {
        w->model->anim = moujaWalkModelAnim;
        w->model->frame = 0.0f;
        w->range = 80.0f;
        w->side = shimPlSideCheck(o, shimNearestPl(o));
        if (w->side == 4) {
            w->vel[0] = -0.7f;
        } else {
            w->vel[0] = 0.7f;
        }
    }
    w->timer++;
    if (w->hit & 6) {
        w->vel[0] *= -1.0f;
        w->vel[2] = 0.0f;
    }
    if (eneNearestPlInX(o, w->range)) {
        w->step = 2;
        w->timer = 0;
    }
    moujaWalkAnim(o, w->model);
}

/* Step 1: paces 20 either side of where it was placed until a player comes within 40. */
void moujaStepPace(Obj *o) {
    Mouja *w = MOUJA(o);
    s32 pad;

    if (w->timer == 0) {
        w->model->anim = moujaWalkModelAnim;
        w->model->frame = 0.0f;
        w->range = 40.0f;
    }
    shimEneSwingX(o, w->homeX - 20.0f, w->homeX + 20.0f, 0.02f, 0.7f);
    w->timer++;
    if (w->hit & 6) {
        w->vel[0] *= -1.0f;
        w->vel[2] = 0.0f;
    }
    if (eneNearestPlInX(o, w->range)) {
        w->step = 2;
        w->timer = 0;
    }
    moujaWalkAnim(o, w->model);
}

/* Step 2: turns to the nearest player and throws an axe ("ono", kind 0xEB) at frame 7.5. */
void moujaStepThrow(Obj *o) {
    Mouja *w = MOUJA(o);
    s16 face;
    Vec3f pos;
    s32 pl;
    f32 dx;

    if (w->timer == 0) {
        w->model->anim = moujaThrowAnim;
        w->model->frame = 0.0f;
        w->animSpeed = 0.1f;
        w->throwing = 0;
        w->vel[0] = 0.0f;
        w->side = shimPlSideCheck(o, shimNearestPl(o));
    }
    w->timer++;
    if (w->throwing == 0) {
        pl = shimNearestPl(o);
        if (plIsInPlay(pl)) {
            tskSendMsg(gGameWork.players[pl], 4, &pos);
            if (eneTurnToPoint(o, pos.x, pos.z, 18.0f) == 1) {
                w->throwing = 1;
            }
        }
    }
    if (w->throwing > 0) {
        if (eneAnimPlayOnce(o, w->model, ANIM_STEP(w->animSpeed)) == 1) {
            if (!eneNearestPlInX(o, w->range)) {
                w->step = w->type;
            }
            w->timer = 0;
        }
        if (w->model->frame >= 7.5f && w->throwing == 1) {
            face = eneFaceWay(o);
            if (face == 4) {
                dx = -15.0f;
            } else {
                dx = 15.0f;
            }
            /* The axe's eneSetup value: its type low, its way high. */
            eneMakeEx(NULL, ENE_MOUJA_ONO, w->zone, w->x + dx, w->y + 10.0f, w->z, 2,
                      (u32 (*)(void *, void *, void *)) ((w->axeType & 0xFFFF) | ((face << 16) & 0xFFFF0000)));
            w->throwing = 2;
            eneSetSd(o, 0x226);
        }
    }
}

/* Plays the walk at a speed from its x velocity: |vx| / 2, 0.3 below 0.1, at most 1. */
void moujaWalkAnim(Obj *o, AnimeFrame *m) {
    Mouja *w = MOUJA(o);
    f32 speed;

    speed = ABS(w->vel[0]) * 0.5f;
    if (ABS(speed) < 0.1f) {
        speed = 0.3f;
    }
    /* An int 1: the game loads 1.0f again for the store (a 1.0f here shares it: measured). */
    if (ABS(speed) > 1) {
        speed = 1.0f;
    }
    eneAnimLoop(o, m, ANIM_STEP(speed));
}

/* The ono's work, at its task + 0x20: an enemy's (EneObj) up to 0x3C. */
typedef struct Ono {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ s32 timer; /* steps in this step */
    /* 0x40 */ f32 homeX; /* where it was thrown from */
    /* 0x44 */ MPolFrame *shadow;
    /* 0x48 */ u8 unused48;
    /* 0x49 */ u8 step; /* its placement's low half (the mouja's axeType) */
    /* 0x4A */ u8 back; /* 1 once 40 from where it was thrown: it slides back */
    /* 0x4B */ u8 way; /* its placement's high half: 4 thrown to -x */
} Ono;

#define ONO(obj) ((Ono *) FX(obj))

#define ATK_ADD_XYZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s16, s32)) acollAtkAddXYZ)
#define DMG_ADD_XYZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s16, s32)) acollDmgAddXYZ)

extern u8 moujaOnoAxeDl[]; /* the axe's display list */

s32 moujaOnoStepFly(Obj *o);
s32 moujaOnoStepSlide(Obj *o);

/* The ono's handler: the axe a mouja throws (kind 0xEB). */
s32 moujaOnoProc(Obj *o, s16 msg, void *arg) {
    Ono *w = ONO(o);
    u32 r;

    switch (msg) {
        case 2:
            o->name = "ono";
            r = eneSetup(o, arg);
            w->step = (u16) (r & 0xFFFF);
            w->way = (u16) ((r >> 16) & 0xFFFF);
            w->model = eneNewPol(o, (Gfx *) moujaOnoAxeDl);
            if (w->model == NULL) {
                return -1;
            }
            w->atk = ATK_ADD_XYZ((Frame *) uda_atk_root_frame, (Frame *) w->model, 30, 0, 50, 20, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 123;
            w->dmg = DMG_ADD_XYZ((Frame *) ene_dmg_root_frame, (Frame *) w->model, 50, 100, 0, 50, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->shadow = shadowCreate();
            if (w->shadow == NULL) {
                return -1;
            }
            w->rot[1] = eneCourseAngle(o, w->way);
            w->timer = 0;
            w->homeX = w->x;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy((Frame *) w->model);
            }
            if (w->atk != NULL) {
                frmDestroy((Frame *) w->atk);
            }
            if (w->dmg != NULL) {
                frmDestroy((Frame *) w->dmg);
            }
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            break;
        case 3:
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->step) {
                case 0:
                    if (moujaOnoStepFly(o) == 1) {
                        return -1;
                    }
                    break;
                case 1:
                    if (moujaOnoStepSlide(o) == 1) {
                        return -1;
                    }
                    break;
                default:
                    PRINTF("ono step error!\n");
                    break;
            }
            eneMoveAndCollide(o, 5.0f, 2.0f, w->z, 4.0f);
            eneSetModelOnCourse(o, (Frame *) w->model);
            if (ABS(w->x - w->homeX) > 120.0f || (w->hit & 2)) {
                PRINTF("ono ONO_RANGE || ON_WALL\n");
                itexPuffDark(w->model->pos[0], w->model->pos[1], w->model->pos[2], 0.05f);
                return -1;
            }
            if (acollAtkWasHit(w->atk) == 1 && (Obj *) w->atk->hitPower != NULL) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, o);
            }
            if (acollDmgWasHit(w->dmg) == 1) {
                eneHitSoundAlt(w->dmg);
                itexPuffYellow(w->model->pos[0], w->model->pos[1], w->model->pos[2], 0.05f);
                return -1;
            }
            if (w->y < -600.0f) {
                PRINTF("ono: 落ちすぎ\n");
                return -1;
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 0.8f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Step 0: flies up spinning and falls; on the ground it puffs and goes to step 1 (its caller
 * sees 0 always). */
s32 moujaOnoStepFly(Obj *o) {
    Ono *w = ONO(o);
    s32 pad;
    FxWork fx;

    if (w->timer == 0) {
        if (w->way == 4) {
            w->vel[0] = -1.0f;
        } else {
            w->vel[0] = 1.0f;
        }
        w->vel[1] = 5.0f;
    }
    w->timer++;
    w->rot[0] += 0.251327419281005882;
    eneFall(o, 0.4f);
    if (w->hit & 1) {
        PRINTF("ono ON_EARTH\n");
        effectPuffDefaults(&fx);
        fx.pos[0] = w->model->pos[0];
        fx.pos[1] = w->model->pos[1];
        fx.pos[2] = w->model->pos[2];
        fx.scale = 0.1f;
        fx.grow = 0.0f;
        fx.vel[1] = 0.05f;
        fx.b = fx.g = fx.r = 100;
        fx.kind = 2;
        effectPuffMake(fx);
        w->rot[0] = 0.0f;
        w->vel[1] = 5.0f;
        shimSdAtModel((Frame *) w->model, 0x2D5);
    }
    return 0;
}

/* Step 1: slides out 40 from where it was thrown, then back; 1 when it lands. */
s32 moujaOnoStepSlide(Obj *o) {
    Ono *w = ONO(o);
    s32 pad[2];

    if (w->timer == 0) {
        w->back = 0;
        w->vel[1] = 0.0f;
    }
    w->timer++;
    w->rot[0] += 0.5235987901164023;
    if (w->back == 0) {
        if (w->way == 4) {
            w->vel[0] -= 0.2222222239f;
        } else {
            w->vel[0] += 0.2222222239f;
        }
        if (ABS(w->x - w->homeX) > 40.0f) {
            w->back = 1;
        }
    } else if (w->back == 1) {
        if (w->way == 4) {
            w->vel[0] += 0.2222222239f;
        } else {
            w->vel[0] -= 0.2222222239f;
        }
    }
    if (w->vel[0] > 2.0f) {
        w->vel[0] = 2.0f;
    } else if (w->vel[0] < -2.0f) {
        w->vel[0] = -2.0f;
    }
    if (w->hit & 1) {
        shimSdAtModel((Frame *) w->model, 0x2D5);
        itexPuffDark(w->model->pos[0], w->model->pos[1], w->model->pos[2], 0.08f);
        return 1;
    }
    return 0;
}

s32 moujaKozStepWait(Obj *o);
s32 moujaKozStepJump(Obj *o);

/* koz_m's handler: a mouja that waits (step 100) until it is let go, jumps out (step 101) and
 * becomes a walking mouja (kind 0xEA, type 1) once a player is near. */
s32 moujaKozProc(Obj *o, s16 msg, void *arg) {
    Mouja *w = MOUJA(o);
    s32 pad;
    s32 way;

    switch (msg) {
        case 2:
            o->name = "koz_m";
            way = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) moujaThrowAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->model->flags |= 0x20;
            w->atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, (Frame *) w->model, 60, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 120;
            w->dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, (Frame *) w->model, 60, 180, 0, 10, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                PRINTF("mouja: SET MISS\n");
                return -1;
            }
            w->shadow = shadowCreate();
            if (w->shadow == NULL) {
                return -1;
            }
            w->step = 100;
            w->rot[1] = eneCourseAngle(o, way);
            w->timer = 0;
            w->homeX = w->x;
            PRINTF("mouja step [%d] dir [%d]\n", w->step, way);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy((Frame *) w->model);
            }
            if (w->atk != NULL) {
                frmDestroy((Frame *) w->atk);
            }
            if (w->dmg != NULL) {
                frmDestroy((Frame *) w->dmg);
            }
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            PRINTF("mouja DESTROY\n");
            break;
        case 3:
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->step) {
                case 100:
                    if (moujaKozStepWait(o) == 1) {
                        return -1;
                    }
                    break;
                case 101:
                    if (moujaKozStepJump(o) == 1) {
                        return -1;
                    }
                    break;
                default:
                    PRINTF("mouja step error!\n");
                    break;
            }
            eneFall(o, 0.4f);
            shimEneMove(o, 10.0f, 8.0f, w->z, 4.0f);
            if (w->hit & 1) {
                w->vel[1] = 0.0f;
            }
            if ((w->hit & 6) && w->step == 100) {
                w->x -= w->vel[0];
                w->z -= w->vel[2];
            }
            if (acollAtkWasHit(w->atk) == 1 && (Obj *) w->atk->hitPower != NULL) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, o);
            }
            if (acollDmgWasHit(w->dmg) == 1) {
                shimEneDeathAct(o, w->dmg, (u32) moujaHalf0Dl, (u32) moujaHalf1Dl);
                return -1;
            }
            if (w->y < -600.0f) {
                PRINTF("mouja: 落ちすぎ\n");
                return -1;
            }
            eneSetModelOnCourse(o, (Frame *) w->model);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Step 100: waits, animating, until its phase is cleared, then goes to step 101. */
s32 moujaKozStepWait(Obj *o) {
    Mouja *w = MOUJA(o);

    if (w->timer == 0) {
        PRINTF("mouja oro step IN\n");
        w->model->anim = moujaKozMWaitAnim;
        w->model->frame = 0.0f;
        w->phase = 1;
        w->vel[0] = 0.0f;
    }
    w->timer++;
    if (w->phase == 0) {
        PRINTF("mouja request\n");
        w->step = 101;
        w->timer = 0;
    }
    eneAnimLoop(o, w->model, ANIM_STEP(w->animSpeed));
    return 0;
}

/* Step 101: jumps out the way it faces, lands, walks; with a player within 20 it makes a mouja
 * (kind 0xEA, type 1) in its place and is deleted (1). */
s32 moujaKozStepJump(Obj *o) {
    Mouja *w = MOUJA(o);
    s32 pad;

    if (w->timer == 0) {
        w->model->anim = moujaWalkModelAnim;
        w->model->frame = 0.0f;
        w->throwing = 0;
        w->phase = 0;
        w->side = eneFaceWay(o);
        if (w->side == 4) {
            w->vel[0] = 0.75f;
        } else {
            w->vel[0] = -0.75f;
        }
        w->hit &= ~1;
        w->vel[1] = 3.0f;
    }
    w->timer++;
    if (w->hit & 1) {
        if (w->phase == 0) {
            w->phase = 1;
            w->vel[1] = 1.5f;
        } else if (w->phase == 1) {
            w->phase = 2;
            w->homeX = w->x;
        } else {
            w->vel[1] = 0.0f;
        }
    }
    if (w->phase == 2) {
        if (w->side == 4) {
            w->vel[0] = 1.5f;
        } else {
            w->vel[0] = -1.5f;
        }
        w->phase = 3;
    } else if (w->phase == 3) {
        if (eneNearestPlInX(o, 20.0f)) {
            eneMakeEx(NULL, ENE_MOUJA, w->zone, w->x, w->y + 20.0f, w->z, 2, (u32 (*)(void *, void *, void *)) 1);
            return 1;
        }
        if (ABS(w->x - w->homeX) > 10.0) {
            w->vel[0] *= -1.0;
        }
        eneTurnAlongCourse(o, 2.0f);
    }
    moujaWalkAnim(o, w->model);
    return 0;
}
