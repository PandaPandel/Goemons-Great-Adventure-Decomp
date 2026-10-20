#include "tlb.h"

/* File 69: the tomosibi (a will-o'-the-wisp flame), the insect, the arijigoku (antlion pit),
 * the hmita and the kmace. */

/* The tomosibi's work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct TomosibiWork {
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
    /* 0x34 */ Anim anim; /* the glow's texture animation */
    /* 0x44 */ PolFrame *glow;
    /* 0x48 */ PolFrame *flame;
    /* 0x4C */ u16 timer;
} TomosibiWork;

/* The insect's work: an enemy's up to 0x34. */
typedef struct InsectWork {
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
    /* 0x34 */ PolFrame *body;
    /* 0x38 */ PolFrame *wing[2];
    /* 0x40 */ RideFrame *ride[2];
    /* 0x48 */ ACollFrame *box;
    /* 0x4C */ u16 timer;
    /* 0x4E */ u8 mode; /* 0 below, 1 rising, 2 wait, 3 opening, 4 dropping (the box live) */
    /* 0x50 */ f32 wingRot;
    /* 0x54 */ f32 baseY;
    /* 0x58 */ f32 ofs; /* its height over baseY */
} InsectWork;

/* The arijigoku's work: an enemy's up to 0x34. */
typedef struct ArijigokuWork {
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
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ s32 timer; /* scrolls the sand texture */
} ArijigokuWork;

/* The hmita's work: an enemy's up to 0x34. */
typedef struct HmitaWork {
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
    /* 0x34 */ PolFrame *body;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ PolFrame *wing[2];
    /* 0x44 */ RideFrame *wingRide[2];
    /* 0x4C */ ACollFrame *box; /* a root for the four wing boxes */
    /* 0x50 */ u16 timer;
    /* 0x52 */ u8 mode; /* 0 open, 1 shutting, 2 opening */
    /* 0x54 */ f32 wingRot;
} HmitaWork;

/* The kmace's work: an enemy's up to 0x34. */
typedef struct KmaceWork {
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
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ ACollFrame *box;
    /* 0x3C */ f32 spin[2]; /* added to rot[0] and rot[1] each step */
} KmaceWork;

/* Message 6's argument (plmove.c): a place on the course, moved by the object ridden. */
typedef struct PlPlace {
    /* 0x0 */ s16 zone;
    /* 0x4 */ f32 place;
    /* 0x8 */ f32 side;
    /* 0xC */ f32 height;
} PlPlace;

/* acollAtkAddXYZ and acollAtkAddYZ return their box (main declares them void). */
typedef ACollFrame *(*AtkAddXYZFunc)(Frame *parent, Frame *model, u16 radius, s16 ox, s16 oy, s16 oz,
                                     s32 a);
typedef ACollFrame *(*AtkAddYZFunc)(Frame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);

extern Frame *gFrmLayer3;
extern Gfx arijigokuDl[];
extern Gfx tomosibiGlowModelDl[];
extern Gfx tomosibiFlameModelDl[];
extern Gfx insectWingDl[];
extern Gfx hmitaWingDl[];
extern Gfx hmitaBodyDl[];
extern Gfx kmaceDl[];
extern u8 arijigokuRide[];
extern u8 hmitaBodyRide[];
extern u8 hmitaWingRide[];
extern RideData insectRide;
extern u8 arijigokuSandTex[];
Gfx tomosibiGlowDl[] = {
    gsDPPipeSync(),
    gsDPSetCombineLERP(PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, ENVIRONMENT, 0, TEXEL0, 0, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, ENVIRONMENT, 0, TEXEL0, 0),
    gsDPSetPrimColor(0, 0, 0xFF, 0xFF, 0x80, 0x00),
    gsDPSetEnvColor(0xFF, 0x00, 0x00, 0xFF),
    gsSPEndDisplayList(),
};
Gfx tomosibiFlameDl[] = {
    gsDPPipeSync(),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, ENVIRONMENT, 0, TEXEL0, 0, 0, 0, 0, ENVIRONMENT, ENVIRONMENT, 0, TEXEL0, 0),
    gsDPSetEnvColor(0xFF, 0xFF, 0x80, 0x40),
    gsSPEndDisplayList(),
};
Gfx arijigokuSandDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetCombineMode(G_CC_MODULATEI, G_CC_MODULATEI),
    gsSPEndDisplayList(),
};

