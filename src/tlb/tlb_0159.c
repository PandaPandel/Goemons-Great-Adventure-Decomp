#include "tlb.h"

/* The "yado" enemy kind (archive file 159): a hermit crab (yadokari). It waits until a player is
 * within 70, turns to it with each step of an animation, walks three loops towards it, attacks,
 * draws back into its shell and turns again. Its placement's type picks how it starts: 1 drops
 * from above (state 0), 2 stands on the ground (state 6). A hit on its shell (the second damage
 * box) rocks it with a sound; a hit on the body flashes it and knocks it back (state 11). The
 * first time its model's position is inside the second ride tree (rideTree2PointIn) it makes a
 * white splash (effectSibukiCreateWhite) with sound 0x298. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

/* libultra's sinf (PR/gu.h declares it), called from the TLB through its address. */
#define sinf (*sinf)

extern u8 etskYadoTurnAnim[];
extern u8 etskYadoWalkAnim[];
extern u8 etskYadoState11Anim[];
extern u8 etskYadoAnim[];
extern u8 etskYadoHalf0Dl[];
extern u8 etskYadoHalf1Dl[];

/* A yado's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Yado {
    /* 0x00 */ EneObj e;
    /* 0x4C */ ACollFrame *shell; /* its second damage box: a hit there only rocks it */
    /* 0x50 */ f32 rock; /* the rocking's phase, 0 to 2 pi */
    /* 0x54 */ u8 way; /* the way it faces along x: 1 or 4 (enePlDirX) */
    /* 0x55 */ u8 loops; /* walk loops done */
    /* 0x56 */ u8 flash; /* frames left of the hit flash */
    /* 0x57 */ u8 alpha; /* the env colour's first value while not flashing */
    /* 0x58 */ u8 timer;
    /* 0x59 */ u8 shellHit; /* the shell box was hit last step */
    /* 0x5A */ u8 step; /* the walk sound's step, 0-3 */
    /* 0x5B */ u8 splashed;
} Yado;

#define YADO(obj) ((Yado *) FX(obj))
#define MODEL(w) ((AnimeFrame *) (w)->e.model)
#define ARM(w) ((ACollFrame *) (w)->e.atk->frame.child)

/* A frame destroyed if it was made. A macro: the game's code for the first one is scheduled as
 * one source line (written over three lines, as1 puts the frmDestroy address's addiu, not w's,
 * in the branch's delay slot). */
#define FRM_DESTROY(f) if ((f) != NULL) { frmDestroy((Frame *) (f)); }

void yadoHang(Obj *o);
void yadoFall(Obj *o);
void yadoTurn(Obj *o);
void yadoWalk(Obj *o);
void yadoAttack(Obj *o);
void yadoTurnBack(Obj *o);
void yadoGroundWait(Obj *o);
void yadoGroundTurn(Obj *o);
void yadoGroundWalk(Obj *o);
void yadoGroundStop(Obj *o);
void yadoGroundTurnBack(Obj *o);
void yadoKnockback(Obj *o);

