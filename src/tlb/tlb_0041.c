#include "tlb.h"

/* The kabuki fight: the director ("main", which makes the others and runs the scene), the kabuki,
 * the sea monk ("umimonk"), the doors, the fog ("kiri"), the shadow ("kage"), the water, the
 * attack points ("atkpoint") and the pillars ("hasira"). The director answers message 10 with
 * argument MAIN_ID with itself, so the others find it through the enemy manager. */

#define MAIN_ID 0x53BD1

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
    /* 0x34 */ Obj *kabuki; /* kind 0x29 */
    /* 0x38 */ Obj *monk; /* kind 0x2A */
    /* 0x3C */ Obj *door1; /* kind 0x2B */
    /* 0x40 */ Obj *door2; /* kind 0x2B */
    /* 0x44 */ Obj *fx[2]; /* kind 0x100 */
    /* 0x4C */ Obj *water; /* kind 0x2E */
    /* 0x50 */ Obj *gauge; /* the pip gauge */
    /* 0x54 */ s32 timer;
    /* 0x58 */ s32 unused58;
    /* 0x5C */ s32 count; /* the gauge's pips */
} MainWork;

/* The kabuki's work. */
typedef struct KabukiWork {
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
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ Obj *kage; /* kind 0x2D */
    /* 0x40 */ ACollFrame *atk;
    /* 0x44 */ ACollFrame *dmg;
    /* 0x48 */ s32 timer;
    /* 0x4C */ s32 count;
    /* 0x50 */ s32 sndTimer;
    /* 0x54 */ u8 pad54[4];
    /* 0x58 */ Obj *pompom; /* the ending's smoke */
    /* 0x5C */ s32 wait;
    /* 0x60 */ u32 steps; /* the attack pattern's step */
    /* 0x64 */ s32 retarget; /* steps to the next random target */
    /* 0x68 */ s32 target; /* the player it goes for */
    /* 0x6C */ u8 pad6C[4];
    /* 0x70 */ f32 gravity;
    /* 0x74 */ f32 jump; /* the leap's speed */
    /* 0x78 */ s32 hits;
    /* 0x7C */ s32 safe; /* after a hit: hits do nothing */
    /* 0x80 */ s32 flash; /* after a hit: flashing */
    /* 0x84 */ u8 alpha; /* the red tint's */
    /* 0x85 */ u8 side; /* toggled when a turn ends */
    /* 0x86 */ s8 turn; /* 1, -1: turning; 0: still */
    /* 0x87 */ u8 guard;
    /* 0x88 */ u8 atBack;
    /* 0x89 */ u8 step; /* the jump's step */
    /* 0x8A */ s8 dustTick;
} KabukiWork;

/* The sea monk's work. */
typedef struct MonkWork {
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
    /* 0x34 */ AnimeFrame *body;
    /* 0x38 */ ACollFrame *aniAtk;
    /* 0x3C */ s32 timer;
    /* 0x40 */ u8 pad40[4];
    /* 0x44 */ Obj *atk; /* the attack point it aims at */
    /* 0x48 */ u8 pad48[8];
    /* 0x50 */ s32 count; /* the attacks left */
    /* 0x54 */ u8 attack; /* which of the kabuki's turns follows */
} MonkWork;

/* A door's work. */
typedef struct DoorWork {
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
    /* 0x34 */ AnimeFrame *anim;
    /* 0x38 */ s32 open; /* the animation runs forward (open) or back */
} DoorWork;

/* The fog's work ("kiri", kind 0x2C): a puff the kabuki blows. */
typedef struct KiriWork {
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
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ ACollFrame *atk;
    /* 0x3C */ ACollFrame *dmg;
    /* 0x40 */ Anim anim; /* the sparkle's frames */
    /* 0x50 */ s32 alpha;
} KiriWork;

/* An attack point's work ("atkpoint"): where the sea monk aims. */
typedef struct AtkPointWork {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone; /* the course */
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
    /* 0x38 */ PolFrame *ring;
    /* 0x3C */ MPolFrame *shadow;
    /* 0x40 */ s32 target; /* the player it follows */
    /* 0x44 */ u8 pad44[4];
    /* 0x48 */ f32 phase; /* in degrees */
    /* 0x4C */ f32 unused4C; /* written 1.0f at its start, never read */
} AtkPointWork;

#define MAIN(obj) ((MainWork *) FX(obj))
#define KIRI(obj) ((KiriWork *) FX(obj))
#define ATKPOINT(obj) ((AtkPointWork *) FX(obj))
#define KABUKI(obj) ((KabukiWork *) FX(obj))
#define MONK(obj) ((MonkWork *) FX(obj))
#define DOOR(obj) ((DoorWork *) FX(obj))

/* The water's work. */
typedef struct WaterWork {
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
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ s32 scroll;
    /* 0x3C */ s32 step; /* the scroll's each frame */
    /* 0x40 */ Gfx *(*draw)(Gfx *, u16);
} WaterWork;

#define WATER(obj) ((WaterWork *) FX(obj))

void kabukiDoorSetOpen(Obj *o, s32 open);

/* acollAtkAddYZ and acollDmgAddYZ return their box (main declares them void). */
typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                    s32 a);

extern Frame *gFrmLayer3;
extern u8 kabukiWaterTex[];

/* A 32x32 RGBA16 texture loaded at tmem and drawn through tile rtile from (uls, ult): one
 * macro, its seven commands on one source line (as tlb_0053's). */
#define gDPLoadScrollBlock(pkt, timg, tmem, rtile, uls, ult)                                               \
    {                                                                                                     \
        gDPSetTextureImage(pkt, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, timg);                                    \
        gDPSetTile(pkt, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, tmem, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, \
                   5, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD);                              \
        gDPLoadSync(pkt);                                                                                 \
        gDPLoadBlock(pkt, G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES));            \
        gDPPipeSync(pkt);                                                                                 \
        gDPSetTile(pkt, G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, tmem, rtile, 0, G_TX_WRAP | G_TX_NOMIRROR, 5,      \
                   G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD);                                 \
        gDPSetTileSize(pkt, rtile, uls, ult, 31 << G_TEXTURE_IMAGE_FRAC, 31 << G_TEXTURE_IMAGE_FRAC);     \
    }
Gfx kabukiUnusedDl[] = {
    gsSPDisplayList(gDrawSetupLitOpaDl),
    gsDPSetRenderMode(G_RM_RA_ZB_OPA_SURF, G_RM_RA_ZB_OPA_SURF2),
    gsSPEndDisplayList(),
    gsDPNoOp(),
};

/* The water's display list: two layers of one texture scrolled against each other (as
 * tlb_0053's obj22ScrollTexB). */
Gfx *kabukiWaterDl(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 0x80, 0x80, 0x80, 0x60);
    gDPLoadScrollBlock(gfx++, kabukiWaterTex, 0, G_TX_RENDERTILE, scroll, scroll);
    gDPLoadScrollBlock(gfx++, kabukiWaterTex, 0x100, 1, scroll + 10, -scroll);
    gSPEndDisplayList(gfx++);
    return gfx;
}

void kabukiMainShowGauge(void);
void kabukiNextState();
void kabukiSetState3(Obj *o);
void kabukiStartEnding(Obj *o);
void kabukiUmimonkNextState();
void kabukiUmimonkSetState5(Obj *o);
void kabukiUmimonkSetState17(Obj *o);
void kabukiDoorSetRotY(Obj *o, f32 ry);

/* "main": makes the others, then runs the scene: the players walk in, the doors open, the
 * camera shows the kabuki and the sea monk, the fight; message 10 with MAIN_ID finds it.
 * The camera positions are int literals (a shared float 80.0f would sit in f20), the tilt goes
 * through a Player pointer, and the last loop has its own index and a pad in its block.
 * FAKE: the empty "if (w != NULL) {}" at the head of message 0 makes w's addiu first, as the
 * game schedules it (IDO_NOTES 344). */
