#include "contpak.h"
#include "heap.h"

/* The game's debug print, compiled to nothing but its arguments: the strings stay in .rodata,
 * in source order (IDO_NOTES 144, 201). The arguments are not known: no code is left. */
#define DPRINTF

/* The Controller Pak and Rumble Pak layer: one ContPak block per port (include/contpak.h). The
 * save file is company "A4", game "NGME", 0x700 bytes. */

extern OSMesgQueue gSiMesgQueue;          /* the SI (controller) message queue */
extern OSContStatus gContQueryStatus[MAXCONTROLLERS];
extern s32 gContPakQueryFrame;             /* the frame the status was last queried on */
extern s32 gGameFrameCount;             /* the frame counter */
extern s8 gContPakRumbleSuspended;              /* set while the game resets: rumble is refused */
/* libultra PFS, by call shape (n64sym agrees; not yet verified by bytes): */
extern u8 gContPakGameName[];  /* the save file's name */
extern u8 gContPakExtName[];  /* the save file's extension */
extern struct { /* gGameWork (object.h's G88338) as far as this file reads it */
    /* 0x00 */ u8 pad0[0x15];
    /* 0x15 */ s8 demoPlaying; /* set while a demo plays: no rumble */
    /* 0x16 */ u8 fadeColor[0x3];
    /* 0x19 */ s8 rumbleSuspended; /* gContPakRumbleSuspended: set while the game resets, no rumble */
} gGameWork;
/* libultra's motor access (on = 1, off = 0). ultralib declares it only from 2.0J, but this game's
 * libultra has it (libsig-verified against Smash's). */
s32 contPakMotorSet(s32 port, s32 on, s32 force);

/* Probe all four ports. */
void contPakProbeAll(void) {
    s32 i;

    DPRINTF("--- pakInitAll ---\n");
    for (i = 0; i < MAXCONTROLLERS; i++) {
        contPakProbe(i);
    }
}

/* Probe one port: a Controller Pak, else (error 10) a Rumble Pak. */
s32 contPakProbe(s32 port) {
    s32 ret = contPakInitPfs(port);

    DPRINTF("--- pakInit ---\n");
    DPRINTF("Controller %d: ");
    if (ret == PFS_ERR_ID_FATAL) {
        ret = contPakInitRumble(port);
        if (ret != 0x200) {
            ret = PFS_ERR_ID_FATAL;
        }
    }
    gContPak[port].error = ret; return ret;
}

/* Open the Controller Pak on a standard controller; 0x101 when the controller reports an error. */
s32 contPakInitPfs(s32 port) {
    s32 ret;
    ContPak *pak = &gContPak[port];

    pak->pakType = 0;
    ret = 0;
    if ((contPakGetStatus(port)->type & CONT_TYPE_NORMAL) == CONT_TYPE_NORMAL) {
        DPRINTF("__vpakInit - ");
        ret = osPfsInitPak(&gSiMesgQueue, &pak->pfs, port);
        if (ret == 0) {
            DPRINTF("succeeded\n");
            pak->pakType = 1;
        } else {
            DPRINTF("failed\n");
        }
    }
    if (contPakGetStatus(port)->errno != 0) {
        ret = 0x101;
    }
    pak->error = ret;
    return ret;
}

/* Is a Rumble Pak in this port? 0x200 when one answers. The store and the return share a line:
 * IDO schedules by source line, and only this way does the store land after the stack pop, as in
 * the original (IDO_NOTES rule 12). */
s32 contPakInitRumble(s32 port) {
    s32 ret = osMotorInit(&gSiMesgQueue, &gContPak[port].pfs, port);

    if (ret == 0) {
        ret = 0x200;
        gContPak[port].pakType = CONTPAK_TYPE_RUMBLE;
    }
    gContPak[port].error = ret; return ret;
}

/* Create the save file (0x700 bytes) and fill it with zeroes. (No `pak` local: the pak address
 * is a spilled temporary under the buffer. The clear is a do-while: a for loop puts the first
 * unrolled store, not the last, in the branch delay slot.) */
s32 contPakCreateFile(s32 port) {
    s32 ret;
    s32 i;
    u8 buf[0x700];

    DPRINTF("--- pakCreateFile ---\n");
    ret = osPfsAllocateFile(&gContPak[port].pfs, 0x4134, 0x4E474D45, gContPakGameName,
                             gContPakExtName, sizeof(buf), &gContPak[port].fileNo);
    if (ret == 0) {
        DPRINTF("allocated to %d\n");
        i = 0;
        do {
            buf[i] = 0;
            i++;
        } while (i < (s32) sizeof(buf));
        contPakWrite(buf, 0, sizeof(buf), port);
    }
    gContPak[port].error = ret;
    return ret;
}

