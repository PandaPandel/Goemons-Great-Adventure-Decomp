#include "tlb.h"

/* File 97: "kozure" (an enemy that walks the course with two it makes and drives: kind 0xED,
 * "koz_m", file 96's mouja, ahead of it, and kind 0xEE, a "sensya", where it stands), "sensya" (a
 * tank a player can ride and fire) and "sen_tama" (its shell). Matched whole, 2026-10-04. */

#define ABS(x) ((x) > 0.0f ? (x) : -(x))
/* How far a is from b, either way. */
#define DIST(a, b) ((b) < (a) ? (a) - (b) : -((a) - (b)))

/* An animation step: "* 2.0 * 1.0f" (add.d, then a mul.d by 1.0), as in files 78 and 96. */
#define ANIM_STEP(x) ((x) * 2.0 * 1.0f)

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 sensyaKozureWalkAnim[]; /* the kozure's walk */
extern u8 sensyaAnim0[];
extern u8 sensyaBodyAnim[];
u32 sensyaKozureAnims[] = { 0x03000498, (u32) sensyaKozureWalkAnim, 0x0300087C }; /* the kozure's animations */
u32 sensyaAnims[] = { (u32) sensyaAnim0, (u32) sensyaBodyAnim, 0x0300025C, 0x00000000, 0x00000000 }; /* the sensya's animations */

/* The kozure's work, at its task + 0x20: an enemy's (EneObj) up to 0x3C. */
typedef struct Kozure {
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
    /* 0x3C */ Obj *sensya; /* kind 0xEE (sensyaProc, "sensya", this file) */
    /* 0x40 */ Obj *lead; /* kind 0xED ("koz_m", file 96's mouja), made ahead of it */
    /* 0x44 */ u32 timer; /* steps in this step */
    /* 0x48 */ f32 animSpeed;
    /* 0x4C */ f32 homeX; /* where it was placed: 300 from it, it puffs away */
    /* 0x50 */ u16 blocked; /* steps it ran into a wall */
    /* 0x52 */ u8 way; /* eneNearestPlWay's */
    /* 0x53 */ u8 prevStep;
    /* 0x54 */ u8 step;
} Kozure;

#define KOZURE(obj) ((Kozure *) FX(obj))

/* The kozure's lead (kind 0xED, "koz_m", file 96's Mouja and its names), the fields the kozure
 * sets. */
typedef struct KozLead {
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
    /* 0x3C */ s32 timer;
    /* 0x40 */ f32 animSpeed;
    /* 0x44 */ f32 homeX;
    /* 0x48 */ f32 range;
    /* 0x4C */ u8 type;
    /* 0x4D */ u8 step;
    /* 0x4E */ u8 throwing;
    /* 0x4F */ u8 side;
    /* 0x50 */ u8 axeType;
    /* 0x51 */ u8 on; /* cleared when the kozure is hit */
} KozLead;

#define KOZLEAD(obj) ((KozLead *) FX(obj))

/* The kozure's sensya (kind 0xEE, this file's Sensya and its names), the fields the kozure
 * sets. */
typedef struct KozM {
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
    /* 0x3C */ ACollFrame *grab;
    /* 0x40 */ AnimeFrame *turret;
    /* 0x44 */ Obj *mark;
    /* 0x48 */ RideFrame *ride;
    /* 0x4C */ RideFrame *ride2;
    /* 0x50 */ MPolFrame *shadow;
    /* 0x54 */ f32 animSpeed;
    /* 0x58 */ f32 rotY;
    /* 0x5C */ f32 save;
    /* 0x60 */ f32 facing;
    /* 0x64 */ s32 ctrl;
    /* 0x68 */ s32 timer; /* steps in this step */
    /* 0x6C */ u8 ctrlRet;
    /* 0x6D */ u8 mode;
    /* 0x6E */ u8 step;
    /* 0x6F */ u8 prevStep;
} KozM;

#define KOZM(obj) ((KozM *) FX(obj))

extern u8 sensyaBodyAnim[]; /* the sensya's body */
extern u8 sensyaTurretAnim[]; /* the sensya's turret */
extern u8 sensyaTurretRide[]; /* the turret's ride data */
extern u8 sensyaBodyRide[]; /* the body's ride data */
extern u8 sensyaTamaShellDl[]; /* the shell's display list */

/* The sensya's work, at its task + 0x20: an enemy's (EneObj) up to 0x34. A player who hits its
 * box while it waits (step 3) rides it and drives it. */
typedef struct Sensya {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ AnimeFrame *model; /* the body */
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *atk; /* the rider's attack box (kind 0x1D) */
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ ACollFrame *grab; /* while it waits: a player who hits it rides it */
    /* 0x40 */ AnimeFrame *turret;
    /* 0x44 */ Obj *mark; /* kind 0xE9, made while it waits for a rider */
    /* 0x48 */ RideFrame *ride; /* on the turret */
    /* 0x4C */ RideFrame *ride2; /* on the body */
    /* 0x50 */ MPolFrame *shadow;
    /* 0x54 */ f32 animSpeed;
    /* 0x58 */ f32 aim; /* the turret's heading: the rider's model yaw */
    /* 0x5C */ f32 save; /* the rider's side, kept while it rides (sensyaRiderSide) */
    /* 0x60 */ f32 facing; /* the rider's last target facing */
    /* 0x64 */ s32 ctrl; /* the rider's control (shimEneChangeCtrl) */
    /* 0x68 */ s32 timer; /* steps in this step */
    /* 0x6C */ u8 ctrlRet;
    /* 0x6D */ u8 rider; /* the player riding it (0-3), 4 none */
    /* 0x6E */ u8 step;
    /* 0x6F */ u8 prevStep;
    /* 0x70 */ u8 fired; /* 1 the shell is out, 2 the shot is over */
    /* 0x71 */ u8 rollTimer; /* steps to the next rolling sound while pushed or pulled */
} Sensya;

