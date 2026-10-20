#include "tlb.h"

/* acoll.c's box makers return the new box (their C falls off the end with _atkLinkAdd's and
 * _dmgLinkAdd's result in v0); tlb.h declares them void as their definitions are. */
typedef ACollFrame *(*AcollAtkAddXYZFunc)(Frame *, Frame *, u16, s16, s16, s16, s32);
typedef ACollFrame *(*AcollDmgAddYZFunc)(Frame *, Frame *, u16, u16, s16, s16, s32);

/* "siro_j": the castle soldier's spawner. Once, it makes a kind 0xE3 ("sirobz") where it
 * stands, with its placement's type. */
typedef struct SiroJObj {
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
    /* 0x34 */ u8 pad34[0x18];
    /* 0x4C */ u8 count; /* soldiers made */
    /* 0x4D */ u8 type;
} SiroJObj;

/* "sirobz": a castle soldier, fading in, then swinging about its post (type 0), along the
 * course after the nearest player (1) or dropping onto it from above (2). */
typedef struct SirobzObj {
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
    /* 0x40 */ s32 timer;
    /* 0x44 */ f32 homeX;
    /* 0x48 */ f32 homeY;
    /* 0x4C */ f32 targetX;
    /* 0x50 */ s32 alpha;
    /* 0x54 */ s16 alphaStep;
    /* 0x56 */ u8 mode; /* 0 the stick, 1 fading in, 2-4 by its type */
    /* 0x57 */ u8 step;
    /* 0x58 */ u8 type;
} SirobzObj;

#define SIROBZ(obj) ((SirobzObj *) FX(obj))

extern u8 sirobzAnim[];

void sirobzStepStick(Obj *obj);
void sirobzStepAppear(Obj *obj);
void sirobzStepPost(Obj *obj);
void sirobzStepSway(Obj *obj);
void sirobzStepDrop(Obj *obj);
void sirobzLean(Obj *obj, f32 scale);

