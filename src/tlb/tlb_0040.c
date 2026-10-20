#include "tlb.h"

/* A scene of six kinds that work together: the director ("main", which makes the others and
 * runs the cutscenes), the spirit, the oni, the door, the pillars and the fragments ("hahen",
 * "hahen2"). The director answers message 10 with argument MAIN_ID with itself, so the others
 * find it through the enemy manager. */

#define MAIN_ID 0x14E4D1D

/* The director's work. */
typedef struct MainWork {
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
    /* 0x34 */ Obj *spirit; /* kind 0x22 */
    /* 0x38 */ Obj *oni; /* kind 0x23 */
    /* 0x3C */ Obj *door; /* kind 0x24 */
    /* 0x40 */ Obj *pillars[3]; /* kind 0x25 */
    /* 0x4C */ Obj *gauge; /* the pip gauge */
    /* 0x50 */ s32 timer;
    /* 0x54 */ s32 unused54;
    /* 0x58 */ s32 arrived; /* the players' walk ended */
    /* 0x5C */ s32 count; /* the gauge's pips */
} MainWork;

/* The spirit's work. */
typedef struct SpiritWork {
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
    /* 0x34 */ AnimeFrame *model;
    /* 0x38 */ PolFrame *glow;
    /* 0x3C */ MPolFrame *shadow;
    /* 0x40 */ ACollFrame *dmg;
    /* 0x44 */ s32 timer;
    /* 0x48 */ s32 sounded;
    /* 0x4C */ s32 sndTimer;
    /* 0x50 */ s32 bounces;
    /* 0x54 */ u8 pad54[4];
    /* 0x58 */ s32 flash; /* after a hit */
    /* 0x5C */ s32 safe; /* hits do nothing */
    /* 0x60 */ u8 alpha;
    /* 0x61 */ u8 glowAlpha;
} SpiritWork;

/* The oni's work. */
typedef struct OniWork {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09;
    /* 0x0A */ u8 hit; /* 1: on the ground */
    /* 0x0B */ u8 pad0B[5];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ AnimeFrame *body;
    /* 0x38 */ PolFrame *body2; /* its rolled-up shape */
    /* 0x3C */ PolFrame *glow;
    /* 0x40 */ RideFrame *ride;
    /* 0x44 */ MPolFrame *shadow;
    /* 0x48 */ ACollFrame *atk;
    /* 0x4C */ ACollFrame *aniAtk;
    /* 0x50 */ ACollFrame *dmg;
    /* 0x54 */ s32 tick; /* 0-9 */
    /* 0x58 */ s32 timer;
    /* 0x5C */ s32 dustTick; /* 0-5 */
    /* 0x60 */ s32 rumble;
    /* 0x64 */ s32 count;
    /* 0x68 */ Obj *rollSnd;
    /* 0x6C */ u8 pad6C[4];
    /* 0x70 */ s32 wait;
    /* 0x74 */ u32 steps; /* the attack pattern's step */
    /* 0x78 */ s32 target; /* the player it goes for */
    /* 0x7C */ u8 pad7C[8];
    /* 0x84 */ s32 hits;
    /* 0x88 */ u8 pad88[4];
    /* 0x8C */ s32 flash;
    /* 0x90 */ f32 bounceVel;
    /* 0x94 */ u8 pad94[4];
    /* 0x98 */ u8 alpha; /* its alpha */
    /* 0x99 */ u8 glowAlpha; /* the glow's */
    /* 0x9A */ s8 dir;
    /* 0x9B */ u8 armored;
    /* 0x9C */ s8 cool;
} OniWork;

/* The door's work. */
typedef struct DoorWork {
    /* 0x00 */ u8 state; /* 1 opening, 2 shutting */
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
    /* 0x34 */ AnimeFrame *anim;
    /* 0x38 */ PolFrame *panels[4];
    /* 0x48 */ Obj *fx[2]; /* kind 0x100, made under it */
} DoorWork;

/* A pillar's work. */
typedef struct PillarWork {
    /* 0x00 */ u8 state; /* 0 standing, 1 shaking */
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
    /* 0x34 */ PolFrame *model;
    /* 0x38 */ s32 shake;
    /* 0x3C */ f32 x0; /* where it stands */
    /* 0x40 */ f32 z0;
    /* 0x44 */ f32 sx; /* its scale */
    /* 0x48 */ f32 sy;
    /* 0x4C */ s32 sndAlt; /* which of the fragments' two landing sounds is next */
    /* 0x50 */ s32 silent; /* 0: the fragments sound when they land */
    /* 0x54 */ s32 timer;
} PillarWork;

/* A fragment's work ("hahen", kind 0x26): a piece of a pillar, thrown up and bouncing. */
typedef struct HahenWork {
    /* 0x00 */ u8 state; /* 0 waiting, 1 flying */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09;
    /* 0x0A */ u8 hit;
    /* 0x0B */ u8 pad0B[5];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *model;
    /* 0x38 */ ACollFrame *atk;
    /* 0x3C */ ACollFrame *dmg;
    /* 0x40 */ MPolFrame *shadow;
    /* 0x44 */ s32 count; /* the wait, then the bounces */
    /* 0x48 */ f32 spin[2];
} HahenWork;

/* The second fragments' work ("hahen2"): the pieces' lives. */
typedef struct Hahen2Work {
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
    /* 0x34 */ PolFrame *pieces[4];
    /* 0x44 */ s16 life[4];
} Hahen2Work;

#define MAIN(obj) ((MainWork *) FX(obj))
#define SPIRIT(obj) ((SpiritWork *) FX(obj))
#define ONI(obj) ((OniWork *) FX(obj))
#define DOOR(obj) ((DoorWork *) FX(obj))
#define PILLAR(obj) ((PillarWork *) FX(obj))
#define HAHEN(obj) ((HahenWork *) FX(obj))
#define HAHEN2(obj) ((Hahen2Work *) FX(obj))

/* fadeSegToRam as this file calls it: with the address as a pointer. Passed as a (u32), IDO
 * keeps a symbol's address in a temp across calls; as a pointer it builds it again at each use,
 * as the game does (the door's four calls). */
typedef u32 (*SegToRamFunc)(void *addr, u32 file);

/* acollAtkAddYZ and acollDmgAddYZ return their box (main declares them void). */
typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                    s32 a);

extern Frame *gFrmLayer3;
extern Gfx oniFightHahenDl[];
extern Gfx oniFightPillarDl[];
extern u8 oniFightDoorAnim[];
extern Gfx oniFightDoorPanelDl0[];
extern Gfx oniFightDoorPanelDl1[];
extern Gfx oniFightDoorPanelDl2[];
extern Gfx oniFightDoorPanelDl3[];
Gfx oniFightDoorDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetEnvColor(0x00, 0x00, 0x00, 0xFF),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, 0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT, 0, 0, 0, SHADE),
    gsSPEndDisplayList(),
};
extern u8 oniFightSpiritAnim[];
extern Gfx oniFightSpiritGlowDl[];
extern u8 oniFightOniBodyAnim[];
extern Gfx oniFightOniBodyDl[];
extern u8 oniFightOniRide[];
extern u8 oniFightOniAtkChain[];
extern u8 oniFightOniState6Anim[];
extern u8 oniFightOniState10Anim[];
extern u8 oniFightOniState17Anim[];
extern u8 oniFightOniState21Anim[];
extern u8 oniFightOniState31Anim[];
extern u8 oniFightOniState27Anim[];
extern u8 oniFightOniState40Anim[];

void oniFightPillarScatter(Obj *o);
void oniFightHahenLandSound(s32 *alt, f32 x, f32 y, f32 z);

void oniFightShowGauge(void);
void oniFightSpiritRise1(Obj *o);
void oniFightSpiritRise3(Obj *o);
void oniFightSpiritRise5(Obj *o);
s32 oniFightSpiritIsDown(Obj *o);
void oniFightSpiritSettle(Obj *o);
void oniFightSpiritScale(Obj *o, f32 s);
void oniFightOniWake(Obj *o);
void oniFightOniStart(Obj *o);
void oniFightDoorOpen(Obj *o);
void oniFightDoorShut(Obj *o);
void oniFightDoorMakeBarrier(Obj *o);
void oniFightDoorKillBarrier(Obj *o);

/* "main": makes the others, then runs the scene: the players walk in, the camera shows the
 * spirit and the oni, the fight; message 10 with MAIN_ID finds it. */
s32 oniFightMainProc(Obj *o, s16 msg, void *arg) {
    s32 pad;
    Player *pl;
    s32 i;
    s32 m;
    s32 n;
    s32 c;
    MainWork *w = MAIN(o);
    Camera *cam = camAiGetCam();

    switch (msg) {
        case 2: {
            o->name = "main";
            eneSetup(o, arg);
            w->spirit = eneMakeUnder(o, ENE_ONI_FIGHT_SPIRIT, 1, 300.0f, 100.0f, 0.0f);
            w->oni = eneMakeUnder(o, ENE_ONI_FIGHT_ONI, 1, 474.0f, 7.0f, 0.0f);
            m = 0;
            w->door = eneMakeEx(o, ENE_ONI_FIGHT_DOOR, 0, 0.0f, 0.0f, 0.0f, 2, (u32 (*)(void *, void *, void *)) &m);
            w->pillars[0] = eneMakeEx(o, ENE_ONI_FIGHT_PILLAR, 0, -150.0f, -3.0f, -52.5f, 2, (u32 (*)(void *, void *, void *)) &m);
            w->pillars[1] = eneMakeEx(o, ENE_ONI_FIGHT_PILLAR, 0, 0.0f, -3.0f, -52.5f, 2, (u32 (*)(void *, void *, void *)) &m);
            w->pillars[2] = eneMakeEx(o, ENE_ONI_FIGHT_PILLAR, 0, 150.0f, -3.0f, -52.5f, 2, (u32 (*)(void *, void *, void *)) &m);
            w->gauge = tskCreate(o, pipGauge_proc, 0, NULL);
            eneCounterReward(2, 1, 590.0f, 8.0f, 0.0f, 1, 560.0f, 8.0f, 0.0f);
            gGameWork.scenarioRunning = 2;
            if (eneCounterGet(2) != 0) {
                w->state = 7;
            } else {
                w->state = 0;
            }
            eneCounterIncrement(2);
            PRINTF("@@@@@@@@@@@@@@@ COUNTER = %x\n @@@@@@@@@@@", eneCounterGet(2));
            w->count = 50;
            break;
        }
        case 10:
            if ((s32) arg == MAIN_ID) {
                return (s32) o;
            }
            break;
        case 0:
            if (w->state < 7 && fadeStartPressed() != 0) {
                sndPlay(7);
                w->state = 7;
            }
            switch (w->state) {
                case 0:
                    n = 0;
                    if (fadeMoveLetterbox(45, 185, 1) != 0) {
                        w->state = 1;
                        w->arrived = 0;
                        w->timer = 70;
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                Vec3f p;
                                s32 zone;

                                plSetAction(gGameWork.players[i], 0xDB);
                                plAskPos(gGameWork.players[i], &p.x, &p.y, &p.z, &zone);
                                p.x = 230.0f;
                                plDemoWalkTo(gGameWork.players[i], p, 2.0f);
                                n++;
                            }
                        }
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                Vec3f p;
                                s32 zone;

                                plAskPos(gGameWork.players[i], &p.x, &p.y, &p.z, &zone);
                                switch (n) {
                                    case 1:
                                        p.z = 0.0f;
                                        break;
                                    case 2:
                                        p.z += 5 - i * 10;
                                        break;
                                    case 3:
                                        p.z += 5 - i * 5;
                                        break;
                                    case 4:
                                        p.z += 5.0 - i * 3.3;
                                        break;
                                }
                                plSetPos(gGameWork.players[i], zone, &p);
                            }
                        }
                    }
                    break;
                case 1:
                    for (i = 0, c = 0; i < 4; i++) {
                        if (plIsInPlay(i) && plIsDemoWalking(gGameWork.players[i])) {
                            c++;
                        }
                    }
                    if (w->arrived == 0 && c == 0) {
                        w->arrived = 1;
                        sndPlay(0xB3);
                    }
                    if (w->timer-- <= 0) {
                        camSetSpeed(cam, 100.0f);
                        camMoveAt(cam, -130.0f, 15.0f, 51.0f);
                        camMoveEye(cam, -60.0f, 30.0f, 100.0f);
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (!camGetMoving(cam)) {
                        camSetSpeed(cam, 300.0f);
                        camMoveAt(cam, 100.0f, 67.0f, 51.0f);
                        camMoveEye(cam, 30.0f, 65.0f, 100.0f);
                        w->state = 3;
                    }
                    break;
                case 3: {
                    f32 x;
                    f32 y;
                    f32 z;

                    camGetAt(cam, &x, &y, &z);
                    if (x >= -70.0f) {
                        oniFightSpiritRise1(w->spirit);
                    }
                    if (x >= -10.0f) {
                        oniFightSpiritRise3(w->spirit);
                    }
                    if (!camGetMoving(cam)) {
                        oniFightSpiritRise5(w->spirit);
                        w->state = 4;
                    }
                    break;
                }
                case 4:
                    if (oniFightSpiritIsDown(w->spirit)) {
                        camSetSpeed(cam, 40.0f);
                        camMoveAt(cam, 100.0f, 55.0f, 51.0f);
                        camMoveEye(cam, 27.0f, 40.0f, 90.0f);
                        oniFightOniWake(w->oni);
                        w->state = 5;
                    }
                    break;
                case 5:
                    if (!camGetMoving(cam)) {
                        scRunStart2(9, 0, 0);
                        camSetSpeed(cam, 120.0f);
                        camMoveAt(cam, 100.0f, 50.0f, 51.0f);
                        camMoveEye(cam, 25.0f, 33.0f, 70.0f);
                        w->state = 6;
                    }
                    break;
                case 6:
                    if (!camGetMoving(cam) && fadeMoveLetterbox(0, 240, 2)) {
                        w->state = 7;
                    }
                    break;
                case 7: {
                    s32 k;

                    k = 0;
                    fadeSetLetterbox(320, 0, 0, 240);
                    scRunEnd();
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            k++;
                        }
                    }
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            Vec3f p;
                            s32 zone;

                            plSetAction(gGameWork.players[i], 0);
                            plAskPos(gGameWork.players[i], &p.x, &p.y, &p.z, &zone);
                            p.x = 230.0f;
                            switch (k) {
                                case 1:
                                    p.z = 0.0f;
                                    break;
                                case 2:
                                    p.z = 5 - i * 10;
                                    break;
                                case 3:
                                    p.z = 5 - i * 5;
                                    break;
                                case 4:
                                    p.z = 5.0 - i * 3.3;
                                    break;
                            }
                            plSetPos(gGameWork.players[i], zone, &p);
                            pl = PL(gGameWork.players[i]);
                            pl->tilt = 0.0f;
                        }
                    }
                    oniFightOniStart(w->oni);
                    oniFightSpiritSettle(w->spirit);
                    camAiSetHold(0);
                    sndPlay(0x98);
                    oniFightShowGauge();
                    oniFightDoorMakeBarrier(w->door);
                    w->state = 8;
                    gGameWork.scenarioRunning = 1;
                    eneRumbleStopAll();
                    break;
                }
                case 9:
                    gGameWork.scenarioRunning = 2;
                    fadeCreate(gEffectTask);
                    fadeSetLetterbox(320, 45, 0, 185);
                    camAiSetHold(1);
                    cam = camAiGetCam();
                    camSetAt(cam, -37.0f, 2.0f, -27.0f);
                    camSetEye(cam, 13.0f, 50.0f, 122.0f);
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            Vec3f p;
                            s32 zone;

                            plSetAction(gGameWork.players[i], 0xD7);
                            plAskPos(gGameWork.players[i], &p.x, &p.y, &p.z, &zone);
                            p.x = 240.0f;
                            plSetPos(gGameWork.players[i], zone, &p);
                            pl = PL(gGameWork.players[i]);
                            pl->tilt = 0.0f;
                        }
                    }
                    w->timer = 300;
                    eneRumbleAll(0x88888888, 300);
                    w->state = 10;
                    /* fallthrough */
                case 10:
                    if (w->timer == 240) {
                        sndPlay(9);
                    }
                    if (w->timer-- <= 0) {
                        w->timer = 70;
                        eneRumbleAll(-1, 70);
                        sndPlay(0x11D);
                        eneMakeEx(o, ENE_BIG_BOMB, 0, -10.0f, 30.0f, 51.0f, 2, (u32 (*)(void *, void *, void *)) 3);
                        fadeSetAll(w->timer, 0.0f, 255.0f, 255, 255, 255);
                        w->state = 11;
                    }
                    break;
                case 11:
                    oniFightSpiritScale(w->spirit, 0.01f);
                    if (w->timer-- < -39) {
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                Vec3f p;
                                s32 zone;

                                plAskPos(gGameWork.players[i], &p.x, &p.y, &p.z, &zone);
                                p.x = 470.0f;
                                p.y = 7.0f;
                                plSetPos(gGameWork.players[i], zone, &p);
                            }
                        }
                        cam = camAiGetCam();
                        camSetAt(cam, 209.0f, 40.0f, 65.0f);
                        camSetEye(cam, 51.0f, 12.0f, 100.0f);
                        w->timer = 20;
                        fadeSetAll(20.0f, 255.0f, 0.0f, 255, 255, 255);
                        w->state = 12;
                    }
                    break;
                case 12:
                    w->timer--;
                    if (w->timer == -40) {
                        oniFightDoorKillBarrier(w->door);
                        oniFightDoorOpen(w->door);
                    }
                    if (w->timer == -100) {
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                Vec3f p;
                                s32 zone;

                                plSetAction(gGameWork.players[i], 0xDB);
                                plAskPos(gGameWork.players[i], &p.x, &p.y, &p.z, &zone);
                                p.x = 770.0f;
                                plDemoWalkTo(gGameWork.players[i], p, 2.0f);
                            }
                        }
                        w->state = 13;
                    }
                    break;
                case 13:
                    for (i = 0, c = 0; i < 4; i++) {
                        if (plIsInPlay(i) && plIsDemoWalking(gGameWork.players[i])) {
                            c++;
                        }
                    }
                    if (c == 0) {
                        oniFightDoorShut(w->door);
                        w->timer = 50;
                        w->state = 14;
                    }
                    break;
                case 14:
                    if (w->timer-- <= 0 && fadeMoveLetterbox(120, 120, 1)) {
                        eneRumbleStopAll();
                        eneCounterSetFlag(2);
                        dmenuQueueDemo(9);
                    }
                    break;
            }
            break;
        case 1:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

