#include "tlb.h"

/* File 133: the ghost Okiku ("okiku"), a demo talker who appears at night with a
 * will-o'-the-wisp until she has given the player her pass (tegata, flag 0xAD); and the cat bus
 * ("nekobus"), its light ("nekoL"), its stop ("ts_oya", "ts_atari") and its demos ("neko_D",
 * "nb_dero": Mokeke forest and Dero-dero village). */

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* Okiku's work, at its task + 0x20: a demo talker's (PersonObj) up to 0x44. */
typedef struct OkikuW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 pad09[3];
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
    /* 0x44 */ u8 flags; /* 1 talking, 2 flag 0x16 set (she moves faster), 4 the sky's tunes change */
    /* 0x48 */ s32 alpha; /* its env colour's alpha (0xFF when shown) */
    /* 0x4C */ s32 timer; /* ticks in the state */
    /* 0x50 */ Obj *pass; /* the pass (kind 0x1C3), made between it and the player */
    /* 0x54 */ u8 pad54[4];
    /* 0x58 */ Obj *onibi; /* its will-o'-the-wisp (kind 0x1C6) */
    /* 0x5C */ s32 wave;
    /* 0x60 */ Camera *cam;
    /* 0x64 */ u8 tuneTimer;
    /* 0x65 */ s8 partake; /* gGameWork.partake while she talks */
} OkikuW;

#define OKIKU(obj) ((OkikuW *) FX(obj))

/* The will-o'-the-wisp's work (kind 0x1C6, another file's: tlb_0114's "gOnibi"), as far as this
 * file sets it. */
typedef struct OnibiW {
    /* 0x00 */ u8 unused00;
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
    /* 0x34 */ PolFrame *flames[3];
    /* 0x40 */ Anim anim[3];
    /* 0x70 */ s32 wave;
    /* 0x74 */ s32 alpha;
    /* 0x78 */ u8 state; /* 2 going out */
} OnibiW;

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

/* The cat bus's work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct NekoBusW {
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
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ Obj *light; /* its light (kind 0x1E8, "nekoL") */
    /* 0x38 */ s32 timer; /* ticks in the state */
    /* 0x3C */ s8 partake; /* gGameWork.partake before it came */
} NekoBusW;

#define NEKOBUS(obj) ((NekoBusW *) FX(obj))

/* The cat bus light's work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct NekoLightW {
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
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ s32 timer;
} NekoLightW;

#define NEKOLIGHT(obj) ((NekoLightW *) FX(obj))

/* The bus stop's work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct TsOyaW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model; /* where it is */
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u8 pad34[0x10];
    /* 0x44 */ Camera *cam;
    /* 0x48 */ s32 timer; /* ticks in the state */
    /* 0x4C */ s32 count; /* ticks every player has stood at the stop */
    /* 0x50 */ s8 partake; /* restored to gGameWork.partake at the end (never set here) */
} TsOyaW;

#define TSOYA(obj) ((TsOyaW *) FX(obj))

/* A player mark's work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct TsAtariW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model; /* where it is */
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *box;
    /* 0x38 */ s32 timer;
    /* 0x3C */ u32 player; /* eneSetup's: the player it is for (plGetIndex) */
    /* 0x40 */ u8 flags; /* 1 its player touches it */
} TsAtariW;

#define TSATARI(obj) ((TsAtariW *) FX(obj))

/* The Dero-dero bus demo's work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct NekoDemoW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model; /* where it is */
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ Obj *bus; /* the cat bus ("nb_dero", kind 0x1EC) */
    /* 0x38 */ Camera *cam;
    /* 0x3C */ s32 timer; /* ticks in the state */
    /* 0x40 */ s8 partake; /* gGameWork.partake before the demo */
} NekoDemoW;

#define NEKODEMO(obj) ((NekoDemoW *) FX(obj))

/* The Dero-dero cat bus's work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct NbDeroW {
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
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ Obj *light; /* its light (kind 0x1E8, "nekoL") */
    /* 0x38 */ u8 pad38[4];
    /* 0x3C */ s32 timer; /* ticks in the state */
} NbDeroW;

#define NBDERO(obj) ((NbDeroW *) FX(obj))

