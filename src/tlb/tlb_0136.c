#include "tlb.h"

/* File 136: the lizard man ("tokage", "toka hagu": the stray lizard man, who is there only at
 * night), a demo talker who gives the player a pass (tegata) when the player takes on his errand. */

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* The lizard man's work, at its task + 0x20: a demo talker's (PersonObj) up to 0x44. */
typedef struct TokageW {
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
    /* 0x48 */ Obj *pass; /* the pass (kind 0x1C3), made between it and the player */
    /* 0x4C */ Anim anim; /* its face (segment 3 of its model) */
    /* 0x5C */ u8 flags; /* 1: the sky's tunes change 300 ticks after the pass */
    /* 0x60 */ s32 timer; /* ticks in the state */
    /* 0x64 */ u8 tuneTimer;
} TokageW;

#define TOKAGE(obj) ((TokageW *) FX(obj))

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

void tokagePassGive(Obj *o, Obj *player);

extern u8 tokageAnim[]; /* the lizard man's animation */
AnimFrame tokageFaceAnim[] = {
    { 0x03002D40, 15 },
    { 0x03003160, 15 },
    { 0, 0 },
}; /* his face's texture animation */
u32 tokageAnimTurn = (u32) tokageAnim; /* his turn to the talker (tokageAnim), played backwards to turn away */
u32 tokageAnimTalk = 0x030003D4; /* his talking loop (0x030003D4) */

void tokageNop(void) {
}

/* The pass goes to the player (state 3). */
void tokagePassGive(Obj *o, Obj *player) {
    PassW *w = (PassW *) FX(o);

    w->state = 3;
    w->player = player;
    w->timer = 0;
}

/* The pass's sound ("eventTegataSdSet"). */
void tokageTegataSdSet(void) {
    DPRINTF("&&&& eventTegataSdSet  pl D がないn");
}

void tokageTegataSdClear(void) {
    DPRINTF("&&&& SD_G_CLEAR1n");
    DPRINTF("&&&& SD_E_CLEAR1n");
    DPRINTF("&&&& SD_S_CLEAR1n");
    DPRINTF("&&&& SD_Y_CLEAR1n");
}

/* "tokage": the lizard man. Talked to, he looks up (his talking animation), talks (his
 * scenario), and if the player takes on his errand (0x93) leaves a pass between them that sparks
 * and goes to the player (scenario 0x20), talks again, and the sky's tunes change. */
