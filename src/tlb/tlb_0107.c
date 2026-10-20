#include "tlb.h"

/* The fortune-teller's shop (uranai, archive file 107): "UranaiTable", the table that runs the
 * visit, "decoURA", its two animated decorations, and "U_cam", the room's camera. The table makes
 * the decoration (kind 0x129), the fortune-teller ("pura", kind 0x1A5) and the camera (kind 0x12A);
 * when a player comes near it turns the camera to the table and starts its scenario. */

/* An enemy's work (EneObj) to 0x34: the head every kind in this file has. */
#define ENE_HEAD                                                                                   \
    /* 0x00 */ u8 state;                                                                           \
    /* 0x01 */ u8 zone;                                                                            \
    /* 0x02 */ s16 spawn;                                                                          \
    /* 0x04 */ EneKind *kind;                                                                      \
    /* 0x08 */ u8 flags;                                                                           \
    /* 0x09 */ u8 hitKind;                                                                         \
    /* 0x0A */ u8 hit;                                                                             \
    /* 0x0B */ s8 hp;                                                                              \
    /* 0x0C */ PolFrame *pol;                                                                      \
    /* 0x10 */ f32 x;                                                                              \
    /* 0x14 */ f32 y;                                                                              \
    /* 0x18 */ f32 z;                                                                              \
    /* 0x1C */ f32 rot[3];                                                                         \
    /* 0x28 */ f32 vel[3];

/* "UranaiTable": the table, at its task + 0x20. */
typedef struct UranaiWork {
    ENE_HEAD
    /* 0x34 */ s32 *scenario; /* its placement's scenario number (stageAddrToRam) */
    /* 0x38 */ Obj *cam; /* "U_cam" (kind 0x12A) */
    /* 0x3C */ Obj *pura; /* the fortune-teller (kind 0x1A5) */
    /* 0x40 */ Obj *deco; /* "decoURA" (kind 0x129) */
    /* 0x44 */ Obj *seat1; /* kind 0x1A6, made at the first fortune */
    /* 0x48 */ Obj *seat2;
} UranaiWork;

/* "decoURA": two animated models on the table. */
typedef struct DecoWork {
    ENE_HEAD
    /* 0x34 */ PolFrame *a;
    /* 0x38 */ PolFrame *b;
    /* 0x3C */ Anim animA;
    /* 0x4C */ Anim animB;
} DecoWork;

/* "U_cam": the room's camera; the table sets `mode` and it moves the camera once. */
typedef struct UCamWork {
    ENE_HEAD
    /* 0x34 */ Camera *cam;
    /* 0x38 */ u8 mode; /* 0 the room, 1 the table, 2 close in, 3 and 4 back; 5 done */
    /* 0x3A */ u16 time;
    /* 0x3C */ s32 ready; /* 1 until the close-in starts */
} UCamWork;

/* The fortune-teller's state byte, at its task + 0x20. */
typedef struct PuraWork {
    /* 0x00 */ u8 state;
} PuraWork;

#define URANAI(obj) ((UranaiWork *) FX(obj))
#define DECO(obj) ((DecoWork *) FX(obj))
#define UCAM(obj) ((UCamWork *) FX(obj))
#define PURA(obj) ((PuraWork *) FX(obj))

extern u8 uranaiDecoAnimATex4[];
extern u8 uranaiDecoAnimATex0[];
extern u8 uranaiDecoAnimATex1[];
extern u8 uranaiDecoAnimBTex3[];
extern u8 uranaiDecoAnimBTex0[];
extern u8 uranaiDecoAnimBTex1[];
AnimFrame uranaiDecoAnimA[] = {
    { (u32) uranaiDecoAnimATex0, 8 },
    { (u32) uranaiDecoAnimATex1, 8 },
    { 0x03001070, 8 },
    { 0x00000000, 0 },
    { (u32) uranaiDecoAnimATex4, 8 },
    { 0x00000000, 0 },
};
AnimFrame uranaiDecoAnimB[] = {
    { (u32) uranaiDecoAnimBTex0, 10 },
    { (u32) uranaiDecoAnimBTex1, 10 },
    { 0x00000000, 0 },
    { (u32) uranaiDecoAnimBTex3, 10 },
    { 0x00000000, 0 },
};
f32 uranaiCamEye[3][3] = { -153.0f, 43.0f, -107.0f, -210.0f, 23.0f, 5.0f, -146.0f, 13.0f, 0.1f };
f32 uranaiCamAt[3][3] = { -42.0f, 0.1f, 0.1f, -11.0f, 48.0f, 0.1f, 6.0f, 25.0f, 0.1f };
extern Gfx uranaiDecoADl[];
extern Gfx uranaiDecoBDl[];

