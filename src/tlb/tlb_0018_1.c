#include "tlb.h"
#include "tlb/tlb_0018.h"

/* A picture that moves, scales and fades to targets in steps: the "siori" (bookmark) task's work
 * (its helpers follow its proc, diarySioriProc), and the same layout for the task whose helpers
 * start at diaryDcursorSetSteps (the journal's cursor). */
typedef struct SioriWork {
    /* 0x00 */ f32 steps;
    /* 0x04 */ f32 x;
    /* 0x08 */ f32 y;
    /* 0x0C */ f32 width;
    /* 0x10 */ f32 height;
    /* 0x14 */ f32 vx;
    /* 0x18 */ f32 vy;
    /* 0x1C */ f32 targetX;
    /* 0x20 */ f32 targetY;
    /* 0x24 */ f32 scale;
    /* 0x28 */ f32 scaleStep;
    /* 0x2C */ f32 scaleTarget;
    /* 0x30 */ Unk88208 *save; /* a journal slot's save data (tlb_0018_0 loads and copies it) */
    /* 0x34 */ PolFrame *frame; /* siori's tag */
    /* 0x38 */ PolFrame *frame2; /* and bookmark */
    /* 0x3C */ DLFrame *list; /* its colour */
    /* 0x40 */ DLFrame *list2; /* its summary */
    /* 0x44 */ TextFrame *text;
    /* 0x48 */ TextFrame *text2;
    /* 0x4C */ s8 slot; /* its journal slot (fadeWriteSave) */
    /* 0x4D */ u8 pad4D;
    /* 0x4E */ s16 r;
    /* 0x50 */ s16 g;
    /* 0x52 */ s16 b;
    /* 0x54 */ s16 a;
    /* 0x56 */ u8 pad56[0x2];
    /* 0x58 */ s32 result;
} SioriWork;

#define SIORI(o) ((SioriWork *) FX(o))

/* Sets a text frame's size and text and centres it on x, y. */
void diaryTextCenter(TextFrame *t, f32 x, f32 y, s16 w, s16 h, u32 text) {
    t->w = w;
    t->x = x;
    t->y = y;
    t->h = h;
    t->text = text;
    t->x -= frmFontGetWidth(t) / 2;
    t->y -= frmFontGetHeight(t) / 2;
}

s32 diaryReturnArg(s32 v) {
    return v;
}

void diaryUnusedStub(void) {
}

extern u32 diarySegRam; /* the address the segment below was loaded to */

/* Loads segment entry n at addr and keeps where it went. */
void diarySegLoad(u32 addr, s32 n) {
    diarySegRam = floadSegToRam(addr, n, 0);
}

extern s32 diaryTexFmt; /* the image format the texture loads use (2: CI, 4: IA) */

/* An older SDK's gDPLoadTextureTile_4b: its render tile's size counts from 0 (this libultra's
 * counts from uls, ult). */
#define gDPLoadTextureTile_4b_0(pkt, timg, fmt, width, height, uls, ult, lrs, lrt, pal, cms, cmt, masks,     \
                                maskt, shifts, shiftt)                                                        \
    {                                                                                                        \
        gDPSetTextureImage(pkt, fmt, G_IM_SIZ_8b, ((width) >> 1), timg);                                     \
        gDPSetTile(pkt, fmt, G_IM_SIZ_8b, (((((lrs) - (uls) + 1) >> 1) + 7) >> 3), 0, G_TX_LOADTILE, 0, cmt, \
                   maskt, shiftt, cms, masks, shifts);                                                       \
        gDPLoadSync(pkt);                                                                                    \
        gDPLoadTile(pkt, G_TX_LOADTILE, (uls) << (G_TEXTURE_IMAGE_FRAC - 1), (ult) << (G_TEXTURE_IMAGE_FRAC), \
                    (lrs) << (G_TEXTURE_IMAGE_FRAC - 1), (lrt) << (G_TEXTURE_IMAGE_FRAC));                   \
        gDPPipeSync(pkt);                                                                                    \
        gDPSetTile(pkt, fmt, G_IM_SIZ_4b, (((((lrs) - (uls) + 1) >> 1) + 7) >> 3), 0, G_TX_RENDERTILE, pal,  \
                   cmt, maskt, shiftt, cms, masks, shifts);                                                  \
        gDPSetTileSize(pkt, G_TX_RENDERTILE, 0, 0, ((lrs) - (uls)) << G_TEXTURE_IMAGE_FRAC,                  \
                       ((lrt) - (ult)) << G_TEXTURE_IMAGE_FRAC);                                              \
    }

