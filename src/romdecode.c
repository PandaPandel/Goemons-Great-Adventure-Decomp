#include "common.h"
#include "dma.h"
#include "heap.h"
#include "romdecode.h"
#include "lib/zlib/zlib.h" /* zlib 1.1.3: the game links its inflate side (canon-decompilation-tools/build_zlib.py) */

/* The game's debug print, compiled to nothing but its arguments: the strings stay in .rodata,
 * in source order (IDO_NOTES 144, 201). The arguments are not known: no code is left. */
#define DPRINTF

/* The ROM reader (one 8 KB buffer), Konami's LZ decoder and the zlib path behind it. The game's
 * name for the decoder is in its dead print ("romDecode"). Its .rodata starts a new 16-byte
 * line after the Pak's (four bytes of padding at ROM 0x3DDCC), so it is a file of its own. */

extern u8 gRomBuf[0x2000]; /* the ROM reader's buffer */
extern u32 gRomReadAddr;        /* its next ROM address */
extern u32 gRomCurAddr;        /* the ROM address of the byte being read */
extern u16 gRomPlane;        /* the plane being decoded */
extern u16 gRomPlaneCount;        /* the number of planes (the output stride) */
extern u8 gFileLoaderBusy;         /* the loading flag */

extern s32 gRomBytesLeft;        /* bytes left in ROM */
extern s32 gRomInputLeft;
extern s32 gRomBufLeft;        /* bytes left in the buffer */

/* Clear the ROM reader's buffer, a u64 at a time. (IDO unrolls the loop by 4: four u64 stores per
 * step, and the end compared is the next symbol, 0x80082CA8.) */
void romClearBuf(void) {
    s32 i;

    for (i = 0; i < (s32) (sizeof(gRomBuf) / sizeof(u64)); i++) {
        ((u64 *) gRomBuf)[i] = 0;
    }
}

/* Read a big-endian word from the ROM reader. */
/* Decompress a file from ROM (Konami LZ; docs: the format in the title page). Each block starts
 * with a big-endian word: its length (header included); bit 31, while the loader runs, hands the
 * file to zlib; a top nibble N > 0 splits the block into N+1 interleaved planes. After each block
 * the input skips to a 16-byte boundary. Returns the end of the output, or NULL on bad data.
 * Matched 2026-10-02 (148 lines at the start): the frame (base at 0x40), gRomInputLeft-- before
 * src's init, base++ before the plane counter; then the simple block's back-reference count
 * decoded with the distance, before the reader call that steps past the second byte: live
 * across the call, the count comes out in the game's a1 (IDO_NOTES 228; decoded after the
 * call it took v1, 20 lines, which the old FAKE "do {} while (0)" boundary fixed), and the
 * plane count cast to u16 (the stored global's type) before the + 1: that cast spends the
 * temporary the game skips before the planar setup (IDO_NOTES 235; without the boundary it
 * changed nothing, 126). The permuter had found the temporary as a FAKE double shift
 * (IDO_NOTES 234). The word reads are written out (this file's READ_BE32 macro, gone). */
u8 *romDecode(u32 rom, u8 *dst, s32 size) {
    u8 *src;
    u32 header;
    u32 end;
    u8 c;
    u16 len;
    u16 dist; /* the back-reference distance, or the run's byte */
    u8 *base;
    u8 *from; /* declared after base: a trailing register-only local takes no frame slot */

    DPRINTF("romDecode %08x(%08x/%08x) -> %08x\n");
    gRomReadAddr = gRomCurAddr = rom;
    gRomInputLeft = size;
    gRomBytesLeft = gRomInputLeft;
    gRomInputLeft--;
    src = gRomBuf;
    romRefill();
    while (gRomInputLeft > 0) {
        header = *src << 24;
        src = romNextByte(src);
        header += *src << 16;
        src = romNextByte(src);
        header += *src << 8;
        src = romNextByte(src);
        header += *src;
        src = romNextByte(src);
        if ((header & 0x80000000) && gFileLoaderBusy != 0) {
            return romInflate(rom, dst, size);
        }
        end = gRomCurAddr + header - 4;
        if ((header & 0xF0000000) == 0) {
            while (gRomCurAddr < end) {
                c = *src;
                src = romNextByte(src);
                if (c < 0x80) {
                    dist = (*src + (c << 8)) & 0x3FF;
                    len = (c >> 2) + 2;
                    src = romNextByte(src);
                    from = dst - dist;
                    while (len--) {
                        *dst++ = *from++;
                    }
                } else if (c < 0xA0) {
                    len = c & 0x1F;
                    while (len--) {
                        *dst++ = *src;
                        src = romNextByte(src);
                    }
                } else if (c < 0xC0) {
                    DPRINTF("lzss decode word\n\ndo not use!!\n\n%X %X %X %02X\n");
                    return NULL;
                } else {
                    if (c < 0xE0) {
                        len = (c & 0x1F) + 2;
                        dist = *src;
                        src = romNextByte(src);
                    } else {
                        dist = 0;
                        if (c < 0xFF) {
                            len = (c & 0x1F) + 2;
                        } else {
                            len = *src + 2;
                            src = romNextByte(src);
                        }
                    }
                    while (len--) {
                        *dst++ = dist;
                    }
                }
            }
        } else {
            gRomPlane = 0;
            gRomPlaneCount = (u16) ((header & 0xF0000000) >> 28) + 1;
            base = dst;
            while (gRomPlane < gRomPlaneCount) {
                if (gRomPlane != 0) {
                    header = *src << 24;
                    src = romNextByte(src);
                    header += *src << 16;
                    src = romNextByte(src);
                    header += *src << 8;
                    src = romNextByte(src);
                    header += *src;
                    src = romNextByte(src);
                    end = gRomCurAddr + header - 4;
                    dst = base;
                }
                header &= 0x0FFFFFFF;
                while (gRomCurAddr < end) {
                    c = *src;
                    src = romNextByte(src);
                    if (c < 0x80) {
                        dist = (*src + (c << 8)) & 0x3FF;
                        src = romNextByte(src);
                        len = (c >> 2) + 2;
                        from = dst - dist * gRomPlaneCount;
                        while (len--) {
                            *dst = *from;
                            dst += gRomPlaneCount;
                            from += gRomPlaneCount;
                        }
                    } else if (c < 0xA0) {
                        len = c & 0x1F;
                        while (len--) {
                            *dst = *src;
                            dst += gRomPlaneCount;
                            src = romNextByte(src);
                        }
                    } else if (c < 0xC0) {
                        DPRINTF("lzss decode word\n\ndo not use!!\n\n%X %X %X %02X\n");
                        return NULL;
                    } else {
                        if (c < 0xE0) {
                            len = (c & 0x1F) + 2;
                            dist = *src;
                            src = romNextByte(src);
                        } else {
                            dist = 0;
                            if (c < 0xFF) {
                                len = (c & 0x1F) + 2;
                            } else {
                                len = *src + 2;
                                src = romNextByte(src);
                            }
                        }
                        while (len--) {
                            *dst = dist;
                            dst += gRomPlaneCount;
                        }
                    }
                }
                base++;
                gRomPlane++;
            }
        }
        while (end++ & 0xF) {
            src = romNextByte(src);
        }
        if (gRomInputLeft != -1 && gFileLoaderBusy == 0) {
            return NULL;
        }
    }
    return dst;
}

