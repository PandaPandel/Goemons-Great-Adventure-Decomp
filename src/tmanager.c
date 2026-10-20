#include "object.h"
#include "fade.h"
#include "file.h"
#include "heap.h"
#include "tmanager.h"

/* The task manager ("tmanager"). Overlay 6's objects are tasks in a pool of 100 blocks of 0x80
 * bytes at 0x8016FCF8. A task can span several blocks in a row. Tasks are linked into a tree
 * under the root at 0x8016FCF0, and messages go down the tree through tskSendMsg. */

extern TaskBlock gTskBlocks[100];
extern Obj *gTskRoot; /* the root */
/* tmanager.c's .data (0x801085E0-0x80108660), in ROM order. */
TaskBlock gTskFreeBlock = { 0 }; /* a free block */



/* A free block's handler, and the root's: the default. */
s32 tskFreeBlockProc(Obj *obj, s16 msg, void *arg) {
    return tskDefaultProc(obj, msg, arg);
}

s32 tskRootProc(Obj *obj, s16 msg, void *arg) {
    return tskDefaultProc(obj, msg, arg);
}

/* Frees a block. */
void tskResetBlock(TaskBlock *b) {
    *b = gTskFreeBlock;
    b->obj.proc = tskFreeBlockProc;
}

void tskInit(void) {
    s32 i;

    for (i = 0; i < 100; i++) {
        tskResetBlock(&gTskBlocks[i]);
    }
    gTskRoot = tskCreateRoot(tskRootProc);
}

/* Takes n free blocks in a row, cleared; NULL if there are none. The bound is a local (the
 * original keeps 100 in a register), and the pointer is taken only once the run is found. */
Obj *tskAllocBlocks(s32 n) {
    s32 i;
    s32 j;
    Obj *t;
    s32 max = 100;

    for (i = 0; i < max; i++) {
        if (gTskBlocks[i].obj.flags & 1) {
            i += gTskBlocks[i].obj.blocks - 1;
            continue;
        }
        for (j = 1; j < n; j++) {
            if (i + j >= 100) {
                goto next;
            }
            if (gTskBlocks[j + i].obj.flags & 1) {
                goto next;
            }
        }
        t = &gTskBlocks[i].obj;
        t->blocks = n;
        t->child = NULL;
        t->parent = NULL;
        t->next = NULL;
        t->prev = NULL;
        t->flags |= 1;
        heapMemClear(t + 1, t->blocks * sizeof(TaskBlock) - sizeof(Obj));
        return t;
    next:;
    }
    PRINTF("Caution: %d個の連続したタスクワークがありません \n", n);
    return NULL;
}

/* A task with no parent: the root. */
Obj *tskCreateRoot(TaskProc proc) {
    Obj *t;

    if (proc == NULL || ((u32) proc & 1)) {
        PRINTF("<tmanager> Error: procedure pointer is incorrect!");
        return NULL;
    }
    t = tskAllocBlocks(1);
    if (t != NULL) {
        t->proc = proc;
    }
    return t;
}

/* A task of one block under parent. */
Obj *tskCreate(Obj *parent, TaskProc proc, u32 file, void *arg) {
    return tskCreateBlocks(parent, proc, 1, file, arg);
}

/* A task of n blocks under parent, with its handler in file: linked last among the children,
 * then sent message 2 (create) with arg. NULL if the handler deleted it. */
Obj *tskCreateBlocks(Obj *parent, TaskProc proc, s32 n, u32 file, void *arg) {
    Obj *t;

    if (parent == NULL) {
        PRINTF("<tmanager> Error: parent pointer is null!");
        return NULL;
    }
    if (proc == NULL || ((u32) proc & 1)) {
        PRINTF("<tmanager> Error: procedure pointer is incorrect!");
        return NULL;
    }
    if (n <= 0) {
        n = 1;
    }
    t = tskAllocBlocks(n);
    if (t == NULL) {
        return NULL;
    }
    t->file = file;
    t->proc = proc;
    tskLink(t, parent);
    tskSendMsg(t, 2, arg);
    if (!(t->flags & 1)) {
        return NULL;
    }
    return t;
}

/* Frees one block of a deleted task. */
void tskFreeBlock(TaskBlock *b) {
    tskResetBlock(b);
}

/* The default handler: messages 0 (run), 3 (draw) and 10 go on to the children in turn, until
 * one returns non-zero. */
s32 tskDefaultProc(Obj *obj, s16 msg, void *arg) {
    Obj *c;
    Obj *next;
    s32 ret;

    switch (msg) {
        default:
            return 0;
        case 4:
        case 5:
        case 7:
        case 8:
            return 0;
        case 1:
        case 2:
            return 0;
        case 0:
        case 3:
        case 10:
            for (c = obj->child; c != NULL; c = next) {
                next = c->next;
                ret = tskSendMsg(c, msg, arg);
                if (ret != 0) {
                    return ret;
                }
            }
            return 0;
    }
}

