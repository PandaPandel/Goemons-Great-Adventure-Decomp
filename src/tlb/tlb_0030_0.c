#include "tlb.h"
#include "tlb/tlb_0030.h"
/* .data: st4clearProc's static skip count (0x08005520), a word nothing reads, st4clearCamScript's
 * static frame count (0x08005528), then IDO's padding to 16 (0x0800552C); tlb_0030_1's starts at
 * 0x08005530. */

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
extern s32 gDemoModelsDelete2;
extern s32 gObjMoveCount;
extern u8 gCamChkActive;
extern f32 gCamChkEyeX;
extern f32 gCamChkEyeY;
extern f32 gCamChkEyeZ;
extern f32 gCamChkAtX;
extern f32 gCamChkAtY;
extern f32 gCamChkAtZ;
extern s32 st4clearFrameCount; /* the scene's frame count (past the file's end, undefined_syms.txt) */

void st4clearCamScript(Obj *obj);

/* "CHADEMO" in the evening: a director built as tlb_0026_0's ST2CLEARDEMO (the demo menu's
 * version: no OBJMOVE, no hold). It loads files 0xBC, 0xCD and 0xC4, sets up the scene
 * director, the effects, a letterboxed fade, the dragon camera (50 degrees, planes 10 and
 * 2000), the sun, an evening sky, the ground, VRAM, the rider and the attack task, stage data
 * 0x72 and geography mode 1, and starts the scene's six characters. Each frame it counts and
 * steps the demo (st4clearCamScript) and keeps the count in st4clearFrameCount. */
