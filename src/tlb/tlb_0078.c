#include "tlb.h"

/* File 78: "uma", the horse enemy with its haniwa rider (kinds 0xB0, 0xB1, 0xE4, 0xE5), the
 * horse a player rides ("noru"), the rider thrown off a beaten horse ("hani_eff", kind 0xE7) and
 * the horse's shot ("uma_tama", kind 0xE8). Every function matches (umaNoruStepRide, the last,
 * on 2026-10-11). The animation tables are flat, "[type * 4 + anim]" (a [type][anim] index
 * spends one t-register less); the animation step is "spd * 2.0 * 1.0f" with spd a variable
 * (ANIM_STEP); the compiled-out prints are the file's strings and some of them (a switch
 * default) decide registers. */

/* The horse's work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct UmaWork {
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
    /* 0x34 */ AnimeFrame *part; /* the horse's second model (umaPartAnims's animations,
                                * the same files as the body's) */
    /* 0x38 */ ACollFrame *atk; /* its attack box (atkMode says what it is) */
    /* 0x3C */ ACollFrame *dmg;
    /* 0x40 */ u16 timer; /* steps in this step */
    /* 0x42 */ u8 owner; /* whose attack root it uses (umaAtkRoots): 0-3 a player, 4 the
                         * enemies, 5 none */
    /* 0x43 */ u8 atkMode; /* 0 an attack box, 1 an animated chain, 2 none */
    /* 0x44 */ u8 type; /* the kind's type: picks the animations and files */
    /* 0x45 */ u8 step;
    /* 0x46 */ u8 sound; /* the gallop's sound phase */
    /* 0x47 */ u8 neighed;
    /* 0x48 */ u8 puff; /* a dust puff made this stride */
    /* 0x49 */ u8 life;
    /* 0x4A */ u8 envA;
    /* 0x4B */ u8 alpha; /* the rider's (it blinks while the horse is hurt) */
    /* 0x4C */ u8 sub; /* the step's own phase */
    /* 0x4D */ u8 dead;
    /* 0x4E */ u8 pad4E[2];
    /* 0x50 */ f32 baseX; /* where it patrols around */
    /* 0x54 */ MPolFrame *shadow;
    /* 0x58 */ AnimeFrame *fx; /* the haniwa rider's model, drawn with its alpha ("effect" is a
                              * function of main's) */
} UmaWork;

#define UMA(obj) ((UmaWork *) FX(obj))

/* The rideable horse's work ("noru"): the horse's up to 0x4C. */
typedef struct NoruWork {
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
    /* 0x34 */ AnimeFrame *part;
    /* 0x38 */ ACollFrame *atk;
    /* 0x3C */ ACollFrame *dmg;
    /* 0x40 */ u16 timer;
    /* 0x42 */ u8 owner; /* the player riding it (0-3), 5 none */
    /* 0x43 */ u8 atkMode;
    /* 0x44 */ u8 type;
    /* 0x45 */ u8 step;
    /* 0x46 */ u8 sound;
    /* 0x47 */ u8 neighed;
    /* 0x48 */ u8 puff;
    /* 0x49 */ u8 life;
    /* 0x4A */ u8 envA;
    /* 0x4B */ u8 unused4B;
    /* 0x4C */ Obj *mark; /* kind 0xE9, made while it waits for a rider */
    /* 0x50 */ u8 phase; /* 0 ridden, 1 rearing or jumping (the rider's 3) */
    /* 0x51 */ u8 anim; /* 1 standing, 2 running */
    /* 0x52 */ u8 hurt; /* frames left blinking after a hit */
    /* 0x53 */ u8 unused53;
    /* 0x54 */ s32 ctrl; /* the rider's control (shimEneChangeCtrl) */
    /* 0x58 */ s32 ctrlRet;
    /* 0x5C */ f32 save; /* a rider's value kept while it rides (umaNoruKeepSide) */
} NoruWork;

#define NORU(obj) ((NoruWork *) FX(obj))

/* The haniwa rider thrown off a beaten horse ("hani_eff", kind 0xE7): it flies off the way the
 * horse faced and bursts where it lands. */
typedef struct HaniWork {
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
    /* 0x34 */ u8 pad34[8];
    /* 0x3C */ f32 baseY;
} HaniWork;

#define HANI(obj) ((HaniWork *) FX(obj))

/* The horse's shot ("uma_tama", kind 0xE8): fired by a rearing horse (umaFire),
 * its maker's argument the way it faced (high half) and its owner (low half). */
typedef struct TamaWork {
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
    /* 0x38 */ f32 startX;
    /* 0x3C */ u8 owner; /* 0-3 a rider's, 4 the horse's own */
} TamaWork;

#define TAMA(obj) ((TamaWork *) FX(obj))

/* An animation step: its "* 1.0" survives in every function, so it is not a double literal
 * (IDO drops "x * 1.0"; a float 1.0f promoted is kept: measured), and the speed it scales is a
 * variable (a literal speed folds the whole product). */
#define ANIM_STEP(x) ((x) * 2.0 * 1.0f)

#define ABS(x) ((x) > 0.0f ? (x) : -(x))

extern u8 umaHaniAnim0[];
extern u8 umaType1Part1Anim[];
extern u8 umaType1Part2Anim[];
extern u8 umaType2Part2Anim[];
extern u8 umaType2Part3Anim[];
extern u8 umaType0Part3Anim[];
extern u8 umaHaniAnim1[];
extern u8 umaType2Body2Anim[];
u32 umaHaniAnims[] = { (u32) umaHaniAnim0, (u32) umaHaniAnim1 };
/* player.h declares it (plmove.c); tlb.h does not make it a far call. */
#define plMoveStickX (*plMoveStickX)

#pragma intrinsic(fabs)

/* The horse's and the rider's animations, four per type, indexed [type * 4 + anim]: a
 * two-dimensional [type][anim] spends one t-register less than the game (measured). */
