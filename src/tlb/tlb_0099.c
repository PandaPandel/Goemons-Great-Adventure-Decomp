#include "tlb.h"

#define DPRINTF

/* The work of the "tanu_j" task (its proc tanuJProc): the rider. */
typedef struct Tlb99RiderW {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 course;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 dist; /* along the course */
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 off; /* off the course */
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ Camera *cam;
    /* 0x38 */ RideFrame *rideRight;
    /* 0x3C */ RideFrame *rideLeft;
    /* 0x40 */ PolFrame *right;
    /* 0x44 */ PolFrame *left;
    /* 0x48 */ u32 count;
    /* 0x4C */ u8 mode; /* 0 waiting (tanuJWait), 1 after (tanuJAfterAttack) */
    /* 0x4D */ u8 done;
    /* 0x4E */ u8 plState; /* 0 no player near, 1 the side is set, 2 a player on it */
    /* 0x50 */ f32 side; /* 85 or -85 */
    /* 0x54 */ u32 setup; /* eneSetup's */
    /* 0x58 */ ACollFrame *atk; /* its attack box while a player is on it ("たぬきあたっく") */
} Tlb99RiderW;

void tanuJPlaceFrame(Obj *o, PolFrame *frame, f32 d);
void tanuJWait(Obj *o);
void tanuJClearEffects(void);
void tanuJAfterAttack(Obj *o);
s32 tanuJCamTurn(Obj *o);

/* acollAtkAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy, s16 oz, s32 a);


/* "tanu" (its proc tanuProc): its work, an enemy's (EneObj) up to 0x38. */
typedef struct Tlb99W {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 course;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 unused09;
    /* 0x0A */ u8 hit; /* 1 on the ground */
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 dist; /* along the course */
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 off; /* off the course */
    /* 0x1C */ f32 rotX;
    /* 0x20 */ f32 rotY;
    /* 0x24 */ f32 rotZ;
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *dmg;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ PolFrame *jump; /* "tanu_j" */
    /* 0x40 */ MPolFrame *shadow;
    /* 0x44 */ u32 stepTimer; /* the step's frame count */
    /* 0x48 */ f32 phase; /* step 4's wobble */
    /* 0x4C */ Tlb99Data *data;
    /* 0x50 */ Obj *prize; /* the tegata (kind 0xF7) it makes in step 3 */
    /* 0x54 */ s8 step;
    /* 0x55 */ u8 subTimer;
    /* 0x56 */ u8 sub;
} Tlb99W;

/* A "piece"'s argument (tanuPieceCreate fills it, tanuPieceProc reads it). */
typedef struct Tlb99PieceArg {
    /* 0x00 */ u8 pad00[0x24];
    /* 0x24 */ f32 pos[3];
    /* 0x30 */ f32 scale;
    /* 0x34 */ u8 pad34[0x10];
} Tlb99PieceArg; /* size 0x44 */

/* "wall" (tanuWallProc): its work. */
typedef struct Tlb99WallW {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 course;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 dist;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 off;
    /* 0x1C */ f32 rotX;
    /* 0x20 */ f32 rotY;
    /* 0x24 */ f32 rotZ;
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ RideFrame *ride;
} Tlb99WallW;

extern u8 tanuJRide[]; /* the ride data of the file set 0x34 */

/* "tanu_eff" (tanuEffProc): its work, an enemy's (EneObj) up to 0x34. */
typedef struct Tlb99EffW {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 course;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 dist;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 off;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ s16 alpha;
    /* 0x36 */ u8 mode; /* 0 with an argument, 1 without */
    /* 0x37 */ u8 r;
    /* 0x38 */ u8 g;
    /* 0x39 */ u8 b;
    /* 0x3C */ u32 setup; /* eneSetup's */
    /* 0x40 */ f32 grow; /* mode 1's scale step */
    /* 0x44 */ u16 step; /* its frame count */
} Tlb99EffW;

extern Gfx tanuEffPuffDl[]; /* the puff's display list */

/* "koban" (tanuKobanProc): its work, an enemy's (EneObj) up to 0x38. */
typedef struct Tlb99KobanW {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 unused09;
    /* 0x0A */ u8 hit; /* 1 on the ground */
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ Anim anim; /* its spin (coinSpinAnim) */
    /* 0x48 */ s16 alpha;
    /* 0x4A */ u8 blink; /* set: it blinks out */
    /* 0x4B */ u8 count; /* blink frames left */
} Tlb99KobanW;

/* acollAtkAddYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);

extern Gfx tanuPieceDl[];
/* the coin's spin, in file set 0xF (file 469); 0 and 2 are tanuPieceDl's and tanuEffPuffDl's
 * addresses in other files */
extern u8 coinSpinTex0[];
extern u8 coinSpinTex1[];
extern u8 coinSpinTex2[];
extern u8 coinSpinTex4[];
extern u8 coinSpinTex5[];
extern u8 coinSpinTex6[];
AnimFrame coinSpinAnim[] = {
    { (u32) coinSpinTex0, 2 },
    { (u32) coinSpinTex1, 2 },
    { (u32) coinSpinTex2, 2 },
    { 0x03000700, 2 },
    { (u32) coinSpinTex4, 2 },
    { (u32) coinSpinTex5, 2 },
    { (u32) coinSpinTex6, 2 },
    { 0x03000F00, 2 },
    { 0, 0 },
    { 0, 0 },
}; /* the coin's spin: eight textures */
extern Gfx tanuKobanCoinDl[];                 /* the coin's display list */