#define SENSYA(obj) ((Sensya *) FX(obj))

/* What the sensya passes to its shell's eneMakeEx (and eneSetup gives back). */
typedef struct SenTamaArg {
    /* 0x00 */ f32 dir;
    /* 0x04 */ u8 owner; /* the rider (0-3), 4 the enemy's own */
} SenTamaArg;

/* The shell's work ("sen_tama", kind 0xEF), at its task + 0x20: an enemy's up to 0x34. */
typedef struct SenTama {
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
    /* 0x3C */ u8 pad3C[4];
    /* 0x40 */ f32 dir;
    /* 0x44 */ u8 owner; /* 0-3 a rider's, 4 the enemy's */
    /* 0x45 */ u8 blast; /* the blast's radius */
    /* 0x46 */ u8 step; /* 0 flying, 1 bursting */
} SenTama;

#define SENTAMA(obj) ((SenTama *) FX(obj))

void sensyaKozureStepNone(Obj *obj);
void sensyaKozureStepWalk(Obj *obj);
void sensyaKozureStepTurn(Obj *obj);
void sensyaKozureSetAnim(Obj *obj, s32 anim);
void sensyaSetAnim(Obj *obj, s32 anim);
s32 sensyaKozureDriveLead(Obj *obj, f32 vx, f32 speed, u8 step, s32 hit);
s32 sensyaKozureLeadOff(Obj *obj);
s32 sensyaKozureDriveMouja(Obj *obj, f32 vx, f32 speed, f32 rotY, u8 step, s32 hit);
f32 sensyaAnimSpeed(Obj *obj);
void sensyaMoveCheck(Obj *obj, f32 h, f32 r, f32 z);
s32 sensyaStepNone(Obj *obj);
s32 sensyaStepRide(Obj *obj);
s32 sensyaStepFire(Obj *obj);
s32 sensyaStepMove(Obj *obj);
s32 sensyaStepWait(Obj *obj);
s32 sensyaPlControl(Obj *obj);
void sensyaRiderSide(Obj *pl, f32 *save);
f32 sensyaRiderFacing(Obj *pl);
void sensyaTurretOnBody(AnimeFrame *src, AnimeFrame *dst, f32 rotY);
s32 sensyaRideMake(Obj *obj, s32 mode);
s32 sensyaRollInterval(f32 speed);
void sensyaPlaceEnemy(Obj *obj, s32 zone, f32 x, f32 y, f32 z);
void sensyaTamaAtkMake(Obj *obj, s32 owner);
s32 sensyaGroundCheck(EneObj *e, f32 h, f32 r);
s32 sensyaWallCheck(EneObj *e, f32 h, f32 r, f32 z);
void sensyaRiderVoice(Obj *obj, Obj *pl);

/* sndloc.c's SoundLocation2 as this file has its own copy: near taken to its absolute value by
 * a compare (not fabsf), and the volume scaled by 256 / max. The dead print's test is against
 * an int 0: as 0.0f, uopt kept no 0.0f in f0 for the other tests and clamps (measured). */
void sensyaSoundLocation(u16 id, CamView *view, f32 x, f32 y, f32 z, f32 near, f32 far,
                            f32 fov, s32 max) {
    f32 t;
    s16 vol;
    s16 pan;
    f32 dist;
    f32 s;
    Vec3f d;
    Vec3f v;

    near = ABS(near);
    d.x = x - view->eye[0];
    d.y = y - view->eye[1];
    d.z = z - view->eye[2];
    if (far == 0.0f) {
        vol = 0;
    } else {
        dist = sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);
        t = (dist - near) / (far - near);
        if (t > 1.0) {
            t = 1.0f;
        } else if (t < 0.0f) {
            t = 0.0f;
        }
        vol = (1 - t) * 256.0f;
        if (vol <= 0) {
            return;
        }
        vol = (vol << 8) / max;
    }
    v.x = view->eye[2] - view->at[2];
    v.z = view->at[0] - view->eye[0];
    s = sqrtf((v.x * v.x + v.z * v.z) * (d.x * d.x + d.z * d.z));
    if (s == 0) {
        PRINTF("SoundLocation: 0除算しました \n");
    }
    pan = (s16) ((v.x * d.x + v.z * d.z) / s / mathSin(fov) * 64.0f) + 0x40;
    if (pan <= 0) {
        pan = 1;
    } else if (pan >= 0x80) {
        pan = 0x7F;
    }
    sndRequest(id, pan, vol);
}

/* The kozure's handler. The levers (each measured): "hit" reused for the hitter (the game loads
 * it straight into a0, and one word less), a 2-link chain for the step zeros (it spends the
 * game's t-register), the step switch on prevStep read back after its store (the game's andi),
 * and one pad word. Where the two "SET MISS"/"moujaD" prints were is a guess (dead code). */
