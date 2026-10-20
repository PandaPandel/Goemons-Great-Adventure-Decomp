#include "tlb.h"

/* File 137: Kensuke and the money. "kensuke", who is out while the hide-and-seek runs and,
 * talked to, opens "money" (kind 0x1C4), the box where the player pays a debt off in ryo; paid,
 * he shakes the ground three times ("obisuAtari", a fading flash) and gives the pass (kind
 * 0x1C3) as file 126's old man does. "kenzou" is a second talker. */

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* Kensuke's work, at its task + 0x20: a demo talker's (PersonObj) up to 0x44. */
typedef struct KensukeW {
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
    /* 0x34 */ PersonData *data;
    /* 0x38 */ ACollFrame *box; /* its talk box */
    /* 0x3C */ u8 flags3C;
    /* 0x3D */ u8 count3D;
    /* 0x3E */ u8 pad3E[2];
    /* 0x40 */ Obj *talker; /* the player talking to it */
    /* 0x44 */ MPolFrame *shadow;
    /* 0x48 */ u8 shakes; /* the ground shakes so far */
    /* 0x49 */ u8 pad49[3];
    /* 0x4C */ Obj *money; /* the money box (kind 0x1C4) */
    /* 0x50 */ Obj *pass; /* the pass (kind 0x1C3) */
    /* 0x54 */ s32 timer; /* ticks in the state */
    /* 0x58 */ u8 flags; /* 1: the sky's tunes change 300 ticks after the pass */
    /* 0x59 */ u8 tuneTimer;
    /* 0x5A */ u8 pad5A[2];
    /* 0x5C */ Camera *cam;
    /* 0x60 */ f32 fov; /* the camera's, kept while it closes in */
} KensukeW;

#define KENSUKE(obj) ((KensukeW *) FX(obj))

/* Kenzou's work: Kensuke's without the pass. */
typedef struct KenzouW {
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
    /* 0x34 */ PersonData *data;
    /* 0x38 */ ACollFrame *box; /* its talk box */
    /* 0x3C */ u8 flags3C;
    /* 0x3D */ u8 count3D;
    /* 0x3E */ u8 pad3E[2];
    /* 0x40 */ Obj *talker; /* the player talking to it */
    /* 0x44 */ MPolFrame *shadow;
    /* 0x48 */ u8 shakes; /* the ground shakes so far */
    /* 0x49 */ u8 pad49[3];
    /* 0x4C */ Obj *money; /* the money box (kind 0x1C4) */
    /* 0x50 */ s32 timer; /* ticks in the state */
    /* 0x54 */ Camera *cam;
    /* 0x58 */ f32 fov; /* the camera's, kept while it closes in */
} KenzouW;

#define KENZOU(obj) ((KenzouW *) FX(obj))

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

/* The money box's work (kind 0x1C4), at its task + 0x20. */
typedef struct MoneyW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ CamView *view; /* its own camera, for the digits */
    /* 0x38 */ PolFrame *rest[4]; /* the debt left, ones first */
    /* 0x48 */ PolFrame *input[3]; /* the sum to pay, ones first */
    /* 0x54 */ s8 digits[3]; /* the sum to pay: ones, tens, hundreds */
    /* 0x57 */ s8 cursor; /* the digit being set (1 or 2) */
    /* 0x58 */ u16 paid;
    /* 0x5A */ u8 pad5A[2];
    /* 0x5C */ s32 debt;
    /* 0x60 */ u8 flags; /* 1 done, 2 the cursor's digit shown, 4 the second debt, 8 paid */
    /* 0x61 */ u8 pad61[3];
    /* 0x64 */ s32 timer;
    /* 0x68 */ u8 ticks;
} MoneyW;

#define MONEY(obj) ((MoneyW *) FX(obj))

/* The sum to pay's digits read by name: IDO sorts a commutative op's operands when one is an
 * array element, and keeps the source order for a struct field (IDO_NOTES 272). */
typedef struct MoneyDigits {
    s8 ones;
    s8 tens;
    s8 hundreds;
} MoneyDigits;

#define DIGITS(w) ((MoneyDigits *) (w)->digits)

/* "obisuAtari"'s work: a flash that fades. */
typedef struct ObisuW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 state;
    /* 0x06 */ s16 alpha;
    /* 0x08 */ s32 env[3];
    /* 0x14 */ s32 prim[3];
    /* 0x20 */ s32 timer;
} ObisuW;

