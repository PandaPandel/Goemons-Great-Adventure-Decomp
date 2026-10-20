#include "common.h"
#include "object.h"
#include "player.h"
#include "acoll.h"
#include "game.h"
#include "boss.h"
#include "bmove.h"
#include "contpak.h"
#include "fade.h"
#include "fileload.h"
#include "joypad.h"
#include "mathutil.h"
#include "polygon.h"
#include "stage.h"
#include "vram.h"

/* The boss battle's movement and state helpers (overlay 10): moving a battle object toward a
 * point or within limits, turning it, gravity, bobbing, the gauges and lives, colour display
 * lists, a button-sequence check, rumble, and clearing the battle's state. */

/* The debug prints ([BmoveX], [bossMoveLimit], [vramCheck] ...) compile to their strings alone:
 * no code, the strings kept in .rodata, as the game's build did. */
#define DPRINTF

/* A battle object's work: its frame and its velocity. */
typedef struct BObj {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 vel[3];
} BObj;

#define BOBJ(task) ((BObj *) ((u32) (task) + sizeof(Obj)))

/* A battle actor's action. */
typedef struct BActor {
    /* 0x00 */ u8 pad0[0x50]; /* the boss's own fields */
    /* 0x50 */ s8 action;
    /* 0x51 */ s8 step;
    /* 0x52 */ s8 nextAction; /* the action and step to resume after the move */
    /* 0x53 */ s8 nextStep;
} BActor;

#define BACTOR(task) ((BActor *) ((u32) (task) + sizeof(Obj)))

extern Gfx *gGfxDlHead; /* the display-list buffer */
extern u32 gGameFrameCount; /* GameState's frame count */
extern Gfx gDrawSetupXluSurfDl[];
s32 gBattleUnused1 = 0;
s32 gAfterImageAlpha1 = 0;
s32 gAfterImageAlpha2 = 0;
/* The battle state, in the order bossClearState clears it. */
s32 gBattleUnused2 = 0;
s32 gBossDownCount = 0; /* a counter (bossAddDownCount) */
s32 gBossFlinchCount = 0; /* a counter (bossAddFlinchCount) */
f32 gMegaShotPosX = 0.0f; /* the cannon position (bappcam.c) */
f32 gMegaShotPosY = 0.0f;
f32 gMegaShotPosZ = 0.0f;
f32 gMegaBurstY = 0.0f;
s32 gBattleBossId = 0; /* the boss, 1-5 (bmenu.c order) */
u32 gBattleFlags = 0; /* flags: 1 hit, 2 the battle is over */
u32 gBrotherFlags = 0;
u32 gBossFlags = 0;
u32 gBattleSndToggles = 0;
u32 gBattleFlags2 = 0;
f32 gCockpitLookAtX = 0.0f; /* the Impact position */
f32 gCockpitLookAtY = 0.0f;
f32 gCockpitLookAtZ = 0.0f;
s32 gImpactPower = 0; /* a gauge, 0-100 */
s32 gBrotherMoveTime = 0;
s32 gBrotherMove = 0;
s32 gBattleUnused3 = 1;
s32 gImpactMoney = 0; /* a gauge, 0-999 (500 at first) */
s32 gImpactArmor = 0; /* two lives, 0-100 */
s32 gImpactArmor2 = 0;
f32 gImpactPosX = 0.0f;
f32 gImpactPosY = 0.0f;
f32 gImpactPosZ = 0.0f;
f32 gBrotherPosX = 0.0f;
f32 gBrotherPosY = 0.0f;
f32 gBrotherPosZ = 0.0f;
f32 gBrotherYaw = 0.0f;
f32 gCockpitRotX = 0.0f;
f32 gCockpitRotY = 0.0f;
f32 gCockpitRotZ = 0.0f;
s32 gImpactHitDelay = 0;
s32 gBrotherDownTime = 0;
s32 gBattleUnused4 = 0;
s32 gBossLifeMax = 0;
s32 gBossLife = 0; /* the boss life */
s32 gBattleUnused5 = 0;
s32 gImpactHitDamage = 0;
f32 gBossPosX = 0.0f; /* the boss position */
f32 gBossPosY = 0.0f;
f32 gBossPosZ = -1600.0f;
f32 gBossYaw = 0.0f;
f32 gCockpitAimX = 0.0f;
f32 gCockpitAimY = 0.0f;
f32 gCockpitAimZ = 0.0f;
f32 gCockpitHeadPitch = 0.0f;
f32 gCockpitHeadYaw = 0.0f;
f32 gBrotherAimX = 0.0f;
f32 gBrotherAimY = 0.0f;
f32 gBrotherAimZ = 0.0f;
f32 gBattleUnused6 = 0.0f;
f32 gBattleUnused7 = 0.0f;
f32 gBattleUnused8 = 0.0f;
f32 gBattleUnused9 = 0.0f;
s32 gImpactMoveTime = 0;
s32 gBattleUnused10 = 0;
s32 gBattleSeqDone = 0; /* 1 once a button sequence is done (bossCheckButtonSeq) */
s32 gBossActionArg = 0;
s32 gBossPhase = 0;
s32 gBossActionCount = 0;
s32 gBossFrameCount = 0;
u8 gValcanSide = 0;
u8 gOvalcanSide = 0;
u8 gBattleUnused11 = 0;
u8 gCockpitShake = 0;
u8 gBattleUnused12 = 0;

/* The controls: the cockpit controller, then a button for each action. */
u16 gCockpitPort = 0; /* the controller (port) */
u16 gCockpitBtnStraight = A_BUTTON; /* the punches */
u16 gCockpitBtnJab = B_BUTTON;
u16 gCockpitBtnShoot = Z_TRIG; /* the shoot button (vulcan, cannon) */
u16 gCockpitBtnBaton = R_TRIG; /* the baton */
u16 gCockpitBtnL = L_TRIG;
u16 gCockpitBtnStart = START_BUTTON;
u16 gCockpitBtnCUp = U_CBUTTONS;
u16 gCockpitBtnCRight = R_CBUTTONS;
u16 gCockpitBtnCDown = D_CBUTTONS;
u16 gCockpitBtnCLeft = L_CBUTTONS;
u16 gCockpitBtnDUp = U_JPAD; /* the D-pad (up, right, down, left) */
u16 gCockpitBtnDRight = R_JPAD;
u16 gCockpitBtnDDown = D_JPAD;
u16 gCockpitBtnDLeft = L_JPAD;
/* The same for the second Impact ("Brother"). */
u16 gBrotherPort = 1; /* the controller (port) */
u16 gBrotherBtnJump = A_BUTTON; /* jump */
u16 gBrotherBtnPunch = B_BUTTON; /* punch */
u16 gBrotherBtnShoot = Z_TRIG; /* shoot */
u16 gBrotherBtnShout = R_TRIG; /* shout */
u16 gBrotherBtnL = L_TRIG;
u16 gBrotherBtnStart = START_BUTTON;
u16 gBrotherBtnCUp = U_CBUTTONS;
u16 gBrotherBtnCRight = R_CBUTTONS;
u16 gBrotherBtnCDown = D_CBUTTONS;
u16 gBrotherBtnCLeft = L_CBUTTONS;
u16 gBrotherBtnDUp = U_JPAD;
u16 gBrotherBtnDRight = R_JPAD;
u16 gBrotherBtnDDown = D_JPAD;
u16 gBrotherBtnDLeft = L_JPAD;
u16 gBattlePort3 = 2;
u16 gBattlePort4 = 3;

