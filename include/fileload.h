#ifndef FILELOAD_H
#define FILELOAD_H

#include "common.h"
#include "enemy.h"

FileSet floadGetSet(s32 n);
s16 floadGetSetFile(s32 n, s32 i);
void floadGetSetRam(void **ram, s32 n);
u32 floadSegToRam(u32 addr, s32 n, s32 i);
void floadFileToTop(u16 file);
s32 floadIsSetLoaded(s32 n);
FileSet floadLoadSet(s32 n);
FileSet floadLoadSetToTop(s32 n);
FileSet floadFreeSet(s32 n);
void floadLoadSetRam(void **ram, s32 n);

#endif
