#include "player.h"
#include "game.h"
#include "audio.h"
#include "camera.h"
#include "diveef.h"
#include "fade.h"
#include "fileres.h"
#include "gfxbuf.h"
#include "gfxmgr.h"
#include "heap.h"
#include "itex.h"
#include "managerpl.h"
#include "pl3d.h"
#include "pldive.h"
#include "plmain.h"
#include "plmodel.h"
#include "plmove.h"
#include "plmsg.h"
#include "plpick.h"
#include "plshot.h"
#include "plsound.h"
#include "plutil.h"
#include "shadow.h"
#include "tmanager.h"
#include "vram.h"

/* The player's task ("player", overlay 9): making one from an init block ("plMain_init"), its
 * handler (0 step, 1 destroy, 2 create, 4 where the model is, 5 where on the course, 7 hit,
 * 8 and 9 passed on to the action), each step's work, and the small questions other code asks
 * of a player's task. */


/* Message 6's argument (as plmove.c's). */
typedef struct PlPlace {
    /* 0x0 */ s16 zone;
    /* 0x4 */ f32 place;
    /* 0x8 */ f32 side;
    /* 0xC */ f32 height;
} PlPlace;

extern Frame *gFrmModelLayer; /* the frame players go under */
extern s16 gFrmKindAnime; /* the player model's frame kind */
extern s32 pl_dmg_root_frame; /* the damage boxes' kind */
/* Per character, the buffer sizes (in the .data after plshot.c). */
extern s32 gPlBuffSizeModel[];
extern s32 gPlBuffSizeTex[];
extern s32 gPlBuffSize2[];
extern s32 gPlBuffSize3[];
extern s32 gPlBuffSize4[];

/* plmain.c's .data (0x801A8720-0x801A87C0). */
PlInit gPlInitDefaults = { /* the defaults */
    0, 0, 0, 0, 1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -8.0f, 0, 0, 0,
};
f32 gPlTopSpeedTable[4] = { 2.6f, 2.6f, 3.0f, 3.0f }; /* per character: the fastest */
s32 gPlCostumeFlagsTable[4] = { 0, 0x20, 0x40, 0x80 }; /* per costume: flags2 */
f32 gPlJumpHeightTable[4] = { 27.0f, 27.0f, 31.5f, 31.5f }; /* per character: the jump height */
f32 gPlJumpCutTable[4] = { 2.0f, 2.0f, 3.0f, 3.0f }; /* per character */
/* Per buffer kind: the size, by character. Two arrays: plGetBuffAddr indexes the first past its
 * end, and names the second by itself (one array of 5 with &gPlBuffSizeTable[1] shares the base
 * register and does not match). */
s32 *gPlBuffSizeTable[1] = { gPlBuffSizeModel };
s32 *gPlBuffSizeTableTail[4] = {
    gPlBuffSizeTex, gPlBuffSize2, gPlBuffSize3, gPlBuffSize4,
};
extern s32 gPlMsgReplies[]; /* per player: the reply to its last message */
extern s32 gGameFrameCount; /* the frame counter */
extern s8 gSavePlayerPips[]; /* per player (10 bytes each): the first row of the HUD's pips */


/* Makes a player's task (and, the first time, the players' parent task). */
Obj *plCreate(Obj *parent, PlInit *init) {
    if (gGameWork.plManagerTask == NULL) {
        gGameWork.plManagerTask = fadeCreateRetry(parent, plManager, 1, 0, NULL);
    }
    if (init->index < 0 || init->index >= 4) {
        PRINTF("plCreate: ERROR\n");
        return NULL;
    }
    return fadeCreateRetry(gGameWork.plManagerTask, plMain, 6, 0, init);
}

PlInit *plMakeInit(PlInit *init, s32 index, s32 chara) {
    *init = gPlInitDefaults;
    init->index = index;
    init->chara = chara;
    init->topSpeed = gPlTopSpeedTable[chara];
    return init;
}

