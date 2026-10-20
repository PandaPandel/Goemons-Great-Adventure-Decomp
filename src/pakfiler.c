#include "object.h"
#include "contpak.h"
#include "ascii.h"
#include "fade.h"
#include "fileload.h"
#include "font.h"
#include "gfxmgr.h"
#include "joypad.h"
#include "mathutil.h"
#include "pakfiler.h"
#include "tmanager.h"

/* The Controller Pak manager ("pakFiler", overlay 8, after the logos). It lists the notes on
 * the Controller Pak in port 1, deletes them, and repairs a damaged Pak. Its tasks:
 * - "pakFiler": watches the Pak being plugged in and pulled out;
 * - "bg": the backdrop;
 * - "list": the note list and its messages. */



/* The manager's work. */
typedef struct PakFiler {
    /* 0x00 */ s32 state; /* 1: the list is reading */
    /* 0x04 */ Obj *bg;
    /* 0x08 */ PakListObj *list;
    /* 0x0C */ s32 pak; /* 1: a Pak at the start; 2: a Pak; 4: none, or an error */
    /* 0x10 */ s32 lastPak;
} PakFiler;

/* The backdrop's work. */
typedef struct PakBg {
    /* 0x00 */ u8 pad00[4];
    /* 0x04 */ s32 alpha;
    /* 0x08 */ DLFrame *frame;
    /* 0x0C */ u32 tex;
} PakBg;

#define ABSF(x) ((x) > 0.0f ? (x) : -(x))

/* The game's debug print, compiled to nothing but its arguments: the strings stay in .rodata and
 * a call in the arguments still runs. */
#define DPRINTF

extern OSMesgQueue gSiMesgQueue; /* the controllers' queue */
/* The backdrop setup: a texture with no palette, alpha from the texture times the environment,
 * colour the environment. */
Gfx gPakFilerBgSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                          G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, ENVIRONMENT, TEXEL0, 0,
                       ENVIRONMENT, 0),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetTextureFilter(G_TF_POINT),
    gsSPEndDisplayList(),
};
/* The fill setup: the environment colour, translucent. */
Gfx gPakFilerFillSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsDPSetBlendColor(0, 0, 0, 1),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0,
                       ENVIRONMENT),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsSPEndDisplayList(),
};
s32 gPakFilerRunning = 0; /* 1 while the manager runs */
/* The Pak character codes in the font: 0 and 15 a space, 1-14 unknown, then the digits, the
 * letters and the punctuation (0x42 codes). */
u8 gPakFilerCharTable[] = " ************** 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#'*+,-./:=?@";
extern Gfx *gGfxDlHead; /* the display-list buffer */
extern u8 gPakFilerMsgStr[]; /* the message line */
extern Frame *gFrmLayers[]; /* the frame pools */
extern s16 gFrmKindDlist; /* "dlist " */
extern u8 pakBackdropTex[]; /* the backdrop, in file 0x109 */
extern Frame *gFrmTextRoot; /* the frame text goes under */
extern s16 gFrmKindCore; /* "null" */
extern u8 gPakFilerLines[28][37]; /* the list's lines */

s32 pakFilerCursorGetRow(PakCursorObj *cursor);
s32 contPakDeleteFile(s32 file, s32 port); /* FAKE: in no header: src/pakfiler.c includes contpak.h already and used here as this file declares it (Reference of an expression of void type or an incomplete type.); contpak.h has `void contPakDeleteFile(s32 file, s32 port)` */ /* void in contpak.c: the list reads v0, which
                                             * still holds the error it stored */
extern Frame *gFramePool10; /* the frame the HUD's frame goes before */

#define FX(obj) ((void *) ((u32) (obj) + sizeof(Obj)))

/* The manager's task. It watches the Pak in port 1: when one is plugged in, the list reads
 * it; when it is pulled out, the list clears. START leaves.
 * Case 2 reads the Pak first and then sets pak = 0 and ORs the bits in (the game's
 * "ori t0,zero,4" is 0 | 4 forwarded). The unused pad is a word of the game's frame (0x30). */
