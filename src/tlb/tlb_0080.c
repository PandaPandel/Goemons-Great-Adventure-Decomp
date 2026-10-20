#include "tlb.h"
/* nuri's seven attack boxes: x, y, z on its model. */
f32 nuriAtkBoxes[][3] = {
    { 0.0f, 4.2e+02f, 12.0f },   { 0.0f, 3.5e+02f, 12.0f }, { 0.0f, 2.8e+02f, 12.0f }, { 0.0f, 2.1e+02f, 12.0f },
    { 0.0f, 1.4e+02f, 12.0f },   { -115.0f, 4e+02f, 12.0f }, { 115.0f, 4e+02f, 12.0f },
};

/* Main's sinf, called through its address as TLB code calls main. */
#define sinf (*sinf)

#define DPRINTF

/* "nuri" (its proc Etsk_nuri_main) and its steps. */
typedef struct NuriW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 unused09;
    /* 0x0A */ u8 flags;
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z; /* across the course: below 0 at the back */
    /* 0x1C */ f32 angle; /* an angle, in radians */
    /* 0x20 */ f32 ry; /* its heading */
    /* 0x24 */ f32 rotZ;
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ u8 pad38[0x10];
    /* 0x48 */ u32 flags2; /* EneObj's behave */
    /* 0x4C */ RideFrame *ride;
    /* 0x50 */ f32 swayPhase;
    /* 0x54 */ f32 sway;
    /* 0x58 */ f32 fallSpeed;
    /* 0x5C */ s16 timer;
    /* 0x5E */ u8 attackHit;
} NuriW;

typedef ACollFrame *(*NuriAtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*NuriAtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy, s16 oz,
                                         s32 a);

void nuriWait(Obj *o);
void nuriWaitPlayer(Obj *o);
void nuriSwing(Obj *o);
void nuriFallDown(Obj *o);
void nuriRaise(Obj *o);
extern Gfx etskNuriDl[]; /* nuri's model, and the knocked-down konuri's */
extern u8 etskNuriRide[]; /* its ride */

/* "nuri", the swinging wall: a pol model with an attack box (radius 30, 500 up) and seven more
 * from its table (kind 0x7D), a ride, set on the ground. Its arg's kind 1 faces it along the
 * course (state 0); kind 2 faces it back, front or along by the side of the course it is on
 * (state 1). Each frame its state's step runs; its model rises by 10/pi of its lean. Drawn at
 * alpha 0x7F. */
