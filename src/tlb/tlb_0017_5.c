#include "tlb.h"
#include "tlb/tlb_0017.h"

/* "sabi"'s work, as its task shows it. */
typedef struct SabiW {
    /* 0x00 */ u8 mode; /* 1 when started with an argument; message 7 sets it */
    /* 0x01 */ u8 state;
    /* 0x02 */ s16 timer; /* ticks in the state */
    /* 0x04 */ u8 look; /* the tube's look and the jet's kind */
    /* 0x05 */ u8 pad05[0x3];
    /* 0x08 */ Frame *light;
    /* 0x0C */ Frame *ambient;
    /* 0x10 */ s32 level; /* the light's colour, 0-255 */
    /* 0x14 */ s32 levelStep; /* added to it each tick */
    /* 0x18 */ Camera *cam; /* "dracam" */
    /* 0x1C */ Obj *jet; /* the jet impact */
    /* 0x20 */ u8 pad20[0x4];
    /* 0x24 */ Obj *humans[4]; /* the demo humans, by player index */
    /* 0x34 */ Obj *eff; /* the tube, the shine, then the lens flare */
    /* 0x38 */ Obj *eff2; /* the light ball, then the afterimage */
    /* 0x3C */ u8 pad3C[0x14];
    /* 0x50 */ Obj *kback; /* effKbackMake's */
} SabiW;

#define SABI(o) ((SabiW *) FX(o))

s32 sabiStep(Obj *obj, SabiW *w);

/* "sabi": one retrace a frame, Goemon's weapon at level 2, the effects, the attack task and the
 * scene director, the dragon camera (40 degrees, planes 5 and 5000), an ambient light and a
 * light with a black light under it, and a black fade, fully in. Started with an argument it
 * sets its mode to 1, else it calls stubReturnZero. Each frame sabiStep runs it;
 * message 7 sets its mode.
 * mode is a u8, as sabiStep reads it (lbu). w is a variable of case 2 alone; the game spills it
 * rather than give it s1. */