/* Button sequences for bossCheckButtonSeq, ended by 0xFFFF (cockpit.c reads them). */
u16 gBattleSeqHundredPunch[] = { U_CBUTTONS, D_CBUTTONS, U_CBUTTONS, 0xFFFF }; /* the hundred punch */
u16 gBattleSeqCannon0[] = { U_CBUTTONS, R_CBUTTONS, D_CBUTTONS, L_CBUTTONS, 0xFFFF }; /* the cannon */
u16 gBattleSeqCannon1[] = { U_CBUTTONS, L_CBUTTONS, D_CBUTTONS, R_CBUTTONS, U_CBUTTONS, 0xFFFF };
u16 gBattleSeqCannon2[] = {
    U_CBUTTONS, U_CBUTTONS | R_CBUTTONS, D_CBUTTONS | R_CBUTTONS, D_CBUTTONS | L_CBUTTONS, 0xFFFF,
};
u16 gBattleSeqCannon3[] = {
    U_CBUTTONS,
    U_CBUTTONS | R_CBUTTONS,
    R_CBUTTONS,
    D_CBUTTONS | R_CBUTTONS,
    D_CBUTTONS,
    D_CBUTTONS | L_CBUTTONS,
    L_CBUTTONS,
    0xFFFF,
};
u16 gBattleSeqCannonUnused[] = {
    U_CBUTTONS,
    U_CBUTTONS | R_CBUTTONS,
    U_CBUTTONS | D_CBUTTONS | R_CBUTTONS,
    D_CBUTTONS | L_CBUTTONS,
    L_CBUTTONS,
    0xFFFF,
};
u16 gBattleSeqCannon4[] = { D_CBUTTONS, D_CBUTTONS, 0xFFFF }; /* a fifth cannon */
u16 gBattleSeqMegaCannon[] = {
    U_CBUTTONS, R_CBUTTONS, D_CBUTTONS, L_CBUTTONS, U_CBUTTONS, R_CBUTTONS, D_CBUTTONS, L_CBUTTONS, 0xFFFF,
}; /* the MEGA cannon (at low armour) */
u16 gBattleSeqSkyKick[] = { D_CBUTTONS, D_CBUTTONS, 0xFFFF }; /* the sky kick */
u16 gBattleSeqBB[] = { B_BUTTON, B_BUTTON, 0xFFFF };
u16 gBattleSeqCRightCRight[] = { R_CBUTTONS, R_CBUTTONS, 0xFFFF };
u16 gBattleSeqCLeftCLeft[] = { L_CBUTTONS, L_CBUTTONS, 0xFFFF };
u16 gBattleSeqBBB[] = { B_BUTTON, B_BUTTON, B_BUTTON, 0xFFFF };
u16 gBattleSeqKonamiCode[] = {
    U_CBUTTONS, U_CBUTTONS, D_CBUTTONS, D_CBUTTONS, L_CBUTTONS, R_CBUTTONS, L_CBUTTONS, R_CBUTTONS,
    B_BUTTON,   A_BUTTON,   0xFFFF,
}; /* up up down down left right left right B A */

/* Render setups. */
Gfx gBossFogLitTexShadeDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsDPPipelineMode(G_PM_NPRIMITIVE),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetCombineKey(G_CK_NONE),
    gsDPSetTextureLOD(G_TL_TILE),
    gsDPSetTextureDetail(G_TD_CLAMP),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsDPSetTextureLUT(G_TT_NONE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                          G_TEXTURE_GEN_LINEAR | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_TEX_EDGE2),
    gsDPSetCombineLERP(TEXEL0, 0, SHADE, 0, 0, 0, 0, TEXEL0, ENVIRONMENT, 0, COMBINED, 0, 0, 0, 0, COMBINED),
    gsDPSetPrimColor(0, 0, 255, 255, 255, 255),
    gsDPSetEnvColor(255, 255, 255, 255),
    gsDPSetFogColor(16, 16, 48, 0),
    gsSPFogPosition(995, 1000),
    gsSPEndDisplayList(),
};

Gfx gBossFogLitTexShade2Dl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsDPPipelineMode(G_PM_NPRIMITIVE),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetCombineKey(G_CK_NONE),
    gsDPSetTextureLOD(G_TL_TILE),
    gsDPSetTextureDetail(G_TD_CLAMP),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsDPSetTextureLUT(G_TT_NONE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                          G_TEXTURE_GEN_LINEAR | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
    gsDPSetAlphaCompare(G_AC_NONE),
    /* G_RM_AA_ZB_TEX_EDGE2 without IM_RD */
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, AA_EN | Z_CMP | Z_UPD | CVG_DST_CLAMP | CVG_X_ALPHA | ALPHA_CVG_SEL |
                                            ZMODE_OPA | GBL_c2(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)),
    gsDPSetCombineLERP(TEXEL0, 0, SHADE, 0, 0, 0, 0, TEXEL0, ENVIRONMENT, 0, COMBINED, 0, 0, 0, 0, COMBINED),
    gsDPSetEnvColor(0, 255, 255, 255),
    gsDPSetFogColor(0, 0, 0, 0),
    gsSPFogPosition(990, 1000),
    gsSPEndDisplayList(),
};

/* The cockpit model setups (cockpit.c, gFrmLayers[2] and [3]); bmove.c draws the first too. */
Gfx gBossLitTexEnvDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetColorDither(G_CD_DISABLE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                          G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_TEX_EDGE2),
    gsDPSetCombineLERP(ENVIRONMENT, TEXEL0, ENV_ALPHA, TEXEL0, 0, 0, 0, TEXEL0, COMBINED, 0, SHADE, 0, 0, 0, 0,
                       COMBINED),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsSPEndDisplayList(),
};

Gfx gBattleStageSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsDPPipelineMode(G_PM_NPRIMITIVE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                          G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
    gsDPSetCombineMode(G_CC_MODULATEIDECALA, G_CC_PASS2),
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_TEX_EDGE2),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsSPEndDisplayList(),
};

Gfx gBossUnlitTexEnvDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetColorDither(G_CD_DISABLE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                          G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_TEX_EDGE2),
    gsDPSetCombineLERP(ENVIRONMENT, TEXEL0, ENV_ALPHA, TEXEL0, 0, 0, 0, TEXEL0, COMBINED, 0, SHADE, 0, 0, 0, 0,
                       COMBINED),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsSPEndDisplayList(),
};

Gfx gBossDitherAlphaDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetColorDither(G_CD_DISABLE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                          G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    /* no named mode: an opaque blend that reads memory, with G_AC_DITHER (3) in the low bits */
    gsDPSetRenderMode(AA_EN | Z_CMP | Z_UPD | IM_RD | CVG_DST_CLAMP | ZMODE_OPA |
                          GBL_c1(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA),
                      GBL_c2(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA) | G_AC_DITHER),
    gsDPSetAlphaCompare(G_AC_DITHER),
    gsDPPipeSync(),
    gsDPSetBlendColor(0, 0, 0, 1),
    gsDPSetEnvColor(0, 0, 0, 64),
    gsSPEndDisplayList(),
};

/* The colour setups bmove.c draws. */
Gfx gBossPrimEnvDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                          G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetCombineLERP(PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0, PRIMITIVE,
                       ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsDPPipeSync(),
    gsDPSetBlendColor(0, 0, 0, 1),
    gsSPEndDisplayList(),
};

Gfx gBossEnvDl[] = {
    gsDPPipeSync(),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, ENVIRONMENT, 0, TEXEL0, 0, 0, 0, 0, ENVIRONMENT, ENVIRONMENT, 0, TEXEL0,
                       0),
    gsSPEndDisplayList(),
};

