#include "object.h"
#include "game.h"
#include "audio.h"
#include "camera.h"
#include "file.h"
#include "fileload.h"
#include "gauge.h"
#include "gfxbuf.h"
#include "gfxmgr.h"
#include "impact.h"
#include "joypad.h"
#include "mathutil.h"
#include "plmain.h"
#include "tmanager.h"

/* The "gauge" task: the two players' HUD. For each player it shows:
 * - a face picture (by character and expression);
 * - lives;
 * - two rows of three pips;
 * - money (ryo), rolling towards its real value like an odometer;
 * - an item picture.
 * A day clock, whose dial turns with the time of day, sits between the two panels. A player not
 * in play sees "press Start". Start opens a character select (Left/Right to choose, A or Start
 * to join), which makes that player in the stage next to one in play. */

extern Obj *gStageTask; /* the stage task */
extern Frame *gFramePool10; /* the frame the HUD's frame goes before */
extern s16 gFrmKindDlist; /* the "dlist " kind */
extern Gfx *gGfxDlHead; /* the display-list buffer */
/* gauge.c's .data (0x80108B70-0x801090B8), in ROM order. */
Obj *gGaugeTask = NULL; /* the gauge task */
Gfx gGaugeSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_DECALRGBA),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetTextureFilter(G_TF_POINT),
    gsSPEndDisplayList(),
};
Gfx gGaugeItemBoxDl[] = {
    gsDPPipeSync(),
    gsDPLoadTextureBlock(0x03004800, G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 28, 0, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD),
    gsSPTextureRectangle(0x0050, 0x0030, 0x00D0, 0x00A0, G_TX_RENDERTILE, 0, 0, 0x0400, 0x0400),
    gsSPTextureRectangle(0x0420, 0x0030, 0x04A0, 0x00A0, G_TX_RENDERTILE, 0, 0, 0x0400, 0x0400),
    gsSPEndDisplayList(),
};
Gfx gGaugeHudBarDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_COPY),
    gsDPSetScissor(G_SC_NON_INTERLACE, 16, 8, 304, 232),
    gsDPSetRenderMode(G_RM_NOOP, G_RM_NOOP2),
    gsDPPipeSync(),
    gsDPLoadTextureBlock(0x03000000, G_IM_FMT_RGBA, G_IM_SIZ_16b, 64, 32, 0, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD),
    gsSPTextureRectangle(0x0040, 0x0318, 0x013C, 0x0394, G_TX_RENDERTILE, 0, 0, 0x1000, 0x0400),
    gsDPPipeSync(),
    gsDPLoadTextureBlock(0x03001000, G_IM_FMT_RGBA, G_IM_SIZ_16b, 64, 32, 0, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD),
    gsSPTextureRectangle(0x0140, 0x0318, 0x023C, 0x0394, G_TX_RENDERTILE, 0, 0, 0x1000, 0x0400),
    gsDPPipeSync(),
    gsDPLoadTextureBlock(0x03002800, G_IM_FMT_RGBA, G_IM_SIZ_16b, 64, 32, 0, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD),
    gsSPTextureRectangle(0x02C0, 0x0318, 0x03BC, 0x0394, G_TX_RENDERTILE, 0, 0, 0x1000, 0x0400),
    gsDPPipeSync(),
    gsDPLoadTextureBlock(0x03003800, G_IM_FMT_RGBA, G_IM_SIZ_16b, 64, 32, 0, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD),
    gsSPTextureRectangle(0x03C0, 0x0318, 0x04BC, 0x0394, G_TX_RENDERTILE, 0, 0, 0x1000, 0x0400),
    gsDPPipeSync(),
    gsDPLoadTextureBlock(0x03004F00, G_IM_FMT_RGBA, G_IM_SIZ_16b, 64, 32, 0, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD),
    gsSPEndDisplayList(),
};
Gfx gGaugeHudBarCenterDl[] = {
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPPipeSync(),
    gsDPLoadTextureBlock(0x03002000, G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 32, 0, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD),
    gsDPSetCycleType(G_CYC_COPY),
    gsDPSetRenderMode(G_RM_NOOP, G_RM_NOOP2),
    gsSPTextureRectangle(0x0240, 0x0318, 0x02BC, 0x0394, G_TX_RENDERTILE, 0, 0, 0x1000, 0x0400),
    gsSPEndDisplayList(),
};
Gfx gGaugeModulatePrimDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, 0, 0, 0, TEXEL0, 0, 0, 0, ENVIRONMENT, 0, 0, 0, TEXEL0),
    gsDPSetEnvColor(0xFF, 0xFF, 0xFF, 0xFF),
    gsSPEndDisplayList(),
};
Gfx gGaugeFaceSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_COPY),
    gsDPSetRenderMode(G_RM_NOOP, G_RM_NOOP2),
    gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_DECALRGBA),
    gsSPEndDisplayList(),
};
u8 *gGaugeFaceTexTable[5][3] = { /* 11 faces by character and expression, the items from [3][2] */
    { (u8 *) 0x030067C0, (u8 *) 0x03006AE0, (u8 *) 0x03006E00 },
    { (u8 *) 0x03007120, (u8 *) 0x03007440, (u8 *) 0x03007760 },
    { (u8 *) 0x030083E0, (u8 *) 0x03008700, (u8 *) 0x03008A20 },
    { (u8 *) 0x03007A80, (u8 *) 0x03007DA0, (u8 *) 0x030080C0 },
    { (u8 *) 0x03009540, (u8 *) 0x030099C0, (u8 *) 0x03009E40 },
};
Vtx gGaugeClockVtx[4] = { /* the quad of the clock */
    { { { -16, -16, 0 }, 0x0000, { -32, 4048 }, { 0xFF, 0x10, 0x10, 0xFF } } },
    { { { -16, 16, 0 }, 0x0000, { -32, -48 }, { 0x00, 0xFF, 0xFF, 0xFF } } },
    { { { 16, 16, 0 }, 0x0000, { 4064, -48 }, { 0x10, 0x10, 0xFF, 0xFF } } },
    { { { 16, -16, 0 }, 0x0000, { 4064, 4048 }, { 0x10, 0xFF, 0x10, 0xFF } } },
};
Gfx gGaugeClockSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD),
    gsSPSetGeometryMode(G_SHADE | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_DECALRGBA),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsSPEndDisplayList(),
};
Gfx gGaugeClockDl[] = {
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPPipeSync(),
    gsDPLoadTextureBlock(0x03008D40, G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 32, 0, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD),
    gsSPVertex(gGaugeClockVtx, 4, 0),
    gsSP1Quadrangle(0, 1, 2, 3, 0),
    gsSPEndDisplayList(),
};
Gfx gGaugeMoneySetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, 0, 0, 0, TEXEL0, 0, 0, 0, ENVIRONMENT, 0, 0, 0, TEXEL0),
    gsDPSetEnvColor(0x00, 0x00, 0x00, 0x00),
    gsDPPipeSync(),
    gsDPLoadTextureBlock(0x03005F00, G_IM_FMT_IA, G_IM_SIZ_8b, 8, 176, 0, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD),
    gsSPEndDisplayList(),
};
f32 gGaugePlayerZStart[3] = { /* the z offset of the first player, by the number in play - 2 */
    3.0f, 3.0f, 3.0f,
};
f32 gGaugePlayerZStep[3] = { -6.0f, -3.0f, -2.0f }; /* the step between them, likewise */
extern u8 gaugeJoinPanelTex[]; /* textures in segment 3 */
extern u8 gaugeHudTex[];
extern u8 gaugeJoinPanel1Tex[];
extern u8 gaugeJoinPanel2Tex[];