s32 oniFightGetCount(void) {
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) MAIN_ID);

    if (o != NULL) {
        return MAIN(o)->count;
    }
    return -1;
}

void oniFightShowGauge(void) {
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) MAIN_ID);

    MainWork *w;

    if (o != NULL) {
        w = MAIN(o);
        pipGaugeOn(w->gauge);
        pipGaugeSetTarget(w->gauge, w->count);
    }
}

s32 oniFightTakeCount(s32 n) {
    s32 pad;
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) MAIN_ID);
    MainWork *w;

    if (o != NULL) {
        w = MAIN(o);
        w->count -= n;
        if (w->count <= 0) {
            w->count = 0;
        }
        pipGaugeSetTarget(w->gauge, w->count);
        return w->count;
    }
    return -1;
}

Obj *oniFightGetSpirit(Obj *o) {
    return MAIN(o)->spirit;
}

Obj *oniFightGetOni(Obj *o) {
    return MAIN(o)->oni;
}

void oniFightCamDir(f32 scale, f32 *x, f32 *y, f32 *z) {
    f32 ex;
    f32 ey;
    f32 ez;
    f32 ax;
    f32 ay;
    f32 az;
    Vec3f d;
    Camera *cam = camAiGetCam();

    camGetEye(cam, &ex, &ey, &ez);
    camGetAt(cam, &ax, &ay, &az);
    d.x = ax - ex;
    d.y = ay - ey;
    d.z = az - ez;
    mathVecNormalize(&d);
    *x = d.x * scale;
    *y = d.y * scale;
    *z = d.z * scale;
}

void oniFightStartEnding(Obj *o) {
    MAIN(o)->state = 9;
}

s32 oniFightEndingFlashed(Obj *o) {
    if (MAIN(o)->state == 11) {
        return TRUE;
    }
    return FALSE;
}

void oniFightOniPark(Obj *o);
void oniFightSparkle(f32 x, f32 y, f32 z, f32 size);

/* "spirit"
 * Matched 2026-10-15 (the last entry below). History, when it was NON_MATCHING:
 * real 236 (last entry), was 238, 496 differing lines, length 0, mix 0 (2026-10-12; the entries below
 * are older states). Was 1017 differing lines (1077), mix distance 1: the frame (0xA0), the homes (glow
 * 0x94, the three course positions 0x64/0x58/0x4C) and the code are the game's, but from the
 * acollDmgAddYZ call on every t-register is one behind (the game spends one more temp before
 * its -30; a reduced copy of case 2 alone gives t8 in every form tried: assign-in-test, !x,
 * &segs[0], casts, the box through a local). States 1 and 5 keep a second 0.0f apart from the
 * shared one, so y and z there are int zeros (1061 to 1017, mix 3 to 1). Sweep 2026-10-11: the
 * game's table has 22 entries (21 is the default): "case 21: break;" (1017, table right); state
 * 7's z as an int zero too (its own mtc1, as the game): 898, mix 0, the game's length.
 * 2026-10-12 (from real 501, 894 lines), real after each:
 * - state 3's y spelled 10.00f (IDO_NOTES 365; the game's 10.0 a queue temp, here it was f0) 274.
 *   2026-10-16: written as the int 10 instead, object byte-identical (measured).
 * - state 5: y before vel[0] 274 (no change); vel[2] and vel[1] joined 274; timer, state,
 *   vel[2], vel[1] joined 274 (all taken back).
 * - state 10: z = p.z right after y (the game loads p.z before the constants) 261.
 * - states 13 and 18: z = p.z before sounded/state 255, 246.
 * - state 5: vel[1] = -0.3f before timer/state/vel[2] 238. Every float now the game's.
 * - case 2's model as "if ((w->model = eneNewAnime(..)) == NULL)" in the full function: 238
 *   (no change; taken back).
 * Left (238): from the acollDmgAddYZ call on every t-register one behind (game t3, here t2):
 * between o->name's t7/t8 and the -30 the game's ugen takes one more temporary than this C's
 * (t9, t0, t1, t2 there, t9, t0, t1 here), none of them visible in the code.
 * 2026-10-12 (second try): the model's test as "*(s32 *) &w->model == 0" (IDO_NOTES 274's spent
 * reload): the -30 takes the game's t3, 236 (kept, FAKE). Next: 0x60 is t9 there, t2 here
 * (andi t4 there, t9 here). Misses: safe before alpha 237; glowAlpha before safe 237; safe and
 * glowAlpha on one line 236 (no change).
 * 2026-10-12 (third try): the scales as a chain "[2] = [1] = [0] = 0.2f": frame 0xA8 (the chain's
 * word), 0x60 still t2, 305; alpha = 0x60 before the three scales: 0x60 still t2, 240. Both
 * taken back.
 * 2026-10-12 (fourth try): the permuter (base 2735, ~400 iterations):
 * state 11's rot[2] store before the state store, 236 to 232 (kept; a free swap of two stores).
 * Diagnostic: alpha = 0x60 right after the dmg kind: 0x60 takes t5 and the glow's ori t2 (243),
 * so after t8 this C's ugen hands out t2 where the game's hands out t9 wherever that statement
 * sits: the register order comes from earlier history (the FAKE's spent register), not from
 * alpha's statement. The FAKE moved to the glow's test ("*(s32 *) &w->glow == 0", the model's
 * plain): 232, the same. Taken back.
 * 2026-10-12 (fifth try): checked 232. Same kind of difference as iki (tlb_0042's
 * dochukiIkiProc): ugen's temporary queue one behind, no uopt range involved; not worked on.
 * MATCHED 2026-10-15 (trace + a scratch instrumented ugen, as for wagaKasaProc):
 * - ugen's free list is FIFO (f_get_one_free_reg takes the head, f_free_reg appends at the
 *   tail; ugen.c 80781-80962, 81664-81840). Its members are fixed per procedure: init_regs
 *   (t6 t7 t8 t9 t0-t5), less the registers uopt's ranges use (gen_regs/remove_from_free_list).
 *   The game's ring here is t3-t9; its only t2 is case 3's "lw t2, 0x38(s0)" (w->glow), which
 *   is a uopt register, not a ugen temp. So the game's uopt had a range in t2: case 3's glow
 *   through the local, "glow = w->glow; glow->dl = gGfxDlHead;" (glow's range takes r9 = t2,
 *   gen_regs drops t2 from ugen's list). With it the model test's FAKE (note 274's reload,
 *   which only spent t2) goes: 232 -> 35 with the FAKE removed. "glow = w->glow" right before
 *   its use (the game loads it there) 29.
 * - case 2: "w->safe = 1;" before "w->alpha = 0x60;": the list is [t3 t9 ...] there, so safe
 *   takes t3 and 0x60 t9 as in the game, and glowAlpha's alpha is forwarded (andi) 24.
 * - with the registers right, the earlier store-order moves were wrong guesses taken back to
 *   the game's line order: state 3 timer before state 20; state 1 timer/state after the vels
 *   12; state 11 rot[2] before timer/state 8; state 12 vel[0] before state 6.
 * - state 17: the soundIncamCam address CSE (uopt's local colouring) is v0 here, v1 in the game.
 *   Called through an s32-returning type (FAKE, the typedef above), v0 is avoided: MATCH. */
