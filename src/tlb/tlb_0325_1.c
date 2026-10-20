#include "tlb.h"
#include "tlb/tlb_0325.h"

/* The boss's work (at obj + 0x20), named from this file's use. */
typedef struct BossW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ s32 goal[3]; /* a place to move to (bfriBmoveX) */
    /* 0x1C */ f32 count; /* +1 every step */
    /* 0x20 */ ACollFrame *atk; /* its attack box (made and freed by bfriProc, tlb_0325_0) */
    /* 0x24 */ ACollFrame *link; /* its damage box */
    /* 0x28 */ Anim anim;
    /* 0x38 */ Obj *cam; /* "dracam", the opening camera's (tlb_0325_3) */
    /* 0x3C */ u8 pad3C[8];
    /* 0x44 */ CamView *view; /* tlb_0325_3's */
    /* 0x48 */ s32 invTimer; /* steps of invincibility left after a hit; 0 re-arms the damage box */
    /* 0x4C */ u8 pad4C[8];
    /* 0x54 */ f32 speed;
    /* 0x58 */ u8 action;
    /* 0x59 */ u8 endPattern; /* 1: end the pattern now (set as each hit reaction ends) */
    /* 0x5A */ u8 state;
    /* 0x5C */ s16 time;
} BossW;

extern AnimFrame bfriTexAnim[]; /* a texture animation (file 0xFA) */
/* The file's .data (0x080179A0, IDO_NOTES 343/349), in the game's order: these first; then
 * bfriHit's static (bfriDizzyHits, 0x080179B0); then bfriAttack3Steps and
 * bfriAttack3Unused, defined after that function; then bfriBachiAtari's static; then the rest,
 * defined after that function. */
s32 bfriNewAction = 1; /* 1: cleared by the boss's steps */
s32 bfriCloudsGather = 1;
s32 bfriCannonStun = 1;
s32 bfriCannonHitLast = 1;
extern u8 bfriNormalAnim[]; /* a model animation (file 0xFB) */

/* This file calls its own and its neighbours' functions far, through t9. */
s32 bfriSway(Obj *obj);
#define bfriBank (*bfriBank)
#define bfriSway (*bfriSway)

/* libultra's, through their addresses (code at 0x08000000 cannot jal them). */
#define sinf (*sinf)
#define cosf (*cosf)

extern u8 bfriStraightHitAnim[]; /* a model animation (file 0xFB) */
extern u8 bfriJabHitNegXAnim[]; /* a model animation (file 0xFB) */
extern s32 bfriCloudsGather;
extern s32 bfriCloudsRun;
extern f32 bfriRingRadius;
extern s32 bfriBarrierState;
extern s32 bfriCannonHitLast;
extern s32 bfriFightOver;
extern s32 bfriEffectsEnd;
extern s32 bfriBitsHidden;
extern s32 bfriCannonStun;
extern s32 bfriAttack3Steps; /* steps in the attack's third state */
extern s32 bfriAttack3Unused;
extern s32 bfriAttackSounded; /* 1: this attack's first sound has played */
extern s32 bfriAttackSoundedSet; /* the same, as the attack sets it (undefined_syms.txt) */
extern s32 bfriLongAttackSteps[]; /* step counts, read in turn by the long attack */
extern s32 bfriPuroBombBlowing;
extern s32 bfriTornadoUnusedFlag;
extern s32 bfriTornadoAlpha;
extern s32 bfriPropellerFull;
extern u8 bfriSenpuuCrashState0Anim[]; /* a model animation (file 0xFB) */
extern u8 bfriSenpuuCrashState1Anim[]; /* a model animation (file 0xFB) */
extern u8 bfriSenpuuCrashState2Anim[]; /* a model animation (file 0xFB) */
extern s32 bfriBitsAlive;
extern s32 bfriBarrierSpark;
extern s32 bfriBarrierSpark2;
extern s32 bfriBitsRemake;
#define bfriBariaMake (*bfriBariaMake)
extern u8 bfriElectricShotStep1Anim[]; /* a model animation (file 0xFB) */
#define bfriRumble (*bfriRumble)
extern u8 bfriTurnSide; /* 0: the boss turns one way, 1: the other */
extern f32 bfriRingAngle; /* an angle, turned 12 degrees a step back */
extern u8 bfriHundredPunchHitAnim[]; /* a model animation (file 0xFB) */

#define bfriTurnTo (*bfriTurnTo)
#define bfriRand (*bfriRand)
extern s32 bfriPlaces1[][2]; /* places (x, z) the boss moves to */
extern s32 bfriPlaces2[][2]; /* places (x, z) the boss moves to */
extern s32 bfriPlaces3[][2]; /* places (x, z) the boss moves to */
#define bfriBmoveX (*bfriBmoveX)
#define bfriBmoveXFar (*bfriBmoveXFar)
#define bfriDdmybombCreate (*bfriDdmybombCreate)

#define bfriFzanzouMake (*bfriFzanzouMake)
extern CamView *gBattleCamView; /* the battle's camera (beffect.c) */
#define bfriSunaEfMake (*bfriSunaEfMake)
#define bfriKoke2EfMake (*bfriKoke2EfMake)

extern s32 bfriAttackAnimFlip; /* which of two attack animations, flipped each time */
extern s32 bfriAttackAnimFlipSet; /* the two, as the flips store them (undefined_syms.txt) */
extern s32 bfriAttack2AnimFlipSet;
extern s32 bfriAttack2Steps; /* steps in that attack's second state */
extern u8 bfriBachiRanbuFlip1Anim[]; /* a model animation (file 0xFB) */
extern u8 bfriBachiRanbuFlip0Anim[]; /* a model animation (file 0xFB) */
extern u8 bfriBachiAtariFlip1Anim[]; /* a model animation (file 0xFB) */
extern u8 bfriElectricShotAnim[]; /* a model animation (file 0xFB) */
extern u8 bfriElectricShotStep2Anim[]; /* a model animation (file 0xFB) */
extern u8 bfriBachiAtariStep1Flip1Anim[]; /* a model animation (file 0xFB) */
extern u8 bfriBachiAtariFlip0Anim[]; /* a model animation (file 0xFB) */
extern u8 bfriBachiAtariStep1Flip0Anim[]; /* a model animation (file 0xFB) */
extern u8 bfriSuperHeadbuttAnim[]; /* a model animation (file 0xFB) */
extern u8 bfriSuperHeadbuttStep1Anim[]; /* a model animation (file 0xFB) */

/* The boss's hits ("風雷神", the wind-and-thunder gods): its damage box takes the Impact's vulcan
 * (15), the other Impact's vulcan (16), the jab (1), the straight (3), the hundred-fist punch
 * (5), the kick (8), the brother attack (0xCA) and the cannon (11). Its prints use the named
 * strings defined at the end of this file (a literal would put a second copy in .rodata).
 * Matched 2026-10-12: bfriDizzyHits is a function static (a lui per access; the game's word at
 * 0x080179B0); the "dct" print in an else; cases 15, 16 and 11 test the life with
 * "if (gBossLife <= 0) break;" (the early exits then jump straight to the join with the kind
 * still in a0); the rand results as call-argument temps. */
