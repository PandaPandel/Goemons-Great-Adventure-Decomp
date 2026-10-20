#include "tlb.h"
#include "tlb/tlb_0025.h"


/* A demo character's work (tlb_0026_1's DemoPlW). */
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

#define DPRINTF

#define sinf (*sinf)

extern Gfx yaeDemoShadowDl[]; /* the shadow */
extern u8 gDemoCamHold;
extern s32 gObjMoveSel;
extern f32 gObjMoveDx;
extern f32 gObjMoveDy;
extern f32 gObjMoveDz;
extern f32 gObjMoveDrot;
extern f32 gObjMovePosX;
extern f32 gObjMovePosY;
extern f32 gObjMovePosZ;
extern f32 gObjMoveRot;
/* The file's .bss (0x08006620 on), laid out by first declaration: these globals in address
 * order, then the demo characters' statics in function order. The words nothing reads are
 * declared where they sit. */
s32 yaeDemoFrame; /* the scene's frame count (tlb_0025_0 counts it) */
s32 yaeDemoUnused2; /* unread */
s32 yaeDemoUnused3; /* unread */
s32 yaeDemoUnused4; /* unread */
f32 yaeDemoSasX; /* Sasuke's x and z, each frame */
f32 yaeDemoSasZ;
f32 yaeDemoGoeX; /* Goemon's x and z, each frame */
f32 yaeDemoGoeZ;
f32 yaeDemoEbiX; /* Ebisumaru's x and z, each frame */
f32 yaeDemoEbiZ;
f32 yaeDemoYaeX; /* Yae's x and z, each frame: the point the others turn toward */
f32 yaeDemoYaeZ;
s32 yaeDemoUnused5; /* unread */
s32 yaeDemoUnused6; /* unread */
f32 yaeDemoYaeTurnX; /* the point Yae turns toward */
f32 yaeDemoYaeTurnZ;

/* "genoGOE" (Goemon, character 0, in the demo) at (20, 0, 23) under the menu's last task;
 * OBJMOVE's model 0, his shadow at y 0.3 under him. 20 steps on the scene's frame count: he
 * turns toward Yae and links his actions (scene runs 0x9E, 0xA3, 0xA9, 0xB4, 0xB5, 0xB6); at
 * 0x730 and 0x1356 he is put back near where he started. */
