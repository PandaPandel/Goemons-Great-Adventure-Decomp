#include "tlb.h"
#include "tlb/tlb_0033.h"

#define DPRINTF

/* "UTADEMO"'s work (tlb_0021_0's OpDemoW layout). */
typedef struct UtaDemoW {
    /* 0x00 */ Obj *cam; /* "democam" */
    /* 0x04 */ Frame *f04; /* the ambient light */
    /* 0x08 */ CamView *view;
    /* 0x0C */ Frame *f0C; /* the key light */
    /* 0x10 */ Frame *f10; /* the back light */
    /* 0x14 */ u8 pad14[4];
    /* 0x18 */ Obj *sky; /* "bgrad" */
    /* 0x1C */ Frame *f1C;
    /* 0x20 */ u8 pad20[4];
    /* 0x24 */ u32 layer3Word; /* gFrmLayer3's word at 0x18, put back on the way out */
    /* 0x28 */ u8 unused28;
    /* 0x29 */ u8 scene; /* utaDemoScript's, 0-10 */
    /* 0x2C */ s32 timer; /* frames of demo run */
} UtaDemoW;

extern Frame *gFrmLayer3;
extern u8 gDemoCamHold;
extern s32 gDemoModelsDelete;
extern s32 gDemoModelsDelete2;
extern s32 gObjMoveCount;
extern f32 gCamChkEyeX;
extern f32 gCamChkEyeY;
extern f32 gCamChkEyeZ;
extern f32 gCamChkAtX;
extern f32 gCamChkAtY;
extern f32 gCamChkAtZ;
extern s32 utaDemoFrame;   /* the demo's frame count (tlb_0033_3's data) */
extern f32 utaDemoUnusedFloats[]; /* [0]: set to 0 here */
extern u32 utaDemoSunTask[]; /* [0]: the sun's task */

void utaDemoScript(Obj *obj);

s32 utaDemoFrame = 1; /* the demo's frame count */
f32 utaDemoUnusedFloats[] = { 1.0f, 1.0f, 1.0f, 1.0f };
u32 utaDemoSunTask[] = { 0x01234567, 0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000 };
u32 utaDemoGhostTask[] = { 0x01234567, 0x3F800000, 0x3F800000, 0x3F800000 };

/* "UTADEMO": the singing demo's director. It loads seventeen files, sets up the scene director
 * (with no files of its own), the effects, a fade, 8 OBJMOVE models, the demo camera (20
 * degrees, planes 30 and 10000) at (0, 0, 110) looking at (0, 0, 100), the sun at 9 o'clock,
 * OPMAIN's three lights, a black sky, the attack task, and the four singers (tlb_0033_1's
 * UTAGOE, UTAEBI, SAS and YAE), with every player's weapon level at 0. Each frame it counts and
 * steps the demo (utaDemoScript) and keeps the count in utaDemoFrame. */
