#include "diary.h"
#include "tlb/tlb_0018.h"

/* The journal screen's Controller Pak error window, "derr": a box of text (one of six messages,
 * in the language the settings name) with a highlight on the chosen line. Up and down move the
 * highlight, A runs the line's handler, B the cancel handler; each handler (tlb_0018_2) reports a
 * result to the parent task (diarySetResult) and ends the window. DerrWork is in diary.h. */

/* The settings at 0x800881F8 (fade.c's Settings); byte 8 is the language: 0 English, 1 French,
 * 2 German. */
typedef struct DerrSettings {
    /* 0x00 */ u8 pad0[8];
    /* 0x08 */ s8 language;
    /* 0x09 */ u8 pad9[7];
} DerrSettings;
extern DerrSettings gSaveSettings;

Gfx diaryErrBoxDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT),
    gsSPEndDisplayList(),
}; /* the box's render setup */
extern char *diaryErrNoNotesText[3];
extern char *diaryErrNoPagesText[3];
extern char *diaryErrPakProblemText[3];
extern char *diaryErrNoPakText[3];
extern char *diaryErrPakDamagedText[3];
extern char *diaryErrRumbleText[5];
extern Gfx diaryErrUnusedDl[];


/* Sets the window's choices for an error code and returns its text, NULL for none. Bit 0x10000
 * marks the codes 1, 11, 0x101 and 0x200 as "no Controller Pak". */
char *diaryErrSetText(Obj *obj, DerrWork *w, s32 code) {
    char *text = diaryErrPakProblemText[gSaveSettings.language];

    switch (code & ~0x10000) {
        case 0:
        case 9:
        case 0x102:
            return NULL;
        case 1:
        case 11:
        case 0x101:
        case 0x200:
            if (code & 0x10000) {
                w->count = 2;
                w->choose[0] = diaryErrNoSave;
                w->choose[1] = diaryErrRetry;
                w->cancel = diaryErrCancel;
                return diaryErrNoPakText[gSaveSettings.language];
            }
            break;
        case 10:
            w->count = 3;
            w->choose[0] = diaryErrRetry;
            w->choose[1] = diaryErrRepair;
            w->choose[2] = diaryErrNoSave;
            w->cancel = diaryErrCancel;
            return diaryErrPakDamagedText[gSaveSettings.language];
        case 7:
            w->count = 2;
            w->lines[1] = 2;
            w->choose[0] = diaryErrNoSave;
            w->choose[1] = diaryErrNoRoomQuit;
            w->cancel = diaryErrCancel;
            return diaryErrNoPagesText[gSaveSettings.language];
        case 8:
            w->count = 2;
            w->lines[1] = 2;
            w->choose[0] = diaryErrNoSave;
            w->choose[1] = diaryErrNoRoomQuit;
            w->cancel = diaryErrCancel;
            return diaryErrNoNotesText[gSaveSettings.language];
        case 0x8000:
            if (contPakProbe(0) == 0x200) {
                diaryErrRumbleOk(obj, w);
                return NULL;
            }
            w->choose[0] = diaryErrRumbleOk;
            w->cancel = diaryErrRumbleOk;
            return diaryErrRumbleText[gSaveSettings.language];
        case 0x100:
            break;
    }
    w->count = 2;
    w->choose[0] = diaryErrNoSave;
    w->choose[1] = diaryErrRetry;
    w->cancel = diaryErrCancel;
    return text;
}

