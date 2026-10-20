#include "tlb.h"
#include "tlb/tlb_0180.h"

/* The staff roll's pages (file 180): the "staff0" task shows a page's pictures one after another
 * through a "dtex" picture task, with a "cinesco" task that squeezes the screen into a letterbox,
 * then goes on to the next page's stage (or the next page in place). */

/* A staff roll picture, shown for a time; a page is a list of them ending in time 0. */
typedef struct StaffLine {
    /* 0x00 */ u32 tex; /* its address in its file (0: a pause) */
    /* 0x04 */ u16 file;
    /* 0x08 */ s32 w;
    /* 0x0C */ s32 h;
    /* 0x10 */ s32 unused10[2]; /* -1 */
    /* 0x18 */ s32 time; /* frames shown (0: the page's end) */
} StaffLine; /* size 0x1C */

/* The "cinesco" task's work. */
typedef struct CinescoWork {
    /* 0x00 */ u8 unused0;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad9[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ DLFrame *frame;
    /* 0x38 */ u32 arg;
    /* 0x3C */ s32 time;
    /* 0x40 */ u8 pad40[4];
    /* 0x44 */ f32 scale; /* the picture's height, 1.0 to 0.5 */
    /* 0x48 */ f32 top; /* its top, 0 to 30 */
    /* 0x4C */ u8 state;
} CinescoWork;

/* The "staff0" task's work. */
typedef struct Staff0Work {
    /* 0x00 */ u8 unused0;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad9[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ s32 time; /* the picture's frames */
    /* 0x38 */ s32 wait; /* frames at the page's end */
    /* 0x3C */ StaffLine *line;
    /* 0x40 */ Obj *fade;
    /* 0x44 */ Obj *dtex;
    /* 0x48 */ u32 tex;
    /* 0x4C */ s32 endWait;
    /* 0x50 */ u8 no; /* the page */
    /* 0x51 */ u8 state;
} Staff0Work;

/* The "dtex" task's work: a picture drawn in tiles (I4 64x64, or RGBA16 64x32), two pages,
 * faded. */
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
    /* 0x28 */ s32 rgba; /* 1: RGBA16 */
} DtexWork;

extern u16 gFramebuffers[3][SCREEN_WIDTH * SCREEN_HEIGHT]; /* the framebuffers */

extern StaffLine staffPage0Lines[];
extern StaffLine staffPage1Lines[];
extern StaffLine staffPage2Lines[];
extern StaffLine staffPage3Lines[];
extern StaffLine staffPage4Lines[];
extern StaffLine staffPage5Lines[];
extern StaffLine staffPage6Lines[];
extern StaffLine staffPage7Lines[];
extern StaffLine staffPage8Lines[];
extern StaffLine staffPage9Lines[];
extern StaffLine staffPage10Lines[];
extern StaffLine staffPage11Lines[];
Gfx staffCinescoDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_DISABLE),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetTextureFilter(G_TF_POINT),
    gsDPSetCombineLERP(0, 0, 0, TEXEL0, 0, 0, 0, 1, 0, 0, 0, TEXEL0, 0, 0, 0, 1),
    gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsSPEndDisplayList(),
};
u16 staffPageFileSets[] = { 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313 };
extern u8 staffPage0Tex0[];
extern u8 staffPage0Tex1[];
/* Each page's pictures are its own file (the word after each texture: files 0x311-0x31C) at the
 * same segment, so one address is a picture of each page: a name for each page's. */