s32 kabukiMainProc(Obj *o, s16 msg, void *arg) {
    MainWork *w = MAIN(o);
    Camera *cam = camAiGetCam();
    s32 i;
    s32 pad;
    Player *pl;

    switch (msg) {
        case 2:
            o->name = "main";
            eneSetup(o, arg);
            w->kabuki = eneMakeUnder(o, ENE_KABUKI, 1, 360.0f, 48.0f, -25.0f);
            w->monk = eneMakeUnder(o, ENE_KABUKI_UMIMONK, 1, 360.0f, -170.0f, -170.0f);
            w->door1 = eneMakeUnder(o, ENE_KABUKI_DOOR, 0, -288.0f, 48.0f, -166.0f);
            if (w->door1 != NULL) {
                kabukiDoorSetRotY(w->door1, 1.570796371f);
            }
            w->door2 = eneMakeUnder(o, ENE_KABUKI_DOOR, 0, 218.0f, 48.0f, -627.0f);
            if (w->door2 != NULL) {
                kabukiDoorSetRotY(w->door2, 0.0f);
            }
            w->water = eneMakeUnder(o, ENE_KABUKI_WATER, 0, 300.0f, 33.0f, -200.0f);
            w->gauge = tskCreate(o, pipGauge_proc, 0, NULL);
            eneCounterReward(3, 1, 740.0f, 49.0f, 0.0f, 1, 700.0f, 49.0f, 0.0f);
            gGameWork.scenarioRunning = 2;
            if (eneCounterGet(3) != 0) {
                w->state = 9;
            } else {
                w->state = 0;
            }
            eneCounterIncrement(3);
            PRINTF("@@@@@@@@@@@@@@@ COUNTER = %x\n @@@@@@@@@@@", eneCounterGet(3));
            w->count = 50;
            sndPlay(0xB4);
            break;
        case 10:
            if ((s32) arg == MAIN_ID) {
                return (s32) o;
            }
            break;
        case 0:
            if (w != NULL) {} /* FAKE */
            if (w->state < 9 && fadeStartPressed() != 0) {
                sndPlay(7);
                w->state = 9;
            }
            switch (w->state) {
                case 0:
                    if (fadeMoveLetterbox(45, 185, 1) != 0) {
                        w->state = 1;
                        w->timer = 80;
                        camSetSpeed(cam, 80.0f);
                        camMoveAt(cam, -227, 72, -133);
                        camMoveEye(cam, -166, 80, 35);
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                Vec3f p;

                                plSetAction(gGameWork.players[i], 0xDB);
                                p.x = 86.59999847f;
                                p.y = 48.0f;
                                p.z = 0.0f;
                                plDemoWalkTo(gGameWork.players[i], p, 0.7f);
                            }
                        }
                    }
                    break;
                case 1:
                    if (!camGetMoving(cam)) {
                        w->timer = 10;
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (w->timer-- <= 0) {
                        w->timer = 50;
                        sndPlay(0x2BB);
                        kabukiDoorSetOpen(w->door1, 1);
                        kabukiDoorSetOpen(w->door2, 1);
                        camSetSpeed(cam, 20.0f);
                        camMoveAt(cam, -264, 64, -155);
                        camMoveEye(cam, -193, 80, -42);
                        w->fx[0] = eneMakeUnder(o, ENE_TANU_WALL_FLAGS_3, 1, 7.0f, 48.0f, 0.0f);
                        w->fx[1] = eneMakeUnder(o, ENE_TANU_WALL_FLAGS_3, 1, 791.0f, 48.0f, 0.0f);
                        w->state = 3;
                    }
                    break;
                case 3:
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            plSetAction(gGameWork.players[i], 0xD7);
                            pl = PL(gGameWork.players[i]);
                            pl->tilt = pl->tilt - 0.3141592741f;
                            if (pl->tilt <= -3.141592741f) {
                                pl->tilt = -3.141592741f;
                            }
                        }
                    }
                    if (w->timer == 20) {
                        scRunStart2(17, 0, 0);
                        sndPlay(0x300);
                    }
                    if (w->timer-- <= 0) {
                        w->timer = 40;
                        w->state = 4;
                    }
                    break;
                case 4:
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            plSetAction(gGameWork.players[i], 0xD7);
                            pl = PL(gGameWork.players[i]);
                            pl->tilt = pl->tilt + 0.3141592741f;
                            if (pl->tilt >= 0.0f) {
                                pl->tilt = 0.0f;
                            }
                        }
                    }
                    if (!scIsRunning()) {
                        gGameWork.scenarioRunning = 2;
                        w->timer = 30;
                        camSetSpeed(cam, 30.0f);
                        camMoveAt(cam, 7, 83, -280);
                        camMoveEye(cam, -176, 80, -45);
                        w->state = 5;
                    }
                    gGameWork.scenarioRunning = 2;
                    break;
                case 5:
                    if (w->timer-- <= 0) {
                        w->timer = 140;
                        camSetSpeed(cam, 20.0f);
                        camMoveEye(cam, -14, 89, -249);
                        w->state = 6;
                    }
                    gGameWork.scenarioRunning = 2;
                    break;
                case 6:
                    if (w->timer == 120) {
                        scRunStart2(10, 0, 0);
                    }
                    if (w->timer-- <= 0) {
                        camSetSpeed(cam, 100.0f);
                        camMoveEye(cam, 43, 89, -239);
                        kabukiNextState(w->kabuki);
                        w->state = 7;
                    }
                    gGameWork.scenarioRunning = 2;
                    break;
                case 7:
                    if (!camGetMoving(cam)) {
                        w->timer = 100;
                        kabukiUmimonkNextState(w->monk);
                        w->state++;
                    }
                    gGameWork.scenarioRunning = 2;
                    break;
                case 8:
                    if (w->timer-- <= 0 && fadeMoveLetterbox(0, 240, 2)) {
                        w->state = 9;
                    }
                    gGameWork.scenarioRunning = 2;
                    break;
                case 9:
                    fadeSetLetterbox(320, 0, 0, 240);
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            Vec3f p;
                            s32 zone;

                            plSetAction(gGameWork.players[i], 0);
                            plAskPos(gGameWork.players[i], &p.x, &p.y, &p.z, &zone);
                            p.x = 86.59999847f;
                            plSetPos(gGameWork.players[i], zone, &p);
                            pl = PL(gGameWork.players[i]);
                            pl->tilt = 0.0f;
                        }
                    }
                    sndPlay(0x8300);
                    scRunEnd();
                    kabukiDoorSetOpen(w->door1, 1);
                    kabukiDoorSetOpen(w->door2, 1);
                    if (w->fx[0] == NULL) {
                        w->fx[0] = eneMakeUnder(o, ENE_TANU_WALL_FLAGS_3, 1, 7.0f, 48.0f, 0.0f);
                    }
                    if (w->fx[1] == NULL) {
                        w->fx[1] = eneMakeUnder(o, ENE_TANU_WALL_FLAGS_3, 1, 791.0f, 48.0f, 0.0f);
                    }
                    kabukiUmimonkSetState5(w->monk);
                    camAiSetHold(0);
                    w->state = 10;
                    kabukiSetState3(w->kabuki);
                    sndPlay(0x98);
                    kabukiMainShowGauge();
                    gGameWork.scenarioRunning = 1;
                    eneRumbleStopAll();
                    break;
                case 11:
                    gGameWork.scenarioRunning = 2;
                    camAiSetHold(1);
                    camSetAt(cam, -15, 64, -311);
                    camSetEye(cam, 137, 79, -265);
                    fadeSetLetterbox(320, 45, 0, 185);
                    sndPlay(9);
                    kabukiStartEnding(w->kabuki);
                    kabukiUmimonkSetState17(w->monk);
                    {
                        s32 j;
                        s32 pad2;

                        for (j = 0; j < 4; j++) {
                            if (plIsInPlay(j)) {
                                Vec3f p;

                                plSetAction(gGameWork.players[j], 0xD7);
                                p.x = 10.0f;
                                p.y = 48.0f;
                                p.z = 0.0f;
                                plSetPos(gGameWork.players[j], 1, &p);
                            }
                        }
                    }
                    w->timer = 40;
                    w->state = 12;
                    break;
                case 15:
                    if (w->timer-- <= 0 && fadeMoveLetterbox(120, 120, 1)) {
                        eneRumbleStopAll();
                        eneCounterSetFlag(3);
                        dmenuQueueDemo(11);
                    }
                    break;
            }
            break;
        case 1:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

s32 kabukiMainGetCount(void) {
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) MAIN_ID);

    if (o != NULL) {
        return MAIN(o)->count;
    }
    return -1;
}

void kabukiMainShowGauge(void) {
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) MAIN_ID);

    MainWork *w;

    if (o != NULL) {
        w = MAIN(o);
        pipGaugeOn(w->gauge);
        pipGaugeSetTarget(w->gauge, w->count);
    }
}

