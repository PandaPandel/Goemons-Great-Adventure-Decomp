#include "tlb.h"

/* The third checkpoint's gate demo (demoSekisyo3): the gatekeepers check the player's passes
 * (tegata), step aside, the gate opens and the players run through; with its sea ("chiumi").
 * The twin of tlb_0122.c (the second checkpoint's), with its own constants. */

extern Frame *gFrmLayer3;
extern Gfx sekisyo3VramDl[];
extern u8 sekisyo3YakuninAnim[];
extern u8 sekisyo3YakuninWalkToAnim[];
extern u8 sekisyo3TobiraAnim[];
extern Gfx sekisyo3ChiumiSeaDl[];
extern u8 sekisyo3ChiumiTex[];

/* The gatekeepers' person data (talk 0x16, a path at sekisyo3YakuninPath, their standing and walking
 * animations sekisyo3YakuninAnim and sekisyo3YakuninWalkToAnim): passed with mode 2, so eneSetup returns it; the
 * gatekeeper's proc does not read it. */
extern u32 sekisyo3YakuninPath[];
u32 sekisyo3YakuninPath[] = { 0x0002005A, 0x00010000 };
u32 sekisyo3YakuninData[] = { 0x00000016, (u32) &sekisyo3YakuninPath, 0x00000000, 0x00000000, 0x00000000, (u32) sekisyo3YakuninAnim, (u32) sekisyo3YakuninWalkToAnim, 0x00000000, 0x00000000, 0x00000000 };

/* The demo's work: a demo talker's (PersonObj, demo.c) to 0x40, then its own. */
typedef struct Seki3DemoWork {
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
} Seki3DemoWork;

/* A gatekeeper's work: an enemy's (EneObj) to 0x34, then its own. */
typedef struct Seki3YakuWork {
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
} Seki3YakuWork;

/* The gate's work. */
typedef struct Seki3GateWork {
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
} Seki3GateWork;

/* The demo camera's work. */
typedef struct Seki3CamWork {
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
} Seki3CamWork;

/* The sea's work: an enemy's (EneObj) to 0x34, then its own. */
typedef struct Seki3UmiWork {
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
    /* 0x34 */ PolFrame *sea;
    /* 0x38 */ s32 scroll; /* the texture's, down one a frame (its low half is used) */
} Seki3UmiWork;

#define SEKI3DEMO(o) ((Seki3DemoWork *) ((u32) (o) + sizeof(Obj)))
#define SEKI3YAKU(o) ((Seki3YakuWork *) ((u32) (o) + sizeof(Obj)))
#define SEKI3GATE(o) ((Seki3GateWork *) ((u32) (o) + sizeof(Obj)))
#define SEKI3CAM(o) ((Seki3CamWork *) ((u32) (o) + sizeof(Obj)))
#define SEKI3UMI(o) ((Seki3UmiWork *) ((u32) (o) + sizeof(Obj)))

void sekisyo3YakuninSetFace(Obj *o, s32 on);
void sekisyo3YakuninWalkTo(Obj *o, f32 x, f32 z, f32 speed);
s32 sekisyo3YakuninIsBusy(Obj *o);
Obj *sekisyo3DemoGetTalker(Obj *o);
void sekisyo3TobiraOpen(Obj *o);
s32 sekisyo3TobiraIsOpening(Obj *o);
void sekisyo3CameraGoIn(Obj *o);
void sekisyo3CameraGoBack(Obj *o);
s32 sekisyo3CameraIsMoving(Obj *o);
Gfx *sekisyo3ChiumiDl(Gfx *gfx, u16 scroll);