s32 Etsk_yado_main(Obj *o, s16 msg, void *arg) {
    Yado *w = YADO(o);
    ACollFrame *a;
    s32 type;
    s32 pad;
    Vec3f p;
    Vec3f push;

    switch (msg) {
        case 2:
            o->name = "yado";
            type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneAnimeModelMake(o, (u32) etskYadoAnim, NULL, 0)) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 90, 90, -30, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((a = ATK_ADD_YZ((Frame *) w->e.atk, w->e.model, 60, 60, 220, 0)) == NULL) {
                return -1;
            }
            a->kind = 120;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 100, 140, 0, 100, 0)) == NULL) {
                return -1;
            }
            if ((w->shell = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 110, 220, 0, -50, 0)) == NULL) {
                return -1;
            }
            if ((w->e.shadow = (Obj *) shadowCreate()) == NULL) {
                return -1;
            }
            w->e.halves[0] = (u32) etskYadoHalf0Dl;
            w->e.halves[1] = (u32) etskYadoHalf1Dl;
            w->timer = 0;
            w->loops = 0;
            w->flash = 0;
            w->splashed = 0;
            w->rock = 0.0f;
            w->e.rot[1] = eneCourseAngle(o, 2);
            if (type == 1) {
                w->e.behave = 0x8406282;
                w->e.hp = 2;
                w->e.dmg->flags &= ~2;
                ARM(w)->flags &= ~2;
                w->e.state = 0;
            } else if (type == 2) {
                if (eneSetOnEarth(o) == -1) {
                    return -1;
                }
                w->e.behave = 0xC04280;
                MODEL(w)->scale[2] = 0.05f;
                MODEL(w)->scale[0] = MODEL(w)->scale[1] = MODEL(w)->scale[2];
                ARM(w)->flags &= ~2;
                w->e.state = 6;
            }
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*yadoHang)(o);
                    break;
                case 1:
                    (*yadoFall)(o);
                    break;
                case 2:
                    (*yadoTurn)(o);
                    break;
                case 3:
                    (*yadoWalk)(o);
                    break;
                case 4:
                    (*yadoAttack)(o);
                    break;
                case 5:
                    (*yadoTurnBack)(o);
                    break;
                case 6:
                    (*yadoGroundWait)(o);
                    break;
                case 7:
                    (*yadoGroundTurn)(o);
                    break;
                case 8:
                    (*yadoGroundWalk)(o);
                    break;
                case 9:
                    (*yadoGroundStop)(o);
                    break;
                case 10:
                    (*yadoGroundTurnBack)(o);
                    break;
                case 11:
                    (*yadoKnockback)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_yado_main: 死亡\n");
                return -1;
            }
            if (w->e.hit & 0x10) {
                MODEL(w)->anim = etskYadoState11Anim;
                MODEL(w)->frame = 0.0f;
                w->e.dmg->flags &= ~2;
                ARM(w)->flags &= ~2;
                w->flash = 12;
                w->e.behave &= ~0x4000;
                w->e.vel[0] = eneKnockbackSpeed(o, w->e.dmg, 4.0f);
                w->e.state = 11;
            }
            if (w->flash == 0) {
                if (acollDmgWasHit(w->shell) == 1) {
                    if (w->shellHit == 0) {
                        eneSetSd(o, 0x2EC);
                        w->shellHit = 1;
                        w->rock = 0.0f;
                    }
                } else {
                    w->shellHit = 0;
                }
            }
            if (w->flash - 1 >= 0) {
                w->flash = w->flash - 1;
            } else {
                w->flash = 0;
            }
            if (w->rock + 0.8 < 6.28318548202514648) {
                w->rock += 0.8;
                w->e.rot[0] = sinf(w->rock) * -3.141592741f / 16;
            } else {
                w->e.rot[0] = 0.0f;
            }
            if (w->splashed == 0) {
                p.x = MODEL(w)->pos[0];
                p.y = MODEL(w)->pos[1];
                p.z = MODEL(w)->pos[2];
                if (rideTree2PointIn(&p, &push)) {
                    eneSetSd(o, 0x298);
                    effectSibukiCreateWhite(p.x, p.y, p.z, 0.1f, 0.1f, 0.007499999832f, 0.007499999832f);
                    w->splashed = 1;
                }
            }
            break;
        case 3:
            if (w->flash != 0) {
                MODEL(w)->dl = eneDlEnvColorLit(o, (w->flash & 2) ? 0x80 : 0, 0xBF, 0, 0,
                                                (w->flash & 2) ? 0 : 1);
            } else {
                MODEL(w)->dl = eneDlEnvColorLit(o, w->alpha, 0xFF, 0xFF, 0xFF, 1);
            }
            break;
        case 1:
            FRM_DESTROY(w->e.model);
            FRM_DESTROY(w->e.atk);
            FRM_DESTROY(w->e.dmg);
            FRM_DESTROY(w->shell);
            FRM_DESTROY(w->e.shadow);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* State 0: hangs until a player is within 70, then drops, facing it. */
void yadoHang(Obj *o) {
    Yado *w = YADO(o);
    s32 pad;

    if (enePlInRangeCheck(o, 70.0f, 0)) {
        w->e.behave |= 0x800000;
        w->way = enePlDirX(o, 0);
        w->e.state = 1;
    }
}

/* State 1: falls; on landing it bounces once. */
void yadoFall(Obj *o) {
    Yado *w = YADO(o);

    if (YADO(o)->e.hit & 1) {
        w->e.vel[1] *= -0.35;
        w->e.state = 2;
    }
}

