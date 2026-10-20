#include "common.h"
#include "file.h"
#include "fileres.h"
#include "heap.h"

/* The game's debug print, compiled to nothing but its arguments: the strings stay in .rodata,
 * in source order (IDO_NOTES 144, 201). The arguments are not known: no code is left. */
#define DPRINTF

/* The resident-file table: which files are in RAM, and where. A stack of up to 256 entries,
 * packed from index 0 and ended by an empty (file 0) entry. An address with bit 30 set is in the
 * TLB pool. Entries with a nonzero mode survive the mode-0 purge (resFreeUnkept), but only while
 * modes are on (gResLockOn). */

typedef struct ResEntry {
    /* 0x0 */ u16 file;
    /* 0x4 */ u32 addr;
    /* 0x8 */ u8 mode;
} ResEntry; /* size 0xC */

#define RES_TLB 0x40000000

extern ResEntry gSegInfoTable[256];
extern s32 gResLockOn; /* modes are on */
extern s32 gResRegistering; /* resRegister is registering */


/* Empty the table. */
void resClearTable(void) {
    s32 i;

    for (i = 0; i < 256; i++) {
        resSetFile(i, 0);
        resSetAddr(i, 0);
        resSetMode(i, 0);
    }
    DPRINTF("セグメント展開情報テーブルを全クリアしました\n");
}

/* Clear every entry's mode. */
void resClearModes(void) {
    s32 i;

    for (i = 0; i < 256; i++) {
        resSetMode(i, 0);
    }
    DPRINTF("lock 情報テーブルを全クリアしました\n");
}

void resModesOn(void) {
    gResLockOn = 1;
    DPRINTF("lock 機能 On\n");
}

void resModesOff(void) {
    gResLockOn = 0;
    DPRINTF("lock 機能 Off\n");
}

/* Allocate from the top of the heap and record the block at the top of the table, as file 0 (the
 * entry that ends the table). */
void *resAllocTop(s32 size) {
    void *p = heapMallocTop(size);
    s32 n = resCount();

    resSetFile(n, 0);
    resSetAddr(n, (u32) p);
    DPRINTF("セグメント展開用の固定領域を %x 以降に %x Hbyte 確保しました\n");
    return p;
}

/* Push a file. The top entry (the free block resAllocTop recorded, if any) moves up one. With the
 * table full, the file takes the last entry and returns 1. */
s32 resPush(u16 file, u32 addr, u8 mode) {
    s32 n = resCount();

    if (n >= 256) {
        DPRINTF("seginfo_Tail エラー\n");
        return 1;
    }
    if (n == 255) {
        resSetFile(n, file);
        if (resGetAddr(n) == 0) {
            resSetAddr(n, addr);
        }
        return 1;
    }
    resSetFile(n + 1, resGetFile(n));
    resSetAddr(n + 1, resGetAddr(n));
    resSetMode(n + 1, resGetMode(n));
    resSetFile(n, file);
    resSetAddr(n, addr);
    resSetMode(n, mode);
    if (gResRegistering == 0) {
        resEndRegister();
    }
    return 0;
}

/* Push a file loaded at addr, size bytes long; the entry above it records the end. */
s32 resRegister(u16 file, u32 addr, u32 size, u8 mode) {
    s32 n;

    gResRegistering = 1;
    if (resPush(file, addr, mode)) {
        return 1;
    }
    n = resCount();
    resSetAddr(n, addr + size);
    resSetMode(n, mode);
    resEndRegister();
    return 0;
}

/* The number of entries: the index of the first empty one. */
s32 resCount(void) {
    s32 i = 0;
    u16 file = resGetFile(0);

    if (file) {
        do {
            if (i == 255) {
                break;
            }
            i++;
            if (file == 0xFFFF) {
                return 255;
            }
            file = resGetFile(i);
            if (file == 0) {
                break;
            }
        } while (i != 256);
    }
    return i;
}

u16 resGetFile(s32 i) {
    if (i >= 256) {
        return 0xFFFF;
    }
    return gSegInfoTable[i].file;
}

u32 resGetAddr(s32 i) {
    if (i >= 256) {
        return -1;
    }
    return gSegInfoTable[i].addr;
}

s32 resGetMode(s32 i) {
    if (i >= 256) {
        return -1;
    }
    if (gResLockOn == 0) {
        return 0;
    }
    return gSegInfoTable[i].mode;
}

/* The index of a file, or -1. */
s32 resFindFile(u16 file) {
    s32 i;

    for (i = 0; i < 256; i++) {
        if (file == resGetFile(i)) {
            return i;
        }
    }
    return -1;
}

