#include "player.h"
#include "aniatk.h"
#include "fade.h"
#include "file.h"
#include "fileres.h"
#include "heap.h"
#include "model.h"
#include "plbox.h"
#include "plmain.h"
#include "plmodel.h"
#include "sndloc.h"
#include "polygon.h"
#include "zanzou.h"

/* The player's model and animation (overlay 9): each action's animation entry ("pl_anime"),
 * its attack box and face animations, and loading the character's model and textures
 * ("pl_model_set", "pl_tex_set"). The prints are an empty macro, so their arguments are still
 * evaluated. */

#define DPRINTF

/* An attack box's animation frame (acoll.h's AniAtkFrame: its time is this frame). */
typedef struct PlAtkFrame {
    /* 0x00 */ u8 pad00[0x24];
    /* 0x24 */ f32 frame;
} PlAtkFrame;

typedef struct PlStat {
    /* 0x0 */ u8 pad0[3];
    /* 0x3 */ s8 wpLv;
    /* 0x4 */ u8 pad4[6];
} PlStat;

typedef struct PlGlow {
    /* 0x0 */ u32 on;
    /* 0x4 */ u32 off;
} PlGlow;

extern u32 gGameFrameCount; /* the frame count */
extern PlStat gPlayerStats[];
extern ACollFrame *pl_atk_root_frame; /* the player attacks' boxes */
/* Per character, in the .data after plshot.c (0x801A8980 on). */
extern PlActAnim gGoemonActAnimTable[]; /* the animations of the actions */
extern PlActAnim gPlEbisuActAnimTable[];
extern PlActAnim gPlSasukeActAnimTable[];
extern PlActAnim gPlYaeActAnimTable[];
extern u8 gGoemonModelPieceTable[];
extern u8 gPlEbisuModelPieceTable[];
extern u8 gPlSasukePieceTable[];
extern u8 gPlYaeModelPieceTable[];
extern u8 gGoemonAtkPieceTable[];
extern u8 gPlEbisuAtkPieceTable[];
extern u8 gPlSasukeAtkPieceTable[];
extern u8 gPlYaeAtkPieceTable[];
extern u8 gGoemonSeg5PieceTable[];
extern u8 gPlEbisuSeg5PieceTable[];
extern u8 gPlSasukeSeg5PieceTable[];
extern u8 gPlYaeSeg5PieceTable[];
extern u32 gGoemonAtkBoxTable[]; /* the attack boxes */
extern u32 gPlEbisuAtkBoxTable[];
extern u32 gPlSasukeAtkBoxTable[];
extern u32 gPlYaeAtkBoxTable[];
extern u16 gGoemonEventSeTable[][4]; /* the four event sounds of the animations (as in plsound.c) */
extern u16 gPlEbisuEventSeTable[][4];
extern u16 gPlSasukeEventSeTable[][4];
extern u16 gPlYaeEventSeTable[][4];

