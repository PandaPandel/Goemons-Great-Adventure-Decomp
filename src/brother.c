#include "common.h"
#include "object.h"
#include "player.h"
#include "acoll.h"
#include "boss.h"
#include "aniatk.h"
#include "audio.h"
#include "bappcam.h"
#include "beffect.h"
#include "bmove.h"
#include "brother.h"
#include "fade.h"
#include "fileload.h"
#include "gfxmgr.h"
#include "impact.h"
#include "itex.h"
#include "shadow.h"
#include "sndloc.h"
#include "tmanager.h"
#include "vram.h"

/* "Brother": the second Impact in a boss battle, played by a second controller (gBrotherPort)
 * when more than one is in, else by itself. It circles the boss 4500 away, between 202.5 and
 * 337.5 degrees, punching, shooting the other side's vulcan ("Ovalcan") and jumping; struck,
 * it is knocked down and gets up again. Two of them: Goemon Impact's and Omitsu's. */

/* "Brother"'s work. */
typedef struct BrotherWork {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ ACollFrame *box; /* its attack chain */
    /* 0x18 */ ACollFrame *dmg; /* its damage box */
    /* 0x1C */ Obj *shadow;
    /* 0x20 */ u8 pad20[4];
    /* 0x24 */ u8 unused24; /* set to 1 when made, never read */
    /* 0x25 */ u8 step; /* while knocked down */
    /* 0x28 */ f32 angle; /* around the boss, in 1/40 degrees (8100-13500) */
    /* 0x2C */ f32 speed; /* the angle's step */
    /* 0x30 */ u8 pad30[0x10];
    /* 0x40 */ s32 alpha; /* the red glow while a life is low, 0-192 */
    /* 0x44 */ s32 fading;
} BrotherWork;

extern Frame *gFrmModelLayer;
extern s16 gFrmKindAnime; /* the "anime" kind */
extern Frame *uda_atk_root_frame; /* the other side's attack boxes' parent */
extern Frame *uda_dmg_root_frame; /* the damage boxes' parent */
extern CamView *gBattleCamView; /* the battle's camera */
extern Obj *gCockpitTask0;
extern Obj *gCockpitTask1;
extern u32 gGameFrameCount; /* GameState's frame count */
s32 gBrotherShoutWait = 0; /* frames until the next shout */
extern u8 brotherOmitsuAnim[]; /* Omitsu's Impact's animations (file 0xEA) */
extern u8 brotherOmitsuKnockDownAnim[];
extern u8 brotherOmitsuGetUpAnim[];
extern u8 brotherOmitsuJumpAnim[];
extern u8 brotherOmitsuPunchAnim[];
extern u8 brotherOmitsuThrownBackAnim[];
extern u8 brotherGoemonAnim[]; /* Goemon Impact's */
extern u8 brotherGoemonKnockDownAnim[];
extern u8 brotherGoemonGetUpAnim[];
extern u8 brotherGoemonJumpAnim[];
extern u8 brotherGoemonPunchAnim[];
extern u8 brotherGoemonThrownBackAnim[];
extern u8 brotherOmitsuAtkChain[]; /* their attack chains */
extern u8 brotherGoemonAtkChain[];
extern Frame *gFrmLayer2;
extern Frame *ene_dmg_root_frame; /* the water battle's damage boxes' parent */
extern u8 brotherWaterOmitsuAnim[]; /* the water battle's: Omitsu's Impact's animations (file 0xEA) */
extern u8 brotherWaterOmitsuJumpAnim[];
extern u8 brotherWaterOmitsuJumpShootAnim[];
extern u8 brotherWaterOmitsuKnockDownAnim[];
extern u8 brotherWaterOmitsuLandAnim[];
extern u8 brotherWaterOmitsuGetUpAnim[];
extern u8 brotherWaterOmitsuThrownBackAnim[];
extern u8 brotherWaterGoemonAnim[]; /* Goemon Impact's */
extern u8 brotherWaterGoemonJumpAnim[];
extern u8 brotherWaterGoemonJumpShootAnim[];
extern u8 brotherWaterGoemonKnockDownAnim[];
extern u8 brotherWaterGoemonLandAnim[];
extern u8 brotherWaterGoemonGetUpAnim[];
extern u8 brotherWaterGoemonThrownBackAnim[];
extern u8 brotherWaterOmitsuAtkChain[]; /* their attack chains */
extern u8 brotherWaterGoemonAtkChain[];

u16 joyGetHeldTime(s32 port, u16 mask); /* FAKE: joypad.h is not included here: see joyGetStickX */
s32 joyGetStickX(s32 port, s32 arg); /* FAKE: joypad.h is not included here: joyGetStickX: the game's code calls it as this file declares it; joypad.h has `s8 joyGetStickX(s32 port, s32 back)` */ /* a controller's stick x */

/* "Brother", Goemon Impact or Omitsu's by flag 0x4000000 of gBattleFlags. Not in battles 0 and
 * 6. Flags in gBrotherFlags: 4 jumping, 8 punching, 0x80 shooting, 0x20 knocked down, 0x40 in
 * the air, 0x400 playing itself, 0x4000 thrown back by the baton, 0x2000 put back.
 * By itself it picks a move (gBrotherMove) for a random time (gBrotherMoveTime): stand, circle either
 * way, jump, shoot "Ovalcan", punch; past 12000 or under 9600 it circles back for 300-360
 * frames; with the boss angry it goes for the Impact. Its damage box reports what struck it,
 * and the Impact takes the damage. */
