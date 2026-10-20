#include "tlb.h"
#include "tlb/tlb_0031.h"
s32 demo22Frame = 1; /* the scene's frame count */
s32 demo22Skip = 0; /* demo22Proc's skip, copied each frame */
/* This file's .data is demo22Frame, demo22Skip, demo22Proc's skip and demo22CamScript's two
 * statics, then IDO's padding to 16 (0x08007D40-0x08007D5F); tlb_0031_1's starts at 0x08007D60. */

#define DPRINTF

/* A demo director's work (tlb_0023_0's CHADEMO layout). */
typedef struct Demo22W {
    /* 0x00 */ Obj *cam; /* "dracam" */
    /* 0x04 */ Frame *f04; /* made by the scene's builders */
    /* 0x08 */ CamView *view;
    /* 0x0C */ Frame *f0C;
    /* 0x10 */ Frame *f10;
    /* 0x14 */ u8 pad14[4];
    /* 0x18 */ Obj *sky; /* "bgrad" */
    /* 0x1C */ Frame *f1C;
    /* 0x20 */ u32 layer3Word; /* gFrmLayer3's word at 0x18, put back on the way out */
    /* 0x24 */ u8 unused24;
    /* 0x25 */ u8 scene;
    /* 0x28 */ s32 timer; /* frames of demo run */
} Demo22W;

extern Frame *gFrmLayer3;
extern u8 gDemoCamHold;
extern s32 gDemoModelsDelete;
extern s32 gObjMoveCount;
extern u8 gCamChkActive;
extern f32 gCamChkEyeX;
extern f32 gCamChkEyeY;
extern f32 gCamChkEyeZ;
extern f32 gCamChkAtX;
extern f32 gCamChkAtY;
extern f32 gCamChkAtZ;
extern Gfx demo22RoomDl[]; /* the room */

void demo22CamScript(Obj *obj);

/* "DEMO22": a director built as OPDEMO3. It loads seven files, sets up the scene director, the
 * effects, a letterboxed fade, OBJMOVE (10 models), the dragon camera (50 degrees, planes 30 and
 * 20000), the sun, a black sky, the attack task, the room model of file 0xCB, four blue torches
 * (tlb_0031_1's TORCH) at the room's corners, and the scene's nine builders. Each frame it counts
 * and steps the demo (demo22CamScript) and keeps the count in demo22Frame. */
