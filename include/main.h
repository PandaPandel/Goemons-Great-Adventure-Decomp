#ifndef MAIN_H
#define MAIN_H

#include "scheduler.h"
#include "game.h"

void mainSchedEventThread(Sched *sc);
void mainSchedBroadcast(Sched *sc, OSMesg msg);
void mainSchedAudioThread(Sched *sc);
void mainSchedGfxThread(Sched *sc);
void mainSchedSwapThread(Sched *sc);
void mainSchedAddTwoPending(Sched *sc);
void mainUnusedStub(s32 unused0, s32 unused1, s32 unused2, s32 unused3);
void main(void);
void mainIdleThread(void *arg);
void mainBootThread(void *arg);
void mainGameThread(void *arg);
void mainResetWipe(void);

#endif
