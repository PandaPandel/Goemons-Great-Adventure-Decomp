#include "common.h"
#include "dma.h"
#include "file.h"
#include "fileload.h"
#include "fileres.h"
#include "heap.h"

/* File sets: file 11 in ROM is a table of 8-byte entries, each four file ids (0: none) that an
 * area loads together. Overlay 6 reads an entry by DMA and loads, finds or frees its files
 * through main's file layer (src/file.c, src/fileres.c). */


/* File set n (past the table's end, the first). */
FileSet floadGetSet(s32 n) {
    FileSet set;
    u32 rom;

    rom = fileRomStart(11) + n * sizeof(FileSet);
    if (fileRomEnd(11) < rom) {
        rom = fileRomStart(11);
    }
    dmaReadRom(rom, &set, sizeof(FileSet));
    return set;
}

/* File set n's file i (0-3), or 0. */
s16 floadGetSetFile(s32 n, s32 i) {
    FileSet set;

    set = floadGetSet(n);
    switch (i) {
        default:
            return 0;
        case 0:
            return set.file[0];
        case 1:
            return set.file[1];
        case 2:
            return set.file[2];
        case 3:
            return set.file[3];
    }
}

/* The RAM addresses of file set n's files. */
void floadGetSetRam(void **ram, s32 n) {
    FileSet set;

    set = floadGetSet(n);
    ram[0] = (void *) fileRamAddr(set.file[0]);
    ram[1] = (void *) fileRamAddr(set.file[1]);
    ram[2] = (void *) fileRamAddr(set.file[2]);
    ram[3] = (void *) fileRamAddr(set.file[3]);
}

/* A segmented address (its low 24 bits an offset) in file set n's file i, in RAM. */
u32 floadSegToRam(u32 addr, s32 n, s32 i) {
    return fileRamAddr(floadGetSetFile(n, i)) + (addr & 0xFFFFFF);
}

/* Loads a file onto the heap's top unless it is loaded. */
void floadFileToTop(u16 file) {
    s32 size;
    void *ram;

    if (resFindFile(file) == -1) {
        size = fileRamSize(file);
        if (size > 0) {
            ram = heapMallocTop(size);
            if (ram != NULL) {
                fileLoadNowKeep(file, ram);
            }
        }
    }
}

/* Whether all of file set n's files are loaded. */
s32 floadIsSetLoaded(s32 n) {
    FileSet set;
    s32 loaded = TRUE;

    set = floadGetSet(n);
    if (set.file[2] != 0 && resFindFile(set.file[2]) == -1) {
        loaded = FALSE;
    }
    if (set.file[1] != 0 && resFindFile(set.file[1]) == -1) {
        loaded = FALSE;
    }
    if (set.file[0] != 0 && resFindFile(set.file[0]) == -1) {
        loaded = FALSE;
    }
    if (set.file[3] != 0 && resFindFile(set.file[3]) == -1) {
        loaded = FALSE;
    }
    return loaded;
}

/* Loads file set n's files now. */
FileSet floadLoadSet(s32 n) {
    FileSet set;

    set = floadGetSet(n);
    if (set.file[2] != 0) {
        fileLoadNow(set.file[2], NULL);
    }
    if (set.file[1] != 0) {
        fileLoadNow(set.file[1], NULL);
    }
    if (set.file[0] != 0) {
        fileLoadNow(set.file[0], NULL);
    }
    if (set.file[3] != 0) {
        fileLoadNow(set.file[3], NULL);
    }
    return set;
}

/* Loads file set n's missing files onto the heap's top. */
FileSet floadLoadSetToTop(s32 n) {
    FileSet set;

    set = floadGetSet(n);
    if (set.file[2] != 0) {
        floadFileToTop(set.file[2]);
    }
    if (set.file[1] != 0) {
        floadFileToTop(set.file[1]);
    }
    if (set.file[0] != 0) {
        floadFileToTop(set.file[0]);
    }
    if (set.file[3] != 0) {
        floadFileToTop(set.file[3]);
    }
    return set;
}

/* Frees file set n's files. */
FileSet floadFreeSet(s32 n) {
    FileSet set;

    set = floadGetSet(n);
    if (set.file[2] != 0) {
        resFree(set.file[2]);
    }
    if (set.file[1] != 0) {
        resFree(set.file[1]);
    }
    if (set.file[0] != 0) {
        resFree(set.file[0]);
    }
    if (set.file[3] != 0) {
        resFree(set.file[3]);
    }
    return set;
}

/* Loads file set n's files now and gives their RAM addresses. */
void floadLoadSetRam(void **ram, s32 n) {
    FileSet set;

    set = floadGetSet(n);
    if (set.file[2] != 0) {
        fileLoadNow(set.file[2], NULL);
    }
    if (set.file[1] != 0) {
        fileLoadNow(set.file[1], NULL);
    }
    if (set.file[0] != 0) {
        fileLoadNow(set.file[0], NULL);
    }
    if (set.file[3] != 0) {
        fileLoadNow(set.file[3], NULL);
    }
    ram[0] = (void *) fileRamAddr(set.file[0]);
    ram[1] = (void *) fileRamAddr(set.file[1]);
    ram[2] = (void *) fileRamAddr(set.file[2]);
    ram[3] = (void *) fileRamAddr(set.file[3]);
}
