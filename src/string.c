#include "object.h"
#include "gfxmgr.h"
#include "heap.h"
#include "msgfrm.h"
#include "scenario.h"
#include "string_c.h"
#include "tmanager.h"

/* The message text ("strRun", scenario.c's strings).
 * - A string task lays characters out one frame each ("afont" frames, afontCreate), up to
 *   200, from its start 8 in from the window's place (variables 0x08000020/0x08000024).
 * - A character's cell is 12 x 12 with gaps of 2 unless a message sets them; a colour stays
 *   until changed.
 * - Negative codes are controls: -2 a new line, -3 half a space, -4 a space, -5 to -11 a
 *   colour, -16/-17 a value on and off, -18/-19 the cell size from the variables and back.
 * - Characters flagged to blink (variable 0x08000058) blink every 30 frames. */

/* A string's work. */
typedef struct StrWork {
    /* 0x00 */ Frame *frame; /* the parent's third frame, which the characters hang from */
    /* 0x04 */ StrChar *chars; /* 200 */
    /* 0x08 */ s16 wait; /* frames to the next character (variable 0x08000028) */
    /* 0x0A */ u8 count;
    /* 0x0C */ s16 x0; /* the start */
    /* 0x0E */ s16 y0;
    /* 0x10 */ s16 x; /* the next character's place */
    /* 0x12 */ s16 y;
    /* 0x14 */ Color3 color;
    /* 0x18 */ s16 var40; /* variable 0x08000040 while -16 is on */
    /* 0x1C */ s32 w; /* the cell */
    /* 0x20 */ s32 h;
    /* 0x24 */ s32 xgap;
    /* 0x28 */ s32 ygap;
    /* 0x2C */ s32 translucent;
    /* 0x30 */ s32 alpha;
} StrWork;

/* string.c's .data (0x80109860-0x8010987C), in ROM order. */
Color3 gStrColorWhite = { 0xFF, 0xFF, 0xFF }; /* white, the default */
Color3 gStrColorBlack = { 0x00, 0x00, 0x00 }; /* the colours of -11 to -6 */
Color3 gStrColorGray = { 0x80, 0x80, 0x80 };
Color3 gStrColorOrange = { 0xFF, 0x60, 0x10 };
Color3 gStrColorBlue = { 0x10, 0x60, 0xFF };
Color3 gStrColorYellow = { 0xFF, 0xFF, 0x00 };
Color3 gStrColorGreen = { 0x00, 0xFF, 0x00 };
extern u8 gFontGlyphs[][0x30]; /* the glyphs */
extern u8 gFontGlyphWidths[]; /* their widths, in twelfths of the cell */
extern s32 *gScVarWinX; /* the script variables (scenario.c) */
extern s32 *gScVarWinY;
extern s32 *gScVarTextWait;
extern s32 *gScVar40;
extern s32 *gScVarCharSize;
extern s32 *gScVarCharXGap;
extern s32 *gScVarCharYGap;
extern s32 *gScVarCharAlpha;
extern s32 *gScVar54;
extern s32 *gScVarCharBlink;
extern s32 *gScVarBlinkTimer;


#define STR(obj) ((StrWork *) ((u32) (obj) + sizeof(Obj)))