u32 umaBodyAnims[] = { 0x03000B74, 0x03001108, 0x03001260, 0x030013E4, 0x03000A24, 0x03000CF4, 0x03000B70, 0x03000CF4, 0x03000A34, 0x03000D04, (u32) umaType2Body2Anim, 0x03000D04 };
u32 umaPartAnims[] = { 0x03000214, 0x03000534, 0x030006C8, (u32) umaType0Part3Anim, 0x03000214, (u32) umaType1Part1Anim, (u32) umaType1Part2Anim, 0x030006D4, 0x03000214, 0x030003E8, (u32) umaType2Part2Anim, (u32) umaType2Part3Anim };
ACollFrame **umaAtkRoots[] = { &pl_atk_root_frame, &pl_atk_root_frame, &pl_atk_root_frame, &pl_atk_root_frame, &ene_atk_root_frame, &uda_atk_root_frame, &pl_dmg_root_frame, &pl_dmg_root_frame, &pl_dmg_root_frame, &pl_dmg_root_frame, &ene_dmg_root_frame, &uda_atk_root_frame, NULL, NULL };
extern u8 umaAtkChain[]; /* the attack chain's animation, in file 0x2F (segment 3) */
extern u8 umaHaniAnim0[]; /* the haniwa rider's animation (file 0x2E) */
extern u8 umaTamaShotDl[]; /* the shot's display list */

void umaStepStand(Obj *obj);
void umaStepWait(Obj *obj);
void umaStepCharge(Obj *obj);
void umaStepShoot(Obj *obj);
void umaStepDamage(Obj *obj);
void umaNoruStepWait(Obj *obj);
void umaNoruStepRide(Obj *obj);
void umaNoruStepHurt(Obj *obj);
void umaNoruMountVoice(Obj *obj, Obj *pl);
f32 umaNoruStickSpeed(s32 port);
void umaPuff(Obj *obj, f32 speed);
void umaHoofSound(Obj *obj);
void umaModelFollow(AnimeFrame *src, AnimeFrame *dst);
void umaNoruKeepSide(Obj *pl, f32 *save);
void umaNoruMove(Obj *obj, f32 h, f32 r, f32 a);
s32 umaNoruLand(EneObj *e, f32 h, f32 r);
s32 umaNoruWallCheck(EneObj *e, f32 h, f32 dx, f32 dz);
void umaRunAnim(Obj *obj, f32 speed, s32 back);
s32 umaRear(Obj *obj, AnimeFrame *fx);
s32 umaFire(Obj *obj, AnimeFrame *fx);
s32 umaSwing(Obj *obj, f32 min, f32 max, f32 accel);
s32 umaTurnToPlayer(Obj *obj, f32 div);
void umaAnimSet(Obj *obj, s32 anim, s32 frame);
void umaHaniAnimSet(Obj *obj, s32 anim, s32 frame);
void umaAtkSet(Obj *obj, s32 mode);
void umaTamaAtkSet(Obj *obj);
void umaStepSet(Obj *obj, s8 step);

/* The horse's handler. w is set at its declaration: IDO rebuilds it from obj's home after each
 * call ("addiu s0,a0,0x20" after eneSetup); the kind switch's "return -1" default is written
 * first; "by" takes the hit power straight into a0; the table's "[type * 4]" is the last lever
 * (a [type][0] index spends one t-register less). */
