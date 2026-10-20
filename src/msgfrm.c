#include "object.h"
#include "file.h"
#include "fileres.h"
#include "gfxmgr.h"
#include "msgfrm.h"

/* The message frames: five frame kinds the scenario's windows are drawn with.
 * - "afont " (200): one character of the 12x12 font, at any size, in a colour (string.c).
 * - "cursor" (5): a menu cursor, 16x16, eight frames of a 32x64 sheet (menu.c).
 * - "prompt" (5): the "next" mark, 16x16, two frames of a 32x16 sheet (window.c).
 * - "win" (45): a window's border strip, 256x8, mirrored (window.c).
 * - "face" (5): a 32x32 face picture (face.c).
 * The cursor and prompt sheets are in file 0xA10; the border and faces are given. Each kind
 * draws only while its frame's flag 1 is set. */

/* msgfrm.c's .data (0x8010ACB0-0x8010AEA8), in ROM order. */
Gfx gAFontSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, ENVIRONMENT, 0, TEXEL0, 0, 0, 0, 0, ENVIRONMENT, ENVIRONMENT, 0, TEXEL0, 0),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetTextureLUT(G_TT_NONE),
    gsSPEndDisplayList(),
};
FileList gMsgFrmFileList[] = { { FILELIST_LAST | 0xA10, NULL } };
Gfx gCursorSetupDl[] = {
    gsDPPipeSync(),
    gsDPPipelineMode(G_PM_NPRIMITIVE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureFilter(G_TF_POINT),
    gsDPSetTextureConvert(G_TC_FILT),
    gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_DECALRGBA),
    gsDPSetCycleType(G_CYC_COPY),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetRenderMode(G_RM_TEX_EDGE, G_RM_TEX_EDGE2),
    gsSPEndDisplayList(),
};
Gfx gPromptSetupDl[] = {
    gsDPPipeSync(),
    gsDPPipelineMode(G_PM_NPRIMITIVE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureFilter(G_TF_POINT),
    gsDPSetTextureConvert(G_TC_FILT),
    gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_DECALRGBA),
    gsDPSetCycleType(G_CYC_COPY),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetRenderMode(G_RM_TEX_EDGE, G_RM_TEX_EDGE2),
    gsSPEndDisplayList(),
};
Gfx gWinSetupDl[] = {
    gsDPPipeSync(),
    gsDPPipelineMode(G_PM_NPRIMITIVE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_DECALRGBA),
    gsDPSetCycleType(G_CYC_COPY),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetRenderMode(G_RM_TEX_EDGE, G_RM_TEX_EDGE2),
    gsSPEndDisplayList(),
};
Gfx gFaceSetupDl[] = {
    gsDPPipeSync(),
    gsDPPipelineMode(G_PM_NPRIMITIVE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_DECALRGBA),
    gsDPSetCycleType(G_CYC_COPY),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetRenderMode(G_RM_TEX_EDGE, G_RM_TEX_EDGE2),
    gsSPEndDisplayList(),
};
extern u8 frmCursorSheet[]; /* the cursor sheet in file 0xA10 */
extern u8 frmPromptSheet[]; /* the prompt sheet */
extern s16 gFrmKindAFont; /* the kinds */
extern s16 gFrmKindCursor;
extern s16 gFrmKindPrompt;
extern s16 gFrmKindWin;
extern s16 gFrmKindFace;


/* afont: the render setup and colour. */
Gfx *frmAFontSetup(Gfx *gfx, Color3 color) {
    gSPDisplayList(gfx++, gAFontSetupDl);
    gDPSetEnvColor(gfx++, color.r, color.g, color.b, 0xFF);
    gSPEndDisplayList(gfx);
    return gfx;
}

/* One glyph at (x, y), a size x size cell (8 x 12 texels scaled by size / 12).
 * The cell width and height (size / 12 * 8, size / 12 * 12) are written inside the rectangle,
 * as "<< 2" ("* 4" folds 8 * 4 into 32), and s and t are computed at the top from x and y:
 * uopt's tie order and first block then fall as the game's (IDO_NOTES 246, 285).
 * font.c frmFontDrawGlyph is the same function on float x/y and w/h parameters. */
Gfx *frmAFontDrawGlyph(Gfx *gfx, s32 x, s32 y, s32 size, u8 *glyph) {
    s32 ix;
    s32 iy;
    s32 s;
    s32 t;

    ix = x * 4.0f + 0.5f;
    iy = y * 4.0f + 0.5f;
    s = x * 32.0f - ix * 8;
    t = y * 32.0f - iy * 8;
    gDPPipeSync(gfx++);
    gDPLoadTextureTile_4b(gfx++, glyph, G_IM_FMT_I, 8, 12, 0, 0, 7, 11, 0, G_TX_CLAMP, G_TX_CLAMP,
                          3, 4, G_TX_NOLOD, G_TX_NOLOD);
    gSPTextureRectangle(gfx++, ix, iy, ix + (size / 12 * 8 << 2), iy + (size / 12 * 12 << 2),
                        G_TX_RENDERTILE, s, t, 0x2000 / (size / 12 * 8), 0x3000 / (size / 12 * 12));
    gSPEndDisplayList(gfx);
    return gfx;
}