s32 oniFightSpiritProc(Obj *o, s16 msg, void *arg) {
    SpiritWork *w = SPIRIT(o);
    AnimeFrame *a;
    PolFrame *glow;
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "spirit";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) oniFightSpiritAnim);
            if (w->model == NULL) {
                return -1;
            }
            a = w->model;
            frmMove(&a->f, gFrmLayer8);
            w->glow = eneNewPol(o, oniFightSpiritGlowDl);
            if (w->glow == NULL) {
                return -1;
            }
            glow = w->glow;
            floadGetSetRam((void **) glow->segs, 0x120);
            frmMove(&w->glow->f, gFrmLayer8);
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &a->f, 30, 60, -30, 0, 0);
            w->dmg->kind = 0x65;
            a->flags |= 2;
            glow->flags |= 2;
            a->scale[0] = 0.2f;
            a->scale[1] = 0.2f;
            a->scale[2] = 0.2f;
            w->safe = 1;
            w->alpha = 0x60;
            w->glowAlpha = w->alpha / 2;
            w->flash = 0;
            w->shadow = shadowCreate();
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            /* The game's: it tests the glow and destroys the model again. */
            if (w->glow != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            break;
        case 3:
            a = w->model;
            a->dl = gGfxDlHead;
            if (w->flash & 2) {
                a->dl = gGfxDlHead;
                gDPSetEnvColor(gGfxDlHead++, 0xD0, 0xD0, 0xD0, 0xA0);
                gDPSetCombine(gGfxDlHead++, 0xFFFFFF, 0xFFFEFB7D);
            } else {
                gDPSetEnvColor(gGfxDlHead++, 0xB0, 0xB0, 0xFF, w->alpha);
                gDPSetCombine(gGfxDlHead++, 0x12FE25, 0xFFFFFBFD);
            }
            gSPEndDisplayList(gGfxDlHead++);
            glow = w->glow;
            glow->dl = gGfxDlHead;
            gDPSetEnvColor(gGfxDlHead++, 0xC0, 0xC0, 0xFF, w->glowAlpha);
            gDPSetCombine(gGfxDlHead++, 0x129A25, 0xFF37FFFF);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            a = w->model;
            glow = w->glow;
            if (w->flash-- <= 0) {
                w->flash = 0;
            }
            if (acollDmgWasHit(w->dmg) == 1 && w->state < 15 && w->safe == 0) {
                eneHitSoundHeavy(w->dmg);
                w->flash = 35;
                oniFightTakeCount(eneHitStrength(w->dmg) + 3);
                w->state = 15;
            }
            switch (w->state) {
                case 0:
                    eneAnimLoop(o, w->model, 0.5f);
                    break;
                case 1:
                    w->x = 290.0f;
                    w->y = 0;
                    w->z = 15.0f;
                    w->vel[0] = 1.3f;
                    w->vel[2] = 0.0f;
                    w->vel[1] = 1.0f;
                    w->timer = 10;
                    w->state = 2;
                    /* fallthrough */
                case 2:
                    if (w->timer-- == 0) {
                        sndPlay(0x292);
                    }
                    eneAnimLoop(o, w->model, 1.0f);
                    w->rot[0] = w->vel[2] * 0.0174532930038800785 * 20.0;
                    w->rot[2] = -w->vel[0] * 0.0174532930038800785 * 20.0;
                    eneMoveObj(o);
                    break;
                case 3:
                    w->timer = 10;
                    w->state = 4;
                    w->x = 400.0f;
                    w->y = 10;
                    w->z = 15.0f;
                    w->vel[0] = -0.5f;
                    w->vel[2] = 0.0f;
                    w->vel[1] = 1.0f;
                    /* fallthrough */
                case 4:
                    if (w->timer-- == 0) {
                        sndPlay(0x292);
                    }
                    eneAnimLoop(o, w->model, 1.0f);
                    w->rot[0] = w->vel[2] * 0.0174532930038800785 * 20.0;
                    w->rot[2] = -w->vel[0] * 0.0174532930038800785 * 20.0;
                    eneMoveObj(o);
                    break;
                case 5:
                    w->x = 474.0f;
                    w->vel[0] = 0.0f;
                    w->y = 120.0f;
                    w->z = 0;
                    w->vel[1] = -0.3f;
                    w->timer = 50;
                    w->state = 6;
                    w->vel[2] = 0.0f;
                    /* fallthrough */
                case 6:
                    if (w->timer-- == 0) {
                        sndPlay(0x1A1);
                    }
                    eneAnimLoop(o, w->model, 0.6f);
                    w->rot[0] = w->vel[2] * 0.0174532930038800785 * 20.0;
                    w->rot[2] = -w->vel[0] * 0.0174532930038800785 * 20.0;
                    eneMoveObj(o);
                    if (w->y < 40.0f) {
                        w->state = 7;
                    }
                    break;
                case 7:
                    w->safe = 1;
                    w->vel[0] = 0.0f;
                    w->vel[1] = 0.0f;
                    w->vel[2] = 0.0f;
                    w->x = 474.0f;
                    w->y = 200.0f;
                    w->z = 0;
                    a->anim = 0;
                    a->frame = 0.0f;
                    glow->dl2 = NULL;
                    break;
                case 8:
                    a->anim = 0;
                    a->frame = 0.0f;
                    glow->dl2 = NULL;
                    break;
                case 10: {
                    Vec3f p;

                    eneGetCoursePos(oniFightGetOni(o->parent), &p);
                    w->x = p.x;
                    w->y = p.y + 40.0f;
                    w->z = p.z;
                    w->vel[1] = 6.0f;
                    w->vel[0] = 0.0f;
                    w->vel[2] = 1.0f;
                    w->rot[2] = -0.7853981853f;
                    a->anim = oniFightSpiritAnim;
                    a->frame = 0.0f;
                    glow->dl2 = oniFightSpiritGlowDl;
                    w->bounces = 0;
                    w->state = 11;
                }
                    /* fallthrough */
                case 11:
                    eneAnimLoop(o, w->model, 1.0f);
                    eneFall(o, 0.3f);
                    eneMoveObj(o);
                    if (w->z >= 0.0f) {
                        w->safe = 0;
                        w->z = 0.0f;
                    }
                    if (eneLandOff((EneObj *) w, 30.0f, -5.0f) != 0) {
                        eneSetModelOnCourse(o, &a->f);
                        oniFightSparkle(a->pos[0], a->pos[1] - 5.0f, a->pos[2], 0.1f);
                        w->vel[1] = 0.0f;
                        w->vel[2] = 0.0f;
                        w->rot[2] = -1.570796371f;
                        w->timer = 30;
                        w->state = 12;
                    }
                    break;
                case 12:
                    eneAnimLoop(o, w->model, 2.0f);
                    if (w->timer-- <= 0) {
                        w->rot[2] += 0.03926990926f;
                        w->y += 0.2f;
                        if (w->rot[2] >= 0.0f) {
                            w->rot[2] = 0.0f;
                            w->vel[1] = 0.2f;
                            w->vel[0] = 1.0f;
                            w->state = 13;
                        }
                    }
                    break;
                case 13: {
                    Vec3f p;

                    eneAnimLoop(o, w->model, 1.0f);
                    eneGetCoursePos(oniFightGetOni(o->parent), &p);
                    eneAccelAxis(p.x, 1.2f, 0.05f, &w->x, &w->vel[0]);
                    eneMoveObj(o);
                    w->rot[2] = -w->vel[0] * 0.0174532930038800785 * 20.0;
                    if (w->y >= 100.0f) {
                        w->x = p.x;
                        w->y = 100.0f;
                        w->z = p.z;
                        w->sounded = 0;
                        w->state = 14;
                        w->vel[0] = 0.0f;
                        w->vel[1] = -1.0f;
                    }
                    break;
                }
                case 14:
                    if (w->y <= 90.0f && w->sounded == 0) {
                        w->sounded = 1;
                        SoundLocation2(0x1A2, gGameWork.view, a->pos[0], a->pos[1], a->pos[2], 0.0f, 0.0f,
                                       gGameWork.view->fovy);
                    }
                    eneAnimLoop(o, w->model, 1.0f);
                    eneMoveObj(o);
                    if (w->y <= 40.0f) {
                        w->state = 7;
                    }
                    break;
                case 15:
                    w->vel[0] = 1.0f;
                    if (a->pos[0] < w->dmg->hitPos[0]) {
                        w->vel[0] = -1.0f;
                    }
                    w->vel[2] = 0.0f;
                    w->vel[1] = 4.0f;
                    a->anim = oniFightSpiritAnim;
                    a->frame = 0.0f;
                    glow->dl2 = oniFightSpiritGlowDl;
                    w->state = 16;
                    /* fallthrough */
                case 16:
                    eneAnimLoop(o, w->model, 1.0f);
                    eneFall(o, 0.4f);
                    if (w->x < 125.0f) {
                        w->vel[0] = fabsf(w->vel[0]);
                    }
                    if (w->x > 610.0f) {
                        w->vel[0] = -fabsf(w->vel[0]);
                    }
                    eneMoveObj(o);
                    if (eneLandOff((EneObj *) w, 30.0f, -5.0f) != 0) {
                        eneSetModelOnCourse(o, &a->f);
                        oniFightSparkle(a->pos[0], a->pos[1], a->pos[2] - 5.0f, 0.1f);
                        w->vel[1] *= -0.5;
                        w->vel[0] *= 0.5;
                        w->rot[2] = -1.570796371f;
                        if (w->vel[1] < 0.5) {
                            w->timer = 20;
                            w->vel[1] = 0.0f;
                            w->vel[0] = 0.0f;
                            if (oniFightGetCount() == 0) {
                                w->state = 19;
                            } else if (w->bounces >= 2) {
                                w->timer = 0;
                                w->safe = 1;
                                w->state = 17;
                            } else {
                                w->bounces++;
                                w->timer = 20;
                                w->state = 12;
                            }
                        }
                    }
                    break;
                case 17:
                    eneAnimLoop(o, w->model, 1.0f);
                    if (w->timer-- <= 0) {
                        w->rot[2] += 0.104719758f;
                        w->y += 0.4f;
                        if (w->rot[2] >= 0.0f) {
                            w->rot[2] = 0.0f;
                            w->vel[1] = 2.0f;
                            if (oniFightGetCount() == 0) {
                                w->state = 19;
                            } else {
                                /* soundIncamCam returns a value (sndloc.c), so the address
                                 * CSE avoids v0 and takes the game's v1. */
                                soundIncamCam(0x292, a->pos[0], a->pos[1], a->pos[2]);
                                soundIncamCam(0x305, a->pos[0], a->pos[1], a->pos[2]);
                                w->state = 18;
                            }
                        }
                    }
                    break;
                case 18:
                    eneAnimLoop(o, w->model, 1.0f);
                    eneMoveObj(o);
                    if (w->y >= 100.0f) {
                        Vec3f p;

                        eneGetCoursePos(oniFightGetOni(o->parent), &p);
                        w->x = p.x;
                        w->y = 100.0f;
                        w->z = p.z;
                        w->sounded = 0;
                        w->state = 14;
                        w->vel[0] = 0.0f;
                        w->vel[1] = -1.0f;
                    }
                    break;
                case 19:
                    w->z = 0.0f;
                    w->rot[0] = 0.0f;
                    w->rot[1] = 0.0f;
                    w->rot[2] = 0.0f;
                    w->x = 360.0f;
                    w->y = 30.0f;
                    eneSetModelOnCourse(o, &a->f);
                    effectPompomCreate(a->pos[0], a->pos[1], a->pos[2], 10.0f, 10.0f, 10.0f, 9, 400, 3, 0.05f, 0.02f);
                    sndPlay(0x10E);
                    w->sndTimer = 0;
                    eneEndPlayersPowerDown();
                    oniFightStartEnding(o->parent);
                    oniFightOniPark(oniFightGetOni(o->parent));
                    w->state = 20;
                    /* fallthrough */
                case 20:
                    if (w->sndTimer-- <= 0) {
                        w->sndTimer = 6;
                        sndPlay(0x513);
                    }
                    if (oniFightEndingFlashed(o->parent)) {
                        w->state = 21;
                    }
                    eneAnimLoop(o, w->model, 1.0f);
                    break;
                case 21:
                    break;
            }
            eneSetModelOnCourse(o, &a->f);
            glow->pos[0] = a->pos[0];
            glow->pos[1] = a->pos[1];
            glow->pos[2] = a->pos[2];
            glow->scale[0] = a->scale[0] * 6.0f;
            glow->scale[1] = a->scale[1] * 6.0f;
            if (a->anim == 0) {
                w->glowAlpha = 0;
            } else {
                w->glowAlpha = 0x40;
            }
            shadowPut(w->shadow, a->pos[0], a->pos[1], a->pos[2], 1.3f, NULL);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void oniFightSpiritRise1(Obj *o) {
    if (SPIRIT(o)->state < 1) {
        SPIRIT(o)->state = 1;
    }
}

void oniFightSpiritRise3(Obj *o) {
    if (SPIRIT(o)->state < 3) {
        SPIRIT(o)->state = 3;
    }
}

void oniFightSpiritRise5(Obj *o) {
    if (SPIRIT(o)->state < 5) {
        SPIRIT(o)->state = 5;
    }
}

s32 oniFightSpiritIsDown(Obj *o) {
    if (SPIRIT(o)->state == 7) {
        return TRUE;
    }
    return FALSE;
}

void oniFightSpiritSettle(Obj *o) {
    SPIRIT(o)->state = 7;
}

void oniFightSpiritLeap(Obj *o) {
    SPIRIT(o)->state = 10;
}

void oniFightNop(void) {
}

void oniFightSpiritScale(Obj *o, f32 s) {
    SPIRIT(o)->model->scale[0] *= s;
    SPIRIT(o)->model->scale[1] *= s;
    SPIRIT(o)->model->scale[2] *= s;
}

void oniFightOniSetState(Obj *o, s32 state, s32 wait);
void oniFightOniStop(Obj *o);
void oniFightOniShowBody(Obj *o);
void oniFightOniShowBall(Obj *o);
void oniFightOniSyncBall(Obj *o);
void oniFightOniDust(Obj *o, f32 x, f32 y, f32 z, f32 size);
s32 oniFightOniSpin(Obj *o, f32 step);
void oniFightOniRollSteer(Obj *o);
s32 oniFightOniRollHit(Obj *o);
void oniFightOniJumpToPillar(Obj *o);
Obj *oniFightNearestPillar(Obj *o);
s32 oniFightOniFacePlayer(Obj *o, s32 pl, f32 step);
void oniFightPillarBreak(Obj *o);

/* A push (message 6's argument): plmove.c's PlPlace (zone, place, side, height). */
typedef struct OniPush {
    /* 0x00 */ s16 zone;
    /* 0x02 */ u8 pad02[2];
    /* 0x04 */ f32 x;
    /* 0x08 */ f32 side;
    /* 0x0C */ f32 y;
} OniPush;

/* "oni"
 * Matched 2026-10-15 (the last entry below). History, when it was NON_MATCHING:
 * real 7 since an earlier try (2026-10-12; only case 44's two constants, see the last
 * entries). Was: real 38, length 0, mix 0 (298 differing lines). The frame (0x108) and every
 * local's home are the game's, but the game's compiler temps sit one word higher (0x3C/0x40,
 * with 0x38 unused; here 0x38/0x3C), so the CSE'd function addresses go to other words (about
 * 20 of the 38). History (the entries below the first are older states of this C):
 * 2026-10-11 state (1665 lines, the two unused words of states 33 and 37 then FAKE pads, from
 * case 3 on registers differing). Measured: the anim address as (u32) twice spills it to 0x44 (3215 lines), as pointers or with
 * the store's (s32) it is built again at each use as in the game; case 3 through the local a
 * gave s2 for the model (3159 to 1665). Sweep 2026-10-11 (IDO_NOTES 346's pad): a pad below
 * the last local (after dz) puts the temps at 0x3C/0x40 and every local right from the top, but
 * the frame is 0x110 (2729); pad[3] + a scalar pad, temps right, locals 4 high (2693); pad[4] +
 * a scalar pad 1665 (no change). The game's state table has 48 entries (sltiu 0x30, no addiu -1;
 * entry 0 is the default's eneFall): "case 0: break;" gives its size and mix 4, but 3178 lines
 * (2952 instructions, 4 short; cases 3, 18, 20 and 27 one short each). Not kept (more lines),
 * but the game's table needs it: put it back once those four cases are found.
 * 2026-10-12, the four found (mnemonic counts against the game: 3 mtc1 and 1 sw more there):
 * - three sounds pass their zeros as 0.0f, 0.0f (two mtc1 zero each in the game; this C's
 *   0, 0 share one): 0x307 (game instruction 961), the 0x108 before state 14 (1106)
 *   and the 0x108 where count is 0 (1512). With them: length 0, mix 2, real 2155;
 * - with "case 0: break;" as well: mix 1, length -1 (the fourth), real 2384;
 * - the fourth is case 3's store of glow to its home (0xF8): the game keeps the
 *   "glow = w->glow" store and uses the loaded value for glow->dl; here uopt gives glow s2
 *   there, so the game's 0xDE000000 (s2), 0xFB000000 (ra) and 0xDF000000 (t5) move to ra, t5,
 *   t4. "glow = w->glow; w->glow->dl = ..." drops the store as dead (2082); "(void) &glow"
 *   changes nothing.
 * All taken back to 1277 (the rule: no file left above its starting real); they go in
 * together once case 3's glow keeps its store. Literals compared one by one in load order:
 * all the game's.
 * 2026-10-12 (from 1277), the four above applied together, then (real after each):
 * - case 3: "if (glow) {}" after glow->dl 2431 (length -2); "(glow = w->glow)->dl = .." 2373;
 *   "do {} while (0);" between def and use 2431; "if (glow) {}" at the end of case 3 2386.
 * - case 3 in a block with "PolFrame *b2 = w->body2" (b2->dl = ..): the glow store comes
 *   back (length 0, mix 0), frame 0x110 (2076); + case 33's pad dropped 2054 (locals off);
 *   pad[5] -> pad[4] instead: every local right, temps still 0x38/0x3C (2046); + case 37's
 *   pad dropped: temps 0x3C/0x40 as the game, the words below 0x78 four high (2042).
 * - case 8's "5.0f; else -5.0f" as a ternary: no change (2042).
 * - IDO_NOTES 365, the float queue one register off from here: case 8's then 5.0f spelled
 *   5.00f 520; state 12's vel[1] 7.00f 309; state 25's 8.00f 294; state 30's 9.00f 639 (kept:
 *   the next one put it right); state 31's 5.00f 79.
 * - case 35's "vel[2] = -5.0f; vel[0] = .." joined on one line 79 (no change); the two swapped
 *   65; case 37's swapped too 62 (case 37's pad dropped: 65 → put back, 70 → 62).
 * - case 37's "state = 34; count--;" joined 62 (no change); swapped 57.
 * - case 0's "flash = 30; if (++hits >= 3)" as "hits++; flash = 30; if (hits >= 3)" 51.
 * - case 44: vel[1] 5.00f 85 (15.0f still a queue temp; the game has 15.0 in f0, 5.0 in f8);
 *   frame = 15.0f moved after vel[1] 51 (314 lines); 15.00f 51 (no change). Open.
 * - 2026-10-12, read with match_function.py trace and proven with ugen-edit (no guessing):
 *   - PROVEN: in ugen's listing, dropping uopt's hoisted "li.s $f0, 5.0" and loading 15.0 into f0
 *     and 5.0 into f8 at their stores gives real 0 (trace/nopromote.blk). That one register
 *     decision is the whole difference.
 *   - The cause, from uopt's trace: the 5.0 constant is one live range (1110) over eleven blocks,
 *     case 44's (bb 564) among them, adjsave 0.833, coloured f0 whole. Case 44's 15.0 is a
 *     single-block range (1108, bb 564) with adjsave 0.0 = (1 use - 1 load) / 1 block, so it is
 *     never coloured. The other 15.0 range (1334, the a->frame clamp: 2 uses over 3 blocks)
 *     has 0.333 and gets f0.
 *   - So in the game, case 44's 15.0 range had a positive priority. globalcolor colours
 *     single-block ranges first (IDO_NOTES 283), so it took f0 in bb 564 first, 1110 was split
 *     there, and that piece was refused: 5.0 was loaded locally into f8. What gave it a second
 *     use (or a merge with 1334) is the open question.
 *   - Not it, measured: case 44's 5.0 unshared ("5.000f"): 42 (the 5.0 range loses a use and
 *     other cases recolour); "a->frame = a->frame = 15.0f": 7 (the duplicate store is folded
 *     before uopt sees it).
 * - b2 at function scope in pad1's place, pad[5], no block in case 3: 51 (same; a FAKE pad less).
 * - the FAKE pads of states 33 and 37 are "CamView *view = gGameWork.view;" as in state 35, with
 *   SoundLocation2(0x10B, view, .., view->fovy): 37's 49, 33's 47.
 * - the tail: scale stores joined on one line, or joined with pos[2]: 47 (no change); pos[2]
 *   written before the three scale stores 41.
 * - b2 declared after glow (pad1 back, pad[4]): no change. Case 0's "glow = ..; a = ..;"
 *   joined: no change; "a = w->body;" first 38.
 * - the temps: case 46's done removed (diagnostic) puts them at 0x3C/0x40 with dx/dy/dz four
 *   high, so the extra word lies below dz (0x40..0x48 here, never touched; the game has two
 *   such words, this C three). "cool--" without its value (diagnostic): no change; a u16 msg
 *   parameter with switch (msg): frame 0x110, 79. A probe script: a switch
 *   adds no word below the last block local; the call temp sits right under it.
 * Left (38): the temps; case 3's 0x80107D18 (t3 here, t4 there) against b2; case 2's
 * frmMove(&w->body2->f) loads gFrmLayer2 before w->body2 (the game after); case 44's 15.0f
 * (the game's in f0, this C's a queue temp) and 5.0f (here f0).
 * 2026-10-12 (another try), the temps, diagnostics only (not kept): every tested post-decrement
 * ("x-- <= 0", "x-- == 0") written without its decrement: no change (temps 0x38/0x3C), so the
 * extra word is not the front end's int temp (measured with a probe script: those share one
 * word); case 3 cut: frame 0xE0 (each gfx macro's block _g takes a word); pad[4] and an unused
 * "s32 padB" after dz: block locals 4 high, temps the same (an unused block local takes no
 * word), and the frame's spare word then sits between the temps and the locals, so the
 * game's unused 0x38 below its temps is a temp word allocated before case 2's spills (kind
 * not found: no chains here). Also hoist (all five nested calls into locals): 75. Cut
 * together (diagnostic), the temps stay 0x38/0x3C: states 1-7, 9-22, 27-32, messages 1/4/6,
 * case 0's head (tick to the hit) and the tail (eneFall to the glow's alpha); cutting 39-42
 * changes the saved registers (frame 0x100), so it says nothing. Not kept: case 2's body test
 * as spirit's "*(s32 *) &w->body == 0" 499; "frmMove((Frame *) w->body2, ..)" 38 (same).
 * Case 44: vel[1] spelled as the int 5: 73 (5.0 leaves f0 for the queue, f10, but 15.0 stays
 * a queue temp, f8, and every later float moves one); frame as the int 15: 38 (no change). So
 * the game's 15.0 is a uopt value (f0) and 5.0 the queue's (f8); this C has it the other way.
 * 2026-10-12, THE TEMPS FOUND: a call's result passed as another call's argument
 *   spends a front-end word (probe: any nested call; one word per
 *   type, the int/pointer one shared with every switch and tested post-decrement, so only the
 *   float one shows). States 32/33's eneTurnToward(o, eneCourseAngle(o, 8), ..) was the float
 *   one: read into a function local `ang` (in one pad word's place, pad[4]): the stack is the
 *   game's, real 38 to 19. Misses: eneFileAddr's result into a local (no change: the int word
 *   stays for the switches).
 *   Then case 2's second frmMove through b2 ("b2 = w->body2; frmMove(&b2->f, gFrmLayer2);"):
 *   19 to 7, fixing both case 2's argument order and case 3's t3/t4 (b2 now has two ranges).
 *   Misses: case 44's vel[1] spelled 5.000f (unique) 54; case 3 "if (b2) {}" 443; case 3's
 *   "b2 = w->body2; b2->dl = .." on one line 20.
 *   Left (7): case 44 only, 15.0 in a queue temp (f8) and 5.0 in f0; the game the reverse.
 *   match_function.py results (198 tries, every call both ways, 21 callees as int): nothing
 *   under 7. Case 44's frame = 15.0 (a double literal): 7 (no change); case 44's four stores
 *   on one line 9; state 19's vel[1] spelled 5.00f 678 (5.0f's range runs through the whole
 *   function: respell only with a reason).
 * 2026-10-12 (another try): case 44's vel[1] as the double 5.0: 42; as 5.000f (unique) again:
 *   42, case 44 then loads 15.0 into f8 and 5.0 into f10 (both queue temps, in the game's
 *   order), so even with 5.0f out of the way 15.0's promoted range (case 42's f0) does not
 *   reach case 44 here; every later float one queue register on. Both taken back.
 * 2026-10-12 (another try): the permuter (base 255) found a goto label after case 44's
 *   "a->frame = 15.0f;" (score 60): checked, real 6, the two loads in the game's order (15.0
 *   first) but 15.0 still in f8 and 5.0 in f0; with vel[1] spelled 5.00f as well: 42 (f8/f10,
 *   every later float one on). Both taken back. The game's 15.0 is a colored value there and
 *   5.0 a queue temp; nothing found that colors 15.0 in that block.
 * 2026-10-12 (another try, 7): case 44's 15.0 through the float local ("ang = 15.0f; a->frame =
 *   ang;"): 7, the same code (uopt propagates the constant, so a variable does not color it).
 *   Taken back.
 * 2026-10-12 (another try, trace, 7): uopt makes ONE range per constant per function, its pieces
 *   disconnected: 15.0's range 1108 is case 42's blocks 532-541 (the clamp and the two +15.0
 *   dust arguments) plus case 44's 564, with nothing live between them (564's liveout has
 *   neither 1108 nor 1110). Colouring 1108 fails (f0 taken at 538 by the 0.0 range 1100, adjsave
 *   1.5, and at 564 by 5.0's 1110), so it is split: the kept piece is {564} alone, adjsave
 *   (1 use - 1 load) / 1 = 0, "not colored (-ve save)"; the rest gives 1334 = {532-537} (f0).
 *   Every coloured single-block range in this function has adjsave >= 1. Diagnostic: case 44's
 *   vel[1] as 5.000f (1110 out of 564): the 15.0 piece is still {564} with adjsave 0 and
 *   uncoloured. So taking 5.0 out of f0 is not enough: the game's 564 piece needs a positive
 *   save of its own (a second 15.0 use in that piece), and no such use is in the game's code.
 *   Open; taken back.
 * MATCHED 2026-10-15 (trace + uopt.c): a single-block constant piece can never be
 *   coloured. compute_save (uopt.c 151016-151262) gives each unit (uses + defs) x freq and
 *   takes the load off (load cost x freq) unless canmoverlod (150920) finds a predecessor inside
 *   the range; adjsave is that over f(N), N = units + default blocks; "<= 0" is "-ve save". So
 *   case 44's 15.0 had to stay in ONE range with case 42's clamp. It could not because the two
 *   dust arguments (blocks 540/541) made 15.0 live through 538-539, where 0.0 (1100, f0, adjsave
 *   1.5) already sat: the whole range failed, split joined 532-541 and left {564} alone (col.txt
 *   "1108 split out 1334", "1108 not colored"). Diagnostic, dust args 15.5f: 15.0 is one range
 *   {532, 533, 564}, adjsave 0.5, coloured whole, but f2 (real 51): 5.0's range 1110 is
 *   constrained (isconstrained, uopt.c 150605: interference count >= the class's registers),
 *   so it is coloured first, by priority, and takes f0 at 564; 15.0 is unconstrained and comes
 *   after, by range number. Fix: the dust's 15.0 spelled 15.00f (365's lever: a separate
 *   constant, same bits, "lui a3,0x4170" unchanged) -> 49; plus case 44's vel[1] spelled
 *   5.000f, which takes 564 out of 1110: 15.0 gets f0 over {532, 533, 564} and the lone 5.000
 *   (one use, adjsave 0) is ugen's queue temp f8. MATCH.
 * 2026-10-16: every apart-spelled constant here is an int literal (5, 7, 8, 9, 15), as a
 * programmer writes them: cfe spells a converted int apart from 5.0f, and the object is
 * byte-identical to the 5.00f/5.000f/15.00f... form (measured). */
s32 oniFightOniProc(Obj *o, s16 msg, void *arg) {
    OniWork *w = ONI(o);
    AnimeFrame *a;
    PolFrame *b2;
    PolFrame *glow;
    f32 ang; /* states 32/33's course angle */
    s32 pad[4]; /* FAKE: frame words */

    switch (msg) {
        case 2:
            o->name = "oni";
            eneSetup(o, arg);
            w->body = eneNewAnime(o, (u32) oniFightOniBodyAnim);
            if (w->body == NULL) {
                return -1;
            }
            a = w->body;
            frmMove(&a->f, gFrmLayer2);
            w->body2 = eneNewPol(o, oniFightOniBodyDl);
            if (w->body2 == NULL) {
                return -1;
            }
            b2 = w->body2;
            frmMove(&b2->f, gFrmLayer2);
            oniFightOniShowBody(o);
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &a->f, 140, 120, 0, 0);
            if (w->atk != NULL) {
                w->atk->kind = 0x78;
            }
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &a->f, 130, 600, 0, 0, 0);
            w->dmg->kind = 0x65;
            w->ride = rideCreate(gRideTree1, (s32) o, &w->body->f, (RideData *) eneFileAddr(o, oniFightOniRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            w->glow = eneNewPol(o, oniFightSpiritGlowDl);
            if (w->glow == NULL) {
                return -1;
            }
            glow = w->glow;
            floadGetSetRam((void **) glow->segs, 0x120);
            frmMove(&w->glow->f, gFrmLayer8);
            w->rot[1] = eneCourseAngle(o, 4);
            /* A pointer here (the anim field's type): as the same (u32) as eneNewAnime's, IDO
             * keeps the address in a temp. */
            a->anim = oniFightOniBodyAnim;
            a->frame = polAnimeLastFrame(w->body);
            eneSetModelOnCourse(o, &a->f);
            glow->flags |= 7;
            w->target = eneRandomPlayerIndex();
            w->wait = 40;
            w->steps = 0;
            w->alpha = 0xF0;
            w->glowAlpha = 0;
            w->tick = 0;
            w->armored = 1;
            w->cool = 0;
            w->shadow = shadowCreate();
            break;
        case 1:
            if (w->body != NULL) {
                frmDestroy(&w->body->f);
            }
            if (w->body2 != NULL) {
                frmDestroy(&w->body2->f);
            }
            if (w->glow != NULL) {
                frmDestroy(&w->glow->f);
            }
            if (w->ride != NULL) {
                frmDestroy(&w->ride->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            if (w->aniAtk != NULL) {
                frmDestroy(&w->aniAtk->frame);
            }
            break;
        case 6:
            ((OniPush *) arg)->x += w->vel[0];
            ((OniPush *) arg)->y += w->vel[1];
            break;
        case 4:
            ((Vec3f *) arg)->x = w->body->pos[0];
            ((Vec3f *) arg)->y = w->body->pos[1];
            ((Vec3f *) arg)->z = w->body->pos[2];
            break;
        case 3:
            a = w->body;
            a->dl = gGfxDlHead;
            if (w->flash & 2) {
                gSPDisplayList(gGfxDlHead++, gDrawSetupEnvTintTexDl);
                gDPSetEnvColor(gGfxDlHead++, 0xD0, 0xD0, 0xD0, 0xA0);
            } else {
                gSPDisplayList(gGfxDlHead++, gDrawSetupEnvTintTexShadeDl);
                gDPSetEnvColor(gGfxDlHead++, 0x40, 0x40, 0x40, w->alpha);
            }
            gSPEndDisplayList(gGfxDlHead++);
            b2 = w->body2;
            b2->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, gDrawSetupEnvTintTexShadeDl);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 0);
            gSPEndDisplayList(gGfxDlHead++);
            glow = w->glow;
            glow->dl = gGfxDlHead;
            gDPSetEnvColor(gGfxDlHead++, 0xFF, 0xFF, 0xFF, w->glowAlpha);
            gDPSetCombine(gGfxDlHead++, 0x129A25, 0xFF37FFFF);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            a = w->body;
            glow = w->glow;
            if (++w->tick >= 10) {
                w->tick = 0;
            }
            if (w->flash-- <= 0) {
                w->flash = 0;
            }
            if (w->cool-- <= 0) {
                w->cool = 0;
            }
            if (w->atk != NULL && acollAtkWasHit(w->atk) == 1) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, o);
            }
            if (w->aniAtk != NULL) {
                ((AniAtkFrame *) w->aniAtk->model)->time = a->frame;
                if (acollAtkWasHit(w->aniAtk) == 1) {
                    tskSendMsg((Obj *) w->aniAtk->hitPower, 7, o);
                }
            }
            if (acollDmgWasHit(w->dmg) == 1) {
                if (w->state < 23 && w->armored == 0) {
                    if (w->flash == 0) {
                        eneHitSoundHeavy(w->dmg);
                        w->hits++;
                        w->flash = 30;
                        if (w->hits >= 3) {
                            w->hits = 0;
                            w->state = 40;
                        }
                    }
                } else if (w->cool == 0) {
                    w->cool = 10;
                }
            }
            switch (w->state) {
                case 0:
                    break;
                case 1:
                    w->alpha -= 4;
                    if (w->alpha == 0) {
                        w->timer = 20;
                        w->state = 2;
                    }
                    break;
                case 2:
                    eneAnimLoop(o, w->body, 0.5f);
                    if (w->timer-- == 0) {
                        sndPlay(0x307);
                        w->glowAlpha = 0xFF;
                        w->state = 3;
                    }
                    break;
                case 3:
                    eneAnimLoop(o, w->body, 0.5f);
                    break;
                case 4:
                    if (w->aniAtk != NULL) {
                        w->aniAtk = (ACollFrame *) aniAtkDelete(w->aniAtk);
                    }
                    a->anim = oniFightOniBodyAnim;
                    a->frame = 0.0f;
                    oniFightOniStop(o);
                    w->armored = 0;
                    w->state = 5;
                    /* fallthrough */
                case 5:
                    eneAnimLoop(o, w->body, 0.5f);
                    if (!plIsInPlay(w->target)) {
                        w->target = eneRandomPlayerIndex();
                    }
                    oniFightOniFacePlayer(o, w->target, 0.07853982f);
                    if (w->wait-- <= 0) {
                        switch (w->steps) {
                            case 0:
                                w->target = eneRandomPlayerIndex();
                                oniFightOniSetState(o, 10, 1);
                                break;
                            case 1:
                                oniFightOniSetState(o, 6, 1);
                                w->target = eneRandomPlayerIndex();
                                break;
                            case 2:
                                oniFightOniSetState(o, 6, 10);
                                break;
                            case 3:
                                w->target = eneRandomPlayerIndex();
                                oniFightOniSetState(o, 31, 10);
                                break;
                            case 4:
                                w->target = eneRandomPlayerIndex();
                                oniFightOniSetState(o, 10, 1);
                                break;
                            case 5:
                                oniFightOniSetState(o, 6, 1);
                                w->target = eneRandomPlayerIndex();
                                break;
                            case 6:
                                oniFightOniSetState(o, 6, 10);
                                break;
                            case 7:
                                w->target = eneRandomPlayerIndex();
                                oniFightOniSetState(o, 23, 10);
                                break;
                            case 8:
                                w->target = eneRandomPlayerIndex();
                                oniFightOniSetState(o, 10, 1);
                                break;
                            case 9:
                                oniFightOniSetState(o, 6, 1);
                                w->target = eneRandomPlayerIndex();
                                break;
                            case 10:
                                oniFightOniSetState(o, 6, 10);
                                break;
                            case 11:
                                oniFightOniSetState(o, 15, 10);
                                w->target = eneRandomPlayerIndex();
                                w->steps = 0;
                                break;
                        }
                    }
                    break;
                case 6:
                    a->anim = oniFightOniState6Anim;
                    a->frame = 0.0f;
                    w->state = 7;
                    /* fallthrough */
                case 7:
                    if (!plIsInPlay(w->target)) {
                        w->target = eneRandomPlayerIndex();
                    }
                    oniFightOniFacePlayer(o, w->target, 0.07853982f);
                    eneAnimPlayOnce(o, w->body, 1.0f);
                    if (a->frame >= 14.0f) {
                        w->state = 8;
                        w->armored = 1;
                        a->frame = 14.0f;
                        if (w->aniAtk != NULL) {
                            w->aniAtk = (ACollFrame *) aniAtkDelete(w->aniAtk);
                        }
                        w->aniAtk = aniAtkCreate(&ene_atk_root_frame->frame, &a->f, (u32) oniFightOniAtkChain, eneGetFile(o, 0), 0);
                        if (w->aniAtk != NULL) {
                            w->aniAtk->kind = 0x78;
                            w->aniAtk->power = (s32) o;
                        }
                    }
                    break;
                case 8: {
                    f32 px;
                    f32 py;
                    f32 pz;
                    f32 d;
                    s32 zone;

                    if (!plIsInPlay(w->target)) {
                        w->target = eneRandomPlayerIndex();
                    }
                    plAskPos(gGameWork.players[w->target], &px, &py, &pz, &zone);
                    d = fabsf(px - w->x);
                    if (oniFightOniFacePlayer(o, w->target, 0.15707964f) == 0 && (w->tick == 0 || w->tick == 5)) {
                        oniFightOniDust(o, 16.0f, 0.0f, -10.0f, 0.1f);
                        oniFightOniDust(o, -16.0f, 0.0f, -10.0f, 0.1f);
                    }
                    if (w->x <= px) {
                        w->vel[0] = 5;
                    } else {
                        w->vel[0] = -5.0f;
                    }
                    if (w->tick == 2 || w->tick == 7) {
                        oniFightOniDust(o, 16.0f, 0.0f, -10.0f, 0.1f);
                        oniFightOniDust(o, -16.0f, 0.0f, -10.0f, 0.1f);
                    }
                    if (d < 40.0f) {
                        w->timer = 10;
                        w->count = 0;
                        oniFightOniStop(o);
                        w->state = 9;
                    }
                    break;
                }
                case 9:
                    if (!plIsInPlay(w->target)) {
                        w->target = eneRandomPlayerIndex();
                    }
                    oniFightOniFacePlayer(o, w->target, 0.15707964f);
                    if (w->timer == 4) {
                        sndPlay(0x308);
                    }
                    if (a->frame >= 20.0f && w->count == 0) {
                        w->count = 1;
                        eneRumbleAll(0xBBBBBBBB, 30);
                        sndPlay(0x155);
                    }
                    if (w->timer-- <= 0) {
                        if (eneAnimPlayOnce(o, w->body, 1.0f) != 0) {
                            w->state = 4;
                        }
                        if (a->frame >= 27.0f) {
                            w->armored = 0;
                            if (w->aniAtk != NULL) {
                                w->aniAtk = (ACollFrame *) aniAtkDelete(w->aniAtk);
                            }
                        }
                    }
                    break;
                case 10:
                    a->anim = oniFightOniState10Anim;
                    a->frame = 0.0f;
                    w->armored = 1;
                    w->state = 11;
                    /* fallthrough */
                case 11:
                    if (!plIsInPlay(w->target)) {
                        w->target = eneRandomPlayerIndex();
                    }
                    if (oniFightOniFacePlayer(o, w->target, 0.31415927f) == 0 && (w->tick == 0 || w->tick == 5)) {
                        oniFightOniDust(o, 16.0f, 0.0f, -10.0f, 0.1f);
                        oniFightOniDust(o, -16.0f, 0.0f, -10.0f, 0.1f);
                    }
                    eneAnimPlayOnce(o, a, 1.0f);
                    if (a->frame >= 18.0f) {
                        SoundLocation2(0x307, gGameWork.view, a->pos[0], a->pos[1], a->pos[2], 0.0f, 0.0f,
                                       gGameWork.view->fovy);
                        oniFightOniDust(o, 16.0f, 0.0f, -10.0f, 0.1f);
                        oniFightOniDust(o, -16.0f, 0.0f, -10.0f, 0.1f);
                        if (eneFaceWay(o) == 1) {
                            w->vel[0] = 2.0f;
                        }
                        if (eneFaceWay(o) == 4) {
                            w->vel[0] = -2.0f;
                        }
                        w->state = 12;
                        w->vel[1] = 7;
                    }
                    break;
                case 12:
                    if (w->x <= 140.0f || w->x >= 600.0f) {
                        w->vel[0] = 0.0f;
                    }
                    eneAnimPlayOnce(o, a, 0.4f);
                    if (w->vel[1] < 0.0f) {
                        w->state = 13;
                    }
                    break;
                case 13:
                    if (w->x <= 140.0f || w->x >= 600.0f) {
                        w->vel[0] = 0.0f;
                    }
                    eneAnimPlayOnce(o, a, -0.2f);
                    if (a->frame <= 13.0f) {
                        a->frame = 13.0f;
                    }
                    if (w->hit & 1) {
                        a->frame = 13.0f;
                        oniFightOniDust(o, 18.0f, 0.0f, -10.0f, 0.2f);
                        oniFightOniDust(o, -18.0f, 0.0f, -10.0f, 0.2f);
                        oniFightOniStop(o);
                        w->armored = 0;
                        eneRumbleAll(0xBBBBBBBB, 10);
                        SoundLocation2(0x108, gGameWork.view, a->pos[0], a->pos[1], a->pos[2], 0.0f, 0.0f,
                                       gGameWork.view->fovy);
                        w->state = 14;
                    }
                    break;
                case 14:
                    if (eneAnimPlayOnce(o, a, -1.0f) != 0) {
                        w->state = 4;
                    }
                    break;
                case 15:
                    a->anim = oniFightOniState10Anim;
                    a->frame = 0.0f;
                    w->count = 4;
                    w->armored = 1;
                    w->state = 16;
                    /* fallthrough */
                case 16:
                    if (!plIsInPlay(w->target)) {
                        w->target = eneRandomPlayerIndex();
                    }
                    if (oniFightOniFacePlayer(o, w->target, 0.31415927f) == 0 && (w->tick == 0 || w->tick == 5)) {
                        oniFightOniDust(o, 16.0f, 0.0f, -10.0f, 0.1f);
                        oniFightOniDust(o, -16.0f, 0.0f, -10.0f, 0.1f);
                    }
                    eneAnimPlayOnce(o, a, 1.0f);
                    if (a->frame >= 18.0f) {
                        w->vel[1] = 6.0f;
                        w->vel[0] = -2.0f;
                        if (eneFaceWay(o) == 1) {
                            w->vel[0] = 2.0f;
                        }
                        if (eneFaceWay(o) == 4) {
                            w->vel[0] = -2.0f;
                        }
                        oniFightOniDust(o, 16.0f, 0.0f, -10.0f, 0.1f);
                        oniFightOniDust(o, -16.0f, 0.0f, -10.0f, 0.1f);
                        w->state = 17;
                    }
                    break;
                case 17:
                    if (w->x <= 140.0f) {
                        w->vel[0] = fabsf(w->vel[0]);
                    }
                    if (w->x >= 600.0f) {
                        w->vel[0] = -fabsf(w->vel[0]);
                    }
                    if (!plIsInPlay(w->target)) {
                        w->target = eneRandomPlayerIndex();
                    }
                    oniFightOniFacePlayer(o, w->target, 0.31415927f);
                    eneAnimPlayOnce(o, a, 0.8f);
                    if (w->vel[1] < 0.0f) {
                        a->anim = oniFightOniState17Anim;
                        a->frame = 0.0f;
                        w->state = 18;
                    }
                    break;
                case 18:
                    if (w->x <= 140.0f) {
                        w->vel[0] = fabsf(w->vel[0]);
                    }
                    if (w->x >= 600.0f) {
                        w->vel[0] = -fabsf(w->vel[0]);
                    }
                    if (!plIsInPlay(w->target)) {
                        w->target = eneRandomPlayerIndex();
                    }
                    oniFightOniFacePlayer(o, w->target, 0.31415927f);
                    eneAnimPlayOnce(o, a, 0.4f);
                    if (a->frame >= 5.0f) {
                        a->frame = 5.0f;
                    }
                    if (w->hit & 1) {
                        a->frame = 5.0f;
                        w->bounceVel = w->vel[0];
                        eneRumbleAll(0xBBBBBBBB, 10);
                        SoundLocation2(0x243, gGameWork.view, a->pos[0], a->pos[1], a->pos[2], 0.0f, 1000.0f,
                                       gGameWork.view->fovy);
                        oniFightOniStop(o);
                        oniFightOniDust(o, 25.0f, 0.0f, 0.0f, 0.2f);
                        oniFightOniDust(o, 0.0f, 0.0f, -10.0f, 0.2f);
                        oniFightOniDust(o, -25.0f, 0.0f, 0.0f, 0.2f);
                        w->state = 19;
                    }
                    break;
                case 19:
                    if (!plIsInPlay(w->target)) {
                        w->target = eneRandomPlayerIndex();
                    }
                    oniFightOniFacePlayer(o, w->target, 0.31415927f);
                    eneAnimPlayOnce(o, a, 1.0f);
                    if (a->frame >= 8.0f) {
                        if (w->count-- <= 0) {
                            w->state = 21;
                        } else {
                            a->frame = 8.0f;
                            w->state = 20;
                            w->vel[1] = 5.0f;
                            w->vel[0] = w->bounceVel;
                        }
                    }
                    break;
                case 20:
                    if (w->x <= 140.0f) {
                        w->vel[0] = fabsf(w->vel[0]);
                    }
                    if (w->x >= 600.0f) {
                        w->vel[0] = -fabsf(w->vel[0]);
                    }
                    if (!plIsInPlay(w->target)) {
                        w->target = eneRandomPlayerIndex();
                    }
                    oniFightOniFacePlayer(o, w->target, 0.31415927f);
                    eneAnimPlayOnce(o, a, -0.2f);
                    if (a->frame <= 5.0f) {
                        a->frame = 5.0f;
                    }
                    if (w->hit & 1) {
                        eneRumbleAll(0xBBBBBBBB, 10);
                        if (w->count == 0) {
                            SoundLocation2(0x108, gGameWork.view, a->pos[0], a->pos[1], a->pos[2], 0.0f, 0.0f,
                                           gGameWork.view->fovy);
                        } else {
                            SoundLocation2(0x243, gGameWork.view, a->pos[0], a->pos[1], a->pos[2], 0.0f, 1000.0f,
                                           gGameWork.view->fovy);
                        }
                        a->frame = 5.0f;
                        w->bounceVel = w->vel[0];
                        oniFightOniStop(o);
                        oniFightOniDust(o, 25.0f, 0.0f, 0.0f, 0.2f);
                        oniFightOniDust(o, 0.0f, 0.0f, -10.0f, 0.2f);
                        oniFightOniDust(o, -25.0f, 0.0f, 0.0f, 0.2f);
                        w->state = 19;
                    }
                    break;
                case 21:
                    if (eneAnimPlayOnce(o, a, 0.4f) != 0) {
                        w->timer = 10;
                        a->anim = oniFightOniState21Anim;
                        a->frame = 0.0f;
                        w->armored = 0;
                        w->state = 22;
                    }
                    break;
                case 22:
                    if (eneAnimPlayOnce(o, a, 0.4f) != 0) {
                        w->state = 4;
                    }
                    break;
                case 23:
                    a->anim = oniFightOniState10Anim;
                    a->frame = 0.0f;
                    w->count = 3;
                    w->state = 24;
                    /* fallthrough */
                case 24:
                    eneAnimPlayOnce(o, a, 1.2f);
                    if (a->frame >= 18.0f) {
                        oniFightOniDust(o, 16.0f, 0.0f, -10.0f, 0.1f);
                        oniFightOniDust(o, -16.0f, 0.0f, -10.0f, 0.1f);
                        w->state = 25;
                        w->vel[1] = 8;
                    }
                    break;
                case 25:
                    eneAnimPlayOnce(o, a, 0.5f);
                    if (w->y > 60.0f) {
                        oniFightOniJumpToPillar(o);
                        w->state = 26;
                        w->vel[1] = 0.0f;
                    }
                    break;
                case 26:
                    eneAnimPlayOnce(o, a, -0.2f);
                    if (a->frame <= 13.0f) {
                        a->frame = 13.0f;
                    }
                    if (w->hit & 1) {
                        CamView *view = gGameWork.view;

                        eneRumbleAll(0x88888888, 10);
                        SoundLocation2(0x108, view, a->pos[0], a->pos[1], a->pos[2], 150.0f, 700.0f, view->fovy);
                        a->frame = 13.0f;
                        oniFightOniDust(o, 18.0f, 0.0f, -10.0f, 0.2f);
                        oniFightOniDust(o, -18.0f, 0.0f, -10.0f, 0.2f);
                        w->state = 27;
                    }
                    break;
                case 27:
                    if (eneAnimPlayOnce(o, a, -1.2f) != 0) {
                        a->anim = oniFightOniState27Anim;
                        a->frame = 0.0f;
                        w->timer = 10;
                        w->state = 28;
                    }
                    break;
                case 28:
                    if (w->timer-- <= 0) {
                        if (a->frame >= 20.0f && a->frame <= 24.0f) {
                            oniFightPillarBreak(oniFightNearestPillar(o));
                        }
                        if (eneAnimPlayOnce(o, w->body, 1.0f) != 0) {
                            oniFightOniDust(o, 17.0f, 0.0f, -10.0f, 0.15f);
                            oniFightOniDust(o, -17.0f, 0.0f, -10.0f, 0.15f);
                            if (--w->count == 0) {
                                a->anim = oniFightOniState10Anim;
                                a->frame = 0.0f;
                                w->state = 29;
                            } else {
                                a->frame = 0.0f;
                                w->timer = 10;
                            }
                        }
                    }
                    break;
                case 29:
                    eneAnimPlayOnce(o, a, 1.0f);
                    if (a->frame >= 18.0f) {
                        oniFightOniDust(o, 16.0f, 0.0f, -10.0f, 0.1f);
                        oniFightOniDust(o, -16.0f, 0.0f, -10.0f, 0.1f);
                        w->state = 30;
                        w->vel[1] = 9;
                    }
                    break;
                case 30:
                    eneAnimPlayOnce(o, a, 0.4f);
                    if (w->y > 80.0f) {
                        if (!plIsInPlay(w->target)) {
                            w->target = eneRandomPlayerIndex();
                        }
                        oniFightOniFacePlayer(o, w->target, 3.1415927f);
                        w->state = 13;
                        w->z = 0.0f;
                    }
                    break;
                case 31:
                    a->anim = oniFightOniState31Anim;
                    a->frame = 0.0f;
                    w->count = 5;
                    w->state = 32;
                    w->vel[1] = 5;
                    /* fallthrough */
                case 32:
                    ang = eneCourseAngle(o, 8);
                    eneTurnToward(o, ang, 0.10471976f);
                    if (eneAnimPlayOnce(o, a, 1.0f) != 0) {
                        oniFightOniShowBall(o);
                        w->state = 33;
                    }
                    break;
                case 33:
                    ang = eneCourseAngle(o, 8);
                    eneTurnToward(o, ang, 0.10471976f);
                    oniFightOniSpin(o, 0.62831855f);
                    if (w->hit & 1) {
                        CamView *view = gGameWork.view;
                        f32 px;
                        f32 py;
                        f32 pz;
                        s32 zone;

                        SoundLocation2(0x10B, view, a->pos[0], a->pos[1], a->pos[2], 0.0f, 1000.0f, view->fovy);
                        w->rollSnd = soundActpanCreate(o, 0x2B1, gGameWork.view, 100.0f, 600.0f, gGameWork.view->fovy);
                        eneRumbleAll(0xBBBBBBBB, 8);
                        w->rumble = 8;
                        oniFightOniDust(o, 25.0f, 0.0f, 0.0f, 0.1f);
                        oniFightOniDust(o, 0.0f, 0.0f, -10.0f, 0.1f);
                        oniFightOniDust(o, -25.0f, 0.0f, 0.0f, 0.1f);
                        if (!plIsInPlay(w->target)) {
                            w->target = eneRandomPlayerIndex();
                        }
                        plAskPos(gGameWork.players[w->target], &px, &py, &pz, &zone);
                        if (px <= w->x) {
                            w->vel[0] = 2.0f;
                        } else {
                            w->vel[0] = -2.0f;
                        }
                        w->count = 5;
                        w->state = 34;
                        w->vel[2] = 5.0f;
                    }
                    break;
                case 34:
                    oniFightOniSpin(o, 0.62831855f);
                    oniFightOniRollSteer(o);
                    if (w->dustTick == 0) {
                        oniFightOniDust(o, 25.0f, 0.0f, 0.0f, 0.1f);
                    }
                    if (w->dustTick == 2) {
                        oniFightOniDust(o, 0.0f, 0.0f, 10.0f, 0.1f);
                    }
                    if (w->dustTick == 4) {
                        oniFightOniDust(o, -25.0f, 0.0f, 0.0f, 0.1f);
                    }
                    if (++w->dustTick >= 6) {
                        w->dustTick = 0;
                    }
                    if (w->rumble-- <= 0) {
                        w->rumble = 8;
                        eneRumbleAll(0xBBBBBBBB, 8);
                        SoundLocation2(0x10B, gGameWork.view, a->pos[0], a->pos[1], a->pos[2], 0.0f, 1000.0f,
                                       gGameWork.view->fovy);
                    }
                    if (oniFightOniRollHit(o) != 0) {
                        oniFightOniStop(o);
                        w->state = 35;
                        w->vel[1] = 4.0f;
                    }
                    if (w->count <= 0 && w->z >= 0.0f) {
                        w->z = 0.0f;
                        oniFightOniStop(o);
                        w->state = 38;
                    }
                    break;
                case 35:
                    oniFightOniSpin(o, -0.62831855f);
                    if (w->hit & 1) {
                        CamView *view = gGameWork.view;
                        f32 px;
                        f32 py;
                        f32 pz;
                        s32 zone;

                        eneRumbleAll(0xBBBBBBBB, 8);
                        SoundLocation2(0x10B, view, a->pos[0], a->pos[1], a->pos[2], 0.0f, 1000.0f, view->fovy);
                        w->rumble = 8;
                        if (!plIsInPlay(w->target)) {
                            w->target = eneRandomPlayerIndex();
                        }
                        plAskPos(gGameWork.players[w->target], &px, &py, &pz, &zone);
                        w->vel[0] = (px - w->x) * 5.0f / 90.0f;
                        w->vel[2] = -5.0f;
                        oniFightOniDust(o, 25.0f, 0.0f, 0.0f, 0.2f);
                        oniFightOniDust(o, 0.0f, 0.0f, 10.0f, 0.2f);
                        oniFightOniDust(o, -25.0f, 0.0f, 0.0f, 0.2f);
                        w->state = 36;
                    }
                    break;
                case 36:
                    oniFightOniSpin(o, -0.62831855f);
                    oniFightOniRollSteer(o);
                    if (w->dustTick == 0) {
                        oniFightOniDust(o, 25.0f, 0.0f, 0.0f, 0.1f);
                    }
                    if (w->dustTick == 2) {
                        oniFightOniDust(o, 0.0f, 0.0f, -10.0f, 0.1f);
                    }
                    if (w->dustTick == 4) {
                        oniFightOniDust(o, -25.0f, 0.0f, 0.0f, 0.1f);
                    }
                    if (++w->dustTick >= 6) {
                        w->dustTick = 0;
                    }
                    if (w->rumble-- <= 0) {
                        eneRumbleAll(0xBBBBBBBB, 8);
                        w->rumble = 8;
                        SoundLocation2(0x10B, gGameWork.view, a->pos[0], a->pos[1], a->pos[2], 0.0f, 1000.0f,
                                       gGameWork.view->fovy);
                    }
                    if (oniFightOniRollHit(o) != 0) {
                        oniFightOniStop(o);
                        w->state = 37;
                        w->vel[1] = 4.0f;
                    }
                    break;
                case 37:
                    oniFightOniSpin(o, 0.62831855f);
                    if (w->hit & 1) {
                        CamView *view = gGameWork.view;
                        f32 px;
                        f32 py;
                        f32 pz;
                        s32 zone;

                        SoundLocation2(0x10B, view, a->pos[0], a->pos[1], a->pos[2], 0.0f, 1000.0f, view->fovy);
                        eneRumbleAll(0xBBBBBBBB, 8);
                        w->rumble = 8;
                        if (!plIsInPlay(w->target)) {
                            w->target = eneRandomPlayerIndex();
                        }
                        plAskPos(gGameWork.players[w->target], &px, &py, &pz, &zone);
                        w->vel[0] = (px - w->x) * 5.0f / 80.0f;
                        w->vel[2] = 5.0f;
                        oniFightOniDust(o, 25.0f, 0.0f, 0.0f, 0.2f);
                        oniFightOniDust(o, 0.0f, 0.0f, -10.0f, 0.2f);
                        oniFightOniDust(o, -25.0f, 0.0f, 0.0f, 0.2f);
                        w->count--;
                        w->state = 34;
                    }
                    break;
                case 38: {
                    f32 step = 0.62831855f;

                    if (w->dir < 0) {
                        step = -0.62831855f;
                    }
                    if (oniFightOniSpin(o, step) != 0) {
                        oniFightOniStop(o);
                        w->rot[0] = 0.0f;
                        oniFightOniShowBody(o);
                        a->anim = oniFightOniState31Anim;
                        a->frame = polAnimeLastFrame(a);
                        tskDestroy(w->rollSnd);
                        w->state = 39;
                    }
                    break;
                }
                case 39:
                    if (eneAnimPlayOnce(o, a, -0.8f) != 0) {
                        w->state = 4;
                    }
                    break;
                case 40:
                    a->anim = oniFightOniState40Anim;
                    a->frame = 0.0f;
                    oniFightOniStop(o);
                    w->state = 41;
                    /* fallthrough */
                case 41:
                    if (w->rot[1] > 3.1415927f) {
                        eneTurnToward(o, 2.3561945f, 0.07853982f);
                    } else {
                        eneTurnToward(o, 0.7853982f, 0.07853982f);
                    }
                    eneAnimPlayOnce(o, a, 1.0f);
                    if (a->frame >= 9.0f) {
                        a->frame = 9.0f;
                        w->vel[1] = 4.0f;
                        SoundLocation2(0x309, gGameWork.view, a->pos[0], a->pos[1], a->pos[2], 0, 0,
                                       gGameWork.view->fovy);
                        w->state = 42;
                    }
                    break;
                case 42:
                    if (w->rot[1] > 3.1415927f) {
                        eneTurnToward(o, 2.3561945f, 0.07853982f);
                    } else {
                        eneTurnToward(o, 0.7853982f, 0.07853982f);
                    }
                    w->vel[2] = -2.5f;
                    if (w->z <= -40.0f) {
                        w->z = -40.0f;
                        w->vel[2] = 0.0f;
                    }
                    eneAnimPlayOnce(o, a, 0.8f);
                    if (a->frame >= 15.0f) {
                        a->frame = 15.0f;
                    }
                    w->alpha += 4;
                    if (w->alpha >= 0xF0) {
                        w->alpha = 0xF0;
                    }
                    if (w->hit & 1) {
                        eneRumbleAll(0xBBBBBBBB, 10);
                        SoundLocation2(0x108, gGameWork.view, a->pos[0], a->pos[1], a->pos[2], 0, 0,
                                       gGameWork.view->fovy);
                        oniFightOniStop(o);
                        oniFightOniDust(o, 20.0f, 0.0f, 15, 0.2f);
                        oniFightOniDust(o, -20.0f, 0.0f, 15, 0.2f);
                        oniFightOniDust(o, 20.0f, 0.0f, -15.0f, 0.2f);
                        oniFightOniDust(o, -20.0f, 0.0f, -15.0f, 0.2f);
                        oniFightSpiritLeap(oniFightGetSpirit(o->parent));
                        w->state = 43;
                    }
                    break;
                case 43: {
                    s32 done = eneAnimPlayOnce(o, a, 1.0f);

                    if (w->rot[1] > 3.1415927f) {
                        eneTurnToward(o, 2.3561945f, 0.07853982f);
                    } else {
                        eneTurnToward(o, 0.7853982f, 0.07853982f);
                    }
                    w->alpha += 4;
                    if (w->alpha >= 0xF0) {
                        w->alpha = 0xF0;
                    }
                    if (done != 0 && w->alpha == 0xF0) {
                        w->alpha = 0xF0;
                        w->state = 44;
                    }
                    break;
                }
                case 44:
                    if (oniFightSpiritIsDown(oniFightGetSpirit(o->parent)) != 0) {
                        w->alpha -= 12;
                        if (w->alpha == 0) {
                            soundIncamCam(0x307, a->pos[0], a->pos[1], a->pos[2]);
                            a->frame = 15.0f;
                            w->glowAlpha = 0xF0;
                            w->state = 45;
                            w->vel[1] = 5;
                        }
                    }
                    break;
                case 45:
                    w->vel[2] = 2.5f;
                    if (w->z >= 0.0f) {
                        w->vel[2] = 0.0f;
                        w->z = 0.0f;
                    }
                    eneAnimPlayOnce(o, a, -0.5f);
                    if (a->frame <= 7.0f) {
                        a->frame = 7.0f;
                    }
                    if (!plIsInPlay(w->target)) {
                        w->target = eneRandomPlayerIndex();
                    }
                    oniFightOniFacePlayer(o, w->target, 0.31415927f);
                    if (w->hit & 1) {
                        SoundLocation2(0x108, gGameWork.view, a->pos[0], a->pos[1], a->pos[2], 0, 0,
                                       gGameWork.view->fovy);
                        oniFightOniDust(o, 16.0f, 0.0f, -10.0f, 0.2f);
                        oniFightOniDust(o, -16.0f, 0.0f, -10.0f, 0.2f);
                        w->state = 46;
                        w->z = 0.0f;
                    }
                    break;
                case 46: {
                    s32 done = eneAnimPlayOnce(o, a, -1.0f);

                    if (!plIsInPlay(w->target)) {
                        w->target = eneRandomPlayerIndex();
                    }
                    if (oniFightOniFacePlayer(o, w->target, 0.31415927f) != 0 && done != 0) {
                        w->state = 4;
                    }
                    break;
                }
                case 47:
                    w->z = -40.0f;
                    w->x = 360.0f;
                    w->y = 0.0f;
                    break;
            }
            eneFall(o, 0.4f);
            eneMoveObj(o);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 40.0f) != 0) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneSetModelOnCourse(o, &a->f);
            oniFightOniSyncBall(o);
            shadowPut(w->shadow, a->pos[0], a->pos[1], a->pos[2], 4.0f, NULL);
            if (w->glowAlpha < 12) {
                w->glowAlpha = 0;
            } else {
                w->glowAlpha -= 12;
            }
            {
                f32 dx;
                f32 dy;
                f32 dz;

                oniFightCamDir(30.0f, &dx, &dy, &dz);
                glow->pos[0] = a->pos[0] + dx;
                glow->pos[1] = a->pos[1] + dy + 40.0f;
                glow->pos[2] = a->pos[2] + dz;
                glow->scale[0] = 6.0f;
                glow->scale[1] = 6.0f;
                glow->scale[2] = 6.0f;
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void oniFightOniSetState(Obj *o, s32 state, s32 wait) {
    ONI(o)->state = state;
    ONI(o)->wait = wait;
    ONI(o)->steps++;
}

void oniFightOniStop(Obj *o) {
    ONI(o)->vel[0] = 0.0f;
    ONI(o)->vel[1] = 0.0f;
    ONI(o)->vel[2] = 0.0f;
}

void oniFightOniWake(Obj *o) {
    ONI(o)->state = 1;
}

void oniFightOniStart(Obj *o) {
    ONI(o)->alpha = 0;
    ONI(o)->glowAlpha = 0;
    ONI(o)->state = 4;
}

void oniFightOniPark(Obj *o) {
    ONI(o)->state = 47;
}

void oniFightOniShowBody(Obj *o) {
    AnimeFrame *a = ONI(o)->body;
    PolFrame *b = ONI(o)->body2;

    a->f.flags |= 2;
    b->f.flags &= ~2;
}

void oniFightOniShowBall(Obj *o) {
    AnimeFrame *a = ONI(o)->body;
    PolFrame *b = ONI(o)->body2;

    a->f.flags &= ~2;
    b->f.flags |= 2;
}

void oniFightOniSyncBall(Obj *o) {
    AnimeFrame *a = ONI(o)->body;
    PolFrame *b = ONI(o)->body2;

    b->pos[0] = a->pos[0];
    b->pos[1] = a->pos[1] + 23.0f;
    b->pos[2] = a->pos[2];
    b->rot[0] = a->rot[0];
    b->rot[1] = a->rot[1];
    b->rot[2] = a->rot[2];
    b->scale[0] = a->scale[0];
    b->scale[1] = a->scale[1];
    b->scale[2] = a->scale[2];
}

void oniFightSparkle(f32 x, f32 y, f32 z, f32 size) {
    ItexInit init;

    init = gItexTemplate;
    init.file = 0x123;
    init.frames = gItexSparkleAnim;
    init.flags = 3;
    init.prim.r = 0xFF; init.prim.g = 0xE0; init.prim.b = 0xA0;
    init.env.r = 0x60; init.env.g = 0x60; init.env.b = 0x50;
    init.pos[0] = x;
    init.pos[1] = y;
    init.pos[2] = z;
    init.grow.x = init.grow.y = init.grow.z = 1.01f;
    init.scale[2] = size;
    init.scale[1] = size;
    init.scale[0] = size;
    tskCreate(gEffectTask, itex, 0, &init);
}

void oniFightOniDust(Obj *o, f32 x, f32 y, f32 z, f32 size) {
    s32 pad;
    AnimeFrame *b = ONI(o)->body;

    mathVecRotateY(ONI(o)->rot[1], &x, &y, &z);
    oniFightSparkle(b->pos[0] + x, b->pos[1], b->pos[2] + z, size);
}

s32 oniFightOniSpin(Obj *o, f32 step) {
    s32 r = 0;
    OniWork *w = ONI(o);

    if (step >= 0.0f) {
        w->dir = 1;
    }
    if (step < 0.0f) {
        w->dir = -1;
    }
    w->rot[0] += step;
    if (w->rot[0] >= 6.283185482f) {
        w->rot[0] -= 6.283185482f;
        r = 1;
    }
    if (w->rot[0] <= 0.0f) {
        w->rot[0] += 6.283185482f;
        r = 1;
    }
    return r;
}

void oniFightOniRollSteer(Obj *o) {
    OniWork *w = ONI(o);
    f32 c;
    f32 a;

    if (w->x <= 150.0f) {
        w->vel[0] = fabsf(w->vel[0]);
    }
    if (w->x >= 600.0f) {
        w->vel[0] = fabsf(w->vel[0]) * -1.0f;
    }
    if (w->vel[0] > 3.0f) {
        w->vel[0] = 3.0f;
    }
    if (w->vel[0] < -3.0f) {
        w->vel[0] = -3.0f;
    }
    a = mathAtan2(fabsf(w->vel[0]), fabsf(w->vel[2]));
    c = eneCourseAngle(o, 8);
    if (w->vel[0] < 0.0f) {
        a *= -1.0f;
    }
    if (w->vel[2] < 0.0f) {
        a *= -1.0f;
    }
    if (w->vel[0] == 0.0f && w->vel[2] == 0.0f) {
        a = 0.0f;
    }
    eneTurnToward(o, c + a, 0.31415927f);
}

s32 oniFightOniRollHit(Obj *o) {
    OniWork *w = ONI(o);
    f32 nx = w->x + ((Vec3f *) w->vel)->x;
    f32 nz = w->z + ((Vec3f *) w->vel)->z;

    if (w->vel[2] >= 0.0f) {
        if (w->z >= 90.0f) {
            return TRUE;
        }
        if (w->z < -135.0f && nz >= -135.0f) {
            if (nx >= 490.0f && nx <= 550.0f) {
                return TRUE;
            }
            if (nx >= 340.0f && nx <= 400.0f) {
                return TRUE;
            }
            if (nx >= 190.0f && nx <= 250.0f) {
                return TRUE;
            }
        }
    } else {
        if (w->z <= -80.0f) {
            return TRUE;
        }
        if (w->z > -80.0f && nz <= -80.0f) {
            if (nx >= 490.0f && nx <= 550.0f) {
                return TRUE;
            }
            if (nx >= 340.0f && nx <= 400.0f) {
                return TRUE;
            }
            if (nx >= 190.0f && nx <= 250.0f) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

void oniFightOniPlace(Obj *o, f32 x, f32 z, f32 ry) {
    ONI(o)->x = x;
    ONI(o)->z = z;
    ONI(o)->y = 80.0f;
    ONI(o)->rot[1] = ry;
    ONI(o)->rot[0] = 0.0f;
    ONI(o)->rot[2] = 0.0f;
}

void oniFightOniJumpToPillar(Obj *o) {
    OniWork *w = ONI(o);

    if (w->x < 220.0f) {
        oniFightOniPlace(o, 160.0f, -80.0f, 2.3561945f);
    } else if (w->x < 295.0f) {
        oniFightOniPlace(o, 280.0f, -125.0f, -0.7853982f);
    } else if (w->x < 370.0f) {
        oniFightOniPlace(o, 310.0f, -80.0f, 2.3561945f);
    } else if (w->x < 445.0f) {
        oniFightOniPlace(o, 430.0f, -125.0f, -0.7853982f);
    } else if (w->x < 520.0f) {
        oniFightOniPlace(o, 460.0f, -80.0f, 2.3561945f);
    } else {
        oniFightOniPlace(o, 580.0f, -125.0f, -0.7853982f);
    }
}

Obj *oniFightNearestPillar(Obj *o) {
    f32 x = ONI(o)->x;
    MainWork *m = MAIN(o->parent);

    if (x < 295.0f) {
        return m->pillars[0];
    } else if (x < 445.0f) {
        return m->pillars[1];
    } else {
        return m->pillars[2];
    }
}

s32 oniFightOniFacePlayer(Obj *o, s32 pl, f32 step) {
    s32 pad1;
    f32 px;
    f32 py;
    f32 pz;
    f32 way;
    f32 a;
    s32 pad;
    s32 zone;

    step = fabsf(step);
    plAskPos(gGameWork.players[pl], &px, &py, &pz, &zone);
    if (ONI(o)->x <= px) {
        way = 1.0f;
    } else {
        way = 4.0f;
    }
    a = eneCourseAngle(o, way);
    if (eneTurnToward(o, a, step) != 0) {
        return TRUE;
    }
    return FALSE;
}

void oniFightDoorPanelPlace(PolFrame *p, f32 x, f32 y, f32 z) {
    frmMove(&p->f, gFrmEffectLayer);
    p->pos[0] = x;
    p->pos[1] = y;
    p->pos[2] = z;
}

/* "door". The list is passed as a pointer (SegToRamFunc): IDO then builds fadeSegToRam's address
 * again at each of the four calls, as the game does. It matched once the last panel was read
 * through a pointer to the panels (the permuter's find, 2026-10-04); before
 * that, fadeSegToRam's dead temp took 0x20 and its address 0x24, 12 lines of 336.
 * 2026-10-17 (a kept FAKE): that pointer's only effect was one more
 * word of cfe's local area (homes 16 against 12); its home is never read. One pad word in its
 * place, the last panel read as the others: MATCH, object identical. */
s32 oniFightDoorProc(Obj *o, s16 msg, void *arg) {
    s32 pad; /* FAKE: OoT's frame pad, the game's 4th word of locals (homes 16; without it 12) */
    DoorWork *w = DOOR(o);
    AnimeFrame *a;

    switch (msg) {
        case 2:
            o->name = "door";
            eneSetup(o, arg);
            w->anim = eneNewAnime(o, (u32) oniFightDoorAnim);
            if (w->anim == NULL) {
                return -1;
            }
            a = w->anim;
            a->pos[0] = 247.0f;
            a->frame = 0.0f;
            a->pos[1] = 0.0f;
            a->pos[2] = 51.0f;
            w->panels[0] = eneNewPol2(o, oniFightDoorPanelDl0, (Gfx *) ((SegToRamFunc) fadeSegToRam)(oniFightDoorDl, o->file));
            if (w->panels[0] == NULL) {
                return -1;
            }
            oniFightDoorPanelPlace(w->panels[0], 0.0f, 0.0f, 0.0f);
            w->panels[1] = eneNewPol2(o, oniFightDoorPanelDl1, (Gfx *) ((SegToRamFunc) fadeSegToRam)(oniFightDoorDl, o->file));
            if (w->panels[1] == NULL) {
                return -1;
            }
            oniFightDoorPanelPlace(w->panels[1], 70.0f, 0.0f, 0.0f);
            w->panels[2] = eneNewPol2(o, oniFightDoorPanelDl2, (Gfx *) ((SegToRamFunc) fadeSegToRam)(oniFightDoorDl, o->file));
            if (w->panels[2] == NULL) {
                return -1;
            }
            oniFightDoorPanelPlace(w->panels[2], 0.0f, 0.0f, 0.0f);
            w->panels[3] = eneNewPol2(o, oniFightDoorPanelDl3, (Gfx *) ((SegToRamFunc) fadeSegToRam)(oniFightDoorDl, o->file));
            if (w->panels[3] == NULL) {
                return -1;
            }
            oniFightDoorPanelPlace(w->panels[3], -45.0f, 0.0f, 0.0f);
            break;
        case 1:
            if (w->anim != NULL) {
                frmDestroy(&w->anim->f);
            }
            if (w->panels[0] != NULL) {
                frmDestroy(&w->panels[0]->f);
            }
            if (w->panels[1] != NULL) {
                frmDestroy(&w->panels[1]->f);
            }
            if (w->panels[2] != NULL) {
                frmDestroy(&w->panels[2]->f);
            }
            if (w->panels[3] != NULL) {
                frmDestroy(&w->panels[3]->f);
            }
            break;
        case 0:
            a = w->anim;
            switch (w->state) {
                case 0:
                    break;
                case 1:
                    if (eneAnimPlayOnce(o, a, 0.02f) != 0) {
                        w->state = 0;
                    }
                    break;
                case 2:
                    if (eneAnimPlayOnce(o, a, -0.2f) != 0) {
                        w->state = 0;
                        eneRumbleAll(0xBBBBBBBB, 10);
                        sndPlay(0x16A);
                        oniFightSparkle(a->pos[0] - 2.0f, a->pos[1] + 7.0f, a->pos[2], 0.1f);
                        oniFightSparkle(a->pos[0] - 2.0f, a->pos[1] + 7.0f + 10.0f, a->pos[2], 0.1f);
                        oniFightSparkle(a->pos[0] - 2.0f, a->pos[1] + 7.0f + 20.0f, a->pos[2], 0.1f);
                        oniFightSparkle(a->pos[0] - 2.0f, a->pos[1] + 7.0f + 30.0f, a->pos[2], 0.1f);
                        oniFightSparkle(a->pos[0] - 2.0f, a->pos[1] + 7.0f + 40.0f, a->pos[2], 0.1f);
                        oniFightSparkle(a->pos[0] - 2.0f, a->pos[1] + 7.0f + 50.0f, a->pos[2], 0.1f);
                        oniFightSparkle(a->pos[0] - 2.0f, a->pos[1] + 7.0f + 60.0f, a->pos[2], 0.1f);
                        oniFightSparkle(a->pos[0] - 2.0f, a->pos[1] + 7.0f + 70.0f, a->pos[2], 0.1f);
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void oniFightDoorOpen(Obj *o) {
    sndPlay(0x2F4);
    DOOR(o)->state = 1;
}

void oniFightDoorShut(Obj *o) {
    DOOR(o)->state = 2;
}

void oniFightDoorMakeBarrier(Obj *o) {
    DoorWork *w = DOOR(o);
    s32 pad;

    w->fx[0] = eneMakeUnder(o, ENE_TANU_WALL_FLAGS_3, 1, 108.0f, -10.0f, 0.0f);
    w->fx[1] = eneMakeUnder(o, ENE_TANU_WALL_FLAGS_3, 1, 630.0f, 0.0f, 0.0f);
}

void oniFightDoorKillBarrier(Obj *o) {
    s32 pad1;
    s32 pad2;

    tskDestroy(DOOR(o)->fx[0]);
    tskDestroy(DOOR(o)->fx[1]);
}

/* "pillar" */
s32 oniFightPillarProc(Obj *o, s16 msg, void *arg) {
    PillarWork *w = PILLAR(o);
    s32 pad;
    PolFrame *m;

    switch (msg) {
        case 2:
            o->name = "pillar";
            eneSetup(o, arg);
            w->model = eneNewPol(o, oniFightPillarDl);
            if (w->model == NULL) {
                return -1;
            }
            m = w->model;
            frmMove(&m->f, gFrmLayer3);
            eneSetModel(o, &m->f);
            w->shake = 0;
            w->x0 = w->x;
            w->z0 = w->z;
            w->sx = m->scale[0];
            w->sy = m->scale[1];
            w->silent = 0;
            w->timer = 0;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
        case 0:
            m = w->model;
            if (w->timer-- <= 0) {
                w->timer = 0;
            }
            if (w->timer == 1) {
                sndPlay(0x824F);
            }
            switch (w->state) {
                case 0:
                    w->shake = 20;
                    w->x = w->x0;
                    w->z = w->z0;
                    m->scale[0] = w->sx;
                    m->scale[1] = w->sy;
                    break;
                case 1:
                    if (w->shake-- >= 11) {
                        w->x = ((w->shake & 1) * 2 - 1) + w->x0;
                        if (w->shake & 2) {
                            m->scale[0] /= 1.1;
                            m->scale[1] = m->scale[0] / 1.1;
                        } else {
                            m->scale[0] *= 1.1;
                            m->scale[1] = m->scale[0] * 1.1;
                        }
                    } else {
                        w->state = 0;
                    }
                    break;
            }
            eneSetModel(o, &m->f);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void oniFightPillarBreak(Obj *o) {
    s32 pad;
    CamView *view;
    PillarWork *w = PILLAR(o);
    PolFrame *m = w->model;

    if (w->state == 0) {
        view = gGameWork.view;
        SoundLocation2(0x155, view, m->pos[0], m->pos[1], m->pos[2], 150.0f, 700.0f, view->fovy);
        eneRumbleAll(0xBBBBBBBB, 30);
        w->timer = 45;
        sndPlay(0x24F);
        oniFightPillarScatter(o);
        w->state = 1;
    }
}

/* "hahen" */
s32 oniFightHahenProc(Obj *o, s16 msg, void *arg) {
    HahenWork *w = HAHEN(o);
    PolFrame *m;
    s32 pad[3];
    u32 full;
    s32 pad2[3];

    switch (msg) {
        case 2:
            o->name = "hahen";
            full = eneSetup(o, arg);
            w->model = eneNewPol(o, oniFightHahenDl);
            if (w->model == NULL) {
                return -1;
            }
            m = w->model;
            frmMove(&m->f, gFrmLayer3);
            m->dl2 = NULL;
            m->scale[0] = m->scale[1] = m->scale[2] = (fadeRand(2) + 2.0f) * 0.01f;
            if (full) {
                w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &m->f, 100, 0, 0, 0);
                if (w->atk != NULL) {
                    w->atk->kind = 0x78;
                }
                w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &m->f, 100, 200, -100, 0, 0);
                w->dmg->kind = 0x65;
                m->scale[0] = 0.07f;
                m->scale[1] = 0.07f;
                m->scale[2] = 0.07f;
                w->shadow = shadowCreate();
            }
            w->rot[0] = fadeRand(360) * 3.1415927f / 180.0f;
            w->rot[1] = fadeRand(360) * 3.1415927f / 180.0f;
            w->rot[2] = fadeRand(360) * 3.1415927f / 180.0f;
            w->spin[0] = fadeRand(20) * 3.1415927f / 180.0f - 0.17453294f;
            w->spin[1] = fadeRand(20) * 3.1415927f / 180.0f - 0.17453294f;
            w->count = fadeRand(20);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            m = w->model;
            if (w->atk != NULL && acollAtkWasHit(w->atk) == 1) {
                if ((Obj *) w->atk->hitPower != NULL) {
                    tskSendMsg((Obj *) w->atk->hitPower, 7, o);
                }
            }
            if (w->dmg != NULL && acollDmgWasHit(w->dmg) == 1) {
                eneHitSoundAlt(w->dmg);
                eneMakeEx(o, ENE_ONI_FIGHT_HAHEN2, 0, m->pos[0], m->pos[1], m->pos[2], 2, NULL);
                itexPuffWhite(m->pos[0], m->pos[1], m->pos[2], 0.15f);
                return -1;
            }
            switch (w->state) {
                case 0:
                    if (w->count-- < 0) {
                        w->count = 0;
                        w->vel[1] = 0.0f;
                        m->dl2 = oniFightHahenDl;
                        w->state++;
                    }
                    break;
                case 1:
                    eneFall(o, 0.4f);
                    eneMoveObj(o);
                    w->hit &= ~1;
                    if (eneLandOff((EneObj *) w, 30.0f, m->scale[1] * -150.0f) != 0) {
                        if (w->count == 0) {
                            if (PILLAR(o->parent)->silent == 0) {
                                (*oniFightHahenLandSound)(&PILLAR(o->parent)->sndAlt, m->pos[0], m->pos[1],
                                                          m->pos[2]);
                            }
                            oniFightSparkle(m->pos[0], m->pos[1], m->pos[2], 0.1f);
                        }
                        if (w->count++ == 1) {
                            return -1;
                        }
                        w->vel[1] = fabsf(w->vel[1]) * 0.4f;
                        w->vel[0] = fadeRand(30) * 0.1f - 1.5;
                        w->vel[2] = fadeRand(30) * 0.1f - 1.5;
                        w->spin[0] = fadeRand(20) * 3.1415927f / 180.0f - 0.17453294f;
                        w->spin[1] = fadeRand(20) * 3.1415927f / 180.0f - 0.17453294f;
                    } else {
                        w->rot[0] += w->spin[0];
                        w->rot[1] += w->spin[1];
                        if (w->y < -10.0f) {
                            return -1;
                        }
                    }
                    if (w->x >= 610.0f) {
                        w->vel[0] = fabsf(w->vel[0]) * -1.0f;
                    }
                    if (w->x <= 120.0f) {
                        w->vel[0] = fabsf(w->vel[0]);
                    }
                    break;
            }
            eneSetModelOnCourse(o, &m->f);
            if (w->shadow != NULL) {
                shadowPut(w->shadow, m->pos[0], m->pos[1], m->pos[2], 2.5f, NULL);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void oniFightPillarScatter(Obj *o) {
    f32 x;
    s32 pad;
    s32 arg;

    x = fadeRand(30) + 100;
    arg = 1;
    while (x < 610.0f) {
        eneMakeEx(o, ENE_ONI_FIGHT_HAHEN, 1, x, 85.0f, 0.0f, 2, (u32 (*)(void *, void *, void *)) &arg);
        x = fadeRand(20) + (x + 50.0f);
    }
    x = fadeRand(30) + 100;
    arg = 1;
    while (x < 610.0f) {
        eneMakeEx(o, ENE_ONI_FIGHT_HAHEN, 1, x, 85.0f, -(f32) (fadeRand(50) + 20), 2,
                  (u32 (*)(void *, void *, void *)) &arg);
        x = fadeRand(60) + (x + 80.0f);
    }
    x = fadeRand(30) + 100;
    while (x < 610.0f) {
        eneMakeEx(o, ENE_ONI_FIGHT_HAHEN, 1, x, 85.0f, fadeRand(30) + 30, 2, (u32 (*)(void *, void *, void *)) &arg);
        x = fadeRand(60) + (x + 80.0f);
    }
}

/* A piece's landing sound at (x, y, z), alternating 0x240 and 0x23F, panned for the game's
 * camera if seen: SoundIncam2's (sndloc.c) code written out, with the view's near and far read
 * first. */
void oniFightHahenLandSound(s32 *alt, f32 x, f32 y, f32 z) {
    s16 pan;
    f32 dist;
    f32 cs;
    f32 s;
    f32 c;
    Vec3f d;
    Vec3f v;
    CamView *view = gGameWork.view;
    f32 near;
    f32 far;

    near = view->zNear;
    far = view->zFar;
    d.x = x - view->eye[0];
    d.y = y - view->eye[1];
    d.z = z - view->eye[2];
    dist = sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);
    if (dist < near || far < dist) {
        return;
    }
    s = mathSinCos((s16) (fabsf(view->fovy) * 0.8f / 3.1415927f * 512.0f), &c);
    v.x = view->at[0] - view->eye[0];
    v.y = view->at[1] - view->eye[1];
    v.z = view->at[2] - view->eye[2];
    cs = (v.x * d.x + v.y * d.y + v.z * d.z) /
         sqrtf((v.x * v.x + v.y * v.y + v.z * v.z) * (d.x * d.x + d.y * d.y + d.z * d.z));
    if (cs < c) {
        return;
    }
    v.x = view->eye[2] - view->at[2];
    v.z = view->at[0] - view->eye[0];
    pan = (s16) ((v.x * d.x + v.z * d.z) / sqrtf((v.x * v.x + v.z * v.z) * (d.x * d.x + d.z * d.z)) /
                 s * 64.0f) +
          64;
    if (pan <= 0) {
        pan = 1;
    } else if (pan >= 128) {
        pan = 127;
    }
    *alt ^= 1;
    if (*alt != 0) {
        sndPlayPan(0x240, pan);
    } else {
        sndPlayPan(0x23F, pan);
    }
}

void oniFightHahen2PieceInit(Obj *o, PolFrame *p, s32 i) {
    Hahen2Work *w = HAHEN2(o);

    if (p != NULL) {
        frmMove(&p->f, gFrmLayer3);
        p->pos[0] = w->x;
        p->pos[1] = w->y;
        p->pos[2] = w->z;
        p->scale[0] = p->scale[1] = p->scale[2] = 0.02f;
        p->rot[0] = fadeRand(360) * 3.1415927f / 180.0f;
        p->rot[1] = fadeRand(360) * 3.1415927f / 180.0f;
        p->rot[2] = fadeRand(360) * 3.1415927f / 180.0f;
        w->life[i] = fadeRand(6) + 15;
    }
}

void oniFightHahen2PieceFall(Obj *o, PolFrame *p, s32 i) {
    Hahen2Work *w = HAHEN2(o);

    if (p != NULL) {
        p->pos[0] += i - 1;
        p->pos[1] -= 1.0;
        if (w->life[i]-- <= 0) {
            frmDestroy(&p->f);
        }
    }
}

/* "hahen2"
 * Matched 2026-10-11; it was parked for the spill slot of eneNewPol's address (the game's 0x24),
 * which pad[5] gives. */
s32 oniFightHahen2Proc(Obj *o, s16 msg, void *arg) {
    Hahen2Work *w = HAHEN2(o);
    s32 pad[5];

    switch (msg) {
        case 2:
            o->name = "hahen2";
            eneSetup(o, arg);
            w->pieces[0] = eneNewPol(o, oniFightHahenDl);
            w->pieces[1] = eneNewPol(o, oniFightHahenDl);
            w->pieces[2] = eneNewPol(o, oniFightHahenDl);
            w->pieces[3] = eneNewPol(o, oniFightHahenDl);
            oniFightHahen2PieceInit(o, w->pieces[0], 0);
            oniFightHahen2PieceInit(o, w->pieces[1], 1);
            oniFightHahen2PieceInit(o, w->pieces[2], 2);
            oniFightHahen2PieceInit(o, w->pieces[3], 3);
            break;
        case 1:
            if (w->pieces[0] != NULL) {
                frmDestroy(&w->pieces[0]->f);
            }
            if (w->pieces[1] != NULL) {
                frmDestroy(&w->pieces[1]->f);
            }
            if (w->pieces[2] != NULL) {
                frmDestroy(&w->pieces[2]->f);
            }
            if (w->pieces[3] != NULL) {
                frmDestroy(&w->pieces[3]->f);
            }
            break;
        case 0:
            oniFightHahen2PieceFall(o, w->pieces[0], 0);
            oniFightHahen2PieceFall(o, w->pieces[1], 1);
            oniFightHahen2PieceFall(o, w->pieces[2], 2);
            oniFightHahen2PieceFall(o, w->pieces[3], 3);
            if (w->pieces[0] == NULL && w->pieces[1] == NULL && w->pieces[2] == NULL &&
                w->pieces[3] == NULL) {
                return -1;
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
