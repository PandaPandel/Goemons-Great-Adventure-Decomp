#include "tlb.h"

/* "sh_hani"'s work (its proc Etsk_sh_hani_main): EneObj's layout (enemy.h), with its own fields. */
typedef struct ShHaniW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 unused09;
    /* 0x0A */ u8 hit; /* 0x10: hit this step */
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ AnimeFrame *anim;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rotX;
    /* 0x20 */ f32 rotY;
    /* 0x24 */ f32 rotZ;
    /* 0x28 */ f32 vel; /* along the course (EneObj's vel[0]) */
    /* 0x2C */ f32 vy;
    /* 0x30 */ f32 vz;
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ Obj *shadow;
    /* 0x40 */ u32 halves[2]; /* its two halves' display lists, for a cut */
    /* 0x48 */ u32 behave; /* EneObj's (enemy.h) */
    /* 0x4C */ s16 timer;
    /* 0x4E */ u8 pad4E;
    /* 0x4F */ u8 next; /* where it goes after backing out: 1 = state 0, 2 = state 1 */
    /* 0x50 */ u8 guard; /* set from its attack animation's frame 2 on */
} ShHaniW;

typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                    s32 a);

/* Its steps, called far. */
void shHaniWalk(Obj *o);
void shHaniWait(Obj *o);
void shHaniGuard(Obj *o);
void shHaniUnguard(Obj *o);
void shHaniTurn(Obj *o);

extern u8 etskShHaniAnim[]; /* an animation in its segment */
extern u8 etskShHaniTurnAnim[]; /* its turn */
extern u8 etskShHaniHalf0Dl[]; /* its two halves */
extern u8 etskShHaniHalf1Dl[];

#define DPRINTF

s32 Etsk_sh_hani_main(Obj *o, s16 msg, void *arg);

/* "sh_hani", a shield haniwa: an animated model with an attack box (radius 42, 80 up, kind 0x78)
 * and a damage box (70 by 160), set on the ground, facing the nearest player along the course.
 * Hit by kinds 13, 10, 14 or 125 it dies; any other hit while it is guarding (guard) from the
 * side it faces turns it round (state 4).
 * etskShHaniAnim's second use is cast (s32), the first (u32): as two expressions IDO builds the
 * address again, as the game does (tlb_0040's lever). */
