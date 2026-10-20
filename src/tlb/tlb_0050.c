#include "tlb.h"

extern u8 waterStreamDlTex[];
extern u8 waterFallDlTex[];
extern u8 waterSuienArg0Anim[];
extern u8 waterSuienArg1Anim[];
extern u8 waterSuienArg2Anim[];
extern u8 waterSuienArg3Anim[];

/* The "suien" task's work (task + 0x20). */
typedef struct SuienWork {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ Anim anim;
    /* 0x14 */ s32 alpha;
    /* 0x18 */ Obj *parent; /* followed: asked for its position (message 4) */
    /* 0x1C */ f32 ofs[3];  /* from the parent */
} SuienWork;

#define SUIEN(task) ((SuienWork *) ((u32) (task) + sizeof(Obj)))

extern u8 waterPoolData1Dl[];
extern u8 waterPoolData2Dl[];
extern u8 waterPoolData1Ride[];
extern u8 waterPoolData2Ride[];

/* A "water" surface's work (task + 0x20): an enemy's (EneObj) up to 0x34. */
typedef struct WaterObj {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ Frame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *frame;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ s32 scroll; /* the texture's t, down one a frame (its low half is used) */
    /* 0x40 */ s32 suien;  /* the second kind's: its "suien" kind (0-2) */
    /* 0x44 */ s32 ofsX;   /* where it makes them, turned with it */
    /* 0x48 */ s32 ofsY;
    /* 0x4C */ s32 ofsZ;
    /* 0x50 */ u16 count;  /* steps: one "suien" every 8 */
} WaterObj;

extern u8 waterSplashData3Dl[];
extern u8 waterSplashData4Dl[];
extern u8 waterSplashData5Dl[];
extern u8 waterStreamData2Dl[];
extern u8 waterStreamData4Dl[];
extern u8 waterStreamData5Dl[];
extern u8 waterStreamData7Dl[];
extern u8 waterStreamData2Ride[];
extern u8 waterStreamData4Ride[];
extern u8 waterStreamData5Ride[];

extern u8 waterTokageDl[];
extern u8 waterFallsDl[];
extern u8 waterFallsVtx[];

/* The "tokage" task's work (task + 0x20). */
typedef struct TokageWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ s16 count; /* steps */
    /* 0x06 */ u8 pad06[6];
    /* 0x0C */ s16 alpha; /* from 0x80, down 2 a step: gone at 0 */
} TokageWork;

#define TOKAGE(task) ((TokageWork *) ((u32) (task) + sizeof(Obj)))

/* A "falls" waterfall's work (task + 0x20): an enemy's (EneObj) up to 0x34. */
typedef struct FallsObj {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ Frame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *frame;
    /* 0x38 */ u8 pad38[4];
    /* 0x3C */ u16 scroll; /* the texture's t, down eight a frame */
    /* 0x3E */ u16 count;  /* steps while it lands somewhere: a "suien" every 8 */
    /* 0x40 */ s16 height; /* the fall's, last step */
    /* 0x44 */ f32 foot[3]; /* where it lands */
} FallsObj;

Gfx *waterStreamDL(Gfx *gfx, u16 scroll);
Gfx *waterRippleDL(Gfx *gfx, u16 scroll);
Gfx *waterFallDL(Gfx *gfx, u16 scroll);
Obj *waterSuienMake(Obj *parent, s32 kind, f32 x, f32 y, f32 z, f32 ry, f32 s);

s32 waterSuienProc(Obj *obj, s16 msg, void *arg);

/* A 32x32 RGBA16 texture loaded into TMEM at tmem for tile rtile, as gDPLoadMultiBlock, with its
 * tile size at (uls, ult): the game's code is one macro (one source line: IDO schedules by line;
 * written as separate statements, two instructions move). */
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

Gfx *waterStreamDL(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 128, 128, 128, 112);
    gDPLoadScrollBlock(gfx++, waterStreamDlTex, 0, G_TX_RENDERTILE, 0, scroll);
    gDPLoadScrollBlock(gfx++, waterStreamDlTex, 0x100, 1, 10, scroll * 2);
    gSPEndDisplayList(gfx++);
    return gfx;
}

Gfx *waterRippleDL(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 128, 128, 128, 96);
    gDPLoadScrollBlock(gfx++, waterStreamDlTex, 0, G_TX_RENDERTILE, scroll, scroll);
    gDPLoadScrollBlock(gfx++, waterStreamDlTex, 0x100, 1, scroll + 10, -scroll);
    gSPEndDisplayList(gfx++);
    return gfx;
}

