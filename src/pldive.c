#include "player.h"
#include "audio.h"
#include "diveef.h"
#include "fade.h"
#include "file.h"
#include "gfxmgr.h"
#include "mathutil.h"
#include "model.h"
#include "plbox.h"
#include "pldive.h"
#include "plmain.h"
#include "plmodel.h"
#include "plonigiri.h"
#include "plpick.h"
#include "plshot.h"
#include "plsound.h"
#include "plutil.h"
#include "polygon.h"
#include "tmanager.h"
#include "vram.h"

/* Diving (overlay 9): Sasuke's and Yae's actions under water. Sasuke (character 2) dives with
 * a screw on his back, a task of its own ("screw", file 0x1AA) that spins faster as he swims
 * and sounds louder with it; Yae (character 3) swims as a mermaid. diveDefault is every dive
 * action's default; __diveStart starts a stage in dive mode. */

/* The screw task's work. */
typedef struct PlScrew {
    /* 0x0 */ AnimeFrame *frame;
    /* 0x4 */ s32 index; /* the player's */
    /* 0x8 */ f32 speed; /* the spin */
} PlScrew;

#define SCREW(obj) ((PlScrew *) ((u32) (obj) + sizeof(Obj)))

/* A dive action's own work (Player.work). */
typedef struct PlDive {
    /* 0x0 */ ACollFrame *box; /* the dive attack's */
} PlDive;

/* A player's weapon state in main (10 bytes each). */
typedef struct PlStat {
    /* 0x0 */ u8 pad0[3];
    /* 0x3 */ s8 wpLv;
    /* 0x4 */ u8 pad4[6];
} PlStat;

/* Sasuke's dive attacks, by weapon level. */
typedef struct PlDiveTbl {
    s32 v[3];
} PlDiveTbl;

extern PlStat gPlayerStats[];
extern Frame *gFrmModelLayer; /* the second frame pool */
extern s16 gFrmKindAnime; /* "anime" */
extern ACollFrame *pl_atk_root_frame; /* the player attacks' boxes */
/* pldive.c's .data (0x801A7290-0x801A72B0). */
s32 gDiveSasNextActionTable[3] = { 0x57, 0x58, 0x59 }; /* the next action, by weapon level */
PlDiveTbl gDiveSasAttackKinds = { { 10, 21, 32 } }; /* the dive attacks, by weapon level */
extern u8 sasScrewAnim[]; /* the screw's animation, in file 0x1AA */



/* Every dive action's default: the dive's flag and depth, and the swimming. */
void diveDefault(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 1:
            plFaceMoveDir(p, 0.5235988f);
            diveLean(p);
            if (p->chara == 2) {
                plHumStart(p);
                diveScrewSound(p);
            }
            return;
        case 0:
            p->flags |= 0x10000;
            p->probeDrop = 10.0f;
            plFormSizeSet(p, 4);
            break;
        case 2:
            p->flags &= ~0x10000;
            p->probeDrop = 0.0f;
            plFormSizeSet(p, 0);
            break;
        case 3:
            p->lean = 0.0f;
            p->targetLean = 0.0f;
            break;
    }
    plBaseDefault(p, msg, a, b);
}

/* The screw's sound: panned to Sasuke, pitched by its spin. */
void diveScrewSound(Player *p) {
    if (p->chara == 2 && p->action != 0x53) {
        sndSetParam(0x252, 2,
                           plSoundPan(gGameWork.view, p->model->pos[0],
                                                  p->model->pos[1], p->model->pos[2], 0.0f, 0.0f,
                                                  gGameWork.view->fovy, p->index) *
                                   2 -
                               0x80);
        sndSetParam(0x252, 0, diveScrewSpin(p->chain) * 630.0f);
    }
}

void diveYaeBegin(Player *p) {
    p->flags |= 0x10000;
    p->probeDrop = 10.0f;
    plFormSizeSet(p, 4);
}

void diveYaeDiveIn(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    switch (msg) {
        case 0:
            p->flags |= 0x10000;
            p->lean = 0.0f;
            p->probeDrop = 10.0f;
            plFormSizeSet(p, 0);
            plDiveefCreate(p, 10.0f);
            plSound(p, 0x15C);
            plSound(p, 0x3CE);
            return;
        case 1:
            if (plAnimEnded(p)) {
                next = 0x44;
            }
            plActChange(p, next);
            return;
        case 3:
            return;
    }
    diveDefault(p, msg, a, b);
}

