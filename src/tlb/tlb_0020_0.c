#include "tlb.h"
#include "tlb/tlb_0020.h"

#define DPRINTF

/* ascii.c's text frame (asciiNew returns it, though tlb.h declares it void). */
typedef struct AsciiFrame {
    /* 0x00 */ Frame f;
    /* 0x14 */ s16 x;
    /* 0x16 */ s16 y;
    /* 0x18 */ char *str;
} AsciiFrame;

typedef AsciiFrame *(*AsciiNewFunc)(Frame *parent, s16 x, s16 y, char *str);

/* A demo director's work (tlb_0023_0's CHADEMO layout). */
typedef struct ChaDemoW {
    /* 0x00 */ Obj *cam; /* "dracam" */
    /* 0x04 */ Frame *f04; /* made by the scene's builders */
    /* 0x08 */ CamView *view;
    /* 0x0C */ Frame *f0C;
    /* 0x10 */ Frame *f10;
    /* 0x14 */ u8 pad14[4];
    /* 0x18 */ Obj *sky; /* "bgrad" */
    /* 0x1C */ AsciiFrame *text; /* a blank line of text */
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
s32 chademoTick = 5; /* the scene's frame count */

void chademoCamScript(Obj *obj);

/* "CHADEMO" at night: a director built as OPDEMO2. It loads files 0xBC, 0x98, 0x7D and 0x78,
 * sets up the scene director, the effects, its own letterboxed fade, OBJMOVE (5 models), the
 * dragon camera (50 degrees, far plane 20000), the sun, a black sky, a blank text line, the
 * ground, VRAM, the rider and the attack task, stage data 0xD and geography mode 1, starts the
 * scene's four characters (tlb_0020_1's starters) and plays sound 0x1E. Each frame it counts and
 * steps the demo (chademoCamScript) and keeps the count in chademoTick.
 * The hold test has an empty else, as tlb_0023_0's CHADEMO (see there). */
s32 chademoMenuProc(Obj *obj, s16 msg, void *arg) {
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "CHADEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0x98);
            floadLoadSet(0x7D);
            floadLoadSet(0x78);
            tskCreate(obj, scParentProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            fadeCreate(obj);
            fadeSetLetterbox(320, 45, 0, 185);
            w->layer3Word = ((u32 *) gFrmLayer3)[6];
            gObjMoveCount = 5;
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
            camBgradSetLines((Camera *) w->sky, 76, 240);
            camBgradMoveTop((ColorFade *) w->sky, 0, 0, 0);
            camBgradMoveBottom((ColorFade *) w->sky, 0, 0, 0);
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            demoJoyChkOpen();
            w->text = ((AsciiNewFunc) asciiNew)(gFrmTextRoot, 18, 177, "                    ");
            w->scene = 0;
            fadeCreate(obj);
            demoFadeStart(60.0f, 255.0f, 0.0f, 0, 0, 0);
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
            (*chademoGoeCreate)(obj);
            (*chademoEbiCreate)(obj);
            (*chademoSasCreate)(obj);
            (*chademoBabaMusumeCreate)(obj);
            camSunSetClockStop(1);
            sndPlay(0x1E);
            break;
        case 1:
            frmDestroy(w->f0C);
            frmDestroy(&w->text->f);
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
                (*chademoCamScript)(obj);
            } else {
                /* empty in the game too: its b over this arm is in the code */
            }
            chademoTick = w->timer;
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

/* This CHADEMO's camera script, by step (scene) and the director's own frame count; then the
 * camera check follows the camera, the chimney smokes (chademoChimneySmoke) and the scenario
 * runs (chademoScenarioStep). 0: one view, fade out at 505, at 535 fade in on the next.
 * 1, 2, 4: the same, at 1798, 3248 and 4548. 3: at 3808 the camera glides back. 5: at 5388
 * sound 7 and the last fade out. */
void chademoCamScript(Obj *obj) {
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad;

    switch (w->scene) {
        case 0:
            camSetEye((Camera *) w->cam, -28.0f, 77.0f, -6.0f);
            camSetAt((Camera *) w->cam, 24.0f, 16.0f, 48.0f);
            if (w->timer == 505) {
                demoFadeStart(30.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (w->timer == 535) {
                demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
                w->scene = 1;
                camSetEye((Camera *) w->cam, 25.0f, 9.0f, 30.0f);
                camSetAt((Camera *) w->cam, -19.0f, 38.0f, -99.0f);
            }
            break;
        case 1:
            if (w->timer == 1788) {
                demoFadeStart(10.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (w->timer == 1798) {
                demoFadeStart(10.0f, 255.0f, 0.0f, 0, 0, 0);
                w->scene = 2;
                camSetEye((Camera *) w->cam, -20.0f, 38.0f, -7.0f);
                camSetAt((Camera *) w->cam, 90.0f, -42.0f, 49.0f);
            }
            break;
        case 2:
            if (w->timer == 3245) {
                demoFadeStart(3.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (w->timer == 3248) {
                demoFadeStart(3.0f, 255.0f, 0.0f, 0, 0, 0);
                w->scene = 3;
                camSetEye((Camera *) w->cam, 25.0f, 9.0f, 30.0f);
                camSetAt((Camera *) w->cam, 7.0f, 38.0f, -99.0f);
            }
            break;
        case 3:
            if (w->timer == 3808) {
                w->scene = 4;
                camSetSpeed((Camera *) w->cam, 220.0f);
                camMoveEye((Camera *) w->cam, 25.0f, 9.0f, 30.0f);
                camMoveAt((Camera *) w->cam, -19.0f, 38.0f, -99.0f);
            }
            break;
        case 4:
            if (w->timer == 4545) {
                demoFadeStart(3.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (w->timer == 4548) {
                demoFadeStart(3.0f, 255.0f, 0.0f, 0, 0, 0);
                w->scene = 5;
                camSetEye((Camera *) w->cam, -20.0f, 38.0f, -7.0f);
                camSetAt((Camera *) w->cam, 90.0f, -42.0f, 49.0f);
            }
            break;
        case 5:
            if (w->timer == 5388) {
                sndPlay(7);
                demoFadeStart(60.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            break;
        case 6:
            break;
    }
    camGetEye((Camera *) w->cam, &gCamChkEyeX, &gCamChkEyeY, &gCamChkEyeZ);
    camGetAt((Camera *) w->cam, &gCamChkAtX, &gCamChkAtY, &gCamChkAtZ);
    (*chademoChimneySmoke)();
    (*chademoScenarioStep)();
}

/* The release "CHADEMO": the night demo as the game plays it. It sets up as the debug one
 * (chademoMenuProc) does, without the text line, marks stage 0x65's flag 0x80 and makes
 * at least three characters available. Start skips it, as GDEMO's skip does (tlb_0019_0), and
 * it ends by itself at frame 5388. The skip count runs its end: a fade to black with sound 7 at
 * 3, the models go and the scene ends at 40, the tunes clear at 46, and at 50 it waits for the
 * screen and goes to stage 0x9A. */
extern s32 gDemoModelsDelete2;
extern s32 gObjMoveSel; /* OBJMOVE: the model chosen, -1 for none */

s32 chademoProc(Obj *obj, s16 msg, void *arg) {
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
            floadLoadSet(0x98);
            floadLoadSet(0x7D);
            floadLoadSet(0x78);
            tskCreate(obj, scParentProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            fadeCreate(obj);
            w->layer3Word = ((u32 *) gFrmLayer3)[6];
            gObjMoveSel = -1;
            gObjMoveCount = 5;
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
            camBgradSetLines((Camera *) w->sky, 76, 240);
            camBgradMoveTop((ColorFade *) w->sky, 0, 0, 0);
            camBgradMoveBottom((ColorFade *) w->sky, 0, 0, 0);
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            demoJoyChkOpen();
            w->scene = 0;
            fadeCreate(obj);
            demoFadeStart(60.0f, 255.0f, 0.0f, 0, 0, 0);
            fadeSetLetterbox(320, 45, 0, 185);
            tskCreate(obj, Geography, 0, NULL);
            DPRINTF("VRAMset\n");
            vramCreate(obj);
            tskCreate(obj, Ride, 0, NULL);
            tskCreate(obj, attack_Proc, 0, NULL);
            stageSetData(0xD);
            geoSetMode(1);
            gSaveData.stageFlags[0x65] |= 0x80;
            if (gSaveData.charaCount < 3) {
                gSaveData.charaCount = 3;
            }
            gCamChkStep = 1.0f;
            gObjMoveStep = 1.0f;
            gCamChkMarkerSize = 0.01f;
            (*chademoGoeCreate)(obj);
            (*chademoEbiCreate)(obj);
            (*chademoSasCreate)(obj);
            (*chademoBabaMusumeCreate)(obj);
            camSunSetClockStop(1);
            sndPlay(0x1E);
            skip = 0;
            break;
        case 1:
            frmDestroy(&w->text->f);
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
                    sndPlay(7);
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
                    dmenuGoStage9A();
                    return -1;
                case 0:
                    break;
                default:
                    skip++;
                    break;
            }
            w->timer++;
            (*chademoCamScript)(obj);
            chademoTick = w->timer;
            if (chademoTick == 5388 && skip == 0) {
                skip = 1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
