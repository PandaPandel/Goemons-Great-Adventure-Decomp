#include "tlb.h"

/* A "piece": a bit of debris thrown up, falling, bouncing and fading (gimPieceProc). */
typedef struct PieceWork {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ f32 pos[3];
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *frame;
    /* 0x38 */ s32 alpha;
    /* 0x3C */ s32 fade;
    /* 0x40 */ s32 puff; /* frames to the next dark puff */
    /* 0x44 */ s32 life;
} PieceWork;

#define PIECE(obj) ((PieceWork *) FX(obj))

/* The boards ("nboard", "fboard"): an enemy's work (EneObj) to 0x34, then their own. */
typedef struct BoardWork {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad9[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *frame;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ f32 accel;
    /* 0x40 */ s32 alpha;
    /* 0x44 */ s32 fade;
    /* 0x48 */ u8 fall; /* 0 still, 1 stood on, 2 falling, 3 landed and fading */
    /* 0x4A */ u16 timer;
} BoardWork;

#define BOARD(obj) ((BoardWork *) FX(obj))

/* "elevator": an enemy's work (EneObj) to 0x34, then its own. */
typedef struct ElevWork {
    /* 0x00 */ u8 unused0;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad9[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *frame;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ u8 state; /* 0 down, 1 boarding, 2 rising, 3 up, 4 leaving, 5 sinking */
    /* 0x3D */ u8 riders; /* players on it this step (message 6) */
    /* 0x3E */ u8 quiet; /* no actpan sound */
    /* 0x40 */ f32 base;
    /* 0x44 */ f32 offset; /* above base, 0 to height */
    /* 0x48 */ f32 speed;
    /* 0x4C */ f32 height;
} ElevWork;

#define ELEV(obj) ((ElevWork *) FX(obj))

/* "ppress": an enemy's work (EneObj) to 0x34, then its own. */
typedef struct PressWork {
    /* 0x00 */ u8 unused0;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad9[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *frame;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ ACollFrame *atk;
    /* 0x40 */ PolFrame *shadow;
    /* 0x44 */ f32 base;
    /* 0x48 */ f32 offset; /* above base, 0 to height */
    /* 0x4C */ f32 height;
    /* 0x50 */ u8 state; /* 0 rising, 1 up, 2 dropping, 3 down */
    /* 0x52 */ s16 timer;
    /* 0x54 */ s16 wait; /* 0: always drawn; else drawn only while a player is below base + 50 + height + this */
} PressWork;

#define PRESS(obj) ((PressWork *) FX(obj))

/* "Hitring" (this file's): a frame and a fade. */
typedef struct RingWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad4[0xC];
    /* 0x10 */ s32 alpha;
} RingWork;

#define RING(obj) ((RingWork *) FX(obj))

typedef struct RingArg {
    /* 0x00 */ s32 unused0;
    /* 0x04 */ f32 pos[3];
} RingArg;

/* "ipunch": an enemy's work (EneObj) to 0x34, then its own. */
typedef struct PunchWork {
    /* 0x00 */ u8 unused0;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad9[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ AnimeFrame *model;
    /* 0x38 */ u8 state; /* 0 punching, 1 out, 2 drawing back, 3 in */
    /* 0x3A */ s16 timer;
    /* 0x3C */ PolFrame *shadow;
    /* 0x40 */ ACollFrame *atk;
} PunchWork;

#define PUNCH(obj) ((PunchWork *) FX(obj))

/* "hkoban": a thrown coin. */
typedef struct KobanWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ ACollFrame *atk;
    /* 0x08 */ f32 vel[3];
    /* 0x14 */ u16 timer;
} KobanWork;

#define KOBAN(obj) ((KobanWork *) FX(obj))

/* "ihead": an enemy's work (EneObj) to 0x34, then its own. */
typedef struct HeadWork {
    /* 0x00 */ u8 unused0;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad9[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ LightFrame *light;
    /* 0x38 */ AnimeFrame *model;
    /* 0x3C */ PolFrame *target; /* at the camera's target: where it throws */
    /* 0x40 */ u16 timer;
    /* 0x42 */ u8 side; /* the next coin from the left (0) or the right */
    /* 0x43 */ u8 fixed; /* placed turned: it neither turns nor throws */
} HeadWork;

#define HEAD(obj) ((HeadWork *) FX(obj))

/* "ibody": an enemy's work (EneObj) to 0x34, then its own. */
typedef struct BodyWork {
    /* 0x00 */ u8 unused0;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad9[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ AnimeFrame *model;
    /* 0x38 */ u8 pad38[2];
    /* 0x3A */ u8 state; /* 0 walking out, 1 turning, 2 walking back, 3 turning */
    /* 0x3C */ s32 last; /* its animation's last frame */
    /* 0x40 */ f32 far; /* the camera's far distance while it is out of view */
} BodyWork;

#define BODY(obj) ((BodyWork *) FX(obj))

/* "edopole": an enemy's work (EneObj) to 0x34, then its own. */
typedef struct PoleWork {
    /* 0x00 */ u8 unused0;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad9[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *frame;
    /* 0x38 */ ACollFrame *atk[4];
} PoleWork;

#define POLE(obj) ((PoleWork *) FX(obj))

/* "gareki": an enemy's work (EneObj) to 0x34, then its own. */
typedef struct GarekiWork {
    /* 0x00 */ u8 unused0;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad9[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ AnimeFrame *model;
    /* 0x38 */ AnimeFrame *shadow; /* a copy of the model, flattened on the ground */
    /* 0x3C */ ACollFrame *atk;
    /* 0x40 */ ACollFrame *dmg;
    /* 0x44 */ f32 accel;
    /* 0x48 */ s32 alpha;
    /* 0x4C */ s32 fade;
    /* 0x50 */ u8 state; /* 0 shaking, 1 falling, 2 crumbling (an animation), 3 fading, 4 broken */
    /* 0x54 */ s32 timer;
} GarekiWork;

#define GAREKI(obj) ((GarekiWork *) FX(obj))

/* "fireball": a flame. */
typedef struct FireWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ Anim anim; /* its texture */
    /* 0x14 */ s32 alpha;
    /* 0x18 */ ACollFrame *atk;
    /* 0x1C */ s32 life; /* steps its attack box lasts */
} FireWork;

#define FIRE(obj) ((FireWork *) FX(obj))

/* "firepoint": an enemy's work (EneObj) to 0x34, then its own. */
typedef struct FirePointWork {
    /* 0x00 */ u8 unused0;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad9[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *frame;
    /* 0x38 */ Anim anim;
    /* 0x48 */ u8 state; /* 0 big fireballs, 1 small ones */
    /* 0x4C */ u32 timer;
    /* 0x50 */ f32 size; /* the height its scale swells to */
} FirePointWork;

#define FIREPOINT(obj) ((FirePointWork *) FX(obj))

/* "firewall": an enemy's work (EneObj) to 0x34, then its own. */
typedef struct FireWallWork {
    /* 0x00 */ u8 unused0;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad9[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ Anim anim;
    /* 0x44 */ u16 tick;
    /* 0x48 */ PolFrame *fires[5]; /* each shows the animation 7 frames after the one before */
} FireWallWork;

#define FIREWALL(obj) ((FireWallWork *) FX(obj))

/* This file passes its display list to fadeSegToRam as a pointer: as an integer, IDO hoists the
 * address into a saved register in the firewalls' loops (a scratch test; the game builds it in a0
 * at each call). */
#define fadeSegToRamP ((u32 (*)(Gfx *, u32)) fadeSegToRam)

/* plCountPlayers returns tskCountChildren's count (its definition has no return statement). */
#define plCountPlayersN ((s32 (*)(void)) plCountPlayers)

/* acollAtkAddYZ and acollDmgAddXYZ return the box they made (their definitions have no return
 * statement). */
#define acollAtkAddYZN ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define acollDmgAddXYZN ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s16, s32)) acollDmgAddXYZ)

/* Message 6's argument (plmove.c's PlPlace): a place on the course, moved by the object. */
typedef struct PlPlace {
    /* 0x0 */ s16 zone;
    /* 0x4 */ f32 place;
    /* 0x8 */ f32 side;
    /* 0xC */ f32 height;
} PlPlace;

extern Frame *gFrmLayer3;
extern Gfx gimPieceDl[];
extern Gfx gimPieceThrowBigDl[];
extern Gfx gimNboardDl[];
extern Gfx gimFboardDl[];
extern u8 gimBoardRide[];
extern Gfx gimElevatorDl[];
extern u8 gimElevatorRide[];
extern Gfx gimPpressDl[];
extern u8 gimPpressRide[];
extern Gfx gimPpressShadowDl[];
extern Gfx gimHitringDl[];
extern u8 gimIpunchAnim[];
extern Gfx gimIpunchShadowDl[];
extern u8 gimIpunchAtkChain[];
extern Gfx gimHkobanDl[];
extern u8 gimIheadAnim[];
extern u8 gimIbodyAnim[];
extern Gfx gimEdopoleDl[];
extern u8 gimGarekiAnim[];
extern u8 gimGarekiAtkChain[];
extern Gfx gimFireballDl[];
extern Gfx gimFireModelDl[];
Gfx gimFireDl[] = {
    gsDPPipeSync(),
    gsDPSetCombineLERP(PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, ENVIRONMENT, 0, TEXEL0, 0, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, ENVIRONMENT, 0, TEXEL0, 0),
    gsDPSetPrimColor(0, 0, 0xFF, 0xFF, 0x80, 0x00),
    gsDPSetEnvColor(0xFF, 0x00, 0x00, 0xFF),
    gsSPEndDisplayList(),
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_CULL_BACK),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetCombineLERP(0, 0, 0, TEXEL0, ENVIRONMENT, 0, TEXEL0, 0, 0, 0, 0, TEXEL0, ENVIRONMENT, 0, TEXEL0, 0),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsSPEndDisplayList(),
    gsDPNoOp(),
}; /* the firepoint's display list: its colours */

/* libultra's, through its address (code at 0x08000000 cannot jal it). */
#define guNormalize (*guNormalize)

f32 gimFirstPlayerHeight(void);
s32 gimHitringProc(Obj *obj, s16 msg, void *arg);

s32 gimPieceProc(Obj *obj, s16 msg, void *arg);
Obj *gimPieceThrow(Obj *parent, f32 x, f32 y, f32 z);

/* The height of the first player in play (0 if none). */
f32 gimFirstPlayerHeight(void) {
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

/* "piece" */
s32 gimPieceProc(Obj *obj, s16 msg, void *arg) {
    PieceWork *w = PIECE(obj);

    switch (msg) {
        case 2:
            obj->name = "piece";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = gimPieceDl;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 0.01999999955f;
            floadGetSetRam((void **) w->frame->segs, 0x57);
            w->rot[1] = fadeRand(360) * PI / 180.0f;
            w->alpha = 255;
            w->life = 40;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->life--;
            if (w->life < 0) {
                w->fade = -16;
            } else if (w->life < -20) {
                return -1;
            }
            w->vel[1] += -0.4;
            if (eneLand((EneObj *) w, 2.0f)) {
                w->vel[1] = -w->vel[1] / 4;
                w->fade = -8;
            }
            w->pos[0] += w->vel[0];
            w->pos[1] += w->vel[1];
            w->pos[2] += w->vel[2];
            w->alpha += w->fade;
            if (w->alpha <= 0) {
                return -1;
            }
            eneSetModelOnCourse(obj, &w->frame->f);
            if (w->puff > 0 && w->puff < 10 && (w->puff & 3) == 1) {
                itexPuffDark(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], w->frame->scale[1]);
            }
            w->puff--;
            w->rot[0] += 0.1745329201f;
            w->rot[1] += 0.08726646006f;
            w->rot[2] += 0.05235987902f;
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

/* A piece near (x, y, z), puffing at once. */
void gimPieceMakeNear(Obj *parent, f32 x, f32 y, f32 z) {
    Obj *o;
    s32 pad;
    PieceWork *w;

    if ((o = tskCreate(parent, gimPieceProc, parent->file, NULL)) != NULL) {
        w = PIECE(o);
        w->pos[0] = fadeRand(20) + x - 10.0f;
        w->pos[1] = y;
        w->pos[2] = fadeRand(20) + z - 10.0f;
        w->puff = 20;
    }
}

/* A piece thrown up from (x, y, z). */
Obj *gimPieceThrow(Obj *parent, f32 x, f32 y, f32 z) {
    Obj *o;
    s32 pad;
    PieceWork *w;

    if ((o = tskCreate(parent, gimPieceProc, parent->file, NULL)) != NULL) {
        PIECE(o)->pos[0] = x;
        PIECE(o)->pos[1] = y;
        PIECE(o)->pos[2] = z;
        w = PIECE(o);
        w->vel[0] = fadeRand(40) / 10.0f - 2.0f;
        w->vel[2] = fadeRand(40) / 10.0f - 2.0f;
        w->vel[1] = 4.0f;
        w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 0.0700000003f;
        w->puff = 1;
    }
    return o;
}

/* A bigger piece of another model, thrown away from the camera side. */
void gimPieceThrowBig(Obj *parent, f32 x, f32 y, f32 z) {
    Obj *o;

    if ((o = gimPieceThrow(parent, x, y, z)) != NULL) {
        PIECE(o)->frame->dl2 = gimPieceThrowBigDl;
        PIECE(o)->frame->scale[0] = PIECE(o)->frame->scale[1] = PIECE(o)->frame->scale[2] = 0.1000000015f;
        PIECE(o)->vel[2] = fabsf(PIECE(o)->vel[2]);
    }
}

/* "nboard": a board to stand on. */
s32 gimNboardProc(Obj *obj, s16 msg, void *arg) {
    BoardWork *w = BOARD(obj);
    s32 angle;

    switch (msg) {
        case 2:
            obj->name = "nboard";
            angle = eneSetup(obj, arg);
            if (w->zone == 0) {
                w->rot[1] = angle * PI / 512;
            } else {
                w->rot[1] = vramCourseHeading(w->zone, w->x);
            }
            if ((w->frame = eneNewPol(obj, gimNboardDl)) == NULL) {
                return -1;
            }
            frmMove(&w->frame->f, gFrmLayer3);
            w->frame->rot[1] = w->rot[1];
            if ((w->ride = rideCreate(gRideTree1, (s32) obj, &w->frame->f,
                                      (RideData *) eneFileAddr(obj, gimBoardRide, 0))) == NULL) {
                return -1;
            }
            break;
        case 1:
            frmDestroy(&w->ride->f);
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "fboard": a board that falls when stood on, then fades back in. The ride data is read into
 * a local first, as in tlb_0053's hasu: passed inline to rideCreate it costs a compiler
 * temporary, which put plIsInPlay's spilled address one word low (0x34 for the game's 0x38). */
s32 gimFboardProc(Obj *obj, s16 msg, void *arg) {
    BoardWork *w = BOARD(obj);
    PolFrame *f;
    s32 i;
    Vec3f at;
    f32 t;
    s32 angle;
    RideData *data;
    s32 zone;
    Vec3f v;

    switch (msg) {
        case 2:
            obj->name = "fboard";
            angle = eneSetup(obj, arg);
            if (w->zone == 0) {
                w->rot[1] = angle * PI / 512;
            } else {
                w->rot[1] = vramCourseHeading(w->zone, w->x);
            }
            w->alpha = 255;
            w->accel = 0.0f;
            if ((w->frame = eneNewPol(obj, gimFboardDl)) == NULL) {
                return -1;
            }
            w->frame->rot[1] = w->rot[1];
            data = (RideData *) eneFileAddr(obj, gimBoardRide, 0);
            if ((w->ride = rideCreate(gRideTree1, (s32) obj, &w->frame->f, data)) == NULL) {
                return -1;
            }
            break;
        case 1:
            frmDestroy(&w->ride->f);
            frmDestroy(&w->frame->f);
            break;
        case 6:
            for (i = 0; i < 4; i++) {
                if (plIsInPlay(i)) {
                    plAskPos(gGameWork.players[i], &v.x, &v.y, &v.z, &zone);
                    if (fabsf(((PlPlace *) arg)->side - v.z) < 1.0f) {
                        plGetSpeed(gGameWork.players[i], &v);
                        if (sqrtf(v.x * v.x + v.y * v.y + v.z * v.z) > 2.0f) {
                            if (w->fall == 0) {
                                w->timer = 0;
                                w->fall = 1;
                            }
                        }
                    }
                }
            }
            ((PlPlace *) arg)->height += w->vel[1];
            break;
        case 0:
            w->timer++;
            w->alpha += w->fade;
            if (w->alpha < 0) {
                w->alpha = 0;
            } else if (w->alpha > 255) {
                w->alpha = 255;
            }
            if (w->alpha == 255) {
                f = w->frame;
                if (f->f.parent != gFrmLayers[1]) {
                    frmMove(&f->f, gFrmLayers[1]);
                }
            } else {
                f = w->frame;
                if (f->f.parent != gFrmLayers[8]) {
                    frmMove(&f->f, gFrmLayers[8]);
                }
            }
            switch (w->fall) {
                case 0:
                    break;
                case 1:
                    if (w->timer > 10) {
                        w->accel = -0.3000000119f;
                        w->timer = 0;
                        w->fall = 2;
                    }
                    break;
                case 2:
                    f = w->frame;
                    w->vel[1] += w->accel;
                    f->pos[1] += w->vel[1];
                    if (vramRayGround((Vec3f *) w->frame->pos, (Vec3f *) w->vel, &at, &t)) {
                        w->vel[1] = 0.0f;
                        frmMove(&w->frame->f, gFrmLayers[8]);
                        w->fade = -8;
                        w->fall = 3;
                    }
                    if (w->timer > 30) {
                        w->fade = -8;
                        w->fall = 3;
                    }
                    break;
                case 3:
                    if (w->alpha <= 0) {
                        w->alpha = 0;
                        w->fade = 8;
                        w->vel[1] = 0.0f;
                        w->frame->pos[1] = w->y;
                        w->timer = 0;
                        w->fall = 0;
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

/* "fboard" (the row): eight falling boards (kind 0x47) 13 apart across its place. */
s32 gimFboardRowProc(Obj *obj, s16 msg, void *arg) {
    EneObj *e = ENE(obj);
    s32 i;
    s32 angle;
    f32 s;
    f32 c;
    f32 d;
    f32 x;
    f32 z;
    s32 pad[2];

    if (msg == 2) {
        obj->name = "fboard";
        angle = eneSetup(obj, arg);
        e->rot[1] = angle * PI / 512;
        s = mathSinCos(angle, &c);
        vramCoursePointXZ(e->zone, e->x, e->z, &x, &z);
        for (i = 0; i < 104; i += 13) {
            d = i + -45.5;
            eneMakeEx(obj, ENE_GIM_FBOARD, 0, s * d + x, e->y, c * d + z, 2, (u32 (*)(void *, void *, void *)) angle);
        }
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "elevator": rises while all the players stand on it, comes down when they get off. */
s32 gimElevatorProc(Obj *obj, s16 msg, void *arg) {
    ElevWork *w = ELEV(obj);
    Obj *s;
    s32 pad;
    s32 c;
    s32 pad2[2];

    switch (msg) {
        case 2:
            obj->name = "elevator";
            c = eneSetup(obj, arg);
            w->frame = eneNewPol(obj, gimElevatorDl);
            if (w->frame == NULL) {
                return -1;
            }
            w->ride = rideCreate(gRideTree1, (s32) obj, &w->frame->f,
                                 (RideData *) eneFileAddr(obj, gimElevatorRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            w->base = w->y;
            w->height = c & 0xFFFF;
            if (w->base + w->height / 2 < gimFirstPlayerHeight()) {
                w->state = 4;
                w->offset = w->height;
            } else {
                w->state = 1;
            }
            w->offset += (s16) (c >> 16);
            if (w->offset < 0.0f) {
                w->offset = 0.0f;
            } else if (w->height < w->offset) {
                w->offset = w->height;
            }
            if (c >> 16) {
                w->quiet = 0;
            } else {
                w->quiet = 1;
            }
            break;
        case 1:
            frmDestroy(&w->ride->f);
            frmDestroy(&w->frame->f);
            break;
        case 6:
            w->riders++;
            break;
        case 4:
            *(Vec3f *) arg = *(Vec3f *) w->frame->pos;
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->speed = 0.0f;
                    if (plCountPlayersN() != w->riders) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    w->speed = 0.0f;
                    if (plCountPlayersN() == w->riders) {
                        w->state = 2;
                    }
                    if (w->base + w->height - 10.0f < gimFirstPlayerHeight()) {
                        w->state = 2;
                    }
                    if (w->state == 2) {
                        tskDestroyChildren(obj);
                        if ((s = soundActpanCreateCam(obj, 0x12A)) != NULL && w->quiet == 0) {
                            soundActpanFlagSet(s, 0);
                        }
                    }
                    break;
                case 2:
                    w->speed = 1.0f;
                    if (w->height < w->offset) {
                        w->state = 3;
                        if (gimFirstPlayerHeight() < w->base + 10.0f) {
                            tskDestroyChildren(obj);
                            if ((s = soundActpanCreateCam(obj, 0x812A)) != NULL && w->quiet == 0) {
                                soundActpanFlagSet(s, 0);
                            }
                            w->state = 5;
                        } else {
                            SoundLocation2(0x12B, gGameWork.view, w->frame->pos[0], w->frame->pos[1],
                                           w->frame->pos[2], 180.0f, 800.0f, 1.570796371f);
                        }
                    }
                    break;
                case 3:
                    w->speed = 0.0f;
                    if (plCountPlayersN() != w->riders) {
                        w->state = 4;
                    }
                    break;
                case 4:
                    w->speed = 0.0f;
                    if (plCountPlayersN() == w->riders) {
                        w->state = 5;
                    }
                    if (gimFirstPlayerHeight() < w->base + 10.0f) {
                        w->state = 5;
                    }
                    if (w->state == 5) {
                        tskDestroyChildren(obj);
                        if ((s = soundActpanCreateCam(obj, 0x12A)) != NULL && w->quiet == 0) {
                            soundActpanFlagSet(s, 0);
                        }
                    }
                    break;
                case 5:
                    w->speed = -1.0f;
                    if (w->offset < 0.0f) {
                        w->state = 0;
                        if (w->base + w->height - 10.0f < gimFirstPlayerHeight()) {
                            tskDestroyChildren(obj);
                            if ((s = soundActpanCreateCam(obj, 0x812A)) != NULL && w->quiet == 0) {
                                soundActpanFlagSet(s, 0);
                            }
                            w->state = 2;
                        } else {
                            SoundLocation2(0x12B, gGameWork.view, w->frame->pos[0], w->frame->pos[1],
                                           w->frame->pos[2], 180.0f, 800.0f, 1.570796371f);
                        }
                    }
                    break;
            }
            w->offset += w->speed;
            w->riders = 0;
            w->y = w->base + w->offset;
            eneSetModelOnCourse(obj, &w->frame->f);
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Whether the press, falling, meets a ride under (x, y, z); if so it stops on it. */
s32 gimPressRideStop(Obj *obj, PressWork *w, f32 x, f32 y, f32 z) {
    Vec3f p;
    Vec3f d;
    Vec3f out;

    p.x = x;
    p.y = y + 10.0f;
    p.z = z;
    d.x = 0.0f;
    d.y = w->vel[1] + -10.0f;
    d.z = 0.0f;
    if (rideMoveStop(&p, &d, &out, (s32) obj)) {
        w->offset = y + 10.0f + out.y - w->base;
        return 1;
    }
    return 0;
}

/* "ppress": a press that rises slowly and drops, crushing and shaking the screen. */
s32 gimPpressProc(Obj *obj, s16 msg, void *arg) {
    PressWork *w = PRESS(obj);
    Obj *o;
    s32 c;
    s32 hit;
    f32 k;
    f32 s;
    f32 cs;
    f32 sc;
    CamView view;
    s32 i;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "ppress";
            c = eneSetup(obj, arg);
            w->frame = eneNewPol(obj, gimPpressDl);
            if (w->frame == NULL) {
                return -1;
            }
            w->ride = rideCreate(gRideTree1, (s32) obj, &w->frame->f,
                                 (RideData *) eneFileAddr(obj, gimPpressRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            w->atk = acollAtkAddYZN(&ene_atk_root_frame->frame, &w->frame->f, 180, 220, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x7D;
            w->atk->flags |= 0x10;
            w->shadow = eneNewPol(obj, gimPpressShadowDl);
            if (w->shadow == NULL) {
                return -1;
            }
            frmMove(&w->shadow->f, gFrmLayer4);
            w->shadow->dl = gShadowSetupDl;
            w->shadow->rot[0] = -1.570796371f;
            w->shadow->scale[0] = w->shadow->scale[1] = w->shadow->scale[2] = 1.0f;
            w->height = (c >> 8) & 0xFF;
            w->rot[1] = (c >> 16) * PI / 512;
            w->base = w->y;
            w->wait = c & 0x7F;
            if (c & 0x80) {
                w->offset = w->height;
                w->timer = 0;
                w->state = 1;
            } else {
                w->state = 0;
            }
            break;
        case 1:
            frmDestroy(&w->atk->frame);
            frmDestroy(&w->ride->f);
            frmDestroy(&w->frame->f);
            frmDestroy(&w->shadow->f);
            break;
        case 6:
            ((PlPlace *) arg)->height += w->vel[1];
            break;
        case 0:
            w->timer++;
            if (w->wait != 0) {
                if (w->base + 50.0f + w->height + w->wait < gimFirstPlayerHeight()) {
                    w->frame->f.flags &= ~2;
                } else {
                    w->frame->f.flags |= 2;
                }
            }
            switch (w->state) {
                case 0:
                    w->atk->flags &= ~2;
                    w->offset += 0.6000000238f;
                    if (w->timer % 18 == 1) {
                        if (sndGetVolume(0x294, 0) == 0) {
                            soundIncamCam(0x294, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2]);
                        } else if (sndGetVolume(0x295, 0) == 0) {
                            soundIncamCam(0x295, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2]);
                        }
                    }
                    if (w->height < w->offset) {
                        w->state = 1;
                        w->timer = 0;
                    }
                    break;
                case 1:
                    if (w->timer > 30) {
                        w->state = 2;
                    }
                    break;
                case 2:
                    w->atk->flags |= 2;
                    w->vel[1] += -0.4;
                    s = mathSinCos((s16) (w->frame->rot[1] / PI * 512), &cs);
                    if (w->offset + ((Vec3f *) w->vel)->y <= 0.0f) {
                        w->offset = 0.0f;
                        hit = 1;
                        itexPuffDark(w->frame->pos[0] + 25.0f * cs, w->frame->pos[1],
                                     w->frame->pos[2] - 25.0f * s, 0.15f);
                        itexPuffDark(w->frame->pos[0] - 25.0f * cs, w->frame->pos[1],
                                     w->frame->pos[2] + 25.0f * s, 0.15f);
                    } else {
                        hit = gimPressRideStop(obj, w, w->frame->pos[0] + 8.0f * cs, w->frame->pos[1],
                                                     w->frame->pos[2] - 8.0f * s);
                        if (hit == 0) {
                            hit = gimPressRideStop(obj, w, w->frame->pos[0] - 8.0f * cs, w->frame->pos[1],
                                                         w->frame->pos[2] + 8.0f * s);
                        }
                        if (hit == 0) {
                            hit = gimPressRideStop(obj, w, w->frame->pos[0] + 25.0f * cs, w->frame->pos[1],
                                                         w->frame->pos[2] - 25.0f * s);
                        }
                        if (hit == 0) {
                            hit = gimPressRideStop(obj, w, w->frame->pos[0] - 25.0f * cs, w->frame->pos[1],
                                                         w->frame->pos[2] + 25.0f * s);
                        }
                    }
                    if (hit) {
                        w->state = 3;
                        w->timer = 0;
                        w->vel[1] = 0.0f;
                        view = *gGameWork.view;
                        view.fovy *= 1.2000000477f;
                        if (SoundIncam2(0x296, &view, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                        view.zNear, 300.0f)) {
                            for (i = 0; i < 4; i++) {
                                if (gGameWork.players[i] != NULL) {
                                    contPakRumbleStart(i, 0xAAAAAAAA, 15);
                                }
                            }
                        }
                    }
                    break;
                case 3:
                    if (w->timer > 30) {
                        w->state = 0;
                    }
                    break;
            }
            w->offset += w->vel[1];
            w->y = w->base + w->offset;
            k = 1.0f - w->offset / 100;
            if (k < 0.0f) {
                w->shadow->f.flags &= ~2;
            } else {
                w->shadow->f.flags |= 2;
                sc = k * 0.6000000238f;
                w->shadow->scale[2] = sc;
                w->shadow->scale[1] = sc;
                w->shadow->scale[0] = sc;
            }
            w->shadow->rot[1] = w->frame->rot[1];
            if (w->atk != NULL && acollAtkWasHit(w->atk) == 1 && (o = (Obj *) w->atk->hitPower) != NULL) {
                tskSendMsg(o, 7, obj);
            }
            eneSetModelOnCourse(obj, &w->frame->f);
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Hitring" (this file's): a flat ring at a point, growing as it fades. */
s32 gimHitringProc(Obj *obj, s16 msg, void *arg) {
    RingWork *w = RING(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Hitring";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = gimHitringDl;
            floadGetSetRam((void **) w->frame->segs, 0x120);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 0.1000000015f;
            w->frame->pos[0] = ((RingArg *) arg)->pos[0];
            w->frame->pos[1] = ((RingArg *) arg)->pos[1];
            w->frame->pos[2] = ((RingArg *) arg)->pos[2];
            w->frame->rot[0] = -1.208815098f;
            w->alpha = 160;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->alpha -= 7;
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            w->frame->scale[0] += 0.1;
            w->frame->scale[1] += 0.1;
            w->frame->scale[2] += 0.1;
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombine(gGfxDlHead++, 0xFF9BFF, 0xFF36FF7F);
            gDPSetEnvColor(gGfxDlHead++, 255, 255, 255, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A Hitring at (x, y, z). */
void gimHitringMake(Obj *parent, f32 x, f32 y, f32 z) {
    s32 pad;
    RingArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    tskCreate(parent, gimHitringProc, parent->file, &arg);
}

/* "ipunch": a fist that punches down out of the wall (an animation), with a Hitring, pieces, a
 * sound and a rumble when it lands, then draws back. */
/* 239 lines (mix 3), frame and slots right, the pieces' loop "slti 30" as the game's (one index
 * for both loops, so IDO cannot turn the test into "bne"): the game keeps &view in s1 across its
 * two SoundIncam2 calls (the copy's destination too); here IDO rebuilds it each time, so one
 * saved register fewer and every s-register is one off (w s2 for s1, obj s3 for s2). Measured:
 * with a second index j, all 241: "30 > i", a do-while, "i < 3" with i * 10; a "CamView *v =
 * &view" local (folded, 249); &view passed as a u32 through a cast SoundIncam2 (305).
 * 2026-10-12 (real 100): in the game s1 is &view (copy, both calls) and then the rumble
 * loop's index, s0 the first loop's index, SoundIncam2's address and the rumble loop's walker
 * (gGameWork + 4i, a saved register); here the walker is spilled around contPakRumbleStart
 * (v1, sp+0x50: the extra temp word) and &view is rebuilt for the second call. Measured: the
 * rumble loop on its own index (pad): walker still spilled, 102; "v = &view; *v = *src" with v
 * in the calls: folded, 100; "v = gGameWork.view; view = *v; ...; v = &view": 114; a FAKE
 * "if (!&view) {}" between the calls: 127.
 * 2026-10-12 (second try): the rumble loop with a walker "Obj **pp" (pad's word) beside i: 100, pp
 * gets a home (0x60) and is spilled the same way, no saved register.
 * 2026-10-12 (third try), match_function.py trace (100): uopt's ranges here: i one range over both
 *   loops (r14 = s0, blocks 30-32 and 37-40), w s1, obj s2, 10.0f f20, the walker (uixa) a
 *   caller-saved v0 range spilled around contPakRumbleStart; &view is no uopt range at all
 *   (ugen forms sp+0x68 in a1 for the copy and the first call and again for the second).
 *   The game's s-registers: s0 = the first loop's i, then SoundIncam2's address, then the
 *   walker; s1 = &view (copy and both calls), then the rumble loop's index; s2 w, s3 obj. So
 *   the game's uopt made &view a range (coloured s1) and the two loops' indices two ranges.
 * 2026-10-16 (force targets), read from uopt-spill's COLOR/COST lines (force none):
 *   - The 11 is the rumble loop only: the game's index s1 and walker s0; here i s0 (one range
 *     with the first loop, 20.5) and the walker (uixa, 15.5) in v1, spilled: its v-register
 *     cost is 10 (contPakRumbleStart), s0 is forbidden and an unused s-register costs more.
 *   - Rumble loop on its own index (pad): j 20.5 is coloured before the walker 15.5 (j's
 *     refs 1 + 10 + 30, the walker's 1 + 10 + 20, the same live blocks), so j takes s0 again.
 *   - "j = 0; do {} while (0); for (; j < 4; ...)" (FAKE block: j's count 2 to 3, 14.0) with
 *     "if (!&view) {}" before the first SoundIncam2 (no FAKE in the loop): 28, and every s-
 *     register is the game's (walker s0, j s1, &view s1, address s0, w s2, obj s3). Left: the
 *     pieces loop's 30 is now a range (prio 5.0) coloured after j, so it takes s1 (s1 used:
 *     cost 0) and the test becomes "bne s0, s1"; with s1 unused (the shared-i form) the 30
 *     is split (5 x 2 <= the unused s1's cost) and ugen emits the game's slti. And the walker
 *     starts "sll t6, zero, 2" (j = 0 in another block), and t-registers shift.
 *   - The constant needs s1 unused when it is coloured, j before w (4.93): j in (4.93, 5.0),
 *     no integer count gives it. "i <= 29": 28 (the same); "if (i) {}" after the pieces
 *     loop (i live after it): 28, the 30 still a range.
 *   - "if (!&view) {}" after the second call with j: 102 (folded, no &view range); before the
 *     first call: 103 (&view 1.5, after the address, takes a1: s1 unused); "if (!&sc &&
 *     ...)" in the rumble loop: 123 (j and walker both count 3, j still first); shared i,
 *     "i = 0; do {} while (0);" before the rumble loop: 113 (i 20.75, s0); seven do-whiles:
 *     185 (i 11.86, after the walker, which takes v1 with s0 unused).
 *   Not found: a form where the 30 is coloured with s1 unused while j still beats w.
 * 2026-10-16 (evidence push): check MATCH (except own section addresses), from 11. Read
 *   in IDA first (decomp:080021A8 and the disassembly), then in uopt's source (n64decomp/ido
 *   uoptloop.c determine_if_unrollable, find_increment_expr; uoptreg2.c compute_save):
 *   - The pieces loop keeps "slti 30" only if its index has a second, non-increment store in
 *     the blocks after the loop, up to the rumble loop's header. find_increment_expr scans the
 *     loop's whole interval and fails on that store, so there is no EQ_INEQ: no "!= 30" and no
 *     30 range. One index for both loops did this before. Here "i = SoundIncam2(0x296, ..)"
 *     does it; the store is dead after the test, so uopt drops it and i stays s0.
 *   - The rumble loop has its own index, port (the game's s1). port must be coloured after
 *     w->model's range (14.6), or that range takes s1 (force 115->s1 119->s0: 43). It must
 *     also come after the walker (15.5), or port takes s0. "port = 0;" before the "if" puts
 *     its def in the test's block. That is a sixth live block, so its count is 3: 13.67.
 *   - A do-while, not a for. A for with no init leaves its own first-test block between
 *     port's def and the walker's start, and the walker starts as "sll t6,zero,2; addu" (25).
 *   - &view then takes s1 by itself (s0 is the address's, s1 is port's, which it does not
 *     meet). The old "if (!&view && ..)" FAKE is gone.
 *   - Measured on the way:
 *     - own index, natural for: 102;
 *     - plus a dead "i = 0" after the pieces loop: 100 (slti back, port still s0);
 *     - "continue" body: 100 (port and the walker both count 3);
 *     - "pad = SoundIncam2(..); if (pad)": the dead store is dropped, 100;
 *     - "if (port) {}" after the loop: the read lands on the walker's range, not port's, 100;
 *     - "i = SoundIncam2(..); port = 0; if (i) for (; port < 4; ..)": 25 (sll);
 *     - this do-while with "port = 0" inside the "if": 100.
 *   - uopt.c 153972-154031: an unused callee-saved register costs f24 = blocks / 4, clamped
 *     to 4..60 (0x1001c364 is the block counter). That fits the picks: the walker kept v1 at
 *     cost 10, and w took an unused s1 against 51. */
s32 gimIpunchProc(Obj *obj, s16 msg, void *arg) {
    Obj *o;
    s32 i;
    PunchWork *w = PUNCH(obj);
    s32 last;
    CamView view;
    f32 sc;
    s32 port;

    switch (msg) {
        case 2:
            obj->name = "ipunch";
            eneSetup(obj, arg);
            w->model = eneNewAnime(obj, (u32) gimIpunchAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 1.0f;
            w->shadow = (PolFrame *) frmCreate(gFrmLayer4, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            w->shadow->dl = gShadowSetupDl;
            w->shadow->dl2 = gimIpunchShadowDl;
            floadGetSetRam((void **) w->shadow->segs, 0x120);
            w->shadow->scale[0] = w->shadow->scale[1] = w->shadow->scale[2] = 0.1000000015f;
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[1] = w->model->pos[1];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->rot[0] = -1.208815098f;
            w->atk = aniAtkCreate(&ene_atk_root_frame->frame, &w->model->f, (u32) gimIpunchAtkChain, eneGetFile(obj, 0), 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x78;
            break;
        case 1:
            if (w->atk != NULL) {
                aniAtkDelete(w->atk);
            }
            frmDestroy(&w->shadow->f);
            frmDestroy(&w->model->f);
            break;
        case 0:
            last = polAnimeLastFrame(w->model);
            switch (w->state) {
                case 0:
                    w->model->frame += 0.1000000015f;
                    if (w->timer == 40) {
                        gimHitringMake(obj, w->model->pos[0], w->model->pos[1] + 2.0f, w->model->pos[2]);
                        for (i = 0; i < 30; i += 10) {
                            gimPieceThrowBig(obj, w->model->pos[0] - 10.0f + i, w->model->pos[1] + 2.0f,
                                                   w->model->pos[2] + 10.0f);
                        }
                        view = *gGameWork.view;
                        view.fovy = 0.7853981853f;
                        SoundIncam2(0x2DD, &view, w->model->pos[0], w->model->pos[1], w->model->pos[2], view.zNear,
                                    view.zFar);
                        i = SoundIncam2(0x296, &view, w->model->pos[0], w->model->pos[1], w->model->pos[2],
                                        view.zNear, view.zFar);
                        port = 0;
                        if (i) {
                            do {
                                if (gGameWork.players[port] != NULL) {
                                    contPakRumbleStart(port, 0xAAAAAAAA, 15);
                                }
                                port++;
                            } while (port < 4);
                        }
                    }
                    if (last < w->model->frame) {
                        w->model->frame = last;
                        w->state = 1;
                        w->timer = 0;
                    }
                    break;
                case 1:
                    if (w->timer > 30) {
                        w->state = 2;
                    }
                    break;
                case 2:
                    w->model->frame -= 0.1000000015f;
                    if (w->model->frame < 0.0f) {
                        w->model->frame = 0.0f;
                        w->state = 3;
                        w->timer = 0;
                    }
                    break;
                case 3:
                    if (w->timer > 30) {
                        w->timer = 0;
                        w->state = 0;
                    }
                    break;
            }
            w->timer++;
            sc = w->model->frame * 3 / last;
            w->shadow->scale[2] = sc;
            w->shadow->scale[1] = sc;
            w->shadow->scale[0] = sc;
            if (w->atk != NULL) {
                ((AniAtkFrame *) w->atk->model)->time = w->model->frame;
                if (acollAtkWasHit(w->atk) == 1 && (o = (Obj *) w->atk->hitPower) != NULL) {
                    tskSendMsg(o, 7, obj);
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "hkoban": a thrown coin, flying straight until it hits the ground or something, or goes. */
s32 gimHkobanProc(Obj *obj, s16 msg, void *arg) {
    KobanWork *w = KOBAN(obj);
    Obj *o;
    Vec3f at;
    f32 t;

    switch (msg) {
        case 2:
            obj->name = "hkoban";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = gimHkobanDl;
            floadGetSetRam((void **) w->frame->segs, 0x57);
            w->frame->flags |= 7;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 0.1000000015f;
            w->atk = acollAtkAddYZN(&ene_atk_root_frame->frame, &w->frame->f, 100, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x78;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->atk->frame);
            break;
        case 0:
            w->timer++;
            if (w->frame->pos[0] < 850.0f || w->timer > 60) {
                return -1;
            }
            if (vramRayGround((Vec3f *) w->frame->pos, (Vec3f *) w->vel, &at, &t)) {
                soundIncamCam(0x522, at.x, at.y, at.z);
                itexFlash(at.x, at.y, at.z, 0.1f);
                ptclSpawnSparksFall(at.x, at.y, at.z);
                return -1;
            }
            w->frame->pos[0] += w->vel[0];
            w->frame->pos[1] += w->vel[1];
            w->frame->pos[2] += w->vel[2];
            if (w->atk != NULL && acollAtkWasHit(w->atk) == 1 && (o = (Obj *) w->atk->hitPower) != NULL) {
                tskSendMsg(o, 7, obj);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A coin thrown from (x, y, z) at (tx, ty, tz), 5 a step, with a puff and a sound. */
Obj *gimHkobanThrow(Obj *parent, f32 x, f32 y, f32 z, f32 tx, f32 ty, f32 tz) {
    Obj *o;
    KobanWork *w;

    if ((o = tskCreate(parent, gimHkobanProc, parent->file, NULL)) != NULL) {
        w = KOBAN(o);
        w->frame->pos[0] = x;
        w->frame->pos[1] = y;
        w->frame->pos[2] = z;
        w->vel[0] = tx - x;
        w->vel[1] = ty - y;
        w->vel[2] = tz - z;
        guNormalize(&w->vel[0], &w->vel[1], &w->vel[2]);
        w->vel[0] *= 5.0f;
        w->vel[1] *= 5.0f;
        w->vel[2] *= 5.0f;
        itexPuffDark(x, y, z, 0.1f);
        soundLocationCam(0x237, x, y, z);
    }
    return o;
}

/* "ihead": a big head that turns to the camera's target and, while a player is in front of it
 * and near, throws coins at the target from its mouth, left and right in turn. */
/* 46 lines (mix 0), every named slot right: the compiler temps (frmCreate's address, the
 * floadLoadSet result) sit one word low (0x3C/0x44, the game's 0x40/0x48), and the coin's
 * arguments take their float registers in another rotation (the game loads 207 before
 * scale[2] and multiplies off * scale[0]). Measured: array-element products 66 (either operand
 * order); these Vec3f field views 46; fwd inlined (frame 0xA0) 148.
 * 2026-10-11: both have an unused word between the spill and floadLoadSet's temp (getonProc's
 * shape); the game has one more spill-area word below. w set again at case 2's top: no change;
 * w set at the top of each case instead of at its declaration 185; "scale.x * off" no change.
 * 2026-10-12: k inlined into its four tests (a named word fewer, IDO_NOTES 243/348): frame
 * 0xA8 to 0xA0, 128 lines; "dir.x = dir.z = 0.0f" (a chain's temp, 255): 46, no change; fwd as
 * "207.0f * scale[2]" with sd's Vec3f view: 46; "scale->z * 207.0f": 46.
 * 2026-10-12 (second try, real 9 to 6): the coin's products as "fwd = 207.0f; fwd *= scale->z; sd =
 * off; sd *= scale->x;" (probe t5: a variable's own value first
 * gives the game's operand order; every form of "a * b" gives scale first). Left: only the two
 * temps one word low. w set again after eneSetup or after the light's frmCreate (IDO_NOTES
 * 354): no change (9); fwd inlined into both uses: 21. A probe: case 2
 * alone leaves one unused word below the spill, case 0's turn test, distance test, sincos or
 * throw alone one each, the whole function three; the game has four. The two scale chains
 * written as the game's code reads ("scale[2] = K; scale[0] = scale[1] = scale[2];", the
 * reload of [2] is in the game): 6, the same code and temps.
 * 2026-10-12 (third try): off as a ternary: 6, no move; the model's scales as one three-link chain
 * "[0] = [1] = [2] = 0.5f" with the target's two-link: 6, no move.
 * gimGarekiProc's lever (one word fewer above the temps, one more below) tried here: off
 * dropped (sd set by the if): frame 0xA0, temps the game's distance from the top (0x38/0x40),
 * 85, length -1; with it, "dir.x = dir.z = 0.0f", the model's three-link chain, ty as a
 * ternary, "(s16) (s32)" in mathSinCos's argument: none adds the bottom word (all 85). A
 * diagnostic split of the model's chain changes the code (177). k merged into angle (one s32
 * for both cases): frame 0xA0, every local 4 low, length +2 (54). fwd merged into dx (dead by
 * then): frame 0xA0 and every slot, temps included, the game's distance from the top, the code
 * the game's but for the frame (43): so the game has one register-only word fewer at the
 * bottom of the locals (s and fwd here) and two more unused words above the saved registers
 * (0x30-0x3C empty there, 0x30-0x34 here). Left: what spends those bottom words.
 * 2026-10-12 (fourth try): both scale chains read through s (the operand-order lever, no float type word): frame
 * 0xA0, every slot 8 low and one float 16 low: 43, the same state as above. Measured from the
 * frame's top the temps are then the game's (104), so the chain's float word is this C's extra
 * word; what is missing is 8 bytes at the frame's bottom.
 * 2026-10-12 (fifth try), match_function.py trace (6): the only difference is frmCreate's address
 *   temp (0x3C here, 0x40 in the game: "sw v1/v0, 60(sp)", three loads). uopt's homes run to
 *   -92 (the f32 chain word, f0, blocks 15 and 17); the temp is at -108, so three home words
 *   (-96..-104) are unused. Frame accounting (args 0x28, saved 8, temp region T, homes L):
 *   here L 26 words and T 4; the game L 25 and T 5 (T + L even). The same shape as
 *   ihandpointProc (tlb_0037): one front-end word fewer with the same code, and the frame
 *   unchanged because the temp region then rounds up by one.
 * 2026-10-15 (6): the words under the homes are uopt's spill slots (uopt.c f_spilltemps,
 *   148313-148985: expression bits in order, each takes the first slot of its size that no
 *   interfering temp holds, else a new one at the frame counter, aligned to its size, going
 *   down; it runs before globalcolor, so HEAD(obj)'s expression, bit 4, coloured s0, holds a
 *   never-written slot). Here: homes to -92, floadLoadSet's struct temp -100..-93 (0x44),
 *   HEAD(obj)'s slot -104 (0x40, empty), frmCreate's address -108 (0x3C), more slots to
 *   -120 (0x30). Diagnostic: a dead local holding frmCreate's address declared before w
 *   (one home more): the struct temp 0x40 and the marker 0x3C, i.e. the marker took slot 0.
 *   The game: struct temp 0x48 (-96), an empty 0x44 (-100, HEAD(obj)'s slot) and the marker
 *   0x40 (-104): the game keeps this C's slot order and has ONE HOME WORD FEWER above the
 *   struct temp; and, to fill 0xA8, its slots reach -120 from -104, one 4-byte slot more than
 *   here (this C: -108 then a double's 8 at -120 after alignment; the game would reach only
 *   -112 with the same slots). So the game has one home fewer and one saved expression more:
 *   a named local of this C that was an expression live across blocks or calls in the game.
 *   Tried (one check each, taken back): w uninitialised, "(w = HEAD(obj))->light = .." and w
 *   set at the top of cases 1 and 0: marker unmoved, 58.
 * Measured 2026-10-15 with the patched uopt (vendor/uopt-spill, PROC 15): homes 100 (the
 *   struct temp the last 8), slots HEAD(obj)'s uadd (bit 4) -104 empty, frmCreate's address (30)
 *   with 89/222/225/246 -108, the load 242 -112: counter 112, 0x30 + 112 = 0xA0, and ugen adds
 *   8 more (frame 0xA8). The game: homes 96 (struct temp 0x48), the same three slots -100/-104/
 *   -108 (0x44 empty, the marker 0x40, 0x3C), counter 108, 0x9C, and ugen's part to 0xA8. The
 *   earlier "fwd merged into dx" form (one home fewer, taken back again, 43): homes 96 and the
 *   slots exactly the game's (-100, -104 marker, -108), but frame 0xA0: in that form ugen
 *   reserves nothing at the bottom, where this C's form reserves 8 and the game's 12. So one
 *   named float local fewer is the uopt half; the other half is ugen's bottom words, which
 *   follow the throw's float code (fwd's register changes it).
 * 2026-10-16 (target: homes 96 + ugen bottom 12): MATCH. The frame rule, read off
 *   spill.txt for every form below: above the saved registers ugen reserves align8(homes) +
 *   align8(slot bytes) (homes 100 + 3 slots = 104 + 16 = 0xA8 - 0x30; homes 96 + 3 or 4 slots =
 *   96 + 16 = 0xA0 - 0x30). No "ugen bottom word" exists: ugen made no temps (f_make_new_temp
 *   never ran, ugen-trace copy in the scratchpad). So with homes 96 the game would need 5 slots;
 *   it has homes 100 instead, with the struct temp NOT last: cfe temps are allocated in
 *   creation order, so a float type word first made AFTER floadLoadSet sits below the struct
 *   temp (0x44, empty in the game). The marker then is slot 0 (0x40) and w's uadd slot 1 (0x3C).
 *   Tries (real): fwd merged into dz 49; merged + s32 pad last 6 (homes 100: frame 0xA8, the
 *   rule's control); k's tests inline, k declared unused 43 (uand saved: 4 slots, still 0xA0);
 *   + "w->timer = w->timer + 1" 43; inline, no k, fwd separate 43 (angle 4 high: the game has
 *   a dead word at k's place); switch on "w->timer & 0x3F" 43 (3 slots); "k = w->timer", tests
 *   on k & 0x3F 48; "if (w->side ^= 1) {}" 76; diagnostic u8 chain: no new word; the model's
 *   chain read through s (float word now made by the target's chain, after floadLoadSet) with
 *   fwd separate: struct temp 0x48, 5; + k inline 5; w set per case 57 (still saved, bit 17);
 *   + a dead local holding frmCreate's address declared before w (k's word): marker slot 0,
 *   MATCH. 2026-10-17: the local made live, case 2's two frmCreate calls made through it (a
 *   function pointer, not a FAKE dead store): MATCH, object identical. */
s32 gimIheadProc(Obj *obj, s16 msg, void *arg) {
    /* case 2 creates its frames through this: frmCreate's address is then the first saved
     * expression (uopt bit order), so its spill slot is above w's, as in the game */
    Frame *(*create)(Frame *, s16) = frmCreate;
    HeadWork *w = HEAD(obj);
    s32 angle;
    f32 dx;
    f32 dz;
    f32 sd;
    Vec3f from;
    Vec3f dir;
    Vec3f at;
    f32 t;
    f32 off;
    f32 ty;
    f32 c;
    f32 s;
    f32 fwd;

    switch (msg) {
        case 2:
            obj->name = "ihead";
            angle = eneSetup(obj, arg);
            w->light =(LightFrame *) create(gFrmLayer2, gFrmKindLight);
            w->model = eneNewAnime(obj, (u32) gimIheadAnim);
            w->target = (PolFrame *) create(gFrmEffectLayer, gFrmKindPol);
            if (w->model == NULL || w->model == NULL || w->target == NULL) {
                return -1;
            }
            w->light->lightNum = 1;
            w->light->color[0] = 255;
            w->light->color[1] = 160;
            w->light->color[2] = 96;
            w->light->dir[0] = 0;
            w->light->dir[1] = -127;
            w->light->dir[2] = 0;
            frmMove(&w->model->f, &w->light->f);
            w->model->dl = gDrawSetupLitOpaDl;
            w->model->scale[2] = 0.5f;
            s = w->model->scale[2];
            w->model->scale[1] = s;
            w->model->scale[0] = s;
            w->model->pos[0] += 200.0f;
            w->model->pos[1] += -275.0f;
            floadLoadSet(0x57);
            floadGetSetRam((void **) w->target->segs, 0x57);
            w->target->flags |= 7;
            w->target->scale[2] = 0.1000000015f;
            w->target->scale[0] = w->target->scale[1] = w->target->scale[2];
            if (angle != 0) {
                w->fixed = 1;
                w->model->rot[1] = angle * PI / 512;
            }
            break;
        case 1:
            frmDestroy(&w->target->f);
            frmDestroy(&w->model->f);
            frmDestroy(&w->light->f);
            break;
        case 3:
            break;
        case 0:
            if (w->fixed == 0) {
                w->target->pos[0] = gGameWork.view->at[0];
                w->target->pos[1] = gGameWork.view->at[1];
                w->target->pos[2] = gGameWork.view->at[2];
                w->model->rot[1] = mathAtan2(w->target->pos[0] - w->model->pos[0],
                                             w->target->pos[2] - w->model->pos[2]);
                if (((s32) (w->model->rot[1] / PI * 512 - 640.0f) & 0x3FF) <= 256) {
                    dx = w->target->pos[0] - w->model->pos[0];
                    dz = w->target->pos[2] - w->model->pos[2];
                    if (!(400.0f < sqrtf(dx * dx + dz * dz))) {
                        if ((w->timer & 0x3F) == 1 || (w->timer & 0x3F) == 5 || (w->timer & 0x3F) == 9
                            || (w->timer & 0x3F) == 13) {
                            from.x = w->target->pos[0];
                            from.y = w->target->pos[1] + 20.0f;
                            from.z = w->target->pos[2];
                            dir.x = 0.0f;
                            dir.y = -60.0f;
                            dir.z = 0.0f;
                            if (vramRayGround(&from, &dir, &at, &t)) {
                                ty = at.y;
                            } else {
                                ty = w->target->pos[1];
                            }
                            if (w->side) {
                                off = 15.0f;
                            } else {
                                off = -15.0f;
                            }
                            w->side ^= 1;
                            s = mathSinCos((s16) (w->model->rot[1] / PI * 512), &c);
                            fwd = 207.0f;
                            fwd *= ((Vec3f *) w->model->scale)->z;
                            sd = off;
                            sd *= ((Vec3f *) w->model->scale)->x;
                            gimHkobanThrow(obj, w->model->pos[0] + fwd * s + sd * c,
                                                   w->model->pos[1] + 654.0f * ((Vec3f *) w->model->scale)->y,
                                                   w->model->pos[2] + fwd * c + sd * s, w->target->pos[0] + sd * c,
                                                   ty, w->target->pos[2] + sd * s);
                        }
                        w->timer++;
                    }
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Whether the body is within 800 of the camera and inside its view cone (0.8 of its fovy). */
s32 gimIbodyInView(BodyWork *w) {
    s32 pad;
    f32 cone;
    f32 dx;
    f32 dz;
    f32 lx;
    f32 lz;
    s32 pad2[2];
    CamView *v = gGameWork.view;

    cone = mathCos(fabsf(v->fovy) * 0.8000000119f);
    dx = w->model->pos[0] - v->eye[0];
    dz = w->model->pos[2] - v->eye[2];
    if (800.0f < sqrtf(dx * dx + dz * dz)) {
        return 0;
    }
    lx = v->at[0] - v->eye[0];
    lz = v->at[2] - v->eye[2];
    if ((lx * dx + lz * dz) / sqrtf((lx * lx + lz * lz) * (dx * dx + dz * dz)) < cone) {
        return 0;
    }
    return 1;
}

/* The body's footstep, panned by where it is across the view. */
/* 86 lines (mix 2), frame right: the game keeps b (at.x - eye.x) in its home (0x28, stored and
 * reloaded) with eye.x, eye.z, a, dx, dz in f2, f12, f18, f14, f16; here IDO leaves ez in memory
 * instead and the pan's s16 work lands in a0/a3 (the game's v0/a0/v1). Measured: v, a, b, dx,
 * dz with eye.x/eye.z inline (temps: frame 0x50, 96); the first form (v first, 88); b computed
 * right after ex (ez still in memory, 86).
 * 2026-10-12 (real 44 to 33, mix 0): b stored once and reloaded once for its three uses
 * is an address-taken b (FAKE "if (&b) {}" after it); with dx, dz, a, b in that order the game's
 * loads come out in its order (one w->model load). Left: ex/ez take f0/f2 (the game's f2/f12,
 * f0 unused: every uopt float one register down) and b is computed first. Measured: a, dx, b,
 * dz 42; a, dx, dz, b 41; dx, a, dz, b 35; eye reads inline (no ex/ez) 39, frame 0x48; the
 * sqrt in a local d: 33, no change; "b = at.x; b -= ex" without the FAKE 53; the FAKE at the
 * body's top: the game's instruction order but ex/ez in f16/f18 and frame 0x48 (35).
 * 2026-10-12 (second try): ex/ez dead-initialised (= 0.0f): 33, no change; eye reads inline with
 * pad1[2] at the top: frame 0x48 (39), without it 35 (eye.x/eye.z still f0/f2: the register
 * shift is not ex/ez being locals); b inline at its two uses as a CSE (as gimIbodyInView's
 * parked eye.z), no b: 57, with ex/ez: 50 (IDO parks eye.z instead); "b = c = at.x - ex" (a
 * chain, c for the pad): 46.
 * 2026-10-12 (third try), match_function.py trace (no FAKE, 46): one block from the if to the pan;
 *   f0 is out for every range in it (the sqrt), so 5 float registers for 6 variables. uopt's
 *   priorities: dx, dz, a, b 4.0 (def + 3 uses), ex, ez 3.0; colouring dx f2, dz f12, a f14,
 *   b f16, ex f18, ez uncoloured (stored 0x38, reloaded). The game's listing is the same code
 *   with ex f2, ez f12, dx f14, dz f16, a f18 and b the uncoloured one (stored to its home 0x28,
 *   reloaded once into a ugen temp f6): i.e. colouring in source order, ex and ez at least as
 *   high as the rest. With the FAKE, b's load is a CSE range (f2) and ex takes f0 (block 2 then
 *   has no sqrt). Tries: eye reads inline (expression ranges 3.0 too): 54, frame 0x50; the sqrt
 *   in a local d (no block boundary: copy-propagated): 46. Open: a form giving ex/ez a fourth
 *   reference or dx/dz/a/b one fewer, in one block, with the same code.
 * Proof 2026-10-16 (match_function.py force, on the 9 form with the ez test):
 *   - Swapping its two float ranges (33, M -12, f2 to f18; 10, M -4, f18 to f2; 14/18/24 pinned
 *     at f12/f14/f16) gives 3.
 *   - The 3 left are ugen's: the mul.s at instruction 43 gets f18, where the game's gets f4.
 *   - So the target is the swap plus ugen's float list one step on.
 * 2026-10-16 (force targets): MATCH. uopt's adjsave = refs / count, count 2 for 2-5 live
 *   bbs; the float ranges are coloured by adjsave, a tie in range (source) order. The game's
 *   colouring is that tie: ex, ez, dx, dz, a all 2.0 (4 refs over 2+ blocks), each live across
 *   the sqrt's block (f0 out). And the pan's float is not stored in a (ugen temp f4, the last 3),
 *   pan is defined in a block where v is dead (v1). Tries (real):
 *   - ez FAKE dropped, a/b then "if (&b)", pan direct: 33 (ex/ez only in block 2: ex f0).
 *   - "if (&b)" before b, ez FAKE after pan: 33 (ez 2.0 f2, ex 1.5 f18).
 *   - the same, FAKE on "(u32) ex + (u32) ez": 23, every float register right; pan a3 (v live
 *     in its block), frame +8 (the pan+64 expression held across the FAKE's block).
 *   - FAKE before pan's statement: 33 (ex/ez die before the sqrt: f0); FAKE "pad = (u32) (ex +
 *     ez)" (no branch): 38 (dead store, its uses not counted); fused into the clamp as "pan <= 0
 *     || ... < 0" 78, "pan <= 0 && ... >= 0" 77 (both emit the test).
 *   - "(u32) (ex + ez)" after pan: 23.
 *   - a, b, then "if (&b)", pan direct, the FAKE inside "if (pan <= 0)": MATCH. */
void gimIbodyStepSound(BodyWork *w) {
    f32 ex;
    f32 ez;
    f32 a;
    f32 dx;
    f32 dz;
    f32 b;
    s16 pan;
    s32 pad;
    CamView *v = gGameWork.view;

    if (gimIbodyInView(w)) {
        ex = v->eye[0];
        ez = v->eye[2];
        dx = w->model->pos[0] - ex;
        dz = w->model->pos[2] - ez;
        a = ez - v->at[2];
        b = v->at[0] - ex;
        if (&b) {} /* FAKE: b's address taken keeps it in its home (0x28), as the game */
        pan = (s16) ((a * dx + b * dz) / sqrtf((a * a + b * b) * (dx * dx + dz * dz)) * 64) + 64;
        if (pan <= 0) {
            /* FAKE: an empty test; one more use of ex and ez, after the sqrt, so
             * both are live across it (f0 out) with uopt priority 2.0 like dx/dz/a, and the tie
             * colours them in source order (ex f2, ez f12, dx f14, dz f16, a f18) */
            if (ex + ez) {}
            pan = 1;
        } else if (pan >= 128) {
            pan = 127;
        }
        sndRequest(0x280, pan, 0x99);
    }
}

/* "ibody": a body that walks back and forth (400 along z), turning at each end, its footsteps
 * heard when the camera sees it; while seen, the camera is pulled out to 1000. */
s32 gimIbodyProc(Obj *obj, s16 msg, void *arg) {
    BodyWork *w = BODY(obj);
    f32 f;
    s32 pad[4];
    Camera *cam;

    switch (msg) {
        case 2:
            obj->name = "ibody";
            eneSetup(obj, arg);
            w->model = eneNewAnime(obj, (u32) gimIbodyAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 0.5f;
            w->last = polAnimeLastFrame(w->model);
            w->far = 400.0f;
            break;
        case 1:
            frmDestroy(&w->model->f);
            break;
        case 0:
            if ((cam = camAiGetCam()) != NULL) {
                camSetSpeed(cam, 16.0f);
                if (gimIbodyInView(w) == 1) {
                    camMoveFar(cam, 1000.0f);
                    w->model->f.flags |= 2;
                } else {
                    camMoveFar(cam, w->far);
                    w->model->f.flags &= ~2;
                }
            }
            switch (w->state) {
                case 0:
                    w->model->pos[2] += 2.0f;
                    eneAnimLoop(obj, w->model, 0.1f);
                    f = w->model->frame;
                    if ((0.0f <= f && f < 0.1) || (w->last / 2 <= f && f < w->last / 2 + 0.1)) {
                        gimIbodyStepSound(w);
                    }
                    if (w->z + 400.0f <= w->model->pos[2]) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    w->model->rot[1] -= 0.01745329238f;
                    eneAnimLoop(obj, w->model, 0.1f);
                    if (w->model->rot[1] <= -3.141592741f) {
                        w->model->rot[1] = -3.141592741f;
                        w->state = 2;
                    }
                    break;
                case 2:
                    w->model->pos[2] -= 2.0f;
                    eneAnimLoop(obj, w->model, 0.1f);
                    f = w->model->frame;
                    if ((0.0f <= f && f < 0.1) || (w->last / 2 <= f && f < w->last / 2 + 0.1)) {
                        gimIbodyStepSound(w);
                    }
                    if (w->model->pos[2] <= w->z) {
                        w->state = 3;
                    }
                    break;
                case 3:
                    w->model->rot[1] += 0.01745329238f;
                    eneAnimLoop(obj, w->model, 0.1f);
                    if (0.0f <= w->model->rot[1]) {
                        w->model->rot[1] = 0.0f;
                        w->state = 0;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "edopole": a pole across the course with four attack boxes along it. Message 0's loop skips an
 * empty box with "continue": written as one "&&" test, IDO coloured the constant 1 (shared with
 * the switch's case-1 test) before the loop end. */
s32 gimEdopoleProc(Obj *obj, s16 msg, void *arg) {
    PoleWork *w = POLE(obj);
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "edopole";
            eneSetup(obj, arg);
            w->frame = eneNewPol(obj, gimEdopoleDl);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->rot[1] = vramCourseHeading(w->zone, w->x) - 1.570796371f;
            for (i = 0; i < 4; i++) {
                w->atk[i] = acollAtkAddYZN(&ene_atk_root_frame->frame, &w->frame->f, 40, 0, i * 40 - 60, 0);
                if (w->atk[i] == NULL) {
                    return 0;
                }
                w->atk[i]->kind = 0x6A;
            }
            break;
        case 1:
            for (i = 0; i < 4; i++) {
                frmDestroy(&w->atk[i]->frame);
            }
            frmDestroy(&w->frame->f);
            break;
        case 0:
            for (i = 0; i < 4; i++) {
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

/* The rubble breaks: pieces at each attack box, the boxes and the shadow gone, a sound. */
void gimGarekiBreak(Obj *obj, GarekiWork *w) {
    ACollFrame *b;

    for (b = w->atk; b != NULL; b = (ACollFrame *) b->frame.child) {
        gimPieceThrow(obj, b->pos[0], b->pos[1], b->pos[2]);
    }
    w->shadow = (AnimeFrame *) frmDestroy(&w->shadow->f);
    if (w->atk != NULL) {
        w->atk = (ACollFrame *) aniAtkDelete(w->atk);
    }
    w->model->f.flags &= ~2;
    SoundIncam2(0x2C0, gGameWork.view, w->model->pos[0], w->model->pos[1], w->model->pos[2], 0.0f, 300.0f);
}

/* "gareki": rubble that shakes loose (puffing), falls, and breaks on the ground or when hit;
 * or, placed in state 2, an animation that crumbles. */
/* 20 lines (mix 0) since (688): the puff's first fadeRand result is spilled at 0x40 here, 0x44
 * in the game (one compiler temp more below it there). Message 1's work-pointer slot is fixed
 * by its test and the next destroy on one line (IDO_NOTES 324; FAKE layout). A pad after the
 * last local moves the temps down (34); "(fadeRand(40) - 20.0f)" grouped: 36. Measured before: c a plain s32 (no home store, 123);
 * "if (c) {}" and "do {} while (c && 0)" keep code (438, 382); s inlined (frame 0xB0, 291);
 * "dir.z = dir.x = 0" fixed the store order (121 to 119); volatile c 24.
 * 2026-10-11: eneGetFile read into a local (pad0's word; fboard's lever) no change (20); w set at
 * the top of each case instead of at its declaration (IDO_NOTES 354) 505.
 * 2026-10-12: both puffs written "fadeRand(40) + pos - 20.0f": 43. The game's temps are 0x38
 * and 0x44 (0x3C, 0x40 empty), this C's 0x38 and 0x40: one phantom temp slot more in the game.
 * 2026-10-12 (second try): pad[2] as pad[1] (the batch-20 "temp 4 too low = one local too many"):
 * frame 0xB8 to 0xB0, 122 lines (real 50); the temps did not move. The shadow's two scale
 * stores as a chain "scale[0] = scale[2] = s" (IDO_NOTES 255, a chain's temp): 189 lines (real
 * 90, length +1), the temps did not move. gimIheadProc has the same one-word-low temps.
 * 2026-10-12 (third try): the shadow's frmCreateNext assigned inside its test (falls22V's lever
 * reversed: there that form cost a spill word): 2, no move; "++w->timer < 30" in state 2: 2.
 * A probe: the offsets read into locals first put the spill in the
 * local's home, not a temp word.
 * 2026-10-12 (fourth try): s as a block local in the ray's hit branch: 2, no move (a block local's
 * word is in the locals' area, not the temps'). The dir chain as "= 0.0" (a double chain): 68,
 * no move; the chain split into two stores: 2, no move (so its word is not between 0x38 and the
 * fadeRand temp).
 * Then (d): MATCH. A diagnostic (the shadow's three gfx commands left out) showed the whole
 * spill area is laid top-down after the locals: the game had one word fewer above the fadeRand
 * temp and one more below it. With no s local and the two scale stores as one chain
 * "scale[0] = scale[2] = (1.0f - t) * x" (its word goes below), the frame and temps are the
 * game's (the chain from s was 189: it reloads; from the expression it does not). */
s32 gimGarekiProc(Obj *obj, s16 msg, void *arg) {
    GarekiWork *w = GAREKI(obj);
    s32 pad0;
    Frame hdr;
    s32 c; /* the game stores eneSetup's result here and never reads it */
    s32 pad[2];
    Vec3f from;
    Vec3f dir;
    Vec3f at;
    f32 t;

    switch (msg) {
        case 2:
            obj->name = "gareki";
            if (&c) {} /* FAKE: c's address taken keeps its store (it was volatile) */
            c = eneSetup(obj, arg);
            w->y -= 20.0f;
            w->rot[1] = fadeRand(360) * PI / 180.0f;
            if ((u32) gimGarekiAnim != 0) {
                w->model = eneNewAnime(obj, (u32) gimGarekiAnim);
            }
            if (w->model == NULL) {
                return -1;
            }
            w->alpha = 255;
            w->timer = 60;
            w->accel = 0.0f;
            w->shadow = (AnimeFrame *) frmCreateNext(gFrmLayer4, gFrmKindAnime);
            if (w->shadow == NULL) {
                return -1;
            }
            hdr = w->shadow->f;
            *w->shadow = *w->model;
            w->shadow->f = hdr;
            w->shadow->scale[1] = 0.0f;
            if ((s32) gimGarekiAtkChain != 0) {
                w->atk = aniAtkCreate(&ene_atk_root_frame->frame, &w->model->f, (u32) gimGarekiAtkChain, eneGetFile(obj, 0), 0);
            }
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x78;
            w->dmg = acollDmgAddXYZN(&ene_dmg_root_frame->frame, &w->model->f, 200, 100, 0, 0, 0, 0);
            break;
        case 1:
            if (w->atk != NULL) { aniAtkDelete(w->atk); } frmDestroy(&w->dmg->frame);
            frmDestroy(&w->model->f);
            frmDestroy(&w->shadow->f);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->timer--;
                    if (w->timer < 40 && (w->timer & 0xF) == 1) {
                        gimPieceMakeNear(obj, w->x, w->y, w->z);
                    }
                    if (w->timer == 0) {
                        w->accel = -0.400000006f;
                        w->state = 1;
                    }
                    break;
                case 1:
                    w->vel[1] += w->accel;
                    if (w->vel[1] < -4.0f) {
                        w->vel[1] = -4.0f;
                    }
                    if (!eneLand((EneObj *) w, 2.0f)) {
                        w->y += w->vel[1];
                    } else {
                        gimGarekiBreak(obj, w);
                        w->timer = 0;
                        w->state = 4;
                        break;
                    }
                    if (w->dmg != NULL && acollDmgWasHit(w->dmg) == 1) {
                        gimGarekiBreak(obj, w);
                        w->timer = 0;
                        w->state = 4;
                    }
                    break;
                case 2:
                    w->timer++;
                    if (w->timer < 30 && (w->timer & 7) == 1) {
                        itexPuffDark(w->model->pos[0] + fadeRand(40) - 20.0f, w->model->pos[1],
                                     w->model->pos[2] + fadeRand(40) - 20.0f, 0.1f);
                    }
                    w->model->frame += 0.5f;
                    if (polAnimeLastFrame(w->model) < w->model->frame) {
                        w->model->frame = polAnimeLastFrame(w->model);
                        frmMove(&w->model->f, gFrmLayer8);
                        if (w->atk != NULL) {
                            w->atk = (ACollFrame *) aniAtkDelete(w->atk);
                        }
                        w->state = 3;
                    }
                    if (w->dmg != NULL && acollDmgWasHit(w->dmg) == 1) {
                        gimGarekiBreak(obj, w);
                        w->timer = 0;
                        w->state = 4;
                    }
                    break;
                case 4:
                    w->timer++;
                    if (w->timer > 60) {
                        eneReleaseSpawn(w->spawn);
                        eneClearSpawnWait(w->spawn);
                        return -1;
                    }
                    break;
                case 3:
                    w->fade = -8;
                    w->alpha += w->fade;
                    if (w->alpha <= 0) {
                        eneReleaseSpawn(w->spawn);
                        eneClearSpawnWait(w->spawn);
                        return -1;
                    }
                    break;
            }
            eneSetModelOnCourse(obj, &w->model->f);
            if (w->atk != NULL) {
                ((AniAtkFrame *) w->atk->model)->time = w->model->frame;
                if (acollAtkWasHit(w->atk) == 1 && (Obj *) w->atk->hitPower != NULL) {
                    tskSendMsg((Obj *) w->atk->hitPower, 7, obj);
                }
            }
            if (w->shadow != NULL) {
                from.x = w->model->pos[0];
                from.y = w->model->pos[1] + 1.0f;
                from.z = w->model->pos[2];
                dir.z = dir.x = 0.0f;
                dir.y = -40.0f;
                if (vramRayGround(&from, &dir, &at, &t)) {
                    w->shadow->f.flags |= 2;
                    w->shadow->pos[0] = at.x;
                    w->shadow->pos[1] = at.y;
                    w->shadow->pos[2] = at.z;
                    w->shadow->scale[0] = w->shadow->scale[2] = (1.0f - t) * ((Vec3f *) w->model->scale)->x;
                } else {
                    w->shadow->f.flags &= ~2;
                }
                w->shadow->frame = w->model->frame;
                w->shadow->rot[1] = w->model->rot[1];
            }
            eneStepHookPost(obj);
            break;
        case 3:
            if (w->model != NULL) {
                w->model->dl = gGfxDlHead;
                gDPPipeSync(gGfxDlHead++);
                gDPSetCombine(gGfxDlHead++, 0x409A81, 0xFF37FFFF);
                gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
                gSPEndDisplayList(gGfxDlHead++);
            }
            if (w->shadow != NULL) {
                w->shadow->dl = gGfxDlHead;
                gDPSetCombine(gGfxDlHead++, 0xFFFFFF, 0xFFFDF6FB);
                gDPSetPrimColor(gGfxDlHead++, 0, 0, 0, 0, 0, w->alpha);
                gSPEndDisplayList(gGfxDlHead++);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "fireball": a flame (an animated texture) rising and growing; its attack box goes when its
 * life runs out, then it fades. */
s32 gimFireballProc(Obj *obj, s16 msg, void *arg) {
    FireWork *w = FIRE(obj);

    switch (msg) {
        case 2:
            obj->name = "fireball";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = gimFireballDl;
            floadGetSetRam((void **) w->frame->segs, 0x125);
            fadeAnimStart(&w->anim, gItexTimedYellowAnim, 0, &w->frame->segs[3], floadGetSetFile(0x125, 0));
            w->frame->flags |= 7;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 0.3000000119f;
            w->alpha = 255;
            w->atk = acollAtkAddYZN(&ene_atk_root_frame->frame, &w->frame->f, 20, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x78;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->atk->frame);
            break;
        case 0:
            fadeAnimStep(&w->anim);
            w->frame->scale[0] *= 1.004999995f;
            w->frame->scale[1] *= 1.004999995f;
            w->frame->scale[2] *= 1.004999995f;
            w->frame->pos[1] += 1.0f;
            if (--w->life == 0) {
                w->atk = (ACollFrame *) frmDestroy(&w->atk->frame);
            }
            if (w->life < 0) {
                w->alpha -= 16;
                if (w->alpha < 0) {
                    return -1;
                }
            }
            if (w->atk != NULL && acollAtkWasHit(w->atk) == 1 && (Obj *) w->atk->hitPower != NULL) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, obj);
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombine(gGfxDlHead++, 0x30D261, 0x55A6FF7F);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 128, 0);
            gDPSetEnvColor(gGfxDlHead++, 255, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A fireball at (x, y, z) whose attack box lasts life steps. */
Obj *gimFireballMake(Obj *parent, f32 x, f32 y, f32 z, s32 life) {
    Obj *o;

    if ((o = tskCreate(parent, gimFireballProc, parent->file, NULL)) != NULL) {
        FIRE(o)->frame->pos[0] = x;
        FIRE(o)->frame->pos[1] = y;
        FIRE(o)->frame->pos[2] = z;
        FIRE(o)->life = life;
    }
    return o;
}

/* "firepoint": a flame that throws fireballs up, every 16 steps for 90 steps (with a sound),
 * then small ones for 90; it swells with each. */
s32 gimFirepointProc(Obj *obj, s16 msg, void *arg) {
    FirePointWork *w = FIREPOINT(obj);

    switch (msg) {
        case 2:
            obj->name = "firepoint";
            eneSetup(obj, arg);
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = gimFireModelDl;
            floadGetSetRam((void **) w->frame->segs, 0x125);
            fadeAnimStart(&w->anim, gItexTimedYellowAnim, 0, &w->frame->segs[3], floadGetSetFile(0x125, 0));
            w->frame->dl = (Gfx *) fadeSegToRamP(gimFireDl, obj->file);
            w->frame->flags |= 2;
            vramCoursePointXZ(w->zone, w->x, w->z, &w->frame->pos[0], &w->frame->pos[2]);
            w->frame->pos[1] = w->y;
            w->frame->scale[0] = w->frame->scale[2] = 0.6999999881f;
            w->frame->scale[1] = w->size = 0.5f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            break;
        case 0:
            fadeAnimStep(&w->anim);
            switch (w->state) {
                case 0:
                    if (w->timer == 1) {
                        soundSuionCreate(obj, 0x29E, gGameWork.view, w->frame->pos[0], w->frame->pos[1],
                                         w->frame->pos[2], 0.0f, 0.0f);
                    }
                    if (w->timer > 90) {
                        w->state = 1;
                        w->timer = 0;
                    }
                    if ((w->timer & 0xF) == 1) {
                        gimFireballMake(obj, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 64);
                        w->size = 0.8999999762f;
                    } else {
                        w->size = 0.6999999881f;
                    }
                    break;
                case 1:
                    if (w->timer > 90) {
                        w->state = 0;
                        w->timer = 0;
                    }
                    if ((w->timer & 0xF) == 1) {
                        gimFireballMake(obj, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 4);
                        w->size = 0.6000000238f;
                    } else {
                        w->size = 0.5;
                    }
                    break;
            }
            if (w->size - w->frame->scale[1] <= -0.01) {
                w->frame->scale[1] -= 0.01;
            } else if (w->size - w->frame->scale[1] >= 0.01) {
                w->frame->scale[1] += 0.01;
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "firewall": five flames across the course, each its own frame of one animated texture (the
 * middle one stays where it was placed). */
s32 gimFirewall5Proc(Obj *obj, s16 msg, void *arg) {
    FireWallWork *w = FIREWALL(obj);
    s32 i;
    s32 c;
    f32 s;
    s32 pad;
    f32 cs;
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "firewall";
            c = eneSetup(obj, arg);
            for (i = 0; i < 5; i++) {
                w->fires[i] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
                if (w->fires[i] != NULL) {
                    w->fires[i]->dl2 = gimFireModelDl;
                    w->fires[i]->segs[0] = eneFileAddr(obj, 0, 0);
                    w->fires[i]->flags |= 2;
                    w->fires[i]->dl = (Gfx *) fadeSegToRamP(gimFireDl, obj->file);
                    vramCoursePointXZ(w->zone, w->x, w->z, &w->fires[i]->pos[0], &w->fires[i]->pos[2]);
                    w->fires[i]->pos[1] = w->y;
                    w->fires[i]->scale[0] = w->fires[i]->scale[1] = w->fires[i]->scale[2] = 0.5f;
                }
            }
            s = mathSinCos(c, &cs);
            if (w->fires[0] != NULL) {
                w->fires[0]->pos[0] += -45.0f * cs;
                w->fires[0]->pos[2] += -45.0f * s;
                w->fires[0]->pos[1] += -5.0f;
            }
            if (w->fires[1] != NULL) {
                w->fires[1]->pos[0] += -20.0f * cs - 5.0f * s;
                w->fires[1]->pos[2] += -20.0f * s - 5.0f * cs;
                w->fires[1]->pos[1] += -5.0f;
                w->fires[1]->scale[0] = w->fires[1]->scale[2] = 1.0f;
                w->fires[1]->scale[1] = 0.6000000238f;
            }
            if (w->fires[3] != NULL) {
                w->fires[3]->pos[0] += 35.0f * cs + 2 * s;
                w->fires[3]->pos[2] += 35.0f * s + 2 * cs;
                w->fires[3]->scale[0] = w->fires[3]->scale[2] = 0.6000000238f;
                w->fires[3]->scale[1] = 0.400000006f;
            }
            if (w->fires[4] != NULL) {
                w->fires[4]->pos[0] += 58.0f * cs;
                w->fires[4]->pos[2] += 58.0f * s;
                w->fires[4]->scale[1] = 0.6999999881f;
            }
            if (w->fires[0] != NULL) {
                fadeAnimStart(&w->anim, gItexTimedYellowAnim, 0, &w->fires[0]->segs[3], eneGetFile(obj, 0));
            }
            for (i = 1; i < 5; i++) {
                if (w->fires[i] != NULL) {
                    w->fires[i]->segs[3] = w->fires[0]->segs[3];
                }
            }
            break;
        case 1:
            for (i = 0; i < 5; i++) {
                frmDestroy(&w->fires[i]->f);
            }
            break;
        case 0:
            fadeAnimStep(&w->anim);
            for (i = 1; i < 5; i++) {
                if (w->fires[i] != NULL) {
                    w->fires[i]->segs[3] = fadeAnimFrameAt(&w->anim, w->tick + i * 7);
                }
            }
            w->tick++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "firewall" (two flames): the short wall. */
s32 gimFirewall2Proc(Obj *obj, s16 msg, void *arg) {
    FireWallWork *w = FIREWALL(obj);
    s32 i;
    s32 c;
    f32 s;
    s32 pad;
    f32 cs;
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "firewall";
            c = eneSetup(obj, arg);
            for (i = 0; i < 2; i++) {
                w->fires[i] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
                if (w->fires[i] != NULL) {
                    w->fires[i]->dl2 = gimFireModelDl;
                    w->fires[i]->segs[0] = eneFileAddr(obj, 0, 0);
                    w->fires[i]->flags |= 2;
                    w->fires[i]->dl = (Gfx *) fadeSegToRamP(gimFireDl, obj->file);
                    vramCoursePointXZ(w->zone, w->x, w->z, &w->fires[i]->pos[0], &w->fires[i]->pos[2]);
                    w->fires[i]->pos[1] = w->y;
                    w->fires[i]->scale[0] = w->fires[i]->scale[1] = w->fires[i]->scale[2] = 0.5f;
                }
            }
            s = mathSinCos(c, &cs);
            if (w->fires[0] != NULL) {
                w->fires[0]->scale[1] = 0.6999999881f;
            }
            if (w->fires[1] != NULL) {
                w->fires[1]->pos[0] += -5.0f * cs;
                w->fires[1]->pos[2] += -5.0f * s;
                w->fires[1]->pos[1] += -2.0f;
            }
            if (w->fires[0] != NULL) {
                fadeAnimStart(&w->anim, gItexTimedYellowAnim, 0, &w->fires[0]->segs[3], eneGetFile(obj, 0));
            }
            for (i = 1; i < 2; i++) {
                if (w->fires[i] != NULL) {
                    w->fires[i]->segs[3] = w->fires[0]->segs[3];
                }
            }
            break;
        case 1:
            for (i = 0; i < 2; i++) {
                frmDestroy(&w->fires[i]->f);
            }
            break;
        case 0:
            fadeAnimStep(&w->anim);
            for (i = 1; i < 2; i++) {
                if (w->fires[i] != NULL) {
                    w->fires[i]->segs[3] = fadeAnimFrameAt(&w->anim, w->tick + i * 7);
                }
            }
            w->tick++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "firewall" (four flames). */
s32 gimFirewall4Proc(Obj *obj, s16 msg, void *arg) {
    FireWallWork *w = FIREWALL(obj);
    s32 i;
    s32 c;
    f32 s;
    s32 pad;
    f32 cs;
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "firewall";
            c = eneSetup(obj, arg);
            for (i = 0; i < 4; i++) {
                w->fires[i] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
                if (w->fires[i] != NULL) {
                    w->fires[i]->dl2 = gimFireModelDl;
                    w->fires[i]->segs[0] = eneFileAddr(obj, 0, 0);
                    w->fires[i]->flags |= 2;
                    w->fires[i]->dl = (Gfx *) fadeSegToRamP(gimFireDl, obj->file);
                    vramCoursePointXZ(w->zone, w->x, w->z, &w->fires[i]->pos[0], &w->fires[i]->pos[2]);
                    w->fires[i]->pos[1] = w->y;
                    w->fires[i]->scale[0] = w->fires[i]->scale[1] = w->fires[i]->scale[2] = 0.5f;
                }
            }
            s = mathSinCos(c, &cs);
            if (w->fires[0] != NULL) {
                w->fires[0]->scale[0] = w->fires[0]->scale[1] = w->fires[0]->scale[2] = 0.8000000119f;
            }
            if (w->fires[1] != NULL) {
                w->fires[1]->pos[0] += -20.0f * cs + 5.0f * s;
                w->fires[1]->pos[2] += -20.0f * s + 5.0f * cs;
                w->fires[1]->pos[1] += -5.0f;
                w->fires[1]->scale[0] = w->fires[1]->scale[2] = 0.8999999762f;
                w->fires[1]->scale[1] = 0.6999999881f;
            }
            if (w->fires[2] != NULL) {
                w->fires[2]->pos[0] += 18.0f * cs + 10.0f * s;
                w->fires[2]->pos[2] += 18.0f * s + 10.0f * cs;
                w->fires[2]->scale[1] = 0.8000000119f;
            }
            if (w->fires[3] != NULL) {
                w->fires[3]->pos[0] += 5.0f * cs - 20.0f * s;
                w->fires[3]->pos[2] += 5.0f * s - 20.0f * cs;
                w->fires[3]->scale[0] = w->fires[3]->scale[2] = 0.6999999881f;
                w->fires[3]->scale[1] = 0.8000000119f;
            }
            if (w->fires[0] != NULL) {
                fadeAnimStart(&w->anim, gItexTimedYellowAnim, 0, &w->fires[0]->segs[3], eneGetFile(obj, 0));
            }
            for (i = 1; i < 4; i++) {
                if (w->fires[i] != NULL) {
                    w->fires[i]->segs[3] = w->fires[0]->segs[3];
                }
            }
            break;
        case 1:
            for (i = 0; i < 4; i++) {
                frmDestroy(&w->fires[i]->f);
            }
            break;
        case 0:
            fadeAnimStep(&w->anim);
            for (i = 1; i < 4; i++) {
                if (w->fires[i] != NULL) {
                    w->fires[i]->segs[3] = fadeAnimFrameAt(&w->anim, w->tick + i * 7);
                }
            }
            w->tick++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
