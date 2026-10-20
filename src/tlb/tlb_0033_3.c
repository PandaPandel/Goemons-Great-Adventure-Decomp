#include "tlb.h"
#include "tlb/tlb_0033.h"

/* "cKIRAN"'s work: two sparkle frames growing together (tlb_0033_2's KiranW). */
typedef struct CKiranW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ PolFrame *frame2;
    /* 0x08 */ u8 pad08[8];
    /* 0x10 */ f32 scale;
    /* 0x14 */ s32 alpha;
} CKiranW;

extern u8 utaCKiranDl[];
extern u8 utaCKiranGlowDl[];
extern s32 gDemoModelsDelete2;

/* "cKIRAN": tlb_0033_2's "KIRAN" (utaKiranPairProc) for this demo: two
 * sparkles at (0, 0, 5) growing by 1.2 a frame up to 10 and brightening by 2; the second at a
 * third of the first's alpha. */
s32 utaCKiranProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    CKiranW *w = (CKiranW *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "cKIRAN";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, (Gfx *) utaCKiranDl, 0xC6);
            w->frame->flags |= 7;
            w->frame2 = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame2 == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame2, (Gfx *) utaCKiranGlowDl, 0xC6);
            w->frame2->flags |= 7;
            w->frame->pos[0] = w->frame2->pos[0] = 0.0f;
            w->frame->pos[1] = w->frame2->pos[1] = 0.0f;
            w->frame->pos[2] = w->frame2->pos[2] = 5.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale = 0.25f;
            w->frame2->scale[0] = w->frame2->scale[1] = w->frame2->scale[2] = w->scale;
            w->alpha = 0x80;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            frmDestroy((Frame *) w->frame2);
            break;
        case 3:
            w->frame->dl = demoColorDL(255, 255, 255, 255, 255, 0, w->alpha);
            w->frame2->dl = demoColorDL(255, 255, 255, 255, 255, 0, w->alpha / 3);
            break;
        case 0:
            if (gDemoModelsDelete2) {
                return -1;
            }
            w->scale *= 1.2;
            if (10.0f < w->scale) {
                w->scale = 10.0f;
            }
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale;
            w->frame2->scale[0] = w->frame2->scale[1] = w->frame2->scale[2] = w->scale;
            w->alpha += 2;
            if (w->alpha >= 0x100) {
                w->alpha = 0xFF;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *utaCKiranCreate(Obj *obj) {
    return tskCreate(obj, utaCKiranProc, obj->file, NULL);
}

/* "cLBALL"'s work. */
typedef struct CLballW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ PolFrame *frame2;
    /* 0x08 */ f32 scale;
    /* 0x0C */ s32 timer;
    /* 0x10 */ s32 alpha;
    /* 0x14 */ s32 state;
    /* 0x18 */ f32 phase; /* its pulse's */
    /* 0x1C */ u8 pad1C[0x4];
    /* 0x20 */ f32 unused20; /* written 5.0f at its start, never read */
    /* 0x24 */ u8 pad24[0x4];
    /* 0x28 */ f32 fall; /* frames it has fallen */
} CLballW;

extern s32 utaDemoFrame; /* the demo's frame count */
extern u8 utaCLballDl[]; /* the ball */
Obj *utaThunderMakerCreate(Obj *obj);

/* "cLBALL", the light ball: two pol frames (file 0x120) at (0, 180, -420), growing by 30% a frame
 * to 10 and pulsing 2% (60 degrees a frame), drawn white at alpha 0xE0. At demo frame 1626 it
 * moves on; 10 frames later it starts its follower (utaThunderMakerCreate) and then falls away
 * (z by 1.25 t^2, y by -5.75 t, to 10000). It ends with the demo's models. */
