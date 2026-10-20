#ifndef TMANAGER_H
#define TMANAGER_H

#include "object.h"

typedef s32 (*TaskProc)(Obj *, s16, void *);

s32 tskFreeBlockProc(Obj *obj, s16 msg, void *arg);
s32 tskRootProc(Obj *obj, s16 msg, void *arg);
void tskResetBlock(TaskBlock *b);
void tskInit(void);
Obj *tskAllocBlocks(s32 n);
Obj *tskCreateRoot(TaskProc proc);
Obj *tskCreate(Obj *parent, TaskProc proc, u32 file, void *arg);
Obj *tskCreateBlocks(Obj *parent, TaskProc proc, s32 n, u32 file, void *arg);
void tskFreeBlock(TaskBlock *b);
s32 tskDefaultProc(Obj *obj, s16 msg, void *arg);
s32 tskSendMsg(Obj *obj, s16 msg, void *arg);
void tskUnlink(Obj *obj);
void tskLink(Obj *obj, Obj *parent);
Obj *tskDestroy(Obj *obj);
void tskDestroyChildren(Obj *obj);
s32 tskCountChildren(Obj *obj);
Obj *tskFind(Obj *obj, TaskProc proc, u32 file);
char *tskGetName(Obj *obj);
s32 tskStubReturnZero(void);

#endif
