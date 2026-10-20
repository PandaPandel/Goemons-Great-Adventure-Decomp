#include "common.h"
#include "object.h"
#include "tilemap.h"

/* A 2D tile map layer's draw (0x5A9780).
 * - The layer scrolls by a speed each frame (+0x3C/+0x40 onto +0x18/+0x1C) and wraps on the
 *   axes marked to wrap (+0x3A/+0x3B), at its size in tiles (+0x20/+0x22) x 16.
 * - It draws 16x16 tiles through a render setup (gTilemapSetupDl), translucent below alpha 256
 *   (+0x38). Up to 18 x 14 tiles fill the screen.
 * - Its palette (+0x2C) can be blended toward a second (+0x30) by +0x36 / 32. The blended
 *   palette is written to a buffer (gGfxDlHead) that advances 0x200 each call.
 * - The map (+0x24) holds a tile number per cell, with bits 15 and 14 flipping it. The tile
 *   textures (+0x28) are loaded eight at a time.
 * This is the "Bg" frame kind's (BGFrame, object.h). */

extern u16 *gGfxDlHead; /* the blended palettes' buffer */
/* tilemap.c's .data (0x8010C160-0x8010C1C0), in ROM order. */
Gfx gTilemapSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureLOD(G_TL_TILE),
    gsDPSetTextureLUT(G_TT_RGBA16),
    gsDPSetColorDither(G_CD_DISABLE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsSPEndDisplayList(),
};

/* Does nothing. */
void bgDummy(s32 a) {
}

/* Draws a tile map layer (see above), and returns the display list's next command.
 * The commands are the standard macros: gDPLoadTLUT_pal256, a 32x128 CI4 block load (eight
 * tiles) with a 16x128 render tile, and gSPTextureRectangle (gSPScisTextureRectangle when the
 * tile starts left of or above the screen), the s and t picking and flipping the tile.
 * The blend reads palA[i]/palB[i] in place for the colour parts, and copies palA[i] into an s32
 * local only after the last part, for the "& 1" (on a variable it stays last in the OR). */
