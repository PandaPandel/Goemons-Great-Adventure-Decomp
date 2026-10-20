#include "tlb.h"
#include "tlb/tlb_0325.h"

/* The boss's work (at obj + 0x20): tlb_0325_1's BossW, with what this file uses. */
typedef struct BossW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ s32 goal[3]; /* a place to move to (bfriBmoveX) */
    /* 0x1C */ f32 count; /* +1 every step */
    /* 0x20 */ ACollFrame *atk; /* its attack box (made and freed by bfriProc, tlb_0325_0) */
    /* 0x24 */ void *link;
    /* 0x28 */ Anim anim;
    /* 0x38 */ Obj *cam; /* "dracam", the opening camera's */
    /* 0x3C */ u8 pad3C[0x8];
    /* 0x44 */ CamView *view;
    /* 0x48 */ s32 invTimer; /* steps of invincibility left after a hit (tlb_0325_1) */
    /* 0x4C */ u8 pad4C[8];
    /* 0x54 */ f32 speed;
    /* 0x58 */ u8 action;
    /* 0x59 */ u8 endPattern; /* 1: end the pattern now (tlb_0325_1, _2) */
    /* 0x5A */ u8 state; /* at 9 and up the opening camera ends early */
    /* 0x5C */ s16 time;
} BossW;

/* A "plasma" of a type at x, y, z (tlb_0325_6). */
#define bfriPlasmaMake (*bfriPlasmaMake)

/* tlb_0325_4's step (called near) and the boss's helpers (far, through t9); as tlb_0325_1. */
#define bfriBank (*bfriBank)
#define bfriTurnTo (*bfriTurnTo)
#define bfriBmoveX (*bfriBmoveX)
#define bfriSunaEfMake (*bfriSunaEfMake)

/* tlb_0325_4's and tlb_0325_6's. */
#define bfriRumble (*bfriRumble)
#define bfriSkyMake (*bfriSkyMake)
#define bfriThunderMake (*bfriThunderMake)
/* This file's own, called far as well (written (*f)() at the calls, as it is defined here). */
void bfriEntranceCam(Obj *obj, s32 frame, s32 arg);

#define sinf (*sinf)

extern f32 bfriFov; /* the camera's field of view, in degrees */
extern s32 bfriNewAction; /* 1: cleared by the boss's steps */
extern s32 bfriEffectsEnd;
extern s32 bfriBarrierState;
extern s32 bfriSkyShown; /* 0: the sky fades out */
extern s32 bfriFlashFrames; /* frames of lightning flash left */
extern s32 bfriBitsRemake;
extern Obj *gBattleTask; /* the battle's task (beffect.c) */
extern u8 bfriDeathDemoAnim[]; /* a model animation (file 0xFB) */
extern u8 bfriEntranceDemoAnim[]; /* a model animation (file 0xFB) */
extern u8 bfriAnim[]; /* a model animation (file 0xFB) */
extern u8 bfriWinDemoAnim[]; /* a model animation (file 0xFB) */

/* The file's .data, in address order (0x08017AC0-0x08017B4F); each function's words are
 * defined at its place. */
AnimFrame bfriTexAnimB[] = { /* a two-texture animation, 250 frames each */
    { 0x0301AA90, 250 },
    { 0x0301BA90, 250 },
    { 0, 0 },
};

/* The battle's opening, one step a frame (the boss's action 0), with its camera
 * (bfriEntranceCam). The boss rises from the sand while the screen rumbles and the
 * lightning flashes, roars (state 3), lands (4) and turns to face the Impact (5); then the
 * fight begins (7, 8). Start (either pilot's, with two players) skips to state 9, as does any
 * battle after a lost one: the boss is placed at once and the letterbox opens (10). */
