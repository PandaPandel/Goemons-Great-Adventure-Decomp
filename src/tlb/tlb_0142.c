#include "tlb.h"

#define DPRINTF

/* Fades out to black at speed (tlb_0126's tsuchiFadeOut, with its first call on the brace's
 * line). */
void obisu2FadeOut(f32 speed) { fadeSetSpeed(speed);
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

/* "obisu_r" (its proc obisuRProc): only what its helpers below touch. */
typedef struct ObisuRW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 unused09;
    /* 0x0A */ u8 hit; /* 1 on the ground */
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ AnimeFrame *anim;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ Frame *frame; /* its damage box */
    /* 0x3C */ MPolFrame *shadow;
    /* 0x40 */ Obj *bubble; /* a "Tbubble" (kind 2) */
    /* 0x44 */ u8 type; /* eneSetup's result: which obisu_r this is */
    /* 0x45 */ u8 flags; /* 1: no gravity (jumping), 2: going */
    /* 0x46 */ u8 pad46[2];
    /* 0x48 */ f32 animSpeed;
    /* 0x4C */ s32 timer;
    /* 0x50 */ u8 goTimer;
} ObisuRW;

/* "Tbubble" (its proc tbubbleProc): its frame, made by frmCreate. */
typedef struct TbubbleW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 state; /* 0 swelling, 1 shrinking */
    /* 0x05 */ u8 pad05[0xF];
    /* 0x14 */ s32 alpha;
    /* 0x18 */ s32 timer;
} TbubbleW;

/* The pass goes to the player (state 3). */
void obisu2PassToPlayer(Obj *o, Obj *player) {
    PassW *w = (PassW *) FX(o);

    w->state = 3;
    w->player = player;
    w->timer = 0;
}

void obisu2EventTegataSdSet(void) {
    DPRINTF("&&&& eventTegataSdSet  pl D がないn");
}

void obisu2EventClearSd(void) {
    DPRINTF("&&&& SD_G_CLEAR1n");
    DPRINTF("&&&& SD_E_CLEAR1n");
    DPRINTF("&&&& SD_S_CLEAR1n");
    DPRINTF("&&&& SD_Y_CLEAR1n");
}

/* acollAtkAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy,
                                     s16 oz, s32 a);

/* acollDmgAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*DmgAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height,
                                     s16 ox, s16 oy, s16 oz, s32 a);

extern Unk88208 gSaveCopy; /* the save as it was (undefined_syms.txt; demo.c's) */

/* "obisu"'s work, at its task + 0x20: a demo talker's (PersonObj) up to 0x44 (tlb_0129's ObisuW,
 * with a walk where that one has its glow). */
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
    /* 0x48 */ u8 flags; /* 1 no fall, 4 the sky's tunes change 300 ticks on */
    /* 0x4C */ s32 timer; /* ticks in the state */
    /* 0x50 */ Camera *cam;
    /* 0x54 */ Obj *pass; /* the pass (kind 0x1C3) */
    /* 0x58 */ u8 tuneTimer;
    /* 0x5C */ u16 *path; /* the walk's command it is at */
} ObisuW;

#define OBISU(obj) ((ObisuW *) FX(obj))

extern u8 obisuMaru2StandAnim[]; /* Obisumaru standing */
extern u8 obisuMaru2WalkAnim[]; /* Obisumaru walking */
extern u8 obisuMaru2JumpAnim[]; /* Obisumaru jumping */
extern u32 obisuRRunAnim; /* obisu_r running */

void obisu2PathInit(Obj *o, PersonData *d);
u16 *obisu2ActStep(Obj *o, u16 *base, u16 *path);

/* "obisu": Obisumaru, gone once the pass is got (flag 0xB4) or until the chase of area 2 is won
 * (0x9B), and with no Ebisumaru among the players. He walks his path; talked to by another, he
 * talks (0x2A8) and walks on; by Ebisumaru, he jumps, turns to him and talks; taken on (flag
 * 0x30), the screen fades and the race begins at the atlas (stage 4, 3) with Ebisumaru alone.
 * Back from it (0x92) he talks again and, if Ebisumaru won (0x31), gives the pass, talks (0x2A9)
 * and, at flag 0x95, spins and shrinks away in puffs (tlb_0129's obisu, which waits for 8 and
 * goes to stage 1, 2). The "*** さようなら" print is placed by its string's order (between
 * case 3's and case 9's), in case 8.
 * Matched 2026-10-12; it was parked for case 11's two un-shared 2.0 loads; eneFall(o, 2) made
 * 2.0 one constant across the function, as tlb_0129's obisu_maru_proc already had it. */
