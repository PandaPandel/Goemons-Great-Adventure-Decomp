#include "tlb.h"

/* File 138: small demo and town pieces. "sekiStopper", a barrier the players stand against
 * ("demoStopper"); "kira" and "kirakira", sparkles and their spawner; two "atemAtari"s, an item
 * box the players stand against ("itemAtari") and an automatic talk ("demoAuto"); "timer", the
 * time a goal must be reached by ("demoTimer"); "tegata", the pass's effect; and
 * "chouninAtari", a townsman's stopper ("jizou"). */

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* A barrier or item box: an enemy's work (EneObj) to 0x34, then its ride. */
typedef struct StopperW {
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
    /* 0x34 */ RideFrame *ride;
} StopperW;

#define STOPPER(obj) ((StopperW *) FX(obj))

/* A sparkle ("kira"). */
typedef struct KiraW {
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
    /* 0x34 */ s16 alpha;
} KiraW;

#define KIRA(obj) ((KiraW *) FX(obj))

/* The sparkles' spawner ("kirakira"). */
typedef struct KirakiraW {
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
    /* 0x34 */ s32 timer;
} KirakiraW;

#define KIRAKIRA(obj) ((KirakiraW *) FX(obj))

/* "demoAuto": a talk that starts by itself. */
typedef struct DemoAutoW {
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
    /* 0x34 */ s32 talk; /* its scenario, from its placement */
} DemoAutoW;

#define DEMOAUTO(obj) ((DemoAutoW *) FX(obj))

/* "chouninAtari": a townsperson's work (PersonObj) up to 0x38, with its own talk data. */
typedef struct ChouninW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 pad09;
    /* 0x0A */ u8 hit;
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PersonData *data; /* its own, below */
    /* 0x38 */ PersonData own;
    /* 0x4C */ u8 stay; /* 1: it does not fall */
} ChouninW;

#define CHOUNIN(obj) ((ChouninW *) FX(obj))

/* "timer": the time left to a goal. */
typedef struct TimerW {
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
    /* 0x34 */ CamView *view; /* its own camera */
    /* 0x38 */ PolFrame *back; /* the clock's face */
    /* 0x3C */ PolFrame *back2; /* and its back, turned over */
    /* 0x40 */ PolFrame *digit[4]; /* hours (tens, ones), minutes (tens, ones) */
    /* 0x50 */ PolFrame *colon; /* "ten" */
    /* 0x54 */ u8 shown[4]; /* the digit each shows (21 at first: none) */
    /* 0x58 */ u16 goal; /* in minutes, a day on (+1440) if it is earlier than now */
    /* 0x5A */ u16 now; /* in minutes */
    /* 0x5C */ u8 days; /* days on to the goal */
} TimerW;

#define TIMER(obj) ((TimerW *) FX(obj))

/* "tegata": the pass, at its task + 0x20 (file 126's PassW). */
typedef struct TegataW {
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
    /* 0x38 */ u8 rgb[3]; /* the glow's colour, new every 10 ticks */
    /* 0x3C */ s16 alpha;
    /* 0x40 */ s32 timer;
    /* 0x44 */ Obj *player; /* the one it goes to */
    /* 0x48 */ Obj *other; /* the other player */
} TegataW;

#define TEGATA(obj) ((TegataW *) FX(obj))

u16 demoTimeOfDayMinutes(void);
void demoTegataGetVoice(Obj *player);

