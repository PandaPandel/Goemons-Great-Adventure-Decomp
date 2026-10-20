#include "common.h"
#include "object.h"
#include "game.h"
#include "camera.h"
#include "acoll.h"
#include "enemy.h"
#include "boss.h"
#include "attack.h"
#include "audio.h"
#include "bappcam.h"
#include "beffect.h"
#include "bmove.h"
#include "bpause.h"
#include "brother.h"
#include "cockpit.h"
#include "effect.h"
#include "fade.h"
#include "file.h"
#include "fileload.h"
#include "fileres.h"
#include "gfxbuf.h"
#include "gfxmgr.h"
#include "heap.h"
#include "impact.h"
#include "joypad.h"
#include "polygon.h"
#include "scenario.h"
#include "sndloc.h"
#include "tlb/tlb_0323.h"
#include "tlb/tlb_0324.h"
#include "tlb/tlb_0325.h"
#include "tlb/tlb_0326.h"
#include "tlb/tlb_0327.h"
#include "tmanager.h"
#include "vram.h"

/* The debug prints are compiled out but keep their strings: `DPRINTF("...");` is `("...");`.
 * The repo's PRINTF (`if (0) osSyncPrintf`) gives other registers here (pakfiler.c does the same). */
#define DPRINTF
/* FAKE: the boss read as a u32 for the sky's tests (msg 3). Read plainly, IDO sees the same load
 * as the light chain's tests above it, hoists it into every arm of that chain ("lui v1; b; lw v1")
 * and the arms' branches no longer go straight to the sky's first test, as in the game. */
#define SKY_BOSS ((s32) *(u32 *) &gBattleBossId)

/* "Cockpit": the Goemon Impact's cockpit, the root task of a boss battle (overlay 10). Its
 * argument is the boss, 1-6. It is one function of 6,672 instructions.
 * - Made (message 2): it decides who flies.
 *   - One player: that player's character pilots and the partner follows from it (Goemon with
 *     Ebisumaru, Sasuke with Yae). In battle 1 the Impact starts as Omitsu's (flag 0x4000000 of
 *     gBattleFlags), and Sasuke's partner becomes Goemon.
 *   - If only player 2, 3 or 4 is alive, that controller becomes the cockpit's (gCockpitPort).
 *   - Its prints say that three and four players are not supported.
 *   It then loads the battle's files and makes the lights, the camera ("dracam"), the cockpit
 *   (a model and its damage box), the boss's stage (files 0x143-0x147) and sky ("bgrad"), and
 *   the battle's other tasks: the shoulder, "Brother", the gauges, the filter, the storm.
 * - Runs (message 0):
 *   - Start opens the pause.
 *   - The stick aims (yaw -600..600, pitch -80..80).
 *   - The two punch buttons (gCockpitBtnStraight, gCockpitBtnJab) give the straight and the jab, which
 *     chain into combos, and a button sequence gives the sky kick. A third sequence gives the
 *     hundred punch, whose fists come from impStormCreate every 3 frames.
 *   - Both punch buttons held: the guard.
 *   - The shoot button (gCockpitBtnShoot) fires the vulcan, held, while money lasts. With the power
 *     gauge full, its sequences fire the cannon (the んが砲).
 *   - gCockpitBtnBaton throws the baton.
 *   - A hit on the damage box takes armour by the boss's attack (bossImpactHit): applied 5
 *     frames later, halved under the guard, and none while the cannon fires.
 * - Draws (message 3): the lights follow the battle's state, and the fog and the sky go by
 *   the boss.
 * - Freed (message 1): its frames and the stage's file.
 * Its debug prints (compiled out, the strings in .rodata) are written where their order puts
 * them. They name each boss attack that hits the cockpit; one of battle 5's eight hits has none,
 * and which one is a guess (0x71 here). */

typedef struct CockpitWork {
    /* 0x00 */ Camera *cam; /* "dracam" */
    /* 0x04 */ LightFrame *light; /* the scene's light */
    /* 0x08 */ LightFrame *light2; /* a second, black */
    /* 0x0C */ AmbFrame *amb;
    /* 0x10 */ CamView *view;
    /* 0x14 */ LightFrame *cpLight; /* the cockpit's own light (under gCockpitRootFrame) */
    /* 0x18 */ AmbFrame *cpAmb;
    /* 0x1C */ PolFrame *model; /* the cockpit */
    /* 0x20 */ ACollFrame *dmg; /* its damage box */
    /* 0x24 */ Frame *unused24; /* freed with the others (cockpitProc), but never made */
    /* 0x28 */ PolFrame *head; /* turned by the aim */
    /* 0x2C */ PolFrame *stage; /* the boss's stage */
    /* 0x30 */ ColorFade *sky; /* "bgrad" */
    /* 0x34 */ Gfx *dl; /* gFrmLayers[3]'s list, put back when freed */
    /* 0x38 */ u8 pad38[0x8];
    /* 0x40 */ Obj *pause;
    /* 0x44 */ f32 yaw;
    /* 0x48 */ f32 pitch;
    /* 0x4C */ f32 eye[3]; /* the camera's look-at point */
    /* 0x58 */ f32 spin; /* the low-armour light's angle, in degrees */
    /* 0x5C */ u32 ram; /* the stage's file */
} CockpitWork;

#define COCKPIT(obj) ((CockpitWork *) ((u32) (obj) + sizeof(Obj)))
#define PLAYER(i) (&gSaveData.players[i])
#define IN_PLAY(p) ((p)->chara >= 0 && (p)->lives > 0)
#define GONE(p) ((p)->chara == -1 || (p)->lives <= 0)
#define DEG_TO_RAD 0.0174532930038800785

/* A colour (three u8) stepped by d, or held at least or at most at (r, g, b). */
#define COLOR_ADD(c, d) \
    (c)[0] += (d);      \
    (c)[1] += (d);      \
    (c)[2] += (d)
#define COLOR_MIN(c, r, g, b) \
    if ((c)[0] < (r)) {       \
        (c)[0] = (r);         \
    }                         \
    if ((c)[1] < (g)) {       \
        (c)[1] = (g);         \
    }                         \
    if ((c)[2] < (b)) {       \
        (c)[2] = (b);         \
    }
#define COLOR_MAX(c, r, g, b) \
    if ((c)[0] >= (r)) {      \
        (c)[0] = (r);         \
    }                         \
    if ((c)[1] >= (g)) {      \
        (c)[1] = (g);         \
    }                         \
    if ((c)[2] >= (b)) {      \
        (c)[2] = (b);         \
    }
#define COLOR_SET(c, r, g, b) \
    (c)[0] = (r);             \
    (c)[1] = (g);             \
    (c)[2] = (b)

extern s8 gRetracesPerFrame; /* retraces per frame */
extern u32 gGameFrameCount; /* the frame counter */
extern Frame *gFrmLayers[]; /* the frame pools */
extern Frame *gFrmLayer8;
extern Frame *gFrmEffectLayer; /* the frame effects go under (gFrmLayers[9]) */
extern s16 gFrmKindPol; /* the "pol" kind */
extern s16 gFrmKindLight; /* the "light" kind */
extern s16 gFrmKindAmbi; /* the "ambi" kind */
extern s16 gFrmKindDlist; /* the "dlist" kind */
extern Frame *pl_dmg_root_frame; /* the damage boxes' parent */
extern Gfx *gGfxDlHead; /* the display list being built */
extern Obj *gEffectTask;
extern Gfx gDrawSetupLitTexEdgeDl[];
extern Gfx gDrawSetupXluSurfDl[];
extern Gfx gBossLitTexEnvDl[];
extern Gfx gBattleStageSetupDl[];
extern u16 gBattleSeqHundredPunch[]; /* the hundred punch's button sequence */
extern u16 gBattleSeqCannon0[]; /* the cannon's four */
extern u16 gBattleSeqCannon1[];
extern u16 gBattleSeqCannon2[];
extern u16 gBattleSeqCannon3[];
extern u16 gBattleSeqCannon4[]; /* a fifth cannon */
extern u16 gBattleSeqMegaCannon[]; /* the MEGA cannon's (at low armour) */
extern u16 gBattleSeqSkyKick[]; /* the sky kick's */
extern DLFrame *gCockpitRootFrame; /* the cockpit's frames' parent */
extern DLFrame *gCockpitPanelFrame; /* the panels' parent */
extern Camera *gBattleCam; /* the battle's camera task */
extern CamView *gBattleCamView; /* the battle's camera */
extern Obj *gCockpitTask0; /* the cockpit, for the battle's other files */
extern Obj *gCockpitTask1;
extern Obj *gCockpitTaskCopy3;
extern Obj *gCockpitTaskCopy4;
/* Lists and data in the battle's files (segment 3). */
extern u8 cockpitTaiStageDl[];
extern u8 cockpitTaiColl[];
extern u8 cockpitFriStageDl[];
extern u8 cockpitKasStageDl[];
extern u8 cockpitBisuStageDl[];
extern u8 cockpitFriColl[];
extern u8 cockpitBisuColl[];
extern u8 cockpitKasColl[];
extern u8 cockpitOmitsuDl[]; /* Omitsu's cockpit */
extern u8 cockpitGoemonDl[]; /* Goemon's */
extern u8 cockpitGoemonHeadDl[];
extern u8 cockpitOmitsuHeadDl[];
/* The bosses' tasks, at their files' TLB-mapped addresses (files 0x143-0x147: tlb_0323-0327). */

s32 gBattlePlayer1Chara = 0; /* the first player character, 0-3 */
s32 gBattlePlayer2Chara = 0; /* the second (the partner with one player) */
s32 gCockpitPilotChara = 0; /* the cockpit pilot character (the voice) */
s32 gBrotherPilotChara = 0; /* its pilot character, 0-3 (the voice) */


/* Matched 2026-10-04; it was parked for IDO's register allocation: the struct temp of the 29
 * floadLoadSet calls (msg 2) lives in s1, which the game sets before the first call, and the
 * frame was 8 bytes short of the game's 0x100.
 * The 38 calls the game makes through a register (jalr) are written `(*func)(...)`.
 * The two `if (1) {}` in msg 2 are block boundaries. The FileSet address range (29
 * floadLoadSet calls, one per uopt block) fits adjsave = 28 / (29 + floor(D / 12)), D its live
 * blocks with no reference. It must rank below the range that first takes s1 (msg 3, the
 * gGfxDlHead address: one block, 7 uses, 0.731). Without them D = 117: 28/38 = 0.737, coloured
 * first and split. Two give D = 121: 28/39 = 0.718, so it takes s1 whole, set before the first
 * call as in the game. SKY_BOSS and the dimming chain's `== 2U` are explained where they are. */
