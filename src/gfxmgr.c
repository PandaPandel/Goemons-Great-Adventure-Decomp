#include "object.h"
#include "game.h"
#include "ascii.h"
#include "camlight.h"
#include "gfxbuf.h"
#include "gfxmgr.h"
#include "heap.h"
#include "polygon.h"
#include "sys.h"
#include "tmanager.h"

/* The graphic manager ("Graphic Manager") and its frames ("frm" in the prints). Frames are
 * nodes of a tree, walked once a frame to build the display list: a frame whose flag 2 is set
 * runs its kind's draw handler. A kind is a pool of frames of one size, with a free list; there
 * are 25 kinds at 0x80172F70. */

typedef struct FrameKind {
    /* 0x00 */ char *name;
    /* 0x04 */ Frame *head; /* the free list */
    /* 0x08 */ Frame *tail;
    /* 0x0C */ s16 count;
    /* 0x0E */ s16 size;
    /* 0x10 */ void *init; /* copied into a freed frame; NULL clears it */
    /* 0x14 */ u8 *buf;
    /* 0x18 */ FrameDraw draw;
} FrameKind; /* size 0x1C */

extern FrameKind gFrmKindTable[25];
extern Frame *gFrmRoot; /* the root */
extern Frame *gFrmLayers[12]; /* display-list frames under the root */
extern s16 gFrmKindCore; /* the "core  " kind */
extern s16 gFrmKindDlist; /* the "dlist " kind */
extern f32 gWorldOriginX;
extern f32 gWorldOriginY;
extern f32 gWorldOriginZ;
extern f32 gFrmDrawOriginX;
extern f32 gFrmDrawOriginY;
extern f32 gFrmDrawOriginZ;
/* gfxmgr.c's .data (0x80108660-0x80108664), in ROM order. */
Obj *gGraphicManagerTask = NULL; /* the graphic manager's task */
extern Obj *gTskRoot; /* the task root */
extern Gfx *gDisplayListHead; /* the display-list head */
extern Gfx gDrawSetupLitOpaDl[];
extern Gfx gDrawSetupLitTexEdgeDl[];
extern Gfx gDrawSetupFogTexEdgeDl[];
extern Gfx gDrawSetupXluDecalDl[];
extern Gfx gDrawSetupXluSurfDl[];
extern Gfx gDrawSetupFogXluInterDl[];



/* The graphic manager's handler. Create: the frame kinds "core  " (bare frames) and "dlist "
 * (display-list frames), the root, and twelve display-list frames under it. Run: unless
 * drawing is off, message 3 (draw) to every task, then the frame tree into the display list.
 * Free: every frame and kind. */
