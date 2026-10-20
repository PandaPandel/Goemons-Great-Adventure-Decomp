#include "tlb.h"

/* The first checkpoint's gate demo (demoSekisyo1): the gatekeepers check the player's passes
 * (tegata), open the gate and the players walk through; tlb_0122 is the second checkpoint's. */

extern Frame *gFrmLayer3;
extern Gfx etskSekisyoDl[];

extern u8 sekisyo1YakuninAnim[];
extern u8 sekisyo1YakuninWalkToAnim[];
extern u8 sekisyo1TobiraAnim[];

/* The gatekeepers' person data (talk 0x16, a path at sekisyo1YakuninPath, their standing and walking
 * animations sekisyo1YakuninAnim and sekisyo1YakuninWalkToAnim): passed with mode 2, so eneSetup returns it. */
extern u32 sekisyo1YakuninPath[];
u32 sekisyo1YakuninPath[] = { 0x0002005A, 0x00010000 };
u32 sekisyo1YakuninData[] = { 0x00000016, (u32) &sekisyo1YakuninPath, 0x00000000, 0x00000000, 0x00000000, (u32) sekisyo1YakuninAnim, (u32) sekisyo1YakuninWalkToAnim, 0x00000000, 0x00000000, 0x00000000 };

/* The demo's work: a demo talker's (PersonObj, demo.c) to 0x40, then its own. */
typedef struct Seki1DemoWork {
    /* 0x00 */ u8 head[0x40];
    /* 0x40 */ Obj *talker; /* the player talking to it (demoTalk) */
    /* 0x44 */ Obj *camera;
    /* 0x48 */ Obj *yaku1; /* the gatekeepers */
    /* 0x4C */ Obj *yaku2;
    /* 0x50 */ Obj *gate;
    /* 0x54 */ u32 data; /* eneSetup's: its talk data */
    /* 0x58 */ u8 state; /* 0 waiting for the talk to end, 1 running */
    /* 0x59 */ u8 step; /* the demo's steps while running */
    /* 0x5C */ Obj *block; /* kind 0x19A, gone when the gate opens */
    /* 0x60 */ s32 timer;
} Seki1DemoWork;

/* A gatekeeper's work: an enemy's (EneObj) to 0x34, then its own. */
typedef struct Seki1YakuWork {
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
} Seki1YakuWork;

/* The gate's work. */
typedef struct Seki1GateWork {
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
} Seki1GateWork;

/* The demo camera's work. */
typedef struct Seki1CamWork {
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
} Seki1CamWork;

#define SEKI1DEMO(o) ((Seki1DemoWork *) ((u32) (o) + sizeof(Obj)))
#define SEKI1YAKU(o) ((Seki1YakuWork *) ((u32) (o) + sizeof(Obj)))
#define SEKI1GATE(o) ((Seki1GateWork *) ((u32) (o) + sizeof(Obj)))
#define SEKI1CAM(o) ((Seki1CamWork *) ((u32) (o) + sizeof(Obj)))

void sekisyo1YakuninSetFace(Obj *o, s32 on);
void sekisyo1YakuninWalkTo(Obj *o, f32 x, f32 z, f32 speed);
s32 sekisyo1YakuninIsBusy(Obj *o);
Obj *sekisyo1DemoGetTalker(Obj *o);
void sekisyo1TobiraOpen(Obj *o);
s32 sekisyo1TobiraIsOpening(Obj *o);
void sekisyo1CameraGoIn(Obj *o);
void sekisyo1CameraGoBack(Obj *o);
s32 sekisyo1CameraIsMoving(Obj *o);