Gfx gBossXluEnvDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetRenderMode(G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2),
    gsDPSetCombineLERP(NOISE, 0, TEXEL0, 0, 0, 0, 0, ENVIRONMENT, NOISE, 0, TEXEL0, 0, 0, 0, 0, ENVIRONMENT),
    gsDPSetAlphaCompare(G_AC_DITHER),
    gsDPPipeSync(),
    gsDPSetBlendColor(0, 0, 0, 1),
    gsDPSetColorDither(G_CD_DISABLE),
    gsSPEndDisplayList(),
};

Gfx gBossXluPrimEnvDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetRenderMode(G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2),
    gsDPSetCombineLERP(1, 0, ENVIRONMENT, PRIMITIVE, 0, 0, 0, ENVIRONMENT, 1, 0, ENVIRONMENT, PRIMITIVE, 0, 0, 0,
                       ENVIRONMENT),
    gsDPSetPrimColor(0, 0, 0, 0, 0, 255),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsDPPipeSync(),
    gsDPSetBlendColor(0, 0, 0, 1),
    gsDPSetColorDither(G_CD_DISABLE),
    gsSPEndDisplayList(),
};

Gfx gBossFogTexShadeDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsDPPipelineMode(G_PM_NPRIMITIVE),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetCombineKey(G_CK_NONE),
    gsDPSetTextureLOD(G_TL_TILE),
    gsDPSetTextureDetail(G_TD_CLAMP),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsDPSetTextureLUT(G_TT_NONE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                          G_TEXTURE_GEN_LINEAR | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_FOG | G_SHADING_SMOOTH),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_TEX_EDGE2),
    gsDPSetCombineLERP(TEXEL0, 0, SHADE, 0, 0, 0, 0, TEXEL0, ENVIRONMENT, 0, COMBINED, 0, 0, 0, 0, COMBINED),
    gsDPSetEnvColor(0, 255, 255, 255),
    gsDPPipeSync(),
    gsDPSetFogColor(0, 0, 0, 0),
    gsSPFogPosition(990, 1000),
    gsSPEndDisplayList(),
};

/* bossFogTexEnvColorDL calls it, then sets the env colour */
Gfx gBossFogTexEnvDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsDPPipelineMode(G_PM_NPRIMITIVE),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetCombineKey(G_CK_NONE),
    gsDPSetTextureLOD(G_TL_TILE),
    gsDPSetTextureDetail(G_TD_CLAMP),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsDPSetTextureLUT(G_TT_NONE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                          G_TEXTURE_GEN_LINEAR | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_FOG | G_SHADING_SMOOTH),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_TEX_EDGE2),
    gsDPSetCombineLERP(ENVIRONMENT, TEXEL0, ENV_ALPHA, TEXEL0, 0, 0, 0, TEXEL0, COMBINED, 0, SHADE, 0, 0, 0, 0,
                       COMBINED),
    gsSPEndDisplayList(),
};

Gfx gBossDepthOnlyDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR |
                          G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_CULL_BACK),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT),
    gsDPSetEnvColor(0, 0, 0, 0),
    gsDPSetRenderMode(G_RM_ZB_XLU_SURF | Z_UPD, G_RM_ZB_XLU_SURF2 | Z_UPD),
    gsSPEndDisplayList(),
};

Gfx gBossFogEnvDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                          G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
    /* G_RM_AA_ZB_TEX_EDGE2 without IM_RD */
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, AA_EN | Z_CMP | Z_UPD | CVG_DST_CLAMP | CVG_X_ALPHA | ALPHA_CVG_SEL |
                                            ZMODE_OPA | GBL_c2(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)),
    gsDPSetCombineLERP(ENVIRONMENT, TEXEL0, ENV_ALPHA, TEXEL0, 0, 0, 0, TEXEL0, COMBINED, 0, SHADE, 0, 0, 0, 0,
                       COMBINED),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsSPEndDisplayList(),
};

/* Sets the velocity to reach (x, y, z) at speed (twice over); 0, and the object put on the
 * point, when it would pass it. BmoveX2's C plus a compiled-out print between w and the
 * distance (its dead branch keeps w in a register); the int 2 is a local, pad a frame word. */
s32 BmoveX(Obj *obj, f32 x, f32 y, f32 z, f32 speed) {
    BObj *w = BOBJ(obj);
    s32 pad;
    f32 d;
    s32 n = 2;

    PRINTF("[BmoveX]相対距離０です。\n");
    d = sqrtf((z - w->frame->pos[2]) * (z - w->frame->pos[2]) +
              ((y - w->frame->pos[1]) * (y - w->frame->pos[1]) +
               (x - w->frame->pos[0]) * (x - w->frame->pos[0])));
    if (d * speed == 0.0f) {
        DPRINTF("[BmoveX]相対距離０です。\n");
        w->vel[0] = w->vel[1] = w->vel[2] = 0.0f;
        return 0;
    }
    w->vel[0] = (x - w->frame->pos[0]) / d * speed * n;
    w->vel[1] = (y - w->frame->pos[1]) / d * speed * n;
    w->vel[2] = (z - w->frame->pos[2]) / d * speed * n;
    if ((x - (w->frame->pos[0] + w->vel[0] * n)) * (x - w->frame->pos[0]) < 0.0f ||
        (y - (w->frame->pos[1] + w->vel[1] * n)) * (y - w->frame->pos[1]) < 0.0f ||
        (z - (w->frame->pos[2] + w->vel[2] * n)) * (z - w->frame->pos[2]) < 0.0f) {
        w->frame->pos[0] = x;
        w->frame->pos[1] = y;
        w->frame->pos[2] = z;
        w->vel[2] = w->vel[1] = w->vel[0] = 0.0f;
        return 0;
    }
    return 1;
}

/* The same (written with dz first in the distance and the pass test's factors the other way
 * round, as the original's operand order shows). */
s32 BmoveX2(Obj *obj, f32 x, f32 y, f32 z, f32 speed) {
    BObj *w = BOBJ(obj);
    f32 d = sqrtf((z - w->frame->pos[2]) * (z - w->frame->pos[2]) +
                  ((y - w->frame->pos[1]) * (y - w->frame->pos[1]) +
                   (x - w->frame->pos[0]) * (x - w->frame->pos[0])));

    if (d * speed == 0.0f) {
        w->vel[0] = w->vel[1] = w->vel[2] = 0.0f;
        return 0;
    }
    w->vel[0] = (x - w->frame->pos[0]) / d * speed * 2;
    w->vel[1] = (y - w->frame->pos[1]) / d * speed * 2;
    w->vel[2] = (z - w->frame->pos[2]) / d * speed * 2;
    if ((x - (w->frame->pos[0] + w->vel[0] * 2)) * (x - w->frame->pos[0]) < 0.0f ||
        (y - (w->frame->pos[1] + w->vel[1] * 2)) * (y - w->frame->pos[1]) < 0.0f ||
        (z - (w->frame->pos[2] + w->vel[2] * 2)) * (z - w->frame->pos[2]) < 0.0f) {
        w->frame->pos[0] = x;
        w->frame->pos[1] = y;
        w->frame->pos[2] = z;
        w->vel[2] = w->vel[1] = w->vel[0] = 0.0f;
        return 0;
    }
    return 1;
}

/* Whether a controller's buttons, looking back from `back` frames ago, went through the
 * sequence seq (ended by 0xFFFF): mode 0 by buttons held then let go, mode 1 by exact presses. */