#define PLAYER(i) (&gSaveData.players[i])
#define IN_PLAY(p) ((p)->chara >= 0 && (p)->lives > 0)
#define GAUGE(obj) ((GaugeWork *) ((u32) (obj) + sizeof(Obj)))
#define PLAYER_WORK(i) ((PlayerObj *) ((u32) gGameWork.players[i] + sizeof(Obj)))

/* Draws a number from a column of digits (w by h each): digits 2 or 3 draw that many with
 * leading zeros; otherwise div is the first place and leading zeros are left out. */
Gfx *gaugeDrawNumber(Gfx *gfx, s32 x, s32 y, s32 w, s32 h, s32 space, s32 value,
                            s32 div) {
    s32 shown = 0;
    s32 d;

    if (div == 2) {
        shown = 1;
        div = 10;
    }
    if (div == 3) {
        shown = 1;
        div = 100;
    }
    while (div > 0) {
        d = value / div;
        value %= div;
        if (d != 0 || shown || div == 1) {
            gSPTextureRectangle(gfx++, x << 2, y << 2, (x + w) << 2, (y + h) << 2,
                                G_TX_RENDERTILE, 0, (d * h) << 5, 1 << 10, 1 << 10);
            shown = 1;
        }
        x += w + space;
        div /= 10;
    }
    return gfx;
}

