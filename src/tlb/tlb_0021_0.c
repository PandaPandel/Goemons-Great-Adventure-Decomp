#include "tlb.h"
#include "tlb/tlb_0021.h"

#define DPRINTF

/* An opening demo's work (OPDEMO1-3, OPMAIN): its camera, view, sky and timer text. */
typedef struct OpDemoW {
    /* 0x00 */ Obj *cam;  /* "democam" */
    /* 0x04 */ Frame *f04; /* made by the scene's builders */
    /* 0x08 */ CamView *view;
    /* 0x0C */ Frame *f0C;
    /* 0x10 */ Frame *f10;
    /* 0x14 */ u8 pad14[4];
    /* 0x18 */ Obj *sky; /* "bgrad" */
    /* 0x1C */ struct AsciiFrame *text; /* "DEMOTIMER" */
    /* 0x20 */ Obj *bg; /* OPDEMO1's "cambg" */
    /* 0x24 */ u32 layer3Word; /* gFrmLayer3's word at 0x18, put back on the way out */
    /* 0x28 */ u8 unused28;
    /* 0x29 */ u8 scene;
    /* 0x2C */ s32 timer; /* frames of demo run */
} OpDemoW;

/* ascii.c's text frame (asciiNew returns it, though tlb.h declares it void). */
typedef struct AsciiFrame {
    /* 0x00 */ Frame f;
    /* 0x14 */ s16 x;
    /* 0x16 */ s16 y;
    /* 0x18 */ char *str;
} AsciiFrame;

typedef AsciiFrame *(*AsciiNewFunc)(Frame *parent, s16 x, s16 y, char *str);

/* libultra's sprintf, called through its address. */
#define sprintf (*sprintf)

extern Frame *gFrmLayer3;
extern u8 gDemoCamHold;
extern s32 gDemoModelsDelete;
extern s32 gDemoModelsDelete2;
extern s32 gObjMoveCount;
extern u8 gCamChkActive; /* read lbu here (demotool.c declares it s8) */
extern f32 gCamChkEyeX;
extern f32 gCamChkEyeY;
extern f32 gCamChkEyeZ;
extern f32 gCamChkAtX;
extern f32 gCamChkAtY;
extern f32 gCamChkAtZ;
/* The overlay's .data starts with this file's: the shared globals (5 and 0x12345670 are the
 * game's placeholders), then the statics of opdemo2CamScript, opdemo3CamScript and OPMAIN. */
f32 op1GoeX = 5.0f; /* the camera's offsets in OPDEMO1's step 2 */
f32 op1GoeY = 5.0f;
f32 op1GoeZ = 5.0f;
s32 opdemo1Frame = 5; /* OPDEMO1's frame counter, for the scene's tasks */
s32 opdemo2Frame = 5; /* OPDEMO2's */
s32 opdemo3Frame = 5; /* OPDEMO3's */
CamView *opDemoView = (CamView *) 0x12345670; /* the demo's camera view, for them */
s32 opDemoPlaying = 5; /* the demo playing: 1-3, 0 for OPMAIN's own */
s32 opDemoUnused = 5;
extern Gfx opdemo2ModelDl[];
extern Gfx opdemo1Model0Dl[];
extern Gfx opdemo1Model1Dl[];
extern Gfx opdemo1Model2Dl[];
extern Gfx opdemo1Model3Dl[];
extern Gfx opdemo1Model4Dl[];

void opdemo1CamScript(Obj *obj);

/* "OPDEMO1": the first opening demo, in the forest. It loads files 0xBC, 0xC0, 0xC1, 0x11D and
 * 0x13B, sets up the scene director, the effects, a letterboxed fade, OBJMOVE (5 models), the
 * camera (50 degrees, far plane 20000), the sun, the sky, the ground, VRAM, the rider and the
 * attack task, then five models of file 0xC0 (ground, tree, fern, branch, branch 2) 400 back, a
 * far background (5 layers at 800), and the scene's other builders. Each frame it is OPDEMO2's
 * frame with its own step (opdemo1CamScript). */