s32 pakFiler_proc(Obj *obj, s16 msg, void *arg) {
    PakFiler *w = FX(obj);
    s32 r;
    u8 pattern;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "pakFiler";
            gPakFilerRunning = 1;
            PRINTF("pakFiler started\n");
            frmFontNewKind();
            floadLoadSet(0x109);
            w->bg = pakFilerStartBg(obj);
            r = pakFilerIsPlug(&pattern);
            w->pak = 0;
            if (r != 0) {
                w->pak |= 4;
            } else if (pattern & 1) {
                w->pak |= 3;
            }
            w->lastPak = w->pak;
            w->list = (PakListObj *) pakFilerStartList(obj);
            w->state = 0;
            if (!(w->pak & 1)) {
                return -1;
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (joyGetHeldTime(0, START_BUTTON) == 1) {
                        return -1;
                    }
                    r = pakFilerIsPlug(&pattern);
                    w->lastPak = w->pak;
                    w->pak &= ~6;
                    if (r != 0) {
                        w->pak = 4;
                        if (w->lastPak & 2) {
                            pakFilerListRead(w->list);
                        }
                    } else if (pattern & 1) {
                        w->pak = 2;
                        if (!(w->lastPak & 2)) {
                            w->state = 1;
                            pakFilerListAskReload(w->list);
                        }
                    } else if (w->lastPak & 2) {
                        pakFilerListRead(w->list);
                    }
                    break;
                case 1:
                    if (joyGetHeldTime(0, START_BUTTON) == 1) {
                        return -1;
                    }
                    r = pakFilerIsPlug(&pattern);
                    w->lastPak = w->pak;
                    w->pak &= ~6;
                    if (r == 0 && (pattern & 1)) {
                        w->pak = 2;
                        if (pakFilerListIsReading(w->list) != 0) {
                            w->state = 0;
                        }
                    } else {
                        w->state = 0;
                    }
                    break;
                case 2:
                    break;
            }
            break;
        case 3:
            if (gPakFilerRunning == 0) {
                PRINTF("pakFiler reloadable\n");
            }
            break;
        case 1:
            contPakProbeAll();
            frmFontDeleteKind();
            gPakFilerRunning = 0;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* 1 while the manager runs. */
s32 pakFilerIsRunning(s32 unused) {
    return gPakFilerRunning;
}

/* The backdrop's task: a 288x224 picture from file 0x109, tinted brown and cream. */
s32 bg_proc(Obj *obj, s16 msg, void *arg) {
    PakBg *w = FX(obj);
    s32 pad;

    switch (msg) {
        case 0:
            break;
        case 2:
            obj->name = "bg";
            floadLoadSet(0x109);
            w->tex = floadSegToRam((u32) pakBackdropTex, 0x109, 0);
            w->frame = (DLFrame *) frmCreate(gFrmLayers[0], gFrmKindDlist);
            w->alpha = 0xFF;
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, fadeSegToRam((u32) gPakFilerBgSetupDl, obj->file));
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, 0, 0, 0,
                              ENVIRONMENT, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, 0, 0, 0,
                              ENVIRONMENT);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 0xCC, 0xB3, 0x80, 0xFF);
            gDPSetEnvColor(gGfxDlHead++, 0x40, 0x26, 0x0C, w->alpha);
            gGfxDlHead = pakFilerDrawI4Tiles(gGfxDlHead, 16.0f, 8.0f, 1.0f, 0x120, 0xE0, w->tex);
            gGfxDlHead++;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Starts the backdrop. */
Obj *pakFilerStartBg(Obj *obj) {
    return tskCreate(obj, bg_proc, obj->file, NULL);
}

/* The list's task. It reads the Pak's notes into lines 3-18 (name at 5, extension at 22,
 * pages at 28), then runs the delete, reload and repair screens. The clearing loop is on one
 * line (it stores before it steps). The name and the extension each have their own u8 (c, d),
 * declared above pattern: the two bytes share the slot word above it, and two variables give j
 * s0 (one c for both loops takes s0 itself). */