s32 Etsk_nuri_main(Obj *o, s16 msg, void *arg);
s32 Etsk_nuri_main(Obj *o, s16 msg, void *arg) {
    NuriW *w = (NuriW *) FX(o);
    ACollFrame *box;
    s32 i;
    s32 kind;

    switch (msg) {
        case 2:
            o->name = "nuri";
            kind = eneSetup(o, arg);
            if ((w->model = eneNewPol(o, etskNuriDl)) == NULL) {
                return -1;
            }
            if ((w->atk = ((NuriAtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 30, 500, 0, 0)) ==
                NULL) {
                return -1;
            }
            w->atk->kind = 0x7D;
            for (i = 0; i < 7; i++) {
                if ((box = ((NuriAtkAddXYZFunc) acollAtkAddXYZ)(w->atk, &w->model->f, 30, nuriAtkBoxes[i][0],
                                                                nuriAtkBoxes[i][1], nuriAtkBoxes[i][2],
                                                                0)) == NULL) {
                    return -1;
                }
                box->kind = 0x7D;
            }
            if ((w->ride = rideCreate(gRideTree1, (s32) o, &w->model->f,
                                      (RideData *) eneFileAddr(o, etskNuriRide, 0))) == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                return -1;
            }
            w->flags2 = 0x801;
            if (kind == 1) {
                w->ry = eneCourseAngle(o, 4);
                w->state = 0;
            } else if (kind == 2) {
                if (w->z < 0.0f) {
                    w->ry = eneCourseAngle(o, 8);
                } else if (w->z > 0.0f) {
                    w->ry = eneCourseAngle(o, 2);
                } else {
                    w->ry = eneCourseAngle(o, 4);
                }
                w->state = 1;
            }
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->state) {
                case 0:
                    (*nuriWait)(o);
                    break;
                case 1:
                    (*nuriWaitPlayer)(o);
                    break;
                case 2:
                    (*nuriSwing)(o);
                    break;
                case 3:
                    (*nuriFallDown)(o);
                    break;
                case 4:
                    (*nuriRaise)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                DPRINTF("Etsk_nuri_main: 敵殺します\n");
                return -1;
            }
            if (w->flags & 8) {
                w->attackHit = 1;
            }
            w->model->pos[1] = ((w->angle > 0.0f) ? w->angle : -w->angle) * 10.0f / 3.1415927f + w->y;
        case 3:
            ((NuriW *) FX(o))->model->dl = gGfxDlHead;
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 0x7F);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            if (w->model != NULL) { frmDestroy(&w->model->f); }
            if (w->atk != NULL) { frmDestroy(&w->atk->frame); }
            if (w->ride != NULL) { frmDestroy((Frame *) w->ride); }
            break;
    }
    /* "Toward the back" / "toward the front": these strings come after this function's own in the
     * .rodata ("Etsk_nuri_main" first), so they are not create's (case 2 falls into case 0); a
     * later step's prints, which one is not known. */
    DPRINTF("奥へ\n");
    DPRINTF("手前へ\n");
    return tskDefaultProc(o, msg, arg);
}

/* With flag 0x10: state 2, sway 7.5 degrees (in radians), swayPhase pi. */
void nuriWait(Obj *o) {
    NuriW *w = (NuriW *) FX(o);

    if (w->flags & 0x10) {
        w->state = 2;
        w->sway = 0.1308997f;
        w->swayPhase = 3.1415927f;
    }
}

/* With a player within 70 in x: turns by 3.75 degrees, switches its attack on, state 3. */
void nuriWaitPlayer(Obj *o) {
    s32 pad;
    NuriW *w = (NuriW *) FX(o);

    if (eneNearestPlInX(o, 70.0f)) {
        w->angle += 0.06544985f;
        w->attackHit = 0;
        w->fallSpeed = 0.0f;
        eneAtkSwitch(o, w->atk, 1);
        w->state = 3;
    }
}

