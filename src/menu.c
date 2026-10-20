#include "object.h"
#include "audio.h"
#include "file.h"
#include "gfxmgr.h"
#include "menu.h"
#include "msgfrm.h"
#include "scenario.h"
#include "string_c.h"
#include "tmanager.h"

/* The scenario menus: choices a player moves through and takes ("scMenu").
 * - The menu base ("menuBaseRun", one under the runner) holds the menus. A menu has an id
 *   (variable 0x08000014) and a player (variable 0x08000000).
 * - A menu is the commands after the one that opened it, in pairs up to an op 18: the entry's
 *   text, then where the entry jumps. Each entry ("scEntry") sits where its text begins.
 * - Up or left and down or right move the cursor (wrapping round), with sound 0x203. A takes
 *   the entry (sound 0x205 if variable 0x080000E8 is set), and B takes it back.
 * - Each player's choice is kept in gMenuChoices and copied to variable 0x08000064. */

/* A menu's work. */
typedef struct MenuWork {
    /* 0x00 */ u8 id;
    /* 0x04 */ ScCmd *cmd; /* the command it reads; once taken, the entry's jump */
    /* 0x08 */ Obj *cursor; /* the entry the cursor is on */
    /* 0x0C */ s32 player;
    /* 0x10 */ s32 choice; /* the cursor's entry, from 0 */
    /* 0x14 */ u8 flags; /* 1: taken */
} MenuWork;

/* The cursor's frame (the "cursor" kind, msgfrm.c). */
typedef struct CursorFrame {
    /* 0x00 */ Frame frame;
    /* 0x14 */ s16 x;
    /* 0x16 */ s16 y;
    /* 0x18 */ u8 blink;
} CursorFrame;

/* An entry's work (also what it is made with). */
typedef struct EntryWork {
    /* 0x00 */ Frame *frame; /* the runner's third frame */
    /* 0x04 */ u8 flags; /* 1: the cursor is on it; 2: made after the first; 4: taken */
    /* 0x06 */ s16 x;
    /* 0x08 */ s16 y;
    /* 0x0C */ void *jump; /* where it goes in the scenario */
    /* 0x10 */ CursorFrame *cursor;
    /* 0x14 */ u8 timer;
    /* 0x15 */ u8 blink;
    /* 0x16 */ u8 state; /* 1: the cursor is shown */
} EntryWork; /* size 0x18 */

extern Obj *gScRunner; /* the scenario runner */
extern u16 gScenarioFiles[]; /* the scenario files */
extern s32 *gScVarPlayer; /* the script variables (scenario.c) */
extern s32 *gScVarMenuId;
extern s32 *gScVarChoice;
extern s32 *gScVarInMenu1;
extern s32 *gScVarInMenu2;
extern s32 *gScVarUiSeMode;
extern s32 gMenuChoices[]; /* each player's choice */


#define MENU(obj) ((MenuWork *) ((u32) (obj) + sizeof(Obj)))
#define ENTRY(obj) ((EntryWork *) ((u32) (obj) + sizeof(Obj)))