void plDestroy(s32 i) {
    Obj *o = gGameWork.players[i];

    if (i < 0 || i >= 4) {
        PRINTF("plDestroy: ERROR\n");
    } else {
        tskDestroy(o);
        aicamDeleteLookat(o);
        PRINTF("plDestroy: destroyed\n");
    }
}

/* The task's handler.
 * How it matched (2026-10-03, from 90 lines): plRestDecrement called with p alone, as
 * managerpl.c defines it (the decompiled ", 10" was case 1's multiplier for the 10-byte unk7A
 * entries, left in a1 at the call: 101, mix distance 1); the permuter's FAKE block split in case
 * 1 dropped (45); case 7's tests read the kind variable (39); the kind an s32, not a u8 (the
 * copy into v1 goes: a match). Earlier: msg is the s16 read from its home, lhu for the switch
 * and lh for the tail, which the plain `switch (msg)` gives (2026-10-17, object identical; it
 * was written "*(u16 *) &msg", plshot.c btarge's old form); p set at its declaration keeps
 * the task in its home (IDO_NOTES 132). */
s32 plMain(Obj *task, s16 msg, void *arg) {
    Player *p = PL(task);
    s32 m[11]; /* a message buffer (word 0 the kind, as plpick.c sends msg[3]); 44 bytes puts
                * m[0] at the game slot 0x30 in its 0x60 frame */
    s32 kind;

    switch (msg) {
        case 2:
            gGameWork.players[((PlInit *) arg)->index] = task;
            gPlMsgReplies[p->index] = 0;
            if (!plMain_init(task, arg)) {
                return -1;
            }
            plMoveResetButtonMap();
            break;
        case 0:
            plStep(p);
            break;
        case 1:
            PRINTF("plMain: player %d caught TM_DESTROY (life=%d)\n", p->index, gSaveData.players[p->index].life);
            if (p->flags2 & 0x100) {
                PRINTF("plMain: delete OMAKE player\n");
                gSaveData.players[p->index].chara = -1;
                gSaveData.players[p->index].life = 0;
                gSaveData.players[p->index].lives = 0;
                gSaveData.players[p->index].money = 0;
                gGameWork.charas[p->index] = 0xFF;
            } else if (gSaveData.players[p->index].life == 0) {
                plRestDecrement(p);
                gSaveData.players[p->index].life = 3;
                if (gSaveData.players[p->index].money < 100) {
                    gSaveData.players[p->index].money = 100;
                }
                gGameWork.charas[p->index] = p->chara;
            }
            frmDestroy(p->dmg);
            frmDestroy(p->model);
            frmDestroy(p->shadow);
            while (gfxGetPendingSwaps() != 0) {
            }
            heapFree((void *) p->pieceBuf);
            heapFree((void *) p->atkBoxBuf);
            heapFree((void *) p->zanzouBuf);
            plFreeBuff((void *) p->modelFile);
            plFreeBuff((void *) p->texFile);
            gGameWork.players[p->index] = NULL;
            break;
        case 4:
            ((Vec3f *) arg)->x = p->model->pos[0];
            ((Vec3f *) arg)->y = p->model->pos[1];
            ((Vec3f *) arg)->z = p->model->pos[2];
            break;
        case 5:
            ((PlPlace *) arg)->zone = p->zone;
            ((PlPlace *) arg)->place = p->place;
            ((PlPlace *) arg)->side = p->side;
            ((PlPlace *) arg)->height = p->height;
            break;
        case 7:
            kind = p->dmg->hitKind;
            if ((!(p->flags & 0x80) || kind < 0x75) && (p->ride == NULL || kind < 0x75)) {
                PRINTF("plMain(%d): DESTROY: kind:%d\n", p->index, kind);
                p->hitKind = kind;
                p->hitBy = (s32) arg;
                plActSend(p, 3, arg, NULL);
            }
            break;
        case 8:
            if (((s32 *) arg)[1] == 1) {
                m[0] = 2;
                plActSend(p, 5, m, NULL);
            }
            break;
        case 9:
            plActSend(p, 6, arg, p);
            return plMsgReplyTake(p);
    }
    return tskDefaultProc(task, msg, arg);
}

