#include "tlb.h"
#include "tlb/tlb_0324.h"

/* The boss's work: its action, the attack pattern it plays and its place in it. */
typedef struct Tlb324StepW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ u8 pad10[0x3C];
    /* 0x4C */ u8 action;
    /* 0x4D */ u8 endPattern; /* 1: the action is over early: end the pattern now */
    /* 0x4E */ u8 step; /* 0 with every new action */
    /* 0x4F */ u8 pad4F;
    /* 0x50 */ s16 timer;
    /* 0x52 */ u8 pattern;
    /* 0x53 */ u8 move;
} Tlb324StepW;

extern s32 btaiEnding; /* the ending: 0 none, 1-3 (btaiEntranceDemo, btaiDeathDemo, btaiWinDemo) */
extern s32 btaiWhirlBubbles;
extern s32 btaiTurned; /* 1: the boss has turned round */
extern s32 btaiInkGone; /* 1: the ink is gone (tlb_0324_6) */
extern s32 btaiEffectsEnd; /* 1: the battle's effects end (tlb_0324_6) */
s32 btaiPatternStep(Obj *o);

/* The boss's frame: bit 21 of gBattleFlags2 follows bit 16 or the ending; tlb_0324_1's
 * btaiHit and the pattern step (btaiPatternStep) run; the boss is drawn at 2.5
 * (bit 14, 18 or 16) or 1.7 (or in action 6); every 100 frames, with btaiWhirlBubbles at 1,
 * btaiTbubbleMakeRand; and when the brothers' bit 14 is set the boss turns round at
 * (0, -1800), stops and starts action 23 (once, until btaiTurned is cleared). It returns the
 * pattern step's result. */
s32 btaiStep(Obj *o) {
    Tlb324StepW *w = (Tlb324StepW *) FX(o);
    s32 ret;
    s32 pad[12];

    if ((gBattleFlags2 & 0x10000) || btaiEnding != 0) {
        gBattleFlags2 |= 0x200000;
    } else {
        gBattleFlags2 &= ~0x200000;
    }
    (*btaiHit)(o);
    ret = (*btaiPatternStep)(o);
    if (w->action != 6) {
        if (gBattleFlags2 & 0x4000) {
            w->frame->scale[2] = 2.5f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
        } else if ((gBattleFlags2 & 0x40000) || (gBattleFlags2 & 0x10000)) {
            w->frame->scale[2] = 2.5f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
        } else {
            w->frame->scale[2] = 1.7f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
        }
    } else {
        w->frame->scale[2] = 1.7f;
        w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
    }
    if (gGameFrameCount % 100 == 0 && btaiWhirlBubbles == 1) {
        (*btaiTbubbleMakeRand)(o);
    }
    if (btaiTurned == 1) {
        btaiTurned = 0;
        btaiEffectsEnd = 0;
        btaiInkGone = 0;
    } else if (gBrotherFlags & 0x4000) {
        btaiEffectsEnd = 1;
        btaiInkGone = 1;
        w->frame->rot[1] += 3.1415927f;
        w->frame->pos[0] = 0.0f;
        w->frame->pos[2] = -1800.0f;
        w->vel[0] = 0.0f;
        w->vel[2] = 0.0f;
        btaiTurned = 1;
        w->action = 23;
        w->step = 0;
    }
    return ret;
}

s32 btaiNextAction(Obj *o);
s32 btaiRunAction(Obj *o);

/* Counts a frame in gBossFrameCount; when the timer runs out or the flag is 1, runs
 * btaiNextAction first; then btaiRunAction, whose result it returns. Both are
 * called through their addresses. */
s32 btaiPatternStep(Obj *o) {
    gBossFrameCount++;
    ((Tlb324StepW *) FX(o))->timer--;
    if (((Tlb324StepW *) FX(o))->timer <= 0 || ((Tlb324StepW *) FX(o))->endPattern == 1) {
        (*btaiNextAction)(o);
    }
    return (*btaiRunAction)(o);
}

