#include "tlb.h"
#include "tlb/tlb_0325.h"

#define DPRINTF

/* The task's work: a timer and a flag its step reads (tlb_0324_2's twin, 0xC further on). */
typedef struct Tlb325StepW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ u8 pad10[0x48];
    /* 0x58 */ u8 action;
    /* 0x59 */ u8 endPattern; /* 1: the action is over early: end the pattern now */
    /* 0x5A */ u8 state; /* tlb_0325_0/_1's name; 0 with every new action */
    /* 0x5B */ u8 pad5B;
    /* 0x5C */ s16 timer;
    /* 0x5E */ u8 pattern;
    /* 0x5F */ u8 move;
} Tlb325StepW;

extern s32 bfriSkyShown; /* 0: the sky fades out */
s32 bfriTurned = 0; /* 1: the boss has turned round */
u32 bfriUnused1[] = { 0x00000000, 0x00000000, 0x00000000 };
f32 bfriFov = 1.0f;
f32 bfriClearedFloat = 1.0f;
f32 bfriClearedFloats[] = { 1.0f, 1.0f };
extern s32 bfriFightOver;
extern s32 bfriEffectsEnd; /* 1: the battle's effects end */
extern f32 bfriRingAngle; /* an angle, turned 12 degrees a step back */
extern s32 bfriBarrierState; /* the boss's barrier state, 4 while it stands */
s32 bfriPatternStep(Obj *o);

/* The boss's frame (tlb_0324_2's btaiStep, for this boss): bit 21 of gBattleFlags2
 * follows bit 16 or the sky's fade; tlb_0325_1's bfriHit and the pattern step run; the
 * angle bfriRingAngle turns a degree, kept within a turn; while the sky is up,
 * bfriCloudsMake gets the boss's speed; when the brothers' bit 14 is set and the boss
 * lives it turns round at (0, -1800), stops and starts action 26 (once, until bfriTurned is
 * cleared); the boss flags' bit 14 follows the barrier; and the boss is drawn at 3 (bit 14) or
 * 1.7. It returns the pattern step's result. */
s32 bfriStep(Obj *o) {
    Tlb325StepW *w = (Tlb325StepW *) FX(o);
    s32 ret;
    s32 pad[12];

    if ((gBattleFlags2 & 0x10000) || bfriSkyShown != 0) {
        gBattleFlags2 |= 0x200000;
    } else {
        gBattleFlags2 &= ~0x200000;
    }
    (*bfriHit)(o);
    ret = (*bfriPatternStep)(o);
    bfriRingAngle += 0.0174532930038800785;
    if (bfriRingAngle > 6.2831855f) {
        bfriRingAngle -= 6.2831855f;
    } else if (bfriRingAngle < 0.0f) {
        bfriRingAngle += 6.2831855f;
    }
    if (bfriSkyShown == 0) {
        (*bfriCloudsMake)(w->vel[0], w->vel[1], w->vel[2]);
    }
    if (bfriTurned == 1) {
        bfriTurned = 0;
        bfriEffectsEnd = 0;
        bfriFightOver = 0;
    } else if ((gBrotherFlags & 0x4000) && gBossLife > 0) {
        bfriEffectsEnd = 1;
        bfriFightOver = 1;
        w->frame->rot[1] += 3.1415927f;
        w->frame->pos[0] = 0.0f;
        w->frame->pos[2] = -1800.0f;
        w->vel[0] = 0.0f;
        w->vel[2] = 0.0f;
        bfriTurned = 1;
        w->action = 26;
        w->state = 0;
    }
    if (bfriBarrierState == 4) {
        gBossFlags |= 0x4000;
    } else {
        gBossFlags &= ~0x4000;
    }
    if (gBattleFlags2 & 0x4000) {
        w->frame->scale[2] = 3.0f;
        w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
    } else {
        w->frame->scale[2] = 1.7f;
        w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
    }
    return ret;
}

s32 bfriNextAction(Obj *o);
s32 bfriRunAction(Obj *o);

/* tlb_0324_2's btaiPatternStep twin: counts a frame; when the timer runs out or the flag is 1,
 * runs bfriNextAction first; then bfriRunAction, whose result it returns. */
s32 bfriPatternStep(Obj *o) {
    gBossFrameCount++;
    ((Tlb325StepW *) FX(o))->timer--;
    if (((Tlb325StepW *) FX(o))->timer <= 0 || ((Tlb325StepW *) FX(o))->endPattern == 1) {
        (*bfriNextAction)(o);
    }
    return (*bfriRunAction)(o);
}

extern s32 bfriNewAction;
extern s32 bfriBitsRemake;

/* The boss's attack patterns for each of its phases: actions, ended by 32 (pick the next
 * pattern). Rows of 32 only are never picked. */
