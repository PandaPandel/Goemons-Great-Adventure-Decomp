#include "tlb.h"

/* File 128: the mushroom hunt (kinoko-tori). "kinomania", the mushroom lover who sends the players
 * to pick the mushrooms and gives a pass (tegata) for them; "kinoDemo", the hunt's manager;
 * "kinoko", the mushrooms (a bit each in gGameWork.kinokoTaken); their display and timer; and
 * "kinoBig", the old woman made big. */

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* The mushroom lover's work, at its task + 0x20: a demo talker's (PersonObj) up to 0x44. */
typedef struct KinoManiaW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 pad09;
    /* 0x0A */ u8 hit; /* 1 on the ground */
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PersonData *data;
    /* 0x38 */ ACollFrame *box; /* its talk box */
    /* 0x3C */ u8 flags3C;
    /* 0x3D */ u8 count;
    /* 0x3E */ u8 pad3E[2];
    /* 0x40 */ Obj *talker; /* the player talking to it */
    /* 0x44 */ MPolFrame *shadow;
    /* 0x48 */ Camera *cam;
    /* 0x4C */ Obj *pass; /* the pass (kind 0x1C3), made between it and the player */
    /* 0x50 */ Anim anim; /* its face (segment 3 of its model) */
    /* 0x60 */ u8 flags; /* 1: the sky's tunes change 300 ticks after the pass */
    /* 0x64 */ s32 timer; /* ticks in the state */
    /* 0x68 */ u8 tuneTimer;
} KinoManiaW;

#define KINOMANIA(obj) ((KinoManiaW *) FX(obj))

/* The pass's work (kind 0x1C3, another file's), as far as this file sets it. */
typedef struct PassW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *pol;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *glow;
    /* 0x38 */ u8 rgb[3];
    /* 0x3B */ u8 pad3B;
    /* 0x3C */ s16 alpha;
    /* 0x3E */ u8 pad3E[2];
    /* 0x40 */ s32 timer;
    /* 0x44 */ Obj *player; /* the one it goes to */
} PassW;

/* The hunt manager's work ("kinoDemo"), at its task + 0x20. */
typedef struct KinoDemoW {
    /* 0x00 */ Obj *disp; /* the mushroom display (kind 0x1C2) */
    /* 0x04 */ Obj *timerObj; /* the hunt's timer (kind 0x16D) */
    /* 0x08 */ Camera *cam;
    /* 0x0C */ u8 state;
    /* 0x10 */ s32 timer; /* ticks in the state */
    /* 0x14 */ u8 pad14[4];
    /* 0x18 */ s8 partake; /* gGameWork.partake while the instructions show */
} KinoDemoW;

#define KINODEMO(obj) ((KinoDemoW *) FX(obj))

/* A mushroom's work ("kinoko"), at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct KinokoW {
    /* 0x00 */ u8 state; /* 0 standing, 1 bobbing, 2 picked */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ MPolFrame *shadow;
    /* 0x38 */ ACollFrame *box; /* its pick box (kind 0x66) */
    /* 0x3C */ u8 id; /* 1-30: its bit in gGameWork.kinokoTaken */
    /* 0x3D */ u8 type;
    /* 0x3E */ u8 wave; /* its bobbing's phase, 0-39 */
} KinokoW;

#define KINOKO(obj) ((KinokoW *) FX(obj))

/* The mushroom count's work ("k_disp"), at its task + 0x20. */
typedef struct KinoDispW {
    /* 0x00 */ CamView *cam; /* its own ortho camera */
    /* 0x04 */ PolFrame *icon; /* a mushroom, turning */
    /* 0x08 */ PolFrame *digits[2]; /* tens, ones */
    /* 0x10 */ PolFrame *cross;
    /* 0x14 */ u8 shown[2]; /* the digits shown (21: none yet) */
    /* 0x16 */ u8 state; /* 0 making the frames, 1 counting */
} KinoDispW;

#define KINODISP(obj) ((KinoDispW *) FX(obj))

/* acollAtkAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy,
                                     s16 oz, s32 a);

void kinokoFadeOut(f32 speed);
void kinokoPassToPlayer(Obj *o, Obj *player);
u8 kinokoIsTaken(u8 id);
void kinokoSetTaken(u8 id);
void kinokoClearAll(void);
s32 kinokoCheck(void);

extern Unk88282 gSaveCopyPlayers[4]; /* the players' entries for a continue (stage.c) */
extern u8 kinokoManiaAnim[]; /* the mushroom lover's animation */
extern AnimFrame kinokoManiaFaceAnim[]; /* his face's texture animation */
extern Gfx kinokoMushroomDl[]; /* a mushroom */
extern Gfx kinokoDispCountCrossDl[]; /* the count's cross */
extern Gfx kinokoDispSetupDl[]; /* the count's render setup (a display list) */
extern Gfx *kinokoDispDigits[]; /* the digits 0-9 */

/* The file's .data, in address order (its externs above stay as declarations). */
extern u8 kinokoDispDigitsDl0[];
extern u8 kinokoDispDigitsDl1[];
extern u8 kinokoDispDigitsDl2[];
extern u8 kinokoDispDigitsDl3[];
extern u8 kinokoDispDigitsDl4[];
extern u8 kinokoDispDigitsDl7[];
extern u8 kinokoDispDigitsDl8[];
extern u8 kinokoManiaFaceTex0[];
extern u8 kinokoManiaFaceTex1[];
Gfx kinokoDispSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(CVG_DST_CLAMP | ZMODE_OPA | CVG_X_ALPHA | FORCE_BL | G_RM_PASS, CVG_DST_CLAMP | ZMODE_OPA | CVG_X_ALPHA | FORCE_BL | GBL_c2(G_BL_CLR_IN, G_BL_0, G_BL_CLR_IN, G_BL_1)),
    gsDPSetCombineLERP(PRIMITIVE, 0, SHADE, 0, 0, 0, 0, PRIMITIVE, PRIMITIVE, 0, SHADE, 0, 0, 0, 0, PRIMITIVE),
    gsDPPipeSync(),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsSPEndDisplayList(),
}; /* the count's render setup (a display list) */
AnimFrame kinokoManiaFaceAnim[] = {
    { (u32) kinokoManiaFaceTex0, 5 },
    { (u32) kinokoManiaFaceTex1, 5 },
    { 0, 0 },
}; /* his face's texture animation */
Gfx *kinokoDispDigits[] = { kinokoDispDigitsDl0, kinokoDispDigitsDl1, kinokoDispDigitsDl2, kinokoDispDigitsDl3, kinokoDispDigitsDl4, (Gfx *) 0x030007C0, (Gfx *) 0x030008A0, kinokoDispDigitsDl7, kinokoDispDigitsDl8, (Gfx *) 0x03000B40 }; /* the digits 0-9 */

/* The screen fades to black at the given speed. The first call is on the brace's line: IDO
 * schedules by line, and only there does "sw ra" come after the call's address (as the game). */
void kinokoFadeOut(f32 speed) { fadeSetSpeed(speed);
    fadeSetColor(0, 0, 0);
    fadeSetTarget(255.0f);
}

/* The pass goes to the player (state 3). */
void kinokoPassToPlayer(Obj *o, Obj *player) {
    PassW *w = (PassW *) FX(o);

    w->state = 3;
    w->player = player;
    w->timer = 0;
}

/* The pass's sound ("eventTegataSdSet"). */
void kinokoEventTegataSdSet(void) {
    DPRINTF("&&&& eventTegataSdSet  pl D がないn");
}

void kinokoEventClearSd(void) {
    DPRINTF("&&&& SD_G_CLEAR1n");
    DPRINTF("&&&& SD_E_CLEAR1n");
    DPRINTF("&&&& SD_S_CLEAR1n");
    DPRINTF("&&&& SD_Y_CLEAR1n");
}

/* "kinomania": the mushroom lover. Talked to, he talks (scenario 0x224); if the player takes on
 * the hunt (0x94) the screen fades and the game goes to the atlas with the hunt's stages open;
 * back from it (0x92) he counts the mushrooms, and if the player hands them over (0x93) he gives
 * the pass between them (kind 0x1C3, sparks and kind 0x1C1), the pass goes to the player
 * (scenarios 0x20, 0x225), and the sky's tunes change. */
