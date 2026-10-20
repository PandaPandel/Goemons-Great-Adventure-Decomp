#ifndef SCENARIO_H
#define SCENARIO_H

#include "object.h"
#include "game.h"
#include "file.h"
#include "cache.h"

/* A scenario command. */
typedef struct ScCmd {
    /* 0x0 */ u8 op;
    /* 0x4 */ s32 arg;
} ScCmd;

/* A cache of 0x80-byte blocks read from ROM (scCacheGetBlock). */
typedef struct ScCache {
    /* 0x00 */ u8 pad0[0x34];
    /* 0x34 */ struct Cache *cache;
    /* 0x38 */ u32 rom;
} ScCache;

void scVarsLocate(void);
void scVarsReset(void);
s32 scParentProc(Obj *obj, s16 msg, void *arg);
s32 scParentNoFilesProc(Obj *obj, s16 msg, void *arg);
s32 scRun_proc(Obj *obj, s16 msg, void *arg);
s32 scRun_proc2(Obj *obj, s16 msg, void *arg);
Obj *scRunStart(s32 n, s32 player, s32 plKind);
Obj *scRunStart2(s32 n, s32 player, s32 plKind);
Obj *scRunEnd(void);
s32 scIsRunning(void);
void scSetNo(Obj *obj, s32 n);
s32 scGetNo(Obj *obj);
void scSetPlNo();
void scSetPlKind();
void scVarsSetPlNo(Obj *obj);
void scVarsSetPlKind(Obj *obj);
void scLoadFiles(void);
void scFreeFiles(void);
void scLoadScenario(s32 n);
void scFreeScenario(s32 n);
Frame *scGetFrame(Obj *obj, s32 i);
void scRunCommands(Obj *obj);
s32 scRunMessage(Obj *obj);
s32 scPadTrg(s32 player, u16 mask);
s32 scPadHold(s32 player, s32 mask);
s32 scPadRepeat(s32 player, s32 mask);
void scPadStickDpad(s32 player, s32 on);
void scCheckCancel(Obj *obj);
void scUpdateWait(Obj *obj);
Obj *scStringTaskCheck(Obj *obj, s32 i);
ScCmd *scGetPc(Obj *obj);
s16 *scGetText(Obj *obj);
Obj *scStringTaskCheck2(Obj *obj, s32 i);
s32 scRunGetNo(Obj *obj);
u16 scGetFile(s32 n);
u32 scGetStart(s32 n);
s32 scIsWindowOpened(Obj *obj);
u8 *scCacheGetBlock(ScCache *c, s32 n);

#endif