/* plmodel.c's .data (0x801A7CF0-0x801A84A0). */
PlActAnim *gPlActAnimTables[4] = { /* per character: the animations of its actions */
    gGoemonActAnimTable, gPlEbisuActAnimTable, gPlSasukeActAnimTable, gPlYaeActAnimTable,
};
u32 gPlModelPieceTables[4] = {
    (u32) gGoemonModelPieceTable, (u32) gPlEbisuModelPieceTable, (u32) gPlSasukePieceTable,
    (u32) gPlYaeModelPieceTable,
};
u32 gPlAtkPieceTables[4] = {
    (u32) gGoemonAtkPieceTable, (u32) gPlEbisuAtkPieceTable, (u32) gPlSasukeAtkPieceTable,
    (u32) gPlYaeAtkPieceTable,
};
u32 gPlZanzouPieceTables[4] = {
    (u32) gGoemonSeg5PieceTable, (u32) gPlEbisuSeg5PieceTable, (u32) gPlSasukeSeg5PieceTable,
    (u32) gPlYaeSeg5PieceTable,
};
u32 *gPlAtkBoxTables[4] = { /* per character: its attack boxes */
    gGoemonAtkBoxTable, gPlEbisuAtkBoxTable, gPlSasukeAtkBoxTable, gPlYaeAtkBoxTable,
};
u16 (*gPlModelEventSoundTables[4])[4] = { /* not named by any code (plsound.c has its own copy) */
    gGoemonEventSeTable, gPlEbisuEventSeTable, gPlSasukeEventSeTable, gPlYaeEventSeTable,
};
/* The glowing weapons: environment-mapped calls of the model pieces. */
Gfx gPlGoemonGlowDl[] = {
    gsSPSetGeometryMode(G_CULL_BACK | G_LIGHTING | G_TEXTURE_GEN),
    gsSPTexture(0x0800, 0x1000, 0, G_TX_RENDERTILE, G_ON),
    gsSPDisplayList(0x04007278),
    gsSPEndDisplayList(),
};
Gfx gPlSasukeGlowDl[] = {
    gsSPSetGeometryMode(G_CULL_BACK | G_LIGHTING | G_TEXTURE_GEN),
    gsSPTexture(0x0800, 0x1000, 0, G_TX_RENDERTILE, G_ON),
    gsSPDisplayList(0x04004F68),
    gsSPEndDisplayList(),
};
Gfx gPlYaeGlowDl[] = {
    gsSPSetGeometryMode(G_CULL_BACK | G_LIGHTING | G_TEXTURE_GEN),
    gsSPTexture(0x0800, 0x1000, 0, G_TX_RENDERTILE, G_ON),
    gsSPDisplayList(0x04006B38),
    gsSPEndDisplayList(),
};
/* Per character, two lists of pieces (NULL ends a list) that the glow swaps. */
Gfx *gPlGoemonGlowOn[] = { (Gfx *) 0x040048E8, NULL };
Gfx *gPlGoemonGlowOff[] = { gPlGoemonGlowDl, NULL };
Gfx *gPlEbisumaruGlowOn[] = { (Gfx *) 0x040048E8, NULL };
Gfx *gPlEbisumaruGlowOff[] = { (Gfx *) 0x04007278, NULL };
Gfx *gPlSasukeGlowOn[] = { (Gfx *) 0x04002E88, (Gfx *) 0x04002DB8, NULL };
Gfx *gPlSasukeGlowOff[] = { gPlSasukeGlowDl, gPlSasukeGlowDl, NULL };
Gfx *gPlYaeGlowOn[] = { (Gfx *) 0x04003110, NULL };
Gfx *gPlYaeGlowOff[] = { gPlYaeGlowDl, NULL };
PlGlow gPlGlowTable[4] = {
    { (u32) gPlGoemonGlowOn, (u32) gPlGoemonGlowOff },
    { (u32) gPlEbisumaruGlowOn, (u32) gPlEbisumaruGlowOff },
    { (u32) gPlSasukeGlowOn, (u32) gPlSasukeGlowOff },
    { (u32) gPlYaeGlowOn, (u32) gPlYaeGlowOff },
};
AnimFrame gPlGlowFramesLow[] = { { 0x03001F60, 1 }, { 0, 0 } };
AnimFrame gPlGlowFramesHigh[] = { { 0x03002F60, 1 }, { 0, 0 } };
AnimFrame *gPlGlowFramesByLevel[4][3] = { /* the glow frames, by weapon level */
    { gPlGlowFramesLow, gPlGlowFramesLow, gPlGlowFramesHigh },
    { gPlGlowFramesLow, gPlGlowFramesLow, gPlGlowFramesLow },
    { gPlGlowFramesLow, gPlGlowFramesLow, gPlGlowFramesHigh },
    { gPlGlowFramesLow, gPlGlowFramesLow, gPlGlowFramesHigh },
};
/* The model pieces each costume swaps (NULL ends a list): per character, the plain model's
 * (segment 4), then the pieces of flag 0x80000000 and of costumes 1-3 (segment 5). */
