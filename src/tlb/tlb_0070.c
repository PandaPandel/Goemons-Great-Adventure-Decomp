#include "tlb.h"

/* File 70: the clasher (a huge crusher that starts turning once a player is past 2830), the
 * clashboard (a ride it smashes into pieces), the pieces, and the ppress. */

/* The clasher's work: an enemy's up to 0x34. */
typedef struct ClasherWork {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ ACollFrame *box; /* made once it starts */
    /* 0x3C */ f32 speed; /* its turn per step */
    /* 0x40 */ u8 mode; /* 0 waiting, 1 turning */
} ClasherWork;

/* A piece's work. */
typedef struct PieceWork {
    /* 0x00 */ u8 pad00[0x10];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ s32 alpha;
    /* 0x3C */ s32 fade;
    /* 0x40 */ s32 timer; /* a dark puff at 1 (every fourth step below 10) */
} PieceWork;

/* The clashboard's work: an enemy's up to 0x34. */
typedef struct ClashboardWork {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ ACollFrame *dmg;
    /* 0x40 */ f32 base[3];
    /* 0x4C */ f32 unused4C; /* written 0.0f at its start, never read */
    /* 0x50 */ u16 timer;
    /* 0x52 */ u8 mode; /* 0 whole, 1 smashed */
} ClashboardWork;

/* The ppress's work: an enemy's up to 0x34. */
typedef struct PpressWork {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ Vec3f vel; /* a struct, not f32[3]: "height + vel.y" keeps the game's operand
                            * order, "height + vel[1]" does not (ppressProc's note) */
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ ACollFrame *atk; /* a root for the four boxes under it */
    /* 0x3C */ RideFrame *ride;
    /* 0x40 */ ACollFrame *dmg;
    /* 0x44 */ f32 height; /* over base */
    /* 0x48 */ f32 top;
    /* 0x4C */ f32 base;
    /* 0x50 */ u8 mode; /* 0 rising, 1 waiting at the top, 2 falling, 3 waiting at the bottom */
    /* 0x52 */ u16 timer;
} PpressWork;

/* acollAtkAddXYZ, acollAtkAddYZ and acollDmgAddXYZ return their box (main declares them void). */
typedef ACollFrame *(*AtkAddXYZFunc)(Frame *parent, Frame *model, u16 radius, s16 ox, s16 oy, s16 oz,
                                     s32 a);
typedef ACollFrame *(*AtkAddYZFunc)(Frame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*DmgAddXYZFunc)(Frame *parent, Frame *model, u16 radius, u16 height, s16 ox, s16 oy,
                                     s16 oz, s32 a);

extern Frame *gFrmLayer3;
extern Gfx clashboardData1Dl[];
extern Gfx clashboardData2Dl[];
extern Gfx clashboardData3Dl[];
extern Gfx clashboardStepDl[];
extern Gfx clasherDl[];
extern u8 clashboardData1Ride[];
extern u8 clashboardData2Ride[];
extern u8 clashboardData3Ride[];
extern Gfx ppressDl[];
extern Gfx ppressPieceDl[];
extern u8 ppressRide[];
f32 ppressBoxOfs[4][3] = { 0.0f, -111.0f, 205.0f, 0.0f, -153.0f, 7e+01f, 0.0f, -195.0f, -7e+01f, 0.0f, -238.0f, -212.0f }; /* the ppress's four boxes (x, y, z) */

s32 clasherProc(Obj *o, s16 msg, void *arg);
s32 clashboardStep(Obj *o, ClashboardWork *w);

/* The place along the course of the first player in play (0 if none). */
f32 clasherFirstPlayerPlace(void) {
    s32 i;
    f32 place;
    f32 height;
    f32 side;
    s32 zone;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plAskPos(gGameWork.players[i], &place, &height, &side, &zone);
            return place;
        }
    }
    return 0.0f;
}

/* The clasher: waits until a player is past 2830, then turns faster and faster (up to 4 degrees
 * a step) and sinks, a huge attack box on it; its sound's volume falls off with the height
 * from the camera. */