s32 list_proc(Obj *obj, s16 msg, void *arg) {
    PakList *w = FX(obj);
    s32 i;
    s32 j;
    u8 c;
    u8 d;
    u8 *p;
    u8 pattern;
    s32 pages;

    switch (msg) {
        case 2:
            obj->name = "list";
            w->port = 0;
            w->unused0C = 0;
            w->text = NULL;
            w->text2 = NULL;
            pakFilerListMakeLines((PakListObj *) obj);
            w->cursor = (PakCursorObj *) pakFilerStartCursor(obj);
            w->state = 0;
            break;
        case 0:
            switch (w->state) {
                case 0:
                    pakFilerListMakeLines((PakListObj *) obj);
                    for (i = 0; i < 16; i++) { w->used[i] = 0; }
                    if ((w->error = pakFilerIsPlug(&pattern)) != 0) {
                        goto error;
                    }
                    if (!(pattern & (1 << w->port))) {
                        w->error = 0x100;
                        goto error;
                    }
                    if ((w->error = contPakProbe(w->port)) != 0) {
                        goto error;
                    }
                    if ((w->error = contPakReadFileStates(w->port)) != 0) {
                        goto error;
                    }
                    pakFilerCursorShow(w->cursor);
                    pakFilerCursorRelease(w->cursor);
                    if (gContPak[w->port].error != PFS_ERR_INVALID) {
                        asciiNew(w->text, 64, 30, " NOTE NAME          PAGES");
                        for (i = 0; i < 16; i++) {
                            DPRINTF("%d, ", i);
                            DPRINTF("%08xbytes, ", gContPak[w->port].files[i].file_size);
                            p = &gPakFilerLines[i + 3][5];
                            if (gContPak[w->port].fileState[i] == PFS_ERR_INVALID ||
                                gContPak[w->port].files[i].file_size == 0) {
                                *pakFilerStrCopy(p, "EMPTY") = '\0';
                                DPRINTF("\n");
                                continue;
                            }
                            w->used[i] |= 1;
                            for (j = 0; j < 16; j++) {
                                c = gContPak[w->port].files[i].game_name[j];

                                if (c == 0) {
                                    break;
                                }
                                *p++ = pakFilerFontChar(c);
                                DPRINTF("%c", pakFilerFontChar(c));
                            }
                            gPakFilerLines[i + 3][21] = '.';
                            DPRINTF(".");
                            p = &gPakFilerLines[i + 3][22];
                            for (j = 0; j < 4; j++) {
                                d = gContPak[w->port].files[i].ext_name[j];

                                if (d == 0) {
                                    break;
                                }
                                *p++ = pakFilerFontChar(d);
                                DPRINTF("%c", pakFilerFontChar(d));
                            }
                            *pakFilerNum3(&gPakFilerLines[i + 3][28],
                                                    gContPak[w->port].files[i].file_size >> 8) = '\0';
                            DPRINTF("\n");
                        }
                    }
                    contPakFreePages(w->port, &pages);
                    pakFilerStrCopy(pakFilerNum3(&gPakFilerLines[23][21], pages), " PAGES FREE");
                    pakFilerStrCopy(&gPakFilerLines[24][19], "REQUIRES 7 PAGES");
                    pakFilerStrCenter(gPakFilerLines[21], 36, "         PRESS START TO EXIT         ");
                    w->state = 1;
                    break;
                case 1:
                    if (w->used[pakFilerCursorGetRow(w->cursor)] & 1) {
                        if (pakFilerCursorIsChosen(w->cursor)) {
                            pakFilerStrCenter(gPakFilerLines[20], 36, "   PRESS THE Z BUTTON TO CONFIRM  ");
                            pakFilerStrCenter(gPakFilerLines[21], 36, "   PRESS THE B BUTTON TO CANCEL   ");
                            w->state = 2;
                            break;
                        }
                        pakFilerStrCenter(gPakFilerLines[20], 36, "   PRESS THE A BUTTON TO DELETE   ");
                    } else {
                        pakFilerStrCenter(gPakFilerLines[20], 36, "           EMPTY GAME SLOT          ");
                        if (pakFilerCursorIsChosen(w->cursor)) {
                            pakFilerCursorRelease(w->cursor);
                        }
                    }
                    pakFilerStrCenter(gPakFilerLines[21], 36, "         PRESS START TO EXIT         ");
                    break;
                case 2:
                    pakFilerCursorHold(w->cursor);
                    if (joyGetHeldTime(0, B_BUTTON) == 1) {
                        pakFilerCursorRelease(w->cursor);
                        w->state = 3;
                    } else if (joyGetHeldTime(0, Z_TRIG) == 1) {
                        w->state = 6;
                    }
                    break;
                case 3:
                    w->state = 1;
                    break;
                case 6:
                    pakFilerStrCopy(gPakFilerLines[20] + 1, "              DELETING             ");
                    if ((w->error = contPakDeleteFile(pakFilerCursorGetRow(w->cursor), w->port)) != 0) {
                        goto error;
                    }
                    pakFilerCursorRelease(w->cursor);
                    pakFilerListRead((PakListObj *) obj);
                    break;
                case 7:
                    pakFilerListMakeLines((PakListObj *) obj);
                    pakFilerCursorHide(w->cursor);
                    pakFilerStrCenter(gPakFilerLines[21], 36, "    PRESS THE A BUTTON TO RELOAD    ");
                    w->state = 8;
                    break;
                case 8:
                    pakFilerCursorHold(w->cursor);
                    if (joyGetHeldTime(0, A_BUTTON) == 1) {
                        pakFilerListRead((PakListObj *) obj);
                    }
                    break;
                case 9:
                    pakFilerListMakeLines((PakListObj *) obj);
                    pakFilerCursorHide(w->cursor);
                    w->state = 10;
                    pakFilerListShowError((PakListObj *) obj);
                    break;
                case 4:
                    pakFilerStrCenter(gPakFilerLines[8], 36, "DAMAGED CONTROLLER PAK");
                    pakFilerStrCenter(gPakFilerLines[12], 36, "PRESS THE Z BUTTON TO REPAIR");
                    pakFilerStrCenter(gPakFilerLines[13], 36, "(DATA MAY BE LOST)");
                    pakFilerStrCenter(gPakFilerLines[16], 36, "OR RECONNECT THE CONTROLLER PAK");
                    pakFilerStrCenter(gPakFilerLines[17], 36, "AND PRESS THE A BUTTON");
                    w->state = 5;
                    break;
                case 5:
                    pakFilerCursorHold(w->cursor);
                    if (joyGetHeldTime(0, Z_TRIG) == 1) {
                        if ((w->error = contPakInitPfsRepair(w->port)) != 0) {
                            goto error;
                        }
                        contPakProbe(w->port);
                        pakFilerListRead((PakListObj *) obj);
                    }
                    break;
                case 10:
                    break;
            }
            break;
        case 1:
            frmDestroy(w->text);
            frmDestroy(w->text2);
            break;
    }
    return tskDefaultProc(obj, msg, arg);

error:
    w->state = 9;
    DPRINTF("list_proc: CREATE: error\n");
    return tskDefaultProc(obj, msg, arg);
}

