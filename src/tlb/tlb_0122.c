#include "tlb.h"

/* The second checkpoint's gate demo (demoSekisyo2): the gatekeepers check the player's passes
 * (tegata), open the gate and the players run through; with its sea ("umi"). */

extern Frame *gFrmLayer3;

extern Gfx sekisyo2VramDl[];
extern u8 sekisyo2YakuninAnim[];
extern u8 sekisyo2YakuninWalkToAnim[];
extern u8 sekisyo2TobiraAnim[];
extern Gfx sekisyo2UmiSeaDl[];
extern u8 sekisyo2UmiTex[];

/* The gatekeepers' person data (talk 0x16, a path at sekisyo2YakuninPath, their standing and walking
 * animations sekisyo2YakuninAnim and sekisyo2YakuninWalkToAnim): passed with mode 2, so eneSetup returns it; the
 * gatekeeper's proc does not read it. */
extern u32 sekisyo2YakuninPath[];
u32 sekisyo2YakuninPath[] = { 0x0002005A, 0x00010000 };
u32 sekisyo2YakuninData[] = { 0x00000016, (u32) &sekisyo2YakuninPath, 0x00000000, 0x00000000, 0x00000000, (u32) sekisyo2YakuninAnim, (u32) sekisyo2YakuninWalkToAnim, 0x00000000, 0x00000000, 0x00000000 };

/* The demo's work: a demo talker's (PersonObj, demo.c) to 0x40, then its own. */
typedef struct Seki2DemoWork {
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
} Seki2DemoWork;

/* A gatekeeper's work: an enemy's (EneObj) to 0x34, then its own. */
typedef struct Seki2YakuWork {
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
} Seki2YakuWork;

/* The gate's work. */
typedef struct Seki2GateWork {
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
} Seki2GateWork;

/* The demo camera's work. */
typedef struct Seki2CamWork {
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
} Seki2CamWork;

/* The sea's work: an enemy's (EneObj) to 0x34, then its own. */
typedef struct Seki2UmiWork {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ Frame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *sea;
    /* 0x38 */ s32 scroll; /* the texture's, down one a frame (its low half is used) */
    /* 0x3C */ s32 count; /* steps, 0-119: the swell's phase */
} Seki2UmiWork;

#define SEKI2DEMO(o) ((Seki2DemoWork *) ((u32) (o) + sizeof(Obj)))
#define SEKI2YAKU(o) ((Seki2YakuWork *) ((u32) (o) + sizeof(Obj)))
#define SEKI2GATE(o) ((Seki2GateWork *) ((u32) (o) + sizeof(Obj)))
#define SEKI2CAM(o) ((Seki2CamWork *) ((u32) (o) + sizeof(Obj)))
#define SEKI2UMI(o) ((Seki2UmiWork *) ((u32) (o) + sizeof(Obj)))

void sekisyo2YakuninSetFace(Obj *o, s32 on);
void sekisyo2YakuninWalkTo(Obj *o, f32 x, f32 z, f32 speed);
s32 sekisyo2YakuninIsBusy(Obj *o);
Obj *sekisyo2DemoGetTalker(Obj *o);
void sekisyo2TobiraOpen(Obj *o);
s32 sekisyo2TobiraIsOpening(Obj *o);
void sekisyo2CameraGoIn(Obj *o);
void sekisyo2CameraGoBack(Obj *o);
s32 sekisyo2CameraIsMoving(Obj *o);
Gfx *sekisyo2UmiDl(Gfx *gfx, u16 scroll);

