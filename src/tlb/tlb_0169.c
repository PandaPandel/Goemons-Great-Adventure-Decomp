#include "tlb.h"

typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                    s32 a);

#define DPRINTF

/* "haita" (its proc Etsk_haita_main) and its steps. */
typedef struct HaitaW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 unused09;
    /* 0x0A */ u8 flags; /* 0x10: stopped dead */
    /* 0x0B */ u8 hp;
    /* 0x0C */ AnimeFrame *anim;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 spin; /* while it leaps */
    /* 0x20 */ f32 ry; /* its heading */
    /* 0x24 */ f32 rotZ;
    /* 0x28 */ f32 vel;
    /* 0x2C */ f32 vy;
    /* 0x30 */ f32 vz;
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ MPolFrame *shadow;
    /* 0x40 */ u8 *halves[2]; /* its two halves' display lists, for a cut: etskHaitaUnk40Dl, etskHaitaUnk44Dl */
    /* 0x48 */ u32 behave; /* EneObj's (enemy.h) */
    /* 0x4C */ f32 leapAngle;
    /* 0x50 */ f32 prevX;
    /* 0x54 */ s16 startX; /* where it stood when it turned */
    /* 0x56 */ s16 startY;
    /* 0x58 */ s16 baseY; /* the height it rises from */
    /* 0x5A */ s16 baseX; /* the place it slides from */
    /* 0x5C */ u8 alpha;
    /* 0x5D */ u8 dir;
    /* 0x5E */ u8 flash; /* frames of its flash */
    /* 0x5F */ u8 wait;
} HaitaW;

void haitaWait(Obj *o);
void haitaRise(Obj *o);
void haitaSlide(Obj *o);
void haitaSlideStop(Obj *o);
void haitaThrowTama1(Obj *o);
void haitaTurn(Obj *o);
void haitaLeap(Obj *o);
void haitaThrowTama2(Obj *o);
void haitaWaitTurn(Obj *o);
void haitaRiseTurn(Obj *o);
void haitaStunned(Obj *o);
extern u8 etskHaitaAnim[]; /* its animations */
extern u8 etskHaitaState10Anim[];
extern u8 etskHaitaUnk40Dl[];
extern u8 etskHaitaUnk44Dl[];

/* "haita", the leaping enemy: an animated model with an attack box (radius 55, 80 up, kind 0x78),
 * a damage box (100 by 160, 20 up) and a shadow, set on the ground, facing the course one way or
 * the other by which side the nearest player is on. Each frame its state's step runs. A hit
 * swaps in its hurt animation; with flag 0x10 it stops dead (state 10) and flashes for 12
 * frames, its damage box off. */
