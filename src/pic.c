#include "pic.h"
#include "game.h"
#include "heap.h"

/* The game's debug print, compiled to nothing but its arguments: the strings stay in .rodata,
 * in source order (IDO_NOTES 144, 201). The arguments are not known: no code is left. */
#define DPRINTF

/* The "PIC" image decoder (include/pic.h). The pixels are decoded into the caller's buffer as
 * RGBA5551 (15/16-bit) or as indices (8/4-bit, with an RGBA5551 palette after them). A bitmap of
 * the pixels written so far lets the indexed decoders fill each run from the pixel before it. */

typedef struct PicColor {
    /* 0x0 */ s16 color;
    /* 0x2 */ s16 prev;
    /* 0x4 */ s16 next;
} PicColor; /* size 0x6 */

/* The bit reader. */
extern s32 gPicBitsUnused0;
extern s32 gPicBitsLeft; /* bits left in the current byte */
extern s32 gPicBitsUnused8;
extern u32 gPicBitsByte; /* the current byte */
extern u8 *gPicBitsNext; /* the next byte */

extern s32 gPicOpenUnused8;
extern s32 gPicWidth; /* the width */
extern s32 gPicHeight; /* the height */
extern s32 gPicTransparentColor; /* the transparent colour */
extern u8 *gPicOpenSrc;
extern u8 gPicOpenFirstByte;
extern PicColor gPicColorCache[128]; /* the colour cache, a ring */
extern s32 gPicColorCacheNewest;           /* its newest entry */
extern u16 *gPicPixels16;          /* the pixels, 15/16-bit */
extern u8 *gPicPixels8;           /* the pixels, 8/4-bit */
extern u8 *gPicWrittenBits;           /* the bitmap of pixels written */
extern u16 gFramebuffers[3][SCREEN_WIDTH * SCREEN_HEIGHT]; /* the framebuffers */


/* Read a PIC's header. */
Pic *picOpen(u8 *src, Pic *pic) {
    picBitsInit(src);
    pic->src = src;
    gPicOpenUnused8 = 8;
    gPicOpenSrc = pic->src;
    gPicOpenFirstByte = *gPicOpenSrc;
    picCacheClear();
    picReadHeader(pic);
    return pic;
}

/* Decode the image into dst, by its depth. */
Pic *picDecode(u8 *src, void *dst, Pic *pic) {
    switch (pic->bits) {
        case 15:
            picCacheClear();
            pic->put = picPut15;
            pic->get = picGetDirect;
            picDecode15(src, dst, pic);
            break;
        case 16:
            picCacheClear();
            pic->put = picPut16;
            pic->get = picGetDirect;
            picDecode16(src, dst, pic);
            break;
        case 4:
            pic->put = picPut4;
            pic->get = picGet4;
            picDecode4(src, dst, pic);
            break;
        case 8:
            pic->put = picPut8;
            pic->get = picGet8;
            picDecode8(src, dst, pic);
            break;
        default:
            DPRINTF("picDecode: unknown format\n");
            break;
    }
    return pic;
}

/* 15-bit: cleared, decoded, then the transparent colour cleared again. */
u16 *picDecode15(u8 *src, u16 *dst, Pic *pic) {
    s32 i;
    s32 n;

    gPicPixels16 = dst;
    pic->dst = dst;
    n = pic->width * pic->height;
    for (i = 0; i < n; i++) {
        gPicPixels16[i] = 0;
    }
    picDecodeDirect(pic);
    if (pic->transparent != PIC_NO_TRANSPARENT) {
        picClearTransparent(pic);
    }
    return dst;
}

/* 16-bit: decoded, then each pixel not written repeats the one before it. (The frame has 16
 * bytes above color; the first two statements share a line, IDO_NOTES rule 12.) */