/* Draws the w x h piece at (s, t) of the loaded 256-wide 4-bit image at (x, y), one texel a
 * pixel, and ends the list there: it returns the end command, for the next drawing to write
 * over. */
Gfx *diaryDrawTile4b(Gfx *gfx, f32 x, f32 y, s32 w, s32 h, s32 s, s32 t) {
    gDPPipeSync(gfx++);
    gDPLoadTextureTile_4b_0(gfx++, diarySegRam, diaryTexFmt, 256, 0, s, t, s + w - 1, t + h - 1,
                            0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    gSPTextureRectangle(gfx++, (s32) (x * 4.0f), (s32) (y * 4.0f), (s32) ((w + x) * 4.0f),
                        (s32) ((y + h) * 4.0f), G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
    gSPEndDisplayList(gfx);
    return gfx;
}

/* diaryDrawTile4b with the texel size 2, then 4 again. */
void diaryDrawTileCI4(Gfx *gfx, f32 x, f32 y, s32 a, s32 b, s32 c, s32 d) {
    diaryTexFmt = 2;
    diaryDrawTile4b(gfx, x, y, a, b, c, d);
    diaryTexFmt = 4;
}

/* Draws n with diaryDrawTile4b's 8 x 10 digit tiles (tile 64 + 8 * digit), from the
 * digit at `place` down, one every 8 across. Leading zeros are left out, but `place` 2 and 3
 * mean two and three digits with their zeros. */
Gfx *diaryDrawNumber(Gfx *gfx, f32 x, f32 y, s32 n, s32 place) {
    s32 shown = FALSE;
    s32 width = 8;
    s32 digit;

    if (place == 2) {
        shown = TRUE;
        place = 10;
    }
    if (place == 3) {
        shown = TRUE;
        place = 100;
    }
    for (; place > 0; place /= 10) {
        digit = n / place;
        n %= place;
        if (digit != 0 || shown || place == 1) {
            gfx = diaryDrawTile4b(gfx, x, y, width, 10, digit * 8 + 64, 75);
            shown = TRUE;
        }
        x += width;
    }
    return gfx;
}

extern u8 diaryGoemonIconTex[]; /* the characters' 16x16 icons (RGBA16) in segment 0x10B: Goemon, */
extern u8 diaryEbisumaruIconTex[]; /* Ebisumaru, */
extern u8 diarySasukeIconTex[]; /* Sasuke, */
extern u8 diaryYaeIconTex[]; /* Yae */

/* Draws character chara's icon at x, y (a 16x16 copy-mode rectangle). */
Gfx *diaryDrawCharaIcon(Gfx *gfx, s32 chara, f32 x, f32 y) {
    u8 *icon;

    switch (chara) {
        case 0:
        default:
            icon = diaryGoemonIconTex;
            break;
        case 1:
            icon = diaryEbisumaruIconTex;
            break;
        case 2:
            icon = diarySasukeIconTex;
            break;
        case 3:
            icon = diaryYaeIconTex;
            break;
    }
    gDPPipeSync(gfx++);
    gDPLoadTextureBlock(gfx++, floadSegToRam((u32) icon, 0x10B, 0), G_IM_FMT_RGBA, G_IM_SIZ_16b, 16, 16, 0,
                        G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK,
                        G_TX_NOLOD, G_TX_NOLOD);
    gSPTextureRectangle(gfx++, (s32) (x * 4.0f), (s32) (y * 4.0f), (s32) ((x + 16.0f - 1.0f) * 4.0f),
                        (s32) ((y + 16.0f - 1.0f) * 4.0f), G_TX_RENDERTILE, 0, 0, 4 << 10, 1 << 10);
    return gfx;
}

/* Puts a text frame at x, y with the name of save point `point` (1-11), or no text. */
void diarySetPointName(TextFrame *t, f32 x, f32 y, s32 point) {
    t->x = x;
    t->y = y;
    switch (point) {
        default:
            t->text = 0;
            break;
        case 1:
            t->text = (u32) "Lost'n Town:INN";
            break;
        case 2:
            t->text = (u32) "Otohime Town:INN";
            break;
        case 3:
            t->text = (u32) "Spook Village:INN";
            break;
        case 4:
            t->text = (u32) "Tree Stump Village:INN";
            break;
        case 5:
            t->text = (u32) "Floating World Town:INN";
            break;
        case 6:
            t->text = (u32) "Creep Village:INN";
            break;
        case 7:
            t->text = (u32) "Home of Wiseman:Front";
            break;
        case 8:
            t->text = (u32) "Edo:Complete";
            break;
        case 9:
            t->text = (u32) "Ryugu:Complete";
            break;
        case 10:
            t->text = (u32) "Mafu Island:Complete";
            break;
        case 11:
            t->text = (u32) "Underworld:Complete";
            break;
    }
}

extern Gfx diarySlotSetupDl[]; /* the slot's display list setups */
extern Gfx diaryLetterDl[];
extern u8 diaryPal[]; /* file 0x10B's palette */
extern u8 diaryPassIconTex[]; /* the pass and money icons */
extern u8 diaryMoneyIconTex[];

/* A journal slot's summary at x, y: "From the beginning" for an empty save; else the slot's
 * frame, its area's name, its lives, passes (tegata, summed) and money, the characters it has
 * (two to four portraits) and the two icons. Returns the list's end. The first portrait's 96 is
 * an integer: as 96.0f it shares the others' constant and temporary, and the frame is 8 larger. */
Gfx *diaryDrawSummary(Gfx *gfx, TextFrame *text, TextFrame *text2, s32 slot, f32 x, f32 y, Unk88208 *save,
                            u32 file) {
    s32 i;
    s32 n;

    diarySegLoad((u32) diaryPal, 0x10B);
    gSPDisplayList(gfx++, fadeSegToRam(diaryLetterDl, file));
    if (save->savePoint == 0) {
        diaryTextCenter(text, x - 20.0f, y, 8, 14, (u32) "From the beginning");
    } else {
        gfx = diaryDrawTile4b(gfx, x - 128.0f + 2.0f, y - 14.0f, 16, 18, slot * 16 + 64, 0);
        diarySetPointName(text, x - 96.0f, y - 12.0f, save->savePoint);
        gfx = diaryDrawTile4b(gfx, x - 60.0f, y + 4.0f, 16, 10, 0, 75);
        gfx = diaryDrawNumber(gfx, x - 40.0f, y + 4.0f, save->players[0].lives - 1, 2);
        for (i = 0, n = 0; i < 6; i++) {
            n += save->tegata[i];
        }
        gfx = diaryDrawNumber(gfx, x + 12.0f, y + 4.0f, n, 10);
        gfx = diaryDrawNumber(gfx, x + 60.0f, y + 4.0f, save->players[0].money, 100);
        gSPDisplayList(gfx++, fadeSegToRam(diarySlotSetupDl, file));
        switch (save->charaCount) {
            default:
                gfx = diaryDrawCharaIcon(gfx, 3, x + 96 + 16.0f, y);
            case 3:
                gfx = diaryDrawCharaIcon(gfx, 2, x + 96.0f, y);
            case 2:
                break;
        }
        gfx = diaryDrawCharaIcon(gfx, 1, x + 96.0f + 16.0f, y - 16.0f);
        gfx = diaryDrawCharaIcon(gfx, 0, x + 96.0f, y - 16.0f);
        gfx = titleDrawRgba16(gfx, file, x - 6.0f, y + 1.0f, 1.0f, 0xFF, 16, 16,
                                     floadSegToRam((u32) diaryPassIconTex, 0x10B, 0));
        gfx = titleDrawRgba16(gfx, file, x + 42.0f, y + 1.0f, 1.0f, 0xFF, 16, 16,
                                     floadSegToRam((u32) diaryMoneyIconTex, 0x10B, 0));
    }
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* "hokusai"'s work. */
typedef struct HokusaiW {
    /* 0x00 */ DLFrame *frame;
    /* 0x04 */ u32 seg; /* segment 0x10B as loaded (KSEG0) */
} HokusaiW;

extern Gfx diaryHokusaiDl[]; /* its display list */

/* The file's .data, in address order (its externs above stay as declarations). */
Gfx diaryFrameDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetCombineLERP(ENVIRONMENT, 0, SHADE, 0, ENVIRONMENT, 0, SHADE, 0, ENVIRONMENT, 0, SHADE, 0, ENVIRONMENT, 0, SHADE, 0),
    gsDPSetEnvColor(0xFF, 0xFF, 0xFF, 0xFF),
    gsSPEndDisplayList(),
};
Gfx diaryHokusaiDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetScissor(G_SC_NON_INTERLACE, 0, 0, 320, 240),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2),
    gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_DECALRGBA),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsDPSetTextureLUT(G_TT_RGBA16),
    gsSPEndDisplayList(),
}; /* its display list */
Gfx diarySlotSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_COPY),
    gsDPSetRenderMode(G_RM_NOOP, G_RM_NOOP2),
    gsSPEndDisplayList(),
};
Gfx diaryLetterDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetScissor(G_SC_NON_INTERLACE, 0, 0, 320, 240),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, ENVIRONMENT, 0, TEXEL0, 0, 0, 0, 0, ENVIRONMENT, ENVIRONMENT, 0, TEXEL0, 0),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetTextureFilter(G_TF_POINT),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetEnvColor(0xFF, 0xFF, 0xFF, 0xFF),
    gsSPEndDisplayList(),
};
u32 diarySegRam = 0x00000000; /* the address the segment below was loaded to */
s32 diaryTexFmt = 4; /* the texel size diaryDrawTile4b's texture loads use */

