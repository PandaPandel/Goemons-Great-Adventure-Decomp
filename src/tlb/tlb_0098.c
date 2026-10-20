#include "tlb.h"

/* The umbrella ghosts (karakasa): "waga" walks the course, opens its umbrella at a player and
 * throws it; "waga_n" is the one a player rides; "waga_k" is the thrown umbrella ("kasa"). */

/* acollAtkAddYZ, acollAtkAddXYZ and acollDmgAddYZ return their box (main declares them void). */
typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz,
                                    s32 a);
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy,
                                     s16 oz, s32 a);
typedef ACollFrame *(*DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height,
                                    s16 oy, s16 oz, s32 a);

/* "waga": the umbrella ghost. */
typedef struct WagaObj {
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
    /* 0x38 */ ACollFrame *swing; /* the umbrella's swing (mode 1) */
    /* 0x3C */ ACollFrame *dmg;
    /* 0x40 */ AnimeFrame *kasa; /* the umbrella's model */
    /* 0x44 */ MPolFrame *shadow;
    /* 0x48 */ u32 timer;
    /* 0x4C */ f32 animSpeed;
    /* 0x50 */ f32 homeX;
    /* 0x54 */ u8 mode;
    /* 0x55 */ u8 dir;
    /* 0x56 */ u8 life;
    /* 0x57 */ u8 alpha;
    /* 0x58 */ u8 step;
    /* 0x59 */ u8 way;
    /* 0x5A */ u8 type;
} WagaObj;

#define WAGA(obj) ((WagaObj *) FX(obj))

/* "waga_n": the umbrella a player rides (waga_noru). */
typedef struct WagaNObj {
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
    /* 0x34 */ ACollFrame *atk; /* while ridden */
    /* 0x38 */ ACollFrame *swing;
    /* 0x3C */ ACollFrame *dmg;
    /* 0x40 */ AnimeFrame *kasa;
    /* 0x44 */ Obj *mark; /* kind 0xE9, over it while it waits */
    /* 0x48 */ MPolFrame *shadow;
    /* 0x4C */ u32 timer;
    /* 0x50 */ u8 pad50[4];
    /* 0x54 */ f32 save; /* the rider's side (wagaNoruSwapSide) */
    /* 0x58 */ s32 ctrl; /* the rider's control (shimEneChangeCtrl) */
    /* 0x5C */ u8 ctrlRet;
    /* 0x5D */ u8 mode; /* 5 waiting for a rider */
    /* 0x5E */ u8 swung;
    /* 0x5F */ u8 pl; /* the rider, 5 none */
    /* 0x60 */ u8 prevMode;
    /* 0x61 */ u8 alpha;
    /* 0x62 */ u8 life;
    /* 0x63 */ u8 flash;
    /* 0x64 */ u8 sounded;
    /* 0x65 */ u8 landed;
} WagaNObj;

#define WAGAN(obj) ((WagaNObj *) FX(obj))

/* "waga_k": the thrown umbrella ("Ene_waga_kasa"), spinning along the course with four
 * fading afterimages. */
typedef struct WagaKObj {
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
    /* 0x34 */ AnimeFrame *ghost[4]; /* the afterimages */
    /* 0x44 */ ACollFrame *atk;
    /* 0x48 */ u32 timer;
    /* 0x4C */ MPolFrame *shadow;
    /* 0x50 */ f32 phase; /* its sway across the course */
    /* 0x54 */ u8 back; /* fading out (from step 30) */
    /* 0x55 */ u8 dir; /* the thrower's facing */
    /* 0x56 */ u8 bright;
    /* 0x57 */ u8 alpha[4]; /* the afterimages' */
    /* 0x5B */ u8 alphaTop;
    /* 0x5C */ u8 type;
} WagaKObj;

#define WAGAK(obj) ((WagaKObj *) FX(obj))

/* player.h declares it (plmove.c); tlb.h does not make it a far call. */
#define plMoveStickX (*plMoveStickX)

#pragma intrinsic(fabs)

/* An animation step (as tlb_0078's): its "* 1.0" survives, so it is a float 1.0f promoted. */
#define ANIM_STEP(x) ((x) * 2.0 * 1.0f)

#define ABS(x) ((x) > 0.0f ? (x) : -(x))

extern u8 wagaBodyAnim3[];

/* The file's .data (0x080037B0-0x080037E3), in address order; wagaStepTest's static animation
 * counter (0x080037E0) ends it. */
u32 wagaBodyAnims[] = { 0x030007EC, 0x03000C98, 0x0300100C, (u32) wagaBodyAnim3 }; /* the body's animations */
u32 wagaKasaAnims[] = { 0x030013A0, 0x03001440, 0x030014D8, 0x03001294 }; /* waga's umbrella's (2: waga_k's, thrown) */
u32 wagaNoruKasaAnims[] = { 0x03001710, 0x030017D4, 0, 0x0300156C }; /* waga_n's umbrella's */

s32 wagaStepTest(Obj *obj);
s32 wagaStepWalk(Obj *obj);
s32 wagaStepSwing(Obj *obj);
s32 wagaStepThrow(Obj *obj);
s32 wagaStepWait(Obj *obj);
s32 wagaStepHit(Obj *obj);
void wagaSetAnim(Obj *obj, AnimeFrame *kasa, s32 i);
void wagaKasaFollow(AnimeFrame *from, AnimeFrame *to);
f32 wagaWalkAnimSpeed(Obj *obj);
s32 wagaNoruStepWait(Obj *obj);
s32 wagaNoruStepShut(Obj *obj);
s32 wagaNoruStepOpen(Obj *obj);
s32 wagaNoruStepSwing(Obj *obj);
s32 wagaNoruControl(Obj *obj);
void wagaNoruSwapSide(Obj *pl, f32 *save);
f32 wagaNoruStick(s32 port);
void wagaMove(Obj *obj, f32 h, f32 r, f32 a);
s32 wagaLand(EneObj *e, f32 h, f32 r);
s32 wagaWallCheck(EneObj *e, f32 h, f32 dx, f32 dz);
void wagaNoruRideVoice(Obj *obj, Obj *pl);
s32 wagaRideStop(Obj *obj, f32 h, f32 r);
void wagaNormalizeXZ(Vec3f *v, Vec3f *out);
s32 wagaRideStopAhead(Obj *obj, Vec3f dir, Vec3f *pos, f32 h, f32 r);
s32 wagaRideStopBehind(Obj *obj, Vec3f dir, Vec3f *pos, f32 h, f32 r);
s32 wagaRideStopUnder(Obj *obj, Vec3f dir, Vec3f *pos, f32 h, f32 r, f32 v, f32 side);

