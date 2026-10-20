#include "tlb.h"
#include "tlb/tlb_0017.h"

/* Segment addresses: the shine's and the cast's animations and segment files, the light ball's
 * display list. */
extern u8 effKagayakeFrame0Anim[];
extern u8 effKagayakeFrame1Anim[];
extern u8 effKagayakeFrame2Anim[];
extern u8 effKagayakeFrame3Anim[];
extern u8 effKagayakeFrame3Tex[];
extern u8 effAllcastFrame0Anim[];
extern u8 effAllcastFrame1Anim[];
extern u8 effAllcastFrame3Anim[];
extern u8 effAllcastFrame3Tex[];
extern u8 effAllcastFrame2Anim[];
extern u8 effLballDl[];

/* "kagayake" (a shine): four "anime" frames playing one animation together, from a frame
 * number up to an end at a step (effKagayakeSetPlay sets them). */
typedef struct KagayakeWork {
    /* 0x00 */ AnimeFrame *frames[4];
    /* 0x10 */ f32 frame; /* the four frames' animation frame */
    /* 0x14 */ f32 step;
    /* 0x18 */ f32 end;
} KagayakeWork;

#define KAGAYAKE(obj) ((KagayakeWork *) FX(obj))

s32 effKagayakeProc(Obj *obj, s16 msg, void *arg) {
    KagayakeWork *w = KAGAYAKE(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "kagayake";
            w->frames[0] = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frames[0] == NULL) {
                return -1;
            }
            w->frames[0]->anim = effKagayakeFrame0Anim;
            floadGetSetRam((void **) w->frames[0]->segs, 0xEF);
            w->frames[0]->scale[2] = 20.0f;
            w->frames[0]->scale[0] = w->frames[0]->scale[1] = w->frames[0]->scale[2];
            w->frames[1] = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frames[1] == NULL) {
                return -1;
            }
            w->frames[1]->anim = effKagayakeFrame1Anim;
            floadGetSetRam((void **) w->frames[1]->segs, 0x13C);
            w->frames[1]->scale[2] = 20.0f;
            w->frames[1]->scale[0] = w->frames[1]->scale[1] = w->frames[1]->scale[2];
            w->frames[2] = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frames[2] == NULL) {
                return -1;
            }
            w->frames[2]->anim = effKagayakeFrame2Anim;
            floadGetSetRam((void **) w->frames[2]->segs, 0xEF);
            w->frames[2]->scale[2] = 20.0f;
            w->frames[2]->scale[0] = w->frames[2]->scale[1] = w->frames[2]->scale[2];
            w->frames[3] = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frames[3] == NULL) {
                return -1;
            }
            w->frames[3]->anim = effKagayakeFrame3Anim;
            floadGetSetRam((void **) w->frames[3]->segs, 0xF4);
            w->frames[3]->scale[2] = 10.0f;
            w->frames[3]->scale[0] = w->frames[3]->scale[1] = w->frames[3]->scale[2];
            w->frames[3]->segs[3] = floadSegToRam((u32) effKagayakeFrame3Tex, 0xF3, 0);
            w->frame = 0.0f;
            w->step = 0.0f;
            w->end = 0.0f;
            break;
        case 1:
            frmDestroy(&w->frames[1]->f);
            frmDestroy(&w->frames[0]->f);
            frmDestroy(&w->frames[2]->f);
            frmDestroy(&w->frames[3]->f);
            break;
        case 0:
            w->frame += w->step;
            if (w->end < w->frame) {
                w->frame = w->end;
            }
            w->frames[0]->frame = w->frame;
            w->frames[1]->frame = w->frame;
            w->frames[2]->frame = w->frame;
            w->frames[3]->frame = w->frame;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *effKagayakeMake(Obj *obj) {
    return tskCreate(obj, effKagayakeProc, obj->file, NULL);
}