extern u8 staffPage1Tex0[];
extern u8 staffPage2Tex0[];
extern u8 staffPage3Tex0[];
extern u8 staffPage4Tex0[];
extern u8 staffPage5Tex0[];
extern u8 staffPage6Tex0[];
extern u8 staffPage7Tex0[];
extern u8 staffPage8Tex0[];
extern u8 staffPage9Tex0[];
extern u8 staffPage10Tex0[];
extern u8 staffPage11Tex0[];
extern u8 staffPage4Tex1[];
extern u8 staffPage6Tex1[];
extern u8 staffPage7Tex1[];
StaffLine staffPage0Lines[] = {
    { 0x00000000, 0x0000, 0, 0, { -1, -1 }, 31 },
    { (u32) staffPage0Tex0, 0x0311, 176, 32, { -1, -1 }, 174 },
    { (u32) staffPage0Tex1, 0x0311, 128, 32, { -1, -1 }, 149 },
    { 0x03001300, 0x0311, 128, 32, { -1, -1 }, 149 },
    { 0x00000000, 0x0000, 0, 0, { -1, -1 }, 0 },
};
extern u8 staffPage1Tex1[];
StaffLine staffPage1Lines[] = {
    { 0x00000000, 0x0000, 0, 0, { -1, -1 }, 24 },
    { (u32) staffPage1Tex0, 0x0312, 96, 32, { -1, -1 }, 174 },
    { (u32) staffPage1Tex1, 0x0312, 144, 32, { -1, -1 }, 149 },
    { 0x00000000, 0x0000, 0, 0, { 0 }, 0 },
};
extern u8 staffPage2Tex1[];
StaffLine staffPage2Lines[] = {
    { 0x00000000, 0x0000, 0, 0, { -1, -1 }, 24 },
    { (u32) staffPage2Tex0, 0x0313, 128, 32, { -1, -1 }, 174 },
    { (u32) staffPage2Tex1, 0x0313, 240, 52, { -1, -1 }, 149 },
    { 0x00000000, 0x0000, 0, 0, { 0 }, 0 },
};
extern u8 staffPage3Tex1[];
extern u8 staffPage3Tex2[];
StaffLine staffPage3Lines[] = {
    { 0x00000000, 0x0000, 0, 0, { -1, -1 }, 24 },
    { (u32) staffPage3Tex0, 0x0314, 128, 52, { -1, -1 }, 174 },
    { (u32) staffPage3Tex1, 0x0314, 144, 32, { -1, -1 }, 149 },
    { (u32) staffPage3Tex2, 0x0314, 144, 52, { -1, -1 }, 149 },
    { 0x00000000, 0x0000, 0, 0, { 0 }, 0 },
};
StaffLine staffPage4Lines[] = {
    { 0x00000000, 0x0000, 0, 0, { -1, -1 }, 24 },
    { (u32) staffPage4Tex0, 0x0315, 128, 32, { -1, -1 }, 174 },
    { (u32) staffPage4Tex1, 0x0315, 128, 32, { -1, -1 }, 149 },
    { 0x03001000, 0x0315, 112, 32, { -1, -1 }, 149 },
    { 0x00000000, 0x0000, 0, 0, { 0 }, 0 },
};
extern u8 staffPage5Tex1[];
StaffLine staffPage5Lines[] = {
    { 0x00000000, 0x0000, 0, 0, { -1, -1 }, 24 },
    { (u32) staffPage5Tex0, 0x0316, 128, 48, { -1, -1 }, 174 },
    { (u32) staffPage5Tex1, 0x0316, 128, 32, { -1, -1 }, 149 },
    { 0x00000000, 0x0000, 0, 0, { 0 }, 0 },
};
StaffLine staffPage6Lines[] = {
    { 0x00000000, 0x0000, 0, 0, { -1, -1 }, 24 },
    { (u32) staffPage6Tex0, 0x0317, 128, 48, { -1, -1 }, 174 },
    { (u32) staffPage6Tex1, 0x0317, 96, 32, { -1, -1 }, 149 },
    { 0x00000000, 0x0000, 0, 0, { 0 }, 0 },
};
StaffLine staffPage7Lines[] = {
    { 0x00000000, 0x0000, 0, 0, { -1, -1 }, 24 },
    { (u32) staffPage7Tex0, 0x0318, 128, 32, { -1, -1 }, 174 },
    { (u32) staffPage7Tex1, 0x0318, 128, 32, { -1, -1 }, 149 },
    { 0x03001000, 0x0318, 128, 32, { -1, -1 }, 149 },
    { 0x00000000, 0x0000, 0, 0, { 0 }, 0 },
};
StaffLine staffPage8Lines[] = {
    { 0x00000000, 0x0000, 0, 0, { -1, -1 }, 24 },
    { (u32) staffPage8Tex0, 0x0319, 256, 48, { -1, -1 }, 174 },
    { 0x03001800, 0x0319, 224, 32, { -1, -1 }, 149 },
    { 0x03002600, 0x0319, 112, 32, { -1, -1 }, 149 },
    { 0x00000000, 0x0000, 0, 0, { 0 }, 0 },
};
StaffLine staffPage9Lines[] = {
    { (u32) staffPage9Tex0, 0x031A, 144, 64, { -1, -1 }, 149 },
    { 0x03001200, 0x031A, 240, 32, { -1, -1 }, 149 },
    { 0x00000000, 0x0000, 0, 0, { 0 }, 0 },
};
StaffLine staffPage10Lines[] = {
    { (u32) staffPage10Tex0, 0x031B, 256, 96, { -1, -1 }, 149 },
    { 0x03003000, 0x031B, 256, 96, { -1, -1 }, 149 },
    { 0x03006000, 0x031B, 128, 32, { -1, -1 }, 149 },
    { 0x03006800, 0x031B, 144, 32, { -1, -1 }, 149 },
    { 0x00000000, 0x0000, 0, 0, { 0 }, 0 },
};
extern u8 staffPage11Tex1[];
StaffLine staffPage11Lines[] = {
    { (u32) staffPage11Tex0, 0x031C, 192, 42, { -1, -1 }, 149 },
    { (u32) staffPage11Tex1, 0x031C, 288, 39, { -1, -1 }, 149 },
    { 0x030096C0, 0x031C, 80, 16, { -1, -1 }, 269 },
    { 0x00000000, 0x0000, 0, 0, { 0 }, 0 },
};
StaffLine *staffPageLines[] = { staffPage0Lines, staffPage1Lines, staffPage2Lines, staffPage3Lines, staffPage4Lines, staffPage5Lines, staffPage6Lines, staffPage7Lines, staffPage8Lines, staffPage9Lines, staffPage10Lines, staffPage11Lines };
void *staffPageCinescoArgs[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
s32 staffPageStages[] = { 257, 258, 259, 261, 260, 262, 263, 264, 265, 0, 0, 0, -1 };
Gfx staffPageDtexDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetTextureFilter(G_TF_POINT),
    gsDPSetCombineLERP(0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0),
    gsDPPipeSync(),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsSPEndDisplayList(),
};

