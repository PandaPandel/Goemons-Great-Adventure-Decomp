#include "tlb.h"

/* File 129: Obisumaru's race. "obisu", the false Ebisumaru who, talked to, starts the chase
 * ("demoObisuRun"); "obisurun", Obisumaru running his course (jumps, course changes, puffs of
 * dust); "obisuAtari", a platform riding with him; "obisuEffect", a glow that follows him. */

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* "obisurun"'s work, at its task + 0x20: an enemy's (EneObj) to 0x34, then its own. */
typedef struct ObisuRunW {
    /* 0x00 */ u8 state; /* 0 waiting, 1 to the start line, 2 the race, 3 on its course, 4 stopped,
                          * 5 hit */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit; /* 1 on the ground */
    /* 0x0B */ s8 hp;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *dmg;
    /* 0x38 */ MPolFrame *shadow;
    /* 0x3C */ Obj *atari; /* the platform riding with it (kind 0x171) */
    /* 0x40 */ Obj *glow; /* its glow (kind 0x172, "obisuEffect") */
    /* 0x44 */ u8 flags; /* 1 no fall, 2 off its course, 4 animation once, 8 grey puffs, 0x10 no
                          * facing its way */
    /* 0x48 */ s16 *course; /* the course's next command */
    /* 0x4C */ s32 jumpTime;
    /* 0x50 */ s32 step; /* ticks in the command */
    /* 0x54 */ f32 savedVel[3]; /* the state when it was hit */
    /* 0x60 */ u8 savedFlags;
    /* 0x64 */ u32 savedAnim;
    /* 0x68 */ f32 savedFrame;
    /* 0x6C */ s32 timer; /* ticks in the state */
    /* 0x70 */ u32 *animes;
} ObisuRunW;

#define OBISURUN(obj) ((ObisuRunW *) FX(obj))

/* "obisuEffect"'s work, at its task + 0x20: an enemy's to 0x34, then its colours. */
typedef struct ObisuEffectW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ s16 alpha;
    /* 0x36 */ u8 env[3];
    /* 0x39 */ u8 prim[3];
    /* 0x3C */ u8 pad3C[4];
    /* 0x40 */ s32 timer;
} ObisuEffectW;

#define OBISUEFFECT(obj) ((ObisuEffectW *) FX(obj))

/* "obisuAtari"'s work, at its task + 0x20: an enemy's to 0x34, then its ride. */
typedef struct ObisuAtariW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ RideFrame *ride;
} ObisuAtariW;

#define OBISUATARI(obj) ((ObisuAtariW *) FX(obj))

/* "obisu"'s work, at its task + 0x20: a demo talker's (PersonObj) up to 0x44. */
typedef struct ObisuW {
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
    /* 0x48 */ u8 flags; /* 1 no fall, 2 walking back, 4 the sky's tunes change 300 ticks on */
    /* 0x4C */ Obj *glow; /* its glow as it leaves (kind 0x172) */
    /* 0x50 */ s32 timer; /* ticks in the state */
    /* 0x54 */ Camera *cam;
    /* 0x58 */ Obj *pass; /* the pass (kind 0x1C3) */
    /* 0x5C */ u8 tuneTimer;
} ObisuW;

#define OBISU(obj) ((ObisuW *) FX(obj))

/* "demoObisuRun_proc"'s work, at its task + 0x20: an enemy's to 0x34, then its own. */
typedef struct DemoObisuRunW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model; /* an empty model, for the goal box */
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ Obj *obisu; /* Obisumaru running (kind 0x16F, "obisurun") */
    /* 0x38 */ Camera *cam;
    /* 0x3C */ ACollFrame *box; /* the goal */
    /* 0x40 */ u8 pad40;
    /* 0x41 */ s8 partake; /* gGameWork.partake, kept while the race runs */
    /* 0x42 */ u8 pad42[2];
    /* 0x44 */ s32 timer; /* ticks in the state */
} DemoObisuRunW;

#define DEMOOBISURUN(obj) ((DemoObisuRunW *) FX(obj))

/* acollAtkAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy,
                                     s16 oz, s32 a);

/* acollDmgAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*DmgAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height,
                                     s16 ox, s16 oy, s16 oz, s32 a);

void obisuRunToStartLine(Obj *o);
void obisuRunStartRace(Obj *o);
f32 obisuRunGetX(Obj *o);
u8 obisuRunGetZone(Obj *o);
void obisuRunSetAnime(Obj *o, s32 n);
void obisuRunStop(Obj *o);
void obisuRunFallOn(Obj *o);
void obisuRunDmgFree(Obj *o);
void obisuRunFaceFixed(Obj *o);
s16 *obisuRunActStep(Obj *o);
Vec3f obisuRunBackPos(Obj *o);
void obisuRunPuff(Obj *o, f32 x, f32 y, f32 z, f32 size);
void obisuAtariSetPos(Obj *o, f32 x, f32 y, f32 z);
void obisuEffectSetPos(Obj *o, f32 x, f32 y, f32 z);
void obisuEffectSetColor(Obj *o, u8 r, u8 g, u8 b);

void obisuFadeOut(f32 speed);
void obisuPassToPlayer(Obj *o, Obj *player);

extern Unk88208 gSaveCopy; /* the save as it was (undefined_syms.txt; demo.c's) */

extern u8 obisuAtariPlatformRide[]; /* the platform's ride data */
extern u8 obisuMaruStandAnim[]; /* Obisumaru's first animation (standing) */
extern u8 obisuEffectGlowDl[]; /* the glow's display list */
extern u8 obisuMaruWalkAnim[]; /* Obisumaru walking */
extern u8 obisuMaruJumpAnim[]; /* Obisumaru jumping */

