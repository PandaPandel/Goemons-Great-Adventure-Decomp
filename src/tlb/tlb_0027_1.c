#include "tlb.h"
#include "tlb/tlb_0027.h"

extern u8 gDemoCamHold;
extern s32 gObjMoveSel; /* OBJMOVE: the model chosen, -1 for none */
extern f32 gObjMovePosX; /* the chosen model's position and angle, as shown */
extern f32 gObjMovePosY;
extern f32 gObjMovePosZ;
extern f32 gObjMoveRot;
extern f32 gObjMoveDx; /* the moves this frame: x, y, z, angle */
extern f32 gObjMoveDy;
extern f32 gObjMoveDz;
extern f32 gObjMoveDrot;
extern s32 dochukiDemoFrame; /* the scene's frame count (tlb_0027_0's data) */
extern Gfx dochukiDemoShadowDl[];         /* a player's shadow's display list */

/* The demo players' work ("genoGOE", "chaSAS", "chaYAE"): the player task, its model, its
 * shadow and the face's two texture animations. */
typedef struct DemoPlW {
    /* 0x00 */ Obj *pl;
    /* 0x04 */ AnimeFrame *model;
    /* 0x08 */ PolFrame *shadow;
    /* 0x0C */ Anim eyes;
    /* 0x1C */ Anim mouth;
    /* 0x2C */ s32 chara;
    /* 0x30 */ u8 pad30[4];
    /* 0x34 */ s32 state;
} DemoPlW;

/* The file's .bss, in the game's order (IDO_NOTES 343): the file-scope words first, then the
 * characters' statics in function order (genoGOE's are globals). */
s32 dochukiDemoBisuShadowAlpha; /* the oni bearer's shadow's alpha (dochukiDemoBisuProc) */
s32 dochukiDemoUnused1; /* a word nothing reads */
f32 dochukiDemoBisuAnimSpeed; /* his animation speed from state 9 on */
s32 dochukiDemoUnused2; /* two words nothing reads */
s32 dochukiDemoUnused3;
f32 dochukiDemoDouAnimSpeed; /* set to 0.1 when the oni is made */
f32 dochukiDemoSasX; /* Sasuke's x and z, each frame */
f32 dochukiDemoSasZ;
f32 dochukiDemoGoeX; /* Goemon's x and z, each frame */
f32 dochukiDemoGoeZ;
f32 dochukiDemoEbiX; /* Ebisumaru's x and z, each frame */
f32 dochukiDemoEbiZ;
f32 dochukiDemoYaeX; /* Yae's x and z, each frame */
f32 dochukiDemoYaeZ;
f32 dochukiDemoBisuX; /* the oni bearer's x and z, each frame */
f32 dochukiDemoBisuZ;
f32 dochukiDemoOniX; /* the oni's x and z, each frame */
f32 dochukiDemoOniZ;
s32 dochukiDemoUnused4; /* three words nothing reads */
s32 dochukiDemoUnused5;
s32 dochukiDemoUnused6;
f32 dochukiDemoGoeAnimSpeed;     /* genoGOE's animation speed from state 5 on */
Player *dochukiDemoGoePlayer; /* his player */
s32 dochukiDemoUnused7;     /* a word nothing reads */

/* "genoGOE" (Goemon, character 0, in the demo) at (-69, -5000, 439) under the menu's last task;
 * OBJMOVE's model 0 moves him. His steps follow the file's frame counter (scenes 0xE7, 0xED,
 * 0xF5 and 0xFC).
 * MATCH (2026-10-11, IDO_NOTES 369): demoTexAnimK0 was called as int-returning for this
 * function: each pair's first call left a result in v0, so the address kept across it took v1,
 * as the game has. Before: real 48. 2026-10-17 (a kept FAKE): since
 * scRunEnd returns its Obj * (commit 53ddf08), the dropped results of its six calls do that, and
 * demoTexAnimK0 is called as the header declares it.
 * Old note: 196 lines, mix distance 0, frame and slots the game's: every demoTexAnimK0 call
 * pair keeps the function's address in v0 (spilled at 0x3C between the two calls) where the
 * game has v1; the twins chaYAE/genoGOE(Ebisumaru) with K3/K1 matched with v1. Measured:
 * "dochukiDemoGoePlayer = PL(w->pl)" then plEnterDemo(w->pl) 204 (the global store stops IDO forwarding
 * w->pl: the game passes plCreate's v0); "dochukiDemoGoePlayer = PL(w->pl = plCreate(..))" 204; a local
 * "Obj *pl" after init 256 (temps 4 low); "Obj *pl; s32 pad;" in place of pad[2] 196.
 * When it matches: the "goecreate" string of the 0x08009140 pragma is this function's first
 * PRINTF, so that pragma goes with it. */
