#include "tlb.h"
#include "tlb/tlb_0324.h"

/* The boss's work, as its movement helpers read it. */
typedef struct BossMoveW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ u8 pad10[0x38];
    /* 0x48 */ f32 speed;
} BossMoveW;

/* Keeps the boss inside the arena (x within 6000, y within 3000, z from -7800 to -800) and
 * publishes its position and heading for the camera. */
void btaiMoveLimit(Obj *o) {
    BossMoveW *w = (BossMoveW *) FX(o);

    if (w->frame->pos[0] > 6000.0f) {
        PRINTF("移動制限[X+]\n");
        w->frame->pos[0] = 6000.0f;
    }
    if (w->frame->pos[0] < -6000.0f) {
        PRINTF("移動制限[X-]\n");
        w->frame->pos[0] = -6000.0f;
    }
    if (w->frame->pos[1] > 3000.0f) {
        PRINTF("移動制限[Y+]\n");
        w->frame->pos[1] = 3000.0f;
    }
    if (w->frame->pos[1] < -3000.0f) {
        PRINTF("移動制限[Y-]\n");
        w->frame->pos[1] = -3000.0f;
    }
    if (w->frame->pos[2] > -800.0f) {
        PRINTF("移動制限[Z+]\n");
        w->frame->pos[2] = -800.0f;
    }
    if (w->frame->pos[2] < -7800.0f) {
        PRINTF("移動制限[Z-]\n");
        w->frame->pos[2] = -7800.0f;
    }
    gBossPosX = w->frame->pos[0];
    gBossPosY = w->frame->pos[1];
    gBossPosZ = w->frame->pos[2];
    gBossYaw = w->frame->rot[1];
}

/* Heads the boss for x, y, z: 0 once within 100. Its speed grows by accel + 1, up to 50. The
 * distance sums z first, then y and x (the order decides which parameter IDO homes). */
s32 btaiBmoveX(Obj *o, f32 x, f32 y, f32 z, f32 accel) {
    BossMoveW *w = (BossMoveW *) FX(o);
    f32 dist = sqrtf((z - w->frame->pos[2]) * (z - w->frame->pos[2]) +
                     ((y - w->frame->pos[1]) * (y - w->frame->pos[1]) +
                      (x - w->frame->pos[0]) * (x - w->frame->pos[0])));

    if (dist < 100.0f) {
        PRINTF("目標地点に到着しました。(%d,%d)->(%d %d)\n");
        return 0;
    }
    w->speed += accel + 1.0f;
    if (w->speed > 50.0f) {
        w->speed = 50.0f;
    }
    if (w->speed * dist == 0.0f) {
        PRINTF("[BmoveX]相対距離０です。\n");
        return 0;
    }
    w->vel[0] = (x - w->frame->pos[0]) / dist * w->speed;
    w->vel[1] = (y - w->frame->pos[1]) / dist * w->speed;
    w->vel[2] = (z - w->frame->pos[2]) / dist * w->speed;
    return 1;
}

/* btaiBmoveX faster: the speed grows by (accel + 1) * 2.5, up to 150. */
s32 btaiBmoveXFast(Obj *o, f32 x, f32 y, f32 z, f32 accel) {
    BossMoveW *w = (BossMoveW *) FX(o);
    f32 dist = sqrtf((z - w->frame->pos[2]) * (z - w->frame->pos[2]) +
                     ((y - w->frame->pos[1]) * (y - w->frame->pos[1]) +
                      (x - w->frame->pos[0]) * (x - w->frame->pos[0])));

    if (dist < 100.0f) {
        PRINTF("目標地点に到着しました。(%d,%d)->(%d %d)\n");
        return 0;
    }
    w->speed += (accel + 1.0f) * 2.5;
    if (w->speed > 150.0f) {
        w->speed = 150.0f;
    }
    if (w->speed * dist == 0.0f) {
        PRINTF("[BmoveX]相対距離０です。\n");
        return 0;
    }
    w->vel[0] = (x - w->frame->pos[0]) / dist * w->speed;
    w->vel[1] = (y - w->frame->pos[1]) / dist * w->speed;
    w->vel[2] = (z - w->frame->pos[2]) / dist * w->speed;
    return 1;
}

/* Turns the boss to face x, z by at most `step` degrees a frame, the difference taken the short
 * way round; 1 once it faces them. */