s32 sabiProc(Obj *obj, s16 msg, void *arg) {
    Frame *light;
    CamView *view;
    s32 pad[3];

    switch (msg) {
        case 2: {
            SabiW *w;

            obj->name = "sabi";
            gRetracesPerFrame = 1;
            gPlayerStats[0].wpLv = 2;
            tskCreate(obj, effect, 0, NULL);
            tskCreate(obj, attack_Proc, 0, NULL);
            tskCreate(obj, scParentProc, 0, NULL);
            w = SABI(obj);
            w->cam = (Camera *) tskCreate(obj, dracam, 0, NULL);
            w->ambient = frmCreate(gFrmLayers[0], gFrmKindAmbi);
            w->light = frmCreate(gFrmLayers[0], gFrmKindLight);
            light = frmCreate(w->light, gFrmKindLight);
            ((LightFrame *) light)->color[0] = 0;
            ((LightFrame *) light)->color[1] = 0;
            ((LightFrame *) light)->color[2] = 0;
            view = camGetView(w->cam);
            view->zNear = 5.0f;
            view->zFar = 5000.0f;
            camSetFov(w->cam, 0.69813174f);
            fadeCreate(obj);
            fadeSetColor(0, 0, 0);
            fadeSetLevel(255.0f);
            if (arg != NULL) {
                w->mode = 1;
            } else {
                stubReturnZero((s32) obj, 1, 10);
            }
            break;
        }
        case 1:
            gRetracesPerFrame = 2;
            frmDestroy(SABI(obj)->light);
            break;
        case 0:
            sabiStep(obj, SABI(obj));
            break;
        case 7:
            SABI(obj)->mode = (s32) arg;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}


/* "sabi"'s step, the ending: once started (its mode), scene 0x1B8 and a vertical fade in; with
 * mode 1, after 20 ticks: Goemon (demo human 0) rises 500 up a tube as the camera follows him;
 * then the jet impact, shot from ten angles over 360 ticks; then the sky (a blue vertical fade),
 * the shine, a yellow sparkle and a dark puff, two light balls as the light goes dark and back;
 * then the other three heroes appear one by one, Goemon walks among the clouds and lens flare,
 * the others join him; a white fade, the all-cast roll, a black fade and the title.
 * The calls into file 17's other parts are near (jal); the struct temp of floadLoadSet's
 * FileSet is the word pair at sp+0x3C. i is declared first: it takes the frame's top word. Case
 * 7's three colour stores are one chain off w->level, as case 8 writes its 255s. */
s32 sabiStep(Obj *obj, SabiW *w) {
    s32 i;
    f32 x;
    f32 y;
    f32 z;
    Obj *ball;

    w->timer++;
    switch (w->state) {
        case 0:
            if (w->mode != 0) {
                scRunStart(0x1B8, 0, 0);
                gGameState.fade = 0;
                gGameState.fadeFrom.r = 0;
                gGameState.fadeFrom.g = 0;
                gGameState.fadeFrom.b = 0;
                fadeSetSpeed(16.0f);
                fadeSetTarget(0.0f);
                w->timer = 0;
                w->state = 1;
            }
            break;
        case 1:
            if (w->timer >= 20 && w->mode == 1) {
                w->state = 2;
            }
            break;
        case 2:
            floadLoadSet(0x13A);
            floadLoadSet(0x13B);
            w->humans[0] = dhumanMake(obj, 0, 0, 0.0f, 500.0f, 0.0f);
            w->eff = effTubeMake(obj, w->look, 0.0f, 0.0f, 0.0f);
            effTubeSetGlowY(w->eff, 500.0f);
            effTubeSetSteps(w->eff, 240);
            effTubeGlowTo(w->eff, -500.0f);
            dhumanSpin(w->humans[0]);
            dhumanSetPos(w->humans[0], 0.0f, 500.0f, 0.0f);
            dhumanSetScale(w->humans[0], 0.1f);
            dhumanSetSteps(w->humans[0], 240);
            dhumanMoveTo(w->humans[0], 0.0f, -500.0f, 0.0f);
            camSetFov(w->cam, 0.87266463f);
            camSetEye(w->cam, 0.0f, 0.0f, 35.0f);
            camSetAt(w->cam, 0.0f, 500.0f, 0.0f);
            camSetSpeed(w->cam, 1.0f);
            fadeSetTarget(0.0f);
            w->timer = 0;
            w->state = 3;
            break;
        case 3:
            dhumanGetPos(w->humans[0], &x, &y, &z);
            camMoveAt(w->cam, x, y, z);
            if (w->timer == 220) {
                fadeSetSpeed(16.0f);
                fadeSetTarget(255.0f);
            } else if (w->timer == 240) {
                floadLoadSet(0xEF);
                w->timer = 0;
                w->state = 4;
            }
            break;
        case 4:
            w->eff = tskDestroy(w->eff);
            for (i = 0; i < 2; i++) {
                dhumanHide(w->humans[i]);
            }
            w->kback = effKbackMake(obj);
            w->jet = effJetimpactMake(obj, w->look, 0.0f, 0.0f, 0.0f);
            fadeSetSpeed(16.0f);
            fadeSetColor(0, 0, 0);
            fadeSetLevel(255.0f);
            fadeSetTarget(0.0f);
            w->timer = 0;
            w->state = 5;
            break;
        case 5:
            if (w->timer == 1) {
                camSetFov(w->cam, 0.69813174f);
                camSetEye(w->cam, 100.0f, 60.0f, 200.0f);
                camSetAt(w->cam, 0.0f, 0.0f, 0.0f);
                effJetimpactSetSteps(w->jet, 60);
                effJetimpactSetPlay(w->jet, 0.0f, 4.0f);
            } else if (w->timer == 30) {
                camSetAt(w->cam, 0.0f, 20.0f, 0.0f);
                camSetEye(w->cam, 30.0f, 40.0f, -150.0f);
            } else if (w->timer == 60) {
                camSetSpeed(w->cam, 60.0f);
                camMoveEye(w->cam, 30.0f, 20.0f, -180.0f);
            } else if (w->timer == 90) {
                effJetimpactSetSteps(w->jet, 60);
                effJetimpactSetPlay(w->jet, 4.0f, 8.0f);
            } else if (w->timer == 150) {
                floadLoadSet(0x13C);
                camSetFov(w->cam, 0.69813174f);
                camSetAt(w->cam, 0.0f, 0.0f, 20.0f);
                camSetEye(w->cam, 0.0f, 100.0f, 40.0f);
                effJetimpactSetSteps(w->jet, 60);
                effJetimpactSetPlay(w->jet, 8.0f, 16.0f);
            } else if (w->timer == 210) {
                camSetFov(w->cam, 0.69813174f);
                camSetAt(w->cam, 0.0f, 0.0f, 0.0f);
                camSetEye(w->cam, 50.0f, 40.0f, 150.0f);
                effJetimpactSetSteps(w->jet, 30);
                effJetimpactSetPlay(w->jet, 16.0f, 19.0f);
            } else if (w->timer == 240) {
                camSetAt(w->cam, 0.0f, 0.0f, 100.0f);
                camSetEye(w->cam, 100.0f, -20.0f, 120.0f);
                effJetimpactSetSteps(w->jet, 60);
                effJetimpactSetPlay(w->jet, 19.0f, 25.0f);
            } else if (w->timer == 300) {
                camSetFov(w->cam, 0.87266463f);
                effJetimpactSetSteps(w->jet, 30);
                effJetimpactSetPlay(w->jet, 28.0f, 33.0f);
                camSetAt(w->cam, 0.0f, 0.0f, -70.0f);
                camSetEye(w->cam, 150.0f, 0.0f, -10.0f);
            } else if (w->timer == 330) {
                effJetimpactSetPlay(w->jet, 33.0f, 38.0f);
                camSetEye(w->cam, -150.0f, 0.0f, -10.0f);
            } else if (w->timer == 360) {
                w->state = 6;
            }
            break;
        case 6:
            floadLoadSet(0xF4);
            w->jet = tskDestroy(w->jet);
            w->kback = tskDestroy(w->kback);
            gGameState.fade = 1;
            gGameState.fadeFrom.r = 0x40;
            gGameState.fadeFrom.g = 0x60;
            gGameState.fadeFrom.b = 0xE0;
            gGameState.fadeStart = 0x14;
            gGameState.fadeTo[0] = 0x50;
            gGameState.fadeTo[1] = 0xE0;
            gGameState.fadeTo[2] = 0xFF;
            gGameState.fadeEnd = 0xA0;
            camSetFov(w->cam, 0.7853982f);
            camSetAt(w->cam, 0.0f, 1540.0f, 0.0f);
            camSetEye(w->cam, 0.0f, 1600.0f, 1000.0f);
            w->eff = effKagayakeMake(obj);
            effKagayakeSetPlay(w->eff, 330.0f, 0.0f, 74.0f);
            w->level = 255;
            w->timer = 0;
            w->state = 7;
            break;
        case 7:
            if (w->timer == 141) {
                itexSparkleYellow(296.0f, 1254.0f, -1622.0f, 3.0f);
            } else if (w->timer == 145) {
                itexPuffDark(360.0f, 863.0f, -4352.0f, 3.0f);
            } else if (w->timer == 194) {
                ball = effLballMake(w->eff, 0.0f, 1350.0f, -3000.0f);
                effLballSetSteps(ball, 16);
                effLballSetScale(ball, 0.0f);
                effLballScaleTo(ball, 60.0f);
                effLballSetAlpha(ball, 255);
                effLballFadeTo(ball, 0);
                w->eff2 = effLballMake(obj, 0.0f, 1350.0f, -3000.0f);
                effLballSetSteps(w->eff2, 60);
                effLballSetAlpha(w->eff2, 255);
                effLballSetScale(w->eff2, 0.0f);
                effLballScaleTo(w->eff2, 80.0f);
                w->levelStep = -8;
            } else if (w->timer == 300) {
                effLballSetSteps(w->eff2, 60);
                effLballFadeTo(w->eff2, 0);
                w->levelStep = 8;
            } else if (w->timer == 360) {
                w->state = 8;
            }
            w->level += w->levelStep;
            if (w->level < 0) {
                w->level = 0;
            } else if (w->level > 255) {
                w->level = 255;
            }
            ((LightFrame *) w->light)->color[0] = ((LightFrame *) w->light)->color[1] =
                ((LightFrame *) w->light)->color[2] = w->level;
            break;
        case 8:
            ((LightFrame *) w->light)->color[0] = ((LightFrame *) w->light)->color[1] =
                ((LightFrame *) w->light)->color[2] = 255;
            w->eff = tskDestroy(w->eff);
            w->eff2 = tskDestroy(w->eff2);
            camSetFov(w->cam, 0.7853982f);
            camSetAt(w->cam, -0.7f, 13.0f, 1.5f);
            camSetEye(w->cam, 15.5f, 13.5f, 15.5f);
            w->humans[1] = dhumanMake(obj, 1, 1, 0.0f, 0.0f, 0.0f);
            dhumanPlayAnime11F(w->humans[1]);
            w->timer = 0;
            w->state = 9;
            break;
        case 9:
            if (w->timer == 30) {
                dhumanHide(w->humans[1]);
                camSetAt(w->cam, 2.0f, 14.0f, -1.5f);
                camSetEye(w->cam, 5.0f, 15.0f, 18.5f);
                w->humans[3] = dhumanMake(obj, 3, 3, 0.0f, 0.0f, 0.0f);
                dhumanPlayAnime11F(w->humans[3]);
            } else if (w->timer == 60) {
                dhumanHide(w->humans[3]);
                camSetAt(w->cam, 0.0f, 10.5f, 0.0f);
                camSetEye(w->cam, 0.0f, 57.0f, 50.0f);
                w->humans[2] = dhumanMake(obj, 2, 2, 0.0f, 0.0f, 0.0f);
                dhumanPlayAnime11F(w->humans[2]);
            } else if (w->timer == 90) {
                fadeSetSpeed(20.0f);
                fadeSetColor(255, 255, 255);
                fadeSetTarget(255.0f);
            } else if (w->timer == 110) {
                dhumanHide(w->humans[2]);
                w->timer = 0;
                w->state = 10;
            }
            break;
        case 10:
            if (w->timer == 1) {
                fadeSetSpeed(20.0f);
                fadeSetTarget(0.0f);
                dhumanShow(w->humans[0]);
                dhumanSetSteps(w->humans[0], 120);
                dhumanSetPos(w->humans[0], 0.0f, 120.0f, 0.0f);
                dhumanSetDir(w->humans[0], 0.0f);
                dhumanMoveTo(w->humans[0], 0.0f, 0.0f, 0.0f);
                dhumanPlayAnime120(w->humans[0]);
                camSetFov(w->cam, 0.87266463f);
                camSetAt(w->cam, 0.0f, 92.0f, 0.0f);
                camSetEye(w->cam, 0.0f, 19.5f, 61.0f);
                camSetSpeed(w->cam, 120.0f);
                camMoveAt(w->cam, 0.0f, 21.0f, 0.0f);
                camMoveEye(w->cam, 0.0f, 17.5f, 46.5f);
                w->eff = effLensflareMake(obj, 0.0f, 1000.0f, -400.0f);
                effCloudMake(w->eff, 100.0f, 200.0f, -200.0f);
                effCloudMake(w->eff, -100.0f, 500.0f, -400.0f);
                effCloudMake(w->eff, 200.0f, 400.0f, -300.0f);
                effCloudMake(w->eff, -200.0f, 500.0f, -500.0f);
                effCloudMake(w->eff, 400.0f, 300.0f, -400.0f);
                effCloudMake(w->eff, -300.0f, 300.0f, -300.0f);
            } else if (w->timer == 120) {
                dhumanSetSteps(w->humans[0], 60);
                dhumanPlayAnime121(w->humans[0]);
                camSetSpeed(w->cam, 60.0f);
                camMoveAt(w->cam, 0.0f, 13.0f, 0.0f);
                camMoveEye(w->cam, 0.0f, 14.0f, 23.0f);
            } else if (w->timer == 180) {
                w->eff = tskDestroy(w->eff);
                w->timer = 0;
                w->state = 11;
            }
            break;
        case 11:
            if (w->timer == 1) {
                w->eff2 = effGhostMake(obj, 0x80);
                dhumanShow(w->humans[3]);
                dhumanSetPos(w->humans[3], 50.0f, 0.0f, 50.0f);
                dhumanSetDir(w->humans[3], 0.0f);
                dhumanSetSteps(w->humans[3], 60);
                dhumanMoveTo(w->humans[3], 0.0f, 0.0f, 0.0f);
                dhumanPlayAnime121(w->humans[3]);
                camSetSpeed(w->cam, 180.0f);
                camMoveFov(w->cam, 0.9599311f);
                camMoveAt(w->cam, 0.0f, 23.5f, 0.0f);
                camMoveEye(w->cam, 0.0f, 21.5f, 52.5f);
            } else if (w->timer == 60) {
                dhumanShow(w->humans[1]);
                dhumanSetPos(w->humans[1], -50.0f, 0.0f, 50.0f);
                dhumanSetDir(w->humans[1], 0.0f);
                dhumanSetSteps(w->humans[1], 60);
                dhumanMoveTo(w->humans[1], 0.0f, 0.0f, 0.0f);
                dhumanPlayAnime121(w->humans[1]);
                dhumanShow(w->humans[2]);
                dhumanSetPos(w->humans[2], -50.0f, 0.0f, 50.0f);
                dhumanSetDir(w->humans[2], 0.0f);
                dhumanSetSteps(w->humans[2], 60);
                dhumanMoveTo(w->humans[2], 0.0f, 0.0f, 0.0f);
                dhumanPlayAnime121(w->humans[2]);
            } else if (w->timer == 210) {
                tskDestroy(w->eff2);
                fadeSetAll(20.0f, 0.0f, 255.0f, 255, 255, 255);
                floadLoadSet(0xC5);
            } else if (w->timer == 230) {
                w->timer = 0;
                w->state = 12;
            }
            break;
        case 12:
            if (w->timer == 1) {
                floadLoadSet(0xC7);
                effAllcastMake(obj);
                fadeSetSpeed(40.0f);
                fadeSetTarget(0.0f);
            } else if (w->timer == 220) {
                fadeSetSpeed(32.0f);
                fadeSetColor(0, 0, 0);
                fadeSetTarget(255.0f);
            } else if (w->timer == 260) {
                stageGotoTitle();
            }
            break;
    }
    return 0;
}
