#include "tlb.h"
#include "tlb/tlb_0018.h"

/* "subt": an area's title card. A camera of its own over Hokusai's picture fading in, two
 * signboards ("kanban") sliding down, Goemon walking in from the right, the area's name on a
 * scroll ("nomaki"); then a fade to black and the area's map (stageGotoAtlasFrom). subtScript
 * runs it by the frame count. */

/* Segment addresses (file 0x10C): the picture, the boards' faces, Goemon's pictures and their
 * shadows, the area names. */
extern u8 subtHokusaiTex[];
extern u8 subtKanbanBoardDl[];
extern u8 subtKanbanBoard2Dl[];
extern u8 subtKanbanBoardKindM1Dl[];
extern u8 subtKanbanBoard2KindM1Dl[];
extern u8 subtGoemonKind0Tex[];
extern u8 subtGoemonShadowKind0Tex[];
extern u8 subtGoemonKind1Tex[];
extern u8 subtGoemonShadowKind1Tex[];
extern u8 subtGoemonKind2Tex[];
extern u8 subtGoemonShadowKind2Tex[];
extern u8 subtGoemonKind3Tex[];
extern u8 subtGoemonShadowKind3Tex[];
extern u8 subtGoemonKind4Tex[];
extern u8 subtNomakiKind0Tex[];
extern u8 subtNomakiKind1Tex[];
extern u8 subtNomakiKind2Tex[];
extern u8 subtNomakiKind3Tex[];
extern u8 subtNomakiKind4Tex[];

extern Gfx diaryLetterDl[]; /* the letters' render setup (tlb_0018_1's data) */
Gfx subtKanbanDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2),
    gsDPSetCombineMode(G_CC_PRIMITIVE, G_CC_PRIMITIVE),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsSPEndDisplayList(),
}; /* the boards' render setup */
Gfx subtKanbanFrame1Dl[] = { { 0xE7000000, 0x00000000 }, { 0xE3000A01, 0x00000000 }, { 0xE3000C00, 0x00080000 }, { 0xE3001402, 0x00000C00 }, { 0xD9C0F9FA, 0x00000000 }, { 0xD9FFFFFF, 0x00200404 }, { 0xE200001C, 0x00504240 }, { 0xFCFFFFFF, 0xFFFEFB7D }, { 0xFB000000, 0x00000080 }, { 0xE2001E01, 0x00000000 }, { 0xDF000000, 0x00000000 }, { 0xE7000000, 0x00000000 }, { 0xE3000A01, 0x00000000 }, { 0xE200001C, 0x00504240 }, { 0xFCFFFFFF, 0xFFFEF379 }, { 0xFB000000, 0x000000FF }, { 0xDF000000, 0x00000000 }, { 0x00000100, 0x02000300 }, { 0x04000500, 0x06000700 }, { 0x08000900, 0x0A000B00 }, { 0x0C00FEFE, 0xFD010001 }, { 0x0101FF00, 0x00000000 }, { 0x00000000, 0x00000000 }, { 0x00000000, 0x00000000 }, { 0x00000000, 0x00000000 }, { 0x00000002, 0x01020202 }, { 0x03020402, 0x05020602 }, { 0x07020802, 0x0C00FEFE }, { 0xFD010001, 0x0101FF00 }, { 0x00000000, 0x00000000 }, { 0x00000000, 0x00000000 }, { 0x00000000, 0x00000000 }, { 0x00000000, 0x00000000 }, { 0x00000003, 0x01030203 }, { 0x03030403, 0x05030603 }, { 0x07030803, 0x09030A03 }, { 0x0B030C00, 0xFEFEFD01 }, { 0x00010101, 0xFF000000 }, { 0x00000000, 0x00000000 }, { 0x00000000, 0x00000000 }, { 0x00000000, 0x00000000 }, { 0x00000004, 0x01040204 }, { 0x03040404, 0x05040604 }, { 0x07040804, 0x09040A04 }, { 0x0B040C00, 0xFEFEFD01 }, { 0x00010101, 0xFF000000 }, { 0x00000000, 0x00000000 }, { 0x00000000, 0x00000000 }, { 0x00000000, 0x00000000 }, { 0x00000005, 0x01050205 }, { 0x03050405, 0x05050605 }, { 0x07050805, 0x0C00FEFE }, { 0xFD010001, 0x0101FF00 }, { 0x00000000, 0x00000000 }, { 0x00000000, 0x00000000 }, { 0x00000000, 0x00000000 }, { 0x00000000, 0x00000000 }, { 0x00000000, 0x00000000 } };