extern u8 demoTimerDigitDl0[];
extern u8 demoTimerDigitDl1[];
extern u8 demoTimerDigitDl2[];
extern u8 demoTimerDigitDl3[];
extern u8 demoTimerDigitDl4[];
extern u8 demoTimerDigitDl7[];
extern u8 demoTimerDigitDl8[];
Gfx demoTimerFaceDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetEnvColor(0x00, 0x00, 0x00, 0xC8),
    gsDPSetCombineLERP(0, 0, 0, SHADE, SHADE, 0, ENVIRONMENT, 0, 0, 0, 0, SHADE, SHADE, 0, ENVIRONMENT, 0),
    gsSPEndDisplayList(),
}; /* the clock face's display list */
Gfx demoTimerBackDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2),
    gsDPSetCombineLERP(0, 0, 0, SHADE, SHADE, 0, 0, 0, 0, 0, 0, SHADE, SHADE, 0, 0, 0),
    gsSPEndDisplayList(),
}; /* its back's */
Gfx demoTimerDigitDl[] = {
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
}; /* the digits' and the colon's */
u32 demoTimerDigitTex[] = { (u32) demoTimerDigitDl0, (u32) demoTimerDigitDl1, (u32) demoTimerDigitDl2, (u32) demoTimerDigitDl3, (u32) demoTimerDigitDl4, 0x030007C0, 0x030008A0, (u32) demoTimerDigitDl7, (u32) demoTimerDigitDl8, 0x03000B40, 0x00000000, 0x00000000, 0x081E0000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 }; /* the digits' textures, 0-9 */
extern u8 demoStopperRide[];
extern u8 itemAtariRide[]; /* the item box's ride data (kind 0x198's file set 0x85): demoStopperRide's address in another file */
extern Gfx demoKiraDl[];
extern u8 demoJizouAnim[];
extern Gfx demoTimerBack2Dl[];
extern Gfx demoTimerColonDl[];
extern Gfx demoTimerBackModelDl[];
extern Gfx demoTegataDl[];
extern Gfx demoTegataGlowDl[];