/* A 32x32 RGBA16 texture (arijigokuSandTex) loaded into tile 0, scrolled down by t. */
Gfx *arijigokuSandGfx(Gfx *gfx, u16 t) {
    gDPPipeSync(gfx++);
    gDPSetTextureImage(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, arijigokuSandTex);
    gDPSetTile(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, 5,
               G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD);
    gDPLoadSync(gfx++);
    gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES));
    gDPPipeSync(gfx++);
    gDPSetTile(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, 5,
               G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD);
    gDPSetTileSize(gfx++, G_TX_RENDERTILE, 0, t, (32 - 1) << G_TEXTURE_IMAGE_FRAC,
                   (32 - 1) << G_TEXTURE_IMAGE_FRAC);
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* The tomosibi: a flame and its glow, both pol frames at its place, the flame pulsing in size.
 * Its data is the size in hundredths (0: 0.2, 5 higher). */
s32 tomosibiProc(Obj *o, s16 msg, void *arg) {
    TomosibiWork *w = (TomosibiWork *) FX(o);
    s32 pad;
    s32 data;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;

    switch (msg) {
        case 2:
            o->name = "tomosibi";
            data = eneSetup(o, arg);
            vramCoursePointXZ(w->zone, w->x, w->z, &x, &z);
            y = w->y;
            if (data == 0) {
                scale = 0.2f;
                y += 5.0f;
            } else {
                scale = data / 100.0f;
            }
            w->flame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->flame == NULL) {
                return -1;
            }
            w->flame->dl2 = tomosibiFlameModelDl;
            floadGetSetRam((void **) w->flame->segs, 0x120);
            w->flame->flags |= 2;
            w->flame->dl = (Gfx *) fadeSegToRam(tomosibiFlameDl, o->file);
            w->flame->pos[0] = x;
            w->flame->pos[1] = y;
            w->flame->pos[2] = z;
            w->flame->scale[0] = w->flame->scale[1] = w->flame->scale[2] = scale * 5.0f;
            w->glow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->glow == NULL) {
                return -1;
            }
            w->glow->dl2 = tomosibiGlowModelDl;
            eneGetSegs(o, w->glow->segs);
            w->glow->flags |= 2;
            w->glow->dl = (Gfx *) fadeSegToRam(tomosibiGlowDl, o->file);
            w->glow->pos[0] = x;
            w->glow->pos[1] = y;
            w->glow->pos[2] = z;
            w->glow->scale[0] = w->glow->scale[1] = w->glow->scale[2] = scale;
            fadeAnimStart(&w->anim, gItexTimedYellowAnim, 0, &w->glow->segs[3], eneGetFile(o, 0));
            break;
        case 1:
            frmDestroy(&w->flame->f);
            frmDestroy(&w->glow->f);
            break;
        case 0:
            w->timer++;
            if (w->timer & 2) {
                scale = 1.05f;
            } else {
                scale = 0.9523810148f;
            }
            w->flame->scale[0] *= scale;
            w->flame->scale[1] *= scale;
            w->flame->scale[2] *= scale;
            fadeAnimStep(&w->anim);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The insect: rises from below its place, opens its two wings and drops back, an attack box on
 * its body that grabs the player (action 0x2F, the player's model held at the box).
 * Matched 2026-10-12 (from real 99): insectRide passed as a pointer through a cast
 * eneFileAddr (IDO_NOTES 378's lever): real 6, the 8 FAKE splits no longer needed; then the
 * grabbed player's Player pointer as a block local "p = PL(pl)" set first and used for the flags
 * and the three model stores (the game rebuilds it, addiu v0,s0,0x20, after the calls; PL(pl)
 * at each use folds the 0x20, and p set after the calls folds it too; its word also moves the
 * spill temp to the game's 0x48): MATCH.
 * Was NON_MATCHING (269 lines, this C 368 for the game's 367, mix distance 3, the game's frame),
 * 2026-10-04:
 * - the game keeps eneFileAddr and rideCreate in s2/s3 across the setup loop's frmMove but
 *   builds insectRide's address in a1 at its use; this C builds it at the loop body's head and
 *   keeps it across frmMove (in s2 at first, spilled to 0x44 now), which moves every later
 *   t-register by one and adds the spill. The game builds no data address early anywhere in
 *   this file (hmitaProc matched only with its second hmitaWingRide under another name).
 * - measured: plain C 368 (s0-s6 saved, o/w in s6/s5); insectRide as a RideData scalar passed as
 *   &insectRide (368); "/ 512" for "/ 512.0f" (364, the game's div.s); the splits sweep's stack of
 *   8 FAKE boundaries, here after "w->mode = 1;" (297: s0-s5 as the game's, o/w in s5/s4; the
 *   same 301 at any of eight block ends); pad0 above x and pad[4] below z (269, frame 0x88).
 * - scratch (measured): IDO here CSEs every data symbol's address (array,
 *   scalar, struct, const, volatile, block-scope extern, "+ 0x2200" offset), keeps it across a
 *   call and hoists it in loops; a literal (0x3002200) is rebuilt at each use but with ori.
 * - 2026-10-12: `hoist` (eneFileAddr's result into a local, in the pad's word) 99, no
 *   change. In the game, eneFileAddr's and rideCreate's addresses are what move up above
 *   frmMove (lui s2/s3 inside the loop body), insectRide is built at its use.
 * - 2026-10-12 (second try, start 99): a block boundary before the ride's statement (a dead
 *   PRINTF, or `do {} while (0);`): 200, length +6; insectRide spelled as an offset from another
 *   symbol (&hmitaBodyRide[-0x150], the 0E44 lever): 99, the same code. cc -S of this C: uopt
 *   takes the address as a register candidate with eneFileAddr and rideCreate (la $11 at the
 *   body's head, then sw $11,68($sp), spilled across frmMove). A probe:
 *   a struct, an array and the result through a local are all candidates (a saved register
 *   there); only a literal 0x3002030 is not, and it is built lui/ori, not the game's lui/addiu. */
s32 insectProc(Obj *o, s16 msg, void *arg) {
    InsectWork *w = (InsectWork *) FX(o);
    s32 data;
    s32 i;
    Obj *pl;
    s32 pad0;
    f32 x;
    f32 z;
    s32 pad[4];

    switch (msg) {
        case 2:
            o->name = "insect";
            data = eneSetup(o, arg);
            w->rot[1] = data * 3.141592741f / 512;
            w->baseY = w->y;
            w->body = eneNewPol(o, NULL);
            if (w->body == NULL) {
                return -1;
            }
            w->body->scale[0] = w->body->scale[1] = w->body->scale[2] = 0.15f;
            w->box = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->body->f, 50, 125, 0, 0);
            if (w->box == NULL) {
                return -1;
            }
            w->box->flags &= ~2;
            for (i = 0; i < 2; i++) {
                w->wing[i] = eneNewPol(o, insectWingDl);
                if (w->wing[i] == NULL) {
                    return -1;
                }
                frmMove(&w->wing[i]->f, gFrmLayer3);
                w->wing[i]->scale[0] = w->body->scale[0];
                w->wing[i]->scale[1] = w->body->scale[1];
                w->wing[i]->scale[2] = w->body->scale[2];
                /* insectRide passed as a pointer (eneFileAddr takes a void *): its address is built
                 * at the call, as the game does. */
                w->ride[i] = rideCreate(gRideTree1, (s32) o, &w->wing[i]->f,
                                        (RideData *) eneFileAddr(o, &insectRide, 0));
                if (w->ride == NULL) {
                    return -1;
                }
            }
            break;
        case 1:
            for (i = 0; i < 2; i++) {
                frmDestroy(&w->ride[i]->f);
                frmDestroy(&w->wing[i]->f);
            }
            frmDestroy(&w->box->frame);
            frmDestroy(&w->body->f);
            break;
        case 0:
            w->timer++;
            switch (w->mode) {
                case 0:
                    w->ofs = w->body->scale[1] * -300.0f;
                    w->wingRot = -0.7853981853f;
                    if (w->timer > 30) {
                        w->timer = 0;
                        w->mode = 1;
                    }
                    break;
                case 1:
                    w->ofs += 300.0f * w->body->scale[1] / 60.0f;
                    if (w->timer > 60) {
                        w->timer = 0;
                        w->mode = 2;
                    }
                    break;
                case 2:
                    if (w->timer > 16) {
                        w->timer = 0;
                        w->mode = 3;
                    }
                    break;
                case 3:
                    w->wingRot += 0.09817477316f;
                    if (w->timer > 8) {
                        soundIncamCam(0x2D5, w->body->pos[0], w->body->pos[1], w->body->pos[2]);
                        w->timer = 0;
                        w->mode = 4;
                    }
                    break;
                case 4:
                    w->ofs -= 300.0f * w->body->scale[1] / 60.0f;
                    if (w->timer == 30) {
                        w->box->flags |= 2;
                    }
                    if (w->timer > 60) {
                        w->box->flags &= ~2;
                        w->timer = 0;
                        w->mode = 0;
                    }
                    break;
            }
            w->y = w->baseY + w->ofs;
            eneSetModelOnCourse(o, &w->body->f);
            mathXZRotateY(w->rot[1], w->body->scale[2] * 75.0f, 0.0f, &x, &z);
            w->wing[0]->rot[0] = w->wingRot;
            w->wing[0]->rot[1] = w->rot[1] - 1.570796371f;
            w->wing[0]->pos[0] = w->x + x;
            w->wing[0]->pos[2] = w->z + z;
            w->wing[0]->pos[1] = w->y;
            w->wing[1]->rot[0] = w->wingRot;
            w->wing[1]->rot[1] = w->rot[1] + 1.570796371f;
            w->wing[1]->pos[0] = w->x - x;
            w->wing[1]->pos[2] = w->z - z;
            w->wing[1]->pos[1] = w->y;
            if (acollAtkWasHit(w->box) == 1) {
                pl = (Obj *) w->box->hitPower;
                if (pl != NULL) {
                    Player *p = PL(pl);

                    p->flags |= 0x10000000;
                    if (plGetAction(pl) != 0x2F) {
                        plSetAction(pl, 0x2F);
                    }
                    p->model->pos[0] = w->box->pos[0];
                    p->model->pos[1] = w->box->pos[1];
                    p->model->pos[2] = w->box->pos[2];
                }
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The arijigoku: an antlion pit (a ride) that pulls whoever stands in it down and toward its
 * middle, and slows a player at the same side; its texture scrolls with the timer. */
s32 arijigokuProc(Obj *o, s16 msg, void *arg) {
    ArijigokuWork *w = (ArijigokuWork *) FX(o);
    s32 data;
    s32 i;
    s32 pad;
    f32 ang;
    f32 x;
    f32 y;
    f32 z;
    f32 pos[3];
    s32 pad2;
    s32 zone;

    switch (msg) {
        case 2:
            o->name = "arijigoku";
            data = eneSetup(o, arg);
            w->pol = eneNewPol(o, arijigokuDl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmEffectLayer);
            w->pol->rot[1] = data * 3.141592741f / 512;
            w->ride = rideCreate(gRideTree1, (s32) o, &w->pol->f, (RideData *) eneFileAddr(o, arijigokuRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            break;
        case 1:
            frmDestroy(&w->ride->f);
            frmDestroy(&w->pol->f);
            break;
        case 7:
            if (arg != NULL) {
                *(s32 *) arg = 6;
            }
            break;
        case 6:
            if (arg != NULL) {
                ang = vramCourseHeading(((PlPlace *) arg)->zone, ((PlPlace *) arg)->place) - 1.570796371f;
                vramCoursePointXZ(((PlPlace *) arg)->zone, ((PlPlace *) arg)->place, 0.0f, &x, &z);
                x -= w->pol->pos[0];
                y = ((PlPlace *) arg)->height - w->pol->pos[1];
                z -= w->pol->pos[2];
                mathVecRotateY(-ang, &x, &y, &z);
                if (1.0f < x) {
                    ((PlPlace *) arg)->place -= 1.0f;
                    ((PlPlace *) arg)->height -= 0.2680000067f;
                } else if (x < -1.0f) {
                    ((PlPlace *) arg)->place += 1.0f;
                    ((PlPlace *) arg)->height -= 0.2680000067f;
                } else {
                    ((PlPlace *) arg)->height -= 0.2680000067f;
                }
                for (i = 0; i < 4; i++) {
                    if (plIsInPlay(i)) {
                        plAskPos(gGameWork.players[i], &pos[0], &pos[1], &pos[2], &zone);
                        if (fabsf(((PlPlace *) arg)->side - pos[2]) < 1) {
                            plSlowDown(i);
                            break;
                        }
                    }
                }
            }
            break;
        case 0:
            w->timer++;
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, fadeSegToRam(arijigokuSandDl, o->file));
            gGfxDlHead = arijigokuSandGfx(gGfxDlHead, w->timer);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* An attack box of radius 60 on the model, x across and (25, 219) up and along; kind 120. */
ACollFrame *hmitaWingBoxAdd(Frame *parent, Frame *model, f32 x) {
    ACollFrame *box = ((AtkAddXYZFunc) acollAtkAddXYZ)(parent, model, 60, x, 25, 219, 0);

    if (box == NULL) {
        return NULL;
    }
    box->kind = 120;
    return box;
}

/* The hmita: a body and two wings (pol frames, each a ride) that flap shut and open again; the
 * four boxes on the wings send message 7 to what they hit. */
s32 hmitaProc(Obj *o, s16 msg, void *arg) {
    HmitaWork *w = (HmitaWork *) FX(o);
    s32 data;
    ACollFrame *c;
    Obj *by;
    s32 pad;
    f32 x;
    f32 z;

    switch (msg) {
        case 2:
            o->name = "hmita";
            data = eneSetup(o, arg);
            w->rot[1] = data * 3.141592741f / 512;
            w->body = eneNewPol(o, hmitaBodyDl);
            w->wing[0] = eneNewPol(o, hmitaWingDl);
            w->wing[1] = eneNewPol(o, hmitaWingDl);
            if (w->body == NULL || w->wing[0] == NULL || w->wing[1] == NULL) {
                return -1;
            }
            frmMove(&w->body->f, gFrmLayer3);
            frmMove(&w->wing[0]->f, gFrmLayer3);
            frmMove(&w->wing[1]->f, gFrmLayer3);
            w->ride = rideCreate(gRideTree1, (s32) o, &w->body->f, (RideData *) eneFileAddr(o, hmitaBodyRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            w->wingRide[0] =
                rideCreate(gRideTree1, (s32) o, &w->wing[0]->f, (RideData *) eneFileAddr(o, hmitaWingRide, 0));
            /* the rides' data passed as pointers (eneFileAddr takes a void *): the game builds
             * hmitaWingRide's address afresh for each call */
            w->wingRide[1] = rideCreate(gRideTree1, (s32) o, &w->wing[1]->f,
                                        (RideData *) eneFileAddr(o, hmitaWingRide, 0));
            w->box = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->body->f, 1, 0, 0, 0);
            if (w->box == NULL) {
                return -1;
            }
            hmitaWingBoxAdd(&w->box->frame, &w->wing[0]->f, 60.0f);
            hmitaWingBoxAdd(&w->box->frame, &w->wing[0]->f, -60.0f);
            hmitaWingBoxAdd(&w->box->frame, &w->wing[1]->f, 60.0f);
            hmitaWingBoxAdd(&w->box->frame, &w->wing[1]->f, -60.0f);
            break;
        case 1:
            frmDestroy(&w->wingRide[1]->f);
            frmDestroy(&w->wingRide[0]->f);
            frmDestroy(&w->ride->f);
            frmDestroy(&w->box->frame);
            frmDestroy(&w->wing[1]->f);
            frmDestroy(&w->wing[0]->f);
            frmDestroy(&w->body->f);
            break;
        case 0:
            w->timer++;
            switch (w->mode) {
                case 0:
                    if (w->timer > 60) {
                        w->timer = 0;
                        w->mode = 1;
                    }
                    break;
                case 1:
                    w->wingRot -= 0.1308996975f;
                    if (w->timer > 11) {
                        soundIncamCam(0x2D0, w->body->pos[0], w->body->pos[1], w->body->pos[2]);
                        w->timer = 0;
                        w->mode = 2;
                    }
                    break;
                case 2:
                    w->wingRot += 0.05235987902f;
                    if (w->timer > 29) {
                        w->timer = 0;
                        w->mode = 0;
                    }
                    break;
            }
            eneSetModelOnCourse(o, &w->body->f);
            mathXZRotateY(w->rot[1], -7.5f, 0.0f, &x, &z);
            w->wing[0]->rot[0] = w->wingRot;
            w->wing[0]->rot[1] = w->rot[1] - 1.570796371f;
            w->wing[0]->pos[0] = w->x + x;
            w->wing[0]->pos[2] = w->z + z;
            w->wing[0]->pos[1] = w->y - 2.099999905f;
            w->wing[1]->rot[0] = w->wingRot;
            w->wing[1]->rot[1] = w->rot[1] + 1.570796371f;
            w->wing[1]->pos[0] = w->x - x;
            w->wing[1]->pos[2] = w->z - z;
            w->wing[1]->pos[1] = w->y - 2.099999905f;
            for (c = (ACollFrame *) w->box->frame.child; c != NULL; c = (ACollFrame *) c->frame.next) {
                if (acollAtkWasHit(c) == 1) {
                    by = (Obj *) c->hitPower;
                    if (by != NULL) {
                        tskSendMsg(by, 7, o);
                    }
                }
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The kmace: a spiked mace turning about x (data 0, its heading in the high half) or about y
 * (data 1); what its box hits gets message 7. */
s32 kmaceProc(Obj *o, s16 msg, void *arg) {
    KmaceWork *w = (KmaceWork *) FX(o);
    s32 data;
    Obj *by;

    switch (msg) {
        case 2:
            o->name = "kmace";
            data = eneSetup(o, arg);
            switch ((u8) data) {
                case 0:
                    w->spin[0] = 0.06981316954f;
                    w->spin[1] = 0.0f;
                    w->rot[1] = (data >> 16) * 3.141592741f / 512;
                    break;
                case 1:
                    w->spin[0] = 0.0f;
                    w->spin[1] = 0.06981316954f;
                    break;
            }
            w->pol = eneNewPol(o, kmaceDl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayer3);
            w->box = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->pol->f, 100, 0, 314, 0);
            if (w->box == NULL) {
                return -1;
            }
            w->box->kind = 120;
            break;
        case 1:
            frmDestroy(&w->box->frame);
            frmDestroy(&w->pol->f);
            break;
        case 0:
            w->rot[0] += w->spin[0];
            w->rot[1] += w->spin[1];
            eneSetModelOnCourse(o, &w->pol->f);
            if (w->box != NULL && acollAtkWasHit(w->box) == 1) {
                by = (Obj *) w->box->hitPower;
                if (by != NULL) {
                    tskSendMsg(by, 7, o);
                }
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
