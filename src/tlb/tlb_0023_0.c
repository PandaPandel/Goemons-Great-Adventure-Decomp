#include "tlb.h"
#include "tlb/tlb_0023.h"

#define DPRINTF

/* "CHADEMO"'s work: its camera, view, lights and sky (tlb_0021_0's OpDemoW, laid out apart). */
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
/* This file's .data: omiDemoFrame, then IDO's padding to 16 (0x08008320-0x0800832F);
 * tlb_0023_1's starts at 0x08008330. */
s32 omiDemoFrame = 5; /* the scene's frame count */
extern Gfx omiDemoGroundDl[]; /* the ground */

void omiDemoCamScript(Obj *obj);

/* "CHADEMO": a character demo's director, built as tlb_0021_0's OPDEMO2. It loads eight files,
 * sets up the scene director, the effects, a full-screen fade and then a letterboxed one,
 * OBJMOVE (7 models), the dragon camera (50 degrees, far plane 20000), the sun, a deep sky, the
 * attack task and the ground model (150 down), and the scene's six builders. Each frame it
 * counts and steps the demo (omiDemoCamScript) and keeps the count in omiDemoFrame.
 * Controller 2's C-down holds the demo; controller 1's L opens the camera check after 10
 * frames.
 * The hold test has an empty else (here and in tlb_0020_0's, tlb_0022_0's and tlb_0025_0's
 * directors; tlb_0024_0's, tlb_0026_0's and tlb_0028_0's have none): IDO keeps the empty arm,
 * so the step ends in a b over it, the test is a plain bnez with a nop and w (s0) is re-formed
 * in the step. Without it IDO takes a branch-likely into the join (2 instructions short). */
s32 omiDemoMenuProc(Obj *obj, s16 msg, void *arg) {
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "CHADEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0x25);
            floadLoadSet(0xC5);
            floadLoadSet(0xC7);
            floadLoadSet(0xBF);
            floadLoadSet(0x13A);
            floadLoadSet(0xC6);
            floadLoadSet(0x13B);
            tskCreate(obj, scParentProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 0, 0, 240);
            w->layer3Word = ((u32 *) gFrmLayer3)[6];
            gObjMoveCount = 7;
            demoObjMoveOpen();
            w->cam = tskCreate(obj, dracam, 0, NULL);
            DPRINTF("カメラを作りました\n");
            camSetFov((Camera *) w->cam, 0.87266463f);
            w->view = camGetView((Camera *) w->cam);
            w->view->zFar = 20000.0f;
            camSunCreate(obj);
            camSunSetView(w->view);
            gCamChkEyeX = 0.0f;
            gCamChkEyeY = 0.0f;
            gCamChkEyeZ = 0.0f;
            gCamChkAtX = 0.0f;
            gCamChkAtY = 1000.0f;
            gCamChkAtZ = -4000.0f;
            w->sky = tskCreate(obj, bgrad, 0, NULL);
            camBgradSetLines((Camera *) w->sky, 36, 240);
            camBgradMoveTop((ColorFade *) w->sky, 48, 48, 255);
            camBgradMoveBottom((ColorFade *) w->sky, 128, 128, 255);
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            demoJoyChkOpen();
            w->scene = 0;
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 45, 0, 185);
            demoFadeStart(60.0f, 255.0f, 0.0f, 255, 255, 255);
            tskCreate(obj, attack_Proc, 0, NULL);
            DPRINTF("地面を出します\n");
            demoModelCreate(obj, 0.0f, -150.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0x25, omiDemoGroundDl, 0, 0);
            gCamChkStep = 1.0f;
            gObjMoveStep = 1.0f;
            gCamChkMarkerSize = 0.01f;
            (*omiDemoGoeCreate)(obj);
            (*omiDemoEbiCreate)(obj);
            (*omiDemoSasCreate)(obj);
            (*omiDemoOmiCreate)(obj);
            (*omiDemoJijiCreate)(obj);
            (*omiDemoKuroCreate)(obj);
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
                (*omiDemoCamScript)(obj);
            } else {
                /* empty in the game too: its b over this arm is in the code */
            }
            omiDemoFrame = w->timer;
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

/* This CHADEMO's camera script, one step a time (scene), on the scene's frame count; then the
 * camera check follows the camera. 0: sound 0x1D at frame 4; at 204 the camera turns. 1-6: a
 * fade out and a fade in on the next view (444, 1594, 2096, 2346, 2686, 3876), the first two
 * ending the running scenario, with sounds 0x1AA and 0x1AB on the glides at 1594 and 2096.
 * 7: at 5686 the last fade out and sound 10. */
