#include "object.h"
#include "game.h"
#include "file.h"
#include "audio.h"
#include "cache.h"
#include "dma.h"
#include "face.h"
#include "fileres.h"
#include "gfxbuf.h"
#include "gfxmgr.h"
#include "joypad.h"
#include "menu.h"
#include "msgfrm.h"
#include "scenario.h"
#include "string_c.h"
#include "swwork.h"
#include "tmanager.h"
#include "window.h"

/* The scenario runner ("scRun"): the game's event scripts (talks, shops, menus).
 * - A scenario is a file (gScenarioFiles[n]) with a start (gScenarioStarts[n]). Its commands are 8 bytes:
 *   an op and an argument. Ops: 4 set a variable (then 9 =, 19 +=, 20 -=, 21 = *, 22/23 =,
 *   24 |=, 25 &=), 6 compare a variable (2 !=, 3 ==, 12 >=, 13 <=, 14 <, 15 >: true skips the
 *   next command), 10 jump, 11 call, 16 show a message, 17 jump by a menu's choice, 18 nothing,
 *   32/33 set/clear an event flag, 34/35 jump if a flag is set/clear, 8 end.
 * - A message is a list of shorts ending in -1: character codes, and negative controls
 *   (-20 open a window, -22 close it, -21/-23 wait for A, -24 new line, -25/-26 open and close
 *   a face).
 * - 35 script variables live in the common scenario file (gScenarioFiles[0]) at 0x08000000-
 *   0x08000088: the player, window numbers and kinds, positions, timers and so on.
 * - Two parent tasks ("Sc親") hold the message, window, face and menu tasks; two runners
 *   (scRun_proc, scRun_proc2) run a scenario, the second tearing down over 7 steps. */

/* A runner's work. */
typedef struct ScWork {
    /* 0x00 */ u8 flags; /* 1: a window was opened */
    /* 0x01 */ u8 player;
    /* 0x02 */ u8 plKind;
    /* 0x04 */ s32 no; /* the scenario */
    /* 0x08 */ ScCmd *pc;
    /* 0x0C */ s16 *text; /* the message being shown */
    /* 0x10 */ u8 state; /* 0 run commands, 1 show a message, 3 wait, 4 menu, 5 end */
    /* 0x14 */ u8 pad14[0x8];
    /* 0x1C */ Obj *str[3]; /* the message string tasks */
    /* 0x28 */ Obj *win[3]; /* the window tasks */
    /* 0x34 */ Obj *face[10];
    /* 0x5C */ Obj *menu;
    /* 0x60 */ Frame *frames[4];
    /* 0x70 */ u8 phase; /* 0 start, 1 run, 2 tear down */
    /* 0x71 */ u8 timer;
} ScWork;

/* What a runner is made with. */
typedef struct ScArg {
    /* 0x0 */ u8 unused0;
    /* 0x1 */ u8 player;
    /* 0x2 */ u8 plKind;
    /* 0x4 */ s32 no;
    /* 0x8 */ u8 pad8[0x68]; /* never set or read: the argument has its runner's work's size */
} ScArg; /* size 0x70 */

/* A parent's work: the cache its per-frame calls keep. */
typedef struct ScParent {
    /* 0x0 */ struct Cache *cache;
} ScParent;

extern u16 gScenarioFiles[]; /* the scenario files (the first holds the variables) */
extern u32 gScenarioStarts[]; /* and their starts */
/* scenario.c's .data (0x80109790-0x801097A0), in ROM order. */
Obj *gScRunner = NULL; /* the runner */
Obj *gScParent = NULL; /* the parent */
FileList gScFileList[] = { { FILELIST_LAST | 0x148, NULL } };
extern Frame *gFrmLayers[]; /* the frame pools */
extern Frame *gFramePool10;
extern s16 gFrmKindCore;
extern s8 gGameFbIndex; /* the framebuffer being drawn */
extern s8 gPlayer1Chara; /* the four players' characters */
extern s8 gPlayer2Chara;
extern s8 gPlayer3Chara;
extern s8 gPlayer4Chara;
extern u8 scFileVarPlayer[]; /* the variables' addresses in the common scenario file */
extern u8 scFileVar04[];
extern u8 scFileVarCancelOn[];
extern u8 scFileVarFastText[];
extern u8 scFileVarWaitTimer[];
extern u8 scFileVarMenuId[];
extern u8 scFileVarWinKind[];
extern u8 scFileVarWinMethod[];
extern u8 scFileVarWinX[];
extern u8 scFileVarWinY[];
extern u8 scFileVarTextWait[];
extern u8 scFileVarFaceNo[];
extern u8 scFileVarFaceKind[];
extern u8 scFileVar34[];
extern u8 scFileVar38[];
extern u8 scFileVar3C[];
extern u8 scFileVar40[];
extern u8 scFileVarCharSize[];
extern u8 scFileVarCharXGap[];
extern u8 scFileVarCharYGap[];
extern u8 scFileVarCharAlpha[];
extern u8 scFileVar54[];
extern u8 scFileVarCharBlink[];
extern u8 scFileVarBlinkTimer[];
extern u8 scFileVar60[];
extern u8 scFileVarChoice[];
extern u8 scFileVar68[];
extern u8 scFileVarInMenu1[];
extern u8 scFileVarInMenu2[];
extern u8 scFileVar74[];
extern u8 scFileVar78[];
extern u8 scFileVar7C[];
extern u8 scFileVar80[];
extern u8 scFileVar84[];
extern u8 scFileVarUiSeMode[];
extern s32 *gScVarPlayer; /* the variables, in RAM */
extern s32 *gScVar04;
extern s32 *gScVarCancelOn;
extern s32 *gScVarFastText;
extern s32 *gScVarWaitTimer;
extern s32 *gScVarMenuId;
extern s32 *gScVarWinKind;
extern s32 *gScVarWinMethod;
extern s32 *gScVarWinX;
extern s32 *gScVarWinY;
extern s32 *gScVarTextWait;
extern s32 *gScVarFaceNo;
extern s32 *gScVarFaceKind;
extern s32 *gScVar34;
extern s32 *gScVar38;
extern s32 *gScVar3C;
extern s32 *gScVar40;
extern s32 *gScVarCharSize;
extern s32 *gScVarCharXGap;
extern s32 *gScVarCharYGap;
extern s32 *gScVarCharAlpha;
extern s32 *gScVar54;
extern s32 *gScVarCharBlink;
extern s32 *gScVarBlinkTimer;
extern s32 *gScVar60;
extern s32 *gScVarChoice;
extern s32 *gScVar68;
extern s32 *gScVarInMenu1;
extern s32 *gScVarInMenu2;
extern s32 *gScVar74;
extern s32 *gScVar78;
extern s32 *gScVar7C;
extern s32 *gScVar80;
extern s32 *gScVar84;
extern s32 *gScVarUiSeMode;

#ifndef AVOID_UB
Obj *strCreate(void); /* FAKE: in no header: used here as this file declares it (The number of arguments doesn't agree with the number in the declaration.); string.c defines `Obj *strCreate(Obj *parent)`, and string_c.h declares it so under AVOID_UB */
#endif

#define SC(obj) ((ScWork *) ((u32) (obj) + sizeof(Obj)))
#define SCP(obj) ((ScWork *) ((obj) + 1))

/* Finds the 35 script variables in the common scenario file. (0x08000040 twice: a slip in
 * the game.)
 * The first 0x08000040 goes through a local, which keeps the game's 0x20 frame. */
