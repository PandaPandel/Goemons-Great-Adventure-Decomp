#include "common.h"
#include "cache.h"
#include "file.h"
#include "fileres.h"
#include "heap.h"

/* A cache of equal slots kept in most-recently-used order, each holding a file ("cacheCreate").
 * A slot's flags lock it against reuse: the cache's mask is or-ed in when a slot is used. */


/* A cache's header and entries, rounded up to 8. */
#define CACHE_HEADER(count) (((count) * sizeof(CacheEntry) + 0x18 + 7) & ~7)

/* Creates a cache of count slots of size bytes, at data or (if NULL) after its entries. */
Cache *cacheCreate(s16 count, s32 size, u8 *data) {
    Cache *cache;
    CacheEntry *e;
    s32 i;
    s32 pad;

    if (data == NULL) {
        cache = heapMalloc(count * size + CACHE_HEADER(count));
        if (cache == NULL) {
            goto fail;
        }
        cache->data = (u8 *) cache + CACHE_HEADER(count);
    } else {
        cache = heapMalloc(count * sizeof(CacheEntry) + 0x18);
        if (cache == NULL) {
            goto fail;
        }
        cache->data = data;
    }
    e = cache->entries;
    heapMemClear(e, count * sizeof(CacheEntry));
    for (i = 0; i < count - 1; i++) {
        e[i].next = &e[i + 1];
        e[i + 1].prev = &e[i];
    }
    cache->head = e;
    cache->tail = &e[count - 1];
    cache->count = count;
    cache->size = size;
    return cache;

fail:
    PRINTF("cacheCreate: メモリーが足りません \n");
    return NULL;
}

void cacheDelete(Cache *cache) {
    heapFree(cache);
}

/* The n-th most recently used slot's file (and its data), or 0. */
s32 cacheGetRecent(Cache *cache, u8 **data, s32 n) {
    CacheEntry *e = cache->head;
    s32 i;

    for (i = 0; i < n; i++) {
        if (e == NULL) {
            *data = NULL;
            return 0;
        }
        e = e->next;
    }
    *data = e->data;
    return e->key;
}

/* The slot for key: the one holding it, the first never used, or the least recently used one
 * not locked. It becomes the most recently used and takes key. Returns the key it held (key
 * itself if it held it), and its data; with every slot locked, key and NULL. */
s32 cacheGetSlot(Cache *cache, u8 **data, s32 key) {
    CacheEntry *e;
    s32 old;

    e = cache->head;
    while (e != NULL) {
        if (e->data == NULL) {
            e->data = cache->data +
                      ((u32) e - (u32) cache->entries) / sizeof(CacheEntry) * cache->size;
            break;
        }
        if (key == e->key) {
            break;
        }
        e = e->next;
    }
    if (e == NULL) {
        e = cache->tail;
        while (e != NULL && e->flags != 0) {
            e = e->prev;
        }
        if (e == NULL) {
            *data = NULL;
            return key;
        }
    }
    cacheMakeRecent(cache, e);
    old = e->key;
    e->key = key;
    e->flags |= cache->mask;
    *data = e->data;
    return old;
}

/* Makes a slot the most recently used. */
void cacheMakeRecent(Cache *cache, CacheEntry *e) {
    CacheEntry *prev;
    CacheEntry *next;

    if (e != cache->head) {
        prev = e->prev;
        next = e->next;
        if (prev != NULL) {
            prev->next = next;
        }
        if (next != NULL) {
            next->prev = prev;
        }
        if (e == cache->tail) {
            cache->tail = prev;
        }
        cache->head->prev = e;
        e->prev = NULL;
        e->next = cache->head;
        cache->head = e;
    }
}

/* The lock bit (1-32) that using a slot sets, or none. */
void cacheSetLockBit(Cache *cache, s32 bit) {
    if (bit != 0) {
        cache->mask = 1 << (bit - 1);
        return;
    }
    cache->mask = 0;
}

/* Clears a lock bit (1-32) from every slot, or every bit (0). */
void cacheClearLockBit(Cache *cache, s32 bit) {
    u32 keep;
    CacheEntry *e;
    s32 i;

    keep = (bit != 0) ? ~(1 << (bit - 1)) : 0;
    e = cache->entries;
    for (i = 0; i < cache->count; i++) {
        e->flags &= keep;
        e++;
    }
}

/* A file's data in the cache, loaded into a slot (and the slot's old file dropped) if needed. */
u8 *cacheLoadFile(Cache *cache, s32 file) {
    s32 old;
    u8 *data;

    old = cacheGetSlot(cache, &data, file);
    if (old != file) {
        if (old != 0) {
            resDrop(old);
        }
        fileLoadNow(file, data);
    }
    return data;
}