/* "hokusai": a display list frame that draws diaryHokusaiDl with segment 0x10B as
 * segment 3. A one-byte pad after w puts the two spills at the game's 0x20 and 0x2C. */
s32 diaryHokusaiProc(Obj *obj, s16 msg, void *arg) {
    HokusaiW *w = (HokusaiW *) FX(obj);
    s8 pad;

    switch (msg) {
        case 2:
            obj->name = "hokusai";
            w->frame = (DLFrame *) frmCreate(gFrmLayers[0], gFrmKindDlist);
            if (w->frame == NULL) {
                return -1;
            }
            w->seg = floadSegToRam(0, 0x10B, 0) + 0x80000000;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, fadeSegToRam(diaryHokusaiDl, obj->file));
            gSPSegment(gGfxDlHead++, 3, w->seg);
            gGfxDlHead = (Gfx *) diaryReturnArg((s32) gGfxDlHead);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 diaryHokusaiProc(Obj *obj, s16 msg, void *arg);

Obj *diaryHokusaiMake(Obj *obj) {
    return tskCreate(obj, diaryHokusaiProc, obj->file, NULL);
}

#define DPRINTF

extern Gfx diaryBookmarkDl[]; /* the bookmark */
extern Gfx diaryBookmarkTagDl[]; /* its tag */
s32 diarySioriReadSave(Obj *obj);

/* "siori", a journal slot's bookmark (its argument: the slot): the bookmark and its tag (file
 * 0x10B) on layer 2, turned -90 degrees, their lists and two texts; it starts blue with a copy
 * of the slot's save. Each frame its x, y and scale move by their steps to their targets, the
 * two models following. Drawn in its colour; above scale 0.9 it shows the slot's summary
 * (diaryDrawSummary), else nothing. */
s32 diarySioriProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    SioriWork *w = SIORI(obj);

    switch (msg) {
        case 2:
            obj->name = "siori";
            w->slot = (s32) arg;
            w->frame2 = (PolFrame *) frmCreate(gFrmLayer2, gFrmKindPol);
            w->frame = (PolFrame *) frmCreate(gFrmLayer2, gFrmKindPol);
            w->list = (DLFrame *) frmCreate(gFrmLayers[0], gFrmKindDlist);
            w->list2 = (DLFrame *) frmCreate(gFrmLayer2, gFrmKindDlist);
            if (w->frame2 == NULL || w->frame == NULL || w->list == NULL || w->list2 == NULL) {
                return -1;
            }
            w->text = frmFontCreate(&w->list2->f, 0, 0, NULL);
            w->text2 = frmFontCreate(&w->list2->f, 0, 0, NULL);
            if (w->text == NULL || w->text2 == NULL) {
                return -1;
            }
            w->frame2->dl2 = diaryBookmarkDl;
            floadGetSetRam((void **) w->frame2->segs, 0x10B);
            w->frame2->rot[0] = -1.5707964f;
            w->frame->dl = (Gfx *) fadeSegToRam(diaryFrameDl, obj->file);
            w->frame->dl2 = diaryBookmarkTagDl;
            floadGetSetRam((void **) w->frame->segs, 0x10B);
            w->frame->rot[0] = -1.5707964f;
            w->steps = 8.0f;
            w->width = 136.0f;
            w->height = 18.0f;
            diarySioriSetBlue(obj);
            if ((w->save = heapMalloc(sizeof(Unk88208))) == NULL) {
                DPRINTF("Error: メモリーが足りません in siori \n");
                return -1;
            }
            diarySioriReadSave(obj);
            break;
        case 1:
            if (w->save != NULL) { heapFree(w->save); }
            frmDestroy(&w->frame->f);
            frmDestroy(&w->frame2->f);
            frmDestroy(&w->list->f);
            frmDestroy(&w->list2->f);
            break;
        case 0:
            if (w->vx != 0.0f) {
                if (fabsf(w->x - w->targetX) <= fabsf(w->vx)) {
                    w->x = w->targetX;
                    w->vx = 0.0f;
                } else {
                    w->x += w->vx;
                }
            }
            if (w->vy != 0.0f) {
                if (fabsf(w->y - w->targetY) <= fabsf(w->vy)) {
                    w->y = w->targetY;
                    w->vy = 0.0f;
                } else {
                    w->y += w->vy;
                }
            }
            if (w->scaleStep != 0.0f) {
                if (fabsf(w->scale - w->scaleTarget) <= fabsf(w->scaleStep)) {
                    w->scale = w->scaleTarget;
                    w->scaleStep = 0.0f;
                } else {
                    w->scale += w->scaleStep;
                }
            }
            w->frame->pos[0] = w->x;
            w->frame->pos[1] = w->y;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale;
            w->frame2->pos[0] = w->x;
            w->frame2->pos[1] = w->y;
            w->frame2->scale[0] = w->frame2->scale[1] = w->frame2->scale[2] = w->scale;
            break;
        case 3:
            w->frame2->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, fadeSegToRam(diaryFrameDl, obj->file));
            gDPSetEnvColor(gGfxDlHead++, w->r, w->g, w->b, w->a);
            gSPEndDisplayList(gGfxDlHead++);
            if (w->list2 != NULL && w->text != NULL && w->text2 != NULL) {
                if (w->scale > 0.9f) {
                    w->list2->dl = gGfxDlHead;
                    gGfxDlHead = diaryDrawSummary(gGfxDlHead, w->text, w->text2, w->slot, w->x, w->y,
                                                        w->save, obj->file);
                } else {
                    w->list2->dl = NULL;
                    w->text->text = 0;
                    w->text2->text = 0;
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "siori" at x, y and scale under obj. */
Obj *diarySioriMake(Obj *obj, void *arg, f32 x, f32 y, f32 scale) {
    Obj *o = tskCreate(obj, diarySioriProc, obj->file, arg);

    if (o != NULL) {
        SIORI(o)->x = x;
        SIORI(o)->y = y;
        SIORI(o)->scale = scale;
    }
    return o;
}

void diarySioriSetSteps(Obj *obj, s32 steps) {
    SIORI(obj)->steps = steps;
}

/* The scale's target, reached in its steps. */
void diarySioriScaleTo(Obj *obj, f32 s) {
    SIORI(obj)->scaleTarget = s;
    SIORI(obj)->scaleStep = (s - SIORI(obj)->scale) / SIORI(obj)->steps;
}

/* The target, reached in its steps. */
void diarySioriMoveTo(Obj *obj, f32 x, f32 y) {
    SioriWork *w = SIORI(obj);

    w->targetX = x;
    w->targetY = y;
    w->vx = (x - w->x) / w->steps;
    w->vy = (y - w->y) / w->steps;
}

/* Only x's target. */
void diarySioriMoveXTo(Obj *obj, f32 x) {
    SIORI(obj)->targetX = x;
    SIORI(obj)->vx = (x - SIORI(obj)->x) / SIORI(obj)->steps;
}

/* Its left edge (x less its scaled width) and y. */
void diarySioriGetLeft(Obj *obj, f32 *x, f32 *y) {
    *x = SIORI(obj)->x - SIORI(obj)->width * SIORI(obj)->scale;
    *y = SIORI(obj)->y;
}

/* Green. */
void diarySioriSetGreen(Obj *obj) {
    SIORI(obj)->g = 0xFF;
    SIORI(obj)->a = 0xFF;
    SIORI(obj)->r = 0;
    SIORI(obj)->b = 0;
}

/* Blue. */
void diarySioriSetBlue(Obj *obj) {
    SIORI(obj)->b = 0xFF;
    SIORI(obj)->a = 0xFF;
    SIORI(obj)->r = 0;
    SIORI(obj)->g = 0;
}

void diarySioriSetKind(Obj *o, s32 v) {
    SIORI(o)->slot = v;
}

Unk88208 *diarySioriGetSave(Obj *obj) {
    return SIORI(obj)->save;
}

/* Makes its journal slot's save data the game's new save, if it has one. */
void diarySioriClearSave(Obj *obj) {
    SioriWork *w = SIORI(obj);

    if (w->save != NULL) {
        fadeNewSave(w->save);
    }
}

/* Writes its slot's save data to journal slot slot (0x300 when it has none); keeps the result
 * in result and returns it. */
s32 diarySioriWriteSave(Obj *obj) {
    s32 pad;
    SioriWork *w = SIORI(obj);
    s32 r;

    if (w->save != NULL) {
        r = fadeWriteSave(w->save, w->slot);
    } else {
        r = 0x300;
    }
    w->result = r;
    return r;
}

/* Reads the bookmark's journal slot from the Controller Pak into its save data: a slot that
 * reads as empty (0x102) or has no data (byte 8 zero) is set up new. Keeps and returns the
 * result (0x300 with no save data). Written as diarySioriWriteSave is (pad first, the 0x300 in
 * an else). */
s32 diarySioriReadSave(Obj *obj) {
    s32 pad;
    SioriWork *w = SIORI(obj);
    s32 result;

    if (w->save != NULL) {
        result = fadeReadSave(w->save, w->slot);
        if (result == 0x102) {
            fadeNewSave(w->save);
        }
        if (w->save->savePoint == 0) {
            fadeNewSave(w->save);
        }
    } else {
        result = 0x300;
    }
    w->result = result;
    return result;
}

s32 diarySioriGetResult(Obj *obj) {
    return SIORI(obj)->result;
}

/* "beshi" (its proc diaryBeshiProc, the "Choose journal" / "Choose Player" prompt):
 * siori's moves, its model, its list and its text. */
typedef struct BeshiW {
    /* 0x00 */ f32 steps;
    /* 0x04 */ f32 x;
    /* 0x08 */ f32 y;
    /* 0x0C */ f32 width;
    /* 0x10 */ f32 height;
    /* 0x14 */ f32 vx;
    /* 0x18 */ f32 vy;
    /* 0x1C */ f32 targetX;
    /* 0x20 */ f32 targetY;
    /* 0x24 */ f32 scale;
    /* 0x28 */ f32 scaleStep;
    /* 0x2C */ f32 scaleTarget;
    /* 0x30 */ u8 pad30[0x4];
    /* 0x34 */ PolFrame *frame;
    /* 0x38 */ u8 pad38[0x4];
    /* 0x3C */ DLFrame *list;
    /* 0x40 */ u8 pad40[0x4];
    /* 0x44 */ TextFrame *text;
    /* 0x48 */ u8 pad48[0x4];
    /* 0x4C */ s8 kind; /* which prompt */
} BeshiW;

extern Gfx diaryPromptDl[]; /* the prompt's model */

/* "beshi", the journal menu's prompt: a model (file 0x10B) on layer 2, turned -90 degrees, a
 * display list and a text. Each frame its x, y and scale move by their steps to their targets;
 * the model follows. Drawn: above scale 0.9 the text is its kind's prompt ("Choose journal",
 * "Which", "Where", "Choose Player"), else none. */
s32 diaryBeshiProc(Obj *obj, s16 msg, void *arg) {
    BeshiW *w = (BeshiW *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "beshi";
            w->frame = (PolFrame *) frmCreate(gFrmLayer2, gFrmKindPol);
            w->list = (DLFrame *) frmCreate(gFrmLayers[0], gFrmKindDlist);
            w->text = frmFontCreate(gFrmLayer2, 0, 0, NULL);
            if (w->frame == NULL || w->list == NULL || w->text == NULL) {
                return -1;
            }
            w->frame->dl = (Gfx *) fadeSegToRam(diaryFrameDl, obj->file);
            w->frame->dl2 = diaryPromptDl;
            floadGetSetRam((void **) w->frame->segs, 0x10B);
            w->frame->rot[0] = -1.5707964f;
            w->steps = 8.0f;
            w->width = 104.0f;
            w->height = 10.0f;
            break;
        case 1:
            frmDestroy(&w->text->frame);
            frmDestroy(&w->frame->f);
            frmDestroy(&w->list->f);
            break;
        case 0:
            if (w->vx != 0.0f) {
                if (fabsf(w->x - w->targetX) <= fabsf(w->vx)) {
                    w->x = w->targetX;
                    w->vx = 0.0f;
                } else {
                    w->x += w->vx;
                }
            }
            if (w->vy != 0.0f) {
                if (fabsf(w->y - w->targetY) <= fabsf(w->vy)) {
                    w->y = w->targetY;
                    w->vy = 0.0f;
                } else {
                    w->y += w->vy;
                }
            }
            if (w->scaleStep != 0.0f) {
                if (fabsf(w->scale - w->scaleTarget) <= fabsf(w->scaleStep)) {
                    w->scale = w->scaleTarget;
                    w->scaleStep = 0.0f;
                } else {
                    w->scale += w->scaleStep;
                }
            }
            w->frame->pos[0] = w->x;
            w->frame->pos[1] = w->y;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale;
            break;
        case 3:
            if (w->text != NULL) {
                if (w->scale > 0.9f) {
                    switch (w->kind) {
                        case 0:
                            diaryTextCenter(w->text, w->x, w->y, 10, 14, (u32) "Choose journal");
                            break;
                        case 1:
                            diaryTextCenter(w->text, w->x, w->y, 10, 14, (u32) "Which");
                            break;
                        case 2:
                            diaryTextCenter(w->text, w->x, w->y, 10, 14, (u32) "Where");
                            break;
                        case 3:
                            diaryTextCenter(w->text, w->x, w->y, 10, 14, (u32) "Choose Player");
                            break;
                    }
                } else {
                    w->text->text = 0;
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "beshi" at x, y and scale under obj. */
Obj *diaryBeshiMake(Obj *obj, f32 x, f32 y, f32 scale) {
    Obj *o = tskCreate(obj, diaryBeshiProc, obj->file, NULL);

    if (o != NULL) {
        ((BeshiW *) FX(o))->x = x;
        ((BeshiW *) FX(o))->y = y;
        ((BeshiW *) FX(o))->scale = scale;
    }
    return o;
}

extern Gfx diaryMenu1Dl[]; /* the menu's two models */
extern Gfx diaryMenu2Dl[];

/* "delcopy", the journal's "Copy journal" / "Erase journal" / "Cancel" menu: siori's two models
 * (file 0x10B) turned -90 degrees, its colour list and a text, laid out as siori's work. Each
 * frame its x, y and scale move by their steps to their targets; both models follow. Drawn: the
 * second model in its colour, and above scale 0.9 its kind's words. */
s32 diaryDelcopyProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    SioriWork *w = SIORI(obj);

    switch (msg) {
        case 2:
            obj->name = "delcopy";
            w->frame2 = (PolFrame *) frmCreate(gFrmLayer2, gFrmKindPol);
            w->frame = (PolFrame *) frmCreate(gFrmLayer2, gFrmKindPol);
            w->list = (DLFrame *) frmCreate(gFrmLayers[0], gFrmKindDlist);
            w->text = frmFontCreate(gFrmLayer2, 0, 0, NULL);
            if (w->frame2 == NULL || w->frame == NULL || w->list == NULL || w->text == NULL) {
                return -1;
            }
            w->frame2->dl2 = diaryMenu1Dl;
            floadGetSetRam((void **) w->frame2->segs, 0x10B);
            w->frame2->rot[0] = -1.5707964f;
            w->frame->dl = (Gfx *) fadeSegToRam(diaryFrameDl, obj->file);
            w->frame->dl2 = diaryMenu2Dl;
            floadGetSetRam((void **) w->frame->segs, 0x10B);
            w->frame->rot[0] = -1.5707964f;
            w->steps = 8.0f;
            w->width = 64.0f;
            w->height = 10.0f;
            diarySioriSetBlue(obj);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->frame2->f);
            frmDestroy(&w->list->f);
            frmDestroy(&w->text->frame);
            break;
        case 0:
            if (w->vx != 0.0f) {
                if (fabsf(w->x - w->targetX) <= fabsf(w->vx)) {
                    w->x = w->targetX;
                    w->vx = 0.0f;
                } else {
                    w->x += w->vx;
                }
            }
            if (w->vy != 0.0f) {
                if (fabsf(w->y - w->targetY) <= fabsf(w->vy)) {
                    w->y = w->targetY;
                    w->vy = 0.0f;
                } else {
                    w->y += w->vy;
                }
            }
            if (w->scaleStep != 0.0f) {
                if (fabsf(w->scale - w->scaleTarget) <= fabsf(w->scaleStep)) {
                    w->scale = w->scaleTarget;
                    w->scaleStep = 0.0f;
                } else {
                    w->scale += w->scaleStep;
                }
            }
            w->frame->pos[0] = w->x;
            w->frame->pos[1] = w->y;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale;
            w->frame2->pos[0] = w->x;
            w->frame2->pos[1] = w->y;
            w->frame2->scale[0] = w->frame2->scale[1] = w->frame2->scale[2] = w->scale;
            break;
        case 3:
            w->frame2->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, fadeSegToRam(diaryFrameDl, obj->file));
            gDPSetEnvColor(gGfxDlHead++, w->r, w->g, w->b, w->a);
            gSPEndDisplayList(gGfxDlHead++);
            if (w->text != NULL) {
                if (w->scale > 0.9f) {
                    switch (w->slot) {
                        case 0:
                            diaryTextCenter(w->text, w->x, w->y, 8, 12, (u32) "Copy journal");
                            break;
                        case 1:
                            diaryTextCenter(w->text, w->x, w->y, 8, 12, (u32) "Erase journal");
                            break;
                        case 2:
                            diaryTextCenter(w->text, w->x, w->y, 8, 12, (u32) "Cancel");
                            break;
                    }
                } else {
                    w->text->text = 0;
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "delcopy" (the "Copy journal" / "Erase journal" / "Cancel" menu) at x, y and scale under
 * obj, laid out as siori's work. */
Obj *diaryDelcopyMake(Obj *obj, s32 v, f32 x, f32 y, f32 scale) {
    Obj *o = tskCreate(obj, diaryDelcopyProc, obj->file, NULL);

    if (o != NULL) {
        SIORI(o)->slot = v;
        SIORI(o)->x = x;
        SIORI(o)->y = y;
        SIORI(o)->scale = scale;
    }
    return o;
}

/* The journal's cursor: SioriWork's moves, and its own model. */
typedef struct DCursorWork {
    /* 0x00 */ f32 steps;
    /* 0x04 */ f32 x;
    /* 0x08 */ f32 y;
    /* 0x0C */ f32 width;
    /* 0x10 */ f32 height;
    /* 0x14 */ f32 vx;
    /* 0x18 */ f32 vy;
    /* 0x1C */ f32 targetX;
    /* 0x20 */ f32 targetY;
    /* 0x24 */ f32 scale;
    /* 0x28 */ f32 scaleStep;
    /* 0x2C */ f32 scaleTarget;
    /* 0x30 */ PolFrame *frame;
} DCursorWork;

extern Gfx diaryCursorDl[]; /* the cursor's model */

/* "dcursor", the journal's cursor: a model of file 0x10B on the effect layer. Each frame its x,
 * y and scale move by their steps until within a step of their targets, where they stop; the
 * model follows, at 0.12 of the scale, turning 4 degrees a frame. */
s32 diaryDcursorProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    DCursorWork *w = (DCursorWork *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "dcursor";
            w->frame = (PolFrame *) frmCreate(gFrmEffectLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl = gDrawSetupLitOpaDl;
            w->frame->dl2 = diaryCursorDl;
            floadGetSetRam((void **) w->frame->segs, 0x10B);
            w->steps = 8.0f;
            w->width = 64.0f;
            w->height = 10.0f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (w->vx != 0.0f) {
                if (fabsf(w->x - w->targetX) <= fabsf(w->vx)) {
                    w->x = w->targetX;
                    w->vx = 0.0f;
                } else {
                    w->x += w->vx;
                }
            }
            if (w->vy != 0.0f) {
                if (fabsf(w->y - w->targetY) <= fabsf(w->vy)) {
                    w->y = w->targetY;
                    w->vy = 0.0f;
                } else {
                    w->y += w->vy;
                }
            }
            if (w->scaleStep != 0.0f) {
                if (fabsf(w->scale - w->scaleTarget) <= fabsf(w->scaleStep)) {
                    w->scale = w->scaleTarget;
                    w->scaleStep = 0.0f;
                } else {
                    w->scale += w->scaleStep;
                }
            }
            w->frame->pos[0] = w->x;
            w->frame->pos[1] = w->y;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale * 0.12f;
            w->frame->rot[0] += 0.06981317f;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 diaryDcursorProc(Obj *obj, s16 msg, void *arg);

/* A "dcursor" at x, y and scale under obj (its work is laid out as siori's). */
Obj *diaryDcursorMake(Obj *obj, f32 x, f32 y, f32 scale) {
    Obj *o = tskCreate(obj, diaryDcursorProc, obj->file, NULL);

    if (o != NULL) {
        SIORI(o)->x = x;
        SIORI(o)->y = y;
        SIORI(o)->scale = scale;
    }
    return o;
}

void diaryDcursorSetSteps(Obj *obj, s32 steps) {
    SIORI(obj)->steps = steps;
}

void diaryDcursorScaleTo(Obj *obj, f32 s) {
    SIORI(obj)->scaleTarget = s;
    SIORI(obj)->scaleStep = (s - SIORI(obj)->scale) / SIORI(obj)->steps;
}

void diaryDcursorMoveTo(Obj *obj, f32 x, f32 y) {
    SioriWork *w = SIORI(obj);

    w->targetX = x;
    w->targetY = y;
    w->vx = (x - w->x) / w->steps;
    w->vy = (y - w->y) / w->steps;
}

typedef struct DtexWork {
    /* 0x00 */ DLFrame *frame;
    /* 0x04 */ f32 x;
    /* 0x08 */ f32 y;
    /* 0x0C */ u32 tex[2];
    /* 0x14 */ s32 page;
    /* 0x18 */ s32 w;
    /* 0x1C */ s32 h;
    /* 0x20 */ s32 alpha;
    /* 0x24 */ s32 state;
} DtexWork;

void diaryDtexSetPos(Obj *obj, f32 x, f32 y) {
    DtexWork *d = (DtexWork *) (obj + 1);

    d->x = x;
    d->y = y;
}