s32 sensyaKozureProc(Obj *obj, s16 msg, void *arg) {
    Kozure *w = KOZURE(obj);
    f32 x;
    s32 hit;
    s32 pad;
    f32 d;

    switch (msg) {
        case 2:
            obj->name = "kozure";
            eneSetup(obj, arg);
            w->model = eneNewAnime(obj, (u32) sensyaKozureWalkAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->model->flags |= 0x20;
            w->atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, (Frame *) w->model, 35, 230, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x78;
            w->dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, (Frame *) w->model, 40, 100, 200, 0,
                                0);
            if (w->dmg == NULL) {
                return -1;
            }
            if (eneSetOnEarth(obj) == -1) {
                return -1;
            }
            shimEneSetLowFlags(obj, 3);
            w->prevStep = w->step = 0;
            w->blocked = 0;
            w->timer = 0;
            w->homeX = w->x;
            w->way = eneNearestPlWay(obj);
            w->rot[1] = eneCourseAngle(obj, w->way);
            if (w->way == 4) {
                x = w->x + 20.0f;
                w->vel[0] = -1.0f;
            } else {
                x = w->x - 20.0f;
                w->vel[0] = 1.0f;
            }
            w->lead = eneMakeEx(NULL, ENE_MOUJA_KOZ, w->zone, x, w->y + 20.0f, w->z, 2,
                                (u32 (*)(void *, void *, void *)) (u32) w->way);
            if (w->lead == NULL) {
                return -1;
            }
            shimEneSetLowFlags(w->lead, 3);
            w->sensya = eneMakeEx(NULL, ENE_SENSYA, w->zone, w->x, w->y + 20.0f, w->z, 2,
                                 (u32 (*)(void *, void *, void *)) (u32) w->way);
            if (w->sensya == NULL) {
                return -1;
            }
            shimEneSetLowFlags(w->sensya, 3);
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
            if (w->sensya != NULL) {
                w->sensya = NULL;
            }
            if (w->lead != NULL) {
                w->lead = NULL;
            }
            break;
        case 0:
            eneStepHookPre(obj);
            w->prevStep = w->step;
            switch (w->prevStep) {
                case 0:
                    sensyaKozureStepWalk(obj);
                    break;
                case 1:
                    sensyaKozureStepTurn(obj);
                    break;
                case 4:
                    sensyaKozureStepNone(obj);
                    break;
                default:
                    PRINTF("kozure: SET MISS\n");
                    PRINTF("kozure step error!\n");
                    break;
            }
            eneFall(obj, 0.4f);
            (*sensyaMoveCheck)(obj, 10.0f, 14.0f, w->z);
            if (w->hit & 1) {
                w->vel[1] = 0.0f;
            }
            if (w->hit & 6) {
                w->x -= w->vel[0];
                w->blocked++;
                w->z -= w->vel[2];
            }
            eneTurnAlongCourse(obj, 10.0f);
            eneSetModelOnCourse(obj, (Frame *) w->model);
            if (acollAtkWasHit(w->atk) == 1) {
                hit = w->atk->hitPower;
                if (hit != 0) {
                    tskSendMsg((Obj *) hit, 7, obj);
                }
            }
            hit = acollDmgWasHit(w->dmg);
            if (w->lead != NULL
                && sensyaKozureDriveLead(w->lead, w->vel[0], w->animSpeed * 5.0f, w->prevStep,
                                          hit) == 1) {
                w->lead = NULL;
            }
            if (w->sensya != NULL) {
                PRINTF("moujaD is [%x]\n", w->sensya);
                sensyaKozureDriveMouja(w->sensya, w->vel[0], w->animSpeed, w->rot[1], w->prevStep,
                                       hit);
            }
            if (hit == 1) {
                eneSetSd2(obj, 0x229, 0x22A, 0);
                effectBurst(w->model->pos[0], w->model->pos[1] + 20.0f, w->model->pos[2]);
                return -1;
            }
            if (w->homeX < w->x) {
                d = w->x - w->homeX;
            } else {
                d = -(w->x - w->homeX);
            }
            if (d > 300.0f || w->blocked >= 31) {
                itexPuffDark(w->model->pos[0], w->model->pos[1], w->model->pos[2], 0.3f);
                if (w->lead != NULL) {
                    w->lead = tskDestroy(w->lead);
                }
                if (w->sensya != NULL) {
                    w->sensya = tskDestroy(w->sensya);
                }
                return -1;
            }
            if (w->y < -600.0f) {
                return -1;
            }
            eneStepHookPost(obj);
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The kozure's step 4: nothing. */
void sensyaKozureStepNone(Obj *obj) {
}

/* The kozure's step 0: it walks for 60 steps (not while it has no lead), then step 1. */
void sensyaKozureStepWalk(Obj *obj) {
    Kozure *w = KOZURE(obj);
    s32 pad;

    if (w->timer == 0) {
        sensyaKozureSetAnim(obj, 1);
    }
    w->timer++;
    if (w->way == 4) {
        w->vel[0] = -0.5f;
    } else {
        w->vel[0] = 0.5f;
    }
    if (w->lead == NULL) {
        w->vel[0] = 0.0f;
    }
    if (w->timer > 60) {
        w->step = 1;
        w->timer = 0;
    }
    w->animSpeed = sensyaAnimSpeed(obj);
    eneAnimLoop(obj, w->model, ANIM_STEP(w->animSpeed));
}

/* The kozure's step 1: it stops and turns to the nearest player's side, plays its animation
 * once, then step 0. */
void sensyaKozureStepTurn(Obj *obj) {
    Kozure *w = KOZURE(obj);
    s32 pad;

    if (w->timer == 0) {
        w->rot[1] = eneCourseAngle(obj, shimPlSideCheck(obj, shimNearestPl(obj)));
        sensyaKozureSetAnim(obj, 2);
        w->animSpeed = 0.15f;
        w->vel[0] = 0.0f;
    }
    w->timer++;
    if (eneAnimPlayOnce(obj, w->model, ANIM_STEP(w->animSpeed)) == 1) {
        w->step = 0;
        w->timer = 0;
    }
}

/* The sensya's handler. Message 9 is a player pushing or pulling it ("push", "pull"). An s16 msg
 * with an explicit "case 3" gives the game's jump table and its lhu of the home ("(u16) msg"
 * gave lh + andi; a u16 msg an andi at entry and a copy). */
s32 sensyaProc(Obj *obj, s16 msg, void *arg) {
    Sensya *w = SENSYA(obj);
    s32 pad[2];
    s32 n;

    switch (msg) {
        case 2:
            obj->name = "sensya";
            w->rot[1] = w->aim = eneCourseAngle(obj, (u8) eneSetup(obj, arg));
            w->model = eneNewAnime(obj, (u32) sensyaBodyAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->model->flags |= 0x20;
            w->turret = eneNewAnime(obj, (u32) sensyaTurretAnim);
            if (w->turret == NULL) {
                return -1;
            }
            w->turret->flags |= 0x20;
            w->atk = NULL;
            w->grab = NULL;
            w->dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, (Frame *) w->turret, 120, 170, 0, 0,
                                0);
            if (w->dmg == NULL) {
                return -1;
            }
            if (sensyaRideMake(obj, 1) == 1) {
                return 1;
            }
            w->shadow = shadowCreate();
            if (w->shadow == NULL) {
                return -1;
            }
            if (eneSetOnEarth(obj) == -1) {
                PRINTF("sensya: SET MISS\n");
                return -1;
            }
            w->rider = 4;
            w->timer = 0;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy((Frame *) w->model);
            }
            if (w->turret != NULL) {
                frmDestroy((Frame *) w->turret);
            }
            if (w->atk != NULL) {
                frmDestroy((Frame *) w->atk);
            }
            if (w->grab != NULL) {
                frmDestroy((Frame *) w->grab);
            }
            if (w->dmg != NULL) {
                frmDestroy((Frame *) w->dmg);
            }
            if (w->mark != NULL) {
                tskDestroy(w->mark);
            }
            if (w->ride != NULL) {
                frmDestroy((Frame *) w->ride);
            }
            if (w->ride2 != NULL) {
                frmDestroy((Frame *) w->ride2);
            }
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            break;
        case 9:
            switch (((s32 *) arg)[0]) {
                default:
                    w->rollTimer = 0;
                    break;
                case 0x70757368:
                    w->vel[0] = ((s32 *) arg)[1] * 0.01f * 0.3;
                    n = sensyaRollInterval(w->vel[0]);
                    if (w->rider != 4 && w->vel[0] != 0.0f) {
                        if (w->rollTimer % n == 0) {
                            shimSdAtModel((Frame *) w->turret, 0x2D8);
                        }
                        w->rollTimer++;
                        if (w->rollTimer >= n) {
                            w->rollTimer = 0;
                        }
                    }
                    return 0x70757368;
                case 0x70756C6C:
                    w->vel[0] = ((s32 *) arg)[1] * 0.01f * 0.3;
                    n = sensyaRollInterval(w->vel[0]);
                    if (w->rider != 4 && w->vel[0] != 0.0f) {
                        if (w->rollTimer % n == 0) {
                            shimSdAtModel((Frame *) w->turret, 0x2D8);
                        }
                        w->rollTimer++;
                        if (w->rollTimer >= n) {
                            w->rollTimer = 0;
                        }
                    }
                    return 0x70756C6C;
            }
            break;
        case 0:
            eneStepHookPre(obj);
            if (w->ctrl != 0 && w->rider < 4) {
                if (plIsPoweredDown(w->rider)) {
                    return -1;
                }
                if (sensyaPlControl(obj) == 1) {
                    return -1;
                }
            }
            switch (w->step) {
                case 0:
                    if (sensyaStepRide(obj) == 1) {
                        return -1;
                    }
                    break;
                case 1:
                    if (sensyaStepFire(obj) == 1) {
                        return -1;
                    }
                    break;
                case 2:
                    if (sensyaStepMove(obj) == 1) {
                        return -1;
                    }
                    break;
                case 3:
                    if (sensyaStepWait(obj) == 1) {
                        return -1;
                    }
                    break;
                case 4:
                    if (sensyaStepNone(obj) == 1) {
                        return -1;
                    }
                    break;
                default:
                    PRINTF("sensya step error!\n");
                    break;
            }
            eneFall(obj, 0.4f);
            (*sensyaMoveCheck)(obj, 10.0f, 14.0f, w->z);
            if (w->hit & 1) {
                w->vel[1] = 0.0f;
            }
            if (w->hit & 2) {
                w->x -= w->vel[0];
                w->z -= w->vel[2];
            }
            if (w->y < -600.0f) {
                return -1;
            }
            eneSetModelOnCourse(obj, (Frame *) w->model);
            if (w->turret != NULL) {
                (*sensyaTurretOnBody)(w->model, w->turret, w->rot[1]);
            }
            w->model->rot[1] = w->aim;
            w->turret->rot[1] = vramCourseHeading(w->zone, w->x);
            w->vel[0] = 0.0f;
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.5f, NULL);
            eneStepHookPost(obj);
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The sensya's step 4: nothing. */
s32 sensyaStepNone(Obj *obj) {
    return 0;
}

/* The sensya's step 0: ridden, its animations stepped by its speed. */
s32 sensyaStepRide(Obj *obj) {
    Sensya *w = SENSYA(obj);

    if (w->timer == 0) {
        sensyaSetAnim(obj, 1);
    }
    w->timer++;
    eneAnimLoop(obj, w->model, ANIM_STEP(sensyaAnimSpeed(obj)));
    eneAnimLoop(obj, w->turret, ANIM_STEP(sensyaAnimSpeed(obj)));
    return 0;
}

/* The sensya's step 1: it fires. At its animation's frame (2.7 ridden by no one, 0.1 ridden) it
 * makes a shell (kind 0xEF) 12 ahead along the turret's aim, puffs smoke there and plays the
 * shot; at the animation's end the shot is over. */
s32 sensyaStepFire(Obj *obj) {
    Sensya *w = SENSYA(obj);
    f32 frame;
    f32 x;
    f32 z;
    SenTamaArg shot;

    if (w->rider == 4) {
        frame = 2.7f;
    } else {
        frame = 0.1f;
    }
    if (w->timer == 0) {
        sensyaSetAnim(obj, 2);
        w->fired = 0;
    }
    w->timer++;
    if (w->model->frame >= frame && w->fired == 0) {
        shot.dir = -(w->aim - vramCourseHeading(w->zone, w->x));
        shot.owner = w->rider;
        eneMakeEx(NULL, ENE_SENSYA_TAMA, w->zone, mathCos(shot.dir) * 12.0f + w->x, w->y + 18.0f,
                  mathSin(shot.dir) * 12.0f + w->z, 2, (u32 (*)(void *, void *, void *)) &shot);
        vramCoursePointXZ(w->zone, mathCos(shot.dir) * 12.0f + w->x,
                          mathSin(shot.dir) * 12.0f + w->z, &x, &z);
        itexPuffWhite(x, w->turret->pos[1] + 25.0f, z, 0.06f);
        w->fired = 1;
        shimSdAtModel((Frame *) w->model, 0x2D6);
    }
    if (eneAnimPlayOnce(obj, w->model, ANIM_STEP(w->animSpeed)) == 1) {
        w->fired = 2;
    }
    eneAnimLoop(obj, w->turret, ANIM_STEP(-sensyaAnimSpeed(obj)));
    return 0;
}

/* The sensya's step 2: as step 0. */
s32 sensyaStepMove(Obj *obj) {
    Sensya *w = SENSYA(obj);

    if (w->timer == 0) {
        sensyaSetAnim(obj, 1);
    }
    w->timer++;
    eneAnimLoop(obj, w->model, ANIM_STEP(sensyaAnimSpeed(obj)));
    eneAnimLoop(obj, w->turret, ANIM_STEP(sensyaAnimSpeed(obj)));
    return 0;
}

/* The sensya's step 3: it waits for a rider. Its first step makes the rides, the mark (kind
 * 0xE9) and its two boxes; a player in state 15, 22, 5 or 6 who hits its box rides it (step 2,
 * the rider's action 0xCF). */
s32 sensyaStepWait(Obj *obj) {
    Sensya *w = SENSYA(obj);
    Obj *pl;
    s32 state;

    if (w->timer == 0) {
        sensyaSetAnim(obj, 1);
        w->fired = 0;
        if (sensyaRideMake(obj, 0) == 1) {
            return 1;
        }
        w->mark = eneMakeEx(NULL, ENE_NORU_MARK, w->zone, w->x, w->y + 24.0f, w->z, 0, NULL);
        if (w->mark == NULL) {
            return 1;
        }
        shimEneSetLowFlags(w->mark, 3);
        if (w->grab != NULL) {
            w->grab = (ACollFrame *) frmDestroy((Frame *) w->grab);
        }
        w->grab = ATK_ADD_YZ((Frame *) ene_atk_root_frame, (Frame *) w->turret, 100, 90, 0, 0);
        if (w->grab == NULL) {
            return 1;
        }
        w->grab->kind = 100;
        if (w->atk != NULL) {
            w->atk = (ACollFrame *) frmDestroy((Frame *) w->atk);
        }
        w->atk = ATK_ADD_YZ((Frame *) pl_atk_root_frame, (Frame *) w->turret, 90, 100, 0, 0);
        if (w->atk == NULL) {
            return 1;
        }
        w->atk->kind = 0x1D;
        w->z = 0.0f;
    }
    w->timer++;
    sensyaPlaceEnemy(w->mark, w->zone, w->x, w->y + 24.0f, w->z);
    if (acollAtkWasHit(w->grab) == 1) {
        pl = (Obj *) w->grab->hitPower;
        if (pl != NULL) {
            state = shimPlGetState((Player *) pl);
            if (state == 15 || state == 22 || state == 5 || state == 6) {
                w->rider = plGetIndex(pl);
                if (shimPlGetHitKind((Player *) pl) == 0) {
                    shimPlSetHitKind((Player *) pl, (s32) obj);
                    w->mark = tskDestroy(w->mark);
                    if (sensyaRideMake(obj, 1) == 1) {
                        return 1;
                    }
                    sensyaRiderSide(gGameWork.players[w->rider], &w->save);
                    if (plSetAction(gGameWork.players[w->rider], 0xCF) == 0) {
                        PRINTF("plExtPaChange fault!!!\n");
                        return 1;
                    }
                    if (w->grab != NULL) {
                        w->grab = (ACollFrame *) frmDestroy((Frame *) w->grab);
                    }
                    w->step = 2;
                    w->timer = 0;
                    w->ctrl = 0x1010;
                    sensyaRiderVoice(obj, pl);
                    shimSdAtModel((Frame *) w->model, 0x2C2);
                }
            }
        }
    }
    eneAnimLoop(obj, w->model, ANIM_STEP(sensyaAnimSpeed(obj)));
    eneAnimLoop(obj, w->turret, ANIM_STEP(sensyaAnimSpeed(obj)));
    return 0;
}

/* The ridden sensya's control: the rider's controls by the step, its answer (1 fire, 2 move,
 * 3 fire, 7 get off) and the aim from the rider's model. 1: the rider is gone. The explicit
 * cases 3 and 4-6 are the game's jump tables (a hole there goes elsewhere). */
s32 sensyaPlControl(Obj *obj) {
    Sensya *w = SENSYA(obj);
    u8 step = 0;

    switch (w->step) {
        case 0:
            w->ctrl = 0x1011;
            break;
        case 1:
            w->ctrl = 0x1012;
            if (w->fired == 2) {
                w->ctrl = 0x1014;
            }
            break;
        case 2:
            w->ctrl = 0x1010;
            break;
        case 3:
            break;
        case 4:
            w->ctrl = 0;
            break;
        default:
            PRINTF("plSensyaControl step error!\n");
            break;
    }
    w->ctrlRet = ((s32 (*)(s32, s32)) shimEneChangeCtrl)(w->ctrl, w->rider);
    if (gGameWork.players[w->rider] == NULL) {
        return 1;
    }
    switch (w->ctrlRet) {
        case 1:
            step = 2;
            w->animSpeed = 0.15f;
            break;
        case 2:
            w->animSpeed = sensyaAnimSpeed(obj);
            break;
        case 3:
            step = 1;
            w->animSpeed = 0.15f;
            break;
        case 4:
        case 5:
        case 6:
            break;
        case 7:
            if (gGameWork.players[w->rider] != NULL
                && shimPlGetHitKind((Player *) gGameWork.players[w->rider]) != 0) {
                shimPlSetHitKind((Player *) gGameWork.players[w->rider], 0);
                sensyaRiderSide(gGameWork.players[w->rider], &w->save);
            }
            PRINTF("sensya_noru noru_step CHANGE!\n");
            w->ctrl = 0;
            step = 3;
            w->timer = 0;
            break;
        default:
            PRINTF("SensyaControl step error!\n");
            break;
    }
    w->aim = plGetModelYaw(gGameWork.players[w->rider]);
    if (sensyaRiderFacing(gGameWork.players[w->rider]) != w->facing) {
        shimSdAtModel((Frame *) w->model, 0x2D7);
    }
    w->facing = sensyaRiderFacing(gGameWork.players[w->rider]);
    if (gGameWork.players[w->rider] == NULL) {
        return 1;
    }
    w->step = step;
    if (w->step != w->prevStep) {
        /* Where these two prints were is a guess: their strings follow the default's. */
        PRINTF("SensyaControl step CHANGE!\n");
        PRINTF("SensyaControl step CHANGE!\n");
        w->timer = 0;
    }
    w->prevStep = w->step;
    return 0;
}

/* A rider's side kept while it rides (file 78's umaNoruKeepSide). */
void sensyaRiderSide(Obj *pl, f32 *save) {
    if (PL(pl)->ride != NULL) {
        *save = PL(pl)->side;
    } else {
        PL(pl)->side = *save;
    }
}

/* A player's target facing. */
f32 sensyaRiderFacing(Obj *pl) {
    return PL(pl)->targetFacing;
}

/* The turret on the body: its place and tilt, turned to rotY. */
void sensyaTurretOnBody(AnimeFrame *src, AnimeFrame *dst, f32 rotY) {
    dst->pos[0] = src->pos[0];
    dst->pos[1] = src->pos[1];
    dst->pos[2] = src->pos[2];
    dst->rot[0] = src->rot[0];
    dst->rot[1] = rotY;
    dst->rot[2] = src->rot[2];
}

/* The sensya's rides (what a player stands on): mode 0 the turret's, 1 the turret's and the
 * body's. 1 if one could not be made. */
s32 sensyaRideMake(Obj *obj, s32 mode) {
    Sensya *w = SENSYA(obj);

    if (w->ride != NULL) {
        w->ride = (RideFrame *) frmDestroy((Frame *) w->ride);
    }
    if (w->ride2 != NULL) {
        w->ride2 = (RideFrame *) frmDestroy((Frame *) w->ride2);
    }
    switch (mode) {
        case 0:
            w->ride = rideCreate(gRideTree1, (s32) obj, (Frame *) w->turret,
                                 (RideData *) eneFileAddr(obj, sensyaTurretRide, 0));
            if (w->ride == NULL) {
                return 1;
            }
            break;
        case 1:
            w->ride = rideCreate(gRideTree1, (s32) obj, (Frame *) w->turret,
                                 (RideData *) eneFileAddr(obj, sensyaTurretRide, 0));
            if (w->ride == NULL) {
                return 1;
            }
            w->ride2 = rideCreate(gRideTree1, (s32) obj, (Frame *) w->model,
                                  (RideData *) eneFileAddr(obj, sensyaBodyRide, 0));
            if (w->ride2 == NULL) {
                return 1;
            }
            break;
    }
    return 0;
}

/* The kozure's animation anim (0-2), from its start. */
void sensyaKozureSetAnim(Obj *obj, s32 anim) {
    KOZURE(obj)->model->anim = sensyaKozureAnims[anim];
    KOZURE(obj)->model->frame = 0.0f;
}

/* The sensya's animation anim (0-2), from its start. */
void sensyaSetAnim(Obj *obj, s32 anim) {
    SENSYA(obj)->model->frame = 0.0f;
    SENSYA(obj)->model->anim = sensyaAnims[anim];
}

/* The kozure's lead (kind 0xED): its speed and animation speed from the kozure, and a hit on
 * the kozure passed on. 1 once the lead is hit. */
s32 sensyaKozureDriveLead(Obj *obj, f32 vx, f32 speed, u8 step, s32 hit) {
    KozLead *w = KOZLEAD(obj);

    w->vel[0] = vx;
    w->animSpeed = speed;
    if (hit == 1) {
        sensyaKozureLeadOff(obj);
    }
    if (acollDmgWasHit(w->dmg) == 1) {
        return 1;
    }
    return 0;
}

/* The lead's flag 0x51 cleared. */
s32 sensyaKozureLeadOff(Obj *obj) {
    KOZLEAD(obj)->on = 0;
    return 0;
}

/* The kozure's sensya (kind 0xEE): its speed, animation speed, heading and step from
 * the kozure (step 5 and 3 when the kozure is hit); a new step restarts its timer. */
s32 sensyaKozureDriveMouja(Obj *obj, f32 vx, f32 speed, f32 rotY, u8 step, s32 hit) {
    KozM *w;

    KOZM(obj)->vel[0] = vx;
    KOZM(obj)->animSpeed = speed;
    w = KOZM(obj);
    if (hit != 1) {
        w->step = step;
    } else {
        w->mode = 5;
        w->step = 3;
    }
    w->rotY = rotY;
    if (w->step != w->prevStep) {
        w->timer = 0;
    }
    w->prevStep = w->step;
    return 0;
}

/* An enemy's animation speed (the kozure's and the sensya's): its speed / 5, within -1 to 1.
 * The first compare is against an int 1: the game loads its own 1.0f for it apart from the
 * clamp's (as 1.0f, IDO shared one register). */
f32 sensyaAnimSpeed(Obj *obj) {
    f32 s = KOZURE(obj)->vel[0] / 5.0f;

    if (s > 1) {
        s = 1.0f;
    } else if (s < -1.0f) {
        s = -1.0f;
    }
    return s;
}

/* Steps between the sensya's rolling sounds at a speed: 30 slow (0.04 and under) to 26 fast
 * (0.68 and over). The quotient in a local t gives the game's "mul.s -0.2, t" order (inline,
 * in either order, IDO put the quotient first). */
s32 sensyaRollInterval(f32 speed) {
    f32 t;
    f32 s = ABS(speed);

    if (s > 0.68f) {
        s = 0.68f;
    } else if (s < 0.04f) {
        s = 0.04f;
    }
    t = (s - 0.04f) / 0.0064f;
    return -0.2f * t + 30.0f;
}

/* An enemy's course and place. */
void sensyaPlaceEnemy(Obj *obj, s32 zone, f32 x, f32 y, f32 z) {
    EneObj *e = (EneObj *) FX(obj);

    e->zone = zone;
    e->x = x;
    e->y = y;
    e->z = z;
}

/* The shell's handler ("sen_tama", kind 0xEF): it flies along its direction under gravity and
 * bursts where it hits something (or 200 from where it was fired), its blast growing to 180. */
s32 sensyaTamaProc(Obj *obj, s16 msg, void *arg) {
    SenTama *w = SENTAMA(obj);
    SenTamaArg *shot;
    Obj *by;

    switch (msg) {
        case 2:
            obj->name = "sen_tama";
            shot = (SenTamaArg *) stageAddrToRam(eneSetup(obj, arg));
            w->model = eneNewPol(obj, (Gfx *) sensyaTamaShellDl);
            if (w->model == NULL) {
                return -1;
            }
            w->owner = shot->owner;
            w->dir = shot->dir;
            sensyaTamaAtkMake(obj, w->owner);
            if (w->atk == NULL) {
                return -1;
            }
            w->model->flags |= 7;
            w->model->scale[2] = 0.15f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->vel[0] = mathCos(w->dir) * 2.3f;
            w->vel[1] = 3.4f;
            w->vel[2] = mathSin(w->dir) * 2.3f;
            w->step = 0;
            w->blast = 6;
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
        case 0:
            eneStepHookPre(obj);
            switch (w->step) {
                case 0:
                    eneFall(obj, 0.4f);
                    shimEneMove(obj, 5.0f, 2.0f, w->z, 4.0f);
                    eneSetModelOnCourse(obj, (Frame *) w->model);
                    if (DIST(w->x, w->startX) > 200.0f) {
                        return -1;
                    }
                    if (acollAtkWasHit(w->atk) == 1 || (w->hit & 3)) {
                        if (w->owner == 4) {
                            by = (Obj *) w->atk->hitPower;
                            if (by != NULL) {
                                tskSendMsg(by, 7, obj);
                            }
                        }
                        if (w->owner != 4) {
                            itexPuffDark(w->model->pos[0], w->model->pos[1], w->model->pos[2],
                                         0.16f);
                        } else {
                            itexPuffDark(w->model->pos[0], w->model->pos[1], w->model->pos[2],
                                         0.08f);
                        }
                        sensyaSoundLocation(0x290, gGameWork.view, w->model->pos[0],
                                               w->model->pos[1], w->model->pos[2], 150.0f, 800.0f,
                                               1.5707964f, 0xFF);
                        w->step = 1;
                    }
                    break;
                case 1:
                    w->model->dl2 = NULL;
                    w->atk->radius = w->blast;
                    w->blast += 15;
                    if (w->blast >= 181) {
                        return -1;
                    }
                    break;
            }
            if (w->y < -600.0f) {
                return -1;
            }
            eneStepHookPost(obj);
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The shell's attack box: a player's (kind 0x1D) for a rider's shell, an enemy's (0x7B). */
void sensyaTamaAtkMake(Obj *obj, s32 owner) {
    SenTama *w = SENTAMA(obj);
    s32 pad;

    if (owner < 4) {
        w->atk = ATK_ADD_YZ((Frame *) pl_atk_root_frame, (Frame *) w->model, 6, 6, 0, 0);
        if (w->atk != NULL) {
            w->atk->kind = 0x1D;
        }
    } else {
        w->atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, (Frame *) w->model, 6, 6, 0, 0);
        if (w->atk != NULL) {
            w->atk->kind = 0x7B;
        }
    }
}

/* An enemy moved, then its ground (hit 1) and wall (hit 2) checks. */
void sensyaMoveCheck(Obj *obj, f32 h, f32 r, f32 z) {
    EneObj *e = (EneObj *) FX(obj);

    eneMove(e);
    e->hit &= ~7;
    if ((*sensyaGroundCheck)(e, h, r)) {
        e->hit |= 1;
    }
    if (sensyaWallCheck(e, h / 2, r, z) < 0) {
        e->hit |= 2;
    }
}

/* An enemy falling (or still): rays down by its fall from its middle and r either side; a hit
 * puts it on the ground (1), and at its middle onto another course (-1). 0: no ground. */
s32 sensyaGroundCheck(EneObj *e, f32 h, f32 r) {
    Vec3f p0;
    Vec3f p1;
    Vec3f p2;
    Vec3f dir;
    Vec3f at;
    CollTri *hit;
    f32 t;

    if (e->vel[1] <= 0.0f) {
        dir.x = 0.0f;
        dir.y = e->vel[1] - h;
        dir.z = 0.0f;
        vramCoursePointXZ(e->zone, e->x, e->z, &p0.x, &p0.z);
        p0.y = e->y + h;
        vramCoursePointXZ(e->zone, e->x - r, e->z, &p1.x, &p1.z);
        p1.y = e->y + h;
        vramCoursePointXZ(e->zone, e->x + r, e->z, &p2.x, &p2.z);
        p2.y = e->y + h;
        if ((hit = vramRayGround(&p0, &dir, &at, &t)) != NULL) {
            e->y = at.y;
            if (e->zone != 0 && hit->zone != 0 && e->zone != hit->zone) {
                e->zone = hit->zone;
                e->x = vramNearestPlaceXZ(e->zone, p0.x, p0.z, &e->z);
                return -1;
            }
            return 1;
        }
        if (vramRayGround(&p1, &dir, &at, &t) != NULL) {
            e->y = at.y;
            return 1;
        }
        if (vramRayGround(&p2, &dir, &at, &t) != NULL) {
            e->y = at.y;
            return 1;
        }
    }
    return 0;
}

/* An enemy's wall check: a ray at height h from its place to r along and z across the course,
 * the way it moves. -1: a wall. */
s32 sensyaWallCheck(EneObj *e, f32 h, f32 r, f32 z) {
    Vec3f to;
    Vec3f from;
    Vec3f dir;
    Vec3f at;
    f32 t;
    CollTri *hit;

    r = ABS(r);
    if (e->vel[0] < 0.0f) {
        r *= -1.0f;
    }
    z = ABS(z);
    if (e->vel[2] < 0.0f) {
        z *= -1.0f;
    }
    h = ABS(h);
    vramCoursePointXZ(e->zone, e->x + r, e->z + z, &to.x, &to.z);
    vramCoursePointXZ(e->zone, e->x, e->z, &from.x, &from.z);
    from.y = e->y + h;
    dir.x = to.x - from.x;
    dir.z = to.z - from.z;
    dir.y = 0.0f;
    hit = vramRayGround(&from, &dir, &at, &t);
    if (hit != NULL) {
        return -1;
    }
    return 0;
}

/* The rider's sound as it gets on, by character (file 78's umaNoruMountVoice). */
void sensyaRiderVoice(Obj *obj, Obj *pl) {
    switch (plGetChara(pl)) {
        case 0:
            shimSdAtModel((Frame *) SENSYA(obj)->model, 0x393);
            break;
        case 1:
            shimSdAtModel((Frame *) SENSYA(obj)->model, 0x372);
            break;
        case 2:
            shimSdAtModel((Frame *) SENSYA(obj)->model, 0x3B2);
            break;
        case 3:
            shimSdAtModel((Frame *) SENSYA(obj)->model, 0x3CF);
            break;
    }
}