s32 wagaProc(Obj *obj, s16 msg, void *arg) {
    WagaObj *w = WAGA(obj);
    s32 pad[2];
    s32 n;

    switch (msg) {
        case 2:
            obj->name = "waga";
            w->type = eneSetup(obj, (EneArg *) arg);
            w->model = eneAnimeModelMake(obj, wagaBodyAnims[0], NULL, 0);
            if (w->model == NULL) {
                return -1;
            }
            w->model->flags |= 0x20;
            w->kasa = eneAnimeModelMake(obj, wagaKasaAnims[0], NULL, 0);
            if (w->kasa == NULL) {
                return -1;
            }
            w->kasa->flags |= 0x20;
            if (w->type != 0) {
                w->model->scale[2] = 0.05f;
                w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                w->kasa->scale[2] = 0.05f;
                w->kasa->scale[0] = w->kasa->scale[1] = w->kasa->scale[2];
            }
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 100, 140, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 120;
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->model->f, 120, 280, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            if (eneSetOnEarth(obj) == -1) {
                PRINTF("wagashi: SET MISS\n");
                return -1;
            }
            w->mode = 0;
            w->timer = 0;
            w->dir = 4;
            w->homeX = w->x;
            if (w->type != 0) {
                w->life = 2;
            } else {
                w->life = 5;
            }
            w->alpha = 0;
            shimEneSetLowFlags(obj, 3);
            w->shadow = shadowCreate();
            if (w->shadow == NULL) {
                return -1;
            }
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->kasa != NULL) {
                frmDestroy(&w->kasa->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->swing != NULL) {
                frmDestroy(&w->swing->frame);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 3:
            w->model->dl = eneDlEnvColorLit(obj, w->alpha, 255, 0, 0, 1);
            w->kasa->dl = eneDlEnvColorLit(obj, w->alpha, 255, 0, 0, 1);
            break;
        case 0:
            eneStepHookPre(obj);
            switch (w->mode) {
                case 0:
                    if (wagaStepWalk(obj) == 1) {
                        return -1;
                    }
                    break;
                case 1:
                    if (wagaStepSwing(obj) == 1) {
                        return -1;
                    }
                    break;
                case 2:
                    if (wagaStepThrow(obj) == 1) {
                        return -1;
                    }
                    break;
                case 3:
                    if (wagaStepWait(obj) == 1) {
                        return -1;
                    }
                    break;
                case 4:
                    if (wagaStepHit(obj) == 1) {
                        return -1;
                    }
                    break;
                case 6:
                    if (wagaStepTest(obj) == 1) {
                        return -1;
                    }
                    break;
                default:
                    PRINTF("waga step error!\n");
                    break;
            }
            eneFall(obj, 0.4f);
            (*wagaMove)(obj, 10.0f, 10.0f, w->z);
            if (w->hit & 1) {
                w->vel[1] = 0.0f;
            }
            if (w->mode != 4 && (w->hit & 6)) {
                w->vel[0] *= -1.0;
            }
            eneSetModelOnCourse(obj, &w->model->f);
            if (w->mode == 3) {
                n = eneFaceWay(obj);
                if (n == 4) {
                    w->model->rot[1] += 0.5235987902f;
                    w->kasa->rot[1] += 0.5235987902f;
                } else if (n == 1) {
                    w->model->rot[1] -= 0.5235987902f;
                    w->kasa->rot[1] += 0.5235987902f;
                }
            }
            if (w->type != 0) {
                shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            } else {
                shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 2.0f, NULL);
            }
            if (w->kasa != NULL) {
                (*wagaKasaFollow)(w->model, w->kasa);
            }
            if (acollAtkWasHit(w->atk) == 1 && (Obj *) w->atk->hitPower != NULL) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, obj);
            }
            if (acollAtkWasHit(w->swing) == 1 && (Obj *) w->swing->hitPower != NULL) {
                tskSendMsg((Obj *) w->swing->hitPower, 7, obj);
            }
            if (acollDmgWasHit(w->dmg) == 1) {
                n = shimAtkKindCheck(w->dmg);
                PRINTF("waga : DAMAGE POINT [%d]\n", n);
                w->dmg->flags &= ~2;
                w->mode = 4;
                w->timer = 0;
                for (; n != 0; n--) {
                    if (--w->life == 0) {
                        w->dmg->flags |= 2;
                        break;
                    }
                }
                if (w->life != 0 || w->type == 0) {
                    eneHitSoundAlt(w->dmg);
                }
            }
            if (w->y < -600.0f) {
                PRINTF("wagaC: 落ちすぎ\n");
                return -1;
            }
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Mode 6: moved by controller 2's stick (a test mode); R steps through the animations. The
 * animation counter is a function-scope static (0x080037E0, the file's .data): as a static local
 * IDO promotes it to a register (a2, incremented in place, stored by name); an extern or a
 * file-scope static gives an address CSE instead. */
s32 wagaStepTest(Obj *obj) {
    static s32 cnt = 0;
    WagaObj *w = WAGA(obj);
    s32 pad;

    if (w->timer == 0) {
        w->animSpeed = 0.3f;
        wagaSetAnim(obj, w->kasa, cnt);
        polAnimeLastFrame(w->model);
        polAnimeLastFrame(w->kasa);
    }
    w->timer++;
    w->vel[0] = joyGetStickX(1, 0) / 24;
    w->vel[1] = joyGetStickY(1, 0) / 24;
    if (joyGetHeldTime(1, R_TRIG) == 1) {
        w->timer = 0;
        cnt++;
        if (cnt >= 4) {
            cnt = 0;
        }
    }
    eneAnimLoop(obj, w->model, ANIM_STEP(w->animSpeed));
    eneAnimLoop(obj, w->kasa, ANIM_STEP(w->animSpeed));
    return 0;
}

/* Mode 0: swings about its post, turned along the course; a player in front within 30 makes
 * it turn to swing its umbrella (mode 1), one between 50 and 90 to throw it (mode 2, the
 * small ones only). */
s32 wagaStepWalk(Obj *obj) {
    WagaObj *w = WAGA(obj);
    s32 pad;
    f32 speed = wagaWalkAnimSpeed(obj);
    f32 a;
    f32 r;
    f32 d;

    if (w->timer == 0) {
        wagaSetAnim(obj, w->kasa, 0);
        w->step = 0;
        w->way = eneNearestPlWay(obj);
        if (w->way == 4) {
            w->vel[0] = -0.1f;
        } else {
            w->vel[0] = 0.1f;
        }
    }
    w->timer++;
    switch (w->step) {
        case 0:
            shimEneSwingX(obj, w->homeX - 30.0f, w->homeX + 30.0f, 0.04f, 1.0f);
            eneTurnAlongCourse(obj, 10.0f);
            w->way = eneNearestPlWay(obj);
            if (eneFaceWay(obj) == w->way) {
                d = eneNearestPlayerDist(obj);
                if (d < 30.0f) {
                    w->step = 1;
                    w->vel[0] = 0.0f;
                } else if (d > 50.0f && d < 90.0f && w->type == 0) {
                    w->step = 2;
                    w->vel[0] = 0.0f;
                }
            }
            break;
        case 1:
            a = eneCourseAngle(obj, w->way);
            r = w->rot[1];
            d = a - r;
            if (d > 3.141592741f) {
                r += 6.283185482f;
            } else if (d < -3.141592741f) {
                r -= 6.283185482f;
            }
            if (eneTurnRotYTo(obj, a, (a < r) ? -0.200000003f : 0.200000003f) == 0) {
                w->mode = 1;
                w->timer = 0;
            }
            break;
        case 2:
            a = eneCourseAngle(obj, w->way);
            r = w->rot[1];
            d = a - r;
            if (d > 3.141592741f) {
                r += 6.283185482f;
            } else if (d < -3.141592741f) {
                r -= 6.283185482f;
            }
            if (eneTurnRotYTo(obj, a, (a < r) ? -0.200000003f : 0.200000003f) == 0) {
                w->mode = 2;
                w->timer = 0;
            }
            break;
    }
    eneAnimLoop(obj, w->model, ANIM_STEP(speed));
    eneAnimLoop(obj, w->kasa, ANIM_STEP(speed));
    return 0;
}

/* Mode 1: swings its umbrella three times (an attack box once the swing is under way). */
s32 wagaStepSwing(Obj *obj) {
    WagaObj *w = WAGA(obj);
    s32 pad;
    f32 speed = 0.5f;

    if (w->timer == 0) {
        wagaSetAnim(obj, w->kasa, 1);
        w->step = 0;
        if (w->swing != NULL) {
            w->swing = (ACollFrame *) frmDestroy(&w->swing->frame);
        }
    }
    w->timer++;
    if (w->swing == NULL && w->model->frame > 5.0) {
        if (w->swing != NULL) {
            w->swing = (ACollFrame *) frmDestroy(&w->swing->frame);
        }
        w->swing = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 60, 0, 140, 200, 0);
        if (w->swing == NULL) {
            return 1;
        }
        w->swing->kind = 120;
    }
    eneAnimLoop(obj, w->kasa, ANIM_STEP(speed));
    if (eneAnimPlayOnce(obj, w->model, ANIM_STEP(speed)) == 1) {
        w->step++;
        if (w->swing != NULL) {
            w->swing = (ACollFrame *) frmDestroy(&w->swing->frame);
        }
        if (w->step >= 3) {
            w->mode = 0;
            w->timer = 0;
        } else {
            wagaSetAnim(obj, w->kasa, 1);
        }
    }
    return 0;
}

