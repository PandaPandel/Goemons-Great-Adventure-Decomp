#include "common.h"
#include "object.h"
#include "player.h"
#include "game.h"
#include "boss.h"
#include "camera.h"
#include "aniatk.h"
#include "audio.h"
#include "bappcam.h"
#include "beffect.h"
#include "bmove.h"
#include "contpak.h"
#include "fade.h"
#include "fileload.h"
#include "gfxmgr.h"
#include "impact.h"
#include "itex.h"
#include "joypad.h"
#include "mathutil.h"
#include "polygon.h"
#include "sndloc.h"
#include "tmanager.h"
#include "vram.h"

/* The game's debug print, argument-only (strings kept, arguments evaluated): what some of this
 * file's functions were built with, where the repo's `if (0)` PRINTF moves registers. */
#define DPRINTF

/* An "mpol" frame's display list and its segments from a file set: the mpol twin of
 * bossSetPolDL (bmove.c), which the game has inline, so a two-statement macro. Its do-while(0)
 * body ends a basic block, the boundary MEGA's hit path needs (IDO_NOTES 228). */
#define MPOL_SET_DL(m, list, n)                             \
    do {                                                    \
        (m)->dl2 = (list);                                  \
        floadGetSetRam((void **) (m)->segs, (n));           \
    } while (0)

/* The Impact's side of the boss battles (overlay 10): its moves ("Valcan", "Jab", "ComboJab",
 * "Straight", "CStraight", "Ovalcan", "Kick", "Guard"), its gauges ("Pgage", "money", "fuel",
 * "bosslife"), "Storm", "MEGA", the battle's backdrop ("Bbg"), the shoulder, the baton, and
 * the closing words ("Bword", "Bclear"). */

/* "Bbg"'s work: the battle's backdrop, faded in and out. */
typedef struct BbgWork {
    /* 0x00 */ BGFrame *frame;
    /* 0x04 */ u8 pad4[8];
    /* 0x0C */ s32 level; /* 0-256 */
} BbgWork;

/* "Pgage"'s work: the power gauge, a box and its bar. */
typedef struct PgageWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ AnimeFrame *bar;
    /* 0x08 */ s32 alpha;
} PgageWork;

/* "money"'s and "fuel"'s work: a panel with three digits. */
typedef struct CounterWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ Anim digits[3];
    /* 0x34 */ s32 alpha;
} CounterWork;

/* "bosslife"'s work: two panels with two digits each, and a third. */
typedef struct BossLifeWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ PolFrame *f1;
    /* 0x08 */ PolFrame *f2;
    /* 0x0C */ Anim digits[4];
    /* 0x4C */ s32 alpha;
} BossLifeWork;

/* A punch's work ("Jab", "ComboJab", "Straight", "CStraight"): the fist and its attack box. */
typedef struct PunchWork {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ ACollFrame *box;
    /* 0x08 */ s32 time;
    /* 0x0C */ s32 stage;
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ s32 count; /* the straights' frames */
    /* 0x18 */ s32 hit; /* the straights: 1 once a hit froze it */
} PunchWork;

/* A vulcan shot's work ("Valcan", "Ovalcan"): a battle object (bmove.c) with an attack box. */
typedef struct ShotWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ ACollFrame *box;
    /* 0x14 */ u8 pad14[4];
    /* 0x18 */ f32 dist; /* squared, from the Impact */
    /* 0x1C */ s32 time;
} ShotWork;

/* The first "Bword"'s work: a word of the closing banner flying into place. */
typedef struct BwordWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ u8 pad10[0x18];
    /* 0x28 */ s32 kind;
    /* 0x2C */ s32 stage;
    /* 0x30 */ s32 count;
    /* 0x34 */ s32 unused34; /* set to 0 when made, never read */
    /* 0x38 */ s32 alpha;
} BwordWork;

/* "Bword"'s work: a word of the closing banner. */
typedef struct WordWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad4[0x24];
    /* 0x28 */ s32 kind;
    /* 0x2C */ s32 stage;
    /* 0x30 */ s32 count;
    /* 0x34 */ u8 pad34[4];
    /* 0x38 */ s32 alpha;
} WordWork;

/* "Bclear"'s work: the battle-won sequence. */
typedef struct ClearWork {
    /* 0x00 */ u8 pad0[0x20];
    /* 0x20 */ s32 stage;
    /* 0x24 */ s32 time;
} ClearWork;

/* "Baton"'s work: a thrown baton (a battle object) with attack and damage boxes. */
typedef struct BatonWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ MPolFrame *model;
    /* 0x18 */ ACollFrame *box;
    /* 0x1C */ ACollFrame *dmg;
    /* 0x20 */ u8 pad20[4];
    /* 0x24 */ f32 dist;
} BatonWork;

/* "MEGA"'s work: the んが砲 cannon's shot (a battle object) with its attack box, and the beam
 * behind it in two layers; each part is a "pol" frame for its place and an "mpol" that draws it. */
typedef struct MegaWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ MPolFrame *model;
    /* 0x14 */ ACollFrame *box;
    /* 0x18 */ PolFrame *beam;
    /* 0x1C */ MPolFrame *beamModel;
    /* 0x20 */ PolFrame *glow;
    /* 0x24 */ MPolFrame *glowModel;
    /* 0x28 */ u8 pad28[8];
    /* 0x30 */ s32 time;
    /* 0x34 */ s32 alpha;
    /* 0x38 */ u32 stage;
    /* 0x3C */ s32 struck; /* 1 once the shot is at the boss */
} MegaWork;

/* "Storm"'s work: one fist of the hundred-punch. */
typedef struct StormWork {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ s32 kind; /* 0-5, or 0-11 while the punches fly */
    /* 0x08 */ s32 time;
    /* 0x0C */ s32 stage;
    /* 0x10 */ s32 alpha;
} StormWork;

/* "Astorm"'s work: the hundred-punch's attack box. */
typedef struct AstormWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ ACollFrame *box;
    /* 0x08 */ AnimeFrame *fx;
    /* 0x0C */ f32 sway;
} AstormWork;

/* The guarding arm's and "Guard"'s work. */
typedef struct ArmWork {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ u8 pad4[4];
    /* 0x08 */ s32 time;
    /* 0x0C */ s32 stage;
} ArmWork;

typedef struct GuardWork {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ u8 pad4[4];
    /* 0x08 */ s32 stage;
    /* 0x0C */ s32 alpha;
} GuardWork;

/* "vc"'s work: an object with vertex colours. */
typedef struct VcWork {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ u8 pad4[4];
    /* 0x08 */ s32 unused8; /* set to 0 when made, never read */
} VcWork;

/* "shoulder"'s work. */
typedef struct ShoulderWork {
    /* 0x00 */ PolFrame *frame;
} ShoulderWork;

/* "Storm"'s argument. */
typedef struct StormArg {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ s32 kind;
    /* 0x08 */ u8 pad8[0xC];
} StormArg; /* size 0x14 */

/* The closing words' arguments ("Bword", "Bclear"). */
typedef struct WordArg {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ f32 pos[3];
    /* 0x1C */ u8 pad1C[0xC];
    /* 0x28 */ s32 n;
    /* 0x2C */ u8 pad2C[0x10];
} WordArg; /* size 0x3C */

typedef struct WordFxArg {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ f32 a[6];
    /* 0x28 */ s32 n;
    /* 0x2C */ u8 pad2C[0x10];
} WordFxArg; /* size 0x3C */

typedef struct ClearArg {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ f32 pos[3];
    /* 0x1C */ s32 n;
    /* 0x20 */ u8 pad20[0xC];
} ClearArg; /* size 0x2C */

extern Obj *gEffectTask; /* the effects' parent */
extern Obj *gBattleTask; /* the battle's task */
extern Frame *gFrmLayers[]; /* the frame pools */
extern Frame *gFrmModelLayer;
extern s16 gFrmKindPol; /* the "pol" kind */
extern s16 gFrmKindBg; /* the background kind */
extern u8 impBackdropMap[]; /* the backdrop, in file 0xEC */
extern u8 impBackdropTiles[];
extern u8 impBackdropPal[];
extern u8 impShoulderDl[]; /* the shoulder, in file 0xEA */
extern Frame *gFrmLayer2;
extern s16 gFrmKindAnime; /* the "anime" kind */
extern u8 impVertexColouredAnim[]; /* the vertex-coloured object, in file 0xEA */
extern u8 impBackdrop2Map[]; /* the second backdrop, in file 0xF1 */
extern u32 gGameFrameCount; /* GameState's frame count */
extern Frame *gFrmLayer8;
extern Gfx *gGfxDlHead; /* the display-list buffer */
extern u8 impArmRightAnim[]; /* the arm, right and left (file 0xEA) */
extern u8 impArmLeftAnim[];
extern u8 impGuardPilot1Anim[]; /* the guard: one pilot's, the other's by side */
extern u8 impGuardPilot2AimLeftAnim[];
extern u8 impGuardPilot2AimRightAnim[];
extern Frame *gCockpitRootFrame;
extern Frame *gCockpitPanelFrame; /* the panels' parent */
/* The digits 0-9: a texture each in the battle files (segment 3), 1 frame each. */
AnimFrame gImpactDigitFrames[] = {
    { 0x03001370, 1 }, { 0x03002370, 1 }, { 0x03003370, 1 }, { 0x03004370, 1 }, { 0x03005370, 1 },
    { 0x03006370, 1 }, { 0x03007370, 1 }, { 0x03008370, 1 }, { 0x03009370, 1 }, { 0x0300A370, 1 },
    { 0, 0 },
};
extern u8 impPowerGaugeDl[]; /* the panels, in file 0xEB */
extern u8 impPowerGaugeBarAnim[];
extern u8 impMoneyPanelPilot2Dl[];
extern u8 impMoneyPanelPilot1Dl[];
extern u8 impFuelPanelPilot2Dl[];
extern u8 impFuelPanelPilot1Dl[];
extern u8 impBossLifePilot2Dl[];
extern u8 impBossLifeF1Pilot2Dl[];
extern u8 impBossLifePilot1Dl[];
extern u8 impBossLifeF1Pilot1Dl[];
extern u8 impBossLifeF2Dl[];
extern Frame *pl_atk_root_frame; /* the Impact's attack boxes' parent */
extern Frame *uda_atk_root_frame; /* the other side's */
extern CamView *gBattleCamView; /* the battle's camera */
extern u8 impJabPilot1Anim[]; /* the jab, by pilot (file 0xEA) */
extern u8 impJabPilot2Anim[];
extern u8 impJabAtkChain[]; /* the jab's attack chain */
extern u8 impStraightPilot2Anim[]; /* the straight, for the second pilot */
extern u8 impKickRightPilot2Anim[]; /* the kick, right and left, by pilot */
extern u8 impKickLeftPilot2Anim[];
extern u8 impKickRightAtkChain[]; /* the kicks' attack chains */
extern u8 impKickLeftAtkChain[];
extern u8 impVulcanShotPilot1Dl[]; /* the vulcan shot, by pilot */
extern u8 impVulcanShotPilot2Dl[];
extern Frame *gFramePool10;
extern u8 impBannerWord0Dl[]; /* the closing banner's words (file 0xF2) */
extern u8 impBannerWord1Dl[];
extern u8 impBannerWord2Dl[];
extern u8 impBannerWord4Dl[];
extern u8 impBannerWord5Dl[];
extern s8 gPlayer1Chara; /* the four players' characters */
extern u8 impStormBlurPilot1Anim[]; /* the hundred-punch's blur, by pilot (file 0xEF) */
extern u8 impStormBlurPilot2Anim[];
extern s16 gFrmKindMpol; /* the "mpol" kind */
extern Frame *pl_dmg_root_frame; /* the damage boxes' parent */
extern Gfx impBatonDl[]; /* the baton (file 0xEA) */
extern u8 impStormFist2Pilot2Anim[]; /* the hundred-punch's fists, by pilot (file 0xEA) */
extern u8 impStormFist2Pilot1Anim[];
extern u8 impStormFist3Pilot2Anim[];
extern u8 impStormFist3Pilot1Anim[];
extern u8 impStormFist4Pilot2Anim[];
extern u8 impStormFist4Pilot1Anim[];
extern u8 impStormFist5Pilot2Anim[];
extern u8 impStormFist5Pilot1Anim[];

extern Frame *gFrmEffectLayer; /* the frame effects go under */
extern Gfx impMegaShotDl[]; /* the cannon's shot and beam (file 0xEE) */
extern Gfx impMegaBeamDl[];
extern Gfx impMegaShotHitDl[];


/* "Valcan": a vulcan shot from one of the Impact's guns (arg 0 the right, else the left),
 * flying at 50 toward a point 3000 ahead (within 80 at random). Struck by the boss's guard
 * (hit kinds 0x1C, 0x31, 0x6D, 0x72, 0x76, or 0xCC while the boss guards) it glances off at
 * a random angle (sound 0x2AB); any other hit ends it. It ends against the ground (sparks)
 * or, against the fourth boss, in the water below -1200 (a splash); far off it shrinks and
 * falls. */
