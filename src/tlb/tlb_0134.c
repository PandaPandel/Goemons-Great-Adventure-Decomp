#include "tlb.h"

/* Fades out to black at speed (tlb_0126's tsuchiFadeOut, with its first call on the brace's
 * line). */
void kakurenboFadeOut(f32 speed) { fadeSetSpeed(speed);
    fadeSetColor(0, 0, 0);
    fadeSetTarget(255.0f);
}

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

/* The pass goes to the player (state 3). */
void kakurenboPassToPlayer(Obj *o, Obj *player) {
    PassW *w = (PassW *) FX(o);

    w->state = 3;
    w->player = player;
    w->timer = 0;
}

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* The pass's sound ("eventTegataSdSet"), its print all that is left (as file 130's). */
void kakurenboEventTegataSdSet(void) {
    DPRINTF("&&&& eventTegataSdSet  pl D がないn");
}

void kakurenboEventClearSd(void) {
    DPRINTF("&&&& SD_G_CLEAR1n");
    DPRINTF("&&&& SD_E_CLEAR1n");
    DPRINTF("&&&& SD_S_CLEAR1n");
    DPRINTF("&&&& SD_Y_CLEAR1n");
}

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* The mission manager's work ("d_Kakurenbo"), at its task + 0x20. */
typedef struct DemoKakurenboW {
    /* 0x00 */ u8 state;
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
    /* 0x34 */ u8 id; /* its placement's (eneSetup) */
    /* 0x38 */ s32 timer; /* ticks in the state */
    /* 0x3C */ Obj *disp; /* the mission's timer (kind 0x1C2) */
    /* 0x40 */ Obj *fuki; /* the speech bubble ("fukidashi") */
    /* 0x44 */ Obj *kodisp; /* the six children's faces ("ko_disp") */
    /* 0x48 */ Camera *cam;
    /* 0x4C */ u8 pad4C[4];
    /* 0x50 */ s8 partake; /* gGameWork.partake while the instructions show */
} DemoKakurenboW;
/* The mission's goal, an hour and a minute of the clock (gGameWork.goalHour and goalMin, words;
 * as s32 fields demoKakurenbo_proc's temps differ). */
#define GW_GOAL_HOUR (*(s32 *) gGameWork.goalHour)
#define GW_GOAL_MIN (*(s32 *) gGameWork.goalMin)

void okeClearBroken(void);
void kanbanClearBroken(void);
Obj *fukidashiCreate(Obj *obj, f32 x, f32 y, f32 z, f32 v);
s32 fukidashiIsGrown(Obj *o);
Obj *kodomoDispCreate(Obj *obj);
void kodomoDispSetFound(Obj *o, s32 i);

/* "d_Kakurenbo": the hide-and-seek mission's manager. Made only while the mission is on (0x13)
 * and the pass is not given (0xAC). The first time (0x1B) it hides the six children (each in
 * one of its places, at random), sets the goal an hour from now and shows the instructions
 * (scenario 0x25C, the bubble); then it makes the timer (kind 0x1C2) and the faces. When all six
 * are found (0x1E-0x23, scenario 0x21), at the time limit, or when a player dies (the mission's
 * 2), it ends and the game goes back to stage 0xF2 with 0x92 set. */
s32 demoKakurenbo_proc(Obj *o, s16 msg, void *arg) {
    DemoKakurenboW *w = (DemoKakurenboW *) FX(o);
    s32 r;
    s32 hour;
    CamView *view;
    s32 min;
    s32 pad[2];
    f32 pos[3];
    s32 i;
    s32 pad2[2];

    switch (msg) {
        case 2:
            o->name = "d_Kakurenbo";
            if (!swWorkTest(0x13)) {
                DPRINTF("demoKakurenbo: かくれんぼ開始されません\n");
                return -1;
            }
            if (swWorkTest(0xAC)) {
                DPRINTF("demoKakurenbo: かくれんぼは開始されません 手形もらってるで\n");
                return -1;
            }
            w->id = eneSetup(o, arg);
            if (!swWorkTest(0x1B)) {
                DPRINTF("demoKakurenbo_proc: かくれんぼインストから開始します\n");
                swWorkSet(0x1B);
                okeClearBroken();
                kanbanClearBroken();
                r = fadeRandRange(1, 100);
                DPRINTF("rand [%d]:", r);
                if (r < 30) {
                    gGameWork.hiddenId[0] = 1;
                } else {
                    gGameWork.hiddenId[0] = 2;
                }
                /* the game's own code has no "else" before each "if (r < 100)": the first place
                 * is always overwritten, so these four children hide in places 2 and 3 only */
                r = fadeRandRange(1, 150);
                DPRINTF("[%d]:", r);
                if (r < 50) {
                    gGameWork.hiddenId[1] = 1;
                }
                if (r < 100) {
                    gGameWork.hiddenId[1] = 2;
                } else {
                    gGameWork.hiddenId[1] = 3;
                }
                r = fadeRandRange(1, 150);
                DPRINTF("[%d]:", r);
                if (r < 50) {
                    gGameWork.hiddenId[2] = 1;
                }
                if (r < 100) {
                    gGameWork.hiddenId[2] = 2;
                } else {
                    gGameWork.hiddenId[2] = 3;
                }
                r = fadeRandRange(1, 150);
                DPRINTF("[%d]:", r);
                if (r < 50) {
                    gGameWork.hiddenId[3] = 1;
                }
                if (r < 100) {
                    gGameWork.hiddenId[3] = 2;
                } else {
                    gGameWork.hiddenId[3] = 3;
                }
                r = fadeRandRange(1, 150);
                DPRINTF("[%d]:", r);
                if (r < 50) {
                    gGameWork.hiddenId[4] = 1;
                }
                if (r < 100) {
                    gGameWork.hiddenId[4] = 2;
                } else {
                    gGameWork.hiddenId[4] = 3;
                }
                r = fadeRandRange(1, 150);
                DPRINTF("[%d]\n", r);
                if (r < 50) {
                    gGameWork.hiddenId[5] = 4;
                }
                if (r < 100) {
                    gGameWork.hiddenId[5] = 5;
                } else {
                    gGameWork.hiddenId[5] = 6;
                }
                DPRINTF(">>>>> red hidden id=[%d]\n", gGameWork.hiddenId[0]);
                DPRINTF(">>>>> black hidden id=[%d]\n", gGameWork.hiddenId[1]);
                DPRINTF(">>>>> blue hidden id=[%d]\n", gGameWork.hiddenId[2]);
                DPRINTF(">>>>> green hidden id=[%d]\n", gGameWork.hiddenId[3]);
                DPRINTF(">>>>> pink hidden id=[%d]\n", gGameWork.hiddenId[4]);
                DPRINTF(">>>>> yellow hidden id=[%d]\n", gGameWork.hiddenId[5]);
                eneMakeEx(NULL, ENE_KODOMO_RUNRUN, 0, 10.0f, 100.0f, 58.0f, 2, (u32 (*)(void *, void *, void *)) 0);
                eneMakeEx(NULL, ENE_KODOMO_RUNRUN_BLACK, 0, -6.0f, 100.0f, 10.0f, 2, (u32 (*)(void *, void *, void *)) 1);
                eneMakeEx(NULL, ENE_KODOMO_RUNRUN_BLUE, 0, 21.0f, 100.0f, 12.0f, 2, (u32 (*)(void *, void *, void *)) 2);
                eneMakeEx(NULL, ENE_KODOMO_RUNRUN_YELLOW, 0, 32.0f, 100.0f, 37.0f, 2, (u32 (*)(void *, void *, void *)) 3);
                eneMakeEx(NULL, ENE_KODOMO_RUNRUN_PINK, 0, -7.0f, 100.0f, 31.0f, 2, (u32 (*)(void *, void *, void *)) 4);
                eneMakeEx(NULL, ENE_KODOMO_RUNRUN_GREEN, 0, 13.0f, 100.0f, 32.0f, 2, (u32 (*)(void *, void *, void *)) 5);
                floadLoadSet(0xD9);
                camSunSetClockStop(1);
                hour = gGameWork.time / 0x10000;
                min = (gGameWork.time - (hour << 16)) / 0x444;
                GW_GOAL_HOUR = hour + 1;
                GW_GOAL_MIN = min;
                DPRINTF(">>>>>  目標(時間、分) [%d]時[%d]分\n", GW_GOAL_HOUR, GW_GOAL_MIN);
                camAiSetHold(1);
                w->cam = camAiGetCam();
                camSetEye(w->cam, 61.0f, 127.0f, 116.0f);
                camSetAt(w->cam, -24.0f, 103.0f, -18.0f);
                gGameWork.scenarioRunning = 2;
                w->state = 0;
            } else {
                DPRINTF("demoKakurenbo_proc: かくれんぼインストなし\n");
                w->state = 5;
            }
            gGameWork.mission = 1;
            DPRINTF("demoKakurenbo セット完了\n");
            break;
        case 1:
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (w->timer == 1) {
                        demoPlStandAll();
                    }
                    if (w->timer == 8) {
                        sndPlay(0xBA);
                        camSunSetClockStop(1);
                        gGameWork.savedDaySeconds = gGameWork.daySeconds;
                        gGameWork.daySeconds = 4800;
                        w->partake = gGameWork.partake;
                        gGameWork.partake = 1;
                        scRunStart(0x25C, 0, 0);
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (!scIsRunning()) {
                        swWorkSet(0x24);
                        camAiSetHold(0);
                        w->state = 2;
                    }
                    break;
                case 2:
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i) && !(gGameWork.players[i]->flags & 2)) {
                            tskSendMsg(gGameWork.players[i], 4, pos);
                            break;
                        }
                    }
                    w->fuki = fukidashiCreate(o, pos[0] + 20.0f, pos[1] + 25.0f, pos[2], 0.2f);
                    w->state = 3;
                    break;
                case 3:
                    if (fukidashiIsGrown(w->fuki)) {
                        sndPlay(0x2E3);
                        w->fuki = tskDestroy(w->fuki);
                        w->state = 5;
                    }
                    break;
                case 5:
                    camSunSetClockStop(0);
                    gGameWork.partake = w->partake;
                    demoPlReleaseAll();
                    w->disp = eneMakeEx(NULL, ENE_DEMO_TIMER, 0, 0.0f, 0.0f, 0.0f, 2,
                                        (u32 (*)(void *, void *, void *)) ((((s8) GW_GOAL_MIN << 8) & 0xFF00)
                                                                           | ((s8) GW_GOAL_HOUR & 0xFF)));
                    DPRINTF("demoKakurenbo: かくれんぼ出現\n");
                    w->kodisp = kodomoDispCreate(o);
                    w->state = 6;
                    break;
                case 6:
                    if (demoEneState2Check(w->disp)) {
                        DPRINTF("***kakurenboz  タイムオーバー\n");
                        demoPlStandAll();
                        w->state = 8;
                    }
                    if (swWorkTest(0x1E) && swWorkTest(0x1F) && swWorkTest(0x20) && swWorkTest(0x21)
                        && swWorkTest(0x22) && swWorkTest(0x23)) {
                        demoPlStandAll();
                        camAiSetHold(1);
                        fadeSetColor(0, 0, 0);
                        fadeSetLevel(128.0f);
                        gGameWork.partake = 1;
                        w->timer = 0;
                        w->state = 7;
                    }
                    if (swWorkTest(0x1E)) {
                        kodomoDispSetFound(w->kodisp, 0);
                    }
                    if (swWorkTest(0x1F)) {
                        kodomoDispSetFound(w->kodisp, 1);
                    }
                    if (swWorkTest(0x20)) {
                        kodomoDispSetFound(w->kodisp, 2);
                    }
                    if (swWorkTest(0x21)) {
                        kodomoDispSetFound(w->kodisp, 3);
                    }
                    if (swWorkTest(0x22)) {
                        kodomoDispSetFound(w->kodisp, 4);
                    }
                    if (swWorkTest(0x23)) {
                        kodomoDispSetFound(w->kodisp, 5);
                    }
                    break;
                case 7:
                    if (300 / gRetracesPerFrame < w->timer && !scIsRunning()) {
                        scRunStart(0x21, 0, 0);
                        w->state = 10;
                    }
                    break;
                case 8:
                    if (40 / gRetracesPerFrame < w->timer) {
                        gGameWork.scenarioRunning = 2;
                        sndPlay(10);
                        w->state = 10;
                    }
                    break;
                case 10:
                    if (!scIsRunning()) {
                        gGameWork.scenarioRunning = 2;
                        w->timer = 0;
                        w->state = 11;
                    }
                    break;
                case 11:
                    if (w->timer == 1) {
                        gGameWork.daySeconds = gGameWork.savedDaySeconds;
                        swWorkReset(0x1B);
                        kakurenboFadeOut(16 / gRetracesPerFrame);
                        swWorkReset(0x13);
                        swWorkReset(0x24);
                        if (w->disp != NULL) {
                            w->disp = tskDestroy(w->disp);
                        }
                        if (w->kodisp != NULL) {
                            w->kodisp = tskDestroy(w->kodisp);
                        }
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        view = camGetView(w->cam);
                        camSetSpeed(w->cam, 20 / gRetracesPerFrame);
                        camMoveAtY(w->cam, view->at[1] + 50.0f);
                        gGameWork.scenarioRunning = 2;
                        if (!swWorkTest(0x1E) || !swWorkTest(0x1F) || !swWorkTest(0x20) || !swWorkTest(0x21)
                            || !swWorkTest(0x22) || !swWorkTest(0x23)) {
                            sndPlay(0x45F);
                        }
                    }
                    if (w->timer == 20 / gRetracesPerFrame) {
                        gGameWork.scenarioRunning = 0;
                        gGameWork.mission = 0;
                        gGameWork.partake = 0;
                        swWorkSet(0x92);
                        stageGotoSpawn(gStageTask->parent, 0xF2, 0, -19.0f, 10.0f, 23.0f, 4.101523876f, 0xD7);
                    }
                    break;
                case 12:
                    break;
            }
            if (gGameWork.mission == 2 && w->state != 11) {
                DPRINTF("%%%%   event manager: PLAYER 死にました信号をうけとったぜ！\n");
                gGameWork.scenarioRunning = 2;
                sndPlay(10);
                demoPlStandAll();
                scRunEnd();
                w->timer = 0;
                w->state = 11;
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* A hiding child's work (red, black, blue, green, pink, yellow), at its task + 0x20: an enemy's
 * (EneObj) up to 0x34. */
typedef struct KakurenboW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09;
    /* 0x0A */ u8 hit; /* 1 on the ground */
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u32 ram; /* kodomoRamWords, in RAM */
    /* 0x38 */ MPolFrame *shadow;
    /* 0x3C */ u8 id; /* its placement's (eneSetup) */
} KakurenboW;

