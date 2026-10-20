#ifndef MANAGERPL_H
#define MANAGERPL_H

#include "player.h"
#include "game.h"

/* The task's work. */
typedef struct PlManager {
    /* 0x00 */ Obj *pending; /* a player's task to delete */
    /* 0x04 */ s32 down[4]; /* steps each player has been powered down */
    /* 0x14 */ s32 ended[4]; /* and steps since its animation ended */
    /* 0x24 */ s32 wait[4]; /* steps waiting for a safe place to come back */
    /* 0x34 */ s32 state; /* 0 playing, 1 restarting, 2 game over, 3 done */
    /* 0x38 */ s32 timer;
} PlManager;

s32 plManager(Obj *task, s16 msg, void *arg);
s32 supervise(PlManager *m);
void plManagerUnusedStub1(void);
s32 plFindFallen(void);
void plManagerUnusedStub2(void);
void plDeleteLater(Player *p);
s32 plRespawnNear(Player *p, Player *near);
s32 plIsSafeSpot(Player *p);
s32 plRestartStage(void);
Player *plFindActive(void);
s32 plCount(void);
s32 plRestDecrement(Player *p);
s32 plDownTimeUp(Player *p);
s32 plHasItem(Player *p);
void plCheckJoin(void);
void plTryJoin(s32 n);
void plSpawnJoined(s32 n, s32 chara);

#endif