/* Starts the note list. */
Obj *pakFilerStartList(Obj *obj) {
    return tskCreate(obj, list_proc, obj->file, NULL);
}

s32 pakFilerListRead(PakListObj *list) {
    list->w.state = 0;
    return 0;
}

s32 pakFilerListIsReading(PakListObj *list) {
    return list->w.state == 0;
}

void pakFilerUnusedStub(void) {
}

/* Has the list read the Pak again. */
void pakFilerListAskReload(PakListObj *list) {
    list->w.state = 7;
}

/* Makes the list's 28 text lines (36 characters, 8 apart from y 16), cleared to spaces. */
void pakFilerListMakeLines(PakListObj *list) {
    PakList *w = &list->w;
    u8 *line;
    s32 i;
    s32 y;
    s32 j;

    if (w->text != NULL) {
        frmDestroy(w->text);
    }
    w->text = frmCreate(gFrmTextRoot, gFrmKindCore);
    for (i = 0, y = 16, line = gPakFilerLines[0]; i < 28; i++, y += 8, line += 37) {
        if (asciiNew(w->text, 16, y, line) == 0) {
            return;
        }
        for (j = 0; j < 36; j++) {
            gPakFilerLines[i][j] = ' ';
        }
        gPakFilerLines[i][j] = '\0';
    }
    if (w->text2 != NULL) {
        frmDestroy(w->text2);
    }
    w->text2 = frmCreate(gFrmTextRoot, gFrmKindCore);
}

/* The message for the list's error. */
s32 pakFilerListShowError(PakListObj *list) {
    switch (list->w.error) {
        case 0:
            break;
        case PFS_ERR_NOPACK:
            pakFilerStrCenter(gPakFilerMsgStr, 36, "NO CONTROLLER PAK");
            break;
        case PFS_ERR_DEVICE:
            pakFilerStrCenter(gPakFilerMsgStr, 36, "UNKNOWN DEVICE");
            break;
        case PFS_ERR_ID_FATAL:
            list->w.state = 4;
            break;
        case 0x100:
            pakFilerStrCenter(gPakFilerMsgStr, 36, "NO CONTROLLER PAK");
            break;
        case 0x101:
            pakFilerStrCenter(gPakFilerMsgStr, 36, "NO CONTROLLER");
            break;
        case 0x200:
            pakFilerStrCenter(gPakFilerMsgStr, 36, "RUMBLE PAK ATTACHED");
            break;
        default:
            pakFilerStrCenter(gPakFilerMsgStr, 36, "ERROR");
            break;
    }
    return 0;
}