/* State 2: on the ground, waits out its timer, then turns a quarter towards its way with one
 * play of the animation and walks (3); its body box is live from frame 5. */
void yadoTurn(Obj *o) {
    Yado *w = YADO(o);

    if (w->e.hit & 1) {
        w->e.behave &= ~0x2000;
        w->e.vel[1] = 0.0f;
        if (w->timer != 0) {
            if (w->timer - 1 >= 0) {
                w->timer = w->timer - 1;
            } else {
                w->timer = 0;
            }
            return;
        }
        if (eneAnimPlayOnce(o, MODEL(w), 0.5f)) {
            MODEL(w)->anim = etskYadoTurnAnim;
            MODEL(w)->frame = 0.0f;
            w->timer = 0;
            w->step = 2;
            w->e.state = 3;
        } else {
            w->e.rot[1] += (w->way == 4) ? 1.570796371f / ((polAnimeLastFrame(MODEL(w)) + 1) / 0.5f)
                                         : -(1.570796371f / ((polAnimeLastFrame(MODEL(w)) + 1) / 0.5f));
        }
        if (MODEL(w)->frame >= 5.0) {
            w->e.dmg->flags |= 2;
        }
    }
}

/* State 3: walks towards its way three animation loops (a step sound every fourth step), then
 * attacks (4); after the first loop it stands for 15 steps. */
void yadoWalk(Obj *o) {
    Yado *w = YADO(o);
    s32 pad[2];

    if (YADO(o)->timer != 0) {
        w->e.vel[0] = 0.0f;
        if (w->timer - 1 > 0) {
            w->timer = w->timer - 1;
        } else {
            w->timer = 0;
            w->step = 2;
        }
        return;
    }
    if (w->step == 0) {
        eneSetSd2(o, 0x2DA, 0, 0);
    }
    if (w->step + 1 < 4) {
        w->step = w->step + 1;
    } else {
        w->step = 0;
    }
    w->e.vel[0] = (w->way == 4) ? -1.6 : 1.6;
    if (eneAnimLoop(o, MODEL(w), 1.5f)) {
        w->loops++;
        if (w->loops == 1) {
            w->timer = 15;
        }
        if (w->loops >= 3) {
            MODEL(w)->anim = etskYadoWalkAnim;
            MODEL(w)->frame = 0.0f;
            w->loops = 0;
            w->e.state = 4;
        }
    }
}

/* State 4: the attack: its claw box is live from frame 7 to 9; then it draws back (5). */
void yadoAttack(Obj *o) {
    Yado *w = YADO(o);
    s32 pad[2];
    s32 end;

    w->e.vel[0] = 0.0f;
    end = eneAnimPlayOnce(o, MODEL(w), 0.5f);
    if (MODEL(w)->frame >= 7.0 && MODEL(w)->frame < 9.0) {
        ARM(w)->flags |= 2;
    } else {
        ARM(w)->flags &= ~2;
    }
    if (end) {
        MODEL(w)->anim = etskYadoAnim;
        MODEL(w)->frame = polAnimeLastFrame(MODEL(w));
        w->e.state = 5;
    }
}

/* State 5: plays the turn backwards (turning the other way), then faces the player, waits 15
 * and turns again (2); its body box is off below frame 5. */
void yadoTurnBack(Obj *o) {
    Yado *w = YADO(o);
    s32 pad[2];

    w->e.vel[0] = 0.0f;
    if (eneAnimPlayOnce(o, MODEL(w), -1.0f)) {
        w->way = enePlDirX(o, 0);
        w->timer = 15;
        w->e.state = 2;
    } else {
        w->e.rot[1] += (w->way == 4) ? 1.570796371f / ((polAnimeLastFrame(MODEL(w)) + 1) / -1.0f)
                                     : -(1.570796371f / ((polAnimeLastFrame(MODEL(w)) + 1) / -1.0f));
    }
    if (MODEL(w)->frame < 5.0) {
        w->e.dmg->flags &= ~2;
    }
}