/* A player's two rows of three pips (grey when not in play). */
Gfx *gaugeDrawPips(Gfx *gfx, s32 player, s32 a, s32 b) {
    s32 i;
    s32 x;
    s32 s;

    if (!IN_PLAY(PLAYER(player))) {
        a = 0;
        b = 0;
    }
    for (i = 0; i < 3; i++) {
        x = player * 128 + 58 + i * 10;
        s = (i < a) ? 8 : 16;
        gSPTextureRectangle(gfx++, x << 2, 200 << 2, (x + 7) << 2, 212 << 2, G_TX_RENDERTILE,
                            s << 5, 0, 4 << 10, 1 << 10);
    }
    for (i = 0; i < 3; i++) {
        x = player * 128 + 58 + i * 10;
        s = (i < b) ? 0 : 16;
        gSPTextureRectangle(gfx++, x << 2, 215 << 2, (x + 7) << 2, 227 << 2, G_TX_RENDERTILE,
                            s << 5, 0, 4 << 10, 1 << 10);
    }
    return gfx;
}

/* A player's name plate, then the character's name (16 pixels each in the texture's s). */
Gfx *gaugeDrawName(Gfx *gfx, s32 player, s32 who) {
    s32 pad[3];
    s32 x;

    gDPPipeSync(gfx++);
    gDPSetAlphaCompare(gfx++, G_AC_THRESHOLD);
    gDPSetBlendColor(gfx++, 0, 0, 0, 1);
    x = player * 127 + 86;
    gSPTextureRectangle(gfx++, x << 2, 213 << 2, (x + 15) << 2, 228 << 2, G_TX_RENDERTILE,
                        48 << 5, 0, 4 << 10, 1 << 10);
    if (who < 0) {
        return gfx;
    }
    if (!IN_PLAY(PLAYER(player))) {
        return gfx;
    }
    gSPTextureRectangle(gfx++, x << 2, 197 << 2, (x + 15) << 2, 212 << 2, G_TX_RENDERTILE,
                        (who * 16) << 5, 16 << 5, 4 << 10, 1 << 10);
    return gfx;
}

/* A player's lives: an icon and two digits. */
Gfx *gaugeDrawLives(Gfx *gfx, s32 player, s32 lives) {
    s32 x;

    if (!IN_PLAY(PLAYER(player))) {
        return gfx;
    }
    x = player * 127;
    gSPTextureRectangle(gfx++, (x + 102) << 2, 200 << 2, (x + 110) << 2, 208 << 2, G_TX_RENDERTILE,
                        0, 80 << 5, 1 << 10, 1 << 10);
    return gaugeDrawNumber(gfx, x + 110, 200, 8, 8, 0, lives, 2);
}

