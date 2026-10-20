#ifndef SHIM_H
#define SHIM_H

#include "enemy.h"
#include "game.h"

s32 shimEneSwingX(Obj *o, f32 min, f32 max, f32 accel, f32 maxspeed);
s32 shimEneSwingY(Obj *o, f32 min, f32 max, f32 accel, f32 maxspeed);
s32 shimNearestPl(Obj *o);
s32 shimPlNearCheck(Obj *o, f32 r);
s32 shimPlNearCheckWorld(Obj *o, f32 r);
u8 shimPlSideCheck(Obj *o, s32 n);
void shimSdAtModel(Frame *f, u16 se);
void shimEneDeathAct(Obj *o, ACollFrame *box, u32 dl, u32 dl2);
s32 shimAtkKindCheck(ACollFrame *box);
void shimEneSetLowFlags(Obj *o, u8 n);
s32 shimIsKind120To126(s32 n);
void shimEneChangeCtrl(s32 ctrl, s32 p);
s32 shimPlGetState(Player *p);
s32 shimPlGetHitKind(Player *p);
void shimPlSetHitKind(Player *p, s32 v);
void shimEneMove(Obj *o, f32 h, f32 r, f32 a, f32 b);

#endif
