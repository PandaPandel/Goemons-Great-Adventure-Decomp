#include "tlb.h"
#include "tlb/tlb_0025.h"

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

extern s32 yaeDemoFrame; /* the scene's frame count (tlb_0025's .bss) */
extern f32 gCamChkEyeX;
extern f32 gCamChkEyeY;
extern f32 gCamChkEyeZ;
extern f32 gCamChkAtX;
extern f32 gCamChkAtY;
extern f32 gCamChkAtZ;

extern Frame *gFrmLayer3;
extern u8 gDemoCamHold;
extern s32 gDemoModelsDelete;
extern s32 gDemoModelsDelete2;
extern s32 gObjMoveCount;
extern u8 gCamChkActive;

/* ascii.c's asciiNew, which returns its text frame (tlb.h declares it void) */
typedef Frame *(*AsciiNewFunc)(Frame *parent, s16 x, s16 y, char *str);

void yaeDemoCamScript(Obj *obj);
/* This file's .data is yaeDemoProc's skip, then IDO's padding to 16 (0x08005750-0x0800575F);
 * tlb_0025_1's starts at 0x08005760. */

/* "CHADEMO" on stage 0xD (the debug-menu build): file 0xBC, the scene director, the effects,
 * OBJMOVE (5 models), the dragon camera (50 degrees, planes 14 and 2000), the sun, a black sky,
 * the joypad check, a blank text line, a letterboxed fade, the ground, VRAM, the rider and the
 * attack task, stage data 0xD, and the scene's four builders. Each frame, unless the camera is
 * held, it counts and steps the demo (yaeDemoCamScript); C-left on pad 2 holds the
 * camera, and Z opens the camera check after frame 10.
 * The hold test has an empty else, as tlb_0023_0's CHADEMO (see there). */