Gfx *gPlGoemonPlainPieces[] = {
    (Gfx *) 0x04001F48, (Gfx *) 0x04002008, (Gfx *) 0x04002170, (Gfx *) 0x04002320,
    (Gfx *) 0x040023E0, (Gfx *) 0x04002548, (Gfx *) 0x040026F8, (Gfx *) 0x040028A0,
    (Gfx *) 0x04002C48, (Gfx *) 0x04002D90, (Gfx *) 0x04002EF0, (Gfx *) 0x04003028,
    (Gfx *) 0x040031B8, (Gfx *) 0x040035F8, (Gfx *) 0x04003698, NULL,
};
Gfx *gPlEbisumaruPlainPieces[] = {
    (Gfx *) 0x04002A98, (Gfx *) 0x04002238, (Gfx *) 0x04001F50, (Gfx *) 0x04001D00,
    (Gfx *) 0x04003100, (Gfx *) 0x04002610, (Gfx *) 0x04001EB0, (Gfx *) 0x04001C50,
    (Gfx *) 0x04003028, (Gfx *) 0x04002538, (Gfx *) 0x040020E0, (Gfx *) 0x04001A18,
    (Gfx *) 0x040033B0, (Gfx *) 0x040028C0, (Gfx *) 0x04001BC8, (Gfx *) 0x04001958,
    (Gfx *) 0x040032D8, (Gfx *) 0x040027E8, NULL,
};
Gfx *gPlSasukePlainPieces[] = {
    (Gfx *) 0x04001EF0, (Gfx *) 0x04002170, (Gfx *) 0x04002298, (Gfx *) 0x04001CA8,
    (Gfx *) 0x04001A88, (Gfx *) 0x040019A8, (Gfx *) 0x04002AA8, (Gfx *) 0x04002848,
    (Gfx *) 0x04001DD8, (Gfx *) 0x04002970, (Gfx *) 0x040027B8, (Gfx *) 0x04001D40,
    (Gfx *) 0x04002570, (Gfx *) 0x04002058, (Gfx *) 0x04002698, (Gfx *) 0x040024F0,
    (Gfx *) 0x04001FD0, NULL,
};
Gfx *gPlYaePlainPieces[] = {
    (Gfx *) 0x04001C40, (Gfx *) 0x04001BD8, (Gfx *) 0x04002628, (Gfx *) 0x04001CF8,
    (Gfx *) 0x04002B28, (Gfx *) 0x04002860, (Gfx *) 0x04002268, (Gfx *) 0x040020E0,
    (Gfx *) 0x04002130, (Gfx *) 0x040028F0, (Gfx *) 0x040027C8, (Gfx *) 0x040021A8,
    (Gfx *) 0x04002A10, (Gfx *) 0x040024A8, (Gfx *) 0x04002038, (Gfx *) 0x04002088,
    (Gfx *) 0x04002AA0, (Gfx *) 0x04002978, (Gfx *) 0x040023E8, NULL,
};
Gfx *gPlGoemonFlagPieces[] = {
    (Gfx *) 0x05002608, (Gfx *) 0x050026D8, (Gfx *) 0x050028A0, (Gfx *) 0x05002A40,
    (Gfx *) 0x05002B10, (Gfx *) 0x05002CD8, (Gfx *) 0x05002E90, (Gfx *) 0x05003090,
    (Gfx *) 0x05003438, (Gfx *) 0x05003578, (Gfx *) 0x05003730, (Gfx *) 0x05003870,
    (Gfx *) 0x05003A58, (Gfx *) 0x05002608, (Gfx *) 0x05002A40, NULL,
};
Gfx *gPlEbisumaruFlagPieces[] = {
    (Gfx *) 0x05004098, (Gfx *) 0x050036C0, (Gfx *) 0x05003338, (Gfx *) 0x05003078,
    (Gfx *) 0x05004B68, (Gfx *) 0x05003B28, (Gfx *) 0x05003270, (Gfx *) 0x05002FA0,
    (Gfx *) 0x05004A78, (Gfx *) 0x05003A20, (Gfx *) 0x05003550, (Gfx *) 0x05002D10,
    (Gfx *) 0x05004E90, (Gfx *) 0x05003E70, (Gfx *) 0x05002F08, (Gfx *) 0x05002C28,
    (Gfx *) 0x05004DA0, (Gfx *) 0x05003D80, NULL,
};
Gfx *gPlSasukeFlagPieces[] = {
    (Gfx *) 0x05002A48, (Gfx *) 0x05002CF0, (Gfx *) 0x05002E18, (Gfx *) 0x050027F0,
    (Gfx *) 0x050024F8, (Gfx *) 0x050023B8, (Gfx *) 0x05003678, (Gfx *) 0x05003408,
    (Gfx *) 0x05002930, (Gfx *) 0x05003540, (Gfx *) 0x05003378, (Gfx *) 0x05002898,
    (Gfx *) 0x05003120, (Gfx *) 0x05002BD8, (Gfx *) 0x05003258, (Gfx *) 0x050030A0,
    (Gfx *) 0x05002B40, NULL,
};
Gfx *gPlYaeFlagPieces[] = {
    (Gfx *) 0x05002D20, (Gfx *) 0x05002CB8, (Gfx *) 0x05003878, (Gfx *) 0x05002DF0,
    (Gfx *) 0x05003E40, (Gfx *) 0x05003B08, (Gfx *) 0x05003420, (Gfx *) 0x050031E8,
    (Gfx *) 0x05003238, (Gfx *) 0x05003BC8, (Gfx *) 0x05003A70, (Gfx *) 0x05003360,
    (Gfx *) 0x05003CF0, (Gfx *) 0x050036A8, (Gfx *) 0x05003298, (Gfx *) 0x050032E8,
    (Gfx *) 0x05003DB0, (Gfx *) 0x05003C58, (Gfx *) 0x050035E8, NULL,
};
Gfx *gPlGoemonCostume1Pieces[] = {
    (Gfx *) 0x050025F8, (Gfx *) 0x05002728, (Gfx *) 0x050028A0, (Gfx *) 0x05002A70,
    (Gfx *) 0x05002B90, (Gfx *) 0x05002D08, (Gfx *) 0x05002ED8, (Gfx *) 0x05003078,
    (Gfx *) 0x050033A8, (Gfx *) 0x050034F0, (Gfx *) 0x050036E0, (Gfx *) 0x05003818,
    (Gfx *) 0x05003A38, (Gfx *) 0x050025F8, (Gfx *) 0x05002A70, NULL,
};
Gfx *gPlGoemonCostume2Pieces[] = {
    (Gfx *) 0x05002F90, (Gfx *) 0x05003138, (Gfx *) 0x05003260, (Gfx *) 0x050033A8,
    (Gfx *) 0x05003550, (Gfx *) 0x05003678, (Gfx *) 0x050037A8, (Gfx *) 0x05003928,
    (Gfx *) 0x05003CD0, (Gfx *) 0x05003E10, (Gfx *) 0x05003FC8, (Gfx *) 0x05004108,
    (Gfx *) 0x050042D8, (Gfx *) 0x05002F90, (Gfx *) 0x050033A8, NULL,
};
Gfx *gPlGoemonCostume3Pieces[] = {
    (Gfx *) 0x05001E88, (Gfx *) 0x05001FE8, (Gfx *) 0x050020A8, (Gfx *) 0x05002140,
    (Gfx *) 0x050022A0, (Gfx *) 0x05002360, (Gfx *) 0x05002410, (Gfx *) 0x05002950,
    (Gfx *) 0x050025B0, (Gfx *) 0x05002668, (Gfx *) 0x05002768, (Gfx *) 0x05002820,
    (Gfx *) 0x05002D30, (Gfx *) 0x05001E88, (Gfx *) 0x05002140, NULL,
};
Gfx *gPlEbisumaruCostume1Pieces[] = {
    (Gfx *) 0x05003938, (Gfx *) 0x05003538, (Gfx *) 0x05002E98, (Gfx *) 0x05002D28,
    (Gfx *) 0x05004000, (Gfx *) 0x050030C0, (Gfx *) 0x05002DE8, (Gfx *) 0x05002C70,
    (Gfx *) 0x05003F28, (Gfx *) 0x05002FE8, (Gfx *) 0x05003398, (Gfx *) 0x05002B00,
    (Gfx *) 0x050041D8, (Gfx *) 0x05003298, (Gfx *) 0x05002BC0, (Gfx *) 0x05002A48,
    (Gfx *) 0x05004100, (Gfx *) 0x050031C0, NULL,
};
Gfx *gPlEbisumaruCostume2Pieces[] = {
    (Gfx *) 0x05004190, (Gfx *) 0x05003840, (Gfx *) 0x05003548, (Gfx *) 0x05003298,
    (Gfx *) 0x05004998, (Gfx *) 0x05003D20, (Gfx *) 0x050033E0, (Gfx *) 0x05003158,
    (Gfx *) 0x05004838, (Gfx *) 0x05003BC0, (Gfx *) 0x05003740, (Gfx *) 0x05002ED8,
    (Gfx *) 0x05004C80, (Gfx *) 0x05004008, (Gfx *) 0x05003020, (Gfx *) 0x05002D98,
    (Gfx *) 0x05004B20, (Gfx *) 0x05003EA8, NULL,
};
Gfx *gPlEbisumaruCostume3Pieces[] = {
    (Gfx *) 0x05003E80, (Gfx *) 0x050041F8, (Gfx *) 0x05004B10, (Gfx *) 0x05003C28,
    (Gfx *) 0x05005788, (Gfx *) 0x05004688, (Gfx *) 0x05003D58, (Gfx *) 0x05003B08,
    (Gfx *) 0x05005648, (Gfx *) 0x05004548, (Gfx *) 0x05004088, (Gfx *) 0x050038C8,
    (Gfx *) 0x05005A58, (Gfx *) 0x05004958, (Gfx *) 0x050039F8, (Gfx *) 0x05003798,
    (Gfx *) 0x05005928, (Gfx *) 0x05004828, NULL,
};
Gfx *gPlSasukeCostume1Pieces[] = {
    (Gfx *) 0x05002240, (Gfx *) 0x05002610, (Gfx *) 0x05002708, (Gfx *) 0x05001ED0,
    (Gfx *) 0x05001C98, (Gfx *) 0x05001BB8, (Gfx *) 0x05002E78, (Gfx *) 0x05002C30,
    (Gfx *) 0x05002128, (Gfx *) 0x05002D58, (Gfx *) 0x05002BA0, (Gfx *) 0x05001F80,
    (Gfx *) 0x05002958, (Gfx *) 0x050024F8, (Gfx *) 0x05002A80, (Gfx *) 0x050028C8,
    (Gfx *) 0x05002350, NULL,
};
Gfx *gPlSasukeCostume2Pieces[] = {
    (Gfx *) 0x05002D88, (Gfx *) 0x05003090, (Gfx *) 0x050031A0, (Gfx *) 0x05002A00,
    (Gfx *) 0x050026D0, (Gfx *) 0x050025D8, (Gfx *) 0x05003988, (Gfx *) 0x050037B0,
    (Gfx *) 0x05002C80, (Gfx *) 0x05003850, (Gfx *) 0x050036A8, (Gfx *) 0x05002B78,
    (Gfx *) 0x05003500, (Gfx *) 0x05002F88, (Gfx *) 0x050035A0, (Gfx *) 0x050033E8,
    (Gfx *) 0x05002E80, NULL,
};
Gfx *gPlSasukeCostume3Pieces[] = {
    (Gfx *) 0x05002838, (Gfx *) 0x05002A30, (Gfx *) 0x05002B30, (Gfx *) 0x050026A8,
    (Gfx *) 0x050023B0, (Gfx *) 0x050021B0, (Gfx *) 0x05002FF0, (Gfx *) 0x05002EE0,
    (Gfx *) 0x050027D0, (Gfx *) 0x05002F68, (Gfx *) 0x05002E78, (Gfx *) 0x05002750,
    (Gfx *) 0x05002D80, (Gfx *) 0x050029B8, (Gfx *) 0x05002E08, (Gfx *) 0x05002D18,
    (Gfx *) 0x05002920, NULL,
};
Gfx *gPlYaeCostume1Pieces[] = {
    (Gfx *) 0x05003970, (Gfx *) 0x05003820, (Gfx *) 0x050049A0, (Gfx *) 0x05003AD8,
    (Gfx *) 0x05005388, (Gfx *) 0x05004D20, (Gfx *) 0x05004508, (Gfx *) 0x05003F78,
    (Gfx *) 0x05004078, (Gfx *) 0x05004E68, (Gfx *) 0x05004BE8, (Gfx *) 0x050043B0,
    (Gfx *) 0x050050D8, (Gfx *) 0x050047E8, (Gfx *) 0x05004188, (Gfx *) 0x05004288,
    (Gfx *) 0x05005220, (Gfx *) 0x05004FA0, (Gfx *) 0x05004690, NULL,
};
Gfx *gPlYaeCostume2Pieces[] = {
    (Gfx *) 0x05003E00, (Gfx *) 0x05003CF8, (Gfx *) 0x05004EC8, (Gfx *) 0x05003F80,
    (Gfx *) 0x05005970, (Gfx *) 0x050052A0, (Gfx *) 0x05004A10, (Gfx *) 0x050044C0,
    (Gfx *) 0x050045C0, (Gfx *) 0x05005408, (Gfx *) 0x05005140, (Gfx *) 0x050048E0,
    (Gfx *) 0x050056A0, (Gfx *) 0x05004CE0, (Gfx *) 0x050046D0, (Gfx *) 0x050047D0,
    (Gfx *) 0x05005808, (Gfx *) 0x05005540, (Gfx *) 0x05004BB0, NULL,
};
Gfx *gPlYaeCostume3Pieces[] = {
    (Gfx *) 0x050034E0, (Gfx *) 0x050033D8, (Gfx *) 0x05004410, (Gfx *) 0x05003660,
    (Gfx *) 0x05004E30, (Gfx *) 0x05004780, (Gfx *) 0x05004020, (Gfx *) 0x05003AB0,
    (Gfx *) 0x05003BB0, (Gfx *) 0x050048F0, (Gfx *) 0x05004620, (Gfx *) 0x05003EE0,
    (Gfx *) 0x05004B88, (Gfx *) 0x050042B8, (Gfx *) 0x05003CC8, (Gfx *) 0x05003DC8,
    (Gfx *) 0x05004CF8, (Gfx *) 0x05004A28, (Gfx *) 0x05004178, NULL,
};
u32 gPlCostume1Files[4] = { 0x01BB, 0x01BE, 0x01C1, 0x01C4 }; /* per character: costume 1 file */
u32 gPlCostume2Files[4] = { 0x01BC, 0x01BF, 0x01C2, 0x01C5 }; /* costume 2 */
u32 gPlCostume3Files[4] = { 0x01BD, 0x01C0, 0x01C3, 0x01C6 }; /* costume 3 */
u32 gPlPlainPiecesTable[4] = { /* per character: the plain model pieces */
    (u32) gPlGoemonPlainPieces, (u32) gPlEbisumaruPlainPieces, (u32) gPlSasukePlainPieces,
    (u32) gPlYaePlainPieces,
};
u32 gPlFlagPiecesTable[4] = { /* flag 0x80000000 */
    (u32) gPlGoemonFlagPieces, (u32) gPlEbisumaruFlagPieces, (u32) gPlSasukeFlagPieces,
    (u32) gPlYaeFlagPieces,
};
u32 gPlCostume1PiecesTable[4] = { /* costume 1 */
    (u32) gPlGoemonCostume1Pieces, (u32) gPlEbisumaruCostume1Pieces, (u32) gPlSasukeCostume1Pieces,
    (u32) gPlYaeCostume1Pieces,
};
u32 gPlCostume2PiecesTable[4] = { /* costume 2 */
    (u32) gPlGoemonCostume2Pieces, (u32) gPlEbisumaruCostume2Pieces, (u32) gPlSasukeCostume2Pieces,
    (u32) gPlYaeCostume2Pieces,
};
u32 gPlCostume3PiecesTable[4] = { /* costume 3 */
    (u32) gPlGoemonCostume3Pieces, (u32) gPlEbisumaruCostume3Pieces, (u32) gPlSasukeCostume3Pieces,
    (u32) gPlYaeCostume3Pieces,
};
extern u32 gPlModelFileTable[]; /* per character: the model's file (the low half) */
extern u32 gPlTexFileTable[]; /* and its textures' */
extern u32 gPlSeg2FileTable[];
extern u32 gPlPiecesFileTable[]; /* the model pieces (passed as a u16: IDO loads the low half) */
extern u32 gPlAtkBoxFileTable[]; /* the attack box pieces */
extern u32 gPlOtherFileTable[]; /* the other pieces */
extern u32 gPlBuffSizeModel[]; /* the model's decode buffer */
extern u32 gPlBuffSizeTex[]; /* the textures' */


