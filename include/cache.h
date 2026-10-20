#ifndef CACHE_H
#define CACHE_H

#include "common.h"

typedef struct CacheEntry {
    /* 0x00 */ u32 flags;
    /* 0x04 */ s32 key; /* the file in it (0: none) */
    /* 0x08 */ u8 *data;
    /* 0x0C */ struct CacheEntry *prev; /* more recently used */
    /* 0x10 */ struct CacheEntry *next;
} CacheEntry; /* size 0x14 */

typedef struct Cache {
    /* 0x00 */ s32 count;
    /* 0x04 */ s32 size; /* each slot's */
    /* 0x08 */ u8 *data; /* the slots */
    /* 0x0C */ CacheEntry *head; /* the most recently used */
    /* 0x10 */ CacheEntry *tail;
    /* 0x14 */ u32 mask; /* or-ed into a slot's flags when it is used */
    /* 0x18 */ CacheEntry entries[1]; /* count of them */
} Cache;

Cache *cacheCreate(s16 count, s32 size, u8 *data);
void cacheDelete(Cache *cache);
s32 cacheGetRecent(Cache *cache, u8 **data, s32 n);
s32 cacheGetSlot(Cache *cache, u8 **data, s32 key);
void cacheMakeRecent(Cache *cache, CacheEntry *e);
void cacheSetLockBit(Cache *cache, s32 bit);
void cacheClearLockBit(Cache *cache, s32 bit);
u8 *cacheLoadFile(Cache *cache, s32 file);

#endif