s32 Etsk_sekisyo_main(Obj *o, s16 msg, void *arg) {
    EneObj *w = ENE(o);

    switch (msg) {
        case 2:
            o->name = "Etsk_sekisyo_main";
            eneSetup(o, arg);
            w->model = (Frame *) eneNewPol(o, etskSekisyoDl);
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

/* "demoSekisyo1_proc": the demo. Talked to, it stops the sun's clock and starts its steps: the
 * camera goes to the gatekeepers, they face the player and scenario 0x16 runs; with the passes
 * held, they step aside, the gate opens, scenario 0x17 runs, the players walk through and the
 * stage goes to area 0 stage 5 on the atlas (or the demo menu's second demo); without them, the
 * camera goes back and the demo ends. */
s32 demoSekisyo1_proc(Obj *o, s16 msg, void *arg) {
    Seki1DemoWork *w = SEKI1DEMO(o);
    s32 pad[4];

    switch (msg) {
        case 2:
            o->name = "demoSekisyo1_proc";
            w->data = eneSetup(o, arg);
            if (demoTalkerMake(o, 0xE8) == -1) {
                return -1;
            }
            demoTalkInit(o, w->data);
            w->yaku1 = eneMakeEx(o, ENE_SEKISYO1_YAKUNIN, 1, 412.0f, -63.0f, -14.0f, 2,
                                 (u32 (*)(void *, void *, void *)) sekisyo1YakuninData);
            w->yaku2 = eneMakeEx(o, ENE_SEKISYO1_YAKUNIN, 1, 412.0f, -63.0f, 14.0f, 2,
                                 (u32 (*)(void *, void *, void *)) sekisyo1YakuninData);
            w->gate = eneMakeEx(o, ENE_SEKISYO1_TOBIRA, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
            w->camera = eneMakeEx(o, ENE_SEKISYO1_CAMERA, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
            fadeCreate(gEffectTask);
            w->block = eneMakeEx(o, ENE_DEMO_STOPPER, 0, -25.0f, 0.0f, 0.0f, 0, NULL);
            PRINTF("demoSekisyo1 セット完了\n");
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
                        w->step = 0;
                    }
                    break;
                case 1:
                    switch (w->step) {
                        case 0:
                            sekisyo1CameraGoIn(w->camera);
                            demoPlStandAll();
                            PRINTF("MONBAN_TALK\n");
                            w->step = 1;
                            break;
                        case 1:
                            if (sekisyo1CameraIsMoving(w->camera) == 0) {
                                sekisyo1YakuninSetFace(w->yaku1, 1);
                                sekisyo1YakuninSetFace(w->yaku2, 1);
                                scRunStart(0x16, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                                PRINTF("GO YAKU_WALK\n");
                                w->step = 2;
                            }
                            break;
                        case 2:
                            if (scIsRunning() == 0) {
                                if (scTegataLeftMin(0) <= 0) {
                                    PRINTF("手形が揃った\n");
                                    w->step = 3;
                                } else {
                                    PRINTF("手形が揃ってない\n");
                                    w->step = 9;
                                }
                            }
                            break;
                        case 3:
                            if (scIsRunning() == 0) {
                                sekisyo1YakuninSetFace(w->yaku1, 0);
                                sekisyo1YakuninSetFace(w->yaku2, 0);
                                sekisyo1YakuninWalkTo(w->yaku1, 452.0f, -7.0f, 0.8f);
                                sekisyo1YakuninWalkTo(w->yaku2, 452.0f, 7.0f, 0.8f);
                                PRINTF("GO MON_OPEN\n");
                                w->step = 4;
                            }
                            break;
                        case 4:
                            if (sekisyo1YakuninIsBusy(w->yaku1) == 0 && sekisyo1YakuninIsBusy(w->yaku2) == 0) {
                                sekisyo1TobiraOpen(w->gate);
                                sndPlay(0x11A);
                                sekisyo1YakuninWalkTo(w->yaku1, 421.0f, -19.0f, 0.4f);
                                sekisyo1YakuninWalkTo(w->yaku2, 421.0f, 19.0f, 0.4f);
                                w->step = 5;
                            }
                            break;
                        case 5:
                            if (sekisyo1TobiraIsOpening(w->gate) == 0 && sekisyo1YakuninIsBusy(w->yaku1) == 0 &&
                                sekisyo1YakuninIsBusy(w->yaku2) == 0) {
                                scRunStart(0x17, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                                sekisyo1YakuninSetFace(w->yaku1, 1);
                                sekisyo1YakuninSetFace(w->yaku2, 1);
                                tskDestroy(w->block);
                                PRINTF("GO PL_WALK_AWAY\n");
                                w->step = 6;
                            }
                            break;
                        case 6:
                            if (scIsRunning() == 0) {
                                demoSetSpeedPlRun4P(735.0f, -65.0f, 0.0f, 3.0f, 735.0f, -65.0f, 0.0f, 3.0f, 735.0f,
                                                    -65.0f, 0.0f, 3.0f, 735.0f, -65.0f, 0.0f, 3.0f);
                                PRINTF("PL_WALK_AWAY\n");
                                w->timer = 0;
                                w->step = 7;
                            }
                            break;
                        case 7:
                            if (fadeCloseLetterbox(120) == 1) {
                                demoPlStandAll();
                                PRINTF("FADE_OUT\n");
                                w->step = 8;
                            }
                            if (w->timer == 156 / gRetracesPerFrame) {
                                sndPlay(10);
                            }
                            break;
                        case 8:
                            camSunSetClockStop(0);
                            camAiSetHold(0);
                            gSaveData.stageFlags[0 * 16 + 5] |= 1;
                            gSaveData.areaFlags[6] |= 1;
                            gaugeSetOn(0);
                            PRINTF("GO DEMO_END\n");
                            if (gSaveData.stageFlags[6 * 16 + 4] & 0x80) {
                                stageGotoAtlas(0, 5);
                            } else {
                                dmenuQueueDemo2();
                            }
                            break;
                        case 9:
                            sekisyo1CameraGoBack(w->camera);
                            w->step = 10;
                            break;
                        case 10:
                            if (sekisyo1CameraIsMoving(w->camera) == 0) {
                                demoPlReleaseAll();
                                sekisyo1YakuninSetFace(w->yaku1, 0);
                                sekisyo1YakuninSetFace(w->yaku2, 0);
                                demoFinish(o);
                                w->state = 0;
                            }
                            break;
                    }
                    break;
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "yakudemo": a gatekeeper. Faces the player while flag 1 is set (its position asked of the
 * demo's talker with message 4), walks to (goalX, goalZ) while flag 2 is, else turns to 90
 * degrees. */
s32 sekisyo1YakuninProc(Obj *o, s16 msg, void *arg) {
    Seki1YakuWork *w = SEKI1YAKU(o);
    Vec3f pos;

    switch (msg) {
        case 2:
            o->name = "yakudemo";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) sekisyo1YakuninAnim);
            if (eneSetOnEarth(o) == -1) {
                PRINTF("personInit: Error: 地面にセット失敗\n");
                return -1;
            }
            eneSetAngle(o, 90.0f);
            w->shadow = shadowCreate();
            PRINTF("sekisyo1Yakunin セット完了\n");
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
                tskSendMsg(sekisyo1DemoGetTalker(o->parent), 4, &pos);
                eneTurnToPoint(o, pos.x, pos.z, gRetracesPerFrame * 2);
            } else if (w->flags & 2) {
                if (eneMoveToXZ(o, w->goalX, w->goalZ, w->speed / gRetracesPerFrame, w->speed / gRetracesPerFrame) == 1) {
                    w->flags &= ~2;
                    personAnimeSet(o, (u32) sekisyo1YakuninAnim);
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
void sekisyo1YakuninSetFace(Obj *o, s32 on) {
    Seki1YakuWork *w = SEKI1YAKU(o);

    if (on == 1) {
        w->flags |= 1;
    } else {
        w->flags &= ~1;
    }
}

/* A gatekeeper's walk to (x, z) at speed, with its walking animation. */
void sekisyo1YakuninWalkTo(Obj *o, f32 x, f32 z, f32 speed) {
    Seki1YakuWork *w = SEKI1YAKU(o);

    w->flags |= 2;
    w->goalX = x;
    w->goalZ = z;
    w->speed = speed;
    personAnimeSet(o, (u32) sekisyo1YakuninWalkToAnim);
}

/* Whether a gatekeeper is busy (facing the player or walking). */
s32 sekisyo1YakuninIsBusy(Obj *o) {
    if (SEKI1YAKU(o)->flags != 0) {
        return 1;
    }
    return 0;
}

/* The demo's talker (the player it is talking to), from the demo task. */
Obj *sekisyo1DemoGetTalker(Obj *o) {
    return SEKI1DEMO(o)->talker;
}

/* "tobidemo": the gate. Opened (flag 1), it plays its animation once. */
s32 sekisyo1TobiraProc(Obj *o, s16 msg, void *arg) {
    s32 pad;
    Seki1GateWork *w = SEKI1GATE(o);

    switch (msg) {
        case 2:
            o->name = "tobidemo";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) sekisyo1TobiraAnim);
            frmMove(&w->model->f, gFrmLayer3);
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 1.0f;
            PRINTF("sekisyo1Tobira セット完了\n");
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
void sekisyo1TobiraOpen(Obj *o) {
    SEKI1GATE(o)->open |= 1;
}

/* Whether the gate is opening. */
s32 sekisyo1TobiraIsOpening(Obj *o) {
    if (SEKI1GATE(o)->open != 0) {
        return 1;
    }
    return 0;
}

/* "camera": the demo's camera. State 0 keeps the view inside the gate's area; 1 moves it to
 * the gatekeepers (then 2, holding); 3 moves it back (then 4, waiting for it to stop, then the
 * camera AI's own again). */
s32 sekisyo1CameraProc(Obj *o, s16 msg, void *arg) {
    Seki1CamWork *w = SEKI1CAM(o);
    CamView *v;

    switch (msg) {
        case 2:
            o->name = "camera";
            eneSetup(o, arg);
            PRINTF("sekisyo1Camera セット完了\n");
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
                    camMoveEye(w->cam, 22.0f, -61.0f, 80.0f);
                    camMoveAt(w->cam, -122.0f, -34.0f, 144.0f);
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

/* An empty function nothing calls (its "jr ra; nop" at 0x0800109C). */
void sekisyo1Dummy(void) {
}

/* Holds the camera AI and moves the demo camera to the gatekeepers (its state 1). */
void sekisyo1CameraGoIn(Obj *o) {
    camAiSetHold(1);
    SEKI1CAM(o)->state = 1;
}

/* Moves the demo camera back (its state 3). */
void sekisyo1CameraGoBack(Obj *o) {
    SEKI1CAM(o)->state = 3;
}

/* Whether the demo camera is moving. */
s32 sekisyo1CameraIsMoving(Obj *o) {
    return camGetMoving(SEKI1CAM(o)->cam);
}
