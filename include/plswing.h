#ifndef PLSWING_H
#define PLSWING_H

#include "player.h"

void plSwingDefault(Player *p, s32 msg, s32 a, s32 b);
s32 plSwingStep(Player *p);
void plSwingBoxOffset(Player *p);
void plSwingHang(Player *p, s32 msg, s32 a, s32 b);
void plSwingAct0A(Player *p, s32 msg, s32 a, s32 b);
void plSwingAct0B(Player *p, s32 msg, s32 a, s32 b);
void plSwingLetGoBoxOffset(Player *p);
void plSwingLetGo(Player *p, s32 msg, s32 a, s32 b);
void plSwingLetGoBack(Player *p, s32 msg, s32 a, s32 b);
void plSwingLetGoFwd(Player *p, s32 msg, s32 a, s32 b);
s32 plSwingAPressed(Player *p);

#endif
