#include "tlb.h"
#include "enemy.h"

/* The fireworks plaza (file 131; strings in IDA files/0131_08000000.bin.i64): "demohanabi",
 * "hidutu", "hanaoya", "hanadeshi", "hanabiHiroba_main", "hanaexit", "hanashot", "reversal",
 * "hana_fire", "particle", "deshi_p". The works hold only the fields the small helpers read;
 * each helper follows its task's proc in the file. */

typedef struct HanaStateW {
    /* 0x00 */ u8 state;
} HanaStateW; /* demohanabi, hidutu, hanaoya: the helpers set only the state */

/* The apprentice's work: an enemy's (EneObj's x, y, z, vel) with a show script. */
typedef struct HanadeshiW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 unused09;
    /* 0x0A */ u8 hit; /* 1: on the ground */
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rotX;
    /* 0x20 */ f32 rotY;
    /* 0x24 */ f32 rotZ;
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u8 pad34[0x4];
    /* 0x38 */ ACollFrame *atk;
    /* 0x3C */ u8 pad3C[0x4];
    /* 0x40 */ Obj *target; /* what it turns to after the talk */
    /* 0x44 */ MPolFrame *shadow;
    /* 0x48 */ u8 flags; /* 1: the animation plays once; 4: it runs backwards */
    /* 0x49 */ u8 pad49[0x3];
    /* 0x4C */ s16 *show; /* the show script it runs (in hanadeshiShows) */
    /* 0x50 */ s32 timer; /* the current command's */
    /* 0x54 */ u8 pad54[0x8];
    /* 0x5C */ f32 speed; /* the animation's */
    /* 0x60 */ Obj *demo; /* demohanabi, whose mortars it fires */
} HanadeshiW;

/* The show scripts the apprentices follow (0x1AC bytes each): commands of s16 words. */
extern s16 hanadeshiShows[][0xD6];

/* "hanaoya"'s work, the fireworks master's: an enemy's (EneObj's) with the talk's player. */
typedef struct HanaoyaW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 unused09;
    /* 0x0A */ u8 hit; /* 1: on the ground */
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ AnimeFrame *anim;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u8 pad34[0x4];
    /* 0x38 */ ACollFrame *atk;
    /* 0x3C */ u8 pad3C[0x4];
    /* 0x40 */ Obj *player; /* who talks to it */
    /* 0x44 */ MPolFrame *shadow;
    /* 0x48 */ Camera *cam;
    /* 0x4C */ u8 flags; /* 1: the animation plays once; 2: it has ended */
    /* 0x4D */ u8 pad4D[0x3];
    /* 0x50 */ s32 timer; /* the state's timer */
    /* 0x54 */ PolFrame *pass; /* the pass ("tegata") it hands over */
} HanaoyaW;

/* A "hidutu" (mortar): an enemy's work (EneObj's model and rot) with its number. */
typedef struct HidutuW {
    /* 0x00 */ u8 state; /* 0: waits to be hit, 1: fired */
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
    /* 0x34 */ u32 dl; /* hidutuDl in RAM */
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ u8 id; /* its number, 1 to 8 */
    /* 0x3D */ u8 pad3D[0x3];
    /* 0x40 */ MPolFrame *shadow;
    /* 0x44 */ s32 timer;
} HidutuW;

/* "hanashot"'s work: the show of shells fired for the sequence the player lit. */
typedef struct HanashotW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 pad01[0x3];
    /* 0x04 */ Camera *cam;
    /* 0x08 */ s32 timer;
    /* 0x0C */ u8 qNo; /* the shell being fired ("qNo") */
    /* 0x0D */ u8 pad0D[0x13];
    /* 0x20 */ Obj *fire; /* its hana_fire */
    /* 0x24 */ f32 atY; /* the camera's look-at height, kept while it shoots */
    /* 0x28 */ s32 wait; /* "randTime": frames to the next shell */
    /* 0x2C */ s8 partake; /* gGameWork's, kept */
    /* 0x2D */ u8 pad2D[0x3];
    /* 0x30 */ s32 endTime;
} HanashotW;

/* "hana_fire"'s work: a firework shell that rises and bursts (or falls and blows up). */
typedef struct HanaFireW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[0xC];
    /* 0x10 */ s32 color[3]; /* the burst's */
    /* 0x1C */ u8 state;
    /* 0x1D */ u8 good; /* 1: the burst, else it falls back and blows up */
    /* 0x1E */ u8 pad1E[0x2];
    /* 0x20 */ f32 speed0;
    /* 0x24 */ f32 speed;
    /* 0x28 */ s32 timer;
    /* 0x2C */ u8 flags; /* 1: it blew up; 2: the burst's other sound */
    /* 0x2D */ u8 pad2D[0x3];
    /* 0x30 */ s32 sun[3]; /* the sky's colour while it fades */
    /* 0x3C */ u8 sunStep[3];
} HanaFireW;

#define HANA_STATE(o) ((HanaStateW *) FX(o))
#define HANADESHI(o) ((HanadeshiW *) FX(o))
#define HIDUTU(o) ((HidutuW *) FX(o))
#define HANASHOT(o) ((HanashotW *) FX(o))
#define HANA_FIRE(o) ((HanaFireW *) FX(o))

#define DPRINTF

/* Takes size bytes from the display-list buffer, as particle.c's GFX_ALLOC (the do-while is the
 * block boundary that gives buf its saved register). */
#define GFX_ALLOC(p, size) \
    do { (p) = (void *) gGfxDlHead; gGfxDlHead = (Gfx *) ((u32) gGfxDlHead + (size)); } while (0)

/* Four stubs whose bodies were compiled out, all but their prints: the pass ("tegata") handed
 * to each player's character on clearing (Goemon, Ebisumaru, Sasuke, Yae). The strings end in
 * a plain "n", as the game has them. */
void hanaUnusedStub1(void) {
    DPRINTF("&&&& eventTegataSdSet  pl D がないn");
    DPRINTF("&&&& SD_G_CLEAR1n");
}

void hanaUnusedStub2(void) {
    DPRINTF("&&&& SD_E_CLEAR1n");
}

void hanaUnusedStub3(void) {
    DPRINTF("&&&& SD_S_CLEAR1n");
}

void hanaUnusedStub4(void) {
    DPRINTF("&&&& SD_Y_CLEAR1n");
}

/* demohanabi's work: its apprentice and master, its eight "hidutu" (mortars), the show's
 * phase, and what it takes from the players while the mission runs. */
typedef struct DemohanabiW {
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
    /* 0x34 */ Obj *deshi; /* hanadeshi */
    /* 0x38 */ Obj *oya; /* hanaoya */
    /* 0x3C */ Obj *hidutu[8];
    /* 0x5C */ u8 pad5C[0x4];
    /* 0x60 */ u32 data; /* its placement's (eneSetup), passed on to the two it makes */
    /* 0x64 */ u8 phase;
    /* 0x65 */ u8 pad65[0x3];
    /* 0x68 */ Camera *cam;
    /* 0x6C */ s32 timer;
    /* 0x70 */ u8 weapon[2]; /* players 1 and 2's weapon levels, kept while the show runs */
    /* 0x72 */ s8 partake; /* gGameWork's, kept while the show runs */
} DemohanabiW;

void H_GAME_FIRE_CHECK(Obj *o);
s32 hanabiSeqCheck(u8 n);
void hanabiSeqClear(void);
void hanaoyaSetState6(Obj *o);
s32 hanaoyaIsState12(Obj *o);
void hanaoyaSetState12(Obj *o);
void deshiSetPos(Obj *o, f32 a, f32 b, f32 c, f32 d);
void deshiTalkWaitSet(Obj *o);
void deshiSetDemo(Obj *o, Obj *demo);
void demoHanabiMakeJizou(void);

/* "demohanabi": the fireworks mission (flag 0x11). Made with flag 0x1A set (the show was
 * seen), it goes on after the show (9: the next sequence; 6: all five done; 10: a wrong one);
 * else it starts with the title (0-3) and the setting (4), then runs the game (5,
 * H_GAME_FIRE_CHECK). 6-8 hand the players to the master; 12 ends the mission, also when
 * the scenario signals a death (gGameWork.mission 2).
 * Matched 2026-10-12; it was parked for its float registers. Pi is spelled three ways so IDO
 * keeps the game's constants apart (IDO_NOTES 365): state 0's double, states 4 and 9 and the
 * roll's 3.1415927f, the rest 3.141592741f. */