/* Mode 2: throws its umbrella: makes a kind 0xF1 ("waga_k") with its type and facing. */
s32 wagaStepThrow(Obj *obj) {
    WagaObj *w = WAGA(obj);
    f32 speed = 0.05f;
    s32 pad;
    s16 face;
    s16 type;

    if (w->timer == 0) {
        face = eneFaceWay(obj);
        type = w->type;
        wagaSetAnim(obj, w->kasa, 2);
        w->step = 0;
        w->kasa->anim = 0;
        eneMakeEx(NULL, ENE_WAGA_KASA, w->zone, w->x, w->y, w->z, 2,
                  (u32 (*)(void *, void *, void *)) ((type & 0xFFFF) | ((face << 16) & 0xFFFF0000)));
        /* the file's strings give this print's order, not its place or arguments */
        PRINTF("WAGAmax [%d] KASAmax [%d]\n", type, face);
        eneSetSd(obj, 0x2E0);
    }
    w->timer++;
    if (eneAnimPlayOnce(obj, w->model, ANIM_STEP(speed)) == 1) {
        w->mode = 0;
        w->timer = 0;
    }
    return 0;
}

/* Mode 3: waits (it has thrown its umbrella). */
s32 wagaStepWait(Obj *obj) {
    WagaObj *w = WAGA(obj);

    if (w->timer == 0) {
        /* the file's strings give these prints' order, not their places (the block is needed:
         * without it IDO folds w into obj's offsets) */
        PRINTF("!!! katana_flg ERROR !!!\n");
        PRINTF("WAGASHI life[%d]\n", w->life);
    }
    w->timer++;
    return 0;
}

/* Mode 4: hit: knocked back, fading; out of life, the big ones die (a reward for the player
 * who hit it) and the small ones leave a kind 0xF2 behind. */
s32 wagaStepHit(Obj *obj) {
    WagaObj *w = WAGA(obj);
    s32 pad;
    s32 reward;
    s32 i;

    if (w->timer == 0) {
        w->step = 0;
        w->vel[0] = 0.0f;
        w->way = eneNearestPlWay(obj);
        if (w->way == 4) {
            w->vel[0] = 3.0f;
        } else {
            w->vel[0] = -3.0f;
        }
        w->alpha = 255;
    }
    if (w->life == 0) {
        if (w->type != 0) {
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
                return 1;
            }
        } else {
            eneSetSd2(obj, 0x229, 0, 0);
            itexPuffYellow(w->model->pos[0], w->model->pos[1] + 20.0f, w->model->pos[2], 0.2f);
            eneMakeEx(NULL, ENE_WAGA_NORU, w->zone, w->x, w->y + 20.0f, w->z, 2,
                      (u32 (*)(void *, void *, void *)) &w->rot[1]);
        }
        return 1;
    }
    w->alpha -= 20;
    w->vel[0] *= 0.9;
    w->timer++;
    if (w->timer > 10) {
        w->dmg->flags |= 2;
        w->mode = 0;
        w->alpha = 0;
        w->timer = 0;
    }
    return 0;
}

/* Sets the body's animation i and the umbrella's (open or shut by its side). */
void wagaSetAnim(Obj *obj, AnimeFrame *kasa, s32 i) {
    WagaObj *w = WAGA(obj);

    w->model->anim = wagaBodyAnims[i];
    w->model->frame = 0.0f;
    if (w->dir == 4) {
        kasa->anim = wagaKasaAnims[i];
        kasa->frame = 0.0f;
    } else {
        kasa->anim = wagaNoruKasaAnims[i];
        kasa->frame = 0.0f;
    }
}

/* Puts the umbrella where the body is. */
void wagaKasaFollow(AnimeFrame *from, AnimeFrame *to) {
    to->pos[0] = from->pos[0];
    to->pos[1] = from->pos[1];
    to->pos[2] = from->pos[2];
    to->rot[0] = from->rot[0];
    to->rot[1] = from->rot[1];
    to->rot[2] = from->rot[2];
}