extern s32 btaiNewAction;

/* The boss's attack patterns for each of its phases: actions, ended by 29 (pick the next
 * pattern). Rows of 29 only are never picked. */
const s32 btaiPhase0Patterns[16][8] = {
    { 1, 8, 5, 1, 29, 0, 0, 0 },
    { 11, 1, 2, 2, 2, 29, 0, 0 },
    { 25, 10, 4, 1, 29, 0, 0, 0 },
    { 12, 3, 29, 0, 0, 0, 0, 0 },
    { 11, 7, 26, 8, 29, 0, 0, 0 },
    { 27, 11, 7, 25, 10, 4, 29, 0 },
    { 26, 2, 2, 26, 10, 4, 29, 0 },
    { 27, 10, 4, 1, 29, 0, 0, 0 },
    { 9, 9, 8, 29, 0, 0, 0, 0 },
    { 25, 10, 4, 1, 29, 0, 0, 0 },
    { 11, 1, 24, 29, 0, 0, 0, 0 },
    { 2, 2, 3, 5, 29, 0, 0, 0 },
    { 11, 7, 2, 9, 9, 8, 29, 0 },
    { 12, 3, 3, 8, 5, 29, 0, 0 },
    { 29, 29, 29, 29, 0, 0, 0, 0 },
    { 29, 29, 29, 29, 0, 0, 0, 0 },
};
const s32 btaiPhase1Patterns[17][8] = {
    { 26, 10, 4, 4, 4, 29, 0, 0 },
    { 27, 12, 3, 3, 29, 0, 0, 0 },
    { 5, 4, 4, 4, 29, 0, 0, 0 },
    { 26, 11, 2, 2, 2, 29, 0, 0 },
    { 9, 8, 8, 2, 12, 8, 29, 0 },
    { 11, 7, 2, 27, 9, 9, 8, 29 },
    { 26, 9, 9, 8, 29, 0, 0, 0 },
    { 9, 5, 4, 29, 0, 0, 0, 0 },
    { 3, 11, 7, 9, 9, 8, 29, 0 },
    { 11, 1, 24, 29, 0, 0, 0, 0 },
    { 27, 6, 5, 29, 0, 0, 0, 0 },
    { 26, 10, 4, 4, 4, 29, 0, 0 },
    { 12, 3, 26, 8, 29, 0, 0, 0 },
    { 11, 7, 9, 9, 9, 8, 29, 0 },
    { 29, 29, 29, 29, 29, 0, 0, 0 },
    { 29, 29, 29, 29, 0, 0, 0, 0 },
    { 29, 29, 29, 29, 0, 0, 0, 0 },
};
const s32 btaiPhase2Patterns[16][8] = {
    { 27, 6, 9, 29, 0, 0, 0, 0 },
    { 26, 10, 4, 4, 4, 29, 0, 0 },
    { 27, 12, 3, 3, 3, 29, 0, 0 },
    { 11, 7, 27, 11, 29, 29, 0, 0 },
    { 26, 10, 4, 4, 4, 29, 0, 0 },
    { 12, 3, 9, 9, 8, 5, 29, 0 },
    { 11, 2, 2, 12, 5, 29, 0, 0 },
    { 27, 5, 4, 4, 4, 29, 0, 0 },
    { 12, 3, 3, 3, 8, 5, 29, 0 },
    { 27, 10, 4, 4, 4, 29, 0, 0 },
    { 27, 6, 8, 29, 0, 0, 0, 0 },
    { 9, 5, 4, 4, 29, 0, 0, 0 },
    { 11, 1, 24, 29, 0, 0, 0, 0 },
    { 11, 2, 7, 26, 10, 5, 29, 0 },
    { 27, 3, 3, 3, 5, 29, 0, 0 },
    { 29, 29, 29, 29, 0, 0, 0, 0 },
};
/* How many patterns each phase picks from. */
const s32 btaiPhasePatternCounts[4] = { 14, 14, 15, 0 };