u32 obisuRunAnimes[] = { (u32) obisuMaruStandAnim, (u32) obisuMaruWalkAnim, (u32) obisuMaruJumpAnim, 0x030014EC, 0x030017E8, 0x03001164 }; /* Obisumaru's animations */
s16 obisuRunCourseStart[] = { 3, 1, 7, 428, 15, 25, 10, 6, -1, 0 }; /* his course to the start line */
s16 obisuRunCourseRace[] = { 3, 1, 7, 709, 0, 25, 10, 3, 2, 8, 1, 759, 0, 20, 25, 3, 1, 7, 1251, 0, 25, 10, 3, 2, 8, 1, 1288, 0, 20, 16, 3, 2, 8, 1, 1334, 0, 20, 16, 3, 2, 8, 1, 1384, 0, 20, 16, 3, 2, 8, 1, 1435, 0, 20, 16, 3, 2, 8, 1, 1486, 0, 20, 16, 3, 2, 8, 1, 1541, 0, 20, 16, 3, 2, 8, 1, 1582, 0, 5, 30, 3, 1, 7, 416, 0, 25, 10, 3, 2, 8, 3, 300, 0, 30, 45, 3, 1, 7, 157, 0, 25, 10, 3, 2, 8, 3, 179, 0, 8, 42, 3, 1, 7, 327, 0, 25, 10, 3, 2, 8, 2, 377, 0, 18, 16, 3, 2, 8, 2, 430, 0, 18, 16, 3, 2, 8, 2, 491, 0, 18, 16, 3, 2, 8, 2, 635, 0, 30, 30, 3, 1, 7, 658, 0, 25, 10, 3, 2, 8, 2, 706, 0, 18, 16, 3, 1, 7, 858, 0, 25, 10, 3, 2, 8, 2, 913, 0, 18, 24, 3, 2, 8, 2, 961, 0, 18, 24, 3, 2, 8, 2, 1003, 0, 18, 24, 3, 2, 8, 2, 1053, 0, 18, 24, 3, 2, 8, 2, 1088, 0, 18, 24, 3, 1, 7, 1217, 0, 25, 10, 3, 2, 8, 2, 1258, 0, 18, 24, 3, 2, 8, 2, 1301, 0, 18, 24, 3, 2, 8, 2, 1352, 0, 18, 24, 3, 1, 7, 1560, 0, 25, 10, 3, 2, 8, 2, 1622, 0, 18, 24, 3, 2, 8, 2, 1674, 0, 18, 24, 3, 1, 7, 1865, 0, 25, 10, 3, 2, 11, 2, 1960, 0, 30, 40, 3, 1, 7, 2139, 0, 40, 10, 6, -1, 0, 0, 0 }; /* his course for the race */

/* The screen fades to black at the given speed. The first call is on the brace's line: IDO
 * schedules by line (file 127's beatFadeOut). */
