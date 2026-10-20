#include "diary.h"
#include "tlb/tlb_0018.h"

/* The journal task's work, as far as its small handlers show it. */
typedef struct DiaryWork DiaryWork;
typedef s32 (*DiaryFunc)(Obj *obj, DiaryWork *w);

typedef struct DiaryEntry {
    /* 0x00 */ u8 flags;
    /* 0x01 */ u8 pad01[0x3];
    /* 0x04 */ Obj *obj; /* a bookmark picture (tlb_0018_1's SioriWork) */
} DiaryEntry;

struct DiaryWork {
    /* 0x00 */ CamView *view; /* the journal's own camera */
    /* 0x04 */ DiaryFunc mode[5]; /* the current screen's handlers, swapped as a set */
    /* 0x18 */ s32 result; /* an error window's handler's answer (diary.h) */
    /* 0x1C */ Obj *bookmark; /* a bookmark (tlb_0018_1's SioriWork) */
    /* 0x20 */ Obj *siori; /* a bookmark picture of its own */
    /* 0x24 */ DiaryEntry entries[5]; /* 0-2 the journal's slots */
    /* 0x4C */ u16 timer;
    /* 0x4E */ u8 pad4E;
    /* 0x4F */ s8 shown; /* entries made so far, as the journal opens */
    /* 0x50 */ s8 cursor; /* the slot: entries[cursor] */
    /* 0x51 */ s8 prevCursor;
    /* 0x52 */ s8 copyFrom;
    /* 0x53 */ s8 toTitle; /* 1: the journal may close to the title */
};

void diaryShareCostumes(DiaryWork *w);
s32 diaryCopyDo(Obj *obj, DiaryWork *w);
s32 diaryLoad(Obj *obj, DiaryWork *w);
s32 diaryEraseCancel(Obj *obj, DiaryWork *w);
s32 diaryEraseDo(Obj *obj, DiaryWork *w);
s32 diaryEraseStart(Obj *obj, DiaryWork *w);
s32 diaryCopyCancel(Obj *obj, DiaryWork *w);
s32 diaryCopyPick(Obj *obj, DiaryWork *w);
s32 diaryCopyStart(Obj *obj, DiaryWork *w);
s32 diaryToMenu(Obj *obj, DiaryWork *w);
s32 diaryClosing(Obj *obj, DiaryWork *w);
s32 diaryMenu(Obj *obj, DiaryWork *w);

s32 diaryOpening(Obj *obj, DiaryWork *w);

/* "diary", the Controller Pak journal: one retrace a frame, no save in use, a fresh save, file
 * 0x10B, its own camera (planes -200 and 200, orthographic), the opening screen
 * (diaryOpening), sound 0xA1 unless it plays, and the Pak's state as an error window
 * (fadePakInit, with bit 16). Each frame it counts and runs the screen's handler (-1 ends it);
 * an error window's answer moves on: -1 to pseleProc, -2 to the title (sound 7),
 * -4 back to the journal, -5 to error 0x8000. */
