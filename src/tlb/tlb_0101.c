#include "tlb.h"

/* "connect": a stage exit. When the players reach it (all of them, any one, or the first one near
 * it, by its kind), it stops the rumble, fades the screen out, plays its tune and moves the game
 * to the stage it leads to (its placement's data: the stage, the angle and the spawn). The
 * "runs" walk the players out through it first (RUN_GO, RUN_BACK) or stand them (RUN_STOP). */

/* An enemy's work (EneObj) to 0x34, then its own. */
typedef struct ConnectWork {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ Frame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u32 timer;
    /* 0x38 */ u8 step; /* 0 starting, 1 waiting for the players, 2 fading, 3 leaving */
    /* 0x39 */ u8 sided; /* the run's way is chosen */
    /* 0x3C */ f32 run; /* how far the players run past it (+-160) */
    /* 0x40 */ s32 plZone; /* the first player's course at the start (0: none) */
    /* 0x44 */ ConnectData *data;
} ConnectWork;

void connectStart(Obj *obj);
void connectWaitNear(Obj *obj);
void connectWaitAll(Obj *obj);
void connectWaitStand(Obj *obj);
void connectWaitAny(Obj *obj);
void connectFade(Obj *obj);
void connectFadeAll(Obj *obj);
void connectFadeStand(Obj *obj);
void connectGo(Obj *obj);
s32 connectPlNear(Obj *obj, s32 i, f32 r);
s32 connectPlNearWorld(Obj *obj, s32 i, f32 r);

s32 connectProc(Obj *obj, s16 msg, void *arg) {
    ConnectWork *w = FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "connect";
            w->data = (ConnectData *) stageAddrToRam(eneSetup(obj, arg));
            w->model = (Frame *) eneNewPol(obj, NULL);
            if (w->model == NULL) {
                PRINTF("connect : ダミー作成失敗\n");
                return -1;
            }
            w->step = 0;
            PRINTF("connect : [%d] へ繋がるconnectが生まれました\n", w->data->stage);
            w->timer = 0;
            break;
        case 1:
            PRINTF("connect : [%d] へ繋がるconnectが死にました\n", w->data->stage);
            if (w->model != NULL) {
                frmDestroy(w->model);
            }
            break;
        case 0:
            switch (w->step) {
                case 0:
                    connectStart(obj);
                    break;
                case 1:
                    if (w->data->kind == 2) {
                        connectWaitAll(obj);
                    } else if (w->data->kind == 4) {
                        connectWaitStand(obj);
                    } else if (w->data->kind == 8) {
                        connectWaitAny(obj);
                    } else {
                        connectWaitNear(obj);
                    }
                    break;
                case 2:
                    if (w->data->kind == 2) {
                        connectFadeAll(obj);
                    } else if (w->data->kind == 4) {
                        connectFadeStand(obj);
                    } else {
                        connectFade(obj);
                    }
                    break;
                case 3:
                    connectGo(obj);
                    break;
            }
            eneSetModelOnCourse(obj, w->model);
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Step 0: three frames on, the first player in play's course. */
void connectStart(Obj *obj) {
    ConnectWork *w = FX(obj);
    s32 i = 0;
    s32 pad;
    f32 x;
    f32 y;
    f32 z;

    if (++w->timer == 3) {
        w->timer = 0;
        w->step = 1;
        while (!plIsInPlay(i)) {
            i++;
        }
        plAskPos(gGameWork.players[i], &x, &y, &z, &w->plZone);
    }
}

/* Step 1, the first player near: unless a player is in a demo, run (or stand) them and fade. */
void connectWaitNear(Obj *obj) {
    ConnectWork *w = FX(obj);
    s32 near;
    s32 i;
    s32 state;

    near = shimPlNearCheck(obj, w->data->radius);
    if (near != 0 && w->sided == 0) {
        if (shimPlSideCheck(obj, shimNearestPl(obj)) == 4) {
            w->run = 160.0f;
        } else {
            w->run = -160.0f;
        }
        w->sided = 1;
    }
    if (near != 0) {
        for (i = 0; i < 4; i++) {
            if (plIsInPlay(i)) {
                state = shimPlGetState((Player *) gGameWork.players[i]);
                if (state >= 0xD6 && state < 0xE6) {
                    PRINTF("connect デモ中なので無視無視\n");
                    return;
                }
            }
        }
        if (w->plZone == 0 || w->data->run == 4) {
            PRINTF("connect RUN_STOP!!!\n");
            demoPlStandAll();
        } else if (w->data->run == 2) {
            PRINTF("connect RUN_BACK!!!\n");
            demoSetTimePlRun(w->x - w->run, w->y, w->z, 8.0f, w->x - w->run, w->y, w->z, 8.0f);
        } else {
            PRINTF("connect RUN_GO!!!\n");
            demoSetTimePlRun(w->x + w->run, w->y, w->z, 8.0f, w->x + w->run, w->y, w->z, 8.0f);
        }
        w->timer = 0;
        w->step = 2;
    } else {
        w->sided = 0;
    }
}

/* Step 1, kind 2: each player in play with flag 0x10000 who is near is marked (flag 0x40) and
 * the fade starts. (Three pad words above in[]: the game's frame.) */
void connectWaitAll(Obj *obj) {
    s32 i;
    s32 pad[3];
    s32 in[4];
    ConnectWork *w;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            if (PL(gGameWork.players[i])->flags & 0x10000) {
                in[i] = i;
            } else {
                in[i] = -1;
            }
        }
    }
    for (i = 0; i < 4; i++) {
        w = FX(obj);
        if (in[i] != -1 && connectPlNear(obj, i, w->data->radius)) {
            ((Player *) (gGameWork.players[i] + 1))->flags |= 0x40;
            w->timer = 0;
            w->step = 2;
        }
    }
}

