#include "tlb.h"
#include "tlb/tlb_0324.h"

/* The boss's work (obj + 0x20): tlb_0324_1's BtaiW. */
typedef struct BtaiW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ s32 target[3]; /* a place it heads for */
    /* 0x1C */ f32 count;
    /* 0x20 */ PolFrame *drop; /* the model that drops in after the death (btaiDeathDemo) */
    /* 0x24 */ ACollFrame *atk;
    /* 0x28 */ ACollFrame *dmg;
    /* 0x2C */ Obj *cam; /* "dracam", the opening camera's */
    /* 0x30 */ u8 pad30[0x8];
    /* 0x38 */ CamView *view;
    /* 0x3C */ s32 invTimer; /* steps of invincibility left after a hit (tlb_0324_1) */
    /* 0x40 */ u8 pad40[8];
    /* 0x48 */ f32 speed;
    /* 0x4C */ u8 action;
    /* 0x4D */ u8 endPattern; /* 1: end the pattern now (tlb_0324_1, _2) */
    /* 0x4E */ u8 step;
    /* 0x4F */ u8 unused4F;
    /* 0x50 */ s16 timer;
} BtaiW;

/* tlb_0324_4's: keeps the boss's frame inside the arena (called near). */
/* This file calls its neighbours' functions far, through t9 (as tlb_0324_1). */
#define btaiRand (*btaiRand)
#define btaiRandSigned (*btaiRandSigned)
#define btaiBmoveX (*btaiBmoveX)
#define btaiTurnTo (*btaiTurnTo)
#define btaiTbubbleMake (*btaiTbubbleMake)
#define btaiT3bubbleMake (*btaiT3bubbleMake)
#define btaiTbubbleMakeRand (*btaiTbubbleMakeRand)
#define btaiRumble (*btaiRumble)
#define btaiTsbubbleMake (*btaiTsbubbleMake)
#define sunaEfMake (*sunaEfMake)
#define btaiUzu1Make (*btaiUzu1Make)
#define btaiUzuSnaMake (*btaiUzuSnaMake)
#define taisambaCreateTask (*taisambaCreateTask)
#define btaiKiranMake (*btaiKiranMake)
#define sinf (*sinf)
#define cosf (*cosf)

extern s32 btaiNewAction;
extern s32 btaiEffectsEnd; /* 1: the battle's effects end (tlb_0324_6) */
extern Obj *gBattleTask; /* the battle's task (beffect.c) */
extern s32 btaiEnding; /* the ending: 0 none (tlb_0324_0) */
extern s32 btaiWhirl1FadeCount; /* the first whirlpool's count where it fades out */
extern f32 btaiWhirl1Scale; /* its pieces' scale */
extern s32 btaiWhirl1Count; /* its count */
extern s32 btaiWhirl1Life; /* its pieces' life */
extern f32 btaiWhirl1Turn; /* its turning speed, in degrees */
extern f32 btaiWhirl1Grow; /* its pieces' growth */
extern f32 btaiWhirlPoint1X; /* the first point's x, y, z */
extern f32 btaiWhirlPoint1Y;
extern f32 btaiWhirlPoint1Z;
extern f32 btaiWhirlPoint2; /* the second point's x, y, z */
extern f32 btaiWhirlPoint2Y;
extern f32 btaiWhirlPoint2Z;
extern u8 btaiEntranceDemoAnim[]; /* a model animation (file 0xF8) */
extern u8 btaiEntranceDemoStep4Anim[]; /* a model animation (file 0xF8) */
extern u8 btaiDeathDemoAnim[]; /* a model animation (file 0xF8) */
extern u8 btaiDeathDemoDroppingDl[]; /* the dropping model's display list (file 0xF6) */
extern u8 btaiWinDemoStep0Anim[];
extern u8 btaiWinDemoStep1Anim[];
extern u8 btaiWinDemoStep2Anim[];
extern u8 btaiWinDemoStep3Anim[];

/* The file's .data, in address order (0x08019F90-0x08019FEF), each word at the place of the
 * function that reads it. */
f32 btaiOpeningFov = 50.0f; /* the opening camera's field of view, in degrees */

/* The sea bream's opening (its action 0). The boss swims in from far away, circling, while
 * the whirlpools turn and the controller rumbles (state 1). It slows by the Impact (2), rears
 * up with two splashes (3), waits (4), plays its roar and is lit by a sparkle (5, 6), and the
 * camera returns to the cockpit (7, 8). Start (as in tlb_0325_3's opening) or a battle after a
 * lost one skips to state 9: the boss is placed at once and the letterbox opens (10). */
