#include "tlb.h"
#include "symbol_pad.h"
#include "tlb/tlb_0024.h"

/* The file's .bss (0x080066B0) starts with Yukihime's three statics, ahead of Goemon's though
 * hers is the last function (IDO_NOTES 337, SYMBOL_PAD). */

/* A demo character's work (tlb_0029_2's DemoPlW). */
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

extern s32 a1clearFrame; /* the scene's frame count (tlb_0024_0) */
extern f32 a1clearGoeX; /* Goemon's x and z, each frame */
extern f32 a1clearGoeZ;
extern f32 a1clearTurnX; /* the x and z the demo characters turn toward */
extern f32 a1clearTurnZ;
extern Gfx a1clearShadowDl[];         /* the shadow */
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

/* "D6GOE" (Goemon, character 0, in the demo) at (326, 4, -54) under the menu's last task, the
 * twin of tlb_0030_1's genoGOE (its three statics, here in .bss); OBJMOVE's model 0. His steps
 * follow the scene's frame count (scene runs 0x8B, 0x91, 0x9B and 0x9C); at the end he turns
 * toward (-319, -802) and walks there. */
s32 a1clearGoeProc(Obj *obj, s16 msg, void *arg) {
    static s32 step;  /* .bss 0x080066BC: his demoPlTunagiAnime step */
    static f32 speed;  /* .bss 0x080066C0: unused here (genoGOE's fall speed) */
    static Player *p;  /* .bss 0x080066C4: his player */
    s32 pad[2];
    DemoPlW *w = (DemoPlW *) (obj + 1);
    PlInit init;

    DPRINTF("goecreate\n"); /* the file's rodata has it first, before the name */
    switch (msg) {
        case 2:
            obj->name = "D6GOE";
            plMakeInit(&init, 0, 0);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = 326.0f;
            init.height = 4.0f;
            init.side = -54.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            DPRINTF("歩きモーション\n");
            plAnimeSet(PL(w->pl), 0xE9);
            w->model = p->model;
            DPRINTF("*** p->model_buff=%x\n", p->model);
            DPRINTF("ゴエモン作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, a1clearShadowDl, 0xBC);
            w->shadow->scale[2] = 0.8f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->chara = 0;
            w->model->rot[1] = 3.1415927f;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 326.0f;
            w->model->pos[1] = 4.0f;
            w->model->pos[2] = -54.0f;
            w->model->rot[1] = 0.95993114f;
            demoTexAnimK0(&w->eyes, 5, &PL(w->pl)->model->segs[3]);
            demoTexAnimK0(&w->mouth, 5, &PL(w->pl)->model->segs[4]);
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
            w->shadow->pos[1] = 2.0f;
            a1clearGoeX = w->model->pos[0];
            a1clearGoeZ = w->model->pos[2];
            switch (w->state) {
                case 0:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (a1clearFrame == 0x3CA) {
                        plAnimeSet(PL(w->pl), 0xFD);
                        demoTexAnimK0(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                        w->model->pos[0] = 294.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -162.0f;
                        w->model->rot[1] = 0.95993114f;
                    }
                    if (a1clearFrame == 0x3CE) {
                        step = 0;
                        scRunEnd();
                        scRunStart(0x8B, 0, 0);
                        w->state = 2;
                    }
                    break;
                case 2:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (demoAnimeStep(w->model, 0.1f, 2)) {
                        plAnimeSet(PL(w->pl), 0xE9);
                        w->state = 4;
                        step = 0;
                        demoTexAnimK0(&w->eyes, 5, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 5, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 4:
                    if (a1clearFrame >= 0x4C9) {
                        fadeAnimStep(&w->eyes);
                        fadeAnimStep(&w->mouth);
                    }
                    if (a1clearFrame == 0x4C8) {
                        demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                    }
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (a1clearFrame == 0x590) {
                        demoTexAnimK0(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    if (a1clearFrame == 0x6B5) {
                        w->model->pos[0] = 440.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -272.0f;
                        w->model->rot[1] = 1.0297443f;
                        w->state++;
                    }
                    break;
                case 5:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (a1clearFrame == 0x960) {
                        step = 0;
                        w->state++;
                    }
                    break;
                case 6:
                    step = demoPlTunagiAnime(obj, 3, step, 2, 2, 0.3f, 0.4f, 0xEB, 0xEC);
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], a1clearTurnX,
                                                      a1clearTurnZ, w->model->rot[1], 0.3f);
                    if (step == -1) {
                        scRunEnd();
                        scRunStart(0x91, 0, 0);
                        w->state++;
                    }
                    break;
                case 7:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (a1clearFrame == 0xF84) {
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK0(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->model->pos[0] = 440.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -272.0f;
                        w->model->rot[1] = 1.0297443f;
                        w->state++;
                    }
                    break;
                case 8:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (a1clearFrame == 0x118C) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 9:
                    if ((step = demoPlTunagiAnime(obj, 3, step, 2, 2, 0.3f, 0.3f, 0xEB, 0xEC)) == -1) {
                        scRunEnd();
                        scRunStart(0x9B, 0, 0);
                        w->state++;
                    }
                    break;
                case 10:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (a1clearFrame == 0x127C) {
                        scRunEnd();
                        scRunStart(0x9C, 0, 0);
                        w->state++;
                    }
                    break;
                case 11:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (a1clearFrame >= 0x12C3) {
                        plAnimeSet(PL(w->pl), 0xDC);
                        demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    } else {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], -319.0f, -802.0f,
                                                          w->model->rot[1], 2.0f);
                    }
                    break;
                case 12:
                    demoWalkTo(w->model, 0.6f, 7.0f, -319.0f, -802.0f);
                    if (a1clearFrame == 0x131C) {
                        return -1;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *a1clearGoeCreate(Obj *obj) {
    return tskCreate(obj, a1clearGoeProc, obj->file, NULL);
}

extern f32 a1clearEbiX; /* Ebisumaru's x and z, each frame */
extern f32 a1clearEbiZ;

/* "D6EBI" (Ebisumaru, character 1, in the demo) at (201, -12, -107) under the menu's last task;
 * OBJMOVE's model 1. His steps follow the scene's frame count (scene runs 0x8D, 0x95, 0x96 and
 * 0x9A); at the end he turns toward (-319, -802) and walks there. */
s32 a1clearEbiProc(Obj *obj, s16 msg, void *arg) {
    static s32 step; /* .bss 0x080066C8: his demoPlTunagiAnime step */
    s32 pad[2];
    DemoPlW *w = (DemoPlW *) (obj + 1);
    Player *p;
    PlInit init;

    switch (msg) {
        case 2:
            obj->name = "D6EBI";
            plMakeInit(&init, 0, 1);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = 201.0f;
            init.height = -12.0f;
            init.side = -107.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            DPRINTF("歩きモーション\n");
            plAnimeSet(PL(w->pl), 0xE9);
            demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
            demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
            w->model = p->model;
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, a1clearShadowDl, 0xBC);
            w->shadow->scale[2] = 0.9f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 1;
            w->state = 0;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 201.0f;
            w->model->pos[1] = -12.0f;
            w->model->pos[2] = -107.0f;
            w->model->rot[1] = 0.9773844f;
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
            w->shadow->pos[1] = 2.0f;
            a1clearEbiX = w->model->pos[0];
            a1clearEbiZ = w->model->pos[2];
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (a1clearFrame == 0x3CA) {
                        w->model->pos[0] = 227.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -59.0f;
                        w->model->rot[1] = 0.9773844f;
                    }
                    if (a1clearFrame == 0x59A) {
                        step = 0;
                        w->state++;
                    }
                    break;
                case 1:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.2f, 0xEB, 0xEC)) == -1) {
                        w->state++;
                        step = 0;
                        scRunEnd();
                        scRunStart(0x8D, 0, 0);
                    }
                    break;
                case 2:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (a1clearFrame >= 0x5FF) {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    if (a1clearFrame == 0x6B2) {
                        scRunEnd();
                        w->state += 2;
                    }
                    break;
                case 3:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.2f, 0.2f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 4:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (a1clearFrame == 0x6B5) {
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->model->pos[0] = 353.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -149.0f;
                        w->model->rot[1] = 1.6231562f;
                        w->state++;
                    }
                    break;
                case 5:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (a1clearFrame == 0x960) {
                        step = 0;
                        w->state++;
                    }
                    break;
                case 6:
                    step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.3f, 0.4f, 0xEB, 0xEC);
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], a1clearTurnX,
                                                      a1clearTurnZ, w->model->rot[1], 0.3f);
                    if (step == -1) {
                        w->state++;
                        step++;
                    }
                    break;
                case 7:
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (!(a1clearFrame & 1)) {
                        fadeAnimStep(&w->eyes);
                    }
                    fadeAnimStep(&w->mouth);
                    if (a1clearFrame == 0xC26) {
                        plAnimeSet(PL(w->pl), 0xFE);
                        w->model->pos[0] = 131.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -140.0f;
                        w->model->rot[1] = -6.143559f;
                        scRunEnd();
                        scRunStart(0x95, 0, 0);
                        w->state++;
                    }
                    break;
                case 8:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (a1clearFrame == 0xCE4) {
                        scRunEnd();
                        scRunStart(0x96, 0, 0);
                    }
                    if (a1clearFrame >= 0xCE5) {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    if (a1clearFrame == 0xDAC) {
                        w->state++;
                    }
                    break;
                case 9:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (a1clearFrame == 0xF84) {
                        plAnimeSet(PL(w->pl), 0xE9);
                        w->model->pos[0] = 353.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -149.0f;
                        w->model->rot[1] = 1.6231562f;
                        w->state++;
                    }
                    break;
                case 10:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (a1clearFrame == 0x107E) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 11:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.3f, 0.3f, 0xEB, 0xEC)) == -1) {
                        scRunEnd();
                        scRunStart(0x9A, 0, 0);
                        w->state++;
                    }
                    break;
                case 12:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (a1clearFrame == 0x1182) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 13:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.3f, 0.2f, 0xEB, 0xE9)) == -1) {
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 14:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (a1clearFrame >= 0x127D) {
                        w->state++;
                    }
                    break;
                case 15:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (a1clearFrame >= 0x12F5) {
                        plAnimeSet(PL(w->pl), 0xDC);
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    } else {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], -319.0f, -802.0f,
                                                          w->model->rot[1], 2.0f);
                    }
                    break;
                case 16:
                    demoWalkTo(w->model, 0.6f, 4.2f, -319.0f, -802.0f);
                    if (a1clearFrame == 0x131C) {
                        return -1;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *a1clearEbiCreate(Obj *obj) {
    return tskCreate(obj, a1clearEbiProc, obj->file, NULL);
}

extern f32 a1clearSasX; /* Sasuke's x and z, each frame */
extern f32 a1clearSasZ;

/* "chaSAS" (Sasuke, character 2, in the demo) at (391, 0, -138) under the menu's last task;
 * OBJMOVE's model 2 moves him. His steps follow the scene's frame count (scene runs 0x8C and
 * 0x99); at the end he turns toward (-319, -802) and walks there, and at 0x131C he is gone. */
s32 a1clearSasProc(Obj *obj, s16 msg, void *arg) {
    static s32 tunagiStep; /* .bss 0x080066CC */
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
            init.place = 391.0f;
            init.height = 0.0f;
            init.side = -138.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            DPRINTF("走りモーション\n");
            plAnimeSet(PL(w->pl), 0xE9);
            w->model = p->model;
            DPRINTF("サスケ作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, a1clearShadowDl, 0xBC);
            w->shadow->scale[2] = 0.8f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 2;
            w->state = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 391.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = -138.0f;
            w->model->rot[1] = -5.6548667f;
            DPRINTF("サスケ作りました\n");
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
            a1clearSasX = w->model->pos[0];
            a1clearSasZ = w->model->pos[2];
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = 2.0f;
            switch (w->state) {
                case 0:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (a1clearFrame == 0x3CA) {
                        w->model->pos[0] = 367.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -270.0f;
                        w->model->rot[1] = -5.6548667f;
                    }
                    if (a1clearFrame == 0x464) {
                        tunagiStep = 0;
                        w->state++;
                    }
                    break;
                case 1:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 2, tunagiStep, 2, 2, 0.3f, 0.3f, 0xEB, 0xEC)) == -1) {
                        tunagiStep = 0;
                        w->state++;
                        scRunEnd();
                        scRunStart(0x8C, 0, 0);
                    }
                    break;
                case 2:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (a1clearFrame >= 0x4F1) {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    if (a1clearFrame == 0x590) {
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 3:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 0, tunagiStep, 2, 3, 0.3f, 0.3f, 0xEB, 0xE9)) == -1) {
                        tunagiStep = 0;
                        w->state++;
                    }
                    break;
                case 4:
                    demoAnimeStep(w->model, 0.3f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (a1clearFrame == 0x6B5) {
                        w->model->pos[0] = 541.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -408.0f;
                        w->model->rot[1] = -5.9341197f;
                        w->state++;
                    }
                    break;
                case 5:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (a1clearFrame == 0x960) {
                        tunagiStep = 0;
                        w->state++;
                    }
                    break;
                case 6:
                    tunagiStep = demoPlTunagiAnime(obj, 2, tunagiStep, 2, 2, 0.3f, 0.4f, 0xEB, 0xEC);
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], a1clearTurnX,
                                                      a1clearTurnZ, w->model->rot[1], 0.3f);
                    if (tunagiStep == -1) {
                        w->state++;
                        tunagiStep++;
                    }
                    break;
                case 7:
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (a1clearFrame == 0xF84) {
                        plAnimeSet(PL(w->pl), 0xE9);
                        w->model->pos[0] = 541.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -408.0f;
                        w->model->rot[1] = -5.9341197f;
                        w->state++;
                    }
                    break;
                case 8:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (a1clearFrame == 0xF8E) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 9:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 2, tunagiStep, 2, 2, 0.3f, 0.3f, 0xEB, 0xEC)) == -1) {
                        scRunEnd();
                        scRunStart(0x99, 0, 0);
                        tunagiStep = 0;
                        w->state++;
                    }
                    break;
                case 10:
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (a1clearFrame == 0x106A) {
                        w->state++;
                    }
                    break;
                case 11:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 0, tunagiStep, 2, 3, 0.3f, 0.2f, 0xEB, 0xE9)) == -1) {
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 12:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (a1clearFrame >= 0x127D) {
                        w->state++;
                    }
                    break;
                case 13:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (a1clearFrame >= 0x12E1) {
                        plAnimeSet(PL(w->pl), 0xDC);
                        w->state++;
                    } else {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], -319.0f, -802.0f,
                                                          w->model->rot[1], 2.0f);
                    }
                    break;
                case 14:
                    demoWalkTo(w->model, 0.6f, 8.4f, -319.0f, -802.0f);
                    if (a1clearFrame == 0x131C) {
                        return -1;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *a1clearSasCreate(Obj *obj) {
    return tskCreate(obj, a1clearSasProc, obj->file, NULL);
}

/* "tono"'s work. */
typedef struct TonoW {
    /* 0x00 */ AnimeFrame *model;
    /* 0x04 */ PolFrame *shadow;
    /* 0x08 */ s32 unused8;
    /* 0x0C */ s32 state;
} TonoW;

extern f32 a1clearTonoX; /* the lord's x and z, each frame */
extern f32 a1clearTonoZ;

/* The file's .data, in address order (its externs above stay as declarations). */
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
/* (a1clearFrame is tlb_0024_0's .data; this file's starts at 0x08005770 with the characters'
 * texture animations, demotool.c's table, which nothing reads) */
AnimFrame a1clearCharaAnims[] = {
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
f32 a1clearSasX = 5.0f; /* Sasuke's x and z, each frame */
f32 a1clearSasZ = 5.0f;
f32 a1clearGoeX = 5.0f; /* Goemon's x and z, each frame */
f32 a1clearGoeZ = 5.0f;
f32 a1clearEbiX = 5.0f; /* Ebisumaru's x and z, each frame */
f32 a1clearEbiZ = 5.0f;
f32 a1clearTonoX = 5.0f; /* the lord's x and z, each frame */
f32 a1clearTonoZ = 5.0f;
f32 a1clearTurnX = 5.0f; /* the x and z the demo characters turn toward */
f32 a1clearTurnZ = 5.0f;
extern u8 a1clearTonoAnim[];          /* the lord's animations (set 0xC8) */
extern u8 a1clearTonoAnim2[];
extern u8 a1clearTonoTunagiEndAnim[];
extern u8 a1clearTonoTunagiAnim[];
extern u8 a1clearTonoState9Anim[];

/* "tono": the lord (set 0xC8) at (738, 0, -24); OBJMOVE's model 4, his shadow under him. He
 * plays his animations on the scene's frame count, linking idle and talk (tunagi), and starts
 * scene runs 0x87, 0x88, 0x8E and 0x9D.
 * His tunagi step is a static, reached by a fresh lui at every use (IDO_NOTES 38). */
s32 a1clearTonoProc(Obj *obj, s16 msg, void *arg) {
    static s32 reserved[2]; /* .bss 0x080066D0: two words of the game's .bss that nothing reads */
    static s32 tunagiStep;  /* .bss 0x080066D8 */
    TonoW *w = (TonoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "tono";
            DPRINTF("TONOstep-->%d\n", w->state);
            DPRINTF("とのさま作ります\n");
            w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->model == NULL) {
                return -1;
            }
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoAnimeSet(w->model, (u32) a1clearTonoAnim, 0xC8, 0.0f);
            demoPolSetDL(w->shadow, a1clearShadowDl, 0xBC);
            w->state = 0;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 738.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = -24.0f;
            w->model->rot[1] = -1.850049f;
            w->shadow->scale[2] = 1.1f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = w->model->pos[1] + 1.0f;
            DPRINTF("とのさま作りました\n");
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
            w->shadow->pos[1] = w->model->pos[1] + 1.0f;
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (a1clearFrame == 0xE6) {
                        scRunStart(0x87, 0, 0);
                    }
                    if (a1clearFrame == 0x1AE) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 1:
                    if ((tunagiStep = tunagi_anime(w->model, tunagiStep, 2, 2, 0.1f, 0.1f, 0xC8, (u32) a1clearTonoTunagiAnim, 0xC8,
                                             (u32) a1clearTonoTunagiEndAnim)) == -1) {
                        scRunEnd();
                        scRunStart(0x88, 0, 0);
                        w->state++;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (a1clearFrame == 0x2F8) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 3:
                    if ((tunagiStep = tunagi_anime(w->model, tunagiStep, 2, 3, 0.1f, 0.1f, 0xC8, (u32) a1clearTonoTunagiAnim, 0xC8,
                                             (u32) a1clearTonoAnim2)) == -1) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 4:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (a1clearFrame == 0x6B5) {
                        w->model->pos[0] = 650.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -12.0f;
                        w->model->rot[1] = -1.6929694f;
                        w->state = 6;
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (a1clearFrame == 0x6C9) {
                        w->state++;
                        demoAnimeSet(w->model, (u32) a1clearTonoAnim2, 0xC8, 0.0f);
                    }
                    break;
                case 6:
                    if ((tunagiStep = tunagi_anime(w->model, tunagiStep, 2, 2, 0.1f, 0.1f, 0xC8, (u32) a1clearTonoTunagiAnim, 0xC8,
                                             (u32) a1clearTonoTunagiEndAnim)) == -1) {
                        scRunEnd();
                        scRunStart(0x8E, 0, 0);
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 7:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (a1clearFrame == 0x7B9) {
                        w->state++;
                    }
                    break;
                case 8:
                    if ((tunagiStep = tunagi_anime(w->model, tunagiStep, 2, 3, 0.1f, 0.1f, 0xC8, (u32) a1clearTonoTunagiAnim, 0xC8,
                                             (u32) a1clearTonoAnim2)) == -1) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 9:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (a1clearFrame == 0xA1E) {
                        w->model->pos[0] = 680.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -40.0f;
                        w->model->rot[1] = -2.0420353f;
                        demoAnimeSet(w->model, (u32) a1clearTonoState9Anim, 0xC8, 0.0f);
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 10:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (a1clearFrame == 0xF84) {
                        demoAnimeSet(w->model, (u32) a1clearTonoAnim2, 0xC8, 0.0f);
                        w->model->pos[0] = 650.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -12.0f;
                        w->model->rot[1] = -1.6929694f;
                        w->state++;
                    }
                    break;
                case 11:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (a1clearFrame == 0x131C) {
                        tunagiStep = 0;
                        w->state++;
                    }
                    break;
                case 12:
                    if ((tunagiStep = tunagi_anime(w->model, tunagiStep, 2, 2, 0.1f, 0.1f, 0xC8, (u32) a1clearTonoTunagiAnim, 0xC8,
                                             (u32) a1clearTonoTunagiEndAnim)) == -1) {
                        scRunEnd();
                        scRunStart(0x9D, 0, 0);
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 13:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (a1clearFrame == 0x1448) {
                        w->state++;
                    }
                    break;
                case 14:
                    if ((tunagiStep = tunagi_anime(w->model, tunagiStep, 2, 3, 0.1f, 0.1f, 0xC8, (u32) a1clearTonoTunagiAnim, 0xC8,
                                             (u32) a1clearTonoAnim2)) == -1) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
            }
            a1clearTonoX = w->model->pos[0];
            a1clearTonoZ = w->model->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 a1clearTonoProc(Obj *obj, s16 msg, void *arg);

Obj *a1clearTonoCreate(Obj *obj) {
    return tskCreate(obj, a1clearTonoProc, obj->file, NULL);
}

extern u8 a1clearYukiAnim[]; /* Yukihime's animations (set 0xC8) */
extern u8 a1clearYukiState1Anim[];
extern u8 a1clearYukiTunagiAnim[];
extern u8 a1clearYukiAnim2[];
extern u8 a1clearYukiState11Anim[];
extern u8 a1clearYukiState12Anim[];

/* "yuki" (Yukihime, set 0xC8, TonoW) at (762, 0, -152); OBJMOVE's model 3, her shadow under
 * her. She plays her animations on the scene's frame count (scene runs 0x89 to 0x98), turning
 * toward Ebisumaru and then Goemon; her x and z are the point the others turn toward.
 * Her statics are the file's first .bss (0x080066B0), ahead of Goemon's though hers is the
 * last function: their numbers wrap past 256 (IDO_NOTES 337, SYMBOL_PAD). */
s32 a1clearYukiProc(Obj *obj, s16 msg, void *arg) {
    static s32 reserved;   /* .bss 0x080066B0: a word nothing reads */
    static s32 tunagiStep; /* .bss 0x080066B4 */
    static s32 reserved2;  /* .bss 0x080066B8: a word nothing reads */
    TonoW *w = (TonoW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "yuki";
            DPRINTF("ゆきひめ作ります\n");
            w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->model == NULL) {
                return -1;
            }
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoAnimeSet(w->model, (u32) a1clearYukiAnim, 0xC8, 0.0f);
            demoPolSetDL(w->shadow, a1clearShadowDl, 0xBC);
            w->state = 0;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 762.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = -152.0f;
            w->model->rot[1] = -1.4486233f;
            w->shadow->scale[2] = 0.8f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            DPRINTF("ゆきひめ作りました\n");
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
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = w->model->pos[1] + 1.0f;
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (a1clearFrame == 0x2DA) {
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 1:
                    if (demoAnimeStep(w->model, 0.1f, 2)) {
                        demoAnimeSet(w->model, (u32) a1clearYukiState1Anim, 0xC8, 0.0f);
                        scRunEnd();
                        scRunStart(0x89, 0, 0);
                        tunagiStep = 0;
                        w->state++;
                    }
                    break;
                case 2:
                    if (demoAnimeStep(w->model, 0.15f, 2)) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0x8A, 0, 0);
                        demoAnimeSet(w->model, (u32) a1clearYukiTunagiAnim, 0xC8, 0.0f);
                    }
                    break;
                case 3:
                    if (demoAnimeStep(w->model, 0.18f, 2)) {
                        demoAnimeSet(w->model, (u32) a1clearYukiAnim2, 0xC8, 0.0f);
                        tunagiStep = 0;
                        w->state++;
                    }
                    break;
                case 4:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (a1clearFrame == 0x3CA) {
                        scRunEnd();
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 5:
                    if ((tunagiStep = tunagi_anime(w->model, tunagiStep, 2, 3, 0.2f, 0.2f, 0xC8, (u32) a1clearYukiTunagiAnim, 0xC8,
                                                   (u32) a1clearYukiAnim)) == -1) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 6:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (a1clearFrame == 0x6B5) {
                        w->model->pos[0] = 700.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -232.0f;
                        w->model->rot[1] = -1.0471976f;
                        w->state++;
                    }
                    break;
                case 7:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (a1clearFrame == 0x7CD) {
                        w->state++;
                    }
                    break;
                case 8:
                    if ((tunagiStep = tunagi_anime(w->model, tunagiStep, 2, 2, 0.1f, 0.1f, 0xC8, (u32) a1clearYukiTunagiAnim, 0xC8,
                                                   (u32) a1clearYukiAnim2)) == -1) {
                        w->state++;
                        tunagiStep = 0;
                        scRunEnd();
                        scRunStart(0x8F, 0, 0);
                    }
                    break;
                case 9:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (a1clearFrame == 0x8C7) {
                        scRunEnd();
                        scRunStart(0x90, 0, 0);
                    }
                    if (a1clearFrame < 0x8C7) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], a1clearEbiX,
                                                          a1clearEbiZ, w->model->rot[1], 0.1f);
                    } else {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], a1clearGoeX,
                                                          a1clearGoeZ, w->model->rot[1], 0.1f);
                    }
                    if (a1clearFrame == 0x95D) {
                        w->state++;
                    }
                    break;
                case 10:
                    if ((tunagiStep = tunagi_anime(w->model, tunagiStep, 2, 3, 0.1f, 0.1f, 0xC8, (u32) a1clearYukiTunagiAnim, 0xC8,
                                                   (u32) a1clearYukiAnim)) == -1) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 11:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (a1clearFrame == 0xA1E) {
                        w->model->pos[0] = 700.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -166.0f;
                        w->model->rot[1] = 4.6425757f;
                        demoAnimeSet(w->model, (u32) a1clearYukiState11Anim, 0xC8, 0.0f);
                        w->state++;
                    }
                    break;
                case 12:
                    if (demoAnimeStep(w->model, 0.1f, 2)) {
                        demoAnimeSet(w->model, (u32) a1clearYukiState12Anim, 0xC8, 0.0f);
                        scRunEnd();
                        scRunStart(0x92, 0, 0);
                        w->state++;
                    }
                    break;
                case 13:
                    demoAnimeStep(w->model, 0.1f, 0);
                    switch (a1clearFrame) {
                        case 0xB04:
                            scRunEnd();
                            scRunStart(0x93, 0, 0);
                            break;
                        case 0xBCC:
                            scRunEnd();
                            scRunStart(0x94, 0, 0);
                            break;
                        case 0xC26:
                            w->state++;
                            break;
                    }
                    break;
                case 14:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (a1clearFrame == 0xDAE) {
                        demoAnimeSet(w->model, (u32) a1clearYukiAnim2, 0xC8, 0.0f);
                        scRunEnd();
                        scRunStart(0x97, 0, 0);
                        w->state++;
                    }
                    break;
                case 15:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (a1clearFrame == 0xE80) {
                        scRunEnd();
                        scRunStart(0x98, 0, 0);
                    }
                    if (a1clearFrame < 0xE80) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], a1clearEbiX,
                                                          a1clearEbiZ, w->model->rot[1], 0.2f);
                    } else {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], a1clearGoeX,
                                                          a1clearGoeZ, w->model->rot[1], 0.2f);
                    }
                    if (a1clearFrame == 0xF7A) {
                        tunagiStep = 0;
                        w->state++;
                        scRunEnd();
                    }
                    break;
                case 16:
                    if ((tunagiStep = tunagi_anime(w->model, tunagiStep, 2, 3, 0.1f, 0.1f, 0xC8, (u32) a1clearYukiTunagiAnim, 0xC8,
                                                   (u32) a1clearYukiAnim)) == -1) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 17:
                    demoAnimeStep(w->model, 0.1f, 0);
                    break;
            }
            a1clearTurnX = w->model->pos[0];
            a1clearTurnZ = w->model->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *a1clearYukiCreate(Obj *obj) {
    return tskCreate(obj, a1clearYukiProc, obj->file, NULL);
}
