#include "tlb.h"

/* File 141: the courier (hikyaku) mission's people. "mi_zukin" and "mo_zukin"
 * (DtskZukinMon_proc: the triangle-hood man as the gatekeeper), "teleclub" (DtskTele_proc) and
 * an event manager. It begins with the same helpers as file 127 (the Beat Mania mission). */

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* A talker's work ("mi_zukin"), at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct ZukinW {
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
    /* 0x48 */ s32 timer; /* ticks since it was made */
} ZukinW;

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

void dtskFadeOut(f32 speed);
void dtskPassGive(Obj *o, Obj *player);

extern u8 dtskMiZukinStandAnim[]; /* the hood man's standing animation */
extern u8 dtskMiZukinTalkAnim[]; /* his talking animation */

/* A fade to black at the given speed (file 127's). One line: IDO schedules by line. */
void dtskFadeOut(f32 speed) { fadeSetSpeed(speed); fadeSetColor(0, 0, 0); fadeSetTarget(255.0f); }

/* The pass goes to the player (state 3; file 127's). */
void dtskPassGive(Obj *o, Obj *player) {
    PassW *w = (PassW *) FX(o);

    w->state = 3;
    w->player = player;
    w->timer = 0;
}

void dtskTegataSdSet(void) {
    DPRINTF("&&&& eventTegataSdSet  pl D がないn");
}

void dtskSdClear(void) {
    DPRINTF("&&&& SD_G_CLEAR1n");
    DPRINTF("&&&& SD_E_CLEAR1n");
    DPRINTF("&&&& SD_S_CLEAR1n");
    DPRINTF("&&&& SD_Y_CLEAR1n");
}

/* "mi_zukin": a hood man who talks (scenario 0x2A3); with flag 0x95 set he says more (0x2A4)
 * and the first again. */
