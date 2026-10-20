#include "common.h"
#include "object.h"
#include "player.h"
#include "game.h"
#include "boss.h"
#include "beffect.h"
#include "bmove.h"
#include "fade.h"
#include "fileload.h"
#include "gfxbuf.h"
#include "gfxmgr.h"
#include "mathutil.h"
#include "polygon.h"
#include "sndloc.h"
#include "tmanager.h"

/* The game's debug print, argument-only (strings kept, arguments evaluated): what some of this
 * file's functions were built with, where the repo's `if (0)` PRINTF moves registers. */
#define DPRINTF

/* The boss battle's effects (overlay 10): "Beffect", the filters ("Filter", "Efilter",
 * "Pfilter"), hit rings, balls and marks, the AT field, smoke, flashes, the mega ball, the
 * demo letterbox ("Belt"), punch sparks, fades and more. Most are a task handler with a
 * spawner that fills the task's argument on the stack. */

/* "Beffect"'s work. */
typedef struct BeffWork {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ s32 alpha;
} BeffWork;

/* An effect's work. */
typedef struct EffWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ Anim anim; /* a texture animation into the frame's segment 6 */
    /* 0x14 */ f32 phase;
    /* 0x18 */ s32 alpha;
    /* 0x1C */ s32 step;
    /* 0x20 */ u8 pad20[0x1C];
    /* 0x3C */ s32 fade;
    /* 0x40 */ u8 pad40[2];
    /* 0x42 */ u8 stage;
} EffWork;

#define EFF(task) ((EffWork *) ((u32) (task) + sizeof(Obj)))

/* The arguments the spawners pass. */
typedef struct HitArg {
    /* 0x00 */ u8 pad0[0x24];
    /* 0x24 */ f32 pos[3];
    /* 0x30 */ f32 scale;
    /* 0x34 */ f32 rot;
    /* 0x38 */ u8 pad38[0xA];
    /* 0x42 */ u8 flip;
} HitArg; /* size 0x44 */

/* "flashF"'s work and argument. */
typedef struct FlashWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad4[0x10];
    /* 0x14 */ f32 time;
    /* 0x18 */ s32 alpha;
    /* 0x1C */ s32 rgb[3];
} FlashWork;

typedef struct FlashArg {
    /* 0x00 */ u8 pad0[0x14];
    /* 0x14 */ f32 time;
    /* 0x18 */ u8 pad18[4];
    /* 0x1C */ s32 rgb[3];
} FlashArg; /* size 0x28 */

/* "Megaball"'s work and argument. */
typedef struct MegaWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 time;
    /* 0x08 */ u8 pad8[4];
    /* 0x0C */ s32 alpha;
} MegaWork;

typedef struct MegaArg {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ f32 time;
    /* 0x08 */ f32 scale;
    /* 0x0C */ u8 padC[4];
    /* 0x10 */ s32 pos[3];
    /* 0x1C */ u8 pad1C[4];
} MegaArg; /* size 0x20 */

/* "Belt"'s work and argument: the demo letterbox, two black bars and their edges. */
typedef struct BeltWork {
    /* 0x00 */ DLFrame *frame;
    /* 0x04 */ s32 topX, topY; /* the top bar's lower right corner */
    /* 0x0C */ s32 bottomX, bottomY; /* the bottom bar's upper left corner */
    /* 0x14 */ s32 time;
} BeltWork;

typedef struct BeltArg {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ s32 top[2];
    /* 0x0C */ s32 bottom[2];
    /* 0x14 */ s32 time;
} BeltArg; /* size 0x18 */

/* "flashP"'s work and argument. */
typedef struct FlashPWork {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ u8 pad4[0xC];
    /* 0x10 */ s32 count;
    /* 0x14 */ s32 step;
    /* 0x18 */ s32 alpha;
} FlashPWork;

typedef struct FlashPArg {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ s32 count;
    /* 0x14 */ u8 pad14[8];
} FlashPArg; /* size 0x1C */

typedef struct MarkArg {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ f32 scale;
    /* 0x14 */ u8 pad14[8];
} MarkArg; /* size 0x1C */

typedef struct AtfArg {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ f32 scale;
    /* 0x14 */ u8 pad14[0xC];
} AtfArg; /* size 0x20 */

typedef struct SmokeArg {
    /* 0x00 */ u8 pad0[0x28];
    /* 0x28 */ f32 pos[3];
    /* 0x34 */ f32 size;
    /* 0x38 */ u8 pad38[4];
    /* 0x3C */ s32 kind;
    /* 0x40 */ f32 dist;
    /* 0x44 */ u8 pad44[4];
} SmokeArg; /* size 0x48 */

/* "smoke6"'s work: six puffs in a ring. */
typedef struct Smoke6Work {
    /* 0x00 */ PolFrame *frames[6];
    /* 0x18 */ Anim anim;
    /* 0x28 */ f32 pos[3];
    /* 0x34 */ u8 pad34[4];
    /* 0x38 */ s32 alpha;
    /* 0x3C */ s32 kind;
    /* 0x40 */ f32 dist;
} Smoke6Work;

/* A spark's work: a battle object's (bmove.c: its frame and its velocity), a point and a fade. */
typedef struct SparkWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ f32 target[3];
    /* 0x1C */ u8 pad1C[8];
    /* 0x24 */ s32 fade;
} SparkWork;

#define SPARK(task) ((SparkWork *) ((u32) (task) + sizeof(Obj)))

/* "effectP"'s work and argument: an animated effect of one of four kinds. "psmoke" passes its
 * argument in a work's shape. */
typedef struct EffPWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ Anim anim;
    /* 0x14 */ f32 pos[3];
    /* 0x20 */ f32 scale;
    /* 0x24 */ s32 alpha;
    /* 0x28 */ s32 kind;
    /* 0x2C */ u8 pad2C[4];
} EffPWork; /* size 0x30 */

typedef struct EffPArg {
    /* 0x00 */ u8 pad0[0x28];
    /* 0x28 */ f32 pos[3];
    /* 0x34 */ f32 scale;
    /* 0x38 */ u8 pad38[4];
    /* 0x3C */ s32 kind;
    /* 0x40 */ u8 pad40[8];
} EffPArg; /* size 0x48 */

/* "reversal"'s work and argument. */
typedef struct RevWork {
    /* 0x00 */ PolFrame *frame; /* a ball, against the fifth boss only */
    /* 0x04 */ PolFrame *ring;
    /* 0x08 */ u8 pad8[0x14];
    /* 0x1C */ s32 fade;
} RevWork;

typedef struct RevArg {
    /* 0x00 */ u8 pad0[8];
    /* 0x08 */ f32 pos[3];
    /* 0x14 */ f32 scale;
    /* 0x18 */ u8 pad18[8];
} RevArg; /* size 0x20 */

/* "Bpiece"'s work and argument: a piece of debris. */
typedef struct PieceWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ f32 fall;
    /* 0x14 */ u8 pad14[0x24];
    /* 0x38 */ s32 unused38; /* copied from the argument's (which the spawner leaves unset), never read */
    /* 0x3C */ s32 fade;
    /* 0x40 */ u8 pad40[2];
    /* 0x42 */ u8 stage;
} PieceWork;

typedef struct PieceArg {
    /* 0x00 */ u8 pad0[0x24];
    /* 0x24 */ f32 pos[3];
    /* 0x30 */ f32 scale;
    /* 0x34 */ u8 pad34[4];
    /* 0x38 */ s32 unused38; /* the spawner leaves it unset */
    /* 0x3C */ u8 pad3C[4];
    /* 0x40 */ s16 spread;
} PieceArg; /* size 0x44 */

/* "Nullattack"'s work and argument: an attack box of a kind, with no model, for one hit. */
typedef struct NullWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ ACollFrame *box;
    /* 0x08 */ s32 kind;
} NullWork;

typedef struct NullArg {
    /* 0x00 */ u8 pad0[8];
    /* 0x08 */ s32 kind;
    /* 0x0C */ f32 pos[3];
} NullArg; /* size 0x18 */

/* "10RYOU"'s work and argument: a 10-ryo coin. */
typedef struct RyouWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad4[0x14];
    /* 0x18 */ s32 time;
    /* 0x1C */ s32 alpha;
    /* 0x20 */ s32 stage;
} RyouWork;

typedef struct RyouArg {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ f32 scale;
    /* 0x18 */ u8 pad18[0xC];
} RyouArg; /* size 0x24 */

/* "Lball"'s work and argument. */
typedef struct LballWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad4[0x24];
    /* 0x28 */ f32 scale;
    /* 0x2C */ u8 pad2C[4];
    /* 0x30 */ s32 time;
    /* 0x34 */ s32 alpha;
    /* 0x38 */ s32 stage;
    /* 0x3C */ s32 unused3C; /* copied from the argument, never read */
} LballWork;

typedef struct LballArg {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ f32 pos[3];
    /* 0x1C */ u8 pad1C[0x20];
    /* 0x3C */ s32 unused3C;
} LballArg; /* size 0x40 */

/* "speffect"'s work and argument: a ring of one of three kinds. */
typedef struct SpWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad4[0xC];
    /* 0x10 */ s32 alpha;
    /* 0x14 */ s32 kind;
    /* 0x18 */ s32 stage;
} SpWork;

typedef struct SpArg {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ s32 kind;
    /* 0x18 */ u8 pad18[4];
} SpArg; /* size 0x1C */

/* "fakeImp"'s work: the Impact's stand-in, played as an animation. */
typedef struct FakeImpWork {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ u8 pad4[0xC];
    /* 0x10 */ s32 unused10; /* set to 0 when made, never read */
    /* 0x14 */ s32 n; /* the argument's n, never read after */
    /* 0x18 */ u32 stage;
    /* 0x1C */ s32 time;
} FakeImpWork;

typedef struct FakeImpArg {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ s32 n;
    /* 0x18 */ u8 pad18[8];
} FakeImpArg; /* size 0x20 */

/* "megaRef"'s work and argument. */
typedef struct MegaRefWork {
    /* 0x00 */ f32 pos; /* given the point's x, y and z in turn */
    /* 0x04 */ u8 pad4[0xC];
    /* 0x10 */ s32 time;
    /* 0x14 */ s32 unused14; /* set to 0 when made, never read */
} MegaRefWork;

typedef struct MegaRefArg {
    /* 0x00 */ f32 pos[3];
    /* 0x0C */ u8 padC[0xC];
} MegaRefArg; /* size 0x18 */

/* "brownF"'s work and argument: a brown screen tint. */
typedef struct BrownWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad4[0x18];
    /* 0x1C */ s32 time;
    /* 0x20 */ s32 stage;
    /* 0x24 */ u8 pad24[4];
    /* 0x28 */ s32 alpha;
} BrownWork;

typedef struct BrownArg {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ f32 pos[3];
    /* 0x1C */ u8 pad1C[8];
    /* 0x24 */ s32 unused24; /* the spawner's 4th parameter; the proc never reads it */
    /* 0x28 */ u8 pad28[4];
} BrownArg; /* size 0x2C */

/* "Shibuki"'s work and argument: a splash of three frames. */
typedef struct ShibukiWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ PolFrame *f1;
    /* 0x08 */ PolFrame *f2;
    /* 0x0C */ s32 unusedC; /* set to 0 when made, never read */
    /* 0x10 */ s32 kind;
    /* 0x14 */ u8 pad14[0xC];
    /* 0x20 */ s32 stage;
    /* 0x24 */ f32 alpha;
} ShibukiWork;

typedef struct ShibukiArg {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ s32 kind;
    /* 0x14 */ f32 pos[3];
    /* 0x20 */ u8 pad20[8];
} ShibukiArg; /* size 0x28 */

/* "fade"'s work, and its argument: the demo letterbox opening from a slit in the middle (or
 * from where the bars are) to top[1] and bottom[1], then to the scissor's edges as time runs
 * out. */
typedef struct FadeWork {
    /* 0x00 */ DLFrame *frame;
    /* 0x04 */ s32 top[2]; /* the top bar's lower right corner, where it ends */
    /* 0x0C */ s32 bottom[2]; /* the bottom bar's upper left corner, where it ends */
    /* 0x14 */ s32 curTop[2]; /* and where they are */
    /* 0x1C */ s32 curBottom[2];
    /* 0x24 */ s32 time;
    /* 0x28 */ s32 keep; /* 0: start from a slit in the middle */
} FadeWork; /* size 0x2C */

/* A FadeWork corner read as named fields: as constant-index array elements, IDO sorts the
 * operands of an OR (IDO_NOTES 198), as it did for "Belt". */
typedef struct FadeCorner {
    s32 x, y;
} FadeCorner;
#define FADE_XY(a) ((FadeCorner *) (a))

typedef struct SparkArg {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ f32 pos[3];
    /* 0x1C */ f32 scale;
    /* 0x20 */ f32 dist;
    /* 0x24 */ u8 pad24[4];
} SparkArg; /* size 0x28 */

extern Gfx *gGfxDlHead; /* the display-list buffer */
extern Obj *gEffectTask; /* the effects' parent */
extern Obj *gBattleTask; /* the battle's task */
extern Frame *gFrmLayer8;
extern s16 gFrmKindPol; /* the "pol" kind */
extern s16 gFrmKindAnime; /* the "anime" kind */
/* Animations: a texture in the battle files (segment 3) and its frames, ended by { 0, 0 }. */
AnimFrame gBeFilterAnim[] = {
    { 0x030380C0, 3 },
    { 0x030390C0, 3 },
    { 0, 0 },
};

AnimFrame gBeSmokeAnim[] = {
    { 0x03000530, 2 }, { 0x03000D30, 2 }, { 0x03001530, 2 }, { 0x03001D30, 2 }, { 0x03002530, 2 },
    { 0x03002D30, 2 }, { 0x03003530, 2 }, { 0x03003D30, 2 }, { 0x03004530, 2 }, { 0x03004D30, 2 },
    { 0x03005530, 2 }, { 0x03005D30, 2 }, { 0, 0 },
};

AnimFrame gBeEffectPAnim0[] = {
    { 0x030000D0, 2 }, { 0x030008D0, 2 }, { 0x030010D0, 2 }, { 0x030018D0, 2 }, { 0x030020D0, 2 },
    { 0x030028D0, 2 }, { 0x030030D0, 2 }, { 0x030038D0, 2 }, { 0x030040D0, 2 }, { 0x030048D0, 2 },
    { 0x030050D0, 2 }, { 0x030058D0, 2 }, { 0x030060D0, 2 }, { 0x030068D0, 2 }, { 0x030070D0, 2 },
    { 0x030078D0, 2 }, { 0, 0 },
};