s32 btaiTurnTo(Obj *o, f32 x, f32 z, f32 step) {
    BossMoveW *w = (BossMoveW *) FX(o);
    f32 target = mathAtan2(w->frame->pos[0] - x, w->frame->pos[2] - z) + 3.1415927f;
    f32 size = (w->frame->rot[1] < target) ? target - w->frame->rot[1] : -(target - w->frame->rot[1]);

    if (target - w->frame->rot[1] > 3.1415927f) {
        w->frame->rot[1] += 6.2831855f;
    } else if (target - w->frame->rot[1] < -3.1415927f) {
        w->frame->rot[1] -= 6.2831855f;
    }
    if (size < step * 0.0174532930038800785) {
        w->frame->rot[1] = target;
        return 1;
    }
    if (target < w->frame->rot[1]) {
        w->frame->rot[1] -= step * 0.0174532930038800785;
    } else {
        w->frame->rot[1] += step * 0.0174532930038800785;
    }
    return 0;
}

/* Banks the boss into its motion: it rolls with its z speed by 0.7 * k degrees a frame and
 * tilts against its x speed by k / 2, each no further than its speed's size times k degrees.
 * The game keeps every value in caller-saved float registers (the degree constant in f18);
 * this C needs f20 and a frame (188 lines). The game recomputes the first size's arms and not
 * the second's; reading the first condition through FX(o) changed nothing.
 * 2026-10-12 (start 117, length +2, frame 0x10 for f20): the code is the game's shape and
 * order; only the colored floats differ (game: DEG f18, (double) k f14, 0.0 f2, roll f0, tilt
 * f2, vel[2] and k * DEG f12; this C: DEG f20, k f0, 0.0 f18, roll f2, tilt f12, vel[2] and
 * k * DEG f14). Tried: both sizes as if/else statements (117, same code); spellings mode (no
 * literal spelled two ways; 117).
 * 2026-10-12 (second try, start 117): this C held the float 0.0f of the two vel tests in f18
 * across both (the game re-makes it, mtc1 zero, per test); the second test's zero spelled `0.f`
 * (IDO_NOTES 365) unshares it: 33, length right, no frame. What is left is a pure colouring
 * permutation (game: (double) k f14, double zero f2, roll f0, tilt f2, vel[2] and k * DEG f12;
 * this C: f2, f0, f12, f0, f14). Misses at 33: the zeros' spellings swapped (`0.f` first; 33,
 * same); greedy (every mode, nothing kept); tilt declared before roll, both set in statements
 * (33, same); the double tests' `0.0` as int `0` (33, same). A register puzzle for the permuter.
 * 2026-10-12 (third try), permuter: "roll++; roll--;" at the function's end
 * raises roll's uses: k and vel[2] take the game's f14/f12 (33 -> 16); then a FAKE do-while at
 * the head of the first then-arm: zero f2, roll f0, tilt f2 as the game (3). The first test
 * reads w->vel[2] plainly now (FX(o) or a cast: the same). Left: -tilt in the last arm takes
 * f14 (roll is live to the end; the game's f0 is roll's, free there). roll++/-- elsewhere:
 * before the vel[0] test 26, in tilt's then-arm 13 (length +1), in the first then-arm 101,
 * before the first if 123; an empty "if (roll) {}" between the blocks: 33.
 * 2026-10-12 (fourth try): without roll++/-- (33) the colouring read off: k f2 (game f14), the
 * double zero f0 (f2), vel[2] f12 -> f14, roll f12 (f0), tilt f0 (f2), -tilt f2 (f0): the game
 * colours roll first, then the zero. Permuter from the 3 form (--stack-diffs, about 1,800
 * iterations, base score 15): nothing better.
 * 2026-10-12 (fifth try): roll++/roll-- at the end of the first if's else-arm (not the function's
 * end): 56. In their place an empty "if (roll) {}" at the end of both arms: 86 (each test is
 * emitted, +6). reloads and empties sweeps: nothing under 3.
 * 2026-10-12 (sixth try): the last else-arm as "roll = -tilt;" compared and stored (-tilt into
 * roll's register): 33 without roll++/--, 4 (length +1) with them; roll++/-- at the head of the
 * last else-arm instead of the end: 4 (-tilt still f14; it shares that block with roll).
 * 2026-10-12 (seventh try), uopt trace (match_function.py trace): colouring takes the constrained
 * ranges first by priority, then the unconstrained ones by range number, each the lowest free
 * colour. A float range is constrained here at 12 interfering ranges: tilt has 12, roll 10
 * (unconstrained, so (double) k, coloured before it, takes f2). roll++/-- at the end made roll
 * constrained (f0) but live through -tilt's blocks, so -tilt took f14 (the 3 form's only
 * difference: -tilt's three f14 where the game has f0). C: roll kept live into tilt's inner then-arm only (it then
 * meets the 0.0f and 2.0 constants: 12, constrained, f0; -tilt is free to take f0). There
 * "roll++; roll--;" is 11: ugen emits the ++ (li.s 1.0 in a temp, as1 drops the dead add) and
 * the temp shifts the else arm's round-robin floats by one; "roll = -roll" 33 (it joins -roll's
 * range); one "roll += roll" 33 (dead, removed before the ranges); "roll += roll; roll +=
 * roll;" (add.s f0,f0,f0, no temp, dropped by as1): MATCH.
 * A kept FAKE (2026-10-17): the permuter's do-while at the head of the first then-arm is gone
 * (catalog #41, redo fakes after a fix: with the roll pair in, it is the same object). Measured
 * in the pair's place, all misses: "roll = roll;" 33; "if (roll) {}" in the same arm 32 (length
 * +6, the float test is emitted); nothing there 33. */