s32 Etsk_sh_hani_main(Obj *o, s16 msg, void *arg) {
    ShHaniW *w = (ShHaniW *) FX(o);
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "sh_hani";
            w->next = eneSetup(o, arg);
            if ((w->anim = eneNewAnime(o, (u32) etskShHaniAnim)) == NULL) {
                return -1;
            }
            if ((w->shadow = (Obj *) shadowCreate()) == NULL) {
                return -1;
            }
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->anim->f, 42, 80, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 0x78;
            if ((w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->anim->f, 70, 160, 0, 0, 0)) == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("Etsk_sh_hani_j: 地面にセット失敗\n");
                return -1;
            }
            w->halves[0] = (u32) etskShHaniHalf0Dl;
            w->halves[1] = (u32) etskShHaniHalf1Dl;
            w->behave = 0x404D01;
            w->guard = 0;
            w->rotY = eneCourseAngle(o, eneNearestPlWay(o));
            if (w->next == 1) {
                w->state = 0;
            } else if (w->next == 2) {
                w->anim->anim = etskShHaniAnim;
                w->state = 1;
            }
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->state) {
                case 0:
                    (*shHaniWalk)(o);
                    break;
                case 1:
                    (*shHaniWait)(o);
                    break;
                case 2:
                    (*shHaniGuard)(o);
                    break;
                case 3:
                    (*shHaniUnguard)(o);
                    break;
                case 4:
                    (*shHaniTurn)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                DPRINTF("Etsk_sh_hani_main: 死亡\n");
                return -1;
            }
            if (w->hit & 0x10) {
                if (w->dmg->hitKind == 13 || w->dmg->hitKind == 10 || w->dmg->hitKind == 14
                    || w->dmg->hitKind == 125) {
                    w->behave |= 0x80000;
                } else if (w->guard && eneNearestPlWay(o) == eneFaceWay(o)) {
                    eneSetSd(o, 0x2AF);
                    w->anim->anim = etskShHaniTurnAnim;
                    w->anim->frame = 1.0f;
                    w->behave &= ~0x4500;
                    w->behave |= 0x280;
                    if (eneFaceWay(o) == 4) {
                        w->vel = 1.0f;
                    } else if (eneFaceWay(o) == 1) {
                        w->vel = -1.0f;
                    }
                    w->state = 4;
                } else {
                    w->behave |= 0x80000;
                }
            }
            break;
        case 3:
            break;
        case 1:
            if (w->anim != NULL) {
                frmDestroy(&w->anim->f);
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

/* Walks toward the nearest player (0.8 one way or the other); at its animation's end, starts
 * animation etskShHaniAnim, stops and goes to state 2.
 * Matched 2026-10-11; it was parked for w's spill slot (the second pad as one word). */
void shHaniWalk(Obj *o) {
    s32 pad;
    ShHaniW *w = (ShHaniW *) FX(o);
    s32 pad2;
    s32 way = eneNearestPlWay(o);

    if (way == 4) {
        ((ShHaniW *) FX(o))->vel = -0.8f;
    } else if (way == 1) {
        ((ShHaniW *) FX(o))->vel = 0.8f;
    }
    if (eneAnimLoop(o, w->anim, 0.88f)) {
        w->anim->anim = etskShHaniAnim;
        w->anim->frame = 0.0f;
        w->state = 2;
        w->vel = 0.0f;
    }
}

/* After 31 frames, state 2. */
void shHaniWait(Obj *o) {
    ShHaniW *w = (ShHaniW *) FX(o);

    w->timer++;
    if (w->timer >= 31) {
        w->timer = 0;
        w->state = 2;
    }
}

/* Plays its animation once (state 3 at its end); guard is set from frame 2 on. */
void shHaniGuard(Obj *o) {
    ShHaniW *w = (ShHaniW *) FX(o);

    if (eneAnimPlayOnce(o, ((ShHaniW *) FX(o))->anim, 1.0f)) {
        ((ShHaniW *) FX(o))->state = 3;
    }
    if (w->anim->frame >= 2.0) {
        w->guard = 1;
    }
}

extern u8 etskShHaniUnguardAnim[]; /* an animation in its segment */

/* After 20 frames, plays its animation backward (guard cleared from frame 2 down: the reverse of
 * shHaniGuard); at its start, by `next`: animation etskShHaniUnguardAnim and state 0, or state 1. */
void shHaniUnguard(Obj *o) {
    ShHaniW *w = (ShHaniW *) FX(o);
    s32 pad[2];
    s32 done;

    ((ShHaniW *) FX(o))->timer++;
    if (((ShHaniW *) FX(o))->timer >= 20) {
        done = eneAnimPlayOnce(o, w->anim, -0.3f);
        if (w->anim->frame <= 2.0) {
            w->guard = 0;
        }
        if (done) {
            w->timer = 0;
            if (w->next == 1) {
                w->anim->anim = etskShHaniUnguardAnim;
                w->anim->frame = 0.0f;
                w->state = 0;
            } else if (w->next == 2) {
                w->state = 1;
            }
        }
    }
}

/* Slows to a stop (0.25 a frame either way) while its animation plays at half speed; stopped,
 * it holds animation etskShHaniAnim's last frame, goes to state 3 and changes its behave bits (off
 * 0x280, on 0x4500). */
void shHaniTurn(Obj *o) {
    ShHaniW *w = (ShHaniW *) FX(o);
    s32 pad[2];

    if (((ShHaniW *) FX(o))->vel > 0.0f) {
        w->vel -= 0.25;
        if (w->vel <= 0.0f) {
            w->vel = 0.0f;
        }
    } else if (w->vel < 0.0f) {
        w->vel += 0.25;
        if (w->vel <= 0.0f) {
            w->vel = 0.0f;
        }
    }
    eneAnimPlayOnce(o, w->anim, 0.5f);
    if (w->vel == 0.0) {
        w->anim->anim = etskShHaniAnim;
        w->anim->frame = polAnimeLastFrame(w->anim);
        w->timer = 0;
        w->behave &= ~0x280;
        w->behave |= 0x4500;
        w->state = 3;
    }
}