s32 demo22MenuProc(Obj *obj, s16 msg, void *arg) {
    Demo22W *w = (Demo22W *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "DEMO22";
            gDmenuLastObj = obj;
            w->timer = 0;
            demo22Frame = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0xCB);
            floadLoadSet(0xCD);
            floadLoadSet(0x29);
            floadLoadSet(0xC3);
            floadLoadSet(0xC4);
            floadLoadSet(0x125);
            tskCreate(obj, scParentProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 45, 0, 185);
            w->layer3Word = ((u32 *) gFrmLayer3)[6];
            gObjMoveCount = 10;
            demoObjMoveOpen();
            w->cam = tskCreate(obj, dracam, 0, NULL);
            DPRINTF("カメラを作りました\n");
            camSetFov((Camera *) w->cam, 0.87266463f);
            w->view = camGetView((Camera *) w->cam);
            w->view->zFar = 20000.0f;
            w->view->zNear = 30.0f;
            camSunCreate(obj);
            camSunSetView(w->view);
            gCamChkEyeX = 0.0f;
            gCamChkEyeY = 0.0f;
            gCamChkEyeZ = 0.0f;
            gCamChkAtX = 0.0f;
            gCamChkAtY = 1000.0f;
            gCamChkAtZ = -4000.0f;
            w->sky = tskCreate(obj, bgrad, 0, NULL);
            camBgradSetLines((Camera *) w->sky, 76, 240);
            camBgradMoveTop((ColorFade *) w->sky, 0, 0, 0);
            camBgradMoveBottom((ColorFade *) w->sky, 0, 0, 0);
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            demoJoyChkOpen();
            w->scene = 0;
            demoFadeStart(60.0f, 255.0f, 0.0f, 0, 0, 0);
            DPRINTF("VRAMset\n");
            tskCreate(obj, attack_Proc, 0, NULL);
            DPRINTF("部屋を出します\n");
            demoModelCreate(obj, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0xCB, demo22RoomDl, 0, 1);
            demo22TorchCreate(obj, 940.0f, 732.0f, 790.0f, 20, 20, 255, 20, 20, 255, 4.0f, 160);
            demo22TorchCreate(obj, 1280.0f, 1072.0f, 210.0f, 20, 20, 255, 20, 20, 255, 4.0f, 160);
            demo22TorchCreate(obj, -1370.0f, 1072.0f, 210.0f, 20, 20, 255, 20, 20, 255, 4.0f, 160);
            demo22TorchCreate(obj, -890.0f, 732.0f, 790.0f, 20, 20, 255, 20, 20, 255, 4.0f, 160);
            gCamChkStep = 10.0f;
            gObjMoveStep = 10.0f;
            gCamChkMarkerSize = 0.1f;
            (*demo22GoeCreate)(obj);
            (*demo22EbiCreate)(obj);
            (*demo22SasCreate)(obj);
            (*demo22YaeCreate)(obj);
            (*demo22SuzakuCreate)(obj);
            (*demo22DochukiCreate)(obj);
            (*demo22BismaruCreate)(obj);
            (*demo22GateCreate)(obj);
            (*demo22HyotanCreate)(obj);
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
            if (!gDemoCamHold) {
                w->timer++;
                (*demo22CamScript)(obj);
            }
            demo22Frame = w->timer;
            if (joyGetHeldTime(1, 4) == 1) {
                if (!gDemoCamHold) {
                    gDemoCamHold = 1;
                } else {
                    gDemoCamHold = 0;
                }
            }
            if (joyGetHeldTime(0, 0x1000) == 1 && !gCamChkActive && w->timer >= 11) {
                demoCamChkOpen(50.0f);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

extern s32 gDemoModelsDelete2;

/* "DEMO22" as played in the game: stage flag 0x6F (bit 7), no OBJMOVE models, dimmer torches,
 * and Start skips it (as tlb_0029_0's mach2DemoProc), stopping every pad's rumble: from 1 it
 * counts frames, at 3 fading out with sound 10, at 40 ending the scene director, at 50 going to
 * the battle (stageGotoBattle(5)); past frame 8520 the skip starts by itself, stopping the
 * rumble too. The skip is copied to demo22Skip each frame. */
s32 demo22Proc(Obj *obj, s16 msg, void *arg) {
    static s32 skip = 1;
    Demo22W *w = (Demo22W *) (obj + 1);
    s32 pad;
    s32 i;
    s32 pad2[2];

    switch (msg) {
        case 2:
            obj->name = "DEMO22";
            gDmenuLastObj = obj;
            w->timer = 0;
            demo22Frame = 0;
            gSaveData.stageFlags[0x6F] |= 0x80;
            skip = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0xCB);
            floadLoadSet(0xCD);
            floadLoadSet(0x29);
            floadLoadSet(0xC3);
            floadLoadSet(0xC4);
            floadLoadSet(0x125);
            tskCreate(obj, scParentProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 45, 0, 185);
            w->layer3Word = ((u32 *) gFrmLayer3)[6];
            gObjMoveCount = 0;
            w->cam = tskCreate(obj, dracam, 0, NULL);
            DPRINTF("カメラを作りました\n");
            camSetFov((Camera *) w->cam, 0.87266463f);
            w->view = camGetView((Camera *) w->cam);
            w->view->zFar = 20000.0f;
            w->view->zNear = 30.0f;
            camSunCreate(obj);
            camSunSetView(w->view);
            gCamChkEyeX = 0.0f;
            gCamChkEyeY = 0.0f;
            gCamChkEyeZ = 0.0f;
            gCamChkAtX = 0.0f;
            gCamChkAtY = 1000.0f;
            gCamChkAtZ = -4000.0f;
            w->sky = tskCreate(obj, bgrad, 0, NULL);
            camBgradSetLines((Camera *) w->sky, 76, 240);
            camBgradMoveTop((ColorFade *) w->sky, 0, 0, 0);
            camBgradMoveBottom((ColorFade *) w->sky, 0, 0, 0);
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            w->scene = 0;
            demoFadeStart(60.0f, 255.0f, 0.0f, 0, 0, 0);
            DPRINTF("VRAMset\n");
            tskCreate(obj, attack_Proc, 0, NULL);
            DPRINTF("部屋を出します\n");
            demoModelCreate(obj, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0xCB, demo22RoomDl, 0, 1);
            demo22TorchCreate(obj, 940.0f, 672.0f, 850.0f, 10, 10, 255, 10, 10, 255, 2.0f, 128);
            demo22TorchCreate(obj, 1280.0f, 1012.0f, 270.0f, 10, 10, 255, 10, 10, 255, 2.0f, 128);
            demo22TorchCreate(obj, -1370.0f, 1012.0f, 270.0f, 10, 10, 255, 10, 10, 255, 2.0f, 128);
            demo22TorchCreate(obj, -890.0f, 672.0f, 850.0f, 10, 10, 255, 10, 10, 255, 2.0f, 128);
            gCamChkStep = 10.0f;
            gObjMoveStep = 10.0f;
            gCamChkMarkerSize = 0.1f;
            (*demo22GoeCreate)(obj);
            (*demo22EbiCreate)(obj);
            (*demo22SasCreate)(obj);
            (*demo22YaeCreate)(obj);
            (*demo22SuzakuCreate)(obj);
            (*demo22DochukiCreate)(obj);
            (*demo22BismaruCreate)(obj);
            (*demo22GateCreate)(obj);
            (*demo22HyotanCreate)(obj);
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
            if (fadeStartPressed() && skip == 0) {
                skip = 1;
                for (i = 0; i < 4; i++) {
                    contPakInitRumble(i);
                    contPakRumbleRequestStop(i);
                }
            }
            switch (skip) {
                case 3:
                    demoFadeForce(30.0f, 0.0f, 255.0f, 0, 0, 0);
                    sndPlay(10);
                    skip++;
                    break;
                case 40:
                    gDemoModelsDelete2 = 1;
                    scRunEnd();
                    skip++;
                    break;
                case 50:
                    while (gfxGetPendingSwaps()) {
                    }
                    stageGotoBattle(5);
                    return -1;
                case 0:
                    break;
                default:
                    skip++;
                    break;
            }
            if (demo22Frame >= 8520 && skip == 0) {
                skip = 1;
                for (i = 0; i < 4; i++) {
                    contPakInitRumble(i);
                    contPakRumbleRequestStop(i);
                }
            }
            demo22Skip = skip;
            w->timer++;
            (*demo22CamScript)(obj);
            demo22Frame = w->timer;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}


#define sinf (*sinf)

/* DEMO22's camera script, one step a time (scene), on the scene's frame count; then the camera
 * check follows the camera. 0: the camera low in the room, sound 0x27. 1, 2: at 900 and 1100 it
 * glides up. 3-7: cuts at 2300, 2520, 3720, 4820 and 5820. 8: a cut at 6020; from 7222 sound
 * 0x197 and 9: the room shakes (a sine on the frame's low bits, growing over each 64 frames,
 * with a rumble every 20 frames while the skip is off) until 7620, a cut, and 10: it shakes on. */
void demo22CamScript(Obj *obj) {
    static f32 unused = 1.0f; /* set to 0 at the start; nothing reads it */
    static f32 shake = 1.0f;
    Demo22W *w = (Demo22W *) (obj + 1);
    s32 frame;
    s32 pad[2];

    switch (w->scene) {
        case 0:
            camSetEye((Camera *) w->cam, 0.0f, 460.0f, 1470.0f);
            camSetAt((Camera *) w->cam, 0.0f, 190.0f, 3400.0f);
            sndPlay(0x27);
            unused = 0.0f;
            w->scene++;
            break;
        case 1:
            if (demo22Frame == 900) {
                camSetSpeed((Camera *) w->cam, 160.0f);
                camMoveEye((Camera *) w->cam, 1540.0f, 440.0f, 2760.0f);
                camMoveAt((Camera *) w->cam, 0.0f, 1080.0f, -310.0f);
                w->scene++;
            }
            break;
        case 2:
            if (demo22Frame == 1100) {
                camSetSpeed((Camera *) w->cam, 160.0f);
                camMoveEye((Camera *) w->cam, 340.0f, 920.0f, 790.0f);
                camMoveAt((Camera *) w->cam, 0.0f, 1080.0f, -1070.0f);
                w->scene++;
            }
            break;
        case 3:
            if (demo22Frame == 2300) {
                camSetEye((Camera *) w->cam, 0.0f, 270.0f, 3030.0f);
                camSetAt((Camera *) w->cam, 0.0f, 740.0f, -1400.0f);
                w->scene++;
            }
            break;
        case 4:
            if (demo22Frame == 2520) {
                camSetEye((Camera *) w->cam, 0.0f, 1150.0f, 740.0f);
                camSetAt((Camera *) w->cam, 0.0f, 1240.0f, -390.0f);
                w->scene++;
            }
            break;
        case 5:
            if (demo22Frame == 3720) {
                camSetEye((Camera *) w->cam, 290.0f, 400.0f, 1200.0f);
                camSetAt((Camera *) w->cam, 0.0f, 270.0f, 1620.0f);
                w->scene++;
            }
            break;
        case 6:
            if (demo22Frame == 4820) {
                camSetEye((Camera *) w->cam, 20.0f, 1190.0f, 520.0f);
                camSetAt((Camera *) w->cam, 0.0f, 1300.0f, -140.0f);
                w->scene++;
            }
            break;
        case 7:
            if (demo22Frame == 5820) {
                camSetEye((Camera *) w->cam, 290.0f, 400.0f, 1200.0f);
                camSetAt((Camera *) w->cam, 0.0f, 270.0f, 1620.0f);
                w->scene++;
            }
            break;
        case 8:
            if (demo22Frame == 6020) {
                camSetEye((Camera *) w->cam, 0.0f, 270.0f, 3030.0f);
                camSetAt((Camera *) w->cam, 0.0f, 740.0f, -1400.0f);
            }
            if (demo22Frame >= 7222) {
                w->scene++;
                sndPlay(0x197);
            }
            break;
        case 9:
            if (demo22Frame >= 7620) {
                camSetEye((Camera *) w->cam, 290.0f, 400.0f, 1200.0f);
                camSetAt((Camera *) w->cam, 0.0f, 270.0f, 1620.0f);
                w->scene++;
            } else {
                shake = sinf((demo22Frame & 7) * 0.7853981851746036) * 10.0f *
                        (demo22Frame & 0x3F) / 63.0f;
                camSetEye((Camera *) w->cam, 0.0f, 270.0f + shake, 3030.0f);
                camSetAt((Camera *) w->cam, 0.0f, 740.0f + shake, -1400.0f);
                if (demo22Frame % 20 == 0 && demo22Skip == 0) {
                    demoRumbleAll(0x88888888, 20);
                }
            }
            break;
        case 10:
            shake = sinf((demo22Frame & 7) * 0.7853981851746036) * 10.0f *
                    (demo22Frame & 0x3F) / 63.0f;
            camSetEye((Camera *) w->cam, 290.0f, 400.0f + shake, 1200.0f);
            camSetAt((Camera *) w->cam, 0.0f, 270.0f + shake, 1620.0f);
            frame = demo22Frame;
            if (frame % 20 == 0 && demo22Skip == 0) {
                demoRumbleAll(0x88888888, 20);
            }
            break;
    }
    camGetEye((Camera *) w->cam, &gCamChkEyeX, &gCamChkEyeY, &gCamChkEyeZ);
    camGetAt((Camera *) w->cam, &gCamChkAtX, &gCamChkAtY, &gCamChkAtZ);
}
