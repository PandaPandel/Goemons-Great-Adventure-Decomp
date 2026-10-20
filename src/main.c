#include "scheduler.h"
#include "game.h"
#include "audio.h"
#include "contpak.h"
#include "dma.h"
#include "file.h"
#include "fileres.h"
#include "gameproc.h"
#include "gfxmgr.h"
#include "heap.h"
#include "joypad.h"
#include "main.h"
#include "sys.h"
#include "tmanager.h"

/* The game's debug print, compiled to nothing but its arguments: the strings stay in .rodata,
 * in source order (IDO_NOTES 144, 201). The arguments are not known: no code is left. */
#define DPRINTF

/* The scheduler: built on the N64 SDK's sample scheduler (include/scheduler.h). It owns the
 * VI/SP/DP events, runs audio and graphics tasks on the RSP, and messages a list of clients. */

extern u8 gSchedBufferIndex; /* the current buffer index, as a byte */

extern u8 gSchedEventStackTop[];        /* stack tops of the four scheduler threads */
extern u8 gSchedAudioStackTop[];
extern u8 gSchedGfxStackTop[];
extern u8 gSchedSwapStackTop[];

/* Create the scheduler: its queues, the VI manager and events, and its four threads. */
void mainSchedCreate(Sched *sc, u8 viMode, u8 retraceCount) {
    sc->curGfxTask = NULL;
    sc->curAudioTask = NULL;
    sc->pendingGfxTask = NULL;
    sc->clients = NULL;
    sc->pendingSwaps = 0;
    sc->bufferIndex = 0;
    sc->viBlack = 1;
    sc->firstSwap = 1;
    sc->retraceMsg = 1;
    sc->prenmiMsg = 3;
    sc->unused8E0 = 0;
    sc->unused8E8 = 0;
    sc->unused8F0 = 0;
    sc->unused8F8 = 0;
    osCreateMesgQueue(&sc->eventQ, sc->eventBuf, 8);
    osCreateMesgQueue(&sc->spQ, sc->spBuf, 8);
    osCreateMesgQueue(&sc->dpQ, sc->dpBuf, 8);
    osCreateMesgQueue(&sc->gfxQ, sc->gfxBuf, 8);
    osCreateMesgQueue(&sc->swapQ, sc->swapBuf, 8);
    osCreateMesgQueue(&sc->audioQ, sc->audioBuf, 8);
    osCreateMesgQueue(&sc->gfxWaitQ, sc->gfxWaitBuf, 8);
    osCreateMesgQueue(&sc->waitQ, sc->waitBuf, 8);
    osCreateViManager(254);
    osViSetMode(&osViModeTable[viMode]);
    osViBlack(1);
    osViSetEvent(&sc->eventQ, (OSMesg) SCHED_RETRACE_MSG, retraceCount);
    osSetEventMesg(OS_EVENT_SP, &sc->spQ, (OSMesg) SCHED_SP_MSG);
    osSetEventMesg(OS_EVENT_DP, &sc->dpQ, (OSMesg) SCHED_DP_MSG);
    osSetEventMesg(OS_EVENT_PRENMI, &sc->eventQ, (OSMesg) SCHED_PRENMI_MSG);
    osCreateThread(&sc->eventThread, 19, (void (*)(void *)) mainSchedEventThread, sc, gSchedEventStackTop, 130);
    osStartThread(&sc->eventThread);
    osCreateThread(&sc->thread3B0, 18, (void (*)(void *)) mainSchedAudioThread, sc, gSchedAudioStackTop, 120);
    osStartThread(&sc->thread3B0);
    osCreateThread(&sc->thread560, 17, (void (*)(void *)) mainSchedGfxThread, sc, gSchedGfxStackTop, 100);
    osStartThread(&sc->thread560);
    osCreateThread(&sc->swapThread, 16, (void (*)(void *)) mainSchedSwapThread, sc, gSchedSwapStackTop, 110);
    osStartThread(&sc->swapThread);
}

OSMesgQueue *mainSchedGetAudioQ(Sched *sc) {
    return &sc->audioQ;
}

OSMesgQueue *mainSchedGetGfxQ(Sched *sc) {
    return &sc->gfxQ;
}

s32 mainSchedGetBufferIndex(Sched *sc) {
    return sc->bufferIndex;
}

void mainSchedEventThread(Sched *sc) {
    OSMesg msg = NULL;

    while (1) {
        osRecvMesg(&sc->eventQ, &msg, OS_MESG_BLOCK);
        switch ((s32) msg) {
            case SCHED_RETRACE_MSG:
                mainSchedBroadcast(sc, &sc->retraceMsg);
                break;
            case SCHED_PRENMI_MSG:
                mainSchedBroadcast(sc, &sc->prenmiMsg);
                break;
        }
    }
}

void mainSchedAddClient(Sched *sc, OSScClient *client, OSMesgQueue *msgQ) {
    OSIntMask mask = osSetIntMask(1);

    client->msgQ = msgQ;
    client->next = sc->clients;
    sc->clients = client;
    osSetIntMask(mask);
}