/* Step 1, kind 4: as the first player near, standing. */
void connectWaitStand(Obj *obj) {
    ((ConnectWork *) FX(obj))->data->run = 4;
    connectWaitNear(obj);
}

/* Step 1, kind 8: any player in play near is marked (0x40). */
void connectWaitAny(Obj *obj) {
    ConnectWork *w = FX(obj);
    s32 i;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            if (connectPlNear(obj, i, w->data->radius)) {
                ((Player *) (gGameWork.players[i] + 1))->flags |= 0x40;
                w->timer = 0;
                w->step = 2;
            }
        }
    }
}

/* Step 2: the rumble off, the fade out and the tune; eight frames on, step 3. */
void connectFade(Obj *obj) {
    ConnectWork *w = FX(obj);
    s32 i;

    if (w->timer == 0) {
        for (i = 0; i < 4; i++) {
            contPakInitRumble(i);
            contPakRumbleRequestStop(i);
        }
        fadeSetAll(8.0f, 0.0f, 255.0f, 0, 0, 0);
        if (w->data->tune != 0) {
            sndPlay(w->data->tune);
        }
    } else {
        for (i = 0; i < 4; i++) {
            contPakRumbleRequestStop(i);
        }
    }
    if (++w->timer >= 9) {
        w->timer = 0;
        w->step = 3;
    }
}

/* Step 2, kind 2: the same, with sound 8 first, unless flag 10 is set and flag 0xA3 is not. */
void connectFadeAll(Obj *obj) {
    ConnectWork *w = FX(obj);
    s32 i;

    if (w->timer == 0) {
        for (i = 0; i < 4; i++) {
            contPakInitRumble(i);
            contPakRumbleRequestStop(i);
        }
        fadeSetAll(8.0f, 0.0f, 255.0f, 0, 0, 0);
        if (!swWorkTest(0xA) || swWorkTest(0xA3)) {
            sndPlay(8);
            if (w->data->tune != 0) {
                sndPlay(w->data->tune);
            }
        }
    } else {
        for (i = 0; i < 4; i++) {
            contPakRumbleRequestStop(i);
        }
    }
    if (++w->timer >= 9) {
        w->timer = 0;
        w->step = 3;
    }
}