void diveYaeDiveInEnd(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    switch (msg) {
        case 0:
            p->flags |= 0x10000;
            p->probeDrop = 10.0f;
            plFormSizeSet(p, 0);
            return;
        case 1:
            if (plAnimEnded(p)) {
                next = 0x47;
                p->height += 8.0f;
            }
            plActChange(p, next);
            return;
        case 3:
            return;
    }
    diveDefault(p, msg, a, b);
}

/* Diving in: down to the water, by a ray 8 deep. */
void diveYaeSurface(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;
    Vec3f from;
    Vec3f dir;
    Vec3f at;
    f32 t;

    switch (msg) {
        case 0:
            vramCoursePointXZ(p->zone, p->place, p->side, &from.x, &from.z);
            dir.x = dir.z = 0.0f;
            dir.y = -8.0f;
            from.y = p->height;
            vramRayGround(&from, &dir, &at, &t);
            p->height -= 8.0f * (t - 0.01f);
            p->lean = 0.0f;
            plDiveefCreate(p, 5.0f);
            plSound(p, 0x15C);
            break;
        case 1:
            if (plAnimEnded(p)) {
                next = 0x52;
            }
            p->lean = 0.0f;
            p->targetLean = 0.0f;
            plActChange(p, next);
            break;
        case 3:
            return;
    }
    diveDefault(p, msg, a, b);
}

void diveSurfaceEnd(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    switch (msg) {
        case 1:
            if (plAnimEnded(p)) {
                next = 0x19;
            }
            if (plActChange(p, next)) {
                return;
            }
            break;
        case 2:
            p->lean = 0.0f;
            p->targetLean = 0.0f;
            break;
        case 3:
            return;
    }
    diveDefault(p, msg, a, b);
}

/* Swimming (Yae). */
void diveYaeSwim(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;
    f32 speed;

    if (msg == 1) {
        diveStickSwim(p);
        speed = plPushLen(p);
        p->animStepAdd = speed * 3.0f;
        next = diveYaeNextAction(p);
        if (next == -1) {
            if (plSpeedLen(p) < 0.05f) {
                next = 0x46;
            }
        } else if (next == 0x4F) {
            next = 0x50;
        }
        plFaceMoveDir(p, 2.0f * speed + 0.2617994f);
        diveLean(p);
        plActChange(p, next);
        return;
    }
    diveDefault(p, msg, a, b);
}

void diveYaeSwimStop(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;

    if (msg == 1) {
        diveStickSwim(p);
        p->animStepAdd = plPushLen(p) * 3.0f;
        next = diveYaeNextAction(p);
        if (next == -1 && plAnimEnded(p)) {
            next = 0x47;
        }
        if (plActChange(p, next)) {
            return;
        }
    }
    diveDefault(p, msg, a, b);
}

void diveYaeIdle(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;

    if (msg == 1) {
        diveStickSwim(p);
        next = diveYaeNextAction(p);
        if (next == -1 && plPushLen(p) >= 0.05f) {
            next = 0x48;
        }
        if (plActChange(p, next)) {
            return;
        }
        if (p->prevAction == 0x4F || p->prevAction == 0x50) {
            diveLean(p);
            p->targetFacing = p->facing;
            return;
        }
    }
    diveDefault(p, msg, a, b);
}

void diveYaeSwimStart(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;

    if (msg == 1) {
        diveStickSwim(p);
        p->animStepAdd = plPushLen(p) * 3.0f;
        next = diveYaeNextAction(p);
        if (next == -1) {
            if (plAnimEnded(p)) {
                next = 0x45;
            } else if (plSpeedLen(p) < 0.05f) {
                next = 0x46;
            }
        } else if (next == 0x4F) {
            next = 0x50;
        }
        if (plActChange(p, next)) {
            return;
        }
    }
    diveDefault(p, msg, a, b);
}