const s32 bfriPhase0Patterns[16][8] = {
    { 28, 1, 15, 4, 4, 4, 32, 32 },
    { 29, 5, 4, 32, 32, 32, 32, 32 },
    { 30, 15, 9, 9, 9, 9, 32, 32 },
    { 29, 1, 16, 8, 32, 32, 32, 32 },
    { 11, 32, 32, 32, 32, 32, 32, 32 },
    { 1, 13, 32, 32, 32, 32, 32, 32 },
    { 11, 31, 31, 5, 32, 32, 32, 32 },
    { 29, 15, 4, 32, 32, 32, 32, 32 },
    { 29, 1, 13, 2, 32, 32, 32, 32 },
    { 30, 12, 9, 8, 11, 32, 32, 32 },
    { 13, 3, 32, 32, 32, 32, 32, 32 },
    { 30, 10, 4, 4, 4, 32, 32, 32 },
    { 30, 15, 4, 32, 32, 32, 32, 32 },
    { 1, 16, 8, 32, 32, 32, 32, 32 },
    { 30, 15, 4, 32, 32, 32, 32, 32 },
    { 32, 32, 32, 32, 32, 32, 32, 32 },
};
const s32 bfriPhase1Patterns[16][8] = {
    { 29, 16, 8, 32, 32, 32, 32, 32 },
    { 30, 1, 15, 9, 8, 11, 32, 32 },
    { 29, 14, 31, 31, 5, 16, 14, 32 },
    { 30, 1, 11, 14, 7, 16, 16, 32 },
    { 15, 4, 32, 32, 32, 32, 32, 32 },
    { 28, 12, 9, 9, 9, 9, 32, 32 },
    { 1, 16, 8, 32, 32, 32, 32, 32 },
    { 29, 12, 4, 4, 4, 32, 32, 32 },
    { 0, 11, 31, 31, 5, 32, 32, 32 },
    { 30, 0, 15, 8, 32, 32, 32, 32 },
    { 1, 13, 3, 32, 32, 32, 32, 32 },
    { 32, 32, 32, 32, 32, 32, 32, 32 },
    { 32, 32, 32, 32, 32, 32, 32, 32 },
    { 32, 32, 32, 32, 32, 32, 32, 32 },
    { 32, 32, 32, 32, 32, 32, 32, 32 },
    { 32, 32, 32, 32, 32, 32, 32, 32 },
};
const s32 bfriPhase2Patterns[16][8] = {
    { 30, 1, 16, 8, 32, 32, 32, 32 },
    { 29, 14, 7, 28, 8, 14, 2, 32 },
    { 11, 11, 32, 32, 32, 32, 32, 32 },
    { 14, 7, 16, 16, 32, 32, 32, 32 },
    { 31, 31, 5, 14, 32, 32, 32, 32 },
    { 13, 3, 11, 32, 32, 32, 32, 32 },
    { 28, 10, 32, 32, 32, 32, 32, 32 },
    { 13, 7, 14, 5, 32, 32, 32, 32 },
    { 29, 7, 15, 4, 8, 32, 32, 32 },
    { 1, 12, 4, 4, 4, 32, 32, 32 },
    { 30, 14, 7, 32, 32, 32, 32, 32 },
    { 16, 31, 31, 5, 14, 32, 32, 32 },
    { 13, 6, 10, 32, 32, 32, 32, 32 },
    { 1, 16, 8, 32, 32, 32, 32, 32 },
    { 32, 32, 32, 32, 32, 32, 32, 32 },
    { 32, 32, 32, 32, 32, 32, 32, 32 },
};
/* How many patterns each phase picks from. */
const s32 bfriPhasePatternCounts[4] = { 15, 11, 14, 0 };

/* tlb_0324_2's btaiNextAction twin: the boss's next move, with its phase from its life
 * (under 3200: 0, under 2240: 1, under 1280: 2). */
s32 bfriNextAction(Obj *o) {
    s32 next;
    Tlb325StepW *w;

    bfriNewAction = 1;
    ((Tlb325StepW *) FX(o))->state = 0;
    ((Tlb325StepW *) FX(o))->move++;
    ((Tlb325StepW *) FX(o))->timer = 0;
    w = (Tlb325StepW *) FX(o);
    switch (gBossPhase) {
        case 0:
            next = bfriPhase0Patterns[w->pattern][w->move];
            break;
        case 1:
            next = bfriPhase1Patterns[w->pattern][w->move];
            break;
        case 2:
            next = bfriPhase2Patterns[w->pattern][w->move];
            break;
    }
    if (next == 32 || w->endPattern == 1) {
        if (++w->pattern >= bfriPhasePatternCounts[gBossPhase]) {
            w->pattern = 0;
        }
        w->move = 0;
        w->endPattern = 0;
        if (gBossLife < 3200.0f) {
            gBossPhase = 0;
        }
        if (gBossLife < 2240.0f) {
            gBossPhase = 1;
        }
        if (gBossLife < 1280.0f) {
            gBossPhase = 2;
        }
    }
    switch (gBossPhase) {
        case 0:
            w->action = bfriPhase0Patterns[w->pattern][w->move];
            break;
        case 1:
            w->action = bfriPhase1Patterns[w->pattern][w->move];
            break;
        case 2:
            w->action = bfriPhase2Patterns[w->pattern][w->move];
            break;
        default:
            DPRINTF("外部インパクトが交代しました\n");
            DPRINTF("Change\n");
            DPRINTF("%d-> %d\n", w->pattern, w->move);
            DPRINTF("風雷神の勝ち\n");
            DPRINTF("ERROR ->%d\n", gBossPhase);
            break;
    }
    bfriBitsRemake = 0;
    return 0;
}


