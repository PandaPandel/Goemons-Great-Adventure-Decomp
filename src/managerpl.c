#include "player.h"
#include "game.h"
#include "audio.h"
#include "camera.h"
#include "fade.h"
#include "gauge.h"
#include "managerpl.h"
#include "pldive.h"
#include "plmain.h"
#include "plmove.h"
#include "plsound.h"
#include "plutil.h"
#include "stage.h"
#include "tmanager.h"
#include "vram.h"

/* The game's debug print, argument-only (strings kept, arguments evaluated): what this file's
 * functions were built with. The repo's `if (0)` PRINTF moves registers here; this makes no
 * code, and each string lands in .rodata at its place (IDO_NOTES 201). */
#define DPRINTF

/* managerpl.c (the game's own name, in a dead print): the players' parent task, "pl親". Each
 * step it supervises the players: one powered down (flag 0x200) long enough comes back next to
 * another player, losing a life, or is removed when it has none; the last one out ends the
 * stage (to a restart or to game over, after a fade). A player below the others or under the
 * course is dropped back (action 0x2E). Players 3 and 4 join with START once the game is
 * finished and enough passes ("tegata") are held. */

#define PLM(task) ((PlManager *) ((task) + 1))

extern Obj *gStageTask; /* the stage task */
extern s8 gPlayerLives[]; /* per player (10 bytes each): lives */



/* The handler, with m set at its declaration and the pad above fell (after it, the pad takes
 * no slot). Its prints are DPRINTF (no code; the string literals stay in .rodata). */
s32 plManager(Obj *task, s16 msg, void *arg) {
    PlManager *m = PLM(task);
    s32 pad[2];
    s32 fell;
    s32 i;

    switch (msg) {
        case 2:
            task->name = "pl親";
            gGameWork.plManagerTask = task;
            m->pending = NULL;
            m->wait[0] = 0;
            m->wait[1] = 0;
            m->wait[2] = 0;
            m->wait[3] = 0;
            m->state = 0;
            break;
        case 0:
            switch (m->state) {
                case 0:
                    if (m->pending != NULL) {
                        tskUnlink(m->pending);
                        tskLink(m->pending, task);
                        m->pending = NULL;
                    }
                    switch (supervise(m)) {
                        case 0:
                            break;
                        case 1:
                            DPRINTF("plManager: GS_BEGIN\n");
                            m->state = 1;
                            m->timer = 15;
                            sndPlay(10);
                            camSunClearTunes();
                            fadeSetSpeed(10.0f);
                            fadeSetColor(0, 0, 0);
                            fadeSetTarget(255.0f);
                            break;
                        case 2:
                            DPRINTF("plManager: GS_OVER\n");
                            m->state = 2;
                            m->timer = 23;
                            sndPlay(10);
                            camSunClearTunes();
                            fadeSetSpeed(15.0f);
                            fadeSetColor(0, 0, 0);
                            fadeSetTarget(255.0f);
                            break;
                    }
                    fell = plFindFallen();
                    for (i = 0; i < 4; i++) {
                        if ((1 << i) & fell) {
                            plSetAction(gGameWork.players[i], 0x2E);
                        }
                    }
                    break;
                case 1:
                    if (--m->timer <= 0) {
                        plRestartStage();
                        m->state = 3;
                    }
                    break;
                case 2:
                    if (--m->timer <= 0) {
                        stageAllOut();
                        m->state = 3;
                    }
                    break;
                case 3:
                    break;
            }
            plCheckJoin();
            break;
        case 1:
            DPRINTF("plManager: destroyed\n");
            gGameWork.plManagerTask = NULL;
            break;
    }
    return tskDefaultProc(task, msg, arg);
}

/* supervise: 1 when the last player in play is out with lives left (a restart), 2 when it is
 * out for good (game over). Its prints are DPRINTF: as the repo's `if (0)` PRINTF, the dead
 * blocks cost the global's fp. */
s32 supervise(PlManager *m) {
    s32 pad[3];
    s32 r = 0;
    s32 left = plCount();
    s32 i;
    Player *p;
    Player *near;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            p = PL(gGameWork.players[i]);
            if (p->flags & 0x200) {
                if (plDownTimeUp(p)) {
                    if (gPlayerLives[p->index * 10] >= 2) {
                        DPRINTF("まだプレイヤー残数がいる\n");
                        if (left == 1) {
                            r = 1;
                            break;
                        }
                        near = plFindActive();
                        if (near != NULL) {
                            if (plIsSafeSpot(near)) {
                                plRespawnNear(p, near);
                            } else {
                                DPRINTF("managerpl.c: supervise(%d): 再発生保留\n", i);
                                if (++m->wait[i] >= 30) {
                                    plRespawnNear(p, near);
                                }
                            }
                        } else {
                            return 1;
                        }
                    } else {
                        if (left == 1) {
                            r = 2;
                            break;
                        }
                        DPRINTF("__restart_here\n");
                        plDestroy(i);
                        left--;
                    }
                } else {
                    m->down[i]++;
                    if (plAnimEnded(p) || p->action == 0x2E) {
                        m->ended[i]++;
                    }
                }
            } else {
                m->down[i] = 0;
                m->ended[i] = 0;
                m->wait[i] = 0;
            }
        }
    }
    return r;
}

