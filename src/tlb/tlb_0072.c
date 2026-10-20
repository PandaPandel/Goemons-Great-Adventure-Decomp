#include "tlb.h"

/* libultra's, through their addresses (code at 0x08000000 cannot jal them). */
#define sinf (*sinf)
#define cosf (*cosf)

extern Frame *gFrmLayer3;

/* acollAtkAddYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);

/* eneMakeEx's last argument, here a value handed to the new enemy (mode 2). */
typedef u32 (*EneMakeArg)(void *, void *, void *);

/* Message 6's argument (plmove.c's PlPlace): a place on the course, moved by the object. */
typedef struct PlPlace {
    /* 0x0 */ s16 zone;
    /* 0x4 */ f32 place;
    /* 0x8 */ f32 side;
    /* 0xC */ f32 height;
} PlPlace;

/* In the stage's segment 3 file: the hot water's texture (32x32 CI4), the models' display lists
 * and their rides' data (RideData). */
extern u8 kamaHotwaterTex[];  /* the hot water's texture */
extern Gfx kamaHotwaterData1Dl[]; /* "hotwater" model 1, its ride kamaHotwaterData1Ride */
extern Gfx kamaHotwaterData2Dl[]; /* model 2, ride kamaHotwaterData2Ride */
extern Gfx kamaHotwaterData3Dl[]; /* model 3, ride kamaHotwaterData3Ride */
extern u8 kamaHotwaterData1Ride[];
extern u8 kamaHotwaterData2Ride[];
extern u8 kamaHotwaterData3Ride[];
extern Gfx kamaBoardDl[]; /* "kboard", its ride kamaBoardRide */
extern u8 kamaBoardRide[];
extern Gfx kamaDl[]; /* "kama", its ride kamaUnderRide */
extern Gfx kamaUnderDl[]; /* the model under the "kama" */
extern u8 kamaUnderRide[];
extern Gfx kamaNettouDl[]; /* "nettou" */
extern Gfx kamaSasiyuDl[]; /* "sasiyu" */

/* A "hotwater" pool's work (task + 0x20): an enemy's (EneObj) up to 0x34. */
typedef struct HotwaterObj {
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
    /* 0x3C */ s32 scroll;    /* the texture's, down one a frame (its low half is used) */
    /* 0x40 */ u16 count;     /* steps: a puff of steam every 16 */
    /* 0x44 */ f32 puffSize;
    /* 0x48 */ s32 spreadX;   /* how far from the centre the puffs rise, x and z (0: at it) */
    /* 0x4C */ s32 spreadZ;
} HotwaterObj;

/* A "kboard" board's work (task + 0x20): an enemy's (EneObj) up to 0x34. */
typedef struct KboardObj {
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
    /* 0x3C */ f32 gravity; /* added to vel[1] each step while it falls */
    /* 0x40 */ s32 alpha;
    /* 0x44 */ s32 fade;    /* added to alpha each step */
    /* 0x48 */ u8 mode;     /* 0 still, 1 stepped on, 2 falling, 3 fading out */
    /* 0x4A */ u16 count;   /* steps in this mode */
} KboardObj;

/* A "kama" cauldron's work (task + 0x20): an enemy's (EneObj) up to 0x34. */
typedef struct KamaObj {
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
    /* 0x3C */ PolFrame *under; /* a second model below it, rocked a little each step */
    /* 0x40 */ f32 angle;       /* of its bobbing */
    /* 0x44 */ f32 speed;       /* added to angle each step */
    /* 0x48 */ f32 baseY;
    /* 0x4C */ f32 amp;         /* how far it bobs */
    /* 0x50 */ u16 count;       /* steps */
} KamaObj;