s32 Etsk_haita_main(Obj *o, s16 msg, void *arg);
s32 Etsk_haita_main(Obj *o, s16 msg, void *arg) {
    HaitaW *w = (HaitaW *) FX(o);
    s32 n;
    s32 a;
    s32 lit;

    switch (msg) {
        case 2:
            o->name = "haita";
            eneSetup(o, arg);
            if ((w->anim = eneAnimeModelMake(o, (u32) etskHaitaAnim, NULL, 0)) == NULL) {
                return -1;
            }
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->anim->f, 55, 80, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 0x78;
            if ((w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->anim->f, 100, 160, 20, 0, 0)) ==
                NULL) {
                return -1;
            }
            if ((w->shadow = shadowCreate()) == NULL) {
                return -1;
            }
            w->halves[0] = etskHaitaUnk40Dl;
            w->halves[1] = etskHaitaUnk44Dl;
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("Etsk_haita: 地面にセット失敗\n");
                return -1;
            }
            w->alpha = 0;
            w->flash = 0;
            w->baseX = w->x;
            w->baseY = w->y;
            w->behave = 0x400002;
            w->hp = 1;
            w->dir = enePlDirX(o, 0);
            w->ry = eneCourseAngle(o, (w->dir == 4) ? 1 : 4);
            w->state = 0;
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->state) {
                case 0:
                    (*haitaWait)(o);
                    break;
                case 1:
                    (*haitaRise)(o);
                    break;
                case 2:
                    (*haitaSlide)(o);
                    break;
                case 3:
                    (*haitaSlideStop)(o);
                    break;
                case 4:
                    (*haitaThrowTama1)(o);
                    break;
                case 5:
                    (*haitaTurn)(o);
                    break;
                case 6:
                    (*haitaLeap)(o);
                    break;
                case 7:
                    (*haitaThrowTama2)(o);
                    break;
                case 8:
                    (*haitaWaitTurn)(o);
                    break;
                case 9:
                    (*haitaRiseTurn)(o);
                    break;
                case 10:
                    (*haitaStunned)(o);
                    break;
            }
            if (acollDmgWasHit(w->dmg) == 1) {
                w->anim->anim = etskHaitaState10Anim;
                w->anim->frame = 0.0f;
            }
            if (eneAfterProc(o) == -1) {
                DPRINTF("Etsk_haita_main: 死亡\n");
                return -1;
            }
            if (w->flags & 0x10) {
                w->flash = 12;
                w->anim->anim = etskHaitaState10Anim;
                w->anim->frame = 0.0f;
                w->vel = 0.0f;
                w->vy = 0.0f;
                w->spin = 0.0f;
                w->behave |= 0x800000;
                if (w->state != 10) {
                    w->wait = 10;
                }
                w->state = 10;
            }
            if ((n = w->flash - 1) > 0) {
                w->flash = n;
                w->dmg->flags &= ~2;
            } else {
                w->flash = 0;
                w->dmg->flags |= 2;
            }
        case 3:
            if (w->flash != 0) {
                a = (w->flash & 2) ? 0x80 : 0;
                lit = (w->flash & 2) ? 0 : 1;
                w->anim->dl = eneDlEnvColorLit(o, a, 255, 255, 255, lit);
            } else {
                w->anim->dl = eneDlEnvColorLit(o, w->alpha, 255, 255, 255, 1);
            }
            break;
        case 1:
            if (w->anim != NULL) { frmDestroy(&w->anim->f); }
            if (w->atk != NULL) { frmDestroy(&w->atk->frame); }
            if (w->dmg != NULL) { frmDestroy(&w->dmg->frame); }
            if (w->shadow != NULL) { frmDestroy((Frame *) w->shadow); }
            break;
    }
    /* A step's print (one that changes vel, haitaSlide or haitaSlideStop; which is not known):
     * its string comes next in the .rodata. */
    DPRINTF("vx %f\n");
    return tskDefaultProc(o, msg, arg);
}

extern u8 etskHaitaWaitAnim[]; /* an animation in its segment */

/* Loops its animation; once a player is within 70, it starts animation etskHaitaWaitAnim and goes to
 * state 1. */
void haitaWait(Obj *o) {
    HaitaW *w = (HaitaW *) FX(o);

    eneAnimLoop(o, w->anim, 1.0f);
    if (enePlInRangeCheck(o, 70.0f, 0)) {
        w->anim->anim = etskHaitaWaitAnim;
        w->anim->frame = 0.0f;
        w->state = 1;
        w->vy = 3.5f;
    }
}

/* Loops its animation at speed 3 (sound 0x2CB as frame 3 passes); once 25 above its base
 * height, it slows by 0.35 a frame and at 0 goes to state 2. */
void haitaRise(Obj *o) {
    s32 pad;
    HaitaW *w = (HaitaW *) FX(o);
    f32 step = 3.0f;

    eneAnimLoop(o, w->anim, step);
    if (3.0 <= w->anim->frame && w->anim->frame < 3.0 + step) {
        eneSetSd2(o, 0x2CB, 0, 0);
    }
    if (w->y - w->baseY >= 25.0f) {
        w->vy -= 0.35;
        if (w->vy <= 0.0f) {
            w->vy = 0.0f;
            w->state = 2;
        }
    }
}

#pragma intrinsic(fabs)

/* Loops its animation at speed 2 (sound 0x2CB as frame 3 passes). Within 25 of where it slides
 * from it turns to the course (as haitaTurn) and speeds up by 0.5 a frame in its
 * direction; at 3 a frame, or once beyond 25, it goes to state 3. */
