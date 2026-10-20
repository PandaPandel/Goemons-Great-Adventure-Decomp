#include "tlb.h"

/* A gear's work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct GearWork {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *frame;
    /* 0x38 */ ACollFrame *atk[6];
} GearWork;

/* A ridden gear's work (gear2). */
typedef struct RideGearWork {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *frame;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ f32 dir; /* 1 or -1 */
} RideGearWork;

/* gear3's work: its teeth's boxes are children of root. */
typedef struct BoxGearWork {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *frame;
    /* 0x38 */ ACollFrame *root;
} BoxGearWork;

/* The frog statue's work (kero): an enemy's header up to 0x34. */
typedef struct KeroWork {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ MAniFrame *mani;
    /* 0x38 */ MPolFrame *mpol;
    /* 0x3C */ RideFrame *ride;
} KeroWork;

/* The lion-dog statue's work (sisi). */
typedef struct SisiWork {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ MPolFrame *mpol;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ s32 timer;
    /* 0x40 */ u8 state; /* 0, 1, 2: the three rocking steps */
} SisiWork;

/* Message 6's argument (plmove.c's): a rider's place on the course, moved by the object. */
typedef struct PlPlace {
    /* 0x0 */ s16 zone;
    /* 0x4 */ f32 place;
    /* 0x8 */ f32 side;
    /* 0xC */ f32 height;
} PlPlace;

/* The lotus leaf's work (hasu): an enemy's header up to 0x34. */
typedef struct HasuWork {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rotX;
    /* 0x20 */ f32 rotY;
    /* 0x24 */ f32 rotZ;
    /* 0x28 */ f32 vx;
    /* 0x2C */ f32 vy;
    /* 0x30 */ f32 vz;
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ PolFrame *shadow;
    /* 0x3C */ RideFrame *ride;
    /* 0x40 */ f32 baseY; /* its resting height */
    /* 0x44 */ f32 push; /* the riders' fall speeds this frame */
} HasuWork;

/* The lift's work (elevator22): an enemy's header up to 0x34. */
typedef struct ElevWork {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rotX;
    /* 0x20 */ f32 rotY;
    /* 0x24 */ f32 rotZ;
    /* 0x28 */ f32 vx;
    /* 0x2C */ f32 vy;
    /* 0x30 */ f32 vz;
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ u8 state; /* 0 low and empty, 1 low, 2 rising, 3 high and empty, 4 high, 5 falling */
    /* 0x3D */ u8 riders; /* this frame's (message 6) */
    /* 0x40 */ f32 baseY;
    /* 0x44 */ f32 offset; /* above baseY */
    /* 0x48 */ f32 speed;
    /* 0x4C */ f32 height; /* its travel */
} ElevWork;

/* The flowing water's work (water): an enemy's header up to 0x34. */
typedef struct WaterWork {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rotX;
    /* 0x20 */ f32 rotY;
    /* 0x24 */ f32 rotZ;
    /* 0x28 */ f32 vx;
    /* 0x2C */ f32 vy;
    /* 0x30 */ f32 vz;
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ s32 scroll; /* its texture's place */
    /* 0x40 */ s32 step; /* added to scroll each frame */
    /* 0x44 */ Gfx *(*draw)(Gfx *gfx, u16 scroll); /* its texture list */
    /* 0x48 */ u16 sound; /* a looping sound (0: none) */
} WaterWork;

/* This file's sparkle's work (itex). */
typedef struct SparkleWork {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ Anim anim;
    /* 0x14 */ s32 alpha;
    /* 0x18 */ Obj *target; /* asked for its place (message 4) each frame */
    /* 0x1C */ f32 offset[3]; /* from that place */
} SparkleWork;

/* The waterfall's work (falls22, falls22V, falls22R): an enemy's header up to 0x34. */
typedef struct FallsWork {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rotX;
    /* 0x20 */ f32 rotY;
    /* 0x24 */ f32 rotZ;
    /* 0x28 */ f32 vx;
    /* 0x2C */ f32 vy;
    /* 0x30 */ f32 vz;
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ u8 pad38[4];
    /* 0x3C */ u16 scroll; /* its texture's place */
    /* 0x3E */ u16 count; /* frames, for the sparkles */
    /* 0x40 */ u8 pad40[4];
    /* 0x44 */ Vec3f foot; /* where it lands (message 4 copies it out) */
} FallsWork;

/* plmain.c defines plCountPlayers void, but it leaves tskCountChildren's count in v0 and the
 * game uses it. */
typedef s32 (*CountFunc)(void);

/* The hit ring's work and argument (this file's Hitring). */
typedef struct RingWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[0x10];
    /* 0x14 */ s32 alpha;
} RingWork;

typedef struct RingArg {
    /* 0x00 */ u8 pad00[4];
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ f32 scale;
    /* 0x14 */ u8 pad14[4];
} RingArg; /* size 0x18: its spawner's frame holds 0x18 bytes */

/* model.c defines mdlAnimNodeMtx's frame as an s32, but the TLB code passes a float. */
typedef void (*AnimNodeMtxFunc)(u32 addr, f32 frame, u8 **base, f32 out[4][4]);

/* acoll.c defines these two void, but the game uses their results. */
typedef ACollFrame *(*AtkAddFunc)(Frame *parent, Frame *model, u16 radius, s16 ox, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*AtkAddYZFunc)(Frame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);

