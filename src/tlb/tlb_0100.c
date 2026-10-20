#include "tlb.h"

/* File 100 ("exit.c", its own print says): "exit" and "escape", a stage's way out. A player
 * within its range freezes the players (or walks them out past it), the screen fades and the
 * game goes to the atlas (the map) with the stage marked as left; an escape marks only the
 * stage it connects to. */

#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)


/* An exit's work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct Exit {
    /* 0x00 */ u8 unused0;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *dmg;
    /* 0x38 */ u32 timer; /* steps in this state */
    /* 0x3C */ u8 state; /* 0 setup, 1 waiting for a player, 2 fading, 3 exit to the atlas, 4
                          * escape to the atlas */
    /* 0x3D */ u8 near; /* a player is in range */
    /* 0x3E */ u8 escape; /* 1: an escape */
    /* 0x3F */ u8 pad3F;
    /* 0x40 */ f32 runX; /* where the players walk to, from its x (+-160) */
    /* 0x44 */ ExitData *data;
    /* 0x48 */ s32 plZone; /* the first player's zone at the start (plAskPos) */
} Exit;

#define EXIT(obj) ((Exit *) FX(obj))

s32 exitStateStart(Obj *o);
void exitStateWait(Obj *o);
void exitStateFade(Obj *o);
void exitStateAtlas(Obj *o);
void exitEscapeStateAtlas(Obj *o);

/* The exit's handler. */
s32 exitProc(Obj *o, s16 msg, void *arg) {
    Exit *w = EXIT(o);
    s32 pad[6];

    switch (msg) {
        case 2:
            o->name = "exit";
            w->data = (ExitData *) stageAddrToRam(eneSetup(o, arg));
            PRINTF("range[%f] area[%d] point[%d] connect[%d] run[%d]\n", w->data->range, w->data->area,
                   w->data->point, w->data->connect, w->data->run);
            w->model = eneNewPol(o, NULL);
            if (w->model == NULL) {
                return -1;
            }
            w->dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, (Frame *) w->model, w->data->range * 10.0f,
                                w->data->range * 20.0f, -w->data->range * 10.0f, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->state = 0;
            w->timer = 0;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy((Frame *) w->model);
            }
            if (w->dmg != NULL) {
                frmDestroy((Frame *) w->dmg);
            }
            PRINTF("exit DESTROY\n");
            break;
        case 3:
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (exitStateStart(o) == -1) {
                        return -1;
                    }
                    break;
                case 1:
                    exitStateWait(o);
                    break;
                case 2:
                    exitStateFade(o);
                    break;
                case 3:
                    exitStateAtlas(o);
                    break;
            }
            eneSetModelOnCourse(o, (Frame *) w->model);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The escape's handler: an exit whose last state is 4. */
s32 exitEscapeProc(Obj *o, s16 msg, void *arg) {
    Exit *w = EXIT(o);
    s32 pad[6];

    switch (msg) {
        case 2:
            o->name = "exit";
            PRINTF("ESCAPE!!!!\n");
            w->data = (ExitData *) stageAddrToRam(eneSetup(o, arg));
            PRINTF("range[%f] area[%d] point[%d] connect[%d] run[%d]\n", w->data->range, w->data->area,
                   w->data->point, w->data->connect, w->data->run);
            w->model = eneNewPol(o, NULL);
            if (w->model == NULL) {
                return -1;
            }
            w->dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, (Frame *) w->model, w->data->range * 10.0f,
                                w->data->range * 20.0f, -w->data->range * 10.0f, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->state = 0;
            w->timer = 0;
            w->escape = 1;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy((Frame *) w->model);
            }
            if (w->dmg != NULL) {
                frmDestroy((Frame *) w->dmg);
            }
            PRINTF("escape DESTROY\n");
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (exitStateStart(o) == -1) {
                        return -1;
                    }
                    break;
                case 1:
                    exitStateWait(o);
                    break;
                case 2:
                    exitStateFade(o);
                    break;
                case 3: /* written out: the game's switch is a jump table (0-4) */
                    break;
                case 4:
                    exitEscapeStateAtlas(o);
                    break;
                default:
                    PRINTF("escape NANDEYANEN!!!!!!!!!\n");
                    break;
            }
            eneSetModelOnCourse(o, (Frame *) w->model);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* State 0: on the third step, reads the first player in play's zone; then state 1. */
