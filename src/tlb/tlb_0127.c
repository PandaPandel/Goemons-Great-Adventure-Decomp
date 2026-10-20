#include "tlb.h"

/* File 127: the Beat Mania mission. "b-mania", the DJ who sends the players to collect his three
 * items (the LP, flag 1; the mic, 2; the headphones, 3) and gives a pass (tegata) when he has
 * them; "demoBeat", the mission's manager; "lpMonster", the monster that carries the LP; the
 * items ("beat_lp", "beat_mic", "hphone") and their displays. */

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* The DJ's work ("b-mania"), at its task + 0x20: a demo talker's (PersonObj) up to 0x44. */
typedef struct BManiaW {
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
    /* 0x4C */ Obj *pass; /* the pass (kind 0x1C3) */
    /* 0x50 */ u8 prevState; /* the state the talk began in */
    /* 0x54 */ s32 timer; /* ticks in the state */
    /* 0x58 */ u8 flags; /* 1: the sky's tunes change 300 ticks after the pass */
    /* 0x59 */ u8 tuneTimer;
} BManiaW;

#define BMANIA(obj) ((BManiaW *) FX(obj))

/* The mission manager's work ("demoBeat"), at its task + 0x20. */
typedef struct DemoBeatW {
    /* 0x00 */ Obj *lp; /* the LP's monster (kind 0x168) */
    /* 0x04 */ Obj *mic; /* the mic (kind 0x163) */
    /* 0x08 */ Obj *hphone; /* the headphones (kind 0x164) */
    /* 0x0C */ Obj *disp; /* the items' display (kind 0x1C2) */
    /* 0x10 */ Camera *cam;
    /* 0x14 */ u8 pad14[4];
    /* 0x18 */ s8 partake; /* gGameWork.partake while the instructions show */
    /* 0x19 */ u8 state;
    /* 0x1C */ s32 timer; /* ticks in the state */
    /* 0x20 */ s32 daySeconds; /* gGameWork.daySeconds, kept while the mission runs */
} DemoBeatW;

#define DEMOBEAT(obj) ((DemoBeatW *) FX(obj))

/* The LP's monster's work ("lpMonster"), at its task + 0x20: an enemy's (EneObj) up to 0x40. */
typedef struct LpMonsterW {
    /* 0x00 */ u8 state; /* 0, 1 walking one way and back, 2 hit, 3 beaten */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit; /* 1 on the ground */
    /* 0x0B */ s8 hp0B;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ MPolFrame *shadow;
    /* 0x40 */ s8 hp; /* 30 hits */
    /* 0x44 */ f32 angle; /* the way it walks (eneCourseAngle), in radians */
    /* 0x48 */ u8 savedState; /* the state and its ticks when it was hit */
    /* 0x4C */ s32 savedTimer;
    /* 0x50 */ s32 timer; /* ticks in the state */
    /* 0x54 */ Obj *child; /* kind 0x198, made with it, following its model */
    /* 0x58 */ Obj *lp; /* the LP on its back (kind 0x162) */
    /* 0x5C */ f32 lpHeight; /* the LP's height above it */
    /* 0x60 */ f32 push[3]; /* its damage box's push */
} LpMonsterW;

#define LPMONSTER(obj) ((LpMonsterW *) FX(obj))

/* Kind 0x198's work, as far as the LP's monster moves it: an enemy's model and place. */
typedef struct FollowW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
} FollowW;

/* An item's work ("beat_lp", "beat_mic", "hphone"), at its task + 0x20: an enemy's (EneObj) up
 * to 0x38. */
typedef struct BeatItemW {
    /* 0x00 */ u8 state; /* 0 carried (the LP), 1 falling, 2 taken */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit; /* 1 on the ground */
    /* 0x0B */ s8 hp0B;
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *atk; /* its touch box (kind 0x66) */
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ MPolFrame *shadow;
    /* 0x40 */ s32 timer; /* ticks in the state */
} BeatItemW;

#define BEATITEM(obj) ((BeatItemW *) FX(obj))

/* An item's display's work ("lpDisp", "micDisp", "hpDisp"), at its task + 0x20: the item's model
 * on the display's camera, growing in and then turning with the others. */
typedef struct ItemDispW {
    /* 0x00 */ PolFrame *pol;
    /* 0x04 */ u8 state; /* 0 growing, 2 shown */
    /* 0x08 */ s32 timer; /* ticks in the state */
} ItemDispW;

#define ITEMDISP(obj) ((ItemDispW *) FX(obj))

/* An animated item's display's work ("micDisp", "hpDisp"): two models on the display's camera. */
typedef struct AnimeDispW {
    /* 0x00 */ AnimeFrame *anime[2];
    /* 0x08 */ u8 state; /* 0 growing, 2 shown */
    /* 0x0C */ s32 timer; /* ticks in the state */
} AnimeDispW;

#define ANIMEDISP(obj) ((AnimeDispW *) FX(obj))

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

/* acollAtkAddYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);

/* acollDmgAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*DmgAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height,
                                     s16 ox, s16 oy, s16 oz, s32 a);

void beatFadeOut(f32 speed);
void beatPassGive(Obj *o, Obj *player);
void beatLpMonsterChildSet(Obj *o, f32 x, f32 y, f32 z, f32 sx, f32 sy, f32 sz);
void beatLpDrop(Obj *o);
void beatLpSetPos(Obj *o, f32 x, f32 y, f32 z, f32 ry);
f32 beatGetRotY(Obj *o);
void beatItemSdSet(Obj *o);
Obj *beatItemDispParentCreate(Obj *o);

extern Unk88282 gSaveCopyPlayers[4]; /* the players' entries for a continue */
extern u8 beatManiaDJAnim[]; /* the DJ's animation */
extern u8 lpMonsterAnim[]; /* the LP's monster's animation */
extern u8 lpMonsterHalf0Dl[]; /* the LP's monster's halves, for its death (demoEneDeathAct) */
extern u8 lpMonsterHalf1Dl[];
extern u8 beatLPDl[]; /* the LP's display list */
extern u8 beatMicAnim[]; /* the mic's animation */
extern u8 beatMicDispAnim[]; /* the mic's second model's animation */
extern u8 beatHeadphonesDl[]; /* the headphones' display list */

Gfx beatItemDispDl[] = {
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
}; /* the items' display's render setup (a display list) */

CamView *beatItemDispView = (CamView *) 0x12345670; /* the items' display's camera (an ortho view) */
Obj *beatLp = (Obj *) 0x12345670; /* the LP the monster carries */
Obj *beatItemDispParent = (Obj *) 0x12345670; /* the mission's task (beatItemDispParentProc) */

/* The screen fades to black at the given speed. The first call is on the brace's line: IDO
 * schedules by line, and only there (or with the whole body on one line) does "sw ra" come
 * after the call's address, as the game has it (measured). */
void beatFadeOut(f32 speed) { fadeSetSpeed(speed);
    fadeSetColor(0, 0, 0);
    fadeSetTarget(255.0f);
}

/* The pass goes to the player (state 3). */
void beatPassGive(Obj *o, Obj *player) {
    PassW *w = (PassW *) FX(o);

    w->state = 3;
    w->player = player;
    w->timer = 0;
}

/* The pass's sound ("eventTegataSdSet") and the clear's, emptied; their prints' strings remain.
 * Nothing calls the second. */
void beatEventTegataSdSet(void) {
    DPRINTF("&&&& eventTegataSdSet  pl D がないn");
}

void beatClearSdSet(void) {
    DPRINTF("&&&& SD_G_CLEAR1n");
    DPRINTF("&&&& SD_E_CLEAR1n");
    DPRINTF("&&&& SD_S_CLEAR1n");
    DPRINTF("&&&& SD_Y_CLEAR1n");
}

