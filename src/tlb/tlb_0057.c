#include "tlb.h"

/* "magma": a scrolling lava surface (some with a ride on it) that burns a player standing in
 * it (message 6, from the course check). */

extern u8 magmaDrawScrollTex[];
extern Gfx magmaData1Dl[];
extern Gfx magmaData2Dl[];
extern Gfx magmaData3Dl[];
extern Gfx magmaData4Dl[];
extern Gfx magmaData5Dl[];
extern Gfx magmaData6Dl[];
extern Gfx magmaData7Dl[];
extern Gfx magmaData8Dl[];
extern Gfx magmaData9Dl[];
extern Gfx magmaData10Dl[];
extern Gfx magmaData11Dl[];
extern Gfx magmaData12Dl[];
extern Gfx magmaData13Dl[];
extern Gfx magmaData14Dl[];
extern Gfx magmaData15Dl[];
extern u8 magmaData1Ride[];
extern u8 magmaData3Ride[];
extern u8 magmaData4Ride[];
extern u8 magmaData5Ride[];
extern u8 magmaData6Ride[];
extern u8 magmaData7Ride[];
extern u8 magmaData8Ride[];
extern u8 magmaData11Ride[];
extern u8 magmaData12Ride[];
extern u8 magmaData13Ride[];
extern u8 magmaData14Ride[];
extern u8 magmaData15Ride[];

/* A lava surface's work ("magma"). */
typedef struct MagmaWork {
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
    /* 0x3C */ u32 scroll;
} MagmaWork;

/* Message 6's argument: a place on the course to burn. */
typedef struct MagmaTouch {
    /* 0x00 */ s16 zone;
    /* 0x04 */ f32 place;
    /* 0x08 */ f32 side;
} MagmaTouch;

/* A 32x32 RGBA16 texture loaded at tmem and drawn through tile rtile from (uls, ult): one macro,
 * its seven commands on one source line (tlb_0053's). */
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

/* The lava's two scrolling texture layers. */
Gfx *magmaDrawScroll(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 0x80, 0x80, 0x80, 0x80);
    gDPLoadScrollBlock(gfx++, magmaDrawScrollTex, 0, G_TX_RENDERTILE, scroll, scroll);
    gDPLoadScrollBlock(gfx++, magmaDrawScrollTex, 0x100, 1, scroll + 10, -scroll);
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* "magma": its kind's value: the model in the low byte (14 drawn 0.12 instead of 0.1), the
 * heading in 512ths of a half turn in the high half. */
s32 magmaProc(Obj *o, s16 msg, void *arg) {
    MagmaWork *w = (MagmaWork *) FX(o);
    s32 i;
    ACollFrame *box;
    s32 data;
    Gfx *dl;
    u32 ride;
    s32 pad[2]; /* the game's frame words */
    s32 zone;
    f32 pos[3];
    s32 pad2;

    switch (msg) {
        case 2:
            o->name = "magma";
            data = eneSetup(o, arg);
            switch (data & 0xFF) {
                default:
                    break;
                /* each model's pair on one line: IDO schedules by line */
                case 1:
                    dl = magmaData1Dl; ride = (u32) magmaData1Ride;
                    break;
                case 2:
                    dl = magmaData2Dl; ride = 0;
                    break;
                case 3:
                    dl = magmaData3Dl; ride = (u32) magmaData3Ride;
                    break;
                case 4:
                    dl = magmaData4Dl; ride = (u32) magmaData4Ride;
                    break;
                case 5:
                    dl = magmaData5Dl; ride = (u32) magmaData5Ride;
                    break;
                case 6:
                    dl = magmaData6Dl; ride = (u32) magmaData6Ride;
                    break;
                case 7:
                    dl = magmaData7Dl; ride = (u32) magmaData7Ride;
                    break;
                case 8:
                    dl = magmaData8Dl; ride = (u32) magmaData8Ride;
                    break;
                case 9:
                    dl = magmaData9Dl; ride = 0;
                    break;
                case 10:
                    dl = magmaData10Dl; ride = 0;
                    break;
                case 11:
                    dl = magmaData11Dl; ride = (u32) magmaData11Ride;
                    break;
                case 12:
                    dl = magmaData12Dl; ride = (u32) magmaData12Ride;
                    break;
                case 13:
                    dl = magmaData13Dl; ride = (u32) magmaData13Ride;
                    break;
                case 14:
                    dl = magmaData14Dl; ride = (u32) magmaData14Ride;
                    break;
                case 15:
                    dl = magmaData15Dl; ride = (u32) magmaData15Ride;
                    break;
            }
            if (dl != NULL) {
                w->pol = eneNewPol(o, dl);
            }
            if (w->pol == NULL) {
                return -1;
            }
            if (ride != 0) {
                w->ride = rideCreate(gRideTree2, (s32) o, &w->pol->f, (RideData *) eneFileAddr(o, ride, 0));
            }
            frmMove(&w->pol->f, gFrmLayers[6]);
            w->pol->scale[2] = 0.1f;
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
            w->rot[1] = (data >> 16) * 3.141592741f / 512;
            if ((data & 0xFF) == 14) {
                w->pol->scale[2] = 0.12f;
                w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
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
        case 6:
            if (arg != NULL) {
                for (i = 0; i < 4; i++) {
                    if (plIsInPlay(i)) {
                        plAskPos(gGameWork.players[i], &pos[0], &pos[1], &pos[2], &zone);
                        if (zone == ((MagmaTouch *) arg)->zone &&
                            fabsf(((MagmaTouch *) arg)->side - pos[2]) < 0.1f &&
                            fabsf(((MagmaTouch *) arg)->place - pos[0]) < 4.0f) {
                            box = plGetDamageBox(gGameWork.players[i]);
                            if (box->hitKind != 110 && box->hitKind != 111 && box->hitKind != 112) {
                                box->hitKind = 124;
                                tskSendMsg(gGameWork.players[i], 7, o);
                            }
                        }
                    }
                }
            }
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gGfxDlHead = magmaDrawScroll(gGfxDlHead, w->scroll);
            w->scroll--;
            eneSetModelOnCourse(o, &w->pol->f);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
