#include "tlb.h"

extern u8 waterScrollTex[];

Gfx *waterDraw(Gfx *gfx, u16 s);

/* A 32 x h I8 texture loaded at tmem and drawn through tile rtile from (uls, ult): one macro,
 * its seven commands on one source line (file 55's, as tlb_0053's RGBA16 one). */
#define gDPLoadScrollBlockI8(pkt, timg, h, maskt, tmem, rtile, uls, ult)                                  \
    {                                                                                                     \
        gDPSetTextureImage(pkt, G_IM_FMT_I, G_IM_SIZ_16b, 1, timg);                                       \
        gDPSetTile(pkt, G_IM_FMT_I, G_IM_SIZ_16b, 0, tmem, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR,    \
                   maskt, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD);                          \
        gDPLoadSync(pkt);                                                                                 \
        gDPLoadBlock(pkt, G_TX_LOADTILE, 0, 0, 32 * (h) / 2 - 1, CALC_DXT(32, G_IM_SIZ_8b_BYTES));        \
        gDPPipeSync(pkt);                                                                                 \
        gDPSetTile(pkt, G_IM_FMT_I, G_IM_SIZ_8b, 4, tmem, rtile, 0, G_TX_WRAP | G_TX_NOMIRROR, maskt,      \
                   G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD);                                 \
        gDPSetTileSize(pkt, rtile, uls, ult, 31 << G_TEXTURE_IMAGE_FRAC, ((h) - 1) << G_TEXTURE_IMAGE_FRAC); \
    }

/* "water"'s display list: two layers of one 32x32 I8 texture (segment 4), scrolled against
 * each other by s, blended by the environment alpha (file 55's waterDrawScroll32). */
Gfx *waterDraw(Gfx *gfx, u16 s) {
    gDPPipeSync(gfx++);
    gDPSetCombineLERP(gfx++, TEXEL0, TEXEL1, ENV_ALPHA, TEXEL1, 0, 0, 0, PRIMITIVE, PRIMITIVE, ENVIRONMENT,
                      COMBINED, ENVIRONMENT, 0, 0, 0, COMBINED);
    gDPSetPrimColor(gfx++, 0, 0, 0xFF, 0xFF, 0xFF, 0xA0);
    gDPSetEnvColor(gfx++, 0x10, 0x40, 0xFF, 0x80);
    gDPLoadScrollBlockI8(gfx++, waterScrollTex, 32, 5, 0, G_TX_RENDERTILE, s, s);
    gDPLoadScrollBlockI8(gfx++, waterScrollTex, 32, 5, 0x100, 1, s + 10, -s);
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* "water": a water surface (a pol model, one of three by the placement's low byte) drawn with
 * waterDraw's scrolling layers; a ride when its shape has collision. */
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
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ s32 scroll;
    /* 0x40 */ s32 step; /* the scroll's each frame */
    /* 0x44 */ Gfx *(*draw)(Gfx *, u16);
} WaterObj;

extern Gfx waterScrollData1Dl[];
extern Gfx waterScrollData2Dl[];
extern Gfx waterScrollData3Dl[];
extern u8 waterScrollData1Ride[];
extern u8 waterScrollData2Ride[];
extern u8 waterScrollData3Ride[];

s32 waterScrollProc(Obj *obj, s16 msg, void *arg) {
    WaterObj *w = (WaterObj *) FX(obj);
    s32 pad[2];
    s32 data;
    Gfx *dl;
    u8 *ride;

    switch (msg) {
        case 2:
            obj->name = "water";
            data = eneSetup(obj, (EneArg *) arg);
            w->draw = waterDraw;
            w->step = -1;
            dl = NULL;
            ride = NULL;
            switch (data & 0xFF) {
                case 1:
                    dl = waterScrollData1Dl; ride = waterScrollData1Ride;
                    break;
                case 2:
                    dl = waterScrollData2Dl; ride = waterScrollData2Ride;
                    break;
                case 3:
                    dl = waterScrollData3Dl; ride = waterScrollData3Ride;
                    break;
            }
            w->pol = eneNewPol(obj, dl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayers[6]);
            w->pol->scale[2] = 0.1f;
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
            w->rot[1] = (f32) (data >> 16) * 3.141592741f / 512;
            if (ride != NULL) {
                w->ride = rideCreate(gRideTree2, (s32) obj, &w->pol->f, (RideData *) eneFileAddr(obj, ride, 0));
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
                ((Vec3f *) arg)->x = w->pol->pos[0];
                ((Vec3f *) arg)->y = w->pol->pos[1];
                ((Vec3f *) arg)->z = w->pol->pos[2];
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