/* The boss's next move: the next action of its pattern. When the pattern ends (29) or the
 * action ended early, it takes the next pattern and its phase from its life (under 2000: 0,
 * under 1400: 1, under 800: 2). */
s32 btaiNextAction(Obj *o) {
    Tlb324StepW *w;
    s32 next;

    btaiNewAction = 1;
    ((Tlb324StepW *) FX(o))->step = 0;
    ((Tlb324StepW *) FX(o))->move++;
    ((Tlb324StepW *) FX(o))->timer = 0;
    w = (Tlb324StepW *) FX(o);
    switch (gBossPhase) {
        case 0:
            next = btaiPhase0Patterns[w->pattern][w->move];
            break;
        case 1:
            next = btaiPhase1Patterns[w->pattern][w->move];
            break;
        case 2:
            next = btaiPhase2Patterns[w->pattern][w->move];
            break;
        default:
            next = 29;
            break;
    }
    if (next == 29 || w->endPattern == 1) {
        if (++w->pattern >= btaiPhasePatternCounts[gBossPhase]) {
            w->pattern = 0;
        }
        w->move = 0;
        w->endPattern = 0;
        if (gBossLife < 2000.0f) {
            gBossPhase = 0;
        }
        if (gBossLife < 1400.0f) {
            gBossPhase = 1;
        }
        if (gBossLife < 800.0f) {
            gBossPhase = 2;
        }
    }
    switch (gBossPhase) {
        case 0:
            w->action = btaiPhase0Patterns[w->pattern][w->move];
            break;
        case 1:
            w->action = btaiPhase1Patterns[w->pattern][w->move];
            break;
        case 2:
            w->action = btaiPhase2Patterns[w->pattern][w->move];
            break;
        default:
            PRINTF("外部インパクトが交代しました\n");
            PRINTF("Change\n");
            PRINTF("%d-> %d\n", w->pattern, w->move);
            PRINTF("しにデモ\n");
            PRINTF("ERROR ->%d\n", gBossPhase);
            break;
    }
    return 0;
}


extern s32 btaiEnding; /* the ending: 0 none, 1-3 (btaiEntranceDemo, btaiDeathDemo, btaiWinDemo) */
extern s32 btaiAction; /* the action this frame */

/* The file's .data, in address order (its externs above stay as declarations). */
s32 btaiWhirlBubbles = 0;
s32 btaiAction = 0; /* the action this frame */
s32 btaiTurned = 0; /* 1: the boss has turned round */
extern s32 btaiEffectsEnd; /* 1: the battle's effects end (tlb_0324_6) */

/* The boss's step: runs its action, or its ending once one began. It is beaten (ending 2)
 * when its life runs out, and wins (ending 3) when Impact's armour does, each outside
 * actions 13-22. When the battle's flag 8 is set (but in actions 6 and 24) it stops and
 * goes back to action 0. The actions are called through their addresses.
 * 284 lines: the game makes the work pointer in v0 for each block (addiu v0,a0,0x20) and
 * keeps gBossLife's address in a1; this C folds the pointer into o's offsets. A w local set
 * in each block folds the same and moves the globals' registers (327). 2026-10-11:
 * FX with a FAKE "^ 0" (tlb_0102's lever) for the whole function: 149, the pointer then made
 * once (a1) where the game makes it again in each block and folds the first read.
 * 2026-10-12 (second try, start 156): `(o + 1)` for FX(o): 156, same. One w local set at its
 * declaration: 37 (uopt remakes a0 + 0x20 in each block, as the game). The frame's bit 2 is
 * the Frame header's (f.flags, offset 0; this C had the AnimeFrame's 0x38: a real fix) 35; ret
 * declared before w (ret's home 0x1C) 3; btaiEffectsEnd stored before gBossLife: MATCH. */
