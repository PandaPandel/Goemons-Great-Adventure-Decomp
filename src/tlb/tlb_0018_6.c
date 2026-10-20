#include "tlb.h"
#include "tlb/tlb_0018.h"

/* "psele" (the player select): what its handlers below read. */
typedef struct PseleW {
    /* 0x00 */ s32 count; /* players to choose for, and their items */
    /* 0x04 */ CamView *view; /* the game's camera while it runs (orthographic) */
    /* 0x08 */ CamView *view8; /* layer 8's */
    /* 0x0C */ CamView *viewFx; /* the effect layer's (orthographic, scissored) */
    /* 0x10 */ s32 (*handler)(Obj *obj, struct PseleW *w); /* the current one */
    /* 0x14 */ Obj *bookmark;
    /* 0x18 */ Obj *items[4];
    /* 0x28 */ Obj *pics[4]; /* the four characters' pictures (tlb_0018_7) */
    /* 0x38 */ s16 choice[4]; /* each player's character, from 1 (0: none yet) */
    /* 0x40 */ u16 event;
    /* 0x42 */ u8 pad42;
    /* 0x43 */ u8 inGame; /* nonzero: B does not go back */
    /* 0x44 */ u8 done; /* nonzero: the task ends */
} PseleW;

s32 pseleOpening(Obj *obj, PseleW *w);

/* "psele", the player select (its argument: B does not go back). It starts the effect and
 * attack tasks, loads sets 0x10B, 0x10A and 0x109, and makes three cameras: the game's
 * (orthographic, a light from straight ahead), layer 8's and the effect layer's (scissored to
 * 16, 8 - 304, 232). It is for the parent's player count (1-4); the title's background tints
 * blue, sound 0xA1 starts if it is not playing, and the opening (pseleOpening) runs
 * first. Each frame it counts its event and runs its handler, until done. */