char *diaryErrNoNotesText[3] = { "There are no empty notes.\nPlease erase some data.\n1 free note (and 7 pages) required.\n    $ Continue without saving\n    $ Delete data at Controller Pak\n       delete mode.", "Il n'y a pas de notes libres.\nEffacez des donn<ees.\n1 note libre (et 7 pages) n<ecessaire.\n    $ Continuez sans sauvegarder\n    $ Effacez les donn<ees en mode\n       d'effacement du Controller Pak", "Es gibt keine leeren Notizen.\nBitte einige Daten l~oschen.\nEine freie Notiz (und 7 Seiten) erforderlich\n    $ Fortsetzung ohne Speicherung\n    $ Die Daten im Comtroller Pak-\n       L~oschmodus l~oschen." }; /* "There are no empty notes." */
char *diaryErrNoPagesText[3] = { "There are not enough empty pages.\nPlease erase some data.\n1 free note (and 7 pages) required.\n    $ Continue without saving\n    $ Delete data at Controller Pak\n       delete mode.", "Il n'y a pas assez de pages libres.\nEffacez des donn<ees\n1 note libre (et 7 pages) n<ecessaire.\n    $ Continuez sans sauvegarder\n    $ Effacez les donn<ees en mode\n       d'effacement du Controller Pak", "Es gibt nicht genug leere Seiten.\nBitte einige Daten l~oschen.\nEine freie Notiz (und 7 Seiten) erforderlich\n    $ Fortsetzung ohne Speicherung\n    $ Die Daten im Comtroller Pak-\n       L~oschmodus l~oschen." }; /* "There are not enough empty pages." */
char *diaryErrPakProblemText[3] = { "There is a problem with\nthe Controller Pak.\nPlease reconnect the Controller Pak\n    $ Continue without saving\n    $ Re-insert Controller Pak", "Il y a un probl>eme avec\nle Controller Pak.\nReconnectez le Controller Pak.\n    $ Continuez sans sauvegarder\n    $ R<eins<erez le Controller Pak", "St~orung des Controller Paks.\nController Pak neu einschieben.\n    $ Fortsetzung ohne Speicherung\n    $ Controller Pak neu einschieben" }; /* "There is a problem with the Controller Pak." */
char *diaryErrNoPakText[3] = { "A Controller Pak is not connected.\nData cannot be saved.\n    $ Continue without saving\n    $ Re-insert Controller Pak", "Le Controller Pak n'est pas raccord<e.\nImpossible de sauvegarder les donn<ees.\n    $ Continuez sans sauvegarder\n    $ R<eins<erez le Controller Pak", "Es ist kein Controller Pak angeschlossen.\nDaten k~onnen nicht gespeichert werden.\n    $ Fortsetzung ohne Speicherung\n    $ Controller Pak neu einschieben" }; /* "A Controller Pak is not connected." */
char *diaryErrPakDamagedText[3] = { "Controller Pak is damaged.\n  $ Re-insert Controller Pak\n  $ Repair(Data may be lost)\n  $ Continue without saving", "Le Controller Pak est endommag<e\nSi vous s<electionnez \"R<eparez\",\nvous risquez de perdre les donn<ees.\n  $ R<eins<erez le Controller Pak\n  $ R<eparez\n  $ Continuer sans sauvegarder", "Controller Pak besch~adigt.\nBei Instandsetzung k~onnen\nDaten verloren gehen.\n  $ Controller Pak neu einschieben\n  $ Instandsetzen\n  $ Ohne Speicherung fortsetzen" }; /* "Controller Pak is damaged." */
/* "Please insert a Rumble Pak", by language; two NULL entries after the three (the display list
 * after them starts at 0x0800BD00, where 8-byte alignment alone would put it at 0x0800BCF8) */
char *diaryErrRumbleText[5] = { "Please insert a Rumble Pak\nif you wish to use it.\nPlease push A Button to continue.", "Ins<erez le Rumble Pak\nsi vous voulez l'utiliser.\nAppuyez sur le bouton A pour continuer.", "Bitte Rumble Pak einsetzen,\nfalls Verwendung erw~unscht.\nZur Fortsetzung des Spiels\ndie A-Taste dr~ucken.", NULL, NULL };
/* a display list nothing in the file names (splat gave it no label) */
Gfx diaryErrUnusedDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2),
    gsDPSetCombineLERP(0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0),
    gsDPPipeSync(),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsSPEndDisplayList(),
};

/* The window's task: arg is the error code (diaryErrOpen). Message 2 builds the text,
 * centred, and the box around it; 0 runs the current proc (-1 from it ends the task); 3 draws
 * the box and the highlight. */