s32 bossCheckButtonSeq(s32 port, u16 *seq, s32 back, s32 mode) {
    s32 held = FALSE;
    s32 i;
    u16 b;

    gBattleSeqDone = 0;
    if (mode == 0) {
        for (i = back; i >= 0; i--) {
            b = joyGetButton(port, i);
            if (!held) {
                if ((b & *seq) == *seq) {
                    held = TRUE;
                }
            } else {
                if ((b & *seq) != *seq) {
                    seq++;
                    held = FALSE;
                }
                if (gBattleSeqDone == 1) {
                    return 0;
                }
                if (*seq == 0xFFFF) {
                    gBattleSeqDone = 1;
                    return 1;
                }
            }
        }
    }
    if (mode == 1) {
        for (i = back; i >= 0; i--) {
            if (joyGetButton(port, i) == *seq) {
                seq++;
            }
            if (gBattleSeqDone == 1) {
                return 0;
            }
            if (*seq == 0xFFFF) {
                gBattleSeqDone = 1;
                return 1;
            }
        }
    }
    return 0;
}

/* Slows the velocity by a factor; 0, and stopped, once every part is within 0.3. */
s32 bossDampVel(Obj *obj, f32 k) {
    BObj *w = BOBJ(obj);

    BOBJ(obj)->vel[0] *= k;
    BOBJ(obj)->vel[1] *= k;
    BOBJ(obj)->vel[2] *= k;
    if (fabsf(BOBJ(obj)->vel[0]) <= 0.3 && fabsf(w->vel[1]) <= 0.3 && fabsf(w->vel[2]) <= 0.3) {
        w->vel[0] = w->vel[1] = w->vel[2] = 0.0f;
        return 0;
    }
    return 1;
}

/* A "pol" frame's second display list and segments (file set n). */
void bossSetPolDL(PolFrame *f, Gfx *dl, s32 n) {
    f->dl2 = dl;
    floadGetSetRam((void **) f->segs, n);
}

/* An "anime" frame's animation, from its start or (at 1) its end. */
void bossSetAnim(AnimeFrame *f, u32 anim, s32 atEnd) {
    f->anim = anim;
    if (atEnd == 1) {
        f->frame = polAnimeLastFrame(f);
        return;
    }
    f->frame = 0.0f;
}

/* An "anime" frame's animation, segments, and frame at a fraction of the way through. */
void bossSetAnimAt(AnimeFrame *f, void *anim, s32 n, f32 at) {
    f->anim = anim;
    floadGetSetRam((void **) f->segs, n);
    f->frame = polAnimeLastFrame(f) * at;
}

/* Steps an "anime" frame's animation (demotool.c's demoAnimeStep). */
s32 bossStepAnim(AnimeFrame *f, f32 speed, s32 mode) {
    if (mode == 0 || mode == 2) {
        f->frame += speed;
        if (polAnimeLastFrame(f) < f->frame) {
            if (mode == 0) {
                f->frame -= polAnimeLastFrame(f);
            } else {
                f->frame = polAnimeLastFrame(f);
            }
            return 1;
        }
    } else if (mode == 1 || mode == 3) {
        f->frame -= speed;
        if (f->frame < 0.0f) {
            if (mode == 1) {
                f->frame += polAnimeLastFrame(f);
            } else {
                f->frame = 0.0f;
            }
            return 1;
        }
    }
    return 0;
}

/* The same, keeping a box's animated attack chain (its model, an "aniAtk" frame) at the same
 * time. */
s32 bossStepAnimAtk(AnimeFrame *f, ACollFrame *g, f32 speed, s32 mode) {
    if (mode == 0 || mode == 2) {
        f->frame += speed;
        if (polAnimeLastFrame(f) < f->frame) {
            if (mode == 0) {
                f->frame -= polAnimeLastFrame(f);
                ((AniAtkFrame *) g->model)->time -= polAnimeLastFrame(f);
            } else {
                ((AniAtkFrame *) g->model)->time = f->frame = polAnimeLastFrame(f);
            }
            return 1;
        }
        ((AniAtkFrame *) g->model)->time = f->frame;
    } else if (mode == 1 || mode == 3) {
        f->frame -= speed;
        if (((AniAtkFrame *) g->model)->time < 0.0f) {
            ((AniAtkFrame *) g->model)->time = 0.0f;
        }
        if (f->frame < 0.0f) {
            if (mode == 1) {
                f->frame += polAnimeLastFrame(f);
                ((AniAtkFrame *) g->model)->time += polAnimeLastFrame(f);
            } else {
                f->frame = 0.0f;
                ((AniAtkFrame *) g->model)->time = 0.0f;
            }
            return 1;
        }
        ((AniAtkFrame *) g->model)->time = f->frame;
    }
    return 0;
}

/* Moves the object by its velocity. */
void bossApplyVel(Obj *obj) {
    BOBJ(obj)->frame->pos[0] += BOBJ(obj)->vel[0];
    BOBJ(obj)->frame->pos[1] += BOBJ(obj)->vel[1];
    BOBJ(obj)->frame->pos[2] += BOBJ(obj)->vel[2];
}

/* The same. */
void bossApplyVel2(Obj *obj) {
    BOBJ(obj)->frame->pos[0] += BOBJ(obj)->vel[0];
    BOBJ(obj)->frame->pos[1] += BOBJ(obj)->vel[1];
    BOBJ(obj)->frame->pos[2] += BOBJ(obj)->vel[2];
}

/* The same for an "mpol" frame: its matrix moves along its own axes. */
void bossApplyVelMpol(Obj *obj) {
    mathMtxPreTrans(((MPolFrame *) BOBJ(obj)->frame)->mtx.m, BOBJ(obj)->vel[0], BOBJ(obj)->vel[1],
                           BOBJ(obj)->vel[2]);
}

/* Tilts the object with its velocity: k degrees per unit. */
void bossTiltByVel(Obj *obj, f32 k) {
    BOBJ(obj)->frame->rot[0] = BOBJ(obj)->vel[2] * 0.0174532930038800785 * k;
    BOBJ(obj)->frame->rot[2] = -BOBJ(obj)->vel[0] * 0.0174532930038800785 * k;
}

/* The same. */
void bossTiltByVel2(Obj *obj, f32 k) {
    BOBJ(obj)->frame->rot[0] = BOBJ(obj)->vel[2] * 0.0174532930038800785 * k;
    BOBJ(obj)->frame->rot[2] = -BOBJ(obj)->vel[0] * 0.0174532930038800785 * k;
}

void bossNop(Obj *obj, s32 a) {
}

/* Turns a frame toward (x, z) by up to step degrees; 1 once it faces it. */
s32 bossTurnToward(AnimeFrame *f, f32 x, f32 z, f32 step) {
    f32 target = mathAtan2(x - f->pos[0], z - f->pos[2]);

    if (target - f->rot[1] > 3.1415927f) {
        f->rot[1] += 6.2831855f;
    } else if (target - f->rot[1] < -3.1415927f) {
        f->rot[1] -= 6.2831855f;
    }
    if (fabsf(target - f->rot[1]) < step * 0.0174532930038800785) {
        f->rot[1] = target;
        return TRUE;
    } else if (target < f->rot[1]) {
        f->rot[1] = f->rot[1] - step * 0.0174532930038800785;
    } else {
        f->rot[1] = f->rot[1] + step * 0.0174532930038800785;
    }
    return FALSE;
}

/* The same (bossTurnToward's twin, instruction for instruction). */
s32 bossTurnToward2(AnimeFrame *f, f32 x, f32 z, f32 step) {
    f32 target = mathAtan2(x - f->pos[0], z - f->pos[2]);

    if (target - f->rot[1] > 3.1415927f) {
        f->rot[1] += 6.2831855f;
    } else if (target - f->rot[1] < -3.1415927f) {
        f->rot[1] -= 6.2831855f;
    }
    if (fabsf(target - f->rot[1]) < step * 0.0174532930038800785) {
        f->rot[1] = target;
        return TRUE;
    } else if (target < f->rot[1]) {
        f->rot[1] = f->rot[1] - step * 0.0174532930038800785;
    } else {
        f->rot[1] = f->rot[1] + step * 0.0174532930038800785;
    }
    return FALSE;
}