s32 clasherProc(Obj *o, s16 msg, void *arg) {
    ClasherWork *w = (ClasherWork *) FX(o);
    Obj *by;
    s32 pad;
    s32 vol;

    switch (msg) {
        case 2:
            o->name = "clasher";
            eneSetup(o, arg);
            w->pol = eneNewPol(o, clasherDl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayer3);
            break;
        case 1:
            sndPlay(0x8250);
            frmDestroy(&w->box->frame);
            frmDestroy(&w->pol->f);
            break;
        case 0:
            switch (w->mode) {
                case 0:
                    if (2830.0f < clasherFirstPlayerPlace()) {
                        w->box = ((AtkAddYZFunc) acollAtkAddYZ)(uda_atk_root_frame, &w->pol->f, 2300, -2260, 0, 0);
                        if (w->box == NULL) {
                            return -1;
                        }
                        w->box->kind = 14;
                        w->box->flags |= 0x10;
                        w->box->power = (s32) o;
                        sndPlay(0x250);
                        w->mode = 1;
                    }
                    break;
                case 1:
                    w->speed += 0.0008726646192f;
                    if (0.06981316954f < w->speed) {
                        w->speed = 0.06981316954f;
                    }
                    w->rot[1] += w->speed;
                    w->y += w->speed * 6.0f;
                    break;
                case 2:
                    break;
            }
            eneSetModelOnCourse(o, &w->pol->f);
            vol = (500.0f - fabsf(w->y - gGameWork.view->at[1])) * 256.0f / 500;
            if (vol < 0) {
                vol = 0;
            }
            sndSetParam(0x250, 1, vol - 256);
            if (w->box != NULL && acollAtkWasHit(w->box) == 1) {
                by = (Obj *) w->box->hitPower;
                if (by != NULL) {
                    tskSendMsg(by, 7, o);
                }
            }
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* A piece of a smashed clashboard: flies up and falls, turning, and fades out; a dark puff at
 * the start. */
s32 clasherPieceProc(Obj *o, s16 msg, void *arg) {
    PieceWork *w = (PieceWork *) FX(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "piece";
            w->pol = (PolFrame *) frmCreateNext(gFrmLayer3, gFrmKindPol);
            if (w->pol == NULL) {
                return -1;
            }
            w->rot[1] = fadeRand(360) * 3.141592741f / 180;
            w->alpha = 255;
            break;
        case 1:
            frmDestroy(&w->pol->f);
            break;
        case 0:
            w->vel[1] += -0.4;
            w->x += w->vel[0];
            w->y += w->vel[1];
            w->z += w->vel[2];
            w->fade = -4;
            w->alpha += w->fade;
            if (w->alpha <= 0) {
                return -1;
            }
            eneSetModelOnCourse(o, &w->pol->f);
            if (w->timer > 0 && w->timer < 10 && (w->timer & 3) == 1) {
                itexPuffDark(w->pol->pos[0], w->pol->pos[1], w->pol->pos[2], 0.1f);
            }
            w->timer--;
            w->rot[0] += 0.1745329201f;
            w->rot[1] += 0.08726646006f;
            w->rot[2] += 0.05235987902f;
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetRenderMode(gGfxDlHead++, G_RM_FOG_SHADE_A, G_RM_AA_ZB_XLU_SURF2);
            gDPSetCombineLERP(gGfxDlHead++, TEXEL0, 0, SHADE, 0, TEXEL0, 0, PRIMITIVE, 0, ENVIRONMENT, 0, COMBINED, 0,
                              0, 0, 0, COMBINED);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Makes a piece (under the effect task) near x, y, z, thrown up and out at random; its model
 * is dl2 with n's files, at the scale. */
void clasherPieceMake(Obj *o, f32 x, f32 y, f32 z, f32 scale, Gfx *dl2, s32 n) {
    Obj *t = tskCreate(gEffectTask, clasherPieceProc, o->file, NULL);
    PieceWork *w;

    if (t != NULL) {
        w = (PieceWork *) FX(t);
        w->x = fadeRand(10) + x - 5.0f;
        w->y = y;
        w->z = fadeRand(10) + z - 5.0f;
        w->vel[0] = (fadeRand(40) - 20) / 10.0f;
        w->vel[2] = (fadeRand(40) - 20) / 10.0f;
        w->vel[1] = 4.0f;
        w->pol->dl2 = dl2;
        floadGetSetRam((void **) w->pol->segs, n);
        w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2] = scale;
        w->timer = 1;
    }
}

/* The clashboard: one of three boards (data's low byte), a ride with a damage box; when the
 * clasher hits it, it breaks into pieces. Message 7 answers 3. The jump table needs cases 3-6
 * written out, and pad0 gives the 0x58 frame. FAKE layout: each board case's two assignments
 * share one line, dl first, so IDO splits their lui/addiu as the game does (IDO_NOTES 351). */
s32 clashboardProc(Obj *o, s16 msg, void *arg) {
    s32 pad0;
    ClashboardWork *w = (ClashboardWork *) FX(o);
    s32 data;
    s32 pad;
    u8 *ride;
    Gfx *dl;

    switch (msg) {
        case 2:
            o->name = "clashboard";
            data = eneSetup(o, arg);
            switch ((u8) data) {
                case 1:
                default:
                    dl = clashboardData1Dl; ride = clashboardData1Ride;
                    break;
                case 2:
                    dl = clashboardData2Dl; ride = clashboardData2Ride;
                    break;
                case 3:
                    dl = clashboardData3Dl; ride = clashboardData3Ride;
                    break;
            }
            w->pol = eneNewPol(o, dl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayer3);
            w->ride = rideCreate(gRideTree1, (s32) o, &w->pol->f, (RideData *) eneFileAddr(o, ride, 0));
            if (w->ride == NULL) {
                return -1;
            }
            w->dmg = ((DmgAddXYZFunc) acollDmgAddXYZ)(ene_dmg_root_frame, &w->pol->f, 100, 50, 0, -25, 0, 0);
            w->rot[1] = (data >> 16) * 3.141592741f / 180;
            w->base[0] = w->x;
            w->base[1] = w->y;
            w->base[2] = w->z;
            w->unused4C = 0.0f;
            break;
        case 1:
            frmDestroy(&w->dmg->frame);
            frmDestroy(&w->ride->f);
            frmDestroy(&w->pol->f);
            break;
        case 7:
            if (arg != NULL) {
                *(s32 *) arg = 3;
            }
            break;
        case 0:
            w->timer++;
            if (clashboardStep(o, w) != 0) {
                return -1;
            }
            eneSetModelOnCourse(o, &w->pol->f);
            eneStepHookPost(o);
            break;
        case 3:
        case 4:
        case 5:
        case 6:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The clashboard's step: whole, it waits for a hit by the clasher (its damage box hit by a task
 * running clasherProc); smashed, it throws three pieces and returns 1 (it is then deleted). */
s32 clashboardStep(Obj *o, ClashboardWork *w) {
    s32 i;
    Obj *by;

    switch (w->mode) {
        case 0:
            if (w->dmg != NULL && acollDmgWasHit(w->dmg) == 1) {
                by = (Obj *) w->dmg->hitPower;
                if (by != NULL && by->proc == clasherProc) {
                    w->timer = 0;
                    w->mode = 1;
                }
            }
            break;
        case 1:
            for (i = 0; i < 3; i++) {
                soundIncamCam(0x2C0, w->pol->pos[0], w->pol->pos[1], w->pol->pos[2]);
                clasherPieceMake(o, w->pol->pos[0], w->pol->pos[1], w->pol->pos[2], 0.0700000003f,
                                       clashboardStepDl, 110);
            }
            return 1;
    }
    return 0;
}

/* The ppress: a press (a ride) that rises to its top (data's second byte), waits, and falls
 * back hard; four boxes under it send message 7 to what they hit, and the clasher smashes it
 * into pieces. Data's low byte 1 starts it at the top.
 * Matched 2026-10-15 (the last entry below). History, when it was NON_MATCHING:
 * (208 lines, this C 378 for the game's 376, mix distance 2, frame 0x60 as the
 * game's), 2026-10-04: in the box loop the game keeps acollAtkAddXYZ's address in s0 and builds
 * 120 for box->kind each pass (li t6); this C spills the address (a temp at 0x38 the game
 * lacks; with "if (box != NULL) box->kind = 120;" it also hoisted 120 into s0, 212), which
 * moves every later t-register by one. "if (box == NULL) continue;" gave 208. Also the falling
 * case loads height into f16 and vel[1] into f10 (the game: f10, f16), both orders measured.
 * Measured: pointer loop "p != &D[4]" (260: IDO CSEs the base and adds 48; the game builds the
 * end as its own address, as the index loop does); block-scoped pointer (260); a FAKE extra use
 * of acollAtkAddXYZ in the loop (260, 120 still in s0); index loop 258; without a height local
 * ("height + vel[1] <= 0", the else "+=") 226 and the game's frame; "data & 0xFF" for the
 * game's lw/andi (223); top before base (212); vel[1] + height (212, no change).
 * 2026-10-12 (real 50): this C's s0 holds the loop's end (ppressBoxOfs + 48, hoisted) and
 * acollAtkAddXYZ's address sits in v0, spilled; the game holds the address in s0 and rebuilds
 * the end each pass. The address in a local function pointer set before the loop: real 64
 * (frame 0x68, the same spill).
 * 2026-10-12 (second try, start 50): "i != 4" (50, same); the offsets as a Vec3f array read
 * as .x/.y/.z, tlb_0058's matched form (50, same; with tlb_0058's "if (box != NULL)" 55).
 * 2026-10-12 (third try): eneFileAddr's result read into a local first (IDO_NOTES 358): new local 65
 * (frame 0x68), in by's word 50 (the 0x38 temp stays: it is the address's spill); ppressBoxOfs
 * as a function static 50 (212 lines, the end still hoisted). A probe:
 * the cast call and a real prototype give the same code (no register pressure there).
 * 2026-10-12 (fourth try): "(AtkAddXYZFunc) &acollAtkAddXYZ" 50; i as u32 50 (no change).
 * 2026-10-12 (fifth try), match_function.py trace: in ugen's listing, uopt's code motion has put
 * "la $2, acollAtkAddXYZ" ahead of the loop and spilled it at 56(sp). That is the whole +2:
 * the game forms the address at each call. It is not the cast: with acoll.c's
 * acollAtkAddXYZ returning _atkLinkAdd's ACollFrame * (as six TLB files' casts suggest),
 * tlb.h to match, and a plain call: 50, the same hoist (reverted). Open: what stops uopt's
 * loop-invariant motion of a far call's address in the game's loop.
 * 2026-10-12 (sixth try), the loop's test after the call in the source (a tie broken by order?):
 * "i = 0; do { ... continue ... } while (++i < 4);" 50, the same code; with "if (box != NULL)" and
 * "i++; } while (i < 4);" 55 (212 lines, as the for form with that if).
 * Probes (the file included as built, the loop varied; the
 * three loop invariants are the address, the end and the 120): s0 goes to the end (this C, and
 * with a dead test of the address before the loop or in it, of i or box after the loop, of box
 * before the continue, a row pointer loop, p = ppressBoxOfs[i]) or to the 120 ("if (box != NULL)"
 * forms, a test of the call in the if); with a dead "if (i) {}" at the loop's head the end is
 * rebuilt each pass as in the game, but the address goes to t1 and is spilled, s0 unused. The
 * address never took s0.
 * 2026-10-12 (seventh try), trace + ugen-edit (full.s
 * 1474-1522): the game's loop is this listing with (1) no "move $16,$0" (this C's dead i = 0
 * before the strength-reduced loop: i is s0's range 60, blocks 20/76/78, shared with the clasher
 * loop), (2) "la $16, acollAtkAddXYZ" in the preheader and "jal $16", no spill at 56, (3) the end
 * "la ppressBoxOfs+48" formed in the latch (a constant left uncoloured). That edit: the loop and
 * everything up to instruction 131 match; from there every t-register is one behind (ugen's
 * round-robin: the latch's la spends one more t-register; the hand edit names $15 where ugen's
 * rotation would go on), real 39 of which all are that rotation. So the target is a colouring
 * one: uopt gives the hoisted address (a PRE temp defined in the preheader, ranges.txt 75,
 * blocks 20:010 21:100, save 5.0, r1 = v0) a caller-saved register and spills it at the call,
 * and the end (285, save 5.0) s0. Tried: the box loop with its own counter j: the dead "j = 0"
 * stays (move $4,$0), frame 0x68, real 67 (reverted). So the dead init is not from sharing i;
 * the game's loop has no counter init at all.
 * Kept: the box loop as a float pointer "for (p = ppressBoxOfs[0]; p != ppressBoxOfs[4];
 * p += 3)": real 44, length +3. The dead counter init is gone, but p is a variable with a home
 * (sw v1,84(sp)) where the game spills a uopt temp (60), the 120 takes s0 and the address is still
 * la'd into v0 and spilled at 56. So the game's loop variable is a strength-reduction temp (an
 * index loop) whose counter's init uopt deleted: open what deletes it.
 * A trace on the pointer form: the address is range 73, "ucvt{71}" (the far call's
 * address expression, not the cast: a probe alias declared ACollFrame * and called as (*f)(..)
 * with no cast gives the same ucvt range and the same code), live bbs 20:010 21:100 only, so
 * uopt sees it dead after the jal in block 21 (not around the back edge), gives it the lowest
 * free register (v0) and it is spilled at the call; the 120 (range 292, blocks 21/24) takes s0.
 * In the game the address's range is coloured s0, so it is live around the loop there: the
 * game's loop shape makes the hoisted address live across the call. Not found yet.
 * Correction from col.txt (same trace): range 73 did span the loop; uopt coloured it after the
 * 120 constant (292, save 5.0, assigned s0) and w (4, 4.89), found no callee-saved register
 * free, and split it ("73 split out 312": 312 uncoloured, the spill; 73's preheader piece v0).
 * So the order is by priority (globalcolor) and the address's whole-loop priority is under 4.89
 * (one use per pass spread over the loop's blocks, compute_save), while 120's (one use, two
 * blocks) is 5.0. In the game the address outranks both the 120 and the end (both uncoloured,
 * rebuilt each pass): its loop has fewer blocks for the address's range or more for 120's.
 * Prior art: tlb_0053's obj22Gear3Proc (matched) holds the address in s2, the 120 in s3, the
 * end in s4: there enough saved registers are free.
 * 2026-10-15, real 44 -> 2 (trace, uopt.c compute_save 151016-151262):
 * - adjsave = save / f(N), N = units + default blocks, f(N) = (N - 2) / 4 + 2 (integer) from
 *   N = 3. The address's range is the preheader (def) plus every loop block; 120's is the loop
 *   blocks only, its load moved out (canmoverlod). With `continue` the loop has 5 blocks (the
 *   continue's jump is one): address 10 / f(6) = 3.3 < 120's 10 / f(5) = 5.0, so 120 took s0.
 *   With "if (box != NULL) box->kind = 120;" it has 4: address 11 / f(5) = 5.5 > 5.0 and the
 *   address takes s0 (pointer form: 70, the end then built as base + 48).
 * - The index loop (the game's end, "la ppressBoxOfs+48", and its spill of uopt's pointer
 *   temp at 60) with i: the dead "i = 0" before the strength-reduced loop is part of i's range,
 *   which the clasher loop (i in s0, as the game) gives s0, so s0 is taken at the preheader and
 *   the address is split again (55). The box loop has its own counter j (dead init, no code):
 *   14, j declared last (data back at 80): 10; c (the clasher-hit loop's box pointer) also
 *   holds the new box, the frame's words as the game's: 5.
 * - The clasher test reads w->dmg->hitPower directly: "by" there joined the hit loop's by,
 *   whose tskSendMsg argument use made the range prefer a0 (the game: v0): 2.
 * Left (2): the falling case's "w->height + w->vel[1]": the game loads height first (f10) and
 *   vel[1] second (f16); here vel[1] is evaluated first whichever way the sum is written
 *   ("w->vel[1] + w->height" gives the same code: ugen reorders, build_tree's swap_tree,
 *   ugen.c 17139/17824). Not found.
 * 2026-10-15 (second try, 2), on the 2: uopt's IR is already height first. pri.txt: expression
 *   218 = uadd{193|0}{193|4}, where bucket 193 is "uilod w+..." in creation order, {193|0} the
 *   height load (first made in case 0) and {193|4} the vel[1] reload. That sum is the CSE that
 *   the else arm's store reuses. uopt's own commutative rule (readnxtinst L43f428, uopt.c
 *   ~84428) swaps the operands unless the first is an op node. So ugen reverses them:
 *   build_tree swaps the two top trees (ugen.c 17139, L413370) when the left one's entry in its
 *   per-tree array at sp+0x9c4 is nonzero and the right one's is zero. That fits both source
 *   orders giving vel[1] first here. In the game ugen kept height first and loaded it before the
 *   vel[1] store (game asm 310 before 316). Measured: "if (w->height + (w->vel[1] += -0.4) <= 0.0f)"
 *   2 (the same code: uopt rebuilds the statement order). Not yet found: what that array holds.
 * MATCHED 2026-10-15 (vendor/ugen-trace + probe):
 *   the swap was not build_tree's. ugen-trace printed every f_swap_tree call in ppressProc: four,
 *   all in f_translate's constant reassociation (ugen.c ~95982-96135), none on this sum. The
 *   operand order is uopt's (IR order, emitted as is). Probes with the same statement sequence:
 *   "w->height + w->vel[1]" with vel an f32[3] gives vel[1] first (a4, b7, this C). With vel[1]
 *   a scalar field at 0x2C (b6, or any struct field), height comes first, as in the game, whatever
 *   the other offsets (b0-b4). So the game's vel is no array: PpressWork's vel is a Vec3f and
 *   the falling case reads vel.y. MATCH (only ppressProc uses PpressWork). */
s32 ppressProc(Obj *o, s16 msg, void *arg) {
    PpressWork *w = (PpressWork *) FX(o);
    s32 i;
    ACollFrame *c;
    s32 data;
    Obj *by;
    s32 j;

    switch (msg) {
        case 2:
            o->name = "ppress";
            data = eneSetup(o, arg);
            w->pol = eneNewPol(o, ppressDl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayer3);
            if (w->zone == 0) {
                w->rot[1] = (data >> 16) * 3.141592741f / 512;
            } else {
                w->rot[1] = vramCourseHeading(w->zone, w->x) - 1.570796371f;
            }
            w->ride = rideCreate(gRideTree1, (s32) o, &w->pol->f, (RideData *) eneFileAddr(o, ppressRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->pol->f, 1, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            for (j = 0; j < 4; j++) {
                c = ((AtkAddXYZFunc) acollAtkAddXYZ)(&w->atk->frame, &w->pol->f, 50, ppressBoxOfs[j][0],
                                                      ppressBoxOfs[j][1], ppressBoxOfs[j][2], 0);
                if (c != NULL) {
                    c->kind = 120;
                }
            }
            w->dmg =((DmgAddXYZFunc) acollDmgAddXYZ)(ene_dmg_root_frame, &w->pol->f, 10, 10, 0, -120, 0, 0);
            w->top = (data >> 8) & 0xFF;
            w->base = w->y;
            switch (data & 0xFF) {
                case 0:
                    w->mode = 0;
                    break;
                case 1:
                    w->timer = 0;
                    w->mode = 1;
                    w->height = w->top;
                    break;
            }
            break;
        case 1:
            frmDestroy(&w->dmg->frame);
            frmDestroy(&w->ride->f);
            frmDestroy(&w->atk->frame);
            frmDestroy(&w->pol->f);
            break;
        case 0:
            w->timer++;
            switch (w->mode) {
                case 0:
                    w->height += 1.0f;
                    if (w->top < w->height) {
                        w->mode = 1;
                        w->timer = 0;
                    }
                    break;
                case 1:
                    if (w->timer >= 16) {
                        if (sndGetVolume(0x2CE, 0) == 0) {
                            soundIncamCam(0x2CE, w->pol->pos[0], w->pol->pos[1] - 30.0f, w->pol->pos[2]);
                        } else {
                            soundIncamCam(0x2CF, w->pol->pos[0], w->pol->pos[1] - 30.0f, w->pol->pos[2]);
                        }
                        w->mode = 2;
                    }
                    break;
                case 2:
                    w->vel.y += -0.4;
                    if (w->height + w->vel.y <= 0.0f) {
                        soundIncamCam(0x156, w->pol->pos[0], w->pol->pos[1], w->pol->pos[2]);
                        w->timer = 0;
                        w->mode = 3;
                        w->height = 0.0f;
                        w->vel.y = 0.0f;
                    } else {
                        w->height += w->vel.y;
                    }
                    break;
                case 3:
                    if (w->timer >= 31) {
                        w->mode = 0;
                    }
                    break;
            }
            w->y = w->base + w->height;
            for (c = (ACollFrame *) w->atk->frame.child; c != NULL; c = (ACollFrame *) c->frame.next) {
                if (acollAtkWasHit(c) == 1) {
                    by = (Obj *) c->hitPower;
                    if (by != NULL) {
                        tskSendMsg(by, 7, o);
                    }
                }
            }
            if (w->dmg != NULL && acollDmgWasHit(w->dmg) == 1) {
                if ((Obj *) w->dmg->hitPower != NULL && ((Obj *) w->dmg->hitPower)->proc == clasherProc) {
                    for (i = 0; i < 3; i++) {
                        clasherPieceMake(o, w->pol->pos[0], w->pol->pos[1], w->pol->pos[2], 1.0f, ppressPieceDl,
                                               110);
                    }
                    return -1;
                }
            }
            eneSetModelOnCourse(o, &w->pol->f);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