void haitaSlide(Obj *o) {
    HaitaW *w = (HaitaW *) FX(o);
    f32 step = 2.0f;

    eneAnimLoop(o, w->anim, step);
    if (3.0 <= w->anim->frame && w->anim->frame < 3.0 + step) {
        eneSetSd2(o, 0x2CB, 0, 0);
    }
    if (fabs(w->x - w->baseX) < 25.0) {
        if (w->dir == 4) {
            eneTurnRotYTo(o, eneCourseAngle(o, 4) + 0.3926991f, -0.5f);
            w->vel += 0.5;
            if (w->vel >= 3.0) {
                w->vel = 3.0f;
                w->state = 3;
            }
        } else {
            eneTurnRotYTo(o, eneCourseAngle(o, 1) - 0.3926991f, 0.5f);
            w->vel -= 0.5;
            if (w->vel <= -3.0) {
                w->vel = -3.0f;
                w->state = 3;
            }
        }
    } else {
        w->state = 3;
    }
}

/* Loops its animation at speed 2 (sound 0x2CB as frame 3 passes); once 25 from where it slides
 * from, it waits 20 and slows by 0.3 a frame in its direction, and at a stop goes to state 4. */
void haitaSlideStop(Obj *o) {
    s32 pad;
    HaitaW *w = (HaitaW *) FX(o);
    f32 step = 2.0f;

    eneAnimLoop(o, w->anim, step);
    if (3.0 <= w->anim->frame && w->anim->frame < 3.0 + step) {
        eneSetSd2(o, 0x2CB, 0, 0);
    }
    if (fabs(w->x - w->baseX) >= 25.0) {
        w->wait = 20;
        if (w->dir == 4) {
            w->vel -= 0.3;
            if (w->vel <= 0.0) {
                w->state = 4;
                w->vel = 0.0f;
            }
        } else {
            w->vel += 0.3;
            if (w->vel >= 0.0) {
                w->state = 4;
                w->vel = 0.0f;
            }
        }
    }
}

/* Loops its animation (sound 0x2CB as frame 3 passes) while its wait runs down. When the wait
 * reaches 15 with the camera within 90, it makes two enemies 0x228 (sound 0x157 each) 5 up
 * and 5 to its front, passing each its direction and its number (mode 2): the argument's two
 * halves are s16s, cast so. When the wait runs out it turns to the nearer player and goes to
 * state 5. */
void haitaThrowTama1(Obj *o) {
    HaitaW *w = (HaitaW *) FX(o);
    f32 step = 1.0f;
    s32 wait;
    s32 i;

    eneAnimLoop(o, w->anim, step);
    if (3.0 <= w->anim->frame && w->anim->frame < 3.0 + step) {
        eneSetSd2(o, 0x2CB, 0, 0);
    }
    wait = w->wait - 1;
    if (wait >= 0) {
        w->wait = wait;
        if (w->wait == 15 && eneCamNearCheck(o, 90.0f)) {
            for (i = 0; i < 2; i++) {
                eneSetSd(o, 0x157);
                if (w->dir == 4) {
                    eneMakeEx(NULL, ENE_ETSK_HAITA_TAMA1, w->zone, w->x - 5.0f, w->y + 5.0f, w->z, 2,
                              (u32 (*)(void *, void *, void *)) (((s16) w->dir & 0xFFFF) | (((s16) i << 16) & 0xFFFF0000)));
                } else if (w->dir == 1) {
                    eneMakeEx(NULL, ENE_ETSK_HAITA_TAMA1, w->zone, w->x + 5.0f, w->y + 5.0f, w->z, 2,
                              (u32 (*)(void *, void *, void *)) (((s16) w->dir & 0xFFFF) | (((s16) i << 16) & 0xFFFF0000)));
                }
            }
        }
    } else {
        w->wait = 0;
        w->dir = enePlDirX(o, 0);
        w->state = 5;
    }
}

extern u8 etskHaitaTurnAnim[]; /* an animation in its segment */

