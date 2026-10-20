#ifndef MENU_H
#define MENU_H

#include "object.h"

s32 menuBaseRun_proc(Obj *obj, s16 msg, void *arg);
Obj *mbCreate(Obj *parent);
void mbDelete(Obj *base);
void mbMenuAdd(Obj *base);
void mbMenuDelete(Obj *base);
void mbAMenuDelete(Obj *base, u8 id);
s32 mbMenuExistCheck(Obj *base);
void *mbScPtrGet(Obj *base, u8 id);
s32 mbMenuCheck(Obj *base);
void mbChoiceToVar(Obj *base);
void *mbScJumpPtrGet(Obj *base);
s32 scMenu_proc(Obj *obj, s16 msg, void *arg);
Obj *menuCreate(Obj *parent);
void mbMenuTaskDelete(Obj *menu);
u8 mbMenuIsDecided(Obj *menu);
void *mbMenuScPtrGet(Obj *menu);
u8 mbMenuIdCheck(Obj *menu, u8 id);
u32 mbMenuPlayerGet(Obj *menu);
u8 mbMenuIdGet(Obj *menu);
Obj *mbMenuCursorGet(Obj *menu);
void mbChoiceSet(Obj *menu, s32 choice, u32 player);
void mbChoiceReset(Obj *menu, s32 player);
void mbPlayerInMenuOn(Obj *menu, s32 player);
void mbPlayerInMenuOff(Obj *menu, s32 player);
s32 mbEntryIndexGet(Obj *entry);
Obj *mbEntryLastGet(Obj *menu);
s32 scAMenu(Obj *obj, s16 msg, void *arg);
Obj *entryCreate(Obj *menu, s16 x, s16 y, void *jump, u8 flags);
void mbEntryDelete(Obj *entry);
void mbEntryCursorOn(Obj *entry);
void mbEntryCursorOff(Obj *entry);
void mbEntryDecide(Obj *entry);
void mbEntryUndecide(Obj *entry);
s32 mbEntryIsDecided(Obj *entry);
void *mbEntryJumpGet(Obj *entry);
void mbEntryJumpSet(Obj *entry, void *jump);

#endif