s32 btaiEntranceDemo(Obj *obj) {
    static s32 slowed = 0; /* 1 once the camera has slowed for the approach */
    static s32 unused = 0;
    static s32 frame = 0;
    BtaiW *w = FX(obj);
    f32 dx;
    f32 dz;
    f32 s;
    f32 c;
    f32 turn;
    s32 pad[11];
    s32 i;
    s32 j;

    PRINTF("登場DEMO\n");
    PRINTF("[tai]登場デモをキャンセルします\n");
    PRINTF("[tai]登場デモをキャンセルします\n");
    PRINTF("demotimer --->%d\r");
    PRINTF("---------------  DEMO START  --------------------\n");
    PRINTF("<DEMO>タイサンバ表示消します\n");
    PRINTF("<DEMO>外部インパクト表示消します\n");
    PRINTF("<DEMO>照準表示消します\n");
    PRINTF("<DEMO>コックピット表示消します\n");
    PRINTF("<DEMO>コックピットLOCKします\n");
    PRINTF("STEP1へ\n");
    PRINTF("カメラ速度変わります\n");
    PRINTF("UZU:目的地に来ました\n");
    PRINTF("<DEMO>タイサンバ表示します\n");
    PRINTF("ライト暗く\n");
    PRINTF("くわ\n");
    PRINTF("STEP6\n");
    PRINTF("<DEMO>照準表示します\n");
    PRINTF("<DEMO>外部インパクト表示します\n");
    PRINTF("<DEMO>コックピット表示します\n");
    PRINTF("---------------------------  デモ終了  ----------------------------\n");
    PRINTF("<DEMO>タイサンバ表示します\n");
    PRINTF("<DEMO>照準表示します\n");
    PRINTF("<DEMO>外部インパクト表示します\n");
    PRINTF("<DEMO>コックピット表示します\n");
    PRINTF("---------------------------  デモ終了  ----------------------------\n");
    if (btaiNewAction == 1) {
        btaiNewAction = 0;
    }
    if (gGameFrameCount % 30 == 0) {
        btaiTbubbleMakeRand(obj);
    }
    if (w->step < 9 && frame < 0x546) {
        if (fadeCountPlayers() == 1) {
            if (joyGetHeldTime(gCockpitPort, gCockpitBtnStart) == 1) {
                w->step = 9;
                btaiEffectsEnd = 1;
                for (i = 0; i < 4; i++) {
                    contPakInitRumble(i);
                    contPakRumbleRequestStop(i);
                }
                fadeSetLetterbox(320, 120, 0, 120);
                sndPlay(10);
                sndPlay(0x7C);
            }
        } else if (fadeCountPlayers() >= 2
                   && (joyGetHeldTime(gCockpitPort, gCockpitBtnStart) == 1
                       || joyGetHeldTime(gBrotherPort, gBrotherBtnStart) == 1)) {
            w->step = 9;
            btaiEffectsEnd = 1;
            for (j = 0; j < 4; j++) {
                contPakInitRumble(j);
                contPakRumbleRequestStop(j);
            }
            fadeSetLetterbox(320, 120, 0, 120);
            sndPlay(10);
            sndPlay(0x7C);
        }
    }
    frame++;
    if (w->step < 9) {
        if (frame == 470) {
            sndPlay(0x7B);
        }
        if (frame == 0x546) {
            scRunStart(3, 0, 0);
        }
        if (frame == 0x56A) {
            sndPlay(0x122);
        }
        if (frame == 0x57F) {
            camSetSpeed((Camera *) w->cam, 125.0f);
            camMoveEye((Camera *) w->cam, 0.0f, 0.0f, 0.0f);
            camMoveAt((Camera *) w->cam, 0.0f, 0.0f, -300.0f);
        }
    }
    if (w->step >= 7) {
        if (w->frame->pos[1] < sinf(w->count / 180.0f * 3.1415927f) * 80.0 + -600.0 + -6.0) {
            w->frame->pos[1] += 3.0f;
        }
        if (w->frame->pos[1] > sinf(w->count / 180.0f * 3.1415927f) * 80.0 + -600.0 + 6.0) {
            w->frame->pos[1] -= 3.0f;
        }
    }
    switch (w->step) {
        case 0:
            w->timer = 800;
            bossSetAnimAt(w->frame, btaiEntranceDemoAnim, 0xF8, 0.0f);
            fadeSetLetterbox(320, 120, 0, 120);
            w->frame->f.flags &= ~2;
            gBrotherFlags |= 0x10000;
            gBattleFlags |= 0x10000000;
            gBattleFlags |= 0x1000000;
            gBattleFlags2 |= 0x400;
            btaiOpeningFov = 50.0f;
            if (w->cam == NULL) {
                w->cam = tskCreate(obj, dracam, 0, NULL);
            }
            camSetFov((Camera *) w->cam, btaiOpeningFov * 0.0174532930038800785);
            camSetSpeed((Camera *) w->cam, 320.0f);
            camSetEye((Camera *) w->cam, 1000.0f, -200.0f, -1000.0f);
            camSetAt((Camera *) w->cam, -4000.0f, -300.0f, -2400.0f);
            w->view = camGetView((Camera *) w->cam);
            w->view->zFar = 20000.0f;
            w->count = 0.0f;
            btaiWhirl1Count = 0;
            btaiWhirl1FadeCount = 600;
            btaiWhirl1Turn = 1.0f;
            btaiWhirl1Scale = 0.1f;
            btaiWhirl1Life = 170;
            btaiWhirl1Grow = 0.01f;
            w->frame->pos[0] = -10000.0f;
            w->frame->pos[2] = -16000.0f;
            w->frame->pos[1] = -600.0f;
            slowed = 0;
            frame = 0;
            w->step = 1;
            if (gGameWork.losses) {
                w->step = 9;
                sndPlay(10);
                sndPlay(0x7C);
                sndSetReverb(1, 15);
                break;
            }
            sndPlay(1);
            sndPlay(0x121);
            sndSetReverb(1, 15);
            break;
        case 1:
            w->count += 1.0f;
            if (!(gGameFrameCount & 1)) {
                fadeMoveLetterbox(50, 190, 1);
            }
            btaiWhirl1Count++;
            turn = w->count / 600.0f + 0.1;
            if (turn > 0.4) {
                turn = 0.4f;
            }
            btaiWhirl1Turn = turn * 30.0f;
            btaiWhirl1Scale += 0.002;
            if (btaiWhirl1Scale > 4.0f) {
                btaiWhirl1Scale = 4.0f;
            }
            camGetEye((Camera *) w->cam, &btaiWhirlPoint1X, &btaiWhirlPoint1Y, &btaiWhirlPoint1Z);
            btaiTurnTo(obj, btaiWhirlPoint1X, btaiWhirlPoint1Z, 1.0f);
            if (!(frame & 7)) {
                btaiRumble(0x55555555, 8);
            }
            if (sqrtf((w->frame->pos[2] - -2000.0f) * (w->frame->pos[2] - -2000.0f)
                      + w->frame->pos[0] * w->frame->pos[0]) < 4000.0f
                && slowed == 0) {
                camSetSpeed((Camera *) w->cam, 100.0f);
                slowed++;
                sndSetReverb(1, 15);
            }
            if (gGameFrameCount % 7 == 0) {
                btaiUzu1Make();
            }
            if (w->timer % 50 == 6) {
                btaiUzuSnaMake();
            }
            dx = (w->frame->pos[0] < 0.0f) ? 1.0f : -1.0f;
            dz = (w->frame->pos[2] < -2000.0f) ? 1.0f : -1.0f;
            s = sinf(w->count * 0.0174532930038800785) * 15.0f;
            c = cosf(w->count * 0.0174532930038800785) * 15.0f;
            w->vel[0] += dx;
            w->vel[2] += dz;
            if (((w->vel[0] > 0.0f) ? w->vel[0] : -w->vel[0]) > 20.0f) {
                w->vel[0] -= dx;
            }
            if (((w->vel[2] > 0.0f) ? w->vel[2] : -w->vel[2]) > 20.0f) {
                w->vel[2] -= dz;
            }
            w->frame->pos[0] += w->vel[0];
            w->frame->pos[2] += w->vel[2];
            btaiWhirlPoint2 = w->frame->pos[0] + s;
            btaiWhirlPoint2Z = w->frame->pos[2] + c;
            btaiWhirlPoint2Y = -800.0f;
            camMoveEye((Camera *) w->cam, 0.0f, -100.0f, 0.0f);
            camMoveAt((Camera *) w->cam, w->frame->pos[0], -300.0f, w->frame->pos[2]);
            if (sqrtf((w->frame->pos[2] - -2000.0f) * (w->frame->pos[2] - -2000.0f)
                      + w->frame->pos[0] * w->frame->pos[0]) < 400.0f) {
                camSetSpeed((Camera *) w->cam, 30.0f);
                w->step = 2;
                w->timer = 140;
                btaiWhirlPoint2 = w->frame->pos[0] + s;
                btaiWhirlPoint2Y = w->frame->pos[1] + c;
            }
            break;
        case 2:
            if (!(frame & 7)) {
                btaiRumble(0x88888888, 8);
            }
            btaiWhirl1Count++;
            if (!(gGameFrameCount & 1) && w->timer > 50) {
                fadeMoveLetterbox(50, 190, 1);
            }
            camMoveEye((Camera *) w->cam, 0.0f, -100.0f, w->frame->pos[2] + 1000.0f);
            camMoveAt((Camera *) w->cam, w->frame->pos[0], -100.0f, w->frame->pos[2]);
            w->frame->pos[0] *= 0.98;
            camGetEye((Camera *) w->cam, &btaiWhirlPoint1X, &btaiWhirlPoint1Y, &btaiWhirlPoint1Z);
            btaiTurnTo(obj, btaiWhirlPoint1X, btaiWhirlPoint1Z, 1.0f);
            if (btaiWhirl1Count % 7 == 0) {
                btaiUzu1Make();
                taisambaCreateTask();
            }
            if (w->timer % 50 == 6) {
                btaiUzuSnaMake();
            }
            if (w->timer < 20 && w->timer % 5 == 0) {
                btaiUzuSnaMake();
            }
            if (w->timer < 10) {
                w->step = 3;
                w->timer = 400;
                w->frame->f.flags |= 2;
                camSetSpeed((Camera *) w->cam, 30.0f);
            }
            break;
        case 3:
            if (!(frame & 7)) {
                btaiRumble(-1, 8);
            }
            btaiWhirl1Count++;
            camGetEye((Camera *) w->cam, &btaiWhirlPoint1X, &btaiWhirlPoint1Y, &btaiWhirlPoint1Z);
            btaiTurnTo(obj, btaiWhirlPoint1X, btaiWhirlPoint1Z, 1.0f);
            if (btaiWhirl1Count % 7 == 0) {
                btaiUzu1Make();
            }
            camMoveEye((Camera *) w->cam, w->frame->pos[0], 100.0f, w->frame->pos[2] + 400.0f);
            camMoveAt((Camera *) w->cam, w->frame->pos[0], 100.0f, w->frame->pos[2]);
            if (w->timer == 100) {
                sndPlay(0x120);
                btaiKiranMake(w->frame->pos[0] + 54.0f, w->frame->pos[1] + 710.0f, w->frame->pos[2] + 290.0f);
                btaiKiranMake(w->frame->pos[0] - 58.0f, w->frame->pos[1] + 710.0f, w->frame->pos[2] + 290.0f);
            }
            if (w->timer == 200) {
                gBattleFlags2 |= 1;
            }
            if (w->timer < 20) {
                gBattleFlags2 &= ~1;
                w->step = 4;
                w->timer = 400;
                camSetSpeed((Camera *) w->cam, 120.0f);
                camMoveEye((Camera *) w->cam, 0.0f, -100.0f, w->frame->pos[2] + 1000.0f);
                w->count = 0.0f;
            }
            break;
        case 4:
            camGetEye((Camera *) w->cam, &btaiWhirlPoint1X, &btaiWhirlPoint1Y, &btaiWhirlPoint1Z);
            if (w->count++ >= 120.0f) {
                w->step = 5;
                w->timer = 600;
                w->count = 0.0f;
                bossSetAnimAt(w->frame, btaiEntranceDemoStep4Anim, 0xF8, 0.0f);
                camSetEye((Camera *) w->cam, btaiWhirlPoint1X, btaiWhirlPoint1Y, btaiWhirlPoint1Z);
                camSetSpeed((Camera *) w->cam, 1000.0f);
            }
            break;
        case 5:
            if (w->count++ > 10.0f) {
                effectKiranCreate(w->frame->pos[0] + 500.0f, w->frame->pos[1] + 700.0f, w->frame->pos[2] - 300.0f);
                unused = 0;
                w->step = 6;
                w->count = 0.0f;
                camSetSpeed((Camera *) w->cam, 1000.0f);
                camMoveEye((Camera *) w->cam, 0.0f, 0.0f, 0.0f);
                camMoveAt((Camera *) w->cam, 0.0f, 0.0f, -2000.0f);
            }
            camMoveEye((Camera *) w->cam, 0.0f, 0.0f, 0.0f);
            break;
        case 6:
            btaiTurnTo(obj, btaiWhirlPoint1X, btaiWhirlPoint1Z, 1.0f);
            if (bossStepAnim(w->frame, 0.063f, 2) == 1) {
                w->step = 7;
                w->timer = 800;
                bossSetAnimAt(w->frame, btaiEntranceDemoAnim, 0xF8, 0.0f);
            }
            camSetFov((Camera *) w->cam, btaiOpeningFov * 0.0174532930038800785);
            btaiOpeningFov = btaiOpeningFov;
            if (btaiOpeningFov != 0.0f) {
                btaiOpeningFov = 55.0f;
            }
            break;
        case 7:
            gCockpitAimX = gCockpitAimY = gCockpitAimZ = 0.0f;
            gCockpitHeadPitch = gCockpitHeadYaw = 0.0f;
            gBattleFlags &= ~0x10000000;
            gBrotherFlags &= ~0x10000;
            gBattleFlags &= ~0x1000000;
            w->step = 8;
            w->timer = 400;
            w->count = 0.0f;
            break;
        case 8:
            w->count += 1.0f;
            bossStepAnim(w->frame, 0.2f, 0);
            camGetEye((Camera *) w->cam, &btaiWhirlPoint1X, &btaiWhirlPoint1Y, &btaiWhirlPoint1Z);
            camMoveAt((Camera *) w->cam, gCockpitLookAtX, gCockpitLookAtY, gCockpitLookAtZ);
            if (btaiWhirlPoint1Z > -1.0f && fadeMoveLetterbox(0, 240, 1)) {
                w->step = 0;
                btaiEnding = 0;
                btaiWhirlPoint1Z = 0.0f;
                w->vel[0] = 0.0f;
                w->vel[2] = 0.0f;
                tskDestroy(w->cam);
                w->cam = NULL;
                gBattleFlags2 &= ~0x400;
                effectKiranCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 500.0f);
                gBossActionArg = 0;
                sndSetReverb(1, 15);
            }
            w->frame->pos[1] = sinf(w->count / 180.0f * 3.1415927f) * 80.0 + -600.0;
            break;
        case 9:
            sndSetReverb(1, 15);
            gCockpitAimX = gCockpitAimY = gCockpitAimZ = 0.0f;
            gCockpitHeadPitch = gCockpitHeadYaw = 0.0f;
            w->frame->pos[0] = 0.0f;
            w->frame->pos[2] = -2000.0f;
            w->frame->pos[1] = -600.0f;
            w->frame->rot[2] = 0.0f;
            w->frame->rot[0] = w->frame->rot[1] = w->frame->rot[2];
            w->vel[0] = 0.0f;
            w->vel[1] = 0.0f;
            w->vel[2] = 0.0f;
            w->frame->f.flags |= 2;
            gBattleFlags &= ~0x10000000;
            gBrotherFlags &= ~0x10000;
            gBattleFlags &= ~0x1000000;
            w->step = 10;
            gBattleFlags2 &= ~1;
            w->timer = 400;
            w->count = 60.0f;
            tskDestroy(w->cam);
            w->cam = NULL;
            fadeOpenLetterbox();
            break;
        case 10:
            bossStepAnim(w->frame, 0.1f, 0);
            btaiEffectsEnd = 0;
            if (fadeMoveLetterbox(0, 240, 1) == 1) {
                btaiEnding = 0;
                w->step = 0;
                w->count = 0.0f;
                gBossActionArg = 0;
                gBattleFlags2 &= ~0x400;
                sndSetReverb(1, 15);
                effectKiranCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 500.0f);
            }
            break;
    }
    return 0;
}