s32 demoHanabi_proc(Obj *obj, s16 msg, void *arg) {
    DemohanabiW *w = (DemohanabiW *) FX(obj);
    f32 r;

    switch (msg) {
        case 2:
            obj->name = "demohanabi";
            if (!swWorkTest(0x11)) {
                DPRINTF("demoHanabi_proc: HANABI_MISSION たってません\n");
                return -1;
            }
            w->data = eneSetup(obj, arg);
            w->weapon[0] = gSaveData.players[0].weaponLevel;
            gSaveData.players[0].weaponLevel = 0;
            w->weapon[1] = gSaveData.players[1].weaponLevel;
            gSaveData.players[1].weaponLevel = 0;
            if (swWorkTest(0x1A)) {
                swWorkReset(0x1A);
                if (hanabiSeqCheck(gGameWork.hanabiQNo)) {
                    gGameWork.hanabiQNo++;
                    if (gGameWork.hanabiQNo >= 5) {
                        DPRINTF(">>>>>>>>>>>>>   全問終了\n");
                        w->state = 6;
                    } else {
                        DPRINTF(">>>>>>>>>>>>>   正解\n");
                        w->state = 9;
                    }
                } else {
                    DPRINTF(">>>>>>>>>>>>>   不正解\n");
                    w->state = 10;
                }
            }
            camAiSetHold(1);
            camSunSetClockStop(1);
            w->timer = 0;
            w->partake = gGameWork.partake;
            gGameWork.partake = 1;
            gGameWork.scenarioRunning = 1;
            gGameWork.mission = 1;
            DPRINTF("花火監視タスクセットＯＫ\n");
            break;
        case 1:
            gSaveData.players[0].weaponLevel = w->weapon[0];
            gSaveData.players[1].weaponLevel = w->weapon[1];
            gGameWork.areaStage = 3;
            break;
        case 0:
            eneStepHookPre(obj);
            switch (w->state) {
                case 0:
                    DPRINTF("demoHanabi_proc: タイトル\n");
                    r = 2.094395161f;
                    demoSetPlPos(0, 1.0f, 0.0f, 38.0f, 3.1415927410125732, 0, -13.0f, 0.0f, 31.0f, 1.745329261f, 0, 3.0f,
                                 0.0f, 73.0f, r, 0, 13.0f, 0.0f, 81.0f, r);
                    w->deshi = eneMakeEx(gEneManager->child->next->next->next, ENE_HANADESHI, 0, 3.0f, 0.0f, 18.0f, 2,
                                         (void *) w->data);
                    if (w->deshi == NULL) {
                        return -1;
                    }
                    deshiSetDemo(w->deshi, obj);
                    camAiSetHold(1);
                    w->cam = camAiGetCam();
                    camSetEye(w->cam, 1.0f, 247.0f, 185.0f);
                    camSetAt(w->cam, 1.0f, 118.0f, 103.0f);
                    w->timer = 0;
                    w->state = 1;
                    break;
                case 1:
                    if (80 / gRetracesPerFrame < w->timer) {
                        camSetSpeed(w->cam, 40 / gRetracesPerFrame);
                        camMoveFov(w->cam, 0.1981319911f);
                        camMoveEye(w->cam, 52.0f, 168.0f, 267.0f);
                        camMoveAt(w->cam, 24.0f, 81.0f, 130.0f);
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (!camGetMoving(w->cam)) {
                        scRunStart(0x229, 0, 0);
                        w->state = 3;
                    }
                    break;
                case 3:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x95)) {
                            swWorkReset(0x95);
                            w->phase = 0;
                            w->timer = 0;
                            w->state = 4;
                        } else {
                            gGameWork.scenarioRunning = 1;
                            deshiTalkWaitSet(w->deshi);
                            demoFinish(w->deshi);
                            demoPlReleaseAll();
                            camSetSpeed(w->cam, 40 / gRetracesPerFrame);
                            camMoveFov(w->cam, 0.6981317401f);
                            camMoveEye(w->cam, 1.0f, 247.0f, 185.0f);
                            camMoveAt(w->cam, 1.0f, 118.0f, 103.0f);
                            w->state = 11;
                        }
                    }
                    break;
                case 4:
                    if (w->timer == 1) {
                        DPRINTF("demoHanabi_proc: セッティング\n");
                        camSetSpeed(w->cam, 30 / gRetracesPerFrame);
                        camMoveFov(w->cam, 0.6981317401f);
                        camMoveEye(w->cam, -4.0f, 135.0f, 45.0f);
                        camMoveAt(w->cam, -4.0f, 0.0f, -41.0f);
                        demoPlReleaseAll();
                        deshiSetPos(w->deshi, 55.0f, 0.0f, -69.0f, 5.497786999f);
                        demoSetPlPos(0, -4.0f, 0.0f, -8.0f, 3.1415927f, 0, -27.0f, 0.0f, -8.0f, 3.1415927f, 0,
                                     -19.0f, 0.0f, -10.0f, 3.1415927f, 0, -49.0f, 0.0f, -4.0f, 3.1415927f);
                        demoPlStandAll();
                        demoHanabiMakeJizou();
                    }
                    if (w->timer == 30 / gRetracesPerFrame) {
                        w->state = 5;
                    }
                    camGetView(w->cam)->roll += (360 / (30 / gRetracesPerFrame)) * 3.1415927f / 180.0f;
                    break;
                case 9:
                    DPRINTF("町から戻って来た 正解\n");
                    w->deshi = eneMakeEx(gEneManager->child->next->next->next, ENE_HANADESHI, 0, 55.0f, 0.0f, -69.0f, 2,
                                         (void *) w->data);
                    eneSetAngle(w->deshi, 315.0f);
                    deshiSetDemo(w->deshi, obj);
                    camAiSetHold(1);
                    w->cam = camAiGetCam();
                    camSetFov(w->cam, 0.6981317401f);
                    camSetEye(w->cam, -4.0f, 135.0f, 45.0f);
                    camSetAt(w->cam, -4.0f, 0.0f, -41.0f);
                    demoSetPlPos(0, -4.0f, 0.0f, -8.0f, 3.1415927f, 0, -27.0f, 0.0f, -8.0f, 3.1415927f, 0, -19.0f,
                                 0.0f, -10.0f, 3.1415927f, 0, -49.0f, 0.0f, -4.0f, 3.1415927f);
                    demoPlStandAll();
                    demoHanabiMakeJizou();
                    scRunStart(0x22D, 0, 0);
                    w->state = 5;
                    break;
                case 10:
                    DPRINTF("もういっかいやるか？\n");
                    w->deshi = eneMakeEx(gEneManager->child->next->next->next, ENE_HANADESHI, 0, 3.0f, 0.0f, 18.0f, 2,
                                         (void *) w->data);
                    deshiSetDemo(w->deshi, obj);
                    camAiSetHold(1);
                    w->cam = camAiGetCam();
                    camSetFov(w->cam, 0.1981319911f);
                    camSetEye(w->cam, 57.0f, 168.0f, 267.0f);
                    camSetAt(w->cam, 24.0f, 81.0f, 130.0f);
                    r = 2.094395161f;
                    demoSetPlPos(0, 0.0f, 0.0f, -8.0f, 3.141592741f, 0, 10.0f, 0.0f, 66.0f, 1.745329261f, 0, 3.0f,
                                 0.0f, 73.0f, r, 0, 13.0f, 0.0f, 81.0f, r);
                    demoPlStandAll();
                    gGameWork.hanabiQNo = 0;
                    hanabiSeqClear();
                    scRunStart(0x22C, 0, 0);
                    w->timer = 0;
                    w->state = 3;
                    break;
                case 5:
                    H_GAME_FIRE_CHECK(obj);
                    break;
                case 6:
                    DPRINTF("オメエ最高だ！\n");
                    gGameWork.scenarioRunning = 0;
                    w->deshi = eneMakeEx(gEneManager->child->next->next->next, ENE_HANADESHI, 0, -24.0f, 0.0f, 18.0f, 2,
                                         (void *) w->data);
                    if (w->deshi == NULL) {
                        return -1;
                    }
                    deshiSetPos(w->deshi, -24.0f, 0.0f, 18.0f, 0.4049710035f);
                    w->oya = eneMakeEx(gEneManager->child->next->next->next, ENE_HANAOYA, 0, -4.0f, 0.0f, 18.0f, 2,
                                       (void *) w->data);
                    if (w->oya == NULL) {
                        return -1;
                    }
                    hanaoyaSetState12(w->oya);
                    camAiSetHold(1);
                    w->cam = camAiGetCam();
                    camSetFov(w->cam, 0.1981319911f);
                    camSetEye(w->cam, 59.0f, 168.0f, 267.0f);
                    camSetAt(w->cam, 24.0f, 81.0f, 130.0f);
                    r = 2.094395161f;
                    demoSetPlPos(0, 1.0f, 0.0f, 38.0f, 3.141592741f, 0, -13.0f, 0.0f, 31.0f, 1.745329261f, 0, 3.0f,
                                 0.0f, 73.0f, r, 0, 13.0f, 0.0f, 81.0f, r);
                    demoPlStandAll();
                    scRunStart(0x22E, 0, 0);
                    w->timer = 0;
                    w->state = 7;
                    break;
                case 7:
                    if (!scIsRunning()) {
                        gGameWork.scenarioRunning = 2;
                        hanaoyaSetState6(w->oya);
                        w->state = 8;
                    }
                    break;
                case 8:
                    if (hanaoyaIsState12(w->oya)) {
                        sndPlay(10);
                        w->timer = 0;
                        w->state = 12;
                    }
                    break;
                case 12:
                    if (w->timer == 1) {
                        gGameWork.mission = 0;
                        fadeSetAll(16 / gRetracesPerFrame, 0.0f, 255.0f, 0, 0, 0);
                    }
                    if (w->timer == 20 / gRetracesPerFrame) {
                        DPRINTF("stageGoPosRyPA HANABI_MISSION_END\n");
                        swWorkSet(0x92);
                        stageGotoSpawn(gStageTask->parent, 0xCF, 0, -21.0f, 10.0f, 14.0f, 3.839724302f, 0xD7);
                    }
                    break;
            }
            if (gGameWork.mission == 2 && w->state != 12) {
                DPRINTF("%%%%   event manager: PLAYER 死にました信号をうけとったぜ！\n");
                sndPlay(10);
                demoPlStandAll();
                w->timer = 0;
                w->state = 12;
            }
            eneStepHookPost(obj);
            w->timer++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* demohanabi: state 1. */
void demoHanabiSetState1(Obj *o) {
    HANA_STATE(o)->state = 1;
}


void hidutuSetState1(Obj *o);
void hidutuSetState0(Obj *o);
void demoHanabiHidutuStopAll(Obj *o);
void hanabiSeqClear(void);
void demoHanabiHidutuSet(Obj *o, u8 n);
void deshiOcsGo(Obj *o);
s32 deshiIsState0(Obj *o);
extern u8 fireworkSeqLengths[]; /* each sequence's length */

/* demohanabi's show: the apprentice plays a sequence of mortars (phases 1-3); flag 0x95 set
 * by the scenario means the player got it right (4: the players are let go), else the show
 * starts again; 5 waits for the whole sequence, 7 fades out and goes to stage 0x47. */
void H_GAME_FIRE_CHECK(Obj *o) {
    CamView *v;
    DemohanabiW *w = (DemohanabiW *) FX(o);
    s32 i;
    s32 pad[2];

    switch (w->phase) {
        case 0:
            if (!scIsRunning()) {
                DPRINTF("HANABI_INIT いきます\n");
                gGameWork.scenarioRunning = 0;
                hanabiSeqClear();
                gGameWork.hanabiCueNo = 0;
                demoHanabiHidutuSet(o, gGameWork.hanabiQNo);
                w->timer = 0;
                w->phase = 1;
            }
            break;
        case 1:
            if (w->timer >= 40 / gRetracesPerFrame) {
                gGameWork.hanabiShows++;
                deshiOcsGo(w->deshi);
                w->phase = 2;
            }
            break;
        case 2:
            if (deshiIsState0(w->deshi)) {
                scRunStart(0x22A, 0, 0);
                w->phase = 3;
            }
            break;
        case 3:
            if (!scIsRunning()) {
                if (swWorkTest(0x95)) {
                    swWorkReset(0x95);
                    w->timer = 0;
                    w->phase = 4;
                } else {
                    demoHanabiHidutuStopAll(o);
                    w->timer = 0;
                    w->phase = 1;
                }
            }
            break;
        case 4:
            gGameWork.hanabiShows = 0;
            demoHanabiHidutuStopAll(o);
            demoPlReleaseAll();
            for (i = 0; i < 4; i++) {
                if (plIsInPlay(i)) {
                    PL(gGameWork.players[i])->flags &= ~0x40000000;
                }
            }
            gGameWork.partake = w->partake;
            w->phase = 5;
            break;
        case 5:
            DPRINTF("H_GAME_FIRE_CHECK:全部火を付けた？ [%d]\n");
            if (gGameWork.hanabiCueNo >= fireworkSeqLengths[gGameWork.hanabiQNo]) {
                w->timer = 0;
                w->phase = 7;
            }
            break;
        case 7:
            if (w->timer == 80 / gRetracesPerFrame) {
                gGameWork.scenarioRunning = 2;
                fadeSetAll(16 / gRetracesPerFrame, 0.0f, 255.0f, 0, 0, 0);
                v = camGetView(w->cam);
                camSetSpeed(w->cam, 10.0f);
                camMoveAtY(w->cam, v->at[1] + 50.0f);
                sndPlay(10);
            }
            if (100 / gRetracesPerFrame < w->timer) {
                DPRINTF("stageGoPosRyPA H_GAME_HANBI_DEMO\n");
                gGameWork.scenarioRunning = 0;
                swWorkSet(0x1A);
                stageGotoSpawn(gStageTask->parent, 0x47, 0, -560.0f, 0.0f, 46.0f, 3.141592741f, 0xD7);
            }
            break;
    }
}

/* Fires mortar n (1 to 8): sounds 0x163 and 0x2E1, and the mortar goes to state 1. The game
 * keeps n * 4 across the two sndPlay calls and reloads o from its home. FAKE: o's address is
 * taken in a dead test so it lives in its home; without it IDO keeps o + n * 4 instead. */
void demoHanabiHidutuFire(Obj *o, u8 n) {
    s32 pad; /* FAKE: the game's frame word */

    if (&o) {} /* FAKE: o's address taken keeps it in its home, reloaded after the calls */
    if (((DemohanabiW *) FX(o))->hidutu[n - 1] != NULL) {
        sndPlay(0x163);
        sndPlay(0x2E1);
        hidutuSetState1(((DemohanabiW *) FX(o))->hidutu[n - 1]);
    }
}

