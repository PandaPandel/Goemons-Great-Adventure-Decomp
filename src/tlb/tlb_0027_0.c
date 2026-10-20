#include "tlb.h"
#include "tlb/tlb_0027.h"

#define DPRINTF

/* A demo director's work (tlb_0023_0's CHADEMO layout). */
typedef struct ChaDemoW {
    /* 0x00 */ Obj *cam; /* "democam" */
    /* 0x04 */ Frame *f04; /* the ambient light */
    /* 0x08 */ CamView *view;
    /* 0x0C */ Frame *f0C; /* the key light */
    /* 0x10 */ Frame *f10; /* the back light */
    /* 0x14 */ u8 pad14[4];
    /* 0x18 */ Obj *sky; /* "bgrad" */
    /* 0x1C */ Frame *f1C;
    /* 0x20 */ u32 layer3Word; /* gFrmLayer3's word at 0x18, put back on the way out */
    /* 0x24 */ u8 unused24;
    /* 0x25 */ u8 scene;
    /* 0x28 */ s32 timer; /* frames of demo run */
} ChaDemoW;

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
/* This file's .data is dochukiDemoFrame, dochukiDemoSkip, dochukiDemoCamScript's two statics and
 * dochukiDemoProc's skip, then IDO's padding to 16 (0x08008550-0x0800856F); tlb_0027_1's starts
 * at 0x08008570. */
s32 dochukiDemoFrame = 1; /* the scene's frame count */
extern Gfx dochukiDemoLandDl[]; /* the land */

void dochukiDemoCamScript(Obj *obj);

/* "CHADEMO" on the land: a director built as OPDEMO2. It loads six files, sets up the scene
 * director, the effects, a letterboxed fade, OBJMOVE (7 models), marks stage flag 0x6B (bit 7),
 * the camera (50 degrees, planes 24 and 20000), its own key, back and ambient lights (as
 * OPMAIN's), a black sky, the attack task and the land model of file 0xC9 at ten times its size
 * 5000 down, and starts the scene's six characters (tlb_0027_1's). Each frame it counts and
 * steps the demo (dochukiDemoCamScript) and keeps the count in dochukiDemoFrame. */
s32 dochukiDemoMenuProc(Obj *obj, s16 msg, void *arg) {
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "CHADEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            dochukiDemoFrame = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0xC9);
            floadLoadSet(0x29);
            floadLoadSet(0xC3);
            floadLoadSet(0x12C);
            floadLoadSet(0xEE);
            tskCreate(obj, scParentProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 45, 0, 185);
            w->layer3Word = ((u32 *) gFrmLayer3)[6];
            gObjMoveCount = 7;
            demoObjMoveOpen();
            gSaveData.stageFlags[0x6B] |= 0x80;
            w->cam = tskCreate(obj, democam, 0, NULL);
            DPRINTF("カメラを作りました\n");
            camSetFov((Camera *) w->cam, 0.87266463f);
            w->view = camGetView((Camera *) w->cam);
            w->view->zFar = 20000.0f;
            w->view->zNear = 24.0f;
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
            ((LightFrame *) w->f0C)->color[0] = 240;
            ((LightFrame *) w->f0C)->color[1] = 250;
            ((LightFrame *) w->f0C)->color[2] = 255;
            w->f10 = frmCreate(gFrmLayers[0], gFrmKindLight);
            ((LightFrame *) w->f10)->color[0] = 0;
            ((LightFrame *) w->f10)->color[1] = 0;
            ((LightFrame *) w->f10)->color[2] = 0;
            ((LightFrame *) w->f10)->dir[0] = -127;
            ((LightFrame *) w->f10)->dir[1] = -127;
            ((LightFrame *) w->f10)->dir[2] = -127;
            w->f04 = frmCreate(gFrmLayers[0], gFrmKindAmbi);
            ((AmbFrame *) w->f04)->color[0] = 64;
            ((AmbFrame *) w->f04)->color[1] = 112;
            ((AmbFrame *) w->f04)->color[2] = 128;
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
            DPRINTF("地形を出します\n");
            demoModelCreate(obj, 0.0f, -5000.0f, 0.0f, 10.0f, 10.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0xC9, dochukiDemoLandDl, 0, 0);
            gCamChkStep = 4.0f;
            gObjMoveStep = 4.0f;
            gCamChkMarkerSize = 1.0f;
            (*dochukiDemoGoeCreate)(obj);
            (*dochukiDemoEbiCreate)(obj);
            (*dochukiDemoSasCreate)(obj);
            (*dochukiDemoYaeCreate)(obj);
            (*dochukiDemoBisuCreate)(obj);
            (*dochukiDemoMach12Create)(obj);
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
                (*dochukiDemoCamScript)(obj);
            }
            dochukiDemoFrame = w->timer;
            DPRINTF("demotimer -->[%d]\n", w->timer);
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
s32 dochukiDemoSkip = 0; /* dochukiDemoProc's skip, copied each frame */