/* Sends a message to a task (not to a sleeping one), with its file mapped for the call. A
 * handler that returns -1 has its task deleted. */
s32 tskSendMsg(Obj *obj, s16 msg, void *arg) {
    s32 ret;
    u32 saved;

    saved = 0;
    if (obj == NULL || !(obj->flags & 1) || (obj->flags & 2)) {
        return 0;
    }
    if (obj->file != 0) {
        saved = fileMappedAddr();
        fileMapTlb(obj->file);
    }
    if (obj->proc != NULL) {
        ret = obj->proc(obj, msg, arg);
    } else {
        ret = 0;
    }
    /* FAKE: the original tests -1 twice (beq, then bnel), as two names for -1 would; one test
     * or a switch gives a single bne. */
    if (ret == -1 || ret == -1) {
        tskDestroy(obj);
        ret = 0;
    }
    if (saved != 0) {
        fileMapTlb(saved);
    }
    return ret;
}

/* Takes a task out of its parent's children (next read first, the parent last). */
void tskUnlink(Obj *obj) {
    Obj *next;
    Obj *prev;
    Obj *parent;

    if (obj == NULL) {
        return;
    }
    if (obj->parent == NULL) {
        return;
    }
    next = obj->next;
    prev = obj->prev;
    parent = obj->parent;
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
    obj->parent = NULL;
    obj->prev = NULL;
    obj->next = NULL;
}

/* Links a task last among parent's children. */
void tskLink(Obj *obj, Obj *parent) {
    Obj *last;

    if (obj->parent != NULL) {
        return;
    }
    obj->parent = parent;
    last = parent->child;
    if (last == NULL) {
        parent->child = obj;
        obj->prev = NULL;
        return;
    }
    while (last->next != NULL) {
        last = last->next;
    }
    last->next = obj;
    obj->prev = last;
}

/* Deletes a task: its children first, then message 1 (free), then its blocks. Returns the task
 * if a child could not be deleted. */
Obj *tskDestroy(Obj *obj) {
    s32 i;
    s32 n;

    if (obj == NULL) {
        PRINTF("<tmanager> Caution: このタスクは既に消去されています\n");
        return NULL;
    }
    if (!(obj->flags & 1)) {
        PRINTF("<tmanager> Caution: このタスクは既に消去されています\n");
        return NULL;
    }
    while (obj->child != NULL) {
        tskDestroy(obj->child);
    }
    if (obj->child != NULL) {
        return obj;
    }
    tskSendMsg(obj, 1, NULL);
    tskUnlink(obj);
    n = obj->blocks;
    for (i = 0; i < n; i++) {
        tskFreeBlock(&((TaskBlock *) obj)[i]);
    }
    return NULL;
}

/* Deletes a task's children. */
void tskDestroyChildren(Obj *obj) {
    Obj *c;
    Obj *prev;

    if (obj == NULL || !(obj->flags & 1)) {
        return;
    }
    c = obj->child;
    while (c != NULL) {
        prev = c;
        tskDestroy(c);
        c = obj->child;
        if (prev == c) {
            PRINTF("<tmanager> リンク構造が不正です\n");
            return;
        }
    }
}

/* How many children a task has. */
s32 tskCountChildren(Obj *obj) {
    Obj *c;
    s32 n;

    n = 0;
    for (c = obj->child; c != NULL; c = c->next) {
        n++;
    }
    return n;
}

/* Finds the task with this handler and file in obj's tree. */
Obj *tskFind(Obj *obj, TaskProc proc, u32 file) {
    Obj *c;
    Obj *found;

    if (obj == NULL) {
        return NULL;
    }
    if (proc == obj->proc && file == obj->file) {
        return obj;
    }
    for (c = obj->child; c != NULL; c = c->next) {
        found = tskFind(c, proc, file);
        if (found != NULL) {
            return found;
        }
    }
    return NULL;
}

/* A task's name, from its file if it has one. The file local is assigned AFTER the test on
 * obj->file: the test's load then takes v0 (lw v0,0x18(a1); move a1,v0 in the jal slot), where
 * assigning it in the test, alone before it, or passing obj->file inline gives a2. */
char *tskGetName(Obj *obj) {
    char *name;
    u32 file;

    if (obj != NULL && (name = obj->name) != NULL) {
        if (obj->file != 0) {
            file = obj->file;
            return (char *) fadeSegToRam((u32) name, file);
        }
        return name;
    }
    return "no name";
}

s32 tskStubReturnZero(void) {
    return 0;
}