s32 obisu_maru2_proc(Obj *o, s16 msg, void *arg) {
    ObisuW *w = OBISU(o);
    s32 i;
    s32 chara;
    u32 data;
    s32 pad2; /* FAKE */
    u16 *path;
    s32 pad1; /* FAKE */
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

    switch (msg) {
        case 2:
            o->name = "obisu";
            if (swWorkTest(0xB4)) {
                DPRINTF(">>>  obisu すでに手形ゲット(即ち死ね)\n");
                return -1;
            }
            if (!swWorkTest(0x9B)) {
                DPRINTF(">>>  obisu エリア２のおいかけっこクリアしていない(即ち死ね)\n");
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
            w->model = eneNewAnime(o, (u32) obisuMaru2StandAnim);
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
            (*obisu2PathInit)(o, w->data);
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
                    w->model->anim = obisuRRunAnim;
                    w->model->frame = 0.0f;
                    w->state = 1;
                    break;
                case 1:
                    if (w->path != NULL) {
                        path = w->path;
                        w->path = obisu2ActStep(o, (u16 *) stageAddrToRam(w->data->path), path);
                    }
                    eneAnimLoop(o, w->model, 0.8f);
                    if (demoTalkEndCheck(o)) {
                        DPRINTF("obisu_maru_proc: ミッション始めませんぜ\n");
                        if (plGetChara(w->talker) == 1) {
                            DPRINTF("obisu_maru_proc: ミッション始めるぜ\n");
                            demoCamZoom_CL(w->talker);
                            w->state = 6;
                            w->timer = 0;
                        } else {
                            DPRINTF("obisu_maru_proc: ミッション始めませんぜ\n");
                            w->state = 2;
                            w->timer = 0;
                        }
                    }
                    break;
                case 2:
                    scRunStart(0x2A8, plGetIndex(w->talker) + 1, plGetChara(w->talker));
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
                        w->model->anim = obisuMaru2JumpAnim;
                        w->model->frame = 0;
                        w->vel[0] = 0;
                        w->flags |= 1;
                        w->hit &= ~1;
                        w->vel[2] = 0;
                        w->vel[1] = 2.0f;
                    }
                    if (polAnimeLastFrame(w->model) / 2 < w->model->frame) {
                        w->flags &= ~1;
                        w->vel[1] = 0.0f;
                    }
                    if (w->hit & 1) {
                        w->model->anim = obisuMaru2StandAnim;
                        w->model->frame = 0.0f;
                        w->state = 5;
                    }
                    eneAnimPlayOnce(o, w->model, 0.8f);
                    break;
                case 5:
                    if (w->talker != NULL) {
                        tskSendMsg(w->talker, 4, &at);
                        if (eneTurnToPoint(o, at.x, at.z, 12.0f) == 1) {
                            w->state = 4;
                        }
                    }
                    break;
                case 4:
                    scRunStart(0x2A8, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    w->state = 7;
                    break;
                case 7:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x30)) {
                            gGameWork.scenarioRunning = 2;
                            w->timer = 0;
                            w->state = 8;
                        } else {
                            w->model->anim = obisuMaru2WalkAnim;
                            w->model->frame = 0.0f;
                            w->state = 9;
                        }
                    }
                    break;
                case 8:
                    if (w->timer == 1) {
                        sndPlay(10);
                        obisu2FadeOut(16 / gRetracesPerFrame);
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
                        demoAreaFlagSetBit7(4);
                        gSaveData.stageFlags[0x42] |= 1;
                        gSaveData.stageFlags[0x42] &= ~0x80;
                        gSaveData.stageFlags[0x3C] |= 1;
                        gSaveData.stageFlags[0x3C] &= ~0x80;
                        for (i = 0; i < 4; i++) {
                            if (gSaveData.players[i].chara != 1 || gSaveData.players[i].lives <= 0) {
                                gSaveData.players[i].chara = -1;
                            }
                        }
                        DPRINTF("*** さようなら\n");
                        stageGotoAtlas(4, 3);
                    }
                    break;
                case 9:
                    personTalkCamOff();
                    demoFinish(o);
                    DPRINTF("また探し始める\n");
                    w->state = 1;
                    break;
                case 10:
                    if (w->timer == 1) {
                        demoPlStandAll();
                    }
                    if (w->timer == 8) {
                        if (!swWorkTest(0x31)) {
                            camSunSetTunes4(0xBE, 0xBB, 0xBC, 0xBD);
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
                        scRunStart(0x2A8, idx + 1, chara);
                        if (swWorkTest(0x31)) {
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
                        obisu2PassToPlayer(w->pass, gGameWork.players[demoFindPlByChara(1)]);
                        w->tuneTimer = 0;
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
                        scRunStart(0x2A9, 0, 0);
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
                    if (w->timer == 1) {
                        sndPlay(0x160);
                    }
                    w->rot[1] += 0.17453294f;
                    w->model->scale[0] -= 0.002f;
                    w->model->scale[2] -= 0.002f;
                    if (!(w->timer & 3)) {
                        itexPuff(w->model->pos[0] + fadeRandRange(1, 6), w->model->pos[1],
                                 w->model->pos[2] + fadeRandRange(1, 6), 0.03f);
                    }
                    if (w->model->scale[0] <= 0.0f) {
                        w->model->scale[2] = 0.001f;
                        w->model->scale[0] = w->model->scale[2];
                        if (300 / gRetracesPerFrame < w->tuneTimer) {
                            demoPlReleaseAll();
                            return -1;
                        }
                    }
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
                    camSunSetTunes4(0xBE, 0xBB, 0xBC, 0xBD);
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

/* Its walk's start, from its table entry (tlb_0109's shopClerkPathInit). */
void obisu2PathInit(Obj *o, PersonData *d) {
    OBISU(o)->path = (u16 *) stageAddrToRam(d->path);
}

/* The walker's work (obisu_maru2_proc's): an enemy's, with a wait counter. */
typedef struct PathW {
    /* 0x00 */ EneObj e;
    /* 0x4C */ u8 pad4C[0xC];
    /* 0x58 */ u8 wait;
} PathW;

/* Runs one command of a walk (s16s from `path`) and returns the next:
 *   0 x z speed: walk to (x, z), facing it (13 degrees a frame), until there;
 *   1 n: go to path[n];
 *   2 deg: stop and turn to deg, until turned;
 *   4 n: stop, wait n + 1 frames;
 *   5 id: sound id at the walker;
 *   6: stop.
 * tlb_0104's wisemanActStep, its counter a byte at 0x58. */
u16 *obisu2ActStep(Obj *o, u16 *base, u16 *path) {
    PathW *p = (PathW *) FX(o);
    s32 pad[2];

    switch (path[0]) {
        case 0: {
            f32 x = (s16) path[1];
            f32 z = (s16) path[2];
            f32 speed = (s16) path[3];

            if (eneMoveToPos(o, x, z, speed) == 1) {
                path += 4;
            }
            eneTurnToPoint(o, x, z, 13.0f);
            break;
        }
        case 1:
            path = base + (s16) path[1];
            break;
        case 2: {
            f32 deg = (s16) path[1];

            p->e.vel[0] = 0.0f;
            p->e.vel[2] = 0.0f;
            if (eneTurnToAngle(o, deg, 13.0f) == 1) {
                path += 2;
            }
            break;
        }
        case 3:
            DPRINTF("超人に行動パターンはいってません\n");
            break;
        case 4: {
            s16 n = path[1];

            if (p->wait == 0) {
                p->e.vel[0] = 0.0f;
                p->e.vel[2] = 0.0f;
            }
            if (n < p->wait++) {
                p->wait = 0;
                path += 2;
            }
            break;
        }
        case 5: {
            s16 se = path[1];

            soundLocationCam(se, p->e.x, p->e.y, p->e.z);
            path += 2;
            break;
        }
        case 6:
            path += 1;
            p->e.vel[2] = 0.0f;
            p->e.vel[1] = 0.0f;
            p->e.vel[0] = 0.0f;
            break;
        default:
            DPRINTF("actStep がへんです");
            break;
    }
    return path;
}

void obisu2UnusedStub1(void) {
}

void obisu2UnusedStub2(void) {
}

void obisu2UnusedStub3(void) {
}

/* "demoObisuRun_proc"'s work. */
typedef struct DemoObisuW {
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
    /* 0x34 */ Obj *obisu; /* obisu_r (kind 0x1CE, then 0x1CF near the goal) */
    /* 0x38 */ Camera *cam;
    /* 0x3C */ ACollFrame *box; /* the goal */
    /* 0x40 */ s32 timer; /* ticks in the state */
    /* 0x44 */ s8 partake; /* gGameWork.partake, kept while the race runs */
} DemoObisuW;

void obisuRSetState2(Obj *o);
f32 obisuRGetX(Obj *o);
void obisuRSetAnime(Obj *o, s32 i);
void obisuRStop(Obj *o);
void obisuRFrameFree(Obj *o);

/* "demoObisuRun_proc": the race with obisu_r. Made while flag 0x30 is set and until 0xB4.
 * obisu_r appears (kind 0x1CE), both talk (0x21B), the race (0x21C) runs; at 5160 retraces a
 * second obisu_r comes down by the goal (0x1CF). When a player touches the goal box: if a player
 * in zone 4 is ahead, obisu_r loses (0x2CB, state 9); otherwise he wins, turns and gloats
 * (0x2CA, state 5). Then the screen fades, flag 0x92 is set, the players are restored and the
 * game goes to stage 0xDE. A player dying ends it too. The pads are frame words the game has
 * (its stack slots, measured). */
s32 demoObisuRun2_proc(Obj *o, s16 msg, void *arg) {
    DemoObisuW *w = (DemoObisuW *) FX(o);
    s32 pad[3];
    f32 best;
    f32 place;
    f32 height;
    f32 side;
    s32 zone;
    s32 i;
    s32 pad2[2];
    s32 j;
    s32 pad3;

    switch (msg) {
        case 2:
            o->name = "demoObisuRun_proc";
            if (!swWorkTest(0x30)) {
                DPRINTF(">>>  おびすまるおっかけは開始されません\n");
                return -1;
            }
            if (swWorkTest(0xB4)) {
                DPRINTF(">>>   おびすまるおっかけは開始されません 手形もらってるで\n");
                return -1;
            }
            eneSetup(o, arg);
            w->model = eneNewPol(o, NULL);
            if (w->model == NULL) {
                return -1;
            }
            gGameWork.mission = 1;
            gGameWork.scenarioRunning = 2;
            w->partake = gGameWork.partake;
            gGameWork.partake = 1;
            gGameWork.areaStage = 3;
            swWorkReset(0x37);
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
                    w->obisu = eneMakeEx(o, ENE_OBISU_R, 1, 244.0f, 122.0f, 15.0f, 2, (u32 (*)(void *, void *, void *)) 5);
                    DPRINTF(">>>  オビス丸出現\n");
                    camAiSetHold(1);
                    w->cam = camAiGetCam();
                    camSetFov(w->cam, 1.31813097f);
                    camSetAt(w->cam, -2305.0f, 128.0f, 1175.0f);
                    camSetEye(w->cam, -2277.0f, 133.0f, 1202.0f);
                    camSunSetClockStop(1);
                    demoPlStandAll();
                    scRunStart2(0x21B, 0, 0);
                    w->state = 1;
                    break;
                case 1:
                    if (!scIsRunning()) {
                        scRunStart2(0x21C, 0, 0);
                        camSetFov(w->cam, 0.6981317401f);
                        demoPlReleaseAll();
                        obisuRSetState2(w->obisu);
                        camAiSetHold(0);
                        w->timer = 0;
                        w->state = 2;
                    }
                    break;
                case 2:
                    w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(uda_atk_root_frame, &w->model->f, 200, 0, 200, 0, 0);
                    w->box->kind = 100;
                    w->box->flags |= 0x10;
                    w->state = 3;
                    break;
                case 3:
                    DPRINTF("\rP100timer[%d]", w->timer);
                    if (w->timer == 5160 / gRetracesPerFrame) {
                        DPRINTF(">>>>   ゴール前オビス丸降臨\n");
                        w->obisu = eneMakeEx(o, ENE_OBI_RF, 4, 2592.0f, 417.0f, 10.0f, 2, (u32 (*)(void *, void *, void *)) 6);
                    }
                    if (acollAtkWasHit(w->box) == 1) {
                        best = 0.0f;
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                plAskPos(gGameWork.players[i], &place, &height, &side, &zone);
                                if (zone == 4) {
                                    best = place;
                                }
                            }
                        }
                        if (obisuRGetX(w->obisu) <= best) {
                            DPRINTF("***  おびす敗け！\n");
                            demoPlStandAll();
                            demoSetPlPos(4, place, 450.0f, side, eneCourseAngle(o, 8), 4, place, 450.0f, side,
                                         eneCourseAngle(o, 8), 4, place, 450.0f, side, eneCourseAngle(o, 8), 4,
                                         place, 450.0f, side, eneCourseAngle(o, 8));
                            w->state = 9;
                        } else {
                            DPRINTF("***  おびす勝利！\n");
                            obisuRFrameFree(w->obisu);
                            obisuRStop(w->obisu);
                            obisuRSetAnime(w->obisu, 0);
                            swWorkSet(0x37);
                            w->state = 5;
                        }
                        w->box->radius = 400;
                        w->box->ofs[1] = 400;
                    }
                    break;
                case 4:
                    if (w->timer > 120 / gRetracesPerFrame) {
                        w->timer = 0;
                        w->state = 13;
                    }
                    break;
                case 5:
                    eneTurnToAngle(w->obisu, eneCourseAngle(o, 4) * 180.0f / 3.1415927f, 20.0f);
                    if (acollAtkWasHit(w->box) == 1) {
                        DPRINTF("&&&&   DEMO_OBISU_TURN111\n");
                        demoPlStandAll();
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        camSetSpeed(w->cam, 60 / gRetracesPerFrame);
                        camMoveFov(w->cam, 0.8981320262f);
                        camMoveAt(w->cam, -1237.0f, 471.0f, -1725.0f);
                        camMoveEye(w->cam, -1155.0f, 453.0f, -1745.0f);
                        w->timer = 0;
                        w->state = 6;
                    }
                    break;
                case 6:
                    plCheck();
                    if (eneTurnToAngle(w->obisu, eneCourseAngle(o, 4) * 180.0f / 3.1415927f, 20.0f)) {
                        w->state = 7;
                    }
                    break;
                case 7:
                    if (camGetMoving(w->cam)) {
                        scRunStart2(0x2CA, 0, 0);
                        obisuRSetAnime(w->obisu, 3);
                        obisuRStop(w->obisu);
                        w->state = 8;
                    }
                    break;
                case 8:
                    if (!scIsRunning()) {
                        gGameWork.scenarioRunning = 2;
                        w->timer = 0;
                        w->state = 13;
                    }
                    break;
                case 9:
                    if (obisuRGetX(w->obisu) >= 3109.0f) {
                        DPRINTF("&&&&   DEMO_OBISU_TURN222\n");
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        camGetView(w->cam);
                        camSetSpeed(w->cam, 60 / gRetracesPerFrame);
                        camMoveFov(w->cam, 0.8781319857f);
                        camMoveAt(w->cam, -1196.0f, 458.0f, -1724.0f);
                        camMoveEye(w->cam, -1234.0f, 469.0f, -1749.0f);
                        w->timer = 0;
                        w->state = 10;
                    }
                    break;
                case 10:
                    if (eneMoveToX(w->obisu, 3109.0f, 1.0f)) {
                        plSetAction(plCheck(), 0x6C);
                        w->state = 11;
                    }
                    break;
                case 11:
                    if (camGetMoving(w->cam)) {
                        scRunStart2(0x2CB, 0, 0);
                        obisuRSetAnime(w->obisu, 4);
                        obisuRStop(w->obisu);
                        w->state = 12;
                    }
                    break;
                case 12:
                    if (!scIsRunning()) {
                        gGameWork.scenarioRunning = 2;
                        w->timer = 0;
                        w->state = 13;
                    }
                    break;
                case 13:
                    if (w->timer == 1) {
                        for (j = 0; j < 4; j++) {
                            contPakInitRumble(j);
                            contPakRumbleRequestStop(j);
                        }
                        if (!swWorkTest(0x31)) {
                            sndPlay(10);
                        }
                        obisu2FadeOut(16 / gRetracesPerFrame);
                        gGameWork.scenarioRunning = 2;
                        if (!swWorkTest(0x31)) {
                            sndPlay(0x45F);
                        }
                    }
                    if (w->timer == 20 / gRetracesPerFrame) {
                        gGameWork.areaStage = 3;
                        gGameWork.scenarioRunning = 0;
                        gGameWork.mission = 0;
                        gGameWork.partake = w->partake;
                        swWorkReset(0x37);
                        swWorkSet(0x92);
                        demoAreaFlagRestore();
                        for (j = 0; j < 4; j++) {
                            if (gSaveCopy.players[j].chara != 1 && gSaveCopy.players[j].chara != -1) {
                                gSaveData.players[j] = gSaveCopy.players[j];
                            }
                        }
                        DPRINTF(">>>>   おしまい転送\n");
                        stageGotoSpawn(gStageTask->parent, 0xDE, 0, -36.0f, 10.0f, -11.0f, 3.1415927f, 0xD7);
                    }
                    break;
                case 14:
                    break;
            }
            if (gGameWork.mission == 2 && w->state != 13) {
                DPRINTF("%%%%   event manager: PLAYER 死にました信号をうけとったぜ！\n");
                gGameWork.scenarioRunning = 2;
                sndPlay(10);
                demoPlStandAll();
                w->timer = 0;
                w->state = 13;
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u32 obisuRAnims[]; /* its animations */
extern u32 obisuRRunAnim; /* obisu_r running */
extern u32 obisuRJumpAnim; /* obisu_r jumping */
extern u32 obisuRKind2Anim;
extern u32 obisuRKind3Anim;
extern u32 obisuRKind4Anim;

Vec3f obisuRBackPos(Obj *o);
s32 obisuPlCourseCheck(Obj *o);
void bijoStartFollow(void);
void bijoStopFollow(void);
Obj *tbubbleCreate(Obj *obj, f32 x, f32 y, f32 z);
void tbubbleGetPos(Obj *o, f32 *pos);

/* "obisu_r": obisu_r of the race's stage, one of six kinds (eneSetup's result). Made while flag
 * 0x30 is set, until 0xB4 or 0x37. Kinds 1, 2, 3, 4 wait in the player's zone (2 with a bubble,
 * 4 making kinds 0x1D0 and 0x1D1); when hit he runs off (state 8 waits for a second hit or flag
 * 0x37), jumps away (13, 14) and goes after 120 retraces. The kind is stored "& 0xFF" (the
 * game masks it twice); a float update written before the state stores keeps the game's
 * schedule (measured). */
s32 obisuRProc(Obj *o, s16 msg, void *arg) {
    ObisuRW *w = (ObisuRW *) FX(o);
    s32 i;
    s32 pad;
    Vec3f p;
    f32 place;
    f32 height;
    f32 side;
    s32 zone;
    s32 pad2;
    f32 pos[3];

    switch (msg) {
        case 2:
            o->name = "obisu_r";
            if (!swWorkTest(0x30)) {
                DPRINTF("****  obisu_r: OBISU_R_MISSION フラグたってないで\n");
                return -1;
            }
            if (swWorkTest(0xB4)) {
                DPRINTF("****  obisu_r: TEGATA2_5A 手形もらってる\n");
                return -1;
            }
            if (swWorkTest(0x37)) {
                DPRINTF("****  obisu_r: 最後のオビスが降臨しているので死ぬ\n");
                return -1;
            }
            w->type = eneSetup(o, arg) & 0xFF;
            switch (w->type) {
                case 1:
                    w->anim = eneNewAnime(o, obisuRAnims[0]);
                    w->state = 9;
                    w->animSpeed = 0.5f;
                    break;
                case 5:
                    w->anim = eneNewAnime(o, obisuRAnims[0]);
                    w->state = 1;
                    w->animSpeed = 0.5f;
                    break;
                case 2:
                    w->anim = eneNewAnime(o, obisuRKind2Anim);
                    w->animSpeed = 0.2f;
                    w->state = 10;
                    break;
                case 3:
                    w->anim = eneNewAnime(o, obisuRKind3Anim);
                    w->animSpeed = 0.7f;
                    w->state = 11;
                    break;
                case 4:
                    w->anim = eneNewAnime(o, obisuRKind4Anim);
                    w->state = 12;
                    w->animSpeed = 0.5f;
                    break;
                case 6:
                    w->anim = eneNewAnime(o, obisuRRunAnim);
                    w->state = 3;
                    w->animSpeed = 0.5f;
                    break;
                default:
                    w->anim = eneNewAnime(o, obisuRAnims[0]);
                    w->state = 7;
                    w->animSpeed = 0.5f;
                    break;
            }
            if (w->anim == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("obisu_run_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->atk = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->anim->f, 2400, 0, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x66;
            w->frame = (Frame *) ((DmgAddXYZFunc) acollDmgAddXYZ)(ene_dmg_root_frame, &w->anim->f, 70, 160, 0, 0, 0, 0);
            if (w->frame == NULL) {
                return -1;
            }
            w->shadow = shadowCreate();
            break;
        case 1:
            if (w->anim != NULL) {
                frmDestroy(&w->anim->f);
            }
            if (w->atk != NULL) {
                frmDestroy((Frame *) w->atk);
            }
            if (w->frame != NULL) {
                frmDestroy(w->frame);
            }
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                case 5:
                    break;
                case 1:
                    eneSetAngle(o, eneCourseAngle(o, 1) * 180.0f / 3.1415927f);
                    w->state = 0;
                    break;
                case 2:
                    w->anim->anim = obisuRRunAnim;
                    w->anim->frame = 0.0f;
                    w->flags &= ~1;
                    w->vel[1] = 0.8f;
                    soundIncamCam(0x2F0, w->anim->pos[0], w->anim->pos[1], w->anim->pos[2]);
                    w->timer = 0;
                    w->state = 13;
                    break;
                case 3:
                    eneSetAngle(o, eneCourseAngle(o, 1) * 180.0f / 3.1415927f);
                    w->atk->radius = 400;
                    w->state = 4;
                    break;
                case 4:
                    if (acollAtkWasHit(w->atk) == 1) {
                        DPRINTF(">>>  END プレーヤーきたぞ！\n");
                        w->vel[0] += 0.8f;
                        w->timer = 0;
                        w->state = 6;
                    }
                    break;
                case 6:
                    eneFaceMoveDir(o);
                    break;
                case 9:
                    if (!obisuPlCourseCheck(o)) {
                        return -1;
                    }
                    eneSetAngle(o, eneCourseAngle(o, 4) * 180.0f / 3.1415927f);
                    w->state = 7;
                    break;
                case 10:
                    p = obisuRBackPos(o);
                    if (!obisuPlCourseCheck(o)) {
                        return -1;
                    }
                    w->bubble = tbubbleCreate(o, p.x, p.y, p.z);
                    eneSetAngle(o, eneCourseAngle(o, 8) * 180.0f / 3.1415927f);
                    w->rot[1] += 3.1415927f;
                    w->state = 7;
                    break;
                case 11:
                    if (!obisuPlCourseCheck(o)) {
                        return -1;
                    }
                    eneSetAngle(o, eneCourseAngle(o, 8) * 180.0f / 3.1415927f);
                    w->state = 7;
                    break;
                case 12:
                    if (!obisuPlCourseCheck(o)) {
                        return -1;
                    }
                    eneSetAngle(o, eneCourseAngle(o, 8) * 180.0f / 3.1415927f);
                    eneMakeEx(o, ENE_BIJO1, w->zone, w->x, w->y + 2.0f, w->z, 0, NULL);
                    eneMakeEx(o, ENE_BIJO2, w->zone, w->x, w->y + 2.0f, w->z, 0, NULL);
                    w->state = 7;
                    break;
                case 7:
                    if (acollAtkWasHit(w->atk) == 1) {
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                plAskPos(gGameWork.players[i], &place, &height, &side, &zone);
                            }
                        }
                        if (zone == w->zone && eneNearestPlWay(o) == 1) {
                            DPRINTF(">>>  プレーヤーが右からきたので死ぬ\n");
                            return -1;
                        }
                        DPRINTF(">>>  プレーヤーきたぞ！さらに待て\n");
                        w->atk->radius = 150;
                        w->timer = 0;
                        w->state = 8;
                    }
                    break;
                case 8:
                    if (acollAtkWasHit(w->atk) == 1 || swWorkTest(0x37)) {
                        if (w->type == 4) {
                            itexPuffGrey(w->anim->pos[0], w->anim->pos[1] + 5.0f, w->anim->pos[2], 0.1f);
                            bijoStartFollow();
                        }
                        if (w->type == 2) {
                            eneSetAngle(o, eneCourseAngle(o, 1) * 180.0f / 3.1415927f);
                            if (w->bubble != NULL) {
                                (*tbubbleGetPos)(w->bubble, pos);
                                ptclSpawnSparksFall(pos[0], pos[1], pos[2]);
                                w->bubble = tskDestroy(w->bubble);
                            }
                        }
                        soundIncamCam(0x2F0, w->anim->pos[0], w->anim->pos[1], w->anim->pos[2]);
                        w->flags |= 2;
                        w->timer = 0;
                        w->state = 13;
                        w->vel[0] = 5.0f;
                    }
                    break;
                case 13:
                    w->anim->anim = obisuRJumpAnim;
                    w->anim->frame = 0.0f;
                    w->flags |= 1;
                    w->hit &= ~1;
                    w->timer = 0;
                    w->state = 14;
                    w->vel[1] = 1.5f;
                    itexPuffGrey(w->anim->pos[0], w->anim->pos[1] + 3.0f, w->anim->pos[2], 0.08f);
                    break;
                case 14:
                    if (polAnimeLastFrame(w->anim) / 2 < w->anim->frame) {
                        w->flags &= ~1;
                        w->vel[1] = 0.0f;
                    }
                    if (w->hit) {
                        w->vel[0] = 5.0f;
                        itexPuffGrey(w->anim->pos[0], w->anim->pos[1] + 3.0f, w->anim->pos[2], 0.08f);
                        soundIncamCam(0x2F0, w->anim->pos[0], w->anim->pos[1], w->anim->pos[2]);
                        w->timer = 0;
                        w->state = 13;
                    }
                    eneAnimPlayOnce(o, w->anim, 1.4f);
                    eneFaceMoveDir(o);
                    break;
            }
            if (w->flags != 0) {
                if (120 / gRetracesPerFrame < w->goTimer++) {
                    if (w->type == 4) {
                        bijoStopFollow();
                    }
                    DPRINTF(">>>  さようなら\n");
                    return -1;
                }
            }
            if (!(w->flags & 1)) {
                eneFall(o, 1.5f);
                w->hit &= ~1;
                if (eneLand((EneObj *) w, 10.0f) == 1) {
                    w->hit |= 1;
                    w->vel[1] = 0.0f;
                }
            }
            eneMove((EneObj *) w);
            if (w->anim != NULL) {
                eneSetModelOnCourse(o, &w->anim->f);
            }
            if (w->flags == 0) {
                eneAnimLoop(o, w->anim, w->animSpeed);
            }
            shadowPut(w->shadow, w->anim->pos[0], w->anim->pos[1], w->anim->pos[2], 2.0f, NULL);
            if (w->anim->pos[1] < 0.0f) {
                DPRINTF("obisu_run_proc: error: 高度が０以下になったのでしねや\n");
                return -1;
            }
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void obisuRSetState2(Obj *o) {
    ((ObisuRW *) FX(o))->state = 2;
}

f32 obisuRGetX(Obj *o) {
    return ((ObisuRW *) FX(o))->x;
}

extern u32 obisuRAnims[]; /* its animations */

/* Starts animation i from its first frame. */
void obisuRSetAnime(Obj *o, s32 i) {
    ((ObisuRW *) FX(o))->anim->anim = obisuRAnims[i];
    ((ObisuRW *) FX(o))->anim->frame = 0.0f;
}

/* Stops (state 0, no velocity). */
void obisuRStop(Obj *o) {
    ObisuRW *w = (ObisuRW *) FX(o);

    w->state = 0;
    w->vel[2] = 0.0f;
    w->vel[1] = 0.0f;
    w->vel[0] = 0.0f;
}

void obisu2UnusedStub4(void) {
}

/* Destroys its frame, if it has one. */
void obisuRFrameFree(Obj *o) {
    s32 pad;
    ObisuRW *w = (ObisuRW *) FX(o);

    if (w->frame != NULL) {
        w->frame = frmDestroy(w->frame);
    }
}

/* A place by obisu_r, returned by value: 12 behind it in x (by its heading), 6 in z, 8 up
 * (tlb_0129's obisuRunBackPos; the first model read through FX(o), the frame's pads measured). */
Vec3f obisuRBackPos(Obj *o) {
    s32 pad[2];
    Vec3f v;
    f32 x;
    f32 y;
    f32 z;
    ObisuRW *w = (ObisuRW *) FX(o);
    s32 pad2;

    x = w->anim->pos[0] + mathSin(((ObisuRW *) FX(o))->anim->rot[1]) * -12.0f;
    z = w->anim->pos[2] + mathCos(w->anim->rot[1]) * 6.0f;
    y = w->anim->pos[1] + 8.0f;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

/* Whether the player playing character 1 is in obisu_r's zone (its byte 0x01). */
s32 obisuPlCourseCheck(Obj *o) {
    s32 pad[3];
    f32 z;
    f32 y;
    f32 x;
    s32 zone;

    plAskPos(gGameWork.players[demoFindPlByChara(1)], &x, &y, &z, &zone);
    if (zone == ((ObisuRW *) FX(o))->zone) {
        DPRINTF("obisuPlCourseCheck: オビ＆エビ  コース同じです\n");
        return 1;
    }
    DPRINTF("obisuPlCourseCheck: オビ＆エビ  コースちがう\n");
    return 0;
}

void obisuRGetPos(Obj *o, f32 *x, f32 *y, f32 *z, s32 *b) {
    *x = ((ObisuRW *) FX(o))->x;
    *y = ((ObisuRW *) FX(o))->y;
    *z = ((ObisuRW *) FX(o))->z;
    *b = ((ObisuRW *) FX(o))->zone;
}

extern u32 obisuRRunAnim; /* obi_r_f's animation (running) */

/* The file's .data, in address order (its externs above stay as declarations). */
u32 obisuRAnims[] = { (u32) obisuMaru2StandAnim }; /* its animations */
u32 obisuRRunAnim = (u32) obisuMaru2WalkAnim; /* obisu_r running */
u32 obisuRJumpAnim = (u32) obisuMaru2JumpAnim; /* obisu_r jumping */
u32 obisuRUnusedAnims[] = { 0x030014EC, 0x030017E8, 0x03001164 };
u32 obisuRKind2Anim = 0x03001A00;
u32 obisuRKind3Anim = 0x03001D1C;
u32 obisuRKind4Anim = 0x03002010;

/* "obi_r_f"'s work. */
typedef struct ObiRFW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ MPolFrame *shadow;
    /* 0x40 */ u8 pad40[4];
    /* 0x44 */ u8 setup; /* eneSetup's result */
    /* 0x45 */ u8 noFall; /* 1: no gravity */
    /* 0x46 */ u8 pad46[2];
    /* 0x48 */ f32 animSpeed;
    /* 0x4C */ s32 timer;
} ObiRFW;

/* "obi_r_f": obisu_r on the run. In state 3 it turns to the course and, if a player is ahead of
 * it in its zone, runs on at once (state 5); otherwise, at random, it waits for a player to hit
 * it (state 4) or runs on (5). Running (6) it faces where it moves; below height 0 it dies.
 * w is set at its declaration with "s32 pad[3]". The setup value is masked to its byte: as1
 * deletes the andi before the sb, but the mask still spends the temporary the game spends. */
s32 obiRFProc(Obj *o, s16 msg, void *arg) {
    ObiRFW *w = (ObiRFW *) FX(o);
    s32 i;
    s32 r;
    s32 pad[3];

    switch (msg) {
        case 2:
            o->name = "obi_r_f";
            w->setup = eneSetup(o, arg) & 0xFF;
            w->model = eneNewAnime(o, obisuRRunAnim);
            w->state = 3;
            w->animSpeed = 0.5f;
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("obisu_run_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->atk = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 600, 0, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x66;
            w->dmg = ((DmgAddXYZFunc) acollDmgAddXYZ)(ene_dmg_root_frame, &w->model->f, 70, 160, 0, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->shadow = shadowCreate();
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
        case 0: {
            f32 best;
            f32 place;
            f32 height;
            f32 side;
            s32 zone;

            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                case 1:
                case 2:
                    break;
                case 3:
                    eneSetAngle(o, eneCourseAngle(o, 1) * 180.0f / 3.1415927f);
                    if (obisuPlCourseCheck(o) == 1) {
                        best = 0.0f;
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                plAskPos(gGameWork.players[i], &place, &height, &side, &zone);
                                best = place;
                            }
                        }
                        if (obisuRGetX(o) <= best) {
                            DPRINTF(">>>>       obisu プレーヤーの方が先いっとる、いそげ\n");
                            w->state = 5;
                            break;
                        }
                    }
                    r = fadeRand(100);
                    if (r < 50) {
                        DPRINTF(">>>>       obisu プレーヤー待ちrand=[%d]\n", r);
                        w->state = 4;
                    } else {
                        DPRINTF(">>>>       obisu とっとといってしまうrand=[%d]\n", r);
                        w->state = 5;
                    }
                    break;
                case 4:
                    if (acollAtkWasHit(w->atk) == 1) {
                        DPRINTF(">>>  END プレーヤーきたぞ！\n");
                        w->timer = 0;
                        w->state = 6;
                        w->vel[0] += 2.0f;
                    }
                    break;
                case 5:
                    DPRINTF(">>>  END プレーヤーきたぞ！\n");
                    w->timer = 0;
                    w->state = 6;
                    w->vel[0] += 2.0f;
                    break;
                case 6:
                    eneFaceMoveDir(o);
                    break;
            }
            if (!(w->noFall & 1)) {
                eneFall(o, 0.8f);
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
            eneAnimLoop(o, w->model, w->animSpeed);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 2.0f, NULL);
            if (w->model->pos[1] < 0.0f) {
                DPRINTF("obisu_run_proc: error: 高度が０以下になったのでしねや\n");
                return -1;
            }
            eneStepHookPost(o);
            w->timer++;
            break;
        }
    }
    return tskDefaultProc(o, msg, arg);
}

/* "bijo1" and "bijo2": an enemy's work with a shadow and a timer. */
typedef struct BijoW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ MPolFrame *shadow;
    /* 0x38 */ s32 timer;
} BijoW;

extern u8 bijo1Anim[]; /* bijo1's animation */

/* "bijo1": stands on the course until switch 0x33 is set, then follows obisu_r (the task's
 * parent) 6 a frame until switch 0x35 is set. */
s32 bijo1Proc(Obj *o, s16 msg, void *arg) {
    BijoW *w = (BijoW *) FX(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "bijo1";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) bijo1Anim);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("****   bijo1: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            eneSetAngle(o, eneCourseAngle(o, 8) * 180.0f / 3.1415927f);
            DPRINTF(">>>>>   bijoL セット完了\n");
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy((Frame *) w->model);
            }
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            swWorkReset(0x35);
            break;
        case 0: {
            f32 x;
            f32 y;
            f32 z;
            s32 zone;

            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (swWorkTest(0x33)) {
                        swWorkReset(0x33);
                        DPRINTF(">>> bijo1 押される\n");
                        w->timer = 0;
                        w->state = 1;
                    }
                    break;
                case 1:
                    w->state = 2;
                    break;
                case 2:
                    obisuRGetPos(o->parent, &x, &y, &z, &zone);
                    eneMoveToXZ(o, x, z, 6.0f, 6.0f);
                    eneFaceMoveDir(o);
                    if (swWorkTest(0x35)) {
                        swWorkReset(0x35);
                        DPRINTF(">>> bijo1 さよなら\n");
                        return -1;
                    }
                    break;
            }
            eneFall(o, 0.4f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneAnimLoop(o, w->model, 0.6f);
            eneMove((EneObj *) w);
            eneSetModelOnCourse(o, (Frame *) w->model);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
        }
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 bijo2Anim[]; /* bijo2's animation */

/* "bijo2": bijo1 on switches 0x34 and 0x36. */
s32 bijo2Proc(Obj *o, s16 msg, void *arg) {
    BijoW *w = (BijoW *) FX(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "bijo2";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) bijo2Anim);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("****   bijo2: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            eneSetAngle(o, eneCourseAngle(o, 8) * 180.0f / 3.1415927f);
            DPRINTF(">>>>>   bijoR セット完了\n");
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy((Frame *) w->model);
            }
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            swWorkReset(0x36);
            break;
        case 0: {
            f32 x;
            f32 y;
            f32 z;
            s32 zone;

            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (swWorkTest(0x34)) {
                        swWorkReset(0x34);
                        DPRINTF(">>> bijo2 押される\n");
                        w->timer = 0;
                        w->state = 1;
                    }
                    break;
                case 1:
                    w->state = 2;
                    break;
                case 2:
                    obisuRGetPos(o->parent, &x, &y, &z, &zone);
                    eneMoveToXZ(o, x, z, 6.0f, 6.0f);
                    eneFaceMoveDir(o);
                    if (swWorkTest(0x36)) {
                        swWorkReset(0x36);
                        DPRINTF(">>> bijo2 さよなら\n");
                        DPRINTF(">>  ころす\n");
                        return -1;
                    }
                    break;
            }
            eneFall(o, 0.4f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneAnimLoop(o, w->model, 0.6f);
            eneMove((EneObj *) w);
            eneSetModelOnCourse(o, (Frame *) w->model);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
        }
    }
    return tskDefaultProc(o, msg, arg);
}

