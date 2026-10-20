#include "tlb.h"

/* The "skerrior" enemy kind (archive file 154): a skeleton warrior. It stands swaying until a
 * player is within 60, turns to face it, walks towards it and, near it, breathes a "fire" (kind
 * 0x20E, the second proc here) along its way. A wall sends it walking back and turning again; a
 * hit flashes it and knocks it back. It fades in while it attacks and out otherwise. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 skerriorFireDl[];
extern u8 etskSkerriorAttackAnim[];
extern u8 etskSkerriorWalkAnim[];
extern u8 etskSkerriorState6Anim[];
extern u8 etskSkerriorHalf0Dl[];
extern u8 etskSkerriorHalf1Dl[];
extern u8 skerriorFireTex0[];
extern u8 skerriorFireTex1[];
extern u8 skerriorFireTex2[];
extern u8 skerriorFireTex7[];
extern u8 skerriorFireTex14[];
AnimFrame skerriorFireAnim[] = {
    { (u32) skerriorFireTex0, 1 },
    { (u32) skerriorFireTex1, 1 },
    { (u32) skerriorFireTex2, 1 },
    { 0x030018D0, 1 },
    { 0x030020D0, 1 },
    { 0x030028D0, 1 },
    { 0x030030D0, 1 },
    { (u32) skerriorFireTex7, 1 },
    { 0x030040D0, 1 },
    { 0x030048D0, 1 },
    { 0x030050D0, 1 },
    { 0x030058D0, 1 },
    { 0x030060D0, 1 },
    { 0x030068D0, 1 },
    { (u32) skerriorFireTex14, 1 },
    { 0x030078D0, 1 },
    { 0, 0 },
    { 0, 0 },
};

/* A skerrior's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Skerrior {
    /* 0x00 */ EneObj e;
    /* 0x4C */ f32 speed; /* the walk's, up to 0.8 (negative walking back) */
    /* 0x50 */ u8 way; /* the way it faces along x: 1 or 4 (enePlDirX) */
    /* 0x51 */ u8 alpha; /* the env colour's first value while not flashing */
    /* 0x52 */ s8 flash; /* frames left of the hit flash */
    /* 0x53 */ s8 dir; /* 1 or -1: the sway's way */
} Skerrior;

/* A fire's work: an enemy's, its texture animation, a timer and its alpha. */
typedef struct SkerriorFire {
    /* 0x00 */ EneObj e;
    /* 0x4C */ Anim anim;
    /* 0x5C */ u8 pad5C[2];
    /* 0x5E */ s8 timer;
    /* 0x5F */ u8 alpha;
} SkerriorFire;

#define SKERRIOR(obj) ((Skerrior *) FX(obj))
#define FIRE(obj) ((SkerriorFire *) FX(obj))
#define MODEL(w) ((AnimeFrame *) (w)->e.model)
#define POL(w) ((PolFrame *) (w)->e.model)

/* A frame destroyed if it was made. */
#define FRM_DESTROY(f) if ((f) != NULL) { frmDestroy((Frame *) (f)); }

void skerriorWait(Obj *o);
void skerriorWalk(Obj *o);
void skerriorWalkStop(Obj *o);
void skerriorAttack(Obj *o);
void skerriorTurn(Obj *o);
void skerriorWallBack(Obj *o);
void skerriorKnockback(Obj *o);

s32 Etsk_skerrior_main(Obj *o, s16 msg, void *arg) {
    Skerrior *w = SKERRIOR(o);
    ACollFrame *a;
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "skerrior";
            eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneAnimeModelMake(o, (u32) etskSkerriorState6Anim, NULL, 0)) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 65, 100, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((a = ATK_ADD_YZ((Frame *) w->e.atk, w->e.model, 40, 250, 40, 0)) == NULL) {
                return -1;
            }
            a->kind = 120;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 90, 330, 0, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->e.shadow = (Obj *) shadowCreate()) == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                return -1;
            }
            w->e.halves[0] = (u32) etskSkerriorHalf0Dl;
            w->e.halves[1] = (u32) etskSkerriorHalf1Dl;
            w->e.behave = 0x8404A82;
            w->flash = 0;
            w->dir = 1;
            w->speed = 0.0f;
            w->e.rot[1] = eneCourseAngle(o, 8);
            w->e.hp = 3;
            w->e.state = 0;
            w->alpha = 0;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*skerriorWait)(o);
                    break;
                case 1:
                    (*skerriorWalk)(o);
                    break;
                case 2:
                    (*skerriorWalkStop)(o);
                    break;
                case 3:
                    (*skerriorAttack)(o);
                    break;
                case 4:
                    (*skerriorTurn)(o);
                    break;
                case 5:
                    (*skerriorWallBack)(o);
                    break;
                case 6:
                    (*skerriorKnockback)(o);
                    break;
            }
            if (w->e.state == 3) {
                if (w->alpha + 16 >= 256) {
                    w->alpha = 255;
                } else {
                    w->alpha = w->alpha + 16;
                }
            } else {
                if (w->alpha - 16 < 0) {
                    w->alpha = 0;
                } else {
                    w->alpha = w->alpha - 16;
                }
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_skerrior_main: 死亡\n");
                return -1;
            }
            if (w->e.hit & 0x10) {
                MODEL(w)->anim = etskSkerriorState6Anim;
                MODEL(w)->frame = 0.0f;
                w->e.dmg->flags &= ~2;
                w->flash = 12;
                w->e.behave &= ~0x4000;
                w->e.vel[0] = eneKnockbackSpeed(o, w->e.dmg, 4.0f);
                w->way = (w->e.vel[0] < 0.0f) ? 1 : 4;
                w->e.state = 6;
            }
            if (w->flash - 1 >= 0) {
                w->flash = w->flash - 1;
            } else {
                w->flash = 0;
            }
            /* fallthrough */
        case 3:
            if (w->flash != 0) {
                MODEL(w)->dl = eneDlEnvColorLit(o, (w->flash & 2) ? 0x80 : 0, 0xFF, 0xFF, 0xFF,
                                                (w->flash & 2) ? 0 : 1);
            } else {
                MODEL(w)->dl = eneDlEnvColorLit(o, w->alpha, 0xFF, 0xFF, 0xFF, 1);
            }
            break;
        case 1:
            FRM_DESTROY(w->e.model);
            FRM_DESTROY(w->e.atk);
            FRM_DESTROY(w->e.dmg);
            FRM_DESTROY(w->e.shadow);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* State 1: walks along its way, speeding up to 0.8 (20/9 of it a step); facing the player and
 * within 35 of it, it stops at the step's middle (frame 10: state 2) or attacks at its start
 * (frame 0: state 3). A wall sends it back (5). */