/* The ROM reader's next byte: p + 1, or the start of the refilled buffer. */
u8 *romNextByte(u8 *p) {
    gRomCurAddr++;
    gRomBufLeft--;
    gRomInputLeft--;
    if (gRomBufLeft != 0) {
        p++;
    } else {
        p = gRomBuf;
        romRefill();
    }
    return p;
}

/* Refill the 8 KB buffer from ROM (a short last block is rounded up to an even size). */
void romRefill(void) {
    gRomBufLeft = sizeof(gRomBuf);
    gRomBytesLeft -= gRomBufLeft;
    if (gRomBytesLeft < 0) {
        gRomBufLeft += (gRomBytesLeft + 1) & ~1;
        gRomBytesLeft = 0;
    }
    if (gRomBufLeft > 0) {
        dmaReadRom(gRomReadAddr, gRomBuf, gRomBufLeft);
    }
    gRomReadAddr += gRomBufLeft;
}

/* The zlib path of the loader: inflate a file (its size word's bit 31 set) from ROM through the
 * same 8 KB buffer. Returns the end of the output, or NULL. (The chunk size is tested as
 * `remaining < 0x2000` with `n` assigned inside: that is the original's branch order, `>= 0x2000`
 * to the DMA, then `n != 0` to the DMA, then the break. `size` and `rom` are set before the
 * z_stream, which orders the prologue.) */
u8 *romInflate(u32 rom, u8 *dst, s32 size) {
    u32 n;
    s32 ret;
    s32 pad;
    z_stream strm;
    s32 pad2;

    size = (size & 0x7FFFFFFF) - 4;
    rom += 4;
    strm.zalloc = NULL;
    strm.zfree = NULL;
    strm.opaque = NULL;
    strm.next_in = gRomBuf;
    strm.avail_out = -1;
    strm.next_out = dst;
    strm.avail_in = 0;
    if (inflateInit(&strm) != Z_OK) {
        DPRINTF("inflateInit failed (%d)\n");
        return NULL;
    }
    while (strm.total_in < size) {
        if (strm.avail_in != 0) {
            DPRINTF("d_stream.avail_in != 0\n");
            n = strm.avail_in;
        } else {
            if (size - strm.total_in < sizeof(gRomBuf)) {
                n = size - strm.total_in;
                if (n == 0) {
                    DPRINTF("left == 0 !!\n");
                    break;
                }
            } else {
                n = sizeof(gRomBuf);
            }
            dmaReadRom(rom, gRomBuf, n);
            strm.next_in = gRomBuf;
        }
        rom += n;
        strm.avail_in = n;
        ret = inflate(&strm, Z_NO_FLUSH);
        if (ret == Z_STREAM_END) {
            DPRINTF("Z_STREAD_END %d");
            break;
        }
        if (ret != Z_OK) {
            DPRINTF("inflate failed (%d)\n");
            return NULL;
        }
    }
    if (inflateEnd(&strm) != Z_OK) {
        DPRINTF("inflateEnd failed (%d)\n");
        return NULL;
    }
    return strm.next_out;
}

/* free and calloc: zlib's zutil.c calls them by these names (tools/libz.py). */
void free(void *p) {
    heapFree(p);
}

/* count * size zeroed bytes. Out of memory is a deliberate crash (a store to address 1). */
void *calloc(s32 count, s32 size) {
    s32 total = count * size;
    void *p = heapMalloc(total);

    if (p != NULL) {
        bzero(p, total);
    } else {
        DPRINTF("calloc: no memory\n");
        *(s32 *) 1 = 0;
    }
    return p;
}
