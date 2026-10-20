#include "tlb.h"

/* "dkeeper": the demo's keeper. While a demo plays it watches its placement's buttons on its
 * controller (and, in an attract demo, any controller's A or Start); once pressed it fades
 * out and goes to the title. */
typedef struct DkeeperObj {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ Frame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u8 mode; /* 0 watching, 1 leaving */
    /* 0x35 */ u8 port; /* the controller it watches */
    /* 0x36 */ u16 buttons; /* the buttons that end the demo */
    /* 0x38 */ u16 timer;
} DkeeperObj;

/* MATCHED 2026-10-12 (trace + ugen-edit). Trace: the constant 1 of "unk15 = 1" was in
 * uopt's {1|2} range (the u8 stores' 1, with mode 0's, range 142 -> a0); the game's is in the
 * compares' {1|0} range (136: blocks 1, 5, 10, 23), which then spans eneSetup's call, has a0/a1
 * forbidden and takes a2; obj then stays in a0 for the name store. Also "i = w->timer" (range 57)
 * took v0 first and pushed the timer load (121) to a0. ugen-edit proof (full.s 45-214, blk_a.s):
 * the 1s in $6, no obj reload, the timer in $2: real 0. C: the 18 test as a plain "else if"
 * (real 10, timer range -> v0), then the store through s8 (real 0). demoPlaying (0x15) is read in
 * contpak.c (an s8 there: no rumble while set); fadeColor (0x16-0x18) is the fade colour (u8).
 * 2026-10-17: GameWork declares demoPlaying an s8 (object.h), so the store is plain (measured).
 * History, NON_MATCHING: 87 lines, mix distance 1, from one instruction: the constant 1 (the msg 1
 * test, unk15's 1) takes a0 here, so the name store reloads obj from its home ("lw t8,0x30");
 * the game keeps obj in a0 for that store and puts the 1 in a2. Measured: no pad, 95 (the
 * temp 0x1C, the game's 0x18); "s32 pad;" fixes the frame, 87; the two empty strings of the
 * file's .rodata as dead prints in cases 2 and 1, 87 (kept: the .rodata needs them); a FAKE
 * "if (obj) {}" before the switch, 87; empties' best, "if (w) {}" there, 86 (not kept); no
 * split helps (splits, 76 positions).
 * 2026-10-12: "w->port = data & 0xFF" (IDO_NOTES 351: as1 deletes the andi, its temp stays
 * spent, so sra takes the game's t1): real 33 to 10. Left: only the constant 1's register (a0
 * here, a2 in the game) and the name store through a reloaded obj. Then: `results` (every call
 * both ways) 10; FAKE "if (obj) {}" before the switch 10. A probe (k0 is
 * this C): data as u32, the timer tests as a switch, the name store and eneSetup on one line,
 * the mode stores' 1 as "1U" or "(u8) 1": every 1 still in a0.
 * 2026-10-12 (second try): the permuter's best (55) reads the timer into a local for mode 1's tests;
 * "i = w->timer; if (i == 1) ... else if (i == 18)": 10, the same. w set per case (case 2 after
 * the name store, case 0 first; the game's dead "addiu v1,v1,0x20" in case 2's exit slot): 45.
 * No w, every field through a FX(obj) macro: 138 (obj then kept in a register, frame 0x18).
 * A probe (bisect): without case 1 (d2) obj stays in a0 for the name
 * store as in the game; without case 0's body (d0) too. So the msg-1 test's 1, in one web with
 * case 0's 1s, outranks obj's first range for a0 here; in the game that web avoids a0, a1, v0
 * and v1, i.e. it also spans case 2's call (unk15's 1 in the same web).
 * 2026-10-12 (third try): the timer test as "== 1U": 13 (the msg dispatch's and mode test's 1 then
 * "li at,1", not kept in a register: the timer test's 1 is what makes the web a register
 * candidate, in both). The two PRINTF("") before their unk15 stores: 10, the same. A FAKE
 * "if (obj) {}" at the head of case 2: 10. reloads and empties sweeps: nothing under 10.
 * The permuter (base 335 to 55): the 18 test through i in an else ("} else { i = w->timer; if
 * (i == 18) ..."): still real 10 but 51 lines to 24, length right: obj stays in a0 for the
 * name store as in the game. Left: the 1 takes a1 at the dispatch and in mode 1 (game a2) and
 * a0 after eneSetup (game a2); the timer is loaded into i's a0 (game v0). The same through
 * data: 24. Both timer tests on i: back to 51. Kept (the form the permuter goes on from).
 * Permuter from this form (base 55, ~200 iterations): an empty "if (w->mode) {}" before the 18
 * test, real 9 (mode loaded into a0, not kept: a FAKE for one line).
 * 2026-10-12 (fourth try): permuter again from this form (--stack-diffs, ~600 iterations, 15 min):
 * nothing under its base 55. */
s32 dkeeperProc(Obj *obj, s16 msg, void *arg) {
    DkeeperObj *w = (DkeeperObj *) FX(obj);
    s32 pad;
    s32 data;
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "dkeeper";
            data = eneSetup(obj, (EneArg *) arg);
            w->port = data & 0xFF; /* the mask as1 deletes spends the game's temp */
            w->buttons = data >> 16;
            gGameWork.demoPlaying = 1; /* an s8 store: the game's 1 shares the compares' (signed) range */
            PRINTF("");
            break;
        case 1:
            gGameWork.demoPlaying = 0;
            PRINTF("");
            break;
        case 0:
            w->timer++;
            switch (w->mode) {
                case 0:
                    if ((joyGetButton(w->port, 0) & w->buttons) == w->buttons) {
                        w->timer = 0;
                        w->mode = 1;
                    }
                    if (joyDemoGetMode() == 2) {
                        for (i = 0; i < 4; i++) {
                            if (gJoyContPads[i].button & (A_BUTTON | START_BUTTON)) {
                                w->timer = 0;
                                w->mode = 1;
                            }
                        }
                    }
                    break;
                case 1:
                    if (w->timer == 1) {
                        sndPlay(10);
                        fadeSetSpeed(16.0f);
                        fadeSetTarget(255.0f);
                    } else if (w->timer == 18) {
                        joyDemoEnd();
                        stageGotoTitle();
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