s32 st4clearMenuProc(Obj *obj, s16 msg, void *arg) {
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "CHADEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0xCD);
            floadLoadSet(0xC4);
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
            w->sky = tskCreate(obj, bgrad, 1, NULL);
            camBgradSetLines((Camera *) w->sky, 45, 185);
            camBgradMoveTop((ColorFade *) w->sky, 176, 96, 64);
            camBgradMoveBottom((ColorFade *) w->sky, 136, 96, 60);
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            w->scene = 0;
            demoFadeStart(60.0f, 255.0f, 0.0f, 0, 0, 0);
            tskCreate(obj, Geography, 0, NULL);
            DPRINTF("VRAMset\n");
            vramCreate(obj);
            tskCreate(obj, Ride, 0, NULL);
            tskCreate(obj, attack_Proc, 0, NULL);
            stageSetData(0x72);
            geoSetMode(1);
            DPRINTF("地形を出します\n");
            gCamChkStep = 1.0f;
            gObjMoveStep = 1.0f;
            gCamChkMarkerSize = 0.01f;
            (*st4clearGoeCreate)(obj);
            (*st4clearEbiCreate)(obj);
            (*st4clearSasCreate)(obj);
            (*st4clearYaeCreate)(obj);
            (*st4clearSuzakuCreate)(obj);
            (*st4clearHyotanCreate)(obj);
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
            (*st4clearCamScript)(obj);
            st4clearFrameCount = w->timer;
            if (joyGetHeldTime(0, 0x1000) == 1 && !gCamChkActive && w->timer >= 11) {
                demoCamChkOpen(50.0f);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The same "CHADEMO" as played in the game: no camera check, stage flag 0x6E (bit 7), and Start
 * skips it (as tlb_0026_0's st2clearProc): from 1 it counts frames, at 3 fading out with sound
 * 10, at 40 ending the scene director, at 50 going on (stageGotoSubt(4)); past frame 12690 the
 * skip starts by itself. */
s32 st4clearProc(Obj *obj, s16 msg, void *arg) {
    static s32 skip = 1;
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "CHADEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            gSaveData.stageFlags[0x6E] |= 0x80;
            skip = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0xCD);
            floadLoadSet(0xC4);
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
            w->sky = tskCreate(obj, bgrad, 1, NULL);
            camBgradSetLines((Camera *) w->sky, 45, 185);
            camBgradMoveTop((ColorFade *) w->sky, 176, 96, 64);
            camBgradMoveBottom((ColorFade *) w->sky, 136, 96, 60);
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            w->scene = 0;
            demoFadeStart(60.0f, 255.0f, 0.0f, 0, 0, 0);
            tskCreate(obj, Geography, 0, NULL);
            DPRINTF("VRAMset\n");
            vramCreate(obj);
            tskCreate(obj, Ride, 0, NULL);
            tskCreate(obj, attack_Proc, 0, NULL);
            stageSetData(0x72);
            geoSetMode(1);
            DPRINTF("地形を出します\n");
            gCamChkStep = 1.0f;
            gObjMoveStep = 1.0f;
            gCamChkMarkerSize = 0.01f;
            (*st4clearGoeCreate)(obj);
            (*st4clearEbiCreate)(obj);
            (*st4clearSasCreate)(obj);
            (*st4clearYaeCreate)(obj);
            (*st4clearSuzakuCreate)(obj);
            (*st4clearHyotanCreate)(obj);
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
                    demoFadeForce(16.0f, 0.0f, 255.0f, 0, 0, 0);
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
                    stageGotoSubt(4);
                    return -1;
                case 0:
                    break;
                default:
                    skip++;
                    break;
            }
            if (st4clearFrameCount >= 12690 && skip == 0) {
                skip = 1;
            }
            w->timer++;
            (*st4clearCamScript)(obj);
            st4clearFrameCount = w->timer;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

u32 st4clearUnused1 = 0; /* nothing reads it */

/* The evening demo's camera script, one step a time (scene), on its own frame count; then the
 * camera check follows the camera. 0: over the scene, sound 0x23 at the start; at 70 the aim
 * drops. 1: a fade out at 450 and, at 480, in on a view across the yard. 2, 3: at 1610 and
 * 1910 the camera swings round. 4, 5, 7, 8: cuts at 2670, 5970, 10170 and 10420. 6: at 9770 a
 * swing. 9: at 12620, done. */
void st4clearCamScript(Obj *obj) {
    static s32 frame = 0;
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[3];

    frame++;
    switch (w->scene) {
        case 0:
            camSetEye((Camera *) w->cam, -55.0f, 115.0f, 55.0f);
            camSetAt((Camera *) w->cam, 0.0f, 240.0f, 0.0f);
            if (frame == 1) {
                sndPlay(0x23);
            }
            if (frame >= 70) {
                camSetSpeed((Camera *) w->cam, 30.0f);
                camMoveAt((Camera *) w->cam, 0.0f, 110.0f, 0.0f);
                w->scene++;
            }
            break;
        case 1:
            if (frame == 450) {
                fadeSetAll(30.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (frame >= 480) {
                camSetEye((Camera *) w->cam, 23.0f, 118.0f, 20.0f);
                camSetAt((Camera *) w->cam, -72.0f, 104.0f, -2.0f);
                fadeSetAll(30.0f, 255.0f, 0.0f, 0, 0, 0);
                w->scene++;
            }
            break;
        case 2:
            if (frame >= 1610) {
                camSetSpeed((Camera *) w->cam, 120.0f);
                camMoveEye((Camera *) w->cam, -16.0f, 114.0f, 16.0f);
                camMoveAt((Camera *) w->cam, 24.0f, 104.0f, 130.0f);
                w->scene++;
            }
            break;
        case 3:
            if (frame >= 1910) {
                camSetSpeed((Camera *) w->cam, 60.0f);
                camMoveEye((Camera *) w->cam, 4.0f, 114.0f, 77.0f);
                camMoveAt((Camera *) w->cam, 24.0f, 116.0f, 130.0f);
                w->scene++;
            }
            break;
        case 4:
            if (frame >= 2670) {
                camSetEye((Camera *) w->cam, -16.0f, 121.0f, -34.0f);
                camSetAt((Camera *) w->cam, -9.0f, 96.0f, 64.0f);
                w->scene++;
            }
            break;
        case 5:
            if (frame >= 5970) {
                camSetEye((Camera *) w->cam, 5.0f, 112.0f, -23.0f);
                camSetAt((Camera *) w->cam, 11.0f, 112.0f, 23.0f);
                w->scene++;
            }
            break;
        case 6:
            if (frame >= 9770) {
                camSetSpeed((Camera *) w->cam, 60.0f);
                camMoveEye((Camera *) w->cam, 13.0f, 114.0f, 0.0f);
                camMoveAt((Camera *) w->cam, 22.0f, 112.0f, 33.0f);
                w->scene++;
            }
            break;
        case 7:
            if (frame >= 10170) {
                camSetEye((Camera *) w->cam, -21.0f, 113.0f, -20.0f);
                camSetAt((Camera *) w->cam, 20.0f, 112.0f, 16.0f);
                w->scene++;
            }
            break;
        case 8:
            if (frame >= 10420) {
                camSetEye((Camera *) w->cam, -16.0f, 121.0f, -34.0f);
                camSetAt((Camera *) w->cam, -9.0f, 96.0f, 64.0f);
                w->scene++;
            }
            break;
        case 9:
            if (frame >= 12620) {
                w->scene++;
            }
            break;
        case 10:
            break;
    }
    camGetEye((Camera *) w->cam, &gCamChkEyeX, &gCamChkEyeY, &gCamChkEyeZ);
    camGetAt((Camera *) w->cam, &gCamChkAtX, &gCamChkAtY, &gCamChkAtZ);
}