s32 diaryErrProc(Obj *obj, s16 msg, void *arg) {
    DerrWork *w = (DerrWork *) (obj + 1);
    char *text;
    s32 i;
    s32 width;
    s32 height;
    s32 j;
    s32 top;

    switch (msg) {
        case 2:
            obj->name = "derr";
            /* One line: written over three, IDO stores lines[1..3] before lines[0]. */
            for (j = 0; j < 4; j++) { w->lines[j] = 1; }
            w->frame = (DLFrame *) frmCreate(gFrmLayers[10], gFrmKindDlist);
            if (w->frame == NULL) {
                return -1;
            }
            text = diaryErrSetText(obj, w, (s32) arg);
            if (text == NULL) {
                return -1;
            }
            w->text = frmFontCreate((Frame *) w->frame, 160, 120, text);
            width = frmFontGetWidth(w->text);
            height = frmFontGetHeight(w->text);
            w->text->x -= width / 2;
            w->text->y -= height / 2;
            w->x = w->text->x - 2.0f;
            w->y = w->text->y - 2.0f;
            w->w = width + 4;
            w->h = height + 4;
            w->alpha = 32;
            w->alphaStep = 2;
            w->proc = diaryErrInput;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 0:
            if (w->proc != NULL) {
                i = w->proc(obj, w);
                if (i != 0) {
                    return i;
                }
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, fadeSegToRam(diaryErrBoxDl, obj->file));
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 192);
            gDPFillRectangle(gGfxDlHead++, w->x, w->y, w->x + w->w, w->y + w->h);
            if (w->count > 0) {
                s32 bottom;

                top = w->y + w->h + w->text->ygap - 3;
                for (i = w->cursor; i < w->count; i++) {
                    top -= (w->text->h + w->text->ygap) * w->lines[i];
                }
                bottom = w->lines[w->cursor] * (w->text->h + w->text->ygap) + top -
                         w->text->ygap + 2;
                gDPPipeSync(gGfxDlHead++);
                gDPSetEnvColor(gGfxDlHead++, 255, 0, 0, w->alpha);
                gDPFillRectangle(gGfxDlHead++, w->x + 1, top, w->x + w->w - 1, bottom);
            }
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The window's proc while it waits for a choice: swings the highlight's alpha, and after 8
 * frames reads every controller (Up and Down repeat after 20 frames, every 5th), moves the
 * cursor, and runs the chosen line's handler on A or the cancel handler on B. */
s32 diaryErrInput(Obj *obj, DerrWork *w) {
    s32 i;
    u16 button;
    s32 held;
    s32 r;

    w->alpha += w->alphaStep;
    /* Each pair on one line: IDO then stores the step first, as the game does. */
    if (w->alpha < 32) {
        w->alpha = 32; w->alphaStep = -w->alphaStep;
    } else if (w->alpha > 128) {
        w->alpha = 128; w->alphaStep = -w->alphaStep;
    }
    /* Both set before the wait: "for (i = 0; ...)" after it gives "bne i,4" for the game's slti. */
    button = 0;
    i = 0;
    if (++w->timer < 8) {
        return 0;
    }
    for (; i < 4; i++) {
        held = joyGetHeldTime(i, U_JPAD);
        if (held == 1 || (held > 20 && held % 5 == 1)) {
            button |= U_JPAD;
        }
        held = joyGetHeldTime(i, D_JPAD);
        if (held == 1 || (held > 20 && held % 5 == 1)) {
            button |= D_JPAD;
        }
        held = joyGetHeldTime(i, L_JPAD);
        if (held == 1 || (held > 20 && held % 5 == 1)) {
            button |= L_JPAD;
        }
        held = joyGetHeldTime(i, R_JPAD);
        if (held == 1 || (held > 20 && held % 5 == 1)) {
            button |= R_JPAD;
        }
        if (joyGetHeldTime(i, A_BUTTON) == 1) {
            button |= A_BUTTON;
        }
        if (joyGetHeldTime(i, B_BUTTON) == 1) {
            button |= B_BUTTON;
        }
    }
    if (w->count >= 2) {
        if (button & U_JPAD) {
            if (--w->cursor < 0) {
                w->cursor = 0;
            }
        } else if (button & D_JPAD) {
            if (++w->cursor > w->count - 1) {
                w->cursor = w->count - 1;
            }
        }
    }
    if (button & A_BUTTON) {
        if (w->choose[w->cursor] != NULL) {
            r = w->choose[w->cursor](obj, w);
            if (r != 0) {
                return r;
            }
        }
    } else if (button & B_BUTTON) {
        if (w->cancel != NULL) {
            r = w->cancel(obj, w);
            if (r != 0) {
                return r;
            }
        }
    }
    return 0;
}

/* The proc of "Continue without saving" when the save has no room (diaryErrNoRoomQuit):
 * plays sound 10 and fades to black, then after 10 frames goes to the Controller Pak screen. */
s32 diaryErrQuit(Obj *obj, DerrWork *w) {
    if (++w->timer == 1) {
        sndPlay(10);
        tskDestroy(fadeCreate(gGameWork.gameProcTask));
        fadeCreate(gGameWork.gameProcTask);
        fadeSetAll(8.0f, 0.0f, 255.0f, 0, 0, 0);
    } else if (w->timer > 10) {
        stageGotoPak();
        return -1;
    }
    return 0;
}

/* Opens the error window under obj for an error code; 0 is no error. */
void diaryErrOpen(Obj *obj, s32 code) {
    if (code != 0) {
        if (tskCreate(obj, diaryErrProc, obj->file, (void *) code) != NULL) {
            diarySetResult(obj, code);
        }
    }
}
