#include "tlb.h"
#include "tlb/tlb_0326.h"

/* "Bkas_proc": the Kashiwagi (kabuki) robot boss. One task drives the whole battle: its body
 * and a second model on it ("cape"), its shadow, the attack and damage boxes, the entrance
 * demo, the attacks, the hits and the two endings. It walks on the ground (vramRayGround,
 * vramCheck) where Bdev (tlb_0327_0) flies; Bdev is the same template.
 *
 * Its debug prints are compiled out; DPRINTF keeps each string, in the game's .rodata order,
 * and emits no code. Where each stood is a best guess. */

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
/* vramCheck takes its point by value (the game copies the Vec3f into a1-a3). */
typedef void (*VramCheckFunc)(AnimeFrame *f, Vec3f from, f32 down, f32 above);

/* This boss's other parts (tlb_0326_1), called far (lui/addiu/jalr) like main's. */
#define bkasOhudaCreate (*bkasOhudaCreate)
#define bkasSensuCreate (*bkasSensuCreate)
#define bkasHandCreate (*bkasHandCreate)
#define bkasWaterProc (*bkasWaterProc)
#define bkasTakiProc (*bkasTakiProc)
#define bkasKabukiBCreate (*bkasKabukiBCreate)
#define wakemiCreate (*wakemiCreate)
#define demoStageCreate (*demoStageCreate)
#define demoLightCreate (*demoLightCreate)
#define demoPaperCreate (*demoPaperCreate)
#define bkasSensuRCreate (*bkasSensuRCreate)
#define bkasBappcamProc (*bkasBappcamProc)

AnimFrame bkasBodyAnim[] = {
    { 0x0301B890, 15 },
    { 0x0301C890, 15 },
    { 0, 0 },
    { 0, 0 },
}; /* the body's texture animation (file 0xF5) */

/* The model animations, in file 0xF5. */
extern u8 bkasBodyIdleAnim[]; /* the body: idle */
extern u8 bkasCapeIdleAnim[]; /* the cape: idle */
extern u8 bkasWakemiStep1Anim[];
extern u8 bkasWakemiStep2Anim[];
extern u8 bkasWakemiStep3Anim[];
extern u8 bkasOhudaStep3Anim[];
extern u8 bkasOhudaStep4Anim[];
extern u8 bkasDanceStep1Anim[];
extern u8 bkasDanceStep2Anim[];
extern u8 bkasDanceStep3Anim[];
extern u8 bkasDanceStep4Anim[];
extern u8 bkasDanceStep5Anim[];
extern u8 bkasDanceStep6Anim[];
extern u8 bkasDanceStep7Anim[];
extern u8 bkasSensuStep2Anim[];
extern u8 bkasAction3RaiseAnim[];
extern u8 bkasAction3ChargeAnim[];
extern u8 bkasAction4RaiseAnim[];
extern u8 bkasAction4ChargeAnim[];
extern u8 bkasKnockedBackAnim0[]; /* knocked back (one side) */
extern u8 bkasKnockedBackAnim1[]; /* knocked back (the other side) */
extern u8 bkasThrownDownAnim[]; /* thrown down */
extern u8 bkasLosingDemoBodyAnim[]; /* the losing demo: the body */
extern u8 bkasLosingDemoCapeAnim[]; /* the losing demo: the cape */

/* The boss's work (at obj + 0x20). */
typedef struct BkasW {
    /* 0x00 */ AnimeFrame *frame; /* the body */
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ ACollFrame *atk; /* its attack box (kind 0xCD) */
    /* 0x18 */ ACollFrame *dmg; /* its damage box (kind 0xCC) */
    /* 0x1C */ MPolFrame *shadow;
    /* 0x20 */ Anim anim; /* the body's texture animation */
    /* 0x30 */ u8 pad30[4];
    /* 0x34 */ AnimeFrame *cape; /* the second model, on the body */
    /* 0x38 */ u8 pad38[0x14];
    /* 0x4C */ s32 hitWait; /* steps before the damage box is live again after a flinch */
    /* 0x50 */ u8 action; /* "Mstep" (bossStartAction sets it) */
    /* 0x51 */ u8 step; /* "Sstep" */
    /* 0x52 */ u8 nextAction; /* the action and step to resume after a move (bossStartActionKind) */
    /* 0x53 */ u8 nextStep;
    /* 0x54 */ s32 timer; /* "Stimer" */
    /* 0x58 */ u8 count; /* steps before the damage box is live (as Bbisu's); the vanishing dash's alpha */
    /* 0x59 */ u8 unused59; /* set 0 with count at the start of the sink-and-charms action, never read */
    /* 0x5C */ s32 glow; /* the yellow glow's alpha, 0-192, while gBossFlags 0x4000 */
} BkasW;

#define BKAS(obj) ((BkasW *) ((u32) (obj) + sizeof(Obj)))

/* One degree. */
#define DEG 0.0174532930038800785

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

/* The body leans harder (the knock-backs). */
#define BODY_TILT1(w)                                    \
    w->frame->rot[0] = w->vel[2] * DEG * 1;              \
    w->frame->rot[2] = -w->vel[0] * DEG * 1;

/* Both models turn toward the velocity. */
#define TURN(w)                                                   \
    bossTurnToDir(w->frame, w->vel[0], w->vel[2], 2.0f);          \
    bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);

/* The body faces the Impact (the arena's origin). */
#define FACE(w) w->frame->rot[1] = mathAtan2(-w->frame->pos[0], -w->frame->pos[2]);

/* A new body animation, with its texture animation. */
#define BODY_ANIM(w, data, at, index)                                   \
    bossSetAnimAt(w->frame, (data), 0xF5, at);                          \
    fadeAnimStart(&w->anim, bkasBodyAnim, index, &w->frame->segs[3], floadGetSetFile(0xF5, 0));

/* Both models to a frame layer. */
#define TO_LAYER(w, layer)             \
    frmMove(&w->frame->f, layer);      \
    frmMove(&w->cape->f, layer);

/* The damage box's size, as made. */
#define DMG_BOX(w, oz)          \
    w->dmg->radius = 400;       \
    w->dmg->height = 600;       \
    w->dmg->ofs[1] = -300;      \
    w->dmg->ofs[2] = oz;

/* The ray down from the body, set at the top of the step. */
#define ON_GROUND(w) vramRayGround(&from, (Vec3f *) w->vel, &at, &t)

#define SOUND(w, id)                                                                                 \
    SoundLocation2(id, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f, \
                   15000.0f, gBattleCamView->fovy);

/* The raise that starts a hand attack: face the Impact, turn the cape, start the raise. Action 3
 * marks the boss busy (flag 4) after the turn, action 4 before it; that is all that differs. */
#define RAISE_FLAG_LATE(w, raise)                                \
    FACE(w);                                                     \
    bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);          \
    gBossFlags |= 4;                                             \
    BODY_ANIM(w, raise, 0.0f, 0);
#define RAISE_FLAG_FIRST(w, raise)                               \
    gBossFlags |= 4;                                             \
    FACE(w);                                                     \
    bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);          \
    BODY_ANIM(w, raise, 0.0f, 0);

/* Actions 3 and 4: the hand ("hand", type 0 or 1) from in front of the Impact, then the charge
 * with the damage box pushed forward. */
#define HAND_ATTACK(w, obj, raiseStep, raise, type, charge)                               \
    MOVE(w);                                                                              \
    bossStepAnim(w->cape, 0.3f, 0);                                                       \
    switch (w->step) {                                                                    \
        case 0:                                                                           \
            MOVE(w);                                                                      \
            CAPE_TILT(w);                                                                 \
            BODY_TILT(w);                                                                 \
            TURN(w);                                                                      \
            gBossFlags &= ~0x8000;                                                        \
            if (bossFlyTowardXZ(obj, w->frame->pos[0], -3500.0f, 1.0f, 20.0f)) {          \
                w->frame->rot[0] = w->frame->rot[2] = 0.0f;                               \
                w->step++;                                                                \
            }                                                                             \
            break;                                                                        \
        case 1:                                                                           \
            bossDampVel(obj, 0.9f);                                                       \
            bossTurnToward2(w->cape, 0.0f, 0.0f, 2.0f);                                   \
            if (bossTurnToward2(w->frame, 0.0f, 0.0f, 2.0f)) {                            \
                w->step++;                                                                \
            }                                                                             \
            break;                                                                        \
        case 2:                                                                           \
            raiseStep(w, raise);                                                          \
            w->timer = 0;                                                                 \
            w->step++;                                                                    \
            break;                                                                        \
        case 3:                                                                           \
            gBossFlags |= 4;                                                              \
            if (w->timer == 10) {                                                         \
                SOUND(w, 0x24C);                                                          \
                SOUND(w, 0x132);                                                          \
            }                                                                             \
            FACE(w);                                                                      \
            bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);                           \
            if (bossStepAnim(w->frame, 1.0f, 2)) {                                        \
                beFlashFCreate(0xFF, 0xFF, 0xFF, 5);                                      \
                effectKiranCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 500.0f); \
                bkasHandCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], type); \
                w->timer = 0;                                                             \
                w->step++;                                                                \
            }                                                                             \
            break;                                                                        \
        case 4:                                                                           \
            gBossFlags |= 4;                                                              \
            if (w->timer > 30) {                                                          \
                w->step++;                                                                \
            }                                                                             \
            break;                                                                        \
        case 5:                                                                           \
            gBossFlags |= 4;                                                              \
            FACE(w);                                                                      \
            bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);                           \
            CAPE_TILT(w);                                                                 \
            w->dmg->radius = 400;                                                         \
            w->dmg->height = 600;                                                         \
            w->dmg->ofs[1] = -300;                                                        \
            w->dmg->ofs[2] = 300;                                                         \
            if (w->timer == 45) {                                                         \
                SOUND(w, 0x2B8);                                                          \
            }                                                                             \
            if (bossFlyTowardXZ(obj, 200.0f, -1500.0f, 2.0f, 30.0f)) {                    \
                w->step++;                                                                \
            }                                                                             \
            break;                                                                        \
        case 6:                                                                           \
            gBossFlags |= 4;                                                              \
            FACE(w);                                                                      \
            CAPE_TILT(w);                                                                 \
            BODY_TILT(w);                                                                 \
            bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);                           \
            bossStepAnim(w->frame, 1.0f, 2);                                              \
            if (bossDampVel(obj, 0.8f) == 0) {                                            \
                w->timer = 0;                                                             \
                w->step++;                                                                \
            }                                                                             \
            break;                                                                        \
        case 7:                                                                           \
            gBossFlags |= 4;                                                              \
            if (w->timer >= 60) {                                                         \
                BODY_ANIM(w, charge, 0.0f, 0);                                            \
                gBossFlags |= 0x8000;                                                     \
                w->step++;                                                                \
            }                                                                             \
            break;                                                                        \
        case 8:                                                                           \
            gBossFlags |= 4;                                                              \
            gBossFlags &= ~0x8000;                                                        \
            if (bossStepAnim(w->frame, 0.5f, 2)) {                                        \
                BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);                                         \
                w->dmg->ofs[2] = 0;                                                       \
                bossClearAction(obj);                                                     \
            }                                                                             \
            break;                                                                        \
        case 9:                                                                           \
            break;                                                                        \
    }                                                                                     \
    w->timer++;

/* bossStartActionKind's moves: stop, walk to a place, stop, then the action and step it was
 * given. */
#define WALK_TO(w, obj, x, z)                                      \
    CAPE_TILT(w);                                                  \
    BODY_TILT(w);                                                  \
    MOVE(w);                                                       \
    TURN(w);                                                       \
    bossStepAnim(w->frame, 0.3f, 0);                               \
    bossStepAnim(w->cape, 0.3f, 0);                                \
    switch (w->step) {                                             \
        case 0:                                                    \
            if (bossDampVel(obj, 0.8f) == 0) {                     \
                w->step++;                                         \
            }                                                      \
            break;                                                 \
        case 1:                                                    \
            if (bossFlyTowardXZ(obj, x, z, 1.6f, 30.0f)) {         \
                w->step++;                                         \
            }                                                      \
            break;                                                 \
        case 2:                                                    \
            if (bossDampVel(obj, 0.8f) == 0) {                     \
                w->action = w->nextAction;                         \
                w->step = w->nextStep;                             \
            }                                                      \
            break;                                                 \
    }

#define ABSDIFF(a, b) ((a) > (b) ? (a) - (b) : -((a) - (b)))

/* Back toward the ground's height (-270) when it drifts. sic: the first test is against +270. */
#define HEIGHT_FIX(w)                                                                      \
    if (w->frame->pos[1] < -280.0f) {                                                      \
        DPRINTF("ボス位置補正[%f]\n", w->frame->pos[1]);                                    \
        w->frame->pos[1] += ABSDIFF(w->frame->pos[1], 270.0f) / 30.0f;                     \
    }                                                                                      \
    if (w->frame->pos[1] > -260.0f) {                                                      \
        w->frame->pos[1] -= (w->frame->pos[1] + 270.0f > 0.0f ? w->frame->pos[1] + 270.0f  \
                                                              : -(w->frame->pos[1] + 270.0f)) / 30.0f; \
    }

