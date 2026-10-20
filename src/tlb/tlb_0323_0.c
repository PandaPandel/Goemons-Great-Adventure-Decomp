#include "tlb.h"
#include "tlb/tlb_0323.h"

/* "Bbisu_proc": the Bismaru robot boss ("ビスマル"), the Goemon Impact's third boss. One task
 * drives the whole battle: its body (one model, file 0xF4, with a shadow), the attack and damage
 * boxes, the entrance demo, the attacks, the hits and the two endings. It is the template of
 * Bdev_proc (tlb_0327_0) without Bdev's cape and after-images; Bismaru also has "Bboost" thrust
 * columns under it, a love barrier ("Lbarri") and a heart beam ("Hbeam").
 *
 * Its debug prints (the hits, "Sstep/Stimer", the demos, BOSSLIMIT) are compiled out;
 * DPRINTF keeps each string, in source order, and emits no code. */

/* A print the original left in: keeps its string, emits no code. */
#define DPRINTF

extern Obj *gBattleTask; /* the battle's task (beffect.c) */
extern CamView *gBattleCamView; /* the battle's camera (beffect.c) */

#define sinf (*sinf)
#define cosf (*cosf)

/* acollAtkAddYZ and acollDmgAddYZ return their box (main declares them void). */
typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                    s32 a);

/* vramCheck (bmove.c) as this file calls it: the ray's start passed whole. */
typedef void (*VramCheckFunc)(AnimeFrame *f, Vec3f from, f32 down, f32 above);

/* This boss's other parts (tlb_0323_1). */
#define bisuHbeamProc (*bisuHbeamProc)
#define bisuNeedleCreate (*bisuNeedleCreate)
#define bisuRoseCreate (*bisuRoseCreate)
#define bisuHpunchCreate (*bisuHpunchCreate)
#define bisuLbarriProc (*bisuLbarriProc)
#define bisuPlateProc (*bisuPlateProc)
#define bisuBVaniProc (*bisuBVaniProc)
#define bisuHpunch2Create (*bisuHpunch2Create)
#define bisuBboostCreate (*bisuBboostCreate)
#define bisuBVDemoProc (*bisuBVDemoProc)
#define bisuBplateDProc (*bisuBplateDProc)
#define bisuBroseDProc (*bisuBroseDProc)
#define bisuBappcamProc (*bisuBappcamProc)
#define bisuHeffectCreate (*bisuHeffectCreate)
#define bisuHringCreate (*bisuHringCreate)

extern u8 bbisuBodyTex0[];
extern u8 bbisuBodyTex1[];
AnimFrame bbisuBodyAnim[] = {
    { (u32) bbisuBodyTex0, 15 },
    { (u32) bbisuBodyTex1, 15 },
    { 0, 0 },
    { 0, 0 },
}; /* the body's texture animation (file 0xF3) */

/* The model animations, in file 0xF4. */
extern u8 bbisuIdleAnim0[]; /* idle */
extern u8 bbisuIdleAnim1[];
extern u8 bbisuIdleAnim2[];
extern u8 bbisuIdleAnim3[];
extern u8 bbisuPunchWindUpAnim0[]; /* the punch: wind up */
extern u8 bbisuPunchWindUpAnim1[];
extern u8 bbisuPunchWindUpAnim2[];
extern u8 bbisuKnockedBackAnim0[]; /* knocked back (one side) */
extern u8 bbisuKnockedBackAnim1[]; /* knocked back (the other side) */
extern u8 bbisuThrownDownAnim0[]; /* thrown down */
extern u8 bbisuThrownDownAnim1[];
extern u8 bbisuThrownDownAnim2[];
extern u8 bbisuDefeatedAnim[]; /* defeated */
extern Gfx bbisuLosingDemoPlateDl[]; /* the losing demo's plate (file 0xF6) */

/* The boss's work (at obj + 0x20). */
typedef struct BbisuW {
    /* 0x00 */ AnimeFrame *frame; /* the body */
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ ACollFrame *atk; /* its attack box (kind 0xCD) */
    /* 0x18 */ ACollFrame *dmg; /* its damage box (kind 0xCC) */
    /* 0x1C */ MPolFrame *shadow;
    /* 0x20 */ Anim anim; /* the body's texture animation */
    /* 0x30 */ PolFrame *plate; /* the losing demo's */
    /* 0x34 */ u8 pad34[0x18];
    /* 0x4C */ s32 hitWait; /* steps before the damage box is live again */
    /* 0x50 */ u8 action; /* "Mstep" (bossStartAction sets it) */
    /* 0x51 */ u8 step; /* "Sstep" */
    /* 0x52 */ u8 pad52[2];
    /* 0x54 */ s32 timer; /* "Stimer" */
    /* 0x58 */ u8 count; /* a choice (fadeRand), or steps before the damage box is live */
    /* 0x5C */ s32 glow; /* the barrier's yellow glow, 0-192, while gBossFlags 0x4000 */
} BbisuW;

#define BBISU(obj) ((BbisuW *) ((u32) (obj) + sizeof(Obj)))

/* Angles: one degree. */
#define DEG 0.0174532930038800785

/* The frame count, as this file reads it. */
#define FRAMES ((u32) gGameState.frameCount)

/* Where the boss is, for the others. */
#define POS_OUT(w)                          \
    gBossPosX = w->frame->pos[0];           \
    gBossPosY = w->frame->pos[1];           \
    gBossPosZ = w->frame->pos[2];           \
    gBossYaw = w->frame->rot[1];

/* The body leans with the velocity. */
#define TILT(w)                                      \
    w->frame->rot[0] = w->vel[2] * DEG * 1;          \
    w->frame->rot[2] = -w->vel[0] * DEG * 1;

/* The body moves by its velocity. */
#define MOVE(w)                          \
    w->frame->pos[0] += w->vel[0];       \
    w->frame->pos[1] += w->vel[1];       \
    w->frame->pos[2] += w->vel[2];

/* The body faces the arena's middle. */
#define FACE(w) w->frame->rot[1] = mathAtan2(-w->frame->pos[0], -w->frame->pos[2]);

/* A column of thrust under the boss every ten frames. */
#define BOOST(w)                                                                                    \
    if (FRAMES % 10 == 0) {                                                                         \
        bisuBboostCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 100.0f);       \
    }

/* A new body animation, with its texture animation. */
#define BODY_ANIM(w, data, at, index)                                   \
    bossSetAnimAt(w->frame, (data), 0xF4, at);                          \
    fadeAnimStart(&w->anim, bbisuBodyAnim, index, &w->frame->segs[3], floadGetSetFile(0xF3, 0));

/* The damage box at its full size (it shrinks while the boss is down). */
#define DMG_BOX(w, r, h, oy)        \
    w->dmg->radius = r;             \
    w->dmg->height = h;             \
    w->dmg->ofs[1] = oy;

/* The ray down to the ground found it: the body stands on it. */
#define ON_GROUND(w) w->frame->pos[1] = at.y + 200.0f + 0.1f;

/* Away from the arena's middle at a speed. A macro is one source line, and IDO schedules by
 * line (IDO_NOTES 12, 324): where the game's code has the three statements on their own lines
 * (the straight's own knock-back) they are written out. */
#define AWAY(w, speed)                                                 \
    angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);             \
    w->vel[0] = sinf(angle) * speed;                                   \
    w->vel[2] = cosf(angle) * speed;

/* The punch toward the cockpit, from one of three places: an "Hpunch" and its sound. */
#define HPUNCH(x, z, type)                                                                         \
    bisuHpunchCreate(x, 0.0f, z, type);                                                            \
    sndRequest(0x27F, soundPanGet(gBattleCamView, x, z, gBattleCamView->fovy), 0xF0);

/* The game reads msg back from its home, `lhu` for the switch and `lh` for tskDefaultProc: a
 * plain `switch (msg)` on the s16 compiles to both here (measured; the switch's range check is
 * an sltiu). A kept FAKE (2026-10-17): that replaces `switch (*(u16 *) &msg)`;
 * `switch ((u16) msg)` is real 5056 (lh, then andi), an ANSI `u16 msg` 68 (andi a1, a spill).
 * Action 0's life tests are one-line ifs (no braces), the game's style: they replace the FAKE
 * layout of the 0.33333 test and the decrement on one line (MATCH, object identical). */
