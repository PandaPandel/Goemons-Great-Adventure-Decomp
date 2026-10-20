#include "tlb.h"
#include "tlb/tlb_0019.h"

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
typedef struct GDemoW {
    /* 0x00 */ Obj *cam; /* "dracam" */
    /* 0x04 */ Frame *f04;
    /* 0x08 */ CamView *view;
    /* 0x0C */ Frame *f0C;
    /* 0x10 */ Frame *f10;
    /* 0x14 */ u8 pad14[4];
    /* 0x18 */ Obj *sky; /* "bgrad" */
    /* 0x1C */ AsciiFrame *text; /* "DEMOTIMER $ nnnn" */
    /* 0x20 */ u32 layer3Word; /* gFrmLayer3's word at 0x18, put back on the way out */
    /* 0x24 */ u8 unused24;
    /* 0x25 */ u8 scene;
    /* 0x28 */ s32 timer; /* frames of demo run */
} GDemoW;

/* libultra's sprintf, called through its address. */
#define sprintf (*sprintf)

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
extern CamView *gGdemoCamView; /* the menu demo's camera view */
/* This file's .data is gdemoFrame, gdemoCamScript's static and gdemoProc's skip, then IDO's
 * padding to 16 (0x08006C40-0x08006C4F); tlb_0019_1's starts at 0x08006C50. */
s32 gdemoFrame = 5; /* the demo's frame count */
extern Gfx gdemoModelDl[];
extern Gfx gdemoShadowCreateDl[];

void gdemoCamScript(Obj *obj);

/* "GDEMO", the debug-menu demo at sunset: the scene director and the effects, files 0xBD, 0xBC,
 * 0xBF and 0xBE, a letterboxed fade, OBJMOVE (5 models), the dragon camera (50 degrees, far
 * plane 20000) whose view the scene's sounds use (gGdemoCamView), a low red light, a dusk sky, the
 * joypad check, the attack task, the ground of file 0xBD turned a quarter, five shadows
 * (gdemoShadowCreate), the scene's builders and the "DEMOTIMER" line. Each frame, unless
 * the camera is held, it counts and steps the demo (gdemoCamScript) and prints the
 * count; held, the line blinks STOP. Z on pad 1 holds the camera, Z opens the camera check after
 * frame 10. */