void obisuFadeOut(f32 speed) { fadeSetSpeed(speed);
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
void obisuPassToPlayer(Obj *o, Obj *player) {
    PassW *w = (PassW *) FX(o);

    w->state = 3;
    w->player = player;
    w->timer = 0;
}

/* The pass's sound ("eventTegataSdSet") and the clear's, emptied; their prints' strings remain
 * (file 127's beatEventTegataSdSet and beatClearSdSet). */
void obisuEventTegataSdSet(void) {
    DPRINTF("&&&& eventTegataSdSet  pl D がないn");
}

void obisuEventClearSd(void) {
    DPRINTF("&&&& SD_G_CLEAR1n");
    DPRINTF("&&&& SD_E_CLEAR1n");
    DPRINTF("&&&& SD_S_CLEAR1n");
    DPRINTF("&&&& SD_Y_CLEAR1n");
}

/* "obisu": Obisumaru, the false Ebisumaru, walking up and down. Gone once the pass is won (flag
 * 0x9B) or with no Ebisumaru among the players. Talked to by another, he talks (0x1F8) and walks
 * on; by Ebisumaru, he jumps, turns to him and talks; taken on (flag 7), the screen fades and the
 * race begins at the atlas (stage 1, 2) with Ebisumaru alone. Back from it (0x92) he talks again
 * and, if Ebisumaru won (8), gives the pass, talks (0x1F9) and, at flag 0x95, runs off glowing.
 * Matched 2026-10-12; it was parked for case 6's shared 0.0f and case 11's two un-shared 2.0
 * loads; eneFall(o, 2) made 2.0 one constant across the function, rematerialised in case 11 as
 * the game does, as its twin tlb_0142's obisu_maru2_proc already had it. */
s32 obisu_maru_proc(Obj *o, s16 msg, void *arg) {
    ObisuW *w = OBISU(o);
    s32 i;
    s32 chara;
    u32 data;
    s32 pad2; /* FAKE */
    Vec3f at;
    CamView *view;
    s32 pad3; /* FAKE */
    Obj *pl;
    s32 idx;
    s32 pad4[2]; /* FAKE */
    f32 x;
    f32 y;
    f32 z;
    Vec3f spark;
    s32 pad5; /* FAKE */
    Vec3f p;

    switch (msg) {
        case 2:
            o->name = "obisu";
            if (swWorkTest(0x9B)) {
                DPRINTF(">>>  obisu すでに手形ゲット(即ち死ね)\n");
                return -1;
            }
            for (i = 0; i < 4; i++) {
                if (gSaveData.players[i].chara == 1) {
                    DPRINTF("****   obisu_maru_proc: エビス丸おったで\n");
                    goto found;
                }
            }
            DPRINTF("*****  obisu_maru_proc: エビス丸がおらへんで  即ち死ね\n");
            return -1;
        found:
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) obisuMaruStandAnim);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("obisu_maru_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 140, 0, 100, 0, 0);
            demoTalkInit(o, data);
            w->rot[1] = 4.712388992f;
            if (swWorkTest(0x92)) {
                swWorkReset(0x92);
                gGameWork.scenarioRunning = 2;
                camSunSetClockStop(1);
                w->state = 10;
            }
            break;
        case 1:
            demoTalkerFree(o);
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            break;
        case 8:
            return demoTalk(o, arg);
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    w->model->anim = obisuMaruWalkAnim;
                    w->model->frame = 0.0f;
                    w->state = 1;
                    break;
                case 1:
                    eneTurnToAngle(o, 270.0f, 20.0f);
                    if (demoTalkEndCheck(o)) {
                        demoCamZoom_CL(w->talker);
                        DPRINTF("obisu_maru_proc: ミッション始めませんぜ\n");
                        if (plGetChara(w->talker) == 1) {
                            DPRINTF("obisu_maru_proc: ミッション始めるぜ\n");
                            w->state = 6;
                            w->timer = 0;
                        } else {
                            DPRINTF("obisu_maru_proc: ミッション始めませんぜ\n");
                            w->state = 2;
                            w->timer = 0;
                        }
                    } else {
                        if (w->timer < 61) {
                            eneAnimLoop(o, w->model, 0.5f);
                        } else {
                            eneAnimLoop(o, w->model, 1.2f);
                        }
                        if (w->flags & 2) {
                            if (eneMoveToPos(o, -39.0f, -44.0f, 2.0f)) {
                                w->flags &= ~2;
                            }
                        } else {
                            if (eneMoveToPos(o, -39.0f, -27.0f, 2.0f)) {
                                w->flags |= 2;
                            }
                        }
                        if (w->timer >= 120) {
                            w->timer = 0;
                        }
                    }
                    break;
                case 2:
                    scRunStart(0x1F8, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    w->state = 3;
                    w->vel[0] = 0.0f;
                    w->vel[1] = 0.0f;
                    w->vel[2] = 0.0f;
                    break;
                case 3:
                    if (!scIsRunning()) {
                        personTalkCamOff();
                        demoFinish(o);
                        DPRINTF("また探し始める\n");
                        w->state = 1;
                    }
                    break;
                case 6:
                    if (w->timer == 1) {
                        w->model->anim = obisuMaruJumpAnim;
                        w->model->frame = 0;
                        w->vel[0] = 0;
                        w->flags |= 1;
                        w->hit &= ~1;
                        w->vel[2] = 0;
                        w->vel[1] = 2.0f;
                        DPRINTF("飛び\n");
                    }
                    if (polAnimeLastFrame(w->model) / 2 < w->model->frame) {
                        w->flags &= ~1;
                        w->vel[1] = 0.0f;
                    }
                    if (w->hit & 1) {
                        w->model->anim = obisuMaruStandAnim;
                        w->model->frame = 0.0f;
                        w->state = 5;
                    }
                    eneAnimPlayOnce(o, w->model, 0.8f);
                    break;
                case 5:
                    if (w->talker != NULL) {
                        tskSendMsg(w->talker, 4, &at);
                        if (eneTurnToPoint(o, at.x, at.z, 3.0f) == 1) {
                            w->state = 4;
                        }
                    }
                    break;
                case 4:
                    scRunStart(0x1F8, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    w->state = 7;
                    break;
                case 7:
                    if (!scIsRunning()) {
                        if (swWorkTest(7)) {
                            gGameWork.scenarioRunning = 2;
                            w->timer = 0;
                            w->state = 8;
                        } else {
                            w->model->anim = obisuMaruWalkAnim;
                            w->model->frame = 0.0f;
                            w->state = 9;
                        }
                    }
                    break;
                case 8:
                    if (w->timer == 1) {
                        sndPlay(10);
                        obisuFadeOut(16 / gRetracesPerFrame);
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        view = camGetView(w->cam);
                        camSetSpeed(w->cam, 10.0f);
                        camMoveAtY(w->cam, view->at[1] + 50.0f);
                        gGameWork.scenarioRunning = 2;
                    }
                    if (w->timer == 20 / gRetracesPerFrame) {
                        gGameWork.scenarioRunning = 0;
                        camAiSetHold(0);
                        demoAreaFlagSave();
                        for (i = 0; i < 4; i++) {
                            gSaveCopy.players[i].chara = -1;
                        }
                        for (i = 0; i < 4; i++) {
                            if (gSaveData.players[i].chara != 1 && gSaveData.players[i].chara != -1) {
                                gSaveCopy.players[i] = gSaveData.players[i];
                            }
                        }
                        demoAreaFlagSetBit7(1);
                        gSaveData.stageFlags[0x11] |= 1;
                        gSaveData.stageFlags[0x11] &= ~0x80;
                        gSaveData.stageFlags[0xA] |= 1;
                        gSaveData.stageFlags[0xA] &= ~0x80;
                        for (i = 0; i < 4; i++) {
                            if (gSaveData.players[i].chara != 1 || gSaveData.players[i].lives <= 0) {
                                gSaveData.players[i].chara = -1;
                            }
                        }
                        stageGotoAtlas(1, 2);
                    }
                    break;
                case 9:
                    if (eneTurnToAngle(o, 161.01414489746094f, 20.0f)) {
                        personTalkCamOff();
                        demoFinish(o);
                        DPRINTF("また探し始める\n");
                        w->state = 1;
                    }
                    break;
                case 10:
                    if (w->timer == 1) {
                        demoPlStandAll();
                    }
                    if (w->timer == 8) {
                        if (!swWorkTest(8)) {
                            camSunSetTunes4(0x96, 0x93, 0x94, 0x95);
                        }
                        pl = plCheck();
                        personCamZoomUp(pl);
                        w->rot[1] = 0.0f;
                        idx = demoFindPlByChara(1);
                        if (idx == 4) {
                            idx = plGetIndex(pl);
                            chara = plGetChara(pl);
                        } else {
                            chara = 1;
                        }
                        scRunStart(0x1F8, idx + 1, chara);
                        if (swWorkTest(8)) {
                            w->state = 11;
                        } else {
                            w->state = 16;
                        }
                    }
                    break;
                case 11:
                    if (!scIsRunning()) {
                        pl = plCheck();
                        if (pl == NULL) {
                            return -1;
                        }
                        plGetModelPos(pl, &x, &y, &z);
                        w->pass = eneMakeEx(NULL, ENE_DEMO_TEGATA, 0, (w->model->pos[0] + x) / 2, w->model->pos[1],
                                            (w->model->pos[2] + z) / 2, 0, NULL);
                        w->timer = 0;
                        w->state = 12;
                    }
                    break;
                case 12:
                    if (w->timer == 200 / gRetracesPerFrame) {
                        demoGetEneModelPos(w->pass, &spark);
                        ptclSpawnSparksFall(spark.x, spark.y, spark.z);
                        eneMakeEx(NULL, ENE_DEMO_KIRAKIRA, 0, spark.x, spark.y, spark.z, 0, NULL);
                        obisuPassToPlayer(w->pass, gGameWork.players[demoFindPlByChara(1)]);
                        w->flags |= 4;
                    }
                    if (w->timer == 260 / gRetracesPerFrame) {
                        scRunStart(0x20, 0, 0);
                        w->timer = 0;
                        w->state = 13;
                    }
                    break;
                case 13:
                    if (!scIsRunning() && w->timer >= 180 / gRetracesPerFrame) {
                        scRunStart(0x1F9, 0, 0);
                        w->state = 14;
                    }
                    break;
                case 14:
                    if (swWorkTest(0x95)) {
                        swWorkReset(0x95);
                        personTalkCamOff();
                        w->timer = 0;
                        w->state = 15;
                    }
                    break;
                case 15:
                    p = obisuRunBackPos(o);
                    if (w->timer == 1) {
                        w->glow = eneMakeEx(o, ENE_OBISU_EFFECT, 0, p.x, p.y, p.z, 2, (u32 (*)(void *, void *, void *)) 0x14);
                        w->flags |= 1;
                        w->vel[1] = 1.0f;
                        w->vel[2] = 1.0f;
                        w->vel[0] = 3.0f;
                    }
                    eneFaceMoveDir(o);
                    if (w->y > 78.0f) {
                        demoPlReleaseAll();
                        camSunSetClockStop(1);
                        return -1;
                    }
                    if (w->timer % 3 == 0) {
                        itexPuffGrey(p.x, p.y, p.z, 0.08f);
                    }
                    obisuEffectSetPos(w->glow, p.x, p.y, p.z);
                    break;
                case 16:
                    if (!scIsRunning()) {
                        w->state = 17;
                    }
                    break;
                case 17:
                    if (eneTurnToAngle(o, 270.0f, 20.0f)) {
                        personTalkCamOff();
                        demoPlReleaseAll();
                        camSunSetClockStop(1);
                        w->state = 0;
                    }
                    break;
            }
            if (w->flags & 4) {
                if (300 / gRetracesPerFrame == w->tuneTimer++) {
                    camSunSetTunes4(0x96, 0x93, 0x94, 0x95);
                }
            }
            personPlBlockCheck(o);
            if (!(w->flags & 1)) {
                if (w->state == 6) {
                    eneFall(o, 2);
                } else {
                    eneFall(o, 0.4f);
                }
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
            if (w->state != 0 && w->state != 1 && w->state != 6 && w->state != 5) {
                eneAnimLoop(o, w->model, 0.5f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 2.0f, NULL);
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "demoObisuRun_proc": the race's manager. Made once the race is taken on (flag 7) and until the
 * pass is won (0x9B). Obisumaru appears at the start (kind 0x16F), the camera shows the course
 * and both talk (0x21B, 0x218-0x21A); both run to the start line, then the race (0x21C) runs
 * until one touches the goal box: Ebisumaru first, Obisumaru stops on the line (0x21E); Obisumaru
 * first, he turns and gloats (0x21D). Then the screen fades, flag 0x92 is set, the players are
 * restored and the game goes back to Obisumaru's stage (0xBA). A player dying ends it too.
 * The three pads are frame words the game has (its stack slots, measured); "mission = 1" first
 * gives the game's hoisted "li v1,1". The turn's angle is written 0.0948633254f: IDO reads
 * that literal as the game's word (0x3DC247B4). */
s32 demoObisuRun_proc(Obj *o, s16 msg, void *arg) {
    DemoObisuRunW *w = DEMOOBISURUN(o);
    CamView *view;
    s32 pad;
    f32 obisuX;
    f32 best;
    f32 place;
    f32 height;
    f32 side;
    s32 zone;
    s32 i;
    s32 pad2;
    s32 j;
    s32 pad3;

    switch (msg) {
        case 2:
            o->name = "demoObisuRun_proc";
            if (!swWorkTest(7)) {
                DPRINTF("demoObisuRun_proc: おびすまるおっかけは開始されません\n");
                return -1;
            }
            if (swWorkTest(0x9B)) {
                DPRINTF("demoObisuRun_proc: おびすまるおっかけは開始されません 手形もらってるで\n");
                return -1;
            }
            eneSetup(o, arg);
            w->model = eneNewPol(o, NULL);
            if (w->model == NULL) {
                return -1;
            }
            gGameWork.mission = 1;
            w->partake = gGameWork.partake;
            gGameWork.partake = 1;
            gGameWork.areaStage = 2;
            gGameWork.scenarioRunning = 2;
            DPRINTF("***  demoObisuRun セット完了\n");
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->obisu = eneMakeEx(o, ENE_OBISU_RUN, 1, 428.0f, 167.0f, 15.0f, 0, NULL);
                    eneSetAngle(w->obisu, 90.0f);
                    DPRINTF("demoObisuRun_roc: オビス丸出現\n");
                    camAiSetHold(1);
                    w->cam = camAiGetCam();
                    view = camGetView(w->cam);
                    view->fovy = 0.6981317401f;
                    camSetEye(w->cam, -706.0f, 165.0f, -20.0f);
                    camSetAt(w->cam, -792.0f, 188.0f, -74.0f);
                    camSunSetClockStop(1);
                    demoPlStandAll();
                    scRunStart2(0x21B, 0, 0);
                    w->state = 9;
                    break;
                case 1:
                    if (!scIsRunning()) {
                        w->timer = 0;
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (w->timer == 1) {
                        demoSetSpeedPlRun(269.0f, 167.0f, 0.0f, 0.5f, 269.0f, 167.0f, 0.0f, 0.5f);
                    }
                    if (demoPlRunEndCheck() && w->timer > 120 / gRetracesPerFrame) {
                        w->timer = 0;
                        w->state = 3;
                    }
                    break;
                case 3:
                    camSetFov(w->cam, 0.95f);
                    camSetAt(w->cam, -899.0f, 187.0f, -51.0f);
                    camSetEye(w->cam, -906.0f, 190.0f, -47.0f);
                    scRunStart2(0x218, 0, 0);
                    w->state = 4;
                    break;
                case 4:
                    if (!scIsRunning()) {
                        w->timer = 0;
                        w->state = 5;
                    }
                    break;
                case 5:
                    scRunStart2(0x219, 0, 0);
                    camSetFov(w->cam, 0.6981317401f);
                    camSetAt(w->cam, -900.0f, 190.0f, -52.0f);
                    camSetEye(w->cam, -891.0f, 194.0f, -47.0f);
                    w->state = 6;
                    break;
                case 6:
                    if (!scIsRunning()) {
                        w->timer = 0;
                        w->state = 7;
                    }
                    break;
                case 7:
                    if (w->timer == 1) {
                        scRunStart2(0x21A, 0, 0);
                        camSetFov(w->cam, 0.6981317401f);
                        camSetAt(w->cam, -902.0f, 268.0f, -216.0f);
                        camSetEye(w->cam, -728.0f, 161.0f, -11.0f);
                        DPRINTF("demoObisuRun_proc: DEMO_EBISU_RUN\n");
                        demoSetSpeedPlRun(428.0f, 167.0f, 0.0f, 1.399999976f, 428.0f, 167.0f, 0.0f, 1.399999976f);
                        obisuRunToStartLine(w->obisu);
                    }
                    if (!scIsRunning()) {
                        w->timer = 0;
                        w->state = 8;
                    }
                    break;
                case 8:
                    if (w->timer == 1) {
                        scRunStart2(0x21B, 0, 0);
                        camSetFov(w->cam, 0.6981317401f);
                        camSetAt(w->cam, -901.0f, 201.0f, -69.0f);
                        camSetEye(w->cam, -694.0f, 167.0f, -43.0f);
                    }
                    if (!scIsRunning()) {
                        w->state = 9;
                    }
                    break;
                case 9:
                    if (!scIsRunning()) {
                        scRunStart2(0x21C, 0, 0);
                        demoPlReleaseAll();
                        obisuRunStartRace(w->obisu);
                        camAiSetHold(0);
                        w->timer = 0;
                        w->state = 10;
                    }
                    break;
                case 10:
                    w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(uda_atk_root_frame, &w->model->f, 100, 0, 100, 0, 0);
                    w->box->kind = 100;
                    w->box->flags |= 0x10;
                    w->state = 11;
                    break;
                case 11:
                    if (acollAtkWasHit(w->box) == 1) {
                        obisuX = 0.0f;
                        best = 0.0f;
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                plAskPos(gGameWork.players[i], &place, &height, &side, &zone);
                                if (zone == 2 && best < place) {
                                    best = place;
                                }
                            }
                        }
                        if (obisuRunGetZone(w->obisu) == 2) {
                            obisuX = obisuRunGetX(w->obisu);
                        }
                        if (obisuX <= best) {
                            demoPlStandAll();
                            demoSetPlPos(2, place, height, side, 3.874631166f, 2, place, height, side, 3.874631166f, 2,
                                         place, height, side, 3.874631166f, 2, place, height, side, 3.874631166f);
                            w->state = 17;
                        } else {
                            obisuRunDmgFree(w->obisu);
                            obisuRunFallOn(w->obisu);
                            w->state = 13;
                        }
                        w->box->radius = 300;
                        w->box->ofs[1] = 300;
                    }
                    break;
                case 12:
                    if (w->timer > 120 / gRetracesPerFrame) {
                        w->state = 21;
                    }
                    break;
                case 13:
                    eneTurnToAngle(w->obisu, 0.0948633254f, 20.0f);
                    if (acollAtkWasHit(w->box) == 1) {
                        DPRINTF("demoObisuRun_proc: DEMO_OBISU_UP\n");
                        demoPlStandAll();
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        view = camGetView(w->cam);
                        view->fovy = 0.6981317401f;
                        camSetSpeed(w->cam, 60 / gRetracesPerFrame);
                        camMoveAt(w->cam, -66.0f, 500.0f, 129.0f);
                        camMoveEye(w->cam, -115.0f, 480.0f, 164.0f);
                        w->timer = 0;
                        w->state = 14;
                    }
                    break;
                case 14:
                    if (eneTurnToAngle(w->obisu, 311.41790771484375f, 20.0f)) {
                        w->state = 15;
                    }
                    break;
                case 15:
                    if (camGetMoving(w->cam)) {
                        scRunStart2(0x21D, 0, 0);
                        obisuRunSetAnime(w->obisu, 3);
                        obisuRunStop(w->obisu);
                        w->state = 16;
                    }
                    break;
                case 16:
                    if (!scIsRunning()) {
                        gGameWork.scenarioRunning = 2;
                        w->timer = 0;
                        w->state = 21;
                    }
                    break;
                case 17:
                    if (obisuRunGetZone(w->obisu) == 2 && obisuRunGetX(w->obisu) >= 2122.0f) {
                        DPRINTF("demoObisuRun_proc: DEMO_EBISU_UP\n");
                        demoSetSpeedPlRun(2140.0f, 482.0f, 0.0f, 1.399999976f, 2140.0f, 482.0f, 0.0f, 1.399999976f);
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        view = camGetView(w->cam);
                        view->fovy = 0.6981317401f;
                        camSetSpeed(w->cam, 60 / gRetracesPerFrame);
                        camMoveAt(w->cam, -78.0f, 505.0f, 117.0f);
                        camMoveEye(w->cam, -71.0f, 473.0f, 177.0f);
                        obisuRunFallOn(w->obisu);
                        obisuRunFaceFixed(w->obisu);
                        w->timer = 0;
                        w->state = 18;
                    }
                    break;
                case 18:
                    if (eneMoveToX(w->obisu, 2125.0f, 1.0f)) {
                        w->state = 19;
                    }
                    break;
                case 19:
                    if (camGetMoving(w->cam)) {
                        scRunStart2(0x21E, 0, 0);
                        obisuRunSetAnime(w->obisu, 4);
                        obisuRunStop(w->obisu);
                        w->state = 20;
                    }
                    break;
                case 20:
                    if (!scIsRunning()) {
                        gGameWork.scenarioRunning = 2;
                        w->timer = 0;
                        w->state = 21;
                    }
                    break;
                case 21:
                    if (w->timer == 1) {
                        for (j = 0; j < 4; j++) {
                            contPakInitRumble(j);
                            contPakRumbleRequestStop(j);
                        }
                        if (!swWorkTest(8)) {
                            sndPlay(10);
                        }
                        obisuFadeOut(16 / gRetracesPerFrame);
                        gGameWork.scenarioRunning = 2;
                        if (!swWorkTest(8)) {
                            sndPlay(0x45F);
                        }
                    }
                    if (w->timer == 20 / gRetracesPerFrame) {
                        gGameWork.areaStage = 2;
                        gGameWork.scenarioRunning = 0;
                        gGameWork.mission = 0;
                        gGameWork.partake = w->partake;
                        swWorkSet(0x92);
                        camSunSetClockStop(0);
                        demoAreaFlagRestore();
                        for (j = 0; j < 4; j++) {
                            if (gSaveCopy.players[j].chara != 1 && gSaveCopy.players[j].chara != -1) {
                                gSaveData.players[j] = gSaveCopy.players[j];
                            }
                        }
                        stageGotoSpawn(gStageTask->parent, 0xBA, 0, -36.0f, 10.0f, -11.0f, 3.1415927f, 0xD7);
                    }
                    break;
            }
            if (gGameWork.mission == 2 && w->state != 21) {
                DPRINTF("%%%%   event manager: PLAYER 死にました信号をうけとったぜ！\n");
                gGameWork.scenarioRunning = 2;
                sndPlay(10);
                demoPlStandAll();
                w->timer = 0;
                w->state = 21;
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "obisurun": Obisumaru running. Made while the race is on (flag 7). Told to (states 1 and 2),
 * he runs a course of commands (obisuRunActStep) to its end; hit, he stops for 30 ticks, red, and
 * then runs on. */
s32 obisuRunProc(Obj *o, s16 msg, void *arg) {
    ObisuRunW *w = OBISURUN(o);
    s32 pad;
    Vec3f p;

    switch (msg) {
        case 2:
            o->name = "obisurun";
            if (!swWorkTest(7)) {
                DPRINTF("obisu_run_proc: OBISU_MISSION フラグたってないで\n");
                return -1;
            }
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) obisuMaruStandAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->animes = obisuRunAnimes;
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("obisu_run_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->dmg = ((DmgAddXYZFunc) acollDmgAddXYZ)(ene_dmg_root_frame, &w->model->f, 70, 160, 0, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->shadow = shadowCreate();
            w->atari = eneMakeEx(o, ENE_OBISU_ATARI, 0, w->model->pos[0], w->model->pos[1], w->model->pos[2], 0, NULL);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->dmg != NULL) {
                frmDestroy((Frame *) w->dmg);
            }
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            break;
        case 4:
            ((Vec3f *) arg)->x = w->model->pos[0];
            ((Vec3f *) arg)->y = w->model->pos[1];
            ((Vec3f *) arg)->z = w->model->pos[2];
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                case 4:
                    break;
                case 1:
                    DPRINTF("スタートラインまで\n");
                    w->course = (s16 *) obisuRunCourseStart;
                    w->state = 3;
                    break;
                case 2:
                    DPRINTF("いけー\n");
                    w->course = (s16 *) obisuRunCourseRace;
                    w->state = 3;
                    break;
                case 3:
                    if (*obisuRunActStep(o) == -1) {
                        DPRINTF("OCS終了\n");
                        w->model->anim = w->animes[0];
                        w->model->frame = 0.0f;
                        w->state = 0;
                    }
                    break;
                case 5:
                    if (w->timer == 1) {
                        w->model->anim = w->animes[5];
                        w->model->frame = 0.0f;
                        if (w->glow != NULL) {
                            obisuEffectSetColor(w->glow, 255, 0, 0);
                        }
                        w->flags |= 8;
                        eneMakeHitSpark(w->zone, w->x, w->y + 10.0f, w->z, 2, w->rot[1]);
                        if (w->dmg->hitKind != 0) {
                            switch (w->dmg->hitKind) {
                                case 18:
                                case 19:
                                case 20:
                                    soundIncamCam(0x22D, w->model->pos[0], w->model->pos[1], w->model->pos[2]);
                                    break;
                                case 9:
                                case 17:
                                    soundIncamCam(0x22C, w->model->pos[0], w->model->pos[1], w->model->pos[2]);
                                    break;
                            }
                        }
                    } else if (w->timer >= 30) {
                        DPRINTF("B vx[%f] vy[%f] vz[%f] time[%f]\n");
                        w->model->anim = w->savedAnim;
                        w->model->frame = w->savedFrame;
                        /* One line: IDO schedules by line; on three lines the loads and stores
                         * come out [0], [1], [2], the game's [2], [1], [0] (measured). */
                        w->vel[0] = w->savedVel[0]; w->vel[1] = w->savedVel[1]; w->vel[2] = w->savedVel[2];
                        w->flags = w->savedFlags;
                        DPRINTF("C vx[%f] vy[%f] vz[%f] time[%f]\n");
                        if (w->glow != NULL) {
                            obisuEffectSetColor(w->glow, 255, 255, 0);
                        }
                        w->state = 3;
                    }
                    eneTurnToAngle(o, eneCourseAngle(o, 8) * 360.0f / 6.2831855f, 20.0f);
                    if (*w->course == 8 && w->timer % 3 == 0) {
                        obisuRunPuff(o, w->model->pos[0], w->model->pos[1], w->model->pos[2], 0.08f);
                    }
                    break;
            }
            if (acollDmgWasHit(w->dmg) == 1 && w->state != 5 && w->dmg->hitKind != 100) {
                DPRINTF("いてーなー\n");
                w->savedFlags = w->flags;
                w->flags |= 1;
                w->savedVel[0] = w->vel[0];
                w->savedVel[1] = w->vel[1];
                w->savedVel[2] = w->vel[2];
                w->vel[2] = 0.0f;
                w->vel[1] = 0.0f;
                w->vel[0] = 0.0f;
                w->savedAnim = w->model->anim;
                w->savedFrame = w->model->frame;
                w->timer = 0;
                w->state = 5;
                DPRINTF("A vx[%f] vy[%f] vz[%f] time[%f]\n");
            }
            if (!(w->flags & 1)) {
                eneFall(o, 0.8f);
                w->hit &= ~1;
                if (eneLand((EneObj *) w, 10.0f) == 1) {
                    w->hit |= 1;
                    w->vel[1] = 0.0f;
                }
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                if (w->flags & 2) {
                    eneSetModel(o, &w->model->f);
                } else {
                    eneSetModelOnCourse(o, &w->model->f);
                }
            }
            if (w->flags & 4) {
                eneAnimPlayOnce(o, w->model, 0.8f);
            } else {
                eneAnimLoop(o, w->model, 0.7f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 2.0f, NULL);
            if (!(w->flags & 0x10)) {
                eneFaceMoveDir(o);
            }
            if (w->model->pos[1] < 0.0f) {
                DPRINTF("obisu_run_proc: error: 高度が０以下になったのでしねや\n");
                return -1;
            }
            if (w->atari != NULL) {
                obisuAtariSetPos(w->atari, w->model->pos[0], w->model->pos[1], w->model->pos[2]);
            }
            if (w->glow != NULL) {
                p = obisuRunBackPos(o);
                obisuEffectSetPos(w->glow, p.x, p.y, p.z);
            }
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Obisumaru goes to the start line (its course 1). */
void obisuRunToStartLine(Obj *o) {
    OBISURUN(o)->state = 1;
}

/* Obisumaru runs the race (its course 2). */
void obisuRunStartRace(Obj *o) {
    OBISURUN(o)->state = 2;
}

/* Obisumaru's place along its course. */
f32 obisuRunGetX(Obj *o) {
    return OBISURUN(o)->x;
}

/* Obisumaru's course (zone). */
u8 obisuRunGetZone(Obj *o) {
    return OBISURUN(o)->zone;
}

/* Obisumaru's animation n, from its start. */
void obisuRunSetAnime(Obj *o, s32 n) {
    ObisuRunW *w = OBISURUN(o);

    w->model->anim = w->animes[n];
    w->model->frame = 0.0f;
}

/* Obisumaru stops (state 4): his glow goes. */
void obisuRunStop(Obj *o) {
    ObisuRunW *w = OBISURUN(o);

    w->glow = tskDestroy(w->glow);
    w->state = 4;
    w->vel[0] = w->vel[1] = w->vel[2] = 0.0f;
}

/* Obisumaru falls again. */
void obisuRunFallOn(Obj *o) {
    OBISURUN(o)->flags &= ~1;
}

/* Obisumaru's damage box goes. */
void obisuRunDmgFree(Obj *o) {
    ObisuRunW *w = OBISURUN(o);

    if (w->dmg != NULL) {
        w->dmg = (ACollFrame *) frmDestroy((Frame *) w->dmg);
    }
}

/* Obisumaru no longer faces his way. */
void obisuRunFaceFixed(Obj *o) {
    OBISURUN(o)->flags |= 0x10;
}

/* The place behind Obisumaru's back (his glow's and his puffs'), returned by value. The frame
 * has one word more than these locals (an unused pad). A sine local "s" in that word gives the
 * frame but sorts the sums' operands (pos first; the game has the product first): measured. */
Vec3f obisuRunBackPos(Obj *o) {
    s32 pad;
    Vec3f v;
    f32 x;
    f32 y;
    f32 z;
    ObisuRunW *w = OBISURUN(o);

    x = w->model->pos[0] + mathSin(w->model->rot[1]) * -10.0f;
    z = w->model->pos[2] + mathSin(w->model->rot[1]) * -10.0f;
    y = w->model->pos[1] + 3.0f;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

/* Obisumaru's course: one command a tick, each a halfword and its arguments; -1 ends it (the
 * caller tests the command it returns). 0 run to x, z at a speed; 1 go on at another place of
 * the race's course; 2 turn to an angle; 3 an animation; 4 wait; 5 a sound; 6 stop; 7 run to x,
 * z at a speed in x and z (glowing when fast); 8 jump to a course point (landing on the ground);
 * 10 run to a course point off the course; 11 jump to a course point (for a set time).
 * Matched 2026-10-12; it was parked for float colouring in case 7 and the f-register rotation
 * in cases 8 and 11 after it; the speeds read whole and divided in place (the game's div.s),
 * as cases 8 and 11, closed it. */
s16 *obisuRunActStep(Obj *o) {
    ObisuRunW *w = OBISURUN(o);
    s32 pad; /* FAKE */

    switch (w->course[0]) {
        case 0: {
            f32 x = w->course[1];
            f32 z = w->course[2];
            f32 speed = w->course[3];

            if (eneMoveToPos(o, x, z, speed) == 1) {
                w->course += 4;
            }
            eneTurnToPoint(o, x, z, 13.0f);
            break;
        }
        case 1: {
            s32 pad1; /* FAKE */

            w->course = &obisuRunCourseRace[w->course[1]];
            break;
        }
        case 2: {
            f32 angle = w->course[1];

            w->vel[0] = 0.0f;
            w->vel[2] = 0.0f;
            if (eneTurnToAngle(o, angle, 13.0f) == 1) {
                w->course += 2;
            }
            break;
        }
        case 3: {
            u8 n = w->course[1];

            w->model->anim = w->animes[n];
            w->model->frame = 0.0f;
            if (n == 2) {
                w->flags |= 4;
            } else {
                w->flags &= ~4;
            }
            w->course += 2;
            break;
        }
        case 4: {
            s32 time = w->course[1];

            if (w->step == 0) {
                w->model->anim = w->animes[0];
                w->model->frame = 0.0f;
                w->vel[0] = 0.0f;
                w->vel[2] = 0.0f;
            }
            if (time < w->step++) {
                w->step = 0;
                w->model->anim = w->animes[1];
                w->model->frame = 0.0f;
                w->course += 2;
            }
            break;
        }
        case 5: {
            s16 id = w->course[1];

            soundLocationCam(id, w->x, w->y, w->z);
            DPRINTF("SoundLocation [%f] [%f] [%f]\n");
            w->course += 2;
            break;
        }
        case 6:
            w->vel[0] = w->vel[1] = w->vel[2] = 0.0f;
            DPRINTF("ＯＣＳおわりです。\n");
            w->course += 1;
            break;
        case 7: {
            f32 x;
            f32 z;
            f32 sx;
            f32 sz;
            Vec3f p;
            Vec3f q;

            x = w->course[1];
            z = w->course[2];
            sx = w->course[3];
            sz = w->course[4];
            sx /= 10.0f;
            sz /= 10.0f;
            if (eneMoveToXZ(o, x, z, sx, sz) == 1) {
                if (w->glow != NULL) {
                    w->glow = tskDestroy(w->glow);
                }
                w->step = 0;
                w->course += 5;
            } else {
                if (w->step == 0) {
                    p = obisuRunBackPos(o);
                    if (sx >= 3.0f) {
                        w->glow = eneMakeEx(o, ENE_OBISU_EFFECT, 0, p.x, p.y, p.z, 2, (u32 (*)(void *, void *, void *)) 0x14);
                    }
                }
                if (w->step % 3 == 0 && sx >= 3.0f) {
                    q = obisuRunBackPos(o);
                    obisuRunPuff(o, q.x, q.y, q.z, 0.08f);
                }
                w->step++;
            }
            break;
        }
        case 8: {
            f32 tx;
            f32 ty;
            f32 tz;
            f32 vx;
            f32 vy;
            f32 crs;
            f32 dist;
            f32 off;
            Vec3f p;
            Vec3f q;

            crs = w->course[1];
            dist = w->course[2];
            off = w->course[3];
            vx = w->course[4];
            vy = w->course[5];
            vx /= 10.0f;
            vy /= 10.0f;
            if (w->step == 0) {
                p = obisuRunBackPos(o);
                vramCoursePoint(crs, dist, off, &tx, &ty, &tz);
                w->jumpTime = sqrtf((tz - w->model->pos[2]) * (tz - w->model->pos[2]) +
                                    (tx - w->model->pos[0]) * (tx - w->model->pos[0])) / vx;
                if (crs == w->zone) {
                    if (w->x < dist) {
                    } else if (dist < w->x) {
                        vx *= -1.0f;
                    } else {
                        vx = 0.0f;
                    }
                } else {
                    if (w->model->pos[0] < tx) {
                    } else if (tx < w->model->pos[0]) {
                        vx *= -1.0f;
                    } else {
                        vx = 0.0f;
                    }
                }
                DPRINTF("JUMP_SPL: vx[%f] vy[%f] savetime[%d]\n");
                DPRINTF("JUMP_SPL: oo x[%f] y[%f] z[%f]\n");
                DPRINTF("JUMP_SPL: oo tx[%f] ty[%f] tz[%f]\n");
                w->vel[0] = vx;
                w->vel[1] = vy;
                w->flags |= 1;
                w->hit &= ~1;
                if (vy >= 3.0f) {
                    w->glow = eneMakeEx(o, ENE_OBISU_EFFECT, 0, p.x, p.y, p.z, 2, (u32 (*)(void *, void *, void *)) 0x14);
                } else {
                    w->glow = eneMakeEx(o, ENE_OBISU_EFFECT, 0, p.x, p.y, p.z, 2, (u32 (*)(void *, void *, void *)) 0xA);
                }
            } else if (w->step >= w->jumpTime / 2) {
                w->flags &= ~1;
                if (w->glow != NULL) {
                    w->glow = tskDestroy(w->glow);
                }
            }
            if (w->hit & 1) {
                DPRINTF("JUMP_SPL: end x[%f] y[%f] z[%f]\n");
                DPRINTF("JUMP_SPL: end tx[%f] ty[%f] tz[%f] crs[%d]\n");
                w->step = 0;
                w->course += 6;
            } else {
                w->step++;
            }
            if (w->step <= w->jumpTime / 2 && w->step % 3 == 0) {
                q = obisuRunBackPos(o);
                if (vy >= 3.0f) {
                    obisuRunPuff(o, q.x, q.y, q.z, 0.08f);
                } else {
                    obisuRunPuff(o, q.x, q.y, q.z, 0.04f);
                }
            }
            break;
        }
        case 11: {
            f32 tx;
            f32 ty;
            f32 tz;
            f32 vx;
            f32 vy;
            f32 crs;
            f32 dist;
            f32 off;
            Vec3f p;
            Vec3f q;

            crs = w->course[1];
            dist = w->course[2];
            off = w->course[3];
            vx = w->course[4];
            vy = w->course[5];
            vx /= 10.0f;
            vy /= 10.0f;
            if (w->step == 0) {
                p = obisuRunBackPos(o);
                vramCoursePoint(crs, dist, off, &tx, &ty, &tz);
                w->jumpTime = sqrtf((tz - w->model->pos[2]) * (tz - w->model->pos[2]) +
                                    (tx - w->model->pos[0]) * (tx - w->model->pos[0])) / vx;
                if (crs == w->zone) {
                    if (w->x < dist) {
                    } else if (dist < w->x) {
                        vx *= -1.0f;
                    } else {
                        vx = 0.0f;
                    }
                } else {
                    if (w->model->pos[0] < tx) {
                    } else if (tx < w->model->pos[0]) {
                        vx *= -1.0f;
                    } else {
                        vx = 0.0f;
                    }
                }
                DPRINTF("JUMP_SPL: vx[%f] vy[%f] savetime[%d]\n");
                w->vel[0] = vx;
                w->vel[1] = vy;
                w->flags |= 1;
                w->hit &= ~1;
                if (vy >= 3.0f) {
                    w->glow = eneMakeEx(o, ENE_OBISU_EFFECT, 0, p.x, p.y, p.z, 2, (u32 (*)(void *, void *, void *)) 0x14);
                } else {
                    w->glow = eneMakeEx(o, ENE_OBISU_EFFECT, 0, p.x, p.y, p.z, 2, (u32 (*)(void *, void *, void *)) 0xA);
                }
            }
            if (w->step >= w->jumpTime / 2) {
                DPRINTF("JUMP_SPL: end x[%f] y[%f] z[%f]\n");
                DPRINTF("JUMP_SPL: end tx[%f] ty[%f] tz[%f] crs[%d]\n");
                if (w->glow != NULL) {
                    w->glow = tskDestroy(w->glow);
                }
                w->vel[0] = 0.0f;
                w->vel[1] = 0.0f;
                w->step = 0;
                w->course += 6;
            } else {
                w->step++;
            }
            if (w->step <= w->jumpTime / 2 && w->step % 3 == 0) {
                q = obisuRunBackPos(o);
                if (vy >= 3.0f) {
                    obisuRunPuff(o, q.x, q.y, q.z, 0.08f);
                } else {
                    obisuRunPuff(o, q.x, q.y, q.z, 0.04f);
                }
            }
            break;
        }
        case 10: {
            f32 crs;
            f32 dist;
            f32 off;
            f32 x;
            f32 y;
            f32 z;
            f32 speed;

            w->flags |= 2;
            crs = w->course[1];
            dist = w->course[2];
            off = w->course[3];
            speed = w->course[4];
            vramCoursePoint(crs, dist, off, &x, &y, &z);
            if (eneMoveToPos(o, x, z, speed) == 1) {
                DPRINTF("COURSE_CHG: x[%f] y[%f] z[%f]\n");
                w->flags &= ~2;
                w->course += 5;
            }
            break;
        }
        default:
            DPRINTF("actStep がへんです\n");
            break;
    }
    return w->course;
}

/* A puff of dust at x, y, z: grey or white (flag 8). */
void obisuRunPuff(Obj *o, f32 x, f32 y, f32 z, f32 size) {
    if (OBISURUN(o)->flags & 8) {
        itexPuffGrey(x, y, z, size);
    } else {
        itexPuffWhite(x, y, z, size);
    }
}

/* "obisuAtari": a platform (its ride data in the file) that rides along with Obisumaru. */
s32 obisuAtari_proc(Obj *o, s16 msg, void *arg) {
    ObisuAtariW *w = OBISUATARI(o);

    switch (msg) {
        case 2:
            o->name = "obisuAtari";
            eneSetup(o, arg);
            w->model = eneNewPol(o, NULL);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[2] = 0.12f;
            w->model->scale[0] = w->model->scale[2];
            w->model->scale[1] = 0.17f;
            w->ride = rideCreate(gRideTree1, (s32) o, &w->model->f, (RideData *) eneFileAddr(o, obisuAtariPlatformRide, 0));
            DPRINTF("obisuAtari_proc: アイテムあたりセットＯＫ\n");
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->ride != NULL) {
                frmDestroy((Frame *) w->ride);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The platform's place (it rides with Obisumaru). */
void obisuAtariSetPos(Obj *o, f32 x, f32 y, f32 z) {
    ObisuAtariW *w = OBISUATARI(o);

    w->x = x;
    w->y = y;
    w->z = z;
}

/* "obisuEffect" (named "obisuAtari" too): a glow of the size its placement gives (tenths), that
 * swells and shrinks every other tick and follows Obisumaru; red while he is hit. */
s32 obisuEffectProc(Obj *o, s16 msg, void *arg) {
    ObisuEffectW *w = OBISUEFFECT(o);
    s32 pad;
    s32 size;

    switch (msg) {
        case 2:
            o->name = "obisuAtari";
            size = eneSetup(o, arg);
            w->model = enePolModelMake(o, (Gfx *) obisuEffectGlowDl, eneDlEnvPrimColor(o, 0, 0, 0, 0, 0, 0, 0), 1);
            w->model->flags |= 7;
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = size / 10;
            w->env[2] = 255;
            w->env[1] = 255;
            w->env[0] = 255;
            w->prim[2] = 0;
            w->prim[1] = 255;
            w->prim[0] = 255;
            w->alpha = 160;
            soundActpanCreateCam(o, 0x8125);
            DPRINTF("obisuEffect_proc: effect セットＯＫ\n");
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
        case 3:
            w->model->dl = eneDlEnvPrimColor(o, w->env[0], w->env[1], w->env[2], w->prim[0], w->prim[1], w->prim[2],
                                             w->alpha);
            break;
        case 4:
            ((Vec3f *) arg)->x = w->model->pos[0];
            ((Vec3f *) arg)->y = w->model->pos[1];
            ((Vec3f *) arg)->z = w->model->pos[2];
            break;
        case 0:
            eneStepHookPre(o);
            if (w->timer % 2) {
                w->model->scale[0] += 0.3;
                w->model->scale[1] += 0.3;
                w->model->scale[2] += 0.3;
                DPRINTF("光れ！\n");
            } else {
                w->model->scale[0] -= 0.3;
                w->model->scale[1] -= 0.3;
                w->model->scale[2] -= 0.3;
                DPRINTF("消えろ！\n");
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

/* The glow's place (it follows Obisumaru). */
void obisuEffectSetPos(Obj *o, f32 x, f32 y, f32 z) {
    ObisuEffectW *w = OBISUEFFECT(o);

    w->x = x;
    w->y = y;
    w->z = z;
}

/* The glow's colour. */
void obisuEffectSetColor(Obj *o, u8 r, u8 g, u8 b) {
    ObisuEffectW *w = OBISUEFFECT(o);

    w->prim[0] = r;
    w->prim[1] = g;
    w->prim[2] = b;
}
