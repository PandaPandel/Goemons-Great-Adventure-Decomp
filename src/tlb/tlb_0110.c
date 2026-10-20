#include "tlb.h"

/* File 110: "cyclops" (a one-eyed giant: it paces 60 either side of where it was placed, turns to
 * a player who comes within 30 and swings at it; five hits beat it, each hit shrinking it). */

#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define ATK_ADD_XYZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s16, s32)) acollAtkAddXYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

/* An animation step: "* 2.0 * 1.0f" (add.d, then a mul.d by 1.0), as in files 78 and 96. */
#define ANIM_STEP(x) ((x) * 2.0 * 1.0f)

#define ABS(x) ((x) > 0.0f ? (x) : -(x))

extern u8 cyclopsStandWalkAnim[]; /* its stand and walk */
extern u8 cyclopsSwingAnim[]; /* its swing */
extern u8 cyclopsHitAnim[]; /* its hit */
extern u8 cyclopsHalf0Dl[];
extern u8 cyclopsHalf1Dl[];

s32 cyclopsDebugAnim = 0; /* the debug step's animation, % 3 */

/* The cyclops's work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct Cyclops {
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
    /* 0x34 */ ACollFrame *atk; /* its body */
    /* 0x38 */ ACollFrame *atk2; /* its head */
    /* 0x3C */ ACollFrame *swing; /* its club, while it swings */
    /* 0x40 */ ACollFrame *dmg;
    /* 0x44 */ s32 timer; /* steps in this step */
    /* 0x48 */ f32 animSpeed; /* the debug step's */
    /* 0x4C */ f32 homeX; /* where it was placed: it paces 60 either side */
    /* 0x50 */ u8 pad50[4];
    /* 0x54 */ u8 type; /* its placement's type */
    /* 0x55 */ u8 step; /* 0 stand, 1 pace, 2 walk, 3 swing, 4 hit, 5 debug */
    /* 0x56 */ u8 flash; /* the red flash's alpha after a hit */
    /* 0x57 */ u8 life; /* hits left */
    /* 0x58 */ u8 turning; /* step 1: 1 turning to the player */
    /* 0x59 */ u8 way; /* eneNearestPlWay's */
    /* 0x5A */ u8 unused5A; /* written 0 by cyclopsStepSwing, never read */
    /* 0x5B */ u8 pad5B;
    /* 0x5C */ MPolFrame *shadow;
} Cyclops;

#define CYCLOPS(obj) ((Cyclops *) FX(obj))

void cyclopsStepDebug(Obj *o);
void cyclopsStepStand(Obj *o);
void cyclopsStepPace(Obj *o);
void cyclopsStepWalk(Obj *o);
void cyclopsStepSwing(Obj *o);
s32 cyclopsStepHit(Obj *o);
void cyclopsAnimWalk(Obj *o, AnimeFrame *m);

/* "cyclops". eneSetup's value goes to the type through a u16 cast, as in file 96: the cast spends
 * the game's two t-registers. */
