#include "snd.h"
#include "audio.h"
#include "sndcmd.h"
#include "sndtrack.h"

/* Konami's sound driver, continued: each tick's track update, and each track's volume envelope
 * (attack, decay, sustain, release; include/snd.h). */

extern ALGlobals *alGlobals;          /* alGlobals */
extern ALVoice gSndVoices[SND_TRACKS]; /* a voice per track */
extern f32 gSndPitchTable[];               /* pitch ratio per semitone */
extern s16 gSndParamStepTable[3];              /* each kind's step toward its target */
extern u16 gSndTicks;                 /* ticks */
extern u8 gSndMusicPaused;                  /* music paused */
extern u16 gSndSpeedUp;                 /* a speed-up added to music's tempo */
extern u8 gSndSpeedUpTicks;                  /* ticks of it left (in 8s) */
extern SndFade gSndFades[2];          /* the crossfade's two sides */
extern u8 *gSndCmdPtr;                 /* the track's commands being read */
/* The command handlers (0xD0-0xFF) for music tracks and for the sound-effect tracks. */
extern void (*gSndMusicCmdTable[0x30])(void);
extern void (*gSndSeCmdTable[0x30])(void);




/* The driver's tick. Every 4 ticks the fades, level ramps (each stopping its sounds when it runs
 * out), the music slots' ramps and the two crossfades move on; every 8, a speed-up (gSndSpeedUp)
 * grows. Then each of the 18 tracks: a voice faded out is stopped, a delayed start counts down,
 * and the track is updated (music not while paused, gSndMusicPaused), with the crossfade's level given
 * to tracks 0-3 (the first) or 4 and up (the second). The fade levels are attenuations that
 * wrap: 0 is full, 0xB0 (-80) silent, so a fade stops when the level falls below 0xB1.
 * Matched 2026-10-03, no FAKE: the speed-up read as the symbol gSndSpeedUp, the
 * crossfade level tested as "(u32) level != 0" (as trkSetVolume's ramps), the step built in d
 * from the rate ("d = rate; d = level * d / 0x4000; d = rate * 3 - d;"), the level summed and
 * read back from the field; one named local (d), so the frame is the game's. check still shows
 * 4 lines there, "lui t6,0x800c; lhu t6,0x3398(t6)" in the .s against "%hi/%lo(gSndSpeedUp)":
 * splat did not pair that lui (scheduled above a bnezl) with its lhu, so the .s has literals;
 * the link makes the same bytes (3C0E800C 95CE3398). A written-out address "*(u16 *) 0x800C3398"
 * puts the load in a second ugen temp (lhu t7) and the track loop's t6-t9 rotation one step off.
 * History, from when it was NON_MATCHING (106 lines, mix distance 0: every instruction is the game's). Everything up to
 * the crossfade loop and the gSndSpeedUpTicks block match. Left, all in or after the crossfade loop:
 * (1) one stack word: the gSndSlotSounds pointer's spill sits at 0x2C, the game's at 0x30. Any
 * second named local (lv here, d at the top) takes a slot, block-scope ones too (measured: an
 * if-block or for-block local goes above the compiler temps), so the game has i, one word and
 * fade only; but the game's loop needs both a level copy (a0, "or a0,t0,zero" at the head of
 * the if's body) and the step's register (v0, divided in place), and no form measured gives
 * either without its own variable (below). (2) In the loop: 3 and &fade in t2/t1 (the game's
 * t1/t2), the level and &fade[i] in a2/t0 (the game's t0/a2), and ugen's t6-t9 rotation one
 * step off there and in the track loop: the FAKE boundary below makes IDO compute &fade[i] at
 * its place, where the game computes it at the "fade[i] =" store. History: 2026-10-02 (428
 * to 397): the crossfade's step computes level * rate / 0x4000 first in the game, so it is
 * written "level -= level * rate / 0x4000 - rate * 3"; IDO still sums it as (level - q) + 3r
 * where the game adds level to (3r - q). The permuter's two empty ifs (FAKE) give 254; its
 * earlier no-op mask on gSndMusicRamp's level (373) is dropped, as it undoes them (446).
 * The game's divide by 0x4000 is the variable form (the product in v0, "bgez; addu at,v0,zero"
 * and one sra; the level copied into a0, the rate in v1 all through); this C's product is a
 * temporary (the sra repeated in the delay slot). u16 level / s16 rate locals for the
 * crossfade: 407 (the same divide); with the product in an s32 q as well: 415.
 * "level += rate * 3 - level * rate / 0x4000" (the game's level + (3r - q)): 393, the loop's
 * constants hoisted and the loop reshaped (2026-10-03). The product alone in an s32 q (in the
 * pad's slot, the fields read straight): 352. The greedy mode (2026-10-03, 254 to 250): a FAKE
 * block boundary. 2026-10-03: the game's one-sra divide is the in-place form (its
 * product's register takes the quotient: "addu at,v0,zero; ...; sra v0,at,14"); the product
 * in q divided in place, "q = level * rate; q /= 0x4000; level -= q - rate * 3": 374.
 * The game copies the level (or a0,t0: a plain move, so not a u16 local) and multiplies the
 * loaded t0, then adds a0 to (3r - q). A u16 level local in the pad's slot, read once and used
 * for the stores and compares: 408 (an andi of the copy; summed (level + 3r) - q). An s32 copy
 * added to the fields' step, "field = level + (3r - field * rate / 0x4000)": 384 (the copy
 * propagated away, 3r computed before the product). 2026-10-03 (from 250): both
 * "gSndAllFadeMusicStep = 0" stores through the written-out address "*(s8 *) 0x800C336E": 534 (an
 * indirect store kills every global CSE after it; the original used the symbol). The step in
 * an s32 d (in the pad's slot), "d = rate * level / 0x4000; d = rate * 3 - d; level += d":
 * 338 lines but mix distance 6 (from 8): the loop's arithmetic is now the game's, instruction
 * for instruction (the in-place divide, "multu level,rate", level + (3r - q)); what is left
 * there is the game's level copy (or a0,t0) and its "addiu t2,sp,0x34" hoisted out of the loop,
 * two more registers: with every caller-saved register taken in the loop the game must keep
 * gSndMusicPaused's value in s0, where this C has t0 free for it. Kept as the base (the game's
 * structure). "gSndAllFadeMusicLevel = gSndAllFadeMusicStep = 0;" (audio.c's chain): 441. With an s32
 * level copy lv in the if's block ("lv = level; d = ..; lv += d; level = lv;") and the three
 * splits-sweep FAKEs taken out: 292, and the code up to the gSndSlotSounds loop is the game's
 * (gSndAllFadeMusicStep by lui each time, gSndMusicPaused's value in s0): the old FAKE "if (gSndSpeedUpTicks)
 * {}" in the loop was what kept "addiu t2,sp,0x34" from being hoisted. Left: the loop index in
 * t0 (the game's a3), the gSndSlotSounds pointer's spill at 0x2C (0x30), and the fade loop's level
 * (t0), copy (a0) and rate (v1) registers. The splits sweep on that: a FAKE block boundary
 * after "if (rate < 0) {" gives 246 (the best so far; i in a3 and the gSndSlotSounds loop the
 * game's). Left: in the fade loop the game copies the level (t0) into a0 ("or a0,t0,zero")
 * and adds into a ugen temp (t6) for the store; here the copy is propagated away (the level in
 * a0) and lv += d is a register of its own (a2), so &fade[i] takes t0 (the game's a2). Then
 * the gSndSpeedUpTicks block onward differs only by the registers that follow from it.
 * 2026-10-03 (scratch compiles): "D.level = lv + d; lv = D.level;" (the read-back in lv,
 * the compares on lv) puts the sum in a ugen temp as the game does. uopt propagates a copy
 * only inside one block: the game's "or a0,t0,zero" needs a block boundary between
 * "lv = level" and its use; a "do {} while (0)" there loses the hoisted &fade, a FAKE
 * "if (!fade) {}" (a use of &fade, no code) keeps it. gSndSpeedUp's address must not be held in
 * a register (the game reaches it by lui both times): written "gSndSpeedUp =
 * gSndSlotSounds[4] + 1" (the same address, audio.c defines them in a row) each name is used
 * once. Together: 106, mix distance 0, the gSndSpeedUpTicks block the game's. Left: the constants 3
 * and &fade (t1/t2) swapped, the level and &fade[i] in a2/t0 (the game's t0/a2), and ugen's t6-t9
 * rotation one step off in the fade loop and the track loop (the game computes &fade[i] before
 * the shifted level). Measured from 106 (scratch, registers and opcodes only; check lines in
 * brackets): the FAKE after the two d lines instead puts 3/&fade and the temps right but
 * permutes the loop's variables (worse); "do {} while (0)" plus "if (!fade) {}" after the store
 * (worse); "lv = level; if (lv != 0) {" with lv in the for's block (no FAKE: every loop
 * register the game's, but the copy lands in block A, beqz's delay slot, one instruction short:
 * [159]); the same with "if ((lv = ..) != 0)", "if (lv == 0) continue;", "d + lv": the copy
 * stays in A; lv at the top with d in the if's block, "register s32 lv": the spill stays at
 * 0x2C; no d (lv the one extra word: the spill at 0x30, but 3r is computed first and the sum
 * regrouped, much worse); no lv (d the one word, a boundary before the store): no copy, worse.
 * The splits sweep from 106 finds nothing lower.
 * 2026-10-03 (the crossfade loop's pairs): the track loop's "unk1A += speed-up" read
 * as the symbol, "gSndSpeedUp", not "*(u16 *) 0x800C3398": 106 to 42, mix 0. ugen loads a
 * symbol as "lui t6; lhu t6,lo(t6)" (one register) but a written-out address into a second
 * temp (lhu t7,0x3398(t6)), which put the whole track loop's t6-t9 rotation one step off.
 * Measured (moves, commutes, compounds, rechains on 106: nothing; scratch compiles of the read
 * as "(0x800C0000 + 0x3398)", "[0x3398 / 2]", "0x800C3398U", "(s32)", volatile, "* 1": all the
 * second temp). The fade loop, measured in scratch compiles: the FAKE "if (!fade) {}" is what
 * swaps 3 and &fade (its extra use of &fade ranks it over 3), and any block boundary inside the
 * if's body hoists &fade[i] to the end of the body's first block (here before 3r, so t9 for its
 * sll; the game computes it after the sum: its body has no boundary before the store).
 * "lv = level * 1" (FAKE, folded late, so uopt keeps lv a copy with no boundary) instead of the
 * boundary: the copy (or a0), 3/&fade (t1/t2) and the t6-t9 temps all the game's, but the level
 * CSE ranks above i and &fade[i] (level a2, &fade[i] a3, i t0 in both loops): 118 (54 with the
 * symbol read), so not kept. Its empties sweep: nothing. Two boundaries (after the copy and
 * after the store), either FAKE kind: &fade[i] still hoisted after the copy. A second store
 * "level = 3r - d; level = lv + level" (a kill instead of a boundary): both stores kept. One
 * named local only (d; the fields read again after the store; the spill at 0x30 as the game's):
 * the temps and &fade[i] a2, i a3 the game's, but no copy (the "* 1" inside "level * 1 + d"
 * folds early). The level CSE's rank fell below i and &fade[i] (t0, as the game's) once the
 * product reads lv instead of the field: "lv = level * 1; d = lv * rate / 0x4000": 10 (uopt
 * substitutes "level * 1" into the product, so the multu still reads t0, and the level CSE
 * loses a use). Left: the spill at 0x2C (lv's block slot; the game's 0x30), "multu v1,t0" where
 * the game has "multu t0,v1", and the 4 relocation lines. Scratch: "rate * lv" also gives v1,t0.
 * "d = rate; d = lv * d / 0x4000": multu t0,v1, 8 (the spill and the relocation lines). One
 * named local: with only d, fields read throughout and "(u32) level + d" in the sum, the frame
 * is the game's but the copy is missing; the (u32) on the TEST instead ("(u32) level != 0", the
 * sum "level + d", the reads on the field) gives the copy (or a0,t0): 4, the relocation lines
 * only. The (u32) on the multiply, on test and multiply, or on test and sum: worse.
 * "gSndSpeedUp++" for the old FAKE "gSndSpeedUp = gSndSlotSounds[4] + 1": still 4, so
 * that FAKE is gone. This form reached the same 4 by another road: every level read in the if's
 * body written "level * 1" (FAKE), so a1's form (no FAKE) is the one kept. */