s32 btaiRunAction(Obj *o) {
    s32 ret;
    Tlb324StepW *w = (Tlb324StepW *) FX(o);

    btaiAction = w->action;
    if (gBossLife <= 0) {
        gBossLife = 0;
    }
    if (gBossLife <= 0 && btaiEnding == 0
        && (w->action < 13 || w->action >= 23)) {
        btaiEffectsEnd = 1;
        gBossLife = 0;
        btaiEnding = 2;
        w->step = 0;
        gBossFlags |= 2;
    }
    if ((gImpactArmor <= 0 || gImpactArmor2 <= 0)
        && (w->action < 13 || w->action >= 23)
        && btaiEnding == 0) {
        btaiEffectsEnd = 1;
        btaiEnding = 3;
        gBossFlags &= ~0x4000;
        w->step = 0;
    }
    if ((gBattleFlags & 8) && w->action != 24
        && w->action != 6) {
        w->vel[0] = 0.0f;
        w->vel[2] = 0.0f;
        gBrotherFlags &= ~0x20000;
        gBattleFlags &= ~8;
        w->step = 0;
        w->action = 0;
        w->frame->f.flags |= 2;
    }
    switch (btaiEnding) {
        case 0:
            gBattleFlags2 &= ~0x200000;
            switch (w->action) {
                case 0:
                    ret = (*btaiNormal)(o);
                    break;
                case 1:
                    ret = (*btaiNeutral)(o);
                    break;
                case 2:
                    ret = (*btaiMarbleShot)(o);
                    break;
                case 3:
                    ret = (*btaiMarbleBarrage)(o);
                    break;
                case 4:
                    ret = (*btaiBiribiriRod)(o);
                    break;
                case 5:
                    ret = (*btaiRushFeint)(o);
                    break;
                case 6:
                    ret = (*btaiWhirlpoolAttack)(o);
                    break;
                case 7:
                    ret = (*btaiTorpedoAttack)(o);
                    break;
                case 8:
                    ret = (*btaiHeadbutt)(o);
                    break;
                case 25:
                    ret = (*btaiNormalSwayLong)(o);
                    break;
                case 26:
                    ret = (*btaiNormalSwayMid)(o);
                    break;
                case 27:
                    ret = (*btaiNormalSwayShort)(o);
                    break;
                case 9:
                    ret = (*btaiMoveBack)(o);
                    break;
                case 10:
                    ret = (*btaiMoveToCockpit)(o);
                    break;
                case 11:
                    ret = (*btaiMoveToBase)(o);
                    break;
                case 12:
                    ret = (*btaiMoveToEdge)(o);
                    break;
                case 13:
                    ret = (*btaiStraightHit)(o);
                    break;
                case 14:
                    ret = (*btaiJabHitNegX)(o);
                    break;
                case 15:
                    ret = (*btaiJabHitPosX)(o);
                    break;
                case 16:
                    ret = (*btaiHundredPunchHit)(o);
                    break;
                case 17:
                    ret = (*btaiKickHit)(o);
                    break;
                case 18:
                    ret = (*btaiCannonHit)(o);
                    break;
                case 19:
                    ret = (*btaiKnockedBack)(o);
                    break;
                case 20:
                    ret = (*btaiExternalImpactHit)(o);
                    break;
                case 21:
                    ret = (*btaiCannonKnockback)(o);
                    break;
                case 22:
                    ret = (*btaiDizzyKnockdown)(o);
                    break;
                case 23:
                    ret = (*btaiImpactChange)(o);
                    break;
                case 24:
                    ret = (*btaiBarrierAttack)(o);
                    break;
            }
            break;
        case 1:
            (*btaiEntranceDemo)(o);
            ret = 0;
            gBattleFlags2 |= 0x200000;
            break;
        case 2:
            ret = (*btaiDeathDemo)(o);
            gBattleFlags2 |= 0x200000;
            break;
        case 3:
            ret = (*btaiWinDemo)(o);
            gBattleFlags2 |= 0x200000;
            break;
    }
    return ret;
}