Gfx *waterFallDL(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 128, 128, 128, 160);
    gDPLoadScrollBlock(gfx++, waterFallDlTex, 0, G_TX_RENDERTILE, 0, scroll);
    gDPLoadScrollBlock(gfx++, waterFallDlTex, 0x100, 1, 10, scroll * 2);
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* "suien": a sparkle ("anime" frame) that follows its parent, growing and fading. */
s32 waterSuienProc(Obj *obj, s16 msg, void *arg) {
    SuienWork *w = (SuienWork *) ((u32) obj + sizeof(Obj));
    u32 file;
    f32 pos[3];

    switch (msg) {
        case 2:
            obj->name = "suien";
            w->frame = (AnimeFrame *) frmCreate(gFrmLayer8, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            switch ((s32) arg) {
                case 0:
                    w->frame->anim = waterSuienArg0Anim;
                    floadGetSetRam((void **) w->frame->segs, 0x123);
                    break;
                case 1:
                    w->frame->anim = waterSuienArg1Anim;
                    floadGetSetRam((void **) w->frame->segs, 0x54);
                    break;
                case 2:
                    w->frame->anim = waterSuienArg2Anim;
                    floadGetSetRam((void **) w->frame->segs, 0x54);
                    break;
                case 3:
                    w->frame->anim = waterSuienArg3Anim;
                    floadGetSetRam((void **) w->frame->segs, 0x54);
                    break;
            }
            file = floadGetSetFile(0x123, 0);
            fadeAnimStart(&w->anim, gItexSparkleAnim, 0, &w->frame->segs[3], file);
            w->alpha = 255;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombine(gGfxDlHead++, 0xFFD3FF, 0xFFA6FF7F);
            gDPSetEnvColor(gGfxDlHead++, 255, 255, 255, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            if (fadeAnimStep(&w->anim)) {
                return -1;
            }
            pos[2] = 0.0f;
            pos[1] = 0.0f;
            pos[0] = 0.0f;
            tskSendMsg(w->parent, 4, pos);
            w->frame->pos[0] = pos[0] + w->ofs[0];
            w->frame->pos[1] = pos[1] + w->ofs[1];
            w->frame->pos[2] = pos[2] + w->ofs[2];
            w->frame->scale[0] *= 1.001;
            w->frame->scale[1] *= 1.01;
            w->frame->scale[2] *= 1.001;
            w->alpha -= 8;
            if (w->alpha < 0) {
                w->alpha = 0;
            } else if (w->alpha > 255) {
                w->alpha = 255;
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Makes a "suien" of the given kind (0-3) under the effect task at (x, y + 5, z), turned ry,
 * scale s, following parent. */
Obj *waterSuienMake(Obj *parent, s32 kind, f32 x, f32 y, f32 z, f32 ry, f32 s) {
    Obj *t;
    SuienWork *w;
    s32 pad;
    f32 pos[3];

    t = tskCreate(gEffectTask, waterSuienProc, parent->file, (void *) kind);
    if (t != NULL) {
        w = SUIEN(t);
        w->frame->pos[0] = x;
        w->frame->pos[1] = y + 5.0f;
        w->frame->pos[2] = z;
        w->frame->rot[1] = ry;
        w->frame->scale[2] = s;
        w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
        w->parent = parent;
        pos[0] = 0.0f;
        pos[1] = 0.0f;
        pos[2] = 0.0f;
        tskSendMsg(w->parent, 4, pos);
        w->ofs[0] = w->frame->pos[0] - pos[0];
        w->ofs[1] = w->frame->pos[1] - pos[1];
        w->ofs[2] = w->frame->pos[2] - pos[2];
    }
    return t;
}

/* "water": a water surface (a "pol" frame you can stand on, a ride), its texture scrolling.
 * eneSetup's data: the low byte the model (1 or 2), the high half its turn (1024 a circle). */
s32 waterPoolProc(Obj *obj, s16 msg, void *arg) {
    WaterObj *w = (WaterObj *) ((u32) obj + sizeof(Obj));
    s32 pad[2];
    s32 data;

    switch (msg) {
        case 2:
            obj->name = "water";
            data = eneSetup(obj, arg);
            switch ((u8) data) {
                case 1:
                    w->frame = eneNewPol(obj, (Gfx *) waterPoolData1Dl);
                    w->ride = rideCreate(gRideTree2, (s32) obj, &w->frame->f,
                                         (RideData *) eneFileAddr(obj, waterPoolData1Ride, 0));
                    break;
                case 2:
                    w->frame = eneNewPol(obj, (Gfx *) waterPoolData2Dl);
                    w->ride = rideCreate(gRideTree2, (s32) obj, &w->frame->f,
                                         (RideData *) eneFileAddr(obj, waterPoolData2Ride, 0));
                    break;
            }
            if (w->frame == NULL) {
                return -1;
            }
            frmMove(&w->frame->f, gFrmLayers[6]);
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->rot[1] = (data >> 16) * PI / 512;
            break;
        case 1:
            if (w->ride != NULL) {
                frmDestroy(&w->ride->f);
            }
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gGfxDlHead = waterRippleDL(gGfxDlHead, w->scroll);
            w->scroll--;
            eneSetModelOnCourse(obj, &w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "water", models 3-5: a surface whose second texture scrolls four a frame, making a "suien"
 * every 8 steps at its offset (models 3 and 4); message 4 gives its place 200 lower. */
s32 waterSplashProc(Obj *obj, s16 msg, void *arg) {
    WaterObj *w = (WaterObj *) ((u32) obj + sizeof(Obj));
    s32 pad[2];
    s32 data;
    f32 s;
    f32 c;
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "water";
            data = eneSetup(obj, arg);
            switch ((u8) data) {
                case 3:
                    w->frame = eneNewPol(obj, (Gfx *) waterSplashData3Dl);
                    w->suien = 1;
                    w->ofsY = -200;
                    w->ofsZ = 80;
                    break;
                case 4:
                    w->frame = eneNewPol(obj, (Gfx *) waterSplashData4Dl);
                    w->suien = 2;
                    w->ofsY = -166;
                    w->ofsZ = 75;
                    break;
                case 5:
                    w->frame = eneNewPol(obj, (Gfx *) waterSplashData5Dl);
                    w->suien = 0;
                    w->ofsY = -23;
                    w->ofsZ = 95;
                    break;
            }
            if (w->frame == NULL) {
                return -1;
            }
            frmMove(&w->frame->f, gFrmLayers[6]);
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->rot[1] = (data >> 16) * PI / 512;
            break;
        case 0:
            w->count++;
            if (w->suien != 0 && (w->count & 7) == 1) {
                s = mathSinCos((s32) (w->frame->rot[1] / PI * 512), &c);
                waterSuienMake(obj, w->suien, w->frame->pos[0] + w->ofsX * c + w->ofsZ * s,
                                       w->frame->pos[1] + w->ofsY, w->frame->pos[2] + w->ofsX * s + w->ofsZ * c,
                                       w->frame->rot[1], 0.1f);
            }
            break;
        case 1:
            if (w->ride != NULL) {
                frmDestroy(&w->ride->f);
            }
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 4:
            if (arg != NULL) {
                ((f32 *) arg)[0] = w->frame->pos[0];
                ((f32 *) arg)[1] = w->frame->pos[1] - 200.0f;
                ((f32 *) arg)[2] = w->frame->pos[2];
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gGfxDlHead = waterFallDL(gGfxDlHead, w->scroll);
            w->scroll -= 4;
            eneSetModelOnCourse(obj, &w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "water", models 2, 4, 5 and 7: as the first kind (rides but model 7's), the second texture
 * scrolling four a frame. */
s32 waterStreamProc(Obj *obj, s16 msg, void *arg) {
    WaterObj *w = (WaterObj *) ((u32) obj + sizeof(Obj));
    s32 pad[2];
    s32 data;

    switch (msg) {
        case 2:
            obj->name = "water";
            data = eneSetup(obj, arg);
            switch ((u8) data) {
                case 2:
                    w->frame = eneNewPol(obj, (Gfx *) waterStreamData2Dl);
                    w->ride = rideCreate(gRideTree2, (s32) obj, &w->frame->f,
                                         (RideData *) eneFileAddr(obj, waterStreamData2Ride, 0));
                    break;
                case 4:
                    w->frame = eneNewPol(obj, (Gfx *) waterStreamData4Dl);
                    w->ride = rideCreate(gRideTree2, (s32) obj, &w->frame->f,
                                         (RideData *) eneFileAddr(obj, waterStreamData4Ride, 0));
                    break;
                case 5:
                    w->frame = eneNewPol(obj, (Gfx *) waterStreamData5Dl);
                    w->ride = rideCreate(gRideTree2, (s32) obj, &w->frame->f,
                                         (RideData *) eneFileAddr(obj, waterStreamData5Ride, 0));
                    break;
                case 7:
                    w->frame = eneNewPol(obj, (Gfx *) waterStreamData7Dl);
                    break;
            }
            if (w->frame == NULL) {
                return -1;
            }
            frmMove(&w->frame->f, gFrmLayers[6]);
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->rot[1] = (data >> 16) * PI / 512;
            break;
        case 1:
            if (w->ride != NULL) {
                frmDestroy(&w->ride->f);
            }
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gGfxDlHead = waterStreamDL(gGfxDlHead, w->scroll);
            w->scroll -= 4;
            eneSetModelOnCourse(obj, &w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "tokage": a splash (a "pol" frame) that rises, stretches and fades. */
s32 waterTokageProc(Obj *obj, s16 msg, void *arg) {
    TokageWork *w = (TokageWork *) ((u32) obj + sizeof(Obj));

    switch (msg) {
        case 2:
            obj->name = "tokage";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = (Gfx *) waterTokageDl;
            floadGetSetRam((void **) w->frame->segs, 0x53);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->frame->pos[1] -= 2.0f;
            w->frame->scale[1] *= 1.015;
            w->frame->scale[0] *= 1.002;
            w->frame->scale[2] *= 1.002;
            w->alpha -= 2;
            if (w->alpha <= 0) {
                return -1;
            }
            w->count++;
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPSetEnvColor(gGfxDlHead++, 16, 16, 16, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Makes a "tokage" under parent at (x, y + 10, z), turned ry, as tall as the fall h is. */
void waterTokageMake(Obj *parent, f32 x, f32 y, f32 z, f32 ry, s16 h) {
    Obj *t = tskCreate(parent, waterTokageProc, parent->file, NULL);

    if (t != NULL) {
        TOKAGE(t)->alpha = 0x80;
        TOKAGE(t)->frame->rot[1] = ry;
        TOKAGE(t)->frame->scale[0] = 0.1f;
        TOKAGE(t)->frame->scale[1] = fabsf(h) * 0.14 / 575;
        TOKAGE(t)->frame->scale[2] = 0.1f;
        TOKAGE(t)->frame->pos[0] = x;
        TOKAGE(t)->frame->pos[1] = y + 10.0f;
        TOKAGE(t)->frame->pos[2] = z;
    }
}

/* "falls": a waterfall, its texture scrolling eight a frame. */
s32 waterFallsProc(Obj *obj, s16 msg, void *arg) {
    FallsObj *w = (FallsObj *) ((u32) obj + sizeof(Obj));
    s32 pad;
    s32 data;

    switch (msg) {
        case 2:
            obj->name = "falls";
            data = eneSetup(obj, arg);
            w->frame = eneNewPol(obj, (Gfx *) waterFallsDl);
            if (w->frame == NULL) {
                return -1;
            }
            frmMove(&w->frame->f, gFrmLayers[6]);
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->rot[1] = (f32) data / 512 * PI;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gGfxDlHead = waterFallDL(gGfxDlHead, w->scroll);
            w->scroll -= 8;
            w->frame->segs[5] = eneFileAddr(obj, waterFallsVtx, 0);
            eneSetModelOnCourse(obj, &w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "fallsR": a waterfall that finds where it lands each frame (a ride below it, 200 down at
 * most): its model's foot vertices are stretched down to there, a "suien" splashes there every 8
 * frames, and a "tokage" rises when the fall grows by more than 100. */
s32 waterFallsRideProc(Obj *obj, s16 msg, void *arg) {
    FallsObj *w = (FallsObj *) ((u32) obj + sizeof(Obj));
    s32 pad[2];
    s32 data;
    Vtx *vtx;
    f32 d[3];
    s16 h;
    s32 pad2;
    f32 s;
    f32 c;

    switch (msg) {
        case 2:
            obj->name = "fallsR";
            data = eneSetup(obj, arg);
            w->frame = eneNewPol(obj, (Gfx *) waterFallsDl);
            if (w->frame == NULL) {
                return -1;
            }
            frmMove(&w->frame->f, gFrmLayers[6]);
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->rot[1] = (f32) data / 512 * PI;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gGfxDlHead = waterFallDL(gGfxDlHead, w->scroll);
            w->scroll -= 8;
            vtx = (Vtx *) gGfxDlHead;
            gGfxDlHead += 32;
            w->frame->segs[5] = (u32) vtx;
            heapMemCopy((u8 *) vtx, (u8 *) eneFileAddr(obj, waterFallsVtx, 0), 0x100);
            d[0] = 0.0f;
            d[2] = 0.0f;
            d[1] = -200.0f;
            if (rideMoveStop((Vec3f *) w->frame->pos, (Vec3f *) d, (Vec3f *) d, (s32) obj)) {
                w->foot[0] = d[0] + w->frame->pos[0];
                w->foot[1] = d[1] + w->frame->pos[1];
                w->foot[2] = d[2] + w->frame->pos[2];
                if ((w->count & 7) == 1) {
                    s = mathSinCos((s32) (w->frame->rot[1] / PI * 512), &c);
                    waterSuienMake(obj, 0, w->foot[0] + fadeRand(5) - 2.0f - 5.0f * s, w->foot[1] - 5.0f,
                                           w->foot[2] + fadeRand(5) - 2.0f - 5.0f * c, w->frame->rot[1], 0.3f);
                }
                w->count++;
            }
            h = (d[1] - 3.0f) / w->frame->scale[1];
            vtx[4].v.ob[1] = h;
            vtx[5].v.ob[1] = h;
            vtx[14].v.ob[1] = h;
            vtx[15].v.ob[1] = h;
            vtx[4].v.tc[1] = (h + 2000) * 373 / 1971 * 32;
            vtx[5].v.tc[1] = (h + 2000) * 373 / 1971 * 32;
            vtx[14].v.tc[1] = (h + 2000) * 373 / 1971 * 32;
            vtx[15].v.tc[1] = (h + 2000) * 373 / 1971 * 32;
            if (w->height + 100 < h) {
                waterTokageMake(obj, d[0] + w->frame->pos[0], d[1] + w->frame->pos[1],
                                       d[2] + w->frame->pos[2], w->frame->rot[1], h - w->height);
            }
            w->height = h;
            eneSetModelOnCourse(obj, &w->frame->f);
            break;
        case 4:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "fallsV": as "fallsR", the fall found on the ground (50 down at most, as far as the ground's
 * hit says), a "suien" every 8 frames at the hit. */
s32 waterFallsGroundProc(Obj *obj, s16 msg, void *arg) {
    FallsObj *w = (FallsObj *) ((u32) obj + sizeof(Obj));
    s32 pad[2];
    s32 data;
    Vtx *vtx;
    f32 d[3];
    f32 at[3];
    f32 t;
    s16 h;
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "fallsV";
            data = eneSetup(obj, arg);
            w->frame = eneNewPol(obj, (Gfx *) waterFallsDl);
            if (w->frame == NULL) {
                return -1;
            }
            frmMove(&w->frame->f, gFrmLayers[6]);
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->rot[1] = (f32) data / 512 * PI;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gGfxDlHead = waterFallDL(gGfxDlHead, w->scroll);
            w->scroll -= 8;
            vtx = (Vtx *) gGfxDlHead;
            gGfxDlHead += 32;
            w->frame->segs[5] = (u32) vtx;
            heapMemCopy((u8 *) vtx, (u8 *) eneFileAddr(obj, waterFallsVtx, 0), 0x100);
            d[0] = 0.0f;
            d[2] = 0.0f;
            d[1] = -50.0f;
            if (vramRayGround((Vec3f *) w->frame->pos, (Vec3f *) d, (Vec3f *) at, &t)) {
                if ((w->count & 7) == 1) {
                    waterSuienMake(obj, 0, at[0] + fadeRand(5) - 2.0f, at[1],
                                           at[2] + fadeRand(5) - 2.0f, w->frame->rot[1], 0.25f);
                }
                w->count++;
                d[1] *= t;
            }
            h = (d[1] - 3.0f) / w->frame->scale[1];
            vtx[4].v.ob[1] = h;
            vtx[5].v.ob[1] = h;
            vtx[14].v.ob[1] = h;
            vtx[15].v.ob[1] = h;
            vtx[4].v.tc[1] = (h + 2000) * 373 / 1971 * 32;
            vtx[5].v.tc[1] = (h + 2000) * 373 / 1971 * 32;
            vtx[14].v.tc[1] = (h + 2000) * 373 / 1971 * 32;
            vtx[15].v.tc[1] = (h + 2000) * 373 / 1971 * 32;
            eneSetModelOnCourse(obj, &w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
