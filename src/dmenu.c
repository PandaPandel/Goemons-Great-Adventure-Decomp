#include "common.h"
#include "object.h"
#include "game.h"
#include "ascii.h"
#include "audio.h"
#include "contpak.h"
#include "dmenu.h"
#include "file.h"
#include "fileload.h"
#include "gauge.h"
#include "gfxmgr.h"
#include "joypad.h"
#include "stage.h"
#include "tlb/tlb_0019.h"
#include "tlb/tlb_0020.h"
#include "tlb/tlb_0022.h"
#include "tlb/tlb_0023.h"
#include "tlb/tlb_0024.h"
#include "tlb/tlb_0025.h"
#include "tlb/tlb_0026.h"
#include "tlb/tlb_0027.h"
#include "tlb/tlb_0028.h"
#include "tlb/tlb_0029.h"
#include "tlb/tlb_0030.h"
#include "tlb/tlb_0031.h"
#include "tlb/tlb_0032.h"
#include "tlb/tlb_0033.h"
#include "tmanager.h"

/* "Dmenu" (overlay 9): the debug DEMO SELECT menu. It lists sixteen demos; the D-pad moves a "$"
 * cursor down the list (held, it repeats every other frame from the 15th), and A, B or Start
 * loads the chosen demo's file and starts its task. The demos' tasks live in their files, mapped
 * at 0x08000000 through the TLB. */


#define DMENU(obj) ((DMenu *) ((u32) (obj) + sizeof(Obj)))

/* dmenu.c's .data (0x801B45C0-0x801B45C8, then IDO's 16-byte alignment), in address order. */
Obj *gDmenuObj = NULL; /* the menu, NULL when there is none */
Obj *gDmenuLastObj = NULL;
extern s8 gDemoCamHold;
extern s32 gDemoForcedFade;
extern Frame *gFrmModelLayer;
extern s16 gFrmKindCore; /* the "null" kind (roots) */
extern s8 gGameStateSkipDraw; /* GameState's unk22 */
extern s8 gRetracesPerFrame; /* GameState's retracesPerFrame */