void trkDriverTick(void) {
    s32 i;
    s32 d;
    u16 fade[2];

    if ((gSndTicks & 3) == 0) {
        if (gSndAllFadeMusicStep != 0) {
            gSndAllFadeMusicLevel += gSndAllFadeMusicStep;
            if (gSndAllFadeMusicLevel < 0xB1) {
                if (gSndAllFadeMusicStep < 0) {
                    gSndAllFadeMusicLevel = 0xB0;
                } else {
                    gSndAllFadeMusicLevel = 0;
                }
                gSndAllFadeMusicStep = 0;
            }
        }
        if (gSndAllFadeSeStep != 0) {
            gSndAllFadeSeLevel += gSndAllFadeSeStep;
            if (gSndAllFadeSeLevel < 0x11) {
                if (gSndAllFadeSeStep < 0) {
                    gSndAllFadeSeLevel = 0x10;
                } else {
                    gSndAllFadeSeLevel = 0;
                }
                gSndAllFadeSeStep = 0;
            }
        }
    }
    if (gSndAllRamp.level != 0 && gSndMusicPaused == 0 && (gSndTicks & 3) == 0) {
        gSndAllRamp.level -= gSndAllRamp.rate;
        if (gSndAllRamp.level < 0x1000) {
            sndStopAll();
        }
    }
    if (gSndMusicRamp.level != 0 && gSndMusicPaused == 0 && (gSndTicks & 3) == 0) {
        gSndMusicRamp.level -= gSndMusicRamp.rate;
        if (gSndMusicRamp.level < 0x1000) {
            sndStopMusic();
            gSndAllFadeMusicStep = 0;
            gSndAllFadeMusicLevel = 0;
        }
    }
    if (gSndMusicPaused == 0 && (gSndTicks & 3) == 0) {
        for (i = 0; i < 4; i++) {
            if (gSndSlotSounds[i] != 0) {
                gSndSlotRamps[i].level -= gSndSlotRamps[i].rate;
                if (gSndSlotRamps[i].level < 0x1000) {
                    sndFreeSlot(i);
                }
            }
        }
    }
    fade[1] = 0;
    fade[0] = 0;
    if (gSndMusicPaused == 0 && (gSndTicks & 3) == 0) {
        for (i = 0; i < 2; i++) {
            if ((u32) gSndFades[i].level != 0) {
                d = gSndFades[i].rate;
                d = gSndFades[i].level * d / 0x4000;
                d = gSndFades[i].rate * 3 - d;
                gSndFades[i].level = gSndFades[i].level + d;
                fade[i] = gSndFades[i].level >> 7;
                if (gSndFades[i].rate < 0) {
                    if (gSndFades[i].level < 0x800) {
                        gSndFades[i].level = 0;
                        fade[i] = 1;
                    }
                } else if (gSndFades[i].level >= 0x8000) {
                    gSndFades[i].level = 0;
                    fade[i] = 0xFF;
                }
            }
        }
    }
    if (gSndSpeedUpTicks != 0 && gSndMusicPaused == 0 && (gSndTicks & 7) == 0) {
        gSndSpeedUpTicks--;
        gSndSpeedUp++;
    }
    if (gSndMusicFadeStep != 0 && (gSndTicks & 3) == 0) {
        gSndMusicFadeLevel += gSndMusicFadeStep;
        if (gSndMusicFadeLevel < 0xB1) {
            if (gSndMusicFadeStep < 0) {
                gSndMusicFadeLevel = 0xB0;
            } else {
                gSndMusicFadeLevel = 0;
            }
            gSndMusicFadeStep = 0;
        }
    }
    for (gSndCurTrackIndex = 0; gSndCurTrackIndex < 18; gSndCurTrackIndex++) {
        gSndCurTrack = &gSndTracks[gSndCurTrackIndex];
        if (gSndCurTrack->voiceStopTicks != 0) {
            gSndCurTrack->voiceStopTicks--;
            if (gSndCurTrack->voiceStopTicks == 0 && gSndCurTrack->voiceOn != 0) {
                gSndCurTrack->voiceOn = 0;
                alSynStopVoice(&alGlobals->drvr, &gSndVoices[gSndCurTrackIndex]);
            }
        }
        if (gSndCurTrack->data == NULL) {
            gSndCurTrack->id = 0;
        }
        if (gSndCurTrack->id != 0) {
            if (gSndCurTrack->startDelay != 0) {
                gSndCurTrack->startDelay--;
            } else if ((gSndCurTrack->id & 0x7FFF) < 0x100) {
                if (gSndMusicPaused == 0) {
                    gSndCurTrack->tempoAcc += gSndSpeedUp;
                    if (fade[0] != 0 && gSndCurTrack->noCrossfade == 0 && gSndCurTrackIndex < 4) {
                        gSndCurTrack->flags |= 1;
                        if (fade[0] == 0xFF) {
                            gSndCurTrack->fadeLevel = 0;
                        } else {
                            gSndCurTrack->fadeLevel = fade[0];
                        }
                    }
                    if (fade[1] != 0 && gSndCurTrack->noCrossfade == 0 && gSndCurTrackIndex >= 4) {
                        gSndCurTrack->flags |= 1;
                        if (fade[1] == 0xFF) {
                            gSndCurTrack->fadeLevel = 0;
                        } else {
                            gSndCurTrack->fadeLevel = fade[1];
                        }
                    }
                    trkUpdate();
                }
            } else {
                trkUpdate();
            }
        }
    }
}

