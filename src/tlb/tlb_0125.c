#include "tlb.h"

/* The fifth checkpoint's gate demo (demoSekisyo5): the gatekeepers check the player's passes
 * (tegata), open the gate and the players run through. The same code as tlb_0124 (sekisyo4)
 * with this checkpoint's places, and no water. */

extern Frame *gFrmLayer3;
extern Gfx sekisyo5VramDl[];
extern u8 sekisyo5YakuninAnim[];
extern u8 sekisyo5YakuninWalkToAnim[];
extern u8 sekisyo5TobiraAnim[];

/* The gatekeepers' person data (talk 0x16, a path at sekisyo5YakuninPath, their standing and walking
 * animations sekisyo5YakuninAnim and sekisyo5YakuninWalkToAnim): passed with mode 2, so eneSetup returns it. */
extern u32 sekisyo5YakuninPath[];
Gfx sekisyo5UnusedDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsDPPipelineMode(G_PM_NPRIMITIVE),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetCombineKey(G_CK_NONE),
    gsDPSetTextureLOD(G_TL_TILE),
    gsDPSetTextureDetail(G_TD_CLAMP),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsDPSetTextureLUT(G_TT_NONE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_FOG | G_SHADING_SMOOTH),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_TEX_EDGE2),
    gsDPSetCombineLERP(ENVIRONMENT, TEXEL0, ENV_ALPHA, TEXEL0, 0, 0, 0, TEXEL0, COMBINED, 0, SHADE, 0, 0, 0, 0, COMBINED),
    gsSPEndDisplayList(),
};
u32 sekisyo5YakuninPath[] = { 0x0002005A, 0x00010000 };
u32 sekisyo5YakuninData[] = { 0x00000016, (u32) &sekisyo5YakuninPath, 0x00000000, 0x00000000, 0x00000000, (u32) sekisyo5YakuninAnim, (u32) sekisyo5YakuninWalkToAnim, 0x00000000, 0x00000000, 0x00000000 };

/* The demo's work: a demo talker's (PersonObj, demo.c) to 0x40, then its own. */
typedef struct Seki5DemoWork {
    /* 0x00 */ u8 head[0x40];
    /* 0x40 */ Obj *talker; /* the player talking to it (demoTalk) */
    /* 0x44 */ Obj *camera;
    /* 0x48 */ Obj *yaku1; /* the gatekeepers */
    /* 0x4C */ Obj *yaku2;
    /* 0x50 */ Obj *gate;
    /* 0x54 */ u32 data; /* eneSetup's: its talk data */
    /* 0x58 */ u8 state;
    /* 0x5C */ Obj *block; /* kind 0x19A, gone when the gate opens */
    /* 0x60 */ s32 timer;
} Seki5DemoWork;

/* A gatekeeper's work: an enemy's (EneObj) to 0x34, then its own. */
typedef struct Seki5YakuWork {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u8 flags; /* 1 faces the player, 2 walks to (goalX, goalZ) */
    /* 0x38 */ f32 goalX;
    /* 0x3C */ f32 goalZ;
    /* 0x40 */ f32 speed;
    /* 0x44 */ MPolFrame *shadow;
} Seki5YakuWork;

/* The gate's work. */
typedef struct Seki5GateWork {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u8 open; /* 1 while it opens */
} Seki5GateWork;

/* The demo camera's work. */
typedef struct Seki5CamWork {
    /* 0x00 */ u8 unused00;
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
    /* 0x34 */ Camera *cam;
    /* 0x38 */ u8 state;
} Seki5CamWork;

#define SEKI5DEMO(o) ((Seki5DemoWork *) ((u32) (o) + sizeof(Obj)))
#define SEKI5YAKU(o) ((Seki5YakuWork *) ((u32) (o) + sizeof(Obj)))
#define SEKI5GATE(o) ((Seki5GateWork *) ((u32) (o) + sizeof(Obj)))
#define SEKI5CAM(o) ((Seki5CamWork *) ((u32) (o) + sizeof(Obj)))

void sekisyo5YakuninSetFace(Obj *o, s32 on);
void sekisyo5YakuninWalkTo(Obj *o, f32 x, f32 z, f32 speed);
s32 sekisyo5YakuninIsBusy(Obj *o);
Obj *sekisyo5DemoGetTalker(Obj *o);
void sekisyo5TobiraOpen(Obj *o);
s32 sekisyo5TobiraIsOpening(Obj *o);
void sekisyo5CameraGoIn(Obj *o);
void sekisyo5CameraGoBack(Obj *o);
s32 sekisyo5CameraIsMoving(Obj *o);