/* The menu's handler: one menu at a time, in a 16-8-304-232 scissor. */
s32 Dmenu(Obj *obj, s16 msg, void *arg) {
    DMenu *w = DMENU(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            if (gDmenuObj != NULL) {
                return -1;
            }
            gDmenuObj = obj;
            gDmenuLastObj = obj;
            obj->name = "Dmenu";
            gDemoForcedFade = 0;
            (*audioNop)();
            gGameState.retracesPerFrame = 1;
            gGameState.scissor.ulx = 16;
            gGameState.scissor.uly = 8;
            gGameState.scissor.lrx = 304;
            gGameState.scissor.lry = 232;
            w->title = (*frmCreate)(gFrmModelLayer, gFrmKindCore);
            w->list = (*frmCreate)(gFrmModelLayer, gFrmKindCore);
            w->proc = dmenuProcSelect;
            w->state = 0;
            floadLoadSet(0xED);
            break;
        case 1:
            gGameState.retracesPerFrame = 1;
            gGameState.scissor.ulx = 0;
            gGameState.scissor.uly = 0;
            gGameState.scissor.lrx = 0;
            gGameState.scissor.lry = 0;
            (*frmDestroy)(w->title);
            (*frmDestroy)(w->list);
            gDmenuObj = NULL;
            break;
        case 0:
            if (w->proc(obj, w) == -1) {
                return -1;
            }
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* The menu's proc once a demo is chosen: nothing. */
s32 dmenuProcDone(Obj *obj, DMenu *w) {
    return 0;
}

/* The menu's proc: lists the demos, then moves the cursor until one is chosen.
 * Case 0 reads the cursor's line through `cursor`, a copy of the reloaded w->cursor, so the
 * line pointer is not one compiler temp shared with case 1. */
s32 dmenuProcSelect(Obj *obj, DMenu *w) {
    AsciiFrame *cursor;
    s32 demo;
    s32 pad[2];

    (*joyGetButton)(0, 0);
    switch (w->state) {
        case 0:
            (*joySetStickDpad)(0, 1);
            gDemoForcedFade = 0;
            (*frmDestroyChildren)(w->title);
            (*frmDestroyChildren)(w->list);
            (*asciiNew)(w->title, 126, 16, "DEMO SELECT");
            (*asciiNew)(w->list, 88, 34, "A1 SEKISYO(OK)");
            (*asciiNew)(w->list, 88, 44, "A1 CHAMISE-SAS(OK)");
            (*asciiNew)(w->list, 88, 54, "A1 OMITSU-IMPACT(OK)");
            (*asciiNew)(w->list, 88, 64, "A1 OMITSU-IMPACT B(OK)");
            (*asciiNew)(w->list, 88, 74, "A1 CLEAR(OK)");
            (*asciiNew)(w->list, 88, 84, "A2 CHAMISE-YAE(OK)");
            (*asciiNew)(w->list, 88, 94, "A2 CLEAR(OK)");
            (*asciiNew)(w->list, 88, 104, "A3 CHAMISE-JIJI(OK)");
            (*asciiNew)(w->list, 88, 114, "A3 DOUCHUKI(OK)");
            (*asciiNew)(w->list, 88, 124, "A3 CLEAR(OK)");
            (*asciiNew)(w->list, 88, 134, "A4 HAKAI(OK)");
            (*asciiNew)(w->list, 88, 144, "A4 CLEAR");
            (*asciiNew)(w->list, 88, 154, "A5 DOUCHUKI2");
            (*asciiNew)(w->list, 88, 164, "A5 CLEAR");
            (*asciiNew)(w->list, 88, 174, "---- UTADEMO ----");
            (*asciiNew)(w->list, 88, 184, "---- UTADEMOMOJI ----");
            w->cursor = (*asciiNew)(w->list->child, 0, 0, "$");
            w->cursor->x = ((AsciiFrame *) w->cursor->f.parent)->x - 16;
            cursor = w->cursor;
            w->cursor->y = ((AsciiFrame *) cursor->f.parent)->y;
            w->state = 1;
            break;
        case 1:
            if ((*joyGetHeldTime)(0, U_JPAD) == 1 ||
                ((*joyGetHeldTime)(0, U_JPAD) >= 15 && (*joyGetHeldTime)(0, U_JPAD) % 2 == 1)) {
                if (w->cursor->f.parent->prev != NULL) {
                    (*frmMove)(&w->cursor->f, w->cursor->f.parent->prev);
                } else {
                    (*frmMove)(&w->cursor->f,
                                              (*frmGetLastChild)(w->cursor->f.parent->parent));
                }
            } else if ((*joyGetHeldTime)(0, D_JPAD) == 1 ||
                       ((*joyGetHeldTime)(0, D_JPAD) >= 15 &&
                        (*joyGetHeldTime)(0, D_JPAD) % 2 == 1)) {
                if (w->cursor->f.parent->next != NULL) {
                    (*frmMove)(&w->cursor->f, w->cursor->f.parent->next);
                } else {
                    (*frmMove)(&w->cursor->f, w->cursor->f.parent->parent->child);
                }
            }
            w->cursor->x = ((AsciiFrame *) w->cursor->f.parent)->x - 16;
            w->cursor->y = ((AsciiFrame *) w->cursor->f.parent)->y;
            if ((*joyGetHeldTime)(0, A_BUTTON | B_BUTTON | START_BUTTON) == 1) {
                demo = (*frmGetIndex)(w->cursor->f.parent);
                (*joySetStickDpad)(0, 0);
                w->proc = dmenuProcDone;
                (*frmDestroyChildren)(w->title);
                (*frmDestroyChildren)(w->list);
                switch (demo) {
                    case 0:
                        gDemoCamHold = 0;
                        (*fileLoadNow)(0x13, w->ram);
                        (*tskCreate)(obj, gdemoProc, 0x13, NULL);
                        break;
                    case 1:
                        gDemoCamHold = 0;
                        (*fileLoadNow)(0x14, w->ram);
                        (*tskCreate)(obj, chademoProc, 0x14, NULL);
                        break;
                    case 2:
                        gDemoCamHold = 0;
                        (*fileLoadNow)(0x17, w->ram);
                        (*tskCreate)(obj, omiDemoProc, 0x17, NULL);
                        break;
                    case 3:
                        gDemoCamHold = 0;
                        (*fileLoadNow)(0x17, w->ram);
                        (*tskCreate)(obj, omiDemoShortProc, 0x17, NULL);
                        break;
                    case 4:
                        gDemoCamHold = 0;
                        (*fileLoadNow)(0x18, w->ram);
                        (*tskCreate)(obj, a1clearProc, 0x18, NULL);
                        break;
                    case 5:
                        gDemoCamHold = 0;
                        (*fileLoadNow)(0x19, w->ram);
                        (*tskCreate)(obj, yaeDemoProc, 0x19, NULL);
                        break;
                    case 6:
                        gDemoCamHold = 0;
                        (*fileLoadNow)(0x1A, w->ram);
                        (*tskCreate)(obj, st2clearProc, 0x1A, NULL);
                        break;
                    case 7:
                        gDemoCamHold = 0;
                        (*fileLoadNow)(0x16, w->ram);
                        (*tskCreate)(obj, jijiDemoProc, 0x16, NULL);
                        break;
                    case 8:
                        gDemoCamHold = 0;
                        (*fileLoadNow)(0x1B, w->ram);
                        (*tskCreate)(obj, dochukiDemoProc, 0x1B, NULL);
                        break;
                    case 9:
                        gDemoCamHold = 0;
                        (*fileLoadNow)(0x1C, w->ram);
                        (*tskCreate)(obj, st3clearProc, 0x1C, NULL);
                        break;
                    case 10:
                        gDemoCamHold = 0;
                        (*fileLoadNow)(0x1D, w->ram);
                        (*tskCreate)(obj, mach2DemoProc, 0x1D, NULL);
                        break;
                    case 11:
                        gDemoCamHold = 0;
                        (*fileLoadNow)(0x1E, w->ram);
                        (*tskCreate)(obj, st4clearProc, 0x1E, NULL);
                        break;
                    case 12:
                        gDemoCamHold = 0;
                        (*fileLoadNow)(0x1F, w->ram);
                        (*tskCreate)(obj, demo22Proc, 0x1F, NULL);
                        break;
                    case 13:
                        gDemoCamHold = 0;
                        (*fileLoadNow)(0x20, w->ram);
                        (*tskCreate)(obj, clr5DemoProc, 0x20, NULL);
                        break;
                    case 14:
                        gDemoCamHold = 0;
                        (*fileLoadNow)(0x21, w->ram);
                        (*tskCreate)(obj, utaDemoProc, 0x21, NULL);
                        break;
                    case 15:
                        gDemoCamHold = 0;
                        (*fileLoadNow)(0x21, w->ram);
                        (*tskCreate)(obj, utaSongProc, 0x21, NULL);
                        break;
                }
            }
            break;
    }
    return 0;
}

/* Queue demo 1 (A1 CHAMISE-SAS). */
void dmenuQueueDemo1(void) {
    dmenuQueueDemo(1);
}

/* Queue demo 2 (A1 SEKISYO). */
void dmenuQueueDemo2(void) {
    dmenuQueueDemo(2);
}

/* Go to stage 0x9A. */
void dmenuGoStage9A(void) {
    gGameWork.nextOverlay = 9;
    stageGoto(gGameWork.gameProcTask, 0x9A);
}

/* Go to stage 0xB6. */
void dmenuGoStageB6(void) {
    gGameWork.nextOverlay = 9;
    stageGoto(gGameWork.gameProcTask, 0xB6);
}

/* Go to stage 0xC9. */
void dmenuGoStageC9(void) {
    gGameWork.nextOverlay = 9;
    stageGoto(gGameWork.gameProcTask, 0xC9);
}

/* Queues demo n (1 to 14) as the next task: its file and its handler there. */
void dmenuQueueDemo(s32 demo) {
    contPakRumbleSuspend();
    gDemoForcedFade = 0;
    gRetracesPerFrame = 1;
    gaugeSetOn(0);
    gGameStateSkipDraw = 2;
    gGameWork.nextParent = gGameWork.gameProcTask;
    switch (demo) {
        case 1:
            gGameWork.next = chademoProc;
            gGameWork.nextFile = 0x14;
            break;
        case 2:
            gGameWork.next = gdemoProc;
            gGameWork.nextFile = 0x13;
            break;
        case 3:
            gGameWork.next = omiDemoProc;
            gGameWork.nextFile = 0x17;
            break;
        case 4:
            gGameWork.next = omiDemoShortProc;
            gGameWork.nextFile = 0x17;
            break;
        case 5:
            gGameWork.next = a1clearProc;
            gGameWork.nextFile = 0x18;
            break;
        case 6:
            gGameWork.next = yaeDemoProc;
            gGameWork.nextFile = 0x19;
            break;
        case 7:
            gGameWork.next = st2clearProc;
            gGameWork.nextFile = 0x1A;
            break;
        case 8:
            gGameWork.next = jijiDemoProc;
            gGameWork.nextFile = 0x16;
            break;
        case 9:
            gGameWork.next = dochukiDemoProc;
            gGameWork.nextFile = 0x1B;
            break;
        case 10:
            gGameWork.next = st3clearProc;
            gGameWork.nextFile = 0x1C;
            break;
        case 11:
            gGameWork.next = mach2DemoProc;
            gGameWork.nextFile = 0x1D;
            break;
        case 12:
            gGameWork.next = st4clearProc;
            gGameWork.nextFile = 0x1E;
            break;
        case 13:
            gGameWork.next = demo22Proc;
            gGameWork.nextFile = 0x1F;
            break;
        case 14:
            gGameWork.next = clr5DemoProc;
            gGameWork.nextFile = 0x20;
            break;
    }
    gGameWork.nextOverlay = 9;
}
