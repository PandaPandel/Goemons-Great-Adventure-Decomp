#include "common.h"
#include "object.h"
#include "camera.h"
#include "boss.h"
#include "bappcam.h"
#include "bmove.h"
#include "fade.h"
#include "gfxmgr.h"
#include "mathutil.h"
#include "tmanager.h"

/* The boss battles' cut-in cameras (overlay 10): "change" (the view swings round when the
 * battle changes), and two "Bappcam"s, for the んが砲 cannon and the hundred-punch (百烈パンチ)
 * attacks. Each makes a camera task (dracam) under itself, letterboxes the screen, moves the
 * eye and look-at point through timed steps, and opens the letterbox again at the end. The
 * work (BCam) is in boss.h; impact.c has a third "Bappcam". */

extern u32 gGameFrameCount; /* GameState's frame count */
extern Obj *gEffectTask;
extern s32 gFramePool10;


/* "change": the eye swings to the far side of the Impact, then settles behind it, bobbing. */
s32 change(Obj *obj, s16 msg, void *arg) {
    BCam *w = BCAM(obj);
    s32 pad[5];
    f32 x2;
    f32 z2;
    f32 x;
    f32 z;
    s32 pad2[2];

    switch (msg) {
        case 2:
            obj->name = "change";
            fadeCreate(gEffectTask);
            gBattleFlags2 |= 0x200000;
            break;
        case 1:
            frmDestroy(w->frame);
            frmDestroy(w->frames[0]);
            frmDestroy(w->frames[1]);
            frmDestroy(w->frames[2]);
            gBattleFlags2 &= ~0x200000;
            break;
        case 0:
            gBattleFlags2 |= 0x200000;
            if (gBattleFlags & 2) {
                return -1;
            }
            switch (w->on) {
                case 0:
                    w->cam = (Camera *) tskCreate(obj, dracam, 0, NULL);
                    if (w->cam == NULL) {
                        return -1;
                    }
                    gBattleFlags2 |= 0x400;
                    bossHeadingOffset(mathAtan2(gBrotherPosX - gImpactPosX, gBrotherPosZ - gImpactPosZ), 0.0f,
                                           3400.0f, &x, &z);
                    camSetSpeed(w->cam, 30.0f);
                    camSetFov(w->cam, 0.5235988f);
                    camSetEye(w->cam, 0.0f, 0.0f, 0.0f);
                    if (gBattleBossId == 4) {
                        camSetAt(w->cam, x, gBrotherPosY + 900.0f + 50.0f, z);
                    } else {
                        camSetAt(w->cam, x, gCockpitLookAtY + 50.0f, z);
                    }
                    camGetView(w->cam)->zFar = 20000.0f;
                    gBattleFlags2 |= 0x100;
                    w->on = 1;
                    w->step = 0;
                    w->timer = 0;
                    break;
                case 1:
                    camGetView(w->cam);
                    gBattleFlags2 |= 0x200000;
                    switch (w->step) {
                        case 0:
                            gBattleFlags |= 0x10000000;
                            bossHeadingOffset(
                                mathAtan2(gBrotherPosX - gImpactPosX, gBrotherPosZ - gImpactPosZ), 0.0f, 3400.0f,
                                &x2, &z2);
                            if (gBattleBossId == 4) {
                                camMoveEye(w->cam, x2, gBrotherPosY + 900.0f, z2);
                            } else {
                                camMoveEye(w->cam, x2, gCockpitLookAtY, z2);
                            }
                            w->timer = 40;
                            w->step++;
                            break;
                        case 1:
                            w->timer--;
                            if (w->timer == 5) {
                                gBrotherFlags |= 0x2000;
                                if (gBattleFlags & 0x4000000) {
                                    gBattleFlags &= ~0x4000000;
                                } else {
                                    gBattleFlags |= 0x4000000;
                                }
                            } else if (w->timer == 4) {
                                gBrotherFlags &= ~0x2000;
                                gBattleFlags &= ~0x1000000;
                                gBattleFlags2 |= 0x800;
                            } else if (w->timer == 0) {
                                camSetSpeed(w->cam, 8.0f);
                                camSetFov(w->cam, 0.87266463f);
                                camMoveEye(w->cam, 0.0f, sinf(gGameFrameCount * 0.0174532930038800785) * 3.0f,
                                                       0.0f);
                                camMoveAt(w->cam, gCockpitLookAtX, gCockpitLookAtY, gCockpitLookAtZ);
                                w->timer = 80;
                                w->step++;
                            }
                            break;
                        case 2:
                            camMoveEye(w->cam, 0.0f, sinf(gGameFrameCount * 0.0174532930038800785) * 3.0f,
                                                   0.0f);
                            camMoveAt(w->cam, gCockpitLookAtX, gCockpitLookAtY, gCockpitLookAtZ);
                            w->timer--;
                            if (w->timer == 60) {
                                gBattleFlags2 &= ~0x100;
                            }
                            if (w->timer == 0) {
                                gBattleFlags2 &= ~0x400;
                                gBattleFlags &= ~0x10000000;
                                return -1;
                            }
                            break;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Bappcam" for the んが砲 cannon: from beside the cannon, looking at the boss; then the eye
 * rises 300 at frame 120, the look-at point follows at 240, and from 300 the letterbox opens. */
s32 bappcamCannon(Obj *obj, s16 msg, void *arg) {
    BCam *w = BCAM(obj);
    s32 pad[3];

    switch (msg) {
        case 2:
            obj->name = "Bappcam";
            (*fadeCreate)(gEffectTask);
            (*fadeSetParent)(gFramePool10);
            w->cam = (Camera *) (*tskCreate)(obj, dracam, 0, NULL);
            if (w->cam == NULL) {
                return -1;
            }
            gBattleFlags2 |= 0x100;
            gBattleFlags2 |= 0x400;
            gBattleFlags2 |= 0x10000;
            gBattleFlags2 |= 0x200000;
            (*camSetFov)(w->cam, 0.69813175f);
            (*camSetSpeed)(w->cam, 120.0f);
            if (gBossPosX > 0.0f) {
                (*camSetEye)(w->cam, gMegaShotPosX - 1000.0f, gMegaShotPosY - 100.0f, gMegaShotPosZ + 1000.0f);
            } else {
                (*camSetEye)(w->cam, gMegaShotPosX + 1000.0f, gMegaShotPosY - 100.0f, gMegaShotPosZ + 1000.0f);
            }
            (*camSetAt)(w->cam, gBossPosX, gMegaBurstY, gBossPosZ);
            (*camGetView)(w->cam)->zFar = 20000.0f;
            gBattleFlags |= 0x10000000;
            gBattleFlags |= 0x1000000;
            gBrotherFlags |= 0x10000;
            (*fadeSetLetterbox)(320, 50, 0, 190);
            w->on = 1;
            w->step = 0;
            w->timer = 0;
            break;
        case 1:
            (*frmDestroy)(w->frame);
            (*frmDestroy)(w->frames[0]);
            (*frmDestroy)(w->frames[1]);
            (*frmDestroy)(w->frames[2]);
            gBattleFlags2 &= ~0x10000;
            gBattleFlags2 &= ~0x200000;
            break;
        case 0:
            gBattleFlags2 |= 0x200000;
            (*camGetView)(w->cam);
            if (gBattleFlags & 2) {
                return -1;
            }
            PRINTF("んが砲カメラstep/timer[%d/%d]\n", w->step, w->timer);
            switch (w->step) {
                case 0:
                    if (w->timer == 120) {
                        if (gBossPosX > 0.0f) {
                            (*camMoveEye)(w->cam, gMegaShotPosX - 1000.0f, gMegaShotPosY + 300.0f,
                                                      gMegaShotPosZ + 1000.0f);
                        } else {
                            (*camMoveEye)(w->cam, gMegaShotPosX + 1000.0f, gMegaShotPosY + 300.0f,
                                                      gMegaShotPosZ + 1000.0f);
                        }
                        (*camMoveAt)(w->cam, gBossPosX, gMegaShotPosY + 300.0f, gBossPosZ);
                        w->step++;
                    }
                    break;
                case 1:
                    if (w->timer == 240) {
                        (*camSetSpeed)(w->cam, 30.0f);
                        (*camMoveAt)(w->cam, gBossPosX, gBossPosY + 300.0f, gBossPosZ);
                        w->step++;
                    }
                    break;
                case 2:
                    if (w->timer == 300) {
                        w->step++;
                    }
                    break;
                case 3:
                    if ((*fadeMoveLetterbox)(0, 240, 20) != 0) {
                        gBattleFlags &= ~0x10000000;
                        gBattleFlags &= ~0x1000000;
                        gBrotherFlags &= ~0x10000;
                        gBattleFlags2 &= ~0x100;
                        gBattleFlags2 &= ~0x400;
                        return -1;
                    }
                    break;
            }
            w->timer++;
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* "Bappcam" for the hundred-punch: from one side of the boss (the side away from it), shaking
 * at random by up to 1000 while the punches land; at 120 the eye moves in and the look-at point
 * rises 300, at 240 it slows, and from 241 the letterbox opens. If the battle ends first, the
 * letterbox is put back at once. */
s32 bappcamHundredPunch(Obj *obj, s16 msg, void *arg) {
    BCam *w = BCAM(obj);
    s32 pad[3];

    switch (msg) {
        case 2:
            obj->name = "Bappcam";
            (*fadeCreate)(gEffectTask);
            (*fadeSetParent)(gFramePool10);
            w->cam = (Camera *) (*tskCreate)(obj, dracam, 0, NULL);
            if (w->cam == NULL) {
                return -1;
            }
            (*camSetSpeed)(w->cam, 8.0f);
            gBattleFlags2 |= 0x100;
            gBattleFlags2 |= 0x200000;
            (*camSetFov)(w->cam, 0.69813175f);
            (*camSetSpeed)(w->cam, 120.0f);
            w->side = gBossPosX;
            if (w->side > 0.0f) {
                (*camSetEye)(w->cam, -1000.0f, 0.0f, 600.0f);
            } else {
                (*camSetEye)(w->cam, 1000.0f, 0.0f, 600.0f);
            }
            if (gBattleBossId == 1) {
                (*camSetAt)(w->cam, gBossPosX, gBossPosY, gBossPosZ);
            }
            if (gBattleBossId == 2) {
                (*camSetAt)(w->cam, gBossPosX, gBossPosY + 600.0f, gBossPosZ);
            }
            if (gBattleBossId == 3) {
                (*camSetAt)(w->cam, gBossPosX, gBossPosY + 600.0f, gBossPosZ);
            }
            if (gBattleBossId == 4) {
                (*camSetAt)(w->cam, gBossPosX, gBossPosY, gBossPosZ);
            }
            if (gBattleBossId == 5) {
                (*camSetAt)(w->cam, gBossPosX, gBossPosY, gBossPosZ);
            }
            (*camGetView)(w->cam)->zFar = 20000.0f;
            gBattleFlags |= 0x10000000;
            gBattleFlags |= 0x1000000;
            gBrotherFlags |= 0x10000;
            (*fadeSetLetterbox)(320, 50, 0, 190);
            w->on = 1;
            w->step = 0;
            w->timer = 0;
            break;
        case 1:
            (*frmDestroy)(w->frame);
            (*frmDestroy)(w->frames[0]);
            (*frmDestroy)(w->frames[1]);
            (*frmDestroy)(w->frames[2]);
            gBattleFlags2 &= ~0x200000;
            break;
        case 0:
            gBattleFlags2 |= 0x200000;
            (*camGetView)(w->cam);
            if (gBattleFlags & 2) {
                PRINTF("[百烈パンチカメラ]本体が死んでいるのでフェードを消します。\n");
                (*fadeSetLetterbox)(1, 1, 319, 239);
                gBattleFlags &= ~0x10000000;
                gBattleFlags &= ~0x1000000;
                gBrotherFlags &= ~0x10000;
                gBattleFlags2 &= ~0x100;
                return -1;
            }
            switch (w->step) {
                case 0:
                    if (gBattleBossId == 1) {
                        (*camMoveAt)(w->cam, fadeRandRange(-1000, 1000) +gBossPosX, fadeRandRange(-1000, 1000) + gBossPosY,
                                                  gBossPosZ);
                    }
                    if (gBattleBossId == 2) {
                        (*camMoveAt)(w->cam, fadeRandRange(-1000, 1000) +gBossPosX,
                                                  fadeRandRange(-1000, 1000) + (gBossPosY + 600.0f), gBossPosZ);
                    }
                    if (gBattleBossId == 3) {
                        (*camMoveAt)(w->cam, fadeRandRange(-1000, 1000) +gBossPosX,
                                                  fadeRandRange(-1000, 1000) + (gBossPosY + 600.0f), gBossPosZ);
                    }
                    if (gBattleBossId == 4) {
                        (*camMoveAt)(w->cam, fadeRandRange(-1000, 1000) +gBossPosX, fadeRandRange(-1000, 1000) + gBossPosY,
                                                  gBossPosZ);
                    }
                    if (gBattleBossId == 5) {
                        (*camMoveAt)(w->cam, fadeRandRange(-1000, 1000) +gBossPosX, fadeRandRange(-1000, 1000) + gBossPosY,
                                                  gBossPosZ);
                    }
                    if (w->timer == 120) {
                        if (w->side > 0.0f) {
                            (*camMoveEye)(w->cam, -1000.0f, 200.0f, 400.0f);
                        } else {
                            (*camMoveEye)(w->cam, 1000.0f, 200.0f, 400.0f);
                        }
                        if (gBattleBossId == 1) {
                            (*camMoveAt)(w->cam, gBossPosX, gBossPosY + 300.0f, gBossPosZ);
                        }
                        if (gBattleBossId == 2) {
                            (*camMoveAt)(w->cam, gBossPosX, gBossPosY + 300.0f + 600.0f, gBossPosZ);
                        }
                        if (gBattleBossId == 3) {
                            (*camMoveAt)(w->cam, gBossPosX, gBossPosY + 300.0f + 600.0f, gBossPosZ);
                        }
                        if (gBattleBossId == 4) {
                            (*camMoveAt)(w->cam, gBossPosX, gBossPosY + 300.0f, gBossPosZ);
                        }
                        if (gBattleBossId == 5) {
                            (*camMoveAt)(w->cam, gBossPosX, gBossPosY + 300.0f, gBossPosZ);
                        }
                        w->step++;
                    }
                    break;
                case 1:
                    if (w->timer == 240) {
                        (*camSetSpeed)(w->cam, 30.0f);
                        if (gBattleBossId == 1) {
                            (*camMoveAt)(w->cam, gBossPosX, gBossPosY + 300.0f, gBossPosZ);
                        }
                        if (gBattleBossId == 2) {
                            (*camMoveAt)(w->cam, gBossPosX, gBossPosY + 300.0f + 600.0f, gBossPosZ);
                        }
                        if (gBattleBossId == 3) {
                            (*camMoveAt)(w->cam, gBossPosX, gBossPosY + 300.0f + 600.0f, gBossPosZ);
                        }
                        if (gBattleBossId == 4) {
                            (*camMoveAt)(w->cam, gBossPosX, gBossPosY + 300.0f, gBossPosZ);
                        }
                        if (gBattleBossId == 5) {
                            (*camMoveAt)(w->cam, gBossPosX, gBossPosY + 300.0f, gBossPosZ);
                        }
                        w->step++;
                    }
                    break;
                case 2:
                    if (gBattleBossId == 1) {
                        (*camMoveAt)(w->cam, fadeRandRange(-1000, 1000) +(gBossPosX + 300.0f),
                                                  fadeRandRange(-1000, 1000) + gBossPosY, gBossPosZ);
                    }
                    if (gBattleBossId == 2) {
                        (*camMoveAt)(w->cam, fadeRandRange(-1000, 1000) +(gBossPosX + 300.0f),
                                                  fadeRandRange(-1000, 1000) + (gBossPosY + 600.0f), gBossPosZ);
                    }
                    if (gBattleBossId == 3) {
                        (*camMoveAt)(w->cam, fadeRandRange(-1000, 1000) +(gBossPosX + 300.0f),
                                                  fadeRandRange(-1000, 1000) + (gBossPosY + 600.0f), gBossPosZ);
                    }
                    if (gBattleBossId == 4) {
                        (*camMoveAt)(w->cam, fadeRandRange(-1000, 1000) +(gBossPosX + 300.0f),
                                                  fadeRandRange(-1000, 1000) + gBossPosY, gBossPosZ);
                    }
                    if (gBattleBossId == 5) {
                        (*camMoveAt)(w->cam, fadeRandRange(-1000, 1000) +(gBossPosX + 300.0f),
                                                  fadeRandRange(-1000, 1000) + gBossPosY, gBossPosZ);
                    }
                    if (w->timer == 241) {
                        w->step++;
                    }
                    break;
                case 3:
                    if ((*fadeMoveLetterbox)(0, 240, 20) != 0) {
                        gBattleFlags &= ~0x10000000;
                        gBattleFlags &= ~0x1000000;
                        gBrotherFlags &= ~0x10000;
                        gBattleFlags2 &= ~0x100;
                        return -1;
                    }
                    break;
            }
            w->timer++;
            PRINTF("\rP100timer[%d]", w->timer);
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}
