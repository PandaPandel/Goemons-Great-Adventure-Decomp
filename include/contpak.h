#ifndef CONTPAK_H
#define CONTPAK_H

/* The Controller Pak and Rumble Pak layer (src/contpak.c): one block per controller port. Every
 * width below is the one the game's own loads and stores use. */

#include "common.h"

#define CONTPAK_TYPE_RUMBLE 2 /* pakType: a Rumble Pak answered osMotorInit */

typedef struct ContPak {
    /* 0x000 */ OSPfs pfs;
    /* 0x068 */ u8 pad68[0x4];
    /* 0x06C */ OSPfsState files[16]; /* the Controller Pak's directory */
    /* 0x26C */ u8 fileState[16]; /* the last osPfsFileState result per directory entry */
    /* 0x27C */ u8 pad27C[0x20];
    /* 0x29C */ s32 fileNo;   /* the game's save file on the pak */
    /* 0x2A0 */ s32 error;    /* the last libultra PFS/motor result */
    /* 0x2A4 */ u16 pakType;
    /* 0x2A6 */ u16 retries;  /* failed motor accesses; 5 gives up */
    /* 0x2A8 */ u16 rumbleMode; /* 0 idle, 1 pattern, 2 stop, 3 timed */
    /* 0x2AA */ u8 pad2AA[0x2];
    /* 0x2AC */ s32 rumbleTime;
    /* 0x2B0 */ u32 rumblePattern; /* one bit per frame, from bit 31 down */
    /* 0x2B4 */ u32 rumbleBit;
    /* 0x2B8 */ s32 rumbleLast;
} ContPak; /* size 0x2BC */

extern ContPak gContPak[MAXCONTROLLERS];

void contPakProbeAll(void);
s32 contPakProbe(s32 port);
s32 contPakInitPfs(s32 port);
s32 contPakInitRumble(s32 port);
s32 contPakCreateFile(s32 port);
s32 contPakFindFile(s32 port);
s32 contPakReadSlot(u8 *buf, s32 slot, s32 port);
s32 contPakRead(u8 *buf, s32 offset, s32 size, s32 port);
s32 contPakReadHeader(u8 *buf, s32 port);
s32 contPakWriteSlot(u8 *buf, s32 slot, s32 port);
s32 contPakWriteHeader(u8 *buf, s32 port);
s32 contPakWrite(u8 *buf, s32 offset, s32 size, s32 port);
s32 contPakCompare(u8 *a, u8 *b, s32 n);
s32 contPakReadFileStates(s32 port);
void contPakFreePages(s32 port, s32 *pages);
void contPakNumFiles(s32 port, s32 *max_files, s32 *files_used);
s32 contPakInitPfsRepair(s32 port);
s32 contPakIsPulled(s32 port);
OSContStatus *contPakGetStatus(s32 port);
s32 contPakRumbleUpdate(s32 port);
s32 contPakMotorSet(s32 port, s32 on, s32 force);
s32 contPakRumbleStop(s32 port);
s32 contPakRumbleStart(s32 port, u32 pattern, s32 time);
s32 contPakRumbleRequestStop(s32 port);
void contPakRumbleSuspend(void);
void contPakRumbleResume(void);
u32 contPakCrc32(u8 *data, u32 size);

#endif