void scVarsLocate(void) {
    u8 *var40;

    gScVarPlayer = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVarPlayer);
    gScVar04 = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVar04);
    gScVarCancelOn = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVarCancelOn);
    gScVarFastText = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVarFastText);
    gScVarWaitTimer = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVarWaitTimer);
    gScVarMenuId = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVarMenuId);
    gScVarWinKind = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVarWinKind);
    gScVarWinMethod = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVarWinMethod);
    gScVarWinX = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVarWinX);
    gScVarWinY = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVarWinY);
    gScVarTextWait = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVarTextWait);
    gScVarFaceNo = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVarFaceNo);
    gScVarFaceKind = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVarFaceKind);
    gScVar34 = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVar34);
    gScVar38 = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVar38);
    var40 = scFileVar40;
    gScVar3C = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVar3C);
    gScVar40 = fileRamAddrAt(gScenarioFiles[0], (u32) var40);
    gScVarCharSize = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVarCharSize);
    gScVarCharXGap = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVarCharXGap);
    gScVarCharYGap = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVarCharYGap);
    gScVarCharAlpha = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVarCharAlpha);
    gScVar54 = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVar54);
    gScVar40 = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVar40);
    gScVarCharBlink = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVarCharBlink);
    gScVarBlinkTimer = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVarBlinkTimer);
    gScVar60 = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVar60);
    gScVarChoice = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVarChoice);
    gScVar68 = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVar68);
    gScVarInMenu1 = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVarInMenu1);
    gScVarInMenu2 = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVarInMenu2);
    gScVar74 = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVar74);
    gScVar78 = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVar78);
    gScVar7C = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVar7C);
    gScVar80 = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVar80);
    gScVar84 = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVar84);
    gScVarUiSeMode = fileRamAddrAt(gScenarioFiles[0], (u32) scFileVarUiSeMode);
}

/* The variables' first values. (0x40 is set twice in both.) */
void scVarsReset(void) {
    *gScVarPlayer = 0;
    *gScVar04 = 0;
    *gScVarCancelOn = 0;
    *gScVarFastText = 0;
    *gScVarWaitTimer = 0;
    *gScVarMenuId = 0;
    *gScVarWinKind = 0;
    *gScVarWinMethod = 0;
    *gScVarWinX = 0x20;
    *gScVarWinY = 0x16;
    *gScVarTextWait = 1;
    *gScVarFaceNo = 0;
    *gScVarFaceKind = 0;
    *gScVar34 = 0x46;
    *gScVar38 = 0x6E;
    *gScVar3C = 0x4B;
    *gScVar40 = 0;
    *gScVarCharSize = 0xC;
    *gScVarCharXGap = 1;
    *gScVarCharYGap = 2;
    *gScVarCharAlpha = 0xFF;
    *gScVar54 = 0;
    *gScVar40 = 0;
    *gScVarCharBlink = 0;
    *gScVarBlinkTimer = 0;
    *gScVar60 = 0;
    *gScVarChoice = -1;
    *gScVar68 = -1;
    *gScVarInMenu1 = 0;
    *gScVarInMenu2 = 0;
    *gScVar74 = 0;
    *gScVar78 = 0;
    *gScVar7C = 0;
    *gScVar80 = 0;
    *gScVar84 = 0;
    *gScVarUiSeMode = 1;
}

/* The two parents. */