void mainSchedRemoveClient(Sched *sc, OSScClient *client) {
    OSScClient *cur = sc->clients;
    OSScClient *prev = NULL;
    OSIntMask mask = osSetIntMask(1);

    while (cur != NULL) {
        if (cur == client) {
            if (prev != NULL) {
                prev->next = client->next;
            } else {
                sc->clients = client->next;
            }
            break;
        }
        prev = cur;
        cur = cur->next;
    }
    osSetIntMask(mask);
}

void mainSchedBroadcast(Sched *sc, OSMesg msg) {
    OSScClient *client = sc->clients;

    while (client != NULL) {
        osSendMesg(client->msgQ, msg, OS_MESG_NOBLOCK);
        client = client->next;
    }
}

/* The audio-task thread: yield any running graphics task, run the audio task, then resume it. */
void mainSchedAudioThread(Sched *sc) {
    OSMesg msg = NULL;
    OSScTask *task = NULL;
    s32 pad[4]; /* unused; IDO still reserves the 16 bytes */
    OSScTask *gfxTask;
    u32 state = 0;

    while (1) {
        osRecvMesg(&sc->audioQ, (OSMesg *) &task, OS_MESG_BLOCK);
        osWritebackDCacheAll();
        gfxTask = sc->curGfxTask;
        if (gfxTask != NULL) {
            osSpTaskYield();
            osRecvMesg(&sc->spQ, &msg, OS_MESG_BLOCK);
            if (osSpTaskYielded(&gfxTask->list)) {
                state = 1;
            } else {
                state = 2;
            }
        }
        sc->curAudioTask = task;
        osSpTaskLoad(&task->list);
        osSpTaskStartGo(&task->list);
        osRecvMesg(&sc->spQ, &msg, OS_MESG_BLOCK);
        sc->curAudioTask = NULL;
        if (sc->pendingGfxTask != NULL) {
            osSendMesg(&sc->gfxWaitQ, &msg, OS_MESG_BLOCK);
        }
        if (state == 1) {
            osSpTaskLoad(&gfxTask->list);
            osSpTaskStartGo(&gfxTask->list);
        } else if (state == 2) {
            osSendMesg(&sc->spQ, &msg, OS_MESG_BLOCK);
        }
        osSendMesg(task->msgQ, task->msg, OS_MESG_BLOCK);
        state = 0;
    }
}

/* The graphics-task thread: wait until the task's framebuffer is off screen, let a running audio
 * task finish, run the task on the RSP/RDP, then hand it to the swap thread. */
void mainSchedGfxThread(Sched *sc) {
    OSMesg msg = NULL;
    OSScTask *task;
    OSScClient client;
    s32 pad[2]; /* unused; IDO still reserves the 8 bytes */

    while (1) {
        osRecvMesg(&sc->gfxQ, (OSMesg *) &task, OS_MESG_BLOCK);
        while (osViGetCurrentFramebuffer() == task->framebuffer ||
               osViGetNextFramebuffer() == task->framebuffer) {
            mainSchedAddClient(sc, &client, &sc->gfxWaitQ);
            osRecvMesg(&sc->gfxWaitQ, &msg, OS_MESG_BLOCK);
            mainSchedRemoveClient(sc, &client);
        }
        if (sc->curAudioTask != NULL) {
            sc->pendingGfxTask = task;
            osRecvMesg(&sc->gfxWaitQ, &msg, OS_MESG_BLOCK);
            sc->pendingGfxTask = NULL;
        }
        sc->curGfxTask = task;
        osSpTaskLoad(&task->list);
        osSpTaskStartGo(&task->list);
        osRecvMesg(&sc->spQ, &msg, OS_MESG_BLOCK);
        sc->curGfxTask = NULL;
        osRecvMesg(&sc->dpQ, &msg, OS_MESG_BLOCK);
        if (sc->viBlack != 0) {
            osViBlack(0);
            sc->viBlack = 0;
        }
        if (task->flags & OS_SC_SWAPBUFFER) {
            osSendMesg(&sc->swapQ, task, OS_MESG_BLOCK);
            sc->pendingSwaps--;
        }
        osSendMesg(task->msgQ, task->msg, OS_MESG_BLOCK);
    }
}

void mainSchedSwapThread(Sched *sc) {
    OSMesg dummy = NULL;
    OSScTask *msg;
    OSScClient client;

    while (1) {
        osRecvMesg(&sc->swapQ, (OSMesg *) &msg, OS_MESG_BLOCK);
        if (sc->firstSwap == 0) {
            while (osViGetCurrentFramebuffer() != osViGetNextFramebuffer()) {
                mainSchedAddClient(sc, &client, &sc->waitQ);
                osRecvMesg(&sc->waitQ, &dummy, OS_MESG_BLOCK);
                mainSchedRemoveClient(sc, &client);
            }
        } else {
            sc->firstSwap = 0;
        }
        osViSwapBuffer(msg->framebuffer);
    }
}

u32 mainSchedGetPending(Sched *sc) {
    return sc->pendingSwaps;
}