/* "sekisyo3Vram": a "pol" model at the layer of gFrmLayer3; stops the sun's clock. */
s32 sekisyo3VramProc(Obj *o, s16 msg, void *arg) {
    EneObj *w = ENE(o);

    switch (msg) {
        case 2:
            o->name = "sekisyo3Vram";
            eneSetup(o, arg);
            w->model = (Frame *) eneNewPol(o, sekisyo3VramDl);
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

/* "demoSekisyo3_proc": the demo. Talked to, it stops the sun's clock, sets the camera, has the
 * gatekeepers face the player and runs scenario 0x1A; with the passes all held, they step
 * aside, the gate opens, scenario 0x1B runs, the players run through and the stage goes to
 * area 2 stage 5 on the atlas; without them, the camera goes back and the demo ends. */
s32 demoSekisyo3_proc(Obj *o, s16 msg, void *arg) {
    Seki3DemoWork *w = SEKI3DEMO(o);
    s32 pad[4];

    switch (msg) {
        case 2:
            o->name = "demoSekisyo3_proc";
            w->data = eneSetup(o, arg);
            if (demoTalkerMake(o, 0xE8) == -1) {
                return -1;
            }
            demoTalkInit(o, w->data);
            w->yaku1 = eneMakeEx(o, ENE_SEKISYO3_YAKUNIN, 1, 430.0f, -60.0f, -15.0f, 2,
                                 (u32 (*)(void *, void *, void *)) sekisyo3YakuninData);
            w->yaku2 = eneMakeEx(o, ENE_SEKISYO3_YAKUNIN, 1, 430.0f, -60.0f, 15.0f, 2,
                                 (u32 (*)(void *, void *, void *)) sekisyo3YakuninData);
            w->gate = eneMakeEx(o, ENE_SEKISYO3_TOBIRA, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
            w->camera = eneMakeEx(o, ENE_SEKISYO3_CAMERA, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
            fadeCreate(gEffectTask);
            w->block = eneMakeEx(o, ENE_DEMO_STOPPER, 0, -9.0f, 0.0f, 5.0f, 0, NULL);
            PRINTF("demoSekisyo3 セット完了\n");
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
                    sekisyo3CameraGoIn(w->camera);
                    demoPlStandAll();
                    PRINTF("MONBAN_TALK\n");
                    w->state = 2;
                    break;
                case 2:
                    if (sekisyo3CameraIsMoving(w->camera) == 0) {
                        sekisyo3YakuninSetFace(w->yaku1, 1);
                        sekisyo3YakuninSetFace(w->yaku2, 1);
                        scRunStart(0x1A, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        PRINTF("GO YAKU_WALK\n");
                        w->state = 3;
                    }
                    break;
                case 3:
                    if (scIsRunning() == 0) {
                        if (scTegataLeftMin(2) <= 0) {
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
                        sekisyo3YakuninSetFace(w->yaku1, 0);
                        sekisyo3YakuninSetFace(w->yaku2, 0);
                        sekisyo3YakuninWalkTo(w->yaku1, 490.0f, -5.0f, 0.8f);
                        sekisyo3YakuninWalkTo(w->yaku2, 490.0f, 5.0f, 0.8f);
                        PRINTF("GO MON_OPEN\n");
                        w->state = 5;
                    }
                    break;
                case 5:
                    if (sekisyo3YakuninIsBusy(w->yaku1) == 0 && sekisyo3YakuninIsBusy(w->yaku2) == 0) {
                        sekisyo3TobiraOpen(w->gate);
                        sndPlay(0x11A);
                        sekisyo3YakuninWalkTo(w->yaku1, 447.0f, -20.0f, 0.4f);
                        sekisyo3YakuninWalkTo(w->yaku2, 447.0f, 20.0f, 0.4f);
                        w->state = 6;
                    }
                    break;
                case 6:
                    if (sekisyo3TobiraIsOpening(w->gate) == 0) {
                        scRunStart(0x1B, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        sekisyo3YakuninSetFace(w->yaku1, 1);
                        sekisyo3YakuninSetFace(w->yaku2, 1);
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
                    gSaveData.stageFlags[2 * 16 + 5] |= 1;
                    gSaveData.stageFlags[30] |= 1;
                    PRINTF("GO DEMO_END\n");
                    stageGotoAtlas(2, 5);
                    break;
                case 10:
                    sekisyo3CameraGoBack(w->camera);
                    w->state = 11;
                    break;
                case 11:
                    if (sekisyo3CameraIsMoving(w->camera) == 0) {
                        demoPlReleaseAll();
                        sekisyo3YakuninSetFace(w->yaku1, 0);
                        sekisyo3YakuninSetFace(w->yaku2, 0);
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

/* "yaku3demo": a gatekeeper. Faces the player while flag 1 is set (its position asked of the
 * demo's talker with message 4), walks to (goalX, goalZ) while flag 2 is, else turns to 90
 * degrees. */
s32 sekisyo3YakuninProc(Obj *o, s16 msg, void *arg) {
    Seki3YakuWork *w = SEKI3YAKU(o);
    Vec3f pos;

    switch (msg) {
        case 2:
            o->name = "yaku3demo";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) sekisyo3YakuninAnim);
            if (eneSetOnEarth(o) == -1) {
                PRINTF("sekisyo3Yakunin: Error: 地面にセット失敗\n");
                return -1;
            }
            eneSetAngle(o, 90.0f);
            w->shadow = shadowCreate();
            PRINTF("sekisyo3Yakunin セット完了\n");
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
                tskSendMsg(sekisyo3DemoGetTalker(o->parent), 4, &pos);
                eneTurnToPoint(o, pos.x, pos.z, gRetracesPerFrame * 5);
            } else if (w->flags & 2) {
                if (eneMoveToXZ(o, w->goalX, w->goalZ, w->speed / gRetracesPerFrame, w->speed / gRetracesPerFrame) == 1) {
                    w->flags &= ~2;
                    personAnimeSet(o, (u32) sekisyo3YakuninAnim);
                } else {
                    eneFaceMoveDir(o);
                }
            } else {
                eneTurnToAngle(o, 90.0f, 5.0f);
            }
            eneAnimLoop(o, w->model, 0.6f);
            eneMove((EneObj *) w);
            eneSetModelOnCourse(o, &w->model->f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* A gatekeeper's facing the player, on (1) or off. */
void sekisyo3YakuninSetFace(Obj *o, s32 on) {
    Seki3YakuWork *w = SEKI3YAKU(o);

    if (on == 1) {
        w->flags |= 1;
    } else {
        w->flags &= ~1;
    }
}

/* A gatekeeper's walk to (x, z) at speed, with its walking animation. */
void sekisyo3YakuninWalkTo(Obj *o, f32 x, f32 z, f32 speed) {
    Seki3YakuWork *w = SEKI3YAKU(o);

    w->flags |= 2;
    w->goalX = x;
    w->goalZ = z;
    w->speed = speed;
    personAnimeSet(o, (u32) sekisyo3YakuninWalkToAnim);
}

/* Whether a gatekeeper is busy (facing the player or walking). */
s32 sekisyo3YakuninIsBusy(Obj *o) {
    if (SEKI3YAKU(o)->flags != 0) {
        return 1;
    }
    return 0;
}

/* The demo's talker (the player it is talking to), from the demo task. */
Obj *sekisyo3DemoGetTalker(Obj *o) {
    return SEKI3DEMO(o)->talker;
}

/* "tobidemo3": the gate. Opened (flag 1), it plays its animation once. */
s32 sekisyo3TobiraProc(Obj *o, s16 msg, void *arg) {
    s32 pad;
    Seki3GateWork *w = SEKI3GATE(o);

    switch (msg) {
        case 2:
            o->name = "tobidemo3";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) sekisyo3TobiraAnim);
            frmMove(&w->model->f, gFrmLayer3);
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 1.0f;
            PRINTF("sekisyo3Tobira セット完了\n");
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
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
void sekisyo3TobiraOpen(Obj *o) {
    SEKI3GATE(o)->open |= 1;
}

/* Whether the gate is opening. */
s32 sekisyo3TobiraIsOpening(Obj *o) {
    if (SEKI3GATE(o)->open != 0) {
        return 1;
    }
    return 0;
}

/* "camera3": the demo's camera. State 0 keeps the view inside the gate's area; 1 moves it to
 * the gatekeepers (then 2, holding); 3 moves it back (then 4, waiting for it to stop, then the
 * camera AI's own again). */
s32 sekisyo3CameraProc(Obj *o, s16 msg, void *arg) {
    Seki3CamWork *w = SEKI3CAM(o);
    CamView *v;

    switch (msg) {
        case 2:
            o->name = "camera3";
            eneSetup(o, arg);
            PRINTF("sekisyo3Camera セット完了\n");
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
                    if (v->eye[0] > 222.0f) {
                        v->eye[0] = 222.0f;
                        v->at[0] = 222.0f;
                    }
                    if (v->eye[0] < 39.0f) {
                        v->eye[0] = 39.0f;
                    }
                    if (v->at[0] < -44.0f) {
                        v->at[0] = -44.0f;
                    }
                    v->eye[1] = -21.0f;
                    v->eye[2] = -10.0f;
                    v->at[1] = -64.0f;
                    v->at[2] = 144.0f;
                    v->zNear = 20.0f;
                    v->zFar = 800.0f;
                    v->fovy = 0.6981317401f;
                    break;
                case 1:
                    camSetSpeed(w->cam, 40 / gRetracesPerFrame);
                    camMoveEye(w->cam, 27.0f, -64.0f, 99.0f);
                    camMoveAt(w->cam, -122.0f, -32.0f, 134.0f);
                    w->state = 2;
                    break;
                case 2:
                    break;
                case 3:
                    camSetSpeed(w->cam, 40 / gRetracesPerFrame);
                    camMoveEye(w->cam, 39.0f, -21.0f, -10.0f);
                    camMoveAt(w->cam, -44.0f, -64.0f, 144.0f);
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

/* An empty function nothing calls (its "jr ra; nop" at 0x08001024). */
void sekisyo3Dummy(void) {
}

/* Holds the camera AI and moves the demo camera to the gatekeepers (its state 1). */
void sekisyo3CameraGoIn(Obj *o) {
    camAiSetHold(1);
    SEKI3CAM(o)->state = 1;
}

/* Moves the demo camera back (its state 3). */
void sekisyo3CameraGoBack(Obj *o) {
    SEKI3CAM(o)->state = 3;
}

/* Whether the demo camera is moving. */
s32 sekisyo3CameraIsMoving(Obj *o) {
    return camGetMoving(SEKI3CAM(o)->cam);
}

/* A 64x32 RGBA16 texture loaded into TMEM at tmem for tile rtile, as gDPLoadMultiBlock, with its
 * tile size at (uls, ult): one macro, as tlb_0050's 32x32 one. */
#define gDPLoadScrollBlock64x32(pkt, timg, tmem, rtile, uls, ult)                                     \
    {                                                                                                  \
        gDPSetTextureImage(pkt, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, timg);                                 \
        gDPSetTile(pkt, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, tmem, G_TX_LOADTILE, 0,                        \
                   G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD); \
        gDPLoadSync(pkt);                                                                              \
        gDPLoadBlock(pkt, G_TX_LOADTILE, 0, 0, 64 * 32 - 1, CALC_DXT(64, G_IM_SIZ_16b_BYTES));        \
        gDPPipeSync(pkt);                                                                              \
        gDPSetTile(pkt, G_IM_FMT_RGBA, G_IM_SIZ_16b, 16, tmem, rtile, 0, G_TX_WRAP | G_TX_NOMIRROR, 5, \
                   G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD);                              \
        gDPSetTileSize(pkt, rtile, uls, ult, (64 - 1) << 2, (32 - 1) << 2);                            \
    }

/* The sea's display list: its texture twice (tiles 0 and 1), scrolled against each other. */
Gfx *sekisyo3ChiumiDl(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 128, 128, 128, 128);
    gDPLoadScrollBlock64x32(gfx++, sekisyo3ChiumiTex, 0, G_TX_RENDERTILE, scroll, scroll);
    gDPLoadScrollBlock64x32(gfx++, sekisyo3ChiumiTex, 0x100, 1, scroll + 10, -scroll);
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* "chiumi": the sea, a "pol" model whose texture scrolls each draw (message 3). */
s32 sekisyo3ChiumiProc(Obj *o, s16 msg, void *arg) {
    Seki3UmiWork *w = SEKI3UMI(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "chiumi";
            eneSetup(o, arg);
            w->sea = eneNewPol(o, sekisyo3ChiumiSeaDl);
            if (w->sea == NULL) {
                return -1;
            }
            frmMove(&w->sea->f, gFrmLayers[6]);
            w->sea->scale[0] = w->sea->scale[1] = w->sea->scale[2] = 1.0f;
            break;
        case 1:
            if (w->sea != NULL) {
                frmDestroy(&w->sea->f);
            }
            break;
        case 3:
            w->sea->dl = gGfxDlHead;
            gGfxDlHead = sekisyo3ChiumiDl(gGfxDlHead, w->scroll);
            w->scroll--;
            eneSetModelOnCourse(o, &w->sea->f);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