/* A player not in play: "press Start" (1-2) or the character select (3-6).
 * Matched 2026-10-04; it was parked for registers: the frame was 0x20 against the game's 0x48,
 * and x + 16's uopt colouring priority put it in t0 instead of v1, until case 3-6's x was
 * computed FIRST, before its inner switch (the batch-48 form: "x + width - 1", "(x + width) <<
 * 2", "x + 16 + 7"), which gave the game's whole register map at once with no FAKE needed. */
Gfx *gaugeDrawJoinPanel(Gfx *gfx, GaugeWork *w, s32 player) {
    s32 s;
    s32 t;
    s32 h;
    s32 width;
    s32 y;
    s32 x;
    u8 *tex;
    u8 panel = w->panel[player] & 0x7F;

    width = 16;
    h = 16;
    if (panel == 0) {
        return gfx;
    }
    y = 0xCD;
    switch (panel) {
        case 1:
        case 2:
            switch (panel) {
                case 1:
                    tex = gaugeJoinPanel1Tex;
                    h = 11;
                    y = 0xCF;
                    break;
                case 2:
                    tex = gaugeJoinPanel2Tex;
                    break;
            }
            x = player * 254 + 25;
            gSPDisplayList(gfx++, gGaugeModulatePrimDl);
            gDPPipeSync(gfx++);
            gDPLoadTextureBlock_4b(gfx++, tex, G_IM_FMT_I, width, h, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                   G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                                   G_TX_NOLOD);
            gSPTextureRectangle(gfx++, x << 2, y << 2, (x + width) << 2, (y + h) << 2,
                                G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
            break;
        case 3:
        case 4:
        case 5:
        case 6:
            x = player * 254 + 25;
            switch (panel) {
                case 3:
                    s = 0; t = 16;
                    break;
                case 4:
                    s = 16; t = 16;
                    break;
                case 5:
                    s = 32; t = 16;
                    break;
                case 6:
                    s = 48; t = 16;
                    break;
            }
            gDPPipeSync(gfx++);
            gDPLoadTextureBlock(gfx++, gaugeJoinPanelTex, G_IM_FMT_RGBA, G_IM_SIZ_16b, 64, 32, 0,
                                G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK,
                                G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gSPTextureRectangle(gfx++, x << 2, y << 2, (x + width - 1) << 2, (y + 16 - 1) << 2,
                                G_TX_RENDERTILE, s << 5, t << 5, 4 << 10, 1 << 10);
            if (w->panel[player] & 0x80) {
                gSPTextureRectangle(gfx++, (x - 8) << 2, y << 2, (x - 1) << 2, (y + 16 - 1) << 2,
                                    G_TX_RENDERTILE, 24 << 5, 0, 4 << 10, 1 << 10);
                gSPTextureRectangle(gfx++, (x + width) << 2, y << 2, (x + 16 + 7) << 2,
                                    (y + 16 - 1) << 2, G_TX_RENDERTILE, 32 << 5, 0, 4 << 10,
                                    1 << 10);
            }
            break;
    }
    return gfx;
}

/* A player's face, or the join panel. The face is the player's character at its weapon level;
 * the caller also passes the character and one more byte of its stats, both unused here. */
Gfx *gaugeDrawFace(Gfx *gfx, GaugeWork *w, s32 player, s32 chara, s32 unused) {
    s32 x;

    if (!IN_PLAY(PLAYER(player))) {
        return gaugeDrawJoinPanel(gfx, w, player);
    }
    gSPDisplayList(gfx++, gGaugeFaceSetupDl);
    gDPPipeSync(gfx++);
    gDPLoadTextureBlock(gfx++, gGaugeFaceTexTable[PLAYER(player)->chara][PLAYER(player)->weaponLevel],
                        G_IM_FMT_RGBA, G_IM_SIZ_16b, 20, 20, 0, G_TX_NOMIRROR | G_TX_CLAMP,
                        G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);
    x = player * 254 + 24;
    gSPTextureRectangle(gfx++, x << 2, 204 << 2, (x + 19) << 2, 223 << 2, G_TX_RENDERTILE, 0, 0,
                        4 << 10, 1 << 10);
    return gfx;
}

/* A player's item picture (items 1-3). */
Gfx *gaugeDrawItem(Gfx *gfx, s32 player) {
    Unk88282 *p = PLAYER(player);
    s32 item = p->food;
    s32 x;

    if (!IN_PLAY(p)) {
        return gfx;
    }
    if (item <= 0 || item >= 4) {
        return gfx;
    }
    gDPPipeSync(gfx++);
    gDPLoadTextureBlock(gfx++, gGaugeFaceTexTable[3][2 + item], G_IM_FMT_RGBA, G_IM_SIZ_16b, 24, 24, 0,
                        G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK,
                        G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    x = player * 244;
    gSPTextureRectangle(gfx++, (x + 24) << 2, 14 << 2, (x + 48) << 2, 38 << 2, G_TX_RENDERTILE, 0,
                        0, 1 << 10, 1 << 10);
    return gfx;
}

/* The day clock: an ortho view, and its dial turned by the time of day (half a turn from noon
 * to midnight). The angle comes from gGameWork.time: the caller's (0.0f) is unused. */
Gfx *gaugeDrawClock(Gfx *gfx, Mtx *m, f32 unused) {
    guOrtho(m, 0.0f, 320.0f, 240.0f, 0.0f, -200.0f, 200.0f, 1.0f);
    gSPMatrix(gfx++, OS_K0_TO_PHYSICAL(m), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    mathMtxTransRotScaleL(m + 1, 160.0f, 214.0f, 0.0f, 0.0f, 0.0f,
                           (f32) (gGameWork.time - (12 << 16)) / (12 << 16) * PI, 1.0f, 1.0f,
                           1.0f);
    gSPMatrix(gfx++, K0_TO_PHYS(m + 1), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_PUSH);
    gSPDisplayList(gfx++, gGaugeClockSetupDl);
    gSPDisplayList(gfx++, gGaugeClockDl);
    gSPPopMatrix(gfx++, G_MTX_MODELVIEW);
    return gfx;
}

/* A player's money: three digits that roll (a quarter of a digit a step) towards the real
 * value, faster the further off it is. i is reused for the quarter and v for the difference
 * (one register each, as in the original); x is set after the test (uopt hoists it to the
 * entry, after player's copy to a3); the t argument is (d * 4) << 5, which spends one more
 * temporary register than d << 7. pad: two slots below d (frame 0x48). */
Gfx *gaugeDrawMoney(Gfx *gfx, s32 player, GaugeWork *w) {
    s32 d[3];
    s32 x;
    s32 div;
    s32 v;
    s32 i;
    s32 pad[2];

    if (!IN_PLAY(PLAYER(player))) {
        w->money[player] = 0;
    }
    x = player * 127 + 105;
    div = 100;
    v = w->money[player] / 4;
    for (i = 0; i < 3; i++) {
        d[i] = v / div;
        v %= div;
        div /= 10;
    }
    i = w->money[player] & 3;
    d[2] = d[2] * 4 + i;
    d[1] *= 4;
    if (d[2] >= 0x25) {
        d[1] += i;
    }
    d[0] *= 4;
    if (d[1] >= 0x25) {
        d[0] += i;
    }
    for (i = 0; i < 3; i++) {
        gSPTextureRectangle(gfx++, x << 2, 212 << 2, (x + 8) << 2, 228 << 2, G_TX_RENDERTILE, 0,
                            (d[i] * 4) << 5, 1 << 10, 1 << 10);
        x += 12;
    }
    if (PLAYER(player)->money < 0) {
        PLAYER(player)->money = 0;
    } else if (PLAYER(player)->money >= 1000) {
        PLAYER(player)->money = 999;
    }
    v = PLAYER(player)->money * 4 - w->money[player];
    if (v > 0) {
        if (v >= 101) {
            w->money[player] += 20;
        } else if (v >= 41) {
            w->money[player] += 4;
        } else {
            w->money[player] += 1;
        }
    } else if (v < 0) {
        if (v < -100) {
            w->money[player] -= 20;
        } else if (v < -40) {
            w->money[player] -= 4;
        } else {
            w->money[player] -= 1;
        }
    }
    return gfx;
}

/* The items. */
Gfx *gaugeDrawItems(Gfx *gfx, GaugeWork *w) {
    gSPDisplayList(gfx++, gGaugeSetupDl);
    gSPDisplayList(gfx++, gGaugeItemBoxDl);
    return gaugeDrawItem(gaugeDrawItem(gfx, 0), 1);
}

/* The rest of the HUD: pips, names, faces, the clock, lives and money. The first three loops
 * are do-whiles (the gfx copy before the branch, the pointer's step in its delay slot) with
 * the counter's reset on the loop's own line (IDO schedules by line). */
Gfx *gaugeDrawHud(Gfx *gfx, GaugeWork *w, Mtx *m) {
    s32 i;
    Unk88282 *p;

    gSPDisplayList(gfx++, gGaugeHudBarDl);
    i = 0; do {
        p = PLAYER(i);
        gfx = gaugeDrawPips(gfx, i, p->life, p->armor);
    } while (++i < 2);
    i = 0; do {
        p = PLAYER(i);
        gfx = gaugeDrawName(gfx, i, p->chara);
    } while (++i < 2);
    i = 0; do {
        p = PLAYER(i);
        gfx = gaugeDrawFace(gfx, w, i, p->chara, p->unused6);
    } while (++i < 2);
    gfx = gaugeDrawClock(gfx, m, 0.0f);
    gSPDisplayList(gfx++, gGaugeHudBarCenterDl);
    gSPDisplayList(gfx++, gGaugeModulatePrimDl);
    gDPPipeSync(gfx++);
    gDPLoadTextureBlock(gfx++, gaugeHudTex, G_IM_FMT_IA, G_IM_SIZ_8b, 8, 88, 0,
                        G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK,
                        G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    for (i = 0; i < 2; i++) {
        if (PLAYER(i)->lives >= 101) {
            PLAYER(i)->lives = 100;
        }
        gfx = gaugeDrawLives(gfx, i, PLAYER(i)->lives - 1);
    }
    gSPDisplayList(gfx++, gGaugeMoneySetupDl);
    for (i = 0; i < 2; i++) {
        gfx = gaugeDrawMoney(gfx, i, w);
    }
    return gfx;
}

/* The gauge task's handler. Draw (message 3) builds the HUD's list in the display-list buffer:
 * segment 3, the items, black bars above and below the letterbox, then the rest.
 * w is set at its declaration, the scissor is read as named fields, and the lives are a 4-pass
 * loop that IDO unrolls into absolute loads, as the game has them. */
s32 gaugeProc(Obj *obj, s16 msg, void *arg) {
    GaugeWork *w = GAUGE(obj);
    Mtx *m;
    s32 i;
    void (**proc)(s32 player, GaugeWork *w);
    s32 pad[2];

    switch (msg) {
        case 2:
            if (gGaugeTask != NULL) {
                return -1;
            }
            gGaugeTask = obj;
            obj->name = "gauge";
            floadLoadSet(0x10E);
            floadLoadSet(0x10D);
            w->frame = (DLFrame *) frmCreatePrev(gFramePool10, gFrmKindDlist);
            if (w->frame == NULL) {
                return -1;
            }
            w->seg = OS_K0_TO_PHYSICAL(fileRamAddr(floadGetSetFile(0x10E, 0)));
            w->money[0] = PLAYER(0)->money * 4;
            w->money[1] = PLAYER(1)->money * 4;
            w->proc[0] = gaugeWaitStart;
            w->proc[1] = gaugeWaitStart;
            for (i = 0; i < 4; i++) {
                w->lives[i] = PLAYER(i)->lives;
            }
            break;
        case 1:
            gGaugeTask = NULL;
            (*frmDestroy)((Frame *) GAUGE(obj)->frame);
            break;
        case 0:
            w->timer++;
            for (i = 0, proc = w->proc; i < 2; i++, proc++) {
                if (*proc != NULL && !IN_PLAY(PLAYER(i))) {
                    (*proc)(i, w);
                }
            }
            break;
        case 3:
            m = gfxAllocMtx(2);
            w->frame->dl = gGfxDlHead;
            gSPSegment(gGfxDlHead++, 3, w->seg);
            gGfxDlHead = gaugeDrawItems(gGfxDlHead, w);
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombineLERP(gGfxDlHead++, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0,
                              ENVIRONMENT, 0, 0, 0, ENVIRONMENT);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 0);
            gDPFillRectangle(gGfxDlHead++, gGameState.scissor.ulx, gGameState.scissor.uly - 1, gGameState.scissor.lrx + 1,
                             gGameState.scissor.uly + 1);
            gDPFillRectangle(gGfxDlHead++, gGameState.scissor.ulx, gGameState.scissor.lry - 1, gGameState.scissor.lrx + 1,
                             gGameState.scissor.lry);
            gGfxDlHead = gaugeDrawHud(gGfxDlHead, w, m);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The work of the n-th player in play (1-4), or NULL. */
PlayerObj *gaugeGetNthPlayer(s32 n) {
    s32 i;
    s32 c = 0;

    for (i = 0; i < 4; i++) {
        if (gGameWork.players[i] != NULL) {
            c++;
        }
        if (c == n) {
            return PLAYER_WORK(i);
        }
    }
    return NULL;
}

/* Spreads the players in play along z by their number (placed, or moved). */
void gaugeSpreadPlayers(s32 reset) {
    PlayerObj *o;
    f32 z;
    s32 n;
    s32 i;

    for (i = 0, n = 0; i < 4; i++) {
        if (PLAYER(i)->chara >= 0 && PLAYER(i)->chara < 4) {
            n++;
        }
    }
    z = gGaugePlayerZStart[n - 2];
    for (i = 1; i <= n; i++) {
        o = gaugeGetNthPlayer(i);
        if (o != NULL) {
            if (reset) {
                o->z = z;
                o->startZ = z;
            } else {
                o->z += z;
            }
        }
        z += gGaugePlayerZStep[n - 2];
    }
}

/* The first player in play whose work has not the 0x200 flag. */
PlayerObj *gaugeJoinPartner(void) {
    PlayerObj *o;
    s32 i;

    for (i = 0; i < 4; i++) {
        if (gGameWork.players[i] != NULL) {
            o = PLAYER_WORK(i);
            if (!(o->flags & 0x200)) {
                return o;
            }
        }
    }
    return NULL;
}

/* A player joins as character `who`, next to a player in play. With no lives, the other player
 * gives one up (if it has two). The mask is an s32 set at its declaration, so IDO keeps
 * 0x20000010 in a register and nors it (managerpl.c plSpawnJoined's form). */
void gaugeJoinPlayer(s32 player, s32 who) {
    PlInit init;
    Obj *o;
    PlayerObj *near;
    s32 mask = 0x20000010;

    near = gaugeJoinPartner();
    if (near == NULL) {
        return;
    }
    PLAYER(player)->chara = who;
    plMakeInit(&init, player, who);
    init.zone = near->zone;
    init.place = near->x;
    init.height = near->y;
    init.side = near->z;
    init.flags &= ~mask;
    init.flags |= near->flags & mask;
    init.flags3 |= 1;
    if (init.flags & 0x10) {
        init.action = 0x124;
    } else {
        init.action = 0x123;
        init.height += 2.0f;
    }
    o = plCreate(gStageTask, &init);
    if (o != NULL) {
        camAiAddLookat(o, player);
        gaugeSpreadPlayers(near->zone);
        if (PLAYER(player)->lives <= 0) {
            if (PLAYER(player ^ 1)->lives >= 2) {
                sndPlay(0x458);
                PLAYER(player ^ 1)->lives--;
                PLAYER(player)->lives++;
                PLAYER(player)->life = 3;
            }
        }
    } else {
        PRINTF("!!!! プレイヤーの作成に失敗しました !!!!\n");
        gGameWork.partake = 1;
        PLAYER(player)->chara = -1;
    }
}

/* The next character after who (by step), of the unk9 available, skipping those in play. */
s32 gaugeNextChara(s32 who, s32 step) {
    s32 n;
    s32 i;

    n = gSaveData.charaCount;
    if (n < 2) {
        n = gSaveData.charaCount = 2;
    }
loop:
    who += step;
    if (who < 0) {
        who += n;
    } else if (who >= n) {
        who -= n;
    }
    for (i = 0; i < 4; i++) {
        if (who == PLAYER(i)->chara && PLAYER(i)->lives > 0) {
            goto loop;
        }
    }
    return who;
}

/* A player not in play: "press Start" (blinking), and Start opens the character select. */
void gaugeWaitStart(s32 player, GaugeWork *w) {
    if (gGameWork.partake == 0 && PLAYER(player)->lives + PLAYER(player ^ 1)->lives >= 2) {
        if (joyGetHeldTime(player, START_BUTTON) == 1) {
            joySetStickDpad(player, 1);
            w->choice[player] = gaugeNextChara(gSaveData.charaCount - 1, 1);
            w->proc[player] = gaugeSelectChara;
            return;
        }
        w->panel[player] = 1;
    } else {
        w->panel[player] = 2;
    }
    if ((w->timer & 0x1F) >= 21) {
        w->panel[player] = 0;
    }
}

/* The character select: Left/Right choose, A or Start joins. */
void gaugeSelectChara(s32 player, GaugeWork *w) {
    if (gGameWork.partake != 0) {
        joySetStickDpad(player, 0);
        w->proc[player] = gaugeWaitStart;
        return;
    }
    if (joyGetHeldTime(player, L_JPAD) == 1) {
        w->choice[player] = gaugeNextChara(w->choice[player], -1);
    }
    if (joyGetHeldTime(player, R_JPAD) == 1) {
        w->choice[player] = gaugeNextChara(w->choice[player], 1);
    }
    w->panel[player] = w->choice[player] + 3;
    if ((w->timer & 0x1F) >= 21) {
        w->panel[player] |= 0x80;
    }
    if (joyGetHeldTime(player, A_BUTTON | START_BUTTON) == 1) {
        joySetStickDpad(player, 0);
        w->proc[player] = gaugeWaitStart;
        gaugeJoinPlayer(player, w->choice[player]);
    }
}

/* The letterbox's bottom edge: with the HUD on (1) its frame is drawn and the edge is at 198;
 * off, at 232. */
void gaugeSetOn(s32 on) {
    GaugeWork *w = (GaugeWork *) ((u32) gGaugeTask + sizeof(Obj));

    if (gGaugeTask == NULL) {
        return;
    }
    if (on == 1) {
        gGameState.scissor.ulx = 16;
        gGameState.scissor.uly = 8;
        gGameState.scissor.lrx = 304;
        gGameState.scissor.lry = 198;
        w->frame->f.flags |= 2;
        return;
    }
    gGameState.scissor.ulx = 16;
    gGameState.scissor.uly = 8;
    gGameState.scissor.lrx = 304;
    gGameState.scissor.lry = 232;
    w->frame->f.flags &= ~2;
}