/* Sets switches 0x33 and 0x34. */
void bijoStartFollow(void) {
    swWorkSet(0x33);
    swWorkSet(0x34);
}

/* Sets switches 0x35 and 0x36. */
void bijoStopFollow(void) {
    swWorkSet(0x35);
    swWorkSet(0x36);
}

/* "Tbubble"'s argument: only the position its starter sets. */
typedef struct TbubbleArg {
    /* 0x00 */ u8 pad00[0x8];
    /* 0x08 */ f32 pos[3];
    /* 0x14 */ u8 pad14[0x8];
} TbubbleArg;

extern u8 tbubbleDl[]; /* the bubble's display list */

/* "Tbubble": a bubble (segments from file 0xE7) that swells and shrinks by 0.02 a frame, turning
 * every 40 retraces, drawn white at alpha 0xE0. */
s32 tbubbleProc(Obj *obj, s16 msg, void *arg) {
    TbubbleW *w = (TbubbleW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Tbubble";
            if ((w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                return -1;
            }
            w->frame->dl2 = (Gfx *) tbubbleDl;
            w->frame->flags |= 0x27;
            floadGetSetRam((void **) w->frame->segs, 0xE7);
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = ((TbubbleArg *) arg)->pos[0];
            w->frame->pos[1] = ((TbubbleArg *) arg)->pos[1];
            w->frame->pos[2] = ((TbubbleArg *) arg)->pos[2];
            w->alpha = 0xE0;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = eneDlEnvPrimColor(obj, 0xFF, 0xFF, 0xFF, 0xA0, 0xA0, 0xA0, w->alpha);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->frame->scale[0] += 0.02f;
                    w->frame->scale[1] += 0.02f;
                    w->frame->scale[2] += 0.02f;
                    if (40 / gRetracesPerFrame < w->timer) {
                        w->timer = 0;
                        w->state = 1;
                    }
                    break;
                case 1:
                    w->frame->scale[0] -= 0.02f;
                    w->frame->scale[1] -= 0.02f;
                    w->frame->scale[2] -= 0.02f;
                    if (40 / gRetracesPerFrame < w->timer) {
                        w->timer = 0;
                        w->state = 0;
                    }
                    break;
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "Tbubble" at x, y, z under obj. */
Obj *tbubbleCreate(Obj *obj, f32 x, f32 y, f32 z) {
    TbubbleArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    return tskCreate(obj, tbubbleProc, obj->file, &arg);
}

/* The bubble's position. */
void tbubbleGetPos(Obj *o, f32 *pos) {
    pos[0] = ((TbubbleW *) FX(o))->frame->pos[0];
    pos[1] = ((TbubbleW *) FX(o))->frame->pos[1];
    pos[2] = ((TbubbleW *) FX(o))->frame->pos[2];
}