/* The kind's draw. */
Gfx *frmAFontDraw(Gfx *gfx, AFontFrame *f) {
    if (f->flags & 1) {
        gfx = frmAFontDrawGlyph(frmAFontSetup(gfx, f->color), f->x, f->y, f->size,
                                     f->glyph);
    }
    return gfx;
}

/* A character frame under parent, shown. */
AFontFrame *afontCreate(Frame *parent) {
    AFontFrame *f = (AFontFrame *) frmCreate(parent, gFrmKindAFont);

    if (f == NULL) {
        PRINTF("afontCreate: Error: font create can't\n");
        return NULL;
    }
    f->flags |= 1;
    return f;
}

void frmAFontDestroy(AFontFrame *f) {
    frmDestroy((Frame *) f);
}

/* Shows and hides it. */
void frmAFontShow(AFontFrame *f) {
    f->flags |= 1;
}

void frmAFontHide(AFontFrame *f) {
    f->flags &= ~1;
}

/* Makes and deletes the kind. */
s32 frmAFontKindCreate(void) {
    gFrmKindAFont = frmNewKind("afont ", (FrameDraw) frmAFontDraw,
                                        sizeof(AFontFrame), 200, NULL);
    PRINTF("FRM_AFont を作成\n");
    if (gFrmKindAFont == -1) {
        return -1;
    }
    return 0;
}

void frmAFontKindDelete(void) {
    frmDeleteKind(gFrmKindAFont);
    PRINTF("FRM_AFont を削除\n");
}

/* Loads and frees the frames' files. */
void frmMsgFilesLoad(void) {
    fileLoadListNow(gMsgFrmFileList);
}

void frmMsgFilesFree(void) {
    resFreeList(gMsgFrmFileList);
}

