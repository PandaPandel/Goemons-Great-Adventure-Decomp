#include "object.h"
#include "audio.h"
#include "file.h"
#include "fileres.h"
#include "gfxmgr.h"
#include "msgfrm.h"
#include "scenario.h"
#include "tmanager.h"
#include "window.h"

/* The message windows ("winRun", scenario.c's windows).
 * - A window is a task under the scenario runner, drawn through its own ortho camera frame:
 *   a background and a frame model ("pol" frames, 0.1 scale), 235 x 55 from its place
 *   (variables 0x08000020/0x08000024).
 * - Its state: 2 opening, 3 open, 1 closing, 0 closed. Five kinds (0-4) have their own
 *   files (gWinKindFileTable) and models; 10 has none; 2 and 3 open without the sound.
 * - A prompt (the blinking "next" mark, promptCreate) shows at its bottom right while
 *   asked for (winPromptOn/winPromptOff). */

/* A window's work (also what it is made with). */
typedef struct WinWork {
    /* 0x00 */ Frame *f1; /* the runner's frames */
    /* 0x04 */ Frame *f0;
    /* 0x08 */ Frame *f2;
    /* 0x0C */ u8 padC[0x24];
    /* 0x30 */ PolFrame *bg;
    /* 0x34 */ PolFrame *fg;
    /* 0x38 */ CamView *cam;
    /* 0x3C */ s32 x;
    /* 0x40 */ s32 y;
    /* 0x44 */ s32 x2; /* its bottom right */
    /* 0x48 */ s32 y2;
    /* 0x4C */ u8 state; /* 2 opening, 3 open, 1 closing, 0 closed */
    /* 0x50 */ u32 kind;
    /* 0x54 */ u8 method;
    /* 0x58 */ s32 player;
    /* 0x5C */ s16 prompt; /* the prompt is asked for */
    /* 0x60 */ struct PromptFrame *promptFrame;
    /* 0x64 */ u8 timer;
    /* 0x65 */ u8 blink;
    /* 0x66 */ u8 promptState;
    /* 0x67 */ u8 pad67[0x5];
} WinWork; /* size 0x6C */

/* The prompt's frame (the "prompt" kind, msgfrm.c). */
typedef struct PromptFrame {
    /* 0x00 */ Frame f;
    /* 0x14 */ s16 x;
    /* 0x16 */ s16 y;
    /* 0x18 */ u8 blink;
} PromptFrame;

/* window.c's .data (0x801097A0-0x80109858), in ROM order. */
#define WIN_FILE_COMMON 5
s32 gWinKindFileTable[6] = { /* each kind's file, then the common one (the loops take all six) */
    0x000009DB, 0x000009DD, 0x000009DE, 0x000009DF, 0x000009DC, 0x000009DA,
};
Gfx *gWinFrameDlTable[5] = { /* each kind's frame model */
    (Gfx *) 0x03000180, (Gfx *) 0x03000180, (Gfx *) 0x030003C0, (Gfx *) 0x03000600,
    (Gfx *) 0x03000180,
};
Gfx *gWinBgDlTable[5] = { /* and background */
    (Gfx *) 0x03000040, (Gfx *) 0x03000040, (Gfx *) 0x03000040, (Gfx *) 0x03000040,
    (Gfx *) 0x03000040,
};
Gfx gWinBgSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetEnvColor(0x00, 0x00, 0x00, 0xB3),
    gsDPSetCombineLERP(0, 0, 0, SHADE, SHADE, 0, ENVIRONMENT, 0, 0, 0, 0, SHADE, SHADE, 0, ENVIRONMENT, 0),
    gsSPEndDisplayList(),
};
Gfx gWinFrameSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2),
    gsDPSetCombineLERP(0, 0, 0, SHADE, SHADE, 0, 0, 0, 0, 0, 0, SHADE, SHADE, 0, 0, 0),
    gsSPEndDisplayList(),
};
extern s16 gFrmKindPol; /* "pol" */
extern s16 gFrmKindCamera; /* "camera" */
extern s32 *gScVarPlayer; /* the script variables (scenario.c) */
extern s32 *gScVarWinX;
extern s32 *gScVarWinY;
extern s32 *gScVarUiSeMode;

#define WIN(obj) ((WinWork *) ((u32) (obj) + sizeof(Obj)))

