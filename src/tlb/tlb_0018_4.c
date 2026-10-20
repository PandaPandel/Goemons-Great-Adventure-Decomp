#include "tlb.h"
#include "tlb/tlb_0018.h"
Gfx titlePictureDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetTextureFilter(G_TF_POINT),
    gsDPSetCombineLERP(0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0),
    gsDPPipeSync(),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsSPEndDisplayList(),
};
//! @bug No end command. On the N64 the list runs on into the next object's first data,
//! tlb_0018_7's titleEmptyDl, which is one; a build laid out otherwise runs on into whatever
//! follows (in a little-endian port, titlePictureDl: the buttons' words drawn white).
Gfx titleTintDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetTextureFilter(G_TF_POINT),
#ifdef AVOID_UB
    gsSPEndDisplayList(),
#endif
};

/* The "title" task's work (its proc is titleProc): only what its helpers and steps touch. */
typedef struct TitleWork TitleWork;
typedef s32 (*TitleStep)(Obj *obj, TitleWork *w);

struct TitleWork {
    /* 0x00 */ CamView *camera;
    /* 0x04 */ TitleStep step;
    /* 0x08 */ s8 cursor; /* the button the menu is on */
    /* 0x09 */ u8 choice;
    /* 0x0A */ u8 players; /* how many play (1-4) */
    /* 0x0B */ u8 pad0B;
    /* 0x0C */ u16 timer;
    /* 0x0E */ u8 pad0E[0x2];
    /* 0x10 */ Obj *tback; /* the tinted picture */
    /* 0x14 */ u8 pad14[0x4];
    /* 0x18 */ Obj *logo;
    /* 0x1C */ u8 pad1C[0x4];
    /* 0x20 */ Obj *buttons[4];
    /* 0x30 */ Obj *copyright;
    /* 0x34 */ u8 pad34[0x4];
    /* 0x38 */ s32 nextProc; /* the next task's proc, set by titleSetNextTask */
    /* 0x3C */ Obj *child;
};

/* The settings at 0x800881F8 (fade.c's Settings), as the title's menus set them. */
typedef struct TitleSettings {
    /* 0x00 */ u8 pad0[8];
    /* 0x08 */ s8 language;  /* 0 English, 1 French, 2 German (tlb_0018_3) */
    /* 0x09 */ s8 soundMode; /* 0 sends sound command 5, 1 command 6 */
    /* 0x0A */ u8 padA[6];
} TitleSettings;
extern TitleSettings gSaveSettings;

/* Title step 0x22: the end of the title's fade-in (titleOpenPlayersMenu). */
s32 titleOpenPlayersMenu(Obj *obj, TitleWork *w);

/* tlb_0018_5's tback, logo, copyright and button helpers. */
s32 titleStepBack(Obj *obj, TitleWork *w);
s32 titleStepIntro(Obj *obj, TitleWork *w);

/* "title": the title screen's task. Created, it reads the saved settings from the Controller
 * Pak (clearing and writing them back when the read fails), sends the saved sound mode, turns
 * on the stick as a d-pad, and makes its own camera and lights. Started without an argument it
 * runs the intro (titleStepIntro); with one, it comes back to the title already
 * faded in, counting the players still in the game (1-4). Each frame it runs its step. */
