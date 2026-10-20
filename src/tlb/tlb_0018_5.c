#include "tlb.h"
#include "tlb/tlb_0018.h"

/* The title screen: its background ("tback", "monoback"), the logo ("tlogo"), the copyright line
 * ("copyright") and the buttons ("tbutton"), with the picture drawers the scene's other files
 * use. */


extern Gfx titlePictureDl[]; /* the pictures' render setup (tlb_0018_4's data) */
extern Gfx titleTintDl[]; /* the tinted picture's render setup (tlb_0018_4's data) */
extern s32 titleTexFmt; /* the pictures' texel format (tlb_0018_7's data): G_IM_FMT_I */

/* A 16-bit picture, width x height texels, drawn at (x, y) at a scale and an alpha in 64 x 32
 * tiles (opaque at 255, translucent below); the list ends there (the returned pointer is the end
 * command's). */
/* Matched 2026-10-12 (from real 33): the load as one gDPLoadTextureTile (real 9: the
 * stores' schedule now the game's), then the rectangle's step written inline twice,
 * (s32) (1.0f / scale * 1024.0f), with no step variable (MATCH: the three s-registers rotate
 * into the game's order). Measured on the way: step in the column loop (9), step and t0 both
 * there (16), pads (nothing better), fadeSegToRam's result in a local (102); with no word where
 * step was declared the frame is 0xA8 (26), so pad1 keeps it.
 * 2026-10-12: the two FAKE pads were the tile's width and height, tw = 64 and th = 32
 * (as titleDrawImage4b's match): still MATCH, no FAKE left. */