/* The shine plays from start to end in dur steps. */
void effKagayakeSetPlay(Obj *obj, f32 dur, f32 start, f32 end) {
    KagayakeWork *w = KAGAYAKE(obj);

    w->frame = start;
    w->end = end;
    w->step = (end - start) / dur;
}

/* "allcast": four "anime" frames, a tenth of their size. */
typedef struct AllcastWork {
    /* 0x00 */ AnimeFrame *frames[4];
    /* 0x10 */ AnimeFrame *unused10; /* destroyed with the four frames, never made */
} AllcastWork;

#define ALLCAST(obj) ((AllcastWork *) FX(obj))

s32 effAllcastProc(Obj *obj, s16 msg, void *arg) {
    AllcastWork *w = ALLCAST(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "allcast";
            w->frames[0] = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frames[0] == NULL) {
                return -1;
            }
            w->frames[0]->anim = effAllcastFrame0Anim;
            floadGetSetRam((void **) w->frames[0]->segs, 0xEF);
            w->frames[0]->scale[2] = 0.1f;
            w->frames[0]->scale[0] = w->frames[0]->scale[1] = w->frames[0]->scale[2];
            w->frames[0]->rot[1] = 0.0f;
            w->frames[1] = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frames[1] == NULL) {
                return -1;
            }
            w->frames[1]->anim = effAllcastFrame1Anim;
            floadGetSetRam((void **) w->frames[1]->segs, 0xEF);
            w->frames[1]->scale[2] = 0.1f;
            w->frames[1]->scale[0] = w->frames[1]->scale[1] = w->frames[1]->scale[2];
            w->frames[1]->rot[1] = 0.0f;
            w->frames[3] = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frames[3] == NULL) {
                return -1;
            }
            w->frames[3]->anim = effAllcastFrame3Anim;
            floadGetSetRam((void **) w->frames[3]->segs, 0xC7);
            w->frames[3]->scale[2] = 0.1f;
            w->frames[3]->scale[0] = w->frames[3]->scale[1] = w->frames[3]->scale[2];
            w->frames[3]->rot[1] = 0.0f;
            w->frames[3]->segs[3] = floadSegToRam((u32) effAllcastFrame3Tex, 0xC7, 0);
            w->frames[2] = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frames[2] == NULL) {
                return -1;
            }
            w->frames[2]->anim = effAllcastFrame2Anim;
            floadGetSetRam((void **) w->frames[2]->segs, 0xC5);
            w->frames[2]->scale[2] = 0.1f;
            w->frames[2]->scale[0] = w->frames[2]->scale[1] = w->frames[2]->scale[2];
            w->frames[2]->rot[1] = 0.0f;
            break;
        case 1:
            frmDestroy(&w->unused10->f);
            frmDestroy(&w->frames[0]->f);
            frmDestroy(&w->frames[3]->f);
            frmDestroy(&w->frames[2]->f);
            frmDestroy(&w->frames[1]->f);
            break;
        case 0:
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *effAllcastMake(Obj *obj) {
    return tskCreate(obj, effAllcastProc, obj->file, NULL);
}

/* "lball" (a light ball): a "pol" frame moving at a set speed towards a target, its size
 * and its alpha moving too (the setters after it). */
typedef struct LballWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 steps; /* how many steps a move takes */
    /* 0x08 */ f32 target[3];
    /* 0x14 */ f32 vel[3]; /* per step, on each axis */
    /* 0x20 */ f32 scale;
    /* 0x24 */ f32 scaleTarget;
    /* 0x28 */ f32 scaleStep;
    /* 0x2C */ s32 alpha;
    /* 0x30 */ s32 alphaStep;
} LballWork;

#define LBALL(obj) ((LballWork *) FX(obj))