/* The animation's step: loops or holds at either end (flag 0x210 & 2 while it runs), runs the
 * face, and records the entry's events. The model goes through m before the length call and
 * before the event call (v1, not v0); the step is recorded before the flags and the action (a
 * store through p cannot pass the step's reload from the stack). */
void plAnimeStep(Player *p) {
    s32 end;
    f32 step = p->animStep + p->animStepAdd;
    AnimeFrame *m;
    PlAtkFrame *atk;
    s32 ev;

    if (p->flags & 0x800000) {
        step *= 0.5f;
    }
    m = p->model;
    m->frame += step;
    end = polAnimeLastFrame(p->model);
    m = p->model;
    if (m->frame >= end) {
        if (p->animFlags & 1) {
            m->frame -= end;
        } else {
            m->frame = end;
        }
        p->animStatus |= 3;
    } else if (m->frame < 0.0f) {
        if (p->animFlags & 1) {
            m->frame += end;
        } else {
            m->frame = 0.0f;
        }
        p->animStatus |= 3;
    } else {
        p->animStatus &= ~2;
    }
    if (!(p->flags & 0x1000000) && (!(p->flags & 0x800000) || (gGameFrameCount & 1))) {
        fadeAnimStep(&p->anims[0]);
        fadeAnimStep(&p->anims[0x10]);
    }
    if (p->animEvents != 0 && !(p->flags2 & 1)) {
        m = p->model;
        ev = soundScoreCheck(p->animEvents, gPlModelFileTable[p->chara], m->frame, step,
                                    p->animFlags, m);
        if (ev != -1 && ev != 0) {
            p->eventBits |= ev;
            p->eventList = p->animEvents;
            p->eventFrame = p->model->frame;
            p->eventStep = step;
            p->eventAnimFlags = p->animFlags;
            p->eventAction = p->action;
            p->eventSeSet = p->seSet;
        }
    }
    if (p->box != NULL && (atk = (PlAtkFrame *) p->box->model) != NULL) {
        atk->frame = p->model->frame;
        if (aniAtkBoxFrameCount(p->box) <= atk->frame) {
            plBoxDelete(p);
        }
    }
}