/* "sekisyo5Vram": a "pol" model at the layer of gFrmLayer3; stops the sun's clock. */
s32 sekisyo5VramProc(Obj *o, s16 msg, void *arg) {
    EneObj *w = ENE(o);

    switch (msg) {
        case 2:
            o->name = "sekisyo5Vram";
            eneSetup(o, arg);
            w->model = (Frame *) eneNewPol(o, sekisyo5VramDl);
            ((PolFrame *) w->model)->scale[0] = ((PolFrame *) w->model)->scale[1] =
                ((PolFrame *) w->model)->scale[2] = 1.0f;
            frmMove(w->model, gFrmLayer3);
            camSunSetClockStop(1);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(w->model);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneSetModel(o, w->model);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "d_Sekisyo5_proc": the demo. Talked to, it stops the sun's clock, sets the camera, has the
 * gatekeepers face the player and runs scenario 0x1E; with the passes all held, they step
 * aside, the gate opens, scenario 0x1F runs, the players run through and the stage goes to
 * area 4 stage 4 on the atlas; without them, the camera goes back and the demo ends. */
s32 d_Sekisyo5_proc(Obj *o, s16 msg, void *arg) {
    Seki5DemoWork *w = SEKI5DEMO(o);
    s32 pad[4];

    switch (msg) {
        case 2:
            o->name = "d_Sekisyo5_proc";
            w->data = eneSetup(o, arg);
            if (demoTalkerMake(o, 0xE8) == -1) {
                return -1;
            }
            demoTalkInit(o, w->data);
            w->yaku1 = eneMakeEx(o, ENE_SEKISYO5_YAKUNIN, 1, 405.0f, -63.0f, 12.0f, 2,
                                 (u32 (*)(void *, void *, void *)) sekisyo5YakuninData);
            w->yaku2 = eneMakeEx(o, ENE_SEKISYO5_YAKUNIN, 1, 410.0f, -63.0f, -20.0f, 2,
                                 (u32 (*)(void *, void *, void *)) sekisyo5YakuninData);
            w->gate = eneMakeEx(o, ENE_SEKISYO5_TOBIRA, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
            w->camera = eneMakeEx(o, ENE_SEKISYO5_CAMERA, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
            fadeCreate(gEffectTask);
            w->block = eneMakeEx(o, ENE_DEMO_STOPPER, 0, -9.0f, 0.0f, 5.0f, 0, NULL);
            PRINTF("demoSekisyo5 セット完了\n");
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            demoTalkerFree(o);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (demoTalkEndCheck(o)) {
                        camSunSetClockStop(1);
                        PRINTF("デモ始まります GO CAM_PL_SET\n");
                        w->state = 1;
                    }
                    break;
                case 1:
                    sekisyo5CameraGoIn(w->camera);
                    demoPlStandAll();
                    PRINTF("MONBAN_TALK\n");
                    w->state = 2;
                    break;
                case 2:
                    if (sekisyo5CameraIsMoving(w->camera) == 0) {
                        sekisyo5YakuninSetFace(w->yaku1, 1);
                        sekisyo5YakuninSetFace(w->yaku2, 1);
                        scRunStart(0x1E, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        PRINTF("GO YAKU_WALK\n");
                        w->state = 3;
                    }
                    break;
                case 3:
                    if (scIsRunning() == 0) {
                        if (scTegataLeftMin(4) <= 0) {
                            PRINTF("手形が揃った\n");
                            w->state = 4;
                        } else {
                            PRINTF("手形が揃ってない\n");
                            w->state = 10;
                        }
                    }
                    break;
                case 4:
                    if (scIsRunning() == 0) {
                        sekisyo5YakuninSetFace(w->yaku1, 0);
                        sekisyo5YakuninSetFace(w->yaku2, 0);
                        sekisyo5YakuninWalkTo(w->yaku1, 459.0f, 12.0f, 0.8f);
                        sekisyo5YakuninWalkTo(w->yaku2, 467.0f, -7.0f, 0.8f);
                        PRINTF("GO MON_OPEN\n");
                        w->state = 5;
                    }
                    break;
                case 5:
                    if (sekisyo5YakuninIsBusy(w->yaku1) == 0 && sekisyo5YakuninIsBusy(w->yaku2) == 0) {
                        sekisyo5TobiraOpen(w->gate);
                        sndPlay(0x11A);
                        sekisyo5YakuninWalkTo(w->yaku1, 421.0f, 30.0f, 0.4f);
                        sekisyo5YakuninWalkTo(w->yaku2, 427.0f, -22.0f, 0.4f);
                        w->state = 6;
                    }
                    break;
                case 6:
                    if (sekisyo5TobiraIsOpening(w->gate) == 0) {
                        scRunStart(0x1F, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        sekisyo5YakuninSetFace(w->yaku1, 1);
                        sekisyo5YakuninSetFace(w->yaku2, 1);
                        tskDestroy(w->block);
                        PRINTF("GO PL_WALK_AWAY\n");
                        w->state = 7;
                    }
                    break;
                case 7:
                    if (scIsRunning() == 0) {
                        demoSetSpeedPlRun4P(838.0f, -65.0f, 0.0f, 3.0f, 838.0f, -65.0f, 0.0f, 3.0f, 838.0f,
                                            -65.0f, 0.0f, 3.0f, 838.0f, -65.0f, 0.0f, 3.0f);
                        PRINTF("PL_WALK_AWAY\n");
                        w->timer = 0;
                        w->state = 8;
                    }
                    break;
                case 8:
                    if (fadeCloseLetterbox(120) == 1) {
                        demoPlStandAll();
                        PRINTF("FADE_OUT\n");
                        w->state = 9;
                    }
                    if (w->timer == 156 / gRetracesPerFrame) {
                        sndPlay(10);
                    }
                    break;
                case 9:
                    camSunSetClockStop(0);
                    camAiSetHold(0);
                    gSaveData.stageFlags[4 * 16 + 4] |= 1;
                    gSaveData.stageFlags[3 * 16 + 13] |= 1;
                    PRINTF("GO DEMO_END\n");
                    stageGotoAtlas(4, 4);
                    break;
                case 10:
                    sekisyo5CameraGoBack(w->camera);
                    w->state = 11;
                    break;
                case 11:
                    if (sekisyo5CameraIsMoving(w->camera) == 0) {
                        demoPlReleaseAll();
                        sekisyo5YakuninSetFace(w->yaku1, 0);
                        sekisyo5YakuninSetFace(w->yaku2, 0);
                        demoFinish(o);
                        w->state = 0;
                    }
                    break;
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "yaku5demo": a gatekeeper. Faces the player while flag 1 is set (its position asked of the
 * demo's talker with message 4), walks to (goalX, goalZ) while flag 2 is, else turns to 90
 * degrees. */
s32 sekisyo5YakuninProc(Obj *o, s16 msg, void *arg) {
    Seki5YakuWork *w = SEKI5YAKU(o);
    Vec3f pos;

    switch (msg) {
        case 2:
            o->name = "yaku5demo";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) sekisyo5YakuninAnim);
            if (eneSetOnEarth(o) == -1) {
                PRINTF("sekisyo5Yakunin: Error: 地面にセット失敗\n");
                return -1;
            }
            eneSetAngle(o, 90.0f);
            w->shadow = shadowCreate();
            PRINTF("sekisyo5Yakunin セット完了\n");
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            if (w->flags & 1) {
                tskSendMsg(sekisyo5DemoGetTalker(o->parent), 4, &pos);
                eneTurnToPoint(o, pos.x, pos.z, gRetracesPerFrame * 5);
            } else if (w->flags & 2) {
                if (eneMoveToXZ(o, w->goalX, w->goalZ, w->speed / gRetracesPerFrame, w->speed / gRetracesPerFrame) == 1) {
                    w->flags &= ~2;
                    personAnimeSet(o, (u32) sekisyo5YakuninAnim);
                } else {
                    eneFaceMoveDir(o);
                }
            } else {
                eneTurnToAngle(o, 90.0f, 5.0f);
            }
            eneAnimLoop(o, w->model, 1.0f);
            eneMove((EneObj *) w);
            eneSetModelOnCourse(o, &w->model->f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* A gatekeeper's facing the player, on (1) or off. */
void sekisyo5YakuninSetFace(Obj *o, s32 on) {
    Seki5YakuWork *w = SEKI5YAKU(o);

    if (on == 1) {
        w->flags |= 1;
    } else {
        w->flags &= ~1;
    }
}

/* A gatekeeper's walk to (x, z) at speed, with its walking animation. */
void sekisyo5YakuninWalkTo(Obj *o, f32 x, f32 z, f32 speed) {
    Seki5YakuWork *w = SEKI5YAKU(o);

    w->flags |= 2;
    w->goalX = x;
    w->goalZ = z;
    w->speed = speed;
    personAnimeSet(o, (u32) sekisyo5YakuninWalkToAnim);
}

/* Whether a gatekeeper is busy (facing the player or walking). */
s32 sekisyo5YakuninIsBusy(Obj *o) {
    if (SEKI5YAKU(o)->flags != 0) {
        return 1;
    }
    return 0;
}

/* The demo's talker (the player it is talking to), from the demo task. */
Obj *sekisyo5DemoGetTalker(Obj *o) {
    return SEKI5DEMO(o)->talker;
}

/* "tobi5": the gate. Opened (flag 1), it plays its animation once. */
s32 sekisyo5TobiraProc(Obj *o, s16 msg, void *arg) {
    s32 pad;
    Seki5GateWork *w = SEKI5GATE(o);

    switch (msg) {
        case 2:
            o->name = "tobi5";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) sekisyo5TobiraAnim);
            frmMove(&w->model->f, gFrmLayer3);
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 1.0f;
            PRINTF("sekisyo5Tobira セット完了\n");
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
        case 3:
            break;
        case 0:
            eneStepHookPre(o);
            if (w->open && eneAnimPlayOnce(o, w->model, 0.05f) == 1) {
                w->open &= ~1;
            }
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Opens the gate. */
void sekisyo5TobiraOpen(Obj *o) {
    SEKI5GATE(o)->open |= 1;
}

/* Whether the gate is opening. */
s32 sekisyo5TobiraIsOpening(Obj *o) {
    if (SEKI5GATE(o)->open != 0) {
        return 1;
    }
    return 0;
}

/* "cam5": the demo's camera. State 0 keeps the view inside the gate's area; 1 moves it to
 * the gatekeepers (then 2, holding); 3 moves it back (then 4, waiting for it to stop, then the
 * camera AI's own again). */
s32 sekisyo5CameraProc(Obj *o, s16 msg, void *arg) {
    Seki5CamWork *w = SEKI5CAM(o);
    CamView *v;

    switch (msg) {
        case 2:
            o->name = "cam5";
            eneSetup(o, arg);
            PRINTF("sekisyo5Camera セット完了\n");
            break;
        case 1:
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                default:
                    w->cam = camAiGetCam();
                    v = camGetView(w->cam);
                    if (v->eye[0] > 286.0f) {
                        v->eye[0] = 286.0f;
                        v->at[0] = 286.0f;
                    }
                    if (v->eye[0] < 47.0f) {
                        v->eye[0] = 47.0f;
                    }
                    if (v->at[0] < -49.0f) {
                        v->at[0] = -49.0f;
                    }
                    v->eye[2] = 0.0f;
                    v->zNear = 20.0f;
                    v->zFar = 800.0f;
                    v->fovy = 0.6981317401f;
                    break;
                case 1:
                    camSetSpeed(w->cam, 40 / gRetracesPerFrame);
                    camMoveEye(w->cam, 58.0f, -62.0f, 47.0f);
                    camMoveAt(w->cam, -78.0f, -31.0f, 125.0f);
                    w->state = 2;
                    break;
                case 2:
                    break;
                case 3:
                    camSetSpeed(w->cam, 40 / gRetracesPerFrame);
                    camMoveEye(w->cam, 47.0f, -31.0f, 0.0f);
                    camMoveAt(w->cam, -22.0f, -54.0f, 117.0f);
                    w->state = 4;
                    break;
                case 4:
                    if (camGetMoving(w->cam) == 0) {
                        camAiSetHold(0);
                        w->state = 0;
                    }
                    break;
            }
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* An empty function nothing calls: splat folded it into the one above (its "jr ra; nop" at
 * 0x08001024). */
void sekisyo5Dummy(void) {
}

/* Holds the camera AI and moves the demo camera to the gatekeepers (its state 1). */
void sekisyo5CameraGoIn(Obj *o) {
    camAiSetHold(1);
    SEKI5CAM(o)->state = 1;
}

/* Moves the demo camera back (its state 3). */
void sekisyo5CameraGoBack(Obj *o) {
    SEKI5CAM(o)->state = 3;
}

/* Whether the demo camera is moving. */
s32 sekisyo5CameraIsMoving(Obj *o) {
    return camGetMoving(SEKI5CAM(o)->cam);
}