/* A "nettou" (boiling water) drop's work (task + 0x20). */
typedef struct NettouWork {
    /* 0x00 */ u8 pad00[0x2C];
    /* 0x2C */ f32 vy;          /* set 0, never read */
    /* 0x30 */ u8 pad30[4];
    /* 0x34 */ PolFrame *frame;
    /* 0x38 */ ACollFrame *atk; /* hurts what it hits */
    /* 0x3C */ s32 alpha;
    /* 0x40 */ s32 fade;        /* added to alpha each step (never set here) */
    /* 0x44 */ s32 life;        /* steps until it goes (alpha set to -8) */
    /* 0x48 */ s32 puff;        /* steps until its puff of steam */
} NettouWork;

/* A "sasiyu" (pouring) kettle's work (task + 0x20): an enemy's (EneObj) up to 0x34. */
typedef struct SasiyuObj {
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
    /* 0x1C */ f32 rot[3];      /* rot[0] its tilt */
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *frame;
    /* 0x38 */ Obj *sound;      /* its pouring sound */
    /* 0x3C */ u8 mode;         /* 0 upright, 1 tilting, 2 pouring, 3 tilting back */
    /* 0x3E */ u16 count;       /* steps in this mode */
} SasiyuObj;

Gfx *kamaHotwaterDL(Gfx *gfx, u16 scroll);
s32 kamaNettouProc(Obj *obj, s16 msg, void *arg);
void kamaNettouMake(Obj *parent, f32 x, f32 y, f32 z);

/* A 32x32 CI4 texture loaded into TMEM at tmem for tile rtile, as gDPLoadMultiBlock_4b, with its
 * tile size at (uls, ult): one macro (one source line), as tlb_0050's RGBA16 one. */
#define gDPLoadScrollBlock_4b(pkt, timg, tmem, rtile, uls, ult)                                       \
    {                                                                                                  \
        gDPSetTextureImage(pkt, G_IM_FMT_CI, G_IM_SIZ_16b, 1, timg);                                   \
        gDPSetTile(pkt, G_IM_FMT_CI, G_IM_SIZ_16b, 0, tmem, G_TX_LOADTILE, 0,                          \
                   G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD); \
        gDPLoadSync(pkt);                                                                              \
        gDPLoadBlock(pkt, G_TX_LOADTILE, 0, 0, ((32 * 32 + 3) >> 2) - 1, CALC_DXT_4b(32));             \
        gDPPipeSync(pkt);                                                                              \
        gDPSetTile(pkt, G_IM_FMT_CI, G_IM_SIZ_4b, ((32 >> 1) + 7) >> 3, tmem, rtile, 0,               \
                   G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD); \
        gDPSetTileSize(pkt, rtile, uls, ult, (32 - 1) << 2, (32 - 1) << 2);                            \
    }

/* The "hotwater" surface: two layers of one texture scrolling against each other. */
Gfx *kamaHotwaterDL(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 128, 128, 128, 128);
    gDPLoadScrollBlock_4b(gfx++, kamaHotwaterTex, 0, G_TX_RENDERTILE, scroll, scroll);
    gDPLoadScrollBlock_4b(gfx++, kamaHotwaterTex, 0x80, 1, scroll + 10, -scroll);
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* "hotwater": a pool of hot water (a "pol" frame you can stand on, a ride), steaming; a player
 * in it (message 6, within 1 across) is sent a hit (message 7). eneSetup's data: the low byte the
 * model (1-3), the high half its turn (1024 a circle). */