/* The point dist ahead and side to the right of a heading. */
void bossHeadingOffset(f32 angle, f32 side, f32 dist, f32 *x, f32 *z) {
    f32 t;

    t = cosf(angle);
    *x = sinf(angle) * dist + side * t;
    t = sinf(angle);
    *z = cosf(angle) * dist - side * t;
}

/* The squared distance between (x1, z1) and (x2, z2). */
f32 bossDistSqXZ(f32 x1, f32 z1, f32 x2, f32 z2) {
    return (x1 - x2) * (x1 - x2) + (z1 - z2) * (z1 - z2);
}

/* The squared distance between two points. */
f32 bossDistSq(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2) {
    return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) + (z1 - z2) * (z1 - z2);
}

/* Gravity: the fall speed grows by g, to 5000 at most. */
s32 bossGravity(Obj *obj, f32 g) {
    BOBJ(obj)->vel[1] -= g;
    if (BOBJ(obj)->vel[1] < -5000.0) {
        BOBJ(obj)->vel[1] = -5000.0f;
    }
    return 0;
}

/* The same. */
s32 bossGravity2(Obj *obj, f32 g) {
    BOBJ(obj)->vel[1] -= g;
    if (BOBJ(obj)->vel[1] < -5000.0) {
        BOBJ(obj)->vel[1] = -5000.0f;
    }
    return 0;
}

void bossStopFall(Obj *obj) {
    BOBJ(obj)->vel[1] = 0.0f;
}

/* The Impact is hit: by `who`, and the gauge fills by fill (to 100). */
void bossImpactHit(s32 who, s32 a, s32 fill) {
    gBattleFlags |= 1;
    gImpactHitDelay = 6;
    gCockpitShake = 6;
    gImpactHitDamage = who;
    gImpactPower += fill;
    if (gImpactPower > 100) {
        gImpactPower = 100;
    }
}

/* The Impact takes damage: from one life or the other by flag 0x4000000. */
void bossImpactDamage(s32 damage, s32 a, s32 fill) {
    gBrotherFlags |= 0x20;
    if (gBattleFlags & 0x4000000) {
        gImpactArmor -= damage;
    } else {
        gImpactArmor2 -= damage;
    }
    gBrotherDownTime = 120;
    if (gImpactArmor <= 0) {
        gImpactArmor = 0;
    }
    if (gImpactArmor2 <= 0) {
        gImpactArmor2 = 0;
    }
    gImpactPower += fill;
    if (gImpactPower > 100) {
        gImpactPower = 100;
    }
}

/* The boss takes damage. */
void bossDamage(s32 damage, s32 a, s32 fill) {
    gBossLife -= damage;
    gBossFlags |= 0x100;
    if (gBossLife <= 0) {
        gBossLife = 0;
    }
    gImpactPower += fill;
    if (gImpactPower > 100) {
        gImpactPower = 100;
    }
}

/* Adds to the 0-999 gauge. */
void bossAddMoney(s32 n) {
    gImpactMoney += n;
    if (gImpactMoney >= 1000) {
        gImpactMoney = 999;
    }
    if (gImpactMoney < 0) {
        gImpactMoney = 0;
    }
}

/* Adds to the other gauge, to 999. */
void bossAddPower(s32 n) {
    gImpactPower += n;
    if (gImpactPower >= 1000) {
        gImpactPower = 999;
    }
    if (gImpactPower < 0) {
        gImpactPower = 0;
    }
}

/* Bobbing: the y speed swings with the frame count. */
void bossBobAccel(Obj *obj, f32 amp, f32 speed) {
    BObj *w = BOBJ(obj);

    w->vel[1] += amp * cosf(gGameFrameCount * 0.0174532930038800785 * speed);
}

/* Halves the x and z speeds outside [xmin, xmax] and [zmin, zmax]. */
void bossMoveLimitXZ(Obj *obj, f32 xmin, f32 xmax, f32 zmin, f32 zmax) {
    BObj *w = BOBJ(obj);

    if (BOBJ(obj)->frame->pos[0] < xmin || xmax < w->frame->pos[0]) {
        DPRINTF("[bossMoveLimit]...X\n");
        w->vel[0] *= 0.5;
        if (w->vel[0] < 0.01) {
            w->vel[0] = 0.0f;
        }
    }
    if (w->frame->pos[2] < zmin || zmax < w->frame->pos[2]) {
        DPRINTF("[bossMoveLimit]...Z\n");
        w->vel[2] *= 0.5;
        if (w->vel[2] < 0.01) {
            w->vel[2] = 0.0f;
        }
    }
}

/* The same for y. */
void bossMoveLimitY(Obj *obj, f32 ymin, f32 ymax) {
    BObj *w = BOBJ(obj);

    if (BOBJ(obj)->frame->pos[1] < ymin || ymax < w->frame->pos[1]) {
        DPRINTF("[bossMoveLimit]...Y\n");
        w->vel[1] *= 0.5;
        if (w->vel[1] < 0.01) {
            w->vel[1] = 0.0f;
        }
    }
}

void bossAddFlinchCount(s32 n) {
    gBossFlinchCount += n;
    if (gBossFlinchCount < 0) {
        gBossFlinchCount = 0;
    }
    DPRINTF("のけぞりダメージカウント[%d]\n", gBossFlinchCount);
}

/* 1, and the counter reset, once it reaches n. */
s32 bossFlinchCountOver(s32 n) {
    if (gBossFlinchCount >= n) {
        DPRINTF("のけぞり回数オーバー[%d/%d]。回避します。\n", gBossFlinchCount, n);
        gBossFlinchCount = 0;
        return 1;
    }
    return 0;
}

/* Starts an action. */
void bossStartAction(Obj *obj, s32 a, s32 action) {
    gBossActionArg = a;
    gBossActionCount++;
    BACTOR(obj)->action = action;
    BACTOR(obj)->step = 0;
}

void bossClearAction(Obj *obj) {
    BACTOR(obj)->action = 0;
    BACTOR(obj)->step = 0;
}

void bossSetFrameFlags(AnimeFrame *f, s32 flags) {
    f->flags |= flags;
}

void bossSetFrameFlags2(AnimeFrame *f, s32 flags) {
    f->flags |= flags;
}

void bossSetMpolFlags(MPolFrame *f, s32 flags) {
    f->flags |= flags;
}

/* Display lists (returned) of a setup in a primitive and an environment colour. */
Gfx *bossPrimEnvColorDL(s32 r, s32 g, s32 b, s32 er, s32 eg, s32 eb, s32 ea) {
    Gfx *dl = gGfxDlHead;

    gSPDisplayList(gGfxDlHead++, gBossPrimEnvDl);
    gDPSetPrimColor(gGfxDlHead++, 0, 0, r, g, b, 255);
    gDPSetEnvColor(gGfxDlHead++, er, eg, eb, ea);
    gSPEndDisplayList(gGfxDlHead++);
    return dl;
}

Gfx *bossEnvColorDL(s32 r, s32 g, s32 b, s32 a) {
    Gfx *dl = gGfxDlHead;

    gSPDisplayList(gGfxDlHead++, gBossEnvDl);
    gDPSetEnvColor(gGfxDlHead++, r, g, b, a);
    gSPEndDisplayList(gGfxDlHead++);
    return dl;
}

Gfx *bossXluEnvColorDL(s32 r, s32 g, s32 b, s32 a) {
    Gfx *dl = gGfxDlHead;

    gSPDisplayList(gGfxDlHead++, gBossXluEnvDl);
    gDPSetEnvColor(gGfxDlHead++, r, g, b, a);
    gSPEndDisplayList(gGfxDlHead++);
    return dl;
}

