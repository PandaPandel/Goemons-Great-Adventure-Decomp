#include "tlb.h"
#include "tlb/tlb_0016.h"

/* File 16, the world map's second object: its 3D pieces. The sea, a waterfall ("taki2") and the
 * lava ("magma") under the map, the two runs of Obisumaru ("obisu") across it with the camera
 * set on them, and the "impact" that flies from the camera to a place on the map and takes the
 * game there. The first object (tlb_0016_0.c) is the map's own task. */

extern u8 mapSeaDl[];
extern u8 mapWaterfallDl[];
extern u8 mapMagma1Dl[];
extern u8 mapMagma2Dl[];
extern u8 mapMagma3Dl[];
extern u8 mapObisuAnim[];
extern u8 mapMagmaTex[];
extern u8 mapImpactAnim[];

/* The sea's, the waterfall's and the lava's work (task + 0x20). */
typedef struct SeaWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ s32 scroll; /* the texture's, down one a frame (its low half is used) */
} SeaWork;

/* "obisu"'s work (task + 0x20): Obisumaru walking along the map's course 1. */
typedef struct ObisuWork {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 dist; /* along course 1 */
    /* 0x08 */ f32 y;
    /* 0x0C */ u16 count; /* steps: gone after 120 */
    /* 0x10 */ Camera *cam; /* the map's */
} ObisuWork;

/* "impact"'s work (task + 0x20). Each of x, y, z and rx moves by its step to its target, the
 * step set to (target - now) / steps (impactMoveTo, impactTurnTo). */
typedef struct ImpactWork {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ s8 dir;   /* 1: to the next area, 2: to the one before */
    /* 0x05 */ s8 state; /* 0 growing, 1 flying, 3 fading out */
    /* 0x06 */ u16 count; /* steps in this state */
    /* 0x08 */ f32 grow;  /* added to scale each step */
    /* 0x0C */ f32 scale;
    /* 0x10 */ f32 speed; /* the animation's */
    /* 0x14 */ f32 steps;
    /* 0x18 */ f32 x;
    /* 0x1C */ f32 vx;
    /* 0x20 */ f32 tx;
    /* 0x24 */ f32 y;
    /* 0x28 */ f32 vy;
    /* 0x2C */ f32 ty;
    /* 0x30 */ f32 z;
    /* 0x34 */ f32 vz;
    /* 0x38 */ f32 tz;
    /* 0x3C */ f32 rx;
    /* 0x40 */ f32 vrx;
    /* 0x44 */ f32 trx;
} ImpactWork;

#define IMPACT(task) ((ImpactWork *) ((u32) (task) + sizeof(Obj)))


Gfx *mapMagmaDL(Gfx *gfx, u16 scroll);
s32 mapSeaProc(Obj *obj, s16 msg, void *arg);
s32 mapTaki2Proc(Obj *obj, s16 msg, void *arg);
s32 mapMagmaProc(Obj *obj, s16 msg, void *arg);
s32 mapObisuProc(Obj *obj, s16 msg, void *arg);
s32 mapObisu2Proc(Obj *obj, s16 msg, void *arg);
void mapImpactMoveTo(Obj *obj, f32 x, f32 y, f32 z);
void mapImpactTurnTo(Obj *obj, f32 rx);
s32 mapImpactLeaveProc(Obj *obj, s16 msg, void *arg);
s32 mapImpactArriveProc(Obj *obj, s16 msg, void *arg);

/* A 32x32 RGBA16 texture loaded into TMEM at tmem for tile rtile, as gDPLoadMultiBlock, with its
 * tile size at (uls, ult) (as tlb_0050.c's: one source line). */
#define gDPLoadScrollBlock(pkt, timg, tmem, rtile, uls, ult)                                          \
    {                                                                                                  \
        gDPSetTextureImage(pkt, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, timg);                                 \
        gDPSetTile(pkt, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, tmem, G_TX_LOADTILE, 0,                        \
                   G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD); \
        gDPLoadSync(pkt);                                                                              \
        gDPLoadBlock(pkt, G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES));        \
        gDPPipeSync(pkt);                                                                              \
        gDPSetTile(pkt, G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, tmem, rtile, 0, G_TX_WRAP | G_TX_NOMIRROR, 5,  \
                   G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD);                              \
        gDPSetTileSize(pkt, rtile, uls, ult, (32 - 1) << 2, (32 - 1) << 2);                            \
    }