s32 umaProc(Obj *obj, s16 msg, void *arg) {
    UmaWork *w = UMA(obj);
    s32 file;
    Obj *by;

    switch (msg) {
        case 2:
            obj->name = "uma";
            eneSetup(obj, arg);
            switch (w->kind->kind) {
                default:
                    PRINTF("uma: SET MISS\n");
                    return -1;
                case 0xB0:
                case 0xE4:
                    w->type = 0;
                    break;
                case 0xB1:
                case 0xE5:
                    w->type = 2;
                    break;
            }
            PRINTF("uma size        %d\n", sizeof(UmaWork));
            PRINTF("uma_noru size   %d\n", sizeof(NoruWork));
            PRINTF("effect size     %d\n", sizeof(HaniWork));
            PRINTF("uma_tama size   %d\n", sizeof(TamaWork));
            if (w->type == 0) {
                file = 0x2F;
            } else if (w->type == 1) {
                file = 0x30;
            } else {
                file = 0x31;
            }
            w->model = eneAnimeModelMake(obj, umaBodyAnims[w->type * 4], NULL, 0);
            if (w->model == NULL) {
                return -1;
            }
            floadGetSetRam((void **) w->model->segs, file);
            w->part = eneAnimeModelMake(obj, umaPartAnims[w->type * 4], NULL, 0);
            if (w->part == NULL) {
                return -1;
            }
            floadGetSetRam((void **) w->part->segs, file);
            w->fx = eneAnimeModelAlpha(obj, umaHaniAnims[0], 0xFF);
            if (w->fx == NULL) {
                return -1;
            }
            floadGetSetRam((void **) w->fx->segs, 0x2E);
            w->model->flags |= 0x20;
            w->part->flags |= 0x20;
            w->fx->flags |= 0x20;
            w->dmg = ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)(
                (Frame *) ene_dmg_root_frame, (Frame *) w->model, 80, 180, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->shadow = shadowCreate();
            if (w->shadow == NULL) {
                return -1;
            }
            if (eneSetOnEarth(obj) == -1) {
                return -1;
            }
            umaAnimSet(obj, 0, 0);
            umaHaniAnimSet(obj, 0, 0);
            w->atkMode = 2;
            umaStepSet(obj, 1);
            w->life = 3;
            w->alpha = 0xFF;
            w->sub = 0;
            w->dead = 0;
            w->owner = 4;
            w->vel[0] = 0.0f;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy((Frame *) w->model);
            }
            if (w->part != NULL) {
                frmDestroy((Frame *) w->part);
            }
            if (w->fx != NULL) {
                frmDestroy((Frame *) w->fx);
            }
            if (w->atk != NULL) {
                if (w->atkMode == 0) {
                    frmDestroy((Frame *) w->atk);
                } else if (w->atkMode == 1) {
                    aniAtkDelete(w->atk);
                }
            }
            if (w->dmg != NULL) {
                frmDestroy((Frame *) w->dmg);
            }
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            break;
        case 3:
            if (w->model != NULL) {
                w->model->dl = eneDlEnvColorLit(obj, w->envA, 0xFF, 0, 0, 1);
            }
            if (w->part != NULL) {
                w->part->dl = eneDlEnvColorLit(obj, w->envA, 0xFF, 0, 0, 1);
            }
            if (w->fx != NULL) {
                w->fx->dl = gGfxDlHead;
                gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
                gSPEndDisplayList(gGfxDlHead++);
            }
            break;
        case 0:
            eneStepHookPre(obj);
            switch (w->step) {
                case 0:
                    umaStepStand(obj);
                    break;
                case 1:
                    umaStepWait(obj);
                    break;
                case 2:
                    umaStepCharge(obj);
                    break;
                case 3:
                    umaStepShoot(obj);
                    break;
                case 4:
                    umaStepDamage(obj);
                    break;
                default:
                    PRINTF("uma step error!\n");
                    break;
            }
            if (w->dead != 0) {
                eneMakeEx(NULL, ENE_UMA_HANI_EFF, w->zone, w->x, w->y, w->z, 2,
                          (u32 (*)(void *, void *, void *)) eneFaceWay(obj));
                eneMakeEx(NULL, ENE_UMA_NORU, w->zone, w->x, w->y, w->z, 2,
                          (u32 (*)(void *, void *, void *)) (u32) w->type);
                return -1;
            }
            eneFall(obj, 0.4f);
            shimEneMove(obj, 10.0f, 8.0f, w->z, 4.0f);
            if (w->hit & 1) {
                w->vel[1] = 0.0f;
            }
            if (w->step != 2 && w->step != 4) {
                eneTurnAlongCourse(obj, 10.0f);
            }
            if (w->step < 5 && acollAtkWasHit(w->atk) == 1) {
                by = (Obj *) w->atk->hitPower;
                if (by != NULL) {
                    tskSendMsg(by, 7, obj);
                }
            }
            if (w->step != 4 && acollDmgWasHit(w->dmg) == 1) {
                switch (w->dmg->hitKind) {
                    case 4:
                    case 5:
                    case 6:
                        shimSdAtModel((Frame *) w->model, 0x220);
                        break;
                    case 18:
                    case 19:
                    case 20:
                        shimSdAtModel((Frame *) w->model, 0x22D);
                        break;
                    case 1:
                    case 23:
                    default:
                        shimSdAtModel((Frame *) w->model, 0x22C);
                        break;
                }
                file = shimAtkKindCheck(w->dmg);
                PRINTF("uma : DAMAGE POINT [%d]\n", file);
                for (; file != 0; file--) {
                    w->life--;
                    if (w->life == 0) {
                        break;
                    }
                }
                umaStepSet(obj, 4);
                w->sub = 0;
            }
            if (w->y < -600.0f) {
                PRINTF("uma: 落ちすぎ\n");
                return -1;
            }
            eneSetModelOnCourse(obj, (Frame *) w->model);
            (*umaModelFollow)(w->model, w->part);
            (*umaModelFollow)(w->model, w->fx);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

void umaStepStand(Obj *obj) {
    UmaWork *w = UMA(obj);
    s32 pad[17];
    f32 step = 0.5f;

    if (w->timer == 0) {
        umaAnimSet(obj, 0, 0);
        umaHaniAnimSet(obj, 0, 0);
        umaAtkSet(obj, 0);
    }
    w->timer++;
    eneSetModelOnCourse(obj, (Frame *) w->fx);
    w->fx->pos[0] += joyGetStickX(1, 0) / 24;
    w->fx->pos[1] += joyGetStickY(1, 0) / 24;
    eneAnimLoop(obj, w->model, ANIM_STEP(step));
    eneAnimLoop(obj, w->part, ANIM_STEP(step));
    eneAnimLoop(obj, w->fx, ANIM_STEP(step));
}

void umaStepWait(Obj *obj) {
    UmaWork *w = UMA(obj);

    if (w->timer == 0) {
        umaAnimSet(obj, 0, 0);
        umaHaniAnimSet(obj, 0, 0);
        umaAtkSet(obj, 0);
        if (eneNearestPlWay(obj) == 4) {
            w->vel[0] = -1.0f;
        } else {
            w->vel[0] = 1.0f;
        }
    }
    w->timer++;
    if (w->hit & 6) {
        w->vel[0] *= -1.0f;
    }
    switch (w->type) {
        case 0:
            if (eneNearestPlInX(obj, 25.0f)) {
                umaStepSet(obj, 2);
            }
            break;
        case 1:
            if (eneNearestPlInX(obj, 50.0f)) {
                umaStepSet(obj, 3);
                w->baseX = w->x;
            }
            break;
        case 2:
            if (eneNearestPlInX(obj, 50.0f)) {
                umaStepSet(obj, 3);
                w->baseX = w->x;
            }
            break;
        default:
            PRINTF("uma type error!\n");
            break;
    }
    umaRunAnim(obj, w->vel[0], 0);
    w->fx->frame = w->model->frame;
}

void umaStepCharge(Obj *obj) {
    UmaWork *w = UMA(obj);

    if (w->timer == 0) {
        umaAtkSet(obj, 1);
        umaAnimSet(obj, 1, 0);
        umaHaniAnimSet(obj, 1, 0);
        w->sub = 0;
        if (eneNearestPlWay(obj) == 4) {
            w->rot[1] = eneCourseAngle(obj, 4);
            w->vel[0] = -0.25f;
        } else {
            w->rot[1] = eneCourseAngle(obj, 1);
            w->vel[0] = 0.25f;
        }
    }
    w->timer++;
    switch (w->sub) {
        case 0:
            ((AniAtkFrame *) w->atk->model)->time = w->model->frame;
            if (umaRear(obj, w->fx) == 1) {
                umaAnimSet(obj, 0, 0);
                umaHaniAnimSet(obj, 0, 0);
                umaAtkSet(obj, 0);
                w->sub = 1;
                w->vel[0] = 0.0f;
            }
            break;
        case 1:
            if (eneFaceWay(obj) == 4) {
                w->vel[0] += 0.1;
            } else {
                w->vel[0] += -0.1;
            }
            if (ABS(w->vel[0]) > 1.8f) {
                w->sub = 2;
            }
            break;
        case 2:
            if (eneFaceWay(obj) == 4) {
                w->vel[0] += -0.1;
            } else {
                w->vel[0] += 0.1;
            }
            if (ABS(w->vel[0]) < 0.1 || !eneNearestPlInX(obj, 60.0f)) {
                umaStepSet(obj, 1);
            }
            break;
    }
    if (w->sub != 0) {
        w->z = 0.0f;
        umaRunAnim(obj, w->vel[0], 1);
        w->fx->frame = w->model->frame;
    }
}

void umaStepShoot(Obj *obj) {
    UmaWork *w = UMA(obj);

    if (w->timer == 0) {
        umaAtkSet(obj, 0);
        umaAnimSet(obj, 0, 0);
        umaHaniAnimSet(obj, 0, 0);
        w->sub = 0;
        if (eneNearestPlWay(obj) == 4) {
            w->vel[0] = -0.25f;
        } else {
            w->vel[0] = 0.25f;
        }
    }
    w->timer++;
    if (w->hit & 6) {
        w->vel[0] *= -1.0f;
    }
    if (w->timer % 60 <= 0) {
        w->sub = 1;
        w->vel[0] = 0.0f;
    }
    if (w->sub == 0) {
        (*umaSwing)(obj, w->baseX - 10.0f, w->baseX + 10.0f, 0.1f);
        umaRunAnim(obj, w->vel[0], 0);
        w->fx->frame = w->model->frame;
    } else if (w->sub == 1) {
        if (umaTurnToPlayer(obj, 10.0f) == 1) {
            umaAtkSet(obj, 0);
            w->model->anim = umaBodyAnims[w->type * 4];
            w->model->frame = 0.0f;
            w->part->anim = umaPartAnims[w->type * 4 + 1];
            w->part->frame = 0.0f;
            umaHaniAnimSet(obj, 0, 0);
            w->sub = 2;
            shimSdAtModel((Frame *) w->model, 0x22F);
        }
    } else if (w->sub == 2) {
        if (umaFire(obj, w->fx) == 1) {
            w->timer = 0;
            w->sub = 0;
        }
    }
}

/* The compare's 1.0 is "(f64) 1.0f": the game keeps it in f12 across the call for the step's
 * "* 1.0f" (a plain 1.0 is a constant of its own; an f64 local rate shares it too but costs
 * the frame two words). */
void umaStepDamage(Obj *obj) {
    UmaWork *w = UMA(obj);
    f32 step = 0.0f;

    if (w->timer == 0) {
        umaAnimSet(obj, 2, 0);
        umaHaniAnimSet(obj, 0, 0);
        umaAtkSet(obj, 2);
        w->sub = 0;
    }
    if (!(w->timer & 1)) {
        w->alpha = 0;
    } else {
        w->alpha = 0xFF;
    }
    w->timer++;
    if (w->sub == 0) {
        if (eneNearestPlWay(obj) == 4) {
            w->sub = 1;
            w->vel[0] = 4.0f;
        } else {
            w->sub = 2;
            w->vel[0] = -4.0f;
        }
    } else if (w->sub == 1) {
        w->vel[0] += -0.2;
    } else if (w->sub == 2) {
        w->vel[0] += 0.2;
    } else {
        PRINTF("back_flg error!!\n");
    }
    if (ABS(w->vel[0]) < (f64) 1.0f) {
        w->alpha = 0xFF;
        umaStepSet(obj, 1);
    }
    if (w->hit & 2) {
        w->x -= w->vel[0];
        w->z -= w->vel[2];
    }
    if (w->life <= 0) {
        w->alpha = 0xFF;
        w->sub = 0;
        w->timer = 0;
        w->dead = 1;
    }
    eneAnimLoop(obj, w->part, ANIM_STEP(step));
    eneAnimLoop(obj, w->model, ANIM_STEP(step));
}

s32 umaNoruProc(Obj *obj, s16 msg, void *arg) {
    NoruWork *w = NORU(obj);
    s32 file;

    switch (msg) {
        case 2:
            obj->name = "noru";
            w->type = eneSetup(obj, arg);
            if (w->type == 0) {
                file = 0x2F;
            } else if (w->type == 1) {
                file = 0x30;
            } else {
                file = 0x31;
            }
            if (floadIsSetLoaded(file) == 0) {
                PRINTF("uma : packetLoad FAULT!!!\n");
                return -1;
            }
            w->model = eneAnimeModelMake(obj, umaBodyAnims[w->type * 4], NULL, 0);
            if (w->model == NULL) {
                return -1;
            }
            floadGetSetRam((void **) w->model->segs, file);
            w->part = eneAnimeModelMake(obj, umaPartAnims[w->type * 4], NULL, 0);
            if (w->part == NULL) {
                return -1;
            }
            floadGetSetRam((void **) w->part->segs, file);
            w->model->flags |= 0x20;
            w->part->flags |= 0x20;
            umaAnimSet(obj, 0, 0);
            w->atkMode = 2;
            umaStepSet(obj, 5);
            w->life = 6;
            w->ctrl = 0;
            w->owner = 5;
            w->vel[0] = 0.0f;
            w->rot[1] = eneCourseAngle(obj, eneNearestPlWay(obj));
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy((Frame *) w->model);
            }
            if (w->part != NULL) {
                frmDestroy((Frame *) w->part);
            }
            if (w->atk != NULL) {
                if (w->atkMode == 0) {
                    frmDestroy((Frame *) w->atk);
                } else if (w->atkMode == 1) {
                    aniAtkDelete(w->atk);
                }
            }
            if (w->dmg != NULL) {
                frmDestroy((Frame *) w->dmg);
            }
            if (w->mark != NULL) {
                tskDestroy(w->mark);
            }
            break;
        case 3:
            if (w->model != NULL) {
                w->model->dl = eneDlEnvColorLit(obj, w->envA, 0xFF, 0, 0, 1);
            }
            if (w->part != NULL) {
                w->part->dl = eneDlEnvColorLit(obj, w->envA, 0xFF, 0, 0, 1);
            }
            break;
        case 0:
            eneStepHookPre(obj);
            switch (w->step) {
                case 5:
                    umaNoruStepWait(obj);
                    break;
                case 6:
                    umaNoruStepRide(obj);
                    break;
                case 7:
                    umaNoruStepHurt(obj);
                    break;
                default:
                    PRINTF("uma_noru step error!\n");
                    break;
            }
            eneFall(obj, 0.4f);
            (*umaNoruMove)(obj, 10.0f, 8.0f, w->z);
            if (w->hit & 2) {
                w->x -= w->vel[0];
                w->z -= w->vel[2];
            }
            if (w->hit & 1) {
                w->vel[1] = 0.0f;
            }
            eneSetModelOnCourse(obj, (Frame *) w->model);
            (*umaModelFollow)(w->model, w->part);
            if (w->ctrl != 0 && w->owner != 5) {
                w->ctrlRet = ((s32 (*)(s32, s32)) shimEneChangeCtrl)(w->ctrl, w->owner);
                switch (w->ctrlRet) {
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                        break;
                    case 6:
                        if (gGameWork.players[w->owner] != NULL &&
                            shimPlGetHitKind((Player *) gGameWork.players[w->owner]) != 0) {
                            shimPlSetHitKind((Player *) gGameWork.players[w->owner], 0);
                            umaNoruKeepSide(gGameWork.players[w->owner], &w->save);
                        }
                        shimSdAtModel((Frame *) w->model, 0x11B);
                        itexPuffDark(w->model->pos[0], w->model->pos[1] + 15.0f, w->model->pos[2], 0.3f);
                        return -1;
                    case 7:
                        if (gGameWork.players[w->owner] != NULL &&
                            shimPlGetHitKind((Player *) gGameWork.players[w->owner]) != 0) {
                            shimPlSetHitKind((Player *) gGameWork.players[w->owner], 0);
                            umaNoruKeepSide(gGameWork.players[w->owner], &w->save);
                        }
                        w->ctrl = 0;
                        PRINTF("uma_noru wait_step CHANGE!\n");
                        umaStepSet(obj, 5);
                        break;
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

void umaNoruStepWait(Obj *obj) {
    NoruWork *w = NORU(obj);
    f32 step = 0.075f;
    Obj *pl;
    s32 state;

    if (w->timer == 0) {
        w->owner = 5;
        if (w->atk != NULL) {
            w->atk = (ACollFrame *) frmDestroy((Frame *) w->atk);
        }
        w->atk = ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)(
            (Frame *) ene_atk_root_frame, (Frame *) w->model, 100, 0, 0, 0);
        w->atkMode = 0;
        w->atk->kind = 100;
        umaAnimSet(obj, 3, 0);
        if (w->dmg != NULL) {
            frmDestroy((Frame *) w->dmg);
        }
        w->mark = eneMakeEx(NULL, ENE_NORU_MARK, w->zone, w->x, w->y + 15.0f, w->z, 0, NULL);
        w->ctrl = 0;
        w->hurt = 0;
        w->envA = 0;
        w->vel[0] = 0.0f;
    }
    w->timer++;
    eneAnimLoop(obj, w->model, ANIM_STEP(step));
    eneAnimLoop(obj, w->part, ANIM_STEP(step));
    if (acollAtkWasHit(w->atk) == 1) {
        pl = (Obj *) w->atk->hitPower;
        if (pl != NULL) {
            state = shimPlGetState((Player *) pl);
            if (state == 15 || state == 22) {
                w->owner = plGetIndex(pl);
                if (shimPlGetHitKind((Player *) pl) == 0) {
                    shimPlSetHitKind((Player *) pl, (s32) obj);
                    umaStepSet(obj, 6);
                    w->mark = tskDestroy(w->mark);
                    if (w->dmg != NULL) {
                        frmDestroy((Frame *) w->dmg);
                    }
                    w->dmg = ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)(
                        (Frame *) pl_dmg_root_frame, (Frame *) w->model, 90, 180, 0, 0, 0);
                    umaNoruMountVoice(obj, pl);
                    umaNoruKeepSide(gGameWork.players[w->owner], &w->save);
                    shimSdAtModel((Frame *) w->model, 0x2C2);
                }
            }
        }
    }
}

/* The ridden horse's step (6). The speed clamp compares with 1 and assigns 1.0f (both 1.0f
 * would share the constant); "pl" (the rider's Player) is set before v's stores. */
void umaNoruStepRide(Obj *obj) {
    NoruWork *w = NORU(obj);
    f32 spd;
    s32 kind;
    Vec3f v;
    Player *pl;

    if (w->timer == 0) {
        umaAtkSet(obj, 0);
        umaAnimSet(obj, 3, 0);
        w->phase = 0;
        w->anim = 0;
        if (gGameWork.players[w->owner] != NULL) {
            plSetAction(gGameWork.players[w->owner], 0xCC);
        } else {
            PRINTF("plExtPaChange fault!!!\n");
        }
        w->ctrl = 0x1010;
    }
    w->timer++;
    w->vel[0] = umaNoruStickSpeed(w->owner);
    w->rot[1] = plGetModelYaw(gGameWork.players[w->owner]);
    spd = ABS(w->vel[0]) * 0.5f;
    if (ABS(spd) < 0.1f) {
        spd = 0.3f;
    }
    if (ABS(spd) > 1) { spd = 1.0f; } /* on one line IDO makes 1.0f first */
    if (w->phase == 0) {
        if (w->ctrlRet == 1) {
            w->ctrl = 0x1010;
        } else if (w->ctrlRet == 2) {
            w->ctrl = 0x1011;
        }
        if (ABS(w->vel[0]) == 0.0 && w->anim != 1) {
            w->anim = 1;
            umaAnimSet(obj, 3, 0);
        } else if (ABS(w->vel[0]) != 0.0 && w->anim != 2) {
            w->anim = 2;
            umaAnimSet(obj, 0, 0);
            w->sound = 0;
        }
        if (w->anim == 1) {
            spd = 0.075f;
            eneAnimLoop(obj, w->model, ANIM_STEP(spd));
            eneAnimLoop(obj, w->part, ANIM_STEP(spd));
        } else if (w->anim == 2) {
            eneAnimLoop(obj, w->part, ANIM_STEP(spd));
            eneAnimLoop(obj, w->model, ANIM_STEP(spd));
            if (ABS(w->vel[0]) > 0.2) {
                umaPuff(obj, w->vel[0]);
            }
            umaHoofSound(obj);
        }
        if (w->hurt != 0) {
            w->hurt--;
            w->envA -= 0x10;
        } else if (acollDmgWasHit(w->dmg) == 1) {
            kind = w->dmg->hitKind;
            if (shimIsKind120To126(kind)) {
                w->phase = 0;
                w->anim = 0;
                w->envA = 0xF0;
                w->life--;
                w->hurt = 0xF;
                PRINTF("uma life is %d\n", w->life);
                if (w->life <= 0) {
                    w->ctrl = 0x1017;
                }
            }
            if (plIsInPlay(w->owner) && kind < 0x75) {
                plExtDamage(gGameWork.players[w->owner], (s32) obj, kind);
            }
        }
    }
    pl = PL(gGameWork.players[w->owner]);
    v.x = w->vel[0];
    v.y = w->vel[0];
    v.z = w->vel[2];
    if (plMoveTiedXZ(pl, &v)) {
        w->vel[0] = w->vel[2] = 0.0f;
    }
    if (w->ctrlRet == 3 && w->phase == 0) {
        PRINTF("uma : hippari!!!!!\n");
        w->phase = 1;
        if (w->type == 0) {
            w->vel[0] = 0.0f;
            umaAnimSet(obj, 1, 0);
            umaAtkSet(obj, 0);
            w->atk->kind = 0x1F;
            w->ctrl = 0x1012;
            w->neighed = 0;
        } else {
            w->model->anim = umaBodyAnims[w->type * 4];
            w->part->anim = umaPartAnims[w->type * 4 + 1];
            w->part->frame = 0.0f;
            w->ctrl = 0x1013;
            shimSdAtModel((Frame *) w->model, 0x22F);
        }
    } else if (w->phase == 1) {
        switch (w->type) {
            case 0:
                w->vel[0] = 0.0f;
                if (umaRear(obj, NULL) == 1) {
                    w->anim = 0;
                    w->phase = 0;
                    w->timer = 0;
                    w->ctrl = 0x1014;
                }
                if (w->model->frame > 23.5) {
                    w->atk->radius = 230;
                } else {
                    w->atk->radius = 80;
                }
                break;
            case 1:
                eneAnimLoop(obj, w->model, ANIM_STEP(spd));
                if (ABS(w->vel[0]) > 0.2) {
                    umaPuff(obj, w->vel[0]);
                }
                umaHoofSound(obj);
                if (umaFire(obj, NULL) == 1) {
                    w->anim = 0;
                    w->phase = 0;
                    w->ctrl = 0x1014;
                }
                break;
            case 2:
                eneAnimLoop(obj, w->model, ANIM_STEP(spd));
                if (ABS(w->vel[0]) > 0.2) {
                    umaPuff(obj, w->vel[0]);
                }
                umaHoofSound(obj);
                if (umaFire(obj, NULL) == 1) {
                    w->anim = 0;
                    w->phase = 0;
                    w->ctrl = 0x1014;
                }
                break;
            default:
                PRINTF("uma type error!\n");
                break;
        }
    }
}

f32 umaNoruStickSpeed(s32 port) {
    s32 x = plMoveStickX(port, 0);

    if (fabs(x) < 20.0) {
        x = 0;
    }
    return x * 0.03f;
}

void umaNoruStepHurt(Obj *obj) {
    NoruWork *w = NORU(obj);

    w->life--;
    w->hurt = 30;
    if (w->life <= 0) {
        w->ctrl = 0x1017;
    }
    PRINTF("uma life is %d\n", w->life);
    umaStepSet(obj, 6);
}

void umaNoruMountVoice(Obj *obj, Obj *pl) {
    switch (plGetChara(pl)) {
        case 0:
            shimSdAtModel((Frame *) NORU(obj)->model, 0x393);
            break;
        case 1:
            shimSdAtModel((Frame *) NORU(obj)->model, 0x372);
            break;
        case 2:
            shimSdAtModel((Frame *) NORU(obj)->model, 0x3B2);
            break;
        case 3:
            shimSdAtModel((Frame *) NORU(obj)->model, 0x3CF);
            break;
    }
}

void umaModelFollow(AnimeFrame *src, AnimeFrame *dst) {
    dst->pos[0] = src->pos[0];
    dst->pos[1] = src->pos[1];
    dst->pos[2] = src->pos[2];
    dst->rot[0] = src->rot[0];
    dst->rot[1] = src->rot[1];
    dst->rot[2] = src->rot[2];
}

void umaPuff(Obj *obj, f32 speed) {
    UmaWork *w = UMA(obj);
    s32 pad;
    FxWork fx;
    f32 s;

    if (w->model->frame > 11.5f && w->puff == 0) {
        s = speed / 150.0f;
        if (ABS(s) > 0.006) {
            s = 0.006f;
        }
        effectPuffDefaults(&fx);
        fx.pos[0] = w->model->pos[0];
        fx.pos[1] = w->model->pos[1];
        fx.pos[2] = w->model->pos[2];
        fx.scale = 0.01f;
        fx.grow = ANIM_STEP(s);
        fx.b = fx.g = fx.r = 200;
        fx.kind = 2;
        fx.vel[1] = 0.1f;
        effectPuffMake(fx);
        w->puff = 1;
    }
    if (w->model->frame < 11.5f) {
        w->puff = 0;
    }
}

void umaHoofSound(Obj *obj) {
    UmaWork *w = UMA(obj);
    s32 pad;

    if (w->model->frame > 14.0f && w->sound == 0) {
        shimSdAtModel((Frame *) w->model, 0x277);
        w->sound = 1;
    } else if (w->model->frame < 2.0f && w->sound == 1) {
        shimSdAtModel((Frame *) w->model, 0x278);
        w->sound = 2;
    } else if (w->model->frame > 5.0f && w->sound == 2) {
        shimSdAtModel((Frame *) w->model, 0x279);
        w->sound = 0;
    }
}

void umaRunAnim(Obj *obj, f32 speed, s32 back) {
    UmaWork *w = UMA(obj);
    s32 pad;
    f32 dir;
    f32 spd;

    if (back == 1) {
        dir = -1.0f;
    } else {
        dir = 1.0f;
    }
    spd = ABS(speed) * 0.5f;
    if (ABS(spd) < 0.1f) {
        spd = 0.3f;
    }
    if (ABS(spd) > 1) {
        spd = 1.0f;
    }
    eneAnimLoop(obj, w->part, ANIM_STEP(spd) * dir);
    eneAnimLoop(obj, w->model, ANIM_STEP(spd) * dir);
    if (ABS(speed) > 0.2) {
        umaPuff(obj, speed);
    }
    if (w->owner != 4) {
        umaHoofSound(obj);
    }
}

void umaNoruKeepSide(Obj *pl, f32 *save) {
    if (PL(pl)->ride != NULL) {
        *save = PL(pl)->side;
    } else {
        PL(pl)->side = *save;
    }
}

void umaAnimSet(Obj *obj, s32 anim, s32 frame) {
    UmaWork *w = UMA(obj);

    w->model->anim = umaBodyAnims[anim + w->type * 4];
    w->model->frame = frame;
    w->part->anim = umaPartAnims[anim + w->type * 4];
    w->part->frame = frame;
}

void umaHaniAnimSet(Obj *obj, s32 anim, s32 frame) {
    UmaWork *w = UMA(obj);

    w->fx->anim = umaHaniAnims[anim];
    w->fx->frame = frame;
}

void umaAtkSet(Obj *obj, s32 mode) {
    UmaWork *w = UMA(obj);

    if (w->atk != NULL) {
        if (w->atkMode == 0) {
            w->atk = (ACollFrame *) frmDestroy((Frame *) w->atk);
        } else if (w->atkMode == 1) {
            w->atk = (ACollFrame *) aniAtkDelete(w->atk);
        }
    }
    switch (mode) {
        case 0:
            w->atk = ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)(
                (Frame *) *umaAtkRoots[w->owner], (Frame *) w->model, 60, 0, 0, 0);
            w->atkMode = 0;
            break;
        case 1:
            w->atk = aniAtkCreate((Frame *) *umaAtkRoots[w->owner], (Frame *) w->model,
                                  (u32) umaAtkChain, floadGetSetFile(0x2F, 0), 0);
            ((AniAtkFrame *) w->atk->model)->time = 0.0f;
            w->atkMode = 1;
            break;
        case 2:
            w->atk = NULL;
            w->atkMode = 2;
            break;
        default:
            PRINTF("uma_Atk_set: case error!\n");
            break;
    }
    if (w->owner == 4 && w->atk != NULL) {
        w->atk->kind = 120;
    }
}

void umaTamaAtkSet(Obj *obj) {
    TAMA(obj)->atk = ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)(
        (Frame *) *umaAtkRoots[TAMA(obj)->owner], (Frame *) TAMA(obj)->model, 6, 6, 0, 0);
}