/* Swings: its angle is sin(swayPhase) times the swing's size sway, the phase going on by 0.1 and
 * the size dying by 0.0004 a frame. With flag 0x10 the size is pushed toward 7.5 degrees by
 * the way it swings. With no size left, state 0; past 7.5 degrees, state 3. 61 lines: the
 * game reloads obj into v0 (not a0) after the first sinf, its two floats sit 4 higher, and
 * one instruction more. A scalar pad in place of pad[2] moved the spills instead (65).
 * 2026-10-11: prev through w (not FX(o) twice) reloads o into v0 as the game: real 24 to 14;
 * a scalar pad and the step's size as a local d (set in each arm, as the game subtracts in
 * each) give the game's whole stack: 10. Left: the game makes 0.0f once (mtc1 zero,f14)
 * before the flags test and uses it in all three compares; here twice, inside and after
 * (one instruction more). A dead "else { push = 0.0f; }": 15.
 * 2026-10-12: push = 0.0f at its declaration 17; the three zeros as the int 0 (which share
 * one register in tlb_0142's obisu) 10, the same; w->angle = cur before the flags test 42.
 * 2026-10-12: the push as a local first 16; the last compare's 0.1308997f spelled ".1308997f"
 * 17; the prints as PRINTF 10; an empty "else {}" on the flags test 15.
 * 2026-10-12: a named "f32 zero" (in pad's place, set before the flags test, used in all
 * three compares): 10, the same code (propagated); push computed before the flags test (so its
 * zero is in the head): 21, uopt does not sink it.
 * 2026-10-12 (second try), probe (n0 is this C, the same code): the push as an if
 * on the product, d = 0.0f at its declaration, d computed once before the way test, the angle
 * stored as soon as it is made, the last test on cur: every one still makes the zero twice.
 * 2026-10-12 (third try): permuter (--stack-diffs, ~1,700 iterations, base 390) and greedy:
 * nothing. A probe (cuts of this C): without the state-3 test, without the
 * way ternary, without the push ternary, without the 0.0004 step, with the way test against
 * 0.5f: the flags block's zero and the end test's zero are always made apart (without the
 * 0.0004 step the block makes three); a zero variable set at its declaration, or there and
 * again before the flags test, is propagated (the same code); the end test's zero spelled 0.f:
 * the same two. So no cut of this C gives one zero; the game's zero starts in the block after
 * the way ternary, while the hoisted 3 (t0) starts right after the last sinf. The way as an
 * if/else: 10, the same.
 * 2026-10-12 (fourth try, trace + ugen-edit, real 10). Trace: the 0.0 range (98) is blocks 7..13
 * (first use: the push test, block 7) with a load at 7 and again at 13 (from block 6, the flags
 * test, it is not available); cur (23) colours first and takes f14, the 0.0 f18. ugen-edit
 * proof (full.s 753-830): one "li.s $f14, 0.0"
 * at the end of block 6 (before the flags load), none in 7 or 13, cur in f18: real 0. Only
 * swapping f14/f18 with both loads kept (blk_b.s): real 4, length +1. So the game's 0.0 range
 * starts in block 6: some 0.0 occurrence survives to range building in block 6 (or the range is
 * extended to the uses' dominator). Measured in a probe: "push = 0.0f;"
 * just before the flags test 17 (the range unchanged, the store is dead); push = 0.0f at its
 * declaration 17 (unchanged). Also learned: as1 hoists a single-def "li" across blocks (ugen
 * puts "li $8, 3" in the state-3 block; the object has it right after the last sinf).
 * 2026-10-12 (fifth try), MATCHED from the trace and uopt's own code
 * (vendor/ido-static-recomp/build/7.1/uopt.c):
 * - An empty "if (way < 0.0f) {}" after the 0.0004 step (IDO_NOTES 378 E5: an empty if emits
 *   nothing but keeps its uses) puts a 0.0 use in the flags block: the 0.0 range (99) now starts
 *   there with one load, the game's instruction stream (length +0), real 14. Left: cur f14,
 *   0.0 f16, 0.1308997 f18; the game has 0.0 f14, 0.1308997 f16, cur f18.
 * - Colouring order is by priority (adjsave). f_compute_save (uopt.c 151016-151262): save = sum
 *   over live bbs of (uses + defs) x freq, minus 1 x freq per needreglod (unless canmoverlod) and
 *   per needed save; count = live bbs + live-through bbs, and from 3 on count = (count-2)/4 + 2;
 *   adjsave = save / count. Here: cur (23) 3/4 = 0.75 (13 blocks), 0.0 (99) (4-1)/4 = 0.75,
 *   0.1308997 (100) (4-2)/3 = 0.667. On the tie cur comes first and takes f14.
 * - One more block in cur's span gives it 14 blocks, count 5, adjsave 0.6: it is coloured last
 *   (f18), the 0.0 first (f14), 0.1308997 then (f16). "if (way < 0.0f) {} else {}" makes that
 *   block: MATCH (only own-section addresses differ). The empty if/else is FAKE: the game's C
 *   had something here (a compiled-out print on the way, most likely) that we cannot see. */
