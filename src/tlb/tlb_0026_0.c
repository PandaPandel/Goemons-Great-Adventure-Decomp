#include "tlb.h"
#include "tlb/tlb_0026.h"
/* This file's .data is st2clearUnused1 and st2clearProc's skip, then IDO's padding to 16
 * (0x08005070-0x0800507F); tlb_0026_1's starts at 0x08005080. */
u32 st2clearUnused1 = 0x00000000;

#define DPRINTF

/* A demo director's work (tlb_0023_0's CHADEMO layout). */
typedef struct ClearDemoW {
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
} ClearDemoW;

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
extern s32 st2clearFrame; /* the scene's frame count (this segment's .bss) */

void st2clearCamScript(Obj *obj);

/* "ST2CLEARDEMO": the demo after stage 2 is cleared, a director built as OPDEMO2. It loads files
 * 0xBC, 0xCE and 0xCD, sets up the scene director, the effects, a letterboxed fade, OBJMOVE (5
 * models), the dragon camera (50 degrees, planes 20 and 2000), the sun, a deep sky, the ground,
 * VRAM, the rider and the attack task, stage data 0x70 and geography mode 1, and starts the
 * scene's five characters (tlb_0026_1's starters). Each frame it counts and steps the demo
 * (st2clearCamScript) and keeps the count in st2clearFrame. */
s32 st2clearMenuProc(Obj *obj, s16 msg, void *arg) {
    ClearDemoW *w = (ClearDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "ST2CLEARDEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0xCE);
            floadLoadSet(0xCD);
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
            w->view->zNear = 20.0f;
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
            camBgradMoveTop((ColorFade *) w->sky, 48, 48, 255);
            camBgradMoveBottom((ColorFade *) w->sky, 128, 128, 255);
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
            stageSetData(0x70);
            geoSetMode(1);
            gCamChkStep = 1.0f;
            gObjMoveStep = 1.0f;
            gCamChkMarkerSize = 0.01f;
            (*st2clearGoeCreate)(obj);
            (*st2clearEbiCreate)(obj);
            (*st2clearSasCreate)(obj);
            (*st2clearYaeCreate)(obj);
            (*st2clearSuzakuCreate)(obj);
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
                (*st2clearCamScript)(obj);
            }
            st2clearFrame = w->timer;
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

/* The clear demo's camera script, one step a time (scene), on the scene's frame count; then the
 * camera check follows the camera. 0: the camera high over the scene, sounds 0x2AA, 0x28B and
 * 0x20. 1: it sinks. 2: at 2529 the aim rises. 3: at 2630 both climb. 4-7: a fade out and, ten
 * frames later, a fade in on the next of three views, at 3330, 3980, 4830 and 6450. 8: at 8350
 * the scene director ends. */
void st2clearCamScript(Obj *obj) {
    ClearDemoW *w = (ClearDemoW *) (obj + 1);
    s32 pad[3];

    switch (w->scene) {
        case 0:
            camSetEye((Camera *) w->cam, -795.0f, 138.0f, 900.0f);
            camSetAt((Camera *) w->cam, -686.0f, 13.0f, 805.0f);
            w->scene++;
            sndPlay(0x2AA);
            sndPlay(0x28B);
            sndPlay(0x20);
            break;
        case 1:
            camSetSpeed((Camera *) w->cam, 300.0f);
            camMoveEye((Camera *) w->cam, -690.0f, 7.0f, 863.0f);
            w->scene++;
            break;
        case 2:
            if (st2clearFrame == 2529) {
                camSetSpeed((Camera *) w->cam, 20.0f);
                camMoveAt((Camera *) w->cam, -686.0f, 42.0f, 805.0f);
                w->scene++;
            }
            break;
        case 3:
            if (st2clearFrame == 2630) {
                camSetSpeed((Camera *) w->cam, 50.0f);
                camMoveEye((Camera *) w->cam, -690.0f, 107.0f, 741.0f);
                camMoveAt((Camera *) w->cam, -686.0f, 112.0f, 714.0f);
                w->scene++;
            }
            break;
        case 4:
            if (st2clearFrame == 3320) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (st2clearFrame == 3330) {
                demoFadeStart(8.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -682.0f, 63.0f, 775.0f);
                camSetAt((Camera *) w->cam, -682.0f, 44.0f, 790.0f);
                w->scene++;
            }
            break;
        case 5:
            if (st2clearFrame == 3970) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (st2clearFrame == 3980) {
                demoFadeStart(8.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -690.0f, 107.0f, 741.0f);
                camSetAt((Camera *) w->cam, -686.0f, 112.0f, 714.0f);
                w->scene++;
            }
            break;
        case 6:
            if (st2clearFrame == 4820) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (st2clearFrame == 4830) {
                demoFadeStart(8.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -682.0f, 63.0f, 775.0f);
                camSetAt((Camera *) w->cam, -682.0f, 44.0f, 790.0f);
                w->scene++;
            }
            break;
        case 7:
            if (st2clearFrame == 6440) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (st2clearFrame == 6450) {
                demoFadeStart(8.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -690.0f, 7.0f, 863.0f);
                camSetAt((Camera *) w->cam, -686.0f, 13.0f, 805.0f);
                w->scene++;
            }
            break;
        case 8:
            if (st2clearFrame == 8350) {
                scRunEnd();
            }
            break;
    }
    camGetEye((Camera *) w->cam, &gCamChkEyeX, &gCamChkEyeY, &gCamChkEyeZ);
    camGetAt((Camera *) w->cam, &gCamChkAtX, &gCamChkAtY, &gCamChkAtZ);
}

extern s32 gDemoModelsDelete2;

/* "ST2CLEARDEMO" as played after the stage: st2clearMenuProc's scene without OBJMOVE or the camera
 * check, marking stage flag 0x69 (bit 7). Start skips it (its static skip, 0x08005074, as OPMAIN's skip): from 1
 * it counts frames, at 3 fading out with sound 10, at 40 ending the scene director, at 50 going
 * on (stageGotoSubt(2)); past frame 8390 the skip starts by itself. */
s32 st2clearProc(Obj *obj, s16 msg, void *arg) {
    static s32 skip = 1;
    ClearDemoW *w = (ClearDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "ST2CLEARDEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            gSaveData.stageFlags[0x69] |= 0x80;
            skip = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0xCE);
            floadLoadSet(0xCD);
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
            w->view->zNear = 20.0f;
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
            camBgradMoveTop((ColorFade *) w->sky, 48, 48, 255);
            camBgradMoveBottom((ColorFade *) w->sky, 128, 128, 255);
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            w->scene = 0;
            demoFadeStart(60.0f, 255.0f, 0.0f, 0, 0, 0);
            tskCreate(obj, Geography, 0, NULL);
            DPRINTF("VRAMset\n");
            vramCreate(obj);
            tskCreate(obj, Ride, 0, NULL);
            tskCreate(obj, attack_Proc, 0, NULL);
            stageSetData(0x70);
            geoSetMode(1);
            gCamChkStep = 1.0f;
            gObjMoveStep = 1.0f;
            gCamChkMarkerSize = 0.01f;
            (*st2clearGoeCreate)(obj);
            (*st2clearEbiCreate)(obj);
            (*st2clearSasCreate)(obj);
            (*st2clearYaeCreate)(obj);
            (*st2clearSuzakuCreate)(obj);
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
                    stageGotoSubt(2);
                    return -1;
                case 0:
                    if (st2clearFrame >= 8390) {
                        skip = 1;
                    }
                    break;
                default:
                    skip++;
                    break;
            }
            w->timer++;
            (*st2clearCamScript)(obj);
            st2clearFrame = w->timer;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