/* plMain_init: the player from its init block. 0 when out of memory.
 * How it matched (2026-10-03, from 168 lines): the history clear as two three-link chains (the
 * game stores rows 2,1,0 then 5,4,3, each chain's innermost first; 162); the two chains after the
 * facing if stored "= 0" as ints (the game's fresh "mtc1 zero,f0" at the join, not the shared
 * 0.0f; 140); the chain clear through a pointer local "b" (the game's "addiu v0,s0,0x284"
 * walker; 130, mix distance 0); "p->index = init->index" right after the chara store with the
 * old FAKE "if (i) {}" taken out (as1 then sinks the store below the floats as the game; 124);
 * then, measured on a scratch file that #includes the permuter's base.c, the buttons cleared as
 * one u16 chain "p->buttons = p->held = 0" (the ugen temp the game spends before the pieceBuf
 * test), the costume's flags2 before its flags, and the FAKE boundary before the ripple test
 * taken out (it spent two temps): a match.
 * The clear (the fakes pass, round 2, 2026-10-04): a plain for loop on the same counter j as
 * the later loops; its FAKE goto form went. uopt's loopunroll (its listing at -zdbug:1 prints
 * "EQ_INEQ" and "UNROLLING" for each loop it takes) leaves alone a loop whose counter is used
 * again after it: no unrolling and no "bne" test, so the clear keeps the game's "sltiu; bnez",
 * the queue and history loops their "slti; bnez", and only the last loop (the chain clear)
 * is unrolled by 4 with "bne", as in the game. With its own counter the clear was unrolled
 * (for) or got "bne" (goto): every try below had a counter of its own. The indexed form
 * "((s32 *) p)[j] = 0" swaps the counter and pointer registers (20 lines).
 * History of the earlier forms (the clear, the 0xE0 reload):
 * 2026-10-02: written as a goto loop the clear stays ROLLED (the first form that does), but IDO
 * turns its test into "bne i, 166" with the counter in v0 and the pointer in v1; the game keeps
 * "sltiu at, i, 166; bnez" with the pointer in v0 (388 -> 385 lines). The 0xE0 reload is not a
 * chain: "e8 = e0 = 0.6f", as fields or as array elements, stores f12 twice with no reload. The
 * index started from a variable ("j = 0; for (i = j; ...)", which keeps file.c fileLoadAttachment's
 * loop rolled) unrolls here (388). The worn costume is read once, signed (lb: costume is s8),
 * and the stage's unk9E/unk9F are read unsigned (lbu: u8), 2026-10-02 (383).
 * The bound on the left (IDO_NOTES 189), 2026-10-02: "for (i = 0; 0xA6 > i; i++)" unrolls (2
 * peeled, then by 4: 386); the goto form with "0xA6 > i" still becomes "bne" (383). The start
 * from a variable (IDO_NOTES 219) in the goto form: "bne" (383). An s32 i against the unsigned
 * sizeof bound: goto, "bne" (383); for, unrolled (386).
 * The 0xE0 reload (383 to 332, the splits sweep): one FAKE block boundary after the 0.6f store
 * stops IDO forwarding it to unk0E8. The function's own strings are all used, so no new dead
 * print can stand for it, but the init print itself can: moved from the top to right after the
 * 0.6f store, its dead "if (0)" block is that boundary, and the FAKE went (the fakes pass,
 * 2026-10-03, still a match; the string keeps its place between "player" and "no memory" in
 * .rodata). The costume read once into a local "c" (the game's lb v0, used for the
 * test and the index): 332 to 276. The clear as a for loop with a volatile store: unrolled
 * (320). A second FAKE boundary, between the unk23C store and its test (the game reloads it,
 * IDO_NOTES 232): 276 to 204 (the splits pass on top of the first); a third after "i = 0":
 * 281, not kept. The clear (204 to 190): a pointer q set before the counter gives the game's
 * registers (pointer v0, counter v1), and a FAKE boundary before "goto clear" (greedy splits'
 * next find) stops IDO turning "i < 166" into "bne" with 166 in a0: the loop is now the game's
 * instruction for instruction. "while (TRUE) { ...; if (i >= 166) break; }": bne (204). Then
 * left: the schedule of the stores after the loop and the t-registers after the first call
 * (both fixed 2026-10-03, above). The permuter's finds on this
 * base (chara/unk020 through PL(task): 192; the flag table through i: 276) do not carry over
 * to the real compile. Sweep B's find on the faithful base (2026-10-03, score 3225 → 2945)
 * joined 29 statements onto one line (176); narrowed: the unk0E8/unk0EC stores on one line (IDO
 * schedules by source line) 190 → 180, the queue loop on one line 180 → 176; the action-to-
 * costume block or the three allocations joined, no change. The joins sweep: the chain clear's
 * loop on one line too, 176 → 174. The greedy mode: a FAKE "if (i) {}" after the clear, 172.
 * The rechains sweep (2026-10-03): "p->startSide = p->side = init->side;" (both f32), 170; its
 * chain-joining step: the history clear's first two stores as one chain, 168.
 * 2026-10-03: the game spends ONE ugen temp (t9) invisibly between the index load and the
 * pieceBuf test. Zero chains of s32 fields ("p->zanzouPart = p->atkBoxPart = 0") spend none; the u16 and
 * s16 chains do (the buttons' chain stores its OUTER target first, so "buttons = held" for the
 * game's 0x48, 0x4A order). Not better on the way: "p->index = init->index" before the FAKE
 * "if (i) {}" (132: the FAKE held its store up), the zero stores on one line (148: as1 reversed
 * them), the PRINTF after the chara store, the baseGravity/gravity chain, "p->dragY = 1", "p->slope
 * = 0", the index through c, j or an initialised local. */