/* A track's update each tick. Every 4 ticks each kind's value steps toward its target. The tempo
 * adds to an accumulator; on each whole step the scale (unkBE) decays (below 16 the track stops),
 * the slides advance, and when the note's wait runs out the next commands are read. */
void trkUpdate(void) {
    s32 k;
    s16 d;

    if ((gSndTicks & 3) == 0) {
        for (k = 0; k < 3; k++) {
            if (gSndCurTrack->param[k] != gSndCurTrack->param0[k]) {
                if (gSndCurTrack->param0[k] < gSndCurTrack->param[k]) {
                    gSndCurTrack->param0[k] += gSndParamStepTable[k];
                    if (gSndCurTrack->param[k] < gSndCurTrack->param0[k]) {
                        gSndCurTrack->param0[k] = gSndCurTrack->param[k];
                    }
                } else {
                    gSndCurTrack->param0[k] -= gSndParamStepTable[k];
                    if (gSndCurTrack->param0[k] < gSndCurTrack->param[k]) {
                        gSndCurTrack->param0[k] = gSndCurTrack->param[k];
                    }
                }
                switch (k) {
                    case 0:
                        gSndCurTrack->flags |= 2;
                        break;
                    case 1:
                        gSndCurTrack->flags |= 1;
                        break;
                    case 2:
                        sndSetPan(gSndCurTrackIndex);
                        break;
                }
            }
        }
    }
    gSndCurTrack->tempoAcc += gSndCurTrack->tempo >> 8;
    if (gSndCurTrack->tempoAcc >= 0x100) {
        gSndCurTrack->tempoAcc &= 0xFF;
        if (gSndCurTrack->stopFadeLevel != 0) {
            d = gSndCurTrack->stopFadeLevel;
            d -= gSndCurTrack->stopFadeStep;
            if (d < 0x10) {
                sndCmdEnd();
                return;
            }
            gSndCurTrack->stopFadeLevel = d;
            gSndCurTrack->flags |= 1;
        }
        for (k = 0; k < 3; k++) {
            if (gSndCurTrack->slideTicks[k] != 0) {
                gSndCurTrack->slideTicks[k]--;
                if (gSndCurTrack->slideTicks[k] != 0) {
                    gSndCurTrack->slide[k] += gSndCurTrack->slideStep[k];
                } else {
                    gSndCurTrack->slide[k] = gSndCurTrack->slideTarget[k];
                }
                switch (k) {
                    case 0:
                        gSndCurTrack->flags |= 2;
                        break;
                    case 1:
                        gSndCurTrack->flags |= 1;
                        break;
                    case 2:
                        sndSetPan(gSndCurTrackIndex);
                        break;
                }
            }
        }
        gSndCurTrack->waitTicks--;
        if (gSndCurTrack->waitTicks == 0) {
            trkNextNote();
            sndCmdPortamentoCheck();
            if (gSndCurTrackIndex >= SND_TRACKS) {
                return;
            }
        } else {
            if (gSndCurTrackIndex >= SND_TRACKS) {
                return;
            }
            trkMusicTick();
        }
        if (gSndCurTrack->tempoSlideTicks != 0) {
            sndCmdTempoSlideTick();
        }
        if (gSndCurTrack->volSlideTicks != 0) {
            sndCmdVolumeSlideTick();
        }
        if (gSndCurTrack->panSlideTicks != 0) {
            sndCmdPanSlideTick();
        }
    } else {
        if (gSndCurTrackIndex >= SND_TRACKS) {
            return;
        }
        trkSeTick();
    }
    if (gSndCurTrack->wobbleSpeed != 0) {
        sndCmdWobbleTick();
    }
}