/* Stops every mortar (state 0) and plays sound 0x8163. */
void demoHanabiHidutuStopAll(Obj *o) {
    DemohanabiW *w = (DemohanabiW *) FX(o);
    s32 i;

    for (i = 0; i < 8; i++) {
        if (w->hidutu[i] != NULL) {
            hidutuSetState0(w->hidutu[i]);
        }
    }
    sndPlay(0x8163);
}

void hanaUnusedStub5(void) {
}

extern u8 fireworkSeqLengths[]; /* each sequence's length */
extern u8 fireworkSeqs[][9]; /* the firework sequences, each ending in 0 */

/* Whether the cue the player lit (gGameWork.hanabiCue) holds sequence n. */
s32 hanabiSeqCheck(u8 n) {
    s32 i;

    for (i = 0; fireworkSeqs[n][i] != 0; i++) {
        if (gGameWork.hanabiCue[i] != fireworkSeqs[n][i]) {
            return 0;
        }
    }
    return 1;
}

/* Whether the last step of sequence n is right and the one before it is not. */
s32 hanabiSeqCheckLast(u8 n) {
    s32 len = fireworkSeqLengths[n];

    if (gGameWork.hanabiCue[len - 2] != fireworkSeqs[n][len - 2]
        && gGameWork.hanabiCue[len - 1] == fireworkSeqs[n][len - 1]) {
        DPRINTF("*****   最後から２本目[%d本目]失敗、１本目[%d本目]成功\n");
        return 1;
    }
    DPRINTF("*****   最後から２本目[%d本目]成功、１本目[%d本目]成功\n");
    return 0;
}

/* Clears the cue's first 8 (gGameWork.hanabiCue[0..7]). */
void hanabiSeqClear(void) {
    s32 i;

    for (i = 0; i < 8; i++) {
        gGameWork.hanabiCue[i] = 0;
    }
}

extern f32 mortarPlaces[][9][3]; /* each layout's mortar places (all 0: no mortar) */

void hidutuSetId(Obj *o, u8 v);

/* Makes demohanabi's eight mortars (enemy 0x178, numbered 1 to 8) at layout n's places, under
 * the enemy manager's fourth group; a place of 0, 0, 0 has none. Stops if one cannot be made. */
void demoHanabiHidutuSet(Obj *o, u8 n) {
    DemohanabiW *w = (DemohanabiW *) FX(o);
    s32 i;
    Obj *m;

    for (i = 0; i < 8; i++) {
        if (mortarPlaces[n][i][0] == 0.0f && mortarPlaces[n][i][1] == 0.0f &&
            mortarPlaces[n][i][2] == 0.0f) {
            w->hidutu[i] = NULL;
        } else {
            m = eneMakeEx(gEneManager->child->next->next->next, ENE_HIDUTU, 0, mortarPlaces[n][i][0],
                          mortarPlaces[n][i][1], mortarPlaces[n][i][2], 0, NULL);
            w->hidutu[i] = m;
            if (m == NULL) {
                DPRINTF("火筒セット id[%d] 失敗\n");
                return;
            }
            hidutuSetId(w->hidutu[i], i + 1);
            DPRINTF("火筒セット id[%d] x[%f] y[%f] z[%f]\n");
        }
    }
}

/* acollDmgAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*DmgAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 ox, s16 oy,
                                     s16 oz, s32 a);

extern Gfx hidutuMortarDl[]; /* the mortar's model */
extern Gfx hidutuDl[];

/* "hidutu": a fireworks mortar. Hit (state 0), it sounds, writes its number into the next
 * place of gGameWork's sequence (hanabiCue, counted by hanabiCueNo) and fires (state 1): a flash
 * every 20 frames at its mouth, its shadow flickering. */
