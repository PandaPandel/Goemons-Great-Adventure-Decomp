#include "tlb.h"
#include "tlb/tlb_0325.h"

s32 bfriBitProc(Obj *obj, s16 msg, void *arg);
s32 bfriBitPaProc(Obj *obj, s16 msg, void *arg);

Obj *bfriBitPaCreate(Obj *obj) {
    PRINTF("ビットパパを登録します\n");
    return tskCreate(obj, bfriBitPaProc, obj->file, NULL);
}

#define DPRINTF

#define DEG2RAD 0.0174532930038800785

/* bfriBitProc's argument: only what its starter sets. */
typedef struct Tlb325Arg14FD4 {
    /* 0x00 */ u8 pad00[0x14];
    /* 0x14 */ f32 pos[3];
    /* 0x20 */ s32 n;
    /* 0x24 */ u8 pad24[0x2C];
} Tlb325Arg14FD4;

/* A bit's work (bfriBitProc). */
typedef struct BitW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ PolFrame *glow;
    /* 0x08 */ f32 vel[3];
    /* 0x14 */ f32 goal[3]; /* where it drifts when blown away */
    /* 0x20 */ s32 id; /* 0-6 */
    /* 0x24 */ ACollFrame *atk;
    /* 0x28 */ ACollFrame *dmg;
    /* 0x2C */ f32 radius; /* its distance from the ring's centre */
    /* 0x30 */ f32 spin;
    /* 0x34 */ f32 unused34; /* set 0.0f when it is blown away, never read */
    /* 0x38 */ u8 hp;
    /* 0x39 */ u8 phase;
    /* 0x3C */ s32 timer;
    /* 0x40 */ s16 alpha;
    /* 0x42 */ u16 hitFlash; /* red */
    /* 0x44 */ u16 whiteFlash;
    /* 0x46 */ u8 mode; /* bfriBarrierState */
    /* 0x48 */ s16 r;
    /* 0x4A */ s16 g;
    /* 0x4C */ s16 b;
} BitW;

typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                    s32 a);

#define sinf (*sinf)
#define cosf (*cosf)
#define bfriRand (*bfriRand)
#define bfriRandSigned (*bfriRandSigned)
#define bfriElecBombCreate (*bfriElecBombCreate)

extern CamView *gBattleCamView; /* the battle's camera (beffect.c) */
extern Gfx bfriBitDl[]; /* the bit */
extern Gfx bfriBitGlowDl[]; /* a glow */
extern s32 bfriFightOver; /* tlb_0325_1's: set when the fight ends */
extern s32 bfriFlashFrames;
f32 bfriBitFrames = 1.0f; /* the frames a bit takes to reach its place */
f32 bfriRingAngle = 0.0f; /* the bits' angle around the boss */
f32 bfriRingRadius = 1.0f; /* the ring's radius */
s32 bfriBitsAlive = 0; /* the bits alive */
s32 bfriBarrierState = 0; /* tlb_0325_1's: the boss's barrier state, 4 while it stands */
s32 bfriBitsHidden = 0; /* 1: the bits are hidden */
s32 bfriBarrierSpark = 0; /* 1: spark across the barrier */
s32 bfriBarrierSpark2 = 0;
s32 bfriBitsRemake = 0;
extern f32 bfriBitX[7]; /* the bits' x */
extern f32 bfriBitY[7]; /* y */
extern f32 bfriBitZ[7]; /* z */
extern f32 bfriBitTrailX[3][7]; /* three points of a trail behind each bit (the barrier): x */
extern f32 bfriBitTrailY[3][7]; /* y */
extern f32 bfriBitTrailZ[3][7]; /* z */