/* A track's next note. Commands 0xD0 and up go to the handlers (music's or sound effects') until
 * a note; 0xF2/0xF3 end the step, 0xFF the track. A note is a key byte (0x00-0x67 with a length
 * byte after, 0x68-0xCF reusing the last length), an optional gate (< 0x80) and a velocity (its
 * low 7 bits). Keys 0x48 and up are drums: 0x67 takes the key from the next byte, then the drum
 * is set up and played at key 0x3C. The note is the key plus five offsets, in 1/256 semitones.
 * A music track then restarts its voice, unless the last note's gate was 0 (a tie), which only
 * updates the envelope, pitch and volume. */
void trkNextNote(void) {
    u8 cmd;
    u8 tie;

    tie = FALSE;
    if (gSndCurTrack->gate == 0) {
        tie = TRUE;
    } else if (gSndCurTrack->voiceOn != 0) {
        gSndCurTrack->voiceStopTicks = 0;
        gSndCurTrack->voiceOn = 0;
        alSynStopVoice(&alGlobals->drvr, &gSndVoices[gSndCurTrackIndex]);
    }
    gSndCmdPtr = gSndCurTrack->data;
    cmd = *gSndCmdPtr++;
    while (cmd >= 0xD0) {
        if (gSndCurTrackIndex >= 16) {
            gSndSeCmdTable[cmd - 0xD0]();
        } else {
            gSndMusicCmdTable[cmd - 0xD0]();
        }
        switch (cmd) {
            case 0xF2:
            case 0xF3:
                gSndCurTrack->data = gSndCmdPtr;
                return;
            case 0xFF:
                return;
        }
        cmd = *gSndCmdPtr++;
    }
    gSndCurTrack->portaStep = gSndCurTrack->note;
    if (cmd < 0x68) {
        gSndCurTrack->key = cmd;
    } else {
        gSndCurTrack->key = cmd - 0x68;
    }
    if (gSndCurTrack->key >= 0x48) {
        if (gSndCurTrack->key == 0x67) {
            gSndCurTrack->key = *gSndCmdPtr;
            gSndCmdPtr++;
        }
        gSndCurTrack->flags |= 4;
        sndCmdDrumInstrument();
        gSndCurTrack->key = 0x3C;
    } else {
        gSndCurTrack->flags &= 0xFB;
    }
    gSndCurTrack->note = gSndCurTrack->key << 8;
    gSndCurTrack->note += gSndCurTrack->instTune;
    gSndCurTrack->note += gSndCurTrack->transpose;
    gSndCurTrack->note += gSndCurTrack->detune;
    gSndCurTrack->note += gSndCurTrack->loopDetune;
    gSndCurTrack->note += gSndCurTrack->loop2Detune;
    if (cmd < 0x68) {
        gSndCurTrack->lastLength = *gSndCmdPtr;
        gSndCmdPtr++;
    }
    gSndCurTrack->waitTicks = gSndCurTrack->lastLength;
    cmd = *gSndCmdPtr++;
    if (cmd < 0x80) {
        gSndCurTrack->lastGate = cmd;
        cmd = *gSndCmdPtr++;
    }
    gSndCurTrack->gate = gSndCurTrack->lastGate;
    if (gSndCurTrack->gate == 0) {
        gSndCurTrack->noteTicks = 0;
    } else {
        gSndCurTrack->noteTicks = (gSndCurTrack->waitTicks * gSndCurTrack->gate) >> 7;
        if (gSndCurTrack->noteTicks == 0) {
            gSndCurTrack->noteTicks = 1;
        }
    }
    gSndCurTrack->velocity = cmd & 0x7F;
    gSndCurTrack->data = gSndCmdPtr;
    if (gSndCurTrackIndex < 16) {
        if (gSndCurTrack->bendAmount == 0) {
            gSndCurTrack->portaTicks = 0;
        } else {
            sndCmdPitchBendNote();
        }
        gSndCurTrack->vibratoOffset = 0;
        if (gSndCurTrack->vibratoDepth != 0) {
            sndCmdVibratoReset();
        }
        if (gSndCurTrack->stateMuted == 0) {
            trkSetPitch();
            if (!tie) {
                gSndCurTrack->voiceVolTicks = 0xFFFF;
                trkEnvAttack();
                gSndCurTrack->voiceOn = 1;
                alSynStartVoice(&alGlobals->drvr, &gSndVoices[gSndCurTrackIndex],
                                   gSndCurTrack->wave);
            } else {
                gSndCurTrack->flags |= 1;
                if (gSndCurTrack->envTicks != 0) {
                    trkEnvTick();
                }
                if (gSndCurTrack->flags & 1) {
                    trkSetVolume();
                }
            }
        }
    }
}