void mainSchedSendTask(Sched *sc, OSMesgQueue *mq, OSScTask *task) {
    OSIntMask mask;

    if (task->flags & OS_SC_SWAPBUFFER) {
        if (++sc->bufferIndex >= 3) {
            sc->bufferIndex = 0;
        }
        gSchedBufferIndex = sc->bufferIndex;
        mask = osSetIntMask(1);
        sc->pendingSwaps++;
        osSetIntMask(mask);
    }
    osSendMesg(mq, task, OS_MESG_BLOCK);
}

void mainSchedAddTwoPending(Sched *sc) {
    sc->pendingSwaps += 2;
}

/* Empty, with four NAMED parameters, which IDO spills to the caller's argument area. */
void mainUnusedStub(s32 unused0, s32 unused1, s32 unused2, s32 unused3) {
}

extern u8 gIdleThread[]; /* the idle thread (OSThread) */
extern u8 gBootThread[]; /* the top of its stack */

void main(void) {
    osInitialize();
    osCreateThread((OSThread *) gIdleThread, 1, mainIdleThread, NULL, gBootThread, 9);
    osStartThread((OSThread *) gIdleThread);
}

extern u8 gGameThread[]; /* thread 6; also the top of thread 5's stack */
extern u8 gPiMgrCmdQ[]; /* the PI command queue; also the top of thread 6's stack */
extern u8 gPiMgrCmdBuf[]; /* the PI command messages */

/* The idle thread: start the PI manager and threads 5 and 6, then idle forever at priority 0. */
void mainIdleThread(void *arg) {
    osCreatePiManager(150, (OSMesgQueue *) gPiMgrCmdQ, (OSMesg *) gPiMgrCmdBuf, 200);
    osCreateThread((OSThread *) gBootThread, 5, mainBootThread, arg, gGameThread, 10);
    osStartThread((OSThread *) gBootThread);
    osCreateThread((OSThread *) gGameThread, 6, mainGameThread, arg, gPiMgrCmdQ, 9);
    osStartThread((OSThread *) gGameThread);
    osSetThreadPri(NULL, 0);
    while (1) {
    }
}

extern OSMesgQueue gDmaMsgQ;
extern OSMesg gDmaMsgBuf[1];
extern OSMesgQueue gBootSchedMsgQ;     /* this thread's scheduler messages */
extern OSMesg gBootSchedMsgBuf[0x40];
extern OSMesgQueue gFrameDueMsgQ;     /* "a frame is due" */
extern OSMesg gFrameDueMsgBuf[1];
extern s8 gRetraceCount;              /* retraces since the last frame */
extern s8 gRetraceCountStore;             /* the same counter under two more names: see the note */
extern s8 gRetraceCountReset;
extern s8 gRetracesPerFrame;              /* retraces per frame */
extern OSMesg gFrameDueMsg;          /* the "frame due" message, and where thread 6 receives it */
extern u16 gPreNmiState;             /* set on PRE-NMI */
extern s32 gTskRoot;
extern u8 gAsciiFont[];  /* file 2's RAM: right after main's BSS */
extern u8 gOvlSharedSlot[];  /* the shared overlay slot (files 8, 9, 10), 0x47000 bytes */
extern u8 gArenaStart[];  /* the heap, right after that slot */
extern u8 gSndBankRam[];  /* file 7's RAM; the heap's end without the Expansion Pak */
extern u8 gHeapEnd[]; /* the same address, as the heap initialiser is given it */
extern u8 gZBuffer[];  /* file 4's RAM: a framebuffer */
extern u16 gFramebuffers[3][SCREEN_WIDTH * SCREEN_HEIGHT]; /* file 5's RAM: the framebuffers */
extern u8 gExpansionRam[];  /* the Expansion Pak's first byte */