/* Free every file whose mode is 0. */
s32 resFreeUnkept(void) {
    s32 i = 0;
    s32 file;
    s32 ret = 0;

    while (TRUE) {
        if (resCount() < i) {
            break;
        }
        file = resGetFile(i);
        if (file == 0) {
            break;
        }
        if (resGetMode(i) == 0) {
            ret = resFree(file);
            i--;
        }
        i++;
    }
    return ret;
}

/* Free every file whose mode is not 0. */
s32 resFreeKept(void) {
    s32 i = 0;
    s32 file;
    s32 ret = 0;

    while (TRUE) {
        if (resCount() < i) {
            break;
        }
        file = resGetFile(i);
        if (file == 0) {
            break;
        }
        if (resGetMode(i) != 0) {
            ret = resFreeForce(file);
            i--;
        }
        i++;
    }
    return ret;
}

/* Free a file unless its mode keeps it. */
s32 resFree(u16 file) {
    return resRelease(file, FALSE);
}

/* Free a file. */
s32 resFreeForce(u16 file) {
    return resRelease(file, TRUE);
}

/* Free a file's memory and drop its entry. Returns 0, -1 if its mode kept it, or the heap's
 * failure. */
s32 resRelease(u16 file, s32 force) {
    s32 i;
    s32 ret;

    resEndRegister();
    i = resFindFile(file);
    if (i == -1) {
        DPRINTF("** セグメント[");
        DPRINTF("%04d");
        DPRINTF("%04x");
        DPRINTF("] は展開領域に存在しません。**\n");
        return 0;
    }
    if (!force && resGetMode(i)) {
        DPRINTF("** セグメント[");
        DPRINTF("%04d");
        DPRINTF("%04x");
        DPRINTF("] はロックされています。**\n");
        ret = -1;
    } else {
        ret = (s32) heapFree((void *) (resGetAddr(i) & ~RES_TLB));
        if (ret == 0) {
            DPRINTF("セグメント[");
            DPRINTF("%04d");
            DPRINTF("%04x");
            DPRINTF("] の展開領域を解放しました。\n");
            resRemove(i);
        } else {
            DPRINTF("** セグメント[");
            DPRINTF("%04d");
            DPRINTF("%04x");
            DPRINTF("] の展開領域を解放できませんでした。**\n");
        }
    }
    return ret;
}

/* Free every file in a list. */
s32 resFreeList(FileList *list) {
    s32 ret;

    while (TRUE) {
        ret = resFree(list->file & 0x0FFFFFFF);
        if (list->file & FILELIST_LAST) {
            break;
        }
        list++;
    }
    return ret;
}

/* Free every file but one. */
s32 resFreeAllBut(u16 keep) {
    s32 i = 0;
    s32 ret = 0;
    s32 file;

    while (TRUE) {
        if (resCount() < i) {
            break;
        }
        file = resGetFile(i);
        if (file == 0) {
            break;
        }
        if (file == keep || (ret = resFree(file)) != 0) {
            i++;
        }
    }
    return ret;
}

/* Free every file not in a list. */
s32 resFreeNotInList(FileList *list) {
    s32 i = 0;
    s32 ret = 0;
    s32 file;

    while (TRUE) {
        if (resCount() < i) {
            break;
        }
        file = resGetFile(i);
        if (file == 0) {
            break;
        }
        if (resInList(file, list) == -1) {
            ret = resFree(file);
            if (ret != 0) {
                i++;
            }
        } else {
            i++;
        }
    }
    return ret;
}

/* Free the block recorded at the top of the table (resAllocTop). Returns its address. */
u32 resFreeTop(void) {
    s32 n;
    u32 ret = 0;

    n = resCount();
    DPRINTF("固定展開領域は確保されていません。\n");
    if (resGetAddr(n) != 0) {
        ret = resGetAddr(n);
        if (heapFree((void *) resGetAddr(n)) == NULL) {
            DPRINTF("固定展開領域 %x を解放しました。\n");
            resRemove(n);
        }
    }
    return ret;
}

/* Free the file at addr. */
u32 resFreeAddr(u32 addr) {
    s32 i = resFindAddr(addr);

    if (i == -1) {
        DPRINTF("展開領域:[%x] はテーブルに存在しません。\n");
        return 0;
    }
    resFree(resGetFile(i));
    DPRINTF("展開領域:[%x] をテーブルから消去しました。\n");
    return addr;
}

/* Drop a file's entry, leaving its memory.
 * The prints test the file number after the call (decimal below 0x8000, as fileLoadOverlay prints
 * it), so file lives in its home across resFindFile: `sw a0` there and `lhu` for the argument.
 * The empty tests are dropped later and leave no code (IDO_NOTES 380). */