/* A music track's tick: its effects, its pitch if changed, its note's length (at the end: the
 * release), its envelope, and its volume if changed or if a fade touches it. */
void trkMusicTick(void) {
    if (gSndCurTrack->portaTicks != 0) {
        sndCmdPortamentoTick();
    }
    if (gSndCurTrack->vibratoDepth != 0) {
        sndCmdVibratoTick();
    }
    if (gSndCurTrack->flags & 2) {
        trkSetPitch();
    }
    if (gSndCurTrack->gate != 0 && gSndCurTrack->waitTicks == 1) {
        trkEnvReleaseNow();
        return;
    }
    if (gSndCurTrack->noteTicks != 0) {
        gSndCurTrack->noteTicks--;
        if (gSndCurTrack->noteTicks == 0) {
            trkEnvRelease();
            return;
        }
    }
    if (gSndCurTrack->envTicks != 0) {
        trkEnvTick();
    }
    if (gSndCurTrack->flags & 1) {
        trkSetVolume();
    } else {
        trkCheckFadeVol();
    }
}

/* Recompute the current track's volume if a fade, ramp or slot level applies to it. */
void trkCheckFadeVol(void) {
    u32 id = gSndCurTrack->id & 0x7FFF;
    s32 i;

    if (id < 0x100) {
        if (gSndAllFadeMusicStep != 0) {
            trkSetVolume();
            return;
        }
    } else if (gSndAllFadeSeStep != 0) {
        trkSetVolume();
        return;
    }
    if (gSndAllRamp.level != 0) {
        trkSetVolume();
        return;
    }
    if (id < 0x100 && (gSndMusicRamp.level != 0 || gSndMusicFadeStep != 0)) {
        trkSetVolume();
        return;
    }
    for (i = 0; i < 4; i++) {
        if (id == gSndSlotSounds[i]) {
            trkSetVolume();
            return;
        }
    }
}