/* "BIT", one of the boss's bits: a ball with a glow (scale 1.3 and 15), an attack box (radius
 * 130, kind 0x41) and a damage box (150 by 250, kind 0x42), five hit points. It follows the
 * boss's barrier state (bfriBarrierState) in its mode, easing over bfriBitFrames
 * frames to its place in a ring around the boss:
 * - 0 and 6: a ring 430 behind the boss whose radius breathes by 100 (6 spaces at most five);
 * - 1: a ring of radius 500, 860 behind, red; when tlb_0325_1 asks, the bit that faces front
 *   flashes white and sparks twice across the barrier's centre;
 * - 2: the ring 860 behind and 100 lower, green, fading in;
 * - 3 and 5: blown away from the boss (5 faster, and it bursts at its timer's end);
 * - 4: the barrier: a tighter ring 688 behind, red, with a three-point trail.
 * Hits by kinds 1, 3, 5 or 8 burst it (ryou drop); kinds 15, 16 and 60 cost a hit point. Its
 * own box hitting bursts it too.
 * Matches (check: "MATCH except own section addresses", 2026-10-12). Mode 1 sets the spin
 * before the colour and decrements alpha in its test (before: alpha first, 1566 lines; the
 * colour before the spin, 1558). The frame pads stand for locals the game declares and does
 * not keep (FAKE). The last three levers, 695 real -> match:
 * - the sparks' positions named in x and sy before each call (the game keeps them in f0/f14
 *   and f0/f2): 695 -> 434, first difference 1110 -> 1402. Misses (695 each): cx and cy
 *   declared the other way round (the slots swap, cy still loads first); the call on one
 *   line; cx + (...) for (...) + cx. Swapping sx and sy's declarations breaks the frame (569);
 * - mode 3's blow-away speeds written (pos - boss) / 500.0f * (f32) (rand + 10) (the game's
 *   mul.s takes the conversion first): 434 -> 157, first difference 1677;
 * - mode 4's -w->timer * 3 for w->timer * -3 (the game's negu is its own instruction into
 *   a t register; as1's expansion of a multiply by -3 negates into at): 157 -> 85 -> match. */
