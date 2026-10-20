#include "tlb.h"

/* "sasrock": a rock that breaks into three "piece"s, falling and fading, when an attack of kind
 * 0xE (a bomb?) hits it. */

/* A "piece": a bit of rock thrown up, falling, turning and fading (as tlb_0048's). */
typedef struct PieceWork {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ f32 pos[3];
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *frame;
    /* 0x38 */ s32 alpha;
    /* 0x3C */ s32 fade;
    /* 0x40 */ s32 puff; /* steps to the next dark puff */
} PieceWork;

#define PIECE(obj) ((PieceWork *) FX(obj))

/* The rock: an enemy's work (EneObj) to 0x34, then its own. */
typedef struct SasrockWork {
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
    /* 0x3C */ ACollFrame *dmg;
} SasrockWork;

/* acollDmgAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*DmgAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height,
                                     s16 ox, s16 oy, s16 oz, s32 a);

extern Frame *gFrmLayer3;
extern Gfx sasrockPieceDl[];
extern Gfx sasrockDl[];
extern u8 sasrockRide[];

s32 sasrockPiece(Obj *obj, s16 msg, void *arg);
void sasrockPieceCreate(Obj *parent, f32 x, f32 y, f32 z);

/* "piece" */
s32 sasrockPiece(Obj *obj, s16 msg, void *arg) {
    PieceWork *w = PIECE(obj);

    switch (msg) {
        case 2:
            obj->name = "piece";
            w->frame = (PolFrame *) frmCreateNext(gFrmLayer3, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = sasrockPieceDl;
            floadGetSetRam((void **) w->frame->segs, 0x5A);
            w->rot[1] = fadeRand(360) * PI / 180.0f;
            w->alpha = 255;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->vel[1] += -0.4;
            w->pos[0] += w->vel[0];
            w->pos[1] += w->vel[1];
            w->pos[2] += w->vel[2];
            w->fade = -4;
            w->alpha += w->fade;
            if (w->alpha <= 0) {
                return -1;
            }
            eneSetModelOnCourse(obj, &w->frame->f);
            if (w->puff > 0 && w->puff < 10 && (w->puff & 3) == 1) {
                itexPuffDark(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                             w->frame->scale[1] * 0.0700000003f);
            }
            w->puff--;
            w->rot[0] += 0.1745329201f;
            w->rot[1] += 0.08726646006f;
            w->rot[2] += 0.05235987902f;
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetRenderMode(gGfxDlHead++, 0xC81049D8, 0);
            gDPSetCombine(gGfxDlHead++, 0x1216A0, 0xFFFFFFF8);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A piece thrown from near (x, y, z). */
void sasrockPieceCreate(Obj *parent, f32 x, f32 y, f32 z) {
    Obj *o;
    PieceWork *w;

    if ((o = tskCreate(gEffectTask, sasrockPiece, parent->file, NULL)) != NULL) {
        (w = PIECE(o))->pos[0] = fadeRand(10) + x - 5.0f;
        w->pos[1] = y;
        w->pos[2] = fadeRand(10) + z - 5.0f;
        w->vel[0] = (fadeRand(40) - 20) / 10.0f;
        w->vel[2] = (fadeRand(40) - 20) / 10.0f;
        w->vel[1] = 4.0f;
        w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 1.0f;
        w->puff = 1;
    }
}

/* "sasrock" */
s32 sasrockProc(Obj *obj, s16 msg, void *arg) {
    SasrockWork *w = (SasrockWork *) FX(obj);
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "sasrock";
            w->rot[1] = (s32) eneSetup(obj, arg) * PI / 512;
            w->pol = eneNewPol(obj, sasrockDl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayer3);
            w->ride = rideCreate(gRideTree1, (s32) obj, &w->pol->f,
                                 (RideData *) eneFileAddr(obj, sasrockRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            w->dmg = ((DmgAddXYZFunc) acollDmgAddXYZ)(ene_dmg_root_frame, &w->pol->f, 100, 400, 0, 0, 0, 0);
            break;
        case 1:
            frmDestroy(&w->dmg->frame);
            frmDestroy(&w->ride->f);
            frmDestroy(&w->pol->f);
            break;
        case 0:
            if (w->dmg != NULL && acollDmgWasHit(w->dmg) == 1 && w->dmg->hitKind == 0xE) {
                soundIncamCam(0x15A, w->pol->pos[0], w->pol->pos[1], w->pol->pos[2]);
                for (i = 0; i < 3; i++) {
                    sasrockPieceCreate(obj, w->pol->pos[0], w->pol->pos[1] + 20.0f, w->pol->pos[2]);
                }
                return -1;
            }
            eneSetModelOnCourse(obj, &w->pol->f);
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