s32 cyclopsProc(Obj *o, s16 msg, void *arg) {
    Cyclops *w = CYCLOPS(o);
    u32 r;
    s32 pad;
    s32 n;
    s32 pad2[2];

    switch (msg) {
        case 2:
            o->name = "cyclops";
            PRINTF("struct Ene_cyclopssize [%d]\n", sizeof(Cyclops));
            r = eneSetup(o, arg);
            w->type = (u16) (r & 0xFFFF);
            if ((w->model = eneAnimeModelMake(o, (u32) cyclopsStandWalkAnim, NULL, 0)) == NULL) {
                return -1;
            }
            w->model->flags |= 0x20;
            if ((w->atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, (Frame *) w->model, 60, 60, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 120;
            if ((w->atk2 = ATK_ADD_YZ((Frame *) ene_atk_root_frame, (Frame *) w->model, 60, 290, 0, 0)) == NULL) {
                return -1;
            }
            w->atk2->kind = 120;
            if ((w->dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, (Frame *) w->model, 100, 350, 0, 0, 0)) ==
                NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                PRINTF("cyclops: SET MISS\n");
                return -1;
            }
            if ((w->shadow = shadowCreate()) == NULL) {
                return -1;
            }
            w->step = 0;
            w->timer = 0;
            w->homeX = w->x;
            w->rot[1] = eneCourseAngle(o, 4);
            w->life = 5;
            PRINTF("cyclops step [%d]\n", w->step);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy((Frame *) w->model);
            }
            if (w->atk != NULL) {
                frmDestroy((Frame *) w->atk);
            }
            if (w->atk2 != NULL) {
                frmDestroy((Frame *) w->atk2);
            }
            if (w->swing != NULL) {
                frmDestroy((Frame *) w->swing);
            }
            if (w->dmg != NULL) {
                frmDestroy((Frame *) w->dmg);
            }
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            PRINTF("cyclops DESTROY\n");
            break;
        case 3:
            if (w->step == 0) {
                w->model->dl = eneDlEnvColorLit(o, 0xA0, 5, 5, 5, 1);
            } else {
                w->model->dl = eneDlEnvColorLit(o, w->flash, 0xFF, 0, 0, 1);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->step) {
                case 0:
                    cyclopsStepStand(o);
                    break;
                case 1:
                    cyclopsStepPace(o);
                    break;
                case 2:
                    cyclopsStepWalk(o);
                    break;
                case 3:
                    cyclopsStepSwing(o);
                    break;
                case 4:
                    if (cyclopsStepHit(o) != 0) {
                        return -1;
                    }
                    break;
                case 5:
                    cyclopsStepDebug(o);
                    break;
                default:
                    PRINTF("cyclops step error!\n");
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
            if (acollAtkWasHit(w->atk) == 1) {
                if ((Obj *) w->atk->hitPower != NULL) {
                    tskSendMsg((Obj *) w->atk->hitPower, 7, o);
                }
            } else if (acollAtkWasHit(w->swing) == 1) {
                if ((Obj *) w->swing->hitPower != NULL) {
                    tskSendMsg((Obj *) w->swing->hitPower, 7, o);
                }
            } else if (acollAtkWasHit(w->atk2) == 1) {
                if ((Obj *) w->atk2->hitPower != NULL) {
                    tskSendMsg((Obj *) w->atk2->hitPower, 7, o);
                }
            }
            if (acollDmgWasHit(w->dmg) == 1) {
                n = shimAtkKindCheck(w->dmg);
                PRINTF("cyclops : DAMAGE POINT [%d]\n", n);
                while (n != 0) {
                    if (--w->life == 0) {
                        shimEneDeathAct(o, w->dmg, (u32) cyclopsHalf0Dl, (u32) cyclopsHalf1Dl);
                        eneReleaseSpawn(w->spawn);
                        return -1;
                    }
                    eneHitSoundHeavy(w->dmg);
                    n--;
                }
                w->dmg->flags &= ~2;
                w->step = 4;
                w->timer = 0;
            }
            if (w->y < -600.0f) {
                PRINTF("cyclops: 落ちすぎ\n");
                return -1;
            }
            eneSetModelOnCourse(o, (Frame *) w->model);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Step 5: driven by controller 2 (a debug step): the stick moves it, C buttons change the
 * animation's speed, R picks the animation. */
void cyclopsStepDebug(Obj *o) {
    Cyclops *w = CYCLOPS(o);
    s32 max;

    if (w->timer == 0) {
        w->animSpeed = 0.5f;
    }
    w->timer++;
    w->vel[0] = joyGetStickX(1, 0) / 24;
    w->vel[1] = joyGetStickY(1, 0) / 24;
    if (joyGetHeldTime(1, 8) > 0) {
        w->animSpeed += 1.0f;
    }
    if (joyGetHeldTime(1, 4) > 0) {
        w->animSpeed -= 1.0f;
    }
    if (joyGetHeldTime(1, 1) > 0) {
        w->animSpeed += 0.1;
    }
    if (joyGetHeldTime(1, 2) > 0) {
        w->animSpeed -= 0.1;
    }
    if (joyGetHeldTime(1, 0x10) == 1) {
        cyclopsDebugAnim++;
    }
    eneAnimLoop(o, w->model, ANIM_STEP(w->animSpeed));
    if (cyclopsDebugAnim % 3 == 0) {
        w->model->anim = cyclopsStandWalkAnim;
    } else if (cyclopsDebugAnim % 3 == 1) {
        w->model->anim = cyclopsSwingAnim;
    } else if (cyclopsDebugAnim % 3 == 2) {
        w->model->anim = cyclopsHitAnim;
    }
    max = polAnimeLastFrame(w->model);
    PRINTF("max [%d] [%f]\n", max, w->animSpeed);
}

/* Step 0: stands facing along the course. */
void cyclopsStepStand(Obj *o) {
    Cyclops *w = CYCLOPS(o);

    if (w->timer == 0) {
        w->rot[1] = eneCourseAngle(o, 8);
        w->model->anim = cyclopsStandWalkAnim;
        w->model->frame = 5.0f;
    }
    w->timer++;
}

/* Step 1: paces 60 either side of where it was placed; when a player within 30 is on the side it
 * faces, it turns to the player and swings (step 3). */
void cyclopsStepPace(Obj *o) {
    Cyclops *w = CYCLOPS(o);
    f32 target;
    f32 rot;
    f32 diff;

    if (w->timer == 0) {
        w->model->anim = cyclopsStandWalkAnim;
        w->model->frame = 5.0f;
        if ((w->way = eneNearestPlWay(o)) == 4) {
            w->vel[0] = -0.2f;
        } else {
            w->vel[0] = 0.2f;
        }
        w->turning = 0;
    }
    w->timer++;
    switch (w->turning) {
        case 0:
            shimEneSwingX(o, w->homeX - 60.0f, w->homeX + 60.0f, 0.1f, 0.5f);
            eneTurnAlongCourse(o, 10.0f);
            w->way = eneNearestPlWay(o);
            if (eneFaceWay(o) == w->way && eneNearestPlayerDist(o) < 30.0f) {
                w->turning = 1;
                w->vel[0] = 0.0f;
            }
            break;
        case 1:
            target = eneCourseAngle(o, w->way);
            rot = w->rot[1];
            diff = target - rot;
            if (diff > 3.141592741f) {
                rot += 6.283185482f;
            } else if (diff < -3.141592741f) {
                rot -= 6.283185482f;
            }
            if (eneTurnRotYTo(o, target, (target < rot) ? -0.2f : 0.2f) == 0) {
                w->step = 3;
                w->timer = 0;
            }
            break;
    }
    cyclopsAnimWalk(o, w->model);
}

/* Step 2: walks. */
void cyclopsStepWalk(Obj *o) {
    Cyclops *w = CYCLOPS(o);

    if (w->timer == 0) {
        w->model->anim = cyclopsStandWalkAnim;
        w->model->frame = 5.0f;
    }
    w->timer++;
    cyclopsAnimWalk(o, w->model);
}

/* Step 3: swings its club (a box out past frame 5), with a cry by the hits it has left; then paces
 * again (step 1). */
void cyclopsStepSwing(Obj *o) {
    Cyclops *w = CYCLOPS(o);
    f32 speed = 0.15f;

    if (w->timer == 0) {
        w->model->anim = cyclopsSwingAnim;
        w->model->frame = 0.0f;
        w->turning = 0;
        w->unused5A = 0;
        if (w->swing != NULL) {
            w->swing = (ACollFrame *) frmDestroy((Frame *) w->swing);
        }
    }
    w->timer++;
    if (w->timer == 14) {
        if (w->life == 4) {
            eneSetSd(o, 0x45B);
        }
    } else if (w->timer == 17) {
        if (w->life == 3) {
            eneSetSd(o, 0x45C);
        } else if (w->life == 2) {
            eneSetSd(o, 0x45D);
        } else if (w->life == 1) {
            eneSetSd(o, 0x45E);
        }
    }
    if (w->swing == NULL && w->model->frame > 5.0) {
        if ((w->swing = ATK_ADD_XYZ((Frame *) ene_atk_root_frame, (Frame *) w->model, 60, 0, 130, 240, 0)) !=
            NULL) {
            w->swing->kind = 120;
        }
    }
    if (eneAnimPlayOnce(o, w->model, ANIM_STEP(speed)) == 1) {
        if (w->swing != NULL) {
            w->swing = (ACollFrame *) frmDestroy((Frame *) w->swing);
        }
        w->step = 1;
        w->timer = 0;
    }
}

/* Step 4: hit: knocked back from the player, shrinking to its hits left (0.025 each) and flashing
 * red; then paces again (step 1). */
s32 cyclopsStepHit(Obj *o) {
    Cyclops *w = CYCLOPS(o);
    f32 speed = 0.2f;

    if (w->timer == 0) {
        w->model->anim = cyclopsHitAnim;
        w->model->frame = 0.0f;
        w->turning = 0;
        w->vel[0] = 0.0f;
        if ((w->way = eneNearestPlWay(o)) == 4) {
            w->vel[0] = 3.0f;
        } else {
            w->vel[0] = -3.0f;
        }
        if (w->swing != NULL) {
            w->swing = (ACollFrame *) frmDestroy((Frame *) w->swing);
        }
        w->flash = 0xA0;
    }
    w->timer++;
    w->model->scale[2] -= 0.005f;
    w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
    PRINTF("cyclops life[%d]\n", w->life);
    if (w->model->scale[0] < (f32) w->life * 0.025) {
        w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = (f32) w->life * 0.025;
    }
    if (w->flash >= 5) {
        w->flash -= 5;
    }
    w->vel[0] *= 0.9;
    if (eneAnimPlayOnce(o, w->model, ANIM_STEP(speed)) == 1) {
        w->dmg->flags |= 2;
        w->step = 1;
        w->flash = 0;
        w->timer = 0;
    }
    return 0;
}

/* Plays the walk at a speed from its x velocity: |vx| / 2, at least 0.1, at most 1. */
void cyclopsAnimWalk(Obj *o, AnimeFrame *m) {
    Cyclops *w = CYCLOPS(o);
    f32 speed;

    speed = ABS(w->vel[0]) * 0.5f;
    if (ABS(speed) < 0.1f) {
        speed = 0.1f;
    }
    if (ABS(speed) > 1) {
        speed = 1.0f;
    }
    eneAnimLoop(o, m, ANIM_STEP(speed));
}
