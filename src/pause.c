#include "object.h"
#include "game.h"
#include "audio.h"
#include "contpak.h"
#include "fade.h"
#include "fileload.h"
#include "font.h"
#include "gauge.h"
#include "gfxbuf.h"
#include "gfxmgr.h"
#include "joypad.h"
#include "pause.h"
#include "stage.h"
#include "swwork.h"
#include "tmanager.h"

/* The "pause" task (Start in a stage, stage.c). While it runs, the frame count stands still and
 * the four controllers' rumble is held.
 * - Opening: the screen dims, and five pictures (file sets gPausePicFileSets) slide in from the right,
 *   one frame each, 32 pixels a step.
 * - The menu has two lines, with a blinking red bar on the chosen one: "Return to the stage" /
 *   "Exit the stage", or in a mission "Return to the mission" / "Quit the mission" ("Return to
 *   the old woman" once flag 0xD is set and 0xA4 is not). A stage can be left only if its flag
 *   bit 0 is set; otherwise, or with pausing off, there is no menu.
 * - A panel shows a count: the six bytes at the save's 0x6E, added up.
 * - Closing slides the pictures out one by one. Leaving the stage goes through
 *   stageGotoAtlas with the area and stage. */

/* A menu line (frmFontCreate's text). */
typedef struct PauseText {
    /* 0x00 */ Frame frame; /* a TextFrame's (font.c): its Frame header */
    /* 0x14 */ f32 x;
    /* 0x18 */ f32 y;
} PauseText;

typedef struct PauseSlide {
    /* 0x0 */ PolFrame *frame;
    /* 0x4 */ f32 speed; /* x per step */
    /* 0x8 */ s16 count; /* steps left */
} PauseSlide; /* size 0xC */

/* The task's work. */
typedef struct PauseWork {
    /* 0x00 */ s32 (*proc)(Obj *obj, struct PauseWork *w); /* nonzero: delete the task */
    /* 0x04 */ PauseText *text[2];
    /* 0x0C */ DLFrame *frame;
    /* 0x10 */ s8 quit; /* the second line was chosen */
    /* 0x12 */ s16 cursor; /* the line chosen, -1 for no menu */
    /* 0x14 */ s32 alpha; /* the bar's, 0x20-0x80 */
    /* 0x18 */ s32 step;
    /* 0x1C */ u16 shown; /* the menu is drawn */
    /* 0x1E */ u16 timer; /* steps in the state */
    /* 0x20 */ PauseSlide slide[5];
} PauseWork;

extern Frame *gFrmLayers[]; /* the frame pools */
extern Frame *gFramePool10;
extern s16 gFrmKindPol; /* the "pol" kind */
extern s16 gFrmKindDlist; /* the "dlist " kind */
extern Gfx *gGfxDlHead; /* the display-list buffer */
extern Gfx gGaugeSetupDl[];
extern Gfx gGaugeModulatePrimDl[];
extern Gfx gGaugeClockSetupDl[];
/* pause.c's .data (0x801090C0-0x801090EC), in ROM order. */
s32 gPausePicFileSets[6] = { /* the pictures' file sets */
    0x00000110, 0x00000112, 0x0000013E, 0x0000013F, 0x00000140, 0x00000141,
};
Gfx *gPausePicDlTable[5] = { /* and their display lists */
    (Gfx *) 0x03000040, (Gfx *) 0x03000040, (Gfx *) 0x03000040, (Gfx *) 0x03000040,
    (Gfx *) 0x03000040,
};
extern u8 pauseCountTex[];
extern u8 pauseCountIaTex[];

s32 pauseOpen(Obj *obj, PauseWork *w);
s32 pauseMenu(Obj *obj, PauseWork *w);
s32 pauseClose(Obj *obj, PauseWork *w);

#define PAUSE(obj) ((PauseWork *) ((u32) (obj) + sizeof(Obj)))
#define PLAYER(i) (&gSaveData.players[i])
#define IN_PLAY(p) ((p)->chara >= 0 && (p)->lives > 0)