/* Thread 5: boot the game, then count retraces and signal each frame; clean up on PRE-NMI.
 * Matched 2026-10-03. What the C needs, each measured:
 * - 0x800C7B10 under three names (file 3's RAM, file 6's RAM, overlay 6's process), so IDO
 *   rebuilds it at each use. gFile3Ram and gFile6Ram must be defined in
 *   undefined_syms.txt (= 0x800C7B10) for the build to link; the check resolves them by name.
 * - 0x80342080 under two names: the heap initialiser's end (gHeapEnd) and file 7's
 *   RAM (gSndBankRam). The file-7 address is carried across the if/else in s0 (built in branch 1,
 *   rebuilt at the end of branch 2) only because branch 1's size and the join's argument use
 *   the SAME u32 conversion of it, "(u32) gSndBankRam": that expression is what IDO PREs, and
 *   it takes s0 ahead of size (s1). With the pointer subtraction and a FAKE second use (the old
 *   18-line form) the address lost s0 to size; with u32 in the size only, the PRE was lost.
 * - The retrace counter under three names (load, store, reset), so nothing is hoisted into s2,
 *   and one s32 for the switch value and the counter (the counter takes v1).
 * How it was found, oldest first: (93 differing lines by permuter.py check; 20 of
 * them are sp offsets, see (3)): IDO
 * keeps three saved registers where the original keeps two. Settled, by measurement: osTvType is
 * unsigned (the original compares it with a throwaway `li at,1`); the PRE-NMI flag is u16 (four
 * saved registers became three); files 3 and 6 are registered at 0x800C7B10 through TWO symbols
 * of their own, not game_proc (one name is CSE'd into s0 across the three uses; the
 * game rebuilds it each time: 133 -> 111 lines); a one-byte local declared first puts client at
 * 0x3C and msg at 0x38 as the game has; the counter goes through an s8 local (`addiu; sll; sra;
 * sb; slt` on one register, no reload: 111 -> 105). File 7's RAM goes through ram7, set in each
 * branch from a DIFFERENT name of 0x80342080 (gSndBankRam / gHeapEnd), so IDO cannot
 * fold it into one constant at the join: the heap call's a1 is now built apart (lui a0 in the
 * beqz slot, addiu a0 in the jal slot), the size sits in s1, the else rebuilds 0x80342080 into
 * s0 at its end and the join passes s0, all as the game. Measured alternatives: one name for all
 * three uses, no ram7 (83 lines, but 0x80342080 held in s1 from BEFORE the heap call); ram7 set
 * from gSndBankRam in both branches (97: folded back to that); heap end as gArenaStart + 0x1877E0
 * (117: nothing held); ram7 set after the size at the end of branch 1 (111: address s1, size
 * s0, swapped). Still different:
 * (1) The counter gRetraceCount's address is held in s2 across the loop; the game rebuilds it with
 *     `lui` per access. In the game NO address is hoisted out of this loop (even osRecvMesg's
 *     gBootSchedMsgQ is rebuilt each pass, here too), while thread 6 (mainGameThread) hoists all seven.
 *     Because of 0(s2), as1 cannot lift the limit's `lb` above the counter's `sb`; the game has
 *     them together and its `lui t9,%hi(gRetracesPerFrame)` in the dispatch block before `lh v1`. No
 *     effect: volatile, `goto` for `while (1)`, `default: break;`, a dead PRINTF in `default:`, a
 *     FAKE `if (0) {}` between the store and the test, a file-scope `static` counter (it is used
 *     by no other function). Worse: gSchedGfxQ + 4 for it; `++gRetraceCount >= limit` (99,
 *     reloads it). Measured 2026-10-02: `static s8` and `static s8 ... = 0` are hoisted the same
 *     (s2 = .bss / .data); `((s8 *) &gSchedGfxQ)[4]` (83) CSEs with the gSchedGfxQ store and
 *     keeps that base in s2; a pointer local `p = &gRetraceCount;` in case 1 (91) makes only
 *     the reset store after osSendMesg absolute; FAKE `do {} while (0);` splits (one at the loop
 *     head: the switch value moves to v0; three in cases 1 and 3) change nothing (93). What does
 *     work, as a probe only: three NAMES for 0x800500C4 (load, store, reset; 48 lines). Nothing
 *     is hoisted, the frame is the game's 0x48 (msg 0x38, client 0x3C), and as1 then moves
 *     `lui t9,%hi(gRetracesPerFrame)` up into the dispatch block by itself, as the game has it. Left
 *     with that probe: the counter is in v0 (the game: v1, which is why its `lui v1` cannot
 *     move up: v1 still holds the switch value) and (2) below.
 * (2) Branch 1 builds 0x80342080 in v0, subtracts from v0, then `move s0,v0`; the game builds it
 *     straight into s0 and subtracts from s0. The extra move shifts every branch offset after it.
 *     No ram7, with gSndBankRam for the size and the join (117): IDO does not carry it across
 *     the join (no PRE insertion at the end of branch 2); it rebuilds it into a1 after the if.
 * (3) Frame: ram7's slot makes it 0x50 (ra 0x24, msg 0x40). By the frame arithmetic (computed,
 *     not measured), without s2 it is the game's 0x48: 40 bytes of locals over ra 0x1C, s1 0x18,
 *     s0 0x14, msg 0x38, client 0x3C. The game's locals are 10 words; without ram7 this C has 8.
 * Measured 2026-10-02: the four callees this file declares void but which return a value
 * (resDrop, contPakInitRumble, contPakRumbleRequestStop as s32, tskCreate as a pointer; gauge.c
 * gaugeJoinPlayer matched on such a prototype) change nothing here (93). Sweep A2's find
 * (2026-10-03, score 1936 → 1928), the switch on a copy of msg: 93, no change. The copies
 * sweep: msg set to NULL only after the first two calls (as a copy, or plainly): 77, but the
 * game stores that NULL in the first call's delay slot (the initializer at the declaration),
 * the frame and s2 stay as they were, and the drop is the diff aligning: not kept. The empties
 * sweep: a FAKE "if (arg) {}" after the first two calls, 93 → 75 and the mix distance 7 → 5
 * (kept); the frame (0x50) and s2 are unchanged. The greedy mode (75 to 67): client and msg
 * declared after size and i, which puts their addresses at the game's offsets; left the frame
 * (0x50 for 0x48) and s2.
 * 2026-10-03 (67 to 30): (1)'s three names adopted, as ram7's two (undefined_syms.txt): the
 * counter is stored as gRetraceCountStore and reset as gRetraceCountReset, so nothing is hoisted
 * into s2 and the frame is the game's 0x48 (this C now the game's length, mix distance 2); with
 * it, client and msg back after cnt (the greedy order was for the 0x50 frame). Left: the counter
 * in v0 (the game v1), (2), and arg's home (sw a0,0x48), which the FAKE "if (arg) {}" costs
 * (without it: 48). The counter as one chained test, "if ((store = load + 1) >= limit)": 95.
 * Greedy on this form: nothing kept. The permuter's FAKE boundary between osRecvMesg and the
 * switch: 34 (the switch value moves to v0; the game has it and the counter both in v1).
 * 2026-10-03: the FAKE "if (arg) {}" moved from after the first two calls to just
 * before the loop: arg is now homed at entry (sw a0,0x48) as the game has it (48 lines, but only
 * because the missing home no longer offsets (2)'s extra move; the diff is now (2) and the
 * counter only, mix distance 2). The splits sweep on that form: no boundary helps (48 best; four
 * around ram7 give 60). (2), measured: ram7 as a u32 ("(u32) gSndBankRam", the call casting it
 * back): 48, no change; size written from gSndBankRam directly instead of from ram7: 48, no
 * change (the address is still a temp in v0 copied into s0). The counter: cnt made an s32 that
 * is ALSO the switch value, "switch (cnt = *msg) { case 1: cnt = (s8) (gRetraceCount + 1);":
 * one live range, so the counter takes the switch value's v1 as the game has it and its lui
 * stays in case 1 (30 lines, every one of them now (2): the extra move and the branch offsets
 * it shifts, mix distance 2). Then (2) as IDO's PRE, no ram7 ("size = gSndBankRam - ...;" in
 * branch 1, "fileLoadNow(7, gSndBankRam)" after the join): 66, the address rebuilt into
 * a1 after the join (no PRE). The same plus a FAKE second use of the address at the END of
 * branch 1, "if (!&gSndBankRam[size]) {}": 64, but PRE now happens exactly as the game has it
 * (the address in a saved register from branch 1 to the join, rebuilt at the end of branch 2
 * just before "li a0,7"), with two faults: the FAKE test is emitted (addu; bnezl; it sits
 * before the else's jump, so it is not a branch to the next block), and the address takes s1
 * and size s0 (the game: address s0, size s1). This is most likely the original's mechanism:
 * a second use of the address in branch 1 that generates no code. The FAKE moved to right after
 * the size statement and made address-only, "if ((u32) gSndBankRam == 4) {}" (no size in it;
 * mid-block, so the empty test is a branch to the next block and is dropped): 18 lines, mix
 * distance 0, KEPT. The instructions are now all the game's; the only difference is the swap
 * of the address and branch 1's size (this C: address s1, size s0; the game: address s0,
 * size s1; branch 2 is the game's: size s0, &osMemSize s1). ram7 is declared but unused
 * (its slot keeps the frame). Two uses in that FAKE, "if (((u32) A == 4) | ((u32) A == 8))":
 * 18, no change (so not a priority tie between the address and size); three uses: 18, no
 * change. Branch 2's size as a variable of its own (expSize, in ram7's old declaration slot),
 * so the two branches' sizes cannot share one live range: 18, no change (kept: no dead local).
 * Branch 1 with no size variable, the expression written twice in the call (a CSE of the whole
 * subtraction instead of the FAKE): 66, no PRE (the address rebuilt into a1 after the join),
 * as the plain no-ram7 form. The FAKE moved before the size statement: 18, identical code.
 * Sweeps on the 18 form: splits (59 boundaries) and empties (102 tests): none below 18.
 * Left: why uopt gives branch 1's size s0 and the address s1 (the game the reverse).
 * 2026-10-03: eight uses of the address in the FAKE (eight ORed compares): 18, no
 * change; so extra occurrences of the address inside one block do not raise its priority (they
 * are one local CSE; a named variable's loads would each count, IDO_NOTES 246). Branch 1 with
 * no size variable (the subtraction written twice in the call) AND the FAKE: 18, identical
 * code (the CSE temp takes s0 as the variable did; not kept: size reads as the original would
 * have it). Branch 2 the same way
 * (no expSize, "osMemSize - 0x400000" twice): 48, the load and the 0x400000 not CSE'd, and
 * &osMemSize moves to s0; the address stays in s1 (reverted). Sweeps on the 18 form: copies
 * (59) and swaps (30), none below 18. PROBE (not code to keep): size taken out of the saved
 * registers ("heapInit(sysArenaAllocBottom(0), <the subtraction>)", so it lives in
 * a1 only): the address then takes s0 (42 lines, all from the probe's own changes). So it is
 * size, coloured BEFORE the address, that pushes the address to s1; not expSize or &osMemSize
 * in branch 2. Needed: the address coloured first. Six FAKE address tests in a row after the
 * size statement (more blocks for size, more uses of the address): 18, no change, so this is
 * not the priority (uses over blocks) of IDO_NOTES 246. From the measurements here and in the
 * ram7 forms: variables are coloured before compiler temps (expSize before &osMemSize; size,
 * as a variable or as the CSE of the subtraction, before the address), and two variables in
 * the order of their first assignment (ram7 before size: ram7 s0). Sweep: joins (31), none
 * below 18. The ram7 variable form again on the current C (ram7 in expSize's slot, size shared by
 * both branches, "ram7 = gSndBankRam; do {} while (0); size = ram7 - ...", the else ending
 * "ram7 = gHeapEnd;"): 42, the boundary does not stop the address being propagated
 * into the subtraction (still built in v0 and copied), and now ram7 s1, size s0 (reverted).
 * The subtraction as a CSE (no size) with the two names of 0x80342080 swapped (heap init
 * given gSndBankRam; the FAKE, the subtraction and the join gHeapEnd), in case the
 * order of compiler temps follows the symbol: 18, identical (reverted). Pads sweep (54
 * variants: declaration order and pads): 18 best, so declaration order does not decide it.
 * A dead initializer "u32 size = 0;" (an earlier first appearance): 18, no change.
 * THE PARAMETER AS FILE 7'S RAM (no FAKE at all): "arg = gSndBankRam; size = (u8 *) arg -
 * gOvlSharedSlot - 0x47000;" in branch 1, "arg = gHeapEnd;" at the end of branch 2,
 * "fileLoadNow(7, arg)", and no "if (arg) {}": 30 lines, mix distance 2. The registers
 * are now the game's (address s0, size s1), arg is homed at entry (sw a0,0x48) with no FAKE,
 * as the game: so the original most likely reused arg here (a parameter is coloured first).
 * Left: the old ram7 fault, the address built in v0, subtracted from v0, then "move s0,v0"
 * (IDO propagates the address into the subtraction); one instruction more, every branch after
 * it offset. The same with "arg = gSndBankRam;" written AFTER the size statement (so the
 * address's last use in the block is the copy into arg): 42, the move stays and the
 * registers swap back (size s0, arg s1): the first assignment decides the order. The C below
 * is the 18-line form; the next lever to find is how the original kept the address out of
 * the subtraction's own temp with arg assigned first (arg form, 30 lines).
 * 2026-10-03: arg assigned at the END of branch 1 (after the two calls, from
 * gSndBankRam) and of branch 2 (gHeapEnd), passed at the join, no FAKE at all: 46,
 * mix distance 8 (address s1 / size s0 as the 18 form, but no PRE: the join rebuilds 0x80342080
 * into a1 and arg is not homed). Reverted. "fileLoadNow(7, gSndBankRam);" written at the
 * end of BOTH branches (no FAKE; in case uopt sinks a common tail call): 46, mix distance 2, no
 * tail merge (each branch keeps its own call; address s1 / size s0 still). Reverted. The arg
 * form re-measured: 30, mix distance 2, as noted; its branch 1 is "lui v0; lui t7; addiu v0;
 * addiu t7; lui at; ori at; subu s1,v0,t7; addu s1,s1,at; move s0,v0" (the game: the address
 * built in s0, its addiu after t7's, subu s1,s0,t7, no move). Reading of all the colourings so
 * far: VARIABLES take saved registers in the order of their first assignment (arg before size:
 * arg s0; size before arg: size s0), and before any compiler temp (size before the CSE'd
 * address; expSize before &osMemSize); a constant's CSE temp loses to size even as a temp. The
 * game's address is a temp (the subtraction reads its register, no copy) coloured BEFORE size,
 * which none of these rules gives: the open question. No size variable (the subtraction written
 * twice in the call, a temp) WITH the FAKE address test moved BEFORE it (so the address's temp
 * is seen first): 18, identical (reverted); the order of compiler temps is not their first
 * occurrence either. A DEAD store "arg = gSndBankRam;" after the size statement as the second
 * use (no FAKE in branch 1, no "if (arg) {}"): 66, mix distance 8: the dead store is removed
 * before PRE (the join rebuilds the address into a1) and arg is not homed. Reverted. The size
 * held in the PARAMETER ("arg = (void *) (<the subtraction>);", the calls on (u32) arg), the
 * FAKE address test kept, no "if (arg) {}": 18, identical code, and arg is homed at entry with
 * no FAKE (an assigned parameter is homed); a parameter is coloured before the temp too. That
 * form with the FAKE moved BETWEEN the two heap calls (the first call's result through size, so
 * the address has a use in the block after sysArenaAllocBottom's call): 18, identical. The embedded
 * assignment "size = (u8 *) (arg = gSndBankRam) - gOvlSharedSlot - 0x47000;" (arg = file 7's RAM,
 * the else ending "arg = gHeapEnd;", no FAKE at all): 30, exactly the arg form
 * (address in v0, "move s0,v0"): IDO propagates the address out of any variable it is assigned
 * to, so a variable never holds it the way the game's s0 does. All reverted: the C below is the
 * 18-line form. Left as before: why the game's address temp beats size to s0.
 * 2026-10-03 (the unfakes mode): the old FAKE "if (arg) {}" after the boot calls no longer does
 * anything (18, the same code and frame without it): removed.
 * 2026-10-03: commutes sweep: one swap, none below 18. A second FAKE address test
 * at the join's head, before "fileLoadNow(7, ...)": 48, length 217, mix distance 8 (the
 * PRE is lost: the join gets its own address); reverted. The size as u32 arithmetic,
 * "(u32) gSndBankRam - (u32) gOvlSharedSlot - 0x47000": 48, length 217, the PRE lost again (the
 * address a v0 temp in the subtraction): the pointer subtraction is what pairs with the FAKE's
 * (u32) use; reverted. The same u32 subtraction AND the join's argument written as the same
 * u32 expression, "fileLoadNow(7, (void *) (u32) gSndBankRam)": 2 lines, mix distance 0,
 * KEPT: the PRE'd expression is now the (u32) conversion of the address, the address takes s0
 * and size s1 as the game. Left: one addiu's place (the game: lui s0; lui t7; addiu t7; addiu
 * s0; this C: lui s0; lui t7; addiu s0; addiu t7). That form with the FAKE address test taken
 * out (no FAKE left in the function): MATCH. */