/* A sound effect track's tick: as trkMusicTick, without the note length. */
void trkSeTick(void) {
    if (gSndCurTrack->portaTicks != 0) {
        sndCmdPortamentoTickSe();
    }
    if (gSndCurTrack->vibratoDepth != 0) {
        sndCmdVibratoStep();
    }
    if (gSndCurTrack->flags & 2) {
        trkSetPitch();
    }
    if (gSndCurTrack->envTicks != 0) {
        trkEnvTick();
    }
    if (gSndCurTrack->flags & 1) {
        trkSetVolume();
    }
}

/* Set the current track's pitch: its note plus its offsets and pitch slide (in 1/256 semitones,
 * 0-0x4800), interpolated in the semitone table. */
void trkSetPitch(void) {
    s32 note;
    u32 frac;
    f32 pitch;

    gSndCurTrack->flags &= 0xFD;
    note = gSndCurTrack->param0[0] + gSndCurTrack->note + gSndCurTrack->wobbleOffset + gSndCurTrack->vibratoOffset;
    note += gSndCurTrack->slide[0];
    if (note < 0) {
        note = 0;
    } else if (note > 0x4800) {
        note = 0x4800;
    }
    frac = note & 0xFF;
    note >>= 8;
    pitch = gSndPitchTable[note + 1] - gSndPitchTable[note];
    pitch = frac * pitch / 256;
    pitch += gSndPitchTable[note];
    if (pitch != gSndCurTrack->pitch) {
        gSndCurTrack->pitch = pitch;
        alSynSetPitch(&alGlobals->drvr, &gSndVoices[gSndCurTrackIndex], pitch);
    }
}

/* Set the current track's volume: its velocity, envelope, kind-1 value and slide, and every fade,
 * ramp and slot that applies, each also shortening the time the voice takes to get there.
 * (The two ramps' levels are tested as u32: that keeps each ramp's address in a3 and its level
 * in v1, as the game does.) */