void nekobusPassGive(Obj *o, Obj *player);
void nekobusOnibiOut(Obj *o);
void nekobusLightSetPos(Obj *o, f32 x, f32 y, f32 z, f32 ry);
void nekobusStopEnd(Obj *o);
void nekobusDeroRunIn(Obj *o);
void nekobusDeroDoorOpen(Obj *o);
void nekobusDeroDoorClose(Obj *o);
void nekobusDeroKneel(Obj *o);
void nekobusDeroLeave(Obj *o);
s32 nekobusDeroIsWaiting(Obj *o);

Obj *nekobusStopTask = (Obj *) 0x12345670; /* the bus stop ("ts_oya") */

extern u8 nekobusOkikuAnim[];
extern u8 nekobusAnim[];
extern u8 nekobusState2Anim[];
extern u8 nekobusLightAnim[];

void nekobusNop(void) {
}

/* The pass goes to the player (state 3). */
void nekobusPassGive(Obj *o, Obj *player) {
    PassW *w = (PassW *) FX(o);

    w->state = 3;
    w->player = player;
    w->timer = 0;
}

void nekobusTegataSdSet(void) {
    DPRINTF("&&&& eventTegataSdSet  pl D がないn");
}

void nekobusTegataSdClear(void) {
    DPRINTF("&&&& SD_G_CLEAR1n");
    DPRINTF("&&&& SD_E_CLEAR1n");
    DPRINTF("&&&& SD_S_CLEAR1n");
    DPRINTF("&&&& SD_Y_CLEAR1n");
}

/* "okiku": the ghost Okiku. Made only until she has given the pass (0xAD); shown at night
 * (before 4:00 or from 21:00), hidden by day. Talked to, she turns the camera on her, talks
 * (scenario 0x297), and if the player takes on her errand (0x93) leaves a pass between them that
 * sparks and goes to the player (scenario 0x20); then she fades out, her wisp goes out, and she
 * rests (0xAD) or waits for night. */
