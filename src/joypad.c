#include "object.h"
#include "contpak.h"
#include "fade.h"
#include "heap.h"
#include "joypad.h"
#include "tmanager.h"

/* The joypad task ("joypad"): reads the four controllers each frame into 64 frames of history,
 * can turn a stick into D-pad presses, and counts how long each button has been held. It also
 * records and plays back demos. Recording only prints the keys to the debugger (DEMO_KEY_START,
 * DEMO_KEY_DATA, DEMO_KEY_END; compiled out). Playback feeds keys from runs of (frames, pad)
 * pairs, one list per player. Starting either resets the random seed to 0x1234 and the frame
 * count to 0, so a demo plays the same every time. */

/* A demo's keys for one player: a pad held for a number of frames; 0 frames ends it. */
typedef struct DemoKey {
    /* 0x0 */ u32 frames;
    /* 0x4 */ PadHist pad;
} DemoKey;

typedef struct DemoPlayer {
    /* 0x0 */ u16 frame; /* in the current run */
    /* 0x2 */ u16 run;
    /* 0x4 */ DemoKey *keys;
} DemoPlayer;

/* The joypad task's work. */
typedef struct DemoWork {
    /* 0x00 */ s8 nPlayers;
    /* 0x01 */ s8 mode; /* 0 live, 1 recording, 2 playing back */
    /* 0x02 */ u8 pad2[2];
    /* 0x04 */ DemoPlayer player[4];
} DemoWork;

/* joypad.c's .data (0x80108B50-0x80108B54), in ROM order. */
Obj *gJoypadTask = NULL; /* the joypad task */
extern OSMesgQueue gSiMesgQueue;
extern OSMesg gJoyMesgBuf;
extern OSContStatus gJoyContStatus[MAXCONTROLLERS];
extern OSContPad gJoyContPads[MAXCONTROLLERS];
extern u8 gJoyHistNewest; /* the newest frame in the history */
extern PadHist gJoyPadHist[64][MAXCONTROLLERS];
extern PadState gJoyPadState[MAXCONTROLLERS];
extern s32 gRandSeed; /* the random seed */
extern u32 gGameFrameCount; /* the frame count */
extern s32 gGameWorkKills;


#define DEMO(obj) ((DemoWork *) ((u32) (obj) + sizeof(Obj)))

/* Starts the controllers; a controller that answers is connected. */
void joyInitControllers(void) {
    OSMesgQueue q;
    OSMesg msg;
    s16 i;
    u8 bits;

    osCreateMesgQueue(&q, &msg, 1);
    osSetEventMesg(OS_EVENT_SI, &q, (OSMesg) 1);
    osContInit(&q, &bits, gJoyContStatus);
    osCreateMesgQueue(&gSiMesgQueue, &gJoyMesgBuf, 1);
    osSetEventMesg(OS_EVENT_SI, &gSiMesgQueue, (OSMesg) 0);
    for (i = 0; i < MAXCONTROLLERS; i++) {
        if ((bits & (1 << i)) && !(gJoyContStatus[i].errno & CONT_NO_RESPONSE_ERROR)) {
            gJoyPadState[i].flags = 1;
        } else {
            gJoyPadState[i].flags = 0;
        }
    }
}

/* Reads the controllers into the next frame of the history. */
void joyReadControllers(void) {
    s16 i;

    osContStartReadData(&gSiMesgQueue);
    osRecvMesg(&gSiMesgQueue, NULL, OS_MESG_BLOCK);
    osContGetReadData(gJoyContPads);
    gJoyHistNewest++;
    gJoyHistNewest &= 0x3F;
    for (i = 0; i < MAXCONTROLLERS; i++) {
        gJoyPadHist[gJoyHistNewest][i].button = gJoyContPads[i].button;
        gJoyPadHist[gJoyHistNewest][i].x = gJoyContPads[i].stick_x;
        gJoyPadHist[gJoyHistNewest][i].y = gJoyContPads[i].stick_y;
    }
}