s32 diaryProc(Obj *obj, s16 msg, void *arg) {
    DiaryWork *w = (DiaryWork *) (obj + 1);
    s32 pad[4];

    switch (msg) {
        case 2:
            obj->name = "diary";
            gRetracesPerFrame = 1;
            gGameWork.diary = -1;
            fadeNewSave(&gSaveData);
            floadLoadSet(0x10B);
            w->view = (CamView *) frmCreate(gFrmLayers[0], gFrmKindCamera);
            if (w->view == NULL) {
                return -1;
            }
            w->view->ortho |= 1;
            w->view->zNear = -200.0f;
            w->view->zFar = 200.0f;
            w->mode[0] = diaryOpening;
            w->prevCursor = -1;
            w->copyFrom = -1;
            if (sndGetVolume(0xA1, 0) == 0) {
                sndPlay(0xA1);
            }
            diaryErrOpen(obj, fadePakInit() | 0x10000);
            break;
        case 1:
            frmDestroy((Frame *) w->view);
            break;
        case 0:
            switch (w->result) {
                case -1:
                    titleSetNextTask(obj->parent, (s32) pseleProc);
                    break;
                case -5:
                    diaryErrOpen(obj, 0x8000);
                    break;
                case -2:
                    titleSetNextTask(obj->parent, (s32) titleProc);
                    sndPlay(7);
                    break;
                case -4:
                    titleSetNextTask(obj->parent, (s32) diaryProc);
                    break;
                case 0:
                    w->timer++;
                    if (w->mode[0] != NULL && w->mode[0](obj, w) != 0) {
                        return -1;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

void diarySetResult(Obj *obj, s32 result) {
    ((DiaryWork *) FX(obj))->result = result;
}

s32 diaryBack(Obj *obj, DiaryWork *w) {
    w->toTitle = 1;
    w->timer = 0;
    w->mode[0] = diaryClosing;
    return 0;
}

/* Load: the chosen slot's save becomes the game's. */
s32 diaryLoad(Obj *obj, DiaryWork *w) {
    Unk88208 *save;

    sndPlay(0x151);
    gGameWork.diary = w->cursor;
    diaryShareCostumes(w);
    save = diarySioriGetSave(w->entries[w->cursor].obj);
    gSaveData = *save;
    w->toTitle = 0;
    w->timer = 0;
    w->mode[0] = diaryClosing;
    return 0;
}

s32 diaryEraseCancel(Obj *obj, DiaryWork *w) {
    w->entries[3].flags &= ~1;
    diarySioriScaleTo(w->entries[3].obj, 1.0f);
    diaryToMenu(obj, w);
    return 0;
}

/* The chosen slot's Controller Pak check; its error, if any, opens the error window. */
s32 diaryEraseDo(Obj *obj, DiaryWork *w) {
    sndPlay(0x205);
    diarySioriClearSave(w->entries[w->cursor].obj);
    diaryErrOpen(obj, diarySioriWriteSave(w->entries[w->cursor].obj));
    return 0;
}

s32 diaryEraseStart(Obj *obj, DiaryWork *w) {
    sndPlay(0x205);
    diarySioriSetKind(w->siori, 1);
    diarySioriSetKind(w->entries[4].obj, 2);
    w->entries[3].flags |= 1;
    diarySioriScaleTo(w->entries[3].obj, 0.0f);
    w->mode[1] = diaryEraseDo;
    w->mode[3] = diaryEraseCancel;
    w->mode[4] = diaryEraseCancel;
    return 0;
}

s32 diaryCopyCancel(Obj *obj, DiaryWork *w) {
    w->entries[w->copyFrom].flags &= ~1;
    w->prevCursor = w->copyFrom;
    w->copyFrom = -1;
    w->entries[4].flags &= ~1;
    diarySioriScaleTo(w->entries[4].obj, 1.0f);
    diaryToMenu(obj, w);
    return 0;
}

/* Copy: the slot left (copyFrom) onto the chosen one, then its Controller Pak check. */
s32 diaryCopyDo(Obj *obj, DiaryWork *w) {
    Unk88208 *src;
    Unk88208 *dst;

    sndPlay(0x205);
    src = diarySioriGetSave(w->entries[w->copyFrom].obj);
    dst = diarySioriGetSave(w->entries[w->cursor].obj);
    *dst = *src;
    diaryErrOpen(obj, diarySioriWriteSave(w->entries[w->cursor].obj));
    return 0;
}

/* The next slot (1, 2, back to 1). */
s32 diaryCopyPick(Obj *obj, DiaryWork *w) {
    s8 c;

    sndPlay(0x205);
    diarySioriSetKind(w->siori, 2);
    c = w->cursor;
    w->copyFrom = c;
    if (c == 2) {
        w->cursor = 1;
    } else {
        w->cursor = c + 1;
    }
    w->prevCursor = -1;
    w->mode[1] = diaryCopyDo;
    w->entries[w->copyFrom].flags |= 1;
    return 0;
}

s32 diaryCopyStart(Obj *obj, DiaryWork *w) {
    sndPlay(0x205);
    diarySioriSetKind(w->siori, 1);
    diarySioriSetKind(w->entries[3].obj, 2);
    w->entries[4].flags |= 1;
    diarySioriScaleTo(w->entries[4].obj, 0.0f);
    w->mode[1] = diaryCopyPick;
    w->mode[2] = diaryCopyCancel;
    w->mode[4] = diaryCopyCancel;
    return 0;
}

/* Back to the journal's first screen: its handlers, and the bookmarks reset. */
s32 diaryToMenu(Obj *obj, DiaryWork *w) {
    w->mode[0] = diaryMenu;
    w->mode[1] = diaryLoad;
    w->mode[2] = diaryCopyStart;
    w->mode[3] = diaryEraseStart;
    w->mode[4] = diaryBack;
    diarySioriSetKind(w->siori, 0);
    diarySioriSetKind(w->entries[3].obj, 0);
    diarySioriSetKind(w->entries[4].obj, 1);
    return 0;
}

/* A character's row of the save data's costume table (Unk88208's costume), copied whole. */
typedef struct DiaryCostumeRow {
    s8 c[4];
} DiaryCostumeRow;

/* Shares the costumes between the journal's three slots: a costume owned in any slot becomes
 * owned in all of them (the one each character wears, [0], is left as it was). */
void diaryShareCostumes(DiaryWork *w) {
    s32 i;
    s32 j;
    s32 k;
    DiaryCostumeRow owned[4];
    DiaryCostumeRow *rows;

    heapMemClear((u8 *) owned, sizeof(owned));
    for (i = 0; i < 3; i++) {
        rows = (DiaryCostumeRow *) diarySioriGetSave(w->entries[i].obj)->costume;
        for (j = 0; j < 4; j++) {
            for (k = 0; k < 3; k++) {
                if (owned[j].c[k + 1] < rows[j].c[k + 1]) {
                    owned[j].c[k + 1] = rows[j].c[k + 1];
                }
            }
        }
    }
    for (i = 0; i < 3; i++) {
        rows = (DiaryCostumeRow *) diarySioriGetSave(w->entries[i].obj)->costume;
        for (j = 0; j < 4; j++) {
            rows[j] = owned[j];
        }
    }
}


/* The journal opening: its own bookmark at frame 20; the three slots' bookmarks at 28, 36 and
 * 44, 46 apart from 74 down (each opening an error window if its save data has one); two more at
 * 52 and 60, 140 apart from 90 across at 204; then at 70 diaryToMenu. Each grows in.
 * n is read again from w->shown before the test and at the end of the if's block, as the game
 * reloads it after the calls. */
s32 diaryOpening(Obj *obj, DiaryWork *w) {
    s32 n;

    switch (w->timer) {
        case 20:
            w->siori = diaryBeshiMake(obj, 160.0f, 36.0f, 0.0f);
            diarySioriScaleTo(w->siori, 1.0f);
            break;
        case 28:
        case 36:
        case 44:
            n = w->shown;
            w->entries[w->shown].obj = diarySioriMake(obj, (void *) n, 160.0f, n * 46 + 74, 0.0f);
            n = w->shown;
            if (w->entries[w->shown].obj != NULL) {
                diaryErrOpen(obj, diarySioriGetResult(w->entries[w->shown].obj));
                diarySioriScaleTo(w->entries[w->shown].obj, 1.0f);
                n = w->shown;
            }
            w->shown = n + 1;
            break;
        case 52:
        case 60:
            n = w->shown - 3;
            w->entries[w->shown].obj = diaryDelcopyMake(obj, n, n * 140 + 90, 204.0f, 0.0f);
            diarySioriScaleTo(w->entries[w->shown].obj, 1.0f);
            w->shown++;
            break;
        case 70:
            diaryToMenu(obj, w);
            break;
    }
    return 0;
}


/* The journal closing: its bookmarks shrink away one by one (frames 1, 8, 16, 24, 32, 40), and
 * at frame 50 it hands over to the title (sound 7) or, if it may not, opens error 0x8000. */
s32 diaryClosing(Obj *obj, DiaryWork *w) {
    switch (w->timer) {
        case 1:
            diaryDcursorScaleTo(w->bookmark, 0.0f);
            diarySioriScaleTo(w->siori, 0.0f);
            break;
        case 8:
            diarySioriScaleTo(w->entries[0].obj, 0.0f);
            break;
        case 16:
            diarySioriScaleTo(w->entries[1].obj, 0.0f);
            break;
        case 24:
            diarySioriScaleTo(w->entries[2].obj, 0.0f);
            break;
        case 32:
            diarySioriScaleTo(w->entries[3].obj, 0.0f);
            break;
        case 40:
            diarySioriScaleTo(w->entries[4].obj, 0.0f);
            break;
        case 50:
            if (w->toTitle) {
                titleSetNextTask(obj->parent, (s32) titleProc);
                sndPlay(7);
            } else {
                diaryErrOpen(obj, 0x8000);
            }
            break;
    }
    return 0;
}


/* A direction is pressed on its first frame and, held 21 frames or more, every 5th frame. */
#define REPEAT(t) ((t) == 1 || ((t) >= 21 && (t) % 5 == 1))

/* The journal's menu: makes its cursor if it has none, then reads all four controllers (the
 * directions with repeat, A and B). Up and down move between the three slots and the two
 * buttons below (left and right between those), skipping entries marked 1 (up to three
 * times, else staying put); A runs the screen's handler for slots (mode[1]) or the buttons
 * (mode[2], mode[3]), B runs mode[4]. A move plays sound 0x119, slides the slot left out and
 * the new one in (168), colours them, and sends the cursor 8 left of the new one.
 * The skip test is written `w->prevCursor != w->cursor`; the other way round, IDO keeps the old slot
 * in s0. */
s32 diaryMenu(Obj *obj, DiaryWork *w) {
    u16 pressed;
    s32 i;
    s32 t;
    f32 x;
    f32 y;

    if (w->bookmark == NULL) {
        w->bookmark = diaryDcursorMake(obj, 0.0f, 0.0f, 1.0f);
    }
    pressed = 0;
    for (i = 0; i < 4; i++) {
        t = joyGetHeldTime(i, 0x800);
        if (REPEAT(t)) {
            pressed |= 0x800;
        }
        t = joyGetHeldTime(i, 0x400);
        if (REPEAT(t)) {
            pressed |= 0x400;
        }
        t = joyGetHeldTime(i, 0x200);
        if (REPEAT(t)) {
            pressed |= 0x200;
        }
        t = joyGetHeldTime(i, 0x100);
        if (REPEAT(t)) {
            pressed |= 0x100;
        }
        if (joyGetHeldTime(i, 0x8000) == 1) {
            pressed |= 0x8000;
        }
        if (joyGetHeldTime(i, 0x4000) == 1) {
            pressed |= 0x4000;
        }
    }
    i = 0;
    do {
        switch (w->cursor) {
            case 0:
                if (pressed & 0x400) {
                    w->cursor = 1;
                }
                if ((pressed & 0x8000) && w->mode[1] != NULL) {
                    w->mode[1](obj, w);
                }
                break;
            case 1:
                if (pressed & 0x800) {
                    w->cursor = 0;
                }
                if (pressed & 0x400) {
                    w->cursor = 2;
                }
                if ((pressed & 0x8000) && w->mode[1] != NULL) {
                    w->mode[1](obj, w);
                }
                break;
            case 2:
                if (pressed & 0x800) {
                    w->cursor = 1;
                }
                if (pressed & 0x400) {
                    if (pressed & 0x100) {
                        w->cursor = 4;
                    } else {
                        w->cursor = 3;
                    }
                }
                if ((pressed & 0x8000) && w->mode[1] != NULL) {
                    w->mode[1](obj, w);
                }
                break;
            case 3:
                if (pressed & 0x800) {
                    w->cursor = 2;
                }
                if (pressed & 0x100) {
                    w->cursor = 4;
                }
                if ((pressed & 0x8000) && w->mode[2] != NULL) {
                    w->mode[2](obj, w);
                }
                pressed |= 0x100;
                break;
            case 4:
                if (pressed & 0x800) {
                    w->cursor = 2;
                }
                if (pressed & 0x200) {
                    w->cursor = 3;
                }
                if ((pressed & 0x8000) && w->mode[3] != NULL) {
                    w->mode[3](obj, w);
                }
                pressed |= 0x200;
                break;
        }
        i++;
    } while ((w->entries[w->cursor].flags & 1) && i != 3);
    if (w->entries[w->cursor].flags & 1) {
        w->cursor = w->prevCursor;
    }
    if ((pressed & 0x4000) && w->mode[4] != NULL) {
        w->mode[4](obj, w);
    }
    if (w->prevCursor != w->cursor) {
        if (w->prevCursor != -1) {
            sndPlay(0x119);
        }
        if (w->prevCursor >= 0) {
            if (w->prevCursor < 3) {
                diarySioriMoveXTo(w->entries[w->prevCursor].obj, 160.0f);
            }
            diarySioriSetBlue(w->entries[w->prevCursor].obj);
        }
        if (w->cursor >= 0) {
            if (w->cursor < 3) {
                diarySioriMoveXTo(w->entries[w->cursor].obj, 168.0f);
            }
            diarySioriSetGreen(w->entries[w->cursor].obj);
        }
    }
    w->prevCursor = w->cursor;
    diarySioriGetLeft(w->entries[w->cursor].obj, &x, &y);
    diaryDtexSetPos(w->bookmark, x - 8.0f, y);
    return 0;
}