/* The cursor's task: up and down over the 16 notes (repeating after 20 frames, every 5), A to
 * choose. It draws a dark panel and the chosen row, pulsing blue, or red while confirming or
 * deleting. */
s32 pakFilerCursorProc(Obj *obj, s16 msg, void *arg) {
    PakCursor *w = FX(obj);
    s32 pad;
    s32 r;
    s32 g;
    s32 b;

    switch (msg) {
        case 2:
            w->row = 0;
            w->frame = (DLFrame *) frmCreate(gFramePool10, gFrmKindDlist);
            joySetStickDpad(0, 1);
            pakFilerCursorShow((PakCursorObj *) obj);
            w->timer = 0;
            w->state = 0;
            break;
        case 0:
            w->timer++;
            switch (w->state) {
                case 0:
                    w->state = 1;
                    break;
                case 1:
                    if (joyGetHeldTime(0, U_JPAD) == 1 ||
                        (joyGetHeldTime(0, U_JPAD) > 20 &&
                         joyGetHeldTime(0, U_JPAD) % 5 == 1)) {
                        if (w->row == 0) {
                            w->row = 15;
                        } else {
                            w->row--;
                        }
                        w->timer = 0;
                    } else if (joyGetHeldTime(0, D_JPAD) == 1 ||
                               (joyGetHeldTime(0, D_JPAD) > 20 &&
                                joyGetHeldTime(0, D_JPAD) % 5 == 1)) {
                        if (w->row == 15) {
                            w->row = 0;
                        } else {
                            w->row++;
                        }
                        w->timer = 0;
                    }
                    if (joyGetHeldTime(0, A_BUTTON) == 1) {
                        w->state = 2;
                    }
                    break;
                case 2:
                case 3:
                case 4:
                case 5:
                    break;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            if (w->shown) {
                pakFilerFillRect(44, 28, 276, 170, 0, 0, 0, 100);
                if (w->state == 4 || w->state == 2) {
                    r = 0xFF;
                    g = 0x20;
                    b = 0x28;
                } else {
                    r = (u8) (ABSF(mathCos((f32) w->timer / 30.0)) * 64.0f);
                    g = (u8) (ABSF(mathCos((f32) w->timer / 30.0)) * 128.0f);
                    b = 0xFF;
                }
                pakFilerFillRect(48, w->row * 8 + 40, 272, w->row * 8 + 48, r, g, b, 150);
            }
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            joySetStickDpad(0, 0);
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *pakFilerStartCursor(Obj *obj) {
    return tskCreate(obj, pakFilerCursorProc, obj->file, NULL);
}

/* The cursor's row. */
s32 pakFilerCursorGetRow(PakCursorObj *cursor) {
    return cursor->w.row;
}

/* Has a row been chosen. */
s32 pakFilerCursorIsChosen(PakCursorObj *cursor) {
    return cursor->w.state == 2;
}

/* Holds the cursor. */
void pakFilerCursorHold(PakCursorObj *cursor) {
    cursor->w.state = 4;
}

/* Lets it move. */
void pakFilerCursorRelease(PakCursorObj *cursor) {
    cursor->w.state = 1;
}

void pakFilerCursorShow(PakCursorObj *cursor) {
    cursor->w.shown = 1;
}

void pakFilerCursorHide(PakCursorObj *cursor) {
    cursor->w.shown = 0;
}

/* A filled rectangle in a colour. */
void pakFilerFillRect(s32 ulx, s32 uly, s32 lrx, s32 lry, s32 r, s32 g, s32 b, s32 a) {
    gSPDisplayList(gGfxDlHead++, gPakFilerFillSetupDl);
    gDPSetEnvColor(gGfxDlHead++, r, g, b, a);
    gDPFillRectangle(gGfxDlHead++, ulx, uly, lrx, lry);
}

/* Draws a w x h 4-bit intensity picture at (x, y), scaled, in 64x64 tiles. Returns the list's
 * end command, not past it.
 * Matched 2026-10-03 by the tile size as two locals, tw = 64 declared first
 * and th = 64 declared last: IDO folds a local constant late, so row * th is hoisted after the
 * dsdx chain (the game's reload of row, which the FAKE "r = row * 1" had stood for), and th is
 * the 22nd local with one compiler temp fewer than the literal 64 (the nx copy at 0x5C).
 * Measured in scratch compiles: one size local for all four uses gives the reload with the nx
 * copy still at 0x60; adding a 22nd local (th for t and tb, th for s and sr, or even an unused
 * scalar) moves it to 0x5C; the size in nx and ny ((w + size - 1) / size) changes nothing.
 * History, kept for the levers (every form below was measured on the C without tw/th):
 * one instruction short. The original stores row and reloads it before
 * `row * 64` (sw v1,0xB0; lw v0,0xB0; sll v0,v0,6), and its copy of nx for the inner loop's
 * entry test sits at 0x5C (this C: 0x60); frame 0xB8 and the 0x38 spill already agree, so the
 * original has one compiler temporary more above that copy (or one more local and one temp
 * fewer). No change to the declarations fixes it: an extra pad, a named dsdx local (in either
 * loop) or no `t` (row * 64 inline) grow the frame to 0xC0; col declared first moves both
 * spills up 4; t set in the outer loop or block-scoped tile locals change nothing.
 * Frame model (measured): 0x38 + 4 * (locals + compiler temps), rounded to 8, any pad word at
 * the bottom; the nx copy is the FIRST temp, right under the locals. Almost all temps come from
 * strength-reducing col * 64: `s` as an explicit second loop variable leaves one temp (frame
 * 0x90). So the original has 22 locals + 10 temps, or 21 + 11 with one temp made before the nx
 * copy (the reload of row may be that temp). Also tried, no fix: s before t, t declared first,
 * `nx > col`, `row << 6` (+ a pad: 0xC0), dsdx named in place of pad (41 lines, no reload);
 * worse: (col + 1) * 64 / (row + 1) * 64 and t after the sr clamp (each +1 temp), row * 64
 * inline (hoisted above the inner guard).
 * Second pass (measured, each against this C): no change (same 25 lines) from `64 * row`,
 * `row << 6`, or `if ((sr = s + 64) > w)`. ny computed before nx: 45 lines, and the nx copy
 * stays at 0x60, so the temps are not ordered by which one is computed first. A ternary sr: 92 lines.
 * A float `inv = 1.0f / scale` local: frame 0xC0 (+1 local, no temp saved). Naming
 * `lrs = sr - 1` cuts nine temps (frame 0x98, 248 lines), so most temps come from the inline
 * `sr - 1` in the macro and the game has it inline. A named `lrt = tb - 1` gives frame 0xC0.
 * The reload looks like row's split live range (register in the outer head and latch, memory
 * in the inner loop) ending one block earlier in the game, before the preheader, not after it.
 * Third pass (measured): compiled unparked, as the build does (pakFiler_proc as asm), the same
 * 25 lines, so it was not an artifact of the check's compile. `(u16) (s32)` on dsdx/dtdy: no change;
 * `(u16)` straight from the float: the unsigned cfc1/ctc1 sequence (79 lines); a `u16 dsdx`
 * local set in the inner loop: frame 0xC0, 65 lines, no reload. A FAKE `if (0) {}` in the outer
 * body (before or after the inner loop): no change; at the end of the inner body: 273 lines.
 * Measured 2026-10-02: a
 * FAKE "do {} while (0);" at the start of the row loop's body, 55; "tb = (row + 1) * 64" for
 * "tb = t + 64" with the pad removed, 51; a FAKE "if (0) {}" at the start of the column loop's
 * body, 99.
 * Fourth pass (2026-10-02, measured): `permuter.py pads` (50 variants): every swap of col, s, t,
 * sr, tb keeps 25 (their order is not it); every pad 29-51; no pad 41; row before pad 29.
 * A FAKE "do {} while (0);" after the column loop (end of the row body): 55 (w/h swap s3/s4).
 * s32 dsdx in pad's place, set just before the rectangle: 41 (nx copy still 0x60, no reload,
 * s7/ra/s8 rotate), so a named dsdx is no extra temp. s32 dsdx declared last (pad kept) and
 * set FIRST in the column body: frame 0xC0, 65 (22 locals + 11 temps: no temp saved).
 * "tb = t + 64" moved up before the sr clamp: frame 0xC0, nx copy 0x68, 65 (+1-2 temps).
 * "s = col << 6": frame 0x90, 241: IDO does not strength-reduce a shift, so the game wrote the
 * multiply, and every temp but the nx copy comes from reducing col * 64. dtdy written
 * "(s32) (1024.0f * (1.0f / scale))" (commuted): the same 25.
 * Where it stands: the code is the game's except that row's register range takes in the inner
 * preheader (the game stores row at the preheader's start and reads it back), and the game has
 * one more frame word above the nx copy. Both point at one more live range or temp in the
 * game's preheader; no form of t, tb, dsdx or the declarations has made it. Untried: a FAKE
 * empty test (IDO_NOTES file.c/vanime.c levers) there, and a decomp-permuter run.
 * 2026-10-03: a FAKE "if (row) {}" just before the column loop: 55; at the end of the row
 * body: 55 (both as the do-while boundaries there).
 * The empties sweep (2026-10-03, 25 to 16): a FAKE "if (t) {}" after the loops. This C is now
 * the game's length (233); in place of the game's reload of row before "row * 64" it stores t
 * (0x9C), and the nx copy is still at 0x60 for 0x5C. Kept: the closest form in lines and length
 * (its mix distance 2 from 1: one sw for the game's lw).
 * 2026-10-03: splits, empties, copies, joins, inlines and pads re-run on this
 * C (with the FAKE): nothing under 16. The FAKE "if (t) {}" moved to the end of the row body
 * (after the column loop): 44. Without the FAKE (measured again): 25, and the code is the
 * game's exactly but for the missing reload (every other line is a branch offset or the nx
 * copy's slot), so the FAKE fixes no colouring: its t store only makes up the length. A FAKE
 * "do {} while (0);" before the row loop (no "if (t)"): 28, IDO turns the row loop's test
 * into a bne (bnel v1,t9). The row loop as "row = 0; while (row < ny) { ...; row++; }": the
 * same 16 (for and while compile alike). "tb = row * 64 + 64" (a second row * 64 for IDO to
 * share): frame 0xC0, nx copy 8 lower with everything, no reload, 54. A FAKE "if (row) {}" at
 * the start of the column body (no "if (t)"): row takes s6 through both loops, 287. The 44 of
 * "if (t) {}" at the end of the row body: w, h and the 0xE7 constant rotate s3/s4/s5, still
 * no reload.
 * What the reload means (reasoned from the code, not proven): the game's row lives in three
 * pieces, v1 for the entry, row head and latch, memory for the column loop, and v0 for the
 * column loop's preheader alone ("sw v1" ends the first piece, "lw v0" starts the third, and
 * no store follows since memory already holds row). This C makes the preheader part of the v1
 * piece. So IDO's split grew the first piece from a unit that does not reach the preheader
 * (the latch, by predecessors?) where this C grows it from one that does. "row = -1; while
 * (++row < ny)" (latch test first in the source): the same 16, IDO canonicalises the loop.
 * Next to try: a source shape that changes the column loop's preheader or the row loop's block
 * order in uopt without changing the code, and a decomp-permuter run on the FAKE-free C (25)
 * scored on the reload alone.
 * 2026-10-03, later: `commutes` on the FAKE C: nothing (4 swaps). `splits` on the
 * FAKE-free C (25): nothing under 25 (before the first statement 28, row body head 55, after
 * the clamps 99, after sr = s + 64 109). A named s32 dsdx in pad's place (21 locals, frame
 * 0xB8) set FIRST in the column body, no FAKE: 41, no reload, ra/s7/s8 rotate (dsdx is still
 * v1 in the preheader, so the order of the hoisted code is not what keeps row's v1).
 * THE RELOAD, FOUND (by IDO's own -S output, which is ugen's code before as1):
 * ugen emits "sw $3,176($sp); lw $2,176($sp)" for row at the preheader's top in this C too;
 * as1 then deletes the load (it forwards the stored register). The game kept the load because
 * its ugen put row's load AFTER the dsdx chain, whose "mfc1 $3" clobbers v1. In this C
 * "t = row * 64" is a moved statement at the preheader's head; an outer-body copy that uopt
 * folds late, "r = row * 1;" with "t = r * 64" inside, puts row * 64 with the t-derived code
 * after dsdx: 6 lines, mix 0, only the nx copy's slot left (0x60 for 0x5C). "r = row" (folded
 * early: row takes s0 for the whole function), "r = row + 0", a u32 r, "t = row * 64" in the
 * outer body, "tb = (t = row * 64) + 64", t after the sr clamp or after gDPPipeSync, row * 64
 * inline: no reload (scratch compiles). r declared last with pad kept: frame 0xC0 (22 locals
 * + 11 temps). Left: one local more and one compiler temp fewer than this C.
 * On this C (6): inlines, joins, copies, empties (57), splits, pads, unfakes: nothing under 6.
 * Scratch compiles (frame and the nx copy's slot read off each): the late copy also works as
 * "r = row / 1", "^ 0", "<< 0", "r = row * 64; t = r * 1", "64 * r", "r << 6" (row - 0, | 0,
 * & -1, (row * 1) * 64 inside: folded early, no reload); every one keeps 11 temps. A 22nd
 * local named for xl, yl, xh, yh, s << 5, t << 5, tb - 1, s, 1 / scale, dsdx (inner, outer,
 * u16), y + t * scale or s * scale + x: frame 0xC0 each, the temps stay 11. Other forms of
 * the body (sr = 64 + s, sr > w, tb = 64 + t, sr = s; sr += 64, s * 32 or 32 * s, (u16)/(s32)/
 * (u32) on s << 5, -1 + sr, sr + -1, col * 64 for uls, x + s * scale, statement orders, nx
 * and ny as statements or ny first or nx in two steps): 11 temps. The 11 temps need both the
 * rectangle's "s << 5" and the inline "sr - 1": with either gone (0, or sr for lrs) the frame
 * drops to 0x90 with the nx copy alone; lrs = 0 alone leaves 10 (one temp is tied to sr - 1).
 * A second column counter, "for (col = 0, i = 0; col < nx; col++, i++)" with s = i * 64 (i
 * declared last): frame 0xB8 with the nx copy at 0x5C as the game, but IDO keeps i (s0) and
 * tests it, so the colouring rotates: 132 lines. Also no: a second row counter (3 more
 * instructions), FAKE empty or address tests of nx, ny, w, h, row, col, s, t, sr, tb after
 * the loops (frame or length off). */
Gfx *pakFilerDrawI4Tiles(Gfx *gfx, f32 x, f32 y, f32 scale, s32 w, s32 h, u32 tex) {
    s32 tw = 64;
    s32 row;
    s32 nx = (w + 63) / 64;
    s32 ny = (h + 63) / 64;
    s32 col;
    s32 s;
    s32 t;
    s32 sr;
    s32 tb;
    s32 th = 64;

    for (row = 0; row < ny; row++) {
        for (col = 0; col < nx; col++) {
            t = row * th;
            s = col * tw;
            sr = s + tw;
            if (w < sr) {
                sr = w;
            }
            tb = t + th;
            if (h < tb) {
                tb = h;
            }
            gDPPipeSync(gfx++);
            gDPLoadTextureTile_4b(gfx++, tex, G_IM_FMT_I, w, h, s, t, sr - 1, tb - 1, 0, G_TX_CLAMP,
                                  G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gSPTextureRectangle(gfx++, (s32) ((s * scale + x) * 4.0f),
                                (s32) ((y + t * scale) * 4.0f), (s32) ((sr * scale + x) * 4.0f),
                                (s32) ((y + tb * scale) * 4.0f), G_TX_RENDERTILE, s << 5, t << 5,
                                (s32) ((1.0f / scale) * 1024.0f), (s32) ((1.0f / scale) * 1024.0f));
        }
    }
    gSPEndDisplayList(gfx);
    return gfx;
}

/* Which controllers have a Controller Pak in. */
s32 pakFilerIsPlug(u8 *pattern) {
    return osPfsIsPlug(&gSiMesgQueue, pattern);
}

/* A Pak character code in the font's ('*' for an unknown one). */
u8 pakFilerFontChar(u8 c) {
    s32 r;

    if (c < 0x42) {
        r = gPakFilerCharTable[c];
    } else {
        r = '*';
    }
    return r;
}

/* Copies a string, returning its end. */
u8 *pakFilerStrCopy(u8 *dst, u8 *src) {
    while (*src != 0) {
        *dst++ = *src++;
    }
    return dst;
}

/* Writes a string centred in a line of a width (the loop on one line, so n is incremented
 * before p, IDO_NOTES rule 12). */
void pakFilerStrCenter(u8 *line, s32 width, u8 *s) {
    s32 n = 0;
    u8 *p = s;

    while (*p++) { n++; }
    pakFilerStrCopy(line + (width - n) / 2, s);
}

/* Writes a number as three digits, leading zeros as spaces, returning the end. */
u8 *pakFilerNum3(u8 *dst, s32 n) {
    s32 d = 100;
    s32 lead = FALSE;
    s32 i;
    u8 c;

    for (i = 0; i < 3; i++) {
        c = n / d + '0';
        n %= d;
        d /= 10;
        if (c != '0') {
            lead = TRUE;
        }
        if (!lead && c == '0') {
            c = ' ';
        }
        *dst++ = c;
    }
    return dst;
}