Gfx *bossXluPrimEnvColorDL(s32 r, s32 g, s32 b, s32 a) {
    Gfx *dl = gGfxDlHead;

    gSPDisplayList(gGfxDlHead++, gBossXluPrimEnvDl);
    gDPSetEnvColor(gGfxDlHead++, r, g, b, a);
    gSPEndDisplayList(gGfxDlHead++);
    return dl;
}

Gfx *bossLitTexEnvColorDL(s32 r, s32 g, s32 b, s32 a) {
    Gfx *dl = gGfxDlHead;

    gSPDisplayList(gGfxDlHead++, gBossLitTexEnvDl);
    gDPSetEnvColor(gGfxDlHead++, r, g, b, a);
    gSPEndDisplayList(gGfxDlHead++);
    return dl;
}

Gfx *bossCombineEnvColorDL(s32 r, s32 g, s32 b, s32 a) {
    Gfx *dl = gGfxDlHead;

    gSPDisplayList(gGfxDlHead++, gDrawSetupXluSurfDl);
    gDPSetEnvColor(gGfxDlHead++, r, g, b, a);
    gDPSetCombine(gGfxDlHead++, 0x629AC5, 0xFF34FE7F);
    gSPEndDisplayList(gGfxDlHead++);
    return dl;
}

/* The same with fog: its colour and its start (to 1000). */
Gfx *bossFogEnvColorDL(s32 r, s32 g, s32 b, s32 a, s32 fr, s32 fg, s32 fb, s32 fa, s32 near) {
    Gfx *dl = gGfxDlHead;

    gSPDisplayList(gGfxDlHead++, gBossFogEnvDl);
    gDPSetEnvColor(gGfxDlHead++, r, g, b, a);
    gDPSetFogColor(gGfxDlHead++, fr, fg, fb, fa);
    gSPFogPosition(gGfxDlHead++, near, 1000);
    gSPEndDisplayList(gGfxDlHead++);
    return dl;
}

/* Bobbing, as a y speed. */
void bossBobVel(Obj *obj, f32 amp, f32 speed) {
    BOBJ(obj)->vel[1] = cosf(gGameFrameCount % 360 * 0.0174532930038800785 * speed) * amp;
}

/* Bobbing, moving a frame's y. */
void bossBobFrame(AnimeFrame *f, f32 amp, f32 speed) {
    f->pos[1] += amp * cosf(gGameFrameCount % 360 * 0.0174532930038800785 * speed);
}

/* Flies toward (x, y, z): speeds capped at 20 (40 against boss 5), accelerating by accel (y by
 * half) until within 100 (300) of it, then slowing; 1 once within on all three axes. */
/* (Matched 2026-10-03; reworked 2026-10-04. The work pointer w first, as in the file's
 * twins: the game's v1 temp, spilled at 0x18. The direction is three scalars nx, ny, nz,
 * declared in that order (homes 0x28, 0x24, 0x20) and divided in natural order; the block
 * boundary after the divides makes them live into the next block, so uopt colours nx f18
 * (saved at 0x28 across the call) and leaves ny and nz in their homes, as the game does (with
 * no boundary they keep registers: 194 lines). The flags are set by the chain
 * "iz = iy = ix = FALSE": their live ranges are then numbered ix, iy, iz, and their equal
 * priorities colour them a1, a2, a3 as the game; the old chain "ix = iz = iy" needed an empty
 * "if (d) {}" to part the priorities (and the direction as a reversed array). Their slots
 * follow the declaration order ix, iy, iz. "return TRUE" in the test and "return FALSE" at
 * the end: IDO hoists v0 = 0 to the y block's head itself. "there" is unused; its word is the
 * game's top slot.
 * The `if (1) {}` (a do-while(0) until 2026-10-17: the same two blocks, object identical, a kept
 * FAKE) stands for a test of d whose body compiled to nothing, between the
 * divides and the first atan2 call. uopt must see two boundaries with an empty block between
 * them: near (8 references) then falls below nx's priority 1.0, nx takes f18 and near is split
 * (0x40 in the then branch, f18 in the else branch), as the game. Measured (uopt trace and
 * objdump diff lines, measured directly): "if (d == 0.0f) {}" in its place gives
 * MATCH (check); so do the game's zero guard "if (d == 0.0f) { d = 1.0f; }" (enecommon.c
 * eneSetVelToward) written after the divides, where it is dead, a print inside such an if,
 * and an if/else of dead stores. One boundary only (a compiled-out PRINTF, "if (0) {}", or
 * uopt's 20-read block end) gives 150 lines: near and nx tie at 1.0 and near, the lower
 * range, takes f18. A test that reads nx, accel or near moves those priorities (155, 102,
 * 150 lines); the zero guard before the divides gives 201. The body was not a print with a
 * literal: the only string between bossFlinchCountOver's and vramCheck's is "was swapped!"
 * (0x8019CD6C), which fits the flag test of bossStartActionIfFlag.) */
s32 bossFlyToward(Obj *obj, f32 x, f32 y, f32 z, f32 accel) {
    s32 there; /* unused: without it every slot sits 4 high */
    f32 d;
    f32 max;
    f32 near;
    u8 ix;
    u8 iy;
    u8 iz;
    f32 dx;
    f32 dy;
    f32 dz;
    BObj *w = BOBJ(obj);
    f32 nx;
    f32 ny;
    f32 nz;

    iz = iy = ix = FALSE;
    if (gBattleBossId == 5) {
        max = 40.0f;
        near = 300.0f;
    } else {
        max = 20.0f;
        near = 100.0f;
    }
    dx = x - w->frame->pos[0];
    dy = y - w->frame->pos[1];
    dz = z - w->frame->pos[2];
    d = sqrtf(dx * dx + dy * dy + dz * dz);
    nx = dx / d;
    ny = dy / d;
    nz = dz / d;
    if (1) {} /* FAKE: OoT's `if (1) {}`, a block boundary (splits): max spilled across the calls */
    mathAtan2(ny, sqrtf(nx * nx + nz * nz));
    mathAtan2(nx, nz);
    if (max < w->vel[0]) {
        w->vel[0] = max;
    } else if (w->vel[0] < -max) {
        w->vel[0] = -max;
    }
    if (max < w->vel[1]) {
        w->vel[1] = max;
    } else if (w->vel[1] < -max) {
        w->vel[1] = -max;
    }
    if (max < w->vel[2]) {
        w->vel[2] = max;
    } else if (w->vel[2] < -max) {
        w->vel[2] = -max;
    }
    if (w->frame->pos[0] < x - near) {
        w->vel[0] += accel;
    } else if (x + near < w->frame->pos[0]) {
        w->vel[0] -= accel;
    } else {
        ix = TRUE;
        w->vel[0] *= 0.95;
    }
    if (w->frame->pos[1] < y - near) {
        w->vel[1] += accel / 2;
    } else if (y + near < w->frame->pos[1]) {
        w->vel[1] -= accel / 2;
    } else {
        iy = TRUE;
        w->vel[1] *= 0.95;
    }
    if (w->frame->pos[2] < z - near) {
        w->vel[2] += accel;
    } else if (z + near < w->frame->pos[2]) {
        w->vel[2] -= accel;
    } else {
        iz = TRUE;
        w->vel[2] *= 0.95;
    }
    if (ix && iy && iz) {
        return TRUE;
    }
    return FALSE;
}