s32 cockpitProc(Obj *obj, s16 msg, void *arg) {
    CockpitWork *w = COCKPIT(obj);
    s32 pad[3]; /* puts x, z and i at the game's slots: 0x14, 0x18 and 0x20 below the frame's top */
    f32 x;
    f32 z;
    s32 j; /* the hundred punch's beat */
    s32 i;

    gCockpitTask0 = obj;
    gCockpitTask1 = obj;
    gCockpitTaskCopy3 = obj;
    gCockpitTaskCopy4 = obj;
    switch (msg) {
        case 2:
            obj->name = "Cockpit";
            gRetracesPerFrame = 1;
            gGameWork.partake = 1;
            frmDeleteKind(gFrmKindPol);
            polInitPolKind(80);
            tskCreate(obj->parent, scParentProc, 0, NULL);
            (*fadeCreate)(gEffectTask);
            w->dl = ((DLFrame *) gFrmLayers[3])->dl;
            ((DLFrame *) gFrmLayers[3])->dl = gBattleStageSetupDl;
            ((DLFrame *) gFrmLayers[2])->dl = gBossLitTexEnvDl;
            gCockpitRootFrame = (DLFrame *) frmCreate(gFrmLayers[9], gFrmKindDlist);
            gCockpitRootFrame->dl = gDrawSetupLitTexEdgeDl;
            gCockpitPanelFrame = (DLFrame *) frmCreate(gFrmEffectLayer, gFrmKindDlist);
            gCockpitPanelFrame->dl = gDrawSetupXluSurfDl;
            tskCreate(obj, attack_Proc, 0, NULL);
            vramCreate(obj);
            tskCreate(obj, effect, 0, NULL);
            gImpactArmor = 500;
            gImpactArmor2 = 500;
            gImpactMoney = 150;
            gBattleFlags = 0;
            gBrotherFlags = 0;
            gBossFlags = 0;
            gBattleSndToggles = 0;
            floadLoadSet(0xEA);
            floadLoadSet(0xEB);
            floadLoadSet(0xEC);
            floadLoadSet(0xEE);
            floadLoadSet(0xEF);
            floadLoadSet(0x125);
            floadLoadSet(0x121);
            floadLoadSet(0xF2);
            floadLoadSet(0x110);
            floadLoadSet(0x112);
            floadLoadSet(0x13E);
            floadLoadSet(0x13F);
            floadLoadSet(0x140);
            if (fadeCountPlayers() == 1) {
                DPRINTF("１人プレイと認識しました。\n");
                if (PLAYER(0)->chara == 0) {
                    DPRINTF("１コンキャラは[ゴエ]です。\n");
                    DPRINTF("自動的に２コンは[エビ]に設定します。\n");
                    gBattlePlayer1Chara = 0;
                    gCockpitPilotChara = 0;
                    gBattlePlayer2Chara = 1;
                    gBrotherPilotChara = 1;
                    if ((s32) arg == 1) {
                        DPRINTF("***おおっと***\n");
                        DPRINTF("ビスマル戦のみひとあじ違います。\n");
                        DPRINTF("おみつインパクトスタートになります。\n");
                        gBattleFlags |= 0x4000000;
                    }
                } else if (PLAYER(0)->chara == 1) {
                    DPRINTF("１コンキャラは[エビ]です。\n");
                    DPRINTF("自動的に２コンは[ゴエ]に設定します。\n");
                    gBattlePlayer1Chara = 1;
                    gCockpitPilotChara = 1;
                    gBattlePlayer2Chara = 0;
                    gBrotherPilotChara = 0;
                    if ((s32) arg == 1) {
                        DPRINTF("***おおっと***\n");
                        DPRINTF("ビスマル戦のみひとあじ違います。\n");
                        DPRINTF("おみつインパクトスタートになります。\n");
                        gBattleFlags |= 0x4000000;
                    }
                } else if (PLAYER(0)->chara == 2) {
                    DPRINTF("１コンキャラは[サス]です。\n");
                    DPRINTF("自動的に２コンは[ヤエ]に設定します。\n");
                    gBattlePlayer1Chara = 2;
                    gCockpitPilotChara = 2;
                    gBattlePlayer2Chara = 3;
                    gBrotherPilotChara = 3;
                    if ((s32) arg == 1) {
                        DPRINTF("***おおっと***\n");
                        DPRINTF("ビスマル戦のみひとあじ違います。\n");
                        DPRINTF("２コンは[ゴエ]に再設定されます。\n");
                        gBattlePlayer2Chara = 0;
                        gBrotherPilotChara = 0;
                        DPRINTF("おみつインパクトスタートになります。\n");
                        gBattleFlags |= 0x4000000;
                    }
                } else if (PLAYER(0)->chara == 3) {
                    DPRINTF("１コンキャラは[ヤエ]です。\n");
                    DPRINTF("自動的に２コンは[サス]に設定します。\n");
                    gBattlePlayer1Chara = 3;
                    gCockpitPilotChara = 3;
                    gBattlePlayer2Chara = 2;
                    gBrotherPilotChara = 2;
                }
                DPRINTF("２Ｐオンリーチェックを開始します。\n");
                if (GONE(PLAYER(0))) {
                    DPRINTF("なんと！１Ｐ死んでるやん。めんどっちーなーもー。\n");
                    if (GONE(PLAYER(2))) {
                        DPRINTF("なんと！３Ｐ死んでるやん。めんどっちーなーもー。\n");
                        if (GONE(PLAYER(3))) {
                            DPRINTF("なんと！４Ｐ死んでるやん。めんどっちーなーもー。\n");
                            DPRINTF("２Ｐオンリーチェック結果[コックピットコントローラーは２コンです]\n");
                            gCockpitPort = 1;
                            DPRINTF("続いてキャラの再設定を行います。\n");
                            if (PLAYER(1)->chara == 0) {
                                DPRINTF("２コンキャラは[ゴエ]です。\n");
                                DPRINTF("１コンは[エビ]に再設定します。\n");
                                gBattlePlayer2Chara = 0;
                                gCockpitPilotChara = 0;
                                gBattlePlayer1Chara = 1;
                                gBrotherPilotChara = 1;
                            } else if (PLAYER(1)->chara == 1) {
                                DPRINTF("２コンキャラは[エビ]です。\n");
                                DPRINTF("１コンは[ゴエ]に再設定します。\n");
                                gBattlePlayer2Chara = 1;
                                gCockpitPilotChara = 1;
                                gBattlePlayer1Chara = 0;
                                gBrotherPilotChara = 0;
                            } else if (PLAYER(1)->chara == 2) {
                                DPRINTF("２コンキャラは[サス]です。\n");
                                DPRINTF("１コンは[ヤエ]に再設定します。\n");
                                gBattlePlayer2Chara = 2;
                                gCockpitPilotChara = 2;
                                gBattlePlayer1Chara = 3;
                                gBrotherPilotChara = 3;
                                if ((s32) arg == 1) {
                                    DPRINTF("***おおっと***\n");
                                    DPRINTF("ビスマル戦のみひとあじ違います。\n");
                                    DPRINTF("相方はゴエモンになります。\n");
                                    gBattlePlayer1Chara = 0;
                                    gBrotherPilotChara = 0;
                                }
                            } else if (PLAYER(1)->chara == 3) {
                                DPRINTF("２コンキャラは[ヤエ]です。\n");
                                DPRINTF("１コンは[サス]に再設定します。\n");
                                gBattlePlayer2Chara = 3;
                                gCockpitPilotChara = 3;
                                gBattlePlayer1Chara = 2;
                                gBrotherPilotChara = 2;
                            }
                        }
                    }
                }
                DPRINTF("２Ｐオンリーチェックを終了します。\n");
                DPRINTF("３Ｐオンリーチェックを開始します。\n");
                if (GONE(PLAYER(0))) {
                    DPRINTF("なんと！１Ｐ死んでるやん。めんどっちーなーもー。\n");
                    if (GONE(PLAYER(1))) {
                        DPRINTF("なんと！２Ｐ死んでるやん。めんどっちーなーもー。\n");
                        if (GONE(PLAYER(3))) {
                            DPRINTF("なんと！４Ｐ死んでるやん。めんどっちーなーもー。\n");
                            DPRINTF("３Ｐオンリーチェック結果[コックピットコントローラーは３コンです]\n");
                            gCockpitPort = 2;
                        }
                    }
                }
                DPRINTF("３Ｐオンリーチェックを終了します。\n");
                DPRINTF("４Ｐオンリーチェックを開始します。\n");
                if (GONE(PLAYER(0))) {
                    DPRINTF("なんと！１Ｐ死んでるやん。めんどっちーなーもー。\n");
                    if (GONE(PLAYER(1))) {
                        DPRINTF("なんと！２Ｐ死んでるやん。めんどっちーなーもー。\n");
                        if (GONE(PLAYER(2))) {
                            DPRINTF("なんと！３Ｐ死んでるやん。めんどっちーなーもー。\n");
                            DPRINTF("４Ｐオンリーチェック結果[コックピットコントローラーは４コンです]\n");
                            gCockpitPort = 3;
                        }
                    }
                }
                DPRINTF("４Ｐオンリーチェックを終了します。\n");
            } else if (fadeCountPlayers() >= 2) {
                DPRINTF("他人数プレイと認識しました。\n");
                if (PLAYER(0)->chara == 0) {
                    DPRINTF("１コンキャラは[ゴエ]です。\n");
                    gBattlePlayer1Chara = 0;
                    gCockpitPilotChara = 0;
                } else if (PLAYER(0)->chara == 1) {
                    DPRINTF("１コンキャラは[エビ]です。\n");
                    gBattlePlayer1Chara = 1;
                    gCockpitPilotChara = 1;
                } else if (PLAYER(0)->chara == 2) {
                    DPRINTF("１コンキャラは[サス]です。\n");
                    gBattlePlayer1Chara = 2;
                    gCockpitPilotChara = 2;
                } else if (PLAYER(0)->chara == 3) {
                    DPRINTF("１コンキャラは[ヤエ]です。\n");
                    gBattlePlayer1Chara = 3;
                    gCockpitPilotChara = 3;
                }
                if (PLAYER(1)->chara == 0) {
                    DPRINTF("２コンキャラは[ゴエ]です。\n");
                    gBattlePlayer2Chara = 0;
                    gBrotherPilotChara = 0;
                } else if (PLAYER(1)->chara == 1) {
                    DPRINTF("２コンキャラは[エビ]です。\n");
                    gBattlePlayer2Chara = 1;
                    gBrotherPilotChara = 1;
                } else if (PLAYER(1)->chara == 2) {
                    DPRINTF("２コンキャラは[サス]です。\n");
                    gBattlePlayer2Chara = 2;
                    gBrotherPilotChara = 2;
                } else if (PLAYER(1)->chara == 3) {
                    DPRINTF("２コンキャラは[ヤエ]です。\n");
                    gBattlePlayer2Chara = 3;
                    gBrotherPilotChara = 3;
                }
                if (fadeCountPlayers() == 3) {
                    DPRINTF("３人対応はしていません。\n");
                }
                if (fadeCountPlayers() == 4) {
                    DPRINTF("４人対応はしていません。\n");
                }
            } else {
                DPRINTF("プレイヤーが設定されていません。\n");
            }
            w->cam = (Camera *) tskCreate(obj, dracam, 0, NULL);
            /* FAKE: OoT's `if (1) {}`, twice: block boundaries (no code) inside the FileSet temp's
             * live range, so it ranks below the range that first takes s1 (see the note). */
            if (1) {}
            camSetFov(w->cam, 1.0471976f);
            if (1) {}
            camSetSpeed(w->cam, 8.0f);
            camSetEye(w->cam, 0.0f, 0.0f, 0.0f);
            camSetAt(w->cam, 0.0f, 0.0f, -300.0f);
            w->view = camGetView(w->cam);
            w->view->zFar = 22000.0f;
            gBattleCam = w->cam;
            gBattleCamView = w->view;
            w->light = (LightFrame *) frmCreate(gFrmLayers[0], gFrmKindLight);
            COLOR_SET(w->light->dir, 0x7F, 0x7F, 0x7F);
            fadeLightNormalize(w->light);
            COLOR_SET(w->light->color, 0xF0, 0xF0, 0xF0);
            w->light2 = (LightFrame *) frmCreate(gFrmLayers[0], gFrmKindLight);
            COLOR_SET(w->light2->dir, 0x7F, 0x7F, 0x7F);
            fadeLightNormalize(w->light2);
            COLOR_SET(w->light2->color, 0, 0, 0);
            w->amb = (AmbFrame *) frmCreate(gFrmLayers[0], gFrmKindAmbi);
            COLOR_SET(w->amb->color, 0x60, 0x60, 0x60);
            w->cpLight = (LightFrame *) frmCreate(&gCockpitRootFrame->f, gFrmKindLight);
            w->cpLight->lightNum = 1;
            COLOR_SET(w->cpLight->color, 0xFF, 0xFF, 0xFF);
            fadeLightNormalize(w->cpLight);
            w->cpAmb = (AmbFrame *) frmCreate(&gCockpitRootFrame->f, gFrmKindAmbi);
            COLOR_SET(w->cpAmb->color, 0x60, 0x60, 0x60);
            w->model = (PolFrame *) frmCreate(&gCockpitRootFrame->f, gFrmKindPol);
            if (gBattleFlags & 0x4000000) {
                bossSetPolDL(w->model, cockpitOmitsuDl, 0xEA);
            } else {
                bossSetPolDL(w->model, cockpitGoemonDl, 0xEA);
            }
            w->model->flags |= 0x20;
            w->dmg = acollDmgAddYZ(pl_dmg_root_frame, &w->model->f, 100, 200, -100, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 0xC9;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 1.0f;
            w->model->pos[0] = 0.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = 0.0f;
            w->head = (PolFrame *) frmCreateNext(gFrmLayer8, gFrmKindPol);
            bossSetPolDL(w->head, cockpitGoemonHeadDl, 0xEA);
            w->head->flags |= 0x20;
            w->head->pos[0] = 0.0f;
            w->head->pos[1] = 0.0f;
            w->head->pos[2] = 0.0f;
            w->head->scale[0] = w->head->scale[1] = w->head->scale[2] = 1.0f;
            if ((s32) arg == 1) {
                gBattleBossId = 1;
                (*bossBattleStart)(gBattleBossId);
                floadLoadSet(0xF4);
                floadLoadSet(0xF6);
                floadLoadSet(0xF3);
                w->ram = (u32) heapMallocPage(fileRamSize(0x143));
                fileLoadNow(0x143, (void *) w->ram);
                w->stage = (PolFrame *) frmCreate(gFrmLayers[3], gFrmKindPol);
                bossSetPolDL(w->stage, cockpitBisuStageDl, 0xF4);
                w->stage->flags |= 0x20;
                vramLoadColl((s32) cockpitBisuColl, 0x2CA);
                tskCreate(obj, Bbisu_proc, 0x143, NULL);
                w->sky = (ColorFade *) tskCreate(obj, bgrad, 0, NULL);
                camBgradSetLines(w->sky, 36, 72);
                camBgradSetTop(w->sky, 0, 100, 255);
                camBgradSetBottom(w->sky, 255, 255, 255);
            } else if ((s32) arg == 2) {
                gBattleBossId = 2;
                (*bossBattleStart)(gBattleBossId);
                floadLoadSet(0xF8);
                floadLoadSet(0xED);
                floadLoadSet(0xF6);
                floadLoadSet(0xF3);
                floadLoadSet(0xF9);
                w->ram = (u32) heapMallocPage(fileRamSize(0x144));
                fileLoadNow(0x144, (void *) w->ram);
                w->stage = (PolFrame *) frmCreate(gFrmLayers[3], gFrmKindPol);
                bossSetPolDL(w->stage, cockpitTaiStageDl, 0xF8);
                w->stage->flags |= 0x20;
                vramLoadColl((s32) cockpitTaiColl, 0x2D0);
                w->sky = (ColorFade *) tskCreate(obj, bgrad, 0, NULL);
                camBgradSetLines(w->sky, 36, 100);
                camBgradSetTop(w->sky, 0, 50, 128);
                camBgradSetBottom(w->sky, 0, 0, 40);
                tskCreate(obj, btaiProc, 0x144, NULL);
            } else if ((s32) arg == 3) {
                gBattleBossId = 3;
                (*bossBattleStart)(gBattleBossId);
                floadLoadSet(0xFB);
                w->ram = (u32) heapMallocPage(fileRamSize(0x145));
                fileLoadNow(0x145, (void *) w->ram);
                w->stage = (PolFrame *) frmCreate(gFrmLayers[3], gFrmKindPol);
                bossSetPolDL(w->stage, cockpitFriStageDl, 0xFB);
                w->stage->flags |= 0x20;
                vramLoadColl((s32) cockpitFriColl, 0x2D3);
                w->sky = (ColorFade *) tskCreate(obj, bgrad, 0, NULL);
                tskCreate(obj, bfriProc, 0x145, NULL);
            } else if ((s32) arg == 4) {
                gBattleBossId = 4;
                (*bossBattleStart)(gBattleBossId);
                floadLoadSet(0xF5);
                w->ram = (u32) heapMallocPage(fileRamSize(0x146));
                fileLoadNow(0x146, (void *) w->ram);
                tskCreate(obj, Bkas_proc, 0x146, NULL);
                w->stage = (PolFrame *) frmCreate(gFrmLayers[3], gFrmKindPol);
                w->stage->flags |= 0x20;
                vramLoadColl((s32) cockpitKasColl, 0x2CB);
                w->sky = (ColorFade *) tskCreate(obj, bgrad, 0, NULL);
                camBgradSetLines(w->sky, 36, 100);
                camBgradSetTop(w->sky, 30, 30, 30);
                camBgradSetBottom(w->sky, 255, 255, 255);
            } else if ((s32) arg == 5) {
                gBattleBossId = 5;
                (*bossBattleStart)(gBattleBossId);
                floadLoadSet(0xF4);
                floadLoadSet(0xF0);
                floadLoadSet(0xF1);
                floadLoadSet(0xD8);
                w->ram = (u32) heapMallocPage(fileRamSize(0x147));
                fileLoadNow(0x147, (void *) w->ram);
                vramLoadColl((s32) cockpitBisuColl, 0x2CA);
                tskCreate(obj, Bdev_proc, 0x147, NULL);
                tskCreate(obj, BbgBoss5, 0x147, NULL);
                w->sky = (ColorFade *) tskCreate(obj, bgrad, 0, NULL);
            } else if ((s32) arg == 6) {
                gBattleBossId = 6;
                floadLoadSet(0xF4);
                floadLoadSet(0xF5);
                w->ram = (u32) heapMallocPage(fileRamSize(0x146));
                fileLoadNow(0x146, (void *) w->ram);
                w->stage = (PolFrame *) frmCreate(gFrmLayers[3], gFrmKindPol);
                bossSetPolDL(w->stage, cockpitBisuStageDl, 0xF4);
                w->stage->flags |= 0x20;
                vramLoadColl((s32) cockpitBisuColl, 0x2CA);
                w->sky = (ColorFade *) tskCreate(obj, bgrad, 0, NULL);
                camBgradSetLines(w->sky, 36, 100);
                camBgradSetTop(w->sky, 30, 30, 30);
                camBgradSetBottom(w->sky, 255, 255, 255);
                tskCreate(obj, vc, 0, NULL);
            }
            tskCreate(obj, shoulder, 0, NULL);
            if (gBattleBossId == 4) {
                tskCreate(obj, BrotherWater, 0, NULL);
            } else {
                tskCreate(obj, Brother, 0, NULL);
            }
            tskCreate(obj, Pgage, 0, NULL);
            tskCreate(obj, money, 0, NULL);
            tskCreate(obj, fuel, 0, NULL);
            tskCreate(obj, bosslife, 0, NULL);
            tskCreate(obj, bePfilterProc, 0, NULL);
            tskCreate(obj, Astorm, 0, NULL);
            break;

        case 1:
            frmDestroy(&w->light->f);
            frmDestroy(&w->light2->f);
            frmDestroy(&w->cpLight->f);
            frmDestroy(&w->amb->f);
            frmDestroy(&w->cpAmb->f);
            frmDestroy(&w->view->f);
            frmDestroy(&w->model->f);
            frmDestroy(w->unused24);
            frmDestroy((Frame *) w->dmg);
            frmDestroy(&w->head->f);
            frmDestroy(&w->stage->f);
            ((DLFrame *) gFrmLayers[3])->dl = w->dl;
            gCockpitRootFrame->dl = NULL;
            gCockpitPanelFrame->dl = NULL;
            ((DLFrame *) gFrmLayers[2])->dl = NULL;
            gBattleFlags2 &= ~0x4000000;
            while (gfxGetPendingSwaps() != 0) {
            }
            resFreeAddr(w->ram);
            w->ram = 0;
            break;

        case 0:
            if (w->pause == NULL) {
                if (!(gBattleFlags2 & 0x200000)) {
                    for (i = 0; i < 4; i++) {
                        if (IN_PLAY(PLAYER(i)) && joyGetHeldTime(i, 0x1000) == 1) {
                            w->pause = tskCreate(obj->parent, bpauseProc, 0, NULL);
                        }
                    }
                }
            } else {
                if (!(w->pause->flags & 1) || w->pause->proc != bpauseProc) {
                    w->pause = NULL;
                }
                return 0;
            }
            if ((gImpactArmor <= 0 || gImpactArmor2 <= 0) && !(gBattleFlags & 2)) {
                DPRINTF("インパクト死亡しました\n");
                gBossFlags |= 0x2000;
                gBattleFlags |= 2;
                (*sndPlay)(0x8176);
                (*sndPlay)(0x51A);
                (*sndPlay)(0x10E);
            }
            /* The pilots: Omitsu's Impact swaps them. */
            if (fadeCountPlayers() == 1) {
                if (gBattleFlags & 0x4000000) {
                    gCockpitPilotChara = gBattlePlayer2Chara;
                    gBrotherPilotChara = gBattlePlayer1Chara;
                } else {
                    gCockpitPilotChara = gBattlePlayer1Chara;
                    gBrotherPilotChara = gBattlePlayer2Chara;
                }
            }
            if (fadeCountPlayers() >= 2) {
                if (gBattleFlags & 0x4000000) {
                    gCockpitPort = 1;
                    gBrotherPort = 0;
                    gCockpitPilotChara = gBattlePlayer2Chara;
                    gBrotherPilotChara = gBattlePlayer1Chara;
                } else {
                    gCockpitPort = 0;
                    gBrotherPort = 1;
                    gCockpitPilotChara = gBattlePlayer1Chara;
                    gBrotherPilotChara = gBattlePlayer2Chara;
                }
            }
            /* Low armour (below 100) for the warning light and the MEGA cannon. */
            if (gBattleFlags & 0x4000000) {
                if (gImpactArmor2 < 100) {
                    gBattleFlags |= 0x8000000;
                } else {
                    gBattleFlags &= ~0x8000000;
                }
            } else if (gImpactArmor < 100) {
                gBattleFlags |= 0x8000000;
            } else {
                gBattleFlags &= ~0x8000000;
            }
            if (gBattleFlags & 0x4000000) {
                bossSetPolDL(w->model, cockpitOmitsuDl, 0xEA);
            } else {
                bossSetPolDL(w->model, cockpitGoemonDl, 0xEA);
            }
            if (gBattleFlags & 0x4000000) {
                bossSetPolDL(w->head, cockpitOmitsuHeadDl, 0xEA);
            } else {
                bossSetPolDL(w->head, cockpitGoemonHeadDl, 0xEA);
            }
            /* The aim. */
            if (gBattleFlags2 & 0x800) {
                w->yaw = 0.0f;
                w->pitch = 0.0f;
                gCockpitHeadPitch = w->head->rot[0] = -(w->pitch * DEG_TO_RAD * 0.2);
                gCockpitHeadYaw = w->head->rot[1] = 3.1415927f - w->yaw * DEG_TO_RAD * 0.2;
                gBattleFlags2 &= ~0x800;
            }
            if (!(gBattleFlags & 0x1000) && !(gBattleFlags2 & 0x400)) {
                if (gBattleFlags & 0x20000000) {
                    w->yaw += joyGetStickX(gCockpitPort, 0) / 8;
                    w->pitch -= joyGetStickY(gCockpitPort, 0) / 8;
                    gCockpitHeadPitch = w->head->rot[0] = -(w->pitch * DEG_TO_RAD * 0.2);
                    gCockpitHeadYaw = w->head->rot[1] = 3.1415927f - w->yaw * DEG_TO_RAD * 0.2;
                } else {
                    w->yaw += joyGetStickX(gCockpitPort, 0) / 8;
                    w->pitch += joyGetStickY(gCockpitPort, 0) / 8;
                    gCockpitHeadPitch = w->head->rot[0] = -(w->pitch * DEG_TO_RAD * 0.2);
                    gCockpitHeadYaw = w->head->rot[1] = 3.1415927f - w->yaw * DEG_TO_RAD * 0.2;
                }
            }
            if (w->yaw > 600.0f) {
                w->yaw = 600.0f;
            }
            if (w->yaw < -600.0f) {
                w->yaw = -600.0f;
            }
            if (w->pitch > 80.0f) {
                w->pitch = 80.0f;
            }
            if (w->pitch < -80.0f) {
                w->pitch = -80.0f;
            }
            /* The second Impact's aim. Its dead zone's test can never be true (a game bug). */
            gBrotherAimY += joyGetStickY(gBrotherPort, 0) * 0.125;
            if (gBrotherAimY > 100.0f) {
                gBrotherAimY = 100.0f;
            }
            if (gBrotherAimY < -100.0f) {
                gBrotherAimY = -100.0f;
            }
            if (joyGetStickY(gBrotherPort, 0) >= -9 &&
                joyGetStickY(gBrotherPort, 0) < 10) {
                if (gBrotherAimY < -2.0f && gBrotherAimY > 2.0f) {
                    gBrotherAimY = 0.0f;
                } else if (gBrotherAimY >= 0.0f) {
                    gBrotherAimY -= 1.0f;
                } else {
                    gBrotherAimY += 1.0f;
                }
            }
            gCockpitAimX = sinf(w->yaw * 3.1415927f * 0.00055555) * 100.0f;
            gCockpitAimY = sinf(w->pitch * 3.1415927f * 0.00055555) * 100.0f;
            w->model->rot[1] = -(w->yaw * DEG_TO_RAD * 0.1);
            gCockpitRotX = w->model->rot[0];
            gCockpitRotY = w->model->rot[1];
            gCockpitRotZ = w->model->rot[2];
            /* The field of view: 40 degrees near the boss (1500 away), else 50. */
            if (bossDistSq(gBossPosX, gBossPosY, gBossPosZ,
                                       gImpactPosX, gImpactPosY, gImpactPosZ) <
                2250000.0f) {
                camMoveFov(w->cam, 0.69813175f);
            } else {
                camMoveFov(w->cam, 0.87266463f);
            }
            if (gBattleFlags & 0x400000) {
                camMoveFov(w->cam, 0.69813175f);
                gBattleFlags |= 0x1000;
                gCockpitShake = 0;
            }
            /* The attack under way. Flag 0x1000 holds the aim while one runs. */
            if (!(gBattleFlags & 0x400000) && !(gBattleFlags2 & 0x400)) {
                if (gBattleFlags & 0x10) {
                    gBattleFlags |= 0x1000;
                    camMoveFov(w->cam, 0.82030475f);
                }
                if (gBattleFlags & 0x2000000) {
                    gBattleFlags |= 0x1000;
                    camMoveFov(w->cam, 0.82030475f);
                    if (gImpactMoveTime == 0) {
                        gBattleFlags &= ~0x2000000;
                        gBattleFlags &= ~0x1000;
                        gCockpitShake = 0;
                    }
                }
                if (gBattleFlags & 4) {
                    /* The hundred punch. */
                    gBattleFlags |= 0x1000;
                    camMoveFov(w->cam, 0.75049162f);
                    gCockpitShake = 5;
                    if ((gBattleFlags2 & 0x2000) && !(gBattleFlags2 & 0x4000) &&
                        tskCreate(obj, bappcamHundredPunch, 0, NULL) != NULL) {
                        DPRINTF("百烈がボスにヒット。タイマーリセットします。\n");
                        gImpactMoveTime = 300;
                        gBattleFlags2 |= 0x4000;
                        gBattleFlags2 &= ~0x2000;
                        if (gBattleFlags & 0x4000000) {
                            (*sndPlay)(0x136);
                        } else {
                            (*sndPlay)(0x135);
                        }
                    }
                    if (!(gBattleFlags & 0x200000) &&
                        joyGetHeldTime(gCockpitPort, gCockpitBtnStraight) == 1) {
                        DPRINTF("百烈天空キックが入力されました。\n");
                        gBattleFlags |= 0x200000;
                    }
                    if (gImpactMoveTime > 90) {
                        gBattleFlags |= 0x40000000;
                        j = gImpactMoveTime % 60;
                        if (gBattleFlags2 & 0x4000) {
                            /* On the boss: a fist every 3 frames. */
                            if (j == 0) {
                                impStormCreate(0);
                            } else if (j == 6) {
                                impStormCreate(0);
                            } else if (j == 12) {
                                impStormCreate(5);
                            } else if (j == 18) {
                                impStormCreate(2);
                            } else if (j == 24) {
                                impStormCreate(3);
                            } else if (j == 30) {
                                impStormCreate(4);
                            } else if (j == 36) {
                                impStormCreate(1);
                            } else if (j == 42) {
                                impStormCreate(2);
                            } else if (j == 48) {
                                impStormCreate(5);
                            } else if (j == 54) {
                                impStormCreate(4);
                            } else if (j == 3) {
                                impStormCreate(10);
                            } else if (j == 9) {
                                impStormCreate(7);
                            } else if (j == 15) {
                                impStormCreate(8);
                            } else if (j == 21) {
                                impStormCreate(11);
                            } else if (j == 33) {
                                impStormCreate(10);
                            } else if (j == 27) {
                                impStormCreate(6);
                            } else if (j == 39) {
                                impStormCreate(6);
                            } else if (j == 45) {
                                impStormCreate(11);
                            } else if (j == 51) {
                                impStormCreate(8);
                            } else if (j == 57) {
                                impStormCreate(9);
                            }
                        } else if (j == 0) {
                            impStormCreate(0);
                        } else if (j == 6) {
                            impStormCreate(0);
                        } else if (j == 12) {
                            impStormCreate(5);
                        } else if (j == 18) {
                            impStormCreate(2);
                        } else if (j == 24) {
                            impStormCreate(3);
                        } else if (j == 30) {
                            impStormCreate(4);
                        } else if (j == 36) {
                            impStormCreate(1);
                        } else if (j == 42) {
                            impStormCreate(2);
                        } else if (j == 48) {
                            impStormCreate(5);
                        } else if (j == 54) {
                            impStormCreate(4);
                        }
                    } else {
                        gCockpitShake = 0;
                        gBattleFlags &= ~0x40000000;
                    }
                    if ((gBattleFlags & 0x200000) && gImpactMoveTime == 85 &&
                        tskCreate(obj, kick_proc2, 0, NULL) != NULL) {
                        DPRINTF("コマンド百烈天空キック発動\n");
                        gImpactMoveTime = 90;
                        gBattleFlags &= ~4;
                        gBattleFlags &= ~0x1000;
                        gBattleFlags2 &= ~0x2000;
                        gBattleFlags2 &= ~0x4000;
                        camMoveFov(w->cam, 0.95993114f);
                        gCockpitShake = 0;
                        bossRumble(0, 0xBBBBBBBB, 10);
                    }
                    if (gImpactMoveTime == 0) {
                        DPRINTF("百烈パンチ終了しました。\n");
                        gBattleFlags &= ~4;
                        gBattleFlags &= ~0x1000;
                        gBattleFlags2 &= ~0x2000;
                        gBattleFlags2 &= ~0x4000;
                        gCockpitShake = 0;
                    }
                } else if (gBattleFlags & 0x10000) {
                    /* The sky kick. */
                    gBattleFlags |= 0x1000;
                    camMoveFov(w->cam, 0.95993114f);
                    if (gImpactMoveTime == 0) {
                        DPRINTF("生天空キック終了しました。\n");
                        gBattleFlags &= ~0x10000;
                        gBattleFlags &= ~0x1000;
                        gCockpitShake = 0;
                    }
                } else if (gBattleFlags & 0x40) {
                    /* A jab: the cockpit turns with it, and a punch now chains a combo. */
                    gBattleFlags |= 0x1000;
                    if (gImpactMoveTime >= 10) {
                        w->model->rot[1] -= (18 - gImpactMoveTime) * DEG_TO_RAD / 3.0;
                    } else {
                        w->model->rot[1] -= gImpactMoveTime * DEG_TO_RAD / 3.0;
                    }
                    gCockpitRotY = w->model->rot[1];
                    if (!(gBattleFlags & 0x20000) && !(gBattleFlags & 0x800000) &&
                        joyGetHeldTime(gCockpitPort, gCockpitBtnJab) == 1) {
                        DPRINTF("コンボジャブが入力されました。\n");
                        gBattleFlags |= 0x20000;
                    } else if (!(gBattleFlags & 0x800000) && !(gBattleFlags & 0x20000) &&
                               joyGetHeldTime(gCockpitPort, gCockpitBtnStraight) == 1) {
                        DPRINTF("コンボストレート２が入力されました。\n");
                        gBattleFlags |= 0x800000;
                    }
                    if (gBattleFlags & 0x20000) {
                        if (gImpactMoveTime == 0 &&
                            tskCreate(obj, Jab, 0, NULL) != NULL) {
                            DPRINTF("コンボジャブ発動\n");
                            gImpactMoveTime = 18;
                            gBattleFlags &= ~0x40;
                            bossRumble(0, 0x88888888, 5);
                        }
                    } else if ((gBattleFlags & 0x800000) && gImpactMoveTime == 0 &&
                               tskCreate(obj, str_proc, 0, NULL) != NULL) {
                        DPRINTF("コンボストレート２発動\n");
                        gImpactMoveTime = 48;
                        gBattleFlags &= ~0x40;
                        bossRumble(0, 0xBBBBBBBB, 5);
                    }
                    if (gImpactMoveTime == 0) {
                        DPRINTF("ジャブ終了しました。\n");
                        gBattleFlags &= ~0x40;
                        gBattleFlags &= ~0x1000;
                    }
                } else if (gBattleFlags & 0x20000) {
                    /* The combo jab. */
                    gBattleFlags |= 0x1000;
                    if (gImpactMoveTime >= 10) {
                        w->model->rot[1] -= (18 - gImpactMoveTime) * DEG_TO_RAD / 3.0;
                    } else {
                        w->model->rot[1] -= gImpactMoveTime * DEG_TO_RAD / 3.0;
                    }
                    gCockpitRotY = w->model->rot[1];
                    if (!(gBattleFlags & 0x80000) && !(gBattleFlags & 0x100000) &&
                        joyGetHeldTime(gCockpitPort, gCockpitBtnStraight) == 1) {
                        DPRINTF("コンボストレートが入力されました。\n");
                        gBattleFlags |= 0x80000;
                    } else if (!(gBattleFlags & 0x100000) && !(gBattleFlags & 0x80000) &&
                               joyGetHeldTime(gCockpitPort, gCockpitBtnJab) == 1) {
                        DPRINTF("コンボジャブ２が入力されました。\n");
                        gBattleFlags |= 0x100000;
                    }
                    if (gBattleFlags & 0x80000) {
                        if (gImpactMoveTime == 0 &&
                            tskCreate(obj, str_proc, 0, NULL) != NULL) {
                            DPRINTF("コンボストレート発動\n");
                            gImpactMoveTime = 48;
                            gBattleFlags &= ~0x20000;
                            bossRumble(0, 0xBBBBBBBB, 5);
                        }
                    } else if ((gBattleFlags & 0x100000) && gImpactMoveTime == 0 &&
                               tskCreate(obj, Jab, 0, NULL) != NULL) {
                        DPRINTF("コンボジャブ２発動\n");
                        gImpactMoveTime = 18;
                        gBattleFlags &= ~0x20000;
                        bossRumble(0, 0x88888888, 5);
                    }
                    if (gImpactMoveTime == 0) {
                        DPRINTF("コンボジャブ終了しました。\n");
                        gBattleFlags &= ~0x20000;
                        gBattleFlags &= ~0x1000;
                    }
                } else if (gBattleFlags & 0x100000) {
                    /* The second combo jab: a punch now gives the combo sky kick. */
                    gBattleFlags |= 0x1000;
                    if (gImpactMoveTime >= 10) {
                        w->model->rot[1] -= (18 - gImpactMoveTime) * DEG_TO_RAD / 3.0;
                    } else {
                        w->model->rot[1] -= gImpactMoveTime * DEG_TO_RAD / 3.0;
                    }
                    gCockpitRotY = w->model->rot[1];
                    if (!(gBattleFlags & 0x40000) &&
                        joyGetHeldTime(gCockpitPort, gCockpitBtnStraight) == 1) {
                        DPRINTF("コンボが天空キック入力されました。\n");
                        gBattleFlags |= 0x40000;
                    }
                    if ((gBattleFlags & 0x40000) && gImpactMoveTime == 0 &&
                        tskCreate(obj, kick_proc, 0, NULL) != NULL) {
                        DPRINTF("コンボ天空キック発動\n");
                        gImpactMoveTime = 90;
                        gBattleFlags &= ~0x100000;
                        bossRumble(0, 0xBBBBBBBB, 10);
                    }
                    if (gImpactMoveTime == 0) {
                        DPRINTF("コンボジャブ２終了しました。\n");
                        gBattleFlags &= ~0x100000;
                        gBattleFlags &= ~0x1000;
                    }
                } else if (gBattleFlags & 0x80000) {
                    /* The combo straight: the cockpit turns the other way. */
                    gBattleFlags |= 0x1000;
                    if (gImpactMoveTime >= 25) {
                        w->model->rot[1] += (48 - gImpactMoveTime) * DEG_TO_RAD / 3.0;
                    } else {
                        w->model->rot[1] += gImpactMoveTime * DEG_TO_RAD / 3.0;
                    }
                    gCockpitRotY = w->model->rot[1];
                    if (gImpactMoveTime == 0) {
                        DPRINTF("コンボストレート終了しました。\n");
                        gBattleFlags &= ~0x80000;
                        gBattleFlags &= ~0x1000;
                    }
                } else if (gBattleFlags & 0x800000) {
                    /* The second combo straight. */
                    gBattleFlags |= 0x1000;
                    if (gImpactMoveTime >= 25) {
                        w->model->rot[1] += (48 - gImpactMoveTime) * DEG_TO_RAD / 3.0;
                    } else {
                        w->model->rot[1] += gImpactMoveTime * DEG_TO_RAD / 3.0;
                    }
                    gCockpitRotY = w->model->rot[1];
                    if (gImpactMoveTime == 0) {
                        DPRINTF("コンボストレート２終了しました。\n");
                        gBattleFlags &= ~0x800000;
                        gBattleFlags &= ~0x1000;
                    }
                } else if (gBattleFlags & 0x80) {
                    /* A straight. */
                    gBattleFlags |= 0x1000;
                    if (gImpactMoveTime >= 25) {
                        w->model->rot[1] += (48 - gImpactMoveTime) * DEG_TO_RAD / 3.0;
                    } else {
                        w->model->rot[1] += gImpactMoveTime * DEG_TO_RAD / 3.0;
                    }
                    gCockpitRotY = w->model->rot[1];
                    if (gImpactMoveTime == 0) {
                        DPRINTF("ストレート終了しました。\n");
                        gBattleFlags &= ~0x80;
                        gBattleFlags &= ~0x1000;
                    }
                } else if (gBattleFlags & 0x40000) {
                    /* The combo sky kick. */
                    gBattleFlags |= 0x1000;
                    camMoveFov(w->cam, 0.95993114f);
                    if (gImpactMoveTime == 0) {
                        DPRINTF("百烈天空キック終了しました。\n");
                        gBattleFlags &= ~0x40000;
                        gBattleFlags &= ~0x1000;
                        gCockpitShake = 0;
                    }
                } else if (gBattleFlags & 0x200000) {
                    /* The hundred punch's sky kick. */
                    gBattleFlags |= 0x1000;
                    camMoveFov(w->cam, 0.95993114f);
                    if (gImpactMoveTime == 0) {
                        DPRINTF("百烈天空キック２終了しました。\n");
                        gBattleFlags &= ~0x200000;
                        gBattleFlags &= ~0x1000;
                        gCockpitShake = 0;
                    }
                }
            }
            /* Both punch buttons held: the guard. */
            if (gImpactMoveTime == 0 && !(gBattleFlags & 0x400000) &&
                joyGetHeldTime(gCockpitPort, gCockpitBtnStraight) != 0 &&
                joyGetHeldTime(gCockpitPort, gCockpitBtnJab) != 0 &&
                !(gBattleFlags2 & 0x400) && gBossLife > 0 &&
                tskCreate(obj, Guard, 0, NULL) != NULL) {
                gBattleFlags |= 0x400000;
            }
            /* A new attack. */
            if (gImpactMoveTime == 0 && !(gBattleFlags & 0x400000) && !(gBattleFlags2 & 0x400)) {
                if (joyGetHeldTime(gCockpitPort, gCockpitBtnStraight) == 1) {
                    if (gBossLife > 0) {
                        if (bossCheckButtonSeq(gCockpitPort, gBattleSeqHundredPunch, 60, 1) != 0) {
                            gCockpitShake = 5;
                            gImpactMoveTime = 300;
                            gBattleFlags |= 4;
                        } else if (bossCheckButtonSeq(gCockpitPort, gBattleSeqSkyKick, 30, 0) != 0) {
                            if (tskCreate(obj, kick_proc, 0, NULL) != NULL) {
                                gImpactMoveTime = 90;
                                gBattleFlags |= 0x10000;
                                bossRumble(0, 0xBBBBBBBB, 10);
                            }
                        } else if (tskCreate(obj, str_proc, 0, NULL) != NULL) {
                            gImpactMoveTime = 48;
                            gBattleFlags |= 0x80;
                            bossRumble(0, 0xBBBBBBBB, 5);
                        }
                    }
                } else if (joyGetHeldTime(gCockpitPort, gCockpitBtnJab) == 1) {
                    if (!(gBattleFlags2 & 0x400) && gBossLife > 0) {
                        if (bossCheckButtonSeq(gCockpitPort, gBattleSeqSkyKick, 30, 0) != 0) {
                            if (tskCreate(obj, kick_proc2, 0, NULL) != NULL) {
                                gImpactMoveTime = 90;
                                gBattleFlags |= 0x10000;
                                bossRumble(0, 0xBBBBBBBB, 10);
                            }
                        } else if (tskCreate(obj, Jab, 0, NULL) != NULL) {
                            gImpactMoveTime = 18;
                            gBattleFlags |= 0x40;
                            bossRumble(0, 0x88888888, 5);
                        }
                    }
                } else if (joyGetHeldTime(gCockpitPort, gCockpitBtnBaton) == 1) {
                    if (tskCreate(obj, Baton, 0, NULL) != NULL) {
                        /* The pilot's shout. */
                        if (gCockpitPilotChara == 0) {
                            (*sndPlay)(0x31B);
                        } else if (gCockpitPilotChara == 1) {
                            (*sndPlay)(0x318);
                        } else if (gCockpitPilotChara == 2) {
                            (*sndPlay)(0x321);
                        } else if (gCockpitPilotChara == 3) {
                            (*sndPlay)(0x31E);
                        }
                        gImpactMoveTime = 80;
                        bossRumble(0, 0x88888888, 10);
                    }
                } else if (joyGetHeldTime(gCockpitPort, gCockpitBtnShoot) == 1 &&
                           !(gBattleFlags & 0x10)) {
                    /* The cannons, with the power gauge full. */
                    if (bossCheckButtonSeq(gCockpitPort, gBattleSeqMegaCannon, 60, 1) != 0) {
                        if (gBossLife > 0 && (gBattleFlags & 0x8000000) &&
                            gImpactPower >= 100 &&
                            tskCreate(obj, MEGA, 0, NULL) != NULL) {
                            DPRINTF("めがんが砲発射します\n");
                            gImpactPower = 0;
                            gBattleFlags |= 0x10;
                            gBattleFlags2 |= 0x40000;
                        }
                    } else if (bossCheckButtonSeq(gCockpitPort, gBattleSeqCannon0, 60, 1) != 0 ||
                               bossCheckButtonSeq(gCockpitPort, gBattleSeqCannon1, 60, 1) != 0 ||
                               bossCheckButtonSeq(gCockpitPort, gBattleSeqCannon2, 60, 1) != 0 ||
                               bossCheckButtonSeq(gCockpitPort, gBattleSeqCannon3, 60, 1) != 0) {
                        if (gImpactPower >= 100 && gBossLife > 0 &&
                            tskCreate(obj, MEGA, 0, NULL) != NULL) {
                            DPRINTF("んが砲発射します\n");
                            gImpactPower = 0;
                            gBattleFlags |= 0x10;
                        }
                    } else if (bossCheckButtonSeq(gCockpitPort, gBattleSeqCannon4, 60, 1) != 0 &&
                               gImpactPower >= 100 && gBossLife > 0 &&
                               tskCreate(obj, MEGA, 0, NULL) != NULL) {
                        DPRINTF("んが砲発射します\n");
                        gImpactPower = 0;
                        gBattleFlags |= 0x10;
                        gBattleFlags2 |= 0x20000;
                    }
                }
            }
            /* The vulcan, held: a shot every 7 frames, from alternate guns, a ryo each. */
            if (!(gBattleFlags & 0x400000) && gImpactMoveTime == 0 &&
                joyGetHeldTime(gCockpitPort, gCockpitBtnShoot) % 7 == 1 &&
                gImpactMoney != 0 && !(gBattleFlags2 & 0x400) && !(gBattleFlags & 0x10)) {
                if (gBossLife > 0 &&
                    tskCreate(obj, Valcan, 0, (void *) (u32) gValcanSide) !=
                        NULL) {
                    gValcanSide ^= 1;
                    gImpactMoveTime = 3;
                    gBattleFlags |= 0x2000000;
                    bossAddMoney(-1);
                    bossRumble(0, 0x88000000, 5);
                }
            } else if (!(gBattleFlags & 0x400000) && gImpactMoveTime == 0 &&
                       joyGetHeldTime(gCockpitPort, gCockpitBtnShoot) == 1 &&
                       gImpactMoney == 0 && !(gBattleFlags2 & 0x400) &&
                       !(gBattleFlags & 0x10) && gBossLife > 0) {
                sndPlay(0x2E2); /* no money */
            }
            if (--gImpactMoveTime <= 0) {
                gImpactMoveTime = 0;
            }
            /* Hits on the cockpit, by the boss's attack: bossImpactHit(damage, attack, power). */
            if (acollDmgWasHit(w->dmg) != 0) {
                if (w->dmg->hitKind == 0x10) {
                    DPRINTF("コックピットは外部バルカンに当たりました。\n");
                    bossImpactHit(0, 0x10, 1);
                    if (!(gBattleFlags & 0x10)) {
                        SoundLocation2(0x228, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                               w->dmg->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                    }
                }
                if (gBattleBossId == 1) {
                    if (w->dmg->hitKind == 0x15) {
                        DPRINTF("コックピットはハートビームに当たりました。\n");
                        bossImpactHit(0x11, 0x15, 1);
                    } else if (w->dmg->hitKind == 0x17) {
                        DPRINTF("コックピットはハリテに当たりました。\n");
                        bossImpactHit(0x44, 0x17, 1);
                    } else if (w->dmg->hitKind == 0x19) {
                        DPRINTF("コックピットはキスに当たりました。\n");
                        bossImpactHit(0x55, 0x19, 1);
                    } else if (w->dmg->hitKind == 0x1B) {
                        DPRINTF("コックピットはトゲに当たりました。\n");
                        bossImpactHit(0x33, 0x1B, 1);
                    } else if (w->dmg->hitKind == 0x1D) {
                        DPRINTF("コックピットはバリアに当たりました。\n");
                        if (!(gBattleFlags & 0x10)) {
                            SoundLocation2(0x228, gBattleCamView, gBossPosX, gBossPosY,
                                                   gBossPosZ, 300.0f, 15000.0f, gBattleCamView->fovy);
                        }
                        bossImpactHit(0x55, 0x1D, 1);
                    } else if (w->dmg->hitKind == 0x1A) {
                        DPRINTF("コックピットはお釜体当りに当たりました。\n");
                        if (!(gBattleFlags & 0x10)) {
                            SoundLocation2(0x27E, gBattleCamView, gBossPosX, gBossPosY,
                                                   gBossPosZ, 0.0f, 0.0f, gBattleCamView->fovy);
                        }
                        bossImpactHit(0x33, 0x1A, 1);
                    }
                } else if (gBattleBossId == 2) {
                    if (w->dmg->hitKind == 0x30) {
                        DPRINTF("コックピットはウツボに当たりました。\n");
                        bossImpactHit(0x33, 0x30, 1);
                    } else if (w->dmg->hitKind == 0x29) {
                        DPRINTF("コックピットはマーブル弾に当たりました。\n");
                        bossImpactHit(0x11, 0x29, 1);
                    } else if (w->dmg->hitKind == 0x2D) {
                        DPRINTF("コックピットはスライド体当りに当たりました。\n");
                        bossImpactHit(0x33, 0x2D, 1);
                    } else if (w->dmg->hitKind == 0x32) {
                        DPRINTF("コックピットは頭突きに当たりました。\n");
                        bossImpactHit(0x33, 0x32, 1);
                    } else if (w->dmg->hitKind == 0x34) {
                        DPRINTF("コックピットはビリビリ棒に当たりました。\n");
                        bossImpactHit(0x22, 0x34, 1);
                    } else if (w->dmg->hitKind == 0x35) {
                        DPRINTF("コックピットは退散バリアーに当たりました。\n");
                        if (!(gBattleFlags & 0x10)) {
                            SoundLocation2(0x228, gBattleCamView, gBossPosX, gBossPosY,
                                                   gBossPosZ, 300.0f, 15000.0f, gBattleCamView->fovy);
                        }
                        bossImpactHit(8, 0x35, 1);
                    }
                } else if (gBattleBossId == 3) {
                    if (w->dmg->hitKind == 0x3E) {
                        DPRINTF("コックピットはビートばちに当たりました。\n");
                        bossImpactHit(0x22, 0x3E, 1);
                    } else if (w->dmg->hitKind == 0x3F) {
                        DPRINTF("コックピットはアフロ頭突きに当たりました。\n");
                        bossImpactHit(0x19, 0x40, 1); /* sic: the next attack's number */
                    } else if (w->dmg->hitKind == 0x40) {
                        DPRINTF("コックピットはビットに当たりました。\n");
                        bossImpactHit(0x33, 0x40, 1);
                    } else if (w->dmg->hitKind == 0x41) {
                        DPRINTF("コックピットはビットバリアーに当たりました。\n");
                        bossImpactHit(0x33, 0x41, 1);
                    } else if (w->dmg->hitKind == 0x43) {
                        DPRINTF("コックピットは旋風クラッシュに当たりました。\n");
                        if (!(gBattleFlags & 0x10)) {
                            SoundLocation2(0x228, gBattleCamView, gBossPosX, gBossPosY,
                                                   gBossPosZ, 300.0f, 15000.0f, gBattleCamView->fovy);
                        }
                        bossImpactHit(0x11, 0x43, 1);
                    } else if (w->dmg->hitKind == 0x44) {
                        DPRINTF("コックピットはお札に当たりました。\n");
                        bossImpactHit(0x19, 0x44, 1);
                    }
                } else if (gBattleBossId == 4) {
                    if (w->dmg->hitKind == 0x50) {
                        DPRINTF("コックピットは扇子に当たりました。\n");
                        bossImpactHit(0x22, 0x50, 1);
                    } else if (w->dmg->hitKind == 0x51) {
                        DPRINTF("コックピットは張り手[右]に当たりました。\n");
                        bossImpactHit(0x22, 0x51, 1);
                        sndPlay(0x228);
                    } else if (w->dmg->hitKind == 0x53) {
                        DPRINTF("コックピットは張り手[左]に当たりました。\n");
                        bossImpactHit(0x55, 0x53, 1);
                        if (!(gBattleFlags & 0x10)) {
                            if (!(gBattleSndToggles & 0x40)) {
                                SoundLocation2(0x417, gBattleCamView, gBossPosX, gBossPosY,
                                                       gBossPosZ, 0.0f, 0.0f, gBattleCamView->fovy);
                                gBattleSndToggles |= 0x40;
                            } else {
                                SoundLocation2(0x418, gBattleCamView, gBossPosX, gBossPosY,
                                                       gBossPosZ, 0.0f, 0.0f, gBattleCamView->fovy);
                                gBattleSndToggles &= ~0x40;
                            }
                        }
                    } else if (w->dmg->hitKind == 0x54) {
                        DPRINTF("コックピットはバリアに当たりました。\n");
                        bossImpactHit(0x44, 0x54, 1);
                        if (!(gBattleFlags & 0x10)) {
                            if (!(gBattleSndToggles & 0x40)) {
                                SoundLocation2(0x417, gBattleCamView, gBossPosX, gBossPosY,
                                                       gBossPosZ, 0.0f, 0.0f, gBattleCamView->fovy);
                                gBattleSndToggles |= 0x40;
                            } else {
                                SoundLocation2(0x418, gBattleCamView, gBossPosX, gBossPosY,
                                                       gBossPosZ, 0.0f, 0.0f, gBattleCamView->fovy);
                                gBattleSndToggles &= ~0x40;
                            }
                        }
                    } else if (w->dmg->hitKind == 0x55) {
                        DPRINTF("コックピットはぶっさしに当たりました。\n");
                        if (!(gBattleFlags & 0x10)) {
                            SoundLocation2(0x228, gBattleCamView, gBossPosX, gBossPosY,
                                                   gBossPosZ, 300.0f, 15000.0f, gBattleCamView->fovy);
                        }
                        bossImpactHit(0x22, 0x55, 1);
                    } else if (w->dmg->hitKind == 0x58) {
                        DPRINTF("コックピットはカブキ玉に当たりました。\n");
                        bossImpactHit(0x55, 0x58, 1);
                        if (!(gBattleFlags & 0x10)) {
                            SoundLocation2(0x27F, gBattleCamView, gBossPosX, gBossPosY,
                                                   gBossPosZ, 0.0f, 0.0f, gBattleCamView->fovy);
                            sndPlay(0x228);
                        }
                    } else if (w->dmg->hitKind == 0x56) {
                        DPRINTF("コックピットは鉄球に当たりました。\n");
                        bossImpactHit(0x55, 0x56, 1);
                        if (!(gBattleFlags & 0x10)) {
                            sndPlay(0x17C);
                            sndPlay(0x228);
                        }
                    }
                } else if (gBattleBossId == 5) {
                    if (w->dmg->hitKind == 0x6C) {
                        DPRINTF("コックピットは鬼火に当たりました。\n");
                        if (!(gBattleFlags & 0x10)) {
                            SoundLocation2(0x27F, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                                   w->dmg->hitPos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                        }
                        bossImpactHit(0x66, 0x6C, 1);
                    } else if (w->dmg->hitKind == 0x6E) {
                        DPRINTF("コックピットは袈裟斬りに当たりました。\n");
                        if (!(gBattleFlags & 0x10)) {
                            SoundLocation2(0x2A3, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                                   w->dmg->hitPos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                        }
                        bossImpactHit(0x55, 0x6E, 1);
                    } else if (w->dmg->hitKind == 0x67) {
                        DPRINTF("コックピットは回転斬りに当たりました。\n");
                        if (!(gBattleFlags & 0x10)) {
                            SoundLocation2(0x27F, gBattleCamView, gBossPosX, gBossPosY,
                                                   gBossPosZ, 0.0f, 0.0f, gBattleCamView->fovy);
                        }
                        bossImpactHit(0x66, 0x67, 1);
                    } else if (w->dmg->hitKind == 0x68) {
                        DPRINTF("コックピットは文字弾に当たりました。\n");
                        if (!(gBattleSndToggles & 0x100)) {
                            SoundLocation2(0x417, gBattleCamView, gBossPosX, gBossPosY,
                                                   gBossPosZ, 0.0f, 0.0f, gBattleCamView->fovy);
                            gBattleSndToggles |= 0x100;
                        } else {
                            SoundLocation2(0x418, gBattleCamView, gBossPosX, gBossPosY,
                                                   gBossPosZ, 0.0f, 0.0f, gBattleCamView->fovy);
                            gBattleSndToggles &= ~0x100;
                        }
                        bossImpactHit(0x33, 0x68, 1);
                    } else if (w->dmg->hitKind == 0x6A) {
                        DPRINTF("コックピットは死神バリアーに当たりました。\n");
                        bossImpactHit(0x44, 0x6A, 1);
                        if (!(gBattleFlags & 0x10)) {
                            SoundLocation2(0x17C, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                                   w->dmg->hitPos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                        }
                    } else if (w->dmg->hitKind == 0x70) {
                        DPRINTF("コックピットはインパクト弾に当たりました。\n");
                        if (!(gBattleFlags & 0x10)) {
                            SoundLocation2(0x228, gBattleCamView, gBossPosX, gBossPosY,
                                                   gBossPosZ, 300.0f, 15000.0f, gBattleCamView->fovy);
                        }
                        bossImpactHit(0x33, 0x70, 1);
                    } else if (w->dmg->hitKind == 0x71) {
                        bossImpactHit(0x55, 0x71, 1);
                        if (!(gBattleFlags & 0x10)) {
                            (*sndPlay)(0x243);
                        }
                    } else if (w->dmg->hitKind == 0x19) {
                        DPRINTF("コックピットはキスに当たりました。\n");
                        bossImpactHit(0x44, 0x19, 1);
                    }
                }
            }
            /* The hit lands 5 frames later (gImpactHitDelay counts down from 6). */
            if (--gImpactHitDelay < 0 && (gBattleFlags & 1)) {
                gImpactHitDelay = 0;
                gImpactHitDamage = 0;
                gCockpitShake = 0;
                gBattleFlags &= ~1;
            }
            if (gImpactHitDelay != 0 && (gBattleFlags & 1)) {
                camMoveFov(w->cam, 0.61086524f);
                gCockpitShake = 6;
                if (gImpactHitDelay == 5 && !(gBattleFlags & 0x400000)) {
                    if (gBattleFlags & 0x10) {
                        DPRINTF("んが砲無敵中\n");
                    } else {
                        tskCreate(obj, beFilterProc, 0, NULL);
                        tskCreate(obj, beEfilterProc, 0, NULL);
                        if ((gGameFrameCount & 3) == 0) {
                            (*beEffectPCreate)(fadeRandRange(-50, 50), fadeRandRange(-10, 10),
                                                   -50.0f, 0.1f, 0);
                            (*beEffectPCreate)(fadeRandRange(0, 50), fadeRandRange(-10, 10),
                                                   -50.0f, 0.1f, 1);
                            (*beEffectPCreate)(fadeRandRange(-50, 0), fadeRandRange(-10, 10),
                                                   -50.0f, 0.2f, 2);
                            (*beEffectPCreate)(fadeRandRange(0, 50), fadeRandRange(-10, 10),
                                                   -50.0f, 0.2f, 2);
                        } else if ((gGameFrameCount & 3) == 2) {
                            (*beEffectPCreate)(fadeRandRange(-50, 50), fadeRandRange(-10, 10),
                                                   -50.0f, 0.2f, 0);
                            (*beEffectPCreate)(fadeRandRange(-50, 50), fadeRandRange(-10, 10),
                                                   -50.0f, 0.1f, 0);
                            (*beEffectPCreate)(fadeRandRange(-50, 0), fadeRandRange(-10, 10),
                                                   -50.0f, 0.1f, 1);
                            (*beEffectPCreate)(fadeRandRange(0, 50), fadeRandRange(-10, 10),
                                                   -50.0f, 0.1f, 1);
                            (*beEffectPCreate)(fadeRandRange(0, 0), fadeRandRange(-10, 10),
                                                   -50.0f, 0.2f, 2);
                            (*beEffectPCreate)(fadeRandRange(0, 10), fadeRandRange(-10, 10),
                                                   -50.0f, 0.2f, 3);
                            (*beEffectPCreate)(fadeRandRange(-30, -20), fadeRandRange(-10, 10),
                                                   -50.0f, 0.2f, 3);
                            (*beEffectPCreate)(fadeRandRange(-40, -30), fadeRandRange(-10, 10),
                                                   -50.0f, 0.2f, 3);
                        }
                        bossRumble(0, 0xFFFF8888, 5);
                        sndPlay(0x283);
                        if (gBattleFlags & 0x4000000) {
                            gImpactArmor2 -= gImpactHitDamage;
                        } else {
                            gImpactArmor -= gImpactHitDamage;
                        }
                        DPRINTF("敵攻撃による装甲の減少は[%d], パワー[%d]\n", gImpactHitDamage, gImpactPower);
                    }
                } else if (gImpactHitDelay == 5) {
                    /* Under the guard: half. */
                    if (gBattleFlags & 0x4000000) {
                        gImpactArmor2 -= (s32) (gImpactHitDamage * 0.5);
                    } else {
                        gImpactArmor -= (s32) (gImpactHitDamage * 0.5);
                    }
                    DPRINTF("ガードオン:敵攻撃による装甲の減少は[%d], パワー[%d]\n", (s32) (gImpactHitDamage * 0.5),
                           gImpactPower);
                    bossRumble(0, 0x88888888, 5);
                    sndPlay(0x283);
                }
                if (gImpactArmor <= 0) {
                    gImpactArmor = 0;
                }
                if (gImpactArmor2 <= 0) {
                    gImpactArmor2 = 0;
                }
                if (gBossLife <= 0) {
                    gBossLife = 0;
                    gBattleFlags |= 0x1000;
                }
            }
            /* The camera: a slow bob, and its look-at point ahead of the head. */
            if (gBattleFlags2 & 0x1000000) {
                camMoveEye(w->cam, 0.0f, 0.0f, 0.0f);
            } else {
                camMoveEye(w->cam, 0.0f, sinf(gGameFrameCount * DEG_TO_RAD) * 3.0f, 0.0f);
            }
            bossHeadingOffset(w->head->rot[1], 0.0f, 300.0f, &w->eye[0], &w->eye[2]);
            w->eye[1] = w->pitch * DEG_TO_RAD * 18.0;
            gCockpitLookAtX = w->eye[0];
            gCockpitLookAtY = w->eye[1];
            gCockpitLookAtZ = w->eye[2];
            camMoveAt(w->cam, w->eye[0], w->eye[1], w->eye[2]);
            /* Shakes: 1 a bob, 5 a quiver (an attack, the end), 6 a jolt (a hit). */
            if (gBattleFlags2 & 0x400) {
                gCockpitShake = 0;
            }
            if (gBossFlags & 0x2000) {
                gCockpitShake = 5;
            }
            if (gCockpitShake != 0) {
                if (gCockpitShake == 1) {
                    if (gGameFrameCount % 20 < 10) {
                        camMoveAt(w->cam, w->eye[0], w->eye[1] + 10.0f, w->eye[2]);
                    } else {
                        camMoveAt(w->cam, w->eye[0], w->eye[1] - 10.0f, w->eye[2]);
                    }
                } else if (gCockpitShake == 5) {
                    if (gGameFrameCount % 10 < 5) {
                        camMoveAt(w->cam, (w->eye[0] - 5.0f) + fadeRand(10),
                                               w->eye[1] + fadeRand(10),
                                               (w->eye[2] - 5.0f) + fadeRand(10));
                    } else {
                        camMoveAt(w->cam, (w->eye[0] - 5.0f) + fadeRand(10),
                                               w->eye[1] - fadeRand(10),
                                               (w->eye[2] - 5.0f) + fadeRand(10));
                    }
                } else if (gCockpitShake == 6) {
                    if (gGameFrameCount % 10 < 5) {
                        camMoveAt(w->cam, (w->eye[0] - 20.0f) + fadeRand(40),
                                               w->eye[1] + 60.0f, w->eye[2]);
                    } else {
                        camMoveAt(w->cam, (w->eye[0] - 20.0f) + fadeRand(40),
                                               w->eye[1] - 60.0f, w->eye[2]);
                    }
                }
            }
            if (gBattleBossId == 4 && (gBattleFlags2 & 0x400000)) {
                gBattleFlags2 &= ~0x400000;
                bossSetPolDL(w->stage, cockpitKasStageDl, 0xF5);
            }
            break;

        case 3:
            if (gBattleFlags & 0x1000000) {
                w->model->f.flags &= ~2;
            } else {
                w->model->f.flags |= 2;
            }
            if ((gBattleFlags & 0x1000000) || (gBattleFlags & 0x10000000)) {
                w->head->f.flags &= ~2;
            } else {
                w->head->f.flags |= 2;
            }
            w->head->dl = gGfxDlHead;
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 255, 255);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 128);
            gSPEndDisplayList(gGfxDlHead++);
            /* Dimming (flag 0x800 of gBossFlags): toward each stage's darkest light.
             * FAKE: `== 2U` below. With a signed 2, IDO keeps the switch's case-2 constant in t0
             * from the dispatch down to this test, and then no longer gives the 2 of the second
             * player's check (msg 2) its own register (t3); every temporary after that shifts. */
            if (gBattleBossId == 1) {
                if (gBossFlags & 0x800) {
                    COLOR_ADD(w->light->color, -6);
                    COLOR_ADD(w->amb->color, -3);
                    COLOR_MIN(w->light->color, 0xFF, 0x33, 0x7A);
                    COLOR_MIN(w->amb->color, 0x30, 0x30, 0x30);
                }
            } else if (gBattleBossId == 2U) {
                if (gBossFlags & 0x800) {
                    COLOR_ADD(w->light->color, -6);
                    COLOR_ADD(w->amb->color, -3);
                    COLOR_MIN(w->light->color, 0x1E, 0x1E, 0x3C);
                    COLOR_MIN(w->amb->color, 0x10, 0x10, 0x30);
                }
            } else if (gBattleBossId == 4) {
                if (gBossFlags & 0x800) {
                    COLOR_ADD(w->light->color, -6);
                    COLOR_ADD(w->amb->color, -3);
                    COLOR_MIN(w->light->color, 0x37, 0x33, 0x16);
                    COLOR_MIN(w->amb->color, 0x10, 0x10, 0x10);
                }
            } else if (gBattleBossId == 5) {
                if (gBossFlags & 0x800) {
                    COLOR_ADD(w->light->color, -6);
                    COLOR_ADD(w->amb->color, -3);
                    COLOR_MIN(w->light->color, 0x37, 0x33, 0x16);
                    COLOR_MIN(w->amb->color, 0x10, 0x10, 0x10);
                }
            }
            /* The battle is over: the Impact shakes and smokes. */
            if (gBattleFlags & 2) {
                gBattleFlags2 |= 0x400;
                gBattleFlags2 |= 0x800;
                gBattleFlags |= 0x10000000;
                gCockpitShake = 5;
                if (!(gGameFrameCount & 7)) {
                    bossRumble(2, 0xF0F0F0F0, 8);
                }
                if (!(gBattleFlags2 & 0x4000000) && gGameFrameCount % 15 == 0) {
                    (*sndPlay)(0x513);
                }
                if (gGameFrameCount % 30 == 10) {
                    (*beEffectPCreate)(fadeRandRange(-50, 0), fadeRandRange(-10, 10), -50.0f,
                                           0.1f, 1);
                }
                if (gGameFrameCount % 30 == 25) {
                    (*beEffectPCreate)(fadeRandRange(0, 50), fadeRandRange(-10, 10), -50.0f,
                                           0.1f, 1);
                }
                if (gGameFrameCount % 5 == 0) {
                    (*beEffectPCreate)(fadeRandRange(0, 10), fadeRandRange(-20, -10), -50.0f,
                                           0.2f, 3);
                }
                if (gGameFrameCount % 5 == 0) {
                    (*beEffectPCreate)(fadeRandRange(-30, -20), fadeRandRange(-20, -10),
                                           -50.0f, 0.2f, 3);
                }
                if (gGameFrameCount % 5 == 0) {
                    (*beEffectPCreate)(fadeRandRange(-40, -30), fadeRandRange(-20, -10),
                                           -50.0f, 0.2f, 3);
                }
                if (gGameFrameCount % 30 == 0) {
                    (*beEffectPCreate)(20.0f, -10.0f, -60.0f, 0.2f, 2);
                }
                if (gGameFrameCount % 30 == 10) {
                    (*beEffectPCreate)(-10.0f, -13.0f, -70.0f, 0.2f, 2);
                }
                if (gGameFrameCount % 30 == 20) {
                    (*beEffectPCreate)(-5.0f, 17.0f, -80.0f, 0.2f, 2);
                }
            }
            if (gBattleFlags2 & 1) {
                /* Hit: every light darkens, toward red in Omitsu's Impact, else toward blue. */
                COLOR_SET(w->cpLight->dir, 0x7F, 0x7F, 0x7F);
                COLOR_ADD(w->light->color, -6);
                COLOR_ADD(w->cpLight->color, -6);
                COLOR_ADD(w->amb->color, -6);
                COLOR_ADD(w->cpAmb->color, -6);
                if (gBattleFlags & 0x4000000) {
                    COLOR_MIN(w->light->color, 0x50, 0x30, 0x30);
                    COLOR_MIN(w->cpLight->color, 0x30, 0x20, 0x20);
                    COLOR_MIN(w->amb->color, 0x60, 0x40, 0x40);
                    COLOR_MIN(w->cpAmb->color, 0x60, 0x30, 0x30);
                } else {
                    COLOR_MIN(w->light->color, 0x30, 0x30, 0x50);
                    COLOR_MIN(w->cpLight->color, 0x20, 0x20, 0x30);
                    COLOR_MIN(w->amb->color, 0x40, 0x40, 0x80);
                    COLOR_MIN(w->cpAmb->color, 0x30, 0x30, 0x60);
                }
            } else if (gBattleFlags & 0x8000000) {
                /* Low armour: a red warning light turns in the cockpit. */
                w->spin += 4.0f;
                if (w->spin >= 360.0f) {
                    w->spin -= 360.0f;
                }
                COLOR_ADD(w->amb->color, 3);
                COLOR_MAX(w->amb->color, 0x60, 0x60, 0x60);
                bossHeadingOffset(w->spin * DEG_TO_RAD, 0.0f, 100.0f, &x, &z);
                w->cpLight->dir[0] = x;
                w->cpLight->dir[1] = 0;
                w->cpLight->dir[2] = z;
                COLOR_SET(w->cpLight->color, 0xC0, 0x10, 0x10);
                COLOR_SET(w->cpAmb->color, 0x40, 0x40, 0x40);
                if (gImpactArmor > 0 && gImpactArmor2 > 0 && !(gBattleFlags2 & 0x400) &&
                    gGameFrameCount % 180 == 0) {
                    (*sndPlay)(0x30B);
                }
            } else {
                if (gBattleFlags2 & 0x100000) {
                    COLOR_SET(w->light->dir, 0, -0x7F, 0);
                    COLOR_SET(w->light->color, 0xFF, 0, 0);
                    COLOR_SET(w->amb->color, 0x80, 0x40, 0x40);
                } else {
                    /* Back toward each stage's light. */
                    COLOR_ADD(w->light->color, 3);
                    COLOR_ADD(w->amb->color, 3);
                    COLOR_ADD(w->cpLight->color, 3);
                    COLOR_ADD(w->cpAmb->color, 3);
                    if (gBattleBossId == 2) {
                        COLOR_MAX(w->light->color, 0xA0, 0xA0, 0xA0);
                        COLOR_MAX(w->cpLight->color, 0x30, 0x30, 0x40);
                        COLOR_MAX(w->amb->color, 0x40, 0x40, 0x40);
                        COLOR_MAX(w->cpAmb->color, 0x20, 0x20, 0x40);
                    } else if (gBattleBossId == 4) {
                        COLOR_MAX(w->light->color, 0xF0, 0xF0, 0xF0);
                        COLOR_MAX(w->cpLight->color, 0x60, 0x60, 0x60);
                        COLOR_MAX(w->amb->color, 0xA0, 0xA0, 0xA0);
                        COLOR_MAX(w->cpAmb->color, 0x30, 0x30, 0x30);
                    } else if (gBattleBossId == 5) {
                        COLOR_SET(w->light->dir, 0, -0x7F, 0);
                        COLOR_MAX(w->light->color, 0xF0, 0xF0, 0xF0);
                        COLOR_MAX(w->cpLight->color, 0x60, 0x60, 0x60);
                        COLOR_MAX(w->amb->color, 0x60, 0x60, 0x60);
                        COLOR_MAX(w->cpAmb->color, 0x30, 0x30, 0x30);
                    } else {
                        COLOR_MAX(w->light->color, 0xF0, 0xF0, 0xF0);
                        COLOR_MAX(w->cpLight->color, 0x60, 0x60, 0x60);
                        COLOR_MAX(w->amb->color, 0x60, 0x60, 0x60);
                        COLOR_MAX(w->cpAmb->color, 0x30, 0x30, 0x30);
                    }
                }
            }
            /* The sky's gradient and the stage's fog, by the boss and the battle's state. */
            if (SKY_BOSS == 1) {
                if (gBattleFlags2 & 0x1000) {
                    camBgradSetLines(w->sky, 50, 190);
                    camBgradMoveTop(w->sky, 0x40, 0, 0);
                    camBgradMoveBottom(w->sky, 10, 10, 10);
                    w->stage->dl = gGfxDlHead;
                    gDPSetFogColor(gGfxDlHead++, 10, 10, 10, 0x80);
                    gSPFogPosition(gGfxDlHead++, 995, 1000);
                    gSPEndDisplayList(gGfxDlHead++);
                } else if (gBattleFlags2 & 1) {
                    if (gBattleFlags & 0x4000000) {
                        camBgradSetLines(w->sky, 50, 190);
                        camBgradMoveTop(w->sky, 50, 20, 20);
                        camBgradMoveBottom(w->sky, 80, 20, 20);
                        w->stage->dl = gGfxDlHead;
                        gDPSetFogColor(gGfxDlHead++, 80, 20, 20, 0x80);
                        gSPFogPosition(gGfxDlHead++, 995, 1000);
                        gSPEndDisplayList(gGfxDlHead++);
                    } else {
                        camBgradSetLines(w->sky, 50, 190);
                        camBgradMoveTop(w->sky, 20, 20, 50);
                        camBgradMoveBottom(w->sky, 20, 20, 80);
                        w->stage->dl = gGfxDlHead;
                        gDPSetFogColor(gGfxDlHead++, 0, 0, 80, 0x80);
                        gSPFogPosition(gGfxDlHead++, 995, 1000);
                        gSPEndDisplayList(gGfxDlHead++);
                    }
                } else if (gBossFlags & 0x800) {
                    camBgradSetLines(w->sky, 36, 100);
                    camBgradMoveTop(w->sky, 30, 0, 30);
                    camBgradMoveBottom(w->sky, 80, 0, 80);
                    w->stage->dl = gGfxDlHead;
                    gDPSetFogColor(gGfxDlHead++, 80, 0, 80, 0x80);
                    gSPFogPosition(gGfxDlHead++, 995, 1000);
                    gSPEndDisplayList(gGfxDlHead++);
                } else {
                    camBgradSetLines(w->sky, 36, 72);
                    camBgradMoveTop(w->sky, 0, 100, 255);
                    camBgradMoveBottom(w->sky, 255, 255, 255);
                    w->stage->dl = gGfxDlHead;
                    gDPSetFogColor(gGfxDlHead++, 0x80, 0x80, 0x80, 0x80);
                    gSPFogPosition(gGfxDlHead++, 995, 1000);
                    gSPEndDisplayList(gGfxDlHead++);
                }
            } else if (SKY_BOSS == 2) {
                camBgradSetLines(w->sky, 36, 100);
                camBgradSetTop(w->sky, 0, 50, 128);
                camBgradSetBottom(w->sky, 0, 0, 40);
                w->stage->dl = gGfxDlHead;
                gDPPipeSync(gGfxDlHead++);
                gDPSetFogColor(gGfxDlHead++, 0, 0, 0x30, 0);
                gSPFogPosition(gGfxDlHead++, 994, 1000);
                gSPEndDisplayList(gGfxDlHead++);
            } else if (SKY_BOSS == 3) {
                if (gBattleFlags2 & 0x1000) {
                    camBgradSetLines(w->sky, 50, 190);
                    camBgradMoveTop(w->sky, 0x40, 0, 0);
                    camBgradMoveBottom(w->sky, 10, 10, 10);
                } else if (gBattleFlags2 & 1) {
                    if (gBattleFlags & 0x4000000) {
                        camBgradSetLines(w->sky, 50, 190);
                        camBgradMoveTop(w->sky, 50, 20, 20);
                        camBgradMoveBottom(w->sky, 80, 20, 20);
                    } else {
                        camBgradSetLines(w->sky, 50, 190);
                        camBgradMoveTop(w->sky, 20, 20, 50);
                        camBgradMoveBottom(w->sky, 20, 20, 80);
                    }
                } else if (gBossFlags & 0x800) {
                    camBgradSetLines(w->sky, 36, 100);
                    camBgradMoveTop(w->sky, 30, 0, 30);
                    camBgradMoveBottom(w->sky, 80, 0, 80);
                } else {
                    camBgradSetLines(w->sky, 36, 100);
                    camBgradMoveTop(w->sky, 0, 0, 20);
                    camBgradMoveBottom(w->sky, 50, 50, 0);
                }
                w->stage->dl = gGfxDlHead;
                gDPPipeSync(gGfxDlHead++);
                gDPSetFogColor(gGfxDlHead++, 0x18, 0x18, 0x18, 0);
                gSPFogPosition(gGfxDlHead++, 995, 1000);
                gSPEndDisplayList(gGfxDlHead++);
            }
            if (SKY_BOSS == 4) {
                if (gBattleFlags2 & 0x1000) {
                    camBgradSetLines(w->sky, 50, 190);
                    camBgradMoveTop(w->sky, 0x40, 0, 0);
                    camBgradMoveBottom(w->sky, 10, 10, 10);
                } else if (gBattleFlags2 & 1) {
                    if (gBattleFlags & 0x4000000) {
                        camBgradSetLines(w->sky, 50, 190);
                        camBgradMoveTop(w->sky, 50, 20, 20);
                        camBgradMoveBottom(w->sky, 80, 20, 20);
                    } else {
                        camBgradSetLines(w->sky, 50, 190);
                        camBgradMoveTop(w->sky, 20, 20, 50);
                        camBgradMoveBottom(w->sky, 20, 20, 80);
                    }
                } else if (gBossFlags & 0x800) {
                    camBgradSetLines(w->sky, 36, 100);
                    camBgradMoveTop(w->sky, 30, 0, 30);
                    camBgradMoveBottom(w->sky, 80, 0, 80);
                } else {
                    camBgradSetLines(w->sky, 36, 100);
                    camBgradMoveTop(w->sky, 10, 10, 10);
                    camBgradMoveBottom(w->sky, 10, 10, 10);
                }
                w->stage->dl = gGfxDlHead;
                gDPPipeSync(gGfxDlHead++);
                gDPSetFogColor(gGfxDlHead++, 0, 0, 0, 0);
                gSPFogPosition(gGfxDlHead++, 994, 1000);
                gSPEndDisplayList(gGfxDlHead++);
            }
            if (SKY_BOSS == 5) {
                if (gBattleFlags2 & 0x1000) {
                    camBgradSetLines(w->sky, 50, 190);
                    camBgradMoveTop(w->sky, 0x40, 0, 0);
                    camBgradMoveBottom(w->sky, 10, 10, 10);
                } else if (gBattleFlags2 & 1) {
                    if (gBattleFlags & 0x4000000) {
                        camBgradSetLines(w->sky, 50, 190);
                        camBgradMoveTop(w->sky, 50, 20, 20);
                        camBgradMoveBottom(w->sky, 80, 20, 20);
                    } else {
                        camBgradSetLines(w->sky, 50, 190);
                        camBgradMoveTop(w->sky, 20, 20, 50);
                        camBgradMoveBottom(w->sky, 20, 20, 80);
                    }
                } else if (gBossFlags & 0x800) {
                    camBgradSetLines(w->sky, 36, 100);
                    camBgradMoveTop(w->sky, 30, 0, 30);
                    camBgradMoveBottom(w->sky, 80, 0, 80);
                } else {
                    camBgradSetLines(w->sky, 50, 190);
                    camBgradMoveTop(w->sky, 0, 0, 0);
                    camBgradMoveBottom(w->sky, 0, 0, 20);
                }
            }
            if (SKY_BOSS == 6) {
                if (gBattleFlags2 & 0x1000) {
                    camBgradSetLines(w->sky, 50, 190);
                    camBgradMoveTop(w->sky, 0x40, 0, 0);
                    camBgradMoveBottom(w->sky, 10, 10, 10);
                } else if (gBattleFlags2 & 1) {
                    if (gBattleFlags & 0x4000000) {
                        camBgradSetLines(w->sky, 50, 190);
                        camBgradMoveTop(w->sky, 50, 20, 20);
                        camBgradMoveBottom(w->sky, 80, 20, 20);
                    } else {
                        camBgradSetLines(w->sky, 50, 190);
                        camBgradMoveTop(w->sky, 20, 20, 50);
                        camBgradMoveBottom(w->sky, 20, 20, 80);
                    }
                } else if (gBossFlags & 0x800) {
                    camBgradSetLines(w->sky, 36, 100);
                    camBgradMoveTop(w->sky, 30, 0, 30);
                    camBgradMoveBottom(w->sky, 80, 0, 80);
                } else {
                    camBgradSetLines(w->sky, 36, 100);
                    camBgradMoveTop(w->sky, 20, 20, 20);
                    camBgradMoveBottom(w->sky, 130, 50, 20);
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
