#include "common.h"
#include "object.h"
#include "game.h"
#include "file.h"
#include "audio.h"
#include "contpak.h"
#include "fade.h"
#include "fileres.h"
#include "gameproc.h"
#include "gfxbuf.h"
#include "gfxmgr.h"
#include "heap.h"
#include "joypad.h"
#include "logo.h"
#include "pakfiler.h"
#include "stage.h"
#include "tmanager.h"

/* The game's root task, "game_proc" (main.c makes it at boot, and stage.c's stageGotoGameProc sends
 * the game back to it). It runs the boot sequence, then every scene change:
 * - state 0-1: sound and file 1 (the list gGameStartFileList), then the Controller Pak check;
 * - 2-4: the Controller Pak manager (pakfiler.c, overlay 8), opened at boot when B is held, or
 *   asked for by a scene change ("pak");
 * - 5-6: the Konami logo (logo.c, overlay 8), until it is done; then the fill goes white;
 * - 7: the logo is deleted, the clock set (time 0x50000, a day of 300 s) and the first scene
 *   chosen (stageGotoTitle);
 * - 8: each scene change. gGameWork.next names the next task's handler; this waits for the
 *   file loads, resets the display and the fade, clears the request and makes the task.
 *   game_proc itself as the next handler restarts the logo; "pak" opens the manager.
 * - 9: waits for a button, then back to 8. */

typedef struct GameProcWork {
    /* 0x0 */ u8 state;
    /* 0x4 */ Obj *logo;   /* the logo's task */
    /* 0x8 */ s32 unused8; /* cleared at the scene change, never read */
    /* 0xC */ Obj *pak;    /* the Controller Pak manager's task */
} GameProcWork;

/* gameproc.c's .data (0x80107780-0x80107788), in ROM order. */
extern u8 gAsciiFont[]; /* the RAM of file 2 */
FileList gGameStartFileList[] = { /* file 2, loaded at the start */
    { FILELIST_LAST | 1, gAsciiFont },
};


/* Case 7's colour, "[2] = [3] = 0xFF; [1] = [2];", reads two bytes back just after storing
 * them, as the game's code does. Case 6 is three statements. The work pointer is set at its
 * declaration (the game keeps it in a stack slot). The clear of unk58 needs s32 i and the loop
 * on ONE line (else it stores [2], [3], [4] where the game stores [4], [3], [2]). */
s32 game_proc(Obj *obj, s16 msg, void *arg) {
    GameProcWork *work = (GameProcWork *) ((u32) obj + sizeof(Obj));
    s32 i;
    Obj *fade;
    Obj *parent;
    void *next;
    s32 n;
    u16 file;
    s32 x;

    switch (msg) {
        case 2:
            obj->name = "game_proc";
            gGameWork.gameProcTask = obj;
            break;
        case 0:
        switch (work->state) {
            case 0:
                sndInit();
                work->state = 1;
                break;
            case 1:
                fileQueueList(gGameStartFileList);
                fileLoadOverlay(8);
                work->state = 3;
                break;
            case 2:
                fileLoadOverlay(8);
                work->pak = tskCreate(obj, pakFiler_proc, 0, NULL);
                work->state = 4;
                break;
            case 3:
                if (joyGetButton(0, 0) & 0x1000) {
                    work->pak = tskCreate(obj, pakFiler_proc, 0, NULL);
                    work->state = 4;
                } else {
                    work->state = 5;
                }
                break;
            case 4:
                if (pakFilerIsRunning(work->pak) == 0) {
                    work->state = 5;
                }
                break;
            case 5:
                work->logo = tskCreate(obj, logoProc, 0, NULL);
                work->state = 6;
                break;
            case 6:
                if (logoIsDone(work->logo) != 0) {
                    gGameState.fadeFrom.r = 0xFF;
                    gGameState.fadeFrom.g = 0xFF;
                    gGameState.fadeFrom.b = 0xFF;
                    work->state = 7;
                }
                break;
            case 7:
                work->logo = tskDestroy(work->logo);
                gGameWork.time = 0x50000;
                gGameWork.daySeconds = 300;
                gGameWork.fadeColor[1] = gGameWork.fadeColor[2] = 0xFF;
                gGameWork.fadeColor[0] = gGameWork.fadeColor[1]; /* FAKE: a second read-back */
                stageGotoTitle();
                work->state = 8;
                break;
            case 8:
                if (gGameWork.next == NULL) {
                    break;
                }
                parent = obj;
                if (gGameWork.nextParent != NULL) {
                    parent = gGameWork.nextParent;
                }
                while (gfxGetPendingSwaps() != 0) {
                }
                if (gGameWork.flags & 1) {
                    resModesOn();
                } else {
                    resModesOff();
                }
                tskDestroyChildren(parent);
                frmGraphicManagerRestart();
                work->unused8 = 0;
                resFreeAllBut(1);
                resFreeTop();
                if (!(gGameWork.flags & 1)) {
                    resFreeKept();
                }
                if (!(gGameWork.flags & 2)) {
                    for (i = 0; i < 5; i++) { gGameWork.tunes[i] = 0; }
                }
                resModesOff();
                gGameState.scissor.ulx = 0;
                gGameState.scissor.uly = 0;
                gGameState.scissor.lrx = 0;
                gGameState.scissor.lry = 0;
                gGameState.fade = 0;
                gGameState.fadeFrom.r = 0;
                gGameState.fadeFrom.g = 0;
                gGameState.fadeFrom.b = 0;
                gGameState.skipDraw = 0;
                if (gGameWork.next == (void *) game_proc) {
                    work->state = 5;
                } else if (gGameWork.next == (void *) 0x70616B) { /* "pak" */
                    work->state = 2;
                } else {
                    fade = fadeCreate(obj);
                    tskDestroy(fade);
                    fadeCreate(obj);
                    fadeSetLevel(255.0f);
                    fadeSetSpeed(8.0f);
                    fadeSetColor(gGameWork.fadeColor[0], gGameWork.fadeColor[1],
                                           gGameWork.fadeColor[2]);
                    fadeSetTarget(0.0f);
                    if (gGameWork.nextOverlay != 0) {
                        fileLoadOverlay(gGameWork.nextOverlay);
                    }
                    if (gGameWork.nextFile != 0) {
                        fileLoadNow(gGameWork.nextFile, NULL);
                    }
                    file = gGameWork.nextFile;
                    next = gGameWork.next;
                    n = gGameWork.nextBlocks;
                    x = gGameWork.nextArg;
                    heapMemClear(&gGameWork.flags, 0x1C);
                    tskCreateBlocks(parent, next, n, file, (void *) x);
                    contPakRumbleResume();
                }
                break;
            case 9:
                if (joyGetHeldTime(3, 0x1000) == 1) {
                    gGameState.skipDraw = 0;
                    work->state = 8;
                    sndPlay(4);
                    break;
                }
                return 0;
        }
        break;
    }
    return tskDefaultProc(obj, msg, arg);
}