s32 btaiEntranceDemoUnused[8] = { 0 }; /* read by no code */

/* The sea bream's death and the battle's win (tlb_0325_3's bfriDeathDemo for this boss). It
 * shakes, rocks and topples with sand bursts, red stars and bubbles, and explodes at frame 900
 * (states 1-7). Then a model from file 0xF6 drops from the sky and bobs (8-10), scenario 14
 * plays (11), the model flies away (12), the letterbox closes on the clear (13, 14), and the
 * battle is won (15). */
s32 btaiDeathDemo(Obj *obj) {
    static s32 frame = 0;
    BtaiW *w = FX(obj);
    s32 pad[10];

    PRINTF("死にDEMO\n");
    PRINTF("\r demo_timer[%d]");
    if (btaiNewAction == 1) {
        btaiNewAction = 0;
    }
    if (w->step > 0) {
        frame++;
    }
    switch (w->step) {
        case 0:
            fadeCreate(gEffectTask);
            frame = 0;
            w->timer = 0x3880;
            w->frame->rot[1] = 0.0f;
            w->frame->rot[0] = w->frame->rot[2] = w->frame->rot[1];
            w->frame->scale[1] = 1.9f;
            w->frame->scale[0] = w->frame->scale[2] = w->frame->scale[1];
            gBattleFlags2 |= 0x400;
            gBossFlags |= 0x800;
            gBattleFlags2 |= 0x1000;
            gBattleFlags2 |= 0x1000000;
            sndPlay(9);
            w->vel[0] = w->vel[1] = w->vel[2] = 0.0f;
            frame = 0;
            gBossPosX = w->frame->pos[0] = 2000.0f;
            gBossPosY = w->frame->pos[1] = -30.0f;
            gBossPosZ = w->frame->pos[2] = -1000.0f;
            tskCreate(obj, Bappcam3, gBattleTask->file, NULL);
            bossSetAnimAt(w->frame, btaiDeathDemoAnim, 0xF8, 0.0f);
            effectKiranCreate(w->frame->pos[0], w->frame->pos[1] + -30.0f + 200.0f, w->frame->pos[2]);
            beFlashFCreate(255, 255, 255, 40);
            effectPompomCreate(w->frame->pos[0], 0.0f, w->frame->pos[2], 250.0f, 250.0f, 250.0f, 7, 0x226, 5, 0.8f, 0.1f);
            effectPompomCreate(w->frame->pos[0], 0.0f, w->frame->pos[2], 250.0f, 250.0f, 250.0f, 7, 0x226, 5, 0.8f, 0.1f);
            effectPompomCreate(w->frame->pos[0], 0.0f, w->frame->pos[2], 250.0f, 250.0f, 250.0f, 8, 0x226, 5, 0.8f, 0.1f);
            effectPompomCreate(w->frame->pos[0], 0.0f, w->frame->pos[2], 250.0f, 250.0f, 250.0f, 8, 0x226, 5, 0.8f, 0.1f);
            effectPompomCreate(w->frame->pos[0], 0.0f, w->frame->pos[2], 250.0f, 250.0f, 250.0f, 10, 0x226, 5, 0.8f, 0.1f);
            sndPlay(0x10E);
            w->step++;
            break;
        case 1:
            if (frame % 12 == 0) {
                sndPlay(0x513);
            }
            if (!(frame & 7)) {
                btaiRumble(0x88888888, 8);
            }
            if (frame < 30) {
                w->vel[1] = 4.0f;
            } else {
                w->vel[1] = 0.0f;
            }
            if (gGameFrameCount % 10 >= 5) {
                w->frame->pos[1] += 2.0f;
            } else {
                w->frame->pos[1] -= 2.0f;
            }
            bossStepAnim(w->frame, 0.2f, 0);
            if (frame == 240) {
                effectPompomCreate(w->frame->pos[0] + 200.0f, 200.0f, w->frame->pos[2], 300.0f, 50.0f, 300.0f, 10, 400, 6, 1.5f, 0.1f);
                effectPompomCreate(w->frame->pos[0] + 200.0f, 200.0f, w->frame->pos[2], 300.0f, 50.0f, 300.0f, 10, 400, 6, 1.0f, 0.1f);
                w->step++;
            }
            break;
        case 2:
            if (frame % 12 == 0) {
                sndPlay(0x513);
            }
            if (!(frame & 7)) {
                btaiRumble(0x88888888, 8);
            }
            bossStepAnim(w->frame, 0.2f, 0);
            if (gGameFrameCount % 10 >= 5) {
                w->frame->pos[1] += 2.0f;
            } else {
                w->frame->pos[1] -= 2.0f;
            }
            w->frame->rot[2] += 0.03490658600776016;
            if (w->frame->rot[2] > 0.31415927f) {
                w->frame->rot[2] = 0.31415927f;
                w->step++;
            }
            break;
        case 3:
            if (frame % 12 == 0) {
                sndPlay(0x513);
            }
            if (!(frame & 7)) {
                btaiRumble(0x88888888, 8);
            }
            w->frame->rot[2] -= 0.0174532930038800785;
            if (w->frame->rot[2] < 0.22689280908185694) {
                w->frame->rot[2] = 0.22689281f;
            }
            if (gGameFrameCount % 10 >= 5) {
                w->frame->pos[1] += 3.0f;
            } else {
                w->frame->pos[1] -= 3.0f;
            }
            bossStepAnim(w->frame, 0.2f, 0);
            if (frame == 480) {
                effectPompomCreate(w->frame->pos[0] - 200.0f, 100.0f, w->frame->pos[2], 200.0f, 100.0f, 200.0f, 10, 300, 6, 1.0f, 0.1f);
                effectPompomCreate(w->frame->pos[0] - 200.0f, 100.0f, w->frame->pos[2], 200.0f, 100.0f, 200.0f, 10, 300, 6, 1.0f, 0.1f);
                sndPlay(0x513);
                w->step++;
            }
            break;
        case 4:
            if (frame % 8 == 0) {
                sndPlay(0x513);
            }
            if (frame % 8 == 0) {
                btaiRumble(0x88888888, 8);
            }
            bossStepAnim(w->frame, 0.2f, 0);
            if (gGameFrameCount % 10 >= 5) {
                w->frame->pos[1] += 3.0f;
            } else {
                w->frame->pos[1] -= 3.0f;
            }
            w->frame->rot[2] -= 0.03490658600776016;
            if (w->frame->rot[2] < -0.31415927f) {
                w->frame->rot[2] = -0.31415927f;
                sndPlay(0x10F);
                w->step++;
            }
            break;
        case 5:
            if (frame % 8 == 0) {
                btaiRumble(0x88888888, 8);
            }
            if (frame % 8 == 0) {
                sndPlay(0x513);
            }
            bossStepAnim(w->frame, 0.2f, 0);
            if (gGameFrameCount % 10 >= 5) {
                w->frame->pos[1] += 4.0f;
            } else {
                w->frame->pos[1] -= 4.0f;
            }
            if (frame % 30 == 0) {
                beRedStarCreate(w->frame->pos[0], -500.0f, w->frame->pos[2]);
            }
            w->frame->rot[2] += 0.0174532930038800785;
            if (w->frame->rot[2] > -0.22689280908185694) {
                w->frame->rot[2] = -0.22689281f;
            }
            if (frame == 600) {
                effectPompomCreate(w->frame->pos[0], -150.0f, w->frame->pos[2], 250.0f, 250.0f, 250.0f, 7, 500, 5, 0.8f, 0.1f);
                effectPompomCreate(w->frame->pos[0], -150.0f, w->frame->pos[2], 250.0f, 250.0f, 250.0f, 7, 500, 5, 0.8f, 0.1f);
                effectPompomCreate(w->frame->pos[0], -150.0f, w->frame->pos[2], 250.0f, 250.0f, 250.0f, 8, 500, 5, 0.8f, 0.1f);
                effectPompomCreate(w->frame->pos[0], -150.0f, w->frame->pos[2], 250.0f, 250.0f, 250.0f, 8, 500, 5, 0.8f, 0.1f);
                effectPompomCreate(w->frame->pos[0], -150.0f, w->frame->pos[2], 250.0f, 250.0f, 250.0f, 10, 500, 5, 0.8f, 0.1f);
                w->step++;
            }
            break;
        case 6:
            if (frame % 8 == 0) {
                btaiRumble(0xA5294A52, 8);
            }
            if (frame % 8 == 0) {
                sndPlay(0x513);
            }
            bossStepAnim(w->frame, 0.2f, 0);
            if (gGameFrameCount % 10 >= 5) {
                w->frame->pos[1] += 5.0f;
            } else {
                w->frame->pos[1] -= 5.0f;
            }
            if (frame % 30 == 0) {
                beRedStarCreate(w->frame->pos[0], -500.0f, w->frame->pos[2]);
            }
            if (frame == 720) {
                btaiT3bubbleMake(w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2] + 100.0f);
                btaiT3bubbleMake(w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2] + 100.0f);
                btaiT3bubbleMake(w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2] + 100.0f);
                btaiT3bubbleMake(w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2] + 100.0f);
                btaiT3bubbleMake(w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2] + 100.0f);
                w->step++;
            }
            break;
        case 7:
            if (frame % 8 == 0) {
                btaiRumble(0xA5294A52, 8);
            }
            if (frame % 8 == 0 && frame < 900) {
                sndPlay(0x513);
            }
            bossStepAnim(w->frame, 0.2f, 0);
            if (gGameFrameCount % 10 >= 5) {
                w->frame->pos[1] += 6.0f;
            } else {
                w->frame->pos[1] -= 6.0f;
            }
            if (frame < 900 && frame % 20 == 0) {
                beRedStarCreate(w->frame->pos[0], -500.0f, w->frame->pos[2]);
            }
            w->frame->rot[0] += 0.006108652551358027;
            if (w->frame->rot[0] > 0.7853982f) {
                w->frame->rot[0] = 0.7853982f;
            }
            w->frame->pos[1] -= 1.75;
            if (frame == 960) {
                beHitballCreate(w->frame->pos[0], 0.0f, w->frame->pos[2], 3.0f);
            }
            if (frame == 870) {
                sndPlay(0x122);
            }
            if (frame == 900) {
                btaiRumble(-1, 50);
                beHitballCreate(w->frame->pos[0], 0.0f, w->frame->pos[2], 3.0f);
                beHitballCreate(w->frame->pos[0], 0.0f, w->frame->pos[2], 3.0f);
                beHitballCreate(w->frame->pos[0], 0.0f, w->frame->pos[2], 3.0f);
                beMeffectCreate(w->frame->pos[0], 0.0f, w->frame->pos[2]);
                beMeffectCreate(w->frame->pos[0], 200.0f, w->frame->pos[2]);
                beMeffectCreate(w->frame->pos[0], 300.0f, w->frame->pos[2]);
                fadeSetAll(30.0f, 0.0f, 255.0f, 255, 255, 255);
                sndPlay(0x11C);
            }
            if (frame == 1040) {
                gBossFlags &= ~0x800;
                gBattleFlags2 &= ~0x1000;
                gBattleFlags2 |= 0x800;
                w->step++;
            }
            break;
        case 8:
            if ((w->drop = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol)) == NULL) {
                return -1;
            }
            bossSetPolDL(w->drop, (Gfx *) btaiDeathDemoDroppingDl, 0xF6);
            bossSetFrameFlags((AnimeFrame *) w->drop, 0x20);
            w->drop->scale[2] = 1.7f;
            w->drop->scale[0] = w->drop->scale[1] = w->drop->scale[2];
            w->drop->pos[0] = 0.0f;
            w->drop->pos[1] = 2500.0f;
            w->drop->pos[2] = -500.0f;
            w->step++;
            break;
        case 9:
            if (frame % 15 == 0) {
                beSmoke6Create(w->drop->pos[0], w->drop->pos[1], w->drop->pos[2], 0.1f, 50.0f, 2);
            }
            if (frame % 15 == 5) {
                beSmoke6Create(w->drop->pos[0], w->drop->pos[1] - 150.0f, w->drop->pos[2], 0.2f, 50.0f, 2);
            }
            if (frame % 15 == 10) {
                btaiTsbubbleMake(w->drop->pos[0], w->drop->pos[1] - 300.0f, w->drop->pos[2]);
            }
            fadeOpenLetterbox();
            if (frame == 1110) {
                fadeSetAll(30.0f, 255.0f, 0.0f, 0, 0, 0);
            }
            if (frame == 1260) {
                sndPlay(0x123);
            }
            if (frame == 1200) {
                sndPlay(0x17);
            }
            w->drop->pos[1] -= 10.0f;
            if (w->drop->pos[1] < -140.0f) {
                w->drop->pos[1] = -140.0f;
                beSmoke6Create(w->drop->pos[0], w->drop->pos[1], w->drop->pos[2], 0.1f, 50.0f, 2);
                w->step++;
            }
            break;
        case 10:
            if (frame % 15 == 0) {
                beSmoke6Create(w->drop->pos[0], w->drop->pos[1], w->drop->pos[2], 0.1f, 50.0f, 2);
            }
            bossBobFrame((AnimeFrame *) w->drop, 1.0f, 2.0f);
            if (frame == 1400) {
                scRunStart(14, 0, 0);
                w->step++;
            }
            break;
        case 11:
            if (frame % 15 == 0) {
                beSmoke6Create(w->drop->pos[0], w->drop->pos[1], w->drop->pos[2], 0.1f, 50.0f, 2);
            }
            bossBobFrame((AnimeFrame *) w->drop, 1.0f, 2.0f);
            w->timer = 10000;
            if (!scIsRunning()) {
                sndPlay(0x124);
                w->step++;
            }
            break;
        case 12:
            if (frame % 5 == 0) {
                beSmoke6Create(w->drop->pos[0], w->drop->pos[1], w->drop->pos[2], 0.1f, 50.0f, 2);
            }
            w->drop->pos[1] += 20.0f;
            w->timer = 10000;
            if (w->drop->pos[1] > 2500.0f) {
                sndPlay(8);
                w->step++;
            }
            break;
        case 13:
            w->timer = 10000;
            if (fadeCloseLetterbox(50)) {
                sndClearReverb();
                clearPackCreate(0.0f, 0.0f, 0.0f, 0);
                w->step++;
            }
            break;
        case 14:
            if (gBattleFlags2 & 0x800000) {
                gBattleFlags2 &= ~0x800000;
                w->step++;
            }
            frame = w->timer = 10000;
            break;
        case 15:
            if (fadeCloseLetterbox(120)) {
                gBattleFlags2 &= ~0x1000000;
                swWorkSet(0xBE);
                bossBattleWin(gBattleBossId);
                dmenuQueueDemo(7);
            }
            break;
    }
    return 0;
}