extern Gfx dochukiDemoCamScriptDl[];


#define sinf (*sinf)
#define cosf (*cosf)

/* DOCHUKI's camera script, one step a time (scene), on the scene's frame count; then sound
 * 0x190 at 4580, the camera check follows the camera and dochukiDemoEffectsStep runs.
 * 0: the camera far below, sound 0x22. 1: a fade, at 150 a wide view and the road model of file
 * 0xC9. 2: the eye spirals in (radius 4000, 3 less a frame); at 390 a cut. 3: a fade, at 1000
 * a cut below. 4: sound 0x18D at 1442, then every 10 frames to 3044 dochukiDemoSmokeCreate; a
 * fade, at 1604 a cut. 5-7: glides at 1804, 2274 (sound 0x18F) and 2574. 8: a fade, at 3274
 * the glide steps set out. 9-11: the camera glides 274 (then 374) frames between set views,
 * with fades at 4067 and 4222, and at 4272 a cut. 12, 13: at 4612 a white flash and six
 * effects; at 4732 a cut. 14-19: fades and cuts to 8235. 20: at 9205 a cut; from 9305 sound
 * 0x197 and 21: the quake grows to 30 with a rumble every 20 frames while the skip is off; at
 * 10065 sound 10, a fade out, every pad's rumble stops and the scene director ends.
 * Its glide and quake statics are tlb_0027's .data and .bss (0x08008558, 0x08009650-0x0800968F):
 * as globals IDO reloads after each store, the game does not (IDO_NOTES 317).
 * The quake's product is read through x ("x = sinf(..) * quake;" then "-4387.0f + x"): written
 * in one expression, IDO puts the constant second. pad[2] (FAKE) is the game's frame words. Case
 * 6's 2234 fade is on one line (IDO_NOTES 12). */
