#include "tlb.h"

/* File 130: the kasa-jizou (straw hats for the stone jizou) mission. "jizou", the jizou who
 * sends the players to put the hats on the right jizou and gives a pass (tegata) for it;
 * "demojizou", the mission's manager; "kjizou", the jizou along the road (true ones and the
 * false "nise" ones); "kasa", the hats; and "k_disp", the count of hats left. */

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* The jizou's work ("jizou"), at its task + 0x20: a demo talker's (PersonObj) up to 0x44. */
typedef struct JizouW {
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
    /* 0x48 */ u8 tuneTimer;
    /* 0x4C */ Camera *cam;
    /* 0x50 */ Obj *pass; /* the pass (kind 0x1C3), made between it and the player */
    /* 0x54 */ u8 flags; /* 1: the sky's tunes change 300 ticks after the pass */
    /* 0x58 */ s32 timer; /* ticks in the state */
    /* 0x5C */ s8 partake; /* gGameWork.partake while it talks */
} JizouW;

#define JIZOU(obj) ((JizouW *) FX(obj))

/* The mission manager's work ("demojizou"), at its task + 0x20. */
typedef struct DemoJizouW {
    /* 0x00 */ Obj *disp; /* the hat count (kind 0x1C2) */
    /* 0x04 */ Obj *timerObj; /* the mission's timer (kind 0x177) */
    /* 0x08 */ Camera *cam;
    /* 0x0C */ u8 state;
    /* 0x10 */ s32 timer; /* ticks in the state */
    /* 0x14 */ s8 partake; /* gGameWork.partake while the instructions show */
} DemoJizouW;

#define DEMOJIZOU(obj) ((DemoJizouW *) FX(obj))

/* A road jizou's work ("kjizou"), at its task + 0x20: a demo talker's (PersonObj) up to 0x44. */
typedef struct KJizouW {
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
    /* 0x34 */ PersonData *data; /* talk: its kind; hours: its id */
    /* 0x38 */ ACollFrame *box; /* its talk box */
    /* 0x3C */ u8 flags3C;
    /* 0x3D */ u8 count;
    /* 0x3E */ u8 pad3E[2];
    /* 0x40 */ Obj *talker; /* the player talking to it */
    /* 0x44 */ MPolFrame *shadow;
    /* 0x48 */ u8 id; /* its bit in gGameWork.jizouKasa */
    /* 0x49 */ u8 type; /* 0 a true jizou, 1 a false one */
    /* 0x4C */ Obj *mark; /* kind 0x198, made under it */
    /* 0x50 */ s8 partake; /* gGameWork.partake while it talks */
} KJizouW;

#define KJIZOU(obj) ((KJizouW *) FX(obj))

/* A hat's work ("kasa"), at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct KasaW {
    /* 0x00 */ u8 state; /* 0 on its jizou, 1 going */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 pad09;
    /* 0x0A */ u8 hit; /* 1 on the ground */
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u8 id; /* its jizou's */
    /* 0x35 */ u8 flags; /* 1: rising (no fall) */
    /* 0x38 */ s32 timer;
} KasaW;

#define KASA(obj) ((KasaW *) FX(obj))

/* The hat count's work ("k_disp"), at its task + 0x20. */
typedef struct KasaDispW {
    /* 0x00 */ CamView *cam; /* its own ortho camera */
    /* 0x04 */ PolFrame *icon; /* a hat */
    /* 0x08 */ PolFrame *digit;
    /* 0x0C */ PolFrame *cross;
    /* 0x10 */ u8 shown; /* the digit shown (21: none yet; never updated) */
    /* 0x11 */ u8 state; /* 0 making the frames, 1 counting */
} KasaDispW;

#define KASADISP(obj) ((KasaDispW *) FX(obj))

/* The jizou with their hats on are a bit each in gGameWork.jizouKasa, by the jizou's id;
 * gGameWork.jizouCorrect counts the true ones and gGameWork.kasaCount all of them. */

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

/* acollAtkAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy,
                                     s16 oz, s32 a);

void jizouFadeOut(f32 speed);
void jizouPassToPlayer(Obj *o, Obj *player);
u8 jizouHasKasa(u8 id);
void jizouSetKasa(u8 id);
void jizouClearKasa(u8 id);
void jizouKasaClearAll(void);

extern Unk88282 gSaveCopyPlayers[4]; /* the players' entries for a continue (stage.c) */
extern u8 jizouAnim[]; /* the jizou's animation (segment 3 of its file) */
extern u8 kasaDispCrossDl[]; /* the count's cross (file set 0x83): jizouAnim's address in another file */
extern Gfx kasaHatDl[]; /* a hat */
extern u8 kasaDispDigitsDl0[];
extern u8 kasaDispDigitsDl1[];
extern u8 kasaDispDigitsDl2[];
extern u8 kasaDispDigitsDl3[];
extern u8 kasaDispDigitsDl4[];
extern u8 kasaDispDigitsDl7[];
extern u8 kasaDispDigitsDl8[];
Gfx kasaDispSetupDl[] = {
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
Gfx *kasaDispDigits[] = { kasaDispDigitsDl0, kasaDispDigitsDl1, kasaDispDigitsDl2, kasaDispDigitsDl3, kasaDispDigitsDl4, (Gfx *) 0x030007C0, (Gfx *) 0x030008A0, kasaDispDigitsDl7, kasaDispDigitsDl8, (Gfx *) 0x03000B40, NULL, NULL }; /* the digits 0-9 */

/* The screen fades to black at the given speed. The first call is on the brace's line: IDO
 * schedules by line, and only there does "sw ra" come after the call's address (as the game). */
void jizouFadeOut(f32 speed) { fadeSetSpeed(speed);
    fadeSetColor(0, 0, 0);
    fadeSetTarget(255.0f);
}

/* The pass goes to the player (state 3). */
void jizouPassToPlayer(Obj *o, Obj *player) {
    PassW *w = (PassW *) FX(o);

    w->state = 3;
    w->player = player;
    w->timer = 0;
}

/* The pass's sound ("eventTegataSdSet"), its prints all that is left. */
void jizouEventTegataSdSet(void) {
    DPRINTF("&&&& eventTegataSdSet  pl D がないn");
}

void jizouEventClearSd(void) {
    DPRINTF("&&&& SD_G_CLEAR1n");
    DPRINTF("&&&& SD_E_CLEAR1n");
    DPRINTF("&&&& SD_S_CLEAR1n");
    DPRINTF("&&&& SD_Y_CLEAR1n");
}

/* "jizou": the jizou who sets the mission. Talked to, he talks (scenario 0x221); if the player
 * takes it on (0x94) the screen fades and the game goes to the atlas with the mission's stages
 * open; back from it (0x92) he talks again, and if the player has done it (0x93) he gives the pass
 * between them (kind 0x1C3, sparks and kind 0x1C1), the pass goes to the player (scenarios 0x20,
 * 0x222), and the sky's tunes change. */
s32 jizou_proc(Obj *o, s16 msg, void *arg) {
    JizouW *w = JIZOU(o);
    s32 pad1;
    u32 data;
    CamView *view;
    s32 i;
    Obj *player;
    s32 pad[2];
    f32 x;
    f32 y;
    f32 z;
    Vec3f spark;

    switch (msg) {
        case 2:
            o->name = "jizou";
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) jizouAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 100, 0, 0);
            demoTalkInit(o, data);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("jizou_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            if (swWorkTest(0x92)) {
                swWorkReset(0x92);
                gGameWork.scenarioRunning = 2;
                w->partake = gGameWork.partake;
                gGameWork.partake = 1;
                camSunSetClockStop(1);
                w->state = 4;
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
                        w->partake = gGameWork.partake;
                        gGameWork.partake = 1;
                        personTalkCamAt(o, w->talker, 80);
                        camSunSetClockStop(1);
                        w->state = 1;
                    }
                    break;
                case 1:
                    scRunStart(0x221, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    DPRINTF("talkNo[%d]\n");
                    w->state = 2;
                    break;
                case 2:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x94)) {
                            swWorkReset(0x94);
                            gGameWork.scenarioRunning = 2;
                            w->timer = 0;
                            w->state = 3;
                        } else {
                            gGameWork.partake = w->partake;
                            camAiSetHold(0);
                            demoFinish(o);
                            camSunSetClockStop(0);
                            w->state = 0;
                        }
                    }
                    break;
                case 3:
                    if (w->timer == 1) {
                        sndPlay(10);
                        camSunSetClockStop(0);
                        jizouFadeOut(16 / gRetracesPerFrame);
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
                    if (w->timer >= 20 / gRetracesPerFrame && !scIsRunning()) {
                        gGameWork.scenarioRunning = 0;
                        gGameWork.partake = w->partake;
                        camAiSetHold(0);
                        demoAreaFlagSave();
                        demoAreaFlagSetBit7(2);
                        gSaveData.stageFlags[0x22] |= 1;
                        gSaveData.stageFlags[0x22] &= ~0x80;
                        gSaveData.stageFlags[0x1B] |= 1;
                        gSaveData.stageFlags[0x1B] &= ~0x80;
                        stageGotoAtlas(2, 3);
                    }
                    break;
                case 4:
                    if (w->timer == 1) {
                        demoPlStandAll();
                    }
                    if (w->timer == 8) {
                        if (gGameWork.jizouCorrect < 7) {
                            DPRINTF("ミッション失敗!![%d]\n", gGameWork.jizouCorrect);
                            camSunSetTunes4(0x84, 0x81, 0x82, 0x83);
                        }
                        personTalkCamAt(o, plCheck(), 80);
                        camSunSetClockStop(1);
                        demoAreaFlagRestore();
                        scRunStart(0x221, 0, 0);
                        w->state = 5;
                    }
                    break;
                case 5:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x93)) {
                            swWorkReset(0x93);
                            w->state = 6;
                        } else {
                            w->state = 9;
                        }
                    }
                    break;
                case 6:
                    player = plCheck();
                    if (player == NULL) {
                        return -1;
                    }
                    plGetModelPos(player, &x, &y, &z);
                    w->pass = eneMakeEx(NULL, ENE_DEMO_TEGATA, 0, (w->model->pos[0] + x) / 2, w->model->pos[1],
                                        (w->model->pos[2] + z) / 2, 0, NULL);
                    w->timer = 0;
                    w->state = 7;
                    break;
                case 7:
                    if (w->timer == 200 / gRetracesPerFrame) {
                        demoGetEneModelPos(w->pass, &spark);
                        ptclSpawnSparksFall(spark.x, spark.y, spark.z);
                        eneMakeEx(NULL, ENE_DEMO_KIRAKIRA, 0, spark.x, spark.y, spark.z, 0, NULL);
                        jizouPassToPlayer(w->pass, plCheck());
                        w->flags |= 1;
                    }
                    if (w->timer == 260 / gRetracesPerFrame) {
                        scRunStart(0x20, 0, 0);
                        w->state = 8;
                    }
                    break;
                case 8:
                    if (!scIsRunning()) {
                        scRunStart(0x222, 0, 0);
                        w->state = 9;
                    }
                    break;
                case 9:
                    if (!scIsRunning()) {
                        gGameWork.partake = w->partake;
                        camAiSetHold(0);
                        camSunSetClockStop(0);
                        demoPlReleaseAll();
                        w->state = 0;
                    }
                    break;
            }
            if (w->flags & 1) {
                if (300 / gRetracesPerFrame == w->tuneTimer++) {
                    w->flags &= ~1;
                    camSunSetTunes4(0x84, 0x81, 0x82, 0x83);
                }
            }
            eneFall(o, 0.4f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.5f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "demojizou": the mission's manager. Made only while the pass is not given (0xA3) and the
 * mission is on (0xA). The first time (0x1B) it starts from its instructions: the clock goes to
 * 6:00 and the day is 600 ticks long; it shows the instructions (scenario 0x255), then makes the
 * hat count (kind 0x1C2) and the timer (kind 0x177). At 18:00, or once the hats are on seven
 * jizou (gGameWork.jizouCorrect, scenario 0x21), or when a player dies (the mission's 2), it ends, and
 * the game goes back to the jizou's stage (0x47) with 0x92 set. */
s32 demoJizou_proc(Obj *o, s16 msg, void *arg) {
    DemoJizouW *w = DEMOJIZOU(o);
    s32 pad[2]; /* the game's frame (0x58, its two call-address temps at 0x34 and 0x38) */
    CamView *view;
    s32 i;

    switch (msg) {
        case 2:
            o->name = "demojizou";
            if (swWorkTest(0xA3)) {
                DPRINTF("demoKinoko_proc: 手形既にあり。地蔵は開始されません\n");
                return -1;
            }
            if (!swWorkTest(0xA)) {
                DPRINTF("demoKasaJizou_proc: JIZOU_MISSION たってません\n");
                return -1;
            }
            if (!swWorkTest(0x1B)) {
                DPRINTF("demoJizou_proc: 地蔵インストから開始します\n");
                swWorkSet(0x1B);
                gGameWork.time = 0x60000;
                gGameWork.savedDaySeconds = gGameWork.daySeconds;
                gGameWork.daySeconds = 600;
                gGameWork.scenarioRunning = 2;
                eneSetForward(o, 1);
                w->state = 0;
            } else {
                DPRINTF("demoJizou_proc: 地蔵インストなし\n");
                w->state = 7;
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
                        demoPlStandAll();
                        eneSetForward(o, 1);
                        jizouKasaClearAll();
                        w->partake = gGameWork.partake;
                        gGameWork.partake = 1;
                        scRunStart2(0x255, 0, 0);
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (!scIsRunning()) {
                        gGameWork.partake = w->partake;
                        camSunSetClockStop(0);
                        demoPlReleaseAll();
                        w->state = 7;
                    }
                    break;
                case 7:
                    w->disp = eneMakeEx(NULL, ENE_DEMO_TIMER, 0, 0.0f, 0.0f, 0.0f, 2,
                                        (u32 (*)(void *, void *, void *)) 0x12);
                    w->timerObj = eneMakeEx(NULL, ENE_KASA_DISP, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
                    eneSetForward(o, 0);
                    w->state = 2;
                    break;
                case 2:
                    if (gGameWork.time >= 0x120000 && gGameWork.time < gGameWork.rate + 0x120000) {
                        demoPlStandAll();
                        eneSetForward(o, 1);
                        scRunEnd();
                        w->timer = 0;
                        w->state = 3;
                    }
                    if (gGameWork.jizouCorrect >= 7) {
                        scRunStart2(0x21, 0, 0);
                        eneSetForward(o, 1);
                        gGameWork.partake = 1;
                        demoPlStandAll();
                        camAiSetHold(1);
                        fadeSetColor(0, 0, 0);
                        fadeSetLevel(128.0f);
                        w->timer = 0;
                        w->state = 4;
                    }
                    break;
                case 3:
                    if (40 / gRetracesPerFrame < w->timer && !scIsRunning()) {
                        gGameWork.scenarioRunning = 2;
                        sndPlay(10);
                        w->timer = 0;
                        w->state = 6;
                    }
                    break;
                case 4:
                    if (40 / gRetracesPerFrame < w->timer && !scIsRunning()) {
                        gGameWork.scenarioRunning = 2;
                        w->timer = 0;
                        w->state = 6;
                    }
                    break;
                case 6:
                    if (w->timer == 1) {
                        for (i = 0; i < 4; i++) {
                            contPakInitRumble(i);
                            contPakRumbleRequestStop(i);
                        }
                        swWorkReset(0x1B);
                        sndClearReverb();
                        eneSetForward(o, 1);
                        jizouFadeOut(16 / gRetracesPerFrame);
                        demoPlStandAll();
                        tskDestroy(w->disp);
                        tskDestroy(w->timerObj);
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        view = camGetView(w->cam);
                        camSetSpeed(w->cam, 20 / gRetracesPerFrame);
                        camMoveAtY(w->cam, view->at[1] + 50.0f);
                        gGameWork.scenarioRunning = 2;
                        gGameWork.daySeconds = gGameWork.savedDaySeconds;
                        if (gGameWork.jizouCorrect < 7) {
                            camSunClearTunes();
                            sndPlay(0x45F);
                        }
                    }
                    if (w->timer == 20 / gRetracesPerFrame) {
                        gGameWork.areaStage = 3;
                        gGameWork.scenarioRunning = 0;
                        gGameWork.mission = 0;
                        gGameWork.partake = 0;
                        swWorkSet(0x92);
                        stageGotoSpawn(gStageTask->parent, 0x47, 0, -366.0f, 0.0f, 137.0f, 2.79252696f, 0xD7);
                    }
                    break;
            }
            if (gGameWork.mission == 2 && w->state != 6) {
                DPRINTF("%%%%   event manager: PLAYER 死にました信号をうけとったぜ！\n");
                gGameWork.scenarioRunning = 2;
                sndPlay(10);
                demoPlStandAll();
                w->timer = 0;
                w->state = 6;
            }
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Matched 2026-10-12; it was parked for the id read's place: message 2's jizouHasKasa reads
 * w->id back in the type-store's jal slot, which two volatile accesses (the type stored, the
 * id read) gave; a natural two-case print switch (2026-10-17) later kept the same order without
 * them. */
/* "kjizou": a jizou along the road, id (its table entry's byte 0xA) and type (0 a true jizou,
 * 1 a false "nise" one). Talked to without its hat, a true one asks for one (scenario 0x252, 0xB:
 * a hat is put on it, counted in gGameWork.jizouCorrect and kasaCount); a false one talks (0x253) and
 * may end the mission (0x3A: the mission's 2); with its hat on it says no more. States 6-8 (0x254,
 * 0x1D: the hat taken back off) are entered from nowhere here. Made again with its hat on, it
 * puts the hat back (kind 0x175). */
s32 kasa_jizou_proc(Obj *o, s16 msg, void *arg) {
    KJizouW *w = KJIZOU(o);
    s32 pad1;
    u32 data;
    s32 pad2[3];
    Vec3f pos;

    switch (msg) {
        case 2:
            o->name = "kjizou";
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) jizouAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 100, 0, 0);
            demoTalkInit(o, data);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF(" kasa_jizou_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            w->id = w->data->hours;
            w->type = w->data->talk;
            switch (w->type) {
                case 0:
                    break;
                case 1:
                    DPRINTF(">>>>   にせ地蔵   ");
                    break;
            }
            DPRINTF("kasa_jizou_proc: id=[%d] kind=[%d]\n", w->id, w->type);
            if (jizouHasKasa(w->id)) {
                w->state = 10;
            }
            w->mark = eneMakeEx(o, ENE_ITEM_ATARI, 0, w->model->pos[0], w->model->pos[1], w->model->pos[2], 2,
                                (u32 (*)(void *, void *, void *)) 0x10201);
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            demoTalkerFree(o);
            DPRINTF("kasa_jizou_proc: 地蔵消去id=[%d]\n", w->id);
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (demoTalkEndCheck(o)) {
                        w->partake = gGameWork.partake;
                        gGameWork.partake = 1;
                        personTalkCamAt(o, w->talker, 100);
                        if (!jizouHasKasa(w->id)) {
                            if (w->type == 0) {
                                DPRINTF(">>>>   地蔵 Talk!  ");
                                w->state = 1;
                            } else if (w->type == 1) {
                                DPRINTF(">>>>   ごくあくにせ地蔵 Talk!  ");
                                w->state = 4;
                            }
                        } else {
                            w->state = 9;
                        }
                    }
                    break;
                case 1:
                    eneSetForward(o, 1);
                    camSunSetClockStop(1);
                    scRunStart2(0x252, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    w->state = 2;
                    break;
                case 2:
                    if (!scIsRunning()) {
                        if (swWorkTest(0xB)) {
                            swWorkReset(0xB);
                            w->state = 3;
                        } else {
                            w->state = 9;
                        }
                    }
                    break;
                case 3:
                    jizouSetKasa(w->id);
                    eneMakeEx(NULL, ENE_KASA, 0, w->model->pos[0], w->model->pos[1] + 5.0f, w->model->pos[2], 2,
                              (u32 (*)(void *, void *, void *)) (u32) w->id);
                    if (w->type != 1) {
                        gGameWork.jizouCorrect++;
                    }
                    gGameWork.kasaCount++;
                    DPRINTF("jizou: 笠(id=[%d]) プット 残り笠[%d] 正しい地蔵[%d]\n", w->id, gGameWork.kasaCount,
                            gGameWork.jizouCorrect);
                    eneMakeEx(NULL, ENE_DEMO_KIRAKIRA, 0, w->model->pos[0], w->model->pos[1] + 10.0f, w->model->pos[2], 0,
                              NULL);
                    soundIncamCam(0x100, w->model->pos[0], w->model->pos[1], w->model->pos[2]);
                    w->state = 9;
                    break;
                case 4:
                    eneSetForward(o, 1);
                    camSunSetClockStop(1);
                    scRunStart2(0x253, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    w->state = 5;
                    break;
                case 5:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x3A)) {
                            swWorkReset(0x3A);
                            eneSetForward(o, 0);
                            gGameWork.mission = 2;
                            gGameWork.partake = w->partake;
                            w->state = 11;
                        } else {
                            w->state = 9;
                        }
                    }
                    break;
                case 6:
                    scRunStart2(0x254, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    w->state = 7;
                    break;
                case 7:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x1D)) {
                            swWorkReset(0x1D);
                            w->state = 8;
                        } else {
                            w->state = 9;
                        }
                    }
                    break;
                case 8:
                    jizouClearKasa(w->id);
                    if (w->type != 1) {
                        gGameWork.jizouCorrect--;
                    }
                    gGameWork.kasaCount--;
                    DPRINTF("jizou: 笠(id=[%d]) プット 残り笠[%d] 正しい地蔵[%d]\n", w->id, gGameWork.kasaCount,
                            gGameWork.jizouCorrect);
                    w->state = 9;
                    break;
                case 9:
                    eneSetForward(o, 0);
                    gGameWork.partake = w->partake;
                    demoFinish(o);
                    camSunSetClockStop(0);
                    camAiSetHold(0);
                    if (!jizouHasKasa(w->id)) {
                        w->state = 0;
                    } else {
                        w->state = 11;
                    }
                    break;
                case 10:
                    DPRINTF("kasa_jizou_proc: 再発生した場合に傘を置くid=[%d]\n", w->id);
                    eneMakeEx(NULL, ENE_KASA, 0, w->model->pos[0], w->model->pos[1] + 5.0f, w->model->pos[2], 2,
                              (u32 (*)(void *, void *, void *)) (u32) w->id);
                    w->state = 11;
                    break;
                case 11:
                    break;
            }
            eneFall(o, 0.4f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.5f);
            eneGetWorldPos(o, &pos);
            shadowPut(w->shadow, pos.x, pos.y, pos.z, 1.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Whether jizou id has its hat (its bit in gGameWork.jizouKasa). */
u8 jizouHasKasa(u8 id) {
    return gGameWork.jizouKasa[(u16) (id / 8)] & (u8) (1 << (u8) (id % 8));
}

/* Jizou id has its hat: its bit is set. */
void jizouSetKasa(u8 id) {
    gGameWork.jizouKasa[(u16) (id / 8)] |= (u8) (1 << (u8) (id % 8));
}

/* Jizou id's hat is taken off: its bit is cleared. */
void jizouClearKasa(u8 id) {
    gGameWork.jizouKasa[(u16) (id / 8)] &= ~(u8) (1 << (u8) (id % 8));
}

/* No hats on any jizou, none counted. */
void jizouKasaClearAll(void) {
    s32 i;

    for (i = 0; i < 2; i++) {
        gGameWork.jizouKasa[i] = 0;
    }
    gGameWork.jizouCorrect = 0;
    gGameWork.kasaCount = 0;
}

/* Message 1's print is behind a test of the state (a compiled-out debug print, the IDO_NOTES 369
 * family): with that dead test the model goes straight into a0 for frmDestroy, and in message 2
 * data stays in its home (reloaded after the bnez), as in the game. Without it: real 11, the
 * model in a t-register and data reloaded into v1. Found 2026-10-12 by the permuter (an empty
 * "if (w->state) {}" in message 1, from the "volatile u32 data" form); with the print inside
 * the test neither volatile nor the old FAKE do-while split is needed. */
/* "kasa": a hat on jizou id (its placement's low byte). Once the jizou's bit is cleared (the
 * hat taken back), it rises and goes with a sparkle (kind 0x1C1). */
s32 kasa_proc(Obj *o, s16 msg, void *arg) {
    /* Set in each case, not at its declaration: the game keeps the task in s1 (IDO_NOTES 132). */
    KasaW *w;
    s32 pad;
    u32 data;

    switch (msg) {
        case 2:
            o->name = "kasa";
            data = eneSetup(o, arg);
            w = KASA(o);
            w->model = eneNewPol(o, kasaHatDl);
            if (w->model == NULL) {
                return -1;
            }
            w->id = data;
            break;
        case 1:
            w = KASA(o);
            if (w->state) {
                DPRINTF("kasa_proc: 笠消去id=[%d]\n", w->id);
            }
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
        case 0:
            w = KASA(o);
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (!jizouHasKasa(w->id)) {
                        DPRINTF("kasa id=[%d]消去\n", w->id);
                        w->timer = 0;
                        w->state = 1;
                    } else {
                        DPRINTF("kasa id=[%d]セット\n", w->id);
                    }
                    break;
                case 1:
                    if (w->timer == 1) {
                        w->flags |= 1;
                        w->vel[1] = 0.5f;
                    }
                    if (w->timer >= 60 / gRetracesPerFrame) {
                        eneMakeEx(NULL, ENE_DEMO_KIRAKIRA, 0, w->model->pos[0], w->model->pos[1] + 10.0f, w->model->pos[2], 0,
                                  NULL);
                        return -1;
                    }
                    break;
            }
            if (!(w->flags & 1)) {
                eneFall(o, 0.4f);
                w->hit &= ~1;
                if (eneLand((EneObj *) w, 10.0f) == 1) {
                    w->hit |= 1;
                    w->vel[1] = 0.0f;
                }
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void jizouNop(void) {
}

/* "k_disp" (kasaDisp_proc, its prints copied from demoTimer_proc): the hats left (7 less
 * gGameWork.kasaCount) on an ortho camera of its own: a hat, a cross and a digit. The digit is
 * made again every frame (shown is never updated). */
s32 kasaDisp_proc(Obj *o, s16 msg, void *arg) {
    KasaDispW *w = KASADISP(o);
    s32 pad1[5];
    s32 n;
    s32 pad2[2];

    switch (msg) {
        case 2:
            o->name = "k_disp";
            w->cam = (CamView *) frmCreate((Frame *) gFramePool10, gFrmKindCamera);
            if (w->cam == NULL) {
                DPRINTF(">>>>>  kasaDisp_proc cam つくれません\n");
                return -1;
            }
            w->cam->ortho |= 1;
            w->cam->zNear = -200.0f;
            w->cam->zFar = 200.0f;
            floadLoadSet(0x83);
            DPRINTF("kasaDisp_proc: セットＯＫ\n");
            break;
        case 1:
            /* On one line: w's addiu goes into the beqz's slot, as the game has it. */
            if (w->cam != NULL) { frmDestroy(&w->cam->f); }
            if (w->cross != NULL) {
                frmDestroy(&w->cross->f);
            }
            if (w->digit != NULL) {
                frmDestroy(&w->digit->f);
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    DPRINTF("KASA_DISP_SETUP\n");
                    w->icon = (PolFrame *) frmCreate(&w->cam->f, gFrmKindPol);
                    if (w->icon == NULL) {
                        DPRINTF(">>>>>  demoTimer_proc kasa つくれません\n");
                        return -1;
                    }
                    w->icon->dl = (Gfx *) fadeSegToRam(kasaDispSetupDl, o->file);
                    w->icon->dl2 = kasaHatDl;
                    w->icon->flags |= 0x20;
                    floadGetSetRam((void **) w->icon->segs, 0x89);
                    w->icon->rot[0] = 3.839724541f;
                    w->icon->pos[0] = 84.0f;
                    w->icon->pos[1] = 49.0f;
                    w->icon->scale[2] = 0.2f;
                    w->icon->scale[0] = w->icon->scale[1] = w->icon->scale[2];
                    w->cross = (PolFrame *) frmCreate(&w->cam->f, gFrmKindPol);
                    if (w->cross == NULL) {
                        DPRINTF(">>>>>  demoTimer_proc  ten つくれません\n");
                        return -1;
                    }
                    /* (s32), not (u32): IDO does not share an address's register between two
                     * casts of it, and the game loads it again here (lui/addiu a0). */
                    w->cross->dl = (Gfx *) fadeSegToRam(kasaDispSetupDl, o->file);
                    w->cross->dl2 = (Gfx *) kasaDispCrossDl; /* the count's file: its cross */
                    w->cross->flags |= 0x20;
                    floadGetSetRam((void **) w->cross->segs, 0x83);
                    w->cross->rot[0] = 3.1415927f;
                    w->cross->pos[0] = 100.0f;
                    w->cross->pos[1] = 36.0f;
                    w->cross->scale[2] = 0.04f;
                    w->cross->scale[0] = w->cross->scale[1] = w->cross->scale[2];
                    w->shown = 21;
                    w->state = 1;
                    break;
                case 1:
                    n = 7 - gGameWork.kasaCount;
                    if (n != w->shown) {
                        if (w->digit != NULL) {
                            w->digit = (PolFrame *) frmDestroy(&w->digit->f);
                        }
                        w->digit = (PolFrame *) frmCreate(&w->cam->f, gFrmKindPol);
                        if (w->digit == NULL) {
                            DPRINTF(">>>>>  demoTimer_proc counter つくれません\n");
                            return -1;
                        }
                        w->digit->dl = (Gfx *) fadeSegToRam(kasaDispSetupDl, o->file);
                        w->digit->dl2 = kasaDispDigits[n];
                        w->digit->flags |= 0x20;
                        floadGetSetRam((void **) w->digit->segs, 0xDC);
                        w->digit->pos[0] = 108.0f;
                        w->digit->pos[1] = 32.0f;
                        w->digit->scale[2] = 0.24f;
                        w->digit->scale[0] = w->digit->scale[1] = w->digit->scale[2];
                    }
                    w->state = 1;
                    break;
                case 2:
                    break;
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