#define OBISU(obj) ((ObisuW *) FX(obj))

/* What kensukeObisuEffectMake passes "obisuAtari". */
typedef struct ObisuArg {
    /* 0x00 */ u8 pad00[0x24];
    /* 0x24 */ f32 size;
    /* 0x28 */ f32 pos[3];
} ObisuArg; /* size 0x34 */

/* acollAtkAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy,
                                     s16 oz, s32 a);

void kensukeTegataSend(Obj *o, Obj *player);
void kensukeSetAnime(Obj *o, s32 n);
s32 kensukeMoneyIsDone(Obj *o);
s32 kensukeMoneyRestCounter(Obj *o);
s32 kensukeMoneyInputCounter(Obj *o);
s32 kensukeObisuEffectProc(Obj *o, s16 msg, void *arg);
void kensukeObisuEffectMake(Obj *o, f32 x, f32 y, f32 z, f32 size);

extern u32 kensukeAnimes[]; /* Kensuke's animations */
extern Gfx kensukeMoneyDigitDl[]; /* the digits' display list */
extern u32 kensukeMoneyDigitTex[]; /* the digits' textures, 0-9 */

/* The file's .data, in address order (its externs above stay as declarations). */
extern u8 kensukeAnim[];
extern u8 kensukeMoneyDigitDl0[];
extern u8 kensukeMoneyDigitDl1[];
extern u8 kensukeMoneyDigitDl2[];
extern u8 kensukeMoneyDigitDl3[];
extern u8 kensukeMoneyDigitDl4[];
extern u8 kensukeAnim2[];
extern u8 kensukeMoneyDigitDl7[];
extern u8 kensukeMoneyDigitDl8[];
Gfx kensukeMoneyDigitDl[] = {
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
}; /* the digits' display list */
u32 kensukeAnimes[] = { (u32) kensukeAnim, 0x03000638, (u32) kensukeAnim2 }; /* Kensuke's animations */
u32 kensukeMoneyDigitTex[] = { (u32) kensukeMoneyDigitDl0, (u32) kensukeMoneyDigitDl1, (u32) kensukeMoneyDigitDl2, (u32) kensukeMoneyDigitDl3, (u32) kensukeMoneyDigitDl4, 0x030007C0, 0x030008A0, (u32) kensukeMoneyDigitDl7, (u32) kensukeMoneyDigitDl8, 0x03000B40, 0x00000000, 0x00000000, 0x00000000 }; /* the digits' textures, 0-9 */
/* GameState's fade (+0x25) and fadeFrom (+0, +2, +4): Kensuke's flash stores each of them with
 * its own lui, which IDO gives only to different names (one name's fields share a base register).
 * The fade has a name (gGameStateFade); fadeFrom[1] and [2] have none yet, so they are reached
 * through the nearest named neighbours, gGameStateFadeStart (+8) and gGameStateSkipDraw (+0x22).
 * Names for 0x800881C2 and 0x800881C4 in undefined_syms.txt would let them be written plainly. */
extern s8 gGameStateFade;
extern u16 gGameStateFadeStart;
extern s8 gGameStateSkipDraw;
extern u8 kensukeAnim[];
extern Gfx kensukeObisuEffectDl[];

void kensukeNop(void) {
}

/* The pass goes to the player (state 3). */
void kensukeTegataSend(Obj *o, Obj *player) {
    PassW *w = (PassW *) FX(o);

    w->state = 3;
    w->player = player;
    w->timer = 0;
}

/* Two empty functions whose prints are compiled out. */
void kensukeEventTegataSdSet(void) {
    DPRINTF("&&&& eventTegataSdSet  pl D がないn");
}

void kensukeEventTegataSdClear(void) {
    DPRINTF("&&&& SD_G_CLEAR1n");
    DPRINTF("&&&& SD_E_CLEAR1n");
    DPRINTF("&&&& SD_S_CLEAR1n");
    DPRINTF("&&&& SD_Y_CLEAR1n");
}

/* "kensuke". Out (gone) while the hide-and-seek (flag 0x13 without 0xAC) runs. Talked to, he
 * asks for the debt (0x269); taken on (0x26), the money box opens; paid, he talks (0x26A) and,
 * if the player is owed the pass (0x25), the screen flashes, the camera closes in and the ground
 * shakes three times; then he talks again (0x26B), gives the pass, and the sky's tunes change. */