s32 yaeDemoGoeProc(Obj *obj, s16 msg, void *arg) {
    static s32 step;  /* .bss 0x08006660: his demoPlTunagiAnime step */
    static f32 speed; /* .bss 0x08006664: his animation speed in step 17 */
    static Player *p; /* .bss 0x08006668: his player */
    s32 pad[2];
    DemoPlW *w = (DemoPlW *) (obj + 1);
    PlInit init;

    DPRINTF("goecreate\n"); /* the file's rodata has it first, before the name */
    switch (msg) {
        case 2:
            obj->name = "genoGOE";
            plMakeInit(&init, 0, 0);
            init.flags |= 0x90000040;
            init.flags2 |= 8;
            init.zone = 0;
            init.place = 20.0f;
            init.height = 0.0f;
            init.side = 23.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            DPRINTF("歩きモーション\n");
            plAnimeSet(PL(w->pl), 0xE9);
            w->model = p->model;
            ((Frame *) p->shadow)->flags &= ~2;
            DPRINTF("*** p->model_buff=%x\n", p->model);
            DPRINTF("ゴエモン作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, yaeDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.07999999821f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->chara = 0;
            w->model->rot[1] = 3.1415927f;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 20.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = 23.0f;
            w->model->rot[1] = 3.3684855f;
            DPRINTF("ゴエモン作りました\n");
            DPRINTF("goecreate\n");
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
            yaeDemoGoeX = w->model->pos[0];
            yaeDemoGoeZ = w->model->pos[2];
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (yaeDemoFrame == 60) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 1:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], yaeDemoYaeX, yaeDemoYaeZ,
                                                      w->model->rot[1], 1.0f);
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.3f, 0.4f, 0xEB, 0xEC)) == -1) {
                        scRunEnd();
                        scRunStart(0x9E, 0, 0);
                        w->state++;
                        step = 0;
                    }
                    break;
                case 2:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], yaeDemoYaeX, yaeDemoYaeZ,
                                                      w->model->rot[1], 1.0f);
                    demoAnimeStep(w->model, 0.3f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 190) {
                        w->state++;
                    }
                    break;
                case 3:
                    if ((step = demoPlTunagiAnime(obj, 5, step, 2, 3, 0.3f, 0.4f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 4:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (yaeDemoFrame == 0x488) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 5:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.3f, 0.4f, 0xEB, 0xEC)) == -1) {
                        scRunEnd();
                        scRunStart(0xA3, 0, 0);
                        w->state++;
                        step = 0;
                    }
                    break;
                case 6:
                    demoAnimeStep(w->model, 0.3f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 0x528) {
                        w->state++;
                    }
                    break;
                case 7:
                    if ((step = demoPlTunagiAnime(obj, 5, step, 2, 3, 0.3f, 0.4f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 8:
                    demoAnimeStep(w->model, 0.1f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 0x730) {
                        w->model->pos[0] = 20.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 23.0f;
                        w->model->rot[1] = 3.5953784f;
                    }
                    if (yaeDemoFrame == 3000) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 9:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.3f, 0.4f, 0xEB, 0xEC)) == -1) {
                        scRunEnd();
                        scRunStart(0xA9, 0, 0);
                        w->state++;
                        step = 0;
                    }
                    break;
                case 10:
                    demoAnimeStep(w->model, 0.6f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 0xC80) {
                        w->state++;
                    }
                    break;
                case 11:
                    if ((step = demoPlTunagiAnime(obj, 5, step, 2, 3, 0.3f, 0.4f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 12:
                    demoAnimeStep(w->model, 0.1f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 0x1144) {
                        plAnimeSet(PL(w->pl), 0xFF);
                        demoTexAnimK0(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 13:
                    if (demoAnimeStep(w->model, 0.1f, 2)) {
                        plAnimeSet(PL(w->pl), 0x100);
                        w->state++;
                    }
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
                case 14:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (yaeDemoFrame == 0x1356) {
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK0(&w->eyes, 5, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 5, &PL(w->pl)->model->segs[4]);
                        w->model->pos[0] = 20.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 23.0f;
                        w->model->rot[1] = 4.310963f;
                        w->state++;
                    }
                    break;
                case 15:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (yaeDemoFrame == 0x16C6) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 16:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.3f, 0.4f, 0xEB, 0xEC)) == -1) {
                        scRunEnd();
                        scRunStart(0xB4, 0, 0);
                        w->state++;
                        step = 0;
                        speed = 0.3f;
                    }
                    break;
                case 17:
                    demoAnimeStep(w->model, speed, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    switch (yaeDemoFrame) {
                        case 0x1766:
                            scRunEnd();
                            scRunStart(0xB5, 0, 0);
                            break;
                        case 0x1842:
                            scRunEnd();
                            speed = 0.1f;
                            break;
                        case 0x186A:
                            scRunStart(0xB6, 0, 0);
                            speed = 0.5f;
                            break;
                        case 0x1914:
                            w->state++;
                            break;
                    }
                    break;
                case 18:
                    if ((step = demoPlTunagiAnime(obj, 5, step, 2, 3, 0.3f, 0.4f, 0xEB, 0xE9)) == -1) {
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

Obj *yaeDemoGoeCreate(Obj *obj) {
    return tskCreate(obj, yaeDemoGoeProc, obj->file, NULL);
}

/* "genoGOE" (the game's name; it is Ebisumaru, character 1, and prints "EBI STEP") at
 * (40, 0, 21) under the menu's last task; OBJMOVE's model 1, his shadow at y 0.3 under him.
 * 23 steps on the scene's frame count: he turns toward Yae and links his actions (scene runs
 * 0x9F, 0xA4, 0xAA, 0xB3); at 0x47E, 0x730, 0x113A and 0x1356 he is put in place. */
s32 yaeDemoEbiProc(Obj *obj, s16 msg, void *arg) {
    static s32 tunagiStep; /* .bss 0x0800666C */
    s32 pad[2];
    DemoPlW *w = (DemoPlW *) (obj + 1);
    Player *p;
    PlInit init;

    switch (msg) {
        case 2:
            obj->name = "genoGOE";
            plMakeInit(&init, 0, 1);
            init.flags |= 0x90000040;
            init.flags2 |= 8;
            init.zone = 0;
            init.place = 40.0f;
            init.height = 0.0f;
            init.side = 21.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            DPRINTF("歩きモーション\n");
            plAnimeSet(PL(w->pl), 0xE9);
            DPRINTF("EBI STEP --->%d\n", w->state);
            w->model = p->model;
            ((Frame *) p->shadow)->flags &= ~2;
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, yaeDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.09f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 1;
            w->state = 0;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 40.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = 21.0f;
            w->model->rot[1] = -2.7750735f;
            DPRINTF("エビス丸作りました\n");
            DPRINTF("goecreate\n");
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
            yaeDemoEbiX = w->model->pos[0];
            yaeDemoEbiZ = w->model->pos[2];
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (yaeDemoFrame == 190) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 1:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], yaeDemoYaeX, yaeDemoYaeZ,
                                                      w->model->rot[1], 1.0f);
                    if ((tunagiStep = demoPlTunagiAnime(obj, 2, tunagiStep, 2, 2, 0.3f, 0.3f, 0xEB, 0xEC)) == -1) {
                        w->state++;
                        tunagiStep = 0;
                        scRunEnd();
                        scRunStart(0x9F, 0, 0);
                    }
                    break;
                case 2:
                    demoAnimeStep(w->model, 0.5f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 0x122) {
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 3:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 0, tunagiStep, 2, 3, 0.4f, 0.1f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                        scRunEnd();
                        tunagiStep = 0;
                    }
                    break;
                case 4:
                    if (yaeDemoFrame == 0x47E) {
                        w->model->pos[0] = 35.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 22.0f;
                        w->model->rot[1] = -2.373648f;
                        w->state++;
                        break;
                    }
                case 5:
                    demoAnimeStep(w->model, 0.1f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 0x528) {
                        w->state++;
                    }
                    break;
                case 6:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 2, tunagiStep, 2, 2, 0.3f, 0.3f, 0xEB, 0xEC)) == -1) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0xA4, 0, 0);
                        tunagiStep = 0;
                    }
                    break;
                case 7:
                    demoAnimeStep(w->model, 0.4f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 0x640) {
                        w->state++;
                        scRunEnd();
                    }
                    break;
                case 8:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 0, tunagiStep, 2, 3, 0.3f, 0.3f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 9:
                    demoAnimeStep(w->model, 0.1f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 0x730) {
                        w->model->pos[0] = 40.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 21.0f;
                        w->model->rot[1] = 4.0142574f;
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], yaeDemoYaeX, yaeDemoYaeZ,
                                                          w->model->rot[1], 40.0f);
                        w->state++;
                    }
                    break;
                case 10:
                    demoAnimeStep(w->model, 0.1f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 0xC80) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 11:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 2, tunagiStep, 2, 2, 0.3f, 0.3f, 0xEB, 0xEC)) == -1) {
                        w->state++;
                        tunagiStep = 0;
                        scRunEnd();
                        scRunStart(0xAA, 0, 0);
                    }
                    break;
                case 12:
                    demoAnimeStep(w->model, 0.5f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 0xD48) {
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 13:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 0, tunagiStep, 2, 3, 0.3f, 0.3f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 14:
                    demoAnimeStep(w->model, 0.1f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 0x113A) {
                        w->model->pos[0] = 35.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 22.0f;
                        w->model->rot[1] = -2.373648f;
                        w->state++;
                    }
                    break;
                case 15:
                    demoAnimeStep(w->model, 0.1f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 0x1144) {
                        plAnimeSet(PL(w->pl), 0xFF);
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 16:
                    if (demoAnimeStep(w->model, 0.1f, 2)) {
                        plAnimeSet(PL(w->pl), 0x100);
                        w->state++;
                    }
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
                case 17:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (yaeDemoFrame == 0x1356) {
                        w->model->pos[0] = 35.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 22.0f;
                        w->model->rot[1] = -2.076942f;
                        plAnimeSet(PL(w->pl), 0xE9);
                        w->state++;
                    }
                    break;
                case 18:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (yaeDemoFrame == 0x15D6) {
                        w->state++;
                    }
                    break;
                case 19:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 2, tunagiStep, 2, 2, 0.3f, 0.3f, 0xEB, 0xEC)) == -1) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0xB3, 0, 0);
                        tunagiStep = 0;
                    }
                    break;
                case 20:
                    demoAnimeStep(w->model, 0.4f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 0x16C6) {
                        w->state++;
                        scRunEnd();
                    }
                    break;
                case 21:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 0, tunagiStep, 2, 3, 0.3f, 0.3f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 22:
                    demoAnimeStep(w->model, 0.1f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *yaeDemoEbiCreate(Obj *obj) {
    return tskCreate(obj, yaeDemoEbiProc, obj->file, NULL);
}

/* "chaSAS" (Sasuke, character 2, in the demo) at (4, 0, 21) under the menu's last task;
 * OBJMOVE's model 2, his shadow at y 0.3 under him. 19 steps on the scene's frame count: he
 * links his actions (scene runs 0xA5, 0xAB, 0xAE, 0xAF, 0xB0 with sound 0x456, 0xB1), is put
 * in place at 0x47E, 0x731, 0x113A and 0x1356, and turns toward Goemon, Yae and the point
 * between them. The integer 2s (pos[2] at 0x1356, the midpoint's halves) are the game's: 2.0f
 * gives a mul.s by 0.5 and a later load of the constant. */
s32 yaeDemoSasProc(Obj *obj, s16 msg, void *arg) {
    static s32 tunagiStep; /* .bss 0x08006670 */
    static f32 speed;      /* .bss 0x08006674: his animation speed in step 13 */
    s32 pad[2];
    DemoPlW *w = (DemoPlW *) (obj + 1);
    Player *p;
    PlInit init;

    switch (msg) {
        case 2:
            obj->name = "chaSAS";
            plMakeInit(&init, 0, 2);
            init.flags |= 0x90000040;
            init.flags2 |= 8;
            init.zone = 0;
            init.place = 4.0f;
            init.height = 0.0f;
            init.side = 21.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            DPRINTF("走りモーション\n");
            plAnimeSet(PL(w->pl), 0xE9);
            w->model = p->model;
            ((Frame *) p->shadow)->flags &= ~2;
            DPRINTF("サスケ作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, yaeDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.065f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 2;
            w->state = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 4.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = 21.0f;
            w->model->rot[1] = 3.089233f;
            DPRINTF("サスケ作りました\n");
            DPRINTF("goecreate\n");
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
            yaeDemoSasX = w->model->pos[0];
            yaeDemoSasZ = w->model->pos[2];
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = 0.3f;
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (yaeDemoFrame == 0x47E) {
                        w->model->pos[0] = 8.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 26.0f;
                        w->model->rot[1] = -2.984513f;
                        w->state++;
                        break;
                    }
                case 1:
                    demoAnimeStep(w->model, 0.1f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 0x640) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 2:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 2, tunagiStep, 2, 2, 0.5f, 0.6f, 0xEB, 0xEC)) == -1) {
                        scRunEnd();
                        scRunStart(0xA5, 0, 0);
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 3:
                    demoAnimeStep(w->model, 0.5f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 0x71C) {
                        w->state++;
                    }
                    break;
                case 4:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 0, tunagiStep, 2, 3, 0.3f, 0.4f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    if (yaeDemoFrame >= 0x731) {
                        w->model->pos[0] = 4.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 21.0f;
                        w->model->rot[1] = 3.089233f;
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], yaeDemoYaeX, yaeDemoYaeZ,
                                                          w->model->rot[1], 40.0f);
                        plAnimeSet(PL(w->pl), 0xE9);
                        w->state += 2;
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.1f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame >= 0x731) {
                        w->model->pos[0] = 4.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 21.0f;
                        w->model->rot[1] = 3.089233f;
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], yaeDemoYaeX, yaeDemoYaeZ,
                                                          w->model->rot[1], 40.0f);
                        plAnimeSet(PL(w->pl), 0xE9);
                        w->state++;
                    }
                    break;
                case 6:
                    demoAnimeStep(w->model, 0.1f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 0xD48) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 7:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 2, tunagiStep, 2, 2, 0.3f, 0.5f, 0xEB, 0xEC)) == -1) {
                        scRunEnd();
                        scRunStart(0xAB, 0, 0);
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 8:
                    demoAnimeStep(w->model, 0.7f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 0xE4C) {
                        w->state++;
                    }
                    break;
                case 9:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 0, tunagiStep, 2, 3, 0.6f, 0.4f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 10:
                    demoAnimeStep(w->model, 0.1f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 0x113A) {
                        w->model->pos[0] = 8.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 26.0f;
                        w->model->rot[1] = -2.984513f;
                        w->state++;
                    }
                    break;
                case 11:
                    demoAnimeStep(w->model, 0.1f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 0x1144) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 12:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 0, tunagiStep, 2, 2, 0.3f, 0.5f, 0xEB, 0xEC)) == -1) {
                        scRunEnd();
                        scRunStart(0xAE, 0, 0);
                        w->state++;
                        tunagiStep = 0;
                        speed = 0.2f;
                    }
                    break;
                case 13:
                    demoAnimeStep(w->model, speed, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 0x120C) {
                        scRunEnd();
                        scRunStart(0xAF, 0, 0);
                        demoTexAnimK2(&w->eyes, 4, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 4, &PL(w->pl)->model->segs[4]);
                        speed = 1.0f;
                    }
                    if (yaeDemoFrame == 0x1356) {
                        w->model->pos[0] = -19.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 2;
                        w->model->rot[1] = 1.3089969f;
                        plAnimeSet(PL(w->pl), 0xE9);
                        scRunEnd();
                        scRunStart(0xB0, 0, 0);
                        sndPlay(0x456);
                        w->state++;
                    }
                    break;
                case 14:
                    if (yaeDemoFrame == 0x140A) {
                        scRunEnd();
                    }
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (yaeDemoFrame == 0x146E) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 15:
                    tunagiStep = demoPlTunagiAnime(obj, 2, tunagiStep, 2, 2, 0.3f, 0.5f, 0xEB, 0xEC);
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], yaeDemoGoeX, yaeDemoGoeZ,
                                                      w->model->rot[1], 1.0f);
                    if (tunagiStep == -1) {
                        scRunEnd();
                        scRunStart(0xB1, 0, 0);
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 16:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], yaeDemoYaeX, yaeDemoYaeZ,
                                                      w->model->rot[1], 1.0f);
                    demoAnimeStep(w->model, 0.7f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 0x150E) {
                        w->state++;
                    }
                    break;
                case 17:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2],
                                                      (yaeDemoGoeX + yaeDemoYaeX) / 2,
                                                      (yaeDemoYaeZ + yaeDemoGoeZ) / 2, w->model->rot[1], 1.0f);
                    if ((tunagiStep = demoPlTunagiAnime(obj, 0, tunagiStep, 2, 3, 0.6f, 0.4f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 18:
                    demoAnimeStep(w->model, 0.1f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *yaeDemoSasCreate(Obj *obj) {
    return tskCreate(obj, yaeDemoSasProc, obj->file, NULL);
}

/* "chaYAE" (Yae, character 3, in the demo) at (0, 0, -14) under the menu's last task;
 * OBJMOVE's model 3, her shadow at y 0.3 under her. Seven steps on the scene's frame count:
 * she sways (a 30-degree sine about -105 degrees), turns toward Goemon, then by turns toward
 * Goemon, Ebisumaru and Sasuke as she talks (actions 0x101-0x103, scene runs 0xA0, 0xA1, 0xA2,
 * 0xA6, 0xA7, 0xA8, 0xAC, 0xAD, 0xB2); at 0x1356 she is put at (8, 0, -15). Her step switch
 * has an explicit empty case 7 (the game's jump table has 8 entries). */
s32 yaeDemoYaeProc(Obj *obj, s16 msg, void *arg) {
    static s32 reserved1; /* .bss 0x08006678-0x0800667C: two words of the game's .bss */
    static s32 reserved2; /* that nothing reads (scalars: IDO 8-aligns a .bss array) */
    static f32 speed;     /* .bss 0x08006680: her animation speed */
    s32 pad[2];
    DemoPlW *w = (DemoPlW *) (obj + 1);
    Player *p;
    PlInit init;

    switch (msg) {
        case 2:
            obj->name = "chaYAE";
            plMakeInit(&init, 0, 3);
            init.flags |= 0x90000040;
            init.flags2 |= 8;
            init.zone = 0;
            init.place = 0.0f;
            init.height = 0.0f;
            init.side = -14.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            DPRINTF("立ちモーション\n");
            plAnimeSet(PL(w->pl), 0xD7);
            w->model = p->model;
            ((Frame *) p->shadow)->flags &= ~2;
            DPRINTF("ヤエ作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, yaeDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.065f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 3;
            w->state = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 0.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = -14.0f;
            w->model->rot[1] = -1.8325957f;
            demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
            demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
            DPRINTF("ヤエ作りました\n");
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
            yaeDemoYaeX = w->model->pos[0];
            yaeDemoYaeZ = w->model->pos[2];
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = 0.3f;
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    w->model->rot[1] = sinf(yaeDemoFrame * 0.0174532930038800785) * 30.0f * 0.0174532930038800785 +
                                       -1.83259576540740832;
                    if (yaeDemoFrame == 220) {
                        w->state++;
                    }
                    break;
                case 1:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], yaeDemoGoeX, yaeDemoGoeZ,
                                                      w->model->rot[1], 2.0f);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (yaeDemoFrame == 0x168) {
                        scRunEnd();
                        scRunStart(0xA0, 0, 0);
                    }
                    if (yaeDemoFrame == 0x244) {
                        plAnimeSet(PL(w->pl), 0x101);
                        demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        speed = 0.2f;
                        yaeDemoYaeTurnX = yaeDemoGoeX;
                        yaeDemoYaeTurnZ = yaeDemoGoeZ;
                        w->state++;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->model, speed, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], yaeDemoYaeTurnX, yaeDemoYaeTurnZ,
                                                      w->model->rot[1], 0.5f);
                    switch (yaeDemoFrame) {
                        case 0x258:
                            scRunEnd();
                            scRunStart(0xA1, 0, 0);
                            speed = 0.4f;
                            demoTexAnimK3(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK3(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                            yaeDemoYaeTurnX = yaeDemoGoeX;
                            yaeDemoYaeTurnZ = yaeDemoGoeZ;
                            break;
                        case 0x320:
                            scRunEnd();
                            scRunStart(0xA2, 0, 0);
                            yaeDemoYaeTurnX = yaeDemoEbiX;
                            yaeDemoYaeTurnZ = yaeDemoEbiZ;
                            break;
                        case 0x44C:
                            scRunEnd();
                            plAnimeSet(PL(w->pl), 0xD7);
                            demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                            speed = 0.1f;
                            yaeDemoYaeTurnX = yaeDemoGoeX;
                            yaeDemoYaeTurnZ = yaeDemoGoeZ;
                            break;
                        case 0x744:
                            speed = 0.3f;
                            plAnimeSet(PL(w->pl), 0x101);
                            scRunEnd();
                            scRunStart(0xA6, 0, 0);
                            yaeDemoYaeTurnX = yaeDemoEbiX;
                            yaeDemoYaeTurnZ = yaeDemoEbiZ;
                            break;
                        case 0x8FC:
                            scRunEnd();
                            speed = 0.01f;
                            break;
                        case 0x938:
                            speed = 0.3f;
                            scRunStart(0xA7, 0, 0);
                            yaeDemoYaeTurnX = yaeDemoSasX;
                            yaeDemoYaeTurnZ = yaeDemoSasZ;
                            break;
                        case 0xA78:
                            scRunEnd();
                            speed = 0.01f;
                            yaeDemoYaeTurnX = yaeDemoGoeX;
                            yaeDemoYaeTurnZ = yaeDemoGoeZ;
                            break;
                        case 0xA96:
                            speed = 0.3f;
                            scRunStart(0xA8, 0, 0);
                            break;
                        case 0xBA4:
                            scRunEnd();
                            plAnimeSet(PL(w->pl), 0xD7);
                            speed = 0.1f;
                            w->state++;
                            break;
                    }
                    break;
                case 3:
                    demoAnimeStep(w->model, speed, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], yaeDemoYaeTurnX, yaeDemoYaeTurnZ,
                                                      w->model->rot[1], 0.5f);
                    if (yaeDemoFrame == 0xE6A) {
                        scRunEnd();
                        w->state++;
                        plAnimeSet(PL(w->pl), 0x102);
                        demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        yaeDemoYaeTurnX = yaeDemoEbiX;
                        yaeDemoYaeTurnZ = yaeDemoEbiZ;
                    }
                    break;
                case 4:
                    if (demoAnimeStep(w->model, 0.2f, 2)) {
                        speed = 0.3f;
                        plAnimeSet(PL(w->pl), 0x103);
                        demoTexAnimK3(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                        scRunStart(0xAC, 0, 0);
                        yaeDemoYaeTurnX = yaeDemoEbiX;
                        yaeDemoYaeTurnZ = yaeDemoEbiZ;
                        w->state++;
                    }
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], yaeDemoYaeTurnX, yaeDemoYaeTurnZ,
                                                      w->model->rot[1], 0.5f);
                    break;
                case 5:
                    demoAnimeStep(w->model, speed, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], yaeDemoYaeTurnX, yaeDemoYaeTurnZ,
                                                      w->model->rot[1], 0.5f);
                    switch (yaeDemoFrame) {
                        case 0xF96:
                            speed = 0.4f;
                            scRunEnd();
                            scRunStart(0xAD, 0, 0);
                            yaeDemoYaeTurnX = yaeDemoGoeX;
                            yaeDemoYaeTurnZ = yaeDemoGoeZ;
                            break;
                        case 0x1126:
                            speed = 0.04f;
                            scRunEnd();
                            break;
                        case 0x1356:
                            plAnimeSet(PL(w->pl), 0xD7);
                            demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                            w->model->pos[0] = 8.0f;
                            w->model->pos[1] = 0.0f;
                            w->model->pos[2] = -15.0f;
                            w->model->rot[1] = -1.2391838f;
                            w->state++;
                            speed = 0.1f;
                            break;
                    }
                    break;
                case 6:
                    demoAnimeStep(w->model, speed, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (yaeDemoFrame == 0x150E) {
                        scRunEnd();
                        scRunStart(0xB2, 0, 0);
                        speed = 0.4f;
                    }
                    if (yaeDemoFrame == 0x15D6) {
                        scRunEnd();
                        speed = 0.1f;
                    }
                    break;
                case 7:
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *yaeDemoYaeCreate(Obj *obj) {
    return tskCreate(obj, yaeDemoYaeProc, obj->file, NULL);
}

typedef struct KemuriWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ Anim anim;
    /* 0x14 */ f32 pos[3];
    /* 0x20 */ f32 scale;
    /* 0x24 */ f32 rise;
    /* 0x28 */ f32 grow;
    /* 0x2C */ s32 alpha;
    /* 0x30 */ s32 fade;
    /* 0x34 */ s32 period;
} KemuriWork;

extern u8 gDemoCamHold;
extern s32 gDemoModelsDelete;
extern Gfx yaeDemoSmokeDl[];                       /* the smoke's display list */
extern s32 yaeDemoFrame;               /* the file's frame counter */
extern u8 yaeDemoSmokeTex0[];
extern u8 yaeDemoSmokeTex1[];
extern u8 yaeDemoSmokeTex2[];
extern u8 yaeDemoSmokeTex3[];
extern u8 yaeDemoSmokeTex4[];
extern u8 yaeDemoSmokeTex6[];
extern u8 demoPieceTex0[];
extern u8 demoPieceTex1[];
extern u8 demoPieceTex2[];
extern u8 demoPieceTex3[];
extern u8 demoPieceTex4[];
extern u8 demoPieceTex7[];
extern u8 demoPieceTex15[];
extern u8 demoPieceTex19[];
extern u8 demoPieceTex78[];
extern u8 demoFaceTex0[];
extern u8 demoFaceTex1[];
extern u8 demoFaceTex2[];
/* This file's .data (0x08005760): the characters' texture animations (demotool.c's table, as
 * tlb_0020_1's; nothing reads it), then the smoke's texture animation. */
AnimFrame yaeDemoCharaAnims[] = {
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
    { (u32) demoPieceTex3, 8 },
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
    { (u32) demoPieceTex3, 7 },
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
    { (u32) demoPieceTex3, 200 },
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
AnimFrame yaeDemoSmokeAnim[] = {
    { (u32) yaeDemoSmokeTex0, 6 },
    { (u32) yaeDemoSmokeTex1, 6 },
    { (u32) yaeDemoSmokeTex2, 6 },
    { (u32) yaeDemoSmokeTex3, 6 },
    { (u32) yaeDemoSmokeTex4, 6 },
    { 0x03002D30, 8 },
    { (u32) yaeDemoSmokeTex6, 10 },
    { 0, 0 },
    { 0, 0 },
};       /* the smoke's texture animation */

s32 yaeDemoKemuriProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    KemuriWork *w = (KemuriWork *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "KEMURI";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, yaeDemoSmokeDl, 0x123);
            fadeAnimStart(&w->anim, yaeDemoSmokeAnim, 0, &w->frame->segs[3], floadGetSetFile(0x123, 0));
            w->frame->flags |= 0x20;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = ((KemuriWork *) arg)->scale;
            w->frame->pos[0] = ((KemuriWork *) arg)->pos[0];
            w->frame->pos[1] = ((KemuriWork *) arg)->pos[1];
            w->frame->pos[2] = ((KemuriWork *) arg)->pos[2];
            w->rise = ((KemuriWork *) arg)->rise;
            w->frame->flags |= 7;
            w->alpha = ((KemuriWork *) arg)->alpha;
            w->period = ((KemuriWork *) arg)->period;
            w->fade = ((KemuriWork *) arg)->fade;
            w->grow = ((KemuriWork *) arg)->grow;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = demoColorDL(255, 255, 255, 128, 128, 128, w->alpha);
            break;
        case 0:
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            if (gDemoModelsDelete == 1) {
                return -1;
            }
            if (!(yaeDemoFrame & 1)) {
                fadeAnimStep(&w->anim);
            }
            if (yaeDemoFrame % w->period == 0) {
                w->alpha -= w->fade;
            }
            if (w->alpha <= 0) {
                w->alpha = 0;
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

void yaeDemoKemuriCreate(f32 x, f32 y, f32 z, f32 scale, f32 grow, f32 rise, s32 alpha, s32 fade,
                            s32 period) {
    KemuriWork arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = scale;
    arg.rise = rise;
    arg.alpha = alpha;
    arg.fade = fade;
    arg.period = period;
    arg.grow = grow;
    tskCreate(gEffectTask, yaeDemoKemuriProc, gDmenuLastObj->file, &arg);
}

/* Every 40 frames, a puff of smoke at (14, 12, 34). */
void yaeDemoChimneySmoke(void) {
    if (yaeDemoFrame % 40 == 0) {
        yaeDemoKemuriCreate(14.0f, 12.0f, 34.0f, 0.07f, 1.0f, 0.1f, 30, 1, 4);
    }
}