/* "seki2Vram": a "pol" model at the layer of gFrmLayer3; stops the sun's clock. */
s32 sekisyo2VramProc(Obj *o, s16 msg, void *arg) {
    EneObj *w = ENE(o);

    switch (msg) {
        case 2:
            o->name = "seki2Vram";
            eneSetup(o, arg);
            w->model = (Frame *) eneNewPol(o, sekisyo2VramDl);
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

/* "demoSekisyo2_proc": the demo. Talked to, it stops the sun's clock, sets the camera, has the
 * gatekeepers face the player and runs scenario 0x18; with the passes all held, they step
 * aside, the gate opens, scenario 0x19 runs, the players run through and the stage goes to
 * area 1 stage 4 on the atlas; without them, the camera goes back and the demo ends. */
s32 demoSekisyo2_proc(Obj *o, s16 msg, void *arg) {
    Seki2DemoWork *w = SEKI2DEMO(o);
    s32 pad[4];

    switch (msg) {
        case 2:
            o->name = "demoSekisyo2_proc";
            w->data = eneSetup(o, arg);
            if (demoTalkerMake(o, 0xE8) == -1) {
                return -1;
            }
            demoTalkInit(o, w->data);
            w->yaku1 = eneMakeEx(o, ENE_SEKISYO2_YAKUNIN, 1, 430.0f, -60.0f, -20.0f, 2,
                                 (u32 (*)(void *, void *, void *)) sekisyo2YakuninData);
            w->yaku2 = eneMakeEx(o, ENE_SEKISYO2_YAKUNIN, 1, 430.0f, -60.0f, 14.0f, 2,
                                 (u32 (*)(void *, void *, void *)) sekisyo2YakuninData);
            w->gate = eneMakeEx(o, ENE_SEKISYO2_TOBIRA, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
            w->camera = eneMakeEx(o, ENE_SEKISYO2_CAMERA, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
            fadeCreate(gEffectTask);
            w->block = eneMakeEx(o, ENE_DEMO_STOPPER, 0, -9.0f, 0.0f, 5.0f, 0, NULL);
            PRINTF("demoSekisyo2 セット完了\n");
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
                    sekisyo2CameraGoIn(w->camera);
                    demoPlStandAll();
                    PRINTF("MONBAN_TALK\n");
                    w->state = 2;
                    break;
                case 2:
                    if (sekisyo2CameraIsMoving(w->camera) == 0) {
                        sekisyo2YakuninSetFace(w->yaku1, 1);
                        sekisyo2YakuninSetFace(w->yaku2, 1);
                        scRunStart(0x18, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        PRINTF("GO TEGATA_CHECK\n");
                        w->state = 3;
                    }
                    break;
                case 3:
                    if (scIsRunning() == 0) {
                        if (scTegataLeftMin(1) <= 0) {
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
                        sekisyo2YakuninSetFace(w->yaku1, 0);
                        sekisyo2YakuninSetFace(w->yaku2, 0);
                        sekisyo2YakuninWalkTo(w->yaku1, 488.0f, -7.0f, 0.8f);
                        sekisyo2YakuninWalkTo(w->yaku2, 488.0f, 7.0f, 0.8f);
                        PRINTF("GO MON_OPEN\n");
                        w->state = 5;
                    }
                    break;
                case 5:
                    if (sekisyo2YakuninIsBusy(w->yaku1) == 0 && sekisyo2YakuninIsBusy(w->yaku2) == 0) {
                        sekisyo2TobiraOpen(w->gate);
                        sndPlay(0x11A);
                        sekisyo2YakuninWalkTo(w->yaku1, 449.0f, -19.0f, 0.35f);
                        sekisyo2YakuninWalkTo(w->yaku2, 449.0f, 19.0f, 0.35f);
                        w->state = 6;
                    }
                    break;
                case 6:
                    if (sekisyo2TobiraIsOpening(w->gate) == 0 && sekisyo2YakuninIsBusy(w->yaku1) == 0 &&
                        sekisyo2YakuninIsBusy(w->yaku2) == 0) {
                        scRunStart(0x19, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        sekisyo2YakuninSetFace(w->yaku1, 1);
                        sekisyo2YakuninSetFace(w->yaku2, 1);
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
                    gSaveData.stageFlags[1 * 16 + 4] |= 1;
                    gSaveData.stageFlags[13] |= 1;
                    PRINTF("GO DEMO_END\n");
                    stageGotoAtlas(1, 4);
                    break;
                case 10:
                    sekisyo2CameraGoBack(w->camera);
                    w->state = 11;
                    break;
                case 11:
                    if (sekisyo2CameraIsMoving(w->camera) == 0) {
                        demoPlReleaseAll();
                        sekisyo2YakuninSetFace(w->yaku1, 0);
                        sekisyo2YakuninSetFace(w->yaku2, 0);
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

/* "yaku2demo": a gatekeeper. Faces the player while flag 1 is set (its position asked of the
 * demo's talker with message 4), walks to (goalX, goalZ) while flag 2 is, else turns to 90
 * degrees. */
s32 sekisyo2YakuninProc(Obj *o, s16 msg, void *arg) {
    Seki2YakuWork *w = SEKI2YAKU(o);
    Vec3f pos;

    switch (msg) {
        case 2:
            o->name = "yaku2demo";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) sekisyo2YakuninAnim);
            if (eneSetOnEarth(o) == -1) {
                PRINTF("sekisyo2Yakunin: Error: 地面にセット失敗\n");
                return -1;
            }
            eneSetAngle(o, 90.0f);
            w->shadow = shadowCreate();
            PRINTF("sekisyo2Yakunin セット完了\n");
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
                tskSendMsg(sekisyo2DemoGetTalker(o->parent), 4, &pos);
                eneTurnToPoint(o, pos.x, pos.z, gRetracesPerFrame * 2);
            } else if (w->flags & 2) {
                if (eneMoveToXZ(o, w->goalX, w->goalZ, w->speed / gRetracesPerFrame, w->speed / gRetracesPerFrame) == 1) {
                    w->flags &= ~2;
                    personAnimeSet(o, (u32) sekisyo2YakuninAnim);
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
void sekisyo2YakuninSetFace(Obj *o, s32 on) {
    Seki2YakuWork *w = SEKI2YAKU(o);

    if (on == 1) {
        w->flags |= 1;
    } else {
        w->flags &= ~1;
    }
}

/* A gatekeeper's walk to (x, z) at speed, with its walking animation. */
void sekisyo2YakuninWalkTo(Obj *o, f32 x, f32 z, f32 speed) {
    Seki2YakuWork *w = SEKI2YAKU(o);

    w->flags |= 2;
    w->goalX = x;
    w->goalZ = z;
    w->speed = speed;
    personAnimeSet(o, (u32) sekisyo2YakuninWalkToAnim);
}

/* Whether a gatekeeper is busy (facing the player or walking). */
s32 sekisyo2YakuninIsBusy(Obj *o) {
    if (SEKI2YAKU(o)->flags != 0) {
        return 1;
    }
    return 0;
}

/* The demo's talker (the player it is talking to), from the demo task. */
Obj *sekisyo2DemoGetTalker(Obj *o) {
    return SEKI2DEMO(o)->talker;
}

/* "tobidemo2": the gate. Opened (flag 1), it plays its animation once. */
s32 sekisyo2TobiraProc(Obj *o, s16 msg, void *arg) {
    s32 pad;
    Seki2GateWork *w = SEKI2GATE(o);

    switch (msg) {
        case 2:
            o->name = "tobidemo2";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) sekisyo2TobiraAnim);
            frmMove(&w->model->f, gFrmLayer3);
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 1.0f;
            PRINTF("sekisyo2Tobira セット完了\n");
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
void sekisyo2TobiraOpen(Obj *o) {
    SEKI2GATE(o)->open |= 1;
}

/* Whether the gate is opening. */
s32 sekisyo2TobiraIsOpening(Obj *o) {
    if (SEKI2GATE(o)->open != 0) {
        return 1;
    }
    return 0;
}

/* "camera2": the demo's camera. State 0 keeps the view inside the gate's area; 1 moves it to
 * the gatekeepers (then 2, holding); 3 moves it back (then 4, waiting for it to stop, then the
 * camera AI's own again). */
s32 sekisyo2CameraProc(Obj *o, s16 msg, void *arg) {
    Seki2CamWork *w = SEKI2CAM(o);
    CamView *v;

    switch (msg) {
        case 2:
            o->name = "camera2";
            eneSetup(o, arg);
            PRINTF("sekisyo2Camera セット完了\n");
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
                    v->eye[1] = -38.0f;
                    v->eye[2] = -39.0f;
                    v->at[1] = -73.0f;
                    v->at[2] = 117.0f;
                    v->zNear = 20.0f;
                    v->zFar = 800.0f;
                    v->fovy = 0.6981317401f;
                    break;
                case 1:
                    camSetSpeed(w->cam, 40 / gRetracesPerFrame);
                    camMoveEye(w->cam, 22.0f, -61.0f, 80.0f);
                    camMoveAt(w->cam, -122.0f, -34.0f, 144.0f);
                    w->state = 2;
                    break;
                case 2:
                    break;
                case 3:
                    camSetSpeed(w->cam, 40 / gRetracesPerFrame);
                    camMoveEye(w->cam, 39.0f, -38.0f, -39.0f);
                    camMoveAt(w->cam, -44.0f, -73.0f, 117.0f);
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

/* An empty function nothing calls (its "jr ra; nop" at 0x08001040). */
void sekisyo2Dummy(void) {
}

/* Holds the camera AI and moves the demo camera to the gatekeepers (its state 1). */
void sekisyo2CameraGoIn(Obj *o) {
    camAiSetHold(1);
    SEKI2CAM(o)->state = 1;
}

/* Moves the demo camera back (its state 3). */
void sekisyo2CameraGoBack(Obj *o) {
    SEKI2CAM(o)->state = 3;
}

/* Whether the demo camera is moving. */
s32 sekisyo2CameraIsMoving(Obj *o) {
    return camGetMoving(SEKI2CAM(o)->cam);
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
Gfx *sekisyo2UmiDl(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 128, 128, 128, 128);
    gDPLoadScrollBlock64x32(gfx++, sekisyo2UmiTex, 0, G_TX_RENDERTILE, scroll, scroll);
    gDPLoadScrollBlock64x32(gfx++, sekisyo2UmiTex, 0x100, 1, scroll + 10, -scroll);
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* "umi": the sea, a "pol" model whose texture scrolls each draw (message 3) and which swells
 * with a cosine over 120 steps. */
s32 sekisyo2UmiProc(Obj *o, s16 msg, void *arg) {
    Seki2UmiWork *w = SEKI2UMI(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "umi";
            eneSetup(o, arg);
            w->sea = eneNewPol(o, sekisyo2UmiSeaDl);
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
            gGfxDlHead = sekisyo2UmiDl(gGfxDlHead, w->scroll);
            w->scroll--;
            eneSetModelOnCourse(o, &w->sea->f);
            break;
        case 0:
            if (w->count == 120) {
                w->count = 0;
            }
            w->vel[0] = mathCos(w->count * 3 * PI / 180.0f) * 0.8;
            w->vel[1] = mathCos(w->count * 3 * PI / 180.0f) * 0.05;
            w->vel[2] = mathCos(w->count * 3 * PI / 180.0f);
            w->count++;
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, w->model);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