s32 opdemo1Proc(Obj *obj, s16 msg, void *arg) {
    OpDemoW *w = (OpDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "OPDEMO1";
            w->timer = 0;
            gDmenuLastObj = obj;
            gDemoModelsDelete2 = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0xC0);
            floadLoadSet(0xC1);
            floadLoadSet(0x11D);
            floadLoadSet(0x13B);
            tskCreate(obj, scParentProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 45, 0, 185);
            w->layer3Word = ((u32 *) gFrmLayer3)[6];
            gObjMoveCount = 5;
            demoObjMoveOpen();
            opDemoPlaying = 1;
            w->cam = tskCreate(obj, democam, 0, NULL);
            DPRINTF("カメラを作りました\n");
            camSetFov((Camera *) w->cam, 0.87266463f);
            w->view = camGetView((Camera *) w->cam);
            w->view->zFar = 20000.0f;
            opDemoView = w->view;
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
            camBgradMoveBottom((ColorFade *) w->sky, 224, 224, 255);
            camBgradMoveTop((ColorFade *) w->sky, 255, 255, 255);
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            demoJoyChkOpen();
            w->text = ((AsciiNewFunc) asciiNew)(gFrmTextRoot, 18, 177, "                    ");
            w->scene = 0;
            demoFadeStart(16.0f, 255.0f, 0.0f, 255, 255, 255);
            tskCreate(obj, Geography, 0, NULL);
            vramCreate(obj);
            tskCreate(obj, Ride, 0, NULL);
            tskCreate(obj, attack_Proc, 0, NULL);
            DPRINTF("地面を出します\n");
            demoModelCreate(obj, 0.0f, 0.0f, -400.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0xC0, opdemo1Model0Dl, 0, 1);
            DPRINTF("樹を出します\n");
            demoModelCreate(obj, 0.0f, 0.0f, -400.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0xC0, opdemo1Model1Dl, 0, 1);
            DPRINTF("シダを出します\n");
            demoModelCreate(obj, 0.0f, 0.0f, -400.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0xC0, opdemo1Model2Dl, 0, 1);
            DPRINTF("枝を出します\n");
            demoModelCreate(obj, 0.0f, 0.0f, -400.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0xC0, opdemo1Model3Dl, 0, 1);
            DPRINTF("枝2を出します\n");
            demoModelCreate(obj, 0.0f, 0.0f, -400.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0xC0, opdemo1Model4Dl, 0, 1);
            (*opGateRipplesCreate)(obj);
            w->bg = cambgCreate(obj, 5, 800.0f);
            cambgMove(w->bg, 0, 30);
            gCamChkStep = 1.0f;
            gObjMoveStep = 1.0f;
            gCamChkMarkerSize = 0.01f;
            (*op1GoeCreate)(obj);
            (*op1EbiCreate)(obj);
            camSunSetClockStop(1);
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
                (*opdemo1CamScript)(obj);
                sprintf(w->text->str, "DEMOTIMER $ %04d ", w->timer);
            } else if (gGameFrameCount % 60 >= 30) {
                sprintf(w->text->str, "DEMOTIMER $ STOP ");
            } else {
                sprintf(w->text->str, "DEMOTIMER $      ");
            }
            opdemo1Frame = w->timer;
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

extern Gfx opdemo1CamModel0Dl[];
extern Gfx opdemo1CamModel1Dl[];
extern Gfx opdemo1CamModel2Dl[];

/* OPDEMO1's camera script, one step a time (scene), then the camera check follows the camera.
 * 0: the camera starts low in the trees, a lens flare, sounds 0x8F and 0x1BF. 1: wait for frame
 * 600. 2: drift; fade out at 1654; at 1670 cut to the gate and fade in. 3: fade out at 1878; at
 * 1940 cut and fade in, rising. 4: the dusk sky, the gate (opGateCreate), the far background
 * moved, and three models at the gate. 5: at 2440, sound 0x51D and the last fade out. */
