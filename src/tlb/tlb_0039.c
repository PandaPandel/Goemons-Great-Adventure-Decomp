#include "tlb.h"

/* The file's .data (IDO_NOTES 343): three setup display lists, then (last) the random number's
 * seed, a static of sanbaRand. */
Gfx sanbaBodyDl[] = {
    gsSPDisplayList(gDrawSetupLitTexEdgeDl),
    gsDPSetRenderMode(AA_EN | Z_CMP | Z_UPD | CVG_DST_CLAMP | ZMODE_OPA | CVG_X_ALPHA | ALPHA_CVG_SEL | GBL_c1(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM), AA_EN | Z_CMP | Z_UPD | CVG_DST_CLAMP | ZMODE_OPA | CVG_X_ALPHA | ALPHA_CVG_SEL | GBL_c2(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)),
    gsDPSetCombineMode(G_CC_MODULATEIDECALA, G_CC_MODULATEIDECALA),
    gsSPEndDisplayList(),
}; /* the body's */
Gfx winchXluDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetRenderMode(G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2),
    gsDPSetCombineLERP(NOISE, 0, TEXEL0, 0, 0, 0, 0, ENVIRONMENT, NOISE, 0, TEXEL0, 0, 0, 0, 0, ENVIRONMENT),
    gsDPSetAlphaCompare(G_AC_DITHER),
    gsDPPipeSync(),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetColorDither(G_CD_DISABLE),
    gsSPEndDisplayList(),
}; /* the winch's, see-through */
Gfx waterSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_XLU_INTER2),
    gsDPSetCombineLERP(TEXEL0, TEXEL1, ENV_ALPHA, TEXEL1, 0, 0, 0, PRIMITIVE, PRIMITIVE, ENVIRONMENT, COMBINED, ENVIRONMENT, 0, 0, 0, COMBINED),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsSPEndDisplayList(),
}; /* the water's */

/* The enemy this file works with, found through gEneManager by its id. */
#define E61_ID 0xE61

/* What this file reads of the enemy's work (after its Obj); the rest is not known yet. */
typedef struct E61Part {
    /* 0x00 */ Obj obj;
    /* 0x20 */ u8 state; /* counted up by sanbaNextState; set and tested by its neighbours */
    /* 0x21 */ u8 zone;
    /* 0x22 */ s16 spawn;
    /* 0x24 */ EneKind *kind;
    /* 0x28 */ u8 flags;
    /* 0x29 */ u8 pad29[7];
    /* 0x30 */ f32 x;
    /* 0x34 */ f32 y;
    /* 0x38 */ f32 z;
    /* 0x3C */ f32 rotX;
    /* 0x40 */ f32 rotY;
    /* 0x44 */ u8 pad44[0x10];
    /* 0x54 */ AnimeFrame *anim;
} E61Part;

/* The same part's work, at its task + 0x20 (E61Part's state on): "sanba"
 * (sanbaProc), the boss's body in the water. */
typedef struct E61PartW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rotX;
    /* 0x20 */ f32 rotY;
    /* 0x24 */ f32 rotZ;
    /* 0x28 */ f32 vx;
    /* 0x2C */ f32 vy;
    /* 0x30 */ f32 vz;
    /* 0x34 */ AnimeFrame *model;
    /* 0x38 */ LightFrame *light;
    /* 0x3C */ f32 turn; /* its turning speed */
    /* 0x40 */ s32 count;
    /* 0x44 */ s32 flag;
    /* 0x48 */ s32 count2;
    /* 0x4C */ s32 soundTimer;
    /* 0x50 */ u8 pad50[0x4];
    /* 0x54 */ s32 target; /* the player it turns to */
    /* 0x58 */ s32 timer;
    /* 0x5C */ s32 step; /* of sanbaNextSequenceStep's sequence */
} E61PartW;

#define E61PART(p) ((E61PartW *) FX(p))

typedef struct E61Work {
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
    /* 0x34 */ E61Part *body; /* kind 0x14 */
    /* 0x38 */ Obj *water; /* kind 0x19 */
    /* 0x3C */ Obj *pldemo; /* kind 0x17: an enemy whose states 2, 3, 5 and 11 this reads */
    /* 0x40 */ Obj *chase; /* kind 0x1B */
    /* 0x44 */ Obj *parts[6]; /* the rafts (kind 0x1C), the parents sanbaMakeWinches makes kind
                               * 0x1D under */
    /* 0x5C */ s32 slots[6];
    /* 0x74 */ Obj *drop; /* kind 0x15 or 0x16 */
    /* 0x78 */ Obj *gauge; /* its life gauge (pipGauge*) */
    /* 0x7C */ Obj *jellyfish[2]; /* kind 0x18 */
    /* 0x84 */ Obj *taiBgm; /* its "tai_bgm" task (sanbaTaiBgmRestart sets it up) */
    /* 0x88 */ s32 timer;
    /* 0x8C */ s32 round;
    /* 0x90 */ s32 life; /* its life, 50 at first, 10 a round */
    /* 0x94 */ f32 camDist; /* the camera's, from the players' middle (sanbaCamEyeAround) */
    /* 0x98 */ f32 camHeight;
    /* 0x9C */ f32 camAngle;
    /* 0xA0 */ f32 camAtY; /* added to the camera's target height */
    /* 0xA4 */ u8 battleBgm;
} E61Work;

/* The tasks' works, only the fields the small helpers read (strings in IDA files/0039: "main",
 * "pldemo", "jellyfish", "water", "tarai", "maruta", "futa", "chase", "ikada", "winch", "sanba",
 * "sibuki", "ripple", "marbl", "cTHUNDER", "tai_bgm"); each helper follows its task's proc. */
typedef struct Tlb39MainW {
    /* 0x00 */ u8 unused00;
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
    /* 0x34 */ u8 pad34[8];
    /* 0x3C */ s32 pldemo;
    /* 0x40 */ s32 chase;
} Tlb39MainW;

typedef struct PldemoW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone; /* set at state 6 */
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ AnimeFrame *m[2]; /* the two models of the opening (sanbaPldemoOpeningAnim0, sanbaPldemoOpeningAnim1) */
    /* 0x3C */ Frame *unused3C; /* destroyed in message 1 if set, never made */
    /* 0x40 */ s32 count;
    /* 0x44 */ u8 pad44[0x4];
    /* 0x48 */ f32 phase;
    /* 0x4C */ u8 unused4C[4]; /* one per player, written 0 at state 9, never read */
} PldemoW;

typedef struct JellyfishW {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x; /* the player's place */
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *model;
    /* 0x38 */ ACollFrame *atk;
    /* 0x3C */ s32 player; /* the player it follows */
} JellyfishW;

typedef struct TaraiW {
    /* 0x00 */ u8 state; /* 0 falling, 1 bounced off */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y; /* its height */
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *model;
    /* 0x38 */ f32 angle; /* the way it bounced off */
} TaraiW;

typedef struct ChaseW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rotX;
    /* 0x20 */ f32 rotY;
    /* 0x24 */ f32 rotZ;
    /* 0x28 */ f32 vx;
    /* 0x2C */ f32 vy;
    /* 0x30 */ f32 vz;
    /* 0x34 */ AnimeFrame *model;
    /* 0x38 */ PolFrame *spark; /* made at the end, sinks into it */
    /* 0x3C */ RideFrame *ride;
    /* 0x40 */ ACollFrame *atk;
    /* 0x44 */ ACollFrame *dmg;
    /* 0x48 */ s32 timer;
    /* 0x4C */ s32 flag;
    /* 0x50 */ f32 speed; /* its speed */
    /* 0x54 */ f32 prev[3]; /* x, y, z a step ago: what a rider is moved by */
    /* 0x60 */ f32 phase;
} ChaseW;

typedef struct IkadaW {
    /* 0x00 */ u8 state; /* 0 settling, 1 floating */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y; /* its height */
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rotX;
    /* 0x20 */ f32 rotY;
    /* 0x24 */ f32 rotZ;
    /* 0x28 */ f32 vx;
    /* 0x2C */ f32 vy; /* its vertical speed */
    /* 0x30 */ f32 vz;
    /* 0x34 */ PolFrame *model;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ u8 pad3C[0x4];
    /* 0x40 */ u8 riders; /* counted by message 6 each step */
} IkadaW;

typedef struct TaiBgmW {
    /* 0x00 */ s32 played;
    /* 0x04 */ s32 wait;
    /* 0x08 */ s32 battle;
} TaiBgmW;

#define TLB39_MAIN(o) ((Tlb39MainW *) FX(o))
#define PLDEMO(o) ((PldemoW *) FX(o))
#define JELLYFISH(o) ((JellyfishW *) FX(o))
#define TARAI(o) ((TaraiW *) FX(o))
#define CHASE(o) ((ChaseW *) FX(o))
#define IKADA(o) ((IkadaW *) FX(o))
#define TAI_BGM(o) ((TaiBgmW *) FX(o))

void sanbaPlayersMiddle(Vec3f *out);
void sanbaCamEyeAround(Vec3f *pos, Vec3f *out, f32 r, f32 dy, f32 deg);
void sanbaMakeWinches(Obj *o, s32 n);
void sanbaMainShowGauge(void);
s32 sanbaPldemoIsState2(Obj *o);
s32 sanbaPldemoIsState3(Obj *o);
s32 sanbaPldemoIsState5(Obj *o);
s32 sanbaPldemoIsState11(Obj *o);
void sanbaPldemoSetState12(Obj *o);
void sanbaJellyfishSetPlayer(Obj *o, s32 v);
f32 sanbaTaraiGetHeight(Obj *o);
void sanbaWaterSetMaxHeight(void);
void sanbaIkadaSetFloatHeight(Obj *o);
void sanbaSetState1(E61Part *p);
s32 sanbaIsState5(E61Part *p);
void sanbaSetState6(E61Part *p);
void sanbaSetDropState(E61Part *p, s32 flag);
s32 sanbaIsState33(E61Part *p);
Obj *sanbaTaiBgmCreate(Obj *obj, void *arg);
void sanbaTaiBgmRestart(Obj *o, s32 v);

#define E61_SKIP_CHECK(w)                                       \
    if (w->state >= 6 && w->state < 11 && fadeStartPressed()) { \
        sndPlay(7);                                             \
        w->state = 11;                                          \
    }

/* camMoveAt's zeros are "0.0f, y, 0" (a float and an int literal: the game's two "li"s). */
/* "main": the stage's boss fight (enemy 0xE61) and its camera. The camera follows the players'
 * middle; states 0-4 are the approach (enemy kind 0x17 sets them going), 5-10 the boss's
 * entrance (Start skips it to 11), 11-12 put the players on the rafts, and 13-21 are the fight's
 * rounds: once its six slots are clear it loses 10 life and a tub falls (15-17) or a log (18-20)
 * until the life is gone. */