/* Starts an action while flag 0x4000 is on. */
void bossStartActionIfFlag(Obj *obj, s32 action) {
    if (gBrotherFlags & 0x4000) {
        DPRINTF("交代された！\n");
        (*bossStartAction)(obj, 0, action);
    }
}

/* Clears the battle's state. */
void bossClearState(void) {
    gBattleUnused2 = 0;
    gBossFlinchCount = 0;
    gMegaShotPosX = 0.0f;
    gMegaShotPosY = 0.0f;
    gMegaShotPosZ = 0.0f;
    gMegaBurstY = 0.0f;
    gBattleFlags = 0;
    gBrotherFlags = 0;
    gBossFlags = 0;
    gBattleSndToggles = 0;
    gBattleFlags2 = 0;
    gCockpitLookAtX = 0.0f;
    gCockpitLookAtY = 0.0f;
    gCockpitLookAtZ = 0.0f;
    gImpactPower = 0;
    gBrotherMoveTime = 0;
    gBrotherMove = 0;
    gBattleUnused3 = 1;
    gImpactMoney = 500;
    gImpactArmor = 100;
    gImpactArmor2 = 100;
    gImpactPosX = 0.0f;
    gImpactPosY = 0.0f;
    gImpactPosZ = 0.0f;
    gBrotherPosX = 0.0f;
    gBrotherPosY = 0.0f;
    gBrotherPosZ = 0.0f;
    gBrotherYaw = 0.0f;
    gCockpitRotX = 0.0f;
    gCockpitRotY = 0.0f;
    gCockpitRotZ = 0.0f;
    gImpactHitDelay = 0;
    gBrotherDownTime = 0;
    gBossLifeMax = 0;
    gBossLife = 0;
    gBattleUnused5 = 0;
    gImpactHitDamage = 0;
    gBossPosX = 0.0f;
    gBossPosY = 0.0f;
    gBossPosZ = 0.0f;
    gBossYaw = 0.0f;
    gCockpitAimX = 0.0f;
    gCockpitAimY = 0.0f;
    gCockpitAimZ = 0.0f;
    gCockpitHeadPitch = 0.0f;
    gCockpitHeadYaw = 0.0f;
    gImpactMoveTime = 0;
    gBattleUnused10 = 0;
    gBattleSeqDone = 0;
    gBossActionArg = 0;
    gBossPhase = 0;
    gBossActionCount = 0;
    gBossFrameCount = 0;
    gValcanSide = 0;
    gOvalcanSide = 0;
    gBattleUnused11 = 0;
    gCockpitShake = 0;
    gBattleUnused12 = 0;
}

/* Puts a frame on the ground under (x, y, z) (a ray down by `down`), `above` higher. */
void vramCheck(AnimeFrame *f, f32 x, f32 y, f32 z, f32 down, f32 above) {
    Vec3f at;
    Vec3f dir;
    f32 tp;

    dir.x = 0.0f;
    dir.y = down;
    dir.z = 0.0f;
    if ((*vramRayGround)((Vec3f *) &x, &dir, &at, &tp) != NULL) {
        f->pos[1] = at.y + above + 0.1f;
    }
    DPRINTF("[vramCheck] チェック[%f],返り値[%f],補正後[%f]\n");
}

Gfx *bossFogTexEnvColorDL(s32 r, s32 g, s32 b, s32 a) {
    Gfx *dl = gGfxDlHead;

    gSPDisplayList(gGfxDlHead++, gBossFogTexEnvDl);
    gDPSetEnvColor(gGfxDlHead++, r, g, b, a);
    gSPEndDisplayList(gGfxDlHead++);
    return dl;
}

/* The same, 1 if there was ground. */
s32 vramCheckGround(AnimeFrame *f, f32 x, f32 y, f32 z, f32 down, f32 above) {
    Vec3f at;
    Vec3f dir;
    f32 tp;

    dir.x = 0.0f;
    dir.y = down;
    dir.z = 0.0f;
    if ((*vramRayGround)((Vec3f *) &x, &dir, &at, &tp) != NULL) {
        f->pos[1] = at.y + above + 0.1f;
        DPRINTF("[vramCheck] チェック[%f],返り値[%f],補正後[%f]\n");
        return 1;
    }
    return 0;
}

/* Turns a frame toward the direction (dx, dz) by up to step degrees; 1 once it faces it. */
s32 bossTurnToDir(AnimeFrame *f, f32 dx, f32 dz, f32 step) {
    f32 target = mathAtan2(dx, dz);

    if (target - f->rot[1] > 3.1415927f) {
        f->rot[1] += 6.2831855f;
    } else if (target - f->rot[1] < -3.1415927f) {
        f->rot[1] -= 6.2831855f;
    }
    if (fabsf(target - f->rot[1]) < step * 0.0174532930038800785) {
        f->rot[1] = target;
        return TRUE;
    } else if (target < f->rot[1]) {
        f->rot[1] = f->rot[1] - step * 0.0174532930038800785;
    } else {
        f->rot[1] = f->rot[1] + step * 0.0174532930038800785;
    }
    return FALSE;
}

/* Keeps the object within [xmin, xmax] and [zmin, zmax]. */
void bossMoveLimitClampXZ(Obj *obj, f32 xmin, f32 xmax, f32 ymin, f32 ymax, f32 zmin, f32 zmax) {
    BObj *w = BOBJ(obj);

    if (BOBJ(obj)->frame->pos[0] < xmin) {
        DPRINTF("[bossMoveLimit]...X-\n");
        w->frame->pos[0] = xmin;
    } else if (xmax < w->frame->pos[0]) {
        DPRINTF("[bossMoveLimit]...X+\n");
        w->frame->pos[0] = xmax;
    }
    if (w->frame->pos[2] < zmin) {
        DPRINTF("[bossMoveLimit]...Z-\n");
        w->frame->pos[2] = zmin;
    } else if (zmax < w->frame->pos[2]) {
        DPRINTF("[bossMoveLimit]...Z+\n");
        w->frame->pos[2] = zmax;
    }
}

/* Starts one of the actions 0x1F-0x27 (kind 1-9). An old-style definition: its s8 parameters
 * are homed as words and stored untruncated (IDO_NOTES 125). */
void bossStartActionKind(obj, kind, a, b)
    Obj *obj;
    s32 kind;
    u8 a;
    u8 b;
{
    BACTOR(obj)->nextAction = a;
    BACTOR(obj)->nextStep = b;
    BACTOR(obj)->step = 0;
    if (kind == 1) {
        BACTOR(obj)->action = 0x1F;
    }
    if (kind == 2) {
        BACTOR(obj)->action = 0x20;
    }
    if (kind == 3) {
        BACTOR(obj)->action = 0x21;
    }
    if (kind == 4) {
        BACTOR(obj)->action = 0x22;
    }
    if (kind == 5) {
        BACTOR(obj)->action = 0x23;
    }
    if (kind == 6) {
        BACTOR(obj)->action = 0x24;
    }
    if (kind == 7) {
        BACTOR(obj)->action = 0x25;
    }
    if (kind == 8) {
        BACTOR(obj)->action = 0x26;
    }
    if (kind == 9) {
        BACTOR(obj)->action = 0x27;
    }
}

/* Flies toward (x, z) on the ground plane: speeds capped at max, accelerating until within 300,
 * then slowing; 1 once within on both axes. */
/* (The work pointer is a local set at the top: IDO rebuilds it from obj's home after the call,
 * reloading obj and adding 0x20, as the game does; set after the call it is folded into the
 * loads. The two flags live on the stack, ix at 0x1F and iz at 0x1D.) */