s32 utaDemoProc(Obj *obj, s16 msg, void *arg) {
    UtaDemoW *w = (UtaDemoW *) (obj + 1);
    s32 i;
    s32 pad; /* FAKE: a frame word (frame 0x50, the temps at 0x30); without it 45 */

    switch (msg) {
        case 2:
            obj->name = "UTADEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            utaDemoFrame = 0;
            utaDemoUnusedFloats[0] = 0.0f;
            gDemoModelsDelete2 = 0;
            floadLoadSet(0xCF);
            floadLoadSet(0xD2);
            floadLoadSet(0xC3);
            floadLoadSet(0xD0);
            floadLoadSet(0x29);
            floadLoadSet(0x100);
            floadLoadSet(0x125);
            floadLoadSet(0xF4);
            floadLoadSet(0xFC);
            floadLoadSet(0xFD);
            floadLoadSet(0xFF);
            floadLoadSet(0xBC);
            floadLoadSet(0xCE);
            floadLoadSet(0xC6);
            floadLoadSet(0xF3);
            floadLoadSet(0xC7);
            floadLoadSet(0x54);
            tskCreate(obj, scParentNoFilesProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            fadeCreate(gEffectTask);
            w->layer3Word = ((u32 *) gFrmLayer3)[6];
            gObjMoveCount = 8;
            w->cam = tskCreate(obj, democam, 0, NULL);
            DPRINTF("カメラを作りました\n");
            camSetFov((Camera *) w->cam, 0.34906587f);
            w->view = camGetView((Camera *) w->cam);
            w->view->zFar = 10000.0f;
            w->view->zNear = 30.0f;
            utaDemoSunTask[0] = (u32) camSunCreate(obj);
            camSunSetView(w->view);
            gTimeOfDay = 9 << 16;
            gCamChkEyeX = 0.0f;
            gCamChkEyeY = 0.0f;
            gCamChkEyeZ = 0.0f;
            gCamChkAtX = 0.0f;
            gCamChkAtY = 1000.0f;
            gCamChkAtZ = -4000.0f;
            w->f0C = frmCreate(gFrmLayers[0], gFrmKindLight);
            ((LightFrame *) w->f0C)->dir[0] = 127;
            ((LightFrame *) w->f0C)->dir[1] = 127;
            ((LightFrame *) w->f0C)->dir[2] = 127;
            fadeLightNormalize((LightFrame *) w->f0C);
            ((LightFrame *) w->f0C)->color[0] = 255;
            ((LightFrame *) w->f0C)->color[1] = 255;
            ((LightFrame *) w->f0C)->color[2] = 255;
            w->f10 = frmCreate(gFrmLayers[0], gFrmKindLight);
            ((LightFrame *) w->f10)->color[0] = 0;
            ((LightFrame *) w->f10)->color[1] = 0;
            ((LightFrame *) w->f10)->color[2] = 0;
            ((LightFrame *) w->f10)->dir[0] = -127;
            ((LightFrame *) w->f10)->dir[1] = -127;
            ((LightFrame *) w->f10)->dir[2] = -127;
            w->f04 = frmCreate(gFrmLayers[0], gFrmKindAmbi);
            ((AmbFrame *) w->f04)->color[0] = 128;
            ((AmbFrame *) w->f04)->color[1] = 128;
            ((AmbFrame *) w->f04)->color[2] = 128;
            w->sky = tskCreate(obj, bgrad, 0, NULL);
            camBgradSetLines((Camera *) w->sky, 76, 239);
            camBgradSetTop((ColorFade *) w->sky, 0, 0, 0);
            camBgradSetBottom((ColorFade *) w->sky, 0, 0, 0);
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            camSetEye((Camera *) w->cam, 0.0f, 0.0f, 110.0f);
            camSetAt((Camera *) w->cam, 0.0f, 0.0f, 100.0f);
            gRetracesPerFrame = 1;
            w->scene = 0;
            fadeSetAll(5.0f, 0.0f, 255.0f, 255, 255, 255);
            DPRINTF("VRAMset\n");
            tskCreate(obj, attack_Proc, 0, NULL);
            DPRINTF("地形を出します\n");
            gCamChkStep = 1.0f;
            gObjMoveStep = 1.0f;
            gCamChkMarkerSize = 0.01f;
            for (i = 0; i < 4; i++) {
                gPlayerStats[i].wpLv = 0;
            }
            (*utaGoeCreate)(obj);
            (*utaEbiCreate)(obj);
            (*utaSasCreate)(obj);
            (*utaYaeCreate)(obj);
            camSunSetClockStop(1);
            break;
        case 1:
            frmDestroy(w->f0C);
            frmDestroy(w->f1C);
            frmDestroy(w->f10);
            frmDestroy(w->f04);
            frmDestroy((Frame *) w->view);
            tskDestroy(w->cam);
            tskDestroy(w->sky);
            ((u32 *) gFrmLayer3)[6] = w->layer3Word;
            break;
        case 3:
            break;
        case 0:
            w->timer++;
            (*utaDemoScript)(obj);
            utaDemoFrame = w->timer;
            DPRINTF("TIMER---> %d\n", w->timer);
            /* The step's dead prints (utaDemoScript's; its strings are here). */
            DPRINTF("背景を出します\n");
            DPRINTF("KAMO1\n");
            DPRINTF("KAMO2\n");
            DPRINTF("KAMO3\n");
            DPRINTF("KAMO4\n");
            DPRINTF("KAMO5\n");
            DPRINTF("KAMO6\n");
            DPRINTF("KAMO7\n");
            DPRINTF("KAMO3\n");
            DPRINTF("タイサンバload\n");
            DPRINTF("デブ鬼load\n");
            DPRINTF("カブキ６４load\n");
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The singers' and the scenery's starters (tlb_0033_2 and _3), called far. */
#define utaOmiCreate (*utaOmiCreate)
#define utaKamomeCreate (*utaKamomeCreate)
#define utaMach2ShrinkCreate (*utaMach2ShrinkCreate)
#define utaHaikeiCreate (*utaHaikeiCreate)
#define utaJimenScrollCreate (*utaJimenScrollCreate)
#define utaBisuBigCreate (*utaBisuBigCreate)
#define utaMach2TrioCreate (*utaMach2TrioCreate)
#define utaDou2Create (*utaDou2Create)
#define utaCutcutCreate (*utaCutcutCreate)
#define utaBisuCreate (*utaBisuCreate)
#define utaBaraCreate (*utaBaraCreate)
#define utaFballCreate (*utaFballCreate)
#define utaZako4Create (*utaZako4Create)
#define utaWaterCreate (*utaWaterCreate)
#define utaZako10Create (*utaZako10Create)
#define utaBoss3Create (*utaBoss3Create)
#define utaFilterCreate (*utaFilterCreate)
#define utaZako14Create (*utaZako14Create)
#define utaGhostCreate (*utaGhostCreate)
#define utaJimenShadeCreate (*utaJimenShadeCreate)
#define utaCKiranCreate (*utaCKiranCreate)
#define utaCLballCreate (*utaCLballCreate)
#define utaCFireCreate (*utaCFireCreate)
#define utaCBKiranCreate (*utaCBKiranCreate)
#define utaCCldmanCreate (*utaCCldmanCreate)
#define utaCCldmanCreateFar (*utaCCldmanCreateFar)

extern u32 utaDemoGhostTask[]; /* [0]: utaGhostCreate's task (tlb_0033_3's data) */
extern Gfx utaDemoModel1Dl[];
extern Gfx utaDemoModel2Dl[];

#define CAM ((Camera *) w->cam)
#define SKY ((ColorFade *) w->sky)
#define KEY ((LightFrame *) w->f0C)
#define AMB ((AmbFrame *) w->f04)

/* The singing demo's script: each frame, by scene (w->scene) and the demo's frame count
 * (utaDemoFrame), it starts the singers' parts, moves the camera, the sky's colours,
 * the fade and the lights, and swaps the files; at frame 2880 it goes to the "sabi" ending.
 * Afterwards it copies the camera's eye and target to the camera checker.
 * w set at its declaration, before the joypad call, keeps obj out of a saved register (the game
 * reloads it from its home). The pad stands for locals the game declares and does not keep
 * (FAKE). The joypad test's result is tested by an empty if (a compiled-out debug test, IDO_NOTES
 * 369): as a bare call statement its u16 result held v0 and the scene took v1. */
void utaDemoScript(Obj *obj) {
    UtaDemoW *w = (UtaDemoW *) (obj + 1);
    s32 pad[13];

    if (joyGetHeldTime(1, 8)) {
    }
    switch (w->scene) {
        case 0:
            if (utaDemoFrame == 5) {
                fadeSetAll(20.0f, 255.0f, 0.0f, 255, 255, 255);
                sndPlay(5);
                sndPlay(0xB9);
                scRunStart2(0x1B7, 0, 0);
                utaHaikeiCreate(obj);
                w->scene++;
                utaDemoFrame = 0;
            }
            break;
        case 1:
            if (utaDemoFrame == 164) {
                utaMach2ShrinkCreate(obj);
            }
            if (utaDemoFrame == 247) {
                fadeSetAll(60.0f, 0.0f, 255.0f, 255, 255, 255);
            }
            if (utaDemoFrame == 263) {
                utaCKiranCreate(obj);
            }
            if (utaDemoFrame == 299) {
                gDemoModelsDelete2 = 1;
            }
            if (utaDemoFrame == 303) {
                camSetFov(CAM, 0.7853982f);
                utaDemoGhostTask[0] = (u32) utaGhostCreate(obj, 0xE0);
                gDemoModelsDelete2 = 0;
                fadeSetAll(3.0f, 255.0f, 0.0f, 255, 255, 255);
                camSetEye(CAM, 35.0f, 230.0f, 235.0f);
                camSetAt(CAM, 35.0f, 210.0f, 0.0f);
                camSetSpeed(CAM, 7.0f);
                camMoveEye(CAM, -25.0f, 185.0f, 235.0f);
                camMoveAt(CAM, -25.0f, 165.0f, 0.0f);
            }
            if (utaDemoFrame == 335) {
                fadeSetAll(4.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye(CAM, -165.0f, 225.0f, 235.0f);
                camSetAt(CAM, -165.0f, 165.0f, 0.0f);
                camSetSpeed(CAM, 7.0f);
                camMoveEye(CAM, 35.0f, 175.0f, 235.0f);
                camMoveAt(CAM, 35.0f, 115.0f, 0.0f);
            }
            if (utaDemoFrame == 366) {
                fadeSetAll(8.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye(CAM, -165.0f, 95.0f, 235.0f);
                camSetAt(CAM, -165.0f, 75.0f, 0.0f);
                camSetSpeed(CAM, 7.0f);
                camMoveEye(CAM, 0.0f, 125.0f, 235.0f);
                camMoveAt(CAM, 0.0f, 105.0f, 0.0f);
            }
            if (utaDemoFrame == 393) {
                fadeSetAll(8.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye(CAM, 85.0f, -60.0f, 235.0f);
                camSetAt(CAM, 85.0f, -80.0f, 0.0f);
                camSetSpeed(CAM, 7.0f);
                camMoveEye(CAM, 0.0f, 105.0f, 235.0f);
                camMoveAt(CAM, 0.0f, 85.0f, 0.0f);
            }
            if (utaDemoFrame == 452) {
                fadeSetAll(54.0f, 0.0f, 255.0f, 255, 255, 255);
                tskDestroy((Obj *) utaDemoGhostTask[0]);
            }
            if (utaDemoFrame == 512) {
                camBgradSetLines(SKY, 76, 239);
                camSetFov(CAM, 0.39095375f);
                camSetEye(CAM, 605.0f, -204.0f, 1661.0f);
                camSetAt(CAM, -152.0f, 225.3f, -3.7f);
                fadeSetAll(27.0f, 255.0f, 0.0f, 255, 255, 255);
                utaJimenScrollCreate(obj);
                camBgradSetTop(SKY, 48, 48, 255);
                camBgradSetBottom(SKY, 128, 128, 255);
                w->scene++;
            }
            break;
        case 2:
            if (utaDemoFrame == 636) {
                utaBisuBigCreate(obj);
            }
            if (utaDemoFrame == 666) {
                camBgradSetSpeed(SKY, 200.0f);
                camBgradMoveTop(SKY, 0, 0, 0);
                camBgradMoveBottom(SKY, 40, 0, 0);
            }
            if (utaDemoFrame == 980) {
                utaCBKiranCreate(obj);
            }
            if (utaDemoFrame == 1011) {
                tskDestroy((Obj *) utaDemoSunTask[0]);
                gGameState.fade = 1;
                utaCutcutCreate(obj);
                camSetFov(CAM, 0.69813174f);
                camSetEye(CAM, 0.0f, 105.0f, 165.0f);
                camSetAt(CAM, 0.0f, 86.0f, 0.0f);
                camBgradSetTop(SKY, 255, 255, 255);
                camBgradSetBottom(SKY, 255, 240, 128);
            }
            if (utaDemoFrame == 1028) {
                fadeSetAll(1.0f, 0.0f, 255.0f, 255, 255, 255);
            }
            if (utaDemoFrame == 1030) {
                fadeSetAll(1.0f, 255.0f, 0.0f, 255, 255, 255);
                camBgradSetBottom(SKY, 255, 255, 255);
                camBgradSetTop(SKY, 128, 240, 128);
            }
            if (utaDemoFrame == 1047) {
                fadeSetAll(1.0f, 0.0f, 255.0f, 255, 255, 255);
            }
            if (utaDemoFrame == 1049) {
                fadeSetAll(1.0f, 255.0f, 0.0f, 255, 255, 255);
                camBgradSetBottom(SKY, 255, 255, 255);
                camBgradSetTop(SKY, 208, 96, 128);
            }
            if (utaDemoFrame == 1067) {
                fadeSetAll(1.0f, 0.0f, 255.0f, 255, 255, 255);
            }
            if (utaDemoFrame == 1069) {
                fadeSetAll(1.0f, 255.0f, 0.0f, 255, 255, 255);
                camSetEye(CAM, 0.0f, 130.0f, 250.0f);
                camSetAt(CAM, 0.0f, 160.0f, 0.0f);
                camBgradSetBottom(SKY, 255, 255, 255);
                camBgradSetTop(SKY, 128, 128, 255);
            }
            if (utaDemoFrame == 1122) {
                camSetSpeed(CAM, 19.0f);
                camMoveEye(CAM, 0.0f, 10.0f, 315.0f);
                camMoveAt(CAM, 0.0f, 160.0f, 0.0f);
            }
            if (utaDemoFrame == 1132) {
                fadeSetAll(28.0f, 0.0f, 255.0f, 255, 255, 255);
            }
            if (utaDemoFrame == 1160) {
                KEY->dir[0] = 0;
                KEY->dir[1] = 127;
                KEY->dir[2] = 0;
                fadeLightNormalize(KEY);
                KEY->color[0] = 255;
                KEY->color[1] = 255;
                KEY->color[2] = 255;
                AMB->color[0] = 64;
                AMB->color[1] = 64;
                AMB->color[2] = 64;
                utaMach2TrioCreate(obj);
                camSetFov(CAM, 0.69813174f);
                camSetEye(CAM, -741.0f, -209.0f, 512.0f);
                camSetAt(CAM, 205.0f, 379.0f, -490.0f);
                camSetSpeed(CAM, 220.0f);
                camMoveEye(CAM, -453.0f, -209.0f, 536.0f);
                camMoveAt(CAM, 131.0f, 379.0f, -490.0f);
                fadeSetAll(14.0f, 255.0f, 0.0f, 255, 255, 255);
                camBgradSetTop(SKY, 0, 0, 0);
                camBgradSetBottom(SKY, 128, 0, 0);
            }
            if (utaDemoFrame == 1380) {
                camSetFov(CAM, 0.69813174f);
                camSetEye(CAM, 741.0f, -209.0f, 512.0f);
                camSetAt(CAM, -205.0f, 379.0f, -490.0f);
                camSetSpeed(CAM, 142.0f);
                camMoveEye(CAM, 453.0f, -209.0f, 536.0f);
                camMoveAt(CAM, -131.0f, 379.0f, -490.0f);
            }
            if (utaDemoFrame == 1506) {
                fadeSetAll(14.0f, 0.0f, 255.0f, 255, 255, 255);
            }
            if (utaDemoFrame == 1522) {
                KEY->dir[0] = 127;
                KEY->dir[1] = 127;
                KEY->dir[2] = 127;
                fadeLightNormalize(KEY);
                KEY->color[0] = 255;
                KEY->color[1] = 240;
                KEY->color[2] = 240;
                fadeSetAll(14.0f, 255.0f, 0.0f, 255, 255, 255);
                demoModelCreate(obj, 0.0f, 0.0f, 0.0f, 5.0f, 5.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0xD0, utaDemoModel1Dl, 0, 2);
                utaDou2Create(obj);
                camSetFov(CAM, 0.34906587f);
                camSetEye(CAM, 0.0f, 305.0f, 7852.0f);
                camSetAt(CAM, 0.0f, 142.0f, -3041.0f);
                camSetSpeed(CAM, 40.0f);
                camMoveEye(CAM, 0.0f, 305.0f, 7700.0f);
                camMoveAt(CAM, 0.0f, 142.0f, -3041.0f);
            }
            if (utaDemoFrame == 1562) {
                camSetFov(CAM, 0.34906587f);
                camSetSpeed(CAM, 40.0f);
                camSetEye(CAM, 0.0f, 380.0f, -145.0f);
                camSetAt(CAM, 0.0f, 283.0f, -660.0f);
                utaCLballCreate(obj);
            }
            if (utaDemoFrame == 1626) {
                camSetFov(CAM, 0.34906587f);
                camSetSpeed(CAM, 70.0f);
                camMoveEye(CAM, 0.0f, 15.0f, 3907.0f);
                camMoveAt(CAM, 0.0f, -440.0f, -660.0f);
                utaCFireCreate(obj);
            }
            if (utaDemoFrame == 1752) {
                w->view->zFar = 5000.0f;
                w->view->zNear = 20.0f;
                KEY->dir[0] = 127;
                KEY->dir[1] = 127;
                KEY->dir[2] = 127;
                fadeLightNormalize(KEY);
                KEY->color[0] = 255;
                KEY->color[1] = 255;
                KEY->color[2] = 255;
                AMB->color[0] = 128;
                AMB->color[1] = 128;
                AMB->color[2] = 128;
                camSetFov(CAM, 0.64577186f);
                gDemoModelsDelete2 = 1;
                camSetEye(CAM, 256.0f, 161.0f, 7.0f);
                camSetAt(CAM, 469.0f, 168.0f, -4.0f);
                camBgradSetTop(SKY, 48, 48, 255);
                camBgradSetBottom(SKY, 128, 128, 255);
                utaBisuCreate(obj);
                utaBaraCreate(obj);
                utaFballCreate(obj);
            }
            if (utaDemoFrame == 1770) {
                fadeSetAll(30.0f, 0.0f, 255.0f, 255, 255, 255);
            }
            if (utaDemoFrame == 1800) {
                w->view->zFar = 13000.0f;
                w->view->zNear = 30.0f;
                utaFilterCreate(obj);
                camSetFov(CAM, 0.7853982f);
                gDemoModelsDelete2 = 0;
                utaWaterCreate(obj);
                camSetEye(CAM, -130.0f, 127.0f, 73.0f);
                camSetAt(CAM, 428.0f, 128.0f, 86.0f);
                utaOmiCreate(obj);
                camBgradSetTop(SKY, 0, 0, 0);
                camBgradSetBottom(SKY, 0, 0, 0);
                fadeSetAll(10.0f, 255.0f, 0.0f, 255, 255, 255);
                demoModelCreate(obj, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0xD2, utaDemoModel2Dl, 0, 0);
                w->scene++;
            }
            break;
        case 3:
            if (utaDemoFrame == 1850) {
                utaKamomeCreate(obj);
                utaZako4Create(obj);
                w->scene++;
            }
            break;
        case 4:
            if (utaDemoFrame == 2076) {
                camBgradSetSpeed(SKY, 10.0f);
                camBgradMoveTop(SKY, 0, 0, 0);
                camBgradMoveBottom(SKY, 0, 0, 0);
            }
            if (utaDemoFrame == 2086) {
                w->scene++;
                camBgradMoveTop(SKY, 48, 48, 255);
                camBgradMoveBottom(SKY, 128, 128, 255);
                utaCCldmanCreate(obj);
            }
            break;
        case 5:
            if (utaDemoFrame == 2180) {
                camSetSpeed(CAM, 30.0f);
                camMoveEye(CAM, -130.0f, 127.0f, 73.0f);
                camMoveAt(CAM, -120.0f, 130.0f, 75.0f);
                w->scene++;
            }
            break;
        case 6:
            if (utaDemoFrame == 2210) {
                w->scene++;
                camSetSpeed(CAM, 21.11111f);
                camMoveEye(CAM, -190.0f, 1109.0f, 1583.0f);
                camMoveAt(CAM, -10.0f, 1199.0f, 1038.0f);
            }
            break;
        case 7:
            if (utaDemoFrame == 2231) {
                camSetSpeed(CAM, 21.11111f);
                camMoveEye(CAM, -170.0f, 1124.0f, 1808.0f);
                camMoveAt(CAM, 80.0f, 1199.0f, 1183.0f);
            }
            if (utaDemoFrame == 2252) {
                camSetSpeed(CAM, 21.11111f);
                camMoveEye(CAM, -545.0f, 849.0f, 1568.0f);
                camMoveAt(CAM, 30.0f, 1199.0f, 1313.0f);
            }
            if (utaDemoFrame == 2273) {
                camSetSpeed(CAM, 21.11111f);
                camMoveEye(CAM, -660.0f, 804.0f, 1278.0f);
                camMoveAt(CAM, 50.0f, 1199.0f, 1263.0f);
            }
            if (utaDemoFrame == 2294) {
                camSetSpeed(CAM, 21.11111f);
                camMoveEye(CAM, -510.0f, 894.0f, 628.0f);
                camMoveAt(CAM, 75.0f, 1199.0f, 1198.0f);
            }
            if (utaDemoFrame == 2315) {
                camSetSpeed(CAM, 21.11111f);
                camMoveEye(CAM, -250.0f, 1004.0f, 353.0f);
                camMoveAt(CAM, 0.0f, 1199.0f, 1038.0f);
            }
            if (utaDemoFrame == 2336) {
                camSetSpeed(CAM, 21.11111f);
                camMoveEye(CAM, -130.0f, 1074.0f, -207.0f);
                camMoveAt(CAM, 0.0f, 1199.0f, 1038.0f);
            }
            if (utaDemoFrame == 2357) {
                camSetSpeed(CAM, 21.11111f);
                camMoveEye(CAM, -135.0f, 1074.0f, -732.0f);
                camMoveAt(CAM, 0.0f, 1199.0f, 1038.0f);
            }
            if (utaDemoFrame == 2378) {
                camSetSpeed(CAM, 21.11111f);
                camMoveEye(CAM, -135.0f, 1074.0f, -1827.0f);
                camMoveAt(CAM, 0.0f, 1199.0f, 1038.0f);
            }
            if (utaDemoFrame == 2385) {
                fadeSetAll(10.0f, 0.0f, 255.0f, 255, 255, 255);
            }
            if (utaDemoFrame == 2390) {
                w->view->zFar = 10000.0f;
                w->view->zNear = 30.0f;
                while (gfxGetPendingSwaps()) {
                }
                floadFreeSet(0xCF);
                floadFreeSet(0xC3);
                floadFreeSet(0xD0);
                floadFreeSet(0x29);
                floadFreeSet(0x100);
                floadFreeSet(0x125);
                floadFreeSet(0xF4);
                floadLoadSet(0x102);
            }
            if (utaDemoFrame == 2391) {
                floadLoadSet(0x105);
            }
            if (utaDemoFrame == 2394) {
                floadLoadSet(0x107);
            }
            if (utaDemoFrame == 2396) {
                floadLoadSet(0xFE);
                floadLoadSet(0x101);
            }
            if (utaDemoFrame == 2398) {
                gDemoModelsDelete2 = 1;
                floadLoadSet(0x103);
                floadLoadSet(0x104);
            }
            if (utaDemoFrame == 2400) {
                gDemoModelsDelete2 = 0;
                fadeSetAll(6.0f, 255.0f, 0.0f, 255, 255, 255);
                camSetFov(CAM, 0.64577186f);
                camSetEye(CAM, -131.7f, 32.5f, 616.5f);
                camSetAt(CAM, -14.8f, 324.0f, -202.0f);
                camBgradSetTop(SKY, 0, 0, 0);
                camBgradSetBottom(SKY, 64, 0, 96);
                utaZako10Create(obj);
                w->scene++;
            }
            break;
        case 8:
            if (utaDemoFrame == 2477) {
                floadLoadSet(0xD5);
                fadeSetAll(6.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (utaDemoFrame == 2480) {
                floadLoadSet(0xD3);
            }
            if (utaDemoFrame == 2483) {
                while (gfxGetPendingSwaps()) {
                }
                floadFreeSet(0x105);
                floadFreeSet(0xD2);
                floadFreeSet(0x104);
                floadFreeSet(0x101);
                floadLoadSet(0xD4);
            }
            if (utaDemoFrame == 2485) {
                fadeSetAll(1.0f, 0.0f, 0.0f, 0, 0, 0);
                camSetFov(CAM, 0.23911011f);
                camSetEye(CAM, -106.0f, 265.0f, 3260.0f);
                camSetAt(CAM, -254.0f, 543.0f, 35.0f);
                utaBoss3Create(obj);
                camBgradSetSpeed(SKY, 16.0f);
                camBgradSetTop(SKY, 255, 255, 255);
                camBgradSetBottom(SKY, 255, 255, 255);
                camBgradMoveTop(SKY, 0, 0, 0);
                camBgradMoveBottom(SKY, 128, 0, 0);
                w->scene++;
            }
            break;
        case 9:
            if (utaDemoFrame == 2513) {
                camBgradSetSpeed(SKY, 16.0f);
                camBgradSetTop(SKY, 255, 255, 255);
                camBgradSetBottom(SKY, 255, 255, 255);
                camBgradMoveTop(SKY, 0, 0, 0);
                camBgradMoveBottom(SKY, 128, 0, 0);
            }
            if (utaDemoFrame == 2550) {
                camBgradSetSpeed(SKY, 16.0f);
                camBgradSetTop(SKY, 255, 255, 255);
                camBgradSetBottom(SKY, 255, 255, 255);
                camBgradMoveTop(SKY, 0, 0, 0);
                camBgradMoveBottom(SKY, 128, 0, 0);
            }
            if (utaDemoFrame == 2641) {
                while (gfxGetPendingSwaps()) {
                }
                floadFreeSet(0x103);
                fadeSetAll(3.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (utaDemoFrame == 2642) {
                floadLoadSet(0x106);
            }
            if (utaDemoFrame == 2644) {
                gDemoModelsDelete2 = 1;
            }
            if (utaDemoFrame == 2647) {
                KEY->dir[0] = 127;
                KEY->dir[1] = 127;
                KEY->dir[2] = 127;
                fadeLightNormalize(KEY);
                fadeSetAll(3.0f, 255.0f, 0.0f, 0, 0, 0);
                utaJimenShadeCreate(obj);
                gDemoModelsDelete2 = 0;
                camSetFov(CAM, 0.7504916f);
                camSetEye(CAM, 546.0f, 114.0f, 383.0f);
                camSetAt(CAM, 0.0f, 131.4f, 0.0f);
                camBgradSetTop(SKY, 0, 0, 0);
                camBgradSetBottom(SKY, 0, 0, 0);
                utaZako14Create(obj);
                w->scene++;
            }
            break;
        case 10:
            if (utaDemoFrame == 2715) {
                camBgradSetSpeed(SKY, 20.0f);
                camBgradMoveTop(SKY, 255, 255, 255);
                camBgradMoveBottom(SKY, 255, 255, 255);
                camSetFov(CAM, 0.69813174f);
                camSetEye(CAM, -7.6f, 132.0f, 147.0f);
                camSetAt(CAM, 12.0f, 125.0f, -4.7f);
            } else if (utaDemoFrame == 2745) {
                camSetFov(CAM, 0.7504916f);
                camSetEye(CAM, 546.0f, 114.0f, 383.0f);
                camSetAt(CAM, 0.0f, 131.4f, 0.0f);
            } else if (utaDemoFrame == 2780) {
                camSetFov(CAM, 0.69813174f);
                camSetEye(CAM, 0.2f, 132.0f, 147.0f);
                camSetAt(CAM, -0.5f, 125.0f, -4.7f);
                camBgradSetSpeed(SKY, 30.0f);
                camBgradMoveTop(SKY, 48, 48, 255);
                camBgradMoveBottom(SKY, 128, 128, 255);
            } else if (utaDemoFrame == 2810) {
                utaCCldmanCreateFar(obj);
                camSetFov(CAM, 0.7504916f);
                camSetEye(CAM, 10546.0f, 114.0f, 383.0f);
                camSetAt(CAM, 10000.0f, 131.4f, 0.0f);
            } else if (utaDemoFrame == 2864) {
                fadeSetAll(16.0f, 0.0f, 255.0f, 0, 0, 0);
            } else if (utaDemoFrame == 2880) {
                stageGotoSabi();
            }
            break;
    }
    camGetEye(CAM, &gCamChkEyeX, &gCamChkEyeY, &gCamChkEyeZ);
    camGetAt(CAM, &gCamChkAtX, &gCamChkAtY, &gCamChkAtZ);
}