s32 sanbaMainProc(Obj *obj, s16 msg, void *arg) {
    E61Work *w = (E61Work *) (obj + 1);
    Camera *cam = camAiGetCam();
    s32 pad[2];
    Vec3f pos;
    Vec3f eye;
    s32 zone;
    s32 pad2;
    s32 i;
    s32 j;
    s32 k;
    Player *pl;
    f32 h;

    camAiSetHold(1);
    switch (msg) {
        case 1:
            break;
        case 2:
            obj->name = "main";
            eneSetup(obj, arg);
            floadLoadSet(0xC6);
            w->body = (E61Part *) eneMakeUnder(obj, ENE_SANBA, 1, 0.0f, 0.0f, 0.0f);
            w->water = eneMakeUnder(obj, ENE_SANBA_WATER, 1, 0.0f, 0.0f, 0.0f);
            w->pldemo = eneMakeUnder(obj, ENE_SANBA_PLDEMO, 1, 3890.0f, 32.0f, 0.0f);
            w->chase = eneMakeUnder(obj, ENE_SANBA_CHASE, 1, 3850.0f, -50.0f, 0.0f);
            w->gauge = tskCreate(obj, pipGauge_proc, 0, NULL);
            w->life = 50;
            pipGaugeSetTarget(w->gauge, 50);
            eneCounterReward(1, 3, 480.0f, 692.0f, 0.0f, 3, 520.0f, 692.0f, 0.0f);
            if ((w->jellyfish[0] = eneMakeUnder(obj, ENE_SANBA_JELLYFISH, 1, 0.0f, 0.0f, 0.0f)) != NULL) {
                sanbaJellyfishSetPlayer(w->jellyfish[0], 0);
            }
            if ((w->jellyfish[1] = eneMakeUnder(obj, ENE_SANBA_JELLYFISH, 1, 0.0f, 0.0f, 0.0f)) != NULL) {
                sanbaJellyfishSetPlayer(w->jellyfish[0], 1); /* the game's: the first one again */
            }
            w->camDist = 160.0f;
            w->camHeight = 40.0f;
            w->camAngle = -60.0f;
            w->camAtY = 0.0f;
            sanbaPlayersMiddle(&pos);
            sanbaCamEyeAround(&pos, &eye, w->camDist, w->camHeight, w->camAngle);
            camSetAt(cam, pos.x, pos.y, pos.z);
            camSetEye(cam, eye.x, eye.y, eye.z);
            w->round = 0;
            /* The game's two dead prints, their strings next in its .rodata; their place and
             * arguments are not in the code. */
            PRINTF("@@@@@@@@@@@@@@@ COUNTER = %x\n @@@@@@@@@@@");
            PRINTF("ikada0=%x, ikada1=%x, ikada2=%x, ikada3=%x, ikada4=%x, ikada5=%x\n");
            if (eneCounterGet(1) == 0) {
                w->battleBgm = 0;
                w->taiBgm = sanbaTaiBgmCreate(obj, (void *) 0);
                w->state = 0;
            } else {
                tskDestroy(w->pldemo);
                w->pldemo = NULL;
                tskDestroy(w->chase);
                w->chase = NULL;
                w->battleBgm = 1;
                w->taiBgm = sanbaTaiBgmCreate(obj, (void *) 1);
                eneCounterIncrement(1);
                w->state = 11;
            }
            break;
        case 10:
            if ((s32) arg == E61_ID) {
                return (s32) obj;
            }
            break;
        case 0:
            eneStepHookPre(obj);
            switch (w->state) {
                case 0:
                    w->camDist = 100.0f;
                    w->camHeight = 40.0f;
                    w->camAngle = -60.0f;
                    w->camAtY = 0.0f;
                    sanbaPlayersMiddle(&pos);
                    sanbaCamEyeAround(&pos, &eye, w->camDist, w->camHeight, w->camAngle);
                    camSetAt(cam, pos.x, pos.y, pos.z);
                    camSetEye(cam, eye.x, eye.y, eye.z);
                    w->state = 1;
                    break;
                case 1:
                    sanbaPlayersMiddle(&pos);
                    if (pos.y < 180.0) {
                        w->camHeight = 40.0f;
                    } else if (pos.y < 380.0) {
                        w->camHeight = (pos.y - 380.0f) / 20.0f + 40.0f;
                    } else if (pos.y < 580.0) {
                        w->camHeight = (580.0f - pos.y) / 10.0f + 30.0f;
                    } else {
                        w->camHeight = 30.0f;
                    }
                    sanbaCamEyeAround(&pos, &eye, w->camDist, w->camHeight, w->camAngle);
                    camSetSpeed(cam, 4.0f);
                    camMoveAt(cam, pos.x, pos.y, pos.z);
                    camMoveEye(cam, eye.x, eye.y, eye.z);
                    if (sanbaPldemoIsState2(w->pldemo)) {
                        w->state = 2;
                    }
                    if (sanbaPldemoIsState11(w->pldemo)) {
                        w->state = 5;
                    }
                    break;
                case 2:
                    w->timer = 100;
                    w->camDist = 270.0f;
                    w->camHeight = 12.0f;
                    w->camAngle = 0.0f;
                    w->camAtY = 35.0f;
                    sanbaPlayersMiddle(&pos);
                    sanbaCamEyeAround(&pos, &eye, w->camDist, w->camHeight, w->camAngle);
                    camSetSpeed(cam, w->timer);
                    camMoveAt(cam, 0.0f, w->camAtY + 80.0f, 0);
                    camMoveEye(cam, eye.x, eye.y, eye.z);
                    w->state = 3;
                    break;
                case 3:
                    if (--w->timer == 0) {
                        sndPlay(0x16);
                        sndPlay(0x16D);
                    }
                    if (sanbaPldemoIsState3(w->pldemo)) {
                        camSetAt(cam, 38.0f, 46.0f, -68.0f);
                        camSetEye(cam, -93.0f, 115.0f, -170.0f);
                        w->state = 4;
                    }
                    break;
                case 4:
                    if (sanbaPldemoIsState5(w->pldemo)) {
                        sndPlay(0xA5);
                        w->state = 0;
                    }
                    break;
                case 5:
                    w->timer = 80;
                    w->camDist = 205.0f;
                    w->camHeight = 25.0f;
                    w->camAngle = 0.0f;
                    w->camAtY = 32.0f;
                    gGameWork.scenarioRunning = 2;
                    eneCounterIncrement(1);
                    sanbaPlayersMiddle(&pos);
                    pos.y = 685.0f;
                    sanbaCamEyeAround(&pos, &eye, w->camDist, w->camHeight, w->camAngle);
                    camSetSpeed(cam, w->timer);
                    camMoveAt(cam, 0.0f, w->camAtY + 685.0f, 0);
                    camMoveEye(cam, eye.x, eye.y, eye.z);
                    w->state = 6;
                case 6:
                    E61_SKIP_CHECK(w);
                    if (w->timer-- <= 0) {
                        sanbaSetState1(w->body);
                        w->timer = 15;
                        w->state = 7;
                    }
                    if (w->timer == 40) {
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                plAskPos(gGameWork.players[i], &pos.x, &pos.y, &pos.z, &zone);
                                pos.x = 100.0f;
                                plSetPos(gGameWork.players[i], 1, &pos);
                            }
                        }
                    }
                    break;
                case 7:
                    E61_SKIP_CHECK(w);
                    if (w->timer-- == 0) {
                        sndPlay(0xB5);
                    }
                    if (sanbaIsState5(w->body)) {
                        w->parts[0] = eneMakeUnder(obj, ENE_SANBA_IKADA, 2, 208.75f, 635.0f, 0.0f);
                        w->parts[1] = eneMakeUnder(obj, ENE_SANBA_IKADA, 2, 307.5f, 635.0f, 0.0f);
                        w->parts[2] = eneMakeUnder(obj, ENE_SANBA_IKADA, 2, 406.25f, 635.0f, 0.0f);
                        w->parts[3] = eneMakeUnder(obj, ENE_SANBA_IKADA, 2, 603.75f, 635.0f, 0.0f);
                        w->parts[4] = eneMakeUnder(obj, ENE_SANBA_IKADA, 2, 702.5f, 635.0f, 0.0f);
                        w->parts[5] = eneMakeUnder(obj, ENE_SANBA_IKADA, 2, 801.25f, 635.0f, 0.0f);
                        w->timer = 40;
                        w->state = 8;
                    }
                    break;
                case 8:
                    E61_SKIP_CHECK(w);
                    if (w->timer-- <= 0) {
                        w->camDist = 170.0f;
                        w->timer = 120;
                        w->camHeight = 27.0f;
                        w->camAngle = 30.0f;
                        w->camAtY = 38.0f;
                        sanbaPlayersMiddle(&pos);
                        pos.y = 685.0f;
                        sanbaCamEyeAround(&pos, &eye, w->camDist, w->camHeight, w->camAngle);
                        camSetAt(cam, 0.0f, w->camAtY + pos.y, 0);
                        camSetEye(cam, eye.x, eye.y, eye.z);
                        scRunStart2(8, 0, 0);
                        w->state = 9;
                    }
                    break;
                case 9:
                    E61_SKIP_CHECK(w);
                    w->camAngle = w->camAngle - 0.5;
                    sanbaPlayersMiddle(&pos);
                    pos.y = 685.0f;
                    sanbaCamEyeAround(&pos, &eye, w->camDist, w->camHeight, w->camAngle);
                    camSetEye(cam, eye.x, eye.y, eye.z);
                    if (w->timer-- <= 0) {
                        for (j = 0; j < 4; j++) {
                            if (plIsInPlay(j)) {
                                plAskPos(gGameWork.players[j], &pos.x, &pos.y, &pos.z, &zone);
                                pos.x = 510 - j * 5 * 4;
                                plSetPos(gGameWork.players[j], 2, &pos);
                            }
                        }
                        w->state = 10;
                    }
                    break;
                case 10:
                    gGameWork.savedSpawn.on = 1;
                    gGameWork.savedSpawn.angle = 2;
                    gGameWork.savedSpawn.x = 510.0f;
                    gGameWork.savedSpawn.y = 692.0f;
                    gGameWork.savedSpawn.z = 0.0f;
                    gGameWork.savedSpawn.facing = eneCourseAngle(obj, 1);
                    sanbaPldemoSetState12(w->pldemo);
                    w->state = 12;
                    break;
                case 11:
                    fadeSetLetterbox(320, 0, 0, 240);
                    scRunEnd();
                    w->camDist = 250.0f;
                    w->camHeight = 35.0f;
                    w->camAngle = -15.0f;
                    w->camAtY = 25.0f;
                    sanbaPlayersMiddle(&pos);
                    pos.y = 685.0f;
                    sanbaCamEyeAround(&pos, &eye, w->camDist, w->camHeight, w->camAngle);
                    camSetAt(cam, 0.0f, w->camAtY + 685.0f, 0.0f);
                    camSetEye(cam, eye.x, eye.y, eye.z);
                    if (w->pldemo != NULL) {
                        tskDestroy(w->pldemo);
                    }
                    if (w->chase != NULL) {
                        tskDestroy(w->chase);
                    }
                    for (k = 0; k < 4; k++) {
                        if (plIsInPlay(k)) {
                            plSetAction(gGameWork.players[k], 0);
                            pl = PL(gGameWork.players[k]);
                            pl->tilt = 0.0f;
                            plAskPos(gGameWork.players[k], &pos.x, &pos.y, &pos.z, &zone);
                            pos.x = 510 - k * 5 * 4;
                            pos.y = 692.0f;
                            plSetPos(gGameWork.players[k], 2, &pos);
                        }
                    }
                    if (w->parts[0] == NULL) {
                        w->parts[0] = eneMakeUnder(obj, ENE_SANBA_IKADA, 2, 208.75f, 680.0f, 0.0f);
                        sanbaIkadaSetFloatHeight(w->parts[0]);
                    }
                    if (w->parts[1] == NULL) {
                        w->parts[1] = eneMakeUnder(obj, ENE_SANBA_IKADA, 2, 307.5f, 680.0f, 0.0f);
                        sanbaIkadaSetFloatHeight(w->parts[1]);
                    }
                    if (w->parts[2] == NULL) {
                        w->parts[2] = eneMakeUnder(obj, ENE_SANBA_IKADA, 2, 406.25f, 680.0f, 0.0f);
                        sanbaIkadaSetFloatHeight(w->parts[2]);
                    }
                    if (w->parts[3] == NULL) {
                        w->parts[3] = eneMakeUnder(obj, ENE_SANBA_IKADA, 2, 603.75f, 680.0f, 0.0f);
                        sanbaIkadaSetFloatHeight(w->parts[3]);
                    }
                    if (w->parts[4] == NULL) {
                        w->parts[4] = eneMakeUnder(obj, ENE_SANBA_IKADA, 2, 702.5f, 680.0f, 0.0f);
                        sanbaIkadaSetFloatHeight(w->parts[4]);
                    }
                    if (w->parts[5] == NULL) {
                        w->parts[5] = eneMakeUnder(obj, ENE_SANBA_IKADA, 2, 801.25f, 680.0f, 0.0f);
                        sanbaIkadaSetFloatHeight(w->parts[5]);
                    }
                    gGameWork.savedSpawn.on = 1;
                    gGameWork.savedSpawn.angle = 2;
                    gGameWork.savedSpawn.x = 510.0f;
                    gGameWork.savedSpawn.y = 692.0f;
                    gGameWork.savedSpawn.z = 0.0f;
                    gGameWork.savedSpawn.facing = eneCourseAngle(obj, 1);
                    if (w->battleBgm == 0) {
                        sanbaTaiBgmRestart(w->taiBgm, 1);
                    }
                    sanbaMainShowGauge();
                    sanbaWaterSetMaxHeight();
                    sanbaSetState6(w->body);
                    w->state = 12;
                    gGameWork.scenarioRunning = 1;
                    eneRumbleStopAll();
                    break;
                case 12:
                    w->camDist = 250.0f;
                    w->camHeight = 35.0f;
                    w->camAngle = -15.0f;
                    w->camAtY = 25.0f;
                    sanbaPlayersMiddle(&pos);
                    pos.y = 685.0f;
                    sanbaCamEyeAround(&pos, &eye, w->camDist, w->camHeight, w->camAngle);
                    camSetAt(cam, 0.0f, w->camAtY + 685.0f, 0.0f);
                    camSetEye(cam, eye.x, eye.y, eye.z);
                    sanbaMakeWinches(obj, 0);
                    w->state = 13;
                    break;
                case 13:
                    sanbaPlayersMiddle(&pos);
                    pos.y = 685.0f;
                    sanbaCamEyeAround(&pos, &eye, w->camDist, w->camHeight, w->camAngle);
                    camSetSpeed(cam, 4.0f);
                    camMoveAt(cam, 0.0f, w->camAtY + 685.0f, 0.0f);
                    camMoveEye(cam, eye.x, eye.y, eye.z);
                    if (w->slots[0] == 0 && w->slots[1] == 0 && w->slots[2] == 0 && w->slots[3] == 0 &&
                        w->slots[4] == 0 && w->slots[5] == 0) {
                        if (w->round & 1) {
                            w->life -= 10;
                            w->state = 18;
                        } else {
                            w->life -= 10;
                            w->state = 15;
                        }
                        if (w->life < 0) {
                            w->life = 0;
                        }
                    }
                    break;
                case 15:
                    sndPlay(0x291);
                    w->drop = eneMakeUnder(obj, ENE_SANBA_TARAI, 1, 0.0f, 0.0f, 0.0f);
                    sanbaSetDropState(w->body, 0);
                    h = sanbaTaraiGetHeight(w->drop);
                    sanbaPlayersMiddle(&pos);
                    pos.y = 685.0f;
                    sanbaCamEyeAround(&pos, &eye, w->camDist, w->camHeight, w->camAngle);
                    camSetAt(cam, 0.0f, h, 0.0f);
                    camSetEye(cam, eye.x, eye.y, eye.z);
                    w->state = 16;
                    break;
                case 16:
                    if (w->drop != NULL) {
                        f32 at = sanbaTaraiGetHeight(w->drop);

                        camSetAt(cam, 0.0f, at, 0.0f);
                        sanbaPlayersMiddle(&pos);
                        pos.y = 685.0f;
                        sanbaCamEyeAround(&pos, &eye, w->camDist, w->camHeight, w->camAngle);
                        camSetSpeed(cam, 4.0f);
                        camMoveEye(cam, eye.x, eye.y, eye.z);
                    } else {
                        w->timer = 60;
                        camSetSpeed(cam, w->timer);
                        camMoveAt(cam, 0.0f, w->camAtY + 685.0f, 0.0f);
                        w->state = 17;
                    }
                    break;
                case 17:
                    sanbaPlayersMiddle(&pos);
                    pos.y = 685.0f;
                    sanbaCamEyeAround(&pos, &eye, w->camDist, w->camHeight, w->camAngle);
                    camSetSpeed(cam, 4.0f);
                    camMoveEye(cam, eye.x, eye.y, eye.z);
                    if (sanbaIsState33(w->body)) {
                        if (w->life > 0) {
                            w->round++;
                            sanbaMakeWinches(obj, w->round);
                            w->state = 13;
                        } else {
                            w->state = 21;
                        }
                    }
                    break;
                case 18:
                    sndPlay(0x8147);
                    w->drop = eneMakeUnder(obj, ENE_SANBA_MARUTA, 1, 0.0f, 0.0f, 0.0f);
                    sanbaSetDropState(w->body, 1);
                    w->camAtY = 50.0f;
                    sanbaPlayersMiddle(&pos);
                    pos.y = 685.0f;
                    sanbaCamEyeAround(&pos, &eye, w->camDist, w->camHeight, w->camAngle);
                    camSetAt(cam, 0.0f, w->camAtY + 685.0f, 0.0f);
                    camSetEye(cam, eye.x, eye.y, eye.z);
                    w->timer = 120;
                    w->state = 19;
                    break;
                case 19:
                    if (w->drop != NULL) {
                        sanbaPlayersMiddle(&pos);
                        pos.y = 685.0f;
                        sanbaCamEyeAround(&pos, &eye, w->camDist, w->camHeight, w->camAngle);
                        camSetSpeed(cam, 4.0f);
                        camMoveEye(cam, eye.x, eye.y, eye.z);
                    } else {
                        w->timer = 60;
                        w->camAtY = 25.0f;
                        camSetSpeed(cam, w->timer);
                        camMoveAt(cam, 0.0f, w->camAtY + 685.0f, 0.0f);
                        w->state = 20;
                    }
                    break;
                case 20:
                    sanbaPlayersMiddle(&pos);
                    pos.y = 685.0f;
                    sanbaCamEyeAround(&pos, &eye, w->camDist, w->camHeight, w->camAngle);
                    camSetSpeed(cam, 4.0f);
                    camMoveEye(cam, eye.x, eye.y, eye.z);
                    if (sanbaIsState33(w->body)) {
                        if (w->life > 0) {
                            w->round++;
                            sanbaMakeWinches(obj, w->round);
                            w->state = 13;
                        } else {
                            w->state = 21;
                        }
                    }
                    break;
                case 21:
                    sanbaPlayersMiddle(&pos);
                    pos.y = 685.0f;
                    sanbaCamEyeAround(&pos, &eye, w->camDist, w->camHeight, w->camAngle);
                    camSetSpeed(cam, 4.0f);
                    camMoveAt(cam, 0.0f, w->camAtY + 685.0f, 0.0f);
                    camMoveEye(cam, eye.x, eye.y, eye.z);
                    break;
            }
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* out: the players' middle, the average of the models' places of every player in play. */
void sanbaPlayersMiddle(Vec3f *out) {
    s32 i;
    s32 n;
    f32 x, y, z;
    s32 pad;

    out->x = 0.0f;
    out->y = 0.0f;
    out->z = 0.0f;
    n = 0;
    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plGetModelPos(gGameWork.players[i], &x, &y, &z);
            n++;
            out->x += x;
            out->y += y;
            out->z += z;
        }
    }
    if (n == 0) {
        n = 1;
    }
    out->x /= n;
    out->y /= n;
    out->z /= n;
}

/* out: the point r from the origin, turned deg degrees from pos's own heading, dy above pos. */
void sanbaCamEyeAround(Vec3f *pos, Vec3f *out, f32 r, f32 dy, f32 deg) {
    s32 pad[3];
    f32 angle = mathAtan2(pos->x, pos->z) + deg / 180.0f * 3.1415927f;

    out->x = mathSin(angle) * r;
    out->y = pos->y + dy;
    out->z = mathCos(angle) * r;
}