s32 utaCLballProc(Obj *obj, s16 msg, void *arg) {
    CLballW *w = (CLballW *) FX(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "cLBALL";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, (Gfx *) utaCLballDl, 0x120);
            w->frame->flags |= 7;
            w->frame2 = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame2 == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame2, (Gfx *) utaCLballDl, 0x120);
            w->frame2->flags |= 7;
            w->frame->pos[0] = w->frame2->pos[0] = 0.0f;
            w->frame->pos[1] = w->frame2->pos[1] = 180.0f;
            w->frame->pos[2] = w->frame2->pos[2] = -420.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale = 0.1f;
            w->frame2->scale[0] = w->frame2->scale[1] = w->frame2->scale[2] = w->scale;
            w->alpha = 0xE0;
            w->timer = 0;
            w->state = 0;
            w->unused20 = 5.0f;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            frmDestroy((Frame *) w->frame2);
            break;
        case 3:
            w->frame->dl = demoColorDL(255, 255, 255, 255, 255, 0, w->alpha);
            w->frame2->dl = demoColorDL(255, 255, 255, 255, 255, 0, w->alpha);
            break;
        case 0:
            if (gDemoModelsDelete2) {
                return -1;
            }
            w->phase += 1.0471976f;
            switch (w->state) {
                case 0:
                    w->scale *= 1.3f;
                    if (w->scale >= 10.0f) {
                        w->scale = 10.0f;
                    }
                    if (utaDemoFrame == 1626) {
                        w->state++;
                        w->fall = 0.0f;
                    }
                    break;
                case 1:
                    if (++w->timer >= 10) {
                        utaThunderMakerCreate(obj);
                        w->state++;
                    }
                    break;
                case 2:
                    w->fall += 1.0f;
                    w->frame->pos[2] = w->fall * 1.25f * w->fall;
                    w->frame->pos[1] = w->fall * -5.75f + 180.0f;
                    if (w->frame->pos[2] > 10000.0f) {
                        w->frame->pos[2] = 10000.0f;
                    }
                    if (w->frame->pos[1] < -10000.0f) {
                        w->frame->pos[1] = -10000.0f;
                    }
                    w->frame2->pos[2] = w->frame->pos[2];
                    w->frame2->pos[1] = w->frame->pos[1];
                    break;
            }
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = (mathCos(w->phase) * 0.02f + 1.0f) * w->scale;
            w->frame2->scale[0] = w->frame2->scale[1] = w->frame2->scale[2] = (mathCos(w->phase) * 0.02f + 1.0f) * w->scale;
            if (w->alpha >= 0x100) {
                w->alpha = 0xFF;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 utaCLballProc(Obj *obj, s16 msg, void *arg);

Obj *utaCLballCreate(Obj *obj) {
    return tskCreate(obj, utaCLballProc, obj->file, NULL);
}

/* "cFIRE"'s work (two blocks). */
typedef struct CFireW {
    /* 0x00 */ PolFrame *frames[8]; /* two to a fire; the rest under the first */
    /* 0x20 */ Anim anims[4]; /* each fire's flicker */
    /* 0x60 */ u8 pad60[0x4];
    /* 0x64 */ s32 alpha;
    /* 0x68 */ u8 pad68[0x4];
    /* 0x6C */ s32 timer;
} CFireW;

extern f32 utaCFirePlaces[][3]; /* the four fires' places */
extern f32 utaCFireScales[];    /* and scales */
extern u32 utaCFireModels[];    /* the eight models */
extern u32 utaCFireFirstFrame[];    /* each fire's first flicker frame */
extern f32 utaCFireDriftTo[];    /* where they drift to */

/* "cFIRE": four fires of two models each (file 0x125), yellow and flickering, at their places;
 * over 100 frames they drift toward (0, 0, 3265), a fifth of the way in x and y. Drawn
 * (255, 255, 128) at alpha 0xC0; it ends with the demo's models. */
s32 utaCFireProc(Obj *obj, s16 msg, void *arg) {
    CFireW *w = (CFireW *) FX(obj);
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "cFIRE";
            for (i = 0; i < 8; i++) {
                f32 scale;

                if ((w->frames[i] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                    return -1;
                }
                if (i != 0) {
                    frmMove(&w->frames[i]->f, &w->frames[0]->f);
                }
                demoPolSetDL(w->frames[i], (Gfx *) utaCFireModels[i], 0x125);
                fadeAnimStart(&w->anims[i / 2], gItexTimedYellowAnim, utaCFireFirstFrame[i / 2],
                              &w->frames[i / 2]->segs[3], floadGetSetFile(0x125, 0));
                w->frames[i]->flags |= 0x27;
                w->frames[i]->pos[0] = utaCFirePlaces[i / 2][0];
                w->frames[i]->pos[1] = utaCFirePlaces[i / 2][1];
                w->frames[i]->pos[2] = utaCFirePlaces[i / 2][2];
                /* one source line: the game loads frames[i] before the scale (IDO_NOTES 12) */
                scale = utaCFireScales[i / 2]; w->frames[i]->scale[2] = scale;
                w->frames[i]->scale[1] = scale;
                w->frames[i]->scale[0] = scale;
            }
            w->alpha = 0xC0;
            w->timer = 0;
            break;
        case 1:
            for (i = 0; i < 8; i++) {
                frmDestroy(&w->frames[i]->f);
            }
            break;
        case 3:
            for (i = 0; i < 8; i++) {
                w->frames[i]->dl = demoColorDL(0xFF, 0xFF, 0x80, 0xFF, 0x28, 0, w->alpha);
            }
            break;
        case 0:
            if (gDemoModelsDelete2) {
                return -1;
            }
            for (i = 0; i < 8; i++) {
                fadeAnimStep(&w->anims[i / 2]);
            }
            w->timer++;
            if (w->timer < 100) {
                for (i = 0; i < 8; i++) {
                    w->frames[i]->pos[0] += (utaCFireDriftTo[0] - w->frames[i]->pos[0]) * 0.2f / (100 - w->timer);
                    w->frames[i]->pos[1] += (utaCFireDriftTo[1] - w->frames[i]->pos[1]) * 0.2f / (100 - w->timer);
                    w->frames[i]->pos[2] += (utaCFireDriftTo[2] - w->frames[i]->pos[2]) / (100 - w->timer);
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 utaCFireProc(Obj *obj, s16 msg, void *arg);

/* Starts a utaCFireProc task (2 blocks) under obj. */
Obj *utaCFireCreate(Obj *obj) {
    return tskCreateBlocks(obj, utaCFireProc, 2, obj->file, NULL);
}

/* "cBKIRAN"'s work: cKIRAN's, with four frames and a state. */
typedef struct CBkiranW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ PolFrame *frame2;
    /* 0x08 */ PolFrame *frame3;
    /* 0x0C */ PolFrame *frame4;
    /* 0x10 */ f32 scale;
    /* 0x14 */ s32 alpha;
    /* 0x18 */ s32 state;
    /* 0x1C */ s32 timer;
} CBkiranW;

extern u8 gDemoCamHold;

#define DPRINTF

/* "cBKIRAN", two sparkles (cKIRAN's pair at (59.7, 109, 657.7) and at (204, 109, 568)): the
 * cores grow by half again a frame to 150 while fading in (2 a frame), the halos at half; they
 * hold 20 frames and shrink (by 1.2) and fade out, going under 20. While the demo camera holds,
 * A and B grow and shrink them by 0.5. */
s32 utaCBKiranProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    CBkiranW *w = (CBkiranW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "cBKIRAN";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, (Gfx *) utaCKiranDl, 0xC6);
            w->frame->flags |= 7;
            w->frame2 = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame2 == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame2, (Gfx *) utaCKiranGlowDl, 0xC6);
            w->frame2->flags |= 7;
            w->frame3 = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame3 == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame3, (Gfx *) utaCKiranDl, 0xC6);
            w->frame3->flags |= 7;
            w->frame4 = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame4 == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame4, (Gfx *) utaCKiranGlowDl, 0xC6);
            w->frame4->flags |= 7;
            w->frame->pos[0] = w->frame2->pos[0] = 59.7f;
            w->frame->pos[1] = w->frame2->pos[1] = 109.0f;
            w->frame->pos[2] = w->frame2->pos[2] = 657.7f;
            w->frame3->pos[0] = w->frame4->pos[0] = 204.0f;
            w->frame3->pos[1] = w->frame4->pos[1] = 109.0f;
            w->frame3->pos[2] = w->frame4->pos[2] = 568.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale = 0.25f;
            w->frame2->scale[0] = w->frame2->scale[1] = w->frame2->scale[2] = w->scale;
            w->frame3->scale[0] = w->frame3->scale[1] = w->frame3->scale[2] = w->scale;
            w->frame4->scale[0] = w->frame4->scale[1] = w->frame4->scale[2] = w->scale;
            w->alpha = 0xC0;
            w->state = 0;
            w->scale = 20.0f;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            frmDestroy((Frame *) w->frame2);
            frmDestroy((Frame *) w->frame3);
            frmDestroy((Frame *) w->frame4);
            break;
        case 3:
            w->frame->dl = demoColorDL(255, 255, 255, 0, 0, 255, w->alpha);
            w->frame2->dl = demoColorDL(255, 255, 255, 0, 0, 255, w->alpha / 3);
            w->frame3->dl = demoColorDL(255, 255, 255, 0, 0, 255, w->alpha);
            w->frame4->dl = demoColorDL(255, 255, 255, 0, 0, 255, w->alpha / 3);
            break;
        case 0:
            if (gDemoModelsDelete2) {
                return -1;
            }
            switch (w->state) {
                case 0:
                    w->alpha += 2;
                    w->scale *= 1.5;
                    if (w->scale > 150.0f) {
                        w->scale = 150.0f;
                        w->state++;
                        w->timer = 0;
                    }
                    break;
                case 1:
                    if (++w->timer >= 20) {
                        w->state++;
                    }
                    break;
                case 2:
                    w->scale /= 1.2;
                    w->alpha -= 2;
                    if (w->scale < 20.0f) {
                        w->scale = 20.0f;
                        return -1;
                    }
                    break;
            }
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale;
            w->frame2->scale[0] = w->frame2->scale[1] = w->frame2->scale[2] = w->scale / 2;
            w->frame3->scale[0] = w->frame3->scale[1] = w->frame3->scale[2] = w->scale;
            w->frame4->scale[0] = w->frame4->scale[1] = w->frame4->scale[2] = w->scale / 2;
            if (w->alpha >= 0x100) {
                w->alpha = 0xFF;
            }
            if (w->alpha < 0) {
                w->alpha = 0;
            }
            if (gDemoCamHold == 1) {
                if (joyGetPadHist(3, 0)->button & A_BUTTON) {
                    w->scale += 0.5;
                }
                if (joyGetPadHist(3, 0)->button & B_BUTTON) {
                    w->scale -= 0.5;
                }
                DPRINTF("**** scale[%.3f] ***\n", w->scale);
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale;
                w->frame2->scale[0] = w->frame2->scale[1] = w->frame2->scale[2] = w->scale;
                w->frame3->scale[0] = w->frame3->scale[1] = w->frame3->scale[2] = w->scale;
                w->frame4->scale[0] = w->frame4->scale[1] = w->frame4->scale[2] = w->scale;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 utaCBKiranProc(Obj *obj, s16 msg, void *arg);

Obj *utaCBKiranCreate(Obj *obj) {
    return tskCreate(obj, utaCBKiranProc, obj->file, NULL);
}

/* The file's .data, in address order (its externs above stay as declarations). */
extern u8 utaCFireModelBDl[];
extern u8 utaCFireModelADl[];
extern u8 utaCFireModelCDl[]; /* file set 0x125's: utaCKiranDl's address in another file */
f32 utaCFirePlaces[][3] = { -2.7e+02f, 165.0f, 3065.0f, 2.7e+02f, 165.0f, 3065.0f, 2.7e+02f, -225.0f, 3065.0f, -2.7e+02f, -225.0f, 3065.0f };
f32 utaCFireScales[] = { 5.0f, 5.0f, 5.0f, 5.0f };
u32 utaCFireModels[] = { (u32) utaCFireModelADl, (u32) utaCFireModelCDl, (u32) utaCFireModelADl, (u32) utaCFireModelCDl, (u32) utaCFireModelBDl, (u32) utaCFireModelADl, (u32) utaCFireModelBDl, (u32) utaCFireModelCDl };
u32 utaCFireFirstFrame[] = { 0x00000000, 0x00000007, 0x00000002, 0x0000000D };
f32 utaCFireDriftTo[] = { 0.0f, 0.0f, 3265.0f };

/* A random number from -n to n - 1 (a linear congruential generator, 15 bits). Its seed is its
 * static, the file's last data word: uopt keeps it in a register and stores it last, as the
 * game does (IDO_NOTES 316). */
s16 utaRand(s16 n) {
    static u32 seed = 3749696;

    seed = seed * 1103515245 + 5963;
    return (s16) (seed & 0x7FFF) / (0x8000 / (s16) (n * 2) + 1) - n;
}

/* "cTHUNDER"'s (utaCThunderProc) argument: only what its starter sets. */
typedef struct CThunderArg {
    /* 0x00 */ u8 pad00[0x18];
    /* 0x18 */ s32 kind;
    /* 0x1C */ u8 pad1C[0xC];
    /* 0x28 */ f32 value; /* the bolt's x */
} CThunderArg;

/* "cTHUNDER"'s work (tlb_0039's ThunderW, without its position). */
typedef struct Tlb33ThunderW {
    /* 0x00 */ PolFrame *model;
    /* 0x04 */ u8 pad04[0xC];
    /* 0x10 */ s32 alpha;
    /* 0x14 */ s32 frame; /* of its four-frame animation */
    /* 0x18 */ s32 kind; /* 0 a bolt, 1 one farther out, 2 a flash */
    /* 0x1C */ s16 prim[3];
    /* 0x22 */ s16 env[3];
} Tlb33ThunderW;

extern Gfx utaCThunderFrame0Dl[]; /* the bolt's four frames */
extern Gfx utaCThunderFrame1Dl[];
extern Gfx utaCThunderFrame2Dl[];
extern Gfx utaCThunderFrame3Dl[];

/* "cTHUNDER": tlb_0039's lightning bolt, larger (2.5 wide) and far off: at the given x (plus up
 * to 100, or 400 for kind 1), height 1920, z about -3500 (-4000). It flickers through its four
 * frames every third frame, stretches 1% a frame and fades; it ends at alpha 0. */
s32 utaCThunderProc(Obj *obj, s16 msg, void *arg) {
    s32 pad[2];
    Tlb33ThunderW *w = (Tlb33ThunderW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "cTHUNDER";
            if ((w->model = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                return -1;
            }
            demoPolSetDL(w->model, utaCThunderFrame0Dl, 0xC6);
            w->kind = ((CThunderArg *) arg)->kind;
            w->model->flags |= 0x22;
            w->model->scale[1] = 1.0f;
            w->model->scale[0] = 2.5f;
            w->model->scale[2] = w->model->scale[0];
            switch (w->kind) {
                case 0:
                    w->model->pos[0] = utaRand(100) + ((CThunderArg *) arg)->value;
                    w->model->pos[1] = 1920.0f;
                    w->model->pos[2] = utaRand(200) - 3500;
                    w->frame = 0;
                    w->alpha = 224;
                    w->model->scale[1] = 2.0f;
                    break;
                case 1:
                    w->model->pos[0] = utaRand(400) + ((CThunderArg *) arg)->value;
                    w->model->pos[1] = 1920.0f;
                    w->model->pos[2] = utaRand(500) - 4000;
                    w->frame = 0;
                    w->alpha = 224;
                    w->model->scale[1] = 2.0f;
                    break;
                case 2:
                    break;
            }
            w->prim[0] = 255;
            w->prim[1] = 255;
            w->prim[2] = 224;
            w->env[0] = 160;
            w->env[1] = 96;
            w->env[2] = 32;
            break;
        case 1:
            frmDestroy(&w->model->f);
            break;
        case 3:
            w->model->dl = demoColorDL(w->prim[0], w->prim[1], w->prim[2], w->env[0], w->env[1], w->env[2],
                                       w->alpha);
            break;
        case 0:
            if (gDemoModelsDelete2) {
                return -1;
            }
            if (gGameFrameCount % 3 == 0) {
                if (++w->frame >= 5) {
                    w->frame = 0;
                }
                switch (w->frame) {
                    case 0:
                        demoPolSetDL(w->model, utaCThunderFrame0Dl, 0xC6);
                        break;
                    case 1:
                        demoPolSetDL(w->model, utaCThunderFrame1Dl, 0xC6);
                        break;
                    case 2:
                        demoPolSetDL(w->model, utaCThunderFrame2Dl, 0xC6);
                        break;
                    case 3:
                        demoPolSetDL(w->model, utaCThunderFrame3Dl, 0xC6);
                        break;
                }
            }
            switch (w->kind) {
                case 0:
                    w->model->scale[1] *= 1.01;
                    if (w->alpha > 208) {
                        w->alpha--;
                    } else {
                        w->alpha -= 20;
                    }
                    break;
                case 1:
                    w->model->scale[1] *= 1.01;
                    w->alpha -= 16;
                    break;
                case 2:
                    w->model->scale[1] *= 1.01;
                    w->model->scale[0] = 2.0f;
                    w->model->scale[2] = w->model->scale[0];
                    if (w->alpha > 208) {
                        w->alpha--;
                    } else {
                        w->alpha -= 20;
                    }
                    break;
            }
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Starts a "cTHUNDER" under obj. */
void utaCThunderCreate(Obj *obj, s32 kind, f32 value) {
    CThunderArg arg;

    arg.kind = kind;
    arg.value = value;
    tskCreate(obj, utaCThunderProc, obj->file, &arg);
}

extern s32 gDemoModelsDelete2;

/* The thunder maker's work: the frames it has run. */
typedef struct ThunderMakerW {
    /* 0x00 */ s32 count;
} ThunderMakerW;

/* Makes "cTHUNDER"s as the demo runs: kind 0 every 10 and 13 frames, kind 1 every 8 and 5,
 * their value -15 or 15 times its own frame count. It goes when the demo deletes its models.
 * 110 lines: the game keeps msg sign-extended in a3 (sh/lh around each call) and obj in its
 * home. With w at the declaration this C homes msg too (118); with w made in case 0 (this
 * form) msg goes to a3 but obj to s0. Also measured: an old-style definition (no change); the
 * filter proc's `(obj + 1)` between two pads (134). See IDO_NOTES 309 (atoya).
 * 2026-10-12 (from real 50): the game uses no saved register at all: w (obj + 0x20)
 * in v1, spilled to 0x18 around each call, msg in a3 (sh/lh), obj in its home. Measured: FAKE
 * "if (w != NULL) {}" after w is set (64: obj homed, but msg homed too); FAKE "if (&obj) {}"
 * there (70); w written out as FX(obj) at each use (50, the same code); "empties" (nothing);
 * "splits" (best 108 raw lines, not kept).
 * 2026-10-12 (second try), from 50: FAKE "if (&obj) {}" at case 0's top (66: obj homed, but w then
 * lives in memory at 0x1C, frame 0x20); w as a case-0 block local (50, no change); that plus
 * "if (&obj) {}" after w is set (70).
 * MATCHED 2026-10-12 (from 50): utaCThunderCreate returns nothing (void, its tskCreate
 * a plain call; it still matches): with w set at the declaration, 64 -> 6, every register the
 * game's (IDO_NOTES 369: a call's dropped result held v0, so the counter's divisions took v1
 * and w lost it). Left was w's spill at 0x1C (game 0x18); w set after the test went back to s0
 * (37), FX(obj) at each use s0 (37), w as a case-0 block local, cases 1-3 first: 6. The
 * permuter's finds (a second pointer local) say one word more under w: "s32 pad;" after w, as
 * this file's utaCLballProc: MATCH. */
s32 utaThunderMakerProc(Obj *obj, s16 msg, void *arg) {
    ThunderMakerW *w = (ThunderMakerW *) FX(obj);
    s32 pad;

    switch (msg) {
        case 0:
            if (gDemoModelsDelete2) {
                return -1;
            }
            w->count++;
            if (utaDemoFrame % 10 == 0) {
                utaCThunderCreate(obj, 0, w->count * -15.0f);
            }
            if (utaDemoFrame % 13 == 0) {
                utaCThunderCreate(obj, 0, w->count * 15.0f);
            }
            if (utaDemoFrame % 8 == 0) {
                utaCThunderCreate(obj, 1, w->count * -15.0f);
            }
            if (utaDemoFrame % 5 == 0) {
                utaCThunderCreate(obj, 1, w->count * 15.0f);
            }
            break;
        case 1:
        case 2:
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 utaThunderMakerProc(Obj *obj, s16 msg, void *arg);

Obj *utaThunderMakerCreate(Obj *obj) {
    return tskCreate(obj, utaThunderMakerProc, obj->file, NULL);
}

/* "cCLOUD" (its proc utaCCloudProc): its frame. */
typedef struct Tlb33W {
    /* 0x00 */ u8 pad00[0x4];
    /* 0x04 */ PolFrame *frame;
    /* 0x08 */ f32 steps; /* the steps a move takes */
    /* 0x0C */ f32 target[3];
    /* 0x18 */ f32 step[3];
    /* 0x24 */ s32 alpha; /* its alpha */
    /* 0x28 */ s32 alphaStep; /* the alpha's step */
} Tlb33W;

extern Gfx utaCCloudDl[]; /* the cloud, in file 0xC6 */

/* "cCLOUD": tlb_0017_2's "cloud" (effCloudProc) for this demo, with its
 * work 4 further on. A "pol" frame moving at a set speed towards a target, fading; gone when
 * its alpha runs out, or at the demo's frame 0x8DF. */
s32 utaCCloudProc(Obj *obj, s16 msg, void *arg) {
    Tlb33W *w = (Tlb33W *) FX(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "cCLOUD";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = utaCCloudDl;
            w->frame->flags = 7;
            floadGetSetRam((void **) w->frame->segs, 0xC6);
            w->frame->scale[2] = 10.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->alpha = 255;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (utaDemoFrame == 0x8DF) {
                return -1;
            }
            if (0.0001f < w->step[0]) {
                if (fabsf(w->target[0] - w->frame->pos[0]) < w->step[0]) {
                    w->frame->pos[0] = w->target[0];
                    w->step[0] = 0.0f;
                } else if (w->target[0] < w->frame->pos[0]) {
                    w->frame->pos[0] -= w->step[0];
                } else {
                    w->frame->pos[0] += w->step[0];
                }
            }
            if (0.0001f < w->step[1]) {
                if (fabsf(w->target[1] - w->frame->pos[1]) < w->step[1]) {
                    w->frame->pos[1] = w->target[1];
                    w->step[1] = 0.0f;
                } else if (w->target[1] < w->frame->pos[1]) {
                    w->frame->pos[1] -= w->step[1];
                } else {
                    w->frame->pos[1] += w->step[1];
                }
            }
            if (0.0001f < w->step[2]) {
                if (fabsf(w->target[2] - w->frame->pos[2]) < w->step[2]) {
                    w->frame->pos[2] = w->target[2];
                    w->step[2] = 0.0f;
                } else if (w->target[2] < w->frame->pos[2]) {
                    w->frame->pos[2] -= w->step[2];
                } else {
                    w->frame->pos[2] += w->step[2];
                }
            }
            w->alpha += w->alphaStep;
            if (w->alpha < 0) {
                return -1;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetEnvColor(gGfxDlHead++, 255, 255, 255, w->alpha);
            gDPSetCombine(gGfxDlHead++, 0xFF9BFF, 0xFF36FF7F);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "cCLOUD" at x, y, z under obj. */
Obj *utaCCloudCreate(Obj *obj, f32 x, f32 y, f32 z) {
    Obj *o = tskCreate(obj, utaCCloudProc, obj->file, NULL);

    if (o != NULL) {
        ((Tlb33W *) FX(o))->frame->pos[0] = x;
        ((Tlb33W *) FX(o))->frame->pos[1] = y;
        ((Tlb33W *) FX(o))->frame->pos[2] = z;
    }
    return o;
}

void utaCCloudSetScale(Obj *o, f32 x, f32 y, f32 z) {
    ((Tlb33W *) FX(o))->frame->scale[0] = x;
    ((Tlb33W *) FX(o))->frame->scale[1] = y;
    ((Tlb33W *) FX(o))->frame->scale[2] = z;
}

void utaCCloudFadeOut(Obj *o, s32 n) {
    ((Tlb33W *) FX(o))->steps = n;
    ((Tlb33W *) FX(o))->alphaStep = -(((Tlb33W *) FX(o))->alpha + n - 1) / n;
}

/* Sends cCLOUD to x, y, z: each axis's step is its distance over the steps. */
void utaCCloudMoveTo(Obj *o, f32 x, f32 y, f32 z) {
    Tlb33W *w = (Tlb33W *) FX(o);

    w->target[0] = x;
    w->target[1] = y;
    w->target[2] = z;
    w->step[0] = fabsf(x - w->frame->pos[0]) / w->steps;
    w->step[1] = fabsf(y - w->frame->pos[1]) / w->steps;
    w->step[2] = fabsf(z - w->frame->pos[2]) / w->steps;
}

/* "cCLDMAN", the cloud man: places its kind's clouds (nine for kind 0, four far out for kind 1),
 * each at its place and scale, and ends with the demo's models. */
s32 utaCCldmanProc(Obj *obj, s16 msg, void *arg) {
    s32 *kind;

    switch (msg) {
        case 2:
            /* FAKE (the permuter's find): the do-while(0) stands for a setup macro. Without it
             * IDO folds the work's address into 0x20(s0); the game forms it in v0 (IDO_NOTES 321). */
            do {
                obj->name = "cCLDMAN";
                kind = (s32 *) FX(obj);
            } while (0);
            *kind = *(s32 *) arg;
            switch (*kind) {
                case 0:
                    utaCCloudSetScale(utaCCloudCreate(obj, 1000.0f, -200.0f, 1100.0f), 30.0f, 20.0f, 10.0f);
                    utaCCloudSetScale(utaCCloudCreate(obj, 900.0f, 300.0f, 1700.0f), 30.0f, 30.0f, 10.0f);
                    utaCCloudSetScale(utaCCloudCreate(obj, 600.0f, 200.0f, -200.0f), 30.0f, 30.0f, 10.0f);
                    utaCCloudSetScale(utaCCloudCreate(obj, 800.0f, -300.0f, 1000.0f), 40.0f, 20.0f, 10.0f);
                    utaCCloudSetScale(utaCCloudCreate(obj, 1200.0f, 500.0f, 600.0f), 30.0f, 20.0f, 10.0f);
                    utaCCloudSetScale(utaCCloudCreate(obj, -400.0f, 600.0f, -100.0f), 35.0f, 20.0f, 10.0f);
                    utaCCloudSetScale(utaCCloudCreate(obj, -450.0f, -350.0f, -450.0f), 40.0f, 20.0f, 10.0f);
                    utaCCloudSetScale(utaCCloudCreate(obj, 500.0f, -100.0f, 2500.0f), 30.0f, 30.0f, 10.0f);
                    utaCCloudSetScale(utaCCloudCreate(obj, -600.0f, 400.0f, 2000.0f), 30.0f, 20.0f, 10.0f);
                    break;
                case 1:
                    utaCCloudSetScale(utaCCloudCreate(obj, 9930.0f, 380.0f, 114.0f), 16.6f, 16.6f, 16.6f);
                    utaCCloudSetScale(utaCCloudCreate(obj, 9758.0f, 354.0f, 281.0f), 37.5f, 37.5f, 37.5f);
                    utaCCloudSetScale(utaCCloudCreate(obj, 9930.0f, 485.0f, -607.0f), 39.1f, 39.1f, 39.1f);
                    utaCCloudSetScale(utaCCloudCreate(obj, 9930.0f, -176.0f, -660.0f), 25.4f, 25.4f, 25.4f);
                    break;
            }
            break;
        case 0:
            if (gDemoModelsDelete2) {
                return -1;
            }
            break;
        case 1:
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "cCLDMAN"'s (utaCCldmanProc) argument: only what its starter sets. */
typedef struct CCldmanArg {
    /* 0x00 */ s32 kind;
    /* 0x04 */ u8 pad04[0x28];
} CCldmanArg;

s32 utaCCldmanProc(Obj *obj, s16 msg, void *arg);

/* Starts a "cCLDMAN" under obj. */
Obj *utaCCldmanCreate(Obj *obj) {
    CCldmanArg arg;

    arg.kind = 0;
    return tskCreate(obj, utaCCldmanProc, obj->file, &arg);
}

/* Starts a "cCLDMAN" under obj, of kind 1. */
Obj *utaCCldmanCreateFar(Obj *obj) {
    CCldmanArg arg;

    arg.kind = 1;
    return tskCreate(obj, utaCCldmanProc, obj->file, &arg);
}