u16 *picDecode16(u8 *src, u16 *dst, Pic *pic) {
    s32 allocated = 0;
    s32 n;
    s32 i;
    s32 pad[4];
    s32 color;

    gPicPixels16 = dst; pic->dst = gPicPixels16;
    n = pic->width * pic->height;
    gPicWrittenBits = picScratchAlloc((n / 8 + 7) & ~7, &allocated);
    picDecodeSkips(pic);
    for (i = 0; i < n; i++) {
        if (picIsWritten(gPicWrittenBits, i)) {
            color = gPicPixels16[i];
        }
        gPicPixels16[i] = color;
    }
    if (allocated) {
        heapFree(gPicWrittenBits);
    }
    return dst;
}

/* 8-bit: the palette (256 colours) after the pixels, then as 16-bit. (width * height is written
 * again for the palette: the CSE gives the frame and the mflo v0 / move s3,v0 copy. The palette
 * is read from pic->dst, the field just stored: IDO forwards that read into the if body as a1,
 * where a read of dst or of the global is reloaded after the store through pic.) */
u8 *picDecode8(u8 *src, u8 *dst, Pic *pic) {
    s32 n;
    s32 i;
    s32 allocated = 0;
    s32 index;

    gPicPixels8 = dst; pic->dst = gPicPixels8;
    n = pic->width * pic->height;
    if (pic->type == 1) {
        pic->palette = (u16 *) (((u32) pic->dst + pic->width * pic->height + 7) & ~7);
        picReadPalette(256, pic);
    }
    gPicWrittenBits = picScratchAlloc(n / 8 + 1, &allocated);
    picDecodeSkips(pic);
    for (i = 0; i < n; i++) {
        if (picIsWritten(gPicWrittenBits, i)) {
            index = gPicPixels8[i];
        }
        gPicPixels8[i] = index;
    }
    if (allocated) {
        heapFree(gPicWrittenBits);
    }
    return dst;
}

/* 4-bit: two pixels a byte, each line padded to an even width; the palette (16 colours) after.
 * (The locals in the game's slot order; the two pointer stores on one line, as the 16-bit
 * decoder.) */
u8 *picDecode4(u8 *src, u8 *dst, Pic *pic) {
    s32 w;
    s32 n;
    s32 i;
    s32 x;
    s32 y;
    s32 index;
    s32 pad;
    s32 allocated = 0;

    gPicPixels8 = dst; pic->dst = gPicPixels8;
    w = (pic->width + 1) & ~1;
    n = pic->height * w;
    if (pic->type == 1) {
        pic->palette = (u16 *) (((u32) pic->dst + (n / 2 + (n & 1)) + 7) & ~7);
        picReadPalette(16, pic);
    }
    gPicWrittenBits = picScratchAlloc(n / 8 + 1, &allocated);
    picDecodeSkips(pic);
    i = 0;
    for (y = 0; y < pic->height; y++) {
        for (x = 0; x < w; x++) {
            if (picIsWritten(gPicWrittenBits, i++)) {
                index = pic->get(pic, x, y);
            }
            pic->put(pic, x, y, index);
        }
    }
    if (allocated) {
        heapFree(gPicWrittenBits);
    }
    return dst;
}

/* Read n palette entries (5 bits each of G, R, B, then 1 of I) as RGBA5551. */
void picReadPalette(s32 n, Pic *pic) {
    s32 i;
    s32 g;
    s32 r;
    s32 b;

    for (i = 0; i < n; i++) {
        g = picReadBits(5);
        r = picReadBits(5);
        b = picReadBits(5);
        pic->palette[i] = ((r << 11) | (g << 6) | (b << 1)) | (u32) picReadBits(1);
        if (pic->transparent != PIC_NO_TRANSPARENT &&
            pic->transparent == (pic->palette[i] & 0xFFFE)) {
            pic->palette[i] &= 0xFFFE;
        }
    }
}

/* The header: "PIC", a comment ended by 0x1A (whose first four characters, in hex, may name the
 * transparent colour), a reserved string ended by 0, then the mode, the depth, width and height. */