s32 umaRear(Obj *obj, AnimeFrame *fx) {
    UmaWork *w = UMA(obj);
    f32 frame;
    f32 spd;

    if (w->model->frame < 22.4f || w->model->frame >= 24.0f) {
        spd = 1.15f;
    } else if (w->model->frame < 23.2f) {
        spd = 0.0175f;
    } else if (w->model->frame < 23.8f) {
        spd = 0.195f;
        if (w->neighed == 0 && fx == NULL) {
            shimSdAtModel((Frame *) w->model, 0x2A8);
            PRINTF(" inanaki\n");
            w->neighed = 1;
        }
    } else {
        spd = 0.0005f;
        if (w->model->frame + ANIM_STEP(spd) > 24.0) {
            frame = 24.0 - ANIM_STEP(spd);
            w->part->frame = frame;
            w->model->frame = frame;
            if (fx != NULL) {
                fx->frame = frame;
            }
        }
    }
    if (fx != NULL) {
        eneAnimPlayOnce(obj, fx, ANIM_STEP(spd));
    }
    eneAnimPlayOnce(obj, w->part, ANIM_STEP(spd));
    if (eneAnimPlayOnce(obj, w->model, ANIM_STEP(spd)) == 1) {
        return 1;
    }
    return 0;
}

s32 umaFire(Obj *obj, AnimeFrame *fx) {
    UmaWork *w = UMA(obj);
    f32 spd = 0.15f;
    s16 way;
    s32 pad;

    if (w->part->frame == 0.0f) {
        way = eneFaceWay(obj);
        eneMakeEx(NULL, ENE_UMA_TAMA, w->zone, w->x, w->y + 10.0f, w->z, 2,
                  (u32 (*)(void *, void *, void *)) ((w->owner & 0xFFFF) | ((way << 16) & 0xFFFF0000)));
    }
    if (fx != NULL) {
        eneAnimPlayOnce(obj, fx, ANIM_STEP(spd * 4.0f));
    }
    if (eneAnimPlayOnce(obj, w->part, ANIM_STEP(spd * 2)) == 1) {
        return 1;
    }
    return 0;
}