/* "b-mania": the DJ. */
s32 beatManiaProc(Obj *o, s16 msg, void *arg) {
    BManiaW *w = BMANIA(o);
    s32 pad0;
    u32 data;
    CamView *view;
    Obj *pl;
    Vec3f at;
    s32 i;
    s32 pad[2];
    f32 x;
    f32 y;
    f32 z;
    Vec3f spark;

    switch (msg) {
        case 2:
            o->name = "b-mania";
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) beatManiaDJAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 100, 0, 0);
            demoTalkInit(o, data);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("beat_mania_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            if (swWorkTest(0x92)) {
                gGameWork.scenarioRunning = 2;
                camSunSetClockStop(1);
                w->state = 11;
            }
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
                    if (eneMoveToPos(o, -19.0f, 7.0f, 2)) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (eneMoveToPos(o, -19.0f, -35.0f, 2)) {
                        w->state = 0;
                    }
                    break;
                case 2:
                    if (w->talker != NULL) {
                        tskSendMsg(w->talker, 4, &at);
                        if (eneTurnToPoint(o, at.x, at.z, 18.0f) == 1) {
                            w->state = 4;
                        }
                    }
                    break;
                case 4:
                    scRunStart(w->data->talk, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    DPRINTF("talkNo[%d]\n", w->data->talk);
                    w->state = 5;
                    break;
                case 5:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x94)) {
                            swWorkReset(0x94);
                            gGameWork.scenarioRunning = 2;
                            w->state = 9;
                            w->timer = 0;
                        } else {
                            personTalkCamOff();
                            demoFinish(o);
                            w->state = w->prevState;
                        }
                    }
                    break;
                case 9:
                    if (w->timer == 1) {
                        beatFadeOut(16 / gRetracesPerFrame);
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        view = camGetView(w->cam);
                        camSetSpeed(w->cam, 10.0f);
                        camMoveAtY(w->cam, view->at[1] + 50.0f);
                        sndPlay(10);
                        gGameWork.scenarioRunning = 2;
                        for (i = 0; i < 4; i++) {
                            gSaveCopyPlayers[i].chara = -1;
                        }
                    }
                    if (w->timer == 20 / gRetracesPerFrame) {
                        gGameWork.scenarioRunning = 0;
                        camAiSetHold(0);
                        demoAreaFlagSave();
                        demoAreaFlagSetBit7(0);
                        gSaveData.stageFlags[1] |= 1;
                        gSaveData.stageFlags[1] &= ~0x80;
                        gSaveData.areaFlags[1] |= 1;
                        gSaveData.areaFlags[1] &= ~0x80;
                        stageGotoAtlas(0, 2);
                    }
                    break;
                case 11:
                    if (w->timer == 1) {
                        demoPlStandAll();
                    }
                    if (w->timer == 8) {
                        if (!swWorkTest(1) || !swWorkTest(2) || !swWorkTest(3)) {
                            camSunSetTunes4(0x79, 0x76, 0x77, 0x78);
                            sndPlay(0x51F);
                        }
                        personCamZoomUp(plCheck());
                        swWorkReset(0x92);
                        demoAreaFlagRestore();
                        scRunStart(w->data->talk, 0, 0);
                        w->state = 12;
                    }
                    break;
                case 12:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x93)) {
                            swWorkReset(0x93);
                            w->state = 6;
                        } else {
                            personTalkCamOff();
                            demoPlReleaseAll();
                            w->state = 0;
                        }
                    }
                    break;
                case 6:
                    pl = plCheck();
                    if (pl == NULL) {
                        return -1;
                    }
                    plGetModelPos(pl, &x, &y, &z);
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
                        beatPassGive(w->pass, plCheck());
                        w->flags |= 1;
                    }
                    if (w->timer == 260 / gRetracesPerFrame) {
                        scRunStart(0x20, 0, 0);
                        w->state = 8;
                    }
                    break;
                case 8:
                    if (!scIsRunning()) {
                        scRunStart(0x1BA, 0, 0);
                        w->state = 13;
                    }
                    break;
                case 13:
                    if (!scIsRunning()) {
                        personTalkCamOff();
                        demoPlReleaseAll();
                        camSunSetClockStop(1);
                        w->state = 0;
                    }
                    break;
            }
            eneStepHookPre(o);
            if (w->flags & 1) {
                if (300 / gRetracesPerFrame == w->tuneTimer++) {
                    w->flags &= ~1;
                    camSunSetTunes4(0x79, 0x76, 0x77, 0x78);
                }
            }
            personPlBlockCheck(o);
            if (w->state == 0 || w->state == 1) {
                if (demoTalkEndCheck(o)) {
                    demoCamZoom_CL(w->talker);
                    w->prevState = w->state;
                    w->vel[0] = 0.0f;
                    w->vel[2] = 0.0f;
                    camSunSetClockStop(1);
                    w->state = 2;
                }
                eneFaceAwayMove(o);
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
            eneAnimLoop(o, w->model, 0.8f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "demoBeat": the mission's manager. Made once the DJ has asked (flag 0) and until the pass is
 * given (0x96). The first time (0x1B) the instructions show; then it resets the three items'
 * flags (1-3), makes them and their display, and runs until the evening (everyone goes home) or
 * until all three are found; then the screen fades and the game goes back to the DJ (0x92). */
s32 demoBeat_proc(Obj *o, s16 msg, void *arg) {
    DemoBeatW *w = DEMOBEAT(o);
    s32 pad;
    CamView *view;
    s32 i;

    switch (msg) {
        case 2:
            o->name = "demoBeat";
            if (!swWorkTest(0)) {
                DPRINTF("demoBeat_proc: ビートマニアアイテム収集は開始されません\n");
                return -1;
            }
            if (swWorkTest(0x96)) {
                DPRINTF("demoBeat_proc: ビートマニアアイテム収集は開始されません 手形もらってるで\n");
                return -1;
            }
            if (!swWorkTest(0x1B)) {
                swWorkSet(0x1B);
                eneSetForward(o, 1);
                gGameWork.time = 0x60000;
                w->state = 0;
            } else {
                w->state = 1;
            }
            beatItemDispParent = beatItemDispParentCreate(o);
            if (beatItemDispParent == NULL) {
                return -1;
            }
            beatItemDispView = (CamView *) frmCreate((Frame *) gFramePool10, gFrmKindCamera);
            if (beatItemDispView == NULL) {
                return -1;
            }
            beatItemDispView->ortho |= 1;
            beatItemDispView->zNear = -200.0f;
            beatItemDispView->zFar = 200.0f;
            w->daySeconds = gGameWork.daySeconds;
            gGameWork.daySeconds = 480;
            gGameWork.mission = 1;
            gGameWork.scenarioRunning = 2;
            gGameWork.areaStage = 2;
            DPRINTF("demoBeat セット完了\n");
            break;
        case 1:
            if (beatItemDispView != NULL) {
                frmDestroy((Frame *) beatItemDispView);
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (w->timer == 1) {
                        demoPlStandAll();
                    }
                    if (w->timer == 8) {
                        camSunSetClockStop(1);
                        w->partake = gGameWork.partake;
                        gGameWork.partake = 1;
                        scRunStart2(0x1F1, 0, 0);
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (!scIsRunning()) {
                        gGameWork.partake = w->partake;
                        camSunSetClockStop(0);
                        demoPlReleaseAll();
                        swWorkReset(1);
                        swWorkReset(2);
                        swWorkReset(3);
                        w->lp = eneMakeEx(NULL, ENE_LP_MONSTER, 1, 5213.0f, 59.0f, 0.0f, 0, NULL);
                        w->mic = eneMakeEx(NULL, ENE_MIC, 3, 155.0f, 184.0f, 0.0f, 0, NULL);
                        w->hphone = eneMakeEx(NULL, ENE_HEAD_PHONE, 2, 659.0f, 210.0f, 0.0f, 0, NULL);
                        w->disp = eneMakeEx(NULL, ENE_DEMO_TIMER, 0, 0.0f, 0.0f, 0.0f, 2,
                                            (u32 (*)(void *, void *, void *)) 0x12);
                        eneSetForward(o, 0);
                        DPRINTF("demoBeat_proc: アイテム拾いスタート\n");
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (gGameWork.time >= 0x120000 && gGameWork.time < gGameWork.rate + 0x120000) {
                        DPRINTF(">>>>   時刻チェック みんな帰れ！\n");
                        demoPlStandAll();
                        w->timer = 0;
                        w->state = 3;
                    } else if (swWorkTest(1) && swWorkTest(2) && swWorkTest(3)) {
                        DPRINTF(">>>>   アイテム全部そろえたら終了\n");
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
                        w->state = 7;
                    }
                    break;
                case 4:
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
                        if (beatItemDispParent != NULL) {
                            beatItemDispParent = tskDestroy(beatItemDispParent);
                        }
                        beatFadeOut(16 / gRetracesPerFrame);
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        view = camGetView(w->cam);
                        camSetSpeed(w->cam, 20 / gRetracesPerFrame);
                        camMoveAtY(w->cam, view->at[1] + 50.0f);
                        if (w->disp != NULL) {
                            w->disp = tskDestroy(w->disp);
                        }
                        swWorkReset(0x1B);
                        gGameWork.scenarioRunning = 2;
                        gGameWork.daySeconds = w->daySeconds;
                        eneSetForward(o, 1);
                        if (!swWorkTest(1) || !swWorkTest(2) || !swWorkTest(3)) {
                            sndPlay(0x45F);
                        }
                    }
                    if (w->timer == 20 / gRetracesPerFrame) {
                        gGameWork.areaStage = 2;
                        swWorkSet(0x92);
                        gGameWork.scenarioRunning = 0;
                        gGameWork.mission = 0;
                        gGameWork.partake = 0;
                        stageGotoSpawn(gStageTask->parent, 0x9C, 0, -19.0f, 10.0f, 15.0f, 3.1415927f, 0xD7);
                    }
                    break;
            }
            DPRINTF("\rgw.event_break [%d]\n", gGameWork.mission);
            if (gGameWork.mission == 2 && w->state != 7) {
                DPRINTF("%%%%   event manager: PLAYER 死にました信号をうけとったぜ！\n");
                gGameWork.scenarioRunning = 2;
                sndPlay(10);
                demoPlStandAll();
                w->timer = 0;
                w->state = 7;
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "lpMonster": the monster that carries the LP. Made in the mission (flag 0) while the LP is
 * not found (1). It walks one way and back; hit, it squashes and springs back; after 30 hits it
 * dies, drops three things (kinds 0x1FC, 0x1FB, 0x1FD) and lets the LP fall. */
s32 lpMonster_proc(Obj *o, s16 msg, void *arg) {
    LpMonsterW *w = LPMONSTER(o);

    switch (msg) {
        case 2:
            o->name = "lpMonster";
            if (!swWorkTest(0)) {
                DPRINTF("lpMonster_proc: BEAT_MISSION たっていません\n");
                return -1;
            }
            if (swWorkTest(1)) {
                DPRINTF("lpMonster_proc :BEAT_LP たっています\n");
                return -1;
            }
            DPRINTF("lpMonster_proc: セットします\n");
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) lpMonsterAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[2] = 0.3f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("lpMonster_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->dmg = ((DmgAddXYZFunc) acollDmgAddXYZ)(ene_dmg_root_frame, &w->model->f, 70, 160, 0, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->push = w->push;
            w->hp = 30;
            w->shadow = shadowCreate();
            w->lpHeight = 42.0f;
            beatLp = w->lp = eneMakeEx(NULL, ENE_LP, 0, w->model->pos[0], w->model->pos[1] + 42.0f,
                                                    w->model->pos[2], 0, NULL);
            w->child = eneMakeEx(o, ENE_ITEM_ATARI, 0, w->model->pos[0], w->model->pos[1], w->model->pos[2], 2,
                                 (u32 (*)(void *, void *, void *)) 0x20302);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->atk != NULL) {
                frmDestroy((Frame *) w->atk);
            }
            if (w->dmg != NULL) {
                frmDestroy((Frame *) w->dmg);
            }
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (w->timer == 1) {
                        w->angle = eneCourseAngle(o, 1);
                    }
                    w->vel[0] = 0.5f;
                    if (w->timer >= 360 / gRetracesPerFrame) {
                        w->state = 1;
                        w->timer = 0;
                    }
                    break;
                case 1:
                    if (w->timer == 1) {
                        w->angle = eneCourseAngle(o, 4);
                    }
                    w->vel[0] = -0.5f;
                    if (w->timer >= 360 / gRetracesPerFrame) {
                        w->state = 0;
                        w->timer = 0;
                    }
                    break;
                case 2:
                    if (w->timer == 1) {
                        DPRINTF("lpMonster_proc: いて〜〜");
                        w->hp--;
                        w->vel[0] = 0.0f;
                        DPRINTF("HP[%d]\n", w->hp);
                        if (w->hp <= 0) {
                            w->state = 3;
                            w->dmg->flags &= ~4;
                        }
                        eneHitSoundAlt(w->dmg);
                        eneMakeHitSpark(0, w->dmg->push[0], w->dmg->push[1], w->dmg->push[2], 3, w->rot[1]);
                    }
                    if (w->hp >= 28) {
                        if (w->timer < 8 / gRetracesPerFrame) {
                            w->model->scale[0] -= 0.03f;
                            w->model->scale[1] -= 0.03f;
                            w->model->scale[2] -= 0.03f;
                            w->lpHeight -= 1.8f;
                            DPRINTF("high[%f]\n", w->lpHeight);
                            break;
                        }
                        if (w->timer < 16 / gRetracesPerFrame) {
                            w->model->scale[0] += 0.01f;
                            w->model->scale[1] += 0.01f;
                            w->model->scale[2] += 0.01f;
                            w->lpHeight += 0.15f;
                            DPRINTF("high[%f]\n", w->lpHeight);
                            break;
                        }
                    }
                    if (w->timer >= 20 / gRetracesPerFrame) {
                        w->dmg->flags |= 2;
                        w->dmg->flags |= 4;
                        w->state = w->savedState;
                        w->timer = w->savedTimer;
                    }
                    break;
                case 3:
                    DPRINTF("lpMonster_proc: 死んだ\n");
                    demoEneDeathAct(o, w->dmg, (u32) lpMonsterHalf0Dl, (u32) lpMonsterHalf1Dl, 1);
                    eneMakeEx(NULL, ENE_ETSK_DANGO_MAIN, w->zone, w->x - 4.0f, w->y, 0.0f, 0, NULL);
                    eneMakeEx(NULL, ENE_ETSK_KOBAN_MAIN, w->zone, w->x + 4.0f, w->y, 0.0f, 0, NULL);
                    eneMakeEx(NULL, ENE_ETSK_NEKO_MAIN_FLAGS_0, w->zone, w->x + 8.0f, w->y, 0.0f, 0, NULL);
                    if (beatLp != NULL) {
                        beatLpDrop(w->lp);
                        beatLp = NULL;
                    }
                    return -1;
            }
            if (acollDmgWasHit(w->dmg) == 1) {
                if (w->state != 2 && w->state != 3) {
                    w->savedState = w->state;
                    w->savedTimer = w->timer;
                    w->dmg->flags &= ~2;
                    w->state = 2;
                    w->timer = 0;
                }
            }
            if (w->state != 2) {
                eneTurnToAngle(o, w->angle * 180.0f / 3.1415927f, 5.0f);
                eneAnimLoop(o, w->model, 0.3f);
            }
            eneMove((EneObj *) w);
            eneFall(o, 0.4f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 4.0f, NULL);
            if (beatLp != NULL) {
                beatLpSetPos(w->lp, w->model->pos[0], w->model->pos[1] + w->lpHeight, w->model->pos[2],
                                       w->model->rot[1] + 1.5707964f);
            }
            beatLpMonsterChildSet(w->child, w->model->pos[0], w->model->pos[1], w->model->pos[2],
                                   w->model->scale[0], w->model->scale[1], w->model->scale[2]);
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Kind 0x198 follows the LP's monster: its place and its model's scale. */
void beatLpMonsterChildSet(Obj *o, f32 x, f32 y, f32 z, f32 sx, f32 sy, f32 sz) {
    FollowW *w = (FollowW *) FX(o);

    w->x = x;
    w->y = y;
    w->z = z;
    w->model->scale[0] = sx;
    w->model->scale[1] = sy;
    w->model->scale[2] = sz;
}

/* "beat_lp": the LP. Carried by its monster until that dies, then it falls; touched, it is
 * found (flag 1): a spark, the LP's display (kind 0x165), scenario 0x1F4. */
s32 lp_proc(Obj *o, s16 msg, void *arg) {
    BeatItemW *w = BEATITEM(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "beat_lp";
            if (!swWorkTest(0)) {
                DPRINTF("lp_proc: BEAT_MISSION たっていません\n");
                return -1;
            }
            if (swWorkTest(1)) {
                DPRINTF("BEAT_LP たっています\n");
                return -1;
            }
            DPRINTF("lp_proc: セットします\n");
            eneSetup(o, arg);
            w->model = eneNewPol(o, (Gfx *) beatLPDl);
            if (w->model == NULL) {
                return -1;
            }
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 100, 50, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x66;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->atk != NULL) {
                frmDestroy((Frame *) w->atk);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                case 1:
                    if (acollAtkWasHit(w->atk) == 1) {
                        beatItemSdSet(o);
                        w->timer = 0;
                        swWorkSet(1);
                        w->state = 2;
                    }
                    break;
                case 2:
                    eneMakeEx(beatItemDispParent, ENE_DEMO_KIRAKIRA, 0, w->model->pos[0], w->model->pos[1], w->model->pos[2], 0,
                              NULL);
                    eneMakeEx(beatItemDispParent, ENE_BEAT_LP_DISP, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
                    beatLp = NULL;
                    DPRINTF("BEAT_LP げっとだぜ\n");
                    scRunStart2(0x1F4, 0, 0);
                    return -1;
            }
            if (w->state == 1 || w->state == 2 || w->state == 3 || w->state == 4) {
                w->rot[1] += 0.08726647f;
            }
            eneMove((EneObj *) w);
            if (w->state != 0) {
                eneFall(o, 0.4f);
                w->hit &= ~1;
                if (eneLand((EneObj *) w, 10.0f) == 1) {
                    w->hit |= 1;
                    w->vel[1] = 0.0f;
                }
            }
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The LP falls (its monster died). */
void beatLpDrop(Obj *o) {
    BEATITEM(o)->state = 1;
}

/* "lpDisp": the LP on the items' display. */
s32 beatLpDispProc(Obj *o, s16 msg, void *arg) {
    ItemDispW *w = ITEMDISP(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "lpDisp";
            w->pol = (PolFrame *) frmCreate((Frame *) beatItemDispView, gFrmKindPol);
            if (w->pol == NULL) {
                return -1;
            }
            w->pol->dl = (Gfx *) fadeSegToRam(beatItemDispDl, o->file);
            w->pol->dl2 = (Gfx *) beatLPDl;
            floadGetSetRam((void **) w->pol->segs, 0x84);
            /* Two spellings of pi: IDO shares a float constant only between literals spelt alike
             * (3.1415927f twice: one load; 3.1415927f and 3.14159265f, PI or 3.1415927: two;
             * measured), and the game loads it twice here (micDisp's pair shares one). */
            w->pol->rot[0] = PI;
            w->pol->rot[2] = 3.1415927f;
            w->pol->pos[0] = 134.0f;
            w->pol->pos[1] = 38.0f;
            w->pol->scale[2] = 0.01f;
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
            break;
        case 1:
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->pol->scale[0] += 0.015f;
                    w->pol->scale[1] += 0.015f;
                    w->pol->scale[2] += 0.015f;
                    w->pol->rot[2] -= 3.1415927f / (20 / gGameState.retracesPerFrame);
                    if (w->timer >= 20 / gGameState.retracesPerFrame) {
                        if (beatItemDispParent->child != NULL) {
                            w->pol->rot[1] = beatGetRotY(beatItemDispParent->child);
                        }
                        w->pol->rot[2] = 0.0f;
                        w->timer = 0;
                        w->state = 2;
                    }
                    break;
                case 2:
                    break;
            }
            w->timer++;
            w->pol->rot[1] += 0.08726647f;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "beat_mic": the mic. Touched, it is found (flag 2): a spark, the mic's display (kind 0x166),
 * scenario 0x1F3. */
s32 mic_proc(Obj *o, s16 msg, void *arg) {
    BeatItemW *w = BEATITEM(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "beat_mic";
            if (!swWorkTest(0)) {
                DPRINTF("mic_proc: BEAT_MISSION たっていません\n");
                return -1;
            }
            if (swWorkTest(2)) {
                DPRINTF("BEAT_MIC たっています\n");
                return -1;
            }
            eneSetup(o, arg);
            w->model = (PolFrame *) eneNewAnime(o, (u32) beatMicAnim);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("mic_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            DPRINTF("mic_proc: セットします\n");
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 100, 50, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x66;
            break;
        case 1:
            DPRINTF("mic_proc:  デストロイ\n");
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->atk != NULL) {
                frmDestroy((Frame *) w->atk);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (acollAtkWasHit(w->atk) == 1) {
                        beatItemSdSet(o);
                        w->model->flags &= ~2;
                        w->timer = 0;
                        swWorkSet(2);
                        w->state = 1;
                    }
                    break;
                case 1:
                    eneMakeEx(beatItemDispParent, ENE_DEMO_KIRAKIRA, 0, w->model->pos[0], w->model->pos[1], w->model->pos[2], 0,
                              NULL);
                    eneMakeEx(beatItemDispParent, ENE_BEAT_MIC_DISP, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
                    DPRINTF("BEAT_MIC げっとだぜ\n");
                    scRunStart2(0x1F3, 0, 0);
                    return -1;
            }
            if (w->state == 0 || w->state == 1 || w->state == 2 || w->state == 3) {
                w->rot[1] += 0.08726647f;
            }
            eneMove((EneObj *) w);
            eneFall(o, 0.4f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "micDisp": the mic on the items' display (two animated models). */
s32 beatMicDispProc(Obj *o, s16 msg, void *arg) {
    AnimeDispW *w = ANIMEDISP(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "micDisp";
            w->anime[0] = (AnimeFrame *) frmCreate((Frame *) beatItemDispView, gFrmKindAnime);
            if (w->anime[0] == NULL) {
                return -1;
            }
            w->anime[0]->dl = (Gfx *) fadeSegToRam(beatItemDispDl, o->file);
            w->anime[0]->anim = beatMicAnim;
            floadGetSetRam((void **) w->anime[0]->segs, 0x84);
            w->anime[0]->rot[0] = 3.1415927f;
            w->anime[0]->rot[2] = 3.1415927f;
            w->anime[0]->pos[0] = 110.0f;
            w->anime[0]->pos[1] = 38.0f;
            w->anime[0]->scale[2] = 0.01f;
            w->anime[0]->scale[0] = w->anime[0]->scale[1] = w->anime[0]->scale[2];
            w->anime[1] = (AnimeFrame *) frmCreate((Frame *) beatItemDispView, gFrmKindAnime);
            if (w->anime[1] == NULL) {
                return -1;
            }
            w->anime[1]->dl = (Gfx *) fadeSegToRam(beatItemDispDl, o->file);
            w->anime[1]->anim = beatMicDispAnim;
            floadGetSetRam((void **) w->anime[1]->segs, 0x84);
            w->anime[1]->rot[0] = 3.1415927f;
            w->anime[1]->rot[2] = 3.1415927f;
            w->anime[1]->pos[0] = 110.0f;
            w->anime[1]->pos[1] = 38.0f;
            w->anime[1]->scale[2] = 0.01f;
            w->anime[1]->scale[0] = w->anime[1]->scale[1] = w->anime[1]->scale[2];
            break;
        case 1:
            /* One line: IDO schedules by line; on four lines the work pointer's "addiu s0" lands
             * before the test instead of in its delay slot (measured). */
            if (w->anime[0] != NULL) { frmDestroy(&w->anime[0]->f); }
            if (w->anime[1] != NULL) {
                frmDestroy(&w->anime[1]->f);
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->anime[0]->scale[0] += 0.015f;
                    w->anime[0]->scale[1] += 0.015f;
                    w->anime[0]->scale[2] += 0.015f;
                    w->anime[0]->rot[2] -= 3.1415927f / (20 / gGameState.retracesPerFrame);
                    w->anime[1]->scale[0] += 0.015f;
                    w->anime[1]->scale[1] += 0.015f;
                    w->anime[1]->scale[2] += 0.015f;
                    w->anime[1]->rot[2] -= 3.1415927f / (20 / gGameState.retracesPerFrame);
                    if (w->timer >= 20 / gGameState.retracesPerFrame) {
                        if (beatItemDispParent->child != NULL) {
                            w->anime[0]->rot[1] = beatGetRotY(beatItemDispParent->child);
                            w->anime[1]->rot[1] = w->anime[0]->rot[1];
                        }
                        w->anime[0]->rot[2] = 0.0f;
                        w->anime[1]->rot[2] = 0.0f;
                        w->timer = 0;
                        w->state = 2;
                    }
                    break;
                case 2:
                    break;
            }
            w->timer++;
            w->anime[0]->rot[1] += 0.08726647f;
            w->anime[0]->frame += 0.02;
            if (polAnimeLastFrame(w->anime[0]) <= w->anime[0]->frame) {
                w->anime[0]->frame -= polAnimeLastFrame(w->anime[0]);
            }
            w->anime[1]->rot[1] += 0.08726647f;
            w->anime[1]->frame += 0.02;
            if (polAnimeLastFrame(w->anime[1]) <= w->anime[1]->frame) {
                w->anime[1]->frame -= polAnimeLastFrame(w->anime[1]);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "hphone": the headphones. Touched, they are found (flag 3): a spark, their display (kind
 * 0x167), scenario 0x1F2. */
s32 headPhone_proc(Obj *o, s16 msg, void *arg) {
    BeatItemW *w = BEATITEM(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "hphone";
            if (!swWorkTest(0)) {
                DPRINTF("headPhone_proc: BEAT_MISSION たっていません\n");
                return -1;
            }
            if (swWorkTest(3)) {
                DPRINTF("BEAT_HPHONE たっています\n");
                return -1;
            }
            DPRINTF("headPhone_proc: セットします\n");
            eneSetup(o, arg);
            w->model = eneNewPol(o, (Gfx *) beatHeadphonesDl);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("personInit: Error: 地面にセット失敗\n");
                return -1;
            }
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 100, 50, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x66;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->atk != NULL) {
                frmDestroy((Frame *) w->atk);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (acollAtkWasHit(w->atk) == 1) {
                        beatItemSdSet(o);
                        w->model->flags &= ~2;
                        w->timer = 0;
                        swWorkSet(3);
                        w->state = 1;
                    }
                    break;
                case 1:
                    eneMakeEx(beatItemDispParent, ENE_DEMO_KIRAKIRA, 0, w->model->pos[0], w->model->pos[1], w->model->pos[2], 0,
                              NULL);
                    eneMakeEx(beatItemDispParent, ENE_BEAT_HP_DISP, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
                    DPRINTF("BEAT_HPHONE げっとだぜ\n");
                    scRunStart2(0x1F2, 0, 0);
                    return -1;
            }
            if (w->state == 0 || w->state == 1 || w->state == 2 || w->state == 3) {
                w->rot[1] += 0.08726647f;
            }
            eneMove((EneObj *) w);
            eneFall(o, 0.4f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "hpDisp": the headphones on the items' display. */
s32 beatHpDispProc(Obj *o, s16 msg, void *arg) {
    ItemDispW *w = ITEMDISP(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "hpDisp";
            w->pol = (PolFrame *) frmCreate((Frame *) beatItemDispView, gFrmKindPol);
            if (w->pol == NULL) {
                return -1;
            }
            w->pol->dl = (Gfx *) fadeSegToRam(beatItemDispDl, o->file);
            w->pol->dl2 = (Gfx *) beatHeadphonesDl;
            floadGetSetRam((void **) w->pol->segs, 0x84);
            w->pol->rot[0] = 3.1415927f;
            w->pol->pos[0] = 86.0f;
            w->pol->pos[1] = 38.0f;
            w->pol->scale[2] = 0.01f;
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
            break;
        case 1:
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->pol->scale[0] += 0.015f;
                    w->pol->scale[1] += 0.015f;
                    w->pol->scale[2] += 0.015f;
                    w->pol->rot[2] -= 3.1415927f / (20 / gGameState.retracesPerFrame);
                    if (w->timer >= 20 / gGameState.retracesPerFrame) {
                        if (beatItemDispParent->child != NULL) {
                            w->pol->rot[1] = beatGetRotY(beatItemDispParent->child);
                        }
                        w->pol->rot[2] = 0.0f;
                        w->timer = 0;
                        w->state = 2;
                    }
                    break;
                case 2:
                    break;
            }
            w->timer++;
            w->pol->rot[1] += 0.08726647f;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The LP rides on its monster: its place and its turn. */
void beatLpSetPos(Obj *o, f32 x, f32 y, f32 z, f32 ry) {
    BeatItemW *w = BEATITEM(o);

    w->x = x;
    w->y = y;
    w->z = z;
    w->rot[1] = ry;
}

/* An object's turn (an enemy's rot[1]): the displays turn with the first one shown. */
f32 beatGetRotY(Obj *o) {
    return ENE(o)->rot[1];
}

/* "itemSdSet": an item is found: the voice of the player who touched it, and the find's
 * jingle at the item. */
void beatItemSdSet(Obj *o) {
    BeatItemW *w = BEATITEM(o);
    Obj *pl;
    s32 pad[2];
    u16 se;

    pl = (Obj *) w->atk->hitPower;
    if (pl == NULL) {
        DPRINTF("&&&& itemSdSet  pl D がない\n");
        return;
    }
    switch (plGetChara(pl)) {
        case 0:
            DPRINTF("&&&& SD_BEAT1\n");
            se = 0x12;
            break;
        case 1:
            DPRINTF("&&&& SD_BEAT2\n");
            se = 0x13;
            break;
        case 2:
            DPRINTF("&&&& SD_BEAT3\n");
            se = 0x14;
            break;
        case 3:
            DPRINTF("&&&& SD_BEAT4\n");
            se = 0x15;
            break;
    }
    sndPlay(se);
    soundIncamCam(0x233, w->model->pos[0], w->model->pos[1], w->model->pos[2]);
}

/* "iD-Parent": the items' displays' parent task, which does nothing of its own. */
s32 beatItemDispParentProc(Obj *o, s16 msg, void *arg) {
    switch (msg) {
        case 0:
            break;
        case 1:
            break;
        case 2:
            o->name = "iD-Parent";
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Makes the items' displays' parent task. */
Obj *beatItemDispParentCreate(Obj *o) {
    return tskCreate(o, beatItemDispParentProc, o->file, NULL);
}
