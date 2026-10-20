#include "game.h"
#include "file.h"
#include "heap.h"
#include "scheduler.h"
#include "sys.h"

/* The screen clear, the game's init, and the memory arena: one block that grows up from its
 * bottom (0x80037990) and down from its top (0x80037994). */

/* The depth buffer (file 4's RAM, 0x8036A000) as the RDP sees it: a link-time symbol at the
 * physical address itself. `(u32) gZBuffer - 0x80000000` compiles to a runtime add instead. */
extern u8 D_36A000[];
#define ZBUFFER_PHYS ((u32) D_36A000)

extern u8 gGameWork[0xE4];
extern u32 gSysArenaBottom; /* the arena's bottom: the next free address going up */
extern u32 gSysArenaTop; /* its top: the last address handed out going down */
extern u32 gSysArenaBase; /* the bottom to return to (sysArenaResetBottom) */
extern u16 gFramebuffers[3][SCREEN_WIDTH * SCREEN_HEIGHT]; /* the framebuffers */

/* Clear the frame: black, then (inside the scissor, if one is set) either one RGBA5551 colour or a
 * vertical fade from fadeFrom at line fadeStart to fadeTo at fadeEnd. The fade is drawn as one
 * primitive-colour rectangle per run of lines whose rounded colour is the same.
 * MATCHED 2026-10-03 (see the end of this note). The history:
 * NON_MATCHING (333 lines, 532 of the original's 537 instructions; the first draft was 693 and
 * 560), 2026-10-02. Read off the game:
 * - the outer loop's bound is a variable ("end", 240): the game tests the first line with
 *   "slti 240" and the loop's foot with "slt v1,t5", t5 set to 240 at the fade's start;
 * - y is an s32 (the game never masks it; as a u16 this C had 8 more andi), and the run of
 *   equal lines is a rotated "while (y < fadeEnd)" whose colour-change exit jumps past
 *   "y = SCREEN_HEIGHT" (the game's beqz at its head and bnel at its foot; 410 to 342);
 * - the scissor corners are signed (s32): IDO converts a u16 to float as unsigned, with the
 *   2^32 fix-up, which the game has for the fade colours (u16) but not for the corners;
 * - the fade's locals are declared in its block: the colour image's "_g", spilled across the
 *   call, then has its home 7 words below the frame's top as in the game (0xB4 of 0xD0);
 * - y is set from fadeStart before n (the game reuses that one load; set later it is reloaded);
 * - the solid colour is computed inside gDPSetFillColor (its command word is stored first).
 * Left: the game's frame is 0xD0 (this C's 0x78), it spills lines at 0x3C where this C spills
 * toB, it keeps 240 in t5 for the outer loop's test (slt), and registers differ.
 * 2026-10-03, sweep B's find (score 5146 → 3638): gDPSetColorDither written out with its length
 * field (len - 1, 1) from a FAKE variable "one" set beside the first cycle type: 333 → 313. IDO
 * folds the word back to the game's constant (0xE3001801); only the schedule moves.
 * 2026-10-03, check's stack map: the game touches only 0x18-0x3C (f20-f24, s0, ra, one float
 * spill) and _g's 0xB4 of its 0xD0; 0x40-0xB0 and 0xB8-0xCC are never touched (an unused array
 * or locals that never needed a home, IDO_NOTES 216). A FAKE "s32 pad[20];" first in the fade
 * block: frame 0xD0, _g at 0xB0 (one word low), 309 lines: the instructions (mix distance 9)
 * are the real difference, so the pad is not kept.
 * The instruction mix (game 537, this C 532): the game has five more mtc1. Four are the double
 * 0.5 of "r += 0.5" etc., rebuilt before each of the three add.d (mtc1 zero + mtc1 at, one
 * shared "lui at,0x3FE0": ugen's li.d each time), where this C keeps 0.5 in f0 for all three:
 * uopt gave the constant a register here and not in the game, so the game's float registers
 * were fuller (or the constant's priority lower) at that point. The fifth is n's conversion:
 * the game converts n in both paths of "if (n == 0) n = 1" (bnel ...; mtc1 a0 in the slot, and
 * again after the li), this C once after the join.
 * 2026-10-03, from screen's branch #10 (game bnezl with the mtc1 in its slot, this C bnez with
 * sub.s there): "dr = toR - r" before the test and "dr /= lines" after: 333 (from 313), not kept;
 * the slot is filled from the join's first instruction only when the join starts with mtc1.
 * 2026-10-03: "lines" declared volatile (FAKE) and "s32 pad[19];" first in the fade block: 327
 * lines, mix distance 7 (two mtc1 nearer), frame 0xD0 with _g at 0xB0. The greedy from there
 * kept no step: its hills (empties, splits) led to 338 lines at mix 6. Not kept.
 * The greedy with its moves mode (2026-10-03, 313 to 309): "b += 0.5;" moved above "r += 0.5;";
 * its hills (moves, volatiles) reached mix 5-6 at 330-340 lines, none lower.
 * 2026-10-03 (batch, by hand): the fade's combine was G_CC_PRIMITIVE, a BEHAVIOUR error: the game's
 * second word is 0xFFFDFCFE, alpha 1 (G_ACMUX_1) in both cycles, not PRIMITIVE (0xFFFDF6FB).
 * Written as gDPSetCombineLERP(0, 0, 0, PRIMITIVE, 0, 0, 0, 1, ...): 309 → 307.
 * Then, each from 307: the inner tests as "(u32) nr != (u32) r" (game: bne conv,ir; this C
 * bne ir,conv): 307, uopt CSEs it back to ir; as "ir != (u32) nr": 307, no flip either (IDO
 * puts the variable first whatever the order). "/ n" with no "lines" (the game's 0x3C is a
 * compiler-temp slot): 309, identical code (the conversion is a temp either way). Not kept.
 * The FAKE "s32 pad[20];" first in the fade block again (frame 0xD0, _g 0xB0 for 0xB4): 307 →
 * 303 at mix 9, kept by the keep rule (the game's 29 untouched words there are real). _g is
 * one word low because of the FAKE "one" above it (the game has 4 named locals + 2 _g there).
 * "b = b + 0.5" etc. for "+=": 303, identical. Not kept.
 * HILL (the game's structure, 2026-10-03): the game's inner-loop colours sit in f0/f2/f12, the
 * SAME registers as toR/toG/toB, and "lines" is the value spilled (0x3C): the original reuses
 * toR/toG/toB as the running colours ("toR = r; while (..) { toR += dr; ...} r = toR;", no
 * nr/ng/nb). That gives the game's bnezl with mtc1 in the slot, toR/toG/toB in f0/f2/f12, lines
 * stored and reloaded: mix 9 → 5, but 323 lines (pad[20] kept: frame 0x130; without it 0xE0,
 * _g 0xC0, 323). Also without ir/ig/ib (PrimColor takes r, g, b; the tests "(u32) toR !=
 * (u32) r"): the game's frame 0xD0 with no pad, _g 0xB0 (the FAKE "one" is the word too many
 * above it, and it puts the temp at 0x38 for 0x3C), 325 lines, mix 5; the compares stay
 * "bne ir,conv". From that 325: the adds as r, g, b (the game's cvt.d.s order): 327; the FAKE
 * "one" removed (gDPSetColorDither written plainly): 331, mix 5, and the frame, _g's home and
 * the temp slot all the game's (check prints no stack line): the hill with NO FAKE at all;
 * then "y = fadeStart" after "if (n == 0) n = 1;" (the game loads fadeStart into a1 and copies
 * it to v0 later): 340, mix 4 (533 instructions), fadeStart in t0, not kept.
 * THE HILL, for the next sweep (331, mix 5, frame exact): the C below with (1) no "one" (plain
 * gDPSetColorDither), no pad, (2) no nr/ng/nb: "toR = r; toG = g; toB = b;" before the inner
 * while, "toR += dr;" etc. in it, "r = toR;" etc. after fill, (3) no ir/ig/ib:
 * "gDPSetPrimColor(gfx++, 0, 0, r, g, b, 0);" and "(u32) toR != (u32) r || ...". Left there:
 * the game rebuilds the double 0.5 for each of the three adds (in f4, f8, f10; this C keeps it
 * in f0 once: f0 is busy in the game's block, free here), the scissor/fill-colour OR order,
 * the inner compares' operand order, and fadeStart's a1 copy. Not kept by the keep rule.
 * CURRENT before this pass: 303 lines, mix 9 = the 309 form + the combine fix + pad[20]
 * (greedy then took it to 299 with "s32 lrx;" in a block to the end: superseded).
 * 2026-10-03 (cc -S read, IDO_NOTES 279's FIFO): THE HILL above with the three
 * "+= 0.5" moved up to right after "db = ..." (before the first command), in r, g, b order:
 * 165 lines, mix 1, frame exact, no FAKE. cc -S showed why: written after the commands, the 0.5
 * got a uopt register (li.d $f0 once, f0 free there); written right after the divides it is
 * rebuilt by ugen for each add (three li.d), and the FIFO then gives the game's adds exactly
 * (li.d f4; add.d f6,f8,f4; li.d f8; add.d f4,f10,f8; li.d f10; add.d f8,f6,f10). The order
 * b, r, g or a "f64 half" local, FAKE boundaries between the adds, "r = r + 0.5": all kept
 * the li.d $f0 when the adds came after the commands.
 * Then the scissor read through gauge.c's named-field view (IDO sorts an OR's array-element
 * operands, a struct field keeps the source order; IDO_NOTES 273): 165 → 149, mix 1; the
 * game's load order (uly, ulx, lrx, lry) and registers.
 * Then "y = fadeStart" moved from above n to after the adds (before the first command): 149 →
 * 96, MIX 0 (537 instructions, the game's): the CSE'd load is copied into y at the join (the
 * game's "move v0,a1"). cc -S of the other places: at the top (ugen's "lhu v1; move v0,v1",
 * as1 folds it), "* 1" on it (same), right after "if (n == 0)" (the copy at the join too, so
 * as the adds' place), after n's statement (copy before the branch). Left at 96: fadeStart's
 * load one register up (a2 for a1, n in a1 for a0), the commands' pointer registers after it,
 * lines' temp word at 0x38 (game 0x3C), and t4 (lrx) not reused for the PrimColor word.
 * Then fadeStart read into "next" first ("next = fadeStart; n = fadeEnd - next; ... y = next;"
 * after the adds): 96 → 90, mix 0, frame exact (no stack line). The CSE'd load across blocks
 * made a uopt temp that took 0x3C before lines' (275); a named variable has no temp word, and
 * reusing next (dead until the loop) adds no frame word, where a new "s32 start" put lines at
 * 0x38 again (cc -S). fadeStart now in a1 as in the game. "top" reused instead: start in t0.
 * Left at 90: n in a2 (game a0), so the dither command's pointer is a0 here (game a2) and the
 * next ones follow; the loop-invariant lrx/ulx shifts hoisted in the other order; "lui t4,
 * 0xFA00" (game reuses lrx's t4). "top = 0" before the commands: no change (cc -S).
 * Then (cc -S read): uopt tested "next < 240" at the loop's entry for "y < end" (it substitutes
 * y's copy, IDO_NOTES 278), so next/fadeStart (a1) lived through all five commands and pushed
 * their pointers off a1. A FAKE dead "next = 0;" after "y = next;" kills the substitution, and
 * the divides by n itself (no "lines = n" copy) give n the game's a0: 90 → 28, mix 0, frame
 * exact, every command pointer the game's (v1, a2, v1, a0, a1). "lines" stays declared,
 * unused: without it the frame is 0xC8 (cc -S). Each alone (cc -S): the kill with "lines":
 * n a2; no "lines" without the kill: n a0 but the dither's pointer t2; "next = y" as the
 * kill: no effect. Left at 28: the inner compares' operand order (game "bne conv,ir": this C
 * "bne ir,conv"), the solid fill colour's OR order and its loads, "move t0,zero" placed.
 * Then the solid colour's fadeFrom read through a named-field view (Rgb16, as the scissor):
 * 28 → 8, mix 0; that block is the game's. The compares as "(u32) toR != (u32) r * 1" (FAKE
 * late fold, so the CSE is an expression at ordering time): 103 lines, mix 1, the loop's
 * registers moved and the compares stayed "bne ir,conv". Not kept.
 * Left at 8: "move t0,zero" (top = 0) one place later than the game's (after the combine's
 * stores, game before them), and the three inner compares' operand order (game "bne conv,ir":
 * the conversion of toR first, the outer loop's (u32) r second).
 * Then "top = 0;" moved above the combine command: 8 → 6 (scratch check; the copy's own check
 * follows). Left at 6: only the three inner compares' operand order.
 * Then the compares written outer colour first, "(u32) r != (u32) toR || ...": MATCH (cc -S
 * showed ugen's "bne conv,ir" before the check). With the conversion first, uopt put the
 * outer loop's CSE first; "- ... != 0" and "^ ... != 0" kept subu/xor; "!(.. == .. && ..)"
 * no flip. This pass's result: MATCH, with one FAKE ("next = 0;") and one unused declaration
 * ("lines", the frame word). A parallel attempt reached 22 (mix 0, no FAKE) by the same road.
 * 2026-10-03, fakes sweep: the dead store is now part of the loop edges init, "top = next = 0;"
 * where "top = 0;" stood (MATCH). A store to next between "y = next" and the guard is needed:
 * uopt propagates the copy globally ("y = next" before the n == 0 branch: the copy lands before
 * the branch and the guard still tests a1, 30 lines), and a live redefinition of the start
 * variable merges its ranges into one register (the top variable: start in t0). Also tried:
 * fadeStart read again into y after the adds with no next and no "lines" (the CSE temp takes
 * the lines word, frame exact, but the guard tests the temp: 26 lines); "y = fadeStart;
 * n = fadeEnd - y;" (y loaded straight into v0, frame 0xC8). */