/* Makes enemies of kind 0x1D under three to six of the enemy's parts, by n (0: parts 1, 3, 5;
 * 1: 0, 2, 3, 5; 2: 0, 1, 2, 3, 5; else all six), into its slots; the rest of the slots 0. */
void sanbaMakeWinches(Obj *o, s32 n) {
    E61Work *w = (E61Work *) (o + 1);

    if (n == 0) {
        w->slots[0] = (s32) eneMakeUnder(w->parts[1], ENE_SANBA_WINCH, 1, 0.0f, 0.0f, 0.0f);
        w->slots[1] = (s32) eneMakeUnder(w->parts[3], ENE_SANBA_WINCH, 1, 0.0f, 0.0f, 0.0f);
        w->slots[2] = (s32) eneMakeUnder(w->parts[5], ENE_SANBA_WINCH, 1, 0.0f, 0.0f, 0.0f);
        w->slots[3] = 0;
        w->slots[4] = 0;
        w->slots[5] = 0;
    } else if (n == 1) {
        w->slots[0] = (s32) eneMakeUnder(w->parts[0], ENE_SANBA_WINCH, 1, 0.0f, 0.0f, 0.0f);
        w->slots[1] = (s32) eneMakeUnder(w->parts[2], ENE_SANBA_WINCH, 1, 0.0f, 0.0f, 0.0f);
        w->slots[2] = (s32) eneMakeUnder(w->parts[3], ENE_SANBA_WINCH, 1, 0.0f, 0.0f, 0.0f);
        w->slots[3] = (s32) eneMakeUnder(w->parts[5], ENE_SANBA_WINCH, 1, 0.0f, 0.0f, 0.0f);
        w->slots[4] = 0;
        w->slots[5] = 0;
    } else if (n == 2) {
        w->slots[0] = (s32) eneMakeUnder(w->parts[0], ENE_SANBA_WINCH, 1, 0.0f, 0.0f, 0.0f);
        w->slots[1] = (s32) eneMakeUnder(w->parts[1], ENE_SANBA_WINCH, 1, 0.0f, 0.0f, 0.0f);
        w->slots[2] = (s32) eneMakeUnder(w->parts[2], ENE_SANBA_WINCH, 1, 0.0f, 0.0f, 0.0f);
        w->slots[3] = (s32) eneMakeUnder(w->parts[3], ENE_SANBA_WINCH, 1, 0.0f, 0.0f, 0.0f);
        w->slots[4] = (s32) eneMakeUnder(w->parts[5], ENE_SANBA_WINCH, 1, 0.0f, 0.0f, 0.0f);
        w->slots[5] = 0;
    } else {
        w->slots[0] = (s32) eneMakeUnder(w->parts[0], ENE_SANBA_WINCH, 1, 0.0f, 0.0f, 0.0f);
        w->slots[1] = (s32) eneMakeUnder(w->parts[1], ENE_SANBA_WINCH, 1, 0.0f, 0.0f, 0.0f);
        w->slots[2] = (s32) eneMakeUnder(w->parts[2], ENE_SANBA_WINCH, 1, 0.0f, 0.0f, 0.0f);
        w->slots[3] = (s32) eneMakeUnder(w->parts[3], ENE_SANBA_WINCH, 1, 0.0f, 0.0f, 0.0f);
        w->slots[4] = (s32) eneMakeUnder(w->parts[4], ENE_SANBA_WINCH, 1, 0.0f, 0.0f, 0.0f);
        w->slots[5] = (s32) eneMakeUnder(w->parts[5], ENE_SANBA_WINCH, 1, 0.0f, 0.0f, 0.0f);
    }
}

/* Clears every one of enemy 0xE61's six slots that holds v. */
void sanbaClearSlot(s32 v) {
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) E61_ID);

    if (o != NULL) {
        E61Work *w = (E61Work *) (o + 1);

        if (((E61Work *) (o + 1))->slots[0] == v) {
            ((E61Work *) (o + 1))->slots[0] = 0;
        }
        if (w->slots[1] == v) {
            w->slots[1] = 0;
        }
        if (w->slots[2] == v) {
            w->slots[2] = 0;
        }
        if (w->slots[3] == v) {
            w->slots[3] = 0;
        }
        if (w->slots[4] == v) {
            w->slots[4] = 0;
        }
        if (w->slots[5] == v) {
            w->slots[5] = 0;
        }
    }
}

void sanbaMainClearDrop(void) {
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) E61_ID);

    if (o != NULL) {
        ((E61Work *) (o + 1))->drop = 0;
    }
}

E61Part *sanbaMainGetBody(void) {
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) E61_ID);

    if (o != NULL) {
        return ((E61Work *) (o + 1))->body;
    }
    return NULL;
}

s32 sanbaMainGetLife(void) {
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) E61_ID);

    if (o != NULL) {
        return ((E61Work *) (o + 1))->life;
    }
    return -1;
}

/* Shows enemy 0xE61's gauge and sets it to its word 0x90. */
void sanbaMainShowGauge(void) {
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) E61_ID);

    if (o != NULL) {
        E61Work *w = (E61Work *) (o + 1);

        pipGaugeOn(((E61Work *) (o + 1))->gauge);
        pipGaugeSetTarget(w->gauge, w->life);
    }
}

void sanbaTaiBgmRestart(Obj *o, s32 v);

void sanbaMainPlayBgm(void) {
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) E61_ID);

    if (o != NULL) {
        ((E61Work *) (o + 1))->battleBgm = 1;
        sanbaTaiBgmRestart(((E61Work *) (o + 1))->taiBgm, 1);
    }
}

void sanbaMainClearChase(Obj *o) {
    TLB39_MAIN(o)->chase = 0;
}

void sanbaMainClearPldemo(Obj *o) {
    TLB39_MAIN(o)->pldemo = 0;
}

extern u8 sanbaPldemoOpeningAnim0[]; /* the opening's two animations */
extern u8 sanbaPldemoOpeningAnim1[];

s32 sanbaWaterMoveTo(f32 target, f32 speed);
void sanbaChaseStart(Obj *o);
void sanbaChaseSpeedUp(Obj *o);
s32 sanbaChaseIsDone(Obj *o);
void sanbaSetState6(E61Part *p);

/* "pldemo": the stage's player demos. The players walk in (state 0), the two models sink to 80
 * and bob while enemy 0x3D3D's water rises to 65 (2-3), rise away and start the chase (4), then
 * at the far end the players walk to 330, 692 (7-8), and on to 45, 692 (9-11); state 13 ends the
 * letterbox, shows enemy 0xE61's gauge and stops. */
s32 sanbaPldemoProc(Obj *obj, s16 msg, void *arg) {
    PldemoW *w = PLDEMO(obj);
    s32 i;
    s32 n;
    s32 pad;

    switch (msg) {
        case 2: {
            f32 x, z;
            f32 dist;

            obj->name = "pldemo";
            eneSetup(obj, arg);
            w->m[0] = eneNewAnime(obj, (u32) sanbaPldemoOpeningAnim0);
            w->m[0]->anim = 0;
            w->m[1] = eneNewAnime(obj, (u32) sanbaPldemoOpeningAnim1);
            w->m[0]->anim = 0; /* the game's: m[1]'s animation runs from the start */
            dist = vramCoursePointXZ(1, 3790.0f, 60.0f, &x, &z);
            w->m[0]->pos[0] = x;
            w->m[0]->pos[1] = 200.0f;
            w->m[0]->pos[2] = z;
            w->m[0]->rot[1] = 3.63860178f;
            w->m[1]->pos[0] = x;
            w->m[1]->pos[1] = 200.0f;
            w->m[1]->pos[2] = z;
            w->m[1]->rot[1] = 3.63860178f;
            break;
        }
        case 1:
            if (w->unused3C != NULL) {
                frmDestroy(w->unused3C);
            }
            if (w->m[0] != NULL) {
                frmDestroy(&w->m[0]->f);
            }
            if (w->m[1] != NULL) {
                frmDestroy(&w->m[1]->f);
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (eneNearestPlayerDist(obj) < 40.0) {
                        gGameWork.scenarioRunning = 2;
                        sndPlay(9);
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                Vec3f goal;

                                plSetAction(gGameWork.players[i], 0xDC);
                                goal.x = i * 5 * 4 + 3780;
                                goal.y = 68.0f;
                                goal.z = 0.0f;
                                plDemoWalkTo(gGameWork.players[i], goal, 2.0f);
                            }
                        }
                        w->state = 1;
                    }
                    break;
                case 1:
                    for (i = 0, n = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            if (plIsDemoWalking(gGameWork.players[i])) {
                                n++;
                            } else {
                                Player *pl = PL(gGameWork.players[i]);

                                pl->tilt = 1.5707964f;
                            }
                        }
                    }
                    if (n == 0) {
                        w->m[0]->anim = sanbaPldemoOpeningAnim0;
                        w->m[1]->anim = sanbaPldemoOpeningAnim1;
                        w->state = 2;
                    }
                    break;
                case 2:
                    w->m[0]->pos[1] = w->m[0]->pos[1] - 0.5;
                    if (w->m[0]->pos[1] <= 80.0f) {
                        w->m[0]->pos[1] = 80.0f;
                        w->phase = 0.0f;
                        scRunStart2(15, 0, 0);
                        w->state = 3;
                    }
                    w->m[1]->pos[1] = w->m[0]->pos[1];
                    eneAnimLoop(obj, w->m[0], 0.5f);
                    eneAnimeLoop(obj, w->m[1], 0.2f);
                    break;
                case 3: {
                    s32 r;

                    w->phase = w->phase + 0.0628318548202514704;
                    w->m[0]->pos[1] = mathSin(w->phase) * 2 + 80.0;
                    r = sanbaWaterMoveTo(65.0f, 0.6f);
                    if (!scIsRunning() && r) {
                        w->count = 0;
                        w->phase = 0.0f;
                        sndPlay(0x160);
                        w->state = 4;
                    }
                    w->m[1]->pos[1] = w->m[0]->pos[1];
                    eneAnimLoop(obj, w->m[0], 0.5f);
                    eneAnimeLoop(obj, w->m[1], 0.2f);
                    gGameWork.scenarioRunning = 2;
                    break;
                }
                case 4:
                    w->phase = w->phase + 0.1;
                    w->m[0]->pos[1] += w->phase;
                    w->m[1]->pos[1] += w->phase;
                    eneAnimLoop(obj, w->m[0], 0.5f);
                    eneAnimeLoop(obj, w->m[1], 0.2f);
                    if (w->count == 15) {
                        sndPlay(7);
                    }
                    w->count++;
                    if (w->m[0]->pos[1] > 150.0) {
                        sanbaChaseStart(obj->next);
                        frmDestroy(&w->m[0]->f);
                        frmDestroy(&w->m[1]->f);
                        w->m[0] = NULL;
                        w->m[1] = NULL;
                        w->state = 5;
                    }
                    break;
                case 5:
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            Player *pl;

                            plSetAction(gGameWork.players[i], 0);
                            pl = PL(gGameWork.players[i]);
                            pl->tilt = 0.0f;
                        }
                    }
                    gGameWork.scenarioRunning = 1;
                    w->state = 6;
                    break;
                case 6:
                    w->zone = 1;
                    w->x = 405.0f;
                    w->y = 665.0f;
                    w->z = 0.0f;
                    w->state = 7;
                case 7:
                    if (eneNearestPlayerDist(obj) < 40.0) {
                        eneEndPlayersPowerDown();
                        gGameWork.scenarioRunning = 2;
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                Vec3f goal2;

                                plSetAction(gGameWork.players[i], 0xDC);
                                goal2.x = 330.0f;
                                goal2.y = 692.0f;
                                goal2.z = 0.0f;
                                plDemoWalkTo(gGameWork.players[i], goal2, 2.0f);
                            }
                        }
                        if (sanbaChaseIsDone(obj->next)) {
                            w->state = 9;
                        } else {
                            w->state = 8;
                        }
                    }
                    break;
                case 8:
                    fadeMoveLetterbox(45, 185, 1);
                    for (i = 0, n = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            if (plIsDemoWalking(gGameWork.players[i])) {
                                n++;
                            } else {
                                Player *pl = PL(gGameWork.players[i]);

                                pl->tilt = 3.1415927f;
                            }
                        }
                    }
                    if (n == 0) {
                        sanbaChaseSpeedUp(obj->next);
                        w->state = 9;
                    }
                    break;
                case 9:
                    fadeMoveLetterbox(45, 185, 1);
                    if (sanbaChaseIsDone(obj->next)) {
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                Vec3f goal3;
                                Player *pl;

                                plSetAction(gGameWork.players[i], 0xDC);
                                pl = PL(gGameWork.players[i]);
                                pl->tilt = 0.0f;
                                goal3.x = 45.0f;
                                goal3.y = 692.0f;
                                goal3.z = 0.0f;
                                w->unused4C[i] = 0;
                                plDemoWalkTo(gGameWork.players[i], goal3, 2.0f);
                            }
                        }
                        w->count = 20;
                        w->state = 10;
                    }
                    break;
                case 10:
                    if (--w->count == 0) {
                        sndPlay(9);
                    }
                    for (i = 0, n = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            if (plIsDemoWalking(gGameWork.players[i])) {
                                n++;
                            }
                        }
                    }
                    if (n == 0) {
                        w->state = 11;
                    }
                    break;
                case 12:
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            Player *pl;

                            plSetAction(gGameWork.players[i], 0);
                            pl = PL(gGameWork.players[i]);
                            pl->tilt = 0.0f;
                        }
                    }
                    w->state = 13;
                    break;
                case 13:
                    if (fadeMoveLetterbox(0, 240, 2)) {
                        (*sanbaMainPlayBgm)();
                        sanbaMainShowGauge();
                        sanbaSetState6(sanbaMainGetBody());
                        gGameWork.scenarioRunning = 1;
                        sanbaMainClearPldemo(obj->parent);
                        return -1;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 sanbaPldemoIsState2(Obj *o) {
    if (ENE(o)->state == 2) {
        return 1;
    }
    return 0;
}

s32 sanbaPldemoIsState3(Obj *o) {
    if (ENE(o)->state == 3) {
        return 1;
    }
    return 0;
}

s32 sanbaPldemoIsState5(Obj *o) {
    if (ENE(o)->state == 5) {
        return 1;
    }
    return 0;
}

s32 sanbaPldemoIsState11(Obj *o) {
    if (ENE(o)->state == 11) {
        return 1;
    }
    return 0;
}

void sanbaPldemoSetState12(Obj *o) {
    ENE(o)->state = 12;
}

#define DPRINTF

/* acollAtkAddYZ and acollDmgAddYZ return their box (main declares them void). */
typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                    s32 a);

extern Gfx sanbaJellyfishDl[]; /* the jellyfish's display list */

u8 sanbaWaterIsElectric(void);

/* "jellyfish": an electric jellyfish riding with a player (player); its attack box is live while
 * enemy 0x3D3D's byte 0x44 is 1 and the player is swimming or diving. */