s32 Bbisu_proc(Obj *obj, s16 msg, void *arg) {
    /* The pads are the game's unused words: two above the locals, one between hx and hz, and
     * one below angle that puts the compiler temps (the cached call addresses at 0x74-0x80,
     * fadeRandRange's at 0x88/0x8C) at the game's slots. */
    s32 pad0[2];
    Vec3f ray; /* a ray down to the ground, from under the body */
    Vec3f at; /* where it hits */
    f32 tp;
    f32 hx; /* a point ahead of the cockpit (bossHeadingOffset) */
    s32 pad1;
    f32 hz;
    f32 angle;
    s32 pad2;
    f32 y; /* the shadow's height */
    BbisuW *w = BBISU(obj);
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "Bbisu_proc";
            gBattleTask = obj;
            if ((w->frame = (AnimeFrame *) frmCreate(gFrmLayer2, gFrmKindAnime)) == NULL) {
                return -1;
            }
            w->shadow = shadowCreate();
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(uda_atk_root_frame, &w->frame->f, 200, 0, 0, 0)) ==
                NULL) {
                return -1;
            }
            w->atk->kind = 0xCD;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 2.0f;
            if ((w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->frame->f, 200, 500, -300, 0,
                                                          0)) == NULL) {
                return -1;
            }
            w->dmg->kind = 0xCC;
            BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
            bossSetFrameFlags2(w->frame, 0x20);
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = -1500.0f;
            w->action = 0;
            w->step = 0;
            w->timer = 0;
            gBossLifeMax = 2000;
            gBossLife = 2000;
            gBossActionArg = 0;
            gBossPhase = 0;
            gBossActionCount = 0;
            DPRINTF("登場デモ開始\n");
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->atk->frame);
            frmDestroy(&w->dmg->frame);
            frmDestroy((Frame *) w->shadow);
            break;
        case 4:
            /* Where the boss is. */
            if (arg != NULL) {
                ((f32 *) arg)[0] = w->frame->pos[0];
                ((f32 *) arg)[1] = w->frame->pos[1];
                ((f32 *) arg)[2] = w->frame->pos[2];
            }
            break;
        case 0:
            if (gBattleFlags2 & 0x4000) {
                if (gImpactMoveTime > 80) {
                    w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 3.0f;
                } else {
                    w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 2.0f;
                }
            } else {
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 2.0f;
            }
            /* The battle opens with the entrance demo. */
            if (!(gBattleFlags2 & 4)) {
                gBattleFlags2 |= 4;
                bossStartAction(obj, 0, 14);
            }
            gMegaBurstY = gBossPosY;
            ray.x = w->frame->pos[0];
            ray.y = w->frame->pos[1] - 600.0f;
            ray.z = w->frame->pos[2];
            acollAtkWasHit(w->atk);

            /* The hits. */
            if (acollDmgWasHit(w->dmg)) {
                if (w->dmg->hitKind == 15) {
                    DPRINTF("ビスマルはバルカンに当たりました。[%d]\n", gBossLife);
                    if (gBossFlags & 4) {
                        bossDamage(0, 15, 1);
                    } else {
                        itexFlash(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 1.0f);
                        beBpieceCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f, 10);
                        SoundLocation2(0x22B, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                       w->dmg->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                        bossDamage(3, 15, 1);
                    }
                    if (gBossLife <= 0) {
                        DPRINTF("ボス死亡しました。\n");
                        gBattleFlags2 |= 0x400;
                        w->dmg->flags &= ~2;
                        AWAY(w, 15.0f);
                        w->count = 30;
                        w->action = 0x2B;
                        w->step = 0;
                        w->vel[1] = 120.0f;
                    }
                } else if (w->dmg->hitKind == 16) {
                    DPRINTF("ビスマルは外部バルカンに当たりました。\n");
                    if (gBossFlags & 4) {
                        bossDamage(0, 16, 1);
                    } else {
                        itexFlash(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 1.0f);
                        beBpieceCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f, 10);
                        SoundLocation2(0x22B, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                       w->dmg->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                        bossDamage(3, 16, 1);
                    }
                    if (gBossLife <= 0) {
                        DPRINTF("ボス死亡しました。\n");
                        gBattleFlags2 |= 0x400;
                        w->dmg->flags &= ~2;
                        AWAY(w, 15.0f);
                        w->count = 30;
                        w->action = 0x2B;
                        w->step = 0;
                        w->vel[1] = 120.0f;
                    }
                } else if (w->dmg->hitKind == 1) {
                    DPRINTF("ビスマルはジャブに当たりました。\n");
                    if (gBossFlags & 0x4000) {
                        DPRINTF("効かぬわ。\n");
                    } else {
                        beHitmarkCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 3.0f);
                        beBpieceCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f, 10);
                        beBpieceCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f, 10);
                        SoundLocation2(0x27C, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                       w->dmg->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                        if (gBossFlags & 0x1000) {
                            bossDamage(0, 1, 1);
                        } else {
                            bossDamage(20, 1, 1);
                        }
                        w->dmg->flags &= ~2;
                        gBossFlags &= ~0x400;
                        gBrotherFlags &= ~0x20000;
                        /* Flinch away from the side hit. */
                        if (w->frame->pos[0] < w->dmg->hitPos[0]) {
                            w->action = 0x2D;
                        } else {
                            w->action = 0x2C;
                        }
                        w->step = 0;
                    }
                    if (gBossLife <= 0) {
                        DPRINTF("ボス死亡しました。\n");
                        gBattleFlags2 |= 0x400;
                        w->dmg->flags &= ~2;
                        AWAY(w, 15.0f);
                        w->count = 30;
                        w->action = 0x2B;
                        w->step = 0;
                        w->vel[1] = 120.0f;
                    }
                } else if (w->dmg->hitKind == 3) {
                    DPRINTF("ビスマルはストレートに当たりました。\n");
                    if (gBossFlags & 0x4000) {
                        DPRINTF("効かぬわ。\n");
                    } else {
                        beHitmarkCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f);
                        beBpieceCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f, 10);
                        beBpieceCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f, 10);
                        SoundLocation2(0x27D, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                       w->dmg->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                        w->dmg->flags &= ~2;
                        bossDamage(50, 3, 2);
                        gBossFlags &= ~0x200;
                        gBossFlags &= ~0x400;
                        gBrotherFlags &= ~0x20000;
                        angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                        w->vel[0] = sinf(angle) * 15.0f;
                        w->vel[2] = cosf(angle) * 15.0f;
                        w->count = 30;
                        w->action = 0x2B;
                        w->step = 0;
                        w->vel[1] = 120.0f;
                    }
                    if (gBossLife <= 0) {
                        DPRINTF("ボス死亡しました。\n");
                        gBattleFlags2 |= 0x400;
                        w->dmg->flags &= ~2;
                        AWAY(w, 15.0f);
                        w->count = 30;
                        w->action = 0x2B;
                        w->step = 0;
                        w->vel[1] = 120.0f;
                    }
                } else if (w->dmg->hitKind == 8) {
                    DPRINTF("ビスマルはキックに当たりました。\n");
                    if (gBossFlags & 0x4000) {
                        DPRINTF("効かぬわ。\n");
                    } else {
                        bossDamage(80, 8, 3);
                        sndPlay(0x102);
                        beHitmarkCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f);
                        beBpieceCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f, 10);
                        beBpieceCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f, 10);
                        gBossFlags &= ~0x400;
                        w->dmg->flags &= ~2;
                        gBossFlags &= ~0x200;
                        gBrotherFlags &= ~0x20000;
                        /* The fifth kick down sends him spinning (action 0x31). */
                        bossAddDownCount(1);
                        if (bossDownCountOver(5)) {
                            angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                            w->vel[0] = sinf(angle) * 80.0f;
                            w->vel[2] = cosf(angle) * 80.0f;
                            w->count = 30;
                            w->action = 0x31;
                            w->step = 0;
                            w->vel[1] = 85.0f;
                        } else {
                            angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                            w->vel[0] = sinf(angle) * 50.0f;
                            w->vel[2] = cosf(angle) * 50.0f;
                            w->count = 30;
                            w->action = 0x2F;
                            w->step = 0;
                            w->vel[1] = 35.0f;
                        }
                    }
                    if (gBossLife <= 0) {
                        DPRINTF("ボス死亡しました。\n");
                        gBattleFlags2 |= 0x400;
                        w->dmg->flags &= ~2;
                        AWAY(w, 15.0f);
                        w->count = 30;
                        w->action = 0x2B;
                        w->step = 0;
                        w->vel[1] = 120.0f;
                    }
                } else if (w->dmg->hitKind == 11) {
                    DPRINTF("ビスマルはんが砲に当たりました。\n");
                    if (gBossFlags & 0x4000) {
                        beMegaRefCreate(0.0f, 0.0f, 0.0f);
                        DPRINTF("効かぬわ。\n");
                    } else {
                        beHitmarkCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f);
                        beBpieceCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f, 10);
                        beBpieceCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f, 10);
                        if (gBattleFlags2 & 0x40000) {
                            bossDamage(800, 11, 0);
                        } else {
                            bossDamage(400, 11, 0);
                        }
                        w->dmg->flags &= ~2;
                        gBossFlags &= ~0x200;
                        gBrotherFlags &= ~0x20000;
                        w->count = 30;
                        w->action = 0x30;
                        w->step = 0;
                    }
                    sndPlay(0x8179);
                    gBossFlags &= ~0x400;
                    if (gBossLife <= 0) {
                        gBattleFlags2 |= 0x400;
                        w->dmg->flags &= ~2;
                    }
                } else if (w->dmg->hitKind == 0xCA) {
                    DPRINTF("ビスマルは外部インパクトに当たりました。\n");
                    if (gBossFlags & 0x4000) {
                        DPRINTF("効かぬわ。\n");
                    } else {
                        beHitmarkCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 3.0f);
                        beBpieceCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f, 10);
                        beBpieceCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f, 10);
                        SoundLocation2(0x27D, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                       w->dmg->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                        w->dmg->flags &= ~2;
                        w->atk->flags &= ~2;
                        gBossFlags &= ~0x200;
                        gBossFlags &= ~0x400;
                        gBrotherFlags &= ~0x20000;
                        angle = mathAtan2(w->dmg->hitPos[0], w->dmg->hitPos[2]);
                        w->vel[0] = sinf(angle) * -50.0f;
                        w->vel[2] = cosf(angle) * -50.0f;
                        w->count = 30;
                        w->action = 0x2B;
                        w->step = 0;
                        w->vel[1] = 110.0f;
                    }
                    if (gBossLife <= 0) {
                        DPRINTF("ボス死亡しました。\n");
                        gBattleFlags2 |= 0x400;
                        w->dmg->flags &= ~2;
                        AWAY(w, 15.0f);
                        w->count = 30;
                        w->action = 0x2B;
                        w->step = 0;
                        w->vel[1] = 120.0f;
                    }
                } else if (w->dmg->hitKind == 5) {
                    DPRINTF("ビスマルは百烈パンチに当たりました。\n");
                    if (gBossFlags & 0x4000) {
                        DPRINTF("効かぬわ。\n");
                    } else {
                        w->frame->pos[1] = 0.0f;
                        gBattleFlags2 |= 0x2000;
                        bossDamage(10, 3, 1);
                        beHitmarkCreate(w->dmg->hitPos[0] + fadeRandRange(-200, 200),
                                        w->dmg->hitPos[1] + fadeRandRange(-200, 200),
                                        w->dmg->hitPos[2] + fadeRandRange(-200, 200), 3.0f);
                        beHitmarkCreate(w->dmg->hitPos[0] + fadeRandRange(-200, 200),
                                        w->dmg->hitPos[1] + fadeRandRange(-200, 200),
                                        w->dmg->hitPos[2] + fadeRandRange(-200, 200), 3.0f);
                        beHitringCreate(w->dmg->hitPos[0] + fadeRandRange(-200, 200),
                                        w->dmg->hitPos[1] + fadeRandRange(-200, 200),
                                        w->dmg->hitPos[2] + fadeRandRange(-200, 200), 1.0f, 0);
                        if (gBattleSndToggles & 2) {
                            sndPlay(0x50E);
                            gBattleSndToggles &= ~2;
                        } else {
                            sndPlay(0x50F);
                            gBattleSndToggles |= 2;
                        }
                        gBossFlags &= ~0x400;
                        gBrotherFlags &= ~0x20000;
                        w->action = 0x2E;
                        w->step = 0;
                    }
                }
            }
            gBossFlags &= ~4;
            bossStartActionIfFlag(obj, 0x12);
            if (gBattleFlags & 8) {
                gBattleFlags &= ~8;
                if (w->action == 9) {
                    bossStartAction(obj, 0, 0x13);
                }
                if (w->action == 0x17) {
                    bossStartAction(obj, 0, 0x13);
                }
                if (w->action == 0x18) {
                    bossStartAction(obj, 0, 0x13);
                }
                if (w->action == 0x19) {
                    bossStartAction(obj, 0, 0x13);
                }
            }

            switch (w->action) {
                case 0:
                    /* Waiting: drift about the middle of the arena and pick the next attack.
                     * The attack order has three lists, by the life left; with the
                     * barrier up (gBossFlags 0x400) some turns change. */
                    BOOST(w);
                    gBossPhase = 0;
                    /* one-line ifs, the game's style (IDO schedules by line, IDO_NOTES 12, 324) */
                    if (gBossLife < gBossLifeMax * 0.66666) gBossPhase = 1;
                    if (gBossLife < gBossLifeMax * 0.33333) gBossPhase = 2;
                    gBossActionArg--;
                    if (gBossActionArg <= 0) {
                        if (gBossPhase == 0) {
                            if (gBossActionCount == 0) {
                                bossStartAction(obj, 60, 1);
                            } else if (gBossActionCount == 1) {
                                bossStartAction(obj, 60, 4);
                            } else if (gBossActionCount == 2) {
                                bossStartAction(obj, 60, 2);
                            } else if (gBossActionCount == 3) {
                                bossStartAction(obj, 60, 5);
                            } else if (gBossActionCount == 4) {
                                bossStartAction(obj, 60, 1);
                            } else if (gBossActionCount == 5) {
                                bossStartAction(obj, 60, 10);
                            } else if (gBossActionCount == 6) {
                                bossStartAction(obj, 60, 3);
                            } else if (gBossActionCount == 7) {
                                bossStartAction(obj, 60, 0x15);
                            } else if (gBossActionCount == 8) {
                                bossStartAction(obj, 60, 3);
                            } else {
                                gBossActionCount = 0;
                            }
                        } else if (gBossPhase == 1) {
                            if (gBossActionCount == 0) {
                                if (gBossFlags & 0x400) {
                                    bossStartAction(obj, 40, 0x17);
                                } else {
                                    bossStartAction(obj, 40, 6);
                                }
                            } else if (gBossActionCount == 1) {
                                if (gBossFlags & 0x400) {
                                    bossStartAction(obj, 40, 0x18);
                                } else {
                                    bossStartAction(obj, 40, 6);
                                }
                            } else if (gBossActionCount == 2) {
                                if (gBossFlags & 0x400) {
                                    bossStartAction(obj, 40, 0x18);
                                } else {
                                    bossStartAction(obj, 30, 6);
                                }
                            } else if (gBossActionCount == 3) {
                                if (gBossFlags & 0x400) {
                                    bossStartAction(obj, 40, 9);
                                } else {
                                    bossStartAction(obj, 30, 6);
                                }
                            } else if (gBossActionCount == 4) {
                                if (gBossFlags & 0x400) {
                                    bossStartAction(obj, 40, 0x17);
                                } else {
                                    bossStartAction(obj, 30, 6);
                                }
                            } else if (gBossActionCount == 5) {
                                if (gBossFlags & 0x400) {
                                    bossStartAction(obj, 40, 0x19);
                                } else {
                                    bossStartAction(obj, 30, 6);
                                }
                            } else if (gBossActionCount == 6) {
                                if (gBossFlags & 0x400) {
                                    bossStartAction(obj, 40, 0x17);
                                } else {
                                    bossStartAction(obj, 30, 6);
                                }
                            } else if (gBossActionCount == 7) {
                                if (gBossFlags & 0x400) {
                                    bossStartAction(obj, 40, 0x14);
                                } else {
                                    bossStartAction(obj, 30, 6);
                                }
                            } else {
                                gBossActionCount = 0;
                            }
                        } else if (gBossPhase == 2) {
                            if (gBossActionCount == 0) {
                                bossStartAction(obj, 30, 3);
                            } else if (gBossActionCount == 1) {
                                bossStartAction(obj, 30, 0x16);
                            } else if (gBossActionCount == 2) {
                                bossStartAction(obj, 30, 0x11);
                            } else if (gBossActionCount == 3) {
                                bossStartAction(obj, 30, 0x15);
                            } else if (gBossActionCount == 4) {
                                bossStartAction(obj, 30, 3);
                            } else if (gBossActionCount == 5) {
                                bossStartAction(obj, 30, 11);
                            } else if (gBossActionCount == 6) {
                                bossStartAction(obj, 30, 6);
                            } else if (gBossActionCount == 7) {
                                if (gBossFlags & 0x400) {
                                    bossStartAction(obj, 30, 0x18);
                                } else {
                                    bossStartAction(obj, 30, 0x11);
                                }
                            } else if (gBossActionCount == 8) {
                                if (gBossFlags & 0x400) {
                                    bossStartAction(obj, 20, 0x17);
                                } else {
                                    bossStartAction(obj, 30, 0x15);
                                }
                            } else if (gBossActionCount == 9) {
                                if (gBossFlags & 0x400) {
                                    bossStartAction(obj, 20, 0x17);
                                } else {
                                    bossStartAction(obj, 30, 11);
                                }
                            } else if (gBossActionCount == 10) {
                                if (gBossFlags & 0x400) {
                                    bossStartAction(obj, 30, 0x18);
                                } else {
                                    bossStartAction(obj, 30, 3);
                                }
                            } else if (gBossActionCount == 11) {
                                if (gBossFlags & 0x400) {
                                    bossStartAction(obj, 30, 9);
                                } else {
                                    bossStartAction(obj, 30, 0x16);
                                }
                            } else if (gBossActionCount == 12) {
                                if (gBossFlags & 0x400) {
                                    bossStartAction(obj, 30, 0x14);
                                } else {
                                    bossStartAction(obj, 30, 0x11);
                                }
                            } else if (gBossActionCount == 13) {
                                bossStartAction(obj, 30, 0x11);
                            } else if (gBossActionCount == 14) {
                                bossStartAction(obj, 30, 0x15);
                            } else if (gBossActionCount == 15) {
                                bossStartAction(obj, 30, 11);
                            } else if (gBossActionCount == 16) {
                                bossStartAction(obj, 30, 3);
                            } else {
                                gBossActionCount = 0;
                            }
                        }
                        /* The Impact is down: the boss's victory. */
                        if (gBossFlags & 0x2000) {
                            bossStartAction(obj, 60, 16);
                        }
                    }
                    if (gImpactArmor <= 0 || gImpactArmor2 <= 0) {
                        DPRINTF("インパクト死亡しました\n");
                        gBossFlags |= 0x2000;
                    }
                    if (!(gBossFlags & 1)) {
                        bossPatrolX(obj, -350.0f, 350.0f, 0.3f);
                        bossPatrolY(obj, 50.0f, -50.0f, 0.05f);
                        bossPatrolZ(obj, -3500.0f, -2500.0f, 0.3f);
                    }
                    bossStepAnim(w->frame, 0.1f, 0);
                    POS_OUT(w);
                    TILT(w);
                    MOVE(w);
                    bossTurnToward2(w->frame, 0.0f, 0.0f, 1.0f);
                    ((VramCheckFunc) vramCheck)(w->frame, ray, w->vel[1], 600.0f);
                    break;
                case 1:
                    /* The heart beam ("Hbeam") from the middle of the arena. */
                    POS_OUT(w);
                    TILT(w);
                    MOVE(w);
                    BOOST(w);
                    switch (w->step) {
                        case 0:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (w->frame->pos[2] > -2000.0f) {
                                w->step = 1;
                            } else if (w->frame->pos[2] < -5000.0f) {
                                w->step = 1;
                            } else {
                                w->step = 2;
                            }
                            break;
                        case 1:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossFlyToward(obj, w->frame->pos[0], 0.0f, -2000.0f, 1.5f)) {
                                w->step++;
                            }
                            break;
                        case 2:
                            bossStepAnim(w->frame, 0.1f, 0);
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 2.0f);
                            if (bossDampVel(obj, 0.85f) == 0) {
                                BODY_ANIM(w, bbisuIdleAnim3, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 3:
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 2.0f);
                            if (w->timer == 30) {
                                tskCreate(obj, bisuHbeamProc, gBattleTask->file, NULL);
                            }
                            if (bossStepAnim(w->frame, 0.2f, 2)) {
                                BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                bossClearAction(obj);
                            }
                            w->timer++;
                            break;
                    }
                    break;
                case 2:
                    /* Two heart beams from the right, two from the middle and two from the left. */
                    POS_OUT(w);
                    TILT(w);
                    MOVE(w);
                    FACE(w);
                    BOOST(w);
                    switch (w->step) {
                        case 0:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossDampVel(obj, 0.85f) == 0) {
                                w->step++;
                            }
                            break;
                        case 1:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossFlyToward(obj, 1000.0f, -200.0f, -2000.0f, 1.5f)) {
                                w->step++;
                            }
                            break;
                        case 2:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossDampVel(obj, 0.85f) == 0) {
                                BODY_ANIM(w, bbisuIdleAnim3, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 3:
                            if (w->timer == 20) {
                                tskCreate(obj, bisuHbeamProc, gBattleTask->file, NULL);
                            }
                            if (w->timer == 40) {
                                tskCreate(obj, bisuHbeamProc, gBattleTask->file, NULL);
                            }
                            if (bossStepAnim(w->frame, 0.2f, 2)) {
                                BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                w->step++;
                            }
                            w->timer++;
                            break;
                        case 4:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossFlyToward(obj, 0.0f, -200.0f, -2000.0f, 1.5f)) {
                                w->step++;
                            }
                            break;
                        case 5:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossDampVel(obj, 0.8f) == 0) {
                                BODY_ANIM(w, bbisuIdleAnim3, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 6:
                            if (w->timer == 20) {
                                tskCreate(obj, bisuHbeamProc, gBattleTask->file, NULL);
                            }
                            if (w->timer == 40) {
                                tskCreate(obj, bisuHbeamProc, gBattleTask->file, NULL);
                            }
                            if (bossStepAnim(w->frame, 0.2f, 2)) {
                                BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                w->step++;
                            }
                            w->timer++;
                            break;
                        case 7:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossFlyToward(obj, -1000.0f, -200.0f, -2000.0f, 1.5f)) {
                                w->step++;
                            }
                            break;
                        case 8:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossDampVel(obj, 0.8f) == 0) {
                                BODY_ANIM(w, bbisuIdleAnim3, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 9:
                            if (w->timer == 20) {
                                tskCreate(obj, bisuHbeamProc, gBattleTask->file, NULL);
                            }
                            if (w->timer == 40) {
                                tskCreate(obj, bisuHbeamProc, gBattleTask->file, NULL);
                            }
                            if (bossStepAnim(w->frame, 0.2f, 2)) {
                                BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                bossClearAction(obj);
                            }
                            w->timer++;
                            break;
                    }
                    break;
                case 3:
                    /* Three heart beams from the middle. */
                    POS_OUT(w);
                    TILT(w);
                    MOVE(w);
                    BOOST(w);
                    switch (w->step) {
                        case 0:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (w->frame->pos[2] > -2000.0f) {
                                w->step = 1;
                            } else if (w->frame->pos[2] < -5000.0f) {
                                w->step = 1;
                            } else {
                                w->step = 2;
                            }
                            break;
                        case 1:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossFlyToward(obj, w->frame->pos[0], 0.0f, -2000.0f, 1.5f)) {
                                w->step++;
                            }
                            break;
                        case 2:
                            bossStepAnim(w->frame, 0.1f, 0);
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 2.0f);
                            if (bossDampVel(obj, 0.85f) == 0) {
                                BODY_ANIM(w, bbisuIdleAnim3, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 3:
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 2.0f);
                            if (w->timer == 10) {
                                tskCreate(obj, bisuHbeamProc, gBattleTask->file, NULL);
                            }
                            if (w->timer == 30) {
                                tskCreate(obj, bisuHbeamProc, gBattleTask->file, NULL);
                            }
                            if (w->timer == 50) {
                                tskCreate(obj, bisuHbeamProc, gBattleTask->file, NULL);
                            }
                            if (bossStepAnim(w->frame, 0.2f, 2)) {
                                BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                bossClearAction(obj);
                            }
                            w->timer++;
                            break;
                    }
                    break;
                case 4:
                    /* The punch ("Hpunch"): from the right, the left or the middle, toward the
                     * cockpit. */
                    POS_OUT(w);
                    TILT(w);
                    MOVE(w);
                    FACE(w);
                    BOOST(w);
                    switch (w->step) {
                        case 0:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                w->count = fadeRand(5);
                                w->step++;
                            }
                            break;
                        case 1:
                            bossStepAnim(w->frame, 0.1f, 0);
                            gBossFlags |= 4;
                            if (w->count == 0) {
                                if (bossFlyToward(obj, 1000.0f, -200.0f, -2000.0f, 1.5f)) {
                                    w->step++;
                                }
                            } else if (w->count == 1) {
                                if (bossFlyToward(obj, -1000.0f, -200.0f, -2000.0f, 1.5f)) {
                                    w->step++;
                                }
                            } else if (bossFlyToward(obj, 0.0f, -200.0f, -2000.0f, 1.5f)) {
                                w->step++;
                            }
                            w->timer = 0;
                            break;
                        case 2:
                            bossStepAnim(w->frame, 0.1f, 0);
                            gBossFlags |= 4;
                            bossDampVel(obj, 0.9f);
                            if (w->timer++ > 40) {
                                w->step++;
                            }
                            break;
                        case 3:
                            bossStepAnim(w->frame, 0.1f, 0);
                            gBossFlags |= 4;
                            if (w->count == 0) {
                                if (bossFlyToward(obj, 800.0f, -200.0f, -800.0f, 1.5f)) {
                                    BODY_ANIM(w, bbisuPunchWindUpAnim0, 0.0f, 0);
                                    w->step++;
                                }
                            } else if (w->count == 1) {
                                if (bossFlyToward(obj, -800.0f, -200.0f, -800.0f, 1.5f)) {
                                    BODY_ANIM(w, bbisuPunchWindUpAnim0, 0.0f, 0);
                                    w->step++;
                                }
                            } else if (bossFlyToward(obj, 0.0f, -200.0f, -800.0f, 1.5f)) {
                                BODY_ANIM(w, bbisuPunchWindUpAnim0, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 4:
                            bossStepAnim(w->frame, 0.1f, 0);
                            gBossFlags |= 4;
                            if (bossDampVel(obj, 0.6f) == 0) {
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 5:
                            gBossFlags |= 4;
                            if (w->timer == 40) {
                                if (w->count == 0) {
                                    HPUNCH(300.0f, -200.0f, 1);
                                } else if (w->count == 1) {
                                    HPUNCH(-300.0f, -200.0f, 2);
                                } else {
                                    HPUNCH(-100.0f, -300.0f, 0);
                                }
                            }
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bbisuPunchWindUpAnim1, 0.0f, 0);
                                w->step++;
                            }
                            w->timer++;
                            break;
                        case 6:
                            gBossFlags |= 4;
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                bossClearAction(obj);
                            }
                            break;
                    }
                    break;
                case 5:
                    /* The one-two: a punch, then a second from a little to the side. */
                    POS_OUT(w);
                    TILT(w);
                    MOVE(w);
                    FACE(w);
                    BOOST(w);
                    switch (w->step) {
                        case 0:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                w->count = fadeRand(5);
                                w->step++;
                            }
                            break;
                        case 1:
                            bossStepAnim(w->frame, 0.1f, 0);
                            gBossFlags |= 4;
                            if (w->count == 0) {
                                if (bossFlyToward(obj, 1000.0f, -200.0f, -2500.0f, 1.5f)) {
                                    w->step++;
                                }
                            } else if (w->count == 1) {
                                if (bossFlyToward(obj, -1000.0f, -200.0f, -2500.0f, 1.5f)) {
                                    w->step++;
                                }
                            } else if (bossFlyToward(obj, 0.0f, -200.0f, -2500.0f, 1.5f)) {
                                w->step++;
                            }
                            w->timer = 0;
                            break;
                        case 2:
                            bossStepAnim(w->frame, 0.1f, 0);
                            gBossFlags |= 4;
                            bossDampVel(obj, 0.9f);
                            if (w->timer++ > 40) {
                                w->step++;
                            }
                            break;
                        case 3:
                            bossStepAnim(w->frame, 0.1f, 0);
                            gBossFlags |= 4;
                            if (w->count == 0) {
                                if (bossFlyToward(obj, 800.0f, -200.0f, -800.0f, 1.5f)) {
                                    BODY_ANIM(w, bbisuPunchWindUpAnim0, 0.0f, 0);
                                    w->step++;
                                }
                            } else if (w->count == 1) {
                                if (bossFlyToward(obj, -800.0f, -200.0f, -800.0f, 1.5f)) {
                                    BODY_ANIM(w, bbisuPunchWindUpAnim0, 0.0f, 0);
                                    w->step++;
                                }
                            } else if (bossFlyToward(obj, 0.0f, -200.0f, -800.0f, 1.5f)) {
                                BODY_ANIM(w, bbisuPunchWindUpAnim0, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 4:
                            bossStepAnim(w->frame, 0.1f, 0);
                            gBossFlags |= 4;
                            if (bossDampVel(obj, 0.6f) == 0) {
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 5:
                            gBossFlags |= 4;
                            if (w->timer == 40) {
                                if (w->count == 0) {
                                    HPUNCH(300.0f, -200.0f, 1);
                                } else if (w->count == 1) {
                                    HPUNCH(-300.0f, -200.0f, 2);
                                } else {
                                    HPUNCH(-100.0f, -200.0f, 0);
                                }
                            }
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bbisuPunchWindUpAnim2, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            w->timer++;
                            break;
                        case 6:
                            gBossFlags |= 4;
                            if (w->timer == 25) {
                                if (w->count == 0) {
                                    HPUNCH(350.0f, -200.0f, 4);
                                } else if (w->count == 1) {
                                    HPUNCH(-250.0f, -200.0f, 5);
                                } else {
                                    HPUNCH(-50.0f, -300.0f, 3);
                                }
                            }
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                bossClearAction(obj);
                            }
                            w->timer++;
                            break;
                    }
                    break;
                case 6:
                    /* The plate ("bVani", "plate"): over on his back, the plate drops on him
                     * and he comes up again behind the barrier (gBossFlags 0x400). */
                    POS_OUT(w);
                    MOVE(w);
                    BOOST(w);
                    if (gBattleFlags & 8) {
                        gBattleFlags &= ~8;
                        bossStartAction(obj, 0, 0x13);
                    }
                    if (gBossFlags & 0x2000) {
                        w->step = 7;
                    }
                    if (FRAMES % 10 == 0) {
                        bisuBboostCreate(w->frame->pos[0], -1000.0f, w->frame->pos[2], 100.0f);
                    }
                    switch (w->step) {
                        case 0:
                            FACE(w);
                            TILT(w);
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                w->step++;
                            }
                            break;
                        case 1:
                            FACE(w);
                            TILT(w);
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossFlyToward(obj, 0.0f, 150.0f, -2000.0f, 1.5f)) {
                                w->step++;
                            }
                            break;
                        case 2:
                            FACE(w);
                            TILT(w);
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossDampVel(obj, 0.85f) == 0) {
                                BODY_ANIM(w, bbisuThrownDownAnim1, 0.0f, 0);
                                tskCreate(obj, bisuBVaniProc, gBattleTask->file, NULL);
                                w->vel[0] = 0.0f;
                                w->vel[1] = 0.0f;
                                w->vel[2] = 0.0f;
                                w->frame->rot[0] = 0.0f;
                                w->frame->rot[1] = 0.0f;
                                w->frame->rot[2] = 0.0f;
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 3:
                            gBossFlags |= 4;
                            if (w->timer == 15) {
                                sndPlay(0x177);
                            }
                            if (FRAMES % 5 == 0) {
                                itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
                            }
                            if (bossStepAnim(w->frame, 0.4f, 2)) {
                                tskCreate(obj, bisuPlateProc, gBattleTask->file, NULL);
                                DMG_BOX(w, 150, 300, -150);
                                w->step++;
                            }
                            break;
                        case 4:
                            gBossFlags |= 4;
                            w->frame->rot[2] += 0.13962634403104063;
                            if (FRAMES % 5 == 0) {
                                itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
                            }
                            if (w->frame->rot[2] > 3.1415927f) {
                                w->frame->rot[2] = 3.1415927f;
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 5:
                            gBossFlags |= 4;
                            if (FRAMES % 5 == 0) {
                                itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
                            }
                            if (w->timer > 90) {
                                DMG_BOX(w, 150, 300, -150);
                                BODY_ANIM(w, bbisuThrownDownAnim2, 0.0f, 0);
                                effectKiranCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 500.0f);
                                beFlashFCreate(0xFF, 0xFF, 0xFF, 20);
                                w->frame->rot[2] = 0.0f;
                                sndPlay(0x8177);
                                bossClearAction(obj);
                                gBossFlags |= 0x400;
                            }
                            break;
                        case 7:
                            break;
                    }
                    w->timer++;
                    break;
                case 9:
                    /* A dash: to one side, up, in close, across, and back. */
                    POS_OUT(w);
                    TILT(w);
                    MOVE(w);
                    FACE(w);
                    BOOST(w);
                    switch (w->step) {
                        case 0:
                            if (bossDampVel(obj, 0.9f) == 0) {
                                DMG_BOX(w, 150, 300, -150);
                                w->count = fadeRand(10);
                                w->step = 1;
                            }
                            break;
                        case 1:
                            if (w->count >= 6) {
                                if (bossFlyToward(obj, 1000.0f, -200.0f, -1500.0f, 2.0f)) {
                                    w->step = 2;
                                }
                            } else if (bossFlyToward(obj, -1000.0f, -200.0f, -1500.0f, 2.0f)) {
                                w->step = 2;
                            }
                            break;
                        case 2:
                            if (bossDampVel(obj, 0.8f) == 0) {
                                w->step = 3;
                            }
                            break;
                        case 3:
                            if (w->count >= 6) {
                                if (bossFlyToward(obj, -1000.0f, 500.0f, -1500.0f, 2.0f)) {
                                    w->step = 4;
                                }
                            } else if (bossFlyToward(obj, 1000.0f, 500.0f, -1500.0f, 2.0f)) {
                                w->step = 4;
                            }
                            break;
                        case 4:
                            if (w->count >= 6) {
                                if (bossFlyToward(obj, -1000.0f, -200.0f, -820.0f, 2.0f)) {
                                    w->step = 5;
                                }
                            } else if (bossFlyToward(obj, 1000.0f, -200.0f, -820.0f, 2.0f)) {
                                w->step = 5;
                            }
                            break;
                        case 5:
                            if (w->count >= 6) {
                                if (bossFlyToward(obj, 1000.0f, 500.0f, -820.0f, 2.0f)) {
                                    w->step = 6;
                                }
                            } else if (bossFlyToward(obj, -1000.0f, 500.0f, -820.0f, 2.0f)) {
                                w->step = 6;
                            }
                            break;
                        case 6:
                            if (w->count >= 6) {
                                if (bossFlyToward(obj, 1000.0f, -200.0f, -1500.0f, 2.0f)) {
                                    w->step = 7;
                                }
                            } else if (bossFlyToward(obj, -1000.0f, -200.0f, -1500.0f, 2.0f)) {
                                w->step = 7;
                            }
                            break;
                        case 7:
                            if (bossDampVel(obj, 0.8f) == 0) {
                                bossClearAction(obj);
                            }
                            break;
                    }
                    break;
                case 10:
                    /* In close, then the second punch ("Hpunch", bisuHpunch2Create) from the right,
                     * the left or the middle. */
                    POS_OUT(w);
                    TILT(w);
                    MOVE(w);
                    FACE(w);
                    BOOST(w);
                    switch (w->step) {
                        case 0:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                w->count = fadeRand(100) % 5;
                                w->step++;
                            }
                            break;
                        case 1:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (w->count == 0) {
                                if (bossFlyToward(obj, 1000.0f, -200.0f, -2500.0f, 1.5f)) {
                                    BODY_ANIM(w, bbisuIdleAnim3, 0.0f, 0);
                                    w->step++;
                                }
                            } else if (w->count == 1) {
                                if (bossFlyToward(obj, -1000.0f, -200.0f, -2500.0f, 1.5f)) {
                                    BODY_ANIM(w, bbisuIdleAnim3, 0.0f, 0);
                                    w->step++;
                                }
                            } else if (bossFlyToward(obj, 0.0f, -200.0f, -2500.0f, 1.5f)) {
                                BODY_ANIM(w, bbisuIdleAnim3, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 2:
                            gBossFlags |= 4;
                            if (bossDampVel(obj, 0.6f) == 0) {
                                w->step++;
                            }
                            break;
                        case 3:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.2f, 2);
                            if (w->count == 0) {
                                if (bossFlyToward(obj, 800.0f, -150.0f, -800.0f, 2.5f)) {
                                    w->step++;
                                }
                            } else if (w->count == 1) {
                                if (bossFlyToward(obj, -800.0f, -150.0f, -800.0f, 2.5f)) {
                                    w->step++;
                                }
                            } else if (bossFlyToward(obj, 0.0f, -150.0f, -800.0f, 2.5f)) {
                                w->step++;
                            }
                            break;
                        case 4:
                            gBossFlags |= 4;
                            if (w->frame->pos[2] > -800.0f) {
                                w->vel[2] = -w->vel[2];
                                w->step++;
                            }
                            break;
                        case 5:
                            gBossFlags |= 4;
                            if (w->count == 0) {
                                bisuHpunch2Create(200.0f, 30.0f, -200.0f, 1);
                                w->step++;
                            } else if (w->count == 1) {
                                bisuHpunch2Create(-200.0f, 30.0f, -200.0f, 2);
                                w->step++;
                            } else {
                                bisuHpunch2Create(0.0f, 30.0f, -400.0f, 0);
                                w->step++;
                            }
                            break;
                        case 6:
                            gBossFlags |= 4;
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                w->step++;
                            }
                            break;
                        case 7:
                            gBossFlags |= 4;
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                bossClearAction(obj);
                            }
                            break;
                    }
                    break;
                case 11:
                    /* The rose ("Rose") at the back of the arena, then needles ("Needle") for
                     * six seconds. */
                    POS_OUT(w);
                    TILT(w);
                    MOVE(w);
                    FACE(w);
                    BOOST(w);
                    switch (w->step) {
                        case 0:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                w->step++;
                            }
                            break;
                        case 1:
                            gBossFlags |= 4;
                            gBrotherFlags |= 0x20000;
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossFlyToward(obj, 0.0f, 200.0f, -4000.0f, 1.5f)) {
                                w->step++;
                            }
                            break;
                        case 2:
                            gBossFlags |= 4;
                            gBrotherFlags |= 0x20000;
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                BODY_ANIM(w, bbisuIdleAnim1, 0.0f, 0);
                                effectKiranCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 500.0f);
                                beFlashFCreate(0xFF, 0xFF, 0xFF, 20);
                                bisuRoseCreate(0.0f, 200.0f, -7000.0f, 3.0f);
                                gBossFlags |= 0x800;
                                w->step++;
                            }
                            break;
                        case 3:
                            gBossFlags |= 4;
                            gBrotherFlags |= 0x20000;
                            if (bossStepAnim(w->frame, 0.2f, 2)) {
                                BODY_ANIM(w, bbisuIdleAnim2, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 4:
                            gBossFlags |= 4;
                            gBrotherFlags |= 0x20000;
                            if (bossStepAnim(w->frame, 0.1f, 2)) {
                                BODY_ANIM(w, bbisuIdleAnim3, 0.0f, 0);
                                w->timer = 360;
                                w->step++;
                            }
                            break;
                        case 5:
                            gBossFlags |= 4;
                            gBrotherFlags |= 0x20000;
                            bossStepAnim(w->frame, 1.0f, 0);
                            if (w->timer % 24 == 0) {
                                bisuNeedleCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 3.0f);
                            }
                            if (w->timer == 0) {
                                BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                w->timer = 180;
                                w->step++;
                            }
                            w->timer--;
                            break;
                        case 6:
                            bossStepAnim(w->frame, 0.05f, 0);
                            gBrotherFlags |= 0x20000;
                            if (w->timer == 0) {
                                BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                gBossFlags &= ~0x800;
                                gBrotherFlags &= ~0x20000;
                                bossClearAction(obj);
                            }
                            w->timer--;
                            break;
                    }
                    break;
                case 12:
                    /* The love barrier ("Lbarri"), once. */
                    POS_OUT(w);
                    DPRINTF("Sstep/Stimer [%d/%d]\n", w->step, w->timer);
                    if (gBossFlags & 0x200) {
                        DPRINTF("既にバリアー展開しているのでやめます。\n");
                        w->action = 0;
                        w->step = 0;
                    } else {
                        BOOST(w);
                        switch (w->step) {
                            case 0:
                                TILT(w);
                                MOVE(w);
                                gBossFlags |= 4;
                                if (bossDampVel(obj, 0.8f) == 0) {
                                    w->step = 1;
                                    BODY_ANIM(w, bbisuIdleAnim3, 0.0f, 0);
                                }
                                w->timer = 0;
                                break;
                            case 1:
                                w->timer++;
                                gBossFlags |= 4;
                                if (w->timer == 30) {
                                    DPRINTF("バリアー展開します。\n");
                                    tskCreate(gEffectTask, bisuLbarriProc, gBattleTask->file, NULL);
                                    gBossFlags |= 0x200;
                                }
                                if (bossStepAnim(w->frame, 0.2f, 2)) {
                                    BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                    bossClearAction(obj);
                                }
                                break;
                        }
                    }
                    break;
                case 13:
                    /* Dizzy after three flinches (bossFlinchCountOver): a charge, then back. */
                    POS_OUT(w);
                    TILT(w);
                    MOVE(w);
                    FACE(w);
                    gBossFlags |= 0x1000;
                    BOOST(w);
                    DPRINTF("Sstep/Stimer [%d/%d]\n", w->step, w->timer);
                    switch (w->step) {
                        case 0:
                            gBossFlags |= 4;
                            BODY_ANIM(w, bbisuIdleAnim1, 0.0f, 1);
                            w->timer = 0;
                            w->step = 1;
                            break;
                        case 1:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.3f, 2);
                            if (w->timer == 35) {
                                SoundLocation2(0x281, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                            }
                            if (bossFlyToward(obj, 0.0f, -200.0f, -2000.0f, 2.0f)) {
                                BODY_ANIM(w, bbisuIdleAnim2, 0.0f, 0);
                                w->step = 2;
                            }
                            break;
                        case 2:
                            gBossFlags |= 4;
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                w->step = 3;
                            }
                            break;
                        case 3:
                            if (bossDampVel(obj, 0.8f) == 0) {
                                BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                bossClearAction(obj);
                                gBossFlags &= ~0x1000;
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 14:
                    /* The entrance demo ("登場デモ"), with its camera ("Bappcam"): Bismaru
                     * bursts out of the floor, spins and lands. Start skips it (steps 19-21),
                     * and so does a lost battle before. */
                    MOVE(w);
                    gBattleFlags2 |= 0x200000;
                    if (w->step != 9 && w->step != 10 && w->step != 11 && w->step != 14 && w->step != 15 &&
                        w->step != 16 && w->step != 18 && w->step != 19 && w->step != 20 && w->step != 21) {
                        if (gGameWork.losses != 0) {
                            for (i = 0; i < 4; i++) {
                                contPakInitRumble(i);
                                contPakRumbleRequestStop(i);
                            }
                            w->step = 19;
                        }
                        if (fadeCountPlayers() == 1) {
                            if (joyGetHeldTime(gCockpitPort, gCockpitBtnStart) == 1) {
                                DPRINTF("[bisu]登場デモをキャンセルします\n");
                                for (i = 0; i < 4; i++) {
                                    contPakInitRumble(i);
                                    contPakRumbleRequestStop(i);
                                }
                                w->step = 19;
                            }
                        } else if (fadeCountPlayers() >= 2 && (joyGetHeldTime(gCockpitPort, gCockpitBtnStart) == 1 ||
                                                               joyGetHeldTime(gBrotherPort, gBrotherBtnStart) == 1)) {
                            DPRINTF("[bisu]登場デモをキャンセルします\n");
                            for (i = 0; i < 4; i++) {
                                contPakInitRumble(i);
                                contPakRumbleRequestStop(i);
                            }
                            w->step = 19;
                        }
                    }
                    switch (w->step) {
                        case 0:
                            gBattleFlags2 |= 0x400;
                            tskCreate(obj, bisuBappcamProc, 0x143, NULL);
                            BODY_ANIM(w, bbisuThrownDownAnim2, 0.0f, 0);
                            w->frame->pos[0] = 0.0f;
                            w->frame->pos[1] = -1300.0f;
                            w->frame->pos[2] = -2000.0f;
                            gBattleFlags |= 0x11000000;
                            gBrotherFlags |= 0x10000;
                            w->timer = 0;
                            w->step = 1;
                            break;
                        case 1:
                            if (w->timer == 240) {
                                w->step = 2;
                            }
                            break;
                        case 2:
                            /* The floor shakes and breaks. */
                            if (!(w->timer & 7)) {
                                bossRumble(2, 0x88888888, 8);
                            }
                            if (w->timer % 12 == 0) {
                                beSmoke6Create(0.0f, -1000.0f, -2000.0f, 1.0f, 200.0f, 0);
                                beBpieceCreate(0.0f, -1000.0f, -2000.0f, 5.0f, 300);
                            }
                            if (w->timer == 300) {
                                w->step = 3;
                            }
                            break;
                        case 3:
                            if (!(w->timer & 7)) {
                                bossRumble(2, 0x88888888, 8);
                            }
                            if (w->timer == 440) {
                                sndPlay(0x7B);
                            }
                            if (w->timer % 12 == 0) {
                                beSmoke6Create(0.0f, -1000.0f, -2000.0f, 3.0f, 100.0f, 0);
                                beBpieceCreate(0.0f, -1000.0f, -2000.0f, 5.0f, 300);
                            }
                            if (w->timer % 12 == 6) {
                                beSmoke6Create(0.0f, -1000.0f, -2000.0f, 1.0f, 400.0f, 0);
                                beBpieceCreate(0.0f, -1000.0f, -2000.0f, 10.0f, 400);
                            }
                            if (w->timer == 480) {
                                w->step = 4;
                            }
                            break;
                        case 4:
                            if (!(w->timer & 7)) {
                                bossRumble(2, 0xA5294A52, 8);
                            }
                            if (w->timer % 12 == 0) {
                                beSmoke6Create(0.0f, -1000.0f, -2000.0f, 5.0f, 100.0f, 0);
                                beBpieceCreate(0.0f, -1000.0f, -2000.0f, 5.0f, 200);
                                beBpieceCreate(0.0f, -1000.0f, -2000.0f, 5.0f, 200);
                            }
                            if (w->timer % 12 == 4) {
                                beSmoke6Create(0.0f, -1000.0f, -2000.0f, 3.0f, 200.0f, 0);
                                beBpieceCreate(0.0f, -600.0f, -2000.0f, 10.0f, 300);
                            }
                            if (w->timer % 12 == 8) {
                                beSmoke6Create(0.0f, -1000.0f, -2000.0f, 1.0f, 400.0f, 0);
                                beBpieceCreate(0.0f, -600.0f, -2000.0f, 10.0f, 300);
                            }
                            if (w->timer == 600) {
                                bossRumble(2, 0xFFFF8888, 30);
                                w->step = 5;
                                w->vel[1] = 70.0f;
                            }
                            break;
                        case 5:
                            /* Up through the floor and out of sight. */
                            if (w->timer % 5 == 0) {
                                itexPuff(0.0f, w->frame->pos[1], -2000.0f, 1.0f);
                                beSmoke6Create(0.0f, w->frame->pos[1], -2000.0f, 1.0f, 200.0f, 2);
                                beBpieceCreate(0.0f, w->frame->pos[1], -2000.0f, 5.0f, 300);
                                beBpieceCreate(0.0f, w->frame->pos[1], -2000.0f, 10.0f, 100);
                                beBpieceCreate(0.0f, w->frame->pos[1], -2000.0f, 10.0f, 100);
                                beSmoke6Create(0.0f, -1000.0f, -2000.0f, 2.0f, 30.0f, 3);
                                beSmoke6Create(0.0f, -500.0f, -2000.0f, 2.0f, 30.0f, 3);
                            }
                            if (w->frame->pos[1] > 5000.0f) {
                                w->vel[1] = 0.0f;
                                w->step = 6;
                            }
                            break;
                        case 6:
                            /* Down again, spinning. */
                            w->vel[1] -= 1.0;
                            w->frame->rot[1] += 0.03490658600776016;
                            if (w->timer % 10 == 5) {
                                itexPuff(0.0f, w->frame->pos[1], -2000.0f, 5.0f);
                                beSmoke6Create(0.0f, w->frame->pos[1], -2000.0f, 3.0f, 300.0f, 2);
                            }
                            if (w->timer % 10 == 0) {
                                beSmoke6Create(0.0f, w->frame->pos[1], -2000.0f, 3.0f, 300.0f, 2);
                            }
                            if (w->frame->pos[1] < 2000.0f) {
                                w->step = 7;
                            }
                            break;
                        case 7:
                            w->frame->rot[1] += DEG;
                            if (w->timer % 10 == 0) {
                                beSmoke6Create(0.0f, w->frame->pos[1], -2000.0f, 1.0f, 300.0f, 2);
                            }
                            if (bossDampVel(obj, 0.95f) == 0) {
                                w->vel[1] = 0.0f;
                                w->step = 8;
                            }
                            break;
                        case 8:
                            w->frame->rot[1] += 0.03490658600776016;
                            bossBobVel(obj, 4.0f, 2.0f);
                            if (w->timer % 10 == 0) {
                                beSmoke6Create(0.0f, w->frame->pos[1], -2000.0f, 1.0f, 300.0f, 2);
                            }
                            if (w->frame->rot[1] >= 6.283185481396829) {
                                w->frame->rot[1] = 6.2831855f;
                                w->vel[1] = 0.0f;
                                w->step = 9;
                            }
                            break;
                        case 9:
                            w->frame->rot[2] += 0.17453293003880077;
                            if (w->timer % 10 == 0) {
                                beSmoke6Create(0.0f, w->frame->pos[1], -2000.0f, 1.0f, 300.0f, 2);
                            }
                            if (w->frame->rot[2] >= 3.1415927406984143) {
                                BODY_ANIM(w, bbisuThrownDownAnim1, 1.0f, 0);
                                /* A double literal: 3.1415927f loads into f0, this into the
                                 * game's float temp (f8), and the rest of the demo follows. */
                                w->frame->rot[2] = 3.1415927406984143;
                                tskCreate(gEffectTask, bisuBVDemoProc, gBattleTask->file, NULL);
                                tskCreate(gEffectTask, bisuBplateDProc, gBattleTask->file, NULL);
                                w->step = 10;
                            }
                            break;
                        case 10:
                            w->frame->rot[2] = 0.0f;
                            w->frame->rot[1] += 0.26179939505820116;
                            if (w->timer % 10 == 0) {
                                beSmoke6Create(0.0f, w->frame->pos[1], -2000.0f, 1.0f, 300.0f, 2);
                            }
                            if (bossStepAnim(w->frame, 0.5f, 3)) {
                                BODY_ANIM(w, bbisuIdleAnim3, 0.0f, 0);
                                w->step = 11;
                            }
                            break;
                        case 11:
                            w->frame->rot[1] += 0.26179939505820116;
                            bossStepAnim(w->frame, 0.05f, 2);
                            if (w->timer % 10 == 0) {
                                beSmoke6Create(0.0f, w->frame->pos[1], -2000.0f, 1.0f, 300.0f, 2);
                            }
                            if (w->timer % 10 == 5) {
                                beSmoke6Create(0.0f, w->frame->pos[1], -2000.0f, 3.0f, 300.0f, 2);
                            }
                            if (w->timer == 1100) {
                                BODY_ANIM(w, bbisuIdleAnim1, 0.0f, 0);
                                w->step = 12;
                            }
                            break;
                        case 12:
                            /* sic: "& 360" (not "% 360") */
                            w->frame->rot[1] += 0.26179939505820116;
                            if (((s32) w->frame->rot[1] & 360) <= 0) {
                                w->step = 13;
                            }
                            break;
                        case 13:
                            w->frame->rot[1] += 0.17453293003880077;
                            if (w->frame->rot[1] > 2.7925268806208123) {
                                w->step++;
                            }
                            break;
                        case 14:
                            w->frame->rot[1] -= 0.06981317201552031;
                            if (w->timer == 1155) {
                                tskCreate(obj, bisuBroseDProc, gBattleTask->file, NULL);
                            }
                            if (bossStepAnim(w->frame, 0.4f, 2)) {
                                w->step++;
                            }
                            break;
                        case 15:
                            if (w->timer == 1200) {
                                BODY_ANIM(w, bbisuIdleAnim2, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 16:
                            if (bossStepAnim(w->frame, 0.2f, 2)) {
                                w->step++;
                            }
                            break;
                        case 17:
                            BOOST(w);
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 1.0f);
                            if (w->timer == 1300) {
                                scRunStart(2, 0, 0);
                                sndPlay(0x316);
                            }
                            bossBobVel(obj, 1.0f, 2.0f);
                            if (w->timer == 1356 || w->timer == 1358) {
                                effectKiranCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 500.0f);
                            }
                            if (w->timer == 1360) {
                                sndPlay(0x128);
                                effectKiranCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 500.0f);
                                w->step++;
                            }
                            break;
                        case 18:
                            BOOST(w);
                            bossBobVel(obj, 1.0f, 2.0f);
                            if (w->timer == 1450) {
                                w->frame->pos[1] = 0.0f;
                            }
                            if (w->timer == 1520) {
                                DPRINTF("登場デモ終了\n");
                                gBattleFlags2 &= ~0x400;
                                gBattleFlags2 |= 8;
                                gBattleFlags2 &= ~0x200000;
                                BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                effectKiranCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 500.0f);
                                bossClearAction(obj);
                            }
                            break;
                        case 19:
                            /* Skipped: close the letterbox over the demo. */
                            fadeCreate(gEffectTask);
                            fadeSetParent(gFramePool10);
                            fadeSetLetterbox(320, 120, 0, 120);
                            sndPlay(10);
                            sndPlay(0x7C);
                            w->step++;
                            break;
                        case 20:
                            BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                            w->frame->pos[0] = 0.0f;
                            w->frame->pos[1] = 0.0f;
                            w->frame->pos[2] = -2000.0f;
                            w->frame->rot[0] = w->frame->rot[1] = w->frame->rot[2] = 0.0f;
                            w->vel[0] = 0.0f;
                            w->vel[1] = 0.0f;
                            w->vel[2] = 0.0f;
                            gBattleFlags &= ~0x10000000;
                            gBattleFlags &= ~0x1000000;
                            gBattleFlags2 &= ~0x100;
                            gBrotherFlags &= ~0x10000;
                            gBattleFlags2 |= 0x8000;
                            w->step++;
                            break;
                        case 21:
                            bossStepAnim(w->frame, 0.05f, 0);
                            bossBobVel(obj, 1.0f, 2.0f);
                            if (fadeMoveLetterbox(0, 240, 1) == 1) {
                                DPRINTF("登場デモキャンセル終了\n");
                                gBattleFlags2 &= ~0x400;
                                gBattleFlags2 |= 8;
                                gBattleFlags2 &= ~0x200000;
                                effectKiranCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 500.0f);
                                bossClearAction(obj);
                            }
                            break;
                        case 22:
                            break;
                    }
                    DPRINTF("\rDemotimer[%d]", w->timer);
                    w->timer++;
                    break;
                case 15:
                    /* Defeated: the losing demo, the plate's farewell, then the battle is won. */
                    MOVE(w);
                    POS_OUT(w);
                    FACE(w);
                    gBattleFlags2 |= 0x200000;
                    switch (w->step) {
                        case 0:
                            fadeCreate(gEffectTask);
                            gBattleFlags2 |= 0x400;
                            gBossFlags |= 0x800;
                            gBattleFlags2 |= 0x1000;
                            gBattleFlags2 |= 0x1000000;
                            sndPlay(9);
                            w->vel[0] = w->vel[1] = w->vel[2] = 0.0f;
                            tskCreate(obj, Bappcam3, 0x143, NULL);
                            w->frame->pos[0] = 2000.0f;
                            gBossPosX = w->frame->pos[0];
                            w->frame->pos[1] = 0.0f;
                            gBossPosY = w->frame->pos[1];
                            w->frame->pos[2] = -1000.0f;
                            gBossPosZ = w->frame->pos[2];
                            BODY_ANIM(w, bbisuDefeatedAnim, 0.0f, 1);
                            effectKiranCreate(w->frame->pos[0], w->frame->pos[1] + 200.0f, w->frame->pos[2]);
                            beFlashFCreate(0xFF, 0xFF, 0xFF, 40);
                            effectPompomCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 250.0f, 250.0f,
                                               250.0f, 7, 550, 5, 0.8f, 0.1f);
                            effectPompomCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 250.0f, 250.0f,
                                               250.0f, 7, 550, 5, 0.8f, 0.1f);
                            effectPompomCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 250.0f, 250.0f,
                                               250.0f, 8, 550, 5, 0.8f, 0.1f);
                            effectPompomCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 250.0f, 250.0f,
                                               250.0f, 8, 550, 5, 0.8f, 0.1f);
                            effectPompomCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 250.0f, 250.0f,
                                               250.0f, 10, 550, 5, 0.8f, 0.1f);
                            sndPlay(0x10E);
                            w->timer = 0;
                            w->step++;
                            break;
                        case 1:
                            if (w->timer % 12 == 0) {
                                sndPlay(0x513);
                            }
                            if (!(w->timer & 7)) {
                                bossRumble(2, 0x88888888, 8);
                            }
                            if (w->timer < 30) {
                                w->vel[1] = 4.0f;
                            } else {
                                w->vel[1] = 0.0f;
                            }
                            if (FRAMES % 10 >= 5) {
                                w->frame->pos[1] += 2.0f;
                            } else {
                                w->frame->pos[1] -= 2.0f;
                            }
                            bossStepAnim(w->frame, 0.2f, 0);
                            if (w->timer == 240) {
                                effectPompomCreate(w->frame->pos[0] + 200.0f, w->frame->pos[1] + 200.0f,
                                                   w->frame->pos[2], 300.0f, 50.0f, 300.0f, 10, 400, 6, 1.5f, 0.1f);
                                effectPompomCreate(w->frame->pos[0] + 200.0f, w->frame->pos[1] + 200.0f,
                                                   w->frame->pos[2], 300.0f, 50.0f, 300.0f, 10, 400, 6, 1.0f, 0.1f);
                                w->step++;
                            }
                            break;
                        case 2:
                            if (!(w->timer & 7)) {
                                bossRumble(2, 0x88888888, 8);
                            }
                            if (w->timer % 12 == 0) {
                                sndPlay(0x513);
                            }
                            bossStepAnim(w->frame, 0.2f, 0);
                            if (FRAMES % 10 >= 5) {
                                w->frame->pos[1] += 2.0f;
                            } else {
                                w->frame->pos[1] -= 2.0f;
                            }
                            w->frame->rot[2] += 0.03490658600776016;
                            if (w->frame->rot[2] > 0.31415927f) {
                                w->frame->rot[2] = 0.31415927f;
                                w->step++;
                            }
                            break;
                        case 3:
                            if (!(w->timer & 7)) {
                                bossRumble(2, 0x88888888, 8);
                            }
                            if (w->timer % 12 == 0) {
                                sndPlay(0x513);
                            }
                            w->frame->rot[2] -= DEG;
                            if (w->frame->rot[2] < 0.22689280908185694) {
                                w->frame->rot[2] = 0.22689280908185694;
                            }
                            if (FRAMES % 10 >= 5) {
                                w->frame->pos[1] += 3.0f;
                            } else {
                                w->frame->pos[1] -= 3.0f;
                            }
                            bossStepAnim(w->frame, 0.2f, 0);
                            if (w->timer == 480) {
                                effectPompomCreate(w->frame->pos[0] - 200.0f, w->frame->pos[1] + 100.0f,
                                                   w->frame->pos[2], 200.0f, 100.0f, 200.0f, 10, 300, 6, 1.0f, 0.1f);
                                effectPompomCreate(w->frame->pos[0] - 200.0f, w->frame->pos[1] + 100.0f,
                                                   w->frame->pos[2], 200.0f, 100.0f, 200.0f, 10, 300, 6, 1.0f, 0.1f);
                                sndPlay(0x513);
                                w->step++;
                            }
                            break;
                        case 4:
                            if (w->timer % 8 == 0) {
                                bossRumble(2, 0x88888888, 8);
                            }
                            if (w->timer % 8 == 0) {
                                sndPlay(0x513);
                            }
                            bossStepAnim(w->frame, 0.2f, 0);
                            if (FRAMES % 10 >= 5) {
                                w->frame->pos[1] += 3.0f;
                            } else {
                                w->frame->pos[1] -= 3.0f;
                            }
                            w->frame->rot[2] -= 0.03490658600776016;
                            if (w->frame->rot[2] < -0.31415927f) {
                                w->frame->rot[2] = -0.31415927f;
                                sndPlay(0x10F);
                                w->step++;
                            }
                            break;
                        case 5:
                            if (w->timer % 8 == 0) {
                                bossRumble(2, 0x88888888, 8);
                            }
                            if (w->timer % 8 == 0) {
                                sndPlay(0x513);
                            }
                            bossStepAnim(w->frame, 0.2f, 0);
                            if (FRAMES % 10 >= 5) {
                                w->frame->pos[1] += 4.0f;
                            } else {
                                w->frame->pos[1] -= 4.0f;
                            }
                            if (w->timer % 30 == 0) {
                                beRedStarCreate(w->frame->pos[0], w->frame->pos[1] - 500.0f, w->frame->pos[2]);
                            }
                            w->frame->rot[2] += DEG;
                            if (w->frame->rot[2] > -0.22689280908185694) {
                                w->frame->rot[2] = -0.22689280908185694;
                            }
                            if (w->timer == 600) {
                                effectPompomCreate(w->frame->pos[0], w->frame->pos[1] - 150.0f, w->frame->pos[2], 250.0f,
                                                   250.0f, 250.0f, 7, 500, 5, 0.8f, 0.1f);
                                effectPompomCreate(w->frame->pos[0], w->frame->pos[1] - 150.0f, w->frame->pos[2], 250.0f,
                                                   250.0f, 250.0f, 7, 500, 5, 0.8f, 0.1f);
                                effectPompomCreate(w->frame->pos[0], w->frame->pos[1] - 150.0f, w->frame->pos[2], 250.0f,
                                                   250.0f, 250.0f, 8, 500, 5, 0.8f, 0.1f);
                                effectPompomCreate(w->frame->pos[0], w->frame->pos[1] - 150.0f, w->frame->pos[2], 250.0f,
                                                   250.0f, 250.0f, 8, 500, 5, 0.8f, 0.1f);
                                effectPompomCreate(w->frame->pos[0], w->frame->pos[1] - 150.0f, w->frame->pos[2], 250.0f,
                                                   250.0f, 250.0f, 10, 500, 5, 0.8f, 0.1f);
                                w->step++;
                            }
                            break;
                        case 6:
                            if (w->timer % 8 == 0) {
                                bossRumble(2, 0xA5294A52, 8);
                            }
                            if (w->timer % 8 == 0) {
                                sndPlay(0x513);
                            }
                            bossStepAnim(w->frame, 0.2f, 0);
                            if (FRAMES % 10 >= 5) {
                                w->frame->pos[1] += 5.0f;
                            } else {
                                w->frame->pos[1] -= 5.0f;
                            }
                            if (w->timer % 30 == 0) {
                                beRedStarCreate(w->frame->pos[0], w->frame->pos[1] - 500.0f, w->frame->pos[2]);
                            }
                            if (w->timer == 720) {
                                w->step++;
                            }
                            break;
                        case 7:
                            /* Toppling and sinking, then the explosion. */
                            if (w->timer % 8 == 0) {
                                bossRumble(2, 0xA5294A52, 8);
                            }
                            if (w->timer % 8 == 0 && w->timer < 960) {
                                sndPlay(0x513);
                            }
                            bossStepAnim(w->frame, 0.2f, 0);
                            if (FRAMES % 10 >= 5) {
                                w->frame->pos[1] += 6.0f;
                            } else {
                                w->frame->pos[1] -= 6.0f;
                            }
                            if (w->timer == 840) {
                                sndPlay(0x315);
                            }
                            if (w->timer < 900 && w->timer % 20 == 0) {
                                beRedStarCreate(w->frame->pos[0], w->frame->pos[1] - 500.0f, w->frame->pos[2]);
                            }
                            w->frame->rot[0] += 0.006108652551358027;
                            if (w->frame->rot[0] > 0.7853982f) {
                                w->frame->rot[0] = 0.7853982f;
                            }
                            w->frame->pos[1] -= 1.75;
                            if (w->timer == 960) {
                                beHitballCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 3.0f);
                            }
                            if (w->timer == 960) {
                                bossRumble(2, 0xFFFFFFFF, 50);
                                beHitballCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 3.0f);
                                beHitballCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 3.0f);
                                beHitballCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 3.0f);
                                beMeffectCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2]);
                                beMeffectCreate(w->frame->pos[0], w->frame->pos[1] + 200.0f, w->frame->pos[2]);
                                beMeffectCreate(w->frame->pos[0], w->frame->pos[1] + 300.0f, w->frame->pos[2]);
                                fadeSetAll(30.0f, 0.0f, 255.0f, 0xFF, 0xFF, 0xFF);
                                sndPlay(0x11C);
                            }
                            if (w->timer == 1040) {
                                gBossFlags &= ~0x800;
                                gBattleFlags2 &= ~0x1000;
                                gBattleFlags2 |= 0x800;
                                w->step++;
                            }
                            break;
                        case 8:
                            /* The plate comes down from above. */
                            if ((w->plate = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol)) == NULL) {
                                return -1;
                            }
                            bossSetPolDL(w->plate, bbisuLosingDemoPlateDl, 0xF6);
                            bossSetFrameFlags((AnimeFrame *) w->plate, 0x20);
                            w->plate->scale[0] = w->plate->scale[1] = w->plate->scale[2] = 1.7f;
                            w->plate->pos[0] = 0.0f;
                            w->plate->pos[1] = 2500.0f;
                            w->plate->pos[2] = -500.0f;
                            w->step++;
                            break;
                        case 9:
                            if (w->timer % 15 == 0) {
                                beSmoke6Create(w->plate->pos[0], w->plate->pos[1], w->plate->pos[2], 0.1f, 50.0f, 2);
                            }
                            if (w->timer % 15 == 5) {
                                beSmoke6Create(w->plate->pos[0], w->plate->pos[1] - 150.0f, w->plate->pos[2], 0.2f,
                                               50.0f, 2);
                            }
                            if (w->timer % 15 == 10) {
                                beSmoke6Create(w->plate->pos[0], w->plate->pos[1] - 300.0f, w->plate->pos[2], 0.4f,
                                               50.0f, 2);
                            }
                            fadeOpenLetterbox();
                            if (w->timer == 1110) {
                                fadeSetAll(30.0f, 255.0f, 0.0f, 0, 0, 0);
                            }
                            if (w->timer == 1125) {
                                sndPlay(0x106);
                            }
                            if (w->timer == 1200) {
                                sndPlay(0x17);
                            }
                            if (w->timer == 1260) {
                                sndPlay(0x123);
                            }
                            w->plate->pos[1] -= 10.0f;
                            if (w->plate->pos[1] < -140.0f) {
                                w->plate->pos[1] = -140.0f;
                                beSmoke6Create(w->plate->pos[0], w->plate->pos[1], w->plate->pos[2], 0.1f, 50.0f, 2);
                                w->step++;
                            }
                            break;
                        case 10:
                            if (w->timer % 15 == 0) {
                                beSmoke6Create(w->plate->pos[0], w->plate->pos[1], w->plate->pos[2], 0.1f, 50.0f, 2);
                            }
                            bossBobFrame((AnimeFrame *) w->plate, 1.0f, 2.0f);
                            if (w->timer == 1400) {
                                DPRINTF("SD_GOGO3をコールします。\n");
                                scRunStart(13, 0, 0);
                                w->step++;
                            }
                            break;
                        case 11:
                            if (w->timer % 15 == 0) {
                                beSmoke6Create(w->plate->pos[0], w->plate->pos[1], w->plate->pos[2], 0.1f, 50.0f, 2);
                            }
                            bossBobFrame((AnimeFrame *) w->plate, 1.0f, 2.0f);
                            if (scIsRunning() == 0) {
                                sndPlay(0x124);
                                w->step++;
                            }
                            break;
                        case 12:
                            if (w->timer % 5 == 0) {
                                beSmoke6Create(w->plate->pos[0], w->plate->pos[1], w->plate->pos[2], 0.1f, 50.0f, 2);
                            }
                            w->plate->pos[1] += 20.0f;
                            if (w->plate->pos[1] > 2500.0f) {
                                w->plate->pos[1] = 2500.0f;
                                sndPlay(8);
                                w->step++;
                            }
                            break;
                        case 13:
                            if (fadeCloseLetterbox(50)) {
                                clearPackCreate(0.0f, 0.0f, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 14:
                            if (gBattleFlags2 & 0x800000) {
                                gBattleFlags2 &= ~0x800000;
                                w->step++;
                            }
                            break;
                        case 15:
                            if (fadeCloseLetterbox(120)) {
                                gBattleFlags2 &= ~0x1000000;
                                swWorkSet(0xBD);
                                bossBattleWin((u8) gBattleBossId);
                                dmenuQueueDemo(5);
                            }
                            break;
                    }
                    DPRINTF("\rDemo step/timer[%d/%d]", w->step, w->timer);
                    w->timer++;
                    break;
                case 16:
                    /* The Impact is down: Bismaru's victory demo, then the battle is lost. */
                    POS_OUT(w);
                    TILT(w);
                    MOVE(w);
                    FACE(w);
                    gBattleFlags2 |= 0x200000;
                    BOOST(w);
                    switch (w->step) {
                        case 0:
                            DPRINTF("ビスマル勝利デモ開始します\n");
                            gBattleFlags2 |= 0x400;
                            gBattleFlags2 |= 0x800;
                            gBattleFlags |= 0x10000000;
                            fadeCreate(gEffectTask);
                            BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                w->step++;
                            }
                            break;
                        case 1:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossFlyToward(obj, 0.0f, -100.0f, -1500.0f, 1.5f)) {
                                w->step++;
                            }
                            break;
                        case 2:
                            bossStepAnim(w->frame, 0.2f, 0);
                            bossBobVel(obj, 1.0f, 2.0f);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                BODY_ANIM(w, bbisuIdleAnim3, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 3:
                            bossBobVel(obj, 1.0f, 2.0f);
                            if (bossStepAnim(w->frame, 0.2f, 2)) {
                                sndPlay(0x316);
                                fadeSetAll(120.0f, 0.0f, 255.0f, 0xFF, 0xFF, 0xFF);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 4:
                            if (w->timer > 120) {
                                DPRINTF("負け。\n");
                                sndPlay(7);
                                sndPlay(0x11D);
                                gBattleFlags2 |= 0x4000000;
                                w->step++;
                            }
                            break;
                        case 5:
                            if (fadeCloseLetterbox(120)) {
                                bossBattleLose((u8) gBattleBossId);
                            }
                            break;
                        case 6:
                            break;
                    }
                    DPRINTF("timer[%d]\n", w->timer);
                    w->timer++;
                    break;
                case 17:
                    /* The barrier ("Lbarri") and the heart rings ("Hring", "Heffect"): he sinks
                     * toward the cockpit, then back. */
                    POS_OUT(w);
                    TILT(w);
                    MOVE(w);
                    FACE(w);
                    BOOST(w);
                    switch (w->step) {
                        case 0:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                w->glow = 0;
                                w->step++;
                            }
                            break;
                        case 1:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossFlyToward(obj, 0.0f, -200.0f, -3500.0f, 1.5f)) {
                                BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 2:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                gBossFlags |= 4;
                                gBossFlags |= 0x4000;
                                gBossFlags |= 0x800;
                                effectKiranCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 500.0f);
                                beFlashFCreate(0xFF, 0xFF, 0xFF, 20);
                                w->timer = 120;
                                if (gBossFlags & 0x200) {
                                    DPRINTF("既にバリアー展開しているのでやめます。\n");
                                } else {
                                    DPRINTF("バリアー展開します。\n");
                                    tskCreate(gEffectTask, bisuLbarriProc, gBattleTask->file, NULL);
                                    gBossFlags |= 0x200;
                                }
                                soundActpanCreate(obj, 0x176, gBattleCamView, 500.0f, 11000.0f, gBattleCamView->fovy);
                                sndPlay(0x2D1);
                                w->step++;
                            }
                            break;
                        case 3:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (FRAMES % 5 == 0) {
                                bisuHringCreate(w->frame->pos[0], -1000.0f, w->frame->pos[2]);
                            }
                            if (FRAMES % 20 == 0) {
                                bisuHeffectCreate(w->frame->pos[0], -1000.0f, w->frame->pos[2]);
                            }
                            if (w->timer-- == 0) {
                                BODY_ANIM(w, bbisuIdleAnim1, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 4:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.03f, 2);
                            if (FRAMES % 20 == 0) {
                                bisuHeffectCreate(w->frame->pos[0], -1000.0f, w->frame->pos[2]);
                            }
                            if (bossFlyToward(obj, 0.0f, -150.0f, -820.0f, 0.05f)) {
                                w->step++;
                            }
                            break;
                        case 5:
                            gBossFlags |= 4;
                            if (bossDampVel(obj, 0.6f) == 0) {
                                w->timer = 60;
                                w->step++;
                            }
                            break;
                        case 6:
                            gBossFlags |= 4;
                            if (w->timer % 10 == 0) {
                                beNullattackCreate(0.0f, 0.0f, 0.0f, 0x1D);
                            }
                            if (w->timer-- == 0) {
                                BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 7:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossFlyToward(obj, 0.0f, -200.0f, -3500.0f, 5.0f)) {
                                w->step++;
                            }
                            break;
                        case 8:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossDampVel(obj, 0.8f) == 0) {
                                w->timer = 60;
                                w->step++;
                            }
                            break;
                        case 9:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (w->timer-- == 0) {
                                bossClearAction(obj);
                                gBossFlags &= ~0x4000;
                                gBossFlags &= ~0x800;
                                gBossFlags &= ~0x200;
                                sndPlay(0x8176);
                            }
                            break;
                    }
                    break;
                case 18:
                    /* Reset to the middle (gBattleFlags 0x4000000 turns it the other way). */
                    POS_OUT(w);
                    TILT(w);
                    MOVE(w);
                    BOOST(w);
                    switch (w->step) {
                        case 0:
                            gBossFlags |= 4;
                            w->atk->flags |= 2;
                            w->dmg->flags |= 2;
                            sndPlay(0x82BE);
                            sndPlay(0x8176);
                            if (gBossFlags & 0x400) {
                                BODY_ANIM(w, bbisuThrownDownAnim2, 0.0f, 0);
                            } else {
                                BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                            }
                            w->frame->pos[0] = 0.0f;
                            w->frame->pos[1] = -200.0f;
                            w->frame->pos[2] = -3000.0f;
                            w->frame->rot[0] = 0.0f;
                            w->vel[0] = 0.0f;
                            w->vel[1] = 0.0f;
                            w->vel[2] = 0.0f;
                            if (gBattleFlags & 0x4000000) {
                                w->frame->rot[1] = -2.0943952f;
                            } else {
                                w->frame->rot[1] = 2.0943952f;
                            }
                            w->frame->rot[2] = 0.0f;
                            gBossFlags &= ~0x4000;
                            gBossFlags &= ~0x800;
                            gBossFlags &= ~0x200;
                            w->timer = 60;
                            w->step++;
                            break;
                        case 1:
                            bossStepAnim(w->frame, 0.1f, 0);
                            gBossFlags |= 4;
                            if (gBattleFlags & 0x4000000) {
                                w->frame->rot[1] = -2.0943952f;
                            } else {
                                w->frame->rot[1] = 2.0943952f;
                            }
                            if (w->timer-- == 0) {
                                w->step++;
                            }
                            break;
                        case 2:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.1f, 0);
                            bossBobVel(obj, 4.0f, 2.0f);
                            if (gBattleFlags & 0x4000000) {
                                w->frame->rot[1] += DEG;
                                if (w->frame->rot[1] > 0.0f) {
                                    w->frame->rot[1] = 0.0f;
                                    bossClearAction(obj);
                                }
                            } else {
                                w->frame->rot[1] -= DEG;
                                if (w->frame->rot[1] < 0.0f) {
                                    w->frame->rot[1] = 0.0f;
                                    bossClearAction(obj);
                                }
                            }
                            break;
                    }
                    break;
                case 19:
                    /* Knocked out of an attack (gBattleFlags 8): wait for it to clear. */
                    POS_OUT(w);
                    TILT(w);
                    MOVE(w);
                    BOOST(w);
                    switch (w->step) {
                        case 0:
                            w->dmg->flags &= ~2;
                            if (gBossFlags & 0x400) {
                                BODY_ANIM(w, bbisuThrownDownAnim2, 0.0f, 0);
                            } else {
                                BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                            }
                            bossDampVel(obj, 0.8f);
                            gBossFlags &= ~0x4000;
                            gBossFlags &= ~0x800;
                            gBossFlags &= ~0x200;
                            w->step++;
                            break;
                        case 1:
                            bossStepAnim(w->frame, 0.1f, 0);
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 2.0f);
                            bossDampVel(obj, 0.8f);
                            if (!(gBattleFlags & 8)) {
                                w->dmg->flags |= 2;
                                bossClearAction(obj);
                            }
                            break;
                    }
                    break;
                case 20:
                    /* Up out of sight, and down again at the back. */
                    POS_OUT(w);
                    TILT(w);
                    MOVE(w);
                    FACE(w);
                    BOOST(w);
                    DPRINTF("Sstep/Stimer [%d/%d]\n", w->step, w->timer);
                    switch (w->step) {
                        case 0:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossDampVel(obj, 0.8f) == 0) {
                                w->vel[0] = 0.0f;
                                w->vel[1] = 0.0f;
                                w->vel[2] = 0.0f;
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 1:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (w->timer > 30) {
                                SoundLocation2(0x2BE, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                                w->step++;
                                w->vel[1] = 50.0f;
                            }
                            break;
                        case 2:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (w->timer % 5 == 0) {
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1] - 300.0f, w->frame->pos[2], 1.0f,
                                               200.0f, 2);
                            }
                            if (w->frame->pos[1] > 3000.0f) {
                                BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                w->frame->pos[0] = 0.0f;
                                w->frame->pos[1] = 3000.0f;
                                w->frame->pos[2] = -3000.0f;
                                w->vel[1] = 0.0f;
                                w->timer = 0;
                                gBossFlags &= ~0x400;
                                w->step++;
                            }
                            break;
                        case 3:
                            if (w->timer > 60) {
                                w->step++;
                            }
                            break;
                        case 4:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (w->timer % 5 == 0) {
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1] - 600.0f, w->frame->pos[2], 1.0f,
                                               200.0f, 2);
                            }
                            if (bossFlyToward(obj, 0.0f, 100.0f, -3000.0f, 1.5f)) {
                                sndPlay(0x51A);
                                w->step++;
                            }
                            break;
                        case 5:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossDampVel(obj, 0.8f) == 0) {
                                bossClearAction(obj);
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 21:
                    /* Up out of sight, down in front of the cockpit, and a one-two. */
                    POS_OUT(w);
                    TILT(w);
                    MOVE(w);
                    FACE(w);
                    BOOST(w);
                    DPRINTF("Sstep/Stimer [%d/%d]\n", w->step, w->timer);
                    switch (w->step) {
                        case 0:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossDampVel(obj, 0.8f) == 0) {
                                w->vel[0] = 0.0f;
                                w->vel[1] = 0.0f;
                                w->vel[2] = 0.0f;
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 1:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (w->timer > 30) {
                                SoundLocation2(0x2BE, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                                w->step++;
                                w->vel[1] = 50.0f;
                            }
                            break;
                        case 2:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (w->timer % 5 == 0) {
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1] - 300.0f, w->frame->pos[2], 1.0f,
                                               200.0f, 2);
                            }
                            if (w->frame->pos[1] > 3000.0f) {
                                BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                w->frame->pos[0] = 0.0f;
                                w->frame->pos[1] = 3000.0f;
                                w->frame->pos[2] = -800.0f;
                                w->vel[1] = 0.0f;
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 3:
                            if (w->timer > 60) {
                                w->step++;
                            }
                            break;
                        case 4:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (w->timer % 5 == 0) {
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1] - 600.0f, w->frame->pos[2], 1.0f,
                                               200.0f, 2);
                            }
                            if (bossFlyToward(obj, 0.0f, -200.0f, -800.0f, 2.0f)) {
                                sndPlay(0x51A);
                                w->step++;
                            }
                            break;
                        case 5:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossDampVel(obj, 0.6f) == 0) {
                                BODY_ANIM(w, bbisuPunchWindUpAnim0, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 6:
                            gBossFlags |= 4;
                            if (w->timer == 40) {
                                HPUNCH(-100.0f, -200.0f, 0);
                            }
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bbisuPunchWindUpAnim2, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 7:
                            gBossFlags |= 4;
                            if (w->timer == 25) {
                                HPUNCH(-50.0f, -300.0f, 3);
                            }
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                bossClearAction(obj);
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 22:
                    /* A somersault, then the second punch from the middle. */
                    POS_OUT(w);
                    MOVE(w);
                    FACE(w);
                    BOOST(w);
                    DPRINTF("Sstep/Stimer [%d/%d]\n", w->step, w->timer);
                    switch (w->step) {
                        case 0:
                            TILT(w);
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                w->step++;
                            }
                            break;
                        case 1:
                            TILT(w);
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossFlyToward(obj, 0.0f, -100.0f, -3500.0f, 1.5f)) {
                                BODY_ANIM(w, bbisuIdleAnim3, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 2:
                            TILT(w);
                            bossDampVel(obj, 0.9f);
                            if (w->timer > 30) {
                                w->step++;
                            }
                            break;
                        case 3:
                            TILT(w);
                            gBossFlags |= 4;
                            if (bossFlyToward(obj, 0.0f, -150.0f, -2000.0f, 1.5f)) {
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 4:
                            gBossFlags |= 4;
                            bossDampVel(obj, 0.9f);
                            w->frame->rot[0] -= 0.17453293003880077;
                            if (w->timer == 22) {
                                SoundLocation2(0x282, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                            }
                            if (w->timer < 25) {
                                w->frame->pos[1] += 50.0f;
                            } else {
                                w->frame->pos[1] -= 50.0f;
                            }
                            if (w->frame->rot[0] < -6.2831855f) {
                                w->frame->rot[0] = 0.0f;
                                w->step++;
                            }
                            break;
                        case 5:
                            gBossFlags |= 4;
                            TILT(w);
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossFlyToward(obj, 0.0f, -150.0f, -800.0f, 1.5f)) {
                                w->step++;
                            }
                            break;
                        case 6:
                            gBossFlags |= 4;
                            TILT(w);
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossDampVel(obj, 0.6f) == 0) {
                                bisuHpunch2Create(0.0f, 30.0f, -400.0f, 0);
                                w->step++;
                            }
                            break;
                        case 7:
                            gBossFlags |= 4;
                            TILT(w);
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 8:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossFlyToward(obj, 0.0f, -150.0f, -2000.0f, 1.5f)) {
                                bossClearAction(obj);
                            }
                            break;
                        case 10:
                            break;
                    }
                    w->timer++;
                    break;
                case 23:
                    /* Rolling charges: from one of five places at the back to the cockpit, and
                     * back again. */
                    POS_OUT(w);
                    MOVE(w);
                    BOOST(w);
                    DPRINTF("Sstep/Stimer [%d/%d]\n", w->step, w->timer);
                    switch (w->step) {
                        case 0:
                            FACE(w);
                            TILT(w);
                            BODY_ANIM(w, bbisuThrownDownAnim2, 0.0f, 0);
                            w->count = fadeRand(100) % 5;
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (w->frame->pos[2] > -2000.0f) {
                                w->step = 1;
                            } else if (w->frame->pos[2] < -5000.0f) {
                                w->step = 1;
                            } else {
                                w->step = 2;
                            }
                            break;
                        case 1:
                            FACE(w);
                            TILT(w);
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (w->count == 0) {
                                if (bossFlyToward(obj, 400.0f, 200.0f, -4000.0f, 1.5f)) {
                                    w->step++;
                                }
                            } else if (w->count == 1) {
                                if (bossFlyToward(obj, 200.0f, 200.0f, -4000.0f, 1.5f)) {
                                    w->step++;
                                }
                            } else if (w->count == 2) {
                                if (bossFlyToward(obj, 0.0f, 200.0f, -4000.0f, 1.5f)) {
                                    w->step++;
                                }
                            } else if (w->count == 3) {
                                if (bossFlyToward(obj, -200.0f, 200.0f, -4000.0f, 1.5f)) {
                                    w->step++;
                                }
                            } else if (w->count == 4) {
                                if (bossFlyToward(obj, -400.0f, 200.0f, -4000.0f, 1.5f)) {
                                    w->step++;
                                }
                            }
                            break;
                        case 2:
                            FACE(w);
                            TILT(w);
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.1f, 0);
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 2.0f);
                            if (bossDampVel(obj, 0.85f) == 0) {
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 3:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.1f, 0);
                            bossDampVel(obj, 0.85f);
                            w->frame->rot[0] += 0.17453293003880077;
                            if (w->frame->rot[0] > 6.2831855f) {
                                w->count = fadeRand(100) % 5;
                                w->step++;
                                w->frame->rot[0] = 0.0f;
                            }
                            break;
                        case 4:
                            FACE(w);
                            w->frame->rot[0] += 0.34906586007760154;
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (bossFlyToward(obj, 0.0f, -200.0f, -800.0f, 2.5f)) {
                                w->step++;
                            }
                            break;
                        case 5:
                            FACE(w);
                            w->frame->rot[0] += 0.34906586007760154;
                            gBossFlags |= 4;
                            if (w->frame->pos[2] > -800.0f) {
                                beNullattackCreate(0.0f, 0.0f, 0.0f, 0x1A);
                                w->count = fadeRand(100) % 5;
                                w->frame->rot[0] = 0.0f;
                                w->vel[2] = -w->vel[2];
                                w->step++;
                            }
                            break;
                        case 6:
                            FACE(w);
                            TILT(w);
                            gBossFlags |= 4;
                            if (w->count == 0) {
                                if (bossFlyToward(obj, 400.0f, 200.0f, -4000.0f, 1.5f)) {
                                    w->step++;
                                }
                            } else if (w->count == 1) {
                                if (bossFlyToward(obj, 200.0f, 200.0f, -4000.0f, 1.5f)) {
                                    w->step++;
                                }
                            } else if (w->count == 2) {
                                if (bossFlyToward(obj, 0.0f, 200.0f, -4000.0f, 1.5f)) {
                                    w->step++;
                                }
                            } else if (w->count == 3) {
                                if (bossFlyToward(obj, -200.0f, 200.0f, -4000.0f, 1.5f)) {
                                    w->step++;
                                }
                            } else if (w->count == 4) {
                                if (bossFlyToward(obj, -400.0f, 200.0f, -4000.0f, 1.5f)) {
                                    w->step++;
                                }
                            }
                            break;
                        case 7:
                            FACE(w);
                            TILT(w);
                            gBossFlags |= 4;
                            if (bossDampVel(obj, 0.9f) == 0) {
                                bossClearAction(obj);
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 24:
                    /* Spinning heart beams ("Hbeam" type 1) across the arena. */
                    POS_OUT(w);
                    TILT(w);
                    MOVE(w);
                    BOOST(w);
                    bossStepAnim(w->frame, 0.1f, 0);
                    DPRINTF("Sstep/Stimer [%d/%d]\n", w->step, w->timer);
                    switch (w->step) {
                        case 0:
                            BODY_ANIM(w, bbisuThrownDownAnim2, 0.0f, 0);
                            w->count = fadeRand(100) % 2;
                            w->step++;
                            break;
                        case 1:
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (w->count == 0) {
                                if (bossFlyToward(obj, 1500.0f, 600.0f, -3000.0f, 2.5f)) {
                                    w->step++;
                                }
                            } else if (w->count == 1) {
                                if (bossFlyToward(obj, -1500.0f, 600.0f, -3000.0f, 2.5f)) {
                                    w->step++;
                                }
                            }
                            break;
                        case 2:
                            bossStepAnim(w->frame, 0.1f, 0);
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 2.0f);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 3:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.1f, 0);
                            bossDampVel(obj, 0.85f);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 4:
                            gBossFlags |= 4;
                            w->frame->rot[1] += 0.08726646501940039;
                            if (w->timer > 20) {
                                w->step++;
                            }
                            break;
                        case 5:
                            gBossFlags |= 4;
                            w->frame->rot[1] += 0.34906586007760154;
                            if (w->timer > 40) {
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 6:
                            gBossFlags |= 4;
                            w->frame->rot[1] += 0.34906586007760154;
                            bossStepAnim(w->frame, 0.1f, 0);
                            if (w->timer % 20 == 0) {
                                tskCreate(obj, bisuHbeamProc, gBattleTask->file, (void *) 1);
                            }
                            if (w->count == 0) {
                                if (bossFlyToward(obj, -1500.0f, -100.0f, -3000.0f, 1.0f)) {
                                    w->timer = 0;
                                    w->step++;
                                }
                            } else if (w->count == 1) {
                                if (bossFlyToward(obj, 1500.0f, -100.0f, -3000.0f, 1.0f)) {
                                    w->timer = 0;
                                    w->step++;
                                }
                            }
                            break;
                        case 7:
                            gBossFlags |= 4;
                            w->frame->rot[1] += 0.08726646501940039;
                            if (w->timer > 30) {
                                w->step++;
                            }
                            break;
                        case 8:
                            gBossFlags |= 4;
                            if (bossTurnToward2(w->frame, 0.0f, 0.0f, 3.0f)) {
                                w->step++;
                            }
                            break;
                        case 9:
                            gBossFlags |= 4;
                            if (bossDampVel(obj, 0.9f) == 0) {
                                bossClearAction(obj);
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 25:
                    /* Drift about for six seconds. */
                    POS_OUT(w);
                    TILT(w);
                    MOVE(w);
                    FACE(w);
                    BOOST(w);
                    DPRINTF("Sstep/Stimer [%d/%d]\n", w->step, w->timer);
                    switch (w->step) {
                        case 0:
                            w->timer = 0;
                            w->step++;
                            break;
                        case 1:
                            bossPatrolX(obj, -350.0f, 350.0f, 0.4f);
                            bossPatrolY(obj, 50.0f, -50.0f, 0.2f);
                            bossPatrolZ(obj, -3500.0f, -2500.0f, 0.4f);
                            if (w->timer > 360) {
                                w->step++;
                            }
                            break;
                        case 2:
                            if (bossDampVel(obj, 0.9f) == 0) {
                                bossClearAction(obj);
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 0x2B:
                    /* Knocked flying (the velocity a hit set): down to the ground, two bounces,
                     * lie there, get up; the losing demo when the life is gone. */
                    POS_OUT(w);
                    MOVE(w);
                    TILT(w);
                    switch (w->step) {
                        case 0:
                            DMG_BOX(w, 200, 500, -300);
                            beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 1);
                            BODY_ANIM(w, bbisuThrownDownAnim0, 0.0f, 1);
                            sndPlay(0x82BE);
                            sndPlay(0x8177);
                            w->step++;
                            break;
                        case 1:
                            DPRINTF("damage flag[%d]\n", w->count);
                            w->count--;
                            w->vel[1] -= 4.0;
                            if (w->count == 0) {
                                w->dmg->flags |= 2;
                            }
                            bossStepAnim(w->frame, 0.3f, 2);
                            if (FRAMES % 10 == 0) {
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 1);
                                beBpieceCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 8.0f, 10);
                            }
                            ray.y = w->frame->pos[1] - 200.0f;
                            if (vramRayGround(&ray, (Vec3f *) w->vel, &at, &tp) != NULL) {
                                SoundLocation2(0x10C, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                                ON_GROUND(w);
                                w->frame->rot[0] = w->frame->rot[1] = w->frame->rot[2] = 0.0f;
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 0);
                                w->vel[1] = 40.0f;
                                w->dmg->flags |= 2;
                                w->step = 2;
                            }
                            break;
                        case 2:
                            w->vel[1] -= 4.0;
                            bossStepAnim(w->frame, 0.3f, 2);
                            ray.y = w->frame->pos[1] - 200.0f;
                            if (vramRayGround(&ray, (Vec3f *) w->vel, &at, &tp) != NULL) {
                                SoundLocation2(0x10C, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                                BODY_ANIM(w, bbisuThrownDownAnim0, 1.0f, 1);
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 0);
                                ON_GROUND(w);
                                w->step = 3;
                                w->timer = 90;
                                w->vel[1] = 30.0f;
                            }
                            break;
                        case 3:
                            w->vel[1] -= 4.0;
                            bossStepAnim(w->frame, 0.3f, 2);
                            ray.y = w->frame->pos[1] - 200.0f;
                            if (vramRayGround(&ray, (Vec3f *) w->vel, &at, &tp) != NULL) {
                                SoundLocation2(0x10C, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                                BODY_ANIM(w, bbisuThrownDownAnim0, 1.0f, 1);
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 0);
                                ON_GROUND(w);
                                w->vel[1] = 0.0f;
                                w->step = 4;
                                w->timer = 90;
                            }
                            break;
                        case 4:
                            w->vel[0] = 0.0f;
                            w->vel[1] = 0.0f;
                            w->vel[2] = 0.0f;
                            if (w->timer % 10 == 0) {
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 0);
                            }
                            if (w->timer-- == 0) {
                                if (gBossLife > 1) {
                                    beReversalCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                                }
                                w->step = 5;
                            }
                            break;
                        case 5:
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 2.0f);
                            w->vel[1] = 8.0f;
                            if (FRAMES % 10 == 0) {
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 0);
                            }
                            if (bossStepAnim(w->frame, 0.4f, 3)) {
                                if (gBossLife <= 0) {
                                    DPRINTF("ボス負けデモを開始します。\n");
                                    bossStartAction(obj, 0, 15);
                                } else {
                                    beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f,
                                                   0);
                                    BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                    bossClearAction(obj);
                                    gBossFlags &= ~1;
                                }
                            }
                            break;
                    }
                    break;
                case 0x2C:
                    /* A flinch from a jab (by the side hit), then back; three flinches dizzy him. */
                    POS_OUT(w);
                    switch (w->step) {
                        case 0:
                            TILT(w);
                            MOVE(w);
                            BODY_ANIM(w, bbisuKnockedBackAnim0, 0.0f, 1);
                            bossAddFlinchCount(1);
                            w->frame->pos[2] -= 100.0f;
                            w->hitWait = 20;
                            w->step++;
                            break;
                        case 1:
                            if (--w->hitWait == 0) {
                                w->dmg->flags |= 2;
                            }
                            if (bossStepAnim(w->frame, 0.8f, 2)) {
                                BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                if (bossFlinchCountOver(3)) {
                                    w->action = 13;
                                    w->step = 0;
                                } else {
                                    bossClearAction(obj);
                                }
                                w->dmg->flags |= 2;
                            }
                            break;
                    }
                    break;
                case 0x2D:
                    POS_OUT(w);
                    switch (w->step) {
                        case 0:
                            TILT(w);
                            MOVE(w);
                            BODY_ANIM(w, bbisuKnockedBackAnim1, 0.0f, 1);
                            bossAddFlinchCount(1);
                            w->step = 1;
                            w->frame->pos[2] -= 100.0f;
                            w->hitWait = 20;
                            break;
                        case 1:
                            if (--w->hitWait == 0) {
                                w->dmg->flags |= 2;
                            }
                            if (bossStepAnim(w->frame, 0.8f, 2)) {
                                BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                if (bossFlinchCountOver(3)) {
                                    w->action = 13;
                                    w->step = 0;
                                } else {
                                    bossClearAction(obj);
                                }
                                w->dmg->flags |= 2;
                            }
                            break;
                    }
                    break;
                case 0x2E:
                    /* The hundred-fist punch's flurry: held in front of the cockpit, then knocked
                     * away. */
                    POS_OUT(w);
                    FACE(w);
                    bossHeadingOffset(gCockpitHeadYaw, 0.0f, 1000.0f, &hx, &hz);
                    if (hx < w->frame->pos[0]) {
                        w->frame->pos[0] -= 10.0f;
                    } else {
                        w->frame->pos[0] += 10.0f;
                    }
                    if (w->frame->pos[2] > -900.0f) {
                        w->frame->pos[2] -= 20.0f;
                    } else {
                        w->frame->pos[2] += 20.0f;
                    }
                    if (w->frame->pos[1] < 0.0f) {
                        w->frame->pos[1] += 1.0f;
                    } else {
                        w->frame->pos[1] -= 1.0f;
                    }
                    switch (w->step) {
                        case 0:
                            w->vel[0] = w->vel[1] = w->vel[2] = 0.0f;
                            w->frame->rot[0] = 0.0f;
                            w->frame->rot[1] = 0.0f;
                            w->frame->rot[2] = 0.0f;
                            BODY_ANIM(w, bbisuKnockedBackAnim0, 0.0f, 1);
                            w->step = 1;
                            break;
                        case 1:
                            if (bossStepAnim(w->frame, 3.0f, 2)) {
                                BODY_ANIM(w, bbisuKnockedBackAnim1, 0.0f, 1);
                                w->step = 3;
                            }
                            break;
                        case 2:
                            if (bossStepAnim(w->frame, 3.0f, 2)) {
                                BODY_ANIM(w, bbisuKnockedBackAnim0, 0.0f, 1);
                                w->step = 3;
                            }
                            break;
                        case 3:
                            gBattleFlags2 &= ~0x2000;
                            if (gBossLife <= 0) {
                                DPRINTF("ボス死亡しました。\n");
                                AWAY(w, 15.0f);
                                w->count = 30;
                                w->action = 0x2B;
                                w->step = 0;
                                w->vel[1] = 120.0f;
                            } else {
                                angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                                w->vel[0] = sinf(angle) * 7.0f;
                                w->vel[2] = cosf(angle) * 7.0f;
                                w->action = 0x2B;
                                w->step = 0;
                                w->vel[1] = 70.0f;
                            }
                            break;
                    }
                    break;
                case 0x2F:
                    /* Knocked down hard (the kick): fall, bounce, lie, get up. */
                    POS_OUT(w);
                    switch (w->step) {
                        case 0:
                            TILT(w);
                            MOVE(w);
                            FACE(w);
                            DMG_BOX(w, 200, 500, -300);
                            BODY_ANIM(w, bbisuThrownDownAnim0, 0.0f, 1);
                            beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 1);
                            w->frame->rot[0] = w->frame->rot[1] = w->frame->rot[2] = 0.0f;
                            sndPlay(0x82BE);
                            sndPlay(0x8177);
                            w->step++;
                            break;
                        case 1:
                            MOVE(w);
                            FACE(w);
                            w->count--;
                            w->vel[1] -= 3.0;
                            if (FRAMES % 10 == 0) {
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 1);
                                beBpieceCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 8.0f, 10);
                            }
                            bossStepAnim(w->frame, 0.5f, 2);
                            ray.y = w->frame->pos[1] - 200.0f;
                            if (vramRayGround(&ray, (Vec3f *) w->vel, &at, &tp) != NULL) {
                                if (gBattleFlags2 & 0x10000) {
                                    sndPlay(0x10C);
                                } else {
                                    SoundLocation2(0x10C, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                                   w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                                }
                                ON_GROUND(w);
                                w->vel[1] = 20.0f;
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 800.0f, 0);
                                w->dmg->flags |= 2;
                                w->hitWait = 40;
                                w->step = 2;
                            }
                            break;
                        case 2:
                            MOVE(w);
                            w->dmg->flags |= 2;
                            w->vel[1] -= 1.0;
                            if (FRAMES % 10 == 0) {
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 1);
                                beBpieceCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 8.0f, 10);
                            }
                            if (w->hitWait-- == 0) {
                                w->step = 3;
                            }
                            ray.y = w->frame->pos[1] - 200.0f;
                            if (vramRayGround(&ray, (Vec3f *) w->vel, &at, &tp) != NULL) {
                                if (gBattleFlags2 & 0x10000) {
                                    sndPlay(0x10C);
                                } else {
                                    SoundLocation2(0x10C, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                                   w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                                }
                                ON_GROUND(w);
                                w->vel[1] = 30.0f;
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 800.0f, 0);
                            }
                            break;
                        case 3:
                            MOVE(w);
                            if (bossDampVel(obj, 0.7f) == 0) {
                                SoundLocation2(0x10C, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 0);
                                w->hitWait = 60;
                                w->step = 4;
                            }
                            break;
                        case 4:
                            MOVE(w);
                            if (FRAMES % 10 == 0) {
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 0);
                            }
                            if (w->hitWait-- == 0) {
                                if (gBossLife > 1) {
                                    beReversalCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                                }
                                w->step = 5;
                            }
                            break;
                        case 5:
                            MOVE(w);
                            TILT(w);
                            w->vel[1] = 8.0f;
                            if (FRAMES % 10 == 0) {
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 0);
                            }
                            if (bossStepAnim(w->frame, 0.3f, 3)) {
                                if (gBossLife <= 0) {
                                    DPRINTF("ボス負けデモを開始します。\n");
                                    bossStartAction(obj, 0, 15);
                                } else {
                                    beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f,
                                                   0);
                                    BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                    bossClearAction(obj);
                                    gBossFlags &= ~1;
                                }
                            }
                            break;
                    }
                    break;
                case 0x30:
                    /* Hit by the cannon: shaken in place, then knocked down. */
                    POS_OUT(w);
                    FACE(w);
                    bossHeadingOffset(gCockpitHeadYaw, 0.0f, 1000.0f, &hx, &hz);
                    gBossFlags &= ~0x4000;
                    gBossFlags &= ~0x800;
                    switch (w->step) {
                        case 2:
                            break;
                        case 0:
                            w->vel[0] = w->vel[1] = w->vel[2] = 0.0f;
                            w->frame->rot[0] = w->frame->rot[1] = w->frame->rot[2] = 0.0f;
                            gBattleFlags2 |= 0x2000000;
                            sndPlay(0x82BE);
                            sndPlay(0x8177);
                            if (w->frame->pos[1] < -600.0f) {
                                w->frame->pos[1] += 500.0f;
                            }
                            DMG_BOX(w, 200, 500, -300);
                            if (w->frame->pos[0] > 0.0f) {
                                BODY_ANIM(w, bbisuKnockedBackAnim0, 0.0f, 1);
                            } else {
                                BODY_ANIM(w, bbisuKnockedBackAnim1, 0.0f, 1);
                            }
                            w->timer = 240;
                            w->step = 1;
                            break;
                        case 1:
                            w->dmg->flags |= 2;
                            bossStepAnim(w->frame, 3.0f, 0);
                            if (w->timer % 10 >= 5) {
                                w->frame->pos[1] += 20.0f;
                            } else {
                                w->frame->pos[1] -= 20.0f;
                            }
                            if (w->timer-- == 0) {
                                DPRINTF("ボス負けデモを開始します。\n");
                                DPRINTF("dead:ボスはん,しになはれ[%d / %d]\n", gBossLife, gBossLifeMax);
                                beReversalCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 3.0f);
                                angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                                w->vel[0] = sinf(angle) * 50.0f;
                                w->vel[2] = cosf(angle) * 50.0f;
                                w->action = 0x2F;
                                w->step = 0;
                                w->vel[1] = 35.0f;
                                gBattleFlags2 &= ~0x2000000;
                                sndPlay(0x315);
                            }
                            break;
                    }
                    break;
                case 0x31:
                    /* The fifth kick: sent spinning, then down, bounce, lie, get up. */
                    POS_OUT(w);
                    TILT(w);
                    switch (w->step) {
                        case 0:
                            TILT(w);
                            MOVE(w);
                            DMG_BOX(w, 200, 500, -300);
                            w->dmg->flags &= ~2;
                            beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 1);
                            BODY_ANIM(w, bbisuThrownDownAnim0, 0.0f, 1);
                            sndPlay(0x82BE);
                            w->step++;
                            break;
                        case 1:
                            MOVE(w);
                            w->frame->rot[0] += 0.5235987901164023;
                            w->frame->rot[1] += 0.6981317201552031;
                            w->frame->rot[2] += 0.872664650194004;
                            w->count--;
                            w->vel[1] -= 3.0;
                            bossStepAnim(w->frame, 0.3f, 2);
                            if (FRAMES % 10 == 0) {
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 1);
                                beBpieceCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 8.0f, 10);
                            }
                            ray.y = w->frame->pos[1] - 200.0f;
                            if (vramRayGround(&ray, (Vec3f *) w->vel, &at, &tp) != NULL) {
                                w->frame->rot[0] = w->frame->rot[1] = w->frame->rot[2] = 0.0f;
                                SoundLocation2(0x10C, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                                ON_GROUND(w);
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 0);
                                w->vel[1] = 40.0f;
                                w->dmg->flags |= 2;
                                w->step++;
                            }
                            break;
                        case 2:
                            TILT(w);
                            MOVE(w);
                            w->vel[1] -= 4.0;
                            bossStepAnim(w->frame, 0.3f, 2);
                            ray.y = w->frame->pos[1] - 200.0f;
                            if (vramRayGround(&ray, (Vec3f *) w->vel, &at, &tp) != NULL) {
                                w->dmg->flags |= 2;
                                SoundLocation2(0x10C, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                                BODY_ANIM(w, bbisuThrownDownAnim0, 1.0f, 1);
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 0);
                                ON_GROUND(w);
                                w->step = 3;
                                w->timer = 90;
                                w->vel[1] = 30.0f;
                            }
                            break;
                        case 3:
                            TILT(w);
                            MOVE(w);
                            w->vel[1] -= 4.0;
                            bossStepAnim(w->frame, 0.3f, 2);
                            ray.y = w->frame->pos[1] - 200.0f;
                            if (vramRayGround(&ray, (Vec3f *) w->vel, &at, &tp) != NULL) {
                                SoundLocation2(0x10C, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                                BODY_ANIM(w, bbisuThrownDownAnim0, 1.0f, 1);
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 0);
                                ON_GROUND(w);
                                w->vel[1] = 0.0f;
                                w->step = 4;
                                w->timer = 90;
                            }
                            break;
                        case 4:
                            MOVE(w);
                            TILT(w);
                            w->vel[2] = 0.0f;
                            w->vel[1] = 0.0f;
                            w->vel[0] = 0.0f;
                            if (w->timer % 10 == 0) {
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 0);
                            }
                            if (w->timer-- == 0) {
                                if (gBossLife > 1) {
                                    beReversalCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                                }
                                w->step = 5;
                            }
                            break;
                        case 5:
                            MOVE(w);
                            TILT(w);
                            w->vel[1] = 8.0f;
                            if (FRAMES % 10 == 0) {
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 0);
                            }
                            if (bossStepAnim(w->frame, 0.4f, 3)) {
                                if (gBossLife <= 0) {
                                    bossStartAction(obj, 0, 15);
                                } else {
                                    beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f,
                                                   0);
                                    BODY_ANIM(w, bbisuIdleAnim0, 0.0f, 0);
                                    bossClearAction(obj);
                                    gBossFlags &= ~1;
                                }
                            }
                            break;
                    }
                    break;
                case 0x37:
                    break;
            }
            break;
        case 3:
            /* Before the draw: keep the boss in the arena, colour him and put his shadow
             * down. In the demos and when knocked flying he may leave the arena's heights. */
            gBossPosX = w->frame->pos[0];
            gBossPosY = w->frame->pos[1];
            gBossPosZ = w->frame->pos[2];
            if (w->action == 14 || w->action == 0x13 || w->action == 0x14 || w->action == 0x15 ||
                w->action == 0x2B) {
                if (w->frame->pos[0] > 6500.0f) {
                    DPRINTF("BOSSLIMIT : X+\n");
                    w->frame->pos[0] = 6500.0f;
                }
                if (w->frame->pos[0] < -6500.0f) {
                    DPRINTF("BOSSLIMIT : X-\n");
                    w->frame->pos[0] = -6500.0f;
                }
                if (w->frame->pos[2] > -800.0f) {
                    DPRINTF("BOSSLIMIT : Z+\n");
                    w->frame->pos[2] = -800.0f;
                }
                if (w->frame->pos[2] < -11000.0f) {
                    DPRINTF("BOSSLIMIT : Z-\n");
                    w->frame->pos[2] = -11000.0f;
                }
            } else {
                if (w->frame->pos[0] > 6500.0f) {
                    DPRINTF("BOSSLIMIT : X+\n");
                    w->frame->pos[0] = 6500.0f;
                }
                if (w->frame->pos[0] < -6500.0f) {
                    DPRINTF("BOSSLIMIT : X-\n");
                    w->frame->pos[0] = -6500.0f;
                }
                if (w->frame->pos[1] > 800.0f) {
                    DPRINTF("BOSSLIMIT : Y+\n");
                    w->frame->pos[1] = 800.0f;
                }
                if (w->frame->pos[1] < -1060.0f) {
                    DPRINTF("BOSSLIMIT : Y-\n");
                    w->frame->pos[1] = -1060.0f;
                }
                if (w->frame->pos[2] > -800.0f) {
                    DPRINTF("BOSSLIMIT : Z+\n");
                    w->frame->pos[2] = -800.0f;
                }
                if (w->frame->pos[2] < -11000.0f) {
                    DPRINTF("BOSSLIMIT : Z-\n");
                    w->frame->pos[2] = -11000.0f;
                }
            }
            if (gBossFlags & 0x4000) {
                /* The barrier's yellow glow. */
                w->glow += 3;
                if (w->glow > 192) {
                    w->glow = 192;
                }
                w->frame->dl = bossFogEnvColorDL(0xFF, 0xFF, 0, w->glow, 0x80, 0x80, 0x80, 0x80, 0x3DE);
            } else if (gBossFlags & 0x100) {
                /* A white flash. */
                w->frame->dl = bossFogEnvColorDL(0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x80, 0x80, 0x80, 0x3DE);
                gBossFlags &= ~0x100;
            } else {
                w->frame->dl = bossFogEnvColorDL(0, 0, 0, 0, 0x80, 0x80, 0x80, 0x80, 0x3DE);
            }
            if (w->action == 11) {
                w->frame->dl = bossLitTexEnvColorDL(0, 0, 0, 0);
            }
            if (w->action == 14) {
                w->frame->dl = bossLitTexEnvColorDL(0, 0, 0, 0);
            }
            if (w->action == 15) {
                w->frame->dl = bossLitTexEnvColorDL(0, 0, 0, 0);
            }
            if (w->action == 0x2E) {
                w->frame->dl = bossLitTexEnvColorDL(0, 0, 0, 0);
            }
            if (w->action == 0x30) {
                w->frame->dl = bossLitTexEnvColorDL(0, 0, 0, 0);
            }
            /* The shadow: on the floor, or a little under him when he is low. */
            if (w->frame->pos[1] <= -600.0f) {
                y = -1000.0f;
            } else {
                y = w->frame->pos[1] - 400.0f;
            }
            if (w->action == 14) {
                shadowPut(w->shadow, w->frame->pos[0], y, w->frame->pos[2], 1.0f, NULL);
            } else {
                shadowPut(w->shadow, w->frame->pos[0], y, w->frame->pos[2], 70.0f, NULL);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
