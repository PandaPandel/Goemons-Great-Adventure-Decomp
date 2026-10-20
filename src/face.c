#include "object.h"
#include "face.h"
#include "file.h"
#include "fileres.h"
#include "gfxbuf.h"
#include "gfxmgr.h"
#include "msgfrm.h"
#include "scenario.h"
#include "tmanager.h"

/* The faces in the message windows ("faceRun", scenario.c's faces).
 * - A face is a task that loads its file, then shows one picture frame (faceFrmCreate) under
 *   the runner's fourth frame, 12 right of and 29 below the window's place (variables
 *   0x08000020/0x08000024). Its kind is variable 0x08000030.
 * - Closing it (faceClose) deletes the picture on the next step, and the task with it. */

/* A face's work. */
typedef struct FaceWork {
    /* 0x00 */ Frame *frame;
    /* 0x04 */ PicFrame *pic;
    /* 0x08 */ s32 kind;
    /* 0x0C */ s32 x;
    /* 0x10 */ s32 y;
    /* 0x14 */ u8 timer;
    /* 0x15 */ u8 closing;
} FaceWork;

/* face.c's .data (0x8010AB30-0x8010ACA8), in ROM order. */
s32 gFaceFileTable[47] = { /* each face's file */
    0x000009E1, 0x000009E2, 0x000009E3, 0x000009E4, 0x000009E5, 0x000009E6, 0x000009E7, 0x000009E8,
    0x000009E9, 0x000009EA, 0x000009EB, 0x000009EC, 0x000009ED, 0x000009EE, 0x000009EF, 0x000009F0,
    0x000009F1, 0x000009F2, 0x000009F3, 0x000009F4, 0x000009F5, 0x000009F6, 0x000009F7, 0x000009F8,
    0x000009F9, 0x000009FA, 0x000009FB, 0x000009FC, 0x000009FD, 0x000009FE, 0x000009FF, 0x00000A00,
    0x00000A01, 0x00000A02, 0x00000A03, 0x00000A04, 0x00000A05, 0x00000A06, 0x00000A07, 0x00000A08,
    0x00000A09, 0x00000A0A, 0x00000A0B, 0x00000A0C, 0x00000A0D, 0x00000A0E, 0x00000A0F,
};
u32 gFacePicTable[47] = { /* and its picture there (0x08xxxxxx) */
    0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000,
    0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000,
    0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000,
    0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000,
    0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000,
    0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000, 0x04000000,
};
extern s32 *gScVarWinX;           /* the script variables (scenario.c) */
extern s32 *gScVarWinY;
extern s32 *gScVarFaceKind;

#define FACE(obj) ((FaceWork *) ((u32) (obj) + sizeof(Obj)))

/* The handler. */
s32 faceRun(Obj *obj, s16 msg, void *arg) {
    FaceWork *w = FACE(obj);

    switch (msg) {
        case 2:
            obj->name = "faceRun";
            w->frame = scGetFrame(obj->parent, 3);
            w->x = *gScVarWinX + 12;
            w->y = *gScVarWinY + 29;
            w->kind = *gScVarFaceKind;
            while (gfxGetPendingSwaps() != 0) {
            }
            faceLoadFile(w->kind);
            w->pic = faceMakePic(w->frame, w->x, w->y, w->kind);
            break;
        case 1:
            while (gfxGetPendingSwaps() != 0) {
            }
            faceFreeFile(w->kind);
            PRINTF("sc_face タスクを削除します。\n");
            break;
        case 0:
            switch (w->closing) {
                case 0:
                    break;
                case 1:
                    if (w->pic != NULL) {
                        frmDestroy((Frame *) w->pic);
                    }
                    return -1;
            }
            break;
    }
    w->timer++;
    return tskDefaultProc(obj, msg, arg);
}

/* Opens a face under the runner (its kind is variable 0x08000030), and closes it. */
Obj *faceOpen(Obj *parent, s32 kind) {
    return tskCreate(parent, faceRun, 0, NULL);
}

Obj *faceClose(Obj *face) {
    FACE(face)->closing = TRUE;
    return NULL;
}

/* A face's picture at (x, y) under parent. */
PicFrame *faceMakePic(Frame *parent, f32 x, f32 y, s32 kind) {
    void *tex = (void *) ((gFacePicTable[kind] & 0xFFFFFF) +
                          fileRamAddr(gFaceFileTable[kind]));
    PicFrame *f = faceFrmCreate(parent);


    if (f == NULL) {
        PRINTF("face frame not enough\n");
        return NULL;
    }
    f->x = x;
    f->y = y;
    f->tex = tex;
    return f;
}

/* Loads and frees a face's file. */
void faceLoadFile(s32 kind) {
    fileLoadNow(gFaceFileTable[kind], NULL);
}

void faceFreeFile(s32 kind) {
    resFree(gFaceFileTable[kind]);
}

void faceSetKind(Obj *face, s32 kind) {
    FACE(face)->kind = kind;
}