/* The walk animation's speed: half the speed along the course, at least 0.1, at most 1. */
f32 wagaWalkAnimSpeed(Obj *obj) {
    WagaObj *w = WAGA(obj);
    f32 s = ABS(w->vel[0] * 0.5f);

    if (ABS(s) < 0.1f) {
        s = 0.1;
    }
    if (s > 1.0f) {
        s = 1.0;
    } else if (s < -1.0f) {
        s = -1.0f;
    }
    return s;
}

/* "waga_n": the umbrella a player rides. It waits (mode 5) until a player jumps on, then
 * floats along under the rider's stick, swinging (mode 1) or shut (mode 3) at the rider's
 * control; hit, it drops the rider. */
s32 wagaNoruProc(Obj *obj, s16 msg, void *arg) {
    WagaNObj *w = WAGAN(obj);
    s32 pad[2];
    f32 *data;

    switch (msg) {
        case 2:
            obj->name = "waga_n";
            data = (f32 *) stageAddrToRam(eneSetup(obj, (EneArg *) arg));
            w->model = eneAnimeModelMake(obj, wagaBodyAnims[0], NULL, 0);
            if (w->model == NULL) {
                return -1;
            }
            w->model->flags |= 0x20;
            w->kasa = eneNewAnime(obj, wagaNoruKasaAnims[0]);
            if (w->kasa == NULL) {
                return -1;
            }
            w->kasa->flags |= 0x20;
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(pl_dmg_root_frame, &w->model->f, 120, 280, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            if (eneSetOnEarth(obj) == -1) {
                PRINTF("waga_n: SET MISS\n");
                return -1;
            }
            shimEneSetLowFlags(obj, 3);
            w->pl = 5;
            w->timer = 0;
            w->mode = 5;
            w->rot[1] = data[0];
            w->alpha = 0;
            w->life = 5;
            w->shadow = shadowCreate();
            if (w->shadow == NULL) {
                return -1;
            }
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->kasa != NULL) {
                frmDestroy(&w->kasa->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->swing != NULL) {
                frmDestroy(&w->swing->frame);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            if (w->mark != NULL) {
                tskDestroy(w->mark);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 3:
            w->model->dl = eneDlEnvColorLit(obj, w->alpha, 255, 0, 0, 1);
            break;
        case 0:
            eneStepHookPre(obj);
            switch (w->mode) {
                case 0:
                    if (wagaNoruStepOpen(obj) == 1) {
                        return -1;
                    }
                    break;
                case 1:
                    if (wagaNoruStepSwing(obj) == 1) {
                        return -1;
                    }
                    break;
                case 3:
                    if (wagaNoruStepShut(obj) == 1) {
                        return -1;
                    }
                    break;
                case 5:
                    if (wagaNoruStepWait(obj) == 1) {
                        return -1;
                    }
                    break;
                default:
                    PRINTF("waga_noru step error!\n");
                    break;
            }
            eneFall(obj, 0.8f);
            if (w->mode != 5) {
                if (plIsPoweredDown(w->pl)) {
                    return -1;
                }
                if (w->mode != 1) {
                    Vec3f vel;
                    Player *pl;

                    pl = PL(gGameWork.players[w->pl]);
                    w->vel[0] = wagaNoruStick(w->pl);
                    vel.x = w->vel[0];
                    vel.y = w->vel[0];
                    vel.z = w->vel[2];
                    if (plMoveTiedXZ(pl, &vel)) {
                        w->vel[2] = 0.0f;
                        w->vel[0] = 0.0f;
                    }
                }
                w->rot[1] = plGetModelYaw(gGameWork.players[w->pl]);
            }
            (*wagaMove)(obj, 10.0f, 10.0f, w->z);
            if (w->hit & 1) {
                if (w->landed == 0 && w->pl != 5) {
                    eneSetSd(obj, 0x2DF);
                }
                w->landed = 1;
                w->vel[1] = 0.0f;
            } else {
                w->landed = 0;
            }
            if (w->hit & 2) {
                w->x -= w->vel[0];
                w->z -= w->vel[2];
            }
            eneSetModelOnCourse(obj, &w->model->f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 2.0f, NULL);
            if (w->kasa != NULL) {
                (*wagaKasaFollow)(w->model, w->kasa);
            }
            if (w->mode != 5 && acollDmgWasHit(w->dmg) == 1) {
                s32 kind = w->dmg->hitKind;

                if (plIsInPlay(w->pl) && kind < 117) {
                    PRINTF("waga : hippari!!!!!\n");
                    plExtDamage(gGameWork.players[w->pl], (s32) obj, kind);
                }
                if (shimIsKind120To126(kind)) {
                    w->dmg->flags &= ~2;
                    if (--w->life == 0) {
                        eneSetSd(obj, 0x11B);
                        itexPuffYellow(w->model->pos[0], w->model->pos[1] + 20.0f, w->model->pos[2], 0.2f);
                    }
                    PRINTF("WAGA life[%d]\n", w->life);
                    w->flash = 30;
                    w->alpha = 255;
                }
            }
            if (w->flash != 0) {
                w->alpha -= 8;
                w->flash--;
            } else {
                w->alpha = 0;
                w->dmg->flags |= 2;
            }
            if (w->y < -600.0f) {
                PRINTF("wagaC: 落ちすぎ\n");
                return -1;
            }
            if (w->ctrl != 0 && w->pl < 4) {
                if (wagaNoruControl(obj) == 1) {
                    return -1;
                }
            }
            eneStepHookPost(obj);
            break;
        case 9: /* the jump table runs to 9 (shimEneChangeCtrl's message) */
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Mode 5: waits for a rider: a player who lands on it (states 15 and 22) and holds nothing
 * gets on: the rider's action 0xD2, an attack box against enemies, control 0x1010. */
s32 wagaNoruStepWait(Obj *obj) {
    WagaNObj *w = WAGAN(obj);
    f32 speed;
    Obj *pl;
    s32 state;

    if (w->timer == 0) {
        wagaSetAnim(obj, w->kasa, 3);
        if (w->swing != NULL) {
            w->swing = (ACollFrame *) frmDestroy(&w->swing->frame);
        }
        w->swing = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 90, 0, 0, 0, 0);
        if (w->swing == NULL) {
            return 1;
        }
        w->swing->kind = 100;
        if (w->atk != NULL) {
            w->atk = (ACollFrame *) frmDestroy(&w->atk->frame);
        }
        w->mark = eneMakeEx(NULL, ENE_NORU_MARK, w->zone, w->x, w->y + 25.0f, w->z, 0, NULL);
        shimEneSetLowFlags(w->mark, 3);
        w->ctrl = 0;
        w->vel[0] = 0.0f;
    }
    speed = 0.4f;
    w->timer++;
    eneAnimLoop(obj, w->model, ANIM_STEP(speed));
    eneAnimLoop(obj, w->kasa, ANIM_STEP(speed));
    if (acollAtkWasHit(w->swing) == 1) {
        pl = (Obj *) w->swing->hitPower;
        if (pl != NULL) {
            state = shimPlGetState((Player *) pl);
            if (state == 15 || state == 22) {
                w->pl = plGetIndex(pl);
                if (shimPlGetHitKind((Player *) pl) == 0) {
                    if (w->mark != NULL) {
                        w->mark = tskDestroy(w->mark);
                    }
                    if (plSetAction(gGameWork.players[w->pl], 0xD2) == 0) {
                        PRINTF("plExtPaChange fault!!!\n");
                        return 1;
                    }
                    w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(pl_atk_root_frame, &w->model->f, 110, 140, 0, 0);
                    if (w->atk == NULL) {
                        return 1;
                    }
                    w->atk->kind = 29;
                    if (w->swing != NULL) {
                        w->swing = (ACollFrame *) frmDestroy(&w->swing->frame);
                    }
                    shimPlSetHitKind((Player *) pl, (s32) obj);
                    wagaNoruSwapSide(gGameWork.players[w->pl], &w->save);
                    w->mode = 3;
                    w->timer = 0;
                    w->ctrl = 0x1010;
                    (*wagaNoruRideVoice)(obj, pl);
                    shimSdAtModel(&w->model->f, 0x2C2);
                }
            }
        }
    }
    return 0;
}

/* Mode 3: ridden, shut. */
s32 wagaNoruStepShut(Obj *obj) {
    WagaNObj *w = WAGAN(obj);
    f32 speed;

    if (w->timer == 0) {
        wagaSetAnim(obj, w->kasa, 3);
    }
    speed = 0.1f;
    w->timer++;
    eneAnimLoop(obj, w->kasa, ANIM_STEP(speed));
    eneAnimLoop(obj, w->model, ANIM_STEP(speed));
    return 0;
}

/* Mode 0: ridden, open; a footstep sound every fifth step on the ground. */
s32 wagaNoruStepOpen(Obj *obj) {
    WagaNObj *w = WAGAN(obj);
    f32 speed;

    if (w->timer == 0) {
        wagaSetAnim(obj, w->kasa, 0);
    }
    if ((w->hit & 1) && w->timer % 5 == 1) {
        eneSetSd(obj, 0x2DE);
    }
    speed = 1.0f;
    w->timer++;
    eneAnimLoop(obj, w->kasa, ANIM_STEP(speed));
    eneAnimLoop(obj, w->model, ANIM_STEP(speed));
    return 0;
}

/* Mode 1: ridden, swings the umbrella once (the rider's attack box from frame 5 to 7); the
 * stick tilts it while it swings. */
s32 wagaNoruStepSwing(Obj *obj) {
    WagaNObj *w = WAGAN(obj);
    f32 speed = 0.4f;
    s32 way;

    if (w->timer == 0) {
        wagaSetAnim(obj, w->kasa, 1);
        if (w->swing != NULL) {
            w->swing = (ACollFrame *) frmDestroy(&w->swing->frame);
        }
        w->swung = 0;
        w->sounded = 0;
        w->vel[0] = 0.0f;
    }
    w->timer++;
    way = eneFaceWay(obj);
    if (way == 4) {
        w->vel[0] += -0.1f;
    } else if (way == 1) {
        w->vel[0] += 0.1f;
    }
    if (w->swing == NULL) {
        if (w->model->frame > 3.0 && w->sounded == 0) {
            eneSetSd(obj, 0x226);
            w->sounded = 1;
        }
        if (w->model->frame > 5.0) {
            if (w->swing != NULL) {
                w->swing = (ACollFrame *) frmDestroy(&w->swing->frame);
            }
            w->swing = ((AtkAddXYZFunc) acollAtkAddXYZ)(pl_atk_root_frame, &w->model->f, 70, 0, 160, 230, 0);
            if (w->swing == NULL) {
                return 1;
            }
            w->swing->kind = 31;
        }
    }
    if (w->model->frame > 7.0) {
        if (w->swing != NULL) {
            w->swing = (ACollFrame *) frmDestroy(&w->swing->frame);
        }
    }
    eneAnimLoop(obj, w->kasa, ANIM_STEP(speed));
    if (eneAnimPlayOnce(obj, w->model, ANIM_STEP(speed)) == 1) {
        w->swung = 1;
        w->sounded = 0;
        if (w->swing != NULL) {
            w->swing = (ACollFrame *) frmDestroy(&w->swing->frame);
        }
    }
    return 0;
}

/* The rider's control: its mode's control code to the rider (shimEneChangeCtrl), and what the
 * rider asks back: shut (1), open (2), swing (3), jump (4), get off and lose it (6, the end),
 * get off (7: it waits again). */
s32 wagaNoruControl(Obj *obj) {
    WagaNObj *w = WAGAN(obj);

    switch (w->mode) {
        case 0:
            w->ctrl = 0x1011;
            break;
        case 1:
            w->ctrl = 0x1012;
            if (w->swung == 1) {
                w->ctrl = 0x1014;
            }
            break;
        case 3:
            w->ctrl = 0x1010;
            break;
        case 5:
            w->ctrl = 0;
            break;
        case 6:
            w->ctrl = 0;
            break;
        default:
            PRINTF("plWagaControl step error!\n");
            break;
    }
    if (w->life == 0) {
        w->ctrl = 0x1017;
    }
    /* shim.c's definition is void (tskSendMsg's result is left in v0); the game passes obj too */
    w->ctrlRet = ((s32 (*)(s32, s32, Obj *)) shimEneChangeCtrl)(w->ctrl, w->pl, obj);
    switch (w->ctrlRet) {
        case 1:
            w->mode = 3;
            break;
        case 2:
            w->mode = 0;
            break;
        case 3:
            w->mode = 1;
            break;
        case 4:
            if (w->hit & 1) {
                w->vel[1] = 7.0f;
                eneSetSd(obj, 0x239);
                PRINTF("JUMP!!\n");
            }
            break;
        case 6:
            if (gGameWork.players[w->pl] != NULL && shimPlGetHitKind((Player *) gGameWork.players[w->pl]) != 0) {
                shimPlSetHitKind((Player *) gGameWork.players[w->pl], 0);
                wagaNoruSwapSide(gGameWork.players[w->pl], &w->save);
            }
            PRINTF("plWagaChangeZ! [%f]\n", w->save);
            return 1;
        default:
            PRINTF("WagaControl step error!\n");
            break;
        case 7:
            if (gGameWork.players[w->pl] != NULL && shimPlGetHitKind((Player *) gGameWork.players[w->pl]) != 0) {
                shimPlSetHitKind((Player *) gGameWork.players[w->pl], 0);
                wagaNoruSwapSide(gGameWork.players[w->pl], &w->save);
            }
            w->ctrl = 0;
            w->mode = 5;
            w->timer = 0;
            PRINTF("WagaControl step CHANGE!\n");
            break;
    }
    if (w->mode != w->prevMode) {
        w->timer = 0;
    }
    w->prevMode = w->mode;
    return 0;
}

/* Swaps the rider's side with the one kept here: kept while the rider stands on something,
 * given back otherwise. */
void wagaNoruSwapSide(Obj *pl, f32 *save) {
    if (PL(pl)->ride != NULL) {
        *save = PL(pl)->side;
    } else {
        PL(pl)->side = *save;
    }
}

/* The rider's stick: 2 or -2 along the course, 0 inside 20. */
f32 wagaNoruStick(s32 port) {
    s32 x = plMoveStickX(port, 0);

    if (fabs(x) < 20.0) {
        return 0.0f;
    }
    return (x >= 0) ? 2.0f : -2.0f;
}

/* "waga_k": the thrown umbrella. Made with its thrower's type and facing (eneMakeEx's value:
 * the facing in the high half); flies 55 steps, then puffs away.
 * Matched 2026-10-13 (the last entry below). History, when it was NON_MATCHING:
 * 250 lines, mix distance 0 (the instructions are the game's): only t-registers
 * and two stack words differ. The game spends one more t-register at "w->dir = v >> 16" (sra
 * t9 where this C has t8) and the rotation stays one off for the rest of the function; its
 * compiler temps sit one word higher (0x2C, 0x30: this C 0x28, 0x2C) and its i at 0x4C (this
 * C 0x44), frame 0x58. Measured: without the pad 268 (frame 0x50); w assigned per case 358
 * (the game's w is the declaration's, sunk into each case); "w->dir = (v = eneSetup()) >> 16"
 * the same as two statements. Scratch forms of the eneSetup lines (u32 v, (u8)/(s8) casts,
 * "& 0xFF", "(s16)", "/ 65536", "% 65536", the dead print before/after, the chain into type)
 * never gave t9: the spent register is elsewhere. "w->bright = w->alphaTop = 0;" (a chain's
 * temps for the two missing words) 308, frame unchanged. Sweep 2026-10-11: "w; v; i; s32 pad;"
 * (one pad word, below the last local) gives the game's frame and every slot: 224 (pad[2] there
 * 246, temps 4 low). The t-register shift is what is left.
 * 2026-10-12 (IDO_NOTES 351's deleted-mask temp, which fixed tlb_0071's and tlb_0077's
 * shifts): "w->dir = (v >> 16) & 0xFF" 175 (sra t5: too many spent); "w->type = v & 0xFF" 175
 * (sra still t8); type stored before dir 136 (sra t8, store order kept). The game's order after
 * the call is t9, t8, t5: a free list, not a count.
 * 2026-10-12 (second try): arg passed as "*(EneArg **) &arg" (tlb_0071 bloodw's lever; arg is in
 * its home already): 136, no change. The pad dropped and "w->bright = w->alphaTop = 0;" (a chain
 * reserves a temp word): 197, frame 0x50 (the chain does not stand for the pad here). The
 * results tool (every call's result kept or dropped, every callee as int): 136-144, nothing.
 * "w->type = v = eneSetup(..); w->dir = v >> 16;" with the pad dropped: 156, frame 0x50 and
 * sra still t8 (the game's sra takes t9, the register the next call's address then reuses).
 * 2026-10-12 (third try, real 136 -> 18): step's "back == 0" arm stores bright before alphaTop
 * (the game's sb 0x56 then 0x5B in both arms; kept, natural); the reloads tool's FAKE
 * "if (obj->name) {}" after the name: 136 -> 10 with that order (name in v0, sra t7, the rest of
 * the function aligned) -- not kept. The permuter (base 845): case 2's facing test through a
 * local, "dir = w->dir; if (dir == 4)" (s16 dir in pad's word): 18, kept: the function from the
 * test on is the game's but for the test's register (v0 here, t6 in the game) and the float
 * pair below it (zero f8 / -1.0 f10 here, f10 / f8 in the game); case 2's head is still one
 * t-register short (sra t8). Both together: 125. No change (137): eneSetup(obj, arg) uncast;
 * eneSetup through an s32-returning cast; "if (arg) {}" before the print; the print's
 * arguments as a bare comma expression; v merged into i (pad0 for v's word); phase = 0.
 * On the 18 form: "w->dir = (v >> 16) & 0xFF" 137 (sra t5); a FAKE "if (obj->flags) {}" after
 * the name 137; the permuter from this form (base 295, ~800 iterations): nothing better.
 * 2026-10-12 (fourth try): the 10 form re-measured (FAKE "if (obj->name) {}" after the name, the test
 * on w->dir, no dir local): 10, all aligned but the head (name v0, sra t7) and the f8/f10 pair, so
 * the pair is its own puzzle. On it: "w->phase = 0" 10, "0.00f" 10 (no change). On the 18 form:
 * "w->vel[1] = -1.0" 18; "w->type = v & 0xFF;" before the dir line 137 (sra t5); "dir = v >> 16;
 * w->dir = dir;" with the test on w->dir 131 (sra t6). The permuter on the 10 form (base 245,
 * ~1100 iterations): nothing.
 * REAL 7 (probes, the function included
 * after tlb_0098.c with only its head varied): ugen's t-registers here go round t5-t8; the game's
 * sra t9 is outside that ring and the ring goes on after it as if the shift took nothing. Only a
 * uopt CSE does that: a dead use of "v >> 16" after the two stores (an empty if, a test of
 * "(u8) (v >> 16) == 4", a print guarded by it) gives sra v1 and puts every later t-register on
 * the game's, from the model's flags on. With the facing test back on w->dir (no dir local,
 * whose word is now the pad): real 7, left only the CSE's register (v1, the game's t9) and the
 * f8/f10 pair. No change to v1 (probes): the test before the stores (no CSE, t8), on w->dir
 * (t8), on w->type, two tests of the shift, also testing v, & 0xFF or obj->name, the type stored
 * first, v as u32, dir stored twice, a dead switch on the shift (waga10.c: the switch v1, the
 * repeated stores no CSE).
 * REAL 3 (same day, probe waga9.c): case 2's "w->vel[1] = -1.0f;" written before the phase's
 * zero (IDO_NOTES 350: the game loads -1.0 first): the f8/f10 pair is the game's and the stores
 * keep the game's order (phase, timer, bright, alphaTop, vel[1]). No change to the pair: a ?:
 * for vel[0], the test inverted, -1 as an int, the alphaTop/bright chain. Left: only the shift's
 * CSE register, v1 here, t9 in the game. A probe of uopt's order (cseorder.c: CSEs live at once
 * get v1, a1, a2, a3, t0, t1, ...) says t9 is not uopt's next choice after v1; not found yet.
 * Without the FAKE (the test on w->dir, vel[1] first): 133.
 * MATCHED 2026-10-13 (IDO_NOTES 284/351 and ugen's own allocator, read with a
 * scratch -finstrument-functions build of vendor ugen.c): ugen's t-register free list is a FIFO.
 * f_get_one_free_reg takes its head (remove_head 0x100213d4), f_free_reg appends a freed register
 * at its tail (append_to_list 0x100213d4), and f_get_one_reg(25) for a call's "la $25" removes t9
 * and the call's free puts it back at the tail. Here the list starts [t6 t7 t8 t9 t5] (init_regs:
 * 14 15 24 25 8-13, gen_regs takes uopt's t0-t4), so right after eneSetup's jal it is
 * [t8 t5 t6 t7 t9] whatever came before, and the shift can only land in t9 after FOUR more ugen
 * temps. They are the packed half's conversions, which as1 deletes ahead of the sb (note 351):
 * "dir = (v >> 16) & 0xFFFF" (s16) is and/sll/sra, "w->dir = dir & 0xFF" the fourth (and), so
 * ugen's chain is sra t8, and t5, sll t6, sra t7, and t9 and as1 leaves "sra t9; sb t9". The
 * game's style (tama1's "no = (setup >> 16) & 0xFFFF", IDO_NOTES 351). Measured: "w->dir =
 * dir" 3 (and t5, sll t6, sra t7: t7), "(u8) dir" 3 (uopt drops the cast), "& 0xFF" MATCH.
 * The local takes pad's word: the frame is unchanged. */
s32 wagaKasaProc(Obj *obj, s16 msg, void *arg) {
    WagaKObj *w = WAGAK(obj);
    s32 v;
    s32 i;
    s16 dir;

    switch (msg) {
        case 2:
            obj->name = "waga_k";
            PRINTF("size struct Ene_waga_kasa [%d]\n", sizeof(WagaKObj));
            v = eneSetup(obj, (EneArg *) arg);
            dir = (v >> 16) & 0xFFFF;
            w->dir = dir & 0xFF;
            w->type = v;
            w->model = eneAnimeModelMake(obj, wagaKasaAnims[2], NULL, 0);
            if (w->model == NULL) {
                return -1;
            }
            w->model->flags |= 0x20;
            for (i = 0; i < 4; i++) {
                w->ghost[i] = eneAnimeModelAlpha(obj, wagaKasaAnims[2], 0);
                if (w->ghost[i] == NULL) {
                    return -1;
                }
                w->ghost[i]->flags |= 0x20;
                w->alpha[i] = 0;
            }
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 50, 240, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 120;
            w->shadow = shadowCreate();
            if (w->shadow == NULL) {
                return -1;
            }
            shimEneSetLowFlags(obj, 3);
            if (w->dir == 4) {
                w->vel[0] = -6.0f;
            } else {
                w->vel[0] = 6.0f;
            }
            w->vel[1] = -1.0f;
            w->phase = 0.0f;
            w->timer = 0;
            w->bright = 0;
            w->alphaTop = 0;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            for (i = 0; i < 4; i++) {
                if (w->ghost[i] != NULL) {
                    frmDestroy(&w->ghost[i]->f);
                }
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 3:
            w->model->dl = eneDlEnvColorLit(obj, w->bright, 255, 255, 0, 0);
            for (i = 0; i < 4; i++) {
                w->ghost[i]->dl = gGfxDlHead;
                gDPSetCombine(gGfxDlHead++, 0xFFFFFF, 0xFFFEFB7D);
                gDPSetEnvColor(gGfxDlHead++, w->bright, w->bright, 0, w->alpha[i]);
                gSPEndDisplayList(gGfxDlHead++);
            }
            break;
        case 0:
            eneStepHookPre(obj);
            if (w->timer == 30) {
                w->back = 1;
            }
            w->z = mathSin(w->phase) * 50.0f;
            w->phase += 0.104719758f;
            if (w->dir == 4) {
                eneAccelMove((EneObj *) w, 0.2f, 0.0333333351f, 0.0f);
            } else {
                eneAccelMove((EneObj *) w, -0.2f, 0.0333333351f, 0.0f);
            }
            if (w->back == 0) {
                w->bright += 4;
                w->alphaTop += 4;
            } else {
                w->bright -= 4;
                w->alphaTop -= 4;
            }
            w->rot[1] += 0.174532930038800771;
            for (i = 0; i < 4; i++) {
                w->alpha[i] -= w->alphaTop / 10;
                if (w->alpha[i] > 200) {
                    w->alpha[i] = 0;
                }
            }
            i = w->timer & 7;
            if (!(i & 1)) {
                wagaKasaFollow(w->model, w->ghost[i / 2]);
                w->alpha[i / 2] = w->alphaTop;
            }
            eneSetModelOnCourse(obj, &w->model->f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1] + 15.0f, w->model->pos[2], 1.0f, NULL);
            if (acollAtkWasHit(w->atk) == 1 && (Obj *) w->atk->hitPower != NULL) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, obj);
            }
            if (w->timer > 54) {
                itexPuffDark(w->model->pos[0], w->model->pos[1] + 20.0f, w->model->pos[2], 0.1f);
                return -1;
            }
            w->timer++;
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The enemies' own move (as tlb_0078's umaNoruMove): stopped by rides
 * (wagaRideStop), moved, landed on three rays at half the height
 * (wagaLand) and its walls checked (wagaWallCheck). */
void wagaMove(Obj *obj, f32 h, f32 r, f32 a) {
    EneObj *e = ENE(obj);
    s32 pad[2];

    e->hit &= ~7;
    wagaRideStop(obj, h, r);
    eneMove(e);
    if ((*wagaLand)(e, h / 2, r)) {
        e->hit |= 1;
    }
    if (wagaWallCheck(e, h / 2, r, a) < 0) {
        e->hit |= 2;
    }
}

/* eneLand on three rays (tlb_0078's umaNoruLand): at x and r either side along the course:
 * 1 on the ground, -1 on a ground of another course (the middle ray: it moves onto that
 * course), 0 in the air. */
s32 wagaLand(EneObj *e, f32 h, f32 r) {
    Vec3f a;
    Vec3f b;
    Vec3f c;
    Vec3f dir;
    Vec3f at;
    CollTri *tri;
    f32 t;

    if (e->vel[1] <= 0.0f) {
        dir.x = 0.0f;
        dir.y = e->vel[1] - h;
        dir.z = 0.0f;
        vramCoursePointXZ(e->zone, e->x, e->z, &a.x, &a.z);
        a.y = e->y + h;
        vramCoursePointXZ(e->zone, e->x - r, e->z, &b.x, &b.z);
        b.y = e->y + h;
        vramCoursePointXZ(e->zone, e->x + r, e->z, &c.x, &c.z);
        c.y = e->y + h;
        tri = vramRayGround(&a, &dir, &at, &t);
        if (tri != NULL) {
            e->y = at.y;
            if (e->zone != 0 && tri->zone != 0 && e->zone != tri->zone) {
                e->zone = tri->zone;
                e->x = vramNearestPlaceXZ(e->zone, a.x, a.z, &e->z);
                return -1;
            }
            return 1;
        }
        if (vramRayGround(&b, &dir, &at, &t) != NULL) {
            e->y = at.y;
            return 1;
        }
        if (vramRayGround(&c, &dir, &at, &t) != NULL) {
            e->y = at.y;
            return 1;
        }
    }
    return 0;
}

/* eneWallCheckEx without the push and with a level ray (tlb_0078's umaNoruWallCheck): -1 a wall
 * (dx, dz) ahead the way the enemy moves, 0 nothing. */
s32 wagaWallCheck(EneObj *e, f32 h, f32 dx, f32 dz) {
    Vec3f to;
    Vec3f from;
    Vec3f dir;
    Vec3f at;
    f32 t;
    s32 pad;

    dx = ABS(dx);
    if (e->vel[0] < 0.0f) {
        dx *= -1.0f;
    }
    dz = ABS(dz);
    if (e->vel[2] < 0.0f) {
        dz *= -1.0f;
    }
    h = ABS(h);
    vramCoursePointXZ(e->zone, e->x + dx, e->z + dz, &to.x, &to.z);
    vramCoursePointXZ(e->zone, e->x, e->z, &from.x, &from.z);
    from.y = e->y + h;
    dir.x = to.x - from.x;
    dir.z = to.z - from.z;
    dir.y = 0.0f;
    if (vramRayGround(&from, &dir, &at, &t) != NULL) {
        return -1;
    }
    return 0;
}

/* A rider's voice for getting on, by the rider's character. */
void wagaNoruRideVoice(Obj *obj, Obj *pl) {
    switch (plGetChara(pl)) {
        case 0:
            shimSdAtModel(&WAGAN(obj)->model->f, 0x393);
            break;
        case 1:
            shimSdAtModel(&WAGAN(obj)->model->f, 0x372);
            break;
        case 2:
            shimSdAtModel(&WAGAN(obj)->model->f, 0x3B2);
            break;
        case 3:
            shimSdAtModel(&WAGAN(obj)->model->f, 0x3CF);
            break;
    }
}

/* Stops the enemy against rides (rideMoveStop) where its move would take it: ahead and behind
 * along the course (a wall: x speed 0) and under it (a floor: on it, y speed 0). */
s32 wagaRideStop(Obj *obj, f32 h, f32 r) {
    EneObj *e = ENE(obj);
    s32 hit = 0;
    Vec3f pos;
    Vec3f dir;
    s32 n;
    s32 pad;
    s32 land;

    vramCourseDirection(e->zone, e->x, &dir);
    wagaNormalizeXZ(&dir, &dir);
    vramCoursePointXZ(e->zone, e->x + ((Vec3f *) e->vel)->x, e->z + ((Vec3f *) e->vel)->z, &pos.x, &pos.z);
    pos.y = e->y + ((Vec3f *) e->vel)->y;
    land = wagaRideStopAhead(obj, dir, &pos, h, r);
    land |= wagaRideStopBehind(obj, dir, &pos, h, r);
    if (land) {
        hit = 1;
        e->hit |= 2;
        e->vel[0] = 0.0f;
    }
    land = 0;
    if (e->vel[1] < 0.0f) {
        land = n = wagaRideStopUnder(obj, dir, &pos, h, r, e->vel[1], 0.0f);
        if (n == 0) {
            land |= n = wagaRideStopUnder(obj, dir, &pos, h, r, e->vel[0], -1.0f);
        }
        if (n == 0) {
            land |= wagaRideStopUnder(obj, dir, &pos, h, r, e->vel[0], 1.0f);
        }
    }
    if (land) {
        hit++;
        e->hit |= 1;
        e->y = pos.y;
        e->vel[1] = 0.0f;
    }
    return hit;
}

/* The course direction made level and of length 1 (x and z), y scaled with them. */
void wagaNormalizeXZ(Vec3f *v, Vec3f *out) {
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

/* A ride ahead: rideMoveStop from half the height up, along the course and down. */
s32 wagaRideStopAhead(Obj *obj, Vec3f dir, Vec3f *pos, f32 h, f32 r) {
    Vec3f p;
    Vec3f d;
    Vec3f out;
    s32 pad[4];

    p.x = pos->x;
    p.y = pos->y + r * 0.5f;
    p.z = pos->z;
    d.x = dir.x * h * 0.5f;
    d.y = -h * 0.2f;
    d.z = dir.z * h * 0.5f;
    if (rideMoveStop(&p, &d, &out, (s32) obj)) {
        return 1;
    }
    return 0;
}

/* A ride behind: as wagaRideStopAhead the other way. */
s32 wagaRideStopBehind(Obj *obj, Vec3f dir, Vec3f *pos, f32 h, f32 r) {
    Vec3f p;
    Vec3f d;
    Vec3f out;
    s32 pad[4];

    p.x = pos->x;
    p.y = pos->y + r * 0.5f;
    p.z = pos->z;
    d.x = -dir.x * h * 0.5f;
    d.y = -r * 0.2f;
    d.z = -dir.z * h * 0.5f;
    if (rideMoveStop(&p, &d, &out, (s32) obj)) {
        return 1;
    }
    return 0;
}

/* A ride underfoot (at side times 0.3 h along the course): lands the place on it. */
s32 wagaRideStopUnder(Obj *obj, Vec3f dir, Vec3f *pos, f32 h, f32 r, f32 v, f32 side) {
    Vec3f p;
    Vec3f d;
    Vec3f out;
    s32 pad[2];

    p.x = pos->x + side * dir.x * 0.3f * h;
    p.y = pos->y + r * 0.5f;
    p.z = pos->z + side * dir.z * 0.3f * h;
    d.x = 0.0f;
    d.y = -r * 0.5f - ABS(v);
    d.z = 0.0f;
    if (rideMoveStop(&p, &d, &out, 0)) {
        pos->y = p.y + out.y;
        return 1;
    }
    return 0;
}