extern Frame *gFrmLayer3;
extern Gfx obj22Gear1Dl[];
extern Gfx obj22Gear2Dl[];
extern u8 obj22Gear2Ride[];
extern Gfx obj22Gear3Dl[];
extern f32 obj22Gear3Teeth[8][3]; /* gear3's teeth */
extern u8 obj22KeroManiAnim[];
extern u8 obj22KeroManiAttach[];
extern u8 obj22KeroRide[];
extern Gfx obj22SisiDl[];
extern u8 obj22SisiRide[];
extern Gfx obj22HitringDl[];
extern Gfx obj22HasuDl[];
extern Gfx obj22HasuShadowDl[];
extern u8 obj22HasuRide[];
extern Gfx obj22HasuShadowDL[]; /* the lotus leaf's shadow */
extern Gfx obj22Elevator22Dl[];
extern u8 obj22Elevator22Ride[];
extern Gfx obj22WaterR1Dl[];
extern Gfx obj22WaterR2Dl[];
extern Gfx obj22WaterR3Dl[];
extern Gfx obj22WaterR4Dl[];
extern Gfx obj22WaterR5Dl[];
extern Gfx obj22WaterR6Dl[];
extern u8 obj22WaterR1Ride[];
extern u8 obj22WaterR2Ride[];
extern u8 obj22WaterR3Ride[];
extern u8 obj22WaterR4Ride[];
extern u8 obj22WaterR5Ride[];
extern u8 obj22ItexAnim[];
extern u8 obj22WaterfallVtx[]; /* the waterfall's vertices */
extern Gfx obj22Falls22Dl[];
extern u8 obj22FallsFootVtx[16]; /* the waterfall's lower vertices */

/* The file's .data, in address order (its externs above stay as declarations). */
Gfx obj22HasuShadowDL[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_CULL_BACK),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT),
    gsDPSetEnvColor(0x00, 0x00, 0x00, 0x00),
    gsDPSetRenderMode(Z_CMP | Z_UPD | IM_RD | CVG_DST_FULL | ZMODE_XLU | FORCE_BL | GBL_c1(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA), Z_CMP | Z_UPD | IM_RD | CVG_DST_FULL | ZMODE_XLU | FORCE_BL | GBL_c2(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)),
    gsSPEndDisplayList(),
}; /* the lotus leaf's shadow */
f32 obj22Gear3Teeth[8][3] = { 244.0f, 298.0f, 46.0f, 244.0f, -294.0f, -46.0f, 244.0f, 46.0f, -298.0f, 244.0f, -46.0f, 298.0f, -244.0f, 298.0f, 46.0f, -244.0f, -294.0f, -46.0f, -244.0f, 46.0f, -298.0f, -244.0f, -46.0f, 298.0f }; /* gear3's teeth */
u8 obj22FallsFootVtx[16] = { 0x00, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0C, 0x0D, 0x10, 0x12, 0x13, 0x16, 0x17, 0x1A }; /* the waterfall's lower vertices */
extern u8 obj22ScrollTexATex[];
extern u8 obj22ScrollTexFallsTex[];

ACollFrame *obj22GearAtkAdd(Frame *model, f32 x, f32 y, f32 z);
s32 obj22GearInView(GearWork *w);
Vtx *obj22FallsFoot(Obj *obj, Vec3f *at, f32 len, f32 scale);

/* A 32x32 RGBA16 texture loaded at tmem and drawn through tile rtile from (uls, ult): the game
 * wrote each load as one macro (its seven commands schedule as one source line). */
#define gDPLoadScrollBlock(pkt, timg, tmem, rtile, uls, ult)                                               \
    {                                                                                                     \
        gDPSetTextureImage(pkt, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, timg);                                    \
        gDPSetTile(pkt, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, tmem, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, \
                   5, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD);                              \
        gDPLoadSync(pkt);                                                                                 \
        gDPLoadBlock(pkt, G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES));            \
        gDPPipeSync(pkt);                                                                                 \
        gDPSetTile(pkt, G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, tmem, rtile, 0, G_TX_WRAP | G_TX_NOMIRROR, 5,      \
                   G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD);                                 \
        gDPSetTileSize(pkt, rtile, uls, ult, 31 << G_TEXTURE_IMAGE_FRAC, 31 << G_TEXTURE_IMAGE_FRAC);     \
    }

/* The game homes scroll and never masks it to 16 bits: a u16 parameter. */
Gfx *obj22ScrollTexA(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 0x80, 0x80, 0x80, 0x70);
    gDPLoadScrollBlock(gfx++, obj22ScrollTexATex, 0, G_TX_RENDERTILE, 0, scroll);
    gDPLoadScrollBlock(gfx++, obj22ScrollTexATex, 0x100, 1, 10, scroll * 2);
    gSPEndDisplayList(gfx++);
    return gfx;
}

Gfx *obj22ScrollTexB(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 0x80, 0x80, 0x80, 0x60);
    gDPLoadScrollBlock(gfx++, obj22ScrollTexATex, 0, G_TX_RENDERTILE, scroll, scroll);
    gDPLoadScrollBlock(gfx++, obj22ScrollTexATex, 0x100, 1, scroll + 10, -scroll);
    gSPEndDisplayList(gfx++);
    return gfx;
}

Gfx *obj22ScrollTexFalls(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 0x80, 0x80, 0x80, 0xA0);
    gDPLoadScrollBlock(gfx++, obj22ScrollTexFallsTex, 0, G_TX_RENDERTILE, 0, scroll);
    gDPLoadScrollBlock(gfx++, obj22ScrollTexFallsTex, 0x100, 1, 10, scroll * 2);
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* Whether the gear is within 800 of the camera and inside 0.8 of its field of view (x and z
 * only). The locals' order is the game's frame. */