s32 staffCinescoProc(Obj *obj, s16 msg, void *arg);
Gfx *staffCinescoDraw(Gfx *gfx, Obj *obj, CinescoWork *w);
s32 staffLineStep(Obj *obj, StaffLine *line, s32 time, s32 center, s32 rgba);
void staffPageInit(Obj *obj, void *arg);
void staffPageFadeOut(Obj *obj);
void staffPageGotoStage(Obj *obj);
void staffPageNext(Obj *obj);
void staffPageEnd(Obj *obj);
Gfx *staffDrawRgba16Tiles(Gfx *gfx, u32 file, f32 x, f32 y, f32 scale, s32 alpha, s32 w, s32 h, u32 tex);
Gfx *staffPageDrawI4Tiles(Gfx *gfx, f32 x, f32 y, f32 scale, s32 w, s32 h, u32 tex);
s32 staffDtexProc(Obj *obj, s16 msg, void *arg);
Obj *staffDtexCreate(Obj *obj, void *arg);
void staffDtexSetTex(Obj *obj, u32 tex0, u32 tex1, s32 w, s32 h);
void staffDtexSetPos(Obj *obj, f32 x, f32 y);
void staffDtexSetAlpha(Obj *obj, s32 alpha);
void staffDtexAddAlpha(Obj *obj, s32 step);
void staffDtexSetRgba(Obj *obj, s32 rgba);

/* The "cinesco" task: after 4 seconds squeezes the screen to half its height, 30 lines down,
 * over 40 retraces, by drawing the framebuffer back over itself. */