extern u8 gFile3Ram[]; /* file 3's RAM (the same address as overlay 6's) */
extern u8 gFile6Ram[]; /* file 6's RAM: overlay 6 */
void mainBootThread(void *arg) {
    s32 cnt;
    OSScClient client;
    s16 *msg = NULL;
    u32 size;
    s32 i;
    u32 expSize;

    dmaInitHandles();
    sysInitGame();
    fileUnmapAllTlb();
    dmaInitGfxTasks();
    osCreateMesgQueue(&gDmaMsgQ, gDmaMsgBuf, 1);
    osCreateMesgQueue(&gBootSchedMsgQ, gBootSchedMsgBuf, 0x40);
    osCreateMesgQueue(&gFrameDueMsgQ, gFrameDueMsgBuf, 1);
    if (osTvType == 1) {
        DPRINTF("**** osTvType = NTSC ****\n");
        mainSchedCreate(&gMainSched, 0, 1);
    } else {
        DPRINTF("**** osTvType = MPAL ****\n");
        mainSchedCreate(&gMainSched, 28, 1);
    }
    mainSchedAddClient(&gMainSched, &client, &gBootSchedMsgQ);
    gSchedGfxQ = mainSchedGetGfxQ(&gMainSched);
    osViSetSpecialFeatures(0x4A);
    if (osMemSize < 0x800000) {
        sysArenaSet(gArenaStart, gHeapEnd);
        size = (u32) gSndBankRam - (u32) gOvlSharedSlot - 0x47000;
        heapInit(sysArenaAllocBottom(size), size);
    } else {
        DPRINTF("!!!! 増設メモリ発見 !!!!\n");
        sysArenaSet(gExpansionRam, gExpansionRam + osMemSize - 0x400000);
        expSize = osMemSize - 0x400000;
        heapInit(sysArenaAllocBottom(expSize), expSize);
    }
    DPRINTF("＜システムバッファ転送＞\n");
    fileLoadNow(7, (void *) (u32) gSndBankRam);
    fileLoadNow(4, gZBuffer);
    fileLoadNow(2, gAsciiFont);
    fileLoadNow(5, gFramebuffers);
    DPRINTF("＜メインプログラム転送＞\n");
    fileLoadNow(3, gFile3Ram);
    fileLoadNow(6, gFile6Ram);
    resDrop(7);
    resDrop(4);
    resDrop(2);
    resDrop(5);
    resDrop(3);
    resDrop(6);
    tskInit();
    tskCreate(gTskRoot, (void *) joyProc, 0, 0);
    tskCreate(gTskRoot, (void *) game_proc, 0, 0);
    tskCreate(gTskRoot, (void *) frmGraphicManagerProc, 0, 0);

    while (1) {
        osRecvMesg(&gBootSchedMsgQ, (OSMesg *) &msg, OS_MESG_BLOCK);
        switch (cnt = *msg) {
            case 1:
                cnt = (s8) (gRetraceCount + 1);
                gRetraceCountStore = cnt;
                if (cnt >= gRetracesPerFrame) {
                    osSendMesg(&gFrameDueMsgQ, &gFrameDueMsg, OS_MESG_NOBLOCK);
                    gRetraceCountReset = 0;
                }
                break;
            case 2:
                break;
            case 3:
                sndCommand(10);
                for (i = 0; i < 4; i++) {
                    contPakInitRumble(i);
                    contPakRumbleRequestStop(i);
                }
                contPakRumbleSuspend();
                gPreNmiState = 1;
                break;
        }
    }
}