void skerriorWalk(Obj *o) {
    Skerrior *w = SKERRIOR(o);
    s32 pad;

    w->e.behave |= 0x4000;
    w->speed += 0.05;
    if (w->speed >= 0.8) {
        w->speed = 0.8f;
    }
    w->e.vel[0] = w->speed * 20.0f / 9.0f;
    if (eneFaceWay(o) == 4) {
        w->e.vel[0] *= -1.0f;
    }
    if (eneFaceWay(o) == enePlDirX(o, 0)) {
        if (MODEL(w)->frame >= 10.0f && MODEL(w)->frame < 10.0f + w->speed) {
            if (enePlInRangeCheck(o, 35.0f, 0)) {
                MODEL(w)->frame = 10.0f;
                w->e.state = 2;
                w->speed = 0.0f;
            }
        } else if (MODEL(w)->frame >= 0.0f && MODEL(w)->frame < w->speed) {
            if (enePlInRangeCheck(o, 35.0f, 0)) {
                MODEL(w)->anim = etskSkerriorWalkAnim;
                MODEL(w)->frame = 0.0f;
                w->e.vel[0] = 0.0f;
                w->e.state = 3;
            }
        }
    }
    eneAnimeLoopLen(o, MODEL(w), w->speed, 20.0f);
    if (w->e.hit & 6) {
        w->speed = 0.0f;
        w->e.state = 5;
    }
}

/* State 2: plays the walk backwards, slowing to -0.8, to its first frame, then attacks (3). */
void skerriorWalkStop(Obj *o) {
    Skerrior *w = SKERRIOR(o);
    s32 pad;

    w->e.behave &= ~0x4000;
    w->speed -= 0.05;
    if (w->speed <= -0.8) {
        w->speed = -0.8f;
    }
    w->e.vel[0] = w->speed * 20.0f / 9.0f;
    if (eneFaceWay(o) == 4) {
        w->e.vel[0] *= -1.0f;
    }
    MODEL(w)->frame += w->speed;
    if (MODEL(w)->frame <= 0.0f) {
        MODEL(w)->anim = etskSkerriorWalkAnim;
        MODEL(w)->frame = 0.0f;
        w->speed = 0.0f;
        w->e.vel[0] = 0.0f;
        w->e.state = 3;
    }
}

/* State 3: the attack: a sound (0x213) at frame 6, the fire's sound (0x221) and the fire itself
 * 35 ahead at frame 7 (the animation slows from frame 8); then it walks again (1). */
void skerriorAttack(Obj *o) {
    Skerrior *w = SKERRIOR(o);
    f32 dx;
    f32 step = 0.4f;

    if (MODEL(w)->frame >= 8.0) {
        step = 0.25f;
    }
    if (MODEL(w)->frame >= 6.0 && MODEL(w)->frame < 6.0 + step) {
        eneSetSd2(o, 0x213, 0, 0);
    }
    if (MODEL(w)->frame >= 7.0 && MODEL(w)->frame < 7.0 + step) {
        eneSetSd2(o, 0x221, 0, 0);
        dx = (eneFaceWay(o) == 1) ? 35.0f : -35.0f;
        eneMakeEx(o, ENE_SKERRIOR_FIRE_MAIN, w->e.zone, w->e.x + dx, w->e.y + 5.0f, w->e.z, 0, NULL);
    }
    if (eneAnimPlayOnce(o, MODEL(w), step)) {
        MODEL(w)->anim = etskSkerriorAttackAnim;
        MODEL(w)->frame = 0.0f;
        if (w->way == 1) {
            w->e.vel[0] = 1.0f;
        } else {
            w->e.vel[0] = -1.0f;
        }
        w->e.state = 1;
    }
}

