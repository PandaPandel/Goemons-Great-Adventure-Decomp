#include "tlb.h"
Gfx waterBUnusedDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_CULL_BACK),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT),
    gsDPSetEnvColor(0x00, 0x00, 0x00, 0x00),
    gsDPSetRenderMode(Z_CMP | Z_UPD | IM_RD | CVG_DST_FULL | ZMODE_XLU | FORCE_BL | GBL_c1(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA), Z_CMP | Z_UPD | IM_RD | CVG_DST_FULL | ZMODE_XLU | FORCE_BL | GBL_c2(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)),
    gsSPEndDisplayList(),
    gsDPNoOp(),
};

extern u8 waterBScrollDlTex[];
extern u8 waterBDl[];
extern u8 waterBRide[];

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
    /* 0x3C */ s32 scroll; /* the textures' place (its low half is used) */
    /* 0x40 */ s32 step;   /* added to it each frame */
    /* 0x44 */ Gfx *(*draw)(Gfx *gfx, u16 scroll); /* the texture list's builder */
} WaterObj;

void waterBUnusedStub1(void) {
}

/* A 32x32 RGBA16 texture loaded into TMEM at tmem for tile rtile, as gDPLoadMultiBlock, with its
 * tile size at (uls, ult): the game's code is one macro (one source line: IDO schedules by line),
 * as in tlb_0050. */
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

/* The water's two scrolling textures (tlb_0050's waterRippleDL again). */
Gfx *waterBScrollDl(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 128, 128, 128, 96);
    gDPLoadScrollBlock(gfx++, waterBScrollDlTex, 0, G_TX_RENDERTILE, scroll, scroll);
    gDPLoadScrollBlock(gfx++, waterBScrollDlTex, 0x100, 1, scroll + 10, -scroll);
    gSPEndDisplayList(gfx++);
    return gfx;
}

void waterBUnusedStub2(void) {
}

/* "water": a water surface (a "pol" frame you can stand on, a ride), its texture scrolling one
 * back a frame. eneSetup's data: the low byte the model (1), the high half its turn (1024 a
 * circle). */
s32 waterBProc(Obj *obj, s16 msg, void *arg) {
    WaterObj *w = (WaterObj *) ((u32) obj + sizeof(Obj));
    s32 pad[2];
    s32 data;

    switch (msg) {
        case 2:
            obj->name = "water";
            data = eneSetup(obj, arg);
            w->draw = waterBScrollDl;
            w->step = -1;
            switch ((u8) data) {
                case 1:
                    w->frame = eneNewPol(obj, (Gfx *) waterBDl);
                    w->ride = rideCreate(gRideTree2, (s32) obj, &w->frame->f,
                                         (RideData *) eneFileAddr(obj, waterBRide, 0));
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
            gGfxDlHead = w->draw(gGfxDlHead, w->scroll);
            w->scroll += w->step;
            eneSetModelOnCourse(obj, &w->frame->f);
            break;
        case 0:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