void nuriSwing(Obj *o) {
    NuriW *w = (NuriW *) FX(o);
    s32 pad;
    f32 s;
    f32 prev = sinf(w->swayPhase) * w->sway;
    f32 cur;
    f32 way;
    f32 push;
    f32 d;

    w->swayPhase += 0.1;
    s = sinf(w->swayPhase);
    cur = sinf(w->swayPhase) * w->sway;
    way = (cur < prev) ? -1.0f : 1.0f;
    w->sway -= 0.0004;
    /* FAKE: a 0.0 use in this block and one block more in cur's span (see above). */
    if (way < 0.0f) {} else {}
    if (w->flags & 0x10) {
        push = (0.1308997f * s > 0.0f) ? 0.1308997f * s : -(0.1308997f * s);
        if (way < 0.0f) {
            d = 0.1308997f - push;
            w->sway += d;
            DPRINTF("yure_plus %f\n", d);
        } else {
            d = 0.1308997f - push;
            w->sway -= d;
            DPRINTF("yure_plus -%f\n", d);
        }
    }
    w->angle = cur;
    if (w->sway <= 0.0f) {
        w->state = 0;
    }
    if (w->angle > 0.1308997f) {
        w->state = 3;
    }
}

/* Swings down faster and faster (by 0.02 * sin(2 * angle + 3pi/2) + 0.02 more a frame); at 90
 * degrees it stops there with sound 0x108, rumbles every player's controller, and goes to
 * state 4. */
void nuriFallDown(Obj *o) {
    NuriW *w = (NuriW *) FX(o);
    f32 step = sinf(w->angle * 2 + 4.71238911151885986) * 0.02 + 0.02;
    s32 i;

    w->fallSpeed += step;
    w->angle += w->fallSpeed;
    if (w->angle >= 1.5707964f) {
        sndPlay(0x108);
        for (i = 0; i < 4; i++) {
            if (gGameWork.players[i] != NULL) {
                contPakRumbleStart(i, 0xAAAAAAAA, 15);
            }
        }
        w->angle = 1.5707964f;
        w->timer = 0;
        w->state = 4;
    }
}

/* After 31 frames: switches its attack off and, with attackHit, eases its angle back to 0 (6% a
 * frame); under 0.005 it is 0 and state 1. */
void nuriRaise(Obj *o) {
    s32 pad;
    NuriW *w = (NuriW *) FX(o);

    ((NuriW *) FX(o))->timer++;
    if (((NuriW *) FX(o))->timer >= 31) {
        eneAtkSwitch(o, w->atk, 0);
        if (w->attackHit) {
            w->angle -= w->angle * 0.06;
            if (w->angle <= 0.005) {
                w->state = 1;
                w->angle = 0.0f;
            }
        }
    }
}

/* "konuri", the swinging enemy (its proc Etsk_konuri_main). */
typedef struct Tlb80EneW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 unused09;
    /* 0x0A */ u8 flags; /* 0x10: hit */
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 tilt;
    /* 0x2C */ f32 vy;
    /* 0x30 */ f32 vz;
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ ACollFrame *body;
    /* 0x3C */ u8 pad3C[0xC];
    /* 0x48 */ u32 flags2; /* EneObj's behave; 2: no hit sound */
    /* 0x4C */ f32 lean;
    /* 0x50 */ f32 speed; /* the animation's, its sign the way it swings */
    /* 0x54 */ s16 turns;
    /* 0x56 */ s16 alpha;
} Tlb80EneW;

/* acollAtkAddYZ and acollDmgAddYZ return their box (main declares them void). */
typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                    s32 a);

void konuriSwing(Obj *o);
void konuriSwingBack(Obj *o);
void konuriSwingBackEnd(Obj *o);
void konuriSwingOn(Obj *o);
void konuriSwingOnEnd(Obj *o);
extern u8 etskKonuriAnim[]; /* konuri's animation */


/* "konuri": an animated model of a 20th with two attack boxes (radius 50, 500 and 200 up, kind
 * 0x78) and a damage box (120 by 500, 100 up), set on the ground, at speed 0.4, facing along the
 * course. Each frame its state's step runs (swinging; knocked back); a hit of kind 0x78 clears
 * flag 2; its model rises by 5/pi of its lean. Drawn at its alpha once a pol. */