void plManagerUnusedStub1(void) {
}

/* The players to drop back: under the course, or further below the highest than the camera's
 * reach (1.4 of it). The locals sit in the game's slots (read off its spills: i at 0x7C, ps at
 * 0x68, ground 0x60, reach 0x50, so i and p are declared first). The second loop's two tests are
 * an if / else if, each with its own "r |= 1 << i" (the game's "b next; ori" then the else's
 * test). The clear "ps[i] = NULL" written before the player load puts the players pointer's
 * step in the loop's delay slot and the compiler temp at 0x48. */
s32 plFindFallen(void) {
    s32 i;
    Player *p;
    Player *ps[4];
    f32 x;
    f32 ground;
    f32 lo = 10000.0f;
    f32 hi = -10000.0f;
    s32 r = 0;
    f32 reach = fadeViewWidth(gGameWork.view) * 1.4f;

    for (i = 0; i < 4; i++) {
        ps[i] = NULL;
        p = PL(gGameWork.players[i]);
        if (gGameWork.players[i] != NULL) {
            if (!(p->flags & 0x10018032)) {
                ps[i] = p;
                vramCoursePoint(p->zone, p->place, p->height, &x, &ground, &x);
                if (p->height < ground - 60.0f) {
                    r |= 1 << i;
                }
                if (ground < lo) {
                    lo = ground;
                }
            }
            if (hi < p->height) {
                hi = p->height;
            }
        }
    }
    for (i = 0; i < 4; i++) {
        if (ps[i] != NULL) {
            if (ps[i]->height < lo - 60.0f) {
                r |= 1 << i;
            } else if (ps[i]->height < hi - reach) {
                r |= 1 << i;
            }
        }
    }
    return r;
}

void plManagerUnusedStub2(void) {
}

/* Deletes the player's task on the manager's next step. */
void plDeleteLater(Player *p) {
    PlManager *m = PLM(gGameWork.plManagerTask);

    m->pending = gGameWork.players[p->index];
}

/* A player comes back next to another (near), with a life fewer when its pips were empty. */
s32 plRespawnNear(Player *p, Player *near) {
    if (gSaveData.players[p->index].life == 0) {
        plRestDecrement(p);
    }
    gSaveData.players[p->index].life = 3;
    if (near == NULL) {
        return 0;
    }
    p->velX = p->velY = p->velZ = 0.0f;
    p->place = near->place;
    p->height = near->height + 2.0f;
    p->zone = near->zone;
    if (near->flags & 0x10000) {
        p->flags &= ~0x200;
        p->flags |= 0x10000;
        p->height -= 2.0f;
        plInvincibleSet(p, 60);
        plBlinkSet(p, 60);
        if (p->flags3 & 1) {
            plJoinSdSet(p);
            p->flags3 &= ~1;
        } else {
            plRestartSdSet(p);
        }
        __diveStart(p);
    } else {
        plActChange(p, 0x123);
    }
    return 1;
}

/* Whether a player stands safely (0 when it is 20 or more below the course). */
s32 plIsSafeSpot(Player *p) {
    f32 x;
    f32 y;

    if (p->flags & 0x10018030) {
        return 1;
    }
    vramCoursePoint(p->zone, p->place, p->height, &x, &y, &x);
    if (p->model->pos[1] < y - 20.0f) {
        return 0;
    }
    return 1;
}

s32 plRestartStage(void) {
    if (gGameWork.mission == 1) {
        gGameWork.mission = 2;
    } else {
        stageRestart(gStageTask->parent);
    }
    return 1;
}

/* The first player in play that is not powered down. Same code as gauge.c's gaugeJoinPartner:
 * the NULL test and the player's address both read the slot. */
Player *plFindActive(void) {
    Player *p;
    s32 i;

    for (i = 0; i < 4; i++) {
        if (gGameWork.players[i] != NULL) {
            p = PL(gGameWork.players[i]);
            if (!(p->flags & 0x200)) {
                return p;
            }
        }
    }
    return NULL;
}

s32 plCount(void) {
    s32 n = 0;
    s32 i;

    for (i = 0; i < 4; i++) {
        if (gGameWork.players[i] != NULL) {
            n++;
        }
    }
    return n;
}