s32 btaiDeathDemoUnused[8] = { 0 }; /* read by no code */

s32 btaiDefeatFrames = 0; /* the defeat's frames (only reset here) */

/* The sea bream's defeat (tlb_0325_3's bfriWinDemo for this boss): it rears up and sinks with
 * bubbles (state 1), rises to its goal (2), sways (3), plays its last animation and fades to
 * white (4), then, after 120 and 125 frames, the stinger and the battle's end (5, 6). Case 3's
 * zeros are 0.0 (IDO_NOTES 326). The bubble timer's if is on one line: over three, IDO built
 * gGameFrameCount's lui before btaiTbubbleMakeRand's address. */
s32 btaiWinDemo(Obj *obj) {
    BtaiW *w = FX(obj);
    s32 pad[11];

    PRINTF("勝ちDEMO\n");
    PRINTF("*** WIN_DEMO_STEP ---> %d\n");
    PRINTF("--------------------  WIN DEMO START  ------------------------\n");
    PRINTF("<DEMO>タイサンバ表示します\n");
    PRINTF("STEP1へ\n");
    btaiMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    if (btaiNewAction == 1) {
        btaiNewAction = 0;
    }
    if (gGameFrameCount % 40 == 0) { btaiTbubbleMakeRand(obj); }
    switch (w->step) {
        case 0:
            w->timer = 18000;
            bossSetAnimAt(w->frame, btaiWinDemoStep0Anim, 0xF8, 0.0f);
            gBattleFlags2 |= 0xC00;
            fadeSetLetterbox(320, 0, 0, 240);
            w->frame->f.flags |= 2;
            w->step = 1;
            btaiEffectsEnd = 1;
            btaiDefeatFrames = 0;
            effectKiranCreate(w->frame->pos[0], w->frame->pos[1] + 200.0f, w->frame->pos[2]);
            btaiT3bubbleMake(w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2]);
            w->target[0] = 0;
            w->target[2] = -2500;
            if (bossDistSqXZ(0.0f, -2500.0f, w->frame->pos[0], w->frame->pos[2]) < 90000.0f) {
                w->step = 2;
            }
            break;
        case 1:
            btaiEffectsEnd = 0;
            if (bossStepAnim(w->frame, 0.105f, 2) == 1) {
                w->step = 2;
                bossSetAnimAt(w->frame, btaiWinDemoStep1Anim, 0xF8, 0.0f);
                sunaEfMake(w->frame->pos[0], -850.0f, w->frame->pos[2], w->vel[0], w->vel[2], 6.0f, 0x80);
                btaiTurnTo(obj, w->target[0], w->target[2], 6.0f);
            } else {
                btaiTurnTo(obj, w->target[0], w->target[2], 4.0f);
                if (btaiRand(20) == 3) {
                    btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                                           btaiRandSigned(20), btaiRandSigned(20), 2.5f);
                }
            }
            break;
        case 2:
            if (!btaiBmoveX(obj, w->target[0], w->target[1] - 600, w->target[2], 1.3f)) {
                w->step = 3;
                sunaEfMake(w->frame->pos[0], -850.0f, w->frame->pos[2], w->vel[0], w->vel[2], 6.0f, 0x80);
                bossSetAnimAt(w->frame, btaiWinDemoStep2Anim, 0xF8, 0.0f);
                w->count = 0.0f;
            }
            if (!(gGameFrameCount & 1)) {
                fadeMoveLetterbox(50, 190, 1);
            } else {
                bossStepAnim(w->frame, ((25.0f - w->speed / 3.0f) / 12.0f) * 0.7, 0);
            }
            break;
        case 3:
            w->frame->rot[2] *= 0.98;
            w->frame->rot[0] *= 0.98;
            if (bossStepAnim(w->frame, 0.126f, 2)) {
                w->step = 4;
                bossSetAnimAt(w->frame, btaiWinDemoStep3Anim, 0xF8, 0.0f);
            }
            btaiTurnTo(obj, 0.0, 0.0, 5.0f);
            w->frame->pos[1] = sinf(w->count / 180.0f * 3.1415927f) * 60.0 + -600.0;
            w->count += 2.0f;
            bossDampVel(obj, 0.93f);
            break;
        case 4:
            w->frame->rot[2] *= 0.98;
            w->frame->rot[0] *= 0.98;
            if (bossStepAnim(w->frame, 0.2f, 2)) {
                fadeSetAll(120.0f, 0.0f, 255.0f, 255, 255, 255);
                w->step = 5;
                btaiTurnTo(obj, 0.0f, 0.0f, 2.0f);
                w->count = 0.0f;
            }
            break;
        case 5:
            w->count += 1.0f;
            if (w->count > 120.0f) {
                sndPlay(7);
                sndPlay(0x11D);
                gBattleFlags2 |= 0x4000000;
                w->step = 6;
                w->count = 0.0f;
            }
            break;
        case 6:
            w->count += 1.0f;
            fadeCloseLetterbox(120);
            if (w->count > 125.0f) {
                bossBattleLose((u8) gBattleBossId);
            }
            break;
    }
    return 0;
}
