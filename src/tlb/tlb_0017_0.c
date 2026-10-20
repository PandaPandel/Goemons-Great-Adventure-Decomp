#include "tlb.h"
#include "tlb/tlb_0017.h"

/* "geton"'s work. */
typedef struct GetonW {
    /* 0x00 */ s32 (*step)(Obj *obj, struct GetonW *w);
    /* 0x04 */ u8 kind; /* 2 in area 0, else 1, when started with an argument; message 7 sets it */
    /* 0x05 */ u8 state; /* getonScript's script step */
    /* 0x06 */ s16 event; /* frames in the step */
    /* 0x08 */ u8 single; /* one rider only */
    /* 0x09 */ u8 pad09[0x3];
    /* 0x0C */ LightFrame *light;
    /* 0x10 */ AmbFrame *ambi;
    /* 0x14 */ Camera *cam; /* the dracam task's */
    /* 0x18 */ Obj *mechs[2]; /* the riders' machines (effJetimpactMake, effDimpactMake) */
    /* 0x20 */ Obj *players[2];
    /* 0x28 */ Obj *fx[8]; /* the scene's effects; fx[7] the hangar (effKbackMake) */
} GetonW;

s32 getonScript(Obj *obj, GetonW *w);

/* "geton", the players getting on: one retrace a frame, the effect and attack tasks, sets
 * 0x13A, 0x13B, 0x125, 0xEA and 0xF4, and the first two players of the save that have a
 * character (each at 500 back), or Goemon if none has. Then the scenario and dracam tasks,
 * an ambient light and a black light, the camera (40 degrees, planes 5 and 5000) and a
 * letterboxed black fade, fully in. getonScript is its first step; message 7 sets
 * its kind. w is set first, before the calls: that leaves the game's unused stack word. */
