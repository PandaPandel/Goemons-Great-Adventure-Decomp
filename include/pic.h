#ifndef PIC_H
#define PIC_H

/* The "PIC" image decoder (src/pic.c): Yanagisawa's X68000 PIC format. Colours come from a
 * 128-entry move-to-front cache or as new values; runs of repeated pixels are coded as lengths,
 * and a colour can be chained down the lines below it. */

#include "common.h"

#define PIC_NO_TRANSPARENT 0x80000000 /* in Pic.transparent */

typedef struct Pic {
    /* 0x00 */ s32 width;
    /* 0x04 */ s32 height;
    /* 0x08 */ s32 bits; /* 4 or 8 (indexed), 15 or 16 (direct colour) */
    /* 0x0C */ s32 size; /* the decoded size, palette included */
    /* 0x10 */ s32 type; /* 0 direct colour, 1 indexed (the palette follows the pixels), 2 mode 4 */
    /* 0x14 */ s32 transparent; /* the colour to clear, or PIC_NO_TRANSPARENT */
    /* 0x18 */ u8 *src;
    /* 0x1C */ void *dst;
    /* 0x20 */ u16 *palette;
    /* 0x24 */ void (*put)(struct Pic *pic, s32 x, s32 y, s32 color);
    /* 0x28 */ s32 (*get)(struct Pic *pic, s32 x, s32 y);
} Pic; /* size 0x2C */

Pic *picOpen(u8 *src, Pic *pic);           /* read the header */
Pic *picDecode(u8 *src, void *dst, Pic *pic); /* decode the image into dst */

u16 *picDecode15(u8 *src, u16 *dst, Pic *pic);
u16 *picDecode16(u8 *src, u16 *dst, Pic *pic);
u8 *picDecode8(u8 *src, u8 *dst, Pic *pic);
u8 *picDecode4(u8 *src, u8 *dst, Pic *pic);
void picReadPalette(s32 n, Pic *pic);
void picReadHeader(Pic *pic);
void picDecodeDirect(Pic *pic);
void picChainDirect(Pic *pic, s32 x, s32 y, s32 color);
void picDecodeSkips(Pic *pic);
void picChainSkips(Pic *pic, s32 x, s32 y, s32 color);
s32 picReadLength(void);
s32 picReadColor(Pic *pic);
void picCacheClear(void);
s32 picCacheNew(Pic *pic, s32 color);
s32 picCacheGet(Pic *pic, s32 i);
s32 picGetDirect(Pic *pic, s32 x, s32 y);
void picPut15(Pic *pic, s32 x, s32 y, s32 color);
void picClearPixel(s32 x, s32 y);
void picPut16(Pic *pic, s32 x, s32 y, s32 color);
s32 picGet8(Pic *pic, s32 x, s32 y);
void picPut8(Pic *pic, s32 x, s32 y, s32 color);
s32 picGet4(Pic *pic, s32 x, s32 y);
void picPut4(Pic *pic, s32 x, s32 y, s32 color);
s32 picParseHex(u8 *hex);
void picClearTransparent(Pic *pic);
void picMarkWritten(u8 *bits, s32 i);
void picUnusedStub(void);
s32 picIsWritten(u8 *bits, s32 i);
void *picScratchAlloc(u32 size, s32 *allocated);
void picBitsInit(u8 *src);
s32 picReadBit(void);
s32 picReadBits(s32 n);

#endif