/* Find the save file and read its directory entry. */
s32 contPakFindFile(s32 port) {
    s32 ret;

    DPRINTF("  (--- pakFindFile ---)\n");
    osPfsFindFile(&gContPak[port].pfs, 0x4134, 0x4E474D45, gContPakGameName, gContPakExtName,
                       &gContPak[port].fileNo);
    DPRINTF("File found. descriptor = 0x%08x\n");
    DPRINTF("file_no = %d\n");
    ret = osPfsFileState(&gContPak[port].pfs, gContPak[port].fileNo,
                             &gContPak[port].files[gContPak[port].fileNo]);
    DPRINTF(" file size: %d\n");
    DPRINTF(" game code: 0x%08x\n");
    DPRINTF(" company code: 0x%04x\n");
    if (ret != 0) {
        gContPak[port].pakType = 1;
    }
    gContPak[port].error = ret; return ret;
}

/* Read save slot `slot` (0x200 bytes from 0x100) and check its CRC; 0x102 when it fails. The
 * slot's first u64 holds the CRC (low word), and is zeroed for the computation. */
s32 contPakReadSlot(u8 *buf, s32 slot, s32 port) {
    s32 ret;
    u32 crc;

    DPRINTF("--- pakReadFile ---\n");
    if ((ret = contPakFindFile(port)) != 0) {
        return ret;
    }
    DPRINTF("read file %d\n");
    ret = contPakRead(buf, (slot << 9) + 0x100, 0x200, port);
    if (ret == 0) {
        crc = *(u64 *) buf;
        *(u64 *) buf = 0;
        if (contPakCrc32(buf, 0x130) != crc) {
            ret = 0x102;
        }
    }
    gContPak[port].error = ret;
    return ret;
}

/* Read from the save file. */
s32 contPakRead(u8 *buf, s32 offset, s32 size, s32 port) {
    ContPak *pak = &gContPak[port];

    return osPfsReadWriteFile(&pak->pfs, pak->fileNo, PFS_READ, offset, size, buf);
}

/* Read the save file's header (0x100 bytes from 0) and check its CRC (over 0x10 bytes). */
s32 contPakReadHeader(u8 *buf, s32 port) {
    s32 ret;
    u32 crc;

    DPRINTF("--- pakReadShared ---\n");
    if ((ret = contPakFindFile(port)) != 0) {
        return ret;
    }
    ret = contPakRead(buf, 0, 0x100, port);
    if (ret == 0) {
        crc = *(u64 *) buf;
        *(u64 *) buf = 0;
        if (contPakCrc32(buf, 0x10) != crc) {
            ret = 0x102;
        }
    }
    gContPak[port].error = ret;
    return ret;
}

/* Write save slot `slot` with its CRC, then read it back and compare; 0x102 on a mismatch. */
s32 contPakWriteSlot(u8 *buf, s32 slot, s32 port) {
    s32 ret;
    s32 pad[2]; /* unused; the original reserves the 8 bytes (IDO_NOTES rules 3b, 17) */
    s32 offset;
    u8 *check;

    DPRINTF("--- pakWriteFile ---\n");
    if ((ret = contPakFindFile(port)) != 0) {
        return ret;
    }
    *(u64 *) buf = 0;
    *(u64 *) buf = contPakCrc32(buf, 0x130);
    offset = (slot << 9) + 0x100;
    ret = contPakWrite(buf, offset, 0x200, port);
    if (ret == 0) {
        check = heapMalloc(0x200);
        if (check == NULL) {
            ret = 0x102;
        } else {
            ret = contPakRead(check, offset, 0x200, port);
            if (ret == 0 && contPakCompare(buf, check, 0x130) != 0) {
                ret = 0x102;
            }
        }
        heapFree(check);
    }
    gContPak[port].error = ret;
    return ret;
}

/* Write the save file's header with its CRC, then read it back and compare. */
s32 contPakWriteHeader(u8 *buf, s32 port) {
    s32 ret;
    s32 pad[3]; /* unused; the same frame as contPakWriteSlot, which has an offset here */
    u8 *check;

    DPRINTF("--- pakWriteShared ---\n");
    if ((ret = contPakFindFile(port)) != 0) {
        return ret;
    }
    *(u64 *) buf = 0;
    *(u64 *) buf = contPakCrc32(buf, 0x10);
    ret = contPakWrite(buf, 0, 0x100, port);
    if (ret == 0) {
        check = heapMalloc(0x100);
        if (check == NULL) {
            ret = 0x102;
        } else {
            ret = contPakRead(check, 0, 0x100, port);
            if (ret == 0 && contPakCompare(buf, check, 0x10) != 0) {
                ret = 0x102;
            }
        }
        heapFree(check);
    }
    gContPak[port].error = ret;
    return ret;
}