s32 kabukiMainTakeCount(s32 n) {
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

void kabukiMainStartEnding(Obj *o) {
    MAIN(o)->state = 11;
}

void kabukiMainFinish(Obj *o) {
    sndPlay(10);
    MAIN(o)->state = 15;
}

Obj *kabukiMainGetKabuki(Obj *o) {
    return MAIN(o)->kabuki;
}

Obj *kabukiMainGetUmimonk(Obj *o) {
    return MAIN(o)->monk;
}

Obj *kabukiMainShutDoor2(Obj *o) {
    MainWork *w = MAIN(o);

    tskDestroy(w->fx[1]);
    kabukiDoorSetOpen(w->door2, 0);
    return w->door2;
}

extern u8 kabukiTurnReverseAnim[];
extern u8 kabukiJumpStartAnim[];

void kabukiTurnReverse(Obj *o) {
    KabukiWork *w = KABUKI(o);

    if (w->side == 0) {
        w->body->anim = kabukiTurnReverseAnim;
        w->body->frame = polAnimeLastFrame(w->body);
        w->turn = -1;
    }
}

void kabukiTurnForward(Obj *o) {
    KabukiWork *w = KABUKI(o);

    if (w->side == 1) {
        w->body->anim = kabukiTurnReverseAnim;
        w->body->frame = 0.0f;
        w->turn = 1;
    }
}

s32 kabukiTurnStep(Obj *o, f32 step) {
    KabukiWork *w = KABUKI(o);

    if (w->turn != 0) {
        step = fabsf(step);
        if (eneAnimPlayOnce(o, w->body, w->turn * step) != 0) {
            w->turn = 0;
            w->side ^= 1;
            return TRUE;
        }
        return FALSE;
    }
    return TRUE;
}

s32 kabukiTurnToPlayer(Obj *o, s32 pl, f32 step) {
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
    if (KABUKI(o)->x <= px) {
        way = 8.0f;
    } else {
        way = 2.0f;
    }
    a = eneCourseAngle(o, way);
    if (eneTurnToward(o, a, step) != 0) {
        return TRUE;
    }
    return FALSE;
}

void kabukiItexSparkle(f32 x, f32 y, f32 z, f32 size) {
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

void kabukiSparkleAt(Obj *o, f32 x, f32 y, f32 z, f32 size) {
    s32 pad;
    AnimeFrame *b = KABUKI(o)->body;

    mathVecRotateY(KABUKI(o)->rot[1], &x, &y, &z);
    kabukiItexSparkle(b->pos[0] + x, b->pos[1], b->pos[2] + z, size);
}

s32 kabukiSparkleTick(Obj *o) {
    if (KABUKI(o)->dustTick-- <= 0) {
        KABUKI(o)->dustTick = 3;
        kabukiSparkleAt(o, 10.0f, 0.0f, 10.0f, 0.1f);
        kabukiSparkleAt(o, -13.0f, 0.0f, -10.0f, 0.1f);
        return TRUE;
    }
    return FALSE;
}

void kabukiJumpStart(Obj *o) {
    AnimeFrame *b = KABUKI(o)->body;

    KABUKI(o)->side = 0;
    KABUKI(o)->step = 0;
    b->anim = kabukiJumpStartAnim;
    b->frame = 0.0f;
}

s32 kabukiJumpStep(Obj *o, f32 step) {
    AnimeFrame *b = KABUKI(o)->body;
    KabukiWork *w;

    step = fabsf(step);
    switch (KABUKI(o)->step) {
        case 0:
            eneAnimPlayOnce(o, b, 0.7f);
            w = KABUKI(o);
            if (b->frame > 7.0) {
                b->frame = 7.0f;
                w->vel[1] = step;
                kabukiSparkleAt(o, 0.0f, 0.0f, 8.0f, 0.1f);
                kabukiSparkleAt(o, 2.0f, 0.0f, -11.0f, 0.1f);
                w->step++;
            }
            break;
        case 1:
            eneAnimPlayOnce(o, b, 0.3f);
            w = KABUKI(o);
            if (b->frame > 9.0) {
                b->frame = 9.0f;
            }
            if (w->vel[1] < 0.0f) {
                b->frame = 9.0f;
                w->step++;
            }
            break;
        case 2:
            eneAnimPlayOnce(o, b, 0.3f);
            w = KABUKI(o);
            if (b->frame > 11.0) {
                b->frame = 11.0f;
            }
            if (w->vel[1] == 0.0f) {
                SoundLocation2(0x2FB, gGameWork.view, b->pos[0], b->pos[1], b->pos[2], 0.0f, 0.0f,
                               gGameWork.view->fovy);
                b->frame = 11.0f;
                kabukiSparkleAt(o, 0.0f, 0.0f, 8.0f, 0.1f);
                kabukiSparkleAt(o, 2.0f, 0.0f, -11.0f, 0.1f);
                w->step++;
            }
            break;
        case 3:
            if (eneAnimPlayOnce(o, b, 0.7f) != 0) {
                return TRUE;
            }
            break;
    }
    return FALSE;
}

void kabukiStop(Obj *o) {
    KABUKI(o)->vel[0] = 0.0f;
    KABUKI(o)->vel[1] = 0.0f;
    KABUKI(o)->vel[2] = 0.0f;
}

void kabukiNextPattern(Obj *o) {
    KabukiWork *w = KABUKI(o);

    if (KABUKI(o)->wait-- <= 0) {
        switch (w->steps) {
            case 0:
                w->state = 7;
                w->wait = 2;
                w->steps++;
                break;
            case 1:
                w->state = 23;
                w->wait = 2;
                w->steps++;
                break;
            case 2:
                w->state = 10;
                w->wait = 30;
                w->steps++;
                break;
            case 3:
                w->state = 32;
                w->wait = 1;
                w->steps++;
                break;
            case 4:
                w->state = 7;
                w->wait = 10;
                w->steps++;
                break;
            case 5:
                w->state = 23;
                w->wait = 1;
                w->steps++;
                break;
            case 6:
                w->state = 10;
                w->wait = 1;
                w->steps++;
                break;
            case 7:
                w->state = 32;
                w->wait = 1;
                w->steps = 0;
                break;
        }
    }
}

void kabukiTakeHit(Obj *o) {
    KabukiWork *w = KABUKI(o);

    eneHitSoundHeavy(w->dmg);
    if (kabukiMainTakeCount(1) == 0) {
        w->state = 39;
        return;
    }
    if (w->hits < 4) {
        w->flash = 30;
        w->safe = 30;
        w->hits++;
    } else {
        w->flash = 30;
        w->safe = 30;
        w->hits = 0;
        kabukiStop(o);
        w->state = 39;
    }
}

void kabukiGuardHit(Obj *o) {
    KABUKI(o)->state = 6;
    KABUKI(o)->safe = 5;
    sndPlay(0x2EC);
}

void kabukiSoundTick(Obj *o) {
    if (KABUKI(o)->sndTimer-- <= 0) {
        KABUKI(o)->sndTimer = 6;
        sndPlay(0x513);
    }
}

/* What the work's pointers at 0x34 and 0x38 point to (kabukiMainProc passes them); its byte at
 * 0x20 is counted up here. */
typedef struct WorkPart {
    /* 0x00 */ Obj obj;
    /* 0x20 */ u8 state;
} WorkPart;

void kabukiNextState(WorkPart *p) {
    p->state++;
}

void kabukiSetState3(Obj *o) {
    KABUKI(o)->state = 3;
}

void kabukiSetState30(Obj *o) {
    kabukiTurnForward(o);
    KABUKI(o)->state = 30;
}

void kabukiSetState46(Obj *o) {
    kabukiTurnForward(o);
    KABUKI(o)->state = 46;
}

void kabukiStartEnding(Obj *o) {
    kabukiTurnForward(o);
    KABUKI(o)->state = 58;
}

extern u8 kabukiBodyAnim[];
extern u8 kabukiRide[];
extern u8 kabukiState7Anim[];
extern u8 kabukiState11Anim[];
extern u8 kabukiState15Anim[];
extern u8 kabukiState26Anim[];
extern u8 kabukiState39Anim[];
extern u8 kabukiState43Anim[];
extern u8 kabukiState44Anim[];
extern u8 kabukiState47Anim[];

void kabukiUmimonkStartAttack0(Obj *o);
void kabukiUmimonkSetState19(Obj *o);
void kabukiUmimonkStartAttack1(Obj *o);
void kabukiUmimonkAnimAdvance(Obj *o, f32 step);
Obj *kabukiKiriMake(Obj *o);

/* A push (message 6's argument): plmove.c's PlPlace (zone, place, side, height). */
typedef struct KabukiPush {
    /* 0x00 */ s16 zone;
    /* 0x02 */ u8 pad02[2];
    /* 0x04 */ f32 x;
    /* 0x08 */ f32 side;
    /* 0x0C */ f32 y;
} KabukiPush;

/* The kabuki's render modes: opaque in both cycles, and a pass-through first cycle for the red
 * tint. */
#define KABUKI_RM_OPA(clk) \
    AA_EN | Z_CMP | Z_UPD | ALPHA_CVG_SEL | GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)

#define KABUKI_SOUND(id)                                                                                   \
    SoundLocation2(id, gGameWork.view, a->pos[0], a->pos[1], a->pos[2], 0.0f, 0.0f, gGameWork.view->fovy)

/* "kabuki"
 * Was parked: 1751 differing lines (2811), this C 2812 instructions, mix distance 3: every
 * state is written out and the code is the game's but for registers and stack slots. The frame
 * (0x140) is the game's with the FAKE pad[8], but the game's block locals sit 0x1C higher (the
 * hit's way 0x110 and side 0x108, state 5's place 0x104..0xF8 with its distance and flag kept in
 * memory at 0xF8/0xF0) and from state 1 on every t-register is one behind (the game builds
 * kabukiBodyAnim in a1 for state 1's store). Measured: as first written 1777 (frame 0x120); the jump
 * table's last entry (case 67) 1775; the pads search's best, a 1-word pad, 1731 (frame 0x128);
 * pad[8] 1751 (frame right). Its .s also carries "!!!!! LAST LAST !!!!!\n", a compiled-out
 * print this C does not have yet (put it back before unparking; likely state 58 or 42).
 * Sweep 2 2026-10-11: pad[8] split into one word at the top and pad2[7] after case 66's i and
 * n (the last block): frame 0x140, state 5's slots 0xF0-0x104 the game's, 1751 to 1727
 * (pad2 first in that block: 1727 too); the hit's side is still 0x10C (game 0x108).
 * 2026-10-12 (check's real): 752 at length +1, mix 3. "!!!!! LAST LAST !!!!!" put back as a
 * PRINTF where state 42 finds the last one (no change to the code: 752). The length: state
 * 46's sound passes its zeros as 0, 0 (one shared mtc1 zero,f0 as the game has; KABUKI_SOUND's
 * 0.0f, 0.0f gave two): length 0, mix 2, real 1145 (the t and f rotation after it moved; the
 * instruction shapes there are now the game's). 0.0, 0.0 gives the same as 0, 0. The game
 * builds kabukiBodyAnim in a1 at all five of its sites (here t2 in state 1, v0 in state 4: the
 * t-registers one behind from there). Misses (1145 each, no change to the code): state 4's
 * test as "a->anim != kabukiBodyAnim"; case 2's call through a (Obj *, void *) pointer with
 * kabukiBodyAnim uncast; state 1's store through a void ** (1123); a function-scope
 * "u32 idle = (u32) kabukiBodyAnim" for state 1 (still t2; 1173).
 * The frame, read slot by slot against the game's: the hit's block is way, place, side (way
 * 0x110, side 0x108: place has a slot of its own though it lives in f0), so the FAKE top pad
 * went; state 6 reads w->x twice, no x local (the game has no slot for one: the blocks from
 * state 13 on moved to the game's); pad2 left case 66 and became pad[9] in state 21's block
 * (case 64's block 4 low of the game's still: pad[10] grows the frame to 0x148 and puts an
 * alignment word among the temps; so does an f64 in it; hoist1 holds a slot at the top).
 * Every local and temp slot is now the game's but for cases 64-66's (4 high). With
 * KABUKI_SOUND in state 46: 729, length +1; with its 0, 0: 1123, length 0. Kept at 729 (the
 * zeros as KABUKI_SOUND), the 0, 0 to be put back when the registers before it are the
 * game's.
 * 2026-10-12 (second try): the first real difference is case 2's eneNewAnime call (the game
 * schedules "lui t9; lui a1; addiu t9; addiu a1", this C both t9 halves first). A
 * function-scope "u32 idle = (u32) kabukiBodyAnim;" passed there: frame 0x148, the schedule the
 * same, 785 (not kept). check's "jump tables: game 2, this C 1" is not the layout: this C's
 * .rodata has the message table (+300), 0.4f (+328) and the state table (+332) in the game's
 * order; why check finds one table is not yet read.
 * 2026-10-12 (third try): THE STACK IS THE GAME'S (729 to 706). A call's result passed as another
 *   call's argument spends a front-end word below the block locals, one per type (probe:
 *   the int one is shared with the switches). The four
 *   eneTurnToward(o, eneCourseAngle(..), ..) (states 3x, 40, 41, 45) spent the float one: read
 *   into a function-scope "f32 ang" in hoist1's slot (eneFileAddr back inline: its int word is
 *   the switches'), the float word goes and state 21's pad[10] puts cases 64-66 at the game's
 *   0x60. Left: the registers (kabukiBodyAnim's a1, the t rotation), length +1. With the stack
 *   right, state 46's zeros as 0, 0 again: length 0, jump tables right, real 1097 (the first
 *   difference then case 2's eneNewAnime schedule, then state 1's a1); taken back to 706.
 * 2026-10-12 (fourth try): MATCH, from 706, real after each:
 * - kabukiBodyAnim as a pointer at all five sites (eneNewAnime through a void * parameter, the
 *   stores and state 4's test through a void * view of a->anim): IDO keeps it as one address
 *   constant, built in a1 at each site as the game, and case 2's schedule is the game's: 634
 *   (probe: all sites pointer-typed share one register; mixed
 *   spellings do not);
 * - state 3: gravity before guard (the game loads 0.4 early) 628;
 * - states 8, 27 and 62: "w->timer--; if (w->timer <= 0)" as "if (w->timer-- <= 0)" (the game
 *   tests the old value: bgtz v1) 511, 267, 98 (with the next two);
 * - state 15 (gravity before state) 261; state 21 (gravity before state 22) 258; state 39
 *   (gravity before side 256; vel[0] before vel[1] 252);
 * - state 46's zeros as 0, 0 (length 0) 443, then state 20's gravity = 0.0f (not 0) 197;
 *   state 58's pompom 25.0 spelled 25.00f (a queue temp in the game) 98 (2026-10-16: the
 *   int 25 is the same, object byte-identical; measured);
 * - state 64: "if (w->timer-- < -39)" 2; state 14's first wall: vel[0] before count: MATCH. */
s32 kabukiProc(Obj *o, s16 msg, void *arg) {
    KabukiWork *w = KABUKI(o);
    AnimeFrame *a;

    f32 ang; /* the course angle the turns aim at */

    switch (msg) {
        case 2:
            o->name = "kabuki";
            eneSetup(o, arg);
            /* kabukiBodyAnim passed and stored as a pointer here and at its four other sites (the
             * anim field is a void *): IDO then keeps it as one address constant and builds it
             * in a1 at every site, as the game does (IDO_NOTES 378). */
            w->body = eneNewAnime(o, kabukiBodyAnim);
            if (w->body == NULL) {
                return -1;
            }
            frmMove(&w->body->f, gFrmLayer2);
            w->ride = rideCreate(gRideTree1, (s32) o, &w->body->f, (RideData *) eneFileAddr(o, kabukiRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->body->f, 100, 100, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x78;
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->body->f, 80, 400, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 0x7A;
            a = w->body;
            w->gravity = 0.4f;
            w->rot[1] = eneCourseAngle(o, 2);
            w->body->anim = kabukiTurnReverseAnim;
            w->body->frame = polAnimeLastFrame(w->body);
            w->side = 0;
            w->turn = 0;
            eneSetModelOnCourse(o, &a->f);
            w->target = eneRandomPlayerIndex();
            w->wait = 40;
            w->steps = 0;
            w->alpha = 0;
            w->atBack = 0;
            w->kage = eneMakeEx(o, ENE_KABUKI_KAGE, 0, a->pos[0], 48.0f, a->pos[2], 2, NULL);
            break;
        case 1:
            if (w->body != NULL) {
                frmDestroy(&w->body->f);
            }
            if (w->ride != NULL) {
                frmDestroy(&w->ride->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            break;
        case 6:
            ((KabukiPush *) arg)->x += w->vel[0];
            ((KabukiPush *) arg)->y += w->vel[1];
            break;
        case 3:
            if (w->flash & 2) {
                w->body->dl = gGfxDlHead;
                gSPDisplayList(gGfxDlHead++, gDrawSetupEnvTintTexDl);
                gDPSetEnvColor(gGfxDlHead++, 0xD0, 0xD0, 0xD0, 0xA0);
                gDPSetRenderMode(gGfxDlHead++, KABUKI_RM_OPA(1), KABUKI_RM_OPA(2));
                gSPEndDisplayList(gGfxDlHead++);
            } else {
                w->body->dl = gGfxDlHead;
                gSPDisplayList(gGfxDlHead++, gDrawSetupEnvTintTexShadeDl);
                gDPSetEnvColor(gGfxDlHead++, 0xFF, 0, 0, w->alpha);
                gDPSetRenderMode(gGfxDlHead++, G_RM_PASS, KABUKI_RM_OPA(2));
                gSPEndDisplayList(gGfxDlHead++);
            }
            break;
        case 0:
            a = w->body;
            if (w->safe-- <= 0) {
                w->safe = 0;
            }
            if (w->flash-- <= 0) {
                w->flash = 0;
            }
            if (w->atk != NULL && acollAtkWasHit(w->atk) == 1) {
                if ((Obj *) w->atk->hitPower != NULL) {
                    tskSendMsg((Obj *) w->atk->hitPower, 7, o);
                }
            }
            if (acollDmgWasHit(w->dmg) == 1) {
                if (w->state < 47) {
                    if (w->dmg->hitKind == 0x78) {
                        w->hits = 0;
                        w->safe = 0;
                        if (w->dmg->kind == 0x7B) {
                            kabukiStop(o);
                            kabukiMainTakeCount(7);
                            w->state = 52;
                        } else {
                            kabukiStop(o);
                            w->state = 47;
                        }
                    } else if (w->safe == 0 && w->atBack == 0 && w->state < 39) {
                        if (w->guard == 0) {
                            kabukiTakeHit(o);
                        } else {
                            s32 way = eneFaceWay(o);
                            f32 place;
                            f32 side;

                            place = vramNearestPlaceXZ(1, w->dmg->hitPos[0], w->dmg->hitPos[2], &side);

                            if (way == 8) {
                                if (w->x < place) {
                                    kabukiGuardHit(o);
                                    w->vel[0] = -1.0f;
                                } else {
                                    kabukiTakeHit(o);
                                }
                            } else if (way == 2) {
                                if (place < w->x) {
                                    kabukiGuardHit(o);
                                    w->vel[0] = 1.0f;
                                } else {
                                    kabukiTakeHit(o);
                                }
                            } else {
                                kabukiTakeHit(o);
                            }
                        }
                    }
                }
            }
            switch (w->state) {
                case 0:
                    kabukiTurnReverse(o);
                    break;
                case 1:
                    w->guard = 1;
                    if (kabukiTurnStep(o, 0.2f) != 0) {
                        a->anim = kabukiBodyAnim;
                        a->frame = 0.0f;
                        w->state = 2;
                    }
                    break;
                case 2:
                    eneAnimLoop(o, a, 0.5f);
                    break;
                case 3:
                    w->dmg->kind = 0x7A;
                    w->gravity = 0.4f;
                    w->guard = 1;
                    kabukiTurnReverse(o);
                    w->state = 4;
                    w->z = 0.0f;
                case 4:
                    if (!plIsInPlay(w->target)) {
                        w->target = eneRandomPlayerIndex();
                    }
                    if (kabukiTurnToPlayer(o, w->target, 0.104719758f) == 0) {
                        kabukiSparkleTick(o);
                    }
                    w->guard = 1;
                    if (kabukiTurnStep(o, 1.0f) != 0) {
                        if (a->anim != kabukiBodyAnim) {
                            a->anim = kabukiBodyAnim;
                            a->frame = 0.0f;
                            w->timer = 0;
                        }
                        w->state = 5;
                    }
                    break;
                case 5: {
                    f32 px;
                    f32 py;
                    f32 pz;
                    f32 d;
                    s32 zone;
                    s32 moved = 0;

                    if (w->retarget-- <= 0) {
                        w->retarget = 100;
                        w->target = eneRandomPlayerIndex();
                    }
                    w->guard = 1;
                    eneAnimLoop(o, a, 0.5f);
                    if (!plIsInPlay(w->target)) {
                        w->target = eneRandomPlayerIndex();
                    }
                    plAskPos(gGameWork.players[w->target], &px, &py, &pz, &zone);
                    d = fabsf(px - w->x);
                    if (kabukiTurnToPlayer(o, w->target, 0.104719758f) == 0) {
                        moved = 1;
                        kabukiSparkleTick(o);
                    }
                    w->vel[0] = 0.0f;
                    if (a->frame > 6.0f && d > 50.0f) {
                        if (w->x <= px) {
                            w->vel[0] = 1.0f;
                        } else {
                            w->vel[0] = -1.0f;
                        }
                        if (moved == 0) {
                            kabukiSparkleTick(o);
                        }
                    }
                    kabukiNextPattern(o);
                    if (d >= 100.0f) {
                        w->guard = 0;
                        w->state = 17;
                    }
                    break;
                }
                case 6: {
                    a->anim = kabukiBodyAnim;
                    a->frame = 0.0f;
                    w->guard = 1;
                    w->side = 1;
                    if (w->x < 35.0f || w->x > 760.0f) {
                        w->vel[0] = 0.0f;
                    }
                    if (w->vel[0] != 0.0f) {
                        kabukiSparkleTick(o);
                    }
                    if (w->safe != 0) {
                        w->state = 3;
                    }
                    break;
                }
                case 7: {
                    s32 way;

                    kabukiStop(o);
                    a->anim = kabukiState7Anim;
                    a->frame = 0.0f;
                    w->timer = 200;
                    w->count = 0;
                    way = eneFaceWay(o);
                    if (way == 8) {
                        w->vel[0] = 1.0f;
                    } else if (way == 2) {
                        w->vel[0] = -1.0f;
                    } else {
                        w->vel[0] = 0.0f;
                    }
                    w->state = 8;
                }
                case 8:
                    if (w->x < 80.0f) {
                        w->x = 80.0f;
                        kabukiStop(o);
                    }
                    if (w->x > 720.0f) {
                        w->x = 720.0f;
                        kabukiStop(o);
                    }
                    eneAnimPlayOnce(o, a, 0.5f);
                    if (a->frame >= 19.0f) {
                        a->frame = 19.0f;
                    }
                    if (a->frame >= 17.0f) {
                        w->guard = 0;
                        if (w->count-- <= 0) {
                            w->count = 2;
                            kabukiKiriMake(o);
                        }
                    }
                    if (w->timer == 166) {
                        KABUKI_SOUND(0x2D3);
                    }
                    if (w->timer-- <= 0) {
                        w->timer = 30;
                        kabukiStop(o);
                        w->state = 9;
                    }
                    if (eneNearestPlayerDist(o) > 200.0f) {
                        w->side = 0;
                        w->guard = 0;
                        kabukiStop(o);
                        w->state = 17;
                    }
                    break;
                case 9:
                    if (w->timer-- <= 0) {
                        w->guard = 1;
                        if (eneAnimPlayOnce(o, a, 0.2f) != 0) {
                            w->state = 3;
                        }
                    }
                    break;
                case 10:
                    kabukiStop(o);
                    kabukiTurnForward(o);
                    w->count = 3;
                    w->state = 11;
                case 11:
                    if (kabukiTurnStep(o, 1.0f) != 0) {
                        w->guard = 0;
                        a->anim = kabukiState11Anim;
                        a->frame = 0.0f;
                        sndPlay(0x24C);
                        w->state = 12;
                    }
                    break;
                case 12:
                    eneAnimPlayOnce(o, a, 0.8f);
                    if (a->frame >= 21.0f) {
                        f32 px;
                        f32 py;
                        f32 pz;
                        s32 zone;

                        a->frame = 21.0f;
                        w->gravity = 0.2f;
                        w->vel[1] = 3.0f;
                        if (!plIsInPlay(w->target)) {
                            w->target = eneRandomPlayerIndex();
                        }
                        plAskPos(gGameWork.players[w->target], &px, &py, &pz, &zone);
                        if (w->x <= px) {
                            w->jump = 2.0f;
                        } else {
                            w->jump = -2.0f;
                        }
                        w->vel[0] = w->jump;
                        KABUKI_SOUND(0x255);
                        w->state = 13;
                    }
                    break;
                case 13:
                    if (w->x <= 55.0f) {
                        w->x = 55.0f;
                        w->vel[0] = 0.0f;
                    }
                    if (w->x >= 750.0f) {
                        w->x = 750.0f;
                        w->vel[0] = 0.0f;
                    }
                    eneAnimPlayOnce(o, a, 0.3f);
                    if (a->frame >= 23.0f) {
                        a->frame = 23.0f;
                    }
                    if (w->vel[1] < 0.0f) {
                        a->frame = 23.0f;
                        w->state = 14;
                    }
                    break;
                case 14:
                    if (w->x <= 55.0f) {
                        w->x = 55.0f;
                        w->vel[0] = 0.0f;
                        w->count = 0;
                    }
                    if (w->x >= 750.0f) {
                        w->x = 750.0f;
                        w->count = 0;
                        w->vel[0] = 0.0f;
                    }
                    eneAnimPlayOnce(o, a, 0.3f);
                    if (a->frame >= 25.0f) {
                        a->frame = 25.0f;
                    }
                    if (w->vel[1] == 0.0f) {
                        w->vel[0] = 0.0f;
                        a->frame = 25.0f;
                        if (eneNearestPlayerDist(o) > 200.0f) {
                            w->side = 0;
                            w->guard = 0;
                            w->state = 17;
                        } else {
                            w->state = 15;
                        }
                    }
                    break;
                case 15:
                    if (eneAnimPlayOnce(o, a, 0.5f) != 0) {
                        if (w->count-- <= 0) {
                            a->anim = kabukiState15Anim;
                            a->frame = 0.0f;
                            w->gravity = 0.4f;
                            w->state = 16;
                        } else {
                            w->gravity = 0.2f;
                            w->vel[1] = 3.0f;
                            w->vel[0] = w->jump;
                            a->frame = 21.0f;
                            soundIncamCam(0x255, a->pos[0], a->pos[1], a->pos[2]);
                            w->state = 13;
                        }
                    }
                    break;
                case 16:
                    if (eneAnimPlayOnce(o, a, 0.3f) != 0) {
                        w->state = 3;
                    }
                    break;
                case 17:
                    kabukiStop(o);
                    w->side = 0;
                    kabukiJumpStart(o);
                    KABUKI_SOUND(0x2FD);
                    w->state = 20;
                    break;
                case 18:
                    kabukiTurnForward(o);
                    w->state = 19;
                case 19:
                    if (!plIsInPlay(w->target)) {
                        w->target = eneRandomPlayerIndex();
                    }
                    kabukiTurnToPlayer(o, w->target, 0.104719758f);
                    if (kabukiTurnStep(o, 1.0f) != 0) {
                        w->guard = 0;
                        kabukiJumpStart(o);
                        KABUKI_SOUND(0x2FD);
                        w->state = 20;
                    }
                    break;
                case 20:
                    if (!plIsInPlay(w->target)) {
                        w->target = eneRandomPlayerIndex();
                    }
                    kabukiTurnToPlayer(o, w->target, 0.104719758f);
                    kabukiJumpStep(o, 8.0f);
                    if (w->step == 2) {
                        f32 px;
                        f32 py;
                        f32 pz;
                        s32 zone;

                        plAskPos(gGameWork.players[w->target], &px, &py, &pz, &zone);
                        if (w->x <= px) {
                            w->vel[0] = 8.0f;
                        } else {
                            w->vel[0] = -8.0f;
                        }
                        w->gravity = 0.0f;
                        w->vel[1] = 0.0f;
                        w->state = 21;
                    }
                    break;
                case 21: {
                    f32 px;
                    f32 py;
                    f32 pz;
                    s32 zone;
                    s32 pad[10]; /* FAKE: words of the game's frame */

                    if (!plIsInPlay(w->target)) {
                        w->target = eneRandomPlayerIndex();
                    }
                    kabukiTurnToPlayer(o, w->target, 0.104719758f);
                    plAskPos(gGameWork.players[w->target], &px, &py, &pz, &zone);
                    if (fabsf(px - w->x) <= 60.0f) {
                        w->vel[0] = 0.0f;
                        w->vel[1] = 0.0f;
                        w->gravity = 0.4f;
                        w->state = 22;
                    }
                    break;
                }
                case 22:
                    if (!plIsInPlay(w->target)) {
                        w->target = eneRandomPlayerIndex();
                    }
                    kabukiTurnToPlayer(o, w->target, 0.104719758f);
                    if (kabukiJumpStep(o, 8.0f) != 0) {
                        w->state = 3;
                    }
                    break;
                case 23:
                case 32:
                    kabukiStop(o);
                    kabukiTurnForward(o);
                    w->state++;
                case 24:
                case 33:
                    if (kabukiTurnStep(o, 1.0f) != 0) {
                        w->guard = 0;
                        kabukiJumpStart(o);
                        KABUKI_SOUND(0x2FD);
                        w->state++;
                    }
                    break;
                case 25:
                case 34:
                    ang = eneCourseAngle(o, 4);
                    eneTurnToward(o, ang, 0.104719758f);
                    if (w->step == 1 || w->step == 2) {
                        if (w->z >= -30.0f) {
                            w->vel[2] = -3.0f;
                        } else {
                            w->vel[2] = 0.0f;
                        }
                        if (w->vel[0] == 0.0f) {
                            if (w->x < 400.0f) {
                                w->vel[0] = 4.0f;
                            } else {
                                w->vel[0] = -4.0f;
                            }
                        }
                    } else {
                        w->atBack = 1;
                        kabukiStop(o);
                    }
                    if (kabukiJumpStep(o, 6.5f) != 0) {
                        w->timer = 10;
                        w->state++;
                    }
                    break;
                case 26:
                    if (w->timer-- <= 0) {
                        a->anim = kabukiState26Anim;
                        a->frame = 0.0f;
                        w->timer = 100;
                        w->state = 27;
                    }
                    break;
                case 27:
                    if (eneAnimPlayOnce(o, a, 0.3f) != 0) {
                        if (w->timer == 100) {
                            kabukiUmimonkStartAttack0(kabukiMainGetUmimonk(o->parent));
                        }
                        if (w->timer-- <= 0) {
                            kabukiTurnReverse(o);
                            kabukiSparkleAt(o, 10.0f, 0.0f, 10.0f, 0.1f);
                            kabukiSparkleAt(o, -13.0f, 0.0f, -10.0f, 0.1f);
                            w->state = 28;
                        }
                    }
                    break;
                case 28:
                    if (kabukiTurnStep(o, 0.5f) != 0) {
                        a->anim = kabukiBodyAnim;
                        a->frame = 0.0f;
                        w->state = 29;
                    }
                    break;
                case 29:
                    eneAnimLoop(o, a, 0.5f);
                    break;
                case 30:
                    if (kabukiTurnStep(o, 1.0f) != 0) {
                        kabukiJumpStart(o);
                        KABUKI_SOUND(0x2FD);
                        w->state = 31;
                    }
                    break;
                case 31:
                    w->atBack = 0;
                    w->guard = 0;
                    if (w->step == 1 || w->step == 2) {
                        if (w->z >= 0.0f) {
                            w->vel[2] = 0.0f;
                            w->z = 0.0f;
                        } else {
                            w->vel[2] = 3.0f;
                        }
                        if (w->vel[0] == 0.0f) {
                            if (w->x < 400.0f) {
                                w->vel[0] = 4.0f;
                            } else {
                                w->vel[0] = -4.0f;
                            }
                        }
                    } else {
                        kabukiStop(o);
                    }
                    if (kabukiJumpStep(o, 6.5f) != 0) {
                        w->state = 3;
                    }
                    break;
                case 35:
                    if (w->timer-- <= 0) {
                        a->anim = kabukiState26Anim;
                        a->frame = 0.0f;
                        w->timer = 80;
                        w->count = 3;
                        w->state = 36;
                    }
                    break;
                case 36:
                    if (eneAnimPlayOnce(o, a, 0.8f) != 0) {
                        eneMakeEx(o, ENE_KABUKI_HASIRA, 1, w->x, 200.0f, 0.0f, 2, (u32 (*)(void *, void *, void *)) 1);
                        eneMakeEx(o, ENE_KABUKI_HASIRA, 1, w->x, 200.0f, 0.0f, 2, (u32 (*)(void *, void *, void *)) 2);
                        eneMakeEx(o, ENE_KABUKI_HASIRA, 1, w->x, 200.0f, 0.0f, 2, (u32 (*)(void *, void *, void *)) 3);
                        eneMakeEx(o, ENE_KABUKI_HASIRA, 1, w->x, 200.0f, 0.0f, 2, (u32 (*)(void *, void *, void *)) 4);
                        eneMakeEx(o, ENE_KABUKI_HASIRA, 1, w->x, 200.0f, 0.0f, 2, (u32 (*)(void *, void *, void *)) 5);
                        w->state = 37;
                    }
                    break;
                case 37:
                    if (w->timer-- <= 0) {
                        w->timer = 45;
                        if (w->count-- == 0) {
                            w->state = 38;
                        } else {
                            a->anim = kabukiState26Anim;
                            a->frame = 0.0f;
                            w->state = 36;
                        }
                    }
                    break;
                case 38:
                    kabukiJumpStart(o);
                    KABUKI_SOUND(0x2FD);
                    w->state = 31;
                    break;
                case 39:
                    sndPlay(0x82D3);
                    w->dmg->kind = 0x7B;
                    w->gravity = 0.4f;
                    w->side = 0;
                    a->anim = kabukiState39Anim;
                    a->frame = 0.0f;
                    w->state = 40;
                    w->vel[0] = 0.0f;
                    w->vel[1] = 4.0f;
                    w->vel[2] = -3.0f;
                case 40:
                    eneAnimPlayOnce(o, a, 0.3f);
                    if (a->frame >= 5.0f) {
                        a->frame = 5.0f;
                    }
                    if (w->z < -30.0f) {
                        w->z = -30.0f;
                        w->vel[2] = 0.0f;
                    }
                    ang = eneCourseAngle(o, 4);
                    eneTurnToward(o, ang, 0.3141592741f);
                    if ((w->hit & 1) && w->vel[1] <= 0.0f) {
                        KABUKI_SOUND(0x2FA);
                        kabukiSparkleAt(o, -50.0f, 0.0f, 10.0f, 0.1f);
                        kabukiSparkleAt(o, -50.0f, 0.0f, -10.0f, 0.1f);
                        kabukiSparkleAt(o, -30.0f, 0.0f, 10.0f, 0.1f);
                        kabukiSparkleAt(o, -30.0f, 0.0f, -10.0f, 0.1f);
                        kabukiSparkleAt(o, -40.0f, 0.0f, 0.0f, 0.1f);
                        a->frame = 5.0f;
                        w->state = 41;
                    }
                    break;
                case 41:
                    ang = eneCourseAngle(o, 4);
                    eneTurnToward(o, ang, 0.3141592741f);
                    if (w->z < -30.0f) {
                        w->z = -30.0f;
                        w->vel[2] = 0.0f;
                    }
                    if (eneAnimPlayOnce(o, a, 0.4f) != 0) {
                        w->timer = 40;
                        w->state = 42;
                        w->vel[2] = 0.0f;
                    }
                    break;
                case 42:
                    if (w->timer-- <= 0) {
                        if (kabukiMainGetCount() == 0) {
                            PRINTF("!!!!! LAST LAST !!!!!\n");
                            kabukiMainStartEnding(o->parent);
                            w->state = 58;
                        } else {
                            w->state = 43;
                        }
                    }
                    break;
                case 43:
                    w->dmg->kind = 0x7B;
                    a->anim = kabukiState43Anim;
                    a->frame = 0.0f;
                    w->state = 44;
                case 44:
                    if (eneAnimPlayOnce(o, a, 0.3f) != 0) {
                        a->anim = kabukiState44Anim;
                        a->frame = 0.0f;
                        w->timer = 0;
                        kabukiUmimonkStartAttack1(kabukiMainGetUmimonk(o->parent));
                        w->state = 45;
                    }
                    break;
                case 45:
                    if (w->timer-- <= 0) {
                        w->timer = 13;
                        KABUKI_SOUND(0x2F8);
                    }
                    eneAnimLoop(o, a, 0.3f);
                    ang = eneCourseAngle(o, 1);
                    eneTurnToward(o, ang, 0.06283185631f);
                    break;
                case 46:
                    w->rot[1] = eneCourseAngle(o, 4);
                    kabukiJumpStart(o);
                    /* its zeros as ints: one shared mtc1, as the game */
                    SoundLocation2(0x2FD, gGameWork.view, a->pos[0], a->pos[1], a->pos[2], 0, 0, gGameWork.view->fovy);
                    w->state = 31;
                    break;
                case 47:
                    kabukiStop(o);
                    w->side = 1;
                    a->anim = kabukiState47Anim;
                    a->frame = 6.0f;
                    w->timer = 80;
                    kabukiSparkleAt(o, 10.0f, 0.0f, 10.0f, 0.1f);
                    kabukiSparkleAt(o, -13.0f, 0.0f, -10.0f, 0.1f);
                    KABUKI_SOUND(0x2FC);
                    w->state = 50;
                    break;
                case 48:
                    w->side = 1;
                    a->anim = kabukiState47Anim;
                    a->frame = 0.0f;
                    w->timer = 80;
                    w->state = 49;
                case 49:
                    eneAnimPlayOnce(o, a, 0.3f);
                    if (a->frame >= 6.0f) {
                        a->frame = 6.0f;
                    }
                    break;
                case 50:
                    eneAnimPlayOnce(o, a, 0.5f);
                    if (a->frame <= 6.0f) {
                        a->frame = 6.0f;
                    }
                    if (a->frame >= 7.0f) {
                        a->frame = 7.0f;
                    }
                    if (w->timer-- <= 0) {
                        w->timer = 40;
                        w->state = 51;
                    }
                    break;
                case 51:
                    if (eneAnimPlayOnce(o, a, 0.2f) == 0) {
                        kabukiUmimonkAnimAdvance(kabukiMainGetUmimonk(o->parent), -0.015f);
                    } else if (w->timer-- <= 0) {
                        kabukiUmimonkNextState(kabukiMainGetUmimonk(o->parent));
                        w->rot[1] = eneCourseAngle(o, 4);
                        kabukiSetState30(o);
                    }
                    break;
                case 52:
                    kabukiStop(o);
                    w->side = 0;
                    w->rot[1] = eneCourseAngle(o, 1);
                    a->anim = kabukiState43Anim;
                    a->frame = polAnimeLastFrame(a);
                    w->timer = 80;
                    KABUKI_SOUND(0x455);
                    w->state = 53;
                case 53:
                    if (eneAnimPlayOnce(o, a, -1.0f) != 0) {
                        kabukiSparkleAt(o, -50.0f, 0.0f, 10.0f, 0.1f);
                        kabukiSparkleAt(o, -50.0f, 0.0f, -10.0f, 0.1f);
                        kabukiSparkleAt(o, -30.0f, 0.0f, 10.0f, 0.1f);
                        kabukiSparkleAt(o, -30.0f, 0.0f, -10.0f, 0.1f);
                        kabukiSparkleAt(o, -40.0f, 0.0f, 0.0f, 0.1f);
                        w->state = 54;
                    }
                    break;
                case 54:
                    a->scale[1] -= 0.04;
                    if (a->scale[1] <= 0.02) {
                        a->scale[1] = 0.02f;
                    }
                    if (w->timer-- <= 0) {
                        if (kabukiMainGetCount() == 0) {
                            kabukiMainStartEnding(o->parent);
                            eneEndPlayersPowerDown();
                            w->state = 58;
                        } else {
                            w->state = 55;
                        }
                    }
                    break;
                case 55:
                    a->scale[1] += 0.01;
                    if (a->scale[1] >= 0.1) {
                        a->scale[1] = 0.1f;
                        w->state = 56;
                    }
                    break;
                case 56:
                    if (eneAnimPlayOnce(o, a, 0.2f) != 0) {
                        w->rot[1] = eneCourseAngle(o, 4);
                        kabukiJumpStart(o);
                        KABUKI_SOUND(0x2FD);
                        kabukiUmimonkSetState5(kabukiMainGetUmimonk(o->parent));
                        w->state = 31;
                    }
                    break;
                case 58:
                    gGameWork.scenarioRunning = 2;
                    sndPlay(0x82D3);
                    a->scale[1] = 0.1f;
                    w->x = 400.0f;
                    w->y = 48.0f;
                    w->z = -25.0f;
                    w->rot[1] = eneCourseAngle(o, 1);
                    kabukiStop(o);
                    w->side = 0;
                    a->anim = kabukiState43Anim;
                    a->frame = polAnimeLastFrame(a);
                    eneRumbleAll(0xBBBBBBBB, 8000);
                    w->pompom = effectPompomCreate(45.0f, 58.0f, -282.0f, 20.0f, 25, 10.0f, 10, 8000,
                                                   3, 0.1f, 0.03f);
                    sndPlay(0x10E);
                    w->sndTimer = 0;
                    w->state = 59;
                case 59:
                    eneAnimPlayOnce(o, a, -0.1f);
                    kabukiSoundTick(o);
                    if (a->frame <= 3.0f) {
                        kabukiSparkleAt(o, -30.0f, 0.0f, 0.0f, 0.1f);
                        kabukiSparkleAt(o, -30.0f, 0.0f, 0.0f, 0.1f);
                        a->frame = 3.0f;
                        w->timer = 20;
                        w->state = 60;
                    }
                    break;
                case 60:
                    kabukiSoundTick(o);
                    if (w->timer-- <= 0) {
                        w->timer = 60;
                        w->count = 120;
                        scRunStart2(18, 0, 0);
                        w->state = 61;
                    }
                    break;
                case 61:
                    kabukiSoundTick(o);
                    if (!scIsRunning()) {
                        sndPlay(0x301);
                        w->state = 62;
                    }
                    gGameWork.scenarioRunning = 2;
                    break;
                case 62:
                    if (w->timer > 0) {
                        kabukiSoundTick(o);
                    }
                    if (w->timer == 17) {
                        kabukiUmimonkSetState19(kabukiMainGetUmimonk(o->parent));
                        eneRumbleAll(-1, 10);
                    }
                    if (w->timer == 1) {
                        sndPlay(0x10A);
                        sndPlay(0x455);
                    }
                    if (w->timer-- <= 0) {
                        a->scale[1] -= 0.03;
                        if (a->scale[1] <= 0.02) {
                            a->scale[1] = 0.02f;
                        }
                        if (eneAnimPlayOnce(o, a, -0.8f) != 0) {
                            a->scale[1] = 0.02f;
                            kabukiSparkleAt(o, -50.0f, 0.0f, 10.0f, 0.1f);
                            kabukiSparkleAt(o, -50.0f, 0.0f, -10.0f, 0.1f);
                            kabukiSparkleAt(o, -30.0f, 0.0f, 10.0f, 0.1f);
                            kabukiSparkleAt(o, -30.0f, 0.0f, -10.0f, 0.1f);
                            kabukiSparkleAt(o, -40.0f, 0.0f, 0.0f, 0.1f);
                            w->timer = 10;
                            w->state = 63;
                        }
                    }
                    break;
                case 63:
                    if (w->timer-- <= 0) {
                        sndPlay(0x11D);
                        w->timer = 70;
                        eneRumbleAll(-1, 70);
                        eneMakeEx(o, ENE_BIG_BOMB, 0, a->pos[0], a->pos[1], a->pos[2], 2, (u32 (*)(void *, void *, void *)) 4);
                        fadeSetAll(w->timer, 0.0f, 255.0f, 0xFF, 0xFF, 0xFF);
                        w->state = 64;
                    }
                    break;
                case 64:
                    if (w->timer == 0) {
                        tskDestroy(w->pompom);
                    }
                    if (w->timer-- < -39) {
                        s32 i;
                        Camera *cam;
                        Vec3f p;
                        s32 zone;

                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                plAskPos(gGameWork.players[i], &p.x, &p.y, &p.z, &zone);
                                p.x = 650.0f;
                                p.y = 48.0f;
                                plSetPos(gGameWork.players[i], zone, &p);
                            }
                        }
                        cam = camAiGetCam();
                        camSetAt(cam, 178.0f, 73.0f, -606.0f);
                        camSetEye(cam, 335.0f, 85.0f, -575.0f);
                        w->timer = 20;
                        fadeSetAll(20.0f, 255.0f, 0.0f, 0xFF, 0xFF, 0xFF);
                        w->state = 65;
                    }
                    break;
                case 65:
                    w->timer--;
                    if (w->timer == -40) {
                        sndPlay(0x454);
                        kabukiMainShutDoor2(o->parent);
                    }
                    if (w->timer == -100) {
                        s32 i;
                        Vec3f p;
                        s32 zone;

                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                plSetAction(gGameWork.players[i], 0xDB);
                                plAskPos(gGameWork.players[i], &p.x, &p.y, &p.z, &zone);
                                p.y = 48.0f;
                                p.x = 800.0f;
                                plDemoWalkTo(gGameWork.players[i], p, 2.0f);
                            }
                        }
                        w->state = 66;
                    }
                    break;
                case 66: {
                    s32 i;
                    s32 n;

                    for (i = 0, n = 0; i < 4; i++) {
                        if (plIsInPlay(i) && plIsDemoWalking(gGameWork.players[i])) {
                            n++;
                        }
                    }
                    if (n == 0) {
                        kabukiMainFinish(o->parent);
                        w->state = 67;
                    }
                    break;
                }
                case 67:
                    break;
            }
            eneFall(o, w->gravity);
            eneMoveObj(o);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 40.0f) != 0) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneSetModelOnCourse(o, &a->f);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 kabukiUmimonkFace1Tex[];
extern u8 kabukiUmimonkFace2Tex[];

void kabukiUmimonkSetFace1(Obj *o) {
    MONK(o)->body->segs[3] = eneFileAddr(o, kabukiUmimonkFace1Tex, 0);
}

void kabukiUmimonkSetFace2(Obj *o) {
    MONK(o)->body->segs[3] = eneFileAddr(o, kabukiUmimonkFace2Tex, 0);
}

void kabukiUmimonkXFromPlayers(f32 *out) {
    s32 i;
    s32 n;
    s32 zone;
    f32 sum;
    f32 px;
    f32 py;
    f32 pz;

    sum = 0.0f;
    n = 0;
    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plAskPos(gGameWork.players[i], &px, &py, &pz, &zone);
            n++;
            sum += px;
        }
    }
    if (n == 0) {
        *out = 0.0f;
        return;
    }
    *out = (sum / n - 398.5f) * 0.6653279662f + 398.5f;
}