/* The stick as D-pad presses (past 40 either way), then the hold counters. */
void joyUpdateHeld(void) {
    u16 b;
    s8 x;
    s8 y;
    s32 i;

    for (i = 0; i < MAXCONTROLLERS; i++) {
        b = gJoyPadHist[gJoyHistNewest][i].button;
        x = gJoyPadHist[gJoyHistNewest][i].x;
        y = gJoyPadHist[gJoyHistNewest][i].y;
        if (gJoyPadState[i].flags & 2) {
            if (x < -40) {
                b |= L_JPAD;
            } else if (x > 40) {
                b |= R_JPAD;
            }
            if (y > 40) {
                b |= U_JPAD;
            } else if (y < -40) {
                b |= D_JPAD;
            }
        }
        gJoyPadHist[gJoyHistNewest][i].button = b;
        if (b & U_JPAD) {
            gJoyPadState[i].held[0]++;
        } else {
            gJoyPadState[i].held[0] = 0;
        }
        if (b & D_JPAD) {
            gJoyPadState[i].held[1]++;
        } else {
            gJoyPadState[i].held[1] = 0;
        }
        if (b & L_JPAD) {
            gJoyPadState[i].held[2]++;
        } else {
            gJoyPadState[i].held[2] = 0;
        }
        if (b & R_JPAD) {
            gJoyPadState[i].held[3]++;
        } else {
            gJoyPadState[i].held[3] = 0;
        }
        if (b & U_CBUTTONS) {
            gJoyPadState[i].held[4]++;
        } else {
            gJoyPadState[i].held[4] = 0;
        }
        if (b & D_CBUTTONS) {
            gJoyPadState[i].held[5]++;
        } else {
            gJoyPadState[i].held[5] = 0;
        }
        if (b & L_CBUTTONS) {
            gJoyPadState[i].held[6]++;
        } else {
            gJoyPadState[i].held[6] = 0;
        }
        if (b & R_CBUTTONS) {
            gJoyPadState[i].held[7]++;
        } else {
            gJoyPadState[i].held[7] = 0;
        }
        if (b & A_BUTTON) {
            gJoyPadState[i].held[8]++;
        } else {
            gJoyPadState[i].held[8] = 0;
        }
        if (b & B_BUTTON) {
            gJoyPadState[i].held[9]++;
        } else {
            gJoyPadState[i].held[9] = 0;
        }
        if (b & Z_TRIG) {
            gJoyPadState[i].held[10]++;
        } else {
            gJoyPadState[i].held[10] = 0;
        }
        if (b & L_TRIG) {
            gJoyPadState[i].held[11]++;
        } else {
            gJoyPadState[i].held[11] = 0;
        }
        if (b & R_TRIG) {
            gJoyPadState[i].held[12]++;
        } else {
            gJoyPadState[i].held[12] = 0;
        }
        if (b & START_BUTTON) {
            gJoyPadState[i].held[13]++;
        } else {
            gJoyPadState[i].held[13] = 0;
        }
    }
}

/* The joypad task's handler. Run: read, update the Controller Paks, then record or play back
 * a demo, then the counters. */