s32 kinokoMania_proc(Obj *o, s16 msg, void *arg) {
    KinoManiaW *w = KINOMANIA(o);
    u32 data;
    CamView *view;
    s32 i;
    Obj *player;
    s32 pad[3]; /* the game's frame: x at 0x5C, five words below view */
    f32 x;
    f32 y;
    f32 z;
    Vec3f spark;

    switch (msg) {
        case 2:
            o->name = "kinomania";
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) kinokoManiaAnim);
            if (w->model == NULL) {
                return -1;
            }
            if (swWorkTest(0xA4)) {
                w->model->scale[2] = 0.15f;
                w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 100, 0, 0);
            demoTalkInit(o, data);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("kinokoMania_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, kinokoManiaFaceAnim, 0, &w->model->segs[3], eneGetFile(o, 1));
            if (swWorkTest(0x92)) {
                swWorkReset(0x92);
                gGameWork.scenarioRunning = 2;
                w->state = 5;
            }
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            demoTalkerFree(o);
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (demoTalkEndCheck(o)) {
                        demoCamZoom_CL(w->talker);
                        w->state = 2;
                    }
                    break;
                case 2:
                    scRunStart(0x224, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    DPRINTF("talkNo[%d]\n");
                    w->state = 3;
                    break;
                case 3:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x94)) {
                            swWorkReset(0x94);
                            gGameWork.scenarioRunning = 2;
                            w->state = 4;
                            w->timer = 0;
                        } else {
                            personTalkCamOff();
                            demoFinish(o);
                            w->state = 0;
                        }
                    }
                    break;
                case 4:
                    if (w->timer == 1) {
                        sndPlay(10);
                        kinokoFadeOut(16 / gRetracesPerFrame);
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        view = camGetView(w->cam);
                        camSetSpeed(w->cam, 20 / gRetracesPerFrame);
                        camMoveAtY(w->cam, view->at[1] + 50.0f);
                        gGameWork.scenarioRunning = 2;
                        for (i = 0; i < 4; i++) {
                            gSaveCopyPlayers[i].chara = -1;
                        }
                    }
                    if (w->timer == 20 / gRetracesPerFrame) {
                        gGameWork.scenarioRunning = 0;
                        camAiSetHold(0);
                        demoAreaFlagSave();
                        demoAreaFlagSetBit7(2);
                        gSaveData.stageFlags[0x21] |= 1;
                        gSaveData.stageFlags[0x21] &= ~0x80;
                        gSaveData.stageFlags[0x24] |= 1;
                        gSaveData.stageFlags[0x24] &= ~0x80;
                        gSaveData.stageFlags[0x1A] |= 1;
                        gSaveData.stageFlags[0x1A] &= ~0x80;
                        gSaveData.stageFlags[0x1D] |= 1;
                        gSaveData.stageFlags[0x1D] &= ~0x80;
                        kinokoClearAll();
                        stageGotoAtlas(2, 3);
                    }
                    break;
                case 5:
                    if (w->timer == 1) {
                        demoPlStandAll();
                    }
                    if (w->timer == 8) {
                        if (kinokoCheck() < 30) {
                            camSunClearTunes();
                            camSunSetTunes4(0x84, 0x81, 0x82, 0x83);
                        }
                        personCamZoomUp(plCheck());
                        swWorkReset(0x92);
                        demoAreaFlagRestore();
                        scRunStart(0x224, 0, 0);
                        w->state = 6;
                    }
                    break;
                case 6:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x93)) {
                            w->state = 7;
                            swWorkReset(0x93);
                        } else {
                            w->state = 10;
                        }
                    }
                    break;
                case 7:
                    player = plCheck();
                    if (player == NULL) {
                        return -1;
                    }
                    plGetModelPos(player, &x, &y, &z);
                    w->pass = eneMakeEx(NULL, ENE_DEMO_TEGATA, 0, (w->model->pos[0] + x) / 2, w->model->pos[1],
                                        (w->model->pos[2] + z) / 2, 0, NULL);
                    w->timer = 0;
                    w->state = 8;
                    break;
                case 8:
                    if (w->timer == 200 / gRetracesPerFrame) {
                        demoGetEneModelPos(w->pass, &spark);
                        ptclSpawnSparksFall(spark.x, spark.y, spark.z);
                        eneMakeEx(NULL, ENE_DEMO_KIRAKIRA, 0, spark.x, spark.y, spark.z, 0, NULL);
                        kinokoPassToPlayer(w->pass, plCheck());
                        w->flags |= 1;
                    }
                    if (w->timer == 260 / gRetracesPerFrame) {
                        scRunStart(0x20, 0, 0);
                        w->state = 9;
                    }
                    break;
                case 9:
                    if (!scIsRunning()) {
                        scRunStart(0x225, 0, 0);
                        w->state = 10;
                    }
                    break;
                case 10:
                    if (!scIsRunning()) {
                        personTalkCamOff();
                        demoPlReleaseAll();
                        camSunSetClockStop(1);
                        w->state = 0;
                    }
                    break;
            }
            eneFall(o, 0.4f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            if (w->flags & 1) {
                if (300 / gRetracesPerFrame == w->tuneTimer++) {
                    w->flags &= ~1;
                    camSunClearTunes();
                    camSunSetTunes4(0x84, 0x81, 0x82, 0x83);
                }
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.3f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            if (w->state == 3 || w->state == 6) {
                fadeAnimStep(&w->anim);
            }
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "kinoDemo": the hunt's manager. Made only while the pass is not given (0xA4) and the hunt is
 * open (0xD). The first time (0x1B) the hunt starts from its instructions: the clock goes to
 * 6:00, the day is 600 ticks long for one player and 720 for more; it shows the instructions
 * (scenario 0x259), then makes the mushroom display (kind 0x1C2) and the timer (kind 0x16D). At
 * 18:00 (or when a player dies: the mission's 2) the hunt ends (scenario 0x226), and the game
 * goes back to the mushroom lover's stage (0xCD) with 0x92 set. */
s32 demoKinoko_proc(Obj *o, s16 msg, void *arg) {
    KinoDemoW *w = KINODEMO(o);
    s32 pad[3]; /* the game's frame (0x60, its two call-address temps at 0x34 and 0x38) */
    CamView *view;
    s32 i;
    s32 n;

    switch (msg) {
        case 2:
            o->name = "kinoDemo";
            if (swWorkTest(0xA4)) {
                DPRINTF("demoKinoko_proc: 手形既にあり。キノコ採りは開始されません\n");
                return -1;
            }
            if (!swWorkTest(0xD)) {
                DPRINTF("demoKinoko_proc: キノコ採りは開始されません\n");
                return -1;
            }
            if (!swWorkTest(0x1B)) {
                DPRINTF("demoKinoko_proc: キノコ採りインストから開始します\n");
                swWorkSet(0x1B);
                gGameWork.time = 0x60000;
                gGameWork.scenarioRunning = 2;
                w->state = 0;
                gGameWork.savedDaySeconds = gGameWork.daySeconds;
                n = fadeCountPlayers();
                if (n == 1) {
                    gGameWork.daySeconds = 600;
                } else if (n == 2) {
                    gGameWork.daySeconds = 720;
                } else {
                    gGameWork.daySeconds = 720;
                }
            } else {
                DPRINTF("demoKinoko_proc: キノコ採りインストなし\n");
                w->state = 1;
            }
            gGameWork.mission = 1;
            gGameWork.areaStage = 3;
            break;
        case 1:
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (w->timer == 1) {
                        demoPlStandAll();
                    }
                    if (w->timer == 8) {
                        sndPlay(0xBA);
                        camSunSetClockStop(1);
                        w->partake = gGameWork.partake;
                        gGameWork.partake = 1;
                        scRunStart2(0x259, 0, 0);
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (!scIsRunning()) {
                        gGameWork.partake = w->partake;
                        camSunSetClockStop(0);
                        demoPlReleaseAll();
                        w->disp = eneMakeEx(NULL, ENE_DEMO_TIMER, 0, 0.0f, 0.0f, 0.0f, 2,
                                            (u32 (*)(void *, void *, void *)) 0x12);
                        w->timerObj = eneMakeEx(NULL, ENE_KINOKO_DISP, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (gGameWork.time >= 0x120000 && gGameWork.time < gGameWork.rate + 0x120000) {
                        camAiSetHold(1);
                        fadeSetColor(0, 0, 0);
                        fadeSetLevel(128.0f);
                        demoPlStandAll();
                        scRunStart2(0x226, 0, 0);
                        eneSetForward(o, 1);
                        gGameWork.partake = 1;
                        w->timer = 0;
                        if (gGameWork.kinokoCount >= 30) {
                            w->state = 3;
                        } else {
                            w->state = 4;
                        }
                    }
                    break;
                case 4:
                    if (40 / gRetracesPerFrame < w->timer && !scIsRunning()) {
                        gGameWork.scenarioRunning = 2;
                        sndPlay(10);
                        w->timer = 0;
                        w->state = 7;
                    }
                    break;
                case 3:
                    if (40 / gRetracesPerFrame < w->timer && !scIsRunning()) {
                        gGameWork.scenarioRunning = 2;
                        w->timer = 0;
                        w->state = 7;
                    }
                    break;
                case 7:
                    if (w->timer == 1) {
                        for (i = 0; i < 4; i++) {
                            contPakInitRumble(i);
                            contPakRumbleRequestStop(i);
                        }
                        swWorkReset(0x1B);
                        kinokoFadeOut(16 / gRetracesPerFrame);
                        sndClearReverb();
                        if (w->disp != NULL) {
                            w->disp = tskDestroy(w->disp);
                        }
                        if (w->timerObj != NULL) {
                            w->timerObj = tskDestroy(w->timerObj);
                        }
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        view = camGetView(w->cam);
                        camSetSpeed(w->cam, 20 / gRetracesPerFrame);
                        camMoveAtY(w->cam, view->at[1] + 50.0f);
                        gGameWork.scenarioRunning = 2;
                        gGameWork.daySeconds = gGameWork.savedDaySeconds;
                        if (gGameWork.kinokoCount < 30) {
                            sndPlay(0x45F);
                        }
                    }
                    if (w->timer == 20 / gRetracesPerFrame) {
                        gGameWork.areaStage = 3;
                        gGameWork.scenarioRunning = 0;
                        gGameWork.mission = 0;
                        gGameWork.partake = 0;
                        swWorkSet(0x92);
                        stageGotoSpawn(gStageTask->parent, 0xCD, 0, -19.0f, 10.0f, 17.0f, 3.1415927f, 0xD7);
                    }
                    break;
            }
            if (gGameWork.mission == 2 && w->state != 7) {
                DPRINTF("%%%%   event manager: PLAYER 死にました信号をうけとったぜ！\n");
                gGameWork.scenarioRunning = 2;
                if (gGameWork.kinokoCount < 30) {
                    sndPlay(10);
                }
                demoPlStandAll();
                w->timer = 0;
                w->state = 7;
            }
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "kinoko": a mushroom, id 1-30 (its placement's low byte), type 0 standing, 1 bobbing, 2
 * standing turned round. Made only if not picked yet; it spins, and picked (its box hit) it
 * counts (gGameWork.kinokoCount), sets its bit, and goes with a sparkle (kind 0x1C1). */
s32 kinoko_proc(Obj *o, s16 msg, void *arg) {
    KinokoW *w = KINOKO(o);
    u32 data;

    switch (msg) {
        case 2:
            o->name = "kinoko";
            data = eneSetup(o, arg);
            w->id = data & 0xFF;
            w->type = (data >> 8) & 0xFF;
            if (kinokoIsTaken(data)) {
                DPRINTF("kinoko_proc:id=[%d]既に採られている\n", w->id);
                return -1;
            }
            w->model = eneNewPol(o, kinokoMushroomDl);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 80, 80, 80, 0, 0);
            if (w->box == NULL) {
                return -1;
            }
            w->box->kind = 0x66;
            w->shadow = shadowCreate();
            DPRINTF("kinoko_proc: キノコセット id=[%d] kind=[%d]\n", w->id, w->type);
            switch (w->type) {
                case 0:
                default:
                    w->state = 0;
                    break;
                case 1:
                    w->state = 1;
                    break;
                case 2:
                    w->rot[0] = 3.1415927f;
                    w->state = 0;
                    break;
            }
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->box != NULL) {
                frmDestroy(&w->box->frame);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    break;
                case 1:
                    if (w->wave == 40) {
                        w->wave = 0;
                    }
                    w->vel[1] = mathCos(w->wave * 9 * 3.1415927f / 180.0f) * 1.5;
                    break;
                case 2:
                    gGameWork.kinokoCount++;
                    kinokoSetTaken(w->id);
                    DPRINTF("kinoko_proc: キノコ採られたid=[%d]  現在[%d]個\n", w->id, gGameWork.kinokoCount);
                    eneMakeEx(NULL, ENE_DEMO_KIRAKIRA, 0, w->model->pos[0], w->model->pos[1], w->model->pos[2], 0, NULL);
                    return -1;
            }
            if (acollAtkWasHit(w->box) == 1) {
                soundLocationCam(0x233, w->model->pos[0], w->model->pos[1], w->model->pos[2]);
                w->state = 2;
            }
            w->rot[1] += 0.06981317f;
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            w->wave++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Whether mushroom id has been picked (its bit in gGameWork.kinokoTaken). */
u8 kinokoIsTaken(u8 id) {
    return gGameWork.kinokoTaken[(u16) (id / 8)] & (u8) (1 << (u8) (id % 8));
}

/* Mushroom id is picked: its bit is set. */
void kinokoSetTaken(u8 id) {
    gGameWork.kinokoTaken[(u16) (id / 8)] |= (u8) (1 << (u8) (id % 8));
}

/* No mushroom picked, none counted. */
void kinokoClearAll(void) {
    s32 i;

    for (i = 0; i < 4; i++) {
        gGameWork.kinokoTaken[i] = 0;
    }
    gGameWork.kinokoCount = 0;
}

/* "kinokoCheck": the mushrooms picked (ids 1-30). */
s32 kinokoCheck(void) {
    s32 n = 0;
    s32 i;

    for (i = 1; i < 31; i++) {
        if (kinokoIsTaken(i)) {
            n++;
        }
    }
    DPRINTF("kinokoCheck 現在キノコ[%d]個です\n", n);
    return n;
}

/* "k_disp" (kinokoDisp_proc, its prints copied from demoTimer_proc): the mushrooms picked, on
 * an ortho camera of its own: a turning mushroom, a cross and two digits of gGameWork.kinokoCount,
 * each digit made again when it changes. */
s32 kinokoDisp_proc(Obj *o, s16 msg, void *arg) {
    KinoDispW *w = KINODISP(o);
    s32 i; /* the digits' loop */
    s32 j; /* the other loops: one variable for all of them colours the registers otherwise */
    s32 pad1; /* the game's frame: six words above d (0x80), four below it, the discarded */
    s32 tens; /* FileSet at 0x64; leftovers of demoTimer_proc's locals, it seems */
    s32 ones;
    s32 d[2];
    s32 pad2[4];

    switch (msg) {
        case 2:
            o->name = "k_disp";
            w->cam = (CamView *) frmCreate((Frame *) gFramePool10, gFrmKindCamera);
            if (w->cam == NULL) {
                DPRINTF(">>>>>  kinokoDisp_proc cam つくれません\n");
                return -1;
            }
            w->cam->ortho |= 1;
            w->cam->zNear = -200.0f;
            w->cam->zFar = 200.0f;
            floadLoadSet(0x83);
            DPRINTF("kinokoDisp_proc: セットＯＫ\n");
            break;
        case 1:
            if (w->cam != NULL) {
                frmDestroy(&w->cam->f);
            }
            if (w->cross != NULL) {
                frmDestroy(&w->cross->f);
            }
            for (j = 0; j < 2; j++) {
                if (w->digits[j] != NULL) {
                    frmDestroy(&w->digits[j]->f);
                }
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    DPRINTF("KINOKO_DISP_SETUP\n");
                    w->icon = (PolFrame *) frmCreate(&w->cam->f, gFrmKindPol);
                    if (w->icon == NULL) {
                        DPRINTF(">>>>>  demoTimer_proc kinoko つくれません\n");
                        return -1;
                    }
                    w->icon->dl = (Gfx *) fadeSegToRam(kinokoDispSetupDl, o->file);
                    w->icon->dl2 = kinokoMushroomDl;
                    w->icon->flags |= 0x20;
                    floadGetSetRam((void **) w->icon->segs, 0x87);
                    w->icon->rot[0] = 3.1415927f;
                    w->icon->pos[0] = 86.0f;
                    w->icon->pos[1] = 36.0f;
                    w->icon->scale[2] = 0.1f;
                    w->icon->scale[0] = w->icon->scale[1] = w->icon->scale[2];
                    w->cross = (PolFrame *) frmCreate(&w->cam->f, gFrmKindPol);
                    if (w->cross == NULL) {
                        DPRINTF(">>>>>  demoTimer_proc  ten つくれません\n");
                        return -1;
                    }
                    /* (s32), not (u32): IDO does not share an address's register between two
                     * casts of it, and the game loads it again here (lui/addiu a0). */
                    w->cross->dl = (Gfx *) fadeSegToRam(kinokoDispSetupDl, o->file);
                    w->cross->dl2 = kinokoDispCountCrossDl;
                    w->cross->flags |= 0x20;
                    floadGetSetRam((void **) w->cross->segs, 0x83);
                    w->cross->rot[0] = 3.1415927f;
                    w->cross->pos[0] = w->icon->pos[0] + 14.0f;
                    w->cross->pos[1] = w->icon->pos[1];
                    w->cross->scale[2] = 0.04f;
                    w->cross->scale[0] = w->cross->scale[1] = w->cross->scale[2];
                    /* On one line: the store comes before the branch, the step in its slot. */
                    for (j = 0; j < 2; j++) { w->shown[j] = 21; }
                    w->state = 1;
                    break;
                case 1:
                    if (gGameWork.kinokoCount == 0) {
                        tens = 0;
                    } else {
                        tens = gGameWork.kinokoCount / 10;
                    }
                    ones = gGameWork.kinokoCount % 10;
                    d[0] = tens;
                    d[1] = ones;
                    for (i = 0; i < 2; i++) {
                        if (w->shown[i] != d[i]) {
                            if (w->digits[i] != NULL) {
                                w->digits[i] = (PolFrame *) frmDestroy(&w->digits[i]->f);
                            }
                            w->shown[i] = d[i];
                            w->digits[i] = (PolFrame *) frmCreate(&w->cam->f, gFrmKindPol);
                            if (w->digits[i] == NULL) {
                                DPRINTF(">>>>>  demoTimer_proc counter[%d] つくれません\n", i);
                                return -1;
                            }
                            w->digits[i]->dl = (Gfx *) fadeSegToRam(kinokoDispSetupDl, o->file);
                            w->digits[i]->dl2 = kinokoDispDigits[d[i]];
                            w->digits[i]->flags |= 0x20;
                            floadGetSetRam((void **) w->digits[i]->segs, 0xDC);
                            w->digits[i]->pos[0] = w->icon->pos[0] + 22.0f + i * 10;
                            w->digits[i]->pos[1] = w->icon->pos[1] - 4.0f;
                            w->digits[i]->scale[2] = 0.24f;
                            w->digits[i]->scale[0] = w->digits[i]->scale[1] = w->digits[i]->scale[2];
                        }
                    }
                    w->icon->rot[1] += 0.034906585f;
                    w->state = 1;
                    break;
                case 2:
                    break;
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "kinoBig" (kinokoBabaBig_proc): once the pass is given (0xA4), the old woman is made big
 * (0x49) and it goes; made again after that, it goes at once. */
s32 kinokoBabaBig_proc(Obj *o, s16 msg, void *arg) {
    switch (msg) {
        case 2:
            o->name = "kinoBig";
            if (swWorkTest(0x49)) {
                DPRINTF(">>>>>>    kinokoBabaBig_proc: ばばすでにでかい  終了\n");
                return -1;
            }
            break;
        case 1:
            break;
        case 0:
            eneStepHookPre(o);
            if (ENE(o)->state == 0 && swWorkTest(0xA4)) {
                DPRINTF(">>>>>>    kinokoBabaBig_proc: ばばでかい\n");
                swWorkSet(0x49);
                return -1;
            }
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