s32 Valcan(Obj *obj, s16 msg, void *arg) {
    ShotWork *w = (ShotWork *) ((u32) obj + sizeof(Obj));
    s32 pad0;
    f32 angle;
    f32 c;
    f32 s;
    s32 pad;
    Vec3f at;
    f32 t;
    s32 pad2;
    f32 tx;
    f32 tz;

    switch (msg) {
        case 2:
            obj->name = "Valcan";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->box = acollAtkAddYZ(pl_atk_root_frame, &w->frame->f, 20, 0, 0, 0);
            if (w->box == NULL) {
                return -1;
            }
            w->box->kind = 0xF;
            if (arg == NULL) {
                if (gBattleFlags & 0x4000000) {
                    bossSetPolDL(w->frame, impVulcanShotPilot2Dl, 0xEA);
                } else {
                    bossSetPolDL(w->frame, impVulcanShotPilot1Dl, 0xEA);
                }
                bossHeadingOffset(gCockpitHeadYaw, 50.0f, -100.0f, &w->frame->pos[0], &w->frame->pos[2]);
                w->frame->pos[1] = gCockpitAimY - 150.0f;
            } else {
                if (gBattleFlags & 0x4000000) {
                    bossSetPolDL(w->frame, impVulcanShotPilot2Dl, 0xEA);
                } else {
                    bossSetPolDL(w->frame, impVulcanShotPilot1Dl, 0xEA);
                }
                bossHeadingOffset(gCockpitHeadYaw, -50.0f, -100.0f, &w->frame->pos[0], &w->frame->pos[2]);
                w->frame->pos[1] = gCockpitAimY - 150.0f;
            }
            (*bossSetFrameFlags)(w->frame, 0x20);
            w->frame->scale[2] = 1.2f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->flags |= 3;
            bossHeadingOffset(gCockpitHeadYaw, 0.0f, 3000.0f, &tx, &tz);
            BmoveX(obj, fadeRand(80) + (tx - 40.0f),
                                   fadeRand(80) + (gCockpitAimY * 60.0f + 120.0f),
                                   fadeRand(80) + (tz - 40.0f), 50.0f);
            w->time = 10000;
            sndPlay(0x237);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->box->frame);
            break;
        case 0:
            w->frame->rot[2] += 0.261799395058201156;
            if (acollAtkWasHit(w->box) == 1) {
                angle = mathAtan2(w->box->pos[0] - w->frame->pos[0], w->box->pos[2] - w->frame->pos[2]);
                c = cosf(angle);
                s = sinf(angle);
                if (w->box->hitKind == 0x1C) {
                    w->box->flags &= ~2;
                    itexGlowBlue(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 0.2f);
                    BmoveX(obj, cosf(fadeRand(350) * 0.0174532930038800785) * 1000.0f,
                                           sinf(fadeRand(350) * 0.0174532930038800785) * 1000.0f,
                                           w->frame->pos[2] * 0.6, 40.0f);
                    SoundLocation2(0x2AB, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                } else if (w->box->hitKind == 0x31) {
                    w->box->flags &= ~2;
                    BmoveX(obj, cosf(fadeRand(350) * 0.0174532930038800785) * 1000.0f,
                                           sinf(fadeRand(350) * 0.0174532930038800785) * 1000.0f,
                                           w->frame->pos[2] * 0.6, 40.0f);
                    SoundLocation2(0x2AB, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                } else if (w->box->hitKind == 0x6D) {
                    w->box->flags &= ~2;
                    BmoveX(obj, cosf(fadeRand(350) * 0.0174532930038800785) * 1000.0f,
                                           sinf(fadeRand(350) * 0.0174532930038800785) * 1000.0f,
                                           w->frame->pos[2] * 0.6, 40.0f);
                    SoundLocation2(0x2AB, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                } else if (w->box->hitKind == 0x72) {
                    w->box->flags &= ~2;
                    BmoveX(obj, cosf(fadeRand(350) * 0.0174532930038800785) * 1000.0f,
                                           sinf(fadeRand(350) * 0.0174532930038800785) * 1000.0f,
                                           w->frame->pos[2] * 0.6, 40.0f);
                    SoundLocation2(0x2AB, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                } else if (w->box->hitKind == 0x76) {
                    w->box->flags &= ~2;
                    BmoveX(obj, cosf(fadeRand(350) * 0.0174532930038800785) * 1000.0f,
                                           sinf(fadeRand(350) * 0.0174532930038800785) * 1000.0f,
                                           w->frame->pos[2] * 0.6, 40.0f);
                    SoundLocation2(0x2AB, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                } else if (gBossFlags & 4) {
                    if (w->box->hitKind == 0xCC) {
                        w->box->flags &= ~2;
                        itexGlowBlue(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 0.5f);
                        BmoveX(obj, cosf(fadeRand(350) * 0.0174532930038800785) * 1000.0f,
                                               sinf(fadeRand(350) * 0.0174532930038800785) * 1000.0f,
                                               w->frame->pos[2] * 0.6, 40.0f);
                        SoundLocation2(0x2AB, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                               300.0f, 15000.0f, gBattleCamView->fovy);
                        w->time = 15;
                    }
                } else {
                    return -1;
                }
            }
            if (gBattleBossId != 5 &&
                vramRayGround((Vec3f *) w->frame->pos, (Vec3f *) w->vel, &at, &t) != 0) {
                itexSparkleYellow(w->frame->pos[0], w->frame->pos[1] + 50.0f, w->frame->pos[2], 0.6f);
                return -1;
            }
            w->dist = bossDistSq(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], gImpactPosX,
                                             gImpactPosY, gImpactPosZ);
            if (gBattleBossId == 4) {
                if (w->frame->pos[1] < -1200.0f) {
                    (*beShibukiCreate)(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 3);
                    itexGlowBlue(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 0.2f);
                    return -1;
                }
            }
            if (w->dist > 4000000.0f) {
                w->frame->scale[0] -= 0.01;
                w->frame->scale[1] -= 0.01;
                w->frame->scale[2] -= 0.01;
                w->vel[1] -= 0.2;
                if (w->frame->scale[0] < 0.05) {
                    return -1;
                }
            } else {
                w->frame->scale[0] -= 0.01;
                w->frame->scale[1] -= 0.01;
                w->frame->scale[2] -= 0.01;
                if (w->frame->scale[0] < 0.5) {
                    w->frame->scale[0] = 0.5f;
                    w->frame->scale[1] = 0.5f;
                    w->frame->scale[2] = 0.5f;
                }
            }
            if (--w->time == 0) {
                return -1;
            }
            bossApplyVel(obj);
            if (gBattleFlags2 & 0x400) {
                return -1;
            }
            if (gBattleFlags & 0x10) {
                return -1;
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Jab": the Impact's fist (by pilot) beside it, with an attack box chain; it rings (sound
 * 0x222) and rumbles. When the box hits, a hit ring rises where it struck and the fist
 * freezes a frame (5 on the return); against an angry boss (flag 0x4000 of gBossFlags) it
 * draws back at once, with a sound if the boss guarded (hit kind 0xCC). */
s32 Jab(Obj *obj, s16 msg, void *arg) {
    PunchWork *w = (PunchWork *) ((u32) obj + sizeof(Obj));
    f32 z;
    f32 angle;
    f32 x;
    f32 y;

    switch (msg) {
        case 2:
            obj->name = "Jab";
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            if (gCockpitAimX >= 0.0f) {
                if (gBattleFlags & 0x4000000) {
                    bossSetAnimAt(w->frame, impJabPilot2Anim, 0xEA, 0.0f);
                } else {
                    bossSetAnimAt(w->frame, impJabPilot1Anim, 0xEA, 0.0f);
                }
                w->box = aniAtkCreate(pl_atk_root_frame, &w->frame->f, (u32) impJabAtkChain,
                                                floadGetSetFile(0xEA, 0), 0);
                if (w->box == NULL) {
                    return -1;
                }
                w->frame->rot[0] = gCockpitHeadPitch - 0.15;
                w->frame->rot[1] = gCockpitHeadYaw - 0.05;
                bossHeadingOffset(gCockpitRotY * 2, -120.0f, -50.0f, &w->frame->pos[0], &w->frame->pos[2]);
            } else {
                if (gBattleFlags & 0x4000000) {
                    bossSetAnimAt(w->frame, impJabPilot2Anim, 0xEA, 0.0f);
                } else {
                    bossSetAnimAt(w->frame, impJabPilot1Anim, 0xEA, 0.0f);
                }
                w->box = aniAtkCreate(pl_atk_root_frame, &w->frame->f, (u32) impJabAtkChain,
                                                floadGetSetFile(0xEA, 0), 0);
                if (w->box == NULL) {
                    return -1;
                }
                w->frame->rot[0] = gCockpitHeadPitch - 0.15;
                w->frame->rot[1] = gCockpitHeadYaw - 0.05;
                bossHeadingOffset(gCockpitRotY, -120.0f, -50.0f, &w->frame->pos[0], &w->frame->pos[2]);
            }
            w->box->kind = 1;
            (*bossSetFrameFlags2)(w->frame, 0x20);
            w->frame->scale[2] = 1.2f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[1] = -100.0f;
            w->stage = 0;
            sndPlayPan(0x222, 0x26);
            bossAddPower(1);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            aniAtkDelete(w->box);
            break;
        case 0:
            switch (w->stage) {
                case 0:
                    if (bossStepAnimAtk(w->frame, (AnimeFrame *) w->box, 1.2f, 2) != 0) {
                        w->time = 10;
                        w->stage = 1;
                    }
                    if (acollAtkWasHit(w->box) == 1) {
                        w->box->flags &= ~2;
                        angle = mathAtan2(aniAtkGetBox(w->box)->pos[0] - w->box->hitPos[0],
                                                       aniAtkGetBox(w->box)->pos[2] - w->box->hitPos[2]);
                        x = w->box->pos[0] + cosf(angle) * w->box->radius;
                        y = w->box->pos[1];
                        z = w->box->pos[2] + sinf(angle) * w->box->radius;
                        beHitringCreate(x, y, z, 1.0f, 0);
                        w->time = 1;
                        w->stage = 3;
                        if (gBossFlags & 0x4000) {
                            if (w->box->hitKind == 0xCC) {
                                SoundLocation2(0x150, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                                       w->frame->pos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                            }
                            PRINTF("げげ、ボス怒ってる。\n");
                            w->stage = 2;
                        }
                    }
                    break;
                case 1:
                    if (--w->time == 0) {
                        w->stage = 2;
                    }
                    if (acollAtkWasHit(w->box) == 1) {
                        w->box->flags &= ~2;
                        angle = mathAtan2(aniAtkGetBox(w->box)->pos[0] - w->box->hitPos[0],
                                                       aniAtkGetBox(w->box)->pos[2] - w->box->hitPos[2]);
                        x = w->box->pos[0] + cosf(angle) * w->box->radius;
                        y = w->box->pos[1];
                        z = w->box->pos[2] + sinf(angle) * w->box->radius;
                        beHitringCreate(x, y, z, 1.0f, 0);
                        w->time = 5;
                        w->stage = 3;
                        if (gBossFlags & 0x4000) {
                            if (w->box->hitKind == 0xCC) {
                                SoundLocation2(0x150, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                                       w->frame->pos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                            }
                            PRINTF("げげ、ボス怒ってる。\n");
                            w->stage = 2;
                        }
                    }
                    break;
                case 2:
                    w->box->flags &= ~2;
                    if (bossStepAnimAtk(w->frame, (AnimeFrame *) w->box, 1.2f, 3) != 0) {
                        return -1;
                    }
                    break;
                case 3:
                    w->box->flags &= ~2;
                    if (--w->time == 0) {
                        PRINTF("ジャブヒット硬直\n");
                        w->stage = 2;
                    }
                    break;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 255);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "ComboJab": the second jab of a combo, from the other side; it freezes 5 frames on a hit
 * (8 on the return). */
s32 ComboJab(Obj *obj, s16 msg, void *arg) {
    PunchWork *w = (PunchWork *) ((u32) obj + sizeof(Obj));
    f32 z;
    f32 angle;
    f32 x;
    f32 y;

    switch (msg) {
        case 2:
            obj->name = "ComboJab";
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            if (gCockpitAimX >= 0.0f) {
                if (gBattleFlags & 0x4000000) {
                    bossSetAnimAt(w->frame, impJabPilot2Anim, 0xEA, 0.0f);
                } else {
                    bossSetAnimAt(w->frame, impJabPilot1Anim, 0xEA, 0.0f);
                }
                w->box = aniAtkCreate(pl_atk_root_frame, &w->frame->f, (u32) impJabAtkChain,
                                                floadGetSetFile(0xEA, 0), 0);
                if (w->box == NULL) {
                    return -1;
                }
                w->frame->rot[0] = gCockpitHeadPitch - 0.15;
                w->frame->rot[1] = gCockpitHeadYaw + 0.05;
                bossHeadingOffset(gCockpitRotY, 120.0f, -50.0f, &w->frame->pos[0], &w->frame->pos[2]);
            } else {
                if (gBattleFlags & 0x4000000) {
                    bossSetAnimAt(w->frame, impJabPilot2Anim, 0xEA, 0.0f);
                } else {
                    bossSetAnimAt(w->frame, impJabPilot1Anim, 0xEA, 0.0f);
                }
                w->box = aniAtkCreate(pl_atk_root_frame, &w->frame->f, (u32) impJabAtkChain,
                                                floadGetSetFile(0xEA, 0), 0);
                if (w->box == NULL) {
                    return -1;
                }
                w->frame->rot[0] = gCockpitHeadPitch - 0.15;
                w->frame->rot[1] = gCockpitHeadYaw + 0.05;
                bossHeadingOffset(gCockpitRotY * 2, 120.0f, -50.0f, &w->frame->pos[0], &w->frame->pos[2]);
            }
            w->box->kind = 1;
            (*bossSetFrameFlags2)(w->frame, 0x20);
            w->frame->scale[2] = 1.2f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[1] = -100.0f;
            w->stage = 0;
            sndPlayPan(0x222, 0x5A);
            bossAddPower(1);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            aniAtkDelete(w->box);
            break;
        case 0:
            switch (w->stage) {
                case 0:
                    if (bossStepAnimAtk(w->frame, (AnimeFrame *) w->box, 1.2f, 2) != 0) {
                        w->time = 10;
                        w->stage++;
                    }
                    if (acollAtkWasHit(w->box) == 1) {
                        w->box->flags &= ~2;
                        angle = mathAtan2(aniAtkGetBox(w->box)->pos[0] - w->box->hitPos[0],
                                                       aniAtkGetBox(w->box)->pos[2] - w->box->hitPos[2]);
                        x = w->box->pos[0] + cosf(angle) * w->box->radius;
                        y = w->box->pos[1];
                        z = w->box->pos[2] + sinf(angle) * w->box->radius;
                        beHitringCreate(x, y, z, 1.0f, 0);
                        w->time = 5;
                        w->stage = 3;
                        if (gBossFlags & 0x4000) {
                            if (w->box->hitKind == 0xCC) {
                                SoundLocation2(0x150, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                                       w->frame->pos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                            }
                            PRINTF("げげ、ボス怒ってる。\n");
                            w->stage = 2;
                        }
                    }
                    break;
                case 1:
                    if (--w->time == 0) {
                        w->stage++;
                    }
                    if (acollAtkWasHit(w->box) == 1) {
                        w->box->flags &= ~2;
                        angle = mathAtan2(aniAtkGetBox(w->box)->pos[0] - w->box->hitPos[0],
                                                       aniAtkGetBox(w->box)->pos[2] - w->box->hitPos[2]);
                        x = w->box->pos[0] + cosf(angle) * w->box->radius;
                        y = w->box->pos[1];
                        z = w->box->pos[2] + sinf(angle) * w->box->radius;
                        beHitringCreate(x, y, z, 1.0f, 0);
                        w->time = 8;
                        w->stage = 3;
                        if (gBossFlags & 0x4000) {
                            if (w->box->hitKind == 0xCC) {
                                SoundLocation2(0x150, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                                       w->frame->pos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                            }
                            PRINTF("げげ、ボス怒ってる。\n");
                            w->stage = 2;
                        }
                    }
                    break;
                case 2:
                    w->box->flags &= ~2;
                    if (bossStepAnimAtk(w->frame, (AnimeFrame *) w->box, 1.2f, 3) != 0) {
                        return -1;
                    }
                    break;
                case 3:
                    if (--w->time == 0) {
                        PRINTF("ジャブヒット硬直\n");
                        w->stage = 2;
                    }
                    break;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 255);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Straight": a long punch (sound 0x223, a stronger rumble). On a hit while it extends it
 * freezes 10 frames and extends again; on a hit at full stretch it freezes 10 and draws back.
 * It gives up after 40 frames ("[str_proc]タイムアウトしました。"). */
s32 str_proc(Obj *obj, s16 msg, void *arg) {
    PunchWork *w = (PunchWork *) ((u32) obj + sizeof(Obj));
    f32 z;
    f32 angle;
    f32 x;
    f32 y;

    switch (msg) {
        case 2:
            obj->name = "Straight";
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            if (gBattleFlags & 0x4000000) {
                bossSetAnimAt(w->frame, impStraightPilot2Anim, 0xEA, 0.0f);
            } else {
                bossSetAnimAt(w->frame, impJabPilot1Anim, 0xEA, 0.0f);
            }
            if (gCockpitAimX >= 0.0f) {
                bossHeadingOffset(gCockpitRotY, 150.0f, -30.0f, &w->frame->pos[0], &w->frame->pos[2]);
            } else {
                bossHeadingOffset(gCockpitRotY * 2, 150.0f, -30.0f, &w->frame->pos[0], &w->frame->pos[2]);
            }
            w->box = aniAtkCreate(pl_atk_root_frame, &w->frame->f, (u32) impJabAtkChain,
                                            floadGetSetFile(0xEA, 0), 0);
            if (w->box == NULL) {
                return -1;
            }
            w->box->kind = 3;
            (*bossSetFrameFlags2)(w->frame, 0x20);
            w->frame->scale[2] = 1.5f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->rot[0] = gCockpitHeadPitch - 0.18;
            w->frame->rot[1] = gCockpitHeadYaw + 0.18;
            w->frame->pos[1] = -110.0f;
            w->stage = 0;
            w->hit = 0;
            w->count = 0;
            sndPlayPan(0x223, 0x5A);
            bossAddPower(2);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            aniAtkDelete(w->box);
            break;
        case 0:
            w->count++;
            switch (w->stage) {
                case 0:
                    if (bossStepAnimAtk(w->frame, (AnimeFrame *) w->box, 0.5f, 2) != 0) {
                        w->time = 20;
                        w->stage = 1;
                    }
                    if (acollAtkWasHit(w->box) == 1) {
                        w->box->flags &= ~2;
                        angle = mathAtan2(aniAtkGetBox(w->box)->pos[0] - w->box->hitPos[0],
                                                       aniAtkGetBox(w->box)->pos[2] - w->box->hitPos[2]);
                        x = w->box->pos[0] + cosf(angle) * w->box->radius;
                        y = w->box->pos[1];
                        z = w->box->pos[2] + sinf(angle) * w->box->radius;
                        beHitringCreate(x, y, z, 1.0f, 0);
                        w->time = 10;
                        w->stage = 3;
                        if (gBossFlags & 0x4000) {
                            if (w->box->hitKind == 0xCC) {
                                SoundLocation2(0x150, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                                       w->frame->pos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                            }
                            PRINTF("げげ、ボス怒ってる。\n");
                            w->stage = 2;
                        }
                    }
                    break;
                case 1:
                    if (--w->time == 0) {
                        w->stage = 2;
                    }
                    if (w->hit == 1) {
                        w->box->flags &= ~2;
                        w->stage = 2;
                    }
                    if (acollAtkWasHit(w->box) == 1) {
                        angle = mathAtan2(aniAtkGetBox(w->box)->pos[0] - w->box->hitPos[0],
                                                       aniAtkGetBox(w->box)->pos[2] - w->box->hitPos[2]);
                        x = w->box->pos[0] + cosf(angle) * w->box->radius;
                        y = w->box->pos[1];
                        z = w->box->pos[2] + sinf(angle) * w->box->radius;
                        beHitringCreate(x, y, z, 1.0f, 0);
                        w->time = 10;
                        w->stage = 4;
                        if (gBossFlags & 0x4000) {
                            if (w->box->hitKind == 0xCC) {
                                SoundLocation2(0x150, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                                       w->frame->pos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                            }
                            PRINTF("げげ、ボス怒ってる。\n");
                            w->stage = 2;
                        }
                    }
                    if (w->count > 40) {
                        PRINTF("[str_proc]タイムアウトしました。\n");
                        w->stage = 2;
                    }
                    break;
                case 2:
                    w->box->flags &= ~2;
                    if (bossStepAnimAtk(w->frame, (AnimeFrame *) w->box, 0.6f, 3) != 0) {
                        return -1;
                    }
                    break;
                case 3:
                    PRINTF("ヒット硬直フェイズ\n");
                    bossStepAnimAtk(w->frame, (AnimeFrame *) w->box, 0.01f, 2);
                    w->box->flags &= ~2;
                    if (--w->time == 0) {
                        w->stage = 0;
                        w->box->flags |= 2;
                    }
                    w->hit = 1;
                    if (w->count > 38) {
                        PRINTF("[str_proc]タイムアウトしました。\n");
                        w->stage = 2;
                    }
                    break;
                case 4:
                    PRINTF("伸び切りヒット硬直フェイズ\n");
                    w->box->flags &= ~2;
                    if (--w->time == 0) {
                        w->stage = 2;
                    }
                    if (w->count > 38) {
                        PRINTF("[str_proc]タイムアウトしました。\n");
                        w->stage = 2;
                    }
                    break;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 255);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "CStraight": the combo's straight, from the other side, with no time limit. */
s32 CStraight(Obj *obj, s16 msg, void *arg) {
    PunchWork *w = (PunchWork *) ((u32) obj + sizeof(Obj));
    f32 z;
    f32 angle;
    f32 x;
    f32 y;

    switch (msg) {
        case 2:
            obj->name = "CStraight";
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            if (gBattleFlags & 0x4000000) {
                bossSetAnimAt(w->frame, impStraightPilot2Anim, 0xEA, 0.0f);
            } else {
                bossSetAnimAt(w->frame, impJabPilot1Anim, 0xEA, 0.0f);
            }
            if (gCockpitAimX >= 0.0f) {
                bossHeadingOffset(gCockpitRotY * 2, -150.0f, -30.0f, &w->frame->pos[0], &w->frame->pos[2]);
            } else {
                bossHeadingOffset(gCockpitRotY, -150.0f, -30.0f, &w->frame->pos[0], &w->frame->pos[2]);
            }
            w->box = aniAtkCreate(pl_atk_root_frame, &w->frame->f, (u32) impJabAtkChain,
                                            floadGetSetFile(0xEA, 0), 0);
            if (w->box == NULL) {
                return -1;
            }
            w->box->kind = 3;
            (*bossSetFrameFlags2)(w->frame, 0x20);
            w->frame->scale[2] = 1.5f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->rot[0] = gCockpitHeadPitch - 0.18;
            w->frame->rot[1] = gCockpitHeadYaw - 0.18;
            w->frame->pos[1] = -110.0f;
            w->stage = 0;
            sndPlayPan(0x223, 0x26);
            bossAddPower(2);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            aniAtkDelete(w->box);
            break;
        case 0:
            switch (w->stage) {
                case 0:
                    if (bossStepAnimAtk(w->frame, (AnimeFrame *) w->box, 0.6f, 2) != 0) {
                        w->time = 20;
                        w->stage = 1;
                    }
                    if (acollAtkWasHit(w->box) == 1) {
                        w->box->flags &= ~2;
                        angle = mathAtan2(aniAtkGetBox(w->box)->pos[0] - w->box->hitPos[0],
                                                       aniAtkGetBox(w->box)->pos[2] - w->box->hitPos[2]);
                        x = w->box->pos[0] + cosf(angle) * w->box->radius;
                        y = w->box->pos[1];
                        z = w->box->pos[2] + sinf(angle) * w->box->radius;
                        beHitringCreate(x, y, z, 1.0f, 0);
                        w->time = 10;
                        w->stage = 3;
                        if (gBossFlags & 0x4000) {
                            if (w->box->hitKind == 0xCC) {
                                SoundLocation2(0x150, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                                       w->frame->pos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                            }
                            PRINTF("げげ、ボス怒ってる。\n");
                            w->stage = 2;
                        }
                    }
                    break;
                case 1:
                    if (--w->time == 0) {
                        w->stage = 2;
                    }
                    if (acollAtkWasHit(w->box) == 1) {
                        angle = mathAtan2(aniAtkGetBox(w->box)->pos[0] - w->box->hitPos[0],
                                                       aniAtkGetBox(w->box)->pos[2] - w->box->hitPos[2]);
                        x = w->box->pos[0] + cosf(angle) * w->box->radius;
                        y = w->box->pos[1];
                        z = w->box->pos[2] + sinf(angle) * w->box->radius;
                        beHitringCreate(x, y, z, 1.0f, 0);
                        w->time = 10;
                        w->stage = 4;
                        if (gBossFlags & 0x4000) {
                            if (w->box->hitKind == 0xCC) {
                                SoundLocation2(0x150, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                                       w->frame->pos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                            }
                            PRINTF("げげ、ボス怒ってる。\n");
                            w->stage = 2;
                        }
                    }
                    break;
                case 2:
                    w->box->flags &= ~2;
                    if (bossStepAnimAtk(w->frame, (AnimeFrame *) w->box, 0.6f, 3) != 0) {
                        return -1;
                    }
                    break;
                case 3:
                    PRINTF("ヒット硬直フェイズ\n");
                    w->box->flags &= ~2;
                    if (--w->time == 0) {
                        w->stage = 0;
                        w->box->flags |= 2;
                    }
                    break;
                case 4:
                    PRINTF("伸び切りヒット硬直フェイズ\n");
                    w->box->flags &= ~2;
                    if (--w->time == 0) {
                        w->stage = 2;
                    }
                    break;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 255);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Ovalcan": a vulcan shot from the other side (by gOvalcanSide, its right or left gun), 1000
 * above gBrotherPosX's point, flying along gBrotherYaw's heading at 40 and climbing by a tenth
 * of gBrotherAimY; harmless for its first 15 frames, spinning and shrinking to 0.3. A hit
 * ends it in sparks (glancing off if the boss guards); it ends in the fourth boss's water,
 * or 5000 away. The distance's y and z are read inline as named fields (only dx is a local):
 * a dy local takes a slot and pushes cosf's spill from the game's 0x34 to 0x30. */
s32 Ovalcan(Obj *obj, s16 msg, void *arg) {
    ShotWork *w = (ShotWork *) ((u32) obj + sizeof(Obj));
    f32 dx;

    switch (msg) {
        case 2:
            obj->name = "Ovalcan";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            if (gOvalcanSide == 0) {
                if (gBattleFlags & 0x4000000) {
                    bossSetPolDL(w->frame, impVulcanShotPilot1Dl, 0xEA);
                } else {
                    bossSetPolDL(w->frame, impVulcanShotPilot2Dl, 0xEA);
                }
                bossHeadingOffset(gBrotherYaw, 20.0f, 80.0f, &w->frame->pos[0], &w->frame->pos[2]);
                w->frame->pos[0] += gBrotherPosX;
                w->frame->pos[1] = gBrotherPosY + 1000.0f;
                w->frame->pos[2] += gBrotherPosZ;
            } else {
                if (gBattleFlags & 0x4000000) {
                    bossSetPolDL(w->frame, impVulcanShotPilot1Dl, 0xEA);
                } else {
                    bossSetPolDL(w->frame, impVulcanShotPilot2Dl, 0xEA);
                }
                bossHeadingOffset(gBrotherYaw, -20.0f, 80.0f, &w->frame->pos[0], &w->frame->pos[2]);
                w->frame->pos[0] += gBrotherPosX;
                w->frame->pos[1] = gBrotherPosY + 1000.0f;
                w->frame->pos[2] += gBrotherPosZ;
            }
            (*bossSetFrameFlags)(w->frame, 0x20);
            w->box = acollAtkAddYZ(uda_atk_root_frame, &w->frame->f, 10, 0, 0, 0);
            if (w->box == NULL) {
                return -1;
            }
            w->box->kind = 0x10;
            w->frame->scale[2] = 0.8f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->flags |= 3;
            w->vel[0] = sinf(gBrotherYaw) * 40.0f;
            w->vel[2] = cosf(gBrotherYaw) * 40.0f;
            w->vel[1] = gBrotherAimY * 0.1;
            w->time = 15;
            SoundLocation2(0x238, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                                   15000.0f, gBattleCamView->fovy);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->box->frame);
            break;
        case 0:
            if (gBattleFlags2 & 0x4000) {
                return -1;
            }
            if (gBattleFlags2 & 0x400) {
                return -1;
            }
            if (gBattleFlags & 0x10) {
                return -1;
            }
            if (--w->time < 0) {
                w->time = 0;
            }
            w->box->flags |= 2;
            if (w->time != 0) {
                w->box->flags &= ~2;
            }
            w->frame->rot[2] += 0.20943951604656094;
            w->frame->scale[0] -= 0.01;
            w->frame->scale[1] -= 0.01;
            w->frame->scale[2] -= 0.01;
            if (w->frame->scale[0] < 0.3) {
                w->frame->scale[2] = 0.3f;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            }
            if (acollAtkWasHit(w->box) == 1) {
                itexSparkleYellow(w->frame->pos[0], w->frame->pos[1] + 50.0f, w->frame->pos[2], 0.6f);
                if (gBossFlags & 4) {
                    if (w->box->hitKind == 0xCC) {
                        w->box->flags &= ~2;
                        itexGlowBlue(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 0.5f);
                        BmoveX(obj, cosf(fadeRand(350) * 0.0174532930038800785) * 1000.0f,
                                               sinf(fadeRand(350) * 0.0174532930038800785) * 1000.0f,
                                               w->frame->pos[2] * 0.6, 40.0f);
                        SoundLocation2(0x2AB, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                               300.0f, 15000.0f, gBattleCamView->fovy);
                        return -1;
                    }
                } else {
                    return -1;
                }
                return -1;
            }
            if (gBattleBossId == 4) {
                if (w->frame->pos[1] < -1200.0f) {
                    (*beShibukiCreate)(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 3);
                    itexGlowBlue(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 0.2f);
                    return -1;
                }
            }
            dx = w->frame->pos[0] - gBrotherPosX;
            w->dist = dx * dx +
                      (((Vec3f *) w->frame->pos)->y - gBrotherPosY) *
                          (((Vec3f *) w->frame->pos)->y - gBrotherPosY) +
                      (((Vec3f *) w->frame->pos)->z - gBrotherPosZ) *
                          (((Vec3f *) w->frame->pos)->z - gBrotherPosZ);
            if (w->dist > 25000000.0f) {
                return -1;
            }
            bossApplyVel(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Kick": the Impact's leg (by pilot and side) twelve times its size below the boss's
 * height, with an attack box chain (kind 8); a sound by pilot (0x325 or 0x224) and a strong
 * rumble. A hit freezes it 5 frames and it kicks on; it gives up after 60 frames. */
s32 kick_proc(Obj *obj, s16 msg, void *arg) {
    PunchWork *w = (PunchWork *) ((u32) obj + sizeof(Obj));
    f32 z;
    f32 angle;
    f32 x;
    f32 y;

    switch (msg) {
        case 2:
            obj->name = "Kick";
            if (gCockpitAimX >= 0.0f) {
                w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
                if (w->frame == NULL) {
                    return -1;
                }
                if (gBattleFlags & 0x4000000) {
                    bossSetAnimAt(w->frame, impKickRightPilot2Anim, 0xEA, 0.0f);
                } else {
                    bossSetAnimAt(w->frame, impArmRightAnim, 0xEA, 0.0f);
                }
                w->box = aniAtkCreate(pl_atk_root_frame, &w->frame->f, (u32) impKickRightAtkChain,
                                                floadGetSetFile(0xEA, 0), 0);
                if (w->box == NULL) {
                    return -1;
                }
                bossHeadingOffset(gCockpitRotY, 100.0f, -140.0f, &w->frame->pos[0], &w->frame->pos[2]);
                w->frame->pos[1] = gCockpitAimY - 140.0f;
                w->frame->rot[0] = gCockpitHeadPitch + 0.1;
                w->frame->rot[1] = gCockpitHeadYaw + 0.1;
            } else {
                w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
                if (w->frame == NULL) {
                    return -1;
                }
                if (gBattleFlags & 0x4000000) {
                    bossSetAnimAt(w->frame, impKickLeftPilot2Anim, 0xEA, 0.0f);
                } else {
                    bossSetAnimAt(w->frame, impArmLeftAnim, 0xEA, 0.0f);
                }
                w->box = aniAtkCreate(pl_atk_root_frame, &w->frame->f, (u32) impKickLeftAtkChain,
                                                floadGetSetFile(0xEA, 0), 0);
                if (w->box == NULL) {
                    return -1;
                }
                w->frame->scale[2] = 4.0f;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                bossHeadingOffset(gCockpitRotY, -100.0f, -140.0f, &w->frame->pos[0], &w->frame->pos[2]);
                w->frame->pos[1] = gCockpitAimY - 140.0f;
                w->frame->rot[0] = gCockpitHeadPitch + 0.1;
                w->frame->rot[1] = gCockpitHeadYaw - 0.1;
            }
            w->frame->scale[2] = 12.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->box->kind = 8;
            w->hit = 0;
            w->count = 0;
            if (gBattleFlags & 0x4000000) {
                (*sndPlay)(0x325);
            } else {
                (*sndPlay)(0x224);
            }
            bossAddPower(3);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            aniAtkDelete(w->box);
            break;
        case 0:
            w->count++;
            switch (w->stage) {
                case 0:
                    w->box->flags |= 2;
                    if (acollAtkWasHit(w->box) == 1) {
                        w->box->flags &= ~2;
                        angle = mathAtan2(aniAtkGetBox(w->box)->pos[0] - w->box->hitPos[0],
                                                       aniAtkGetBox(w->box)->pos[2] - w->box->hitPos[2]);
                        x = w->box->pos[0] + cosf(angle) * w->box->radius;
                        y = w->box->pos[1];
                        z = w->box->pos[2] + sinf(angle) * w->box->radius;
                        beHitringCreate(x, y, z, 1.0f, 0);
                        w->time = 5;
                        w->stage = 3;
                        if (gBossFlags & 0x4000) {
                            w->box->flags &= ~2;
                            if (w->box->hitKind == 0xCC) {
                                SoundLocation2(0x150, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                                       w->frame->pos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                            }
                            PRINTF("げげ、ボス怒ってる。\n");
                            w->stage = 2;
                        }
                    }
                    if (bossStepAnimAtk(w->frame, (AnimeFrame *) w->box, 0.4f, 2) != 0) {
                        w->time = 20;
                        w->stage = 1;
                    }
                    break;
                case 1:
                    if (--w->time == 0) {
                        w->stage++;
                    }
                    if (acollAtkWasHit(w->box) == 1) {
                        w->box->flags &= ~2;
                        angle = mathAtan2(aniAtkGetBox(w->box)->pos[0] - w->box->hitPos[0],
                                                       aniAtkGetBox(w->box)->pos[2] - w->box->hitPos[2]);
                        x = w->box->pos[0] + cosf(angle) * w->box->radius;
                        y = w->box->pos[1];
                        z = w->box->pos[2] + sinf(angle) * w->box->radius;
                        beHitringCreate(x, y, z, 1.0f, 0);
                        w->time = 5;
                        w->stage = 3;
                        if (gBossFlags & 0x4000) {
                            if (w->box->hitKind == 0xCC) {
                                SoundLocation2(0x150, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                                       w->frame->pos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                            }
                            PRINTF("げげ、ボス怒ってる。\n");
                            w->stage = 2;
                        }
                    }
                    if (w->count > 60) {
                        PRINTF("[kick_proc]タイムアウトしました。\n");
                        w->stage = 2;
                    }
                    break;
                case 2:
                    w->box->flags &= ~2;
                    if (bossStepAnimAtk(w->frame, (AnimeFrame *) w->box, 0.4f, 3) != 0) {
                        return -1;
                    }
                    w->box->flags &= ~2;
                    break;
                case 3:
                    bossStepAnimAtk(w->frame, (AnimeFrame *) w->box, 0.4f, 2);
                    w->hit = 1;
                    if (--w->time == 0) {
                        w->stage = 0;
                    }
                    if (w->count > 60) {
                        PRINTF("[kick_proc]タイムアウトしました。\n");
                        w->stage = 2;
                    }
                    break;
                case 4:
                    if (--w->time == 0) {
                        w->stage = 0;
                    }
                    if (w->count > 60) {
                        PRINTF("[kick_proc]タイムアウトしました。\n");
                        w->stage = 2;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The second "Kick": the same, tilted back (-0.18), giving up after 60 frames even on a
 * first hit. */
s32 kick_proc2(Obj *obj, s16 msg, void *arg) {
    PunchWork *w = (PunchWork *) ((u32) obj + sizeof(Obj));
    f32 z;
    f32 angle;
    f32 x;
    f32 y;

    switch (msg) {
        case 2:
            obj->name = "Kick";
            if (gCockpitAimX >= 0.0f) {
                w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
                if (w->frame == NULL) {
                    return -1;
                }
                if (gBattleFlags & 0x4000000) {
                    bossSetAnimAt(w->frame, impKickRightPilot2Anim, 0xEA, 0.0f);
                } else {
                    bossSetAnimAt(w->frame, impArmRightAnim, 0xEA, 0.0f);
                }
                w->box = aniAtkCreate(pl_atk_root_frame, &w->frame->f, (u32) impKickRightAtkChain,
                                                floadGetSetFile(0xEA, 0), 0);
                if (w->box == NULL) {
                    return -1;
                }
                bossHeadingOffset(gCockpitRotY, 100.0f, -140.0f, &w->frame->pos[0], &w->frame->pos[2]);
                w->frame->pos[1] = gCockpitAimY - 140.0f;
                w->frame->rot[0] = -0.18f;
                w->frame->rot[1] = gCockpitHeadYaw + 0.1;
            } else {
                w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
                if (w->frame == NULL) {
                    return -1;
                }
                if (gBattleFlags & 0x4000000) {
                    bossSetAnimAt(w->frame, impKickLeftPilot2Anim, 0xEA, 0.0f);
                } else {
                    bossSetAnimAt(w->frame, impArmLeftAnim, 0xEA, 0.0f);
                }
                w->box = aniAtkCreate(pl_atk_root_frame, &w->frame->f, (u32) impKickLeftAtkChain,
                                                floadGetSetFile(0xEA, 0), 0);
                if (w->box == NULL) {
                    return -1;
                }
                w->frame->scale[2] = 4.0f;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                bossHeadingOffset(gCockpitRotY, -100.0f, -140.0f, &w->frame->pos[0], &w->frame->pos[2]);
                w->frame->pos[1] = gCockpitAimY - 140.0f;
                w->frame->rot[0] = -0.18f;
                w->frame->rot[1] = gCockpitHeadYaw - 0.05;
            }
            w->frame->scale[2] = 12.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->box->kind = 8;
            w->hit = 0;
            w->count = 0;
            if (gBattleFlags & 0x4000000) {
                (*sndPlay)(0x325);
            } else {
                (*sndPlay)(0x224);
            }
            bossAddPower(3);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            aniAtkDelete(w->box);
            break;
        case 0:
            w->count++;
            switch (w->stage) {
                case 0:
                    w->box->flags |= 2;
                    if (acollAtkWasHit(w->box) == 1) {
                        w->box->flags &= ~2;
                        angle = mathAtan2(aniAtkGetBox(w->box)->pos[0] - w->box->hitPos[0],
                                                       aniAtkGetBox(w->box)->pos[2] - w->box->hitPos[2]);
                        x = w->box->pos[0] + cosf(angle) * w->box->radius;
                        y = w->box->pos[1];
                        z = w->box->pos[2] + sinf(angle) * w->box->radius;
                        beHitringCreate(x, y, z, 1.0f, 0);
                        w->time = 5;
                        w->stage = 3;
                        if (gBossFlags & 0x4000) {
                            w->box->flags &= ~2;
                            if (w->box->hitKind == 0xCC) {
                                SoundLocation2(0x150, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                                       w->frame->pos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                            }
                            PRINTF("げげ、ボス怒ってる。\n");
                            w->stage = 2;
                        }
                        if (w->count > 60) {
                            PRINTF("[kick_proc]タイムアウトしました。\n");
                            w->stage = 2;
                            break;
                        }
                    }
                    if (bossStepAnimAtk(w->frame, (AnimeFrame *) w->box, 0.4f, 2) != 0) {
                        w->time = 20;
                        w->stage = 1;
                    }
                    break;
                case 1:
                    if (--w->time == 0) {
                        w->stage++;
                    }
                    if (acollAtkWasHit(w->box) == 1) {
                        w->box->flags &= ~2;
                        angle = mathAtan2(aniAtkGetBox(w->box)->pos[0] - w->box->hitPos[0],
                                                       aniAtkGetBox(w->box)->pos[2] - w->box->hitPos[2]);
                        x = w->box->pos[0] + cosf(angle) * w->box->radius;
                        y = w->box->pos[1];
                        z = w->box->pos[2] + sinf(angle) * w->box->radius;
                        beHitringCreate(x, y, z, 1.0f, 0);
                        w->time = 5;
                        w->stage = 3;
                        if (gBossFlags & 0x4000) {
                            w->box->flags &= ~2;
                            if (w->box->hitKind == 0xCC) {
                                SoundLocation2(0x150, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                                       w->frame->pos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                            }
                            PRINTF("げげ、ボス怒ってる。\n");
                            w->stage = 2;
                        }
                    }
                    if (w->count > 60) {
                        PRINTF("[kick_proc]タイムアウトしました。\n");
                        w->stage = 2;
                    }
                    break;
                case 2:
                    if (bossStepAnimAtk(w->frame, (AnimeFrame *) w->box, 0.4f, 3) != 0) {
                        return -1;
                    }
                    w->box->flags &= ~2;
                    break;
                case 3:
                    bossStepAnimAtk(w->frame, (AnimeFrame *) w->box, 0.4f, 2);
                    if (--w->time == 0) {
                        w->stage = 0;
                    }
                    if (w->count > 60) {
                        PRINTF("[kick_proc]タイムアウトしました。\n");
                        w->stage = 2;
                    }
                    break;
                case 4:
                    if (--w->time == 0) {
                        w->stage = 0;
                    }
                    if (w->count > 60) {
                        PRINTF("[kick_proc]タイムアウトしました。\n");
                        w->stage = 2;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The guarding arm (unnamed): on the boss's side of the Impact (by gCockpitAimX), 90 out and
 * 70 down, turned to the boss; it rings (sound 0x224), rumbles, plays 70% of its animation,
 * holds 10 frames, plays out at 1.2 and ends. */
s32 impGuardArm(Obj *obj, s16 msg, void *arg) {
    ArmWork *w = (ArmWork *) ((u32) obj + sizeof(Obj));

    switch (msg) {
        case 2:
            if (gCockpitAimX >= 0.0f) {
                w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
                if (w->frame == NULL) {
                    return -1;
                }
                bossSetAnimAt(w->frame, impArmRightAnim, 0xEA, 0.0f);
                w->frame->scale[2] = 6.0f;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                bossHeadingOffset(gCockpitRotY, 90.0f, 0.0f, &w->frame->pos[0], &w->frame->pos[2]);
                w->frame->pos[1] = -70.0f;
                w->frame->rot[1] = gCockpitHeadYaw + 3.1415927f + 0.1;
                w->frame->rot[2] = 1.4660934f;
            } else {
                w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
                if (w->frame == NULL) {
                    return -1;
                }
                bossSetAnimAt(w->frame, impArmLeftAnim, 0xEA, 0.0f);
                bossHeadingOffset(gCockpitRotY, -90.0f, 0.0f, &w->frame->pos[0], &w->frame->pos[2]);
                w->frame->pos[1] = -70.0f;
                w->frame->scale[2] = 6.0f;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                w->frame->rot[1] = gCockpitHeadYaw + 3.1415927f - 0.1;
                w->frame->rot[2] = -1.4660934f;
            }
            sndPlayPan(0x224, 0x40);
            bossAddPower(3);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            switch (w->stage) {
                case 0:
                    w->frame->frame += 1.0;
                    if (polAnimeLastFrame(w->frame) * 70 * 0.01 < w->frame->frame) {
                        w->frame->frame = polAnimeLastFrame(w->frame) * 70 * 0.01;
                        w->time = 10;
                        w->stage++;
                    }
                    break;
                case 1:
                    if (--w->time == 0) {
                        w->stage++;
                    }
                    break;
                case 2:
                    w->frame->frame += 1.2;
                    if (polAnimeLastFrame(w->frame) < w->frame->frame) {
                        tskDestroy(obj);
                        return 0;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Guard": the Impact's guard (by pilot, and for one pilot by side), at the boss's heading;
 * it plays in, then holds, dimming to 0xD0, until the pilots' buttons let go, then plays out
 * and clears the attack flags (0x1000 of gBattleFlags and the rest). */
s32 Guard(Obj *obj, s16 msg, void *arg) {
    GuardWork *w = (GuardWork *) ((u32) obj + sizeof(Obj));

    switch (msg) {
        case 2:
            obj->name = "Guard";
            gBattleFlags |= 0x1000;
            w->frame = (AnimeFrame *) frmCreateNext(gFrmLayer8, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            if (gBattleFlags & 0x4000000) {
                if (gCockpitAimX >= 0.0f) {
                    bossSetAnimAt(w->frame, impGuardPilot2AimRightAnim, 0xEA, 0.0f);
                } else {
                    bossSetAnimAt(w->frame, impGuardPilot2AimLeftAnim, 0xEA, 0.0f);
                }
            } else {
                bossSetAnimAt(w->frame, impGuardPilot1Anim, 0xEA, 0.0f);
            }
            w->frame->scale[2] = 0.7f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[1] = -100.0f;
            w->frame->rot[1] = gCockpitHeadYaw;
            w->alpha = 255;
            w->stage = 0;
            break;
        case 0:
            switch (w->stage) {
                case 0:
                    if (bossStepAnim(w->frame, 0.1f, 2) != 0) {
                        w->stage++;
                    }
                    break;
                case 1:
                    w->alpha--;
                    if (w->alpha < 0xD0) {
                        w->alpha = 0xD0;
                    }
                    bossStepAnim(w->frame, 0.1f, 2);
                    if (joyGetHeldTime(gCockpitPort, gCockpitBtnJab) == 0 ||
                        joyGetHeldTime(gCockpitPort, gCockpitBtnStraight) == 0) {
                        w->stage++;
                    }
                    break;
                case 2:
                    if (bossStepAnim(w->frame, 0.1f, 3) != 0) {
                        gBattleFlags &= ~0x400000;
                        gBattleFlags &= ~0x1000;
                        gBattleFlags &= ~0x20000;
                        gBattleFlags &= ~0x40000;
                        gBattleFlags &= ~0x80000;
                        gBattleFlags &= ~0x100000;
                        gBattleFlags &= ~0x800000;
                        gBattleFlags &= ~4;
                        return -1;
                    }
                    break;
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Pgage": the power gauge (gImpactPower, 0-100): a box and a bar played to the gauge's share;
 * the bar pulses (faster over 50), and when full it flashes purple and yellow. */
s32 Pgage(Obj *obj, s16 msg, void *arg) {
    PgageWork *w = (PgageWork *) ((u32) obj + sizeof(Obj));
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "Pgage";
            w->bar = (AnimeFrame *) frmCreate(gCockpitPanelFrame, gFrmKindAnime);
            if (w->bar == NULL) {
                return -1;
            }
            w->frame = (PolFrame *) frmCreate(gCockpitRootFrame, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, impPowerGaugeDl, 0xEB);
            (*bossSetFrameFlags)(w->frame, 0x20);
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 0.0f;
            bossSetAnimAt(w->bar, impPowerGaugeBarAnim, 0xEB, 0.0f);
            (*bossSetFrameFlags2)(w->bar, 0x20);
            w->bar->scale[2] = 0.1f;
            w->bar->scale[0] = w->bar->scale[1] = w->bar->scale[2];
            w->bar->pos[0] = 0.0f;
            w->bar->pos[1] = 0.0f;
            w->bar->pos[2] = 0.0f;
            w->alpha = 0;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->bar->f);
            break;
        case 0:
            if (gBattleFlags & 0x1000000) {
                w->frame->f.flags &= ~2;
                w->bar->f.flags &= ~2;
            } else {
                w->frame->f.flags |= 2;
                w->bar->f.flags |= 2;
            }
            w->frame->rot[0] = gCockpitRotX;
            w->frame->rot[1] = gCockpitRotY;
            w->frame->rot[2] = gCockpitRotZ;
            w->bar->rot[0] = gCockpitRotX;
            w->bar->rot[1] = gCockpitRotY;
            w->bar->rot[2] = gCockpitRotZ;
            if (gImpactPower > 50) {
                if ((gGameFrameCount & 0x1F) >= 16) {
                    w->alpha = 256 - (gGameFrameCount & 0xF) * 16;
                } else {
                    w->alpha = (gGameFrameCount & 0xF) * 16;
                }
                if (w->alpha >= 255) {
                    w->alpha = 255;
                }
                if (w->alpha <= 0) {
                    w->alpha = 0;
                }
            } else {
                if ((gGameFrameCount & 0x3F) >= 32) {
                    w->alpha = 256 - (gGameFrameCount & 0x1F) * 8;
                } else {
                    w->alpha = (gGameFrameCount & 0x1F) * 8;
                }
                if (w->alpha >= 255) {
                    w->alpha = 255;
                }
                if (w->alpha <= 0) {
                    w->alpha = 0;
                }
            }
            if (gImpactPower > 100) {
                gImpactPower = 100;
            }
            bossSetAnimAt(w->bar, impPowerGaugeBarAnim, 0xEB, gImpactPower * 0.01f);
            break;
        case 3:
            if (gImpactPower == 100) {
                if (gGameFrameCount % 6 >= 3) {
                    w->bar->dl = (*bossXluPrimEnvColorDL)(230, 0, 230, 255);
                } else {
                    w->bar->dl = (*bossXluPrimEnvColorDL)(230, 230, 50, 255);
                }
            } else {
                w->bar->dl = (*bossXluPrimEnvColorDL)(250, 250, 0, w->alpha);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "money": the 0-999 gauge (gImpactMoney) in three digits on a panel (by pilot). */
s32 money(Obj *obj, s16 msg, void *arg) {
    CounterWork *w = (CounterWork *) ((u32) obj + sizeof(Obj));
    s32 pad;
    s32 d100;
    s32 d10;
    s32 d1;
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "money";
            w->frame = (PolFrame *) frmCreate(gCockpitPanelFrame, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, impMoneyPanelPilot1Dl, 0xEB);
            (*bossSetFrameFlags)(w->frame, 0x20);
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 0.0f;
            w->alpha = 255;
            fadeAnimStart(&w->digits[0], gImpactDigitFrames, 0, &w->frame->segs[3],
                                   floadGetSetFile(0xEB, 0));
            fadeAnimStart(&w->digits[1], gImpactDigitFrames, 0, &w->frame->segs[4],
                                   floadGetSetFile(0xEB, 0));
            fadeAnimStart(&w->digits[2], gImpactDigitFrames, 0, &w->frame->segs[5],
                                   floadGetSetFile(0xEB, 0));
            break;
        case 0:
            if (gBattleFlags & 0x1000000) {
                w->frame->f.flags &= ~2;
            } else {
                w->frame->f.flags |= 2;
            }
            if (gBattleFlags & 0x4000000) {
                bossSetPolDL(w->frame, impMoneyPanelPilot2Dl, 0xEB);
            } else {
                bossSetPolDL(w->frame, impMoneyPanelPilot1Dl, 0xEB);
            }
            d100 = gImpactMoney * 0.01;
            d10 = (gImpactMoney - d100 * 100) * 0.1;
            d1 = (gImpactMoney - d100 * 100) - d10 * 10;
            fadeAnimStart(&w->digits[0], gImpactDigitFrames, d100, &w->frame->segs[3],
                                   floadGetSetFile(0xEB, 0));
            fadeAnimStart(&w->digits[1], gImpactDigitFrames, d10, &w->frame->segs[4],
                                   floadGetSetFile(0xEB, 0));
            fadeAnimStart(&w->digits[2], gImpactDigitFrames, d1, &w->frame->segs[5],
                                   floadGetSetFile(0xEB, 0));
            w->frame->rot[0] = gCockpitRotX;
            w->frame->rot[1] = gCockpitRotY;
            w->frame->rot[2] = gCockpitRotZ;
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "fuel": the pilot's life (gImpactArmor, or gImpactArmor2 for the other pilot) in three digits. */
s32 fuel(Obj *obj, s16 msg, void *arg) {
    CounterWork *w = (CounterWork *) ((u32) obj + sizeof(Obj));
    s32 pad;
    s32 d100;
    s32 d10;
    s32 d1;
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "fuel";
            w->frame = (PolFrame *) frmCreate(gCockpitPanelFrame, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, impFuelPanelPilot1Dl, 0xEB);
            (*bossSetFrameFlags)(w->frame, 0x20);
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 0.0f;
            w->alpha = 255;
            fadeAnimStart(&w->digits[0], gImpactDigitFrames, 0, &w->frame->segs[3],
                                   floadGetSetFile(0xEB, 0));
            fadeAnimStart(&w->digits[1], gImpactDigitFrames, 0, &w->frame->segs[4],
                                   floadGetSetFile(0xEB, 0));
            fadeAnimStart(&w->digits[2], gImpactDigitFrames, 0, &w->frame->segs[5],
                                   floadGetSetFile(0xEB, 0));
            break;
        case 0:
            if (gBattleFlags & 0x1000000) {
                w->frame->f.flags &= ~2;
            } else {
                w->frame->f.flags |= 2;
            }
            if (gBattleFlags & 0x4000000) {
                bossSetPolDL(w->frame, impFuelPanelPilot2Dl, 0xEB);
            } else {
                bossSetPolDL(w->frame, impFuelPanelPilot1Dl, 0xEB);
            }
            if (gBattleFlags & 0x4000000) {
                d100 = gImpactArmor2 * 0.01;
                d10 = (gImpactArmor2 - d100 * 100) * 0.1;
                d1 = (gImpactArmor2 - d100 * 100) - d10 * 10;
            } else {
                d100 = gImpactArmor * 0.01;
                d10 = (gImpactArmor - d100 * 100) * 0.1;
                d1 = (gImpactArmor - d100 * 100) - d10 * 10;
            }
            fadeAnimStart(&w->digits[0], gImpactDigitFrames, d100, &w->frame->segs[3],
                                   floadGetSetFile(0xEB, 0));
            fadeAnimStart(&w->digits[1], gImpactDigitFrames, d10, &w->frame->segs[4],
                                   floadGetSetFile(0xEB, 0));
            fadeAnimStart(&w->digits[2], gImpactDigitFrames, d1, &w->frame->segs[5],
                                   floadGetSetFile(0xEB, 0));
            w->frame->rot[0] = gCockpitRotX;
            w->frame->rot[1] = gCockpitRotY;
            w->frame->rot[2] = gCockpitRotZ;
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "bosslife": the boss's life (gBossLife) in four digits on two panels (by pilot), and a
 * third panel; the thousands show 0 below 1000. The scale copies go through a float local
 * `s` (one local slot above the digits) where the other gauges chain them: the chains cost
 * a compiler temp, the local takes a slot instead, and the digits' homes land at 0x38-0x44. */
s32 bosslife(Obj *obj, s16 msg, void *arg) {
    BossLifeWork *w = (BossLifeWork *) ((u32) obj + sizeof(Obj));
    f32 s;
    s32 d1000;
    s32 d100;
    s32 d10;
    s32 d1;

    switch (msg) {
        case 2:
            obj->name = "bosslife";
            w->frame = (PolFrame *) frmCreate(gCockpitPanelFrame, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->f1 = (PolFrame *) frmCreate(gCockpitPanelFrame, gFrmKindPol);
            if (w->f1 == NULL) {
                return -1;
            }
            w->f2 = (PolFrame *) frmCreate(gCockpitPanelFrame, gFrmKindPol);
            if (w->f2 == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, impBossLifePilot2Dl, 0xEB);
            (*bossSetFrameFlags)(w->frame, 0x20);
            w->frame->scale[2] = 0.1f;
            s = w->frame->scale[2];
            w->frame->scale[1] = s;
            w->frame->scale[0] = s;
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 0.0f;
            fadeAnimStart(&w->digits[0], gImpactDigitFrames, 4, &w->frame->segs[3],
                                   floadGetSetFile(0xEB, 0));
            fadeAnimStart(&w->digits[1], gImpactDigitFrames, 6, &w->frame->segs[4],
                                   floadGetSetFile(0xEB, 0));
            bossSetPolDL(w->f1, impBossLifeF1Pilot2Dl, 0xEB);
            (*bossSetFrameFlags)(w->f1, 0x20);
            w->f1->scale[2] = 0.1f;
            s = w->f1->scale[2];
            w->f1->scale[1] = s;
            w->f1->scale[0] = s;
            w->f1->pos[0] = 0.0f;
            w->f1->pos[1] = 0.0f;
            w->f1->pos[2] = 0.0f;
            fadeAnimStart(&w->digits[2], gImpactDigitFrames, 4, &w->f1->segs[3],
                                   floadGetSetFile(0xEB, 0));
            fadeAnimStart(&w->digits[3], gImpactDigitFrames, 9, &w->f1->segs[4],
                                   floadGetSetFile(0xEB, 0));
            bossSetPolDL(w->f2, impBossLifeF2Dl, 0xEB);
            (*bossSetFrameFlags)(w->f2, 0x20);
            w->f2->scale[2] = 0.1f;
            s = w->f2->scale[2];
            w->f2->scale[1] = s;
            w->f2->scale[0] = s;
            w->alpha = 255;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->f1->f);
            frmDestroy(&w->f2->f);
            break;
        case 0:
            if (gBattleFlags & 0x1000000) {
                w->frame->f.flags &= ~2;
                w->f1->f.flags &= ~2;
                w->f2->f.flags &= ~2;
            } else {
                w->frame->f.flags |= 2;
                w->f1->f.flags |= 2;
                w->f2->f.flags |= 2;
            }
            if (gBattleFlags & 0x4000000) {
                bossSetPolDL(w->frame, impBossLifePilot2Dl, 0xEB);
                bossSetPolDL(w->f1, impBossLifeF1Pilot2Dl, 0xEB);
            } else {
                bossSetPolDL(w->frame, impBossLifePilot1Dl, 0xEB);
                bossSetPolDL(w->f1, impBossLifeF1Pilot1Dl, 0xEB);
            }
            w->frame->rot[0] = gCockpitRotX;
            w->frame->rot[1] = gCockpitRotY;
            w->frame->rot[2] = gCockpitRotZ;
            w->f1->rot[0] = gCockpitRotX;
            w->f1->rot[1] = gCockpitRotY;
            w->f1->rot[2] = gCockpitRotZ;
            w->f2->rot[0] = gCockpitRotX;
            w->f2->rot[1] = gCockpitRotY;
            w->f2->rot[2] = gCockpitRotZ;
            d1000 = gBossLife * 0.001;
            d100 = (gBossLife - d1000 * 1000) * 0.01;
            d10 = ((gBossLife - d1000 * 1000) - d100 * 100) * 0.1;
            d1 = ((gBossLife - d1000 * 1000) - d100 * 100) - d10 * 10;
            if (gBossLife < 1000) {
                d1000 = 0;
            }
            fadeAnimStart(&w->digits[0], gImpactDigitFrames, d1, &w->frame->segs[3],
                                   floadGetSetFile(0xEB, 0));
            fadeAnimStart(&w->digits[1], gImpactDigitFrames, d10, &w->frame->segs[4],
                                   floadGetSetFile(0xEB, 0));
            fadeAnimStart(&w->digits[2], gImpactDigitFrames, d100, &w->f1->segs[3],
                                   floadGetSetFile(0xEB, 0));
            fadeAnimStart(&w->digits[3], gImpactDigitFrames, d1000, &w->f1->segs[4],
                                   floadGetSetFile(0xEB, 0));
            break;
        case 3:
            w->frame->dl = (*bossXluPrimEnvColorDL)(0, 0, 0, w->alpha);
            w->f1->dl = (*bossXluPrimEnvColorDL)(0, 0, 0, w->alpha);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "Storm" of a kind. */
void impStormCreate(s32 kind) {
    StormArg arg;

    arg.kind = kind;
    tskCreate(gEffectTask, Storm, 0, &arg);
}

/* "Storm": one fist of the hundred-punch (百烈パンチ), by kind: which arm, which side of the
 * Impact, which angle; while the punches fly (flag 0x4000 of gBattleFlags2) the fists are
 * bigger and further out, kinds 6-11 a little smaller. The first six alternate two sounds.
 * It plays its animation, holds 5 frames, then moves under gFrmLayer8 at half strength and
 * plays out, fading by 3. It ends if the Impact is down. */
s32 Storm(Obj *obj, s16 msg, void *arg) {
    StormWork *w = (StormWork *) ((u32) obj + sizeof(Obj));

    switch (msg) {
        case 2:
            obj->name = "Storm";
            w->kind = ((StormArg *) arg)->kind;
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->scale[2] = 1.2f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            if (gBattleFlags & 2) {
                PRINTF("インパクト死亡してるので百烈パンチを消します。\n");
                return -1;
            }
            if (gBattleFlags2 & 0x4000) {
                w->frame->scale[2] = 1.5f;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                if (w->kind == 0 || w->kind == 6) {
                    if (gBattleFlags & 0x4000000) {
                        bossSetAnimAt(w->frame, impStraightPilot2Anim, 0xEA, 0.3f);
                    } else {
                        bossSetAnimAt(w->frame, impJabPilot1Anim, 0xEA, 0.3f);
                    }
                    if (gCockpitAimX >= 0.0f) {
                        bossHeadingOffset(gCockpitRotY * 2, -275.0f, 80.0f, &w->frame->pos[0], &w->frame->pos[2]);
                    } else {
                        bossHeadingOffset(gCockpitRotY, -275.0f, 80.0f, &w->frame->pos[0], &w->frame->pos[2]);
                    }
                    w->frame->pos[1] = -140.0f;
                    w->frame->rot[0] = gCockpitHeadPitch - 0.2;
                    w->frame->rot[1] = gCockpitHeadYaw;
                    if (w->kind == 6) {
                        w->frame->scale[2] = 1.2f;
                        w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                        w->frame->rot[0] = gCockpitHeadPitch - 0.3;
                    }
                }
                if (w->kind == 1 || w->kind == 7) {
                    if (gBattleFlags & 0x4000000) {
                        bossSetAnimAt(w->frame, impStraightPilot2Anim, 0xEA, 0.3f);
                    } else {
                        bossSetAnimAt(w->frame, impJabPilot1Anim, 0xEA, 0.3f);
                    }
                    if (gCockpitAimX >= 0.0f) {
                        bossHeadingOffset(gCockpitRotY, 275.0f, 80.0f, &w->frame->pos[0], &w->frame->pos[2]);
                    } else {
                        bossHeadingOffset(gCockpitRotY * 2, 275.0f, 80.0f, &w->frame->pos[0], &w->frame->pos[2]);
                    }
                    w->frame->pos[1] = -140.0f;
                    w->frame->rot[0] = gCockpitHeadPitch - 0.2;
                    w->frame->rot[1] = gCockpitHeadYaw;
                    if (w->kind == 7) {
                        w->frame->scale[2] = 1.2f;
                        w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                        w->frame->rot[0] = gCockpitHeadPitch - 0.3;
                    }
                }
                if (w->kind == 2 || w->kind == 8) {
                    if (gBattleFlags & 0x4000000) {
                        bossSetAnimAt(w->frame, impStormFist2Pilot2Anim, 0xEA, 0.3f);
                    } else {
                        bossSetAnimAt(w->frame, impStormFist2Pilot1Anim, 0xEA, 0.3f);
                    }
                    if (gCockpitAimX >= 0.0f) {
                        bossHeadingOffset(gCockpitRotY * 2, -275.0f, 80.0f, &w->frame->pos[0], &w->frame->pos[2]);
                    } else {
                        bossHeadingOffset(gCockpitRotY, -275.0f, 80.0f, &w->frame->pos[0], &w->frame->pos[2]);
                    }
                    w->frame->pos[1] = -140.0f;
                    w->frame->rot[0] = gCockpitHeadPitch + 0.2;
                    w->frame->rot[1] = gCockpitHeadYaw + 0.2;
                    if (w->kind == 8) {
                        w->frame->scale[2] = 1.2f;
                        w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                        w->frame->rot[0] = gCockpitHeadPitch + 0.3;
                    }
                }
                if (w->kind == 3 || w->kind == 9) {
                    if (gBattleFlags & 0x4000000) {
                        bossSetAnimAt(w->frame, impStormFist3Pilot2Anim, 0xEA, 0.3f);
                    } else {
                        bossSetAnimAt(w->frame, impStormFist3Pilot1Anim, 0xEA, 0.3f);
                    }
                    if (gCockpitAimX >= 0.0f) {
                        bossHeadingOffset(gCockpitRotY, 275.0f, 80.0f, &w->frame->pos[0], &w->frame->pos[2]);
                    } else {
                        bossHeadingOffset(gCockpitRotY * 2, 275.0f, 80.0f, &w->frame->pos[0], &w->frame->pos[2]);
                    }
                    w->frame->pos[1] = -140.0f;
                    w->frame->rot[0] = gCockpitHeadPitch + 0.2;
                    w->frame->rot[1] = gCockpitHeadYaw + 0.2;
                    if (w->kind == 9) {
                        w->frame->scale[2] = 1.2f;
                        w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                        w->frame->rot[0] = gCockpitHeadPitch + 0.3;
                    }
                }
                if (w->kind == 4 || w->kind == 10) {
                    if (gBattleFlags & 0x4000000) {
                        bossSetAnimAt(w->frame, impStormFist2Pilot2Anim, 0xEA, 0.3f);
                    } else {
                        bossSetAnimAt(w->frame, impStormFist2Pilot1Anim, 0xEA, 0.3f);
                    }
                    if (gCockpitAimX >= 0.0f) {
                        bossHeadingOffset(gCockpitRotY * 2, -275.0f, 80.0f, &w->frame->pos[0], &w->frame->pos[2]);
                    } else {
                        bossHeadingOffset(gCockpitRotY, -275.0f, 80.0f, &w->frame->pos[0], &w->frame->pos[2]);
                    }
                    w->frame->pos[1] = -140.0f;
                    w->frame->rot[0] = gCockpitHeadPitch - 0.6;
                    w->frame->rot[1] = gCockpitHeadYaw;
                    if (w->kind == 10) {
                        w->frame->scale[2] = 1.2f;
                        w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                        w->frame->rot[0] = gCockpitHeadPitch - 0.4;
                    }
                }
                if (w->kind == 5 || w->kind == 11) {
                    if (gBattleFlags & 0x4000000) {
                        bossSetAnimAt(w->frame, impStormFist3Pilot2Anim, 0xEA, 0.3f);
                    } else {
                        bossSetAnimAt(w->frame, impStormFist3Pilot1Anim, 0xEA, 0.3f);
                    }
                    if (gCockpitAimX >= 0.0f) {
                        bossHeadingOffset(gCockpitRotY, 275.0f, 80.0f, &w->frame->pos[0], &w->frame->pos[2]);
                    } else {
                        bossHeadingOffset(gCockpitRotY * 2, 275.0f, 80.0f, &w->frame->pos[0], &w->frame->pos[2]);
                    }
                    w->frame->pos[1] = -140.0f;
                    w->frame->rot[0] = gCockpitHeadPitch - 0.6;
                    w->frame->rot[1] = gCockpitHeadYaw;
                    if (w->kind == 11) {
                        w->frame->scale[2] = 1.2f;
                        w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                        w->frame->rot[0] = gCockpitHeadPitch - 0.4;
                    }
                }
            } else if (w->kind == 0) {
                if (gBattleFlags & 0x4000000) {
                    bossSetAnimAt(w->frame, impStraightPilot2Anim, 0xEA, 0.3f);
                } else {
                    bossSetAnimAt(w->frame, impJabPilot1Anim, 0xEA, 0.3f);
                }
                if (gCockpitAimX >= 0.0f) {
                    bossHeadingOffset(gCockpitRotY * 2, -50.0f, 0.0f, &w->frame->pos[0], &w->frame->pos[2]);
                } else {
                    bossHeadingOffset(gCockpitRotY, -50.0f, 0.0f, &w->frame->pos[0], &w->frame->pos[2]);
                }
                w->frame->pos[1] = -140.0f;
                w->frame->rot[0] = gCockpitHeadPitch - 0.1;
                w->frame->rot[1] = gCockpitHeadYaw;
            } else if (w->kind == 1) {
                if (gBattleFlags & 0x4000000) {
                    bossSetAnimAt(w->frame, impStraightPilot2Anim, 0xEA, 0.3f);
                } else {
                    bossSetAnimAt(w->frame, impJabPilot1Anim, 0xEA, 0.3f);
                }
                if (gCockpitAimX >= 0.0f) {
                    bossHeadingOffset(gCockpitRotY, 50.0f, 0.0f, &w->frame->pos[0], &w->frame->pos[2]);
                } else {
                    bossHeadingOffset(gCockpitRotY * 2, 50.0f, 0.0f, &w->frame->pos[0], &w->frame->pos[2]);
                }
                w->frame->pos[1] = -140.0f;
                w->frame->rot[0] = gCockpitHeadPitch - 0.1;
                w->frame->rot[1] = gCockpitHeadYaw;
            } else if (w->kind == 2) {
                if (gBattleFlags & 0x4000000) {
                    bossSetAnimAt(w->frame, impStraightPilot2Anim, 0xEA, 0.3f);
                } else {
                    bossSetAnimAt(w->frame, impJabPilot1Anim, 0xEA, 0.3f);
                }
                if (gCockpitAimX >= 0.0f) {
                    bossHeadingOffset(gCockpitRotY * 2, -150.0f, -50.0f, &w->frame->pos[0], &w->frame->pos[2]);
                } else {
                    bossHeadingOffset(gCockpitRotY, -150.0f, -50.0f, &w->frame->pos[0], &w->frame->pos[2]);
                }
                w->frame->pos[1] = -120.0f;
                w->frame->rot[0] = gCockpitHeadPitch - 0.15;
                w->frame->rot[1] = gCockpitHeadYaw + 0.2;
            } else if (w->kind == 3) {
                if (gBattleFlags & 0x4000000) {
                    bossSetAnimAt(w->frame, impStormFist3Pilot2Anim, 0xEA, 0.3f);
                } else {
                    bossSetAnimAt(w->frame, impStormFist3Pilot1Anim, 0xEA, 0.3f);
                }
                if (gCockpitAimX >= 0.0f) {
                    bossHeadingOffset(gCockpitRotY, 150.0f, -50.0f, &w->frame->pos[0], &w->frame->pos[2]);
                } else {
                    bossHeadingOffset(gCockpitRotY * 2, 150.0f, -50.0f, &w->frame->pos[0], &w->frame->pos[2]);
                }
                w->frame->pos[1] = -120.0f;
                w->frame->rot[0] = gCockpitHeadPitch - 0.15;
                w->frame->rot[1] = gCockpitHeadYaw - 0.2;
            } else if (w->kind == 4) {
                if (gBattleFlags & 0x4000000) {
                    bossSetAnimAt(w->frame, impStormFist4Pilot2Anim, 0xEA, 0.3f);
                } else {
                    bossSetAnimAt(w->frame, impStormFist4Pilot1Anim, 0xEA, 0.3f);
                }
                if (gCockpitAimX >= 0.0f) {
                    bossHeadingOffset(gCockpitRotY * 2, -150.0f, -50.0f, &w->frame->pos[0], &w->frame->pos[2]);
                } else {
                    bossHeadingOffset(gCockpitRotY, -150.0f, -50.0f, &w->frame->pos[0], &w->frame->pos[2]);
                }
                w->frame->pos[1] = 120.0f;
                w->frame->rot[0] = gCockpitHeadPitch - 0.15;
                w->frame->rot[1] = gCockpitHeadYaw;
            } else if (w->kind == 5) {
                if (gBattleFlags & 0x4000000) {
                    bossSetAnimAt(w->frame, impStormFist5Pilot2Anim, 0xEA, 0.3f);
                } else {
                    bossSetAnimAt(w->frame, impStormFist5Pilot1Anim, 0xEA, 0.3f);
                }
                if (gCockpitAimX >= 0.0f) {
                    bossHeadingOffset(gCockpitRotY, 150.0f, -50.0f, &w->frame->pos[0], &w->frame->pos[2]);
                } else {
                    bossHeadingOffset(gCockpitRotY * 2, 150.0f, -50.0f, &w->frame->pos[0], &w->frame->pos[2]);
                }
                w->frame->pos[1] = 120.0f;
                w->frame->rot[0] = gCockpitHeadPitch - 0.15;
                w->frame->rot[1] = gCockpitHeadYaw;
            }
            if (w->kind == 0 || w->kind == 1 || w->kind == 2 || w->kind == 3 || w->kind == 4 || w->kind == 5) {
                if (gBattleSndToggles & 1) {
                    sndPlay(0x50C);
                    gBattleSndToggles &= ~1;
                } else {
                    sndPlay(0x50D);
                    gBattleSndToggles |= 1;
                }
            }
            (*bossSetFrameFlags2)(w->frame, 0x20);
            w->alpha = 255;
            w->stage = 0;
            bossRumble(0, 0x88888888, 5);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (gBattleFlags & 2) {
                return -1;
            }
            switch (w->stage) {
                case 0:
                    if (bossStepAnim(w->frame, 0.3f, 2) != 0) {
                        w->time = 5;
                        w->stage++;
                    }
                    break;
                case 1:
                    if (--w->time == 0) {
                        frmMove(&w->frame->f, gFrmLayer8);
                        w->alpha = 0x80;
                        w->stage++;
                    }
                    break;
                case 2:
                    w->alpha -= 3;
                    if (bossStepAnim(w->frame, 0.3f, 3) != 0) {
                        return -1;
                    }
                    break;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Astorm": the hundred-punch's attack box, 500 ahead of the Impact, and its blur. While the
 * punches fly (flag 0x4000 of gBattleFlags2 and gImpactMoveTime past 90) the box grows to 900 and
 * hits every eighth frame, and the blur sways toward the boss; while flag 0x40000000 of
 * gBattleFlags is on, a 300 box hits every eighth frame. It ends if the Impact is down (flag 2
 * of gBattleFlags). */
s32 Astorm(Obj *obj, s16 msg, void *arg) {
    AstormWork *w = (AstormWork *) ((u32) obj + sizeof(Obj));

    switch (msg) {
        case 2:
            obj->name = "Astorm";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->box = acollAtkAddYZ(pl_atk_root_frame, &w->frame->f, 300, 0, 0, 0);
            if (w->box == NULL) {
                return -1;
            }
            w->fx = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->fx == NULL) {
                return -1;
            }
            if (gBattleFlags & 2) {
                PRINTF("インパクト死亡してるので百烈パンチあたりを消します。\n");
                return -1;
            }
            w->box->kind = 5;
            bossHeadingOffset(gCockpitHeadYaw, 0.0f, 500.0f, &w->frame->pos[0], &w->frame->pos[2]);
            w->box->flags &= ~2;
            if (gBattleFlags & 0x4000000) {
                bossSetAnimAt(w->fx, impStormBlurPilot2Anim, 0xEF, 0.0f);
            } else {
                bossSetAnimAt(w->fx, impStormBlurPilot1Anim, 0xEF, 0.0f);
            }
            w->fx->f.flags &= ~2;
            w->fx->scale[2] = 0.7f;
            w->fx->scale[0] = w->fx->scale[1] = w->fx->scale[2];
            w->fx->pos[1] = -350.0f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->box->frame);
            frmDestroy(&w->fx->f);
            break;
        case 0:
            if (gBattleFlags & 2) {
                PRINTF("インパクト死亡してるので百烈パンチあたりを消します。\n");
                return -1;
            }
            if (gBattleFlags & 0x4000000) {
                bossSetAnimAt(w->fx, impStormBlurPilot2Anim, 0xEF, 0.0f);
            } else {
                bossSetAnimAt(w->fx, impStormBlurPilot1Anim, 0xEF, 0.0f);
            }
            bossHeadingOffset(gCockpitHeadYaw, 0.0f, 500.0f, &w->frame->pos[0], &w->frame->pos[2]);
            w->frame->pos[1] = gCockpitAimY * 10.0f;
            if (gBattleFlags2 & 0x4000) {
                if (gImpactMoveTime > 90) {
                    w->box->radius = 900;
                    w->fx->f.flags |= 2;
                    if (gGameFrameCount % 10 >= 5) {
                        w->sway += 0.06981317201552031;
                    } else {
                        w->sway -= 0.06981317201552031;
                    }
                    w->fx->rot[1] = mathAtan2(gBossPosX, gBossPosZ) + w->sway;
                    if (!(gGameFrameCount & 7)) {
                        w->box->flags |= 2;
                    } else {
                        w->box->flags &= ~2;
                    }
                }
            } else if (gBattleFlags & 0x40000000) {
                w->box->radius = 300;
                if (!(gGameFrameCount & 7)) {
                    w->box->flags |= 2;
                } else {
                    w->box->flags &= ~2;
                }
            } else {
                w->fx->f.flags &= ~2;
                w->box->flags &= ~2;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Bbg": the battle's backdrop (file 0xEC), 32 by 16, scrolling with gCockpitAimX and
 * gCockpitAimY; it fades out by 16 a frame while flag 0x100 of gBattleFlags2 is on, and back in. */
s32 Bbg(Obj *obj, s16 msg, void *arg) {
    BbgWork *w = (BbgWork *) ((u32) obj + sizeof(Obj));

    switch (msg) {
        case 2:
            obj->name = "Bbg";
            w->frame = (BGFrame *) frmCreate(gFrmLayers[0], gFrmKindBg);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->w = 0x20;
            w->frame->h = 0x10;
            w->frame->tileCount = 0x3D;
            w->frame->img1 = floadSegToRam((u32) impBackdropMap, 0xEC, 0);
            w->frame->img2 = floadSegToRam((u32) impBackdropTiles, 0xEC, 0);
            w->frame->palA = (u16 *) floadSegToRam((u32) impBackdropPal, 0xEC, 0);
            w->frame->blend = 0;
            w->level = 0x100;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->frame->x = gCockpitAimX * 4.0f;
            w->frame->y = -gCockpitAimY * 1.5;
            if (gBattleFlags2 & 0x100) {
                w->level -= 16;
                if (w->level <= 0) {
                    w->level = 0;
                }
            } else {
                w->level += 16;
                if (w->level >= 0x100) {
                    w->level = 0x100;
                }
            }
            w->frame->alpha = w->level;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "shoulder": the Impact's shoulder, turned with the boss's heading (gCockpitRotY); while it
 * punches (flag 0x80 or 0x800000 of gBattleFlags) it swings by gImpactMoveTime, one way or the
 * other by flag 0x80000. */
s32 shoulder(Obj *obj, s16 msg, void *arg) {
    ShoulderWork *w = (ShoulderWork *) ((u32) obj + sizeof(Obj));
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "shoulder";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, impShoulderDl, 0xEA);
            (*bossSetFrameFlags)(w->frame, 0x20);
            w->frame->scale[2] = 1.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 0.0f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (gBattleFlags & 0x1000000) {
                w->frame->f.flags &= ~2;
            } else {
                w->frame->f.flags |= 2;
            }
            w->frame->rot[1] = gCockpitRotY;
            if ((gBattleFlags & 0x80) || (gBattleFlags & 0x800000)) {
                if (gImpactMoveTime >= 32) {
                    if (gBattleFlags & 0x80000) {
                        w->frame->rot[1] -= (48 - gImpactMoveTime) * 0.0174532930038800785;
                    } else {
                        w->frame->rot[1] += (48 - gImpactMoveTime) * 0.0174532930038800785;
                    }
                } else if (gImpactMoveTime >= 16 && gImpactMoveTime < 32) {
                    if (gBattleFlags & 0x80000) {
                        w->frame->rot[1] -= 0.279252688062081256;
                    } else {
                        w->frame->rot[1] += 0.279252688062081256;
                    }
                } else if (gBattleFlags & 0x80000) {
                    w->frame->rot[1] -= gImpactMoveTime * 0.0174532930038800785;
                } else {
                    w->frame->rot[1] += gImpactMoveTime * 0.0174532930038800785;
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Baton": a baton thrown 3000 ahead at 40 and up by 12, spinning, under gravity, trailing
 * stars every fifth frame. A hit (unless of kind 0x74) ends it in sparks, with debris if its
 * damage box was struck, and a sound unless the hit was of kind 0xCB; so does the ground
 * (sparks, debris, the sound), or 6000 away. */
s32 Baton(Obj *obj, s16 msg, void *arg) {
    BatonWork *w = (BatonWork *) ((u32) obj + sizeof(Obj));
    s32 pad;
    f32 tx;
    f32 tz;
    Vec3f at;
    f32 t;

    switch (msg) {
        case 2:
            obj->name = "Baton";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->model = (MPolFrame *) frmCreate(gFrmModelLayer, gFrmKindMpol);
            if (w->model == NULL) {
                return -1;
            }
            MPOL_SET_DL(w->model, impBatonDl, 0xEA);
            (*bossSetMpolFlags)(w->model, 0x20);
            (*bossHeadingOffset)(gCockpitHeadYaw, 100.0f, -200.0f, &w->frame->pos[0], &w->frame->pos[2]);
            w->frame->pos[1] = -80.0f;
            w->box = acollAtkAddYZ(pl_atk_root_frame, &w->frame->f, 50, 0, 0, 0);
            if (w->box == NULL) {
                return -1;
            }
            w->box->kind = 0xC;
            w->dmg = (*acollDmgAddYZ)(pl_dmg_root_frame, &w->frame->f, 20, 20, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 0xD;
            w->frame->scale[2] = 6.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            bossHeadingOffset(gCockpitHeadYaw, 0.0f, 3000.0f, &tx, &tz);
            BmoveX(obj, tx, gCockpitAimY * 60.0f, tz, 40.0f);
            w->vel[1] += 12.0f;
            (*sndPlay)(0x2C8);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->model->f);
            frmDestroy(&w->box->frame);
            frmDestroy(&w->dmg->frame);
            break;
        case 0:
            w->frame->rot[1] = mathAtan2(w->vel[0], w->vel[2]);
            w->frame->rot[0] -= 0.26179939505820116;
            w->frame->rot[2] = -0.62831855f;
            bossGravity(obj, 0.4f);
            if (gGameFrameCount % 5 == 0) {
                beStarCreate(w->frame->pos[0] + fadeRandRange(-100, 100),
                                       w->frame->pos[1] + fadeRandRange(-100, 100),
                                       w->frame->pos[2] + fadeRandRange(-100, 100));
                beStarCreate(w->frame->pos[0] + fadeRandRange(-100, 100),
                                       w->frame->pos[1] + fadeRandRange(-100, 100),
                                       w->frame->pos[2] + fadeRandRange(-100, 100));
            }
            if (acollAtkWasHit(w->box) == 1 && w->box->hitKind != 0x74) {
                (*itexGlowBlue)(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
                if (w->box->hitKind != 0xCB) {
                    SoundLocation2(0x228, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                }
                return -1;
            }
            if (acollDmgWasHit(w->dmg) != 0 && w->box->hitKind != 0x74) {
                (*itexGlowBlue)(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
                (*beBpieceCreate)(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 10);
                (*beBpieceCreate)(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 10);
                if (w->box->hitKind != 0xCB) {
                    SoundLocation2(0x228, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                           300.0f, 15000.0f, gBattleCamView->fovy);
                }
                return -1;
            }
            if (gBattleBossId != 5 &&
                vramRayGround((Vec3f *) w->frame->pos, (Vec3f *) w->vel, &at, &t) != 0) {
                itexSparkleYellow(w->frame->pos[0], w->frame->pos[1] + 50.0f, w->frame->pos[2], 0.6f);
                (*beBpieceCreate)(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 10);
                (*beBpieceCreate)(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 5.0f, 10);
                SoundLocation2(0x228, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                                       15000.0f, gBattleCamView->fovy);
                return -1;
            }
            if ((w->dist = bossDistSq(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                                  gImpactPosX, gImpactPosY, gImpactPosZ)) >
                36000000.0f) {
                return -1;
            }
            bossApplyVel(obj);
            mathMtxIdent(w->model->mtx.m);
            mathMtxScaleRot(w->model->mtx.m, w->frame->scale[0]);
            mathMtxPreRotY(w->model->mtx.m, w->frame->rot[1]);
            mathMtxPreRotZ(w->model->mtx.m, w->frame->rot[2]);
            mathMtxPreRotX(w->model->mtx.m, w->frame->rot[0]);
            mathMtxPostTrans(w->model->mtx.m, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2]);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "MEGA": the Impact's cannon, the んが砲. It charges for 120 frames (sparks gathering, a white
 * flash), then fires a shot 3000 ahead with its beam behind it (sound 0x173, or 0x174 by flag
 * 0x4000000 of gBattleFlags), spinning, the beam stretching. A hit on the boss (kind 0xCC) puts
 * it out if the boss is angry; otherwise the cannon's camera ("Bappcam") watches the shot burst
 * at the boss for 240 frames (sound 0x175). The ground, the time or the beam's length stops it
 * for 60 frames. Then it fades out and shrinks away. Its position is kept in gMegaShotPosX.
 * Matched 2026-10-02 (476 to 0). The frame pointer that feeds a call's arguments is read into a
 * local f right before the call ("f = w->frame; call(f->pos[0], ...)"): a named local gets v0,
 * an inline w->frame gets v1. Only before calls: stores through it and the beam/glow copies stay
 * inline (the game reloads w->frame after each store). Then (214 to 0): a block boundary after
 * the hit's 0xEE call (IDO_NOTES 228), which is the do-while(0) of MPOL_SET_DL, the mpol
 * display-list macro used at all six of the file's sites; the stage-1 hit test's at and t
 * declared in message 0's block, above stage 0's tx/tz (the game's slot order); stage 1's dist,
 * the discarded distances of the hit path, in the word under tx/tz; and stage 3's two flag words
 * cleared in the game's order (gBattleFlags's 0x400 before gBattleFlags2's 0x20000).
 * pad[4] (FAKE) is 16 bytes of locals the code never touches. Without it only the frame changes
 * (0x88 to 0x78): every slot keeps its offset, since IDO lays locals out from the bottom in
 * reverse declaration order. So the original declared four more scalar words (or one 16-byte
 * object) anywhere above message 0's at. In a function with memory locals IDO gives every
 * declared local its word, used or not, register-only too (measured), so they were unused,
 * dead-stored or register-only locals. One fits with the same
 * code: a len for the sqrtf of stages 0 and 4 ("len = sqrtf(...); rot[0] = mathAtan2(vel[1],
 * len)"); pitch or yaw locals would have to be stored at once, not natural. Valcan's frame has
 * the same shape (w, a word, angle/c/s and a word, then at/t). Not a loop index: IDO does not
 * unroll a three-call loop (scratch loops.c), and the nine M6effect calls are straight code. */
s32 MEGA(Obj *obj, s16 msg, void *arg) {
    MegaWork *w = (MegaWork *) ((u32) obj + sizeof(Obj));
    PolFrame *f;
    s32 pad[4]; /* FAKE: frame */

    switch (msg) {
        case 2:
            obj->name = "MEGA";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->model = (MPolFrame *) frmCreate(gFrmLayer8, gFrmKindMpol);
            if (w->model == NULL) {
                return -1;
            }
            w->beam = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->beam == NULL) {
                return -1;
            }
            w->beamModel = (MPolFrame *) frmCreateNext(gFrmLayer8, gFrmKindMpol);
            if (w->beamModel == NULL) {
                return -1;
            }
            w->glow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->beam == NULL) { /* sic: the beam again, not the glow */
                return -1;
            }
            w->glowModel = (MPolFrame *) frmCreate(gFrmLayer8, gFrmKindMpol);
            if (w->glow == NULL) { /* sic */
                return -1;
            }
            w->box = acollAtkAddYZ(pl_atk_root_frame, &w->frame->f, 20, 0, 0, 0);
            if (w->box == NULL) {
                return -1;
            }
            w->box->kind = 0xB;
            w->box->flags &= ~2;
            f = w->frame;
            bossHeadingOffset(gCockpitHeadYaw, 0.0f, 150.0f, &f->pos[0], &f->pos[2]);
            w->time = 120;
            w->alpha = 0x60;
            gMegaShotPosX = 0.0f;
            gMegaShotPosY = 0.0f;
            gMegaShotPosZ = 0.0f;
            gBattleFlags2 |= 1;
            gBattleFlags2 |= 0x100;
            w->struck = 0;
            bossRumble(0, 0x13579ADF, 90);
            (*sndPlay)(0x172);
            break;
        case 0: {
            Vec3f at;
            f32 t;

            gMegaShotPosX = w->frame->pos[0];
            gMegaShotPosY = w->frame->pos[1];
            gMegaShotPosZ = w->frame->pos[2];
            gImpactMoveTime = 1;
            if ((w->stage == 1 || w->stage == 2) && gGameFrameCount % 10 == 0) {
                f = w->frame;
                beMeffectCreate(f->pos[0], f->pos[1] + 10.0f, f->pos[2]);
            }
            if ((gBattleFlags & 8) || (gBattleFlags & 0x400)) {
                DPRINTF("んが砲のアタリを消します。\n");
                w->box->flags &= ~2;
            }
            switch (w->stage) {
                case 0:
                    DPRINTF("Megastep[0]\n");
                    if (w->time > 90 && w->time % 5 == 0) {
                        f = w->frame;
                        beM6effectCreate(f->pos[0], 0.0f, f->pos[2], 1.0f, 100.0f);
                    } else if (w->time > 70 && w->time % 5 == 0) {
                        f = w->frame;
                        beM6effectCreate(f->pos[0], 0.0f, f->pos[2], 1.0f, 100.0f);
                        f = w->frame;
                        beM6effectCreate(f->pos[0], 0.0f, f->pos[2], 1.0f, 100.0f);
                        f = w->frame;
                        beM6effectCreate(f->pos[0], 0.0f, f->pos[2], 1.0f, 100.0f);
                        f = w->frame;
                        beM6effectCreate(f->pos[0], 0.0f, f->pos[2], 1.0f, 200.0f);
                        f = w->frame;
                        beM6effectCreate(f->pos[0], 0.0f, f->pos[2], 1.0f, 200.0f);
                        f = w->frame;
                        beM6effectCreate(f->pos[0], 0.0f, f->pos[2], 1.0f, 200.0f);
                        f = w->frame;
                        beM6effectCreate(f->pos[0], 0.0f, f->pos[2], 0.5f, 100.0f);
                        f = w->frame;
                        beM6effectCreate(f->pos[0], 0.0f, f->pos[2], 0.5f, 100.0f);
                        f = w->frame;
                        beM6effectCreate(f->pos[0], 0.0f, f->pos[2], 0.5f, 100.0f);
                    }
                    if (w->time == 30) {
                        f = w->frame;
                        beM5effectCreate(f->pos[0], 0.0f, f->pos[2]);
                    }
                    if (w->time-- == 0) {
                        f32 tx;
                        f32 tz;

                        w->box->flags |= 2;
                        beFlashFCreate(0xFF, 0xFF, 0xFF, 60);
                        if (gBattleFlags & 0x4000000) {
                            MPOL_SET_DL(w->model, impMegaShotDl, 0xEE);
                        } else {
                            MPOL_SET_DL(w->model, impMegaShotDl, 0xEE);
                        }
                        (*bossSetMpolFlags)(w->model, 0x20);
                        f = w->frame;
                        bossHeadingOffset(gCockpitHeadYaw, 0.0f, 50.0f, &f->pos[0], &f->pos[2]);
                        w->frame->pos[1] = -150.0f;
                        w->frame->scale[2] = 24.0f;
                        w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                        bossHeadingOffset(gCockpitHeadYaw, 0.0f, 3000.0f, &tx, &tz);
                        BmoveX(obj, tx, gCockpitAimY * 60.0f, tz, 60.0f);
                        w->frame->rot[0] = mathAtan2(
                            w->vel[1], sqrtf(w->vel[2] * w->vel[2] + w->vel[0] * w->vel[0]));
                        w->frame->rot[1] = mathAtan2(w->vel[0], w->vel[2]);
                        MPOL_SET_DL(w->beamModel, impMegaBeamDl, 0xEE);
                        (*bossSetMpolFlags)(w->beamModel, 0x20);
                        w->beam->scale[2] = 6.0f;
                        w->beam->scale[0] = w->beam->scale[1] = w->beam->scale[2];
                        w->beam->pos[0] = w->frame->pos[0];
                        w->beam->pos[1] = w->frame->pos[1];
                        w->beam->pos[2] = w->frame->pos[2];
                        w->beam->rot[0] = w->frame->rot[0];
                        w->beam->rot[1] = w->frame->rot[1];
                        w->beam->rot[2] = w->frame->rot[2];
                        MPOL_SET_DL(w->glowModel, impMegaBeamDl, 0xEE);
                        (*bossSetMpolFlags)(w->glowModel, 0x20);
                        w->glow->scale[2] = 1.0f;
                        w->glow->scale[0] = w->glow->scale[1] = w->glow->scale[2];
                        w->glow->pos[0] = w->frame->pos[0];
                        w->glow->pos[1] = w->frame->pos[1];
                        w->glow->pos[2] = w->frame->pos[2];
                        w->glow->rot[0] = w->frame->rot[0];
                        w->glow->rot[1] = w->frame->rot[1];
                        bossRumble(0, 0x88888888, 400);
                        if (gBattleFlags & 0x4000000) {
                            (*sndPlay)(0x174);
                        } else {
                            (*sndPlay)(0x173);
                        }
                        gCockpitShake = 5;
                        w->time = 90;
                        w->stage = 1;
                    }
                    break;
                case 1: {
                    f32 dist;

                    DPRINTF("Megastep[1]\n");
                    w->frame->rot[2] += 0.17453293003880077;
                    w->beam->rot[2] += 0.17453293003880077;
                    w->glow->rot[2] += 0.17453293003880077;
                    bossApplyVel(obj);
                    if (gGameFrameCount % 5 == 0) {
                        itexGlowBlue(w->frame->pos[0] + (*fadeRandRange)(-100, 100),
                                               w->frame->pos[1] + (*fadeRandRange)(-100, 100),
                                               w->frame->pos[2] + (*fadeRandRange)(-100, 100), 0.2f);
                        itexGlowBlue(w->frame->pos[0] + (*fadeRandRange)(-100, 100),
                                               w->frame->pos[1] + (*fadeRandRange)(-100, 100),
                                               w->frame->pos[2] + (*fadeRandRange)(-100, 100), 0.2f);
                    }
                    w->frame->scale[0] = 24.0f;
                    w->frame->scale[1] = 24.0f;
                    w->frame->scale[2] = 24.0f;
                    w->beam->scale[0] = 6.0f;
                    w->beam->scale[1] = 6.0f;
                    w->beam->scale[2] -= 1.1;
                    w->glow->scale[0] = 1.0f;
                    w->glow->scale[1] = 1.0f;
                    w->glow->scale[2] -= 1.1;
                    if ((*acollAtkWasHit)(w->box) == 1 && w->box->hitKind == 0xCC) {
                        if (gBossFlags & 0x4000) {
                            DPRINTF("ノー！消失します。\n");
                            w->box->flags &= ~2;
                            w->stage = 3;
                        } else {
                            tskCreate(obj, bappcamCannon, 0, NULL);
                            MPOL_SET_DL(w->model, impMegaShotHitDl, 0xEE);
                            f = w->frame;
                            beM3effectCreate(f->pos[0], f->pos[1], f->pos[2]);
                            w->frame->pos[0] = 0.0f;
                            w->frame->pos[1] = -150.0f;
                            w->frame->pos[2] = 0.0f;
                            f = w->frame;
                            bossHeadingOffset(gCockpitHeadYaw, 0.0f, 150.0f, &f->pos[0], &f->pos[2]);
                            BmoveX(obj, gBossPosX, gMegaBurstY, gBossPosZ,
                                                   60.0f);
                            if (gBattleBossId == 1) {
                                f = w->frame;
                                bossHeadingOffset(f->rot[1], 0.0f, -300.0f, &f->pos[0], &f->pos[2]);
                                dist = (*bossDistSq)(gBossPosX, gBossPosY, gBossPosZ, 0.0f, 0.0f, 0.0f);
                            } else if (gBattleBossId == 2) {
                                f = w->frame;
                                bossHeadingOffset(f->rot[1], 0.0f, -350.0f, &f->pos[0], &f->pos[2]);
                                dist = (*bossDistSq)(gBossPosX, gBossPosY, gBossPosZ, 0.0f, 0.0f, 0.0f);
                            } else if (gBattleBossId == 3) {
                                f = w->frame;
                                bossHeadingOffset(f->rot[1], 0.0f, -400.0f, &f->pos[0], &f->pos[2]);
                                dist = (*bossDistSq)(gBossPosX, gBossPosY, gBossPosZ, 0.0f, 0.0f, 0.0f);
                            } else {
                                f = w->frame;
                                bossHeadingOffset(f->rot[1], 0.0f, -300.0f, &f->pos[0], &f->pos[2]);
                                dist = (*bossDistSq)(gBossPosX, gBossPosY, gBossPosZ, 0.0f, 0.0f, 0.0f);
                            }
                            w->frame->pos[0] += gBossPosX;
                            w->frame->pos[2] += gBossPosZ;
                            w->frame->pos[1] = gMegaBurstY;
                            if (gBattleBossId == 1 && gBossPosY < -600.0f) {
                                w->frame->pos[1] += 500.0f;
                            }
                            f = w->frame;
                            beLballCreate(f->pos[0], f->pos[1], f->pos[2], 0);
                            w->box->flags &= ~2;
                            w->time = 240;
                            w->stage = 4;
                            (*sndPlay)(0x175);
                            contPakRumbleRequestStop(gCockpitPort);
                            bossRumble(2, -1, 400);
                        }
                    }
                    if (gBattleBossId != 5 &&
                        vramRayGround((Vec3f *) w->frame->pos, (Vec3f *) w->vel, &at, &t) != 0) {
                        f = w->frame;
                        itexGlowBlue(f->pos[0], f->pos[1], f->pos[2], 10.0f);
                        w->time = 60;
                        w->stage = 2;
                        contPakRumbleRequestStop(gCockpitPort);
                        bossRumble(0, 0x88888888, 60);
                    } else {
                        if (w->time-- == 0) {
                            w->time = 60;
                            w->stage = 2;
                            contPakRumbleRequestStop(gCockpitPort);
                            bossRumble(0, 0x88888888, 60);
                        }
                        if (w->beam->scale[2] > 100.0) {
                            w->time = 60;
                            w->stage = 2;
                            contPakRumbleRequestStop(gCockpitPort);
                            bossRumble(0, 0x88888888, 60);
                        }
                        if (w->glow->scale[2] < -97.0) {
                            w->time = 60;
                            w->stage = 2;
                            contPakRumbleRequestStop(gCockpitPort);
                            bossRumble(0, 0x88888888, 60);
                        }
                    }
                    break;
                }
                case 2:
                    DPRINTF("Megastep[2]\n");
                    w->box->flags &= ~2;
                    gBattleFlags |= 8;
                    gBattleFlags |= 0x400;
                    w->frame->scale[0] = 24.0f;
                    w->frame->scale[1] = 24.0f;
                    w->frame->scale[2] = 24.0f;
                    w->frame->rot[2] += 0.17453293003880077;
                    w->beam->scale[0] = 6.0f;
                    w->beam->scale[1] = 6.0f;
                    w->beam->rot[2] += 0.17453293003880077;
                    w->glow->scale[0] = 1.0f;
                    w->glow->scale[1] = 1.0f;
                    w->glow->rot[2] += 0.17453293003880077;
                    if (w->time-- == 0) {
                        w->stage = 3;
                        bossRumble(0, 0xF000F000, 60);
                    }
                    break;
                case 3:
                    DPRINTF("MegaStep[3]\n");
                    gCockpitShake = 0;
                    gBattleFlags2 &= ~1;
                    gBattleFlags &= ~0x10;
                    gBattleFlags &= ~0x1000;
                    if (w->struck == 0) {
                        gBattleFlags2 &= ~0x100;
                    }
                    w->frame->rot[2] += 0.17453293003880077;
                    w->beam->rot[2] += 0.17453293003880077;
                    w->glow->rot[2] += 0.17453293003880077;
                    w->alpha -= 3;
                    if (w->alpha < 0) {
                        w->alpha = 0;
                    }
                    w->frame->scale[0] -= 0.2;
                    if (w->frame->scale[0] <= 0.0f) {
                        return -1;
                    }
                    w->beam->scale[0] -= 0.2;
                    w->beam->scale[1] -= 0.2;
                    if (w->beam->scale[0] < 0.0f) {
                        w->beam->scale[0] = 0.0f;
                        w->beam->scale[1] = 0.0f;
                        w->beam->scale[2] = 0.0f;
                    }
                    w->glow->scale[0] -= 0.2;
                    w->glow->scale[1] -= 0.2;
                    if (w->glow->scale[0] < 0.0f) {
                        w->glow->scale[0] = 0.0f;
                        w->glow->scale[1] = 0.0f;
                        w->glow->scale[2] = 0.0f;
                    }
                    break;
                case 4:
                    DPRINTF("MegaStep[4]\n");
                    w->struck = 1;
                    w->frame->rot[0] =
                        mathAtan2(w->vel[1], sqrtf(w->vel[2] * w->vel[2] + w->vel[0] * w->vel[0]));
                    w->frame->rot[1] = mathAtan2(w->vel[0], w->vel[2]);
                    w->frame->rot[2] -= 0.17453293003880077;
                    w->beam->rot[2] += 0.17453293003880077;
                    w->beam->pos[0] = w->frame->pos[0];
                    w->beam->pos[1] = w->frame->pos[1];
                    w->beam->pos[2] = w->frame->pos[2];
                    w->beam->rot[0] = w->frame->rot[0];
                    w->beam->rot[1] = w->frame->rot[1];
                    w->beam->rot[2] = w->frame->rot[2];
                    w->glow->rot[2] -= 0.17453293003880077;
                    w->glow->pos[0] = w->frame->pos[0];
                    w->glow->pos[1] = w->frame->pos[1];
                    w->glow->pos[2] = w->frame->pos[2];
                    w->glow->rot[0] = w->frame->rot[0];
                    w->glow->rot[1] = w->frame->rot[1];
                    w->glow->rot[2] = w->frame->rot[2];
                    w->time--;
                    w->beam->scale[2] = 50.0f;
                    w->glow->scale[2] = 50.0f;
                    if (w->time > 40 && w->time % 5 == 0) {
                        beM7effectCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 3.0f, 500.0f);
                        beM7effectCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 3.0f, 500.0f);
                        beM7effectCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 3.0f, 500.0f);
                    }
                    if (w->time > 20 && !(w->time & 7)) {
                        beM3effectTintCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                               w->frame->rot[1]);
                    }
                    w->frame->scale[0] = 24.0f;
                    w->frame->scale[1] = 24.0f;
                    w->frame->scale[2] = 6.0f;
                    w->beam->scale[0] = 6.0f;
                    w->beam->scale[1] = 6.0f;
                    w->glow->scale[0] = 1.0f;
                    w->glow->scale[1] = 1.0f;
                    if (w->time == 0) {
                        w->stage = 3;
                        contPakRumbleRequestStop(gCockpitPort);
                        bossRumble(0, 0xF000F000, 60);
                    }
                    break;
            }
            break;
        }
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->model->f);
            frmDestroy(&w->box->frame);
            frmDestroy(&w->beam->f);
            frmDestroy(&w->beamModel->f);
            frmDestroy(&w->glow->f);
            frmDestroy(&w->glowModel->f);
            gMegaShotPosX = 0.0f;
            gMegaShotPosY = 0.0f;
            gMegaShotPosZ = 0.0f;
            gBattleFlags2 &= ~0x100;
            gBattleFlags &= ~8;
            gBattleFlags &= ~0x400;
            gBattleFlags2 &= ~0x20000;
            gBattleFlags2 &= ~0x40000;
            break;
        case 3:
            mathMtxIdent(w->model->mtx.m);
            mathMtxScaleRot(w->model->mtx.m, w->frame->scale[0]);
            mathMtxPreRotY(w->model->mtx.m, w->frame->rot[1]);
            mathMtxPreRotX(w->model->mtx.m, w->frame->rot[0]);
            mathMtxPreRotZ(w->model->mtx.m, w->frame->rot[2]);
            mathMtxPostTrans(w->model->mtx.m, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2]);
            mathMtxIdent(w->beamModel->mtx.m);
            mathMtxScaleRotX(w->beamModel->mtx.m, w->beam->scale[0]);
            mathMtxScaleRotY(w->beamModel->mtx.m, w->beam->scale[1]);
            mathMtxScaleRotZ(w->beamModel->mtx.m, w->beam->scale[2]);
            mathMtxPostRotZ(w->beamModel->mtx.m, w->beam->rot[2]);
            mathMtxPostRotX(w->beamModel->mtx.m, w->beam->rot[0]);
            mathMtxPostRotY(w->beamModel->mtx.m, w->beam->rot[1]);
            mathMtxPostTrans(w->beamModel->mtx.m, w->beam->pos[0], w->beam->pos[1], w->beam->pos[2]);
            mathMtxIdent(w->glowModel->mtx.m);
            mathMtxScaleRotX(w->glowModel->mtx.m, w->glow->scale[0]);
            mathMtxScaleRotY(w->glowModel->mtx.m, w->glow->scale[1]);
            mathMtxScaleRotZ(w->glowModel->mtx.m, w->glow->scale[2]);
            mathMtxPostRotZ(w->glowModel->mtx.m, w->glow->rot[2]);
            mathMtxPostRotX(w->glowModel->mtx.m, w->glow->rot[0]);
            mathMtxPostRotY(w->glowModel->mtx.m, w->glow->rot[1]);
            mathMtxPostTrans(w->glowModel->mtx.m, w->glow->pos[0], w->glow->pos[1], w->glow->pos[2]);
            if (gBattleFlags & 0x4000000) {
                w->frame->dl = (*bossPrimEnvColorDL)(0xFF, 0xFF, 0xFF, 0xFF, 0, 0, w->alpha * 2);
                w->model->dl = (*bossPrimEnvColorDL)(0xFF, 0xFF, 0xFF, 0xFF, 0, 0, w->alpha * 2);
                w->beam->dl = (*bossPrimEnvColorDL)(0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0x40, w->alpha);
                w->beamModel->dl = (*bossPrimEnvColorDL)(0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0x40, w->alpha);
            } else {
                w->frame->dl = (*bossPrimEnvColorDL)(0xFF, 0xFF, 0xFF, 0, 0, 0xFF, w->alpha * 2);
                w->model->dl = (*bossPrimEnvColorDL)(0xFF, 0xFF, 0xFF, 0, 0, 0xFF, w->alpha * 2);
                w->beam->dl = (*bossPrimEnvColorDL)(0xFF, 0xFF, 0xFF, 0x40, 0x40, 0xFF, w->alpha);
                w->beamModel->dl = (*bossPrimEnvColorDL)(0xFF, 0xFF, 0xFF, 0x40, 0x40, 0xFF, w->alpha);
            }
            w->glow->dl = (*bossPrimEnvColorDL)(0xFF, 0xFF, 0xFF, 0x80, 0x80, 0x80, w->alpha);
            w->glowModel->dl = (*bossPrimEnvColorDL)(0xFF, 0xFF, 0xFF, 0x80, 0x80, 0x80, w->alpha);
            if (gBattleFlags2 & 0x40000) {
                w->frame->dl = (*bossPrimEnvColorDL)(0, 0, 0, 0xFF, 0, 0xFF, w->alpha * 2);
                w->model->dl = (*bossPrimEnvColorDL)(0xFF, 0, 0xFF, 0xFF, 0x80, 0xFF, w->alpha * 2);
                w->beam->dl = (*bossPrimEnvColorDL)(0, 0, 0, 0xFF, 0x78, 0xFF, w->alpha);
                w->beamModel->dl = (*bossPrimEnvColorDL)(0xFF, 0x5A, 0, 0xFF, 0x78, 0, w->alpha);
            }
            if (!(gGameFrameCount & 3)) {
                w->frame->dl = (*bossPrimEnvColorDL)(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, w->alpha * 2);
                w->model->dl = (*bossPrimEnvColorDL)(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, w->alpha * 2);
                w->beam->dl = (*bossPrimEnvColorDL)(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, w->alpha);
                w->beamModel->dl = (*bossPrimEnvColorDL)(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, w->alpha);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "vc": an object with vertex colours, six times its size at (0, 0, -100), animating; every
 * 180 frames it is lit white for 60, then black at half and full strength for 60 each. */
s32 vc(Obj *obj, s16 msg, void *arg) {
    VcWork *w = (VcWork *) ((u32) obj + sizeof(Obj));
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "vc";
            w->frame = (AnimeFrame *) frmCreate(gFrmLayer2, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetAnimAt(w->frame, impVertexColouredAnim, 0xEA, 0.0f);
            (*bossSetFrameFlags2)(w->frame, 0x20);
            w->frame->scale[2] = 6.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = -100.0f;
            w->unused8 = 0;
            PRINTF("頂点カラー物体発生\n");
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            bossStepAnim(w->frame, 0.1f, 0);
            break;
        case 3:
            if (gGameFrameCount % 180 > 120) {
                w->frame->dl = (*bossFogTexEnvColorDL)(0, 0, 0, 255);
                PRINTF("\r[0]");
            } else if (gGameFrameCount % 180 > 60) {
                w->frame->dl = (*bossFogTexEnvColorDL)(0, 0, 0, 0x80);
                PRINTF("\r[1]");
            } else {
                w->frame->dl = (*bossFogTexEnvColorDL)(255, 255, 255, 0x80);
                PRINTF("\r[2]");
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The second "Bbg": the backdrop of file 0xF1, scrolled by a fifth of gCockpitAimX (up to 4 a
 * frame) and a tenth of gCockpitAimY, and rising by 2 a frame while flag 0x200 of gBattleFlags2 is
 * on. */
s32 BbgBoss5(Obj *obj, s16 msg, void *arg) {
    BbgWork *w = (BbgWork *) ((u32) obj + sizeof(Obj));
    f32 dx;
    f32 dy;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "Bbg";
            w->frame = (BGFrame *) (*frmCreate)(gFrmLayers[0], gFrmKindBg);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->w = 0x20;
            w->frame->h = 0x10;
            w->frame->tileCount = 0x40;
            w->frame->img1 = (*floadSegToRam)((u32) impBackdrop2Map, 0xF1, 0);
            w->frame->img2 = (*floadSegToRam)((u32) impBackdropTiles, 0xF1, 0);
            w->frame->palA = (u16 *) (*floadSegToRam)((u32) impBackdropPal, 0xF1, 0);
            w->level = 0x40;
            break;
        case 1:
            (*frmDestroy)(&w->frame->f);
            break;
        case 0:
            dx = (s32) (gCockpitAimX * 0.2);
            dy = (s32) (-gCockpitAimY * 0.1);
            if (dx > 4.0) {
                dx = 4.0f;
            }
            if (dx < -4.0) {
                dx = -4.0f;
            }
            w->frame->x += dx;
            w->frame->y += dy;
            w->frame->alpha = w->level;
            if (w->level >= 40) {
                w->level = 40;
            }
            if (w->level <= 0) {
                w->level = 0;
            }
            if (gBattleFlags2 & 0x200) {
                w->frame->x += 0.0f;
                w->frame->y += 2.0f;
            }
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* The third "Bappcam": a fixed shot from (2000, 100, 0) at (2000, 200, -1000), letterboxed;
 * the eye then moves toward (1000, -300, -1000) looking at (2000, 500, -1000), from frame 640
 * toward (2000, 300, 1000) looking at (2000, 100, -1000), and at frame 1040 it ends. */
s32 Bappcam3(Obj *obj, s16 msg, void *arg) {
    s32 pad[10];
    BCam *w = BCAM(obj);

    switch (msg) {
        case 2:
            obj->name = "Bappcam";
            (*fadeCreate)(gEffectTask);
            w->cam = (Camera *) (*tskCreate)(obj, dracam, 0, NULL);
            if (w->cam == NULL) {
                return -1;
            }
            gBattleFlags2 |= 0x100;
            (*camSetFov)(w->cam, 0.69813175f);
            (*camSetSpeed)(w->cam, 300.0f);
            (*camSetEye)(w->cam, 2000.0f, 100.0f, 0.0f);
            (*camSetAt)(w->cam, 2000.0f, 200.0f, -1000.0f);
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
            break;
        case 0:
            (*camGetView)(w->cam);
            switch (w->step) {
                case 0:
                    (*camSetFov)(w->cam, 0.69813175f);
                    (*camMoveEye)(w->cam, 1000.0f, -300.0f, -1000.0f);
                    (*camMoveAt)(w->cam, 2000.0f, 500.0f, -1000.0f);
                    if (w->timer == 640) {
                        w->step++;
                    }
                    break;
                case 1:
                    (*camMoveEye)(w->cam, 2000.0f, 300.0f, 1000.0f);
                    (*camMoveAt)(w->cam, 2000.0f, 100.0f, -1000.0f);
                    if (w->timer == 1040) {
                        gBattleFlags &= ~0x1000000;
                        gBattleFlags2 &= ~0x100;
                        return -1;
                    }
                    break;
                case 2:
                    break;
                case 3:
                    break;
                case 4:
                    break;
                case 5:
                    break;
                case 6:
                    break;
            }
            w->timer++;
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* The first "Bword": one of the closing banner's nine words, big at its start, shrinking by
 * 0.03 a frame as it flies to its place in the line (x -14 to 18, y 5 or -3) with a trail
 * of the second "Bword" every other frame; there it shrinks to 0.09 and moves under the
 * effects' frame, and goes after 400 frames. The first word instead stays, nudged by the
 * D-pad (a leftover tool: "effect txy[%f][%f]"). */
s32 Bword(Obj *obj, s16 msg, void *arg) {
    BwordWork *w = (BwordWork *) ((u32) obj + sizeof(Obj));

    switch (msg) {
        case 2:
            obj->name = "Bword";
            w->frame = (PolFrame *) (*frmCreateNext)(gFramePool10, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->kind = ((WordArg *) arg)->n;
            if (w->kind == 0) {
                (*bossSetPolDL)(w->frame, impBannerWord0Dl, 0xF2);
            }
            if (w->kind == 1) {
                (*bossSetPolDL)(w->frame, impBannerWord1Dl, 0xF2);
            }
            if (w->kind == 2) {
                (*bossSetPolDL)(w->frame, impBannerWord2Dl, 0xF2);
            }
            if (w->kind == 3) {
                (*bossSetPolDL)(w->frame, impBannerWord0Dl, 0xF2);
            }
            if (w->kind == 4) {
                (*bossSetPolDL)(w->frame, impBannerWord4Dl, 0xF2);
            }
            if (w->kind == 5) {
                (*bossSetPolDL)(w->frame, impBannerWord5Dl, 0xF2);
            }
            if (w->kind == 6) {
                (*bossSetPolDL)(w->frame, impBannerWord2Dl, 0xF2);
            }
            if (w->kind == 7) {
                (*bossSetPolDL)(w->frame, impBannerWord0Dl, 0xF2);
            }
            if (w->kind == 8) {
                (*bossSetPolDL)(w->frame, impBannerWord1Dl, 0xF2);
            }
            (*bossSetFrameFlags)(w->frame, 0x20);
            if (w->kind == 0) {
                w->frame->pos[0] = -97.0f;
                w->frame->pos[1] = 111.0f;
                w->frame->pos[2] = 100.0f;
                w->frame->scale[2] = 2.3f;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            }
            if (w->kind == 1) {
                w->frame->pos[0] = -137.0f;
                w->frame->pos[1] = 5.0f;
                w->frame->pos[2] = 100.0f;
                w->frame->scale[2] = 2.3f;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            }
            if (w->kind == 2) {
                w->frame->pos[0] = -97.0f;
                w->frame->pos[1] = -101.0f;
                w->frame->pos[2] = 100.0f;
                w->frame->scale[2] = 2.3f;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            }
            if (w->kind == 3) {
                w->frame->pos[0] = 95.0f;
                w->frame->pos[1] = -93.0f;
                w->frame->pos[2] = 100.0f;
                w->frame->scale[2] = 2.3f;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            }
            if (w->kind == 4) {
                w->frame->pos[0] = 138.0f;
                w->frame->pos[1] = 8.0f;
                w->frame->pos[2] = 100.0f;
                w->frame->scale[2] = 2.3f;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            }
            if (w->kind == 5) {
                w->frame->pos[0] = 97.0f;
                w->frame->pos[1] = 96.0f;
                w->frame->pos[2] = 100.0f;
                w->frame->scale[2] = 2.3f;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            }
            if (w->kind == 6) {
                w->frame->pos[0] = -3.0f;
                w->frame->pos[1] = -151.0f;
                w->frame->pos[2] = 100.0f;
                w->frame->scale[2] = 2.3f;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            }
            if (w->kind == 7) {
                w->frame->pos[0] = -3.0f;
                w->frame->pos[1] = -151.0f;
                w->frame->pos[2] = 100.0f;
                w->frame->scale[2] = 2.3f;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            }
            if (w->kind == 8) {
                w->frame->pos[0] = -3.0f;
                w->frame->pos[1] = -151.0f;
                w->frame->pos[2] = 100.0f;
                w->frame->scale[2] = 2.3f;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            }
            w->alpha = 255;
            w->count = 0;
            w->unused34 = 0;
            w->stage = 0;
            break;
        case 1:
            (*frmDestroy)(&w->frame->f);
            break;
        case 0:
            (*bossApplyVel)(obj);
            if (w->kind == 0) {
                switch (w->stage) {
                    case 0:
                        w->frame->scale[0] -= 0.03;
                        w->frame->scale[1] -= 0.03;
                        w->frame->scale[2] -= 0.03;
                        if (!(w->count & 1)) {
                            clearWordEffectCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], w->frame->scale[0],
                                                   w->frame->scale[1], w->frame->scale[2], 0);
                        }
                        if ((*BmoveX)(obj, -14.0f, 5.0f, -50.0f, 1.5f) == 0) {
                            w->frame->scale[2] = 0.09f;
                            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                            frmMove(&w->frame->f, gFrmEffectLayer);
                            w->vel[2] = 0.0f;
                            w->vel[1] = 0.0f;
                            w->vel[0] = 0.0f;
                            w->frame->rot[2] = 0.0f;
                            w->stage++;
                        }
                        break;
                    case 1:
                        if (joyGetHeldTime(gCockpitPort, gCockpitBtnDUp) == 1) {
                            w->frame->pos[1] -= 1.0f;
                        }
                        if (joyGetHeldTime(gCockpitPort, gCockpitBtnDDown) == 1) {
                            w->frame->pos[1] += 1.0f;
                        }
                        if (joyGetHeldTime(gCockpitPort, gCockpitBtnDRight) == 1) {
                            w->frame->pos[0] += 1.0f;
                        }
                        if (joyGetHeldTime(gCockpitPort, gCockpitBtnDLeft) == 1) {
                            w->frame->pos[0] -= 1.0f;
                        }
                        PRINTF("effect txy[%f][%f]\n", w->frame->pos[0], w->frame->pos[1]);
                        break;
                }
            } else if (w->kind == 1) {
                switch (w->stage) {
                    case 0:
                        w->frame->scale[0] -= 0.03;
                        w->frame->scale[1] -= 0.03;
                        w->frame->scale[2] -= 0.03;
                        if (!(w->count & 1)) {
                            clearWordEffectCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], w->frame->scale[0],
                                                   w->frame->scale[1], w->frame->scale[2], 1);
                        }
                        if ((*BmoveX)(obj, -8.0f, 5.0f, -50.0f, 1.5f) == 0) {
                            w->frame->scale[2] = 0.09f;
                            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                            frmMove(&w->frame->f, gFrmEffectLayer);
                            w->vel[2] = 0.0f;
                            w->vel[1] = 0.0f;
                            w->vel[0] = 0.0f;
                            w->frame->rot[2] = 0.0f;
                            w->stage++;
                        }
                        break;
                    case 1:
                        if (w->count > 400) {
                            return -1;
                        }
                        break;
                }
            } else if (w->kind == 2) {
                switch (w->stage) {
                    case 0:
                        w->frame->scale[0] -= 0.03;
                        w->frame->scale[1] -= 0.03;
                        w->frame->scale[2] -= 0.03;
                        if (!(w->count & 1)) {
                            clearWordEffectCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], w->frame->scale[0],
                                                   w->frame->scale[1], w->frame->scale[2], 2);
                        }
                        if ((*BmoveX)(obj, -2.0f, 5.0f, -50.0f, 1.5f) == 0) {
                            w->frame->scale[2] = 0.09f;
                            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                            frmMove(&w->frame->f, gFrmEffectLayer);
                            w->vel[2] = 0.0f;
                            w->vel[1] = 0.0f;
                            w->vel[0] = 0.0f;
                            w->frame->rot[2] = 0.0f;
                            w->stage++;
                        }
                        break;
                    case 1:
                        if (w->count > 400) {
                            return -1;
                        }
                        break;
                }
            } else if (w->kind == 3) {
                switch (w->stage) {
                    case 0:
                        w->frame->scale[0] -= 0.03;
                        w->frame->scale[1] -= 0.03;
                        w->frame->scale[2] -= 0.03;
                        if (!(w->count & 1)) {
                            clearWordEffectCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], w->frame->scale[0],
                                                   w->frame->scale[1], w->frame->scale[2], 3);
                        }
                        if ((*BmoveX)(obj, 4.0f, 5.0f, -50.0f, 1.5f) == 0) {
                            w->frame->scale[2] = 0.09f;
                            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                            frmMove(&w->frame->f, gFrmEffectLayer);
                            w->vel[2] = 0.0f;
                            w->vel[1] = 0.0f;
                            w->vel[0] = 0.0f;
                            w->frame->rot[2] = 0.0f;
                            w->stage++;
                        }
                        break;
                    case 1:
                        if (w->count > 400) {
                            return -1;
                        }
                        break;
                }
            } else if (w->kind == 4) {
                switch (w->stage) {
                    case 0:
                        w->frame->scale[0] -= 0.03;
                        w->frame->scale[1] -= 0.03;
                        w->frame->scale[2] -= 0.03;
                        if (!(w->count & 1)) {
                            clearWordEffectCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], w->frame->scale[0],
                                                   w->frame->scale[1], w->frame->scale[2], 4);
                        }
                        if ((*BmoveX)(obj, -6.0f, -3.0f, -50.0f, 1.5f) == 0) {
                            w->frame->scale[2] = 0.09f;
                            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                            frmMove(&w->frame->f, gFrmEffectLayer);
                            w->vel[2] = 0.0f;
                            w->vel[1] = 0.0f;
                            w->vel[0] = 0.0f;
                            w->frame->rot[2] = 0.0f;
                            w->stage++;
                        }
                        break;
                    case 1:
                        if (w->count > 400) {
                            return -1;
                        }
                        break;
                }
            } else if (w->kind == 5) {
                switch (w->stage) {
                    case 0:
                        w->frame->scale[0] -= 0.03;
                        w->frame->scale[1] -= 0.03;
                        w->frame->scale[2] -= 0.03;
                        if (!(w->count & 1)) {
                            clearWordEffectCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], w->frame->scale[0],
                                                   w->frame->scale[1], w->frame->scale[2], 5);
                        }
                        if ((*BmoveX)(obj, 0.0f, -3.0f, -50.0f, 1.5f) == 0) {
                            w->frame->scale[2] = 0.09f;
                            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                            frmMove(&w->frame->f, gFrmEffectLayer);
                            w->vel[2] = 0.0f;
                            w->vel[1] = 0.0f;
                            w->vel[0] = 0.0f;
                            w->frame->rot[2] = 0.0f;
                            w->stage++;
                        }
                        break;
                    case 1:
                        if (w->count > 400) {
                            return -1;
                        }
                        break;
                }
            } else if (w->kind == 6) {
                switch (w->stage) {
                    case 0:
                        w->frame->scale[0] -= 0.03;
                        w->frame->scale[1] -= 0.03;
                        w->frame->scale[2] -= 0.03;
                        if (!(w->count & 1)) {
                            clearWordEffectCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], w->frame->scale[0],
                                                   w->frame->scale[1], w->frame->scale[2], 6);
                        }
                        if ((*BmoveX)(obj, 6.0f, -3.0f, -50.0f, 1.5f) == 0) {
                            w->frame->scale[2] = 0.09f;
                            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                            frmMove(&w->frame->f, gFrmEffectLayer);
                            w->vel[2] = 0.0f;
                            w->vel[1] = 0.0f;
                            w->vel[0] = 0.0f;
                            w->frame->rot[2] = 0.0f;
                            w->stage++;
                        }
                        break;
                    case 1:
                        if (w->count > 400) {
                            return -1;
                        }
                        break;
                }
            } else if (w->kind == 7) {
                switch (w->stage) {
                    case 0:
                        w->frame->scale[0] -= 0.03;
                        w->frame->scale[1] -= 0.03;
                        w->frame->scale[2] -= 0.03;
                        if (!(w->count & 1)) {
                            clearWordEffectCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], w->frame->scale[0],
                                                   w->frame->scale[1], w->frame->scale[2], 6);
                        }
                        if ((*BmoveX)(obj, 12.0f, -3.0f, -50.0f, 1.5f) == 0) {
                            w->frame->scale[2] = 0.09f;
                            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                            frmMove(&w->frame->f, gFrmEffectLayer);
                            w->vel[2] = 0.0f;
                            w->vel[1] = 0.0f;
                            w->vel[0] = 0.0f;
                            w->frame->rot[2] = 0.0f;
                            w->stage++;
                        }
                        break;
                    case 1:
                        if (w->count > 400) {
                            return -1;
                        }
                        break;
                }
            } else if (w->kind == 8) {
                switch (w->stage) {
                    case 0:
                        w->frame->scale[0] -= 0.03;
                        w->frame->scale[1] -= 0.03;
                        w->frame->scale[2] -= 0.03;
                        if (!(w->count & 1)) {
                            clearWordEffectCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], w->frame->scale[0],
                                                   w->frame->scale[1], w->frame->scale[2], 6);
                        }
                        if ((*BmoveX)(obj, 18.0f, -3.0f, -50.0f, 1.5f) == 0) {
                            w->frame->scale[2] = 0.09f;
                            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                            frmMove(&w->frame->f, gFrmEffectLayer);
                            w->vel[2] = 0.0f;
                            w->vel[1] = 0.0f;
                            w->vel[0] = 0.0f;
                            w->frame->rot[2] = 0.0f;
                            w->stage++;
                        }
                        break;
                    case 1:
                        if (w->count > 400) {
                            return -1;
                        }
                        break;
                }
            }
            w->count++;
            break;
        case 3:
            w->frame->dl = (*bossCombineEnvColorDL)(0, 0, 0, 255);
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* A "Bword" (word n of the banner), under the battle's task; the point is unused. */
void clearWordCreate(f32 x, f32 y, f32 z, s32 n) {
    WordArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.n = n;
    arg.pos[2] = z;
    if ((*tskCreate)(gEffectTask, Bword, gBattleTask->file, &arg) == NULL) {
        PRINTF("[clearWordCreate]タスクの作成に失敗しました。\n");
    }
}

/* The second "Bword": a word of the closing banner (one of nine, file 0xF2) at a point and a
 * scale, shrinking a little while it fades by 10 (from 0x80). */
s32 BwordEffect(Obj *obj, s16 msg, void *arg) {
    WordWork *w = (WordWork *) ((u32) obj + sizeof(Obj));

    switch (msg) {
        case 2:
            obj->name = "Bword";
            w->frame = (PolFrame *) (*frmCreateNext)(gFramePool10, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->kind = ((WordFxArg *) arg)->n;
            if (w->kind == 0) {
                (*bossSetPolDL)(w->frame, impBannerWord0Dl, 0xF2);
            }
            if (w->kind == 1) {
                (*bossSetPolDL)(w->frame, impBannerWord1Dl, 0xF2);
            }
            if (w->kind == 2) {
                (*bossSetPolDL)(w->frame, impBannerWord2Dl, 0xF2);
            }
            if (w->kind == 3) {
                (*bossSetPolDL)(w->frame, impBannerWord0Dl, 0xF2);
            }
            if (w->kind == 4) {
                (*bossSetPolDL)(w->frame, impBannerWord4Dl, 0xF2);
            }
            if (w->kind == 5) {
                (*bossSetPolDL)(w->frame, impBannerWord5Dl, 0xF2);
            }
            if (w->kind == 6) {
                (*bossSetPolDL)(w->frame, impBannerWord2Dl, 0xF2);
            }
            if (w->kind == 7) {
                (*bossSetPolDL)(w->frame, impBannerWord0Dl, 0xF2);
            }
            if (w->kind == 8) {
                (*bossSetPolDL)(w->frame, impBannerWord1Dl, 0xF2);
            }
            (*bossSetFrameFlags)(w->frame, 0x27);
            w->frame->scale[0] = ((WordFxArg *) arg)->a[3];
            w->frame->scale[1] = ((WordFxArg *) arg)->a[4];
            w->frame->scale[2] = ((WordFxArg *) arg)->a[5];
            w->frame->pos[0] = ((WordFxArg *) arg)->a[0];
            w->frame->pos[1] = ((WordFxArg *) arg)->a[1];
            w->frame->pos[2] = ((WordFxArg *) arg)->a[2];
            w->alpha = 0x80;
            w->count = 0;
            w->stage = 0;
            break;
        case 1:
            (*frmDestroy)(&w->frame->f);
            break;
        case 0:
            w->frame->scale[0] -= 0.005;
            w->frame->scale[1] -= 0.005;
            w->frame->scale[2] -= 0.005;
            w->alpha -= 10;
            if (w->alpha < 0) {
                return -1;
            }
            break;
        case 3:
            w->frame->dl = (*bossCombineEnvColorDL)(0x80, 0x80, 0x80, w->alpha);
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* The second "Bword" at a point and a scale, under the battle's task. */
void clearWordEffectCreate(f32 a, f32 b, f32 c, f32 d, f32 e, f32 f, s32 n) {
    WordFxArg arg;

    arg.a[0] = a;
    arg.a[1] = b;
    arg.a[2] = c;
    arg.a[3] = d;
    arg.a[4] = e;
    arg.a[5] = f;
    arg.n = n;
    if ((*tskCreate)(gEffectTask, BwordEffect, gBattleTask->file, &arg) == NULL) {
        PRINTF("[clearWordEffectCreate]タスクの作成に失敗しました。\n");
    }
}

/* "Bclear": the battle won. A brown tint; at frame 15 a sound (0x1B); the banner's nine words
 * from frame 5 to 85, ten apart; a sound (0x206) every 10 frames from 70 to 150; at 300 flag
 * 0x800000 of gBattleFlags2; at 330 the first player's character's voice (0x13A, 0x139, 0x13B,
 * 0x13C); it ends at 900. */
s32 Bclear(Obj *obj, s16 msg, void *arg) {
    ClearWork *w = (ClearWork *) ((u32) obj + sizeof(Obj));
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Bclear";
            w->time = 0;
            w->stage = 0;
            break;
        case 1:
            break;
        case 3:
            break;
        case 0:
            PRINTF("\rPackTimer[%d]", w->time);
            switch (w->stage) {
                case 0:
                    (*beBrownFCreate)(0.0f, 0.0f, 180.0f, 0);
                    w->stage++;
                    break;
                case 1:
                    if (w->time == 15) {
                        (*sndPlay)(0x1B);
                    }
                    if (w->time == 5) {
                        clearWordCreate(3.0f, 11.0f, -50.0f, 0);
                    }
                    if (w->time == 15) {
                        clearWordCreate(3.0f, 5.0f, -50.0f, 1);
                    }
                    if (w->time == 25) {
                        clearWordCreate(3.0f, -1.0f, -50.0f, 2);
                    }
                    if (w->time == 35) {
                        clearWordCreate(-3.0f, 8.0f, -50.0f, 3);
                    }
                    if (w->time == 45) {
                        clearWordCreate(-3.0f, 2.0f, -50.0f, 4);
                    }
                    if (w->time == 55) {
                        clearWordCreate(-3.0f, -4.0f, -50.0f, 5);
                    }
                    if (w->time == 65) {
                        clearWordCreate(-3.0f, -10.0f, -50.0f, 6);
                    }
                    if (w->time == 75) {
                        clearWordCreate(-3.0f, -10.0f, -50.0f, 7);
                    }
                    if (w->time == 85) {
                        clearWordCreate(-3.0f, -10.0f, -50.0f, 8);
                    }
                    if (w->time == 70) {
                        (*sndPlay)(0x206);
                    }
                    if (w->time == 80) {
                        (*sndPlay)(0x206);
                    }
                    if (w->time == 90) {
                        (*sndPlay)(0x206);
                    }
                    if (w->time == 100) {
                        (*sndPlay)(0x206);
                    }
                    if (w->time == 110) {
                        (*sndPlay)(0x206);
                    }
                    if (w->time == 120) {
                        (*sndPlay)(0x206);
                    }
                    if (w->time == 130) {
                        (*sndPlay)(0x206);
                    }
                    if (w->time == 140) {
                        (*sndPlay)(0x206);
                    }
                    if (w->time == 150) {
                        (*sndPlay)(0x206);
                    }
                    if (w->time == 155) {
                        w->stage++;
                    }
                    break;
                case 2:
                    if (w->time == 300) {
                        gBattleFlags2 |= 0x800000;
                        w->stage++;
                    }
                    break;
                case 3:
                    if (w->time == 330) {
                        if (gPlayer1Chara == 0) {
                            (*sndPlay)(0x13A);
                        } else if (gPlayer1Chara == 1) {
                            (*sndPlay)(0x139);
                        } else if (gPlayer1Chara == 2) {
                            (*sndPlay)(0x13B);
                        } else if (gPlayer1Chara == 3) {
                            (*sndPlay)(0x13C);
                        }
                    }
                    if (w->time == 900) {
                        return -1;
                    }
                    break;
            }
            w->time++;
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* A "Bclear", under the battle's task. */
void clearPackCreate(f32 x, f32 y, f32 z, s32 n) {
    ClearArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.n = n;
    arg.pos[2] = z;
    if ((*tskCreate)(gEffectTask, Bclear, gBattleTask->file, &arg) == NULL) {
        PRINTF("[clearPackCreate]タスクの作成に失敗しました。\n");
    }
}