s32 siroJProc(Obj *obj, s16 msg, void *arg) {
    SiroJObj *w = (SiroJObj *) FX(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "siro_j";
            w->type = eneSetup(obj, (EneArg *) arg);
            w->model = eneNewAnime(obj, 0);
            w->count = 0;
            shimEneSetLowFlags(obj, 2);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
        case 0:
            eneStepHookPre(obj);
            if (w->count == 0) {
                eneMakeEx(NULL, ENE_SIROBZ, w->zone, w->x, w->y, w->z, 2, (u32 (*)(void *, void *, void *)) w->type);
                w->count++;
            }
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 sirobzProc(Obj *obj, s16 msg, void *arg) {
    SirobzObj *w = (SirobzObj *) FX(obj);
    s32 pad[7];
    s32 reward;
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "sirobz";
            w->type = eneSetup(obj, (EneArg *) arg);
            PRINTF("sirobz type is %d\n", w->type);
            w->model = eneAnimeModelAlpha(obj, (u32) sirobzAnim, 0);
            if (w->model == NULL) {
                return -1;
            }
            w->atk = ((AcollAtkAddXYZFunc) acollAtkAddXYZ)((Frame *) ene_atk_root_frame, &w->model->f, 40, 0, 10, 24, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 120;
            w->dmg = ((AcollDmgAddYZFunc) acollDmgAddYZ)((Frame *) ene_dmg_root_frame, &w->model->f, 80, 120, -60, 24, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->shadow = shadowCreate();
            if (w->shadow == NULL) {
                return -1;
            }
            w->mode = 1;
            w->timer = 0;
            w->alpha = 0;
            w->alphaStep = 1;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
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
        case 3:
            w->model->dl = gGfxDlHead;
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            eneStepHookPre(obj);
            switch (w->mode) {
                case 0:
                    sirobzStepStick(obj);
                    break;
                case 1:
                    sirobzStepAppear(obj);
                    break;
                case 2:
                    sirobzStepPost(obj);
                    break;
                case 3:
                    sirobzStepSway(obj);
                    break;
                case 4:
                    sirobzStepDrop(obj);
                    break;
                default:
                    PRINTF("sirobz step error!\n");
                    break;
            }
            if (w->mode != 1) {
                if (acollAtkWasHit(w->atk) == 1 && (Obj *) w->atk->hitPower != NULL) {
                    tskSendMsg((Obj *) w->atk->hitPower, 7, obj);
                }
            }
            if (acollDmgWasHit(w->dmg) == 1) {
                gGameWork.kills++;
                PRINTF("eneDeathAct(): %d 人斬り\n", gGameWork.kills);
                reward = 0;
                switch (w->dmg->hitKind) {
                    case 18:
                    case 19:
                    case 20:
                        eneSetSd2(obj, 0x22D, 0, 0);
                        eneBlowOff(obj, w->dmg);
                        reward = 1;
                        break;
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                        reward = 1;
                    default:
                        eneSetSd2(obj, 0x229, 0x22A, 0);
                        effectBurst(w->model->pos[0], w->model->pos[1] + 5.0f, w->model->pos[2]);
                        break;
                }
                if (reward) {
                    for (i = 0; i < 4; i++) {
                        if ((Obj *) w->dmg->hitPower != NULL && (Obj *) w->dmg->hitPower == gGameWork.players[i]) {
                            eneKillReward(obj, PL(gGameWork.players[i])->index, 0);
                        }
                    }
                }
                return -1;
            }
            eneMove((EneObj *) w);
            if (eneLandOff((EneObj *) w, 10.0f, -5.0f)) {
                w->hit |= 1;
            }
            if (w->hit & 2) {
                w->vel[0] *= -1.0f;
            }
            eneTurnAlongCourse(obj, 10.0f);
            (*sirobzLean)(obj, 1.0f);
            eneSetModelOnCourse(obj, &w->model->f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.5f, NULL);
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Mode 0: moved by controller 2's stick (a test mode). */
void sirobzStepStick(Obj *obj) {
    SirobzObj *w = (SirobzObj *) FX(obj);
    s32 pad[2];

    if (w->timer == 0) {
        w->alpha = 255;
    }
    w->timer++;
    w->vel[0] = joyGetStickX(1, 0) / 12;
    w->vel[1] = joyGetStickY(1, 0) / 12;
}

/* Mode 1: fading in where it stands, then on to its type's mode. */
void sirobzStepAppear(Obj *obj) {
    SirobzObj *w = (SirobzObj *) FX(obj);

    if (w->timer == 0) {
        w->vel[0] = w->vel[1] = w->vel[2] = w->rot[0] = w->rot[2] = 0.0f;
        w->rot[1] = 1.570796371f;
        w->alpha = 0;
        w->step = 1;
        w->homeX = w->x;
        w->homeY = w->y;
    }
    w->timer++;
    if (w->step == 1) {
        w->alpha += 12.0;
        if (w->alpha >= 256) {
            w->alpha = 255;
            w->step = 2;
        }
    }
    if (w->step == 2) {
        w->timer = 0;
        w->step = 0;
        if (w->type == 1) {
            w->mode = 3;
        } else if (w->type == 2) {
            w->mode = 4;
        } else {
            w->mode = 2;
        }
    }
}

/* Mode 2 (type 0): sways about its post, near the nearest player's place along the course. */
/* The game keeps obj in its home (reloaded for each call) and w alone in s0, the old timer in
 * v1: the FAKE empty test of the timer gives that; without it obj takes s1. */
void sirobzStepPost(Obj *obj) {
    SirobzObj *w = (SirobzObj *) FX(obj);
    s32 pad;
    f32 side;
    f32 height;
    f32 place;
    s32 zone;
    s32 pl = shimNearestPl(obj);
    f64 rate = 1.0;

    if (w->timer) {} /* FAKE */
    w->timer++;
    if (shimPlNearCheck(obj, 100.0f)) {
        if (w->step == 0) {
            shimSdAtModel(&w->model->f, 0x292);
            w->step = 1;
        }
    }
    eneAnimLoop(obj, w->model, ((0.0f < w->vel[0]) ? w->vel[0] : -w->vel[0]) * 0.5f * 2.0 * rate);
    plAskPos(gGameWork.players[pl], &place, &height, &side, &zone);
    if (sqrtf((w->x - w->homeX) * (w->x - w->homeX)) < 100.0f) {
        w->targetX = place;
    }
    w->alpha -= w->alphaStep * 2;
    if (w->alpha > 208) {
        w->alpha = 208;
        w->alphaStep = -w->alphaStep;
    } else if (w->alpha < 144) {
        w->alphaStep = -w->alphaStep;
    }
    shimEneSwingX(obj, w->targetX - 2.0, w->targetX + 2.0, 0.03f, 1.0f);
    shimEneSwingY(obj, height + 20.0, height + 27.0, 0.06f, 1.0f);
}

/* Mode 3 (type 1): sways about where it appeared. */
/* The game makes w (obj + 0x20, s0) first thing and reads the timer and step through it;
 * without the FAKE empty test of obj IDO folds those first reads into obj's offsets and makes
 * w later. "f64 rate" is the game's mul.d by a register 1.0 (a literal folds away). */
void sirobzStepSway(Obj *obj) {
    SirobzObj *w = (SirobzObj *) FX(obj);
    f32 accel;
    f64 rate = 1.0;

    if (obj) {} /* FAKE: keeps obj live from the top (found by empties) */
    w->timer++;
    if (w->step == 0 && shimPlNearCheck(obj, 100.0f)) {
        shimSdAtModel(&w->model->f, 0x292);
        w->step = 1;
    }
    eneAnimLoop(obj, w->model, ((0.0f < w->vel[0]) ? w->vel[0] : -w->vel[0]) * 0.5f * 2.0 * rate);
    w->alpha -= w->alphaStep * 2;
    if (w->alpha > 208) {
        w->alpha = 208;
        w->alphaStep = -w->alphaStep;
    } else if (w->alpha < 144) {
        w->alphaStep = -w->alphaStep;
    }
    accel = 0.05f;
    shimEneSwingX(obj, w->homeX - 20.0, w->homeX + 20.0, accel, accel * 10.0f);
    accel = 0.1f;
    shimEneSwingY(obj, w->homeY - 2.0, w->homeY + 2.0, accel, accel * 10.0f);
}

/* Mode 4 (type 2): follows the nearest player along the course, then drops onto it.
 * "f64 rate = 1.0" gives the game's mul.d by a register 1.0. The speed (the ternary) is a
 * local declared before a pad word, which puts w's spill at the game's 0x28 (the permuter's
 * find). FAKE: the empty test of the timer before its increment loads it into a0 as the game
 * does (mode 2's form). */
void sirobzStepDrop(Obj *obj) {
    SirobzObj *w = (SirobzObj *) FX(obj);
    f32 d;
    f64 rate = 1.0;
    f32 side;
    f32 height;
    f32 place;
    s32 zone;
    s32 pl;
    f32 speed;
    s32 pad;

    pl = shimNearestPl(obj);
    if (w->timer) {} /* FAKE */
    w->timer++;
    if (w->step == 0 && shimPlNearCheck(obj, 100.0f)) {
        shimSdAtModel(&w->model->f, 0x292);
        w->step = 1;
    }
    plAskPos(gGameWork.players[pl], &place, &height, &side, &zone);
    if (place < w->x) {
        d = w->x - place;
    } else {
        d = -(w->x - place);
    }
    if (d < 20.0f) {
        w->step = 2;
    }
    speed = (0.0f < w->vel[0]) ? w->vel[0] : -w->vel[0];
    eneAnimLoop(obj, w->model, speed * 0.5f * 2.0 * rate);
    w->alpha -= w->alphaStep * 2;
    if (w->alpha > 208) {
        w->alpha = 208;
        w->alphaStep = -w->alphaStep;
    } else if (w->alpha < 144) {
        w->alphaStep = -w->alphaStep;
    }
    if (w->step == 2) {
        shimEneSwingY(obj, height + 100.0f, height + 100.0f, 0.1f, 2.0f);
    } else {
        shimEneSwingX(obj, place, place, 0.1f, 0.8f);
        shimEneSwingY(obj, height + 15.0f, height + 15.0f, 0.1f, 1.0f);
    }
}

/* Leans it with its speed along the course (rot[0] and rot[2], opposite ways). */
void sirobzLean(Obj *obj, f32 scale) {
    SirobzObj *w = (SirobzObj *) FX(obj);

    if (w->vel[0] * 0.0174532930038800785 * scale * 10.0 > 0.0) {
        w->rot[0] = w->vel[0] * 0.0174532930038800785 * scale * 10.0;
        scale = -(w->vel[0] * 0.0174532930038800785 * scale * 10.0);
    } else {
        w->rot[0] = scale = -(w->vel[0] * 0.0174532930038800785 * scale * 10.0);
    }
    w->rot[2] = scale;
}