/* The count panel: an icon (file set 0x141) and the six counts' sum in two digits. (The sum's
 * loop is on one line: written over three, the unrolled loop's `li a0,2` and first load are
 * scheduled differently.) */
Gfx *pauseDrawCount(Gfx *gfx) {
    s32 i;
    s32 total = 0;

    for (i = 0; i < 6; i++) { total += gSaveData.tegata[i]; }
    gSPDisplayList(gfx++, gGaugeSetupDl);
    gDPPipeSync(gfx++);
    gDPLoadTextureBlock(gfx++, floadSegToRam((u32) pauseCountTex, 0x141, 0), G_IM_FMT_RGBA,
                        G_IM_SIZ_16b, 24, 24, 0, G_TX_NOMIRROR | G_TX_CLAMP,
                        G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);
    gSPTextureRectangle(gfx++, 128 << 2, 112 << 2, 152 << 2, 136 << 2, G_TX_RENDERTILE, 0, 0,
                        1 << 10, 1 << 10);
    gSPDisplayList(gfx++, gGaugeModulatePrimDl);
    gDPPipeSync(gfx++);
    gDPLoadTextureBlock(gfx++, floadSegToRam((u32) pauseCountIaTex, 0x10E, 0), G_IM_FMT_IA,
                        G_IM_SIZ_8b, 8, 176, 0, G_TX_NOMIRROR | G_TX_CLAMP,
                        G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);
    return gaugeDrawNumber(gfx, 168, 120, 8, 16, 1, total, 2);
}

/* An empty function nothing calls (splat folds it into the one before, so its assembly is in
 * pauseDrawCount's). */
void pauseUnusedStub(void) {
}

/* The handler. (One pad word puts the spills where the game has them; the bar's bounce at the
 * top sets the alpha and flips the step on one line.) */
