#ifndef HEAP_H
#define HEAP_H

#include "common.h"

typedef struct HeapBlock {
    /* 0x0 */ u32 addr; /* the allocation's start */
    /* 0x4 */ s32 size; /* its size, header included */
    /* 0x8 */ struct HeapBlock *prev;
    /* 0xC */ struct HeapBlock *next;
} HeapBlock; /* size 0x10 */

void heapMemClear(u8 *p, s32 n);
void heapMemCopy(u8 *dst, u8 *src, s32 n);
void heapInit(HeapBlock *blk, s32 size);
void *heapMalloc(s32 size);
void *heapMallocPage(s32 size);
void *heapMallocTop(s32 size);
void *heapMallocTopPage(s32 size);
void heapRepair(HeapBlock *blk);
void *heapFree(void *p);

#endif
