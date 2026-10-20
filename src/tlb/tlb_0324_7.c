#include "tlb.h"
#include "tlb/tlb_0324.h"

#define DPRINTF

/* The barrier task's (btaiUtsuboProc) argument: only what its starter sets. */
typedef struct BarriArg {
    /* 0x00 */ u8 pad00[0x48];
    /* 0x48 */ s32 slot; /* its place, from btaiUtsuboTakeSlot */
    /* 0x4C */ u8 pad4C[0x7];
    /* 0x53 */ u8 side;
    /* 0x54 */ u8 pad54[0x4];
} BarriArg;

/* A moray eel's ("Utsubo", btaiUtsuboProc) work. */
typedef struct BarriW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ f32 aim[3]; /* random, x and z (btaiUtsuboAttack) */
    /* 0x1C */ Anim anim; /* its texture animation */
    /* 0x2C */ f32 scale; /* grows to 1 by 0.02 a frame */
    /* 0x30 */ f32 speed; /* btaiUtsuboChase's: up by 0.2 a frame to 15 */
    /* 0x34 */ ACollFrame *box0; /* its attack box */
    /* 0x38 */ ACollFrame *box1; /* its damage box */
    /* 0x3C */ f32 tilt;
    /* 0x40 */ s32 phase; /* its sway's, 0-359 */
    /* 0x44 */ s32 timer;
    /* 0x48 */ s32 slot; /* its slot in btaiBarriSlots */
    /* 0x4C */ s32 alpha; /* up by 6 to 120 */
    /* 0x50 */ u8 state;
    /* 0x51 */ u8 biting; /* 1: its bite counts */
    /* 0x52 */ u8 knockedBack; /* 1: knocked back */
    /* 0x53 */ u8 side; /* from its argument */
} BarriW;

/* Three slots (the eels' positions), each with an in-use flag and its eel (tlb_0324's .bss). */
typedef struct BarriSlot {
    /* 0x00 */ f32 pos[3];
    /* 0x0C */ s32 used;
    /* 0x10 */ Obj *obj;
} BarriSlot; /* size 0x14 */

extern BarriSlot btaiBarriSlots[3];
extern u8 btaiUtsuboTex0[];
s32 btaiEelsMade = 1; /* eels made */
AnimFrame btaiUtsuboAnim[] = {
    { (u32) btaiUtsuboTex0, 10 },
    { 0x030019E0, 50 },
    { 0x00000000, 0 },
}; /* the eel's texture animation */
extern u8 btaiUtsuboEelAnim[]; /* the eel's animation */
extern s32 btaiWhirlPiecesEnd; /* 1 ends the boss's pieces */
extern CamView *gBattleCamView; /* the battle's camera (beffect.c) */
void btaiUtsuboChase(Obj *obj);
void btaiUtsuboNeutral(Obj *obj);
void btaiUtsuboAttack(Obj *obj);
void btaiUtsuboEscape(Obj *obj);
void btaiUtsuboEnd(Obj *obj);
void btaiUtsuboMove(Obj *obj);

typedef ACollFrame *(*Tlb324AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*Tlb324DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                          s32 a);

/* "Utsubo", one of the sea boss's moray eels: an animated model (file 0xF9) with a texture
 * animation, an attack box (radius 200, kind 0x30) and a damage box (150 by 150, kind 0x31). It
 * comes from z -2300 at a random x and y, aimed at a random x, grows from a hundredth (a
 * thousandth on the other side) by 0.02 a frame and sways with a sine of its frame count. Each
 * frame its state's step runs; it is gone past z 200 or -4000, or when the boss ends its pieces.
 * Hit by kinds 3, 11, 5 or 8 it is knocked back 100 and flees (state 5), with a hit ring and
 * sound 0x150; kind 1 bounces it back at a fifth of its speed. Its bite hits the player
 * (btaiHagataMake, sound 0x27E). It holds one of the battle's three slots; destroyed, it frees
 * it. The starter's and the steps' prints are DPRINTFs; each step's sits at its start, a guess
 * (they emit no code).
 * Matched 2026-10-12; it was parked for its scale chains, w set once at its declaration, and a
 * wrong field (the sway is rot[0]). */