s32 exitStateStart(Obj *o) {
    Exit *w = EXIT(o);
    s32 i;
    f32 place;
    f32 height;
    f32 side;
    s32 pad[2];

    w->timer++;
    if (w->timer == 3) {
        i = 0;
        while (!plIsInPlay(i)) {
            i++;
        }
        PRINTF("exit.c : **** plD is %d ****\n", i);
        plAskPos(gGameWork.players[i], &place, &height, &side, &w->plZone);
        w->timer = 0;
        w->state = 1;
    }
    return 0;
}

/* State 1: a player in range freezes the players (or walks them out past it); then state 2. */
void exitStateWait(Obj *o) {
    Exit *w = EXIT(o);
    s32 pad;
    s32 near;

    near = shimPlNearCheck(o, w->data->range);
    if (near && !w->near) {
        if (shimPlSideCheck(o, shimNearestPl(o)) == 4) {
            w->runX = 160.0f;
        } else {
            w->runX = -160.0f;
        }
        w->near = 1;
    }
    if (near) {
        if (w->plZone == 0 || w->data->run == 4) {
            demoPlFreezeAll();
        } else if (w->data->run == 2) {
            demoSetTimePlRun(w->x - w->runX, w->y, w->z, 30.0f, w->x - w->runX, w->y, w->z, 30.0f);
        } else {
            demoSetTimePlRun(w->x + w->runX, w->y, w->z, 30.0f, w->x + w->runX, w->y, w->z, 30.0f);
        }
        w->timer = 0;
        w->state = 2;
    } else {
        w->near = 0;
    }
}

/* State 2: stops the rumble, fades out over 30 steps; then state 3 (4 for an escape). */
void exitStateFade(Obj *o) {
    Exit *w = EXIT(o);
    s32 i;

    if (w->timer == 0) {
        for (i = 0; i < 4; i++) {
            contPakInitRumble(i);
            contPakRumbleRequestStop(i);
        }
        fadeSetAll(30.0f, 0.0f, 255.0f, 0, 0, 0);
        sndPlay(10);
    } else {
        for (i = 0; i < 4; i++) {
            contPakRumbleRequestStop(i);
        }
    }
    w->timer++;
    if (w->timer > 30) {
        w->timer = 0;
        if (w->escape) {
            w->state = 4;
        } else {
            w->state = 3;
        }
    }
}

/* State 3: marks the stage left (and the one it connects to) and goes to the atlas. */
void exitStateAtlas(Obj *o) {
    Exit *w = EXIT(o);

    if (w->timer == 0) {
        sndClearReverb();
        sndPlay(0x521);
        gSaveData.stageFlags[w->data->area * 16 + w->data->point] |= 1;
        if (w->data->connect != -1) {
            gSaveData.areaFlags[w->data->area * 16 + w->data->connect] |= 1;
        }
        PRINTF("exit : ATLAS GO\n");
        stageGotoAtlas(w->data->area, w->data->point);
    }
    w->timer++;
}

/* State 4 (an escape): marks only the stage it connects to and goes to the atlas. */
void exitEscapeStateAtlas(Obj *o) {
    Exit *w = EXIT(o);

    if (w->timer == 0) {
        sndClearReverb();
        sndPlay(0x521);
        if (w->data->connect != -1) {
            gSaveData.areaFlags[w->data->area * 16 + w->data->connect] |= 1;
        }
        PRINTF("escape : ATLAS GO\n");
        stageGotoAtlas(w->data->area, w->data->point);
    }
    w->timer++;
}
