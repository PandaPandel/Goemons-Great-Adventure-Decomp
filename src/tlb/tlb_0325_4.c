#include "tlb.h"
#include "tlb/tlb_0325.h"
u32 bfriSkyShown = 0x00000001;
AnimFrame bfriTexAnim[] = {
    { 0x0301AA90, 250 },
    { 0x0301BA90, 250 },
    { 0x00000000, 0 },
};

/* The boss's work, as its movement helpers read it (tlb_0324_4's BossMoveW, the speed later). */
typedef struct BossMoveW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ u8 pad10[0x44];
    /* 0x54 */ f32 speed;
} BossMoveW;

/* Keeps the boss inside the arena (x within 6000, y within 3000, z from -7000 to -860) and
 * publishes its position and heading for the camera. */
void bfriMoveLimit(Obj *o) {
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
    if (w->frame->pos[2] > -860.0f) {
        PRINTF("移動制限[Z+]\n");
        w->frame->pos[2] = -860.0f;
    }
    if (w->frame->pos[2] < -7000.0f) {
        PRINTF("移動制限[Z-]\n");
        w->frame->pos[2] = -7000.0f;
    }
    gBossPosX = w->frame->pos[0];
    gBossPosY = w->frame->pos[1];
    gBossPosZ = w->frame->pos[2];
    gBossYaw = w->frame->rot[1];
}

/* Heads the boss for x, y, z: 0 once within 200. Its speed grows by accel + 1, up to 32 plus 8
 * a difficulty step (gBossPhase). The distance sums z, then y and x, as tlb_0324_4's. */
s32 bfriBmoveX(Obj *o, f32 x, f32 y, f32 z, f32 accel) {
    BossMoveW *w = (BossMoveW *) FX(o);
    f32 dist = sqrtf((z - w->frame->pos[2]) * (z - w->frame->pos[2]) +
                     ((y - w->frame->pos[1]) * (y - w->frame->pos[1]) +
                      (x - w->frame->pos[0]) * (x - w->frame->pos[0])));

    if (dist < 200.0f) {
        PRINTF("目標地点に到着しました。(%d,%d)->(%d %d)\n");
        return 0;
    }
    w->speed += accel + 1.0f;
    if (w->speed > gBossPhase * 8 + 32) {
        w->speed = gBossPhase * 8 + 32;
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

/* bfriBmoveX from farther: 0 once within 300, the speed up to 50 plus 8 a
 * difficulty step. */
s32 bfriBmoveXFar(Obj *o, f32 x, f32 y, f32 z, f32 accel) {
    BossMoveW *w = (BossMoveW *) FX(o);
    f32 dist = sqrtf((z - w->frame->pos[2]) * (z - w->frame->pos[2]) +
                     ((y - w->frame->pos[1]) * (y - w->frame->pos[1]) +
                      (x - w->frame->pos[0]) * (x - w->frame->pos[0])));

    if (dist < 300.0f) {
        PRINTF("目標地点に到着しました。(%d,%d)->(%d %d)\n");
        return 0;
    }
    w->speed += accel + 1.0f;
    if (w->speed > gBossPhase * 8 + 50) {
        w->speed = gBossPhase * 8 + 50;
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
 * way round; 1 once it faces them (tlb_0324_4's btaiTurnTo). */
s32 bfriTurnTo(Obj *o, f32 x, f32 z, f32 step) {
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

#define ABS(x) ((x) > 0.0f ? (x) : -(x))

/* Leans the boss into its flight: its pitch moves toward the z speed and its roll against the
 * x speed (0.7 and 0.5 of a degree per unit of step), each held within the speed's own angle. */
void bfriBank(Obj *o, f32 step) {
    BossMoveW *w = (BossMoveW *) FX(o);
    f32 limX = ABS(w->vel[2] * 0.0174532930038800785 * step);
    f32 limZ = ABS(w->vel[0] * 0.0174532930038800785 * step);

    if (w->vel[2] > 0.0f) {
        w->frame->rot[0] += step * 0.0174532930038800785 * 0.7;
        if (limX < w->frame->rot[0]) {
            w->frame->rot[0] = limX;
        }
    } else {
        w->frame->rot[0] -= step * 0.0174532930038800785 * 0.7;
        if (w->frame->rot[0] < -limX) {
            w->frame->rot[0] = -limX;
        }
    }
    if (w->vel[0] < 0.0f) {
        w->frame->rot[2] += step * 0.0174532930038800785 / 2;
        if (limZ < w->frame->rot[2]) {
            w->frame->rot[2] = limZ;
        }
    } else {
        w->frame->rot[2] -= step * 0.0174532930038800785 / 2;
        if (w->frame->rot[2] < -limZ) {
            w->frame->rot[2] = -limZ;
        }
    }
}

/* Rumbles the cockpit's controller, and the brother's too when two play. */
void bfriRumble(u32 pattern, s32 time) {
    contPakRumbleStart(gCockpitPort, pattern, time);
    if (fadeCountPlayers() >= 2) {
        contPakRumbleStart(gBrotherPort, pattern, time);
    }
}