s32 dtskMiZukinProc(Obj *o, s16 msg, void *arg) {
    ZukinW *w = (ZukinW *) FX(o);
    s32 pad;
    u32 data;
    s32 pad2[2];

    switch (msg) {
        case 2:
            o->name = "mi_zukin";
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) dtskMiZukinStandAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[2] = 0.17f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 100, 0, 0);
            demoTalkInit(o, data);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("beat_mania_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            w->rot[1] = 1.04311204f;
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            demoTalkerFree(o);
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (demoTalkEndCheck(o)) {
                        demoCamZoom_CL(w->talker);
                        w->state = 1;
                    }
                    break;
                case 1:
                    scRunStart(0x2A3, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    w->model->anim = dtskMiZukinTalkAnim;
                    w->model->frame = 0.0f;
                    w->state = 2;
                    break;
                case 2:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x95)) {
                            swWorkReset(0x95);
                            w->state = 3;
                        } else {
                            w->state = 5;
                        }
                    }
                    break;
                case 3:
                    scRunStart(0x2A4, 0, 0);
                    w->state = 4;
                    break;
                case 4:
                    if (!scIsRunning()) {
                        scRunStart(0x2A3, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        w->state = 5;
                    }
                    break;
                case 5:
                    if (!scIsRunning()) {
                        w->model->anim = dtskMiZukinStandAnim;
                        w->model->frame = 0.0f;
                        personTalkCamOff();
                        demoFinish(o);
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

/* The gatekeeper's work ("mo_zukin"), at its task + 0x20: a talker's (ZukinW) up to 0x48. */
typedef struct MoZukinW {
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
    /* 0x48 */ Obj *pass; /* the pass (kind 0x1C3) */
    /* 0x4C */ Camera *cam;
    /* 0x50 */ s32 timer; /* ticks in the state */
    /* 0x54 */ u8 flags; /* 1: the sky's tunes change 300 ticks after the pass */
    /* 0x55 */ u8 tuneTimer;
    /* 0x56 */ s8 partake; /* gGameWork.partake, kept while it talks */
} MoZukinW;

void dtskZukinMonSmoke(Obj *o);

extern Unk88282 gSaveCopyPlayers[4]; /* the players' entries for a continue */
extern u8 dtskZukinMonStandAnim[]; /* the gatekeeper's standing animation */
extern u8 dtskZukinMonTalkAnim[]; /* his talking animation */

/* "mo_zukin" (DtskZukinMon_proc): the triangle-hood man as the gatekeeper, gone while the courier
 * mission runs (flag 0x29 without 0xB6). He talks (0x2A0); with flag 0x94 the screen fades and
 * the game goes to stage 0xDC; with 0x92 (back from there) he talks to Ebisumaru (0x2A1) and,
 * with 0x93, gives the pass (kind 0x1C3) and the sky's tunes change. */
s32 dtskZukinMonProc(Obj *o, s16 msg, void *arg) {
    MoZukinW *w = (MoZukinW *) FX(o);
    s32 pad0;
    u32 data;
    CamView *view;
    s32 pad1;
    Vec3f at;
    s32 i;
    s32 n;
    f32 x;
    f32 y;
    f32 z;
    s32 pad2;
    Vec3f spark;
    s32 pad3[3];

    switch (msg) {
        case 2:
            o->name = "mo_zukin";
            if (swWorkTest(0x29) && !swWorkTest(0xB6)) {
                DPRINTF("****  三角頭巾 as 門番  飛脚ミッション中  終了\n");
                return -1;
            }
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) dtskZukinMonStandAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[2] = 0.17f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 100, 0, 0);
            demoTalkInit(o, data);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("*****   DtskZukinMon_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            w->rot[1] = 5.405745983f;
            if (swWorkTest(0x92)) {
                swWorkReset(0x92);
                w->partake = gGameWork.partake;
                gGameWork.partake = 1;
                gGameWork.scenarioRunning = 2;
                w->model->anim = dtskZukinMonTalkAnim;
                w->model->frame = 0.0f;
                w->state = 5;
            }
            DPRINTF(">>>>   DtskZukinMon_proc: SET OK!\n");
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            demoTalkerFree(o);
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
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
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (w->talker != NULL) {
                        tskSendMsg(w->talker, 4, &at);
                        if (eneTurnToPoint(o, at.x, at.z, 10.0f) == 1) {
                            w->state = 2;
                        }
                    }
                    break;
                case 2:
                    scRunStart(0x2A0, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    w->model->anim = dtskZukinMonTalkAnim;
                    w->model->frame = 0.0f;
                    w->state = 3;
                    break;
                case 3:
                    if (!scIsRunning()) {
                        w->model->anim = dtskZukinMonStandAnim;
                        w->model->frame = 0.0f;
                        if (swWorkTest(0x94)) {
                            swWorkReset(0x94);
                            gGameWork.scenarioRunning = 2;
                            w->timer = 0;
                            w->state = 4;
                        } else {
                            gGameWork.partake = w->partake;
                            demoFinish(o);
                            camAiSetHold(0);
                            camSunSetClockStop(0);
                            w->state = 0;
                        }
                    }
                    break;
                case 4:
                    if (w->timer == 1) {
                        sndPlay(10);
                        dtskFadeOut(16 / gRetracesPerFrame);
                        for (i = 0; i < 4; i++) {
                            gSaveCopyPlayers[i] = gSaveData.players[i];
                        }
                        for (i = 0; i < 4; i++) {
                            if (gSaveData.players[i].chara != 3) {
                                gSaveData.players[i].chara = -1;
                            }
                        }
                        fadeSetAll(16 / gRetracesPerFrame, 0.0f, 255.0f, 0, 0, 0);
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        view = camGetView(w->cam);
                        camSetSpeed(w->cam, 20 / gRetracesPerFrame);
                        camMoveAtY(w->cam, view->at[1] + 50.0f);
                        gGameWork.scenarioRunning = 2;
                    }
                    if (w->timer >= 20 / gRetracesPerFrame) {
                        if (!scIsRunning()) {
                            gGameWork.scenarioRunning = 0;
                            camAiSetHold(0);
                            stageGotoSpawn(gStageTask->parent, 0xDC, 0, 0.0f, 0.0f, 0.0f, 5.23598814f, 0x139);
                        }
                    }
                    break;
                case 5:
                    n = demoFindPlByChara(3);
                    personTalkCamAt(o, gGameWork.players[n], 80);
                    camSunSetClockStop(1);
                    demoPlStandAll();
                    scRunStart(0x2A1, n + 1, 3);
                    w->state = 6;
                    break;
                case 6:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x93)) {
                            swWorkReset(0x93);
                            w->state = 7;
                        } else {
                            w->state = 10;
                        }
                    }
                    break;
                case 7:
                    plGetModelPos(gGameWork.players[demoFindPlByChara(3)], &x, &y, &z);
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
                        dtskPassGive(w->pass, gGameWork.players[demoFindPlByChara(3)]);
                        w->flags |= 1;
                    }
                    if (w->timer == 260 / gRetracesPerFrame) {
                        scRunStart(0x20, demoFindPlByChara(3) + 1, 3);
                        w->state = 9;
                    }
                    break;
                case 9:
                    if (!scIsRunning()) {
                        scRunStart(0x2A2, demoFindPlByChara(3) + 1, 3);
                        w->state = 10;
                    }
                    break;
                case 10:
                    if (!scIsRunning()) {
                        gGameWork.partake = w->partake;
                        w->model->anim = dtskZukinMonStandAnim;
                        w->model->frame = 0.0f;
                        camSunSetClockStop(0);
                        camAiSetHold(0);
                        demoPlReleaseAll();
                        w->state = 0;
                    }
                    break;
            }
            if (w->flags & 1) {
                if (300 / gRetracesPerFrame == w->tuneTimer++) {
                    w->flags &= ~1;
                    camSunClearTunes();
                    camSunSetTunes4(0xBE, 0xBB, 0xBC, 0xBD);
                }
            }
            dtskZukinMonSmoke(o);
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