s32 plMain_init(Obj *task, PlInit *init) {
    Player *p = PL(task);
    s32 j;
    s32 c;
    s32 *q;
    u8 *b;

    task->name = "player";
    q = (s32 *) p;
    for (j = 0; j < sizeof(Player) / 4; j++) {
        *q++ = 0;
    }
    p->chara = init->chara;
    p->index = init->index;
    p->actionSteps = 0;
    p->totalSteps = 0;
    p->state = 0;
    p->phase = 0;
    p->buttons = p->held = 0;
    p->ceilingKind = 0;
    p->wallKind = 0;
    p->ground = 0;
    p->groundSurface = 0;
    p->baseGravity = -1.0714285f;
    p->gravity = -1.0714285f;
    p->slope = 0.0f;
    p->baseDrag = 0.6f;
    PRINTF("----\nplMain_init(%d): init->init_flags: %d\n", init->index, init->flags);
    p->drag = p->baseDrag; p->dragY = 1.0f;
    plInitJump(p);
    p->loadedPiece = -1;
    p->atkBoxPart = 0;
    p->zanzouPart = 0;
    p->pieceBuf = (s32) plGetBuffAddr(p, 2);
    p->atkBoxBuf = (s32) plGetBuffAddr(p, 3);
    p->zanzouBuf = (s32) plGetBuffAddr(p, 4);
    if (p->pieceBuf == 0 || p->atkBoxBuf == 0 || p->zanzouBuf == 0) {
        PRINTF("plMain_init: no memory\n");
        return 0;
    }
    p->action = init->action;
    p->defaultAction = init->defaultAction;
    p->prevAction = init->action;
    p->flags = init->flags;
    p->flags2 = init->flags2 | 8;
    if ((c = gSaveData.costume[p->chara][0]) != 0) {
        p->flags2 |= gPlCostumeFlagsTable[c];
        p->flags &= 0x7FFFFFFF;
    }
    p->flags3 = init->flags3;
    for (j = 0; j < 3; j++) { p->queue[j] = -1; }
    p->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
    p->shadow = shadowCreate();
    if (pl_model_set(p)) {
        return 0;
    }
    plAnimeSetCurAction(p);
    plFaceAnimeStart(p);
    p->box = NULL;
    p->dmg = acollDmgAddYZ(pl_dmg_root_frame, p->model, 0, 0, 0, 0, 0);
    if (p->dmg == NULL) {
        return 0;
    }
    p->dmg->power = (s32) task;
    p->ripple = plMoyaCreate(task);
    if (p->ripple == 0) {
        return 0;
    }
    p->zone = init->zone;
    p->place = init->place;
    p->height = init->height;
    p->startSide = p->side = init->side;
    for (j = 0; j < 4; j++) {
        PL_HIST(p)[0][j] = PL_HIST(p)[1][j] = PL_HIST(p)[2][j] = 0.0f;
        PL_HIST(p)[3][j] = PL_HIST(p)[4][j] = PL_HIST(p)[5][j] = 0.0f;
    }
    p->jumpPush = 0.0f;
    p->maxSpeed = init->topSpeed;
    p->maxFall = init->maxFallSpeed;
    p->lean = init->lean;
    p->facing = init->facing;
    p->roll = init->roll;
    if (p->flags & 0x10) {
        p->model->rot[1] = init->facing;
    } else {
        p->targetLean = p->lean;
        p->targetFacing = p->facing;
        p->model->rot[1] = vramCourseHeading(p->zone, p->place) + p->facing;
    }
    p->leanOffset = p->tilt = p->rollOffset = 0;
    p->modelOffset[0] = p->modelOffset[1] = p->modelOffset[2] = 0;
    plFormSizeSet(p, 0);
    p->probeDrop = 0.0f;
    p->model->scale[0] = 0.1f;
    p->model->scale[1] = 0.1f;
    p->model->scale[2] = 0.1f;
    vramCoursePointXZ(p->zone, p->place, p->side, &p->model->pos[0], &p->model->pos[2]);
    p->model->pos[1] = p->height;
    p->model->flags |= 0x20;
    p->animStatus = 0;
    p->charge = 0;
    p->releasedCharge = 0;
    p->seAltBits = 0x55555555;
    p->goal = 0;
    p->invincibleTimer = 0;
    p->blinkTimer = 0;
    p->ride = NULL;
    p->onigiriTimer = 0;
    p->partner = 0;
    p->wallObj = 0;
    p->clinger = NULL;
    p->goalSpeed = 0.0f;
    p->waterDepth = 0.0f;
    p->prevWaterDepth = 0.0f;
    p->toSurface[0] = 0.0f;
    p->toSurface[1] = 0.0f;
    p->toSurface[2] = 0.0f;
    b = (u8 *) &p->chain;
    for (j = 0; j < 20; j++) { b[j] = 0; }
    if (!(p->flags2 & 0x100)) {
        gGameWork.charas[p->index] = p->chara;
    }
    if (p->flags & 0x10000) {
        __diveStart(p);
        if (p->chara == 0 || p->chara == 1) {
            p->action = 0xE6;
            p->prevAction = 0xE6;
        }
    }
    plActSend(p, 0, NULL, NULL);
    if (!(p->flags & 0x40)) {
        if (p->flags & 0x10) {
            pl3dStep(p);
        } else {
            plMoveStep(p);
        }
    }
    if (p->shadow != NULL) {
        if (!(p->flags2 & 8)) {
            p->shadow->flags |= 2;
        } else {
            p->shadow->flags &= ~2;
        }
    }
    if (p->flags3 & 1) {
        plJoinSdSet(p);
        p->flags3 &= ~1;
    }
    if (gGameWork.sndStopAllFirst != 0) {
        sndPlay(1);
        gGameWork.sndStopAllFirst = 0;
    }
    if (gGameWork.restartSdCount != 0) {
        plRestartSdSet(p);
        gGameWork.restartSdCount--;
    }
    if (p->chara == 2 && !(p->flags & 0x10000)) {
        plHumStop(p);
    }
    return 1;
}