/* Step 2, kind 4: the same, with sound 0x8252 first. */
void connectFadeStand(Obj *obj) {
    ConnectWork *w = FX(obj);
    s32 i;

    if (w->timer == 0) {
        for (i = 0; i < 4; i++) {
            contPakInitRumble(i);
            contPakRumbleRequestStop(i);
        }
        fadeSetAll(8.0f, 0.0f, 255.0f, 0, 0, 0);
        sndPlay(0x8252);
        if (!swWorkTest(0xA) || swWorkTest(0xA3)) {
            sndPlay(8);
            if (w->data->tune != 0) {
                sndPlay(w->data->tune);
            }
        }
    } else {
        for (i = 0; i < 4; i++) {
            contPakRumbleRequestStop(i);
        }
    }
    if (++w->timer >= 9) {
        w->timer = 0;
        w->step = 3;
    }
}

/* Step 3: go to the stage (four stages first play their demo once, from the stage menu). */
void connectGo(Obj *obj) {
    ConnectWork *w = FX(obj);
    s32 pad;

    if (w->timer == 0) {
        w->timer++;
        sndClearReverb();
        switch (w->data->stage) {
            case 0x8B:
                if (!(gSaveData.stageFlags[0x6F] & 0x80)) {
                    dmenuQueueDemo(0xD);
                    sndPlay(0xA);
                    return;
                }
                break;
            case 0x9A:
                if (!(gSaveData.stageFlags[0x65] & 0x80)) {
                    dmenuQueueDemo(1);
                    sndPlay(0xA);
                    return;
                }
                break;
            case 0xB6:
                if (!(gSaveData.stageFlags[0x68] & 0x80)) {
                    dmenuQueueDemo(6);
                    sndPlay(0xA);
                    return;
                }
                break;
            case 0xC9:
                if (!swWorkTest(0xBF) && !(gSaveData.stageFlags[0x6A] & 0x80)) {
                    dmenuQueueDemo(8);
                    sndPlay(0xA);
                    return;
                }
                break;
        }
        if (gGameWork.stage == 0x4D) {
            PRINTF("******>> 77面connect: SW_HANABI_SD_NONE ON!\n");
            swWorkSet(0x4D);
            gGameWork.mission = 0;
        }
        stageGotoSpawnNoState(gStageTask->parent, w->data->stage, w->data->angle, w->data->x, w->data->y,
                              w->data->z, w->data->facing);
    }
}

/* Whether player i is within r (a box) of it, on its course (off the courses: the next). */
s32 connectPlNear(Obj *obj, s32 i, f32 r) {
    s32 pad[2];
    f32 x;
    f32 y;
    f32 z;
    s32 zone;
    ConnectWork *w = FX(obj);
    f32 c;
    f32 d;
    s32 pad2[2];

    if (plIsInPlay(i)) {
        plAskPos(gGameWork.players[i], &x, &y, &z, &zone);
        if (zone == 0) {
            return connectPlNearWorld(obj, i, r);
        }
        if (zone == w->zone) {
            c = w->x;
            if (c < x) {
                d = x - c;
            } else {
                d = -(x - c);
            }
            if (d <= r) {
                c = w->y;
                if (c < y) {
                    d = y - c;
                } else {
                    d = -(y - c);
                }
                if (d <= r) {
                    c = w->z;
                    if (c < z) {
                        d = z - c;
                    } else {
                        d = -(z - c);
                    }
                    if (d <= r) {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

/* The same in the world. */
s32 connectPlNearWorld(Obj *obj, s32 i, f32 r) {
    s32 pad[2];
    f32 x;
    f32 y;
    f32 z;
    f32 ex;
    f32 ey;
    f32 ez;
    s32 pad2[4];

    vramCoursePointXZ(ENE(obj)->zone, ENE(obj)->x, ENE(obj)->z, &ex, &ez);
    ey = ENE(obj)->y;
    if (plIsInPlay(i)) {
        plGetModelPos(gGameWork.players[i], &x, &y, &z);
        if (((ex < x) ? x - ex : -(x - ex)) <= r) {
            if (((ey < y) ? y - ey : -(y - ey)) <= r) {
                if (((ez < z) ? z - ez : -(z - ez)) <= r) {
                    return 1;
                }
            }
        }
    }
    return 0;
}