/* The dive attack: slow down, open the attack box, then dart along the lean. */
void diveYaeDash(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;
    s32 t;
    PlDive *w = (PlDive *) p->work;

    switch (msg) {
        case 0:
            p->state = 0;
            w->box = NULL;
            break;
        case 1:
            t = plAnimPercent(p);
            switch (p->state) {
                case 0:
                    if (t >= 10) {
                        plInvincibleSet(p, 0x73);
                    }
                    if (t < 50) {
                        diveStickSwim(p);
                        p->pushX *= 0.75f;
                        p->pushY *= 0.75f;
                    }
                    if (t > 32) {
                        if ((w->box = acollAtkAddYZ((Frame *) pl_atk_root_frame, &p->model->f, 80,
                                                             0, 50, 0)) != NULL) {
                            w->box->kind = 0x18;
                        }
                        p->state = 1;
                    }
                    break;
                case 1:
                    if (t >= 50) {
                        p->pushX = ((mathCos(p->facing) >= 0.0f)
                                         ? mathCos(p->lean)
                                         : -mathCos(p->lean)) *
                                    6.0f;
                        p->pushY = -mathSin(p->lean) * 6.0f;
                        p->state = 2;
                        plSound(p, 0x44D);
                    }
                    break;
            }
            if (plAnimEnded(p)) {
                next = 0x4A;
            }
            plFaceMoveDir(p, 0.5235988f);
            plLeanTo(p, 0.19634955f);
            plActChange(p, next);
            return;
        case 2:
            frmDestroy((Frame *) w->box);
            break;
    }
    diveDefault(p, msg, a, b);
}

void diveYaeDash2(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    switch (msg) {
        case 0:
            plInvincibleSet(p, 0xF);
            break;
        case 1:
            diveStickSwim(p);
            plFaceMoveDir(p, 0.5235988f);
            if (plPushLen(p) >= 0.010000001f) {
                diveLean(p);
            } else {
                plLeanTo(p, 0.19634955f);
            }
            if (plAnimEnded(p)) {
                next = 0x4B;
            }
            plActChange(p, next);
            return;
    }
    diveDefault(p, msg, a, b);
}

void diveYaeDash3(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;

    if (msg == 1) {
        diveStickSwim(p);
        plFaceMoveDir(p, 0.5235988f);
        next = diveYaeNextAction(p);
        if (next == -1) {
            if (plPushLen(p) >= 0.010000001f) {
                next = 0x45;
            }
            diveLean(p);
        } else {
            plLeanTo(p, 0.19634955f);
        }
        if (plAnimEnded(p)) {
            next = 0x4C;
        }
        plActChange(p, next);
        return;
    }
    diveDefault(p, msg, a, b);
}

void diveYaeDashEnd(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;

    if (msg == 1) {
        diveStickSwim(p);
        next = diveYaeNextAction(p);
        if (next == -1 && plPushLen(p) >= 0.010000001f) {
            next = 0x45;
        }
        if (plAnimEnded(p)) {
            next = 0x47;
        }
        plActChange(p, next);
    }
    diveDefault(p, msg, a, b);
}

/* Hit under water (Yae). */
void diveYaeHit(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    switch (msg) {
        case 0:
            diveDefault(p, msg, a, b);
            if (plDamage(p, p->damage, p->weaponDamage)) {
                plActChange(p, 0x4E);
                return;
            }
            plInvincibleSet(p, 0x4B);
            plVoiceDamage(p);
            break;
        case 1:
            if (plAnimEnded(p)) {
                next = 0x47;
            }
            plActChange(p, next);
            return;
        case 2:
            diveDefault(p, msg, a, b);
            plInvincibleSet(p, 0x4B);
            plBlinkSet(p, 0x4B);
            break;
        case 3:
            break;
    }
    plActNoneDefault(p, msg, a, b);
}

/* Eating under water (Yae). */
void diveYaeEat(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 1:
            if (p->actionSteps == 0x3C) {
                p->onigiri = plOnigiriCreate(p);
            }
            return;
        case 0:
            plBlinkSet(p, 0);
            p->flags |= 0x200;
            plSound(p, 0x114);
            plEatVoice(p);
            break;
        case 2:
            p->lean = 0.0f;
            p->flags &= ~0x200;
            p->targetLean = 0.0f;
            break;
    }
    diveDefault(p, msg, a, b);
}