void bfriHit(Obj *obj) {
    static s32 bfriDizzyHits = 1; /* hits taken in action 0x17 (dizzy); 5 knock it down */
    BossW *w = FX(obj);
    s32 pad[11];

    if (--w->invTimer < 0) {
        w->invTimer = 0;
        w->link->flags |= 2;
    } else {
        PRINTF("dct---->%d\n", w->invTimer);
    }
    if (acollDmgWasHit(w->link) && bfriBarrierState != 4 && w->invTimer == 0) {
        PRINTF("HIT!!\n");
        switch (w->link->hitKind) {
            case 15:
                PRINTF("風雷神はバルカンに当たりました。[%d]\n", gBossLife);
                if (gBossLife <= 0) {
                    break;
                }
                if (!(gBossFlags & 4)) {
                    SoundLocation2(0x22B, gBattleCamView, w->link->hitPos[0], w->link->hitPos[1],
                                   w->link->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                    bossDamage(3, 15, 1);
                    itexFlash(w->link->hitPos[0], w->link->hitPos[1], w->link->hitPos[2], 1.0f);
                }
                if (gBossLife <= 0) {
                    w->action = 0x18;
                    w->time = 800;
                    w->state = 0;
                    bfriCloudsGather = 1;
                    bfriEffectsEnd = 1;
                    w->endPattern = 0;
                    gBossLife = 0;
                    w->link->flags &= ~2;
                }
                break;
            case 16:
                PRINTF("風雷神は外部バルカンに当たりました。\n");
                if (gBossLife <= 0) {
                    break;
                }
                if (gBossFlags & 4) {
                    itexFlash(w->link->hitPos[0], w->link->hitPos[1], w->link->hitPos[2], 1.0f);
                } else {
                    SoundLocation2(0x22B, gBattleCamView, w->link->hitPos[0], w->link->hitPos[1],
                                   w->link->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                    bossDamage(3, 15, 1);
                }
                bossDamage(1, 16, 1);
                if (gBossLife <= 0) {
                    w->action = 0x18;
                    w->time = 800;
                    w->state = 0;
                    bfriCloudsGather = 1;
                    bfriEffectsEnd = 1;
                    w->endPattern = 0;
                    gBossLife = 0;
                    w->link->flags &= ~2;
                }
                break;
            case 1:
                PRINTF("風雷神はジャブに当たりました。\n");
                bossDamage(20, 1, 1);
                w->endPattern = 0;
                w->link->flags &= ~2;
                w->invTimer = 16;
                SoundLocation2(0x27C, gBattleCamView, w->link->hitPos[0], w->link->hitPos[1], w->link->hitPos[2],
                               300.0f, 15000.0f, gBattleCamView->fovy);
                if (w->frame->pos[0] < w->link->hitPos[0]) {
                    w->action = 0x13;
                } else {
                    w->action = 0x12;
                }
                w->time = 800;
                w->state = 0;
                if (gBossLife <= 0) {
                    w->action = 0x18;
                    bfriCloudsGather = 1;
                    bfriEffectsEnd = 1;
                    gBossLife = 0;
                    w->link->flags &= ~2;
                }
                break;
            case 3:
                PRINTF("風雷神はストレートに当たりました。\n");
                w->endPattern = 0;
                bossDamage(50, 3, 2);
                SoundLocation2(0x27D, gBattleCamView, w->link->hitPos[0], w->link->hitPos[1], w->link->hitPos[2],
                               300.0f, 15000.0f, gBattleCamView->fovy);
                w->link->flags &= ~2;
                w->invTimer = 30;
                w->time = 800;
                w->state = 0;
                if (w->action == 0x17) {
                    bfriDizzyHits++;
                }
                if (bfriDizzyHits >= 5) {
                    bfriDizzyHits = 0;
                    w->action = 0x19;
                } else {
                    w->action = 0x11;
                }
                break;
            case 5:
                PRINTF("*********  風雷神は百烈パンチに当たりました。**********\n");
                bossDamage(10, 5, 1);
                w->action = 0x14;
                w->time = 800;
                w->state = 0;
                gBattleFlags2 |= 0x2000;
                beHitmarkCreate(w->link->hitPos[0] + fadeRandRange(-200, 200),
                                w->link->hitPos[1] + fadeRandRange(-200, 200),
                                w->link->hitPos[2] + fadeRandRange(-200, 200), 3.0f);
                beHitmarkCreate(w->link->hitPos[0] + fadeRandRange(-200, 200),
                                w->link->hitPos[1] + fadeRandRange(-200, 200),
                                w->link->hitPos[2] + fadeRandRange(-200, 200), 3.0f);
                beHitringCreate(w->link->hitPos[0] + fadeRandRange(-200, 200),
                                w->link->hitPos[1] + fadeRandRange(-200, 200),
                                w->link->hitPos[2] + fadeRandRange(-200, 200), 1.0f, 0);
                if (gBattleSndToggles & 2) {
                    sndPlay(0x50E);
                    gBattleSndToggles &= ~2;
                } else {
                    sndPlay(0x50F);
                    gBattleSndToggles |= 2;
                }
                break;
            case 8:
                PRINTF("風雷神はキックに当たりました。\n");
                w->endPattern = 0;
                beHitmarkCreate(w->link->hitPos[0], w->link->hitPos[1], w->link->hitPos[2], 5.0f);
                bossDamage(80, 8, 3);
                itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 10.0f);
                sndPlay(0x102);
                w->link->flags &= ~2;
                w->invTimer = 30;
                if (w->action == 0x17) {
                    bfriDizzyHits++;
                }
                if (bfriDizzyHits >= 5) {
                    bfriDizzyHits = 0;
                    w->action = 0x19;
                } else {
                    w->action = 0x15;
                }
                w->time = 800;
                w->state = 0;
                break;
            case 0xCA:
                PRINTF("風雷神はブラザー攻撃に当たりました。\n");
                if (gBossLife > 0) {
                    w->endPattern = 0;
                    beHitmarkCreate(w->link->hitPos[0], w->link->hitPos[1], w->link->hitPos[2], 5.0f);
                    bossDamage(0, 0xCA, 0);
                    SoundLocation2(0x27D, gBattleCamView, w->link->hitPos[0], w->link->hitPos[1],
                                   w->link->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                    w->link->flags &= ~2;
                    w->invTimer = 20;
                    w->action = 0x17;
                    w->time = 800;
                    w->state = 0;
                }
                break;
            case 11:
                PRINTF("風雷神はんが砲に当たりました。\n");
                w->endPattern = 0;
                if (gBossLife <= 0) {
                    break;
                }
                beHitmarkCreate(w->link->hitPos[0], w->link->hitPos[1], w->link->hitPos[2], 5.0f);
                if (gBattleFlags2 & 0x40000) {
                    bossDamage(800, 11, 0);
                } else {
                    bossDamage(400, 11, 0);
                }
                w->link->flags &= ~2;
                w->invTimer = 30;
                w->time = 800;
                w->action = 0x16;
                w->state = 0;
                bfriCannonHitLast = 1;
                bfriCannonStun = 1;
                break;
        }
        if (w->link->hitKind != 11) {
            bfriCannonHitLast = 0;
        }
        if (gBossLife <= 0) {
            bfriFightOver = 1;
        }
    }
}

/* .data after bfriHit's static (0x080179B4, 0x080179B8) */
s32 bfriAttack3Steps = 1; /* steps in the attack's third state */
s32 bfriAttack3Unused = 1;

s32 bfriNormal(Obj *obj) {
    BossW *w = FX(obj);
    s32 pad[10];

    PRINTF("ノーマル\n");
    PRINTF("ゆらゆら....\n");
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    bfriMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    bfriBank(obj, 0.3f);
    w->count += 2.0f;
    if (w->frame->pos[1] < sinf(w->count / 180.0f * 3.141592741f) * 100.0f + -200.0f + -6.0f) {
        w->frame->pos[1] += 3.0f;
    }
    if (w->frame->pos[1] > sinf(w->count / 180.0f * 3.141592741f) * 100.0f + -200.0f + 6.0f) {
        w->frame->pos[1] -= 3.0f;
    }
    w->frame->rot[2] *= 0.94;
    w->frame->rot[2] *= 0.94;
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    switch (w->state) {
        case 0:
            w->time = 350;
            bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            w->state = 1;
            w->speed = 0.0f;
            w->link->flags |= 2;
            break;
        case 1:
            if (bossDampVel(obj, 0.97f) == 0) {
                w->state = 2;
                w->time = 300;
            }
            bfriTurnTo(obj, 0.0f, 0.0f, 3.0f);
            bossStepAnim(w->frame, 0.32f, 0);
            break;
        case 2:
            if (bfriTurnTo(obj, 0.0f, 0.0f, 3.0f)) {
                w->state = 3;
                w->time = 90;
            }
            bossStepAnim(w->frame, 0.32f, 0);
            break;
        case 3:
            bossStepAnim(w->frame, 0.32f, 0);
            break;
    }
    return 0;
}

s32 bfriNormalSwayLong(Obj *obj) {
    BossW *w = FX(obj);

    PRINTF("ノーマルゆらゆら\n");
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    bfriMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    bfriBank(obj, 0.3f);
    w->count += 1.0f;
    switch (w->state) {
        case 0:
            bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            w->state = 1;
            w->time = 210;
            w->speed = 0.0f;
            w->link->flags |= 2;
            break;
        case 1:
            bfriSway(obj);
            break;
    }
    bossStepAnim(w->frame, 0.64f, 0);
    return 0;
}

s32 bfriNormalSwayMid(Obj *obj) {
    BossW *w = FX(obj);

    PRINTF("ノーマルゆらゆら\n");
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    bfriMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    bfriBank(obj, 0.3f);
    w->count += 1.0f;
    switch (w->state) {
        case 0:
            bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            w->state = 1;
            w->time = 100;
            w->speed = 0.0f;
            w->link->flags |= 2;
            break;
        case 1:
            bfriSway(obj);
            break;
    }
    bossStepAnim(w->frame, 0.64f, 0);
    return 0;
}

s32 bfriNormalSwayShort(Obj *obj) {
    BossW *w = FX(obj);

    PRINTF("ノーマルゆらゆら\n");
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    bfriMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    bfriBank(obj, 0.3f);
    w->count += 1.0f;
    switch (w->state) {
        case 0:
            bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            w->state = 1;
            w->time = 35;
            w->speed = 0.0f;
            w->link->flags |= 2;
            break;
        case 1:
            bfriSway(obj);
            break;
    }
    bossStepAnim(w->frame, 0.64f, 0);
    return 0;
}

s32 bfriNeutral(Obj *obj) {
    BossW *w = FX(obj);
    s32 pad[10];

    PRINTF("ニュートラル\n");
    PRINTF("ゆらゆら....\n");
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    bfriMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    bfriBank(obj, 0.5f);
    w->count += 2.0f;
    if (w->frame->pos[2] > -1200.0f) {
        if (w->frame->pos[1] > -297.0f) {
            w->frame->pos[1] -= 3.0f;
        }
        if (w->frame->pos[1] < -303.0f) {
            w->frame->pos[1] += 3.0f;
        }
    } else {
        if (w->frame->pos[1] < sinf(w->count / 180.0f * 3.141592741f) * 100.0f + -200.0f + -6.0f) {
            w->frame->pos[1] += 3.0f;
        }
        if (w->frame->pos[1] > sinf(w->count / 180.0f * 3.141592741f) * 100.0f + -200.0f + 6.0f) {
            w->frame->pos[1] -= 3.0f;
        }
    }
    w->frame->rot[2] *= 0.94;
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    switch (w->state) {
        case 0:
            w->time = 300;
            bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            w->state = 1;
            w->speed = 0.0f;
            w->link->flags |= 2;
            break;
        case 1:
            if (bossDampVel(obj, 0.95f) == 0) {
                w->state = 2;
                w->time = 300;
            }
            bfriTurnTo(obj, 0.0f, 0.0f, 3.0f);
            bossStepAnim(w->frame, 0.32f, 0);
            break;
        case 2:
            if (bfriTurnTo(obj, 0.0f, 0.0f, 5.0f)) {
                w->time = 0;
            }
            bossStepAnim(w->frame, 0.32f, 0);
            break;
    }
    return 0;
}

s32 bfriElectricShot(Obj *obj) {
    BossW *w = FX(obj);
    s32 pad[12];

    PRINTF("---------->エレクトリック弾\n");
    PRINTF("shot1\n");
    PRINTF("shot2\n");
    PRINTF("----->time %d\n");
    bfriMoveLimit(obj);
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    bfriMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    if (w->frame->pos[1] < sinf(w->count / 180.0f * 3.141592741f) * 80.0 + -200.0+ -6.0) {
        w->frame->pos[1] += 3.0f;
    }
    if (w->frame->pos[1] > sinf(w->count / 180.0f * 3.141592741f) * 80.0 + -200.0+ 6.0) {
        w->frame->pos[1] -= 3.0f;
    }
    bfriRingAngle -= 0.0174532930038800785;
    bfriRingAngle -= 0.0174532930038800785;
    bossDampVel(obj, 0.99f);
    if (w->frame->pos[2] > -2400.0f) {
        w->frame->pos[2] -= 20.0f;
    }
    switch (w->state) {
        case 0:
            w->time = 800;
            w->state = 1;
            bossSetAnimAt(w->frame, bfriElectricShotAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            bfriTurnTo(obj, 0.0f, 0.0f, 4.0f);
            bfriBarrierState = 1;
            w->count = 0.0f;
            bfriAttack3Steps = 0;
            bfriAttack3Unused = 0;
            break;
        case 1:
            bfriRingAngle -= 0.0174532930038800785;
            w->count += 1.0f;
            if ((s32) w->count == 12) {
                bfriBarrierSpark = 1;
                bfriRumble(0xBBBBBBBB, 7);
            }
            if ((s32) w->count == 28) {
                bfriBarrierSpark2 = 1;
                bfriRumble(0xBBBBBBBB, 7);
            }
            if (bossStepAnim(w->frame, 0.32f, 2)) {
                w->state = 2;
                w->count = 0.0f;
                bossSetAnimAt(w->frame, bfriElectricShotStep1Anim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            }
            bfriTurnTo(obj, 0.0f, 0.0f, 4.0f);
            break;
        case 2:
            bfriRingAngle -= 0.0174532930038800785;
            bfriRingAngle -= 0.0174532930038800785;
            bfriTurnTo(obj, 0.0f, 0.0f, 6.0f);
            w->count += 1.0f;
            bfriAttack3Steps++;
            if ((s32) w->count == 12) {
                bfriBarrierSpark = 1;
                SoundLocation2(0x254, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
            }
            if ((s32) w->count == 28) {
                bfriBarrierSpark2 = 1;
                SoundLocation2(0x254, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
            }
            if (bossStepAnim(w->frame, 0.32f, 2)) {
                w->frame->frame = 0.0f;
                w->count = 0.0f;
                if (bfriAttack3Steps > 200 || bfriBitsAlive == 0) {
                    w->speed = 0.0f;
                    w->state = 3;
                    bfriBarrierState = 0;
                    bossSetAnimAt(w->frame, bfriElectricShotStep2Anim, 0xFB, 0.0f);
                    fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                }
            }
            break;
        case 3:
            bfriRingAngle -= 0.0174532930038800785;
            bfriTurnTo(obj, 0.0f, 0.0f, 6.0f);
            if (bossStepAnim(w->frame, 0.32f, 2)) {
                w->time = 0;
                bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            }
            break;
    }
    return 0;
}

s32 bfriBarrierAttack(Obj *obj) {
    BossW *w = FX(obj);
    s32 pad[11];

    PRINTF("----->バリア\n");
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    bfriMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    bfriBank(obj, 0.3f);
    w->count += 2.0f;
    if (w->frame->pos[2] > -1200.0f) {
        if (w->frame->pos[1] > -297.0f) {
            w->frame->pos[1] -= 3.0f;
        }
        if (w->frame->pos[1] < -303.0f) {
            w->frame->pos[1] += 3.0f;
        }
    } else {
        if (w->frame->pos[1] < sinf(w->count / 180.0f * 3.141592741f) * 100.0f + -200.0f + -6.0f) {
            w->frame->pos[1] += 3.0f;
        }
        if (w->frame->pos[1] > sinf(w->count / 180.0f * 3.141592741f) * 100.0f + -200.0f + 6.0f) {
            w->frame->pos[1] -= 3.0f;
        }
    }
    w->frame->rot[2] *= 0.94;
    w->frame->rot[2] *= 0.94;
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    if (bfriBitsAlive == 7) {
        bfriBarrierState = 4;
    }
    switch (w->state) {
        case 0:
            w->time = 4200;
            bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            w->state = 1;
            w->speed = 0.0f;
            w->link->flags |= 2;
            w->count = 0.0f;
            bfriBariaMake();
            soundActpanCreate(obj, 0x176, gBattleCamView, 500.0f, 11000.0f, gBattleCamView->fovy);
            bfriBitsRemake = 1;
            gBossFlags |= 0x4004;
            break;
        case 1:
            w->count += 1.0f;
            bossDampVel(obj, 0.97f);
            if (w->frame->pos[2] < -2800.0f) {
                w->state = 2;
                w->count = 0.0f;
            }
            bfriTurnTo(obj, 0.0f, 0.0f, 1.0f);
            bossStepAnim(w->frame, 0.32f, 0);
            if (w->frame->pos[2] > -2800.0f) {
                w->frame->pos[2] -= 10.0f;
            }
            if (w->frame->pos[2] < -2811.0f) {
                w->frame->pos[2] += 10.0f;
            }
            w->frame->pos[0] *= 0.97;
            break;
        case 2:
            w->count += 1.0f;
            bossDampVel(obj, 0.97f);
            if (w->count > 700.0f) {
                w->state = 3;
                w->count = 0.0f;
            }
            bfriTurnTo(obj, 0.0f, 0.0f, 1.0f);
            bossStepAnim(w->frame, 0.32f, 0);
            break;
        case 3:
            w->count += 1.0f;
            bossDampVel(obj, 0.97f);
            if (w->count >= 1900.0f) {
                w->state = 4;
                w->count = 0.0f;
            }
            bfriTurnTo(obj, 0.0f, 0.0f, 6.0f);
            bossStepAnim(w->frame, 0.32f, 0);
            w->vel[0] = -w->frame->pos[0] / 330.0f;
            w->vel[2] = -w->frame->pos[2] / 330.0f;
            if (sqrtf(w->frame->pos[2] * w->frame->pos[2] + w->frame->pos[0] * w->frame->pos[0]) < 900.0f &&
                gGameFrameCount % 15 == 0) {
                bfriDdmybombCreate(obj, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f, 3, 10, 67);
            }
            break;
        case 4:
            bfriBitsHidden = 1;
            w->vel[2] = -10.0f;
            w->vel[0] = 0.0f;
            if (w->count++ > 300.0f) {
                w->time = 0;
                bfriBarrierState = 0;
                gBossFlags &= ~0x4000;
                gBossFlags &= ~4;
                sndPlay(0x8176);
            }
            break;
    }
    return 0;
}

/* Matched 2026-10-11; it was parked for bfriAttackAnimFlip's address held in a register, gone
 * with it a function static and "D = cond ? 0 : 1; switch (D)". */
s32 bfriBachiAtari(Obj *obj) {
    static s32 bfriAttackAnimFlip = 1; /* which of two attack animations, flipped each time */
    BossW *w = FX(obj);
    f32 dist;

    PRINTF("バチアタリ\n");
    PRINTF("dist-->%d\n");
    bfriMoveLimit(obj);
    bossDampVel(obj, 0.98f);
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    dist = sqrtf(w->frame->pos[2] * w->frame->pos[2] + w->frame->pos[0] * w->frame->pos[0]);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    w->frame->pos[0] *= 0.98;
    w->frame->pos[2] *= 0.98;
    if (w->frame->pos[1] > -295.0f) {
        w->frame->pos[1] -= 5.0f;
    }
    if (w->frame->pos[1] < -305.0f) {
        w->frame->pos[1] += 5.0f;
    }
    switch (w->state) {
        case 0:
            w->time = 800;
            w->state = 1;
            bfriAttackAnimFlip = (bfriAttackAnimFlip > 0) ? 0 : 1;
            switch (bfriAttackAnimFlip) {
                case 0:
                    bossSetAnimAt(w->frame, bfriBachiAtariFlip0Anim, 0xFB, 0.0f);
                    break;
                case 1:
                    bossSetAnimAt(w->frame, bfriBachiAtariFlip1Anim, 0xFB, 0.0f);
                    break;
            }
            fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            break;
        case 1:
            if (w->time == 777) {
                bfriDdmybombCreate(obj, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f, 3, 10, 62);
                if (dist > 1200.0f) {
                    SoundLocation2(0x133, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                   300.0f, 15000.0f, gBattleCamView->fovy);
                }
            }
            if (bossStepAnim(w->frame, 0.42f, 2)) {
                w->state = 2;
                w->time = 100;
                switch (bfriAttackAnimFlip) {
                    case 0:
                        bossSetAnimAt(w->frame, bfriBachiAtariStep1Flip0Anim, 0xFB, 0.0f);
                        break;
                    case 1:
                        bossSetAnimAt(w->frame, bfriBachiAtariStep1Flip1Anim, 0xFB, 0.0f);
                        break;
                }
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            } else {
                bfriTurnTo(obj, 0.0f, 0.0f, 6.0f);
            }
            break;
        case 2:
            if (bossStepAnim(w->frame, 0.42f, 2)) {
                w->time = 0;
                w->count = 0.0f;
                bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            }
            break;
    }
    return 0;
}

/* The rest of the file's .data, after bfriBachiAtari's static (declaration order). */
s32 bfriAttackSounded = 1; /* 1: this attack's first sound has played */
s32 bfriLongAttackSteps[] = { 82, 78, 75, 72, 68, 62, 60, 58, 57, 57, -1 }; /* step counts, read in turn */

/* The attack sets its first-sound flag under a second name, bfriAttackSoundedSet: under one name IDO
 * keeps the address in a register for the store after the test, where the game forms it again
 * (IDO_NOTES 298). */
s32 bfriSuperHeadbutt(Obj *obj) {
    BossW *w = FX(obj);
    AnimeFrame *f;
    s32 pad[2];
    f32 dist;
    s32 pad2[11];

    PRINTF("スーパー頭突き\n");
    PRINTF("ani_flame -->%f\n");
    PRINTF("dist %d\n");
    bfriMoveLimit(obj);
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    f = w->frame;
    dist = sqrtf(f->pos[2] * f->pos[2] + f->pos[0] * f->pos[0]);
    f->pos[0] += w->vel[0];
    f = w->frame;
    f->pos[1] += w->vel[1];
    f = w->frame;
    f->pos[2] += w->vel[2];
    bfriRingAngle -= 0.0523598790116402354;
    f = w->frame;
    f->rot[2] *= 0.94;
    f = w->frame;
    f->rot[0] *= 0.94;
    if ((s32) gGameFrameCount % 5 == 0 && w->state < 2) {
        f = w->frame;
        bfriFzanzouMake(obj, f->pos[0], f->pos[1], f->pos[2] - 20.0f, f->rot[0], f->rot[1], f->rot[2],
                               f->frame, w->state);
    }
    switch (w->state) {
        case 0:
            if (bfriBarrierState != 3) {
                bfriBarrierState = 0;
            }
            w->time = 2800;
            w->state = 1;
            w->speed = 0.0f;
            bfriAttackSounded = 0;
            bossSetAnimAt(w->frame, bfriSuperHeadbuttAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            bfriTurnTo(obj, 0.0f, 0.0f, 6.0f);
            bfriRingRadius = 1100.0f;
            bfriCloudsRun = 0;
            gBossFlags |= 4;
            break;
        case 1:
            bfriTurnTo(obj, 0.0f, 0.0f, 4.0f);
            bfriRingAngle -= 0.0698131720155203139;
            f = w->frame;
            if (f->pos[2] > -3200.0f) {
                f->pos[2] -= 16.0f;
            }
            f = w->frame;
            if ((s32) f->frame == 15 && bfriAttackSounded == 0) {
                bfriAttackSoundedSet = 1;
                SoundLocation2(0x133, gBattleCamView, f->pos[0], f->pos[1], f->pos[2], 300.0f, 15000.0f,
                               gBattleCamView->fovy);
                f = w->frame;
            }
            if (bossStepAnim(f, 0.24f, 2)) {
                f = w->frame;
                bfriSunaEfMake(f->pos[0], 0.0f, f->pos[2], w->vel[0], w->vel[2], 13.0f, 240);
                w->state = 2;
                bfriRingRadius = 1340.0f;
                f = w->frame;
                SoundLocation2(0x17B, gBattleCamView, f->pos[0], f->pos[1], f->pos[2], 300.0f, 15000.0f,
                               gBattleCamView->fovy);
            }
            break;
        case 2:
            bfriRingAngle -= 0.0872664650194003855;
            w->count += 1.0f;
            bfriTurnTo(obj, 0.0f, 0.0f, 6.0f);
            w->speed += 3.0f;
            if (w->speed > gBossPhase * 22 + 144) {
                w->speed = gBossPhase * 22 + 144;
            }
            if (w->time % 20 == 0) {
                f = w->frame;
                bfriSunaEfMake(f->pos[0], 0.0f, f->pos[2], w->vel[0], w->vel[2], 8.0f, 64);
            }
            w->speed += 0.5;
            if (w->speed > gBossPhase * 22 + 144) {
                w->speed = gBossPhase * 22 + 144;
            }
            w->vel[0] = -sinf(w->frame->rot[1] - 3.141592741f) * w->speed;
            w->vel[2] = -cosf(w->frame->rot[1] - 3.141592741f) * w->speed;
            if (dist <= 1000.0f) {
                w->state = 3;
                f = w->frame;
                bfriDdmybombCreate(obj, f->pos[0], f->pos[1], f->pos[2], 700.0f, 3, 10, 64);
                bossSetAnimAt(w->frame, bfriSuperHeadbuttStep1Anim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                f = w->frame;
                bfriSunaEfMake(f->pos[0], 0.0f, f->pos[2], w->vel[0], w->vel[2], 7.0f, 240);
            }
            break;
        case 3:
            bfriRingAngle -= 0.0174532930038800785;
            bfriTurnTo(obj, 0.0f, 0.0f, 6.0f);
            bossDampVel(obj, 0.96f);
            if (bossStepAnim(w->frame, 0.16f, 2)) {
                bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                w->time = 0;
                bfriRingRadius = 700.0f;
                bfriCloudsRun = 1;
                gBossFlags &= ~4;
            }
            break;
    }
    return 0;
}

/* The long attack: the boss drifts and bobs on a sine, then counts bfriLongAttackCount up to
 * 10000 while stepping through bfriLongAttackSteps, fading the tornado in (to 240) and out.
 * Its three counters are function statics (IDO_NOTES 343); "s32 pad[11]; s32 pad2;" give the
 * game's frame 0x70 and place the sinf/sndPlay temp at its 0x2C. Matched 2026-10-11: case 2 reads the static counter
 * directly (no local copy); IDO keeps it in a0 and stores it back itself before the calls. */
s32 bfriSenpuuCrash(Obj *obj) {
    static s32 bfriLongAttackIndex = 1;
    static s32 bfriLongAttackLeft = 1;
    static u32 bfriLongAttackCount = 1;
    BossW *w = FX(obj);
    s32 pad[11];
    s32 pad2;

    PRINTF("---------->センプークラッシュ\n");
    PRINTF("timer -->%d\n");
    PRINTF("プロペラ発射\n");
    bfriMoveLimit(obj);
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    bfriMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    w->frame->pos[0] *= 0.98;
    if (w->frame->pos[2] > -2800.0f) {
        w->frame->pos[2] -= 10.0f;
    }
    if (w->frame->pos[2] < -2811.0f) {
        w->frame->pos[2] += 10.0f;
    }
    if (w->frame->pos[1] < sinf(w->count / 180.0f * 3.141592741f) * 80.0 + -200.0 + -6.0) {
        w->frame->pos[1] += 3.0f;
    }
    if (w->frame->pos[1] > sinf(w->count / 180.0f * 3.141592741f) * 80.0 + -200.0 + 6.0) {
        w->frame->pos[1] -= 3.0f;
    }
    bfriRingAngle += 0.034906586007760157;
    bossDampVel(obj, 0.97f);
    switch (w->state) {
        case 0:
            w->time = 12800;
            w->state = 1;
            bossSetAnimAt(w->frame, bfriSenpuuCrashState0Anim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            bfriTurnTo(obj, 0.0f, 0.0f, 4.0f);
            bfriBarrierState = 2;
            sndPlay(0x24E);
            w->count = 0.0f;
            bfriPropellerFull = 1;
            bfriLongAttackCount = 0;
            bfriPuroperaMake();
            bfriCloudsRun = 0;
            gBrotherFlags |= 0x20000;
            bfriPuroBombBlowing = 0;
            gBossFlags |= 4;
            bfriLongAttackLeft = 0;
            bfriLongAttackIndex = 0;
            break;
        case 1:
            bfriRingAngle += 0.034906586007760157;
            w->count += 1.0f;
            if ((s32) w->count == 118) {
                sndPlay(0x253);
            }
            if (bossStepAnim(w->frame, 0.064f, 2)) {
                w->state = 2;
                w->count = 0.0f;
                bossSetAnimAt(w->frame, bfriSenpuuCrashState1Anim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                bfriTornadoUnusedFlag = 0;
                bfriTornadoAlpha = 0;
                bfriLongAttackLeft = bfriLongAttackSteps[bfriLongAttackIndex];
                sndPlay(0x17A);
            }
            bfriTurnTo(obj, 0.0f, 0.0f, 4.0f);
            break;
        case 2:
            bfriRingAngle += 0.0698131720155203139;
            bfriTurnTo(obj, 0.0f, 0.0f, 6.0f);
            w->count += 1.0f;
            bossStepAnim(w->frame, 0.32f, 0);
            if (w->frame->frame > 11.0f) {
                w->count = 0.0f;
            }
            if (bfriPuroBombBlowing == 0) {
                bfriLongAttackCount++;
                if (bfriLongAttackLeft == -1) {
                    bfriLongAttackCount = 9760;
                    bfriPuroBombBlowing = 1;
                    bfriLongAttackLeft = -2;
                    break;
                }
                if (bfriLongAttackLeft-- == 0) {
                    bfriLongAttackIndex++;
                    bfriLongAttackLeft = bfriLongAttackSteps[bfriLongAttackIndex];
                    bfriPuroBombCreate();
                }
            }
            if (bfriLongAttackCount < 260) {
                bfriTornadoAlpha++;
            }
            if (bfriTornadoAlpha > 240) {
                bfriTornadoAlpha = 240;
            }
            if (bfriLongAttackCount > 9760) {
                bfriTornadoAlpha = 10000 - bfriLongAttackCount;
            }
            if (bfriTornadoAlpha < 0) {
                bfriTornadoAlpha = 0;
            }
            if (bfriLongAttackCount == 10000) {
                w->state = 3;
                w->speed = 0.0f;
                bfriBarrierState = 0;
                bossSetAnimAt(w->frame, bfriSenpuuCrashState2Anim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                sndPlay(0x51A);
            }
            if (gGameFrameCount % 6 == 0) {
                bfriTatTestMake();
            }
            break;
        case 3:
            bfriRingAngle += 0.034906586007760157;
            bfriTurnTo(obj, 0.0f, 0.0f, 6.0f);
            if (bossStepAnim(w->frame, 0.16f, 2)) {
                w->time = 0;
                bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                bfriPropellerFull = 0;
                bfriCloudsRun = 1;
                gBrotherFlags &= ~0x20000;
                gBossFlags &= ~4;
            }
            break;
    }
    return 0;
}

s32 bfriBitBodySlam(Obj *obj) {
    BossW *w = FX(obj);
    s32 pad[10];

    PRINTF("ビット体当たり\n");
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    bfriMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    bfriBank(obj, 0.3f);
    w->count += 2.0f;
    if (w->frame->pos[1] < sinf(w->count / 180.0f * 3.141592741f) * 100.0f + -200.0f + -6.0f) {
        w->frame->pos[1] += 3.0f;
    }
    if (w->frame->pos[1] > sinf(w->count / 180.0f * 3.141592741f) * 100.0f + -200.0f + 6.0f) {
        w->frame->pos[1] -= 3.0f;
    }
    w->frame->rot[2] *= 0.94;
    w->frame->rot[2] *= 0.94;
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    switch (w->state) {
        case 0:
            w->time = 3200;
            bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            w->state = 1;
            w->speed = 0.0f;
            w->link->flags |= 2;
            bfriBarrierState = 3;
            bfriBitsHidden = 0;
            w->count = 0.0f;
            break;
        case 1:
            w->count += 1.0f;
            bossDampVel(obj, 0.97f);
            if (w->count >= 200.0f) {
                w->state = 2;
                w->time = 100;
            }
            bfriTurnTo(obj, 0.0f, 0.0f, 6.0f);
            bossStepAnim(w->frame, 0.32f, 0);
            break;
        case 2:
            bfriBitsHidden = 1;
            break;
    }
    return 0;
}

/* The flip attack: moves the boss by its velocity, damps x (and z past -860), and alternates
 * between two animation sets (bfriAttack2AnimFlip, a function static) through four states.
 * Matched 2026-10-11: the frame pointer is re-read inside the z clamp only ("f = w->frame;" in
 * the if), so the clamp path reloads pos[2] before pos[0] for dist as the game does. */
s32 bfriBachiRanbu(Obj *obj) {
    static s32 bfriAttack2AnimFlip = 1;
    BossW *w = FX(obj);
    f32 dist;
    AnimeFrame *f;

    PRINTF("-------->バチ乱舞\n");
    PRINTF("bati_loop-->%d\r");
    bfriMoveLimit(obj);
    bossDampVel(obj, 0.98f);
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    f = w->frame;
    f->pos[0] += w->vel[0];
    f = w->frame;
    f->pos[1] += w->vel[1];
    f = w->frame;
    f->pos[2] += w->vel[2];
    bfriRingAngle -= 0.0174532930038800785;
    f = w->frame;
    f->pos[0] *= 0.98;
    f = w->frame;
    if (f->pos[2] < -860.0f) {
        f->pos[2] *= 0.98;
        f = w->frame;
    }
    dist = sqrtf(f->pos[2] * f->pos[2] + f->pos[0] * f->pos[0]);
    switch (w->state) {
        case 0:
            w->time = 800;
            w->state = 1;
            w->count = 0.0f;
            bfriAttack2Steps = 0;
            bfriRingRadius = 1200.0f;
            bfriAttack2AnimFlip = (bfriAttack2AnimFlip > 0) ? 0 : 1;
            switch (bfriAttack2AnimFlip) {
                case 0:
                    bossSetAnimAt(w->frame, bfriBachiAtariFlip0Anim, 0xFB, 0.0f);
                    break;
                case 1:
                    bossSetAnimAt(w->frame, bfriBachiAtariFlip1Anim, 0xFB, 0.0f);
                    break;
            }
            fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            break;
        case 1:
            if (w->count == 29.0f) {
                bfriDdmybombCreate(obj, f->pos[0], f->pos[1], f->pos[2], 300.0f, 3, 10, 62);
            }
            if (w->count == 21.0f && dist > 1200.0f) {
                f = w->frame;
                SoundLocation2(0x133, gBattleCamView, f->pos[0], f->pos[1], f->pos[2], 300.0f, 15000.0f,
                               gBattleCamView->fovy);
            }
            w->count += 1.0f;
            bfriRingAngle -= 0.0174532930038800785;
            bfriRingAngle -= 0.0174532930038800785;
            if (bossStepAnim(w->frame, 0.42f, 2)) {
                w->state = 2;
                w->time = 600;
                w->count = 0.0f;
                switch (bfriAttack2AnimFlip) {
                    case 0:
                        bossSetAnimAt(w->frame, bfriBachiRanbuFlip0Anim, 0xFB, 0.0f);
                        break;
                    case 1:
                        bossSetAnimAt(w->frame, bfriBachiRanbuFlip1Anim, 0xFB, 0.0f);
                        break;
                }
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            } else {
                bfriTurnTo(obj, 0.0f, 0.0f, 6.0f);
            }
            break;
        case 2:
            bfriRingAngle -= 0.0523598790116402354;
            w->count += 1.0f;
            bfriAttack2Steps++;
            if (w->count == 23.0f && dist > 1200.0f) {
                f = w->frame;
                SoundLocation2(0x133, gBattleCamView, f->pos[0], f->pos[1], f->pos[2], 300.0f, 15000.0f,
                               gBattleCamView->fovy);
            }
            if (w->count == 28.0f) {
                f = w->frame;
                bfriDdmybombCreate(obj, f->pos[0], f->pos[1], f->pos[2], 300.0f, 3, 10, 62);
            }
            if (w->count == 54.0f) {
                f = w->frame;
                bfriDdmybombCreate(obj, f->pos[0], f->pos[1], f->pos[2], 300.0f, 3, 10, 62);
            }
            bossStepAnim(w->frame, 0.56f, 0);
            if (w->count > 58.0f) {
                w->count = 0.0f;
                if (bfriAttack2Steps >= 301) {
                    w->state = 3;
                    w->time = 500;
                    switch (bfriAttack2AnimFlip) {
                        case 0:
                            bossSetAnimAt(w->frame, bfriBachiAtariStep1Flip0Anim, 0xFB, 0.0f);
                            break;
                        case 1:
                            bossSetAnimAt(w->frame, bfriBachiAtariStep1Flip1Anim, 0xFB, 0.0f);
                            break;
                    }
                    fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                }
            } else {
                bfriTurnTo(obj, 0.0f, 0.0f, 6.0f);
            }
            break;
        case 3:
            if (bossStepAnim(f, 0.42f, 2)) {
                w->time = 0;
                w->count = 0.0f;
                bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                bfriRingRadius = 700.0f;
            }
            break;
    }
    return 0;
}

/* The rest of the file's .data, after bfriSenpuuCrash's and bfriBachiRanbu's statics. */
s32 bfriAttack2Steps = 1; /* steps in that attack's second state */
s32 bfriPlaces1[][2] = { { -1500, -3300 }, { 800, -3300 }, { 1500, -2300 }, { -400, -2300 },
                                 { -1500, -3000 }, { 1800, -2400 }, { 500, -2400 }, { -400, -2300 },
                                 { 0, 0 } }; /* places (x, z) the boss moves to */
s32 bfriPlaces2[][2] = { { 300, -960 }, { 0, -1060 }, { 300, -960 }, { 0, -1060 }, { 0, 0 } };
s32 bfriPlaces3[][2] = { { 1000, -4000 }, { 0, -4000 }, { -1000, -3700 }, { 0, -3700 }, { 0, 0 } };
u8 bfriTurnSide = 1; /* 0: the boss turns one way, 1: the other */

/* dist is computed and never used (as in its siblings that do use it); IDO drops the sqrt but
 * keeps the loads, which is why pos[0] sits in f2. */
s32 bfriNormalHeadbutt(Obj *obj) {
    BossW *w = FX(obj);
    f32 dist;

    PRINTF("ノーマル頭突き\n");
    bfriMoveLimit(obj);
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    dist = sqrtf(w->frame->pos[2] * w->frame->pos[2] + w->frame->pos[0] * w->frame->pos[0]);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    bfriRingAngle -= 0.034906586007760157;
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    bossDampVel(obj, 0.95f);
    w->frame->pos[0] *= 0.98;
    w->frame->pos[2] *= 0.98;
    switch (w->state) {
        case 0:
            if (bfriBarrierState != 3) {
                bfriBarrierState = 0;
            }
            w->time = 2800;
            w->state = 1;
            w->speed = 0.0f;
            bossSetAnimAt(w->frame, bfriSuperHeadbuttAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            bfriTurnTo(obj, 0.0f, 0.0f, 6.0f);
            break;
        case 1:
            bfriTurnTo(obj, 0.0f, 0.0f, 4.0f);
            bfriRingAngle -= 0.0698131720155203139;
            if (bossStepAnim(w->frame, 0.8f, 2)) {
                bfriDdmybombCreate(obj, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f, 3, 10, 63);
                bossSetAnimAt(w->frame, bfriSuperHeadbuttStep1Anim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                w->state = 2;
            }
            break;
        case 2:
            bfriTurnTo(obj, 0.0f, 0.0f, 6.0f);
            if (bossStepAnim(w->frame, 0.8f, 2)) {
                bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                w->time = 0;
                bfriRingRadius = 700.0f;
                bfriCloudsRun = 1;
            }
            break;
    }
    return 0;
}

/* dist is computed and never used, as in bfriNormalHeadbutt. */
s32 bfriNormalHeadbuttDeco(Obj *obj) {
    BossW *w = FX(obj);
    f32 dist;

    PRINTF("ノーマル頭突き（かざり）\n");
    bfriMoveLimit(obj);
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    dist = sqrtf(w->frame->pos[2] * w->frame->pos[2] + w->frame->pos[0] * w->frame->pos[0]);
    if ((s32) gGameFrameCount % 5 == 0 && w->state < 2) {
        bfriFzanzouMake(obj, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 20.0f, w->frame->rot[0],
                               w->frame->rot[1], w->frame->rot[2], w->frame->frame, w->state);
    }
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    bfriRingAngle -= 0.122173051027160556;
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    bossDampVel(obj, 0.95f);
    switch (w->state) {
        case 0:
            if (bfriBarrierState != 3) {
                bfriBarrierState = 0;
            }
            w->time = 2800;
            w->state = 1;
            w->speed = 0.0f;
            bossSetAnimAt(w->frame, bfriSuperHeadbuttAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            bfriTurnTo(obj, 0.0f, 0.0f, 6.0f);
            break;
        case 1:
            bfriTurnTo(obj, 0.0f, 0.0f, 4.0f);
            if (w->time == 2790) {
                SoundLocation2(0x282, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
            }
            if (bossStepAnim(w->frame, 1.12f, 2)) {
                bfriDdmybombCreate(obj, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f, 3, 10, 63);
                bossSetAnimAt(w->frame, bfriSuperHeadbuttStep1Anim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                w->state = 2;
            }
            break;
        case 2:
            bfriTurnTo(obj, 0.0f, 0.0f, 6.0f);
            if (bossStepAnim(w->frame, 1.12f, 2)) {
                bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                w->time = 0;
                bfriRingRadius = 700.0f;
                bfriCloudsRun = 1;
            }
            break;
    }
    return 0;
}

s32 bfriBitSuperHeadbutt(Obj *obj) {
    BossW *w = FX(obj);
    AnimeFrame *f;
    s32 pad[2];
    f32 dist;
    s32 pad2[11];

    PRINTF("-------->ビット／スーパー頭突き\n");
    PRINTF("dist %d\n");
    bfriMoveLimit(obj);
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    f = w->frame;
    dist = sqrtf(f->pos[2] * f->pos[2] + f->pos[0] * f->pos[0]);
    f->pos[0] += w->vel[0];
    f = w->frame;
    f->pos[1] += w->vel[1];
    f = w->frame;
    f->pos[2] += w->vel[2];
    bfriRingAngle -= 0.0523598790116402354;
    f = w->frame;
    f->rot[2] *= 0.94;
    f = w->frame;
    f->rot[0] *= 0.94;
    if ((s32) gGameFrameCount % 5 == 0 && w->state == 1) {
        f = w->frame;
        bfriFzanzouMake(obj, f->pos[0], f->pos[1], f->pos[2] - 20.0f, f->rot[0], f->rot[1], f->rot[2],
                               f->frame, w->state);
    }
    switch (w->state) {
        case 0:
            bfriBarrierState = 3;
            w->time = 2800;
            w->state = 1;
            w->speed = 0.0f;
            bossSetAnimAt(w->frame, bfriSuperHeadbuttAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            bfriTurnTo(obj, 0.0f, 0.0f, 6.0f);
            bfriRingRadius = 1100.0f;
            bfriCloudsRun = 0;
            gBossFlags |= 4;
            break;
        case 1:
            bfriTurnTo(obj, 0.0f, 0.0f, 4.0f);
            f = w->frame;
            if (f->pos[2] > -3500.0f) {
                f->pos[2] -= 10.0f;
            }
            f = w->frame;
            if (bossStepAnim(f, 0.0672f, 2)) {
                f = w->frame;
                bfriSunaEfMake(f->pos[0], 0.0f, f->pos[2], w->vel[0], w->vel[2], 13.0f, 240);
                w->state = 2;
            }
            break;
        case 2:
            bfriRingAngle -= 0.0872664650194003855;
            w->count += 1.0f;
            bfriTurnTo(obj, 0.0f, 0.0f, 6.0f);
            w->speed += 3.0f;
            if (w->speed > gBossPhase * 22 + 144) {
                w->speed = gBossPhase * 22 + 144;
            }
            if (w->time % 20 == 0) {
                f = w->frame;
                bfriSunaEfMake(f->pos[0], 0.0f, f->pos[2], w->vel[0], w->vel[2], 8.0f, 64);
            }
            w->speed += 0.5;
            if (w->speed > gBossPhase * 22 + 144) {
                w->speed = gBossPhase * 22 + 144;
            }
            w->vel[0] = -sinf(w->frame->rot[1] - 3.141592741f) * w->speed;
            w->vel[2] = -cosf(w->frame->rot[1] - 3.141592741f) * w->speed;
            if (dist <= 900.0f) {
                w->state = 3;
                f = w->frame;
                bfriDdmybombCreate(obj, f->pos[0], f->pos[1], f->pos[2], 300.0f, 3, 10, 64);
                bossSetAnimAt(w->frame, bfriSuperHeadbuttStep1Anim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                f = w->frame;
                bfriSunaEfMake(f->pos[0], 0.0f, f->pos[2], w->vel[0], w->vel[2], 7.0f, 240);
            }
            break;
        case 3:
            bfriRingAngle -= 0.0174532930038800785;
            bfriTurnTo(obj, 0.0f, 0.0f, 6.0f);
            bossDampVel(obj, 0.96f);
            if (bossStepAnim(w->frame, 0.16f, 2)) {
                bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                w->time = 0;
                bfriRingRadius = 700.0f;
                bfriCloudsRun = 1;
                gBossFlags &= ~4;
            }
            break;
    }
    return 0;
}

s32 bfriMove1(Obj *obj) {
    BossW *w = FX(obj);
    s32 pad[13];
    s32 i;

    sinf(w->frame->rot[1] - 3.141592741f);
    cosf(w->frame->rot[1] - 3.141592741f);
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    bfriTurnTo(obj, 0.0f, 0.0f, 2.0f);
    bfriMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    bfriBank(obj, 2.0f);
    if (w->frame->pos[1] < sinf(w->count / 180.0f * 3.141592741f) * 100.0f + -200.0f + -6.0f) {
        w->frame->pos[1] += 3.0f;
    }
    if (w->frame->pos[1] > sinf(w->count / 180.0f * 3.141592741f) * 100.0f + -200.0f + 6.0f) {
        w->frame->pos[1] -= 3.0f;
    }
    switch (w->state) {
        case 0:
            PRINTF("----------> 移動1 \n");
            w->time = 500;
            w->speed = 0.0f;
            w->count = 0.0f;
            i = bfriRand(4);
            w->goal[0] = bfriPlaces1[i][0];
            w->goal[2] = bfriPlaces1[i][1];
            if (bossDistSqXZ(w->goal[0], w->goal[2], w->frame->pos[0], w->frame->pos[2]) < 250000.0f) {
                w->time = 0;
            }
            w->state = 1;
            bfriBarrierState = 0;
            break;
        case 1:
            if (bfriBmoveX(obj, w->goal[0], w->goal[1] - 200, w->goal[2], 0.2f) == 0) {
                w->state = 2;
                w->count = 0.0f;
            } else {
                bossStepAnim(w->frame, 0.78f, 0);
            }
            break;
        case 2:
            if (bossStepAnim(w->frame, 0.64f, 2)) {
                w->time = 0;
                w->frame->rot[0] *= 0.94;
                w->frame->rot[2] *= 0.94;
            }
            bfriTurnTo(obj, 0.0f, 0.0f, 2.0f);
            bossDampVel(obj, 0.97f);
            w->count += 2.0f;
            break;
    }
    return 0;
}

s32 bfriMoveForward(Obj *obj) {
    BossW *w = FX(obj);
    s32 pad[13];
    s32 i;

    sinf(w->frame->rot[1] - 3.141592741f);
    cosf(w->frame->rot[1] - 3.141592741f);
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    bfriMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    bfriBank(obj, 2.0f);
    bfriTurnTo(obj, 0.0f, 0.0f, 2.0f);
    if (w->frame->pos[1] < sinf(w->count / 180.0f * 3.141592741f) * 100.0f + -200.0f + -6.0f) {
        w->frame->pos[1] += 3.0f;
    }
    if (w->frame->pos[1] > sinf(w->count / 180.0f * 3.141592741f) * 100.0f + -200.0f + 6.0f) {
        w->frame->pos[1] -= 3.0f;
    }
    switch (w->state) {
        case 0:
            PRINTF("----------> 移動(前方） \n");
            w->time = 500;
            w->speed = 0.0f;
            w->count = 0.0f;
            i = bfriRand(4);
            w->goal[0] = bfriPlaces2[i][0];
            w->goal[2] = bfriPlaces2[i][1];
            if (bossDistSqXZ(w->goal[0], w->goal[2], w->frame->pos[0], w->frame->pos[2]) < 250000.0f) {
                w->time = 0;
            }
            w->state = 1;
            break;
        case 1:
            if (bfriBmoveX(obj, w->goal[0], w->goal[1] - 200, w->goal[2], 0.2f) == 0) {
                w->state = 2;
                w->count = 0.0f;
            } else {
                bossStepAnim(w->frame, 0.52f, 0);
            }
            break;
        case 2:
            if (bossStepAnim(w->frame, 0.96f, 2)) {
                w->time = 0;
            }
            w->frame->rot[0] *= 0.94;
            w->frame->rot[2] *= 0.94;
            bossDampVel(obj, 0.97f);
            w->count += 2.0f;
            break;
    }
    return 0;
}

s32 bfriMoveToBase(Obj *obj) {
    BossW *w = FX(obj);
    s32 pad[14];

    sinf(w->frame->rot[1] - 3.141592741f);
    cosf(w->frame->rot[1] - 3.141592741f);
    bfriTurnTo(obj, 0.0f, 0.0f, 2.0f);
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    bfriMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    bfriBank(obj, 2.0f);
    if (w->frame->pos[1] < sinf(w->count / 180.0f * 3.141592741f) * 100.0f + -200.0f + -6.0f) {
        w->frame->pos[1] += 3.0f;
    }
    if (w->frame->pos[1] > sinf(w->count / 180.0f * 3.141592741f) * 100.0f + -200.0f + 6.0f) {
        w->frame->pos[1] -= 3.0f;
    }
    switch (w->state) {
        case 0:
            PRINTF("----------> 移動(基本位置） \n");
            w->goal[0] = 0;
            w->goal[2] = -3000;
            w->time = 500;
            w->speed = 0;
            w->count = 0;
            if (bossDistSqXZ(w->goal[0], w->goal[2], w->frame->pos[0], w->frame->pos[2]) < 250000.0f) {
                w->time = 0;
            }
            w->state = 1;
            bfriBarrierState = 0;
            break;
        case 1:
            if (bfriBmoveX(obj, w->goal[0], w->goal[1] - 200, w->goal[2], 0.2f) == 0) {
                w->state = 2;
                w->count = 0.0f;
            } else {
                bossStepAnim(w->frame, 0.52f, 0);
            }
            break;
        case 2:
            if (bossStepAnim(w->frame, 0.32f, 2)) {
                w->time = 0;
            }
            bossDampVel(obj, 0.95f);
            w->frame->rot[0] *= 0.94;
            w->frame->rot[2] *= 0.94;
            w->count += 2.0f;
            break;
    }
    return 0;
}

s32 bfriMoveToBase2(Obj *obj) {
    BossW *w = FX(obj);
    s32 pad[13];
    s32 i;

    sinf(w->frame->rot[1] - 3.141592741f);
    cosf(w->frame->rot[1] - 3.141592741f);
    bfriTurnTo(obj, 0.0f, 0.0f, 2.0f);
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    bfriMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    bfriBank(obj, 2.0f);
    if (w->frame->pos[1] < sinf(w->count / 180.0f * 3.141592741f) * 100.0f + -200.0f + -6.0f) {
        w->frame->pos[1] += 3.0f;
    }
    if (w->frame->pos[1] > sinf(w->count / 180.0f * 3.141592741f) * 100.0f + -200.0f + 6.0f) {
        w->frame->pos[1] -= 3.0f;
    }
    switch (w->state) {
        case 0:
            PRINTF("----------> 移動(基本位置） \n");
            w->time = 500;
            w->speed = 0.0f;
            w->count = 0.0f;
            i = bfriRand(4);
            w->goal[0] = bfriPlaces3[i][0];
            w->goal[2] = bfriPlaces3[i][1];
            if (bossDistSqXZ(w->goal[0], w->goal[2], w->frame->pos[0], w->frame->pos[2]) < 250000.0f) {
                w->time = 0;
            }
            w->state = 1;
            bfriBarrierState = 0;
            break;
        case 1:
            if (bfriBmoveX(obj, w->goal[0], w->goal[1] - 200, w->goal[2], 0.2f) == 0) {
                w->state = 2;
                w->count = 0.0f;
            } else {
                bossStepAnim(w->frame, 0.52f, 0);
            }
            break;
        case 2:
            if (bossStepAnim(w->frame, 0.64f, 2)) {
                w->time = 0;
            }
            bossDampVel(obj, 0.97f);
            w->frame->rot[0] *= 0.94;
            w->frame->rot[2] *= 0.94;
            w->count += 2.0f;
            break;
    }
    return 0;
}

s32 bfriMoveForward2(Obj *obj) {
    BossW *w = FX(obj);
    s32 pad[13];
    s32 i;

    sinf(w->frame->rot[1] - 3.141592741f);
    cosf(w->frame->rot[1] - 3.141592741f);
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    bfriMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    bfriBank(obj, 2.0f);
    bfriTurnTo(obj, 0.0f, 0.0f, 2.0f);
    if (w->frame->pos[1] < sinf(w->count / 180.0f * 3.141592741f) * 100.0f + -200.0f + -6.0f) {
        w->frame->pos[1] += 3.0f;
    }
    if (w->frame->pos[1] > sinf(w->count / 180.0f * 3.141592741f) * 100.0f + -200.0f + 6.0f) {
        w->frame->pos[1] -= 3.0f;
    }
    switch (w->state) {
        case 0:
            PRINTF("----------> 移動(前方） \n");
            w->time = 500;
            w->speed = 0.0f;
            w->count = 0.0f;
            i = bfriRand(4);
            w->goal[0] = bfriPlaces2[i][0];
            w->goal[2] = bfriPlaces2[i][1];
            if (bossDistSqXZ(w->goal[0], w->goal[2], w->frame->pos[0], w->frame->pos[2]) < 250000.0f) {
                w->time = 0;
            }
            w->state = 1;
            if (bfriBarrierState != 3) {
                bfriBarrierState = 0;
            }
            break;
        case 1:
            if (bfriBmoveXFar(obj, w->goal[0], w->goal[1] - 200, w->goal[2], 0.4f) == 0) {
                w->state = 2;
                w->count = 0.0f;
            } else {
                bossStepAnim(w->frame, 0.52f, 0);
            }
            break;
        case 2:
            if (bossStepAnim(w->frame, 1.28f, 2)) {
                w->time = 0;
            }
            w->frame->rot[0] *= 0.94;
            w->frame->rot[2] *= 0.94;
            bossDampVel(obj, 0.95f);
            w->count += 2.0f;
            break;
    }
    return 0;
}

s32 bfriFloatMove(Obj *obj) {
    BossW *w = FX(obj);
    s32 pad[12];
    f32 dist;

    PRINTF("----------> ふよふよ移動 \n");
    PRINTF("dist ---> %d  \r");
    bfriTurnTo(obj, 0.0f, 0.0f, 2.0f);
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    bfriMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    bfriBank(obj, 1.0f);
    if (w->frame->pos[1] < sinf(w->count / 180.0f * 3.141592741f) * 100.0f + -200.0f + -6.0f) {
        w->frame->pos[1] += 3.0f;
    }
    if (w->frame->pos[1] > sinf(w->count / 180.0f * 3.141592741f) * 100.0f + -200.0f + 6.0f) {
        w->frame->pos[1] -= 3.0f;
    }
    switch (w->state) {
        case 0:
            w->time = 1200;
            w->speed = 0.0f;
            w->count = 0.0f;
            w->count = bfriRand(40) + 50;
            w->state = 1;
            break;
        case 1:
            bossStepAnim(w->frame, 0.48f, 0);
            w->vel[0] = sinf(w->count * 1.7 * 0.0174532930038800785) * (gBossPhase * 8 + 32) * 0.4;
            w->vel[0] += sinf(gGameFrameCount * 2 * 0.0174532930038800785) * (gBossPhase * 8 + 32) * 0.2;
            w->vel[2] = cosf(w->count * 2 * 0.0174532930038800785) * (gBossPhase * 8 + 32) / 8;
            w->vel[2] += (gBossPhase * 8 + 32) / 3 +
                         cosf(gGameFrameCount * 0.0174532930038800785) * (gBossPhase * 8 + 32) / 3;
            dist = sqrtf(w->frame->pos[2] * w->frame->pos[2] + w->frame->pos[0] * w->frame->pos[0]);
            if (w->count-- <= 0.0f || dist < 1000.0f) {
                w->state = 2;
            }
            break;
        case 2:
            if (bossStepAnim(w->frame, 0.8f, 2)) {
                w->time = 0;
            }
            bossDampVel(obj, 0.98f);
            w->frame->rot[0] *= 0.94;
            w->frame->rot[2] *= 0.94;
            w->count += 2.0f;
            break;
    }
    return 0;
}

s32 bfriStraightHit(Obj *obj) {
    BossW *w = FX(obj);
    AnimeFrame *f;
    f64 tilt = 1.0;
    s32 pad[7];

    bfriMoveLimit(obj);
    w->frame->rot[0] = w->vel[2] * 0.0174532930038800785 * tilt;
    w->frame->rot[2] = -w->vel[0] * 0.0174532930038800785 * tilt;
    f = w->frame;
    f->pos[0] += w->vel[0];
    f = w->frame;
    f->pos[1] += w->vel[1];
    f = w->frame;
    f->pos[2] += w->vel[2];
    bfriRingAngle -= 0.0174532930038800785;
    switch (w->state) {
        case 0:
            bossSetAnimAt(w->frame, bfriStraightHitAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            w->time = 800;
            fadeAnimStart(&w->anim, bfriTexAnim, 1, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            w->speed = gCockpitHeadPitch / 0.0174532930038800785 * 2 / 3.0 + 40.0;
            w->vel[0] = sinf(w->frame->rot[1] - 3.141592741f) * w->speed;
            w->vel[2] = cosf(w->frame->rot[1] - 3.141592741f) * w->speed;
            f = w->frame;
            w->vel[1] = 10.0 - gCockpitHeadPitch / 0.0174532930038800785 * 2 / 3.0;
            f->pos[0] += w->vel[0] * 5.0f;
            f = w->frame;
            f->pos[2] += w->vel[2] * 5.0f;
            w->state = 1;
            bfriCloudsGather = 1;
            bfriRingRadius = 1200.0f;
            bfriBitsHidden = 0;
            bfriBarrierState = 0;
            gBossFlags &= ~4;
            break;
        case 1:
            bfriTurnTo(obj, 0.0f, 0.0f, 60.0f);
            w->vel[1] -= 0.9;
            w->speed -= 0.6;
            if (w->speed < 0.0f) {
                w->speed = 0.0f;
            }
            w->vel[0] = sinf(w->frame->rot[1] - 3.141592741f) * w->speed;
            w->vel[2] = cosf(w->frame->rot[1] - 3.141592741f) * w->speed;
            bossStepAnim(w->frame, 0.13f, 2);
            f = w->frame;
            if (f->pos[1] < -500.0f) {
                f->pos[1] = -500.0f;
                f = w->frame;
                bfriKoke2EfMake(f->pos[0], 0.0f, f->pos[2] + 300.0f);
                w->state = 2;
                w->vel[1] = 33.0f;
                w->count = 34.0f;
                bossSetAnimAt(w->frame, bfriStraightHitAnim, 0xFB, 1.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                f = w->frame;
                SoundLocation2(0x10C, gBattleCamView, f->pos[0], f->pos[1], f->pos[2], 300.0f, 15000.0f,
                               gBattleCamView->fovy);
            }
            break;
        case 2:
            w->vel[1] -= 1.0f;
            w->vel[0] = sinf(w->frame->rot[1] - 3.141592741f) * w->speed;
            w->vel[2] = cosf(w->frame->rot[1] - 3.141592741f) * w->speed;
            w->speed -= 0.8;
            if (w->count > 0.0f) {
                w->count -= 1.0f;
            }
            bossStepAnim(w->frame, 0.026f, 3);
            if (w->frame->pos[1] <= -200.0f) {
                w->state = 3;
            }
            break;
        case 3:
            w->vel[1] -= 0.4;
            bfriRingRadius = 700.0f;
            w->vel[0] = sinf(w->frame->rot[1] - 3.141592741f) * w->speed;
            w->vel[2] = cosf(w->frame->rot[1] - 3.141592741f) * w->speed;
            w->speed -= 0.7;
            if (w->speed < 0.0f) {
                w->speed = 0.0f;
            }
            f = w->frame;
            if (f->pos[1] > -200.0f) {
                w->vel[1] = 0.0f;
                /* FAKE: a compiled-out print stood here (see bfriKnockedBack); OoT/MM's `if (1) {}`. */
                if (1) {}
                f = w->frame;
            }
            if (bossStepAnim(f, 0.26f, 3)) {
                w->time = 0;
                w->endPattern = 1;
                bfriCloudsGather = 0;
                bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                bfriBitsHidden = 1;
                /* bossSetAnimAt takes the anim as a pointer (tlb.h, bmove.c, 2026-10-17): IDO
                 * builds the address afresh at each call as the game does (IDO_NOTES 378); as
                 * (u32) at both calls it kept the first one's in a temp (88 lines), and a (s32)
                 * cast here stood in for the pointer (a kept FAKE: with no cast through the old
                 * u32 parameter, real 21). The print above: none is real 1 (the w->frame reload
                 * goes); PRINTF("\n") matches the code but adds 0x10 to .rodata. */
                bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                bfriCloudsRun = 1;
                bfriRingRadius = 700.0f;
            }
            break;
    }
    return 0;
}

s32 bfriJabHitNegX(Obj *obj) {
    f64 tilt = 1.0;
    BossW *w = FX(obj);
    s32 pad[8];

    bfriMoveLimit(obj);
    w->frame->rot[0] = w->vel[2] * 0.0174532930038800785 * tilt;
    w->frame->rot[2] = -w->vel[0] * 0.0174532930038800785 * tilt;
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    switch (w->state) {
        case 0:
            bossSetAnimAt(w->frame, bfriJabHitNegXAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            fadeAnimStart(&w->anim, bfriTexAnim, 1, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            bfriCloudsGather = 1;
            w->state = 1;
            if (w->frame->pos[2] > -960.0f) {
                w->frame->pos[2] -= 40.0f;
            }
            w->vel[2] = w->vel[2] - 13.0f;
            w->vel[1] = 0.0f;
            w->time = 200;
            w->count = 20.0f;
            bfriBarrierState = 0;
            gBossFlags &= ~4;
            break;
        case 1:
            w->count -= 1.0f;
            w->vel[0] *= 0.95;
            w->vel[2] *= 0.8;
            if (bossStepAnim(w->frame, 0.65f, 2)) {
                bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                w->time = 0;
                w->endPattern = 1;
                bfriCloudsGather = 0;
                bfriCloudsRun = 1;
                bfriRingRadius = 700.0f;
            }
            break;
    }
    return 0;
}

s32 bfriJabHitPosX(Obj *obj) {
    BossW *w = FX(obj);
    f64 tilt = 1.0;

    bfriMoveLimit(obj);
    w->frame->rot[0] = w->vel[2] * 0.0174532930038800785 * tilt;
    w->frame->rot[2] = -w->vel[0] * 0.0174532930038800785 * tilt;
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    switch (w->state) {
        case 0:
            bossSetAnimAt(w->frame, bfriJabHitNegXAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnim, 1, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            w->state = 1;
            if (w->frame->pos[2] > -960.0f) {
                w->frame->pos[2] -= 40.0f;
            }
            w->vel[2] = w->vel[2] - 13.0f;
            w->vel[1] = 0.0f;
            w->time = 200;
            w->count = 20.0f;
            bfriCloudsGather = 1;
            bfriBarrierState = 0;
            gBossFlags &= ~4;
            break;
        case 1:
            w->vel[0] *= 0.95;
            w->vel[2] *= 0.8;
            w->count -= 1.0f;
            if (bossStepAnim(w->frame, 0.65f, 2)) {
                bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                w->time = 0;
                w->endPattern = 1;
                bfriCloudsGather = 0;
                bfriCloudsRun = 1;
                bfriRingRadius = 700.0f;
            }
            break;
    }
    return 0;
}

s32 bfriHundredPunchHit(Obj *obj) {
    BossW *w = FX(obj);
    s32 pad[8];
    f32 x = 0.0f;
    s32 pad2;
    f32 z = 0.0f;

    PRINTF("ひゃくれつおわり\n");
    PRINTF("timer -> %d \n");
    bfriMoveLimit(obj);
    bossHeadingOffset(gCockpitHeadYaw, 0.0f, 1000.0f, &x, &z);
    if (w->frame->pos[0] > x) {
        w->frame->pos[0] -= 10.0f;
    } else {
        w->frame->pos[0] += 10.0f;
    }
    w->frame->pos[1] += 1.0f;
    bfriRingAngle -= 0.209439516046560942;
    if ((gBattleFlags2 & 0x4000) && gImpactMoveTime < 81) {
        w->action = 24;
        w->time = 800;
        w->state = 0;
        return 0;
    }
    switch (w->state) {
        case 0:
            w->vel[0] = w->vel[1] = w->vel[2] = 0.0f;
            w->frame->rot[0] = 0.0f;
            w->frame->rot[1] = 0.0f;
            w->frame->rot[2] = 0.0f;
            bossSetAnimAt(w->frame, bfriJabHitNegXAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnim, 1, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            w->state = 1;
            w->time = 2000;
            bfriCloudsGather = 1;
            gBossFlags &= ~4;
            bfriBarrierState = 0;
            break;
        case 1:
            if (bossStepAnim(w->frame, 2.6f, 2)) {
                bossSetAnimAt(w->frame, bfriHundredPunchHitAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 1, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                w->state = 2;
            }
            break;
        case 2:
            if (bossStepAnim(w->frame, 2.6f, 2)) {
                bossSetAnimAt(w->frame, bfriJabHitNegXAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 1, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                w->state = 3;
            }
            break;
        case 3:
            if (bossStepAnim(w->frame, 0.39f, 2) && !(gBattleFlags2 & 0x4000)) {
                bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                bfriCloudsRun = 1;
                bfriRingRadius = 700.0f;
                w->action = 24;
                w->time = 800;
                w->state = 0;
            }
            break;
    }
    return 0;
}

s32 bfriDizzyKnockdown(Obj *obj) {
    BossW *w = FX(obj);
    f64 tilt = 1.0;

    bfriMoveLimit(obj);
    w->frame->rot[0] = w->vel[2] * 0.0174532930038800785 * tilt;
    w->frame->rot[2] = -w->vel[0] * 0.0174532930038800785 * tilt;
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    if (w->state == 0) {
        bossSetAnimAt(w->frame, bfriStraightHitAnim, 0xFB, 0.0f);
        fadeAnimStart(&w->anim, bfriTexAnim, 1, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
        w->time = 800;
        w->speed = 70.0f;
        w->vel[0] = sinf(w->frame->rot[1] - 3.141592741f) * w->speed;
        w->vel[2] = cosf(w->frame->rot[1] - 3.141592741f) * w->speed;
        w->vel[1] = 17.0f;
        w->frame->pos[0] += w->vel[0] * 5.0f;
        w->frame->pos[2] += w->vel[2] * 5.0f;
        w->state = 1;
        w->action = 21;
        gBossFlags &= ~4;
    }
    return 0;
}

s32 bfriKickHit(Obj *obj) {
    BossW *w = FX(obj);
    f64 tilt = 1.0;
    f32 angle;
    s32 pad[6];
    f32 dir;

    bfriMoveLimit(obj);
    w->frame->rot[0] = w->vel[2] * 0.0174532930038800785 * tilt;
    w->frame->rot[2] = -w->vel[0] * 0.0174532930038800785 * tilt;
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    switch (w->state) {
        case 0:
            bossSetAnimAt(w->frame, bfriStraightHitAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnim, 1, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            w->time = 800;
            w->speed = 60.0f;
            w->vel[0] = sinf(w->frame->rot[1] - 3.141592741f) * w->speed;
            w->vel[2] = cosf(w->frame->rot[1] - 3.141592741f) * w->speed;
            w->vel[1] = 13.0f;
            w->frame->pos[0] += w->vel[0] * 5.0f;
            w->frame->pos[2] += w->vel[2] * 5.0f;
            if (w->frame->pos[0] > 0.0f) {
                bfriTurnSide = 0;
            } else {
                bfriTurnSide = 1;
            }
            w->state = 1;
            gBossFlags &= ~4;
            bfriBarrierState = 0;
            break;
        case 1:
            angle = mathAtan2(-w->frame->pos[0], -w->frame->pos[2]);
            if (bfriTurnSide == 0) {
                w->frame->rot[1] += w->speed / 5.0f * 0.0174532930038800785;
            } else {
                w->frame->rot[1] -= w->speed / 5.0f * 0.0174532930038800785;
            }
            w->vel[1] -= 0.4;
            w->speed -= 0.3;
            if (w->speed < 0.0f) {
                w->speed = 0.0f;
            }
            dir = angle - 3.141592741f;
            w->vel[0] = sinf(dir) * w->speed;
            w->vel[2] = cosf(dir) * w->speed;
            bossStepAnim(w->frame, 0.13f, 2);
            if (w->frame->pos[1] < -500.0f) {
                w->vel[2] = 0.0f;
                w->frame->pos[1] = -500.0f;
                bfriKoke2EfMake(w->frame->pos[0], 0.0f, w->frame->pos[2] + 300.0f);
                w->state = 2;
                w->vel[1] = 8.0f;
                w->count = 34.0f;
                bossSetAnimAt(w->frame, bfriStraightHitAnim, 0xFB, 1.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 1, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                SoundLocation2(0x10C, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
            }
            break;
        case 2:
            w->vel[1] -= 0.1;
            w->vel[0] *= 0.9;
            bfriTurnTo(obj, 0.0f, 0.0f, 4.0f);
            w->count -= 1.0f;
            bossStepAnim(w->frame, 0.013f, 3);
            if (w->count == 0.0f) {
                w->state = 3;
            }
            break;
        case 3:
            w->vel[1] -= 0.03;
            if (bossStepAnim(w->frame, 0.26f, 3)) {
                w->time = 0;
                w->endPattern = 1;
                bfriCloudsGather = 0;
                bfriCannonStun = 0;
                bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                bfriCloudsRun = 1;
                bfriRingRadius = 700.0f;
            }
            break;
    }
    return 0;
}

s32 bfriCannonHit(Obj *obj) {
    BossW *w = FX(obj);
    AnimeFrame *f;
    f64 tilt = 1.0;

    bfriMoveLimit(obj);
    w->frame->rot[0] = w->vel[2] * 0.0174532930038800785 * tilt;
    w->frame->rot[2] = -w->vel[0] * 0.0174532930038800785 * tilt;
    f = w->frame;
    f->pos[0] += w->vel[0];
    f = w->frame;
    f->pos[1] += w->vel[1];
    f = w->frame;
    f->pos[2] += w->vel[2];
    bfriTurnTo(obj, 0.0f, 0.0f, 60.0f);
    switch (w->state) {
        case 0:
            f = w->frame;
            f->f.flags |= 2;
            w->vel[0] = 0.0f;
            w->vel[1] = 0.0f;
            w->vel[2] = 0.0f;
            w->frame->rot[0] = 0.0f;
            w->frame->rot[1] = 0.0f;
            w->frame->rot[2] = 0.0f;
            w->time = 800;
            w->frame->pos[1] = 0.0f;
            bfriFightOver = 1;
            bfriBarrierState = 0;
            bfriCloudsRun = 0;
            gBrotherFlags &= ~0x20000;
            gBossFlags &= ~4;
            f = w->frame;
            if (f->pos[0] > 0.0f) {
                bossSetAnimAt(f, bfriJabHitNegXAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 1, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            } else {
                bossSetAnimAt(f, bfriHundredPunchHitAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 1, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            }
            w->time = 800;
            w->state = 1;
            w->count = 240.0f;
            gBrotherFlags &= ~0x8000;
            bfriCloudsGather = 1;
            break;
        case 1:
            w->link->flags |= 2;
            bossStepAnim(w->frame, 1.95f, 0);
            if ((s32) w->count-- < 1) {
                f = w->frame;
                mathAtan2(f->pos[0], f->pos[2]);
                bfriFightOver = 0;
                sndPlay(0x30A);
                w->time = 800;
                w->action = 21;
                w->state = 0;
                bfriCloudsGather = 0;
                bfriCannonHitLast = 0;
                bfriEffectsEnd = 0;
                bfriCloudsRun = 1;
                bfriRingRadius = 700.0f;
            }
            break;
    }
    return 0;
}

s32 bfriBrotherAttackHit(Obj *obj) {
    BossW *w = FX(obj);
    AnimeFrame *f;
    f64 tilt = 1.0;
    s32 pad[7];

    bfriMoveLimit(obj);
    w->frame->rot[0] = w->vel[2] * 0.0174532930038800785 * tilt;
    w->frame->rot[2] = -w->vel[0] * 0.0174532930038800785 * tilt;
    f = w->frame;
    f->pos[0] += w->vel[0];
    f = w->frame;
    f->pos[1] += w->vel[1];
    f = w->frame;
    f->pos[2] += w->vel[2];
    bfriRingAngle -= 0.0174532930038800785;
    switch (w->state) {
        case 0:
            bossSetAnimAt(w->frame, bfriElectricShotAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnim, 1, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            w->time = 800;
            w->speed = 60.0f;
            w->vel[0] = sinf(w->frame->rot[1]) * w->speed;
            w->vel[2] = cosf(w->frame->rot[1]) * w->speed;
            f = w->frame;
            w->vel[1] = 10.0 - gCockpitHeadPitch / 0.0174532930038800785 * 2 / 3.0;
            f->pos[0] += w->vel[0] * 5.0f;
            f = w->frame;
            f->pos[2] += w->vel[2] * 5.0f;
            w->state = 1;
            bfriCloudsGather = 1;
            bfriRingRadius = 1200.0f;
            bfriBitsHidden = 0;
            bfriBarrierState = 0;
            gBossFlags &= ~4;
            w->count = 0.0f;
            break;
        case 1:
            bfriTurnTo(obj, 0.0f, 0.0f, 60.0f);
            w->vel[1] -= 0.9;
            w->speed -= 0.6;
            if (w->speed < 0.0f) {
                w->speed = 0.0f;
            }
            w->vel[0] = sinf(w->frame->rot[1]) * w->speed;
            w->vel[2] = cosf(w->frame->rot[1]) * w->speed;
            bossStepAnim(w->frame, 0.26f, 2);
            f = w->frame;
            if (f->pos[1] < -500.0f) {
                f->pos[1] = -500.0f;
                f = w->frame;
                bfriKoke2EfMake(f->pos[0], 0.0f, f->pos[2] + 300.0f);
                w->state = 2;
                w->vel[1] = 33.0f;
                w->count = 34.0f;
                bossSetAnimAt(w->frame, bfriElectricShotStep2Anim, 0xFB, 0.0f);
                f = w->frame;
                SoundLocation2(0x10C, gBattleCamView, f->pos[0], f->pos[1], f->pos[2], 300.0f, 15000.0f,
                               gBattleCamView->fovy);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            }
            break;
        case 2:
            w->vel[1] -= 1.0f;
            w->vel[0] = sinf(w->frame->rot[1]) * w->speed;
            w->vel[2] = cosf(w->frame->rot[1]) * w->speed;
            w->speed -= 0.8;
            if (w->count > 0.0f) {
                w->count -= 1.0f;
            }
            bossStepAnim(w->frame, 0.26f, 2);
            if (w->frame->pos[1] <= -200.0f) {
                w->state = 3;
            }
            break;
        case 3:
            w->vel[1] -= 0.4;
            bfriRingRadius = 700.0f;
            w->vel[0] = sinf(w->frame->rot[1]) * w->speed;
            w->vel[2] = cosf(w->frame->rot[1]) * w->speed;
            w->speed -= 0.7;
            if (w->speed < 0.0f) {
                w->speed = 0.0f;
            }
            f = w->frame;
            if (f->pos[1] > -200.0f) {
                w->vel[1] = 0.0f;
                /* FAKE: a compiled-out print stood here (see bfriKnockedBack); OoT/MM's `if (1) {}`. */
                if (1) {}
                f = w->frame;
            }
            if (bossStepAnim(f, 0.26f, 2)) {
                w->time = 800;
                w->endPattern = 1;
                bfriCloudsGather = 0;
                bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                bfriBitsHidden = 1;
                bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                bfriCloudsRun = 1;
                bfriRingRadius = 700.0f;
                w->link->flags |= 2;
            }
            break;
    }
    return 0;
}

s32 bfriKnockedBack(Obj *obj) {
    BossW *w = FX(obj);
    AnimeFrame *f;
    f64 tilt = 1.0;

    bfriMoveLimit(obj);
    w->frame->rot[0] = w->vel[2] * 0.0174532930038800785 * tilt;
    w->frame->rot[2] = -w->vel[0] * 0.0174532930038800785 * tilt;
    f = w->frame;
    f->pos[0] += w->vel[0];
    f = w->frame;
    f->pos[1] += w->vel[1];
    f = w->frame;
    f->pos[2] += w->vel[2];
    switch (w->state) {
        case 0:
            bossSetAnimAt(w->frame, bfriStraightHitAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnim, 1, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            w->time = 800;
            w->speed = 27.0f;
            w->vel[0] = 0.0f;
            w->vel[2] = 40.0f;
            w->vel[1] = 16.0f;
            f = w->frame;
            f->f.flags |= 2;
            w->state = 1;
            bfriCloudsGather = 1;
            gBattleFlags2 &= ~1;
            gBossFlags &= ~4;
            break;
        case 1:
            bfriTurnTo(obj, 0.0f, 0.0f, 60.0f);
            w->vel[1] -= 0.7;
            w->speed -= 0.4;
            if (w->speed < 0.0f) {
                w->speed = 0.0f;
            }
            w->vel[0] = sinf(w->frame->rot[1] - 3.141592741f) * w->speed;
            w->vel[2] = cosf(w->frame->rot[1] - 3.141592741f) * w->speed;
            bossStepAnim(w->frame, 0.13f, 2);
            f = w->frame;
            if (f->pos[1] < -500.0f) {
                f->pos[1] = -500.0f;
                f = w->frame;
                SoundLocation2(0x10C, gBattleCamView, f->pos[0], f->pos[1], f->pos[2], 300.0f, 15000.0f,
                               gBattleCamView->fovy);
                f = w->frame;
                bfriKoke2EfMake(f->pos[0], 0.0f, f->pos[2] + 300.0f);
                w->state = 2;
                w->vel[1] = 20.0f;
                w->count = 34.0f;
                bossSetAnimAt(w->frame, bfriStraightHitAnim, 0xFB, 1.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            }
            break;
        case 2:
            w->vel[1] -= 0.7;
            w->vel[0] *= 0.9;
            if (w->count > 0.0f) {
                w->count -= 1.0f;
            }
            bossStepAnim(w->frame, 0.013f, 3);
            if (w->frame->pos[1] <= -200.0f) {
                w->state = 3;
            }
            break;
        case 3:
            f = w->frame;
            w->vel[1] -= 0.4;
            if (f->pos[1] > -200.0f) {
                w->vel[1] = 0.0f;
                /* FAKE: a compiled-out print stood here, written as OoT/MM's `if (1) {}` (MM's
                 * z_skelanime.c: "Debug prints here, required to match"): the block is why IDO
                 * reloads w->frame after it (42 lines without it). */
                if (1) {}
                f = w->frame;
            }
            if (bossStepAnim(f, 0.26f, 3)) {
                w->time = 800;
                w->endPattern = 1;
                bfriCloudsGather = 0;
                bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                bfriRingRadius = 700.0f;
            }
            break;
    }
    return 0;
}

s32 bfriImpactChange(Obj *obj) {
    BossW *w = FX(obj);
    s32 pad[10];

    PRINTF("--------------->   インパクト交代\n");
    PRINTF("ゆらゆら....\n");
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    bfriMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    bfriBank(obj, 0.3f);
    w->count += 2.0f;
    if (w->frame->pos[1] < sinf(w->count / 180.0f * 3.141592741f) * 100.0f + -200.0f + -6.0f) {
        w->frame->pos[1] += 3.0f;
    }
    if (w->frame->pos[1] > sinf(w->count / 180.0f * 3.141592741f) * 100.0f + -200.0f + 6.0f) {
        w->frame->pos[1] -= 3.0f;
    }
    w->frame->rot[2] *= 0.94;
    w->frame->rot[2] *= 0.94;
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    switch (w->state) {
        case 0:
            w->time = 300;
            bossSetAnimAt(w->frame, bfriNormalAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            w->state = 1;
            w->speed = 0.0f;
            w->link->flags |= 2;
            bfriCloudsGather = 0;
            bfriBarrierState = 6;
            bfriRingRadius = 700.0f;
            bfriBitsHidden = 1;
            gBossFlags &= ~0x4000;
            gBossFlags |= 4;
            break;
        case 1:
            if (bossDampVel(obj, 0.97f) == 0) {
                w->state = 2;
                w->time = 600;
            }
            bfriTurnTo(obj, 0.0f, 0.0f, 1.0f);
            bossStepAnim(w->frame, 0.32f, 0);
            break;
        case 2:
            if (bfriTurnTo(obj, 0.0f, 0.0f, 1.0f)) {
                w->state = 3;
                w->time = 90;
            }
            bossStepAnim(w->frame, 0.32f, 0);
            break;
        case 3:
            gBossFlags &= ~4;
            bfriBarrierState = 0;
            bossStepAnim(w->frame, 0.32f, 0);
            w->endPattern = 1;
            break;
    }
    return 0;
}

/* Its callers above reach it far, through t9; its definition takes the plain name. */
#undef bfriSway

s32 bfriSway(Obj *obj) {
    BossW *w = FX(obj);
    s32 pad;
    f32 tz;
    f32 dx;
    f32 dz;
    s32 pad2[2];

    bfriTurnTo(obj, 0.0f, 0.0f, 6.0f);
    tz = sinf(w->count / 180.0f * 3.141592741f * 2.4f) * 600.0f + -2500.0f;
    dx = (sinf(w->count / 180.0f * 3.141592741f) * 1300.0f - w->frame->pos[0]) / 60.0f;
    dz = (tz - w->frame->pos[2]) / 60.0f;
    if (dx > 0.0f) {
        w->vel[0] += 0.6;
    } else {
        w->vel[0] -= 0.6;
    }
    if (dz > 0.0f) {
        w->vel[2] += 0.3;
    } else {
        w->vel[2] -= 0.3;
    }
    if (w->vel[0] > 30.0f) {
        w->vel[0] = 30.0f;
    }
    if (w->vel[0] < -30.0f) {
        w->vel[0] = -30.0f;
    }
    if (w->vel[2] > 8.0f) {
        w->vel[2] = 8.0f;
    }
    if (w->vel[2] < -8.0f) {
        w->vel[2] = -8.0f;
    }
    if (w->frame->rot[0] < w->vel[2] * 2 * 0.0174532930038800785- 0.0698131720155203139) {
        w->frame->rot[0] += 0.034906586007760157;
    }
    if (w->frame->rot[0] > w->vel[2] * 2 * 0.0174532930038800785+ 0.0698131720155203139) {
        w->frame->rot[0] -= 0.034906586007760157;
    }
    if (w->frame->rot[2] < -w->vel[1] * 2 * 0.0174532930038800785- 0.0698131720155203139) {
        w->frame->rot[2] += 0.034906586007760157;
    }
    if (w->frame->rot[2] > -w->vel[1] * 2 * 0.0174532930038800785+ 0.0698131720155203139) {
        w->frame->rot[2] -= 0.034906586007760157;
    }
    if (w->frame->rot[0] > 0.523598790116402313) {
        w->frame->rot[0] = 0.5235987902f;
    }
    if (w->frame->rot[0] < -0.523598790116402313) {
        w->frame->rot[0] = -0.5235987902f;
    }
    if (w->frame->rot[2] > 0.523598790116402313) {
        w->frame->rot[2] = 0.5235987902f;
    }
    if (w->frame->rot[2] < -0.523598790116402313) {
        w->frame->rot[2] = -0.5235987902f;
    }
    return 0;
}