s32 Brother(Obj *obj, s16 msg, void *arg) {
    BrotherWork *w = (BrotherWork *) ((u32) obj + sizeof(Obj));
    s32 pad0;
    Vec3f at;
    f32 t;

    switch (msg) {
        case 2:
            obj->name = "Brother";
            if (gBattleBossId == 0) {
                return -1;
            }
            if (gBattleBossId == 6) {
                return -1;
            }
            w->frame = (AnimeFrame *) frmCreateNext(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            w->shadow = shadowCreate();
            if (gBattleFlags & 0x4000000) {
                PRINTF("おみつスタートします。[%d]", gBattleBossId);
                bossSetAnimAt(w->frame, brotherOmitsuAnim, 0xEA, 0.0f);
                w->box = aniAtkCreate(uda_atk_root_frame, &w->frame->f, brotherOmitsuAtkChain,
                                                floadGetSetFile(0xEA, 0), 0);
                if (w->box == NULL) {
                    return -1;
                }
            } else {
                PRINTF("ゴエモンインパクトスタートします。[%d]", gBattleBossId);
                bossSetAnimAt(w->frame, brotherGoemonAnim, 0xEA, 0.0f);
                w->box = aniAtkCreate(uda_atk_root_frame, &w->frame->f, brotherGoemonAtkChain,
                                                floadGetSetFile(0xEA, 0), 0);
                if (w->box == NULL) {
                    return -1;
                }
            }
            w->frame->flags |= 0x20;
            w->box->kind = 0xCA;
            w->dmg = acollDmgAddYZ(uda_dmg_root_frame, &w->frame->f, 30, 200, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 0xCB;
            w->frame->scale[2] = 6.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->rot[0] = 0.0f;
            w->frame->pos[1] = 1000.0f;
            w->angle = 11500.0f;
            w->alpha = 0;
            w->fading = 0;
            w->unused24 = 1;
            w->vel[1] = 0.0f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            aniAtkDelete(w->box);
            frmDestroy(&w->dmg->frame);
            frmDestroy((Frame *) w->shadow);
            break;
        case 0:
            at.x = w->frame->pos[0];
            at.y = w->frame->pos[1];
            at.z = w->frame->pos[2];
            w->box->flags &= ~2;
            if (fadeCountPlayers() == 1) {
                gBrotherFlags |= 0x400;
            }
            gBrotherShoutWait--;
            if (gBrotherShoutWait <= 0) {
                gBrotherShoutWait = 0;
            }
            if (fadeCountPlayers() >= 2 && joyGetHeldTime(gBrotherPort, gBrotherBtnShout) == 1 &&
                gBossLife > 0 && gImpactArmor > 0 && gImpactArmor2 > 0 &&
                !(gBrotherFlags & 0x20) && gBrotherShoutWait == 0) {
                gBrotherShoutWait = 90;
                if (gBrotherPilotChara == 0) {
                    (*sndPlay)(0x31C);
                } else if (gBrotherPilotChara == 1) {
                    (*sndPlay)(0x319);
                } else if (gBrotherPilotChara == 2) {
                    (*sndPlay)(0x322);
                } else if (gBrotherPilotChara == 3) {
                    (*sndPlay)(0x31F);
                }
            }
            if (gBrotherFlags & 0x40000) {
                w->dmg->flags &= ~2;
                w->frame->pos[1] = 2500.0f;
                w->vel[1] = 1.0f;
            }
            if (gBrotherFlags & 0x80000) {
                gBrotherFlags &= ~0x80000;
                gBrotherFlags &= ~0x20000;
                gBrotherFlags |= 0x20;
            }
            if (gBattleFlags & 0x4000000) {
                if (gImpactArmor < 100 && !(gBrotherFlags & 0x20)) {
                    if (gGameFrameCount % 20 == 0) {
                        bePsmokeCreate(w->frame->pos[0], w->frame->pos[1] + 1000.0f, w->frame->pos[2], 2.0f, 0);
                    }
                    if (gGameFrameCount % 20 == 10) {
                        bePsmokeCreate(w->frame->pos[0], w->frame->pos[1] + 1000.0f, w->frame->pos[2], 1.0f, 0);
                    }
                }
            } else if (gImpactArmor2 < 100 && !(gBrotherFlags & 0x20)) {
                if (gGameFrameCount % 20 == 0) {
                    bePsmokeCreate(w->frame->pos[0], w->frame->pos[1] + 1000.0f, w->frame->pos[2], 2.0f, 0);
                }
                if (gGameFrameCount % 20 == 10) {
                    bePsmokeCreate(w->frame->pos[0], w->frame->pos[1] + 1000.0f, w->frame->pos[2], 1.0f, 0);
                }
            }
            if (gBattleBossId == 5) {
                if (!(gBrotherFlags & 0x20)) {
                    if (gGameFrameCount % 40 == 0) {
                        beSpeffectCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 0);
                    }
                    if (gGameFrameCount % 40 == 20) {
                        beSpeffectCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 0);
                    }
                }
            } else if (!(gBrotherFlags & 0x20) && !(gBrotherFlags & 0x80) &&
                       !(gBrotherFlags & 4) && !(gBrotherFlags & 8)) {
                if (gGameFrameCount % 20 == 0) {
                    bePsmokeCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f, 1);
                }
                if (gGameFrameCount % 20 == 10) {
                    bePsmokeCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f, 1);
                }
            }
            if (gBossFlags & 0x2000) {
                gBrotherFlags |= 0x20;
            }
            if (gBrotherFlags & 0x20000) {
                gBrotherFlags |= 0x20;
            }
            w->dmg->flags |= 2;
            if (gBattleFlags2 & 0x400) {
                w->dmg->flags &= ~2;
            }
            if (gBrotherFlags & 0x4000) {
                w->frame->rot[1] = 0.0f;
                if (gBattleFlags & 0x4000000) {
                    bossSetAnimAt(w->frame, brotherOmitsuThrownBackAnim, 0xEA, 0.0f);
                    aniAtkDelete(w->box);
                    w->box = aniAtkCreate(uda_atk_root_frame, &w->frame->f, brotherOmitsuAtkChain,
                                                    floadGetSetFile(0xEA, 0), 0);
                    if (w->box == NULL) {
                        return -1;
                    }
                } else {
                    bossSetAnimAt(w->frame, brotherGoemonThrownBackAnim, 0xEA, 0.0f);
                    aniAtkDelete(w->box);
                    w->box = aniAtkCreate(uda_atk_root_frame, &w->frame->f, brotherGoemonAtkChain,
                                                    floadGetSetFile(0xEA, 0), 0);
                    if (w->box == NULL) {
                        return -1;
                    }
                }
                w->box->kind = 0xCA;
                w->box->flags &= ~2;
                gBrotherFlags &= ~0x4000;
                if (gBrotherPilotChara == 0) {
                    (*sndPlay)(0x31A);
                } else if (gBrotherPilotChara == 1) {
                    (*sndPlay)(0x317);
                } else if (gBrotherPilotChara == 2) {
                    (*sndPlay)(0x320);
                } else if (gBrotherPilotChara == 3) {
                    (*sndPlay)(0x31D);
                }
            }
            if (gBrotherFlags & 0x2000) {
                w->box->flags &= ~2;
                if (gBattleFlags & 0x4000000) {
                    if (w->frame->pos[0] > 0.0f) {
                        w->angle = 10000.0f;
                    } else {
                        w->angle = 12000.0f;
                    }
                    bossSetAnimAt(w->frame, brotherOmitsuAnim, 0xEA, 0.0f);
                    aniAtkDelete(w->box);
                    w->box = aniAtkCreate(uda_atk_root_frame, &w->frame->f, brotherOmitsuAtkChain,
                                                    floadGetSetFile(0xEA, 0), 0);
                    if (w->box == NULL) {
                        return -1;
                    }
                } else {
                    if (w->frame->pos[0] > 0.0f) {
                        w->angle = 10000.0f;
                    } else {
                        w->angle = 12000.0f;
                    }
                    bossSetAnimAt(w->frame, brotherGoemonAnim, 0xEA, 0.0f);
                    aniAtkDelete(w->box);
                    w->box = aniAtkCreate(uda_atk_root_frame, &w->frame->f, brotherGoemonAtkChain,
                                                    floadGetSetFile(0xEA, 0), 0);
                    if (w->box == NULL) {
                        return -1;
                    }
                }
                w->box->flags &= ~2;
            }
            (*acollAtkWasHit)(w->box);
            if (acollDmgWasHit(w->dmg) != 0) {
                if (w->dmg->hitKind == 0xF) {
                    PRINTF("外部インパクトはバルカンに当たりました。\n");
                    itexSparkleYellow(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 0.6f);
                    SoundLocation2(0x228, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                    bossImpactDamage(0, 0xF, 1);
                } else if (w->dmg->hitKind == 0xCD) {
                    PRINTF("外部インパクトはボスに当たりました。\n");
                    SoundLocation2(0x27E, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                    bossImpactDamage(0, 0xCD, 1);
                    w->box->flags &= ~2;
                    w->dmg->flags &= ~2;
                } else if (w->dmg->hitKind == 0xB) {
                    PRINTF("外部インパクトはんが砲に当たりました。\n");
                    SoundLocation2(0x247, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                    bossImpactDamage(0, 0x14, 1);
                } else if (w->dmg->hitKind == 0xC) {
                    PRINTF("外部インパクトはバトンに当たりました。\n");
                    if (!(gBrotherFlags & 0x20000) && gBossLife >= 2 &&
                        tskCreate(gCockpitTask0, change, 0, NULL) != NULL) {
                        gBrotherFlags |= 0x4000;
                    }
                } else if (w->dmg->hitKind == 0x15) {
                    PRINTF("外部インパクトはハートビームに当たりました。\n");
                    SoundLocation2(0x247, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                    bossImpactDamage(0, 0x15, 1);
                } else if (w->dmg->hitKind == 0x1B) {
                    PRINTF("外部インパクトはトゲに当たりました。\n");
                    SoundLocation2(0x247, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                    bossImpactDamage(0, 0x1B, 1);
                } else if (w->dmg->hitKind == 0x29) {
                    PRINTF("外部インパクトはマーブル弾に当たりました。\n");
                    SoundLocation2(0x247, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                    bossImpactDamage(0, 0x29, 1);
                } else if (w->dmg->hitKind == 0x41) {
                    PRINTF("外部インパクトは風雷ビットに当たりました。\n");
                    SoundLocation2(0x247, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                    bossImpactDamage(0, 0x41, 1);
                } else if (w->dmg->hitKind == 0x51) {
                    PRINTF("外部インパクトはセンスに当たりました。\n");
                    SoundLocation2(0x247, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                    bossImpactDamage(0, 0x51, 1);
                } else if (w->dmg->hitKind == 0x56) {
                    PRINTF("外部インパクトはカブキ弾に当たりました。\n");
                    SoundLocation2(0x247, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                    bossImpactDamage(0, 0x56, 1);
                } else if (w->dmg->hitKind == 0x6A) {
                    PRINTF("外部インパクトは文字弾に当たりました。\n");
                    SoundLocation2(0x247, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                    bossImpactDamage(0, 0x6A, 1);
                } else if (w->dmg->hitKind == 0x6C) {
                    PRINTF("外部インパクトは鉄球に当たりました。\n");
                    SoundLocation2(0x27F, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                    bossImpactDamage(0, 0x6C, 1);
                } else if (w->dmg->hitKind == 0x6E) {
                    PRINTF("外部インパクトは鬼火に当たりました。\n");
                    SoundLocation2(0x247, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                    bossImpactDamage(0, 0x6E, 1);
                }
            }
            if ((gBrotherFlags & 0x400) && !(gBattleFlags2 & 0x400)) {
                gBrotherMoveTime--;
                if (gBrotherMoveTime < 0) {
                    gBrotherMoveTime = 0;
                }
                if (gBrotherMove != 0 && gBrotherMove != 1 && gBrotherMove != 2) {
                    if (gBrotherMove == 3) {
                        if (!(gBrotherFlags & 4) && !(gBrotherFlags & 8) &&
                            !(gBrotherFlags & 0x80) && !(gBrotherFlags & 0x20)) {
                            if (gBattleFlags & 0x4000000) {
                                bossSetAnimAt(w->frame, brotherOmitsuJumpAnim, 0xEA, 0.0f);
                            } else {
                                bossSetAnimAt(w->frame, brotherGoemonJumpAnim, 0xEA, 0.0f);
                            }
                            gBrotherFlags |= 4;
                            w->vel[1] = 40.0f;
                        } else if ((gBrotherFlags & 4) && !(gBrotherFlags & 0x80) &&
                                   !(gBrotherFlags & 0x20)) {
                            if (gBattleFlags & 0x4000000) {
                                bossSetAnimAt(w->frame, brotherOmitsuPunchAnim, 0xEA, 0.0f);
                            } else {
                                bossSetAnimAt(w->frame, brotherGoemonPunchAnim, 0xEA, 0.0f);
                            }
                            gBrotherFlags &= ~4;
                            gBrotherFlags |= 0x80;
                        }
                    } else if (gBrotherMove == 4) {
                        if (!(gBattleFlags2 & 0x4000) && gGameFrameCount % 20 == 0 && gImpactMoney != 0 &&
                            !(gBrotherFlags & 0x20) && !(gBattleFlags & 0x10) &&
                            tskCreate(gCockpitTask1, Ovalcan, 0,
                                                   (void *) (u32) gOvalcanSide) != NULL) {
                            gOvalcanSide ^= 1;
                            bossAddMoney(-1);
                            gBrotherFlags |= 1;
                        }
                    } else if (gBrotherMove == 5 && !(gBrotherFlags & 8) &&
                               !(gBrotherFlags & 0x80) && !(gBrotherFlags & 0x20)) {
                        if (gBattleFlags & 0x4000000) {
                            bossSetAnimAt(w->frame, brotherOmitsuPunchAnim, 0xEA, 0.0f);
                        } else {
                            bossSetAnimAt(w->frame, brotherGoemonPunchAnim, 0xEA, 0.0f);
                        }
                        gBrotherFlags |= 8;
                    }
                }
                if (gBrotherMoveTime == 0) {
                    if (w->angle > 12000.0f) {
                        gBrotherMove = 6;
                    } else if (w->angle < 9600.0f) {
                        gBrotherMove = 7;
                    } else if (gBattleFlags2 & 0x400) {
                        gBrotherMove = 0;
                    } else {
                        gBrotherMove = fadeRand(6);
                    }
                    if (gBrotherMove == 0) {
                        gBrotherMoveTime = fadeRandRange(30, 90);
                    } else if (gBrotherMove == 1) {
                        gBrotherMoveTime = fadeRandRange(60, 120);
                    } else if (gBrotherMove == 2) {
                        gBrotherMoveTime = fadeRandRange(60, 120);
                    } else if (gBrotherMove == 3) {
                        gBrotherMoveTime = fadeRandRange(10, 30);
                    } else if (gBrotherMove == 4) {
                        gBrotherMoveTime = fadeRandRange(30, 90);
                    } else if (gBrotherMove == 5) {
                        gBrotherMoveTime = fadeRandRange(15, 45);
                    } else if (gBrotherMove == 6) {
                        gBrotherMoveTime = fadeRandRange(300, 360);
                        gBrotherMove = 2;
                    } else if (gBrotherMove == 7) {
                        gBrotherMoveTime = fadeRandRange(300, 360);
                        gBrotherMove = 1;
                    }
                    if (gBossFlags & 0x4000) {
                        gBrotherMove = 8;
                    }
                }
            }
            if (!(gBattleFlags2 & 0x400) && !(gBrotherFlags & 0x20) && !(gBrotherFlags & 8) &&
                !(gBrotherFlags & 0x80) && !(gBattleFlags & 0x10)) {
                w->angle += joyGetStickX(gBrotherPort, 0) / 8;
                w->speed = joyGetStickX(gBrotherPort, 0) / 8;
            }
            if (w->angle > 13500.0f) {
                w->angle = 13500.0f;
            }
            if (w->angle < 8100.0f) {
                w->angle = 8100.0f;
            }
            w->frame->pos[0] = cosf(w->angle * 0.017453293003880078 * 0.025) * 4500.0f;
            w->frame->pos[2] = sinf(w->angle * 0.017453293003880078 * 0.025) * 4500.0f;
            if (!(gBrotherFlags & 0x20) || (gBrotherFlags & 0x200)) {
                if (gBrotherFlags & 0x400) {
                    if (gBrotherMove == 1 && !(gBrotherFlags & 0x20) && !(gBrotherFlags & 4) &&
                        !(gBrotherFlags & 0x80)) {
                        w->speed = 7.0f;
                        w->angle += 7.0f;
                    } else if (gBrotherMove == 2 && !(gBrotherFlags & 0x20) &&
                               !(gBrotherFlags & 4) && !(gBrotherFlags & 0x80)) {
                        w->angle -= 7.0f;
                        w->speed = -7.0f;
                    } else if (gBrotherMove == 8 && !(gBrotherFlags & 0x20) &&
                               !(gBrotherFlags & 4) && !(gBrotherFlags & 0x80)) {
                        bossTurnToward2(w->frame, gImpactPosX, gImpactPosZ, 10.0f);
                        if (w->angle < 11000.0f) {
                            w->angle -= 10.0f;
                            w->speed = -10.0f;
                            if (w->angle < 8100.0f) {
                                w->angle = 8100.0f;
                            }
                        } else {
                            w->speed = 10.0f;
                            w->angle += 10.0f;
                            if (w->angle > 13500.0f) {
                                w->angle = 13500.0f;
                            }
                        }
                    }
                }
                if (w->speed > -4.0f && w->speed < 4.0f) {
                    bossTurnToward2(w->frame, gBossPosX, gBossPosZ, 2.0f);
                } else if (w->speed >= 0.0f) {
                    bossTurnToward2(w->frame, 2000.0f, -800.0f, 2.0f);
                } else if (w->speed < 0.0f) {
                    bossTurnToward2(w->frame, -2000.0f, -800.0f, 2.0f);
                }
            }
            if (!(gBattleFlags2 & 0x400) && gBossLife > 0 && fadeCountPlayers() >= 2 &&
                joyGetHeldTime(gBrotherPort, gBrotherBtnShoot) % 10 == 1 &&
                gImpactMoney != 0 && !(gBrotherFlags & 0x20) && !(gBattleFlags & 0x10)) {
                if (tskCreate(gCockpitTask1, Ovalcan, 0, (void *) (u32) gOvalcanSide) !=
                    NULL) {
                    gOvalcanSide ^= 1;
                    gImpactMoney--;
                    gBrotherFlags |= 1;
                    (*bossRumble)(1, 0x88000000, 5);
                } else {
                    joyGetHeldTime(gBrotherPort, gBrotherBtnShoot);
                }
            }
            if ((gBrotherFlags & 0x80) && (gBrotherFlags & 0x40) && !(gBrotherFlags & 0x20)) {
                if (bossStepAnimAtk(w->frame, w->box, 0.6f, 2) != 0) {
                    w->box->flags &= ~2;
                } else if (!(gBattleFlags & 0x10)) {
                    w->box->flags |= 2;
                } else {
                    w->box->flags &= ~2;
                }
            } else if ((gBrotherFlags & 0x80) && !(gBrotherFlags & 0x40)) {
                if (!(gBattleFlags & 0x10)) {
                    w->box->flags |= 2;
                } else {
                    w->box->flags &= ~2;
                }
                if (!(gBrotherFlags & 0x100) && !(gBrotherFlags & 0x20)) {
                    if (gBattleBossId == 5) {
                        beSpeffectCreate(w->frame->pos[0], w->frame->pos[1] + 50.0f, w->frame->pos[2], 1);
                    } else {
                        beSmoke6Create(w->frame->pos[0], w->frame->pos[1] + 50.0f, w->frame->pos[2], 5.0f,
                                               400.0f, 0);
                    }
                    gBrotherFlags |= 0x100;
                }
                if (bossStepAnimAtk(w->frame, w->box, 0.6f, 2) != 0) {
                    w->box->flags &= ~2;
                    if (gBattleFlags & 0x4000000) {
                        bossSetAnimAt(w->frame, brotherOmitsuAnim, 0xEA, 0.0f);
                    } else {
                        bossSetAnimAt(w->frame, brotherGoemonAnim, 0xEA, 0.0f);
                    }
                    gBrotherFlags &= ~0x80;
                    gBrotherFlags &= ~0x100;
                }
            } else if ((gBrotherFlags & 4) && (gBrotherFlags & 0x40) && !(gBrotherFlags & 0x20)) {
                bossStepAnim(w->frame, 0.6f, 2);
            } else if ((gBrotherFlags & 4) && !(gBrotherFlags & 0x40) && !(gBrotherFlags & 0x20)) {
                if (bossStepAnim(w->frame, 0.6f, 2) != 0) {
                    if (gBattleFlags & 0x4000000) {
                        bossSetAnimAt(w->frame, brotherOmitsuAnim, 0xEA, 0.0f);
                    } else {
                        bossSetAnimAt(w->frame, brotherGoemonAnim, 0xEA, 0.0f);
                    }
                    if (!(gBrotherFlags & 0x20)) {
                        if (gBattleBossId == 5) {
                            beSpeffectCreate(w->frame->pos[0], w->frame->pos[1] + 50.0f, w->frame->pos[2], 1);
                        } else {
                            beSmoke6Create(w->frame->pos[0], w->frame->pos[1] + 50.0f, w->frame->pos[2], 5.0f,
                                                   400.0f, 0);
                        }
                        gBrotherFlags &= ~4;
                    }
                }
            } else if ((gBrotherFlags & 8) && !(gBrotherFlags & 0x20)) {
                if (!(gBattleFlags & 0x10)) {
                    w->box->flags |= 2;
                } else {
                    w->box->flags &= ~2;
                }
                if (bossStepAnimAtk(w->frame, w->box, 0.6f, 2) != 0) {
                    if (gBattleFlags & 0x4000000) {
                        bossSetAnimAt(w->frame, brotherOmitsuAnim, 0xEA, 0.0f);
                        aniAtkDelete(w->box);
                        w->box = aniAtkCreate(uda_atk_root_frame, &w->frame->f, brotherOmitsuAtkChain,
                                                        floadGetSetFile(0xEA, 0), 0);
                        if (w->box == NULL) {
                            return -1;
                        }
                    } else {
                        bossSetAnimAt(w->frame, brotherGoemonAnim, 0xEA, 0.0f);
                        aniAtkDelete(w->box);
                        w->box = aniAtkCreate(uda_atk_root_frame, &w->frame->f, brotherGoemonAtkChain,
                                                        floadGetSetFile(0xEA, 0), 0);
                        if (w->box == NULL) {
                            return -1;
                        }
                    }
                    w->box->kind = 0xCA;
                    w->box->flags &= ~2;
                    gBrotherFlags &= ~8;
                }
            }
            if (fadeCountPlayers() >= 2 && !(gBattleFlags2 & 0x400)) {
                if (joyGetHeldTime(gBrotherPort, gBrotherBtnJump) == 1 &&
                    !(gBrotherFlags & 4) && !(gBrotherFlags & 8) && !(gBrotherFlags & 0x80) &&
                    !(gBrotherFlags & 0x20)) {
                    if (gBattleFlags & 0x4000000) {
                        bossSetAnimAt(w->frame, brotherOmitsuJumpAnim, 0xEA, 0.0f);
                    } else {
                        bossSetAnimAt(w->frame, brotherGoemonJumpAnim, 0xEA, 0.0f);
                    }
                    gBrotherFlags |= 4;
                    w->vel[1] = 40.0f;
                } else if (joyGetHeldTime(gBrotherPort, gBrotherBtnPunch) == 1 &&
                           (gBrotherFlags & 4) && !(gBrotherFlags & 0x80) &&
                           !(gBrotherFlags & 0x20)) {
                    if (gBattleFlags & 0x4000000) {
                        bossSetAnimAt(w->frame, brotherOmitsuPunchAnim, 0xEA, 0.0f);
                        aniAtkDelete(w->box);
                        w->box = aniAtkCreate(uda_atk_root_frame, &w->frame->f, brotherOmitsuAtkChain,
                                                        floadGetSetFile(0xEA, 0), 0);
                        if (w->box == NULL) {
                            return -1;
                        }
                    } else {
                        bossSetAnimAt(w->frame, brotherGoemonPunchAnim, 0xEA, 0.0f);
                        aniAtkDelete(w->box);
                        w->box = aniAtkCreate(uda_atk_root_frame, &w->frame->f, brotherGoemonAtkChain,
                                                        floadGetSetFile(0xEA, 0), 0);
                        if (w->box == NULL) {
                            return -1;
                        }
                    }
                    if (!(gBattleFlags & 0x10)) {
                        w->box->flags |= 2;
                    } else {
                        w->box->flags &= ~2;
                    }
                    SoundLocation2(0x222, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                    gBrotherFlags &= ~4;
                    gBrotherFlags |= 0x80;
                } else if (joyGetHeldTime(gBrotherPort, gBrotherBtnPunch) == 1 &&
                           !(gBrotherFlags & 8) && !(gBrotherFlags & 0x80) &&
                           !(gBrotherFlags & 0x20)) {
                    if (gBattleFlags & 0x4000000) {
                        bossSetAnimAt(w->frame, brotherOmitsuPunchAnim, 0xEA, 0.0f);
                    } else {
                        bossSetAnimAt(w->frame, brotherGoemonPunchAnim, 0xEA, 0.0f);
                    }
                    SoundLocation2(0x222, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                    gBrotherFlags |= 8;
                }
            }
            if (gBrotherFlags & 0x20) {
                w->box->flags &= ~2;
                w->dmg->flags &= ~2;
                switch (w->step) {
                    case 0:
                        w->box->flags &= ~2;
                        w->dmg->flags &= ~2;
                        if (gBattleFlags & 0x4000000) {
                            bossSetAnimAt(w->frame, brotherOmitsuKnockDownAnim, 0xEA, 0.0f);
                        } else {
                            bossSetAnimAt(w->frame, brotherGoemonKnockDownAnim, 0xEA, 0.0f);
                        }
                        gBrotherFlags |= 0x1000;
                        w->vel[1] = 40.0f;
                        w->step++;
                        break;
                    case 1:
                        bossStepAnim(w->frame, 0.5f, 2);
                        if (!(gBrotherFlags & 0x40)) {
                            w->vel[1] = 40.0f;
                            if (gBattleBossId == 5) {
                                beSpeffectCreate(w->frame->pos[0], w->frame->pos[1] + 50.0f, w->frame->pos[2], 1);
                            } else {
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1] + 50.0f, w->frame->pos[2],
                                                       5.0f, 400.0f, 0);
                            }
                            SoundLocation2(0x247, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                                   w->frame->pos[2], 0.0f, 15000.0f, gBattleCamView->fovy);
                            (*bossRumble)(1, 0xF0F0F0F0, 10);
                            w->step++;
                        }
                        break;
                    case 2:
                        bossStepAnim(w->frame, 0.5f, 2);
                        if (!(gBrotherFlags & 0x40)) {
                            w->vel[1] = 0.0f;
                            if (gBattleFlags & 0x4000000) {
                                bossSetAnimAt(w->frame, brotherOmitsuKnockDownAnim, 0xEA, 1.0f);
                            } else {
                                bossSetAnimAt(w->frame, brotherGoemonKnockDownAnim, 0xEA, 1.0f);
                            }
                            if (gBattleBossId == 5) {
                                beSpeffectCreate(w->frame->pos[0], w->frame->pos[1] + 50.0f, w->frame->pos[2], 1);
                            } else {
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1] + 50.0f, w->frame->pos[2],
                                                       5.0f, 400.0f, 0);
                            }
                            SoundLocation2(0x247, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                                   w->frame->pos[2], 0.0f, 15000.0f, gBattleCamView->fovy);
                            (*bossRumble)(1, 0xF0F0F0F0, 10);
                            w->step++;
                        }
                        break;
                    case 3:
                        if (gBossFlags & 0x2000) {
                            if (gGameFrameCount % 5 == 0) {
                                if (gBattleBossId == 5) {
                                    beSpeffectCreate(w->frame->pos[0], w->frame->pos[1] + 50.0f,
                                                           w->frame->pos[2], 1);
                                } else {
                                    itexPuffGrey(w->frame->pos[0], w->frame->pos[1] + 100.0f,
                                                           w->frame->pos[2], 2.0f);
                                    itexPuffGrey(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
                                }
                            }
                        } else if (!(gBrotherFlags & 0x20000)) {
                            if (joyGetHeldTime(gBrotherPort, gBrotherBtnPunch) == 1 ||
                                joyGetHeldTime(gBrotherPort, gBrotherBtnJump) == 1) {
                                gBrotherDownTime -= 10;
                            }
                            gBrotherDownTime--;
                            if (gBrotherDownTime < 31) {
                                if (gBattleFlags & 0x4000000) {
                                    bossSetAnimAt(w->frame, brotherOmitsuGetUpAnim, 0xEA, 0.0f);
                                } else {
                                    bossSetAnimAt(w->frame, brotherGoemonGetUpAnim, 0xEA, 0.0f);
                                }
                                w->step++;
                                w->vel[1] = 40.0f;
                            }
                        }
                        break;
                    case 4:
                        bossStepAnim(w->frame, 0.5f, 2);
                        if (!(gBrotherFlags & 0x40)) {
                            f32 tx;
                            f32 tz;

                            bossHeadingOffset(w->frame->rot[1], 0.0f, -400.0f, &tx, &tz);
                            if (gBattleBossId == 5) {
                                beSpeffectCreate(w->frame->pos[0], w->frame->pos[1] + 50.0f, w->frame->pos[2], 1);
                            } else {
                                beSmoke6Create(w->frame->pos[0], w->frame->pos[1] + 50.0f, w->frame->pos[2],
                                                       5.0f, 400.0f, 0);
                            }
                            if (gBattleFlags & 0x4000000) {
                                bossSetAnimAt(w->frame, brotherOmitsuAnim, 0xEA, 0.0f);
                            } else {
                                bossSetAnimAt(w->frame, brotherGoemonAnim, 0xEA, 0.0f);
                            }
                            (*bossRumble)(1, 0xF0F0F0F0, 10);
                            gBrotherFlags &= ~0x20;
                            gBrotherFlags &= ~0x80;
                            gBrotherFlags &= ~4;
                            gBrotherFlags &= ~0x100;
                            w->step = 0;
                            w->dmg->flags |= 2;
                        }
                        break;
                }
            }
            if (gBrotherFlags & 0x20) {
                w->vel[1] -= 2.0;
            } else {
                w->vel[1] -= 1.0;
            }
            if (vramRayGround((Vec3f *) w->frame->pos, (Vec3f *) w->vel, &at, &t) == 0) {
                w->frame->pos[1] += w->vel[1];
                gBrotherFlags |= 0x40;
            } else {
                w->frame->pos[1] = at.y + 0.1f;
                w->vel[1] = 0.0f;
                gBrotherFlags &= ~0x40;
            }
            if (!(gBrotherFlags & 4) && !(gBrotherFlags & 8) && !(gBrotherFlags & 0x20) &&
                !(gBrotherFlags & 0x80)) {
                bossStepAnim(w->frame, 0.6f, 0);
            }
            gBrotherPosX = w->frame->pos[0];
            gBrotherPosY = w->frame->pos[1];
            gBrotherPosZ = w->frame->pos[2];
            gBrotherYaw = w->frame->rot[1];
            break;
        case 3:
            if (w->fading == 0) {
                w->alpha += 3;
            } else if (w->fading == 1) {
                w->alpha -= 3;
            }
            if (w->alpha > 192) {
                w->alpha = 192;
                w->fading = 1;
            } else if (w->alpha < 0) {
                w->alpha = 0;
                w->fading = 0;
            }
            if (gBrotherFlags & 0x10000) {
                w->frame->f.flags &= ~2;
                w->shadow->flags &= ~2;
            } else {
                w->frame->f.flags |= 2;
                w->shadow->flags |= 2;
                shadowPut(w->shadow, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 40.0f, 0);
            }
            if (gBrotherFlags & 0x1000) {
                w->frame->dl = bossLitTexEnvColorDL(0xFF, 0xFF, 0, 0xFF);
                gBrotherFlags &= ~0x1000;
            } else if (gBrotherFlags & 0x800) {
                w->frame->dl = bossLitTexEnvColorDL(fadeRand(0xFF), fadeRand(0xFF),
                                                      fadeRand(0xFF), 0x80);
            } else {
                w->frame->dl = bossLitTexEnvColorDL(0, 0, 0, 0);
                if (gBattleFlags & 0x4000000) {
                    if (gImpactArmor < 100) {
                        w->frame->dl = bossLitTexEnvColorDL(0x80, 0, 0, w->alpha);
                    }
                } else if (gImpactArmor2 < 100) {
                    w->frame->dl = bossLitTexEnvColorDL(0x80, 0, 0, w->alpha);
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Brother" in the water battle (the same moves): it stands 300 lower, with its ground
 * found from 300 above; its shots and landings splash ("Shibuki") instead of smoking. It
 * takes only the vulcan (which costs the Impact), the boss, the cannon, the baton and the
 * fan; knocked down, it gets up in four steps. */
s32 BrotherWater(Obj *obj, s16 msg, void *arg) {
    BrotherWork *w = (BrotherWork *) ((u32) obj + sizeof(Obj));
    s32 pad0;
    Vec3f from;
    Vec3f at;
    f32 t;
    f32 tx;
    f32 tz;

    switch (msg) {
        case 2:
            obj->name = "Brother";
            if (gBattleBossId == 0) {
                return -1;
            }
            if (gBattleBossId == 6) {
                return -1;
            }
            w->frame = (AnimeFrame *) frmCreate(gFrmLayer2, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            w->shadow = shadowCreate();
            if (gBattleFlags & 0x4000000) {
                PRINTF("おみつスタートします。[%d]", gBattleBossId);
                bossSetAnimAt(w->frame, brotherWaterOmitsuAnim, 0xEA, 0.0f);
                w->box = aniAtkCreate(uda_atk_root_frame, &w->frame->f, brotherWaterOmitsuAtkChain,
                                                floadGetSetFile(0xEA, 0), 0);
                if (w->box == NULL) {
                    return -1;
                }
            } else {
                PRINTF("おみつスタートします。[%d]", gBattleBossId);
                bossSetAnimAt(w->frame, brotherWaterGoemonAnim, 0xEA, 0.0f);
                w->box = aniAtkCreate(uda_atk_root_frame, &w->frame->f, brotherWaterGoemonAtkChain,
                                                floadGetSetFile(0xEA, 0), 0);
                if (w->box == NULL) {
                    return -1;
                }
            }
            w->frame->flags |= 0x20;
            w->box->kind = 0xCA;
            w->dmg = acollDmgAddYZ(ene_dmg_root_frame, &w->frame->f, 30, 200, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 0xCB;
            w->frame->scale[2] = 6.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->rot[0] = 0.0f;
            w->frame->pos[1] = 1000.0f;
            w->angle = 11500.0f;
            w->alpha = 0;
            w->fading = 0;
            w->unused24 = 1;
            w->vel[1] = 0.0f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            aniAtkDelete(w->box);
            frmDestroy(&w->dmg->frame);
            frmDestroy((Frame *) w->shadow);
            break;
        case 0:
            from.x = w->frame->pos[0];
            from.y = w->frame->pos[1] - 300.0f;
            from.z = w->frame->pos[2];
            w->box->flags &= ~2;
            if (fadeCountPlayers() == 1) {
                gBrotherFlags |= 0x400;
            }
            gBrotherShoutWait--;
            if (gBrotherShoutWait <= 0) {
                gBrotherShoutWait = 0;
            }
            if (fadeCountPlayers() >= 2 && joyGetHeldTime(gBrotherPort, gBrotherBtnShout) == 1 &&
                gBossLife > 0 && gImpactArmor > 0 && gImpactArmor2 > 0 &&
                !(gBrotherFlags & 0x20) && gBrotherShoutWait == 0) {
                gBrotherShoutWait = 90;
                if (gBrotherPilotChara == 0) {
                    (*sndPlay)(0x31C);
                } else if (gBrotherPilotChara == 1) {
                    (*sndPlay)(0x319);
                } else if (gBrotherPilotChara == 2) {
                    (*sndPlay)(0x322);
                } else if (gBrotherPilotChara == 3) {
                    (*sndPlay)(0x31F);
                }
            }
            if (gBattleFlags & 0x4000000) {
                if (gImpactArmor < 100 && !(gBrotherFlags & 0x20)) {
                    if (gGameFrameCount % 20 == 0) {
                        bePsmokeCreate(w->frame->pos[0], w->frame->pos[1] + 1000.0f, w->frame->pos[2], 2.0f, 0);
                    }
                    if (gGameFrameCount % 20 == 10) {
                        bePsmokeCreate(w->frame->pos[0], w->frame->pos[1] + 1000.0f, w->frame->pos[2], 1.0f, 0);
                    }
                }
            } else if (gImpactArmor2 < 100 && !(gBrotherFlags & 0x20)) {
                if (gGameFrameCount % 20 == 0) {
                    bePsmokeCreate(w->frame->pos[0], w->frame->pos[1] + 1000.0f, w->frame->pos[2], 2.0f, 0);
                }
                if (gGameFrameCount % 20 == 10) {
                    bePsmokeCreate(w->frame->pos[0], w->frame->pos[1] + 1000.0f, w->frame->pos[2], 1.0f, 0);
                }
            }
            if (gBossFlags & 0x2000) {
                gBrotherFlags |= 0x20;
            }
            if (gBrotherFlags & 0x20000) {
                gBrotherFlags |= 0x20;
            }
            w->dmg->flags |= 2;
            if (gBattleFlags2 & 0x400) {
                w->dmg->flags &= ~2;
            }
            if (gBrotherFlags & 0x4000) {
                w->frame->rot[1] = 0.0f;
                if (gBattleFlags & 0x4000000) {
                    bossSetAnimAt(w->frame, brotherWaterOmitsuThrownBackAnim, 0xEA, 0.0f);
                    aniAtkDelete(w->box);
                    w->box = aniAtkCreate(uda_atk_root_frame, &w->frame->f, brotherWaterOmitsuAtkChain,
                                                    floadGetSetFile(0xEA, 0), 0);
                    if (w->box == NULL) {
                        return -1;
                    }
                } else {
                    bossSetAnimAt(w->frame, brotherWaterGoemonThrownBackAnim, 0xEA, 0.0f);
                    aniAtkDelete(w->box);
                    w->box = aniAtkCreate(uda_atk_root_frame, &w->frame->f, brotherWaterGoemonAtkChain,
                                                    floadGetSetFile(0xEA, 0), 0);
                    if (w->box == NULL) {
                        return -1;
                    }
                }
                w->box->kind = 0xCA;
                w->box->flags &= ~2;
                gBrotherFlags &= ~0x4000;
                if (gBrotherPilotChara == 0) {
                    (*sndPlay)(0x31A);
                } else if (gBrotherPilotChara == 1) {
                    (*sndPlay)(0x317);
                } else if (gBrotherPilotChara == 2) {
                    (*sndPlay)(0x320);
                } else if (gBrotherPilotChara == 3) {
                    (*sndPlay)(0x31D);
                }
            }
            if (gBrotherFlags & 0x2000) {
                if (gBattleFlags & 0x4000000) {
                    if (w->frame->pos[0] > 0.0f) {
                        w->angle = 10000.0f;
                    } else {
                        w->angle = 12000.0f;
                    }
                    bossSetAnimAt(w->frame, brotherWaterOmitsuAnim, 0xEA, 0.0f);
                    aniAtkDelete(w->box);
                    w->box = aniAtkCreate(uda_atk_root_frame, &w->frame->f, brotherWaterOmitsuAtkChain,
                                                    floadGetSetFile(0xEA, 0), 0);
                    if (w->box == NULL) {
                        return -1;
                    }
                } else {
                    if (w->frame->pos[0] > 0.0f) {
                        w->angle = 10000.0f;
                    } else {
                        w->angle = 12000.0f;
                    }
                    bossSetAnimAt(w->frame, brotherWaterGoemonAnim, 0xEA, 0.0f);
                    aniAtkDelete(w->box);
                    w->box = aniAtkCreate(uda_atk_root_frame, &w->frame->f, brotherWaterGoemonAtkChain,
                                                    floadGetSetFile(0xEA, 0), 0);
                    if (w->box == NULL) {
                        return -1;
                    }
                }
                w->box->flags &= ~2;
            }
            (*acollAtkWasHit)(w->box);
            if (acollDmgWasHit(w->dmg) != 0) {
                if (w->dmg->hitKind == 0xF) {
                    PRINTF("外部インパクトはバルカンに当たりました。\n");
                    itexSparkleYellow(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 0.6f);
                    SoundLocation2(0x228, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                    bossImpactDamage(1, 0xF, 1);
                } else if (w->dmg->hitKind == 0xCD) {
                    PRINTF("外部インパクトはボスに当たりました。\n");
                    SoundLocation2(0x27E, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                    bossImpactDamage(0, 0x14, 1);
                    w->dmg->flags &= ~2;
                } else if (w->dmg->hitKind == 0xB) {
                    PRINTF("外部インパクトはんが砲に当たりました。\n");
                    SoundLocation2(0x247, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                    bossImpactDamage(0, 0x14, 1);
                } else if (w->dmg->hitKind == 0xC) {
                    PRINTF("外部インパクトはバトンに当たりました。\n");
                    if (!(gBrotherFlags & 0x20000) && gBossLife >= 2 &&
                        tskCreate(gCockpitTask0, change, 0, NULL) != NULL) {
                        gBrotherFlags |= 0x4000;
                    }
                } else if (w->dmg->hitKind == 0x51) {
                    PRINTF("外部インパクトはセンスに当たりました。\n");
                    SoundLocation2(0x243, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                    bossImpactDamage(0, 0x51, 1);
                }
            }
            if ((gBrotherFlags & 0x400) && !(gBattleFlags2 & 0x400)) {
                gBrotherMoveTime--;
                if (gBrotherMoveTime < 0) {
                    gBrotherMoveTime = 0;
                }
                if (gBrotherMove != 0 && gBrotherMove != 1 && gBrotherMove != 2) {
                    if (gBrotherMove == 3) {
                        if (!(gBrotherFlags & 4) && !(gBrotherFlags & 8) &&
                            !(gBrotherFlags & 0x80) && !(gBrotherFlags & 0x20)) {
                            if (gBattleFlags & 0x4000000) {
                                bossSetAnimAt(w->frame, brotherWaterOmitsuJumpAnim, 0xEA, 0.0f);
                            } else {
                                bossSetAnimAt(w->frame, brotherWaterGoemonJumpAnim, 0xEA, 0.0f);
                            }
                            gBrotherFlags |= 4;
                            w->vel[1] = 40.0f;
                        } else if ((gBrotherFlags & 4) && !(gBrotherFlags & 0x80) &&
                                   !(gBrotherFlags & 0x20)) {
                            if (gBattleFlags & 0x4000000) {
                                bossSetAnimAt(w->frame, brotherWaterOmitsuJumpShootAnim, 0xEA, 0.0f);
                            } else {
                                bossSetAnimAt(w->frame, brotherWaterGoemonJumpShootAnim, 0xEA, 0.0f);
                            }
                            gBrotherFlags &= ~4;
                            gBrotherFlags |= 0x80;
                        }
                    } else if (gBrotherMove == 4) {
                        if (!(gBattleFlags2 & 0x4000) && gGameFrameCount % 20 == 0 && gImpactMoney != 0 &&
                            !(gBrotherFlags & 0x20) && !(gBattleFlags & 0x10)) {
                            gOvalcanSide ^= 1;
                            bossAddMoney(-1);
                            tskCreate(gCockpitTask1, Ovalcan, 0,
                                                   (void *) (u32) gOvalcanSide);
                            gBrotherFlags |= 1;
                        }
                    } else if (gBrotherMove == 5 && !(gBrotherFlags & 8) &&
                               !(gBrotherFlags & 0x80) && !(gBrotherFlags & 0x20)) {
                        if (gBattleFlags & 0x4000000) {
                            bossSetAnimAt(w->frame, brotherWaterOmitsuJumpShootAnim, 0xEA, 0.0f);
                        } else {
                            bossSetAnimAt(w->frame, brotherWaterGoemonJumpShootAnim, 0xEA, 0.0f);
                        }
                        gBrotherFlags |= 8;
                    }
                }
                if (gBrotherMoveTime == 0) {
                    if (w->angle > 12000.0f) {
                        gBrotherMove = 6;
                    } else if (w->angle < 9600.0f) {
                        gBrotherMove = 7;
                    } else if (gBattleFlags2 & 0x400) {
                        gBrotherMove = 0;
                    } else {
                        gBrotherMove = fadeRand(6);
                    }
                    if (gBrotherMove == 0) {
                        gBrotherMoveTime = fadeRandRange(30, 90);
                    } else if (gBrotherMove == 1) {
                        gBrotherMoveTime = fadeRandRange(60, 120);
                    } else if (gBrotherMove == 2) {
                        gBrotherMoveTime = fadeRandRange(60, 120);
                    } else if (gBrotherMove == 3) {
                        gBrotherMoveTime = fadeRandRange(10, 30);
                    } else if (gBrotherMove == 4) {
                        gBrotherMoveTime = fadeRandRange(30, 90);
                    } else if (gBrotherMove == 5) {
                        gBrotherMoveTime = fadeRandRange(15, 45);
                    } else if (gBrotherMove == 6) {
                        gBrotherMoveTime = fadeRandRange(300, 360);
                        gBrotherMove = 2;
                    } else if (gBrotherMove == 7) {
                        gBrotherMoveTime = fadeRandRange(300, 360);
                        gBrotherMove = 1;
                    }
                    if (gBossFlags & 0x4000) {
                        PRINTF("インパクト緊急行動中\n");
                        gBrotherMove = 8;
                    }
                }
            }
            if (!(gBattleFlags2 & 0x400) && !(gBrotherFlags & 0x20) && !(gBrotherFlags & 8) &&
                !(gBrotherFlags & 0x80) && !(gBattleFlags & 0x10)) {
                w->angle += joyGetStickX(gBrotherPort, 0) / 12;
                w->speed = joyGetStickX(gBrotherPort, 0) / 12;
            }
            if (w->angle > 13500.0f) {
                w->angle = 13500.0f;
            }
            if (w->angle < 8100.0f) {
                w->angle = 8100.0f;
            }
            w->frame->pos[0] = cosf(w->angle * 0.017453293003880078 * 0.025) * 4500.0f;
            w->frame->pos[2] = sinf(w->angle * 0.017453293003880078 * 0.025) * 4500.0f;
            if (!(gBrotherFlags & 0x20) || (gBrotherFlags & 0x200)) {
                if (gBrotherFlags & 0x400) {
                    if (gBrotherMove == 1 && !(gBrotherFlags & 0x20) && !(gBrotherFlags & 4) &&
                        !(gBrotherFlags & 0x80)) {
                        w->speed = 7.0f;
                        w->angle += 7.0f;
                    } else if (gBrotherMove == 2 && !(gBrotherFlags & 0x20) &&
                               !(gBrotherFlags & 4) && !(gBrotherFlags & 0x80)) {
                        w->angle -= 7.0f;
                        w->speed = -7.0f;
                    } else if (gBrotherMove == 8 && !(gBrotherFlags & 0x20) &&
                               !(gBrotherFlags & 4) && !(gBrotherFlags & 0x80)) {
                        bossTurnToward2(w->frame, gImpactPosX, gImpactPosZ, 10.0f);
                        if (w->angle < 11000.0f) {
                            w->angle -= 10.0f;
                            w->speed = -10.0f;
                            if (w->angle < 8100.0f) {
                                w->angle = 8100.0f;
                            }
                        } else {
                            w->speed = 10.0f;
                            w->angle += 10.0f;
                            if (w->angle > 13500.0f) {
                                w->angle = 13500.0f;
                            }
                        }
                    }
                }
                if (w->speed > -4.0f && w->speed < 4.0f) {
                    bossTurnToward2(w->frame, gBossPosX, gBossPosZ, 2.0f);
                } else if (w->speed >= 0.0f) {
                    bossTurnToward2(w->frame, 2000.0f, -800.0f, 2.0f);
                } else if (w->speed < 0.0f) {
                    bossTurnToward2(w->frame, -2000.0f, -800.0f, 2.0f);
                }
            }
            if (!(gBattleFlags2 & 0x400)) {
                if (joyGetHeldTime(gBrotherPort, gBrotherBtnShoot) % 10 == 1 &&
                    gImpactMoney != 0 && !(gBrotherFlags & 0x20) && !(gBattleFlags & 0x10)) {
                    gOvalcanSide ^= 1;
                    gImpactMoney--;
                    tskCreate(gCockpitTask1, Ovalcan, 0, (void *) (u32) gOvalcanSide);
                    gBrotherFlags |= 1;
                } else {
                    joyGetHeldTime(gBrotherPort, gBrotherBtnShoot);
                }
            }
            if ((gBrotherFlags & 0x80) && (gBrotherFlags & 0x40) && !(gBrotherFlags & 0x20)) {
                if (bossStepAnimAtk(w->frame, w->box, 0.6f, 2) != 0) {
                    w->box->flags &= ~2;
                } else if (!(gBattleFlags & 0x10)) {
                    w->box->flags |= 2;
                } else {
                    w->box->flags &= ~2;
                }
            } else if ((gBrotherFlags & 0x80) && !(gBrotherFlags & 0x40)) {
                if (!(gBattleFlags & 0x10)) {
                    w->box->flags |= 2;
                } else {
                    w->box->flags &= ~2;
                }
                if (!(gBrotherFlags & 0x100) && !(gBrotherFlags & 0x20)) {
                    (*beShibukiCreate)(w->frame->pos[0], w->frame->pos[1] + 700.0f, w->frame->pos[2], 0);
                    gBrotherFlags |= 0x100;
                }
                if (bossStepAnimAtk(w->frame, w->box, 0.6f, 2) != 0) {
                    w->box->flags &= ~2;
                    if (!(gBrotherFlags & 0x20)) {
                        if (gBattleFlags & 0x4000000) {
                            bossSetAnimAt(w->frame, brotherWaterOmitsuAnim, 0xEA, 0.0f);
                        } else {
                            bossSetAnimAt(w->frame, brotherWaterGoemonAnim, 0xEA, 0.0f);
                        }
                    }
                    gBrotherFlags &= ~0x80;
                    gBrotherFlags &= ~0x100;
                }
            } else if ((gBrotherFlags & 4) && (gBrotherFlags & 0x40) && !(gBrotherFlags & 0x20)) {
                bossStepAnim(w->frame, 0.6f, 2);
            } else if ((gBrotherFlags & 4) && !(gBrotherFlags & 0x40) && !(gBrotherFlags & 0x20)) {
                if (bossStepAnim(w->frame, 0.6f, 2) != 0) {
                    if (gBattleFlags & 0x4000000) {
                        bossSetAnimAt(w->frame, brotherWaterOmitsuAnim, 0xEA, 0.0f);
                    } else {
                        bossSetAnimAt(w->frame, brotherWaterGoemonAnim, 0xEA, 0.0f);
                    }
                    if (!(gBrotherFlags & 0x20)) {
                        (*beShibukiCreate)(w->frame->pos[0], w->frame->pos[1] + 700.0f, w->frame->pos[2], 0);
                        gBrotherFlags &= ~4;
                    }
                }
            } else if ((gBrotherFlags & 8) && !(gBrotherFlags & 0x20)) {
                if (!(gBattleFlags & 0x10)) {
                    w->box->flags |= 2;
                } else {
                    w->box->flags &= ~2;
                }
                if (bossStepAnimAtk(w->frame, w->box, 0.6f, 2) != 0) {
                    if (gBattleFlags & 0x4000000) {
                        bossSetAnimAt(w->frame, brotherWaterOmitsuAnim, 0xEA, 0.0f);
                        aniAtkDelete(w->box);
                        w->box = aniAtkCreate(uda_atk_root_frame, &w->frame->f, brotherOmitsuAtkChain,
                                                        floadGetSetFile(0xEA, 0), 0);
                        if (w->box == NULL) {
                            return -1;
                        }
                    } else {
                        bossSetAnimAt(w->frame, brotherWaterGoemonAnim, 0xEA, 0.0f);
                        aniAtkDelete(w->box);
                        w->box = aniAtkCreate(uda_atk_root_frame, &w->frame->f, brotherGoemonAtkChain,
                                                        floadGetSetFile(0xEA, 0), 0);
                        if (w->box == NULL) {
                            return -1;
                        }
                    }
                    w->box->kind = 0xCA;
                    w->box->flags &= ~2;
                    gBrotherFlags &= ~8;
                }
            }
            if (!(gBattleFlags2 & 0x400)) {
                if (joyGetHeldTime(gBrotherPort, gBrotherBtnJump) == 1 &&
                    !(gBrotherFlags & 4) && !(gBrotherFlags & 8) && !(gBrotherFlags & 0x80) &&
                    !(gBrotherFlags & 0x20)) {
                    if (gBattleFlags & 0x4000000) {
                        bossSetAnimAt(w->frame, brotherWaterOmitsuJumpAnim, 0xEA, 0.0f);
                    } else {
                        bossSetAnimAt(w->frame, brotherWaterGoemonJumpAnim, 0xEA, 0.0f);
                    }
                    gBrotherFlags |= 4;
                    w->vel[1] = 40.0f;
                } else if (joyGetHeldTime(gBrotherPort, gBrotherBtnPunch) == 1 &&
                           (gBrotherFlags & 4) && !(gBrotherFlags & 0x80) &&
                           !(gBrotherFlags & 0x20)) {
                    if (gBattleFlags & 0x4000000) {
                        bossSetAnimAt(w->frame, brotherWaterOmitsuJumpShootAnim, 0xEA, 0.0f);
                        aniAtkDelete(w->box);
                        w->box = aniAtkCreate(uda_atk_root_frame, &w->frame->f, brotherOmitsuAtkChain,
                                                        floadGetSetFile(0xEA, 0), 0);
                        if (w->box == NULL) {
                            return -1;
                        }
                    } else {
                        bossSetAnimAt(w->frame, brotherWaterGoemonJumpShootAnim, 0xEA, 0.0f);
                        aniAtkDelete(w->box);
                        w->box = aniAtkCreate(uda_atk_root_frame, &w->frame->f, brotherGoemonAtkChain,
                                                        floadGetSetFile(0xEA, 0), 0);
                        if (w->box == NULL) {
                            return -1;
                        }
                    }
                    if (!(gBattleFlags & 0x10)) {
                        w->box->flags |= 2;
                    } else {
                        w->box->flags &= ~2;
                    }
                    SoundLocation2(0x222, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                    gBrotherFlags &= ~4;
                    gBrotherFlags |= 0x80;
                } else if (joyGetHeldTime(gBrotherPort, gBrotherBtnPunch) == 1 &&
                           !(gBrotherFlags & 8) && !(gBrotherFlags & 0x80) &&
                           !(gBrotherFlags & 0x20)) {
                    if (gBattleFlags & 0x4000000) {
                        bossSetAnimAt(w->frame, brotherWaterOmitsuJumpShootAnim, 0xEA, 0.0f);
                    } else {
                        bossSetAnimAt(w->frame, brotherWaterGoemonJumpShootAnim, 0xEA, 0.0f);
                    }
                    SoundLocation2(0x222, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                    gBrotherFlags |= 8;
                }
            }
            if (gBrotherFlags & 0x20) {
                w->box->flags &= ~2;
                w->dmg->flags &= ~2;
                switch (w->step) {
                    case 0:
                        w->box->flags &= ~2;
                        w->dmg->flags &= ~2;
                        if (gBattleFlags & 0x4000000) {
                            bossSetAnimAt(w->frame, brotherWaterOmitsuKnockDownAnim, 0xEA, 0.0f);
                        } else {
                            bossSetAnimAt(w->frame, brotherWaterGoemonKnockDownAnim, 0xEA, 0.0f);
                        }
                        gBrotherFlags |= 0x1000;
                        w->vel[1] = 40.0f;
                        w->step++;
                        break;
                    case 1:
                        bossStepAnim(w->frame, 0.5f, 2);
                        if (!(gBrotherFlags & 0x40)) {
                            w->vel[1] = 0.0f;
                            (*beShibukiCreate)(w->frame->pos[0], w->frame->pos[1] + 700.0f, w->frame->pos[2], 0);
                            if (gBattleFlags & 0x4000000) {
                                bossSetAnimAt(w->frame, brotherWaterOmitsuLandAnim, 0xEA, 0.0f);
                            } else {
                                bossSetAnimAt(w->frame, brotherWaterGoemonLandAnim, 0xEA, 0.0f);
                            }
                            SoundLocation2(0x247, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                                   w->frame->pos[2], 0.0f, 15000.0f, gBattleCamView->fovy);
                            (*bossRumble)(1, 0xF0F0F0F0, 10);
                            w->step++;
                        }
                        break;
                    case 2:
                        bossStepAnim(w->frame, 1.0f, 0);
                        if (gBossFlags & 0x2000) {
                            if (gGameFrameCount % 5 == 0) {
                                itexPuffGrey(w->frame->pos[0], w->frame->pos[1] + 100.0f, w->frame->pos[2],
                                                       2.0f);
                                itexPuffGrey(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
                            }
                        } else if (!(gBrotherFlags & 0x20000)) {
                            if (joyGetHeldTime(gBrotherPort, gBrotherBtnPunch) == 1 ||
                                joyGetHeldTime(gBrotherPort, gBrotherBtnJump) == 1) {
                                gBrotherDownTime -= 10;
                            }
                            gBrotherDownTime--;
                            if (gBrotherDownTime < 31) {
                                if (gBattleFlags & 0x4000000) {
                                    bossSetAnimAt(w->frame, brotherWaterOmitsuGetUpAnim, 0xEA, 0.0f);
                                } else {
                                    bossSetAnimAt(w->frame, brotherWaterGoemonGetUpAnim, 0xEA, 0.0f);
                                }
                                w->step++;
                                w->vel[1] = 40.0f;
                            }
                        }
                        break;
                    case 3:
                        bossStepAnim(w->frame, 0.2f, 2);
                        if (!(gBrotherFlags & 0x40)) {
                            (*beShibukiCreate)(w->frame->pos[0], w->frame->pos[1] + 700.0f, w->frame->pos[2], 0);
                            bossHeadingOffset(w->frame->rot[1], 0.0f, -400.0f, &tx, &tz);
                            if (gBattleFlags & 0x4000000) {
                                bossSetAnimAt(w->frame, brotherWaterOmitsuAnim, 0xEA, 0.0f);
                            } else {
                                bossSetAnimAt(w->frame, brotherWaterGoemonAnim, 0xEA, 0.0f);
                            }
                            (*bossRumble)(1, 0xF0F0F0F0, 10);
                            gBrotherFlags &= ~0x20;
                            gBrotherFlags &= ~0x80;
                            gBrotherFlags &= ~4;
                            gBrotherFlags &= ~0x100;
                            w->step = 0;
                            w->dmg->flags |= 2;
                        }
                        break;
                }
            }
            if (gBrotherFlags & 0x20) {
                w->vel[1] -= 2.0;
            } else {
                w->vel[1] -= 1.0;
            }
            if (vramRayGround(&from, (Vec3f *) w->vel, &at, &t) == 0) {
                w->frame->pos[1] += w->vel[1];
                gBrotherFlags |= 0x40;
            } else {
                w->frame->pos[1] = at.y + 0.1f + 300.0f;
                w->vel[1] = 0.0f;
                gBrotherFlags &= ~0x40;
            }
            if (!(gBrotherFlags & 4) && !(gBrotherFlags & 8) && !(gBrotherFlags & 0x20) &&
                !(gBrotherFlags & 0x80)) {
                bossStepAnim(w->frame, 0.6f, 0);
            }
            gBrotherPosX = w->frame->pos[0];
            gBrotherPosY = w->frame->pos[1];
            gBrotherPosZ = w->frame->pos[2];
            gBrotherYaw = w->frame->rot[1];
            break;
        case 3:
            if (w->fading == 0) {
                w->alpha += 3;
            } else if (w->fading == 1) {
                w->alpha -= 3;
            }
            if (w->alpha > 192) {
                w->alpha = 192;
                w->fading = 1;
            } else if (w->alpha < 0) {
                w->alpha = 0;
                w->fading = 0;
            }
            if (gBrotherFlags & 0x10000) {
                w->frame->f.flags &= ~2;
                w->shadow->flags &= ~2;
            } else {
                w->frame->f.flags |= 2;
                w->shadow->flags |= 2;
                shadowPut(w->shadow, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 40.0f, 0);
            }
            if (gBrotherFlags & 0x1000) {
                w->frame->dl = bossLitTexEnvColorDL(0xFF, 0xFF, 0, 0xFF);
                gBrotherFlags &= ~0x1000;
            } else if (gBrotherFlags & 0x800) {
                w->frame->dl = bossLitTexEnvColorDL(fadeRand(0xFF), fadeRand(0xFF),
                                                      fadeRand(0xFF), 0x80);
            } else {
                w->frame->dl = bossLitTexEnvColorDL(0, 0, 0, 0);
                if (gBattleFlags & 0x4000000) {
                    if (gImpactArmor < 100) {
                        w->frame->dl = bossLitTexEnvColorDL(0x80, 0, 0, w->alpha);
                    }
                } else if (gImpactArmor2 < 100) {
                    w->frame->dl = bossLitTexEnvColorDL(0x80, 0, 0, w->alpha);
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