s32 yaeDemoMenuProc(Obj *obj, s16 msg, void *arg) {
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "CHADEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            floadLoadSet(0xBC);
            tskCreate(obj, scParentProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            fadeCreate(gEffectTask);
            w->layer3Word = ((u32 *) gFrmLayer3)[6];
            gObjMoveCount = 5;
            demoObjMoveOpen();
            w->cam = tskCreate(obj, dracam, 0, NULL);
            DPRINTF("カメラを作りました\n");
            camSetFov((Camera *) w->cam, 0.87266463f);
            w->view = camGetView((Camera *) w->cam);
            w->view->zFar = 2000.0f;
            w->view->zNear = 14.0f;
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
            w->f1C = ((AsciiNewFunc) asciiNew)(gFrmTextRoot, 18, 177, "                    ");
            w->scene = 0;
            demoFadeStart(60.0f, 255.0f, 0.0f, 0, 0, 0);
            fadeSetLetterbox(320, 45, 0, 185);
            tskCreate(obj, Geography, 0, NULL);
            DPRINTF("VRAMset\n");
            vramCreate(obj);
            tskCreate(obj, Ride, 0, NULL);
            tskCreate(obj, attack_Proc, 0, NULL);
            stageSetData(0xD);
            geoSetMode(1);
            gCamChkStep = 1.0f;
            gObjMoveStep = 1.0f;
            gCamChkMarkerSize = 0.01f;
            (*yaeDemoGoeCreate)(obj);
            (*yaeDemoEbiCreate)(obj);
            (*yaeDemoSasCreate)(obj);
            (*yaeDemoYaeCreate)(obj);
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
                (*yaeDemoCamScript)(obj);
            } else {
                /* empty in the game too: its b over this arm is in the code */
            }
            yaeDemoFrame = w->timer;
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
 * camera check follows the camera and the smoke goes up (yaeDemoChimneySmoke). 0: the
 * camera on the riders, sound 0x21. 1-5: a fade out and, ten frames later (30 at 550/580), a
 * fade in on the next of three views, at 580, 1150, 1840, 3680 and 4410. 6: fade out at 4890,
 * sound 0x562 at 5010, at 5130 fade in on a high view. 7: at 6820 fade out, sound 10, and the
 * scene director ends. */
void yaeDemoCamScript(Obj *obj) {
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[3];

    switch (w->scene) {
        case 0:
            camSetEye((Camera *) w->cam, -28.0f, 32.0f, -38.0f);
            camSetAt((Camera *) w->cam, 17.0f, 1.0f, 5.0f);
            sndPlay(0x21);
            w->scene++;
            break;
        case 1:
            if (yaeDemoFrame == 550) {
                demoFadeStart(30.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (yaeDemoFrame == 580) {
                demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 9.0f, 14.0f, 3.0f);
                camSetAt((Camera *) w->cam, -4.0f, 12.0f, -21.0f);
                w->scene++;
            }
            break;
        case 2:
            if (yaeDemoFrame == 1140) {
                demoFadeStart(10.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (yaeDemoFrame == 1150) {
                demoFadeStart(10.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 6.0f, 17.0f, -5.0f);
                camSetAt((Camera *) w->cam, 17.0f, 13.0f, 17.0f);
                w->scene++;
            }
            break;
        case 3:
            if (yaeDemoFrame == 1830) {
                demoFadeStart(10.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (yaeDemoFrame == 1840) {
                demoFadeStart(10.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -28.0f, 32.0f, -38.0f);
                camSetAt((Camera *) w->cam, 17.0f, 1.0f, 5.0f);
                w->scene++;
            }
            break;
        case 4:
            if (yaeDemoFrame == 3670) {
                demoFadeStart(10.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (yaeDemoFrame == 3680) {
                demoFadeStart(10.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 9.0f, 14.0f, 3.0f);
                camSetAt((Camera *) w->cam, -4.0f, 12.0f, -21.0f);
                w->scene++;
            }
            break;
        case 5:
            if (yaeDemoFrame == 4400) {
                demoFadeStart(10.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (yaeDemoFrame == 4410) {
                demoFadeStart(10.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 6.0f, 17.0f, -5.0f);
                camSetAt((Camera *) w->cam, 17.0f, 13.0f, 17.0f);
                w->scene++;
            }
            break;
        case 6:
            if (yaeDemoFrame == 4890) {
                demoFadeStart(60.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (yaeDemoFrame == 5010) {
                sndPlay(0x562);
            }
            if (yaeDemoFrame == 5130) {
                demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -59.0f, 58.0f, -42.0f);
                camSetAt((Camera *) w->cam, 17.0f, 1.0f, 10.0f);
                w->scene++;
            }
            break;
        case 7:
            if (yaeDemoFrame == 6820) {
                demoFadeStart(60.0f, 0.0f, 255.0f, 0, 0, 0);
                sndPlay(10);
                scRunEnd();
            }
            break;
    }
    camGetEye((Camera *) w->cam, &gCamChkEyeX, &gCamChkEyeY, &gCamChkEyeZ);
    camGetAt((Camera *) w->cam, &gCamChkAtX, &gCamChkAtY, &gCamChkAtZ);
    (*yaeDemoChimneySmoke)();
}

/* This CHADEMO as played in the game: stage flag 0x68 (bit 7), three characters, event 0xCB,
 * and Start skips it (as tlb_0028_0's st3clearProc): from 1 it counts frames, at 3 fading out
 * with sound 10, at 40 ending the scene director, at 46 clearing the sun's tunes, at 50 going
 * to stage 0xB6 (dmenuGoStageB6); at frame 6590 the skip starts by itself.
 * Matched 2026-10-12; it was parked for the promoted skip's register, as tlb_0022_0's
 * jijiDemoProc. */
s32 yaeDemoProc(Obj *obj, s16 msg, void *arg) {
    static s32 skip = 1;
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "CHADEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            skip = 0;
            floadLoadSet(0xBC);
            tskCreate(obj, scParentProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            fadeCreate(gEffectTask);
            w->layer3Word = ((u32 *) gFrmLayer3)[6];
            gObjMoveCount = 5;
            w->cam = tskCreate(obj, dracam, 0, NULL);
            DPRINTF("カメラを作りました\n");
            camSetFov((Camera *) w->cam, 0.87266463f);
            w->view = camGetView((Camera *) w->cam);
            w->view->zFar = 2000.0f;
            w->view->zNear = 14.0f;
            camSunCreate(obj);
            camSunSetView(w->view);
            gSaveData.stageFlags[0x68] |= 0x80;
            gSaveData.charaCount = 4;
            swWorkSet(0xCB);
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
            fadeSetLetterbox(320, 45, 0, 185);
            tskCreate(obj, Geography, 0, NULL);
            DPRINTF("VRAMset\n");
            vramCreate(obj);
            tskCreate(obj, Ride, 0, NULL);
            tskCreate(obj, attack_Proc, 0, NULL);
            stageSetData(0xD);
            geoSetMode(1);
            gCamChkStep = 1.0f;
            gObjMoveStep = 1.0f;
            gCamChkMarkerSize = 0.01f;
            (*yaeDemoGoeCreate)(obj);
            (*yaeDemoEbiCreate)(obj);
            (*yaeDemoSasCreate)(obj);
            (*yaeDemoYaeCreate)(obj);
            camSunSetClockStop(1);
            break;
        case 1:
            frmDestroy(w->f1C);
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
                case 46:
                    camSunClearTunes();
                    skip++;
                    break;
                case 50:
                    while (gfxGetPendingSwaps()) {
                    }
                    dmenuGoStageB6();
                    return -1;
                case 0:
                    break;
                default:
                    skip++;
                    break;
            }
            w->timer++;
            (*yaeDemoCamScript)(obj);
            yaeDemoFrame = w->timer;
            if (yaeDemoFrame == 6590 && skip == 0) {
                skip = 1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