void trkSetVolume(void) {
    s32 vol;
    s32 ticks;
    s16 s;
    u32 id;
    s32 i;

    gSndCurTrack->flags &= 0xFE;
    vol = gSndCurTrack->velocity + gSndCurTrack->loopVelocity + gSndCurTrack->loop2Velocity;
    vol *= gSndCurTrack->volume;
    if (vol >= 0x800000) {
        vol = 0x7FFF;
    } else if (vol < 0) {
        vol = 0;
    } else {
        vol >>= 8;
    }
    ticks = gSndCurTrack->envTicks;
    vol = (gSndCurTrack->envLevel * vol) >> 8;
    if (gSndCurTrack->param0[1] != 0) {
        vol = ((gSndCurTrack->param0[1] + 0x100) * vol) >> 8;
        if (vol >= 0x8000) {
            vol = 0x7FFF;
        }
    }
    if (gSndCurTrack->slide[1] != 0) {
        s = gSndCurTrack->slide[1] / 64;
        if (s < -0xFF) {
            vol = 0;
        } else {
            vol = ((s + 0x100) * vol) >> 8;
            if (vol >= 0x8000) {
                vol = 0x7FFF;
            }
        }
    }
    if (gSndCurTrack->stopFadeLevel != 0) {
        vol = (gSndCurTrack->stopFadeLevel * vol) >> 8;
        ticks = trkScaleTicks(ticks, gSndCurTrack->stopFadeLevel, 2);
    }
    if ((u32) gSndAllRamp.level != 0 && (gSndCurTrack->id & 0x7FFF) != 0x45F) {
        vol = (gSndAllRamp.level * vol) >> 16;
        ticks = trkScaleTicks(ticks, gSndAllRamp.level >> 8, 4 - (gSndAllRamp.rate >> 8));
    }
    if ((gSndCurTrack->id & 0x7FFF) < 0x100) {
        if ((u32) gSndMusicRamp.level != 0) {
            vol = (gSndMusicRamp.level * vol) >> 16;
            ticks = trkScaleTicks(ticks, gSndMusicRamp.level >> 8, 4 - (gSndMusicRamp.rate >> 8));
        }
        if (gSndMusicFadeLevel != 0) {
            vol = (gSndMusicFadeLevel * vol) >> 8;
            if (gSndMusicFadeStep < 0) {
                ticks = trkScaleTicks(ticks, gSndMusicFadeLevel, 2);
            }
        }
    }
    id = gSndCurTrack->id & 0x7FFF;
    if (id < 0x100) {
        if (gSndAllFadeMusicLevel != 0) {
            vol = (gSndAllFadeMusicLevel * vol) >> 8;
            if (gSndAllFadeMusicStep < 0) {
                ticks = trkScaleTicks(ticks, gSndAllFadeMusicLevel, 1);
                id = gSndCurTrack->id & 0x7FFF;
            }
        }
    } else if (gSndAllFadeSeLevel != 0 && id != 0x203 && id != 0x205) {
        vol = (gSndAllFadeSeLevel * vol) >> 8;
        if (gSndAllFadeSeStep < 0) {
            ticks = trkScaleTicks(ticks, gSndAllFadeSeLevel, 1);
            id = gSndCurTrack->id & 0x7FFF;
        }
    }
    for (i = 0; i < 4; i++) {
        if (id == gSndSlotSounds[i]) {
            vol = (gSndSlotRamps[i].level * vol) >> 16;
            ticks = trkScaleTicks(ticks, gSndSlotRamps[i].level, 4 - (gSndSlotRamps[i].rate >> 8));
            break;
        }
    }
    if (gSndCurTrack->fadeLevel != 0) {
        vol = (gSndCurTrack->fadeLevel * vol) >> 8;
    }
    if (gSndCurTrack->voiceVol != (s16) vol || gSndCurTrack->voiceVolTicks != (u16) ticks) {
        gSndCurTrack->voiceVol = vol;
        gSndCurTrack->voiceVolTicks = ticks;
        alSynSetVol(&alGlobals->drvr, &gSndVoices[gSndCurTrackIndex], vol, ticks * 5000);
    }
}

/* Scale v by the current track's envelope, once it is past its decay: v * scale / 256 below
 * 0x200, else scale * x * 2 (x at least 1). One `return v` for every path: IDO turns the
 * first path's branch to it into its own `jr ra` and fills the last one's slot with the move. */
s32 trkScaleTicks(s32 v, u8 scale, s8 x) {
    if (gSndCurTrack->envState >= 2) {
        if (v < 0x200) {
            v = scale * v;
            v = v >> 8;
        } else {
            if (x <= 0) {
                x = 1;
            }
            v = (scale * x) << 1;
        }
    }
    return v;
}

/* Attack: from 0x20 (if the envelope has an attack) up to 0x100 over its ticks. */
void trkEnvAttack(void) {
    gSndCurTrack->envTicks = 0;
    if (gSndCurTrack->envAttack != 0) {
        gSndCurTrack->envLevel = 0x20;
        trkSetVolume();
        gSndCurTrack->envState = 0;
        gSndCurTrack->envLevel = 0x100;
        gSndCurTrack->envTicks = gSndCurTrack->envAttack;
        trkSetVolume();
    } else {
        gSndCurTrack->envLevel = 0x100;
        trkSetVolume();
        trkEnvDecay();
    }
}

/* Decay: down by its level, over its ticks. */
void trkEnvDecay(void) {
    if (gSndCurTrack->envDecayLevel != 0) {
        gSndCurTrack->envState = 1;
        gSndCurTrack->envLevel -= gSndCurTrack->envDecayLevel;
        gSndCurTrack->envTicks = gSndCurTrack->envDecay;
        trkSetVolume();
    } else {
        trkEnvSustain();
    }
}

/* Sustain: down to 0 over its ticks, if it has any. */
void trkEnvSustain(void) {
    gSndCurTrack->envState = 2;
    if (gSndCurTrack->envSustain != 0) {
        gSndCurTrack->envLevel = 0;
        gSndCurTrack->envTicks = gSndCurTrack->envSustain;
        trkSetVolume();
    }
}

/* Release: down to 0 over its ticks. */
void trkEnvRelease(void) {
    gSndCurTrack->envState = 3;
    gSndCurTrack->envLevel = 0;
    gSndCurTrack->envTicks = gSndCurTrack->envRelease;
    trkSetVolume();
}

