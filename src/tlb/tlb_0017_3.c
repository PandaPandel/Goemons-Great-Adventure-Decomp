#include "tlb.h"
#include "tlb/tlb_0017.h"

#define DPRINTF

/* "opening"'s work, as its task and step read it. */
typedef struct OpeningW {
    /* 0x00 */ u8 kind; /* 0: wait; 1, 2: a blue-to-white sky fade; 3 on: a plain fill */
    /* 0x01 */ u8 state;
    /* 0x02 */ s16 timer;
    /* 0x04 */ u8 pad04[0x4];
    /* 0x08 */ Frame *f08;
    /* 0x0C */ u8 pad0C[0x4];
    /* 0x10 */ Camera *cam;
    /* 0x14 */ u8 pad14[0x8];
    /* 0x1C */ Obj *players[2];
    /* 0x24 */ Obj *child;
} OpeningW;

s32 openingStep(Obj *obj, OpeningW *w);

/* "opening": one retrace a frame, the effects and the attack task, and the first two players
 * of the save that have a character (each at 500 back), or Goemon if none has; the AI camera
 * (40 degrees, planes 5 and 20000); a letterboxed black fade, fully in. Each frame
 * openingStep runs it; message 7 sets its kind. */
s32 openingProc(Obj *obj, s16 msg, void *arg) {
    OpeningW *w;
    CamView *view;
    s32 i;
    s32 n;
    s32 chara;

    switch (msg) {
        case 2:
            obj->name = "opening";
            gRetracesPerFrame = 1;
            tskCreate(obj, effect, 0, NULL);
            tskCreate(obj, attack_Proc, 0, NULL);
            w = (OpeningW *) (obj + 1);
            for (i = 0, n = 0; i < 4; i++) {
                chara = gSaveData.players[i].chara;
                if (chara >= 0 && chara < 4) {
                    w->players[n++] = dhumanMake(obj, i, chara, 0.0f, -500.0f, 0.0f);
                    if (n >= 2) {
                        break;
                    }
                }
            }
            if (n == 0) {
                w->players[0] = dhumanMake(obj, 0, 0, 0.0f, -500.0f, 0.0f);
            }
            camAiCreate(obj);
            w->cam = camAiGetCam();
            view = camGetView(w->cam);
            view->zNear = 5.0f;
            view->zFar = 20000.0f;
            camSetFov(w->cam, 0.69813174f);
            fadeCreate(obj);
            fadeSetLetterbox(320, 38, 0, 190);
            fadeSetColor(0, 0, 0);
            fadeSetLevel(255.0f);
            stubReturnZero((s32) obj, 1, 10);
            break;
        case 1:
            gRetracesPerFrame = 2;
            frmDestroy(((OpeningW *) (obj + 1))->f08);
            break;
        case 0:
            openingStep(obj, (OpeningW *) (obj + 1));
            break;
        case 7:
            ((OpeningW *) (obj + 1))->kind = (s32) arg;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The opening's step. State 0: once it has a kind, the screen's fill becomes a vertical fade
 * (kinds 1 and 2: from (0x40, 0x60, 0xE0) at line 20 to (0x80, 0xC0, 0xFF) at line 200) or a
 * plain black, and the screen fades in. State 1: kind 1 goes on after 20 frames. State 2: it
 * starts its child task and sets the camera at (0, 15, 0) from (0, 20, 300). */
s32 openingStep(Obj *obj, OpeningW *w) {
    w->timer++;
    DPRINTF("\r timer=%d ", w->timer);
    switch (w->state) {
        case 0:
            if (w->kind != 0) {
                if (w->kind < 3) {
                    gGameState.fade = 1;
                    gGameState.fadeFrom.r = 0x40;
                    gGameState.fadeFrom.g = 0x60;
                    gGameState.fadeFrom.b = 0xE0;
                    gGameState.fadeStart = 20;
                    gGameState.fadeTo[0] = 0x80;
                    gGameState.fadeTo[1] = 0xC0;
                    gGameState.fadeTo[2] = 0xFF;
                    gGameState.fadeEnd = 200;
                } else {
                    gGameState.fade = 0;
                    gGameState.fadeFrom.r = 0;
                    gGameState.fadeFrom.g = 0;
                    gGameState.fadeFrom.b = 0;
                }
                fadeSetSpeed(16.0f);
                fadeSetTarget(0.0f);
                w->timer = 0;
                w->state = 1;
            }
            break;
        case 1:
            if (w->timer >= 20 && w->kind == 1) {
                w->state = 2;
            }
            break;
        case 2:
            w->child = openingIdleMake(obj);
            w->state = 3;
            camSetAt(w->cam, 0.0f, 15.0f, 0.0f);
            camSetEye(w->cam, 0.0f, 20.0f, 300.0f);
            break;
        case 3:
            break;
    }
    return 0;
}