/* A step of the player.
 * The drowning roll is "(s32) (2.0f * speed) + 1 > r": the +1 is what loads r before the
 * trunc. */
void plStep(Player *p) {
    s32 r;

    plUpdateBlink(p);
    if (!(p->flags & 0x40)) {
        plMoveReadButtons(p);
        plActSend(p, 1, NULL, NULL);
        if (!(p->flags & 0x10000000)) {
            if (p->flags & 0x10) {
                pl3dStep(p);
            } else {
                plMoveStep(p);
            }
        }
        if ((p->flags & 0x10) && plLen2D(p->velX, p->velZ) > 3.2f && p->action == 4 &&
            !(p->actionSteps & 3)) {
            itexPuffWhite(p->model->pos[0], p->model->pos[1] + 1.5f, p->model->pos[2], 0.03f);
        }
        plAnimeStep(p);
        plCountInvincible(p);
        if (p->height < -1000.0f) {
            plActChange(p, 0x2E);
        }
        if (p->slowTimer != 0) {
            if (--p->slowTimer == 0) {
                p->flags &= ~0x08000000;
            }
        }
        if (p->waterObj != 0 && (p->flags & 0x10000) && p->waterDepth > 10.0f) {
            r = fadeRand(64);
            if ((s32) (2.0f * plSpeedLen(p)) + 1 > r) {
                plSuichuAwaCreate(gGameWork.players[p->index]);
            }
        }
        plMoyaPlace(p->ripple, p);
        if (p->eventBits & 0x10) {
            plLandGroundSound(p);
        }
        if (p->eventBits & 0x20) {
            plSound(p, (u16) plFootstepSe(p, 0x500));
        }
        if (p->eventBits & 0xF) {
            plAnimEventSounds(p);
        }
        p->eventBits = 0;
        if ((p->buttons & 0x2000) && !(p->flags & 0x10230E30)) {
            if (++p->charge == 10) {
                fadeCreateRetry(gGameWork.players[p->index], plkira, 1, 0, p);
            }
            if (p->chara == 3) {
                if (!(p->flags & 0x3030) && plIsCharged(p) && p->chain == NULL) {
                    p->chain = fadeCreateRetry(gGameWork.players[p->index], btargM,
                                                      1, 0x1AF, p);
                }
                if (p->chain != NULL) {
                    plSendTagMsg(p->chain, 0xB3CEC7A7, 0, 0);
                }
            }
        } else {
            plChargeRelease(p);
        }
        gPlMsgReplies[p->index] = 0;
        p->flags &= ~0x1000;
        if (p->chara == 1) {
            if (PL_HONI(p)[1] != 0) {
                if (--PL_HONI(p)[1] == 0) {
                    PL_HONI(p)[0] = 0;
                }
            }
        }
        if (p->onigiriTimer != 0) {
            p->onigiriTimer--;
        }
        if (p->pipsCooldown != 0) {
            p->pipsCooldown--;
        }
        if (p->lifeCooldown != 0) {
            p->lifeCooldown--;
        }
        plGroundKnockback(p);
        p->totalSteps++;
        p->actionSteps++;
    }
}

