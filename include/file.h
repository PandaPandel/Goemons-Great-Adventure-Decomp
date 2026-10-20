#ifndef FILE_H
#define FILE_H

/* The file loader (src/file.c) on the "Nisitenma-Ichigo" file system: a queue of load requests,
 * each a list of (file, RAM) pairs, serviced once a frame by thread 6. */

#include "common.h"

#define FILELIST_LAST 0x40000000 /* in FileList.file: the list's last entry */

typedef struct FileList {
    /* 0x0 */ u32 file; /* the file number, | FILELIST_LAST on the last entry */
    /* 0x4 */ void *ram;
} FileList; /* size 0x8 */

typedef struct FileReq {
    /* 0x0 */ FileList *list;
    /* 0x4 */ u8 mode;
} FileReq; /* size 0x8 */

extern FileReq gFileQueue[8];  /* the request queue */
extern u8 gFileQueueCount;          /* requests queued */
extern FileList gFileQueueSingleList[];  /* entries for single-file requests */
extern u16 gFileSingleListUsed;         /* entries used */

void fileInit(void);
void fileClearQueue(void);
void fileQueueList(FileList *list);
void fileQueueListKeep(FileList *list);
void fileQueueListMode(FileList *list, u8 mode);
void fileQueue(u32 file, void *ram);
void fileQueueKeep(u32 file, void *ram);
void fileQueueMode(u32 file, void *ram, u8 mode);
void fileLoadListNow(FileList *list);
void fileLoadListNowKeep(FileList *list);
void fileLoadListNowMode(FileList *list, u8 mode);
void fileLoadNow(u32 file, void *ram);
void fileLoadNowKeep(u32 file, void *ram);
void fileLoadNowMode(u32 file, void *ram, u8 mode);
void fileServiceQueue(void);
u32 fileLoadAtNextFree(u32 file, u8 mode);
void fileEmptyStub(void);
void *fileAllocAndLoad(u32 file, u8 mode);
void *fileLoadToAddr(u32 file, void *ram, u8 mode);
void *fileLoad(u32 file, u8 *ram);
s32 fileLoadAttachments(u16 file, void *ram);
u8 fileLoadAttachment(u32 file, u8 *dst);
u8 *fileLoadOrSubfile(u32 file, u8 *ram);
u32 fileRomSize(u32 file, u32 *rom);
u32 fileRomStart(u16 file);
u32 fileRomEnd(u16 file);
s32 fileRamSize(u32 file);
u8 fileGroup(u32 file);
u32 fileRamAddr(u32 file);
u32 fileRamAddrAt(u32 file, u32 offset);
u32 fileIsTlbMapped(u32 file);
u32 fileMappedAddr(void);
s32 fileMapTlb(u32 arg);
void fileUnmapAllTlb(void);
s32 fileLoadOverlay(u16 file);

#endif