Gfx *titleDrawRgba16(Gfx *gfx, u32 file, f32 x, f32 y, f32 scale, s32 alpha, s32 width,
                            s32 height, u32 tex) {
    s32 tw;
    s32 row;
    s32 cols = (width + 63) / 64;
    s32 rows = (height + 31) / 32;
    s32 th;
    s32 col;
    s32 s0;
    s32 t0;
    s32 s1;
    s32 t1;

    gSPDisplayList(gfx++, fadeSegToRam(titlePictureDl, file));
    gDPSetEnvColor(gfx++, 0, 0, 0, alpha);
    if (alpha >= 255) {
        gDPSetRenderMode(gfx++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
    } else {
        gDPSetRenderMode(gfx++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    }
    tw = 64;
    th = 32;
    for (row = 0; row < rows; row++) {
        t0 = row * th;
        for (col = 0; col < cols; col++) {
            s0 = col * tw;
            s1 = s0 + tw;
            t1 = t0 + th;
            if (width < s1) {
                s1 = width;
            }
            if (height < t1) {
                t1 = height;
            }
            gDPPipeSync(gfx++);
            gDPLoadTextureTile(gfx++, tex, G_IM_FMT_RGBA, G_IM_SIZ_16b, width, height, s0, t0, s1 - 1, t1 - 1, 0,
                               G_TX_CLAMP, G_TX_CLAMP, 0, 0, 0, 0);
            gSPTextureRectangle(gfx++, (s32) ((x + s0 * scale) * 4.0f),
                                (s32) ((y + t0 * scale) * 4.0f), (s32) ((x + s1 * scale) * 4.0f),
                                (s32) ((y + t1 * scale) * 4.0f), G_TX_RENDERTILE, s0 << 5, t0 << 5,
                                (s32) (1.0f / scale * 1024.0f), (s32) (1.0f / scale * 1024.0f));
        }
    }
    gSPEndDisplayList(gfx);
    return gfx;
}

/* The tinted picture's render setup with the environment colour (r, g, b, a). */
Gfx *titleSetTint(Gfx *gfx, u32 file, s32 r, s32 g, s32 b, s32 a) {
    gSPDisplayList(gfx++, fadeSegToRam(titleTintDl, file));
    gDPSetEnvColor(gfx++, r, g, b, a);
    return gfx;
}

/* A 4-bit picture, width x height texels, drawn at (x, y) at a scale in 64 x 64 tiles; the list
 * ends there (the returned pointer is the end command's). */
/* Same instructions but for registers and stack slots (212 lines, mix 2). Frame and homes now
 * right (step and t0 set in the row loop: 222 -> 212). Still: the game stores row and reloads it
 * at once ("sw v1,0xB0; lw v0,0xB0") at the top of each row, its compiler temps sit at
 * 0x44/0x48/0x4C/0x5C (this 0x40/0x44/0x48/0x60), and its s-registers rank ult << 2 (s1) above
 * the column copy (s3), pipe sync (s4) and the 8b size (s5); here the 8b size is s1. Tried: s0/t0
 * inline (350, mix 6), s1/t1 as (col + 1) * 64 (no change), pads (best 220 before the row-loop
 * move), splits (nothing below 212), a pad after rows (228, frame 0xC0, temps right), s0 inline
 * with t0 in the row loop (348, mix 8), t0 back in the column loop with step in the row loop
 * (222: temps 0x44/0x48/0x4C right, frame 0xC0). The game's temps say t0 sits in the column
 * loop and its frame says two fewer named words; titleDrawRgba16 is its 16-bit twin.
 * 2026-10-12, the levers that matched the twin titleDrawRgba16: the load as one
 * gDPLoadTextureTile_4b (real 119 -> 25; the macro's render tile is G_IM_SIZ_4b and the game
 * agrees), then the step written inline twice with no step assignment (18, kept; step's
 * declaration stays as the frame's word). Left: the game stores row at the row loop's head and
 * reloads it at once (sw v1,0xB0; lw v0,0xB0) where this C shifts v1 (length -1), and the temps
 * one word low (0x40.. for 0x44..). From 18: t0 in the column loop (30, frame 0xC0, temps
 * right); that and step's declaration removed (29, frame 0xB8, homes 4 high). 2026-10-12
 * (second try), from 18: t0 and step both assigned in the column loop, step used twice (46, frame
 * 0xC0); t0 after s0 in the column loop, no step (29, temps 0x40/44/48/64, homes 0xAC-B4). In
 * every form the row value is shifted from v1 before the store; the game's sll reads the
 * reloaded home, so in the game row*64 is computed after row's spill point.
 * 2026-10-12 (third try), from 18: t0 = *(s32 *) &row * 64 (a FAKE home): 72, row
 * then lives in memory throughout (the game keeps it in v1 outside the column loop).
 * MATCHED 2026-10-12 (from 18): the tile's size is two variables, tw (first, in step's
 * old word) and th (after rows), both set to 64 before the loops and used for s0, s1, t0 and
 * t1. The permuter found one variable (new_var = 64 for row * 64 and both + 64: 24 lines);
 * step as that variable: 5 (temps 0x3C/0x60, game 0x38/0x5C); a new n after rows with step
 * unused: 6 (temps 0x40-0x48, game 0x44-0x4C); n with no step: 13 (homes 4 high). Both words
 * as the tile's width and height: MATCH. */
Gfx *titleDrawImage4b(Gfx *gfx, f32 x, f32 y, f32 scale, s32 width, s32 height, u32 tex) {
    s32 tw;
    s32 row;
    s32 cols = (width + 63) / 64;
    s32 rows = (height + 63) / 64;
    s32 th;
    s32 col;
    s32 s0;
    s32 t0;
    s32 s1;
    s32 t1;

    tw = 64;
    th = 64;
    for (row = 0; row < rows; row++) {
        t0 = row * th;
        for (col = 0; col < cols; col++) {
            s0 = col * tw;
            s1 = s0 + tw;
            t1 = t0 + th;
            if (width < s1) {
                s1 = width;
            }
            if (height < t1) {
                t1 = height;
            }
            gDPPipeSync(gfx++);
            gDPLoadTextureTile_4b(gfx++, tex, titleTexFmt, width, height, s0, t0, s1 - 1, t1 - 1, 0,
                                  G_TX_CLAMP, G_TX_CLAMP, 0, 0, 0, 0);
            gSPTextureRectangle(gfx++, (s32) ((x + s0 * scale) * 4.0f),
                                (s32) ((y + t0 * scale) * 4.0f), (s32) ((x + s1 * scale) * 4.0f),
                                (s32) ((y + t1 * scale) * 4.0f), G_TX_RENDERTILE, s0 << 5, t0 << 5,
                                (s32) (1.0f / scale * 1024.0f), (s32) (1.0f / scale * 1024.0f));
        }
    }
    gSPEndDisplayList(gfx);
    return gfx;
}

/* titleDrawImage4b for a colour-indexed picture (its palette loaded before). */
Gfx *titleDrawImageCI4(Gfx *gfx, f32 x, f32 y, f32 scale, s32 width, s32 height, u32 tex) {
    titleTexFmt = G_IM_FMT_CI;
    gfx = titleDrawImage4b(gfx, x, y, scale, width, height, tex);
    titleTexFmt = G_IM_FMT_I;
    return gfx;
}

void titleUnusedStub1(void) {
}

void titleUnusedStub2(void) {
}

/* Segment addresses (file 0x109): the title's background pictures. */
extern u8 titleBack0Tex[];
extern u8 titleBack1Tex[];
extern u8 titleBack2Tex[];

/* "tback": the title's background, two pictures over each other (the second fading in over the
 * first) and a third tinted one fading in over both, its colours moving to a target in steps. */
typedef struct TbackWork {
    /* 0x00 */ DLFrame *frames[3];
    /* 0x0C */ s16 steps;
    /* 0x0E */ s16 alpha; /* the second picture's */
    /* 0x10 */ s16 alphaStep;
    /* 0x12 */ s16 tintAlpha; /* the tinted picture's */
    /* 0x14 */ s16 tintAlphaStep;
    /* 0x16 */ u8 pad16[2];
    /* 0x18 */ u32 tex[3];
    /* 0x24 */ f32 r; /* the primitive colour, each with its step */
    /* 0x28 */ f32 vr;
    /* 0x2C */ f32 g;
    /* 0x30 */ f32 vg;
    /* 0x34 */ f32 b;
    /* 0x38 */ f32 vb;
    /* 0x3C */ s16 targetR;
    /* 0x3E */ s16 targetG;
    /* 0x40 */ s16 targetB;
    /* 0x42 */ s16 envTargetR;
    /* 0x44 */ s16 envTargetG;
    /* 0x46 */ s16 envTargetB;
    /* 0x48 */ f32 envR; /* the environment colour, each with its step */
    /* 0x4C */ f32 envVr;
    /* 0x50 */ f32 envG;
    /* 0x54 */ f32 envVg;
    /* 0x58 */ f32 envB;
    /* 0x5C */ f32 envVb;
} TbackWork; /* size 0x60 */

#define TBACK(obj) ((TbackWork *) FX(obj))

s32 titleTbackProc(Obj *obj, s16 msg, void *arg) {
    TbackWork *w = TBACK(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "tback";
            w->frames[0] = (DLFrame *) frmCreate(gFrmLayers[0], gFrmKindDlist);
            w->frames[1] = (DLFrame *) frmCreate(gFrmLayers[0], gFrmKindDlist);
            w->frames[2] = (DLFrame *) frmCreate(gFrmLayers[0], gFrmKindDlist);
            if (w->frames[0] == NULL || w->frames[1] == NULL || w->frames[2] == NULL) {
                return -1;
            }
            w->steps = 16;
            w->alpha = 0;
            w->tex[0] = floadSegToRam((u32) titleBack0Tex, 0x109, 0);
            w->tex[1] = floadSegToRam((u32) titleBack1Tex, 0x109, 0);
            w->tex[2] = floadSegToRam((u32) titleBack2Tex, 0x109, 0);
            w->r = 255.0f;
            w->g = 224.0f;
            w->b = 160.0f;
            w->envR = 80.0f;
            w->envG = 48.0f;
            w->envB = 16.0f;
            break;
        case 1:
            frmDestroy(&w->frames[2]->f);
            frmDestroy(&w->frames[1]->f);
            frmDestroy(&w->frames[0]->f);
            break;
        case 0:
            w->alpha += w->alphaStep;
            if (w->alpha < 0) {
                w->alpha = 0;
            } else if (w->alpha > 255) {
                w->alpha = 255;
            }
            w->tintAlpha += w->tintAlphaStep;
            if (w->tintAlpha < 0) {
                w->tintAlpha = 0;
            } else if (w->tintAlpha > 255) {
                w->tintAlpha = 255;
            }
            if (w->vr != 0.0f) {
                if (fabsf(w->r - w->targetR) <= fabsf(w->vr)) {
                    w->r = w->targetR;
                    w->vr = 0.0f;
                } else {
                    w->r += w->vr;
                }
            }
            if (w->vg != 0.0f) {
                if (fabsf(w->g - w->targetG) <= fabsf(w->vg)) {
                    w->g = w->targetG;
                    w->vg = 0.0f;
                } else {
                    w->g += w->vg;
                }
            }
            if (w->vb != 0.0f) {
                if (fabsf(w->b - w->targetB) <= fabsf(w->vb)) {
                    w->b = w->targetB;
                    w->vb = 0.0f;
                } else {
                    w->b += w->vb;
                }
            }
            break;
        case 3:
            if (w->alpha < 255 && w->tintAlpha < 255) {
                w->frames[0]->dl = gGfxDlHead;
                gGfxDlHead = titleDrawRgba16(gGfxDlHead, obj->file, 16.0f, 8.0f, 1.0f, 255,
                                                    0x120, 0xE0, w->tex[0]);
                gGfxDlHead++;
            } else {
                w->frames[0]->dl = NULL;
            }
            if (w->alpha > 0 && w->tintAlpha < 255) {
                w->frames[1]->dl = gGfxDlHead;
                gGfxDlHead = titleDrawRgba16(gGfxDlHead, obj->file, 16.0f, 8.0f, 1.0f,
                                                    w->alpha, 0x120, 0xE0, w->tex[1]);
                gGfxDlHead++;
            } else {
                w->frames[1]->dl = NULL;
            }
            if (w->tintAlpha > 0) {
                w->frames[2]->dl = gGfxDlHead;
                gSPDisplayList(gGfxDlHead++, fadeSegToRam(titleTintDl, obj->file));
                gDPSetCombine(gGfxDlHead++, 0x30FE61, 0x55FEFB7D);
                gDPSetPrimColor(gGfxDlHead++, 0, 0, (s32) w->r, (s32) w->g, (s32) w->b, 255);
                gDPSetEnvColor(gGfxDlHead++, (s32) w->envR, (s32) w->envG, (s32) w->envB,
                               w->tintAlpha);
                gGfxDlHead = titleDrawImage4b(gGfxDlHead, 16.0f, 8.0f, 1.0f, 0x120, 0xE0,
                                                    w->tex[2]);
                gGfxDlHead++;
            } else {
                w->frames[2]->dl = NULL;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *titleTbackMake(Obj *obj) {
    return tskCreate(obj, titleTbackProc, obj->file, NULL);
}

/* The second picture fades in, in steps. */
void titleTbackFadeInSecond(Obj *obj, s32 steps) {
    TbackWork *w = TBACK(obj);

    w->steps = steps;
    w->alpha = 0;
    w->alphaStep = 256 / w->steps;
}

/* The tinted picture fades in, in steps. */
void titleTbackTintIn(Obj *obj, s32 steps) {
    TbackWork *w = TBACK(obj);

    w->steps = steps;
    w->tintAlpha = 0;
    w->tintAlphaStep = 256 / w->steps;
}

/* The tinted picture fades out, in steps. */
void titleTbackTintOut(Obj *obj, s32 steps) {
    TbackWork *w = TBACK(obj);

    w->steps = steps;
    w->tintAlpha = 255;
    w->tintAlphaStep = -(256 / w->steps);
}

void titleTbackSetSteps(Obj *obj, s32 steps) {
    TBACK(obj)->steps = steps;
}

/* The tint's colours move to (r, g, b) and (envR, envG, envB) in the steps. */
void titleTbackTintTo(Obj *obj, s32 r, s32 g, s32 b, s32 envR, s32 envG, s32 envB) {
    TbackWork *w = TBACK(obj);

    w->targetR = r;
    w->vr = (w->targetR - w->r) / w->steps;
    w->targetG = g;
    w->vg = (w->targetG - w->g) / w->steps;
    w->targetB = b;
    w->vb = (w->targetB - w->b) / w->steps;
    w->envTargetR = envR;
    w->envTargetG = envG;
    w->envTargetB = envB;
    w->envVr = (w->envTargetR - w->envR) / w->steps;
    w->envVg = (w->envTargetG - w->envG) / w->steps;
    w->envVb = (w->envTargetB - w->envB) / w->steps;
}

/* "monoback": the tinted picture alone, fading in. */
typedef struct MonobackWork {
    /* 0x00 */ s32 steps;
    /* 0x04 */ DLFrame *frame;
    /* 0x08 */ s32 alpha;
    /* 0x0C */ s32 alphaStep;
    /* 0x10 */ u32 tex;
} MonobackWork;

#define MONOBACK(obj) ((MonobackWork *) FX(obj))

s32 titleMonobackProc(Obj *obj, s16 msg, void *arg) {
    MonobackWork *w = MONOBACK(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "monoback";
            w->frame = (DLFrame *) frmCreate(gFrmLayers[10], gFrmKindDlist);
            if (w->frame == NULL) {
                return -1;
            }
            w->steps = 16;
            w->alpha = 0;
            w->tex = floadSegToRam((u32) titleBack2Tex, 0x109, 0);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->alpha += w->alphaStep;
            if (w->alpha < 0) {
                w->alpha = 0;
            } else if (w->alpha > 255) {
                w->alpha = 255;
            }
            break;
        case 3:
            if (w->alpha > 0) {
                w->frame->dl = gGfxDlHead;
                gSPDisplayList(gGfxDlHead++, fadeSegToRam(titleTintDl, obj->file));
                gDPSetCombine(gGfxDlHead++, 0x30FE61, 0x55FEFB7D);
                gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 224, 160, 255);
                gDPSetEnvColor(gGfxDlHead++, 80, 48, 16, w->alpha);
                gGfxDlHead = titleDrawImage4b(gGfxDlHead, 16.0f, 8.0f, 1.0f, 0x120, 0xE0,
                                                    w->tex);
                gGfxDlHead++;
            } else {
                w->frame->dl = NULL;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *titleMonobackMake(Obj *obj) {
    return tskCreate(obj, titleMonobackProc, obj->file, NULL);
}

/* It fades in, in steps. */
void titleMonobackFadeIn(Obj *obj, s32 steps) {
    MonobackWork *w = MONOBACK(obj);

    w->steps = steps;
    w->alpha = 0;
    w->alphaStep = 256 / steps;
}

void titleMonobackSetLayer(Obj *obj, Frame *layer) {
    frmMove(&MONOBACK(obj)->frame->f, layer);
}

/* Segment addresses (file 0x109): the logo, the copyright line, the buttons. */
extern u8 titleLogoTex[];
extern u8 titleCopyrightTex[];
extern u8 titleButtonTex[];
extern u8 titleButtonKind9Tex[];
extern u8 diaryPal[];

/* "tlogo": the title logo, 269 x 102, drawn around its centre at a scale, fading by a step. */
typedef struct TlogoWork {
    /* 0x00 */ f32 steps;
    /* 0x04 */ f32 x;
    /* 0x08 */ f32 y;
    /* 0x0C */ f32 scale;
    /* 0x10 */ DLFrame *frame;
    /* 0x14 */ s32 alpha;
    /* 0x18 */ s32 alphaStep;
} TlogoWork;

#define TLOGO(obj) ((TlogoWork *) FX(obj))

s32 titleTlogoProc(Obj *obj, s16 msg, void *arg) {
    s32 left = -134; /* the logo's corner from its centre */
    s32 top = -53;
    s32 width = 269;
    s32 height = 102;
    s32 pad;
    TlogoWork *w = TLOGO(obj);
    TlogoWork *init = arg;

    switch (msg) {
        case 2:
            obj->name = "tlogo";
            w->x = init->x;
            w->y = init->y;
            w->scale = init->scale;
            w->frame = (DLFrame *) frmCreate(gFrmLayer2, gFrmKindDlist);
            if (w->frame == NULL) {
                return -1;
            }
            w->alpha = 0;
            w->steps = 16.0f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->alpha += w->alphaStep;
            if (w->alpha < 0) {
                w->alpha = 0;
            } else if (w->alpha > 255) {
                w->alpha = 255;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gGfxDlHead = titleDrawRgba16(gGfxDlHead, obj->file, w->x + left * w->scale,
                                                w->y + top * w->scale, w->scale, w->alpha, width,
                                                height, floadSegToRam((u32) titleLogoTex, 0x109, 0));
            gGfxDlHead++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *titleTlogoMake(Obj *parent, s32 x, s32 y, s32 scale) {
    TlogoWork init;

    init.x = x;
    init.y = y;
    init.scale = scale;
    return tskCreate(parent, titleTlogoProc, parent->file, &init);
}

/* The logo fades in, in steps. */
void titleTlogoFadeIn(Obj *obj, s32 steps) {
    TlogoWork *w = TLOGO(obj);

    w->steps = steps;
    w->alpha = 0;
    w->alphaStep = 256.0f / w->steps;
}

/* The logo fades out, in steps. */
void titleTlogoFadeOut(Obj *obj, s32 steps) {
    TLOGO(obj)->alphaStep = -(256 / steps);
}

s32 titleTlogoGetAlpha(Obj *obj) {
    return TLOGO(obj)->alpha;
}

/* "copyright": the copyright line, 280 x 30 at (20, 199). */
typedef struct CopyrightWork {
    /* 0x00 */ DLFrame *frame;
    /* 0x04 */ s32 alpha;
    /* 0x08 */ s32 alphaStep;
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ u32 file;
} CopyrightWork;

#define COPYRIGHT(obj) ((CopyrightWork *) FX(obj))

s32 titleCopyrightProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    CopyrightWork *w = COPYRIGHT(obj);

    switch (msg) {
        case 2:
            obj->name = "copyright";
            w->frame = (DLFrame *) frmCreate(gFrmLayer2, gFrmKindDlist);
            if (w->frame == NULL) {
                return -1;
            }
            w->alpha = 0;
            w->x = 20.0f;
            w->y = 199.0f;
            w->file = floadGetSetFile(0x109, 0);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->alpha += w->alphaStep;
            if (w->alpha < 0) {
                w->alpha = 0;
            } else if (w->alpha > 255) {
                w->alpha = 255;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gGfxDlHead = titleDrawRgba16(gGfxDlHead, obj->file, w->x, w->y, 1.0f, 255, 280,
                                                30, floadSegToRam((u32) titleCopyrightTex, 0x109, 0));
            gGfxDlHead++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *titleCopyrightMake(Obj *obj) {
    return tskCreate(obj, titleCopyrightProc, obj->file, NULL);
}

void titleCopyrightFadeIn(Obj *obj, s32 steps) {
    COPYRIGHT(obj)->alphaStep = 256 / steps;
}

void titleCopyrightFadeOut(Obj *obj, s32 steps) {
    COPYRIGHT(obj)->alphaStep = -(256 / steps);
}

/* The settings at 0x800881F8 (fade.c's Settings); byte 8 is the language: 0 English, 1 French,
 * 2 German. */
typedef struct TitleSettings {
    /* 0x00 */ u8 pad0[8];
    /* 0x08 */ s8 language;
    /* 0x09 */ u8 pad9[7];
} TitleSettings;
extern TitleSettings gSaveSettings;

/* The buttons' words, one per language (tlb_0018_7's data). */
extern char *titleStartText[3]; /* "Start" */
extern char *titleOptionText[3]; /* "Option" */
extern char *titleStereoText[3]; /* "Stereo" */
extern char *titleMonauralText[3]; /* "Monaural" */
extern char *titleLanguageText[3]; /* "Language" */
extern char *titleSoundText[3]; /* "Sound" */
extern char *titleEnglishText[3]; /* "English" */
extern char *titleFrenchText[3]; /* "French" */
extern char *titleGermanText[3]; /* "German" */


/* "tbutton": a title button, a plate with a word on it (kinds 1-9) or a picture plate (kinds 10
 * and up); the word blinks through colours while it is chosen. */
typedef struct TbuttonWork {
    /* 0x00 */ DLFrame *frame;
    /* 0x04 */ TextFrame *text;
    /* 0x08 */ s32 alpha;
    /* 0x0C */ s32 alphaStep;
    /* 0x10 */ s32 kind;
    /* 0x14 */ f32 x;
    /* 0x18 */ f32 y;
    /* 0x1C */ u32 file;
    /* 0x20 */ s32 r;
    /* 0x24 */ s32 g;
    /* 0x28 */ s32 b;
    /* 0x2C */ u16 timer;
    /* 0x2E */ u8 blink;
} TbuttonWork; /* size 0x30 */

#define TBUTTON(obj) ((TbuttonWork *) FX(obj))

s32 titleTbuttonProc(Obj *obj, s16 msg, void *arg) {
    TbuttonWork *w = TBUTTON(obj);
    TbuttonWork *init = arg;
    s32 width;
    s32 height;
    s32 pad[3]; /* the game's frame has three more words, never touched */

    switch (msg) {
        case 2:
            obj->name = "tbutton";
            w->kind = init->kind;
            w->x = init->x;
            w->y = init->y;
            w->frame = (DLFrame *) frmCreate(gFrmLayer2, gFrmKindDlist);
            if (w->frame == NULL) {
                return -1;
            }
            w->text = frmFontCreate(&w->frame->f, 0, 0, NULL);
            if (w->text == NULL) {
                return -1;
            }
            w->alpha = 0;
            w->file = floadGetSetFile(0x109, 0);
            w->r = 255;
            w->g = 255;
            w->b = 255;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->timer++;
            w->alpha += w->alphaStep;
            if (w->alpha < 0) {
                w->alpha = 0;
            } else if (w->alpha > 255) {
                w->alpha = 255;
            }
            if (w->blink) {
                switch ((w->timer >> 1) & 3) {
                    case 0:
                        w->r = 0;
                        w->g = 255;
                        w->b = 0;
                        break;
                    case 1:
                        w->r = 255;
                        w->g = 255;
                        w->b = 0;
                        break;
                    case 2:
                        w->r = 255;
                        w->g = 255;
                        w->b = 255;
                        break;
                    case 3:
                        w->r = 0;
                        w->g = 0;
                        w->b = 0;
                        break;
                }
            } else {
                w->r = 255;
                w->g = 255;
                w->b = 255;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            if (w->kind < 10) {
                width = 80;
                height = 16;
                gGfxDlHead = titleDrawRgba16(
                    gGfxDlHead, obj->file, w->x - width / 2, w->y - height / 2, 1.0f, w->alpha,
                    width, height, fadeSegToRam(titleButtonTex, w->file));
                gGfxDlHead++;
                if (w->text != NULL) {
                    switch (w->kind) {
                        case 1:
                            diaryTextCenter(w->text, w->x, w->y, 8, 12,
                                                   titleStartText[gSaveSettings.language]);
                            break;
                        case 2:
                            diaryTextCenter(w->text, w->x, w->y, 8, 12,
                                                   titleOptionText[gSaveSettings.language]);
                            break;
                        case 3:
                            diaryTextCenter(w->text, w->x, w->y, 8, 12,
                                                   titleStereoText[gSaveSettings.language]);
                            break;
                        case 4:
                            diaryTextCenter(w->text, w->x, w->y, 8, 12,
                                                   titleMonauralText[gSaveSettings.language]);
                            break;
                        case 5:
                            diaryTextCenter(w->text, w->x, w->y, 8, 12,
                                                   titleLanguageText[gSaveSettings.language]);
                            break;
                        case 6:
                            diaryTextCenter(w->text, w->x, w->y, 8, 12,
                                                   titleSoundText[gSaveSettings.language]);
                            break;
                        case 7:
                            diaryTextCenter(w->text, w->x, w->y, 8, 12,
                                                   titleEnglishText[gSaveSettings.language]);
                            break;
                        case 8:
                            diaryTextCenter(w->text, w->x, w->y, 8, 12,
                                                   titleFrenchText[gSaveSettings.language]);
                            break;
                        case 9:
                            diaryTextCenter(w->text, w->x, w->y, 8, 12,
                                                   titleGermanText[gSaveSettings.language]);
                            break;
                    }
                    w->text->r = w->r;
                    w->text->g = w->g;
                    w->text->b = w->b;
                    w->text->a = w->alpha;
                }
            } else {
                diarySegLoad((u32) diaryPal, 0x10B);
                width = 56;
                height = 16;
                gGfxDlHead = titleDrawRgba16(
                    gGfxDlHead, obj->file, w->x - width / 2, w->y - height / 2, 1.0f, w->alpha,
                    width, height, fadeSegToRam(titleButtonKind9Tex, w->file));
                gGfxDlHead = titleSetTint(gGfxDlHead, obj->file, w->r, w->g, w->b,
                                                    w->alpha);
                gGfxDlHead = diaryDrawTile4b(gGfxDlHead, w->x - width / 2 + 20.0f,
                                                    w->y - height / 2 + 2.0f, 22, 12,
                                                    (w->kind - 10) * 24, 111);
                gGfxDlHead++;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *titleTbuttonMake(Obj *parent, s32 kind, f32 x, f32 y) {
    TbuttonWork init;

    init.kind = kind;
    init.x = x;
    init.y = y;
    return tskCreate(parent, titleTbuttonProc, parent->file, &init);
}

/* The button fades in, in steps. */
void titleTbuttonFadeIn(Obj *obj, s32 steps) {
    TBUTTON(obj)->alphaStep = 256 / steps;
}

/* The button fades out, in steps. */
void titleTbuttonFadeOut(Obj *obj, s32 steps) {
    TBUTTON(obj)->alphaStep = -(256 / steps);
}

/* The word blinks (on 1) or stays white. */
void titleTbuttonSetBlink(Obj *obj, s32 on) {
    if (on == 1) {
        TBUTTON(obj)->blink = 1;
    } else {
        TBUTTON(obj)->blink = 0;
    }
}

const char titleStartTextEn[] = "Start";

const char titleStartTextFr[] = "START";

const char titleStartTextDe[] = "start";

const char titleOptionTextEn[] = "Option";

const char titleOptionTextFr[] = "OPTION";

const char titleOptionTextDe[] = "option";

const char titleStereoTextEn[] = "Stereo";

const char titleStereoTextFr[] = "STEREO";

const char titleStereoTextDe[] = "stereo";

const char titleMonauralTextEn[] = "Monaural";

const char titleMonauralTextFr[] = "MONAURAL";

const char titleMonauralTextDe[] = "monaural";

const char titleLanguageTextEn[] = "Language";

const char titleLanguageTextFr[] = "LANGUAGE";

const char titleLanguageTextDe[] = "language";

const char titleSoundTextEn[] = "Sound";

const char titleSoundTextFr[] = "SOUND";

const char titleSoundTextDe[] = "sound";

const char titleEnglishTextEn[] = "English";

const char titleEnglishTextFr[] = "ENGLISH";

const char titleEnglishTextDe[] = "english";

const char titleFrenchTextEn[] = "French";

const char titleFrenchTextFr[] = "FRENCH";

const char titleFrenchTextDe[] = "french";

const char titleGermanTextEn[] = "German";

const char titleGermanTextFr[] = "GERMAN";

const char titleGermanTextDe[] = "german";