/* "tegata" (Eprg_tanu): its work, an enemy's (EneObj) up to 0x38. */
typedef struct Tlb99TegataW {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 course;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 dist;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 off;
    /* 0x1C */ f32 rotX;
    /* 0x20 */ f32 rotY;
    /* 0x24 */ f32 rotZ;
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ Obj *rider; /* kind 0xF5, made under it */
    /* 0x3C */ s32 count;
    /* 0x40 */ f32 accel; /* added to vel[1] */
    /* 0x44 */ f32 spin; /* added to rotY */
    /* 0x48 */ f32 y0; /* where it began */
    /* 0x4C */ f32 sparkle;
    /* 0x50 */ u8 state; /* 0 rising, 1 sinking, 2 bobbing */
} Tlb99TegataW;

extern Gfx eprgTanuHandPrintDl[]; /* the hand print's display list */

/* A "piece"'s work (tanuPieceProc). */
typedef struct Tlb99PieceW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ f32 fall; /* the speed it has fallen by */
    /* 0x14 */ u8 pad14[0x24];
    /* 0x38 */ s32 unused38; /* written 0x40 at its start, never read */
    /* 0x3C */ s32 alpha;
    /* 0x40 */ u8 pad40[0x2];
    /* 0x42 */ u8 state; /* 0 thrown, 1-3 bounces at -1000 */
} Tlb99PieceW;

extern Gfx tanuPieceDl[]; /* the piece's display list (file set 0x11E) */

s32 tanuPieceProc(Obj *o, s16 msg, void *arg);
void tanuStepReset(Obj *o);
void tanuFootDust(Obj *o, f32 side);
void tanuWalkDust(Obj *o);
s32 tanuAngleToDir(Obj *o, f32 angle);
s32 tanuTegataIsTaken(Obj *o);
void tanuPieceCreate(Obj *o, f32 x, f32 y, f32 z, f32 scale, s32 spread);

/* acollDmgAddYZ returns its box (main declares it void). */
typedef ACollFrame *(*DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                    s32 a);

void tanuHopStep(Obj *o);
void tanuGrowStep(Obj *o);
void tanuHitStep(Obj *o);
void tanuTegataStep(Obj *o);
void tanuZakuStep(Obj *o);
void tanuEndStep(Obj *o);
void tanuPlaceFrame(Obj *o, PolFrame *frame);

extern Gfx tanuDl[]; /* tanu's display list */
extern u8 tanuRide[];  /* tanu's ride data */
extern Gfx tanuHitStepDl[];
extern s32 tanuZakuCount; /* this file's .bss: the count of kind 0xF6 made (its mode, odd/even) */
extern s32 tanuZakuCountInc; /* the same count, as step 4 stores its increment (undefined_syms.txt) */

/* "tanu_j": the rider (a model and two frames to ride, left and right); waits for a player
 * (tanuJWait), then throws (tanuJAfterAttack). tanuJRide's second use is
 * cast (s32), the first (u32): IDO builds the address again across rideCreate, as the game
 * does (with one expression it keeps it in a temp). */