/* The gatekeeper's pipe smoke: a puff (kind 0xFD) at one of four places by his timer. */
void dtskZukinMonSmoke(Obj *o) {
    MoZukinW *w = (MoZukinW *) FX(o);

    switch (w->timer % 20) {
        case 0:
            eneMakeEx(NULL, ENE_ROOM_YADOYA_YUGE, 0, w->x - 5.0f, w->y + 20.0f, w->z - 5.0f, 0, NULL);
            break;
        case 4:
            eneMakeEx(NULL, ENE_ROOM_YADOYA_YUGE, 0, w->x + 2.0f, w->y + 22.0f, w->z, 0, NULL);
            break;
        case 10:
            eneMakeEx(NULL, ENE_ROOM_YADOYA_YUGE, 0, w->x + 9.0f, w->y + 16.0f, w->z + 5.0f, 0, NULL);
            break;
        case 16:
            eneMakeEx(NULL, ENE_ROOM_YADOYA_YUGE, 0, w->x - 5.0f, w->y + 16.0f, w->z + 5.0f, 0, NULL);
            break;
    }
}

/* The teleclub's work ("teleclub"), at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct TeleW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *pol; /* the scene */
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ Camera *cam;
    /* 0x38 */ LightFrame *light; /* its own light, */
    /* 0x3C */ AmbFrame *ambi; /* and ambient light */
    /* 0x40 */ s32 timer; /* ticks in the state */
    /* 0x44 */ s8 partake; /* gGameWork.partake, kept while it runs */
} TeleW;

extern Gfx dtskTeleSceneDl[]; /* the teleclub scene */
extern Unk88208 gSaveCopy; /* the save as it was (undefined_syms.txt; demo.c's) */

/* "teleclub" (DtskTele_proc): the courier mission's scene. A fixed camera, a blue light and a fade
 * in; Ebisumaru's talk (0x1AE before the game is cleared, flag 0xB3; 0x1AF after); the mission
 * runs (gGameWork.mission 1) until it is over (2, stopped here: "event manager: 中断"); then the
 * fade out, the players as they were, flag 0x92, and the game goes to stage 0x6E. */
/* Matched 2026-10-11; it was parked for case 1's first block, scheduled around w's address
 * against frmDestroy's; putting the first destroy and the second test on one source line
 * (IDO_NOTES 324, kensukeMoneyProc's find) gave it. */