s32 btaiUtsuboProc(Obj *obj, s16 msg, void *arg);
s32 btaiUtsuboProc(Obj *obj, s16 msg, void *arg) {
    BarriW *w = (BarriW *) FX(obj);
    s32 pad;
    s32 i;
    s32 n;
    s32 pad2; /* FAKE: a stack word (a removed local's); without it real 25 */

    switch (msg) {
        case 2:
            obj->name = "Utsubo";
            if ((w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime)) == NULL) {
                return -1;
            }
            btaiEelsMade++;
            bossSetAnimAt(w->frame, btaiUtsuboEelAnim, 0xF9, 0.0f);
            fadeAnimStart(&w->anim, btaiUtsuboAnim, 0, &w->frame->segs[3], floadGetSetFile(0xF9, 0));
            if ((w->box0 = ((Tlb324AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 200, 0, 0, 0)) ==
                NULL) {
                return -1;
            }
            w->box0->kind = 0x30;
            if ((w->box1 = ((Tlb324DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->frame->f, 150, 150, -100, 100,
                                                                 0)) == NULL) {
                return -1;
            }
            w->box1->kind = 0x31;
            w->frame->pos[0] = (*btaiRandSigned)(100);
            w->frame->pos[1] = (*btaiRandSigned)(100);
            w->frame->pos[2] = -2300.0f;
            w->vel[0] = 0.0f;
            w->vel[2] = 0.0f;
            w->vel[1] = 100.0f;
            w->frame->rot[1] = 0.0f;
            w->aim[0] = (*btaiRandSigned)(10);
            w->aim[2] = -2000.0f;
            w->side = ((BarriArg *) arg)->side;
            w->speed = 0.0f;
            if (w->side) {
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale = 0.01f;
                w->tilt = 0.0f;
            } else {
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale = 0.001f;
                w->tilt = 0.0f;
            }
            w->slot = ((BarriArg *) arg)->slot;
            btaiBarriSlots[w->slot].obj = obj;
            w->phase = 0;
            w->state = 1;
            w->timer = 100;
            w->biting = 0;
            w->knockedBack = 0;
            w->alpha = 0;
            break;
        case 1:
            DPRINTF("うつぼ ID:%d を消します \n", w->slot);
            btaiBarriSlots[w->slot].used = 0;
            n = 0;
            for (i = 0; i < 3; i++) {
                if (btaiBarriSlots[w->slot].used == 1) {
                    n++;
                }
            }
            DPRINTF("残りウツボ数 : %d\n", n);
            if (n == 0) {
                DPRINTF("ウツボは全て消しました\n");
                DPRINTF("おやびん出現!!\n");
            }
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            if (w->box0 != NULL) {
                frmDestroy(&w->box0->frame);
            }
            if (w->box1 != NULL) {
                frmDestroy(&w->box1->frame);
            }
            break;
        case 0:
            if (++w->phase >= 360) {
                w->phase = 0;
            }
            fadeAnimStep(&w->anim);
            if (w->scale < 1.0f) {
                w->scale += 0.02;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale;
            }
            if (btaiWhirlPiecesEnd == 1) {
                return -1;
            }
            switch (w->state) {
                case 1:
                    (*btaiUtsuboChase)(obj);
                    break;
                case 0:
                    (*btaiUtsuboNeutral)(obj);
                    break;
                case 3:
                    (*btaiUtsuboAttack)(obj);
                    break;
                case 4:
                    (*btaiUtsuboEscape)(obj);
                    break;
                case 5:
                    (*btaiUtsuboEnd)(obj);
                    break;
            }
            if (w->frame->pos[2] >= 200.0f) {
                return -1;
            }
            if (w->frame->pos[2] < -4000.0f) {
                return -1;
            }
            (*btaiUtsuboMove)(obj);
            w->frame->rot[0] = (*sinf)(w->phase * 0.0174532930038800771) * 0.0174532930038800771 * 10.0 + w->tilt;
            if (w->frame->rot[0] > 6.2831855f) {
                w->frame->rot[0] -= 6.2831855f;
            }
            if (w->frame->rot[0] < 0.0f) {
                w->frame->rot[0] += 6.2831855f;
            }
            if (acollDmgWasHit(w->box1) && w->knockedBack == 0) {
                if (w->box1->hitKind == 3 || w->box1->hitKind == 11 || w->box1->hitKind == 5 ||
                    w->box1->hitKind == 8) {
                    (*btaiU3bubbleMake)(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2]);
                    w->knockedBack = 1;
                    w->biting = 0;
                    w->tilt = 0.0f;
                    w->vel[2] = -70.0f;
                    w->frame->pos[2] -= 100.0f;
                    w->state = 5;
                    w->box1->flags &= ~2;
                    w->box0->flags &= ~2;
                    beHitringCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f, 0);
                    SoundLocation2(0x150, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                                   15000.0f, gBattleCamView->fovy);
                }
                if (w->box1->hitKind == 1) {
                    SoundLocation2(0x150, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                                   15000.0f, gBattleCamView->fovy);
                    w->vel[0] = -w->vel[0] * 0.2;
                    w->vel[2] = -w->vel[2] * 0.2;
                }
            }
            if (acollAtkWasHit(w->box0) == 1 && w->biting == 1) {
                (*btaiHagataMake)(obj, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2]);
                w->biting = 0;
                w->box0->flags &= ~2;
                SoundLocation2(0x27E, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

extern Obj *gBattleTask; /* the battle's task (beffect.c) */
s32 btaiUtsuboProc(Obj *obj, s16 msg, void *arg);

/* Starts a barrier in the first free slot, on one side or the other, under the battle's task;
 * nothing when all three slots are taken. */
void btaiUtsuboCreate(s32 side) {
    BarriArg arg;

    if (side) {
        arg.side = 1;
    } else {
        arg.side = 0;
    }
    arg.slot = (*btaiUtsuboTakeSlot)();
    if (arg.slot >= 0) {
        tskCreate(gBattleTask, btaiUtsuboProc, gBattleTask->file, &arg);
        DPRINTF("うつぼ ID:%d をつくりました\n", arg.slot);
    } else {
        DPRINTF("うつぼはつくれませんでした。\n");
    }
}

/* The eel's work as its chase reads it (BarriW; btaiUtsuboChase's only caller is btaiUtsuboProc). */
typedef struct BossChaseW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ f32 target[3]; /* the place it heads for */
    /* 0x1C */ Anim anim;
    /* 0x2C */ f32 scale;
    /* 0x30 */ f32 speed;
    /* 0x34 */ ACollFrame *box0; /* its attack box: bit 1 of its flags cleared while it chases */
    /* 0x38 */ ACollFrame *box1;
    /* 0x3C */ f32 tilt;
    /* 0x40 */ s32 phase;
    /* 0x44 */ s32 timer;
    /* 0x48 */ s32 slot;
    /* 0x4C */ s32 alpha;
    /* 0x50 */ u8 state;
} BossChaseW;

/* The boss chases its target: its animation steps at 0.3; it speeds up by 0.2 a frame to 15;
 * within 200 of the target, or when the timer runs out, it stops for 70 frames. Its x and z
 * speeds lean 0.2 a frame toward the target (to 15, or 18 forward), and it animates at 0.07
 * coming forward, 0.15 going back. The upper clamps of speed and vel[0] store an integer 15 (a
 * second lui, as in the game); the others reuse the compare's register.
 * Matched 2026-10-12; it was parked for its float colouring (0.2's register), gone with the two
 * empty readers of the sum below. */
void btaiUtsuboChase(Obj *obj) {
    BossChaseW *w = (BossChaseW *) FX(obj);
    f32 dx;
    f32 dz;

    bossStepAnim(w->frame, 0.3f, 0);
    w->timer--;
    w->box0->flags &= ~2;
    w->speed += 0.2;
    if (w->speed > 15.0f) {
        w->speed = 15;
    }
    dx = w->target[0] - w->frame->pos[0];
    dz = w->target[2] - w->frame->pos[2];
    /* FAKE: two empty readers of the sum (IDO_NOTES 204's family). They make it a common value
     * that uopt colors before 0.2 (sum f12, pos0/target0 f14/f16, 0.2 f18, as the game's); they
     * stand for compiled-out debug uses. One reader gave 0.2 f12 (real 15). A kept FAKE
     * (2026-10-17): a `dist` local in the test instead is real 83. */
    if (dx * dx + dz * dz) {
    }
    if (dx * dx + dz * dz) {
    }
    if (dx * dx + dz * dz < 40000.0f || w->timer <= 0) {
        w->state = 0;
        w->timer = 70;
    }
    if (w->frame->pos[0] < w->target[0]) {
        w->vel[0] += 0.2;
        if (w->vel[0] > 15) {
            w->vel[0] = 15;
        }
    } else {
        w->vel[0] -= 0.2;
        if (w->vel[0] < -15.0f) {
            w->vel[0] = -15.0f;
        }
    }
    if (w->frame->pos[2] < w->target[2]) {
        w->vel[2] += 0.2;
        if (w->vel[2] > 18.0f) {
            w->vel[2] = 18.0f;
        }
        bossStepAnim(w->frame, 0.07f, 0);
    } else {
        w->vel[2] -= 0.2;
        if (w->vel[2] < -15.0f) {
            w->vel[2] = -15.0f;
        }
        bossStepAnim(w->frame, 0.15f, 0);
    }
    w->vel[1] = 0.0f;
}

AnimFrame btaiBarrierAnim[] = { { 0x030019E0, 5 }, { (u32) btaiUtsuboTex0, 250 }, { 0x030019E0, 5 }, { 0x00000000, 0 } }; /* the barrier's texture animation */

/* A barrier step: plays its animation at 0.15, keeps both boxes' bit 2 set while its timer runs
 * down; at 0 it goes to state 3 and starts its texture animation (file 0xF9). Its z speed
 * eases by 0.2 a frame. */
void btaiUtsuboNeutral(Obj *obj) {
    BarriW *w = (BarriW *) FX(obj);
    s32 pad[2];

    DPRINTF("UTU id:%dニュートラル\n", w->slot);
    bossStepAnim(w->frame, 0.15f, 0);
    (*btaiTurnTo)(obj, 0.0f, 0.0f, 0.3f);
    w->timer--;
    w->box1->flags |= 2;
    w->box0->flags |= 2;
    if (w->timer <= 0) {
        w->state = 3;
        w->biting = 1;
        w->box0->flags |= 2;
        fadeAnimStart(&w->anim, btaiBarrierAnim, 0, &w->frame->segs[3], floadGetSetFile(0xF9, 0));
    }
    if (w->vel[2] > 0.0f) {
        w->vel[2] -= 0.2;
    } else {
        w->vel[2] += 0.2;
    }
}

extern AnimFrame btaiUtsuboAnim[]; /* the barrier's bounce animation */

/* A barrier step: plays its animation at 0.3, rises 1 faster a frame (to 90 on side 1, 70 on
 * side 0), fades in by 6 to 120, spins 10 degrees a frame and draws in toward the middle. On its
 * first frame here it turns its rise over, takes a new random aim and starts its bounce
 * animation (file 0xF9); above 200 it bounces down at half speed and does the same. */
void btaiUtsuboAttack(Obj *obj) {
    BarriW *w = (BarriW *) FX(obj);
    s32 pad[2];

    DPRINTF("UTU id%d:攻撃\n", w->slot);
    bossStepAnim(w->frame, 0.3f, 0);
    (*btaiTurnTo)(obj, 0.0f, 0.0f, 0.3f);
    w->vel[2] += 1.0f;
    if (w->side) {
        if (w->vel[2] > 90.0f) {
            w->vel[2] = 90.0f;
        }
    } else {
        if (w->vel[2] > 70.0f) {
            w->vel[2] = 70.0f;
        }
    }
    if (w->alpha < 120) {
        w->alpha += 6;
    }
    w->frame->rot[2] += 0.174532930038800771;
    if (w->frame->rot[2] > 6.2831855f) {
        w->frame->rot[2] -= 6.2831855f;
    }
    w->frame->pos[0] *= 0.94;
    w->frame->pos[1] *= 0.99;
    if (w->biting == 0) {
        w->state = 4;
        w->vel[2] = -w->vel[2];
        w->aim[0] = (*btaiRandSigned)(100);
        w->aim[2] = (*btaiRandSigned)(800) - 2000;
        fadeAnimStart(&w->anim, btaiUtsuboAnim, 0, &w->frame->segs[3], floadGetSetFile(0xF9, 0));
    }
    if (w->frame->pos[2] > 200.0f) {
        w->vel[2] = -w->vel[2] / 2;
        w->frame->pos[2] = 200.0f;
        w->biting = 0;
        w->state = 4;
        w->aim[0] = btaiRandSigned(100);
        w->aim[2] = btaiRandSigned(800) - 2000;
        fadeAnimStart(&w->anim, btaiUtsuboAnim, 0, &w->frame->segs[3], floadGetSetFile(0xF9, 0));
    }
    w->vel[0] = 0.0f;
    w->vel[1] = 0.0f;
}

/* A barrier step: plays its animation at 0.2 with box 1's bit 2 set, levels its tilt by 4
 * degrees a frame, speeds up in z by 1 to at most 70, and moves 4 a frame in x and y by the
 * sign of its position. */
void btaiUtsuboEscape(Obj *obj) {
    BarriW *w = (BarriW *) FX(obj);

    DPRINTF("UTU id:%dえすけーぷ\n", w->slot);
    bossStepAnim(w->frame, 0.2f, 0);
    w->box1->flags |= 2;
    if (w->tilt < -0.0698131720155203139) {
        w->tilt += 0.0698131720155203139;
    }
    if (w->tilt > 0.0698131720155203139) {
        w->tilt -= 0.0698131720155203139;
    }
    w->vel[2] += 1.0f;
    if (w->vel[2] > 70.0f) {
        w->vel[2] = 70.0f;
    }
    if (w->frame->pos[0] > 0.0f) {
        w->vel[0] = 4.0f;
    } else {
        w->vel[0] = -4.0f;
    }
    if (w->frame->pos[1] > 0.0f) {
        w->vel[1] = 4.0f;
    } else {
        w->vel[1] = -4.0f;
    }
}

/* A barrier step: plays the animation, tilts back by 2 degrees a frame to at most 40, and rises
 * ever faster (2 a frame); once it is moving up it goes on to state 4. Its sideways speeds
 * stay 0. */
void btaiUtsuboEnd(Obj *obj) {
    BarriW *w = (BarriW *) FX(obj);

    DPRINTF("UTU id:%d 終り\n", w->slot);
    bossStepAnim(w->frame, 0.2f, 0);
    w->frame->rot[2] = 0.0f;
    w->tilt -= 0.034906586007760157;
    if (w->tilt < -0.698131720155203084) {
        w->tilt = -0.6981317401f;
    }
    w->vel[2] += 2.0f;
    if (w->vel[2] > 0.0f) {
        w->state = 4;
    }
    w->vel[0] = 0.0f;
    w->vel[1] = 0.0f;
}


/* Moves a barrier by its speed after recording where it was in its slot; near another barrier
 * (within 300 deep and 100 across or up) it undoes its own move across and up and steps 3
 * away in depth. */
void btaiUtsuboMove(Obj *obj) {
    BarriW *w = (BarriW *) FX(obj);
    s32 i;

    btaiBarriSlots[w->slot].pos[0] = w->frame->pos[0];
    btaiBarriSlots[w->slot].pos[1] = w->frame->pos[1];
    btaiBarriSlots[w->slot].pos[2] = w->frame->pos[2];
    w->frame->pos[2] += w->vel[2];
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    for (i = 0; i < 3; i++) {
        if (i != w->slot && btaiBarriSlots[i].used == 1) {
            if (((btaiBarriSlots[i].pos[2] < w->frame->pos[2]) ? w->frame->pos[2] - btaiBarriSlots[i].pos[2]
                                                         : -(w->frame->pos[2] - btaiBarriSlots[i].pos[2])) < 300.0f) {
                if (((btaiBarriSlots[i].pos[0] < w->frame->pos[0]) ? w->frame->pos[0] - btaiBarriSlots[i].pos[0]
                                                             : -(w->frame->pos[0] - btaiBarriSlots[i].pos[0])) < 100.0f) {
                    if (w->vel[0] > 0.0f) {
                        w->frame->pos[0] -= w->vel[0];
                    } else {
                        w->frame->pos[0] += w->vel[0];
                    }
                }
                if (((btaiBarriSlots[i].pos[1] < w->frame->pos[1]) ? w->frame->pos[1] - btaiBarriSlots[i].pos[1]
                                                             : -(w->frame->pos[1] - btaiBarriSlots[i].pos[1])) < 100.0f) {
                    if (w->vel[1] > 0.0f) {
                        w->frame->pos[1] -= w->vel[1];
                    } else {
                        w->frame->pos[1] += w->vel[1];
                    }
                }
                if (w->frame->pos[2] < btaiBarriSlots[i].pos[2]) {
                    w->frame->pos[2] -= 3.0f;
                } else {
                    w->frame->pos[2] += 3.0f;
                }
            }
        }
    }
}