/* Sets the action's animation, unless its entry keeps the old one (flag 8). */
void plAnimeSetCurAction(Player *p) {
    s32 action = p->action;
    PlActAnim *e = &gPlActAnimTables[p->chara][action];

    if (!(e->flags & 8)) {
        plAnimeSet(p, action);
    }
}

/* Sets an action's animation: its model piece, attack box, flags, glow and events. The zero
 * stores are int literals (their 0.0f is not shared with the step test's), the step is stored
 * before its zero, the glow table is indexed in the call (no local), and the last call's piece
 * is read into a local inside the if (the test's load sits in v0). */
void plAnimeSet(Player *p, s32 action) {
    PlActAnim *e = &gPlActAnimTables[p->chara][action];
    s16 seg = e->seg;
    s32 piece;

    if (seg != p->loadedPiece) {
        p->loadedPiece = seg;
        p->model->segs[0] = mdlLoadPart(gPlPiecesFileTable[p->chara],
                                                   gPlModelPieceTables[p->chara], seg, p->pieceBuf);
        if (e->atk != -1) {
            p->atkBoxPart = mdlLoadPart(gPlAtkBoxFileTable[p->chara],
                                               gPlAtkPieceTables[p->chara], e->atk, p->atkBoxBuf);
        } else {
            p->atkBoxPart = 0;
        }
        if (e->zanzouPiece != -1) {
            p->zanzouPart = mdlLoadPart(gPlOtherFileTable[p->chara],
                                               gPlZanzouPieceTables[p->chara], e->zanzouPiece,
                                               p->zanzouBuf);
        } else {
            p->zanzouPart = 0;
        }
    }
    if (e->atk != -1 && p->atkBoxPart != 0) {
        p->box = aniAtkCreate(
            pl_atk_root_frame, p->model, (gPlAtkBoxTables[p->chara][e->atk] & 0xFFFFFF) + p->atkBoxPart,
            0, p->atkBoxPart);
    }
    p->model->anim = e->anim;
    p->animStep = e->step;
    p->animStepAdd = 0;
    p->animFlags = e->flags;
    if (p->animStep < 0.0f) {
        p->model->frame = polAnimeLastFrame(p->model);
    } else {
        p->model->frame = 0;
    }
    p->animEvents = e->events;
    p->seSet = e->eventSeSet;
    p->animStatus &= ~1;
    if (p->flags2 & 0x20) {
        mdlSwapDLs(e->anim, p->model->segs[0], gPlPlainPiecesTable[p->chara],
                               gPlCostume1PiecesTable[p->chara]);
    } else if (p->flags2 & 0x40) {
        mdlSwapDLs(e->anim, p->model->segs[0], gPlPlainPiecesTable[p->chara],
                               gPlCostume2PiecesTable[p->chara]);
    } else if (p->flags2 & 0x80) {
        mdlSwapDLs(e->anim, p->model->segs[0], gPlPlainPiecesTable[p->chara],
                               gPlCostume3PiecesTable[p->chara]);
    } else if (p->flags & 0x80000000) {
        mdlSwapDLs(e->anim, p->model->segs[0], gPlPlainPiecesTable[p->chara],
                               gPlFlagPiecesTable[p->chara]);
    }
    if (p->animFlags & 0x20) {
        p->model->flags |= 0x10;
        mdlSwapDLs(e->anim, p->model->segs[0], gPlGlowTable[p->chara].on,
                               gPlGlowTable[p->chara].off);
        fadeAnimStart(&p->anims[0x20],
                               gPlGlowFramesByLevel[p->chara][gPlayerStats[p->index].wpLv], 0,
                               &p->model->segs[5], 0x306);
    } else {
        p->model->flags &= ~0x10;
        mdlSwapDLs(e->anim, p->model->segs[0], gPlGlowTable[p->chara].off,
                               gPlGlowTable[p->chara].on);
    }
    if (e->zanzouPiece != -1 && p->zanzouPart != 0) {
        piece = p->zanzouPart;
        zanzouStart(p->index, piece, e);
    }
}