s32 titleProc(Obj *obj, s16 msg, void *arg) {
    TitleWork *w = (TitleWork *) FX(obj);
    LightFrame *light;
    s32 i;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "title";
            gRetracesPerFrame = 1;
            frmFontNewKind();
            fadePakInit();
            if (fadeReadSettings()) {
                PRINTF("共通領域は、初期化されていません \n");
                fadeClearSettings();
                fadeWriteSettings();
            }
            if (gSaveSettings.soundMode == 0) {
                sndPlay(5);
            } else {
                sndPlay(6);
            }
            for (i = 0; i < 4; i++) {
                joySetStickDpad(i, 1);
            }
            w->camera =(CamView *) frmCreate(gFrmLayers[0], gFrmKindCamera);
            if (w->camera == NULL) {
                return -1;
            }
            w->camera->ortho |= 1;
            w->camera->zNear = -200.0f;
            w->camera->zFar = 200.0f;
            frmCreate(&w->camera->f, gFrmKindAmbi);
            frmCreate(&w->camera->f, gFrmKindLight);
            light = (LightFrame *) frmCreate(&w->camera->f, gFrmKindLight);
            light->color[0] = 0;
            light->color[1] = 0;
            light->color[2] = 0;
            floadLoadSet(0x109);
            floadLoadSet(0x10B);
            if (arg == NULL) {
                w->step = titleStepIntro;
            } else {
                w->players = 0;
                for (i = 0; i < 4; i++) {
                    if (gSaveData.players[i].chara >= 0 && gSaveData.players[i].lives > 0) {
                        w->players++;
                    }
                }
                if (w->players <= 0) {
                    w->players = 1;
                } else if (w->players >= 5) {
                    w->players = 4;
                }
                w->tback = titleTbackMake(obj);
                titleTbackTintIn(w->tback, 1);
                w->child = tskCreate(obj, pseleProc, obj->file, (void *) 1);
            }
            break;
        case 1:
            fadeWriteSettings();
            for (i = 0; i < 4; i++) {
                joySetStickDpad(i, 0);
            }
            frmDestroy(&((TitleWork *) FX(obj))->camera->f);
            frmFontDeleteKind();
            break;
        case 0:
            w->timer++;
            if (w->step != NULL) {
                w->step(obj, w);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 titleStepMainMenu(Obj *obj, TitleWork *w);

/* Title step: puts up the whole title at once (the tinted picture, the logo at 160, 92, the
 * copyright and buttons 1 and 6 at 112 and 208, 184), then goes to titleStepMainMenu. */
s32 titleStepShowAll(Obj *obj, TitleWork *w) {
    tskDestroyChildren(obj);
    w->tback = titleTbackMake(obj);
    titleTbackFadeInSecond(w->tback, 1);
    w->logo = titleTlogoMake(obj, 160, 92, 1);
    titleTlogoFadeIn(w->logo, 1);
    w->copyright = titleCopyrightMake(obj);
    titleCopyrightFadeIn(w->copyright, 1);
    w->buttons[0] = titleTbuttonMake(obj, 1, 112.0f, 184.0f);
    titleTbuttonFadeIn(w->buttons[0], 1);
    w->buttons[1] = titleTbuttonMake(obj, 6, 208.0f, 184.0f);
    titleTbuttonFadeIn(w->buttons[1], 1);
    w->timer = 0;
    w->step = titleStepMainMenu;
    return 0;
}

/* Title step: at frame 1 it fades the tinted picture out and the logo (at 160, 92) and the
 * copyright in, over 32 steps; at frame 0x22 it goes on to titleOpenPlayersMenu. */
s32 titleStepBack(Obj *obj, TitleWork *w) {
    if (w->timer == 1) {
        titleTbackTintOut(w->tback, 32);
        w->logo = titleTlogoMake(obj, 160, 92, 1);
        titleTlogoFadeIn(w->logo, 32);
        w->copyright = titleCopyrightMake(obj);
        titleCopyrightFadeIn(w->copyright, 32);
    } else if (w->timer == 0x22) {
        titleOpenPlayersMenu(obj, w);
    }
    return 0;
}

/* Title step, the intro: any player's start or A (after frame 60) skips to the whole title;
 * frame 1 fades in from gGameWork's fade colour (unk16[0..2], then cleared) and makes the tinted
 * picture; 60 fades it in; 120 the logo; 200 the copyright and buttons 1 and 6; 220 goes on to
 * titleStepMainMenu. */
s32 titleStepIntro(Obj *obj, TitleWork *w) {
    s32 i;

    if (w->timer > 60) {
        for (i = 0; i < 4; i++) {
            if (joyGetHeldTime(i, 0x9000) == 1) {
                w->timer = 0;
                w->step = titleStepShowAll;
            }
        }
    }
    if (w->timer == 1) {
        tskDestroy(fadeCreate(gGameWork.gameProcTask));
        fadeCreate(gGameWork.gameProcTask);
        fadeSetLevel(255.0f);
        fadeSetSpeed(32.0f);
        fadeSetColor(gGameWork.fadeColor[0], gGameWork.fadeColor[1], gGameWork.fadeColor[2]);
        fadeSetTarget(0.0f);
        /* as game_proc sets this colour (gameproc.c, case 7): a chain, then [0] from [1]; each
         * read-back spends a ugen temporary, so the la at 220 takes t8 as in the game */
        gGameWork.fadeColor[1] = gGameWork.fadeColor[2] = 0;
        gGameWork.fadeColor[0] = gGameWork.fadeColor[1]; /* FAKE: a second read-back, as gameproc.c case 7 */
        w->tback = titleTbackMake(obj);
    } else if (w->timer == 60) {
        titleTbackFadeInSecond(w->tback, 60);
    } else if (w->timer == 120) {
        w->logo = titleTlogoMake(obj, 160, 92, 1);
        titleTlogoFadeIn(w->logo, 60);
    } else if (w->timer == 200) {
        w->copyright = titleCopyrightMake(obj);
        titleCopyrightFadeIn(w->copyright, 16);
        w->buttons[0] = titleTbuttonMake(obj, 1, 112.0f, 184.0f);
        titleTbuttonFadeIn(w->buttons[0], 16);
        w->buttons[1] = titleTbuttonMake(obj, 6, 208.0f, 184.0f);
        titleTbuttonFadeIn(w->buttons[1], 16);
    } else if (w->timer == 220) {
        w->timer = 0;
        w->step = titleStepMainMenu;
    }
    return 0;
}

s32 titleOpenSoundMenu(Obj *obj, TitleWork *w);
s32 titleStepDemo(Obj *obj, TitleWork *w);

/* Title step, the first menu (buttons 1 and 6): left and right (repeating every 5 frames after
 * 20) move the cursor, the cursor's button blinks, A chooses (0: titleOpenPlayersMenu, 1:
 * titleOpenSoundMenu); after 600 idle frames the demo starts. */
s32 titleStepMainMenu(Obj *obj, TitleWork *w) {
    s32 pad[3];
    u8 old = w->cursor;
    u16 held = 0;
    s32 i;
    s32 t;

    for (i = 0; i < 4; i++) {
        t = joyGetHeldTime(i, 0x200);
        if (t == 1 || (t > 20 && t % 5 == 1)) {
            held |= 0x200;
        }
        t = joyGetHeldTime(i, 0x100);
        if (t == 1 || (t > 20 && t % 5 == 1)) {
            held |= 0x100;
        }
        if (joyGetHeldTime(i, 0x9000) == 1) {
            held |= 0x9000;
        }
        if (joyGetHeldTime(i, 0x4000) == 1) {
            held |= 0x4000;
        }
    }
    if (held) {
        w->timer = 0;
    }
    if (held & 0x200) {
        w->cursor = 0;
    } else if (held & 0x100) {
        w->cursor = 1;
    }
    if (old != w->cursor) {
        sndPlay(0x119);
    }
    switch (w->cursor) {
        case 0:
            titleTbuttonSetBlink(w->buttons[0], 1);
            titleTbuttonSetBlink(w->buttons[1], 0);
            if (held & 0x8000) {
                sndPlay(0x205);
                w->choice = w->cursor;
                titleOpenPlayersMenu(obj, w);
            }
            break;
        case 1:
            titleTbuttonSetBlink(w->buttons[0], 0);
            titleTbuttonSetBlink(w->buttons[1], 1);
            if (held & 0x8000) {
                sndPlay(0x205);
                titleOpenSoundMenu(obj, w);
            }
            break;
    }
    if (w->timer > 600) {
        w->timer = 0;
        w->step = titleStepDemo;
    }
    return 0;
}

/* Title step: clears the buttons and puts up buttons 1 and 6 (fading in over 8), then goes to
 * titleStepMainMenu. */
s32 titleOpenMainMenu(Obj *obj, TitleWork *w) {
    s32 i;

    w->cursor = 0;
    w->timer = 0;
    w->step = titleStepMainMenu;
    for (i = 0; i < 4; i++) {
        if (w->buttons[i] != NULL) {
            w->buttons[i] = tskDestroy(w->buttons[i]);
        }
    }
    w->buttons[0] = titleTbuttonMake(obj, 1, 112.0f, 184.0f);
    titleTbuttonFadeIn(w->buttons[0], 8);
    w->buttons[1] = titleTbuttonMake(obj, 6, 208.0f, 184.0f);
    titleTbuttonFadeIn(w->buttons[1], 8);
    return 0;
}

s32 titleOpenLanguageMenu(Obj *obj, TitleWork *w);

/* Title step, the second menu (buttons 5 and 6): left/right moves the cursor, A on 0 goes to
 * titleOpenLanguageMenu and on 1 to titleOpenSoundMenu; B goes back to the first menu. */
s32 titleStepOptionMenu(Obj *obj, TitleWork *w) {
    s32 pad[3];
    u8 old = w->cursor;
    u16 held = 0;
    s32 i;
    s32 t;

    for (i = 0; i < 4; i++) {
        t = joyGetHeldTime(i, 0x200);
        if (t == 1 || (t > 20 && t % 5 == 1)) {
            held |= 0x200;
        }
        t = joyGetHeldTime(i, 0x100);
        if (t == 1 || (t > 20 && t % 5 == 1)) {
            held |= 0x100;
        }
        if (joyGetHeldTime(i, 0x9000) == 1) {
            held |= 0x9000;
        }
        if (joyGetHeldTime(i, 0x4000) == 1) {
            held |= 0x4000;
        }
    }
    if (held) {
        w->timer = 0;
    }
    if (held & 0x200) {
        w->cursor = 0;
    } else if (held & 0x100) {
        w->cursor = 1;
    }
    if (old != w->cursor) {
        sndPlay(0x119);
    }
    switch (w->cursor) {
        case 0:
            titleTbuttonSetBlink(w->buttons[0], 1);
            titleTbuttonSetBlink(w->buttons[1], 0);
            if (held & 0x8000) {
                sndPlay(0x205);
                titleOpenLanguageMenu(obj, w);
            }
            break;
        case 1:
            titleTbuttonSetBlink(w->buttons[0], 0);
            titleTbuttonSetBlink(w->buttons[1], 1);
            if (held & 0x8000) {
                sndPlay(0x205);
                titleOpenSoundMenu(obj, w);
            }
            break;
    }
    if (held & 0x4000) {
        titleOpenMainMenu(obj, w);
    }
    return 0;
}

/* Title step: clears the buttons and puts up buttons 5 and 6 (fading in over 8), then goes to
 * titleStepOptionMenu. */
s32 titleOpenOptionMenu(Obj *obj, TitleWork *w) {
    s32 i;

    w->cursor = 0;
    w->timer = 0;
    w->step = titleStepOptionMenu;
    for (i = 0; i < 4; i++) {
        if (w->buttons[i] != NULL) {
            w->buttons[i] = tskDestroy(w->buttons[i]);
        }
    }
    w->buttons[0] = titleTbuttonMake(obj, 5, 112.0f, 184.0f);
    titleTbuttonFadeIn(w->buttons[0], 8);
    w->buttons[1] = titleTbuttonMake(obj, 6, 208.0f, 184.0f);
    titleTbuttonFadeIn(w->buttons[1], 8);
    return 0;
}


/* Title step, the language menu (three buttons): left/right moves the cursor (held for
 * repeat), the cursor's button blinks; A sets the language to the cursor and goes back to the
 * second menu (titleOpenOptionMenu), as B does without setting it. */
s32 titleStepLanguageMenu(Obj *obj, TitleWork *w) {
    s32 pad[3];
    u8 old = w->cursor;
    u16 held = 0;
    s32 i;
    s32 t;

    for (i = 0; i < 4; i++) {
        t = joyGetHeldTime(i, 0x200);
        if (t == 1 || (t > 20 && t % 5 == 1)) {
            held |= 0x200;
        }
        t = joyGetHeldTime(i, 0x100);
        if (t == 1 || (t > 20 && t % 5 == 1)) {
            held |= 0x100;
        }
        if (joyGetHeldTime(i, 0x9000) == 1) {
            held |= 0x9000;
        }
        if (joyGetHeldTime(i, 0x4000) == 1) {
            held |= 0x4000;
        }
    }
    if (held) {
        w->timer = 0;
    }
    if (held & 0x200) {
        w->cursor--;
        if (w->cursor < 0) {
            w->cursor = 0;
        }
    } else if (held & 0x100) {
        w->cursor++;
        if (w->cursor >= 3) {
            w->cursor = 2;
        }
    }
    if (old != w->cursor) {
        sndPlay(0x119);
    }
    titleTbuttonSetBlink(w->buttons[old], 0);
    titleTbuttonSetBlink(w->buttons[w->cursor], 1);
    if (held & 0x8000) {
        gSaveSettings.language = w->cursor;
        sndPlay(0x205);
        titleOpenOptionMenu(obj, w);
    }
    if (held & 0x4000) {
        titleOpenOptionMenu(obj, w);
    }
    return 0;
}

s32 titleStepLanguageMenu(Obj *obj, TitleWork *w);

/* Title step: clears the buttons and puts up buttons 7, 8 and 9 (at x 64, 160 and 256, fading
 * in over 8), then goes to titleStepLanguageMenu. */
s32 titleOpenLanguageMenu(Obj *obj, TitleWork *w) {
    s32 i;

    w->cursor = 0;
    w->timer = 0;
    w->step = titleStepLanguageMenu;
    for (i = 0; i < 4; i++) {
        if (w->buttons[i] != NULL) {
            w->buttons[i] = tskDestroy(w->buttons[i]);
        }
    }
    w->buttons[0] = titleTbuttonMake(obj, 7, 64.0f, 184.0f);
    titleTbuttonFadeIn(w->buttons[0], 8);
    w->buttons[1] = titleTbuttonMake(obj, 8, 160.0f, 184.0f);
    titleTbuttonFadeIn(w->buttons[1], 8);
    w->buttons[2] = titleTbuttonMake(obj, 9, 256.0f, 184.0f);
    titleTbuttonFadeIn(w->buttons[2], 8);
    return 0;
}

/* Title step, the sound menu (two buttons): moving the cursor plays sound command 5 (cursor 0) or
 * 6 (cursor 1) to let the player hear the mode; A keeps the cursor as the sound mode, B leaves it.
 * On leaving it sets the saved mode again (5 or 6), sends command 7 and goes back to the first
 * menu. */
s32 titleStepSoundMenu(Obj *obj, TitleWork *w) {
    s32 pad[3];
    u8 old;
    s32 leave = 0;
    u16 held = 0;
    s32 i;
    s32 t;

    old = w->cursor;
    for (i = 0; i < 4; i++) {
        t = joyGetHeldTime(i, 0x200);
        if (t == 1 || (t > 20 && t % 5 == 1)) {
            held |= 0x200;
        }
        t = joyGetHeldTime(i, 0x100);
        if (t == 1 || (t > 20 && t % 5 == 1)) {
            held |= 0x100;
        }
        if (joyGetHeldTime(i, 0x9000) == 1) {
            held |= 0x9000;
        }
        if (joyGetHeldTime(i, 0x4000) == 1) {
            held |= 0x4000;
        }
    }
    if (held & 0x200) {
        w->cursor = 0;
    } else if (held & 0x100) {
        w->cursor = 1;
    }
    if (old != w->cursor) {
        sndPlay(0x119);
        switch (w->cursor) {
            case 0:
                sndPlay(5);
                break;
            case 1:
                sndPlay(6);
                break;
        }
    }
    switch (w->cursor) {
        case 0:
            titleTbuttonSetBlink(w->buttons[0], 1);
            titleTbuttonSetBlink(w->buttons[1], 0);
            break;
        case 1:
            titleTbuttonSetBlink(w->buttons[0], 0);
            titleTbuttonSetBlink(w->buttons[1], 1);
            break;
    }
    if (held & 0x8000) {
        sndPlay(0x205);
        gSaveSettings.soundMode = w->cursor;
        leave = 1;
    } else if (held & 0x4000) {
        leave = 1;
    }
    if (leave) {
        if (gSaveSettings.soundMode == 0) {
            sndPlay(5);
        } else {
            sndPlay(6);
        }
        sndPlay(7);
        titleOpenMainMenu(obj, w);
    }
    return 0;
}

/* Title step: opens the sound menu: the cursor starts on the saved sound mode; clears the
 * buttons and puts up buttons 3 and 4 (fading in over 8). */
s32 titleOpenSoundMenu(Obj *obj, TitleWork *w) {
    s32 i;

    sndPlay(0xB1);
    w->cursor = gSaveSettings.soundMode;
    w->timer = 0;
    w->step = titleStepSoundMenu;
    for (i = 0; i < 4; i++) {
        if (w->buttons[i] != NULL) {
            w->buttons[i] = tskDestroy(w->buttons[i]);
        }
    }
    w->buttons[0] = titleTbuttonMake(obj, 3, 112.0f, 184.0f);
    titleTbuttonFadeIn(w->buttons[0], 8);
    w->buttons[1] = titleTbuttonMake(obj, 4, 208.0f, 184.0f);
    titleTbuttonFadeIn(w->buttons[1], 8);
    return 0;
}

s32 titleStepLeave(Obj *obj, TitleWork *w);

/* Title step, the third menu (buttons 10 and 11): left/right moves the cursor; A takes the choice
 * (the number of players, cursor + 1) into titleStepLeave; B goes back to the first
 * menu. */
s32 titleStepPlayersMenu(Obj *obj, TitleWork *w) {
    s32 pad[3];
    u8 old = w->cursor;
    u16 held = 0;
    s32 i;
    s32 t;

    for (i = 0; i < 4; i++) {
        t = joyGetHeldTime(i, 0x200);
        if (t == 1 || (t > 20 && t % 5 == 1)) {
            held |= 0x200;
        }
        t = joyGetHeldTime(i, 0x100);
        if (t == 1 || (t > 20 && t % 5 == 1)) {
            held |= 0x100;
        }
        if (joyGetHeldTime(i, 0x9000) == 1) {
            held |= 0x9000;
        }
        if (joyGetHeldTime(i, 0x4000) == 1) {
            held |= 0x4000;
        }
    }
    if (held & 0x200) {
        w->cursor = 0;
    } else if (held & 0x100) {
        w->cursor = 1;
    }
    if (old != w->cursor) {
        sndPlay(0x119);
    }
    if (held & 0x8000) {
        sndPlay(0x151);
        w->players = w->cursor + 1;
        w->timer = 0;
        w->step = titleStepLeave;
    }
    if (held & 0x4000) {
        titleOpenMainMenu(obj, w);
        return 0;
    }
    switch (w->cursor) {
        case 0:
            titleTbuttonSetBlink(w->buttons[0], 1);
            titleTbuttonSetBlink(w->buttons[1], 0);
            break;
        case 1:
            titleTbuttonSetBlink(w->buttons[0], 0);
            titleTbuttonSetBlink(w->buttons[1], 1);
            break;
    }
    return 0;
}

/* Title step: clears the buttons and puts up buttons 10 and 11 (fading in over 8), then goes to
 * titleStepPlayersMenu. */
s32 titleOpenPlayersMenu(Obj *obj, TitleWork *w) {
    s32 i;

    w->cursor = 0;
    w->timer = 0;
    w->step = titleStepPlayersMenu;
    for (i = 0; i < 4; i++) {
        if (w->buttons[i] != NULL) {
            w->buttons[i] = tskDestroy(w->buttons[i]);
        }
    }
    w->buttons[0] = titleTbuttonMake(obj, 10, 112.0f, 184.0f);
    titleTbuttonFadeIn(w->buttons[0], 8);
    w->buttons[1] = titleTbuttonMake(obj, 11, 208.0f, 184.0f);
    titleTbuttonFadeIn(w->buttons[1], 8);
    return 0;
}

/* tlb_0018_5's fade-outs, and tlb_0018_0's journal proc. */
s32 titleStepNextTask(Obj *obj, TitleWork *w);

/* Title step on a choice: at frame 1 everything fades out over 32; at 0x22 the logo, copyright
 * and buttons go; at 0x24 choice 0 opens the journal (diaryProc) and choice 1 sets
 * the game's next task to stubCallDefault. */
s32 titleStepLeave(Obj *obj, TitleWork *w) {
    s32 i;

    if (w->timer == 1) {
        titleTbackTintIn(w->tback, 32);
        titleTlogoFadeOut(w->logo, 32);
        titleCopyrightFadeOut(w->copyright, 32);
        for (i = 0; i < 4; i++) {
            if (w->buttons[i] != NULL) {
                titleTbuttonFadeOut(w->buttons[i], 32);
            }
        }
    } else if (w->timer == 0x22) {
        w->logo = tskDestroy(w->logo);
        w->copyright = tskDestroy(w->copyright);
        for (i = 0; i < 4; i++) {
            if (w->buttons[i] != NULL) {
                w->buttons[i] = tskDestroy(w->buttons[i]);
            }
        }
    } else if (w->timer == 0x24) {
        switch (w->choice) {
            case 0:
                w->nextProc = (s32) diaryProc;
                w->step = titleStepNextTask;
                break;
            case 1:
                gGameWork.next = stubCallDefault;
                break;
        }
    }
    return 0;
}

/* Title step for the demos: at frame 1 it fades out to black; at 0x22 it goes to the next demo
 * stage (2, 0x12 and 0x11 in turn). */
s32 titleStepDemo(Obj *obj, TitleWork *w) {
    if (w->timer == 1) {
        fadeSetSpeed(32.0f);
        fadeSetColor(0, 0, 0);
        fadeSetTarget(255.0f);
    } else if (w->timer == 0x22) {
        switch (gGameWork.titleDemo % 3) {
            case 0:
                stageGoto(gGameWork.gameProcTask, 2);
                break;
            case 1:
                stageGoto(gGameWork.gameProcTask, 0x12);
                break;
            case 2:
                stageGoto(gGameWork.gameProcTask, 0x11);
                break;
        }
        gGameWork.titleDemo++;
    }
    return 0;
}

/* Title step: when the next task's proc is set, ends the child and starts it (or, for the title
 * itself, goes back to titleStepBack). The field is read at each use, as the game does. */
s32 titleStepNextTask(Obj *obj, TitleWork *w) {
    TaskProc next = (TaskProc) w->nextProc;

    if (next != NULL) {
        if (w->child != NULL) {
            w->child = tskDestroy(w->child);
        }
        if ((TaskProc) w->nextProc == titleProc) {
            w->timer = 0;
            w->step = titleStepBack;
        } else {
            w->child = tskCreate(obj, (TaskProc) w->nextProc, obj->file, NULL);
        }
        w->nextProc = 0;
    }
    return 0;
}

void titleSetNextTask(Obj *obj, s32 v) {
    ((TitleWork *) FX(obj))->nextProc = v;
}

/* The title's tinted picture. */
Obj *titleGetTback(Obj *obj) {
    return ((TitleWork *) FX(obj))->tback;
}

/* The number of players chosen. */
u8 titleGetPlayers(Obj *obj) {
    return ((TitleWork *) FX(obj))->players;
}