/* "Sc親": the scenario tasks' parent, with the window, string, face and menu systems. */
s32 scParentProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    ScParent *w = (ScParent *) SC(obj);

    switch (msg) {
        case 2:
            if (gScParent != NULL) {
                return -1;
            }
            gScParent = obj;
            obj->name = "Sc親";
            PRINTF(">>>>>>>    Sc親生まれる\n");
            if (frmAFontKindCreate() == -1) {
                return -1;
            }
            if (frmCursorKindCreate() == -1) {
                return -1;
            }
            if (frmPromptKindCreate() == -1) {
                return -1;
            }
            if (frmFaceKindCreate() == -1) {
                return -1;
            }
            scLoadFiles();
            frmMsgFilesLoad();
            winLoadAllFiles();
            break;
        case 1:
            PRINTF(">>>>>>>     Sc親死す\n");
            gScParent = NULL;
            while (gfxGetPendingSwaps() != 0) {
            }
            frmAFontKindDelete();
            frmCursorKindDelete();
            frmPromptKindDelete();
            frmFaceKindDelete();
            scFreeFiles();
            frmMsgFilesFree();
            winFreeAllFiles();
            break;
        case 0:
            if (w->cache != NULL) {
                cacheClearLockBit(w->cache, gGameFbIndex + 1);
                cacheSetLockBit(w->cache, gGameFbIndex + 1);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The same without the window and face resources (their load and free are left out). */
s32 scParentNoFilesProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    ScParent *w = (ScParent *) SC(obj);

    switch (msg) {
        case 2:
            if (gScParent != NULL) {
                return -1;
            }
            gScParent = obj;
            obj->name = "Sc親";
            PRINTF(">>>>>>>    Sc親生まれる\n");
            if (frmAFontKindCreate() == -1) {
                return -1;
            }
            if (frmCursorKindCreate() == -1) {
                return -1;
            }
            if (frmPromptKindCreate() == -1) {
                return -1;
            }
            if (frmFaceKindCreate() == -1) {
                return -1;
            }
            break;
        case 1:
            PRINTF(">>>>>>>     Sc親死す\n");
            gScParent = NULL;
            frmAFontKindDelete();
            frmCursorKindDelete();
            frmPromptKindDelete();
            frmFaceKindDelete();
            break;
        case 0:
            if (w->cache != NULL) {
                cacheClearLockBit(w->cache, gGameFbIndex + 1);
                cacheSetLockBit(w->cache, gGameFbIndex + 1);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

struct Cache *scParentGetCache(Obj *obj) {
    return *(struct Cache **) SC(obj);
}

s32 scWorkGetNo(Obj *obj) {
    return SC(obj)->no;
}

s32 scRun_proc(Obj *obj, s16 msg, void *arg);

/* The two runners.
 * The first matched 2026-10-03 (from 10 lines). With case 1's two plain zero stores (the
 * original's store block) every temporary from case 0 on was one behind (t5/t6/t7 for
 * t6/t7/t8): the original spends one ugen temporary, with no instruction for it, between case
 * 2's t4 and case 0. It is the menu's test: "w->menu = f(obj); if (*(s32 *) &w->menu == 0)"
 * (FAKE). uopt does not forward a pointer store into an s32 read of the same word, so ugen
 * reloads it ("sw $2,92($17); lw $13,92($17); bne $13,0"); as1 forwards that reload into the
 * bnez and deletes it, and t5 stays spent. Found with IDO's "cc -S" (ugen before as1) on a
 * scratch copy, where every tried form shows its temporaries directly. Read back through obj
 * ("SC(obj)->menu") uopt does not forward either, but the base is s0, not w, so as1 keeps the
 * lw; a volatile read is kept by as1 (".set volatile"). Every tried form that spent t5 in case
 * 1 instead (a chain of the two zero stores, a test of D after its store) put the stores out of
 * order or left one or two instructions; "if (D == NULL) break;" falling into case 0 gave the
 * game's case 1, but its edge into case 0 costs the constant 1 its saved register (li at,1 for
 * the game's s1 at both tests), so it cannot be the original's form.
 * Unfaked 2026-10-03 (fakes sweep): the "pri frame" print sits between the menu's store and a
 * plain "if (w->menu == NULL)". The print's dead if (0) block ends uopt's block, so uopt reloads
 * w->menu after it (the same "lw $13,92($17)" as the s32 read), as1 forwards that reload and t5
 * stays spent; check: MATCH except own section addresses. The string order in .rodata is
 * unchanged (the pri frame string is still first). The assignment inside the test, "if
 * ((w->menu = f(obj)) == NULL)", spends nothing (16 lines, case 0 one t-register low).
 * The second runner spends the same t5 (its case 0 is t6/t7/t8 in the game, t5/t6/t7 here) and
 * has the same store block, but no menu test: its temporary is spent elsewhere after its t4 (a
 * word stored and read back through another type is the lead).
 * The second runner (331 lines): w at the declaration puts every "addiu w,obj,0x20" where the
 * original has it (per case it folded case 2 into obj offsets: 335), but IDO keeps w in MEMORY
 * (stored and reloaded inside one block: "sw t0,0x28(sp); lbu v0,0x70(t0); lw t2,0x28(sp)"),
 * obj takes s2 and the frame is 0x40. The original has w in s2, obj in s3, a 0x28 frame (no
 * local slots at all) and case 1's counter in s0 with its walker in s1 (ours swapped). Measured,
 * no change to w's spill: a block-scoped w for case 0 (both spilled, 332), no "a" local,
 * "register" on the locals, "default: break;" in both switches, the timer-4 use as SC(obj)->no
 * (a shorter live range, 328), DPRINTF (330); a second index for the phase-2 loops: 417.
 * The second runner, greedy pass 4 (2026-10-03, 305 to 225, mix 7 to 5): case 1's "sc_main"
 * print moved above its two zero stores (moves: a dead print crosses anything), then "r" declared
 * volatile (FAKE: the game stores r and reloads it). */
/* A runner ("scRun"): it loads its scenario, runs commands and messages until one waits, and
 * frees its tasks at the end. */
s32 scRun_proc(Obj *obj, s16 msg, void *arg) {
    ScWork *w = SC(obj);
    ScArg *a = arg;
    s32 i;
    s32 r;

    switch (msg) {
        case 2:
            obj->name = "scRun";
            if (gScRunner != NULL) {
                PRINTF("scRun_proc: Error: シナリオタスクはキドウしています\n");
                return -1;
            }
            gScRunner = obj;
            audioNop();
            w->no = a->no;
            w->player = a->player;
            w->plKind = a->plKind;
            w->state = 0;
            scLoadScenario(w->no);
            w->frames[0] = frmCreate(gFramePool10, gFrmKindCore);
            w->frames[1] = frmCreate(gFramePool10, gFrmKindCore);
            w->frames[2] = frmCreate(gFramePool10, gFrmKindCore);
            w->frames[3] = frmCreate(gFramePool10, gFrmKindCore);
            w->menu = mbCreate(obj);
            PRINTF("scRun: Warning: pri frame not enough\n");
            if (w->menu == NULL) {
                PRINTF("scRun: Warning: menuD memory not enough\n");
                return -1;
            }
            break;
        case 1:
            while (gfxGetPendingSwaps() != 0) {
            }
            for (i = 0; i < 4; i++) {
                if (w->frames[i] != NULL) {
                    frmDestroy(w->frames[i]);
                }
            }
            scFreeScenario(w->no);
            gGameWork.scenarioRunning = 0;
            gScRunner = NULL;
            PRINTF("sc_main タスクを削除します。\n");
            break;
        case 0:
            switch (w->phase) {
                case 0:
                    w->pc = fileRamAddrAt(gScenarioFiles[w->no], gScenarioStarts[w->no]);
                    scVarsLocate();
                    scVarsReset();
                    scVarsSetPlNo(obj);
                    scVarsSetPlKind(obj);
                    w->phase = 1;
                    PRINTF("scRun_proc: sc no[%d] pl no[%d] pl k[%d]\n", w->no, w->player, w->plKind);
                    break;
                case 1:
                    scCheckCancel(obj);
                    do {
                        scRunCommands(obj);
                        scUpdateWait(obj);
                        r = scRunMessage(obj);
                        if (r == 1) {
                            break;
                        }
                        if (r == -1) {
                            return -1;
                        }
                    } while (TRUE);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The second runner: the same, keeping the parent's systems, and at the end freeing its
 * faces, strings, windows and menu over 7 steps.
 * Measured 2026-10-02: the game's frame is 0x28, saved registers only, so it has no w or a
 * locals (with them this C spilled w, frame 0x40, 331 lines); written through SC(obj) and arg
 * (the macros below), the frame is the game's (305). Left: the game builds obj + 0x20 in each
 * case (s2 in cases 2 and 0, s1 in case 1) where IDO here folds it into offsets from obj. A w
 * assigned in each case (332) or declared in a block per case (332, frame 0x48) both spill.
 * The w local with greedy splits (IDO_NOTES 228): one boundary after "gScRunner = obj;",
 * 331 to 319, still above this form's 305; this form: no boundary helps. Three function-scope
 * pointers, one per case (IDO_NOTES 240), each set from SC(obj) at its case's head: they spill
 * too (frame 0x48, 335; 2026-10-03).
 * 2026-10-03, E7278's shape (w and a declared, r a plain s32): 330, mix 26 (w's
 * per-case obj + 0x20 is a CSE temp kept at 0x28(sp)). Why it spills: the function's SIZE.
 * Scratch compiles (measured on a scratch copy): w gets a register
 * once the phase-2 timer chain loses any three of its four wait loops (two with no prints), and
 * six FAKE "do {} while (0);" blocks ANYWHERE (before the switch, in case 1, after it) take it
 * away again, even with w assigned per case; obj is near the same line (seven blocks in case 1
 * plus nine in the timer bodies leave obj in memory too). So w's priority is far too low for a
 * function this long, and E7278 is shorter. Fixed with three FAKE address-only uses of w
 * ("if (!&gFrmLayers[(s32) w + (s32) w]) {}", mid-block: case 2 before frames[3], phase 0
 * after scVarsSetPlNo, timer 1 before the menu print): 40 lines, mix 0, the game's frame 0x28,
 * w in s2 and obj in s3 as the game. At a block's END (before a break) the same test leaves a
 * branch (+9 instructions); six of them spill w again. Left: case 1's counter and walker
 * swapped (game counter s0 set in the dispatch's delay slot, walker s1), and every t-register
 * one low from case 0 on (the first runner's spent temporary, see its note).
 * One FAKE with eight references (case 2, before frames[3]) does what the three do (40) and
 * leaves room: six more FAKE blocks no longer spill w. With it, the first runner's chain in
 * case 1, "unk12 = (s32) (D = NULL)": 24 lines, mix 0 (the game's t6.. from case 0 on; the
 * stores D first, the game sb first). Scratch (no check): an empty test of D after the two
 * plain stores spends t5 too but +2 instructions (31 rough); "if (w->frames[3] == NULL) {}"
 * after case 2's stores spends it, +4. Case 1's swap, not changed by (scratch, all 43 rough,
 * w kept): declaration order (six orders), u32 i, "i = 0" before/after the loop, "i != 4", a
 * continue, a Frame * local, a block-scoped w1 = SC(obj), w assigned per case; a block-scoped
 * j or r as the index (the counter and walker go to v1/a1 spilled around the call, as in the
 * first runner's game: 272); SC(obj)->frames[i] (counter s0 in the slot as the game, but the
 * walker folds to obj + 0x80 in v1, spilled: 235); a FAKE use of i in the loop (+code).
 * More scratch on this form (24; "rough" = my objdump diff, 29 for it): case 1's loop as a while,
 * a for with the step in the body, a do-while, a goto loop, "if (..) {} else", "++i < 4" from
 * -1, "*(w->frames + i)", "i[w->frames]", "i < 4U", "(u32) i", sizeof bounds, the call's
 * result into r, w used (FAKE, dead print) before or after the loop, FAKE blocks before, in or
 * after the loop: all 29, the swap stays. One index per loop (j for phase 2, or for case 1, or
 * r) costs saved registers (only s0/s1 used, 262-272). In ugen ("cc -S") every loop's counter
 * is $17 and walker $16 (one counter temp for the four loops); the game's case 1 has them the
 * other way round from its phase-2 loops, and its delay slot holds the counter's init (ugen
 * emitting the walker's init first). An always-taken "if (D == NULL) break;" falling into case
 * 0 (the first runner's trial) puts the counter's init in the slot as the game, but still s1,
 * and the constant 1 loses s1 (44 rough, +3). The "warning" print under natural tests of the
 * frames (||, !, [0], [3], a loop, &&) with no FAKE: w still spills. sweeps on this form:
 * copies (16 tried) and moves (14 tried): nothing below 24.
 * 2026-10-03: case 1's swap fixed, 24 to 10, mix 0. Case 1 has its OWN index
 * (j) and every print in this function but the "pri frame" warning is the empty-macro DPRINTF
 * (the first runner takes the same mix and still matches: its prints 1, 3, 4, 5 as DPRINTF give
 * its matched code, only the "pri frame" one must stay a PRINTF block, scratch s15). The same
 * form with all nine as DPRINTF is also 10. Why (cc -S scratch, r2/a0/scr):
 * uopt pays for a saved register only from a live range whose benefit covers the save; others
 * then share the paid registers for free. With one i for all four loops, the four loops' SR
 * counter and walker are one range each (the same expressions i*4, w + i*4), they pay for s0/s1
 * and case 1 must take the phase-2 order (walker s0, counter s1). With a j for case 1 and the
 * if (0) prints, nothing pays except obj and w (frame 0x40-0x48, everything else in t-registers
 * spilled around the calls: 262), which is why a separate index looked wrong before. The prints'
 * dead "if (0)" blocks lower the phase-2 ranges' priority (size): with all nine as DPRINTF (no
 * block) phase 2's i pays again and case 1's own j-ranges colour as the game (counter s0 set in
 * the dispatch's slot, walker s1 = obj + 0x20). Proof without DPRINTF: a fourth phase-2 loop on i
 * also gives case 1 the game's registers. It is a count: any 7 of the 9 prints as DPRINTF keep
 * it (each single one and the pair 4+5 back as PRINTF: same code), 6 (4-9) or fewer collapse it
 * (262), so where the empty blocks sit does not matter, only how many. Caution: the first
 * runner (matched with PRINTF) with all its prints as DPRINTF does NOT match (obj s1, the constant
 * 1 in at), so the original file is not DPRINTF throughout; the original may have had fewer blocks
 * here some other way. FAKE w still needed (without it w spills: 181). One shared i with all
 * prints as DPRINTF: case 1 still swapped (rough 28 for the committed form's 29), so the j is the
 * fix and DPRINTF only lets it pay.
 * Left: the store order at case 1's end (the chain stores D first; the game sb first, D in the
 * b's slot) and the .rodata address of "scRun" (own section). On this form (scratch, rough
 * lines, the chain = 15): plain "unk12 = 0; D = NULL;" 29 (no t5 spent: case 0 one t-register
 * low, the store block itself the game's); "if (D == NULL) break;" after them 17; "if ((D =
 * NULL) == NULL) break;" 17; the s32 read-back test with break 17; an empty test of D 16; "if
 * (D = NULL) {}" 16; the reversed chain "D = (Obj *) (unk12 = 0)" 22; "(s16)" in the chain 16,
 * "(u8) (u32)" 15, "!= NULL" 34; "unk12 = j = 0" 33, "= r = 0" 29; D read back from unk12 22-23;
 * the stores inside DPRINTF's arguments 28-29; a FAKE block after the chain 18 (plain 32); j as
 * u32 same, as s16/u16/u8/s8 46-51. With the plain stores, the "pri frame" warning under "if
 * (w->frames[3] == NULL)" (PRINTF or DPRINTF body, or an s32 read-back test): 29, the empty test
 * is dropped before ugen and spends nothing; under a test of all four frames, or an empty s32
 * read-back test after it: the registers collapse (272-275). Checks: 24 (as committed), 10 (all
 * nine DPRINTF + j), 10 (this form). The store order (ugen hand-edited and
 * assembled with as0 + as1): no form below 24 then; its one untried lead, the
 * walker start built in a temp and moved ("addu t5,s3,32; move s1,t5", which as1 folds into
 * "addiu s1,s3,0x20"), may spend the hidden t5.
 * 2026-10-03 (scratch compiles: tmpl.c,
 * run.sh, asmx.sh). (1) A body #included INSIDE a function puts every statement on the
 * #include's line (.loc), and as1 schedules by line: r2's scratch "rough" counts were measured
 * that way (the chain: 15 there, 9 with real lines). tmpl.c is the whole function #included at
 * file level, hooks one statement per line; its chain form compiles byte for byte as this copy.
 * (2) Proven with as0 + as1 on hand-edited ugen: the chain form's ugen with its case 1 tail
 * replaced by the plain stores' ("la $2,D; sb $0,unk12; sw $0,0($2)", their own .loc lines) is
 * the game's code exactly; so is the same with the walker start through $13 ("addu $13,$19,32;
 * move $17,$13" folds). So the plain stores plus one spent ugen temporary anywhere between case
 * 2's t4 and case 0's first temporary is the whole answer. (3) Real-line scratch, plain stores
 * (rough = r3/a0's objdump diff with the game's 3 pad nops; the target is 3): plain 23; walker
 * forms (w = SC(obj) in case 1, a block-scoped w1, f = w->frames) 23-27; the frame tested
 * through an s32 view in the loop (t5 spent but two loads, counter/walker swapped) 21; the
 * reversed chain or D from unk12 (an lb reload, s8 never forwarded) 19; narrow casts in the chain
 * (s16, u16, u8, s8, u8 u32) 9-10, D still first; j as s16/u8 40-41, u32 22. An s32 read-back of
 * D after the plain stores spends t5 exactly (case 0 on becomes the game's) but its branch stays
 * a block for as1: empty test "{}" 8-9 (one more "b" block), "if (..) break;" before the break
 * 8-9 (as1 resolves the branch, the leftover "b; move a0,s3" stays), and with the "pri frame"
 * print kept as a PRINTF any such block collapses the colouring (257-261); "if (*(s32 *) &D ==
 * 0) break;" with NO break after (falling into case 0): case 1 byte for byte the game's, t5
 * spent, but the edge into case 0 costs the constant 1 its s1 (li at,1 at four tests): 22, as in
 * the first runner. The menu read back as an s32 after its store (the first runner's FAKE) with
 * an empty body: everything the game's but "bnez v0,end" and a "b end; move a0,s3" (+3): 7.
 * D stored back from its own s32 read-back ("D = (Obj *) *(s32 *) &D;" after the plain stores):
 * t5 spent, everything the game's but one extra "sw zero,0(v0)" (as1 forwards the reload, does
 * not drop the doubled store): 4. Dead locals given the read-back (r, i, a) are removed by uopt
 * (23). "pri frame" under "if (*(s32 *) &w->frames[3] == 0) PRINTF/DPRINTF" is dropped (23);
 * with "{}" it stays (20). Case 2's four frames as a loop (IDO does not unroll it fully): 88-221.
 * Chains into j's start (j = i = 0, j = r = 0, r = j = 0, j = r after r = 0): 23.
 * MATCHED (check: "MATCH except own section addresses", the 4 lines of "scRun"'s .rodata
 * address): fileres.c resCountAddr's never-true unsigned test, "if ((u32) obj + 1 < 0) break;",
 * at case 1's head, with the plain stores and all nine prints as DPRINTF. ugen computes obj + 1
 * into t5 ("addu $13,$19,1") for a branch that is never taken; as1 deletes the branch and then
 * the dead addu, and t5 stays spent with no instruction; a deleted branch BEFORE the stores
 * leaves the stores and the case's "b" one block, so the D store lands in the b's slot as the
 * game. Scratch (rough 3 = the game's code; only its 3 pad nops differ): the same test right
 * after the loop (before the stores) or at case 0's head: 3; between the two stores 7 (the
 * boundary splits the store block); "(u32) w + 1" 6 (w kept live into case 1); the test with a
 * body ("{ D = NULL; }") after the stores or at case 0's head: ugen branches around the body
 * with an always-taken "bgeu", and as1 leaves a block (8, 8 +3 instructions); with the "pri
 * frame" print kept as a PRINTF (one dead block more) the colouring collapses (256). This form
 * matched too, with a FAKE address test in case 2's wait loop (uopt hoists the loop-invariant
 * address, ugen's "mul $13; addu" are dead and as1 deletes them); not kept.
 * Fakes sweep (2026-10-03, ugen scratch copies): the first
 * runner's natural form (the pri frame print after the menu's store) spends nothing here, as no
 * test follows the store. The pri frame print under tests of the frames (three or four terms,
 * either order, with or without the eight-use FAKE) spends t5 but collapses the colouring (frame
 * 0x48, w in s0). "if (w->frames[3] == NULL)" after the menu call reloads the frame and spends
 * exactly t5 with the colouring kept, but at the case end ugen leaves "bne; b" to the end. A
 * single test before the menu call is dropped (nothing spent). Dead print blocks (PRINTF) at the
 * head, middle or end of case 1, between its two stores, or for the pri frame print: nothing
 * spent. Without the eight-use FAKE, every form tried (pri frame print moved, as a PRINTF, under
 * a test of frames[3]) spills w (frame 0x48).
 * Unparked (2026-10-03): the parked .s carried the string block 0x8010D928 for this
 * function AND the later ones (E78E8/E7968's errors, the interpreters' prints, the two
 * "scStringTaskCheck" prints); those now come from their own C (see SC_STR below). */
/* The game's debug print as an empty macro: it compiles to its arguments (IDO_NOTES 144). */
#define DPRINTF
s32 scRun_proc2(Obj *obj, s16 msg, void *arg) {
    ScWork *w = SC(obj);
    ScArg *a = arg;
    s32 i;
    s32 j;
    s32 r;

    switch (msg) {
        case 2:
            obj->name = "scRun";
            if (gScRunner != NULL) {
                DPRINTF("scRun_proc: Error: シナリオタスクはキドウしています\n");
                return -1;
            }
            gScRunner = obj;
            w->no = a->no;
            w->player = a->player;
            w->plKind = a->plKind;
            w->state = 0;
            scLoadScenario(w->no);
            while (gfxGetPendingSwaps() != 0) {
            }
            scLoadFiles();
            frmMsgFilesLoad();
            winLoadAllFiles();
            w->frames[0] = frmCreate(gFrmLayers[10], gFrmKindCore);
            w->frames[1] = frmCreate(gFrmLayers[10], gFrmKindCore);
            w->frames[2] = frmCreate(gFrmLayers[10], gFrmKindCore);
            /* FAKE: an address-only use of w (eight references, no code) raises its priority. */
            if (!&gFrmLayers[(s32) w + (s32) w + (s32) w + (s32) w + (s32) w + (s32) w + (s32) w + (s32) w]) {}
            w->frames[3] = frmCreate(gFrmLayers[10], gFrmKindCore);
            DPRINTF("scRun: Warning: pri frame not enough\n");
            w->menu = mbCreate(obj);
            break;
        case 1:
            /* FAKE: never true (unsigned). ugen computes obj + 1 into the original's spent
             * temporary (t5) and as1 deletes the never-taken branch and the dead addu (see the
             * note); the original's dead code here is unknown. */
            if ((u32) obj + 1 < 0) {
                break;
            }
            for (j = 0; j < 4; j++) {
                if (w->frames[j] != NULL) {
                    frmDestroy(w->frames[j]);
                }
            }
            DPRINTF("sc_main タスクを削除します。\n");
            gGameWork.scenarioRunning = 0;
            gScRunner = NULL;
            break;
        case 0:
            switch (w->phase) {
                case 0:
                    w->pc = fileRamAddrAt(gScenarioFiles[w->no], gScenarioStarts[w->no]);
                    scVarsLocate();
                    scVarsReset();
                    scVarsSetPlNo(obj);
                    scVarsSetPlKind(obj);
                    w->phase = 1;
                    DPRINTF("scRun_proc: sc no[%d] pl no[%d] pl k[%d]\n", w->no, w->player, w->plKind);
                    break;
                case 1:
                    scCheckCancel(obj);
                    do {
                        scRunCommands(obj);
                        scUpdateWait(obj);
                        r = scRunMessage(obj);
                        if (r == 1) {
                            break;
                        }
                        if (r == -1) {
                            DPRINTF("**** sc 終了処理へ\n");
                            w->timer = 0;
                            w->phase = 2;
                            break;
                        }
                    } while (TRUE);
                    break;
                case 2:
                    if (w->timer++ >= 7) {
                        return -1;
                    }
                    if (w->timer == 1) {
                        DPRINTF("******  face_tsk delete\n");
                        for (i = 0; i < 10; i++) {
                            if (w->face[i] != NULL) {
                                w->face[i] = faceClose(w->face[i]);
                            }
                        }
                        DPRINTF("******  str_tsk delete\n");
                        for (i = 0; i < 3; i++) {
                            if (w->str[i] != NULL) {
                                w->str[i] = tskDestroy(w->str[i]);
                            }
                        }
                        DPRINTF("******  win_tsk delete\n");
                        for (i = 0; i < 3; i++) {
                            if (w->win[i] != NULL) {
                                w->win[i] = tskDestroy(w->win[i]);
                            }
                        }
                        DPRINTF("******  menuD delete\n");
                        if (w->menu != NULL) {
                            w->menu = tskDestroy(w->menu);
                        }
                    } else if (w->timer == 2) {
                        while (gfxGetPendingSwaps() != 0) {
                        }
                        frmMsgFilesFree();
                    } else if (w->timer == 3) {
                        while (gfxGetPendingSwaps() != 0) {
                        }
                        winFreeAllFiles();
                    } else if (w->timer == 4) {
                        while (gfxGetPendingSwaps() != 0) {
                        }
                        scFreeScenario(w->no);
                    } else if (w->timer == 5) {
                        while (gfxGetPendingSwaps() != 0) {
                        }
                        scFreeFiles();
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Runs scenario n for a player with the first runner (not while one runs). Pausing is off
 * meanwhile. */
Obj *scRunStart(s32 n, s32 player, s32 plKind) {
    Obj *obj;
    s32 pad;
    ScArg a;

    if (gScRunner != NULL) {
        PRINTF("scRun_proc: Error: シナリオタスクは起動しています\n");
        return NULL;
    }
    a.no = n;
    a.player = player;
    a.plKind = plKind;
    obj = tskCreateBlocks(gScParent, scRun_proc, 2, 0, &a);
    gGameWork.scenarioRunning = 2;
    return obj;
}

/* The same with the second runner. */
Obj *scRunStart2(s32 n, s32 player, s32 plKind) {
    Obj *obj;
    s32 pad;
    ScArg a;

    if (gScRunner != NULL) {
        PRINTF("scRun_proc: Error: シナリオタスクは起動しています\n");
        return NULL;
    }
    a.no = n;
    a.player = player;
    a.plKind = plKind;
    obj = tskCreateBlocks(gScParent, scRun_proc2, 2, 0, &a);
    gGameWork.scenarioRunning = 2;
    return obj;
}

/* Ends the running scenario. */
Obj *scRunEnd(void) {
    return tskDestroy(gScRunner);
}

/* Whether a scenario runs. */
s32 scIsRunning(void) {
    if (gScRunner != NULL) {
        return 1;
    }
    return 0;
}

void scSetNo(Obj *obj, s32 n) {
    SC(obj)->no = n;
}

s32 scGetNo(Obj *obj) {
    return SC(obj)->no;
}

/* Old-style definitions: the char comes in as an int, is stored to its home and is used
 * untruncated (sw a1,4(sp)). */
void scSetPlNo(obj, player)
Obj *obj;
u8 player;
{
    SC(obj)->player = player;
}

void scSetPlKind(obj, plKind)
Obj *obj;
u8 plKind;
{
    SC(obj)->plKind = plKind;
}

/* The player and the second value into the variables 0x08000000 and 0x08000004. */
void scVarsSetPlNo(Obj *obj) {
    *gScVarPlayer = SC(obj)->player;
}

void scVarsSetPlKind(Obj *obj) {
    *gScVar04 = SC(obj)->plKind;
}

void scLoadFiles(void) {
    fileLoadListNow(gScFileList);
}

void scFreeFiles(void) {
    resFreeList(gScFileList);
}

/* Loads and frees scenario n's file. */
void scLoadScenario(s32 n) {
    fileLoadNow(gScenarioFiles[n], NULL);
}

void scFreeScenario(s32 n) {
    resFree(gScenarioFiles[n]);
}

Frame *scGetFrame(Obj *obj, s32 i) {
    return SC(obj)->frames[i];
}

/* A print's string: the literal itself. The dead calls stay: where a dead print sits decides
 * registers. */
#define SC_STR(s) s

/* The command and message interpreters. */

/* Runs commands until one shows a message, waits or ends. There is no pointer local for the
 * command: every access reads through w->pc, as scRunMessage reads w->text (the game's s1 is
 * IDO's PRE register for w->pc, reloaded after each store through *v). */
void scRunCommands(Obj *obj) {
    ScWork *w = SC(obj);
    s32 stop = FALSE;
    s32 *v;
    s32 val;
    s32 op;

    if (w->state != 0) {
        return;
    }
    do {
        switch (w->pc->op) {
            case 16:
                w->state = 1;
                w->text = fileRamAddrAt(gScenarioFiles[w->no], w->pc->arg);
                stop = TRUE;
                break;
            case 4:
                w->state = 0;
                v = fileRamAddrAt(gScenarioFiles[0], w->pc->arg);
                w->pc++;
                switch (w->pc->op) {
                    case 9:
                        *v = w->pc->arg;
                        break;
                    case 19:
                        *v += w->pc->arg;
                        break;
                    case 20:
                        *v -= w->pc->arg;
                        break;
                    case 21:
                        *v = *(s32 *) w->pc->arg;
                        break;
                    case 22:
                    case 23:
                        *v = w->pc->arg;
                        break;
                    case 24:
                        *v |= w->pc->arg;
                        break;
                    case 25:
                        *v &= w->pc->arg;
                        break;
                }
                w->pc++;
                break;
            case 6:
                w->state = 0;
                v = fileRamAddrAt(gScenarioFiles[0], w->pc->arg);
                w->pc++;
                op = w->pc->op;
                val = w->pc->arg;
                switch (op) {
                    case 2:
                        if (val != *v) {
                            w->pc++;
                            w->pc++;
                        } else {
                            w->pc++;
                        }
                        break;
                    case 3:
                        if (val == *v) {
                            w->pc++;
                            w->pc++;
                        } else {
                            w->pc++;
                        }
                        break;
                    case 12:
                        if (*v >= val) {
                            w->pc++;
                            w->pc++;
                        } else {
                            w->pc++;
                        }
                        break;
                    case 13:
                        if (val >= *v) {
                            w->pc++;
                            w->pc++;
                        } else {
                            w->pc++;
                        }
                        break;
                    case 14:
                        if (val < *v) {
                            w->pc++;
                            w->pc++;
                        } else {
                            w->pc++;
                        }
                        break;
                    case 15:
                        if (*v < val) {
                            w->pc++;
                            w->pc++;
                        } else {
                            w->pc++;
                        }
                        break;
                }
                break;
            case 10:
                w->state = 0;
                w->pc = fileRamAddrAt(gScenarioFiles[w->no], w->pc->arg);
                break;
            case 11:
                w->state = 0;
                val = w->pc->arg;
                ((void (*)(void)) val)();
                w->pc++;
                break;
            case 17:
                PRINTF(SC_STR("SC_MENU_S: mbMenuAdd(L->menuD)\n"));
                mbMenuAdd(w->menu);
                w->pc = mbScPtrGet(w->menu, *gScVarMenuId);
                break;
            case 18:
                PRINTF(SC_STR("scRun_proc: SC_MENU_E\n"));
                w->pc++;
                break;
            case 32:
                w->state = 0;
                swWorkSet(w->pc->arg);
                w->pc++;
                break;
            case 33:
                w->state = 0;
                swWorkReset(w->pc->arg);
                w->pc++;
                break;
            case 34:
                w->state = 0;
                if (swWorkTest(w->pc->arg) != 0) {
                    w->pc++;
                    w->pc = fileRamAddrAt(gScenarioFiles[w->no], w->pc->arg);
                } else {
                    w->pc++;
                    w->pc++;
                }
                break;
            case 35:
                w->state = 0;
                if (swWorkTest(w->pc->arg) == 0) {
                    w->pc++;
                    w->pc = fileRamAddrAt(gScenarioFiles[w->no], w->pc->arg);
                } else {
                    w->pc++;
                    w->pc++;
                }
                break;
            case 8:
                val = mbMenuExistCheck(w->menu);
                PRINTF(SC_STR("SC_MBRACE_E: mbMenuExistCheck(L->menuD)\n"));
                stop = TRUE;
                if (val == 1) {
                    w->state = 4;
                } else {
                    w->state = 5;
                }
                break;
        }
    } while (!stop);
}

/* Shows the message: characters go to the window's string task, controls open and close
 * windows and faces and wait for A. Returns 1 to wait for the next frame, 0 to go on, -1 at
 * the end. The C follows the game's block order: the window cases test "!= NULL" first and
 * share a "return 1" after an else; the face cases end each path in its own continue; the A
 * button's tests compare the loaded characters (the game puts the constants' registers first),
 * the next command through a pointer; w->flags, not SC(obj)->flags; case 4 breaks to a "return
 * 0" after the switch and jumps to the default's "return 1" past case 5. */
s32 scRunMessage(Obj *obj) {
    ScWork *w = SC(obj);
    s16 *s;
    ScCmd *next;

    switch (w->state) {
        case 1:
            while (*w->text != -1) {
                switch (*w->text) {
                    case -20:
                        if (w->win[*gScVarMenuId] != NULL) {
                            if (winGetState(w->win[*gScVarMenuId]) == 3) {
                                w->str[*gScVarMenuId] = scStringTaskCheck(obj, *gScVarMenuId);
                                w->text++;
                                continue;
                            }
                        } else {
                            w->win[*gScVarMenuId] = winCreate(obj, *gScVarWinKind, *gScVarWinMethod);
                            w->flags |= 1;
                            PRINTF(SC_STR("scRun_proc: WINDOW OPEN します。[%d]番[%d]方法\n"), *gScVarMenuId,
                                   *gScVarWinMethod);
                        }
                        return 1;
                    case -22:
                        if (w->win[*gScVarMenuId] != NULL) {
                            if (winGetState(w->win[*gScVarMenuId]) == 3) {
                                PRINTF(SC_STR("scRun_proc: [%d]番WINDOW CLOSE します。\n"), *gScVarMenuId);
                                winClose(w->win[*gScVarMenuId], *gScVarWinMethod);
                                return 1;
                            }
                            if (winGetState(w->win[*gScVarMenuId]) == 0) {
                                PRINTF(SC_STR("scRun_proc: [%d]番WINDOW CLOSE しました。\n"), *gScVarMenuId);
                                w->win[*gScVarMenuId] = tskDestroy(w->win[*gScVarMenuId]);
                                w->str[*gScVarMenuId] = strDestroy(w->str[*gScVarMenuId]);
                                mbAMenuDelete(w->menu, *gScVarMenuId);
                                w->text++;
                                continue;
                            }
                        } else {
                            PRINTF(SC_STR("scRun_proc: Warning: [%d]番WINDOW 既にCLOSE しています。\n"),
                                   *gScVarMenuId);
                            w->text++;
                        }
                        return 1;
                    case -23:
                        winPromptOn(w->win[*gScVarMenuId]);
                    case -21:
                        if (scPadTrg(w->player, A_BUTTON) == 1) {
                            PRINTF(SC_STR("scRun_proc: SCS_WAIT: TRG_A を実行しました。\n"));
                            winPromptOff(w->win[*gScVarMenuId]);
                            w->text++;
                        }
                        return 1;
                    case -24:
                        scStringTaskCheck(obj, *gScVarMenuId);
                        _strClear(w->str[*gScVarMenuId]);
                        w->text++;
                        continue;
                    case -19:
                    case -18:
                    case -17:
                    case -16:
                    case -15:
                    case -14:
                    case -13:
                    case -12:
                    case -11:
                    case -10:
                    case -9:
                    case -8:
                    case -7:
                    case -5:
                    case -2:
                        scStringTaskCheck(obj, *gScVarMenuId);
                        _strAdd(w->str[*gScVarMenuId], *w->text);
                        w->text++;
                        continue;
                    case -25:
                        if (w->face[*gScVarFaceNo] != NULL) {
                            PRINTF(SC_STR("scRun_proc: Err: 顔ウィンドウは既に開いています no[%d]\n"), *gScVarFaceNo);
                            w->text++;
                            continue;
                        }
                        PRINTF(SC_STR("scRun_proc: [%d]番 種類[%d]FACE OPEN します。\n"), *gScVarFaceNo,
                               *gScVarFaceKind);
                        w->face[*gScVarFaceNo] = faceOpen(obj, *gScVarFaceKind);
                        w->text++;
                        continue;
                    case -26:
                        if (w->face[*gScVarFaceNo] == NULL) {
                            PRINTF(SC_STR("scRun_proc: Err: FACE CLOSE: 顔ウィンドウ開いていない\n"));
                            continue;
                        }
                        PRINTF(SC_STR("scRun_proc: [%d]番 種類[%d] FACE CLOSE します。\n"), *gScVarFaceNo,
                               *gScVarFaceKind);
                        w->face[*gScVarFaceNo] = faceClose(w->face[*gScVarFaceNo]);
                        w->text++;
                        continue;
                    default:
                        scStringTaskCheck(obj, *gScVarMenuId);
                        if (*gScVarTextWait != 0) {
                            if (*gScVarFastText == 1) {
                                if (scPadHold(*gScVarPlayer, A_BUTTON) != 0) {
                                    PRINTF(SC_STR("******   A BUTTON\n"));
                                    _strAdd(w->str[*gScVarMenuId], *w->text);
                                    if (*++w->text == -23 || *w->text == -21) {
                                        return 1;
                                    }
                                    if (*w->text == -1 && (next = w->pc + 1)->op == 16) {
                                        s = (s16 *) fileRamAddrAt(gScenarioFiles[w->no],
                                                                       next->arg);
                                        if (*s == -23 || *s == -21) {
                                            return 1;
                                        }
                                    }
                                    return 0;
                                }
                                PRINTF(SC_STR("******   NORMAL1\n"));
                                if (strAddByTimer(w->str[*gScVarMenuId], *w->text) != 0) {
                                    w->text++;
                                }
                                return 1;
                            }
                            PRINTF(SC_STR("******   NORMAL2\n"));
                            if (strAddByTimer(w->str[*gScVarMenuId], *w->text) != 0) {
                                w->text++;
                            }
                            return 1;
                        }
                        _strAdd(w->str[*gScVarMenuId], *w->text);
                        w->text++;
                        return 0;
                }
            }
            w->state = 0;
            w->pc++;
            return 1;
        case 4:
            mbChoiceToVar(w->menu);
            if (mbMenuCheck(w->menu) == 0) {
                goto wait;
            }
            w->pc = mbScJumpPtrGet(w->menu);
            mbChoiceToVar(w->menu);
            mbMenuDelete(w->menu);
            w->state = 0;
            break;
        case 5:
            return -1;
        default:
        case 0:
        case 2:
        case 3:
        wait:
            return 1;
    }
    return 0;
}

/* Whether a player (1-4, or 0 for anyone in play) has just pressed `mask`. */
s32 scPadTrg(s32 player, u16 mask) {
    if (player == 1 && joyGetHeldTime(0, mask) == 1) {
        return 1;
    }
    if (player == 2 && joyGetHeldTime(1, mask) == 1) {
        return 1;
    }
    if (player == 3 && joyGetHeldTime(2, mask) == 1) {
        return 1;
    }
    if (player == 4 && joyGetHeldTime(3, mask) == 1) {
        return 1;
    }
    if (player == 0) {
        if (gPlayer1Chara != -1 && joyGetHeldTime(0, mask) == 1) {
            return 1;
        }
        if (gPlayer2Chara != -1 && joyGetHeldTime(1, mask) == 1) {
            return 1;
        }
        if (gPlayer3Chara != -1 && joyGetHeldTime(2, mask) == 1) {
            return 1;
        }
        if (gPlayer4Chara != -1 && joyGetHeldTime(3, mask) == 1) {
            return 1;
        }
    }
    return 0;
}

/* Whether a player (1-4, or 0 for anyone in play) holds `mask`. */
s32 scPadHold(s32 player, s32 mask) {
    if (player == 1 && (joyGetButton(0, 0) & mask)) {
        return 1;
    }
    if (player == 2 && (joyGetButton(1, 0) & mask)) {
        return 1;
    }
    if (player == 3 && (joyGetButton(2, 0) & mask)) {
        return 1;
    }
    if (player == 4 && (joyGetButton(3, 0) & mask)) {
        return 1;
    }
    if (player == 0) {
        if (gPlayer1Chara != -1 && (joyGetButton(0, 0) & mask)) {
            return 1;
        }
        if (gPlayer2Chara != -1 && (joyGetButton(1, 0) & mask)) {
            return 1;
        }
        if (gPlayer3Chara != -1 && (joyGetButton(2, 0) & mask)) {
            return 1;
        }
        if (gPlayer4Chara != -1 && (joyGetButton(3, 0) & mask)) {
            return 1;
        }
    }
    return 0;
}

/* The same as scPadTrg with auto-repeat: pressed, or held 21 frames or more and on
 * every fifth after. (The mask is an s32 here, cut to the callee's u16 at each call: IDO reads
 * it from its home as a halfword per test and re-masks it after the first call, as the game
 * does; the callers in menu.c pass constants.) */
s32 scPadRepeat(s32 player, s32 mask) {
    if (player == 1 && (joyGetHeldTime(0, mask) == 1 ||
                        (joyGetHeldTime(0, mask) >= 21 && joyGetHeldTime(0, mask) % 5 == 1))) {
        return 1;
    }
    if (player == 2 && (joyGetHeldTime(1, mask) == 1 ||
                        (joyGetHeldTime(1, mask) >= 21 && joyGetHeldTime(1, mask) % 5 == 1))) {
        return 1;
    }
    if (player == 3 && (joyGetHeldTime(2, mask) == 1 ||
                        (joyGetHeldTime(2, mask) >= 21 && joyGetHeldTime(2, mask) % 5 == 1))) {
        return 1;
    }
    if (player == 4 && (joyGetHeldTime(3, mask) == 1 ||
                        (joyGetHeldTime(3, mask) >= 21 && joyGetHeldTime(3, mask) % 5 == 1))) {
        return 1;
    }
    if (player == 0) {
        if (gPlayer1Chara != -1 &&
            (joyGetHeldTime(0, mask) == 1 ||
             (joyGetHeldTime(0, mask) >= 21 && joyGetHeldTime(0, mask) % 5 == 1))) {
            return 1;
        }
        if (gPlayer2Chara != -1 &&
            (joyGetHeldTime(1, mask) == 1 ||
             (joyGetHeldTime(1, mask) >= 21 && joyGetHeldTime(1, mask) % 5 == 1))) {
            return 1;
        }
        if (gPlayer3Chara != -1 &&
            (joyGetHeldTime(2, mask) == 1 ||
             (joyGetHeldTime(2, mask) >= 21 && joyGetHeldTime(2, mask) % 5 == 1))) {
            return 1;
        }
        if (gPlayer4Chara != -1 &&
            (joyGetHeldTime(3, mask) == 1 ||
             (joyGetHeldTime(3, mask) >= 21 && joyGetHeldTime(3, mask) % 5 == 1))) {
            return 1;
        }
    }
    return 0;
}

/* Holds (on) or frees a player's controller (1-4, or 0 for everyone in play). */
void scPadStickDpad(s32 player, s32 on) {
    if (player == 1) {
        joySetStickDpad(0, on);
    }
    if (player == 2) {
        joySetStickDpad(1, on);
    }
    if (player == 3) {
        joySetStickDpad(2, on);
    }
    if (player == 4) {
        joySetStickDpad(3, on);
    }
    if (player == 0) {
        if (gPlayer1Chara != -1) {
            joySetStickDpad(0, on);
        }
        if (gPlayer2Chara != -1) {
            joySetStickDpad(1, on);
        }
        if (gPlayer3Chara != -1) {
            joySetStickDpad(2, on);
        }
        if (gPlayer4Chara != -1) {
            joySetStickDpad(3, on);
        }
    }
}

/* B ends the scenario when the variable 0x08000008 allows it. */
void scCheckCancel(Obj *obj) {
    if (*gScVarCancelOn == 1 && scPadTrg(*gScVarPlayer, B_BUTTON) != 0) {
        SC(obj)->state = 5;
    }
}

/* The wait timer (variable 0x08000010): counting down it waits; at 0 a wait ends; -1 starts
 * a wait for the next message. */
void scUpdateWait(Obj *obj) {
    ScWork *w = SC(obj);

    if (*gScVarWaitTimer > 0) {
        *gScVarWaitTimer -= 1;
        w->state = 3;
        return;
    }
    if (w->state == 3 && *gScVarWaitTimer == 0) {
        w->state = 0;
        w->text++;
        return;
    }
    if (*gScVarWaitTimer == -1) {
        w->state = 3;
    }
}

/* Window i's string task, made if it has none (the work pointer is assigned after its
 * declaration, so IDO adds i * 4 to obj again after the call). */
Obj *scStringTaskCheck(Obj *obj, s32 i) {
    ScWork *w;

    w = SCP(obj);
    if (w->str[i] == NULL) {
        DPRINTF("scStringTaskCheck: no string task[%d] !!!!\n", i);
#ifndef AVOID_UB
        //! @bug strCreate takes the parent task; the original passes none, and a0 still holds
        //! obj, so the string task is made under the runner.
        w->str[i] = strCreate();
#else
        w->str[i] = strCreate(obj);
#endif
    }
    return w->str[i];
}

ScCmd *scGetPc(Obj *obj) {
    return SC(obj)->pc;
}

s16 *scGetText(Obj *obj) {
    return SC(obj)->text;
}

/* The same as scStringTaskCheck. */
Obj *scStringTaskCheck2(Obj *obj, s32 i) {
    ScWork *w;

    w = SCP(obj);
    if (w->str[i] == NULL) {
        DPRINTF("scStringTaskCheck: no string task[%d] !!!!\n", i);
#ifndef AVOID_UB
        //! @bug As in scStringTaskCheck: no argument, and a0 still holds obj.
        w->str[i] = strCreate();
#else
        w->str[i] = strCreate(obj);
#endif
    }
    return w->str[i];
}

s32 scRunGetNo(Obj *obj) {
    return SC(obj)->no;
}

u16 scGetFile(s32 n) {
    return gScenarioFiles[n];
}

u32 scGetStart(s32 n) {
    return gScenarioStarts[n];
}

/* Whether the runner opened a window. */
s32 scIsWindowOpened(Obj *obj) {
    if (SC(obj)->flags & 1) {
        return 1;
    }
    return 0;
}

/* Block n (0x80 bytes) from ROM through a cache: read in when it is not the cached one. */
u8 *scCacheGetBlock(ScCache *c, s32 n) {
    s32 pad;
    u8 *data;
    s32 key = n + 1;
    s32 found;

    found = cacheGetSlot(c->cache, &data, key);
    if (data == NULL) {
        return NULL;
    }
    if (found != key) {
        dmaReadRom(c->rom + (n << 7), data, 0x80);
    }
    return data;
}
