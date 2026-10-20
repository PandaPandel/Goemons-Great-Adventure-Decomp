#include "object.h"
#include "ascii.h"
#include "gfxmgr.h"

/* The "ascii" frame kind: a line of text in the 8x8 debug font, a 128x32 4-bit texture
 * (0x800C7310) with its palette (0x800379C8). Each character's texture coordinates are in a
 * table from ' ' on (0x800379F0). */

/* ascii.c's .data (0x80108AD0-0x80108B48), in ROM order. */
Gfx gAsciiSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_COPY),
    gsDPPipelineMode(G_PM_NPRIMITIVE),
    gsDPSetTextureLOD(G_TL_TILE),
    gsDPSetTextureLUT(G_TT_RGBA16),
    gsDPSetTextureDetail(G_TD_CLAMP),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureFilter(G_TF_POINT),
    gsDPSetTextureConvert(G_TC_FILT),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_DECALRGBA),
    gsDPSetRenderMode(G_RM_NOOP, G_RM_NOOP2),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsSPEndDisplayList(),
};
extern u8 gAsciiFont[]; /* the font */
extern u16 gAsciiFontTlut[]; /* its palette */
extern s16 gAsciiGlyphTable[]; /* each character's place in it: s, t from ' ' on */
extern s16 gFrmKindAscii; /* the "ascii" kind */

/* An "ascii" frame's draw: the font's setup, then one clipped texture rectangle
 * (gSPScisTextureRectangle) for each character that is not a space. The table is read as
 * s16 pairs indexed "(*p - ' ') << 1": a Glyph[] index folds the -32 into the base. */
Gfx *asciiDraw(Gfx *gfx, AsciiFrame *f) {
    u8 *p;
    s16 dx;
    s16 x;
    s16 y;
    s16 x2;
    s16 y2;
    s16 s;
    s16 t;

    p = (u8 *) f->str;
    if (p == NULL) {
        return gfx;
    }
    gSPDisplayList(gfx++, gAsciiSetupDl);
    gDPPipeSync(gfx++);
    gDPLoadTextureBlock_4b(gfx++, gAsciiFont, G_IM_FMT_CI, 128, 32, 0, G_TX_CLAMP,
                           G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    gDPLoadTLUT_pal16(gfx++, 0, gAsciiFontTlut);
    dx = 0;
    while (*p != '\0') {
        if (*p != ' ') {
            x = f->x + dx;
            y = f->y;
            x2 = x + 7;
            y2 = y + 7;
            s = gAsciiGlyphTable[(*p - ' ') << 1];
            t = gAsciiGlyphTable[((*p - ' ') << 1) + 1];
            gSPScisTextureRectangle(gfx++, x << 2, y << 2, x2 << 2, y2 << 2,
                                    G_TX_RENDERTILE, s, t, 4 << 10, 1 << 10);
        }
        p++;
        dx += 8;
    }
    return gfx;
}

/* A line of text at (x, y) under parent. */
AsciiFrame *asciiNew(Frame *parent, s16 x, s16 y, char *str) {
    AsciiFrame *f = (AsciiFrame *) frmCreate(parent, gFrmKindAscii);

    f->x = x;
    f->y = y;
    f->str = str;
    return f;
}

/* Makes the "ascii" kind, of count frames. */
void asciiNewKind(s32 count) {
    gFrmKindAscii = frmNewKind("ascii", (FrameDraw) asciiDraw,
                                        sizeof(AsciiFrame), count, NULL);
}