void picReadHeader(Pic *pic) {
    u8 hex[4];
    s32 mode;

    picReadBits(8);
    picReadBits(8);
    picReadBits(8);
    gPicTransparentColor = PIC_NO_TRANSPARENT;
    hex[0] = picReadBits(8);
    if (hex[0] != 0x1A) {
        hex[1] = picReadBits(8);
        hex[2] = picReadBits(8);
        hex[3] = picReadBits(8);
        gPicTransparentColor = picParseHex(hex);
        while (picReadBits(8) != 0x1A) {
        }
    }
    while (picReadBits(8) != 0) {
    }
    picReadBits(8);
    mode = picReadBits(4);
    picReadBits(4);
    pic->bits = picReadBits(16);
    gPicWidth = picReadBits(16);
    gPicHeight = picReadBits(16);
    pic->width = gPicWidth;
    pic->height = gPicHeight;
    pic->dst = NULL;
    pic->palette = NULL;
    pic->transparent = gPicTransparentColor;
    if (mode == 4) {
        pic->type = 2;
        pic->size = ((gPicWidth + 1) & ~1) * gPicHeight * pic->bits / 8;
    } else if (pic->bits == 15 || pic->bits == 16) {
        pic->type = 0;
        pic->size = gPicWidth * gPicHeight * 2;
    } else {
        pic->type = 1;
        pic->size = (gPicWidth * gPicHeight * pic->bits / 8 + (gPicWidth * gPicHeight & 1) * 2 +
                     (pic->bits == 4) * 32 + (pic->bits == 8) * 512 + 7) & ~7;
    }
}

/* Decode direct colour: runs of pixels copied from the line above, each ended by a new colour
 * (which may chain down the lines below it). */
void picDecodeDirect(Pic *pic) {
    s32 x = -1;
    s32 y = 0;
    s32 color = 0;
    u32 len;
    s32 above;

    while (TRUE) {
        len = picReadLength() - 1;
        while (len != 0) {
            x++;
            if (x == gPicWidth) {
                y++;
                x = 0;
                if (y == gPicHeight) {
                    return;
                }
            }
            above = pic->get(pic, x, y);
            if (above != 0) {
                if (pic->bits == 16) {
                    color = above;
                } else {
                    color = above & 0xFFFE;
                }
            }
            pic->put(pic, x, y, color);
            len--;
        }
        x++;
        if (x == gPicWidth) {
            y++;
            x = 0;
            if (y == gPicHeight) {
                return;
            }
        }
        color = picReadColor(pic);
        pic->put(pic, x, y, color);
        if (picReadBit()) {
            picChainDirect(pic, x, y, color);
        }
    }
}

/* A chain: the colour continues down one line per code, moving -1, 0, +1 or +-2 across. */
void picChainDirect(Pic *pic, s32 x, s32 y, s32 color) {
    s32 draw = TRUE;
    u32 code;

    while (TRUE) {
        code = picReadBits(2);
        switch (code) {
            case 0:
                if (!picReadBit()) {
                    return;
                }
                if (picReadBit() == 0) {
                    x -= 2;
                } else {
                    x += 2;
                }
                break;
            case 1:
                x--;
                break;
            case 2:
                break;
            case 3:
                x++;
                break;
        }
        if (x >= gPicWidth) {
            return;
        }
        y++;
        if (y >= gPicHeight) {
            draw = FALSE;
        }
        if (draw) {
            pic->put(pic, x, y, color);
        }
    }
}

/* Decode indexed or 16-bit colour: each run length is a skip to the next new colour; the pixels
 * skipped are filled in afterwards from the bitmap of those written. */