/* The same without the print (action 11 has none: the game's .rodata holds the string once). */
#define HEIGHT_FIX_QUIET(w)                                                                \
    if (w->frame->pos[1] < -280.0f) {                                                      \
        w->frame->pos[1] += ABSDIFF(w->frame->pos[1], 270.0f) / 30.0f;                     \
    }                                                                                      \
    if (w->frame->pos[1] > -260.0f) {                                                      \
        w->frame->pos[1] -= (w->frame->pos[1] + 270.0f > 0.0f ? w->frame->pos[1] + 270.0f  \
                                                              : -(w->frame->pos[1] + 270.0f)) / 30.0f; \
    }

/* The game reads msg back from its home, `lhu` for the switch and `lh` for tskDefaultProc: a
 * plain `switch (msg)` gives both (a kept FAKE, 2026-10-17, in place of `*(u16 *) &msg`; as in
 * tlb_0323_0).
 *  - Every layer is read through gFrmLayers[], as the twin Bdev_proc writes them: with one base
 *    for all the layer reads, uopt holds no layer address across a frmMove, as in the game. The
 *    effect layer as its own name at both moves of actions 13 and 18 is real 12 (a kept FAKE,
 *    2026-10-17).
 *  - 0.1f has two spellings, as the game's .rodata and its saved-register use show (IDO_NOTES
 *    365): 0.10f where the game keeps it in a saved register (action 14's steps 2 and 9,
 *    action 15's step-1 pompoms and step 3's second pompom), 0.1f everywhere else.
 *  - The landing at the end of a fall is written out at each of its three uses, not as a
 *    macro: a macro puts it all on one source line, and IDO schedules by line (IDO_NOTES 12).
 *  - Action 0's two life tests are one-line ifs (test and store on one line, no braces), as in
 *    tlb_0323_0: that gives the game's schedule of the hoisted &gBossActionArg. They replace
 *    a FAKE layout (the 0.33333 test and the decrement on one line), a kept FAKE, 2026-10-17. */