s32 bfriEntranceDemo(Obj *obj) {
    static s32 unused = 1;
    static s32 frame = 0;
    BossW *w = FX(obj);
    s32 i;
    s32 pad[16];

    PRINTF("---------------------- 風雷神 登場DEMO -------------------\n");
    PRINTF("[tai]登場デモをキャンセルします\n");
    PRINTF("[tai]登場デモをキャンセルします\n");
    PRINTF("------------------  DEMO START  ---------------------\n");
    PRINTF("<DEMO>風雷神表示消します\n");
    PRINTF("<DEMO>外部インパクト表示消します\n");
    PRINTF("<DEMO>照準表示消します\n");
    PRINTF("<DEMO>コックピット表示消します\n");
    PRINTF("<DEMO>コックピットLOCKします\n");
    PRINTF("STEP1へ\n");
    PRINTF("---------> STEP2へ");
    PRINTF("---------> STEP3へ");
    PRINTF("<DEMO>風雷神表示します\n");
    PRINTF("---------> STEP4へ");
    PRINTF("<DEMO>照準表示します\n");
    PRINTF("<DEMO>外部インパクト表示します\n");
    PRINTF("<DEMO>コックピット表示します\n");
    PRINTF("[demottimer -> %d]\r");
    PRINTF("---------------------------  デモ終了  ----------------------------\n");
    PRINTF("<DEMO>風雷神表示します\n");
    PRINTF("<DEMO>照準表示します\n");
    PRINTF("<DEMO>外部インパクト表示します\n");
    PRINTF("<DEMO>コックピット表示します\n");
    PRINTF("---------------------------  デモ終了  ----------------------------\n");
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    if (w->state >= 0 && w->state < 5 && frame < 0x46A) {
        if (fadeCountPlayers() == 1) {
            if (joyGetHeldTime(gCockpitPort, gCockpitBtnStart) == 1) {
                w->state = 9;
                bfriEffectsEnd = 1;
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
            w->state = 9;
            bfriEffectsEnd = 1;
            fadeSetLetterbox(320, 120, 0, 120);
            for (i = 0; i < 4; i++) {
                contPakInitRumble(i);
                contPakRumbleRequestStop(i);
            }
            sndPlay(10);
            sndPlay(0x7C);
        }
    }
    if (w->state < 9) {
        (*bfriEntranceCam)(obj, frame, 0);
    }
    switch (w->state) {
        case 0:
            w->time = 0x1A90;
            fadeSetLetterbox(320, 120, 0, 120);
            w->frame->f.flags &= ~2;
            gBrotherFlags |= 0x10000;
            gBattleFlags |= 0x10000000;
            gBattleFlags |= 0x1000000;
            gBattleFlags2 |= 0x400;
            bfriBitsRemake = -1;
            frame = 0;
            bfriSkyMake();
            w->frame->pos[0] = w->count = 0.0f;
            w->frame->pos[2] = -3200.0f;
            w->frame->pos[1] = -200.0f;
            unused = 0;
            w->state = 1;
            if (gGameWork.losses) {
                w->state = 9;
                sndPlay(10);
                sndPlay(0x7C);
                break;
            }
            sndPlay(1);
            sndPlay(0x1A6);
            break;
        case 1:
            if (!(frame & 1)) {
                fadeMoveLetterbox(50, 190, 1);
            }
            if (frame % 80 == 0) {
                bfriFlashFrames = 5;
                bfriRumble(0x88888888, 6);
            }
            if (frame % 65 == 0) {
                bfriFlashFrames = 5;
                bfriRumble(0x88888888, 6);
            }
            if (frame % 10 == 0 && frame >= 0x2A5) {
                bfriThunderMake(0);
            }
            if (frame == 0x2D6) {
                w->state = 2;
            }
            break;
        case 2:
            if (frame % 6 == 0) {
                bfriThunderMake(2);
                bfriFlashFrames = 4;
            }
            if (frame == 0x402) {
                w->state = 3;
                bfriRumble(-1, 80);
                sndPlay(0x1A8);
            }
            break;
        case 3:
            bossStepAnim(w->frame, 0.02f, 2);
            if (frame == 0x43E) {
                w->state = 4;
            }
            if (frame == 0x420) {
                w->frame->f.flags |= 2;
                bossSetAnimAt(w->frame, bfriEntranceDemoAnim, 0xFB, 0.4f);
                fadeAnimStart(&w->anim, bfriTexAnimB, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            }
            break;
        case 4:
            if (w->frame->pos[2] < -2800.0f) {
                w->frame->pos[2] += 10.0f;
            }
            if (bossStepAnim(w->frame, 0.2f, 3)) {
                bossSetAnimAt(w->frame, bfriAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnimB, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
                w->state = 5;
            }
            break;
        case 5:
            bossStepAnim(w->frame, 0.1f, 0);
            if (frame >= 0x507) {
                fadeOpenLetterbox();
            }
            if (frame == 0x506) {
                gCockpitAimX = gCockpitAimY = gCockpitAimZ = 0.0f;
                gCockpitHeadPitch = gCockpitHeadYaw = 0.0f;
                gBattleFlags &= ~0x10000000;
                gBrotherFlags &= ~0x10000;
                gBattleFlags &= ~0x1000000;
            }
            if (frame == 0x556) {
                w->state = 7;
            }
            break;
        case 6:
            bossStepAnim(w->frame, 0.1f, 0);
            w->state = 7;
            break;
        case 7:
            bossStepAnim(w->frame, 0.1f, 0);
            effectKiranCreate(0.0f, 0.0f, -1800.0f);
            w->state = 8;
            w->count = 0.0f;
            break;
        case 8:
            bossStepAnim(w->frame, 0.1f, 0);
            (*bfriEntranceCam)(obj, frame, -1);
            w->state = 0;
            w->count = 0.0f;
            gBossActionArg = 0;
            gBattleFlags2 &= ~0x400;
            bfriSkyShown = 0;
            break;
        case 9:
            gCockpitAimX = gCockpitAimY = gCockpitAimZ = 0.0f;
            gCockpitHeadPitch = gCockpitHeadYaw = 0.0f;
            w->frame->pos[0] = 0.0f;
            w->frame->pos[2] = -2000.0f;
            w->frame->pos[1] = -200.0f;
            w->frame->rot[2] = 0.0f;
            w->frame->rot[0] = w->frame->rot[1] = w->frame->rot[2];
            w->vel[0] = 0.0f;
            w->vel[1] = 0.0f;
            w->vel[2] = 0.0f;
            w->frame->f.flags |= 2;
            gBattleFlags &= ~0x10000000;
            gBrotherFlags &= ~0x10000;
            gBattleFlags &= ~0x1000000;
            w->state = 10;
            gBattleFlags2 &= ~1;
            bossStepAnim(w->frame, 0.1f, 0);
            w->count = 60.0f;
            (*bfriEntranceCam)(obj, frame, -1);
            if (!(gGameFrameCount & 1)) {
                fadeOpenLetterbox();
            }
            break;
        case 10:
            bossStepAnim(w->frame, 0.1f, 0);
            fadeOpenLetterbox();
            fadeOpenLetterbox();
            bfriEffectsEnd = 0;
            if (w->count-- < 0.0f) {
                bfriSkyShown = 0;
                w->count = 0.0f;
                w->state = 0;
                gBossActionArg = 0;
                gBattleFlags2 &= ~0x400;
            }
            break;
    }
    frame++;
    return 0;
}

/* The battle's opening camera, one step a frame (frame counts from 0; arg -1 ends it): a pull
 * back along x with a hum every 12 frames from 507 to 625, a slow zoom in, then a move to the
 * fight's view at 1166 and a sparkle at 1208. Its words are its statics: the game reaches each
 * with a fresh lui (IDO_NOTES 38). Its zeros are three constants (IDO_NOTES 326): the
 * plasma's are integer 0s (one mtc1 and a mov.s), the final eye's 0.0f and the final at's 0.0. */
void bfriEntranceCam(Obj *obj, s32 frame, s32 arg) {
    static s32 state = 1;
    static f32 eyeX = 1.0f;
    static f32 eyeY = 1.0f;
    static f32 eyeZ = 1.0f;
    static f32 atX = 1.0f;
    static f32 atY = 1.0f;
    static f32 atZ = 1.0f;
    BossW *w = FX(obj);

    PRINTF("カメラを作りました\n");
    PRINTF("<cam> ステップ１に\n");
    PRINTF("[demottimer -> %d] [cx:%d cy:%d cz:%d] [dx:%d dy:%d dz:%d]\r");
    PRINTF("\n<cam> ステップ2に\n");
    PRINTF("[demottimer -> %d] [cx:%d cy:%d cz:%d] [dx:%d dy:%d dz:%d]\r");
    PRINTF("<cam> ステップ3に\n");
    PRINTF("<cam> ステップ4に\n");
    PRINTF("カメラ消します\n");
    if (frame == 0x46A) {
        scRunStart(4, 0, 0);
    }
    if (frame == 0) {
        bfriFov = 50.0f;
        eyeX = 1600.0f;
        eyeY = 100.0f;
        eyeZ = -800.0f;
        atX = 0.0f;
        atY = 1000.0f;
        atZ = -3000.0f;
        if (w->cam == NULL) {
            w->cam = tskCreate(obj, dracam, 0, NULL);
            camSetFov((Camera *) w->cam, bfriFov * 0.0174532930038800785);
        }
        w->view = camGetView((Camera *) w->cam);
        w->view->zFar = 20000.0f;
        state = 0;
    }
    switch (state) {
        case 0:
            if (w->state >= 9) {
                state++;
                break;
            }
            if (frame % 12 == 0 && frame >= 0x1FB && frame < 0x272) {
                sndPlay(0x52D);
            }
            if (frame == 0x272) {
                sndPlay(0x1A7);
                state = 1;
            }
            eyeX -= 2.0f;
            if (frame == 0xE2) {
                sndPlay(0x7B);
            }
            if (w->cam != NULL) {
                camSetEye((Camera *) w->cam, eyeX, eyeY, eyeZ);
                camSetAt((Camera *) w->cam, atX, atY, atZ);
            }
            break;
        case 1:
            atY *= 0.996;
            eyeX *= 0.996;
            eyeZ *= 0.996;
            if (frame == 0x2D6) {
                bfriPlasmaMake(0, 0, -2500.0f, 200);
            }
            if (frame == 0x48E) {
                state = 2;
                camSetSpeed((Camera *) w->cam, 40.0f);
                camMoveEye((Camera *) w->cam, 600.0f, 0, -2200.0f);
                camMoveAt((Camera *) w->cam, 0, 200.0f, -2800.0f);
            } else if (w->cam != NULL) {
                camSetEye((Camera *) w->cam, eyeX, eyeY, eyeZ);
                camSetAt((Camera *) w->cam, atX, atY, atZ);
            }
            break;
        case 2:
            camGetEye((Camera *) w->cam, &eyeX, &eyeY, &eyeZ);
            if (frame == 0x4B8) {
                effectKiranCreate(-800.0f, 0.0f, -3600.0f);
                state = 3;
            }
            break;
        case 3:
            if (frame == 0x4FC) {
                state = 4;
                camSetSpeed((Camera *) w->cam, 80.0f);
                camMoveEye((Camera *) w->cam, 0.0f, 0.0f, 0.0f);
                camMoveAt((Camera *) w->cam, 0.0, 0.0, -3000.0f);
            }
            break;
        case 4:
            break;
    }
    if (arg == -1) {
        tskDestroy(w->cam);
        w->cam = NULL;
    }
}

s32 bfriEntranceCamUnused[8] = { 0 }; /* read by no code */

/* The boss's death and the battle's win (its last action). The boss shakes, rocks from side to
 * side and slowly topples while the sand bursts and the controller rumbles (states 1-7). Red
 * stars rise, and at frame 960 it explodes in a white flash. The screen fades to black (8),
 * the letterbox closes on the clear (9, 10), and the battle is won (11). */
s32 bfriDeathDemo(Obj *obj) {
    static s32 frame = 0;
    BossW *w = FX(obj);
    s32 pad[11];

    PRINTF("死にDEMO\n");
    PRINTF("\r demo_timer[%d]");
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    if (w->state > 0) {
        frame++;
    }
    w->time = 0x3880;
    switch (w->state) {
        case 0:
            fadeCreate(gEffectTask);
            frame = 0;
            w->time = 0x3880;
            w->frame->rot[1] = 0.0f;
            w->frame->rot[0] = w->frame->rot[2] = w->frame->rot[1];
            gBossLife = 10000;
            gBattleFlags2 |= 0x400;
            gBossFlags |= 0x800;
            gBattleFlags2 |= 0x1000;
            gBattleFlags2 |= 0x1000000;
            sndPlay(9);
            w->vel[0] = w->vel[1] = w->vel[2] = 0.0f;
            tskCreate(obj, Bappcam3, gBattleTask->file, NULL);
            frame = 0;
            gBossPosX = w->frame->pos[0] = 2000.0f;
            gBossPosY = w->frame->pos[1] = 40.0f;
            gBossPosZ = w->frame->pos[2] = -1000.0f;
            bossSetAnimAt(w->frame, bfriDeathDemoAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnimB, 1, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            effectKiranCreate(w->frame->pos[0], w->frame->pos[1] + 200.0f, w->frame->pos[2]);
            beFlashFCreate(255, 255, 255, 40);
            effectPompomCreate(w->frame->pos[0], 0.0f, w->frame->pos[2], 250.0f, 250.0f, 250.0f, 7, 0x226, 5, 0.8f, 0.1f);
            effectPompomCreate(w->frame->pos[0], 0.0f, w->frame->pos[2], 250.0f, 250.0f, 250.0f, 7, 0x226, 5, 0.8f, 0.1f);
            effectPompomCreate(w->frame->pos[0], 0.0f, w->frame->pos[2], 250.0f, 250.0f, 250.0f, 8, 0x226, 5, 0.8f, 0.1f);
            effectPompomCreate(w->frame->pos[0], 0.0f, w->frame->pos[2], 250.0f, 250.0f, 250.0f, 8, 0x226, 5, 0.8f, 0.1f);
            effectPompomCreate(w->frame->pos[0], 0.0f, w->frame->pos[2], 250.0f, 250.0f, 250.0f, 10, 0x226, 5, 0.8f, 0.1f);
            sndPlay(0x10E);
            w->state++;
            break;
        case 1:
            if (frame % 12 == 0) {
                sndPlay(0x513);
            }
            if (!(frame & 7)) {
                bfriRumble(0x88888888, 8);
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
                w->state++;
            }
            break;
        case 2:
            if (!(frame & 7)) {
                bfriRumble(0x88888888, 8);
            }
            if (frame % 12 == 0) {
                sndPlay(0x513);
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
                w->state++;
            }
            break;
        case 3:
            if (!(frame & 7)) {
                bfriRumble(0x88888888, 8);
            }
            if (frame % 12 == 0) {
                sndPlay(0x513);
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
                w->state++;
            }
            break;
        case 4:
            if (frame % 8 == 0) {
                bfriRumble(0x88888888, 8);
            }
            if (frame % 8 == 0) {
                sndPlay(0x513);
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
                w->state++;
            }
            break;
        case 5:
            if (frame % 8 == 0) {
                bfriRumble(0x88888888, 8);
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
                w->state++;
            }
            break;
        case 6:
            if (frame % 8 == 0) {
                bfriRumble(0xA5294A52, 8);
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
                w->state++;
            }
            break;
        case 7:
            if (frame % 8 == 0) {
                bfriRumble(0xA5294A52, 8);
            }
            if (frame % 8 == 0) {
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
            if (frame == 960) {
                bfriRumble(-1, 50);
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
                w->state++;
            }
            break;
        case 8:
            if (frame == 1100) {
                fadeSetAll(30.0f, 255.0f, 0.0f, 0, 0, 0);
            }
            if (frame > 1130) {
                w->state++;
            }
            break;
        case 9:
            if (fadeCloseLetterbox(50)) {
                clearPackCreate(0.0f, 0.0f, 0.0f, 0);
                w->state++;
            }
            break;
        case 10:
            if (gBattleFlags2 & 0x800000) {
                gBattleFlags2 &= ~0x800000;
                w->state++;
            }
            break;
        case 11:
            if (fadeCloseLetterbox(120)) {
                swWorkSet(0xBF);
                bossBattleWin(gBattleBossId);
                dmenuQueueDemo(10);
            }
            break;
    }
    return 0;
}

s32 bfriDeathDemoUnused[8] = { 0 }; /* read by no code */

s32 bfriDefeatFrames = 0; /* the defeat's frames */

/* The boss's defeat: it sinks to the middle and stops (state 1), rises to its goal (2), sways and
 * slows (3), plays its last animation and fades to white (4), then, after 120 and 125 frames, the
 * stinger and the battle's end (5, 6). */
s32 bfriWinDemo(Obj *obj) {
    BossW *w = FX(obj);
    s32 pad[11];

    PRINTF("勝ちDEMO\n");
    PRINTF("\r demo_timer[%d]");
    PRINTF("--------------------  WIN DEMO START  ------------------------\n");
    PRINTF("<DEMO>風雷神表示します\n");
    PRINTF("STEP1へ\n");
    bfriMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    if (bfriNewAction == 1) {
        bfriNewAction = 0;
    }
    w->time = 0x3880;
    if (w->state > 0) {
        bfriDefeatFrames++;
    }
    bfriBank(obj, 0.6f);
    switch (w->state) {
        case 0:
            w->time = 0x3880;
            bossSetAnimAt(w->frame, bfriAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnimB, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            gBattleFlags2 |= 0xC00;
            fadeSetLetterbox(320, 0, 0, 240);
            w->frame->f.flags |= 2;
            w->state = 1;
            bfriEffectsEnd = 1;
            bfriDefeatFrames = 0;
            bfriBarrierState = 0;
            effectKiranCreate(0.0f, 0.0f, -1400.0f);
            w->goal[0] = 0;
            w->goal[2] = -2500;
            break;
        case 1:
            bfriEffectsEnd = 0;
            if (bfriTurnTo(obj, w->goal[0], w->goal[2], 2.0f)) {
                w->state = 2;
                bfriSunaEfMake(w->frame->pos[0], -850.0f, w->frame->pos[2], w->vel[0], w->vel[2], 6.0f, 0x80);
            }
            break;
        case 2:
            if (!bfriBmoveX(obj, w->goal[0], w->goal[1] - 200, w->goal[2], 0.2f)) {
                w->state = 3;
                bfriSunaEfMake(w->frame->pos[0], -850.0f, w->frame->pos[2], w->vel[0], w->vel[2], 6.0f, 0x80);
                w->count = 0.0f;
            } else {
                bossStepAnim(w->frame, (((gBossPhase * 8 + 32) / 2 - w->speed / 3.0f)/ 12.0f) * 1.3, 0);
            }
            break;
        case 3:
            w->frame->rot[2] *= 0.98;
            w->frame->rot[0] *= 0.98;
            if (bossStepAnim(w->frame, 0.096f, 2) && bfriTurnTo(obj, 0, 0, 3.0f)) {
                w->state = 4;
                bossSetAnimAt(w->frame, bfriWinDemoAnim, 0xFB, 0.0f);
                fadeAnimStart(&w->anim, bfriTexAnimB, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            }
            bfriTurnTo(obj, 0.0f, 0.0f, 2.0f);
            w->frame->pos[1] = sinf(w->count / 180.0f * 3.1415927f) * 60.0 + -200.0;
            w->count += 2.0f;
            bossDampVel(obj, 0.93f);
            break;
        case 4:
            w->frame->rot[2] *= 0.98;
            w->frame->rot[0] *= 0.98;
            bfriTurnTo(obj, 0.0f, 0.0f, 3.0f);
            if (bossStepAnim(w->frame, 0.2f, 2)) {
                fadeSetAll(120.0f, 0.0f, 255.0f, 255, 255, 255);
                w->state = 5;
                bfriTurnTo(obj, 0.0f, 0.0f, 2.0f);
                w->count = 0.0f;
            }
            break;
        case 5:
            w->count += 1.0f;
            if (w->count > 120.0f) {
                sndPlay(7);
                sndPlay(0x11D);
                gBattleFlags2 |= 0x4000000;
                w->state = 6;
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