/* The jump: its speed up from the height, and the pull that stops it. */
void plInitJump(Player *p) {
    f32 k;

    p->jumpHeight = gPlJumpHeightTable[p->chara];
    p->jumpCut = gPlJumpCutTable[p->chara];
    k = (sqrt(1.1479591f - p->jumpHeight * -8.571428f) + 1.0714285f) * 0.5;
    p->jumpSpeed = k;
    p->jumpHoldSteps = -(k / -1.0714285f) - p->jumpCut;
}

/* plAskPos: where a player's task is on the course. */
void plAskPos(Obj *task, f32 *place, f32 *height, f32 *side, s32 *zone) {
    s32 i;

    if (task == NULL) {
        /* The second values of the prints are guesses (dead code). */
        PRINTF("plAskPos(): FATAL: illegal Task pointer: %08x, %d\n", task, 0);
        *place = 0.0f;
        *height = 0.0f;
        *side = 0.0f;
        *zone = 1;
        return;
    }
    for (i = 0; i < 4; i++) {
        if (task == gGameWork.players[i]) {
            *place = PL(task)->place;
            *height = PL(task)->height;
            *side = PL(task)->side;
            *zone = PL(task)->zone;
            return;
        }
    }
    PRINTF("plAskPos(): FATAL: illegal Task pointer: %08x (No player exists), %d\n", task, i);
    *place = 0.0f;
    *height = 0.0f;
    *side = 0.0f;
    *zone = 1;
}

