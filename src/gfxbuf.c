#include "object.h"
#include "game.h"
#include "scheduler.h"
#include "gfxbuf.h"

/* Display lists are built in two buffers of 0x20000 bytes, one per frame (gGfxDlBuffers); the
 * pointer gGfxDlHead is where the next command goes. Matrices, vertices and lights for a frame
 * are taken from the same buffer. */

extern Gfx *gGfxDlHead;
extern u8 gGfxDlBuffers[2][0x20000];
/* gfxbuf.c's .data (0x80107950-0x80107958), in ROM order. */
Gfx gGfxEndDl = { /* gSPEndDisplayList: where a copied list ends */
    { 0xDF000000, 0x00000000 },
};
extern u16 gFramebuffers[3][SCREEN_WIDTH * SCREEN_HEIGHT]; /* the framebuffers */

/* Starts building in buffer i. */
void gfxStartBuffer(s32 i) {
    gGfxDlHead = (Gfx *) gGfxDlBuffers[i];
}

/* Bytes left in this frame's buffer, less the list's end. The difference in a local puts it in
 * v1, as the game has it. */
s32 gfxGetFreeBytes(void) {
    s32 n = gGfxDlBuffers[gGameState.fbIndex] - (u8 *) gGfxDlHead;

    return n + 0x1FFF8;
}

/* Writes buffer i back from the data cache, up to the list's end. */
void gfxWritebackBuffer(s32 i) {
    /* A debug meter: how full the two buffers are, in percent, checked against 100 by a print
     * that compiled to nothing (no string in .rodata). The dead percent takes v0, which puts
     * the end in v1 and the base in a3, as the game has; its int to float to int conversions
     * (removed after ugen) spend the two temporaries the game spends before the index (t8/t9).
     * The constants (lui, no literal) leave .rodata as it was. Without the if it loses v0. */
    s32 percent = ((u8 *) gGfxDlHead - (u8 *) gGfxDlBuffers) * 100.0f / sizeof(gGfxDlBuffers);

    if (percent > 100) {
    }
    osWritebackDCache(gGfxDlBuffers[i & 1], (u8 *) gGfxDlHead - gGfxDlBuffers[i & 1] - 8);
}

/* n matrices from the buffer: the buffer's pointer, summed as a u32. (Which functions held the
 * two "over" prints is a guess; their strings come first in the file.) */
Mtx *gfxAllocMtx(s32 n) {
    Mtx *m = (Mtx *) gGfxDlHead;

    PRINTF("Error: local display list buffer over !! (%d byte)\n", n);
    gGfxDlHead = (Gfx *) ((u32) gGfxDlHead + n * sizeof(Mtx));
    return m;
}

/* n vertices. */
Vtx *gfxAllocVtx(s32 n) {
    Vtx *v = (Vtx *) gGfxDlHead;

    PRINTF("Error: local display list buffer over !! (%d byte)\n", n);
    gGfxDlHead = (Gfx *) ((u32) gGfxDlHead + n * sizeof(Vtx));
    return v;
}

/* n look-at pairs. */
LookAt *gfxAllocLookAt(s32 n) {
    LookAt *l = (LookAt *) gGfxDlHead;

    gGfxDlHead = (Gfx *) ((u32) gGfxDlHead + n * sizeof(LookAt));
    return l;
}

/* n viewports. */
Vp *gfxAllocVp(s32 n) {
    Vp *vp = (Vp *) gGfxDlHead;

    gGfxDlHead = (Gfx *) ((u32) gGfxDlHead + n * sizeof(Vp));
    return vp;
}

/* n sets of seven lights. */
Lights7 *gfxAllocLights7(s32 n) {
    Lights7 *l = (Lights7 *) gGfxDlHead;

    gGfxDlHead = (Gfx *) ((u32) gGfxDlHead + n * sizeof(Lights7));
    return l;
}

/* Copies a display list up to its end; returns where the copy ends. The end is one 64-bit
 * compare (all four words loaded before the first branch), written src first with the end's
 * address inline (a named "end" local puts its lui in v1); src steps before dst. */
Gfx *gfxCopyDisplayList(Gfx *dst, Gfx *src) {
    while (*(u64 *) src != *(u64 *) &gGfxEndDl) {
        *dst = *src;
        src++;
        dst++;
    }
    return dst;
}

u32 gfxGetPendingSwaps(void) {
    return gMainSched.pendingSwaps;
}

/* The framebuffer being drawn (beffect.c uses the index). */
s32 gfxGetFbIndex(void) {
    return mainSchedGetBufferIndex(&gMainSched);
}

/* The framebuffer before the one being drawn. */
u16 *gfxGetPrevFramebuffer(void) {
    s32 i = mainSchedGetBufferIndex(&gMainSched) - 1;

    if (i < 0) {
        i = 2;
    }
    return gFramebuffers[i];
}

/* A segmented address in RAM, from the addresses of segments 3-13 (segs[0] is segment 3); an
 * address with its top bit set is already one. 0 for a bad or empty segment. The two ranges
 * (3-8, 9-13) are tested one after the other as an "&&" early return, with no switch table; the
 * sum is pointer arithmetic, which puts base first in the add. */
u32 gfxSegmentToRam(u32 addr, u32 *segs) {
    u32 id;
    u32 base;

    if (addr == 0) {
        return 0;
    }
    if (addr & 0x80000000) {
        return addr;
    }
    id = ((addr >> 24) & 0xF) - 3;
    if (id >= 6 && id >= 11) {
        PRINTF("アドレスのセグメントIDが異常です \n");
        return 0;
    }
    base = segs[id];
    if (base == 0) {
        PRINTF("Segment ID %d が登録されていません \n", (addr >> 24) & 0xF);
        return 0;
    }
    return (u32) ((u8 *) base + (addr & 0xFFFFFF));
}