AnimFrame gBeEffectPAnim12[] = {
    { 0x030000D0, 2 }, { 0x030008D0, 2 }, { 0x030010D0, 2 }, { 0x030018D0, 2 }, { 0x030020D0, 2 },
    { 0x030028D0, 2 }, { 0x030030D0, 2 }, { 0x030038D0, 2 }, { 0x030040D0, 2 }, { 0x030048D0, 2 },
    { 0x030050D0, 2 }, { 0x030058D0, 2 }, { 0x030060D0, 2 }, { 0x030068D0, 2 }, { 0x030070D0, 2 },
    { 0x030078D0, 2 }, { 0, 0 },
};

/* the raster setup */
Gfx gBeBspaceRasterDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                          G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_DISABLE),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetTextureFilter(G_TF_POINT),
    gsDPSetCombineLERP(0, 0, 0, TEXEL0, 0, 0, 0, 1, 0, 0, 0, TEXEL0, 0, 0, 0, 1),
    gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsSPEndDisplayList(),
};
extern u8 beFilterDl[];
extern u8 bePfilterDl[];
extern u8 beHitringDl[];
extern u8 beBallDl[];
extern u8 beHitmarkDl[];
extern u8 beATfieldDl[];
extern u8 beMeffectDl[];
extern u8 beFlashPAnim[];
extern u8 beFlashPLoopAnim[];
extern Frame *gFramePool10;
extern s16 gFrmKindDlist; /* the "dlist" kind */
extern u8 beFlashDl[]; /* the flash, in file 0x120 */
extern u8 beStarDl[]; /* the star, in file 0x124 */
extern u8 beRedStarDl[];
extern u8 beSmokeDl[];
extern Frame *gCockpitPanelFrame;
extern Gfx beBpieceDebrisDl[]; /* a piece of debris, in file 0x11E */
extern Frame *ene_atk_root_frame; /* the enemies' attack boxes' parent */
extern u8 beRyouCoinDl[]; /* the 10-ryo coin, in file 0xEA */
extern CamView *gBattleCamView; /* the battle's camera */
extern Frame *gFrmLayer2;
extern u8 beFakeImpactOmitsuAnim1[]; /* the Impact's animations, by pilot (file 0xEA) */
extern u8 beFakeImpactGoemonAnim1[];
extern u8 beFakeImpactOmitsuAnim2[];
extern u8 beFakeImpactGoemonAnim2[];
extern u8 beFakeImpactOmitsuAnim3[];
extern u8 beFakeImpactGoemonAnim3[];
extern Frame *gFrmLayers[]; /* the frame pools */
extern Frame *gCockpitRootFrame;
extern u8 beShibukiPart1Dl[]; /* the splash's three parts (file 0xEE) */
extern u8 beShibukiPart2Dl[];
extern u8 beShibukiPart3Dl[];
extern u16 gFramebuffers[3][SCREEN_WIDTH * SCREEN_HEIGHT]; /* the framebuffers */
extern u32 gGameFrameCount; /* GameState's frame count */
extern s32 gBeFadeShorten; /* 1: "fade" cuts its time to what opening fully takes */


/* "Beffect": an "anime" frame at (0, -50, -300) five times its size, playing once while it
 * fades. */