s32 kamaHotwaterProc(Obj *obj, s16 msg, void *arg) {
    HotwaterObj *w = (HotwaterObj *) ((u32) obj + sizeof(Obj));
    s32 data;
    s32 i;
    s32 pad;
    Gfx *dl;
    u8 *ride;
    s32 pad2[2];
    s32 zone;
    f32 p[3]; /* place, height, side */
    f32 dz;
    f32 dx;
    s32 pad3;

    switch (msg) {
        case 2:
            obj->name = "hotwater";
            data = eneSetup(obj, arg);
            w->rot[1] = (data >> 16) * PI / 512;
            switch ((u8) data) {
                case 1:
                    w->puffSize = 0.1f;
                    dl = kamaHotwaterData1Dl; ride = kamaHotwaterData1Ride;
                    w->spreadZ = w->spreadX = 0;
                    break;
                case 2:
                    w->puffSize = 0.15f;
                    dl = kamaHotwaterData2Dl; ride = kamaHotwaterData2Ride;
                    w->spreadZ = w->spreadX = 50;
                    break;
                case 3:
                    w->puffSize = 0.2f;
                    dl = kamaHotwaterData3Dl; ride = kamaHotwaterData3Ride;
                    w->spreadZ = w->spreadX = 150;
                    break;
            }
            if (dl != NULL) {
                w->frame = eneNewPol(obj, dl);
            }
            if (w->frame == NULL) {
                return -1;
            }
            if (ride != NULL) {
                w->ride = rideCreate(gRideTree2, (s32) obj, &w->frame->f,
                                     (RideData *) eneFileAddr(obj, ride, 0));
            }
            frmMove(&w->frame->f, gFrmLayers[6]);
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            break;
        case 1:
            if (w->ride != NULL) {
                frmDestroy(&w->ride->f);
            }
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 6:
            if (arg != NULL) {
                for (i = 0; i < 4; i++) {
                    if (plIsInPlay(i)) {
                        plAskPos(gGameWork.players[i], &p[0], &p[1], &p[2], &zone);
                        if (fabsf(((PlPlace *) arg)->side - p[2]) < 1.0f) {
                            plGetDamageBox(gGameWork.players[i])->hitKind = 0x7C;
                            tskSendMsg(gGameWork.players[i], 7, obj);
                        }
                    }
                }
            }
            break;
        case 0:
            w->count++;
            if ((w->count & 0xF) == 1) {
                dx = (w->spreadX > 0) ? fadeRand(w->spreadX) - (f32) w->spreadX / 2 : 0.0f;
                dz = (w->spreadZ > 0) ? fadeRand(w->spreadZ) - (f32) w->spreadZ / 2 : 0.0f;
                itexPuff(w->frame->pos[0] + dx, w->frame->pos[1], w->frame->pos[2] + dz, w->puffSize);
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gGfxDlHead = kamaHotwaterDL(gGfxDlHead, w->scroll);
            w->scroll--;
            eneSetModelOnCourse(obj, &w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "kboard": a board on the water (a ride). A player moving on it faster than 2 (message 6, within
 * 1 across) starts it: after 10 steps it falls (gravity -0.3) until it lands or 30 steps pass,
 * then fades out and comes back where it was. Opaque, it is drawn with the models. */
s32 kamaBoardProc(Obj *obj, s16 msg, void *arg) {
    KboardObj *w = (KboardObj *) ((u32) obj + sizeof(Obj));
    s32 data;
    s32 i;
    f32 at[3];
    f32 t;
    s32 pad[2];
    s32 zone;
    f32 v[3]; /* place, height, side; then the player's speed */

    switch (msg) {
        case 2:
            obj->name = "kboard";
            data = eneSetup(obj, arg);
            w->rot[1] = data * PI / 512;
            w->alpha = 255;
            w->gravity = 0.0f;
            w->frame = eneNewPol(obj, kamaBoardDl);
            if (w->frame == NULL) {
                return -1;
            }
            frmMove(&w->frame->f, gFrmLayers[3]);
            w->frame->rot[1] = w->rot[1];
            w->ride = rideCreate(gRideTree1, (s32) obj, &w->frame->f,
                                 (RideData *) eneFileAddr(obj, kamaBoardRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            break;
        case 1:
            frmDestroy(&w->ride->f);
            frmDestroy(&w->frame->f);
            break;
        case 7:
            if (arg != NULL) {
                *(s32 *) arg = 3;
            }
            break;
        case 6:
            for (i = 0; i < 4; i++) {
                if (plIsInPlay(i)) {
                    plAskPos(gGameWork.players[i], &v[0], &v[1], &v[2], &zone);
                    if (fabsf(((PlPlace *) arg)->side - v[2]) < 1.0f) {
                        plGetSpeed(gGameWork.players[i], (Vec3f *) v);
                        if (sqrtf(v[2] * v[2] + (v[1] * v[1] + v[0] * v[0])) > 2.0f && w->mode == 0) {
                            w->count = 0;
                            w->mode = 1;
                        }
                    }
                }
            }
            ((PlPlace *) arg)->height += w->vel[1];
            break;
        case 0:
            w->count++;
            w->alpha += w->fade;
            if (w->alpha < 0) {
                w->alpha = 0;
            } else if (w->alpha > 255) {
                w->alpha = 255;
            }
            if (w->alpha == 255) {
                if (w->frame->f.parent != gFrmLayers[1]) {
                    frmMove(&w->frame->f, gFrmLayers[1]);
                }
            } else {
                if (w->frame->f.parent != gFrmLayers[8]) {
                    frmMove(&w->frame->f, gFrmLayers[8]);
                }
            }
            switch (w->mode) {
                case 0:
                    break;
                case 1:
                    if (w->count > 10) {
                        w->gravity = -0.3f;
                        w->count = 0;
                        w->mode = 2;
                    }
                    break;
                case 2:
                    w->vel[1] += w->gravity;
                    w->frame->pos[1] += w->vel[1];
                    if (vramRayGround((Vec3f *) w->frame->pos, (Vec3f *) w->vel, (Vec3f *) at, &t)) {
                        w->vel[1] = 0.0f;
                        frmMove(&w->frame->f, gFrmLayers[8]);
                        w->fade = -8;
                        w->mode = 3;
                    }
                    if (w->count > 30) {
                        w->fade = -8;
                        w->mode = 3;
                    }
                    break;
                case 3:
                    if (w->alpha <= 0) {
                        w->alpha = 0;
                        w->fade = 8;
                        w->frame->pos[1] = w->y;
                        w->count = 0;
                        w->mode = 0;
                    }
                    break;
            }
            eneStepHookPost(obj);
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombine(gGfxDlHead++, 0x409A81, 0xFF37FFFF);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "kboard" (a row): makes six boards (enemy 0xA4) 20 apart across the course, from -60 to 40,
 * each handed this one's data. */
s32 kamaBoardRowProc(Obj *obj, s16 msg, void *arg) {
    EneObj *e = ENE(obj);
    s32 data;
    f32 s;
    f32 fi;
    f32 c;
    s32 i;
    f32 x;
    f32 z;
    f32 px;
    f32 pz;

    if (msg == 2) {
        obj->name = "kboard";
        data = eneSetup(obj, arg);
        e->rot[1] = (f32) data * PI / 512;
        s = mathSinCos(data, &c);
        vramCoursePointXZ(e->zone, e->x, e->z, &x, &z);
        for (i = -60; i < 60; i += 20) {
            fi = i;
            px = c * fi + x;
            pz = z - s * fi;
            eneMakeEx(obj, ENE_KAMA_BOARD, 0, px, e->y, pz, 2, (EneMakeArg) data);
        }
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "kama": a cauldron (a ride) bobbing up and down (amp the data's low byte, a cycle of 180 steps
 * starting at the phase its second byte picks: 0, 45, 90 or 135), with a second model under it
 * that rocks; it makes enemy 0xA3 on that model. A player on it (message 6) bobs with it. The data's
 * high half is its turn (1024 a circle). */
s32 kamaProc(Obj *obj, s16 msg, void *arg) {
    KamaObj *w = (KamaObj *) ((u32) obj + sizeof(Obj));
    s32 data;
    s32 phase;
    s32 period = 180;
    f32 circle = PI * 2;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "kama";
            data = eneSetup(obj, arg);
            w->speed = circle / 180.0f;
            w->baseY = w->y;
            w->amp = data & 0xFF;
            w->rot[1] = (data >> 16) * PI / 512;
            switch ((data >> 8) & 0xFF) {
                default:
                case 0:
                    phase = 0;
                    break;
                case 1:
                    phase = 45;
                    break;
                case 2:
                    phase = 90;
                    break;
                case 3:
                    phase = 135;
                    break;
            }
            w->angle = (f32) ((phase + gGameFrameCount) % period * 2) * PI / period;
            w->frame = eneNewPol(obj, kamaDl);
            if (w->frame == NULL) {
                return -1;
            }
            frmMove(&w->frame->f, gFrmLayer3);
            w->under = eneNewPol(obj, kamaUnderDl);
            if (w->under == NULL) {
                return -1;
            }
            frmMove(&w->under->f, gFrmLayer3);
            w->under->pos[1] -= w->amp + 20.0f;
            w->under->rot[1] = w->rot[1] - PI / 2;
            w->ride = rideCreate(gRideTree1, (s32) obj, &w->frame->f,
                                 (RideData *) eneFileAddr(obj, kamaUnderRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            eneMakeEx(obj, ENE_KAMA_HOTWATER, 0, w->under->pos[0], w->under->pos[1] + 10.0f, w->under->pos[2], 2,
                      (EneMakeArg) 1);
            break;
        case 1:
            frmDestroy(&w->ride->f);
            frmDestroy(&w->frame->f);
            break;
        case 6:
            if (arg != NULL) {
                ((PlPlace *) arg)->height += cosf(w->angle) * w->speed * w->amp;
            }
            break;
        case 7:
            if (arg != NULL) {
                *(s32 *) arg = 3;
            }
            break;
        case 0:
            w->count++;
            switch (w->count & 7) {
                case 0:
                case 4:
                    w->under->rot[0] = 0.0f;
                    break;
                case 1:
                case 3:
                    w->under->rot[0] = 0.008726646192f;
                    break;
                case 2:
                    w->under->rot[0] = 0.01745329238f;
                    break;
                case 5:
                case 7:
                    w->under->rot[0] = -0.008726646192f;
                    break;
                case 6:
                    w->under->rot[0] = -0.01745329238f;
                    break;
            }
            w->angle += w->speed;
            w->y = sinf(w->angle) * w->amp + w->baseY;
            eneSetModelOnCourse(obj, &w->frame->f);
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "nettou": a drop of boiling water (a "pol" frame with an attack box) falling from a kettle: it
 * stretches as it falls, puffs steam once, hits what it touches (message 7 to the box that hit
 * it) and is gone after 60 steps. */
s32 kamaNettouProc(Obj *obj, s16 msg, void *arg) {
    NettouWork *w = (NettouWork *) ((u32) obj + sizeof(Obj));
    s32 pad;
    Obj *by; /* the one its box hit */

    switch (msg) {
        case 2:
            obj->name = "nettou";
            w->frame = (PolFrame *) frmCreateNext(gFrmLayers[6], gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = kamaNettouDl;
            floadGetSetRam((void **) w->frame->segs, 0x70);
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[2];
            w->frame->flags |= 7;
            w->alpha = 255;
            w->frame->scale[1] = 0.01f;
            w->life = 60;
            w->vy = 0.0f;
            w->puff = fadeRand(20) + 10;
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 100, -100, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x78;
            break;
        case 1:
            frmDestroy(&w->atk->frame);
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (--w->puff == 0) {
                itexPuff(w->frame->pos[0], w->frame->pos[1] - 10.0f, w->frame->pos[2], 0.1f);
            }
            if (--w->life < 0) {
                w->alpha = -8;
            }
            w->alpha += w->fade;
            if (w->alpha <= 0) {
                return -1;
            }
            w->frame->scale[1] += 0.0075f;
            if (w->frame->scale[1] > 0.1) {
                w->frame->scale[1] = 0.1f;
                w->frame->pos[1] += -3.0f;
            } else {
                w->frame->pos[1] += -0.2;
            }
            w->frame->scale[0] *= 0.99;
            w->frame->scale[2] *= 0.99;
            if (w->atk != NULL && acollAtkWasHit(w->atk) == 1) {
                by = (Obj *) w->atk->hitPower;
                if (by != NULL) {
                    tskSendMsg(by, 7, obj);
                }
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombine(gGfxDlHead++, 0xFF9A67, 0xF50F257F);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 255, w->alpha);
            gDPSetEnvColor(gGfxDlHead++, 255, 208, 96, 128);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Makes a "nettou" drop under parent at (x, y, z). */
void kamaNettouMake(Obj *parent, f32 x, f32 y, f32 z) {
    Obj *t = tskCreate(parent, kamaNettouProc, parent->file, NULL);

    if (t != NULL) {
        ((NettouWork *) FX(t))->frame->pos[0] = x;
        ((NettouWork *) FX(t))->frame->pos[1] = y;
        ((NettouWork *) FX(t))->frame->pos[2] = z;
    }
}

/* "sasiyu": a kettle that tilts (30 steps), pours boiling water ("nettou" drops every 8 steps, 20
 * in front of it, with its sound while the camera sees it) for 30 steps, tilts back and rests 30
 * steps. */
s32 kamaSasiyuProc(Obj *obj, s16 msg, void *arg) {
    SasiyuObj *w = (SasiyuObj *) ((u32) obj + sizeof(Obj));
    s32 data;
    s32 pad;
    f32 dx;
    f32 dz;

    switch (msg) {
        case 2:
            obj->name = "sasiyu";
            data = eneSetup(obj, arg);
            w->rot[1] = data * PI / 512;
            w->frame = eneNewPol(obj, kamaSasiyuDl);
            if (w->frame == NULL) {
                return -1;
            }
            frmMove(&w->frame->f, gFrmLayer3);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->count++;
            switch (w->mode) {
                case 0:
                    w->rot[0] = 0.0f;
                    if (w->count > 30) {
                        w->count = 0;
                        w->mode = 1;
                    }
                    break;
                case 1:
                    w->rot[0] += 0.05235987902f;
                    if (w->count > 30) {
                        if (soundInFovCheck(gGameWork.view, w->frame->pos[0], w->frame->pos[1] - 50.0f,
                                            w->frame->pos[2]) == 1) {
                            w->sound = soundSuionCreate(obj, 0x2CD, gGameWork.view, w->frame->pos[0],
                                                        w->frame->pos[1] - 50.0f, w->frame->pos[2], 0.0f, 0.0f);
                        }
                        w->count = 0;
                        w->mode = 2;
                    }
                    break;
                case 2:
                    w->rot[0] = PI / 2;
                    if (!soundInFovCheck(gGameWork.view, w->frame->pos[0], w->frame->pos[1] - 50.0f,
                                         w->frame->pos[2])) {
                        w->sound = tskDestroy(w->sound);
                    }
                    if ((w->count & 7) == 1) {
                        mathXZRotateY(w->rot[1], 0.0f, 20.0f, &dx, &dz);
                        kamaNettouMake(obj, w->frame->pos[0] + dx, w->frame->pos[1],
                                               w->frame->pos[2] + dz);
                    }
                    if (w->count > 30) {
                        w->sound = tskDestroy(w->sound);
                        w->count = 0;
                        w->mode = 3;
                    }
                    break;
                case 3:
                    w->rot[0] -= 0.05235987902f;
                    if (w->count > 30) {
                        w->count = 0;
                        w->mode = 0;
                    }
                    break;
            }
            eneSetModelOnCourse(obj, &w->frame->f);
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