void plGetModelPos(Obj *task, f32 *x, f32 *y, f32 *z) {
    *x = PL(task)->model->pos[0];
    *y = PL(task)->model->pos[1];
    *z = PL(task)->model->pos[2];
}

f32 plGetModelYaw(Obj *task) {
    return PL(task)->model->rot[1];
}

/* Whether player i is in play. */
s32 plIsInPlay(s32 i) {
    if (i < 0 || i >= 4) {
        return 0;
    }
    return (gGameWork.players[i] != NULL) ? 1 : 0;
}

/* Whether player i is powered down (flag 0x200). */
s32 plIsPoweredDown(s32 i) {
    Player *p;

    if (plIsInPlay(i)) {
        p = PL(gGameWork.players[i]);
        if (p->flags & 0x200) {
            return 1;
        }
    }
    return 0;
}

s32 plIsSwimming(s32 i) {
    return PL(gGameWork.players[i])->flags & 0x8000;
}

s32 plIsDiving(s32 i) {
    return PL(gGameWork.players[i])->flags & 0x10000;
}

ACollFrame *plGetDamageBox(Obj *task) {
    return PL(task)->dmg;
}

/* plExtDamage: a hit from outside (as message 3). */
void plExtDamage(Obj *task, s32 a, s32 kind) {
    PRINTF("plExtDamage(%d): %d\n", PL(task)->index, kind);
    PL(task)->hitKind = kind;
    PL(task)->hitBy = a;
    plHitOrItem(PL(task));
}

/* Which player a task is (-1: none). */
s32 plGetIndex(Obj *task) {
    s32 i;

    for (i = 0; i < 4; i++) {
        if (task == gGameWork.players[i]) {
            return i;
        }
    }
    return -1;
}

s32 plGetChara(Obj *task) {
    return PL(task)->chara;
}

/* Last step's push. */
void plGetLastPush(Obj *task, Vec3f *out) {
    out->x = PL(task)->lastPushX;
    out->y = PL(task)->lastPushY;
    out->z = PL_HIST(PL(task))[5][1];
}

/* This step's speed. */
void plGetSpeed(Obj *task, Vec3f *out) {
    out->x = PL(task)->velX;
    out->y = PL(task)->velY;
    out->z = PL(task)->velZ;
}

void plCountPlayers(void) {
    tskCountChildren(gGameWork.plManagerTask);
}

s32 plGetAction(Obj *task) {
    return PL(task)->action;
}

void plHalt(Obj *task) {
    PL(task)->flags |= 0x40;
}

void plContinue(Obj *task) {
    PL(task)->flags &= ~0x40;
}