s32 kensukeProc(Obj *o, s16 msg, void *arg) {
    KensukeW *w = KENSUKE(o);
    u32 data;
    s32 pad[2];
    s32 i;
    s32 pad2;
    f32 x;
    f32 y;
    f32 z;
    Vec3f spark;

    switch (msg) {
        case 2:
            o->name = "kensuke";
            if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
                DPRINTF("****  カクレンボ中につき不在\n");
                return -1;
            }
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) kensukeAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->rot[1] = 1.171962976f;
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 100, 0, 0);
            demoTalkInit(o, data);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("tsuchiJisan_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
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
                        gGameWork.scenarioRunning = 2;
                        personCamZoomUp(w->talker);
                        kensukeSetAnime(o, 1);
                        w->state = 1;
                    }
                    break;
                case 1:
                    scRunStart(0x269, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    w->state = 2;
                    break;
                case 2:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x26)) {
                            swWorkReset(0x26);
                            w->money = eneMakeEx(NULL, ENE_KENSUKE_MONEY, 0, 0.0f, 0.0f, 0.0f, 2, NULL);
                            w->state = 3;
                        } else {
                            w->state = 9;
                        }
                    }
                    break;
                case 3:
                    if (kensukeMoneyIsDone(w->money)) {
                        w->timer = 0;
                        w->state = 4;
                    }
                    break;
                case 4:
                    if (50 / gRetracesPerFrame < w->timer) {
                        w->money = tskDestroy(w->money);
                        scRunStart(0x26A, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        w->state = 5;
                    }
                    break;
                case 5:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x25)) {
                            fadeSetAll(40 / gRetracesPerFrame, 0.0f, 255.0f, 255, 160, 160);
                            w->timer = 0;
                            w->state = 10;
                        } else {
                            w->state = 9;
                        }
                    }
                    break;
                case 10:
                    if (w->timer == 40 / gRetracesPerFrame) {
                        fadeSetAll(40 / gRetracesPerFrame, 255.0f, 0.0f, 255, 160, 160);
                        kensukeSetAnime(o, 2);
                        gGameStateFade = 0;
                        gGameState.fadeFrom.r = 0xFF;
                        ((u16 *) &gGameStateFadeStart)[-3] = 0; /* fadeFrom[1] */
                        ((u16 *) &gGameStateSkipDraw)[-15] = 0; /* fadeFrom[2] */
                        w->x = -11.0f;
                        w->z = -94.0f;
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        w->fov = camGetView(w->cam)->fovy;
                        camSetEye(w->cam, 34.0f, 11.0f, -90.0f);
                        camSetAt(w->cam, -125.0f, 50.0f, -98.0f);
                        camSetFov(w->cam, 0.6597344279f);
                        camSetSpeed(w->cam, 40 / gRetracesPerFrame);
                        camMoveEye(w->cam, 24.0f, 14.0f, -91.0f);
                        camMoveAt(w->cam, -141.0f, 53.0f, -99.0f);
                        camMoveFov(w->cam, 0.4997349977f);
                        w->timer = 0;
                        w->state = 11;
                    }
                    break;
                case 11:
                    if (w->timer == (40 / gRetracesPerFrame) / 2) {
                        w->timer = 0;
                        w->state = 12;
                    }
                    break;
                case 12:
                    if (w->timer == 1) {
                        kensukeObisuEffectMake(o, w->model->pos[0], w->model->pos[1], w->model->pos[2], 500.0f);
                        fadeRumbleAll(0x66443322, 3);
                    }
                    if (eneAnimLoop(o, w->model, 0.8f)) {
                        fadeRumbleAll(0x66443322, 3);
                        w->shakes++;
                    }
                    if (w->shakes >= 3) {
                        w->timer = 0;
                        w->state = 13;
                    }
                    break;
                case 13:
                    if (w->timer == 1) {
                        for (i = 0; i < 4; i++) {
                            contPakInitRumble(i);
                            contPakRumbleRequestStop(i);
                        }
                        fadeSetAll(40 / gRetracesPerFrame, 0.0f, 255.0f, 255, 160, 160);
                    }
                    if (w->timer == 40 / gRetracesPerFrame) {
                        gGameStateFade = 1;
                        w->x = -25.0f;
                        w->z = 6.0f;
                        camSetEye(w->cam, 85.0f, 20.0f, 10.0f);
                        camSetAt(w->cam, w->x, 25.0f, w->z);
                        camSetFov(w->cam, w->fov);
                        kensukeSetAnime(o, 1);
                        fadeSetAll(40 / gRetracesPerFrame, 255.0f, 0.0f, 255, 160, 160);
                    }
                    if (w->timer == (40 / gRetracesPerFrame) * 2) {
                        scRunStart(0x26B, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        w->timer = 0;
                        w->state = 6;
                    }
                    break;
                case 6:
                    if (!scIsRunning()) {
                        if (w->talker == NULL) {
                            return -1;
                        }
                        plGetModelPos(w->talker, &x, &y, &z);
                        w->pass = eneMakeEx(NULL, ENE_DEMO_TEGATA, 0, (w->model->pos[0] + x) / 2, w->model->pos[1],
                                            (w->model->pos[2] + z) / 2, 0, NULL);
                        w->timer = 0;
                        w->state = 7;
                    }
                    break;
                case 7:
                    if (w->timer == 200 / gRetracesPerFrame) {
                        demoGetEneModelPos(w->pass, &spark);
                        ptclSpawnSparksFall(spark.x, spark.y, spark.z);
                        eneMakeEx(NULL, ENE_DEMO_KIRAKIRA, 0, spark.x, spark.y, spark.z, 0, NULL);
                        kensukeTegataSend(w->pass, w->talker);
                        w->flags |= 1;
                    }
                    if (w->timer == 260 / gRetracesPerFrame) {
                        scRunStart(0x26C, 0, 0);
                        w->state = 8;
                    }
                    break;
                case 8:
                    if (!scIsRunning()) {
                        w->state = 9;
                    }
                    break;
                case 9:
                    gGameWork.scenarioRunning = 0;
                    kensukeSetAnime(o, 0);
                    personTalkCamOff();
                    demoFinish(o);
                    w->state = 0;
                    break;
            }
            if (w->flags & 1) {
                if (300 / gRetracesPerFrame == w->tuneTimer++) {
                    w->flags &= ~1;
                    camSunClearTunes();
                    camSunSetTunes4(0x89, 0x86, 0x87, 0x88);
                }
            }
            if (w->state < 10) {
                eneAnimLoop(o, w->model, 0.5f);
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Kensuke's animation n, from its start. */
void kensukeSetAnime(Obj *o, s32 n) {
    KensukeW *w = KENSUKE(o);

    w->model->anim = kensukeAnimes[n];
    w->model->frame = 0.0f;
}

/* "money" (kind 0x1C4): the box where a player pays Kensuke's debt (3500 ryo; 100 for the second,
 * flag 4). Its own camera shows the debt left (four digits) and the sum to pay (three digits, the
 * tens or hundreds blinking): left and right move the cursor, up and down set the digit, A pays
 * (the sum, or what is left if it is more; never more than the player has), B leaves. The paid
 * sum counts down into the debt, 1 ryo a step (4 above 40). Flag 1 tells Kensuke it is done.
 * Matched 2026-10-11; it was parked for message 1's frmDestroy address taking the beqz's delay
 * slot ahead of w's addiu (file 138's tegata and demoTimer, and tlb_0048's gareki, show the
 * same); the permuter's one-line layout of the first destroy and the first loop's head
 * (IDO_NOTES 324) gave it. */
s32 kensukeMoneyProc(Obj *o, s16 msg, void *arg) {
    MoneyW *w = MONEY(o);
    s32 i;
    s32 step;
    s32 rest;
    s32 pay;
    s32 port;
    s32 pad;
    s32 sum;
    s32 pad2;

    switch (msg) {
        case 2:
            o->name = "money";
            switch ((u8) eneSetup(o, arg)) {
                case 0:
                    w->paid = gSaveData.debtPaid;
                    w->debt = 3500;
                    break;
                case 1:
                    w->flags |= 4;
                    w->paid = gSaveData.debt2Paid;
                    w->debt = 100;
                    break;
            }
            scRunStart(0x26D, 0, 0);
            w->view = (CamView *) frmCreate((Frame *) gFramePool10, gFrmKindCamera);
            if (w->view == NULL) {
                DPRINTF(">>>>>  money_proc cam つくれません\n");
                return -1;
            }
            w->view->ortho |= 1;
            w->view->zNear = -200.0f;
            w->view->zFar = 200;
            w->cursor = 1;
            DPRINTF("money: セットＯＫ\n");
            break;
        case 1:
            /* The view's destroy on one line (the game's style, IDO_NOTES 324), so IDO schedules
             * w's addiu into the beqz's slot as the game does. */
            if (w->view != NULL) { frmDestroy(&w->view->f); }
            for (i = 0; i < 4; i++) {
                if (w->rest[i] != NULL) {
                    frmDestroy(&w->rest[i]->f);
                }
            }
            for (i = 0; i < 3; i++) {
                if (w->input[i] != NULL) {
                    frmDestroy(&w->input[i]->f);
                }
            }
            scRunEnd();
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (kensukeMoneyRestCounter(o) == -1) {
                        return -1;
                    }
                    if (kensukeMoneyInputCounter(o) == -1) {
                        return -1;
                    }
                    w->state = 1;
                    break;
                case 1:
                    port = demoFindPlByChara(0);
                    joySetStickDpad(port, 1);
                    if (joyGetHeldTime(port, 0x200) == 1 ||
                        (joyGetHeldTime(port, 0x200) >= 21 && joyGetHeldTime(port, 0x200) % 5 == 1)) {
                        sndPlay(0x204);
                        w->cursor++;
                        if (w->cursor >= 3) {
                            w->cursor = 1;
                        }
                        w->timer = 0;
                        w->flags |= 2;
                    } else if (joyGetHeldTime(port, 0x100) == 1 ||
                               (joyGetHeldTime(port, 0x100) >= 21 && joyGetHeldTime(port, 0x100) % 5 == 1)) {
                        sndPlay(0x204);
                        w->cursor--;
                        if (w->cursor <= 0) {
                            w->cursor = 2;
                        }
                        w->timer = 0;
                        w->flags |= 2;
                    } else if (joyGetHeldTime(port, 0x800) == 1 ||
                               (joyGetHeldTime(port, 0x800) >= 21 && joyGetHeldTime(0, 0x800) % 5 == 1)) {
                        sndPlay(0x204);
                        w->digits[w->cursor]++;
                        if (w->digits[w->cursor] >= 10) {
                            w->digits[w->cursor] = 0;
                        }
                        w->timer = 0;
                        w->flags |= 2;
                    } else if (joyGetHeldTime(port, 0x400) == 1 ||
                               (joyGetHeldTime(port, 0x400) >= 21 && joyGetHeldTime(0, 0x400) % 5 == 1)) {
                        sndPlay(0x204);
                        w->digits[w->cursor]--;
                        if (w->digits[w->cursor] < 0) {
                            w->digits[w->cursor] = 9;
                        }
                        w->timer = 0;
                        w->flags |= 2;
                    } else if (joyGetHeldTime(port, 0x8000) == 1) {
                        w->timer = 0;
                        w->flags |= 2;
                        if (gSaveData.players[port].money - w->digits[0] - w->digits[2] * 100 - w->digits[1] * 10 < 0) {
                            DPRINTF(">>> プレーヤー所持金たらん\n");
                            w->state = 3;
                        } else if (DIGITS(w)->ones + DIGITS(w)->hundreds * 100 + DIGITS(w)->tens * 10 == 0) {
                            DPRINTF(">>> プレーヤー０円払うな\n");
                            w->state = 4;
                        } else if (w->debt - w->paid < DIGITS(w)->ones + DIGITS(w)->hundreds * 100 + DIGITS(w)->tens * 10) {
                            DPRINTF(">>> 借金返済しすぎ\n");
                            rest = w->debt - w->paid;
                            w->digits[2] = rest / 100;
                            w->digits[1] = (rest - w->digits[2] * 100) / 10;
                            pay = DIGITS(w)->tens * 10 + DIGITS(w)->hundreds * 100;
                            w->digits[0] = rest - pay;
                            gSaveData.players[port].money = gSaveData.players[port].money - pay - w->digits[0];
                            joySetStickDpad(port, 0);
                            sndPlay(0x205);
                            w->timer = 0;
                            w->flags |= 8;
                            w->state = 2;
                        } else {
                            gSaveData.players[port].money = gSaveData.players[port].money - w->digits[2] * 100 -
                                                            w->digits[1] * 10 - w->digits[0];
                            joySetStickDpad(port, 0);
                            sndPlay(0x205);
                            w->timer = 0;
                            w->flags |= 8;
                            w->state = 2;
                        }
                    } else if (joyGetHeldTime(port, 0x4000) == 1) {
                        w->timer = 0;
                        w->state = 4;
                    }
                    if (kensukeMoneyInputCounter(o) == -1) {
                        return -1;
                    }
                    if (20 / gRetracesPerFrame < w->timer) {
                        if (w->flags & 2) {
                            w->flags &= ~2;
                        } else {
                            w->flags |= 2;
                        }
                        w->timer = 0;
                    }
                    if (w->flags & 2) {
                        w->input[w->cursor]->f.flags |= 2;
                    } else {
                        w->input[w->cursor]->f.flags &= ~2;
                    }
                    break;
                case 2:
                    if (!(w->ticks++ & 1)) {
                        sndPlay(0x2F3);
                    }
                    sum = DIGITS(w)->ones + DIGITS(w)->hundreds * 100 + DIGITS(w)->tens * 10;
                    step = (sum > 40) ? 4 : 1;
                    sum -= step;
                    if (sum > 40 || w->timer >= 2 / gRetracesPerFrame) {
                        DIGITS(w)->hundreds = sum / 100;
                        DIGITS(w)->tens = (sum - DIGITS(w)->hundreds * 100) / 10;
                        DIGITS(w)->ones = sum - DIGITS(w)->tens * 10 - DIGITS(w)->hundreds * 100;
                        w->paid += step;
                        if (w->flags & 4) {
                            gSaveData.debt2Paid += step;
                        } else {
                            gSaveData.debtPaid += step;
                        }
                        if (kensukeMoneyRestCounter(o) == -1) {
                            return -1;
                        }
                        if (kensukeMoneyInputCounter(o) == -1) {
                            return -1;
                        }
                        if (sum <= 0) {
                            w->timer = 0;
                            w->state = 4;
                        }
                        if (sum <= 40) {
                            w->timer = 0;
                        }
                    }
                    break;
                case 3:
                    if (20 / gRetracesPerFrame < w->timer) {
                        w->state = 1;
                    }
                    break;
                case 4:
                    if (w->flags & 8) {
                        if (w->timer < 24 / gRetracesPerFrame) {
                            if (!(w->ticks++ & 1)) {
                                sndPlay(0x2F3);
                            }
                        }
                        if (w->timer == 24 / gRetracesPerFrame) {
                            sndPlay(0x209);
                        }
                    }
                    if (w->timer == 60 / gRetracesPerFrame) {
                        w->flags |= 1;
                    }
                    break;
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The money box is done. */
s32 kensukeMoneyIsDone(Obj *o) {
    if (MONEY(o)->flags & 1) {
        return TRUE;
    }
    return FALSE;
}

/* The money box's debt left, made again: four digits under its camera. */
s32 kensukeMoneyRestCounter(Obj *o) {
    MoneyW *w = MONEY(o);
    s32 i;
    s32 rest;
    s32 digit[4];

    for (i = 0; i < 4; i++) {
        if (w->rest[i] != NULL) {
            w->rest[i] = (PolFrame *) frmDestroy(&w->rest[i]->f);
        }
    }
    rest = w->debt - w->paid;
    digit[3] = rest / 1000;
    digit[2] = (rest - digit[3] * 1000) / 100;
    digit[1] = (rest - (digit[3] * 1000 + digit[2] * 100)) / 10;
    digit[0] = rest - digit[1] * 10 - (digit[3] * 1000 + digit[2] * 100);
    for (i = 0; i < 4; i++) {
        w->rest[i] = (PolFrame *) frmCreate(&w->view->f, gFrmKindPol);
        if (w->rest[i] == NULL) {
            DPRINTF(">>>>>  money rest counter[%d] つくれません\n", i);
            return -1;
        }
        w->rest[i]->dl = (Gfx *) fadeSegToRam(kensukeMoneyDigitDl, o->file);
        w->rest[i]->dl2 = (Gfx *) kensukeMoneyDigitTex[digit[i]];
        w->rest[i]->flags |= 0x20;
        floadGetSetRam((void **) w->rest[i]->segs, 0xDC);
        w->rest[i]->pos[0] = 140 - i * 10;
        w->rest[i]->pos[1] = 148.0f;
        w->rest[i]->scale[2] = 0.3f;
        w->rest[i]->scale[0] = w->rest[i]->scale[1] = w->rest[i]->scale[2];
    }
    return 0;
}

/* The money box's sum to pay, made again: three digits under its camera. */
s32 kensukeMoneyInputCounter(Obj *o) {
    MoneyW *w = MONEY(o);
    s32 i;

    for (i = 0; i < 3; i++) {
        if (w->input[i] != NULL) {
            w->input[i] = (PolFrame *) frmDestroy(&w->input[i]->f);
        }
    }
    for (i = 0; i < 3; i++) {
        w->input[i] = (PolFrame *) frmCreate(&w->view->f, gFrmKindPol);
        if (w->input[i] == NULL) {
            DPRINTF(">>>>>  money input counter[%d] つくれません\n", i);
            return -1;
        }
        w->input[i]->dl = (Gfx *) fadeSegToRam(kensukeMoneyDigitDl, o->file);
        w->input[i]->dl2 = (Gfx *) kensukeMoneyDigitTex[w->digits[i]];
        w->input[i]->flags |= 0x20;
        floadGetSetRam((void **) w->input[i]->segs, 0xDC);
        w->input[i]->pos[0] = 140 - i * 10;
        w->input[i]->pos[1] = 176.0f;
        w->input[i]->scale[2] = 0.3f;
        w->input[i]->scale[0] = w->input[i]->scale[1] = w->input[i]->scale[2];
    }
    return 0;
}

/* "kenzou": Kensuke's twin (a demo talker). Talked to, he asks for his debt (0x1EB); taken on
 * (0x26), the money box opens for the second debt (100 ryo); paid, he talks (0x1EC) and, if
 * flag 0x39 is set, the screen flashes, the camera closes in and the ground shakes once; then
 * he talks twice more (0x1ED, 0x1EE). */
s32 kensukeKenzouProc(Obj *o, s16 msg, void *arg) {
    KenzouW *w = KENZOU(o);
    u32 data;
    s32 i;
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "kenzou";
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) kensukeAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->rot[1] = 1.006291986f;
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 100, 0, 0);
            demoTalkInit(o, data);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("*** kenzou_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
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
                        demoCamZoom_CL(w->talker);
                        kensukeSetAnime(o, 1);
                        w->state = 1;
                    }
                    break;
                case 1:
                    scRunStart(0x1EB, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    w->state = 2;
                    break;
                case 2:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x26)) {
                            swWorkReset(0x26);
                            w->money = eneMakeEx(NULL, ENE_KENSUKE_MONEY, 0, 0.0f, 0.0f, 0.0f, 2,
                                                 (u32 (*)(void *, void *, void *)) 1);
                            w->state = 3;
                        } else {
                            w->state = 9;
                        }
                    }
                    break;
                case 3:
                    if (kensukeMoneyIsDone(w->money)) {
                        w->timer = 0;
                        w->state = 4;
                    }
                    break;
                case 4:
                    if (50 / gGameState.retracesPerFrame < w->timer) {
                        tskDestroy(w->money);
                        scRunStart(0x1EC, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        w->state = 5;
                    }
                    break;
                case 5:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x39)) {
                            fadeSetAll(40 / gGameState.retracesPerFrame, 0.0f, 255.0f, 175, 160, 255);
                            w->timer = 0;
                            w->state = 10;
                        } else {
                            w->state = 9;
                        }
                    }
                    break;
                case 10:
                    if (w->timer == 40 / gGameState.retracesPerFrame) {
                        fadeSetAll(40 / gGameState.retracesPerFrame, 255.0f, 0.0f, 160, 160, 255);
                        kensukeSetAnime(o, 2);
                        gGameState.fade = 0;
                        gGameState.fadeFrom.r = 0xFF;
                        gGameState.fadeFrom.g = 0;
                        gGameState.fadeFrom.b = 0;
                        w->x = -23.0f;
                        w->z = -83.0f;
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        w->fov = camGetView(w->cam)->fovy;
                        camSetEye(w->cam, 22.0f, 11.0f, -71.0f);
                        camSetAt(w->cam, -131.0f, 46.0f, -106.0f);
                        camSetFov(w->cam, 0.6597344279f);
                        w->timer = 0;
                        w->state = 11;
                    }
                    break;
                case 11:
                    if (w->timer == (40 / gGameState.retracesPerFrame) / 2) {
                        w->timer = 0;
                        w->state = 12;
                    }
                    break;
                case 12:
                    if (w->timer == 1) {
                        kensukeObisuEffectMake(o, w->model->pos[0], w->model->pos[1], w->model->pos[2], 20);
                        fadeRumbleAll(0x66443322, 3);
                    }
                    if (eneAnimLoop(o, w->model, 0.8f)) {
                        fadeRumbleAll(0x66443322, 3);
                        w->shakes++;
                    }
                    if (w->shakes > 0) {
                        w->timer = 0;
                        w->state = 13;
                    }
                    break;
                case 13:
                    if (w->timer == 1) {
                        for (i = 0; i < 4; i++) {
                            contPakInitRumble(i);
                            contPakRumbleRequestStop(i);
                        }
                        fadeSetAll(40 / gGameState.retracesPerFrame, 0.0f, 255.0f, 160, 160, 255);
                    }
                    if (w->timer == 40 / gGameState.retracesPerFrame) {
                        gGameState.fade = 1;
                        w->x = -35.0f;
                        w->z = 0.0f;
                        camSetEye(w->cam, 85.0f, 20.0f, 20.0f);
                        camSetAt(w->cam, w->x, 25.0f, w->z);
                        camSetFov(w->cam, w->fov);
                        kensukeSetAnime(o, 1);
                        fadeSetAll(40 / gGameState.retracesPerFrame, 255.0f, 0.0f, 160, 160, 255);
                    }
                    if (w->timer == (40 / gGameState.retracesPerFrame) * 2) {
                        scRunStart(0x1ED, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        w->timer = 0;
                        w->state = 6;
                    }
                    break;
                case 6:
                    if (!scIsRunning()) {
                        sndPlay(0x233);
                        scRunStart(0x1EE, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        w->state = 8;
                    }
                    break;
                case 8:
                    if (!scIsRunning()) {
                        w->state = 9;
                    }
                    break;
                case 9:
                    personTalkCamOff();
                    kensukeSetAnime(o, 0);
                    demoFinish(o);
                    w->state = 0;
                    break;
            }
            if (w->state < 10) {
                eneAnimLoop(o, w->model, 0.5f);
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "obisuAtari": a flash at the given place that fades out. */
s32 kensukeObisuEffectProc(Obj *o, s16 msg, void *arg) {
    ObisuW *w = OBISU(o);
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "obisuAtari";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            w->frame->dl2 = kensukeObisuEffectDl;
            w->frame->flags |= 0x20;
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->flags |= 7;
            floadGetSetRam((void **) w->frame->segs, 0x120);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = ((ObisuArg *) arg)->size;
            w->frame->pos[0] = ((ObisuArg *) arg)->pos[0];
            w->frame->pos[1] = ((ObisuArg *) arg)->pos[1];
            w->frame->pos[2] = ((ObisuArg *) arg)->pos[2];
            w->env[2] = w->env[1] = w->env[0] = 255;
            w->prim[2] = 0;
            w->prim[1] = w->prim[0] = 255;
            w->alpha = 240;
            DPRINTF("obisuEffect_proc: effect セットＯＫ\n");
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 3:
            if (w->frame != NULL) {
                w->frame->dl = eneDlEnvPrimColor(o, w->env[0], w->env[1], w->env[2], w->prim[0], w->prim[1],
                                                 w->prim[2], w->alpha);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    w->timer = 0;
                    w->state = 1;
                    break;
                case 1:
                    w->alpha -= 10;
                    if (w->alpha <= 0) {
                        w->alpha = 0;
                        return -1;
                    }
                    break;
            }
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Makes an "obisuAtari" flash of the given size at x, y, z. */
void kensukeObisuEffectMake(Obj *o, f32 x, f32 y, f32 z, f32 size) {
    ObisuArg a;

    a.pos[0] = x;
    a.pos[1] = y;
    a.pos[2] = z;
    a.size = size;
    tskCreate(o, kensukeObisuEffectProc, o->file, &a);
}