/* Write to the save file. */
s32 contPakWrite(u8 *buf, s32 offset, s32 size, s32 port) {
    ContPak *pak = &gContPak[port];

    return osPfsReadWriteFile(&pak->pfs, pak->fileNo, PFS_WRITE, offset, size, buf);
}

/* Compare n bytes: 0 when equal, else 1. */
s32 contPakCompare(u8 *a, u8 *b, s32 n) {
    u8 *p = a;
    u8 *q = b;
    s32 i;
    s32 differ;

    for (i = 0; i < n; i++) {
        differ = *p++ != *q++;
        if (differ) {
            return 1;
        }
    }
    return 0;
}

/* Read every directory entry's state; stop at the first pak-level error (2, 3 or 4). (Which
 * accesses go through `pak` and which through the array decides the order of the two pointer
 * increments.) */
s32 contPakReadFileStates(s32 port) {
    ContPak *pak = &gContPak[port];
    s32 i;

    DPRINTF("--- pakReadDir ---\n");
    for (i = 0; i < 16; i++) {
        gContPak[port].fileState[i] =
            osPfsFileState(&gContPak[port].pfs, i, &gContPak[port].files[i]);
        if (gContPak[port].fileState[i] == PFS_ERR_CONTRFAIL ||
            gContPak[port].fileState[i] == PFS_ERR_NEW_PACK ||
            gContPak[port].fileState[i] == PFS_ERR_INCONSISTENT) {
            return gContPak[port].error = pak->fileState[i];
        }
    }
    gContPak[port].error = 0;
    return 0;
}

/* Delete a file from the pak, by its directory entry. */
void contPakDeleteFile(s32 file, s32 port) {
    ContPak *pak = &gContPak[port];
    OSPfsState *state = &pak->files[file];

    DPRINTF("--- pakDeteleFile ---\n");
    gContPak[port].error = osPfsDeleteFile(&pak->pfs, state->company_code, state->game_code,
                                                     (u8 *) state->game_name, (u8 *) state->ext_name);
}

/* The pak's free space, in 256-byte pages. */
void contPakFreePages(s32 port, s32 *pages) {
    s32 ret;
    s32 bytes;

    DPRINTF("--- pakFreeBlocks ---\n");
    ret = osPfsFreeBlocks(&gContPak[port].pfs, &bytes);
    *pages = bytes / 256;
    DPRINTF("free blocks: %d\n");
    gContPak[port].error = ret;
}

void contPakNumFiles(s32 port, s32 *max_files, s32 *files_used) {
    ContPak *pak = &gContPak[port];

    DPRINTF("--- pakNumFiles ---\n");
    gContPak[port].error = osPfsNumFiles(&pak->pfs, max_files, files_used);
    DPRINTF("num_files: max:%d, current:%d\n");
}

/* Open the Controller Pak; a damaged ID gets one repair. */
s32 contPakInitPfsRepair(s32 port) {
    s32 ret = osPfsInitPak(&gSiMesgQueue, &gContPak[port].pfs, port);

    if (ret == PFS_ERR_ID_FATAL) {
        DPRINTF("pakRepair\n");
        ret = osPfsRepairId(&gContPak[port].pfs);
    }
    return ret;
}

/* Has the pak in this port been pulled? */
s32 contPakIsPulled(s32 port) {
    return contPakGetStatus(port)->status & CONT_CARD_PULL;
}

/* This port's controller status, queried at most once a frame.
 * The game's address is ONE compiler temp: computed before osContGetQuery and spilled across it
 * (0x1C), computed again in the else block, joined in a0 for the return. That is IDO's partial
 * redundancy elimination of the return's &gContQueryStatus[port], which needs an else block to put
 * the second computation in: the else below, which holds the dead "cached" print. */
OSContStatus *contPakGetStatus(s32 port) {
    DPRINTF(" (--- __pakGetQuery ---) ");
    if (gGameFrameCount != gContPakQueryFrame) {
        DPRINTF("call\n");
        osContStartQuery(&gSiMesgQueue);
        osRecvMesg(&gSiMesgQueue, NULL, OS_MESG_BLOCK);
        osContGetQuery(&gContQueryStatus[port]);
        gContPakQueryFrame = gGameFrameCount;
    } else {
        DPRINTF("cached\n");
    }
    return &gContQueryStatus[port];
}