Gfx *sysClearFrame(Gfx *gfx) {
    s32 ulx;
    s32 uly;
    s32 lrx;
    s32 lry;

    gDPPipeSync(gfx++);
    gDPSetCycleType(gfx++, G_CYC_FILL);
    gDPSetColorImage(gfx++, G_IM_FMT_RGBA, gGameState.colorImageSize, SCREEN_WIDTH,
                     VIRTUAL_TO_PHYSICAL(gFramebuffers[mainSchedGetBufferIndex(&gMainSched)]));
    gDPSetFillColor(gfx++, 0);
    gDPFillRectangle(gfx++, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
    if (gGameState.scissor.ulx | gGameState.scissor.uly | gGameState.scissor.lrx | gGameState.scissor.lry) {
        ulx = gGameState.scissor.ulx;
        uly = gGameState.scissor.uly;
        lrx = gGameState.scissor.lrx;
        lry = gGameState.scissor.lry;
        gDPSetScissor(gfx++, G_SC_NON_INTERLACE, ulx, uly, lrx, lry);
    } else {
        ulx = 0;
        uly = 0;
        lrx = SCREEN_WIDTH - 1;
        lry = SCREEN_HEIGHT - 1;
    }
    if (gGameState.fade) {
        f32 r;
        f32 g;
        f32 b;
        f32 toR;
        f32 toG;
        f32 toB;
        f32 lines; /* unused: a frame word in the game */
        f32 dr;
        f32 dg;
        f32 db;
        s32 n;
        s32 y;
        s32 top;
        s32 next;
        s32 end = SCREEN_HEIGHT;

        r = gGameState.fadeFrom.r;
        g = gGameState.fadeFrom.g;
        b = gGameState.fadeFrom.b;
        toR = gGameState.fadeTo[0];
        toG = gGameState.fadeTo[1];
        toB = gGameState.fadeTo[2];
        next = gGameState.fadeStart;
        n = gGameState.fadeEnd - next;
        if (n == 0) {
            n = 1;
        }
        dr = (toR - r) / n;
        dg = (toG - g) / n;
        db = (toB - b) / n;
        r += 0.5;
        g += 0.5;
        b += 0.5;
        y = next;
        gDPPipeSync(gfx++);
        gDPSetColorDither(gfx++, G_CD_MAGICSQ);
        gDPSetCycleType(gfx++, G_CYC_1CYCLE);
        gDPSetRenderMode(gfx++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
        top = next = 0; /* the zero in next is never read: it stops uopt testing next for y at the loop entry (IDO_NOTES 278, 282) */
        gDPSetCombineLERP(gfx++, 0, 0, 0, PRIMITIVE, 0, 0, 0, 1, 0, 0, 0, PRIMITIVE, 0, 0, 0, 1);
        while (y < end) {
            gDPSetPrimColor(gfx++, 0, 0, r, g, b, 0);
            toR = r;
            toG = g;
            toB = b;
            while (y < gGameState.fadeEnd) {
                toR += dr;
                toG += dg;
                toB += db;
                if ((u32) r != (u32) toR || (u32) g != (u32) toG || (u32) b != (u32) toB) {
                    goto fill;
                }
                y++;
            }
            y = SCREEN_HEIGHT;
        fill:
            next = y + 1;
            gDPFillRectangle(gfx++, ulx, top, lrx, next);
            r = toR;
            g = toG;
            b = toB;
            y = next;
            top = next;
        }
    } else {
        gDPSetFillColor(gfx++, (GPACK_RGBA5551(gGameState.fadeFrom.r, gGameState.fadeFrom.g, gGameState.fadeFrom.b, gGameState.fillAlpha) << 16) |
                                   GPACK_RGBA5551(gGameState.fadeFrom.r, gGameState.fadeFrom.g, gGameState.fadeFrom.b, gGameState.fillAlpha));
        gDPFillRectangle(gfx++, ulx, uly, lrx - 1, lry - 1);
    }
    gDPPipeSync(gfx++);
    return gfx;
}

/* Clear the depth buffer. */
Gfx *sysClearZBuffer(Gfx *gfx) {
    gDPPipeSync(gfx++);
    gDPSetCycleType(gfx++, G_CYC_FILL);
    gDPSetScissor(gfx++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    gDPSetColorImage(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, ZBUFFER_PHYS);
    gDPSetDepthImage(gfx++, ZBUFFER_PHYS);
    gDPSetFillColor(gfx++, (GPACK_ZDZ(G_MAXFBZ, 0) << 16) | GPACK_ZDZ(G_MAXFBZ, 0));
    gDPFillRectangle(gfx++, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
    gDPPipeSync(gfx++);
    return gfx;
}

/* The game's init: clear the state blocks and set their first values, then empty the file
 * loader's queue. */
void sysInitGame(void) {
    s32 i;

    heapMemClear(&gGameState, sizeof(gGameState));
    heapMemClear(gGameWork, sizeof(gGameWork));
    heapMemClear(&gSaveData, sizeof(gSaveData));
    for (i = 0; i < 4; i++) {
        gSaveData.players[i].chara = -1;
        gSaveData.players[i].life = 3;
        gSaveData.players[i].armor = 3;
        gSaveData.players[i].lives = 16;
        gSaveData.players[i].money = 500;
    }
    gSaveData.charaCount = 4;
    gGameState.fillAlpha = 1;
    gGameState.colorImageSize = G_IM_SIZ_16b;
    gGameState.retracesPerFrame = 1;
    fileInit();
}

/* Take size bytes from the bottom of the arena (8-aligned). */
void *sysArenaAllocBottom(s32 size) {
    u32 p = (gSysArenaBottom + 7) & ~7;

    gSysArenaBottom = p;
    gSysArenaBottom += size;
    return (void *) p;
}

/* ... and zeroed. */
void *sysArenaCallocBottom(s32 size) {
    void *p = sysArenaAllocBottom(size);

    heapMemClear(p, size);
    return p;
}

/* Take size bytes from the top of the arena (8-aligned). */
void *sysArenaAllocTop(s32 size) {
    gSysArenaTop = (gSysArenaTop - size) & ~7;
    return (void *) gSysArenaTop;
}

/* ... and zeroed. */
void *sysArenaCallocTop(s32 size) {
    void *p = sysArenaAllocTop(size);

    heapMemClear(p, size);
    return p;
}

void sysArenaSetBottom(u32 bottom) {
    gSysArenaBottom = bottom;
}

void sysArenaResetBottom(void) {
    gSysArenaBottom = gSysArenaBase;
}

void sysArenaSetTop(u32 top) {
    gSysArenaTop = top;
}

/* Set the arena to [bottom, top). */
void sysArenaSet(void *bottom, void *top) {
    sysArenaSetBottom((u32) bottom);
    sysArenaSetTop((u32) top);
}

/* Reset the arena's bottom, with a new top. */
void sysArenaReset(u32 top) {
    sysArenaResetBottom();
    sysArenaSetTop(top);
}

/* This file's .data (subsegment 0x25140), in ROM order. */
Gfx gSysEmptyDl[] = {
    gsSPEndDisplayList(),
};
u16 gAsciiFontTlut[20] = {
    0x0, 0xFFFF, 0x18C7, 0xDEF7, 0xE739, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0,
};
s16 gAsciiGlyphTable[0xD8] = {
    0xF00, 0x300, 0xC00, 0x200, 0xD00, 0x200, 0xE00, 0x200,
    0xB00, 0x200, 0xF00, 0x300, 0xF00, 0x300, 0xF00, 0x200,
    0x900, 0x200, 0xA00, 0x200, 0x700, 0x200, 0x500, 0x200,
    0x0, 0x300, 0x400, 0x200, 0xF00, 0x300, 0x600, 0x200,
    0x0, 0x0, 0x100, 0x0, 0x200, 0x0, 0x300, 0x0,
    0x400, 0x0, 0x500, 0x0, 0x600, 0x0, 0x700, 0x0,
    0x800, 0x0, 0x900, 0x0, 0x100, 0x300, 0xF00, 0x300,
    0xF00, 0x300, 0x200, 0x300, 0xF00, 0x300, 0x300, 0x300,
    0x400, 0x300, 0xA00, 0x0, 0xB00, 0x0, 0xC00, 0x0,
    0xD00, 0x0, 0xE00, 0x0, 0xF00, 0x0, 0x0, 0x100,
    0x100, 0x100, 0x200, 0x100, 0x300, 0x100, 0x400, 0x100,
    0x500, 0x100, 0x600, 0x100, 0x700, 0x100, 0x800, 0x100,
    0x900, 0x100, 0xA00, 0x100, 0xB00, 0x100, 0xC00, 0x100,
    0xD00, 0x100, 0xE00, 0x100, 0xF00, 0x100, 0x0, 0x200,
    0x100, 0x200, 0x200, 0x200, 0x300, 0x200, 0xF00, 0x300,
    0xF00, 0x300, 0xF00, 0x300, 0xF00, 0x300, 0x800, 0x200,
    0xF00, 0x300, 0xA00, 0x0, 0xB00, 0x0, 0xC00, 0x0,
    0xD00, 0x0, 0xE00, 0x0, 0xF00, 0x0, 0x0, 0x100,
    0x100, 0x100, 0x200, 0x100, 0x300, 0x100, 0x400, 0x100,
    0x500, 0x100, 0x600, 0x100, 0x700, 0x100, 0x800, 0x100,
    0x900, 0x100, 0xA00, 0x100, 0xB00, 0x100, 0xC00, 0x100,
    0xD00, 0x100, 0xE00, 0x100, 0xF00, 0x100, 0x0, 0x200,
    0x100, 0x200, 0x200, 0x200, 0x300, 0x200, 0x0, 0x0,
    0x100, 0x0, 0x200, 0x0, 0x300, 0x0, 0x400, 0x0,
    0x500, 0x0, 0x600, 0x0, 0x700, 0x0, 0x800, 0x0,
    0x900, 0x0, 0xA00, 0x0, 0xB00, 0x0, 0xC00, 0x0,
    0xD00, 0x0, 0xE00, 0x0, 0xF00, 0x0, 0x0, 0x0,
};
Vp gSysScreenVp = { {
    { SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2, G_MAXZ / 2, 0 },
    { SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2, G_MAXZ / 2, 0 },
} };
Gfx gSysScreenSetupDl[] = {
    gsDPPipeSync(),
    gsSPViewport(&gSysScreenVp),
    gsDPSetScissor(G_SC_NON_INTERLACE, 16, 8, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 8),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                          G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPTexture(0, 0, 0, G_TX_RENDERTILE, G_OFF),
    gsSPEndDisplayList(),
};
void (*gSysPrintfTable[2])(const char *fmt, ...) = { rmonPrintf, osSyncPrintf };
