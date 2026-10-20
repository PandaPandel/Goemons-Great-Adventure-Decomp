#include "tlb.h"
#include "tlb/tlb_0327.h"

/* "Bdev_proc": the Dev robot boss ("デヴィル", the devil), the Goemon Impact's fourth boss. One
 * task drives the whole battle: its body and a second model that follows it ("cape"), two blue
 * after-images of both, the attack and damage boxes, the entrance demo, the attacks, the hits
 * and the two endings. Bbisu (tlb_0323_0) and Bkas (tlb_0326_0) are the same template.
 *
 * Its debug prints (the hits, "Mstep/Sstep/Stimer", the demos, BOSSLIMIT) are compiled out;
 * DPRINTF keeps each string, in source order, and emits no code. */

/* A print the original left in: keeps its string, emits no code. */
#define DPRINTF

extern Obj *gBattleTask; /* the battle's task (beffect.c) */
extern CamView *gBattleCamView; /* the battle's camera (beffect.c) */
extern s32 gAfterImageAlpha1; /* the first after-image's alpha, 0-48 (bmove.c) */
extern s32 gAfterImageAlpha2; /* the second's, 0-32 */

#define sinf (*sinf)
#define cosf (*cosf)

/* acollAtkAddYZ and acollDmgAddYZ return their box (main declares them void). */
typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                    s32 a);

/* This boss's other parts (tlb_0327_1), each started under the effect task. */
#define onibiCreate (*onibiCreate)
#define pentaCreate (*pentaCreate)
#define ironballCreate (*ironballCreate)
#define pentaniCreate (*pentaniCreate)
#define kesaEffectCreate (*kesaEffectCreate)
#define pentaniDemoCreate (*pentaniDemoCreate)
#define curseballRCreate (*curseballRCreate)
#define impactBulletCreate (*impactBulletCreate)
#define bdevBappcamProc (*bdevBappcamProc)
#define bdevBisuGCreate (*bdevBisuGCreate)
#define bdevBisuG2Create (*bdevBisuG2Create)

AnimFrame bdevBodyAnim[] = {
    { 0x030189F0, 15 },
    { 0x030199F0, 15 },
    { 0, 0 },
    { 0, 0 },
}; /* the body's texture animation (file 0xF0) */

/* The model animations, in file 0xF0. */
extern u8 bdevBodyIdleAnim[]; /* the body: idle */
extern u8 bdevCapeIdleAnim[]; /* the cape: idle */
extern u8 bdevFakeImpactWindUpAnim[]; /* the fake Impact attack: wind up */
extern u8 bdevFakeImpactStep3Anim[];
extern u8 bdevFakeImpactStep4Anim[];
extern u8 bdevFakeImpactStep5Anim[];
extern u8 bdevFakeImpactStep7Anim[];
extern u8 bdevSlashRaiseAnim[]; /* the slash: raise */
extern u8 bdevSlashSwingAnim[]; /* the slash: swing */
extern u8 bdevSplitBeginAnim[]; /* the split: begin */
extern u8 bdevSplitEndAnim[]; /* the split: end */
extern u8 bdevBarrierStep2Anim[];
extern u8 bdevBarrierStep3Anim[];
extern u8 bdevIronBallRaiseAnim[]; /* the iron ball: raise */
extern u8 bdevIronBallHoldAnim[]; /* the iron ball: hold */
extern u8 bdevIronBallLowerAnim[]; /* the iron ball: lower */
extern u8 bdevWillOWispsBodyAnim[]; /* the will-o'-the-wisps: the body */
extern u8 bdevCapeAttackAnim[]; /* the cape's attack */
extern u8 bdevKnockedBackAnim0[]; /* knocked back (one side) */
extern u8 bdevKnockedBackAnim1[]; /* knocked back (the other side) */
extern u8 bdevHundredFistAnim[];
extern u8 bdevThrownDownAnim[]; /* thrown down */
extern u8 bdevKnockedDownAnim[];
extern u8 bdevEntranceDemoAnim[];
extern u8 bdevCapeFoldedAnim[]; /* the cape, folded */
extern u8 bdevGhostShipStep3Anim[];
extern u8 bdevGhostShipStep4Anim[];

/* The boss's work (at obj + 0x20). */
typedef struct BdevW {
    /* 0x00 */ AnimeFrame *frame; /* the body */
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ ACollFrame *atk; /* its attack box (kind 0xCD) */
    /* 0x18 */ ACollFrame *dmg; /* its damage box (kind 0xCC) */
    /* 0x1C */ Frame *shadow; /* Bbisu's and Bkas's shadow slot: freed with the rest; nothing here makes it */
    /* 0x20 */ Anim anim; /* the body's texture animation */
    /* 0x30 */ u8 pad30[4];
    /* 0x34 */ AnimeFrame *cape; /* the second model, on the body, turned on its own */
    /* 0x38 */ AnimeFrame *ghost[4]; /* the after-images: body, cape, body, cape */
    /* 0x48 */ u8 pad48[4];
    /* 0x4C */ s32 hitWait; /* steps before the damage box is live again after a flinch */
    /* 0x50 */ u8 action; /* "Mstep" (bossStartAction sets it) */
    /* 0x51 */ u8 step; /* "Sstep" */
    /* 0x52 */ u8 nextAction; /* the action and step to resume after a move (bossStartActionKind) */
    /* 0x53 */ u8 nextStep;
    /* 0x54 */ s32 timer; /* "Stimer" */
    /* 0x58 */ u8 count; /* Bbisu's/Bkas's: set to 30 when knocked back (a random 0-99 once); not read here */
    /* 0x5C */ s32 glow; /* the yellow glow's alpha, 0-160, while gBossFlags 0x4000 */
} BdevW;

#define BDEV(obj) ((BdevW *) ((u32) (obj) + sizeof(Obj)))

/* Angles: one degree, and the multiples the code adds. */
#define DEG 0.0174532930038800785
#define DEG_120 2.0943951604656093
#define DEG_240 4.1887903209312185

/* The after-images fade out. */
#define TRAIL_FADE()                     \
    gAfterImageAlpha1 -= 4;            \
    gAfterImageAlpha2 -= 6;            \
    if (gAfterImageAlpha1 < 0) {       \
        gAfterImageAlpha1 = 0;         \
    }                                    \
    if (gAfterImageAlpha2 < 0) {       \
        gAfterImageAlpha2 = 0;         \
    }

/* The after-images come back, turned the body's way. */
#define TRAIL_SHOW(w)                                  \
    gAfterImageAlpha1 += 8;                          \
    gAfterImageAlpha2 += 12;                         \
    if (gAfterImageAlpha1 > 48) {                    \
        gAfterImageAlpha1 = 48;                      \
    }                                                  \
    if (gAfterImageAlpha2 > 32) {                    \
        gAfterImageAlpha2 = 32;                      \
    }                                                  \
    w->ghost[0]->rot[1] = w->frame->rot[1];            \
    w->ghost[1]->rot[1] = w->frame->rot[1];            \
    w->ghost[2]->rot[1] = w->frame->rot[1];            \
    w->ghost[3]->rot[1] = w->frame->rot[1];

/* The after-images on the body, a third of a turn apart (the three-way split). */
#define GHOSTS_ON_BODY(w)                                                                              \
    w->ghost[0]->pos[0] = w->ghost[1]->pos[0] = w->ghost[2]->pos[0] = w->ghost[3]->pos[0] =            \
        w->frame->pos[0];                                                                              \
    w->ghost[0]->pos[1] = w->ghost[1]->pos[1] = w->ghost[2]->pos[1] = w->ghost[3]->pos[1] =            \
        w->frame->pos[1];                                                                              \
    w->ghost[0]->pos[2] = w->ghost[1]->pos[2] = w->ghost[2]->pos[2] = w->ghost[3]->pos[2] =            \
        w->frame->pos[2];                                                                              \
    w->ghost[0]->rot[1] = w->frame->rot[1] + DEG_120;                                                  \
    w->ghost[1]->rot[1] = w->frame->rot[1] + DEG_120;                                                  \
    w->ghost[2]->rot[1] = w->frame->rot[1] + DEG_240;                                                  \
    w->ghost[3]->rot[1] = w->frame->rot[1] + DEG_240;

/* The body moves by its velocity. */
#define MOVE(w)                          \
    w->frame->pos[0] += w->vel[0];       \
    w->frame->pos[1] += w->vel[1];       \
    w->frame->pos[2] += w->vel[2];

/* The cape leans with the velocity. */
#define CAPE_TILT(w)                                     \
    if (w->vel[2] * DEG * 0.5 > 0.0) {                   \
        w->cape->rot[0] = w->vel[2] * DEG * 0.5;         \
    } else {                                             \
        w->cape->rot[0] = -(w->vel[2] * DEG * 0.5);      \
    }                                                    \
    w->cape->rot[2] = -w->vel[0] * DEG * 0.5;

/* The body leans with the velocity. */
#define BODY_TILT(w)                                     \
    w->frame->rot[0] = w->vel[2] * DEG * 0.75;           \
    w->frame->rot[2] = -w->vel[0] * DEG * 0.5;

/* Both models turn toward the velocity. */
#define TURN(w)                                                   \
    bossTurnToDir(w->frame, w->vel[0], w->vel[2], 2.0f);          \
    bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);

/* bossSetAnimAt takes the animation as a pointer (tlb.h, bmove.c). As a (u32) conversion at
 * each call the address is an expression to uopt, not a constant: it is then kept and spilled
 * where the game rebuilds it (lui/addiu). */

/* A new body animation, with its texture animation. */
#define BODY_ANIM(w, data, at, index)                                   \
    bossSetAnimAt(w->frame, data, 0xF0, at);                            \
    fadeAnimStart(&w->anim, bdevBodyAnim, index, &w->frame->segs[3], floadGetSetFile(0xF0, 0));

/* Back to waiting: the after-images return and the action ends. */
#define END_ACTION(w, obj) \
    TRAIL_SHOW(w)          \
    bossClearAction(obj);

#define ABSDIFF(a, b) ((a) > (b) ? (a) - (b) : -((a) - (b)))

/* The game reads msg back from its home, `lhu` for the switch and `lh` for tskDefaultProc: a
 * plain `switch (msg)` gives both (a kept FAKE, 2026-10-17, in place of `*(u16 *) &msg`; the
 * same in Btai's, Bfri's, Bbisu's and Bkas's procs). A u16 msg masks a1 and spills it, a (u16)
 * cast loads lh and masks (7947).
 *  - bossSetAnimAt takes the address as a pointer: a (u32) cast makes it an expression, and
 *    IDO spills it rather than forming it again.
 *  - GHOSTS_ON_BODY's chains are written ghost[0] = [1] = [2] = [3] (IDO stores the innermost
 *    first, as the game stores ghost[3] first).
 *  - Case 0's sqrt goes into angle (the game has no local for it).
 *  - The flag pairs follow the game's order (actions 10, 12 and 15), the macros' uses are one
 *    line each (IDO_NOTES 12), and action 6's cape turn is DEG.
 * The way here (real 577 to a match, 2026-10-12) is in the CHANGELOG, (738)-(740). */

/* FAKE: create's bossSetFrameFlags2(w->ghost[1], 0x20) called as returning s32 (its result
 * dropped; IDO_NOTES 369). With a void callee v0 is free at that call, and IDO colors
 * w->ghost[1] v0 and w->ghost[3] v1 through the whole function; the game has the reverse. All
 * six calls as s32: real 187 (the cape and ghost[3] move instead). Only this call matches. What
 * the original wrote here is unknown. A kept FAKE (2026-10-17): the plain void call is real 38. */