/* The blink while flag 0x100 lasts (0x164 counts it down); flag 0x02000000 hides the model. */
void plUpdateBlink(Player *p) {
    s32 n;

    if (p->flags & 0x02000000) {
        p->model->f.flags &= ~2;
        return;
    }
    if (p->flags & 0x100) {
        if (gGameFrameCount & 1) {
            p->model->f.flags &= ~2;
        } else {
            p->model->f.flags |= 2;
        }
    }
    n = p->blinkTimer;
    if (n != -1 && n != 0) {
        p->blinkTimer = n - 1;
    }
    n = p->blinkTimer;
    if (n <= 0 || n == 0) {
        p->flags &= ~0x100;
        p->model->f.flags |= 2;
    }
}

void plCountInvincible(Player *p) {
    if (p->invincibleTimer != -1 && p->invincibleTimer != 0) {
        if (--p->invincibleTimer <= 0) {
            p->flags &= ~0x80;
        }
    }
}

/* Makes a player jump down (0xD7; 0xE3 for characters 2 and 3 when carried). */
s32 plEnterDemo(Obj *task) {
    s32 next;

    if (plGetIndex(task) == -1) {
        return 0;
    }
    next = 0xD7;
    if ((PL(task)->flags & 0x10000) && (PL(task)->chara == 2 || PL(task)->chara == 3)) {
        next = 0xE3;
    }
    return plSetAction(task, next);
}

/* The demo's walk: to goal at speed. */
void plDemoWalkTo(Obj *task, Vec3f goal, f32 speed) {
    *(Vec3f *) &PL(task)->goalX = goal;
    PL(task)->goalSpeed = speed;
    PL(task)->goal |= 1;
}

void plDemoWalkToPlace(Obj *task, f32 place, f32 speed) {
    s32 pad;
    Vec3f goal;

    goal.x = place;
    goal.y = 0.0f;
    goal.z = PL(task)->side;
    plDemoWalkTo(task, goal, speed);
}

void plSetPos(Obj *task, s32 zone, Vec3f *pos) {
    PL(task)->place = pos->x;
    PL(task)->height = pos->y;
    PL(task)->side = pos->z;
    PL(task)->zone = zone;
}

s32 plIsDemoWalking(Obj *task) {
    return (PL(task)->goal != 0) ? 1 : 0;
}

s32 plSetAction(Obj *task, s32 next) {
    plActChange(PL(task), next);
    return 1;
}

void plSlowDown(s32 i) {
    Player *p;

    if (plIsInPlay(i)) {
        p = PL(gGameWork.players[i]);
        p->slowTimer = 3;
        p->flags |= 0x08000000;
    }
}

/* Ends the power-down (flag 0x200) with one pip. The player pointer is set at its declaration
 * (it stays a variable, so the flags are tested through the task and reloaded through it). */
s32 plEndPowerDown(Obj *task) {
    Player *p = PL(task);

    if (PL(task)->flags & 0x200) {
        p->flags &= ~0x200;
        gSavePlayerPips[p->index * 10] = 1;
        plSetAction(task, 0);
        return 1;
    }
    return 0;
}

/* plGetBuffAddr: a buffer for the player's files, by kind (0 and 1 from the heap's top). */
void *plGetBuffAddr(Player *p, s32 kind) {
    s32 **tbl;
    s32 size;

    if (p->chara < 0 || p->chara >= 4 || kind < 0 || kind >= 5) {
        PRINTF("plGetBuffAddr: ???\n");
        return NULL;
    }
#ifndef AVOID_UB
    tbl = &gPlBuffSizeTable[kind]; /* kinds 1-4 run on into gPlBuffSizeTableTail (see above) */
#else
    tbl = (kind == 0) ? &gPlBuffSizeTable[0] : &gPlBuffSizeTableTail[kind - 1];
#endif
    size = (*tbl)[p->chara];
    if (kind == 0 || tbl == gPlBuffSizeTableTail) {
        return heapMallocTop(size);
    }
    return heapMalloc(size);
}

void plFreeBuff(void *buf) {
    if (buf != NULL) {
        if (resFindAddr(buf) == -1) {
            heapFree(buf);
        } else {
            resFreeAddr(buf);
        }
    }
}