s32 obj22GearInView(GearWork *w) {
    PolFrame *f;
    f32 c;
    f32 dx;
    f32 dz;
    f32 len;
    f32 ax;
    f32 az;
    f32 alen;
    CamView *view;

    view = gGameWork.view;
    c = mathCos(fabsf(view->fovy) * 0.8f);
    f = w->frame;
    dx = f->pos[0] - view->eye[0];
    dz = f->pos[2] - view->eye[2];
    len = sqrtf(dx * dx + dz * dz);
    if (800.0f < len) {
        return 0;
    }
    ax = view->at[0] - view->eye[0];
    az = view->at[2] - view->eye[2];
    alen = sqrtf((ax * ax + az * az) * (dx * dx + dz * dz));
    if ((ax * dx + az * dz) / alen < c) {
        return 0;
    }
    return 1;
}

/* An attack box (kind 0x78, radius 0x50) on the gear's model at (x, y, z). acoll.c defines
 * acollAtkAddXYZ void, but the game uses its result: called through a cast. */
ACollFrame *obj22GearAtkAdd(Frame *model, f32 x, f32 y, f32 z) {
    ACollFrame *a = ((AtkAddFunc) acollAtkAddXYZ)((Frame *) ene_atk_root_frame, model, 0x50, x, y, z, 0);

    if (a == NULL) {
        return NULL;
    }
    a->kind = 0x78;
    return a;
}

/* "gear1": a turning gear (one turn every 256 frames) with six attack boxes on its teeth; its
 * sound 0x249 plays while it is in view. A hit box's victim gets message 7.
 * Case 0's loop is written in edopole's form (tlb_0048's gimEdopoleProc: "continue" on an empty
 * box, then "== 1 && (Obj *) hitPower != NULL"); a pad keeps the game's 0x48 frame. */