/* Loops its animation at speed 2 (sound 0x2CB as frame 3 passes) while it turns to the course,
 * 22.5 degrees ahead or behind by its direction. Turned, it starts animation etskHaitaTurnAnim, keeps
 * where it stands, and goes to state 6. */
void haitaTurn(Obj *o) {
    HaitaW *w = (HaitaW *) FX(o);
    f32 step = 2.0f;
    s32 turning;

    eneAnimLoop(o, w->anim, step);
    if (3.0 <= w->anim->frame && w->anim->frame < 3.0 + step) {
        eneSetSd2(o, 0x2CB, 0, 0);
    }
    if (w->dir == 4) {
        turning = eneTurnRotYTo(o, eneCourseAngle(o, 4) + 0.3926991f, -0.5f);
    } else {
        turning = eneTurnRotYTo(o, eneCourseAngle(o, 1) - 0.3926991f, 0.5f);
    }
    if (!turning) {
        w->anim->anim = etskHaitaTurnAnim;
        w->anim->frame = 0.0f;
        w->startX = w->x;
        w->startY = w->y;
        w->leapAngle = -1.5707964f;
        w->state = 6;
        w->prevX = w->x;
    }
}

#define sinf (*sinf)
#define cosf (*cosf)

/* The leap: from where it stood, 60 across (its way) and up in a loop of 15, by an angle that
 * grows 0.12 a frame while rising (spinning with the distance covered) and 0.08 falling (its
 * animation etskHaitaWaitAnim at speed 3). Down at three quarters of a turn it faces the player and
 * goes to state 7 at -2. Sound 0x2CB as its animation's frame 3 passes. The angle is a local a;
 * each "step =" comes after its "leapAngle +=" (the game loads 1.0 and 3.0 late); the move's
 * difference is a local d, worked out before "step = 1.0f". */
void haitaLeap(Obj *o) {
    HaitaW *w = (HaitaW *) FX(o);
    f32 a;
    f32 step;
    f32 d;
    s32 pad[2];

    if (w->dir == 4) {
        w->x = w->startX - (sinf(w->leapAngle) * 60.0f / 2 + 30.0f);
    } else {
        w->x = sinf(w->leapAngle) * 60.0f / 2 + 30.0f + w->startX;
    }
    w->y = w->startY - (sinf(w->leapAngle) * 15.0f + 15.0f);
    w->y -= cosf(w->leapAngle) * 15.0f;
    a = w->leapAngle;
    if (a >= -1.5707964f && a < 1.5707964f) {
        w->leapAngle += 0.12;
        d = w->x - w->prevX;
        step = 1.0f;
        w->spin = fabsf(d) / 4.3 * 3.1415927410125732 / 3.0;
    } else {
        w->anim->anim = etskHaitaWaitAnim;
        w->leapAngle += 0.08;
        step = 3.0f;
        if (w->leapAngle >= 4.71238911151886) {
            w->dir = enePlDirX(o, 0);
            w->state = 7;
            w->vy = -2.0f;
        }
    }
    eneAnimLoop(o, w->anim, step);
    if (3.0 <= w->anim->frame && w->anim->frame < 3.0 + step) {
        eneSetSd2(o, 0x2CB, 0, 0);
    }
    w->prevX = w->x;
}

/* Loops its animation at speed 2 (sound 0x2CB as frame 3 passes) while it turns to the course.
 * Once 30 from its start height it slows by 2 a frame; stopped, it keeps its x, and with the
 * camera within 90 makes an enemy 0x229 (sound 0x164) 10 to its front and 14 up, passing it
 * its direction (mode 2); then it waits 25 in state 8. */
