#ifndef PERSON_H
#define PERSON_H

#include "enemy.h"
#include "camera.h"
#include "game.h"

s32 personInit(Obj *o, u32 *anims, EneArg *arg, u8 mode);
s32 personInitMainTable(Obj *o, u32 *anims, EneArg *arg);
s32 personTalk(Obj *o, PersonTalk *t);
void personAct(Obj *o);
void personDelete(Obj *o);
void personNormal(Obj *o);
void personTurnToPlayer(Obj *o);
void personTalkStart(Obj *o);
void personTalking(Obj *o);
void personTalkEnd(Obj *o);
void personAnimeSet(Obj *o, u32 anim);
void personFlagOn(Obj *o, u8 mask);
void personFlagOff(Obj *o, u8 mask);
s32 personFlagCheck(Obj *o, u8 mask);
u8 personBoxKindGet(Obj *o);
u16 *personActStepRun(Obj *o, u16 *base, u16 *path, u32 *anims);
void personPlCollCheck(Obj *o, Player *pl, Vec3f *vel);
s32 personPlInWay(Obj *o, Obj *pl);
void personPlBlockCheck(Obj *o);
void personTalkCam(Obj *o);
void personTalkCamAt(Obj *o, Obj *player, u16 dist);
s32 personHoursCheck(Obj *o, u8 hours);
void demoCamZoom_CL(Obj *player);
void personTalkCamOff(void);
void personCamZoomUp(Obj *player);

#endif
