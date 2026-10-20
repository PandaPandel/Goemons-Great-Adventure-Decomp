#ifndef PLMODEL_H
#define PLMODEL_H

#include "player.h"

void plAnimeStep(Player *p);
void plAnimeSetCurAction(Player *p);
s32 pl_model_set(Player *p);
void plFaceAnimeStart(Player *p);
PlActAnim *plActAnimeGet(Player *p);
void plGlowOff(Player *p);
s32 __enoughInflateBufferP(u16 file);
s32 plInflateWorkCheck(void);

#endif