#define SET_FLAGS2_S32 ((s32 (*)(AnimeFrame *, s32)) bossSetFrameFlags2)
s32 Bdev_proc(Obj *obj, s16 msg, void *arg) {
    s32 pad0[9];
    f32 hx = 0.0f; /* a point ahead of the cockpit (bossHeadingOffset) */
    s32 pad1;
    f32 hz = 0.0f;
    f32 angle;
    s32 pad2;
    s32 pad3;
    f32 ox = 0.0f; /* an offset from the body (bossHeadingOffset) */
    f32 oz = 0.0f;
    s32 pad4;
    s32 pad5;
    BdevW *w = BDEV(obj);
    f32 dx1;
    f32 dy1;
    f32 dz1;
    f32 dx2;
    f32 dy2;
    f32 dz2;
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "Bdev_proc";
            gBattleTask = obj;
            if ((w->frame = (AnimeFrame *) frmCreate(gFrmLayers[2], gFrmKindAnime)) == NULL) {
                return -1;
            }
            if ((w->cape = (AnimeFrame *) frmCreate(gFrmLayers[2], gFrmKindAnime)) == NULL) {
                return -1;
            }
            if ((w->ghost[0] = (AnimeFrame *) frmCreate(gFrmLayers[8], gFrmKindAnime)) == NULL) {
                return -1;
            }
            if ((w->ghost[1] = (AnimeFrame *) frmCreate(gFrmLayers[8], gFrmKindAnime)) == NULL) {
                return -1;
            }
            if ((w->ghost[2] = (AnimeFrame *) frmCreate(gFrmLayers[8], gFrmKindAnime)) == NULL) {
                return -1;
            }
            if ((w->ghost[3] = (AnimeFrame *) frmCreate(gFrmLayers[8], gFrmKindAnime)) == NULL) {
                return -1;
            }
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(uda_atk_root_frame, &w->frame->f, 400, 0, 0, 0)) ==
                NULL) {
                return -1;
            }
            w->atk->kind = 0xCD;
            if ((w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->frame->f, 350, 700, -350, 0,
                                                          0)) == NULL) {
                return -1;
            }
            w->dmg->kind = 0xCC;
            BODY_ANIM(w, bdevBodyIdleAnim, 0.0f, 0);
            bossSetFrameFlags2(w->frame, 0x20);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 2.0f;
            bossSetAnimAt(w->cape, bdevCapeIdleAnim, 0xF0, 0.0f);
            bossSetFrameFlags2(w->cape, 0x20);
            w->cape->scale[0] = w->cape->scale[1] = w->cape->scale[2] = 2.0f;
            /* The after-images run a little behind the body's animation. */
            bossSetAnimAt(w->ghost[0], bdevBodyIdleAnim, 0xF0, 0.11f);
            fadeAnimStart(&w->anim, bdevBodyAnim, 0, &w->frame->segs[3], floadGetSetFile(0xF0, 0));
            bossSetFrameFlags2(w->ghost[0], 0x20);
            w->ghost[0]->scale[0] = w->ghost[0]->scale[1] = w->ghost[0]->scale[2] = 2.0f;
            bossSetAnimAt(w->ghost[1], bdevCapeIdleAnim, 0xF0, 0.11f);
            SET_FLAGS2_S32(w->ghost[1], 0x20);
            w->ghost[1]->scale[0] = w->ghost[1]->scale[1] = w->ghost[1]->scale[2] = 2.0f;
            bossSetAnimAt(w->ghost[2], bdevBodyIdleAnim, 0xF0, 0.22f);
            fadeAnimStart(&w->anim, bdevBodyAnim, 0, &w->frame->segs[3], floadGetSetFile(0xF0, 0));
            bossSetFrameFlags2(w->ghost[0], 0x20); /* sic: the game sets the first one's again */
            w->ghost[2]->scale[0] = w->ghost[2]->scale[1] = w->ghost[2]->scale[2] = 2.0f;
            bossSetAnimAt(w->ghost[3], bdevCapeIdleAnim, 0xF0, 0.22f);
            bossSetFrameFlags2(w->ghost[3], 0x20);
            w->ghost[3]->scale[0] = w->ghost[3]->scale[1] = w->ghost[3]->scale[2] = 2.0f;
            w->frame->pos[0] = 3000.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 3000.0f;
            w->cape->pos[0] = w->frame->pos[0];
            w->cape->pos[1] = w->frame->pos[1];
            w->cape->pos[2] = w->frame->pos[2];
            w->ghost[0]->pos[0] = w->frame->pos[0];
            w->ghost[0]->pos[1] = w->frame->pos[1];
            w->ghost[0]->pos[2] = w->frame->pos[2] - 500.0f;
            w->ghost[1]->pos[0] = w->frame->pos[0];
            w->ghost[1]->pos[1] = w->frame->pos[1];
            w->ghost[1]->pos[2] = w->frame->pos[2] - 500.0f;
            w->ghost[2]->pos[0] = w->frame->pos[0];
            w->ghost[2]->pos[1] = w->frame->pos[1];
            w->ghost[2]->pos[2] = w->frame->pos[2] - 1000.0f;
            w->ghost[3]->pos[0] = w->frame->pos[0];
            w->ghost[3]->pos[1] = w->frame->pos[1];
            w->ghost[3]->pos[2] = w->frame->pos[2] - 1000.0f;
            w->action = 0;
            w->step = 0;
            w->timer = 0;
            gBossLifeMax = 6666;
            gBossLife = 6666;
            gBossActionArg = 0;
            gBossPhase = 0;
            gBossActionCount = 0;
            DPRINTF("登場デモ開始\n");
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->cape->f);
            frmDestroy(&w->atk->frame);
            frmDestroy(&w->dmg->frame);
            frmDestroy(&w->ghost[0]->f);
            frmDestroy(&w->ghost[1]->f);
            frmDestroy(&w->ghost[2]->f);
            frmDestroy(&w->ghost[3]->f);
            frmDestroy(w->shadow);
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
            /* A pentagram under the arena every three seconds, but in the entrance demo. */
            if (w->action != 12) {
                if (gGameFrameCount % 180 == 0) {
                    pentaCreate(0.0f, -500.0f, -4000.0f, 0);
                }
                if (gGameFrameCount % 180 == 45) {
                    pentaCreate(0.0f, -500.0f, -4000.0f, 0);
                }
            }
            /* The after-images follow: the first toward the body, the second toward the first. */
            if (ABSDIFF(w->frame->pos[0], w->ghost[0]->pos[0]) > 1.0 &&
                ABSDIFF(w->frame->pos[1], w->ghost[0]->pos[1]) > 1.0 &&
                ABSDIFF(w->frame->pos[2], w->ghost[0]->pos[2]) > 1.0) {
                /* The distance in angle: the game has no local of its own for it (its frame). */
                angle = (*sqrtf)(bossDistSq(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], w->ghost[0]->pos[0],
                                        w->ghost[0]->pos[1], w->ghost[0]->pos[2]));
                dx1 = (w->frame->pos[0] - w->ghost[0]->pos[0]) / angle *45.0f;
                dy1 = (w->frame->pos[1] - w->ghost[0]->pos[1]) / angle *45.0f;
                dz1 = (w->frame->pos[2] - w->ghost[0]->pos[2]) / angle *45.0f;
            } else {
                dx1 = w->vel[0];
                dy1 = w->vel[1];
                dz1 = w->vel[2];
            }
            if (ABSDIFF(w->ghost[0]->pos[0], w->ghost[2]->pos[0]) > 1.0 &&
                ABSDIFF(w->ghost[0]->pos[1], w->ghost[2]->pos[1]) > 1.0 &&
                ABSDIFF(w->ghost[0]->pos[2], w->ghost[2]->pos[2]) > 1.0) {
                angle = (*sqrtf)(bossDistSq(w->ghost[0]->pos[0], w->ghost[0]->pos[1], w->ghost[0]->pos[2],
                                        w->ghost[2]->pos[0], w->ghost[2]->pos[1], w->ghost[2]->pos[2]));
                dx2 = (w->ghost[0]->pos[0] - w->ghost[2]->pos[0]) / angle *40.0f;
                dy2 = (w->ghost[0]->pos[1] - w->ghost[2]->pos[1]) / angle *40.0f;
                dz2 = (w->ghost[0]->pos[2] - w->ghost[2]->pos[2]) / angle *40.0f;
            } else {
                dx2 = w->vel[0];
                dy2 = w->vel[1];
                dz2 = w->vel[2];
            }
            w->ghost[0]->pos[0] += dx1;
            w->ghost[0]->pos[1] += dy1;
            w->ghost[0]->pos[2] += dz1;
            w->ghost[1]->pos[0] += dx1;
            w->ghost[1]->pos[1] += dy1;
            w->ghost[1]->pos[2] += dz1;
            w->ghost[2]->pos[0] += dx2;
            w->ghost[2]->pos[1] += dy2;
            w->ghost[2]->pos[2] += dz2;
            w->ghost[3]->pos[0] += dx2;
            w->ghost[3]->pos[1] += dy2;
            w->ghost[3]->pos[2] += dz2;
            w->ghost[0]->rot[0] = w->frame->rot[0];
            w->ghost[1]->rot[0] = w->frame->rot[0];
            w->ghost[2]->rot[0] = w->frame->rot[0];
            w->ghost[3]->rot[0] = w->frame->rot[0];
            if (w->vel[0] != 0.0f || w->vel[1] != 0.0f || w->vel[2] != 0.0f) {
                bossTurnToDir(w->ghost[0], dx1, dz1, 6.0f);
                bossTurnToDir(w->ghost[1], dx1, dz1, 6.0f);
                bossTurnToDir(w->ghost[2], dx2, dz2, 6.0f);
                bossTurnToDir(w->ghost[3], dx2, dz2, 6.0f);
            }
            w->ghost[0]->rot[2] = w->frame->rot[2];
            w->ghost[1]->rot[2] = w->frame->rot[2];
            w->ghost[2]->rot[2] = w->frame->rot[2];
            w->ghost[3]->rot[2] = w->frame->rot[2];
            if (gBattleFlags2 & 0x4000) {
                if (gImpactMoveTime > 80) {
                    w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 2.0f;
                } else {
                    w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 2.0f;
                }
            } else {
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 2.0f;
            }
            /* The battle opens with the entrance demo. */
            if (!(gBattleFlags2 & 4)) {
                gBattleFlags2 |= 4;
                bossStartAction(obj, 0, 12);
            }
            w->cape->pos[0] = w->frame->pos[0];
            w->cape->pos[1] = w->frame->pos[1];
            w->cape->pos[2] = w->frame->pos[2];
            gMegaBurstY = gBossPosY;
            acollAtkWasHit(w->atk);

            /* The hits. */
            if (acollDmgWasHit(w->dmg)) {
                if (w->dmg->hitKind == 15) {
                    DPRINTF("バルカンに当たりました。[%d]\n", gBossLife);
                    if (gBossFlags & 4) {
                        bossDamage(0, 15, 1);
                    } else {
                        itexFlash(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 1.0f);
                        SoundLocation2(0x22B, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                       w->dmg->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                        bossDamage(6, 15, 1);
                    }
                    if (gBossLife <= 0) {
                        DPRINTF("ボス死亡しました。\n");
                        gBattleFlags2 |= 0x400;
                        w->dmg->flags &= ~2;
                        angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                        w->vel[0] = sinf(angle) * 15.0f;
                        w->vel[2] = cosf(angle) * 15.0f;
                        w->count = 30;
                        w->action = 0x2B;
                        w->step = 0;
                        w->vel[1] = 120.0f;
                    }
                } else if (w->dmg->hitKind == 16) {
                    DPRINTF("外部バルカンに当たりました。\n");
                    if (gBossFlags & 4) {
                        bossDamage(0, 16, 1);
                    } else {
                        itexFlash(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 1.0f);
                        SoundLocation2(0x22B, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                       w->dmg->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                        bossDamage(6, 16, 1);
                    }
                    if (gBossLife <= 0) {
                        DPRINTF("ボス死亡しました。\n");
                        gBattleFlags2 |= 0x400;
                        w->dmg->flags &= ~2;
                        angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                        w->vel[0] = sinf(angle) * 15.0f;
                        w->vel[2] = cosf(angle) * 15.0f;
                        w->count = 30;
                        w->action = 0x2B;
                        w->step = 0;
                        w->vel[1] = 120.0f;
                    }
                } else if (w->dmg->hitKind == 1) {
                    DPRINTF("ジャブに当たりました。\n");
                    if (gBossFlags & 0x4000) {
                        DPRINTF("効かぬわ。\n");
                    } else {
                        beHitmarkCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 3.0f);
                        SoundLocation2(0x27C, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                       w->dmg->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                        if (gBossFlags & 0x1000) {
                            bossDamage(0, 1, 1);
                        } else {
                            bossDamage(40, 1, 1);
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
                        angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                        w->vel[0] = sinf(angle) * 50.0f;
                        w->vel[2] = cosf(angle) * 50.0f;
                        w->count = 30;
                        w->action = 0x2F;
                        w->step = 0;
                        w->vel[1] = 35.0f;
                    }
                } else if (w->dmg->hitKind == 3) {
                    DPRINTF("ストレートに当たりました。\n");
                    if (gBossFlags & 0x4000) {
                        DPRINTF("効かぬわ。\n");
                    } else {
                        beHitmarkCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f);
                        SoundLocation2(0x27D, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                       w->dmg->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                        w->dmg->flags &= ~2;
                        bossDamage(100, 3, 2);
                        gBossFlags &= ~0x200;
                        gBossFlags |= 0x8000;
                        gBrotherFlags &= ~0x20000;
                        w->frame->rot[0] = w->frame->rot[1] = w->frame->rot[2] = 0.0f;
                        angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                        w->vel[0] = sinf(angle) * 30.0f;
                        w->vel[2] = cosf(angle) * 30.0f;
                        w->count = 30;
                        w->action = 0x2B;
                        w->step = 0;
                        w->vel[1] = 30.0f;
                    }
                    if (gBossLife <= 0) {
                        DPRINTF("ボス死亡しました。\n");
                        gBattleFlags2 |= 0x400;
                        w->dmg->flags &= ~2;
                        angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                        w->vel[0] = sinf(angle) * 50.0f;
                        w->vel[2] = cosf(angle) * 50.0f;
                        w->count = 30;
                        w->action = 0x2F;
                        w->step = 0;
                        w->vel[1] = 35.0f;
                    }
                } else if (w->dmg->hitKind == 8) {
                    DPRINTF("キックに当たりました。\n");
                    if (gBossFlags & 0x4000) {
                        DPRINTF("効かぬわ。\n");
                    } else {
                        bossDamage(160, 8, 3);
                        sndPlay(0x102);
                        beHitmarkCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f);
                        w->dmg->flags &= ~2;
                        gBossFlags &= ~0x200;
                        gBossFlags |= 0x8000;
                        gBrotherFlags &= ~0x20000;
                        angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                        w->vel[0] = sinf(angle) * 50.0f;
                        w->vel[2] = cosf(angle) * 50.0f;
                        w->count = 30;
                        w->vel[1] = 35.0f;
                        w->frame->rot[1] = mathAtan2(-w->frame->pos[0], -w->frame->pos[2]);
                        w->action = 0x2F;
                        w->step = 0;
                    }
                    if (gBossLife <= 0) {
                        DPRINTF("ボス死亡しました。\n");
                        gBattleFlags2 |= 0x400;
                        w->dmg->flags &= ~2;
                        angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                        w->vel[0] = sinf(angle) * 50.0f;
                        w->vel[2] = cosf(angle) * 50.0f;
                        w->count = 30;
                        w->action = 0x2F;
                        w->step = 0;
                        w->vel[1] = 35.0f;
                    }
                } else if (w->dmg->hitKind == 0x73) {
                    /* Bismaru, knocked flying into the boss. */
                    DPRINTF("ふっとんだビスマルに当たりました。\n");
                    beHitmarkCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f);
                    SoundLocation2(0x27D, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2],
                                   300.0f, 15000.0f, gBattleCamView->fovy);
                    w->dmg->flags &= ~2;
                    gBossFlags &= ~0x4000;
                    gBossFlags &= ~0x200;
                    gBossFlags |= 0x8000;
                    gBrotherFlags &= ~0x20000;
                    w->frame->rot[0] = w->frame->rot[1] = w->frame->rot[2] = 0.0f;
                    angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                    w->vel[0] = sinf(angle) * 30.0f;
                    w->vel[2] = cosf(angle) * 30.0f;
                    w->count = 30;
                    w->action = 0x2B;
                    w->step = 0;
                    w->vel[1] = 30.0f;
                    if (gBossLife <= 0) {
                        DPRINTF("ボス死亡しました。\n");
                        gBattleFlags2 |= 0x400;
                        w->dmg->flags &= ~2;
                        angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                        w->vel[0] = sinf(angle) * 15.0f;
                        w->vel[2] = cosf(angle) * 15.0f;
                        w->count = 30;
                        w->action = 0x2B;
                        w->step = 0;
                        w->vel[1] = 120.0f;
                    }
                } else if (w->dmg->hitKind == 11) {
                    DPRINTF("んが砲に当たりました。\n");
                    if (gBossFlags & 0x4000) {
                        beMegaRefCreate(0.0f, 0.0f, 0.0f);
                        DPRINTF("効かぬわ。\n");
                    } else {
                        DPRINTF("効いてるよ。\n");
                        beHitmarkCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f);
                        if (gBattleFlags2 & 0x40000) {
                            bossDamage(1600, 11, 0);
                        } else {
                            bossDamage(800, 11, 0);
                        }
                        w->dmg->flags &= ~2;
                        gBossFlags &= ~0x200;
                        gBrotherFlags &= ~0x20000;
                        w->count = 30;
                        w->action = 0x30;
                        w->step = 0;
                    }
                    gBrotherFlags &= ~0x40000;
                    gBrotherFlags |= 0x80000;
                    if (gBossLife <= 0) {
                        gBattleFlags2 |= 0x400;
                        w->dmg->flags &= ~2;
                    }
                } else if (w->dmg->hitKind == 0xCA) {
                    DPRINTF("外部インパクトに攻撃されました。\n");
                    if (gBossFlags & 0x4000) {
                        DPRINTF("効かぬわ。\n");
                    } else {
                        beHitmarkCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 3.0f);
                        SoundLocation2(0x27D, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                       w->dmg->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                        w->dmg->flags &= ~2;
                        gBossFlags &= ~0x200;
                        angle = mathAtan2(w->dmg->hitPos[0], w->dmg->hitPos[2]);
                        w->vel[0] = sinf(angle) * -30.0f;
                        w->vel[2] = cosf(angle) * -30.0f;
                        w->action = 0x2B;
                        w->step = 0;
                        w->vel[1] = 40.0f;
                    }
                    if (gBossLife <= 0) {
                        DPRINTF("ボス死亡しました。\n");
                        gBattleFlags2 |= 0x400;
                        w->dmg->flags &= ~2;
                        angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                        w->vel[0] = sinf(angle) * 15.0f;
                        w->vel[2] = cosf(angle) * 15.0f;
                        w->count = 30;
                        w->action = 0x2B;
                        w->step = 0;
                        w->vel[1] = 120.0f;
                    }
                } else if (w->dmg->hitKind == 5) {
                    DPRINTF("百烈パンチに当たりました。\n");
                    if (gBossFlags & 0x4000) {
                        DPRINTF("効かぬわ。\n");
                    } else {
                        w->frame->pos[1] = -100.0f;
                        gBattleFlags2 |= 0x2000;
                        bossDamage(20, 3, 1);
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
            bossStartActionIfFlag(obj, 8);
            if (gBattleFlags & 8) {
                gBattleFlags &= ~8;
                if (w->action == 4) {
                    bossStartAction(obj, 30, 9);
                }
                if (w->action == 13) {
                    bossStartAction(obj, 30, 9);
                }
            }

            switch (w->action) {
                case 0:
                    /* Waiting: drift about the far side of the arena and pick the next attack.
                     * The attack order has three lists, by the life left. */
                    TRAIL_SHOW(w);
                    w->cape->f.flags |= 2;
                    gBossPhase = 0;
                    gBossActionArg--;
                    if (gBossLife < gBossLifeMax * 0.66666) {
                        gBossPhase = 1;
                    }
                    if (gBossLife < gBossLifeMax * 0.33333) {
                        gBossPhase = 2;
                    }
                    if (gBossActionArg <= 0) {
                        if (gBossPhase == 0) {
                            if (gBossActionCount == 0) {
                                bossStartAction(obj, 40, 13);
                            } else if (gBossActionCount == 1) {
                                bossStartAction(obj, 40, 2);
                            } else if (gBossActionCount == 2) {
                                bossStartAction(obj, 40, 17);
                            } else if (gBossActionCount == 3) {
                                bossStartAction(obj, 40, 4);
                            } else if (gBossActionCount == 4) {
                                bossStartAction(obj, 40, 5);
                            } else if (gBossActionCount == 5) {
                                bossStartAction(obj, 40, 7);
                            } else if (gBossActionCount == 6) {
                                bossStartAction(obj, 40, 2);
                            } else {
                                gBossActionCount = 0;
                            }
                        } else if (gBossPhase == 1) {
                            if (gBossActionCount == 0) {
                                bossStartAction(obj, 30, 6);
                            } else if (gBossActionCount == 1) {
                                bossStartAction(obj, 30, 5);
                            } else if (gBossActionCount == 2) {
                                bossStartAction(obj, 30, 4);
                            } else if (gBossActionCount == 3) {
                                bossStartAction(obj, 30, 13);
                            } else if (gBossActionCount == 4) {
                                bossStartAction(obj, 60, 5);
                            } else if (gBossActionCount == 5) {
                                bossStartAction(obj, 20, 5);
                            } else if (gBossActionCount == 6) {
                                bossStartAction(obj, 30, 4);
                            } else if (gBossActionCount == 7) {
                                bossStartAction(obj, 30, 6);
                            } else if (gBossActionCount == 8) {
                                bossStartAction(obj, 30, 13);
                            } else if (gBossActionCount == 9) {
                                bossStartAction(obj, 30, 10);
                            } else if (gBossActionCount == 10) {
                                bossStartAction(obj, 30, 2);
                            } else {
                                gBossActionCount = 0;
                            }
                        } else if (gBossPhase == 2) {
                            if (gBossActionCount == 0) {
                                bossStartAction(obj, 20, 6);
                            } else if (gBossActionCount == 1) {
                                bossStartAction(obj, 20, 4);
                            } else if (gBossActionCount == 2) {
                                bossStartAction(obj, 20, 17);
                            } else if (gBossActionCount == 3) {
                                bossStartAction(obj, 60, 5);
                            } else if (gBossActionCount == 4) {
                                bossStartAction(obj, 10, 5);
                            } else if (gBossActionCount == 5) {
                                bossStartAction(obj, 20, 7);
                            } else if (gBossActionCount == 6) {
                                bossStartAction(obj, 20, 13);
                            } else if (gBossActionCount == 7) {
                                bossStartAction(obj, 20, 10);
                            } else if (gBossActionCount == 8) {
                                bossStartAction(obj, 20, 4);
                            } else if (gBossActionCount == 9) {
                                bossStartAction(obj, 20, 7);
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
                        bossPatrolX(obj, -1200.0f, 1200.0f, 0.6f);
                        bossPatrolY(obj, 800.0f, -400.0f, 0.3f);
                        bossPatrolZ(obj, -6000.0f, -5000.0f, 0.3f);
                    }
                    gBossPosX = w->frame->pos[0];
                    gBossPosY = w->frame->pos[1];
                    gBossPosZ = w->frame->pos[2];
                    gBossYaw = w->frame->rot[1];
                    BODY_TILT(w);
                    CAPE_TILT(w);
                    MOVE(w);
                    TURN(w);
                    bossStepAnim(w->frame, 0.3f, 0);
                    bossStepAnim(w->cape, 0.3f, 0);
                    bossStepAnim(w->ghost[0], 0.3f, 0);
                    bossStepAnim(w->ghost[1], 0.3f, 0);
                    bossStepAnim(w->ghost[2], 0.3f, 0);
                    bossStepAnim(w->ghost[3], 0.3f, 0);
                    break;
                case 1:
                    /* Back off to the far left, then wait. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d]\n", w->action, w->step, w->timer);
                    MOVE(w);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    TURN(w);
                    switch (w->step) {
                        case 0:
                            bossStartActionKind(obj, 1, 1, 3);
                            break;
                        case 1:
                            break;
                        case 2:
                            break;
                        case 3:
                            bossClearAction(obj);
                            break;
                    }
                    w->timer++;
                    break;
                case 2:
                    /* The iron ball: come forward, raise it and throw it at the Impact. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d]\n", w->action, w->step, w->timer);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    MOVE(w);
                    TURN(w);
                    bossStepAnim(w->cape, 0.3f, 0);
                    switch (w->step) {
                        case 0:
                            bossStepAnim(w->frame, 0.3f, 0);
                            if (bossFlyToward(obj, w->frame->pos[0], -100.0f, -6000.0f, 3.0f)) {
                                w->step++;
                            }
                            break;
                        case 1:
                            TRAIL_FADE();
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 4.0f);
                            if (bossDampVel(obj, 0.8f) == 0) {
                                w->step++;
                            }
                            break;
                        case 2:
                            TRAIL_FADE();
                            bossStepAnim(w->frame, 0.3f, 0);
                            if (bossTurnToward2(w->frame, 0.0f, 0.0f, 4.0f)) {
                                BODY_ANIM(w, bdevIronBallRaiseAnim, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 3:
                            TRAIL_FADE();
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                w->step++;
                            }
                            break;
                        case 4:
                            TRAIL_FADE();
                            if (bossStepAnim(w->frame, 0.4f, 2)) {
                                BODY_ANIM(w, bdevIronBallHoldAnim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 5:
                            if (w->timer == 70) {
                                bossHeadingOffset(gBossYaw, 100.0f, 0.0f, &ox, &oz);
                                ironballCreate(w->frame->pos[0] + ox, w->frame->pos[1], w->frame->pos[2] + oz,
                                                       fadeRand(100) % 3);
                                SoundLocation2(0x216, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                            }
                            bossStepAnim(w->frame, 0.2f, 0);
                            if (w->timer == 150) {
                                BODY_ANIM(w, bdevIronBallLowerAnim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 6:
                            if (bossStepAnim(w->frame, 0.4f, 2)) {
                                BODY_ANIM(w, bdevBodyIdleAnim, 0.0f, 0);
                                END_ACTION(w, obj);
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 3:
                    /* The big slash ("Kesaeffect") from the middle of the arena. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d]\n", w->action, w->step, w->timer);
                    MOVE(w);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    TURN(w);
                    bossStepAnim(w->cape, 0.3f, 0);
                    switch (w->step) {
                        case 0:
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 4.0f);
                            if (bossDampVel(obj, 0.8f) == 0) {
                                w->frame->rot[0] = w->frame->rot[2] = 0.0f;
                                w->step++;
                            }
                            break;
                        case 1:
                            if (bossTurnToward2(w->frame, 0.0f, 0.0f, 4.0f)) {
                                w->step++;
                            }
                            break;
                        case 2:
                            w->frame->rot[1] = mathAtan2(-w->frame->pos[0], -w->frame->pos[2]);
                            if (bossFlyToward(obj, 100.0f, -200.0f, -900.0f, 3.0f) && bossDampVel(obj, 0.8f) == 0) {
                                TRAIL_FADE();
                                BODY_ANIM(w, bdevSlashRaiseAnim, 0.0f, 0);
                                w->frame->rot[0] = w->frame->rot[2] = 0.0f;
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 3:
                            w->frame->rot[1] = mathAtan2(-w->frame->pos[0], -w->frame->pos[2]);
                            if (w->timer == 40) {
                                kesaEffectCreate(0.0f, 0.0f, -400.0f, 0);
                                beFlashFCreate(0x80, 0, 0, 20);
                            }
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bdevSlashSwingAnim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 4:
                            w->frame->rot[1] = mathAtan2(-w->frame->pos[0], -w->frame->pos[2]);
                            bossFlyToward(obj, 0.0f, -200.0f, -3000.0f, 2.0f);
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bdevBodyIdleAnim, 0.0f, 0);
                                END_ACTION(w, obj);
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 4:
                    /* The three-way split: the after-images fan out a third of a turn apart and the
                     * boss sweeps round the arena, firing at the Impact. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d]\n", w->action, w->step, w->timer);
                    MOVE(w);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    bossStepAnim(w->cape, 0.3f, 0);
                    gBossFlags |= 4;
                    switch (w->step) {
                        case 0:
                            TRAIL_FADE();
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 4.0f);
                            if (bossDampVel(obj, 0.8f) == 0) {
                                w->frame->rot[0] = w->frame->rot[2] = 0.0f;
                                w->step++;
                            }
                            break;
                        case 1:
                            TRAIL_FADE();
                            if (bossTurnToward2(w->frame, 0.0f, 0.0f, 4.0f)) {
                                BODY_ANIM(w, bdevSplitBeginAnim, 0.0f, 0);
                                bossSetAnimAt(w->ghost[0], bdevSplitBeginAnim, 0xF0, 0.0f);
                                bossSetAnimAt(w->ghost[2], bdevSplitBeginAnim, 0xF0, 0.0f);
                                w->step++;
                            }
                            break;
                        case 2:
                            TRAIL_SHOW(w);
                            GHOSTS_ON_BODY(w);
                            w->frame->rot[1] -= 0.08726646501940039;
                            w->cape->rot[1] -= 0.08726646501940039;
                            w->frame->rot[0] *= 0.9;
                            w->frame->rot[2] *= 0.9;
                            w->cape->rot[0] *= 0.9;
                            w->cape->rot[2] *= 0.9;
                            bossStepAnim(w->ghost[0], 0.3f, 2);
                            bossStepAnim(w->ghost[2], 0.3f, 2);
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                w->step++;
                            }
                            break;
                        case 3:
                            TRAIL_SHOW(w);
                            GHOSTS_ON_BODY(w);
                            w->frame->rot[1] -= 0.34906586007760154;
                            w->cape->rot[1] -= 0.34906586007760154;
                            w->frame->rot[0] *= 0.8;
                            w->frame->rot[2] *= 0.8;
                            w->cape->rot[0] *= 0.8;
                            w->cape->rot[2] *= 0.8;
                            if (!(fadeRand(100) & 1)) {
                                w->step = 4;
                            } else {
                                w->step = 7;
                            }
                            break;
                        case 4:
                            /* Round by the right. */
                            GHOSTS_ON_BODY(w);
                            w->frame->rot[1] -= 0.34906586007760154;
                            w->cape->rot[1] -= 0.34906586007760154;
                            if (bossFlyToward(obj, 2000.0f, 0.0f, -2500.0f, 3.0f)) {
                                w->step++;
                            }
                            break;
                        case 5:
                            GHOSTS_ON_BODY(w);
                            w->frame->rot[1] -= 0.34906586007760154;
                            w->cape->rot[1] -= 0.34906586007760154;
                            if (bossFlyToward(obj, 2000.0f, 0.0f, -800.0f, 3.0f)) {
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 6:
                            GHOSTS_ON_BODY(w);
                            if (w->timer >= 48 && w->timer < 91 && !(w->timer & 7)) {
                                beNullattackCreate(0.0f, 0.0f, 0.0f, 0x68);
                            }
                            w->frame->rot[1] -= 0.34906586007760154;
                            w->cape->rot[1] -= 0.34906586007760154;
                            if (bossFlyToward(obj, -2000.0f, 0.0f, -800.0f, 3.0f)) {
                                w->step = 10;
                            }
                            break;
                        case 7:
                            /* Round by the left. */
                            GHOSTS_ON_BODY(w);
                            w->frame->rot[1] -= 0.34906586007760154;
                            w->cape->rot[1] -= 0.34906586007760154;
                            if (bossFlyToward(obj, -2000.0f, 0.0f, -2500.0f, 3.0f)) {
                                w->step++;
                            }
                            break;
                        case 8:
                            GHOSTS_ON_BODY(w);
                            w->frame->rot[1] -= 0.34906586007760154;
                            w->cape->rot[1] -= 0.34906586007760154;
                            if (bossFlyToward(obj, -2000.0f, 0.0f, -800.0f, 3.0f)) {
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 9:
                            GHOSTS_ON_BODY(w);
                            if (w->timer >= 48 && w->timer < 91 && !(w->timer & 7)) {
                                beNullattackCreate(0.0f, 0.0f, 0.0f, 0x68);
                            }
                            w->frame->rot[1] -= 0.34906586007760154;
                            w->cape->rot[1] -= 0.34906586007760154;
                            if (bossFlyToward(obj, 2000.0f, 0.0f, -800.0f, 3.0f)) {
                                w->step = 10;
                            }
                            break;
                        case 10:
                            TRAIL_FADE();
                            GHOSTS_ON_BODY(w);
                            w->frame->rot[1] -= 0.17453293003880077;
                            w->cape->rot[1] -= 0.17453293003880077;
                            if (bossFlyToward(obj, 0.0f, -200.0f, -4000.0f, 3.0f)) {
                                BODY_ANIM(w, bdevSplitEndAnim, 0.0f, 0);
                                bossSetAnimAt(w->ghost[0], bdevSplitEndAnim, 0xF0, 0.0f);
                                bossSetAnimAt(w->ghost[2], bdevSplitEndAnim, 0xF0, 0.0f);
                                w->step++;
                            }
                            break;
                        case 11:
                            TRAIL_FADE();
                            GHOSTS_ON_BODY(w);
                            TURN(w);
                            bossStepAnim(w->ghost[0], 0.3f, 2);
                            bossStepAnim(w->ghost[2], 0.3f, 2);
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bdevBodyIdleAnim, 0.0f, 0);
                                bossSetAnimAt(w->ghost[0], bdevBodyIdleAnim, 0xF0, 0.0f);
                                bossSetAnimAt(w->ghost[2], bdevBodyIdleAnim, 0xF0, 0.0f);
                                bossClearAction(obj);
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 5:
                    /* Three will-o'-the-wisps ("Onibi"), one after another. */
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    MOVE(w);
                    TURN(w);
                    bossStepAnim(w->cape, 0.3f, 0);
                    gBossFlags |= 4;
                    switch (w->step) {
                        case 0:
                            TRAIL_FADE();
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 4.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 4.0f);
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossStepAnim(w->cape, 0.3f, 0);
                            if (bossFlyToward(obj, w->frame->pos[0], -100.0f, -6000.0f, 3.0f)) {
                                w->step++;
                            }
                            break;
                        case 1:
                            TRAIL_FADE();
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 4.0f);
                            bossDampVel(obj, 0.8f);
                            if (bossTurnToward2(w->frame, 0.0f, 0.0f, 4.0f)) {
                                BODY_ANIM(w, bdevWillOWispsBodyAnim, 0.0f, 0);
                                bossSetAnimAt(w->cape, bdevCapeAttackAnim, 0xF0, 0.0f);
                                SoundLocation2(0x259, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 2:
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 4.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 4.0f);
                            bossStepAnim(w->cape, 0.3f, 2);
                            bossDampVel(obj, 0.8f);
                            if (w->timer == 40) {
                                DPRINTF("鬼火[1]発射します。\n");
                                bossHeadingOffset(gBossYaw, 0.0f, 300.0f, &ox, &oz);
                                onibiCreate(w->frame->pos[0] + ox, w->frame->pos[1] - 200.0f,
                                                       w->frame->pos[2] + oz, 1);
                                SoundLocation2(0x292, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                            }
                            if (w->timer == 60) {
                                DPRINTF("鬼火[2]発射します。\n");
                                bossHeadingOffset(gBossYaw, 0.0f, 300.0f, &ox, &oz);
                                onibiCreate(w->frame->pos[0] + ox, w->frame->pos[1] - 200.0f,
                                                       w->frame->pos[2] + oz, 2);
                                SoundLocation2(0x292, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                            }
                            if (w->timer == 80) {
                                DPRINTF("鬼火[3]発射します。\n");
                                bossHeadingOffset(gBossYaw, 0.0f, 300.0f, &ox, &oz);
                                onibiCreate(w->frame->pos[0] + ox, w->frame->pos[1] - 200.0f,
                                                       w->frame->pos[2] + oz, 3);
                                SoundLocation2(0x292, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                            }
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bdevBodyIdleAnim, 0.0f, 0);
                                bossSetAnimAt(w->cape, bdevCapeIdleAnim, 0xF0, 0.0f);
                                w->step++;
                            }
                            break;
                        case 3:
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossStepAnim(w->cape, 0.3f, 0);
                            if (w->timer == 220) {
                                END_ACTION(w, obj);
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 6:
                    /* The fake Impact: the boss turns into a copy of the Impact that fires at it. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d]\n", w->action, w->step, w->timer);
                    MOVE(w);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    bossStepAnim(w->cape, 0.3f, 0);
                    gBossFlags |= 4;
                    switch (w->step) {
                        case 0:
                            TRAIL_FADE();
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 4.0f);
                            bossStepAnim(w->frame, 0.3f, 0);
                            if (bossDampVel(obj, 0.8f) == 0) {
                                w->frame->rot[0] = w->frame->rot[2] = 0.0f;
                                w->step++;
                            }
                            break;
                        case 1:
                            TRAIL_FADE();
                            bossStepAnim(w->frame, 0.3f, 0);
                            if (bossTurnToward2(w->frame, 0.0f, 0.0f, 4.0f)) {
                                beReversalCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                                SoundLocation2(0x259, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                                gBrotherFlags |= 0x20000;
                                w->step++;
                            }
                            break;
                        case 2:
                            TRAIL_FADE();
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 4.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 4.0f);
                            bossStepAnim(w->frame, 0.3f, 0);
                            if (bossFlyToward(obj, gBrotherPosX, gBrotherPosY + 600.0f, gBrotherPosZ - 800.0f, 0.5f) &&
                                bossStepAnim(w->frame, 0.3f, 0)) {
                                BODY_ANIM(w, bdevFakeImpactWindUpAnim, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 3:
                            bossDampVel(obj, 0.8f);
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 4.0f);
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                effectKiranCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 500.0f);
                                beFlashFCreate(0xFF, 0xFF, 0xFF, 20);
                                BODY_ANIM(w, bdevFakeImpactStep3Anim, 0.0f, 0);
                                gBrotherFlags |= 0x40000;
                                bossHeadingOffset(gBossYaw, -100.0f, 0.0f, &ox, &oz);
                                fakeImpactCreate(w->frame->pos[0] + ox, w->frame->pos[1], w->frame->pos[2] + oz, 0);
                                SoundLocation2(0x27E, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                                w->step++;
                            }
                            break;
                        case 4:
                            if (bossStepAnim(w->frame, 0.2f, 2)) {
                                BODY_ANIM(w, bdevFakeImpactStep4Anim, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 5:
                            if (bossStepAnim(w->frame, 0.2f, 2)) {
                                BODY_ANIM(w, bdevFakeImpactStep5Anim, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 6:
                            w->frame->rot[1] -= 0.03490658600776016;
                            w->cape->rot[1] -= DEG;
                            if (w->timer == 18) {
                                SoundLocation2(0x133, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                            }
                            if (bossStepAnim(w->frame, 0.2f, 2)) {
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 7:
                            if (w->timer > 30) {
                                BODY_ANIM(w, bdevFakeImpactStep7Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 8:
                            w->frame->rot[1] += 0.06981317201552031;
                            w->cape->rot[1] += 0.06981317201552031;
                            if (w->timer == 45) {
                                SoundLocation2(0x133, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                            }
                            if (w->timer == 45) {
                                effectKiranCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 500.0f);
                                beFlashFCreate(0xFF, 0xFF, 0xFF, 20);
                                bossHeadingOffset(gBossYaw, -1400.0f, 800.0f, &ox, &oz);
                                impactBulletCreate(w->frame->pos[0] + ox, w->frame->pos[1], w->frame->pos[2] + oz,
                                                       0);
                            }
                            if (bossStepAnim(w->frame, 0.4f, 2)) {
                                BODY_ANIM(w, bdevBodyIdleAnim, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 9:
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 1.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 1.0f);
                            bossStepAnim(w->frame, 0.1f, 0);
                            bossStepAnim(w->cape, 0.1f, 0);
                            bossBobVel(obj, 4.0f, 2.0f);
                            if (w->timer > 120) {
                                TRAIL_SHOW(w);
                                gBrotherFlags &= ~0x40000;
                                gBrotherFlags |= 0x80000;
                                bossClearAction(obj);
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 7:
                    /* The barrier: from the middle of the arena, three "Pentagram" walls the
                     * Impact can't hurt through (gBossFlags 0x4000), then back. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d]\n", w->action, w->step, w->timer);
                    MOVE(w);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    bossStepAnim(w->cape, 0.3f, 0);
                    switch (w->step) {
                        case 0:
                            bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);
                            bossStartActionKind(obj, 2, 7, 1);
                            break;
                        case 1:
                            bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->ghost[0], 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->ghost[1], 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->ghost[2], 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->ghost[3], 0.0f, 0.0f, 3.0f);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                effectKiranCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 500.0f);
                                beFlashFCreate(0xFF, 0xFF, 0xFF, 20);
                                soundActpanCreate(obj, 0x176, gBattleCamView, 500.0f, 11000.0f, gBattleCamView->fovy);
                                w->step++;
                            }
                            break;
                        case 2:
                            bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 4.0f);
                            bossTurnToward2(w->ghost[0], 0.0f, 0.0f, 4.0f);
                            bossTurnToward2(w->ghost[1], 0.0f, 0.0f, 4.0f);
                            bossTurnToward2(w->ghost[2], 0.0f, 0.0f, 4.0f);
                            bossTurnToward2(w->ghost[3], 0.0f, 0.0f, 4.0f);
                            if (bossTurnToward2(w->frame, 0.0f, 0.0f, 2.0f)) {
                                BODY_ANIM(w, bdevBarrierStep2Anim, 0.0f, 0);
                                bossSetAnimAt(w->ghost[0], bdevBarrierStep2Anim, 0xF0, 0.0f);
                                bossSetAnimAt(w->ghost[2], bdevBarrierStep2Anim, 0xF0, 0.0f);
                                w->step++;
                            }
                            break;
                        case 3:
                            gBossFlags |= 4;
                            gBossFlags |= 0x4000;
                            gBossFlags |= 0x800;
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 1.0f);
                            bossTurnToward2(w->ghost[0], 0.0f, 0.0f, 4.0f);
                            bossTurnToward2(w->ghost[1], 0.0f, 0.0f, 4.0f);
                            bossTurnToward2(w->ghost[2], 0.0f, 0.0f, 4.0f);
                            bossTurnToward2(w->ghost[3], 0.0f, 0.0f, 4.0f);
                            bossStepAnim(w->ghost[0], 0.5f, 2);
                            bossStepAnim(w->ghost[2], 0.5f, 2);
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bdevBarrierStep3Anim, 0.0f, 0);
                                bossSetAnimAt(w->ghost[0], bdevBarrierStep3Anim, 0xF0, 0.33f);
                                bossSetAnimAt(w->ghost[2], bdevBarrierStep3Anim, 0xF0, 0.66f);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 4:
                            w->frame->rot[1] = mathAtan2(-w->frame->pos[0], -w->frame->pos[2]);
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.6f, 0);
                            bossStepAnim(w->cape, 0.6f, 0);
                            bossStepAnim(w->ghost[0], 0.6f, 0);
                            bossStepAnim(w->ghost[2], 0.6f, 0);
                            if (w->timer > 180) {
                                w->step++;
                            }
                            break;
                        case 5:
                            w->frame->rot[1] = mathAtan2(-w->frame->pos[0], -w->frame->pos[2]);
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.6f, 0);
                            bossStepAnim(w->cape, 0.6f, 0);
                            bossStepAnim(w->ghost[0], 0.6f, 0);
                            bossStepAnim(w->ghost[2], 0.6f, 0);
                            if (bossFlyToward(obj, 0.0f, -400.0f, -1400.0f, 0.5f)) {
                                w->step++;
                            }
                            break;
                        case 6:
                            w->frame->rot[1] = mathAtan2(-w->frame->pos[0], -w->frame->pos[2]);
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.6f, 0);
                            bossStepAnim(w->cape, 0.6f, 0);
                            bossStepAnim(w->ghost[0], 0.6f, 0);
                            bossStepAnim(w->ghost[2], 0.6f, 0);
                            if (w->frame->pos[2] > -1400.0f) {
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 7:
                            w->frame->rot[1] = mathAtan2(-w->frame->pos[0], -w->frame->pos[2]);
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.2f, 0);
                            bossStepAnim(w->cape, 0.2f, 0);
                            bossStepAnim(w->ghost[0], 0.2f, 0);
                            bossStepAnim(w->ghost[2], 0.2f, 0);
                            bossDampVel(obj, 0.8f);
                            if (w->timer % 10 == 0) {
                                beNullattackCreate(0.0f, 0.0f, 0.0f, 0x70);
                            }
                            if (w->timer > 120) {
                                w->step++;
                            }
                            break;
                        case 8:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossStepAnim(w->ghost[0], 0.6f, 0);
                            bossStepAnim(w->ghost[2], 0.6f, 0);
                            w->ghost[0]->rot[1] = 0.0f;
                            w->ghost[1]->rot[1] = 0.0f;
                            w->ghost[2]->rot[1] = 0.0f;
                            w->ghost[3]->rot[1] = 0.0f;
                            if (bossFlyToward(obj, 0.0f, -300.0f, -4000.0f, 1.0f)) {
                                BODY_ANIM(w, bdevBarrierStep2Anim, 1.0f, 0);
                                bossSetAnimAt(w->ghost[0], bdevBarrierStep2Anim, 0xF0, 0.9f);
                                bossSetAnimAt(w->ghost[2], bdevBarrierStep2Anim, 0xF0, 0.8f);
                                w->step++;
                            }
                            break;
                        case 9:
                            gBossFlags |= 4;
                            bossDampVel(obj, 0.9f);
                            w->ghost[0]->rot[1] = 0.0f;
                            w->ghost[1]->rot[1] = 0.0f;
                            w->ghost[2]->rot[1] = 0.0f;
                            w->ghost[3]->rot[1] = 0.0f;
                            bossStepAnim(w->ghost[0], 0.2f, 3);
                            bossStepAnim(w->ghost[2], 0.2f, 3);
                            if (bossStepAnim(w->frame, 0.2f, 3)) {
                                BODY_ANIM(w, bdevBodyIdleAnim, 0.0f, 0);
                                bossSetAnimAt(w->cape, bdevCapeIdleAnim, 0xF0, 0.0f);
                                bossSetAnimAt(w->ghost[0], bdevBodyIdleAnim, 0xF0, 0.1f);
                                bossSetAnimAt(w->ghost[2], bdevBodyIdleAnim, 0xF0, 0.3f);
                                gBossFlags &= ~0x4000;
                                gBossFlags &= ~0x800;
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 10:
                            gBossFlags |= 4;
                            TRAIL_FADE();
                            bossStepAnim(w->frame, 0.1f, 0);
                            bossStepAnim(w->frame, 0.1f, 0);
                            bossStepAnim(w->ghost[0], 0.1f, 0);
                            bossStepAnim(w->ghost[2], 0.1f, 0);
                            if (w->timer > 60) {
                                sndPlay(0x8176);
                                bossClearAction(obj);
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 8:
                    /* Reset to the middle (gBattleFlags 0x4000000 turns it the other way). */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d]\n", w->action, w->step, w->timer);
                    MOVE(w);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    bossStepAnim(w->cape, 0.3f, 0);
                    gBossFlags |= 4;
                    TRAIL_FADE();
                    switch (w->step) {
                        case 0:
                            gBossFlags |= 4;
                            w->atk->flags |= 2;
                            w->dmg->flags |= 2;
                            sndPlay(0x8176);
                            BODY_ANIM(w, bdevBodyIdleAnim, 0.0f, 0);
                            bossSetAnimAt(w->cape, bdevCapeIdleAnim, 0xF0, 0.0f);
                            bossSetAnimAt(w->ghost[0], bdevBodyIdleAnim, 0xF0, 0.0f);
                            bossSetAnimAt(w->ghost[1], bdevCapeIdleAnim, 0xF0, 0.0f);
                            bossSetAnimAt(w->ghost[2], bdevBodyIdleAnim, 0xF0, 0.0f);
                            bossSetAnimAt(w->ghost[3], bdevCapeIdleAnim, 0xF0, 0.0f);
                            w->cape->f.flags |= 2;
                            w->frame->pos[0] = 0.0f;
                            w->frame->pos[1] = -300.0f;
                            w->frame->pos[2] = -4000.0f;
                            w->frame->rot[0] = w->frame->rot[1] = w->frame->rot[2] = 0.0f;
                            w->vel[0] = 0.0f;
                            w->vel[1] = 0.0f;
                            w->vel[2] = 0.0f;
                            if (gBattleFlags & 0x4000000) {
                                w->frame->rot[1] = -2.0943952f;
                                w->cape->rot[1] = -2.0943952f;
                            } else {
                                w->frame->rot[1] = 2.0943952f;
                                w->cape->rot[1] = 2.0943952f;
                            }
                            w->frame->rot[2] = 0.0f;
                            gBossFlags &= ~0x4000;
                            gBossFlags &= ~0x800;
                            w->timer = 60;
                            w->step++;
                            break;
                        case 1:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossStepAnim(w->cape, 0.3f, 0);
                            bossBobVel(obj, 4.0f, 2.0f);
                            if (gBattleFlags & 0x4000000) {
                                w->frame->rot[1] = -2.0943952f;
                                w->cape->rot[1] = -2.0943952f;
                            } else {
                                w->frame->rot[1] = 2.0943952f;
                                w->cape->rot[1] = 2.0943952f;
                            }
                            if (w->timer-- == 0) {
                                w->step++;
                            }
                            break;
                        case 2:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossStepAnim(w->cape, 0.3f, 0);
                            bossBobVel(obj, 4.0f, 2.0f);
                            if (gBattleFlags & 0x4000000) {
                                w->frame->rot[1] += DEG;
                                w->cape->rot[1] += DEG;
                                if (w->frame->rot[1] > 0.0f) {
                                    w->frame->rot[1] = 0.0f;
                                    END_ACTION(w, obj);
                                }
                            } else {
                                w->frame->rot[1] -= DEG;
                                w->cape->rot[1] -= DEG;
                                if (w->frame->rot[1] < 0.0f) {
                                    w->frame->rot[1] = 0.0f;
                                    END_ACTION(w, obj);
                                }
                            }
                            break;
                    }
                    break;
                case 9:
                    /* Knocked out of an attack (gBattleFlags 8): wait for it to clear. */
                    MOVE(w);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    bossStepAnim(w->cape, 0.3f, 0);
                    gBossFlags |= 4;
                    TRAIL_FADE();
                    switch (w->step) {
                        case 0:
                            w->dmg->flags &= ~2;
                            BODY_ANIM(w, bdevBodyIdleAnim, 0.0f, 0);
                            bossSetAnimAt(w->cape, bdevCapeIdleAnim, 0xF0, 0.0f);
                            bossSetAnimAt(w->ghost[0], bdevBodyIdleAnim, 0xF0, 0.0f);
                            bossSetAnimAt(w->ghost[1], bdevCapeIdleAnim, 0xF0, 0.0f);
                            bossSetAnimAt(w->ghost[2], bdevBodyIdleAnim, 0xF0, 0.0f);
                            bossSetAnimAt(w->ghost[3], bdevCapeIdleAnim, 0xF0, 0.0f);
                            gBossFlags &= ~0x4000;
                            gBossFlags &= ~0x800;
                            w->step++;
                            break;
                        case 1:
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossStepAnim(w->cape, 0.3f, 0);
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 2.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 2.0f);
                            bossDampVel(obj, 0.8f);
                            if (!(gBattleFlags & 8)) {
                                TRAIL_SHOW(w);
                                w->dmg->flags |= 2;
                                bossClearAction(obj);
                            }
                            break;
                    }
                    break;
                case 10:
                    /* Bismaru's ghost ship ("BisuG"): call it from the back of the arena. The third
                     * time sends "Kensuke". With Bismaru already out, the boss gives up. */
                    MOVE(w);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);
                    bossStepAnim(w->cape, 0.3f, 0);
                    switch (w->step) {
                        case 0:
                            TRAIL_FADE();
                            if (gBossFlags & 0x20000) {
                                DPRINTF("すでにビスマルがいるのでやる気がなくなりました。\n");
                                bossClearAction(obj);
                            } else if (bossFlyToward(obj, 0.0f, 200.0f, -5000.0f, 3.0f)) {
                                w->step++;
                            }
                            break;
                        case 1:
                            TRAIL_FADE();
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->ghost[0], 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->ghost[1], 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->ghost[2], 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->ghost[3], 0.0f, 0.0f, 3.0f);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                gBossFlags |= 0x4000;
                                gBossFlags |= 0x800;
                                gBrotherFlags |= 0x20000;
                                gBossFlags |= 4;
                                w->step++;
                            }
                            break;
                        case 2:
                            gBossFlags |= 4;
                            gBrotherFlags |= 0x20000;
                            TRAIL_FADE();
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->ghost[0], 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->ghost[1], 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->ghost[2], 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->ghost[3], 0.0f, 0.0f, 3.0f);
                            if (bossTurnToward2(w->frame, 0.0f, 0.0f, 3.0f)) {
                                w->step++;
                            }
                            break;
                        case 3:
                            gBossFlags |= 4;
                            gBrotherFlags |= 0x20000;
                            gBossFlags |= 0x4000;
                            gBossFlags |= 0x800;
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 2.0f);
                            bossTurnToward2(w->ghost[0], 0.0f, 0.0f, 2.0f);
                            bossTurnToward2(w->ghost[1], 0.0f, 0.0f, 2.0f);
                            bossTurnToward2(w->ghost[2], 0.0f, 0.0f, 2.0f);
                            bossTurnToward2(w->ghost[3], 0.0f, 0.0f, 2.0f);
                            bossStepAnim(w->ghost[0], 0.1f, 2);
                            bossStepAnim(w->ghost[2], 0.1f, 2);
                            if (bossStepAnim(w->frame, 0.2f, 2)) {
                                BODY_ANIM(w, bdevGhostShipStep3Anim, 0.0f, 0);
                                bossSetAnimAt(w->ghost[0], bdevGhostShipStep3Anim, 0xF0, 0.1f);
                                bossSetAnimAt(w->ghost[2], bdevGhostShipStep3Anim, 0xF0, 0.2f);
                                pentaniCreate(0.0f, -500.0f, -3000.0f, 0);
                                sndPlay(0x180);
                                if (joyGetHeldTime(gCockpitPort, gCockpitBtnL)) {
                                    bdevBisuG2Create(0.0f, -500.0f, -3000.0f);
                                } else if (!(gBossFlags & 0x100000) && !(gBossFlags & 0x200000)) {
                                    DPRINTF("幽霊１回目です。\n");
                                    gBossFlags |= 0x100000;
                                    bdevBisuGCreate(0.0f, -500.0f, -3000.0f);
                                } else if ((gBossFlags & 0x100000) && !(gBossFlags & 0x200000)) {
                                    DPRINTF("幽霊２回目です。\n");
                                    gBossFlags |= 0x200000;
                                    bdevBisuGCreate(0.0f, -500.0f, -3000.0f);
                                } else if ((gBossFlags & 0x100000) && (gBossFlags & 0x200000)) {
                                    DPRINTF("幽霊３回目です。ケンスケ発進\n");
                                    gBossFlags &= ~0x100000;
                                    gBossFlags &= ~0x200000;
                                    bdevBisuG2Create(0.0f, -500.0f, -3000.0f);
                                }
                                w->step++;
                            }
                            break;
                        case 4:
                            gBossFlags |= 4;
                            gBrotherFlags |= 0x20000;
                            bossStepAnim(w->ghost[0], 0.1f, 2);
                            bossStepAnim(w->ghost[2], 0.1f, 2);
                            if (bossStepAnim(w->frame, 0.1f, 2)) {
                                BODY_ANIM(w, bdevGhostShipStep4Anim, 0.0f, 0);
                                bossSetAnimAt(w->ghost[0], bdevGhostShipStep4Anim, 0xF0, 0.33f);
                                bossSetAnimAt(w->ghost[2], bdevGhostShipStep4Anim, 0xF0, 0.66f);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 5:
                            /* Until the ship is gone. */
                            gBossFlags |= 4;
                            gBrotherFlags |= 0x20000;
                            bossStepAnim(w->frame, 0.5f, 0);
                            bossStepAnim(w->cape, 0.5f, 0);
                            bossStepAnim(w->ghost[0], 0.5f, 0);
                            bossStepAnim(w->ghost[2], 0.5f, 0);
                            if (!(gBossFlags & 0x20000)) {
                                w->ghost[0]->rot[1] = 0.0f;
                                w->ghost[1]->rot[1] = 0.0f;
                                w->ghost[2]->rot[1] = 0.0f;
                                w->ghost[3]->rot[1] = 0.0f;
                                w->step++;
                            }
                            break;
                        case 6:
                            gBossFlags |= 4;
                            gBrotherFlags |= 0x20000;
                            bossStepAnim(w->ghost[0], 0.1f, 2);
                            bossStepAnim(w->ghost[2], 0.1f, 2);
                            if (bossStepAnim(w->frame, 0.1f, 2)) {
                                BODY_ANIM(w, bdevGhostShipStep3Anim, 1.0f, 0);
                                bossSetAnimAt(w->ghost[0], bdevGhostShipStep3Anim, 0xF0, 0.9f);
                                bossSetAnimAt(w->ghost[2], bdevGhostShipStep3Anim, 0xF0, 0.8f);
                                w->step++;
                            }
                            break;
                        case 7:
                            gBossFlags |= 4;
                            gBrotherFlags |= 0x20000;
                            bossStepAnim(w->ghost[0], 0.4f, 3);
                            bossStepAnim(w->ghost[2], 0.4f, 3);
                            if (bossStepAnim(w->frame, 0.4f, 3)) {
                                BODY_ANIM(w, bdevBodyIdleAnim, 0.0f, 0);
                                bossSetAnimAt(w->cape, bdevCapeIdleAnim, 0xF0, 0.0f);
                                bossSetAnimAt(w->ghost[0], bdevBodyIdleAnim, 0xF0, 0.1f);
                                bossSetAnimAt(w->ghost[2], bdevBodyIdleAnim, 0xF0, 0.3f);
                                gBossFlags &= ~0x4000;
                                gBossFlags &= ~0x800;
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 8:
                            TRAIL_FADE();
                            gBrotherFlags |= 0x20000;
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.1f, 0);
                            bossStepAnim(w->frame, 0.1f, 0);
                            bossStepAnim(w->ghost[0], 0.1f, 0);
                            bossStepAnim(w->ghost[2], 0.1f, 0);
                            if (w->timer > 30) {
                                gBrotherFlags &= ~0x20000;
                                bossClearAction(obj);
                            }
                            break;
                        case 10:
                            break;
                    }
                    w->timer++;
                    break;
                case 11:
                    /* Dizzy after three flinches (bossFlinchCountOver): a slash, then back. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d]\n", w->action, w->step, w->timer);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    MOVE(w);
                    TURN(w);
                    gBossFlags |= 4;
                    gBossFlags |= 0x1000;
                    TRAIL_FADE();
                    switch (w->step) {
                        case 0:
                            gBossFlags |= 4;
                            BODY_ANIM(w, bdevSlashRaiseAnim, 0.0f, 1);
                            bossSetAnimAt(w->cape, bdevCapeFoldedAnim, 0xF0, 0.0f);
                            w->step++;
                            break;
                        case 1:
                            gBossFlags |= 4;
                            bossFlyToward(obj, 0.0f, -200.0f, -3000.0f, 2.0f);
                            bossStepAnim(w->cape, 0.5f, 2);
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bdevSlashSwingAnim, 0.0f, 1);
                                w->step++;
                            }
                            break;
                        case 2:
                            gBossFlags |= 4;
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bdevBodyIdleAnim, 0.0f, 0);
                                bossSetAnimAt(w->cape, bdevCapeIdleAnim, 0xF0, 0.0f);
                                bossClearAction(obj);
                                gBossFlags &= ~0x1000;
                            }
                            break;
                        case 3:
                            break;
                        case 4:
                            break;
                    }
                    w->timer++;
                    break;
                case 12:
                    /* The entrance demo ("登場デモ"), with its camera ("Bappcam"). Start skips it
                     * (steps 10-12), and so does a lost battle before. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d]\n", w->action, w->step, w->timer);
                    MOVE(w);
                    gBattleFlags2 |= 0x200000;
                    if (w->step != 7 && w->step != 8 && w->step != 9 && w->step != 10 && w->step != 11 &&
                        w->step != 12) {
                        if (gGameWork.losses != 0) {
                            gBattleFlags2 |= 0x80000;
                            gBossFlags &= ~0x10000;
                            for (i = 0; i < 4; i++) {
                                contPakInitRumble(i);
                                contPakRumbleRequestStop(i);
                            }
                            w->step = 10;
                        }
                        if (fadeCountPlayers() == 1) {
                            if (joyGetHeldTime(gCockpitPort, gCockpitBtnStart) == 1) {
                                DPRINTF("[bisu]登場デモをキャンセルします\n");
                                gBattleFlags2 |= 0x80000;
                                gBossFlags &= ~0x10000;
                                for (i = 0; i < 4; i++) {
                                    contPakInitRumble(i);
                                    contPakRumbleRequestStop(i);
                                }
                                w->step = 10;
                            }
                        } else if (fadeCountPlayers() >= 2 && (joyGetHeldTime(gCockpitPort, gCockpitBtnStart) == 1 ||
                                                               joyGetHeldTime(gBrotherPort, gBrotherBtnStart) == 1)) {
                            DPRINTF("[bisu]登場デモをキャンセルします\n");
                            gBattleFlags2 |= 0x80000;
                            gBossFlags &= ~0x10000;
                            for (i = 0; i < 4; i++) {
                                contPakInitRumble(i);
                                contPakRumbleRequestStop(i);
                            }
                            w->step = 10;
                        }
                    }
                    switch (w->step) {
                        case 0:
                            gBattleFlags2 |= 0x400;
                            gBossFlags |= 0x10000;
                            gBattleFlags2 |= 0x100000;
                            gBattleFlags2 |= 0x200;
                            w->ghost[0]->f.flags &= ~2;
                            w->ghost[1]->f.flags &= ~2;
                            w->ghost[2]->f.flags &= ~2;
                            w->ghost[3]->f.flags &= ~2;
                            TRAIL_FADE();
                            tskCreate(obj, bdevBappcamProc, 0x147, NULL);
                            w->frame->pos[0] = 0.0f;
                            w->frame->pos[1] = -2800.0f;
                            w->frame->pos[2] = -3000.0f;
                            w->vel[0] = 0.0f;
                            w->vel[1] = 0.0f;
                            w->vel[2] = 0.0f;
                            w->frame->rot[0] = 0.0f;
                            w->frame->rot[1] = 0.0f;
                            w->frame->rot[2] = 0.0f;
                            w->cape->rot[0] = 0.0f;
                            w->cape->rot[1] = 0.0f;
                            w->cape->rot[2] = 0.0f;
                            BODY_ANIM(w, bdevEntranceDemoAnim, 0.0f, 0);
                            bossSetAnimAt(w->cape, bdevCapeIdleAnim, 0xF0, 0.0f);
                            gBattleFlags |= 0x11000000;
                            gBrotherFlags |= 0x10000;
                            sndPlay(1);
                            sndPlay(0x7D);
                            w->timer = 0;
                            w->step++;
                            break;
                        case 1:
                            TRAIL_SHOW(w);
                            GHOSTS_ON_BODY(w);
                            if (w->timer > 120) {
                                bdevCuBalDCreate(0.0f, 1000.0f, -3000.0f, 0);
                                w->step++;
                            }
                            break;
                        case 2:
                            if (w->timer > 360) {
                                pentaniDemoCreate(0.0f, -200.0f, -3000.0f, 0);
                                tskCreate(obj, beBspaceProc, 0x147, (void *) 1);
                                bossRumble(2, 0xFFFF8888, 4);
                                w->step++;
                            }
                            break;
                        case 3:
                            if (w->timer % 60 == 0) {
                                bossRumble(2, 0xBBBBBBBB, 8);
                            }
                            if (w->timer > 530) {
                                w->step++;
                            }
                            break;
                        case 4:
                            /* Rising out of the floor. */
                            if (w->timer % 50 == 0) {
                                bossRumble(2, 0xBBBBBBBB, 8);
                            }
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossStepAnim(w->cape, 0.3f, 0);
                            if (w->timer == 552) {
                                sndPlay(0x1A3);
                            }
                            if (w->timer == 893) {
                                sndPlay(0x1A4);
                            }
                            if (w->timer >= 1150) {
                                gBossFlags &= ~0x10000;
                            }
                            if (w->timer == 1150) {
                                sndPlay(0x1A5);
                            }
                            w->frame->pos[1] += 4.0;
                            if (w->frame->pos[1] > 800.0f) {
                                w->frame->pos[1] = 800.0f;
                                BODY_ANIM(w, bdevBarrierStep2Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 5:
                            if (w->timer == 1281) {
                                sndPlay(0x215);
                            }
                            if (w->timer == 1311) {
                                sndPlay(0x215);
                            }
                            if (w->timer == 1349) {
                                sndPlay(0x215);
                            }
                            if (w->timer % 40 == 0) {
                                bossRumble(2, 0xBBBBBBBB, 8);
                            }
                            bossStepAnim(w->cape, 0.3f, 0);
                            if (bossStepAnim(w->frame, 0.2f, 2)) {
                                BODY_ANIM(w, bdevBarrierStep3Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 6:
                            if (w->timer == 1380) {
                                sndPlay(0x215);
                            }
                            if (w->timer % 30 == 0) {
                                bossRumble(2, 0xBBBBBBBB, 8);
                            }
                            bossStepAnim(w->cape, 0.3f, 0);
                            if (bossStepAnim(w->frame, 0.2f, 2)) {
                                BODY_ANIM(w, bdevBarrierStep2Anim, 1.0f, 0);
                                w->step++;
                            }
                            break;
                        case 7:
                            if (w->timer == 1426) {
                                sndPlay(0x215);
                            }
                            if (w->timer == 1458) {
                                sndPlay(0x215);
                            }
                            if (w->timer % 30 == 0) {
                                bossRumble(2, 0xBBBBBBBB, 8);
                            }
                            bossStepAnim(w->cape, 0.3f, 0);
                            if (bossStepAnim(w->frame, 0.2f, 3)) {
                                effectKiranCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 500.0f);
                                BODY_ANIM(w, bdevBodyIdleAnim, 0.0f, 0);
                                bossSetAnimAt(w->cape, bdevCapeIdleAnim, 0xF0, 0.0f);
                                w->step++;
                            }
                            break;
                        case 8:
                            TRAIL_SHOW(w);
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossStepAnim(w->cape, 0.3f, 0);
                            if (w->timer == 1580) {
                                w->frame->pos[0] = 0.0f;
                                w->frame->pos[1] = -200.0f;
                                w->frame->pos[2] = -3500.0f;
                                w->frame->rot[0] = w->frame->rot[1] = w->frame->rot[2] = 0.0f;
                                w->vel[0] = w->vel[1] = w->vel[2] = 0.0f;
                            }
                            if (w->timer > 1700) {
                                DPRINTF("登場デモ終了\n");
                                gBattleFlags2 &= ~0x400;
                                gBattleFlags2 |= 8;
                                gBattleFlags2 &= ~0x100000;
                                gBossFlags &= ~0x10000;
                                BODY_ANIM(w, bdevBodyIdleAnim, 0.0f, 0);
                                bossSetAnimAt(w->cape, bdevCapeIdleAnim, 0xF0, 0.0f);
                                effectKiranCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 500.0f);
                                w->timer = 0;
                                w->step = 11;
                            }
                            break;
                        case 9:
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossStepAnim(w->cape, 0.3f, 0);
                            if (w->timer > 30) {
                                bossClearAction(obj);
                            }
                            break;
                        case 10:
                            /* Skipped: close the letterbox over the demo. */
                            fadeCreate(gEffectTask);
                            fadeSetParent((s32) gFrmLayers[10]);
                            fadeSetLetterbox(320, 120, 0, 120);
                            sndPlay(10);
                            sndPlay(0x7E);
                            w->step++;
                            break;
                        case 11:
                            BODY_ANIM(w, bdevBodyIdleAnim, 0.0f, 0);
                            bossSetAnimAt(w->cape, bdevCapeIdleAnim, 0xF0, 0.0f);
                            w->frame->pos[0] = 0.0f;
                            w->frame->pos[1] = -200.0f;
                            w->frame->pos[2] = -3500.0f;
                            w->frame->rot[0] = w->frame->rot[1] = w->frame->rot[2] = 0.0f;
                            w->vel[0] = 0.0f;
                            w->vel[1] = 0.0f;
                            w->vel[2] = 0.0f;
                            gBattleFlags &= ~0x10000000;
                            gBattleFlags &= ~0x1000000;
                            gBattleFlags2 &= ~0x100;
                            gBrotherFlags &= ~0x10000;
                            gBattleFlags2 |= 0x8000;
                            gBattleFlags2 |= 8;
                            gBattleFlags2 &= ~0x100000;
                            gBattleFlags2 &= ~0x200;
                            w->ghost[0]->f.flags |= 2;
                            w->ghost[1]->f.flags |= 2;
                            w->ghost[2]->f.flags |= 2;
                            w->ghost[3]->f.flags |= 2;
                            w->step++;
                            break;
                        case 12:
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossStepAnim(w->cape, 0.3f, 0);
                            if (fadeMoveLetterbox(0, 240, 1) == 1) {
                                DPRINTF("登場デモキャンセル終了\n");
                                gBattleFlags2 &= ~0x400;
                                gBattleFlags2 |= 8;
                                gBattleFlags2 &= ~0x200000;
                                effectKiranCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 500.0f);
                                bossClearAction(obj);
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 13:
                    /* The slash with the cape: from the right side, toward the Impact. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d]\n", w->action, w->step, w->timer);
                    MOVE(w);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    TURN(w);
                    switch (w->step) {
                        case 0:
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossStepAnim(w->cape, 0.3f, 0);
                            if (bossFlyToward(obj, w->frame->pos[0], 0.0f, -5000.0f, 2.0f)) {
                                w->step++;
                            }
                            break;
                        case 1:
                            TRAIL_FADE();
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossStepAnim(w->cape, 0.3f, 0);
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 4.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 4.0f);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                w->step++;
                            }
                            break;
                        case 2:
                            TRAIL_FADE();
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossStepAnim(w->cape, 0.3f, 0);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 4.0f);
                            if (bossTurnToward2(w->frame, 0.0f, 0.0f, 4.0f)) {
                                BODY_ANIM(w, bdevSlashRaiseAnim, 0.0f, 0);
                                bossSetAnimAt(w->cape, bdevCapeAttackAnim, 0xF0, 0.0f);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 3:
                            gBossFlags |= 4;
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 4.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 4.0f);
                            bossFlyToward(obj, 100.0f, -400.0f, -900.0f, 3.0f);
                            bossStepAnim(w->frame, 0.4f, 2);
                            bossStepAnim(w->cape, 0.4f, 0);
                            if (w->timer > 30) {
                                w->step++;
                            }
                            break;
                        case 4:
                            gBossFlags |= 4;
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 4.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 4.0f);
                            bossStepAnim(w->cape, 0.4f, 0);
                            if (bossFlyToward(obj, 100.0f, -500.0f, -1100.0f, 3.0f) && w->frame->pos[2] > -1100.0f) {
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 5:
                            gBossFlags |= 4;
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 4.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 4.0f);
                            if (w->timer == 15) {
                                SoundLocation2(0x214, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                                kesaEffectCreate(0.0f, 0.0f, -400.0f, 0);
                                beFlashFCreate(0xFF, 0, 0, 20);
                            }
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bdevSlashSwingAnim, 0.0f, 0);
                                w->vel[2] = -w->vel[2];
                                w->step++;
                            }
                            break;
                        case 6:
                            gBossFlags |= 4;
                            bossFlyToward(obj, 0.0f, -200.0f, -3000.0f, 2.0f);
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bdevBodyIdleAnim, 0.0f, 0);
                                bossSetAnimAt(w->cape, bdevCapeIdleAnim, 0xF0, 0.0f);
                                bossClearAction(obj);
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 15:
                    /* Defeated: the boss's losing demo, then the battle is won. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d]\n", w->action, w->step, w->timer);
                    MOVE(w);
                    gBossPosX = w->frame->pos[0];
                    gBossPosY = w->frame->pos[1];
                    gBossPosZ = w->frame->pos[2];
                    gBossYaw = w->frame->rot[1];
                    w->frame->rot[1] = mathAtan2(-w->frame->pos[0], -w->frame->pos[2]);
                    gBattleFlags2 |= 0x200000;
                    w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 1.6f;
                    w->cape->f.flags |= 2;
                    DPRINTF("\rDemo step/timer[%d/%d]", w->step, w->timer);
                    switch (w->step) {
                        case 0:
                            fadeCreate(gEffectTask);
                            gBattleFlags2 |= 0x400;
                            gBossFlags |= 0x800;
                            gBattleFlags2 |= 0x1000;
                            gBossFlags &= ~0x10000;
                            sndPlay(9);
                            w->vel[0] = w->vel[1] = w->vel[2] = 0.0f;
                            tskCreate(obj, Bappcam3, 0x147, NULL);
                            w->frame->pos[0] = 2000.0f;
                            gBossPosX = w->frame->pos[0];
                            w->frame->pos[1] = 0.0f;
                            gBossPosY = w->frame->pos[1];
                            w->frame->pos[2] = -1000.0f;
                            gBossPosZ = w->frame->pos[2];
                            BODY_ANIM(w, bdevEntranceDemoAnim, 0.0f, 1);
                            bossSetAnimAt(w->cape, bdevCapeFoldedAnim, 0xF0, 0.0f);
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
                            if (gGameFrameCount % 10 >= 5) {
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
                            if (w->timer % 12 == 0) {
                                sndPlay(0x513);
                            }
                            if (!(w->timer & 7)) {
                                bossRumble(2, 0x88888888, 8);
                            }
                            bossStepAnim(w->frame, 0.2f, 0);
                            if (gGameFrameCount % 10 >= 5) {
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
                            if (w->timer % 12 == 0) {
                                sndPlay(0x513);
                            }
                            if (!(w->timer & 7)) {
                                bossRumble(2, 0x88888888, 8);
                            }
                            w->frame->rot[2] -= DEG;
                            if (w->frame->rot[2] < 0.22689280908185694) {
                                w->frame->rot[2] = 0.22689280908185694;
                            }
                            if (gGameFrameCount % 10 >= 5) {
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
                                sndPlay(0x513);
                            }
                            if (w->timer % 8 == 0) {
                                bossRumble(2, 0x88888888, 8);
                            }
                            bossStepAnim(w->frame, 0.2f, 0);
                            if (gGameFrameCount % 10 >= 5) {
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
                                sndPlay(0x513);
                            }
                            if (w->timer % 8 == 0) {
                                bossRumble(2, 0x88888888, 8);
                            }
                            bossStepAnim(w->frame, 0.2f, 0);
                            if (gGameFrameCount % 10 >= 5) {
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
                                sndPlay(0x513);
                            }
                            if (w->timer % 8 == 0) {
                                bossRumble(2, 0xA5294A52, 8);
                            }
                            bossStepAnim(w->frame, 0.2f, 0);
                            if (gGameFrameCount % 10 >= 5) {
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
                            if (w->timer % 8 == 0 && w->timer < 960) {
                                sndPlay(0x513);
                            }
                            if (w->timer % 8 == 0) {
                                bossRumble(2, 0xA5294A52, 8);
                            }
                            bossStepAnim(w->frame, 0.2f, 0);
                            if (gGameFrameCount % 10 >= 5) {
                                w->frame->pos[1] += 6.0f;
                            } else {
                                w->frame->pos[1] -= 6.0f;
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
                                w->step = 13;
                            }
                            break;
                        case 13:
                            if (w->timer == 1100) {
                                fadeSetAll(30.0f, 0.0f, 255.0f, 0xFF, 0xFF, 0xFF);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 14:
                            if (w->timer == 30) {
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 15:
                            if (fadeCloseLetterbox(120)) {
                                bossBattleWin((u8) gBattleBossId);
                                stageGoto(gGameWork.gameProcTask, 0x8B);
                                gGameWork.flags &= ~1;
                                gGameWork.nextOverlay = 9;
                            }
                            break;
                        case 16:
                            break;
                    }
                    w->timer++;
                    break;
                case 16:
                    /* The Impact is down: the devil's victory demo, then the battle is lost. */
                    MOVE(w);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    TURN(w);
                    bossStepAnim(w->cape, 0.3f, 0);
                    TRAIL_FADE();
                    gBattleFlags2 |= 0x200000;
                    switch (w->step) {
                        case 0:
                            DPRINTF("デヴィル勝利デモ開始します\n");
                            gBattleFlags2 |= 0x400;
                            gBattleFlags2 |= 0x800;
                            gBattleFlags |= 0x10000000;
                            fadeCreate(gEffectTask);
                            BODY_ANIM(w, bdevBodyIdleAnim, 0.0f, 0);
                            bossSetAnimAt(w->cape, bdevCapeIdleAnim, 0xF0, 0.0f);
                            bossStepAnim(w->frame, 0.3f, 0);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                w->step++;
                            }
                            break;
                        case 1:
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 2.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 2.0f);
                            bossStepAnim(w->frame, 0.3f, 0);
                            if (bossFlyToward(obj, -200.0f, -300.0f, -1800.0f, 1.5f)) {
                                w->step++;
                            }
                            break;
                        case 2:
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 2.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 2.0f);
                            bossStepAnim(w->frame, 0.2f, 0);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                BODY_ANIM(w, bdevSlashRaiseAnim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 3:
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 2.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 2.0f);
                            if (bossStepAnim(w->frame, 0.2f, 2)) {
                                SoundLocation2(0x214, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                                kesaEffectCreate(0.0f, 0.0f, -400.0f, 0);
                                beFlashFCreate(0xFF, 0, 0, 20);
                                fadeSetAll(120.0f, 0.0f, 255.0f, 0xFF, 0, 0);
                                BODY_ANIM(w, bdevSlashSwingAnim, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 4:
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 2.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 2.0f);
                            bossStepAnim(w->frame, 0.4f, 2);
                            if (w->timer > 120) {
                                DPRINTF("インパクト負け。\n");
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
                    }
                    w->timer++;
                    break;
                case 17:
                    /* The curse balls ("CuBalR"), thrown from the cape. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d]\n", w->action, w->step, w->timer);
                    MOVE(w);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    TURN(w);
                    switch (w->step) {
                        case 0:
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossStepAnim(w->cape, 0.3f, 0);
                            if (bossFlyToward(obj, w->frame->pos[0], -200.0f, -5000.0f, 2.0f)) {
                                w->step++;
                            }
                            break;
                        case 1:
                            TRAIL_FADE();
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossStepAnim(w->cape, 0.3f, 0);
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 4.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 4.0f);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                w->step++;
                            }
                            break;
                        case 2:
                            TRAIL_FADE();
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossStepAnim(w->cape, 0.3f, 0);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 4.0f);
                            if (bossTurnToward2(w->frame, 0.0f, 0.0f, 4.0f)) {
                                BODY_ANIM(w, bdevSlashRaiseAnim, 0.0f, 0);
                                bossSetAnimAt(w->cape, bdevCapeAttackAnim, 0xF0, 0.0f);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 3:
                            TRAIL_FADE();
                            bossStepAnim(w->cape, 0.4f, 0);
                            if (w->timer == 45) {
                                SoundLocation2(0x214, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                            }
                            if (w->timer == 40) {
                                bossHeadingOffset(gBossYaw, -700.0f, 100.0f, &ox, &oz);
                                curseballRCreate(w->frame->pos[0] + ox, w->frame->pos[1] + 900.0f,
                                                       w->frame->pos[2] + oz, 0);
                            }
                            if (w->timer == 49) {
                                bossHeadingOffset(gBossYaw, -100.0f, 100.0f, &ox, &oz);
                                curseballRCreate(w->frame->pos[0] + ox, w->frame->pos[1], w->frame->pos[2] + oz,
                                                       3);
                            }
                            if (bossStepAnim(w->frame, 0.4f, 2)) {
                                bossSetAnimAt(w->cape, bdevCapeIdleAnim, 0xF0, 0.0f);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 4:
                            bossStepAnim(w->cape, 0.3f, 0);
                            if (w->timer > 15) {
                                BODY_ANIM(w, bdevSlashSwingAnim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 5:
                            bossStepAnim(w->cape, 0.4f, 0);
                            if (bossStepAnim(w->frame, 0.4f, 2)) {
                                BODY_ANIM(w, bdevBodyIdleAnim, 0.0f, 0);
                                bossSetAnimAt(w->cape, bdevCapeIdleAnim, 0xF0, 0.0f);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 6:
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossStepAnim(w->cape, 0.3f, 0);
                            if (w->timer > 60) {
                                END_ACTION(w, obj);
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 0x1F:
                    /* bossStartActionKind's moves: to one of nine places, then the action and step
                     * it was given. */
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    MOVE(w);
                    TURN(w);
                    bossStepAnim(w->frame, 0.3f, 0);
                    bossStepAnim(w->cape, 0.3f, 0);
                    bossStepAnim(w->ghost[0], 0.3f, 0);
                    bossStepAnim(w->ghost[1], 0.3f, 0);
                    bossStepAnim(w->ghost[2], 0.3f, 0);
                    bossStepAnim(w->ghost[3], 0.3f, 0);
                    if (w->step == 0 && bossFlyToward(obj, -3000.0f, -200.0f, -6000.0f, 1.5f)) {
                        w->action = w->nextAction;
                        w->step = w->nextStep;
                    }
                    break;
                case 0x20:
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    MOVE(w);
                    TURN(w);
                    bossStepAnim(w->frame, 0.3f, 0);
                    bossStepAnim(w->cape, 0.3f, 0);
                    bossStepAnim(w->ghost[0], 0.3f, 0);
                    bossStepAnim(w->ghost[1], 0.3f, 0);
                    bossStepAnim(w->ghost[2], 0.3f, 0);
                    bossStepAnim(w->ghost[3], 0.3f, 0);
                    if (w->step == 0 && bossFlyToward(obj, 0.0f, -200.0f, -6000.0f, 1.0f)) {
                        w->action = w->nextAction;
                        w->step = w->nextStep;
                    }
                    break;
                case 0x21:
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    MOVE(w);
                    TURN(w);
                    bossStepAnim(w->frame, 0.3f, 0);
                    bossStepAnim(w->cape, 0.3f, 0);
                    bossStepAnim(w->ghost[0], 0.3f, 0);
                    bossStepAnim(w->ghost[1], 0.3f, 0);
                    bossStepAnim(w->ghost[2], 0.3f, 0);
                    bossStepAnim(w->ghost[3], 0.3f, 0);
                    if (w->step == 0 && bossFlyToward(obj, 3000.0f, -200.0f, -6000.0f, 1.0f)) {
                        w->action = w->nextAction;
                        w->step = w->nextStep;
                    }
                    break;
                case 0x22:
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    MOVE(w);
                    TURN(w);
                    bossStepAnim(w->frame, 0.3f, 0);
                    bossStepAnim(w->cape, 0.3f, 0);
                    bossStepAnim(w->ghost[0], 0.3f, 0);
                    bossStepAnim(w->ghost[1], 0.3f, 0);
                    bossStepAnim(w->ghost[2], 0.3f, 0);
                    bossStepAnim(w->ghost[3], 0.3f, 0);
                    if (w->step == 0 && bossFlyToward(obj, -5000.0f, -200.0f, -3000.0f, 1.0f)) {
                        w->action = w->nextAction;
                        w->step = w->nextStep;
                    }
                    break;
                case 0x23:
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    MOVE(w);
                    TURN(w);
                    bossStepAnim(w->frame, 0.3f, 0);
                    bossStepAnim(w->cape, 0.3f, 0);
                    bossStepAnim(w->ghost[0], 0.3f, 0);
                    bossStepAnim(w->ghost[1], 0.3f, 0);
                    bossStepAnim(w->ghost[2], 0.3f, 0);
                    bossStepAnim(w->ghost[3], 0.3f, 0);
                    if (w->step == 0 && bossFlyToward(obj, 0.0f, -200.0f, -3000.0f, 1.0f)) {
                        w->action = w->nextAction;
                        w->step = w->nextStep;
                    }
                    break;
                case 0x24:
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    MOVE(w);
                    TURN(w);
                    bossStepAnim(w->frame, 0.3f, 0);
                    bossStepAnim(w->cape, 0.3f, 0);
                    bossStepAnim(w->ghost[0], 0.3f, 0);
                    bossStepAnim(w->ghost[1], 0.3f, 0);
                    bossStepAnim(w->ghost[2], 0.3f, 0);
                    bossStepAnim(w->ghost[3], 0.3f, 0);
                    if (w->step == 0 && bossFlyToward(obj, 5000.0f, -200.0f, -3000.0f, 1.0f)) {
                        w->action = w->nextAction;
                        w->step = w->nextStep;
                    }
                    break;
                case 0x25:
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    MOVE(w);
                    TURN(w);
                    bossStepAnim(w->frame, 0.3f, 0);
                    bossStepAnim(w->cape, 0.3f, 0);
                    bossStepAnim(w->ghost[0], 0.3f, 0);
                    bossStepAnim(w->ghost[1], 0.3f, 0);
                    bossStepAnim(w->ghost[2], 0.3f, 0);
                    bossStepAnim(w->ghost[3], 0.3f, 0);
                    if (w->step == 0 && bossFlyToward(obj, -3000.0f, -200.0f, -800.0f, 1.0f)) {
                        w->action = w->nextAction;
                        w->step = w->nextStep;
                    }
                    break;
                case 0x26:
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    MOVE(w);
                    TURN(w);
                    bossStepAnim(w->frame, 0.3f, 0);
                    bossStepAnim(w->cape, 0.3f, 0);
                    bossStepAnim(w->ghost[0], 0.3f, 0);
                    bossStepAnim(w->ghost[1], 0.3f, 0);
                    bossStepAnim(w->ghost[2], 0.3f, 0);
                    bossStepAnim(w->ghost[3], 0.3f, 0);
                    if (w->step == 0 && bossFlyToward(obj, 0.0f, -200.0f, -800.0f, 1.0f)) {
                        w->action = w->nextAction;
                        w->step = w->nextStep;
                    }
                    break;
                case 0x27:
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    MOVE(w);
                    TURN(w);
                    bossStepAnim(w->frame, 0.3f, 0);
                    bossStepAnim(w->cape, 0.3f, 0);
                    bossStepAnim(w->ghost[0], 0.3f, 0);
                    bossStepAnim(w->ghost[1], 0.3f, 0);
                    bossStepAnim(w->ghost[2], 0.3f, 0);
                    bossStepAnim(w->ghost[3], 0.3f, 0);
                    if (w->step == 0 && bossFlyToward(obj, 3000.0f, -200.0f, -800.0f, 1.0f)) {
                        w->action = w->nextAction;
                        w->step = w->nextStep;
                    }
                    break;
                case 0x2B:
                    /* Knocked flying (the velocity a hit set): thrown down, then up again; the
                     * losing demo when the life is gone. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d]\n", w->action, w->step, w->timer);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    MOVE(w);
                    TRAIL_FADE();
                    switch (w->step) {
                        case 0:
                            gBossFlags &= ~0x800;
                            BODY_ANIM(w, bdevThrownDownAnim, 0.0f, 1);
                            w->cape->f.flags &= ~2;
                            w->cape->rot[1] = 0.0f;
                            w->timer = 0;
                            w->step++;
                            break;
                        case 1:
                            w->vel[1] -= 1.0;
                            if (gGameFrameCount % 10 == 0) {
                                beSpeffectCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 0);
                            }
                            if (w->timer == 50) {
                                SoundLocation2(0x282, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                            }
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                w->dmg->flags |= 2;
                                w->step++;
                            }
                            break;
                        case 2:
                            w->vel[1] -= 1.0;
                            if (w->timer > 70) {
                                w->step++;
                            }
                            break;
                        case 3:
                            bossDampVel(obj, 0.9f);
                            if (w->timer > 90) {
                                if (gBossLife > 1) {
                                    beReversalCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                                }
                                BODY_ANIM(w, bdevThrownDownAnim, 1.0f, 1);
                                w->step++;
                            }
                            break;
                        case 4:
                            if (w->timer == 115) {
                                SoundLocation2(0x282, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                            }
                            if (bossStepAnim(w->frame, 0.3f, 3)) {
                                bossSetAnimAt(w->cape, bdevCapeIdleAnim, 0xF0, 0.0f);
                                if (gBossLife <= 0) {
                                    DPRINTF("ボス負けデモを開始します。\n");
                                    bossStartAction(obj, 0, 15);
                                } else {
                                    BODY_ANIM(w, bdevBodyIdleAnim, 0.0f, 0);
                                    END_ACTION(w, obj);
                                    gBossFlags &= ~1;
                                }
                            }
                            break;
                        case 6:
                            break;
                    }
                    w->timer++;
                    break;
                case 0x2C:
                    /* A flinch from a jab (by the side hit), then back; three flinches dizzy it. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d]\n", w->action, w->step, w->timer);
                    gBossPosX = w->frame->pos[0];
                    gBossPosY = w->frame->pos[1];
                    gBossPosZ = w->frame->pos[2];
                    gBossYaw = w->frame->rot[1];
                    switch (w->step) {
                        case 0:
                            w->frame->rot[0] = w->vel[2] * DEG * 1;
                            w->frame->rot[2] = -w->vel[0] * DEG * 1;
                            MOVE(w);
                            TRAIL_FADE();
                            BODY_ANIM(w, bdevKnockedBackAnim1, 0.0f, 1);
                            bossSetAnimAt(w->cape, bdevCapeFoldedAnim, 0xF0, 0.0f);
                            w->cape->f.flags |= 2;
                            w->cape->rot[1] = 0.0f;
                            bossAddFlinchCount(1);
                            w->frame->pos[2] -= 100.0f;
                            w->hitWait = 20;
                            w->step++;
                            break;
                        case 1:
                            if (--w->hitWait == 0) {
                                w->dmg->flags |= 2;
                            }
                            bossStepAnim(w->cape, 0.8f, 2);
                            if (bossStepAnim(w->frame, 0.8f, 2)) {
                                BODY_ANIM(w, bdevBodyIdleAnim, 0.0f, 1);
                                bossSetAnimAt(w->cape, bdevCapeIdleAnim, 0xF0, 0.0f);
                                TRAIL_SHOW(w);
                                if (bossFlinchCountOver(3)) {
                                    w->action = 11;
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
                    gBossPosX = w->frame->pos[0];
                    gBossPosY = w->frame->pos[1];
                    gBossPosZ = w->frame->pos[2];
                    gBossYaw = w->frame->rot[1];
                    switch (w->step) {
                        case 0:
                            w->frame->rot[0] = w->vel[2] * DEG * 1;
                            w->frame->rot[2] = -w->vel[0] * DEG * 1;
                            MOVE(w);
                            TRAIL_FADE();
                            BODY_ANIM(w, bdevKnockedBackAnim0, 0.0f, 1);
                            bossSetAnimAt(w->cape, bdevCapeFoldedAnim, 0xF0, 0.0f);
                            w->cape->f.flags |= 2;
                            w->cape->rot[1] = 0.0f;
                            bossAddFlinchCount(1);
                            w->frame->pos[2] -= 100.0f;
                            w->hitWait = 20;
                            w->step++;
                            break;
                        case 1:
                            if (--w->hitWait == 0) {
                                w->dmg->flags |= 2;
                            }
                            bossStepAnim(w->cape, 0.8f, 2);
                            if (bossStepAnim(w->frame, 0.8f, 2)) {
                                BODY_ANIM(w, bdevBodyIdleAnim, 0.0f, 1);
                                bossSetAnimAt(w->cape, bdevCapeIdleAnim, 0xF0, 0.0f);
                                TRAIL_SHOW(w);
                                if (bossFlinchCountOver(3)) {
                                    w->action = 11;
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
                    /* The hundred-fist punch's flurry: held in front of the cockpit, then knocked away. */
                    gBossPosX = w->frame->pos[0];
                    gBossPosY = w->frame->pos[1];
                    gBossPosZ = w->frame->pos[2];
                    gBossYaw = w->frame->rot[1];
                    w->frame->rot[1] = mathAtan2(-w->frame->pos[0], -w->frame->pos[2]);
                    TRAIL_FADE();
                    gBossFlags &= ~0x800;
                    bossHeadingOffset(gCockpitHeadYaw, 0.0f, 1000.0f, &hx, &hz);
                    w->cape->f.flags |= 2;
                    w->cape->rot[1] = 0.0f;
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
                    if (w->frame->pos[1] < -100.0f) {
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
                            BODY_ANIM(w, bdevHundredFistAnim, 0.0f, 1);
                            bossSetAnimAt(w->cape, bdevCapeFoldedAnim, 0xF0, 0.0f);
                            w->step = 1;
                            break;
                        case 1:
                            bossStepAnim(w->cape, 3.0f, 2);
                            if (bossStepAnim(w->frame, 2.0f, 2)) {
                                BODY_ANIM(w, bdevKnockedBackAnim1, 0.0f, 1);
                                bossSetAnimAt(w->cape, bdevCapeFoldedAnim, 0xF0, 0.0f);
                                w->step = 2;
                            }
                            break;
                        case 2:
                            gBattleFlags2 &= ~0x2000;
                            if (gBossLife <= 0) {
                                DPRINTF("ボス死亡しました。\n");
                                angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                                w->vel[0] = sinf(angle) * 50.0f;
                                w->vel[2] = cosf(angle) * 50.0f;
                                w->count = 30;
                                w->action = 0x2B;
                                w->step = 0;
                                w->vel[1] = 35.0f;
                            } else {
                                angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                                w->vel[0] = sinf(angle) * 10.0f;
                                w->vel[2] = cosf(angle) * 10.0f;
                                w->action = 0x2B;
                                w->step = 0;
                                w->vel[1] = 30.0f;
                            }
                            break;
                    }
                    break;
                case 0x2F:
                    /* Knocked down hard (the kick, or a jab at the end): fall, lie, get up. */
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    MOVE(w);
                    TRAIL_FADE();
                    switch (w->step) {
                        case 0:
                            TRAIL_FADE();
                            gBossFlags &= ~0x800;
                            BODY_ANIM(w, bdevThrownDownAnim, 0.0f, 1);
                            w->cape->f.flags &= ~2;
                            w->timer = 0;
                            w->step++;
                            break;
                        case 1:
                            w->vel[1] -= 1.0;
                            if (gGameFrameCount % 10 == 0) {
                                beSpeffectCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 0);
                            }
                            if (bossStepAnim(w->frame, 1.0f, 2)) {
                                w->dmg->flags |= 2;
                                BODY_ANIM(w, bdevKnockedDownAnim, 0.0f, 1);
                                w->step++;
                            }
                            w->timer++;
                            break;
                        case 2:
                            w->vel[1] -= 0.5;
                            bossStepAnim(w->frame, 0.6f, 0);
                            if (w->timer > 60) {
                                w->step++;
                            }
                            w->timer++;
                            break;
                        case 3:
                            w->vel[1] -= 0.5;
                            if (bossStepAnim(w->frame, 0.6f, 2)) {
                                w->count = fadeRand(100);
                                w->step++;
                            }
                            break;
                        case 4:
                            bossDampVel(obj, 0.95f);
                            w->dmg->flags |= 2;
                            if (w->timer > 60) {
                                if (gBossLife > 1) {
                                    beReversalCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                                }
                                BODY_ANIM(w, bdevThrownDownAnim, 1.0f, 1);
                                w->timer = 0;
                                w->step++;
                            }
                            w->timer++;
                            break;
                        case 5:
                            if (bossStepAnim(w->frame, 0.3f, 3)) {
                                bossSetAnimAt(w->cape, bdevCapeIdleAnim, 0xF0, 0.0f);
                                w->cape->f.flags |= 2;
                                if (gBossLife <= 0) {
                                    DPRINTF("ボス負けデモを開始します。\n");
                                    bossStartAction(obj, 0, 15);
                                } else {
                                    BODY_ANIM(w, bdevBodyIdleAnim, 0.0f, 0);
                                    END_ACTION(w, obj);
                                    gBossFlags &= ~1;
                                }
                            }
                            w->timer++;
                            break;
                        case 6:
                            break;
                    }
                    break;
                case 0x30:
                    /* Hit by the cannon: shaken in place, then knocked down. */
                    gBossPosX = w->frame->pos[0];
                    gBossPosY = w->frame->pos[1];
                    gBossPosZ = w->frame->pos[2];
                    gBossYaw = w->frame->rot[1];
                    w->frame->rot[1] = mathAtan2(-w->frame->pos[0], -w->frame->pos[2]);
                    TRAIL_FADE();
                    bossHeadingOffset(gCockpitHeadYaw, 0.0f, 1000.0f, &hx, &hz);
                    gBossFlags &= ~0x4000;
                    gBossFlags &= ~0x800;
                    DPRINTF("Sstep/Stimer [%d/%d]\n", w->step, w->timer);
                    switch (w->step) {
                        case 0:
                            w->vel[0] = w->vel[1] = w->vel[2] = 0.0f;
                            w->frame->rot[0] = w->frame->rot[1] = w->frame->rot[2] = 0.0f;
                            w->cape->f.flags |= 2;
                            if (w->frame->pos[0] > 0.0f) {
                                BODY_ANIM(w, bdevKnockedBackAnim1, 0.0f, 1);
                                bossSetAnimAt(w->cape, bdevCapeFoldedAnim, 0xF0, 0.0f);
                            } else {
                                BODY_ANIM(w, bdevKnockedBackAnim0, 0.0f, 1);
                                bossSetAnimAt(w->cape, bdevCapeFoldedAnim, 0xF0, 0.0f);
                            }
                            w->timer = 240;
                            w->step++;
                            break;
                        case 1:
                            w->dmg->flags |= 2;
                            bossStepAnim(w->frame, 2.0f, 0);
                            bossStepAnim(w->cape, 2.0f, 0);
                            if (w->timer % 10 >= 5) {
                                w->frame->pos[1] += 20.0f;
                            } else {
                                w->frame->pos[1] -= 20.0f;
                            }
                            if (w->timer-- == 0) {
                                DPRINTF("dead:ボスはん,しになはれ[%d / %d]\n", gBossLife, gBossLifeMax);
                                angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                                w->vel[0] = sinf(angle) * 50.0f;
                                w->vel[2] = cosf(angle) * 50.0f;
                                w->action = 0x2F;
                                w->step = 0;
                                w->vel[1] = 35.0f;
                            }
                            break;
                    }
                    break;
                case 0x37:
                    break;
            }
            break;
        case 3:
            /* Before the draw: keep the boss in the arena, and colour the after-images and the
             * glows. */
            if (w->frame->pos[0] > 6500.0f) {
                DPRINTF("BOSSLIMIT : X+\n");
                w->frame->pos[0] = 6500.0f;
            }
            if (w->frame->pos[0] < -6500.0f) {
                DPRINTF("BOSSLIMIT : X-\n");
                w->frame->pos[0] = -6500.0f;
            }
            if (w->frame->pos[1] > 2000.0f) {
                DPRINTF("BOSSLIMIT : Y+\n");
                w->frame->pos[1] = 2000.0f;
            }
            if (w->frame->pos[1] < -2000.0f) {
                DPRINTF("BOSSLIMIT : Y-\n");
                w->frame->pos[1] = -2000.0f;
            }
            if (w->frame->pos[2] > -800.0f) {
                DPRINTF("BOSSLIMIT : Z+\n");
                w->frame->pos[2] = -800.0f;
            }
            if (w->frame->pos[2] < -11000.0f) {
                DPRINTF("BOSSLIMIT : Z-\n");
                w->frame->pos[2] = -11000.0f;
            }
            gBossPosX = w->frame->pos[0];
            gBossPosY = w->frame->pos[1];
            gBossPosZ = w->frame->pos[2];
            gBossYaw = w->frame->rot[1];
            w->ghost[0]->dl = bossCombineEnvColorDL(0, 0, 0x80, gAfterImageAlpha1);
            w->ghost[1]->dl = bossCombineEnvColorDL(0, 0, 0x80, gAfterImageAlpha1);
            w->ghost[2]->dl = bossCombineEnvColorDL(0, 0, 0x80, gAfterImageAlpha2);
            w->ghost[3]->dl = bossCombineEnvColorDL(0, 0, 0x80, gAfterImageAlpha2);
            if (gBossFlags & 0x4000) {
                /* The barrier's yellow glow. */
                w->glow += 3;
                if (w->glow > 160) {
                    w->glow = 160;
                }
                w->frame->dl = bossFogEnvColorDL(0xFF, 0xFF, 0, w->glow, 0, 0, 0, 0, 0x3E3);
                w->cape->dl = bossFogEnvColorDL(0xFF, 0xFF, 0, w->glow, 0, 0, 0, 0, 0x3E3);
            } else if (gBossFlags & 0x100) {
                /* A white flash. */
                w->frame->dl = bossFogEnvColorDL(0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0, 0x3E3);
                w->cape->dl = bossFogEnvColorDL(0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0, 0x3E3);
                gBossFlags &= ~0x100;
            } else if (gBossFlags & 0x10000) {
                /* Black, flashing white once a second (the entrance demo). */
                if (gGameFrameCount % 60 == 0) {
                    w->frame->dl = bossFogEnvColorDL(0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0, 0x3E3);
                    w->cape->dl = bossFogEnvColorDL(0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0, 0x3E3);
                } else {
                    w->frame->dl = bossFogEnvColorDL(0, 0, 0, 0xFF, 0, 0, 0, 0, 0x3E3);
                    w->cape->dl = bossFogEnvColorDL(0, 0, 0, 0xFF, 0, 0, 0, 0, 0x3E3);
                }
            } else {
                w->glow -= 3;
                if (w->glow <= 0) {
                    w->glow = 0;
                }
                w->frame->dl = bossFogEnvColorDL(0, 0, 0, w->glow, 0, 0, 0, 0, 0x3E3);
                w->cape->dl = bossFogEnvColorDL(0, 0, 0, w->glow, 0, 0, 0, 0, 0x3E3);
            }
            if (w->action == 0x2E) {
                w->frame->dl = bossLitTexEnvColorDL(0, 0, 0, 0);
            }
            if (w->action == 0x30) {
                w->frame->dl = bossLitTexEnvColorDL(0, 0, 0, 0);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