void diveYaeBazooka(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    switch (msg) {
        case 0:
            p->state = 0;
            p->facing = p->targetFacing;
            break;
        case 1:
            switch (p->state) {
                case 0:
                    if (p->model->frame >= 8.0f) {
                        plSound(p, 0x11E);
                        plDbazFire(p);
                        p->state = 1;
                    }
                case 1:
                    if (p->model->frame >= 18.0f) {
                        next = diveYaeNextAction(p);
                        if (next == -1) {
                            diveStickSwim(p);
                            if (plPushLen(p) >= 0.05f) {
                                next = 0x45;
                            } else {
                                p->pushY = 0.0f;
                                p->pushX = 0.0f;
                            }
                        }
                    }
                    break;
            }
            if (next == -1 && plAnimEnded(p)) {
                next = 0x47;
            }
            plActChange(p, next);
            return;
    }
    diveDefault(p, msg, a, b);
}

void diveYaeSwimBazooka(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    switch (msg) {
        case 0:
            p->state = 0;
            p->facing = p->targetFacing;
            break;
        case 1:
            switch (p->state) {
                case 0:
                    if (p->model->frame >= 8.0f) {
                        plSound(p, 0x11E);
                        plDbazFire(p);
                        p->state = 1;
                    }
                    break;
                case 1:
                    if (p->model->frame >= 23.0f) {
                        next = diveYaeNextAction(p);
                        if (next == -1) {
                            diveStickSwim(p);
                            if (plPushLen(p) >= 0.05f) {
                                next = 0x45;
                            } else {
                                p->pushY = 0.0f;
                                p->pushX = 0.0f;
                            }
                        }
                    }
                    break;
            }
            if (next == -1 && plAnimEnded(p)) {
                next = 0x47;
            }
            plActChange(p, next);
            return;
    }
    diveDefault(p, msg, a, b);
}

/* B or Z held (and not busy): a dive attack. */
s32 diveIsAttackHeld(Player *p) {
    if (!(p->flags & 0x3020) && (p->held & 0x6000)) {
        return TRUE;
    }
    return FALSE;
}

/* The next action from the buttons, or -1. */
s32 diveYaeNextAction(Player *p) {
    s32 next = -1;

    if (diveIsAttackHeld(p)) {
        next = 0x4F;
    } else if (diveIsAHeld(p)) {
        next = 0x49;
    } else if (plCanSurface(p)) {
        next = 0x51;
    }
    return next;
}

/* Sasuke's: the attack by his weapon's level. */
s32 diveSasNextAction(Player *p) {
    s32 next = -1;

    if (diveIsAttackHeld(p)) {
        next = gDiveSasNextActionTable[gPlayerStats[p->index].wpLv];
    } else if (plCanSurface(p)) {
        next = 0x54;
    }
    return next;
}

/* A held. */
s32 diveIsAHeld(Player *p) {
    return (p->held & A_BUTTON) ? TRUE : FALSE;
}

/* Swimming by the stick. */
void diveStickSwim(Player *p) {
    f32 k = diveDepthFactor(p);

    p->pushX = plStickSpeedX(p) * k * 0.18f;
    p->pushY = plStickSpeedY(p) * k * 0.18f;
}

f32 diveDepthFactor(Player *p) {
    f32 r;

    if (p->swimDepth < p->waterDepth) {
        r = 1.0f;
    } else {
        r = p->waterDepth / p->swimDepth;
    }
    return r;
}

void diveUnusedStub1(void) {
}

/* Leans the body along the way it moves. */
void diveLean(Player *p) {
    if (plSpeedLen(p) > 0.8f || plPushLen(p) > 0.05f) {
        if (cosf(p->facing) >= 0.0f) {
            p->targetLean = mathAtan2(p->velX, p->velY) - 1.5707963f;
        } else {
            p->targetLean = mathAtan2(-p->velX, p->velY) - 1.5707963f;
        }
    } else {
        p->targetLean = 0.0f;
    }
    plLeanTo(p, 0.19634955f);
}

void diveUnusedStub2(void) {
}

/* Sasuke dives: with the screw on. */
void diveSasBegin(Player *p) {
    p->flags |= 0x10000;
    p->probeDrop = 10.0f;
    plFormSizeSet(p, 4);
    p->chain = fadeCreateRetry(gGameWork.players[p->index], sasScrew, 1,
                                      0x1AA, p);
}