/* cursor: frame blink & 7 of the sheet (two columns of four), copied at (x, y). */
Gfx *frmCursorDrawAt(Gfx *gfx, s32 x, s32 y, u8 blink) {
    u8 *sheet = fileRamAddrAt(0xA10, (u32) frmCursorSheet);

    gSPDisplayList(gfx++, gCursorSetupDl);
    gDPLoadTextureBlock(gfx++, sheet, G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 64, 0, G_TX_WRAP, G_TX_WRAP,
                        G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    gSPTextureRectangle(gfx++, x << 2, y << 2, (x + 15) << 2, (y + 15) << 2, G_TX_RENDERTILE,
                        (blink & 4) * 4 << 5, (blink & 3) * 16 << 5, 4 << 10, 1 << 10);
    gSPEndDisplayList(gfx);
    return gfx;
}

Gfx *frmCursorDraw(Gfx *gfx, MarkFrame *f) {
    if (f->flags & 1) {
        gfx = frmCursorDrawAt(gfx, f->x, f->y, f->blink);
    }
    return gfx;
}

MarkFrame *cursorCreate(Frame *parent) {
    MarkFrame *f = (MarkFrame *) frmCreate(parent, gFrmKindCursor);

    if (f == NULL) {
        PRINTF("cursorCreate: Error: cursor create can't\n");
        return NULL;
    }
    f->flags |= 1;
    return f;
}

void frmCursorDestroy(MarkFrame *f) {
    frmDestroy((Frame *) f);
}

s32 frmCursorKindCreate(void) {
    gFrmKindCursor = frmNewKind("cursor", (FrameDraw) frmCursorDraw,
                                        sizeof(MarkFrame), 5, NULL);
    PRINTF("FRM_Cursor を作成\n");
    if (gFrmKindCursor == -1) {
        return -1;
    }
    return 0;
}

void frmCursorKindDelete(void) {
    frmDeleteKind(gFrmKindCursor);
    PRINTF("FRM_Cursor を削除\n");
}

/* prompt: frame blink & 1 of the sheet, copied at (x, y). */
Gfx *frmPromptDrawAt(Gfx *gfx, s32 x, s32 y, u8 blink) {
    u8 *sheet = fileRamAddrAt(0xA10, (u32) frmPromptSheet);

    gSPDisplayList(gfx++, gPromptSetupDl);
    gDPLoadTextureBlock(gfx++, sheet, G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 16, 0, G_TX_WRAP, G_TX_WRAP,
                        G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    gSPTextureRectangle(gfx++, x << 2, y << 2, (x + 15) << 2, (y + 15) << 2, G_TX_RENDERTILE,
                        (blink & 1) * 16 << 5, 0, 4 << 10, 1 << 10);
    gSPEndDisplayList(gfx);
    return gfx;
}

Gfx *frmPromptDraw(Gfx *gfx, MarkFrame *f) {
    if (f->flags & 1) {
        gfx = frmPromptDrawAt(gfx, f->x, f->y, f->blink);
    }
    return gfx;
}

MarkFrame *promptCreate(Frame *parent) {
    MarkFrame *f = (MarkFrame *) frmCreate(parent, gFrmKindPrompt);

    if (f == NULL) {
        PRINTF("promptCreate: Error: prompt create can't\n");
        return NULL;
    }
    f->flags |= 1;
    return f;
}

void frmPromptDestroy(MarkFrame *f) {
    frmDestroy((Frame *) f);
}

s32 frmPromptKindCreate(void) {
    gFrmKindPrompt = frmNewKind("prompt", (FrameDraw) frmPromptDraw,
                                        sizeof(MarkFrame), 5, NULL);
    PRINTF("FRM_Prompt を作成\n");
    if (gFrmKindPrompt == -1) {
        return -1;
    }
    return 0;
}

void frmPromptKindDelete(void) {
    frmDeleteKind(gFrmKindPrompt);
    PRINTF("FRM_Prompt を削除\n");
}

/* win: a 256x8 border strip, copied at (x, y). */
Gfx *frmWindowDrawAt(Gfx *gfx, void *tex, s32 x, s32 y) {
    gSPDisplayList(gfx++, gWinSetupDl);
    gDPSetTextureLUT(gfx++, G_TT_NONE);
    gDPLoadTextureBlock(gfx++, tex, G_IM_FMT_RGBA, G_IM_SIZ_16b, 256, 8, 0, G_TX_WRAP, G_TX_MIRROR,
                        8, 3, G_TX_NOLOD, G_TX_NOLOD);
    gSPTextureRectangle(gfx++, x << 2, y << 2, (x + 255) << 2, (y + 7) << 2, G_TX_RENDERTILE,
                        256 << 5, 8 << 5, 4 << 10, 1 << 10);
    gSPEndDisplayList(gfx);
    return gfx;
}

Gfx *frmWindowDraw(Gfx *gfx, PicFrame *f) {
    if (f->flags & 1) {
        gfx = frmWindowDrawAt(gfx, f->tex, f->x, f->y);
    }
    return gfx;
}

PicFrame *winFrmCreate(Frame *parent) {
    PicFrame *f = (PicFrame *) frmCreate(parent, gFrmKindWin);

    if (f == NULL) {
        PRINTF("winFrmCreate: Error: window create can't\n");
        return NULL;
    }
    f->flags |= 1;
    return f;
}

void frmWindowDestroy(PicFrame *f) {
    frmDestroy((Frame *) f);
}

s32 frmWindowKindCreate(void) {
    gFrmKindWin = frmNewKind("win", (FrameDraw) frmWindowDraw,
                                        sizeof(PicFrame), 45, NULL);
    PRINTF("FRM_Window を作成\n");
    if (gFrmKindWin == -1) {
        return -1;
    }
    return 0;
}

void frmWindowKindDelete(void) {
    frmDeleteKind(gFrmKindWin);
    PRINTF("FRM_Window を削除\n");
}

/* face: a 32x32 picture, copied at (x, y). */
Gfx *frmFaceDrawAt(Gfx *gfx, void *tex, s32 x, s32 y) {
    gSPDisplayList(gfx++, gFaceSetupDl);
    gDPLoadTextureBlock(gfx++, tex, G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 32, 0, G_TX_WRAP, G_TX_MIRROR,
                        5, 5, G_TX_NOLOD, G_TX_NOLOD);
    gSPTextureRectangle(gfx++, x << 2, y << 2, (x + 31) << 2, (y + 31) << 2, G_TX_RENDERTILE,
                        32 << 5, 32 << 5, 4 << 10, 1 << 10);
    gSPEndDisplayList(gfx);
    return gfx;
}

Gfx *frmFaceDraw(Gfx *gfx, PicFrame *f) {
    if (f->flags & 1) {
        gfx = frmFaceDrawAt(gfx, f->tex, f->x, f->y);
    }
    return gfx;
}

PicFrame *faceFrmCreate(Frame *parent) {
    PicFrame *f = (PicFrame *) frmCreate(parent, gFrmKindFace);

    if (f == NULL) {
        PRINTF("faceFrmCreate: Error: facew create can't\n");
        return NULL;
    }
    f->flags |= 1;
    return f;
}

void frmFaceDestroy(PicFrame *f) {
    frmDestroy((Frame *) f);
}

s32 frmFaceKindCreate(void) {
    gFrmKindFace = frmNewKind("face", (FrameDraw) frmFaceDraw,
                                        sizeof(PicFrame), 5, NULL);
    PRINTF("FRM_Face を作成\n");
    if (gFrmKindFace == -1) {
        return -1;
    }
    return 0;
}

void frmFaceKindDelete(void) {
    frmDeleteKind(gFrmKindFace);
    PRINTF("FRM_Face を削除\n");
}