s32 beBeffectProc(Obj *obj, s16 msg, void *arg) {
    BeffWork *w = (BeffWork *) ((u32) obj + sizeof(Obj));
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "Beffect";
            w->frame = (AnimeFrame *) frmCreate(gFrmLayer8, gFrmKindAnime);
            w->frame->scale[2] = 5.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = -50.0f;
            w->frame->pos[2] = -300.0f;
            w->alpha = 255;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->alpha -= 4;
            if (w->alpha < 0) {
                w->alpha = 0;
            }
            w->frame->frame += 0.2;
            if (polAnimeLastFrame(w->frame) < w->frame->frame) {
                return -1;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetRenderMode(gGfxDlHead++, 0x5049D8, 0);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Filter": a full-screen tint in the battle's colour (blue, black, green or red by flags 8,
 * 0x10, 0x20, 0x40 of gBossFlags; else white), fading out; hidden while flag 0x1000000 of
 * gBattleFlags is on. */
s32 beFilterProc(Obj *obj, s16 msg, void *arg) {
    EffWork *w = EFF(obj);

    switch (msg) {
        case 2:
            obj->name = "Filter";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, beFilterDl, 0xEA);
            (*bossSetFrameFlags)(w->frame, 0x20);
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[2] = 0.0f;
            w->frame->pos[0] = w->frame->pos[1] = w->frame->pos[2];
            fadeAnimStart(&w->anim, gBeFilterAnim, 0, &w->frame->segs[3],
                                   floadGetSetFile(0xEA, 0));
            fadeAnimStep(&w->anim);
            w->alpha = 0xD0;
            break;
        case 0:
            if (gBattleFlags & 0x1000000) {
                w->frame->f.flags &= ~2;
            } else {
                w->frame->f.flags |= 2;
            }
            fadeAnimStep(&w->anim);
            w->frame->rot[0] = gCockpitRotX;
            w->frame->rot[1] = gCockpitRotY;
            w->frame->rot[2] = gCockpitRotZ;
            w->alpha -= 8;
            if (w->alpha < 0) {
                return -1;
            }
            break;
        case 3:
            if (gBossFlags & 8) {
                w->frame->dl = bossXluEnvColorDL(0, 0, 255, w->alpha);
            } else if (gBossFlags & 0x10) {
                w->frame->dl = bossXluEnvColorDL(0, 0, 0, w->alpha);
            } else if (gBossFlags & 0x20) {
                w->frame->dl = bossXluEnvColorDL(0, 255, 0, w->alpha);
            } else if (gBossFlags & 0x40) {
                w->frame->dl = bossXluEnvColorDL(255, 0, 0, w->alpha);
            } else {
                w->frame->dl = bossXluEnvColorDL(255, 255, 255, w->alpha);
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Efilter": the same at half the strength, without the animation's steps. */
s32 beEfilterProc(Obj *obj, s16 msg, void *arg) {
    EffWork *w = EFF(obj);

    switch (msg) {
        case 2:
            obj->name = "Efilter";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, beFilterDl, 0xEA);
            (*bossSetFrameFlags)(w->frame, 0x20);
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[2] = 0.0f;
            w->frame->pos[0] = w->frame->pos[1] = w->frame->pos[2];
            w->alpha = 255;
            fadeAnimStart(&w->anim, gBeFilterAnim, 0, &w->frame->segs[3],
                                   floadGetSetFile(0xEA, 0));
            break;
        case 0:
            if (gBattleFlags & 0x1000000) {
                w->frame->f.flags &= ~2;
            } else {
                w->frame->f.flags |= 2;
            }
            w->frame->rot[0] = gCockpitRotX;
            w->frame->rot[1] = gCockpitRotY;
            w->frame->rot[2] = gCockpitRotZ;
            w->alpha -= 8;
            if (w->alpha < 0) {
                return -1;
            }
            break;
        case 3:
            if (gBossFlags & 8) {
                w->frame->dl = bossXluPrimEnvColorDL(0, 0, 255, w->alpha / 2);
            } else if (gBossFlags & 0x10) {
                w->frame->dl = bossXluPrimEnvColorDL(0, 0, 0, w->alpha / 2);
            } else if (gBossFlags & 0x20) {
                w->frame->dl = bossXluPrimEnvColorDL(0, 255, 0, w->alpha / 2);
            } else if (gBossFlags & 0x40) {
                w->frame->dl = bossXluPrimEnvColorDL(255, 0, 0, w->alpha / 2);
            } else {
                w->frame->dl = bossXluPrimEnvColorDL(255, 255, 255, w->alpha / 2);
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Pfilter": while flag 0x8000000 of gBattleFlags is on, a black screen pulsing in and out
 * (a phase of 512 steps of 4); hidden while flag 0x1000000 or the 0x400 of gBattleFlags2 is on. */
s32 bePfilterProc(Obj *obj, s16 msg, void *arg) {
    EffWork *w = EFF(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "Pfilter";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, bePfilterDl, 0xEA);
            (*bossSetFrameFlags)(w->frame, 0x20);
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[2] = 0.0f;
            w->frame->pos[0] = w->frame->pos[1] = w->frame->pos[2];
            w->alpha = 0;
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
            if (gBattleFlags & 0x8000000) {
                w->frame->f.flags |= 2;
                if ((gBattleFlags & 0x1000000) || (gBattleFlags2 & 0x400)) {
                    w->frame->f.flags &= ~2;
                } else {
                    w->frame->f.flags |= 2;
                }
                w->phase += 4.0f;
                if (w->phase > 512.0f) {
                    w->phase = 0.0f;
                }
                if (w->phase < 255.0f) {
                    w->alpha = w->phase;
                } else {
                    w->alpha = 512.0f - w->phase;
                }
                if (w->alpha >= 256) {
                    w->alpha = 255;
                }
                if (w->alpha < 0) {
                    w->alpha = 0;
                }
                w->frame->rot[0] = gCockpitRotX;
                w->frame->rot[1] = gCockpitRotY;
                w->frame->rot[2] = gCockpitRotZ;
            } else {
                w->frame->f.flags &= ~2;
                w->alpha = 0;
            }
            break;
        case 3:
            w->frame->dl = bossXluPrimEnvColorDL(0, 0, 0, w->alpha / 2);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A hit ring at a point, a tenth of `scale` and growing, laid flat if flip is 1. */
void beHitringCreate(f32 x, f32 y, f32 z, f32 scale, s32 flip) {
    HitArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = scale;
    arg.flip = flip;
    tskCreate(gEffectTask, beHitringProc, 0, &arg);
}

/* "Hitring": grows by 8% a frame while it fades by 4 (from 0x70). */
s32 beHitringProc(Obj *obj, s16 msg, void *arg) {
    EffWork *w = EFF(obj);

    switch (msg) {
        case 2:
            obj->name = "Hitring";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, beHitringDl, 0xEE);
            (*bossSetFrameFlags)(w->frame, 0x20);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                ((HitArg *) arg)->scale / 10.0f;
            w->frame->pos[0] = ((HitArg *) arg)->pos[0];
            w->frame->pos[1] = ((HitArg *) arg)->pos[1];
            w->frame->pos[2] = ((HitArg *) arg)->pos[2];
            if (((HitArg *) arg)->flip == 1) {
                w->frame->rot[0] = -1.5707964f;
            } else {
                w->frame->flags |= 2;
            }
            w->fade = 0x70;
            break;
        case 0:
            w->fade -= 4;
            if (w->fade <= 0) {
                w->fade = 0;
                return -1;
            }
            w->frame->scale[0] *= 1.08;
            w->frame->scale[1] *= 1.08;
            w->frame->scale[2] *= 1.08;
            break;
        case 3:
            w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, 255, 255, 255, w->fade);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A hit ball at a point. */
void beHitballCreate(f32 x, f32 y, f32 z, f32 scale) {
    HitArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = scale;
    tskCreate(gEffectTask, beHitballProc, 0, &arg);
}

/* "Hitball": spins and grows by 10% a frame while it fades (to 100 times its size). */
s32 beHitballProc(Obj *obj, s16 msg, void *arg) {
    EffWork *w = EFF(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Hitball";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, beBallDl, 0x120);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                ((HitArg *) arg)->scale;
            w->frame->pos[0] = ((HitArg *) arg)->pos[0];
            w->frame->pos[1] = ((HitArg *) arg)->pos[1];
            w->frame->pos[2] = ((HitArg *) arg)->pos[2];
            w->fade = 255;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->frame->rot[1] += 0.13962634403104063;
            w->frame->scale[0] *= 1.1;
            w->frame->scale[1] *= 1.1;
            w->frame->scale[2] *= 1.1;
            w->fade -= 4;
            if (w->fade <= 0) {
                w->fade = 0;
                return -1;
            }
            if (w->frame->scale[0] > 100.0f) {
                return -1;
            }
            break;
        case 3:
            w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 0, 255, 0, 0, w->fade);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A hit mark at a point. */
void beHitmarkCreate(f32 x, f32 y, f32 z, f32 scale) {
    MarkArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = scale;
    tskCreate(gEffectTask, beHitmarkProc, 0, &arg);
}

/* "Hitmark": grows from nothing by 0.1 a frame while it fades by 8 (from 0xD0). */
s32 beHitmarkProc(Obj *obj, s16 msg, void *arg) {
    EffWork *w = EFF(obj);

    switch (msg) {
        case 2:
            obj->name = "Hitmark";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, beHitmarkDl, 0xEE);
            (*bossSetFrameFlags)(w->frame, 0x27);
            w->frame->pos[0] = ((MarkArg *) arg)->pos[0];
            w->frame->pos[1] = ((MarkArg *) arg)->pos[1];
            w->frame->pos[2] = ((MarkArg *) arg)->pos[2];
            w->frame->scale[2] = 0.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->alpha = 0xD0;
        case 0:
            w->frame->scale[2] += 0.1;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->alpha -= 8;
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            break;
        case 3:
            w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, 0, 0, 255, w->alpha);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The AT field at a point, under the battle's task. */
void beATfieldCreate(f32 x, f32 y, f32 z, f32 scale) {
    AtfArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = scale;
    if ((*tskCreate)(gEffectTask, ATfield_proc, gBattleTask->file, &arg) == NULL) {
        PRINTF("[ATfield_proc]隊長,タスクの作成に失敗しました。\n");
    }
}

/* "ATf": fades in by 10 to 0x80 (then stays at 255), and on step 1 fades out. */
s32 ATfield_proc(Obj *obj, s16 msg, void *arg) {
    EffWork *w = EFF(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "ATf";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, beATfieldDl, 0xEE);
            (*bossSetFrameFlags)(w->frame, 0x27);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                ((AtfArg *) arg)->scale;
            w->frame->pos[0] = ((AtfArg *) arg)->pos[0];
            w->frame->pos[1] = ((AtfArg *) arg)->pos[1];
            w->frame->pos[2] = ((AtfArg *) arg)->pos[2];
            w->alpha = 0;
            w->step = 0;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            switch (w->step) {
                case 0:
                    w->alpha += 10;
                    if (w->alpha > 0x80) {
                        w->alpha = 255;
                        w->step++;
                    }
                    break;
                case 1:
                    w->alpha -= 10;
                    if (w->alpha <= 0) {
                        w->alpha = 0;
                        return -1;
                    }
                    break;
            }
            break;
        case 3:
            w->frame->dl = (*bossPrimEnvColorDL)(0x80, 0x1E, 0x32, 255, 255, 255, w->alpha);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Smoke ("smoke6") of a kind: six puffs of a size in a ring dist around a point. */
void beSmoke6Create(f32 x, f32 y, f32 z, f32 size, f32 dist, s32 kind) {
    SmokeArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.size = size;
    arg.dist = dist;
    arg.kind = kind;
    tskCreate(gEffectTask, beSmoke6Proc, 0, &arg);
}

/* "smoke6": six puffs at random angles, 60 degrees apart in a ring dist around the point (flat
 * for kinds 0, 2 and 3, upright for 1), playing the smoke animation while they fade by 10;
 * they grow by 0.2 and rise by 1 a frame (kind 3 stretches up by 2 instead). Kinds 2 and 3
 * are white, 0 and 1 a warm glow. */
s32 beSmoke6Proc(Obj *obj, s16 msg, void *arg) {
    Smoke6Work *w = (Smoke6Work *) ((u32) obj + sizeof(Obj));
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "smoke6";
            w->frames[0] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frames[0] == NULL) {
                return -1;
            }
            w->frames[1] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frames[1] == NULL) {
                return -1;
            }
            w->frames[2] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frames[2] == NULL) {
                return -1;
            }
            w->frames[3] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frames[3] == NULL) {
                return -1;
            }
            w->frames[4] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frames[4] == NULL) {
                return -1;
            }
            w->frames[5] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frames[5] == NULL) {
                return -1;
            }
            bossSetPolDL(w->frames[0], beSmokeDl, 0x123);
            bossSetPolDL(w->frames[1], beSmokeDl, 0x123);
            bossSetPolDL(w->frames[2], beSmokeDl, 0x123);
            bossSetPolDL(w->frames[3], beSmokeDl, 0x123);
            bossSetPolDL(w->frames[4], beSmokeDl, 0x123);
            bossSetPolDL(w->frames[5], beSmokeDl, 0x123);
            fadeAnimStart(&w->anim, gBeSmokeAnim, 0, &w->frames[0]->segs[3],
                                   floadGetSetFile(0x123, 0));
            (*bossSetFrameFlags)(w->frames[0], 0x23);
            w->frames[0]->scale[0] = w->frames[0]->scale[1] = w->frames[0]->scale[2] = ((SmokeArg *) arg)->size;
            w->frames[0]->rot[2] = fadeRand(360) * 0.0174532930038800785;
            (*bossSetFrameFlags)(w->frames[1], 0x23);
            w->frames[1]->scale[0] = w->frames[1]->scale[1] = w->frames[1]->scale[2] = ((SmokeArg *) arg)->size;
            w->frames[1]->rot[2] = fadeRand(360) * 0.0174532930038800785;
            (*bossSetFrameFlags)(w->frames[2], 0x23);
            w->frames[2]->scale[0] = w->frames[2]->scale[1] = w->frames[2]->scale[2] = ((SmokeArg *) arg)->size;
            w->frames[2]->rot[2] = fadeRand(360) * 0.0174532930038800785;
            (*bossSetFrameFlags)(w->frames[3], 0x23);
            w->frames[3]->scale[0] = w->frames[3]->scale[1] = w->frames[3]->scale[2] = ((SmokeArg *) arg)->size;
            w->frames[3]->rot[2] = fadeRand(360) * 0.0174532930038800785;
            (*bossSetFrameFlags)(w->frames[4], 0x23);
            w->frames[4]->scale[0] = w->frames[4]->scale[1] = w->frames[4]->scale[2] = ((SmokeArg *) arg)->size;
            w->frames[4]->rot[2] = fadeRand(360) * 0.0174532930038800785;
            (*bossSetFrameFlags)(w->frames[5], 0x23);
            w->frames[5]->scale[0] = w->frames[5]->scale[1] = w->frames[5]->scale[2] = ((SmokeArg *) arg)->size;
            w->frames[5]->rot[2] = fadeRand(360) * 0.0174532930038800785;
            w->dist = ((SmokeArg *) arg)->dist;
            w->kind = ((SmokeArg *) arg)->kind;
            if (w->kind == 0 || w->kind == 2 || w->kind == 3) {
                bossHeadingOffset(0.0f, 0.0f, w->dist, &w->frames[0]->pos[0], &w->frames[0]->pos[2]);
                bossHeadingOffset(1.0471976f, 0.0f, w->dist, &w->frames[1]->pos[0], &w->frames[1]->pos[2]);
                bossHeadingOffset(2.0943952f, 0.0f, w->dist, &w->frames[2]->pos[0], &w->frames[2]->pos[2]);
                bossHeadingOffset(3.1415927f, 0.0f, w->dist, &w->frames[3]->pos[0], &w->frames[3]->pos[2]);
                bossHeadingOffset(4.1887903f, 0.0f, w->dist, &w->frames[4]->pos[0], &w->frames[4]->pos[2]);
                bossHeadingOffset(5.2359877f, 0.0f, w->dist, &w->frames[5]->pos[0], &w->frames[5]->pos[2]);
            } else if (w->kind == 1) {
                bossHeadingOffset(0.0f, 0.0f, w->dist, &w->frames[0]->pos[0], &w->frames[0]->pos[1]);
                bossHeadingOffset(1.0471976f, 0.0f, w->dist, &w->frames[1]->pos[0], &w->frames[1]->pos[1]);
                bossHeadingOffset(2.0943952f, 0.0f, w->dist, &w->frames[2]->pos[0], &w->frames[2]->pos[1]);
                bossHeadingOffset(3.1415927f, 0.0f, w->dist, &w->frames[3]->pos[0], &w->frames[3]->pos[1]);
                bossHeadingOffset(4.1887903f, 0.0f, w->dist, &w->frames[4]->pos[0], &w->frames[4]->pos[1]);
                bossHeadingOffset(5.2359877f, 0.0f, w->dist, &w->frames[5]->pos[0], &w->frames[5]->pos[1]);
            }
            w->pos[0] = ((SmokeArg *) arg)->pos[0];
            w->pos[1] = ((SmokeArg *) arg)->pos[1];
            w->pos[2] = ((SmokeArg *) arg)->pos[2];
            w->frames[0]->pos[0] += w->pos[0];
            w->frames[0]->pos[1] += w->pos[1];
            w->frames[0]->pos[2] += w->pos[2];
            w->frames[1]->pos[0] += w->pos[0];
            w->frames[1]->pos[1] += w->pos[1];
            w->frames[1]->pos[2] += w->pos[2];
            w->frames[2]->pos[0] += w->pos[0];
            w->frames[2]->pos[1] += w->pos[1];
            w->frames[2]->pos[2] += w->pos[2];
            w->frames[3]->pos[0] += w->pos[0];
            w->frames[3]->pos[1] += w->pos[1];
            w->frames[3]->pos[2] += w->pos[2];
            w->frames[4]->pos[0] += w->pos[0];
            w->frames[4]->pos[1] += w->pos[1];
            w->frames[4]->pos[2] += w->pos[2];
            w->frames[5]->pos[0] += w->pos[0];
            w->frames[5]->pos[1] += w->pos[1];
            w->frames[5]->pos[2] += w->pos[2];
            w->alpha = 255;
            break;
        case 0:
            if (fadeAnimStep(&w->anim) != 0) {
                return -1;
            }
            w->alpha -= 10;
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            if (w->kind == 3) {
                w->frames[0]->scale[1] += 2.0;
                w->frames[1]->scale[1] += 2.0;
                w->frames[2]->scale[1] += 2.0;
                w->frames[3]->scale[1] += 2.0;
                w->frames[4]->scale[1] += 2.0;
                w->frames[5]->scale[1] += 2.0;
            } else {
                w->frames[0]->scale[2] += 0.2;
                w->frames[0]->scale[0] = w->frames[0]->scale[1] = w->frames[0]->scale[2];
                w->frames[1]->scale[2] += 0.2;
                w->frames[1]->scale[0] = w->frames[1]->scale[1] = w->frames[1]->scale[2];
                w->frames[2]->scale[2] += 0.2;
                w->frames[2]->scale[0] = w->frames[2]->scale[1] = w->frames[2]->scale[2];
                w->frames[3]->scale[2] += 0.2;
                w->frames[3]->scale[0] = w->frames[3]->scale[1] = w->frames[3]->scale[2];
                w->frames[4]->scale[2] += 0.2;
                w->frames[4]->scale[0] = w->frames[4]->scale[1] = w->frames[4]->scale[2];
                w->frames[5]->scale[2] += 0.2;
                w->frames[5]->scale[0] = w->frames[5]->scale[1] = w->frames[5]->scale[2];
                w->frames[0]->pos[1] += 1.0f;
                w->frames[1]->pos[1] += 1.0f;
                w->frames[2]->pos[1] += 1.0f;
                w->frames[3]->pos[1] += 1.0f;
                w->frames[4]->pos[1] += 1.0f;
                w->frames[5]->pos[1] += 1.0f;
            }
            break;
        case 1:
            frmDestroy(&w->frames[0]->f);
            frmDestroy(&w->frames[1]->f);
            frmDestroy(&w->frames[2]->f);
            frmDestroy(&w->frames[3]->f);
            frmDestroy(&w->frames[4]->f);
            frmDestroy(&w->frames[5]->f);
            break;
        case 3:
            if (w->kind == 2 || w->kind == 3) {
                w->frames[0]->dl = (*bossPrimEnvColorDL)(255, 255, 255, 255, 255, 255, w->alpha);
                w->frames[1]->dl = (*bossPrimEnvColorDL)(255, 255, 255, 255, 255, 255, w->alpha);
                w->frames[2]->dl = (*bossPrimEnvColorDL)(255, 255, 255, 255, 255, 255, w->alpha);
                w->frames[3]->dl = (*bossPrimEnvColorDL)(255, 255, 255, 255, 255, 255, w->alpha);
                w->frames[4]->dl = (*bossPrimEnvColorDL)(255, 255, 255, 255, 255, 255, w->alpha);
                w->frames[5]->dl = (*bossPrimEnvColorDL)(255, 255, 255, 255, 255, 255, w->alpha);
            } else if (w->kind == 0 || w->kind == 1) {
                w->frames[0]->dl = (*bossPrimEnvColorDL)(255, 200, 160, 255, 255, 180, w->alpha);
                w->frames[1]->dl = (*bossPrimEnvColorDL)(255, 200, 160, 255, 255, 180, w->alpha);
                w->frames[2]->dl = (*bossPrimEnvColorDL)(255, 200, 160, 255, 255, 180, w->alpha);
                w->frames[3]->dl = (*bossPrimEnvColorDL)(255, 200, 160, 255, 255, 180, w->alpha);
                w->frames[4]->dl = (*bossPrimEnvColorDL)(255, 200, 160, 255, 255, 180, w->alpha);
                w->frames[5]->dl = (*bossPrimEnvColorDL)(255, 200, 160, 255, 255, 180, w->alpha);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* An "Neffect" at a point. */
void beNeffectCreate(f32 x, f32 y, f32 z, f32 scale) {
    HitArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = scale;
    tskCreate(gEffectTask, beNeffectProc, 0, &arg);
}

/* "Neffect": a twenty-fifth of `scale`, growing slowly while it fades by 4 (from 0xC0). */
s32 beNeffectProc(Obj *obj, s16 msg, void *arg) {
    EffWork *w = EFF(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Neffect";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, beMeffectDl, 0xEE);
            (*bossSetFrameFlags)(w->frame, 0x22);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = ((HitArg *) arg)->scale / 25.0f;
            w->frame->pos[0] = ((HitArg *) arg)->pos[0];
            w->frame->pos[1] = ((HitArg *) arg)->pos[1];
            w->frame->pos[2] = ((HitArg *) arg)->pos[2];
            w->fade = 0xC0;
            break;
        case 0:
            w->frame->scale[2] += 0.0005;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->fade -= 4;
            if (w->fade <= 0) {
                w->fade = 0;
                return -1;
            }
            break;
        case 3:
            w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, 0, 0, 255, w->fade);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* An "Meffect" at a point. */
void beMeffectCreate(f32 x, f32 y, f32 z) {
    HitArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    tskCreate(gEffectTask, beMeffectProc, 0, &arg);
}

/* "Meffect": laid flat, growing by 0.1 and rising by 10 a frame while it fades by 3. */
s32 beMeffectProc(Obj *obj, s16 msg, void *arg) {
    EffWork *w = EFF(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Meffect";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, beMeffectDl, 0xEE);
            (*bossSetFrameFlags)(w->frame, 0x20);
            w->frame->scale[2] = 0.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = ((HitArg *) arg)->pos[0];
            w->frame->pos[1] = ((HitArg *) arg)->pos[1];
            w->frame->pos[2] = ((HitArg *) arg)->pos[2];
            w->frame->rot[0] = -1.5707964f;
            w->fade = 0xE0;
            break;
        case 0:
            w->fade -= 3;
            if (w->fade <= 0) {
                w->fade = 0;
                return -1;
            }
            w->frame->scale[0] += 0.1;
            w->frame->scale[1] += 0.1;
            w->frame->scale[2] += 0.1;
            w->frame->pos[1] += 10.0f;
            break;
        case 3:
            w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 0xC0, 0x80, 0x80, 0x60, w->fade);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* An "Meffect2" at a point. */
void beMeffect2Create(f32 x, f32 y, f32 z) {
    HitArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    tskCreate(gEffectTask, beMeffect2Proc, 0, &arg);
}

/* "Meffect2": shrinks by 0.01 a frame (from 0.2) while it fades by 6 (from 0x80). */
s32 beMeffect2Proc(Obj *obj, s16 msg, void *arg) {
    EffWork *w = EFF(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Meffect2";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, beMeffectDl, 0xEE);
            (*bossSetFrameFlags)(w->frame, 0x22);
            w->frame->scale[2] = 0.2f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = ((HitArg *) arg)->pos[0];
            w->frame->pos[1] = ((HitArg *) arg)->pos[1];
            w->frame->pos[2] = ((HitArg *) arg)->pos[2];
            w->fade = 0x80;
            break;
        case 0:
            w->fade -= 6;
            if (w->fade <= 0) {
                w->fade = 0;
                return -1;
            }
            w->frame->scale[0] -= 0.01;
            w->frame->scale[1] -= 0.01;
            w->frame->scale[2] -= 0.01;
            if (w->frame->scale[0] <= 0.0f) {
                w->frame->scale[0] = 0.0f;
                w->frame->scale[1] = 0.0f;
                w->frame->scale[2] = 0.0f;
                return -1;
            }
            break;
        case 3:
            w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, 0, 0, 255, w->fade);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* An "M3effect" at a point. */
void beM3effectCreate(f32 x, f32 y, f32 z) {
    HitArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    tskCreate(gEffectTask, beM3effectProc, 0, &arg);
}

/* "M3effect": grows by 0.1 a frame while it fades by 5 (from 0x80). */
s32 beM3effectProc(Obj *obj, s16 msg, void *arg) {
    EffWork *w = EFF(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "M3effect";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, beMeffectDl, 0xEE);
            (*bossSetFrameFlags)(w->frame, 0x22);
            w->frame->scale[2] = 0.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = ((HitArg *) arg)->pos[0];
            w->frame->pos[1] = ((HitArg *) arg)->pos[1];
            w->frame->pos[2] = ((HitArg *) arg)->pos[2];
            w->fade = 0x80;
            break;
        case 0:
            w->fade -= 5;
            if (w->fade <= 0) {
                w->fade = 0;
                return -1;
            }
            w->frame->scale[0] += 0.1;
            w->frame->scale[1] += 0.1;
            w->frame->scale[2] += 0.1;
            break;
        case 3:
            w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, 0, 0, 255, w->fade);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The second "M3effect" at a point, turned. */
void beM3effectTintCreate(f32 x, f32 y, f32 z, f32 rot) {
    HitArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.rot = rot;
    tskCreate(gEffectTask, beM3effectTintProc, 0, &arg);
}

/* The second "M3effect": grows by 7% a frame while it fades by 6, red- or blue-tinged by which
 * pilot (flag 0x4000000 of gBattleFlags). */
s32 beM3effectTintProc(Obj *obj, s16 msg, void *arg) {
    EffWork *w = EFF(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "M3effect";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, beHitmarkDl, 0xEE);
            (*bossSetFrameFlags)(w->frame, 0x20);
            w->frame->scale[2] = 1.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = ((HitArg *) arg)->pos[0];
            w->frame->pos[1] = ((HitArg *) arg)->pos[1];
            w->frame->pos[2] = ((HitArg *) arg)->pos[2];
            w->frame->rot[1] = ((HitArg *) arg)->rot;
            w->fade = 255;
            break;
        case 0:
            w->fade -= 6;
            if (w->fade <= 0) {
                w->fade = 0;
                return -1;
            }
            w->frame->scale[0] *= 1.07;
            w->frame->scale[1] *= 1.07;
            w->frame->scale[2] *= 1.07;
            break;
        case 3:
            if (gBattleFlags & 0x4000000) {
                w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, 255, 0x40, 0x40, w->fade);
            } else {
                w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, 0x40, 0x40, 255, w->fade);
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "flashF": a full-screen flash of a colour, fading over a time; hidden while flag
 * 0x1000000 of gBattleFlags is on. */
s32 beFlashFProc(Obj *obj, s16 msg, void *arg) {
    FlashWork *w = (FlashWork *) ((u32) obj + sizeof(Obj));
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "flashF";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, beFilterDl, 0xEA);
            (*bossSetFrameFlags)(w->frame, 0x20);
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[2] = 0.0f;
            w->frame->pos[0] = w->frame->pos[1] = w->frame->pos[2];
            w->rgb[0] = ((FlashArg *) arg)->rgb[0];
            w->rgb[1] = ((FlashArg *) arg)->rgb[1];
            w->rgb[2] = ((FlashArg *) arg)->rgb[2];
            w->time = ((FlashArg *) arg)->time;
            w->alpha = 255;
        case 0:
            w->frame->rot[0] = gCockpitRotX;
            w->frame->rot[1] = gCockpitRotY;
            w->frame->rot[2] = gCockpitRotZ;
            w->alpha -= (s32) (255.0f / w->time);
            if (w->alpha < 0) {
                return -1;
            }
            break;
        case 3:
            if (gBattleFlags & 0x1000000) {
                w->frame->f.flags &= ~2;
            } else {
                w->frame->f.flags |= 2;
            }
            w->frame->dl = bossXluPrimEnvColorDL(w->rgb[0], w->rgb[1], w->rgb[2], w->alpha);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A flash of a colour over `time` frames. */
void beFlashFCreate(s32 r, s32 g, s32 b, s32 time) {
    FlashArg arg;

    arg.rgb[0] = r;
    arg.rgb[1] = g;
    arg.rgb[2] = b;
    arg.time = time;
    tskCreate(gEffectTask, beFlashFProc, 0, &arg);
}

/* A mega ball at a point, of a size, for a time. */
void beMegaballCreate(f32 x, f32 y, f32 z, f32 scale, s32 time) {
    MegaArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = scale;
    arg.time = time;
    tskCreate(gEffectTask, beMegaballProc, 0, &arg);
}

/* "Megaball": a red ball twice `scale`, spinning and shrinking by 0.2 a frame; after its
 * time, it fades by 5 (from 0x80). */
s32 beMegaballProc(Obj *obj, s16 msg, void *arg) {
    MegaWork *w = (MegaWork *) ((u32) obj + sizeof(Obj));
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Megaball";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, beBallDl, 0x120);
            (*bossSetFrameFlags)(w->frame, 0x20);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = ((MegaArg *) arg)->scale * 2;
            w->frame->pos[0] = ((MegaArg *) arg)->pos[0];
            w->frame->pos[1] = ((MegaArg *) arg)->pos[1];
            w->frame->pos[2] = ((MegaArg *) arg)->pos[2];
            w->time = ((MegaArg *) arg)->time;
            w->alpha = 0x80;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->frame->rot[1] += 0.13962634403104063;
            w->time -= 1.0f;
            if (w->time < 0.0f) {
                w->time = 0.0f;
            }
            if (w->time == 0.0f) {
                w->alpha -= 5;
                if (w->alpha < 0) {
                    return -1;
                }
            }
            w->frame->scale[0] -= 0.2;
            w->frame->scale[1] -= 0.2;
            w->frame->scale[2] -= 0.2;
            break;
        case 3:
            w->frame->dl = bossXluEnvColorDL(255, 0, 0, 0x80);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Belt": the demo letterbox, for a time: a black bar from the scissor's top left to `top`, one
 * from `bottom` to the scissor's bottom right, and a one-pixel black edge along each. The
 * corners and the scissor are read as named fields: as array elements IDO swapped every
 * fill rectangle's OR operands. */
s32 beBeltProc(Obj *obj, s16 msg, void *arg) {
    BeltWork *w = (BeltWork *) ((u32) obj + sizeof(Obj));
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Belt";
            w->frame = (DLFrame *) frmCreate(gFramePool10, gFrmKindDlist);
            if (w->frame == NULL) {
                return -1;
            }
            w->topX = ((BeltArg *) arg)->top[0];
            w->topY = ((BeltArg *) arg)->top[1];
            w->bottomX = ((BeltArg *) arg)->bottom[0];
            w->bottomY = ((BeltArg *) arg)->bottom[1];
            w->time = ((BeltArg *) arg)->time;
            DPRINTF("デモ用黒帯発生しました[%d]\n", w->time);
            break;
        case 0:
            if (w->time-- == 0) {
                DPRINTF("デモ用黒帯消滅しました\n");
                return -1;
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCycleType(gGfxDlHead++, G_CYC_FILL);
            gDPSetRenderMode(gGfxDlHead++, 0, 0);
            gDPSetFillColor(gGfxDlHead++, 0);
            gDPFillRectangle(gGfxDlHead++, gGameState.scissor.ulx, gGameState.scissor.uly, w->topX, w->topY);
            gDPFillRectangle(gGfxDlHead++, w->bottomX, w->bottomY, gGameState.scissor.lrx, gGameState.scissor.lry);
            gDPPipeSync(gGfxDlHead++);
            gDPSetCycleType(gGfxDlHead++, G_CYC_1CYCLE);
            gDPSetAlphaCompare(gGfxDlHead++, G_AC_NONE);
            gDPSetRenderMode(gGfxDlHead++, 0x504240, 0);
            gDPSetCombine(gGfxDlHead++, 0xFFFFFF, 0xFFFEFB7D);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 0);
            gDPFillRectangle(gGfxDlHead++, gGameState.scissor.ulx, w->topY + 1, gGameState.scissor.lrx, w->topY + 2);
            gDPFillRectangle(gGfxDlHead++, gGameState.scissor.ulx, w->bottomY - 1, gGameState.scissor.lrx, w->bottomY);
            gDPFillRectangle(gGfxDlHead++, gGameState.scissor.ulx, gGameState.scissor.uly, gGameState.scissor.ulx + 1,
                             gGameState.scissor.lry);
            gDPFillRectangle(gGfxDlHead++, gGameState.scissor.lrx - 1, gGameState.scissor.uly, gGameState.scissor.lrx,
                             gGameState.scissor.lry);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The demo letterbox for a time. */
void beBeltCreate(s32 x1, s32 y1, s32 x2, s32 y2, s32 time) {
    BeltArg arg;

    arg.top[0] = x1;
    arg.top[1] = y1;
    arg.bottom[0] = x2;
    arg.bottom[1] = y2;
    arg.time = time;
    if (tskCreate(gEffectTask, beBeltProc, 0, &arg) == NULL) {
        PRINTF("隊長,タスクの作成に失敗しました。\n");
    }
}

/* A "flashP" at a point, held for count frames. */
void beFlashPCreate(f32 x, f32 y, f32 z, s32 count) {
    FlashPArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.count = count;
    arg.pos[2] = z;
    (*tskCreate)(gEffectTask, beFlashPProc, gBattleTask->file, &arg);
}

/* "flashP": an animated flash four times its size at a random angle: plays its first animation
 * once, then loops a second for count frames, then fades by 10. */
s32 beFlashPProc(Obj *obj, s16 msg, void *arg) {
    FlashPWork *w = (FlashPWork *) ((u32) obj + sizeof(Obj));

    switch (msg) {
        case 2:
            obj->name = "flashP";
            w->frame = (AnimeFrame *) (*frmCreate)(gFrmLayer8, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            (*bossSetAnimAt)(w->frame, beFlashPAnim, 0xEE, 0.0f);
            (*bossSetFrameFlags2)(w->frame, 0x20);
            w->frame->scale[2] = 4.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = ((FlashPArg *) arg)->pos[0];
            w->frame->pos[1] = ((FlashPArg *) arg)->pos[1];
            w->frame->pos[2] = ((FlashPArg *) arg)->pos[2];
            w->frame->rot[1] = fadeRand(360) * 0.0174532930038800785;
            w->count = ((FlashPArg *) arg)->count;
            w->step = 0;
            w->alpha = 255;
            break;
        case 0:
            switch (w->step) {
                case 0:
                    if ((*bossStepAnim)(w->frame, 0.3f, 2) != 0) {
                        (*bossSetAnimAt)(w->frame, beFlashPLoopAnim, 0xEE, 0.0f);
                        w->step++;
                    }
                    break;
                case 1:
                    (*bossStepAnim)(w->frame, 0.1f, 0);
                    if (w->count-- == 0) {
                        w->step++;
                    }
                    break;
                case 2:
                    (*bossStepAnim)(w->frame, 0.1f, 0);
                    w->alpha -= 10;
                    if (w->alpha <= 0) {
                        w->alpha = 0;
                        return -1;
                    }
                    break;
            }
            break;
        case 3:
            w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, 255, 255, 0, w->alpha);
            break;
        case 1:
            (*frmDestroy)(&w->frame->f);
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* An "M5effect" at a point. */
void beM5effectCreate(f32 x, f32 y, f32 z) {
    HitArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    tskCreate(gEffectTask, beM5effectProc, 0, &arg);
}

/* "M5effect": shrinks by 0.01 a frame (from 0.1), black at a fade of 0xA0. */
s32 beM5effectProc(Obj *obj, s16 msg, void *arg) {
    EffWork *w = EFF(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "M5effect";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, beMeffectDl, 0xEE);
            (*bossSetFrameFlags)(w->frame, 0x22);
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = ((HitArg *) arg)->pos[0];
            w->frame->pos[1] = ((HitArg *) arg)->pos[1];
            w->frame->pos[2] = ((HitArg *) arg)->pos[2];
            w->fade = 0xA0;
            break;
        case 0:
            w->frame->scale[0] -= 0.01;
            w->frame->scale[1] -= 0.01;
            w->frame->scale[2] -= 0.01;
            if (w->frame->scale[0] < 0.0f) {
                w->frame->scale[2] = 0.0f;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                return -1;
            }
            break;
        case 3:
            w->frame->dl = (*bossPrimEnvColorDL)(0, 0, 0, 0, 0, 0, w->fade);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* An "M6effect" at a point: a spark that starts dist away and gathers in. */
void beM6effectCreate(f32 x, f32 y, f32 z, f32 scale, f32 dist) {
    SparkArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = scale;
    arg.dist = dist;
    tskCreate(gEffectTask, beM6effectProc, 0, &arg);
}

/* "M6effect": a spark of a random size (0.3 to 1.7 times `scale`) at a random point dist from
 * the spawn point, flying back to it at 2 and fading by 3 (by 5 once there); it flickers
 * white every fifth frame and is red- or blue-tinged by which pilot. */
s32 beM6effectProc(Obj *obj, s16 msg, void *arg) {
    SparkWork *w = SPARK(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "M6effect";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, beFlashDl, 0x120);
            (*bossSetFrameFlags)(w->frame, 0x22);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                fadeRandRange(3, 17) / 10.0f * ((SparkArg *) arg)->scale;
            (*bossHeadingOffset)(fadeRand(360) * 0.0174532930038800785, 0.0f,
                                      ((SparkArg *) arg)->dist, &w->frame->pos[0], &w->frame->pos[1]);
            (*bossHeadingOffset)(fadeRand(360) * 0.0174532930038800785, 0.0f,
                                      ((SparkArg *) arg)->dist, &w->frame->pos[0], &w->frame->pos[2]);
            w->frame->pos[0] += ((SparkArg *) arg)->pos[0];
            w->frame->pos[1] += ((SparkArg *) arg)->pos[1];
            w->frame->pos[2] += ((SparkArg *) arg)->pos[2];
            w->fade = 0xC0;
            w->target[0] = ((SparkArg *) arg)->pos[0];
            w->target[1] = ((SparkArg *) arg)->pos[1];
            w->target[2] = ((SparkArg *) arg)->pos[2];
            break;
        case 0:
            bossApplyVel(obj);
            if (BmoveX(obj, w->target[0], w->target[1], w->target[2], 2.0f) == 0) {
                w->fade -= 2;
            }
            w->fade -= 3;
            if (w->fade <= 0) {
                return -1;
            }
            break;
        case 3:
            if (gBattleFlags & 0x4000000) {
                if (gGameFrameCount % 5 == 0) {
                    w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, 255, 255, 255, w->fade);
                } else {
                    w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, 255, 0, 0, w->fade);
                }
            } else if (gGameFrameCount % 5 == 0) {
                w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, 255, 255, 255, w->fade);
            } else {
                w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, 0, 0, 255, w->fade);
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* An "M7effect" at a point. */
void beM7effectCreate(f32 x, f32 y, f32 z, f32 scale, f32 dist) {
    SparkArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = scale;
    arg.dist = dist;
    tskCreate(gEffectTask, beM7effectProc, 0, &arg);
}

/* "M7effect": a spark of a random size flying from the point toward a random one within 300,
 * at 20, fading by 3; grey-orange while flag 0x40000 of gBattleFlags2 is on. */
s32 beM7effectProc(Obj *obj, s16 msg, void *arg) {
    SparkWork *w = SPARK(obj);
    s32 pad;
    f32 x;
    f32 y;
    f32 z;
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "M7effect";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, beFlashDl, 0x120);
            (*bossSetFrameFlags)(w->frame, 0x22);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                fadeRandRange(3, 17) / 10.0f * ((SparkArg *) arg)->scale;
            x = ((SparkArg *) arg)->pos[0] + fadeRandRange(-300, 300);
            y = ((SparkArg *) arg)->pos[1] + fadeRandRange(-300, 300);
            z = ((SparkArg *) arg)->pos[2] + fadeRandRange(-300, 300);
            w->frame->pos[0] = ((SparkArg *) arg)->pos[0];
            w->frame->pos[1] = ((SparkArg *) arg)->pos[1];
            w->frame->pos[2] = ((SparkArg *) arg)->pos[2];
            BmoveX(obj, x, y, z, 20.0f);
            w->fade = 0xC0;
            break;
        case 0:
            bossApplyVel(obj);
            w->fade -= 3;
            if (w->fade <= 0) {
                return -1;
            }
            break;
        case 3:
            if (gBattleFlags & 0x4000000) {
                w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, 255, 0, 0, w->fade);
            } else {
                w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, 0, 0, 255, w->fade);
            }
            if (gBattleFlags2 & 0x40000) {
                w->frame->dl = (*bossPrimEnvColorDL)(0x80, 0x80, 0x80, 255, 0x80, 0, w->fade);
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "PunchE" at a point. */
void bePunchECreate(f32 x, f32 y, f32 z, f32 scale, f32 dist) {
    SparkArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = scale;
    arg.dist = dist;
    tskCreate(gEffectTask, bePunchEProc, 0, &arg);
}

/* "PunchE": a punch's spark of a random size, flying from the point at 5 toward a point dist
 * away at a random angle (in x and y around the origin, at the point's z), fading by 3. */
s32 bePunchEProc(Obj *obj, s16 msg, void *arg) {
    SparkWork *w = SPARK(obj);
    s32 pad;
    f32 x = 0.0f;
    f32 y = 0.0f;
    f32 z;
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "PunchE";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, beFlashDl, 0x120);
            (*bossSetFrameFlags)(w->frame, 0x22);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                fadeRandRange(3, 17) / 10.0f * ((SparkArg *) arg)->scale;
            (*bossHeadingOffset)(fadeRand(360) * 0.0174532930038800785, 0.0f,
                                      ((SparkArg *) arg)->dist, &x, &y);
            z = ((SparkArg *) arg)->pos[2];
            w->frame->pos[0] = ((SparkArg *) arg)->pos[0];
            w->frame->pos[1] = ((SparkArg *) arg)->pos[1];
            w->frame->pos[2] = ((SparkArg *) arg)->pos[2];
            BmoveX(obj, x, y, z, 5.0f);
            w->fade = 0xC0;
            break;
        case 0:
            bossApplyVel(obj);
            w->fade -= 3;
            if (w->fade <= 0) {
                return -1;
            }
            break;
        case 3:
            if (gBattleFlags & 0x4000000) {
                w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, 255, 0, 0, w->fade);
            } else {
                w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, 0, 0, 255, w->fade);
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "fade": the opening letterbox, drawn as "Belt" is.
 * The empty test of gBeFadeShorten just before the keep test stands for a test whose body
 * compiled to nothing: it starts the address's live range there and emits no code. */
s32 beFadeProc(Obj *obj, s16 msg, void *arg) {
    FadeWork *w = (FadeWork *) ((u32) obj + sizeof(Obj));
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "fade";
            w->frame = (DLFrame *) (*frmCreate)(gFramePool10, gFrmKindDlist);
            if (w->frame == NULL) {
                return -1;
            }
            w->top[0] = gGameState.scissor.lrx;
            w->top[1] = ((FadeWork *) arg)->top[1];
            w->bottom[0] = gGameState.scissor.ulx;
            w->bottom[1] = ((FadeWork *) arg)->bottom[1];
            w->time = ((FadeWork *) arg)->time;
            w->keep = ((FadeWork *) arg)->keep;
            w->curTop[0] = w->top[0];
            w->curBottom[0] = w->bottom[0];
            if (w->keep == 0) {
                w->curTop[1] = w->curBottom[1] = (w->bottom[1] + w->top[1]) / 2;
            }
            DPRINTF("デモ用黒帯発生しました[%d]\n", w->time);
            break;
        case 0:
            if (w->time-- == 0) {
                DPRINTF("デモ用黒帯消滅しました\n");
                return -1;
            }
            if (gBeFadeShorten) {} /* FAKE: starts &gBeFadeShorten's live range above the keep test */
            if (w->keep == 0) {
                if ((w->curTop[1] - gGameState.scissor.uly) * 2 < w->time) {
                    if (!(gGameState.frameCount & 1)) {
                        if (w->top[1] + 1 < w->curTop[1]) {
                            w->curTop[1]--;
                        }
                        if (w->curBottom[1] < w->bottom[1] - 1) {
                            w->curBottom[1]++;
                        }
                    }
                    if (gBeFadeShorten == 1) {
                        gBeFadeShorten = 0;
                        w->time = (w->curTop[1] - gGameState.scissor.uly) * 2;
                    }
                } else if (!(gGameState.frameCount & 1)) {
                    if (gGameState.scissor.uly < w->curTop[1]) {
                        w->curTop[1]--;
                    }
                    if (w->curBottom[1] < gGameState.scissor.lry) {
                        w->curBottom[1]++;
                    }
                }
            }
            break;
        case 1:
            (*frmDestroy)(&w->frame->f);
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCycleType(gGfxDlHead++, G_CYC_FILL);
            gDPSetRenderMode(gGfxDlHead++, 0, 0);
            gDPSetFillColor(gGfxDlHead++, 0);
            gDPFillRectangle(gGfxDlHead++, gGameState.scissor.ulx, gGameState.scissor.uly, FADE_XY(w->top)->x,
                             FADE_XY(w->curTop)->y);
            gDPFillRectangle(gGfxDlHead++, FADE_XY(w->bottom)->x, FADE_XY(w->curBottom)->y,
                             gGameState.scissor.lrx, gGameState.scissor.lry);
            gDPPipeSync(gGfxDlHead++);
            gDPSetCycleType(gGfxDlHead++, G_CYC_1CYCLE);
            gDPSetAlphaCompare(gGfxDlHead++, G_AC_NONE);
            gDPSetRenderMode(gGfxDlHead++, 0x504240, 0);
            gDPSetCombine(gGfxDlHead++, 0xFFFFFF, 0xFFFEFB7D);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 0);
            gDPFillRectangle(gGfxDlHead++, gGameState.scissor.ulx, FADE_XY(w->curTop)->y + 1,
                             gGameState.scissor.lrx, FADE_XY(w->curTop)->y + 2);
            gDPFillRectangle(gGfxDlHead++, gGameState.scissor.ulx, FADE_XY(w->curBottom)->y - 1,
                             gGameState.scissor.lrx, FADE_XY(w->curBottom)->y);
            gDPFillRectangle(gGfxDlHead++, gGameState.scissor.ulx, gGameState.scissor.uly,
                             gGameState.scissor.ulx + 1, gGameState.scissor.lry);
            gDPFillRectangle(gGfxDlHead++, gGameState.scissor.lrx - 1, gGameState.scissor.uly,
                             gGameState.scissor.lrx, gGameState.scissor.lry);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* The opening letterbox, from bars at y1 and y2, for a time. */
void beFadeCreate(s32 y1, s32 y2, s32 time, s32 keep) {
    FadeWork arg;

    arg.top[1] = y1;
    arg.bottom[1] = y2;
    arg.time = time;
    arg.keep = keep;
    gBeFadeShorten = 0;
    (*tskCreate)(gEffectTask, beFadeProc, 0, &arg);
}

/* A "Star" at a point. */
void beStarCreate(f32 x, f32 y, f32 z) {
    HitArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    tskCreate(gEffectTask, beStarProc, 0, &arg);
}

/* "Star": a star of a random size at a random angle, growing by 0.1 a frame (to 5) while it
 * fades by 5 (from 0xA0), its edges a random colour each frame. */
s32 beStarProc(Obj *obj, s16 msg, void *arg) {
    EffWork *w = EFF(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Star";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            (*bossSetPolDL)(w->frame, beStarDl, 0x124);
            (*bossSetFrameFlags)(w->frame, 0x22);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                fadeRandRange(3, 17) / 10.0f * 2.5;
            w->frame->pos[0] = ((HitArg *) arg)->pos[0];
            w->frame->pos[1] = ((HitArg *) arg)->pos[1];
            w->frame->pos[2] = ((HitArg *) arg)->pos[2];
            w->frame->rot[2] = fadeRand(360) * 0.0174532930038800785;
            w->fade = 0xA0;
            break;
        case 0:
            w->frame->scale[0] += 0.1;
            w->frame->scale[1] += 0.1;
            w->frame->scale[2] += 0.1;
            w->fade -= 5;
            if (w->fade < 0) {
                return -1;
            }
            if (w->frame->scale[0] > 5.0f) {
                w->frame->scale[2] = 5.0f;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                return -1;
            }
            break;
        case 3:
            w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, (*fadeRand)(255),
                                                     (*fadeRand)(255),
                                                     (*fadeRand)(255), w->fade);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The second "Star" at a point. */
void beRedStarCreate(f32 x, f32 y, f32 z) {
    HitArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    tskCreate(gEffectTask, beRedStarProc, 0, &arg);
}

/* The second "Star": a red spinning star that squashes flat, stretches up to 12, then fades
 * by 4 (from 0xA0) as it keeps stretching. */
s32 beRedStarProc(Obj *obj, s16 msg, void *arg) {
    EffWork *w = EFF(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Star";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            (*bossSetPolDL)(w->frame, beRedStarDl, 0xEE);
            (*bossSetFrameFlags)(w->frame, 0x20);
            w->frame->scale[0] = 4.0f;
            w->frame->scale[1] = 8.0f;
            w->frame->scale[2] = 4.0f;
            w->frame->pos[0] = ((HitArg *) arg)->pos[0];
            w->frame->pos[1] = ((HitArg *) arg)->pos[1];
            w->frame->pos[2] = ((HitArg *) arg)->pos[2];
            w->fade = 0xA0;
            w->stage = 0;
            break;
        case 0:
            switch (w->stage) {
                case 0:
                    w->frame->rot[1] += 0.13962634403104063;
                    w->frame->scale[0] += 0.4;
                    w->frame->scale[1] -= 0.4;
                    w->frame->scale[2] += 0.4;
                    if (w->frame->scale[1] < 0.1) {
                        w->stage++;
                    }
                    break;
                case 1:
                    w->frame->rot[1] += 0.13962634403104063;
                    w->frame->scale[0] += 0.2;
                    w->frame->scale[1] += 0.8;
                    w->frame->scale[2] += 0.2;
                    if (w->frame->scale[0] > 12.0) {
                        w->stage++;
                    }
                    break;
                case 2:
                    w->frame->rot[1] += 0.13962634403104063;
                    w->frame->scale[0] += 0.2;
                    w->frame->scale[1] += 0.8;
                    w->frame->scale[2] += 0.2;
                    w->fade -= 4;
                    if (w->fade <= 0) {
                        w->fade = 0;
                        return -1;
                    }
                    break;
            }
            break;
        case 3:
            w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 0, 255, 0, 0, w->fade);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* An "effectP" of a kind at a point, of a size. */
void beEffectPCreate(f32 x, f32 y, f32 z, f32 scale, s32 kind) {
    EffPArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = scale;
    arg.kind = kind;
    tskCreate(gEffectTask, beEffectPProc, 0, &arg);
}

/* "effectP": an animation (by kind) at a random angle and size, rising slowly and growing by
 * 1% a frame while it fades by 10; it ends with the animation. Kinds 0 and 1 are red, 2 blue,
 * 3 black (at half strength, rising faster). */
s32 beEffectPProc(Obj *obj, s16 msg, void *arg) {
    EffPWork *w = (EffPWork *) ((u32) obj + sizeof(Obj));
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "effectP";
            w->frame = (PolFrame *) frmCreate(gCockpitPanelFrame, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, beSmokeDl, 0x123);
            if (((EffPArg *) arg)->kind == 0) {
                fadeAnimStart(&w->anim, gBeEffectPAnim0, 0, &w->frame->segs[3],
                                       floadGetSetFile(0x11F, 0));
            }
            if (((EffPArg *) arg)->kind == 1) {
                fadeAnimStart(&w->anim, gBeEffectPAnim12, 0, &w->frame->segs[3],
                                       floadGetSetFile(0x121, 0));
            }
            if (((EffPArg *) arg)->kind == 2) {
                fadeAnimStart(&w->anim, gBeEffectPAnim12, 0, &w->frame->segs[3],
                                       floadGetSetFile(0x121, 0));
            }
            if (((EffPArg *) arg)->kind == 3) {
                fadeAnimStart(&w->anim, gBeSmokeAnim, 0, &w->frame->segs[3],
                                       floadGetSetFile(0x123, 0));
            }
            (*bossSetFrameFlags)(w->frame, 0x23);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                fadeRandRange(3, 17) / 10.0f * ((EffPArg *) arg)->scale;
            w->frame->rot[2] = fadeRand(360) * 0.0174532930038800785;
            w->kind = ((EffPArg *) arg)->kind;
            w->pos[0] = ((EffPArg *) arg)->pos[0];
            w->pos[1] = ((EffPArg *) arg)->pos[1];
            w->pos[2] = ((EffPArg *) arg)->pos[2];
            w->frame->pos[0] += w->pos[0];
            w->frame->pos[1] += w->pos[1];
            w->frame->pos[2] += w->pos[2];
            w->alpha = 255;
            break;
        case 0:
            if (fadeAnimStep(&w->anim) != 0) {
                return -1;
            }
            w->alpha -= 10;
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            w->frame->scale[2] *= 1.01;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[1] += 0.01;
            if (w->kind == 3) {
                w->frame->pos[1] += 1.0f;
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            if (w->kind == 0) {
                w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 0, 255, 0, 0, w->alpha);
            }
            if (w->kind == 1) {
                w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 0, 255, 0, 0, w->alpha);
            }
            if (w->kind == 2) {
                w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, 0, 0, 255, w->alpha);
            }
            if (w->kind == 3) {
                w->frame->dl = (*bossPrimEnvColorDL)(0, 0, 0, 0, 0, 0, 0x80);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "reversal" at a point, of a size. */
void beReversalCreate(f32 x, f32 y, f32 z, f32 scale) {
    RevArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = scale;
    tskCreate(gEffectTask, beReversalProc, 0, &arg);
}

/* "reversal": a blue ring laid flat, growing by 10% a frame while it fades by 5 (from 0xE0);
 * against the fifth boss, also a spinning ball ten times its size. Sets flag 0x20 of
 * gBrotherFlags. */
s32 beReversalProc(Obj *obj, s16 msg, void *arg) {
    RevWork *w = (RevWork *) ((u32) obj + sizeof(Obj));
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "reversal";
            if (gBattleBossId == 5) {
                w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
                if (w->frame == NULL) {
                    return -1;
                }
            }
            w->ring = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->ring == NULL) {
                return -1;
            }
            if (gBattleBossId == 5) {
                bossSetPolDL(w->frame, beBallDl, 0x120);
                (*bossSetFrameFlags)(w->frame, 0x20);
            }
            bossSetPolDL(w->ring, beMeffectDl, 0xEE);
            (*bossSetFrameFlags)(w->ring, 0x20);
            if (gBattleBossId == 5) {
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = ((RevArg *) arg)->scale * 10.0f;
                w->frame->pos[0] = ((RevArg *) arg)->pos[0];
                w->frame->pos[1] = ((RevArg *) arg)->pos[1];
                w->frame->pos[2] = ((RevArg *) arg)->pos[2];
            }
            w->ring->scale[0] = w->ring->scale[1] = w->ring->scale[2] = ((RevArg *) arg)->scale;
            w->ring->pos[0] = ((RevArg *) arg)->pos[0];
            w->ring->pos[1] = ((RevArg *) arg)->pos[1];
            w->ring->pos[2] = ((RevArg *) arg)->pos[2];
            w->ring->rot[0] = -1.5707964f;
            w->fade = 0xE0;
            gBrotherFlags |= 0x20;
            break;
        case 1:
            if (gBattleBossId == 5) {
                frmDestroy(&w->frame->f);
            }
            frmDestroy(&w->ring->f);
            break;
        case 0:
            w->fade -= 5;
            if (w->fade <= 0) {
                w->fade = 0;
                return -1;
            }
            if (gBattleBossId == 5) {
                w->frame->rot[1] += 0.17453293003880077;
                w->frame->scale[2] *= 1.1;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            }
            w->ring->scale[2] *= 1.1;
            w->ring->scale[0] = w->ring->scale[1] = w->ring->scale[2];
            break;
        case 3:
            if (gBattleBossId == 5) {
                w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, 0, 0, 255, w->fade);
            }
            w->ring->dl = (*bossPrimEnvColorDL)(255, 255, 255, 0, 0, 255, w->fade);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Bpiece": a piece of debris of a random size (0.7 to 1.2 times `scale`), tumbling; it
 * jumps up at random, falls, bounces twice off y = -1000 (at 90%), and ends at the third
 * landing or when it has faded (by 4 a frame). */
s32 beBpieceProc(Obj *obj, s16 msg, void *arg) {
    PieceWork *w = (PieceWork *) ((u32) obj + sizeof(Obj));

    switch (msg) {
        case 2:
            obj->name = "Bpiece";
            w->unused38 = ((PieceArg *) arg)->unused38;
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->flags |= 0x20;
            w->stage = 0;
            w->frame->dl2 = beBpieceDebrisDl;
            floadGetSetRam((void **) w->frame->segs, 0x11E);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                fadeRandRange(70, 120) / 100.0f * ((PieceArg *) arg)->scale;
            w->frame->pos[0] = ((PieceArg *) arg)->pos[0];
            w->frame->pos[1] = ((PieceArg *) arg)->pos[1];
            w->frame->pos[2] = ((PieceArg *) arg)->pos[2];
            w->fade = 255;
            w->stage = 0;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->frame->rot[0] += 0.05235988f;
            w->frame->rot[1] += 0.08726647f;
            w->frame->rot[2] += 0.122173056f;
            w->fade -= 4;
            if (w->fade <= 0) {
                w->fade = 0;
                return -1;
            }
            if (w->stage == 0) {
                w->vel[0] = fadeRandRange(-7, 7);
                w->vel[1] = fadeRandRange(20, 30);
                w->vel[2] = fadeRandRange(-7, 7);
                w->stage++;
                w->fall = 0.0f;
            } else if (w->stage == 1) {
                if (w->frame->pos[1] <= -1000.0f) {
                    w->vel[1] = -(w->vel[1] * 0.9);
                    w->stage++;
                }
            } else if (w->stage == 2) {
                if (w->frame->pos[1] <= -1000.0f) {
                    w->vel[1] = -(w->vel[1] * 0.9);
                    w->stage++;
                }
            } else if (w->stage == 3 && w->frame->pos[1] <= -1000.0f) {
                return -1;
            }
            w->fall += 0.2;
            w->vel[1] -= w->fall;
            if (w->vel[1] < -40.0f) {
                w->vel[1] = -40.0f;
            }
            w->frame->pos[0] += w->vel[0];
            w->frame->pos[1] += w->vel[1];
            w->frame->pos[2] += w->vel[2];
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->fade);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "Bpiece" of a size, at a random point within spread of a point. */
void beBpieceCreate(f32 x, f32 y, f32 z, f32 scale, s32 spread) {
    PieceArg arg;

    arg.pos[0] = fadeRand(spread) + (x - spread / 2);
    arg.pos[1] = fadeRand(spread) + (y - spread / 2);
    arg.pos[2] = fadeRand(spread) + (z - spread / 2);
    arg.spread = spread;
    arg.scale = scale;
    tskCreate(gEffectTask, beBpieceProc, 0, &arg);
}

/* "psmoke": a puff of smoke at a random angle, growing by 0.2 a frame while it fades by 5;
 * kind 0 is dark and rises, kind 1 is a warm glow that ends with its animation. Gone at once
 * while the battle hides its effects (flag 0x4000 of gBrotherFlags, 0x400 of gBattleFlags2, 0x10 of
 * gBattleFlags). */
s32 bePsmokeProc(Obj *obj, s16 msg, void *arg) {
    EffPWork *w = (EffPWork *) ((u32) obj + sizeof(Obj));
    EffPWork *a = arg;

    switch (msg) {
        case 2:
            obj->name = "psmoke";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            if (gBrotherFlags & 0x4000) {
                return -1;
            }
            if (gBattleFlags2 & 0x400) {
                return -1;
            }
            if (gBattleFlags & 0x10) {
                return -1;
            }
            if (gBattleFlags & 4) {
                w->frame->f.flags &= ~2;
            }
            if (a->kind == 0) {
                bossSetPolDL(w->frame, beSmokeDl, 0x123);
                fadeAnimStart(&w->anim, gBeSmokeAnim, 0, &w->frame->segs[3],
                                       floadGetSetFile(0x123, 0));
                (*bossSetFrameFlags)(w->frame, 0x23);
            } else if (a->kind == 1) {
                bossSetPolDL(w->frame, beSmokeDl, 0x123);
                fadeAnimStart(&w->anim, gBeSmokeAnim, 0, &w->frame->segs[3],
                                       floadGetSetFile(0x123, 0));
                (*bossSetFrameFlags)(w->frame, 0x23);
            }
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = a->scale;
            w->frame->rot[2] = fadeRand(360) * 0.0174532930038800785;
            w->kind = a->kind;
            w->pos[0] = a->pos[0];
            w->pos[1] = a->pos[1];
            w->pos[2] = a->pos[2];
            w->frame->pos[0] += w->pos[0];
            w->frame->pos[1] += w->pos[1];
            w->frame->pos[2] += w->pos[2];
            w->alpha = 255;
            break;
        case 0:
            if (w->kind == 1) {
                if (fadeAnimStep(&w->anim) != 0) {
                    return -1;
                }
            } else {
                fadeAnimStep(&w->anim);
            }
            w->alpha -= 5;
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            w->frame->scale[2] += 0.2;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            if (w->kind == 0) {
                w->frame->pos[1] += 1.0f;
            }
            if (gBrotherFlags & 0x4000) {
                return -1;
            }
            if (gBattleFlags2 & 0x400) {
                return -1;
            }
            if (gBattleFlags & 0x10) {
                return -1;
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            if (w->kind == 0) {
                w->frame->dl = (*bossPrimEnvColorDL)(0, 0, 0, 0x40, 0, 0, w->alpha);
            } else if (w->kind == 1) {
                w->frame->dl = (*bossPrimEnvColorDL)(255, 200, 160, 255, 255, 180, w->alpha);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "psmoke" of a kind at a point, of a size. */
void bePsmokeCreate(f32 x, f32 y, f32 z, f32 scale, s32 kind) {
    EffPWork arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = scale;
    arg.kind = kind;
    tskCreate(gEffectTask, bePsmokeProc, 0, &arg);
}

/* The second "reversal": the ball and the ring both, thirty times their size. */
s32 beReversalBigProc(Obj *obj, s16 msg, void *arg) {
    RevWork *w = (RevWork *) ((u32) obj + sizeof(Obj));
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "reversal";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->ring = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->ring == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, beBallDl, 0x120);
            (*bossSetFrameFlags)(w->frame, 0x20);
            bossSetPolDL(w->ring, beMeffectDl, 0xEE);
            (*bossSetFrameFlags)(w->ring, 0x20);
            w->frame->scale[2] = 30.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = ((RevArg *) arg)->pos[0];
            w->frame->pos[1] = ((RevArg *) arg)->pos[1];
            w->frame->pos[2] = ((RevArg *) arg)->pos[2];
            w->ring->scale[2] = 30.0f;
            w->ring->scale[0] = w->ring->scale[1] = w->ring->scale[2];
            w->ring->pos[0] = ((RevArg *) arg)->pos[0];
            w->ring->pos[1] = ((RevArg *) arg)->pos[1];
            w->ring->pos[2] = ((RevArg *) arg)->pos[2];
            w->ring->rot[0] = -1.5707964f;
            w->fade = 0xE0;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->ring->f);
            break;
        case 0:
            w->fade -= 5;
            if (w->fade <= 0) {
                w->fade = 0;
                return -1;
            }
            w->frame->rot[1] += 0.17453293003880077;
            w->frame->scale[2] *= 1.1;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->ring->scale[2] *= 1.1;
            w->ring->scale[0] = w->ring->scale[1] = w->ring->scale[2];
            break;
        case 3:
            w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, 0, 0, 255, w->fade);
            w->ring->dl = (*bossPrimEnvColorDL)(255, 255, 255, 0, 0, 255, w->fade);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The second "reversal" at a point. */
void beReversalBigCreate(f32 x, f32 y, f32 z) {
    RevArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    tskCreate(gEffectTask, beReversalBigProc, 0, &arg);
}

/* The second "psmoke": the same puffs, grey for kind 0, always shown; kind 1 never gets its
 * display list set (no bossSetPolDL). */
s32 bePsmokeGreyProc(Obj *obj, s16 msg, void *arg) {
    EffPWork *w = (EffPWork *) ((u32) obj + sizeof(Obj));
    EffPWork *a = arg;

    switch (msg) {
        case 2:
            obj->name = "psmoke";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            if (a->kind == 0) {
                bossSetPolDL(w->frame, beSmokeDl, 0x123);
                fadeAnimStart(&w->anim, gBeSmokeAnim, 0, &w->frame->segs[3],
                                       floadGetSetFile(0x123, 0));
                (*bossSetFrameFlags)(w->frame, 0x23);
            } else if (a->kind == 1) {
                fadeAnimStart(&w->anim, gBeSmokeAnim, 0, &w->frame->segs[3],
                                       floadGetSetFile(0x123, 0));
                (*bossSetFrameFlags)(w->frame, 0x23);
            }
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = a->scale;
            w->frame->rot[2] = fadeRand(360) * 0.0174532930038800785;
            w->kind = a->kind;
            w->pos[0] = a->pos[0];
            w->pos[1] = a->pos[1];
            w->pos[2] = a->pos[2];
            w->frame->pos[0] = w->pos[0];
            w->frame->pos[1] = w->pos[1];
            w->frame->pos[2] = w->pos[2];
            w->alpha = 255;
            break;
        case 0:
            if (w->kind == 1) {
                if (fadeAnimStep(&w->anim) != 0) {
                    return -1;
                }
            } else {
                fadeAnimStep(&w->anim);
            }
            w->alpha -= 5;
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            w->frame->scale[2] += 0.2;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            if (w->kind == 0) {
                w->frame->pos[1] += 1.0f;
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            if (w->kind == 0) {
                w->frame->dl = (*bossPrimEnvColorDL)(200, 200, 200, 100, 100, 100, w->alpha);
            } else if (w->kind == 1) {
                w->frame->dl = (*bossPrimEnvColorDL)(255, 200, 160, 255, 255, 180, w->alpha);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The second "psmoke" of a kind at a point, of a size. */
void bePsmokeGreyCreate(f32 x, f32 y, f32 z, f32 scale, s32 kind) {
    EffPWork arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = scale;
    arg.kind = kind;
    tskCreate(gEffectTask, bePsmokeGreyProc, 0, &arg);
}

/* A "Nullattack" of a kind at a point, under the battle's task. */
void beNullattackCreate(f32 x, f32 y, f32 z, s32 kind) {
    NullArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.kind = kind;
    arg.pos[2] = z;
    if ((*tskCreate)(gEffectTask, beNullattackProc, gBattleTask->file, &arg) == NULL) {
        PRINTF("隊長,タスクの作成に失敗しました。\n");
    }
}

/* "Nullattack": an invisible attack box of radius 1000 at a point; its kind is one of eight
 * (0x58, 0x55, 0x1A, 0x1D, 0x19, 0x67, 0x68, 0x70). It ends at its first hit. */
s32 beNullattackProc(Obj *obj, s16 msg, void *arg) {
    NullWork *w = (NullWork *) ((u32) obj + sizeof(Obj));
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Nullattack";
            w->frame = (PolFrame *) (*frmCreate)(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->box = (*acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 1000, 0, 0, 0);
            if (w->box == NULL) {
                return -1;
            }
            w->kind = ((NullArg *) arg)->kind;
            if (w->kind == 0x58) {
                w->box->kind = 0x58;
            }
            if (w->kind == 0x55) {
                w->box->kind = 0x55;
            }
            if (w->kind == 0x1A) {
                w->box->kind = 0x1A;
            }
            if (w->kind == 0x1D) {
                w->box->kind = 0x1D;
            }
            if (w->kind == 0x19) {
                w->box->kind = 0x19;
            }
            if (w->kind == 0x67) {
                w->box->kind = 0x67;
            }
            if (w->kind == 0x68) {
                w->box->kind = 0x68;
            }
            if (w->kind == 0x70) {
                w->box->kind = 0x70;
            }
            w->frame->scale[2] = 1.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = ((NullArg *) arg)->pos[0];
            w->frame->pos[1] = ((NullArg *) arg)->pos[1];
            w->frame->pos[2] = ((NullArg *) arg)->pos[2];
            (*bossSetFrameFlags)(w->frame, 0x20);
            break;
        case 3:
            break;
        case 0:
            if ((*acollAtkWasHit)(w->box) == 1) {
                w->box->flags &= ~2;
                return -1;
            }
            break;
        case 1:
            (*frmDestroy)(&w->frame->f);
            (*frmDestroy)(&w->box->frame);
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* "10RYOU": a 10-ryo coin, half `scale`, 500 out from the centre toward the point; it rings
 * (sound 0x232), adds 10 to the 0-999 gauge, waits 32 frames, then rises by 1 a frame while it
 * fades by 6. */
s32 beRyouProc(Obj *obj, s16 msg, void *arg) {
    RyouWork *w = (RyouWork *) ((u32) obj + sizeof(Obj));
    s32 pad;
    f32 x;
    f32 z;
    s32 pad2[3];

    switch (msg) {
        case 2:
            obj->name = "10RYOU";
            w->frame = (PolFrame *) frmCreate(gCockpitPanelFrame, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, beRyouCoinDl, 0xEA);
            (*bossSetFrameFlags)(w->frame, 0x27);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = ((RyouArg *) arg)->scale / 2;
            bossHeadingOffset(mathAtan2(((RyouArg *) arg)->pos[0], ((RyouArg *) arg)->pos[2]),
                                   0.0f, 500.0f, &x, &z);
            w->frame->pos[0] = x;
            w->frame->pos[1] = ((RyouArg *) arg)->pos[1];
            w->frame->pos[2] = z;
            (*SoundLocation2)(0x232, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                      300.0f, 15000.0f, gBattleCamView->fovy);
            bossAddMoney(10);
            w->alpha = 255;
            w->stage = 0;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            switch (w->stage) {
                case 0:
                    if (w->time++ > 30) {
                        w->stage++;
                    }
                    break;
                case 1:
                    w->frame->pos[1] += 1.0f;
                    w->alpha -= 6;
                    if (w->alpha < 0) {
                        return -1;
                    }
                    if (w->frame->pos[1] < -1000.0f) {
                        return -1;
                    }
                    break;
            }
            break;
        case 3:
            w->frame->dl = (*bossXluPrimEnvColorDL)(0, 0, 0, w->alpha);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "10RYOU" coin toward a point, of a size, under the battle's task. */
void ryouCreate(f32 x, f32 y, f32 z, f32 scale) {
    RyouArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = scale;
    if ((*tskCreate)(gEffectTask, beRyouProc, gBattleTask->file, &arg) == NULL) {
        PRINTF("[ryouCreate]隊長,タスクの作成に失敗しました。\n");
    }
}

/* An "Lball" at a point, under the battle's task. */
void beLballCreate(f32 x, f32 y, f32 z, s32 unused) {
    LballArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.unused3C = unused;
    arg.pos[2] = z;
    if ((*tskCreate)(gEffectTask, beLballProc, gBattleTask->file, &arg) == NULL) {
        PRINTF("隊長,タスクの作成に失敗しました。\n");
    }
}

/* "Lball": a ball of light ten times its size with a "flashP" held 120 frames; it flickers in
 * size for 120 frames, then at twice the size to 240, then swells by 3 a frame while it fades
 * by 10. Gone at once while flag 0x4000 of gBattleFlags2 is on. */
s32 beLballProc(Obj *obj, s16 msg, void *arg) {
    LballWork *w = (LballWork *) ((u32) obj + sizeof(Obj));

    switch (msg) {
        case 2:
            obj->name = "Lball";
            w->frame = (PolFrame *) (*frmCreate)(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            (*bossSetPolDL)(w->frame, beFlashDl, 0x120);
            (*bossSetFrameFlags)(w->frame, 0x22);
            w->frame->pos[0] = ((LballArg *) arg)->pos[0];
            w->frame->pos[1] = ((LballArg *) arg)->pos[1];
            w->frame->pos[2] = ((LballArg *) arg)->pos[2];
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale = 10.0f;
            (*beFlashPCreate)(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 120);
            w->unused3C = ((LballArg *) arg)->unused3C;
            w->alpha = 255;
            w->time = 0;
            w->stage = 0;
            break;
        case 0:
            if (gBattleFlags2 & 0x4000) {
                return -1;
            }
            switch (w->stage) {
                case 0:
                    w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                        (*fadeRandRange)(9, 11) / 10.0f * w->scale;
                    if (w->time > 120) {
                        w->stage++;
                    }
                    break;
                case 1:
                    w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                        (*fadeRandRange)(9, 11) / 10.0f * (w->scale * 2);
                    if (w->time > 240) {
                        w->stage++;
                    }
                    break;
                case 2:
                    w->frame->scale[2] += 3.0;
                    w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                    w->alpha -= 10;
                    if (w->alpha < 0) {
                        return -1;
                    }
                    break;
            }
            w->time++;
            break;
        case 3:
            if (gGameFrameCount % 5 == 0) {
                w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, 255, 255, 255, w->alpha);
            } else {
                w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, 255, 255, 0, w->alpha);
            }
            break;
        case 1:
            (*frmDestroy)(&w->frame->f);
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* "speffect": a purple ring of a kind: 0 and 1 stand tilted and rise by 10 a frame (and go at
 * once while the battle hides its effects), 2 lies flat; it grows by 4% a frame, fades in by
 * 10 and out again. */
s32 beSpeffectProc(Obj *obj, s16 msg, void *arg) {
    SpWork *w = (SpWork *) ((u32) obj + sizeof(Obj));
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "speffect";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->kind = ((SpArg *) arg)->kind;
            if (w->kind == 0 || w->kind == 1) {
                if (gBrotherFlags & 0x4000) {
                    return -1;
                }
                if (gBattleFlags2 & 0x400) {
                    return -1;
                }
                if (gBattleFlags & 0x10) {
                    return -1;
                }
                if (gBrotherFlags & 0x10000) {
                    return -1;
                }
            }
            if (w->kind == 0) {
                bossSetPolDL(w->frame, beHitringDl, 0xEE);
            } else if (w->kind == 1) {
                bossSetPolDL(w->frame, beHitringDl, 0xEE);
            } else if (w->kind == 2) {
                bossSetPolDL(w->frame, beHitringDl, 0xEE);
            }
            (*bossSetFrameFlags)(w->frame, 0x20);
            if (w->kind == 0) {
                w->frame->scale[2] = 0.3f;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            }
            if (w->kind == 1) {
                w->frame->scale[2] = 0.3f;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            }
            if (w->kind == 2) {
                w->frame->scale[2] = 0.3f;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            }
            if (w->kind == 0) {
                w->frame->rot[0] = 2.0943952f;
            }
            if (w->kind == 1) {
                w->frame->rot[0] = 2.0943952f;
            }
            if (w->kind == 2) {
                w->frame->rot[0] = 1.5707964f;
            }
            w->frame->pos[0] = ((SpArg *) arg)->pos[0];
            w->frame->pos[1] = ((SpArg *) arg)->pos[1];
            w->frame->pos[2] = ((SpArg *) arg)->pos[2];
            w->alpha = 0;
            w->stage = 0;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (w->kind == 0 || w->kind == 1) {
                if (gBrotherFlags & 0x4000) {
                    return -1;
                }
                if (gBattleFlags2 & 0x400) {
                    return -1;
                }
                if (gBattleFlags & 0x10) {
                    return -1;
                }
                if (gBrotherFlags & 0x10000) {
                    return -1;
                }
            }
            w->frame->scale[2] *= 1.04;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            if (w->kind != 2) {
                w->frame->pos[1] += 10.0f;
            }
            switch (w->stage) {
                case 0:
                    w->alpha += 10;
                    if (w->alpha >= 256) {
                        w->alpha = 255;
                        w->stage++;
                    }
                    break;
                case 1:
                    w->alpha -= 10;
                    if (w->alpha <= 0) {
                        w->alpha = 0;
                        return -1;
                    }
                    break;
                case 2:
                    break;
                case 3:
                    break;
            }
            break;
        case 3:
            w->frame->dl = (*bossPrimEnvColorDL)(100, 0, 255, 0, 100, 255, w->alpha);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "speffect" of a kind at a point. */
void beSpeffectCreate(f32 x, f32 y, f32 z, s32 kind) {
    SpArg arg;

    arg.kind = kind;
    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    tskCreate(gEffectTask, beSpeffectProc, 0, &arg);
}

/* "fakeImp": the Impact (by pilot) at the boss's position and heading, twenty times its size,
 * playing three animations in turn, holding 30 frames, then a last one for 13. Gone at once
 * while the battle hides its effects. */
s32 beFakeImpactProc(Obj *obj, s16 msg, void *arg) {
    FakeImpWork *w = (FakeImpWork *) ((u32) obj + sizeof(Obj));

    switch (msg) {
        case 2:
            obj->name = "fakeImp";
            w->frame = (AnimeFrame *) frmCreate(gFrmLayer2, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            if (gBattleFlags & 0x4000000) {
                (*bossSetAnimAt)(w->frame, beFakeImpactOmitsuAnim1, 0xEA, 0.0f);
            } else {
                (*bossSetAnimAt)(w->frame, beFakeImpactGoemonAnim1, 0xEA, 0.0f);
            }
            (*bossSetFrameFlags2)(w->frame, 0x20);
            w->n = ((FakeImpArg *) arg)->n;
            w->frame->scale[2] = 20.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = gBossPosX;
            w->frame->pos[1] = gBossPosY;
            w->frame->pos[2] = gBossPosZ;
            w->frame->rot[1] = gBossYaw;
            w->unused10 = 0;
            w->time = 0;
            w->stage = 0;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (gBrotherFlags & 0x4000) {
                return -1;
            }
            if (gBattleFlags2 & 0x400) {
                return -1;
            }
            w->frame->rot[1] = gBossYaw;
            switch (w->stage) {
                case 0:
                    if ((*bossStepAnim)(w->frame, 0.2f, 2) != 0) {
                        if (gBattleFlags & 0x4000000) {
                            (*bossSetAnimAt)(w->frame, beFakeImpactOmitsuAnim2, 0xEA, 0.0f);
                        } else {
                            (*bossSetAnimAt)(w->frame, beFakeImpactGoemonAnim2, 0xEA, 0.0f);
                        }
                        w->stage++;
                    }
                    break;
                case 1:
                    if ((*bossStepAnim)(w->frame, 0.2f, 2) != 0) {
                        if (gBattleFlags & 0x4000000) {
                            (*bossSetAnimAt)(w->frame, beFakeImpactOmitsuAnim3, 0xEA, 0.0f);
                        } else {
                            (*bossSetAnimAt)(w->frame, beFakeImpactGoemonAnim3, 0xEA, 0.0f);
                        }
                        w->stage++;
                    }
                    break;
                case 2:
                    if ((*bossStepAnim)(w->frame, 0.2f, 2) != 0) {
                        w->time = 0;
                        w->stage++;
                    }
                    break;
                case 3:
                    if (w->time++ >= 30) {
                        if (gBattleFlags & 0x4000000) {
                            (*bossSetAnimAt)(w->frame, beFakeImpactOmitsuAnim3, 0xEA, 1.0f);
                        } else {
                            (*bossSetAnimAt)(w->frame, beFakeImpactGoemonAnim3, 0xEA, 1.0f);
                        }
                        w->time = 0;
                        w->stage++;
                    }
                    break;
                case 4:
                    (*bossStepAnim)(w->frame, 0.4f, 3);
                    if (w->time++ >= 13) {
                        return -1;
                    }
                    break;
            }
            break;
        case 3:
            w->frame->dl = (*bossLitTexEnvColorDL)(0, 0, 0, 0);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "fakeImp" under the battle's task (the point is unused: it stands where the boss is). */
void fakeImpactCreate(f32 x, f32 y, f32 z, s32 n) {
    FakeImpArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.n = n;
    arg.pos[2] = z;
    if ((*tskCreate)(gEffectTask, beFakeImpactProc, gBattleTask->file, &arg) == NULL) {
        PRINTF("[fakeImpactCreate]隊長,タスクの作成に失敗しました。\n");
    }
}

/* "Bspace": the raster, a frame of the display-list kind; arg 0 is the battle's (under the
 * first pool), 1 a demo's (under gFrmLayer8), which ends when flag 0x80000 of gBattleFlags2 is
 * set (and clears it). */
s32 beBspaceProc(Obj *obj, s16 msg, void *arg) {
    BspaceWork *w = (BspaceWork *) ((u32) obj + sizeof(Obj));
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Bspace";
            if ((s32) arg == 0) {
                w->demo = 0;
                PRINTF("通常ラスター発生します。\n");
                w->frame = (DLFrame *) (*frmCreateNext)(gFrmLayers[0], gFrmKindDlist);
                if (w->frame == NULL) {
                    return -1;
                }
            } else if ((s32) arg == 1) {
                w->demo = 1;
                PRINTF("デモ専用ラスター発生します。\n");
                w->frame = (DLFrame *) (*frmCreateNext)(gFrmLayer8, gFrmKindDlist);
                if (w->frame == NULL) {
                    return -1;
                }
            }
            w->unusedC = 0;
            break;
        case 1:
            (*frmDestroy)(&w->frame->f);
            break;
        case 0:
            if (w->demo == 1 && (gBattleFlags2 & 0x80000)) {
                PRINTF("デモ専用ラスター消失します。\n");
                gBattleFlags2 &= ~0x80000;
                return -1;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gGfxDlHead = beBspaceDrawRaster(gGfxDlHead, obj, w);
            gSPEndDisplayList(gGfxDlHead++);
            w->time++;
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* Draws rows y to y + h of the framebuffer being drawn back over it, stretched by skew rows at
 * each end (squeezed if skew is negative), in ten strips 32 wide. Matched 2026-10-03 by taking
 * the framebuffer's address at the declaration, straight from the call (an s32 fb and
 * gFramebuffers[fb] in the macro gave other registers, 206 lines), then dropping an old FAKE
 * "if (i) {}" (46 lines with it). */
Gfx *beDrawRasterBand(Gfx *gfx, s32 y, s32 h, s32 skew) {
    u16 *img = gFramebuffers[(*gfxGetFbIndex)()];
    s32 top = y;
    s32 bottom;
    s32 y0;
    s32 y1;
    s32 dy;
    s32 i;
    s32 left;
    s32 right;

    if (y < gGameState.scissor.uly) {
        top = gGameState.scissor.uly;
    } else if (y >= gGameState.scissor.lry) {
        return gfx;
    }
    bottom = y + h;
    if (y + h < gGameState.scissor.uly) {
        return gfx;
    }
    y0 = top - skew;
    if (bottom >= gGameState.scissor.lry) {
        bottom = gGameState.scissor.lry - 1;
    }
    y1 = bottom + skew;
    if (y1 - y0 <= 0) {
        return gfx;
    }
    if (bottom - top <= 0) {
        bottom = top + 1;
    }
    dy = bottom - top;
    for (i = 0; i < 10; i++) {
        left = i * 32;
        right = (i + 1) * 32;
        gDPPipeSync(gfx++);
        gDPLoadTextureTile(gfx++, img, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, SCREEN_HEIGHT, left,
                           top, right - 1, bottom - 1, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK,
                           G_TX_NOLOD, G_TX_NOLOD);
        gSPTextureRectangle(gfx++, left << 2, y0 << 2, right << 2, y1 << 2, G_TX_RENDERTILE, left << 5, top << 5,
                            1 << 10, (dy << 10) / (y1 - y0));
    }
    return gfx;
}

/* "Bspace"'s raster: the screen in bands 60 rows high, scrolling down a row a frame, every
 * other band squeezed by 4 rows at each end and the rest stretched by 4. */
Gfx *beBspaceDrawRaster(Gfx *gfx, Obj *obj, BspaceWork *w) {
    s32 pad;
    s32 base;
    s32 odd;
    s32 i;

    gSPDisplayList(gfx++, gBeBspaceRasterDl);
    base = w->time % 60 - 60;
    odd = (w->time / 60) & 1;
    for (i = 0; i < 4; i++) {
        gfx = beDrawRasterBand(gfx, base + i * 120 + odd * 60, 60, -4);
    }
    for (i = 0; i < 4; i++) {
        gfx = beDrawRasterBand(gfx, base + i * 120 + (odd ^ 1) * 60, 60, 4);
    }
    return gfx;
}

/* "megaRef": for 62 frames, an "M3effect" 500 beyond the boss every fifth frame. */
s32 beMegaRefProc(Obj *obj, s16 msg, void *arg) {
    MegaRefWork *w = (MegaRefWork *) ((u32) obj + sizeof(Obj));

    switch (msg) {
        case 2:
            obj->name = "megaRef";
            w->pos = ((MegaRefArg *) arg)->pos[0];
            w->pos = ((MegaRefArg *) arg)->pos[1];
            w->pos = ((MegaRefArg *) arg)->pos[2];
            w->time = 0;
            w->unused14 = 0;
            break;
        case 1:
            break;
        case 0:
            if (gGameFrameCount % 5 == 0) {
                (*beM3effectCreate)(gBossPosX, gBossPosY, gBossPosZ + 500.0f);
            }
            if (w->time++ > 60) {
                return -1;
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "megaRef" at a point, under the battle's task. */
void beMegaRefCreate(f32 x, f32 y, f32 z) {
    MegaRefArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    if ((*tskCreate)(gEffectTask, beMegaRefProc, gBattleTask->file, &arg) == NULL) {
        PRINTF("[megaRef]隊長,タスクの作成に失敗しました。\n");
    }
}

/* "brownF": a brown tint fading in by 20 to 0xA0, for 401 frames. */
s32 beBrownFProc(Obj *obj, s16 msg, void *arg) {
    BrownWork *w = (BrownWork *) ((u32) obj + sizeof(Obj));
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "brownF";
            w->frame = (PolFrame *) (*frmCreateNext)(gCockpitRootFrame, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            (*bossSetPolDL)(w->frame, beFilterDl, 0xEA);
            (*bossSetFrameFlags)(w->frame, 0x27);
            w->frame->scale[2] = 0.2f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = ((BrownArg *) arg)->pos[0];
            w->frame->pos[1] = ((BrownArg *) arg)->pos[1];
            w->frame->pos[2] = ((BrownArg *) arg)->pos[2];
            w->alpha = 0;
            w->time = 0;
            w->stage = 0;
            break;
        case 0:
            switch (w->stage) {
                case 0:
                    w->alpha += 20;
                    if (w->alpha > 160) {
                        w->alpha = 160;
                    }
                    if (w->time > 400) {
                        return -1;
                    }
                    break;
                case 1:
                    break;
                case 2:
                    break;
                case 3:
                    break;
            }
            w->time++;
            break;
        case 3:
            w->frame->dl = (*bossXluPrimEnvColorDL)(0, 0, 0, w->alpha);
            break;
        case 1:
            (*frmDestroy)(&w->frame->f);
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* A "brownF" at a point, under the battle's task. */
void beBrownFCreate(f32 x, f32 y, f32 z, s32 unused) {
    BrownArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.unused24 = unused;
    arg.pos[2] = z;
    if ((*tskCreate)(gEffectTask, beBrownFProc, gBattleTask->file, &arg) == NULL) {
        PRINTF("隊長,タスクの作成に失敗しました。\n");
    }
}

/* "Shibuki": a splash of three parts at random angles, by kind (3 is small; 2 starts at
 * 0xC0, though the fade starts at 255 for all): it shoots up to a height (16, 32 for kind 1,
 * 20 for kind 3) as it widens, then widens further as it falls and fades by 10 a frame. */
s32 beShibukiProc(Obj *obj, s16 msg, void *arg) {
    ShibukiWork *w = (ShibukiWork *) ((u32) obj + sizeof(Obj));
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Shibuki";
            w->kind = ((ShibukiArg *) arg)->kind;
            w->frame = (PolFrame *) (*frmCreate)(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->f1 = (PolFrame *) (*frmCreate)(gFrmLayer8, gFrmKindPol);
            if (w->f1 == NULL) {
                return -1;
            }
            w->f2 = (PolFrame *) (*frmCreate)(gFrmLayer8, gFrmKindPol);
            if (w->f2 == NULL) {
                return -1;
            }
            (*bossSetPolDL)(w->frame, beShibukiPart1Dl, 0xEE);
            (*bossSetPolDL)(w->f1, beShibukiPart2Dl, 0xEE);
            (*bossSetPolDL)(w->f2, beShibukiPart3Dl, 0xEE);
            (*bossSetFrameFlags)(w->frame, 0x20);
            (*bossSetFrameFlags)(w->f1, 0x20);
            (*bossSetFrameFlags)(w->f2, 0x20);
            w->frame->pos[0] = ((ShibukiArg *) arg)->pos[0];
            w->frame->pos[1] = ((ShibukiArg *) arg)->pos[1];
            w->frame->pos[2] = ((ShibukiArg *) arg)->pos[2];
            w->f1->pos[0] = ((ShibukiArg *) arg)->pos[0];
            w->f1->pos[1] = ((ShibukiArg *) arg)->pos[1];
            w->f1->pos[2] = ((ShibukiArg *) arg)->pos[2];
            w->f2->pos[0] = ((ShibukiArg *) arg)->pos[0];
            w->f2->pos[1] = ((ShibukiArg *) arg)->pos[1];
            w->f2->pos[2] = ((ShibukiArg *) arg)->pos[2];
            w->frame->rot[1] = (*fadeRand)(360) * 0.0174532930038800785;
            w->f1->rot[1] = (*fadeRand)(360) * 0.0174532930038800785;
            w->f2->rot[1] = (*fadeRand)(360) * 0.0174532930038800785;
            if (w->kind == 0) {
                w->frame->scale[2] = 2.0f;
                w->frame->scale[0] = w->frame->scale[2];
                w->frame->scale[1] = 2.0f;
                w->f1->scale[2] = 3.0f;
                w->f1->scale[0] = w->f1->scale[2];
                w->f1->scale[1] = 2.0f;
                w->f2->scale[2] = 4.0f;
                w->f2->scale[0] = w->f2->scale[2];
                w->f2->scale[1] = 2.0f;
                w->alpha = 255.0f;
            } else if (w->kind == 1) {
                w->frame->scale[2] = 4.0f;
                w->frame->scale[0] = w->frame->scale[2];
                w->frame->scale[1] = 4.0f;
                w->f1->scale[2] = 6.0f;
                w->f1->scale[0] = w->f1->scale[2];
                w->f1->scale[1] = 4.0f;
                w->f2->scale[2] = 8.0f;
                w->f2->scale[0] = w->f2->scale[2];
                w->f2->scale[1] = 4.0f;
                w->alpha = 255.0f;
            } else if (w->kind == 2) {
                w->frame->scale[2] = 4.0f;
                w->frame->scale[0] = w->frame->scale[2];
                w->frame->scale[1] = 4.0f;
                w->f1->scale[2] = 6.0f;
                w->f1->scale[0] = w->f1->scale[2];
                w->f1->scale[1] = 4.0f;
                w->f2->scale[2] = 8.0f;
                w->f2->scale[0] = w->f2->scale[2];
                w->f2->scale[1] = 4.0f;
                w->alpha = 192.0f;
            } else if (w->kind == 3) {
                w->frame->scale[2] = 0.5f;
                w->frame->scale[0] = w->frame->scale[2];
                w->frame->scale[1] = 0.5f;
                w->f1->scale[2] = 1.0f;
                w->f1->scale[0] = w->f1->scale[2];
                w->f1->scale[1] = 0.5f;
                w->f2->scale[2] = 1.0f;
                w->f2->scale[0] = w->f2->scale[2];
                w->f2->scale[1] = 0.5f;
                w->alpha = 255.0f;
            }
            w->stage = 0;
            w->unusedC = 0;
            w->alpha = 255.0f;
            break;
        case 1:
            (*frmDestroy)(&w->frame->f);
            (*frmDestroy)(&w->f1->f);
            (*frmDestroy)(&w->f2->f);
            break;
        case 0:
            switch (w->stage) {
                case 0:
                    if (w->kind == 0) {
                        w->frame->scale[2] += 0.3;
                        w->frame->scale[0] = w->frame->scale[2];
                        w->f1->scale[2] += 0.3;
                        w->f1->scale[0] = w->f1->scale[2];
                        w->f2->scale[2] += 0.3;
                        w->f2->scale[0] = w->f2->scale[2];
                        w->frame->scale[1] += 1.2;
                        w->f1->scale[1] += 0.6;
                        w->f2->scale[1] += 0.6;
                        if (w->frame->scale[1] > 16.0) {
                            w->stage++;
                        }
                    } else if (w->kind == 1) {
                        w->frame->scale[2] += 0.3;
                        w->frame->scale[0] = w->frame->scale[2];
                        w->f1->scale[2] += 0.3;
                        w->f1->scale[0] = w->f1->scale[2];
                        w->f2->scale[2] += 0.3;
                        w->f2->scale[0] = w->f2->scale[2];
                        w->frame->scale[1] += 1.2;
                        w->f1->scale[1] += 0.6;
                        w->f2->scale[1] += 0.6;
                        if (w->frame->scale[1] > 32.0) {
                            w->stage++;
                        }
                    } else if (w->kind == 2) {
                        w->frame->scale[2] += 0.3;
                        w->frame->scale[0] = w->frame->scale[2];
                        w->f1->scale[2] += 0.3;
                        w->f1->scale[0] = w->f1->scale[2];
                        w->f2->scale[2] += 0.3;
                        w->f2->scale[0] = w->f2->scale[2];
                        w->frame->scale[1] += 1.2;
                        w->f1->scale[1] += 0.6;
                        w->f2->scale[1] += 0.6;
                        if (w->frame->scale[1] > 16.0) {
                            w->stage++;
                        }
                    } else if (w->kind == 3) {
                        w->frame->scale[1] += 2.0;
                        w->f1->scale[1] += 0.2;
                        w->f2->scale[1] += 0.2;
                        if (w->frame->scale[1] > 20.0) {
                            w->stage++;
                        }
                    }
                    break;
                case 1:
                    if (w->kind == 0) {
                        w->frame->scale[2] += 0.6;
                        w->frame->scale[0] = w->frame->scale[2];
                        w->f1->scale[2] += 0.6;
                        w->f1->scale[0] = w->f1->scale[2];
                        w->f2->scale[2] += 0.6;
                        w->f2->scale[0] = w->f2->scale[2];
                        w->frame->scale[1] -= 0.6;
                        w->f1->scale[1] -= 0.75;
                        w->f2->scale[1] -= 0.6;
                        if (w->f2->scale[1] < 0.01) {
                            w->f2->scale[1] = 0.01f;
                        }
                        w->frame->pos[1] -= 6.0f;
                        w->f1->pos[1] -= 6.0f;
                        w->f2->pos[1] -= 6.0f;
                    } else if (w->kind == 1) {
                        w->frame->scale[2] += 0.6;
                        w->frame->scale[0] = w->frame->scale[2];
                        w->f1->scale[2] += 0.6;
                        w->f1->scale[0] = w->f1->scale[2];
                        w->f2->scale[2] += 0.6;
                        w->f2->scale[0] = w->f2->scale[2];
                        w->frame->scale[1] -= 0.6;
                        w->f1->scale[1] -= 0.75;
                        w->f2->scale[1] -= 0.6;
                        if (w->f2->scale[1] < 0.01) {
                            w->f2->scale[1] = 0.01f;
                        }
                        w->frame->pos[1] -= 6.0f;
                        w->f1->pos[1] -= 6.0f;
                        w->f2->pos[1] -= 6.0f;
                    } else if (w->kind == 2) {
                        w->frame->scale[2] += 0.6;
                        w->frame->scale[0] = w->frame->scale[2];
                        w->f1->scale[2] += 0.6;
                        w->f1->scale[0] = w->f1->scale[2];
                        w->f2->scale[2] += 0.6;
                        w->f2->scale[0] = w->f2->scale[2];
                        w->frame->scale[1] -= 0.6;
                        w->f1->scale[1] -= 0.75;
                        w->f2->scale[1] -= 0.6;
                        if (w->f2->scale[1] < 0.01) {
                            w->f2->scale[1] = 0.01f;
                        }
                        w->frame->pos[1] -= 6.0f;
                        w->f1->pos[1] -= 6.0f;
                        w->f2->pos[1] -= 6.0f;
                    } else if (w->kind == 3) {
                        w->frame->scale[2] += 0.03;
                        w->frame->scale[0] = w->frame->scale[2];
                        w->f1->scale[2] += 0.03;
                        w->f1->scale[0] = w->f1->scale[2];
                        w->f2->scale[2] += 0.03;
                        w->f2->scale[0] = w->f2->scale[2];
                        w->frame->scale[1] -= 0.1;
                        w->f1->scale[1] -= 0.2;
                        w->f2->scale[1] -= 0.2;
                        if (w->f2->scale[1] < 0.01) {
                            w->f2->scale[1] = 0.01f;
                        }
                        w->frame->pos[1] -= 3.0f;
                        w->f1->pos[1] -= 3.0f;
                        w->f2->pos[1] -= 3.0f;
                    }
                    w->alpha -= 10.0f;
                    if (w->alpha < 0.0f) {
                        return -1;
                    }
                    break;
            }
            break;
        case 3:
            w->frame->dl = (*bossPrimEnvColorDL)(255, 255, 255, 0, 0, 255, w->alpha);
            w->f1->dl = (*bossPrimEnvColorDL)(255, 255, 255, 0, 0, 255, w->alpha);
            w->f2->dl = (*bossPrimEnvColorDL)(255, 255, 255, 0, 0, 255, w->alpha);
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* A "Shibuki" of a kind at a point, under the battle's task. */
void beShibukiCreate(f32 x, f32 y, f32 z, s32 kind) {
    ShibukiArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.kind = kind;
    arg.pos[2] = z;
    if ((*tskCreate)(gEffectTask, beShibukiProc, gBattleTask->file, &arg) == NULL) {
        PRINTF("隊長,タスクの作成に失敗しました。\n");
    }
}
