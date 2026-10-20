#include "tlb.h"
#include "tlb/tlb_0022.h"

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
    /* 0x04 */ Frame *f04;
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
/* This file's .data is jijiDemoFrame, jijiDemoCamScript's angle and jijiDemoProc's skip, then
 * IDO's padding to 16 (0x08005A50-0x08005A5F); tlb_0022_1's starts at 0x08005A60. */
s32 jijiDemoFrame = 1; /* the scene's frame count */

void jijiDemoCamScript(Obj *obj);

/* "CHADEMO" on stage 0xD at night: a director built as OPDEMO2. It loads files 0xBC and 0xC5,
 * sets up the scene director, the effects, a letterboxed fade, OBJMOVE (5 models), the dragon
 * camera (50 degrees, planes 10 and 2000), the sun, a black sky, a blank text line, the ground,
 * VRAM, the rider and the attack task, stage data 0xD and geography mode 1, and starts the
 * scene's five characters (tlb_0022_1's starters, Sasuke and Yae among them). Each frame it
 * counts and steps the demo (jijiDemoCamScript) and keeps the count in jijiDemoFrame.
 * The hold test has an empty else: the game jumps over it (b at the step's end, bnez with a
 * nop), where without it IDO takes a branch-likely into the join. */
s32 jijiDemoMenuProc(Obj *obj, s16 msg, void *arg) {
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "CHADEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            jijiDemoFrame = 0;
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
            camBgradMoveTop((ColorFade *) w->sky, 0, 0, 0);
            camBgradMoveBottom((ColorFade *) w->sky, 0, 0, 0);
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            demoJoyChkOpen();
            w->text = ((AsciiNewFunc) asciiNew)(gFrmTextRoot, 18, 177, "                    ");
            w->scene = 0;
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
            (*jijiDemoGoeCreate)(obj);
            (*jijiDemoEbiCreate)(obj);
            (*jijiDemoSasCreate)(obj);
            (*jijiDemoYaeCreate)(obj);
            (*jijiDemoJijiCreate)(obj);
            camSunSetClockStop(1);
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
            if (!gDemoCamHold) {
                w->timer++;
                (*jijiDemoCamScript)(obj);
            } else {
                /* empty in the game too: its b over this arm is in the code */
            }
            jijiDemoFrame = w->timer;
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

#define sinf (*sinf)
#define cosf (*cosf)

/* The night CHADEMO's camera script, one step a time (scene), on the scene's frame count; then
 * the camera check follows the camera and the chimney smokes (jijiDemoChimneySmoke).
 * 0: the camera on the riders, sound 0x80. 1: at frame 139 it pulls back. 2: at 469 a cut.
 * 3: at 2779 the aim moves and a swing starts; 4: the eye circles (50 around -23,-15, a degree
 * every 14 frames) until 3939, a cut. 5: at 4239 a cut. 6: at 7560 fade out, sound 10, and the
 * scene director ends. */
void jijiDemoCamScript(Obj *obj) {
    static f32 angle = 0.0f;
    f32 x;
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad;

    switch (w->scene) {
        case 0:
            camSetEye((Camera *) w->cam, -10.0f, 19.0f, -9.0f);
            camSetAt((Camera *) w->cam, -43.0f, 13.0f, -24.0f);
            sndPlay(0x80);
            w->scene++;
            break;
        case 1:
            if (jijiDemoFrame == 139) {
                camSetSpeed((Camera *) w->cam, 160.0f);
                camMoveEye((Camera *) w->cam, 21.0f, 17.0f, 4.0f);
                camMoveAt((Camera *) w->cam, -44.0f, 14.0f, -24.0f);
                w->scene++;
            }
            break;
        case 2:
            if (jijiDemoFrame == 469) {
                camSetEye((Camera *) w->cam, -56.0f, 30.0f, -27.0f);
                camSetAt((Camera *) w->cam, 32.0f, -3.0f, 5.0f);
                w->scene++;
            }
            break;
        case 3:
            if (jijiDemoFrame == 2779) {
                w->scene++;
                angle = 30.0f;
                camSetAt((Camera *) w->cam, -23.0f, 10.0f, -15.0f);
            }
            break;
        case 4:
            angle += 1.0f;
            x = sinf(angle / 14.0f * 0.0174532930038800785) * 50.0f + -23.0f;
            camSetEye((Camera *) w->cam, x, 14.0f, cosf(angle / 14.0f * 0.0174532930038800785) * 50.0f + -15.0f);
            if (jijiDemoFrame == 3939) {
                camSetEye((Camera *) w->cam, -18.0f, 16.0f, -34.0f);
                camSetAt((Camera *) w->cam, -6.0f, 13.0f, -15.0f);
                w->scene++;
            }
            break;
        case 5:
            if (jijiDemoFrame == 4239) {
                camSetEye((Camera *) w->cam, -47.0f, 20.0f, -6.0f);
                camSetAt((Camera *) w->cam, -8.0f, 11.0f, -6.0f);
                w->scene++;
            }
            break;
        case 6:
            if (jijiDemoFrame == 7560) {
                demoFadeStart(60.0f, 0.0f, 255.0f, 0, 0, 0);
                sndPlay(10);
                scRunEnd();
            }
            break;
        case 7:
            break;
    }
    camGetEye((Camera *) w->cam, &gCamChkEyeX, &gCamChkEyeY, &gCamChkEyeZ);
    camGetAt((Camera *) w->cam, &gCamChkAtX, &gCamChkAtY, &gCamChkAtZ);
    (*jijiDemoChimneySmoke)();
}

extern s32 gDemoModelsDelete2;

/* The same night "CHADEMO" as played in the game: no OBJMOVE models, text line or camera check,
 * stage flag 0x6A (bit 7), and Start skips it (as tlb_0028_0's st3clearProc): from 1 it counts
 * frames, at 3 fading out with sound 10, at 40 ending the scene director, at 46 clearing the
 * sun's tunes, at 50 going to stage 0xC9 (dmenuGoStageC9); at frame 7320 the skip starts by
 * itself.
 * Matched 2026-10-12; it was parked for the promoted skip's register (v0 here, a2 in the game). */
s32 jijiDemoProc(Obj *obj, s16 msg, void *arg) {
    static s32 skip = 1;
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "CHADEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            jijiDemoFrame = 0;
            skip = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0xC5);
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
            gSaveData.stageFlags[0x6A] |= 0x80;
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            w->scene = 0;
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
            (*jijiDemoGoeCreate)(obj);
            (*jijiDemoEbiCreate)(obj);
            (*jijiDemoSasCreate)(obj);
            (*jijiDemoYaeCreate)(obj);
            (*jijiDemoJijiCreate)(obj);
            camSunSetClockStop(1);
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
            w->timer++;
            (*jijiDemoCamScript)(obj);
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
                    dmenuGoStageC9();
                    return -1;
                case 0:
                    break;
                default:
                    skip++;
                    break;
            }
            jijiDemoFrame = w->timer;
            if (jijiDemoFrame == 7320 && skip == 0) {
                skip = 1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