void opdemo1CamScript(Obj *obj) {
    OpDemoW *w = (OpDemoW *) (obj + 1);
    s32 pad;

    switch (w->scene) {
        case 0:
            camSetEye((Camera *) w->cam, 0.0f, 40.0f, -135.0f);
            camSetAt((Camera *) w->cam, 5.0f, 249.0f, -200.0f);
            camSetSpeed((Camera *) w->cam, 500.0f);
            camMoveEye((Camera *) w->cam, 0.0f, 40.0f, -135.0f);
            camMoveAt((Camera *) w->cam, 20.0f, 20.0f, -600.0f);
            (*opLensflareCreate)(obj, 10.0f, 484.0f, -424.0f);
            sndPlay(0x8F);
            sndPlay(0x1BF);
            w->scene++;
            break;
        case 1:
            if (w->timer == 600) {
                w->scene++;
            }
            break;
        case 2:
            camSetSpeed((Camera *) w->cam, 60.0f);
            camMoveEye((Camera *) w->cam, op1GoeX + 16.0f, op1GoeY + 20.0f,
                       op1GoeZ + 75.0f);
            camMoveAt((Camera *) w->cam, op1GoeX - 21.0f, op1GoeY + 16.0f,
                      op1GoeZ - 30.0f);
            if (w->timer == 1654) {
                demoFadeStart(16.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (w->timer == 1670) {
                w->scene++;
                camSetEye((Camera *) w->cam, 54.0f, 11.0f, -562.0f);
                camSetAt((Camera *) w->cam, -11.0f, 18.0f, -653.0f);
                camSetSpeed((Camera *) w->cam, 300.0f);
                camMoveAt((Camera *) w->cam, -11.0f, 35.0f, -653.0f);
                demoFadeStart(16.0f, 255.0f, 0.0f, 0, 0, 0);
            }
            break;
        case 3:
            if (w->timer == 1878) {
                demoFadeStart(60.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (w->timer == 1940) {
                w->scene++;
                demoFadeStart(32.0f, 255.0f, 0.0f, 0, 0, 0);
                gDemoModelsDelete2 = 1;
                camSetEye((Camera *) w->cam, 15.0f, -4.0f, -535.0f);
                camSetAt((Camera *) w->cam, 20.0f, -15.0f, -700.0f);
                camSetSpeed((Camera *) w->cam, 800.0f);
                camMoveEye((Camera *) w->cam, 15.0f, -4.0f, -312.0f);
                camMoveAt((Camera *) w->cam, 20.0f, 264.0f, -700.0f);
            }
            break;
        case 4:
            gDemoModelsDelete2 = 0;
            camBgradSetLines((Camera *) w->sky, 76, 140);
            camBgradMoveTop((ColorFade *) w->sky, 160, 160, 255);
            camBgradMoveBottom((ColorFade *) w->sky, 255, 255, 255);
            (*opGateCreate)(obj);
            cambgMove(w->bg, 0, -50);
            demoModelCreate(obj, 22.0f, -31.0f, -788.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0xC1, opdemo1CamModel0Dl, 0, 0);
            demoModelCreate(obj, 22.0f, -31.0f, -788.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0xC0, opdemo1CamModel1Dl, 0, 0);
            demoModelCreate(obj, 22.0f, -31.0f, -788.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0xC0, opdemo1CamModel2Dl, 0, 0);
            w->scene++;
            break;
        case 5:
            if (w->timer == 2440) {
                sndPlay(0x51D);
                demoFadeStart(60.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            break;
    }
    camGetEye((Camera *) w->cam, &gCamChkEyeX, &gCamChkEyeY, &gCamChkEyeZ);
    camGetAt((Camera *) w->cam, &gCamChkAtX, &gCamChkAtY, &gCamChkAtZ);
}

void opdemo2CamScript(Obj *obj);

/* "OPDEMO2": the second opening demo. It loads files 0xBC, 0xC2, 0xC1 and 0xC5, sets up the
 * scene director, the effects, a letterboxed fade, OBJMOVE (7 models), the camera (50 degrees,
 * far plane 20000), the sun, the sky, the attack task and the stage model, then starts Goemon
 * (OP2GOE), Ebisumaru (OP2EBI) and the old man (jiji): op2GoeCreate, op2EbiCreate and
 * op2JijiCreate. Each frame it
 * counts and steps the demo (opdemo2CamScript) and shows the count. Controller 2's C-down
 * holds the demo; controller 1's L opens the camera check after 10 frames. */
s32 opdemo2Proc(Obj *obj, s16 msg, void *arg) {
    OpDemoW *w = (OpDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "OPDEMO2";
            w->timer = 0;
            gDmenuLastObj = obj;
            gDemoModelsDelete2 = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0xC2);
            floadLoadSet(0xC1);
            floadLoadSet(0xC5);
            tskCreate(obj, scParentProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 45, 0, 185);
            w->layer3Word = ((u32 *) gFrmLayer3)[6];
            gObjMoveCount = 7;
            demoObjMoveOpen();
            opDemoPlaying = 2;
            w->cam = tskCreate(obj, democam, 0, NULL);
            DPRINTF("カメラを作りました\n");
            camSetFov((Camera *) w->cam, 0.87266463f);
            opDemoView = w->view = camGetView((Camera *) w->cam);
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
            camBgradMoveBottom((ColorFade *) w->sky, 224, 224, 255);
            camBgradMoveTop((ColorFade *) w->sky, 255, 255, 255);
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            demoJoyChkOpen();
            w->text = ((AsciiNewFunc) asciiNew)(gFrmTextRoot, 18, 177, "                    ");
            w->scene = 0;
            demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
            tskCreate(obj, attack_Proc, 0, NULL);
            demoModelCreate(obj, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0xC2, opdemo2ModelDl, 0, 1);
            gCamChkStep = 1.0f;
            gObjMoveStep = 1.0f;
            gCamChkMarkerSize = 0.01f;
            DPRINTF("座敷を出します\n");
            (*op2GoeCreate)(obj);
            (*op2EbiCreate)(obj);
            (*op2JijiCreate)(obj);
            camSunSetClockStop(1);
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
                (*opdemo2CamScript)(obj);
                sprintf(w->text->str, "DEMOTIMER $ %04d ", w->timer);
            } else if (gGameFrameCount % 60 >= 30) {
                sprintf(w->text->str, "DEMOTIMER $ STOP ");
            } else {
                sprintf(w->text->str, "DEMOTIMER $      ");
            }
            opdemo2Frame = w->timer;
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
            DPRINTF("研究室を出します\n");
            DPRINTF("窓の光を出します\n");
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

extern Gfx opdemo2CamModel0Dl[];
extern Gfx opdemo2CamModel1Dl[];

#define sinf (*sinf)

/* OPDEMO2's camera script, by scene against OPDEMO2's frame count (opdemo2Frame), then the
 * camera check follows the camera. Most steps fade out over 3 frames and cut 5 frames later.
 * 3 (1459) adds the room (op2FusumaCreate); 4 (1659) the stage model, the lab (op2Mach2Create), a
 * second model and the evening sky; 5 is a short scene on exact frames (a white flash and sound
 * 0x246 at 1989). 11 (4620) sets the last shot; 12 sways the camera's look up and down (its
 * size grows by 0.01 a frame to 2.5) and flashes white every 40 and 30 frames until 4760, when
 * it fades to white. */
void opdemo2CamScript(Obj *obj) {
    static f32 swaySize = 1.0f;
    OpDemoW *w = (OpDemoW *) (obj + 1);
    s32 pad;

    switch (w->scene) {
        case 0:
            camSetEye((Camera *) w->cam, -184.0f, 16.0f, -6.0f);
            camSetAt((Camera *) w->cam, 56.0f, 35.0f, -5.0f);
            camSetSpeed((Camera *) w->cam, 300.0f);
            camMoveEye((Camera *) w->cam, -154.0f, 16.0f, -6.0f);
            w->scene++;
            break;
        case 1:
            if (opdemo2Frame == 30) {
                sndPlay(0x90);
            }
            if (opdemo2Frame == 288) {
                demoFadeStart(8.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (opdemo2Frame == 298) {
                camSetEye((Camera *) w->cam, -23.0f, 19.0f, 26.0f);
                camSetAt((Camera *) w->cam, -4.0f, 16.0f, -6.0f);
                demoFadeStart(5.0f, 255.0f, 0.0f, 0, 0, 0);
                w->scene++;
            }
            break;
        case 2:
            if (opdemo2Frame == 714) {
                demoFadeStart(3.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (opdemo2Frame == 719) {
                camSetEye((Camera *) w->cam, -7.0f, 28.0f, -64.0f);
                camSetAt((Camera *) w->cam, -6.0f, 17.0f, -5.0f);
                demoFadeStart(3.0f, 255.0f, 0.0f, 0, 0, 0);
                w->scene++;
            }
            break;
        case 3:
            if (opdemo2Frame == 1389) {
                demoFadeStart(30.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (opdemo2Frame == 1419) {
                gDemoModelsDelete2 = 1;
            }
            if (opdemo2Frame == 1459) {
                gDemoModelsDelete2 = 0;
                (*op2FusumaCreate)(obj);
                demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -47.0f, 69.0f, -4.0f);
                camSetAt((Camera *) w->cam, 23.0f, -13.0f, -145.0f);
                w->scene++;
            }
            break;
        case 4:
            if (opdemo2Frame == 1629) {
                demoFadeStart(30.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (opdemo2Frame == 1658) {
                gDemoModelsDelete2 = 1;
            }
            if (opdemo2Frame == 1659) {
                gDemoModelsDelete2 = 0;
                demoModelCreate(obj, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0xC2, opdemo2CamModel0Dl, 0, 1);
                (*op2Mach2Create)(obj);
                demoModelCreate(obj, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0xC2, opdemo2CamModel1Dl, 0, 2);
                demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
                camBgradSetLines((Camera *) w->sky, 76, 240);
                camBgradMoveTop((ColorFade *) w->sky, 160, 160, 255);
                camBgradMoveBottom((ColorFade *) w->sky, 112, 112, 255);
                camSetEye((Camera *) w->cam, -6.0f, 34.0f, -62.0f);
                camSetAt((Camera *) w->cam, 12.0f, 17.0f, -14.0f);
                camSetSpeed((Camera *) w->cam, 300.0f);
                camMoveEye((Camera *) w->cam, -6.0f, 12.0f, -62.0f);
                w->scene++;
            }
            break;
        case 5:
            switch (opdemo2Frame) {
                case 1969:
                    camSetSpeed((Camera *) w->cam, 20.0f);
                    camMoveEye((Camera *) w->cam, -25.0f, 19.0f, -2.0f);
                    camMoveAt((Camera *) w->cam, -24.0f, 41.0f, 27.0f);
                    demoFadeStart(20.0f, 0.0f, 255.0f, 255, 255, 255);
                    break;
                case 1989:
                    demoFadeStart(60.0f, 255.0f, 0.0f, 255, 255, 255);
                    sndPlay(0x246);
                    break;
                case 2049:
                    camSetSpeed((Camera *) w->cam, 100.0f);
                    camMoveEye((Camera *) w->cam, -25.0f, 12.0f, -106.0f);
                    camMoveAt((Camera *) w->cam, -23.0f, 35.0f, -14.0f);
                    break;
                case 2164:
                    demoFadeStart(3.0f, 0.0f, 255.0f, 0, 0, 0);
                    break;
                case 2169:
                    camSetEye((Camera *) w->cam, -44.0f, -1.0f, -53.0f);
                    camSetAt((Camera *) w->cam, -11.0f, 15.0f, -94.0f);
                    camSetSpeed((Camera *) w->cam, 300.0f);
                    camMoveAt((Camera *) w->cam, -11.0f, 15.0f, -41.0f);
                    demoFadeStart(3.0f, 255.0f, 0.0f, 0, 0, 0);
                    w->scene++;
                    break;
            }
            break;
        case 6:
            if (opdemo2Frame == 2324) {
                demoFadeStart(3.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (opdemo2Frame == 2329) {
                demoFadeStart(3.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -19.0f, 38.0f, -121.0f);
                camSetAt((Camera *) w->cam, -24.0f, 9.0f, 16.0f);
                camSetSpeed((Camera *) w->cam, 500.0f);
                camMoveEye((Camera *) w->cam, -19.0f, 38.0f, -80.0f);
                w->scene++;
            }
            break;
        case 7:
            if (opdemo2Frame == 2634) {
                demoFadeStart(3.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (opdemo2Frame == 2639) {
                demoFadeStart(3.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -52.0f, 35.0f, 20.0f);
                camSetAt((Camera *) w->cam, -19.0f, 9.0f, -38.0f);
                w->scene++;
            }
            break;
        case 8:
            if (opdemo2Frame == 3114) {
                demoFadeStart(3.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (opdemo2Frame == 3119) {
                demoFadeStart(3.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -47.0f, 44.0f, -27.0f);
                camSetAt((Camera *) w->cam, -36.0f, 31.0f, -7.0f);
                w->scene++;
            }
            break;
        case 9:
            if (opdemo2Frame == 3945) {
                demoFadeStart(3.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (opdemo2Frame == 3950) {
                demoFadeStart(3.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -52.0f, 35.0f, 20.0f);
                camSetAt((Camera *) w->cam, -19.0f, 9.0f, -38.0f);
                w->scene++;
            }
            break;
        case 10:
            if (opdemo2Frame == 4225) {
                demoFadeStart(3.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (opdemo2Frame == 4230) {
                demoFadeStart(3.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 24.0f, 62.0f, -29.0f);
                camSetAt((Camera *) w->cam, -19.0f, 21.0f, 12.0f);
                w->scene++;
            }
            break;
        case 11:
            if (opdemo2Frame == 4615) {
                demoFadeStart(3.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (opdemo2Frame == 4620) {
                demoFadeStart(3.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -73.0f, -10.0f, -95.0f);
                camSetAt((Camera *) w->cam, -25.0f, 61.0f, 50.0f);
                camSetSpeed((Camera *) w->cam, 700.0f);
                camMoveEye((Camera *) w->cam, 0.0f, -10.0f, -95.0f);
                swaySize = 0.1f;
                w->scene++;
            }
            break;
        case 12:
            if (swaySize < 2.5) {
                swaySize += 0.01;
            }
            camSetAt((Camera *) w->cam, -25.0f, sinf(opdemo2Frame * 70) * swaySize + 61.0f, 50.0f);
            if (opdemo2Frame < 4760) {
                if (opdemo2Frame % 40 == 37) {
                    demoFadeStart(3.0f, 0.0f, 160.0f, 255, 255, 255);
                }
                if (opdemo2Frame % 40 == 0) {
                    demoFadeStart(3.0f, 160.0f, 0.0f, 255, 255, 255);
                }
                if (opdemo2Frame % 30 == 38) {
                    demoFadeStart(3.0f, 0.0f, 96.0f, 255, 255, 255);
                }
                if (opdemo2Frame % 30 == 0) {
                    demoFadeStart(3.0f, 96.0f, 0.0f, 255, 255, 255);
                }
            }
            if (opdemo2Frame == 4760) {
                demoFadeStart(60.0f, 0.0f, 255.0f, 255, 255, 255);
            }
            break;
    }
    camGetEye((Camera *) w->cam, &gCamChkEyeX, &gCamChkEyeY, &gCamChkEyeZ);
    camGetAt((Camera *) w->cam, &gCamChkAtX, &gCamChkAtY, &gCamChkAtZ);
}

#undef sinf

extern Gfx opdemo2CamModel0Dl[];

void opdemo3CamScript(Obj *obj);

/* "OPDEMO3": the third opening demo, OPDEMO2's twin. It loads files 0xBC, 0xC2, 0xC1, 0xF4,
 * 0x13B, 0xC5, 0xC6 and 0xC3, paints a deeper sky, sets the stage model at twice its size, and
 * builds its scene with op3GoeCreate, op3EbiCreate, op3JijiCreate and op3BisuCreate. It then
 * fades in from white and fixes the camera at (256, 161, 7) looking at (469, 168, -4) before
 * op3BeamCreate. Each frame it steps with opdemo3CamScript. */
s32 opdemo3Proc(Obj *obj, s16 msg, void *arg) {
    OpDemoW *w = (OpDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "OPDEMO3";
            w->timer = 0;
            gDmenuLastObj = obj;
            gDemoModelsDelete2 = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0xC2);
            floadLoadSet(0xC1);
            floadLoadSet(0xF4);
            floadLoadSet(0x13B);
            floadLoadSet(0xC5);
            floadLoadSet(0xC6);
            floadLoadSet(0xC3);
            tskCreate(obj, scParentProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 45, 0, 185);
            w->layer3Word = ((u32 *) gFrmLayer3)[6];
            gObjMoveCount = 7;
            demoObjMoveOpen();
            opDemoPlaying = 3;
            w->cam = tskCreate(obj, democam, 0, NULL);
            DPRINTF("カメラを作りました\n");
            camSetFov((Camera *) w->cam, 0.87266463f);
            opDemoView = w->view = camGetView((Camera *) w->cam);
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
            w->text = ((AsciiNewFunc) asciiNew)(gFrmTextRoot, 18, 177, "                    ");
            w->scene = 0;
            demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
            tskCreate(obj, attack_Proc, 0, NULL);
            gDemoModelsDelete2 = 0;
            demoModelCreate(obj, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0xC2, opdemo2CamModel0Dl, 0, 1);
            gCamChkStep = 1.0f;
            gObjMoveStep = 1.0f;
            gCamChkMarkerSize = 0.02f;
            DPRINTF("研究室を出します\n");
            (*op3GoeCreate)(obj);
            (*op3EbiCreate)(obj);
            (*op3JijiCreate)(obj);
            (*op3BisuCreate)(obj);
            camSunSetClockStop(1);
            demoFadeStart(60.0f, 255.0f, 0.0f, 0xFF, 0xFF, 0xFF);
            camSetEye((Camera *) w->cam, 256.0f, 161.0f, 7.0f);
            camSetAt((Camera *) w->cam, 469.0f, 168.0f, -4.0f);
            (*op3BeamCreate)(obj);
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
                (*opdemo3CamScript)(obj);
                sprintf(w->text->str, "DEMOTIMER $ %04d ", w->timer);
            } else if (gGameFrameCount % 60 >= 30) {
                sprintf(w->text->str, "DEMOTIMER $ STOP ");
            } else {
                sprintf(w->text->str, "DEMOTIMER $      ");
            }
            opdemo3Frame = w->timer;
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
            DPRINTF("城を作ります\n");
            DPRINTF("研究室を出します\n");
            DPRINTF("研究室を出します\n");
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}


#define sinf (*sinf)

/* OPDEMO3's camera script, steps by scene against OPDEMO3's frame count (opdemo3Frame), then the
 * camera check follows the camera. 0: the eye sways (sound 0x91 at 20) or flies in until 300,
 * then a fade at 320 and a cut at 330. 1-3: fades and cuts at 503/508, 578/608, 803/808. 4: at
 * 1369 a model, a cut to the far view and four scene builders, sound 0x1B9. 5: at 1900 the stage
 * model again. 6: at 2283 the far view, sound 0x1BA. 7: a rise at 2783, the stage at 2943. 8: a
 * last turn at 3956; at 4186 the fade out and the scene director's end.
 * Step 0's eye flies in from z -302, gaining 0.05 a frame up to 6, then sways about -144 by a
 * sine whose size shrinks 0.01 a frame (its statics are the game's 0x08010948-0x08010954). */
void opdemo3CamScript(Obj *obj) {
    static f32 eyeZ = -302.0f;
    static f32 speed = 2.0f;
    static f32 sway = 1.0f;
    static s32 phase = 1; /* 1: flying in, then swaying */
    OpDemoW *w = (OpDemoW *) (obj + 1);
    s32 pad;

    switch (w->scene) {
        case 0:
            if (opdemo3Frame == 20) {
                sndPlay(0x91);
            }
            if (opdemo3Frame < 300) {
                if (phase == 1) {
                    eyeZ += speed;
                    speed += 0.05;
                    if (speed > 6.0f) {
                        speed = 6.0f;
                    }
                    if (eyeZ > -144.0f) {
                        phase++;
                    }
                } else {
                    eyeZ = sinf(opdemo3Frame * 20) * sway + -144.0f;
                    sway -= 0.01;
                    if (sway < 0.0f) {
                        sway = 0.0f;
                    }
                }
                camSetEye((Camera *) w->cam, 6.0f, 44.0f, eyeZ);
                camSetAt((Camera *) w->cam, 9.0f, 26.0f, -18.0f);
            }
            if (opdemo3Frame == 300) {
                camSetSpeed((Camera *) w->cam, 300.0f);
                camMoveEye((Camera *) w->cam, 6.0f, 44.0f, -144.0f);
                camMoveAt((Camera *) w->cam, 166.0f, 79.0f, -18.0f);
            }
            if (opdemo3Frame == 320) {
                demoFadeStart(7.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (opdemo3Frame == 330) {
                camSetEye((Camera *) w->cam, 256.0f, 161.0f, 7.0f);
                camSetAt((Camera *) w->cam, 469.0f, 168.0f, -4.0f);
                camSetSpeed((Camera *) w->cam, 200.0f);
                demoFadeStart(10.0f, 255.0f, 0.0f, 0, 0, 0);
                (*op3BaraCreate)(obj);
                (*op3FballCreate)(obj);
                w->scene++;
            }
            break;
        case 1:
            if (opdemo3Frame == 503) {
                demoFadeStart(5.0f, 0.0f, 255.0f, 255, 255, 255);
            }
            if (opdemo3Frame == 508) {
                demoFadeStart(5.0f, 255.0f, 0.0f, 255, 255, 255);
                camSetSpeed((Camera *) w->cam, 200.0f);
                camMoveEye((Camera *) w->cam, -60.0f, 13.0f, 6.0f);
                camMoveAt((Camera *) w->cam, 469.0f, 168.0f, 8.0f);
                w->scene++;
            }
            break;
        case 2:
            if (opdemo3Frame == 578) {
                demoFadeStart(30.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (opdemo3Frame == 608) {
                demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 131.0f, 24.0f, 22.0f);
                camSetAt((Camera *) w->cam, 97.0f, 26.0f, -18.0f);
                camSetSpeed((Camera *) w->cam, 200.0f);
                camMoveEye((Camera *) w->cam, 131.0f, 41.0f, 22.0f);
                camMoveAt((Camera *) w->cam, 97.0f, 30.0f, -18.0f);
                w->scene++;
            }
            break;
        case 3:
            if (opdemo3Frame == 803) {
                demoFadeStart(3.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (opdemo3Frame == 808) {
                demoFadeStart(3.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 20.0f, -21.0f, 22.0f);
                camSetAt((Camera *) w->cam, 180.0f, 110.0f, -18.0f);
                camSetSpeed((Camera *) w->cam, 900.0f);
                camMoveEye((Camera *) w->cam, 20.0f, 123.0f, 22.0f);
                w->scene++;
            }
            break;
        case 4:
            if (opdemo3Frame == 1334) {
                demoFadeStart(30.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (opdemo3Frame == 1364) {
                gDemoModelsDelete2 = 1;
            }
            if (opdemo3Frame == 1369) {
                gDemoModelsDelete2 = 0;
                demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
                demoModelCreate(obj, 22.0f, -31.0f, -788.0f, 2.0f, 2.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0xC1, opdemo1CamModel0Dl, 0, 0);
                camSetEye((Camera *) w->cam, -1382.0f, 700.0f, -861.0f);
                camSetAt((Camera *) w->cam, -610.0f, 2055.0f, -1328.0f);
                camSetSpeed((Camera *) w->cam, 400.0f);
                camMoveEye((Camera *) w->cam, -1382.0f, 1033.0f, -861.0f);
                w->scene++;
                (*op3MachineCreate)(obj);
                (*op3BisuElegantCreate)(obj);
                (*op3BeamSparkleCreate)(obj);
                (*op3BeamCreate)(obj);
                sndPlay(0x1B9);
            }
            break;
        case 5:
            if (opdemo3Frame == 1892) {
                demoFadeStart(4.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (opdemo3Frame == 1898) {
                gDemoModelsDelete2 = 1;
            }
            if (opdemo3Frame == 1900) {
                gDemoModelsDelete2 = 0;
                demoFadeStart(6.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, 201.0f, 81.0f, -23.0f);
                camSetAt((Camera *) w->cam, 109.0f, 44.0f, -29.0f);
                camSetSpeed((Camera *) w->cam, 800.0f);
                camMoveEye((Camera *) w->cam, 299.0f, 178.0f, -20.0f);
                camMoveAt((Camera *) w->cam, 107.0f, 44.0f, -28.0f);
                demoModelCreate(obj, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0xC2, opdemo2CamModel0Dl, 0, 1);
                w->scene++;
            }
            break;
        case 6:
            if (opdemo3Frame == 2275) {
                demoFadeStart(5.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (opdemo3Frame == 2281) {
                gDemoModelsDelete2 = 1;
            }
            if (opdemo3Frame == 2283) {
                gDemoModelsDelete2 = 0;
                demoModelCreate(obj, 22.0f, -31.0f, -788.0f, 2.0f, 2.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0xC1, opdemo1CamModel0Dl, 0, 0);
                demoFadeStart(6.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -368.0f, 1789.0f, -861.0f);
                camSetAt((Camera *) w->cam, -443.0f, 1796.0f, -1271.0f);
                camSetSpeed((Camera *) w->cam, 300.0f);
                camMoveEye((Camera *) w->cam, -368.0f, 1224.0f, -861.0f);
                (*op3BeamSparkleCreate)(obj);
                sndPlay(0x1BA);
                w->scene++;
            }
            break;
        case 7:
            if (opdemo3Frame == 2783) {
                camMoveAt((Camera *) w->cam, -443.0f, 2796.0f, -1271.0f);
            }
            if (opdemo3Frame == 2913) {
                demoFadeStart(28.0f, 0.0f, 255.0f, 0, 0, 0);
            }
            if (opdemo3Frame == 2941) {
                gDemoModelsDelete2 = 1;
            }
            if (opdemo3Frame == 2943) {
                gDemoModelsDelete2 = 0;
                demoModelCreate(obj, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0xC2, opdemo2CamModel0Dl, 0, 1);
                demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
                camSetEye((Camera *) w->cam, -151.0f, 72.0f, -14.0f);
                camSetAt((Camera *) w->cam, 190.0f, -22.0f, -8.0f);
                w->scene++;
            }
            break;
        case 8:
            if (opdemo3Frame == 3956) {
                camSetSpeed((Camera *) w->cam, 100.0f);
                camMoveAt((Camera *) w->cam, 100.0f, 0.0f, -177.0f);
            }
            if (opdemo3Frame == 4186) {
                demoFadeStart(60.0f, 0.0f, 255.0f, 0, 0, 0);
                scRunEnd();
            }
            break;
    }
    camGetEye((Camera *) w->cam, &gCamChkEyeX, &gCamChkEyeY, &gCamChkEyeZ);
    camGetAt((Camera *) w->cam, &gCamChkAtX, &gCamChkAtY, &gCamChkAtZ);
}

#undef sinf

extern s32 gDemoForcedFade;
extern s32 gObjMoveSel;

/* "OPMAIN": the opening as the game plays it, the three demos in one task. It sets up what they
 * share (the dragon camera, 9 o'clock, two lights and an ambient, the sky) and plays them as
 * stages (opDemoPlaying): 0 builds the forest (OPDEMO1's scene), 1 runs it (2542 frames), then
 * swaps in OPDEMO2's room for 2 (5012 frames), then OPDEMO3's lab for 3 (4246 frames). Start
 * skips: from 1 it counts frames, at 3 fading out with sound 10, at 40 ending the scene director,
 * at 50 going to the title (stageGotoSubt(0)). The end of stage 3 starts the same sequence at 16.
 * The skip switch sits inside `if (skip != 0)` (the game's plain beqz), and case 40's scRunEnd()
 * comes before its skip++ (the game reloads the count after the call). */
s32 opmainProc(Obj *obj, s16 msg, void *arg) {
    static s32 skip = 1; /* 0, or counting up from 1 once Start is pressed */
    OpDemoW *w = (OpDemoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "OPMAIN";
            w->timer = 0;
            gDmenuLastObj = obj;
            opDemoPlaying = 0;
            opdemo1Frame = opdemo2Frame = opdemo3Frame = 0;
            floadLoadSet(0xBC);
            floadLoadSet(0xC0);
            floadLoadSet(0xC1);
            floadLoadSet(0x11D);
            floadLoadSet(0xC2);
            floadLoadSet(0x13B);
            gDemoForcedFade = 0;
            skip = 0;
            tskCreate(obj, scParentProc, 0, NULL);
            tskCreate(obj, effect, 0, NULL);
            fadeCreate(gEffectTask);
            fadeSetLetterbox(320, 45, 0, 185);
            w->layer3Word = ((u32 *) gFrmLayer3)[6];
            gObjMoveCount = 5;
            gObjMoveSel = -1;
            w->cam = tskCreate(obj, dracam, 0, NULL);
            DPRINTF("カメラを作りました\n");
            camSetFov((Camera *) w->cam, 0.87266463f);
            opDemoView = w->view = camGetView((Camera *) w->cam);
            w->view->zFar = 20000.0f;
            camSunCreate(obj);
            camSunSetView(w->view);
            gTimeOfDay = 9 << 16;
            camSunSetClockStop(1);
            gRetracesPerFrame = 1;
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
            camBgradMoveBottom((ColorFade *) w->sky, 224, 224, 255);
            camBgradMoveTop((ColorFade *) w->sky, 255, 255, 255);
            gDemoModelsDelete = 0;
            gDemoCamHold = 0;
            w->scene = 0;
            demoFadeStart(16.0f, 255.0f, 0.0f, 255, 255, 255);
            tskCreate(obj, attack_Proc, 0, NULL);
            (*op1GoeCreate)(obj);
            (*op1EbiCreate)(obj);
            skip = 0;
            break;
        case 1:
            frmDestroy(w->f0C);
            frmDestroy(&w->text->f);
            frmDestroy(w->f10);
            frmDestroy(w->f04);
            frmDestroy((Frame *) w->view);
            tskDestroy(w->cam);
            tskDestroy(w->sky);
            tskDestroy(w->bg);
            ((u32 *) gFrmLayer3)[6] = w->layer3Word;
            break;
        case 3:
            break;
        case 0:
            if (fadeStartPressed() == 1 && skip == 0) {
                DPRINTF("###################  デモを飛ばします ####################\n");
                skip = 1;
            }
            if (skip != 0) {
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
                        stageGotoSubt(0);
                        return -1;
                    default:
                        skip++;
                        break;
                }
            }
            switch (opDemoPlaying) {
                case 0:
                    DPRINTF("地面を出します\n");
                    demoModelCreate(obj, 0.0f, 0.0f, -400.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0xC0, opdemo1Model0Dl, 0, 1);
                    DPRINTF("樹を出します\n");
                    demoModelCreate(obj, 0.0f, 0.0f, -400.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0xC0, opdemo1Model1Dl, 0, 1);
                    DPRINTF("シダを出します\n");
                    demoModelCreate(obj, 0.0f, 0.0f, -400.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0xC0, opdemo1Model2Dl, 0, 1);
                    DPRINTF("枝を出します\n");
                    demoModelCreate(obj, 0.0f, 0.0f, -400.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0xC0, opdemo1Model3Dl, 0, 1);
                    DPRINTF("枝2を出します\n");
                    demoModelCreate(obj, 0.0f, 0.0f, -400.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0xC0, opdemo1Model4Dl, 0, 1);
                    (*opGateRipplesCreate)(obj);
                    w->bg = cambgCreate(obj, 5, 800.0f);
                    cambgMove(w->bg, 0, 30);
                    gCamChkStep = 1.0f;
                    gObjMoveStep = 1.0f;
                    gCamChkMarkerSize = 0.01f;
                    opDemoPlaying = 1;
                    break;
                case 1:
                    w->timer++;
                    (*opdemo1CamScript)(obj);
                    opdemo1Frame = w->timer;
                    if (opdemo1Frame == 2538) {
                        gDemoModelsDelete2 = 1;
                        tskDestroy(w->bg);
                    }
                    if (opdemo1Frame == 2542) {
                        w->timer = 0;
                        gDemoModelsDelete2 = 0;
                        opDemoPlaying = 2;
                        while (gfxGetPendingSwaps()) {
                        }
                        floadFreeSet(0x11D);
                        floadFreeSet(0xC0);
                        floadLoadSet(0xF4);
                        floadLoadSet(0xC5);
                        floadLoadSet(0xC6);
                        gCamChkEyeX = 0.0f;
                        gCamChkEyeY = 0.0f;
                        gCamChkEyeZ = 0.0f;
                        gCamChkAtX = 0.0f;
                        gCamChkAtY = 1000.0f;
                        gCamChkAtZ = -4000.0f;
                        camBgradSetLines((Camera *) w->sky, 76, 240);
                        camBgradMoveBottom((ColorFade *) w->sky, 224, 224, 255);
                        camBgradMoveTop((ColorFade *) w->sky, 255, 255, 255);
                        w->scene = 0;
                        demoModelCreate(obj, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0xC2, opdemo2ModelDl, 0, 1);
                        gCamChkStep = 1.0f;
                        gObjMoveStep = 1.0f;
                        gCamChkMarkerSize = 0.01f;
                        gObjMoveCount = 6;
                        demoFadeStart(30.0f, 255.0f, 0.0f, 0, 0, 0);
                        DPRINTF("座敷を出します\n");
                        (*op2JijiCreate)(obj);
                        (*op2GoeCreate)(obj);
                        (*op2EbiCreate)(obj);
                    }
                    break;
                case 2:
                    w->timer++;
                    (*opdemo2CamScript)(obj);
                    opdemo2Frame = w->timer;
                    if (opdemo2Frame == 5008) {
                        gDemoModelsDelete2 = 1;
                    }
                    if (opdemo2Frame == 5012) {
                        floadLoadSet(0xC3);
                        w->timer = 0;
                        gDemoModelsDelete2 = 0;
                        gObjMoveCount = 7;
                        opDemoPlaying = 3;
                        gCamChkEyeX = 0.0f;
                        gCamChkEyeY = 0.0f;
                        gCamChkEyeZ = 0.0f;
                        gCamChkAtX = 0.0f;
                        gCamChkAtY = 1000.0f;
                        gCamChkAtZ = -4000.0f;
                        camBgradSetLines((Camera *) w->sky, 36, 240);
                        camBgradMoveTop((ColorFade *) w->sky, 48, 48, 255);
                        camBgradMoveBottom((ColorFade *) w->sky, 128, 128, 255);
                        gDemoModelsDelete = 0;
                        gDemoCamHold = 0;
                        w->scene = 0;
                        gDemoModelsDelete2 = 0;
                        demoModelCreate(obj, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0xC2, opdemo2CamModel0Dl, 0, 1);
                        gCamChkStep = 1.0f;
                        gObjMoveStep = 1.0f;
                        gCamChkMarkerSize = 0.02f;
                        DPRINTF("研究室を出します\n");
                        (*op3GoeCreate)(obj);
                        (*op3EbiCreate)(obj);
                        (*op3JijiCreate)(obj);
                        (*op3BisuCreate)(obj);
                        demoFadeStart(60.0f, 255.0f, 0.0f, 255, 255, 255);
                        camSetEye((Camera *) w->cam, 256.0f, 161.0f, 7.0f);
                        camSetAt((Camera *) w->cam, 469.0f, 168.0f, -4.0f);
                        (*op3BeamCreate)(obj);
                    }
                    break;
                case 3:
                    w->timer++;
                    opdemo3Frame = w->timer;
                    (*opdemo3CamScript)(obj);
                    if (opdemo3Frame == 4246 && skip == 0) {
                        skip = 16;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