s32 nekobusOkikuProc(Obj *o, s16 msg, void *arg) {
    OkikuW *w = OKIKU(o);
    u32 data;
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "okiku";
            if (swWorkTest(0xAD)) {
                DPRINTF("****  手形をもらっている\n");
                return -1;
            }
            data = eneSetup(o, arg);
            w->model = eneAnimeModelAlpha(o, (u32) nekobusOkikuAnim, 0);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 0xA0, 0, 0xA0, 0, 0);
            if (w->box == NULL) {
                return -1;
            }
            demoTalkInit(o, data);
            if (gGameWork.time < 0x150000 && gGameWork.time > 0x40000) {
                DPRINTF("**********   鬼火でません\n");
                DPRINTF("**********   手形でません\n");
                w->alpha = 0;
                w->model->f.flags &= ~2;
                w->state = 7;
                break;
            }
            w->onibi = eneMakeEx(NULL, ENE_SHOUNIN_ONIBI, 0, w->model->pos[0], w->model->pos[1], w->model->pos[2], 0, NULL);
            w->alpha = 0xFF;
            break;
        case 8:
            return demoTalk(o, arg);
        case 3:
            w->model->dl = gGfxDlHead;
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            demoTalkerFree(o);
            break;
        case 0: {
            Vec3f pos;
            s32 i;
            s32 pad2;
            f32 x;
            f32 y;
            f32 z;
            Vec3f spark;

            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i) && !(gGameWork.players[i]->flags & 2)) {
                            tskSendMsg(gGameWork.players[i], 4, &pos);
                            eneTurnToPoint(o, pos.x, pos.z, 10.0f);
                            break;
                        }
                    }
                    if (demoTalkEndCheck(o)) {
                        camSunSetClockStop(1);
                        w->partake = gGameWork.partake;
                        gGameWork.partake = 1;
                        w->timer = 0;
                        w->state = 1;
                    }
                    if (gGameWork.time < 0x150000 && gGameWork.time > 0x40000) {
                        DPRINTF(">>>    おきく消える......");
                        w->timer = 0;
                        w->state = 6;
                    }
                    break;
                case 1:
                    if (w->timer == 1) {
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        camSetSpeed(w->cam, 40 / gRetracesPerFrame);
                        camMoveEye(w->cam, 1034.0f, 340.0f, 732.0f);
                        camMoveAt(w->cam, 915.0f, 382.0f, 635.0f);
                    }
                    if (!camGetMoving(w->cam)) {
                        w->timer = 0;
                        w->state = 2;
                    }
                    break;
                case 2:
                    scRunStart2(0x297, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    w->flags |= 1;
                    w->state = 3;
                    break;
                case 3:
                    if (swWorkTest(0x16)) {
                        w->flags |= 2;
                    } else {
                        w->flags &= ~2;
                    }
                    if (!scIsRunning()) {
                        if (swWorkTest(0x93)) {
                            swWorkReset(0x93);
                            w->state = 4;
                        } else {
                            w->flags &= ~1;
                            gGameWork.partake = w->partake;
                            camSunSetClockStop(0);
                            camAiSetHold(0);
                            demoFinish(o);
                            w->state = 0;
                        }
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
                        if (w->pass != NULL) {
                            demoGetEneModelPos(w->pass, &spark);
                        }
                        ptclSpawnSparksFall(spark.x, spark.y, spark.z);
                        eneMakeEx(NULL, ENE_DEMO_KIRAKIRA, 0, spark.x, spark.y, spark.z, 0, NULL);
                        if (w->pass != NULL) {
                            nekobusPassGive(w->pass, w->talker);
                        }
                        w->flags |= 4;
                    }
                    if (w->timer == 260 / gRetracesPerFrame) {
                        scRunStart2(0x20, 0, 0);
                        w->state = 6;
                    }
                    break;
                case 6:
                    if (w->alpha-- < 1) {
                        w->alpha = 0;
                        if (w->onibi != NULL) {
                            nekobusOnibiOut(w->onibi);
                        }
                        if (swWorkTest(0xAD)) {
                            DPRINTF("成仏\n");
                            gGameWork.partake = w->partake;
                            camSunSetClockStop(0);
                            camAiSetHold(0);
                            return -1;
                        }
                        DPRINTF("夜待ち\n");
                        w->model->f.flags &= ~2;
                        w->state = 7;
                    }
                    break;
                case 7:
                    if (gGameWork.time > 0x150000) {
                        DPRINTF("**********   鬼火でません\n");
                        DPRINTF(">>>    出現\n");
                        w->wave = 0;
                        w->y = 346.0f;
                        w->model->f.flags |= 2;
                        w->onibi = eneMakeEx(NULL, ENE_SHOUNIN_ONIBI, 0, w->model->pos[0], w->model->pos[1],
                                             w->model->pos[2], 0, NULL);
                        w->state = 8;
                    }
                    break;
                case 8:
                    if (w->alpha++ >= 0xFF) {
                        w->alpha = 0xFF;
                        w->state = 0;
                    }
                    break;
            }
            if (w->flags & 4) {
                if (300 / gRetracesPerFrame == w->tuneTimer++) {
                    w->flags &= ~4;
                    camSunClearTunes();
                    camSunSetTunes5(0x518, 0x5A, 0x5B, 0x5C, 0x5D);
                }
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            if (w->flags & 2) {
                eneAnimLoop(o, w->model, 1.4f);
            } else {
                eneAnimLoop(o, w->model, 0.8f);
            }
            eneStepHookPost(o);
            w->timer++;
            break;
        }
    }
    return tskDefaultProc(o, msg, arg);
}

/* The will-o'-the-wisp goes out (state 2). */
void nekobusOnibiOut(Obj *o) {
    ((OnibiW *) FX(o))->state = 2;
}

/* "nekobus": the cat bus of the Mokeke forest demo. It runs in to the stop (1181, 54), lurches
 * to a halt, opens its door, shakes and sets the players down at (1193, 129), shakes again with
 * the pad rumbling, and runs off to (1033, 171); 120 ticks on, the stop ends its wait. */