/* The menu base's handler. */
s32 menuBaseRun_proc(Obj *obj, s16 msg, void *arg) {
    switch (msg) {
        case 0:
        case 1:
            break;
        case 2:
            obj->name = "menuBaseRun";
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *mbCreate(Obj *parent) {
    Obj *base = tskCreate(parent, menuBaseRun_proc, 0, NULL);

    if (base == NULL) {
        PRINTF("mbCreate: Warning menuBaseTask is NULL\n");
    }
    return base;
}

void mbDelete(Obj *base) {
    tskDestroy(base);
}

/* Adds a menu, unless one with the id (variable 0x08000014) is there. */
void mbMenuAdd(Obj *base) {
    Obj *m;

    for (m = base->child; m != NULL; m = m->next) {
        if (mbMenuIdCheck(m, *gScVarMenuId) == TRUE) {
            PRINTF("mbMenuAdd: Warning: 既にメニューがあります。\n");
            return;
        }
    }
    if (menuCreate(base) == NULL) {
        PRINTF("mbMenuAdd: Warning: menu 追加できません\n");
    }
}

/* Deletes the taken menus, last first. */
void mbMenuDelete(Obj *base) {
    Obj *m = base->child;

    while (m->next != NULL) {
        m = m->next;
    }
    for (; m != NULL; m = m->prev) {
        if (mbMenuIsDecided(m) == TRUE) {
            mbMenuTaskDelete(m);
        } else {
            PRINTF("mbMenuDelete: Warning: まだ決定していません。\n");
        }
    }
}

/* Deletes the menu with an id. */
void mbAMenuDelete(Obj *base, u8 id) {
    Obj *m;

    for (m = base->child; m != NULL; m = m->next) {
        if (mbMenuIdCheck(m, id) == TRUE) {
            mbMenuTaskDelete(m);
            return;
        }
    }
    PRINTF("mbAMenuDelete: 該当なし\n");
}

/* Whether there are menus. */
s32 mbMenuExistCheck(Obj *base) {
    if (base->child != NULL) {
        return TRUE;
    }
    return FALSE;
}

/* The command pointer of the menu with an id. */
void *mbScPtrGet(Obj *base, u8 id) {
    Obj *m;

    for (m = base->child; m != NULL; m = m->next) {
        if (mbMenuIdCheck(m, id) == TRUE) {
            return mbMenuScPtrGet(m);
        }
    }
    PRINTF("mbScPtrGet: 該当なし\n");
    return NULL;
}

/* Whether any menu is taken. */
s32 mbMenuCheck(Obj *base) {
    Obj *m = base->child;

    if (m == NULL) {
        return FALSE;
    }
    for (; m != NULL; m = m->next) {
        if (mbMenuIsDecided(m) == TRUE) {
            PRINTF("mbMenuCheck: いずれかが決定している\n");
            return TRUE;
        }
    }
    return FALSE;
}

/* Copies each menu's choice to variable 0x08000064. Every case writes the same variable, as
 * in the game. Player 0 is the default's, which makes the switch a jump table. */
void mbChoiceToVar(Obj *base) {
    Obj *m = base->child;
    u32 player;

    if (m == NULL) {
        return;
    }
    for (; m != NULL; m = m->next) {
        player = mbMenuPlayerGet(m);
        switch (player) {
            case 1:
                *gScVarChoice = gMenuChoices[player];
                break;
            case 2:
                *gScVarChoice = gMenuChoices[player];
                break;
            case 3:
                *gScVarChoice = gMenuChoices[player];
                break;
            case 4:
                *gScVarChoice = gMenuChoices[player];
                break;
            case 0:
            default:
                *gScVarChoice = gMenuChoices[player];
                break;
        }
    }
}

/* Where the taken menu's entry jumps. */
void *mbScJumpPtrGet(Obj *base) {
    Obj *m = base->child;

    if (m == NULL) {
        return NULL;
    }
    for (; m != NULL; m = m->next) {
        if (mbMenuIsDecided(m) == TRUE) {
            return mbEntryJumpGet(mbMenuCursorGet(m));
        }
    }
    PRINTF("mbScJumpPtrGet: Warning: entryD なし\n");
    return NULL;
}

/* A menu's handler. (The locals in this order give the game's stack, no at 0x58 and pos at
 * 0x64; the text file is read in place, gScenarioFiles[no], which gives the entry s4 and the file
 * s5; the choice is stepped in its own statement before it is recorded.) */
s32 scMenu_proc(Obj *obj, s16 msg, void *arg) {
    MenuWork *w = MENU(obj);
    s32 pad[2];
    Obj *str;
    u16 *file; /* unused: it keeps its slot in the game's frame */
    Obj *e;
    StrPos pos;
    s16 *text;
    u8 i;
    s32 no;

    switch (msg) {
        case 2:
            obj->name = "scMenu";
            w->id = *gScVarMenuId;
            w->cmd = scGetPc(gScRunner);
            w->player = *gScVarPlayer;
            w->choice = 0;
            str = scStringTaskCheck2(gScRunner, *gScVarMenuId);
            no = scRunGetNo(gScRunner);
            mbChoiceReset(obj, w->player);
            mbPlayerInMenuOn(obj, w->player);
            scPadStickDpad(*gScVarPlayer, 1);
            i = 0;
            while (TRUE) {
                w->cmd++;
                if (w->cmd->op == 18) {
                    break;
                }
                pos = strGetPos(str);
                if (i == 0) {
                    e = w->cursor = entryCreate(obj, pos.x, pos.y, NULL, 1);
                } else {
                    e = entryCreate(obj, pos.x, pos.y, NULL, 2);
                }
                for (text = fileRamAddrAt(gScenarioFiles[no], w->cmd->arg); *text != -1; text++) {
                    _strAdd(str, *text);
                }
                w->cmd++;
                mbEntryJumpSet(e, fileRamAddrAt(gScenarioFiles[no], w->cmd->arg));
                i++;
            }
            break;
        case 1:
            mbPlayerInMenuOff(obj, w->player);
            break;
        case 0:
            if (!(w->flags & 1)) {
                if (scPadRepeat(w->player, 0x800) == TRUE ||
                    scPadRepeat(w->player, 0x200) == TRUE) {
                    (*sndPlay)(0x203);
                    if (w->cursor->prev != NULL) {
                        mbEntryCursorOff(w->cursor);
                        w->cursor = w->cursor->prev;
                        mbEntryCursorOn(w->cursor);
                        w->choice--;
                        gMenuChoices[w->player] = w->choice;
                    } else {
                        mbEntryCursorOff(w->cursor);
                        w->cursor = mbEntryLastGet(w->cursor->parent);
                        mbEntryCursorOn(w->cursor);
                        gMenuChoices[w->player] = w->choice = mbEntryIndexGet(w->cursor);
                    }
                } else if (scPadRepeat(w->player, 0x400) == TRUE ||
                           scPadRepeat(w->player, 0x100) == TRUE) {
                    (*sndPlay)(0x203);
                    if (w->cursor->next != NULL) {
                        mbEntryCursorOff(w->cursor);
                        w->cursor = w->cursor->next;
                        mbEntryCursorOn(w->cursor);
                        w->choice++;
                        gMenuChoices[w->player] = w->choice;
                    } else {
                        mbEntryCursorOff(w->cursor);
                        w->cursor = w->cursor->parent->child;
                        mbEntryCursorOn(w->cursor);
                        gMenuChoices[w->player] = w->choice = 0;
                    }
                }
                if (scPadTrg(w->player, 0x8000) == TRUE) {
                    if (*gScVarUiSeMode != 0) {
                        (*sndPlay)(0x205);
                    }
                    gMenuChoices[w->player] = w->choice;
                    w->cmd = mbEntryJumpGet(w->cursor);
                    mbEntryDecide(w->cursor);
                    scPadStickDpad(w->player, 0);
                    w->flags |= 1;
                }
            } else if (scPadTrg(w->player, 0x4000) == TRUE) {
                mbChoiceReset(obj, w->player);
                mbEntryUndecide(w->cursor);
                scPadStickDpad(w->player, 1);
                w->flags &= ~1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Adds a menu under the base, and deletes one. */
Obj *menuCreate(Obj *parent) {
    Obj *m = tskCreate(parent, scMenu_proc, 0, NULL);

    if (m == NULL) {
        PRINTF("menuCreate: Warning menuTask is NULL\n");
    }
    return m;
}

void mbMenuTaskDelete(Obj *menu) {
    tskDestroy(menu);
}

/* Whether it is taken. */
u8 mbMenuIsDecided(Obj *menu) {
    if (MENU(menu)->flags & 1) {
        return TRUE;
    }
    return FALSE;
}

void *mbMenuScPtrGet(Obj *menu) {
    return MENU(menu)->cmd;
}

/* Whether it has the id. */
u8 mbMenuIdCheck(Obj *menu, u8 id) {
    if (MENU(menu)->id == id) {
        return TRUE;
    }
    return FALSE;
}

u32 mbMenuPlayerGet(Obj *menu) {
    return MENU(menu)->player;
}

u8 mbMenuIdGet(Obj *menu) {
    return MENU(menu)->id;
}

Obj *mbMenuCursorGet(Obj *menu) {
    return MENU(menu)->cursor;
}

/* Sets a player's choice and copies it to variable 0x08000064 (every case the same, as in the
 * game). Player 0 is the default's, which makes the switch a jump table. */
void mbChoiceSet(Obj *menu, s32 choice, u32 player) {
    s32 *c = &gMenuChoices[player];

    *c = choice;
    switch (player) {
        case 1:
            *gScVarChoice = *c;
            break;
        case 2:
            *gScVarChoice = *c;
            break;
        case 3:
            *gScVarChoice = *c;
            break;
        case 4:
            *gScVarChoice = *c;
            break;
        case 0:
        default:
            *gScVarChoice = *c;
            break;
    }
}

void mbChoiceReset(Obj *menu, s32 player) {
    mbChoiceSet(menu, 0, player);
}

/* Marks players 1 and 2 as in a menu (variables 0x0800006C, 0x08000070), and not. */
void mbPlayerInMenuOn(Obj *menu, s32 player) {
    switch (player) {
        case 1:
            *gScVarInMenu1 = 1;
            break;
        case 2:
            *gScVarInMenu2 = 1;
            break;
    }
}

void mbPlayerInMenuOff(Obj *menu, s32 player) {
    switch (player) {
        case 1:
            *gScVarInMenu1 = 0;
            break;
        case 2:
            *gScVarInMenu2 = 0;
            break;
    }
}

/* An entry's place: how many come before it. */
s32 mbEntryIndexGet(Obj *entry) {
    Obj *e;
    s32 n = 0;

    for (e = entry->prev; e != NULL; e = e->prev) {
        n++;
    }
    return n;
}

/* A menu's last entry. */
Obj *mbEntryLastGet(Obj *menu) {
    Obj *e = menu->child;

    while (e->next != NULL) {
        e = e->next;
    }
    return e;
}

/* An entry's handler. */
s32 scAMenu(Obj *obj, s16 msg, void *arg) {
    EntryWork *w = ENTRY(obj);
    EntryWork *init = arg;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "scEntry";
            w->frame = scGetFrame(gScRunner, 2);
            w->x = init->x;
            w->y = init->y;
            w->jump = init->jump;
            w->flags = init->flags;
            break;
        case 1:
            if (w->cursor != NULL) {
                frmDestroy((Frame *) w->cursor);
            }
            PRINTF("sc_menu タスクを削除します。\n");
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (w->flags & 1) {
                        w->cursor = cursorCreate(w->frame);
                        if (w->cursor != NULL) {
                            w->cursor->x = w->x;
                            w->cursor->y = w->y;
                            w->cursor->blink = w->blink;
                            w->state = 1;
                        } else {
                            PRINTF("scAMenu: Warning: cursor frame not enough\n");
                        }
                    }
                    break;
                case 1:
                    if (!(w->timer & 1)) {
                        w->blink++;
                    }
                    if (w->flags & 4) {
                        w->blink = 0;
                    }
                    w->cursor->blink = w->blink;
                    if (!(w->flags & 1)) {
                        w->cursor = (CursorFrame *) frmDestroy((Frame *) w->cursor);
                        w->state = 0;
                    }
                    break;
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Adds an entry at (x, y) under a menu. */
Obj *entryCreate(Obj *menu, s16 x, s16 y, void *jump, u8 flags) {
    Obj *e;
    EntryWork init;

    init.x = x;
    init.y = y;
    init.jump = jump;
    init.flags = flags;
    e = tskCreate(menu, scAMenu, 0, &init);
    if (e == NULL) {
        PRINTF("entryCreate: Warning: entry Task is NULL\n");
    }
    return e;
}

void mbEntryDelete(Obj *entry) {
    tskDestroy(entry);
}

/* The cursor on and off; taken and not. */
void mbEntryCursorOn(Obj *entry) {
    ENTRY(entry)->flags |= 1;
}

void mbEntryCursorOff(Obj *entry) {
    ENTRY(entry)->flags &= ~1;
}

void mbEntryDecide(Obj *entry) {
    ENTRY(entry)->flags |= 4;
}

void mbEntryUndecide(Obj *entry) {
    ENTRY(entry)->flags &= ~4;
}

s32 mbEntryIsDecided(Obj *entry) {
    if (ENTRY(entry)->flags & 4) {
        return TRUE;
    }
    return FALSE;
}

void *mbEntryJumpGet(Obj *entry) {
    return ENTRY(entry)->jump;
}

void mbEntryJumpSet(Obj *entry, void *jump) {
    ENTRY(entry)->jump = jump;
}