/* Hokusai's picture, 272 x 128 at (24, 48), its alpha moving by a step. */
typedef struct SubtHokusaiWork {
    /* 0x00 */ DLFrame *frame;
    /* 0x04 */ u32 tex;
    /* 0x08 */ s32 alpha;
    /* 0x0C */ s32 alphaStep;
} SubtHokusaiWork;

#define SUBT_HOKUSAI(obj) ((SubtHokusaiWork *) FX(obj))

s32 subtHokusaiProc(Obj *obj, s16 msg, void *arg) {
    SubtHokusaiWork *w = SUBT_HOKUSAI(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "hokusai";
            w->frame = (DLFrame *) frmCreate(gFrmLayers[0], gFrmKindDlist);
            if (w->frame == NULL) {
                return -1;
            }
            w->tex = floadSegToRam((u32) subtHokusaiTex, 0x10C, 0);
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
            gGfxDlHead = titleDrawRgba16(gGfxDlHead, obj->file, 24.0f, 48.0f, 1.0f, w->alpha,
                                                0x110, 0x80, w->tex);
            gGfxDlHead++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *subtHokusaiMake(Obj *obj) {
    return tskCreate(obj, subtHokusaiProc, obj->file, NULL);
}

/* The picture fades in to opaque in steps. */
void subtHokusaiFadeIn(Obj *obj, s32 steps) {
    SubtHokusaiWork *w = SUBT_HOKUSAI(obj);

    w->alphaStep = (255 - w->alpha) / steps;
}

/* A signboard: two "pol" frames turned over, its face by the area (-1: the title's own), moving
 * to a target in steps. */
typedef struct SubtKanbanWork {
    /* 0x00 */ PolFrame *frames[2];
    /* 0x08 */ u8 pad8[4];
    /* 0x0C */ s16 kind;
    /* 0x10 */ f32 steps;
    /* 0x14 */ f32 x;
    /* 0x18 */ f32 vx;
    /* 0x1C */ f32 targetX;
    /* 0x20 */ f32 y;
    /* 0x24 */ f32 vy;
    /* 0x28 */ f32 targetY;
} SubtKanbanWork;

#define SUBT_KANBAN(obj) ((SubtKanbanWork *) FX(obj))

s32 subtKanbanProc(Obj *obj, s16 msg, void *arg) {
    SubtKanbanWork *w = SUBT_KANBAN(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "kanban";
            w->kind = (s32) arg;
            w->frames[1] = (PolFrame *) frmCreate(gFrmLayer2, gFrmKindPol);
            if (w->frames[1] == NULL) {
                return -1;
            }
            w->frames[0] = (PolFrame *) frmCreate(gFrmLayer2, gFrmKindPol);
            if (w->frames[0] == NULL) {
                return -1;
            }
            w->frames[0]->dl = (Gfx *) fadeSegToRam(subtKanbanDl, obj->file);
            w->frames[1]->dl = (Gfx *) fadeSegToRam(subtKanbanFrame1Dl, obj->file);
            floadGetSetRam((void **) w->frames[0]->segs, 0x10C);
            floadGetSetRam((void **) w->frames[1]->segs, 0x10C);
            w->frames[0]->rot[0] = 3.141592741f;
            w->frames[1]->rot[0] = 3.141592741f;
            switch (w->kind) {
                case -1:
                    w->frames[0]->dl2 = (Gfx *) subtKanbanBoardKindM1Dl;
                    w->frames[1]->dl2 = (Gfx *) subtKanbanBoard2KindM1Dl;
                    break;
                case 0:
                default:
                    w->frames[0]->dl2 = (Gfx *) subtKanbanBoardDl;
                    w->frames[1]->dl2 = (Gfx *) subtKanbanBoard2Dl;
                    break;
                case 1:
                    w->frames[0]->dl2 = (Gfx *) subtKanbanBoardDl;
                    w->frames[1]->dl2 = (Gfx *) subtKanbanBoard2Dl;
                    break;
                case 2:
                    w->frames[0]->dl2 = (Gfx *) subtKanbanBoardDl;
                    w->frames[1]->dl2 = (Gfx *) subtKanbanBoard2Dl;
                    break;
                case 3:
                    w->frames[0]->dl2 = (Gfx *) subtKanbanBoardDl;
                    w->frames[1]->dl2 = (Gfx *) subtKanbanBoard2Dl;
                    break;
                case 4:
                    w->frames[0]->dl2 = (Gfx *) subtKanbanBoardDl;
                    w->frames[1]->dl2 = (Gfx *) subtKanbanBoard2Dl;
                    break;
            }
            break;
        case 1:
            frmDestroy(&w->frames[0]->f);
            frmDestroy(&w->frames[1]->f);
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
            w->frames[0]->pos[0] = w->x;
            w->frames[0]->pos[1] = w->y;
            w->frames[1]->pos[0] = w->x;
            w->frames[1]->pos[1] = w->y;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A signboard at (x, y), its face by kind. */
Obj *subtKanbanMake(Obj *parent, s32 kind, f32 x, f32 y) {
    Obj *o;

    if ((o = tskCreate(parent, subtKanbanProc, parent->file, (void *) kind)) != NULL) {
        SUBT_KANBAN(o)->x = x;
        SUBT_KANBAN(o)->y = y;
    }
    return o;
}

void subtKanbanSetSteps(Obj *obj, s32 steps) {
    SUBT_KANBAN(obj)->steps = steps;
}

/* The board's target, reached in its steps. */
void subtKanbanMoveTo(Obj *obj, f32 x, f32 y) {
    SubtKanbanWork *w = SUBT_KANBAN(obj);

    w->targetX = x;
    w->targetY = y;
    w->vx = (x - w->x) / w->steps;
    w->vy = (y - w->y) / w->steps;
}

/* Goemon (the area's own picture of him) with his shadow, moving to a target in steps. Each
 * picture is drawn at an offset from (x, y). */
typedef struct SubtGoemonWork {
    /* 0x00 */ DLFrame *frame;
    /* 0x04 */ u16 kind;
    /* 0x08 */ f32 steps;
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 vx;
    /* 0x14 */ f32 targetX;
    /* 0x18 */ f32 y;
    /* 0x1C */ f32 vy;
    /* 0x20 */ f32 targetY;
    /* 0x24 */ u32 tex;
    /* 0x28 */ s16 texX;
    /* 0x2A */ s16 texY;
    /* 0x2C */ s16 texW;
    /* 0x2E */ s16 texH;
    /* 0x30 */ u32 shadow; /* 0: none */
    /* 0x34 */ s16 shadowX;
    /* 0x36 */ s16 shadowY;
    /* 0x38 */ s16 shadowW;
    /* 0x3A */ s16 shadowH;
} SubtGoemonWork;

#define SUBT_GOEMON(obj) ((SubtGoemonWork *) FX(obj))

s32 subtGoemonProc(Obj *obj, s16 msg, void *arg) {
    SubtGoemonWork *w = SUBT_GOEMON(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "goemon";
            w->kind = (s32) arg;
            w->frame = (DLFrame *) frmCreate(gFrmLayers[0], gFrmKindDlist);
            if (w->frame == NULL) {
                return -1;
            }
            switch (w->kind) {
                case 0:
                default:
                    w->tex = (u32) subtGoemonKind0Tex;
                    w->texW = 0x70;
                    w->texH = 0xDB;
                    w->shadow = (u32) subtGoemonShadowKind0Tex;
                    w->shadowX = 0x3E;
                    w->shadowY = 0x25;
                    w->shadowW = 0x30;
                    w->shadowH = 0x83;
                    break;
                case 1:
                    w->tex = (u32) subtGoemonKind1Tex;
                    w->texX = -0x3E;
                    w->texW = 0xA6;
                    w->texH = 0xD9;
                    w->shadow = (u32) subtGoemonShadowKind1Tex;
                    w->shadowX = -0x32;
                    w->shadowY = 0x28;
                    w->shadowW = 0xA0;
                    w->shadowH = 0x80;
                    break;
                case 2:
                    w->tex = (u32) subtGoemonKind2Tex;
                    w->texX = -0x36;
                    w->texY = 9;
                    w->texW = 0x93;
                    w->texH = 0xC3;
                    w->shadow = (u32) subtGoemonShadowKind2Tex;
                    w->shadowX = -0x21;
                    w->shadowY = 0x28;
                    w->shadowW = 0x90;
                    w->shadowH = 0x80;
                    break;
                case 3:
                    w->tex = (u32) subtGoemonKind3Tex;
                    w->texX = -0x20;
                    w->texW = 0x89;
                    w->texH = 0xD8;
                    w->shadow = (u32) subtGoemonShadowKind3Tex;
                    w->shadowX = -0xF;
                    w->shadowY = 0x28;
                    w->shadowW = 0x90;
                    w->shadowH = 0x80;
                    break;
                case 4:
                    w->tex = (u32) subtGoemonKind4Tex;
                    w->texX = -0x40;
                    w->texW = 0xBD;
                    w->texH = 0xE0;
                    w->shadow = 0;
                    break;
            }
            w->tex = floadSegToRam(w->tex, 0x10C, 0);
            w->shadow = floadSegToRam(w->shadow, 0x10C, 0);
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
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            if (w->shadow != 0) {
                gGfxDlHead = titleSetTint(gGfxDlHead, obj->file, 0, 0, 0, 0x80);
                gGfxDlHead = titleDrawImage4b(gGfxDlHead, w->x + w->shadowX, w->y + w->shadowY,
                                                    1.0f, w->shadowW, w->shadowH, w->shadow);
            }
            gGfxDlHead = titleDrawRgba16(gGfxDlHead, obj->file, w->x + w->texX, w->y + w->texY,
                                                1.0f, 255, w->texW, w->texH, w->tex);
            gGfxDlHead++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Goemon at (x, y), the area's picture of him by kind. */
Obj *subtGoemonMake(Obj *parent, s32 kind, f32 x, f32 y) {
    Obj *o;

    if ((o = tskCreate(parent, subtGoemonProc, parent->file, (void *) kind)) != NULL) {
        SUBT_GOEMON(o)->x = x;
        SUBT_GOEMON(o)->y = y;
    }
    return o;
}

void subtGoemonSetSteps(Obj *obj, s32 steps) {
    SUBT_GOEMON(obj)->steps = steps;
}

/* Goemon's target, reached in his steps. */
void subtGoemonMoveTo(Obj *obj, f32 x, f32 y) {
    SubtGoemonWork *w = SUBT_GOEMON(obj);

    w->targetX = x;
    w->targetY = y;
    w->vx = (x - w->x) / w->steps;
    w->vy = (y - w->y) / w->steps;
}

/* "moji": a tile of letters (s, t; width x height) at (x, y) in yellow, its alpha moving by a
 * step. Nothing in this file makes one. */
typedef struct SubtMojiWork {
    /* 0x00 */ DLFrame *frame;
    /* 0x04 */ s32 x;
    /* 0x08 */ s32 y;
    /* 0x0C */ s32 width;
    /* 0x10 */ s32 height;
    /* 0x14 */ s32 s;
    /* 0x18 */ s32 t;
    /* 0x1C */ s32 alpha;
    /* 0x20 */ s32 alphaStep;
} SubtMojiWork;

#define SUBT_MOJI(obj) ((SubtMojiWork *) FX(obj))

s32 subtMojiProc(Obj *obj, s16 msg, void *arg) {
    SubtMojiWork *w = SUBT_MOJI(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "moji";
            w->frame = (DLFrame *) frmCreate(gFrmLayers[0], gFrmKindDlist);
            if (w->frame == NULL) {
                return -1;
            }
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
            gSPDisplayList(gGfxDlHead++, fadeSegToRam(diaryLetterDl, obj->file));
            gDPSetEnvColor(gGfxDlHead++, 255, 255, 0, w->alpha);
            gGfxDlHead = diaryDrawTile4b(gGfxDlHead, w->x, w->y, w->width, w->height, w->s, w->t);
            gGfxDlHead++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Empty, called by nothing; splat folds it into subtMojiProc's asm (its last jr ra; nop). */
void subtUnusedStub(void) {
}

/* "nomaki": the area's name on a scroll, centred on (x, y), fading in. */
typedef struct SubtNomakiWork {
    /* 0x00 */ DLFrame *frame;
    /* 0x04 */ s32 alpha;
    /* 0x08 */ u32 tex;
    /* 0x0C */ u8 padC[4];
    /* 0x10 */ s32 kind;
    /* 0x14 */ s32 x;
    /* 0x18 */ s32 y;
    /* 0x1C */ s32 width;
    /* 0x20 */ s32 height;
} SubtNomakiWork;

#define SUBT_NOMAKI(obj) ((SubtNomakiWork *) FX(obj))

s32 subtNomakiProc(Obj *obj, s16 msg, void *arg) {
    SubtNomakiWork *w = SUBT_NOMAKI(obj);
    u8 *tex;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "nomaki";
            w->kind = (s32) arg;
            w->alpha = 0;
            w->frame = (DLFrame *) frmCreate(gFrmLayers[0], gFrmKindDlist);
            if (w->frame == NULL) {
                return -1;
            }
            /* Each case's settings on one line: IDO schedules by line, and only this puts the
             * width's store in the branch's delay slot as the game has it. */
            switch (w->kind) {
                case 0:
                default:
                    w->width = 0x100; w->height = 0x13; tex = subtNomakiKind0Tex;
                    break;
                case 1:
                    w->width = 0x110; w->height = 0x14; tex = subtNomakiKind1Tex;
                    break;
                case 2:
                    w->width = 0xD0; w->height = 0x10; tex = subtNomakiKind2Tex;
                    break;
                case 3:
                    w->width = 0x110; w->height = 0x24; tex = subtNomakiKind3Tex;
                    break;
                case 4:
                    w->width = 0xE0; w->height = 0x14; tex = subtNomakiKind4Tex;
                    break;
            }
            w->tex = floadSegToRam((u32) tex, 0x10C, 0);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->alpha += 4;
            if (w->alpha > 255) {
                w->alpha = 255;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gGfxDlHead = titleSetTint(gGfxDlHead, obj->file, 255, 255, 0, w->alpha);
            gGfxDlHead = titleDrawImage4b(gGfxDlHead, w->x - w->width / 2, w->y - w->height / 2,
                                                1.0f, w->width, w->height, w->tex);
            gGfxDlHead++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The scroll centred on (x, y), the area's name by kind. */
Obj *subtNomakiMake(Obj *parent, s32 x, s32 y, s32 kind) {
    Obj *o;

    if ((o = tskCreate(parent, subtNomakiProc, parent->file, (void *) kind)) != NULL) {
        SUBT_NOMAKI(o)->x = x;
        SUBT_NOMAKI(o)->y = y;
    }
    return o;
}

/* The title card's task: its camera, and the script run each frame (-1 from it ends the task). */
typedef struct SubtWork SubtWork;
struct SubtWork {
    /* 0x00 */ s32 (*script)(Obj *obj, SubtWork *w);
    /* 0x04 */ CamView *camera;
    /* 0x08 */ u8 area;
    /* 0x0A */ u16 time; /* frames since it began */
    /* 0x0C */ Obj *hokusai;
    /* 0x10 */ Obj *kanban; /* the area's board */
    /* 0x14 */ Obj *titleKanban; /* the title's board */
    /* 0x18 */ Obj *goemon;
};

#define SUBT(obj) ((SubtWork *) FX(obj))

s32 subtScript(Obj *obj, SubtWork *w);

s32 subtProc(Obj *obj, s16 msg, void *arg) {
    SubtWork *w = SUBT(obj);
    LightFrame *light;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "subt";
            gRetracesPerFrame = 1;
            w->area = gGameWork.area;
            w->camera = (CamView *) frmCreate(gFrmLayers[0], gFrmKindCamera);
            if (w->camera == NULL) {
                return -1;
            }
            w->camera->ortho |= 1;
            w->camera->zNear = -200.0f;
            w->camera->zFar = 200.0f;
            frmCreate(&w->camera->f, gFrmKindAmbi);
            frmCreate(&w->camera->f, gFrmKindLight);
            light = (LightFrame *) frmCreate(&w->camera->f, gFrmKindLight);
            light->color[0] = 0;
            light->color[1] = 0;
            light->color[2] = 0;
            floadLoadSet(0x10C);
            w->script = subtScript;
            break;
        case 1:
            frmDestroy(&w->camera->f);
            break;
        case 0:
            w->time++;
            if (w->script != NULL && w->script(obj, w) != 0) {
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The title card's script, by the frame. */
s32 subtScript(Obj *obj, SubtWork *w) {
    if (w->time == 1) {
        w->hokusai = subtHokusaiMake(obj);
        subtHokusaiFadeIn(w->hokusai, 32);
        w->titleKanban = subtKanbanMake(obj, -1, 70.0f, -50.0f);
        w->kanban = subtKanbanMake(obj, w->area, 52.0f, -70.0f);
        sndPlay(0xB6);
    } else if (w->time == 0x20) {
        subtKanbanSetSteps(w->titleKanban, 16);
        subtKanbanMoveTo(w->titleKanban, 70.0f, 76.0f);
    } else if (w->time == 0x28) {
        subtKanbanSetSteps(w->kanban, 16);
        subtKanbanMoveTo(w->kanban, 52.0f, 56.0f);
    } else if (w->time == 0x30) {
        w->goemon = subtGoemonMake(obj, w->area, 320.0f, 8.0f);
        subtGoemonSetSteps(w->goemon, 16);
        subtGoemonMoveTo(w->goemon, 192.0f, 8.0f);
    } else if (w->time == 0x38) {
        subtNomakiMake(obj, 160, 192, w->area);
    } else if (w->time == 0x10A) {
        fadeCreate(gGameWork.gameProcTask);
        fadeSetColor(0, 0, 0);
        fadeSetLevel(0.0f);
        fadeSetSpeed(32.0f);
        fadeSetTarget(255.0f);
    } else if (w->time == 0x15A) {
        if (gGameWork.area == 0) {
            stageGotoAtlasFrom(NULL, gGameWork.area, 0);
        } else {
            stageGotoAtlasFrom(NULL, gGameWork.area, -4);
        }
    }
    return 0;
}
