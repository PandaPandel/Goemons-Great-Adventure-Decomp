#include "tlb.h"
#include "tlb/tlb_0032.h"

/* The argument-only print: its string stays, no code. */
#define DPRINTF

/* Main's sinf, called through its address as TLB code calls main. */
#define sinf (*sinf)

extern u8 gDemoCamHold;
extern s32 gDemoModelsDelete;
extern s32 gDemoModelsDelete2;
extern s32 gObjMoveSel; /* OBJMOVE: the model chosen, -1 for none */
extern f32 gObjMovePosX; /* the chosen model's position and angle, as shown */
extern f32 gObjMovePosY;
extern f32 gObjMovePosZ;
extern f32 gObjMoveRot;
extern f32 gObjMoveDx; /* the moves this frame: x, y, z, angle */
extern f32 gObjMoveDy;
extern f32 gObjMoveDz;
extern f32 gObjMoveDrot;
extern s32 clr5DemoFrame; /* the file's frame counter */
extern Gfx clr5DemoShadowDl[];         /* a player's shadow's display list */

/* The demo players' work ("genoGOE", "chaSAS", "chaYAE"): the player task, its model, its
 * shadow and the face's two texture animations (as tlb_0027_1's). */
typedef struct DemoPlW {
    /* 0x00 */ Obj *pl;
    /* 0x04 */ AnimeFrame *model;
    /* 0x08 */ PolFrame *shadow;
    /* 0x0C */ Anim eyes;
    /* 0x1C */ Anim mouth;
    /* 0x2C */ s32 chara;
    /* 0x30 */ s32 timer;
    /* 0x34 */ s32 state;
} DemoPlW;

/* The segment's .bss after tlb_0032_0's panel (0x080094C0), in the game's order (IDO_NOTES 343):
 * the file's words, then genoGOE's statics (in his proc below), then his player. */
s32 clr5DemoUnused4[12];
f32 clr5DemoSasukeX;
f32 clr5DemoSasukeZ;
f32 clr5DemoGoeX; /* Goemon's x and z, each frame: the point the others turn toward */
f32 clr5DemoGoeZ;
f32 clr5DemoEbiX;
f32 clr5DemoEbiZ;
f32 clr5DemoYaeX;
f32 clr5DemoYaeZ;
f32 clr5DemoSuzakuX;
f32 clr5DemoSuzakuZ;
f32 clr5DemoDoucyuX;
f32 clr5DemoDoucyuZ;
f32 clr5DemoFireX;
f32 clr5DemoFireY;
f32 clr5DemoFireZ;
f32 clr5DemoFireScale;
s32 clr5DemoUnused5; /* two words nothing reads */
s32 clr5DemoUnused6;
extern Player *clr5DemoGoePlayer; /* his player */

/* "genoGOE" (Goemon, character 0, in the demo) at (-9, 0, 247) under the menu's last task;
 * OBJMOVE's model 0 moves him. His steps follow the file's frame counter (scenes 0x164, 0x165,
 * 0x156, 0x15C, 0x15D and 0x163). */