void haitaThrowTama2(Obj *o) {
    HaitaW *w = (HaitaW *) FX(o);
    f32 step = 2.0f;

    eneAnimLoop(o, w->anim, step);
    if (3.0 <= w->anim->frame && w->anim->frame < 3.0 + step) {
        eneSetSd2(o, 0x2CB, 0, 0);
    }
    if (w->dir == 4) {
        eneTurnRotYTo(o, eneCourseAngle(o, 4) + 0.3926991f, -0.5f);
    } else {
        eneTurnRotYTo(o, eneCourseAngle(o, 1) - 0.3926991f, 0.5f);
    }
    if (fabs(w->y - w->startY) >= 30.0) {
        w->vy -= 2.0;
        if (w->vy <= 0.0f) {
            w->vy = 0.0f;
            w->startX = w->x;
            if (eneCamNearCheck(o, 90.0f)) {
                eneSetSd(o, 0x164);
                if (w->dir == 4) {
                    eneMakeEx(NULL, ENE_ETSK_HAITA_TAMA2, w->zone, w->x - 10.0f, w->y + 14.0f, w->z, 2,
                              (u32 (*)(void *, void *, void *)) w->dir);
                } else {
                    eneMakeEx(NULL, ENE_ETSK_HAITA_TAMA2, w->zone, w->x + 10.0f, w->y + 14.0f, w->z, 2,
                              (u32 (*)(void *, void *, void *)) w->dir);
                }
            }
            w->wait = 25;
            w->state = 8;
        }
    }
}

/* Loops its animation (sound 0x2CB as frame 3 passes); when its wait runs out it turns to the
 * nearer player and goes to state 9 at speed 3.5. */
void haitaWaitTurn(Obj *o) {
    HaitaW *w = (HaitaW *) FX(o);
    f32 step = 1.0f;
    s32 wait;

    eneAnimLoop(o, w->anim, step);
    if (3.0 <= w->anim->frame && w->anim->frame < 3.0 + step) {
        eneSetSd2(o, 0x2CB, 0, 0);
    }
    wait = w->wait - 1;
    if (wait >= 0) {
        w->wait = wait;
    } else {
        w->wait = 0;
        w->dir = enePlDirX(o, 0);
        w->state = 9;
        w->vy = 3.5f;
    }
}

/* haitaRise while turning to the course as haitaTurn does: loops its
 * animation at speed 3, and once 25 above its base height slows by 0.35 a frame, at 0 going to
 * state 2. */
void haitaRiseTurn(Obj *o) {
    HaitaW *w = (HaitaW *) FX(o);
    f32 step = 3.0f;

    eneAnimLoop(o, w->anim, step);
    if (3.0 <= w->anim->frame && w->anim->frame < 3.0 + step) {
        eneSetSd2(o, 0x2CB, 0, 0);
    }
    if (w->dir == 4) {
        eneTurnRotYTo(o, eneCourseAngle(o, 4) + 0.3926991f, -0.5f);
    } else {
        eneTurnRotYTo(o, eneCourseAngle(o, 1) - 0.3926991f, 0.5f);
    }
    if (w->y - w->baseY >= 25.0f) {
        w->vy -= 0.35;
        if (w->vy <= 0.0f) {
            w->vy = 0.0f;
            w->state = 2;
        }
    }
}

/* On the ground (hit bit 1) and with flash clear, it counts its wait down; at the end it drops
 * behave bit 0x800000, starts animation etskHaitaWaitAnim, turns to the player and goes to state 9. */
void haitaStunned(Obj *o) {
    HaitaW *w = (HaitaW *) FX(o);
    s32 n;

    if (ENE(o)->hit & 1) {
        if (w->flash == 0) {
            n = w->wait - 1;
            if (n >= 0) {
                w->wait = n;
            } else {
                w->wait = 0;
                w->behave &= ~0x800000;
                w->anim->anim = etskHaitaWaitAnim;
                w->anim->frame = 0.0f;
                w->dir = enePlDirX(o, 0);
                w->state = 9;
                w->vy = 3.5f;
            }
        }
    }
}


/* "tama2"'s work: EneObj's start, then its attack box. */
typedef struct HaitaTamaW {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 unused09;
    /* 0x0A */ u8 hit; /* EneObj's */
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rotX;
    /* 0x20 */ f32 rotY;
    /* 0x24 */ f32 rot;
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ u8 pad38[0x10];
    /* 0x48 */ u32 behave; /* EneObj's */
} HaitaTamaW;

extern Gfx etskHaitaTama1Dl[]; /* the small shot */