s32 nekobusProc(Obj *o, s16 msg, void *arg) {
    NekoBusW *w = NEKOBUS(o);

    switch (msg) {
        case 2:
            o->name = "nekobus";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) nekobusAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 0.3f;
            w->light = eneMakeEx(NULL, ENE_NEKOBUS_LIGHT, 0, w->model->pos[0], w->model->pos[1], w->model->pos[2], 0, NULL);
            DPRINTF(">>>>  ネコバス set ok\n");
            camSunSetClockStop(1);
            w->partake = gGameWork.partake;
            gGameWork.partake = 1;
            gGameWork.scenarioRunning = 1;
            break;
        case 3:
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (eneMoveToXZ(o, 1181.0f, 54.0f, 1.5f / gRetracesPerFrame, 1.5f / gRetracesPerFrame)) {
                        DPRINTF("&&&&&   bus stop\n");
                        w->timer = 0;
                        w->state = 1;
                        w->vel[0] = w->vel[1] = w->vel[2] = 0.0f;
                    }
                    eneAnimLoop(o, w->model, 0.35f);
                    break;
                case 1:
                    if (w->timer < 4 / gRetracesPerFrame) {
                        DPRINTF(">>> がくん1\n");
                        eneAnimLoop(o, w->model, 0.35f);
                        w->vel[0] = -0.3f;
                    } else if (w->timer < 8 / gRetracesPerFrame) {
                        DPRINTF(">>> がくん2\n");
                        eneAnimLoop(o, w->model, -0.35f);
                        w->vel[0] = 0.3f;
                    } else {
                        w->vel[0] = 0.1f;
                        if (eneAnimPlayOnce(o, w->model, -0.35f)) {
                            DPRINTF(">>> やってくる終了\n");
                            w->timer = 0;
                            w->state = 2;
                            w->vel[0] = 0.0f;
                        }
                    }
                    break;
                case 2:
                    if (w->timer == 1) {
                        w->model->anim = nekobusState2Anim;
                        w->model->frame = 0.0f;
                    }
                    if (eneAnimPlayOnce(o, w->model, 0.02f)) {
                        w->timer = 0;
                        w->state = 3;
                    }
                    break;
                case 3:
                    if (w->timer >= 60 / gRetracesPerFrame) {
                        w->timer = 0;
                        w->state = 4;
                    }
                    break;
                case 4:
                    if (w->timer < 12 / gRetracesPerFrame) {
                        w->y -= 0.09f;
                    } else if (w->timer < 30 / gRetracesPerFrame) {
                        w->y += 0.04f;
                    } else {
                        demoSetPlPos(1, 1193.0f, 129.0f, 0.0f, 0.0f, 1, 1193.0f, 129.0f, 0.0f, 0.0f, 1, 1193.0f,
                                     129.0f, 0.0f, 0.0f, 1, 1193.0f, 129.0f, 0.0f, 0.0f);
                        w->timer = 0;
                        w->state = 5;
                    }
                    break;
                case 5:
                    if (w->timer == 1) {
                        w->model->anim = nekobusState2Anim;
                        w->model->frame = polAnimeLastFrame(w->model);
                        fadeRumbleAll(0x66600000, 15);
                    }
                    if (eneAnimPlayOnce(o, w->model, -0.05f)) {
                        w->timer = 0;
                        w->state = 6;
                    }
                    break;
                case 6:
                    if (w->timer >= 60 / gRetracesPerFrame) {
                        w->timer = 0;
                        w->state = 7;
                    }
                    break;
                case 7:
                    if (w->timer == 1) {
                        fadeRumbleAll(0x66655544, 50);
                        w->model->anim = nekobusAnim;
                        w->model->frame = 0.0f;
                    }
                    eneAnimLoop(o, w->model, 1.75f);
                    if (!(w->timer & 1)) {
                        DPRINTF("FORWARDDDDDD\n");
                        w->y += 0.2f;
                    } else {
                        DPRINTF("BACKKKKKKKKK\n");
                        w->y -= 0.2f;
                    }
                    if (w->timer > 50 / gRetracesPerFrame) {
                        DPRINTF("ENDDDDDDDDDDDDD\n");
                        fadeRumbleAll(0x66664444, 80);
                        w->timer = 0;
                        w->state = 8;
                    }
                    break;
                case 8:
                    if (eneMoveToXZ(o, 1033.0f, 171.0f, 1.0f / gRetracesPerFrame, 1.0f / gRetracesPerFrame)) {
                        w->timer = 0;
                        w->state = 10;
                    }
                    eneTurnToAngle(o, 50.0f, 0.3f);
                    eneAnimLoop(o, w->model, 0.35f);
                    if (w->timer == 120 / gRetracesPerFrame) {
                        nekobusStopEnd(nekobusStopTask);
                    }
                    break;
                case 10:
                    break;
            }
            nekobusLightSetPos(w->light, w->model->pos[0], w->model->pos[1], w->model->pos[2],
                                   w->model->rot[1]);
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            w->timer++;
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "nekoL": the cat bus's light, a model the bus carries along (nekobusLightSetPos). */
s32 nekobusLightProc(Obj *o, s16 msg, void *arg) {
    NekoLightW *w = NEKOLIGHT(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "nekoL";
            eneSetup(o, arg);
            w->model = eneAnimeModelMake(o, (u32) nekobusLightAnim, NULL, 1);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 0.3f;
            DPRINTF(">>>>  ネコバスライト set ok\n");
            break;
        case 3:
            w->model->dl = eneDlEnvPrimColor(o, 0xFF, 0xFF, 0, 0xFF, 0xFF, 0xFF, 0x40);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            if (w->state == 0) {
                eneAnimLoop(o, w->model, 0.3f);
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            w->timer++;
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The light goes to the bus: at (x, y, z), turned ry. */
void nekobusLightSetPos(Obj *o, f32 x, f32 y, f32 z, f32 ry) {
    NekoLightW *w = NEKOLIGHT(o);

    w->zone = 0;
    w->x = x;
    w->y = y;
    w->z = z;
    w->rot[1] = ry;
}

/* "ts_oya": the Mokeke forest bus stop. Made once the game is finished (allClear) and until its
 * demo has run (flag 0x44). From 2:00 to 4:00 it waits for every player in play to stand at the
 * stop (x 1180-1215) for 120 ticks; then it sets 0x44, puts the players in a demo, calls the cat
 * bus (kind 0x1E5) and turns the camera on it; when the bus has gone (nekobusStopEnd) the
 * letterbox closes and the players go to stage 0x8A at 5:00, with areas 2-9 and 3-6 open. */
s32 nekobusStopProc(Obj *o, s16 msg, void *arg) {
    TsOyaW *w = TSOYA(o);
    s32 i;
    Player *pl;
    s32 pad[3];
    Vec3f pos;
    s32 zone;
    s32 pad2;
    s32 pad3;

    switch (msg) {
        case 2:
            o->name = "ts_oya";
            if (swWorkTest(0x44)) {
                DPRINTF("****  DEBUG: もけけの森のバスデモ行ってます\n");
                return -1;
            }
            if (gSaveData.allClear == 0) {
                DPRINTF("*****  ネコバス: まだゲームクリアしていない\n");
                return -1;
            }
            eneSetup(o, arg);
            w->model = eneNewPol(o, NULL);
            if (w->model == NULL) {
                return -1;
            }
            nekobusStopTask = o;
            DPRINTF(">>>  neko bus セーットッ！！\n");
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (gGameWork.time >= 0x20000 && gGameWork.time <= 0x40000) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    w->count++;
                    DPRINTF("\rP100timer[%d]", w->count);
                    if (gGameWork.time >= 0x40000) {
                        w->state = 1;
                    }
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i) == 1) {
                            plAskPos(gGameWork.players[i], &pos.x, &pos.y, &pos.z, &zone);
                            if (pos.x < 1180.0f || 1215.0f < pos.x) {
                                w->count = 0;
                                break;
                            }
                        }
                    }
                    if (w->count >= 120 / gRetracesPerFrame) {
                        DPRINTF(">>>  ネコバスきそう(^o^)/\n");
                        w->timer = 0;
                        w->state = 2;
                    }
                    break;
                case 2:
                    swWorkSet(0x44);
                    DPRINTF("*****  debug : ネコバス出発デモフラグＯＮ\n");
                    gGameWork.scenarioRunning = 1;
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i) == 1) {
                            plEnterDemo(gGameWork.players[i]);
                            pl = PL(gGameWork.players[i]);
                            pl->flags |= 0x10000000;
                            pl->model->rot[1] = 0.0f;
                        }
                    }
                    demoPlStandAll();
                    eneMakeEx(o, ENE_NEKOBUS, 1, 1341.0f, -87.0f, 54.0f, 0, NULL);
                    camAiSetHold(1);
                    w->cam = camAiGetCam();
                    camSetSpeed(w->cam, 180 / gRetracesPerFrame);
                    camMoveEye(w->cam, -53.0f, -58.0f, 390.0f);
                    camMoveAt(w->cam, 26.0f, -62.0f, 250.0f);
                    sndPlay(0x16B);
                    w->timer = 0;
                    w->state = 4;
                    break;
                case 3:
                    if (w->timer == 1) {
                        gGameWork.scenarioRunning = 2;
                    }
                    if (fadeCloseLetterbox(120) == 1) {
                        gGameWork.time = 0x50000;
                        gGameWork.scenarioRunning = 0;
                        gGameWork.partake = w->partake;
                        camSunSetClockStop(0);
                        camAiSetHold(0);
                        gSaveData.stageFlags[54] |= 1;
                        gSaveData.stageFlags[41] |= 1;
                        stageGotoSpawn(gStageTask->parent, 0x8A, 0, -876.0f, -6.0f, 24.0f, 3.1415927f, 0xD7);
                    }
                    if (w->timer == 156 / gRetracesPerFrame) {
                        sndPlay(10);
                    }
                    break;
                case 4:
                    break;
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