/* State 6: on the ground, waits until a player is within 70, then faces it (7). */
void yadoGroundWait(Obj *o) {
    Yado *w;

    if (enePlInRangeCheck(o, 70.0f, 0)) {
        w = YADO(o);
        w->way = enePlDirX(o, 0);
        w->e.state = 7;
    }
}

/* State 7: waits out its timer, then turns a quarter towards its way and walks (8). */
void yadoGroundTurn(Obj *o) {
    Yado *w = YADO(o);

    if (YADO(o)->timer != 0) {
        if (w->timer - 1 >= 0) {
            w->timer = w->timer - 1;
        } else {
            w->timer = 0;
        }
    } else if (eneAnimPlayOnce(o, MODEL(w), 0.5f)) {
        MODEL(w)->anim = etskYadoTurnAnim;
        MODEL(w)->frame = 0.0f;
        w->timer = 0;
        w->step = 2;
        w->e.state = 8;
    } else {
        w->e.rot[1] += (w->way == 4) ? 1.570796371f / ((polAnimeLastFrame(MODEL(w)) + 1) / 0.5f)
                                     : -(1.570796371f / ((polAnimeLastFrame(MODEL(w)) + 1) / 0.5f));
    }
}

/* State 8: walks towards its way three animation loops, then stops (9). */
void yadoGroundWalk(Obj *o) {
    Yado *w = YADO(o);
    s32 pad[2];

    if (YADO(o)->timer != 0) {
        w->e.vel[0] = 0.0f;
        if (w->timer - 1 > 0) {
            w->timer = w->timer - 1;
        } else {
            w->timer = 0;
            w->step = 2;
        }
        return;
    }
    if (w->step == 0) {
        eneSetSd2(o, 0x2D9, 0, 0);
    }
    if (w->step + 1 < 4) {
        w->step = w->step + 1;
    } else {
        w->step = 0;
    }
    w->e.vel[0] = (w->way == 4) ? -1.0 : 1.0;
    if (eneAnimLoop(o, MODEL(w), 1.5f)) {
        w->loops++;
        if (w->loops == 1) {
            w->timer = 15;
        }
        if (w->loops >= 3) {
            MODEL(w)->anim = etskYadoAnim;
            MODEL(w)->frame = polAnimeLastFrame(MODEL(w));
            w->loops = 0;
            w->e.state = 10;
        }
    }
}

/* State 9: stops (one play of its animation), then draws back (10). */
void yadoGroundStop(Obj *o) {
    Yado *w = YADO(o);
    s32 pad[2];

    w->e.vel[0] = 0.0f;
    if (eneAnimPlayOnce(o, MODEL(w), 0.5f)) {
        MODEL(w)->anim = etskYadoAnim;
        MODEL(w)->frame = polAnimeLastFrame(MODEL(w));
        w->e.state = 10;
    }
}

/* State 10: plays the turn backwards, then faces the player, waits 15 and turns again (7). */
void yadoGroundTurnBack(Obj *o) {
    Yado *w = YADO(o);
    s32 pad[2];

    w->e.vel[0] = 0.0f;
    if (eneAnimPlayOnce(o, MODEL(w), -1.0f)) {
        w->way = enePlDirX(o, 0);
        w->timer = 15;
        w->e.state = 7;
    } else {
        w->e.rot[1] += (w->way == 4) ? 1.570796371f / ((polAnimeLastFrame(MODEL(w)) + 1) / -1.0f)
                                     : -(1.570796371f / ((polAnimeLastFrame(MODEL(w)) + 1) / -1.0f));
    }
}

/* State 11: knocked back; slows down, and when the hit animation is over it attacks (4). */
void yadoKnockback(Obj *o) {
    Yado *w = YADO(o);
    s32 pad;

    if (YADO(o)->e.vel[0] < 0.0f) {
        w->e.vel[0] += 0.363636363636363646;
    } else if (w->e.vel[0] > 0.0f) {
        w->e.vel[0] -= 0.363636363636363646;
    }
    if (eneAnimPlayOnce(o, MODEL(w), polAnimeLastFrame(MODEL(w)) / 12.0f)) {
        MODEL(w)->anim = etskYadoWalkAnim;
        MODEL(w)->frame = 0.0f;
        w->e.dmg->flags |= 2;
        w->e.behave |= 0x4000;
        w->e.state = 4;
    }
}
