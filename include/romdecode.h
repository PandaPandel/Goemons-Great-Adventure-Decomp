#ifndef ROMDECODE_H
#define ROMDECODE_H

#include "common.h"

void romClearBuf(void);
u8 *romDecode(u32 rom, u8 *dst, s32 size);
u8 *romNextByte(u8 *p);
void romRefill(void);
u8 *romInflate(u32 rom, u8 *dst, s32 size);
void free(void *p);
void *calloc(s32 count, s32 size);

#endif