void diveSasDiveIn(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    switch (msg) {
        case 0:
            p->flags |= 0x10000;
            p->lean = 0.0f;
            p->probeDrop = 10.0f;
            plFormSizeSet(p, 0);
            p->chain = NULL;
            plDiveefCreate(p, 10.0f);
            plSound(p, 0x3B1);
            plSound(p, 0x15C);
            return;
        case 1:
            if (plAnimEnded(p)) {
                next = 0x56;
                p->chain = fadeCreateRetry(gGameWork.players[p->index],
                                                  sasScrew, 1, 0x1AA, p);
                p->height += 5.0f;
                plHumStart(p);
            }
            plActChange(p, next);
            return;
        case 3:
            return;
    }
    diveDefault(p, msg, a, b);
}

/* Sasuke comes up: the screw goes. */
void diveSasSurface(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            plDiveefCreate(p, 0.0f);
            plSound(p, 0x15C);
            break;
        case 2:
            diveDefault(p, msg, a, b);
            if (p->chain != NULL) {
                p->chain = tskDestroy(p->chain);
            }
            plHumStop(p);
            p->lean = 0.0f;
            p->targetLean = 0.0f;
            return;
        case 3:
            return;
        default:
            diveSurfaceEnd(p, msg, a, b);
            break;
    }
    diveDefault(p, msg, a, b);
}

/* Swimming (Sasuke). */
void diveSasSwim(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;
    f32 speed;

    if (msg == 1) {
        diveStickSwim(p);
        speed = plPushLen(p);
        p->animStepAdd = speed * 3.0f;
        next = diveSasNextAction(p);
        if (next == -1 && plSpeedLen(p) < 0.05f) {
            next = 0x56;
        }
        plFaceMoveDir(p, 2.0f * speed + 0.2617994f);
        diveLean(p);
        diveScrewSound(p);
        plActChange(p, next);
        return;
    }
    diveDefault(p, msg, a, b);
}

void diveSasIdle(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;

    if (msg == 1) {
        diveStickSwim(p);
        next = diveSasNextAction(p);
        if (next == -1 && plPushLen(p) >= 0.05f) {
            next = 0x55;
        }
        if (plActChange(p, next)) {
            return;
        }
    }
    diveDefault(p, msg, a, b);
}

/* Sasuke's dive attack, by his weapon's level. */
void diveSasAttack(Player *p, s32 msg, s32 a, s32 b) {
    s32 t;
    s32 next;
    PlDiveTbl tbl = gDiveSasAttackKinds;

    switch (msg) {
        case 0:
            plBoxSetKind(p, tbl.v[gPlayerStats[p->index].wpLv]);
            plAttackSound(p);
            plChargeRelease(p);
            break;
        case 1:
            next = -1;
            diveStickSwim(p);
            plFaceMoveDir(p, 0.5235988f);
            diveLean(p);
            diveScrewSound(p);
            p->pushY = 0.0f;
            p->pushX = 0.0f;
            t = plAnimPercent(p);
            if (t == 100) {
                next = 0x56;
            } else if (t >= 75) {
                next = diveSasNextAction(p);
            }
            plActSwitch(p, next);
            return;
        case 2:
            plBoxDelete(p);
            break;
    }
    diveDefault(p, msg, a, b);
}

void diveSasAttackLv0(Player *p, s32 msg, s32 a, s32 b) {
    diveSasAttack(p, msg, a, b);
}

void diveSasAttackLv1(Player *p, s32 msg, s32 a, s32 b) {
    diveSasAttack(p, msg, a, b);
}

void diveSasAttackLv2(Player *p, s32 msg, s32 a, s32 b) {
    diveSasAttack(p, msg, a, b);
}

/* Hit under water (Sasuke). */
void diveSasHit(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    switch (msg) {
        case 0:
            diveDefault(p, msg, a, b);
            if (plDamage(p, p->damage, p->weaponDamage)) {
                plActChange(p, 0x5B);
                return;
            }
            plInvincibleSet(p, 0x4B);
            plVoiceDamage(p);
            break;
        case 1:
            if (plAnimEnded(p)) {
                next = 0x56;
            }
            plActChange(p, next);
            return;
        case 2:
            plInvincibleSet(p, 0x4B);
            plBlinkSet(p, 0x4B);
            diveDefault(p, msg, a, b);
            break;
        case 3:
            break;
    }
    plActNoneDefault(p, msg, a, b);
}

