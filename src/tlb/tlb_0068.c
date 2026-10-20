#include "tlb.h"

/* water: a water surface, its texture (segment 4's) drawn twice over itself, the two layers
 * scrolling against each other; a ride of the second tree (rideTree2PointIn finds it). */

extern u8 waterRideTex[];
extern Gfx waterRideData1Dl[];
extern Gfx waterRideData2Dl[];
extern Gfx waterRideData3Dl[];
extern Gfx waterRideData4Dl[];
extern Gfx waterRideData5Dl[];
extern Gfx waterRideData6Dl[];
extern u8 waterRideData1Ride[];
extern u8 waterRideData3Ride[];
extern u8 waterRideData4Ride[];
extern u8 waterRideData5Ride[];
extern u8 waterRideData6Ride[];

/* water's work, at its task + 0x20: an enemy's head (EneObj) up to 0x34. */
typedef struct WaterWork {
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
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ s32 scroll;
    /* 0x40 */ s32 step;
    /* 0x44 */ Gfx *(*draw)(Gfx *gfx, u16 scroll);
} WaterWork;

/* gbi.h's gDPLoadMultiBlock with the tile's top-left corner (uls, ult) given: one block, so it
 * is one source line as the gbi.h loaders are (written as separate statements, IDO schedules
 * the load commands' constants differently). */
#define gDPLoadMultiBlockAt(pkt, timg, tmem, rtile, fmt, siz, width, height, pal, cms, cmt, masks, maskt, shifts,  \
                            shiftt, uls, ult)                                                                       \
    {                                                                                                               \
        gDPSetTextureImage(pkt, fmt, siz##_LOAD_BLOCK, 1, timg);                                                    \
        gDPSetTile(pkt, fmt, siz##_LOAD_BLOCK, 0, tmem, G_TX_LOADTILE, 0, cmt, maskt, shiftt, cms, masks, shifts); \
        gDPLoadSync(pkt);                                                                                           \
        gDPLoadBlock(pkt, G_TX_LOADTILE, 0, 0, (((width) * (height) + siz##_INCR) >> siz##_SHIFT) - 1,             \
                     CALC_DXT(width, siz##_BYTES));                                                                 \
        gDPPipeSync(pkt);                                                                                           \
        gDPSetTile(pkt, fmt, siz, (((width) * siz##_LINE_BYTES) + 7) >> 3, tmem, rtile, pal, cmt, maskt, shiftt,   \
                   cms, masks, shifts);                                                                             \
        gDPSetTileSize(pkt, rtile, uls, ult, ((width) - 1) << G_TEXTURE_IMAGE_FRAC,                                 \
                       ((height) - 1) << G_TEXTURE_IMAGE_FRAC)                                                      \
    }

/* The surface's textures: segment 4's 32x32 I8 image in both tiles, the second shifted 10
 * across and scrolling the other way. */
Gfx *waterDrawTexture(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetCombineLERP(gfx++, TEXEL0, TEXEL1, ENV_ALPHA, TEXEL1, 0, 0, 0, PRIMITIVE, PRIMITIVE, ENVIRONMENT, COMBINED,
                      ENVIRONMENT, 0, 0, 0, COMBINED);
    gDPSetPrimColor(gfx++, 0, 0, 255, 255, 255, 160);
    gDPSetEnvColor(gfx++, 16, 64, 255, 128);
    gDPLoadMultiBlockAt(gfx++, waterRideTex, 0, G_TX_RENDERTILE, G_IM_FMT_I, G_IM_SIZ_8b, 32, 32, 0, G_TX_WRAP,
                        G_TX_WRAP, 5, 5, G_TX_NOLOD, G_TX_NOLOD, scroll, scroll);
    gDPLoadMultiBlockAt(gfx++, waterRideTex, 0x100, 1, G_IM_FMT_I, G_IM_SIZ_8b, 32, 32, 0, G_TX_WRAP, G_TX_WRAP, 5, 5,
                        G_TX_NOLOD, G_TX_NOLOD, scroll + 10, -scroll);
    gSPEndDisplayList(gfx++);
    return gfx;
}

s32 waterRideProc(Obj *o, s16 msg, void *arg) {
    WaterWork *w = (WaterWork *) FX(o);
    s32 pad;
    s32 pad2;
    s32 data;
    Gfx *dl;
    u32 ride;

    switch (msg) {
        case 2:
            o->name = "water";
            data = eneSetup(o, arg);
            w->draw = waterDrawTexture;
            w->step = -1;
            dl = NULL;
            ride = 0;
            switch (data & 0xFF) {
                case 1:
                    dl = waterRideData1Dl; ride = (u32) waterRideData1Ride;
                    break;
                case 2:
                    dl = waterRideData2Dl;
                    break;
                case 3:
                    dl = waterRideData3Dl; ride = (u32) waterRideData3Ride;
                    break;
                case 4:
                    dl = waterRideData4Dl; ride = (u32) waterRideData4Ride;
                    break;
                case 5:
                    dl = waterRideData5Dl; ride = (u32) waterRideData5Ride;
                    break;
                case 6:
                    dl = waterRideData6Dl; ride = (u32) waterRideData6Ride;
                    break;
            }
            w->pol = eneNewPol(o, dl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove((Frame *) w->pol, gFrmLayers[6]);
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2] = 0.1f;
            w->rot[1] = (data >> 16) * 3.1415927f / 512;
            if (ride != 0) {
                w->ride = rideCreate(gRideTree2, (s32) o, (Frame *) w->pol, (RideData *) eneFileAddr(o, ride, 0));
            }
            break;
        case 1:
            if (w->ride != NULL) {
                frmDestroy((Frame *) w->ride);
            }
            if (w->pol != NULL) {
                frmDestroy((Frame *) w->pol);
            }
            break;
        case 4:
            if (arg != NULL) {
                ((Vec3f *) arg)->x = w->pol->pos[0];
                ((Vec3f *) arg)->y = w->pol->pos[1];
                ((Vec3f *) arg)->z = w->pol->pos[2];
            }
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gGfxDlHead = w->draw(gGfxDlHead, w->scroll);
            w->scroll += w->step;
            eneSetModelOnCourse(o, (Frame *) w->pol);
            break;
        case 0:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