s32 obj22Gear1Proc(Obj *obj, s16 msg, void *arg) {
    GearWork *w = (GearWork *) ((u32) obj + sizeof(Obj));
    s32 i;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "gear1";
            eneSetup(obj, arg);
            if ((w->frame = eneNewPol(obj, obj22Gear1Dl)) == NULL) {
                return -1;
            }
            frmMove(&w->frame->f, gFrmLayer3);
            w->atk[0] = obj22GearAtkAdd(&w->frame->f, -72.0f, 1100.0f, 0.0f);
            w->atk[1] = obj22GearAtkAdd(&w->frame->f, 920.0f, 620.0f, 0.0f);
            w->atk[2] = obj22GearAtkAdd(&w->frame->f, -1000.0f, 487.0f, 0.0f);
            w->atk[3] = obj22GearAtkAdd(&w->frame->f, 996.0f, -487.0f, 0.0f);
            w->atk[4] = obj22GearAtkAdd(&w->frame->f, -915.0f, -624.0f, 0.0f);
            w->atk[5] = obj22GearAtkAdd(&w->frame->f, 75.0f, -1100.0f, 0.0f);
            break;
        case 1:
            for (i = 0; i < 6; i++) {
                frmDestroy(&w->atk[i]->frame);
            }
            frmDestroy(&w->frame->f);
            sndPlay(0x8249);
            break;
        case 0:
            if (obj22GearInView(w)) {
                if (sndGetVolume(0x249, 0) == 0) {
                    sndPlay(0x249);
                }
            } else if (sndGetVolume(0x249, 0) != 0) {
                sndPlay(0x8249);
            }
            w->frame->rot[2] = (s32) ((gGameFrameCount & 0xFF) - 2) * 0.024543693f;
            for (i = 0; i < 6; i++) {
                if (w->atk[i] == NULL) {
                    continue;
                }
                if (acollAtkWasHit(w->atk[i]) == 1 && (Obj *) w->atk[i]->hitPower != NULL) {
                    tskSendMsg((Obj *) w->atk[i]->hitPower, 7, obj);
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "gear2": a gear the players ride, turning one way or the other (eneSetup's result picks
 * which), half a turn every 512 frames. */
s32 obj22Gear2Proc(Obj *obj, s16 msg, void *arg) {
    RideGearWork *w = (RideGearWork *) ((u32) obj + sizeof(Obj));
    RideData *data;
    u32 back;

    switch (msg) {
        case 2:
            obj->name = "gear2";
            back = eneSetup(obj, arg);
            if ((w->frame = eneNewPol(obj, obj22Gear2Dl)) == NULL) {
                return -1;
            }
            frmMove(&w->frame->f, gFrmLayer3);
            data = (RideData *) eneFileAddr(obj, obj22Gear2Ride, 0);
            if ((w->ride = rideCreate(gRideTree1, (s32) obj, &w->frame->f, data)) == NULL) {
                return -1;
            }
            if (back) {
                w->dir = -1.0f;
            } else {
                w->dir = 1.0f;
            }
            break;
        case 1:
            frmDestroy(&w->ride->f);
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->frame->rot[1] = w->dir * 3.1415927f / 256 * (gGameFrameCount & 0x1FF);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "gear3": a turning gear whose eight teeth (obj22Gear3Teeth) are attack boxes under one parent
 * box. */
s32 obj22Gear3Proc(Obj *obj, s16 msg, void *arg) {
    BoxGearWork *w = (BoxGearWork *) ((u32) obj + sizeof(Obj));
    ACollFrame *a;
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "gear3";
            eneSetup(obj, arg);
            if ((w->frame = eneNewPol(obj, obj22Gear3Dl)) == NULL) {
                return -1;
            }
            frmMove(&w->frame->f, gFrmLayer3);
            if ((w->root = ((AtkAddYZFunc) acollAtkAddYZ)((Frame *) ene_atk_root_frame, &w->frame->f, 1, 0, 0, 0)) ==
                NULL) {
                return -1;
            }
            for (i = 0; i < 8; i++) {
                a = ((AtkAddFunc) acollAtkAddXYZ)(&w->root->frame, &w->frame->f, 0x28, obj22Gear3Teeth[i][0],
                                                   obj22Gear3Teeth[i][1], obj22Gear3Teeth[i][2], 0);
                if (a != NULL) {
                    a->kind = 0x78;
                }
            }
            break;
        case 1:
            frmDestroy(&w->root->frame);
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->frame->rot[0] = (s32) ((gGameFrameCount & 0xFF) - 2) * 0.024543693f;
            for (a = (ACollFrame *) w->root->frame.child; a != NULL; a = (ACollFrame *) a->frame.next) {
                if (acollAtkWasHit(a) == 1 && (Obj *) a->hitPower != NULL) {
                    tskSendMsg((Obj *) a->hitPower, 7, obj);
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "kero": a frog statue the players ride: an animated "mani" frame (turned by eneSetup's
 * result, a tenth the size) with an "mpol" child, placed from its course point; its croak
 * (0x30C) at frames 3-3.2 of the animation. Its stack frame needs two unused words first. */
s32 obj22KeroProc(Obj *obj, s16 msg, void *arg) {
    s32 pad[2];
    KeroWork *w = (KeroWork *) ((u32) obj + sizeof(Obj));
    s32 rot;
    f32 x;
    f32 y;
    f32 z;

    switch (msg) {
        case 2:
            obj->name = "kero";
            rot = eneSetup(obj, arg);
            vramCoursePointXZ(w->zone, w->x, w->z, &x, &z);
            w->zone = 0;
            w->x = x;
            w->z = z;
            if ((w->mani = (MAniFrame *) frmCreate(gFrmLayer3, gFrmKindMani)) == NULL) {
                return -1;
            }
            w->mani->anim = obj22KeroManiAnim;
            eneGetSegs(obj, w->mani->segs);
            mathMtxPostRotY(w->mani->mtx.m, rot * 3.1415927f / 512);
            mathMtxPostTrans(w->mani->mtx.m, w->x, w->y, w->z);
            mathMtxScaleRot(w->mani->mtx.m, 0.1f);
            if ((w->mpol = (MPolFrame *) frmCreate(&w->mani->f, gFrmKindMpol)) == NULL) {
                return -1;
            }
            eneGetSegs(obj, w->mpol->segs);
            if ((w->ride = rideCreate(gRideTree1, (s32) obj, &w->mpol->f,
                                      (RideData *) eneFileAddr(obj, obj22KeroRide, 0))) == NULL) {
                return -1;
            }
            break;
        case 1:
            frmDestroy(&w->ride->f);
            frmDestroy(&w->mpol->f);
            frmDestroy(&w->mani->f);
            break;
        case 6:
            if (arg != NULL) {
            }
            break;
        case 0:
            w->mani->frame += 0.2f;
            if (w->mani->frame >= 3.0f && w->mani->frame < 3.2f) {
                soundIncamCam(0x30C, -125.0f, 212.0f, 824.0f);
            }
            if (w->mani->frame >= polAnimLastFrame(w->mani->anim, w->mani->segs) + 1) {
                w->mani->frame -= polAnimLastFrame(w->mani->anim, w->mani->segs) + 1;
            }
            ((AnimNodeMtxFunc) mdlAnimNodeMtx)((u32) obj22KeroManiAttach, w->mani->frame, (u8 **) w->mpol->segs,
                                               w->mpol->mtx.m);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "sisi": a lion-dog statue the players ride, rocking forward and back in three 61-frame
 * steps (a thud, 0x2AC, at the end of the last); message 6 moves a rider's place with it. */
s32 obj22SisiProc(Obj *obj, s16 msg, void *arg) {
    SisiWork *w = (SisiWork *) ((u32) obj + sizeof(Obj));
    RideData *data;
    s32 rot;
    f32 x;
    f32 z;
    f32 angle;
    Vec3f r;
    f32 k;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "sisi";
            rot = eneSetup(obj, arg);
            vramCoursePointXZ(w->zone, w->x, w->z, &x, &z);
            w->zone = 0;
            w->x = x;
            w->z = z;
            if ((w->mpol = (MPolFrame *) frmCreate(gFrmModelLayer, gFrmKindMpol)) == NULL) {
                return -1;
            }
            frmMove(&w->mpol->f, gFrmLayer3);
            w->mpol->dl2 = obj22SisiDl;
            eneGetSegs(obj, w->mpol->segs);
            mathMtxPostRotY(w->mpol->mtx.m, rot * 3.1415927f / 512);
            mathMtxPostTrans(w->mpol->mtx.m, w->x, w->y, w->z);
            mathMtxScaleRot(w->mpol->mtx.m, 0.1f);
            mathMtxPreRotX(w->mpol->mtx.m, 0.3926991f);
            data = (RideData *) eneFileAddr(obj, obj22SisiRide, 0);
            if ((w->ride = rideCreate(gRideTree1, (s32) obj, &w->mpol->f, data)) == NULL) {
                return -1;
            }
            break;
        case 1:
            frmDestroy(&w->ride->f);
            frmDestroy(&w->mpol->f);
            break;
        case 6:
            if (arg != NULL) {
                vramCoursePointXZ(((PlPlace *) arg)->zone, ((PlPlace *) arg)->place, 0.0f, &r.x, &r.z);
                r.x -= w->x;
                r.z -= w->z;
                r.y = ((PlPlace *) arg)->height - w->y;
                angle = vramCourseHeading(((PlPlace *) arg)->zone, ((PlPlace *) arg)->place);
                mathVecRotateY(-(angle - 1.570796371f), &r.x, &r.y, &r.z);
                switch (w->state) {
                    case 0:
                        k = 0.001636246219f;
                        break;
                    case 1:
                        k = 0.006544984877f;
                        break;
                    case 2:
                        k = -0.01308996975f;
                        break;
                }
                ((PlPlace *) arg)->place -= r.y * k;
                ((PlPlace *) arg)->height += r.x * k;
            }
            break;
        case 0:
            w->timer++;
            switch (w->state) {
                case 0:
                    if (w->timer > 60) {
                        w->state = 1;
                        w->timer = 0;
                    } else {
                        mathMtxPreRotX(w->mpol->mtx.m, -0.00327249244f);
                    }
                    break;
                case 1:
                    if (w->timer > 60) {
                        w->state = 2;
                        w->timer = 0;
                    } else {
                        mathMtxPreRotX(w->mpol->mtx.m, -0.00981747728f);
                    }
                    break;
                case 2:
                    if (w->timer > 60) {
                        soundLocationCam(0x2AC, w->x, w->y, w->z);
                        w->state = 0;
                        w->timer = 0;
                    } else {
                        mathMtxPreRotX(w->mpol->mtx.m, 0.0130899698f);
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Hitring": this file's own hit ring, laid flat at arg's place and scale; grows by 2% a frame
 * while it fades by 10 (from 0xA0). */
s32 obj22HitringProc(Obj *obj, s16 msg, void *arg) {
    RingWork *w = (RingWork *) ((u32) obj + sizeof(Obj));
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Hitring";
            if ((w->frame = (PolFrame *) frmCreate(((Frame **) &gFrmLayer4)[1], gFrmKindPol)) == NULL) {
                return -1;
            }
            w->frame->dl2 = obj22HitringDl;
            floadGetSetRam((void **) w->frame->segs, 0x120);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = ((RingArg *) arg)->scale;
            w->frame->pos[0] = ((RingArg *) arg)->pos[0];
            w->frame->pos[1] = ((RingArg *) arg)->pos[1];
            w->frame->pos[2] = ((RingArg *) arg)->pos[2];
            w->frame->rot[0] = -1.570796371f;
            w->alpha = 0xA0;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->alpha -= 10;
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            w->frame->scale[0] *= 1.02;
            w->frame->scale[1] *= 1.02;
            w->frame->scale[2] *= 1.02;
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombineLERP(gGfxDlHead++, 0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, ENVIRONMENT,
                              TEXEL0, 0, ENVIRONMENT, 0);
            gDPSetEnvColor(gGfxDlHead++, 255, 255, 255, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A hit ring at a point, twelve times `scale`, under obj. */
void obj22HitringCreate(Obj *obj, f32 x, f32 y, f32 z, f32 scale) {
    RingArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = scale * 12.0f;
    tskCreate(obj, obj22HitringProc, obj->file, &arg);
}

/* "hasu": a lotus leaf floating on the water (eneSetup's result is its size, in thousandths),
 * with its shadow (obj22HasuShadowDL's list) and a ride frame. Players standing on it push it down by
 * their fall speed; it springs back to its height, and a hard push makes a ring. The ride data
 * read into a local first and one unused word give the game's stack frame. */
s32 obj22HasuProc(Obj *obj, s16 msg, void *arg) {
    HasuWork *w = (HasuWork *) ((u32) obj + sizeof(Obj));
    s32 pad;
    s32 size;
    RideData *data;
    s32 i;
    s32 zone;
    Vec3f v;

    switch (msg) {
        case 2:
            obj->name = "hasu";
            size = eneSetup(obj, arg);
            if ((w->pol = eneNewPol(obj, obj22HasuDl)) == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayers[3]);
            w->pol->scale[0] = w->pol->scale[2] = (f32) size / 1000.0f;
            w->pol->scale[1] = 0.15f;
            if ((w->shadow = (PolFrame *) frmCreateNext(gFrmLayers[3], gFrmKindPol)) == NULL) {
                return -1;
            }
            w->shadow->dl2 = obj22HasuShadowDl;
            eneGetSegs(obj, w->shadow->segs);
            w->shadow->dl = (Gfx *) fadeSegToRam(obj22HasuShadowDL, obj->file);
            *(Vec3f *) w->shadow->pos = *(Vec3f *) w->pol->pos;
            *(Vec3f *) w->shadow->scale = *(Vec3f *) w->pol->scale;
            data = (RideData *) eneFileAddr(obj, obj22HasuRide, 0);
            if ((w->ride = rideCreate(gRideTree1, (s32) obj, &w->pol->f, data)) == NULL) {
                return -1;
            }
            w->rotY = vramCourseHeading(w->zone, w->x);
            w->baseY = w->y;
            break;
        case 1:
            frmDestroy(&w->ride->f);
            frmDestroy(&w->pol->f);
            frmDestroy(&w->shadow->f);
            break;
        case 6:
            ((PlPlace *) arg)->height += w->vy;
            for (i = 0; i < 4; i++) {
                if (plIsInPlay(i)) {
                    plAskPos(gGameWork.players[i], &v.x, &v.y, &v.z, &zone);
                    if (fabsf(((PlPlace *) arg)->side - v.z) < 1.0f) {
                        plGetSpeed(gGameWork.players[i], &v);
                        w->push += v.y;
                    }
                }
            }
            break;
        case 0:
            if (w->push < -5.0f) {
                obj22HitringCreate(obj, w->pol->pos[0], w->baseY, w->pol->pos[2], w->pol->scale[0]);
            }
            w->vy += w->push;
            w->push = 0.0f;
            w->vy += -(w->y - w->baseY) * 0.3;
            if (w->y <= w->baseY) {
                w->vy *= 0.1;
            }
            w->y += w->vy;
            if (w->y - w->baseY < -3.579999924f) {
                w->y = w->baseY - 3.579999924f;
            }
            eneSetModelOnCourse(obj, &w->pol->f);
            eneSetModelOnCourse(obj, &w->shadow->f);
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The height of the first player in play (0 with none). */
f32 obj22FirstPlayerY(void) {
    s32 i;
    f32 x;
    f32 y;
    f32 z;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plGetModelPos(gGameWork.players[i], &x, &y, &z);
            return y;
        }
    }
    return 0.0f;
}

/* "elevator22": a lift that rises eneSetup's result above where it is made and back. It waits
 * (states 0/3) until the players riding it (counted by message 6 each frame) are all of the
 * players, or one is already at the other end, then moves (2 up, 5 down) at 1 a frame, with a
 * hum (0x12A) and a stop (0x12B). Message 4 copies its place to arg. */
s32 obj22Elevator22Proc(Obj *obj, s16 msg, void *arg) {
    ElevWork *w = (ElevWork *) ((u32) obj + sizeof(Obj));
    s32 pad;
    s32 height;
    RideData *data;

    switch (msg) {
        case 2:
            obj->name = "elevator22";
            height = eneSetup(obj, arg);
            if ((w->pol = eneNewPol(obj, obj22Elevator22Dl)) == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayer3);
            data = (RideData *) eneFileAddr(obj, obj22Elevator22Ride, 0);
            if ((w->ride = rideCreate(gRideTree1, (s32) obj, &w->pol->f, data)) == NULL) {
                return -1;
            }
            w->baseY = w->y;
            w->height = height;
            if (w->baseY + w->height / 2 < obj22FirstPlayerY()) {
                w->state = 4;
                w->offset = w->height;
            } else {
                w->state = 1;
            }
            break;
        case 1:
            frmDestroy(&w->ride->f);
            frmDestroy(&w->pol->f);
            sndPlay(0x812A);
            sndPlay(0x812B);
            break;
        case 6:
            w->riders++;
            break;
        case 4:
            *(Vec3f *) arg = *(Vec3f *) w->pol->pos;
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->speed = 0.0f;
                    if (((CountFunc) plCountPlayers)() != w->riders) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    w->speed = 0.0f;
                    if (((CountFunc) plCountPlayers)() == w->riders) {
                        w->state = 2;
                    }
                    if (w->baseY + w->height - 10.0f < obj22FirstPlayerY()) {
                        w->state = 2;
                    }
                    if (w->state == 2) {
                        tskDestroyChildren(obj);
                        soundActpanCreateCam(obj, 0x12A);
                    }
                    break;
                case 2:
                    w->speed = 1.0f;
                    if (w->height < w->offset) {
                        w->state = 3;
                        if (obj22FirstPlayerY() < w->baseY + 10.0f) {
                            tskDestroyChildren(obj);
                            soundActpanCreateCam(obj, 0x812A);
                            w->state = 5;
                        } else {
                            SoundLocation2(0x12B, gGameWork.view, w->pol->pos[0], w->pol->pos[1], w->pol->pos[2],
                                           180.0f, 800.0f, 1.570796371f);
                        }
                    }
                    break;
                case 3:
                    w->speed = 0.0f;
                    if (((CountFunc) plCountPlayers)() != w->riders) {
                        w->state = 4;
                    }
                    break;
                case 4:
                    w->speed = 0.0f;
                    if (((CountFunc) plCountPlayers)() == w->riders) {
                        w->state = 5;
                    }
                    if (obj22FirstPlayerY() < w->baseY + 10.0f) {
                        w->state = 5;
                    }
                    if (w->state == 5) {
                        tskDestroyChildren(obj);
                        soundActpanCreateCam(obj, 0x12A);
                    }
                    break;
                case 5:
                    w->speed = -1.0f;
                    if (w->offset < 0.0f) {
                        w->state = 0;
                        if (w->baseY + w->height - 10.0f < obj22FirstPlayerY()) {
                            tskDestroyChildren(obj);
                            soundActpanCreateCam(obj, 0x812A);
                            w->state = 2;
                        } else {
                            SoundLocation2(0x12B, gGameWork.view, w->pol->pos[0], w->pol->pos[1], w->pol->pos[2],
                                           180.0f, 800.0f, 1.570796371f);
                        }
                    }
                    break;
            }
            w->offset += w->speed;
            w->riders = 0;
            w->y = w->baseY + w->offset;
            eneSetModelOnCourse(obj, &w->pol->f);
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "water": a stretch of flowing water, kind 1-6 from eneSetup's low byte (its angle in the
 * upper half, 512 a half turn), each a model with its scrolling texture list (obj22ScrollTexA,
 * obj22ScrollTexB or obj22ScrollTexFalls) and, for 1-5, a ride frame; a looping sound if its work has
 * one. Message 4 copies its place to arg. */
s32 obj22WaterProc(Obj *obj, s16 msg, void *arg) {
    s32 pad[2];
    WaterWork *w = (WaterWork *) ((u32) obj + sizeof(Obj));
    s32 r;

    switch (msg) {
        case 2:
            obj->name = "water";
            r = eneSetup(obj, arg);
            w->draw = obj22ScrollTexB;
            w->step = -1;
            switch ((u8) r) {
                case 1:
                    w->pol = eneNewPol(obj, obj22WaterR1Dl);
                    w->ride = rideCreate(gRideTree2, (s32) obj, &w->pol->f,
                                         (RideData *) eneFileAddr(obj, obj22WaterR1Ride, 0));
                    break;
                case 2:
                    w->pol = eneNewPol(obj, obj22WaterR2Dl);
                    w->ride = rideCreate(gRideTree2, (s32) obj, &w->pol->f,
                                         (RideData *) eneFileAddr(obj, obj22WaterR2Ride, 0));
                    break;
                case 3:
                    w->pol = eneNewPol(obj, obj22WaterR3Dl);
                    w->ride = rideCreate(gRideTree2, (s32) obj, &w->pol->f,
                                         (RideData *) eneFileAddr(obj, obj22WaterR3Ride, 0));
                    break;
                case 4:
                    w->draw = obj22ScrollTexA;
                    w->step = -4;
                    w->pol = eneNewPol(obj, obj22WaterR4Dl);
                    w->ride = rideCreate(gRideTree2, (s32) obj, &w->pol->f,
                                         (RideData *) eneFileAddr(obj, obj22WaterR4Ride, 0));
                    break;
                case 5:
                    w->draw = obj22ScrollTexA;
                    w->step = -4;
                    w->pol = eneNewPol(obj, obj22WaterR5Dl);
                    w->ride = rideCreate(gRideTree2, (s32) obj, &w->pol->f,
                                         (RideData *) eneFileAddr(obj, obj22WaterR5Ride, 0));
                    break;
                case 6:
                    w->draw = obj22ScrollTexFalls;
                    w->step = -4;
                    w->pol = eneNewPol(obj, obj22WaterR6Dl);
                    break;
            }
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, ((Frame **) &gFrmLayer4)[2]);
            w->pol->scale[2] = 0.1f;
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
            w->rotY = (r >> 16) * 3.1415927f / 512;
            if (w->sound != 0) {
                soundActpanCreate(obj, w->sound, gGameWork.view, 200.0f, 400.0f, gGameWork.view->fovy);
            }
            break;
        case 1:
            if (w->ride != NULL) {
                frmDestroy(&w->ride->f);
            }
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            break;
        case 4:
            if (arg != NULL) {
                ((f32 *) arg)[0] = w->pol->pos[0];
                ((f32 *) arg)[1] = w->pol->pos[1];
                ((f32 *) arg)[2] = w->pol->pos[2];
            }
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gGfxDlHead = w->draw(gGfxDlHead, w->scroll);
            w->scroll += w->step;
            eneSetModelOnCourse(obj, &w->pol->f);
            break;
        case 0:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "itex": this file's sparkle, an animated "anime" frame that follows its target task (asked
 * for its place with message 4 each frame), growing while it fades by 8. */
s32 obj22ItexProc(Obj *obj, s16 msg, void *arg) {
    s32 pad[2];
    SparkleWork *w = (SparkleWork *) ((u32) obj + sizeof(Obj));
    Vec3f v;

    switch (msg) {
        case 2:
            obj->name = "itex";
            if ((w->frame = (AnimeFrame *) frmCreate(gFrmLayer8, gFrmKindAnime)) == NULL) {
                return -1;
            }
            w->frame->anim = obj22ItexAnim;
            floadGetSetRam((void **) w->frame->segs, 0x123);
            w->frame->scale[2] = 0.3f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            fadeAnimStart(&w->anim, gItexSparkleAnim, 0, &w->frame->segs[3], floadGetSetFile(0x123, 0));
            w->alpha = 0xFF;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (fadeAnimStep(&w->anim)) {
                return -1;
            }
            v.x = v.y = v.z = 0.0f;
            tskSendMsg(w->target, 4, &v);
            w->frame->pos[0] = w->offset[0] + v.x;
            w->frame->pos[1] = w->offset[1] + v.y;
            w->frame->pos[2] = w->offset[2] + v.z;
            w->frame->scale[0] *= 1.001;
            w->frame->scale[1] *= 1.01;
            w->frame->scale[2] *= 1.001;
            w->alpha -= 8;
            if (w->alpha < 0) {
                w->alpha = 0;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0,
                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 255, 0);
            gDPSetEnvColor(gGfxDlHead++, 255, 255, 255, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A sparkle at (x, y + 5, z), turned rotY, of a size, following target from there. */
Obj *obj22ItexCreate(Obj *target, void *arg, f32 x, f32 y, f32 z, f32 rotY, f32 scale) {
    Obj *t;
    SparkleWork *w;
    s32 pad;
    Vec3f v;

    t = tskCreate(gEffectTask, obj22ItexProc, target->file, arg);
    if (t != NULL) {
        w = (SparkleWork *) ((u32) t + sizeof(Obj));
        w->frame->pos[0] = x;
        w->frame->pos[1] = y + 5.0f;
        w->frame->pos[2] = z;
        w->frame->rot[1] = rotY;
        w->frame->scale[2] = scale;
        w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
        w->target = target;
        v.x = 0.0f;
        v.y = 0.0f;
        v.z = 0.0f;
        tskSendMsg(w->target, 4, &v);
        w->offset[0] = w->frame->pos[0] - v.x;
        w->offset[1] = w->frame->pos[1] - v.y;
        w->offset[2] = w->frame->pos[2] - v.z;
    }
    return t;
}

/* The waterfall's foot: a sparkle every eight frames near at, and this frame's copy of its
 * vertices (obj22WaterfallVtx) with the 16 lower ones (obj22FallsFootVtx) moved down to len (in the model's
 * scale) and their texture stretched to match. w is made at its declaration and the test reads
 * through it; the two unused words give the game's stack frame. */
Vtx *obj22FallsFoot(Obj *obj, Vec3f *at, f32 len, f32 scale) {
    FallsWork *w = (FallsWork *) ((u32) obj + sizeof(Obj));
    s32 i;
    s16 h;
    s32 pad;
    Vtx *vtx;
    s32 pad2;
    s16 t;

    if ((w->count & 7) == 1) {
        obj22ItexCreate(obj, NULL, fadeRand(5) + at->x - 2.0f, at->y, fadeRand(5) + at->z - 2.0f,
                               w->pol->rot[1], scale);
    }
    w->count++;
    vtx = (Vtx *) gGfxDlHead;
    gGfxDlHead = (Gfx *) ((u8 *) gGfxDlHead + 0x420);
    heapMemCopy((u8 *) vtx, (u8 *) eneFileAddr(obj, obj22WaterfallVtx, 0), 0x420);
    h = len / w->pol->scale[1];
    t = ((h + 0xDE5) * 568 / 0xDE5) << 5;
    for (i = 0; i < 16; i++) {
        vtx[obj22FallsFootVtx[i]].v.ob[1] += h + 0xDE5;
        vtx[obj22FallsFootVtx[i]].v.tc[1] = t;
    }
    return vtx;
}

/* "falls22": a waterfall turned eneSetup's result (512 a half turn), its texture scrolling
 * (obj22ScrollTexFalls); its vertices as in the file. */
s32 obj22Falls22Proc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    FallsWork *w = (FallsWork *) ((u32) obj + sizeof(Obj));
    s32 rot;

    switch (msg) {
        case 2:
            obj->name = "falls22";
            rot = eneSetup(obj, arg);
            if ((w->pol = eneNewPol(obj, obj22Falls22Dl)) == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, ((Frame **) &gFrmLayer4)[2]);
            w->rotY = (f32) rot / 512 * 3.1415927f;
            break;
        case 1:
            frmDestroy(&w->pol->f);
            break;
        case 0:
            eneSetModelOnCourse(obj, &w->pol->f);
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gGfxDlHead = obj22ScrollTexFalls(gGfxDlHead, w->scroll);
            w->scroll -= 8;
            w->pol->segs[5] = eneFileAddr(obj, obj22WaterfallVtx, 0);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "falls22V": a waterfall that finds the ground below it (a ray 50 down from 10 under it) and
 * ends there, with its foot's sparkles; its foot's place for message 4.
 * Matched 2026-10-12; it was parked for t's stack slot. eneNewPol's result is stored, then
 * tested on its own line: the game's frame. */
s32 obj22Falls22VProc(Obj *obj, s16 msg, void *arg) {
    s32 pad; /* FAKE: an unused word at the top of the frame (a local the game never kept) */
    FallsWork *w = (FallsWork *) ((u32) obj + sizeof(Obj));
    s32 rot;
    Vec3f from;
    Vec3f dir;
    Vec3f hit;
    f32 t;

    switch (msg) {
        case 2:
            obj->name = "falls22V";
            rot = eneSetup(obj, arg);
            w->pol = eneNewPol(obj, obj22Falls22Dl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, ((Frame **) &gFrmLayer4)[2]);
            w->rotY = (f32) rot / 512 * 3.1415927f;
            break;
        case 1:
            frmDestroy(&w->pol->f);
            break;
        case 0:
            eneSetModelOnCourse(obj, &w->pol->f);
            break;
        case 4:
            if (arg != NULL) {
                *(Vec3f *) arg = w->foot;
            }
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gGfxDlHead = obj22ScrollTexFalls(gGfxDlHead, w->scroll);
            w->scroll -= 8;
            from.x = w->pol->pos[0];
            from.y = w->pol->pos[1] - 10.0f;
            from.z = w->pol->pos[2];
            dir.x = 0.0f;
            dir.y = -50.0f;
            dir.z = 0.0f;
            if (vramRayGround(&from, &dir, &hit, &t) != NULL) {
                w->foot = hit;
                w->pol->segs[5] = (u32) obj22FallsFoot(obj, &hit, dir.y * t - 10.0f, 0.25f);
            } else {
                w->pol->segs[5] = eneFileAddr(obj, obj22WaterfallVtx, 0);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "falls22R": a waterfall that falls onto a ride (a probe 100 down) and ends there, 10 out
 * along its own heading, with its foot's sparkles; its foot's place for message 4. The angle is
 * "(s32) (rot / pi * 512)", a float straight into mathSinCos's u16.
 * Matched 2026-10-12; it was parked for c's stack slot. As in falls22V, eneNewPol's result is
 * stored, then tested on its own line. */
s32 obj22Falls22RProc(Obj *obj, s16 msg, void *arg) {
    s32 pad; /* FAKE: an unused word at the top of the frame (a local the game never kept) */
    FallsWork *w = (FallsWork *) ((u32) obj + sizeof(Obj));
    s32 rot;
    Vec3f from;
    Vec3f dir;
    Vec3f out;
    f32 s;
    s32 pad2; /* FAKE: an unused word between s and c */
    f32 c;

    switch (msg) {
        case 2:
            obj->name = "falls22R";
            rot = eneSetup(obj, arg);
            w->pol = eneNewPol(obj, obj22Falls22Dl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, ((Frame **) &gFrmLayer4)[2]);
            w->rotY = (f32) rot / 512 * 3.1415927f;
            break;
        case 1:
            frmDestroy(&w->pol->f);
            break;
        case 0:
            eneSetModelOnCourse(obj, &w->pol->f);
            break;
        case 4:
            if (arg != NULL) {
                *(Vec3f *) arg = w->foot;
            }
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gGfxDlHead = obj22ScrollTexFalls(gGfxDlHead, w->scroll);
            w->scroll -= 8;
            from.x = w->pol->pos[0];
            from.y = w->pol->pos[1];
            from.z = w->pol->pos[2];
            dir.x = 0.0f;
            dir.y = -100.0f;
            dir.z = 0.0f;
            if (rideMoveStop(&from, &dir, &out, (s32) obj)) {
                s = mathSinCos((s32) (w->pol->rot[1] / 3.1415927f * 512), &c);
                from.x += out.x + 10.0f * c;
                from.y += out.y;
                from.z += out.z + 10.0f * s;
                w->foot = from;
                w->pol->segs[5] = (u32) obj22FallsFoot(obj, &from, out.y, 0.3f);
            } else {
                w->pol->segs[5] = eneFileAddr(obj, obj22WaterfallVtx, 0);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