/* The boss's step (tlb_0324_2's btaiRunAction, for this boss): runs its action, or its ending
 * once one began. It is beaten (ending 2) when its life runs out, and wins (ending 3) when
 * Impact's armour does, each outside actions 17-25. When the battle's flag 8 is set (but in
 * actions 3 and 6) it stops and goes back to action 0. The actions are called through their
 * addresses.
 * Matched 2026-10-12 with its twin's levers: one w local set at its declaration (after ret), and
 * bfriEffectsEnd stored before gBossLife (the game's store order). */
s32 bfriRunAction(Obj *o) {
    s32 ret;
    Tlb325StepW *w = (Tlb325StepW *) FX(o);

    if (gBossLife <= 0) {
        gBossLife = 0;
    }
    if (gBossLife <= 0 && bfriSkyShown == 0
        && (w->action < 17 || w->action >= 26)) {
        bfriEffectsEnd = 1;
        gBossLife = 0;
        bfriSkyShown = 2;
        w->state = 0;
        gBossFlags |= 2;
    }
    if ((gImpactArmor <= 0 || gImpactArmor2 <= 0)
        && (w->action < 17 || w->action >= 26)
        && bfriSkyShown == 0) {
        gBossFlags &= ~0x4000;
        bfriEffectsEnd = 1;
        bfriSkyShown = 3;
        w->state = 0;
    }
    if ((gBattleFlags & 8) && w->action != 3
        && w->action != 6) {
        w->vel[0] = 0.0f;
        w->vel[2] = 0.0f;
        gBrotherFlags &= ~0x20000;
        gBattleFlags &= ~8;
        w->state = 0;
        w->action = 0;
    }
    switch (bfriSkyShown) {
        case 0:
            gBattleFlags2 &= ~0x200000;
            switch (w->action) {
                case 0:
                    ret = (*bfriNormal)(o);
                    break;
                case 1:
                    ret = (*bfriNeutral)(o);
                    break;
                case 2:
                    ret = (*bfriElectricShot)(o);
                    break;
                case 3:
                    ret = (*bfriBarrierAttack)(o);
                    break;
                case 4:
                    ret = (*bfriBachiAtari)(o);
                    break;
                case 6:
                    ret = (*bfriSenpuuCrash)(o);
                    break;
                case 7:
                    ret = (*bfriBitBodySlam)(o);
                    break;
                case 5:
                    ret = (*bfriSuperHeadbutt)(o);
                    break;
                case 8:
                    ret = (*bfriBachiRanbu)(o);
                    break;
                case 9:
                    ret = (*bfriNormalHeadbutt)(o);
                    break;
                case 31:
                    ret = (*bfriNormalHeadbuttDeco)(o);
                    break;
                case 10:
                    ret = (*bfriBitSuperHeadbutt)(o);
                    break;
                case 11:
                    ret = (*bfriMove1)(o);
                    break;
                case 12:
                    ret = (*bfriMoveForward)(o);
                    break;
                case 13:
                    ret = (*bfriMoveToBase)(o);
                    break;
                case 14:
                    ret = (*bfriMoveToBase2)(o);
                    break;
                case 15:
                    ret = (*bfriMoveForward2)(o);
                    break;
                case 16:
                    ret = (*bfriFloatMove)(o);
                    break;
                case 28:
                    ret = (*bfriNormalSwayLong)(o);
                    break;
                case 29:
                    ret = (*bfriNormalSwayMid)(o);
                    break;
                case 30:
                    ret = (*bfriNormalSwayShort)(o);
                    break;
                case 17:
                    ret = (*bfriStraightHit)(o);
                    break;
                case 18:
                    ret = (*bfriJabHitNegX)(o);
                    break;
                case 19:
                    ret = (*bfriJabHitPosX)(o);
                    break;
                case 20:
                    ret = (*bfriHundredPunchHit)(o);
                    break;
                case 21:
                    ret = (*bfriKickHit)(o);
                    break;
                case 22:
                    ret = (*bfriCannonHit)(o);
                    break;
                case 23:
                    ret = (*bfriBrotherAttackHit)(o);
                    break;
                case 24:
                    ret = (*bfriKnockedBack)(o);
                    break;
                case 25:
                    ret = (*bfriDizzyKnockdown)(o);
                    break;
                case 26:
                    ret = (*bfriImpactChange)(o);
                    break;
                case 32:
                    ret = (*bfriNormal)(o);
                    break;
            }
            break;
        case 1:
            gBattleFlags2 |= 0x200000;
            (*bfriEntranceDemo)(o);
            ret = 0;
            break;
        case 2:
            gBattleFlags2 |= 0x200000;
            ret = (*bfriDeathDemo)(o);
            break;
        case 3:
            gBattleFlags2 |= 0x200000;
            ret = (*bfriWinDemo)(o);
            break;
    }
    return ret;
}
