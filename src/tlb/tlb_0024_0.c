#include "tlb.h"
#include "tlb/tlb_0024.h"

#define DPRINTF

/* "CHADEMO"'s work (tlb_0023_0's layout). */
typedef struct ChaDemoW {
    /* 0x00 */ Obj *cam; /* "democam" */
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
/* This file's .data: a1clearFrame, then IDO's padding to 16 (0x08005760-0x0800576F);
 * tlb_0024_1's starts at 0x08005770. */
s32 a1clearFrame = 5; /* the scene's frame count */
extern Gfx a1clearCastleDl[]; /* the castle */

void a1clearCamScript(Obj *obj);

/* "CHADEMO": a character demo's director at the castle, built as tlb_0023_0's. It loads files
 * 0xBC, 0xBD and 0xC8, sets up the scene director, the effects, a letterboxed fade, OBJMOVE (6
 * models), the camera (50 degrees, far plane 20000), a deep sky, the attack task, the castle
 * model of file 0xBD at ten times its size, and the scene's five builders, then fixes the camera
 * at (3255, 786, 297) looking at (-486, 1298, -6). Each frame it counts and steps the demo
 * (a1clearCamScript) and keeps the count in a1clearFrame. */
s32 a1clearMenuProc(Obj *obj, s16 msg, void *arg) {
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "CHADEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0xBD);
            floadLoadSet(0xC8);
            tskCreate(obj, scParentProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 45, 0, 185);
            w->layer3Word = ((u32 *) gFrmLayer3)[6];
            gObjMoveCount = 6;
            demoObjMoveOpen();
            w->cam = tskCreate(obj, democam, 0, NULL);
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
            camBgradSetLines((Camera *) w->sky, 76, 240);
            camBgradMoveTop((ColorFade *) w->sky, 48, 48, 255);
            camBgradMoveBottom((ColorFade *) w->sky, 128, 128, 255);
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            demoJoyChkOpen();
            w->scene = 0;
            fadeCreate(gEffectTask);
            demoFadeStart(60.0f, 255.0f, 0.0f, 0, 0, 0);
            tskCreate(obj, attack_Proc, 0, NULL);
            DPRINTF("城を出します\n");
            demoModelCreate(obj, 0.0f, 0.0f, 0.0f, 10.0f, 10.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0xBD, a1clearCastleDl, 0, 0);
            gCamChkStep = 2.0f;
            gObjMoveStep = 2.0f;
            gCamChkMarkerSize = 0.01f;
            DPRINTF("部屋を出します\n");
            (*a1clearYukiCreate)(obj);
            (*a1clearTonoCreate)(obj);
            (*a1clearGoeCreate)(obj);
            (*a1clearEbiCreate)(obj);
            (*a1clearSasCreate)(obj);
            camSetEye((Camera *) w->cam, 3255.0f, 786.0f, 297.0f);
            camSetAt((Camera *) w->cam, -486.0f, 1298.0f, -6.0f);
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
                (*a1clearCamScript)(obj);
            }
            a1clearFrame = w->timer;
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
extern Gfx a1clearCamScriptRoomDl[]; /* the room */

/* The castle scene's camera script, one step a time (scene), on the scene's frame count; then
 * the camera check follows the camera. 0: a glide in on the castle, with sound 0x1C. 1: a fade
 * out at 170; at 198 the old models go, and at 200 the room (file 0xC8) is made, with sound
 * 0x306. 2: a glide at 330. 3-8: cuts, each fading out at 960, 1707, 2580, 3100, 3492 or 3962
 * and back in 10 frames later; the last three end the running scenario. 9: at 5692 the last
 * fade out, with sound 7. */
void a1clearCamScript(Obj *obj) {
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[3];

    switch (w->scene) {
        case 0:
            camSetEye((Camera *) w->cam, 2822.0f, 1068.0f, 303.0f);
            camSetAt((Camera *) w->cam, -326.0f, 1886.0f, 76.0f);
            camSetSpeed((Camera *) w->cam, 260.0f);
            camMoveEye((Camera *) w->cam, 2606.0f, 1068.0f, 303.0f);
            camMoveAt((Camera *) w->cam, -326.0f, 1886.0f, 76.0f);
            sndPlay(0x1C);
            w->scene++;
            break;
        case 1:
            if (a1clearFrame == 170) {
                demoFadeStart(30.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (a1clearFrame == 198) {
                gDemoModelsDelete2 = 1;
            }
            if (a1clearFrame == 200) {
                gDemoModelsDelete2 = 0;
                demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 614.0f, 123.0f, -57.0f);
                camSetAt((Camera *) w->cam, 747.0f, 159.0f, -22.0f);
                demoModelCreate(obj, 0.0f, 0.0f, 0.0f, 10.0f, 10.0f, 10.0f, 0.0f, -1.5707964f, 0.0f, 0xC8, a1clearCamScriptRoomDl, 0,
                                0);
                sndPlay(0x306);
                w->scene++;
            }
            break;
        case 2:
            if (a1clearFrame == 330) {
                camSetSpeed((Camera *) w->cam, 300.0f);
                camMoveEye((Camera *) w->cam, 486.0f, 133.0f, -103.0f);
                camMoveAt((Camera *) w->cam, 615.0f, 135.0f, -86.0f);
                w->scene++;
            }
            break;
        case 3:
            if (a1clearFrame == 960) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (a1clearFrame == 970) {
                demoFadeStart(8.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 366.0f, 129.0f, 153.0f);
                camSetAt((Camera *) w->cam, 355.0f, 127.0f, 84.0f);
                w->scene++;
            }
            break;
        case 4:
            if (a1clearFrame == 1707) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (a1clearFrame == 1717) {
                demoFadeStart(8.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 356.0f, 211.0f, 243.0f);
                camSetAt((Camera *) w->cam, 535.0f, 125.0f, -86.0f);
                w->scene++;
            }
            break;
        case 5:
            if (a1clearFrame == 2580) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (a1clearFrame == 2590) {
                demoFadeStart(8.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 418.0f, 169.0f, -73.0f);
                camSetAt((Camera *) w->cam, 529.0f, 155.0f, -110.0f);
                w->scene++;
            }
            break;
        case 6:
            if (a1clearFrame == 3100) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
                scRunEnd();
            }
            if (a1clearFrame == 3110) {
                demoFadeStart(8.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 148.0f, 125.0f, 41.0f);
                camSetAt((Camera *) w->cam, 55.0f, 125.0f, -998.0f);
                w->scene++;
            }
            break;
        case 7:
            if (a1clearFrame == 3492) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
                scRunEnd();
            }
            if (a1clearFrame == 3502) {
                demoFadeStart(8.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 418.0f, 169.0f, -73.0f);
                camSetAt((Camera *) w->cam, 529.0f, 155.0f, -110.0f);
                w->scene++;
            }
            break;
        case 8:
            if (a1clearFrame == 3962) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
                scRunEnd();
            }
            if (a1clearFrame == 3972) {
                demoFadeStart(8.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 356.0f, 211.0f, 243.0f);
                camSetAt((Camera *) w->cam, 535.0f, 125.0f, -86.0f);
                w->scene++;
            }
            break;
        case 9:
            if (a1clearFrame == 5692) {
                demoFadeStart(30.0f, 0.0f, 255.0f, 0, 0, 0);
                sndPlay(7);
                scRunEnd();
            }
            break;
    }
    camGetEye((Camera *) w->cam, &gCamChkEyeX, &gCamChkEyeY, &gCamChkEyeZ);
    camGetAt((Camera *) w->cam, &gCamChkAtX, &gCamChkAtY, &gCamChkAtZ);
}