s32 Etsk_konuri_main(Obj *o, s16 msg, void *arg) {
    Tlb80EneW *w = (Tlb80EneW *) FX(o);
    ACollFrame *box;
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "konuri";
            eneSetup(o, arg);
            if ((w->model = eneNewAnime(o, (u32) etskKonuriAnim)) == NULL) {
                return -1;
            }
            w->model->scale[2] = 0.05f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 50, 500, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 0x78;
            if ((box = ((AtkAddYZFunc) acollAtkAddYZ)(w->atk, &w->model->f, 50, 200, 0, 0)) == NULL) {
                return -1;
            }
            box->kind = 0x78;
            if ((w->body = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->model->f, 120, 500, 100, 0, 0)) ==
                NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("Etsk_konuri_main: 地面にセット失敗\n");
                return -1;
            }
            w->flags2 = 0x400803;
            w->alpha = 0xFF;
            w->speed = 0.4f;
            w->rot[1] = eneCourseAngle(o, 4);
            w->state = 0;
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->state) {
                case 0:
                    (*konuriSwing)(o);
                    break;
                case 1:
                    (*konuriSwingBack)(o);
                    break;
                case 2:
                    (*konuriSwingBackEnd)(o);
                    break;
                case 3:
                    (*konuriSwingOn)(o);
                    break;
                case 4:
                    (*konuriSwingOnEnd)(o);
                    break;
            }
            if (acollDmgWasHit(w->body) == 1 && w->body->hitKind == 0x78) {
                w->flags2 &= ~2;
            }
            if (eneAfterProc(o) == -1) {
                DPRINTF("Etsk_konuri_main: 敵殺します\n");
                return -1;
            }
            if (w->model != NULL) {
                w->model->pos[1] = ((w->rot[0] > 0.0f) ? w->rot[0] : -w->rot[0]) * 5.0f / 3.1415927f + w->y;
            }
        case 3:
            if (w->model->f.kind == gFrmKindPol) {
                ((PolFrame *) w->model)->dl = gGfxDlHead;
                gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
                gSPEndDisplayList(gGfxDlHead++);
            }
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->atk != NULL) {
                frmDestroy((Frame *) w->atk);
            }
            if (w->body != NULL) {
                frmDestroy((Frame *) w->body);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}


/* Animates at its speed; on frames 2-5 and 11-14 it leans 1.5 times its speed the way it swings.
 * Past frame 17 its swing turns over every second pass. When hit (sound 0x23D unless flag 2), it
 * is knocked back (state 1 forward, 3 back) and swaps its models and boxes for the knocked-down
 * model of a 20th, with an attack box (radius 60, 500 up, kind 0x78). Both turn tests read the
 * model's frame directly. The backward test is the forward one mirrored (frame > 17 + speed);
 * spelled "17 + speed < frame", IDO swaps the add's operands. */
