#include "tlb.h"

/* File 108: the "STAGE CLEAR" message ("STC", mesStageClear_proc): a camera of its own, file set
 * 0x4A loaded, and ten letters ("chaST", kind 0x12C) made one every five steps. Each letter flies
 * in from its own start to the middle with a trail of seven afterimages, then the letters spread
 * to their places, fade, and shrink away in pairs from the ends in. */

/* The message's work ("struct MES_STC_W"), at its task + 0x20: an enemy's (EneObj) to 0x34. */
typedef struct MesStcWork {
    /* 0x00 */ u8 step;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ Obj *moji[10]; /* the letters, in the order they were made */
    /* 0x5C */ u16 timer; /* steps in this step */
    /* 0x5E */ u8 count; /* letters made */
    /* 0x5F */ u8 unused5F;
} MesStcWork;

#define MESSTC(obj) ((MesStcWork *) FX(obj))

/* A letter's work ("struct MES_MOJI_W"). */
typedef struct MesMojiWork {
    /* 0x00 */ u8 step; /* set by the message (mesMojiSetStep) */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *trail[7]; /* the afterimages */
    /* 0x50 */ u8 pad50[4];
    /* 0x54 */ u8 trailAlpha[7];
    /* 0x5B */ u8 alpha;
    /* 0x5C */ u8 index; /* which letter, 0-9 (its maker's argument) */
    /* 0x5D */ u8 unused5D;
    /* 0x5E */ u16 timer;
} MesMojiWork;

#define MESMOJI(obj) ((MesMojiWork *) FX(obj))

extern u8 mesStageClearMojiDl0[];
extern u8 mesStageClearMojiDl1[];
extern u8 mesStageClearMojiDl2[];
extern u8 mesStageClearMojiDl5[];
extern u8 mesStageClearMojiDl6[];
CamView *mesStageClearCam = NULL; /* "mes_cam": the message's camera */
Gfx mesStageClearMojiSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetColorDither(G_CD_DISABLE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetCombineLERP(ENVIRONMENT, TEXEL0, ENV_ALPHA, TEXEL0, 0, 0, 0, TEXEL0, ENVIRONMENT, TEXEL0, ENV_ALPHA, TEXEL0, 0, 0, 0, TEXEL0),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsSPEndDisplayList(),
}; /* a letter's draw setup (TLB address: fadeSegToRam) */
Gfx *mesStageClearMojiDls[] = { mesStageClearMojiDl0, mesStageClearMojiDl1, mesStageClearMojiDl2, (Gfx *) 0x03000358, (Gfx *) 0x03000458, mesStageClearMojiDl5, mesStageClearMojiDl6, (Gfx *) 0x03000458, mesStageClearMojiDl2, (Gfx *) 0x03000758 }; /* the ten letters' display lists (file set 0x4A, segment 3) */
f32 mesStageClearMojiStartX[] = { -2e+01f, -22.5f, -25.0f, -22.5f, -2e+01f, 2e+01f, 22.5f, 25.0f, 22.5f, 2e+01f }; /* where each letter starts, x */
f32 mesStageClearMojiStartY[] = { 21.0f, 13.0f, 5.0f, -3.0f, -11.0f, -11.0f, -3.0f, 5.0f, 13.0f, 21.0f }; /* and y */
f32 mesStageClearMojiPlaceX[] = { -21.3f, -16.8f, -12.3f, -7.8f, -3.3f, 3.3f, 7.8f, 12.3f, 16.8f, 21.3f }; /* its place in the word, x */

/* The debug print, compiled to nothing but its arguments (as tlb_0035.c's): the strings stay in
 * .rodata and no dead branch is left. With PRINTF's three "if (0)" blocks in the letter's
 * handler its loop values lose their saved registers (IDO_NOTES 281). */
#define DPRINTF

void mesStageClearMojiSetStep(Obj *obj, s32 step);
Gfx *mesStageClearMojiDl(Obj *obj, s32 a, s32 r, s32 g, s32 b);
Gfx *mesStageClearTrailDl(s32 a);

/* The message's handler ("mesStageClear_proc"). The two pad words give the game's 0x68 frame
 * (one word leaves it at 0x60 and moves the file set's return slot down). */