s32 dochukiDemoGoeProc(Obj *obj, s16 msg, void *arg) {
    Obj *pl;
    s32 pad;
    DemoPlW *w = (DemoPlW *) (obj + 1);
    PlInit init;

    switch (msg) {
        case 2:
            PRINTF("goecreate\n");
            obj->name = "genoGOE";
            plMakeInit(&init, 0, 0);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = -69.0f;
            init.height = -5000.0f;
            init.side = 439.0f;
            w->pl = pl = plCreate(gDmenuLastObj, &init);
            dochukiDemoGoePlayer = PL(pl);
            plEnterDemo(pl);
            PRINTF("デモモード突入\n");
            PRINTF("歩きモーション\n");
            plAnimeSet(PL(w->pl), 0x105);
            w->model = dochukiDemoGoePlayer->model;
            PRINTF("*** p->model_buff=%x\n", w->model);
            PRINTF("ゴエモン作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, dochukiDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.8f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->chara = 0;
            w->model->rot[1] = 3.141592741f;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -69.0f;
            w->model->pos[1] = -5000.0f;
            w->model->pos[2] = 439.0f;
            w->model->rot[1] = 0.05235987902f;
            PRINTF("ゴエモン作りました\n");
            PRINTF("goecreate\n");
            break;
        case 1:
            frmDestroy(&w->model->f);
            frmDestroy(&w->shadow->f);
            tskDestroy(w->pl);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, 128);
            break;
        case 0:
            w->model->f.flags |= 2;
            if (gObjMoveSel == 0) {
                w->model->pos[0] += gObjMoveDx;
                w->model->pos[1] += gObjMoveDy;
                w->model->pos[2] += gObjMoveDz;
                w->model->rot[1] += gObjMoveDrot;
                gObjMovePosX = w->model->pos[0];
                gObjMovePosY = w->model->pos[1];
                gObjMovePosZ = w->model->pos[2];
                gObjMoveRot = w->model->rot[1];
                if (gGameFrameCount % 20 >= 10) {
                    w->model->f.flags |= 2;
                } else {
                    w->model->f.flags &= ~2;
                }
            }
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = 0.3f;
            dochukiDemoGoeX = w->model->pos[0];
            dochukiDemoGoeZ = w->model->pos[2];
            switch (w->state) {
                case 0:
                    w->model->scale[2] = 1.0f;
                    w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                    w->model->pos[0] = -69.0f;
                    w->model->pos[1] = -5000.0f;
                    w->model->pos[2] = 439.0f;
                    w->model->rot[1] = 0.05235987902f;
                    w->state++;
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.1f, 0);
                    switch (dochukiDemoFrame) {
                        case 0x186:
                            w->model->pos[1] -= 1000.0f;
                            break;
                        case 1000:
                            w->model->pos[0] = -69.0f;
                            w->model->pos[1] = -5000.0f;
                            w->model->pos[2] = 439.0f;
                            w->model->rot[1] = 0.05235987902f;
                            plAnimeSet(PL(w->pl), 0x10F);
                            demoTexAnimK0(&w->eyes, 3, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK0(&w->mouth, 3, &PL(w->pl)->model->segs[4]);
                            break;
                        case 0x406:
                            scRunEnd();
                            scRunStart(0xE7, 0, 0);
                            w->state++;
                            break;
                    }
                    break;
                case 2:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.5f, 0);
                    if (dochukiDemoFrame == 0x4F6) {
                        scRunEnd();
                        demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 3:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (dochukiDemoFrame == 0x5B6) {
                        plAnimeSet(PL(w->pl), 0x110);
                        demoTexAnimK0(&w->eyes, 8, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 8, &PL(w->pl)->model->segs[4]);
                        w->state++;
                        dochukiDemoGoeAnimSpeed = 0.0f;
                    }
                    break;
                case 4:
                    demoAnimeStep(w->model, 0.07f, 2);
                    if (dochukiDemoFrame >= 0x645) {
                        w->state++;
                    }
                    break;
                case 5:
                    if (dochukiDemoFrame == 0x127C) {
                        w->state++;
                        plAnimeSet(PL(w->pl), 0x111);
                        demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                        w->model->pos[0] = -69.0f;
                        w->model->pos[1] = -5000.0f;
                        w->model->pos[2] = 295.0f;
                        w->model->rot[1] = 0.05235987902f;
                    }
                    dochukiDemoGoeAnimSpeed = 0.1f;
                    break;
                case 6:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, dochukiDemoGoeAnimSpeed, 0);
                    switch (dochukiDemoFrame) {
                        case 0x12CC:
                            demoTexAnimK0(&w->eyes, 3, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK0(&w->mouth, 3, &PL(w->pl)->model->segs[4]);
                            scRunEnd();
                            scRunStart(0xED, 0, 0);
                            dochukiDemoGoeAnimSpeed = 0.3f;
                            break;
                        case 0x1394:
                            demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                            scRunEnd();
                            dochukiDemoGoeAnimSpeed = 0.1f;
                            break;
                        case 0x1C93:
                            demoTexAnimK0(&w->eyes, 3, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK0(&w->mouth, 3, &PL(w->pl)->model->segs[4]);
                            scRunEnd();
                            scRunStart(0xF5, 0, 0);
                            dochukiDemoGoeAnimSpeed = 0.3f;
                            break;
                        case 0x1D6F:
                            demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                            dochukiDemoGoeAnimSpeed = 0.1f;
                            scRunEnd();
                            break;
                        case 0x246D:
                            plAnimeSet(PL(w->pl), 0x112);
                            w->state++;
                            break;
                    }
                    break;
                case 7:
                    if (demoAnimeStep(w->model, 0.1f, 2)) {
                        plAnimeSet(PL(w->pl), 0x113);
                        scRunStart(0xFC, 0, 0);
                        w->state++;
                    }
                    break;
                case 8:
                    demoAnimeStep(w->model, 0.3f, 0);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 dochukiDemoGoeProc(Obj *obj, s16 msg, void *arg);

Obj *dochukiDemoGoeCreate(Obj *obj) {
    return tskCreate(obj, dochukiDemoGoeProc, obj->file, NULL);
}


/* "genoGOE" (Ebisumaru, character 1, in the demo) at (-203, -5000, 285) under the menu's last
 * task; OBJMOVE's model 1 moves him. His steps follow the file's frame counter (scene 0xF6 at
 * frame 0x1D6F). */
s32 dochukiDemoEbiProc(Obj *obj, s16 msg, void *arg) {
    static f32 dochukiDemoEbiAnimSpeed; /* his animation speed from state 4 on */
    s32 pad[2];
    DemoPlW *w = (DemoPlW *) (obj + 1);
    Player *p;
    PlInit init;

    switch (msg) {
        case 2:
            obj->name = "genoGOE";
            plMakeInit(&init, 0, 1);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = -203.0f;
            init.height = -5000.0f;
            init.side = 285.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            PRINTF("デモモード突入\n");
            PRINTF("歩きモーション\n");
            plAnimeSet(PL(w->pl), 0x105);
            w->model = p->model;
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, dochukiDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.9f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 1;
            w->state = 0;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -203.0f;
            w->model->pos[1] = -5000.0f;
            w->model->pos[2] = 285.0f;
            w->model->rot[1] = 0.0f;
            PRINTF("エビス丸作りました\n");
            PRINTF("goecreate\n");
            break;
        case 1:
            frmDestroy(&w->model->f);
            frmDestroy(&w->shadow->f);
            tskDestroy(w->pl);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, 128);
            break;
        case 0:
            w->model->f.flags |= 2;
            if (gObjMoveSel == 1) {
                w->model->pos[0] += gObjMoveDx;
                w->model->pos[1] += gObjMoveDy;
                w->model->pos[2] += gObjMoveDz;
                w->model->rot[1] += gObjMoveDrot;
                gObjMovePosX = w->model->pos[0];
                gObjMovePosY = w->model->pos[1];
                gObjMovePosZ = w->model->pos[2];
                gObjMoveRot = w->model->rot[1];
                if (gGameFrameCount % 20 >= 10) {
                    w->model->f.flags |= 2;
                } else {
                    w->model->f.flags &= ~2;
                }
            }
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = 0.3f;
            dochukiDemoEbiX = w->model->pos[0];
            dochukiDemoEbiZ = w->model->pos[2];
            switch (w->state) {
                case 0:
                    w->model->pos[0] = -203.0f;
                    w->model->pos[1] = -5000.0f;
                    w->model->pos[2] = 285.0f;
                    w->model->rot[1] = 0.0f;
                    scRunEnd();
                    w->state++;
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.1f, 0);
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (dochukiDemoFrame == 0x186) {
                        w->model->pos[1] -= 1000.0f;
                    }
                    if (dochukiDemoFrame == 1000) {
                        w->model->pos[0] = -203.0f;
                        w->model->pos[1] = -5000.0f;
                        w->model->pos[2] = 285.0f;
                        w->model->rot[1] = 0.0f;
                        w->state++;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->model, 0.1f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (dochukiDemoFrame == 0x5CA) {
                        plAnimeSet(PL(w->pl), 0x110);
                        w->state++;
                        dochukiDemoEbiAnimSpeed = 0.0f;
                    }
                    break;
                case 3:
                    demoAnimeStep(w->model, 0.05f, 2);
                    if (dochukiDemoFrame == 0x644) {
                        w->state++;
                    }
                    break;
                case 4:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (dochukiDemoFrame == 0x127C) {
                        plAnimeSet(PL(w->pl), 0x111);
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->model->pos[0] = -203.0f;
                        w->model->pos[1] = -5000.0f;
                        w->model->pos[2] = 157.0f;
                        w->model->rot[1] = 0.0f;
                        w->state++;
                        dochukiDemoEbiAnimSpeed = 0.1f;
                    }
                    break;
                case 5:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, dochukiDemoEbiAnimSpeed, 0);
                    switch (dochukiDemoFrame) {
                        case 0x1D6F:
                            scRunEnd();
                            scRunStart(0xF6, 0, 0);
                            demoTexAnimK1(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK1(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                            dochukiDemoEbiAnimSpeed = 0.3f;
                            break;
                        case 0x1E19:
                            demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                            dochukiDemoEbiAnimSpeed = 0.1f;
                            break;
                        case 0x2481:
                            plAnimeSet(PL(w->pl), 0x112);
                            w->state++;
                            break;
                    }
                    break;
                case 6:
                    if (demoAnimeStep(w->model, 0.1f, 2)) {
                        plAnimeSet(PL(w->pl), 0xD7);
                        w->state++;
                    }
                    break;
                case 7:
                    demoAnimeStep(w->model, 0.3f, 0);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 dochukiDemoEbiProc(Obj *obj, s16 msg, void *arg);

Obj *dochukiDemoEbiCreate(Obj *obj) {
    return tskCreate(obj, dochukiDemoEbiProc, obj->file, NULL);
}


/* "chaSAS" (Sasuke, character 2, in the demo) at (220, -5000, 437) under the menu's last task;
 * OBJMOVE's model 2 moves him. He runs in at frame 0x4F6 (scene 0xE8 when the run ends); his
 * steps follow the file's frame counter (scene 0xF7 at frame 0x1E37).
 * The run's step (0x080096F4) and his animation speed (0x080096F8) are statics of this
 * function: IDO then forms the step's address afresh at each use, as the game does (IDO_NOTES
 * 343). */
s32 dochukiDemoSasProc(Obj *obj, s16 msg, void *arg) {
    static s32 dochukiDemoSasRunStep; /* the run's step (demoPlTunagiAnime) */
    static f32 dochukiDemoSasAnimSpeed; /* his animation speed from state 7 on */
    s32 pad[2];
    DemoPlW *w = (DemoPlW *) (obj + 1);
    Player *p;
    PlInit init;

    switch (msg) {
        case 2:
            obj->name = "chaSAS";
            plMakeInit(&init, 0, 2);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = 220.0f;
            init.height = -5000.0f;
            init.side = 437.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            PRINTF("デモモード突入\n");
            PRINTF("走りモーション\n");
            plAnimeSet(PL(w->pl), 0x105);
            w->model = p->model;
            PRINTF("サスケ作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, dochukiDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.65f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 2;
            w->state = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 220.0f;
            w->model->pos[1] = -5000.0f;
            w->model->pos[2] = 437.0f;
            w->model->rot[1] = 0.0f;
            PRINTF("サスケ作りました\n");
            PRINTF("goecreate\n");
            break;
        case 1:
            frmDestroy(&w->model->f);
            frmDestroy(&w->shadow->f);
            tskDestroy(w->pl);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, 128);
            break;
        case 0:
            w->model->f.flags |= 2;
            if (gObjMoveSel == 2) {
                w->model->pos[0] += gObjMoveDx;
                w->model->pos[1] += gObjMoveDy;
                w->model->pos[2] += gObjMoveDz;
                w->model->rot[1] += gObjMoveDrot;
                gObjMovePosX = w->model->pos[0];
                gObjMovePosY = w->model->pos[1];
                gObjMovePosZ = w->model->pos[2];
                gObjMoveRot = w->model->rot[1];
                if (gGameFrameCount % 20 >= 10) {
                    w->model->f.flags |= 2;
                } else {
                    w->model->f.flags &= ~2;
                }
            }
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            dochukiDemoSasX = w->model->pos[0];
            dochukiDemoSasZ = w->model->pos[2];
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = 0.3f;
            switch (w->state) {
                case 0:
                    w->model->pos[0] = 220.0f;
                    w->model->pos[1] = -5000.0f;
                    w->model->pos[2] = 437.0f;
                    w->model->rot[1] = 0.0f;
                    w->state++;
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (dochukiDemoFrame == 0x186) {
                        w->model->pos[1] -= 1000.0f;
                    }
                    if (dochukiDemoFrame == 1000) {
                        w->model->pos[0] = 220.0f;
                        w->model->pos[1] = -5000.0f;
                        w->model->pos[2] = 437.0f;
                        w->model->rot[1] = 0.0f;
                    }
                    if (dochukiDemoFrame == 0x4F6) {
                        w->state++;
                        dochukiDemoSasRunStep++;
                    }
                    break;
                case 2:
                    dochukiDemoSasRunStep = demoPlTunagiAnime(obj, 2, dochukiDemoSasRunStep, 2, 2, 0.3f, 0.3f, 0x10C, 0x10D);
                    if (dochukiDemoSasRunStep == -1) {
                        scRunEnd();
                        scRunStart(0xE8, 0, 0);
                        dochukiDemoSasRunStep = 0;
                        w->state++;
                    }
                    break;
                case 3:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.5f, 0);
                    if (dochukiDemoFrame == 0x5A0) {
                        w->state++;
                    }
                    if (dochukiDemoFrame == 0x5C0) {
                        plAnimeSet(PL(w->pl), 0x110);
                        w->state = 6;
                        dochukiDemoSasAnimSpeed = 0.0f;
                    }
                    break;
                case 4:
                    dochukiDemoSasRunStep = demoPlTunagiAnime(obj, 0, dochukiDemoSasRunStep, 2, 3, 0.3f, 0.3f, 0x10C, 0x10B);
                    if (dochukiDemoSasRunStep == -1) {
                        scRunEnd();
                        w->state++;
                    }
                    if (dochukiDemoFrame == 0x5C0) {
                        plAnimeSet(PL(w->pl), 0x110);
                        w->state = 6;
                        dochukiDemoSasAnimSpeed = 0.0f;
                    }
                    break;
                case 5:
                    if (dochukiDemoFrame == 0x5C0) {
                        plAnimeSet(PL(w->pl), 0x110);
                        demoTexAnimK2(&w->eyes, 5, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 5, &PL(w->pl)->model->segs[4]);
                        w->state++;
                        dochukiDemoSasAnimSpeed = 0.0f;
                    }
                    demoAnimeStep(w->model, 0.1f, 2);
                    if (dochukiDemoFrame == 0x644) {
                        w->state = 7;
                    }
                    break;
                case 6:
                    demoAnimeStep(w->model, 0.1f, 2);
                    if (dochukiDemoFrame == 0x644) {
                        w->state++;
                    }
                    break;
                case 7:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (dochukiDemoFrame == 0x127C) {
                        w->state++;
                        plAnimeSet(PL(w->pl), 0x111);
                        demoTexAnimK2(&w->eyes, 3, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 3, &PL(w->pl)->model->segs[4]);
                        w->model->pos[0] = 220.0f;
                        w->model->pos[1] = -5000.0f;
                        w->model->pos[2] = 165.0f;
                        w->model->rot[1] = 0.0f;
                    }
                    dochukiDemoSasAnimSpeed = 0.1f;
                    break;
                case 8:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, dochukiDemoSasAnimSpeed, 0);
                    if (dochukiDemoFrame == 0x1E37) {
                        scRunEnd();
                        scRunStart(0xF7, 0, 0);
                        demoTexAnimK2(&w->eyes, 3, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 3, &PL(w->pl)->model->segs[4]);
                        dochukiDemoSasAnimSpeed = 0.3f;
                    }
                    if (dochukiDemoFrame == 0x1F13) {
                        demoTexAnimK2(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        scRunEnd();
                        dochukiDemoSasAnimSpeed = 0.1f;
                    }
                    if (dochukiDemoFrame == 0x246D) {
                        plAnimeSet(PL(w->pl), 0x112);
                        w->state++;
                    }
                    break;
                case 9:
                    if (demoAnimeStep(w->model, 0.1f, 2)) {
                        plAnimeSet(PL(w->pl), 0x113);
                        w->state++;
                    }
                    break;
                case 10:
                    demoAnimeStep(w->model, 0.3f, 0);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 dochukiDemoSasProc(Obj *obj, s16 msg, void *arg);

Obj *dochukiDemoSasCreate(Obj *obj) {
    return tskCreate(obj, dochukiDemoSasProc, obj->file, NULL);
}

s32 dochukiDemoUnused8; /* two words nothing reads */
s32 dochukiDemoUnused9;

/* "chaYAE": Yae in the demo, a player task of character 3 at (98, -5000, 267) under the menu's
 * last task; OBJMOVE's model 3 moves her. Her steps follow the file's frame counter (scene 0xE9
 * at frame 0x5C0). */
s32 dochukiDemoYaeProc(Obj *obj, s16 msg, void *arg) {
    static f32 dochukiDemoYaeUnusedF; /* set to 0 when she starts to move */
    s32 pad[2];
    DemoPlW *w = (DemoPlW *) (obj + 1);
    Player *p;
    PlInit init;

    switch (msg) {
        case 2:
            obj->name = "chaYAE";
            plMakeInit(&init, 0, 3);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = 98.0f;
            init.height = -5000.0f;
            init.side = 267.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            PRINTF("デモモード突入\n");
            PRINTF("立ちモーション\n");
            plAnimeSet(PL(w->pl), 0x105);
            w->model = p->model;
            PRINTF("ヤエ作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, dochukiDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.65f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 3;
            w->state = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 98.0f;
            w->model->pos[1] = -5000.0f;
            w->model->pos[2] = 267.0f;
            w->model->rot[1] = 0.0f;
            demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
            demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
            PRINTF("ヤエ作りました\n");
            break;
        case 1:
            frmDestroy(&w->model->f);
            frmDestroy(&w->shadow->f);
            tskDestroy(w->pl);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, 128);
            break;
        case 0:
            w->model->f.flags |= 2;
            if (gObjMoveSel == 3) {
                w->model->pos[0] += gObjMoveDx;
                w->model->pos[1] += gObjMoveDy;
                w->model->pos[2] += gObjMoveDz;
                w->model->rot[1] += gObjMoveDrot;
                gObjMovePosX = w->model->pos[0];
                gObjMovePosY = w->model->pos[1];
                gObjMovePosZ = w->model->pos[2];
                gObjMoveRot = w->model->rot[1];
                if (gGameFrameCount % 20 >= 10) {
                    w->model->f.flags |= 2;
                } else {
                    w->model->f.flags &= ~2;
                }
            }
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            dochukiDemoYaeX = w->model->pos[0];
            dochukiDemoYaeZ = w->model->pos[2];
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = 0.3f;
            switch (w->state) {
                case 0:
                    w->state++;
                    w->model->pos[0] = 98.0f;
                    w->model->pos[1] = -5000.0f;
                    w->model->pos[2] = 267.0f;
                    w->model->rot[1] = 0.0f;
                    break;
                case 1:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (dochukiDemoFrame == 0x186) {
                        w->model->pos[1] -= 1000.0f;
                    }
                    if (dochukiDemoFrame == 1000) {
                        w->model->pos[0] = 98.0f;
                        w->model->pos[1] = -5000.0f;
                        w->model->pos[2] = 267.0f;
                        w->model->rot[1] = 0.0f;
                        w->state++;
                    }
                    break;
                case 2:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (dochukiDemoFrame == 0x5C0) {
                        plAnimeSet(PL(w->pl), 0x110);
                        demoTexAnimK3(&w->eyes, 9, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 9, &PL(w->pl)->model->segs[4]);
                        w->state++;
                        dochukiDemoYaeUnusedF = 0.0f;
                    }
                    if (dochukiDemoFrame == 0x5C0) {
                        scRunEnd();
                        scRunStart(0xE9, 0, 0);
                    }
                    break;
                case 3:
                    demoAnimeStep(w->model, 0.1f, 2);
                    if (dochukiDemoFrame == 0x644) {
                        w->state = 5;
                    }
                    break;
                case 4:
                    demoAnimeStep(w->model, 0.1f, 2);
                    if (dochukiDemoFrame == 0x642) {
                        scRunEnd();
                    }
                    if (dochukiDemoFrame == 0x644) {
                        w->state++;
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (dochukiDemoFrame == 0x127C) {
                        plAnimeSet(PL(w->pl), 0x111);
                        demoTexAnimK3(&w->eyes, 7, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 7, &PL(w->pl)->model->segs[4]);
                        w->model->pos[0] = 98.0f;
                        w->model->pos[1] = -5000.0f;
                        w->model->pos[2] = 159.0f;
                        w->model->rot[1] = 0.0f;
                        w->state++;
                    }
                    break;
                case 6:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (dochukiDemoFrame == 0x2495) {
                        plAnimeSet(PL(w->pl), 0x112);
                        w->state++;
                    }
                    break;
                case 7:
                    if (demoAnimeStep(w->model, 0.1f, 2)) {
                        plAnimeSet(PL(w->pl), 0xD7);
                        w->state++;
                    }
                    break;
                case 8:
                    demoAnimeStep(w->model, 0.3f, 0);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 dochukiDemoYaeProc(Obj *obj, s16 msg, void *arg);

Obj *dochukiDemoYaeCreate(Obj *obj) {
    return tskCreate(obj, dochukiDemoYaeProc, obj->file, NULL);
}

/* "bisu"'s work: Bismaru's model and its shadow. */
typedef struct BisuW {
    /* 0x00 */ AnimeFrame *model;
    /* 0x04 */ PolFrame *shadow;
    /* 0x08 */ u8 pad08[4];
    /* 0x0C */ s32 state;
} BisuW;

extern u8 dochukiDemoBisuAnim[]; /* Bismaru's animations */
extern u8 dochukiDemoBisuFrame390Anim[];
extern u8 dochukiDemoBisuState5Anim[];
extern u8 dochukiDemoBisuState8Anim[];
extern u8 dochukiDemoBisuState9Anim[];

/* "bisu" (Bismaru, an anime model of file 0x12C/0xC3) at (522, 147, 1289); OBJMOVE's model 4
 * moves him. His animations and the scenes 0xE4-0xFB follow the file's frame counter. */
s32 dochukiDemoBisuProc(Obj *obj, s16 msg, void *arg) {
    s32 pad[2];
    BisuW *w = (BisuW *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "bisu";
            PRINTF("ビス丸作ります\n");
            w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->model == NULL) {
                return -1;
            }
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoAnimeSet(w->model, (u32) dochukiDemoBisuAnim, 0x12C, 0);
            demoPolSetDL(w->shadow, dochukiDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.8f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 522.0f;
            w->model->pos[1] = 147.0f;
            w->model->pos[2] = 1289.0f;
            w->model->rot[1] = 0.0f;
            PRINTF("ビス丸作りました\n");
            break;
        case 1:
            frmDestroy(&w->model->f);
            frmDestroy(&w->shadow->f);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, dochukiDemoBisuShadowAlpha);
            break;
        case 0:
            w->model->f.flags |= 2;
            if (gObjMoveSel == 4) {
                w->model->pos[0] += gObjMoveDx;
                w->model->pos[1] += gObjMoveDy;
                w->model->pos[2] += gObjMoveDz;
                w->model->rot[1] += gObjMoveDrot;
                gObjMovePosX = w->model->pos[0];
                gObjMovePosY = w->model->pos[1];
                gObjMovePosZ = w->model->pos[2];
                gObjMoveRot = w->model->rot[1];
                if (gGameFrameCount % 20 >= 10) {
                    w->model->f.flags |= 2;
                } else {
                    w->model->f.flags &= ~2;
                }
            }
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            switch (w->state) {
                case 0:
                    w->state++;
                    break;
                case 1:
                    if (dochukiDemoFrame == 150) {
                        w->model->pos[0] = 522.0f;
                        w->model->pos[1] = 147.0f;
                        w->model->pos[2] = 1289.0f;
                        w->model->rot[1] = 0.0f;
                        w->state++;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->model, 0.3f, 0);
                    switch (dochukiDemoFrame) {
                        case 0xA0:
                            scRunStart(0xE4, 0, 0);
                            break;
                        case 0x186:
                            demoAnimeSet(w->model, (u32) dochukiDemoBisuFrame390Anim, 0xC3, 0);
                            break;
                        case 0x1A4:
                            scRunEnd();
                            scRunStart(0xE5, 0, 0);
                            break;
                        case 0x2BC:
                            scRunEnd();
                            scRunStart(0xE6, 0, 0);
                            break;
                        case 1000:
                            scRunEnd();
                            w->state++;
                            break;
                    }
                    break;
                case 3:
                    if (dochukiDemoFrame == 0x644) {
                        w->state++;
                    }
                    break;
                case 4:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (dochukiDemoFrame == 0x658) {
                        scRunStart(0xEA, 0, 0);
                        w->state++;
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (dochukiDemoFrame == 0x7A2) {
                        scRunEnd();
                        scRunStart(0xEB, 0, 0);
                        demoAnimeSet(w->model, (u32) dochukiDemoBisuState5Anim, 0xC3, 0);
                        w->state++;
                    }
                    break;
                case 6:
                    demoAnimeStep(w->model, 0.2f, 2);
                    if (dochukiDemoFrame == 0x87E) {
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 7:
                    demoAnimeStep(w->model, 0.2f, 2);
                    if (dochukiDemoFrame == 0x1178) {
                        w->state++;
                    }
                    break;
                case 8:
                    if (dochukiDemoFrame >= 0x13E5) {
                        demoAnimeSet(w->model, (u32) dochukiDemoBisuState8Anim, 0xC3, 0);
                        w->model->pos[0] = 566.0f;
                        w->model->pos[1] = 147.0f;
                        w->model->pos[2] = 1393.0f;
                        w->model->rot[1] = 3.577924967f;
                        w->state++;
                        dochukiDemoBisuAnimSpeed = 0.2f;
                    }
                    break;
                case 9:
                    demoAnimeStep(w->model, dochukiDemoBisuAnimSpeed, 0);
                    if (dochukiDemoFrame == 0x1628) {
                        scRunStart(0xF0, 0, 0);
                        dochukiDemoBisuAnimSpeed = 0.4f;
                    }
                    if (dochukiDemoFrame == 0x17A4) {
                        scRunEnd();
                        scRunStart(0xF1, 0, 0);
                        dochukiDemoBisuAnimSpeed = 0.6f;
                    }
                    if (dochukiDemoFrame == 0x18EE) {
                        dochukiDemoBisuAnimSpeed = 0.1f;
                    }
                    if (dochukiDemoFrame == 0x1A2E) {
                        scRunEnd();
                        scRunStart(0xF3, 0, 0);
                        dochukiDemoBisuAnimSpeed = 0.3f;
                    }
                    if (dochukiDemoFrame == 0x1AE2) {
                        scRunEnd();
                        scRunStart(0xF4, 0, 0);
                        dochukiDemoBisuAnimSpeed = 0.5f;
                    }
                    if (dochukiDemoFrame == 0x1F45) {
                        demoAnimeSet(w->model, (u32) dochukiDemoBisuState9Anim, 0xC3, 0);
                        scRunEnd();
                        w->model->pos[0] = 154.0f;
                        w->model->pos[1] = 1459.0f;
                        w->model->pos[2] = 37.0f;
                        w->model->rot[1] = -0.1396263391f;
                        w->state++;
                        dochukiDemoBisuAnimSpeed = 0.1f;
                    }
                    break;
                case 10:
                    demoAnimeStep(w->model, dochukiDemoBisuAnimSpeed, 0);
                    switch (dochukiDemoFrame) {
                        case 0x1F59:
                            scRunStart(0xF8, 0, 0);
                            break;
                        case 0x203F:
                            scRunEnd();
                            scRunStart(0xF9, 0, 0);
                            dochukiDemoBisuAnimSpeed = 0.5f;
                            break;
                        case 0x2157:
                            dochukiDemoBisuAnimSpeed = 0.3f;
                            scRunEnd();
                            scRunStart(0xFA, 0, 0);
                            dochukiDemoBisuAnimSpeed = 0.3f;
                            break;
                        case 0x2265:
                            scRunEnd();
                            scRunStart(0xFB, 0, 0);
                            dochukiDemoBisuAnimSpeed = 0.6f;
                            break;
                        case 0x232D:
                            w->model->pos[2] = -1000.0f;
                            break;
                    }
                    break;
            }
            dochukiDemoBisuX = w->model->pos[0];
            dochukiDemoBisuZ = w->model->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 dochukiDemoBisuProc(Obj *obj, s16 msg, void *arg);

Obj *dochukiDemoBisuCreate(Obj *obj) {
    return tskCreate(obj, dochukiDemoBisuProc, obj->file, NULL);
}

extern u8 demoPieceTex0[];
extern u8 demoPieceTex1[];
extern u8 demoPieceTex2[];
extern u8 demoPieceTex4[];
extern u8 demoPieceTex7[];
extern u8 demoPieceTex15[];
extern u8 demoPieceTex19[];
extern u8 demoPieceTex78[];
extern u8 demoFaceTex0[];
extern u8 demoFaceTex1[];
extern u8 demoFaceTex2[];
extern u8 dochukiDemoSmokeTex0[];
extern u8 dochukiDemoSmokeTex1[];
extern u8 dochukiDemoSmokeTex2[];
extern u8 dochukiDemoSmokeTex3[];
extern u8 dochukiDemoSmokeTex4[];
extern u8 dochukiDemoSmokeTex6[];
extern u8 dochukiDemoOniTex0[];
extern u8 dochukiDemoOniTex1[];
/* This file's .data (0x08008570), in address order: the characters' texture animations
 * (demotool.c's table, as tlb_0020_1's; nothing reads it), the oni's, then the smoke's (below). */
AnimFrame dochukiDemoCharaAnims[] = {
    { (u32) demoFaceTex2, 8 },
    { 0, 0 },
    { 0x04002100, 8 },
    { 0x04002940, 6 },
    { 0x04003180, 5 },
    { (u32) demoFaceTex1, 5 },
    { 0x04001080, 6 },
    { (u32) demoFaceTex1, 8 },
    { 0x04003180, 6 },
    { 0x04002940, 7 },
    { 0x04002100, 8 },
    { 0, 0 },
    { 0x04002520, 13 },
    { 0x04002D60, 13 },
    { 0, 0 },
    { 0x040035A0, 13 },
    { 0, 0 },
    { 0x03005350, 13 },
    { 0, 0 },
    { 0x03005770, 13 },
    { 0, 0 },
    { 0x03001570, 7 },
    { (u32) demoPieceTex0, 6 },
    { 0x03001570, 7 },
    { (u32) demoPieceTex2, 7 },
    { (u32) demoPieceTex1, 7 },
    { (u32) demoPieceTex0, 8 },
    { 0x03001570, 7 },
    { (u32) demoPieceTex2, 9 },
    { (u32) dochukiDemoSmokeTex1, 8 },
    { (u32) demoPieceTex4, 8 },
    { (u32) demoPieceTex1, 7 },
    { 0x03001570, 7 },
    { (u32) demoPieceTex2, 8 },
    { (u32) demoPieceTex1, 9 },
    { 0x03001570, 7 },
    { (u32) demoPieceTex0, 7 },
    { (u32) demoPieceTex2, 6 },
    { 0x03001570, 7 },
    { (u32) demoPieceTex0, 7 },
    { 0x03001570, 7 },
    { 0, 0 },
    { 0x03002E30, 7 },
    { 0x03001990, 6 },
    { 0x030021D0, 7 },
    { (u32) demoPieceTex7, 7 },
    { 0x03002E30, 7 },
    { 0x03001990, 8 },
    { 0x030021D0, 9 },
    { 0x030025F0, 8 },
    { 0x03002A10, 8 },
    { 0x03002E30, 7 },
    { (u32) demoPieceTex7, 7 },
    { 0x030021D0, 8 },
    { (u32) demoPieceTex7, 9 },
    { 0x03002E30, 7 },
    { 0x03001990, 7 },
    { 0x030021D0, 6 },
    { 0x03002E30, 7 },
    { 0x03001990, 7 },
    { 0x03002E30, 7 },
    { 0, 0 },
    { 0x030046F0, 7 },
    { 0x03003250, 6 },
    { 0x030046F0, 7 },
    { 0x03003A90, 7 },
    { 0x03003670, 7 },
    { 0x03003250, 8 },
    { 0x030046F0, 7 },
    { 0x03003A90, 9 },
    { (u32) demoPieceTex15, 8 },
    { 0x030042D0, 8 },
    { 0x030046F0, 7 },
    { 0x03003670, 7 },
    { 0x03003A90, 8 },
    { 0x030046F0, 7 },
    { 0x03003670, 9 },
    { 0x03003250, 7 },
    { 0x030046F0, 7 },
    { 0x03003A90, 6 },
    { 0x03003250, 7 },
    { 0x030046F0, 7 },
    { 0, 0 },
    { 0x03001570, 2 },
    { (u32) dochukiDemoSmokeTex1, 7 },
    { 0x03001570, 3 },
    { (u32) demoPieceTex1, 7 },
    { (u32) demoPieceTex2, 7 },
    { 0x03001570, 2 },
    { (u32) demoPieceTex0, 7 },
    { (u32) demoPieceTex2, 8 },
    { 0, 0 },
    { (u32) demoPieceTex4, 40 },
    { (u32) demoPieceTex2, 7 },
    { (u32) demoPieceTex1, 7 },
    { (u32) demoPieceTex0, 70 },
    { (u32) demoPieceTex4, 10 },
    { (u32) demoPieceTex4, 10 },
    { (u32) demoPieceTex0, 10 },
    { (u32) dochukiDemoSmokeTex1, 200 },
    { 0, 0 },
    { (u32) demoPieceTex19, 60 },
    { 0x03004B10, 7 },
    { (u32) demoPieceTex19, 10 },
    { 0x03004B10, 7 },
    { (u32) demoPieceTex19, 40 },
    { 0x03004B10, 7 },
    { (u32) demoPieceTex19, 40 },
    { 0, 0 },
    { (u32) demoFaceTex0, 8 },
    { (u32) demoFaceTex1, 6 },
    { 0x04000C60, 5 },
    { 0x04001080, 5 },
    { 0x04001080, 8 },
    { 0x04000C60, 6 },
    { (u32) demoFaceTex1, 7 },
    { (u32) demoFaceTex0, 8 },
    { 0, 0 },
    { 0x0300A5D0, 13 },
    { 0, 0 },
    { 0x040018C0, 13 },
    { 0, 0 },
    { 0x03007030, 2 },
    { 0x03006C10, 7 },
    { 0x03005FB0, 570 },
    { 0, 0 },
    { 0x03005B90, 7 },
    { 0x030063D0, 7 },
    { 0x03007030, 7 },
    { 0x03005FB0, 7 },
    { 0x03007030, 7 },
    { 0x03005B90, 7 },
    { 0x03007030, 7 },
    { 0x030063D0, 7 },
    { 0x030067F0, 7 },
    { 0x03006C10, 7 },
    { 0x03007030, 7 },
    { 0x03005FB0, 7 },
    { 0x030063D0, 7 },
    { 0x03005FB0, 7 },
    { 0x03007030, 7 },
    { 0x03005B90, 7 },
    { 0x030063D0, 7 },
    { 0x03007030, 7 },
    { 0x03005B90, 7 },
    { 0x03007030, 7 },
    { 0, 0 },
    { 0x03007450, 7 },
    { 0x03007C90, 7 },
    { 0x030088F0, 7 },
    { 0x03007870, 7 },
    { 0x030088F0, 7 },
    { 0x03007450, 7 },
    { 0x030088F0, 7 },
    { 0x03007C90, 7 },
    { 0x030080B0, 7 },
    { 0x030084D0, 7 },
    { 0x030088F0, 7 },
    { 0x03007870, 7 },
    { 0x03007C90, 7 },
    { 0x03007870, 7 },
    { 0x030088F0, 7 },
    { 0x03007450, 7 },
    { 0x03007C90, 7 },
    { 0x030088F0, 7 },
    { 0x03007450, 7 },
    { 0x030088F0, 7 },
    { 0, 0 },
    { 0x03008D10, 7 },
    { 0x03009550, 7 },
    { 0x0300A1B0, 7 },
    { 0x03009130, 7 },
    { 0x0300A1B0, 7 },
    { 0x03008D10, 7 },
    { 0x0300A1B0, 7 },
    { 0x03009550, 7 },
    { 0x03009970, 7 },
    { 0x03009D90, 7 },
    { 0x0300A1B0, 7 },
    { 0x03009130, 7 },
    { 0x03009550, 7 },
    { 0x03009130, 7 },
    { 0x0300A1B0, 7 },
    { 0x03008D10, 7 },
    { 0x03009550, 7 },
    { 0x0300A1B0, 7 },
    { 0x03008D10, 7 },
    { 0x0300A1B0, 7 },
    { 0, 0 },
    { 0x04000C60, 8 },
    { 0x04001080, 6 },
    { 0x04001CE0, 5 },
    { 0x04002520, 5 },
    { 0x04002520, 8 },
    { 0x04001CE0, 6 },
    { 0x04001080, 7 },
    { 0x04000C60, 8 },
    { 0, 0 },
    { 0x040018C0, 13 },
    { 0, 0 },
    { 0x040014A0, 13 },
    { 0, 0 },
    { 0x0300A9F0, 3 },
    { 0x0300B230, 4 },
    { 0x0300AE10, 4 },
    { 0x0300A9F0, 3 },
    { 0x0300B230, 4 },
    { 0x0300B650, 4 },
    { 0x0300BA70, 2 },
    { 0x0300AE10, 4 },
    { 0x0300B230, 4 },
    { 0x0300AE10, 4 },
    { 0x0300A9F0, 3 },
    { 0x0300B230, 4 },
    { 0x0300A9F0, 3 },
    { 0, 0 },
    { 0x0300C2B0, 3 },
    { 0x0300CAF0, 4 },
    { 0x0300C6D0, 4 },
    { 0x0300C2B0, 3 },
    { 0x0300CAF0, 4 },
    { 0x0300CF10, 4 },
    { 0x0300D330, 2 },
    { 0x0300C6D0, 4 },
    { 0x0300CAF0, 4 },
    { 0x0300C6D0, 4 },
    { 0x0300C2B0, 3 },
    { 0x0300CAF0, 4 },
    { 0x0300C2B0, 3 },
    { 0, 0 },
    { 0x0300DB70, 3 },
    { 0x0300E3B0, 4 },
    { 0x0300DF90, 4 },
    { 0x0300DB70, 3 },
    { 0x0300E3B0, 4 },
    { 0x0300E7D0, 4 },
    { 0x0300EBF0, 2 },
    { 0x0300DF90, 4 },
    { 0x0300E3B0, 4 },
    { 0x0300DF90, 4 },
    { 0x0300DB70, 3 },
    { 0x0300E3B0, 4 },
    { 0x0300DB70, 3 },
    { 0, 0 },
    { (u32) demoFaceTex1, 52 },
    { 0x04000C60, 1 },
    { 0x040014A0, 2 },
    { 0x04002100, 4 },
    { (u32) demoFaceTex1, 52 },
    { 0x04000C60, 1 },
    { 0x040014A0, 2 },
    { 0x04002100, 4 },
    { (u32) demoFaceTex1, 5 },
    { 0x04000C60, 1 },
    { 0x040014A0, 2 },
    { 0x04002100, 4 },
    { (u32) demoFaceTex1, 42 },
    { 0, 0 },
    { (u32) demoFaceTex1, 10 },
    { 0x04000C60, 10 },
    { 0x040014A0, 400 },
    { 0x040018C0, 3 },
    { 0x04001080, 3 },
    { (u32) demoFaceTex2, 3 },
    { 0x04001080, 3 },
    { 0x040018C0, 3 },
    { 0x04002520, 3 },
    { 0x04002520, 3 },
    { 0, 0 },
    { (u32) demoPieceTex78, 13 },
    { 0, 0 },
    { 0x030146B0, 13 },
    { 0, 0 },
    { 0x0300F850, 6 },
    { 0x03010CF0, 4 },
    { 0x03010090, 8 },
    { 0x03010CF0, 4 },
    { 0x0300FC70, 10 },
    { 0x03010CF0, 4 },
    { 0x0300F850, 12 },
    { 0x03010090, 8 },
    { 0x030104B0, 8 },
    { 0x03010CF0, 4 },
    { 0x030108D0, 8 },
    { 0x03010CF0, 3 },
    { 0, 0 },
    { 0x03011110, 4 },
    { 0x030125B0, 2 },
    { 0x03011950, 5 },
    { 0x030125B0, 2 },
    { 0x03011530, 6 },
    { 0x030125B0, 2 },
    { 0x03011110, 6 },
    { 0x03011950, 5 },
    { 0x03011D70, 5 },
    { 0x030125B0, 4 },
    { 0x03012190, 2 },
    { 0x030125B0, 2 },
    { 0, 0 },
    { 0x030125B0, 2 },
    { 0x03012190, 15 },
    { 0x03011950, 15 },
    { 0x03011530, 16 },
    { 0x03011950, 15 },
    { 0x03011530, 400 },
    { 0, 0 },
    { 0x030125B0, 2000 },
    { 0, 0 },
    { 0x030129D0, 4 },
    { 0x03013E70, 2 },
    { 0x03013210, 5 },
    { 0x03013E70, 2 },
    { 0x03012DF0, 6 },
    { 0x03013E70, 2 },
    { 0x030129D0, 6 },
    { 0x03013210, 5 },
    { 0x03013630, 5 },
    { 0x03013E70, 4 },
    { 0x03013A50, 2 },
    { 0x03013E70, 2 },
    { 0, 0 },
    { 0x03014AD0, 2000 },
    { 0, 0 },
};
AnimFrame dochukiDemoOniAnim[] = { /* the oni's texture animation */
    { (u32) dochukiDemoOniTex0, 7 },
    { (u32) dochukiDemoOniTex1, 12 },
    { (u32) dochukiDemoOniTex0, 3 },
    { (u32) dochukiDemoOniTex1, 22 },
    { (u32) dochukiDemoOniTex0, 3 },
    { (u32) dochukiDemoOniTex1, 16 },
    { (u32) dochukiDemoOniTex0, 5 },
    { (u32) dochukiDemoOniTex1, 12 },
    { (u32) dochukiDemoOniTex0, 13 },
    { (u32) dochukiDemoOniTex1, 14 },
    { 0, 0 },
};
extern u8 dochukiDemoDouAnim[]; /* the oni's animations */
extern u8 dochukiDemoDouState0Anim[];
extern u8 dochukiDemoDouState1Anim[];
extern u8 dochukiDemoDouState3Anim[];

/* "dou"'s work. */
typedef struct DouW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ u8 pad04[4];
    /* 0x08 */ Anim anim; /* the texture animation */
    /* 0x18 */ u8 pad18[4];
    /* 0x1C */ s32 state;
} DouW;

/* "dou" (the Douchuu oni, file 0x29): OBJMOVE's model 6 moves it. It drops in from 1300 up at
 * frame 0xCCA, then steps through its animations by the file's frame counter, starting the
 * scenes 0xEE, 0xEF and 0xF2 on the way; gone at frame 0x232D. */
s32 dochukiDemoDouProc(Obj *obj, s16 msg, void *arg) {
    DouW *w = (DouW *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "dou";
            PRINTF("ドウチュウ鬼作ります\n");
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            demoAnimeSet(w->frame, (u32) dochukiDemoDouAnim, 0x29, 0);
            fadeAnimStart(&w->anim, dochukiDemoOniAnim, 0, &w->frame->segs[3], floadGetSetFile(0x29, 0));
            w->state = 0;
            w->frame->scale[2] = 1.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 0.0f;
            w->frame->rot[1] = 0.0f;
            dochukiDemoDouAnimSpeed = 0.1f;
            PRINTF("ドウチュウ鬼作りました\n");
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, gDrawSetupEnvTintTexShadeDl);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 0);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            w->frame->f.flags |= 2;
            if (gObjMoveSel == 6) {
                w->frame->pos[0] += gObjMoveDx;
                w->frame->pos[1] += gObjMoveDy;
                w->frame->pos[2] += gObjMoveDz;
                w->frame->rot[1] += gObjMoveDrot;
                gObjMovePosX = w->frame->pos[0];
                gObjMovePosY = w->frame->pos[1];
                gObjMovePosZ = w->frame->pos[2];
                gObjMoveRot = w->frame->rot[1];
                if (gGameFrameCount % 20 >= 10) {
                    w->frame->f.flags |= 2;
                } else {
                    w->frame->f.flags &= ~2;
                }
            }
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            dochukiDemoOniX = w->frame->pos[0];
            dochukiDemoOniZ = w->frame->pos[2];
            switch (w->state) {
                case 0:
                    if (dochukiDemoFrame == 0xCCA) {
                        demoAnimeSet(w->frame, (u32) dochukiDemoDouState0Anim, 0x29, 0);
                        fadeAnimStart(&w->anim, dochukiDemoOniAnim, 0, &w->frame->segs[3],
                                      floadGetSetFile(0x29, 0));
                        w->state++;
                        w->frame->pos[1] = 1300.0f;
                    }
                    if (w->frame->pos[1] < 1300.0f && dochukiDemoFrame >= 0xAD7) {
                        w->frame->pos[1] += 2.6;
                    }
                    break;
                case 1:
                    demoAnimeStep(w->frame, 0.06f, 0);
                    if (dochukiDemoFrame == 0xFE8) {
                        w->frame->frame += 4.2;
                    }
                    if (dochukiDemoFrame == 0x10B0) {
                        demoAnimeSet(w->frame, (u32) dochukiDemoDouState1Anim, 0x29, 0);
                        fadeAnimStart(&w->anim, dochukiDemoOniAnim, 0, &w->frame->segs[3],
                                      floadGetSetFile(0x29, 0));
                        w->state++;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->frame, 0.154f, 2);
                    if (dochukiDemoFrame == 0x127C) {
                        w->state++;
                    }
                    break;
                case 3:
                    if (dochukiDemoFrame == 0x13E4) {
                        demoAnimeSet(w->frame, (u32) dochukiDemoDouState3Anim, 0x29, 0);
                        fadeAnimStart(&w->anim, dochukiDemoOniAnim, 0, &w->frame->segs[3],
                                      floadGetSetFile(0x29, 0));
                        w->state++;
                    }
                    break;
                case 4:
                    demoAnimeStep(w->frame, 0.1f, 0);
                    if (dochukiDemoFrame == 0x1402) {
                        scRunStart(0xEE, 0, 0);
                    }
                    if (dochukiDemoFrame == 0x14AC) {
                        scRunEnd();
                        scRunStart(0xEF, 0, 0);
                    }
                    if (dochukiDemoFrame > 0x14AC && dochukiDemoFrame < 0x1588) {
                        fadeAnimStep(&w->anim);
                    }
                    if (dochukiDemoFrame == 0x1588) {
                        fadeAnimStart(&w->anim, dochukiDemoOniAnim, 0, &w->frame->segs[3],
                                      floadGetSetFile(0x29, 0));
                    }
                    if (dochukiDemoFrame == 0x1902) {
                        scRunEnd();
                        scRunStart(0xF2, 0, 0);
                    }
                    if (dochukiDemoFrame == 0x232D) {
                        return -1;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 dochukiDemoDouProc(Obj *obj, s16 msg, void *arg);

Obj *dochukiDemoDouCreate(Obj *obj) {
    return tskCreate(obj, dochukiDemoDouProc, obj->file, NULL);
}

/* "MACH12"'s work: one anime frame and its animation speed. */
typedef struct Mach12W {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 speed;
} Mach12W;

extern u8 dochukiDemoMachineAnim[]; /* the machine's animation */

/* "MACH12": a machine (anime of file 0xC9, scale 10) at the origin; OBJMOVE's model 5 moves it.
 * At frame 150 it drops to height 0, at 2274 it starts the next task, and from 2275 on it
 * animates. */
s32 dochukiDemoMach12Proc(Obj *obj, s16 msg, void *arg) {
    Mach12W *w = (Mach12W *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "MACH12";
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            demoAnimeSet(w->frame, (u32) dochukiDemoMachineAnim, 0xC9, 0.0f);
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 0.0f;
            w->frame->scale[2] = 10.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->rot[1] = 0.0f;
            w->speed = 0.3f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = demoShadeDL();
            break;
        case 0:
            w->frame->f.flags |= 2;
            if (gObjMoveSel == 5) {
                w->frame->pos[0] += gObjMoveDx;
                w->frame->pos[1] += gObjMoveDy;
                w->frame->pos[2] += gObjMoveDz;
                w->frame->rot[1] += gObjMoveDrot;
                gObjMovePosX = w->frame->pos[0];
                gObjMovePosY = w->frame->pos[1];
                gObjMovePosZ = w->frame->pos[2];
                gObjMoveRot = w->frame->rot[1];
                if (gGameFrameCount % 20 >= 10) {
                    w->frame->f.flags |= 2;
                } else {
                    w->frame->f.flags &= ~2;
                }
            }
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            switch (dochukiDemoFrame) {
                case 150:
                    w->frame->pos[1] = 0.0f;
                    break;
                case 1000:
                case 1604:
                    break;
                case 2274:
                    (*dochukiDemoDouCreate)(gDmenuLastObj);
                    break;
            }
            if (dochukiDemoFrame >= 0x8E3) {
                demoAnimeStep(w->frame, 0.024f, 2);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 dochukiDemoMach12Proc(Obj *obj, s16 msg, void *arg);

Obj *dochukiDemoMach12Create(Obj *obj) {
    return tskCreate(obj, dochukiDemoMach12Proc, obj->file, NULL);
}

/* The smoke tasks' work ("SMOKE", "KEMURI"); also KEMURI's create argument. */
typedef struct Tlb27KemuriWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ Anim anim;
    /* 0x14 */ f32 pos[3];
    /* 0x20 */ f32 scale;
    /* 0x24 */ f32 grow;
    /* 0x28 */ f32 rise;   /* added to the frame's height each frame */
    /* 0x2C */ s32 alpha;  /* the env colour's alpha */
    /* 0x30 */ s32 period;
    /* 0x34 */ s32 fade;   /* taken off alpha every period frames */
} Tlb27KemuriWork;

extern Gfx dochukiDemoSmokeDl[];               /* the smoke's display list */
AnimFrame dochukiDemoSmokeAnim[] = { /* the smoke's texture animation (this file's last .data) */
    { (u32) dochukiDemoSmokeTex0, 16 },
    { (u32) dochukiDemoSmokeTex1, 16 },
    { (u32) dochukiDemoSmokeTex2, 16 },
    { (u32) dochukiDemoSmokeTex3, 16 },
    { (u32) dochukiDemoSmokeTex4, 16 },
    { 0x03002D30, 18 },
    { (u32) dochukiDemoSmokeTex6, 10 },
    { 0, 0 },
};

/* "SMOKE": a smoke frame at a random x, falling 8 a frame and fading 3, its alpha from the
 * file's counter; gone at alpha 0 or below -198. */
s32 dochukiDemoSmokeProc(Obj *obj, s16 msg, void *arg) {
    Tlb27KemuriWork *w = (Tlb27KemuriWork *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "SMOKE";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, dochukiDemoSmokeDl, 0x123);
            fadeAnimStart(&w->anim, dochukiDemoSmokeAnim, 0, &w->frame->segs[3],
                          floadGetSetFile(0x123, 0));
            w->frame->flags |= 0x27;
            w->frame->scale[2] = 7.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = fadeRandRange(-243, 273);
            w->frame->pos[1] = -4900.0f;
            w->frame->pos[2] = 1000.0f;
            if (dochukiDemoFrame < 0x624) {
                w->alpha = dochukiDemoFrame + 0x476;
            } else {
                return -1;
            }
            w->period = 0;
            if (w->alpha > 128) {
                w->alpha = 128;
            }
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = demoColorDL(230, 90, 90, 100, 30, 50, w->alpha);
            break;
        case 0:
            fadeAnimStep(&w->anim);
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            w->alpha -= 3;
            w->frame->pos[2] -= 8.0f;
            if (w->alpha > 128) {
                w->alpha = 128;
            }
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            if (w->frame->pos[2] < -198.0f) {
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 dochukiDemoSmokeProc(Obj *obj, s16 msg, void *arg);

Obj *dochukiDemoSmokeCreate(Obj *obj) {
    return tskCreate(obj, dochukiDemoSmokeProc, obj->file, NULL);
}

/* "KEMURI" (smoke): a pol frame of file 0x123 with an animated texture, rising while its alpha
 * fades every `period` frames of the file's counter. */
s32 dochukiDemoKemuriProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    Tlb27KemuriWork *w = (Tlb27KemuriWork *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "KEMURI";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, dochukiDemoSmokeDl, 0x123);
            fadeAnimStart(&w->anim, dochukiDemoSmokeAnim, 0, &w->frame->segs[3], floadGetSetFile(0x123, 0));
            w->frame->flags |= 0x20;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = ((Tlb27KemuriWork *) arg)->scale;
            w->frame->pos[0] = ((Tlb27KemuriWork *) arg)->pos[0];
            w->frame->pos[1] = ((Tlb27KemuriWork *) arg)->pos[1];
            if (dochukiDemoFrame >= 0x1615 && dochukiDemoFrame < 0x18EE) {
                w->frame->pos[1] += 600.0f;
            }
            w->frame->pos[2] = ((Tlb27KemuriWork *) arg)->pos[2];
            w->rise = ((Tlb27KemuriWork *) arg)->rise;
            w->frame->flags |= 7;
            w->alpha = ((Tlb27KemuriWork *) arg)->alpha;
            w->period = ((Tlb27KemuriWork *) arg)->period;
            w->fade = ((Tlb27KemuriWork *) arg)->fade;
            w->grow = ((Tlb27KemuriWork *) arg)->grow;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = demoColorDL(230, 90, 90, 100, 30, 50, w->alpha);
            break;
        case 0:
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            if (dochukiDemoFrame == 0x18EE) {
                return -1;
            }
            if (!(dochukiDemoFrame & 1)) {
                fadeAnimStep(&w->anim);
            }
            if (dochukiDemoFrame % w->period == 0) {
                w->alpha -= w->fade;
            }
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            w->frame->pos[1] += w->rise;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "KEMURI"'s (dochukiDemoKemuriProc) argument, laid out as tlb_0020_1's KemuriWork. */
typedef struct Tlb27KemuriArg {
    /* 0x00 */ u8 pad00[0x14];
    /* 0x14 */ f32 pos[3];
    /* 0x20 */ f32 scale;
    /* 0x24 */ f32 rise;
    /* 0x28 */ f32 grow;
    /* 0x2C */ s32 alpha;
    /* 0x30 */ s32 fade;
    /* 0x34 */ s32 period;
} Tlb27KemuriArg;

s32 dochukiDemoKemuriProc(Obj *obj, s16 msg, void *arg);

/* A puff of smoke at x, y, z under the menu's last task. */
void dochukiDemoKemuriCreate(f32 x, f32 y, f32 z, f32 scale, f32 rise, f32 grow, s32 alpha, s32 period,
                            s32 fade) {
    Tlb27KemuriArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = scale;
    arg.grow = grow;
    arg.alpha = alpha;
    arg.period = period;
    arg.fade = fade;
    arg.rise = rise;
    tskCreate(gDmenuLastObj, dochukiDemoKemuriProc, gDmenuLastObj->file, &arg);
}

/* "Meffect"'s (dochukiDemoMeffectProc) argument: only the position its starter sets. */
typedef struct MeffectArg {
    /* 0x00 */ u8 pad00[0x24];
    /* 0x24 */ f32 pos[3];
    /* 0x30 */ u8 pad30[0x14];
} MeffectArg;

/* "Meffect": a pol frame of file 0xEE growing from nothing and rising while it fades. */
typedef struct MeffectW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[0x38];
    /* 0x3C */ s32 alpha;
} MeffectW;

extern Gfx dochukiDemoMeffectDl[]; /* the effect's display list */

s32 dochukiDemoMeffectProc(Obj *obj, s16 msg, void *arg) {
    MeffectW *w = (MeffectW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Meffect";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, dochukiDemoMeffectDl, 0xEE);
            w->frame->flags |= 0x20;
            w->frame->scale[2] = 0.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = ((MeffectArg *) arg)->pos[0];
            w->frame->pos[1] = ((MeffectArg *) arg)->pos[1];
            w->frame->pos[2] = ((MeffectArg *) arg)->pos[2];
            w->frame->rot[0] = -1.5707964f;
            w->alpha = 224;
            break;
        case 0:
            w->alpha -= 3;
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            w->frame->scale[0] += 0.1;
            w->frame->scale[1] += 0.1;
            w->frame->scale[2] += 0.1;
            w->frame->pos[1] += 10.0f;
            break;
        case 3:
            w->frame->dl = demoColorDL(255, 255, 192, 128, 128, 96, w->alpha);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "Meffect" at x, y, z under obj. */
void dochukiDemoMeffectCreate(Obj *obj, f32 x, f32 y, f32 z) {
    MeffectArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    tskCreate(obj, dochukiDemoMeffectProc, obj->file, &arg);
}

/* "Hitball"'s (dochukiDemoHitballProc) argument: only what its starter sets. */
typedef struct HitballArg {
    /* 0x00 */ u8 pad00[0x24];
    /* 0x24 */ f32 pos[3];
    /* 0x30 */ f32 scale;
    /* 0x34 */ u8 pad34[0x10];
} HitballArg;

extern Gfx dochukiDemoBallDl[]; /* the ball's display list */

/* "Hitball": a pol frame of file 0x120 spinning and growing by 1.3 a frame while it fades; gone
 * at alpha 0 or past scale 100. */
s32 dochukiDemoHitballProc(Obj *obj, s16 msg, void *arg) {
    MeffectW *w = (MeffectW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Hitball";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, dochukiDemoBallDl, 0x120);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = ((HitballArg *) arg)->scale;
            w->frame->pos[0] = ((HitballArg *) arg)->pos[0];
            w->frame->pos[1] = ((HitballArg *) arg)->pos[1];
            w->frame->pos[2] = ((HitballArg *) arg)->pos[2];
            w->alpha = 255;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->frame->rot[1] += 0.13962634403104063;
            w->frame->scale[0] *= 1.3;
            w->frame->scale[1] *= 1.3;
            w->frame->scale[2] *= 1.3;
            w->alpha -= 4;
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            if (w->frame->scale[0] > 100.0f) {
                return -1;
            }
            break;
        case 3:
            w->frame->dl = demoColorDL(255, 0, 255, 50, 0, 50, w->alpha);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "Hitball" at x, y, z under obj. */
void dochukiDemoHitballCreate(Obj *obj, f32 x, f32 y, f32 z, f32 v) {
    HitballArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = v;
    tskCreate(obj, dochukiDemoHitballProc, obj->file, &arg);
}

/* "ball"'s work: a ball and its glow, sharing one colour. */
typedef struct BallW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ PolFrame *glow;  /* drawn at 10 times the ball's scale */
    /* 0x08 */ f32 scaleXZ;
    /* 0x0C */ f32 scaleY;
    /* 0x10 */ u8 color[6]; /* prim r, g, b, env r, g, b */
    /* 0x18 */ s32 alpha;
    /* 0x1C */ s32 glowAlpha;
    /* 0x20 */ u8 phase;
} BallW;

extern Gfx dochukiDemoBallGlowDl[]; /* the glow's display list */

/* "ball": a ball 1600 up spinning; at the file's frame 0x22C9 it plays sound 0x18E and brightens
 * until frame 0x232D, then flattens and stretches until it is gone.
 * Without the FAKE block boundary before the colours, 255 takes a0 and 50 v0 (the game has them
 * the other way round); any boundary between the scale chain and the colours does it (splits). */
s32 dochukiDemoBallProc(Obj *obj, s16 msg, void *arg) {
    BallW *w = (BallW *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "ball";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, dochukiDemoBallDl, 0x120);
            w->glow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->glow == NULL) {
                return -1;
            }
            demoPolSetDL(w->glow, dochukiDemoBallGlowDl, 0x120);
            w->glow->flags |= 7;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scaleXZ = w->scaleY = 10.0f;
            w->glow->pos[0] = 0.0f;
            w->frame->pos[0] = w->glow->pos[0];
            w->glow->pos[1] = 1600.0f;
            w->frame->pos[1] = w->glow->pos[1];
            w->glow->pos[2] = 0.0f;
            w->frame->pos[2] = w->glow->pos[2];
            w->alpha = 150;
            w->glowAlpha = 20;
            if (1) {} /* FAKE: a block boundary before the colours */
            w->color[0] = 255;
            w->color[1] = 0;
            w->color[2] = 255;
            w->color[3] = 50;
            w->color[4] = 0;
            w->color[5] = 50;
            w->phase = 0;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->glow->f);
            break;
        case 0:
            w->frame->rot[1] += 0.13962634403104063;
            w->frame->scale[0] = w->frame->scale[2] = w->scaleXZ;
            w->frame->scale[1] = w->scaleY;
            w->glow->scale[0] = w->glow->scale[2] = w->scaleXZ * 10.0f;
            w->glow->scale[1] = w->scaleY * 10.0f;
            switch (w->phase) {
                case 0:
                    if (dochukiDemoFrame == 0x22C9) {
                        sndPlay(0x18E);
                        w->phase++;
                    }
                    break;
                case 1:
                    if (w->color[1] < 255) {
                        w->color[1] += 2;
                    }
                    if (w->color[4] < 50) {
                        w->color[4] += 1;
                    }
                    w->alpha++;
                    w->glowAlpha++;
                    if (w->alpha > 255) {
                        w->alpha = 255;
                    }
                    if (w->glowAlpha > 80) {
                        w->glowAlpha = 80;
                    }
                    if (dochukiDemoFrame == 0x232D) {
                        w->phase++;
                    }
                    break;
                case 2:
                    w->scaleXZ *= 0.97;
                    w->scaleY *= 1.12;
                    if (w->scaleY > 100.0f) {
                        w->scaleY = 100.0f;
                    }
                    if (w->scaleXZ < 0.1) {
                        w->scaleXZ = 0.1f;
                        return -1;
                    }
                    break;
            }
            break;
        case 3:
            w->frame->dl = demoColorDL(w->color[0], w->color[1], w->color[2], w->color[3], w->color[4],
                                       w->color[5], w->alpha);
            w->glow->dl = demoColorDL(w->color[0], w->color[1], w->color[2], w->color[3], w->color[4],
                                      w->color[5], w->glowAlpha);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 dochukiDemoBallProc(Obj *obj, s16 msg, void *arg);

Obj *dochukiDemoBallCreate(Obj *obj) {
    return tskCreate(obj, dochukiDemoBallProc, obj->file, NULL);
}

/* "Star" (its proc dochukiDemoStarProc): its frame and what its starter sets. */
typedef struct StarW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ s32 alpha;
    /* 0x08 */ u8 phase;
    /* 0x0C */ s32 timer;
} StarW;

extern Gfx dochukiDemoStarDl[]; /* the star's display list */

/* "Star": spins while it stretches (phase 0, 10 frames), then grows (phase 1, to frame 20),
 * then grows and fades (phase 2); gone at alpha 0 or at the file's frame 0x1614. */
s32 dochukiDemoStarProc(Obj *obj, s16 msg, void *arg) {
    StarW *w = (StarW *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "Star";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                tskDestroy(obj);
                return -1;
            }
            demoPolSetDL(w->frame, dochukiDemoStarDl, 0xEE);
            w->timer = 0;
            break;
        case 0:
            w->timer++;
            switch (w->phase) {
                case 0:
                    w->frame->rot[1] += 0.13962634403104063;
                    w->frame->scale[0] += 0.4;
                    w->frame->scale[1] -= 0.4;
                    w->frame->scale[2] += 0.4;
                    if (w->timer == 10) {
                        w->phase++;
                    }
                    break;
                case 1:
                    w->frame->rot[1] += 0.13962634403104063;
                    w->frame->scale[0] += 0.2;
                    w->frame->scale[1] += 0.8;
                    w->frame->scale[2] += 0.2;
                    if (w->timer == 20) {
                        w->phase++;
                    }
                    break;
                case 2:
                    w->frame->rot[1] += 0.13962634403104063;
                    w->frame->scale[0] += 0.2;
                    w->frame->scale[1] += 0.8;
                    w->frame->scale[2] += 0.2;
                    w->alpha -= 4;
                    if (w->alpha <= 0) {
                        w->alpha = 0;
                        return -1;
                    }
                    break;
            }
            if (dochukiDemoFrame >= 0x1614) {
                return -1;
            }
            break;
        case 3:
            w->frame->dl = demoColorDL(255, 0, 200, 100, 0, 40, w->alpha);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "Star" at x, y, z under obj, scaled s across and 70 high. */
void dochukiDemoStarCreate(Obj *obj, f32 x, f32 y, f32 z, s32 n, f32 s) {
    Obj *o = tskCreate(obj, dochukiDemoStarProc, obj->file, NULL);

    if (o != NULL) {
        ((StarW *) FX(o))->frame->flags |= 0x20;
        ((StarW *) FX(o))->frame->scale[0] = s;
        ((StarW *) FX(o))->frame->scale[1] = 70.0f;
        ((StarW *) FX(o))->frame->scale[2] = s;
        ((StarW *) FX(o))->frame->pos[0] = x;
        ((StarW *) FX(o))->frame->pos[1] = y;
        ((StarW *) FX(o))->frame->pos[2] = z;
        ((StarW *) FX(o))->alpha = n;
        ((StarW *) FX(o))->phase = 0;
    }
}

/* The scene's effects by the file's frame counter: before frame 0xA0E and after 0x127C, three
 * puffs of smoke every 15 frames at each of four places in turn; then rising smoke at the
 * centre, every 20 frames to 0xCCA and every 30 to 0x10B0; then a growing star every 20 frames
 * to 0x1614; and the ball at frame 0x127C. */
void dochukiDemoEffectsStep(void) {
    if (dochukiDemoFrame < 0xA0E || dochukiDemoFrame >= 0x127D) {
        if (dochukiDemoFrame % 60 == 0) {
            (*dochukiDemoKemuriCreate)(-648.0f, -335.0f, 874.0f, 8.0f, 1.1f, 10.0f, 200, 10, 4);
            (*dochukiDemoKemuriCreate)(-924.0f, -335.0f, 594.0f, 8.0f, 1.1f, 10.0f, 200, 10, 4);
            (*dochukiDemoKemuriCreate)(-924.0f, -335.0f, 218.0f, 8.0f, 1.1f, 10.0f, 200, 10, 4);
        }
        if (dochukiDemoFrame % 60 == 15) {
            (*dochukiDemoKemuriCreate)(648.0f, -335.0f, 874.0f, 8.0f, 1.1f, 10.0f, 200, 10, 4);
            (*dochukiDemoKemuriCreate)(924.0f, -335.0f, 594.0f, 8.0f, 1.1f, 10.0f, 200, 10, 4);
            (*dochukiDemoKemuriCreate)(924.0f, -335.0f, 218.0f, 8.0f, 1.1f, 10.0f, 200, 10, 4);
        }
        if (dochukiDemoFrame % 60 == 30) {
            (*dochukiDemoKemuriCreate)(-924.0f, -335.0f, 297.0f, 8.0f, 1.1f, 10.0f, 200, 10, 4);
            (*dochukiDemoKemuriCreate)(-56.0f, -335.0f, 1061.0f, 8.0f, 1.1f, 10.0f, 200, 10, 4);
            (*dochukiDemoKemuriCreate)(-524.0f, -335.0f, 1097.0f, 8.0f, 1.1f, 10.0f, 200, 10, 4);
        }
        if (dochukiDemoFrame % 60 == 45) {
            (*dochukiDemoKemuriCreate)(924.0f, -335.0f, 297.0f, 8.0f, 1.1f, 10.0f, 200, 10, 4);
            (*dochukiDemoKemuriCreate)(56.0f, -335.0f, 1061.0f, 8.0f, 1.1f, 10.0f, 200, 10, 4);
            (*dochukiDemoKemuriCreate)(524.0f, -335.0f, 1097.0f, 8.0f, 1.1f, 10.0f, 200, 10, 4);
        }
    }
    if (dochukiDemoFrame > 0xA0E && dochukiDemoFrame < 0xCCA && dochukiDemoFrame % 20 == 0) {
        (*dochukiDemoKemuriCreate)(0.0f, 335.0f, 0.0f, 8.0f, 1.1f, 10.0f, 100, 3, 4);
        (*dochukiDemoKemuriCreate)(0.0f, 335.0f, 0.0f, 8.0f, 1.1f, 20.0f, 100, 3, 4);
    }
    if (dochukiDemoFrame > 0xCCA && dochukiDemoFrame < 0x10B0 && dochukiDemoFrame % 30 == 0) {
        (*dochukiDemoKemuriCreate)(0.0f, 1200.0f, 0.0f, 8.0f, 1.1f, 10.0f, 60, 3, 4);
    }
    if (dochukiDemoFrame > 0x10B0 && dochukiDemoFrame < 0x1614 && dochukiDemoFrame % 20 == 0) {
        s32 n = (dochukiDemoFrame - 0xF20) / 10;

        if (n > 40) {
            n = 40;
        }
        (*dochukiDemoStarCreate)(gDmenuLastObj, 0.0f, 1000.0f, 0.0f, n, 3.5f);
    }
    if (dochukiDemoFrame == 0x127C) {
        (*dochukiDemoBallCreate)(gDmenuLastObj);
    }
}