/* The red child's work: a hiding child's (KakurenboW) with a damage box before its shadow. */
typedef struct KakurenboRedW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09;
    /* 0x0A */ u8 hit; /* 1 on the ground */
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u32 ram; /* kodomoRamWords, in RAM */
    /* 0x38 */ u8 id; /* its placement's (eneSetup) */
    /* 0x3C */ ACollFrame *dmg; /* hit, it is found */
    /* 0x40 */ MPolFrame *shadow;
} KakurenboRedW;

/* acollDmgAddXYZ and acollAtkAddXYZ return their box (main declares them void). */
typedef ACollFrame *(*DmgAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height,
                                     s16 ox, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy,
                                     s16 oz, s32 a);

extern Gfx kodomoModelDl[];
extern u32 kodomoRamWords[5]; /* five words fadeSegToRam makes RAM addresses of */
extern u32 kodomoAnim;
extern u32 kodomoRedModel;

void kakurenboPlayerShout(Obj *obj);

/* "red": the child found by hitting him. */
s32 red_proc(Obj *o, s16 msg, void *arg) {
    KakurenboRedW *w = (KakurenboRedW *) FX(o);
    s32 pad[2];
    Obj *pl;
    f32 pos[3];

    switch (msg) {
        case 2:
            o->name = "red";
            if (!swWorkTest(0x13)) {
                return -1;
            }
            if (swWorkTest(0xAC)) {
                return -1;
            }
            if (swWorkTest(0x1E)) {
                return -1;
            }
            w->id = eneSetup(o, arg) & 0xFF;
            if (w->id != gGameWork.hiddenId[0]) {
                return -1;
            }
            w->model = eneNewAnime(o, kodomoRedModel);
            if (w->model == NULL) {
                return -1;
            }
            w->model->dl = (Gfx *) fadeSegToRam(kodomoModelDl, o->file);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("red_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            w->dmg = ((DmgAddXYZFunc) acollDmgAddXYZ)(ene_dmg_root_frame, &w->model->f, 100, 100, 0, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->ram = fadeSegToRam(kodomoRamWords, o->file);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (w->id != gGameWork.hiddenId[0]) {
                        return -1;
                    }
                    if (acollDmgWasHit(w->dmg) == 1) {
                        kakurenboPlayerShout(o);
                        eneMakeHitSpark(w->zone, w->x, w->y + 10.0f, w->z, 2, w->rot[1]);
                        w->dmg->flags &= ~4;
                        swWorkSet(0x1E);
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (eneAnimPlayOnce(o, w->model, 0.3f)) {
                        scRunStart(0x262, 0, 0);
                        w->model->anim = kodomoAnim;
                        w->model->frame = 0.0f;
                        w->state = 2;
                    }
                    break;
                case 2:
                    pl = (Obj *) w->dmg->hitPower;
                    if (pl != NULL) {
                        tskSendMsg(pl, 4, pos);
                        eneTurnToPoint(o, pos[0], pos[2], 18.0f);
                    }
                    eneAnimLoop(o, w->model, 0.3f);
                    break;
            }
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u32 kodomoBlackAnims; /* 0x08006DEC: the black child's animation once found */
extern u32 kodomoBlackModel;

/* "black": the child found by hitting him. */
s32 black_proc(Obj *o, s16 msg, void *arg) {
    KakurenboRedW *w = (KakurenboRedW *) FX(o);
    s32 pad[2];
    Obj *pl;
    f32 pos[3];

    switch (msg) {
        case 2:
            o->name = "black";
            if (!swWorkTest(0x13)) {
                DPRINTF("***  no KAKURENBO_MISSION\n");
                return -1;
            }
            if (swWorkTest(0xAC)) {
                DPRINTF("***  tegata\n");
                return -1;
            }
            if (swWorkTest(0x1F)) {
                DPRINTF("***  見つかっていた\n");
                return -1;
            }
            w->id = eneSetup(o, arg) & 0xFF;
            if (w->id != gGameWork.hiddenId[1]) {
                return -1;
            }
            w->model = eneNewAnime(o, kodomoBlackModel);
            if (w->model == NULL) {
                return -1;
            }
            w->model->dl = (Gfx *) fadeSegToRam(kodomoModelDl, o->file);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("black_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            w->dmg = ((DmgAddXYZFunc) acollDmgAddXYZ)(ene_dmg_root_frame, &w->model->f, 100, 100, 0, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->ram = fadeSegToRam(kodomoRamWords, o->file);
            w->rot[1] = 4.33195591f;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (w->id != gGameWork.hiddenId[1]) {
                        return -1;
                    }
                    if (acollDmgWasHit(w->dmg) == 1) {
                        kakurenboPlayerShout(o);
                        eneMakeHitSpark(w->zone, w->x, w->y + 10.0f, w->z, 2, w->rot[1]);
                        w->dmg->flags &= ~4;
                        swWorkSet(0x1F);
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (eneAnimPlayOnce(o, w->model, 0.3f)) {
                        scRunStart(0x261, 0, 0);
                        w->model->anim = kodomoBlackAnims;
                        w->model->frame = 0.0f;
                        w->state = 2;
                    }
                    break;
                case 2:
                    pl = (Obj *) w->dmg->hitPower;
                    if (pl != NULL) {
                        tskSendMsg(pl, 4, pos);
                        eneTurnToPoint(o, pos[0], pos[2], 18.0f);
                    }
                    eneAnimLoop(o, w->model, 0.3f);
                    break;
            }
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u32 kodomoBlueAnims;
extern u32 kodomoBlueModel;

/* "blue": the child behind a signboard. */
s32 blue_proc(Obj *o, s16 msg, void *arg) {
    KakurenboRedW *w = (KakurenboRedW *) FX(o);
    s32 pad[2];
    Obj *pl;
    f32 pos[3];

    switch (msg) {
        case 2:
            o->name = "blue";
            if (!swWorkTest(0x13)) {
                return -1;
            }
            if (swWorkTest(0xAC)) {
                return -1;
            }
            if (swWorkTest(0x20)) {
                return -1;
            }
            w->id = eneSetup(o, arg) & 0xFF;
            if (w->id != gGameWork.hiddenId[2]) {
                return -1;
            }
            w->model = eneNewAnime(o, kodomoBlueModel);
            if (w->model == NULL) {
                return -1;
            }
            w->model->dl = (Gfx *) fadeSegToRam(kodomoModelDl, o->file);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("blue_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            w->dmg = ((DmgAddXYZFunc) acollDmgAddXYZ)(ene_dmg_root_frame, &w->model->f, 100, 100, 0, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->ram = fadeSegToRam(kodomoRamWords, o->file);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (w->id != gGameWork.hiddenId[2]) {
                        return -1;
                    }
                    if (swWorkTest(0x4B)) {
                        swWorkReset(0x4B);
                        swWorkSet(0x20);
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (eneAnimPlayOnce(o, w->model, 0.3f)) {
                        scRunStart(0x25F, 0, 0);
                        w->model->anim = kodomoBlueAnims;
                        w->model->frame = 0.0f;
                        w->state = 2;
                        swWorkSet(0x20);
                    }
                    break;
                case 2:
                    pl = (Obj *) w->dmg->hitPower;
                    if (pl != NULL) {
                        tskSendMsg(pl, 4, pos);
                        eneTurnToPoint(o, pos[0], pos[2], 18.0f);
                    }
                    eneAnimLoop(o, w->model, 0.3f);
                    break;
            }
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u32 kodomoGreenAnims;
extern u32 kodomoGreenModel;

/* "green": the child found by hitting him. */
s32 green_proc(Obj *o, s16 msg, void *arg) {
    KakurenboRedW *w = (KakurenboRedW *) FX(o);
    s32 pad[2];
    Obj *pl;
    f32 pos[3];

    switch (msg) {
        case 2:
            o->name = "green";
            if (!swWorkTest(0x13)) {
                return -1;
            }
            if (swWorkTest(0xAC)) {
                return -1;
            }
            if (swWorkTest(0x21)) {
                return -1;
            }
            w->id = eneSetup(o, arg) & 0xFF;
            if (w->id != gGameWork.hiddenId[3]) {
                return -1;
            }
            w->model = eneNewAnime(o, kodomoGreenModel);
            if (w->model == NULL) {
                return -1;
            }
            w->model->dl = (Gfx *) fadeSegToRam(kodomoModelDl, o->file);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("green_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            w->dmg = ((DmgAddXYZFunc) acollDmgAddXYZ)(ene_dmg_root_frame, &w->model->f, 100, 100, 0, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->ram = fadeSegToRam(kodomoRamWords, o->file);
            w->rot[1] = 5.332115173f;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (w->id != gGameWork.hiddenId[3]) {
                        return -1;
                    }
                    if (acollDmgWasHit(w->dmg) == 1) {
                        swWorkSet(0x21);
                        kakurenboPlayerShout(o);
                        eneMakeHitSpark(w->zone, w->x, w->y + 10.0f, w->z, 2, w->rot[1]);
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (eneAnimPlayOnce(o, w->model, 0.3f)) {
                        scRunStart(0x260, 0, 0);
                        w->model->anim = kodomoGreenAnims;
                        w->model->frame = 0.0f;
                        w->state = 2;
                    }
                    break;
                case 2:
                    pl = (Obj *) w->dmg->hitPower;
                    if (pl != NULL) {
                        tskSendMsg(pl, 4, pos);
                        eneTurnToPoint(o, pos[0], pos[2], 18.0f);
                    }
                    eneAnimLoop(o, w->model, 0.3f);
                    break;
            }
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern Gfx kodomoModelDl[];
extern u32 kodomoRamWords[5]; /* five words fadeSegToRam makes RAM addresses of */
extern u32 kodomoPinkAnims;
extern u32 kodomoPinkModel;

/* "pink": the child hiding in a tub. */
s32 pink_proc(Obj *o, s16 msg, void *arg) {
    KakurenboW *w = (KakurenboW *) FX(o);
    s32 pad[2];
    Obj *pl;
    f32 pos[3];

    switch (msg) {
        case 2:
            o->name = "pink";
            if (!swWorkTest(0x13)) {
                return -1;
            }
            if (swWorkTest(0xAC)) {
                return -1;
            }
            if (swWorkTest(0x22)) {
                return -1;
            }
            w->id = eneSetup(o, arg) & 0xFF;
            if (w->id != gGameWork.hiddenId[4]) {
                DPRINTF("-->> PINK DESTROY id=[%d]\n", w->id);
                return -1;
            }
            w->model = eneNewAnime(o, kodomoPinkModel);
            if (w->model == NULL) {
                return -1;
            }
            w->model->dl = (Gfx *) fadeSegToRam(kodomoModelDl, o->file);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("pink_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            w->ram = fadeSegToRam(kodomoRamWords, o->file);
            DPRINTF(">>>>>>>>  ピンクセット id=[%d]\n", w->id);
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
            switch (w->state) {
                case 0:
                    if (w->id != gGameWork.hiddenId[4]) {
                        return -1;
                    }
                    if (swWorkTest(0x4A)) {
                        DPRINTF(">>>>>>>>>> PINK みつかったメッセージ受けとった！！\n");
                        swWorkReset(0x4A);
                        swWorkSet(0x22);
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (eneAnimPlayOnce(o, w->model, 0.3f)) {
                        scRunStart(0x25E, 0, 0);
                        w->model->anim = kodomoPinkAnims;
                        w->model->frame = 0.0f;
                        w->state = 2;
                    }
                    break;
                case 2:
                    pl = plCheck();
                    if (pl != NULL) {
                        tskSendMsg(pl, 4, pos);
                        eneTurnToPoint(o, pos[0], pos[2], 18.0f);
                    }
                    eneAnimLoop(o, w->model, 0.3f);
                    break;
            }
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u32 kodomoYellowAnims;
extern u32 kodomoYellowModel;

/* "yellow": the child hiding in a tub. */
s32 yellow_proc(Obj *o, s16 msg, void *arg) {
    KakurenboW *w = (KakurenboW *) FX(o);
    s32 pad[2];
    Obj *pl;
    f32 pos[3];

    switch (msg) {
        case 2:
            o->name = "yellow";
            if (!swWorkTest(0x13)) {
                return -1;
            }
            if (swWorkTest(0xAC)) {
                return -1;
            }
            if (swWorkTest(0x23)) {
                return -1;
            }
            w->id = eneSetup(o, arg) & 0xFF;
            if (w->id != gGameWork.hiddenId[5]) {
                return -1;
            }
            w->model = eneNewAnime(o, kodomoYellowModel);
            if (w->model == NULL) {
                return -1;
            }
            w->model->dl = (Gfx *) fadeSegToRam(kodomoModelDl, o->file);
            w->shadow = shadowCreate();
            w->ram = fadeSegToRam(kodomoRamWords, o->file);
            DPRINTF(">>>>>>>>  イエローセット id=[%d]\n", w->id);
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
            switch (w->state) {
                case 0:
                    if (w->id != gGameWork.hiddenId[5]) {
                        return -1;
                    }
                    if (swWorkTest(0x4C)) {
                        DPRINTF(">>>>>>>>>> YELLOW みつかったメッセージ受けとった！！\n");
                        swWorkReset(0x4C);
                        swWorkSet(0x23);
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (eneAnimPlayOnce(o, w->model, 0.3f)) {
                        scRunStart(0x25D, 0, 0);
                        w->model->anim = kodomoYellowAnims;
                        w->model->frame = 0.0f;
                        w->state = 2;
                    }
                    break;
                case 2:
                    pl = plCheck();
                    if (pl != NULL) {
                        tskSendMsg(pl, 4, pos);
                        eneTurnToPoint(o, pos[0], pos[2], 18.0f);
                    }
                    eneAnimLoop(o, w->model, 0.3f);
                    break;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "oke_body", a tub the children hide in, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct OkeW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09;
    /* 0x0A */ u8 hit;
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u32 ram; /* kodomoRamWords or, broken, kodomoRamWords2, in RAM */
    /* 0x38 */ AnimeFrame *lid; /* its lid ("futa") */
    /* 0x3C */ ACollFrame *dmg;
    /* 0x40 */ MPolFrame *shadow;
    /* 0x44 */ u8 id; /* its placement's (eneSetup): its bit in gGameWork.okeBroken */
    /* 0x48 */ s32 timer;
} OkeW;

extern u8 okeBodyAnim[]; /* the tub's animation */
extern u8 okeBodyLidAnim[]; /* the lid's */
extern u32 kodomoRamWords2[5];

u8 okeIsBroken(u8 n);
void okeSetBroken(u8 n);

/* "oke_body": a tub. Hit, it breaks (its bit in gGameWork.okeBroken set) and its lid flies off;
 * the ones yellow and pink hide in (placements gGameWork.hiddenId[5] and [4]) also set flags 0x4C
 * and 0x4A, which find them. */
s32 oke_body_proc(Obj *o, s16 msg, void *arg) {
    OkeW *w = (OkeW *) FX(o);
    s32 last;

    switch (msg) {
        case 2:
            o->name = "oke_body";
            w->id = eneSetup(o, arg) & 0xFF;
            w->model = eneNewAnime(o, (u32) okeBodyAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->model->dl = (Gfx *) fadeSegToRam(kodomoModelDl, o->file);
            w->dmg = ((DmgAddXYZFunc) acollDmgAddXYZ)(ene_dmg_root_frame, &w->model->f, 100, 100, 0, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->shadow = shadowCreate();
            w->ram = fadeSegToRam(kodomoRamWords, o->file);
            if (swWorkTest(0x13)) {
                DPRINTF(">>>>>>>  かくれんぼミッション発生 id=[%d]\n", w->id);
                if (okeIsBroken(w->id)) {
                    DPRINTF(">>>>>>>  オケ既に壊されている id=[%d]\n", w->id);
                    w->state = 2;
                    break;
                }
                w->state = 3;
            } else {
                w->state = 0;
            }
            DPRINTF(">>>>>>>  フタ発生 id=[%d]\n", w->id);
            w->lid = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->lid == NULL) {
                return -1;
            }
            w->lid->pos[0] = w->model->pos[0];
            w->lid->pos[1] = w->model->pos[1];
            w->lid->pos[2] = w->model->pos[2];
            w->lid->scale[2] = 0.1f;
            w->lid->scale[0] = w->lid->scale[1] = w->lid->scale[2];
            w->lid->anim = okeBodyLidAnim;
            w->lid->flags |= 0x20;
            floadGetSetRam((void **) w->lid->segs, 0xDB);
            DPRINTF(">>>>>>>  オケセット id=[%d]\n", w->id);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            if (w->lid != NULL) {
                frmDestroy(&w->lid->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (acollDmgWasHit(w->dmg) == 1) {
                        eneMakeHitSpark(w->zone, w->x, w->y + 10.0f, w->z, 2, w->rot[1]);
                        SoundLocation2(0x2C0, gGameWork.view, w->model->pos[0], w->model->pos[1], w->model->pos[2],
                                       gGameWork.view->zNear + 100.0f, gGameWork.view->zFar, gGameWork.view->fovy);
                        w->ram = fadeSegToRam(kodomoRamWords2, o->file);
                        DPRINTF(">>>>>>>>>    おけ破壊 id=[%d]\n", w->id);
                        w->state = 1;
                    }
                    break;
                case 1:
                    w->lid->pos[1] -= 0.3;
                    w->lid->rot[2] += 0.1;
                    if (eneAnimPlayOnce(o, w->model, 0.8f)) {
                        w->lid = (AnimeFrame *) frmDestroy(&w->lid->f);
                        w->state = 8;
                    }
                    break;
                case 3:
                    if (w->id == gGameWork.hiddenId[5]) {
                        DPRINTF(">>>>>>>>>    イエロー用おけ  アニメなし id=[%d]\n", w->id);
                        w->state = 4;
                    } else if (w->id == gGameWork.hiddenId[4]) {
                        DPRINTF(">>>>>>>>>    ピンク用おけ id=[%d]\n", w->id);
                        w->state = 6;
                    } else {
                        DPRINTF(">>>>>>>>>    それ以外はノーマル id=[%d]\n", w->id);
                        w->state = 7;
                    }
                    break;
                case 4:
                    if (acollDmgWasHit(w->dmg) == 1) {
                        eneMakeHitSpark(w->zone, w->x, w->y + 10.0f, w->z, 2, w->rot[1]);
                        SoundLocation2(0x2C0, gGameWork.view, w->model->pos[0], w->model->pos[1], w->model->pos[2],
                                       gGameWork.view->zNear + 100.0f, gGameWork.view->zFar, gGameWork.view->fovy);
                        swWorkSet(0x4C);
                        okeSetBroken(w->id);
                        DPRINTF(">>>>>>>>>    イエロー用おけ破壊  id=[%d]\n", w->id);
                        w->state = 5;
                    }
                    break;
                case 5:
                    last = polAnimeLastFrame(w->lid);
                    w->lid->frame += 0.8;
                    DPRINTF("time [%f] max[%d]\n", w->lid->frame, last);
                    if (last < w->lid->frame) {
                        w->lid->frame = last;
                        w->state = 8;
                    }
                    break;
                case 6:
                    if (acollDmgWasHit(w->dmg) == 1) {
                        eneMakeHitSpark(w->zone, w->x, w->y + 10.0f, w->z, 2, w->rot[1]);
                        SoundLocation2(0x2C0, gGameWork.view, w->model->pos[0], w->model->pos[1], w->model->pos[2],
                                       gGameWork.view->zNear + 100.0f, gGameWork.view->zFar, gGameWork.view->fovy);
                        swWorkSet(0x4A);
                        okeSetBroken(w->id);
                        DPRINTF(">>>>>>>>>    ピンク用おけ破壊  id=[%d]\n", w->id);
                        w->state = 1;
                    }
                    break;
                case 7:
                    if (acollDmgWasHit(w->dmg) == 1) {
                        eneMakeHitSpark(w->zone, w->x, w->y + 10.0f, w->z, 2, w->rot[1]);
                        SoundLocation2(0x2C0, gGameWork.view, w->model->pos[0], w->model->pos[1], w->model->pos[2],
                                       gGameWork.view->zNear + 100.0f, gGameWork.view->zFar, gGameWork.view->fovy);
                        okeSetBroken(w->id);
                        w->ram = fadeSegToRam(kodomoRamWords2, o->file);
                        DPRINTF(">>>>>>>>>    おけ破壊 id=[%d]\n", w->id);
                        w->state = 1;
                    }
                    break;
                case 2:
                    w->model->frame = polAnimeLastFrame(w->model);
                    w->ram = fadeSegToRam(kodomoRamWords2, o->file);
                    w->state = 8;
                    break;
                case 8:
                    break;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 2.0f, NULL);
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Whether bit n of gGameWork.okeBroken is set. */
u8 okeIsBroken(u8 n) {
    return gGameWork.okeBroken & (u8) (1 << (u8) (n % 8));
}

/* Sets bit n of gGameWork.okeBroken. */
void okeSetBroken(u8 n) {
    gGameWork.okeBroken |= (u8) (1 << (u8) (n % 8));
}

void kakurenboUnusedStub1(void) {
}

void okeClearBroken(void) {
    gGameWork.okeBroken = 0;
}

/* The red child's work ("red", kodomoRed_proc, who sets the hide-and-seek mission), at its task
 * + 0x20: a demo talker's (PersonObj) up to 0x44, as file 130's jizou. */
typedef struct KodomoRedW {
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
    /* 0x4C */ u8 flags; /* 1: the sky's tunes change 300 ticks after the pass */
    /* 0x50 */ Obj *pass; /* the pass (kind 0x1C3), made between it and the player */
    /* 0x54 */ u8 tuneTimer;
    /* 0x58 */ s32 timer; /* ticks in the state */
} KodomoRedW;

/* "red": the child who sets the hide-and-seek mission. Talked to, he talks (scenario 0x25A); if
 * the player takes it on (0x94) the screen fades and the game goes to stage 0x70; back from it
 * (0x92) he talks again, and if the player has found them all (0x93) he gives the pass between
 * them (kind 0x1C3, sparks and kind 0x1C1), the pass goes to the player (scenarios 0x20, 0x25B),
 * and the sky's tunes change. Gone while the mission is on and not yet done. */
s32 kodomoRed_proc(Obj *o, s16 msg, void *arg) {
    s32 pad1;
    KodomoRedW *w = (KodomoRedW *) FX(o);
    u32 data;
    CamView *view;
    s32 pad;
    f32 pos[3];
    s32 i;
    f32 pos2[3];
    Obj *player;
    s32 pad2;
    f32 x;
    f32 y;
    f32 z;
    Vec3f spark;

    switch (msg) {
        case 2:
            o->name = "red";
            if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
                DPRINTF("kodomo*_proc: KAKURENBO_MISSION フラグたってる\n");
                return -1;
            }
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, kodomoAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->model->dl = (Gfx *) fadeSegToRam(kodomoModelDl, o->file);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("kodomoRed_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 100, 0, 0);
            demoTalkInit(o, data);
            w->shadow = shadowCreate();
            if (swWorkTest(0x92)) {
                DPRINTF(">>>> イベントから帰って来た場合\n");
                DPRINTF("******* gw.escape[%d]\n");
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
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i) && !(gGameWork.players[i]->flags & 2)) {
                            tskSendMsg(gGameWork.players[i], 4, pos);
                            eneTurnToPoint(o, pos[0], pos[2], 10.0f);
                            break;
                        }
                    }
                    if (demoTalkEndCheck(o)) {
                        demoCamZoom_CL(w->talker);
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (w->talker != NULL) {
                        tskSendMsg(w->talker, 4, pos2);
                        if (eneTurnToPoint(o, pos2[0], pos2[2], 10.0f) == 1) {
                            w->state = 2;
                        }
                    }
                    break;
                case 2:
                    scRunStart(0x25A, plGetIndex(w->talker) + 1, plGetChara(w->talker));
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
                        kakurenboFadeOut(16 / gRetracesPerFrame);
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        view = camGetView(w->cam);
                        camSetSpeed(w->cam, 20 / gRetracesPerFrame);
                        camMoveAtY(w->cam, view->at[1] + 50.0f);
                        gGameWork.scenarioRunning = 2;
                    }
                    if (w->timer == 20 / gRetracesPerFrame) {
                        gGameWork.scenarioRunning = 0;
                        camAiSetHold(0);
                        stageGotoSpawn(gStageTask->parent, 0x70, 0, 4.0f, 100.0f, 73.0f, 3.141592741f, 0xD7);
                    }
                    break;
                case 5:
                    if (w->timer == 1) {
                        demoPlStandAll();
                    }
                    if (w->timer == 8) {
                        if (!swWorkTest(0x1E) || !swWorkTest(0x1F) || !swWorkTest(0x20) || !swWorkTest(0x21)
                            || !swWorkTest(0x22) || !swWorkTest(0x23)) {
                            camSunClearTunes();
                            camSunSetTunes4(0x89, 0x86, 0x87, 0x88);
                        }
                        demoCamZoom_CL(plCheck());
                        scRunStart(0x25A, 0, 0);
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
                        kakurenboPassToPlayer(w->pass, plCheck());
                        w->flags |= 1;
                    }
                    if (w->timer == 260 / gRetracesPerFrame) {
                        scRunStart(0x20, 0, 0);
                        w->state = 9;
                    }
                    break;
                case 9:
                    if (!scIsRunning()) {
                        scRunStart(0x25B, 0, 0);
                        w->state = 10;
                    }
                    break;
                case 10:
                    if (!scIsRunning()) {
                        personTalkCamOff();
                        demoPlReleaseAll();
                        w->state = 0;
                    }
                    break;
            }
            if (w->flags & 1) {
                if (300 / gRetracesPerFrame == w->tuneTimer++) {
                    w->flags &= ~1;
                    camSunClearTunes();
                    camSunSetTunes4(0x89, 0x86, 0x87, 0x88);
                }
            }
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.6f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

extern u32 kodomoBlackAnims; /* the children's animation they are made with (kodomoBlackMakeAnim) */

/* "black": the black-clad child, a person; gone while the hide-and-seek mission (flag 0x13) is on
 * and not yet done (flag 0xAC). */
s32 kodomoBlackProc(Obj *o, s16 msg, void *arg) {
    s32 pad[3]; /* the game's frame is 0x30 with swWorkTest's spilled address at 0x1C: three unused
                 * words */

    switch (msg) {
        case 2:
            o->name = "black";
            if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
                DPRINTF("kodomo*_proc: KAKURENBO_MISSION フラグたってる\n");
                return -1;
            }
            if (personInitMainTable(o, &kodomoBlackAnims, arg) == -1) {
                return -1;
            }
            break;
        case 1:
            personDelete(o);
            break;
        case 8:
            return personTalk(o, arg);
        case 0:
            personAct(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u32 kodomoBlueAnims; /* the children's animation they are made with (kodomoMakeAnim) */
extern u32 kodomoBlueAnims; /* the children's animation they are made with (kodomoMakeAnim) */

s32 kodomoBlue_proc(Obj *o, s16 msg, void *arg) {
    s32 pad[3]; /* the game's frame is 0x30 with swWorkTest's spilled address at 0x1C: three unused
                 * words */

    switch (msg) {
        case 2:
            o->name = "blue";
            if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
                DPRINTF("kodomo*_proc: KAKURENBO_MISSION フラグたってる\n");
                return -1;
            }
            if (personInitMainTable(o, &kodomoBlueAnims, arg) == -1) {
                return -1;
            }
            break;
        case 1:
            personDelete(o);
            break;
        case 8:
            return personTalk(o, arg);
        case 0:
            personAct(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u32 kodomoYellowAnims; /* the children's animation they are made with (kodomoMakeAnim) */
extern u32 kodomoYellowAnims; /* the children's animation they are made with (kodomoMakeAnim) */

s32 kodomoYellow_proc(Obj *o, s16 msg, void *arg) {
    s32 pad[3]; /* the game's frame is 0x30 with swWorkTest's spilled address at 0x1C: three unused
                 * words */

    switch (msg) {
        case 2:
            o->name = "yellow";
            if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
                DPRINTF("kodomo*_proc: KAKURENBO_MISSION フラグたってる\n");
                return -1;
            }
            if (personInitMainTable(o, &kodomoYellowAnims, arg) == -1) {
                return -1;
            }
            break;
        case 1:
            personDelete(o);
            break;
        case 8:
            return personTalk(o, arg);
        case 0:
            personAct(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u32 kodomoPinkAnims; /* the children's animation they are made with (kodomoMakeAnim) */
extern u32 kodomoPinkAnims; /* the children's animation they are made with (kodomoMakeAnim) */

s32 kodomoPink_proc(Obj *o, s16 msg, void *arg) {
    s32 pad[3]; /* the game's frame is 0x30 with swWorkTest's spilled address at 0x1C: three unused
                 * words */

    switch (msg) {
        case 2:
            o->name = "pink";
            if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
                DPRINTF("kodomo*_proc: KAKURENBO_MISSION フラグたってる\n");
                return -1;
            }
            if (personInitMainTable(o, &kodomoPinkAnims, arg) == -1) {
                return -1;
            }
            break;
        case 1:
            personDelete(o);
            break;
        case 8:
            return personTalk(o, arg);
        case 0:
            personAct(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u32 kodomoGreenAnims; /* the children's animation they are made with (kodomoMakeAnim) */
extern u32 kodomoGreenAnims; /* the children's animation they are made with (kodomoMakeAnim) */

s32 kodomoGreen_proc(Obj *o, s16 msg, void *arg) {
    s32 pad[3]; /* the game's frame is 0x30 with swWorkTest's spilled address at 0x1C: three unused
                 * words */

    switch (msg) {
        case 2:
            o->name = "green";
            if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
                DPRINTF("kodomo*_proc: KAKURENBO_MISSION フラグたってる\n");
                return -1;
            }
            if (personInitMainTable(o, &kodomoGreenAnims, arg) == -1) {
                return -1;
            }
            break;
        case 1:
            personDelete(o);
            break;
        case 8:
            return personTalk(o, arg);
        case 0:
            personAct(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "fukidashi"'s (fukidashi_proc) argument: only what its starter sets. */
typedef struct FukidashiArg {
    /* 0x00 */ u8 pad00[0x8];
    /* 0x08 */ f32 pos[3];
    /* 0x14 */ f32 scale; /* the scale */
    /* 0x18 */ u8 pad18[0x8];
} FukidashiArg;

/* "fukidashi", the speech bubble: a back and a bubble that grows in three steps. */
typedef struct FukidashiW {
    /* 0x00 */ PolFrame *back;
    /* 0x04 */ PolFrame *pol; /* this step's bubble */
    /* 0x08 */ f32 pos[3];
    /* 0x14 */ f32 scale;
    /* 0x18 */ u8 step; /* 0-2, then 3 */
    /* 0x19 */ u8 state; /* 0 draw the next step, 1 wait, 2 done */
    /* 0x1A */ u8 pad1A[2];
    /* 0x1C */ s32 timer;
} FukidashiW;

extern Gfx *fukidashiSteps[3]; /* the bubble's three steps (fukidashiStepsDl0, fukidashiStepsDl1, fukidashiStepsDl2) */
extern Gfx fukidashiBubbleBackDl[];             /* the bubble's back */

s32 fukidashi_proc(Obj *obj, s16 msg, void *arg) {
    FukidashiW *w = (FukidashiW *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "fukidashi";
            w->scale = ((FukidashiArg *) arg)->scale;
            w->pos[0] = ((FukidashiArg *) arg)->pos[0];
            w->pos[1] = ((FukidashiArg *) arg)->pos[1];
            w->pos[2] = ((FukidashiArg *) arg)->pos[2];
            w->step = 0;
            w->pol = NULL;
            break;
        case 1:
            if (w->back != NULL) {
                frmDestroy(&w->back->f);
            }
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (w->pol != NULL) {
                        w->pol = (PolFrame *) frmDestroy(&w->pol->f);
                    }
                    w->pol = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
                    if (w->pol == NULL) {
                        return -1;
                    }
                    w->pol->dl2 = fukidashiSteps[w->step];
                    w->pol->flags |= 0x27;
                    floadGetSetRam((void **) w->pol->segs, 0xD9);
                    w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2] = w->scale;
                    w->pol->pos[0] = w->pos[0];
                    w->pol->pos[1] = w->pos[1];
                    w->pol->pos[2] = w->pos[2];
                    if (w->back == NULL) {
                        w->back = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
                        if (w->back == NULL) {
                            return -1;
                        }
                        w->back->dl2 = fukidashiBubbleBackDl;
                        w->back->flags |= 0x27;
                        floadGetSetRam((void **) w->back->segs, 0xD9);
                        w->back->scale[0] = w->back->scale[1] = w->back->scale[2] = w->scale;
                        w->back->pos[0] = w->pos[0];
                        w->back->pos[1] = w->pos[1];
                        w->back->pos[2] = w->pos[2];
                    }
                    w->step++;
                    w->timer = 0;
                    w->state = 1;
                    break;
                case 1:
                    if (100 / gRetracesPerFrame < w->timer) {
                        if (w->step >= 3) {
                            w->state = 2;
                        } else {
                            if (w->pol != NULL) {
                                w->pol = (PolFrame *) frmDestroy(&w->pol->f);
                            }
                            w->state = 0;
                        }
                    }
                    break;
                case 2:
                    break;
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "fukidashi" (speech bubble) at x, y, z under obj. */
Obj *fukidashiCreate(Obj *obj, f32 x, f32 y, f32 z, f32 v) {
    FukidashiArg arg;
    s32 pad[2];

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = v;
    return tskCreate(obj, fukidashi_proc, obj->file, &arg);
}

/* Whether the bubble has finished growing. */
s32 fukidashiIsGrown(Obj *o) {
    if (((FukidashiW *) FX(o))->state == 2) {
        return 1;
    }
    return 0;
}

/* "ko_disp", the six children's faces on screen (its proc kodomoDisp_proc). */
typedef struct KoDispW {
    /* 0x00 */ CamView *cam;
    /* 0x04 */ AnimeFrame *kids[6];
    /* 0x1C */ u8 flags[6]; /* 1 found, 2 its face is shown */
    /* 0x22 */ u8 state;
} KoDispW;

extern Gfx koDispKidDl[];
extern u32 koDispFaceAnims[6]; /* each face's animation */
extern s32 koDispFaceSets[6]; /* each face's file set */
extern u8 koDispFoundFlags[];   /* each child's "found" flag */

s32 kodomoDisp_proc(Obj *o, s16 msg, void *arg) {
    KoDispW *w = (KoDispW *) FX(o);
    s32 pad;
    s32 i;
    s32 j;
    s32 k;

    switch (msg) {
        case 2:
            o->name = "ko_disp";
            floadLoadSet(0x8F);
            floadLoadSet(0x90);
            floadLoadSet(0x91);
            floadLoadSet(0x92);
            floadLoadSet(0x93);
            floadLoadSet(0x94);
            w->cam = (CamView *) frmCreate(gFrmLayers[10], gFrmKindCamera);
            if (w->cam == NULL) {
                DPRINTF(">>>>>  kodomoDisp_proc cam つくれません\n");
                return -1;
            }
            w->cam->ortho |= 1;
            w->cam->zNear = -200.0f;
            w->cam->zFar = 200.0f;
            DPRINTF("kodomoDisp_proc: セットＯＫ\n");
            break;
        case 1:
            for (i = 0; i < 6; i++) {
                if (w->kids[i] != NULL) {
                    frmDestroy(&w->kids[i]->f);
                }
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    DPRINTF("KODOMO_DISP_SETUP\n");
                    for (j = 0; j < 6; j++) {
                        w->kids[j] = (AnimeFrame *) frmCreate(&w->cam->f, gFrmKindAnime);
                        if (w->kids[j] == NULL) {
                            DPRINTF(">>>>>  kodomoDisp_proc kodomo[%d] つくれません\n", j);
                            return -1;
                        }
                        w->kids[j]->dl = (Gfx *) fadeSegToRam(koDispKidDl, o->file);
                        w->kids[j]->anim = koDispFaceAnims[j];
                        w->kids[j]->flags |= 0x20;
                        floadGetSetRam((void **) w->kids[j]->segs, koDispFaceSets[j]);
                        w->kids[j]->rot[0] = 3.141592741f;
                        w->kids[j]->pos[0] = j * 12 + 78;
                        w->kids[j]->pos[1] = 36.0f;
                        w->kids[j]->scale[2] = 0.15f;
                        w->kids[j]->scale[0] = w->kids[j]->scale[1] = w->kids[j]->scale[2];
                        if (swWorkTest(koDispFoundFlags[j])) {
                            w->kids[j]->frame = polAnimeLastFrame(w->kids[j]);
                            w->flags[j] |= 2;
                        }
                    }
                    w->state = 1;
                    break;
                case 1:
                    for (k = 0; k < 6; k++) {
                        if ((w->flags[k] & 1) && !(w->flags[k] & 2)) {
                            w->kids[k]->frame += 0.3;
                            if (polAnimeLastFrame(w->kids[k]) < w->kids[k]->frame) {
                                w->kids[k]->frame = polAnimeLastFrame(w->kids[k]);
                                w->flags[k] |= 2;
                            }
                        }
                    }
                    break;
                case 2:
                    break;
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

Obj *kodomoDispCreate(Obj *obj) {
    return tskCreate(obj, kodomoDisp_proc, obj->file, NULL);
}

void kodomoDispSetFound(Obj *o, s32 i) {
    ((KoDispW *) FX(o))->flags[i] |= 1;
}

/* "kanban", a signboard the children hide behind, at its task + 0x20: an enemy's (EneObj) up
 * to 0x34. */
typedef struct KanbanW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09;
    /* 0x0A */ u8 hit; /* 1 on the ground */
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u32 ram; /* kodomoRamWords or, broken, kodomoRamWords2, in RAM */
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ u8 id; /* its placement's (eneSetup): its bit in gGameWork.kanbanBroken */
} KanbanW;

extern u8 kanbanAnim[]; /* the signboard's animation */
extern u32 kodomoRamWords2[5];

u8 kanbanIsBroken(u8 n);
void kanbanSetBroken(u8 n);

/* "kanban": a signboard. Hit, it breaks (its bit in gGameWork.kanbanBroken set); the one blue hides
 * behind (placement gGameWork.hiddenId[2]) also sets flag 0x4B, which finds him. */
s32 kanban_proc(Obj *o, s16 msg, void *arg) {
    KanbanW *w = (KanbanW *) FX(o);

    switch (msg) {
        case 2:
            o->name = "kanban";
            w->id = eneSetup(o, arg) & 0xFF;
            w->model = eneNewAnime(o, (u32) kanbanAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->dmg = ((DmgAddXYZFunc) acollDmgAddXYZ)(ene_dmg_root_frame, &w->model->f, 100, 100, 0, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->ram = fadeSegToRam(kodomoRamWords, o->file);
            if (swWorkTest(0x13)) {
                DPRINTF(">>>>>>>  看板かくれんぼミッション中発生 id=[%d]\n", w->id);
                if (kanbanIsBroken(w->id)) {
                    DPRINTF(">>>>>>>  看板既に壊されている id=[%d]\n", w->id);
                    w->state = 6;
                } else {
                    w->state = 3;
                }
            } else {
                w->state = 0;
            }
            DPRINTF("*** kanban セットＯＫ\n");
            break;
        case 1:
            /* on one line: IDO schedules by line, and only so does "addiu s0" fill the
             * branch's delay slot as in the game */
            if (w->model != NULL) { frmDestroy(&w->model->f); }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (acollDmgWasHit(w->dmg) == 1) {
                        eneMakeHitSpark(w->zone, w->x, w->y + 10.0f, w->z, 2, w->rot[1]);
                        if (w->dmg->hitKind != 0) {
                            switch (w->dmg->hitKind) {
                                case 4:
                                case 5:
                                case 6:
                                case 23:
                                    eneSetSd2(o, 0x220, 0, 0);
                                    break;
                                case 18:
                                case 19:
                                case 20:
                                    eneSetSd2(o, 0x22D, 0, 0);
                                    break;
                                case 1:
                                default:
                                    eneSetSd2(o, 0x22C, 0, 0);
                                    break;
                            }
                        }
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (eneAnimPlayOnce(o, w->model, 0.5f)) {
                        w->state = 2;
                    }
                    if (w->model->frame == 17.0f) {
                        soundIncamCam(0x25A, w->model->pos[0], w->model->pos[1], w->model->pos[2]);
                    }
                    break;
                case 2:
                    break;
                case 3:
                    if (w->id == gGameWork.hiddenId[2]) {
                        DPRINTF(">>>>>>>>>    ブルー用看板  アニメなし id=[%d]\n", w->id);
                        w->state = 4;
                    } else {
                        DPRINTF(">>>>>>>>>    それ以外はノーマル id=[%d]\n", w->id);
                        w->state = 5;
                    }
                    break;
                case 4:
                    if (acollDmgWasHit(w->dmg) == 1) {
                        eneMakeHitSpark(w->zone, w->x, w->y + 10.0f, w->z, 2, w->rot[1]);
                        if (w->dmg->hitKind != 0) {
                            switch (w->dmg->hitKind) {
                                case 4:
                                case 5:
                                case 6:
                                case 23:
                                    eneSetSd2(o, 0x220, 0, 0);
                                    break;
                                case 18:
                                case 19:
                                case 20:
                                    eneSetSd2(o, 0x22D, 0, 0);
                                    break;
                                case 1:
                                default:
                                    eneSetSd2(o, 0x22C, 0, 0);
                                    break;
                            }
                        }
                        swWorkSet(0x4B);
                        kanbanSetBroken(w->id);
                        w->ram = fadeSegToRam(kodomoRamWords2, o->file);
                        DPRINTF(">>>>>>>>>    ブルー用看板倒壊  id=[%d]\n", w->id);
                        w->state = 1;
                    }
                    break;
                case 5:
                    if (acollDmgWasHit(w->dmg) == 1) {
                        eneMakeHitSpark(w->zone, w->x, w->y + 10.0f, w->z, 2, w->rot[1]);
                        if (w->dmg->hitKind != 0) {
                            switch (w->dmg->hitKind) {
                                case 4:
                                case 5:
                                case 6:
                                case 23:
                                    eneSetSd2(o, 0x220, 0, 0);
                                    break;
                                case 18:
                                case 19:
                                case 20:
                                    eneSetSd2(o, 0x22D, 0, 0);
                                    break;
                                case 1:
                                default:
                                    eneSetSd2(o, 0x22C, 0, 0);
                                    break;
                            }
                        }
                        kanbanSetBroken(w->id);
                        w->ram = fadeSegToRam(kodomoRamWords2, o->file);
                        DPRINTF(">>>>>>>>>    看板破壊 id=[%d]\n", w->id);
                        w->state = 1;
                    }
                    break;
                case 6:
                    w->model->frame = polAnimeLastFrame(w->model);
                    w->ram = fadeSegToRam(kodomoRamWords2, o->file);
                    w->state = 2;
                    break;
            }
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Whether bit n of gGameWork.kanbanBroken is set. */
u8 kanbanIsBroken(u8 n) {
    return gGameWork.kanbanBroken & (u8) (1 << (u8) (n % 8));
}

/* Sets bit n of gGameWork.kanbanBroken. */
void kanbanSetBroken(u8 n) {
    gGameWork.kanbanBroken |= (u8) (1 << (u8) (n % 8));
}

void kakurenboUnusedStub2(void) {
}

void kanbanClearBroken(void) {
    gGameWork.kanbanBroken = 0;
}

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* A running child's work ("runrun", the kid who runs off), at its task + 0x20: an enemy's
 * (EneObj) up to 0x38. */
typedef struct RunrunW {
    /* 0x00 */ u8 state; /* 0 waiting for flag 0x24, 1 turning away, 2 running off */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09;
    /* 0x0A */ u8 hit; /* 1 on the ground */
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u8 pad34[4];
    /* 0x38 */ MPolFrame *shadow;
    /* 0x3C */ Vec3f home; /* where it was set */
    /* 0x48 */ f32 angle; /* the way it faces (radians); it runs off the other way */
} RunrunW;

extern u32 kodomoAnim; /* the children's animation they are made with (kodomoMakeAnim) */
extern u32 kodomoAnimRun; /* the runaway child's run (0x030005EC) */

/* "runrun": a child who waits for flag 0x24, then turns round and runs 90 units off and goes. */
s32 kodomoRunrunProc(Obj *o, s16 msg, void *arg) {
    RunrunW *w = (RunrunW *) FX(o);
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "runrun";
            if (!swWorkTest(0x13)) {
                return -1;
            }
            if (swWorkTest(0xAC)) {
                return -1;
            }
            eneSetup(o, arg);
            w->model = eneNewAnime(o, kodomoAnim);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("yellow_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            w->home.x = w->model->pos[0];
            w->home.y = w->model->pos[1];
            w->home.z = w->model->pos[2];
            w->model->rot[1] = w->angle = 0.5235987902f;
            DPRINTF(">>>>>>>>  逃げ子供セット red kodomo\n");
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
            switch (w->state) {
                case 0:
                    if (swWorkTest(0x24)) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (eneTurnToAngle(o, (w->angle + 3.141592741f) * 180.0f / 3.141592741f, 8.0f)) {
                        w->model->anim = kodomoAnimRun;
                        w->model->frame = 0.0f;
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (eneMoveToPos3D(o, mathSin(w->angle + 3.141592741f) * 90.0f + w->home.x, w->home.y,
                                       mathCos(w->angle + 3.141592741f) * 90.0f + w->home.z, 3.0f)) {
                        DPRINTF("いね kodomo\n");
                        return -1;
                    }
                    break;
            }
            eneAnimLoop(o, w->model, 0.3f);
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u32 kodomoBlackAnims; /* the children's animation they are made with (kodomoBlackMakeAnim) */
extern u32 kodomoBlackRunAnim; /* the runaway child's run (0x030005EC) */

s32 kodomoRunrunBlackProc(Obj *o, s16 msg, void *arg) {
    RunrunW *w = (RunrunW *) FX(o);
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "runrun";
            if (!swWorkTest(0x13)) {
                return -1;
            }
            if (swWorkTest(0xAC)) {
                return -1;
            }
            eneSetup(o, arg);
            w->model = eneNewAnime(o, kodomoBlackAnims);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("yellow_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            w->home.x = w->model->pos[0];
            w->home.y = w->model->pos[1];
            w->home.z = w->model->pos[2];
            w->model->rot[1] = w->angle = 0.87266463f;
            DPRINTF(">>>>>>>>  逃げ子供セット black kodomo\n");
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
            switch (w->state) {
                case 0:
                    if (swWorkTest(0x24)) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (eneTurnToAngle(o, (w->angle + 3.141592741f) * 180.0f / 3.141592741f, 8.0f)) {
                        w->model->anim = kodomoBlackRunAnim;
                        w->model->frame = 0.0f;
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (eneMoveToPos3D(o, mathSin(w->angle + 3.141592741f) * 90.0f + w->home.x, w->home.y,
                                       mathCos(w->angle + 3.141592741f) * 90.0f + w->home.z, 3.0f)) {
                        DPRINTF("いね kodomo\n");
                        return -1;
                    }
                    break;
            }
            eneAnimLoop(o, w->model, 0.3f);
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u32 kodomoBlueAnims; /* the children's animation they are made with (kodomoMakeAnim) */
extern u32 kodomoBlueRunAnim; /* the runaway child's run (0x030005EC) */

s32 kodomoRunrunBlueProc(Obj *o, s16 msg, void *arg) {
    RunrunW *w = (RunrunW *) FX(o);
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "runrun";
            if (!swWorkTest(0x13)) {
                return -1;
            }
            if (swWorkTest(0xAC)) {
                return -1;
            }
            eneSetup(o, arg);
            w->model = eneNewAnime(o, kodomoBlueAnims);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("yellow_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            w->home.x = w->model->pos[0];
            w->home.y = w->model->pos[1];
            w->home.z = w->model->pos[2];
            w->model->rot[1] = w->angle = 2.268928f;
            DPRINTF(">>>>>>>>  逃げ子供セット blue kodomo\n");
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
            switch (w->state) {
                case 0:
                    if (swWorkTest(0x24)) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (eneTurnToAngle(o, (w->angle + 3.141592741f) * 180.0f / 3.141592741f, 8.0f)) {
                        w->model->anim = kodomoBlueRunAnim;
                        w->model->frame = 0.0f;
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (eneMoveToPos3D(o, mathSin(w->angle + 3.141592741f) * 90.0f + w->home.x, w->home.y,
                                       mathCos(w->angle + 3.141592741f) * 90.0f + w->home.z, 3.0f)) {
                        DPRINTF("いね kodomo\n");
                        return -1;
                    }
                    break;
            }
            eneAnimLoop(o, w->model, 0.3f);
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u32 kodomoYellowAnims; /* the children's animation they are made with (kodomoMakeAnim) */
extern u32 kodomoYellowRunAnim; /* the runaway child's run (0x030005EC) */

s32 kodomoRunrunYellowProc(Obj *o, s16 msg, void *arg) {
    RunrunW *w = (RunrunW *) FX(o);
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "runrun";
            if (!swWorkTest(0x13)) {
                return -1;
            }
            if (swWorkTest(0xAC)) {
                return -1;
            }
            eneSetup(o, arg);
            w->model = eneNewAnime(o, kodomoYellowAnims);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("yellow_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            w->home.x = w->model->pos[0];
            w->home.y = w->model->pos[1];
            w->home.z = w->model->pos[2];
            w->model->rot[1] = w->angle = 4.363323f;
            DPRINTF(">>>>>>>>  逃げ子供セット yellow kodomo\n");
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
            switch (w->state) {
                case 0:
                    if (swWorkTest(0x24)) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (eneTurnToAngle(o, (w->angle + 3.141592741f) * 180.0f / 3.141592741f, 8.0f)) {
                        w->model->anim = kodomoYellowRunAnim;
                        w->model->frame = 0.0f;
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (eneMoveToPos3D(o, mathSin(w->angle + 3.141592741f) * 90.0f + w->home.x, w->home.y,
                                       mathCos(w->angle + 3.141592741f) * 90.0f + w->home.z, 3.0f)) {
                        DPRINTF("いね kodomo\n");
                        return -1;
                    }
                    break;
            }
            eneAnimLoop(o, w->model, 0.3f);
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u32 kodomoPinkAnims; /* the children's animation they are made with (kodomoMakeAnim) */
extern u32 kodomoPinkRunAnim; /* the runaway child's run (0x030005EC) */

s32 kodomoRunrunPinkProc(Obj *o, s16 msg, void *arg) {
    RunrunW *w = (RunrunW *) FX(o);
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "runrun";
            if (!swWorkTest(0x13)) {
                return -1;
            }
            if (swWorkTest(0xAC)) {
                return -1;
            }
            eneSetup(o, arg);
            w->model = eneNewAnime(o, kodomoPinkAnims);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("yellow_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            w->home.x = w->model->pos[0];
            w->home.y = w->model->pos[1];
            w->home.z = w->model->pos[2];
            w->model->rot[1] = w->angle = 1.9198622f;
            DPRINTF(">>>>>>>>  逃げ子供セット pink kodomo\n");
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
            switch (w->state) {
                case 0:
                    if (swWorkTest(0x24)) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (eneTurnToAngle(o, (w->angle + 3.141592741f) * 180.0f / 3.141592741f, 8.0f)) {
                        w->model->anim = kodomoPinkRunAnim;
                        w->model->frame = 0.0f;
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (eneMoveToPos3D(o, mathSin(w->angle + 3.141592741f) * 90.0f + w->home.x, w->home.y,
                                       mathCos(w->angle + 3.141592741f) * 90.0f + w->home.z, 3.0f)) {
                        DPRINTF("いね kodomo\n");
                        return -1;
                    }
                    break;
            }
            eneAnimLoop(o, w->model, 0.3f);
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u32 kodomoGreenAnims; /* the children's animation they are made with (kodomoMakeAnim) */
extern u32 kodomoGreenRunAnim; /* the runaway child's run (0x030005EC) */

/* The file's .data, in address order (its externs above stay as declarations). */
extern u8 fukidashiStepsDl2[];
extern u8 fukidashiStepsDl1[];
extern u8 kodomoMakeAnim[];
extern u8 fukidashiStepsDl0[];
extern u8 kodomoYellowAnim[];
extern u8 kodomoGreenAnim[];
extern u8 kodomoPinkAnim[];
extern u8 kodomoYellowRunAnimHeader[];
extern u8 kodomoPinkRunAnimHeader[];
extern u8 kodomoRedModelAnim[];
extern u8 kodomoBlueModelAnim[];
/* the black child's (file set 0x90): kodomoMakeAnim's and kodomoRedModelAnim's addresses in another file */
extern u8 kodomoBlackMakeAnim[];
extern u8 kodomoBlackModelAnim[];
Gfx koDispKidDl[] = {
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
};
Gfx kodomoModelDl[] = {
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR),
    gsSPSetGeometryMode(G_CULL_BACK | G_LIGHTING),
    gsSPEndDisplayList(),
};
u32 kodomoAnim = (u32) kodomoMakeAnim;
u32 kodomoAnimRun = 0x030005EC; /* the runaway child's run (0x030005EC) */
u32 kodomoRedModel = (u32) kodomoRedModelAnim;
u32 kodomoBlackAnims = (u32) kodomoBlackMakeAnim; /* 0x08006DEC: the black child's animation once found */
u32 kodomoBlackRunAnim = 0x030005EC; /* the runaway child's run (0x030005EC) */
u32 kodomoBlackModel = (u32) kodomoBlackModelAnim;
u32 kodomoBlueAnims = 0x03000344;
u32 kodomoBlueRunAnim = 0x0300075C; /* the runaway child's run (0x030005EC) */
u32 kodomoBlueModel = (u32) kodomoBlueModelAnim;
u32 kodomoYellowAnims = (u32) kodomoYellowAnim;
u32 kodomoYellowRunAnim = (u32) kodomoYellowRunAnimHeader; /* the runaway child's run (0x030005EC) */
u32 kodomoYellowModel = 0x03000A18;
u32 kodomoPinkAnims = (u32) kodomoPinkAnim;
u32 kodomoPinkRunAnim = (u32) kodomoPinkRunAnimHeader; /* the runaway child's run (0x030005EC) */
u32 kodomoPinkModel = 0x03000A84;
u32 kodomoGreenAnims = (u32) kodomoGreenAnim;
u32 kodomoGreenRunAnim = 0x0300064C; /* the runaway child's run (0x030005EC) */
u32 kodomoGreenModel = 0x030009FC;
u32 kodomoRamWords[5] = { 0x00000000, 0x00000000, 0x05080000, 0x00000000, 0x00000000 }; /* five words fadeSegToRam makes RAM addresses of */
u32 kodomoRamWords2[5] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 };
Gfx *fukidashiSteps[3] = { fukidashiStepsDl0, fukidashiStepsDl1, fukidashiStepsDl2 }; /* the bubble's three steps (fukidashiStepsDl0, fukidashiStepsDl1, fukidashiStepsDl2) */
u32 koDispFaceAnims[6] = { (u32) kodomoRedModelAnim, (u32) kodomoBlackModelAnim, (u32) kodomoBlueModelAnim, 0x030009FC, 0x03000A84, 0x03000A18 }; /* each face's animation */
s32 koDispFaceSets[6] = { 143, 144, 145, 148, 147, 146 }; /* each face's file set */
u8 koDispFoundFlags[] = { 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };   /* each child's "found" flag */

s32 kodomoRunrunGreenProc(Obj *o, s16 msg, void *arg) {
    RunrunW *w = (RunrunW *) FX(o);
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "runrun";
            if (!swWorkTest(0x13)) {
                return -1;
            }
            if (swWorkTest(0xAC)) {
                return -1;
            }
            eneSetup(o, arg);
            w->model = eneNewAnime(o, kodomoGreenAnims);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("yellow_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            w->home.x = w->model->pos[0];
            w->home.y = w->model->pos[1];
            w->home.z = w->model->pos[2];
            w->model->rot[1] = w->angle = 5.759587f;
            DPRINTF(">>>>>>>>  逃げ子供セット green kodomo\n");
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
            switch (w->state) {
                case 0:
                    if (swWorkTest(0x24)) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (eneTurnToAngle(o, (w->angle + 3.141592741f) * 180.0f / 3.141592741f, 8.0f)) {
                        w->model->anim = kodomoGreenRunAnim;
                        w->model->frame = 0.0f;
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (eneMoveToPos3D(o, mathSin(w->angle + 3.141592741f) * 90.0f + w->home.x, w->home.y,
                                       mathCos(w->angle + 3.141592741f) * 90.0f + w->home.z, 3.0f)) {
                        DPRINTF("いね kodomo\n");
                        return -1;
                    }
                    break;
            }
            eneAnimLoop(o, w->model, 0.3f);
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The jizou's work ("jizou", the stone jizou who stands in the way), at its task + 0x20: a demo
 * talker's (PersonObj) up to 0x44, as file 130's. */
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
    /* 0x34 */ PersonData *data; /* talk: its kind (0 or 1) */
    /* 0x38 */ ACollFrame *box; /* its talk box */
    /* 0x3C */ u8 flags3C;
    /* 0x3D */ u8 count;
    /* 0x3E */ u8 pad3E[2];
    /* 0x40 */ Obj *talker; /* the player talking to it */
    /* 0x44 */ MPolFrame *shadow;
    /* 0x48 */ s32 timer;
} JizouW;

extern u8 kakurenboJizouAnim[]; /* the jizou's animation */

/* "jizou": a jizou who stands in the way while the hide-and-seek mission (flag 0x13) is on and
 * not yet done (0xAC). Talked to, he talks (scenario 0x265). */
s32 kakurenboJizou_proc(Obj *o, s16 msg, void *arg) {
    s32 pad;
    JizouW *w = (JizouW *) FX(o);
    u32 data;
    s32 pad2;

    switch (msg) {
        case 2:
            o->name = "jizou";
            if (!swWorkTest(0x13) || swWorkTest(0xAC)) {
                DPRINTF("****  jizou: stopper stop....\n");
                return -1;
            }
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) kakurenboJizouAnim);
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
            switch (w->data->talk) {
                case 0:
                    w->model->scale[0] = 0.4f;
                    w->model->scale[2] = 0.6f;
                    w->model->scale[1] = 0.2f;
                    w->box->radius = 150;
                    break;
                case 1:
                    w->model->scale[2] = 0.2;
                    w->model->scale[0] = w->model->scale[2];
                    w->model->scale[1] = 0.2f;
                    w->box->radius = 150;
                    break;
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
                        gGameWork.partake = 1;
                        personTalkCamAt(o, w->talker, 80);
                        w->state = 1;
                    }
                    break;
                case 1:
                    scRunStart(0x265, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    w->state = 2;
                    break;
                case 2:
                    if (!scIsRunning()) {
                        gGameWork.partake = 0;
                        demoFinish(o);
                        camAiSetHold(0);
                        /* the two stopper prints' places are a guess: only their order is
                         * the game's (its .rodata) */
                        DPRINTF("$$$  exit jizou stopper\n");
                        DPRINTF("$$$  connect jizou stopper\n");
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

/* The player who last hit the hiding child obj's damage box (the box's hitPower, the other
 * box's owner) cries out where its model is: sound 0x22E for character 1, else 0x22C. */
void kakurenboPlayerShout(Obj *obj) {
    s32 pad[2];
    Obj *pl = (Obj *) ((KakurenboRedW *) FX(obj))->dmg->hitPower;
    s32 chara;
    f32 pos[3];

    if (pl != NULL) {
        chara = plGetChara(pl);
        plGetModelPos(pl, &pos[0], &pos[1], &pos[2]);
        switch (chara) {
            case 0:
            case 2:
            case 3:
                soundIncamCam(0x22C, pos[0], pos[1], pos[2]);
                break;
            case 1:
                soundIncamCam(0x22E, pos[0], pos[1], pos[2]);
                break;
        }
    }
}

s32 kakurenboIdleProc(Obj *obj, s16 msg, void *arg) {
    return tskDefaultProc(obj, msg, arg);
}