s32 tokageProc(Obj *o, s16 msg, void *arg) {
    TokageW *w = TOKAGE(o);
    u32 data;
    s32 pad;
    f32 x;
    f32 y;
    f32 z;
    Vec3f spark;

    switch (msg) {
        case 2:
            o->name = "tokage";
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) tokageAnim);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("obisu_run_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 100, 0, 0);
            demoTalkInit(o, data);
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, tokageFaceAnim, 0, &w->model->segs[3], eneGetFile(o, 0));
            gGameWork.tokageSet = 1;
            DPRINTF("とかげのおっさんセット\n");
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            demoTalkerFree(o);
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            gGameWork.tokageSet = 0;
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (demoTalkEndCheck(o)) {
                        w->model->anim = tokageAnimTurn;
                        w->model->frame = 0.0f;
                        demoCamZoom_CL(w->talker);
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (eneAnimPlayOnce(o, w->model, 0.3f)) {
                        scRunStart(w->data->talk, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        w->model->anim = tokageAnimTalk;
                        w->model->frame = 0.0f;
                        w->state = 2;
                    }
                    break;
                case 2:
                    fadeAnimStep(&w->anim);
                    if (eneAnimLoop(o, w->model, 0.3f) == 1 && !scIsRunning()) {
                        if (swWorkTest(0x93)) {
                            swWorkReset(0x93);
                            w->state = 4;
                        } else {
                            w->model->anim = tokageAnimTurn;
                            w->model->frame = polAnimeLastFrame(w->model);
                            w->state = 3;
                        }
                    }
                    break;
                case 3:
                    if (eneAnimPlayOnce(o, w->model, -0.3f) && !scIsRunning()) {
                        personTalkCamOff();
                        demoFinish(o);
                        w->state = 0;
                    }
                    break;
                case 4:
                    if (w->talker == NULL) {
                        return -1;
                    }
                    plGetModelPos(w->talker, &x, &y, &z);
                    w->pass = eneMakeEx(NULL, ENE_DEMO_TEGATA, 0, (w->model->pos[0] + x) / 2, w->model->pos[1],
                                        (w->model->pos[2] + z) / 2, 0, NULL);
                    w->timer = 0;
                    w->state = 5;
                    break;
                case 5:
                    if (w->timer == 200 / gRetracesPerFrame) {
                        demoGetEneModelPos(w->pass, &spark);
                        ptclSpawnSparksFall(spark.x, spark.y, spark.z);
                        eneMakeEx(NULL, ENE_DEMO_KIRAKIRA, 0, spark.x, spark.y, spark.z, 0, NULL);
                        tokagePassGive(w->pass, w->talker);
                        w->flags |= 1;
                    }
                    if (w->timer == 260 / gRetracesPerFrame) {
                        DPRINTF(">>>  おっさんがSD CALL します\n");
                        scRunStart(0x20, 0, 0);
                        w->model->anim = tokageAnimTurn;
                        w->model->frame = polAnimeLastFrame(w->model);
                        w->state = 6;
                    }
                    break;
                case 6:
                    if (!scIsRunning()) {
                        DPRINTF("tokageHowManyContact [%d]人目\n");
                        scRunStart(w->data->talk, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        w->timer = 0;
                        w->state = 3;
                    }
                    break;
            }
            if (w->flags & 1) {
                if (300 / gRetracesPerFrame == w->tuneTimer++) {
                    w->flags &= ~1;
                    camSunClearTunes();
                    camSunSetTunes4(0x84, 0x81, 0x82, 0x83);
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
            /* An int 2, as the pass's "/ 2"s: one constant to IDO, which then loads it at each use
             * (a 2.0f here let it keep the halves' 2.0 in f0). */
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 2, NULL);
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void tokageNop2(void) {
}

void tokageNop3(void) {
}

void tokageNop4(void) {
}

void tokageNop5(void) {
}

/* "toka hagu": the stray lizard man, there only at night (from 20:00 to 4:00); as "tokage", but
 * the pass's scenario 0x20 is the talker's, and the sky's tunes change for stage 0xC0 apart. */
s32 tokageHaguProc(Obj *o, s16 msg, void *arg) {
    TokageW *w = TOKAGE(o);
    u32 data;
    s32 pad;
    f32 x;
    f32 y;
    f32 z;
    Vec3f spark;

    switch (msg) {
        case 2:
            o->name = "toka hagu";
            if (gGameWork.time >= 0x140000 || gGameWork.time <= 0x40000) {
                DPRINTF(">>>>>>  おっさんおるで\n");
            } else {
                DPRINTF(">>>>>>  おっさんおらん\n");
                return -1;
            }
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) tokageAnim);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("obisu_run_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 100, 0, 0);
            demoTalkInit(o, data);
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, tokageFaceAnim, 0, &w->model->segs[3], eneGetFile(o, 0));
            gGameWork.tokageSet = 1;
            DPRINTF("はぐれとかげのおっさんセット\n");
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            demoTalkerFree(o);
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            gGameWork.tokageSet = 0;
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (demoTalkEndCheck(o)) {
                        w->model->anim = tokageAnimTurn;
                        w->model->frame = 0.0f;
                        demoCamZoom_CL(w->talker);
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (eneAnimPlayOnce(o, w->model, 0.3f)) {
                        scRunStart(w->data->talk, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        w->model->anim = tokageAnimTalk;
                        w->model->frame = 0.0f;
                        w->state = 2;
                    }
                    break;
                case 2:
                    fadeAnimStep(&w->anim);
                    if (eneAnimLoop(o, w->model, 0.3f) == 1 && !scIsRunning()) {
                        if (swWorkTest(0x93)) {
                            swWorkReset(0x93);
                            w->state = 4;
                        } else {
                            w->model->anim = tokageAnimTurn;
                            w->model->frame = polAnimeLastFrame(w->model);
                            w->state = 3;
                        }
                    }
                    break;
                case 3:
                    if (eneAnimPlayOnce(o, w->model, -0.3f) && !scIsRunning()) {
                        personTalkCamOff();
                        demoFinish(o);
                        w->state = 0;
                    }
                    break;
                case 4:
                    if (w->talker == NULL) {
                        return -1;
                    }
                    plGetModelPos(w->talker, &x, &y, &z);
                    w->pass = eneMakeEx(NULL, ENE_DEMO_TEGATA, 0, (w->model->pos[0] + x) / 2, w->model->pos[1],
                                        (w->model->pos[2] + z) / 2, 0, NULL);
                    w->timer = 0;
                    w->state = 5;
                    break;
                case 5:
                    if (w->timer == 200 / gRetracesPerFrame) {
                        demoGetEneModelPos(w->pass, &spark);
                        ptclSpawnSparksFall(spark.x, spark.y, spark.z);
                        eneMakeEx(NULL, ENE_DEMO_KIRAKIRA, 0, spark.x, spark.y, spark.z, 0, NULL);
                        tokagePassGive(w->pass, w->talker);
                        w->flags |= 1;
                    } else if (w->timer == 260 / gRetracesPerFrame) {
                        DPRINTF(">>>  おっさんがSD CALL します\n");
                        scRunStart(0x20, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        w->model->anim = tokageAnimTurn;
                        w->model->frame = polAnimeLastFrame(w->model);
                        w->state = 6;
                    }
                    break;
                case 6:
                    if (!scIsRunning()) {
                        scRunStart(w->data->talk, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        w->timer = 0;
                        w->state = 3;
                    }
                    break;
            }
            if (w->flags & 1) {
                if (300 / gRetracesPerFrame == w->tuneTimer++) {
                    w->flags &= ~1;
                    camSunClearTunes();
                    if (gGameWork.stage == 0xC0) {
                        camSunSetTunes5(0x518, 0x44, 0x45, 0x46, 0x47);
                    } else {
                        camSunSetTunes4(0x79, 0x76, 0x77, 0x78);
                    }
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
            /* An int 2, as in "tokage". */
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 2, NULL);
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