s32 tanuJProc(Obj *o, s16 msg, void *arg) {
    s32 pad;
    Tlb99RiderW *w = (Tlb99RiderW *) FX(o);

    switch (msg) {
        case 2:
            o->name = "tanu_j";
            w->setup = eneSetup(o, arg);
            w->model = eneNewPol(o, NULL);
            if (w->model == NULL) {
                return -1;
            }
            w->cam = camAiGetCam();
            if (w->cam == NULL) {
                PRINTF("dcam is NULL!\n");
            }
            floadLoadSet(0x34);
            w->right = eneNewPol(o, NULL);
            floadGetSetRam((void **) w->right->segs, 0x33);
            if (w->right == NULL) {
                return -1;
            }
            w->left = eneNewPol(o, NULL);
            floadGetSetRam((void **) w->left->segs, 0x33);
            if (w->left == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                PRINTF("tanu: SET MISS\n");
                return -1;
            }
            w->rideLeft = rideCreate(gRideTree1, (s32) o, &w->left->f,
                                     (RideData *) floadSegToRam((u32) tanuJRide, 0x34, 0));
            if (w->rideLeft == NULL) {
                return -1;
            }
            w->rideRight = rideCreate(gRideTree1, (s32) o, &w->right->f,
                                      (RideData *) floadSegToRam((s32) tanuJRide, 0x34, 0));
            if (w->rideRight == NULL) {
                return -1;
            }
            w->count = 0;
            w->done = 0;
            w->mode = 0;
            break;
        case 1:
            camAiSetHold(0);
            if (w->rideRight != NULL) {
                frmDestroy(&w->rideRight->f);
            }
            if (w->rideLeft != NULL) {
                frmDestroy(&w->rideLeft->f);
            }
            if (w->right != NULL) {
                frmDestroy(&w->right->f);
            }
            if (w->left != NULL) {
                frmDestroy(&w->left->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            break;
        case 3:
            break;
        case 0:
            if (w->mode == 0) {
                tanuJWait(o);
            } else {
                tanuJAfterAttack(o);
            }
            break;
    }
    if (w->model != NULL) {
        eneSetModelOnCourse(o, &w->model->f);
    }
    return tskDefaultProc(o, msg, arg);
}

/* The rider waiting (mode 0): picks its side when a player comes near; with a player on it, turns
 * the camera, then makes kind 0xF5 and goes to mode 1; holds the camera while a player is on it,
 * and keeps an attack box on itself. */
void tanuJWait(Obj *o) {
    s32 pad;
    Tlb99RiderW *w = (Tlb99RiderW *) FX(o);
    s32 pl = shimPlNearCheck(o, 80.0f);

    if (pl != 0 && w->plState == 0) {
        if (shimPlSideCheck(o, shimNearestPl(o)) == 4) {
            w->side = 85.0f;
        } else {
            w->side = -85.0f;
        }
        w->plState = 1;
    }
    if (pl == 2) {
        if (w->done == 0) {
            if (tanuJCamTurn(o) == 1) {
                PRINTF("たぬきあたっく発生\n");
                tanuJClearEffects();
                eneMakeEx(NULL, ENE_TANU_EFF, w->course, w->dist, w->y + 80.0f, w->off, 2,
                          (u32 (*)(void *, void *, void *)) w->setup);
                w->done = 1;
                w->mode = 1;
                w->count = 0;
            } else {
                tanuJPlaceFrame(o, w->left, 85.0f);
                tanuJPlaceFrame(o, w->right, 85.0f);
            }
            if (w->atk == NULL) {
                w->atk = ((AtkAddXYZFunc) acollAtkAddXYZ)(pl_atk_root_frame, &w->model->f, 2000, 0, 150, 0, 0);
                w->atk->kind = 0x1F;
            }
        }
        camAiSetHold(1);
        w->plState = 2;
    } else if (pl == 1 && w->plState == 1) {
        tanuJPlaceFrame(o, w->left, 85.0f);
        tanuJPlaceFrame(o, w->right, 85.0f);
        camAiSetHold(0);
        w->count = 0;
    } else {
        if (w->atk != NULL) {
            w->atk = (ACollFrame *) frmDestroy(&w->atk->frame);
        }
        w->plState = 0;
        w->count = 0;
        tanuJPlaceFrame(o, w->left, 85.0f);
        tanuJPlaceFrame(o, w->right, 85.0f);
        camAiSetHold(0);
    }
}

/* Stops enemy spawning, clears the effects, waits for the pending frame swaps, then frees file
 * set 0x11F. */
void tanuJClearEffects(void) {
    eneEnableSpawn(0);
    PRINTF("tanu : tskDestroyChild START\n");
    tskDestroyChildren(gEffectTask);
    PRINTF("tanu : duCheckDraw START\n");
    do {
    } while (gfxGetPendingSwaps() != 0);
    PRINTF("tanu : packetFree START\n");
    floadFreeSet(0x11F);
    PRINTF("tanu : ALL OK\n");
}

/* The rider after it has thrown (mode 1): at count 30 its attack box goes; counts on; places its
 * two frames 95 ahead and 95 behind on the course; holds the camera. */
void tanuJAfterAttack(Obj *o) {
    s32 pad;
    Tlb99RiderW *w = (Tlb99RiderW *) FX(o);

    if (w == NULL) { /* FAKE: keeps w a pointer (unfolded into obj's offsets), as the game */
    }
    if (w->count == 30) {
        if (w->atk != NULL) {
            frmDestroy(&w->atk->frame);
            PRINTF("たぬきあたっく削除\n");
        }
    }
    w->count++;
    tanuJPlaceFrame(o, w->left, 95.0f);
    tanuJPlaceFrame(o, w->right, -95.0f);
    camAiSetHold(1);
}

/* On its first frame turns the camera to look along the course at the rider; returns 1 after 31
 * frames (and restarts the count). */
s32 tanuJCamTurn(Obj *o) {
    s32 pad;
    Tlb99RiderW *w = (Tlb99RiderW *) FX(o);
    f32 ex;
    f32 ey;
    f32 ez;
    f32 ax;
    f32 ay;
    f32 az;

    if (w->count == 0) {
        vramCoursePointXZ(w->course, w->dist, w->off + 180.0f, &ex, &ez);
        ey = w->y + 40.0f;
        vramCoursePointXZ(w->course, w->dist, w->off, &ax, &az);
        ay = w->y + 20.0f;
        camSetSpeed(w->cam, 30.0f);
        camMoveEye(w->cam, ex, ey, ez);
        camMoveAt(w->cam, ax, ay, az);
    }
    if (w->count > 30) {
        w->count = 0;
        return 1;
    }
    w->count++;
    return 0;
}

/* Places frame on the enemy's course, d further along, 20 lower, tilted as the enemy and turned
 * to the course. */
void tanuJPlaceFrame(Obj *o, PolFrame *frame, f32 d) {
    s32 pad;
    EneObj *w = ENE(o);

    vramCoursePoint(ENE(o)->zone, ENE(o)->x + d, ENE(o)->z, &frame->pos[0], &frame->pos[1], &frame->pos[2]);
    frame->pos[1] -= 20.0f;
    frame->rot[0] = w->rot[0];
    frame->rot[1] = eneCourseAngle(o, 8);
    frame->rot[2] = w->rot[2];
}

/* "tanu_eff": the puff tanu appears in. Mode 0 (made with an argument): at frame 1 makes kind
 * 0xF4 above itself, then from frame 5 grows to scale 4 and fades out. Mode 1: a pulsing,
 * turning cloud whose colour changes every 10 frames. */
s32 tanuEffProc(Obj *o, s16 msg, void *arg) {
    s32 pad;
    Tlb99EffW *w = (Tlb99EffW *) FX(o);

    switch (msg) {
        case 2:
            o->name = "tanu_eff";
            w->setup = eneSetup(o, arg);
            if (w->setup != 0) {
                w->mode = 0;
            } else {
                w->mode = 1;
            }
            w->model = enePolModelMake(o, tanuEffPuffDl, eneDlEnvPrimColor(o, 0, 0, 0, 0, 0, 0, 0), 1);
            if (w->model == NULL) {
                return -1;
            }
            w->model->dl2 = NULL;
            w->model->flags |= 3;
            w->model->scale[2] = 3.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->r = w->g = w->b = 255;
            w->alpha = 255;
            w->step = 0;
            shimEneSetLowFlags(o, 3);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
        case 3:
            w->model->dl = eneDlEnvPrimColor(o, w->r, w->g, w->b, w->r, w->g, w->b, w->alpha);
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->mode) {
                case 0:
                    if (w->step == 1) {
                        eneMakeEx(NULL, ENE_TANU, w->course, w->dist, w->y + 10.0f, w->off, 2,
                                  (u32 (*)(void *, void *, void *)) w->setup);
                        sndPlay(0x11F);
                    }
                    if (w->step >= 5) {
                        w->model->dl2 = tanuEffPuffDl;
                        w->model->scale[2] = 4.0f;
                        w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                        w->alpha -= 10;
                        if (w->alpha < 0) {
                            return -1;
                        }
                        eneSetModelOnCourse(o, &w->model->f);
                        w->step = 5;
                    }
                    break;
                case 1:
                    if (w->step == 0) {
                        w->model->dl2 = tanuEffPuffDl;
                        w->model->scale[2] = 1.0f;
                        w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                        w->grow = 0.02f;
                        w->alpha = 160;
                    }
                    w->model->scale[2] += w->grow;
                    w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                    w->model->rot[2] += 0.349065860077601542;
                    if (w->model->scale[0] > 1.3 || w->model->scale[0] < 1.0) {
                        w->grow *= -1.0;
                    }
                    if (w->step % 10 == 0) {
                        w->r = fadeRandRange(128, 255);
                        w->g = fadeRandRange(128, 255);
                        w->b = fadeRandRange(128, 255);
                    }
                    break;
            }
            w->step++;
            eneStepHookPost(o);
            PRINTF("EFF step [%d]\n", w->step);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "tanu": the tanuki that turns into a pot of coins; its steps: 0 hops (tanuHopStep),
 * 1 grows and spins (tanuGrowStep), 2 hit (tanuHitStep), 3 the tegata
 * scene (tanuTegataStep), 4 the coins (tanuZakuStep), 5 the end
 * (tanuEndStep). */
s32 tanuProc(Obj *o, s16 msg, void *arg) {
    Tlb99W *w = (Tlb99W *) FX(o);
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "tanu";
            w->data = (Tlb99Data *) stageAddrToRam(eneSetup(o, arg));
            PRINTF("area %d point %d connect [%d][%d][%d][%d] tegata %d\n", w->data->area, w->data->point,
                   w->data->connect[0], w->data->connect[1], w->data->connect[2], w->data->connect[3],
                   w->data->tegata);
            floadLoadSet(0x33);
            w->model = eneNewPol(o, tanuDl);
            if (w->model == NULL) {
                return -1;
            }
            floadGetSetRam((void **) w->model->segs, 0x33);
            w->jump = eneNewPol(o, NULL);
            if (w->jump == NULL) {
                return -1;
            }
            floadGetSetRam((void **) w->jump->segs, 0x33);
            w->model->flags |= 0x20;
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->model->f, 120, 300, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->ride = rideCreate(gRideTree1, (s32) o, &w->jump->f,
                                 (RideData *) floadSegToRam((u32) tanuRide, 0x33, 0));
            w->shadow = shadowCreate();
            w->step = 0;
            tanuStepReset(o);
            fadeCreate(gEffectTask);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            if (w->ride != NULL) {
                frmDestroy(&w->ride->f);
            }
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->step) {
                case 0:
                    tanuHopStep(o);
                    break;
                case 1:
                    tanuGrowStep(o);
                    break;
                case 2:
                    tanuHitStep(o);
                    break;
                case 3:
                    tanuTegataStep(o);
                    break;
                case 4:
                    tanuZakuStep(o);
                    break;
                case 5:
                    tanuEndStep(o);
                    break;
                default:
                    PRINTF("tanu step error!\n");
                    break;
            }
            eneFall(o, 0.2f);
            eneMoveAndCollide(o, 10.0f, 2.0f, 0.0f, 3.0f);
            eneSetModelOnCourse(o, &w->model->f);
            tanuPlaceFrame(o, w->jump);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* tanu step 0: turned to the course and scaled on its first frame; hops (sound 0x23B) each time
 * it lands, and goes to step 1 when it lands from a hop. */
void tanuHopStep(Obj *o) {
    s32 pad;
    Tlb99W *w = (Tlb99W *) FX(o);

    if (w->stepTimer == 0) {
        w->rotY = eneCourseAngle(o, 8);
        w->model->scale[2] = 0.04f;
        w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
    }
    w->stepTimer++;
    if (w->sub == 1) {
        eneFall(o, 0.2f);
    }
    if (w->hit & 1) {
        if (w->sub == 1) {
            tanuStepReset(o);
            w->step = 1;
            return;
        }
        sndPlay(0x23B);
        w->sub = 1;
        w->vel[1] = 3.0f;
    }
}

/* tanu step 1: walks (tanuWalkDust); grows to scale 0.1 (sound 0x15D), waits 11
 * frames (sound 0x24F), then spins, puffing dust every 10 frames, until it is hit after 31: step
 * 2, its box lower. */
void tanuGrowStep(Obj *o) {
    s32 pad;
    Tlb99W *w = (Tlb99W *) FX(o);

    if (0) { /* FAKE: keeps w a pointer (unfolded into obj's offsets) and spends t6, as the game */
    }
    if (w->stepTimer % 10 == 0 && w->sub == 3) {
        tanuFootDust(o, 1.0f);
        tanuFootDust(o, -1.0f);
    }
    tanuWalkDust(o);
    w->stepTimer++;
    switch (w->sub) {
        case 0:
            sndPlay(0x15D);
            w->sub = 1;
            break;
        case 1:
            w->model->scale[1] += (f32) w->subTimer * 0.001;
            w->model->scale[0] = w->model->scale[2] = w->model->scale[1];
            w->subTimer++;
            if (w->model->scale[0] > 0.1) {
                w->model->scale[2] = 0.1f;
                w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                w->sub = 2;
                w->subTimer = 0;
            }
            break;
        case 2:
            if (++w->subTimer >= 11) {
                w->sub = 3;
                w->subTimer = 0;
                sndPlay(0x24F);
            }
            break;
        case 3:
            w->rotY -= 0.349065860077601542;
            if (++w->subTimer >= 60) {
                w->subTimer = 36;
            }
            if (acollDmgWasHit(w->dmg) == 1 && w->subTimer >= 31) {
                tanuStepReset(o);
                w->step = 2;
                w->dmg->height = 180;
            }
            break;
    }
}

/* tanu step 2, hit: five pieces fly off, the model takes its second list, a sound; after 6 frames
 * step 3, or step 4 once its tegata flag is set. */
void tanuHitStep(Obj *o) {
    Tlb99W *w = (Tlb99W *) FX(o);
    s32 i;

    if (w->stepTimer == 0) {
        for (i = 0; i < 5; i++) {
            (*tanuPieceCreate)(o, w->model->pos[0], w->model->pos[1] + 20.0f, w->model->pos[2], 1.0f, 3);
        }
        w->model->dl2 = tanuHitStepDl;
        w->jump->scale[1] = 0.05f;
        sndPlay(0x112);
        shimSdAtModel(&w->model->f, 0x229);
        camSunSetClockStop(1);
    }
    if (++w->stepTimer > 5) {
        tanuStepReset(o);
        if (swWorkTest(w->data->tegata) == 0) {
            w->step = 3;
        } else {
            w->step = 4;
        }
    }
}

/* tanu step 3: at 10 frames makes kind 0xF7 above itself; from 31 on, once that is hit, the
 * tegata scene runs (a fade, sounds, the area's tegata and its flag), then step 5. */
void tanuTegataStep(Obj *o) {
    Tlb99W *w = (Tlb99W *) FX(o);

    if (w->stepTimer == 0) {
        PRINTF("tanu : GO tegata_step\n");
    }
    if (++w->stepTimer == 10) {
        w->prize = eneMakeEx(o, ENE_EPRG_TANU, w->course, w->dist, w->y + 10.0f, w->off, 2, NULL);
        sndPlay(0x2BD);
    } else if (w->stepTimer >= 31 && w->prize != NULL) {
        if (tanuTegataIsTaken(w->prize) == 1) {
            sndPlay(0x233);
            w->sub = 1;
            gGameWork.scenarioRunning = 2;
        }
    }
    if (w->sub == 1) {
        if (++w->subTimer == 5) {
            eneMakeEx(o, ENE_MES_STAGE_CLEAR, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
            fadeSetAll(10.0f, 0.0f, 128.0f, 0, 0, 0);
        } else if (w->subTimer == 13) {
            sndPlay(0xB8);
        } else if (w->subTimer == 145) {
            sndPlay(0xA);
        } else if (w->subTimer == 155) {
            scTegataAddArea(w->data->area);
            swWorkSet(w->data->tegata);
            w->sub = 2;
        }
    }
    if (w->sub == 2) {
        fadeSetAll(30.0f, 128.0f, 255.0f, 0, 0, 0);
        tanuStepReset(o);
        w->step = 5;
    }
}

/* tanu step 4: from its turn against the course picks how often (tanuAngleToDir), then
 * from frame 11 to 119 makes kind 0xF6 that often, wobbling; sounds at 130 and 170; at 180 a
 * fade and step 5. The count's increment is stored under a second name (undefined_syms.txt):
 * the game forms the address again for the store. */
void tanuZakuStep(Obj *o) {
    s32 pad;
    Tlb99W *w = (Tlb99W *) FX(o);
    f32 a;

    if (w->stepTimer == 0) {
        a = w->rotY;
        a -= eneCourseAngle(o, 8);
        while (a < 0.0f) {
            a += 6.2831855f;
        }
        w->sub = tanuAngleToDir(o, a);
        sndPlay(7);
        w->phase = 0.0f;
        PRINTF("tanu : GO zaku_step\n");
    }
    w->stepTimer++;
    if (w->stepTimer >= 11 && w->stepTimer < 120) {
        if (w->stepTimer % w->sub == 0) {
            eneMakeEx(NULL, ENE_TANU_KOBAN, w->course, w->dist, w->y + 10.0f, w->off, 2,
                      (u32 (*)(void *, void *, void *)) (tanuZakuCount % 2));
            if (w->subTimer == 0) {
                sndPlay(0xB7);
            }
            w->subTimer = 1;
            tanuZakuCountInc = tanuZakuCount + 1;
        }
    } else if (w->stepTimer == 130) {
        sndPlay(0x209);
    } else if (w->stepTimer == 170) {
        sndPlay(0xA);
    }
    if (w->stepTimer == 180) {
        fadeSetAll(30.0f, 0.0f, 255.0f, 0, 0, 0);
        tanuStepReset(o);
        w->step = 5;
    }
    w->phase += 0.4;
    if (w->phase >= 6.2831855f) {
        w->phase -= 6.2831855f;
    }
    w->rotX = mathCos(w->phase) * 0.08;
    w->rotZ = mathSin(w->phase) * 0.08;
}

/* tanu step 5, the end: stops every rumble, stands the players at 15 frames, and at 30 marks its
 * stage and the stages it connects to and leaves for the atlas. */
void tanuEndStep(Obj *o) {
    Tlb99W *w = (Tlb99W *) FX(o);
    s32 i;
    s32 j;

    if (w->stepTimer == 0) {
        for (i = 0; i < 4; i++) {
            contPakInitRumble(i);
            contPakRumbleRequestStop(i);
        }
    }
    if (w->stepTimer == 15) {
        demoPlStandAll();
    }
    if (w->stepTimer == 30) {
        demoPlReleaseAll();
        ((u8 (*)[16]) gSaveData.stageFlags)[w->data->area][w->data->point] |= 1;
        for (j = 0; j < 4; j++) {
            if (w->data->connect[j] != -1) {
                ((u8 (*)[16]) gSaveData.areaFlags)[w->data->area][w->data->connect[j]] |= 1;
            }
        }
        sndClearReverb();
        stageGotoAtlas(w->data->area, w->data->point);
    }
    w->stepTimer++;
}

/* Sorts an angle (radians, 0 to 2 pi) into a direction bit: 1 near 0, 2, 4, 6, 8 near pi. */
s32 tanuAngleToDir(Obj *o, f32 angle) {
    if (angle > 3.1415927f) {
        if (angle > 5.969026207923889) {
            return 1;
        }
        if (angle > 5.3407076597213745) {
            return 2;
        }
        if (angle > 4.3982298374176025) {
            return 4;
        }
        if (angle > 3.4557520151138306) {
            return 6;
        }
        return 8;
    }
    if (angle > 2.827433466911316) {
        return 8;
    }
    if (angle > 2.1991149187088013) {
        return 6;
    }
    if (angle > 1.2566370964050293) {
        return 4;
    }
    if (angle > 0.3141592741012573) {
        return 2;
    }
    return 1;
}

/* Places frame at tanu's course point, height and tilt. */
void tanuPlaceFrame(Obj *o, PolFrame *frame) {
    Tlb99W *w = (Tlb99W *) FX(o);

    vramCoursePointXZ(w->course, w->dist, w->off, &frame->pos[0], &frame->pos[2]);
    frame->pos[1] = w->y;
    frame->rot[0] = w->rotX;
    frame->rot[2] = w->rotZ;
}

void tanuStepReset(Obj *o) {
    ((Tlb99W *) FX(o))->stepTimer = 0;
    ((Tlb99W *) FX(o))->subTimer = 0;
    ((Tlb99W *) FX(o))->sub = 0;
}

/* tanu's walk dust: one big puff beside it on the step's first frame, then three small ones
 * around it every third frame up to 15. */
void tanuWalkDust(Obj *o) {
    f64 scale = 1.0;
    FxWork dust;
    s32 i;
    FxWork puff;
    Tlb99W *w = (Tlb99W *) FX(o);

    if (w->stepTimer == 0) {
        effectPuffDefaults(&puff);
        vramCoursePointXZ(w->course, w->dist, w->off + 21.0f, &puff.pos[0], &puff.pos[2]);
        puff.pos[1] = w->y + 15.0f;
        puff.r = puff.g = 255; puff.b = 128;
        puff.kind = 2;
        puff.scale = 0.9f;
        puff.grow = 0.0f;
        puff.vel[1] = 0.1f;
        effectPuffMake(puff);
    }
    if (w->stepTimer < 15 && w->stepTimer % 3 == 0) {
        for (i = 0; i < 3; i++) {
            effectPuffDefaults(&dust);
            vramCoursePointXZ(w->course, fadeRandRange(-14, 14) + w->dist, w->off + 20.0f, &dust.pos[0],
                              &dust.pos[2]);
            dust.pos[1] = fadeRandRange(15, 35) + w->y;
            dust.scale = 0.5f;
            dust.grow = -0.016f;
            dust.vel[1] = (fadeRandRange(-1, 1) / 10.0f * 2.0 + 0.3) * scale;
            dust.r = dust.g = 255; dust.b = 128;
            dust.kind = 2;
            effectPuffMake(dust);
        }
    }
}

/* A dust puff at tanu's feet, side * 12 to the side of its facing. The game's scale is set
 * before its growth (their constants' order in .rodata), the position's sums read the model's
 * position first, and one word sits above w. */
void tanuFootDust(Obj *o, f32 side) {
    s32 pad;
    Tlb99W *w = (Tlb99W *) FX(o);
    FxWork fx;

    effectPuffDefaults(&fx);
    fx.pos[0] = w->model->pos[0] + mathSin(w->rotY) * 12.0f * side;
    fx.pos[2] = w->model->pos[2] + mathCos(w->rotY) * 12.0f * side;
    fx.pos[1] = w->model->pos[1];
    fx.scale = 0.2f;
    fx.grow = 0.002f;
    fx.vel[1] = 0.02f;
    fx.r = fx.g = fx.b = 200;
    fx.kind = 2;
    effectPuffMake(fx);
}

/* "koban" (kind 0xF6, made by tanu's step 4): a gold coin thrown out to one side (its mode),
 * spinning; it blinks out, or is gone when it lands or is taken (a sound, sparks). The hitter is
 * tested as a pointer (kpaFireball's form), and floadGetSetFile's result is a local declared
 * first: the game names it where a temp would sit. */
s32 tanuKobanProc(Obj *o, s16 msg, void *arg) {
    s16 file;
    Tlb99KobanW *w = (Tlb99KobanW *) FX(o);
    u32 mode;

    switch (msg) {
        case 2:
            o->name = "koban";
            mode = eneSetup(o, arg);
            w->model = enePolModelMake(o, tanuKobanCoinDl, NULL, 1);
            if (w->model == NULL) {
                return -1;
            }
            w->model->flags |= 7;
            file = floadGetSetFile(0xF, 0);
            fadeAnimStart(&w->anim, coinSpinAnim, 0, &w->model->segs[3], file);
            w->model->scale[2] = 0.17f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            if (mode == 0) {
                w->vel[0] = fadeRandRange(-30, -5) / 10.0f;
            } else {
                w->vel[0] = fadeRandRange(5, 30) / 10.0f;
            }
            w->vel[1] = fadeRandRange(5, 8);
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 20, 0, 0, 0);
            w->atk->kind = 0x6B;
            w->alpha = 255;
            w->count = 30;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            break;
        case 3:
            if (w->model != NULL) {
                w->model->dl = gGfxDlHead;
                gDPSetCombineMode(gGfxDlHead++, G_CC_DECALRGBA, G_CC_DECALRGBA);
                gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
                gSPEndDisplayList(gGfxDlHead++);
            }
            break;
        case 0:
            eneStepHookPre(o);
            fadeAnimStep(&w->anim);
            eneFall(o, 0.4f);
            eneMove((EneObj *) w);
            w->hit &= ~1;
            eneLandGround(o, 10.0f);
            if (w->hit & 1) {
                return -1;
            }
            if (w->blink) {
                if (--w->count & 1) {
                    w->alpha = 255;
                } else {
                    w->alpha = 64;
                }
                if (w->count == 0) {
                    return -1;
                }
            }
            eneSetModelOnCourse(o, &w->model->f);
            if (acollAtkWasHit(w->atk) == 1) {
                if ((Obj *) w->atk->hitPower != NULL) {
                    tskSendMsg((Obj *) w->atk->hitPower, 7, o);
                }
                ptclSpawnSparksFall(w->model->pos[0], w->model->pos[1], w->model->pos[2]);
                sndPlay(0x232);
                return -1;
            }
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "tegata" (kind 0xF7, made by tanu's step 3): the hand print; springs up, spinning, sinks back
 * to 25 above where it began, and then makes kind 0xF5 below itself (the rider) and can be
 * taken (tanuTegataIsTaken); it bobs there, sparkling until then. */
s32 Eprg_tanu(Obj *o, s16 msg, void *arg) {
    s32 pad;
    Tlb99TegataW *w = (Tlb99TegataW *) FX(o);

    switch (msg) {
        case 2:
            o->name = "tegata";
            eneSetup(o, arg);
            w->model = eneNewPol(o, eprgTanuHandPrintDl);
            if (w->model == NULL) {
                return -1;
            }
            w->model->flags |= 0x20;
            w->model->scale[2] = 0.15f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 50, 0, 0, 0);
            if (w->atk == NULL) {
                PRINTF("Eprg_tanu : tamaD is NULL\n");
                return -1;
            }
            w->atk->kind = 0x71;
            w->atk->flags &= ~2;
            w->count = 0;
            w->state = 0;
            w->y0 = w->y;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->rider != NULL) {
                tskDestroy(w->rider);
            }
            break;
        case 0:
            eneStepHookPre(o);
            if (w->count == 0) {
                w->vel[1] = 7.0f;
                w->accel = -0.4f;
                w->spin = 0.6981317401f;
                w->model->scale[2] = 0.05f;
                w->model->scale[0] = w->model->scale[2];
            }
            w->count++;
            switch (w->state) {
                case 0:
                    if (w->y0 + 50.0f < w->y) {
                        w->accel = -0.4f;
                        w->spin = 0.6981317401f;
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (w->y < w->y0 + 25.0f) {
                        w->accel = 6.4f;
                        w->model->scale[2] = 0.15f;
                        w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                        w->rider = eneMakeEx(NULL, ENE_TANU_EFF, w->course, w->dist, w->y, w->off - 20.0f, 0, NULL);
                        w->state = 2;
                        w->atk->flags |= 2;
                    }
                    break;
                case 2:
                    w->spin = 0.174532935f;
                    if (w->y < w->y0 + 25.0f) {
                        w->accel = 0.1f;
                    } else if (w->y0 + 25.0f < w->y) {
                        w->accel = -0.1f;
                    }
                    break;
            }
            w->vel[1] += w->accel;
            w->rotY += w->spin;
            eneMoveAndCollide(o, 5.0f, 4.0f, w->off, 4.0f);
            eneSetModelOnCourse(o, &w->model->f);
            if (!(w->count & 1) && w->state < 2) {
                w->sparkle = fadeRandRange(10, 15) / 100.0f;
                itexSparkle(w->model->pos[0], w->model->pos[1], w->model->pos[2], w->sparkle);
            }
            eneStepHookPost(o);
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Whether the tegata (o, Eprg_tanu) was taken: then sparks at it, its rider goes,
 * its box is off and its model's second list too. */
s32 tanuTegataIsTaken(Obj *o) {
    s32 pad;
    Tlb99TegataW *w = (Tlb99TegataW *) FX(o);

    if (acollAtkWasHit(w->atk) == 1) {
        ptclSpawnSparksFall(w->model->pos[0], w->model->pos[1], w->model->pos[2]);
        if (w->rider != NULL) {
            w->rider = tskDestroy(w->rider);
        }
        w->atk->flags &= ~2;
        w->model->dl2 = NULL;
        return 1;
    }
    return 0;
}

/* "piece": a shard of tanu's pot; flies up, spinning, falls through three bounces at -1000 and
 * fades out. */
s32 tanuPieceProc(Obj *o, s16 msg, void *arg) {
    s32 pad[3];
    Tlb99PieceW *w = (Tlb99PieceW *) FX(o);
    f32 s;

    switch (msg) {
        case 2:
            o->name = "piece";
            w->unused38 = 0x40;
            if ((w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                return -1;
            }
            w->frame->flags |= 0x20;
            w->frame->dl2 = tanuPieceDl;
            floadGetSetRam((void **) w->frame->segs, 0x11E);
            s = fadeRandRange(70, 110) / 100.0f * ((Tlb99PieceArg *) arg)->scale;
            w->frame->scale[2] = s;
            w->frame->scale[1] = s;
            w->frame->scale[0] = s;
            w->frame->pos[0] = ((Tlb99PieceArg *) arg)->pos[0];
            w->frame->pos[1] = ((Tlb99PieceArg *) arg)->pos[1];
            w->frame->pos[2] = ((Tlb99PieceArg *) arg)->pos[2];
            w->alpha = 255;
            w->state = 0;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->frame->rot[0] += 0.0523598790116402354;
            w->frame->rot[1] += 0.349065860077601542;
            w->alpha -= 4;
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            if (w->state == 0) {
                w->vel[0] = fadeRandRange(-10, 10) / 10.0f;
                w->vel[1] = fadeRandRange(10, 20) / 10.0f;
                w->vel[2] = fadeRandRange(-10, 10) / 10.0f;
                w->state++;
                w->fall = 0.0f;
            } else if (w->state == 1) {
                if (w->frame->pos[1] <= -1000.0f) {
                    w->vel[1] = -(w->vel[1] * 0.9);
                    w->state++;
                }
            } else if (w->state == 2) {
                if (w->frame->pos[1] <= -1000.0f) {
                    w->vel[1] = -(w->vel[1] * 0.9);
                    w->state++;
                }
            } else if (w->state == 3) {
                if (w->frame->pos[1] <= -1000.0f) {
                    return -1;
                }
            }
            w->fall += 0.04;
            w->vel[1] -= w->fall;
            if (w->vel[1] < -4.0f) {
                w->vel[1] = -4.0f;
            }
            w->frame->pos[0] += w->vel[0];
            w->frame->pos[1] += w->vel[1];
            w->frame->pos[2] += w->vel[2];
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Starts a "piece" (tanuPieceProc) at x, y, z, each moved by up to spread / 2. */
void tanuPieceCreate(Obj *o, f32 x, f32 y, f32 z, f32 scale, s32 spread) {
    Tlb99PieceArg arg;

    arg.pos[0] = fadeRand(spread) + (x - spread / 2);
    arg.pos[1] = fadeRand(spread) + (y - spread / 2);
    arg.pos[2] = fadeRand(spread) + (z - spread / 2);
    arg.scale = scale;
    tskCreate(gEffectTask, tanuPieceProc, o->file, &arg);
}

/* "wall": a model with a ride (something to stand on), turned to its course, 20 lower. */
s32 tanuWallProc(Obj *o, s16 msg, void *arg) {
    s32 pad;
    Tlb99WallW *w = (Tlb99WallW *) FX(o);

    switch (msg) {
        case 2:
            o->name = "wall";
            eneSetup(o, arg);
            floadLoadSet(0x34);
            w->model = eneNewPol(o, NULL);
            if (w->model == NULL) {
                return -1;
            }
            if (w->course != 0) {
                w->rotY = eneCourseAngle(o, 8);
            }
            w->ride = rideCreate(gRideTree1, (s32) o, &w->model->f, (RideData *) floadSegToRam((u32) tanuJRide, 0x34, 0));
            if (w->ride == NULL) {
                return -1;
            }
            w->y -= 20.0f;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->ride != NULL) {
                frmDestroy(&w->ride->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            if (w->course != 0) {
                eneSetModelOnCourse(o, &w->model->f);
            } else {
                eneSetModel(o, &w->model->f);
            }
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