void konuriSwing(Obj *o) {
    Tlb80EneW *w = (Tlb80EneW *) FX(o);
    f32 frame;
    s32 pad[2];

    eneAnimLoop(o, w->model, w->speed);
    frame = w->model->frame;
    if ((frame >= 11.0f && frame <= 14.0f) || (frame >= 2.0f && frame <= 5.0f)) {
        w->lean = ((w->speed > 0.0f) ? w->speed : -w->speed) * 1.5;
    } else {
        w->lean = 0.0f;
    }
    if (w->speed > 0.0f) {
        w->tilt = -w->lean;
        if (w->model->frame >= 17.0f && w->model->frame < 17.0f + w->speed) {
            w->turns++;
            if (w->turns == 2) {
                w->turns = 0;
                w->speed *= -1.0f;
            }
        }
    } else if (w->speed < 0.0f) {
        w->tilt = w->lean;
        if (w->model->frame <= 17.0f && w->model->frame > 17.0f + w->speed) {
            w->turns++;
            if (w->turns == 2) {
                w->turns = 0;
                w->speed *= -1.0f;
            }
        }
    }
    if (w->flags & 0x10) {
        if (!(w->flags2 & 2)) {
            eneSetSd(o, 0x23D);
        }
        if (eneKnockbackSpeed(o, w->body, 1.0f) > 0.0f) {
            w->state = 1;
        } else {
            w->state = 3;
        }
        if (w->atk != NULL) {
            frmDestroy((Frame *) w->atk);
        }
        if (w->body != NULL) {
            frmDestroy((Frame *) w->body);
        }
        if (w->model != NULL) {
            frmDestroy((Frame *) w->model);
        }
        if ((w->model = (AnimeFrame *) eneNewPol(o, etskNuriDl)) != NULL) {
            w->model->scale[2] = 0.05f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(pl_atk_root_frame, &w->model->f, 60, 500, 0, 0)) != NULL) {
                w->atk->kind = 0x78;
            }
            w->tilt = 0.0f;
        }
    }
}

/* The task whose proc is Etsk_konuri_main (it calls the four steps below through their
 * addresses): what they touch. */
typedef struct Tlb80SwingW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ Frame *frame;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 angle; /* in radians */
    /* 0x20 */ f32 rotY;
    /* 0x24 */ f32 rotZ;
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u8 pad34[0x14];
    /* 0x48 */ u32 flags;
    /* 0x4C */ u8 pad4C[0xA];
    /* 0x56 */ s16 timer;
} Tlb80SwingW;

/* Swings back by 0.1; at -45 degrees, moves its frame to layer 8 and goes to state 2. */
void konuriSwingBack(Obj *o) {
    s32 pad;
    Tlb80SwingW *w = (Tlb80SwingW *) FX(o);

    ((Tlb80SwingW *) FX(o))->angle -= 0.1;
    if (((Tlb80SwingW *) FX(o))->angle <= -0.7853982f) {
        frmMove(w->frame, gFrmLayer8);
        w->state = 2;
    }
}

/* Swings back by 0.1 while the timer runs down by 48; under 64, it moves 10 on, is set on the
 * course again, and swaps flag 1 for flag 0x80000. */
void konuriSwingBackEnd(Obj *o) {
    s32 pad;
    Tlb80SwingW *w = (Tlb80SwingW *) FX(o);

    ((Tlb80SwingW *) FX(o))->angle -= 0.1;
    ((Tlb80SwingW *) FX(o))->timer -= 48;
    if (((Tlb80SwingW *) FX(o))->timer < 64) {
        w->x += 10.0f;
        if (w->frame != NULL) {
            eneSetModelOnCourse(o, w->frame);
        }
        w->flags &= ~1;
        w->flags |= 0x80000;
    }
}

/* Swings on by 0.1; at 45 degrees, moves its frame to layer 8 and goes to state 4. */
void konuriSwingOn(Obj *o) {
    s32 pad;
    Tlb80SwingW *w = (Tlb80SwingW *) FX(o);

    ((Tlb80SwingW *) FX(o))->angle += 0.1;
    if (((Tlb80SwingW *) FX(o))->angle >= 0.7853982f) {
        frmMove(w->frame, gFrmLayer8);
        w->state = 4;
    }
}

/* konuriSwingBackEnd the other way: swings on, moves 10 back. */
void konuriSwingOnEnd(Obj *o) {
    s32 pad;
    Tlb80SwingW *w = (Tlb80SwingW *) FX(o);

    ((Tlb80SwingW *) FX(o))->angle += 0.1;
    ((Tlb80SwingW *) FX(o))->timer -= 48;
    if (((Tlb80SwingW *) FX(o))->timer < 64) {
        w->x -= 10.0f;
        if (w->frame != NULL) {
            eneSetModelOnCourse(o, w->frame);
        }
        w->flags &= ~1;
        w->flags |= 0x80000;
    }
}