/* The rumble state machine, once a frame per port. */
s32 contPakRumbleUpdate(s32 port) {
    ContPak *pak = &gContPak[port];

    switch (pak->rumbleMode) {
        case 0:
            break;
        case 1: /* the pattern, one bit a frame, for rumbleTime frames */
            if (pak->pakType == CONTPAK_TYPE_RUMBLE) {
                contPakMotorSet(port, pak->rumblePattern & pak->rumbleBit, pak->rumbleLast);
                pak->rumbleLast = pak->rumblePattern & pak->rumbleBit;
                if (--pak->rumbleTime <= 0) {
                    pak->rumbleMode = 2;
                } else {
                    pak->rumbleBit >>= 1;
                    if (pak->rumbleBit == 0) {
                        pak->rumbleBit = 0x80000000;
                    }
                }
            }
            break;
        case 2: /* stop, retrying up to 5 times */
            if (contPakMotorSet(port, 0, 1) == 0) {
                pak->rumbleMode = 0;
            }
            if (++pak->retries >= 5) {
                pak->rumbleMode = 0;
            }
            break;
        case 3: /* keep stopping for rumbleTime frames */
            if (pak->rumbleTime > 0) {
                pak->rumbleTime--;
                contPakMotorSet(port, 0, 1);
            } else {
                pak->rumbleMode = 0;
            }
            break;
    }
    return 0;
}

/* Turn the motor on or off. Only on a change (`force` is the last state) or while retrying; on is
 * refused while the game-state flags at 0x80088338 +0x15/+0x19 are set. After 5 failures, stop. */
s32 contPakMotorSet(s32 port, s32 on, s32 force) {
    ContPak *pak;
    s32 ret;

    if (on && gGameWork.demoPlaying == 0 && gGameWork.rumbleSuspended == 0) {
        pak = &gContPak[port];
        if (pak->retries != 0 || !force) {
            ret = __osMotorAccess(&pak->pfs, 1);
        }
    } else {
        pak = &gContPak[port];
        if (pak->retries != 0 || force) {
            ret = __osMotorAccess(&pak->pfs, 0);
        }
    }
    if (ret != 0) {
        if (++pak->retries >= 5) {
            pak->retries = 0;
            pak->rumbleMode = 2;
        }
    }
    return ret;
}

/* Stop the rumble. */
s32 contPakRumbleStop(s32 port) {
    ContPak *pak = &gContPak[port];

    if (pak->pakType != CONTPAK_TYPE_RUMBLE) {
        return 1;
    }
    pak->rumbleTime = 0;
    pak->retries = 0;
    pak->rumbleMode = 2;
    return 0;
}

/* Rumble with a pattern (one bit per frame, from bit 31) for a time. */
s32 contPakRumbleStart(s32 port, u32 pattern, s32 time) {
    ContPak *pak = &gContPak[port];

    if (pak->pakType != CONTPAK_TYPE_RUMBLE || gContPakRumbleSuspended != 0) {
        return 1;
    }
    pak->rumbleMode = 1;
    pak->rumbleTime = time;
    pak->rumblePattern = pattern;
    pak->rumbleBit = 0x80000000;
    pak->rumbleLast = 0;
    return 0;
}

s32 contPakRumbleRequestStop(s32 port) {
    if (gContPakRumbleSuspended != 0) {
        return 1;
    }
    gContPak[port].rumbleMode = 2;
    return 0;
}

/* Reset: refuse new rumbles, re-probe every port and set each to a 3-frame stop. */
void contPakRumbleSuspend(void) {
    s32 i;

    gContPakRumbleSuspended = 1;
    for (i = 0; i < MAXCONTROLLERS; i++) {
        contPakInitRumble(i);
        gContPak[i].rumbleMode = 3;
        gContPak[i].rumbleTime = 3;
    }
}

void contPakRumbleResume(void) {
    s32 i;

    gContPakRumbleSuspended = 0;
    for (i = 0; i < MAXCONTROLLERS; i++) {
        gContPak[i].rumbleMode = 0;
    }
}

/* CRC-32 (polynomial 0x04C11DB7, MSB first, initial and final value inverted). */
u32 contPakCrc32(u8 *data, u32 size) {
    u32 crc = 0xFFFFFFFF;
    u32 i;
    s32 bit;

    for (i = 0; i < size; i++) {
        crc ^= data[i] << 24;
        for (bit = 0; bit < 8; bit++) {
            if (crc & 0x80000000) {
                crc = (crc << 1) ^ 0x04C11DB7;
            } else {
                crc <<= 1;
            }
        }
    }
    return ~crc;
}

/* This file's .data (subsegment 0x25140), in ROM order. */
u8 gContPakGameName[16] = {
    0x20, 0x2B, 0x1E, 0x1A, 0x2D, 0xF, 0x20, 0x28, 0x1E, 0x26, 0x28, 0x27, 0x0, 0x0, 0x0, 0x0,
};
s32 gContPakQueryFrame = -1;