void umaStepSet(Obj *obj, s8 step) {
    UMA(obj)->step = step;
    UMA(obj)->timer = 0;
}

s32 umaSwing(Obj *obj, f32 min, f32 max, f32 accel) {
    UmaWork *w = UMA(obj);

    if (w->vel[0] > 2.0) {
        w->vel[0] = 2.0f;
    } else if (w->vel[0] < -2.0) {
        w->vel[0] = -2.0f;
    }
    if (w->x < min) {
        w->vel[0] += accel;
    } else if (max < w->x) {
        w->vel[0] -= accel;
    } else {
        if (w->vel[0] > 0.0f) {
            w->vel[0] += accel;
        } else if (w->vel[0] < 0.0f) {
            w->vel[0] -= accel;
        } else {
            w->vel[0] = -accel;
        }
        return 0;
    }
    return 1;
}

s32 umaTurnToPlayer(Obj *obj, f32 div) {
    UmaWork *w = UMA(obj);
    f32 target = eneCourseAngle(obj, eneNearestPlWay(obj));
    f32 d = target - w->rot[1];
    f32 turn;

    if (d > 3.1415927f) {
        w->rot[1] += 6.2831855f;
        d = target - w->rot[1];
    } else if (d < -3.1415927f) {
        w->rot[1] -= 6.2831855f;
        d = target - w->rot[1];
    }
    turn = 3.1415927f / div;
    if (((w->rot[1] < target) ? d : -d) < turn) {
        w->rot[1] = target;
        return 1;
    } else {
        if (target < w->rot[1]) {
            w->rot[1] -= turn;
        } else {
            w->rot[1] += turn;
        }
        return 0;
    }
}

