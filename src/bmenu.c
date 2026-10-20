#include "common.h"
#include "object.h"
#include "game.h"
#include "ascii.h"
#include "audio.h"
#include "bmenu.h"
#include "cockpit.h"
#include "gfxmgr.h"
#include "joypad.h"
#include "stubs.h"
#include "tmanager.h"

/* "Bmenu" (overlay 10): the debug BOSS SELECT menu, the boss battles' twin of dmenu.c. It lists
 * five bosses; the D-pad moves a "$" cursor (held, it repeats every other frame from the 15th),
 * A or Start starts the chosen boss's battle (the "cockpit" task, given the boss's number from
 * 1), and B closes the menu. */

#define BMENU(task) ((BMenu *) ((u32) (task) + sizeof(Obj)))

Obj *gBmenuTask = NULL; /* the menu, NULL when there is none */
extern Frame *gFrmModelLayer;
extern s16 gFrmKindCore; /* the "null" kind (roots) */



/* The menu's handler: one menu at a time, in a 16-8-304-232 scissor. */
s32 Bmenu(Obj *obj, s16 msg, void *arg) {
    BMenu *w = BMENU(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            if (gBmenuTask != NULL) {
                return -1;
            }
            gBmenuTask = obj;
            obj->name = "Bmenu";
            audioNop();
            gGameState.retracesPerFrame = 1;
            gGameState.scissor.ulx = 16;
            gGameState.scissor.uly = 8;
            gGameState.scissor.lrx = 304;
            gGameState.scissor.lry = 232;
            w->title = frmCreate(gFrmModelLayer, gFrmKindCore);
            w->list = frmCreate(gFrmModelLayer, gFrmKindCore);
            w->proc = bmenuProcSelect;
            w->state = 0;
            break;
        case 1:
            gGameState.retracesPerFrame = 1;
            gGameState.scissor.ulx = 0;
            gGameState.scissor.uly = 0;
            gGameState.scissor.lrx = 0;
            gGameState.scissor.lry = 0;
            frmDestroy(w->title);
            frmDestroy(w->list);
            gBmenuTask = NULL;
            break;
        case 0:
            if (w->proc(obj, w) == -1) {
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The menu's proc once a boss is chosen: nothing. */
s32 bmenuProcChosen(Obj *obj, BMenu *w) {
    return 0;
}

/* The menu's proc: lists the bosses, then moves the cursor until one is chosen. */
s32 bmenuProcSelect(Obj *obj, BMenu *w) {
    s32 pad;
    s32 boss;

    joyGetButton(0, 0);
    switch (w->state) {
        case 0:
            joySetStickDpad(0, 1);
            frmDestroyChildren(w->title);
            frmDestroyChildren(w->list);
            asciiNew(w->title, 126, 56, "BOSS SELECT");
            asciiNew(w->list, 138, 84, "BISUMARU");
            asciiNew(w->list, 138, 100, "TAISAMBA");
            asciiNew(w->list, 138, 116, "FURAIJIN");
            asciiNew(w->list, 138, 132, "KABUKI-F");
            asciiNew(w->list, 138, 148, "DEVIL-SG");
            w->cursor = asciiNew(w->list->child, 0, 0, "$");
            w->cursor->x = ((AsciiFrame *) w->cursor->f.parent)->x - 16;
            w->cursor->y = ((AsciiFrame *) w->cursor->f.parent)->y;
            w->state = 1;
            break;
        case 1:
            if (joyGetHeldTime(0, U_JPAD) == 1 ||
                (joyGetHeldTime(0, U_JPAD) >= 15 && joyGetHeldTime(0, U_JPAD) % 2 == 1)) {
                if (w->cursor->f.parent->prev != NULL) {
                    frmMove(&w->cursor->f, w->cursor->f.parent->prev);
                } else {
                    frmMove(&w->cursor->f, frmGetLastChild(w->cursor->f.parent->parent));
                }
            } else if (joyGetHeldTime(0, D_JPAD) == 1 ||
                       (joyGetHeldTime(0, D_JPAD) >= 15 && joyGetHeldTime(0, D_JPAD) % 2 == 1)) {
                if (w->cursor->f.parent->next != NULL) {
                    frmMove(&w->cursor->f, w->cursor->f.parent->next);
                } else {
                    frmMove(&w->cursor->f, w->cursor->f.parent->parent->child);
                }
            }
            if (joyGetHeldTime(0, B_BUTTON) == 1) {
                stubNop(obj->parent);
                return -1;
            }
            w->cursor->x = ((AsciiFrame *) w->cursor->f.parent)->x - 16;
            w->cursor->y = ((AsciiFrame *) w->cursor->f.parent)->y;
            if (joyGetHeldTime(0, A_BUTTON | START_BUTTON) == 1) {
                boss = frmGetIndex(w->cursor->f.parent);
                joySetStickDpad(0, 0);
                w->proc = bmenuProcChosen;
                frmDestroyChildren(w->title);
                frmDestroyChildren(w->list);
                tskCreate(obj, cockpitProc, 0, (void *) (boss + 1));
            }
            break;
    }
    return 0;
}