/* Eating under water (Sasuke). */
void diveSasEat(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 1:
            plLeanTo(p, 0.13089968f); /* pi/24, a bit under */
            if (p->actionSteps == 0x3C) {
                p->onigiri = plOnigiriCreate(p);
            }
            return;
        case 0:
            plBlinkSet(p, 0);
            p->flags |= 0x200;
            p->targetLean = 3.1415927f;
            plSound(p, 0x114);
            plEatVoice(p);
            plHumStop(p);
            break;
        case 2:
            p->lean = 0.0f;
            p->flags &= ~0x200;
            p->targetLean = 0.0f;
            break;
    }
    diveDefault(p, msg, a, b);
}

/* The screw ("screw"): follows Sasuke's back, a quarter bigger, and spins with his swimming. */
s32 sasScrew(Obj *obj, s16 msg, void *arg) {
    PlScrew *w = SCREW(obj);
    Player *pl;
    PlActAnim *anim;
    f32 end;
    s32 pad;
    f32 x, y, z;

    switch (msg) {
        case 2:
            obj->name = "screw";
            w->frame = frmCreate(gFrmModelLayer, gFrmKindAnime);
            w->frame->anim = sasScrewAnim;
            w->frame->segs[1] = fileRamAddr(0x1AA);
            w->index = *(s32 *) arg;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                0.1f;
            w->speed = 0.0f;
            break;
        case 0:
            if (!plIsInPlay(w->index)) {
                return -1;
            }
            pl = PL(gGameWork.players[w->index]);
            if (!(pl->flags & 0x10000)) {
                return -1;
            }
            anim = plActAnimeGet(pl);
            if (anim->attach == 0) {
                PRINTF("sasScrew: うそはこモデルがありません(自爆します)\n");
                return -1;
            }
            mdlAnimNodePos(anim->attach, pl->model->frame, pl->model->segs[0], &x, &y, &z);
            w->frame->pos[0] = pl->model->pos[0];
            w->frame->pos[1] = pl->model->pos[1];
            w->frame->pos[2] = pl->model->pos[2];
            w->frame->rot[0] = pl->model->rot[0];
            w->frame->rot[1] = pl->model->rot[1];
            w->frame->rot[2] = pl->model->rot[2];
            w->frame->scale[0] = pl->model->scale[0] * 1.25f;
            w->frame->scale[1] = pl->model->scale[1] * 1.25f;
            w->frame->scale[2] = pl->model->scale[2] * 1.25f;
            w->speed += sqrtf(pl->lastPushY * pl->lastPushY + pl->lastPushX * pl->lastPushX);
            w->frame->frame += w->speed;
            end = polAnimeLastFrame(w->frame);
            if (end < w->frame->frame) {
                w->frame->frame -= end;
            }
            if (w->speed > 2.5f) {
                w->speed = 2.5f;
            } else if (w->speed < 0.8f && !(pl->flags & 0x200)) {
                w->speed = 0.8f;
            }
            w->speed *= 0.95f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

void diveScrewSetPos(Player *p, Obj *screw, s32 unused) {
    SCREW(screw)->frame->pos[0] = p->model->pos[0];
    SCREW(screw)->frame->pos[1] = p->model->pos[1];
    SCREW(screw)->frame->pos[2] = p->model->pos[2];
}

/* The screw's spin, 0 without one. */
f32 diveScrewSpin(Obj *obj) {
    if (obj != NULL) {
        return SCREW(obj)->speed;
    }
    return 0.0f;
}

/* Starts a stage in dive mode. */
void __diveStart(Player *p) {
    if (p->chara != 3 && p->chara != 2) {
        PRINTF("__diveStart(): only sas and yae can start in dive mode\n");
        p->flags &= ~0x10000;
        plActChange(p, 0x18);
        return;
    }
    if (p->flags & 0x10000) {
        switch (p->chara) {
            case 2:
                p->action = 0x56;
                diveSasBegin(p);
                break;
            case 3:
                p->action = 0x47;
                diveYaeBegin(p);
                break;
        }
        plAnimeSetCurAction(p);
        plFaceAnimeStart(p);
    }
}

void diveNonDiverWait(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            plInvincibleSet(p, -1);
            break;
        case 1:
            return;
        case 2:
            plInvincibleSet(p, 0);
            break;
    }
    plBaseDefault(p, msg, a, b);
}