s32 staffCinescoProc(Obj *obj, s16 msg, void *arg) {
    CinescoWork *w = (CinescoWork *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "cinesco";
            if ((u32) arg >= 0x8000000) {
                w->arg = eneSetup(obj, arg);
            } else {
                w->arg = (u32) arg;
            }
            w->time = 0;
            w->state = 0;
            w->scale = 1.0f;
            w->top = 0.0f;
            w->frame = (DLFrame *) frmCreateNext((Frame *) gFramePool10, gFrmKindDlist);
            if (w->frame == NULL) {
                return -1;
            }
            break;
        case 0:
            w->time++;
            if (w->state == 0) {
                w->scale = 1.0f;
                w->top = 0.0f;
                if (w->time >= 240 / gGameState.retracesPerFrame) {
                    w->state = 1;
                }
            } else if (w->state == 1) {
                f32 v;

                v = w->scale + -0.5f / (40 / gGameState.retracesPerFrame);
                if (v <= 0.5f) {
                    w->scale = 0.5f;
                } else {
                    w->scale = v;
                }
                v = w->top + 30.0f / (40 / gGameState.retracesPerFrame);
                if (v >= 30.0f) {
                    w->top = 30.0f;
                } else {
                    w->top = v;
                }
                if (w->scale == 0.5f && w->top == 30.0f) {
                    w->state = 2;
                }
            } else if (w->state == 2) {
                w->top = 30.0f;
                w->scale = 0.5f;
            }
            PRINTF("staff_name_disp: time = %d\n", w->time);
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 3:
            if (w->scale != 1.0f || w->top != 0.0f) {
                w->frame->dl = gGfxDlHead;
                gGfxDlHead = staffCinescoDraw(gGfxDlHead, obj, w);
                gSPEndDisplayList(gGfxDlHead++);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The letterbox: the framebuffer drawn back in strips 8 wide, w->scale high from w->top, and
 * black above and below.
 * Matched 2026-10-16; it was parked for the loop's induction registers and the scale product's
 * operand order; a statement leading with a stored, constant-foldable value (as Majora's Mask's
 * Camera_Fixed2 resolves the same fake) gave the game's "mul.s 240,scale" and "add.s y,product". */
Gfx *staffCinescoDraw(Gfx *gfx, Obj *obj, CinescoWork *w) {
    u16 *fb = gFramebuffers[gfxGetFbIndex()];
    s32 i;
    s32 left;
    s32 right;
    s32 top;
    s32 bottom;
    s32 ty;
    s32 th;

    gSPDisplayList(gfx++, fadeSegToRam(staffCinescoDl, obj->file));
    ty = 0;
    th = SCREEN_HEIGHT;
    for (i = 0; i < SCREEN_WIDTH / 8; i++) {
        left = i * 8;
        right = (i + 1) * 8;
        top = w->top * 4.0f;
        bottom = 0;
        bottom += (240.0f * w->scale + w->top) * 4.0f;
        gDPPipeSync(gfx++);
        gDPLoadTextureTile(gfx++, fb, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, SCREEN_HEIGHT, left, ty,
                           right - 1, ty + th - 1, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK,
                           G_TX_NOLOD, G_TX_NOLOD);
        gSPTextureRectangle(gfx++, left << 2, top, right << 2, bottom, G_TX_RENDERTILE, left << 5, 0, 1 << 10,
                            (s32) (1.0f / w->scale * 1024.0f));
    }
    gDPPipeSync(gfx++);
    gDPSetCycleType(gfx++, G_CYC_1CYCLE);
    gDPSetCombineMode(gfx++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPSetAlphaCompare(gfx++, G_AC_NONE);
    gDPSetRenderMode(gfx++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
    gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, 255);
    gDPFillRectangle(gfx++, 0, 0, SCREEN_WIDTH, (s32) (w->top + 1));
    gDPFillRectangle(gfx++, 0, (s32) (w->top + 240.0f * w->scale), SCREEN_WIDTH, SCREEN_HEIGHT);
    return gfx;
}

/* One picture of a page at its frame time: 1 at its end; sets the dtex task's picture, centred
 * (or at line 160), and fades it in over 32 frames and out over its last 32. */
s32 staffLineStep(Obj *obj, StaffLine *line, s32 time, s32 center, s32 rgba) {
    Staff0Work *w;

    if (time == line->time) {
        return 1;
    }
    w = (Staff0Work *) FX(obj);
    if (line->tex == 0) {
        return 0;
    }
    staffDtexSetRgba(w->dtex, rgba);
    w->tex = fadeSegToRam(line->tex, line->file);
    staffDtexSetTex(w->dtex, w->tex, w->tex, line->w, line->h);
    if (!center) {
        staffDtexSetPos(w->dtex, (SCREEN_WIDTH - w->line->w) / 2, 160.0f);
    } else {
        staffDtexSetPos(w->dtex, (SCREEN_WIDTH - w->line->w) / 2, (SCREEN_HEIGHT - w->line->h) / 2);
    }
    if (time == 1) {
        staffDtexSetAlpha(w->dtex, 8);
    } else if (time < 32) {
        staffDtexAddAlpha(w->dtex, 8);
    } else if (line->time - 32 < time) {
        staffDtexAddAlpha(w->dtex, -8);
    }
    return 0;
}

/* The "staff0" task's start: its page (arg, or eneSetup's value), its dtex and cinesco tasks,
 * the page's file set; the first page fades in from white. */
void staffPageInit(Obj *obj, void *arg) {
    Staff0Work *w = (Staff0Work *) FX(obj);
    s32 pad;
    FileSet set;

    if ((u32) arg >= 0x8000000) {
        w->no = eneSetup(obj, arg);
    } else {
        w->no = (u32) arg;
    }
    w->line = staffPageLines[w->no];
    w->dtex = staffDtexCreate(obj, (void *) 1);
    tskCreateBlocks(gStaffMTask, staffCinescoProc, 1, obj->file, staffPageCinescoArgs[w->no]);
    set = floadLoadSet(staffPageFileSets[w->no]);
    w->time = 0;
    w->wait = 0;
    w->endWait = 0;
    w->state = 0;
    gGameWork.demoPlaying = 1;
    w->fade = NULL;
    if (w->no == 0) {
        w->fade = fadeCreate(obj);
        fadeSetAll(40.0f, 255.0f, 0.0f, 255, 255, 255);
    }
}

/* A page's end: fade to black. */
void staffPageFadeOut(Obj *obj) {
    Staff0Work *w = (Staff0Work *) FX(obj);

    if (w->fade == NULL) {
        w->fade = fadeCreate(obj);
    }
    fadeSetAll(58.0f, 0.0f, 255.0f, 0, 0, 0);
}

/* To the next page's stage. */
void staffPageGotoStage(Obj *obj) {
    Staff0Work *w = (Staff0Work *) FX(obj);

    stageGoto(gStaffMTask, staffPageStages[w->no]);
}

/* The next page, in place (centred). */
void staffPageNext(Obj *obj) {
    Staff0Work *w = (Staff0Work *) FX(obj);
    s32 pad;

    w->no++;
    w->line = staffPageLines[w->no];
    floadLoadSet(staffPageFileSets[w->no]);
    w->time = 0;
    w->wait = 0;
    w->state = 1;
}

/* The last page done. */
void staffPageEnd(Obj *obj) {
    Staff0Work *w = (Staff0Work *) FX(obj);

    w->state = 2;
}

/* The "staff0" task. */
s32 staffPageProc(Obj *obj, s16 msg, void *arg) {
    Staff0Work *w = (Staff0Work *) FX(obj);
    s32 last;
    s32 pad[2];

    switch (msg) {
        case 3:
            break;
        case 2:
            obj->name = "staff0";
            staffPageInit(obj, arg);
            break;
        case 0:
            switch (w->state) {
                case 2:
                    break;
                case 0:
                    if (w->line->time == 0) {
                        if (w->no < 9) {
                            w->wait++;
                            if (w->wait == 1) {
                                staffPageFadeOut(obj);
                            }
                            if (w->wait == 60 && gStaffMTask != NULL) {
                                staffPageGotoStage(obj);
                            }
                        } else if (gStaffMTask != NULL) {
                            staffPageNext(obj);
                        }
                        break;
                    }
                    w->time++;
                    if (w->no < 9) {
                        if (staffLineStep(obj, w->line, w->time, 0, 0)) {
                            w->time = 0;
                            w->wait = 0;
                            w->line++;
                        }
                    } else {
                        if (staffLineStep(obj, w->line, w->time, 1, 0)) {
                            w->time = 0;
                            w->wait = 0;
                            w->line++;
                        }
                    }
                    break;
                case 1:
                    if (w->line->time == 0) {
                        if (gStaffMTask != NULL) {
                            if (staffPageStages[w->no] < 0) {
                                staffPageEnd(obj);
                            } else if (w->no < 11) {
                                staffPageNext(obj);
                            }
                        }
                        if (w->no >= 11) {
                            w->endWait++;
                            if (w->endWait >= 15) {
                                stageGotoNtegata();
                            }
                        }
                    } else {
                        w->time++;
                        if (w->no >= 11) {
                            last = 1;
                        } else {
                            last = 0;
                        }
                        if (staffLineStep(obj, w->line, w->time, 1, last)) {
                            w->time = 0;
                            w->wait = 0;
                            w->line++;
                        }
                    }
                    break;
            }
            PRINTF("L->no = [%d]\n", w->no);
            break;
        case 1:
            joyDemoEnd();
            gGameWork.demoPlaying = 0;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* An RGBA16 picture of w x h in 64x32 tiles at (x, y), scaled, with an alpha: the I4 one below
 * for 16-bit texels. Ends the list without stepping past the end command. */
Gfx *staffDrawRgba16Tiles(Gfx *gfx, u32 file, f32 x, f32 y, f32 scale, s32 alpha, s32 w, s32 h, u32 tex) {
    s32 tw = 64;
    s32 row;
    s32 nx = (w + 63) / 64;
    s32 ny = (h + 31) / 32;
    s32 col;
    s32 s;
    s32 t;
    s32 sr;
    s32 tb;
    s32 th = 32;

    gDPSetEnvColor(gfx++, 0, 0, 0, alpha);
    if (alpha >= 0xFF) {
        gDPSetRenderMode(gfx++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
    } else {
        gDPSetRenderMode(gfx++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    }
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
            gDPLoadTextureTile(gfx++, tex, G_IM_FMT_RGBA, G_IM_SIZ_16b, w, h, s, t, sr - 1, tb - 1, 0, G_TX_CLAMP,
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

/* An I4 picture of w x h in 64x64 tiles at (x, y), scaled: file 36's staffDrawI4Tiles (and
 * pakfiler.c's pakFilerDrawI4Tiles), word for word. Ends the list without stepping past the end
 * command. */
Gfx *staffPageDrawI4Tiles(Gfx *gfx, f32 x, f32 y, f32 scale, s32 w, s32 h, u32 tex) {
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

/* The "dtex" task: a picture in two pages (w->tex), flipped every frame while state is 0 or 1;
 * in 1 it also fades out by 5 a frame, then stops (2). File 36's staffDtex with an
 * RGBA16 kind. */
s32 staffDtexProc(Obj *obj, s16 msg, void *arg) {
    DtexWork *w = (DtexWork *) ((u32) obj + sizeof(Obj));
    s32 pad[6];

    switch (msg) {
        case 2:
            obj->name = "dtex";
            w->frame = (DLFrame *) frmCreateNext((Frame *) gFramePool10, gFrmKindDlist);
            if (w->frame == NULL) {
                return -1;
            }
            w->page = 0;
            w->alpha = 0xFF;
            w->rgba = 0;
            w->state = 2;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->page ^= 1;
                    break;
                case 1:
                    w->page ^= 1;
                    w->alpha -= 5;
                    if (w->alpha <= 0) {
                        w->alpha = 0;
                        w->state = 2;
                    }
                    break;
                case 2:
                    break;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, fadeSegToRam(staffPageDtexDl, 0xB4));
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            if (w->alpha >= 0xFF) {
                gDPSetRenderMode(gGfxDlHead++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
            } else {
                gDPSetRenderMode(gGfxDlHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
            }
            if (w->rgba == 1) {
                gGfxDlHead = staffDrawRgba16Tiles(gGfxDlHead, obj->file, w->x, w->y, 1.0f, w->alpha, w->w, w->h,
                                                    w->tex[w->page]);
                gSPEndDisplayList(gGfxDlHead++);
            } else {
                gGfxDlHead = staffPageDrawI4Tiles(gGfxDlHead, w->x, w->y, 1.0f, w->w, w->h, w->tex[w->page]);
                gGfxDlHead++;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Makes a dtex task under obj. */
Obj *staffDtexCreate(Obj *obj, void *arg) {
    return tskCreate(obj, staffDtexProc, obj->file, arg);
}

/* A dtex task's picture (both pages) and size. */
void staffDtexSetTex(Obj *obj, u32 tex0, u32 tex1, s32 w, s32 h) {
    DtexWork *d = (DtexWork *) FX(obj);

    d->tex[0] = tex0;
    d->tex[1] = tex1;
    d->w = w;
    d->h = h;
}

/* A dtex task's position. */
void staffDtexSetPos(Obj *obj, f32 x, f32 y) {
    DtexWork *d = (DtexWork *) FX(obj);

    d->x = x;
    d->y = y;
}

/* Two empty functions nothing calls; the split folds the second into the first's .s (check
 * shows its two lines as differences; the whole file's .text is the game's size, 0x1990). */
void staffDummy0(void) {
}

void staffDummy1(void) {
}

/* A dtex task's alpha. */
void staffDtexSetAlpha(Obj *obj, s32 alpha) {
    DtexWork *d = (DtexWork *) FX(obj);

    d->alpha = alpha;
}

/* A dtex task's alpha stepped, kept in 0-255. */
void staffDtexAddAlpha(Obj *obj, s32 step) {
    DtexWork *d = (DtexWork *) FX(obj);

    d->alpha += step;
    if (d->alpha >= 0xFF) {
        d->alpha = 0xFF;
    }
    if (d->alpha <= 0) {
        d->alpha = 0;
    }
}

/* A dtex task's texel kind (1: RGBA16). */
void staffDtexSetRgba(Obj *obj, s32 rgba) {
    DtexWork *d = (DtexWork *) FX(obj);

    d->rgba = rgba;
}
