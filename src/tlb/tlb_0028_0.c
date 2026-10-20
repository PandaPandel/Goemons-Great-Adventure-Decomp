#include "tlb.h"
#include "tlb/tlb_0028.h"

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
/* This file's .data is st3clearFrame, st3clearUnused1 and st3clearProc's skip, then IDO's padding
 * to 16 (0x08004BF0-0x08004BFF); tlb_0028_1's starts at 0x08004C00. */
s32 st3clearFrame = 1; /* the scene's frame count */
u32 st3clearUnused1 = 0x00000000;

void st3clearCamScript(Obj *obj);

/* "CHADEMO" with a red sky: a director built as OPDEMO2. It loads files 0xBC and 0xC5, sets up
 * the scene director, the effects, a letterboxed fade, OBJMOVE (5 models), the dragon camera
 * (50 degrees, planes 10 and 2000), the sun, a red sky, the ground, VRAM, the rider and the
 * attack task, stage data 0x71 and geography mode 1, and starts the scene's five characters
 * (tlb_0028_1's starters, Yae among them). Each frame it counts and steps the demo
 * (st3clearCamScript) and keeps the count in st3clearFrame. */
s32 st3clearMenuProc(Obj *obj, s16 msg, void *arg) {
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "CHADEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            st3clearFrame = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0xC5);
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
            camBgradMoveTop((ColorFade *) w->sky, 255, 48, 48);
            camBgradMoveBottom((ColorFade *) w->sky, 255, 160, 160);
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
            stageSetData(0x71);
            geoSetMode(1);
            gCamChkStep = 1.0f;
            gObjMoveStep = 1.0f;
            gCamChkMarkerSize = 0.01f;
            (*st3clearGoeCreate)(obj);
            (*st3clearEbiCreate)(obj);
            (*st3clearSasCreate)(obj);
            (*st3clearYaeCreate)(obj);
            (*st3clearJijiCreate)(obj);
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
                (*st3clearCamScript)(obj);
            }
            st3clearFrame = w->timer;
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

/* The red-sky CHADEMO's camera script, one step a time (scene), on the scene's frame count; then
 * the camera check follows the camera. 0: the camera on the riders, sounds 0x81 and 0x1A0.
 * 1: sound 0x404 at frames 2 and 70; at 90 the camera lowers its aim. 2: at 1006 it swings
 * left; fade out at 1416. 3: at 1427 a cut; fade in at 1432. 4: at 3343 the scene director
 * ends and the camera cuts. 5: at 4255 the same, then 6: at 8156 the director ends. */
void st3clearCamScript(Obj *obj) {
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[3];

    switch (w->scene) {
        case 0:
            camSetEye((Camera *) w->cam, -133.0f, 19.0f, 296.0f);
            camSetAt((Camera *) w->cam, -145.0f, 85.0f, 183.0f);
            sndPlay(0x81);
            sndPlay(0x1A0);
            w->scene++;
            break;
        case 1:
            if (st3clearFrame == 2) {
                sndPlay(0x404);
            }
            if (st3clearFrame == 70) {
                sndPlay(0x404);
            }
            if (st3clearFrame == 90) {
                camSetSpeed((Camera *) w->cam, 110.0f);
                camMoveEye((Camera *) w->cam, -133.0f, 19.0f, 296.0f);
                camMoveAt((Camera *) w->cam, -145.0f, 12.0f, 183.0f);
                w->scene++;
            }
            break;
        case 2:
            if (st3clearFrame == 1006) {
                camSetSpeed((Camera *) w->cam, 80.0f);
                camMoveEye((Camera *) w->cam, -133.0f, 19.0f, 296.0f);
                camMoveAt((Camera *) w->cam, -179.0f, 12.0f, 183.0f);
            }
            if (st3clearFrame == 1416) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
                w->scene++;
            }
            break;
        case 3:
            if (st3clearFrame == 1427) {
                camSetEye((Camera *) w->cam, -204.0f, 14.0f, 269.0f);
                camSetAt((Camera *) w->cam, -174.0f, 10.0f, 213.0f);
            }
            if (st3clearFrame == 1432) {
                demoFadeStart(8.0f, 255.0f, 0.0f, 0, 0, 0);
                w->scene++;
            }
            break;
        case 4:
            if (st3clearFrame == 3343) {
                scRunEnd();
                camSetEye((Camera *) w->cam, -194.0f, 12.0f, 240.0f);
                camSetAt((Camera *) w->cam, -149.0f, 9.0f, 207.0f);
                w->scene++;
            }
            break;
        case 5:
            if (st3clearFrame == 4255) {
                scRunEnd();
                camSetEye((Camera *) w->cam, -116.0f, 31.0f, 224.0f);
                camSetAt((Camera *) w->cam, -187.0f, 6.0f, 210.0f);
                w->scene++;
            }
        case 6:
            if (st3clearFrame == 8156) {
                scRunEnd();
                w->scene++;
            }
            break;
    }
    camGetEye((Camera *) w->cam, &gCamChkEyeX, &gCamChkEyeY, &gCamChkEyeZ);
    camGetAt((Camera *) w->cam, &gCamChkAtX, &gCamChkAtY, &gCamChkAtZ);
}

extern s32 gDemoModelsDelete2;

/* The same red-sky "CHADEMO" as played in the game: no OBJMOVE models or camera check, stage
 * flag 0x6C (bit 7), and Start skips it (as tlb_0026_0's st2clearProc): from 1 it counts
 * frames, at 3 fading out with sound 10, at 40 ending the scene director, at 50 going on
 * (stageGotoSubt(3)); past frame 8256 the skip starts by itself. */
s32 st3clearProc(Obj *obj, s16 msg, void *arg) {
    static s32 skip = 1;
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "CHADEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            st3clearFrame = 0;
            gSaveData.stageFlags[0x6C] |= 0x80;
            skip = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0xC5);
            tskCreate(obj, scParentProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 45, 0, 185);
            w->layer3Word = ((u32 *) gFrmLayer3)[6];
            gObjMoveCount = 5;
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
            camBgradMoveTop((ColorFade *) w->sky, 255, 48, 48);
            camBgradMoveBottom((ColorFade *) w->sky, 255, 160, 160);
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            w->scene = 0;
            demoFadeStart(60.0f, 255.0f, 0.0f, 0, 0, 0);
            tskCreate(obj, Geography, 0, NULL);
            DPRINTF("VRAMset\n");
            vramCreate(obj);
            tskCreate(obj, Ride, 0, NULL);
            tskCreate(obj, attack_Proc, 0, NULL);
            stageSetData(0x71);
            geoSetMode(1);
            gCamChkStep = 1.0f;
            gObjMoveStep = 1.0f;
            gCamChkMarkerSize = 0.01f;
            (*st3clearGoeCreate)(obj);
            (*st3clearEbiCreate)(obj);
            (*st3clearSasCreate)(obj);
            (*st3clearYaeCreate)(obj);
            (*st3clearJijiCreate)(obj);
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
                    stageGotoSubt(3);
                    return -1;
                case 0:
                    break;
                default:
                    skip++;
                    break;
            }
            if (st3clearFrame >= 8256 && skip == 0) {
                skip = 1;
            }
            w->timer++;
            (*st3clearCamScript)(obj);
            st3clearFrame = w->timer;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Ends the running scenario and starts scenario n. */
void st3clearScenarioStart(s32 n) {
    scRunEnd();
    scRunStart(n, 0, 0);
}