f32 kabukiAtkpointGetX(Obj *o);

void kabukiUmimonkXFromAtkpoint(Obj *o, f32 *out) {
    *out = (kabukiAtkpointGetX(MONK(o)->atk) - 398.5f) * 0.6653279662f + 398.5f;
}

void kabukiUmimonkNextState(WorkPart *p) {
    p->state++;
}

void kabukiUmimonkSetState5(Obj *o) {
    MONK(o)->state = 5;
}

void kabukiUmimonkStartAttack0(Obj *o) {
    MONK(o)->attack = 0;
    MONK(o)->state = 7;
}

void kabukiUmimonkSetState17(Obj *o) {
    MONK(o)->state = 17;
}

void kabukiUmimonkSetState19(Obj *o) {
    MONK(o)->state = 19;
}

void kabukiUmimonkStartAttack1(Obj *o) {
    MONK(o)->attack = 1;
    MONK(o)->state = 7;
}

void kabukiUmimonkAnimAdvance(Obj *o, f32 step) {
    AnimeFrame *b = MONK(o)->body;

    b->frame += step;
}

void kabukiUnusedStub1(void) {
}

extern u8 kabukiUmimonkBodyAnim[];
extern u8 kabukiUmimonkState7Anim[];
extern u8 kabukiUmimonkAtkChain[];