void uranaiDecoStop(Obj *o);
void uranaiDecoPlay(Obj *o);
s32 uranaiCameraMoving(Obj *o);
void uranaiCameraRoom(Obj *o);
void uranaiCameraTable(Obj *o);
void uranaiCameraCloseIn(Obj *o);
void uranaiCameraFovReset(Obj *o);
void uranaiCameraRelease(Obj *o);
s32 uranaiMakePair(Obj *o);
s32 uranaiPuraSetState(Obj *o, s32 state);
s32 uranaiCameraReady(Obj *o);

/* "UranaiTable" */
/* The dead "uranaiInit" print sits before eneSetup: after it, IDO reloads eneSetup's result early
 * into a2. */
s32 uranaiTableProc(Obj *o, s16 msg, void *arg) {
    s32 pad;
    UranaiWork *w = URANAI(o);
    u32 type;

    switch (msg) {
        case 2:
            o->name = "UranaiTable";
            PRINTF("struct URANAI_W seze [%d]\n", sizeof(UranaiWork));
            PRINTF("uranaiInit: talkNo[%d] r[%d] h[%d] ry[%f]\n", 0, 0, 0, 0.0);
            type = eneSetup(o, arg);
            if ((w->pol = eneNewPol(o, NULL)) == NULL) {
                return -1;
            }
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2] = 0.4f;
            w->scenario = (s32 *) stageAddrToRam(type);
            w->deco = eneMakeEx(o, ENE_URANAI_DECO, 0, 0.0f, 1.0f, 0.0f, 0, NULL);
            if (w->deco == NULL) {
                PRINTF("tableUranai_proc: deco NULL!!\n");
                return -1;
            }
            w->pura = eneMakeEx(o, ENE_SHOP_PURA, 0, 95.0f, 5.0f, 0.0f, 0, NULL);
            if (w->pura == NULL) {
                PRINTF("tableUranai_proc: pura NULL!!\n");
                return -1;
            }
            uranaiDecoStop(w->deco);
            w->cam = eneMakeEx(o, ENE_URANAI_CAMERA, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
            if (w->cam == NULL) {
                PRINTF("dcam is NULL!\n");
                return -1;
            }
            uranaiCameraRoom(w->cam);
            camAiSetHold(1);
            w->seat1 = NULL;
            w->seat2 = NULL;
            camSunClearTunes();
            gGameWork.scenarioRunning = 2;
            PRINTF("tableUranai_proc: 占い屋テーブルセットＯＫ\n");
            break;
        case 1:
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            gGameWork.scenarioRunning = 0;
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (shimPlNearCheckWorld(o, 80.0f)) {
                        demoPlStandAll();
                        uranaiCameraTable(w->cam);
                        w->state = 2;
                    }
                    break;
                case 1:
                    if (shimPlNearCheckWorld(o, 20.0f)) {
                        camAiSetHold(1);
                        demoPlStandAll();
                        uranaiCameraCloseIn(w->cam);
                        uranaiPuraSetState(w->pura, 0);
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (uranaiCameraMoving(w->cam) == 0) {
                        if (uranaiCameraReady(w->cam) == 1) {
                            uranaiPuraSetState(w->pura, 0);
                            uranaiCameraCloseIn(w->cam);
                        } else {
                            w->state = 3;
                        }
                    }
                    break;
                case 3:
                    scRunStart(*w->scenario, 0, 0);
                    w->state = 4;
                    uranaiCameraFovReset(w->cam);
                    uranaiDecoPlay(w->deco);
                    if (w->seat1 == NULL) {
                        uranaiMakePair(o);
                    }
                    demoSetPlPos(0, 0.0f, 0.0f, -10.0f, 1.570796371f, 0, 0.0f, 0.0f, 10.0f, 1.570796371f, 0,
                                 0.0f, 0.0f, -20.0f, 1.570796371f, 0, 0.0f, 0.0f, 20.0f, 1.570796371f);
                    if (sndGetMusic() != 0xC0) {
                        sndPlay(1);
                        sndPlay(0xC0);
                    }
                    break;
                case 4:
                    if (swWorkTest(0x12F)) {
                        swWorkReset(0x12F);
                        uranaiCameraCloseIn(w->cam);
                        uranaiPuraSetState(w->pura, 3);
                    }
                    if (!scIsRunning()) {
                        demoFinish(o);
                        demoPlReleaseAll();
                        uranaiCameraRelease(w->cam);
                        w->state = 1;
                    }
                    break;
                case 5:
                    break;
            }
            eneMove((EneObj *) w);
            if (w->pol != NULL) {
                eneSetModelOnCourse(o, &w->pol->f);
            }
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "decoURA": two models on the table, each with its texture animation (0: start them, 1: play). */
s32 uranaiDecoProc(Obj *o, s16 msg, void *arg) {
    s32 pad[2];
    DecoWork *w = DECO(o);

    switch (msg) {
        case 2:
            o->name = "decoURA";
            eneSetup(o, arg);
            if ((w->pol = eneNewPol(o, NULL)) == NULL) {
                return -1;
            }
            if ((w->a = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol)) == NULL) {
                return -1;
            }
            w->a->dl2 = uranaiDecoADl;
            w->a->scale[0] = w->a->scale[1] = w->a->scale[2] = 1.0f;
            floadGetSetRam((void **) w->a->segs, 0x48);
            if ((w->b = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol)) == NULL) {
                return -1;
            }
            w->b->dl2 = uranaiDecoBDl;
            w->b->scale[0] = w->b->scale[1] = w->b->scale[2] = 1.0f;
            floadGetSetRam((void **) w->b->segs, 0x48);
            fadeAnimStart(&w->animA, uranaiDecoAnimA, 4, &w->a->segs[3], floadGetSetFile(0x48, 0));
            fadeAnimStart(&w->animB, uranaiDecoAnimB, 3, &w->b->segs[3], floadGetSetFile(0x48, 0));
            w->a->pos[0] = 80.0f;
            w->a->pos[1] = 30.0f;
            w->a->pos[2] = 0.0f;
            w->b->pos[0] = 0.0f;
            w->b->pos[1] = 5.0f;
            w->b->pos[2] = 0.0f;
            break;
        case 1:
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            if (w->a != NULL) {
                frmDestroy(&w->a->f);
            }
            if (w->b != NULL) {
                frmDestroy(&w->b->f);
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    fadeAnimStart(&w->animA, uranaiDecoAnimA, 4, &w->a->segs[3], floadGetSetFile(0x48, 0));
                    fadeAnimStart(&w->animB, uranaiDecoAnimB, 3, &w->b->segs[3], floadGetSetFile(0x48, 0));
                    break;
                case 1:
                    fadeAnimStep(&w->animA);
                    fadeAnimStep(&w->animB);
                    break;
            }
            eneMove((EneObj *) w);
            if (w->pol != NULL) {
                eneSetModelOnCourse(o, &w->pol->f);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The decoration: stop its animations (the table sets this before it starts). */
void uranaiDecoStop(Obj *o) {
    DECO(o)->state = 0;
}

/* The decoration: play its animations. */
void uranaiDecoPlay(Obj *o) {
    DECO(o)->state = 1;
}

/* "U_cam": the room's camera. Each mode the table sets moves the camera once, then rests (5). */
s32 uranaiCameraProc(Obj *o, s16 msg, void *arg) {
    UCamWork *w = UCAM(o);
    s32 pad[7];
    CamView *view;
    s32 pad2;

    switch (msg) {
        case 2:
            o->name = "U_cam";
            eneSetup(o, arg);
            if ((w->pol = eneNewPol(o, NULL)) == NULL) {
                return -1;
            }
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2] = 1.0f;
            w->cam = camAiGetCam();
            w->time = 0;
            w->ready = 1;
            PRINTF("roomUranai_camera セット完了\n");
            break;
        case 0:
            eneStepHookPre(o);
            view = camGetView(w->cam);
            view->zNear = 20.0f;
            view->zFar = 10000.0f;
            switch (w->mode) {
                case 0:
                    camStop(w->cam);
                    view->eye[0] = uranaiCamEye[0][0];
                    view->eye[1] = uranaiCamEye[0][1];
                    view->eye[2] = uranaiCamEye[0][2];
                    view->at[0] = uranaiCamAt[0][0];
                    view->at[1] = uranaiCamAt[0][1];
                    view->at[2] = uranaiCamAt[0][2];
                    w->mode = 5;
                    break;
                case 1:
                    camStop(w->cam);
                    camSetSpeed(w->cam, 30.0f);
                    camMoveEye(w->cam, uranaiCamEye[1][0], uranaiCamEye[1][1],
                               uranaiCamEye[1][2]);
                    camMoveAt(w->cam, uranaiCamAt[1][0], uranaiCamAt[1][1],
                              uranaiCamAt[1][2]);
                    w->mode = 5;
                    break;
                case 2:
                    camStop(w->cam);
                    camSetSpeed(w->cam, 60.0f);
                    camMoveEye(w->cam, uranaiCamEye[2][0], uranaiCamEye[2][1],
                               uranaiCamEye[2][2]);
                    camMoveAt(w->cam, uranaiCamAt[2][0], uranaiCamAt[2][1],
                              uranaiCamAt[2][2]);
                    camMoveFov(w->cam, 0.23f);
                    w->mode = 5;
                    w->ready = 0;
                    break;
                case 3:
                    camStop(w->cam);
                    camMoveFov(w->cam, 0.698132f);
                    w->mode = 5;
                    break;
                case 4:
                    camAiSetHold(0);
                    camSetSpeed(w->cam, 30.0f);
                    camMoveFov(w->cam, 0.698132f);
                    w->mode = 5;
                    break;
                case 5:
                    break;
                case 6:
                    break;
                default:
                    PRINTF("roomUranai_camera CRAZY\n");
                    break;
            }
            w->time++;
            eneStepHookPost(o);
            break;
        case 1:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The camera is still moving. */
s32 uranaiCameraMoving(Obj *o) {
    return camGetMoving(UCAM(o)->cam);
}

/* The camera's modes, set by the table. */
void uranaiCameraRoom(Obj *o) {
    UCAM(o)->mode = 0;
}

void uranaiCameraTable(Obj *o) {
    UCAM(o)->mode = 1;
}

void uranaiCameraCloseIn(Obj *o) {
    UCAM(o)->mode = 2;
}

void uranaiCameraFovReset(Obj *o) {
    UCAM(o)->mode = 3;
}

void uranaiCameraRelease(Obj *o) {
    UCAM(o)->mode = 4;
}

void uranaiDummy(void) {
}

/* The two kind 0x1A6 objects either side of the table, made at the first fortune. */
s32 uranaiMakePair(Obj *o) {
    UranaiWork *w = URANAI(o);
    s32 pad;

    w->seat1 = eneMakeEx(NULL, ENE_SHOP_DANCER, 0, 65.0f, 0.0f, -50.0f, 0, NULL);
    w->seat2 = eneMakeEx(NULL, ENE_SHOP_DANCER, 0, 65.0f, 0.0f, 50.0f, 0, NULL);
    if (w->seat1 == NULL || w->seat2 == NULL) {
        return -1;
    }
    return 0;
}

/* The fortune-teller's state. */
s32 uranaiPuraSetState(Obj *o, s32 state) {
    PURA(o)->state = state;
    return 0;
}

/* The camera is ready (1 until its close-in starts). */
s32 uranaiCameraReady(Obj *o) {
    return UCAM(o)->ready;
}