s32 clr5DemoGoeProc(Obj *obj, s16 msg, void *arg) {
    static s32 step; /* .bss 0x08009538: his demoPlTunagiAnime step */
    static s32 unused; /* 0x0800953C: a word nothing reads */
    s32 pad;
    DemoPlW *w = (DemoPlW *) (obj + 1);
    Obj *pl;
    PlInit init;

    switch (msg) {
        case 2:
            DPRINTF("goecreate\n");
            obj->name = "genoGOE";
            plMakeInit(&init, 0, 0);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = -9.0f;
            init.height = 0.0f;
            init.side = 247.0f;
            pl = plCreate(gDmenuLastObj, &init);
            w->pl = pl;
            clr5DemoGoePlayer = PL(pl);
            plEnterDemo(pl);
            PRINTF("デモモード突入\n");
            PRINTF("歩きモーション\n");
            plAnimeSet(PL(w->pl), 0xE9);
            w->model = clr5DemoGoePlayer->model;
            PRINTF("*** p->model_buff=%x\n", clr5DemoGoePlayer->model);
            PRINTF("ゴエモン作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, clr5DemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.08f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->chara = 0;
            w->model->rot[1] = 3.141592741f;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -9.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = 247.0f;
            w->model->rot[1] = 0.1570796371f;
            PRINTF("ゴエモン作りました\n");
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
            w->shadow->pos[1] = w->model->pos[1] + 0.3;
            clr5DemoGoeX = w->model->pos[0];
            clr5DemoGoeZ = w->model->pos[2];
            switch (w->state) {
                case 0:
                    w->model->scale[2] = 0.097f;
                    w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                    w->model->pos[0] = -9.0f;
                    w->model->pos[1] = 0.0f;
                    w->model->pos[2] = 247.0f;
                    w->model->rot[1] = 0.1570796371f;
                    w->state++;
                    step = 0;
                    demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                    demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                    break;
                case 1:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (clr5DemoFrame >= 0x113) {
                        step = demoPlTunagiAnime(obj, 3, step, 2, 2, 0.6, 0.6, 0xEB, 0xEC);
                        if (step == -1) {
                            w->state++;
                            scRunEnd();
                            scRunStart(0x164, 0, 0);
                            step = 0;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.6, 0);
                    }
                    break;
                case 2:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (clr5DemoFrame >= 0x1B4) {
                        step = demoPlTunagiAnime(obj, 1, step, 2, 3, 0.6, 0.6, 0xEB, 0xE9);
                        if (step == -1) {
                            scRunEnd();
                            w->state++;
                            step = 0;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.6, 0);
                    }
                    break;
                case 3:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (clr5DemoFrame >= 0x300) {
                        plAnimeSet(PL(w->pl), 0x6E);
                        demoTexAnimK0(&w->eyes, 7, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 7, &PL(w->pl)->model->segs[4]);
                        w->state++;
                        scRunEnd();
                        scRunStart(0x165, 0, 0);
                    } else {
                        demoAnimeStep(w->model, 0.6, 0);
                    }
                    break;
                case 4:
                    demoAnimeStep(w->model, 1.0f, 2);
                    if (clr5DemoFrame == 0x35F) {
                        scRunEnd();
                    }
                    if (clr5DemoFrame >= 0x3C3) {
                        plAnimeSet(PL(w->pl), 0x10B);
                        demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 5:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (clr5DemoFrame >= 0x846) {
                        step = 0;
                        w->state++;
                    }
                    break;
                case 6:
                    step = demoPlTunagiAnime(obj, 3, step, 2, 2, 0.4, 1.0f, 0x10C, 0x10D);
                    if (step == -1) {
                        w->state++;
                        step = 0;
                        scRunEnd();
                        scRunStart(0x156, 0, 0);
                    }
                    break;
                case 7:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 1.0f, 0);
                    if (clr5DemoFrame >= 0x9B6) {
                        w->state++;
                        step = 0;
                        scRunEnd();
                    }
                    break;
                case 8:
                    step = demoPlTunagiAnime(obj, 1, step, 2, 3, 1.0f, 0.4, 0x10B, 0x10B);
                    if (step == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 9:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (clr5DemoFrame >= 0xC4A) {
                        plAnimeSet(PL(w->pl), 0x2B);
                        demoTexAnimK0(&w->eyes, 8, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 8, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 10:
                    demoAnimeStep(w->model, 0.4f, 2);
                    if (clr5DemoFrame >= 0xCEA) {
                        w->model->pos[0] = 72.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 664.0f;
                        w->model->rot[1] = 1.832595706f;
                        plAnimeSet(PL(w->pl), 0xFD);
                        demoTexAnimK0(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 11:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    w->model->pos[1] = 1.0f;
                    if (clr5DemoFrame >= 0x1262) {
                        demoTexAnimK0(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                        scRunEnd();
                        scRunStart(0x15C, 0, 0);
                        w->state++;
                    }
                    break;
                case 12:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (demoAnimeStep(w->model, 0.2f, 2)) {
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK0(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 13:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (clr5DemoFrame >= 0x13F2) {
                        scRunEnd();
                        scRunStart(0x15D, 0, 0);
                        w->state++;
                    }
                    break;
                case 14:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (clr5DemoFrame >= 0x1582) {
                        demoTexAnimK0(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 15:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (clr5DemoFrame >= 0x1DB6) {
                        step = 0;
                        w->state++;
                    }
                    break;
                case 16:
                    step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2, 0.5f, 0xEB, 0xEC);
                    if (step == -1) {
                        w->state++;
                        step = 0;
                        scRunEnd();
                        scRunStart(0x163, 0, 0);
                    }
                    break;
                case 17:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.5f, 0);
                    if (clr5DemoFrame >= 0x1F46) {
                        w->state++;
                        scRunEnd();
                    }
                    break;
                case 18:
                    step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.5f, 0.2, 0xEB, 0xE9);
                    if (step == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 19:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 clr5DemoGoeProc(Obj *obj, s16 msg, void *arg);

Player *clr5DemoGoePlayer; /* 0x08009540: his player, the file's last .bss word */

Obj *clr5DemoGoeCreate(Obj *obj) {
    return tskCreate(obj, clr5DemoGoeProc, obj->file, NULL);
}

extern f32 clr5DemoEbiX; /* Ebisumaru's x and z, each frame */
extern f32 clr5DemoEbiZ;

void clr5DemoOnaraCreate(Obj *obj, f32 x, f32 y, f32 z, s32 n, f32 s);

/* "genoGOE" (Ebisumaru, character 1, in the demo) at (-22, 0, 244) under the menu's last task;
 * OBJMOVE's model 1 moves him. His steps follow the file's frame counter (scenes 0x157 and
 * 0x159); at 0xA3F he lets out three "ONARA" clouds. */
/* 192 differing lines, mix distance 0, frame and slots the game's: every demoTexAnimK1 call
 * pair keeps the function's address (spilled at 0x38) in v0 where the game has v1 (all ten
 * pairs; fadeAnimStep's and frmDestroy's pairs are v1 in both), plus the a0/a1 setup that
 * moves with it. As tlb_0027_1's genoGOE with demoTexAnimK0 (its note); chaSAS/chaYAE here
 * (K2/K3) matched with v1. Measured: "if (w->timer-- == 0) {..}" 293 (an sltiu into v1),
 * "if (w->timer--) break;" 192; state 6's "w->state++; w->timer = 30;" on one line (the
 * game stores the timer first) fixed t3/t5. A scratch compile with K1's calls renamed (another
 * symbol, or demoTexAnimK2 itself) still gave v0: not the symbol, the function's shape.
 * 2026-10-11 (real 60): demoTexAnimK1 called as int-returning: MATCH.
 * 2026-10-17 (a kept FAKE): since scRunEnd returns its Obj *
 * (commit 53ddf08), the dropped results of its four calls do that, and demoTexAnimK1 is called
 * as the header declares it. */
s32 clr5DemoEbiProc(Obj *obj, s16 msg, void *arg) {
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
            init.place = -22.0f;
            init.height = 0.0f;
            init.side = 244.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            PRINTF("デモモード突入\n");
            plAnimeSet(PL(w->pl), 0xE9);
            w->model = p->model;
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, clr5DemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.09f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 1;
            w->state = 0;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -22.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = 244.0f;
            w->model->rot[1] = 0.34906587f;
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
            w->shadow->pos[1] = w->model->pos[1] + 0.3;
            clr5DemoEbiX = w->model->pos[0];
            clr5DemoEbiZ = w->model->pos[2];
            switch (w->state) {
                case 0:
                    w->model->pos[0] = -22.0f;
                    w->model->pos[1] = 0.0f;
                    w->model->pos[2] = 244.0f;
                    w->model->rot[1] = 0.34906587f;
                    demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                    demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    w->state++;
                    break;
                case 1:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (clr5DemoFrame >= 0x300) {
                        plAnimeSet(PL(w->pl), 0x6E);
                        demoTexAnimK1(&w->eyes, 5, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 5, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    } else {
                        demoAnimeStep(w->model, 0.8f, 0);
                    }
                    break;
                case 2:
                    demoAnimeStep(w->model, 1.0f, 2);
                    if (clr5DemoFrame >= 0x3C3) {
                        plAnimeSet(PL(w->pl), 0x10B);
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 3:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (clr5DemoFrame >= 0x9B6) {
                        w->model->pos[0] = 25.0f;
                        w->model->pos[1] = 0;
                        w->model->pos[2] = 370.0f;
                        w->model->rot[1] = 0.0f;
                        w->state++;
                    }
                    break;
                case 4:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (clr5DemoFrame >= 0x9C7) {
                        if (demoAnimeStep(w->model, 0.4f, 2)) {
                            plAnimeSet(PL(w->pl), 0x107);
                            demoTexAnimK1(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK1(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                            scRunEnd();
                            scRunStart(0x157, 0, 0);
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.4f, 0);
                    }
                    break;
                case 5:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (demoAnimeStep(w->model, 0.8f, 2)) {
                        plAnimeSet(PL(w->pl), 0x10B);
                        demoTexAnimK1(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    } else {
                        w->model->pos[2] += 0.4;
                    }
                    break;
                case 6:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (clr5DemoFrame >= 0xA3F) {
                        w->state++; w->timer = 30;
                        scRunEnd();
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 7:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (w->timer--) {
                        break;
                    }
                    plAnimeSet(PL(w->pl), 0x83);
                    demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                    demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    w->timer = 0;
                    w->state++;
                    break;
                case 8:
                    demoAnimeStep(w->model, 0.4f, 2);
                    switch (w->timer++) {
                        case 2:
                            sndPlay(0x193);
                            clr5DemoOnaraCreate(obj, w->model->pos[0] - 1.0f, w->model->pos[1] + 5.0f,
                                                   w->model->pos[2] + 9.0f, 180, 0.07f);
                            clr5DemoOnaraCreate(obj, w->model->pos[0] + 2.0f, w->model->pos[1] + 7.0f,
                                                   w->model->pos[2] + 5.0f, 180, 0.08f);
                            break;
                        case 4:
                            clr5DemoOnaraCreate(obj, w->model->pos[0] - 3.0f, w->model->pos[1] + 10.0f,
                                                   w->model->pos[2] + 7.0f, 180, 0.07f);
                            break;
                        case 9:
                            clr5DemoOnaraCreate(obj, w->model->pos[0] + 1.0f, w->model->pos[1] + 8.0f,
                                                   w->model->pos[2] + 7.0f, 180, 0.06f);
                            w->state++;
                            break;
                    }
                    break;
                case 9:
                    if (clr5DemoFrame >= 0xA99) {
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK1(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                        w->state++;
                        w->model->pos[0] = 25.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 395.0f;
                        w->model->rot[1] = -0.34906587f;
                    }
                    break;
                case 10:
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (clr5DemoFrame >= 0xB7A) {
                        scRunEnd();
                        scRunStart(0x159, 0, 0);
                        w->state++;
                    }
                    break;
                case 11:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (clr5DemoFrame >= 0xBAC) {
                        if (demoAnimeStep(w->model, 0.4f, 2)) {
                            plAnimeSet(PL(w->pl), 0x108);
                            demoTexAnimK1(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK1(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.4f, 0);
                    }
                    break;
                case 12:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 2);
                    if (clr5DemoFrame >= 0xC4A) {
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 13:
                    w->model->pos[1] = 1.0f;
                    if (clr5DemoFrame >= 0xCEA) {
                        w->model->pos[0] = 64.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 675.0f;
                        w->model->rot[1] = 1.954768777f;
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 14:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (clr5DemoFrame >= 0x1DB7) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], clr5DemoGoeX, clr5DemoGoeZ,
                                                          w->model->rot[1], 0.5f);
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 clr5DemoEbiProc(Obj *obj, s16 msg, void *arg);

Obj *clr5DemoEbiCreate(Obj *obj) {
    return tskCreate(obj, clr5DemoEbiProc, obj->file, NULL);
}

extern f32 clr5DemoSasukeX; /* Sasuke's x and z, each frame */
extern f32 clr5DemoSasukeZ;

/* "chaSAS" (Sasuke, character 2, in the demo) at (-2, 0, 251) under the menu's last task;
 * OBJMOVE's model 2 moves him. His steps follow the file's frame counter (scene 0x166 at frame
 * 0xC4A). */
s32 clr5DemoSasProc(Obj *obj, s16 msg, void *arg) {
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
            init.place = -2.0f;
            init.height = 0.0f;
            init.side = 251.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            PRINTF("デモモード突入\n");
            PRINTF("走りモーション\n");
            plAnimeSet(PL(w->pl), 0xE9);
            w->model = p->model;
            PRINTF("サスケ作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, clr5DemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.065f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 2;
            w->state = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -2.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = 251.0f;
            w->model->rot[1] = 0.01745329238f;
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
            clr5DemoSasukeX = w->model->pos[0];
            clr5DemoSasukeZ = w->model->pos[2];
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = w->model->pos[1] + 0.3;
            switch (w->state) {
                case 0:
                    w->model->pos[0] = -2.0f;
                    w->model->pos[1] = 0.0f;
                    w->model->pos[2] = 251.0f;
                    w->model->rot[1] = 0.01745329238f;
                    w->state++;
                    break;
                case 1:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (clr5DemoFrame >= 0x300) {
                        plAnimeSet(PL(w->pl), 0x6E);
                        demoTexAnimK2(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    } else {
                        demoAnimeStep(w->model, 0.6f, 0);
                    }
                    break;
                case 2:
                    demoAnimeStep(w->model, 1.0f, 2);
                    if (clr5DemoFrame >= 0x3C3) {
                        plAnimeSet(PL(w->pl), 0x10B);
                        demoTexAnimK2(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 3:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (clr5DemoFrame >= 0xC4A) {
                        plAnimeSet(PL(w->pl), 0x2B);
                        demoTexAnimK2(&w->eyes, 5, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 5, &PL(w->pl)->model->segs[4]);
                        w->state++;
                        scRunEnd();
                        scRunStart(0x166, 0, 0);
                    }
                    break;
                case 4:
                    demoAnimeStep(w->model, 0.4f, 2);
                    if (clr5DemoFrame == 0xCEA) {
                        scRunEnd();
                    }
                    if (clr5DemoFrame >= 0xCEA) {
                        w->model->pos[0] = 76.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 656.0f;
                        w->model->rot[1] = 1.588249683f;
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK2(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 5:
                    w->model->pos[1] = 1.0f;
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (clr5DemoFrame >= 0x1DB7) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], clr5DemoGoeX, clr5DemoGoeZ,
                                                          w->model->rot[1], 0.7f);
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 clr5DemoSasProc(Obj *obj, s16 msg, void *arg);

Obj *clr5DemoSasCreate(Obj *obj) {
    return tskCreate(obj, clr5DemoSasProc, obj->file, NULL);
}

extern f32 clr5DemoYaeX; /* Yae's x and z, each frame */
extern f32 clr5DemoYaeZ;

/* "chaYAE" (Yae, character 3, in the demo) at (5, 0, 245) under the menu's last task; OBJMOVE's
 * model 3 moves her. Her steps follow the file's frame counter (scene 0x161 at frame 0x196A);
 * from frame 0x1DB7 she turns toward Sasuke's point. */
s32 clr5DemoYaeProc(Obj *obj, s16 msg, void *arg) {
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
            init.place = 5.0f;
            init.height = 0.0f;
            init.side = 245.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            PRINTF("デモモード突入\n");
            PRINTF("立ちモーション\n");
            plAnimeSet(PL(w->pl), 0xD7);
            w->model = p->model;
            PRINTF("ヤエ作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, clr5DemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.065f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 3;
            w->state = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 5.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = 245.0f;
            w->model->rot[1] = -0.01745329238f;
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
            clr5DemoYaeX = w->model->pos[0];
            clr5DemoYaeZ = w->model->pos[2];
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = w->model->pos[1] + 0.3;
            switch (w->state) {
                case 0:
                    w->state++;
                    w->model->pos[0] = 5.0f;
                    w->model->pos[1] = 0.0f;
                    w->model->pos[2] = 245.0f;
                    w->model->rot[1] = -0.01745329238f;
                    break;
                case 1:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (clr5DemoFrame >= 0x300) {
                        plAnimeSet(PL(w->pl), 0x6E);
                        demoTexAnimK3(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    } else {
                        demoAnimeStep(w->model, 0.6f, 0);
                    }
                    break;
                case 2:
                    demoAnimeStep(w->model, 1.0f, 2);
                    if (clr5DemoFrame >= 0x3C3) {
                        plAnimeSet(PL(w->pl), 0x10B);
                        demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 3:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (clr5DemoFrame >= 0xC4A) {
                        plAnimeSet(PL(w->pl), 0x2B);
                        demoTexAnimK3(&w->eyes, 9, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 9, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 4:
                    demoAnimeStep(w->model, 0.4f, 2);
                    if (clr5DemoFrame >= 0xCEA) {
                        w->model->pos[0] = 68.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 647.0f;
                        w->model->rot[1] = 1.605702996f;
                        plAnimeSet(PL(w->pl), 0xD7);
                        demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 5:
                    w->model->pos[1] = 1.0f;
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (clr5DemoFrame >= 0x196A) {
                        if (demoAnimeStep(w->model, 0.4f, 2)) {
                            plAnimeSet(PL(w->pl), 0x101);
                            demoTexAnimK3(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK3(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                            scRunEnd();
                            scRunStart(0x161, 0, 0);
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.3f, 0);
                    }
                    break;
                case 6:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.5f, 0);
                    if (clr5DemoFrame >= 0x1B5E && demoAnimeStep(w->model, 0.4f, 2)) {
                        plAnimeSet(PL(w->pl), 0xD7);
                        demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 7:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (clr5DemoFrame >= 0x1DB7) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], clr5DemoGoeX, clr5DemoGoeZ,
                                                          w->model->rot[1], 0.5f);
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 clr5DemoYaeProc(Obj *obj, s16 msg, void *arg);

Obj *clr5DemoYaeCreate(Obj *obj) {
    return tskCreate(obj, clr5DemoYaeProc, obj->file, NULL);
}

/* The demo's anime models' work ("suzaku", "doucyu"): the model, its shadow, its texture
 * animation and its stage. */
typedef struct DouW {
    /* 0x00 */ AnimeFrame *model;
    /* 0x04 */ PolFrame *shadow;
    /* 0x08 */ Anim anim;
    /* 0x18 */ u8 pad18[4];
    /* 0x1C */ s32 state;
} DouW;

extern Gfx clr5DemoSuzakuModelAnim[]; /* the old man's animations (file 0xCD) */
extern Gfx clr5DemoSuzakuState1Anim[];
extern Gfx clr5DemoSuzakuState2Anim[];
extern Gfx clr5DemoSuzakuState4Anim[];
extern Gfx clr5DemoSuzakuState5Anim[];
extern Gfx clr5DemoSuzakuAnim2[];
extern Gfx clr5DemoSuzakuState16Anim[];
extern Gfx clr5DemoSuzakuState17Anim[];
extern AnimFrame clr5DemoSuzakuAnim[]; /* his texture animations */
extern AnimFrame clr5DemoSuzakuAnimB[];
extern s32 clr5DemoSuzakuUnusedFlag;
extern f32 clr5DemoSuzakuX; /* his x and z, each frame */
extern f32 clr5DemoSuzakuZ;

/* "suzaku" (the wise old man, "物知りじいさん", file 0xCD) with his shadow at (15, 0, 245);
 * OBJMOVE's model 4 moves him. He steps through his animations by the file's frame counter
 * (scenes 0x152, 0x15A-0x162), turning toward the others, and goes up in a puff at the end. */
s32 clr5DemoSuzakuProc(Obj *obj, s16 msg, void *arg) {
    DouW *w = (DouW *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "suzaku";
            PRINTF("物知りじいさん作ります\n");
            w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->model == NULL) {
                return -1;
            }
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoAnimeSet(w->model, (u32) clr5DemoSuzakuModelAnim, 0xCD, 0.0f);
            fadeAnimStart(&w->anim, clr5DemoSuzakuAnim, 0, &w->model->segs[3], floadGetSetFile(0xCD, 0));
            demoPolSetDL(w->shadow, clr5DemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.08f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            clr5DemoSuzakuUnusedFlag = 0;
            w->state = 0;
            w->model->scale[2] = 0.097f;
            PRINTF("物知りじいさん作りました\n");
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 15.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = 245.0f;
            w->model->rot[1] = -0.174532935f;
            break;
        case 1:
            frmDestroy(&w->model->f);
            frmDestroy(&w->shadow->f);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, 128);
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
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = w->model->pos[1] + 0.3;
            switch (w->state) {
                case 0:
                    w->model->pos[0] = 15.0f;
                    w->model->pos[1] = 0.0f;
                    w->model->pos[2] = 245.0f;
                    w->model->rot[1] = -0.174532935f;
                    w->state++;
                    break;
                case 1:
                    if (clr5DemoFrame >= 0x300) {
                        demoAnimeSet(w->model, (u32) clr5DemoSuzakuState1Anim, 0xCD, 0.0f);
                        fadeAnimStart(&w->anim, clr5DemoSuzakuAnimB, 3, &w->model->segs[3],
                                      floadGetSetFile(0xCD, 0));
                        w->state++;
                    } else {
                        demoAnimeStep(w->model, 0.6f, 0);
                    }
                    break;
                case 2:
                    demoAnimeStep(w->model, 1.0f, 2);
                    if (clr5DemoFrame >= 0x3C3) {
                        demoAnimeSet(w->model, (u32) clr5DemoSuzakuState2Anim, 0xCD, 0.0f);
                        fadeAnimStart(&w->anim, clr5DemoSuzakuAnim, 1, &w->model->segs[3],
                                      floadGetSetFile(0xCD, 0));
                        w->state++;
                    }
                    break;
                case 3:
                    if (clr5DemoFrame >= 0x526) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0x152, 0, 0);
                    } else {
                        demoAnimeStep(w->model, 0.4f, 0);
                    }
                    break;
                case 4:
                    fadeAnimStep(&w->anim);
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (clr5DemoFrame >= 0x607) {
                        demoAnimeSet(w->model, (u32) clr5DemoSuzakuState4Anim, 0xCD, 0.0f);
                        fadeAnimStart(&w->anim, clr5DemoSuzakuAnim, 0, &w->model->segs[3],
                                      floadGetSetFile(0xCD, 0));
                        w->state++;
                        scRunEnd();
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (clr5DemoFrame >= 0xC4A) {
                        demoAnimeSet(w->model, (u32) clr5DemoSuzakuState5Anim, 0xCD, 0.0f);
                        fadeAnimStart(&w->anim, clr5DemoSuzakuAnimB, 2, &w->model->segs[3],
                                      floadGetSetFile(0xCD, 0));
                        w->state++;
                    }
                    break;
                case 6:
                    demoAnimeStep(w->model, 0.4f, 2);
                    if (clr5DemoFrame >= 0xCEA) {
                        w->model->pos[0] = 98.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 659.0f;
                        w->model->rot[1] = -1.291543722f;
                        demoAnimeSet(w->model, (u32) clr5DemoSuzakuModelAnim, 0xCD, 0.0f);
                        fadeAnimStart(&w->anim, clr5DemoSuzakuAnim, 0, &w->model->segs[3],
                                      floadGetSetFile(0xCD, 0));
                        w->model->pos[1] = 1.0f;
                        w->state++;
                    }
                    break;
                case 7:
                    if (clr5DemoFrame >= 0xDE4) {
                        if (demoAnimeStep(w->model, 0.2f, 2)) {
                            demoAnimeSet(w->model, (u32) clr5DemoSuzakuAnim2, 0xCD, 0.0f);
                            fadeAnimStart(&w->anim, clr5DemoSuzakuAnim, 1, &w->model->segs[3],
                                          floadGetSetFile(0xCD, 0));
                            w->state++;
                            scRunEnd();
                            scRunStart(0x15A, 0, 0);
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
                case 8:
                    fadeAnimStep(&w->anim);
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (clr5DemoFrame < 0xEAC) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], clr5DemoSasukeX, clr5DemoSasukeZ,
                                                          w->model->rot[1], 0.3f);
                    } else {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], clr5DemoGoeX, clr5DemoGoeZ,
                                                          w->model->rot[1], 0.2f);
                    }
                    if (clr5DemoFrame >= 0xFD8) {
                        scRunEnd();
                        scRunStart(0x15B, 0, 0);
                        w->state++;
                    }
                    break;
                case 9:
                    fadeAnimStep(&w->anim);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (clr5DemoFrame >= 0x1230) {
                        scRunEnd();
                        demoAnimeSet(w->model, (u32) clr5DemoSuzakuModelAnim, 0xCD, 0.0f);
                        fadeAnimStart(&w->anim, clr5DemoSuzakuAnim, 0, &w->model->segs[3],
                                      floadGetSetFile(0xCD, 0));
                        w->state++;
                    }
                    break;
                case 10:
                    if (clr5DemoFrame >= 0x15E6) {
                        if (demoAnimeStep(w->model, 0.3f, 2)) {
                            demoAnimeSet(w->model, (u32) clr5DemoSuzakuAnim2, 0xCD, 0.0f);
                            fadeAnimStart(&w->anim, clr5DemoSuzakuAnim, 1, &w->model->segs[3],
                                          floadGetSetFile(0xCD, 0));
                            w->state++;
                            scRunEnd();
                            scRunStart(0x15E, 0, 0);
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
                case 11:
                    fadeAnimStep(&w->anim);
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (clr5DemoFrame >= 0x1776) {
                        scRunEnd();
                        scRunStart(0x15F, 0, 0);
                        w->state++;
                    }
                    break;
                case 12:
                    fadeAnimStep(&w->anim);
                    demoAnimeStep(w->model, 0.15f, 0);
                    if (clr5DemoFrame >= 0x183E) {
                        scRunEnd();
                        scRunStart(0x160, 0, 0);
                        w->state++;
                    }
                    break;
                case 13:
                    fadeAnimStep(&w->anim);
                    if (clr5DemoFrame >= 0x199D) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], clr5DemoYaeX, clr5DemoYaeZ,
                                                          w->model->rot[1], 0.15f);
                    }
                    if (clr5DemoFrame >= 0x196A) {
                        if (demoAnimeStep(w->model, 0.2f, 2)) {
                            demoAnimeSet(w->model, (u32) clr5DemoSuzakuModelAnim, 0xCD, 0.0f);
                            fadeAnimStart(&w->anim, clr5DemoSuzakuAnim, 0, &w->model->segs[3],
                                          floadGetSetFile(0xCD, 0));
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
                case 14:
                    if (clr5DemoFrame >= 0x1B5E) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], clr5DemoGoeX, clr5DemoGoeZ,
                                                          w->model->rot[1], 0.3f);
                        if (demoAnimeStep(w->model, 0.2f, 2)) {
                            demoAnimeSet(w->model, (u32) clr5DemoSuzakuAnim2, 0xCD, 0.0f);
                            fadeAnimStart(&w->anim, clr5DemoSuzakuAnim, 1, &w->model->segs[3],
                                          floadGetSetFile(0xCD, 0));
                            w->state++;
                            scRunEnd();
                            scRunStart(0x162, 0, 0);
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
                case 15:
                    fadeAnimStep(&w->anim);
                    if (clr5DemoFrame >= 0x1CBC) {
                        if (demoAnimeStep(w->model, 0.2f, 2)) {
                            demoAnimeSet(w->model, (u32) clr5DemoSuzakuModelAnim, 0xCD, 0.0f);
                            fadeAnimStart(&w->anim, clr5DemoSuzakuAnim, 0, &w->model->segs[3],
                                          floadGetSetFile(0xCD, 0));
                            scRunEnd();
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
                case 16:
                    if (demoAnimeStep(w->model, 0.2f, 2)) {
                        demoAnimeSet(w->model, (u32) clr5DemoSuzakuState16Anim, 0xCD, 0.0f);
                        fadeAnimStart(&w->anim, clr5DemoSuzakuAnim, 0, &w->model->segs[3],
                                      floadGetSetFile(0xCD, 0));
                        w->state++;
                    }
                    break;
                case 17:
                    if (demoAnimeStep(w->model, 0.1f, 2)) {
                        demoAnimeSet(w->model, (u32) clr5DemoSuzakuState17Anim, 0xCD, 0.0f);
                        fadeAnimStart(&w->anim, clr5DemoSuzakuAnim, 0, &w->model->segs[3],
                                      floadGetSetFile(0xCD, 0));
                        w->state++;
                    }
                    break;
                case 18:
                    if (demoAnimeStep(w->model, 0.4f, 2)) {
                        scRunEnd();
                        itexPuff(w->model->pos[0], w->model->pos[1] + 10.0f, w->model->pos[2], 0.1f);
                        sndPlay(0x19E);
                        w->model->pos[1] += 1000.0f;
                        w->state++;
                    }
                    break;
                case 19:
                    break;
            }
            clr5DemoSuzakuX = w->model->pos[0];
            clr5DemoSuzakuZ = w->model->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 clr5DemoSuzakuProc(Obj *obj, s16 msg, void *arg);

Obj *clr5DemoSuzakuCreate(Obj *obj) {
    return tskCreate(obj, clr5DemoSuzakuProc, obj->file, NULL);
}

extern Gfx clr5DemoDoucyuModelAnim[];                 /* the oni's first animation (file 0x29) */
extern Gfx clr5DemoDoucyuAnim2[];                 /* its second */
extern AnimFrame clr5DemoDoucyuAnim[]; /* its texture animation */
extern f32 clr5DemoDoucyuX;                   /* its x and z, each frame (where "EF DOUCYU" starts) */
extern f32 clr5DemoDoucyuZ;

void clr5DemoOnaraCreateSlow(Obj *obj, f32 x, f32 y, f32 z, s32 n, f32 s);
void clr5DemoSandOnaraCreate(Obj *obj, f32 x, f32 y, f32 z, s32 n, f32 s);
void clr5DemoEfDoucyuCreate(Obj *obj);

/* "doucyu": the Douchuu oni (file 0x29) with its shadow at (0, 0, 300); OBJMOVE's model 5 moves
 * it. It steps through its animations by the file's frame counter, letting out "ONARA" clouds
 * at frames 0xB5-0xB9, starting "EF DOUCYU" at 0x35F and the scenes 0x167, 0x150, 0x151, 0x153,
 * 0x154, 0x155 and 0x158 on the way. State 0's zeros are written 0 and 0.0f: the game makes two
 * fresh mtc1 zeros. */
s32 clr5DemoDoucyuProc(Obj *obj, s16 msg, void *arg) {
    DouW *w = (DouW *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "doucyu";
            PRINTF("ドウチュウ鬼作ります\n");
            w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->model == NULL) {
                return -1;
            }
            PRINTF("ドウチュウ鬼作りました\n");
            PRINTF("道テスト作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            PRINTF("道テスト作りました\n");
            demoAnimeSet(w->model, (u32) clr5DemoDoucyuModelAnim, 0x29, 0.0f);
            fadeAnimStart(&w->anim, clr5DemoDoucyuAnim, 0, &w->model->segs[3], floadGetSetFile(0x29, 0));
            demoPolSetDL(w->shadow, clr5DemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.08f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 0.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = 300.0f;
            w->model->rot[1] = 3.141592741f;
            break;
        case 1:
            frmDestroy(&w->model->f);
            frmDestroy(&w->shadow->f);
            break;
        case 3:
            w->model->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, gDrawSetupEnvTintTexShadeDl);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 0);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            w->model->f.flags |= 2;
            if (gObjMoveSel == 5) {
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
                    w->model->pos[0] = 0;
                    w->model->pos[1] = 0.0f;
                    w->model->pos[2] = 300.0f;
                    w->model->rot[1] = 3.141592741f;
                    w->state++;
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (clr5DemoFrame >= 0x88) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0x167, 0, 0);
                    }
                    break;
                case 2:
                    if (clr5DemoFrame >= 0xA1) {
                        demoAnimeSet(w->model, (u32) clr5DemoDoucyuAnim2, 0x29, 0.0f);
                        fadeAnimStart(&w->anim, clr5DemoDoucyuAnim, 0, &w->model->segs[3],
                                      floadGetSetFile(0x29, 0));
                        w->state++;
                    }
                    break;
                case 3:
                    demoAnimeStep(w->model, 1.0f, 2);
                    switch (clr5DemoFrame) {
                        case 0xB5:
                            sndPlay(0x21B);
                            (*clr5DemoSandOnaraCreate)(obj, w->model->pos[0] - 3.0f, w->model->pos[1],
                                                      w->model->pos[2] + 12.0f, 180, 0.05f);
                            (*clr5DemoSandOnaraCreate)(obj, w->model->pos[0] + 10.0f, w->model->pos[1],
                                                      w->model->pos[2] - 2.0f, 180, 0.07f);
                            break;
                        case 0xB7:
                            (*clr5DemoSandOnaraCreate)(obj, w->model->pos[0] + 5.0f, w->model->pos[1],
                                                      w->model->pos[2] + 20.0f, 180, 0.06f);
                            break;
                        case 0xB9:
                            (*clr5DemoSandOnaraCreate)(obj, w->model->pos[0] + 4.0f, w->model->pos[1],
                                                      w->model->pos[2] - 15.0f, 180, 0.05f);
                            break;
                        case 0xF7:
                            scRunEnd();
                            w->state++;
                            break;
                    }
                    break;
                case 4:
                    if (clr5DemoFrame >= 0x218) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0x150, 0, 0);
                    }
                    break;
                case 5:
                    if (clr5DemoFrame >= 0x35F) {
                        w->state++;
                        clr5DemoEfDoucyuCreate(obj);
                        sndPlay(0x292);
                        sndPlay(0x191);
                    }
                    break;
                case 6:
                    if (clr5DemoFrame >= 0x3FA) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0x151, 0, 0);
                    }
                    break;
                case 7:
                    if (clr5DemoFrame == 0x50D) {
                        scRunEnd();
                    }
                    if (clr5DemoFrame >= 0x620) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0x153, 0, 0);
                    }
                    break;
                case 8:
                    if (clr5DemoFrame >= 0x684) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0x154, 0, 0);
                    }
                    break;
                case 9:
                    if (clr5DemoFrame >= 0x77E) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0x155, 0, 0);
                    }
                    break;
                case 10:
                    if (clr5DemoFrame >= 0xA99) {
                        w->state++;
                        clr5DemoOnaraCreateSlow(obj, w->model->pos[0] + 5.0f, w->model->pos[1] + 30.0f,
                                               w->model->pos[2] - 5.0f, 180, 0.15f);
                    }
                    break;
                case 11:
                    if (clr5DemoFrame == 0xAB2) {
                        scRunEnd();
                        scRunStart(0x158, 0, 0);
                    }
                    if (clr5DemoFrame == 0xB48) {
                        w->state++;
                        scRunEnd();
                    }
                    break;
                case 12:
                    break;
            }
            clr5DemoDoucyuX = w->model->pos[0];
            clr5DemoDoucyuZ = w->model->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 clr5DemoDoucyuProc(Obj *obj, s16 msg, void *arg);

Obj *clr5DemoDoucyuCreate(Obj *obj) {
    return tskCreate(obj, clr5DemoDoucyuProc, obj->file, NULL);
}

s32 clr5DemoIdleProc(Obj *obj, s16 msg, void *arg) {
    return tskDefaultProc(obj, msg, arg);
}

Obj *clr5DemoIdleCreate(Obj *obj) {
    return tskCreate(obj, clr5DemoIdleProc, obj->file, NULL);
}

/* libultra's cosf, called through its address as tlb.h does for main's functions (tlb.h's
 * generator leaves libultra's math out). */
#define cosf (*cosf)

/* "EF DOUCYU"'s sway (clr5DemoEfDoucyuProc calls it): turns *angle by step degrees, past 360
 * back round, and returns 0.02 times its cosine. */
f32 clr5DemoEfDoucyuSway(s32 *angle, s32 step) {
    s32 a = *angle + step;

    if (a > 360) {
        a -= 360;
    }
    *angle = a;
    return cosf(a * 0.0174532930038800785) * 0.02;
}

/* "EF DOUCYU" (its proc clr5DemoEfDoucyuProc): its flame frame, texture animation and two
 * colours (as "EF FIRE"), its stage and its sway's angle. */
typedef struct DoucyuW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ Anim anim;
    /* 0x14 */ u8 prim[3];
    /* 0x17 */ u8 env[3];
    /* 0x1A */ u8 alpha;
    /* 0x1B */ u8 pad1B[0x9];
    /* 0x24 */ s32 state;
    /* 0x28 */ s32 angle; /* clr5DemoEfDoucyuSway's */
} DoucyuW;

extern Gfx clr5DemoEfDoucyuFlameDl[];                   /* the flame's display list (file 0xCC) */
extern AnimFrame clr5DemoEfDoucyuAnim[];   /* its texture animation */
extern f32 clr5DemoFireX;                     /* where it is now, for "EF FIRE": x */
extern f32 clr5DemoFireY;                     /* y */
extern f32 clr5DemoFireZ;                     /* z */
extern f32 clr5DemoFireScale;                     /* its scale */

Obj *clr5DemoEfFireCreate(Obj *obj);

/* "EF DOUCYU": a flame (file 0xCC) that rises, sways and grows by demo frame (0x369, 0x607,
 * 0x846, 0x9B6, 0xA99), with an "EF FIRE" following it. */
s32 clr5DemoEfDoucyuProc(Obj *obj, s16 msg, void *arg) {
    DoucyuW *w = (DoucyuW *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "EF DOUCYU";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, clr5DemoEfDoucyuFlameDl, 0xCC);
            fadeAnimStart(&w->anim, clr5DemoEfDoucyuAnim, 0, &w->frame->segs[3], floadGetSetFile(0xCC, 0));
            w->frame->pos[0] = clr5DemoDoucyuX;
            clr5DemoFireX = w->frame->pos[0];
            w->frame->pos[1] = 10.0f;
            clr5DemoFireY = w->frame->pos[1];
            w->frame->pos[2] = clr5DemoDoucyuZ;
            clr5DemoFireZ = w->frame->pos[2];
            w->frame->scale[2] = 0.11f;
            clr5DemoFireScale = w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->flags |= 0x22;
            w->alpha = 255;
            w->prim[0] = w->prim[1] = w->prim[2] = w->env[0] = w->env[1] = w->env[2] = 200;
            clr5DemoEfFireCreate(obj);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = demoColorDL(w->prim[0], w->prim[1], w->prim[2], w->env[0], w->env[1], w->env[2],
                                       w->alpha);
            break;
        case 0:
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            switch (w->state) {
                case 0:
                    if (clr5DemoFrame >= 0x369) {
                        w->state++;
                    }
                    break;
                case 1:
                    w->frame->pos[1] += 0.2;
                    if (20.0f < w->frame->pos[1]) {
                        w->state++;
                    }
                    break;
                case 2:
                    w->frame->pos[1] += clr5DemoEfDoucyuSway(&w->angle, 4);
                    if (clr5DemoFrame >= 0x607) {
                        w->state++;
                        w->frame->pos[1] = 35.0f;
                        w->frame->pos[2] = 250.0f;
                        w->frame->scale[2] = 0.3f;
                        clr5DemoFireScale = w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                    }
                    break;
                case 3:
                    w->frame->pos[1] += clr5DemoEfDoucyuSway(&w->angle, 4);
                    if (clr5DemoFrame >= 0x846) {
                        w->state++;
                    }
                    break;
                case 4:
                    if (clr5DemoFrame >= 0x9B6) {
                        w->frame->pos[1] = 40.0f;
                        w->frame->pos[2] = clr5DemoDoucyuZ;
                        w->frame->scale[2] = 0.11f;
                        clr5DemoFireScale = w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                        w->state++;
                    } else {
                        w->frame->pos[1] += 0.2;
                    }
                    break;
                case 5:
                    w->frame->pos[1] += clr5DemoEfDoucyuSway(&w->angle, 4);
                    if (clr5DemoFrame >= 0xA99) {
                        fadeAnimStart(&w->anim, clr5DemoEfDoucyuAnim, 1, &w->frame->segs[3],
                                      floadGetSetFile(0xCC, 0));
                        w->state++;
                    }
                    break;
                case 6:
                    w->frame->pos[1] += clr5DemoEfDoucyuSway(&w->angle, 60);
                    break;
            }
            clr5DemoFireX = w->frame->pos[0];
            clr5DemoFireY = w->frame->pos[1];
            clr5DemoFireZ = w->frame->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 clr5DemoEfDoucyuProc(Obj *obj, s16 msg, void *arg);

/* Starts an "EF DOUCYU" under obj. It returns nothing: doucyu's call to it keeps the game's
 * registers only so. */
void clr5DemoEfDoucyuCreate(Obj *obj) {
    tskCreate(obj, clr5DemoEfDoucyuProc, obj->file, NULL);
}

/* "EF FIRE" (its proc clr5DemoEfFireProc): its flame frame, texture animation and two
 * colours (as tlb_0031_1's TORCH). */
typedef struct FireW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ Anim anim;
    /* 0x14 */ u8 prim[3];
    /* 0x17 */ u8 env[3];
    /* 0x1A */ u8 alpha;
} FireW;

extern Gfx clr5DemoEfFireFlameDl[];                   /* the flame's display list (file 0xCC) */
extern AnimFrame clr5DemoFlameAnim[];   /* the flame's texture animation */

/* "EF FIRE": a flame (file 0xCC) with an animated texture that follows the point and scale
 * "EF DOUCYU" sets each frame. */
s32 clr5DemoEfFireProc(Obj *obj, s16 msg, void *arg) {
    FireW *w = (FireW *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "EF FIRE";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, clr5DemoEfFireFlameDl, 0xCC);
            fadeAnimStart(&w->anim, clr5DemoFlameAnim, 0, &w->frame->segs[3], floadGetSetFile(0xCC, 0));
            w->frame->pos[0] = clr5DemoFireX;
            w->frame->pos[1] = clr5DemoFireY;
            w->frame->pos[2] = clr5DemoFireZ;
            w->frame->scale[2] = clr5DemoFireScale;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->flags |= 0x22;
            w->alpha = 255;
            w->prim[0] = w->prim[1] = w->prim[2] = w->env[0] = w->env[1] = w->env[2] = 200;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = demoColorDL(w->prim[0], w->prim[1], w->prim[2], w->env[0], w->env[1], w->env[2],
                                       w->alpha);
            break;
        case 0:
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            fadeAnimStep(&w->anim);
            w->frame->pos[0] = clr5DemoFireX;
            w->frame->pos[1] = clr5DemoFireY;
            w->frame->pos[2] = clr5DemoFireZ;
            w->frame->scale[2] = clr5DemoFireScale;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *clr5DemoEfFireCreate(Obj *obj) {
    return tskCreate(obj, clr5DemoEfFireProc, obj->file, NULL);
}

/* The two "ONARA" (fart cloud) tasks' work: what their starters set. */
typedef struct OnaraW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ Anim anim;
    /* 0x14 */ u8 pad14[0x10];
    /* 0x24 */ f32 rise; /* its rise each step */
    /* 0x28 */ f32 grow;  /* its scale's factor each step */
    /* 0x2C */ s32 n;     /* its alpha */
    /* 0x30 */ s32 fade; /* the alpha's fade each step */
    /* 0x34 */ s32 altAnim; /* which texture animation */
} OnaraW;

#define ONARA(o) ((OnaraW *) FX(o))

s32 clr5DemoOnaraProc(Obj *obj, s16 msg, void *arg);

extern Gfx clr5DemoCloudDl[];                      /* the cloud's display list (file 0x123) */
extern AnimFrame clr5DemoOnaraAnim[];       /* its texture animation */
extern AnimFrame clr5DemoOnaraAnimAlt[];       /* its other texture animation (altAnim) */

/* "ONARA": a green cloud (file 0x123) that grows, rises and fades out. */
/* The animation table (the ternary) spills to 0x2C, one word above w's: only when
 * floadGetSetFile's result, passed straight to fadeAnimStart, is not an s16. IDO keeps one
 * stack word per type for a call result used as an argument (IDO_NOTES 375), and the s16 one
 * sat between them (real 3, the permuter's find: an int return type). Main's function returns
 * s16; the game's TLB headers likely declared it returning an int. The call through a cast
 * pointer stands for that (FAKE). Earlier forms, all one word off: pads before or after w,
 * the ternary in a local, no w, `altAnim == 0 ?`. */
s32 clr5DemoOnaraProc(Obj *obj, s16 msg, void *arg) {
    s32 pad[2];
    OnaraW *w = (OnaraW *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "ONARA";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, clr5DemoCloudDl, 0x123);
            fadeAnimStart(&w->anim, w->altAnim ? clr5DemoOnaraAnimAlt : clr5DemoOnaraAnim, 0,
                          &w->frame->segs[3], ((u32 (*)(s32, s32)) floadGetSetFile)(0x123, 0));
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = demoColorDL(40, 160, 40, 20, 100, 10, w->n);
            break;
        case 0:
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            if (gDemoModelsDelete == 1) {
                return -1;
            }
            fadeAnimStep(&w->anim);
            w->n -= w->fade;
            if (w->n <= 0) {
                w->n = 0;
                return -1;
            }
            w->frame->scale[0] *= w->grow;
            w->frame->scale[1] *= w->grow;
            w->frame->scale[2] *= w->grow;
            w->frame->pos[1] += w->rise;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 clr5DemoSandOnaraProc(Obj *obj, s16 msg, void *arg);

/* The four starters below copy n (the fifth argument) first: the game loads it before the
 * constant stores and stores it after them (IDO_NOTES 295). */

/* An "ONARA" (clr5DemoOnaraProc) at x, y, z, scaled s, growing by 1.1 (fade 10). */
void clr5DemoOnaraCreate(Obj *obj, f32 x, f32 y, f32 z, s32 n, f32 s) {
    Obj *o = tskCreate(obj, clr5DemoOnaraProc, obj->file, NULL);

    if (o != NULL) {
        ONARA(o)->frame->flags |= 0x27;
        ONARA(o)->frame->scale[0] = ONARA(o)->frame->scale[1] = ONARA(o)->frame->scale[2] = s;
        ONARA(o)->frame->pos[0] = x;
        ONARA(o)->frame->pos[1] = y;
        ONARA(o)->frame->pos[2] = z;
        ONARA(o)->n = n;
        ONARA(o)->fade = 10;
        ONARA(o)->rise = 0.0f;
        ONARA(o)->grow = 1.1f;
        ONARA(o)->altAnim = 0;
    }
}

/* The same, growing by 1.02 (fade and altAnim 1). */
void clr5DemoOnaraCreateSlow(Obj *obj, f32 x, f32 y, f32 z, s32 n, f32 s) {
    Obj *o = tskCreate(obj, clr5DemoOnaraProc, obj->file, NULL);

    if (o != NULL) {
        ONARA(o)->frame->flags |= 0x27;
        ONARA(o)->frame->scale[0] = ONARA(o)->frame->scale[1] = ONARA(o)->frame->scale[2] = s;
        ONARA(o)->frame->pos[0] = x;
        ONARA(o)->frame->pos[1] = y;
        ONARA(o)->frame->pos[2] = z;
        ONARA(o)->n = n;
        ONARA(o)->fade = 1;
        ONARA(o)->rise = 0.0f;
        ONARA(o)->grow = 1.02f;
        ONARA(o)->altAnim = 1;
    }
}

/* The other "ONARA": clr5DemoOnaraProc in a sandy colour. */
/* Its twin's C, with the same FAKE call (see clr5DemoOnaraProc). */
s32 clr5DemoSandOnaraProc(Obj *obj, s16 msg, void *arg) {
    s32 pad[2];
    OnaraW *w = (OnaraW *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "ONARA";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, clr5DemoCloudDl, 0x123);
            fadeAnimStart(&w->anim, w->altAnim ? clr5DemoOnaraAnimAlt : clr5DemoOnaraAnim, 0,
                          &w->frame->segs[3], ((u32 (*)(s32, s32)) floadGetSetFile)(0x123, 0));
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = demoColorDL(220, 200, 170, 200, 180, 170, w->n);
            break;
        case 0:
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            if (gDemoModelsDelete == 1) {
                return -1;
            }
            fadeAnimStep(&w->anim);
            w->n -= w->fade;
            if (w->n <= 0) {
                w->n = 0;
                return -1;
            }
            w->frame->scale[0] *= w->grow;
            w->frame->scale[1] *= w->grow;
            w->frame->scale[2] *= w->grow;
            w->frame->pos[1] += w->rise;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The other "ONARA" (clr5DemoSandOnaraProc), as clr5DemoOnaraCreate. */
void clr5DemoSandOnaraCreate(Obj *obj, f32 x, f32 y, f32 z, s32 n, f32 s) {
    Obj *o = tskCreate(obj, clr5DemoSandOnaraProc, obj->file, NULL);

    if (o != NULL) {
        ONARA(o)->frame->flags |= 0x27;
        ONARA(o)->frame->scale[0] = ONARA(o)->frame->scale[1] = ONARA(o)->frame->scale[2] = s;
        ONARA(o)->frame->pos[0] = x;
        ONARA(o)->frame->pos[1] = y;
        ONARA(o)->frame->pos[2] = z;
        ONARA(o)->n = n;
        ONARA(o)->fade = 10;
        ONARA(o)->rise = 0.0f;
        ONARA(o)->grow = 1.1f;
        ONARA(o)->altAnim = 0;
    }
}

/* The other "ONARA", as clr5DemoOnaraCreateSlow. */
void clr5DemoSandOnaraCreateSlow(Obj *obj, f32 x, f32 y, f32 z, s32 n, f32 s) {
    Obj *o = tskCreate(obj, clr5DemoSandOnaraProc, obj->file, NULL);

    if (o != NULL) {
        ONARA(o)->frame->flags |= 0x27;
        ONARA(o)->frame->scale[0] = ONARA(o)->frame->scale[1] = ONARA(o)->frame->scale[2] = s;
        ONARA(o)->frame->pos[0] = x;
        ONARA(o)->frame->pos[1] = y;
        ONARA(o)->frame->pos[2] = z;
        ONARA(o)->n = n;
        ONARA(o)->fade = 1;
        ONARA(o)->rise = 0.0f;
        ONARA(o)->grow = 1.02f;
        ONARA(o)->altAnim = 1;
    }
}

/* "model" (its proc clr5DemoModelProc): its frame and its env colour. */
typedef struct ModelW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ s32 r;
    /* 0x08 */ s32 g;
    /* 0x0C */ s32 b;
    /* 0x10 */ s32 a;
} ModelW;

extern Gfx clr5DemoModelPolDl[];               /* the model's display list (file 0x2B) */
extern Gfx clr5DemoModelDl[];     /* its render setup */

/* The file's .data, in address order (its externs above stay as declarations). */
extern u8 demoPieceTex0[];
extern u8 demoPieceTex1[];
extern u8 clr5DemoOnaraTex0[];
extern u8 clr5DemoEfDoucyuTex[];
extern u8 demoPieceTex2[];
extern u8 clr5DemoOnaraTex1[];
extern u8 demoPieceTex4[];
extern u8 clr5DemoOnaraTex2[];
extern u8 clr5DemoOnaraTex3[];
extern u8 demoPieceTex7[];
extern u8 clr5DemoFlameTex1[];
extern u8 clr5DemoOnaraTex4[];
extern u8 clr5DemoOnaraTex6[];
extern u8 clr5DemoFlameTex4[];
extern u8 demoPieceTex15[];
extern u8 demoPieceTex19[];
extern u8 clr5DemoDoucyuTex0[];
extern u8 clr5DemoDoucyuTex1[];
extern u8 demoPieceTex78[];
extern u8 demoFaceTex0[];
extern u8 demoFaceTex1[];
extern u8 demoFaceTex2[];
AnimFrame clr5DemoCharaAnims[] = {
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
    { (u32) clr5DemoOnaraTex1, 8 },
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
    { (u32) clr5DemoOnaraTex1, 7 },
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
    { (u32) clr5DemoOnaraTex1, 200 },
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
Gfx clr5DemoModelDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetCombineLERP(ENVIRONMENT, TEXEL0, ENV_ALPHA, TEXEL0, TEXEL0, 0, SHADE, 0, ENVIRONMENT, TEXEL0, ENV_ALPHA, TEXEL0, TEXEL0, 0, SHADE, 0),
    gsDPSetEnvColor(0x00, 0xFF, 0xFF, 0xFF),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPPipeSync(),
    gsSPEndDisplayList(),
};     /* its render setup */
AnimFrame clr5DemoSuzakuAnim[] = {
    { 0x03009E30, 10 },
    { 0x0300A250, 8 },
    { 0x00000000, 0 },
}; /* his texture animations */
AnimFrame clr5DemoSuzakuAnimB[] = { { 0x03009E30, 8 }, { 0x0300A250, 8 }, { 0x0300A670, 8 }, { 0x0300AA90, 8 }, { 0x00000000, 0 } };
u32 clr5DemoUnused2[] = { 0x00000064 };
s32 clr5DemoSuzakuUnusedFlag = 1;
u32 clr5DemoUnused3 = 0x3F800000;
AnimFrame clr5DemoDoucyuAnim[] = {
    { (u32) clr5DemoDoucyuTex0, 7 },
    { (u32) clr5DemoDoucyuTex1, 12 },
    { 0x00000000, 0 },
}; /* its texture animation */
AnimFrame clr5DemoEfDoucyuAnim[] = {
    { (u32) clr5DemoEfDoucyuTex, 2 },
    { 0x03001020, 2 },
    { 0x00000000, 0 },
};   /* its texture animation */
AnimFrame clr5DemoFlameAnim[] = {
    { 0x03001820, 2 },
    { (u32) clr5DemoFlameTex1, 2 },
    { 0x03002820, 2 },
    { 0x03003020, 2 },
    { (u32) clr5DemoFlameTex4, 2 },
    { 0x03004020, 2 },
    { 0x03004820, 2 },
    { 0x03005020, 2 },
    { 0x03005820, 2 },
    { 0x03006020, 2 },
    { 0x03006820, 2 },
    { 0x03007020, 2 },
    { 0x03007820, 2 },
    { 0x03008020, 2 },
    { 0x03008820, 2 },
    { 0x03009020, 2 },
    { 0x00000000, 0 },
};   /* the flame's texture animation */
AnimFrame clr5DemoOnaraAnim[] = {
    { (u32) clr5DemoOnaraTex0, 3 },
    { (u32) clr5DemoOnaraTex1, 3 },
    { (u32) clr5DemoOnaraTex2, 3 },
    { (u32) clr5DemoOnaraTex3, 3 },
    { (u32) clr5DemoOnaraTex4, 3 },
    { 0x03002D30, 4 },
    { (u32) clr5DemoOnaraTex6, 5 },
    { 0x00000000, 0 },
};       /* its texture animation */
AnimFrame clr5DemoOnaraAnimAlt[] = { { (u32) clr5DemoOnaraTex0, 8 }, { (u32) clr5DemoOnaraTex1, 8 }, { (u32) clr5DemoOnaraTex2, 8 }, { (u32) clr5DemoOnaraTex3, 10 }, { (u32) clr5DemoOnaraTex4, 10 }, { 0x03002D30, 12 }, { (u32) clr5DemoOnaraTex6, 15 }, { 0x00000000, 0 }, { 0x00000000, 0 } };       /* its other texture animation (altAnim) */

/* "model": a model (file 0x2B) at (153, -113, 196) whose env colour's red and green pulse;
 * OBJMOVE's model 6 moves it. */
s32 clr5DemoModelProc(Obj *obj, s16 msg, void *arg) {
    ModelW *w = (ModelW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "model";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, clr5DemoModelPolDl, 0x2B);
            w->frame->flags |= 0x20;
            w->frame->scale[0] = 0.16f;
            w->frame->scale[1] = 0.16f;
            w->frame->scale[2] = 0.16f;
            w->frame->pos[0] = 153.0f;
            w->frame->pos[1] = -113.0f;
            w->frame->pos[2] = 196.0f;
            w->frame->rot[0] = 0.0f;
            w->frame->rot[1] = -1.570796371f;
            w->frame->rot[2] = 0.0f;
            w->r = 50; w->g = 70; w->b = 250;
            w->a = 170;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            DPRINTF("モデルを消しました\n");
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, fadeSegToRam(clr5DemoModelDl, obj->file));
            gDPSetEnvColor(gGfxDlHead++, w->r, w->g, w->b, w->a);
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
            w->r = sinf(clr5DemoFrame * 4 * 0.0174532930038800785) * 50.0f + 80.0f;
            w->g = sinf(clr5DemoFrame * 5 * 0.0174532930038800785) * 50.0f + 120.0f;
            if (gDemoModelsDelete == 1 || gDemoModelsDelete2 == 1) {
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 clr5DemoModelProc(Obj *obj, s16 msg, void *arg);

Obj *clr5DemoModelCreate(Obj *obj) {
    return tskCreate(obj, clr5DemoModelProc, obj->file, NULL);
}