extern u8 gFileLoaderBusy; /* set while the file loader runs; LoadFile checks it */

/* Thread 6: the game's frame loop. */
void mainGameThread(void *arg) {
    while (1) {
        osRecvMesg(&gFrameDueMsgQ, &gFrameDueMsg, OS_MESG_BLOCK);
        if (gPreNmiState != 0) {
            mainResetWipe();
        } else {
            dmaBeginFrame(gGameState.fbIndex);
            tskSendMsg(gTskRoot, 0, 0);
            if (gGameState.skipDraw != 2 && mainSchedGetPending(&gMainSched) < 2) {
                dmaSubmitFrame();
                gGameState.fbIndex++;
                if (gGameState.fbIndex >= 3) {
                    gGameState.fbIndex = 0;
                }
            }
            gFileLoaderBusy = 1;
            fileServiceQueue();
            gFileLoaderBusy = 0;
            gGameState.frameCount++;
        }
    }
}

extern u16 gResetWipeStep;   /* the wipe's step, counted from 0 */
extern u16 gFbPixelSize;   /* the color image's pixel size (G_IM_SIZ_*) */
extern Gfx *gDisplayListHead;  /* the display-list head */

/* Reset (PRE-NMI): the "blinds" wipe. On the first step, copy the frame on screen into the other
 * two framebuffers. Then, each frame, fill black bars that widen by one pixel a step: vertical
 * ones every 16 pixels, then horizontal ones. At step 17 the Y scale goes back to 1.0, and once
 * the wipe is past step 19 and the reset is due, the PRE-NMI flag becomes 2. */
