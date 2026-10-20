#include "tlb.h"
#include "tlb/tlb_0032.h"

#define DPRINTF

/* A demo director's work (tlb_0021_0's OpDemoW layout). */
typedef struct ChaDemoW {
    /* 0x00 */ Obj *cam; /* "dracam" */
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
    /* 0x29 */ u8 scene;
    /* 0x2C */ s32 timer; /* frames of demo run */
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
s32 clr5DemoFrame = 1; /* the scene's frame count */
s32 clr5DemoUnused1 = 0; /* nothing reads it */

void clr5DemoCamScript(Obj *obj);

/* "CHADEMO" at the castle: a director built as OPDEMO2, at half the frame rate. It loads five
 * files, sets up the scene director, the effects, a letterboxed fade, OBJMOVE (5 models), the
 * dragon camera (50 degrees, planes 10 and 2000), the sun, OPMAIN's three lights, a black sky,
 * the ground, VRAM, the rider and the attack task, geography mode 1, tlb_0032_1's
 * clr5DemoModelCreate, and the scene's six characters. Each frame it counts and steps the demo
 * (clr5DemoCamScript) and keeps the count in clr5DemoFrame. */
s32 clr5DemoMenuProc(Obj *obj, s16 msg, void *arg) {
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "CHADEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            clr5DemoFrame = 0;
            gRetracesPerFrame = 2;
            floadLoadSet(0xBC);
            floadLoadSet(0xCD);
            floadLoadSet(0x29);
            floadLoadSet(0xCC);
            floadLoadSet(0x2B);
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
            gCamChkEyeX = 0.0f;
            gCamChkEyeY = 0.0f;
            gCamChkEyeZ = 0.0f;
            gCamChkAtX = 0.0f;
            gCamChkAtY = 1000.0f;
            gCamChkAtZ = -4000.0f;
            w->sky = tskCreate(obj, bgrad, 0, NULL);
            camBgradSetLines((Camera *) w->sky, 20, 100);
            camBgradMoveTop((ColorFade *) w->sky, 0, 0, 0);
            camBgradMoveBottom((ColorFade *) w->sky, 0, 0, 0);
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            demoJoyChkOpen();
            w->scene = 0;
            demoFadeStart(60.0f, 255.0f, 0.0f, 0, 0, 0);
            tskCreate(obj, Geography, 0, NULL);
            DPRINTF("VRAMset\n");
            vramCreate(obj);
            tskCreate(obj, Ride, 0, NULL);
            tskCreate(obj, attack_Proc, 0, NULL);
            geoSetMode(1);
            DPRINTF("城を出します\n");
            clr5DemoModelCreate(obj);
            gCamChkStep = 1.0f;
            gObjMoveStep = 1.0f;
            gCamChkMarkerSize = 0.01f;
            (*clr5DemoGoeCreate)(obj);
            (*clr5DemoEbiCreate)(obj);
            (*clr5DemoSasCreate)(obj);
            (*clr5DemoYaeCreate)(obj);
            (*clr5DemoSuzakuCreate)(obj);
            (*clr5DemoDoucyuCreate)(obj);
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
                (*clr5DemoCamScript)(obj);
            }
            clr5DemoFrame = w->timer;
            DPRINTF("DEMO22 TIME *****[ %10d ]\n", w->timer);
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

/* The 5CLR scene's camera script, one step a time (scene), on the scene's frame count; then the
 * camera check follows the camera. It opens on a background panel (cambg 6 at 800, alpha 0x40)
 * with sound 0x24 and cuts with eight-frame fades at 237, 853, 963 (a glide), 1283, 1533 (sound
 * 0x192), 2476 and 2703; a white flash at 2748, sound 0x194 at 2768, sound 0x25 and back from
 * white at 2888, sound 0x195 at 3146. At 3306 the panel goes, stage data 0x73 is set, the sky
 * turns to sunset and the frame rate goes to 60 (sounds 0x1A0 and 0x26); then cuts at 3481, 4646
 * and 5496, and at 8406 the last fade, the scene's end and sound 10. The panel is a static, the
 * first word of tlb_0032's .bss (0x080094B0). */
void clr5DemoCamScript(Obj *obj) {
    static Obj *bg;
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[3];

    switch (w->scene) {
        case 0:
            camSetEye((Camera *) w->cam, -59.0f, 12.0f, 243.0f);
            camSetAt((Camera *) w->cam, 38.0f, 16.0f, 338.0f);
            bg = cambgCreate(obj, 6, 800.0f);
            cambgMove(bg, 0, 0);
            cambgScrollY(bg, 0);
            cambgSetAlpha(bg, 0x40);
            sndPlay(0x24);
            w->scene++;
            break;
        case 1:
            if (clr5DemoFrame == 237) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (clr5DemoFrame == 247) {
                demoFadeStart(8.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -3.0f, 18.0f, 283.0f);
                camSetAt((Camera *) w->cam, -4.0f, -10.0f, 167.0f);
                w->scene++;
            }
            break;
        case 2:
            if (clr5DemoFrame == 853) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (clr5DemoFrame == 863) {
                demoFadeStart(8.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -59.0f, 12.0f, 243.0f);
                camSetAt((Camera *) w->cam, 38.0f, 16.0f, 338.0f);
                w->scene++;
            }
            break;
        case 3:
            if (clr5DemoFrame == 963) {
                w->scene++;
                camSetSpeed((Camera *) w->cam, 150.0f);
                camMoveEye((Camera *) w->cam, -9.0f, 18.0f, 288.0f);
                camMoveAt((Camera *) w->cam, 10.0f, 22.0f, 315.0f);
            }
            break;
        case 4:
            if (clr5DemoFrame == 1283) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (clr5DemoFrame == 1293) {
                demoFadeStart(8.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 3.0f, 33.0f, 259.0f);
                camSetAt((Camera *) w->cam, 95.0f, -155.0f, 125.0f);
                w->scene++;
            }
            break;
        case 5:
            if (clr5DemoFrame == 1533) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (clr5DemoFrame == 1543) {
                sndPlay(0x192);
                demoFadeStart(8.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -12.0f, -13.0f, 174.0f);
                camSetAt((Camera *) w->cam, -5.0f, 24.0f, 248.0f);
                w->scene++;
            }
            break;
        case 6:
            if (clr5DemoFrame == 2476) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (clr5DemoFrame == 2486) {
                demoFadeStart(8.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 15.0f, 40.0f, 412.0f);
                camSetAt((Camera *) w->cam, 24.0f, 23.0f, 398.0f);
                w->scene++;
            }
            break;
        case 7:
            if (clr5DemoFrame == 2703) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (clr5DemoFrame == 2713) {
                demoFadeStart(8.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -11.0f, 44.0f, 300.0f);
                camSetAt((Camera *) w->cam, 40.0f, 22.0f, 290.0f);
            }
            if (clr5DemoFrame == 2748) {
                demoFadeStart(100.0f, 0.0f, 255.0f, 255, 255, 255);
            }
            if (clr5DemoFrame == 2768) {
                sndPlay(0x194);
                w->scene++;
            }
            break;
        case 8:
            if (clr5DemoFrame == 2888) {
                sndPlay(0x25);
                demoFadeStart(8.0f, 255.0f, 0.0f, 255, 255, 255);
                camSetEye((Camera *) w->cam, 7.0f, 14.0f, 413.0f);
                camSetAt((Camera *) w->cam, 293.0f, -29.0f, 157.0f);
                w->scene++;
            }
            break;
        case 9:
            if (clr5DemoFrame == 3136) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (clr5DemoFrame == 3146) {
                sndPlay(0x195);
                demoFadeStart(8.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 2.0f, 22.0f, 278.0f);
                camSetAt((Camera *) w->cam, 3.0f, 7.0f, 245.0f);
                w->scene++;
            }
            break;
        case 10:
            if (clr5DemoFrame == 3296) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (clr5DemoFrame == 3306) {
                tskDestroy(bg);
                stageSetData(0x73);
                camBgradMoveTop((ColorFade *) w->sky, 255, 64, 16);
                camBgradMoveBottom((ColorFade *) w->sky, 255, 240, 48);
                gRetracesPerFrame = 1;
            }
            if (clr5DemoFrame == 3312) {
                sndPlay(0x1A0);
                sndPlay(0x26);
            }
            if (clr5DemoFrame == 3316) {
                demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -75.0f, 20.0f, 700.0f);
                camSetAt((Camera *) w->cam, 490.0f, 5.0f, 430.0f);
                w->scene++;
            }
            break;
        case 11:
            if (clr5DemoFrame == 3481) {
                demoFadeStart(20.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (clr5DemoFrame == 3506) {
                demoFadeStart(20.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 77.0f, 10.0f, 673.0f);
                camSetAt((Camera *) w->cam, 131.0f, 15.0f, 614.0f);
                w->scene++;
            }
            break;
        case 12:
            if (clr5DemoFrame == 4646) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (clr5DemoFrame == 4656) {
                demoFadeStart(8.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 100.0f, 11.0f, 663.0f);
                camSetAt((Camera *) w->cam, -61.0f, 15.0f, 648.0f);
                w->scene++;
            }
            break;
        case 13:
            if (clr5DemoFrame == 5496) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (clr5DemoFrame == 5506) {
                demoFadeStart(8.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 63.0f, 29.0f, 708.0f);
                camSetAt((Camera *) w->cam, 86.0f, 9.0f, 660.0f);
                w->scene++;
            }
            break;
        case 14:
            if (clr5DemoFrame == 8406) {
                demoFadeStart(30.0f, 0.0f, 255.0f, 0, 0, 0);
                scRunEnd();
                sndPlay(10);
            }
            break;
    }
    camGetEye((Camera *) w->cam, &gCamChkEyeX, &gCamChkEyeY, &gCamChkEyeZ);
    camGetAt((Camera *) w->cam, &gCamChkAtX, &gCamChkAtY, &gCamChkAtZ);
}

extern s32 gDemoModelsDelete2;

/* "5CLRDEMO": the castle scene after area 5, as the game plays it. It sets up as the debug
 * director (clr5DemoMenuProc) does, with no OBJMOVE models, marks stage 0x70's flag 0x80
 * and fades in from white. Start skips it; its skip count runs one of two ends, by the frame
 * rate. At one retrace a frame: a fade (speed 30) and sound 10 at 3, the scene ends at 40, and
 * at 50 it goes to the big map; it ends by itself at frame 8146. At two: the same at speed 15,
 * at 3, 20 and 25; it ends by itself at frame 8206. */
s32 clr5DemoProc(Obj *obj, s16 msg, void *arg) {
    static s32 skip = 2;
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "5CLRDEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            clr5DemoFrame = 0;
            gRetracesPerFrame = 2;
            gSaveData.stageFlags[0x70] |= 0x80;
            skip = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0xCD);
            floadLoadSet(0x29);
            floadLoadSet(0xCC);
            floadLoadSet(0x2B);
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
            demoFadeStart(60.0f, 255.0f, 0.0f, 255, 255, 255);
            tskCreate(obj, Geography, 0, NULL);
            DPRINTF("VRAMset\n");
            vramCreate(obj);
            tskCreate(obj, Ride, 0, NULL);
            tskCreate(obj, attack_Proc, 0, NULL);
            geoSetMode(1);
            DPRINTF("城を出します\n");
            (*clr5DemoModelCreate)(obj);
            gCamChkStep = 1.0f;
            gObjMoveStep = 1.0f;
            gCamChkMarkerSize = 0.01f;
            (*clr5DemoGoeCreate)(obj);
            (*clr5DemoEbiCreate)(obj);
            (*clr5DemoSasCreate)(obj);
            (*clr5DemoYaeCreate)(obj);
            (*clr5DemoSuzakuCreate)(obj);
            (*clr5DemoDoucyuCreate)(obj);
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
            if (gRetracesPerFrame == 1) {
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
                        DPRINTF("大マップに飛びます\n");
                        while (gfxGetPendingSwaps()) {
                        }
                        stageGotoAtlasFrom(NULL, 0, -3);
                        return -1;
                    case 0:
                        if (clr5DemoFrame >= 8146) {
                            skip = 1;
                        }
                        break;
                    default:
                        skip++;
                        break;
                }
            } else {
                switch (skip) {
                    case 3:
                        demoFadeForce(15.0f, 0.0f, 255.0f, 0, 0, 0);
                        sndPlay(10);
                        skip++;
                        break;
                    case 20:
                        gDemoModelsDelete2 = 1;
                        scRunEnd();
                        skip++;
                        break;
                    case 25:
                        DPRINTF("大マップに飛びます\n");
                        while (gfxGetPendingSwaps()) {
                        }
                        stageGotoAtlasFrom(NULL, 0, -3);
                        return -1;
                    case 0:
                        if (clr5DemoFrame >= 8206) {
                            skip = 1;
                        }
                        break;
                    default:
                        skip++;
                        break;
                }
            }
            w->timer++;
            (*clr5DemoCamScript)(obj);
            clr5DemoFrame = w->timer;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