s32 dtskTeleProc(Obj *o, s16 msg, void *arg) {
    TeleW *w = (TeleW *) FX(o);
    s32 pad;
    s32 i;
    s32 n;
    s32 pad2;

    switch (msg) {
        case 2:
            o->name = "teleclub";
            eneSetup(o, arg);
            w->pol = eneNewPol(o, dtskTeleSceneDl);
            camSunRefreshHold(1);
            camAiSetHold(1);
            w->cam = camAiGetCam();
            camSetEye(w->cam, 57.0f, 40.0f, -39.0f);
            camSetAt(w->cam, -53.0f, -2.0f, 37.0f);
            camGetView(w->cam);
            w->light = (LightFrame *) frmCreate(gFrmLayers[0], gFrmKindLight);
            w->light->color[0] = 0;
            w->light->color[1] = 0;
            w->light->color[2] = 255;
            w->light->dir[0] = 0;
            w->light->dir[1] = -100;
            w->light->dir[2] = -100;
            w->ambi = (AmbFrame *) frmCreate(gFrmLayers[0], gFrmKindAmbi);
            w->ambi->color[0] = 0;
            w->ambi->color[1] = 0;
            w->ambi->color[2] = 255;
            fadeSetLevel(255.0f);
            fadeSetColor(0, 0, 0);
            gGameState.fade = 0;
            gGameState.fadeFrom.r = 0;
            gGameState.fadeFrom.g = 0;
            gGameState.fadeFrom.b = 0;
            sndPlay(1);
            gGameWork.mission = 1;
            w->partake = gGameWork.partake;
            gGameWork.partake = 1;
            gGameWork.scenarioRunning = 2;
            fadeSetAll(79 / gRetracesPerFrame, 255.0f, 0.0f, 0, 0, 0);
            break;
        case 1:
            if (w->pol != NULL) { frmDestroy(&w->pol->f); } if (w->light != NULL) {
                frmDestroy(&w->light->f);
            }
            if (w->ambi != NULL) {
                frmDestroy(&w->ambi->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (w->timer == 8) {
                        sndPlay(0x207);
                    }
                    if (w->timer == 79 / gRetracesPerFrame) {
                        w->timer = 0;
                        w->state = 1;
                    }
                    break;
                case 1:
                    n = demoFindPlByChara(3);
                    if (!swWorkTest(0xB3)) {
                        DPRINTF("*******   DtskTele_proc: まだゲームクリアしていない\n");
                        scRunStart(0x1AE, n + 1, 3);
                    } else {
                        DPRINTF("*******    DtskTele_proc: ゲームクリアしていた\n");
                        scRunStart(0x1AF, n + 1, 3);
                    }
                    gGameWork.scenarioRunning = 0;
                    gGameWork.mission = 1;
                    gGameWork.partake = 1;
                    w->timer = 0;
                    w->state = 2;
                    break;
                case 2:
                    if (!scIsRunning()) {
                        gGameWork.scenarioRunning = 2;
                        if (!swWorkTest(0xB3)) {
                            if (!swWorkTest(0x2E)) {
                                sndPlay(10);
                            }
                        } else {
                            if (!swWorkTest(0x2F)) {
                                sndPlay(10);
                            }
                        }
                        w->timer = 0;
                        w->state = 3;
                    }
                    break;
                case 3:
                    if (w->timer == 1) {
                        gGameWork.scenarioRunning = 2;
                        fadeSetAll(30 / gRetracesPerFrame, 0.0f, 255.0f, 0, 0, 0);
                        if (!swWorkTest(0xB3)) {
                            if (!swWorkTest(0x2E)) {
                                sndPlay(0x45F);
                                camSunClearTunes();
                            }
                        } else {
                            if (!swWorkTest(0x2F)) {
                                sndPlay(0x45F);
                                camSunClearTunes();
                            }
                        }
                    }
                    if (w->timer == 30 / gRetracesPerFrame) {
                        gGameWork.areaStage = 3;
                        gGameWork.scenarioRunning = 0;
                        gGameWork.mission = 0;
                        gGameWork.partake = w->partake;
                        for (i = 0; i < 4; i++) {
                            if (gSaveCopy.players[i].chara != 3 && gSaveCopy.players[i].chara != -1) {
                                gSaveData.players[i] = gSaveCopy.players[i];
                            }
                        }
                        swWorkSet(0x92);
                        camSunSetClockStop(0);
                        stageGotoSpawn(gStageTask->parent, 0x6E, 0, -937.0f, 0.0f, 788.0f, 3.141592741f, 0xD7);
                    }
                    break;
            }
            if (gGameWork.mission == 2 && w->state != 3) {
                gGameWork.scenarioRunning = 2;
                sndPlay(10);
                DPRINTF("%%%%   event manager: 中断\n");
                scRunEnd();
                w->timer = 0;
                w->state = 2;
            }
            eneSetModelOnCourse(o, &w->pol->f);
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