/* "sekiStopper": a barrier on the course the players stand against (a ride). */
s32 demoStopper_proc(Obj *o, s16 msg, void *arg) {
    StopperW *w = STOPPER(o);

    switch (msg) {
        case 2:
            o->name = "sekiStopper";
            eneSetup(o, arg);
            w->pol = eneNewPol(o, NULL);
            if (w->pol == NULL) {
                return -1;
            }
            w->pol->scale[2] = 1.0f;
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
            w->ride = rideCreate(gRideTree1, (s32) o, &w->pol->f, (RideData *) eneFileAddr(o, demoStopperRide, 0));
            DPRINTF("demoStopper_proc: デモストッパーセットＯＫ\n");
            break;
        case 1:
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            if (w->ride != NULL) {
                frmDestroy(&w->ride->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneMove((EneObj *) w);
            if (w->pol != NULL) {
                eneSetModelOnCourse(o, &w->pol->f);
            }
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "kira": a sparkle of a random size that shrinks, fades and rises, in random prim colours. */
s32 demoKiraProc(Obj *o, s16 msg, void *arg) {
    KiraW *w = KIRA(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "kira";
            eneSetup(o, arg);
            w->pol = enePolModelMake(o, demoKiraDl, NULL, 1);
            if (w->pol == NULL) {
                return -1;
            }
            w->pol->flags |= 0x27;
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2] = fadeRandRange(1, 5) / 10.0f + 0.3;
            w->alpha = 160;
            break;
        case 3:
            w->pol->dl = eneDlEnvPrimColor(o, 240, 240, 160, fadeRand(255), fadeRand(255), fadeRand(255), w->alpha);
            break;
        case 1:
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            break;
        case 0:
            w->pol->scale[0] -= 0.01;
            w->pol->scale[1] -= 0.01;
            w->pol->scale[2] -= 0.01;
            w->alpha -= 5;
            if (w->alpha < 0) {
                return -1;
            }
            if (w->pol->scale[0] < 0.1) {
                w->pol->scale[2] = 0.1f;
                w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
                return -1;
            }
            w->vel[1] += 0.01;
            eneMove((EneObj *) w);
            if (w->pol != NULL) {
                eneSetModelOnCourse(o, &w->pol->f);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "kirakira": makes sparkles (kind 0x1C0) around itself every 3 and 6 ticks, for 16 ticks. */
s32 demoKirakiraProc(Obj *o, s16 msg, void *arg) {
    KirakiraW *w = KIRAKIRA(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "kirakira";
            eneSetup(o, arg);
            w->pol = eneNewPol(o, NULL);
            if (w->pol == NULL) {
                return -1;
            }
            break;
        case 1:
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            break;
        case 0:
            if (w->timer % 3 == 0) {
                eneMakeEx(NULL, ENE_DEMO_KIRA, 0, w->pol->pos[0] - fadeRand(8), w->pol->pos[1] + fadeRand(6),
                          w->pol->pos[2] - fadeRand(8), 0, NULL);
            }
            if (w->timer % 6 == 0) {
                eneMakeEx(NULL, ENE_DEMO_KIRA, 0, w->pol->pos[0] + fadeRand(8), w->pol->pos[1] + fadeRand(6),
                          w->pol->pos[2] + fadeRand(8), 0, NULL);
            }
            if (w->timer >= 16) {
                return -1;
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "atemAtari" ("itemAtari"): an item box the players stand against, its size from its
 * placement (x, y and z scales in tenths, a byte each). */
s32 itemAtari_proc(Obj *o, s16 msg, void *arg) {
    StopperW *w = STOPPER(o);
    s32 pad;
    u32 data;

    switch (msg) {
        case 2:
            o->name = "atemAtari";
            data = eneSetup(o, arg);
            w->pol = eneNewPol(o, NULL);
            if (w->pol == NULL) {
                return -1;
            }
            w->pol->scale[0] = (data & 0xFF) * 0.1;
            w->pol->scale[1] = ((data >> 8) & 0xFF) * 0.1;
            w->pol->scale[2] = ((data >> 16) & 0xFF) * 0.1;
            w->ride = rideCreate(gRideTree1, (s32) o, &w->pol->f, (RideData *) eneFileAddr(o, itemAtariRide, 0));
            DPRINTF("itemAtari_proc: アイテムあたりセットＯＫ[%f]\n", w->pol->scale[0]);
            break;
        case 1:
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            if (w->ride != NULL) {
                frmDestroy(&w->ride->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneMove((EneObj *) w);
            if (w->pol != NULL) {
                eneSetModelOnCourse(o, &w->pol->f);
            }
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "atemAtari" ("demoAuto"): starts its talk at once, the players standing, and ends with it
 * (flag 0x92 reset). */
s32 demoAuto_proc(Obj *o, s16 msg, void *arg) {
    DemoAutoW *w = DEMOAUTO(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "atemAtari";
            if (!swWorkTest(0x92)) {
                DPRINTF("demoAuto_proc: オートデモ始まりません\n");
            }
            w->talk = eneSetup(o, arg);
            DPRINTF("demoAuto_proc: オートデモセットＯＫ\n");
            break;
        case 1:
            break;
        case 0:
            switch (w->state) {
                case 0:
                    scRunStart(w->talk, 0, 0);
                    demoPlStandAll();
                    w->state = 1;
                    break;
                case 1:
                    if (!scIsRunning()) {
                        demoPlReleaseAll();
                        swWorkReset(0x92);
                        return -1;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "timer" ("demoTimer"): the time left to a goal, hours and minutes on a clock under its own
 * camera. Its placement holds the goal's hour (byte 0), minute (byte 1) and how many days on
 * (byte 2); at the goal it stops (state 2).
 * Matched 2026-10-16; it was parked for a spare ugen integer temp (the days' srl one register
 * late); reading the days back as a masked byte (as itemAtari_proc already does) and writing
 * the now-test with the call result first (IDO_NOTES 30) closed it. */
s32 demoTimer_proc(Obj *o, s16 msg, void *arg) {
    s32 pad_x[6]; /* FAKE: frame words (greedy's pad) */
    s32 perHour = 60;
    u32 data;
    s32 j;
    s32 d[4];
    u16 now;
    s32 left;
    s32 h;
    s32 i;
    TimerW *w = TIMER(o);

    switch (msg) {
        case 2:
            o->name = "timer";
            data = eneSetup(o, arg);
            w->days = (data >> 16) & 0xFF;
            w->goal = (data & 0xFF) * perHour + ((data >> 8) & 0xFF);
            DPRINTF(">>>>>  目標(時間、分) [%d]時[%d]分[%d]日\n", data & 0xFF, (data >> 8) & 0xFF, data >> 16);
            w->now = demoTimeOfDayMinutes() - 1;
            if (w->goal < w->now) {
                w->goal += 1440;
            }
            w->view = (CamView *) frmCreate((Frame *) gFramePool10, gFrmKindCamera);
            if (w->view == NULL) {
                DPRINTF(">>>>>  demoTimer_proc cam つくれません\n");
                return -1;
            }
            w->view->ortho |= 1;
            w->view->zNear = -200.0f;
            w->view->zFar = 200.0f;
            DPRINTF("demoTimer_proc: セットＯＫ\n");
            break;
        case 1:
            /* The first destroy on one line (the game's style, IDO_NOTES 324, 347). */
            if (w->view != NULL) { frmDestroy(&w->view->f); }
            if (w->back != NULL) {
                frmDestroy(&w->back->f);
            }
            if (w->back2 != NULL) {
                frmDestroy(&w->back2->f);
            }
            if (w->colon != NULL) {
                frmDestroy(&w->colon->f);
            }
            for (j = 0; j < 4; j++) {
                if (w->digit[j] != NULL) {
                    frmDestroy(&w->digit[j]->f);
                }
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    DPRINTF("TIMER_SETUP\n");
                    w->back = (PolFrame *) frmCreate(&w->view->f, gFrmKindPol);
                    if (w->back == NULL) {
                        DPRINTF(">>>>>  demoTimer_proc back つくれません\n");
                        return -1;
                    }
                    w->back->dl = (Gfx *) fadeSegToRam(demoTimerFaceDl, o->file);
                    w->back->dl2 = demoTimerBackModelDl;
                    w->back->flags |= 0x20;
                    floadGetSetRam((void **) w->back->segs, 0xDC);
                    w->back->pos[0] = 22.0f;
                    w->back->pos[1] = 42.0f;
                    w->back->scale[2] = 0.028f;
                    w->back->scale[0] = w->back->scale[1] = w->back->scale[2];
                    w->back2 = (PolFrame *) frmCreate(&w->view->f, gFrmKindPol);
                    if (w->back2 == NULL) {
                        DPRINTF(">>>>>  demoTimer_proc back つくれません\n");
                        return -1;
                    }
                    w->back2->dl = (Gfx *) fadeSegToRam(demoTimerBackDl, o->file);
                    w->back2->dl2 = demoTimerBack2Dl;
                    w->back2->flags |= 0x20;
                    floadGetSetRam((void **) w->back2->segs, 0xDC);
                    w->back2->rot[0] = 3.141592741f;
                    w->back2->pos[0] = 22.0f;
                    w->back2->pos[1] = 42.0f;
                    w->back2->scale[2] = 0.028f;
                    w->back2->scale[0] = w->back2->scale[1] = w->back2->scale[2];
                    w->colon = (PolFrame *) frmCreate(&w->view->f, gFrmKindPol);
                    if (w->colon == NULL) {
                        DPRINTF(">>>>>  demoTimer_proc  ten つくれません\n");
                        return -1;
                    }
                    w->colon->dl = (Gfx *) fadeSegToRam(demoTimerDigitDl, o->file);
                    w->colon->dl2 = demoTimerColonDl;
                    w->colon->flags |= 0x20;
                    ((s32 (*)(void **ram, s32 n)) floadGetSetRam)((void **) w->colon->segs, 0xDC);
                    w->colon->pos[0] = w->back->pos[0] + 34.0f;
                    w->colon->pos[1] = w->back->pos[1] + 9.0f;
                    w->colon->scale[2] = 0.24f;
                    w->colon->scale[0] = w->colon->scale[1] = w->colon->scale[2];
                    w->shown[3] = 21;
                    w->shown[2] = 21;
                    w->shown[1] = 21;
                    w->shown[0] = 21;
                    w->state = 1;
                    break;
                case 1:
                    now = demoTimeOfDayMinutes();
                    if (w->now != now) {
                        w->now = now;
                        if (now == 0) {
                            if (w->days != 0) {
                                w->goal -= 1440;
                            }
                            w->days--;
                        }
                        if (w->goal - now < 0) {
                            w->state = 2;
                            break;
                        }
                        h = (w->goal - now) / perHour;
                        left = (w->goal - now) % perHour;
                        if (h) {} /* FAKE: lifts h and the difference above left (left to a0) */
                        if (h <= 0) {
                            d[0] = 0;
                            d[1] = 0;
                        } else {
                            d[0] = h / 10;
                            d[1] = h % 10;
                        }
                        if (left <= 0) {
                            d[2] = 0;
                            d[3] = 0;
                        } else {
                            d[2] = left / 10;
                            d[3] = left % 10;
                        }
                        for (i = 0; i < 4; i++) {
                            if (w->shown[i] != d[i]) {
                                if (w->digit[i] != NULL) {
                                    w->digit[i] = (PolFrame *) frmDestroy(&w->digit[i]->f);
                                }
                                w->shown[i] = d[i];
                                w->digit[i] = (PolFrame *) frmCreate(&w->view->f, gFrmKindPol);
                                if (w->digit[i] == NULL) {
                                    DPRINTF(">>>>>  demoTimer_proc counter[%d] つくれません\n", i);
                                    return -1;
                                }
                                w->digit[i]->dl = (Gfx *) fadeSegToRam(demoTimerDigitDl, o->file);
                                w->digit[i]->dl2 = (Gfx *) demoTimerDigitTex[d[i]];
                                w->digit[i]->flags |= 0x20;
                                floadGetSetRam((void **) w->digit[i]->segs, 0xDC);
                                if (i < 2) {
                                    w->digit[i]->pos[0] = w->back->pos[0] + 12.0f + i * 10;
                                } else {
                                    w->digit[i]->pos[0] = w->back->pos[0] + 27.0f + i * 10;
                                }
                                w->digit[i]->pos[1] = w->back->pos[1] + 9.0f;
                                w->digit[i]->scale[2] = 0.24f;
                                w->digit[i]->scale[0] = w->digit[i]->scale[1] = w->digit[i]->scale[2];
                            }
                        }
                    }
                    w->state = 1;
                    break;
                case 2:
                    break;
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The time of day in minutes (hours * 60 + minutes). */
u16 demoTimeOfDayMinutes(void) {
    s32 h = gTimeOfDay / 0x10000;
    s32 m = gTimeOfDay - (h << 16);

    m = m / 0x444;
    return h * 60 + m;
}

/* "tegata": the pass (kind 0x1C3). It rises turning (0), sinks (1) and bobs (2) until it is
 * sent to a player (state 3, file 126's tsuchiPassToPlayer and file 137's kensukeTegataSend);
 * then its glow goes, the player's character cries out, and 30 ticks on so does the other
 * player's.
 * Matched 2026-10-11; it was parked for the frmDestroy address's addiu sitting in message 1's
 * beqz delay slot ahead of w's addiu (as in money and demoTimer, and tlb_0048's gareki);
 * putting the first destroy and the second test on one source line (IDO_NOTES 324,
 * kensukeMoneyProc's find) gave it. */
s32 demoTegataProc(Obj *o, s16 msg, void *arg) {
    TegataW *w = TEGATA(o);
    s32 pad[3];

    switch (msg) {
        case 2:
            o->name = "tegata";
            eneSetup(o, arg);
            w->pol = eneNewPol(o, demoTegataDl);
            if (w->pol == NULL) {
                return -1;
            }
            w->pol->flags |= 0x20;
            w->pol->scale[2] = 0.1f;
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
            w->glow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->glow == NULL) {
                return -1;
            }
            w->glow->dl2 = demoTegataGlowDl;
            w->glow->flags |= 0x23;
            floadGetSetRam((void **) w->glow->segs, 0x120);
            w->glow->scale[2] = 0.5f;
            w->glow->scale[0] = w->glow->scale[1] = w->glow->scale[2];
            w->glow->pos[0] = w->pol->pos[0];
            w->glow->pos[1] = w->pol->pos[1];
            w->glow->pos[2] = w->pol->pos[2];
            w->alpha = 160;
            DPRINTF("手形効果セットＯＫ\n");
            break;
        case 3:
            if (w->glow != NULL) {
                w->glow->dl = eneDlEnvPrimColor(o, w->rgb[0], w->rgb[1], w->rgb[2], w->rgb[0], w->rgb[1],
                                                w->rgb[2], w->alpha);
            }
            break;
        case 1:
            if (w->pol != NULL) { frmDestroy(&w->pol->f); } if (w->glow != NULL) {
                frmDestroy(&w->glow->f);
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (w->timer == 0) {
                        sndPlay(0x159);
                    }
                    w->pol->pos[1] += 1.0f;
                    w->pol->rot[1] += 0.5235987902f;
                    w->glow->pos[1] = w->pol->pos[1];
                    if (40 / gRetracesPerFrame < w->timer) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    w->pol->pos[1] -= 1.0f;
                    w->pol->rot[1] += 0.5235987902f;
                    w->glow->pos[1] = w->pol->pos[1];
                    if (10 / gRetracesPerFrame < w->timer) {
                        w->pol->flags |= 2;
                        w->timer = 0;
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (w->timer == 40) {
                        w->timer = 0;
                    }
                    w->pol->pos[1] += 0.4 * mathCos(w->timer * 9 * 3.141592741f / 180.0f);
                    w->glow->pos[1] += 0.4 * mathCos(w->timer * 9 * 3.141592741f / 180.0f);
                    break;
                case 3:
                    w->pol->f.flags &= ~2;
                    if (w->glow != NULL) {
                        w->glow = (PolFrame *) frmDestroy(&w->glow->f);
                    }
                    sndPlay(0xB8);
                    soundIncamCam(0x233, w->pol->pos[0], w->pol->pos[1], w->pol->pos[2]);
                    w->timer = 0;
                    w->state = 4;
                    break;
                case 4:
                    if (w->timer == 66.0 / gRetracesPerFrame) {
                        demoTegataGetVoice(w->player);
                        if (plGetIndex(w->player) == 0) {
                            DPRINTF("****>>>  1P=");
                            w->other = gGameWork.players[1];
                        } else {
                            DPRINTF("****>>>  2P=");
                            w->other = gGameWork.players[0];
                        }
                        if (w->other == NULL) {
                            return -1;
                        }
                        w->timer = 0;
                        w->state = 5;
                    }
                    break;
                case 5:
                    if (w->timer == 30 / gRetracesPerFrame) {
                        if (w->other != NULL) {
                            if (plIsInPlay(plGetIndex(w->other))) {
                                demoTegataGetVoice(w->other);
                            }
                            return -1;
                        }
                    }
                    break;
            }
            if (w->timer % 10 == 0) {
                w->rgb[0] = fadeRandRange(0x80, 0xFF);
                w->rgb[1] = fadeRandRange(0x80, 0xFF);
                w->rgb[2] = fadeRandRange(0x80, 0xFF);
            }
            if (w->glow != NULL) {
                if (!(w->timer & 1)) {
                    w->glow->scale[0] += 0.3;
                    w->glow->scale[1] += 0.3;
                    w->glow->scale[2] += 0.3;
                } else {
                    w->glow->scale[0] -= 0.3;
                    w->glow->scale[1] -= 0.3;
                    w->glow->scale[2] -= 0.3;
                }
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The player's character's cry (0x16F, 0x16E, 0x170, 0x171) where the player is. */
void demoTegataGetVoice(Obj *player) {
    u16 se;
    Vec3f pos;

    switch (plGetChara(player)) {
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
    plGetModelPos(player, &pos.x, &pos.y, &pos.z);
    soundIncamCam(se, pos.x, pos.y, pos.z);
}

/* "chouninAtari" ("jizou"): a townsman's stopper, a talker the players stand against. Its
 * placement holds its radius and height (bytes 0, 1), whether it is seen (byte 2) and whether it
 * stays where it is put, not falling (byte 3). */
s32 demoJizouProc(Obj *o, s16 msg, void *arg) {
    ChouninW *w = CHOUNIN(o);
    s32 pad;
    u32 data;

    switch (msg) {
        case 2:
            o->name = "chouninAtari";
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) demoJizouAnim);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("jizou_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->own.talk = 0;
            w->own.path = 0;
            w->own.radiusHeight[0] = data & 0xFF;
            w->own.radiusHeight[1] = (data >> 8) & 0xFF;
            if (((data >> 16) & 0xFF) == 0) {
                w->model->f.flags &= ~2;
                DPRINTF("*** debug: 姿消す\n");
            }
            w->data = &w->own;
            if ((data >> 24) & 0xFF) {
                w->stay |= 1;
            }
            DPRINTF("町人あたりストッパーセット r=[%d] h=[%d]\n", w->own.radiusHeight[0], w->own.radiusHeight[1]);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            if (!(w->stay & 1)) {
                eneFall(o, 0.4f);
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
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
