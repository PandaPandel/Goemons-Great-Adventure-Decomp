#ifndef FILERES_H
#define FILERES_H

#include "common.h"
#include "file.h"

void resClearTable(void);
void resClearModes(void);
void resModesOn(void);
void resModesOff(void);
void *resAllocTop(s32 size);
s32 resPush(u16 file, u32 addr, u8 mode);
s32 resRegister(u16 file, u32 addr, u32 size, u8 mode);
s32 resCount(void);
u16 resGetFile(s32 i);
u32 resGetAddr(s32 i);
s32 resGetMode(s32 i);
s32 resFindFile(u16 file);
s32 resFreeUnkept(void);
s32 resFreeKept(void);
s32 resFree(u16 file);
s32 resFreeForce(u16 file);
s32 resRelease(u16 file, s32 force);
s32 resFreeList(FileList *list);
s32 resFreeAllBut(u16 keep);
s32 resFreeNotInList(FileList *list);
u32 resFreeTop(void);
u32 resFreeAddr(u32 addr);
s32 resDrop(u16 file);
s32 resSetFile(s32 i, u16 file);
s32 resSetAddr(s32 i, u32 addr);
s32 resSetMode(s32 i, u8 mode);
s32 resRemove(s32 i);
s32 resInList(u16 file, FileList *list);
s32 resFindAddr(u32 addr);
void resEndRegister(void);
s32 resCountAddr(u32 addr);

#endif