/* The handler.
 * - The work pointer set at its declaration keeps obj in its home (frame 0x38, no s-register;
 *   IDO_NOTES 132); the pad gives the game's 0x38 frame (w spilled at 0x1C).
 * - x and y are read before x2 and y2 are made from them (the game's temporaries' order).
 * - The state test is a switch (case 2/3 first: its block comes first, then case 1's).
 * - The file's other strings, which its assembly carried, come from the prints after it. */
s32 winRun_proc(Obj *obj, s16 msg, void *arg) {
    WinWork *w = WIN(obj);
    WinWork *init = arg;
    s32 pad[3];

    switch (msg) {
        case 2:
            obj->name = "winRun";
            w->f1 = scGetFrame(obj->parent, 1);
            w->f0 = scGetFrame(obj->parent, 0);
            w->f2 = scGetFrame(obj->parent, 2);
            w->kind = init->kind;
            w->state = init->state;
            w->method = init->method;
            w->x = *gScVarWinX;
            w->y = *gScVarWinY;
            w->x2 = w->x + 235;
            w->y2 = w->y + 55;
            w->player = *gScVarPlayer;
            PRINTF("winRun_proc: win set x[%d] y[%d] [%d]番 method[%d] kind[%d]\n", w->x, w->y,
                   w->player, w->method, w->kind);
            switch (w->kind) {
                default:
                case 0:
                case 1:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                    if (win_init(obj) == -1) {
                        return -1;
                    }
                    if (scIsWindowOpened((Obj *) obj->parent) == 0 && *gScVarUiSeMode != 2) {
                        sndPlay(0x200);
                    }
                    break;
                case 2:
                    win_init(obj);
                    break;
                case 3:
                    win_init(obj);
                    break;
                case 10:
                    break;
            }
            w->prompt = 0;
            break;
        case 1:
            if (w->promptFrame != NULL) {
                frmDestroy((Frame *) w->promptFrame);
            }
            if (w->cam != NULL) {
                frmDestroy((Frame *) w->cam);
            }
            PRINTF("sc_window タスクを削除します。\n");
            break;
        case 0:
            switch (w->state) {
                case 2:
                case 3:
                    w->state = 3;
                    break;
                case 1:
                    w->state = 0;
                    break;
            }
            switch (w->promptState) {
                case 0:
                    if (w->prompt == 1) {
                        w->promptFrame = promptCreate(w->f2);
                        if (w->promptFrame != NULL) {
                            w->promptFrame->x = w->x2;
                            w->promptFrame->y = w->y2;
                            w->promptFrame->blink = w->blink;
                            w->promptState = 1;
                        } else {
                            PRINTF("winRun_proc: Warning: prompt frame not enough\n");
                        }
                    }
                    break;
                case 1:
                    if (!(w->timer & 0xF)) {
                        w->blink++;
                    }
                    w->promptFrame->blink = w->blink;
                    if (w->prompt == 0) {
                        w->promptFrame = (PromptFrame *) frmDestroy((Frame *) w->promptFrame);
                        w->promptState = 0;
                    }
                    break;
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Opens a window of a kind under the runner, with a way of opening ("winCreate": its failure
 * print, dead, leaves the code as a plain return of the call). */
Obj *winCreate(Obj *parent, s32 kind, u8 method) {
    WinWork init;
    Obj *win;

    init.kind = kind;
    init.method = method;
    init.state = 2;
    win = tskCreateBlocks(parent, winRun_proc, 2, 0, &init);
    if (win == NULL) {
        PRINTF("winCreate Warning: あかん失敗したわ\n");
    }
    return win;
}

/* Closes it. */
s32 winClose(Obj *win, u8 method) {
    winSetState(win, 1);
    winSetMethod(win, method);
    return 0;
}

/* Loads and frees a window kind's files (with the common one). */
void winLoadFiles(Obj *win) {
    WinWork *w = WIN(win);

    if (w->kind != 10) {
        fileLoadNow(gWinKindFileTable[w->kind], NULL);
        fileLoadNow(gWinKindFileTable[WIN_FILE_COMMON], NULL);
        PRINTF("ウィンドウ関連セグメント転送[%d]番\n", w->kind);
    }
}

void winFreeFiles(Obj *win) {
    WinWork *w = WIN(win);

    if (w->kind != 10) {
        resFree(gWinKindFileTable[w->kind]);
        resFree(gWinKindFileTable[WIN_FILE_COMMON]);
        PRINTF("ウィンドウ関連セグメント解放[%d]番\n", w->kind);
    }
}

/* Loads and frees all six window files. */
void winLoadAllFiles(void) {
    s32 i;

    for (i = 0; i < 6; i++) {
        fileLoadNow(gWinKindFileTable[i], NULL);
    }
}

void winFreeAllFiles(void) {
    s32 i;

    for (i = 0; i < 6; i++) {
        resFree(gWinKindFileTable[i]);
    }
}

u8 winGetState(Obj *win) {
    return WIN(win)->state;
}

/* These three are old-style definitions: a char parameter comes in as an int, is stored to
 * its home, and is used from the register (sw a1,4(sp)). */
void winSetState(win, state)
Obj *win;
u8 state;
{
    WIN(win)->state = state;
}

void winSetMethod(win, method)
Obj *win;
u8 method;
{
    WIN(win)->method = method;
}

void winSetKind(win, kind)
Obj *win;
u8 kind;
{
    WIN(win)->kind = kind;
}

/* A window's place (the game writes .x twice, so .y is left unset). */
WinPos winGetPos(Obj *win) {
    s32 pad;
    WinPos p;

    p.x = WIN(win)->x;
    p.x = WIN(win)->y;
    return p;
}

s32 winGetPlayer(Obj *win) {
    return WIN(win)->player;
}

/* Asks for the prompt, and stops asking. */
void winPromptOn(Obj *win) {
    if (win != NULL) {
        WIN(win)->prompt = 1;
    } else {
        PRINTF("winPromptOn: Error: No sc_window Task\n");
    }
}

/* Its print says "On" as well (the game's string). */
void winPromptOff(Obj *win) {
    if (win != NULL) {
        WIN(win)->prompt = 0;
    } else {
        PRINTF("winPromptOn: Error: No sc_window Task\n");
    }
}

/* A picture (addr in file) at (x, y) under parent. */
PicFrame *winMakePicture(Frame *parent, f32 x, f32 y, s32 file, u32 addr) {
    PicFrame *p = winFrmCreate(parent);
    void *tex;

    if (p == NULL) {
        return NULL;
    }
    tex = fileRamAddrAt(file, addr);
    p->x = x;
    p->y = y;
    p->tex = tex;
    return p;
}

/* Makes a window's camera and its two models.
 * - The "_winMake cam" print names the camera, so it goes with the camera failure (the bg
 *   and fg failures give the same code with it; its string is the file's last either way). */
s32 win_init(Obj *obj) {
    WinWork *w = WIN(obj);

    w->cam = (CamView *) (*frmCreate)(w->f0, gFrmKindCamera);
    if (w->cam == NULL) {
        PRINTF("win_init: Warning: win frame が作ることができません\n");
        PRINTF(">>>>>  _winMake cam つくれません\n");
        return -1;
    }
    w->bg = (PolFrame *) (*frmCreate)((Frame *) w->cam, gFrmKindPol);
    if (w->bg == NULL) {
        return -1;
    }
    w->fg = (PolFrame *) (*frmCreate)((Frame *) w->cam, gFrmKindPol);
    if (w->fg == NULL) {
        return -1;
    }
    w->cam->ortho |= 1;
    w->cam->zNear = -200.0f;
    w->cam->zFar = 200.0f;
    w->bg->dl = gWinBgSetupDl;
    w->bg->dl2 = gWinBgDlTable[w->kind];
    w->bg->flags |= 0x20;
    w->bg->segs[0] = fileRamAddr(gWinKindFileTable[w->kind]);
    w->bg->pos[0] = w->x + 2;
    w->bg->pos[1] = w->y;
    w->bg->scale[0] = w->bg->scale[1] = w->bg->scale[2] = 0.1f;
    w->fg->dl = gWinFrameSetupDl;
    w->fg->dl2 = gWinFrameDlTable[w->kind];
    w->fg->flags |= 0x20;
    w->fg->segs[0] = fileRamAddr(gWinKindFileTable[WIN_FILE_COMMON]);
    w->fg->pos[0] = w->x + 2;
    w->fg->pos[1] = w->y;
    w->fg->scale[0] = w->fg->scale[1] = w->fg->scale[2] = 0.1f;
    return 0;
}