void dochukiDemoCamScript(Obj *obj) {
    static f32 angle = 0.0f;
    static f32 radius = 4000.0f;
    static f32 quake;                 /* the quake's size (0x08009650, .bss) */
    static f32 eyeX, eyeY, eyeZ;      /* the glide steps' camera */
    static f32 atX, atY, atZ;
    static f32 dEyeX, dEyeY, dEyeZ;   /* what each frame adds to them */
    static f32 dAtX, dAtY, dAtZ;
    f32 x;
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    f32 z;
    s32 pad[2]; /* FAKE: two frame words the game has above i */
    s32 i;

    switch (w->scene) {
        case 0:
            camSetEye((Camera *) w->cam, -184.0f, -4387.0f, 1018.0f);
            camSetAt((Camera *) w->cam, 69.0f, -5161.0f, 2.0f);
            camSetSpeed((Camera *) w->cam, 160.0f);
            camMoveEye((Camera *) w->cam, -184.0f, -4387.0f, 1204.0f);
            camMoveAt((Camera *) w->cam, 69.0f, -5161.0f, 2.0f);
            sndPlay(0x22);
            w->scene++;
            break;
        case 1:
            if (dochukiDemoFrame == 120) {
                demoFadeStart(24.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (dochukiDemoFrame == 150) {
                camSetFov((Camera *) w->cam, 1.0471976f);
                demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
                gDemoModelsDelete2 = 0;
                demoModelCreate(obj, 0.0f, 0.0f, 0.0f, 10.0f, 10.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0xC9, dochukiDemoCamScriptDl, 0, 0);
                camSetAt((Camera *) w->cam, 0.0f, 303.0f, 0.0f);
                w->scene++;
                angle = 0.0f;
            }
            break;
        case 2:
            angle += 1.0f;
            x = sinf(angle / 20.0f * 0.0174532930038800785) * radius;
            z = cosf(angle / 20.0f * 0.0174532930038800785) * radius;
            radius -= 3.0f;
            camSetEye((Camera *) w->cam, x, 485.0f, z);
            if (dochukiDemoFrame == 390) {
                camSetFov((Camera *) w->cam, 0.87266463f);
                camSetEye((Camera *) w->cam, 674.0f, 244.0f, 1458.0f);
                camSetAt((Camera *) w->cam, 533.0f, 295.0f, 1294.0f);
                camSetSpeed((Camera *) w->cam, 700.0f);
                camMoveEye((Camera *) w->cam, 562.0f, 244.0f, 1462.0f);
                camMoveAt((Camera *) w->cam, 533.0f, 295.0f, 1294.0f);
                scRunEnd();
                w->scene++;
            }
            break;
        case 3:
            if (dochukiDemoFrame == 120) {
                demoFadeStart(24.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (dochukiDemoFrame == 1000) {
                camSetFov((Camera *) w->cam, 0.87266463f);
                demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
                gDemoModelsDelete2 = 0;
                camSetEye((Camera *) w->cam, -184.0f, -4387.0f, 1204.0f);
                camSetAt((Camera *) w->cam, 69.0f, -5161.0f, 2.0f);
                w->scene++;
            }
            break;
        case 4:
            if (dochukiDemoFrame == 1442) {
                sndPlay(0x18D);
            }
            if (dochukiDemoFrame % 10 == 0 && dochukiDemoFrame >= 1443 && dochukiDemoFrame < 3044) {
                (*dochukiDemoSmokeCreate)(obj);
            }
            if (dochukiDemoFrame == 1574) {
                demoFadeStart(24.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (dochukiDemoFrame == 1604) {
                gDemoModelsDelete2 = 0;
                scRunEnd();
                camSetFov((Camera *) w->cam, 0.87266463f);
                demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 517.0f, 284.0f, 1407.0f);
                camSetAt((Camera *) w->cam, 525.0f, 299.0f, 1222.0f);
                w->scene++;
            }
            break;
        case 5:
            if (dochukiDemoFrame == 1804) {
                camSetSpeed((Camera *) w->cam, 530.0f);
                camMoveEye((Camera *) w->cam, 1169.0f, 100.0f, 2843.0f);
                camMoveAt((Camera *) w->cam, 525.0f, 299.0f, 1222.0f);
                w->scene++;
            }
            break;
        case 6:
            /* on one line (IDO_NOTES 12), for the game's mtc1 after t9's addiu */
            if (dochukiDemoFrame == 2234) { demoFadeStart(39.0f, 0.0f, 255.0f, 0, 0, 0); }
            if (dochukiDemoFrame == 2274) {
                sndPlay(0x18F);
                demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetFov((Camera *) w->cam, 0.87266463f);
                camSetEye((Camera *) w->cam, -2070.0f, -206.0f, 3180.0f);
                camSetAt((Camera *) w->cam, 0.0f, 500.0f, 0.0f);
                camSetSpeed((Camera *) w->cam, 640.0f);
                camMoveEye((Camera *) w->cam, -1020.0f, 390.0f, 1930.0f);
                camMoveAt((Camera *) w->cam, 0.0f, 500.0f, 0.0f);
                w->scene++;
            }
            break;
        case 7:
            if (dochukiDemoFrame == 2574) {
                camSetFov((Camera *) w->cam, 1.2217305f);
                camSetEye((Camera *) w->cam, 50.0f, 1500.0f, 17.0f);
                camSetAt((Camera *) w->cam, -37.0f, 1030.0f, -2.0f);
                camSetSpeed((Camera *) w->cam, 800.0f);
                camMoveEye((Camera *) w->cam, 49.0f, 1870.0f, 42.0f);
                camMoveAt((Camera *) w->cam, -55.0f, 1250.0f, 12.0f);
                w->scene++;
            }
            break;
        case 8:
            if (dochukiDemoFrame == 3194) {
                demoFadeStart(80.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (dochukiDemoFrame == 3274) {
                w->view->zNear = 10.0f;
                demoFadeStart(60.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetFov((Camera *) w->cam, 1.2217305f);
                eyeX = 26.0f;
                eyeY = 39.0f;
                eyeZ = 100.0f;
                atX = 26.0f;
                atY = 39.0f;
                atZ = 0.0f;
                dEyeX = (-137.0f - eyeX) / 274.0f;
                dEyeY = (138.0f - eyeY) / 274.0f;
                dEyeZ = (36.0f - eyeZ) / 274.0f;
                dAtX = (18.0f - atX) / 274.0f;
                dAtY = (143.0f - atY) / 274.0f;
                dAtZ = (7.0f - atZ) / 274.0f;
                w->scene++;
            }
            break;
        case 9:
            camSetEye((Camera *) w->cam, eyeX, eyeY + 1300.0f, eyeZ);
            camSetAt((Camera *) w->cam, atX, atY + 1300.0f, atZ);
            eyeX += dEyeX;
            eyeY += dEyeY;
            eyeZ += dEyeZ;
            atX += dAtX;
            atY += dAtY;
            atZ += dAtZ;
            if (dochukiDemoFrame >= 3525) {
                camSetEye((Camera *) w->cam, -115.0f, 1655.0f, -141.0f);
                camSetAt((Camera *) w->cam, 612.0f, 1310.0f, -443.0f);
            }
            if (dochukiDemoFrame == 3548) {
                eyeX = -164.0f;
                eyeY = 188.0f;
                eyeZ = -1.0f;
                atX = 15.0f;
                atY = 178.0f;
                atZ = 9.0f;
                dEyeX = (-61.0f - eyeX) / 274.0f;
                dEyeY = (380.0f - eyeY) / 274.0f;
                dEyeZ = (-171.0f - eyeZ) / 274.0f;
                dAtX = (3.0f - atX) / 274.0f;
                dAtY = (341.0f - atY) / 274.0f;
                dAtZ = (20.0f - atZ) / 274.0f;
                w->scene++;
            }
            break;
        case 10:
            camSetEye((Camera *) w->cam, eyeX, eyeY + 1300.0f, eyeZ);
            camSetAt((Camera *) w->cam, atX, atY + 1300.0f, atZ);
            eyeX += dEyeX;
            eyeY += dEyeY;
            eyeZ += dEyeZ;
            atX += dAtX;
            atY += dAtY;
            atZ += dAtZ;
            if (dochukiDemoFrame >= 3799) {
                camSetEye((Camera *) w->cam, 136.0f, 1711.0f, 38.0f);
                camSetAt((Camera *) w->cam, 8.0f, 1690.0f, 4.0f);
            }
            if (dochukiDemoFrame == 3822) {
                eyeX = -16.0f;
                eyeY = 422.0f;
                eyeZ = -178.0f;
                atX = 1.0f;
                atY = 372.0f;
                atZ = 22.0f;
                dEyeX = (153.0f - eyeX) / 374.0f;
                dEyeY = (505.0f - eyeY) / 374.0f;
                dEyeZ = (-84.0f - eyeZ) / 374.0f;
                dAtX = (-4.0f - atX) / 374.0f;
                dAtY = (438.0f - atY) / 374.0f;
                dAtZ = (27.0f - atZ) / 374.0f;
                w->scene++;
            }
            break;
        case 11:
            camSetEye((Camera *) w->cam, eyeX, eyeY + 1300.0f, eyeZ);
            camSetAt((Camera *) w->cam, atX, atY + 1300.0f, atZ);
            eyeX += dEyeX;
            eyeY += dEyeY;
            eyeZ += dEyeZ;
            atX += dAtX;
            atY += dAtY;
            atZ += dAtZ;
            if (dochukiDemoFrame == 4067) {
                demoFadeStart(3.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (dochukiDemoFrame == 4072) {
                demoFadeStart(3.0f, 255.0f, 0.0f, 0, 0, 0);
            }
            if (dochukiDemoFrame >= 4073) {
                camSetEye((Camera *) w->cam, -15.0f, 1719.0f, 102.0f);
                camSetAt((Camera *) w->cam, -11.0f, 1738.0f, 0.0f);
            }
            if (dochukiDemoFrame == 4222) {
                demoFadeStart(50.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (dochukiDemoFrame == 4272) {
                w->view->zNear = 24.0f;
                demoFadeStart(60.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetFov((Camera *) w->cam, 0.87266463f);
                camSetAt((Camera *) w->cam, -16.0f, 1706.0f, 23.0f);
                camSetEye((Camera *) w->cam, 99.0f, 1609.0f, 122.0f);
                camSetSpeed((Camera *) w->cam, 200.0f);
                camMoveAt((Camera *) w->cam, -16.0f, 1706.0f, 23.0f);
                camMoveEye((Camera *) w->cam, 144.0f, 1502.0f, 174.0f);
                w->scene++;
            }
            break;
        case 12:
            if (dochukiDemoFrame == 4472) {
                gDemoModelsDelete2 = 0;
                camSetSpeed((Camera *) w->cam, 100.0f);
                camMoveAt((Camera *) w->cam, -16.0f, 1586.0f, -12.0f);
                camMoveEye((Camera *) w->cam, 439.0f, 1388.0f, 200.0f);
                w->scene++;
            }
            break;
        case 13:
            if (dochukiDemoFrame == 4612) {
                demoFadeStart(30.0f, 0.0f, 255.0f, 255, 255, 255);
                (*dochukiDemoMeffectCreate)(obj, 0.0f, 1526.0f, 0.0f);
                (*dochukiDemoMeffectCreate)(obj, 0.0f, 1646.0f, 0.0f);
                (*dochukiDemoMeffectCreate)(obj, 0.0f, 1766.0f, 0.0f);
                (*dochukiDemoHitballCreate)(obj, 0.0f, 1646.0f, 0.0f, 0.3f);
                (*dochukiDemoHitballCreate)(obj, 0.0f, 1646.0f, 0.0f, 0.4f);
                (*dochukiDemoHitballCreate)(obj, 0.0f, 1646.0f, 0.0f, 0.5f);
            }
            if (dochukiDemoFrame == 4732) {
                demoFadeStart(70.0f, 255.0f, 0.0f, 255, 255, 255);
                gDemoModelsDelete2 = 0;
                camSetEye((Camera *) w->cam, -203.0f, -4883.0f, 923.0f);
                camSetAt((Camera *) w->cam, 69.0f, -4997.0f, 3.0f);
                camSetSpeed((Camera *) w->cam, 800.0f);
                camMoveEye((Camera *) w->cam, -203.0f, -4883.0f, 723.0f);
                camMoveAt((Camera *) w->cam, 69.0f, -4997.0f, 3.0f);
                w->scene++;
            }
            break;
        case 14:
            if (dochukiDemoFrame == 5082) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (dochukiDemoFrame == 5092) {
                demoFadeStart(10.0f, 255.0f, 0.0f, 0, 0, 0);
                gDemoModelsDelete2 = 0;
                camSetFov((Camera *) w->cam, 0.87266463f);
                camSetEye((Camera *) w->cam, -9.0f, 1713.0f, 110.0f);
                camSetAt((Camera *) w->cam, 61.0f, 1388.0f, -709.0f);
                camSetSpeed((Camera *) w->cam, 260.0f);
                camMoveEye((Camera *) w->cam, -9.0f, 1841.0f, 110.0f);
                camMoveAt((Camera *) w->cam, 61.0f, 1388.0f, -709.0f);
                w->scene++;
            }
            break;
        case 15:
            if (dochukiDemoFrame == 5652) {
                scRunEnd();
                camSetEye((Camera *) w->cam, -270.0f, 1436.0f, 869.0f);
                camSetAt((Camera *) w->cam, 510.0f, 122.0f, 1352.0f);
                camSetSpeed((Camera *) w->cam, 800.0f);
                camMoveEye((Camera *) w->cam, -270.0f, 1120.0f, 869.0f);
                camMoveAt((Camera *) w->cam, 510.0f, 122.0f, 1352.0f);
                w->scene++;
            }
            break;
        case 16:
            if (dochukiDemoFrame == 6382) {
                scRunEnd();
                camSetEye((Camera *) w->cam, -266.0f, 60.0f, 2801.0f);
                camSetAt((Camera *) w->cam, 22.0f, 882.0f, 4.0f);
                camSetSpeed((Camera *) w->cam, 1000.0f);
                camMoveEye((Camera *) w->cam, 1046.0f, 60.0f, 2801.0f);
                camMoveAt((Camera *) w->cam, 22.0f, 882.0f, 4.0f);
                w->scene++;
            }
            break;
        case 17:
            if (dochukiDemoFrame == 7255) {
                demoFadeStart(18.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (dochukiDemoFrame == 7275) {
                scRunEnd();
                demoFadeStart(10.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -3.0f, -4739.0f, 955.0f);
                camSetAt((Camera *) w->cam, 13.0f, -5101.0f, 3.0f);
                camSetSpeed((Camera *) w->cam, 1000.0f);
                camMoveEye((Camera *) w->cam, -3.0f, -4739.0f, 675.0f);
                camMoveAt((Camera *) w->cam, 13.0f, -5069.0f, 3.0f);
                w->scene++;
            }
            break;
        case 18:
            if (dochukiDemoFrame == 7985) {
                demoFadeStart(10.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (dochukiDemoFrame == 8005) {
                demoFadeStart(10.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 129.0f, 1553.0f, 199.0f);
                camSetAt((Camera *) w->cam, 165.0f, 1611.0f, -41.0f);
                w->scene++;
            }
            break;
        case 19:
            if (dochukiDemoFrame == 8235) {
                camSetSpeed((Camera *) w->cam, 1000.0f);
                camMoveEye((Camera *) w->cam, 65.0f, -91.0f, 5247.0f);
                camMoveAt((Camera *) w->cam, 165.0f, 1591.0f, -25.0f);
                w->scene++;
            }
            break;
        case 20:
            if (dochukiDemoFrame == 9205) {
                scRunEnd();
                camSetAt((Camera *) w->cam, 69.0f, -5161.0f, 2.0f);
                camSetEye((Camera *) w->cam, -184.0f, -4387.0f, 1018.0f);
            }
            if (dochukiDemoFrame == 9305) {
                w->scene++;
                sndPlay(0x197);
            }
            break;
        case 21:
            quake = (dochukiDemoFrame - 9305) / 40.0f;
            if (dochukiDemoFrame % 20 == 0 && dochukiDemoSkip == 0) {
                demoRumbleAll(0x88888888, 20);
            }
            if (quake > 30.0f) {
                quake = 30.0f;
            }
            x = sinf(dochukiDemoFrame * 10) * quake;
            camSetEye((Camera *) w->cam, -184.0f, -4387.0f + x, 1018.0f);
            if (dochukiDemoFrame == 10065) {
                sndPlay(10);
                demoFadeStart(60.0f, 0.0f, 255.0f, 0, 0, 0);
                for (i = 0; i < 4; i++) {
                    contPakInitRumble(i);
                    contPakRumbleRequestStop(i);
                }
                scRunEnd();
            }
            break;
    }
    if (dochukiDemoFrame == 4580) {
        sndPlay(0x190);
    }
    camGetEye((Camera *) w->cam, &gCamChkEyeX, &gCamChkEyeY, &gCamChkEyeZ);
    camGetAt((Camera *) w->cam, &gCamChkAtX, &gCamChkAtY, &gCamChkAtZ);
    (*dochukiDemoEffectsStep)();
}

/* "DOCHUKI", the road demo as played in the game: the dragon camera, no OBJMOVE models, and
 * Start skips it (as tlb_0029_0's mach2DemoProc), stopping every pad's rumble: from 1 it counts
 * frames, at 3 fading out with sound 10, at 40 ending the scene director, at 50 going to the
 * battle (stageGotoBattle(3)); at frame 9825 the skip starts by itself. */
s32 dochukiDemoProc(Obj *obj, s16 msg, void *arg) {
    static s32 skip = 1;
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[2];
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "DOCHUKI";
            gDmenuLastObj = obj;
            w->timer = 0;
            dochukiDemoFrame = 0;
            skip = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0xC9);
            floadLoadSet(0x29);
            floadLoadSet(0xC3);
            floadLoadSet(0x12C);
            floadLoadSet(0xEE);
            tskCreate(obj, scParentProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 45, 0, 185);
            w->layer3Word = ((u32 *) gFrmLayer3)[6];
            gObjMoveCount = 0;
            gSaveData.stageFlags[0x6B] |= 0x80;
            w->cam = tskCreate(obj, dracam, 0, NULL);
            DPRINTF("カメラを作りました\n");
            camSetFov((Camera *) w->cam, 0.87266463f);
            w->view = camGetView((Camera *) w->cam);
            w->view->zFar = 20000.0f;
            w->view->zNear = 24.0f;
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
            ((LightFrame *) w->f0C)->color[0] = 240;
            ((LightFrame *) w->f0C)->color[1] = 250;
            ((LightFrame *) w->f0C)->color[2] = 255;
            w->f10 = frmCreate(gFrmLayers[0], gFrmKindLight);
            ((LightFrame *) w->f10)->color[0] = 0;
            ((LightFrame *) w->f10)->color[1] = 0;
            ((LightFrame *) w->f10)->color[2] = 0;
            ((LightFrame *) w->f10)->dir[0] = -127;
            ((LightFrame *) w->f10)->dir[1] = -127;
            ((LightFrame *) w->f10)->dir[2] = -127;
            w->f04 = frmCreate(gFrmLayers[0], gFrmKindAmbi);
            ((AmbFrame *) w->f04)->color[0] = 64;
            ((AmbFrame *) w->f04)->color[1] = 112;
            ((AmbFrame *) w->f04)->color[2] = 128;
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
            DPRINTF("地形を出します\n");
            demoModelCreate(obj, 0.0f, -5000.0f, 0.0f, 10.0f, 10.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0xC9, dochukiDemoLandDl, 0, 0);
            gCamChkStep = 4.0f;
            gObjMoveStep = 4.0f;
            gCamChkMarkerSize = 1.0f;
            (*dochukiDemoGoeCreate)(obj);
            (*dochukiDemoEbiCreate)(obj);
            (*dochukiDemoSasCreate)(obj);
            (*dochukiDemoYaeCreate)(obj);
            (*dochukiDemoBisuCreate)(obj);
            (*dochukiDemoMach12Create)(obj);
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
                    stageGotoBattle(3);
                    return -1;
                case 0:
                    break;
                default:
                    skip++;
                    break;
            }
            dochukiDemoSkip = skip;
            w->timer++;
            (*dochukiDemoCamScript)(obj);
            dochukiDemoFrame = w->timer;
            if (dochukiDemoFrame == 9825 && skip == 0) {
                skip = 1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