/* Loads the character's model and textures ("pl_model_set"); TRUE if it could not. */
s32 pl_model_set(Player *p) {
    u32 mdl;
    u32 tex;

    DPRINTF("pl_anime: 攻撃あたりがたりません\n");
    DPRINTF("pl_model_set: PLAYER KIND: %d\n", p->chara);
    mdl = gPlModelFileTable[p->chara];
    tex = gPlTexFileTable[p->chara];
    if (gPlBuffSizeModel[p->chara] < fileRamSize(mdl)) {
        DPRINTF("pl_model_set: small decode buffer. Needs 0x%08x bytes.\n",
                fileRamSize(mdl));
        return TRUE;
    }
    if (gPlBuffSizeTex[p->chara] < fileRamSize(tex)) {
        DPRINTF("pl_tex_set: small decode buffer. Needs 0x%08x bytes.\n", fileRamSize(tex));
        return TRUE;
    }
    if (resFindFile(mdl) == -1) {
        p->modelFile = plGetBuffAddr(p, 0);
        p->texFile = plGetBuffAddr(p, 1);
        if (p->modelFile == 0 || p->texFile == 0) {
            DPRINTF("pl_model_set: no memory\n");
            return TRUE;
        }
        if (p->flags2 & 0x100) {
            DPRINTF("pl_model_set(): omake player\n");
            if (!plInflateWorkCheck()) {
                return TRUE;
            }
            fileLoadNow(mdl, p->modelFile);
            if (!plInflateWorkCheck()) {
                return TRUE;
            }
            fileLoadNow(tex, p->texFile);
        } else {
            if (!plInflateWorkCheck()) {
                return TRUE;
            }
            fileLoadNowKeep(mdl, p->modelFile);
            if (!plInflateWorkCheck()) {
                return TRUE;
            }
            fileLoadNowKeep(tex, p->texFile);
        }
    } else {
        DPRINTF("pl_model_set: already transed\n");
        p->modelFile = resGetAddr(resFindFile(mdl));
        p->texFile = resGetAddr(resFindFile(tex));
    }
    p->model->segs[1] = p->modelFile;
    if (p->flags2 & 0x20) {
        if (!__enoughInflateBufferP(gPlCostume1Files[p->chara])) {
            return TRUE;
        }
        fileLoadNow(gPlCostume1Files[p->chara], 0);
        p->model->segs[2] = fileRamAddr(gPlCostume1Files[p->chara]);
    } else if (p->flags2 & 0x40) {
        if (!__enoughInflateBufferP(gPlCostume2Files[p->chara])) {
            return TRUE;
        }
        fileLoadNow(gPlCostume2Files[p->chara], 0);
        p->model->segs[2] = fileRamAddr(gPlCostume2Files[p->chara]);
    } else if (p->flags2 & 0x80) {
        if (!__enoughInflateBufferP(gPlCostume3Files[p->chara])) {
            return TRUE;
        }
        fileLoadNow(gPlCostume3Files[p->chara], 0);
        p->model->segs[2] = fileRamAddr(gPlCostume3Files[p->chara]);
    } else if (p->flags & 0x80000000) {
        if (!__enoughInflateBufferP(gPlSeg2FileTable[p->chara])) {
            return TRUE;
        }
        fileLoadNow(gPlSeg2FileTable[p->chara], 0);
        p->model->segs[2] = fileRamAddr(gPlSeg2FileTable[p->chara]);
    }
    return FALSE;
}