void mainResetWipe(void) {
    s16 i;
    s16 pending;
    s16 lo;
    s16 hi;

    if (!(gResetWipeStep & 3)) {
        for (i = 0; i < 4; i++) {
            contPakInitRumble(i);
        }
    }
    for (i = 0; i < 4; i++) {
        contPakMotorSet(i, 0, 1);
    }
    gRetracesPerFrame = 1;
    pending = mainSchedGetPending(&gMainSched);
    if ((pending == 0 && gResetWipeStep == 0) ||
        (pending < 2 && gResetWipeStep != 0 && gResetWipeStep < 17)) {
        if (!gResetWipeStep) {
            switch (mainSchedGetBufferIndex(&gMainSched)) {
                case 0:
                    heapMemCopy(gFramebuffers[0], gFramebuffers[2], sizeof(gFramebuffers[0]));
                    heapMemCopy(gFramebuffers[1], gFramebuffers[2], sizeof(gFramebuffers[0]));
                    break;
                case 1:
                    heapMemCopy(gFramebuffers[1], gFramebuffers[0], sizeof(gFramebuffers[0]));
                    heapMemCopy(gFramebuffers[2], gFramebuffers[0], sizeof(gFramebuffers[0]));
                    break;
                case 2:
                    heapMemCopy(gFramebuffers[0], gFramebuffers[1], sizeof(gFramebuffers[0]));
                    heapMemCopy(gFramebuffers[2], gFramebuffers[1], sizeof(gFramebuffers[0]));
                    break;
            }
        }
        gResetWipeStep++;
        dmaBeginDisplayList(gGameState.fbIndex);
        gDPSetScissor(gDisplayListHead++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        gDPSetCycleType(gDisplayListHead++, G_CYC_FILL);
        gDPSetColorImage(gDisplayListHead++, G_IM_FMT_RGBA, gFbPixelSize, SCREEN_WIDTH,
                         VIRTUAL_TO_PHYSICAL(gFramebuffers[mainSchedGetBufferIndex(&gMainSched)]));
        gDPSetFillColor(gDisplayListHead++, 0x00010001);
        for (i = 8; i < SCREEN_WIDTH + 1; i += 16) {
            lo = i - gResetWipeStep / 2;
            hi = i + gResetWipeStep / 2;
            if (lo < 0) {
                lo = 0;
            }
            if (lo >= SCREEN_WIDTH) {
                lo = SCREEN_WIDTH - 1;
            }
            if (hi >= SCREEN_WIDTH) {
                hi = SCREEN_WIDTH - 1;
            }
            gDPFillRectangle(gDisplayListHead++, lo, 0, hi, SCREEN_HEIGHT - 1);
        }
        for (i = 8; i < SCREEN_HEIGHT + 1; i += 16) {
            lo = i - gResetWipeStep / 2;
            hi = i + gResetWipeStep / 2;
            if (lo < 0) {
                lo = 0;
            }
            if (lo >= SCREEN_HEIGHT) {
                lo = SCREEN_HEIGHT - 1;
            }
            if (hi >= SCREEN_HEIGHT) {
                hi = SCREEN_HEIGHT - 1;
            }
            gDPFillRectangle(gDisplayListHead++, 0, lo, SCREEN_WIDTH - 1, hi);
        }
        gDPPipeSync(gDisplayListHead++);
        dmaSubmitFrame();
        gGameState.fbIndex++;
        if (gGameState.fbIndex >= 3) {
            gGameState.fbIndex = 0;
        }
        if (gResetWipeStep == 17) {
            osViSetYScale(1.0f);
            sndCommand(1);
        }
    }
    if (gPreNmiState == 1 && gResetWipeStep >= 19 && osAfterPreNMI() == 0) {
        gPreNmiState = 2;
    }
}

/* This file's .data (subsegment 0x25140), in ROM order. */
OSMesg gFrameDueMsg = NULL;
s32 gMainUnusedWord = 0; /* not referenced: 4 bytes before the next u16 */
u16 gPreNmiState = 0;
u16 gResetWipeStep = 0;