/* The lava's texture, its two layers scrolling against each other. */
Gfx *mapMagmaDL(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 128, 128, 128, 128);
    gDPLoadScrollBlock(gfx++, mapMagmaTex, 0, G_TX_RENDERTILE, scroll, scroll);
    gDPLoadScrollBlock(gfx++, mapMagmaTex, 0x100, 1, scroll + 10, -scroll);
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* "sea": the sea under the map, at the height its argument gives. */
s32 mapSeaProc(Obj *obj, s16 msg, void *arg) {
    SeaWork *w = (SeaWork *) ((u32) obj + sizeof(Obj));
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "sea";
            w->frame = (PolFrame *) frmCreate(gFrmLayers[6], gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = (Gfx *) mapSeaDl;
            floadGetSetRam((void **) w->frame->segs, 0x113);
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[1] = (s32) arg;
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gGfxDlHead = mapSeaDL(gGfxDlHead, w->scroll);
            w->scroll--;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "taki1": the first waterfall's handler, left empty (its name is all that is left of it). The
 * game has an empty function here (the extra "jr ra; nop" at the end of mapSeaProc's .s:
 * splat folds a function nothing calls into the one before it) and the unreferenced string
 * "taki1" between "sea" and "taki2" in the file's .rodata: a dead print keeps it, no code. */
void mapTaki1Proc(void) {
    PRINTF("taki1");
}

/* "taki2": a waterfall. */
s32 mapTaki2Proc(Obj *obj, s16 msg, void *arg) {
    SeaWork *w = (SeaWork *) ((u32) obj + sizeof(Obj));
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "taki2";
            w->frame = (PolFrame *) frmCreate(gFrmLayers[6], gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = (Gfx *) mapWaterfallDl;
            floadGetSetRam((void **) w->frame->segs, 0x115);
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gGfxDlHead = mapTaki2DL(gGfxDlHead, w->scroll);
            w->scroll--;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "magma": the lava, model 1-3 (its argument). */
s32 mapMagmaProc(Obj *obj, s16 msg, void *arg) {
    SeaWork *w = (SeaWork *) ((u32) obj + sizeof(Obj));
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "magma";
            w->frame = (PolFrame *) frmCreate(gFrmLayers[6], gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            switch ((s32) arg) {
                case 1:
                    w->frame->dl2 = (Gfx *) mapMagma1Dl;
                    break;
                case 2:
                    w->frame->dl2 = (Gfx *) mapMagma2Dl;
                    break;
                case 3:
                    w->frame->dl2 = (Gfx *) mapMagma3Dl;
                    break;
            }
            floadGetSetRam((void **) w->frame->segs, 0x116);
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gGfxDlHead = mapMagmaDL(gGfxDlHead, w->scroll);
            w->scroll--;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "obisu": Obisumaru running along the map's course 1, the map's camera held on him, for 120
 * steps. */
s32 mapObisuProc(Obj *obj, s16 msg, void *arg) {
    ObisuWork *w = (ObisuWork *) ((u32) obj + sizeof(Obj));
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "obisu";
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            floadGetSetRam((void **) w->frame->segs, 0x88);
            w->frame->anim = mapObisuAnim;
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->dist = 701.0f;
            w->y = -15.0f;
            mapSetHold(obj->parent, 1);
            mapSetCamHold(obj->parent, 1);
            w->cam = mapGetCamera(obj->parent);
            camSetAt(w->cam, 99.0f, 67.0f, -193.0f);
            camSetEye(w->cam, 147.0f, 111.0f, -296.0f);
            camSetFov(w->cam, 1.0471976f);
            break;
        case 1:
            mapSetHold(obj->parent, 0);
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (++w->count > 120) {
                return -1;
            }
            w->frame->frame += 0.4;
            if (w->frame->frame >= polAnimeLastFrame(w->frame) + 1) {
                w->frame->frame -= polAnimeLastFrame(w->frame) + 1;
            }
            w->dist += -1.75f;
            w->y += -0.35f;
            vramCoursePointXZ(1, w->dist, 0.0f, &w->frame->pos[0], &w->frame->pos[2]);
            w->frame->pos[1] = w->y;
            w->frame->rot[1] = vramCourseHeading(1, w->dist) + PI;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "obisu": the second run, from 631 along course 1. */
s32 mapObisu2Proc(Obj *obj, s16 msg, void *arg) {
    ObisuWork *w = (ObisuWork *) ((u32) obj + sizeof(Obj));
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "obisu";
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            floadGetSetRam((void **) w->frame->segs, 0x88);
            w->frame->anim = mapObisuAnim;
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->dist = 631.0f;
            w->y = 0.0f;
            mapSetHold(obj->parent, 1);
            mapSetCamHold(obj->parent, 1);
            w->cam = mapGetCamera(obj->parent);
            camSetAt(w->cam, -49.0f, 36.0f, -164.0f);
            camSetEye(w->cam, -21.0f, 149.0f, -273.0f);
            camSetFov(w->cam, 0.87266463f);
            break;
        case 1:
            mapSetHold(obj->parent, 0);
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (++w->count > 120) {
                return -1;
            }
            w->frame->frame += 0.4;
            if (w->frame->frame >= polAnimeLastFrame(w->frame) + 1) {
                w->frame->frame -= polAnimeLastFrame(w->frame) + 1;
            }
            w->dist += -2.4416666f;
            w->y += 0.0f;
            vramCoursePointXZ(1, w->dist, 0.0f, &w->frame->pos[0], &w->frame->pos[2]);
            w->frame->pos[1] = w->y;
            w->frame->rot[1] = vramCourseHeading(1, w->dist) + PI;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Sends the impact to (x, y, z) in its steps. */
void mapImpactMoveTo(Obj *obj, f32 x, f32 y, f32 z) {
    ImpactWork *w = IMPACT(obj);

    w->tx = x;
    w->ty = y;
    w->tz = z;
    w->vx = (x - w->x) / w->steps;
    w->vy = (y - w->y) / w->steps;
    w->vz = (z - w->z) / w->steps;
}

/* Turns the impact to rx in its steps. */
void mapImpactTurnTo(Obj *obj, f32 rx) {
    ImpactWork *w = IMPACT(obj);

    w->trx = rx;
    w->vrx = (rx - w->rx) / w->steps;
}

/* "impact": grows on the map, flies at the camera, then fades the screen out and takes the game
 * to the next area or the one before (dir). */
s32 mapImpactLeaveProc(Obj *obj, s16 msg, void *arg) {
    ImpactWork *w = (ImpactWork *) (obj + 1);
    s32 pad;
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "impact";
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            floadGetSetRam((void **) w->frame->segs, 0xBD);
            w->frame->anim = mapImpactAnim;
            w->scale = 0.01f;
            w->speed = 0.0f;
            mapSetHold(obj->parent, 1);
            break;
        case 1:
            mapSetHold(obj->parent, 0);
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->count++;
            switch (w->state) {
                case 0:
                    w->scale += 0.02;
                    if (w->scale >= 0.2) {
                        w->scale = 0.2f;
                        w->count = 0;
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (w->count == 6) {
                        sndPlay(0x15E);
                    }
                    if (w->count == 1) {
                        w->speed = 0.5f;
                    } else if (w->count == 4) {
                        w->steps = 30.0f;
                        mapImpactMoveTo(obj, gGameWork.view->eye[0], gGameWork.view->eye[1] + 10.0f,
                                               gGameWork.view->eye[2]);
                        w->steps = 16.0f;
                        mapImpactTurnTo(obj, mathAtan2(sqrtf((w->tx - w->x) * (w->tx - w->x) +
                                                                    (w->tz - w->z) * (w->tz - w->z)),
                                                              w->ty - w->y));
                    } else if (w->count > 40) {
                        w->count = 0;
                        w->state = 3;
                    }
                    break;
                case 3:
                    if (w->count == 1) {
                        tskDestroy(fadeCreate(gGameWork.gameProcTask));
                        fadeCreate(gGameWork.gameProcTask);
                        fadeSetSpeed(16.0f);
                        fadeSetColor(0, 0, 0);
                        fadeSetLevel(0.0f);
                        fadeSetTarget(255.0f);
                    } else if (w->count == 18) {
                        switch (w->dir) {
                            case 1:
                                stageGotoAtlasFrom(obj->parent, gGameWork.area + 1, -2);
                                break;
                            case 2:
                                stageGotoAtlasFrom(obj->parent, gGameWork.area - 1, -1);
                                break;
                        }
                    }
                    break;
            }
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale;
            w->frame->frame += w->speed;
            if (w->frame->frame > polAnimeLastFrame(w->frame)) {
                w->frame->frame = polAnimeLastFrame(w->frame);
            }
            if (w->vx != 0.0f) {
                if (fabsf(w->x - w->tx) <= fabsf(w->vx)) {
                    w->x = w->tx;
                    w->vx = 0.0f;
                } else {
                    w->x += w->vx;
                }
            }
            if (w->vy != 0.0f) {
                if (fabsf(w->y - w->ty) <= fabsf(w->vy)) {
                    w->y = w->ty;
                    w->vy = 0.0f;
                } else {
                    w->y += w->vy;
                }
            }
            if (w->vz != 0.0f) {
                if (fabsf(w->z - w->tz) <= fabsf(w->vz)) {
                    w->z = w->tz;
                    w->vz = 0.0f;
                } else {
                    w->z += w->vz;
                }
            }
            if (w->vrx != 0.0f) {
                if (fabsf(w->rx - w->trx) <= fabsf(w->vrx)) {
                    w->rx = w->trx;
                    w->vrx = 0.0f;
                } else {
                    w->rx += w->vrx;
                }
            }
            w->frame->pos[0] = w->x;
            w->frame->pos[1] = w->y;
            w->frame->pos[2] = w->z;
            w->frame->rot[0] = w->rx;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Makes an "impact" under parent at (x, y, z), facing the camera. */
void mapImpactLeaveMake(Obj *parent, s32 dir, f32 x, f32 y, f32 z) {
    Obj *t = tskCreate(parent, mapImpactLeaveProc, parent->file, NULL);

    if (t != NULL) {
        IMPACT(t)->dir = dir;
        IMPACT(t)->x = x;
        IMPACT(t)->y = y;
        IMPACT(t)->z = z;
        IMPACT(t)->frame->rot[1] = mathAtan2(gGameWork.view->eye[0] - x, gGameWork.view->eye[2] - z);
    }
}

/* "impact": the other way, from the camera down to a place on the map, shrinking. */
s32 mapImpactArriveProc(Obj *obj, s16 msg, void *arg) {
    ImpactWork *w = (ImpactWork *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "impact";
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            floadGetSetRam((void **) w->frame->segs, 0xBD);
            w->frame->anim = mapImpactAnim;
            w->scale = 0.2f;
            mapPlayersHide();
            mapSetHold(obj->parent, 1);
            break;
        case 1:
            mapPlayersShow();
            mapSetHold(obj->parent, 0);
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (++w->count == 1) {
                w->grow = -0.0025f;
                if (sndGetVolume(0x15E, 0) == 0) {
                    sndPlay(0x15F);
                }
            } else if (w->count == 40) {
                return -1;
            }
            w->scale += w->grow;
            if (w->scale < 0.0f) {
                w->scale = 0.0f;
            }
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale;
            w->frame->frame += w->speed;
            if (w->frame->frame > polAnimeLastFrame(w->frame)) {
                w->frame->frame = polAnimeLastFrame(w->frame);
            }
            if (w->vx != 0.0f) {
                if (fabsf(w->x - w->tx) <= fabsf(w->vx)) {
                    w->x = w->tx;
                    w->vx = 0.0f;
                } else {
                    w->x += w->vx;
                }
            }
            if (w->vy != 0.0f) {
                if (fabsf(w->y - w->ty) <= fabsf(w->vy)) {
                    w->y = w->ty;
                    w->vy = 0.0f;
                } else {
                    w->y += w->vy;
                }
            }
            if (w->vz != 0.0f) {
                if (fabsf(w->z - w->tz) <= fabsf(w->vz)) {
                    w->z = w->tz;
                    w->vz = 0.0f;
                } else {
                    w->z += w->vz;
                }
            }
            if (w->vrx != 0.0f) {
                if (fabsf(w->rx - w->trx) <= fabsf(w->vrx)) {
                    w->rx = w->trx;
                    w->vrx = 0.0f;
                } else {
                    w->rx += w->vrx;
                }
            }
            w->frame->pos[0] = w->x;
            w->frame->pos[1] = w->y;
            w->frame->pos[2] = w->z;
            w->frame->rot[0] = w->rx;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Makes the other "impact" under parent: from the camera to (x, y, z), its animation run
 * backwards from its end. */
void mapImpactArriveMake(Obj *parent, s32 dir, f32 x, f32 y, f32 z) {
    Obj *t = tskCreate(parent, mapImpactArriveProc, parent->file, NULL);
    ImpactWork *w;
    s32 pad;

    if (t != NULL) {
        w = IMPACT(t);
        w->dir = dir;
        w->x = gGameWork.view->eye[0];
        w->y = gGameWork.view->eye[1] + 10.0f;
        w->z = gGameWork.view->eye[2];
        w->frame->frame = polAnimeLastFrame(w->frame);
        w->frame->rot[1] = mathAtan2(x - w->x, z - w->z);
        w->steps = 40.0f;
        mapImpactMoveTo(t, x, y, z);
        w->rx = mathAtan2(sqrtf((w->tx - w->x) * (w->tx - w->x) + (w->tz - w->z) * (w->tz - w->z)), w->ty - w->y);
    }
    PRINTF("atimpact(%1.0f,%1.0f,%1.0f) \n", x, y, z);
}

/* A map area's objects: the sea 46 down, the waterfall, and Obisumaru's first run once flag 7
 * is set and 0x9B is not. */
void mapSetupSeaFalls(Obj *obj) {
    tskCreate(obj, mapSeaProc, obj->file, (void *) -46);
    tskCreate(obj, mapTaki2Proc, obj->file, NULL);
    if (swWorkTest(7) && !swWorkTest(0x9B)) {
        floadLoadSet(0x88);
        tskCreate(obj, mapObisuProc, obj->file, NULL);
    }
}

/* A map area's objects: the sky's program 8, the sea and the three lavas. */
void mapSetupLava(Obj *obj) {
    camSunSetProgram(8);
    camSunSetFogNear(0x398);
    tskCreate(obj, mapSeaProc, obj->file, NULL);
    tskCreate(obj, mapMagmaProc, obj->file, (void *) 1);
    tskCreate(obj, mapMagmaProc, obj->file, (void *) 2);
    tskCreate(obj, mapMagmaProc, obj->file, (void *) 3);
}

/* A map area's objects: the sky's program 6 and the sea. */
void mapSetupSea(Obj *obj) {
    camSunSetProgram(6);
    tskCreate(obj, mapSeaProc, obj->file, NULL);
}

/* A map area's objects: the sky's program 7, and Obisumaru's second run once flag 0x30 is set
 * and 0xB4 is not. */
void mapSetupObisu2(Obj *obj) {
    camSunSetProgram(7);
    camSunSetFogNear(0x398);
    if (swWorkTest(0x30) && !swWorkTest(0xB4)) {
        floadLoadSet(0x88);
        tskCreate(obj, mapObisu2Proc, obj->file, NULL);
    }
}
