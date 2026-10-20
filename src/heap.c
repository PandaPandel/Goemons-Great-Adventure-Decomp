#include "common.h"
#include "heap.h"

/* The game's debug print, compiled to nothing but its arguments: the strings stay in .rodata,
 * in source order (IDO_NOTES 144, 201). The arguments are not known: no code is left. */
#define DPRINTF

/* The heap: a first-fit allocator over one region, each allocation followed by a 16-byte header
 * on a doubly linked list between two sentinel headers (the region's first and last 16 bytes).
 * Allocations are 8-aligned, or 4 KB-aligned for the TLB pool (heapMallocPage, heapMallocTopPage). */

typedef struct Heap {
    /* 0x0 */ u32 base;
    /* 0x4 */ s32 size;
    /* 0x8 */ HeapBlock *head; /* the sentinel at base */
    /* 0xC */ HeapBlock *tail; /* the sentinel at base + size - 0x10 */
} Heap;

extern Heap gHeap;
extern u32 gHeapAlignMask; /* the alignment mask: 7, or 0xFFF for the TLB pool */

/* memclear. */
void heapMemClear(u8 *p, s32 n) {
    s32 head = (u32) p & 7;

    while (head > 0 && n > 0) {
        head--;
        n--;
        *p++ = 0;
    }
    while (n >= 8) {
        n -= 8;
        *(u64 *) p = 0;
        p += 8;
    }
    while (n > 0) {
        n--;
        *p++ = 0;
    }
}

/* Copy n bytes. */
void heapMemCopy(u8 *dst, u8 *src, s32 n) {
    while (n--) {
        *dst = *src;
        src++;
        dst++;
    }
}

/* Set the heap up over [base, base + size): cleared, with a sentinel header at each end.
 * head is read back from the heap's base just stored, not copied from blk: IDO forwards the
 * stored value (the parameter's home, loaded once into t6 after the call) and leaves blk in
 * memory; a copy or blk itself gets a register (v1, or a0 with a spill around the call). The
 * (s32) casts give the game's sums (base evaluated first, size loaded first). */
void heapInit(HeapBlock *blk, s32 size) {
    HeapBlock *head;
    HeapBlock *tail;

    heapMemClear((u8 *) blk, size);
    gHeap.base = (u32) blk;
    gHeap.size = size;
    head = (HeapBlock *) gHeap.base;
    head->prev = NULL;
    head->next = NULL;
    head->size = sizeof(HeapBlock);
    head->addr = gHeap.base;
    tail = (HeapBlock *) ((s32) gHeap.base + gHeap.size) - 1;
    tail->next = NULL;
    tail->size = sizeof(HeapBlock);
    tail->prev = head;
    tail->addr = (s32) gHeap.base + gHeap.size - sizeof(HeapBlock);
    head->next = tail;
    gHeap.tail = tail;
    gHeap.head = head;
}

/* malloc: the first gap from the bottom that fits. */
void *heapMalloc(s32 size) {
    s32 need;
    HeapBlock *blk;
    HeapBlock *next;
    HeapBlock *new;
    s32 addr;

    if (size <= 0) {
        return NULL;
    }
    need = (size + 0x17) & ~7;
    blk = gHeap.head;
    next = blk->next;
    if (next != NULL) {
        do {
            addr = ((u32) blk + gHeapAlignMask + sizeof(HeapBlock)) & ~gHeapAlignMask;
            if ((s32) (next->addr - addr) >= need) {
                goto found;
            }
            blk = next;
            next = next->next;
        } while (next != NULL);
    }
    return NULL;
found:
    new = (HeapBlock *) (addr + need) - 1;
    new->prev = blk;
    new->next = blk->next;
    blk->next->prev = new;
    blk->next = new;
    new->addr = addr;
    new->size = need;
    return (void *) addr;
}

/* malloc, 4 KB-aligned (the TLB pool). */
void *heapMallocPage(s32 size) {
    void *p;

    gHeapAlignMask = 0xFFF;
    p = heapMalloc(size);
    gHeapAlignMask = 7;
    return p;
}

/* malloc from the top: the first gap from the top that fits. */
void *heapMallocTop(s32 size) {
    HeapBlock *blk;
    HeapBlock *prev;
    HeapBlock *new;
    s32 addr;
    s32 need;

    if (size <= 0) {
        return NULL;
    }
    need = (size + 0x17) & ~7;
    blk = gHeap.tail;
    prev = blk->prev;
    if (prev != NULL) {
        do {
            addr = (blk->addr - need) & ~gHeapAlignMask;
            if (addr >= (u32) prev + sizeof(HeapBlock)) {
                goto found;
            }
            blk = prev;
            prev = prev->prev;
        } while (prev != NULL);
    }
    return NULL;
found:
    new = (HeapBlock *) (addr + need) - 1;
    new->prev = prev;
    new->next = blk;
    blk->prev->next = new;
    blk->prev = new;
    new->addr = addr;
    new->size = need;
    return (void *) addr;
}

/* malloc from the top, 4 KB-aligned. */
void *heapMallocTopPage(s32 size) {
    void *p;

    gHeapAlignMask = 0xFFF;
    p = heapMallocTop(size);
    gHeapAlignMask = 7;
    return p;
}

/* Repair the list from its tail after free found it broken: relink blk where the backward walk
 * finds the break (or rebuild the tail sentinel if its own link is not a KSEG0 pointer).
 * FAKE (decomp-permuter): `heap` exists only to make IDO load the heap's size before its base, as
 * the original does; and only as `(*heap).base`, since `heap->base` compiles the other way. */
void heapRepair(HeapBlock *blk) {
    HeapBlock *b;
    HeapBlock *prev;
    Heap *heap;

    if (((u32) gHeap.tail & 0xFF000000) == 0x80000000) {
        if (((u32) gHeap.tail->prev & 0xFF000000) != 0x80000000) {
            gHeap.tail->prev = blk;
            gHeap.tail->next = NULL;
            heap = &gHeap;
            gHeap.tail->size = sizeof(HeapBlock);
            gHeap.tail->addr = gHeap.size + (*heap).base - sizeof(HeapBlock);
            return;
        }
        for (b = gHeap.tail; b != NULL; b = prev) {
            prev = b->prev;
            if (b != prev->next) {
                DPRINTF("ロストされたチェーンを破棄します\n");
                b->prev = blk;
                blk->next = b;
                return;
            }
            if (blk == prev) {
                blk->next = b;
                return;
            }
        }
    }
    DPRINTF("mallocの管理構造体が破壊されています。修復できませんでした\n");
}

/* free: unlink p's block and clear the memory. Returns NULL, or p if it was not found (or the
 * list was broken, which heapRepair repairs). */
void *heapFree(void *p) {
    HeapBlock *blk;
    HeapBlock *next;

    if (p == NULL) {
        return NULL;
    }
    for (blk = gHeap.head; blk != NULL; blk = next) {
        next = blk->next;
        if (next != NULL && blk != next->prev) {
            heapRepair(blk);
            return p;
        }
        if ((u32) p == blk->addr) {
            goto found;
        }
    }
    DPRINTF("Warning: free しようとした領域が見当たりません...\n");
    return p;
found:
    blk->prev->next = next;
    blk->next->prev = blk->prev;
    heapMemClear(p, blk->size);
    return NULL;
}

/* This file's .data (subsegment 0x25140), in ROM order. */
u32 gHeapAlignMask = 7;