s32 pseleProc(Obj *obj, s16 msg, void *arg) {
    PseleW *w = (PseleW *) FX(obj);
    LightFrame *light;
    Obj *tback;
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "psele";
            w->inGame = (s32) arg;
            gRetracesPerFrame = 1;
            tskCreate(obj, effect, 0, NULL);
            floadLoadSet(0x10B);
            floadLoadSet(0x10A);
            floadLoadSet(0x109);
            tskCreate(obj, attack_Proc, 0, NULL);
            if ((w->view = (CamView *) frmCreate(gFrmLayers[0], gFrmKindCamera)) == NULL) {
                return -1;
            }
            w->view->ortho |= 1;
            w->view->zNear = -200.0f;
            w->view->zFar = 200.0f;
            gGameWork.view = w->view;
            light = (LightFrame *) frmCreate(&w->view->f, gFrmKindLight);
            light->lightNum = 1;
            light->dir[0] = 0;
            light->dir[1] = 0x7F;
            light->dir[2] = 0;
            if ((w->view8 = (CamView *) frmCreate(gFrmLayer8, gFrmKindCamera)) == NULL) {
                return -1;
            }
            w->view8->eye[1] = 100.0f;
            if ((w->viewFx = (CamView *) frmCreate(gFrmEffectLayer, gFrmKindCamera)) == NULL) {
                return -1;
            }
            w->viewFx->ortho |= 1;
            w->viewFx->zNear = -200.0f;
            w->viewFx->zFar = 200.0f;
            w->viewFx->scissor.ulx = 16;
            w->viewFx->scissor.uly = 8;
            w->viewFx->scissor.lrx = 304;
            w->viewFx->scissor.lry = 232;
            if ((w->count = titleGetPlayers(obj->parent)) <= 0) {
                w->count = 1;
            } else if (w->count >= 5) {
                w->count = 4;
            }
            w->handler = pseleOpening;
            if ((tback = titleGetTback(obj->parent)) != NULL) {
                titleTbackSetSteps(tback, 30);
                titleTbackTintTo(tback, 140, 170, 255, 0, 50, 160);
            }
            if (sndGetVolume(0xA1, 0) == 0) {
                sndPlay(0xA1);
            }
            break;
        case 1:
            frmDestroy(&w->viewFx->f);
            frmDestroy(&w->view8->f);
            frmDestroy(&w->view->f);
            break;
        case 0:
            w->event++;
            if (w->handler != NULL) {
                w->handler(obj, w);
            }
            if (w->done) {
                return 0;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 pseleEnding(Obj *obj, PseleW *w);
s32 pseleBack(Obj *obj, PseleW *w);
s32 pseleCheck(Obj *obj, PseleW *w);

/* The player select opening. At event 1: its bookmark at (160, 32), and the four characters'
 * pictures sliding from (400, 128) to 70 apart from 56 across over 32 frames; the characters
 * not yet available (gSaveData.charaCount) cannot be chosen (-1). At event 0x24: the pictures
 * of the ones that can be chosen are marked, each player gets an item, and the check
 * (pseleCheck) takes over. */
s32 pseleOpening(Obj *obj, PseleW *w) {
    s32 i;

    if (w->event == 1) {
        w->bookmark = diaryBeshiMake(obj, 160.0f, 32.0f, 0.0f);
        diarySioriSetKind(w->bookmark, 3);
        diarySioriScaleTo(w->bookmark, 1.0f);
        for (i = 0; i < 4; i++) {
            w->pics[i] = pseleTateMake(obj, i, 400.0f, 128.0f);
            pseleTateSetSteps(w->pics[i], 32);
            pseleTateMoveTo(w->pics[i], i * 70 + 56, 128.0f);
            if (i < gSaveData.charaCount) {
                w->choice[i] = 0;
            } else {
                w->choice[i] = -1;
            }
        }
    } else if (w->event == 0x24) {
        for (i = 0; i < 4; i++) {
            if (w->choice[i] == 0) {
                pseleTateShow(w->pics[i]);
            }
        }
        for (i = 0; i < w->count; i++) {
            w->items[i] = pselePscurMake(obj, i, w->count);
        }
        w->handler = pseleCheck;
    }
    return 0;
}

/* The player select's check. Once as many players have chosen as it is for, the save data's
 * four players take the characters chosen (a character's entry gets its player's number; the
 * rest none), the choices are spent (100) and pseleEnding takes over. With no
 * choice made yet, B (held one frame) on any controller goes back (pseleBack). */
s32 pseleCheck(Obj *obj, PseleW *w) {
    s32 n = 0;
    s32 i;

    for (i = 0; i < 4; i++) {
        if (w->choice[i] > 0) {
            n++;
        }
    }
    if (n >= w->count) {
        for (i = 0; i < 4; i++) {
            gSaveData.players[i].chara = -1;
        }
        for (i = 0; i < 4; i++) {
            /* FAKE: one line, for the game's order of the 100 and the 10 (IDO_NOTES 12) */
            if (w->choice[i] > 0) { gSaveData.players[w->choice[i] - 1].chara = i; } w->choice[i] = 100;
        }
        w->event = 0;
        w->handler = pseleEnding;
    } else if (n == 0) {
        if (w->inGame != 0) {
            return 0;
        }
        for (i = 0; i < 4; i++) {
            if (joyGetHeldTime(i, 0x4000) == 1) {
                w->event = 0;
                w->handler = pseleBack;
                return 0;
            }
        }
    }
    return 0;
}

/* The player select's ending. At event 30 (sound 7) it fades to black. At 66, a new game
 * starts the opening; otherwise the save's resume point (gSaveData.savePoint) gives the area and
 * stage to take the players to on the map.
 * Matched 2026-10-11; it was parked for schedule: it only matches with pseleCheck's finished C
 * compiled before it. Each case's two stores share one source line, as the original schedules
 * them (IDO picks the store order from the line, IDO_NOTES 351; on separate lines it was 28
 * lines off). */
s32 pseleEnding(Obj *obj, PseleW *w) {
    if (w->event == 30) {
        sndPlay(7);
        fadeSetLevel(0.0f);
        fadeSetSpeed(32.0f);
        fadeSetColor(0, 0, 0);
        fadeSetTarget(255.0f);
    } else if (w->event == 66) {
        if (!w->inGame) {
            switch (gSaveData.savePoint) {
                case 0:
                    gGameWork.area = 0;
                    gGameWork.areaStage = 0;
                    stageGotoOpening();
                    return 0;
                case 1:
                    gGameWork.area = 0; gGameWork.areaStage = 2;
                    break;
                case 2:
                    gGameWork.area = 1; gGameWork.areaStage = 2;
                    break;
                case 3:
                    gGameWork.area = 2; gGameWork.areaStage = 3;
                    break;
                case 4:
                    gGameWork.area = 3; gGameWork.areaStage = 3;
                    break;
                case 5:
                    gGameWork.area = 4; gGameWork.areaStage = 3;
                    break;
                case 6:
                    gGameWork.area = 3; gGameWork.areaStage = 6;
                    break;
                case 7:
                    gGameWork.area = 0; gGameWork.areaStage = 0;
                    break;
                case 8:
                    gGameWork.area = 1; gGameWork.areaStage = 0;
                    break;
                case 9:
                    gGameWork.area = 2; gGameWork.areaStage = 0;
                    break;
                case 10:
                    gGameWork.area = 3; gGameWork.areaStage = 0;
                    break;
                case 11:
                    gGameWork.area = 4; gGameWork.areaStage = 0;
                    break;
                case 12:
                    gGameWork.area = 0; gGameWork.areaStage = 6;
                    break;
                case 13:
                    gGameWork.area = 1; gGameWork.areaStage = 5;
                    break;
                case 14:
                    gGameWork.area = 2; gGameWork.areaStage = 6;
                    break;
                case 15:
                    gGameWork.area = 3; gGameWork.areaStage = 5;
                    break;
                case 16:
                    gGameWork.area = 4; gGameWork.areaStage = 5;
                    break;
            }
            stageResetClockToAtlas();
        } else {
            stageGotoAtlasFrom(NULL, gGameWork.area, gGameWork.areaStage);
        }
    }
    return 0;
}


/* The player select's handler. At event 1 it closes: its items go, the bookmark shrinks away,
 * and the four pictures slide off to 400, 128 over 32 frames. At event 0x24 it tints the
 * title's picture warm over 30 frames and has the title open the journal. */
s32 pseleBack(Obj *obj, PseleW *w) {
    s32 i;
    Obj *tback;

    if (w->event == 1) {
        for (i = 0; i < w->count; i++) {
            w->items[i] = tskDestroy(w->items[i]);
        }
        diarySioriScaleTo(w->bookmark, 0.0f);
        for (i = 0; i < 4; i++) {
            if (w->pics[i] != NULL) {
                pseleTateHide(w->pics[i]);
                pseleTateSetSteps(w->pics[i], 32);
                pseleTateMoveTo(w->pics[i], 400.0f, 128.0f);
            }
        }
    } else if (w->event == 0x24) {
        tback = titleGetTback(obj->parent);
        if (tback != NULL) {
            titleTbackSetSteps(tback, 30);
            titleTbackTintTo(tback, 255, 224, 160, 80, 48, 16);
        }
        titleSetNextTask(obj->parent, (s32) diaryProc);
    }
    return 0;
}

/* The task before it in the file: a u16 table at its work + 0x38 (only this is known). */
typedef struct Tlb18TableW {
    /* 0x00 */ u8 pad00[0x38];
    /* 0x38 */ u16 table[1];
} Tlb18TableW;

u16 pseleGetTaker(Obj *o, s32 i) {
    return ((Tlb18TableW *) FX(o))->table[i];
}

void pseleSetTaker(Obj *o, s32 i, u16 v) {
    ((Tlb18TableW *) FX(o))->table[i] = v + 1;
}