/* The handler. */
s32 strRun_proc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    StrWork *w;

    switch (msg) {
        case 2:
            obj->name = "strRun";
            w = STR(obj);
            if ((w->chars = heapMalloc(200 * sizeof(StrChar))) == NULL) {
                PRINTF("strRun_proc: err: メモリ確保エラー\n");
                return -1;
            }
            w->x0 = w->x = *gScVarWinX + 8;
            w->y0 = w->y = *gScVarWinY + 8;
            w->frame = scGetFrame(obj->parent, 2);
            w->color = gStrColorWhite;
            w->w = 12;
            w->h = 12;
            w->xgap = 2;
            w->ygap = 2;
            w->wait = *gScVarTextWait;
            w->var40 = 0;
            w->translucent = 0;
            w->alpha = 0xFF;
            break;
        case 1:
            _strClear(obj);
            heapFree(STR(obj)->chars);
            PRINTF("sc_string タスクを削除します。\n");
            break;
        case 0:
            strBlinkAll(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Starts and ends a string under the runner. */
Obj *strCreate(Obj *parent) {
    return tskCreate(parent, strRun_proc, 0, 0);
}

Obj *strDestroy(Obj *obj) {
    PRINTF("strDestroy: tskDestroy\n");
    return tskDestroy(obj);
}

/* A character's frame: a glyph of size s at (x, y) in a colour. */
FontFrame *_strInit(Frame *parent, void *glyph, s32 no, Color3 color, f32 s, f32 x,
                                  f32 y) {
    FontFrame *f;

    PRINTF("_strInit: string no[%d] s[%f] x[%d] y[%d]\n", no, s, (s32) x, (s32) y);
    if (glyph == NULL) {
        PRINTF("_strInit: Warning: font buffer が NULL です\n");
        return NULL;
    }
    f = afontCreate(parent);
    if (f == NULL) {
        PRINTF("_strInit: Warning: afont frame が作ることができません\n");
        return NULL;
    }
    f->x = x;
    f->y = y;
    f->size = s;
    f->glyph = glyph;
    f->color = color;
    return f;
}

/* Sets character c at the string's place. */
void _strSet(Obj *obj, s32 c) {
    StrWork *w = STR(obj);
    s32 pad[2];

    if (c < 0) {
        PRINTF("_strSet: 文字データが変です\n");
        return;
    }
    if (w->count >= 200) {
        PRINTF("_strSet: Warning: 文字列オーバーです\n");
        return;
    }
    w->chars[w->count].frame =
        _strInit(w->frame, gFontGlyphs[c], c, w->color, w->w, w->x, w->y);
    w->count++;
}

/* Adds a character or a control, with the look the variables set. */
void _strAdd(Obj *obj, s32 c) {
    StrWork *w = STR(obj);

    if (obj == NULL) {
        PRINTF("_strAdd: Error: No sc_string Task\n");
        return;
    }
    strSetSize(obj, *gScVarCharSize, *gScVarCharXGap, *gScVarCharYGap);
    strSetAlpha(obj, *gScVarCharAlpha);
    strSetCharFlags(obj);
    strSetBlinkTimer(obj, *gScVarBlinkTimer);
    switch (c) {
        default:
            _strSet(obj, c);
            /* fallthrough */
        case -4:
            w->x += gFontGlyphWidths[c] * (f32) w->w / 12.0f + w->xgap;
            break;
        case -11:
        case -10:
        case -9:
        case -8:
        case -7:
        case -6:
        case -5:
            strSetColor(obj, c);
            break;
        case -16:
            w->var40 = *gScVar40;
            break;
        case -17:
            w->var40 = 0;
            break;
        case -18:
            strSetSize(obj, *gScVarCharSize, *gScVarCharXGap, *gScVarCharYGap);
            break;
        case -19:
            strSetSize(obj, 12, 2, 2);
            break;
        case -3:
            w->x += (w->w + w->xgap) / 2;
            break;
        case -2:
            w->x = w->x0;
            w->y += w->h + w->ygap;
            break;
    }
}

/* Adds c when the wait runs out; true when it did. */
s32 strAddByTimer(Obj *obj, s32 c) {
    s32 added;
    StrWork *w = STR(obj);

    if (obj == NULL) {
        PRINTF("strAddByTimer: Error: No sc_string Task\n");
        return TRUE;
    }
    w->wait--;
    if (w->wait <= 0) {
        _strAdd(obj, c);
        w->wait = *gScVarTextWait;
        return TRUE;
    }
    return FALSE;
}

/* Clears the string: frees every character and goes back to the start. */
void _strClear(Obj *obj) {
    StrWork *w = STR(obj);
    s32 i;

    if (obj == NULL) {
        PRINTF("_strClear: Error: No sc_string Task\n");
        return;
    }
    for (i = 0; i < w->count; i++) {
        if (w->chars[i].frame != NULL) {
            w->chars[i].frame = (FontFrame *) frmDestroy((Frame *) w->chars[i].frame);
        }
    }
    w->count = 0;
    w->x = w->x0;
    w->y = w->y0;
}

/* The next character's place. */
StrPos strGetPos(Obj *obj) {
    s32 pad1;
    s32 pad2;
    StrPos p;

    p.x = STR(obj)->x;
    p.y = STR(obj)->y;
    return p;
}

/* Sets a colour: -11 to -6, else white. (Its cases are 0x800E9EA4-0x800E9F7F, after the
 * default's `jr ra`: symbol_addrs.txt gives the function its size so splat keeps them in it.) */
void strSetColor(Obj *obj, s32 c) {
    switch (c) {
        case -5:
        default:
            STR(obj)->color = gStrColorWhite;
            break;
        case -11:
            STR(obj)->color = gStrColorBlack;
            break;
        case -6:
            STR(obj)->color = gStrColorGray;
            break;
        case -8:
            STR(obj)->color = gStrColorOrange;
            break;
        case -9:
            STR(obj)->color = gStrColorBlue;
            break;
        case -7:
            STR(obj)->color = gStrColorYellow;
            break;
        case -10:
            STR(obj)->color = gStrColorGreen;
            break;
    }
}

/* The cell size and gaps. */
void strSetSize(Obj *obj, s32 size, s32 xgap, s32 ygap) {
    STR(obj)->w = STR(obj)->h = size;
    STR(obj)->xgap = xgap;
    STR(obj)->ygap = ygap;
}

/* The alpha; below 255 is translucent. */
void strSetAlpha(Obj *obj, s32 alpha) {
    STR(obj)->alpha = alpha;
    if (alpha >= 0xFF) {
        STR(obj)->translucent = FALSE;
    } else {
        STR(obj)->translucent = TRUE;
    }
}

/* The next character's flags, from the variables. */
void strSetCharFlags(Obj *obj) {
    StrChar *c = &STR(obj)->chars[STR(obj)->count];

    if (*gScVar54 == 1) {
        c->flags |= 1;
    } else {
        c->flags &= ~1;
    }
    if (*gScVar40 == 1) {
        c->flags |= 2;
    } else {
        c->flags &= ~2;
    }
    if (*gScVarCharBlink == 1) {
        c->flags |= 4;
    } else {
        c->flags &= ~4;
    }
}

/* The next character's blink timer. */
void strSetBlinkTimer(Obj *obj, s32 timer) {
    STR(obj)->chars[STR(obj)->count].timer = timer & 0xFF;
}

/* Blinks the characters. */
void strBlinkAll(Obj *obj) {
    StrWork *w = STR(obj);
    s32 i;

    for (i = 0; i < w->count; i++) {
        strBlinkChar(&w->chars[i]);
    }
}

void strBlinkChar(StrChar *c) {
    if ((c->flags & 4) && ++c->timer % 30 == 0) {
        c->timer = 0;
        if (c->frame->flags & 1) {
            c->frame->flags &= ~1;
        } else {
            c->frame->flags |= 1;
        }
    }
}