s32 gdemoMenuProc(Obj *obj, s16 msg, void *arg) {
    GDemoW *w = (GDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "GDEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            tskCreate(obj, scParentProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            floadLoadSet(0xBD);
            floadLoadSet(0xBC);
            floadLoadSet(0xBF);
            floadLoadSet(0xBE);
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 45, 0, 185);
            w->layer3Word = ((u32 *) gFrmLayer3)[6];
            gObjMoveCount = 5;
            demoObjMoveOpen();
            w->cam = tskCreate(obj, dracam, 0, NULL);
            DPRINTF("カメラを作りました\n");
            camSetFov((Camera *) w->cam, 0.87266463f);
            w->view = camGetView((Camera *) w->cam);
            w->view->zFar = 20000.0f;
            gGdemoCamView = w->view;
            gCamChkEyeX = 0.0f;
            gCamChkEyeY = 0.0f;
            gCamChkEyeZ = 0.0f;
            gCamChkAtX = 0.0f;
            gCamChkAtY = 1000.0f;
            gCamChkAtZ = -4000.0f;
            gCamChkStep = 10.0f;
            gObjMoveStep = 10.0f;
            w->f0C = frmCreate(gFrmLayers[0], gFrmKindLight);
            ((LightFrame *) w->f0C)->dir[0] = 0;
            ((LightFrame *) w->f0C)->dir[1] = -127;
            ((LightFrame *) w->f0C)->dir[2] = -127;
            fadeLightNormalize((LightFrame *) w->f0C);
            ((LightFrame *) w->f0C)->color[0] = 240;
            ((LightFrame *) w->f0C)->color[1] = 112;
            ((LightFrame *) w->f0C)->color[2] = 80;
            w->f10 = frmCreate(gFrmLayers[0], gFrmKindLight);
            ((LightFrame *) w->f10)->color[0] = 0;
            ((LightFrame *) w->f10)->color[1] = 0;
            ((LightFrame *) w->f10)->color[2] = 0;
            ((LightFrame *) w->f10)->dir[0] = 0;
            ((LightFrame *) w->f10)->dir[1] = -127;
            ((LightFrame *) w->f10)->dir[2] = -127;
            w->f04 = frmCreate(gFrmLayers[0], gFrmKindAmbi);
            ((AmbFrame *) w->f04)->color[0] = 96;
            ((AmbFrame *) w->f04)->color[1] = 48;
            ((AmbFrame *) w->f04)->color[2] = 32;
            w->sky = tskCreate(obj, bgrad, 0, NULL);
            camBgradSetLines((Camera *) w->sky, 76, 240);
            camBgradMoveTop((ColorFade *) w->sky, 0, 0, 10);
            camBgradMoveBottom((ColorFade *) w->sky, 60, 23, 0);
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            demoJoyChkOpen();
            tskCreate(obj, attack_Proc, 0, NULL);
            demoModelCreate(obj, 0.0f, 0.0f, -4000.0f, 10.0f, 10.0f, 10.0f, 0.0f, -1.5707964f, 0.0f, 0xBD, gdemoModelDl, 0,
                            0);
            (*gdemoShadowCreate)(obj, -338.0f, -29.0f, -3035.0f, 14.225f, 1.0f, 4.0f, 0.0f, 0.0f, 0.0f, 0xBD,
                                      gdemoShadowCreateDl);
            (*gdemoShadowCreate)(obj, 317.0f, -31.0f, -2923.0f, 1.409f, 1.0f, 4.0f, 0.0f, 0.0f, 0.0f, 0xBD,
                                      gdemoShadowCreateDl);
            (*gdemoShadowCreate)(obj, 842.0f, -31.0f, -3283.0f, 11.435f, 1.0f, 6.0f, 0.0f, 0.0f, 0.0f, 0xBD,
                                      gdemoShadowCreateDl);
            (*gdemoShadowCreate)(obj, 1429.0f, -31.0f, -2905.0f, 1.449f, 1.0f, 4.0f, 0.0f, 0.0f, 0.0f, 0xBD,
                                      gdemoShadowCreateDl);
            (*gdemoShadowCreate)(obj, 1889.0f, -31.0f, -3024.0f, 10.48f, 1.0f, 4.0f, 0.0f, 0.0f, 0.0f, 0xBD,
                                      gdemoShadowCreateDl);
            (*gdemoWaterCreate)(obj);
            (*gdemoGenoimpCreate)(obj);
            (*gdemoGenoTrioCreate)(obj);
            w->text = ((AsciiNewFunc) asciiNew)(gFrmTextRoot, 18, 177, "                    ");
            sprintf(w->text->str, "DEMOTIMER $ %04d ", w->timer);
            DPRINTF("%d\n");
            w->scene = 0;
            demoFadeStart(60.0f, 255.0f, 0.0f, 0, 0, 0);
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
                (*gdemoCamScript)(obj);
                sprintf(w->text->str, "DEMOTIMER $ %04d ", w->timer);
            } else if (gGameFrameCount % 60 >= 30) {
                sprintf(w->text->str, "DEMOTIMER $ STOP ");
            } else {
                sprintf(w->text->str, "DEMOTIMER $      ");
            }
            gdemoFrame = w->timer;
            if (joyGetHeldTime(0, 0x2000) == 1) {
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

/* GDEMO's camera script, by step (scene) and the director's own frame count; then the camera
 * check follows the camera, a cloud of dust rises every 14 frames until 1750
 * (gdemoKemuriCreate) and gdemoScenarioStep runs. 0: the camera high, diving, sounds
 * 0x1F and 0x1B7. 1: sound 0x514 every 30 frames (60 from 225) until 538; the dive slows until
 * 200; from 341 it glides on. 2: at 491. 3: a fade out at 700; from 731 sound 0x51E, a cut, the
 * light turned and a fade in. 4: gdemoLightningA each frame; at 1420 the task
 * gdemoGenoKuroProc; at 1750 a cut and a fade out. 5: a fade in at 1753;
 * gdemoLightningB each frame; the scene director ends at 4840; at 5140 the last fade out
 * and sound 10. 6: gdemoLightningB. */
void gdemoCamScript(Obj *obj) {
    static s32 period = 1; /* of the dive's sound */
    GDemoW *w = (GDemoW *) (obj + 1);
    s32 pad;

    switch (w->scene) {
        case 0:
            camSetEye((Camera *) w->cam, 940.0f, 1430.0f, -2810.0f);
            camSetAt((Camera *) w->cam, 970.0f, 1990.0f, -3840.0f);
            camSetSpeed((Camera *) w->cam, 3000.0f);
            camMoveEye((Camera *) w->cam, 900.0f, 250.0f, -1840.0f);
            camMoveAt((Camera *) w->cam, 1120.0f, 1810.0f, -5180.0f);
            sndPlay(0x1F);
            sndPlay(0x1B7);
            w->scene = 1;
            DPRINTF("<main>ステップ１へ\n");
            period = 30;
            break;
        case 1:
            if (w->timer == 225) {
                period = 60;
            }
            if (w->timer % period == 0 && w->timer < 538) {
                sndPlay(0x514);
            }
            if (w->timer < 200) {
                camSetSpeed((Camera *) w->cam, 2940 - w->timer * 14);
                DPRINTF("speed %d\n");
                camMoveEye((Camera *) w->cam, 900.0f, 250.0f, -1840.0f);
                camMoveAt((Camera *) w->cam, 1120.0f, 1810.0f, -5180.0f);
            }
            if (w->timer == 200) {
                camSetSpeed((Camera *) w->cam, 140.0f);
                camMoveEye((Camera *) w->cam, 900.0f, 250.0f, -1840.0f);
                camMoveAt((Camera *) w->cam, 1120.0f, 1810.0f, -5180.0f);
            }
            if (w->timer >= 341) {
                camSetSpeed((Camera *) w->cam, 190.0f);
                camMoveEye((Camera *) w->cam, 761.0f, 79.0f, -1176.0f);
                camMoveAt((Camera *) w->cam, 1095.0f, 1477.0f, -4896.0f);
                w->scene = 2;
                DPRINTF("<main>ステップ2へ\n");
            }
            break;
        case 2:
            if (w->timer >= 491) {
                w->scene = 3;
                DPRINTF("<main>ステップ3へ\n");
            }
            break;
        case 3:
            if (w->timer == 700) {
                demoFadeStart(30.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (w->timer >= 731) {
                sndPlay(0x51E);
                camSetEye((Camera *) w->cam, 1140.0f, 1050.0f, -2470.0f);
                camSetAt((Camera *) w->cam, 810.0f, 130.0f, -1850.0f);
                camSetSpeed((Camera *) w->cam, 1000.0f);
                camMoveEye((Camera *) w->cam, 1060.0f, 700.0f, -2470.0f);
                camMoveAt((Camera *) w->cam, 810.0f, 130.0f, -1850.0f);
                ((LightFrame *) w->f0C)->dir[0] = 0;
                ((LightFrame *) w->f0C)->dir[1] = 127;
                ((LightFrame *) w->f0C)->dir[2] = -127;
                fadeLightNormalize((LightFrame *) w->f0C);
                demoFadeStart(5.0f, 255.0f, 0.0f, 0, 0, 0);
                w->scene = 4;
                DPRINTF("<main>ステップ4へ 俯瞰視点にします\n");
            }
            break;
        case 4:
            if (w->timer == 720) {
                demoFadeStart(10.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            (*gdemoLightningA)(obj);
            if (w->timer == 1420) {
                tskCreate(obj, gdemoGenoKuroProc, gDmenuLastObj->file, NULL);
            }
            if (w->timer == 1750) {
                w->scene = 5;
                camSetEye((Camera *) w->cam, 1160.0f, 320.0f, -1380.0f);
                camSetAt((Camera *) w->cam, 880.0f, 180.0f, -1950.0f);
                camSetSpeed((Camera *) w->cam, 1030.0f);
                demoFadeStart(2.0f, 0.0f, 255.0f, 0, 0, 0);
                w->scene = 5;
                DPRINTF("<main>ステップ5へ  会話アングルにかわります\n");
            }
            break;
        case 5:
            if (w->timer == 1753) {
                demoFadeStart(2.0f, 255.0f, 0.0f, 0, 0, 0);
            }
            (*gdemoLightningB)(obj);
            if (w->timer == 5140) {
                demoFadeStart(100.0f, 0.0f, 255.0f, 0, 0, 0);
                sndPlay(10);
            }
            if (w->timer == 4840) {
                scRunEnd();
            }
            break;
        case 6:
            (*gdemoLightningB)(obj);
            break;
    }
    camGetEye((Camera *) w->cam, &gCamChkEyeX, &gCamChkEyeY, &gCamChkEyeZ);
    camGetAt((Camera *) w->cam, &gCamChkAtX, &gCamChkAtY, &gCamChkAtZ);
    if (w->timer % 14 == 0 && w->timer < 1750) {
        (*gdemoKemuriCreate)(fadeRandRange(0, 4000) + 280.0f, 1020.0f, -5830.0f);
    }
    (*gdemoScenarioStep)();
}

extern s32 gDemoModelsDelete2;
extern s32 gObjMoveSel; /* OBJMOVE: the model chosen, -1 for none */

/* "GDEMO" as played in the game: no joypad check or text line, stage flag 0x64 (bit 7), and
 * Start skips it ("飛ばします"): from 1 it counts frames, at 3 fading out with sound 10, at 40
 * ending the scene director and setting the time to 9 o'clock, at 50 going to the atlas
 * (stageGotoAtlasFrom); at frame 4940 the skip starts by itself. */
s32 gdemoProc(Obj *obj, s16 msg, void *arg) {
    static s32 skip = 1;
    GDemoW *w = (GDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "GDEMO";
            gDmenuLastObj = obj;
            w->timer = 0;
            tskCreate(obj, scParentProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            floadLoadSet(0xBD);
            floadLoadSet(0xBC);
            floadLoadSet(0xBF);
            floadLoadSet(0xBE);
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 45, 0, 185);
            w->layer3Word = ((u32 *) gFrmLayer3)[6];
            gObjMoveCount = 5;
            gObjMoveSel = -1;
            w->cam = tskCreate(obj, dracam, 0, NULL);
            DPRINTF("カメラを作りました\n");
            camSetFov((Camera *) w->cam, 0.87266463f);
            w->view = camGetView((Camera *) w->cam);
            w->view->zFar = 20000.0f;
            gGdemoCamView = w->view;
            gCamChkEyeX = 0.0f;
            gCamChkEyeY = 0.0f;
            gCamChkEyeZ = 0.0f;
            gCamChkAtX = 0.0f;
            gCamChkAtY = 1000.0f;
            gCamChkAtZ = -4000.0f;
            gCamChkStep = 10.0f;
            gObjMoveStep = 10.0f;
            w->f0C = frmCreate(gFrmLayers[0], gFrmKindLight);
            ((LightFrame *) w->f0C)->dir[0] = 0;
            ((LightFrame *) w->f0C)->dir[1] = -127;
            ((LightFrame *) w->f0C)->dir[2] = -127;
            fadeLightNormalize((LightFrame *) w->f0C);
            ((LightFrame *) w->f0C)->color[0] = 240;
            ((LightFrame *) w->f0C)->color[1] = 112;
            ((LightFrame *) w->f0C)->color[2] = 80;
            w->f10 = frmCreate(gFrmLayers[0], gFrmKindLight);
            ((LightFrame *) w->f10)->color[0] = 0;
            ((LightFrame *) w->f10)->color[1] = 0;
            ((LightFrame *) w->f10)->color[2] = 0;
            ((LightFrame *) w->f10)->dir[0] = 0;
            ((LightFrame *) w->f10)->dir[1] = -127;
            ((LightFrame *) w->f10)->dir[2] = -127;
            w->f04 = frmCreate(gFrmLayers[0], gFrmKindAmbi);
            ((AmbFrame *) w->f04)->color[0] = 96;
            ((AmbFrame *) w->f04)->color[1] = 48;
            ((AmbFrame *) w->f04)->color[2] = 32;
            w->sky = tskCreate(obj, bgrad, 0, NULL);
            camBgradSetLines((Camera *) w->sky, 76, 240);
            camBgradMoveTop((ColorFade *) w->sky, 0, 0, 10);
            camBgradMoveBottom((ColorFade *) w->sky, 60, 23, 0);
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            tskCreate(obj, attack_Proc, 0, NULL);
            demoModelCreate(obj, 0.0f, 0.0f, -4000.0f, 10.0f, 10.0f, 10.0f, 0.0f, -1.5707964f, 0.0f, 0xBD, gdemoModelDl, 0,
                            0);
            (*gdemoShadowCreate)(obj, -338.0f, -29.0f, -3035.0f, 14.225f, 1.0f, 4.0f, 0.0f, 0.0f, 0.0f, 0xBD,
                                      gdemoShadowCreateDl);
            (*gdemoShadowCreate)(obj, 317.0f, -31.0f, -2923.0f, 1.409f, 1.0f, 4.0f, 0.0f, 0.0f, 0.0f, 0xBD,
                                      gdemoShadowCreateDl);
            (*gdemoShadowCreate)(obj, 842.0f, -31.0f, -3283.0f, 11.435f, 1.0f, 6.0f, 0.0f, 0.0f, 0.0f, 0xBD,
                                      gdemoShadowCreateDl);
            (*gdemoShadowCreate)(obj, 1429.0f, -31.0f, -2905.0f, 1.449f, 1.0f, 4.0f, 0.0f, 0.0f, 0.0f, 0xBD,
                                      gdemoShadowCreateDl);
            (*gdemoShadowCreate)(obj, 1889.0f, -31.0f, -3024.0f, 10.48f, 1.0f, 4.0f, 0.0f, 0.0f, 0.0f, 0xBD,
                                      gdemoShadowCreateDl);
            (*gdemoWaterCreate)(obj);
            (*gdemoGenoimpCreate)(obj);
            (*gdemoGenoTrioCreate)(obj);
            gSaveData.stageFlags[0x64] |= 0x80;
            w->scene = 0;
            skip = 0;
            demoFadeStart(60.0f, 255.0f, 0.0f, 0, 0, 0);
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
            if (fadeStartPressed() && skip == 0) {
                skip = 1;
                DPRINTF("飛ばします");
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
                    gTimeOfDay = 9 << 16;
                    skip++;
                    break;
                case 50:
                    while (gfxGetPendingSwaps()) {
                    }
                    stageGotoAtlasFrom(obj, 0, 5);
                    return -1;
                case 0:
                    break;
                default:
                    skip++;
                    break;
            }
            if (w->timer == 4940 && skip == 0) {
                skip = 1;
            }
            w->timer++;
            (*gdemoCamScript)(obj);
            gdemoFrame = w->timer;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