s32 effLballProc(Obj *obj, s16 msg, void *arg) {
    LballWork *w = LBALL(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "lball";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = (Gfx *) effLballDl;
            w->frame->flags = 7;
            floadGetSetRam((void **) w->frame->segs, 0x120);
            w->alpha = 255;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (0.0001f < w->vel[0]) {
                if (fabsf(w->target[0] - w->frame->pos[0]) < w->vel[0]) {
                    w->frame->pos[0] = w->target[0];
                    w->vel[0] = 0.0f;
                } else if (w->target[0] < w->frame->pos[0]) {
                    w->frame->pos[0] -= w->vel[0];
                } else {
                    w->frame->pos[0] += w->vel[0];
                }
            }
            if (0.0001f < w->vel[1]) {
                if (fabsf(w->target[1] - w->frame->pos[1]) < w->vel[1]) {
                    w->frame->pos[1] = w->target[1];
                    w->vel[1] = 0.0f;
                } else if (w->target[1] < w->frame->pos[1]) {
                    w->frame->pos[1] -= w->vel[1];
                } else {
                    w->frame->pos[1] += w->vel[1];
                }
            }
            if (0.0001f < w->vel[2]) {
                if (fabsf(w->target[2] - w->frame->pos[2]) < w->vel[2]) {
                    w->frame->pos[2] = w->target[2];
                    w->vel[2] = 0.0f;
                } else if (w->target[2] < w->frame->pos[2]) {
                    w->frame->pos[2] -= w->vel[2];
                } else {
                    w->frame->pos[2] += w->vel[2];
                }
            }
            if (0.0001f < w->scaleStep) {
                if (fabsf(w->scaleTarget - w->scale) < w->scaleStep) {
                    w->scale = w->scaleTarget;
                    w->scaleStep = 0.0f;
                } else if (w->scaleTarget < w->scale) {
                    w->scale -= w->scaleStep;
                } else {
                    w->scale += w->scaleStep;
                }
            }
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale;
            w->alpha += w->alphaStep;
            if (w->alpha < 0) {
                w->alpha = 0;
            } else if (w->alpha > 255) {
                w->alpha = 255;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetEnvColor(gGfxDlHead++, 255, 255, 255, w->alpha);
            gDPSetCombine(gGfxDlHead++, 0xFF9BFF, 0xFF36FF7F);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A light ball at (x, y, z). */
Obj *effLballMake(Obj *parent, f32 x, f32 y, f32 z) {
    Obj *o;

    if ((o = tskCreate(parent, effLballProc, parent->file, NULL)) != NULL) {
        LBALL(o)->frame->pos[0] = x;
        LBALL(o)->frame->pos[1] = y;
        LBALL(o)->frame->pos[2] = z;
    }
    return o;
}

void effLballSetScale(Obj *obj, f32 scale) {
    LBALL(obj)->scale = scale;
}

void effLballSetSteps(Obj *obj, s32 steps) {
    LBALL(obj)->steps = steps;
}

void effLballSetAlpha(Obj *obj, s32 alpha) {
    LBALL(obj)->alpha = alpha;
}

/* The alpha's step to reach alpha in the move's steps (rounded up). */
void effLballFadeTo(Obj *obj, s32 alpha) {
    LballWork *w = LBALL(obj);

    w->alphaStep = ((alpha - w->alpha) + w->steps - 1.0f) / w->steps;
}

/* The size's target, reached in the move's steps. */
void effLballScaleTo(Obj *obj, f32 scale) {
    LballWork *w = LBALL(obj);

    w->scaleTarget = scale;
    w->scaleStep = fabsf(scale - w->scale) / w->steps;
}

/* The move's target, reached in its steps on each axis. */
void effLballMoveTo(Obj *obj, f32 x, f32 y, f32 z) {
    LballWork *w = LBALL(obj);

    w->target[0] = x;
    w->target[1] = y;
    w->target[2] = z;
    w->vel[0] = fabsf(x - w->frame->pos[0]) / w->steps;
    w->vel[1] = fabsf(y - w->frame->pos[1]) / w->steps;
    w->vel[2] = fabsf(z - w->frame->pos[2]) / w->steps;
}