s32 bfriBitProc(Obj *obj, s16 msg, void *arg) {
    BitW *w = (BitW *) FX(obj);
    f32 x;
    s32 pad1[5];
    f32 sx;
    f32 sy;
    s32 pad2[5];
    f32 z;
    f32 a;
    s32 n;
    s32 i;

    switch (msg) {
        case 2:
            w->id = ((Tlb325Arg14FD4 *) arg)->n;
            obj->name = "BIT";
            if ((w->frame = (PolFrame *) frmCreate(gFrmLayers[2], gFrmKindPol)) == NULL) {
                return -1;
            }
            if ((w->glow = (PolFrame *) frmCreate(gFrmLayers[8], gFrmKindPol)) == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, bfriBitDl, 0xFB);
            bossSetPolDL(w->glow, bfriBitGlowDl, 0x120);
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 130, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 0x41;
            if ((w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->frame->f, 150, 250, -80, 0, 0)) ==
                NULL) {
                return -1;
            }
            w->dmg->kind = 0x42;
            w->frame->pos[0] = ((Tlb325Arg14FD4 *) arg)->pos[0];
            w->frame->pos[1] = ((Tlb325Arg14FD4 *) arg)->pos[1];
            w->frame->pos[2] = ((Tlb325Arg14FD4 *) arg)->pos[2];
            w->vel[2] = w->vel[0] = w->vel[1] = w->spin = 0.0f;
            w->frame->scale[2] = 1.3f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->mode = 0;
            w->hp = 5;
            w->radius = bfriRingRadius;
            w->r = w->g = w->b = 0;
            w->timer = 0;
            w->hitFlash = 0;
            w->whiteFlash = 0;
            w->phase = 0;
            w->alpha = 0;
            w->glow->scale[2] = 15.0f;
            w->glow->scale[0] = w->glow->scale[1] = w->glow->scale[2];
            w->glow->flags |= 2;
            w->glow->pos[0] = w->frame->pos[0];
            w->glow->pos[1] = w->frame->pos[1];
            w->glow->pos[2] = w->frame->pos[2];
            DPRINTF("ビットを作りました\n");
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            if (w->glow != NULL) {
                frmDestroy(&w->glow->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, gBossLitTexEnvDl);
            if (w->hitFlash != 0) {
                gDPSetEnvColor(gGfxDlHead++, 255, 0, 0, 255);
                w->hitFlash--;
            } else if (w->whiteFlash != 0) {
                gDPSetEnvColor(gGfxDlHead++, 255, 255, 255, 255);
                w->whiteFlash--;
            } else {
                gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 0);
            }
            gSPEndDisplayList(gGfxDlHead++);
            w->glow->dl = bossPrimEnvColorDL(w->r, w->g, w->b, w->r / 4, w->g / 4, w->b / 4, w->alpha);
            break;
        case 0:
            if (bfriFightOver != 0) {
                return -1;
            }
            if (w->mode != 4) {
                if (acollDmgWasHit(w->dmg)) {
                    if (w->dmg->hitKind == 1 || w->dmg->hitKind == 3 || w->dmg->hitKind == 5 ||
                        w->dmg->hitKind == 8) {
                        if (!(gBattleFlags2 & 0x4000)) {
                            itexPuffYellow(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                            ryouCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
                            itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 0.7f);
                            SoundLocation2(0x228, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                           w->dmg->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                            return -1;
                        }
                    } else if (w->dmg->hitKind == 15 || w->dmg->hitKind == 16 || w->dmg->hitKind == 60) {
                        if (--w->hp <= 0) {
                            itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 0.7f);
                            SoundLocation2(0x228, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                           w->dmg->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                            return -1;
                        }
                        w->hitFlash = 5;
                        DPRINTF("ビット:のこりHP ->%d\n", w->hp);
                    }
                }
                if (acollAtkWasHit(w->atk) == 1) {
                    effectPompomCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 10.0f, 10.0f, 10.0f, 7,
                                       10, 2, 1.0f, 0.0f);
                    effectPompomCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 10.0f, 10.0f, 10.0f, 8,
                                       10, 2, 1.0f, 0.0f);
                    SoundLocation2(0x228, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                   300.0f, 15000.0f, gBattleCamView->fovy);
                    return -1;
                }
            }
            if (gBossLife <= 0) {
                bfriBarrierState = 5;
            }
            w->mode = bfriBarrierState;
            if (bfriBitsHidden == 1) {
                w->frame->flags |= 2;
            } else {
                w->frame->flags &= ~2;
                w->spin += 9 * DEG2RAD;
            }
            bfriBitFrames = 15.0f;
            switch (w->mode) {
                case 0:
                    w->phase = 0;
                    w->timer++;
                    w->radius = sinf(w->timer * 10 * DEG2RAD) * 100.0f + bfriRingRadius;
                    sx = sinf(bfriRingAngle + 360 / bfriBitsAlive * w->id * DEG2RAD) * w->radius;
                    sy = cosf(bfriRingAngle + 360 / bfriBitsAlive * w->id * DEG2RAD) * w->radius;
                    x = gBossPosX - cosf(gBossYaw) * sx - sinf(gBossYaw - PI) * -430.0f;
                    z = gBossPosZ + sinf(gBossYaw) * sx - cosf(gBossYaw - PI) * -430.0f;
                    w->vel[0] = (x - w->frame->pos[0]) / bfriBitFrames;
                    w->vel[1] = (sy + 200.0f - w->frame->pos[1]) / bfriBitFrames;
                    w->vel[2] = (z - w->frame->pos[2]) / bfriBitFrames;
                    w->spin += 3 * DEG2RAD;
                    if (w->spin > 6.2831855f) {
                        w->spin -= 6.2831855f;
                    }
                    w->frame->rot[1] = 0.0f;
                    w->frame->rot[0] = 0.0f;
                    w->frame->rot[2] = w->spin;
                    w->frame->rot[1] = mathAtan2(w->frame->pos[0], w->frame->pos[2]) - PI;
                    w->alpha -= 2;
                    if (w->alpha < 0) {
                        w->alpha = 0;
                    }
                    break;
                case 6:
                    w->phase = 0;
                    w->timer++;
                    w->radius = sinf(w->timer * 10 * DEG2RAD) * 100.0f + bfriRingRadius;
                    n = bfriBitsAlive;
                    if (n > 5) {
                        n = 5;
                    }
                    sx = sinf(bfriRingAngle + 360 / n * w->id * DEG2RAD) * w->radius;
                    sy = cosf(bfriRingAngle + 360 / n * w->id * DEG2RAD) * w->radius;
                    x = gBossPosX - cosf(gBossYaw) * sx - sinf(gBossYaw - PI) * -430.0f;
                    z = gBossPosZ + sinf(gBossYaw) * sx - cosf(gBossYaw - PI) * -430.0f;
                    w->vel[0] = (x - w->frame->pos[0]) / bfriBitFrames;
                    w->vel[1] = (sy + 200.0f - w->frame->pos[1]) / bfriBitFrames;
                    w->vel[2] = (z - w->frame->pos[2]) / bfriBitFrames;
                    w->spin += 3 * DEG2RAD;
                    if (w->spin > 6.2831855f) {
                        w->spin -= 6.2831855f;
                    }
                    w->frame->rot[1] = 0.0f;
                    w->frame->rot[0] = 0.0f;
                    w->frame->rot[2] = w->spin;
                    w->frame->rot[1] = mathAtan2(w->frame->pos[0], w->frame->pos[2]) - PI;
                    w->alpha -= 2;
                    if (w->alpha < 0) {
                        w->alpha = 0;
                    }
                    break;
                case 1: {
                    f32 cx;
                    f32 cy;
                    f32 cz;

                    cx = (bfriBitX[0] + bfriBitX[1] + bfriBitX[2] + bfriBitX[3] + bfriBitX[4]) / 7.0f;
                    cy = (bfriBitY[0] + bfriBitY[1] + bfriBitY[2] + bfriBitY[3] + bfriBitY[4]) / 7.0f;
                    cz = (bfriBitZ[0] + bfriBitZ[1] + bfriBitZ[2] + bfriBitZ[3] + bfriBitZ[4]) / 7.0f;
                    w->phase = 0;
                    w->radius = 500.0f;
                    sx = sinf(bfriRingAngle + 360 / bfriBitsAlive * w->id * DEG2RAD) * w->radius;
                    sy = cosf(bfriRingAngle + 360 / bfriBitsAlive * w->id * DEG2RAD) * w->radius;
                    x = gBossPosX - cosf(gBossYaw) * sx + sinf(gBossYaw - PI) * -860.0f;
                    z = gBossPosZ + sinf(gBossYaw) * sx + cosf(gBossYaw - PI) * -860.0f;
                    w->vel[0] = (x - w->frame->pos[0]) / bfriBitFrames;
                    w->vel[1] = (sy + 200.0f - w->frame->pos[1]) / bfriBitFrames;
                    w->vel[2] = (z - w->frame->pos[2]) / bfriBitFrames;
                    w->spin += 12 * DEG2RAD;
                    w->r = 255;
                    w->g = 0;
                    w->b = 64;
                    if (--w->alpha < 0) {
                        w->alpha = 0;
                    }
                    if (w->spin > 6.2831855f) {
                        w->spin -= 6.2831855f;
                    }
                    w->frame->rot[0] = 0.0f;
                    w->frame->rot[2] = w->spin;
                    w->frame->rot[1] = mathAtan2(w->frame->pos[0], w->frame->pos[2]) - PI;
                    if (bfriBarrierSpark != 0 && cosf(w->spin) > 0.7f) {
                        w->whiteFlash = 5;
                        bfriBarrierSpark = 0;
                        bfriFlashFrames = 5;
                        x = (w->frame->pos[0] - cx) / 8 * 3 + cx;
                        sy = (w->frame->pos[1] - cy) / 8 * 3 + cy;
                        bfriElecBombCreate(obj, x, sy, cz);
                        x = cx - (w->frame->pos[0] - cx) / 8 * 3;
                        sy = cy - (w->frame->pos[1] - cy) / 8 * 3;
                        bfriElecBombCreate(obj, x, sy, cz);
                    }
                    break;
                }
                case 2:
                    w->phase = 0;
                    w->radius = bfriRingRadius;
                    sx = sinf(bfriRingAngle + 360 / bfriBitsAlive * w->id * DEG2RAD) * w->radius;
                    sy = cosf(bfriRingAngle + 360 / bfriBitsAlive * w->id * DEG2RAD) * w->radius;
                    x = gBossPosX - cosf(gBossYaw) * sx + sinf(gBossYaw - PI) * -860.0f;
                    z = gBossPosZ + sinf(gBossYaw) * sx + cosf(gBossYaw - PI) * -860.0f;
                    w->vel[0] = (x - w->frame->pos[0]) / bfriBitFrames;
                    w->vel[1] = (sy - 100.0f - w->frame->pos[1]) / bfriBitFrames;
                    w->vel[2] = (z - w->frame->pos[2]) / bfriBitFrames;
                    w->spin -= 36 * DEG2RAD;
                    if (w->spin < 0.0f) {
                        w->spin += 6.2831855f;
                    }
                    w->frame->rot[1] = 0.0f;
                    w->frame->rot[0] = 0.0f;
                    w->frame->rot[2] = w->spin;
                    w->frame->rot[1] = mathAtan2(w->frame->pos[0], w->frame->pos[2]) - PI;
                    if (w->alpha < 160) {
                        w->alpha += 5;
                    }
                    w->r = 0;
                    w->g = 255;
                    w->b = 144;
                    break;
                case 3:
                    w->r = 255;
                    w->g = 112;
                    w->b = 0;
                    w->frame->rot[0] += 7 * DEG2RAD;
                    a = mathAtan2(gBossPosX, gBossPosZ);
                    sinf(a + PI);
                    cosf(a + PI);
                    switch (w->phase) {
                        case 0:
                            w->vel[0] = (w->frame->pos[0] - gBossPosX) / 500.0f *
                                        (f32) (bfriRand(10) + 10);
                            w->vel[2] = (w->frame->pos[2] - gBossPosZ) / 500.0f *
                                        (f32) (bfriRand(10) + 10);
                            w->vel[1] = (w->frame->pos[1] - gBossPosY) / 500.0f *
                                        (f32) (bfriRand(10) + 10);
                            while ((w->vel[0] > 0.0f ? w->vel[0] : -w->vel[0]) >
                                   (f32) ((gBossPhase + 60) / 2)) {
                                w->vel[0] *= 0.95;
                            }
                            while ((w->vel[1] > 0.0f ? w->vel[1] : -w->vel[1]) >
                                   (f32) ((gBossPhase + 60) / 2)) {
                                w->vel[1] *= 0.95;
                            }
                            while ((w->vel[2] > 0.0f ? w->vel[2] : -w->vel[2]) >
                                   (f32) ((gBossPhase + 60) / 2)) {
                                w->vel[2] *= 0.95;
                            }
                            w->phase = 1;
                            w->timer = (bfriRand(10) + 30) * w->id + 20;
                            w->unused34 = 0.0f;
                            w->goal[0] = bfriRandSigned(20);
                            w->goal[1] = bfriRandSigned(20);
                            w->goal[2] = 100.0f;
                            break;
                        case 1:
                            w->timer--;
                            w->vel[0] *= 0.95;
                            w->vel[2] *= 0.95;
                            w->vel[1] *= 0.95;
                            w->spin += 12 * DEG2RAD;
                            if (w->timer <= 0) {
                                w->phase = 2;
                                w->vel[0] = (w->goal[0] - w->frame->pos[0]) / 100.0f;
                                w->vel[1] = (w->goal[1] - w->frame->pos[1]) / 98.0f;
                                w->vel[2] = (w->goal[2] - w->frame->pos[2]) / 100.0f;
                            }
                            if (w->alpha < 80) {
                                w->alpha += 2;
                            }
                            break;
                        case 2:
                            w->timer++;
                            if (w->alpha < 176) {
                                w->alpha += 2;
                            }
                            break;
                    }
                    if (w->spin > 6.2831855f) {
                        w->spin -= 6.2831855f;
                    }
                    if (w->frame->pos[2] > 50.0f) {
                        return -1;
                    }
                    break;
                case 4:
                    w->phase = 0;
                    w->r = 255;
                    w->g = 0;
                    w->b = 80;
                    w->timer++;
                    n = bfriBitsAlive;
                    if (n > 5) {
                        n = 5;
                    }
                    if (w->timer > 360) {
                        w->timer = 0;
                    }
                    w->radius = sinf((-w->timer * 3 + 360 / n * w->id) * DEG2RAD) * 100.0f * 1.6 +
                                bfriRingRadius * 0.7;
                    sx = sinf(bfriRingAngle + 360 / n * w->id * DEG2RAD) * w->radius * 1.0;
                    sy = cosf(bfriRingAngle + 360 / n * w->id * DEG2RAD) * w->radius * 1.1;
                    x = gBossPosX - cosf(gBossYaw) * sx + sinf(gBossYaw - PI) * -688.0;
                    z = gBossPosZ + sinf(gBossYaw) * sx + cosf(gBossYaw - PI) * -688.0;
                    w->vel[0] = (x - w->frame->pos[0]) / bfriBitFrames;
                    w->vel[1] = (sy - w->frame->pos[1]) / bfriBitFrames;
                    w->vel[2] = (z - w->frame->pos[2]) / bfriBitFrames;
                    w->spin += 3 * DEG2RAD;
                    if (w->spin > 6.2831855f) {
                        w->spin -= 6.2831855f;
                    }
                    w->frame->rot[1] = 0.0f;
                    w->frame->rot[0] = 0.0f;
                    w->frame->rot[2] = w->spin;
                    w->frame->rot[1] = mathAtan2(w->frame->pos[0], w->frame->pos[2]) - PI;
                    w->alpha += 2;
                    if (w->alpha > 255) {
                        w->alpha = 255;
                    }
                    for (i = 0; i < 3; i++) {
                        w->radius = sinf((-w->timer * 3 + 360 / bfriBitsAlive * w->id) * DEG2RAD) * 100.0f *
                                        1.6 +
                                    bfriRingRadius;
                        w->radius *= 0.01;
                        sx = sinf(bfriRingAngle + 360 / bfriBitsAlive * (w->id + 2) * DEG2RAD) *
                             w->radius;
                        sy = cosf(bfriRingAngle + 360 / bfriBitsAlive * (w->id + 2) * DEG2RAD) *
                             w->radius;
                        x = gBossPosX - cosf(gBossYaw) * sx + sinf(gBossYaw - PI) * -688.0;
                        z = gBossPosZ + sinf(gBossYaw) * sx + cosf(gBossYaw - PI) * -688.0;
                        bfriBitTrailX[i][w->id] = x;
                        bfriBitTrailY[i][w->id] = sy;
                        bfriBitTrailZ[i][w->id] = z;
                    }
                    break;
                case 5:
                    w->r = 64;
                    w->g = 0;
                    w->b = 0;
                    w->frame->rot[0] += 17 * DEG2RAD;
                    a = mathAtan2(gBossPosX, gBossPosZ);
                    sinf(a + PI);
                    cosf(a + PI);
                    switch (w->phase) {
                        case 0:
                            w->vel[0] = (w->frame->pos[0] - gBossPosX) / 500.0f * 30.0f;
                            w->vel[2] = (w->frame->pos[2] - gBossPosZ) / 500.0f * 30.0f;
                            w->vel[1] = (w->frame->pos[1] - gBossPosY) / 500.0f * 30.0f;
                            while ((w->vel[0] > 0.0f ? w->vel[0] : -w->vel[0]) >
                                   (f32) (gBossPhase + 60)) {
                                w->vel[0] *= 0.95;
                            }
                            while ((w->vel[1] > 0.0f ? w->vel[1] : -w->vel[1]) >
                                   (f32) (gBossPhase + 60)) {
                                w->vel[1] *= 0.95;
                            }
                            while ((w->vel[2] > 0.0f ? w->vel[2] : -w->vel[2]) >
                                   (f32) (gBossPhase + 60)) {
                                w->vel[2] *= 0.95;
                            }
                            w->phase = 1;
                            w->timer = (bfriRand(10) + 50) * w->id + 20;
                            w->unused34 = 0.0f;
                            w->goal[0] = bfriRandSigned(20);
                            w->goal[1] = bfriRandSigned(20);
                            w->goal[2] = 100.0f;
                            break;
                        case 1:
                            w->timer--;
                            w->vel[0] *= 0.95;
                            w->vel[2] *= 0.95;
                            w->vel[1] *= 0.95;
                            w->spin += 12 * DEG2RAD;
                            if (w->timer <= 0) {
                                itexPuffYellow(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                                itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 0.7f);
                                SoundLocation2(0x228, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                               w->dmg->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                                return -1;
                            }
                            if (w->alpha < 80) {
                                w->alpha += 2;
                            }
                            break;
                    }
                    if (w->spin > 6.2831855f) {
                        w->spin -= 12.566371f;
                    }
                    if (w->frame->pos[2] > 50.0f) {
                        return -1;
                    }
                    break;
            }
            w->frame->pos[0] += w->vel[0];
            w->frame->pos[1] += w->vel[1];
            w->frame->pos[2] += w->vel[2];
            w->glow->pos[0] = w->frame->pos[0];
            w->glow->pos[1] = w->frame->pos[1];
            w->glow->pos[2] = w->frame->pos[2];
            bfriBitX[w->id] = w->frame->pos[0];
            bfriBitY[w->id] = w->frame->pos[1];
            bfriBitZ[w->id] = w->frame->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Starts a bfriBitProc task at x, y, z under obj. */
void bfriBitCreate(Obj *obj, f32 x, f32 y, f32 z, s32 n) {
    Tlb325Arg14FD4 arg;
    s32 pad[2];

    arg.pos[2] = z;
    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.n = n;
    DPRINTF("BIT ID %dを作ります\n", n); /* its place by the .rodata order */
    tskCreate(obj, bfriBitProc, obj->file, &arg);
}

/* "BIT_PA", the bits' parent. It turns the bits' angle a degree a step and counts its bits,
 * each with its own ID (0-6). Every 30 frames while the barrier is down or rising (or at once
 * when tlb_0325_1 asks), it starts the first missing bit at the boss, up to seven (five while
 * the barrier stands). The game calls bfriBitCreate through a register (lui t9; jalr), so the
 * call is written "(*bfriBitCreate)(...)".
 * Matched 2026-10-12; it was parked for the constant 1's register, gone with the creation loop
 * on its own counter j (i only in the clears) and "child = obj->child; bw = FX(child);" before
 * the used[] clear. */
s32 bfriBitPaProc(Obj *obj, s16 msg, void *arg) {
    s32 pad0[2];
    s32 n;
    s32 pad[2];
    s32 used[7];
    s32 i;
    Obj *child;
    BitW *bw;
    s32 j;

    switch (msg) {
        case 1:
        case 3:
            break;
        case 2:
            obj->name = "BIT_PA";
            bfriBitsAlive = 0;
            bfriRingAngle = 0.0f;
            bfriBitFrames = 15.0f;
            bfriRingRadius = 700.0f;
            bfriBarrierState = 0;
            bfriBitsHidden = 1;
            bfriBarrierSpark = 0;
            bfriBarrierSpark2 = 0;
            for (i = 0; i < 7; i++) {
                bfriBitX[i] = 0.0f;
                bfriBitY[i] = 0.0f;
                bfriBitZ[i] = 0.0f;
            }
            break;
        case 0:
            n = 0;
            bfriRingAngle += 0.0174532930038800785;
            if (bfriRingAngle > 6.2831855f) {
                bfriRingAngle -= 6.2831855f;
            } else if (bfriRingAngle < 0.0f) {
                bfriRingAngle += 6.2831855f;
            }
            child = obj->child;
            bw = FX(child);
            for (i = 0; i < 7; i++) {
                used[i] = 0;
            }
            while (TRUE) {
                if (child == NULL) {
                    break;
                }
                n++;
                if (used[bw->id]) {
                    DPRINTF("<ビットパパ>ビットのID設定に異常が発生しました。\n");
                    break;
                }
                used[bw->id] = 1;
                child = child->next;
                bw = FX(child);
            }
            if (((gGameFrameCount % 30 == 0 && (bfriBarrierState == 0 || bfriBarrierState == 2))
                 || bfriBitsRemake == 1)
                && bfriBitsRemake != -1) {
                if (bfriBarrierState == 4 && n >= 5) {
                    break;
                }
                if (n < 7) {
                    for (j = 0; j < 7; j++) {
                        if (!used[j]) {
                            DPRINTF("ビットID %d を作成します\n", j);
                            (*bfriBitCreate)(obj, gBossPosX, gBossPosY, gBossPosZ, j);
                            DPRINTF("ビットID %d を作成しました\n", j);
                            n++;
                            break;
                        }
                    }
                }
            }
            bfriBitsAlive = n;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