/* "tama1", one of the haita's spread shots: eneSetup gives its number (high half) and way; it
 * flies at 4 at an angle of a fifth of a turn per number (from 198 degrees its way, from -18
 * the other), turned to match, its attack box carrying its speed. It flashes every other frame
 * and is gone (behave 0x4000000) when hit. Its number and way are masked out of the setup value
 * ("no" first): the masks' dead andi spend the temporaries the game spends. FAKE: msg 1's tests
 * share one line (IDO_NOTES 347). */
s32 Etsk_haita_tama1(Obj *o, s16 msg, void *arg) {
    HaitaTamaW *w = (HaitaTamaW *) FX(o);
    s32 setup;
    Vec3f v;
    s32 pad;
    f32 angle;
    s16 way;
    s16 no;

    switch (msg) {
        case 2:
            o->name = "tama1";
            setup = eneSetup(o, arg);
            no = (setup >> 16) & 0xFFFF;
            way = setup & 0xFFFF;
            DPRINTF("no %d, atk_dir %d\n", no, way);
            if ((w->model = eneNewPol(o, etskHaitaTama1Dl)) == NULL) {
                return -1;
            }
            w->model->scale[2] = 0.2f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->flags |= 3;
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 20, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 0x7B;
            w->behave = 4;
            if (way == 4) {
                angle = no * 0.62831855f + 3.455752f;
            } else {
                angle = -0.31415927f - no * 0.62831855f;
            }
            w->vel[0] = cosf(angle) * 4.0;
            w->vel[1] = sinf(angle) * 4.0;
            w->rot = mathAtan2(w->vel[1], w->vel[0]) + 1.5707964f;
            v.x = w->vel[0];
            v.y = w->vel[1];
            v.z = w->vel[2];
            *(Vec3f *) &w->atk->info = v;
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            if (gGameFrameCount & 1) {
                w->model->dl2 = NULL;
            } else {
                w->model->dl2 = etskHaitaTama1Dl;
            }
            if (w->hit & 0xE) {
                w->behave |= 0x4000000;
            }
            if (eneAfterProc(o) == -1) {
                DPRINTF("Etsk_haita_tama1: 死亡\n");
                return -1;
            }
            break;
        case 3:
            break;
        case 1:
            /* The first destroy on one line (the game's style) */
            if (w->model != NULL) { frmDestroy((Frame *) w->model); }
            if (w->atk != NULL) {
                frmDestroy((Frame *) w->atk);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern Gfx etskHaitaTama2Dl[]; /* the shot */

/* "tama2", the haita's shot: a ball growing from 0.05 to 0.25 (0.01 a frame), with two attack
 * boxes (50 above and 60 below), flying 4 a frame its way, drawn green over blue. */
s32 Etsk_haita_tama2(Obj *o, s16 msg, void *arg) {
    HaitaTamaW *w = (HaitaTamaW *) FX(o);
    ACollFrame *atk;
    s32 way;
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "tama2";
            way = eneSetup(o, arg);
            if ((w->model = enePolModelMake(o, etskHaitaTama2Dl, NULL, 1)) == NULL) {
                return -1;
            }
            w->model->scale[2] = 0.05f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->flags |= 7;
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 10, 50, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 0x7B;
            if ((atk = ((AtkAddYZFunc) acollAtkAddYZ)(w->atk, &w->model->f, 10, -60, 0, 0)) == NULL) {
                return -1;
            }
            atk->kind = 0x7B;
            w->vel[0] = (way == 4) ? -4.0 : 4.0;
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            if (w->model->scale[0] + 0.01 < 0.25) {
                w->model->scale[0] += 0.01;
            } else {
                w->model->scale[0] = 0.25f;
            }
            w->model->scale[1] = w->model->scale[2] = w->model->scale[0];
            if (eneAfterProc(o) == -1) {
                DPRINTF("Etsk_haita_tama2: 死亡\n");
                return -1;
            }
        case 3:
            w->model->dl = eneDlEnvPrimColor(o, 0xBF, 0xFF, 0xBF, 0, 0, 0xFF, 0xFF);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy((Frame *) w->model);
            }
            if (w->atk != NULL) {
                frmDestroy((Frame *) w->atk);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