Gfx *bgDraw(Gfx *gfx, Frame *frame) {
    BGFrame *f = (BGFrame *) frame;
    f32 x;
    f32 y;
    f32 x0;
    f32 y0;
    s16 n;
    s16 row;
    s16 col;
    s16 tx;
    s16 tx0;
    s16 ty0;
    s16 ty;
    u16 *buf;
    u16 i;
    s32 skip;
    u8 g;
    u8 c;
    s32 pa;

    buf = gGfxDlHead;
    gGfxDlHead += 0x100;
    f->x += f->vx;
    if (f->wrapX) {
        if (f->x >= f->w * 16) {
            f->x -= f->w * 16;
        } else if (f->x < 0) {
            f->x += f->w * 16;
        }
    }
    f->y += f->vy;
    if (f->wrapY) {
        if (f->y >= f->h * 16) {
            f->y -= f->h * 16;
        } else if (f->y < 0) {
            f->y += f->h * 16;
        }
    }
    if (f->x >= 0) {
        tx = f->x / 16;
    } else {
        tx = (f->x + 1) / 16 - 1;
    }
    if (f->y >= 0) {
        ty = f->y / 16;
    } else {
        ty = (f->y + 1) / 16 - 1;
    }
    tx0 = tx;
    ty0 = ty;
    if (f->offX != 0) {
        x = f->offX + 16;
    } else if (f->x >= 0) {
        x = (s32) f->x;
        while (x >= 16) {
            x -= 16;
        }
        if (x != 0) {
            x = 16 - x;
        } else {
            x = 16;
        }
    } else {
        x = ((s32) f->x > 0) ? (s32) f->x : -(s32) f->x;
        while (x > 16) {
            x -= 16;
        }
    }
    if (f->offY != 0) {
        y = f->offY + 8;
    } else if (f->y >= 0) {
        y = (s32) f->y;
        while (y >= 16) {
            y -= 16;
        }
        if (y != 0) {
            y = 8 - y;
        } else {
            y = 8;
        }
    } else {
        y = ((s32) f->y > 0) ? (s32) f->y : -(s32) f->y;
        while (y >= 16) {
            y -= 16;
        }
        if (y != 0) {
            y = y - 8;
        } else {
            y = 8;
        }
    }
    x0 = x;
    y0 = y;
    gSPDisplayList(gfx++, gTilemapSetupDl);
    if (f->alpha < 0x100) {
        gDPSetCombineMode(gfx++, G_CC_MODULATERGBA_PRIM, G_CC_MODULATERGBA_PRIM);
        gDPSetRenderMode(gfx++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
        gDPSetPrimColor(gfx++, 0, 0, 255, 255, 255, f->alpha);
    } else {
        gDPSetCombineMode(gfx++, G_CC_DECALRGBA, G_CC_DECALRGBA);
        gDPSetRenderMode(gfx++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
    }
    gDPPipeSync(gfx++);
    if (f->blend == 0) {
        gDPLoadTLUT_pal256(gfx++, f->palA);
    } else if (f->blend >= 32) {
        gDPLoadTLUT_pal256(gfx++, f->palB);
    } else {
        for (i = 0; i < 256; i++) {
            skip = (u8) (((PAL_GET(f->palA, i) & 0xF100) >> 11) +
                         (f32) (((PAL_GET(f->palB, i) & 0xF100) >> 11) - ((PAL_GET(f->palA, i) & 0xF100) >> 11)) / 32 * f->blend);
            g = (u8) (((PAL_GET(f->palA, i) & 0x7C0) >> 6) +
                      (f32) (((PAL_GET(f->palB, i) & 0x7C0) >> 6) - ((PAL_GET(f->palA, i) & 0x7C0) >> 6)) / 32 * f->blend);
            c = (u8) (((PAL_GET(f->palA, i) & 0x3E) >> 1) +
                      (f32) (((PAL_GET(f->palB, i) & 0x3E) >> 1) - ((PAL_GET(f->palA, i) & 0x3E) >> 1)) / 32 * f->blend);
            pa = PAL_GET(f->palA, i);
            PAL_SET(buf, i, (skip << 11) | (g << 6) | (c << 1) | (pa & 1));
        }
        gDPLoadTLUT_pal256(gfx++, buf);
    }
    for (n = 0; n < f->tileCount; n += 8) {
        gDPLoadTextureBlock(gfx++, f->img2 + (n << 8), G_IM_FMT_CI, G_IM_SIZ_8b, 16, 128, 0,
                            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                            G_TX_NOLOD, G_TX_NOLOD);
        row = 0;
        skip = 0;
        while (TRUE) {
            col = 0;
            while (TRUE) {
                if (f->wrapY == 0 && (ty < 0 || ty >= f->h)) {
                    skip = 1;
                }
                if (f->wrapX == 0 && (tx < 0 || tx >= f->w)) {
                    skip = 1;
                }
                if (!skip) {
                    if ((((u16 *) f->img1)[tx + ty * f->w] & ~(BG_CELL_MIRROR_S | BG_CELL_MIRROR_T)) >= n &&
                        (((u16 *) f->img1)[tx + ty * f->w] & ~(BG_CELL_MIRROR_S | BG_CELL_MIRROR_T)) < n + 8) {
                        gDPPipeSync(gfx++);
                        gDPSetTile(gfx++, G_IM_FMT_CI, G_IM_SIZ_8b, 2, 0, G_TX_RENDERTILE, 0,
                                   (((u16 *) f->img1)[tx + ty * f->w] & BG_CELL_MIRROR_T) ? G_TX_MIRROR : G_TX_NOMIRROR,
                                   (((u16 *) f->img1)[tx + ty * f->w] & BG_CELL_MIRROR_T) ? 7 : 0, G_TX_NOLOD,
                                   (((u16 *) f->img1)[tx + ty * f->w] & BG_CELL_MIRROR_S) ? G_TX_MIRROR : G_TX_NOMIRROR,
                                   (((u16 *) f->img1)[tx + ty * f->w] & BG_CELL_MIRROR_S) ? 4 : 0, G_TX_NOLOD);
                        if (x < 0 || y < 0) {
                            gSPScisTextureRectangle(
                                gfx++, x * 4, y * 4, (x + 16) * 4, (y + 16) * 4, G_TX_RENDERTILE,
                                (((u16 *) f->img1)[tx + ty * f->w] & BG_CELL_MIRROR_S) ? 16 << 5 : 0,
                                (((u16 *) f->img1)[tx + ty * f->w] & BG_CELL_MIRROR_T)
                                    ? ((7 - (((u16 *) f->img1)[tx + ty * f->w] & ~(BG_CELL_MIRROR_S | BG_CELL_MIRROR_T))) % 8 * 16 + 128) << 5
                                    : (((u16 *) f->img1)[tx + ty * f->w] & ~(BG_CELL_MIRROR_S | BG_CELL_MIRROR_T)) % 8 * 16 << 5,
                                1 << 10, 1 << 10);
                        } else {
                            gSPTextureRectangle(
                                gfx++, x * 4, y * 4, (x + 16) * 4, (y + 16) * 4, G_TX_RENDERTILE,
                                (((u16 *) f->img1)[tx + ty * f->w] & BG_CELL_MIRROR_S) ? 16 << 5 : 0,
                                (((u16 *) f->img1)[tx + ty * f->w] & BG_CELL_MIRROR_T)
                                    ? ((7 - (((u16 *) f->img1)[tx + ty * f->w] & ~(BG_CELL_MIRROR_S | BG_CELL_MIRROR_T))) % 8 * 16 + 128) << 5
                                    : (((u16 *) f->img1)[tx + ty * f->w] & ~(BG_CELL_MIRROR_S | BG_CELL_MIRROR_T)) % 8 * 16 << 5,
                                1 << 10, 1 << 10);
                        }
                    }
                }
                col++;
                if ((x0 >= 16 && col >= MIN(f->w, 18)) || (x0 < 16 && col >= ((f->w < 18) ? f->w + 1 : 19))) {
                    break;
                }
                tx++;
                x += 16;
                skip = 0;
                if (f->wrapX && tx >= f->w) {
                    tx -= f->w;
                }
            }
            x = x0;
            tx = tx0;
            row++;
            if ((y0 >= 8 && row >= MIN(f->h, 14)) || (y0 < 8 && row >= ((f->h < 14) ? f->h + 1 : 15))) {
                break;
            }
            ty++;
            y += 16;
            skip = 0;
            if (f->wrapY && ty >= f->h) {
                ty -= f->h;
            }
        }
        y = y0;
        ty = ty0;
    }
    gSPEndDisplayList(gfx);
    return gfx;
}