/* Starts the action's face animations (eyes and mouth). */
void plFaceAnimeStart(Player *p) {
    PlActAnim *e = &gPlActAnimTables[p->chara][p->action];

    if (!(e->flags & 8)) {
        fadeAnimStart(&p->anims[0], e->face[0], 0, &p->model->segs[3],
                               gPlTexFileTable[p->chara]);
        fadeAnimStart(&p->anims[0x10], e->face[1], 0, &p->model->segs[4],
                               gPlTexFileTable[p->chara]);
    }
}

/* The action's animation entry. */
PlActAnim *plActAnimeGet(Player *p) {
    return &gPlActAnimTables[p->chara][p->action];
}

/* Stops the glow. */
void plGlowOff(Player *p) {
    PlActAnim *e = &gPlActAnimTables[p->chara][p->action];

    p->model->flags &= ~0x10;
    mdlSwapDLs(e->anim, p->model->segs[0], gPlGlowTable[p->chara].off,
                           gPlGlowTable[p->chara].on);
}

/* Whether a file can be inflated (a block its size plus 0xC000). */
s32 __enoughInflateBufferP(u16 file) {
    s32 block = heapMalloc(fileRamSize(file) + 0xC000);

    if (block == 0) {
        DPRINTF("__enoughInflateBufferP: no memory\n");
        return FALSE;
    }
    heapFree(block);
    return TRUE;
}

s32 plInflateWorkCheck(void) {
    s32 block = heapMalloc(0xC000);

    if (block == 0) {
        DPRINTF("__enoughInflateBufferP: no memory\n");
        return FALSE;
    }
    heapFree(block);
    return TRUE;
}