s32 frmGraphicManagerProc(Obj *obj, s16 msg, void *arg) {
    s32 i;

    switch (msg) {
        case 2:
            gGraphicManagerTask = obj;
            obj->name = "Graphic Manager";
            PRINTF("グラフィックマネージャーをインストールします。\n");
            heapMemClear(gFrmKindTable, sizeof(gFrmKindTable));
            gFrmDrawOriginX = gWorldOriginX = 0.0f;
            gFrmDrawOriginY = gWorldOriginY = 0.0f;
            gFrmDrawOriginZ = gWorldOriginZ = 0.0f;
            gFrmKindCore = frmNewKind("core  ", NULL, sizeof(Frame), 20, NULL);
            gFrmKindDlist = frmNewKind("dlist ", (FrameDraw) frmDlistDraw,
                                                sizeof(DLFrame), 40, NULL);
            camlightMakeCameraKind(5);
            camlightMakeLightKind(4);
            camlightMakeAmbiKind(2);
            asciiNewKind(50);
            polInitPolKind(60);
            polInitAnimeKind(80);
            polInitMpolKind(20);
            polInitManiKind(10);
            polInitGpolKind(80);
            polInitGaniKind(20);
            gFrmRoot = frmCreateRoot(gFrmKindCore);
            for (i = 0; i < 12; i++) {
                gFrmLayers[i] = frmCreate(gFrmRoot, gFrmKindDlist);
            }
            ((DLFrame *) gFrmLayers[0])->dl = gDrawSetupLitOpaDl;
            ((DLFrame *) gFrmLayers[1])->dl = gDrawSetupLitTexEdgeDl;
            ((DLFrame *) gFrmLayers[3])->dl = gDrawSetupFogTexEdgeDl;
            ((DLFrame *) gFrmLayers[4])->dl = gDrawSetupXluDecalDl;
            ((DLFrame *) gFrmLayers[5])->dl = gDrawSetupXluSurfDl;
            ((DLFrame *) gFrmLayers[6])->dl = gDrawSetupFogXluInterDl;
            ((DLFrame *) gFrmLayers[7])->dl = gDrawSetupXluDecalDl;
            ((DLFrame *) gFrmLayers[8])->dl = gDrawSetupXluSurfDl;
            ((DLFrame *) gFrmLayers[10])->dl = gDrawSetupLitOpaDl;
            break;
        case 1:
            frmDestroy(gFrmRoot);
            for (i = 0; i < 25; i++) {
                frmDeleteKind(i);
            }
            gGraphicManagerTask = NULL;
            break;
        case 0:
            if (gGameState.skipDraw == 0 && gGameState.noDraw == 0) {
                gDisplayListHead = sysClearFrame(gDisplayListHead);
                tskSendMsg(gTskRoot, 3, NULL);
                gFrmDrawOriginX = gWorldOriginX;
                gFrmDrawOriginY = gWorldOriginY;
                gFrmDrawOriginZ = gWorldOriginZ;
                camlightStartLights();
                camlightResetCamera();
                gDisplayListHead = frmDrawTree(gDisplayListHead, gFrmRoot);
                gfxGetFreeBytes();
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Restarts the graphic manager. */
void frmGraphicManagerRestart(void) {
    tskDestroy(gGraphicManagerTask);
    tskCreate(gTskRoot, frmGraphicManagerProc, 0, NULL);
}

/* Draws a frame and, depth first, its children. */
Gfx *frmDrawTree(Gfx *gfx, Frame *f) {
    Frame *c;

    if (f->flags & 2) {
        if (gFrmKindTable[f->kind].draw != NULL) {
            gfx = gFrmKindTable[f->kind].draw(gfx, f);
        }
    }
    for (c = f->child; c != NULL; c = c->next) {
        gfx = frmDrawTree(gfx, c);
    }
    return gfx;
}

/* A display-list frame's draw: its list, called or copied in. The Frame parameter cast into a
 * DLFrame local is what keeps the original's copy of f in a2 (the list goes to a3). */
Gfx *frmDlistDraw(Gfx *gfx, Frame *frame) {
    DLFrame *f = (DLFrame *) frame;

    if (f->dl != NULL) {
        if (f->dlFlags & 8) {
            gfx = gfxCopyDisplayList(gfx, f->dl);
        } else {
            gSPDisplayList(gfx++, f->dl);
        }
    }
    return gfx;
}

/* Resets a frame of a kind: cleared, or its kind's first contents copied in. */
void frmReset(Frame *f, s32 kind) {
    FrameKind *k = &gFrmKindTable[kind];

    if (k->init == NULL) {
        heapMemClear(f, k->size);
    } else {
        heapMemCopy(f, k->init, k->size);
    }
    f->kind = kind;
}

/* frmNewKind: a pool of count frames of size bytes, each reset, on the free list. Returns the
 * kind, or -1. The free-kind search is "buf != NULL && i < 25" (the original reads the next
 * kind's buffer before the bound, one past the end at 25); the table is indexed directly, as in
 * the kind delete; size and count are s32 parameters (no sign-extension at entry). Every frame
 * address is "buf + size * n", the head too: "buf + size * 0" makes the tail's size load share
 * the link loop's register (a0), and that is what puts j + 1 in v0 and size in a0 in the loop.
 * Written "(Frame *) buf", the tail's size was a fresh t4 and the loop's two swapped. */
s32 frmNewKind(char *name, FrameDraw draw, s32 size, s32 count, void *init) {
    s32 i;
    s32 j;

#ifndef AVOID_UB
    //! @bug reads gFrmKindTable[25], past the table, before the bound test.
    for (i = 0; gFrmKindTable[i].buf != NULL && i < 25; i++) {
#else
    for (i = 0; i < 25 && gFrmKindTable[i].buf != NULL; i++) {
#endif
    }
    if (i >= 25) {
        PRINTF("<frame> Caution: frame kind is full \n");
        return -1;
    }
    if (size <= 0 || count <= 0) {
        PRINTF("<frame> Error: パラメーターが不正です in frmNewKind \n");
        return -1;
    }
    gFrmKindTable[i].size = size;
    gFrmKindTable[i].count = count;
    gFrmKindTable[i].name = name;
    gFrmKindTable[i].draw = draw;
    gFrmKindTable[i].init = init;
    gFrmKindTable[i].buf = heapMalloc(gFrmKindTable[i].size * gFrmKindTable[i].count);
    if (gFrmKindTable[i].buf == NULL) {
        PRINTF("<frame> Caution: not enough memory in frmNewKind \n");
        return -1;
    }
    for (j = 0; j < gFrmKindTable[i].count; j++) {
        frmReset((Frame *) (gFrmKindTable[i].buf + gFrmKindTable[i].size * j), i);
    }
    for (j = 0; j < gFrmKindTable[i].count - 1; j++) {
        ((Frame *) (gFrmKindTable[i].buf + gFrmKindTable[i].size * j))->child =
            (Frame *) (gFrmKindTable[i].buf + gFrmKindTable[i].size * (j + 1));
        ((Frame *) (gFrmKindTable[i].buf + gFrmKindTable[i].size * (j + 1)))->parent =
            (Frame *) (gFrmKindTable[i].buf + gFrmKindTable[i].size * j);
    }
    gFrmKindTable[i].head = (Frame *) (gFrmKindTable[i].buf + gFrmKindTable[i].size * 0);
    gFrmKindTable[i].tail = (Frame *) (gFrmKindTable[i].buf + gFrmKindTable[i].size * (gFrmKindTable[i].count - 1));
    return i;
}

/* Deletes a kind, and any of its frames still in use. Indexing gFrmKindTable[kind] everywhere,
 * not through a FrameKind * local, is what lets the original keep the count in v0 across the
 * loop (a local made a second copy of the pointer and reloaded the count every pass). */
void frmDeleteKind(s32 kind) {
    Frame *f;
    s32 i;

    if (kind >= 25 || kind < 0) {
        PRINTF("<frame> Error: 無効なカインドを消去しようとしています \n");
        return;
    }
    for (i = 0; i < gFrmKindTable[kind].count; i++) {
        f = (Frame *) (gFrmKindTable[kind].buf + gFrmKindTable[kind].size * i);
        if (f->flags & 1) {
            PRINTF("<frame> Caution: カインドを消去しようとしたところ使用中のワークが存在したので、強制的に削除します。\n");
            frmDestroy(f);
        }
    }
    if (gFrmKindTable[kind].buf != NULL) {
        heapFree(gFrmKindTable[kind].buf);
    }
    gFrmKindTable[kind].buf = NULL;
    gFrmKindTable[kind].size = 0;
    gFrmKindTable[kind].count = 0;
    gFrmKindTable[kind].init = NULL;
    gFrmKindTable[kind].draw = NULL;
    gFrmKindTable[kind].head = NULL;
    gFrmKindTable[kind].tail = NULL;
}

/* A frame of a kind from its free list, in use and drawn. */
Frame *frmAlloc(s32 kind) {
    FrameKind *k;
    Frame *f;

    if (kind < 0) {
        return NULL;
    }
    k = &gFrmKindTable[kind];
    if (k->head == NULL) {
        return NULL;
    }
    f = k->head;
    k->head = f->child;
    if (k->head != NULL) {
        k->head->parent = NULL;
    } else {
        k->tail = NULL;
    }
    f->child = NULL;
    f->flags |= 3;
    return f;
}

/* A root frame. */
Frame *frmCreateRoot(s32 kind) {
    Frame *f = frmAlloc(kind);

    if (f == NULL) {
        PRINTF("ルートフレームが作れません \n");
    }
    return f;
}

/* frmCreate: a frame, last among parent's children. */
Frame *frmCreate(Frame *parent, s16 kind) {
    Frame *f;

    if (parent == NULL) {
        PRINTF("Warning: pointer is NULL in frmCreate \n");
        return NULL;
    }
    f = frmAlloc(kind);
    if (f == NULL) {
        return NULL;
    }
    frmLink(f, parent);
    return f;
}

/* frmCreateNext: a frame after sib. */
Frame *frmCreateNext(Frame *sib, s16 kind) {
    Frame *f;

    if (sib == NULL) {
        PRINTF("Warning: pointer is NULL in frmCreateNext \n");
        return NULL;
    }
    f = frmAlloc(kind);
    if (f == NULL) {
        return NULL;
    }
    frmLinkNext(f, sib);
    return f;
}

/* frmCreatePrev: a frame before sib. */
Frame *frmCreatePrev(Frame *sib, s16 kind) {
    Frame *f;

    if (sib == NULL) {
        PRINTF("Warning: pointer is NULL in frmCreatePrev \n");
        return NULL;
    }
    f = frmAlloc(kind);
    if (f == NULL) {
        return NULL;
    }
    frmLinkPrev(f, sib);
    return f;
}

/* frmCreateTop: a frame, first among parent's children. */
Frame *frmCreateTop(Frame *parent, s16 kind) {
    Frame *f;

    if (parent == NULL) {
        PRINTF("Warning: pointer is NULL in frmCreateTop \n");
        return NULL;
    }
    f = frmAlloc(kind);
    if (f == NULL) {
        return NULL;
    }
    if (parent->child != NULL) {
        frmLinkPrev(f, parent->child);
    } else {
        frmLink(f, parent);
    }
    return f;
}

/* Returns a frame to the end of its kind's free list, reset. The call takes the kind from the
 * frame again. */
void frmFree(Frame *f) {
    FrameKind *k;
    s16 kind = f->kind;

    frmReset(f, f->kind);
    k = &gFrmKindTable[kind];
    if (k->tail != NULL) {
        k->tail->child = f;
        f->parent = k->tail;
        k->tail = f;
    } else {
        k->head = f;
        k->tail = f;
    }
}

/* Takes a frame out of its parent's children. The registers (next v0, prev v1, parent a1, the
 * tested f->parent a2) follow the order the locals first appear: the locals are cleared after
 * the f test, so f keeps a0 and next, prev and parent come ahead of the test's f->parent. */
void frmUnlink(Frame *f) {
    Frame *next;
    Frame *prev;
    Frame *parent;

    if (f == NULL) {
        return;
    }
    next = NULL;
    prev = NULL;
    parent = NULL;
    if (f->parent == NULL) {
        return;
    }
    next = f->next;
    prev = f->prev;
    parent = f->parent;
    if (prev != NULL) {
        prev->next = next;
        if (next != NULL) {
            next->prev = prev;
        }
    } else {
        parent->child = next;
        if (next != NULL) {
            next->prev = prev;
        }
    }
    f->parent = NULL;
    f->prev = NULL;
    f->next = NULL;
}

/* Links a frame last among parent's children. */
void frmLink(Frame *f, Frame *parent) {
    Frame *last;

    if (parent == NULL || f->parent != NULL) {
        return;
    }
    f->parent = parent;
    last = parent->child;
    if (last == NULL) {
        parent->child = f;
        f->prev = NULL;
        return;
    }
    while (last->next != NULL) {
        last = last->next;
    }
    last->next = f;
    f->prev = last;
}

/* Links a frame after sib. */
void frmLinkNext(Frame *f, Frame *sib) {
    if (sib == NULL || f->parent != NULL) {
        return;
    }
    f->parent = sib->parent;
    if (sib->next != NULL) {
        sib->next->prev = f;
    }
    f->next = sib->next;
    f->prev = sib;
    sib->next = f;
}

/* Links a frame before sib. */
void frmLinkPrev(Frame *f, Frame *sib) {
    if (sib == NULL || f->parent != NULL) {
        return;
    }
    f->parent = sib->parent;
    if (sib->prev == NULL) {
        sib->parent->child = f;
    } else {
        sib->prev->next = f;
        f->prev = sib->prev;
    }
    f->next = sib;
    sib->prev = f;
}

/* Moves a frame to the end of parent's children. */
void frmMove(Frame *f, Frame *parent) {
    frmUnlink(f);
    frmLink(f, parent);
}

/* A frame marked free whose links all agree is still in the tree: it is marked in use again.
 * -1 if a link disagrees. */
s32 frmRevive(Frame *f) {
    if (f->parent != NULL && (f->parent->flags & 1) && f == f->parent->child) {
        if (f->prev != NULL && f != f->prev->next) {
            return -1;
        }
        if (f->next != NULL && f != f->next->prev) {
            return -1;
        }
        if (f->child != NULL && f != f->child->parent) {
            return -1;
        }
        PRINTF("<frame> 状況から考えてこのフレームは生きているので復活させます\n");
        f->flags |= 1;
        return 0;
    }
    return -1;
}

/* frmDestroy: a frame and its children, back to their pools. Returns the frame if a child could
 * not be freed. */
Frame *frmDestroy(Frame *f) {
    Frame *self = f;

    if (f == NULL) {
        return NULL;
    }
    if (!(f->flags & 1) && frmRevive(f) == -1) {
        PRINTF("<frame> Caution: このフレームは既に消去されています\n");
        return NULL;
    }
    while (self->child != NULL) {
        frmDestroy(f->child);
    }
    if (f->child != NULL) {
        PRINTF("<frame> Error: System error in frmDestroy ! \n");
        return f;
    }
    frmUnlink(f);
    frmFree(f);
    return NULL;
}

/* Destroys a frame's children. */
void frmDestroyChildren(Frame *f) {
    Frame *c;

    if (f == NULL) {
        return;
    }
    if (!(f->flags & 1) && frmRevive(f) == -1) {
        PRINTF("<frame> Caution: このフレームは既に消去されています\n");
        return;
    }
    c = f->child;
    while (c != NULL) {
        frmDestroy(c);
        if (c == f->child) {
            PRINTF("<frame> リンク構造が不正です\n");
            return;
        }
        c = f->child;
    }
}

/* A frame's place among its siblings. */
s32 frmGetIndex(Frame *f) {
    Frame *p;
    s32 n = 0;

    for (p = f->prev; p != NULL; p = p->prev) {
        n++;
    }
    return n;
}

/* A frame's last child. */
Frame *frmGetLastChild(Frame *f) {
    Frame *last = f->child;

    while (last->next != NULL) {
        last = last->next;
    }
    return last;
}

/* How many children a frame has. */
s32 frmCountChildren(Frame *f) {
    Frame *c;
    s32 n = 0;

    for (c = f->child; c != NULL; c = c->next) {
        n++;
    }
    return n;
}

/* How many frames of a kind are free. */
s32 frmCountFree(s32 kind) {
    Frame *f;
    s32 n = 0;

    for (f = gFrmKindTable[kind].head; f != NULL; f = f->child) {
        n++;
    }
    return n;
}

/* 1 if f is a frame in the pool of its own kind. The end is a u32 sum: written as a pointer sum
 * the loop body is 3 units lighter and IDO unrolls it by 4, not by 2 as the game does
 * (IDO_NOTES rule 44). */
s32 frmIsValid(Frame *f) {
    s32 i;

    for (i = 0; i < 25; i++) {
        if ((u32) f >= (u32) gFrmKindTable[i].buf &&
            (u32) f < (u32) gFrmKindTable[i].buf + gFrmKindTable[i].size * gFrmKindTable[i].count) {
            if (i == f->kind) {
                return 1;
            }
            return 0;
        }
    }
    return 0;
}