s32 getonProc(Obj *obj, s16 msg, void *arg) {
    GetonW *w;
    LightFrame *light;
    CamView *view;
    s32 i;
    s32 n;
    s32 chara;

    switch (msg) {
        case 2:
            w = (GetonW *) FX(obj);
            obj->name = "geton";
            gRetracesPerFrame = 1;
            tskCreate(obj, effect, 0, NULL);
            floadLoadSet(0x13A);
            floadLoadSet(0x13B);
            floadLoadSet(0x125);
            floadLoadSet(0xEA);
            floadLoadSet(0xF4);
            tskCreate(obj, attack_Proc, 0, NULL);
            w = (GetonW *) FX(obj);
            for (i = 0, n = 0; i < 4; i++) {
                chara = gSaveData.players[i].chara;
                if (chara >= 0 && chara < 4) {
                    w->players[n] = dhumanMake(obj, i, chara, 0.0f, -500.0f, 0.0f);
                    dhumanHide(w->players[n]);
                    n++;
                    if (n >= 2) {
                        break;
                    }
                }
            }
            if (n == 0) {
                w->players[0] = dhumanMake(obj, 0, 0, 0.0f, -500.0f, 0.0f);
            }
            tskCreate(obj, scParentProc, 0, NULL);
            w->cam = (Camera *) tskCreate(obj, dracam, 0, NULL);
            w->ambi = (AmbFrame *) frmCreate(gFrmLayers[0], gFrmKindAmbi);
            w->light = (LightFrame *) frmCreate(gFrmLayers[0], gFrmKindLight);
            light = (LightFrame *) frmCreate(&w->light->f, gFrmKindLight);
            light->color[0] = 0;
            light->color[1] = 0;
            light->color[2] = 0;
            view = camGetView(w->cam);
            view->zNear = 5.0f;
            view->zFar = 5000.0f;
            camSetFov(w->cam, 0.69813174f);
            fadeCreate(obj);
            fadeSetLetterbox(320, 38, 0, 190);
            fadeSetColor(0, 0, 0);
            fadeSetLevel(255.0f);
            if (arg != NULL) {
                if (gGameWork.area == 0) {
                    w->kind = 2;
                } else {
                    w->kind = 1;
                }
            } else {
                stubReturnZero((s32) obj, 1, 10);
            }
            w->step = getonScript;
            break;
        case 1:
            gRetracesPerFrame = 2;
            frmDestroy(&((GetonW *) FX(obj))->light->f);
            break;
        case 0:
            do {
                w = (GetonW *) FX(obj);
            } while (0);
            w->event++;
            w->step(obj, w);
            break;
        case 7:
            ((GetonW *) FX(obj))->kind = (s32) arg;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* What the scenario hands its callbacks (the callbacks are listed in sctable): only the event
 * read below. */
typedef struct Tlb17ScArg {
    /* 0x00 */ u8 pad00[0x6];
    /* 0x06 */ s16 event;
} Tlb17ScArg;

/* A scenario callback: on event 1 it fades out to black with sound 10; on event 0x24 it goes to
 * the battle after the area's. */
s32 getonScCallback(Obj *o, Tlb17ScArg *a) {
    if (a->event == 1) {
        fadeSetSpeed(32.0f);
        fadeSetColor(0, 0, 0);
        fadeSetTarget(255.0f);
        sndPlay(10);
    } else if (a->event == 0x24) {
        stageGotoBattle(gGameWork.area + 1);
    }
    return 0;
}


/* geton's script: the riders boarding Impact, shot by shot on the step's frame count. Start (held
 * for one frame by any player in play, past area 0) skips to the fade-out callback.
 * - Steps 0-1: the vertical fade's lines, a second rider if needed, a fade in; then the kind
 *   picks the opening (2, 10, 14 or 17; even kinds with one rider).
 * - 2-9: the sparkles, the machines flying in from -2000 and up to the hangar, sounds 0x1B3 and
 *   0x75.
 * - 10-13: the riders looking up as they are drawn in (scene 0x1B9).
 * - 14-16: the cockpit's assembly in two versions (one rider or two), cut by cut.
 * - 17-26: Impact's head lighting up, two white flashes, sparks, and the fade to the battle (or
 *   to demo 4 in area 0).
 * Step 12's first destroy loop is on one source line (IDO schedules by line) and its second is a
 * while loop: the game's store order. */
s32 getonScript(Obj *obj, GetonW *w);
s32 getonScript(Obj *obj, GetonW *w) {
    s32 i;
    f32 z;
    f32 y;
    f32 x;

    if (gGameWork.area > 0) {
        for (i = 0; i < 4; i++) {
            if (gSaveData.players[i].chara >= 0 && gSaveData.players[i].lives > 0
                && joyGetHeldTime(i, 0x1000) == 1) {
                w->state = 0;
                w->event = 0;
                w->step = (s32 (*)(Obj *, GetonW *)) getonScCallback;
                return 0;
            }
        }
    }
    switch (w->state) {
        case 0:
            if (w->kind != 0) {
                gGameState.fade = 1;
                gGameState.fadeFrom.r = 0x40;
                gGameState.fadeFrom.g = 0x60;
                gGameState.fadeFrom.b = 0xE0;
                gGameState.fadeStart = 0x14;
                gGameState.fadeTo[0] = 0x80;
                gGameState.fadeTo[1] = 0xC0;
                gGameState.fadeTo[2] = 0xFF;
                gGameState.fadeEnd = 0xC8;
                if (w->kind == 2) {
                    Obj *t = w->players[0];

                    w->players[0] = w->players[1];
                    w->players[1] = t;
                    if (w->players[0] == NULL) {
                        if (gPlayer1Chara == 0) {
                            w->players[0] = dhumanMake(obj, 1, 1, 0.0f, -500.0f, 0.0f);
                        } else {
                            w->players[0] = dhumanMake(obj, 1, 0, 0.0f, -500.0f, 0.0f);
                        }
                        dhumanHide(w->players[0]);
                    }
                } else if (w->kind >= 3) {
                    gGameState.fade = 0;
                    gGameState.fadeFrom.r = 0;
                    gGameState.fadeFrom.g = 0;
                    gGameState.fadeFrom.b = 0;
                }
                fadeSetSpeed(16.0f);
                fadeSetTarget(0.0f);
                w->event = 0;
                w->state = 1;
            }
            break;
        case 1:
            if (w->event >= 20) {
                switch (w->kind) {
                    case 1:
                        w->state = 2;
                        break;
                    case 2:
                        w->single = 1;
                        w->state = 2;
                        break;
                    case 3:
                        w->state = 10;
                        break;
                    case 4:
                        w->single = 1;
                        w->state = 10;
                        break;
                    case 5:
                        w->state = 14;
                        break;
                    case 6:
                        w->single = 1;
                        w->state = 14;
                        break;
                    case 7:
                        w->state = 17;
                        break;
                    case 8:
                        w->single = 1;
                        w->state = 17;
                        break;
                }
            }
            break;
        case 2:
            sndPlay(1);
            sndPlay(0x28);
            z = 0.0f;
            y = 0.0f;
            x = -2000.0f;
            camSetSpeed(w->cam, 4.0f);
            camSetEye(w->cam, 100.0f, 0.0f, 0.0f);
            camSetAt(w->cam, z, y, x);
            w->fx[0] = effKirariMake(obj, z, y, x);
            effKirariSetSteps(w->fx[0], 20.0f);
            effKirariSetScale(w->fx[0], 0.0f);
            effKirariScaleTo(w->fx[0], 0.05f);
            if (w->single == 0) {
                w->fx[2] = effKirariMake(obj, z - 150.0f, y, x - 50.0f);
                effKirariSetSteps(w->fx[2], 20.0f);
                effKirariSetScale(w->fx[2], 0.0f);
                effKirariScaleTo(w->fx[2], 0.05f);
            }
            w->event = 0;
            w->state = 3;
            break;
        case 3:
            if (w->event >= 26) {
                effKirariSetSteps(w->fx[0], 20.0f);
                effKirariScaleTo(w->fx[0], 0.0f);
                if (w->fx[2] != NULL) {
                    effKirariSetSteps(w->fx[2], 20.0f);
                    effKirariScaleTo(w->fx[2], 0.0f);
                }
                w->state = 6;
            }
            break;
        case 6:
            w->fx[1] = effShowerMake(obj, 0.0f, 0.0f, 7500.0f, 0.0f);
            effShowerSetSteps(w->fx[1], 60.0f);
            effShowerSetScale(w->fx[1], 0.1f, 0.1f, 1.0f);
            effShowerScaleTo(w->fx[1], 2.0f);
            w->mechs[0] = effJetimpactMake(obj, w->single, 0.0f, 0.0f, -2000.0f);
            effJetimpactSetScale(w->mechs[0], 0.0f);
            effJetimpactSetSteps(w->mechs[0], 120);
            effJetimpactScaleTo(w->mechs[0], 1.0f);
            if (w->single == 0) {
                w->mechs[1] = effJetimpactMake(obj, w->single ^ 1, -150.0f, 0.0f, -2050.0f);
                effJetimpactSetScale(w->mechs[1], 0.0f);
                effJetimpactSetSteps(w->mechs[1], 120);
                effJetimpactScaleTo(w->mechs[1], 1.0f);
            }
            w->event = 0;
            w->state = 7;
            break;
        case 7:
            effJetimpactGetPos(w->mechs[0], &z, &y, &x);
            camMoveAt(w->cam, z, y, x);
            if (w->event == 60) {
                effShowerSetScroll(w->fx[1], 10);
            } else if (w->event == 120) {
                effJetimpactAddItexs(w->mechs[0]);
                effJetimpactSetSteps(w->mechs[0], 240);
                effJetimpactMoveTo(w->mechs[0], 0.0f, 0.0f, 2000.0f);
                if (w->mechs[1] != NULL) {
                    effJetimpactAddItexs(w->mechs[1]);
                    effJetimpactSetSteps(w->mechs[1], 240);
                    effJetimpactMoveTo(w->mechs[1], -150.0f, 0.0f, 1950.0f);
                }
            } else if (w->event >= 301) {
                for (i = 0; i < 8; i++) {
                    w->fx[i] = tskDestroy(w->fx[i]);
                }
                w->state = 8;
            }
            if (w->event < 240 && (w->event & 0x1F) == 1) {
                Obj *star = effCloudMake(obj, fadeRand(200) - 50, fadeRand(200) - 100, 200.0f);

                effCloudSetScale(star, fadeRand(8) + 12, fadeRand(8) + 12, 1.0f);
                effCloudFadeOut(star, 60);
                effCloudMoveTo(star, 0.0f, 0.0f, -2000.0f);
            }
            break;
        case 8:
            sndPlay(0x1B3);
            w->fx[2] = effLensflareMake(obj, -150.0f, 1000.0f, -400.0f);
            effJetimpactSetPos(w->mechs[0], 0.0f, 500.0f, -2000.0f);
            effJetimpactSetSteps(w->mechs[0], 60);
            effJetimpactMoveTo(w->mechs[0], 0.0f, 500.0f, -200.0f);
            effJetimpactGetPos(w->mechs[0], &z, &y, &x);
            if (w->mechs[1] != NULL) {
                effJetimpactSetPos(w->mechs[1], -150.0f, 500.0f, -2050.0f);
                effJetimpactSetSteps(w->mechs[1], 60);
                effJetimpactMoveTo(w->mechs[1], -150.0f, 500.0f, -200.0f);
            }
            camSetSpeed(w->cam, 1.0f);
            camSetEye(w->cam, 100.0f, 0.0f, 0.0f);
            camSetAt(w->cam, z, y, x);
            w->event = 0;
            w->state = 9;
            break;
        case 9:
            effJetimpactSetSteps(w->mechs[0], 60);
            effJetimpactMoveTo(w->mechs[0], 0.0f, 500.0f, -200.0f);
            effJetimpactGetPos(w->mechs[0], &z, &y, &x);
            camMoveAt(w->cam, z, y, x);
            if (w->mechs[1] != NULL) {
                effJetimpactSetSteps(w->mechs[1], 60);
                effJetimpactMoveTo(w->mechs[1], -150.0f, 500.0f, -200.0f);
            }
            if (w->event == 108) {
                sndPlay(0x75);
            }
            if (w->event == 159) {
                (*effLensflareSetFade)(w->fx[2], -15);
            } else if (w->event == 300) {
                w->fx[2] = tskDestroy(w->fx[2]);
                effJetimpactFadeItexs(w->mechs[0]);
                if (w->mechs[1] != NULL) {
                    effJetimpactFadeItexs(w->mechs[1]);
                }
            } else if (w->event == 360) {
                effJetimpactStop(w->mechs[0]);
                if (w->mechs[1] != NULL) {
                    effJetimpactStop(w->mechs[1]);
                }
                w->state = 10;
            }
            break;
        case 10:
            sndPlay(0x1B4);
            if (w->mechs[0] == NULL) {
                w->mechs[0] = effJetimpactMake(obj, w->single, 0.0f, 500.0f, -200.0f);
            }
            if (w->mechs[1] == NULL && w->single == 0) {
                w->mechs[1] = effJetimpactMake(obj, w->single ^ 1, -150.0f, 500.0f, -200.0f);
            }
            effJetimpactGetPos(w->mechs[0], &z, &y, &x);
            camSetEye(w->cam, 100.0f, 0.0f, 0.0f);
            camSetAt(w->cam, z, y, x);
            w->fx[0] = effTractorMake(obj, w->single, z, y - 10.0f, x);
            effTractorAimAt(w->fx[0], 90.0f, 0.0f, -20.0f);
            effTractorSetScale(w->fx[0], 0.02f, 0.0f, 0.02f);
            effTractorSetSteps(w->fx[0], 60.0f);
            effTractorScaleTo(w->fx[0], 0.02f, 0.1f, 0.02f);
            if (w->mechs[1] != NULL && w->players[1] != NULL) {
                effJetimpactGetPos(w->mechs[1], &z, &y, &x);
                w->fx[1] = effTractorMake(obj, w->single ^ 1, z, y - 10.0f, x);
                effTractorAimAt(w->fx[1], z + 200.0f, y - 500.0f, x + 150.0f);
                effTractorSetScale(w->fx[1], 0.02f, 0.0f, 0.02f);
                effTractorSetSteps(w->fx[1], 60.0f);
                effTractorScaleTo(w->fx[1], 0.02f, 0.1f, 0.02f);
            }
            w->event = 0;
            w->state = 11;
            break;
        case 11:
            if (w->event == 60) {
                dhumanShow(w->players[0]);
                dhumanSetPos(w->players[0], 88.0f, 0.0f, -24.0f);
                dhumanSetSteps(w->players[0], 30);
                dhumanSetScale(w->players[0], 0.1f);
                dhumanMoveTo(w->players[0], 88.0f, 30.0f, -24.0f);
                if (w->fx[1] != NULL) {
                    effJetimpactGetPos(w->mechs[1], &z, &y, &x);
                    dhumanShow(w->players[1]);
                    dhumanSetPos(w->players[1], z + 200.0f, y - 500.0f, x + 140.0f);
                    dhumanSetSteps(w->players[1], 180);
                    dhumanSetScale(w->players[1], 0.2f);
                    dhumanMoveTo(w->players[1], z, y - 10.0f, x);
                    dhumanScaleTo(w->players[1], 0.05f);
                }
            } else if (w->event == 90) {
                effJetimpactGetPos(w->mechs[0], &z, &y, &x);
                dhumanSetSteps(w->players[0], 240);
                dhumanMoveTo(w->players[0], z, y, x);
                dhumanScaleTo(w->players[0], 0.01f);
            } else if (w->event == 240) {
                effTractorSetSteps(w->fx[0], 60.0f);
                effTractorScaleTo(w->fx[0], 0.0f, 0.1f, 0.0f);
                if (w->fx[1] != NULL) {
                    effTractorSetSteps(w->fx[1], 60.0f);
                    effTractorScaleTo(w->fx[1], 0.0f, 0.1f, 0.0f);
                }
            } else if (w->event == 330) {
                fadeSetSpeed(32.0f);
                fadeSetColor(0, 0, 0);
                fadeSetTarget(255.0f);
            } else if (w->event == 370) {
                gGameState.fade = 0;
                gGameState.fadeFrom.r = 0;
                gGameState.fadeFrom.g = 0;
                gGameState.fadeFrom.b = 0;
                w->event = 0;
                w->state = 12;
            }
            break;
        case 12:
            scRunStart(0x1B9, 0, 0);
            for (i = 0; i < 2; i++) { w->mechs[i] = tskDestroy(w->mechs[i]); }
            i = 0;
            while (i < 8) {
                w->fx[i] = tskDestroy(w->fx[i]);
                i++;
            }
            w->fx[0] = effTubeMake(obj, w->single, 0.0f, 0.0f, 0.0f);
            effTubeSetGlowY(w->fx[0], 500.0f);
            effTubeSetSteps(w->fx[0], 240);
            effTubeGlowTo(w->fx[0], -500.0f);
            dhumanSpin(w->players[0]);
            dhumanSetPos(w->players[0], 0.0f, 500.0f, 0.0f);
            dhumanSetScale(w->players[0], 0.1f);
            dhumanSetSteps(w->players[0], 240);
            dhumanMoveTo(w->players[0], 0.0f, -500.0f, 0.0f);
            camSetFov(w->cam, 0.87266463f);
            camSetEye(w->cam, 0.0f, 0.0f, 35.0f);
            camSetAt(w->cam, 0.0f, 500.0f, 0.0f);
            camSetSpeed(w->cam, 1.0f);
            fadeSetTarget(0.0f);
            w->event = 0;
            w->state = 13;
            break;
        case 13:
            dhumanGetPos(w->players[0], &z, &y, &x);
            camMoveAt(w->cam, z, y, x);
            if (w->event == 200) {
                fadeSetTarget(255.0f);
            } else if (w->event == 240) {
                w->event = 0;
                w->state = 14;
            }
            break;
        case 14:
            w->fx[0] = tskDestroy(w->fx[0]);
            for (i = 0; i < 2; i++) {
                dhumanHide(w->players[i]);
            }
            w->fx[7] = effKbackMake(obj);
            w->mechs[0] = effJetimpactMake(obj, w->single, 0.0f, 0.0f, 0.0f);
            fadeSetSpeed(32.0f);
            fadeSetColor(0, 0, 0);
            fadeSetLevel(255.0f);
            fadeSetTarget(0.0f);
            w->event = 0;
            if (w->single == 0) {
                w->state = 15;
            } else {
                w->state = 16;
            }
            break;
        case 15:
            if (w->event == 1) {
                camSetFov(w->cam, 0.69813174f);
                camSetEye(w->cam, 100.0f, 60.0f, 200.0f);
                camSetAt(w->cam, 0.0f, 0.0f, 0.0f);
            } else if (w->event == 60) {
                effJetimpactSetSteps(w->mechs[0], 120);
                effJetimpactSetPlay(w->mechs[0], 0.0f, 4.0f);
            } else if (w->event == 120) {
                camSetAt(w->cam, 0.0f, 20.0f, 0.0f);
                camSetEye(w->cam, 30.0f, 40.0f, -150.0f);
            } else if (w->event == 210) {
                camSetSpeed(w->cam, 120.0f);
                camMoveEye(w->cam, 30.0f, 20.0f, -180.0f);
            } else if (w->event == 240) {
                effJetimpactSetSteps(w->mechs[0], 180);
                effJetimpactSetPlay(w->mechs[0], 4.0f, 8.0f);
            } else if (w->event == 420) {
                effJetimpactHide(w->mechs[0]);
                effKbackHide(w->fx[7]);
                camSetFov(w->cam, 0.87266463f);
                camSetAt(w->cam, 0.0f, 13.0f, 0.0f);
                camSetEye(w->cam, -120.0f, 8.0f, 20.0f);
                w->fx[0] = effGim2Make(obj);
                effGim2SetPlay(w->fx[0], 180.0f, 0.0f, 59.0f);
                dhumanShow(w->players[0]);
                dhumanSetAction13A(w->players[0]);
                effGim2SetRider(w->fx[0], w->players[0]);
            } else if (w->event == 630) {
                tskDestroy(w->fx[0]);
                effJetimpactShow(w->mechs[0]);
                effKbackShow(w->fx[7]);
                dhumanHide(w->players[0]);
                camSetFov(w->cam, 0.69813174f);
                camSetAt(w->cam, 0.0f, 0.0f, 20.0f);
                camSetEye(w->cam, 0.0f, 100.0f, 40.0f);
            } else if (w->event == 660) {
                effJetimpactSetSteps(w->mechs[0], 60);
                effJetimpactSetPlay(w->mechs[0], 8.0f, 10.0f);
            } else if (w->event == 720) {
                effJetimpactSetSteps(w->mechs[0], 120);
                effJetimpactSetPlay(w->mechs[0], 10.0f, 16.0f);
                camSetSpeed(w->cam, 120.0f);
                camMoveFov(w->cam, 0.87266463f);
            } else if (w->event == 840) {
                camSetAt(w->cam, 0.0f, 0.0f, 0.0f);
                camSetEye(w->cam, 50.0f, 40.0f, 150.0f);
                camSetFov(w->cam, 0.69813174f);
            } else if (w->event == 870) {
                effJetimpactSetSteps(w->mechs[0], 120);
                effJetimpactSetPlay(w->mechs[0], 16.0f, 19.0f);
            } else if (w->event == 950) {
                effKbackSparkleMake(obj, 15.0f, 0.0f, 40.0f, 0.2f);
                effKbackSparkleMake(obj, -15.0f, 0.0f, 40.0f, 0.2f);
            } else if (w->event == 990) {
                camSetSpeed(w->cam, 180.0f);
                camMoveEye(w->cam, 40.0f, 0.0f, 240.0f);
            } else if (w->event == 1170) {
                effJetimpactSetSteps(w->mechs[0], 40);
                effJetimpactSetPlay(w->mechs[0], 19.0f, 22.0f);
            } else if (w->event == 1230) {
                effJetimpactSetPlay(w->mechs[0], 22.0f, 25.0f);
            } else if (w->event == 1290) {
                effJetimpactHide(w->mechs[0]);
                effKbackHide(w->fx[7]);
                camSetFov(w->cam, 0.87266463f);
                camSetAt(w->cam, -20.0f, -25.0f, 17.0f);
                camSetEye(w->cam, -340.0f, 25.0f, 109.0f);
                w->fx[0] = effGimMake(obj);
            } else if (w->event == 1320) {
                effGimSetPlay(w->fx[0], 120.0f, 0.0f, 35.0f);
            } else if (w->event == 1470) {
                tskDestroy(w->fx[0]);
                effJetimpactShow(w->mechs[0]);
                effKbackShow(w->fx[7]);
                camSetFov(w->cam, 0.69813174f);
                camSetAt(w->cam, 0.0f, 0.0f, 20.0f);
                camSetEye(w->cam, 0.0f, 150.0f, 40.0f);
                effJetimpactSetSteps(w->mechs[0], 60);
                effJetimpactSetPlay(w->mechs[0], 25.0f, 28.0f);
            } else if (w->event == 1560) {
                camSetFov(w->cam, 0.87266463f);
                effJetimpactSetSteps(w->mechs[0], 60);
                effJetimpactSetPlay(w->mechs[0], 28.0f, 33.0f);
                camSetAt(w->cam, 0.0f, 0.0f, -70.0f);
                camSetEye(w->cam, 150.0f, 0.0f, -10.0f);
            } else if (w->event == 1600) {
                effKbackSparkleMake(obj, 30.0f, 0.0f, -32.0f, 0.2f);
            } else if (w->event == 1650) {
                effJetimpactSetPlay(w->mechs[0], 33.0f, 38.0f);
                camSetEye(w->cam, -150.0f, 0.0f, -10.0f);
            } else if (w->event == 1690) {
                effKbackSparkleMake(obj, -30.0f, 0.0f, -32.0f, 0.2f);
            } else if (w->event == 1740) {
                effJetimpactSetTilt(w->mechs[0], 0.0f);
                camSetFov(w->cam, 1.0471976f);
                camSetAt(w->cam, 0.0f, 0.0f, 0.0f);
                camSetEye(w->cam, 0.0f, 0.0f, 100.0f);
                camSetSpeed(w->cam, 60.0f);
                camMoveEye(w->cam, 0.0f, 50.0f, 100.0f);
                camMoveAt(w->cam, 0.0f, 50.0f, 0.0f);
            } else if (w->event == 1800) {
                effJetimpactSetSteps(w->mechs[0], 120);
                effJetimpactSetPlay(w->mechs[0], 38.0f, 42.0f);
                camSetSpeed(w->cam, 180.0f);
                camMoveEye(w->cam, 0.0f, 0.0f, 400.0f);
                camMoveAt(w->cam, 0.0f, -20.0f, 0.0f);
            } else if (w->event == 1980) {
                fadeSetSpeed(16.0f);
                fadeSetColor(0, 0, 0);
                fadeSetTarget(255.0f);
            } else if (w->event == 2000) {
                w->state = 17;
            }
            break;
        case 16:
            if (w->event == 1) {
                camSetFov(w->cam, 0.69813174f);
                camSetAt(w->cam, 0.0f, 0.0f, 0.0f);
                camSetEye(w->cam, -100.0f, 200.0f, 100.0f);
            } else if (w->event == 60) {
                effJetimpactSetSteps(w->mechs[0], 300);
                effJetimpactSetPlay(w->mechs[0], 0.0f, 9.0f);
                camSetSpeed(w->cam, 300.0f);
                camMoveAt(w->cam, 0.0f, 20.0f, 0.0f);
                camMoveEye(w->cam, 100.0f, 100.0f, 100.0f);
            } else if (w->event == 390) {
                camSetAt(w->cam, 0.0f, 0.0f, 50.0f);
                camSetEye(w->cam, 0.0f, 150.0f, 70.0f);
            } else if (w->event == 420) {
                effJetimpactSetSteps(w->mechs[0], 40);
                effJetimpactSetPlay(w->mechs[0], 9.0f, 13.0f);
            } else if (w->event == 480) {
                effJetimpactHide(w->mechs[0]);
                effKbackHide(w->fx[7]);
                camSetFov(w->cam, 0.87266463f);
                camSetAt(w->cam, -20.0f, -25.0f, 17.0f);
                camSetEye(w->cam, -340.0f, 25.0f, 109.0f);
                w->fx[0] = effGimMake(obj);
            } else if (w->event == 510) {
                effGimSetPlay(w->fx[0], 120.0f, 0.0f, 35.0f);
            } else if (w->event == 660) {
                tskDestroy(w->fx[0]);
                effJetimpactShow(w->mechs[0]);
                effKbackShow(w->fx[7]);
                camSetFov(w->cam, 0.69813174f);
                camSetAt(w->cam, 0.0f, 0.0f, 100.0f);
                camSetEye(w->cam, 100.0f, -20.0f, 120.0f);
            } else if (w->event == 690) {
                effJetimpactSetSteps(w->mechs[0], 40);
                effJetimpactSetPlay(w->mechs[0], 13.0f, 20.0f);
            } else if (w->event == 750) {
                camSetSpeed(w->cam, 60.0f);
                camMoveEye(w->cam, 100.0f, -40.0f, 160.0f);
                effJetimpactSetSteps(w->mechs[0], 60);
                effJetimpactSetPlay(w->mechs[0], 20.0f, 24.0f);
            } else if (w->event == 840) {
                camSetAt(w->cam, 0.0f, 0.0f, -50.0f);
                camSetEye(w->cam, 0.0f, 150.0f, -30.0f);
                camSetSpeed(w->cam, 240.0f);
                camMoveAt(w->cam, 0.0f, 0.0f, 50.0f);
                camMoveEye(w->cam, 0.0f, 150.0f, 70.0f);
            } else if (w->event == 900) {
                effJetimpactSetSteps(w->mechs[0], 180);
                effJetimpactSetPlay(w->mechs[0], 24.0f, 30.0f);
            } else if (w->event == 1080) {
                camSetFov(w->cam, 0.69813174f);
                camSetAt(w->cam, 0.0f, 0.0f, 0.0f);
                camSetEye(w->cam, 30.0f, 20.0f, -150.0f);
                effJetimpactSetSteps(w->mechs[0], 90);
                effJetimpactSetPlay(w->mechs[0], 30.0f, 34.0f);
            } else if (w->event == 1140) {
                effKbackSparkleMake(obj, 15.0f, 0.0f, -55.0f, 0.2f);
                effKbackSparkleMake(obj, -15.0f, 0.0f, -55.0f, 0.2f);
            } else if (w->event == 1200) {
                effJetimpactSetSteps(w->mechs[0], 180);
                effJetimpactSetPlay(w->mechs[0], 34.0f, 39.0f);
            } else if (w->event == 1410) {
                effJetimpactHide(w->mechs[0]);
                effKbackHide(w->fx[7]);
                camSetFov(w->cam, 0.87266463f);
                camSetAt(w->cam, 0.0f, 13.0f, 0.0f);
                camSetEye(w->cam, -120.0f, 8.0f, 20.0f);
                w->fx[0] = effGim2Make(obj);
                effGim2SetPlay(w->fx[0], 180.0f, 0.0f, 59.0f);
                dhumanShow(w->players[0]);
                dhumanSetAction13A(w->players[0]);
                effGim2SetRider(w->fx[0], w->players[0]);
            } else if (w->event == 1620) {
                tskDestroy(w->fx[0]);
                effJetimpactShow(w->mechs[0]);
                effKbackShow(w->fx[7]);
                dhumanHide(w->players[0]);
                effJetimpactSetTilt(w->mechs[0], 0.0f);
                camSetFov(w->cam, 0.87266463f);
                camSetAt(w->cam, 0.0f, 0.0f, 0.0f);
                camSetEye(w->cam, 0.0f, 0.0f, 100.0f);
                camSetSpeed(w->cam, 60.0f);
                camMoveEye(w->cam, 0.0f, 50.0f, 100.0f);
                camMoveAt(w->cam, 0.0f, 50.0f, 0.0f);
            } else if (w->event == 1680) {
                effJetimpactSetSteps(w->mechs[0], 120);
                effJetimpactSetPlay(w->mechs[0], 39.0f, 42.0f);
                camSetSpeed(w->cam, 180.0f);
                camMoveFov(w->cam, 1.2217306f);
            } else if (w->event == 1890) {
                effJetimpactSetSteps(w->mechs[0], 60);
                effJetimpactSetPlay(w->mechs[0], 42.0f, 46.0f);
            } else if (w->event == 1980) {
                fadeSetSpeed(16.0f);
                fadeSetColor(0, 0, 0);
                fadeSetTarget(255.0f);
            } else if (w->event == 2000) {
                w->state = 19;
            }
            break;
        case 17:
        case 19:
            w->mechs[0] = tskDestroy(w->mechs[0]);
            w->fx[7] = tskDestroy(w->fx[7]);
            w->fx[1] = effGhostMake(obj, 250);
            w->mechs[0] = effDimpactMake(obj, w->single, 0.0f, 0.0f, 0.0f);
            effDimpactSetDir(w->mechs[0], 3.1415927f);
            camSetFov(w->cam, 0.5235988f);
            camSetAt(w->cam, 0.0f, 150.0f, 0.0f);
            camSetEye(w->cam, 0.0f, 100.0f, 200.0f);
            camSetSpeed(w->cam, 120.0f);
            camMoveAt(w->cam, 0.0f, 100.0f, 0.0f);
            camMoveEye(w->cam, 0.0f, 20.0f, 200.0f);
            camMoveFov(w->cam, 1.3962635f);
            fadeSetSpeed(16.0f);
            fadeSetColor(0, 0, 0);
            fadeSetLevel(255.0f);
            fadeSetTarget(0.0f);
            w->event = 0;
            w->state = 18;
            break;
        case 18:
            if (w->event == 20) {
                effDimpactSetSteps(w->mechs[0], 120);
                effDimpactSetPlay(w->mechs[0], 0.0f, 25.0f);
                effDimpactSetTurn(w->mechs[0], 0.02617994f);
            } else if (w->event == 140) {
                effDimpactSetTurn(w->mechs[0], 0.0f);
                fadeSetSpeed(8.0f);
                fadeSetColor(255, 255, 255);
                fadeSetTarget(255.0f);
            } else if (w->event == 150) {
                w->state = 21;
            }
            break;
        case 21:
            w->fx[1] = tskDestroy(w->fx[1]);
            w->fx[0] = effKirakiraMake(obj);
            fadeSetSpeed(64.0f);
            fadeSetColor(255, 255, 255);
            fadeSetLevel(255.0f);
            fadeSetTarget(0.0f);
            w->event = 0;
            w->state = 22;
            break;
        case 22:
            if (w->event == 120) {
                w->state = 23;
            }
            break;
        case 23:
            w->mechs[1] = effDimpactMake(obj, w->single ^ 1, 200.0f, 0.0f, 200.0f);
            effDimpactSetSteps(w->mechs[0], 120);
            effDimpactMoveTo(w->mechs[0], -75.0f, 0.0f, 0.0f);
            effDimpactSetPlay(w->mechs[0], 25.0f, 50.0f);
            effDimpactSetSteps(w->mechs[1], 120);
            effDimpactSetPose2(w->mechs[1]);
            effDimpactMoveTo(w->mechs[1], 75.0f, 0.0f, 0.0f);
            effDimpactSetTurn(w->mechs[1], 0.15707964f);
            w->event = 0;
            w->state = 24;
            break;
        case 24:
            if (w->event == 120) {
                effDimpactSetTurn(w->mechs[1], 0.0f);
                fadeSetSpeed(8.0f);
                fadeSetColor(255, 255, 255);
                fadeSetTarget(255.0f);
            } else if (w->event == 130) {
                w->state = 25;
            }
            break;
        case 25:
            w->fx[0] = tskDestroy(w->fx[0]);
            w->fx[0] = effKflowerMake(obj);
            fadeSetSpeed(64.0f);
            fadeSetColor(255, 255, 255);
            fadeSetLevel(255.0f);
            fadeSetTarget(0.0f);
            w->event = 0;
            w->state = 26;
            break;
        case 26:
            if ((w->event & 7) == 1) {
                Obj *spark = effHeartbMake(obj, 0.0f, 100.0f, 0.0f);

                effHeartbMoveTo(spark, fadeRand(400) - 200, fadeRand(200) - 50, 200.0f);
            }
            if (w->event == 120) {
                fadeSetSpeed(16.0f);
                fadeSetColor(0, 0, 0);
                fadeSetTarget(255.0f);
                if (gGameWork.area != 0) {
                    sndPlay(10);
                }
            } else if (w->event == 140) {
                if (gGameWork.area == 0) {
                    dmenuQueueDemo(4);
                } else {
                    stageGotoBattle(gGameWork.area + 1);
                }
            }
            break;
    }
    return 0;
}