void omiDemoCamScript(Obj *obj) {
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[3];

    switch (w->scene) {
        case 0:
            camSetEye((Camera *) w->cam, 2.0f, 18.0f, -15.0f);
            camSetAt((Camera *) w->cam, 4.0f, -3.0f, -105.0f);
            if (omiDemoFrame == 4) {
                sndPlay(0x1D);
            }
            if (omiDemoFrame == 204) {
                camSetSpeed((Camera *) w->cam, 160.0f);
                camMoveEye((Camera *) w->cam, 2.0f, 18.0f, -15.0f);
                camMoveAt((Camera *) w->cam, -36.0f, 4.0f, 31.0f);
                w->scene++;
            }
            break;
        case 1:
            if (omiDemoFrame == 414) {
                demoFadeStart(30.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (omiDemoFrame == 444) {
                scRunEnd();
                demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 16.0f, 14.0f, -29.0f);
                camSetAt((Camera *) w->cam, -13.0f, 11.0f, -82.0f);
                w->scene++;
            }
            break;
        case 2:
            if (omiDemoFrame == 1564) {
                demoFadeStart(30.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (omiDemoFrame == 1594) {
                scRunEnd();
                demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -2.0f, 129.0f, 73.0f);
                camSetAt((Camera *) w->cam, 0.0f, -51.0f, -88.0f);
                camSetSpeed((Camera *) w->cam, 1000.0f);
                camMoveEye((Camera *) w->cam, -3.0f, 80.0f, 30.0f);
                camMoveAt((Camera *) w->cam, 0.0f, -34.0f, -88.0f);
                sndPlay(0x1AA);
                w->scene++;
            }
            break;
        case 3:
            if (omiDemoFrame == 2066) {
                demoFadeStart(30.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (omiDemoFrame == 2096) {
                demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
                sndPlay(0x1AB);
                camSetEye((Camera *) w->cam, -10.0f, 4.0f, -118.0f);
                camSetAt((Camera *) w->cam, 0.0f, 226.0f, 896.0f);
                camSetSpeed((Camera *) w->cam, 300.0f);
                camMoveEye((Camera *) w->cam, -10.0f, 4.0f, -118.0f);
                camMoveAt((Camera *) w->cam, 0.0f, 226.0f, 121.0f);
                w->scene++;
            }
            break;
        case 4:
            if (omiDemoFrame == 2316) {
                demoFadeStart(30.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (omiDemoFrame == 2346) {
                demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -5.0f, 95.0f, 87.0f);
                camSetAt((Camera *) w->cam, -1.0f, 268.0f, 221.0f);
                w->scene++;
            }
            break;
        case 5:
            if (omiDemoFrame == 2656) {
                demoFadeStart(30.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (omiDemoFrame == 2686) {
                demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -3.0f, 80.0f, 30.0f);
                camSetAt((Camera *) w->cam, 0.0f, -34.0f, -88.0f);
                w->scene++;
            }
            break;
        case 6:
            if (omiDemoFrame == 2666) {
                demoFadeStart(20.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (omiDemoFrame == 3876) {
                demoFadeStart(20.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -33.0f, 16.0f, -24.0f);
                camSetAt((Camera *) w->cam, 77.0f, -1.0f, -81.0f);
                w->scene++;
            }
            break;
        case 7:
            if (omiDemoFrame == 5686) {
                demoFadeStart(50.0f, 0.0f, 255.0f, 0, 0, 0);
                sndPlay(10);
            }
            break;
    }
    camGetEye((Camera *) w->cam, &gCamChkEyeX, &gCamChkEyeY, &gCamChkEyeZ);
    camGetAt((Camera *) w->cam, &gCamChkAtX, &gCamChkAtY, &gCamChkAtZ);
}

void omiDemoShortCamScript(Obj *obj);

/* The demo menu's "CHADEMO" of this scene: omiDemoMenuProc's set-up with five files, a fade in
 * from black, and three builders; each frame it counts and runs the camera step
 * (omiDemoShortCamScript), with no hold and no camera check. */
s32 omiDemoShortMenuProc(Obj *obj, s16 msg, void *arg) {
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "CHADEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0x25);
            floadLoadSet(0xC5);
            floadLoadSet(0xC7);
            floadLoadSet(0xBF);
            tskCreate(obj, scParentProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 0, 0, 240);
            w->layer3Word = ((u32 *) gFrmLayer3)[6];
            gObjMoveCount = 7;
            demoObjMoveOpen();
            w->cam = tskCreate(obj, dracam, 0, NULL);
            DPRINTF("カメラを作りました\n");
            camSetFov((Camera *) w->cam, 0.87266463f);
            w->view = camGetView((Camera *) w->cam);
            w->view->zFar = 20000.0f;
            camSunCreate(obj);
            camSunSetView(w->view);
            gCamChkEyeX = 0.0f;
            gCamChkEyeY = 0.0f;
            gCamChkEyeZ = 0.0f;
            gCamChkAtX = 0.0f;
            gCamChkAtY = 1000.0f;
            gCamChkAtZ = -4000.0f;
            w->sky = tskCreate(obj, bgrad, 0, NULL);
            camBgradSetLines((Camera *) w->sky, 36, 240);
            camBgradMoveTop((ColorFade *) w->sky, 48, 48, 255);
            camBgradMoveBottom((ColorFade *) w->sky, 128, 128, 255);
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            demoJoyChkOpen();
            w->scene = 0;
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 45, 0, 185);
            demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
            tskCreate(obj, attack_Proc, 0, NULL);
            DPRINTF("地面を出します\n");
            demoModelCreate(obj, 0.0f, -150.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0x25, omiDemoGroundDl, 0, 0);
            gCamChkStep = 1.0f;
            gObjMoveStep = 1.0f;
            gCamChkMarkerSize = 0.01f;
            (*omiDemoShortOmiCreate)(obj);
            (*omiDemoShortJijiCreate)(obj);
            (*omiDemoShortKuroCreate)(obj);
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
            (*omiDemoShortCamScript)(obj);
            omiDemoFrame = w->timer;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The scene's camera step's work (its two callers pass their own task): only what it touches. */
typedef struct Tlb23CamW {
    /* 0x00 */ Camera *cam;
    /* 0x04 */ Frame *f04;
    /* 0x08 */ CamView *view;
    /* 0x0C */ Frame *f0C;
    /* 0x10 */ Frame *f10;
    /* 0x14 */ u8 pad14[4];
    /* 0x18 */ Obj *sky;
    /* 0x1C */ Frame *f1C;
    /* 0x20 */ u32 layer3Word;
    /* 0x24 */ u8 unused24;
    /* 0x25 */ u8 state; /* 0: the set shot; 1: done */
} Tlb23CamW;

extern s32 omiDemoFrame; /* the scene's frame count */

/* A camera step: holds the set shot (eye 10, 14, -55, looking at -25, 13, -10); at frame 1390 it
 * fades out to black over 60 (sound 10), ends the scenario and is done. */
void omiDemoShortCamScript(Obj *obj) {
    Tlb23CamW *w = (Tlb23CamW *) FX(obj);

    switch (w->state) {
        case 0:
            camSetEye(w->cam, 10.0f, 14.0f, -55.0f);
            camSetAt(w->cam, -25.0f, 13.0f, -10.0f);
            if (omiDemoFrame == 1390) {
                demoFadeStart(60.0f, 0.0f, 255.0f, 0, 0, 0);
                sndPlay(10);
                scRunEnd();
                w->state++;
            }
            break;
        case 1:
            break;
    }
}

extern s32 gDemoModelsDelete2;

/* The release "CHADEMO": the day demo as the game plays it. It sets up as the debug one
 * (omiDemoMenuProc) does, with lights (a white key light, a black fill and a grey
 * ambient), and marks stage 0x66's flag 0x80. Start skips it, and it ends by itself at frame
 * 5446. The skip count runs its end: a fade to black with sound 10 at 3, the models go and the
 * scene ends at 40, and at 50 it waits for the screen and goes to battle 1.
 * Its skip count is a zero-initialised static, the first word of tlb_0023's .bss (0x08009630).
 * Case 40's scRunEnd result (an Obj *) keeps v0 busy, so the count takes v1 as in the game; case
 * 2's gDemoCamHold = 0 comes before w->scene = 0, the game's store order.
 * Matched 2026-10-12; it was parked for the skip count's register. */
s32 omiDemoProc(Obj *obj, s16 msg, void *arg) {
    static s32 skip;
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "CHADEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0x25);
            floadLoadSet(0xC5);
            floadLoadSet(0xC7);
            floadLoadSet(0xBF);
            floadLoadSet(0x13A);
            floadLoadSet(0xC6);
            floadLoadSet(0x13B);
            tskCreate(obj, scParentProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 0, 0, 240);
            w->layer3Word = ((u32 *) gFrmLayers[3])[6];
            gObjMoveCount = 7;
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
            w->cam = tskCreate(obj, dracam, 0, NULL);
            DPRINTF("カメラを作りました\n");
            camSetFov((Camera *) w->cam, 0.87266463f);
            w->view = camGetView((Camera *) w->cam);
            w->view->zFar = 20000.0f;
            gSaveData.stageFlags[0x66] |= 0x80;
            gCamChkEyeX = 0.0f;
            gCamChkEyeY = 0.0f;
            gCamChkEyeZ = 0.0f;
            gCamChkAtX = 0.0f;
            gCamChkAtY = 1000.0f;
            gCamChkAtZ = -4000.0f;
            w->sky = tskCreate(obj, bgrad, 0, NULL);
            camBgradSetLines((Camera *) w->sky, 20, 100);
            camBgradMoveTop((ColorFade *) w->sky, 48, 48, 255);
            camBgradMoveBottom((ColorFade *) w->sky, 128, 128, 255);
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            w->scene = 0;
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 45, 0, 185);
            demoFadeStart(60.0f, 255.0f, 0.0f, 255, 255, 255);
            tskCreate(obj, attack_Proc, 0, NULL);
            DPRINTF("地面を出します\n");
            demoModelCreate(obj, 0.0f, -150.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0x25, omiDemoGroundDl, 0, 0);
            gCamChkStep = 1.0f;
            gObjMoveStep = 1.0f;
            gCamChkMarkerSize = 0.01f;
            (*omiDemoGoeCreate)(obj);
            (*omiDemoEbiCreate)(obj);
            (*omiDemoSasCreate)(obj);
            (*omiDemoOmiCreate)(obj);
            (*omiDemoJijiCreate)(obj);
            (*omiDemoKuroCreate)(obj);
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
                    stageGotoBattle(1);
                    return -1;
                case 0:
                    break;
                default:
                    skip++;
                    break;
            }
            w->timer++;
            (*omiDemoCamScript)(obj);
            omiDemoFrame = w->timer;
            if (omiDemoFrame == 5446 && skip == 0) {
                skip = 1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The second release "CHADEMO": the short scene (1150 frames) of the same director, as the
 * debug one (omiDemoShortMenuProc) builds it: five files, no OBJMOVE models, the lights, one
 * retrace a frame, the deep sky over the whole screen, a fade in from black and three builders.
 * Start skips it: at 3 sound 10 and a fade to black, at 40 the models go and the scene ends,
 * and at 50 it waits for the screen and goes to battle 1. Its skip count is the second word of
 * tlb_0023's .bss (0x08009634). Its cases 40 and 2 are written as omiDemoProc's.
 * Matched 2026-10-12; it was parked for the skip count's register, as omiDemoProc. */
s32 omiDemoShortProc(Obj *obj, s16 msg, void *arg) {
    static s32 skip;
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "CHADEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0x25);
            floadLoadSet(0xC5);
            floadLoadSet(0xC7);
            floadLoadSet(0xBF);
            tskCreate(obj, scParentProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 0, 0, 240);
            w->layer3Word = ((u32 *) gFrmLayers[3])[6];
            gObjMoveCount = 0;
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
            gRetracesPerFrame = 1;
            w->cam = tskCreate(obj, dracam, 0, NULL);
            DPRINTF("カメラを作りました\n");
            camSetFov((Camera *) w->cam, 0.87266463f);
            w->view = camGetView((Camera *) w->cam);
            w->view->zFar = 20000.0f;
            gCamChkEyeX = 0.0f;
            gCamChkEyeY = 0.0f;
            gCamChkEyeZ = 0.0f;
            gCamChkAtX = 0.0f;
            gCamChkAtY = 1000.0f;
            gCamChkAtZ = -4000.0f;
            w->sky = tskCreate(obj, bgrad, 0, NULL);
            camBgradSetLines((Camera *) w->sky, 36, 240);
            camBgradMoveTop((ColorFade *) w->sky, 48, 48, 255);
            camBgradMoveBottom((ColorFade *) w->sky, 128, 128, 255);
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            w->scene = 0;
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 45, 0, 185);
            demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
            tskCreate(obj, attack_Proc, 0, NULL);
            DPRINTF("地面を出します\n");
            demoModelCreate(obj, 0.0f, -150.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0x25, omiDemoGroundDl, 0, 0);
            gCamChkStep = 1.0f;
            gObjMoveStep = 1.0f;
            gCamChkMarkerSize = 0.01f;
            (*omiDemoShortOmiCreate)(obj);
            (*omiDemoShortJijiCreate)(obj);
            (*omiDemoShortKuroCreate)(obj);
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
            }
            switch (skip) {
                case 3:
                    sndPlay(10);
                    demoFadeForce(30.0f, 0.0f, 255.0f, 0, 0, 0);
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
                    stageGotoBattle(1);
                    return -1;
                case 0:
                    break;
                default:
                    skip++;
                    break;
            }
            w->timer++;
            (*omiDemoShortCamScript)(obj);
            omiDemoFrame = w->timer;
            if (omiDemoFrame == 1150 && skip == 0) {
                skip = 1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