s32 hidutu_proc(Obj *obj, s16 msg, void *arg) {
    HidutuW *w = HIDUTU(obj);
    PolFrame *m;

    switch (msg) {
        case 2:
            obj->name = "hidutu";
            eneSetup(obj, arg);
            w->model = eneNewPol(obj, hidutuMortarDl);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[2] = 0.150000006f;
            m = w->model;
            w->model->scale[0] = m->scale[1] = m->scale[2];
            if (eneSetOnEarth(obj) == -1) {
                DPRINTF("hidutu_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->dmg = ((DmgAddXYZFunc) acollDmgAddXYZ)(ene_dmg_root_frame, &w->model->f, 50, 40, 0, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->shadow = shadowCreate();
            w->rot[0] -= 0.5235987902f;
            w->dl = fadeSegToRam(hidutuDl, obj->file);
            break;
        case 1:
            m = w->model;
            if (m != NULL) {
                frmDestroy(&m->f);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(obj);
            switch (w->state) {
                case 0:
                    if (acollDmgWasHit(w->dmg) == 1) {
                        sndPlay(0x163);
                        sndPlay(0x2E1);
                        DPRINTF("hanabi: キューNo[%d] にid[%d] 点火\n", gGameWork.hanabiCueNo, w->id);
                        gGameWork.hanabiCue[gGameWork.hanabiCueNo] = w->id;
                        gGameWork.hanabiCueNo++;
                        w->timer = w->id;
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (w->timer % (20 / gRetracesPerFrame) == 0) {
                        m = w->model;
                        itexFlash(m->pos[0] - 3.0f, m->pos[1] + 15.0f, m->pos[2] - 10.0f, 0.03f);
                    }
                    break;
            }
            eneMove((EneObj *) w);
            m = w->model;
            if (m != NULL) {
                eneSetModelOnCourse(obj, &m->f);
                m = w->model;
            }
            if (w->state == 1) {
                if (w->timer % ((w->id + 20) / gRetracesPerFrame) == 0) {
                    shadowPut(w->shadow, m->pos[0], m->pos[1], m->pos[2], 0.8999999762f, NULL);
                } else {
                    shadowPut(w->shadow, m->pos[0], m->pos[1], m->pos[2], 1.0f, NULL);
                }
            } else {
                shadowPut(w->shadow, m->pos[0], m->pos[1], m->pos[2], 1.0f, NULL);
            }
            eneStepHookPost(obj);
            w->timer++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* hidutu: state 1. */
void hidutuSetState1(Obj *o) {
    HANA_STATE(o)->state = 1;
}

/* hidutu: state 0. */
void hidutuSetState0(Obj *o) {
    HANA_STATE(o)->state = 0;
}

void hidutuSetId(Obj *o, u8 v) {
    HIDUTU(o)->id = v;
}

extern u32 hanaoyaAnim; /* its animation */

/* acollAtkAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy, s16 oz, s32 a);
/* mdlAnimNodePos/Rot take the frame as a float here, as ombupl.c and pldive.c call them. */
typedef void (*AnimNodeFunc)(u32 addr, f32 frame, u32 base, f32 *x, f32 *y, f32 *z);
typedef void (*AnimNodeRotFunc)(u8 *addr, f32 frame, u32 base, f32 *x, f32 *y, f32 *z);

extern u8 hanaoyaMasterAnim[]; /* the master's model */
extern u8 hidutuDlEnd[]; /* the end of the hidutu's display list (file set 0x8C): hanaoyaMasterAnim's address in another file */
extern u8 hanaoyaPassAttach[]; /* the animation whose node holds the pass */
extern Gfx hanaoyaPassDl[]; /* the pass, in file 0x3D */

/* "hanaoya": the fireworks master, a talker, out at night only (gGameWork.time outside
 * 0x40001-0x13FFFF). Talked to (0-3), he asks; agreed (flag 0x94), the players go to the
 * square (4). After the show (10, flag 0x92) he hands over the pass ("tegata", 6-9): the pass
 * follows his hand's node, then becomes enemy 0x1C1 and the scenario 0x20 runs.
 * Matched 2026-10-12; it was parked for state 7's float temps and state 2's spill one word high.
 * State 7's sums are written product first, "x * 0.1 + ((Vec3f *) pos)->x" (IDO_NOTES 273), and
 * its scale goes through sc, a local declared last, which puts plGetIndex's temp at the game's
 * 0x58. */
s32 hanaoya_proc(Obj *obj, s16 msg, void *arg) {
    HanaoyaW *w = (HanaoyaW *) FX(obj);
    s32 pad1;
    u32 data;
    CamView *v;
    s32 pad2;
    f32 pos[3];
    s32 i;
    f32 pos2[3];
    f32 x;
    f32 y;
    f32 z;
    f32 rx;
    f32 ry;
    f32 rz;
    u32 base;
    s32 pad3;
    f32 pp[3];
    s32 other;
    f32 q[3];
    s32 chara;
    u16 se;
    f32 sc;

    switch (msg) {
        case 2:
            obj->name = "hanaoya";
            if (gGameWork.time >= 0x140000 || gGameWork.time <= 0x40000) {
                DPRINTF(">>>>>>  花火職人発生\n");
            } else {
                DPRINTF(">>>>>>  花火職人発生しません\n");
                return -1;
            }
            data = eneSetup(obj, arg);
            w->anim = eneNewAnime(obj, (u32) hanaoyaMasterAnim);
            if (w->anim == NULL) {
                return -1;
            }
            w->atk = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->anim->f, 100, 0, 100, 0, 0);
            demoTalkInit(obj, data);
            if (eneSetOnEarth(obj) == -1) {
                DPRINTF("Etsk_tsuchikorobi_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            if (swWorkTest(0x92)) {
                gGameWork.scenarioRunning = 2;
                swWorkReset(0x92);
                camSunSetClockStop(1);
                w->state = 10;
            }
            break;
        case 8:
            return demoTalk(obj, arg);
        case 1:
            demoTalkerFree(obj);
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            if (w->pass != NULL) {
                frmDestroy(&w->pass->f);
            }
            break;
        case 0:
            eneStepHookPre(obj);
            switch (w->state) {
                case 0:
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i) && !(gGameWork.players[i]->flags & 2)) {
                            tskSendMsg(gGameWork.players[i], 4, pos);
                            eneTurnToPoint(obj, pos[0], pos[2], 10.0f);
                            break;
                        }
                    }
                    if (demoTalkEndCheck(obj)) {
                        personCamZoomUp(w->player);
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (w->player != NULL) {
                        tskSendMsg(w->player, 4, pos2);
                        if (eneTurnToPoint(obj, pos2[0], pos2[2], 18.0f) == 1) {
                            w->state = 2;
                        }
                    }
                    break;
                case 2:
                    scRunStart(0x228, plGetIndex(w->player) + 1, plGetChara(w->player));
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
                            demoFinish(obj);
                            w->state = 0;
                        }
                    }
                    break;
                case 4:
                    if (w->timer == 1) {
                        sndPlay(10);
                        fadeSetAll(16 / gRetracesPerFrame, 0.0f, 255.0f, 0, 0, 0);
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        v = camGetView(w->cam);
                        camSetSpeed(w->cam, 10.0f);
                        camMoveAtY(w->cam, v->at[1] + 50.0f);
                    }
                    if (w->timer == 20 / gRetracesPerFrame) {
                        DPRINTF("stageGoPosRyPA OYA_HIROBA_GO\n");
                        gGameWork.scenarioRunning = 0;
                        camAiSetHold(0);
                        stageGotoSpawn(gStageTask->parent, 0x4D, 0, 1.0f, 0.0f, 38.0f, 3.141592741f, 0xD7);
                    }
                    break;
                case 6:
                    if (3.0f < w->anim->frame) {
                        DPRINTF("*****   手形セットステップに\n");
                        w->timer = 0;
                        w->state = 7;
                    }
                    break;
                case 7:
                    w->anim->anim = hanaoyaAnim;
                    base = fileRamAddr(floadGetSetFile(0x8A, 0));
                    ((AnimNodeFunc) mdlAnimNodePos)((u32) hanaoyaPassAttach, w->anim->frame, base, &x, &y, &z);
                    ((AnimNodeRotFunc) mdlAnimNodeRot)(hanaoyaPassAttach, w->anim->frame, base, &rx, &ry, &rz);
                    if (w->timer == 1) {
                        w->pass = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
                        if (w->pass == NULL) {
                            return -1;
                        }
                        w->pass->scale[2] = 0.05000000075f;
                        sc = w->pass->scale[2];
                        w->pass->scale[1] = sc;
                        w->pass->scale[0] = sc;
                        w->pass->dl2 = hanaoyaPassDl;
                        w->pass->flags |= 0x20;
                        floadGetSetRam((void **) w->pass->segs, 0x3D);
                    }
                    DPRINTF("&&& x[%f] y[%f] z[%f]\n");
                    w->pass->pos[0] = x * 0.1 + ((Vec3f *) w->anim->pos)->x;
                    w->pass->pos[1] = y * 0.1 + ((Vec3f *) w->anim->pos)->y;
                    w->pass->pos[2] = z * 0.1 + ((Vec3f *) w->anim->pos)->z;
                    w->pass->rot[0] = rx + 1.570796371f;
                    w->pass->rot[1] = ry;
                    w->pass->rot[2] = rz + 3.141592741f;
                    if (w->flags & 2) {
                        eneMakeEx(NULL, ENE_DEMO_KIRAKIRA, 0, w->pass->pos[0], w->pass->pos[1], w->pass->pos[2], 0, NULL);
                        sndPlay(0xB8);
                        soundIncamCam(0x233, w->pass->pos[0], w->pass->pos[1], w->pass->pos[2]);
                        if (w->pass != NULL) {
                            w->pass = (PolFrame *) frmDestroy(&w->pass->f);
                        }
                        w->timer = 0;
                        w->state = 8;
                    }
                    break;
                case 8:
                    if (w->timer == 36.0 / gRetracesPerFrame) {
                        plGetModelPos(gGameWork.players[demoFindPlByChara(2)], &pp[0], &pp[1], &pp[2]);
                        soundIncamCam(0x170, pp[0], pp[1], pp[2]);
                    }
                    if (w->timer == 66.0 / gRetracesPerFrame) {
                        other = (demoFindPlByChara(2) == 0) ? 1 : 0;
                        if (gGameWork.players[other] == NULL) {
                            DPRINTF("&&&& tegataGet 他人はいない\n");
                            break;
                        }
                        chara = plGetChara(gGameWork.players[other]);
                        plGetModelPos(gGameWork.players[other], &q[0], &q[1], &q[2]);
                        switch (chara) {
                            case 0:
                                DPRINTF("&&&& tegataGet SD_G_CLEAR\n");
                                se = 0x16F;
                                break;
                            case 1:
                                DPRINTF("&&&& tegataGet  SD_E_CLEAR\n");
                                se = 0x16E;
                                break;
                            case 2:
                                DPRINTF("&&&& tegataGet  SD_S_CLEAR\n");
                                se = 0x170;
                                break;
                            case 3:
                                DPRINTF("&&&& tegataGet  SD_Y_CLEAR\n");
                                se = 0x171;
                                break;
                        }
                        soundIncamCam(se, q[0], q[1], q[2]);
                    }
                    if (w->timer == 96.0 / gRetracesPerFrame) {
                        scRunStart(0x20, 0, 0);
                        w->state = 9;
                    }
                    break;
                case 9:
                    if (!scIsRunning()) {
                        w->state = 12;
                    }
                    break;
                case 10:
                    if (w->timer == 1) {
                        demoPlStandAll();
                    }
                    if (w->timer == 8) {
                        camSunClearTunes();
                        camSunSetTunes4(0x84, 0x81, 0x82, 0x83);
                        personCamZoomUp(plCheck());
                        if (swWorkTest(0xA5)) {
                            scRunStart(0x228, 0, 0);
                            w->state = 11;
                        } else {
                            gGameWork.hanabiQNo = 0;
                            hanabiSeqClear();
                            swWorkReset(0x1A);
                            scRunStart(0x22C, 0, 0);
                            w->state = 11;
                        }
                    }
                    break;
                case 11:
                    if (!scIsRunning()) {
                        personTalkCamOff();
                        demoPlReleaseAll();
                        w->state = 0;
                    }
                    break;
                case 12:
                    break;
            }
            eneFall(obj, 0.4f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->anim != NULL) {
                eneSetModelOnCourse(obj, &w->anim->f);
            }
            if (w->flags & 1) {
                if (eneAnimPlayOnce(obj, w->anim, 0.1f)) {
                    w->flags |= 2;
                }
            } else {
                eneAnimLoop(obj, w->anim, 0.5f);
            }
            shadowPut(w->shadow, w->anim->pos[0], w->anim->pos[1], w->anim->pos[2], 1.0f, NULL);
            eneStepHookPost(obj);
            w->timer++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Starts its animation from the first frame; state 6. */
void hanaoyaSetState6(Obj *o) {
    HanaoyaW *w = (HanaoyaW *) FX(o);

    w->flags &= ~2;
    w->flags |= 1;
    w->anim->anim = hanaoyaAnim;
    w->anim->frame = 0.0f;
    w->timer = 0;
    w->state = 6;
}

/* hanaoya: whether it is in state 12. */
s32 hanaoyaIsState12(Obj *o) {
    if (HANA_STATE(o)->state == 12) {
        return 1;
    }
    return 0;
}

void hanaUnusedStub6(void) {
}

/* hanaoya: state 12. */
void hanaoyaSetState12(Obj *o) {
    HANA_STATE(o)->state = 12;
}

/* acollAtkAddYZ returns its box (main declares it void), as tlb_0039 calls it. */
typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);

extern u8 hanadeshiAnim[]; /* the apprentice's model, in file 0x8D */
extern void *hanadeshiAnims[]; /* the apprentice's animations (also its person table) */

s16 *deshiActStep(Obj *o, u8 n);

/* "hanadeshi": the fireworks apprentice, a talker. State 1 waits for the talk to end and lets
 * the players stand, 2 turns to its target and starts demohanabi's show, 3 runs the show
 * script for show gGameWork.hanabiQNo until its end (-1). */
s32 hanadeshi_proc(Obj *obj, s16 msg, void *arg) {
    HanadeshiW *w = HANADESHI(obj);
    u32 data;
    s32 pad;
    f32 pos[3];

    switch (msg) {
        case 2:
            obj->name = "hanadeshi";
            data = eneSetup(obj, arg);
            w->model = eneNewAnime(obj, (u32) hanadeshiAnim);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(obj) == -1) {
                DPRINTF("Etsk_tsuchikorobi_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 100, 100, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x66;
            demoTalkInit(obj, data);
            break;
        case 8:
            return demoTalk(obj, arg);
        case 1:
            demoTalkerFree(obj);
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(obj);
            switch (w->state) {
                case 0:
                    break;
                case 1:
                    if (demoTalkEndCheck(obj)) {
                        demoPlStandAll();
                        w->state = 2;
                    } else {
                        eneTurnToAngle(obj, 0.0f, 5.0f);
                    }
                    break;
                case 2:
                    if (w->target != NULL) {
                        tskSendMsg(w->target, 4, pos);
                        if (eneTurnToPoint(obj, pos[0], pos[2], 10.0f) == 1) {
                            demoHanabiSetState1(w->demo);
                            w->state = 0;
                        }
                    }
                    break;
                case 3:
                    if (*deshiActStep(obj, gGameWork.hanabiQNo) == -1) {
                        w->state = 0;
                    }
                    break;
            }
            eneFall(obj, 0.4f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(obj, &w->model->f);
            }
            if (w->model->anim == hanadeshiAnims[1]) {
                w->speed = 1.0f;
            } else {
                w->speed = 0.5f;
            }
            if (w->flags & 4) {
                if (w->flags & 1) {
                    eneAnimPlayOnce(obj, w->model, w->speed * -1.0f);
                } else {
                    eneAnimLoop(obj, w->model, w->speed * -1.0f);
                }
            } else if (w->flags & 1) {
                eneAnimPlayOnce(obj, w->model, w->speed);
            } else {
                eneAnimLoop(obj, w->model, w->speed);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

void deshiSetPos(Obj *o, f32 a, f32 b, f32 c, f32 d) {
    HANADESHI(o)->x = a;
    HANADESHI(o)->y = b;
    HANADESHI(o)->z = c;
    HANADESHI(o)->rotY = d;
}

/* hanadeshi: takes the show entry for gGameWork's byte 0xB9; state 3. */
void deshiOcsGo(Obj *o) {
    DPRINTF("deshiOcsGo: 行動セット\n");
    HANADESHI(o)->show = hanadeshiShows[gGameWork.hanabiQNo];
    HANADESHI(o)->state = 3;
}

/* "deshiTalkWaitSet": state 1, waiting for the talk. */
void deshiTalkWaitSet(Obj *o) {
    DPRINTF("deshiTalkWaitSet: 会話待ち状態セット\n");
    HANADESHI(o)->state = 1;
}

s32 deshiIsState0(Obj *o) {
    if (HANADESHI(o)->state == 0) {
        return 1;
    }
    return 0;
}

/* hanadeshi: the demohanabi whose mortars it fires. */
void deshiSetDemo(Obj *o, Obj *demo) {
    HANADESHI(o)->demo = demo;
}


void demoHanabiHidutuFire(Obj *o, u8 n);

/* hanadeshi: runs one step of its show script on show n, and returns where the script is.
 * 0: walk to (x, z) at a speed, facing it; 1: go to word k of show n; 2: turn to an angle;
 * 3: stop; 4: stand for a time; 5: a sound at it; 6: stand still; 7: an animation (which,
 * once, waiting for its end, backwards); 8: fire mortar k. */
s16 *deshiActStep(Obj *o, u8 n) {
    HanadeshiW *w = HANADESHI(o);
    f32 speed;
    f32 x;
    f32 z;
    s32 time;
    s32 se;
    s32 k;
    u8 anim;
    u8 once;
    u8 wait;
    u8 reverse;

    switch (*(u16 *) w->show) {
        case 0:
            x = w->show[1];
            z = w->show[2];
            speed = w->show[3];
            if (eneMoveToPos(o, x, z, speed) == 1) {
                w->show += 4;
            }
            eneTurnToPoint(o, x, z, 13.0f);
            break;
        case 1:
            w->show = hanadeshiShows[n] + w->show[1];
            break;
        case 2:
            speed = w->show[1];
            w->vel[0] = 0.0f;
            w->vel[2] = 0.0f;
            if (eneTurnToAngle(o, speed, 13.0f) == 1) {
                w->show += 2;
            }
            break;
        case 7:
            anim = w->show[1];
            once = w->show[2];
            wait = w->show[3];
            reverse = w->show[4];
            if (w->timer == 0) {
                w->model->anim = hanadeshiAnims[anim];
                if (reverse == 0) {
                    w->flags &= ~4;
                    w->model->frame = 0.0f;
                } else {
                    DPRINTF("reverse!\n");
                    w->flags |= 4;
                    w->model->frame = polAnimeLastFrame(w->model);
                }
                if (once == 1) {
                    w->flags |= 1;
                } else {
                    w->flags &= ~1;
                }
                w->timer++;
            }
            if (wait == 1) {
                if (reverse == 0) {
                    if (w->model->frame < polAnimeLastFrame(w->model)) {
                        break;
                    }
                } else if (0.0f < w->model->frame) {
                    break;
                }
            }
            w->timer = 0;
            w->show += 5;
            break;
        case 4:
            time = w->show[1];
            DPRINTF("dddddd[%d]\n", time);
            if (w->timer == 0) {
                w->model->anim = hanadeshiAnims[0];
                w->model->frame = 0.0f;
                w->vel[0] = 0.0f;
                w->vel[2] = 0.0f;
            }
            if (time < w->timer++) {
                w->timer = 0;
                w->show += 2;
            }
            break;
        case 5:
            se = w->show[1];
            DPRINTF("SoundLocation [%f] [%f] [%f]\n", w->x, w->y, w->z);
            soundLocationCam(se, w->x, w->y, w->z);
            w->show += 2;
            break;
        case 6:
            w->vel[0] = w->vel[1] = w->vel[2] = 0.0f;
            w->show += 1;
            break;
        case 3:
            DPRINTF("ＯＣＳおわりです。\n");
            goto end;
        case 8:
            k = w->show[1];
            DPRINTF("id[%d] に火筒に点火\n", k);
            demoHanabiHidutuFire(w->demo, k);
            w->show += 2;
            break;
        default:
            DPRINTF("actStep がへんです\n");
        end:
            break;
    }
    return w->show;
}

extern Gfx hanabiHirobaDl[]; /* the square's model */
extern Frame *gFrmLayer3;

/* "hanabiHiroba_main": the fireworks square's model, an enemy at 0.8 scale on layer 3. The
 * work pointer serves cases 2 and 1; case 0 reads through ENE(o). */
s32 hanabiHiroba_main(Obj *o, s16 msg, void *arg) {
    EneObj *w = ENE(o);

    switch (msg) {
        case 2:
            o->name = "hanabiHiroba_main";
            eneSetup(o, arg);
            w->model = (Frame *) eneNewPol(o, hanabiHirobaDl);
            if (w->model == NULL) {
                return -1;
            }
            ((PolFrame *) w->model)->scale[2] = 0.8f;
            ((PolFrame *) w->model)->scale[0] = ((PolFrame *) w->model)->scale[1] =
                ((PolFrame *) w->model)->scale[2];
            frmMove(w->model, gFrmLayer3);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(w->model);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneSetModel(o, ENE(o)->model);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "hanaexit"'s work: the exit's model and its glow, which pulses. */
typedef struct HanaexitW {
    /* 0x00 */ PolFrame *door;
    /* 0x04 */ PolFrame *glow;
    /* 0x08 */ u8 alpha; /* the glow's */
    /* 0x09 */ u8 fading; /* 0: fading out, 1: in */
} HanaexitW;

extern Gfx hanaexitDl[]; /* the exit's model, in file 0x8D */
extern Gfx hanaexitGlowDl[]; /* its glow */

/* "hanaexit": the square's exit, a model turning slowly and a glow on layer 8 whose alpha
 * swings between 16 and 160. */
s32 hanaexit_proc(Obj *obj, s16 msg, void *arg) {
    HanaexitW *w = FX(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "hanaexit";
            w->door = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->door == NULL) {
                return -1;
            }
            w->door->dl2 = hanaexitDl;
            floadGetSetRam((void **) w->door->segs, 0x8D);
            w->door->pos[0] = 3.0f;
            w->door->pos[1] = 20.0f;
            w->door->pos[2] = 87.0f;
            w->door->rot[0] = -1.570796371f;
            w->door->scale[2] = 2.0f;
            w->door->scale[0] = w->door->scale[1] = w->door->scale[2];
            w->glow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->glow == NULL) {
                return -1;
            }
            w->glow->dl2 = hanaexitGlowDl;
            floadGetSetRam((void **) w->glow->segs, 0x8D);
            w->glow->flags |= 7;
            w->glow->pos[0] = 1.0f;
            w->glow->pos[1] = 20.0f;
            w->glow->pos[2] = 82.0f;
            w->glow->scale[2] = 2.400000095f;
            w->glow->scale[0] = w->glow->scale[1] = w->glow->scale[2];
            w->alpha = 160;
            break;
        case 1:
            if (w->door != NULL) {
                frmDestroy(&w->door->f);
            }
            if (w->glow != NULL) {
                frmDestroy(&w->glow->f);
            }
            break;
        case 3:
            w->glow->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            switch (w->fading) {
                case 0:
                    w->alpha -= 4;
                    if (w->alpha < 16) {
                        w->fading = 1;
                    }
                    break;
                case 1:
                    w->alpha += 4;
                    if (w->alpha++ > 160) {
                        w->fading = 0;
                    }
                    break;
            }
            eneStepHookPre(obj);
            w->door->rot[2] += 0.2;
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

extern Obj * hanashotShellTask; /* hanashot, where the shells go */
extern Obj * reversalTask; /* the reversal running (0: none) */
extern Obj * hanashotTask; /* hanashot */
extern f32 shellFirePlaces[][3]; /* where each shell is fired from */
extern u8 shellColors[][3]; /* each shell's colour (and the sky's at its burst) */

Obj *hanabiFireCreate(Obj *obj, f32 x, f32 y, f32 z, s32 a, s32 b, s32 c, u8 d, f32 e);
s32 hanabiFireIsState7(Obj *o);
void hanabiFireAskFail(Obj *o);
void hanabiFireAskFailClear(Obj *o);

/* "hanashot": the fireworks show after the player lit a sequence (flag 0x1A). It fires one
 * shell per step of sequence gGameWork.hanabiQNo (1, 2), good ones where the player was right;
 * then (5) all right on sequence 4 goes to the grand finale (6-8, eight good shells, 10), else
 * the players go back to the square or to stage 0x4D. Any player's button 0x1000 cancels.
 * Matched 2026-10-12 (the FAKE below). Was NON_MATCHING: 56 lines, mix distance 0, one call's schedule: in state 1's good shell the game
 * loads the three colour bytes (lbu t7, t9, a5) before any argument store, this C stores each
 * after its load. Measured: w set per case 911/662; x, y, z locals for the place 255; c (the
 * colour row) inside the branch 82 (the place and colour pointers swap v1/t0), c set before
 * the test 56; the three place loads on one line 56; f32 *p for the place 76; c after x, y, z
 * 84; "(f32) (q * 0.1 + 3.8)" 56; s32 r, g, b locals for the bytes 710 (every t-register two
 * higher); u8 r, g, b after the pads 764 (frame 0x80). 2026-10-11, no change (56): the
 * call broken after c[2]; the print before it joined to its line.
 * 2026-10-12 (real 10; state 7's call, the same form, matches with its loads first, so the
 * difference is state 1's own, perhaps its u32 conversion's bgez): c[] read as Color3 fields
 * 10; the if's line joined with the print 10; the colours as shellColors[q][n] in the call (c
 * gone) 16; c set inside the branch on the call's line 16; the call broken before c[0] 10.
 * 2026-10-12 (second try): "u8 r = c[0], g = c[1], b = c[2]" as block locals of the good branch,
 * passed as r, g, b: the call's order is then the game's (three lbu, then the conversion, d and
 * the stores), but the frame grows 8 (pad[4] -> pad[2] puts it back: 294) and every t-register
 * from case 2 on is two higher (r, g, b take t-registers as variables); the same as s32 at
 * block scope: frame +8 even with pad[2] (308). The game's three bytes are temps, not variables.
 * 2026-10-12 (third try): a probe (state 1 alone) reproduces this C's
 * interleaved lbu/sw, so the cause is local to the call. The permuter (fresh import, --stack-diffs,
 * ~300 iterations): nothing under its base score 430.
 * 2026-10-12 (fourth try, probe): in isolation the call groups its loads
 * (g0, g4); a test between c's set and the call interleaves them (g5); c set in the branch groups
 * them (g10), but in the whole function the place and colour pointers then swap v1/t0 in states 1
 * and 7 (16; also the colours read straight from the table, 16; c before x in state 7, 16; c
 * declared first, 16). c set both before the test and again in the branch: MATCH (FAKE). */
s32 hanashot_proc(Obj *obj, s16 msg, void *arg) {
    HanashotW *w = HANASHOT(obj);
    CamView *v;
    s32 i;
    u32 q;
    f32 x;
    f32 y;
    f32 z;
    u8 *c;
    s32 pad[4];

    switch (msg) {
        case 2:
            obj->name = "hanashot";
            if (!swWorkTest(0x1A)) {
                DPRINTF("花火イベントなし\n");
                return -1;
            }
            demoPlStandAll();
            camSunSetClockStop(1);
            camAiSetHold(1);
            w->cam = camAiGetCam();
            camSetEye(w->cam, -198.0f, 61.0f, 460.0f);
            camSetAt(w->cam, -169.0f, 111.0f, 311.0f);
            w->timer = 0;
            hanashotShellTask = obj;
            gGameWork.scenarioRunning = 2;
            w->partake = gGameWork.partake;
            gGameWork.partake = 1;
            sndPlay(0x288);
            fadeSetAll(120 / gRetracesPerFrame, 255.0f, 0.0f, 0, 0, 0);
            reversalTask = NULL;
            hanashotTask = obj;
            break;
        case 1:
            hanashotTask = NULL;
            hanashotShellTask = NULL;
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (w->timer == 100 / gRetracesPerFrame) {
                        w->timer = 0;
                        w->state = 1;
                    }
                    break;
                case 1:
                    q = w->qNo;
                    c = shellColors[q];
                    x = shellFirePlaces[q][0];
                    y = shellFirePlaces[q][1];
                    z = shellFirePlaces[q][2];
                    DPRINTF("&&&&  打ち上げ 初期値算出[%d]番目\n", q);
                    if (gGameWork.hanabiCue[q] == fireworkSeqs[gGameWork.hanabiQNo][q]) {
                        /* FAKE: c set again in the branch. The first set (above) gives c its
                         * register (v1); this one, in the call's block, makes the three colour
                         * loads table reads, which IDO schedules ahead of the argument stores. */
                        c = shellColors[q];
                        DPRINTF("&&&& 正解の打ち上げ！！[%d]番目\n", q);
                        w->fire =hanabiFireCreate(obj, x, y, z, c[0], c[1], c[2], 1, q * 0.1 + 3.8);
                        if (w->fire == NULL) {
                            DPRINTF("****  hanabiD is NULL\n");
                            return -1;
                        }
                    } else {
                        DPRINTF("**** 不正解の打ち上げ！！[%d]番目\n", q);
                        w->fire = hanabiFireCreate(obj, x, y, z, 0, 0, 0, 0, q * 0.1 + 3.8);
                        if (w->fire == NULL) {
                            DPRINTF("****  Error: hanabiD is NULL\n");
                            return -1;
                        }
                    }
                    w->wait = fadeRandRange(30, 60) / gRetracesPerFrame;
                    w->timer = 0;
                    w->state = 2;
                    DPRINTF("****>>>  L->randTime[%d]  待ちます\n", w->wait);
                    break;
                case 2:
                    if (120 / gRetracesPerFrame + w->wait < w->timer) {
                        DPRINTF("&&&&&  花火炸裂、周りも光る[%d]番目\n", w->qNo);
                        w->qNo++;
                        if (w->qNo >= fireworkSeqLengths[gGameWork.hanabiQNo]) {
                            DPRINTF(">>>>  打ち上げ終了\n");
                            w->timer = 0;
                            w->state = 4;
                        } else {
                            DPRINTF(">>>>  次いってみよう\n");
                            w->timer = 0;
                            w->state = 1;
                        }
                    }
                    break;
                case 4:
                    if (hanabiFireIsState7(w->fire)) {
                        w->timer = 0;
                        w->state = 5;
                    }
                    break;
                case 5:
                    if (w->timer == 1) {
                        for (i = 0; i < 4; i++) {
                            contPakInitRumble(i);
                            contPakRumbleRequestStop(i);
                        }
                        if (hanabiSeqCheckLast(gGameWork.hanabiQNo)) {
                            w->endTime = 300;
                        } else {
                            w->endTime = 80;
                        }
                    }
                    if (gGameWork.hanabiQNo == 4 && hanabiSeqCheck(gGameWork.hanabiQNo)) {
                        w->qNo = 0;
                        w->timer = 0;
                        w->state = 6;
                        break;
                    }
                    if (w->timer == w->endTime / gRetracesPerFrame) {
                        DPRINTF("********  end timing [%d]\n", w->timer);
                        fadeSetAll(16 / gRetracesPerFrame, 0.0f, 255.0f, 0, 0, 0);
                        sndPlay(10);
                        if (!hanabiSeqCheck(gGameWork.hanabiQNo)) {
                            sndPlay(0x45F);
                        }
                    }
                    if (w->timer >= (w->endTime + 16) / gRetracesPerFrame) {
                        gGameWork.scenarioRunning = 0;
                        gGameWork.partake = w->partake;
                        gGameWork.mission = 0;
                        if (!hanabiSeqCheck(gGameWork.hanabiQNo)) {
                            DPRINTF("stageGoPosRyPA HANABI_SHOT_GO_HIROBA\n");
                            gGameWork.areaStage = 3;
                            swWorkSet(0x92);
                            stageGotoSpawn(gStageTask->parent, 0xCF, 0, -21.0f, 10.0f, 14.0f, 3.839724302f, 0xD7);
                        } else {
                            DPRINTF("それ以外で全問正解ならばゲームの続き gw.hanabiQNo=[%d]\n", gGameWork.hanabiQNo);
                            stageGotoSpawn(gStageTask->parent, 0x4D, 0, -1.0f, 0.0f, -11.0f, 3.141592741f, 0xD7);
                        }
                    }
                    break;
                case 6:
                    if (120 / gRetracesPerFrame < w->timer) {
                        w->timer = 0;
                        w->state = 7;
                    }
                    break;
                case 7:
                    q = w->qNo;
                    x = shellFirePlaces[q][0];
                    y = shellFirePlaces[q][1];
                    z = shellFirePlaces[q][2];
                    c = shellColors[q];
                    DPRINTF("&&&&  打ち上げ 初期値算出[%d]番目\n", q);
                    w->fire = hanabiFireCreate(obj, x, y, z, c[0], c[1], c[2], 1, 4.0f);
                    if (w->fire == NULL) {
                        DPRINTF("****  hanabiD is NULL\n");
                        return -1;
                    }
                    if (w->qNo & 1) {
                        hanabiFireAskFail(w->fire);
                    } else {
                        hanabiFireAskFailClear(w->fire);
                    }
                    w->wait = fadeRandRange(1, 8) / gRetracesPerFrame;
                    w->timer = 0;
                    w->state = 8;
                    DPRINTF("****>>>  L->randTime[%d]  待ちます\n", w->wait);
                    break;
                case 8:
                    if (8 / gRetracesPerFrame + w->wait < w->timer) {
                        DPRINTF("&&&&&  花火炸裂、周りも光る[%d]番目\n", w->qNo);
                        w->timer = 0;
                        w->qNo++;
                        if (w->qNo >= 8) {
                            DPRINTF(">>>>  打ち上げ終了\n");
                            w->state = 10;
                        } else {
                            w->state = 7;
                        }
                    }
                    break;
                case 10:
                    if (w->timer == 600 / gRetracesPerFrame) {
                        for (i = 0; i < 4; i++) {
                            contPakInitRumble(i);
                            contPakRumbleRequestStop(i);
                        }
                        fadeSetAll(16 / gRetracesPerFrame, 0.0f, 255.0f, 0, 0, 0);
                        sndPlay(10);
                    }
                    if (w->timer >= 616 / gRetracesPerFrame) {
                        DPRINTF("全問題終了 && 全問正解ならば 広場の真中に gw.hanabiQNo=[%d]\n", gGameWork.hanabiQNo);
                        gGameWork.scenarioRunning = 0;
                        gGameWork.areaStage = 3;
                        stageGotoSpawn(gStageTask->parent, 0x4D, 0, -7.0f, 0.0f, 40.0f, 3.141592741f, 0xD7);
                    }
                    break;
            }
            camSunSetColor(16, 16, 16);
            v = camGetView(w->cam);
            if (reversalTask != NULL) {
                if (!(w->timer & 1)) {
                    v->at[1] += 2.0f;
                } else {
                    v->at[1] -= 2.0f;
                }
            } else {
                v->at[1] += 0.015 * gRetracesPerFrame;
                v->eye[1] += 0.015 * gRetracesPerFrame;
                v->eye[0] += 0.01 * gRetracesPerFrame;
            }
            for (i = 0; i < 4; i++) {
                if (plIsInPlay(i) == 1 && joyGetHeldTime(i, 0x1000) == 1 && w->state < 5) {
                    DPRINTF("[%d]P でキャンセルかかりました\n", i);
                    gGameWork.scenarioRunning = 2;
                    w->timer = 70 / gRetracesPerFrame;
                    w->state = 5;
                }
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* hanashot: keeps the camera's look-at height. */
void hanashotSaveAtY(Obj *o) {
    HANASHOT(o)->atY = camGetView(HANASHOT(o)->cam)->at[1];
}

/* hanashot: puts it back. */
void hanashotRestoreAtY(Obj *o) {
    camGetView(HANASHOT(o)->cam)->at[1] = HANASHOT(o)->atY;
}

u8 hanashotGetShotNo(Obj *o) {
    return HANASHOT(o)->qNo;
}

/* "reversal"'s (reversal_proc) argument: only what its starter sets. */
typedef struct ReversalArg {
    /* 0x00 */ u8 pad00[0x8];
    /* 0x08 */ f32 pos[3];
    /* 0x14 */ f32 scale; /* its size */
    /* 0x18 */ u8 pad18[0x8];
} ReversalArg;

/* "reversal"'s work: two frames that grow, turn and fade. */
typedef struct ReversalW {
    /* 0x00 */ PolFrame *ring;
    /* 0x04 */ PolFrame *flash;
    /* 0x08 */ u8 pad08[0x10];
    /* 0x18 */ s32 alpha;
    /* 0x1C */ s32 time; /* messages handled */
} ReversalW;

extern Gfx reversalRingDl[]; /* the ring, in file 0x120 */
extern Gfx reversalFlashDl[]; /* the flash, in file 0xEE */

s32 reversal_proc(Obj *obj, s16 msg, void *arg);

/* "reversal": a ring and a flash at the argument's place that grow and fade over 0xE0 / 5
 * steps; the camera's height is kept meanwhile, and both controllers rumble.
 * Matched with "s32 pad[2]" (the frame) and case 1's one-line "if (p != NULL) { frmDestroy(p); }"
 * statements (IDO_NOTES 290, the delay slot). */
s32 reversal_proc(Obj *obj, s16 msg, void *arg) {
    ReversalW *w = FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "reversal";
            w->ring = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->ring == NULL) {
                return -1;
            }
            w->flash = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->flash == NULL) {
                return -1;
            }
            w->ring->dl2 = reversalRingDl;
            w->ring->flags |= 0x20;
            floadGetSetRam((void **) w->ring->segs, 0x120);
            w->ring->scale[0] = w->ring->scale[1] = w->ring->scale[2] = ((ReversalArg *) arg)->scale * 10.0f;
            w->ring->pos[0] = ((ReversalArg *) arg)->pos[0];
            w->ring->pos[1] = ((ReversalArg *) arg)->pos[1];
            w->ring->pos[2] = ((ReversalArg *) arg)->pos[2];
            w->flash->dl2 = reversalFlashDl;
            w->flash->flags |= 0x20;
            floadGetSetRam((void **) w->flash->segs, 0xEE);
            w->flash->scale[0] = w->flash->scale[1] = w->flash->scale[2] = ((ReversalArg *) arg)->scale;
            w->flash->pos[0] = ((ReversalArg *) arg)->pos[0];
            w->flash->pos[1] = ((ReversalArg *) arg)->pos[1];
            w->flash->pos[2] = ((ReversalArg *) arg)->pos[2];
            w->flash->rot[0] = -1.570796371f;
            w->alpha = 0xE0;
            hanashotSaveAtY(hanashotTask);
            reversalTask = obj;
            if (plIsInPlay(0)) {
                contPakRumbleStart(0, 0x66655663, 50);
            }
            if (plIsInPlay(1)) {
                contPakRumbleStart(1, 0x66655663, 50);
            }
            break;
        case 1:
            if (w->ring != NULL) { frmDestroy(&w->ring->f); }
            if (w->flash != NULL) { frmDestroy(&w->flash->f); }
            break;
        case 0:
            w->alpha -= 5;
            if (w->alpha <= 0) {
                hanashotRestoreAtY(hanashotTask);
                reversalTask = NULL;
                w->alpha = 0;
                return -1;
            }
            w->ring->rot[1] += 0.174532935f;
            w->ring->scale[2] *= 1.05;
            w->ring->scale[0] = w->ring->scale[1] = w->ring->scale[2];
            w->flash->scale[2] *= 1.05;
            w->flash->scale[0] = w->flash->scale[1] = w->flash->scale[2];
            break;
        case 3:
            w->ring->dl = eneDlEnvPrimColor(obj, 255, 0, 0, 255, 255, 100, w->alpha);
            w->flash->dl = eneDlEnvPrimColor(obj, 255, 0, 0, 255, 255, 100, w->alpha);
            break;
    }
    w->time++;
    return tskDefaultProc(obj, msg, arg);
}

/* A "reversal" at x, y, z under obj. */
void hanabiReversalCreate(Obj *obj, f32 x, f32 y, f32 z, f32 v) {
    ReversalArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = v;
    tskCreate(obj, reversal_proc, obj->file, &arg);
}

/* "hana_fire"'s (hana_fire_proc) argument: only what its starter sets. */
typedef struct HanaFireArg {
    /* 0x00 */ u8 pad00[0x4];
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ s32 color[3];
    /* 0x1C */ u8 unused1C;
    /* 0x1D */ u8 good;
    /* 0x1E */ u8 pad1E[0x2];
    /* 0x20 */ f32 speed0;
    /* 0x24 */ u8 pad24[0x20];
} HanaFireArg;

extern Gfx hanaFireShellDl[]; /* the shell, in file 0x120 */

void effectKawaraFly(Obj *obj, f32 x, f32 y, f32 z);

/* "hana_fire": a firework shell. It rises (0) until its speed runs out; a good one (1 -> 2)
 * bursts in its colours and lights the sky, a bad one falls back (3) and blows up (4) with a
 * reversal and flying tiles, the sky red. Then the sky's colour fades back (6, 7) and it ends. */
s32 hana_fire_proc(Obj *obj, s16 msg, void *arg) {
    HanaFireW *w = HANA_FIRE(obj);
    PolFrame *m;
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "hana_fire";
            w->frame = (PolFrame *) frmCreate(gFrmLayer2, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = hanaFireShellDl;
            m = w->frame;
            m->flags |= 0x20;
            floadGetSetRam((void **) w->frame->segs, 0x120);
            w->frame->scale[2] = 0.07999999821f;
            m = w->frame;
            w->frame->scale[0] = m->scale[1] = m->scale[2];
            w->frame->pos[0] = ((HanaFireArg *) arg)->pos[0];
            w->frame->pos[1] = ((HanaFireArg *) arg)->pos[1];
            w->frame->pos[2] = ((HanaFireArg *) arg)->pos[2];
            m = w->frame;
            w->color[0] = ((HanaFireArg *) arg)->color[0];
            w->color[1] = ((HanaFireArg *) arg)->color[1];
            w->color[2] = ((HanaFireArg *) arg)->color[2];
            w->good = ((HanaFireArg *) arg)->good;
            w->speed0 = ((HanaFireArg *) arg)->speed0;
            w->speed = w->speed0;
            soundIncamCam(0x2C6, m->pos[0], m->pos[1], m->pos[2]);
            break;
        case 1:
            m = w->frame;
            if (m != NULL) {
                frmDestroy(&m->f);
            }
            break;
        case 3:
            if (w->frame != NULL) {
                w->frame->dl = eneDlEnvPrimColor(obj, 255, 255, 255, 255, 255, 255, 160);
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    m = w->frame;
                    m->pos[1] += w->speed;
                    w->speed -= 0.01499999966f * gGameState.retracesPerFrame;
                    if (w->speed <= 0.0f) {
                        w->speed = 0.0f;
                        w->timer = 0;
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (w->good == 1) {
                        DPRINTF(">>>>>   hanabi fire 正解\n");
                        w->timer = 0;
                        w->state = 2;
                    } else {
                        DPRINTF(">>>>>   hanabi fire 誤り\n");
                        w->timer = 0;
                        w->state = 3;
                        w->speed = 0;
                    }
                    break;
                case 2:
                    DPRINTF(">>>>>   hanabi fire たまやー\n");
                    i = fadeRandRange(15, 25);
                    m = w->frame;
                    ptclSpawnHanabi(m->pos[0], m->pos[1], m->pos[2], i / 10, w->color[0], w->color[1], w->color[2]);
                    if (w->flags & 2) {
                        m = w->frame;
                        soundIncamCam(0x152, m->pos[0], m->pos[1], m->pos[2]);
                    } else {
                        m = w->frame;
                        soundIncamCam(0x153, m->pos[0], m->pos[1], m->pos[2]);
                    }
                    i = hanashotGetShotNo(obj->parent);
                    w->sun[0] = shellColors[i][0];
                    w->sun[1] = shellColors[i][1];
                    w->sun[2] = shellColors[i][2];
                    w->frame = (PolFrame *) frmDestroy(&w->frame->f);
                    w->timer = 0;
                    w->state = 6;
                    break;
                case 3:
                    m = w->frame;
                    m->pos[1] -= w->speed;
                    w->speed += 0.01499999966f * gGameState.retracesPerFrame;
                    if (w->speed0 <= w->speed) {
                        DPRINTF(">>>>>   hanabi fire 4\n");
                        w->timer = 0;
                        w->state = 4;
                        w->speed = 0.0f;
                    }
                    break;
                case 4:
                    DPRINTF(">>>>>   hanabi fire 52\n");
                    DPRINTF(">>>>>   qNo[%d]\n"); /* its argument is compiled out; unknown */
                    w->sun[0] = 0xF0;
                    w->sun[1] = 0;
                    w->sun[2] = 0;
                    w->flags |= 1;
                    sndPlay(0x154);
                    m = w->frame;
                    hanabiReversalCreate(hanashotShellTask, m->pos[0], m->pos[1], m->pos[2], 0.1000000015f);
                    m = w->frame;
                    effectKawaraFly(hanashotShellTask, m->pos[0], m->pos[1], m->pos[2]);
                    DPRINTF(">>>>>   hanabi fire ボカン\n");
                    w->frame = (PolFrame *) frmDestroy(&w->frame->f);
                    w->timer = 0;
                    w->state = 6;
                    break;
                case 6:
                    w->sun[0] -= 0x60;
                    if (w->sun[0] < 16) {
                        w->sun[0] = 16;
                    }
                    w->sun[1] -= 0x60;
                    if (w->sun[1] < 16) {
                        w->sun[1] = 16;
                    }
                    w->sun[2] -= 0x60;
                    if (w->sun[2] < 16) {
                        w->sun[2] = 16;
                    }
                    if (w->sun[0] > 16) {
                        w->sunStep[0] = w->sun[0] / (20 / gGameState.retracesPerFrame);
                    } else {
                        w->sunStep[0] = 16;
                    }
                    if (w->sun[1] > 16) {
                        w->sunStep[1] = w->sun[1] / (20 / gGameState.retracesPerFrame);
                    } else {
                        w->sunStep[1] = 16;
                    }
                    if (w->sun[2] > 16) {
                        w->sunStep[2] = w->sun[2] / (20 / gGameState.retracesPerFrame);
                    } else {
                        w->sunStep[2] = 16;
                    }
                    w->state = 7;
                    break;
                case 7:
                    camSunSetColor(w->sun[0], w->sun[1], w->sun[2]);
                    w->sun[0] -= w->sunStep[0];
                    if (w->sun[0] < 16) {
                        w->sun[0] = 16;
                    }
                    w->sun[1] -= w->sunStep[1];
                    if (w->sun[1] < 16) {
                        w->sun[1] = 16;
                    }
                    w->sun[2] -= w->sunStep[2];
                    if (w->sun[2] < 16) {
                        w->sun[2] = 16;
                    }
                    if (w->sun[0] <= 16 && w->sun[1] <= 16 && w->sun[2] <= 16) {
                        w->state = 8;
                    }
                    break;
                case 8:
                    return -1;
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "hana_fire" (firework) at x, y, z under obj. */
Obj *hanabiFireCreate(Obj *obj, f32 x, f32 y, f32 z, s32 a, s32 b, s32 c, u8 d, f32 e) {
    HanaFireArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.color[0] = a;
    arg.color[1] = b;
    arg.color[2] = c;
    arg.good = d;
    arg.speed0 = e;
    DPRINTF(">>>>   hanabiFireCreate just do it!\n");
    return tskCreate(obj, hana_fire_proc, obj->file, &arg);
}

/* hana_fire: whether it is in state 7. */
s32 hanabiFireIsState7(Obj *o) {
    if (HANA_FIRE(o)->state == 7) {
        return 1;
    }
    return 0;
}

void hanaUnusedStub7(void) {
}

/* hana_fire: the burst's other sound ("hanabiFireAskFail", shaking). */
void hanabiFireAskFail(Obj *o) {
    DPRINTF(">>>>   hanabiFireAskFail がたがた step=[%d]\n", HANA_FIRE(o)->state);
    HANA_FIRE(o)->flags |= 2;
}

/* hana_fire: back to the usual sound (not shaking). */
void hanabiFireAskFailClear(Obj *o) {
    DPRINTF(">>>>   hanabiFireAskFail がたがたしない step=[%d]\n", HANA_FIRE(o)->state);
    HANA_FIRE(o)->flags &= ~2;
}

/* One of "particle"'s sixteen pieces (a roof tile): a position and a speed in texels (the
 * centre is 0), and a brightness whose top four bits are what is plotted. */
typedef struct ParticlePiece {
    /* 0x0 */ s8 x;
    /* 0x1 */ s8 y;
    /* 0x2 */ s8 vx;
    /* 0x3 */ s8 vy; /* in quarter texels */
    /* 0x4 */ u8 life; /* the brightness */
} ParticlePiece; /* size 5 */

/* "particle"'s work (the roof tiles flying, effectKawaraFly), as particle.c's ParticleWork. */
typedef struct ParticleW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 flags; /* 1: they fall; 2: filtered */
    /* 0x05 */ u8 time;
    /* 0x06 */ Color3 color;
    /* 0x09 */ ParticlePiece pieces[16];
} ParticleW;

extern Gfx hanaParticleDl[]; /* the particle draw setup */
extern Vtx hanaParticleVtx[]; /* the quad */

/* The file's .data, in address order (its externs above stay as declarations). */
Obj *hanashotShellTask = (Obj *) 0x12345670; /* hanashot, where the shells go */
Obj *reversalTask = (Obj *) 0x12345670; /* the reversal running (0: none) */
Obj *hanashotTask = (Obj *) 0x12345670; /* hanashot */
u8 fireworkSeqLengths[] = { 0x04, 0x05, 0x06, 0x07, 0x08, 0x00, 0x00, 0x00 }; /* each sequence's length */
u8 fireworkSeqs[][9] = {
    { 0x01, 0x02, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x01, 0x05, 0x03, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00 },
    { 0x01, 0x05, 0x03, 0x06, 0x02, 0x04, 0x00, 0x00, 0x00 },
    { 0x01, 0x05, 0x03, 0x04, 0x07, 0x02, 0x06, 0x00, 0x00 },
    { 0x01, 0x07, 0x02, 0x08, 0x04, 0x06, 0x03, 0x05, 0x00 },
}; /* the firework sequences, each ending in 0 */
f32 mortarPlaces[][9][3] = {
    { { 15.0f, 0.0f, -65.0f }, { -25.0f, 0.0f, -65.0f }, { 15.0f, 0.0f, -30.0f }, { -25.0f, 0.0f, -30.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
    { { 35.0f, 0.0f, -65.0f }, { -5.0f, 0.0f, -65.0f }, { -45.0f, 0.0f, -65.0f }, { 15.0f, 0.0f, -30.0f }, { -25.0f, 0.0f, -30.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
    { { 35.0f, 0.0f, -65.0f }, { -5.0f, 0.0f, -65.0f }, { -45.0f, 0.0f, -65.0f }, { 35.0f, 0.0f, -30.0f }, { -5.0f, 0.0f, -30.0f }, { -45.0f, 0.0f, -30.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
    { { 35.0f, 0.0f, -65.0f }, { 10.0f, 0.0f, -65.0f }, { -15.0f, 0.0f, -65.0f }, { -40.0f, 0.0f, -65.0f }, { 25.0f, 0.0f, -30.0f }, { -5.0f, 0.0f, -30.0f }, { -30.0f, 0.0f, -30.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
    { { 35.0f, 0.0f, -65.0f }, { 10.0f, 0.0f, -65.0f }, { -15.0f, 0.0f, -65.0f }, { -40.0f, 0.0f, -65.0f }, { 35.0f, 0.0f, -30.0f }, { 10.0f, 0.0f, -30.0f }, { -15.0f, 0.0f, -30.0f }, { -40.0f, 0.0f, -30.0f }, { 0.0f, 0.0f, 0.0f } },
}; /* each layout's mortar places (all 0: no mortar) */
Gfx hidutuDl[] = {
    { 0x00000000, 0x00000000 },
    { 0x05080000, 0x00000000 },
    { 0x00000000, (u32) hidutuDlEnd },
};
u32 hanaoyaAnim = 0x03000634; /* its animation */
void *hanadeshiAnims[] = { hanadeshiAnim, (void *) 0x03000668, (void *) 0x03000A2C }; /* the apprentice's animations (also its person table) */
s16 hanadeshiShows[][0xD6] = { 7, 1, 0, 0, 0, 0, 30, -65, 10, 2, 270, 7, 2, 1, 1, 0, 8, 1, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, 15, -50, 10, 0, -25, -50, 10, 0, -35, -65, 10, 2, 90, 7, 2, 1, 1, 0, 8, 2, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, -35, -30, 10, 2, 90, 7, 2, 1, 1, 0, 8, 4, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, -25, -20, 10, 0, 15, -20, 10, 0, 30, -30, 10, 2, 270, 7, 2, 1, 1, 0, 8, 3, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, 55, -69, 10, 2, 315, 7, 0, 0, 0, 0, 6, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 1, 0, 0, 0, 0, 45, -65, 10, 2, 270, 7, 2, 1, 1, 0, 8, 1, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, 30, -20, 10, 0, -30, -20, 10, 0, -35, -30, 10, 2, 90, 7, 2, 1, 1, 0, 8, 5, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, -60, -65, 10, 2, 90, 7, 2, 1, 1, 0, 8, 3, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, -45, -55, 10, 0, 25, -45, 10, 0, 25, -30, 10, 2, 270, 7, 2, 1, 1, 0, 8, 4, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, 10, -65, 10, 2, 270, 7, 2, 1, 1, 0, 8, 2, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, 55, -69, 10, 2, 315, 7, 0, 0, 0, 0, 6, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 1, 0, 0, 0, 0, 45, -65, 10, 2, 270, 7, 2, 1, 1, 0, 8, 1, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, 35, -50, 10, 0, 5, -30, 10, 2, 270, 7, 2, 1, 1, 0, 8, 5, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, -5, -55, 10, 0, -50, -55, 10, 0, -60, -65, 10, 2, 90, 7, 2, 1, 1, 0, 8, 3, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, -60, -30, 10, 2, 90, 7, 2, 1, 1, 0, 8, 6, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, -45, -50, 10, 0, -20, -65, 10, 2, 90, 7, 2, 1, 1, 0, 8, 2, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, 20, -30, 10, 2, 90, 7, 2, 1, 1, 0, 8, 4, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, 35, -50, 10, 0, 55, -69, 10, 2, 315, 7, 0, 0, 0, 0, 6, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 1, 0, 0, 0, 0, 45, -65, 10, 2, 270, 7, 2, 1, 1, 0, 8, 1, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, 35, -30, 10, 2, 270, 7, 2, 1, 1, 0, 8, 5, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, 25, -45, 10, 0, 0, -65, 10, 2, 270, 7, 2, 1, 1, 0, 8, 3, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, -15, -50, 10, 0, -30, -65, 10, 2, 270, 7, 2, 1, 1, 0, 8, 4, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, -45, -30, 10, 2, 90, 7, 2, 1, 1, 0, 8, 7, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, -35, -45, 10, 0, 0, -65, 10, 2, 90, 7, 2, 1, 1, 0, 8, 2, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, 10, -30, 10, 2, 270, 7, 2, 1, 1, 0, 8, 6, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, 25, -50, 10, 0, 55, -69, 10, 2, 315, 7, 0, 0, 0, 0, 6, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 1, 0, 0, 0, 0, 45, -65, 10, 2, 270, 7, 2, 1, 1, 0, 8, 1, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, 35, -50, 10, 0, -5, -30, 10, 2, 270, 7, 2, 1, 1, 0, 8, 7, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, -5, -65, 10, 2, 90, 7, 2, 1, 1, 0, 8, 2, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, -30, -30, 10, 2, 270, 7, 2, 1, 1, 0, 8, 8, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, -30, -65, 10, 2, 270, 7, 2, 1, 1, 0, 8, 4, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, -15, -55, 10, 0, 0, -30, 10, 2, 90, 7, 2, 1, 1, 0, 8, 6, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, 0, -65, 10, 2, 270, 7, 2, 1, 1, 0, 8, 3, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, 25, -30, 10, 2, 90, 7, 2, 1, 1, 0, 8, 5, 7, 2, 1, 1, 1, 7, 1, 0, 0, 0, 0, 35, -50, 10, 0, 55, -69, 10, 2, 315, 7, 0, 0, 0, 0, 6, -1 };
f32 shellFirePlaces[][3] = {
    { -166.0f, 137.0f, -235.0f },
    { -14.0f, 137.0f, -231.0f },
    { -59.0f, 137.0f, -125.0f },
    { -244.0f, 140.0f, -243.0f },
    { -55.0f, 140.0f, -125.0f },
    { 29.0f, 140.0f, -145.0f },
    { -78.0f, 140.0f, -150.0f },
    { -300.0f, 137.0f, -198.0f },
    { -48.0f, 137.0f, -198.0f },
    { -110.0f, 137.0f, -198.0f },
    { -47.0f, 140.0f, -16.0f },
}; /* where each shell is fired from */
u8 shellColors[][3] = {
    { 0xD0, 0x20, 0x20 },
    { 0x20, 0xD0, 0x20 },
    { 0x20, 0x20, 0xD0 },
    { 0xD0, 0xD0, 0x20 },
    { 0x20, 0xD0, 0xD0 },
    { 0xD0, 0x20, 0xD0 },
    { 0xD0, 0xD0, 0xD0 },
    { 0x80, 0xB0, 0xF0 },
    { 0xF0, 0xB0, 0xF0 },
    { 0xF0, 0x20, 0x20 },
    { 0xD0, 0x20, 0x20 },
    { 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00 },
}; /* each shell's colour (and the sky's at its burst) */
Gfx hanaParticleDl[] = {
    gsDPPipeSync(),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_CULL_BACK),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetTextureLOD(G_TL_TILE),
    gsDPSetTextureDetail(G_TD_CLAMP),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsDPSetTextureConvert(G_TC_FILT),
    gsDPSetCombineLERP(0, 0, 0, PRIMITIVE, PRIMITIVE, 0, TEXEL0, 0, 0, 0, 0, PRIMITIVE, PRIMITIVE, 0, TEXEL0, 0),
    gsDPSetRenderMode(G_RM_ZB_CLD_SURF, G_RM_ZB_CLD_SURF2),
    gsSPEndDisplayList(),
}; /* the particle draw setup */
Vtx hanaParticleVtx[] = {
    {{{ -20, -20, 0 }, 0, { 0, 4096 }, { 0x00, 0x00, 0x7F, 0xFF }}},
    {{{ 20, 20, 0 }, 0, { 4096, 0 }, { 0x00, 0x00, 0x7F, 0xFF }}},
    {{{ 20, -20, 0 }, 0, { 4096, 4096 }, { 0x00, 0x00, 0x7F, 0xFF }}},
    {{{ -20, 20, 0 }, 0, { 0, 0 }, { 0x00, 0x00, 0x7F, 0xFF }}},
}; /* the quad */

/* "particle", the roof tiles' handler: particle.c's particle with its display lists in this
 * file (fadeSegToRam), the frame at 10x scale, and the frame destroyed only if made. */
s32 hanaParticle(Obj *obj, s16 msg, void *arg) {
    u8 *buf;
    ParticleW *w = FX(obj);
    s32 i;
    s32 n;
    s32 x;
    s32 y;
    s32 k;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "particle";
            w->frame = (PolFrame *) frmCreate(gFrmEffectLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl = (Gfx *) fadeSegToRam(hanaParticleDl, obj->file);
            w->frame->flags |= 7;
            w->frame->scale[2] = 10.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->time = 0x40;
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 0:
            n = 0;
            if (w->time == 0) {
                return -1;
            }
            w->time--;
            for (i = 0; i < 16; i++) {
                if (w->pieces[i].life >= 0x10) {
                    w->pieces[i].life -= 4;
                    x = w->pieces[i].x + w->pieces[i].vx;
                    y = w->pieces[i].y + (w->pieces[i].vy >> 2);
                    if ((u16) (x + 0x80) >= 0x100 || (u16) (y + 0x80) >= 0x100) {
                        w->pieces[i].life &= 0xF;
                    } else {
                        w->pieces[i].x = x;
                        w->pieces[i].y = y;
                        n++;
                        if (w->flags & 1) {
                            if (w->pieces[i].vy < 0x18) {
                                if ((w->pieces[i].life & 3) + 1 == 4) {
                                    w->pieces[i].vy++;
                                } else {
                                    w->pieces[i].life++;
                                }
                            }
                        }
                    }
                }
            }
            if (n == 0) {
                return -1;
            }
            break;
        case 3:
            GFX_ALLOC(buf, 0x800);
            heapMemClear(buf, 0x800);
            for (i = 0; i < 16; i++) {
                if (w->pieces[i].life >= 0x10) {
                    k = (w->pieces[i].x >> 2) + ((w->pieces[i].y >> 2) << 6) + 0x820;
                    buf[k >> 1] |= (w->pieces[i].life & 0xF0) >> ((k & 1) * 4);
                }
            }
            w->frame->dl2 = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPLoadTextureBlock_4b(gGfxDlHead++, buf, G_IM_FMT_I, 64, 64, 0, G_TX_CLAMP, G_TX_CLAMP,
                                   G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->color.r, w->color.g, w->color.b, 255);
            if (w->flags & 2) {
                gDPSetTextureFilter(gGfxDlHead++, G_TF_BILERP);
            } else {
                gDPSetTextureFilter(gGfxDlHead++, G_TF_POINT);
            }
            gSPVertex(gGfxDlHead++, fadeSegToRam(hanaParticleVtx, obj->file), 4, 0);
            gSP2Triangles(gGfxDlHead++, 2, 1, 0, 0, 1, 3, 0, 0);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "particle" (flying roof tiles) under obj at x, y, z, grey, its sixteen pieces each with a
 * random speed (-4 to 3 across, -16 to 15 up) and life (128 to 255). */
void effectKawaraFly(Obj *obj, f32 x, f32 y, f32 z) {
    Obj *o = tskCreate(obj, hanaParticle, obj->file, NULL);
    ParticleW *w;
    s32 i;

    DPRINTF("^^^^   effectKawaraFly\n");
    if (o != NULL) {
        ((ParticleW *) FX(o))->color.r = 160;
        ((ParticleW *) FX(o))->color.g = 160;
        ((ParticleW *) FX(o))->color.b = 160;
        ((ParticleW *) FX(o))->frame->pos[0] = x;
        ((ParticleW *) FX(o))->frame->pos[1] = y;
        ((ParticleW *) FX(o))->frame->pos[2] = z;
        w = (ParticleW *) FX(o);
        for (i = 0; i < 16; i++) {
            w->pieces[i].vx = fadeRand(8) - 4;
            w->pieces[i].vy = fadeRand(32) - 16;
            w->pieces[i].life = fadeRand(128) + 128;
        }
    }
}


/* "deshi_p": the fireworks apprentice as a person; made only once flag 0xA5 is set (the pass
 * not yet given, the print says). */
s32 deshiPerson_proc(Obj *o, s16 msg, void *arg) {
    switch (msg) {
        case 2:
            o->name = "deshi_p";
            if (!swWorkTest(0xA5)) {
                DPRINTF(">>>> deshiPerson_proc: deshiPerson まだ手形もらっていない{終了}\n");
                return -1;
            }
            if (personInitMainTable(o, hanadeshiAnims, arg) == -1) {
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

/* Makes enemy 0x1EF at (3, 0, 84) in zone 0, mode 2, word 0x1E55, under the enemy manager's
 * first child's third sibling after it. */
void demoHanabiMakeJizou(void) {
    eneMakeEx(gEneManager->child->next->next->next, ENE_DEMO_JIZOU_FILE_138, 0, 3.0f, 0.0f, 84.0f, 2, (void *) 0x1E55);
}