s32 bossFlyTowardXZ(Obj *obj, f32 x, f32 z, f32 accel, f32 max) {
    f32 dx;
    f32 dz;
    u8 ix;
    u8 pad;
    u8 iz;
    f32 d;
    BObj *w = BOBJ(obj);

    ix = FALSE;
    dx = x - w->frame->pos[0];
    iz = FALSE;
    dz = z - w->frame->pos[2];
    d = sqrtf(dx * dx + dz * dz);
    mathAtan2(dx / d, dz / d);
    if (max < w->vel[0]) {
        w->vel[0] = max;
    } else if (w->vel[0] < -max) {
        w->vel[0] = -max;
    }
    if (max < w->vel[2]) {
        w->vel[2] = max;
    } else if (w->vel[2] < -max) {
        w->vel[2] = -max;
    }
    if (w->frame->pos[0] < x - 300.0f) {
        w->vel[0] += accel;
    } else if (x + 300.0f < w->frame->pos[0]) {
        w->vel[0] -= accel;
    } else {
        ix = TRUE;
        w->vel[0] *= 0.8;
    }
    if (w->frame->pos[2] < z - 300.0f) {
        w->vel[2] += accel;
    } else if (z + 300.0f < w->frame->pos[2]) {
        w->vel[2] -= accel;
    } else {
        iz = TRUE;
        w->vel[2] *= 0.8;
    }
    if (ix && iz) {
        return 1;
    }
    return 0;
}

/* Patrols x between min and max: turns at the ends, keeps going between, and starts at
 * random when still. 1 at an end. (The 1 is set first: "return 1" is shared at the end, and
 * the three cases in between share one "return 0"; a pad word puts the pointer's spill where
 * the game has it.) */
s32 bossPatrolX(Obj *obj, f32 min, f32 max, f32 accel) {
    BObj *w = BOBJ(obj);
    f32 lim;
    s32 pad;

    if (gBattleBossId == 5) {
        lim = 40.0f;
    } else {
        lim = 20.0f;
    }
    if (lim < w->vel[0]) {
        w->vel[0] = lim;
    } else if (w->vel[0] < -lim) {
        w->vel[0] = -lim;
    }
    if (w->frame->pos[0] < min) {
        w->vel[0] += accel;
    } else if (max < w->frame->pos[0]) {
        w->vel[0] -= accel;
    } else {
        if (w->vel[0] > 0.0f) {
            w->vel[0] += accel;
        } else if (w->vel[0] < 0.0f) {
            w->vel[0] -= accel;
        } else {
            w->vel[0] = (fadeRand(100) - 50) * 0.04;
        }
        return 0;
    }
    return 1;
}

/* The same for y (gentler against boss 5: half the step, written `/ 2`, which IDO keeps a
 * division; `/ 2.0f` becomes `* 0.5f`). */
s32 bossPatrolY(Obj *obj, f32 min, f32 max, f32 accel) {
    BObj *w = BOBJ(obj);
    f32 lim;
    s32 pad;

    if (gBattleBossId == 5) {
        lim = 40.0f;
    } else {
        lim = 20.0f;
    }
    if (lim < w->vel[1]) {
        w->vel[1] = lim;
    } else if (w->vel[1] < -lim) {
        w->vel[1] = -lim;
    }
    if (w->frame->pos[1] < min) {
        w->vel[1] += accel;
    } else if (max < w->frame->pos[1]) {
        w->vel[1] -= accel;
    } else {
        if (gBattleBossId == 5) {
            if (w->vel[1] > 0.0f) {
                w->vel[1] += accel / 2;
            } else if (w->vel[1] < 0.0f) {
                w->vel[1] -= accel / 2;
            } else {
                w->vel[1] = (fadeRand(100) - 50) * 0.01;
            }
            return 0;
        }
        if (w->vel[1] > 0.0f) {
            w->vel[1] += accel;
        } else if (w->vel[1] < 0.0f) {
            w->vel[1] -= accel;
        } else {
            w->vel[1] = (fadeRand(100) - 50) * 0.01;
        }
        return 0;
    }
    return 1;
}

/* The same for z. (The result 1 is set first: "return 1" is shared at the end, the three
 * "keep going" cases share one "return 0".) */
s32 bossPatrolZ(Obj *obj, f32 min, f32 max, f32 accel) {
    BObj *b = BOBJ(obj);
    f32 lim;
    s32 pad;

    if (gBattleBossId == 5) {
        lim = 40.0f;
    } else {
        lim = 20.0f;
    }
    if (lim < b->vel[2]) {
        b->vel[2] = lim;
    } else if (b->vel[2] < -lim) {
        b->vel[2] = -lim;
    }
    if (b->frame->pos[2] < min) {
        b->vel[2] += accel;
    } else if (max < b->frame->pos[2]) {
        b->vel[2] -= accel;
    } else {
        if (b->vel[2] > 0.0f) {
            b->vel[2] += accel;
        } else if (b->vel[2] < 0.0f) {
            b->vel[2] -= accel;
        } else {
            b->vel[2] = (fadeRand(100) - 50) * 0.02;
        }
        return 0;
    }
    return 1;
}

void bossAddDownCount(s32 n) {
    gBossDownCount += n;
    if (gBossDownCount < 0) {
        gBossDownCount = 0;
    }
    DPRINTF("現在のダウンカウント[%d]\n", gBossDownCount);
}

/* 1, and the counter reset, once it reaches n. */
s32 bossDownCountOver(s32 n) {
    if (gBossDownCount >= n) {
        DPRINTF("ダウン回数オーバー[%d/%d]。特殊ダウンします。\n", gBossDownCount, n);
        gBossDownCount = 0;
        return 1;
    }
    return 0;
}

/* Marks a boss (in the save, at 0x80088310) as met. */
void bossBattleStart(u8 boss) {
    DPRINTF("ボス戦開始処理を行います。\n");
    gSaveData.stageFlags[0x5E + boss] = 1;
}

/* Marks a boss as beaten, and the current stage as cleared (5 more for the last boss). */
void bossBattleWin(u8 boss) {
    DPRINTF("ボス戦勝利処理を行います。\n");
    gSaveData.stageFlags[0x5E + boss] = 2;
    gGameWork.losses = 0;
    if (boss == 5) {
        gSaveData.stageFlags[gGameWork.area * 16 + gGameWork.areaStage] |= 1;
        return;
    }
    gSaveData.stageFlags[gGameWork.area * 16 + gGameWork.areaStage] |= 5;
}

/* A lost battle: the third in a row ends the game, else the battle starts again. */
void bossBattleLose(s32 a) {
    DPRINTF("ボス戦敗北処理を行います。\n");
    gGameWork.losses++;
    if (gGameWork.losses >= 3) {
        DPRINTF("ボスに３回負けました。引退します。\n");
        gGameWork.losses = 0;
        (*stageGotoGameOver)();
        return;
    }
    DPRINTF("まだまだ若いもんには負けんぞぅ\n");
    (*stageGotoBattle)(gBattleBossId);
}

/* Rumbles the pilots' controllers: with one player, who 0 or 2 is the first; with two, 0 the
 * first, 1 the second, else both. */
void bossRumble(s32 who, u32 pattern, s32 time) {
    if (fadeCountPlayers() == 1) {
        if (who == 0) {
            contPakRumbleStart(gCockpitPort, pattern, time);
        } else if (who == 2) {
            contPakRumbleStart(gCockpitPort, pattern, time);
        }
    }
    if (fadeCountPlayers() == 2) {
        if (who == 0) {
            contPakRumbleStart(gCockpitPort, pattern, time);
            return;
        }
        if (who == 1) {
            contPakRumbleStart(gBrotherPort, pattern, time);
            return;
        }
        contPakRumbleStart(gCockpitPort, pattern, time);
        contPakRumbleStart(gBrotherPort, pattern, time);
    }
}