s32 sanbaJellyfishProc(Obj *obj, s16 msg, void *arg) {
    JellyfishW *w = JELLYFISH(obj);
    s32 pad;
    f32 x, y, z;

    switch (msg) {
        case 2:
            obj->name = "jellyfish";
            eneSetup(obj, arg);
            w->model = eneNewPol(obj, sanbaJellyfishDl);
            w->model->scale[2] = 0.01f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->dl2 = NULL;
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 2, 2, 0, 0)) != NULL) {
                w->atk->flags &= ~2;
                w->atk->kind = 120;
            }
            break;
        case 1:
            if (w->model != NULL) { frmDestroy(&w->model->f); }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            break;
        case 0:
            if (acollAtkWasHit(w->atk) == 1) {
                DPRINTF("BIRIBIRIBIRI!!!!!!!!\n");
                tskSendMsg((Obj *) w->atk->hitPower, 7, obj);
            }
            w->atk->flags &= ~2;
            if (plIsInPlay(w->player)) {
                plGetModelPos(gGameWork.players[w->player], &x, &y, &z);
                w->x = x;
                w->y = y;
                w->z = z;
                if (sanbaWaterIsElectric() == 1) {
                    if ((plIsSwimming(w->player) || plIsDiving(w->player)) && w->atk != NULL) {
                        w->atk->flags |= 2;
                    }
                }
            } else {
                w->x = 0.0f;
                w->y = 0.0f;
                w->z = 0.0f;
            }
            eneSetModel(obj, &w->model->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

void sanbaJellyfishSetPlayer(Obj *o, s32 v) {
    JELLYFISH(o)->player = v;
}

extern Gfx waterSetupDl[]; /* the water's setup display list (this file's data) */
extern u8 sanbaWaterTex[]; /* the water's 32x32 I8 texture, in the stage's segment */

/* "water"'s display list: its setup list, white with alpha a, the environment colour (r, g, b),
 * and one 32x32 I8 texture in two tiles, scrolled by t (tile 0 along both axes, tile 1 by
 * (t + 10, -t)). */
Gfx *sanbaWaterDl(Obj *o, Gfx *gfx, u16 t, u8 r, u8 g, u8 b, u8 a) {
    gDPPipeSync(gfx++);
    gSPDisplayList(gfx++, fadeSegToRam(waterSetupDl, o->file));
    gDPSetPrimColor(gfx++, 0, 0, 255, 255, 255, a);
    gDPSetEnvColor(gfx++, r, g, b, 128);
    gDPSetTextureImage(gfx++, G_IM_FMT_I, G_IM_SIZ_16b, 1, sanbaWaterTex); gDPSetTile(gfx++, G_IM_FMT_I, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD); gDPLoadSync(gfx++); gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, ((32 * 32 + 1) >> 1) - 1, CALC_DXT(32, G_IM_SIZ_8b_BYTES)); gDPPipeSync(gfx++); gDPSetTile(gfx++, G_IM_FMT_I, G_IM_SIZ_8b, ((32 * G_IM_SIZ_8b_LINE_BYTES) + 7) >> 3, 0, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD);
    gDPSetTileSize(gfx++, G_TX_RENDERTILE, t, t, (32 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC);
    gDPSetTextureImage(gfx++, G_IM_FMT_I, G_IM_SIZ_16b, 1, sanbaWaterTex); gDPSetTile(gfx++, G_IM_FMT_I, G_IM_SIZ_16b, 0, 0x100, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD); gDPLoadSync(gfx++); gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, ((32 * 32 + 1) >> 1) - 1, CALC_DXT(32, G_IM_SIZ_8b_BYTES));
    gDPPipeSync(gfx++); gDPSetTile(gfx++, G_IM_FMT_I, G_IM_SIZ_8b, ((32 * G_IM_SIZ_8b_LINE_BYTES) + 7) >> 3, 0x100, 1, 0, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD); gDPSetTileSize(gfx++, 1, t + 10, -t, (32 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC);
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* A second enemy this file finds through gEneManager, and what it reads of its work. */
#define E3D3D_ID 0x3D3D

typedef struct E3D3DWork {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y; /* its height */
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *model;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ s32 scroll; /* its texture's */
    /* 0x40 */ s32 step; /* added to scroll each draw */
    /* 0x44 */ u8 electric; /* 1: electrified (thunder, blinking colours) */
    /* 0x45 */ u8 pad45[0x3];
    /* 0x48 */ s32 blink;
    /* 0x4C */ s32 moving; /* set while sanbaWaterMoveTo moves it (sound 0x182) */
    /* 0x50 */ s32 timer; /* to the next thunder */
} E3D3DWork;

#define E3D3D(o) ((E3D3DWork *) ((o) + 1))

extern Gfx sanbaWaterModelDl[]; /* the water's model */
extern u8 sanbaWaterRide[]; /* its ride data */

void sanbaCThunderCreate(Obj *obj, s32 kind, f32 x, f32 y, f32 z);

/* "water" (enemy 0x3D3D): the water surface riders stand on, 60 below; while electrified
 * (electric) it throws a "cTHUNDER" every 5 steps and blinks yellow and blue. */
s32 sanbaWaterProc(Obj *obj, s16 msg, void *arg) {
    E3D3DWork *w = E3D3D(obj);

    switch (msg) {
        case 2:
            obj->name = "water";
            eneSetup(obj, arg);
            w->step = -1;
            w->model = eneNewPol(obj, sanbaWaterModelDl);
            w->ride = rideCreate(gRideTree2, (s32) obj, &w->model->f,
                                 (RideData *) eneFileAddr(obj, sanbaWaterRide, 0));
            if (w->model == NULL) {
                return -1;
            }
            frmMove(&w->model->f, gFrmLayers[6]);
            w->model->scale[2] = 0.1f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->timer = 0;
            w->x = 0.0f;
            w->z = 0.0f;
            w->y = -60.0f;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->ride != NULL) {
                frmDestroy((Frame *) w->ride);
            }
            break;
        case 10:
            if ((s32) arg == E3D3D_ID) {
                return (s32) obj;
            }
            break;
        case 0:
            if (w->electric != 0) {
                if (w->timer-- <= 0) {
                    w->timer = 4;
                    sanbaCThunderCreate(obj, 0, 0.0f, 785.0f, 0.0f);
                }
            }
            break;
        case 3:
            w->model->dl = gGfxDlHead;
            if (w->electric == 0) {
                w->blink = 0;
                gGfxDlHead = sanbaWaterDl(obj, gGfxDlHead, w->scroll, 16, 64, 255, 224);
            } else {
                s32 lit = ++w->blink & 2;

                if (lit) {
                    gGfxDlHead = sanbaWaterDl(obj, gGfxDlHead, w->scroll, 0, 48, 176, 192);
                } else {
                    gGfxDlHead = sanbaWaterDl(obj, gGfxDlHead, w->scroll, 192, 192, 48, 160);
                }
            }
            w->scroll += w->step;
            eneSetModel(obj, &w->model->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Sets enemy 0x3D3D's byte 0x44: sound 0x148 when it becomes 1, 0x8148 when 0. */
void sanbaWaterSetElectric(u8 v) {
    s32 pad[3];
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) E3D3D_ID);

    if (o != NULL) {
        ((E3D3DWork *) (o + 1))->electric = v;
        if (v == 1) {
            sndPlay(0x148);
        }
        if (v == 0) {
            sndPlay(0x8148);
        }
    }
}

u8 sanbaWaterIsElectric(void) {
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) E3D3D_ID);

    if (o != NULL) {
        return ((E3D3DWork *) (o + 1))->electric;
    }
    return 0;
}

/* Moves enemy 0x3D3D's height (word 0x14) toward target (685 at most) by speed; sound 0x182 as it
 * starts, 0x8182 as it stops. 1 once there.
 * Matched 2026-10-12, with sanbaUnusedStub below; it was parked for where o and the return
 * value are kept. */
s32 sanbaWaterMoveTo(f32 target, f32 speed) {
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) E3D3D_ID);
    s32 pad[2];

    if (o != NULL) {
        E3D3DWork *w = E3D3D(o);
        f32 d;

        if (685.0f < target) {
            target = 685.0f;
        }
        speed = fabsf(speed);
        d = target - w->y;
        if (d != 0.0f) {
            if (w->moving == 0) {
                w->moving = 1;
                sndPlay(0x182);
            }
        } else if (w->moving == 1) {
            w->moving = 0;
            sndPlay(0x8182);
        }
        w = E3D3D(o);
        if (speed < fabsf(d)) {
            if (d >= 0.0f) {
                w->y += speed;
            } else {
                w->y -= speed;
            }
            return 0;
        } else {
            w->y = target;
            return 1;
        }
    }
    return 0;
}

/* Empty; nothing calls it (the second jr ra at the end of sanbaWaterMoveTo's asm). */
void sanbaUnusedStub(void) {
}

void sanbaWaterSetMaxHeight(void) {
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) E3D3D_ID);

    if (o != NULL) {
        ((E3D3DWork *) (o + 1))->y = 685.0f;
    }
}

void sanbaMainClearDrop(void);
E61Part *sanbaMainGetBody(void);
void sanbaMainShowGauge(void);
void sanbaNextState(E61Part *p);

/* "tarai" (a washtub): falls from 1300; at 760 it hits enemy 0xE61 (its gauge shows, its part
 * counts up, a rumble) and bounces off a random way, spinning, until it is below 500. */