/* The bus has gone: the stop closes the demo (state 3). */
void nekobusStopEnd(Obj *o) {
    TsOyaW *w = TSOYA(o);

    w->timer = 0;
    w->state = 3;
}

/* "ts_atari": a player's mark at the bus stop (eneSetup's value is the player): its flag 1 is set
 * while that player touches it. */
s32 nekobusStopMarkProc(Obj *o, s16 msg, void *arg) {
    TsAtariW *w = TSATARI(o);

    switch (msg) {
        case 2:
            o->name = "ts_atari";
            w->player = eneSetup(o, arg);
            w->model = eneNewPol(o, NULL);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 100, 0, 0);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->box != NULL) {
                frmDestroy(&w->box->frame);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (acollAtkWasHit(w->box) == 1) {
                        if (plGetIndex((Obj *) w->box->hitPower) == w->player) {
                            DPRINTF(">>>  担当プレーヤー[%d]Pがあたった\n", w->player);
                            w->flags |= 1;
                        } else {
                            DPRINTF(">>>  担当プレーヤー[%d]Pがあたっていない\n", w->player);
                            w->flags &= ~1;
                        }
                    } else {
                        DPRINTF(">>>  担当プレーヤー[%d]Pがあたっていない\n", w->player);
                        w->flags &= ~1;
                    }
                    break;
                case 1:
                    break;
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

void nekobusNop2(void) {
}

/* "neko_D": the Dero-dero village bus demo. Made once the game is finished and the Mokeke forest
 * demo has run (0x44) but not this one (0x45). From black it fades in on the cat bus ("nb_dero",
 * kind 0x1EC), which comes in, stops, opens its door, lets the players out at the village, shakes
 * and leaves; then it sets 0x45 and gives the players back. */
s32 nekobusDeroDemoProc(Obj *o, s16 msg, void *arg) {
    NekoDemoW *w = NEKODEMO(o);

    switch (msg) {
        case 2:
            o->name = "neko_D";
            if (gSaveData.allClear == 0) {
                DPRINTF("*****  DEBUG:ネコバス: まだゲームクリアしていない\n");
                return -1;
            }
            if (!swWorkTest(0x44)) {
                DPRINTF("****  DEBUG: もけけの森のバスデモ行ってません\n");
                return -1;
            }
            if (swWorkTest(0x45)) {
                DPRINTF("****  DEBUG: デロデロバスデモおわってます\n");
                return -1;
            }
            eneSetup(o, arg);
            w->model = eneNewPol(o, NULL);
            if (w->model == NULL) {
                return -1;
            }
            fadeSetAll(120 / gRetracesPerFrame, 255.0f, 0.0f, 0, 0, 0);
            camSunSetClockStop(1);
            camAiSetHold(1);
            w->cam = camAiGetCam();
            camSetEye(w->cam, -984.0f, 99.0f, 59.0f);
            camSetAt(w->cam, -1047.0f, 38.0f, -75.0f);
            w->partake = gGameWork.partake;
            gGameWork.partake = 1;
            gGameWork.scenarioRunning = 2;
            DPRINTF(">>>  neko bus デモセット！！デロデロ村\n");
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (w->timer == 0) {
                        w->bus = eneMakeEx(NULL, ENE_NEKOBUS_DERO, 0, -920.0f, -23.0f, -45.0f, 0, NULL);
                        camSetSpeed(w->cam, 180 / gRetracesPerFrame);
                        camMoveEye(w->cam, -1019.0f, -8.0f, 35.0f);
                        camMoveAt(w->cam, -1068.0f, 31.0f, -112.0f);
                    }
                    if (w->timer == 120 / gRetracesPerFrame) {
                        nekobusDeroRunIn(w->bus);
                        w->timer = 0;
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (!camGetMoving(w->cam)) {
                        sndPlay(0x16C);
                        w->timer = 0;
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (nekobusDeroIsWaiting(w->bus)) {
                        w->timer = 0;
                        w->state = 3;
                    }
                    break;
                case 3:
                    if (w->timer == 60 / gRetracesPerFrame) {
                        nekobusDeroDoorOpen(w->bus);
                        w->timer = 0;
                        w->state = 4;
                    }
                    break;
                case 4:
                    if (nekobusDeroIsWaiting(w->bus)) {
                        demoSetPlPos(0, -1050.0f, -12.0f, -80.0f, 0.0f, 0, -1045.0f, -12.0f, -82.0f, 0.0f, 0,
                                     -1030.0f, -12.0f, -84.0f, 0.0f, 0, -1025.0f, -12.0f, -82.0f, 0.0f);
                        demoPlStandAll();
                        w->timer = 0;
                        w->state = 5;
                    }
                    break;
                case 5:
                    if (w->timer == 60 / gRetracesPerFrame) {
                        nekobusDeroKneel(w->bus);
                        w->timer = 0;
                        w->state = 6;
                    }
                    break;
                case 6:
                    if (nekobusDeroIsWaiting(w->bus)) {
                        w->timer = 0;
                        w->state = 7;
                    }
                    break;
                case 7:
                    if (w->timer == 60 / gRetracesPerFrame) {
                        nekobusDeroDoorClose(w->bus);
                        w->timer = 0;
                        w->state = 8;
                    }
                    break;
                case 8:
                    if (nekobusDeroIsWaiting(w->bus)) {
                        w->timer = 0;
                        w->state = 9;
                    }
                    break;
                case 9:
                    if (nekobusDeroIsWaiting(w->bus)) {
                        nekobusDeroLeave(w->bus);
                        w->timer = 0;
                        w->state = 10;
                    }
                    break;
                case 10:
                    if (nekobusDeroIsWaiting(w->bus)) {
                        if (w->bus != NULL) {
                            tskDestroy(w->bus);
                        }
                        w->timer = 0;
                        w->state = 11;
                    }
                    break;
                case 11:
                    swWorkSet(0x45);
                    camSunSetClockStop(0);
                    camAiSetHold(0);
                    gGameWork.partake = w->partake;
                    gGameWork.scenarioRunning = 0;
                    demoPlReleaseAll();
                    return -1;
                case 12:
                    demoPlReleaseAll();
                    break;
            }
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "nb_dero": the cat bus of the Dero-dero village demo, driven by "neko_D": each step it is set
 * to (nekobusDeroRunIn and the others) runs and then waits in state 12. 0: runs in to x -1076;
 * 1: lurches to a halt; 2-3: opens its door; 4-6: sinks and rises with the pad rumbling;
 * 7-8: closes the door; 9-10: shakes, puffing smoke; 11: runs off to x -1169. */
s32 nekobusDeroProc(Obj *o, s16 msg, void *arg) {
    NbDeroW *w = NBDERO(o);

    switch (msg) {
        case 2:
            o->name = "nb_dero";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) nekobusAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 0.3f;
            w->light = eneMakeEx(o, ENE_NEKOBUS_LIGHT, 0, w->model->pos[0], w->model->pos[1], w->model->pos[2], 0, NULL);
            w->state = 12;
            DPRINTF(">>>>  ネコバス set ok\n");
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    w->vel[0] = -0.6f;
                    if (w->x < -1076.0f) {
                        w->timer = 0;
                        w->state = 1;
                    }
                    eneAnimLoop(o, w->model, 0.35f);
                    break;
                case 1:
                    if (w->timer < 4 / gRetracesPerFrame) {
                        DPRINTF(">>> がくん1\n");
                        eneAnimLoop(o, w->model, 0.35f);
                        w->vel[0] = -0.3f;
                    } else if (w->timer < 8 / gRetracesPerFrame) {
                        DPRINTF(">>> がくん2\n");
                        eneAnimLoop(o, w->model, -0.35f);
                        w->vel[0] = 0.3f;
                    } else if (eneAnimPlayOnce(o, w->model, -0.35f)) {
                        DPRINTF(">>> やってくる終了\n");
                        w->state = 12;
                        w->vel[0] = 0.0f;
                    }
                    break;
                case 2:
                    DPRINTF(">>> ドアオープン\n");
                    w->model->anim = nekobusState2Anim;
                    w->model->frame = 0.0f;
                    w->state = 3;
                    break;
                case 3:
                    if (eneAnimPlayOnce(o, w->model, 0.02f)) {
                        w->timer = 0;
                        w->state = 12;
                    }
                    break;
                case 7:
                    w->model->anim = nekobusState2Anim;
                    w->model->frame = polAnimeLastFrame(w->model);
                    w->state = 8;
                    break;
                case 8:
                    if (eneAnimPlayOnce(o, w->model, -0.05f)) {
                        w->timer = 0;
                        w->state = 12;
                    }
                    break;
                case 4:
                    fadeRumbleAll(0x66600000, 15);
                    w->timer = 0;
                    w->state = 5;
                    break;
                case 5:
                    if (w->timer >= 18 / gRetracesPerFrame) {
                        w->timer = 0;
                        w->state = 6;
                    }
                    w->y -= 0.09;
                    break;
                case 6:
                    if (w->timer >= 40 / gRetracesPerFrame) {
                        w->timer = 0;
                        w->state = 12;
                    }
                    w->y += 0.01;
                    break;
                case 9:
                    DPRINTF(">>> ゆれる\n");
                    w->model->anim = nekobusAnim;
                    w->model->frame = 0.0f;
                    fadeRumbleAll(0x66655544, 50);
                    w->timer = 0;
                    w->state = 10;
                    break;
                case 10:
                    eneAnimLoop(o, w->model, 1.75f);
                    if (w->timer % (20 / gRetracesPerFrame) == 0) {
                        itexPuffWhite(fadeRand(30) / 10 - 1038, -8.0f, -12.0f, 0.2f);
                    }
                    if (w->timer > 50 / gRetracesPerFrame) {
                        DPRINTF(">>> いってしまう\n");
                        fadeRumbleAll(0x66664444, 80);
                        w->timer = 0;
                        w->state = 11;
                    }
                    if (!(w->timer & 1)) {
                        w->y += 0.1f;
                    } else {
                        w->y -= 0.1f;
                    }
                    break;
                case 11:
                    w->vel[0] -= 0.03;
                    if (w->x < -1169.0f) {
                        w->state = 12;
                    }
                    eneAnimLoop(o, w->model, 0.35f);
                    break;
                case 12:
                    break;
            }
            nekobusLightSetPos(w->light, w->model->pos[0], w->model->pos[1], w->model->pos[2],
                                   w->model->rot[1]);
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            w->timer++;
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The Dero-dero bus runs in (state 0). */
void nekobusDeroRunIn(Obj *o) {
    NBDERO(o)->state = 0;
}

/* It opens its door (state 2). */
void nekobusDeroDoorOpen(Obj *o) {
    NBDERO(o)->state = 2;
}

/* It closes its door (state 7). */
void nekobusDeroDoorClose(Obj *o) {
    NBDERO(o)->state = 7;
}

/* It sinks and rises (state 4). */
void nekobusDeroKneel(Obj *o) {
    NBDERO(o)->state = 4;
}

/* It shakes and leaves (state 9). */
void nekobusDeroLeave(Obj *o) {
    NBDERO(o)->state = 9;
}

/* Whether it waits (state 12), done with its step. */
s32 nekobusDeroIsWaiting(Obj *o) {
    if (NBDERO(o)->state == 12) {
        return TRUE;
    }
    return FALSE;
}