void picDecodeSkips(Pic *pic) {
    s32 n;
    s32 len;
    s32 pos;
    s32 total;
    s32 x;
    s32 y;
    s32 color;

    pic->put(pic, 0, 0, 0);
    pos = -1;
    total = pic->width * pic->height;
    while (pos < total) {
        n = 1;
        while (picReadBit()) {
            n++;
        }
        len = picReadBits(n) + (1 << n);
        pos = pos + len - 1;
        if (pos < total) {
            x = pos % pic->width;
            y = pos / pic->width;
            color = picReadColor(pic);
            pic->put(pic, x, y, color);
            if (picReadBit()) {
                picChainSkips(pic, x, y, color);
            }
        }
    }
}

/* A chain, for picDecodeSkips. */
void picChainSkips(Pic *pic, s32 x, s32 y, s32 color) {
    u32 code;

    while (TRUE) {
        code = picReadBits(2);
        switch (code) {
            case 1:
                pic->put(pic, --x, ++y, color);
                break;
            case 2:
                pic->put(pic, x, ++y, color);
                break;
            case 3:
                pic->put(pic, ++x, ++y, color);
                break;
            case 0:
                if (!picReadBit()) {
                    return;
                }
                if (picReadBit()) {
                    x += 2;
                } else {
                    x -= 2;
                }
                pic->put(pic, x, ++y, color);
                break;
        }
    }
}

/* A length: n one-bits and a zero, then n bits; 2^n - 1 plus their value. */
s32 picReadLength(void) {
    s32 n = 1;

    while (picReadBit()) {
        n++;
    }
    return picReadBits(n) + (1 << n) - 1;
}

/* A colour: from the cache (a 1 bit, then its 7-bit index) or new. Indexed colours are read as is. */
s32 picReadColor(Pic *pic) {
    if (pic->bits >= 15) {
        if (picReadBit()) {
            return picCacheGet(pic, picReadBits(7));
        }
        return picCacheNew(pic, picReadBits(pic->bits));
    }
    return picReadBits(pic->bits);
}

/* Empty the colour cache: 128 entries in a ring, newest 0. */
void picCacheClear(void) {
    s32 i;

    for (i = 0; i < 128; i++) {
        gPicColorCache[i].color = 0;
        gPicColorCache[i].next = i + 1;
        gPicColorCache[i].prev = i - 1;
    }
    gPicColorCache[127].next = 0;
    gPicColorCache[0].prev = 127;
    gPicColorCacheNewest = 0;
}

/* A new colour (X68000 GRB) into the cache's oldest entry, which becomes the newest. */
s32 picCacheNew(Pic *pic, s32 color) {
    if (pic->bits == 15) {
        color = ((color >> 5) & 0x3FF) + ((color & 0x1F) << 10);
        gPicColorCacheNewest = gPicColorCache[gPicColorCacheNewest].next;
        gPicColorCache[gPicColorCacheNewest].color = color;
        return color << 1;
    }
    color = ((color & 0xFFC0) >> 5) | ((color & 0x3E) << 10) | (color & 1);
    gPicColorCacheNewest = gPicColorCache[gPicColorCacheNewest].next;
    gPicColorCache[gPicColorCacheNewest].color = color;
    return color;
}

/* A colour from the cache: entry i moves to the newest place. */
s32 picCacheGet(Pic *pic, s32 i) {
    s32 newest = gPicColorCacheNewest;

    if (i != newest) {
        gPicColorCache[gPicColorCache[i].next].prev = gPicColorCache[i].prev;
        gPicColorCache[gPicColorCache[i].prev].next = gPicColorCache[i].next;
        gPicColorCache[gPicColorCache[newest].next].prev = i;
        gPicColorCache[i].next = gPicColorCache[newest].next;
        gPicColorCache[newest].next = i;
        gPicColorCache[i].prev = newest;
        gPicColorCacheNewest = i;
    }
    return (pic->bits == 16) ? gPicColorCache[i].color : gPicColorCache[i].color << 1;
}