s32 joyProc(Obj *obj, s16 msg, void *arg) {
    s32 i;

    switch (msg) {
        case 2:
            gJoypadTask = obj;
            obj->name = "joypad";
            gJoyHistNewest = 0;
            heapMemClear(gJoyPadHist, sizeof(gJoyPadHist));
            heapMemClear(gJoyPadState, sizeof(gJoyPadState));
            joyInitControllers();
            contPakRumbleResume();
            contPakProbeAll();
            for (i = 0; i < MAXCONTROLLERS; i++) {
                contPakRumbleStop(i);
            }
            DEMO(obj)->mode = 0;
            break;
        case 1:
            gJoypadTask = NULL;
            break;
        case 0:
            joyReadControllers();
            for (i = 0; i < MAXCONTROLLERS; i++) {
                contPakRumbleUpdate(i);
            }
            if (DEMO(obj)->mode == 1) {
                joyDemoRecordFrame();
            } else if (DEMO(obj)->mode == 2) {
                joyDemoPlayFrame();
            }
            joyUpdateHeld();
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A controller's buttons, back frames ago. */
u16 joyGetButton(s32 port, s32 back) {
    return gJoyPadHist[(gJoyHistNewest - back) & 0x3F][port].button;
}

/* Its stick. */
s8 joyGetStickX(s32 port, s32 back) {
    return gJoyPadHist[(gJoyHistNewest - back) & 0x3F][port].x;
}

s8 joyGetStickY(s32 port, s32 back) {
    return gJoyPadHist[(gJoyHistNewest - back) & 0x3F][port].y;
}

PadHist *joyGetPadHist(s32 port, s32 back) {
    return &gJoyPadHist[(gJoyHistNewest - back) & 0x3F][port];
}

PadState *joyGetPadState(s32 port) {
    return &gJoyPadState[port];
}

/* Makes a controller's stick work the D-pad, or stop. */
void joySetStickDpad(s32 port, s32 on) {
    if (on) {
        gJoyPadState[port].flags |= 2;
        return;
    }
    gJoyPadState[port].flags &= ~2;
}

/* The longest any of the buttons in mask has been held. */
u16 joyGetHeldTime(s32 port, u16 mask) {
    u16 n = 0;

    if ((mask & U_JPAD) && n < gJoyPadState[port].held[0]) {
        n = gJoyPadState[port].held[0];
    }
    if ((mask & D_JPAD) && n < gJoyPadState[port].held[1]) {
        n = gJoyPadState[port].held[1];
    }
    if ((mask & L_JPAD) && n < gJoyPadState[port].held[2]) {
        n = gJoyPadState[port].held[2];
    }
    if ((mask & R_JPAD) && n < gJoyPadState[port].held[3]) {
        n = gJoyPadState[port].held[3];
    }
    if ((mask & U_CBUTTONS) && n < gJoyPadState[port].held[4]) {
        n = gJoyPadState[port].held[4];
    }
    if ((mask & D_CBUTTONS) && n < gJoyPadState[port].held[5]) {
        n = gJoyPadState[port].held[5];
    }
    if ((mask & L_CBUTTONS) && n < gJoyPadState[port].held[6]) {
        n = gJoyPadState[port].held[6];
    }
    if ((mask & R_CBUTTONS) && n < gJoyPadState[port].held[7]) {
        n = gJoyPadState[port].held[7];
    }
    if ((mask & A_BUTTON) && n < gJoyPadState[port].held[8]) {
        n = gJoyPadState[port].held[8];
    }
    if ((mask & B_BUTTON) && n < gJoyPadState[port].held[9]) {
        n = gJoyPadState[port].held[9];
    }
    if ((mask & Z_TRIG) && n < gJoyPadState[port].held[10]) {
        n = gJoyPadState[port].held[10];
    }
    if ((mask & L_TRIG) && n < gJoyPadState[port].held[11]) {
        n = gJoyPadState[port].held[11];
    }
    if ((mask & R_TRIG) && n < gJoyPadState[port].held[12]) {
        n = gJoyPadState[port].held[12];
    }
    if ((mask & START_BUTTON) && n < gJoyPadState[port].held[13]) {
        n = gJoyPadState[port].held[13];
    }
    return n;
}

/* Stops recording or playing back. */
void joyDemoEnd(void) {
    DemoWork *w = DEMO(gJoypadTask);

    w->mode = 0;
    PRINTF("\nDEMO_KEY_END\n");
}

s8 joyDemoGetMode(void) {
    DemoWork *w = DEMO(gJoypadTask);

    return w->mode;
}

/* Records a frame: each player's keys go to the debugger. */
void joyDemoRecordFrame(void) {
    DemoWork *w = DEMO(gJoypadTask);
    PadHist *h;
    s32 i;

    for (i = 0; i < w->nPlayers; i++) {
        h = joyGetPadHist(i, 0);
        PRINTF("\nDEMO_KEY_DATA %d %d %d %d  ", i, h->button, h->x, h->y);
    }
}

/* Plays a frame back: each player's pad from the current run. The first run of 0 frames ends
 * the demo. */
void joyDemoPlayFrame(void) {
    DemoWork *w = DEMO(gJoypadTask);
    DemoKey *k;
    s32 i;

    for (i = 0; i < w->nPlayers; i++) {
        if (w->player[i].keys != NULL) {
            k = &w->player[i].keys[w->player[i].run];
            if (k->frames == 0) {
                w->mode = 0;
                return;
            }
            *joyGetPadHist(i, 0) = k->pad;
            w->player[i].frame++;
            if (w->player[i].frame >= k->frames) {
                w->player[i].frame = 0;
                w->player[i].run++;
            }
        }
    }
}

/* A demo's start: the random seed, the frame count and the players' pads reset. */
void joyDemoReset(void) {
    DemoWork *w;
    PadHist *h;
    s32 i;

    gRandSeed = 0x1234;
    w = DEMO(gJoypadTask);
    gGameFrameCount = 0;
    gGameWorkKills = 0;
    for (i = 0; i < w->nPlayers; i++) {
        h = joyGetPadHist(i, 0);
        h->button = 0;
        h->x = 0;
        h->y = 0;
    }
    joyUpdateHeld();
}

/* Starts recording n players. */
void joyDemoStartRecord(s32 n) {
    DemoWork *w = DEMO(gJoypadTask);

    w->mode = 1;
    w->nPlayers = n;
    PRINTF("\nDEMO_KEY_START %d\n", n);
    joyDemoReset();
}

/* Starts playing back a demo: data[0] is the player count, data[1-4] each player's keys
 * (segmented addresses in file, if file is not 0). */
void joyDemoStartPlay(u32 *data, u32 file) {
    DemoWork *w = DEMO(gJoypadTask);
    s32 i;

    if (file != 0) {
        data = (u32 *) fadeSegToRam((u32) data, file);
    }
    w->nPlayers = data[0];
    for (i = 0; i < 4; i++) {
        w->player[i].frame = 0;
        w->player[i].run = 0;
        if (file != 0) {
            if (data[i + 1] == 0) {
                w->player[i].keys = NULL;
            } else {
                w->player[i].keys = (DemoKey *) fadeSegToRam(data[i + 1], file);
            }
        }
    }
    w->mode = 2;
    joyDemoReset();
}