s32 sanbaTaraiProc(Obj *obj, s16 msg, void *arg) {
    TaraiW *w = TARAI(obj);

    switch (msg) {
        case 2:
            obj->name = "tarai";
            eneSetup(obj, arg);
            if ((w->model = eneNewPol(obj, sanbaJellyfishDl)) == NULL) {
                return -1;
            }
            w->z = 0.0f;
            w->x = 0.0f;
            w->y = 1300.0f;
            sndPlay(0x8147);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
        case 0:
            eneFall(obj, 0.4f);
            eneMove((EneObj *) w);
            switch (w->state) {
                case 0:
                    if (w->y <= 760.0f) {
                        w->angle = (f32) fadeRand(360) / 180.0 * 3.14159274101257324;
                        w->vel[0] = mathCos(w->angle) * 4.0;
                        w->vel[1] = 6.0f;
                        w->vel[2] = mathSin(w->angle) * 4.0;
                        sanbaMainShowGauge();
                        sndPlay(0x149);
                        eneRumbleAll(0xBBBBBBBB, 20);
                        sanbaMainClearDrop();
                        sanbaNextState(sanbaMainGetBody());
                        w->state = 1;
                    }
                    break;
                case 1:
                    w->rot[0] += mathSin(w->angle) * 2.0 / 180.0 * 3.14159274101257324;
                    w->rot[2] += mathCos(w->angle) * 2.0 / 180.0 * 3.14159274101257324;
                    if (w->y <= 500.0f) {
                        return -1;
                    }
                    break;
            }
            eneSetModel(obj, &w->model->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

f32 sanbaTaraiGetHeight(Obj *o) {
    return TARAI(o)->y;
}

/* "maruta" (the log, sanbaMarutaProc): its work. */
typedef struct MarutaW {
    /* 0x00 */ u8 state; /* 0 waiting, 1 falling over, 2 rising again */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rotX;
    /* 0x20 */ f32 rotY;
    /* 0x24 */ f32 rotZ;
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *ends[2]; /* its two ends' models */
    /* 0x3C */ f32 tilt; /* in degrees, 90 standing */
    /* 0x40 */ f32 speed; /* of the tilt */
    /* 0x44 */ s32 timer;
    /* 0x48 */ u8 hit; /* set while it is down on enemy 0xE61's part */
} MarutaW;

extern Gfx sanbaMarutaLogEndDl[]; /* the log's end */

f32 sanbaGetRotY(E61Part *p);
void sanbaMarutaPlaceEnds(Obj *o);

/* "maruta" (a log): stands at 90 degrees beside enemy 0xE61's part, 110 out across its heading
 * and 1190 up; after 30 steps it falls over onto it (a rumble, sound 0x103), counting the part up
 * at 8 degrees, waits 20 steps, rises again (sound 0x29C at 20 degrees) and goes at 45. */
s32 sanbaMarutaProc(Obj *obj, s16 msg, void *arg) {
    MarutaW *w = (MarutaW *) FX(obj);
    s32 pad;
    f32 a;

    switch (msg) {
        case 2:
            obj->name = "maruta";
            eneSetup(obj, arg);
            if ((w->ends[0] = eneNewPol(obj, sanbaMarutaLogEndDl)) == NULL) {
                return -1;
            }
            if ((w->ends[1] = eneNewPol(obj, sanbaMarutaLogEndDl)) == NULL) {
                return -1;
            }
            a = sanbaGetRotY(sanbaMainGetBody());
            w->rotY = a + 1.5707964f;
            w->rotX = 3.1415927f;
            w->tilt = 90.0f;
            w->x = mathSin(w->rotY) * 110.0f;
            w->y = 1190.0f;
            w->z = mathCos(a + 1.5707964f) * 110.0f;
            sanbaMarutaPlaceEnds(obj);
            sndPlay(0x8147);
            w->timer = 30;
            w->hit = 0;
            break;
        case 1:
            if (w->ends[0] != NULL) { frmDestroy(&w->ends[0]->f); }
            if (w->ends[1] != NULL) {
                frmDestroy(&w->ends[1]->f);
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (w->timer-- <= 0) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    w->speed = w->speed - 0.08;
                    if (w->speed <= -3.0) {
                        w->speed = -3.0f;
                    }
                    w->tilt += w->speed;
                    if (w->tilt <= 0.0f) {
                        eneRumbleAll(0xBBBBBBBB, 20);
                        sndPlay(0x103);
                        sanbaMainClearDrop();
                        w->timer = 20;
                        w->speed = 0.0f;
                        w->tilt = 0.0f;
                        sanbaMainShowGauge();
                        w->state = 2;
                    }
                    if (w->hit == 0 && w->tilt <= 8.0f) {
                        w->hit = 1;
                        sanbaNextState(sanbaMainGetBody());
                    }
                    break;
                case 2:
                    if (w->timer-- <= 0) {
                        w->speed = w->speed + 0.02;
                        if (1.0 <= w->speed) {
                            w->speed = 1.0f;
                        }
                        w->tilt += w->speed;
                        if (w->hit == 1 && 20.0f <= w->tilt) {
                            w->hit = 0;
                            sndPlay(0x29C);
                            sanbaNextState(sanbaMainGetBody());
                        }
                        if (45.0 <= w->tilt) {
                            return -1;
                        }
                    }
                    break;
            }
            w->rotX = mathSin(w->tilt / 180.0 * 3.14159274101257324);
            sanbaMarutaPlaceEnds(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Places the log's two end models 40 to either side of its middle, across its heading. */
void sanbaMarutaPlaceEnds(Obj *o) {
    MarutaW *w = (MarutaW *) FX(o);
    s32 pad;
    f32 a = w->rotY - 1.5707964f;
    f32 dx = mathSin(a) * 40.0f;
    f32 dz = mathCos(a) * 40.0f;

    w->ends[0]->rot[0] = -w->rotX;
    w->ends[0]->rot[1] = w->rotY;
    w->ends[0]->pos[0] = w->x + dx;
    w->ends[0]->pos[1] = w->y;
    w->ends[0]->pos[2] = w->z + dz;
    w->ends[1]->rot[0] = w->rotX;
    w->ends[1]->rot[1] = w->rotY;
    w->ends[1]->pos[0] = dx - w->x;
    w->ends[1]->pos[1] = w->y;
    w->ends[1]->pos[2] = dz - w->z;
}

/* "futa" (a lid): its work. */
typedef struct FutaW {
    /* 0x00 */ u8 state; /* 0 shut, 1 sliding open, 2 open */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rotX;
    /* 0x20 */ f32 rotY;
    /* 0x24 */ f32 rotZ;
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *model;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ f32 x0; /* where it started */
    /* 0x40 */ f32 slide; /* how far it has slid, up to 60 either way */
    /* 0x44 */ s32 dir; /* eneSetup's result: the way it slides (negative: back) */
} FutaW;

extern Gfx sanbaFutaLidDl[]; /* the lid's display list */
extern u8 sanbaFutaRide[]; /* its ride data */

/* "futa": a lid on the course that a rider can stand on; set to state 1, it slides 60 open, 10
 * a step. */
s32 sanbaFutaProc(Obj *obj, s16 msg, void *arg) {
    FutaW *w = (FutaW *) FX(obj);
    s32 i;
    s32 pad;
    RideData *data;

    switch (msg) {
        case 2:
            obj->name = "futa";
            w->dir = eneSetup(obj, arg);
            if ((w->model = eneNewPol(obj, sanbaFutaLidDl)) == NULL) {
                return -1;
            }
            w->x0 = w->x;
            data = (RideData *) eneFileAddr(obj, sanbaFutaRide, 0);
            if ((w->ride = rideCreate(gRideTree1, (s32) obj, &w->model->f, data)) == NULL) {
                return -1;
            }
            break;
        case 1:
            if (w->model != NULL) { frmDestroy(&w->model->f); }
            if (w->ride != NULL) {
                frmDestroy((Frame *) w->ride);
            }
            break;
        case 0:
            if (w->dir >= 0) {
                w->rotY = eneCourseAngle(obj, 1);
            } else {
                w->rotY = eneCourseAngle(obj, 4);
            }
            switch (w->state) {
                case 0:
                    for (i = 0; i < 4; i++) {
                        Vec3f pos;

                        if (plIsInPlay(i)) {
                            plGetModelPos(gGameWork.players[i], &pos.x, &pos.y, &pos.z);
                        }
                    }
                    break;
                case 1:
                    if (w->dir >= 0) {
                        w->slide += 10.0f;
                        if (60.0f <= w->slide) {
                            w->slide = 60.0f;
                        }
                        w->state = 2;
                    } else {
                        w->slide -= 10.0f;
                        if (w->slide <= 60.0f) { /* the game's test (-60 meant) */
                            w->slide = -60.0f;
                        }
                        w->state = 2;
                    }
                    w->x = w->x0 + w->slide;
                    break;
                case 2:
                    break;
            }
            eneSetModelOnCourse(obj, &w->model->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

extern u8 sanbaChaseAnim[]; /* the chaser's model (animation) */
extern u8 sanbaChaseRide[]; /* its ride data */

void sanbaMainClearChase(Obj *o);

/* "chase": the thing that chases the players up the course; its riders move with it (message
 * 6). Set to state 1 (sanbaChaseStart) it rises from the water, then runs back along the
 * course; hit from the front it bounces (5-7), on the back (8-9); below 405 it is caught by a
 * spark (10-12) and sinks while enemy 0x3D3D's water goes back to 685.
 * Matched 2026-10-17; it was parked for registers: state 10 keeps w->model's load (the first
 * compare and the sound's arguments) in s0, where IDO's CSE range for the same expression
 * always wins the tie for v0 instead, since it crosses no call. The game's s0 there is `m`
 * itself (a variable's range colours to s0 wherever it is), which needs a fresh use of its own
 * after the sound call: the model's and the spark's heights as block-scoped float locals,
 * reassigned only on the sound's path (so the skip path keeps them in f0/f2), give it one. No
 * FAKE. */
s32 sanbaChaseProc(Obj *obj, s16 msg, void *arg) {
    ChaseW *w = CHASE(obj);
    AnimeFrame *m;
    ACollFrame *box;
    f32 f; /* state 4's bob scale, state 10's model height */

    switch (msg) {
        case 2:
            obj->name = "chase";
            eneSetup(obj, arg);
            if ((w->model = eneNewAnime(obj, (u32) sanbaChaseAnim)) == NULL) {
                return -1;
            }
            if ((w->ride = rideCreate(gRideTree1, (s32) obj, &w->model->f,
                                      (RideData *) eneFileAddr(obj, sanbaChaseRide, 0))) == NULL) {
                return -1;
            }
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 200, 200, 0, 0)) == NULL) {
                return -1;
            }
            box = w->atk;
            box->flags &= ~2;
            w->atk->kind = 120;
            if ((w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->model->f, 100, 200, 0, 0, 0)) ==
                NULL) {
                return -1;
            }
            w->dmg->flags &= ~2;
            w->dmg->kind = 101;
            w->speed = 2.8f;
            break;
        case 1:
            if ((m = w->model) != NULL) { frmDestroy(&m->f); }
            if (w->ride != NULL) {
                frmDestroy((Frame *) w->ride);
            }
            if ((box = w->atk) != NULL) {
                frmDestroy(&box->frame);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            if (w->spark != NULL) {
                frmDestroy(&w->spark->f);
            }
            sndPlay(0x8182);
            break;
        case 6:
            ((f32 *) arg)[1] += w->x - w->prev[0];
            ((f32 *) arg)[3] += w->y - w->prev[1];
            break;
        case 0:
            w->prev[0] = w->x;
            w->prev[1] = w->y;
            w->prev[2] = w->z;
            if ((box = w->atk) != NULL && acollAtkWasHit(box) == 1) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, obj);
                if (w->state < 5) {
                    w->state = 5;
                }
            }
            if (w->dmg != NULL && acollDmgWasHit(w->dmg) == 1 && w->state < 8) {
                m = w->model;
                SoundLocation2(0x14F, gGameWork.view, m->pos[0], m->pos[1], m->pos[2], 0.0f, 0.0f,
                               gGameWork.view->fovy);
                w->state = 8;
            }
            switch (w->state) {
                case 0:
                    break;
                case 1:
                    w->rotY = eneCourseAngle(obj, 8);
                    w->model->anim = sanbaChaseAnim;
                    w->model->frame = 0.0f;
                    w->state = 2;
                case 2:
                    eneAccelAxis(67.0f, 4.0f, 0.4f, &w->y, &w->vy);
                    if (70.0f < w->y && w->flag == 0) {
                        eneRumbleAll(0xBBBBBBBB, 20);
                        w->flag = 1;
                        m = w->model;
                        SoundLocation2(0x29C, gGameWork.view, m->pos[0], m->pos[1], m->pos[2], 0.0f, 0.0f,
                                       gGameWork.view->fovy);
                        m = w->model;
                        effectSibukiCreateWhite(m->pos[0], 85.0f, m->pos[2], 0.1f, 0.1f, 0.02f, 0.02f);
                    }
                    if (w->vy < 0.0f && w->y < 70.0f) {
                        w->y = 70.0f;
                        w->state = 3;
                        m = w->model; effectSibukiCreateWhite(m->pos[0], 85.0f, m->pos[2], 0.1f, 0.1f, 0.02f, 0.02f);
                    }
                    sanbaWaterMoveTo(85.0f, 0.5f);
                    eneAnimLoop(obj, w->model, 0.6f);
                    break;
                case 3:
                    m = w->model;
                    SoundLocation2(0x2B7, gGameWork.view, m->pos[0], m->pos[1], m->pos[2], 0.0f, 0.0f,
                                   gGameWork.view->fovy);
                    w->vy = 0.0f;
                    box = w->atk;
                    box->flags |= 2;
                    w->dmg->flags |= 2;
                    w->state = 4;
                case 4:
                    w->phase = w->phase + 0.139626344045003264;
                    if (6.2831855f <= w->phase) {
                        w->phase -= 6.2831855f;
                    }
                    f = mathSin(w->phase) * 0.01 + 0.1;
                    w->model->scale[1] = f;
                    w->model->scale[2] = f;
                    w->model->scale[0] = f;
                    eneAnimLoop(obj, w->model, 0.4f);
                    if (150.0 < eneNearestPlayerDist(obj)) {
                        w->vx = -(w->speed * 5.0f);
                    } else {
                        w->vx = -w->speed;
                    }
                    if (w->x <= 440.0f) {
                        w->speed = 2.8f;
                    }
                    if (w->x <= 405.0f) {
                        if ((w->spark = eneNewPol(obj, sanbaJellyfishDl)) != NULL) {
                            floadGetSetRam((void **) w->spark->segs, 0x26);
                            w->spark->scale[2] = 0.02f;
                            w->spark->scale[0] = w->spark->scale[1] = w->spark->scale[2];
                            w->spark->pos[0] = w->model->pos[0];
                            w->spark->pos[1] = w->model->pos[1] + 100.0f;
                            w->spark->pos[2] = w->model->pos[2];
                        }
                        w->state = 10;
                    }
                    sanbaWaterMoveTo(w->y + 15.0f, 2.0f);
                    eneMove((EneObj *) w);
                    eneLand((EneObj *) w, 15.0f);
                    eneTurnAlongCourse(obj, 8.0f);
                    break;
                case 5:
                    w->vx = w->vx * -0.5f;
                    w->state = 6;
                case 6:
                    w->vx = w->vx / 1.05f;
                    if (fabsf(w->vx) < 0.1f) {
                        w->timer = 20;
                        w->state = 7;
                    }
                    eneMove((EneObj *) w);
                    eneLand((EneObj *) w, 15.0f);
                    break;
                case 7:
                    if (w->timer-- <= 0) {
                        w->state = 3;
                        w->vx = -w->speed;
                    }
                    break;
                case 8:
                    w->vx = w->speed * 0.5f;
                    w->state = 9;
                case 9:
                    w->vx = w->vx / 1.04f;
                    if (fabsf(w->vx) < 0.1f) {
                        w->flag = 0;
                        w->state = 3;
                        w->vx = -w->speed;
                    }
                    eneMove((EneObj *) w);
                    eneLand((EneObj *) w, 15.0f);
                    break;
                case 10:
                    sndPlay(0x8182);
                    w->spark->pos[1] -= 3.0f;
                    m = w->model;
                    f = m->pos[1];
                    if (w->spark->pos[1] <= f + 40.0f && w->flag == 0) {
                        w->flag = 1;
                        soundIncamCam(0x14F, m->pos[0], m->pos[1] + 10.0f, m->pos[2]);
                        f = w->model->pos[1];
                    }
                    if (w->spark->pos[1] <= f + 35.0f) {
                        w->spark->dl2 = NULL;
                        w->state = 11;
                    }
                    break;
                case 11:
                    m = w->model;
                    soundIncamCam(0x11B, m->pos[0], m->pos[1] + 10.0f, m->pos[2]);
                    m = w->model;
                    itexSparkleYellow(m->pos[0], m->pos[1] + 20.0f, m->pos[2], 0.35f);
                    m = w->model;
                    ptclSpawnSparksFall(m->pos[0], m->pos[1] + 20.0f, m->pos[2]);
                    eneRumbleAll(-1, 20);
                    w->model->anim = 0;
                    w->state = 12;
                case 12:
                    if (sanbaWaterMoveTo(685.0f, 0.5f)) {
                        sndPlay(0x8182);
                        sanbaMainClearChase(obj->parent);
                        return -1;
                    }
                    break;
            }
            eneSetModelOnCourse(obj, &w->model->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

void sanbaChaseStart(Obj *o) {
    CHASE(o)->state = 1;
}

void sanbaChaseSpeedUp(Obj *o) {
    if (o != NULL) {
        CHASE(o)->speed = 14.0f;
    }
}

/* True once the chase reaches state 11, or when there is none. */
s32 sanbaChaseIsDone(Obj *o) {
    if (o != NULL) {
        if (CHASE(o)->state >= 11) {
            return 1;
        }
        return 0;
    }
    return 1;
}

extern Gfx sanbaIkadaRaftDl[]; /* the raft's model */
extern u8 sanbaIkadaRide[]; /* its ride data */

/* "ikada" (a raft): floats at 683 on the course; a rider (message 6) rides its bob and sinks
 * it to 680 (no lower than 679.5); message 7 answers 3. */
s32 sanbaIkadaProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    IkadaW *w = IKADA(obj);

    switch (msg) {
        case 2:
            obj->name = "ikada";
            eneSetup(obj, arg);
            if ((w->model = eneNewPol(obj, sanbaIkadaRaftDl)) == NULL) {
                DPRINTF("!!!!!!!   IKADA POL MAKE FALD !!!!!!!\n");
                return -1;
            }
            if ((w->ride = rideCreate(gRideTree1, (s32) obj, &w->model->f,
                                      (RideData *) eneFileAddr(obj, sanbaIkadaRide, 0))) == NULL) {
                DPRINTF("!!!!!!!   IKADA BOX MAKE FALD !!!!!!!\n");
                return -1;
            }
            w->vy = 0.5f;
            break;
        case 1:
            DPRINTF("!!!!!!!   IKADA DESTROY :%x  !!!!!!!\n", obj);
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->ride != NULL) {
                frmDestroy((Frame *) w->ride);
            }
            break;
        case 6:
            w->riders++;
            ((f32 *) arg)[3] += w->vy;
            break;
        case 7:
            if (arg != NULL) {
                *(s32 *) arg = 3;
            }
            break;
        case 0:
            w->rotY = eneCourseAngle(obj, 1);
            switch (w->state) {
                case 0:
                    eneAccelAxis(683.0f, 0.6f, 0.1f, &w->y, &w->vy);
                    if (w->y <= 683.0f && w->vy < 0.0f) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (w->riders != 0) {
                        eneAccelAxis(680.0f, 0.1f, 0.03f, &w->y, &w->vy);
                        if (w->y < 679.5) {
                            w->y = 679.5f;
                        }
                        w->riders = 0;
                    } else {
                        eneAccelAxis(683.0f, 0.1f, 0.02f, &w->y, &w->vy);
                    }
                    break;
            }
            eneSetModelOnCourse(obj, &w->model->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

f32 sanbaIkadaGetVelY(Obj *o) {
    return IKADA(o)->vy;
}

void sanbaIkadaSetFloatHeight(Obj *o) {
    IKADA(o)->y = 683.0f;
}

/* "winch"'s work. */
typedef struct WinchW {
    /* 0x00 */ u8 state; /* 0 fading in, 1 standing (hit it 4 times), 2 broken */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x; /* its parent's (the raft's) place, 5 up */
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rotX;
    /* 0x20 */ f32 rotY;
    /* 0x24 */ f32 rotZ;
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *model;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ ACollFrame *dmg;
    /* 0x40 */ u8 alpha;
    /* 0x41 */ u8 fading; /* drawn see-through while 1 */
    /* 0x42 */ u8 pad42[0x2];
    /* 0x44 */ s32 timer;
    /* 0x48 */ s32 hitTimer; /* blinks it after a hit */
    /* 0x4C */ s32 hits;
} WinchW;

extern Gfx winchXluDl[]; /* the winch's see-through setup list (this file's data) */
extern Gfx sanbaWinchDl[]; /* the winch's model */
extern u8 sanbaWinchRide[]; /* its ride data */

f32 sanbaIkadaGetVelY(Obj *o);

/* "winch": stands on its parent (the raft) and fades in; four hits break it (sparkles, sound
 * 0x11B) and 20 steps later it clears itself from enemy 0xE61's slots and goes. The ride data
 * and the height are read into locals before use (IDO_NOTES 358: each is a local word, not a
 * spill temp), which puts case 3's spilled display-list pointer at the game's 0x54. */
s32 sanbaWinchProc(Obj *obj, s16 msg, void *arg) {
    WinchW *w = (WinchW *) FX(obj);
    s32 pad[2];
    Vec3f pos;
    RideData *data;
    f32 height;

    switch (msg) {
        case 2:
            obj->name = "winch";
            eneSetup(obj, arg);
            if ((w->model = eneNewPol(obj, sanbaWinchDl)) == NULL) {
                return -1;
            }
            frmMove(&w->model->f, gFrmEffectLayer);
            w->model->rot[1] = eneCourseAngle(obj, 8);
            data = (RideData *) eneFileAddr(obj, sanbaWinchRide, 0);
            if ((w->ride = rideCreate(gRideTree1, (s32) obj, &w->model->f, data)) == NULL) {
                return -1;
            }
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->model->f, 40, 100, 0, 0, 0);
            w->dmg->flags &= ~2;
            if (w->dmg == NULL) {
                return -1;
            }
            w->alpha = 0;
            w->fading = 1;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->ride != NULL) {
                frmDestroy((Frame *) w->ride);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            break;
        case 6:
            height = sanbaIkadaGetVelY(obj->parent);
            ((f32 *) arg)[3] += height;
            break;
        case 3:
            if (w->fading) {
                w->model->dl = gGfxDlHead;
                gSPDisplayList(gGfxDlHead++, fadeSegToRam(winchXluDl, obj->file));
                gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
                gSPEndDisplayList(gGfxDlHead++);
            } else {
                if (w->hitTimer & 2) {
                    w->alpha = 192;
                } else {
                    w->alpha = 0;
                }
                w->model->dl = gGfxDlHead;
                gSPDisplayList(gGfxDlHead++, gDrawSetupEnvTintTexDl);
                gDPSetEnvColor(gGfxDlHead++, 192, 192, 192, w->alpha);
                gDPSetRenderMode(gGfxDlHead++, 0x552038, 0);
                gSPEndDisplayList(gGfxDlHead++);
            }
            break;
        case 0:
            w->rotY = eneCourseAngle(obj, 8);
            w->zone = eneGetCoursePos(obj->parent, &pos);
            w->x = pos.x;
            w->y = pos.y + 5.0f;
            w->z = pos.z;
            switch (w->state) {
                case 0:
                    w->alpha += 4;
                    if (w->alpha >= 252) {
                        w->alpha = 252;
                        w->fading = 0;
                        w->dmg->flags |= 2;
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (acollDmgWasHit(w->dmg) == 1) {
                        if (w->hitTimer == 0) {
                            w->hitTimer = 20;
                            w->hits++;
                            eneHitSoundHeavy(w->dmg);
                        }
                    }
                    if (w->hitTimer-- <= 0) {
                        w->hitTimer = 0;
                    }
                    if (w->hits >= 4) {
                        itexSparkleYellow(w->model->pos[0], w->model->pos[1] + 10.0f, w->model->pos[2], 0.15f);
                        ptclSpawnSparksFall(w->model->pos[0], w->model->pos[1] + 10.0f, w->model->pos[2]);
                        soundIncamCam(0x11B, w->model->pos[0], w->model->pos[1] + 10.0f, w->model->pos[2]);
                        w->model->dl2 = NULL;
                        w->timer = 20;
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (w->timer-- <= 0) {
                        sanbaClearSlot((s32) obj);
                        return -1;
                    }
                    break;
            }
            eneSetModelOnCourse(obj, &w->model->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

extern Gfx sanbaBodyDl[]; /* the body's setup list (this file's data) */
extern u8 sanbaBodyState1Anim[]; /* the body's animations */
extern u8 sanbaBodyState3Anim[];
extern u8 sanbaBodyState4Anim[];
extern u8 sanbaBodyState9Anim[];
extern u8 sanbaBodyState14Anim[];
extern u8 sanbaBodyState18Anim[];
extern u8 sanbaBodyState25Anim[];
extern u8 sanbaBodyState29Anim[];

f32 sanbaCamHeading(void);
void sanbaNextSequenceStep(E61Part *p);
Obj *sanbaSibukiMake(f32 x, f32 y, f32 z, f32 s, f32 h, f32 a, f32 b, f32 c, f32 d);
Obj *sanbaRippleMake(Obj *parent, f32 angle);
Obj *sanbaMarblMake(Obj *parent, f32 angle);
void sanbaWaterSetElectric(u8 v);
s32 sanbaMainGetLife(void);

/* Matched 2026-10-12. The float constants are spelled apart (IDO_NOTES 365; the FAKE is
 * marked at state 9); state 8 and 36's angle is s, state 33's test is "<= 0" with 34 first. The
 * last word: fadeSegToRam's result read into dl first (IDO_NOTES 358) frees the spill word that
 * kept plIsInPlay's temp (state 37's loop) at 0x40, game 0x44 (real 3 to MATCH); dl takes the
 * word the old pad held. Before that, measured: i at function scope after pad 3 (no change); no
 * pad (i at 0x58, the temp right) 4; pad and i in state 37's block 3; match_function.py results
 * nothing under 7. The game's late .rodata holds unreferenced 0.02 copies (0x9598-0x95A0,
 * 0x9650-0x9668, 0x9728): check the full build's bytes. */
/* "sanba": the boss's body (enemy kind 0x14, E61Part), rising out of the water at the middle of
 * the stage, turning to the camera or a player, throwing (kinds 0x1F and 0x20), dropping and
 * splashing (kind 0x1E), and at the end sinking and starting the battle's end. */
s32 sanbaProc(Obj *obj, s16 msg, void *arg) {
    E61PartW *w = E61PART(obj);
    f32 a;
    f32 r;
    s32 done;
    f32 s;
    s32 last;
    f32 x, y, z;
    Gfx *dl;

    switch (msg) {
        case 2:
            obj->name = "sanba";
            eneSetup(obj, arg);
            if ((w->light = (LightFrame *) frmCreate(gFrmLayer2, gFrmKindLight)) == NULL) {
                return -1;
            }
            w->light->lightNum = 1;
            w->light->color[0] = 64;
            w->light->color[1] = 192;
            w->light->color[2] = 255;
            w->light->dir[0] = 0;
            w->light->dir[1] = -127;
            w->light->dir[2] = 0;
            dl = (Gfx *) fadeSegToRam(sanbaBodyDl, obj->file);
            if ((w->model = eneNewAnime2(obj, (u32) sanbaBodyState1Anim, dl)) == NULL) {
                return -1;
            }
            frmMove(&w->model->f, &w->light->f);
            w->z = 0.0f;
            w->x = 0.0f;
            w->y = -280.0f;
            w->model->anim = 0;
            w->model->scale[2] = 0.6f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->timer = 100;
            w->step = -1;
            break;
        case 1:
            if (w->light != NULL) { frmDestroy(&w->light->f); }
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    break;
                case 1:
                    w->model->anim = sanbaBodyState1Anim;
                    w->model->frame = 0.0f;
                    w->rotY = sanbaCamHeading();
                    w->state = 2;
                    w->flag = 0;
                    w->vy = 5.0f;
                    w->y = 250.0f;
                    break;
                case 2:
                    eneAccelAxis(600.0f, 5.0f, 0.5f, &w->y, &w->vy);
                    if (480.0f <= w->y && w->flag == 0) {
                        w->flag = 1;
                        sndPlay(0x14A);
                        eneRumbleAll(0xBBBBBBBB, 40);
                        sanbaSibukiMake(0.0f, 685.0f, 0, 0.7, 0.7f, 0.04f, 0.04f, 0.04f, 0.01f);
                    }
                    if (520.0f < w->y) {
                        eneAnimPlayOnce(obj, w->model, 0.5f);
                        if (6.0f <= w->model->frame) {
                            w->model->frame = 6.0f;
                        }
                    }
                    if (w->vy < 0.0f) {
                        sanbaSibukiMake(0.0f, 685.0f, 0, 0.7f, 0.4f, 0.03f, 0.03f, 0.03f, 0.01f);
                        w->state = 3;
                    }
                    break;
                case 3:
                    eneAnimPlayOnce(obj, w->model, 0.5f);
                    eneAccelAxis(400.0f, 5.0f, 0.4f, &w->y, &w->vy);
                    if (w->y <= 400.0f && 0.0f <= w->vy) {
                        w->model->anim = sanbaBodyState3Anim;
                        w->model->frame = 0.0f;
                        w->state = 4;
                    }
                    break;
                case 4:
                    if (520.0f <= w->y && w->flag == 1) {
                        w->flag = 2;
                        sndPlay(0x14B);
                        sanbaSibukiMake(0.0f, 685.0f, 0, 0.7f, 0.4f, 0.02f, 0.02f, 0.02f, 0.01f);
                    }
                    if (520.0f <= w->y) {
                        eneAnimPlayOnce(obj, w->model, 0.4f);
                    }
                    eneAccelAxis(600.0f, 5.0f, 0.5f, &w->y, &w->vy);
                    if (w->y <= 600.0f && w->vy < 0.0f) {
                        w->y = 600.0f;
                        w->model->anim = sanbaBodyState4Anim;
                        w->model->frame = 0.0f;
                        sanbaSibukiMake(0.0f, 685.0f, 0, 0.7f, 0.2f, 0.02f, 0.02f, 0.02f, 0.01f);
                        w->state = 5;
                    }
                    break;
                case 5:
                    eneAnimLoop(obj, w->model, 0.2f);
                    break;
                case 6:
                    w->model->anim = sanbaBodyState4Anim;
                    w->model->frame = 0.0f;
                    if (fadeRand(100) < 50) {
                        w->turn = 0.017453294f;
                    } else {
                        w->turn = -0.017453294f;
                    }
                    w->count = 20;
                    w->soundTimer = 30;
                    w->state = 7;
                    w->y = 600.0f;
                case 7:
                    eneAnimLoop(obj, w->model, 0.2f);
                    w->rotY += w->turn;
                    if (6.2831855f <= w->rotY) {
                        w->rotY -= 6.2831855f;
                    }
                    if (w->rotY < 0.0f) {
                        w->rotY += 6.2831855f;
                    }
                    if (w->count-- <= 0) {
                        w->count = 30;
                        sanbaSibukiMake(0.0f, 685.0f, 0, 0.7f, 0.2f, 0.02f, 0.02f, 0.02f, 0.01f);
                    }
                    if (w->soundTimer-- <= 0) {
                        w->soundTimer = 40;
                        sndPlay(0x29B);
                    }
                    if (w->timer-- <= 0) {
                        w->state = 8;
                    }
                    break;
                case 8:
                    s = sanbaCamHeading();
                    if (3.1415927f < s - w->rotY) {
                        w->rotY += 6.2831855f;
                    } else if (s - w->rotY < -3.1415927f) {
                        w->rotY -= 6.2831855f;
                    }
                    if (fabsf(s - w->rotY) < 0.15707964f) {
                        w->rotY = s;
                        sanbaNextSequenceStep((E61Part *) obj);
                    } else if (s < w->rotY) {
                        w->rotY -= 0.15707964f;
                    } else {
                        w->rotY += 0.15707964f;
                    }
                    break;
                case 9:
                    w->model->anim = sanbaBodyState9Anim;
                    /* Here and in states 10-27 below: 0.f and .2f are spelled apart from 0.0f and
                     * 0.2f, so uopt keeps or drops them in registers as the game does (IDO_NOTES
                     * 365); so are the ints 580, 8, 2, 6 and the double 0.0 (measured).
                     * 0.f cannot be the double 0.0 too (real 614) and the int 0 is
                     * already the z of other calls. */
                    w->model->frame = 0.f;
                    sanbaSibukiMake(0.f, 685.0f, 0.f, 0.7f, 0.2f, 0.02f, 0.02f, 0.02f, 0.01f);
                    w->state = 10;
                case 10:
                    w->y -= 0.5f;
                    if (w->y < 580) {
                        w->y = 580;
                    }
                    eneAnimPlayOnce(obj, w->model, 0.2f);
                    if (8.0f <= w->model->frame) {
                        w->model->frame = 8.0f;
                        w->state = 11;
                    }
                    break;
                case 11:
                    eneAnimPlayOnce(obj, w->model, 0.2f);
                    if (8.0f <= w->model->frame) {
                        w->model->frame = 8.0f;
                        a = sanbaCamHeading();
                        r = w->rotY;
                        if (3.1415927f < a - r) {
                            r += 6.2831855f;
                        } else if (a - r < -3.1415927f) {
                            r -= 6.2831855f;
                        }
                        if (a < r) {
                            w->turn = -0.012217305f;
                        } else {
                            w->turn = 0.012217305f;
                        }
                        sanbaSibukiMake(0.f, 685.0f, 0.f, 0.7f, 0.2f, 0.02f, 0.02f, 0.02f, 0.01f);
                        sanbaRippleMake(obj, w->rotY);
                        sndPlay(0x147);
                        w->count = 180;
                        w->state = 12;
                    }
                    break;
                case 12:
                    eneAnimPlayOnce(obj, w->model, 0.2f);
                    if (8 <= w->model->frame) {
                        sanbaRippleMake(obj, w->rotY);
                        w->model->frame = w->model->frame - 8 + 7.0f;
                    }
                    if ((w->count & 7) == 0) {
                        sanbaSibukiMake(0.f, 685.0f, 0.f, 0.7f, 0.2f, 0.02f, 0.02f, 0.02f, 0.01f);
                    }
                    w->rotY += w->turn;
                    if (w->count-- <= 0) {
                        sndPlay(0x8147);
                        w->state = 13;
                    }
                    break;
                case 13:
                    w->y += 0.5f;
                    if (600.0f < w->y) {
                        w->y = 600.0f;
                    }
                    done = eneAnimPlayOnce(obj, w->model, 0.1f);
                    s = fabsf(w->turn);
                    r = s - 0.0017453294f;
                    if (s < 0.0017453294f) {
                        r = 0.0f;
                    }
                    if (w->turn < 0.0f) {
                        r *= -1.0f;
                    }
                    w->turn = r;
                    w->rotY += r;
                    if (done && 0.0f == w->turn && 600.0f == w->y) {
                        sanbaSibukiMake(0.0f, 685.0f, 0, 0.7f, 0.2f, 0.02f, 0.02f, 0.02f, 0.01f);
                        w->state = 6;
                    }
                    break;
                case 14:
                    w->model->anim = sanbaBodyState14Anim;
                    w->model->frame = 0.f;
                    sanbaSibukiMake(0.f, 685.0f, 0.f, 0.7f, 0.2f, 0.02f, 0.02f, 0.02f, 0.01f);
                    w->target = eneRandomPlayerIndex();
                    w->count = 0;
                    w->state = 15;
                case 15:
                    if (w->soundTimer-- == 0) {
                        sndPlay(0x29B);
                    }
                    if (w->count-- < 0) {
                        if (!plIsInPlay(w->target)) {
                            w->target = eneRandomPlayerIndex();
                        }
                        plGetModelPos(gGameWork.players[w->target], &x, &y, &z);
                        a = mathAtan2(x, z);
                        if (3.1415927f < a - w->rotY) {
                            w->rotY += 6.2831855f;
                        } else if (a - w->rotY < -3.1415927f) {
                            w->rotY -= 6.2831855f;
                        }
                        if (fabsf(a - w->rotY) < 0.07853982f) {
                            w->rotY = a;
                            sanbaSibukiMake(0.0, 685.0f, 0.0, 0.7f, 0.2f, 0.02f, 0.02f, 0.02f, 0.01f);
                            w->state = 16;
                        } else if (a < w->rotY) {
                            w->rotY -= 0.07853982f;
                        } else {
                            w->rotY += 0.07853982f;
                        }
                    }
                    break;
                case 16:
                    if (w->soundTimer-- == 0) {
                        sndPlay(0x29B);
                    }
                    eneAnimPlayOnce(obj, w->model, 0.2f);
                    if (4.0f <= w->model->frame) {
                        w->model->frame = 4.0f;
                        sndPlay(0x231);
                        sanbaMarblMake(obj, w->rotY);
                        sanbaSibukiMake(0.0, 685.0f, 0.0, 0.7f, 0.2f, 0.02f, 0.02f, 0.02f, 0.01f);
                        w->state = 17;
                    }
                    break;
                case 17:
                    if (w->soundTimer-- == 0) {
                        sndPlay(0x29B);
                    }
                    if (eneAnimPlayOnce(obj, w->model, 0.2f)) {
                        w->state = 6;
                    }
                    break;
                case 18:
                    w->model->anim = sanbaBodyState18Anim;
                    w->model->frame = 0.0f;
                    sanbaSibukiMake(0.0f, 685.0f, 0.0f, 0.7f, 0.2f, 0.02f, 0.02f, 0.02f, 0.01f);
                    w->state = 19;
                case 19:
                    if (eneAnimPlayOnce(obj, w->model, 0.3f)) {
                        w->state = 20;
                        w->count2 = 0;
                        w->vy = 2;
                        sanbaSibukiMake(0.0f, 685.0f, 0.0f, 0.7f, 0.7f, 0.04f, 0.04f, 0.04f, 0.01f);
                    }
                    break;
                case 20:
                    if (w->count2 == 0 && w->y < 600.0f) {
                        w->count2 = 1;
                        sndPlay(0x29A);
                    }
                    eneAccelAxis(500.0f, 5.0f, 0.4f, &w->y, &w->vy);
                    if (w->y <= 500.0f) {
                        w->y = 500.0f;
                        w->state = 21;
                        w->count = 100;
                        eneRumbleAll(0x88888888, 100);
                        sanbaWaterSetElectric(1);
                    }
                    break;
                case 21:
                    if (w->count-- <= 0) {
                        sndPlay(0x14B);
                        sanbaSibukiMake(0.0f, 685.0f, 0.0f, 0.7f, 0.4f, 0.02f, 0.02f, 0.02f, 0.01f);
                        sanbaWaterSetElectric(0);
                        w->state = 22;
                    }
                    break;
                case 22:
                    w->y -= 6;
                    if (w->y <= 450.0f) {
                        w->state = 32;
                    }
                    break;
                case 24:
                    sanbaWaterSetElectric(0);
                    sndPlay(0x8147);
                    eneRumbleStopAll();
                    w->y += 2.0f;
                    if (600.0f < w->y) {
                        w->y = 600.0f;
                    }
                    eneAnimLoop(obj, w->model, 0.2f);
                    break;
                case 25:
                    sndPlay(0x8147);
                    w->model->anim = sanbaBodyState25Anim;
                    w->model->frame = 0.0f;
                    w->count = 10;
                    sanbaSibukiMake(0.0f, 685.0f, 0.0f, 0.7f, 0.2f, 0.02f, 0.02f, 0.02f, 0.01f);
                    w->state = 26;
                case 26:
                    if (w->count-- == 0) {
                        sndPlay(0x2B4);
                    }
                    eneAnimPlayOnce(obj, w->model, 0.2f);
                    if (3.0f <= w->model->frame) {
                        sndPlay(0x2CC);
                        w->count = 0;
                        w->flag = 0;
                        w->state = 27;
                    }
                    break;
                case 27:
                    last = polAnimeLastFrame(w->model);
                    w->model->frame += .2f;
                    if ((f32) last <= w->model->frame) {
                        w->model->frame = w->model->frame - (f32) last + 4.0f;
                    }
                    w->y -= 1.0f;
                    if (w->y <= 580.0f && w->flag == 0) {
                        w->flag = 1;
                        sndPlay(0x29C);
                    }
                    if (w->y < 450.0f) {
                        sndPlay(0x82CC);
                        w->state = 32;
                    }
                    if (w->count-- <= 0 && 500.0f < w->y) {
                        w->count = 20;
                        sanbaSibukiMake(0.0f, 685.0f, 0.0f, 0.6f, 0.1f, 0.02f, 0.02f, 0.02f, 0.01f);
                    }
                    break;
                case 28:
                    sanbaWaterSetElectric(0);
                    sndPlay(0x8147);
                    eneRumbleStopAll();
                    w->y += 2.0f;
                    if (600.0f < w->y) {
                        w->y = 600.0f;
                    }
                    eneAnimLoop(obj, w->model, 0.2f);
                    break;
                case 29:
                    sndPlay(0x8147);
                    w->model->anim = sanbaBodyState29Anim;
                    w->model->frame = 0.0f;
                    w->state = 30;
                    w->count2 = 15;
                    sanbaSibukiMake(0.0f, 685.0f, 0.0f, 0.7f, 0.2f, 0.02f, 0.02f, 0.02f, 0.01f);
                case 30:
                    if (w->count2-- == 0) {
                        sndPlay(0x2B4);
                    }
                    eneAnimPlayOnce(obj, w->model, 0.5f);
                    if (2.0f <= w->model->frame) {
                        w->count = 0;
                        w->flag = 0;
                        w->model->frame = 2.0f;
                    }
                    break;
                case 31:
                    eneAnimPlayOnce(obj, w->model, 0.1f);
                    w->y -= 0.8f;
                    if (w->flag == 0) {
                        w->flag = 1;
                        sndPlay(0x2CC);
                    }
                    if (w->y <= 580.0f && w->flag == 1) {
                        w->flag = 2;
                        sndPlay(0x29C);
                    }
                    if (w->y < 450.0f) {
                        sndPlay(0x82CC);
                        w->state = 32;
                    }
                    if (w->count-- <= 0 && 500.0f < w->y) {
                        w->count = 30;
                        sanbaSibukiMake(0.0f, 685.0f, 0.0f, 0.6f, 0.1f, 0.02f, 0.02f, 0.02f, 0.01f);
                    }
                    break;
                case 32:
                    w->model->anim = sanbaBodyState3Anim;
                    w->model->frame = 0.0f;
                    w->flag = 0;
                    w->state = 33;
                    w->vy = 0.0f;
                case 33:
                    if (520.0f <= w->y) {
                        eneAnimPlayOnce(obj, w->model, 0.2f);
                    }
                    eneAccelAxis(600.0f, 5.0f, 0.5f, &w->y, &w->vy);
                    if (520.0f <= w->y && w->flag == 0) {
                        w->flag = 1;
                        sndPlay(0x14B);
                        sanbaSibukiMake(0.0f, 685.0f, 0.0f, 0.7f, 0.4f, 0.02f, 0.02f, 0.02f, 0.01f);
                    }
                    if (w->y <= 600.0f && w->vy < 0.0f) {
                        w->y = 600.0f;
                        sanbaSibukiMake(0.0f, 685.0f, 0.0f, 0.7f, 0.2f, 0.02f, 0.02f, 0.02f, 0.01f);
                        if (sanbaMainGetLife() <= 0) {
                            w->state = 34;
                        } else {
                            w->state = 6;
                        }
                    }
                    break;
                case 34:
                    gGameWork.scenarioRunning = 2;
                    w->model->anim = sanbaBodyState4Anim;
                    w->model->frame = 0.0f;
                    w->count = 20;
                    w->state = 35;
                case 35:
                    eneAnimLoop(obj, w->model, 0.2f);
                    if (w->count-- <= 0) {
                        sndPlay(0x29B);
                        sanbaSibukiMake(0.0f, 685.0f, 0.0f, 0.7f, 0.4f, 0.02f, 0.02f, 0.02f, 0.01f);
                        w->state = 36;
                    }
                    break;
                case 36:
                    eneAnimPlayOnce(obj, w->model, 0.5f);
                    s = sanbaCamHeading() + 3.1415927f;
                    if (3.1415927f < s - w->rotY) {
                        w->rotY += 6.2831855f;
                    } else if (s - w->rotY < -3.1415927f) {
                        w->rotY -= 6.2831855f;
                    }
                    if (fabsf(s - w->rotY) < 0.15707964f) {
                        w->rotY = s;
                        w->flag = 0;
                        w->vy = 5.0f;
                        sndPlay(0x14E);
                        sanbaSibukiMake(0.0f, 685.0f, 0.0f, 0.7f, 0.2f, 0.02f, 0.02f, 0.02f, 0.01f);
                        w->state = 37;
                    } else if (s < w->rotY) {
                        w->rotY -= 0.15707964f;
                    } else {
                        w->rotY += 0.15707964f;
                    }
                    break;
                case 37: {
                    s32 i;

                    eneAnimPlayOnce(obj, w->model, 0.5f);
                    eneAccelAxis(400.0f, 5.0f, 0.4f, &w->y, &w->vy);
                    if (w->vy <= 0.0f && w->flag == 0) {
                        w->flag = 1;
                        sanbaSibukiMake(0.0f, 685.0f, 0.0f, 0.7f, 0.7f, 0.04f, 0.04f, 0.04f, 0.01f);
                    }
                    if (w->y <= 600.0f && w->flag == 1) {
                        w->flag = 2;
                        SoundLocation2(0x14D, gGameWork.view, w->model->pos[0], w->model->pos[1], w->model->pos[2],
                                       0.0f, 1000.0f, gGameWork.view->fovy);
                    }
                    if (w->y <= 500.0f && w->flag == 2) {
                        w->flag = 3;
                        sanbaSibukiMake(0.0f, 685.0f, 0.0f, 0.4f, 0.2f, 0.04f, 0.06f, 0.04f, 0.02f);
                    }
                    if (w->y <= 400.0f) {
                        SoundLocation2(0x122, gGameWork.view, w->model->pos[0], w->model->pos[1], w->model->pos[2],
                                       0.0f, 1000.0f, gGameWork.view->fovy);
                        scRunStart2(16, 0, 0);
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                plSetAction(gGameWork.players[i], 0xD7);
                            }
                        }
                        w->state = 38;
                    }
                    break;
                }
                case 38:
                    if (!scIsRunning()) {
                        sndPlay(10);
                        fadeCreate(obj);
                        fadeSetSpeed(32.0f);
                        fadeSetLevel(0.0f);
                        fadeSetColor(0, 0, 0);
                        fadeSetTarget(255.0f);
                        w->count = 32;
                        eneEndPlayersPowerDown();
                        w->state = 39;
                    }
                    break;
                case 39:
                    if (w->count-- <= 0) {
                        eneRumbleStopAll();
                        eneCounterSetFlag(1);
                        stageGotoBattle(2);
                    }
                    break;
            }
            eneSetModel(obj, &w->model->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The camera's heading: the angle of its eye in x and z (tlb_0037's iheadCamHeading twin; the
 * named cam local declared first gives the game's frame). */
f32 sanbaCamHeading(void) {
    Camera *cam;
    f32 z;
    f32 y;
    f32 x;

    cam = camAiGetCam();
    camGetEye(cam, &x, &y, &z);
    return mathAtan2(x, z);
}


/* The part's next step: a state and how long it lasts; the tenth sets state 9 and starts over. */
void sanbaNextSequenceStep(E61Part *p) {
    E61PartW *w = E61PART(p);

    switch (++w->step) {
        case 0:
            w->state = 14; w->timer = 10;
            break;
        case 1:
            w->state = 14; w->timer = 10;
            break;
        case 2:
            w->state = 14; w->timer = 5;
            break;
        case 3:
            w->state = 18; w->timer = 20;
            break;
        case 4:
            w->state = 14; w->timer = 10;
            break;
        case 6:
            w->state = 14; w->timer = 10;
            break;
        case 7:
            w->state = 9; w->timer = 20;
            break;
        case 8:
            w->state = 18; w->timer = 20;
            break;
        case 9:
            w->state = 9; w->timer = 20;
            w->step = -1;
            break;
    }
}

void sanbaSetState1(E61Part *p) {
    p->state = 1;
}

s32 sanbaIsState5(E61Part *p) {
    if (p->state == 5) {
        return 1;
    }
    return 0;
}

void sanbaSetState6(E61Part *p) {
    p->state = 6;
}

f32 sanbaGetRotY(E61Part *p) {
    return p->rotY;
}

extern u8 sanbaBodyState4Anim[]; /* an animation in the enemy's segment */

/* Plays animation sanbaBodyState4Anim from its start; state 0x18, or 0x1C with flag; sounds 0x8148 and
 * 0x8147. */
void sanbaSetDropState(E61Part *p, s32 flag) {
    s32 pad;

    p->anim->anim = sanbaBodyState4Anim;
    p->anim->frame = 0.0f;
    if (flag == 0) {
        p->state = 0x18;
    } else {
        p->state = 0x1C;
    }
    sndPlay(0x8148);
    sndPlay(0x8147);
}

void sanbaNextState(E61Part *p) {
    p->state++;
}

s32 sanbaIsState33(E61Part *p) {
    if (p->state == 0x21) {
        return 1;
    }
    return 0;
}

/* "sibuki" (a splash, enemy kind 0x1E, made by sanbaSibukiMake under gEffectTask): its
 * work. */
typedef struct E1EWork {
    /* 0x00 */ u8 state; /* 0 rising, 1 falling */
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
    /* 0x34 */ PolFrame *m[3]; /* three models, sized 1, 1.5 and 2 times */
    /* 0x40 */ s32 alpha;
    /* 0x44 */ f32 a; /* the spread a step */
    /* 0x48 */ f32 b;
    /* 0x4C */ f32 c; /* the rise a step */
    /* 0x50 */ f32 d;
    /* 0x54 */ s32 count; /* steps rising */
} E1EWork;

#define E1E(o) ((E1EWork *) FX(o))

extern Gfx sanbaSibukiSplashDl0[]; /* the splash's three models */
extern Gfx sanbaSibukiSplashDl1[];
extern Gfx sanbaSibukiSplashDl2[];

/* "sibuki": a splash of three rings turned at random, spreading by a and rising by c (the rings
 * 2, 1.3 and 1 times) for count steps, then falling, fading 8 a step. */
s32 sanbaSibukiProc(Obj *obj, s16 msg, void *arg) {
    E1EWork *w = E1E(obj);
    s32 pad[3];

    switch (msg) {
        case 2:
            obj->name = "sibuki";
            eneSetup(obj, arg);
            if ((w->m[0] = eneNewPol(obj, sanbaSibukiSplashDl0)) == NULL) {
                return -1;
            }
            frmMove(&w->m[0]->f, gFrmLayer8);
            if ((w->m[1] = eneNewPol(obj, sanbaSibukiSplashDl1)) == NULL) {
                return -1;
            }
            frmMove(&w->m[1]->f, gFrmLayer8);
            if ((w->m[2] = eneNewPol(obj, sanbaSibukiSplashDl2)) == NULL) {
                return -1;
            }
            frmMove(&w->m[2]->f, gFrmLayer8);
            w->m[0]->rot[1] = (f32) fadeRand(360) / 180.0f * 3.1415927f;
            w->m[1]->rot[1] = (f32) fadeRand(360) / 180.0f * 3.1415927f;
            w->m[2]->rot[1] = (f32) fadeRand(360) / 180.0f * 3.1415927f;
            w->alpha = 255;
            DPRINTF("SIBUKIINITDATA sx = %f,sy = %f,sz = %f\n", w->a, w->c, w->a);
            break;
        case 1:
            frmDestroy(&w->m[0]->f);
            frmDestroy(&w->m[1]->f);
            frmDestroy(&w->m[2]->f);
            break;
        case 3:
            w->m[0]->dl = gGfxDlHead;
            w->m[1]->dl = gGfxDlHead;
            w->m[2]->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, gDrawSetupPrimEnvTexBlendDl);
            gDPSetEnvColor(gGfxDlHead++, 255, 255, 255, w->alpha);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 0, 0, 255, 0);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->m[0]->scale[2] += w->a;
                    w->m[0]->scale[0] = w->m[0]->scale[2];
                    w->m[1]->scale[2] += w->a;
                    w->m[1]->scale[0] = w->m[1]->scale[2];
                    w->m[2]->scale[2] += w->a;
                    w->m[2]->scale[0] = w->m[2]->scale[2];
                    w->m[0]->scale[1] += w->c * 2.0f;
                    w->m[1]->scale[1] += 1.3f * w->c;
                    w->m[2]->scale[1] += w->c;
                    if (w->count-- <= 0) {
                        w->state++;
                    }
                    break;
                case 1:
                    w->m[0]->scale[2] += w->a;
                    w->m[0]->scale[0] = w->m[0]->scale[2];
                    w->m[1]->scale[2] += w->a;
                    w->m[1]->scale[0] = w->m[1]->scale[2];
                    w->m[2]->scale[2] += w->a;
                    w->m[2]->scale[0] = w->m[2]->scale[2];
                    w->m[0]->scale[1] -= 1.5f * w->c;
                    w->m[1]->scale[1] -= 1.2f * w->c;
                    w->m[2]->scale[1] -= w->c;
                    if (w->m[2]->scale[1] < 0.0001f) {
                        w->m[2]->scale[1] = 0.0001f;
                    }
                    break;
            }
            if ((w->alpha -= 8) < 0) {
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Makes an enemy of kind 0x1E at x, y, z under gEffectTask: its three models there, scaled s,
 * 1.5 s and 2 s across and h high. The models go through a local "m": without it IDO loads h
 * after the 1.5f's multiply. */
Obj *sanbaSibukiMake(f32 x, f32 y, f32 z, f32 s, f32 h, f32 a, f32 b, f32 c, f32 d) {
    Obj *o = eneMakeUnder(gEffectTask, ENE_SANBA_SIBUKI, 0, x, y, z);

    if (o != NULL) {
        f32 s15 = 1.5f * s;
        f32 s2 = s + s;
        PolFrame **m = E1E(o)->m;

        m[2]->pos[0] = x;
        m[0]->pos[0] = m[1]->pos[0] = m[2]->pos[0];
        m[2]->pos[1] = y;
        m[0]->pos[1] = m[1]->pos[1] = m[2]->pos[1];
        m[2]->pos[2] = z;
        m[0]->pos[2] = m[1]->pos[2] = m[2]->pos[2];
        m[0]->scale[2] = s;
        m[0]->scale[0] = s;
        m[0]->scale[1] = h;
        m[1]->scale[2] = s15;
        m[1]->scale[0] = s15;
        m[1]->scale[1] = h;
        m[2]->scale[2] = s2;
        m[2]->scale[0] = s2;
        m[2]->scale[1] = h;
        E1E(o)->a = a;
        E1E(o)->b = b;
        E1E(o)->c = c;
        E1E(o)->d = d;
        E1E(o)->count = 10;
    }
    return o;
}

/* "ripple"'s work: an enemy's place and speed (eneMoveObj), its model and attack box. */
typedef struct RippleW {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vx;
    /* 0x2C */ f32 vy;
    /* 0x30 */ f32 vz;
    /* 0x34 */ PolFrame *model;
    /* 0x38 */ ACollFrame *atk;
    /* 0x3C */ u8 pad3C[0x4];
    /* 0x40 */ s32 timer;
    /* 0x44 */ s32 alpha;
} RippleW;

extern Gfx sanbaRippleDl[]; /* the ripple's display list */

/* "ripple": a ring rising faster and faster for 60 steps, its attack box hitting with kind 120,
 * drawn yellow. */
s32 sanbaRippleProc(Obj *obj, s16 msg, void *arg) {
    RippleW *w = (RippleW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "ripple";
            eneSetup(obj, arg);
            if ((w->model = eneNewPol(obj, sanbaRippleDl)) == NULL) {
                return -1;
            }
            frmMove(&w->model->f, gFrmLayer8);
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 150, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 120;
            w->model->flags |= 7;
            w->alpha = 255;
            w->timer = 60;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            break;
        case 3:
            w->model->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, gDrawSetupPrimEnvTexBlendDl);
            gDPSetEnvColor(gGfxDlHead++, 255, 255, 0, w->alpha);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 255, 0);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            if (acollAtkWasHit(w->atk) == 1) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, obj);
            }
            if (w->timer-- <= 0) {
                return -1;
            }
            w->vy += 0.04;
            eneMoveObj(obj);
            eneSetModel(obj, &w->model->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Enemy kinds 0x1F and 0x20 (made by sanbaRippleMake/sanbaMarblMake): an enemy's work (ENE) with
 * its heading where EneObj keeps the shadow. */
typedef struct ThrownW {
    /* 0x00 */ u8 unused00;
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
    /* 0x34 */ u8 pad34[8];
    /* 0x3C */ f32 angle;
} ThrownW;

#define THROWN(o) ((ThrownW *) FX(o))

/* Makes an enemy of kind 0x1F under parent, 72 out along angle and 132 above it, moving out at
 * 4 and down at 0.8. */
Obj *sanbaRippleMake(Obj *parent, f32 angle) {
    EneObj *e;
    s32 pad;
    Obj *o;
    f32 y = ENE(parent)->y;
    s32 pad2[2];

    o = eneMakeUnder(parent, ENE_SANBA_RIPPLE, 0, 0.0f, 0.0f, 0.0f);
    if (o != NULL) {
        THROWN(o)->angle = angle;
        e = ENE(o);
        e->x = mathSin(angle) * 120.0f * 0.6f;
        e->y = y + 132.0f;
        e->z = mathCos(angle) * 120.0f * 0.6f;
        e->vel[0] = mathSin(angle) * 4.0f;
        e->vel[1] = -0.8f;
        e->vel[2] = mathCos(angle) * 4.0f;
    }
    return o;
}

/* "marbl"'s work: an enemy's (eneMoveObj), its model and attack box. */
typedef struct MarblW {
    /* 0x00 */ u8 unused00;
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
    /* 0x38 */ ACollFrame *atk;
} MarblW;

extern Gfx sanbaMarblColour0Dl[]; /* the marble's four colours */
extern Gfx sanbaMarblColour1Dl[];
extern Gfx sanbaMarblColour2Dl[];
extern Gfx sanbaMarblColour3Dl[];

/* "marbl": a marble of one of four colours (at random) that falls until it is below 685 or
 * hits; it bursts in yellow sparkles and sparks with sound 0x229 as it goes. */
s32 sanbaMarblProc(Obj *obj, s16 msg, void *arg) {
    MarblW *w = (MarblW *) FX(obj);
    s32 r;
    s32 pad[4];
    CamView *view;
    PolFrame *m;

    switch (msg) {
        case 2:
            r = fadeRand(100);
            obj->name = "marbl";
            eneSetup(obj, arg);
            if (r < 25) {
                w->model = eneNewPol(obj, sanbaMarblColour0Dl);
            } else if (r < 50) {
                w->model = eneNewPol(obj, sanbaMarblColour1Dl);
            } else if (r < 75) {
                w->model = eneNewPol(obj, sanbaMarblColour2Dl);
            } else {
                w->model = eneNewPol(obj, sanbaMarblColour3Dl);
            }
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[2] = 0.2f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 40, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 120;
            w->model->flags |= 7;
            break;
        case 1:
            m = w->model;
            view = gGameWork.view;
            if (m != NULL) {
                itexSparkleYellow(m->pos[0], m->pos[1] + 5.0f, m->pos[2], 0.2f);
                ptclSpawnSparksFall(m->pos[0], m->pos[1] + 5.0f, m->pos[2]);
                SoundLocation2(0x229, view, w->model->pos[0], w->model->pos[1] + 5.0f, w->model->pos[2], 150.0f,
                               800.0f, view->fovy);
            }
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            break;
        case 0:
            if (acollAtkWasHit(w->atk) == 1) {
                view = gGameWork.view;
                SoundLocation2(0x229, view, w->model->pos[0], w->model->pos[1] + 5.0f, w->model->pos[2], 150.0f,
                               800.0f, view->fovy);
                tskSendMsg((Obj *) w->atk->hitPower, 7, obj);
                return -1;
            }
            if (w->y <= 685.0f) {
                return -1;
            }
            eneMoveObj(obj);
            eneSetModel(obj, &w->model->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Makes an enemy of kind 0x20 under parent, 72 out along angle and 132 above it, moving out at
 * 1.15 and down at 1. */
Obj *sanbaMarblMake(Obj *parent, f32 angle) {
    EneObj *e;
    s32 pad;
    Obj *o;
    f32 y = ENE(parent)->y;
    s32 pad2[2];

    o = eneMakeUnder(parent, ENE_SANBA_MARBL, 0, 0.0f, 0.0f, 0.0f);
    if (o != NULL) {
        THROWN(o)->angle = angle;
        e = ENE(o);
        e->x = mathSin(angle) * 120.0f * 0.6f;
        e->y = y + 132.0f;
        e->z = mathCos(angle) * 120.0f * 0.6f;
        e->vel[0] = mathSin(angle) * 1.15f;
        e->vel[1] = -1.0f;
        e->vel[2] = mathCos(angle) * 1.15f;
    }
    return o;
}

/* A random number from -n to n - 1 (a linear congruential generator, 15 bits), as tlb_0324's
 * btaiRandSigned (tlb_0324_rand.c): its seed is its static (0x00393740, this file's last .data
 * word), which uopt keeps in a register and stores back last (IDO_NOTES 316). Parked until
 * (689) with the seed as an extern (18 at best, through a FAKE second name). */
s16 sanbaRand(s16 n) {
    static u32 seed = 3749696;

    seed = seed * 1103515245 + 5963;
    return (s16) (seed & 0x7FFF) / (0x8000 / (s16) (n * 2) + 1) - n;
}

/* "cTHUNDER"'s (sanbaCThunderProc) argument: only what its starter sets. */
typedef struct Tlb39ThunderArg {
    /* 0x00 */ u8 pad00[0x4];
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ u8 pad10[0x8];
    /* 0x18 */ s32 kind;
    /* 0x1C */ u8 pad1C[0xC];
} Tlb39ThunderArg;

/* "cTHUNDER"'s work. */
typedef struct ThunderW {
    /* 0x00 */ PolFrame *model;
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ s32 alpha;
    /* 0x14 */ s32 frame; /* of its four-frame animation */
    /* 0x18 */ s32 kind; /* 0 a bolt near pos, 1 one farther out, 2 a flash */
    /* 0x1C */ s16 prim[3];
    /* 0x22 */ s16 env[3];
} ThunderW;

extern s32 gDemoModelsDelete2;
extern Gfx sanbaCThunderBoltFrame0Dl[]; /* the bolt's four frames */
extern Gfx sanbaCThunderBoltFrame1Dl[];
extern Gfx sanbaCThunderBoltFrame2Dl[];
extern Gfx sanbaCThunderBoltFrame3Dl[];

s16 sanbaRand(s16 n);

/* "cTHUNDER": a lightning bolt at a random place around pos (kind 0: 80 across, 1: 400 and
 * 500), yellow, growing taller by 1% a step and fading. */
s32 sanbaCThunderProc(Obj *obj, s16 msg, void *arg) {
    s32 pad[2];
    ThunderW *w = (ThunderW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "cTHUNDER";
            w->kind = ((Tlb39ThunderArg *) arg)->kind;
            w->pos[0] = ((Tlb39ThunderArg *) arg)->pos[0];
            w->pos[1] = ((Tlb39ThunderArg *) arg)->pos[1];
            w->pos[2] = ((Tlb39ThunderArg *) arg)->pos[2];
            if ((w->model = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                return -1;
            }
            w->model->dl2 = sanbaCThunderBoltFrame0Dl;
            floadGetSetRam((void **) w->model->segs, 0xC6);
            w->model->flags |= 0x22;
            w->model->scale[1] = 0.1f;
            w->model->scale[0] = 0.1f;
            w->model->scale[2] = w->model->scale[0];
            switch (w->kind) {
                case 0:
                    w->model->pos[0] = w->pos[0] + sanbaRand(80);
                    w->model->pos[1] = w->pos[1];
                    w->model->pos[2] = w->pos[2] + sanbaRand(80);
                    w->frame = 0;
                    w->alpha = 224;
                    w->model->scale[1] = 0.1f;
                    break;
                case 1:
                    w->model->pos[0] = w->pos[0] + sanbaRand(400);
                    w->model->pos[1] = w->pos[1];
                    w->model->pos[2] = w->pos[2] + sanbaRand(500);
                    w->frame = 0;
                    w->alpha = 224;
                    w->model->scale[1] = 0.1f;
                    w->model->scale[0] = 0.1f;
                    w->model->scale[2] = w->model->scale[0];
                    break;
            }
            w->prim[0] = 255;
            w->prim[1] = 255;
            w->prim[2] = 224;
            w->env[0] = 160;
            w->env[1] = 96;
            w->env[2] = 32;
            break;
        case 1:
            frmDestroy(&w->model->f);
            break;
        case 3:
            w->model->dl = demoColorDL(w->prim[0], w->prim[1], w->prim[2], w->env[0], w->env[1], w->env[2],
                                       w->alpha);
            break;
        case 0:
            if (gDemoModelsDelete2) {
                return -1;
            }
            if (gGameFrameCount % 3 == 0) {
                if (++w->frame >= 5) {
                    w->frame = 0;
                }
                switch (w->frame) {
                    case 0:
                        w->model->dl2 = sanbaCThunderBoltFrame0Dl;
                        break;
                    case 1:
                        w->model->dl2 = sanbaCThunderBoltFrame1Dl;
                        break;
                    case 2:
                        w->model->dl2 = sanbaCThunderBoltFrame2Dl;
                        break;
                    case 3:
                        w->model->dl2 = sanbaCThunderBoltFrame3Dl;
                        break;
                }
            }
            switch (w->kind) {
                case 0:
                    w->model->scale[1] *= 1.01;
                    if (w->alpha > 208) {
                        w->alpha--;
                    } else {
                        w->alpha -= 20;
                    }
                    break;
                case 1:
                    w->model->scale[1] *= 1.01;
                    w->alpha -= 16;
                    break;
                case 2:
                    w->model->scale[1] *= 1.01;
                    w->model->scale[0] = 0.1f;
                    w->model->scale[2] = w->model->scale[0];
                    if (w->alpha > 208) {
                        w->alpha--;
                    } else {
                        w->alpha -= 20;
                    }
                    break;
            }
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "cTHUNDER" of a kind at x, y, z under obj. */
void sanbaCThunderCreate(Obj *obj, s32 kind, f32 x, f32 y, f32 z) {
    Tlb39ThunderArg arg;

    arg.kind = kind;
    arg.pos[0] = x;
    arg.pos[2] = z;
    arg.pos[1] = y;
    tskCreate(obj, sanbaCThunderProc, obj->file, &arg);
}

/* "tai_bgm": after 4 steps, plays the music 0x98 (arg nonzero) or 0xA5, once. */
s32 sanbaTaiBgmProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    TaiBgmW *w = TAI_BGM(obj);

    switch (msg) {
        case 2:
            obj->name = "tai_bgm";
            w->played = 0;
            w->wait = 4;
            w->battle = (s32) arg;
            break;
        case 0:
            if (w->played == 0) {
                if (w->wait-- <= 0) {
                    if (w->battle != 0) {
                        soundPlayAfterOne(0x98);
                        sndSetReverb(1, 15);
                    } else {
                        soundPlayAfterOne(0xA5);
                        sndSetReverb(1, 15);
                    }
                    w->played = 1;
                }
            }
            break;
        case 1:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Starts a sanbaTaiBgmProc task under obj. */
Obj *sanbaTaiBgmCreate(Obj *obj, void *arg) {
    return tskCreate(obj, sanbaTaiBgmProc, obj->file, arg);
}

void sanbaTaiBgmRestart(Obj *o, s32 v) {
    TAI_BGM(o)->played = 0;
    TAI_BGM(o)->wait = 0;
    TAI_BGM(o)->battle = v;
}