s32 mesStageClearProc(Obj *obj, s16 msg, void *arg) {
    MesStcWork *w = MESSTC(obj);
    s32 i;
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "STC";
            DPRINTF("struct MES_STC_W size %d\n", sizeof(MesStcWork));
            mesStageClearCam = (CamView *) frmCreate((Frame *) gFramePool10, gFrmKindCamera);
            if (mesStageClearCam == NULL) {
                DPRINTF("mes_cam is NULL\n");
                return -1;
            }
            floadLoadSet(0x4A);
            if (floadIsSetLoaded(0x4A) == 0) {
                DPRINTF("mesStageClear_proc : packetLoad FAULT!!!\n");
                return -1;
            }
            mesStageClearCam->scissor.ulx = gGameState.scissor.ulx;
            mesStageClearCam->scissor.uly = gGameState.scissor.uly;
            mesStageClearCam->scissor.lrx = gGameState.scissor.lrx;
            mesStageClearCam->scissor.lry = gGameState.scissor.lry;
            eneSetup(obj, arg);
            w->model = eneNewPol(obj, NULL);
            if (w->model == NULL) {
                return -1;
            }
            w->count = 0;
            w->step = 1;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy((Frame *) w->model);
            }
            if (mesStageClearCam != NULL) {
                frmDestroy((Frame *) mesStageClearCam);
            }
            for (i = 0; i < 10; i++) {
                if (w->moji[i] != NULL) {
                    tskDestroy(w->moji[i]);
                }
            }
            break;
        case 3:
            break;
        case 0:
            eneStepHookPre(obj);
            switch (w->step) {
                case 0:
                default:
                    DPRINTF("mes_proc step ERROR!\n");
                    break;
                case 1:
                    if (w->timer % 5 == 1) {
                        w->moji[w->count] = eneMakeEx(obj, ENE_MES_STAGE_CLEAR_MOJI, 0, 0.0f, 0.0f, 0.0f, 2,
                                                      (u32 (*)(void *, void *, void *)) (u32) w->count);
                        w->count++;
                        if (w->count == 10) {
                            w->step++;
                        }
                    }
                    break;
                case 2:
                    if (w->timer == 78) {
                        for (i = 0; i < 10; i++) {
                            if (w->moji[i] != NULL) {
                                mesStageClearMojiSetStep(w->moji[i], 1);
                            }
                        }
                        w->step++;
                    }
                    break;
                case 3:
                    if (w->timer == 98) {
                        for (i = 0; i < 10; i++) {
                            if (w->moji[i] != NULL) {
                                mesStageClearMojiSetStep(w->moji[i], 2);
                            }
                        }
                        w->timer = 0;
                        w->step++;
                    }
                    break;
                case 4:
                    i = w->timer % 100;
                    if (i == 40) {
                        mesStageClearMojiSetStep(w->moji[0], 3);
                        mesStageClearMojiSetStep(w->moji[9], 3);
                    } else if (i == 45) {
                        mesStageClearMojiSetStep(w->moji[1], 3);
                        mesStageClearMojiSetStep(w->moji[8], 3);
                    } else if (i == 50) {
                        mesStageClearMojiSetStep(w->moji[2], 3);
                        mesStageClearMojiSetStep(w->moji[7], 3);
                    } else if (i == 55) {
                        mesStageClearMojiSetStep(w->moji[3], 3);
                        mesStageClearMojiSetStep(w->moji[6], 3);
                    } else if (i == 60) {
                        mesStageClearMojiSetStep(w->moji[4], 3);
                        mesStageClearMojiSetStep(w->moji[5], 3);
                    } else if (i == 70) {
                        w->step = 0;
                    }
                    break;
            }
            eneSetModelOnCourse(obj, (Frame *) w->model);
            w->timer++;
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A letter's handler ("chaST"). ONE index for every loop and for the afterimage picked in step 0
 * (a second index gives w another register than the game's s4); the pad is the word
 * above the two function-address temps (0x60 frame). In step 1 the x store is written first:
 * the game loads its table value before the step and timer stores. */
s32 mesStageClearMojiProc(Obj *obj, s16 msg, void *arg) {
    MesMojiWork *w = MESMOJI(obj);
    s32 i;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "chaST";
            DPRINTF("struct MES_MOJI_W size %d\n", sizeof(MesMojiWork));
            w->index = eneSetup(obj, arg);
            w->model = (PolFrame *) frmCreate((Frame *) mesStageClearCam, gFrmKindPol);
            if (w->model == NULL) {
                DPRINTF("chaST MOJI NULL\n");
                return -1;
            }
            w->model->dl2 = mesStageClearMojiDls[w->index];
            floadGetSetRam((void **) w->model->segs, 0x4A);
            w->model->scale[2] = 0.3f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->x = mesStageClearMojiStartX[w->index];
            w->y = mesStageClearMojiStartY[w->index];
            w->z = 274.0f;
            w->step = 0;
            w->alpha = 0;
            for (i = 0; i < 7; i++) {
                w->trail[i] = (PolFrame *) frmCreate((Frame *) mesStageClearCam, gFrmKindPol);
                if (w->trail[i] != NULL) {
                    floadGetSetRam((void **) w->trail[i]->segs, 0x4A);
                    w->trail[i]->pos[2] = 274.0f;
                } else {
                    DPRINTF("p[%d] is NULL\n", i);
                }
                w->trailAlpha[i] = 0;
            }
            break;
        case 3:
            w->model->dl = mesStageClearMojiDl(obj, w->alpha, 0xFF, 0xFF, 0xFF);
            for (i = 0; i < 7; i++) {
                if (w->trail[i] != NULL) {
                    w->trail[i]->dl = mesStageClearTrailDl(w->trailAlpha[i]);
                }
            }
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy((Frame *) w->model);
            }
            for (i = 0; i < 7; i++) {
                if (w->trail[i] != NULL) {
                    frmDestroy((Frame *) w->trail[i]);
                }
            }
            break;
        case 0:
            switch (w->step) {
                case 0:
                    w->x += (0.0f - mesStageClearMojiStartX[w->index]) / 12.0f;
                    w->y += (3.0f - mesStageClearMojiStartY[w->index]) / 12.0f;
                    w->model->scale[2] += -0.01666666754f;
                    w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                    i = w->timer % 7;
                    if (i != -1 && w->trail[i] != NULL) {
                        w->trail[i]->dl2 = mesStageClearMojiDls[w->index];
                        w->trail[i]->scale[0] = w->trail[i]->scale[1] = w->trail[i]->scale[2] =
                            w->model->scale[0];
                        w->trail[i]->pos[0] = w->x;
                        w->trail[i]->pos[1] = w->y;
                        w->trailAlpha[i] = 0xFF;
                    }
                    if (w->timer >= 12) {
                        w->x = 0.0f;
                        w->y = 3.0f;
                        w->step = 4;
                        w->alpha = 0xFF;
                        w->timer = 0;
                        for (i = 0; i < 7; i++) {
                            if (w->trail[i] != NULL) {
                                w->trail[i] = (PolFrame *) frmDestroy((Frame *) w->trail[i]);
                            }
                        }
                    }
                    break;
                case 1:
                    w->x += mesStageClearMojiPlaceX[w->index] / 6.0f;
                    if (w->timer >= 6) {
                        w->x = mesStageClearMojiPlaceX[w->index];
                        w->step = 4;
                        w->timer = 0;
                    }
                    break;
                case 2:
                    if (w->timer == 1) {
                        w->alpha = 0xFF;
                    }
                    w->alpha -= 10;
                    if (w->alpha < 10) {
                        w->alpha = 0;
                        w->step = 4;
                    }
                    break;
                case 3:
                    w->model->scale[1] -= 0.01;
                    if (w->index < 5) {
                        w->vel[0] += 0.2;
                    } else {
                        w->vel[0] -= 0.2;
                    }
                    if (w->model->scale[1] <= 0.0f) {
                        w->model->scale[1] = 0.0f;
                        return -1;
                    }
                    break;
                case 4:
                    if (w->alpha > 20) {
                        w->alpha -= 20;
                    } else {
                        w->alpha = 0;
                    }
                    w->timer = 0;
                    break;
            }
            for (i = 0; i < 7; i++) {
                if (w->trailAlpha[i] != 0) {
                    w->trailAlpha[i] -= 40;
                    if (w->trailAlpha[i] < 40) {
                        w->trailAlpha[i] = 0;
                    }
                }
            }
            eneMoveObj(obj);
            eneSetModel(obj, (Frame *) w->model);
            w->timer++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Gfx *mesStageClearMojiDl(Obj *obj, s32 a, s32 r, s32 g, s32 b) {
    Gfx *dl = gGfxDlHead;

    gSPDisplayList(gGfxDlHead++, fadeSegToRam(mesStageClearMojiSetupDl, obj->file));
    gDPSetEnvColor(gGfxDlHead++, r, g, b, a);
    gSPEndDisplayList(gGfxDlHead++);
    return dl;
}

Gfx *mesStageClearTrailDl(s32 a) {
    Gfx *dl = gGfxDlHead;

    gSPDisplayList(gGfxDlHead++, gDrawSetupXluSurfDl);
    gDPSetCombine(gGfxDlHead++, 0xFF9BFF, 0xFF34FE7F);
    gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, a);
    gSPEndDisplayList(gGfxDlHead++);
    return dl;
}

void mesStageClearMojiSetStep(Obj *obj, s32 step) {
    if (obj != NULL) {
        MESMOJI(obj)->step = step;
    }
}