/* A life fewer. The print's argument (the count left) is a guess: a DPRINTF makes no code. */
s32 plRestDecrement(Player *p) {
    gSaveData.players[p->index].lives--;
    DPRINTF("plRestDecrement %d\n", gSaveData.players[p->index].lives);
    if (gSaveData.players[p->index].lives <= 0) {
        gSaveData.players[p->index].lives = 0;
        return 0;
    }
    return 1;
}

/* Whether a powered-down player has waited long enough (0x28 steps after its animation, 0x73
 * with an item; or 400 steps). */
s32 plDownTimeUp(Player *p) {
    PlManager *m;
    s32 limit;

    if (gGameWork.plManagerTask != NULL) {
        m = PLM(gGameWork.plManagerTask);
    } else {
        return 1;
    }
    if (p != NULL && plIsInPlay(p->index) && (p->flags & 0x200)) {
        if (plHasItem(p)) {
            limit = 0x73;
        } else {
            limit = 0x28;
        }
        if (limit < m->ended[p->index] || m->down[p->index] >= 401) {
            return 1;
        }
    }
    return 0;
}

s32 plHasItem(Player *p) {
    if (p != NULL && plIsInPlay(p->index) &&
        (gSaveData.players[p->index].food != 0 || p->onigiri != NULL)) {
        return 1;
    }
    return 0;
}

/* Players 3 and 4 join with START. */
void plCheckJoin(void) {
    s32 i;

    for (i = 2; i < 4; i++) {
        if (plMoveHeldTime(i, 1) > 0 && plMoveHeldTime(i, 0x1000) == 1) {
            plTryJoin(i);
        }
    }
}

/* A join-in's checks: the game finished, 44 passes, no other join under way, players 1..n-1
 * present and n absent; then the first character nobody uses.
 * n sits in s0 because the chara loop holds six values in v0..a3 (walker, end, 1, used, task,
 * p); p stays an `addiu` (not folded into the load) because its value is read from the array.
 * `players = 0;` above the if puts `move v1,zero` in the first delay slot; the comma init
 * schedules `move a0,zero` after the two addiu. p is assigned in the if, so its live range is
 * one block and it is coloured before the task (v1, task a0). Its four prints are DPRINTF. */
void plTryJoin(s32 n) {
    s32 players;
    s32 sum;
    s32 i;
    s32 used[4];

    players = 0;
    if (gSaveData.allClear != 0) {
        for (sum = 0, i = 0; i < 6; i++) {
            sum += gSaveData.tegata[i];
        }
        if (sum < 0x2C) {
            DPRINTF("not enough tegata\n");
            return;
        }
        if (gGameWork.partake != 0) {
            DPRINTF("gw.partake != 0\n");
            return;
        }
        for (i = 0; i < n; i++) {
            if (gGameWork.players[i] != NULL) {
                players++;
            }
        }
        if (players != n) {
            DPRINTF("ex_players != pl_num\n");
            return;
        }
        if (gGameWork.players[n] != NULL) {
            DPRINTF("plD[pl_num] != 0\n");
            return;
        }
        for (i = 0; i < 4; i++) {
            used[i] = 0;
        }
        for (i = 0; i < 4; i++) {
            if (gGameWork.players[i] != NULL) {
                Player *p;

                p = PL(gGameWork.players[i]);
                used[p->chara] = 1;
            }
        }
        for (i = 0; i < 4; i++) {
            if (used[i] == 0) {
                plSpawnJoined(n, i);
                return;
            }
        }
    }
}

/* A new player n as character chara, next to one in play.
 * The mask is an s32 set at its declaration, so IDO keeps 0x20000010 in a register and `nor`s it
 * (assigned next to its use, it folds ~0x20000010); the same local then holds the new object,
 * as the frame has no slot for a separate one. Its print is a DPRINTF. */
void plSpawnJoined(s32 n, s32 chara) {
    PlInit init;
    s32 mask = 0x20000010;
    Player *near;
    Unk88282 *s;

    near = plFindActive();
    if (near == NULL) {
        s = &gSaveData.players[n];
    } else {
        s = &gSaveData.players[n];
        s->chara = chara;
        s->lives = 3;
        s->life = 3;
        s->money = 100;
        s->weaponLevel = 0;
        s->food = 0;
        plMakeInit(&init, n, chara);
        init.zone = near->zone;
        init.place = near->place;
        init.height = near->height;
        init.side = near->side;
        init.flags &= ~mask;
        init.flags |= near->flags & mask;
        init.flags2 |= 0x100;
        init.flags3 |= 1;
        if (init.flags & 0x10) {
            init.action = 0;
        } else {
            init.action = 0x123;
        }
        mask = (s32) plCreate(gStageTask, &init);
        if (mask != 0) {
            camAiAddLookat((Obj *) mask, n);
            gaugeSpreadPlayers(near->zone);
            return;
        }
        DPRINTF("!!!! プレイヤーの作成に失敗しました !!!!\n");
    }
    s->chara = -1;
}