s32 pause_proc(Obj *obj, s16 msg, void *arg) {
    PauseWork *w;
    s32 i;
    s32 pad;

    w = PAUSE(obj);
    switch (msg) {
        case 2:
            obj->name = "pause";
            fadeCreate(gGameWork.gameProcTask);
            frmFontNewKind();
            w->frame = (DLFrame *) frmCreate(gFramePool10, gFrmKindDlist);
            if (w->frame == NULL) {
                return -1;
            }
            for (i = 0; i < 5; i++) {
                w->slide[i].frame = (PolFrame *) frmCreate(gFramePool10, gFrmKindPol);
                if (w->slide[i].frame == NULL) {
                    return -1;
                }
            }
            sndPlay(3);
            for (i = 0; i < 4; i++) {
                contPakInitRumble(i);
                contPakRumbleRequestStop(i);
            }
            contPakRumbleSuspend();
            if (gGameWork.scenarioRunning != 0 ||
                (!(gSaveData.stageFlags[gGameWork.area * 16 + gGameWork.areaStage] &
                   1) &&
                 gGameWork.mission != 1)) {
                w->cursor = -1;
            }
            w->proc = pauseOpen;
            break;
        case 1:
            contPakRumbleResume();
            sndPlay(4);
            for (i = 0; i < 5; i++) {
                frmDestroy((Frame *) w->slide[i].frame);
            }
            for (i = 0; i < 2; i++) {
                frmFontDestroy(w->text[i]);
            }
            frmDestroy((Frame *) w->frame);
            fadeSetLevel(0.0f);
            while (gfxGetPendingSwaps() != 0) {
            }
            floadFreeSet(0x110);
            floadFreeSet(0x112);
            floadFreeSet(0x13E);
            floadFreeSet(0x13F);
            floadFreeSet(0x140);
            floadFreeSet(0x141);
            frmFontDeleteKind();
            break;
        case 0:
            w->timer++;
            for (i = 0; i < 4; i++) {
                contPakRumbleRequestStop(i);
            }
            if (w->proc != NULL && w->proc(obj, w) != 0) {
                return -1;
            }
            for (i = 0; i < 5; i++) {
                if (w->slide[i].frame != NULL && w->slide[i].count != 0) {
                    w->slide[i].frame->pos[0] += w->slide[i].speed;
                    w->slide[i].count--;
                }
            }
            w->alpha += w->step;
            if (w->alpha < 0x20) {
                w->alpha = 0x20;
                w->step = -w->step;
            } else if (w->alpha > 0x80) {
                w->alpha = 0x80; w->step = -w->step;
            }
            gGameState.frameCount--;
            break;
        case 3:
            if (w->frame != NULL) {
                w->frame->dl = gGfxDlHead;
                gSPDisplayList(gGfxDlHead++, gGaugeSetupDl);
                gDPPipeSync(gGfxDlHead++);
                gDPSetCombineLERP(gGfxDlHead++, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0,
                                  0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT);
                gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 0);
                gDPFillRectangle(gGfxDlHead++, 16, 8, 304, 232);
                if (w->shown) {
                    if (w->cursor >= 0) {
                        gDPSetEnvColor(gGfxDlHead++, 255, 0, 0, w->alpha);
                        gDPFillRectangle(gGfxDlHead++, 80, w->cursor * 18 + 147, 240,
                                         w->cursor * 18 + 163);
                    }
                    gGfxDlHead = pauseDrawCount(gGfxDlHead);
                }
                gSPEndDisplayList(gGfxDlHead++);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Opening. Step 1 dims the screen; step 8 loads the pictures (or closes if one is missing) and
 * starts them sliding; from step 9, once the last is in, the menu lines are made.
 * (One index for every loop and no other locals: the x, the stopping point and the speed are
 * written inline, so IDO's strength-reduced temps take spill slots and the frame is the game's
 * 0x50. The file-set loop's unsigned count keeps the game's sltiu 6.) */
s32 pauseOpen(Obj *obj, PauseWork *w) {
    s32 j;

    if (w->timer == 1) {
        fadeSetSpeed(8.0f);
        fadeSetColor(0, 0, 0);
        fadeSetTarget(160.0f);
    } else if (w->timer == 8) {
        for (j = 0; j < sizeof(gPausePicFileSets) / sizeof(gPausePicFileSets[0]); j++) {
            floadLoadSet(gPausePicFileSets[j]);
            if (!floadIsSetLoaded(gPausePicFileSets[j])) {
                w->proc = pauseClose;
                w->timer = 0;
                return 0;
            }
        }
        for (j = 0; j < 5; j++) {
            w->slide[j].frame->dl = gGaugeClockSetupDl;
            w->slide[j].frame->dl2 = gPausePicDlTable[j];
            w->slide[j].frame->pos[0] = j * 0x40 + 0x160;
            w->slide[j].frame->pos[1] = 80.0f;
            w->slide[j].frame->rot[0] = 3.1415927f;
            w->slide[j].frame->scale[0] = w->slide[j].frame->scale[1] =
                w->slide[j].frame->scale[2] = 0.5f;
            floadGetSetRam((void **) w->slide[j].frame->segs, gPausePicFileSets[j]);
            w->slide[j].speed = -32.0f;
            w->slide[j].count = (w->slide[j].frame->pos[0] - (j * 0x20 + 0x60)) / fabsf(-32.0f);
        }
    }
    if (w->timer >= 9 && w->slide[4].count == 0) {
        if (w->cursor >= 0) {
            if (gGameWork.mission == 1) {
                w->text[0] =
                    frmFontCreate(gFrmLayers[10], 98, 148, "Return to the mission");
                if ((*swWorkTest)(0xD) && !(*swWorkTest)(0xA4)) {
                    w->text[1] =
                        frmFontCreate(gFrmLayers[10], 86, 166, "Return to the old woman");
                } else {
                    w->text[1] =
                        frmFontCreate(gFrmLayers[10], 98, 166, "Quit the mission");
                }
            } else {
                w->text[0] = frmFontCreate(gFrmLayers[10], 104, 148, "Return to the stage");
                w->text[1] = frmFontCreate(gFrmLayers[10], 104, 166, "Exit the stage");
            }
            for (j = 0; j < 2; j++) {
                if (w->text[j] != NULL) {
                    w->text[j]->x = 160 - (*frmFontGetWidth)(w->text[j]) / 2;
                    w->text[j]->y = j * 18 + 149;
                }
            }
        }
        w->alpha = 0x20;
        w->step = 4;
        for (j = 0; j < 4; j++) {
            joySetStickDpad(j, 1);
        }
        w->timer = 0;
        w->proc = pauseMenu;
    }
    return 0;
}

/* The menu: Up/Down choose; B returns; A or Start on the second line leaves (in a mission,
 * it sets the game block's +0x13 to 2 instead). With no menu, A, B or Start returns. */
s32 pauseMenu(Obj *obj, PauseWork *w) {
    s32 i;
    s32 old;

    w->shown = 1;
    old = w->cursor;
    for (i = 0; i < 4; i++) {
        if (IN_PLAY(PLAYER(i))) {
            switch (w->cursor) {
                case 0:
                    if (joyGetHeldTime(i, D_JPAD) == 1) {
                        w->cursor = 1;
                        break;
                    }
                case -1:
                    if (joyGetHeldTime(i, A_BUTTON | B_BUTTON | START_BUTTON) == 1) {
                        w->timer = 0;
                        w->proc = pauseClose;
                        return 0;
                    }
                    break;
                case 1:
                    if (joyGetHeldTime(i, U_JPAD) == 1) {
                        w->cursor = 0;
                        break;
                    }
                    if (joyGetHeldTime(i, B_BUTTON) == 1) {
                        w->timer = 0;
                        w->proc = pauseClose;
                        return 0;
                    }
                    if (joyGetHeldTime(i, A_BUTTON | START_BUTTON) == 1) {
                        if (gGameWork.mission == 1) {
                            gGameWork.mission = 2;
                        } else {
                            w->quit = 1;
                        }
                        w->timer = 0;
                        w->proc = pauseClose;
                        return 0;
                    }
                    break;
            }
        }
    }
    if (old != w->cursor) {
        sndPlay(0x203);
    }
    return 0;
}

/* Closing: the lines go at step 1, the pictures slide out at steps 1-9, the screen fades at
 * 15 (to black when leaving), and at 27 the task ends or the stage is left. Only three of the
 * five pictures' frames are freed at step 15; the handler frees all five again.
 * The two loops are do-whiles: a for put the pointer step before the result's store. */
s32 pauseClose(Obj *obj, PauseWork *w) {
    s32 i;

    w->shown = 0;
    if (w->timer == 1) {
        if (w->cursor != -1) {
            sndPlay(0x205);
        }
        i = 0;
        do {
            w->text[i] = frmFontDestroy(w->text[i]);
        } while (++i != 2);
        w->slide[0].count = 30;
        w->slide[0].speed = -32.0f;
    } else if (w->timer == 3) {
        w->slide[1].count = 30;
        w->slide[1].speed = -32.0f;
    } else if (w->timer == 5) {
        w->slide[2].count = 30;
        w->slide[2].speed = -32.0f;
    } else if (w->timer == 7) {
        w->slide[3].count = 30;
        w->slide[3].speed = -32.0f;
    } else if (w->timer == 9) {
        w->slide[4].count = 30;
        w->slide[4].speed = -32.0f;
    } else if (w->timer == 15) {
        i = 0;
        do {
            w->slide[i].frame = (PolFrame *) frmDestroy((Frame *) w->slide[i].frame);
        } while (++i != 3);
        w->frame = (DLFrame *) frmDestroy((Frame *) w->frame);
        if (w->quit) {
            (*sndPlay)(10);
            fadeSetSpeed(8.0f);
            fadeSetTarget(255.0f);
        } else {
            fadeSetSpeed(8.0f);
            fadeSetTarget(0.0f);
        }
    } else if (w->timer == 27) {
        for (i = 0; i < 4; i++) {
            joySetStickDpad(i, 0);
        }
        if (w->quit) {
            stageGotoAtlas(gGameWork.area, gGameWork.areaStage);
        } else {
            return 1;
        }
    }
    return 0;
}