/* 15/16-bit: a pixel, as the decoder's colour. */
s32 picGetDirect(Pic *pic, s32 x, s32 y) {
    u16 p = *(gPicPixels16 + gPicWidth * y + x);

    return (((p & 0x3E) >> 1) << 11) | (((p & 0x7C0) >> 6) << 6) | (((p & 0xF800) >> 11) << 1) | (p & 1);
}

/* 15-bit: set a pixel (opaque). */
void picPut15(Pic *pic, s32 x, s32 y, s32 color) {
    s32 a;

    if (x >= 0 && x < gPicWidth && y >= 0 && y <= gPicHeight) {
        if (pic->bits == 16) {
            a = color & 1;
        } else {
            a = 1;
        }
        *(gPicPixels16 + gPicWidth * y + x) =
            (((color & 0x3E) >> 1) << 11) | (((color & 0x7C0) >> 6) << 6) | (((color & 0xF800) >> 11) << 1) | a;
    } else {
        DPRINTF("PIC: decode overrun(pset). x=%d, y=%d\n");
    }
}

/* Clear a pixel. */
void picClearPixel(s32 x, s32 y) {
    if (x >= 0 && x < gPicWidth && y >= 0 && y <= gPicHeight) {
        *(gPicPixels16 + gPicWidth * y + x) = 0;
    } else {
        DPRINTF("PIC: decode overrun(reset). x=%d, y=%d\n");
    }
}

/* 16-bit: set a pixel, and mark it written.
 * FAKE (decomp-permuter): the empty `if (y)` only makes IDO take the original's one extra
 * temporary (the 8-bit writer, picPut8, has a real empty test). */
void picPut16(Pic *pic, s32 x, s32 y, s32 color) {
    if (x >= 0 && x < gPicWidth && y >= 0 && y <= gPicHeight) {
        if (y) {
        }
        *(gPicPixels16 + gPicWidth * y + x) =
            (((color & 0x3E) >> 1) << 11) | (((color & 0x7C0) >> 6) << 6) | (((color & 0xF800) >> 11) << 1) | (color & 1);
        picMarkWritten(gPicWrittenBits, gPicWidth * y + x);
    } else {
        DPRINTF("PIC: decode overrun(pset). x=%d, y=%d\n");
    }
}

/* 8-bit: a pixel. */
s32 picGet8(Pic *pic, s32 x, s32 y) {
    return gPicPixels8[gPicWidth * y + x];
}

/* 8-bit: set a pixel, and mark it written. The bounds test does nothing. */
void picPut8(Pic *pic, s32 x, s32 y, s32 color) {
    if (x < 0 || x >= gPicWidth || y < 0 || y > gPicHeight) {
        DPRINTF("PIC: decode overrun(pset8). x=%d, y=%d\n");
    }
    *(gPicPixels8 + gPicWidth * y + x) = color;
    picMarkWritten(gPicWrittenBits, gPicWidth * y + x);
}

/* 4-bit: a pixel (the high nibble first). */
s32 picGet4(Pic *pic, s32 x, s32 y) {
    s32 w = (gPicWidth + 1) & ~1;
    s32 i = w * y + x;

    if (i & 1) {
        return gPicPixels8[i / 2] & 0xF;
    }
    return (gPicPixels8[i / 2] & 0xF0) >> 4;
}

/* 4-bit: set a pixel (ORed in), and mark it written. (The colour is cast to u16, which IDO
 * drops but which costs the temporary register the game skips in each branch.) */
void picPut4(Pic *pic, s32 x, s32 y, s32 color) {
    s32 w = (gPicWidth + 1) & ~1;
    s32 i;

    if (x >= 0 && x < gPicWidth && y >= 0 && y <= gPicHeight) {
        i = w * y + x;
        if (i & 1) {
            gPicPixels8[i / 2] |= (u16) color;
        } else {
            gPicPixels8[i / 2] |= (u16) (color << 4);
        }
        picMarkWritten(gPicWrittenBits, i);
    } else {
        DPRINTF("PIC: decode overrun(pset4). x=%d, y=%d\n");
    }
}