/* "A1CLEAR": the castle scene after area 1, as the game plays it. It sets up as the debug
 * director (a1clearMenuProc) does, with lights (a white key light from above and in
 * front, a black fill and a light grey ambient) and no OBJMOVE models, marks stage 0x67's flag
 * 0x80 and fades in from white. Start skips it, and it ends by itself at frame 5352. The skip
 * count runs its end: a fade to black with sound 10 at 3, the models go and the scene ends at
 * 40, and at 50 it waits for the screen and calls stageGotoSubt(1). Its skip count is a static
 * in tlb_0024's .bss (0x080066A0).
 * Matched 2026-10-12. Was NON_MATCHING (114 lines, same length): registers, as tlb_0023_0's
 * release CHADEMOs: the game holds the skip count in v1, IDO here in v0 (fixed by the sndPlay
 * FAKE below, IDO_NOTES 369, to real 6); the last 6: w->scene = 0 after gDemoCamHold = 0.
 * 2026-10-17 (a kept FAKE): since scRunEnd returns its Obj *
 * (commit 53ddf08), its dropped result in case 40 keeps v0 busy, so sndPlay is called plainly. */
s32 a1clearProc(Obj *obj, s16 msg, void *arg) {
    static s32 skip;
    ChaDemoW *w = (ChaDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "A1CLEAR";
            gDmenuLastObj = obj;
            w->timer = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0xBD);
            floadLoadSet(0xC8);
            tskCreate(obj, scParentProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 0, 0, 240);
            w->layer3Word = ((u32 *) gFrmLayers[3])[6];
            w->f0C = frmCreate(gFrmLayers[0], gFrmKindLight);
            ((LightFrame *) w->f0C)->dir[0] = 0;
            ((LightFrame *) w->f0C)->dir[1] = 127;
            ((LightFrame *) w->f0C)->dir[2] = 100;
            fadeLightNormalize((LightFrame *) w->f0C);
            ((LightFrame *) w->f0C)->color[0] = 255;
            ((LightFrame *) w->f0C)->color[1] = 255;
            ((LightFrame *) w->f0C)->color[2] = 255;
            w->f10 = frmCreate(gFrmLayers[0], gFrmKindLight);
            ((LightFrame *) w->f10)->color[0] = 0;
            ((LightFrame *) w->f10)->color[1] = 0;
            ((LightFrame *) w->f10)->color[2] = 0;
            ((LightFrame *) w->f10)->dir[0] = 0;
            ((LightFrame *) w->f10)->dir[1] = -127;
            ((LightFrame *) w->f10)->dir[2] = -100;
            w->f04 = frmCreate(gFrmLayers[0], gFrmKindAmbi);
            ((AmbFrame *) w->f04)->color[0] = 160;
            ((AmbFrame *) w->f04)->color[1] = 160;
            ((AmbFrame *) w->f04)->color[2] = 160;
            gObjMoveCount = 0;
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
            gSaveData.stageFlags[0x67] |= 0x80;
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            w->scene = 0;
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 45, 0, 185);
            demoFadeStart(60.0f, 255.0f, 0.0f, 255, 255, 255);
            tskCreate(obj, attack_Proc, 0, NULL);
            DPRINTF("城を出します\n");
            demoModelCreate(obj, 0.0f, 0.0f, 0.0f, 10.0f, 10.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0xBD, a1clearCastleDl, 0, 0);
            gCamChkStep = 2.0f;
            gObjMoveStep = 2.0f;
            gCamChkMarkerSize = 0.01f;
            (*a1clearYukiCreate)(obj);
            (*a1clearTonoCreate)(obj);
            (*a1clearGoeCreate)(obj);
            (*a1clearEbiCreate)(obj);
            (*a1clearSasCreate)(obj);
            camSetEye((Camera *) w->cam, 3255.0f, 786.0f, 297.0f);
            camSetAt((Camera *) w->cam, -486.0f, 1298.0f, -6.0f);
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
                    stageGotoSubt(1);
                    return -1;
                case 0:
                    break;
                default:
                    skip++;
                    break;
            }
            w->timer++;
            (*a1clearCamScript)(obj);
            a1clearFrame = w->timer;
            if (a1clearFrame == 5352 && skip == 0) {
                skip = 1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
