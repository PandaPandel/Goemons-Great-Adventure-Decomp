#include "tlb.h"
#include "tlb/tlb_0029.h"

/* Ends the running scenario and starts scenario n. */
void mach2DemoCamScenarioStart(s32 n) {
    scRunEnd();
    scRunStart(n, 0, 0);
}

#define DPRINTF

/* A demo director's work (tlb_0023_0's CHADEMO layout). */
typedef struct ChaDemoW {
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
/* This file's .data is mach2DemoFrame, mach2DemoSkip, mach2DemoUnused1 and mach2DemoProc's skip
 * (0x08004C00-0x08004C0F); tlb_0029_2's starts at 0x08004C10 (tlb_0029_1 has none). */
s32 mach2DemoFrame = 1; /* the scene's frame count */
extern Gfx mach2DemoCastleDl[]; /* the castle */

void mach2DemoCamScript(Obj *obj);

/* "CHADEMO" at the castle at night: a director built as OPDEMO2. It loads files 0xC2, 0xBC and
 * 0xCB, sets up the scene director, the effects, a letterboxed fade, OBJMOVE (5 models), the
 * dragon camera (50 degrees, planes 10 and 2000), the sun, a black sky, the attack task and the
 * castle model of file 0xCB, and starts the scene's five characters (tlb_0029_2's, genoGOE among
 * them). Each frame it counts and steps the demo (mach2DemoCamScript) and keeps the count in
 * mach2DemoFrame. */
s32 mach2DemoMenuProc(Obj *obj, s16 msg, void *arg) {
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "CHADEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            mach2DemoFrame = 0;
            floadLoadSet(0xC2);
            floadLoadSet(0xBC);
            floadLoadSet(0xCB);
            tskCreate(obj, scParentProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 45, 0, 185);
            w->layer3Word = ((u32 *) gFrmLayer3)[6];
            gObjMoveCount = 5;
            demoObjMoveOpen();
            w->cam = tskCreate(obj, dracam, 0, NULL);
            DPRINTF("カメラを作りました\n");
            camSetFov((Camera *) w->cam, 0.87266463f);
            w->view = camGetView((Camera *) w->cam);
            w->view->zFar = 2000.0f;
            w->view->zNear = 10.0f;
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
            DPRINTF("城を出します\n");
            demoModelCreate(obj, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0xCB, mach2DemoCastleDl, 0, 0);
            gCamChkStep = 1.0f;
            gObjMoveStep = 1.0f;
            gCamChkMarkerSize = 0.01f;
            (*mach2DemoGoeCreate)(obj);
            (*mach2DemoEbiCreate)(obj);
            (*mach2DemoSasCreate)(obj);
            (*mach2DemoYaeCreate)(obj);
            (*mach2DemoMachineCreate)(obj);
            camSunSetClockStop(1);
            DPRINTF("SD_D_OUT1 set!\n");
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
                (*mach2DemoCamScript)(obj);
            }
            mach2DemoFrame = w->timer;
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

s32 mach2DemoSkip = 0; /* mach2DemoProc's skip, copied each frame: the quake rumbles only at 0 */
u32 mach2DemoUnused1 = 0x00000000;

#define sinf (*sinf)

/* The castle CHADEMO's camera script, one step a time (scene), on the scene's frame count; then
 * the camera check follows the camera and mach2DemoEffectsStep runs. 0: the camera low by the
 * castle, a fade in, sounds 0x29 and 0x19F. 1: at 313 it climbs. 2: at 694 it swings high; at
 * 1475 a cut. 3: at 2046 a cut, sound 0x219 at 2114, at 2189 a white flash and sound 0x513.
 * 4: sound 0x513 every 12 frames, 0x11D at 2389, at 2459 a cut and a white fade in. 5: sound
 * 0x197 at 2769, then the quake: the camera shakes (a sine on the frame's low bits, growing over
 * each 64 frames), from 3220 on a second view, with a rumble every 20 frames; at 3250 scenario
 * 0x120; at 4210 the rumble stops on all four pads and the scene director ends.
 * The game multiplies by 1.0f and by the double 1.0, which IDO folds away when they are
 * literals; as locals (amp, scale) they stay, as in the game. */
void mach2DemoCamScript(Obj *obj) {
    f64 scale;
    f32 amp;
    s32 i;
    f32 shake;
    s32 pad[2];
    ChaDemoW *w = (ChaDemoW *) (obj + 1);

    switch (w->scene) {
        case 0:
            camSetEye((Camera *) w->cam, 19.0f, 8.0f, 15.0f);
            camSetAt((Camera *) w->cam, -25.0f, 18.0f, 207.0f);
            demoFadeStart(10.0f, 255.0f, 0.0f, 0, 0, 0);
            sndPlay(0x29);
            sndPlay(0x19F);
            w->scene++;
            break;
        case 1:
            if (mach2DemoFrame == 313) {
                camSetSpeed((Camera *) w->cam, 220.0f);
                camMoveEye((Camera *) w->cam, 34.0f, 25.0f, 44.0f);
                camMoveAt((Camera *) w->cam, -3.0f, 31.0f, -96.0f);
                w->scene++;
            }
            break;
        case 2:
            if (mach2DemoFrame == 694) {
                camSetSpeed((Camera *) w->cam, 780.0f);
                camMoveEye((Camera *) w->cam, 94.0f, 68.0f, -22.0f);
                camMoveAt((Camera *) w->cam, 1.0f, 31.0f, -96.0f);
            }
            if (mach2DemoFrame == 1475) {
                camSetEye((Camera *) w->cam, 23.0f, 3.0f, 28.0f);
                camSetAt((Camera *) w->cam, -121.0f, 109.0f, 331.0f);
                w->scene++;
            }
            break;
        case 3:
            if (mach2DemoFrame == 2046) {
                camSetEye((Camera *) w->cam, 31.0f, 22.0f, 112.0f);
                camSetAt((Camera *) w->cam, 0.0f, 36.0f, -80.0f);
            }
            if (mach2DemoFrame == 2114) {
                sndPlay(0x219);
            }
            if (mach2DemoFrame == 2189) {
                demoFadeStart(210.0f, 0.0f, 255.0f, 255, 255, 255);
                sndPlay(0x513);
                w->scene++;
            }
            break;
        case 4:
            if ((mach2DemoFrame - 2189) % 12 == 0 && mach2DemoFrame < 2387) {
                sndPlay(0x513);
            }
            if (mach2DemoFrame == 2389) {
                sndPlay(0x11D);
            }
            if (mach2DemoFrame == 2459) {
                camSetEye((Camera *) w->cam, 19.0f, 6.0f, 25.0f);
                camSetAt((Camera *) w->cam, -138.0f, 84.0f, 328.0f);
                demoFadeStart(70.0f, 255.0f, 0.0f, 255, 255, 255);
                w->scene++;
            }
            break;
        case 5:
            if (mach2DemoFrame == 2769) {
                sndPlay(0x197);
            }
            if (mach2DemoFrame >= 2770) {
                amp = 1.0f;
                scale = 1.0;
                shake = sinf((mach2DemoFrame & 7) * 0.7853981851746036) * amp *
                        (mach2DemoFrame & 0x3F) / 63.0f;
                shake *= scale;
                if (mach2DemoFrame < 3220) {
                    camSetEye((Camera *) w->cam, 19.0f, 6.0f + shake, 25.0f);
                    camSetAt((Camera *) w->cam, -138.0f, 84.0f + shake, 328.0f);
                    if (mach2DemoFrame % 20 == 0 && mach2DemoSkip == 0) {
                        demoRumbleAll(0xBBBBBBBB, 20);
                    }
                } else {
                    camSetEye((Camera *) w->cam, 65.0f, 57.0f, -40.0f);
                    camSetAt((Camera *) w->cam, -150.0f, -102.0f + shake, 174.0f);
                    if (mach2DemoFrame % 20 == 0 && mach2DemoSkip == 0) {
                        demoRumbleAll(0x88888888, 20);
                    }
                }
            }
            if (mach2DemoFrame == 3250) {
                mach2DemoCamScenarioStart(0x120);
            }
            if (mach2DemoFrame == 4210) {
                for (i = 0; i < 4; i++) {
                    contPakInitRumble(i);
                    contPakRumbleRequestStop(i);
                }
                scRunEnd();
            }
            break;
    }
    camGetEye((Camera *) w->cam, &gCamChkEyeX, &gCamChkEyeY, &gCamChkEyeZ);
    camGetAt((Camera *) w->cam, &gCamChkAtX, &gCamChkAtY, &gCamChkAtZ);
    (*mach2DemoEffectsStep)();
}

/* The castle CHADEMO as played in the game: files 0xC2, 0xBC and 0xCB, stage flag 0x6D (bit 7),
 * no OBJMOVE models, the castle model (file 0xCB) and the scene's five builders, and Start skips
 * it (as tlb_0030_0's st4clearProc), stopping every pad's rumble: from 1 it counts frames, at 3
 * fading out with sound 10, at 40 ending the scene director, at 50 going to the battle
 * (stageGotoBattle(4)); past frame 4280 the skip starts by itself. The skip is copied to
 * mach2DemoSkip each frame, so the quake does not rumble once it runs. */
extern s32 gDemoModelsDelete2;

s32 mach2DemoProc(Obj *obj, s16 msg, void *arg) {
    static s32 skip = 1;
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[2];
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "CHADEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            mach2DemoFrame = 0;
            gSaveData.stageFlags[0x6D] |= 0x80;
            skip = 0;
            floadLoadSet(0xC2);
            floadLoadSet(0xBC);
            floadLoadSet(0xCB);
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
            w->view->zFar = 2000.0f;
            w->view->zNear = 10.0f;
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
            DPRINTF("城を出します\n");
            demoModelCreate(obj, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0xCB, mach2DemoCastleDl, 0, 0);
            gCamChkStep = 1.0f;
            gObjMoveStep = 1.0f;
            gCamChkMarkerSize = 0.01f;
            (*mach2DemoGoeCreate)(obj);
            (*mach2DemoEbiCreate)(obj);
            (*mach2DemoSasCreate)(obj);
            (*mach2DemoYaeCreate)(obj);
            (*mach2DemoMachineCreate)(obj);
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
                    stageGotoBattle(4);
                    return -1;
                case 0:
                    break;
                default:
                    skip++;
                    break;
            }
            if (mach2DemoFrame >= 4280 && skip == 0) {
                skip = 1;
            }
            mach2DemoSkip = skip;
            w->timer++;
            (*mach2DemoCamScript)(obj);
            mach2DemoFrame = w->timer;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