void btaiBank(Obj *o, f32 k) {
    BossMoveW *w = (BossMoveW *) FX(o);
    f32 roll = (w->vel[2] * 0.0174532930038800785 * k > 0.0)
                   ? w->vel[2] * 0.0174532930038800785 * k
                   : -(w->vel[2] * 0.0174532930038800785 * k);
    f32 tilt = (w->vel[0] * 0.0174532930038800785 * k > 0.0) ? w->vel[0] * 0.0174532930038800785 * k
                                                            : -(w->vel[0] * 0.0174532930038800785 * k);

    if (w->vel[2] > 0.f) {
        w->frame->rot[0] += k * 0.0174532930038800785 * 0.7;
        if (w->frame->rot[0] > roll) {
            w->frame->rot[0] = roll;
        }
    } else {
        w->frame->rot[0] -= k * 0.0174532930038800785 * 0.7;
        if (w->frame->rot[0] < -roll) {
            w->frame->rot[0] = -roll;
        }
    }
    if (w->vel[0] < 0.0f) {
        w->frame->rot[2] += k * 0.0174532930038800785 / 2;
        if (w->frame->rot[2] > tilt) {
            w->frame->rot[2] = tilt;
            roll += roll; /* FAKE: keeps roll live into this arm (constrained, f0) */
            roll += roll;
        }
    } else {
        w->frame->rot[2] -= k * 0.0174532930038800785 / 2;
        if (w->frame->rot[2] < -tilt) {
            w->frame->rot[2] = -tilt;
        }
    }
}

extern f32 btaiPushVelX; /* the x and z velocity the boss is pushed toward (tlb_0324_1) */
extern f32 btaiPushVelZ;
extern f32 btaiPushStartVelX; /* its x and z velocity when the push began */
extern f32 btaiPushStartVelZ;

/* Moves the boss's x and z velocity a third of the way from where the push began toward the
 * push's, undoing the step when it would pass the push in size. */
void btaiEaseVel(Obj *o) {
    BossMoveW *w = (BossMoveW *) FX(o);
    f32 dx = (btaiPushVelX - btaiPushStartVelX) / 3.0f;
    f32 dz = (btaiPushVelZ - btaiPushStartVelZ) / 3.0f;

    w->vel[0] += dx;
    if ((w->vel[0] > 0.0f ? w->vel[0] : -w->vel[0]) >
        (btaiPushVelX > 0.0f ? btaiPushVelX : -btaiPushVelX)) {
        w->vel[0] -= dx;
    }
    w->vel[2] += dz;
    if ((w->vel[2] > 0.0f ? w->vel[2] : -w->vel[2]) >
        (btaiPushVelZ > 0.0f ? btaiPushVelZ : -btaiPushVelZ)) {
        w->vel[2] -= dz;
    }
}

/* Rumbles the cockpit's controller, and the brother's too when two play. */
void btaiRumble(u32 pattern, s32 time) {
    contPakRumbleStart(gCockpitPort, pattern, time);
    if (fadeCountPlayers() >= 2) {
        contPakRumbleStart(gBrotherPort, pattern, time);
    }
}