/* Release at once. */
void trkEnvReleaseNow(void) {
    gSndCurTrack->voiceStopTicks = 1;
    gSndCurTrack->envState = 3;
    gSndCurTrack->envLevel = 0;
    gSndCurTrack->envTicks = 1;
    trkSetVolume();
}

/* A tick of the envelope: at the end of the attack, decay; at the end of the decay, sustain. */
void trkEnvTick(void) {
    gSndCurTrack->envTicks--;
    if (gSndCurTrack->voiceVolTicks != 0) {
        gSndCurTrack->voiceVolTicks--;
    }
    if (gSndCurTrack->envTicks == 0) {
        switch (gSndCurTrack->envState) {
            case 0:
                trkEnvDecay();
                break;
            case 1:
                trkEnvSustain();
                break;
        }
    }
}

/* This file's .data (subsegment 0x25140), in ROM order. */
void (*gSndMusicCmdTable[0x30])(void) = {
    sndCmdTempo, sndCmdTempoSlide, sndCmdInstrument, sndCmdVolInstrument,
    sndCmdVolInstrumentPan, sndCmdVolume, sndCmdVolumeSlide, sndCmdEnvAttackDecay,
    sndCmdEnvSustain, sndCmdEnvRelease, sndCmdFxParams, sndCmdEnd,
    sndCmdFadeHold, sndCmdPan, sndCmdPanSlide, sndCmdTranspose,
    sndCmdDetune, sndCmdVibrato, sndCmdVibratoRise, sndCmdWobble,
    sndCmdPortamentoSkip, sndCmdPitchBend, sndCmdGlide, sndCmdLoopStart,
    sndCmdLoopEnd, sndCmdLoop2Start, sndCmdLoop2End, sndCmdMark,
    sndCmdGotoMark, sndCmdRepeatStart, sndCmdRepeat, sndCmdFxMix,
    sndCmdBank, sndCmdStateMute, sndCmdRest, sndCmdTie,
    sndCmdEnd, sndCmdEnd, sndCmdEnd, sndCmdEnd,
    sndCmdScaleBase, sndCmdRandomVariation, sndCmdEvent, sndCmdEventArg,
    sndCmdEventKind2, sndCmdEventKind3, sndCmdEventKind4, sndCmdEnd,
};
void (*gSndSeCmdTable[0x30])(void) = {
    sndCmdTempo, sndCmdTempoSlide, sndCmdSkip1, sndCmdSkip2,
    sndCmdSkip3, sndCmdSkip1, sndCmdSkip2, sndCmdSkip3,
    sndCmdSkip1, sndCmdSkip1, sndCmdSkip2, sndCmdSkip2,
    sndCmdFadeHold, sndCmdSkip1, sndCmdSkip2, sndCmdSkip1,
    sndCmdSkip1, sndCmdSkip3, sndCmdSkip1, sndCmdSkip3,
    sndCmdSkip3, sndCmdSkip3, sndCmdSkip1, sndCmdLoopStart,
    sndCmdLoopEnd, sndCmdLoop2Start, sndCmdLoop2End, sndCmdMark,
    sndCmdGotoMark, sndCmdRepeatStart, sndCmdRepeat, sndCmdSkip1,
    sndCmdSkip1, sndCmdStateMute, sndCmdRest, sndCmdTie,
    sndCmdEnd, sndCmdEnd, sndCmdEnd, sndCmdEnd,
    sndCmdScaleBase, sndCmdRandomVariation, sndCmdEvent, sndCmdEventArg,
    sndCmdEventKind2, sndCmdEventKind3, sndCmdEventKind4, sndCmdEnd,
};
s16 gSndParamStepTable[3] = { 10, 50, 50 };
f32 gSndPitchTable[74] = {
    0.03125f, 0.0331082195f, 0.0350769386f, 0.0371627174f,
    0.0393725298f, 0.0417137183f, 0.0441941544f, 0.0468220636f,
    0.0496062487f, 0.0525559671f, 0.0556811243f, 0.0589920953f,
    0.0625f, 0.066216439f, 0.0701538771f, 0.0743254349f,
    0.0787450597f, 0.0834274366f, 0.0883883089f, 0.0936441272f,
    0.0992124975f, 0.105111934f, 0.111362249f, 0.117984191f,
    0.125f, 0.132432878f, 0.140307754f, 0.14865087f,
    0.157490119f, 0.166854873f, 0.176776618f, 0.187288254f,
    0.198424995f, 0.210223868f, 0.222724497f, 0.235968381f,
    0.25f, 0.264865756f, 0.280615509f, 0.297301739f,
    0.314980239f, 0.333709747f, 0.353553236f, 0.374576509f,
    0.39684999f, 0.420447737f, 0.445448995f, 0.471936762f,
    0.5f, 0.529731512f, 0.561231017f, 0.594603479f,
    0.629960477f, 0.667419493f, 0.707106471f, 0.749153018f,
    0.79369998f, 0.840895474f, 0.890897989f, 0.943873525f,
    1.0f, 1.05946302f, 1.12246203f, 1.18920696f,
    1.25992095f, 1.33483899f, 1.41421294f, 1.49830604f,
    1.58739996f, 1.68179095f, 1.78179598f, 1.88774705f,
    2.0f, 2.0f,
};