s32 Bkas_proc(Obj *obj, s16 msg, void *arg) {
    s32 pad2[2];
    Vec3f from; /* the ray down from the body */
    Vec3f at; /* where it met the ground */
    f32 t;
    f32 hx; /* a point ahead of the cockpit (bossHeadingOffset) */
    s32 pad0;
    f32 hz;
    f32 angle;
    s32 pad1[2];
    f32 ox; /* an offset from the body (bossHeadingOffset) */
    f32 oz;
    BkasW *w = BKAS(obj);
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "Bkas_proc";
            gBattleTask = obj;
            if ((w->frame = (AnimeFrame *) frmCreate(gFrmLayers[2], gFrmKindAnime)) == NULL) {
                return -1;
            }
            if ((w->cape = (AnimeFrame *) frmCreate(gFrmLayers[2], gFrmKindAnime)) == NULL) {
                return -1;
            }
            w->shadow = shadowCreate();
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(uda_atk_root_frame, &w->frame->f, 300, 0, 0, 0)) ==
                NULL) {
                return -1;
            }
            w->atk->kind = 0xCD;
            if ((w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->frame->f, 400, 600, -300, 0,
                                                          0)) == NULL) {
                return -1;
            }
            w->dmg->kind = 0xCC;
            BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
            bossSetFrameFlags2(w->frame, 0x20);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 2.0f;
            bossSetAnimAt(w->cape, bkasCapeIdleAnim, 0xF5, 0.0f);
            bossSetFrameFlags2(w->cape, 0x20);
            w->cape->scale[0] = w->cape->scale[1] = w->cape->scale[2] = 2.0f;
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = -3000.0f;
            w->cape->pos[0] = w->frame->pos[0];
            w->cape->pos[1] = w->frame->pos[1];
            w->cape->pos[2] = w->frame->pos[2];
            w->action = 0;
            w->step = 0;
            w->timer = 0;
            gBossLifeMax = 3000;
            gBossLife = 3000;
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
            frmDestroy(&w->shadow->f);
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
            /* Splashes under the boss three times a second, but in the entrance demo. */
            if (w->action != 14 && gGameFrameCount % 20 == 0) {
                beShibukiCreate(w->frame->pos[0], -1200.0f, w->frame->pos[2], 2);
            }
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
                bossStartAction(obj, 0, 14);
            }
            w->cape->pos[0] = w->frame->pos[0];
            w->cape->pos[1] = w->frame->pos[1];
            w->cape->pos[2] = w->frame->pos[2];
            gMegaBurstY = gBossPosY;
            from.x = w->frame->pos[0];
            from.y = w->frame->pos[1] - 1800.0f;
            from.z = w->frame->pos[2];
            acollAtkWasHit(w->atk);

            /* The hits. */
            if (acollDmgWasHit(w->dmg)) {
                if (w->dmg->hitKind == 15) {
                    DPRINTF("バルカンに当たりました。[%d]\n", gBossLife);
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
                        angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                        w->vel[0] = sinf(angle) * 15.0f; w->vel[2] = cosf(angle) * 15.0f;
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
                        beBpieceCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f, 10);
                        SoundLocation2(0x22B, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                       w->dmg->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                        bossDamage(3, 16, 1);
                    }
                    if (gBossLife <= 0) {
                        DPRINTF("ボス死亡しました。\n");
                        gBattleFlags2 |= 0x400;
                        w->dmg->flags &= ~2;
                        angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                        w->vel[0] = sinf(angle) * 15.0f; w->vel[2] = cosf(angle) * 15.0f;
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
                        gBossFlags |= 0x8000;
                        gBrotherFlags &= ~0x20000;
                        TO_LAYER(w, gFrmLayers[2]);
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
                        w->vel[0] = sinf(angle) * 15.0f; w->vel[2] = cosf(angle) * 15.0f;
                        w->count = 30;
                        w->action = 0x2B;
                        w->step = 0;
                        w->vel[1] = 120.0f;
                    }
                } else if (w->dmg->hitKind == 3) {
                    DPRINTF("ストレートに当たりました。\n");
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
                        gBossFlags |= 0x8000;
                        gBrotherFlags &= ~0x20000;
                        TO_LAYER(w, gFrmLayers[2]);
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
                        angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                        w->vel[0] = sinf(angle) * 15.0f; w->vel[2] = cosf(angle) * 15.0f;
                        w->count = 30;
                        w->action = 0x2B;
                        w->step = 0;
                        w->vel[1] = 120.0f;
                    }
                } else if (w->dmg->hitKind == 8) {
                    DPRINTF("キックに当たりました。\n");
                    if (gBossFlags & 0x4000) {
                        DPRINTF("効かぬわ。\n");
                    } else {
                        bossDamage(80, 8, 3);
                        sndPlay(0x102);
                        beHitmarkCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f);
                        beBpieceCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f, 10);
                        beBpieceCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f, 10);
                        w->dmg->flags &= ~2;
                        gBossFlags &= ~0x200;
                        gBossFlags |= 0x8000;
                        gBrotherFlags &= ~0x20000;
                        TO_LAYER(w, gFrmLayers[2]);
                        /* The second kick down throws it far. */
                        bossAddDownCount(1);
                        if (bossDownCountOver(2)) {
                            angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                            w->vel[0] = sinf(angle) * 80.0f;
                            w->vel[2] = cosf(angle) * 80.0f;
                            w->count = 30;
                            w->action = 0x2F;
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
                        angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                        w->vel[0] = sinf(angle) * 15.0f; w->vel[2] = cosf(angle) * 15.0f;
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
                        beHitmarkCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f);
                        beBpieceCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f, 10);
                        beBpieceCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f, 10);
                        if (gBattleFlags2 & 0x40000) {
                            bossDamage(800, 11, 0);
                        } else {
                            bossDamage(400, 11, 0);
                        }
                        TO_LAYER(w, gFrmLayers[2]);
                        w->dmg->flags &= ~2;
                        gBossFlags &= ~0x200;
                        gBossFlags |= 0x8000;
                        gBrotherFlags &= ~0x20000;
                        w->count = 30;
                        w->action = 0x30;
                        w->step = 0;
                    }
                    if (gBossLife <= 0) {
                        gBattleFlags2 |= 0x400;
                        w->dmg->flags &= ~2;
                    }
                } else if (w->dmg->hitKind == 0xCA) {
                    DPRINTF("外部インパクトに攻撃されました。\n");
                    if (gBossFlags & 0x4000) {
                        DPRINTF("効かぬわ。\n");
                    } else {
                        w->cape->f.flags |= 2;
                        beHitmarkCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 3.0f);
                        beBpieceCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f, 10);
                        beBpieceCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f, 10);
                        SoundLocation2(0x27D, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                       w->dmg->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                        w->dmg->flags &= ~2;
                        gBossFlags &= ~0x200;
                        gBrotherFlags &= ~0x20000;
                        TO_LAYER(w, gFrmLayers[2]);
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
                        angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                        w->vel[0] = sinf(angle) * 15.0f; w->vel[2] = cosf(angle) * 15.0f;
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
                        w->frame->pos[1] = 0.0f;
                        TO_LAYER(w, gFrmLayers[2]);
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
                if (w->action == 9) {
                    bossStartAction(obj, 0, 9);
                }
                if (w->action == 0x17) {
                    bossStartAction(obj, 0, 9);
                }
                if (w->action == 0x18) {
                    bossStartAction(obj, 0, 9);
                }
                if (w->action == 0x19) {
                    bossStartAction(obj, 0, 9);
                }
            }

            switch (w->action) {
                case 0:
                    /* Waiting: walk about the far side of the arena and pick the next attack.
                     * The attack order has three lists, by the life left. */
                    w->cape->f.flags |= 2;
                    gBossPhase = 0;
                    /* one-line ifs, the game's style (IDO schedules by line, IDO_NOTES 12, 324) */
                    if (gBossLife < gBossLifeMax * 0.66666) gBossPhase = 1;
                    if (gBossLife < gBossLifeMax * 0.33333) gBossPhase = 2;
                    gBossActionArg--;
                    if (gBossActionArg <= 0) {
                        if (gBossPhase == 0) {
                            if (gBossActionCount == 0) {
                                bossStartAction(obj, 40, 2);
                            } else if (gBossActionCount == 1) {
                                bossStartAction(obj, 40, 17);
                            } else if (gBossActionCount == 2) {
                                bossStartAction(obj, 40, 3);
                            } else if (gBossActionCount == 3) {
                                bossStartAction(obj, 40, 17);
                            } else if (gBossActionCount == 4) {
                                bossStartAction(obj, 40, 13);
                            } else if (gBossActionCount == 5) {
                                bossStartAction(obj, 40, 2);
                            } else if (gBossActionCount == 6) {
                                bossStartAction(obj, 40, 6);
                            } else if (gBossActionCount == 7) {
                                bossStartAction(obj, 40, 4);
                            } else {
                                gBossActionCount = 0;
                            }
                        } else if (gBossPhase == 1) {
                            if (gBossActionCount == 0) {
                                bossStartAction(obj, 30, 19);
                            } else if (gBossActionCount == 1) {
                                bossStartAction(obj, 30, 20);
                            } else if (gBossActionCount == 2) {
                                bossStartAction(obj, 30, 3);
                            } else if (gBossActionCount == 3) {
                                bossStartAction(obj, 30, 17);
                            } else if (gBossActionCount == 4) {
                                bossStartAction(obj, 30, 4);
                            } else if (gBossActionCount == 5) {
                                bossStartAction(obj, 30, 18);
                            } else if (gBossActionCount == 6) {
                                bossStartAction(obj, 30, 6);
                            } else if (gBossActionCount == 7) {
                                bossStartAction(obj, 30, 7);
                            } else if (gBossActionCount == 8) {
                                bossStartAction(obj, 30, 13);
                            } else {
                                gBossActionCount = 0;
                            }
                        } else if (gBossPhase == 2) {
                            if (gBossActionCount == 0) {
                                bossStartAction(obj, 20, 19);
                            } else if (gBossActionCount == 1) {
                                bossStartAction(obj, 30, 20);
                            } else if (gBossActionCount == 2) {
                                bossStartAction(obj, 20, 13);
                            } else if (gBossActionCount == 3) {
                                bossStartAction(obj, 20, 7);
                            } else if (gBossActionCount == 4) {
                                bossStartAction(obj, 30, 6);
                            } else if (gBossActionCount == 5) {
                                bossStartAction(obj, 20, 21);
                            } else if (gBossActionCount == 6) {
                                bossStartAction(obj, 20, 18);
                            } else if (gBossActionCount == 7) {
                                bossStartAction(obj, 30, 6);
                            } else if (gBossActionCount == 8) {
                                bossStartAction(obj, 20, 21);
                            } else if (gBossActionCount == 9) {
                                bossStartAction(obj, 20, 12);
                            } else if (gBossActionCount == 10) {
                                bossStartAction(obj, 20, 4);
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
                        bossPatrolZ(obj, -6000.0f, -3000.0f, 0.3f);
                        HEIGHT_FIX(w);
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
                    ((VramCheckFunc) vramCheck)(w->frame, from, w->vel[1], 1800.0f);
                    break;
                case 1:
                    /* Fall to the ground, then walk back. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d][%f]\n", w->action, w->step, w->timer, w->vel[1]);
                    w->vel[1] -= 1.0;
                    if (ON_GROUND(w)) {
                        w->frame->pos[1] = at.y + 1800.0f + 0.1f;
                        w->vel[1] = 0.0f;
                    }
                    MOVE(w);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    TURN(w);
                    switch (w->step) {
                        case 0:
                            bossStepAnim(w->frame, 0.3f, 0);
                            if (bossFlyTowardXZ(obj, w->frame->pos[0], -4000.0f, 2.0f, 30.0f)) {
                                w->step++;
                            }
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
                    /* The paper charms ("Ohuda"): come forward and throw three. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d][%f]\n", w->action, w->step, w->timer, w->vel[1]);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    MOVE(w);
                    bossStepAnim(w->cape, 0.3f, 0);
                    switch (w->step) {
                        case 0:
                            TURN(w);
                            if (bossFlyTowardXZ(obj, w->frame->pos[0], -4000.0f, 2.0f, 30.0f)) {
                                w->step++;
                            }
                            break;
                        case 1:
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 3.0f);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                w->step++;
                            }
                            break;
                        case 2:
                            gBossFlags |= 4;
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 3.0f);
                            if (bossFlyTowardXZ(obj, 0.0f, -800.0f, 2.0f, 30.0f)) {
                                w->step++;
                            }
                            break;
                        case 3:
                            gBossFlags |= 4;
                            if (bossDampVel(obj, 0.8f) == 0) {
                                BODY_ANIM(w, bkasOhudaStep3Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 4:
                            gBossFlags |= 4;
                            if (bossStepAnim(w->frame, 1.0f, 2)) {
                                BODY_ANIM(w, bkasOhudaStep4Anim, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 5:
                            gBossFlags |= 4;
                            if (w->timer == 30) {
                                bkasOhudaCreate(0.0f, 0.0f, -400.0f, 0);
                                if (!(gBattleFlags & 0x10)) {
                                    sndPlay(0x158);
                                }
                            }
                            if (w->timer == 60) {
                                bkasOhudaCreate(50.0f, -10.0f, -400.0f, 1);
                                if (!(gBattleFlags & 0x10)) {
                                    sndPlay(0x158);
                                }
                            }
                            if (w->timer == 85) {
                                if (fadeRand(100) > 95) {
                                    bkasOhudaCreate(-50.0f, -30.0f, -400.0f, 3);
                                    if (!(gBattleFlags & 0x10)) {
                                        sndPlay(0x158);
                                    }
                                } else {
                                    bkasOhudaCreate(-50.0f, -30.0f, -400.0f, 2);
                                    if (!(gBattleFlags & 0x10)) {
                                        sndPlay(0x158);
                                    }
                                }
                            }
                            if (bossStepAnim(w->frame, 0.4f, 2)) {
                                BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 6:
                            gBossFlags |= 4;
                            if (w->timer > 30) {
                                bossClearAction(obj);
                            }
                            break;
                        case 7:
                            break;
                        case 8:
                            break;
                    }
                    w->timer++;
                    break;
                case 3:
                    /* A hand (type 0), then the charge. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d][%f]\n", w->action, w->step, w->timer, w->vel[1]);
                    HAND_ATTACK(w, obj, RAISE_FLAG_LATE, bkasAction3RaiseAnim, 0, bkasAction3ChargeAnim);
                    break;
                case 4:
                    /* The other hand (type 1), then the charge. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d][%f]\n", w->action, w->step, w->timer, w->vel[1]);
                    HAND_ATTACK(w, obj, RAISE_FLAG_FIRST, bkasAction4RaiseAnim, 1, bkasAction4ChargeAnim);
                    break;
                case 6:
                    /* The four "wakemi" (doubles) and the barrier (gBossFlags 0x4000). */
                    DPRINTF("Sstep/Stimer [%d/%d]\n", w->step, w->timer);
                    MOVE(w);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    bossStepAnim(w->cape, 0.3f, 0);
                    switch (w->step) {
                        case 0:
                            bossStartActionKind(obj, 2, 6, 1);
                            break;
                        case 1:
                            gBossFlags |= 4;
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 2.0f);
                            if (bossTurnToward2(w->frame, 0.0f, 0.0f, 2.0f)) {
                                BODY_ANIM(w, bkasWakemiStep1Anim, 0.0f, 0);
                                w->timer = 0;
                                soundActpanCreate(obj, 0x176, gBattleCamView, 500.0f, 11000.0f, gBattleCamView->fovy);
                                w->step++;
                            }
                            break;
                        case 2:
                            gBossFlags |= 4;
                            gBossFlags |= 0x4000;
                            gBossFlags |= 0x800;
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 1.0f);
                            if (w->timer == 20) {
                                sndPlay(0x17F);
                            }
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bkasWakemiStep2Anim, 0.0f, 0);
                                w->step++;
                            }
                            w->timer++;
                            break;
                        case 3:
                            gBossFlags |= 4;
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                beFlashFCreate(0xFF, 0xFF, 0xFF, 5);
                                effectKiranCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 500.0f);
                                BODY_ANIM(w, bkasWakemiStep3Anim, 0.0f, 0);
                                wakemiCreate(w->frame->pos[0] + 1000.0f, w->frame->pos[1], -1000.0f, 0);
                                wakemiCreate(w->frame->pos[0] - 1000.0f, w->frame->pos[1], -1000.0f, 1);
                                wakemiCreate(w->frame->pos[0] + 2000.0f, w->frame->pos[1], -2000.0f, 2);
                                wakemiCreate(w->frame->pos[0] - 2000.0f, w->frame->pos[1], -2000.0f, 3);
                                w->step++;
                            }
                            break;
                        case 4:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.2f, 0);
                            if (bossFlyTowardXZ(obj, 0.0f, -1500.0f, 0.2f, 15.0f)) {
                                w->step++;
                            }
                            break;
                        case 5:
                            gBossFlags |= 4;
                            if (bossDampVel(obj, 0.8f) == 0) {
                                w->timer = 120;
                                w->step++;
                            }
                            break;
                        case 6:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.2f, 0);
                            if (!(w->timer & 7)) {
                                beNullattackCreate(0.0f, 0.0f, 0.0f, 0x55);
                            }
                            if (w->timer-- == 0) {
                                BODY_ANIM(w, bkasWakemiStep2Anim, 1.0f, 0);
                                w->step++;
                            }
                            break;
                        case 7:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.5f, 3);
                            if (bossFlyTowardXZ(obj, 0.0f, -4000.0f, 0.2f, 20.0f)) {
                                BODY_ANIM(w, bkasWakemiStep1Anim, 1.0f, 0);
                                w->step++;
                            }
                            break;
                        case 8:
                            gBossFlags |= 4;
                            if (bossStepAnim(w->frame, 0.5f, 3)) {
                                BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
                                gBossFlags &= ~0x4000;
                                gBossFlags &= ~0x800;
                                sndPlay(0x8176);
                                bossClearAction(obj);
                            }
                            break;
                    }
                    break;
                case 7:
                    /* The "kabukiB" (type 9) and a long dance. */
                    DPRINTF("Sstep/Stimer [%d/%d]\n", w->step, w->timer);
                    MOVE(w);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    bossStepAnim(w->cape, 0.3f, 0);
                    switch (w->step) {
                        case 0:
                            TURN(w);
                            TO_LAYER(w, gFrmLayers[2]);
                            if (bossFlyTowardXZ(obj, w->frame->pos[0], -5000.0f, 2.0f, 30.0f)) {
                                w->step++;
                            }
                            break;
                        case 1:
                            gBossFlags |= 4;
                            bossDampVel(obj, 0.9f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 2.0f);
                            if (bossTurnToward2(w->frame, 0.0f, 0.0f, 2.0f)) {
                                BODY_ANIM(w, bkasDanceStep1Anim, 0.0f, 0);
                                gBrotherFlags |= 0x20000;
                                gBossFlags |= 0x800;
                                w->step++;
                            }
                            break;
                        case 2:
                            gBossFlags |= 4;
                            bossDampVel(obj, 0.9f);
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bkasDanceStep2Anim, 0.0f, 0);
                                bkasKabukiBCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 9);
                                sndPlay(0x17E);
                                w->timer = 120;
                                w->step++;
                            }
                            break;
                        case 3:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossDampVel(obj, 0.8f);
                            if (w->timer-- == 0) {
                                BODY_ANIM(w, bkasDanceStep3Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 4:
                            gBossFlags |= 4;
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bkasDanceStep4Anim, 0.0f, 0);
                                w->timer = 120;
                                w->step++;
                            }
                            break;
                        case 5:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.3f, 0);
                            if (w->timer-- == 0) {
                                BODY_ANIM(w, bkasDanceStep5Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 6:
                            gBossFlags |= 4;
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bkasDanceStep6Anim, 0.0f, 0);
                                w->timer = 360;
                                w->step++;
                            }
                            break;
                        case 7:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.3f, 0);
                            if (w->timer-- == 0) {
                                BODY_ANIM(w, bkasDanceStep7Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 8:
                            gBossFlags |= 4;
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
                                gBossFlags &= ~0x800;
                                gBrotherFlags &= ~0x20000;
                                bossClearAction(obj);
                            }
                            break;
                    }
                    break;
                case 8:
                    /* Reset to the middle (gBattleFlags 0x4000000 turns it the other way). */
                    MOVE(w);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    bossStepAnim(w->cape, 0.3f, 0);
                    from.y = w->frame->pos[1] - 1800.0f;
                    switch (w->step) {
                        case 0:
                            gBossFlags |= 4;
                            w->atk->flags |= 2;
                            w->dmg->flags |= 2;
                            TO_LAYER(w, gFrmLayers[2]);
                            gBossFlags |= 0x8000;
                            sndPlay(0x8176);
                            BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
                            bossSetAnimAt(w->cape, bkasCapeIdleAnim, 0xF5, 0.0f);
                            w->cape->f.flags |= 2;
                            w->frame->pos[0] = 0.0f;
                            w->frame->pos[1] = -200.0f;
                            w->frame->pos[2] = -3500.0f;
                            w->frame->rot[0] = 0.0f;
                            w->frame->rot[2] = 0.0f;
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
                            gBossFlags &= ~0x4000;
                            gBossFlags &= ~0x800;
                            w->timer = 60;
                            w->step++;
                            break;
                        case 1:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossStepAnim(w->cape, 0.3f, 0);
                            if (gBattleFlags & 0x4000000) {
                                w->frame->rot[1] = -2.0943952f;
                                w->cape->rot[1] = -2.0943952f;
                            } else {
                                w->frame->rot[1] = 2.0943952f;
                                w->cape->rot[1] = 2.0943952f;
                            }
                            w->frame->pos[1] = -200.0f;
                            if (w->timer-- == 0) {
                                w->step++;
                            }
                            break;
                        case 2:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossStepAnim(w->cape, 0.3f, 0);
                            w->frame->pos[1] = -200.0f;
                            if (gBattleFlags & 0x4000000) {
                                w->frame->rot[1] += DEG;
                                w->cape->rot[1] += DEG;
                                if (w->frame->rot[1] > 0.0f) {
                                    w->frame->rot[1] = 0.0f;
                                    bossClearAction(obj);
                                }
                            } else {
                                w->frame->rot[1] -= DEG;
                                w->cape->rot[1] -= DEG;
                                if (w->frame->rot[1] < 0.0f) {
                                    w->frame->rot[1] = 0.0f;
                                    bossClearAction(obj);
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
                    switch (w->step) {
                        case 0:
                            w->dmg->flags &= ~2;
                            BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
                            bossSetAnimAt(w->cape, bkasCapeIdleAnim, 0xF5, 0.0f);
                            bossDampVel(obj, 0.9f);
                            gBossFlags &= ~0x4000;
                            gBossFlags &= ~0x800;
                            gBossFlags |= 4;
                            gBossFlags |= 0x8000;
                            from.y = w->frame->pos[1] - 1800.0f;
                            w->step++;
                            break;
                        case 1:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossStepAnim(w->cape, 0.3f, 0);
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 2.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 2.0f);
                            bossDampVel(obj, 0.8f);
                            if (!(gBattleFlags & 8)) {
                                w->dmg->flags |= 2;
                                bossClearAction(obj);
                            }
                            break;
                    }
                    break;
                case 11:
                    /* Dizzy after three flinches (bossFlinchCountOver): a dance back to the middle. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d][%f]\n", w->action, w->step, w->timer, w->vel[1]);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    MOVE(w);
                    TURN(w);
                    gBossFlags |= 4;
                    gBossFlags |= 0x1000;
                    HEIGHT_FIX_QUIET(w);
                    switch (w->step) {
                        case 0:
                            BODY_ANIM(w, bkasDanceStep1Anim, 0.0f, 0);
                            gBossFlags |= 0x8000;
                            w->timer = 0;
                            w->step++;
                            break;
                        case 1:
                            bossFlyTowardXZ(obj, 0.0f, -4000.0f, 1.6f, 30.0f);
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bkasDanceStep2Anim, 0.0f, 1);
                                w->step++;
                            }
                            break;
                        case 2:
                            bossFlyTowardXZ(obj, 0.0f, -4000.0f, 1.6f, 30.0f);
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bkasDanceStep3Anim, 0.0f, 1);
                                w->step++;
                            }
                            break;
                        case 3:
                            bossFlyTowardXZ(obj, 0.0f, -4000.0f, 1.6f, 30.0f);
                            if (w->timer == 30) {
                                SOUND(w, 0x281);
                            }
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bkasDanceStep4Anim, 0.0f, 1);
                                w->step++;
                            }
                            break;
                        case 4:
                            bossFlyTowardXZ(obj, 0.0f, -4000.0f, 1.6f, 30.0f);
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bkasDanceStep5Anim, 0.0f, 1);
                                w->step++;
                            }
                            break;
                        case 5:
                            bossFlyTowardXZ(obj, 0.0f, -4000.0f, 1.6f, 30.0f);
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bkasDanceStep6Anim, 0.0f, 1);
                                w->step++;
                            }
                            break;
                        case 6:
                            bossFlyTowardXZ(obj, 0.0f, -4000.0f, 1.6f, 30.0f);
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bkasDanceStep7Anim, 0.0f, 1);
                                w->step++;
                            }
                            break;
                        case 7:
                            bossFlyTowardXZ(obj, -3000.0f, -4000.0f, 1.6f, 30.0f);
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 8:
                            bossStepAnim(w->frame, 0.5f, 0);
                            if (bossFlyTowardXZ(obj, 0.0f, -4000.0f, 1.6f, 30.0f)) {
                                w->step++;
                            }
                            break;
                        case 9:
                            bossStepAnim(w->frame, 0.5f, 0);
                            if (bossDampVel(obj, 0.8f) == 0) {
                                bossClearAction(obj);
                                gBossFlags &= ~0x1000;
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 12:
                    /* The "kabukiB" (type 0) and a shorter dance. */
                    DPRINTF("Sstep/Stimer [%d/%d]\n", w->step, w->timer);
                    gBossFlags |= 4;
                    MOVE(w);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    bossStepAnim(w->cape, 0.3f, 0);
                    switch (w->step) {
                        case 0:
                            TURN(w);
                            TO_LAYER(w, gFrmLayers[2]);
                            if (bossFlyTowardXZ(obj, w->frame->pos[0], -5000.0f, 2.0f, 30.0f)) {
                                w->step++;
                            }
                            break;
                        case 1:
                            bossDampVel(obj, 0.9f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 2.0f);
                            if (bossTurnToward2(w->frame, 0.0f, 0.0f, 2.0f)) {
                                BODY_ANIM(w, bkasDanceStep1Anim, 0.0f, 0);
                                gBrotherFlags |= 0x20000;
                                gBossFlags |= 0x800;
                                w->step++;
                            }
                            break;
                        case 2:
                            bossDampVel(obj, 0.9f);
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bkasDanceStep2Anim, 0.0f, 0);
                                bkasKabukiBCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 0);
                                sndPlay(0x17E);
                                w->timer = 120;
                                w->step++;
                            }
                            break;
                        case 3:
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossDampVel(obj, 0.8f);
                            if (w->timer-- == 0) {
                                BODY_ANIM(w, bkasDanceStep3Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 4:
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bkasDanceStep4Anim, 0.0f, 0);
                                w->timer = 120;
                                w->step++;
                            }
                            break;
                        case 5:
                            bossStepAnim(w->frame, 0.3f, 0);
                            if (w->timer-- == 0) {
                                BODY_ANIM(w, bkasDanceStep5Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 6:
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bkasDanceStep6Anim, 0.0f, 0);
                                w->timer = 120;
                                w->step++;
                            }
                            break;
                        case 7:
                            bossStepAnim(w->frame, 0.3f, 0);
                            if (w->timer-- == 0) {
                                BODY_ANIM(w, bkasDanceStep7Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 8:
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
                                gBossFlags &= ~0x800;
                                gBrotherFlags &= ~0x20000;
                                bossClearAction(obj);
                            }
                            break;
                    }
                    break;
                case 13:
                    /* The vanishing dash: fade out (count, the alpha), come back at one side,
                     * dash across in front of the Impact and back. */
                    MOVE(w);
                    bossStepAnim(w->cape, 0.3f, 0);
                    switch (w->step) {
                        case 0:
                            CAPE_TILT(w);
                            BODY_TILT(w);
                            TURN(w);
                            if (bossFlyTowardXZ(obj, w->frame->pos[0], -5000.0f, 1.0f, 20.0f)) {
                                w->frame->rot[0] = w->frame->rot[2] = 0.0f;
                                w->step++;
                            }
                            break;
                        case 1:
                            gBossFlags |= 4;
                            bossDampVel(obj, 0.9f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 3.0f);
                            if (bossTurnToward2(w->frame, 0.0f, 0.0f, 2.0f)) {
                                w->step++;
                            }
                            break;
                        case 2:
                            gBossFlags |= 4;
                            FACE(w);
                            bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 3.0f);
                            BODY_ANIM(w, bkasWakemiStep1Anim, 0.0f, 0);
                            w->timer = 0;
                            w->step++;
                            break;
                        case 3:
                            gBossFlags |= 4;
                            if (w->timer == 15) {
                                SOUND(w, 0x281);
                            }
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bkasWakemiStep2Anim, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 4:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.5f, 0);
                            if (w->timer > 20) {
                                w->step++;
                            }
                            break;
                        case 5:
                            gBossFlags |= 4;
                            FACE(w);
                            w->frame->rot[0] += DEG;
                            if (w->frame->rot[0] > 0.3926991f) {
                                w->frame->rot[0] = 0.3926991f;
                            }
                            if (w->timer == 25) {
                                SOUND(w, 0x2B8);
                            }
                            if (bossFlyTowardXZ(obj, 200.0f, -2500.0f, 2.0f, 30.0f)) {
                                SOUND(w, 0x2B5);
                                TO_LAYER(w, gFrmLayers[9]);
                                w->count = 0xFF;
                                w->step++;
                            }
                            break;
                        case 6:
                            gBossFlags |= 4;
                            bossDampVel(obj, 0.9f);
                            w->count -= 8;
                            if (w->count < 9) {
                                w->count = 0;
                                if (fadeRand(100) > 50) {
                                    w->frame->pos[0] = 2500.0f;
                                    SOUND(w, 0x2B5);
                                    w->step = 7;
                                } else {
                                    w->frame->pos[0] = -2500.0f;
                                    SOUND(w, 0x2B5);
                                    w->step = 9;
                                }
                            }
                            break;
                        case 7:
                            gBossFlags |= 4;
                            FACE(w);
                            bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);
                            w->count += 8;
                            if (w->count >= 240) {
                                TO_LAYER(w, gFrmLayers[2]);
                                w->step++;
                            }
                            break;
                        case 8:
                            gBossFlags |= 4;
                            FACE(w);
                            bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);
                            if (bossFlyTowardXZ(obj, 800.0f, -600.0f, 2.0f, 30.0f) && w->frame->pos[2] > -850.0f) {
                                beNullattackCreate(0.0f, 0.0f, 0.0f, 0x58);
                                w->step = 11;
                            }
                            break;
                        case 9:
                            gBossFlags |= 4;
                            FACE(w);
                            bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);
                            w->count += 8;
                            if (w->count >= 240) {
                                TO_LAYER(w, gFrmLayers[2]);
                                w->step++;
                            }
                            break;
                        case 10:
                            gBossFlags |= 4;
                            FACE(w);
                            bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);
                            if (bossFlyTowardXZ(obj, -800.0f, -600.0f, 2.0f, 30.0f) && w->frame->pos[2] > -850.0f) {
                                beNullattackCreate(0.0f, 0.0f, 0.0f, 0x58);
                                w->step++;
                            }
                            break;
                        case 11:
                            gBossFlags |= 4;
                            FACE(w);
                            bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);
                            if (bossFlyTowardXZ(obj, w->frame->pos[0], -3000.0f, 2.0f, 30.0f)) {
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 12:
                            gBossFlags |= 4;
                            bossDampVel(obj, 0.9f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 2.0f);
                            if (bossStepAnim(w->frame, 0.5f, 3)) {
                                BODY_ANIM(w, bkasWakemiStep1Anim, 1.0f, 0);
                                w->step++;
                            }
                            break;
                        case 13:
                            gBossFlags |= 4;
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 2.0f);
                            if (w->timer == 25) {
                                SOUND(w, 0x281);
                            }
                            w->frame->rot[0] -= DEG;
                            if (w->frame->rot[0] < 0.0f) {
                                w->frame->rot[0] = 0.0f;
                            }
                            if (bossStepAnim(w->frame, 0.5f, 3)) {
                                BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
                                bossClearAction(obj);
                            }
                            break;
                        case 14:
                            break;
                    }
                    w->timer++;
                    break;
                case 14:
                    /* The entrance demo, with its camera ("Bappcam"): the boss rises out of the
                     * water on its stage. Start skips it (steps 10-12), and so does a lost
                     * battle before. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d][%f]\n", w->action, w->step, w->timer, w->vel[1]);
                    gBattleFlags2 |= 0x200000;
                    if (w->step != 7 && w->step != 8 && w->step != 9 && w->step != 10 && w->step != 11 &&
                        w->step != 12) {
                        if (gGameWork.losses != 0) {
                            for (i = 0; i < 4; i++) {
                                contPakInitRumble(i);
                                contPakRumbleRequestStop(i);
                            }
                            w->step = 10;
                        }
                        if (fadeCountPlayers() == 1) {
                            if (joyGetHeldTime(gCockpitPort, gCockpitBtnStart) == 1) {
                                for (i = 0; i < 4; i++) {
                                    contPakInitRumble(i);
                                    contPakRumbleRequestStop(i);
                                }
                                w->step = 10;
                            }
                        } else if (fadeCountPlayers() >= 2 && (joyGetHeldTime(gCockpitPort, gCockpitBtnStart) == 1 ||
                                                               joyGetHeldTime(gBrotherPort, gBrotherBtnStart) == 1)) {
                            for (i = 0; i < 4; i++) {
                                contPakInitRumble(i);
                                contPakRumbleRequestStop(i);
                            }
                            w->step = 10;
                        }
                    }
                    /* Paper falling on the stage. */
                    if (gGameFrameCount % 15 == 0) {
                        demoPaperCreate(650.0f, 0.0f, -300.0f);
                    }
                    if (gGameFrameCount % 15 == 5) {
                        demoPaperCreate(700.0f, 0.0f, -400.0f);
                    }
                    if (gGameFrameCount % 15 == 10) {
                        demoPaperCreate(750.0f, 0.0f, -500.0f);
                    }
                    switch (w->step) {
                        case 0:
                            gBattleFlags2 |= 0x400;
                            gBattleFlags |= 0x11000000;
                            gBrotherFlags |= 0x10000;
                            tskCreate(obj, bkasBappcamProc, 0x146, NULL);
                            w->vel[0] = w->vel[1] = w->vel[2] = 0.0f;
                            w->frame->rot[0] = 0.0f;
                            w->frame->rot[1] = 0.0f;
                            w->frame->rot[2] = 0.0f;
                            w->cape->rot[0] = 0.0f;
                            w->cape->rot[1] = 0.0f;
                            w->cape->rot[2] = 0.0f;
                            w->frame->pos[0] = 0.0f;
                            w->frame->pos[1] = -2500.0f;
                            w->frame->pos[2] = -3700.0f;
                            BODY_ANIM(w, bkasWakemiStep3Anim, 0.0f, 0);
                            bossSetAnimAt(w->cape, bkasCapeIdleAnim, 0xF5, 0.0f);
                            sndPlay(1);
                            sndPlay(0x1AD);
                            w->timer = 0;
                            w->step++;
                            break;
                        case 1:
                            /* The lifts and the stage, with two flashes. */
                            if (w->timer == 300) {
                                demoLightCreate(-1200.0f, -1320.0f, -3250.0f, 0);
                                demoStageCreate(50.0f, -1000.0f, -3000.0f);
                                sndPlay(0x1B0);
                                fadeSetAll(10.0f, 0.0f, 255.0f, 0xFF, 0xFF, 0x80);
                            }
                            if (w->timer == 310) {
                                fadeSetAll(10.0f, 255.0f, 0.0f, 0xFF, 0xFF, 0xFF);
                            }
                            if (w->timer == 360) {
                                demoLightCreate(1290.0f, -1300.0f, -3750.0f, 1);
                                sndPlay(0x1B0);
                                fadeSetAll(10.0f, 0.0f, 255.0f, 0xFF, 0xFF, 0x80);
                            }
                            if (w->timer == 370) {
                                fadeSetAll(10.0f, 255.0f, 0.0f, 0xFF, 0xFF, 0xFF);
                            }
                            if (w->timer == 449) {
                                sndPlay(0x1B1);
                            }
                            if (w->timer > 450) {
                                w->step++;
                            }
                            break;
                        case 2:
                            /* Rising. */
                            bossStepAnim(w->frame, 0.10f, 0);
                            bossStepAnim(w->cape, 0.2f, 0);
                            w->frame->pos[1] += 5.0f;
                            if (w->frame->pos[1] > -300.0f) {
                                w->frame->pos[1] = -300.0f;
                                BODY_ANIM(w, bkasWakemiStep2Anim, 1.0f, 0);
                                w->step++;
                            }
                            break;
                        case 3:
                            bossStepAnim(w->cape, 0.2f, 0);
                            if (bossStepAnim(w->frame, 0.3f, 3)) {
                                BODY_ANIM(w, bkasWakemiStep1Anim, 1.0f, 0);
                                w->step++;
                            }
                            break;
                        case 4:
                            bossStepAnim(w->cape, 0.2f, 0);
                            if (bossStepAnim(w->frame, 0.3f, 3)) {
                                BODY_ANIM(w, bkasBodyIdleAnim, 1.0f, 0);
                                w->step++;
                            }
                            break;
                        case 5:
                            bossStepAnim(w->cape, 0.2f, 0);
                            if (bossStepAnim(w->frame, 0.3f, 0)) {
                                BODY_ANIM(w, bkasAction3RaiseAnim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 6:
                            bossStepAnim(w->cape, 0.2f, 0);
                            if (w->timer == 1042) {
                                sndPlay(0x1AE);
                            }
                            if (w->timer == 1141) {
                                sndPlay(0x1AF);
                            }
                            if (w->timer == 1180) {
                                sndPlay(0x7F);
                            }
                            if (bossStepAnim(w->frame, 0.2f, 2)) {
                                effectKiranCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 500.0f);
                                sndPlay(0x1B2);
                                w->step++;
                            }
                            break;
                        case 7:
                            bossStepAnim(w->cape, 0.2f, 2);
                            if (w->timer > 1430) {
                                BODY_ANIM(w, bkasAction3RaiseAnim, 1.0f, 0);
                                w->step++;
                            }
                            break;
                        case 8:
                            bossStepAnim(w->cape, 0.2f, 0);
                            if (w->timer == 1497) {
                                sndPlay(0x132);
                            }
                            if (bossStepAnim(w->frame, 0.5f, 3)) {
                                BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 9:
                            bossStepAnim(w->frame, 0.10f, 0);
                            bossStepAnim(w->cape, 0.10f, 0);
                            if (w->timer == 1600) {
                                gBattleFlags2 |= 0x400000;
                                effectKiranCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 500.0f);
                            }
                            if (w->timer > 1780) {
                                tskCreate(obj, bkasWaterProc, 0x146, NULL);
                                tskCreate(obj, bkasTakiProc, 0x146, NULL);
                                w->frame->pos[0] = 0.0f;
                                w->frame->pos[1] = -270.0f;
                                w->frame->pos[2] = -4000.0f;
                                w->frame->rot[0] = w->frame->rot[1] = w->frame->rot[2] = 0.0f;
                                w->vel[0] = 0.0f;
                                w->vel[1] = 0.0f;
                                w->vel[2] = 0.0f;
                                DPRINTF("登場デモ終了\n");
                                gBattleFlags2 &= ~0x400;
                                gBattleFlags2 |= 8;
                                gBattleFlags2 &= ~0x200000;
                                BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
                                bossSetAnimAt(w->cape, bkasCapeIdleAnim, 0xF5, 0.0f);
                                effectKiranCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 500.0f);
                                sndSetReverb(1, 15);
                                bossClearAction(obj);
                            }
                            break;
                        case 10:
                            /* Skipped: close the letterbox over the demo. */
                            fadeCreate(gEffectTask);
                            fadeSetParent(gFramePool10);
                            fadeSetLetterbox(320, 120, 0, 120);
                            sndPlay(10);
                            sndPlay(0x7C);
                            w->step++;
                            break;
                        case 11:
                            tskCreate(obj, bkasWaterProc, 0x146, NULL);
                            tskCreate(obj, bkasTakiProc, 0x146, NULL);
                            gBattleFlags2 |= 0x400000;
                            BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
                            bossSetAnimAt(w->cape, bkasCapeIdleAnim, 0xF5, 0.0f);
                            w->frame->pos[0] = 0.0f;
                            w->frame->pos[1] = -270.0f;
                            w->frame->pos[2] = -4000.0f;
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
                        case 12:
                            bossStepAnim(w->frame, 0.3f, 0);
                            bossStepAnim(w->cape, 0.3f, 0);
                            if (fadeMoveLetterbox(0, 240, 1) == 1) {
                                DPRINTF("登場デモキャンセル終了\n");
                                gBattleFlags2 &= ~0x400;
                                gBattleFlags2 |= 8;
                                gBattleFlags2 &= ~0x200000;
                                effectKiranCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 500.0f);
                                sndSetReverb(1, 15);
                                bossClearAction(obj);
                            }
                            break;
                        case 13:
                            break;
                        case 14:
                            break;
                        case 15:
                            break;
                    }
                    w->timer++;
                    break;
                case 15:
                    /* Defeated: the boss's losing demo, then the battle is won. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d][%f]\n", w->action, w->step, w->timer, w->vel[1]);
                    MOVE(w);
                    gBossPosX = w->frame->pos[0];
                    gBossPosY = w->frame->pos[1];
                    gBossPosZ = w->frame->pos[2];
                    gBossYaw = w->frame->rot[1];
                    FACE(w);
                    gBattleFlags2 |= 0x200000;
                    w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 1.6f;
                    w->cape->f.flags |= 2;
                    DPRINTF("Demo step/timer[%d/%d]\n", w->step, w->timer);
                    switch (w->step) {
                        case 0:
                            fadeCreate(gEffectTask);
                            gBattleFlags2 |= 0x400;
                            gBossFlags |= 0x800;
                            gBattleFlags2 |= 0x1000;
                            gBossFlags &= ~0x10000;
                            gBattleFlags2 |= 0x1000000;
                            sndPlay(9);
                            w->vel[0] = w->vel[1] = w->vel[2] = 0.0f;
                            tskCreate(obj, Bappcam3, 0x146, NULL);
                            w->frame->pos[0] = 2000.0f;
                            gBossPosX = w->frame->pos[0];
                            w->frame->pos[1] = 0.0f;
                            gBossPosY = w->frame->pos[1];
                            w->frame->pos[2] = -1000.0f;
                            gBossPosZ = w->frame->pos[2];
                            BODY_ANIM(w, bkasLosingDemoBodyAnim, 0.0f, 1);
                            bossSetAnimAt(w->cape, bkasLosingDemoCapeAnim, 0xF5, 0.0f);
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
                                                   w->frame->pos[2], 300.0f, 50.0f, 300.0f, 10, 400, 6, 1.5f, 0.10f);
                                effectPompomCreate(w->frame->pos[0] + 200.0f, w->frame->pos[1] + 200.0f,
                                                   w->frame->pos[2], 300.0f, 50.0f, 300.0f, 10, 400, 6, 1.0f, 0.10f);
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
                                                   w->frame->pos[2], 200.0f, 100.0f, 200.0f, 10, 300, 6, 1.0f, 0.10f);
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
                                fadeSetAll(30.0f, 255.0f, 0.0f, 0, 0, 0);
                            }
                            if (w->timer > 1130) {
                                w->step++;
                            }
                            break;
                        case 14:
                            /* The clear pack. */
                            if (fadeCloseLetterbox(50)) {
                                sndClearReverb();
                                clearPackCreate(0.0f, 0.0f, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 15:
                            if (gBattleFlags2 & 0x800000) {
                                gBattleFlags2 &= ~0x800000;
                                w->step++;
                            }
                            break;
                        case 16:
                            if (fadeCloseLetterbox(120)) {
                                gBattleFlags2 &= ~0x1000000;
                                swWorkSet(0xC0);
                                bossBattleWin((u8) gBattleBossId);
                                dmenuQueueDemo(12);
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 16:
                    /* The Impact is down: the boss's victory, then the battle is lost. */
                    gBattleFlags2 |= 0x200000;
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    MOVE(w);
                    bossStepAnim(w->cape, 0.3f, 0);
                    switch (w->step) {
                        case 0:
                            TURN(w);
                            if (bossFlyTowardXZ(obj, w->frame->pos[0], -4000.0f, 2.0f, 30.0f)) {
                                w->step++;
                            }
                            break;
                        case 1:
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 3.0f);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                w->step++;
                            }
                            break;
                        case 2:
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 3.0f);
                            if (bossFlyTowardXZ(obj, 0.0f, -800.0f, 2.0f, 30.0f)) {
                                w->step++;
                            }
                            break;
                        case 3:
                            if (bossDampVel(obj, 0.8f) == 0) {
                                BODY_ANIM(w, bkasOhudaStep3Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 4:
                            if (bossStepAnim(w->frame, 1.0f, 2)) {
                                BODY_ANIM(w, bkasOhudaStep4Anim, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 5:
                            if (w->timer == 30) {
                                sndPlay(0x158);
                                bkasOhudaCreate(0.0f, 0.0f, -400.0f, 3);
                            }
                            if (w->timer == 60) {
                                sndPlay(0x158);
                                bkasOhudaCreate(50.0f, -10.0f, -400.0f, 3);
                            }
                            if (w->timer == 85) {
                                sndPlay(0x158);
                                bkasOhudaCreate(-50.0f, -30.0f, -400.0f, 3);
                            }
                            if (bossStepAnim(w->frame, 0.4f, 2)) {
                                BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
                                fadeSetAll(120.0f, 0.0f, 255.0f, 0xFF, 0xFF, 0xFF);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 6:
                            bossStepAnim(w->frame, 0.2f, 0);
                            if (w->timer > 120) {
                                DPRINTF("負け。\n");
                                sndPlay(7);
                                sndPlay(0x11D);
                                gBattleFlags2 |= 0x4000000;
                                w->step++;
                            }
                            break;
                        case 7:
                            if (fadeCloseLetterbox(120)) {
                                bossBattleLose((u8) gBattleBossId);
                            }
                            break;
                        case 8:
                            break;
                    }
                    w->timer++;
                    break;
                case 17:
                    /* A fan ("Sensu") thrown from the middle of the stage. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d][%f]\n", w->action, w->step, w->timer, w->vel[1]);
                    MOVE(w);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    bossStepAnim(w->cape, 0.3f, 0);
                    switch (w->step) {
                        case 0:
                            TURN(w);
                            if (bossFlyTowardXZ(obj, w->frame->pos[0], -4000.0f, 2.0f, 30.0f)) {
                                w->step++;
                            }
                            break;
                        case 1:
                            gBossFlags |= 4;
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 3.0f);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                w->step++;
                            }
                            break;
                        case 2:
                            gBossFlags |= 4;
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 3.0f);
                            BODY_ANIM(w, bkasSensuStep2Anim, 0.0f, 0);
                            w->timer = 0;
                            w->step++;
                            break;
                        case 3:
                            gBossFlags |= 4;
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 3.0f);
                            if (w->timer == 40) {
                                SOUND(w, 0x281);
                            }
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bkasDanceStep4Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 4:
                            gBossFlags |= 4;
                            if (w->timer == 70) {
                                bossHeadingOffset(gBossYaw, 0.0f, 500.0f, &ox, &oz);
                                bkasSensuCreate(ox + gBossPosX, gBossPosY + 500.0f, oz + gBossPosZ, 0);
                            }
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bkasDanceStep5Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 5:
                            gBossFlags |= 4;
                            if (w->timer == 118) {
                                SOUND(w, 0x133);
                            }
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bkasDanceStep7Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 6:
                            gBossFlags |= 4;
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
                                bossSetAnimAt(w->cape, bkasCapeIdleAnim, 0xF5, 0.0f);
                                bossClearAction(obj);
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 18:
                    /* The vanishing dash, twice: fade out, back at one side, across, out, back at
                     * the other side and across again. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d][%f]\n", w->action, w->step, w->timer, w->vel[1]);
                    MOVE(w);
                    bossStepAnim(w->cape, 0.3f, 0);
                    switch (w->step) {
                        case 0:
                            CAPE_TILT(w);
                            BODY_TILT(w);
                            TURN(w);
                            if (bossFlyTowardXZ(obj, w->frame->pos[0], -5000.0f, 1.0f, 20.0f)) {
                                w->frame->rot[0] = w->frame->rot[2] = 0.0f;
                                w->step++;
                            }
                            break;
                        case 1:
                            gBossFlags |= 4;
                            bossDampVel(obj, 0.9f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 3.0f);
                            if (bossTurnToward2(w->frame, 0.0f, 0.0f, 2.0f)) {
                                w->step++;
                            }
                            break;
                        case 2:
                            gBossFlags |= 4;
                            FACE(w);
                            bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 3.0f);
                            BODY_ANIM(w, bkasWakemiStep1Anim, 0.0f, 0);
                            w->timer = 0;
                            w->step++;
                            break;
                        case 3:
                            gBossFlags |= 4;
                            if (w->timer == 15) {
                                SOUND(w, 0x281);
                            }
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bkasWakemiStep2Anim, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 4:
                            gBossFlags |= 4;
                            bossStepAnim(w->frame, 0.5f, 0);
                            if (w->timer > 20) {
                                w->step++;
                            }
                            break;
                        case 5:
                            gBossFlags |= 4;
                            FACE(w);
                            w->frame->rot[0] += DEG;
                            if (w->frame->rot[0] > 0.3926991f) {
                                w->frame->rot[0] = 0.3926991f;
                            }
                            if (bossFlyTowardXZ(obj, 200.0f, -2500.0f, 2.0f, 30.0f)) {
                                SOUND(w, 0x2B5);
                                TO_LAYER(w, gFrmLayers[9]);
                                w->count = 0xFF;
                                w->step++;
                            }
                            break;
                        case 6:
                            gBossFlags |= 4;
                            bossDampVel(obj, 0.9f);
                            w->count -= 8;
                            if (w->count < 9) {
                                w->count = 0;
                                if (fadeRand(100) > 50) {
                                    w->frame->pos[0] = 2500.0f;
                                    SOUND(w, 0x2B5);
                                    w->step = 7;
                                } else {
                                    w->frame->pos[0] = -2500.0f;
                                    SOUND(w, 0x2B5);
                                    w->step = 11;
                                }
                            }
                            break;
                        case 7:
                            gBossFlags |= 4;
                            FACE(w);
                            bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);
                            w->count += 8;
                            if (w->count >= 240) {
                                SOUND(w, 0x2B5);
                                w->step++;
                            }
                            break;
                        case 8:
                            gBossFlags |= 4;
                            w->count -= 8;
                            if (w->count < 9) {
                                w->count = 0;
                                w->frame->pos[0] = -2500.0f;
                                SOUND(w, 0x2B5);
                                w->step++;
                            }
                            break;
                        case 9:
                            gBossFlags |= 4;
                            FACE(w);
                            bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);
                            w->count += 8;
                            if (w->count >= 240) {
                                TO_LAYER(w, gFrmLayers[2]);
                                w->step++;
                            }
                            break;
                        case 10:
                            gBossFlags |= 4;
                            FACE(w);
                            bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);
                            if (bossFlyTowardXZ(obj, -800.0f, -600.0f, 2.0f, 30.0f) && w->frame->pos[2] > -850.0f) {
                                beNullattackCreate(0.0f, 0.0f, 0.0f, 0x58);
                                w->step = 15;
                            }
                            break;
                        case 11:
                            gBossFlags |= 4;
                            FACE(w);
                            bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);
                            w->count += 8;
                            if (w->count >= 240) {
                                SOUND(w, 0x2B5);
                                w->step++;
                            }
                            break;
                        case 12:
                            gBossFlags |= 4;
                            w->count -= 8;
                            if (w->count < 9) {
                                w->count = 0;
                                w->frame->pos[0] = 2500.0f;
                                SOUND(w, 0x2B5);
                                w->step++;
                            }
                            break;
                        case 13:
                            gBossFlags |= 4;
                            FACE(w);
                            bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);
                            w->count += 8;
                            if (w->count >= 240) {
                                TO_LAYER(w, gFrmLayers[2]);
                                w->step++;
                            }
                            break;
                        case 14:
                            gBossFlags |= 4;
                            FACE(w);
                            bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);
                            if (bossFlyTowardXZ(obj, 800.0f, -600.0f, 2.0f, 30.0f) && w->frame->pos[2] > -850.0f) {
                                beNullattackCreate(0.0f, 0.0f, 0.0f, 0x58);
                                w->step++;
                            }
                            break;
                        case 15:
                            gBossFlags |= 4;
                            FACE(w);
                            bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);
                            if (bossFlyTowardXZ(obj, w->frame->pos[0], -3000.0f, 2.0f, 30.0f)) {
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 16:
                            gBossFlags |= 4;
                            bossDampVel(obj, 0.9f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 2.0f);
                            if (bossStepAnim(w->frame, 0.5f, 3)) {
                                BODY_ANIM(w, bkasWakemiStep1Anim, 1.0f, 0);
                                w->step++;
                            }
                            break;
                        case 17:
                            gBossFlags |= 4;
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 2.0f);
                            if (w->timer == 25) {
                                SOUND(w, 0x281);
                            }
                            w->frame->rot[0] -= DEG;
                            if (w->frame->rot[0] < 0.0f) {
                                w->frame->rot[0] = 0.0f;
                            }
                            if (bossStepAnim(w->frame, 0.5f, 3)) {
                                BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
                                bossClearAction(obj);
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 19:
                    /* Down through the stage: sink under the water, come up in front of the
                     * Impact and throw the paper charms. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d][%f]\n", w->action, w->step, w->timer, w->vel[1]);
                    MOVE(w);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    TURN(w);
                    bossStepAnim(w->cape, 0.3f, 0);
                    switch (w->step) {
                        case 0:
                            if (bossFlyTowardXZ(obj, w->frame->pos[0], -4000.0f, 1.0f, 20.0f)) {
                                BODY_ANIM(w, bkasDanceStep1Anim, 0.0f, 0);
                                w->frame->rot[0] = w->frame->rot[2] = 0.0f;
                                w->count = 0;
                                w->unused59 = 0;
                                w->step++;
                                w->vel[1] = 40.0f;
                            }
                            break;
                        case 1:
                            gBossFlags |= 4;
                            w->dmg->flags &= ~2;
                            w->vel[1] -= 1.5;
                            if (w->frame->pos[1] < -700.0f && w->count == 0) {
                                SOUND(w, 0x14D);
                                w->count = 1;
                            }
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bkasDanceStep2Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 2:
                            gBossFlags |= 4;
                            w->vel[1] -= 1.5;
                            if (w->frame->pos[1] < -700.0f && w->count == 0) {
                                SOUND(w, 0x14D);
                                w->count = 1;
                            }
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bkasDanceStep3Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 3:
                            gBossFlags |= 4;
                            w->vel[1] -= 1.5;
                            if (w->frame->pos[1] < -700.0f && w->count == 0) {
                                SOUND(w, 0x14D);
                                w->count = 1;
                            }
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bkasDanceStep4Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 4:
                            gBossFlags |= 4;
                            w->vel[1] -= 1.5;
                            if (w->frame->pos[1] < -700.0f && w->count == 0) {
                                SOUND(w, 0x14D);
                                w->count = 1;
                            }
                            if (bossStepAnim(w->frame, 0.5f, 2)) {
                                BODY_ANIM(w, bkasDanceStep5Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 5:
                            gBossFlags |= 4;
                            w->vel[1] -= 1.5;
                            if (w->frame->pos[1] < -700.0f && w->count == 0) {
                                SOUND(w, 0x14D);
                                w->count = 1;
                            }
                            if (w->frame->pos[1] < -3000.0f) {
                                w->vel[0] = w->vel[1] = w->vel[2] = 0.0f;
                                w->frame->pos[1] = -3000.0f;
                                BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 6:
                            gBossFlags |= 4;
                            FACE(w);
                            bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);
                            if (w->frame->pos[1] < -700.0f && w->count == 0) {
                                SOUND(w, 0x14D);
                                w->count = 1;
                            }
                            bossStepAnim(w->frame, 0.3f, 0);
                            if (w->timer > 90) {
                                w->frame->pos[0] = 0.0f;
                                w->frame->pos[2] = -1200.0f;
                                w->frame->rot[0] = w->frame->rot[1] = w->frame->rot[2] = 0.0f;
                                w->timer = 0;
                                w->vel[1] = 100.0f;
                                w->step++;
                            }
                            break;
                        case 7:
                            gBossFlags |= 4;
                            FACE(w);
                            bossTurnToDir(w->cape, w->vel[0], w->vel[2], 3.0f);
                            w->vel[1] -= 1.7;
                            bossStepAnim(w->frame, 0.3f, 0);
                            if (w->timer == 30) {
                                SOUND(w, 0x14E);
                            }
                            if (w->frame->pos[1] > -50.0f) {
                                w->dmg->flags |= 2;
                                BODY_ANIM(w, bkasOhudaStep3Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 8:
                            gBossFlags |= 4;
                            w->vel[1] -= 1.0;
                            if (ON_GROUND(w)) {
                                w->frame->pos[1] = at.y + 1800.0f + 0.1f;
                                w->vel[1] = 0.0f;
                            }
                            if (bossStepAnim(w->frame, 1.0f, 2)) {
                                BODY_ANIM(w, bkasOhudaStep4Anim, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 9:
                            gBossFlags |= 4;
                            w->vel[1] -= 1.0;
                            if (ON_GROUND(w)) {
                                w->frame->pos[1] = at.y + 1800.0f + 0.1f;
                                w->vel[1] = 0.0f;
                            }
                            if (w->timer == 30) {
                                bkasOhudaCreate(0.0f, 0.0f, -400.0f, 0);
                            }
                            if (w->timer == 60) {
                                bkasOhudaCreate(50.0f, -10.0f, -400.0f, 1);
                            }
                            if (w->timer == 85) {
                                if (fadeRand(100) > 95) {
                                    bkasOhudaCreate(-50.0f, -30.0f, -400.0f, 3);
                                } else {
                                    bkasOhudaCreate(-50.0f, -30.0f, -400.0f, 2);
                                }
                            }
                            if (bossStepAnim(w->frame, 0.4f, 2)) {
                                BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 10:
                            gBossFlags |= 4;
                            if (w->timer > 30) {
                                bossClearAction(obj);
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 20:
                    /* Three fans ("SensuR"), left, middle and right. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d][%f]\n", w->action, w->step, w->timer, w->vel[1]);
                    MOVE(w);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    bossStepAnim(w->cape, 0.3f, 0);
                    switch (w->step) {
                        case 0:
                            TURN(w);
                            if (bossFlyTowardXZ(obj, w->frame->pos[0], -4000.0f, 2.0f, 30.0f)) {
                                w->step++;
                            }
                            break;
                        case 1:
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 3.0f);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                w->step++;
                            }
                            break;
                        case 2:
                            gBossFlags |= 4;
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 3.0f);
                            BODY_ANIM(w, bkasSensuStep2Anim, 0.0f, 0);
                            w->timer = 0;
                            w->step++;
                            break;
                        case 3:
                            gBossFlags |= 4;
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 3.0f);
                            if (w->timer == 40) {
                                SOUND(w, 0x281);
                            }
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bkasDanceStep4Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 4:
                            gBossFlags |= 4;
                            if (w->timer == 70) {
                                bossHeadingOffset(gBossYaw, 800.0f, 500.0f, &ox, &oz);
                                bkasSensuRCreate(ox + gBossPosX, gBossPosY + 500.0f, oz + gBossPosZ, 0);
                                bossHeadingOffset(gBossYaw, 0.0f, 500.0f, &ox, &oz);
                                bkasSensuRCreate(ox + gBossPosX, gBossPosY + 700.0f, oz + gBossPosZ, 0);
                                bossHeadingOffset(gBossYaw, -800.0f, 500.0f, &ox, &oz);
                                bkasSensuRCreate(ox + gBossPosX, gBossPosY + 500.0f, oz + gBossPosZ, 0);
                            }
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bkasDanceStep5Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 5:
                            gBossFlags |= 4;
                            if (w->timer == 120) {
                                SOUND(w, 0x133);
                            }
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bkasDanceStep7Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 6:
                            gBossFlags |= 4;
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
                                bossSetAnimAt(w->cape, bkasCapeIdleAnim, 0xF5, 0.0f);
                                bossClearAction(obj);
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 21:
                    /* Eight fans ("SensuR", types 0-7) at once. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d][%f]\n", w->action, w->step, w->timer, w->vel[1]);
                    MOVE(w);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    bossStepAnim(w->cape, 0.3f, 0);
                    switch (w->step) {
                        case 0:
                            TURN(w);
                            if (bossFlyTowardXZ(obj, w->frame->pos[0], -4000.0f, 2.0f, 30.0f)) {
                                w->step++;
                            }
                            break;
                        case 1:
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 3.0f);
                            if (bossDampVel(obj, 0.9f) == 0) {
                                w->step++;
                            }
                            break;
                        case 2:
                            gBossFlags |= 4;
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 3.0f);
                            BODY_ANIM(w, bkasSensuStep2Anim, 0.0f, 0);
                            w->timer = 0;
                            w->step++;
                            break;
                        case 3:
                            gBossFlags |= 4;
                            bossTurnToward2(w->frame, 0.0f, 0.0f, 3.0f);
                            bossTurnToward2(w->cape, 0.0f, 0.0f, 3.0f);
                            if (w->timer == 40) {
                                SOUND(w, 0x281);
                            }
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bkasDanceStep4Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 4:
                            gBossFlags |= 4;
                            if (w->timer == 70) {
                                bossHeadingOffset(gBossYaw, 0.0f, 500.0f, &ox, &oz);
                                bkasSensuRCreate(ox + gBossPosX, gBossPosY + 300.0f, oz + gBossPosZ, 0);
                                bkasSensuRCreate(ox + gBossPosX, gBossPosY + 300.0f, oz + gBossPosZ, 1);
                                bkasSensuRCreate(ox + gBossPosX, gBossPosY + 300.0f, oz + gBossPosZ, 2);
                                bkasSensuRCreate(ox + gBossPosX, gBossPosY + 300.0f, oz + gBossPosZ, 3);
                                bkasSensuRCreate(ox + gBossPosX, gBossPosY + 300.0f, oz + gBossPosZ, 4);
                                bkasSensuRCreate(ox + gBossPosX, gBossPosY + 300.0f, oz + gBossPosZ, 5);
                                bkasSensuRCreate(ox + gBossPosX, gBossPosY + 300.0f, oz + gBossPosZ, 6);
                                bkasSensuRCreate(ox + gBossPosX, gBossPosY + 300.0f, oz + gBossPosZ, 7);
                            }
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bkasDanceStep5Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 5:
                            gBossFlags |= 4;
                            if (w->timer == 120) {
                                SOUND(w, 0x133);
                            }
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bkasDanceStep7Anim, 0.0f, 0);
                                w->step++;
                            }
                            break;
                        case 6:
                            gBossFlags |= 4;
                            if (bossStepAnim(w->frame, 0.3f, 2)) {
                                BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
                                bossSetAnimAt(w->cape, bkasCapeIdleAnim, 0xF5, 0.0f);
                                bossClearAction(obj);
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 0x1F:
                    WALK_TO(w, obj, -3000.0f, -6000.0f);
                    break;
                case 0x20:
                    WALK_TO(w, obj, 0.0f, -6000.0f);
                    break;
                case 0x21:
                    WALK_TO(w, obj, 3000.0f, -6000.0f);
                    break;
                case 0x22:
                    WALK_TO(w, obj, -4000.0f, -3000.0f);
                    break;
                case 0x23:
                    WALK_TO(w, obj, 0.0f, -3000.0f);
                    break;
                case 0x24:
                    WALK_TO(w, obj, 4000.0f, -3000.0f);
                    break;
                case 0x25:
                    WALK_TO(w, obj, -1000.0f, -1000.0f);
                    break;
                case 0x26:
                    WALK_TO(w, obj, 0.0f, -1200.0f);
                    break;
                case 0x27:
                    WALK_TO(w, obj, 1000.0f, -1000.0f);
                    break;
                case 0x2B:
                    /* Thrown: fly, splash down, bounce, lie (count counts the damage box back
                     * on), get up; the losing demo when the life is gone. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d][%f]\n", w->action, w->step, w->timer, w->vel[1]);
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    MOVE(w);
                    DPRINTF("Sstep/Stimer [%d/%d]\n", w->step, w->timer);
                    switch (w->step) {
                        case 0:
                            DMG_BOX(w, 0);
                            gBossFlags |= 0x8000;
                            BODY_ANIM(w, bkasThrownDownAnim, 0.0f, 1);
                            w->cape->f.flags &= ~2;
                            w->cape->rot[1] = 0.0f;
                            w->timer = 0;
                            w->step++;
                            break;
                        case 1:
                            w->count--;
                            w->vel[1] -= 4.0;
                            if (w->count == 0) {
                                DPRINTF("damage flag[%d]\n", w->count);
                                w->dmg->flags |= 2;
                            }
                            bossStepAnim(w->frame, 0.3f, 2);
                            gBossFlags &= ~0x8000;
                            if (gGameFrameCount % 10 == 0) {
                                beBpieceCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 8.0f, 10);
                            }
                            from.y = w->frame->pos[1] - 600.0f;
                            if (ON_GROUND(w)) {
                                beShibukiCreate(w->frame->pos[0], w->frame->pos[1] + 500.0f, w->frame->pos[2], 1);
                                SOUND(w, 0x14D);
                                w->frame->pos[1] = at.y + 600.0f + 0.1f;
                                w->frame->rot[0] = w->frame->rot[1] = w->frame->rot[2] = 0.0f;
                                w->vel[1] = 40.0f;
                                w->dmg->flags |= 2;
                                w->step++;
                            }
                            break;
                        case 2:
                            w->vel[1] -= 4.0;
                            bossStepAnim(w->frame, 0.3f, 2);
                            from.y = w->frame->pos[1] - 600.0f;
                            if (ON_GROUND(w)) {
                                beShibukiCreate(w->frame->pos[0], w->frame->pos[1] + 500.0f, w->frame->pos[2], 1);
                                w->frame->pos[1] = at.y + 600.0f + 0.1f;
                                w->step++;
                                w->timer = 90;
                                w->vel[1] = 30.0f;
                            }
                            break;
                        case 3:
                            w->vel[1] -= 4.0;
                            bossStepAnim(w->frame, 0.3f, 2);
                            from.y = w->frame->pos[1] - 600.0f;
                            if (ON_GROUND(w)) {
                                w->frame->pos[1] = at.y + 600.0f + 0.1f;
                                w->vel[1] = 0.0f;
                                w->step = 4;
                                if (fadeRand(100) > 75) {
                                    w->hitWait = 90;
                                } else {
                                    w->hitWait = 150;
                                }
                            }
                            break;
                        case 4:
                            w->vel[0] = 0.0f;
                            w->vel[1] = 0.0f;
                            w->vel[2] = 0.0f;
                            if (w->timer-- == 0) {
                                if (gBossLife > 1) {
                                    beReversalCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                                }
                                w->step++;
                                w->vel[1] = 70.0f;
                            }
                            break;
                        case 5:
                            w->timer++;
                            w->vel[1] -= 1.0;
                            if (w->timer == 10) {
                                SOUND(w, 0x14E);
                            }
                            if (gGameFrameCount % 10 == 0) {
                                beShibukiCreate(w->frame->pos[0], -1200.0f, w->frame->pos[2], 1);
                            }
                            bossStepAnim(w->frame, 0.4f, 3);
                            from.y = w->frame->pos[1] - 1800.0f;
                            if (w->vel[1] < 0.0f && ON_GROUND(w)) {
                                w->frame->pos[1] = at.y + 1800.0f + 0.1f;
                                w->vel[1] = 0.0f;
                                if (gBossLife <= 0) {
                                    DPRINTF("ボス負けデモを開始します。\n");
                                    bossStartAction(obj, 0, 15);
                                } else {
                                    BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
                                    bossSetAnimAt(w->cape, bkasCapeIdleAnim, 0xF5, 0.0f);
                                    bossClearAction(obj);
                                    gBossFlags &= ~1;
                                }
                            }
                            break;
                    }
                    break;
                case 0x2C:
                    /* A flinch from a jab (by the side hit), then back; three flinches dizzy it. */
                    DPRINTF("Sstep/Stimer [%d/%d]\n", w->step, w->timer);
                    gBossPosX = w->frame->pos[0];
                    gBossPosY = w->frame->pos[1];
                    gBossPosZ = w->frame->pos[2];
                    gBossYaw = w->frame->rot[1];
                    switch (w->step) {
                        case 0:
                            BODY_TILT1(w);
                            MOVE(w);
                            w->dmg->ofs[2] = 0;
                            gBossFlags |= 0x8000;
                            w->cape->f.flags |= 2;
                            w->cape->rot[1] = 0.0f;
                            BODY_ANIM(w, bkasKnockedBackAnim1, 0.0f, 1);
                            bossAddFlinchCount(1);
                            w->vel[1] = 0.0f;
                            w->frame->pos[2] -= 100.0f;
                            w->hitWait = 20;
                            w->step++;
                            break;
                        case 1:
                            w->hitWait--;
                            gBossFlags &= ~0x8000;
                            if (w->hitWait == 0) {
                                w->dmg->flags |= 2;
                            }
                            if (bossStepAnim(w->frame, 0.8f, 2)) {
                                BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
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
                            BODY_TILT1(w);
                            MOVE(w);
                            w->dmg->ofs[2] = 0;
                            gBossFlags |= 0x8000;
                            w->cape->f.flags |= 2;
                            w->cape->rot[1] = 0.0f;
                            BODY_ANIM(w, bkasKnockedBackAnim0, 0.0f, 1);
                            bossAddFlinchCount(1);
                            w->frame->pos[2] -= 100.0f;
                            w->vel[1] = 0.0f;
                            w->hitWait = 20;
                            w->step++;
                            break;
                        case 1:
                            w->hitWait--;
                            gBossFlags &= ~0x8000;
                            if (w->hitWait == 0) {
                                w->dmg->flags |= 2;
                            }
                            if (bossStepAnim(w->frame, 0.8f, 2)) {
                                BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
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
                    FACE(w);
                    w->dmg->ofs[2] = 0;
                    gBossFlags &= ~0x800;
                    w->cape->f.flags |= 2;
                    w->cape->rot[1] = 0.0f;
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
                            w->cape->f.flags |= 2;
                            gBossFlags |= 0x8000;
                            BODY_ANIM(w, bkasKnockedBackAnim1, 0.0f, 1);
                            w->step++;
                            break;
                        case 1:
                            gBossFlags &= ~0x8000;
                            bossStepAnim(w->frame, 2.0f, 0);
                            if (bossStepAnim(w->frame, 2.0f, 2)) {
                                BODY_ANIM(w, bkasKnockedBackAnim0, 0.0f, 1);
                                w->step = 3;
                            }
                            break;
                        case 2:
                            bossStepAnim(w->frame, 2.0f, 0);
                            if (bossStepAnim(w->frame, 2.0f, 2)) {
                                BODY_ANIM(w, bkasKnockedBackAnim0, 0.0f, 1);
                                w->step++;
                            }
                            break;
                        case 3:
                            gBattleFlags2 &= ~0x2000;
                            if (gBossLife <= 0) {
                                DPRINTF("ボス死亡しました。\n");
                                angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                                w->vel[0] = sinf(angle) * 40.0f;
                                w->vel[2] = cosf(angle) * 40.0f;
                                w->count = 30;
                                w->action = 0x2B;
                                w->step = 0;
                                w->vel[1] = 120.0f;
                            } else {
                                angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                                w->vel[0] = sinf(angle) * 30.0f;
                                w->vel[2] = cosf(angle) * 30.0f;
                                w->action = 0x2B;
                                w->step = 0;
                                w->vel[1] = 70.0f;
                            }
                            break;
                    }
                    break;
                case 0x2F:
                    /* Kicked down: fly smoking, splash down, bounce, lie, get up. */
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    MOVE(w);
                    switch (w->step) {
                        case 0:
                            DMG_BOX(w, 0);
                            gBossFlags |= 0x8000;
                            BODY_ANIM(w, bkasThrownDownAnim, 0.0f, 1);
                            beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 1);
                            w->cape->f.flags &= ~2;
                            w->frame->rot[0] = w->frame->rot[1] = w->frame->rot[2] = 0.0f;
                            w->timer = 0;
                            w->step++;
                            break;
                        case 1:
                            w->count--;
                            w->vel[1] -= 3.0;
                            gBossFlags &= ~0x8000;
                            if (gGameFrameCount % 10 == 0) {
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 1);
                                beBpieceCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 8.0f, 10);
                            }
                            bossStepAnim(w->frame, 0.5f, 2);
                            from.y = w->frame->pos[1] - 600.0f;
                            if (ON_GROUND(w)) {
                                beShibukiCreate(w->frame->pos[0], w->frame->pos[1] + 500.0f, w->frame->pos[2], 1);
                                if (gBattleFlags2 & 0x10000) {
                                    sndPlay(0x14D);
                                } else {
                                    SOUND(w, 0x14D);
                                }
                                w->frame->pos[1] = at.y + 600.0f + 0.1f;
                                w->vel[1] = 20.0f;
                                w->dmg->flags |= 2;
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 2:
                            w->dmg->flags |= 2;
                            w->vel[1] -= 2.0;
                            if (gGameFrameCount % 10 == 0) {
                                beBpieceCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 8.0f, 10);
                            }
                            from.y = w->frame->pos[1] - 600.0f;
                            if (ON_GROUND(w)) {
                                beShibukiCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1);
                                w->frame->pos[1] = at.y + 600.0f + 0.1f;
                                w->vel[1] = 30.0f;
                            }
                            if (w->timer > 40) {
                                w->timer = 0;
                                w->step++;
                            }
                            break;
                        case 3:
                            bossDampVel(obj, 0.8f);
                            if (gGameFrameCount % 10 == 0) {
                                beShibukiCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1);
                            }
                            if (w->timer > 30) {
                                if (gBossLife > 1) {
                                    beReversalCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                                }
                                BODY_ANIM(w, bkasThrownDownAnim, 1.0f, 1);
                                w->frame->rot[0] = w->frame->rot[1] = w->frame->rot[2] = 0.0f;
                                w->vel[1] = 70.0f;
                                w->step++;
                            }
                            break;
                        case 4:
                            w->vel[1] -= 1.0;
                            if (w->timer == 40) {
                                SOUND(w, 0x14E);
                            }
                            if (gGameFrameCount % 10 == 0) {
                                beShibukiCreate(w->frame->pos[0], -1200.0f, w->frame->pos[2], 1);
                            }
                            bossStepAnim(w->frame, 0.4f, 3);
                            from.y = w->frame->pos[1] - 1800.0f;
                            if (w->vel[1] < 0.0f && ON_GROUND(w)) {
                                w->frame->pos[1] = at.y + 1800.0f + 0.1f;
                                w->vel[1] = 0.0f;
                                if (gBossLife <= 0) {
                                    DPRINTF("ボス負けデモを開始します。\n");
                                    bossStartAction(obj, 0, 15);
                                } else {
                                    BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
                                    bossSetAnimAt(w->cape, bkasCapeIdleAnim, 0xF5, 0.0f);
                                    bossClearAction(obj);
                                    gBossFlags &= ~1;
                                }
                            }
                            break;
                    }
                    w->timer++;
                    break;
                case 0x30:
                    /* Hit by the cannon: shaken in place, then knocked down. */
                    DPRINTF("Mstep/Sstep/Stimer [%d/%d/%d][%f]\n", w->action, w->step, w->timer, w->vel[1]);
                    gBossPosX = w->frame->pos[0];
                    gBossPosY = w->frame->pos[1];
                    gBossPosZ = w->frame->pos[2];
                    gBossYaw = w->frame->rot[1];
                    FACE(w);
                    bossHeadingOffset(gCockpitHeadYaw, 0.0f, 1000.0f, &hx, &hz);
                    gBossFlags &= ~0x4000;
                    gBossFlags &= ~0x800;
                    DPRINTF("Sstep/Stimer [%d/%d]\n", w->step, w->timer);
                    switch (w->step) {
                        case 0:
                            w->vel[0] = w->vel[1] = w->vel[2] = 0.0f;
                            w->frame->rot[0] = w->frame->rot[1] = w->frame->rot[2] = 0.0f;
                            w->cape->f.flags |= 2;
                            w->cape->rot[1] = 0.0f;
                            DMG_BOX(w, 0);
                            gBossFlags |= 0x8000;
                            if (w->frame->pos[0] > 0.0f) {
                                BODY_ANIM(w, bkasKnockedBackAnim0, 0.0f, 1);
                            } else {
                                BODY_ANIM(w, bkasKnockedBackAnim1, 0.0f, 1);
                            }
                            w->timer = 240;
                            w->step = 1;
                            break;
                        case 1:
                            gBossFlags &= ~0x8000;
                            w->dmg->flags |= 2;
                            bossStepAnim(w->frame, 3.0f, 0);
                            if (w->timer % 10 >= 5) {
                                w->frame->pos[1] += 20.0f;
                            } else {
                                w->frame->pos[1] -= 20.0f;
                            }
                            if (w->timer-- == 0) {
                                beReversalCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 3.0f);
                                angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                                w->vel[0] = sinf(angle) * 50.0f;
                                w->vel[2] = cosf(angle) * 50.0f;
                                w->action = 0x2F;
                                w->step = 0;
                                w->vel[1] = 35.0f;
                            }
                            break;
                        case 2:
                            break;
                    }
                    break;
                case 0x31:
                    /* Thrown spinning (not started in this file): tumble, splash down, bounce,
                     * lie, get up. */
                    CAPE_TILT(w);
                    BODY_TILT(w);
                    MOVE(w);
                    TURN(w);
                    switch (w->step) {
                        case 0:
                            BODY_TILT1(w);
                            MOVE(w);
                            DMG_BOX(w, 0);
                            w->dmg->flags &= ~2;
                            gBossFlags |= 0x8000;
                            beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 1);
                            BODY_ANIM(w, bkasThrownDownAnim, 0.0f, 1);
                            w->frame->rot[0] = w->frame->rot[1] = w->frame->rot[2] = 0.0f;
                            w->step++;
                            break;
                        case 1:
                            MOVE(w);
                            gBossFlags &= ~0x8000;
                            w->frame->rot[0] += 0.5235987901164023;
                            w->frame->rot[1] += 0.6981317201552031;
                            w->frame->rot[2] += 0.872664650194004;
                            w->count--;
                            w->vel[1] -= 3.0;
                            bossStepAnim(w->frame, 0.3f, 2);
                            if (gGameFrameCount % 10 == 0) {
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 1);
                                beBpieceCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 8.0f, 10);
                            }
                            from.y = w->frame->pos[1] - 600.0f;
                            if (ON_GROUND(w)) {
                                w->frame->rot[0] = w->frame->rot[1] = w->frame->rot[2] = 0.0f;
                                SOUND(w, 0x14D);
                                w->frame->pos[1] = at.y + 600.0f + 0.1f;
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 0);
                                w->vel[1] = 40.0f;
                                w->dmg->flags |= 2;
                                w->step++;
                            }
                            break;
                        case 2:
                            BODY_TILT1(w);
                            MOVE(w);
                            w->vel[1] -= 4.0;
                            bossStepAnim(w->frame, 0.3f, 2);
                            from.y = w->frame->pos[1] - 600.0f;
                            if (ON_GROUND(w)) {
                                w->dmg->flags |= 2;
                                BODY_ANIM(w, bkasThrownDownAnim, 1.0f, 1);
                                w->frame->pos[1] = at.y + 600.0f + 0.1f;
                                w->step = 3;
                                w->timer = 90;
                                w->vel[1] = 30.0f;
                            }
                            break;
                        case 3:
                            BODY_TILT1(w);
                            MOVE(w);
                            w->vel[1] -= 4.0;
                            bossStepAnim(w->frame, 0.3f, 2);
                            from.y = w->frame->pos[1] - 600.0f;
                            if (ON_GROUND(w)) {
                                /* sic: the texture animation from file 0xF4 */
                                bossSetAnimAt(w->frame, bkasThrownDownAnim, 0xF5, 1.0f);
                                fadeAnimStart(&w->anim, bkasBodyAnim, 1, &w->frame->segs[3],
                                              floadGetSetFile(0xF4, 0));
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 0);
                                w->frame->pos[1] = at.y + 600.0f + 0.1f;
                                w->vel[1] = 0.0f;
                                w->step = 4;
                                w->timer = 90;
                            }
                            break;
                        case 4:
                            MOVE(w);
                            BODY_TILT1(w);
                            w->vel[2] = w->vel[1] = w->vel[0] = 0.0f;
                            if (w->timer % 10 == 0) {
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 400.0f, 0);
                            }
                            if (w->timer-- == 0) {
                                if (gBossLife > 1) {
                                    beReversalCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                                }
                                w->step = 5;
                                w->vel[1] = 70.0f;
                            }
                            break;
                        case 5:
                            MOVE(w);
                            w->timer++;
                            w->vel[1] -= 1.0;
                            if (w->timer == 30) {
                                SOUND(w, 0x14E);
                            }
                            if (gGameFrameCount % 10 == 0) {
                                beShibukiCreate(w->frame->pos[0], -1200.0f, w->frame->pos[2], 1);
                            }
                            bossStepAnim(w->frame, 0.4f, 3);
                            from.y = w->frame->pos[1] - 1800.0f;
                            if (w->vel[1] < 0.0f && ON_GROUND(w)) {
                                w->frame->pos[1] = at.y + 1800.0f + 0.1f;
                                w->vel[1] = 0.0f;
                                if (gBossLife <= 0) {
                                    DPRINTF("ボス負けデモを開始します。\n");
                                    bossStartAction(obj, 0, 15);
                                } else {
                                    BODY_ANIM(w, bkasBodyIdleAnim, 0.0f, 0);
                                    bossSetAnimAt(w->cape, bkasCapeIdleAnim, 0xF5, 0.0f);
                                    bossClearAction(obj);
                                    gBossFlags &= ~1;
                                }
                                DPRINTF("dead:ボスはん,しになはれ[%d / %d]\n", gBossLife, gBossLifeMax);
                            }
                            break;
                    }
                    break;
                case 0x37:
                    break;
            }
            break;
        case 3:
            /* Before the draw: keep the boss in the arena, colour it and put its shadow down. */
            gBossPosX = w->frame->pos[0];
            gBossPosY = w->frame->pos[1];
            gBossPosZ = w->frame->pos[2];
            gBossYaw = w->frame->rot[1];
            if (w->action == 0x13 || w->action == 0x2B || w->action == 0x2F || w->action == 0x30 ||
                w->action == 0x31 || w->action == 14) {
                /* Falling or thrown: no height limit. */
                if (w->frame->pos[0] > 5500.0f) {
                    DPRINTF("BOSSLIMIT : X+\n");
                    w->frame->pos[0] = 5500.0f;
                }
                if (w->frame->pos[0] < -5500.0f) {
                    DPRINTF("BOSSLIMIT : X-\n");
                    w->frame->pos[0] = -5500.0f;
                }
                if (w->frame->pos[2] > -800.0f) {
                    DPRINTF("BOSSLIMIT : Z+\n");
                    w->frame->pos[2] = -800.0f;
                }
                if (w->frame->pos[2] < -8000.0f) {
                    DPRINTF("BOSSLIMIT : Z-\n");
                    w->frame->pos[2] = -8000.0f;
                }
            } else {
                if (w->frame->pos[0] > 5500.0f) {
                    DPRINTF("BOSSLIMIT : X+\n");
                    w->frame->pos[0] = 5500.0f;
                }
                if (w->frame->pos[0] < -5500.0f) {
                    DPRINTF("BOSSLIMIT : X-\n");
                    w->frame->pos[0] = -5500.0f;
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
                if (w->frame->pos[2] < -8000.0f) {
                    DPRINTF("BOSSLIMIT : Z-\n");
                    w->frame->pos[2] = -8000.0f;
                }
            }
            if ((w->action == 13 && w->step == 6) || (w->action == 13 && w->step == 7) ||
                (w->action == 13 && w->step == 9) || (w->action == 0x12 && w->step == 6) ||
                (w->action == 0x12 && w->step == 7) || (w->action == 0x12 && w->step == 8) ||
                (w->action == 0x12 && w->step == 9) || (w->action == 0x12 && w->step == 11) ||
                (w->action == 0x12 && w->step == 12) || (w->action == 0x12 && w->step == 13)) {
                /* Vanishing and coming back in the dash. */
                w->frame->dl = bossXluEnvColorDL(0xFF, 0xFF, 0xFF, w->count);
                w->cape->dl = bossXluEnvColorDL(0xFF, 0xFF, 0xFF, w->count);
            } else if (gBossFlags & 0x4000) {
                /* The yellow glow. */
                w->glow += 3;
                if (w->glow > 192) {
                    w->glow = 192;
                }
                w->frame->dl = bossFogEnvColorDL(0xFF, 0xFF, 0, w->glow, 0, 0, 0, 0x80, 0x3DE);
                w->cape->dl = bossFogEnvColorDL(0xFF, 0xFF, 0, w->glow, 0, 0, 0, 0x80, 0x3DE);
            } else if (gBossFlags & 0x100) {
                /* A white flash. */
                w->frame->dl = bossFogEnvColorDL(0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0x80, 0x3DE);
                w->cape->dl = bossFogEnvColorDL(0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0x80, 0x3DE);
                gBossFlags &= ~0x100;
            } else if (w->glow != 0) {
                w->glow -= 3;
                if (w->glow <= 0) {
                    w->glow = 0;
                }
                w->frame->dl = bossFogEnvColorDL(0, 0, 0, w->glow, 0, 0, 0, 0x80, 0x3DE);
                w->cape->dl = bossFogEnvColorDL(0, 0, 0, w->glow, 0, 0, 0, 0x80, 0x3DE);
            } else {
                w->frame->dl = bossFogEnvColorDL(0, 0, 0, 0, 0, 0, 0, 0x80, 0x3DE);
                w->cape->dl = bossFogEnvColorDL(0, 0, 0, 0, 0, 0, 0, 0x80, 0x3DE);
            }
            if (w->action == 14 || w->action == 0x2E || w->action == 0x30) {
                w->frame->dl = bossLitTexEnvColorDL(0, 0, 0, 0);
                w->cape->dl = bossLitTexEnvColorDL(0, 0, 0, 0);
            }
            shadowPut(w->shadow, w->frame->pos[0], w->frame->pos[1] <= -600.0f ? -1000.0f : w->frame->pos[1] - 400.0f,
                      w->frame->pos[2], 160.0f, NULL);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