/* State 4: turns towards its way, swaying; turned and back at the sway's start, it walks (1). */
void skerriorTurn(Obj *o) {
    Skerrior *w = SKERRIOR(o);
    s32 pad[2];
    s32 turning;
    f32 step;

    step = (w->way == 1) ? 0.08 : -0.08;
    turning = eneTurnRotYTo(o, eneCourseAngle(o, w->way), step);
    if (eneAnimeStepTo(o, MODEL(w), w->dir * 0.06f, 0.4f)) {
        if (MODEL(w)->frame == 0.0 && turning == 0) {
            MODEL(w)->anim = etskSkerriorAttackAnim;
            MODEL(w)->frame = 0.0f;
            w->e.state = 1;
        } else {
            w->dir = -w->dir;
        }
    }
}

/* State 0: sways until a player is within 60, then turns to it (4). */
void skerriorWait(Obj *o) {
    Skerrior *w = SKERRIOR(o);
    s32 pad[2];

    if (eneAnimeStepTo(o, MODEL(w), w->dir * 0.06f, 0.4f)) {
        w->dir = -w->dir;
    }
    if (enePlInRangeCheck(o, 60.0f, 0)) {
        w->way = enePlDirX(o, 0);
        w->e.state = 4;
    }
}

/* State 5: after a wall, walks back to the step's first frame, turned the other way, then
 * turns (4). */
void skerriorWallBack(Obj *o) {
    Skerrior *w = SKERRIOR(o);
    s32 pad;

    w->e.behave &= ~0x4000;
    w->speed -= 0.05;
    if (w->speed <= -0.8) {
        w->speed = -0.8f;
    }
    w->e.vel[0] = w->speed * 20.0f / 9.0f;
    if (eneFaceWay(o) == 4) {
        w->e.vel[0] *= -1.0f;
        w->way = 1;
    } else {
        w->way = 4;
    }
    MODEL(w)->frame += w->speed;
    if (MODEL(w)->frame <= 0.0f) {
        MODEL(w)->anim = etskSkerriorState6Anim;
        MODEL(w)->frame = 0.0f;
        w->speed = 0.0f;
        w->e.vel[0] = 0.0f;
        w->e.state = 4;
    }
}

/* State 6: knocked back; faces its way, slows down, and when the hit animation is over it
 * walks again (1). */
void skerriorKnockback(Obj *o) {
    Skerrior *w = SKERRIOR(o);
    s32 pad;
    f32 step;

    step = (MODEL(w)->frame < 1.0) ? 0.5 : 0.1;
    w->e.rot[1] = eneCourseAngle(o, w->way);
    if (w->e.vel[0] < 0.0f) {
        w->e.vel[0] += 0.363636363636363646;
    } else if (w->e.vel[0] > 0.0f) {
        w->e.vel[0] -= 0.363636363636363646;
    }
    if (eneAnimPlayOnce(o, MODEL(w), step)) {
        MODEL(w)->anim = etskSkerriorAttackAnim;
        MODEL(w)->frame = 0.0f;
        w->e.dmg->flags |= 2;
        w->e.behave |= 0x4000;
        w->speed = 0.0f;
        w->e.state = 1;
    }
}

/* "fire": the skerrior's breath, a flame that grows 2.5% a step; after 16 steps it sets its
 * behave's 0x40000 for eneAfterProc. */
s32 skerriorFireMain(Obj *o, s16 msg, void *arg) {
    SkerriorFire *w = FIRE(o);

    switch (msg) {
        case 2:
            o->name = "fire";
            eneSetup(o, arg);
            if ((w->e.model = (Frame *) enePolModelMake(o, (Gfx *) skerriorFireDl, NULL, 1)) == NULL) {
                return -1;
            }
            floadGetSetRam((void **) POL(w)->segs, 0x121);
            POL(w)->scale[2] = 0.3f;
            POL(w)->scale[0] = POL(w)->scale[1] = POL(w)->scale[2];
            POL(w)->flags = 2;
            fadeAnimStart(&w->anim, skerriorFireAnim, 0, &POL(w)->segs[3], floadGetSetFile(0x121, 0));
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 35, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 123;
            w->alpha = 255;
            w->timer = 16;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            fadeAnimStep(&w->anim);
            POL(w)->scale[0] *= 1.025;
            POL(w)->scale[1] = POL(w)->scale[2] = POL(w)->scale[0];
            if (w->timer - 1 <= 0) {
                w->e.behave |= 0x40000;
            } else {
                w->timer = w->timer - 1;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_skerrior_fire: 死亡\n");
                return -1;
            }
            /* fallthrough */
        case 3:
            POL(w)->dl = eneDlEnvPrimColor(o, 0xBF, 0xFF, 0xFF, 0, 0x3F, 0x3F, w->alpha);
            break;
        case 1:
            FRM_DESTROY(w->e.model);
            FRM_DESTROY(w->e.atk);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