/* An empty function nothing calls: splat folded it into the one above (its "jr ra; nop" at
 * 0x080031C8). */
void umaDummy(void) {
}

s32 umaHaniEffProc(Obj *obj, s16 msg, void *arg) {
    HaniWork *w = HANI(obj);
    s32 pad;
    s32 way;

    switch (msg) {
        case 2:
            obj->name = "hani_eff";
            way = eneSetup(obj, arg);
            w->model = eneAnimeModelAlpha(obj, (u32) umaHaniAnim0, 0xFF);
            if (w->model == NULL) {
                return -1;
            }
            if (way == 4) {
                w->vel[0] = 2.0f;
            } else {
                w->vel[0] = -2.0f;
            }
            w->baseY = w->y;
            w->vel[1] = 6.0f;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy((Frame *) w->model);
            }
            break;
        case 3:
            if (w->model != NULL) {
                w->model->dl = gGfxDlHead;
                gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 0xFF);
                gSPEndDisplayList(gGfxDlHead++);
            }
            break;
        case 0:
            eneStepHookPre(obj);
            w->vel[1] -= 0.8;
            if (w->hit & 1) {
                gGameWork.kills++;
                eneSetSd2(obj, 0x229, 0x22A, 0);
                effectBurst(w->model->pos[0], w->model->pos[1] + 5.0f, w->model->pos[2]);
                return -1;
            }
            shimEneMove(obj, 10.0f, 8.0f, w->z, 4.0f);
            eneSetModelOnCourse(obj, (Frame *) w->model);
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 umaTamaProc(Obj *obj, s16 msg, void *arg) {
    TamaWork *w = TAMA(obj);
    f32 spd;
    s16 way;
    u32 c;
    ACollInfo v;
    Obj *by;

    switch (msg) {
        case 2:
            obj->name = "uma_tama";
            c = eneSetup(obj, arg);
            w->owner = c & 0xFFFF;
            way = (c >> 16) & 0xFFFF;
            w->model = enePolModelMake(obj, (Gfx *) umaTamaShotDl, eneDlEnvPrimColor(obj, 0, 0, 0, 0, 0, 0, 0xFF), 1);
            if (w->model == NULL) {
                return -1;
            }
            umaTamaAtkSet(obj);
            if (w->owner == 4) {
                spd = 1.8f;
                w->atk->kind = 0x7B;
            } else {
                spd = 5.399999619f;
                w->atk->kind = 0x1D;
            }
            if (w->atk == NULL) {
                return -1;
            }
            w->model->flags |= 7;
            w->model->scale[2] = 0.4f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            if (way == 4) {
                w->vel[0] = -spd;
            } else {
                w->vel[0] = spd;
            }
            w->vel[1] = 0.0f;
            ((f32 *) &v)[0] = w->vel[0];
            ((f32 *) &v)[1] = w->vel[1];
            ((f32 *) &v)[2] = w->vel[2];
            w->atk->info = v;
            w->startX = w->x;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy((Frame *) w->model);
            }
            if (w->atk != NULL) {
                frmDestroy((Frame *) w->atk);
            }
            break;
        case 3:
            w->model->dl = eneDlEnvPrimColor(obj, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0, 0xFF);
            break;
        case 0:
            eneStepHookPre(obj);
            shimEneMove(obj, 5.0f, 4.0f, w->z, 4.0f);
            eneSetModelOnCourse(obj, (Frame *) w->model);
            if (w->hit & 1) {
                itexPuffDark(w->model->pos[0], w->model->pos[1], w->model->pos[2], 0.08f);
                return -1;
            }
            if (((w->startX < w->x) ? w->x - w->startX : -(w->x - w->startX)) > 200.0f) {
                return -1;
            }
            if (acollAtkWasHit(w->atk) == 1) {
                if (w->owner == 4) {
                    by = (Obj *) w->atk->hitPower;
                    if (by != NULL) {
                        tskSendMsg(by, 7, obj);
                    }
                }
                return -1;
            }
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The rideable horse's own shimEneMove: lands it on three rays (umaNoruLand) and
 * checks walls at half the height (umaNoruWallCheck). */
void umaNoruMove(Obj *obj, f32 h, f32 r, f32 a) {
    EneObj *e = ENE(obj);

    eneMove(e);
    e->hit &= ~7;
    if ((*umaNoruLand)(e, h, r)) {
        e->hit |= 1;
    }
    if (umaNoruWallCheck(e, h / 2, r, a) < 0) {
        e->hit |= 2;
    }
}

/* eneLand on three rays, at x and r either side along the course: 1 on the ground, -1 on a
 * ground of another course (the middle ray: it moves onto that course), 0 in the air. */
s32 umaNoruLand(EneObj *e, f32 h, f32 r) {
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

/* eneWallCheckEx without the push and with a level ray: -1 a wall (dx, dz) ahead the way the
 * enemy moves, 0 nothing. */
s32 umaNoruWallCheck(EneObj *e, f32 h, f32 dx, f32 dz) {
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