void kabukiAtkpointRetarget(Obj *o);
s32 kabukiAtkpointArrived(Obj *o);
s32 kabukiAtkpointGetPos(Obj *o, f32 *x, f32 *y, f32 *z);

/* "umimonk" */
s32 kabukiUmimonkProc(Obj *o, s16 msg, void *arg) {
    MonkWork *w = MONK(o);
    AnimeFrame *a;
    s32 pad;
    f32 cx;
    f32 cy;
    f32 cz;
    s32 pad2;
    f32 px;
    f32 py;
    f32 pz;

    switch (msg) {
        case 2:
            o->name = "umimonk";
            eneSetup(o, (EneArg *) arg);
            w->body = eneNewAnime(o, (u32) kabukiUmimonkBodyAnim);
            if (w->body == NULL) {
                return -1;
            }
            w->body->scale[2] = 0.7f;
            w->body->scale[0] = w->body->scale[1] = w->body->scale[2];
            eneSetModelOnCourse(o, &w->body->f);
            frmMove(&w->body->f, gFrmLayer3);
            kabukiUmimonkSetFace1(o);
            kabukiUmimonkSetFace1(o);
            break;
        case 1:
            if (w->body != NULL) {
                frmDestroy(&w->body->f);
            }
            break;
        case 0:
            a = w->body;
            switch (w->state) {
                case 0:
                    break;
                case 1:
                    sndPlay(0x14B);
                    effectSibukiCreateWhite(a->pos[0], 33.0f, a->pos[2], 1.2f, 0.5f, 0.03f, 0.08f);
                    w->state = 2;
                case 2:
                    w->y += 8.0f;
                    if (w->y > 50.0f) {
                        w->timer = 10;
                        w->y = 50.0f;
                        sndPlay(0x302);
                        effectSibukiCreateWhite(a->pos[0], 33.0f, a->pos[2], 1, 0.2f, 0.02f, 0.02f);
                        w->state = 3;
                    }
                    break;
                case 3:
                    if (!(w->timer & 1)) {
                        w->y = 50.0f;
                    } else {
                        w->y = 48.0f;
                    }
                    if (w->timer-- <= 0) {
                        w->y = 50.0f;
                        effectSibukiCreateWhite(a->pos[0], 33.0f, a->pos[2], 1, 0.2f, 0.02f, 0.02f);
                        w->state = 4;
                    }
                case 4:
                    eneAnimLoop(o, a, 0.5f);
                    break;
                case 5:
                    kabukiUmimonkSetFace1(o);
                    a->anim = kabukiUmimonkBodyAnim;
                    a->frame = 0.0f;
                    w->y = 50.0f;
                    if (w->aniAtk != NULL) {
                        w->aniAtk = (ACollFrame *) aniAtkDelete(w->aniAtk);
                    }
                    if (w->atk != NULL) {
                        w->atk = tskDestroy(w->atk);
                    }
                    w->state = 6;
                case 6:
                    eneAnimLoop(o, a, 0.5f);
                    enePlayersCenter(&cx, &cy, &cz);
                    eneTurnToward(o, mathAtan2(cx - a->pos[0], cz - a->pos[2]), 0.06283185631f);
                    kabukiUmimonkXFromPlayers(&cx);
                    w->vel[0] = (cx - w->x) / 30.0f;
                    w->x += w->vel[0];
                    break;
                case 7:
                    SoundLocation2(0x302, gGameWork.view, a->pos[0], a->pos[1], a->pos[2], 0.0f, 0.0f,
                                   gGameWork.view->fovy);
                    kabukiUmimonkSetFace2(o);
                    a->anim = kabukiUmimonkState7Anim;
                    a->frame = 0.0f;
                    w->count = 5;
                    effectSibukiCreateWhite(a->pos[0], 33.0f, a->pos[2], 1, 0.2f, 0.02f, 0.02f);
                    w->atk = eneMakeUnder(o, ENE_KABUKI_ATKPOINT, 1, w->x, 50.0f, 0.0f);
                    if (w->atk == NULL) {
                        w->state = 5;
                    } else {
                        kabukiAtkpointRetarget(w->atk);
                        w->state = 11;
                    }
                    break;
                case 8:
                    w->rot[1] += 0.01163552888f;
                    eneAnimPlayOnce(o, a, 0.5f);
                    if (a->frame >= 9.0f) {
                        eneRumbleAll(-1, 20);
                        SoundLocation2(0x10A, gGameWork.view, a->pos[0], a->pos[1], a->pos[2], 0.0f, 0.0f,
                                       gGameWork.view->fovy);
                        w->timer = 20;
                        a->frame = 9.0f;
                        w->state = 9;
                    }
                    break;
                case 9:
                    if (w->timer-- <= 0) {
                        if (w->aniAtk != NULL) {
                            w->aniAtk = (ACollFrame *) aniAtkDelete(w->aniAtk);
                        }
                        w->state = 10;
                    }
                    break;
                case 10:
                    w->rot[1] -= 0.00930842307503986117;
                    if (eneAnimPlayOnce(o, a, 0.4f) != 0) {
                        if (w->count-- <= 0) {
                            if (w->attack != 0) {
                                kabukiSetState46(kabukiMainGetKabuki(o->parent));
                            } else {
                                kabukiSetState30(kabukiMainGetKabuki(o->parent));
                            }
                            tskDestroy(w->atk);
                            w->atk = NULL;
                            w->state = 5;
                        } else {
                            kabukiAtkpointRetarget(w->atk);
                            w->state = 11;
                        }
                    }
                    break;
                case 11:
                    kabukiAtkpointGetPos(w->atk, &px, &py, &pz);
                    eneTurnToward(o, mathAtan2(px - a->pos[0], pz - a->pos[2]), 0.06283185631f);
                    kabukiUmimonkXFromAtkpoint(o, &px);
                    w->vel[0] = (px - w->x) / 15.0f;
                    w->x += w->vel[0];
                    if (kabukiAtkpointArrived(w->atk) && w->vel[0] <= 1.0f) {
                        a->anim = kabukiUmimonkState7Anim;
                        a->frame = 0.0f;
                        effectSibukiCreateWhite(a->pos[0], 33.0f, a->pos[2], 1, 0.2f, 0.02f, 0.02f);
                        if (w->aniAtk != NULL) {
                            w->aniAtk = (ACollFrame *) aniAtkDelete(w->aniAtk);
                        }
                        w->aniAtk = aniAtkCreate(&uda_atk_root_frame->frame, &a->f, (u32) kabukiUmimonkAtkChain,
                                                 eneGetFile(o, 0), 0);
                        if (w->aniAtk != NULL) {
                            w->aniAtk->kind = 0x78;
                        }
                        w->state = 8;
                    }
                    break;
                case 12:
                    eneRumbleAll(-1, 20);
                    SoundLocation2(0x10A, gGameWork.view, a->pos[0], a->pos[1], a->pos[2], 0.0f, 0.0f,
                                   gGameWork.view->fovy);
                    kabukiUmimonkSetFace1(o);
                    a->anim = kabukiUmimonkState7Anim;
                    a->frame = 9.0f;
                    w->timer = 80;
                    w->state = 13;
                case 13:
                    if (w->timer-- <= 0) {
                        eneAnimPlayOnce(o, a, 0.1f);
                    }
                    break;
                case 14:
                    eneRumbleAll(-1, 20);
                    SoundLocation2(0x10A, gGameWork.view, a->pos[0], a->pos[1], a->pos[2], 0.0f, 0.0f,
                                   gGameWork.view->fovy);
                    kabukiUmimonkSetFace1(o);
                    a->anim = kabukiUmimonkState7Anim;
                    a->frame = 8.149999619f;
                    w->state = 15;
                    break;
                case 16:
                    if (eneAnimPlayOnce(o, a, -0.2f) != 0) {
                        w->state = 5;
                    }
                    break;
                case 17:
                    kabukiUmimonkSetFace1(o);
                    a->anim = kabukiUmimonkState7Anim;
                    a->frame = 0.0f;
                    w->x = 562.0f;
                    w->y = 50.0f;
                    w->z = -149.0f;
                    w->rot[1] = 0.4688940048f;
                    w->state = 18;
                    break;
                case 19:
                    eneAnimPlayOnce(o, a, 0.5f);
                    if (a->frame >= 9.0f) {
                        a->frame = 9.0f;
                    }
                    break;
            }
            if (w->aniAtk != NULL) {
                ((AniAtkFrame *) w->aniAtk->model)->time = a->frame;
                if (acollAtkWasHit(w->aniAtk) == 1) {
                    if (w->aniAtk->hitKind == 0x7A) {
                        if (w->state < 12) {
                            w->state = 14;
                            if (w->aniAtk != NULL) {
                                w->aniAtk = (ACollFrame *) aniAtkDelete(w->aniAtk);
                            }
                            if (w->atk != NULL) {
                                w->atk = tskDestroy(w->atk);
                            }
                        }
                    } else if (w->aniAtk->hitKind == 0x7B) {
                        if (w->atk != NULL) {
                            w->atk = tskDestroy(w->atk);
                        }
                        if (w->state < 12) {
                            w->state = 12;
                        }
                    } else if ((Obj *) w->aniAtk->hitPower != NULL) {
                        tskSendMsg((Obj *) w->aniAtk->hitPower, 7, o);
                    }
                }
            }
            eneSetModelOnCourse(o, &a->f);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 kabukiDoorAnim[];

/* "door" */
s32 kabukiDoorProc(Obj *o, s16 msg, void *arg) {
    DoorWork *w = DOOR(o);

    switch (msg) {
        case 2:
            o->name = "door";
            eneSetup(o, arg);
            w->anim = eneNewAnime(o, (u32) kabukiDoorAnim);
            if (w->anim == NULL) {
                return -1;
            }
            frmMove(&w->anim->f, gFrmLayer3);
            w->open = FALSE;
            break;
        case 1:
            if (w->anim != NULL) {
                frmDestroy(&w->anim->f);
            }
            break;
        case 0:
            if (w->open) {
                eneAnimPlayOnce(o, w->anim, 0.2f);
            } else {
                eneAnimPlayOnce(o, w->anim, -0.2f);
            }
            eneSetModel(o, &w->anim->f);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void kabukiDoorSetOpen(Obj *o, s32 open) {
    DOOR(o)->open = open;
}

void kabukiDoorSetRotY(Obj *o, f32 ry) {
    DOOR(o)->rot[1] = ry;
}

extern Gfx kabukiKiriDl[];

/* "kiri" */
s32 kabukiKiriProc(Obj *o, s16 msg, void *arg) {
    KiriWork *w = KIRI(o);
    s32 pad;
    PolFrame *m;

    switch (msg) {
        case 2:
            o->name = "kiri";
            eneSetup(o, (EneArg *) arg);
            w->pol = eneNewPol(o, kabukiKiriDl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayer8);
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->pol->f, 35, 15, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x78;
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->pol->f, 30, 30, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 0x7A;
            m = w->pol;
            fadeAnimStart(&w->anim, gItexSparkleAnim, 0, &w->pol->segs[3], floadGetSetFile(0x123, 0));
            w->alpha = 0xFF;
            m->scale[0] = 0.05f;
            m->scale[1] = 0.05f;
            m->flags |= 7;
            m->scale[2] = 0.05f;
            break;
        case 1:
            if (w->pol != NULL) { frmDestroy(&w->pol->f); }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            break;
        case 3:
            w->pol->dl = eneDlEnvPrimColor(o, 0, 0x30, 0, 0x80, 0xFF, 0, w->alpha);
            break;
        case 0:
            m = w->pol;
            if (w->atk != NULL && acollAtkWasHit(w->atk) == 1) {
                if ((Obj *) w->atk->hitPower != NULL) {
                    tskSendMsg((Obj *) w->atk->hitPower, 7, o);
                }
            }
            fadeAnimStep(&w->anim);
            eneMoveObj(o);
            eneSetModelOnCourse(o, &w->pol->f);
            w->vel[0] -= 0.05;
            w->vel[1] += 0.05;
            m->scale[0] += 0.03;
            if (m->scale[0] >= 0.7) {
                m->scale[0] = 0.7f;
            }
            m->scale[1] = m->scale[2] = m->scale[0];
            w->alpha -= 8;
            if (w->alpha <= 64) {
                if (w->atk != NULL) {
                    w->atk = (ACollFrame *) frmDestroy(&w->atk->frame);
                }
            }
            if (w->alpha <= 0) {
                return -1;
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

Obj *kabukiKiriMake(Obj *o) {
    KabukiWork *w = KABUKI(o);
    s32 way = eneFaceWay(o);
    s32 pad[3];
    f32 vx;
    f32 x;
    f32 y;
    f32 z;
    Obj *e;

    if (way == 2) {
        x = w->x - 16.0f;
        vx = -3.7f;
    } else if (way == 8) {
        x = w->x + 16.0f;
        vx = 3.7f;
    } else {
        x = w->x;
        vx = 0.0f;
    }
    y = w->y + 26.0f;
    z = w->z;
    e = eneMakeUnder(o, ENE_KABUKI_KIRI, 1, x, y, z);
    if (e != NULL) {
        KIRI(e)->vel[0] = vx;
        KIRI(e)->vel[1] = -1.0f;
    }
    return e;
}

/* The shadow's work ("kage"): a dark disc under its parent. */
typedef struct KageWork {
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
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ s32 small; /* eneSetup's value: 0 for the bigger shadow */
} KageWork;

#define KAGE(obj) ((KageWork *) FX(obj))

extern Gfx kabukiKageDl[];

/* "kage"
 * FAKE: the empty "if (w != NULL) {}" after case 0's w keeps the work pointer and spills it
 * across eneWorldPos as the game does; without it IDO folds the read into an offset
 * (IDO_NOTES 344). */
s32 kabukiKageProc(Obj *o, s16 msg, void *arg) {
    s32 pad[2];
    PolFrame *p;
    Vec3f pos;
    KageWork *w;
    s32 pad2;

    switch (msg) {
        case 2:
            o->name = "kage";
            w = KAGE(o);
            w->small = eneSetup(o, (EneArg *) arg);
            w->pol = eneNewPol(o, kabukiKageDl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayer4);
            w->rot[0] = -1.570796371f;
            p = w->pol;
            if (w->small == 0) {
                p->scale[0] = p->scale[1] = 1.3f;
            } else {
                p->scale[0] = p->scale[1] = 1.0f;
            }
            break;
        case 1:
            if (KAGE(o)->pol != NULL) {
                frmDestroy(&KAGE(o)->pol->f);
            }
            break;
        case 3:
            p = KAGE(o)->pol;
            p->dl = gGfxDlHead;
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 0, 0, 0, 0xC0);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            w = KAGE(o);
            if (w != NULL) {} /* FAKE */
            p = w->pol;
            eneWorldPos(o->parent, &pos);
            w->x = pos.x;
            w->y = 48.5f;
            w->z = pos.z;
            if (w->small == 0) {
                p->scale[0] = p->scale[1] = 1.3f;
            } else {
                p->scale[0] = p->scale[1] = 1.0f;
            }
            eneSetModel(o, &p->f);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern Gfx kabukiWaterModelDl[];

/* "water" */
s32 kabukiWaterProc(Obj *o, s16 msg, void *arg) {
    WaterWork *w = WATER(o);

    switch (msg) {
        case 2:
            o->name = "water";
            eneSetup(o, (EneArg *) arg);
            w->draw = kabukiWaterDl;
            w->step = -1;
            w->pol = eneNewPol(o, kabukiWaterModelDl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayers[6]);
            w->pol->scale[2] = 0.1f;
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
            w->rot[1] = 1.570796371f;
            break;
        case 1:
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gGfxDlHead = w->draw(gGfxDlHead, w->scroll);
            w->scroll += w->step;
            eneSetModelOnCourse(o, &w->pol->f);
            break;
        case 0:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern Gfx kabukiAtkpointDl[];
extern Gfx kabukiAtkpointRingDl[];

#define sinf (*sinf)

/* "atkpoint" */
s32 kabukiAtkpointProc(Obj *o, s16 msg, void *arg) {
    AtkPointWork *w = ATKPOINT(o);
    PolFrame *m;
    PolFrame *r;
    s32 pad;
    f32 x;
    f32 z;
    f32 px;
    f32 py;
    f32 pz;
    s32 zone;
    s32 pad2;
    s32 pad3;
    s32 pad4;
    f32 s;

    switch (msg) {
        case 2:
            o->name = "atkpoint";
            eneSetup(o, (EneArg *) arg);
            w->model = eneNewPol(o, kabukiAtkpointDl);
            if (w->model == NULL) {
                return -1;
            }
            m = w->model;
            w->ring = eneNewPol(o, kabukiAtkpointRingDl);
            if (w->ring == NULL) {
                return -1;
            }
            r = w->ring;
            m->scale[0] = 0.2f;
            m->scale[1] = 0.2f;
            m->flags |= 7;
            m->scale[2] = 0.2f;
            r->flags |= 7;
            r->scale[0] = 0.2f;
            r->scale[1] = 0.2f;
            r->scale[2] = 0.2f;
            w->shadow = shadowCreate();
            w->unused4C = 1.0f;
            w->target = eneRandomPlayerIndex();
            break;
        case 1:
            if (w->model != NULL) { frmDestroy(&w->model->f); }
            if (w->ring != NULL) {
                frmDestroy(&w->ring->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            m = w->model;
            r = w->ring;
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (!plIsInPlay(w->target)) {
                        w->target = eneRandomPlayerIndex();
                        if (w->target == 0) {
                            return -1;
                        }
                    }
                    plAskPos(gGameWork.players[w->target], &px, &py, &pz, &zone);
                    if (w->x < px) {
                        w->vel[0] = 4.0f;
                    }
                    if (px <= w->x) {
                        w->vel[0] = -4.0f;
                    }
                    if (w->x < 40.0f) {
                        w->x = 40.0f;
                        w->state = 1;
                    }
                    if (w->x > 730.0f) {
                        w->x = 730.0f;
                        w->state = 1;
                    }
                    eneMoveObj(o);
                    w->z = 0.0f;
                    w->y = 50.0f;
                    r->scale[0] = 0.2f;
                    r->scale[1] = 0.2f;
                    r->scale[2] = 0.2f;
                    if (fabsf(px - w->x) <= fabsf(w->vel[0] + 1.0f)) {
                        w->state = 1;
                    }
                    eneSetModelOnCourse(o, &m->f);
                    eneSetModelOnCourse(o, &r->f);
                    r->pos[1] += 15.0f;
                    break;
                case 1:
                    w->phase += 30.0;
                    if (w->phase >= 360.0) {
                        w->phase -= 360.0;
                    }
                    m->pos[1] = sinf(w->phase / 180.0 * 3.14159274101257324) * 2.5 + w->y;
                    s = sinf(w->phase / 180.0 * 3.14159274101257324) * 0.05 + 0.2;
                    m->scale[0] = m->scale[1] = m->scale[2] = s;
                    break;
                case 2:
                    w->phase = 0.0f;
                    m->pos[1] = w->y;
                    r->scale[0] = 0.0f;
                    r->scale[1] = 0.0f;
                    r->scale[2] = 0.0f;
                    m->scale[0] = 0.0f;
                    m->scale[1] = 0.0f;
                    m->scale[2] = 0.0f;
                    break;
            }
            vramCoursePointXZ(w->zone, w->x, w->z, &x, &z);
            shadowPut(w->shadow, x, w->y, z, 0.1f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void kabukiUnusedStub2(void) {
}

void kabukiAtkpointRetarget(Obj *o) {
    AtkPointWork *w = ATKPOINT(o);

    if (o != NULL) {
        w->target = eneRandomPlayerIndex();
        w->state = 0;
    }
}

s32 kabukiAtkpointArrived(Obj *o) {
    if (o != NULL && ATKPOINT(o)->state == 1) {
        return TRUE;
    }
    return FALSE;
}

f32 kabukiAtkpointGetX(Obj *o) {
    if (o != NULL) {
        return ATKPOINT(o)->x;
    }
    return 0.0f;
}

s32 kabukiAtkpointGetPos(Obj *o, f32 *x, f32 *y, f32 *z) {
    AtkPointWork *w = ATKPOINT(o);

    if (o != NULL) {
        *x = w->model->pos[0];
        *y = w->model->pos[1];
        *z = w->model->pos[2];
        return TRUE;
    }
    return FALSE;
}

void kabukiItexBomb(f32 x, f32 y, f32 z, f32 size) {
    ItexInit init;

    init = gItexTemplate;
    init.file = 0x11F;
    init.frames = gItexBombAnim;
    init.flags = 3;
    init.prim.r = 0xFF; init.prim.g = 0xFF; init.prim.b = 0x80;
    init.env.r = 0xFF; init.env.g = 0x28; init.env.b = 0;
    init.pos[0] = x;
    init.pos[1] = y;
    init.fade = 0;
    init.grow.x = init.grow.y = init.grow.z = 1.01f;
    init.pos[2] = z;
    init.scale[2] = size;
    init.scale[1] = size;
    init.scale[0] = size;
    tskCreate(gEffectTask, itex, 0, &init);
}

/* A pillar's work ("hasira"): it drops from above where the players stand, then bursts. */
typedef struct HasiraWork {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone; /* the course */
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ ACollFrame *atk[3];
    /* 0x44 */ ACollFrame *dmg;
    /* 0x48 */ Obj *kage; /* kind 0x2D */
    /* 0x4C */ s32 timer;
} HasiraWork;

#define HASIRA(obj) ((HasiraWork *) FX(obj))

extern Gfx kabukiHasiraDl[];

/* "hasira" */
s32 kabukiHasiraProc(Obj *o, s16 msg, void *arg) {
    HasiraWork *w = HASIRA(o);
    s32 pad;
    PolFrame *m;
    Vec3f pos;
    s32 data;

    switch (msg) {
        case 2:
            o->name = "hasira";
            data = eneSetup(o, (EneArg *) arg);
            w->pol = eneNewPol(o, kabukiHasiraDl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayer3);
            m = w->pol;
            enePlayersCenterOnCourse(&pos.x, &pos.y, &pos.z);
            w->y = 200.0f;
            w->z = 0.0f;
            switch (data) {
                case 1:
                    w->timer = 0;
                    w->x = pos.x + 120.0f;
                    break;
                case 2:
                    w->timer = 10;
                    w->x = pos.x + 60.0f;
                    break;
                case 3:
                    w->timer = 20;
                    w->x = pos.x;
                    break;
                case 4:
                    w->timer = 30;
                    w->x = pos.x - 60.0f;
                    break;
                case 5:
                    w->timer = 40;
                    w->x = pos.x - 120.0f;
                    break;
            }
            if (w->x >= 740.0f) {
                return -1;
            }
            if (w->x <= 40.0f) {
                return -1;
            }
            eneSetModelOnCourse(o, &m->f);
            w->atk[0] = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &m->f, 10, 10, 0, 0);
            if (w->atk[0] == NULL) {
                return -1;
            }
            if (w->atk[0] != NULL) {
                w->atk[0]->kind = 0x78;
            }
            w->atk[1] = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &m->f, 10, 30, 0, 0);
            if (w->atk[1] == NULL) {
                return -1;
            }
            if (w->atk[1] != NULL) {
                w->atk[1]->kind = 0x78;
            }
            w->atk[2] = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &m->f, 10, 60, 0, 0);
            if (w->atk[2] == NULL) {
                return -1;
            }
            if (w->atk[2] != NULL) {
                w->atk[2]->kind = 0x78;
            }
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &m->f, 10, 60, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 0x65;
            w->kage = eneMakeEx(o, ENE_KABUKI_KAGE, 0, m->pos[0], 48.0f, m->pos[2], 2,
                                (u32 (*)(void *, void *, void *)) 1);
            break;
        case 1:
            if (w->pol != NULL) { frmDestroy(&w->pol->f); }
            if (w->atk[0] != NULL) {
                frmDestroy(&w->atk[0]->frame);
            }
            if (w->atk[1] != NULL) {
                frmDestroy(&w->atk[1]->frame);
            }
            if (w->atk[2] != NULL) {
                frmDestroy(&w->atk[2]->frame);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            break;
        case 0:
            m = w->pol;
            if (w->atk[0] != NULL && acollAtkWasHit(w->atk[0]) == 1) {
                if ((Obj *) w->atk[0]->hitPower != NULL) {
                    tskSendMsg((Obj *) w->atk[0]->hitPower, 7, o);
                }
            }
            if (w->atk[1] != NULL && acollAtkWasHit(w->atk[1]) == 1) {
                if ((Obj *) w->atk[1]->hitPower != NULL) {
                    tskSendMsg((Obj *) w->atk[1]->hitPower, 7, o);
                }
            }
            if (w->atk[2] != NULL && acollAtkWasHit(w->atk[2]) == 1) {
                if ((Obj *) w->atk[2]->hitPower != NULL) {
                    tskSendMsg((Obj *) w->atk[2]->hitPower, 7, o);
                }
            }
            switch (w->state) {
                case 0:
                    if (w->timer-- <= 0) {
                        w->rot[1] += 0.1570796371f;
                        eneFall(o, 0.4f);
                        eneMoveObj(o);
                        if (w->y <= 45.0f) {
                            eneRumbleAll(0xBBBBBBBB, 20);
                            kabukiItexSparkle(m->pos[0] + 5.0f, 48.0f, m->pos[2] + 5.0f, 0.1f);
                            kabukiItexSparkle(m->pos[0] - 5.0f, 48.0f, m->pos[2] + 5.0f, 0.1f);
                            kabukiItexSparkle(m->pos[0] + 5.0f, 48.0f, m->pos[2] - 5.0f, 0.1f);
                            kabukiItexSparkle(m->pos[0] - 5.0f, 48.0f, m->pos[2] - 5.0f, 0.1f);
                            soundIncamCam(0x108, m->pos[0], m->pos[1], m->pos[2]);
                            w->timer = 20;
                            w->state = 1;
                        }
                    }
                    break;
                case 1:
                    if (--w->timer == 0) {
                        if (w->pol != NULL) {
                            w->pol = (PolFrame *) frmDestroy(&w->pol->f);
                        }
                    }
                    if (w->timer == 0) {
                        vramCoursePointXZ(w->zone, w->x, w->z, &pos.x, &pos.z);
                        kabukiItexBomb(pos.x, w->y + 5.0f, pos.z, 0.1f);
                        soundIncamCam(0x11B, pos.x, w->y, pos.z);
                    }
                    if (w->timer == -2) {
                        vramCoursePointXZ(w->zone, w->x, w->z, &pos.x, &pos.z);
                        kabukiItexBomb(pos.x, w->y + 20.0f, pos.z, 0.1f);
                        soundIncamCam(0x11B, pos.x, w->y, pos.z);
                    }
                    if (w->timer == -4) {
                        vramCoursePointXZ(w->zone, w->x, w->z, &pos.x, &pos.z);
                        kabukiItexBomb(pos.x, w->y + 35.0f, pos.z, 0.1f);
                        soundIncamCam(0x11B, pos.x, w->y, pos.z);
                    }
                    if (w->timer == -6) {
                        vramCoursePointXZ(w->zone, w->x, w->z, &pos.x, &pos.z);
                        kabukiItexBomb(pos.x, w->y + 50.0f, pos.z, 0.1f);
                        soundIncamCam(0x11B, pos.x, w->y, pos.z);
                    }
                    if (w->timer == -8) {
                        vramCoursePointXZ(w->zone, w->x, w->z, &pos.x, &pos.z);
                        kabukiItexBomb(pos.x, w->y + 65.0f, pos.z, 0.1f);
                        soundIncamCam(0x11B, pos.x, w->y, pos.z);
                        return -1;
                    }
                    break;
            }
            if (w->pol != NULL) {
                eneSetModelOnCourse(o, &m->f);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