/* Four hex digits, or PIC_NO_TRANSPARENT. */
s32 picParseHex(u8 *hex) {
    s32 value = 0;
    s32 i;
    s32 d;
    u8 *digits = (u8 *) "0123456789abcdef";

    for (i = 0; i < 4; i++) {
        for (d = 0; d < 16; d++) {
            if (*hex == digits[d]) {
                goto found;
            }
        }
        return PIC_NO_TRANSPARENT;
    found:
        hex++;
        value += d << ((3 - i) * 4);
    }
    return value;
}

/* Clear every pixel of the transparent colour. */
void picClearTransparent(Pic *pic) {
    s32 x;
    s32 y;

    for (y = 0; y < pic->height; y++) {
        for (x = 0; x < pic->width; x++) {
            if (pic->transparent == (pic->get(pic, x, y) & 0xFFFE)) {
                picClearPixel(x, y);
            }
        }
    }
}

void picMarkWritten(u8 *bits, s32 i) {
    bits[i >> 3] |= 1 << (i & 7);
}

void picUnusedStub(void) {
}

s32 picIsWritten(u8 *bits, s32 i) {
    return (bits[i >> 3] & (1 << (i & 7))) ? TRUE : FALSE;
}

/* Zeroed scratch memory: from the heap (*allocated = 1), or else the framebuffer after the one on
 * screen, while the draw-skip flag is set.
 * (The clear stays a rolled loop only with the bound in a local on the left, "n > i"; "p[i++] = 0"
 * puts the store before the pointer step, where "p[i] = 0; i++" stores at -4 in the delay slot.
 * The ternary gives the "b" to the next instruction.) */
void *picScratchAlloc(u32 size, s32 *allocated) {
    u32 *p;
    s32 i;
    s32 fb;
    u32 n;

    size = (size + 7) & ~7;
    p = heapMalloc(size);
    if (p == NULL) {
        if (gGameState.skipDraw != 0) {
            DPRINTF("PIC: warning: use frame buffer\n");
            fb = (gGameState.shownIndex == 2) ? 0 : gGameState.shownIndex + 1;
            p = (u32 *) gFramebuffers[fb];
        } else {
            DPRINTF("PIC: fatal: no memory\n");
            return NULL;
        }
    } else {
        *allocated = 1;
    }
    n = size / 4;
    for (i = 0; n > i;) {
        p[i++] = 0;
    }
    return p;
}

/* Start reading bits at src. */
void picBitsInit(u8 *src) {
    gPicBitsUnused0 = 0;
    gPicBitsLeft = 0;
    gPicBitsUnused8 = 8;
    gPicBitsByte = 0;
    gPicBitsNext = src;
}

/* One bit, MSB first. */
s32 picReadBit(void) {
    if (--gPicBitsLeft >= 0) {
        return (gPicBitsByte >> gPicBitsLeft) & 1;
    }
    gPicBitsLeft = 7;
    gPicBitsByte = *gPicBitsNext++;
    return (gPicBitsByte >> 7) & 1;
}

/* n bits, MSB first. */
s32 picReadBits(s32 n) {
    u32 value = 0;

    while (gPicBitsLeft < n) {
        n -= gPicBitsLeft;
        value |= (gPicBitsByte & ((1 << gPicBitsLeft) - 1)) << n;
        gPicBitsByte = *gPicBitsNext++;
        gPicBitsLeft = 8;
    }
    gPicBitsLeft -= n;
    return ((gPicBitsByte >> gPicBitsLeft) & ((1 << n) - 1)) | value;
}

/* This file's .data (subsegment 0x25140), in ROM order. */
s32 gPicBitsUnused0 = 0;
s32 gPicBitsLeft = 0;
s32 gPicBitsUnused8 = 8;
u32 gPicBitsByte = 0;