s32 resDrop(u16 file) {
    s32 i = resFindFile(file);

    if (i == -1) {
        DPRINTF("セグメント[");
        if (file < 0x8000) {
            DPRINTF("%04d", file);
        } else {
            DPRINTF("%04x", file);
        }
        DPRINTF("] は展開領域に存在しません。\n");
        return -1;
    }
    resRemove(i);
    DPRINTF("セグメント[");
    if (file < 0x8000) {
        DPRINTF("%04d", file);
    } else {
        DPRINTF("%04x", file);
    }
    DPRINTF("] をテーブルから削除しました。\n");
    return 0;
}

s32 resSetFile(s32 i, u16 file) {
    if (i >= 256) {
        return -1;
    }
    gSegInfoTable[i].file = file;
    return 0;
}

s32 resSetAddr(s32 i, u32 addr) {
    if (i >= 256) {
        return -1;
    }
    gSegInfoTable[i].addr = addr;
    return 0;
}

s32 resSetMode(s32 i, u8 mode) {
    if (i >= 256) {
        return -1;
    }
    gSegInfoTable[i].mode = mode;
    return 0;
}

/* Remove entry i, moving the ones above it down. */
s32 resRemove(s32 i) {
    s32 n;
    s32 j;

    if (i >= 256 || (n = resCount()) < i) {
        return -1;
    }
    for (j = i; j <= n; j++) {
        resSetFile(j, resGetFile(j + 1));
        resSetAddr(j, resGetAddr(j + 1));
        resSetMode(j, resGetMode(j + 1));
    }
    return 0;
}

/* 0 if the file is in the list, else -1. */
s32 resInList(u16 file, FileList *list) {
    while (TRUE) {
        if ((u16) (list->file & 0x0FFFFFFF) == file) {
            return 0;
        }
        if (list->file & FILELIST_LAST) {
            break;
        }
        list++;
    }
    return -1;
}

/* The index of the entry at addr, or -1. */
s32 resFindAddr(u32 addr) {
    s32 i;

    for (i = 0; i < 256; i++) {
        if (resGetAddr(i) == addr) {
            return i;
        }
    }
    return -1;
}

/* End a registration (resRegister): print the last entry, and the fixed buffer above it.
 * The prints' tests are dropped after register allocation. What stays is `li a2,12; multu a0,a2`
 * with no `mflo`: the entry's index times 12, with 12 in a register because two indexes use it
 * (i, then i after the step; IDO_NOTES 110). A multu writes hi/lo, so it is not deleted. */
void resEndRegister(void) {
    s32 last = resCount() - 1;
    s32 i;

    if (last < 0) {
        last = 0;
    }
    i = last;
    DPRINTF("(%03d)展開セグメント[", i);
    if (gSegInfoTable[i].file < 0x8000) {
        DPRINTF("%04d", gSegInfoTable[i].file);
    } else {
        DPRINTF("%04x", gSegInfoTable[i].file);
    }
    DPRINTF("]展開アドレス[%08x]", gSegInfoTable[i].addr);
    if (gSegInfoTable[i].mode != 0) {
        DPRINTF("** locked **");
    }
    i++;
    if (gSegInfoTable[i].file == 0) {
        DPRINTF("固定バッファ[%08x]", gSegInfoTable[i].addr);
    }
    DPRINTF("\n");
    gResRegistering = 0;
}

/* How many entries are at addr, each one printed.
 * The print's test of the file number (decimal below 0x8000, as fileLoadOverlay prints it) is
 * dropped after register allocation, but its loop-invariant 0x8000 has taken a2 by then: the
 * loop's end pointer is in a3 and a2 is never written. The strings from " <segment> [" on are
 * the last in the file's .rodata, so here they keep their order. */
s32 resCountAddr(u32 addr) {
    s32 n = 0;
    s32 i;

    for (i = 0; i < 256; i++) {
        if (addr == (gSegInfoTable[i].addr & ~RES_TLB)) {
            n++;
            DPRINTF(" <segment> [");
            if (gSegInfoTable[i].file < 0x8000) {
                DPRINTF("%04d", gSegInfoTable[i].file);
            } else {
                DPRINTF("%04x", gSegInfoTable[i].file);
            }
            DPRINTF("]\n");
            DPRINTF("name");
        }
    }
    return n;
}

/* This file's .data (subsegment 0x25140), in ROM order. */
s32 gResLockOn = 0;
s32 gResRegistering = 0;
