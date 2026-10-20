#include "tlb.h"
#include "symbol_pad.h"
#include "tlb/tlb_0022.h"


/* The file's .bss (0x08006A90 on):
 * - jiji's statics come first, though his is the last function: their symbol numbers wrap
 *   past 256 (IDO_NOTES 337, SYMBOL_PAD);
 * - then the file's globals, in address order;
 * - then the characters' statics, in function order. */
f32 jijiDemoSasX; /* Sasuke's x and z, each frame */
f32 jijiDemoSasZ;
f32 jijiDemoGoeX; /* Goemon's x and z, each frame */
f32 jijiDemoGoeZ;
f32 jijiDemoEbiX; /* Ebisumaru's x and z, each frame */
f32 jijiDemoEbiZ;
f32 jijiDemoYaeX; /* Yae's x and z, each frame */
f32 jijiDemoYaeZ;
f32 jijiDemoJijiX; /* jiji's x and z, each frame */
f32 jijiDemoJijiZ;
f32 jijiDemoTurnX; /* a point to turn toward (where Sasuke's first run ended, ...) */
f32 jijiDemoTurnZ;

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

#ifndef DPRINTF
#define DPRINTF
#endif

extern s32 jijiDemoFrame; /* the scene's frame count (tlb_0022_0's data) */
extern Gfx jijiDemoShadowDl[]; /* the shadow */
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

/* "genoGOE" (Goemon, character 0) at (42, 0, 105) under the menu's last task, the twin of
 * tlb_0030_1's (step, speed and player as statics, here in .bss); OBJMOVE's model 0, his shadow
 * at y 0.3 under him. At frame 0x1D6 he stands at (1, 0, -14) and walks on; then he links his
 * actions on the scene's frame count (scene runs 0xCC, 0xD9, 0xE1 to 0xE3), his walk speed
 * changing with the talk in step 25. Steps 1-10 are unused (the jump table's gaps), and step 11
 * runs on into step 12. His "goecreate" print is the file's first string. */
s32 jijiDemoGoeProc(Obj *obj, s16 msg, void *arg) {
    static s32 step;  /* .bss 0x08006ACC: his demoPlTunagiAnime step */
    static f32 speed; /* .bss 0x08006AD0: his walk speed in step 25 */
    static Player *p; /* .bss 0x08006AD4: his player */
    s32 pad[2];
    DemoPlW *w = (DemoPlW *) (obj + 1);
    PlInit init;

    DPRINTF("goecreate\n");
    switch (msg) {
        case 2:
            obj->name = "genoGOE";
            plMakeInit(&init, 0, 0);
            init.flags |= 0x90000040;
            init.flags2 |= 8;
            init.zone = 0;
            init.place = 42.0f;
            init.height = 0.0f;
            init.side = 105.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            DPRINTF("歩きモーション\n");
            plAnimeSet(PL(w->pl), 0xDA);
            w->model = p->model;
            DPRINTF("*** p->model_buff=%x\n", p->model);
            ((Frame *) p->shadow)->flags &= ~2;
            DPRINTF("ゴエモン作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, jijiDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.08f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->chara = 0;
            w->model->rot[1] = 3.1415927f;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 42.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = 105.0f;
            w->model->rot[1] = 3.1415927f;
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
            jijiDemoGoeX = w->model->pos[0];
            jijiDemoGoeZ = w->model->pos[2];
            switch (w->state) {
                case 0:
                    if (jijiDemoFrame == 0x1D6) {
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK0(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->model->scale[2] = 0.097f;
                        w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                        w->model->pos[0] = 1.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -14.0f;
                        w->model->rot[1] = 4.5029497f;
                        w->state = 12;
                    }
                    break;
                case 11:
                    if (jijiDemoFrame >= 0xC9) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], jijiDemoJijiX,
                                                          jijiDemoJijiZ, w->model->rot[1], 1.0f);
                    } else if (w->model->rot[1] < 3.822271167849737) {
                        w->model->rot[1] += 0.006981317201552032;
                    }
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (jijiDemoFrame == 0x28) {
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK0(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->model->scale[2] = 0.097f;
                        w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                        w->model->pos[0] = 1.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -14.0f;
                        w->model->rot[1] = 4.5029497f;
                        w->state = 2;
                    }
                    /* fallthrough */
                case 12:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (jijiDemoFrame == 0x1E0) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 13:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.4f, 0.4f, 0xEB, 0xEC)) == -1) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0xCC, 0, 0);
                        step = 0;
                    }
                    break;
                case 14:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (jijiDemoFrame == 0x2D0) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 15:
                    if ((step = demoPlTunagiAnime(obj, 5, step, 2, 3, 0.2f, 0.2f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                    }
                    break;
                case 16:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (jijiDemoFrame == 0xA32) {
                        plAnimeSet(PL(w->pl), 0x34);
                        demoTexAnimK0(&w->eyes, 5, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 5, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 17:
                    if (demoAnimeStep(w->model, 0.1f, 2)) {
                        w->state += 2;
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK0(&w->eyes, 5, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 5, &PL(w->pl)->model->segs[4]);
                    }
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], jijiDemoEbiX,
                                                      jijiDemoEbiZ, w->model->rot[1], 2.0f);
                    break;
                case 18:
                    if (demoAnimeStep(w->model, 0.04f, 3)) {
                        w->state++;
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK0(&w->eyes, 5, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 5, &PL(w->pl)->model->segs[4]);
                    }
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], jijiDemoJijiX,
                                                      jijiDemoJijiZ, w->model->rot[1], 1.0f);
                    break;
                case 19:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], jijiDemoJijiX,
                                                      jijiDemoJijiZ, w->model->rot[1], 1.0f);
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (jijiDemoFrame == 0xF64) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 20:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.4f, 0.4f, 0xEB, 0xEC)) == -1) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0xD9, 0, 0);
                        step = 0;
                    }
                    break;
                case 21:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.6f, 0);
                    if (jijiDemoFrame == 0x1090) {
                        w->state++;
                        step = 0;
                        scRunEnd();
                    }
                    break;
                case 22:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.2f, 0.2f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 23:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (jijiDemoFrame == 0x1982) {
                        w->state++;
                        demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 24:
                    if ((step = demoPlTunagiAnime(obj, 3, step, 2, 2, 0.4f, 0.4f, 0xEB, 0xEC)) == -1) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0xE1, 0, 0);
                        step = 0;
                        speed = 0.4f;
                    }
                    break;
                case 25:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, speed, 0);
                    switch (jijiDemoFrame) {
                        case 0x1AAE:
                            speed = 0.2f;
                            scRunEnd();
                            scRunStart(0xE2, 0, 0);
                            break;
                        case 0x1B30:
                            speed = 0.1f;
                            demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                            break;
                        case 0x1B94:
                            speed = 0.5f;
                            demoTexAnimK0(&w->eyes, 3, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK0(&w->mouth, 3, &PL(w->pl)->model->segs[4]);
                            scRunEnd();
                            scRunStart(0xE3, 0, 0);
                            break;
                        case 0x1C5C:
                            w->state++;
                            demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                            step = 0;
                            break;
                    }
                    break;
                case 26:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.2f, 0.2f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                    }
                    break;
                case 27:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.3f, 0);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *jijiDemoGoeCreate(Obj *obj) {
    return tskCreate(obj, jijiDemoGoeProc, obj->file, NULL);
}

/* "genoGOE" (the game's name; it is Ebisumaru, character 1, who prints "EBI STEP") at
 * (21, 0, 99) under the menu's last task; OBJMOVE's model 1. At frame 0x1D6 he stands at
 * (-2, 0, 1); he links his actions (scene runs 0xCD, 0xD3, 0xD4), and at 0xA50 jumps in a blue
 * sparkle (sound 0x22C, action 0x104), bouncing three times, then walks back to (-2, 1) and
 * turns toward the others. Steps 1-10 are unused (the jump table's gaps). Case 19's turn passes
 * the double 1.0 (the game's one mtc1 f0 and mfc1 a3, IDO_NOTES 326). */
s32 jijiDemoEbiProc(Obj *obj, s16 msg, void *arg);
s32 jijiDemoEbiProc(Obj *obj, s16 msg, void *arg) {
    static s32 step; /* .bss 0x08006AD8: his demoPlTunagiAnime step, then his bounce count */
    static f32 vx;   /* .bss 0x08006ADC: his jump's velocity */
    static f32 vy;
    static f32 vz;
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
            init.place = -2.0f;
            init.height = 0.0f;
            init.side = 1.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            DPRINTF("歩きモーション\n");
            plAnimeSet(PL(w->pl), 0xDA);
            DPRINTF("EBI STEP --->%d\n", w->state);
            w->model = p->model;
            ((Frame *) p->shadow)->flags &= ~2;
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, jijiDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.09f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 1;
            w->state = 0;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 21.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = 99.0f;
            w->model->rot[1] = 2.6529005f;
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
            jijiDemoEbiX = w->model->pos[0];
            jijiDemoEbiZ = w->model->pos[2];
            switch (w->state) {
                case 0:
                    if (jijiDemoFrame == 0x1D6) {
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->model->pos[0] = -2.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 1.0f;
                        w->model->rot[1] = 4.171337f;
                        scRunEnd();
                        w->state = 12;
                    }
                    break;
                case 11:
                    if (jijiDemoFrame >= 0xC9) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], jijiDemoJijiX,
                                                          jijiDemoJijiZ, w->model->rot[1], 1.0f);
                    } else if (w->model->rot[1] < 3.6651915308148166) {
                        w->model->rot[1] += 0.01745329300388008;
                    }
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (jijiDemoFrame == 0x28) {
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->model->pos[0] = -2.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 1.0f;
                        w->model->rot[1] = 4.171337f;
                        w->state = 2;
                    }
                    break;
                case 12:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (jijiDemoFrame == 0x2D0) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 13:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.2f, 0xEB, 0xEC)) == -1) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0xCD, 0, 0);
                        step = 0;
                    }
                    break;
                case 14:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (jijiDemoFrame == 0x398) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 15:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.2f, 0.2f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                    }
                    break;
                case 16:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (jijiDemoFrame == 0x8A2) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 17:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.2f, 0xEB, 0xEC)) == -1) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0xD3, 0, 0);
                        step = 0;
                        jijiDemoTurnX = jijiDemoEbiX;
                        jijiDemoTurnZ = jijiDemoEbiZ;
                    }
                    break;
                case 18:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.6f, 0);
                    if (jijiDemoFrame == 0x9A6) {
                        scRunEnd();
                        scRunStart(0xD4, 0, 0);
                    }
                    if (jijiDemoFrame == 0xA50) {
                        itexSparkleBlue(-4.0f, 14.0f, 0.0f, 0.02f);
                        sndPlay(0x22C);
                        plAnimeSet(PL(w->pl), 0x104);
                        demoTexAnimK1(&w->eyes, 6, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 6, &PL(w->pl)->model->segs[4]);
                        vx = 0.0f;
                        vy = 1.0f;
                        vz = 1.0f;
                        w->state++;
                        step = 0;
                        scRunEnd();
                    }
                    break;
                case 19:
                    demoAnimeStep(w->model, 0.3f, 2);
                    vx *= 0.97;
                    vy -= 0.1;
                    vz *= 0.97;
                    w->model->pos[0] += vx;
                    w->model->pos[1] += vy;
                    w->model->pos[2] += vz;
                    if (w->model->pos[1] < 0.0f) {
                        w->model->pos[1] = 0.1f;
                        step++;
                        vy = 1.0f - step * 0.25f;
                    }
                    if (step == 3) {
                        plAnimeSet(PL(w->pl), 0xDA);
                        w->state++;
                    }
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], -2.0f, 1.0,
                                                      w->model->rot[1], 1.0);
                    break;
                case 20:
                    if (step++ == 30) {
                        w->state++;
                    }
                    break;
                case 21:
                    if (demoWalkTo(w->model, 0.2f, 0.3f, -2.0f, 1.0f)) {
                        w->state++;
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 22:
                    demoAnimeStep(w->model, 0.2f, 0);
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], jijiDemoJijiX,
                                                      jijiDemoJijiZ, w->model->rot[1], 1.0f);
                    if (jijiDemoFrame == 0x1B94) {
                        w->state++;
                    }
                    break;
                case 23:
                    demoAnimeStep(w->model, 0.2f, 0);
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], jijiDemoGoeX,
                                                      jijiDemoGoeZ, w->model->rot[1], 0.3f);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *jijiDemoEbiCreate(Obj *obj) {
    return tskCreate(obj, jijiDemoEbiProc, obj->file, NULL);
}

/* "chaSAS" (Sasuke, character 2, in this demo): OBJMOVE's model 2 moves him, and his shadow
 * follows him. At frame 470 he is put at (24, 0, -20) and walks to (1, -27); then he stands
 * turning toward a point until 2010, runs (actions 0xEB/0xEC) and, when the run ends, the
 * running scene ends and scene 0xD2 starts; at 2210 he runs back (0xEB/0xE9), and from 7060 he
 * turns toward another point. The runs' step is a static of this function (0x08006AE8). */
s32 jijiDemoSasProc(Obj *obj, s16 msg, void *arg) {
    static s32 step;     /* .bss 0x08006AE8 */
    static s32 reserved; /* .bss 0x08006AEC: a word of the game's .bss that nothing reads */
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
            init.place = 1.0f;
            init.height = 0.0f;
            init.side = -27.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            DPRINTF("走りモーション\n");
            plAnimeSet(PL(w->pl), 0xDA);
            DPRINTF("立ち止まります\n");
            w->model = p->model;
            ((Frame *) p->shadow)->flags &= ~2;
            DPRINTF("サスケ作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, jijiDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.065f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 2;
            w->state = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 24.0f;
            w->model->pos[1] = -200.0f; /* below the floor until frame 470 */
            w->model->pos[2] = -20.0f;
            w->model->rot[1] = -2.0943952f;
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
            jijiDemoSasX = w->model->pos[0];
            jijiDemoSasZ = w->model->pos[2];
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = 0.3f;
            switch (w->state) {
                case 0:
                    if (jijiDemoFrame == 470) {
                        w->model->pos[0] = 24.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -20.0f;
                        w->model->rot[1] = -2.0943952f;
                        w->state++;
                    }
                    break;
                case 1:
                    if (demoWalkTo(w->model, 0.2f, 0.2f, 1.0f, -27.0f)) {
                        w->state = 11;
                        plAnimeSet(PL(w->pl), 0xD7);
                    }
                    break;
                case 11:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], jijiDemoJijiX,
                                                      jijiDemoJijiZ, w->model->rot[1], 2.0f);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (jijiDemoFrame == 2010) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 12:
                    step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.3f, 0.4f, 0xEB, 0xEC);
                    if (step == -1) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0xD2, 0, 0);
                        step = 0;
                        jijiDemoTurnX = jijiDemoSasX;
                        jijiDemoTurnZ = jijiDemoSasZ;
                    }
                    break;
                case 13:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.6f, 0);
                    if (jijiDemoFrame == 2210) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 14:
                    step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.2f, 0.2f, 0xEB, 0xE9);
                    if (step == -1) {
                        w->state++;
                    }
                    break;
                case 15:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (jijiDemoFrame == 7060) {
                        w->state++;
                    }
                    break;
                case 16:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], jijiDemoGoeX,
                                                      jijiDemoGoeZ, w->model->rot[1], 0.5f);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *jijiDemoSasCreate(Obj *obj) {
    return tskCreate(obj, jijiDemoSasProc, obj->file, NULL);
}

/* "chaYAE" (Yae, character 3, in this demo), the twin of tlb_0029_2's: OBJMOVE's model 3 moves
 * her, and her shadow follows her. At frame 470 she is put at (-8, 1, 12) and walks to
 * (-18, 2); then she stands turning toward a point, with faces 8 at 2510 and 0 at 3940; at
 * 5050 action 0x101 (faces 2) and scene 0xDC; her animation slows at 5280 and 5330 (faces 7),
 * faces 6 and scene 0xDD at 5380, she stands again at 5620, and from 7060 she turns toward
 * another point. Her step-12 animation speed is a static of this function (0x08006AF8, this
 * file's .bss, reached by a fresh lui at each use). */
s32 jijiDemoYaeProc(Obj *obj, s16 msg, void *arg) {
    static s32 reserved[2]; /* the game's .bss has 8 bytes here (0x08006AF0) that nothing reads */
    static f32 animSpeed;
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
            init.place = -18.0f;
            init.height = 1.0f;
            init.side = 2.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            DPRINTF("立ちモーション\n");
            plAnimeSet(PL(w->pl), 0xDA);
            DPRINTF("立ち止まります\n");
            w->model = p->model;
            ((Frame *) p->shadow)->flags &= ~2;
            DPRINTF("ヤエ作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, jijiDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.065f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 3;
            w->state = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -8.0f;
            w->model->pos[1] = -199.0f; /* below the floor until frame 470 */
            w->model->pos[2] = 12.0f;
            w->model->rot[1] = -1.989675403f;
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
            jijiDemoYaeX = w->model->pos[0];
            jijiDemoYaeZ = w->model->pos[2];
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = 0.3f;
            switch (w->state) {
                case 0:
                    if (jijiDemoFrame == 470) {
                        w->state++;
                        w->model->pos[0] = -8.0f;
                        w->model->pos[1] = 1.0f;
                        w->model->pos[2] = 12.0f;
                        w->model->rot[1] = -1.989675403f;
                    }
                    break;
                case 1:
                    if (demoWalkTo(w->model, 0.2f, 0.2f, -18.0f, 2.0f)) {
                        w->state = 11;
                        plAnimeSet(PL(w->pl), 0xD7);
                    }
                    break;
                case 11:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], jijiDemoJijiX,
                                                      jijiDemoJijiZ,
                                                      w->model->rot[1], 0.7f);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (jijiDemoFrame == 2510) {
                        demoTexAnimK3(&w->eyes, 8, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 8, &PL(w->pl)->model->segs[4]);
                    }
                    if (jijiDemoFrame == 3940) {
                        demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    if (jijiDemoFrame == 5050) {
                        w->state++;
                        plAnimeSet(PL(w->pl), 0x101);
                        demoTexAnimK3(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                        animSpeed = 0.4f;
                        scRunEnd();
                        scRunStart(0xDC, 0, 0);
                    }
                    break;
                case 12:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, animSpeed, 0);
                    switch (jijiDemoFrame) {
                        case 5280:
                            animSpeed = 0.04f;
                            demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                            break;
                        case 5330:
                            animSpeed = 0.1f;
                            demoTexAnimK3(&w->eyes, 7, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK3(&w->mouth, 7, &PL(w->pl)->model->segs[4]);
                            break;
                        case 5380:
                            demoTexAnimK3(&w->eyes, 6, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK3(&w->mouth, 6, &PL(w->pl)->model->segs[4]);
                            scRunEnd();
                            scRunStart(0xDD, 0, 0);
                            break;
                        case 5620:
                            plAnimeSet(PL(w->pl), 0xD7);
                            demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                            w->state++;
                            break;
                    }
                    break;
                case 13:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (jijiDemoFrame == 7060) {
                        w->state++;
                    }
                    break;
                case 14:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], jijiDemoGoeX,
                                                      jijiDemoGoeZ,
                                                      w->model->rot[1], 0.4f);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *jijiDemoYaeCreate(Obj *obj) {
    return tskCreate(obj, jijiDemoYaeProc, obj->file, NULL);
}

/* "jiji"'s work (tlb_0021_2's JijiW). */
typedef struct JijiW {
    /* 0x00 */ AnimeFrame *model;
    /* 0x04 */ PolFrame *shadow;
    /* 0x08 */ s32 timer;
    /* 0x0C */ s32 state;
} JijiW;

extern u8 jijiDemoJijiAnim[]; /* the old man's animations (set 0xC5) */
extern u8 jijiDemoJijiTunagiAnim[];
extern u8 jijiDemoJijiState1Anim[];
extern u8 jijiDemoJijiState3Anim[];
extern u8 jijiDemoJijiState2Anim[];

/* "jiji": the wise old man (set 0xC5) at (-23, 0, -15); OBJMOVE's model 4. 22 steps on the
 * scene's frame count: he links idle and talk with tunagi_anime and turns toward Ebisumaru,
 * Goemon, Yae or the points between them as each speaks (scene runs 0xCA-0xE0). His x and z
 * are where the others turn in their step 11.
 * His statics (0x08006A90-0x08006A98) are the file's first .bss, though his is its last
 * function: their numbers wrap (IDO_NOTES 337, as yuki and WATER). */
s32 jijiDemoJijiProc(Obj *obj, s16 msg, void *arg) {
    static s32 alpha; /* .bss 0x08006A90: his shadow's alpha (never set: 0) */
    static s32 step;  /* .bss 0x08006A94: his tunagi_anime step */
    static f32 speed; /* .bss 0x08006A98: his animation speed */
    s32 pad[2];
    JijiW *w = (JijiW *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "jiji";
            DPRINTF("物知りじいさん作ります\n");
            w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->model == NULL) {
                return -1;
            }
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoAnimeSet(w->model, (u32) jijiDemoJijiAnim, 0xC5, 0.0f);
            demoPolSetDL(w->shadow, jijiDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.08f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -23.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = -15.0f;
            w->model->rot[1] = -5.113815f;
            DPRINTF("物知りじいさん作りました\n");
            speed = 0.1f;
            break;
        case 1:
            frmDestroy(&w->model->f);
            frmDestroy(&w->shadow->f);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, alpha);
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
                    demoAnimeStep(w->model, speed, 0);
                    if (jijiDemoFrame == 0x28) {
                        speed = 0.4f;
                        scRunEnd();
                        scRunStart(0xCA, 0, 0);
                    }
                    if (jijiDemoFrame == 0x8C) {
                        speed = 0.1f;
                    }
                    if (jijiDemoFrame == 0xF0) {
                        speed = 0.3f;
                        scRunEnd();
                        scRunStart(0xCB, 0, 0);
                        w->state = 1;
                    }
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (jijiDemoFrame == 0x398) {
                        w->state++;
                    }
                    break;
                case 2:
                    if (demoAnimeStep(w->model, 0.2f, 2)) {
                        demoAnimeSet(w->model, (u32) jijiDemoJijiState2Anim, 0xC5, 0.0f);
                        scRunEnd();
                        scRunStart(0xCE, 0, 0);
                        w->state++;
                    }
                    break;
                case 3:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], jijiDemoEbiX,
                                                      jijiDemoEbiZ, w->model->rot[1], 1.0f);
                    if (demoAnimeStep(w->model, 0.07f, 2)) {
                        w->state++;
                        step = 0;
                        scRunEnd();
                        scRunStart(0xCF, 0, 0);
                        demoAnimeSet(w->model, (u32) jijiDemoJijiState3Anim, 0xC5, 0.0f);
                    }
                    break;
                case 4:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2],
                                                      (jijiDemoGoeX + jijiDemoEbiX) / 2,
                                                      (jijiDemoGoeX + jijiDemoEbiZ) / 2,
                                                      w->model->rot[1], 1.0f);
                    if (demoAnimeStep(w->model, 0.1f, 2)) {
                        w->state++;
                        step = 0;
                        scRunEnd();
                        scRunStart(0xD0, 0, 0);
                        demoAnimeSet(w->model, (u32) jijiDemoJijiState1Anim, 0xC5, 0.0f);
                        speed = 0.3f;
                        step = 0;
                    }
                    break;
                case 5:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], jijiDemoGoeX,
                                                      jijiDemoGoeZ, w->model->rot[1], 1.0f);
                    demoAnimeStep(w->model, speed, 0);
                    if (jijiDemoFrame == 0x6AE) {
                        scRunEnd();
                        scRunStart(0xD1, 0, 0);
                        speed = 0.5f;
                    }
                    if (jijiDemoFrame >= 0x7DB) {
                        w->state = 8;
                    }
                    break;
                case 8:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2],
                                                      (jijiDemoEbiX + jijiDemoGoeX) / 2,
                                                      (jijiDemoEbiZ + jijiDemoGoeZ) / 2,
                                                      w->model->rot[1], 1.0f);
                    if ((step = tunagi_anime(w->model, step, 2, 3, 0.2f, 0.2f, 0xC5, (u32) jijiDemoJijiTunagiAnim, 0xC5,
                                             (u32) jijiDemoJijiAnim)) == -1) {
                        w->state++;
                        step = 0;
                        jijiDemoTurnX = (jijiDemoGoeX + jijiDemoEbiX) / 2;
                        jijiDemoTurnZ = (jijiDemoGoeZ + jijiDemoEbiZ) / 2;
                    }
                    break;
                case 9:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], jijiDemoTurnX,
                                                      jijiDemoTurnZ, w->model->rot[1], 1.0f);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (jijiDemoFrame == 0xAE6) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 10:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], jijiDemoYaeX,
                                                      jijiDemoYaeZ, w->model->rot[1], 1.0f);
                    if ((step = tunagi_anime(w->model, step, 2, 2, 0.4f, 0.4f, 0xC5, (u32) jijiDemoJijiTunagiAnim, 0xC5,
                                             (u32) jijiDemoJijiState1Anim)) == -1) {
                        w->state++;
                        step = 0;
                        scRunEnd();
                        scRunStart(0xD5, 0, 0);
                        speed = 0.3f;
                    }
                    break;
                case 11:
                    demoAnimeStep(w->model, speed, 0);
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], jijiDemoTurnX,
                                                      jijiDemoTurnZ, w->model->rot[1], 1.0f);
                    switch (jijiDemoFrame) {
                        case 0xC62:
                            scRunEnd();
                            scRunStart(0xD6, 0, 0);
                            jijiDemoTurnX = jijiDemoYaeX;
                            jijiDemoTurnZ = jijiDemoYaeZ;
                            break;
                        case 0xD66:
                            scRunEnd();
                            scRunStart(0xD7, 0, 0);
                            jijiDemoTurnX = jijiDemoYaeX;
                            jijiDemoTurnZ = jijiDemoYaeZ;
                            speed = 0.3f;
                            break;
                        case 0xE1A:
                            scRunEnd();
                            scRunStart(0xD8, 0, 0);
                            speed = 0.5f;
                            jijiDemoTurnX = jijiDemoGoeX;
                            jijiDemoTurnZ = jijiDemoGoeZ;
                            break;
                        case 0xF64:
                            w->state++;
                            break;
                    }
                    break;
                case 12:
                    if ((step = tunagi_anime(w->model, step, 2, 3, 0.2f, 0.2f, 0xC5, (u32) jijiDemoJijiTunagiAnim, 0xC5,
                                             (u32) jijiDemoJijiAnim)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 13:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (jijiDemoFrame == 0x109A) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 14:
                    if ((step = tunagi_anime(w->model, step, 2, 2, 0.4f, 0.4f, 0xC5, (u32) jijiDemoJijiTunagiAnim, 0xC5,
                                             (u32) jijiDemoJijiState1Anim)) == -1) {
                        w->state++;
                        step = 0;
                        scRunEnd();
                        scRunStart(0xDA, 0, 0);
                        jijiDemoTurnX = jijiDemoGoeX;
                        jijiDemoTurnZ = jijiDemoGoeZ;
                        speed = 0.3f;
                    }
                    break;
                case 15:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], jijiDemoTurnX,
                                                      jijiDemoTurnZ, w->model->rot[1], 1.0f);
                    demoAnimeStep(w->model, speed, 0);
                    switch (jijiDemoFrame) {
                        case 0x1248:
                            scRunEnd();
                            scRunStart(0xDB, 0, 0);
                            jijiDemoTurnX = jijiDemoEbiX;
                            jijiDemoTurnZ = jijiDemoEbiZ;
                            break;
                        case 0x13BA:
                            w->state++;
                            break;
                    }
                    break;
                case 16:
                    jijiDemoTurnX = jijiDemoGoeX;
                    jijiDemoTurnZ = jijiDemoGoeZ;
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], jijiDemoEbiX,
                                                      jijiDemoEbiZ, w->model->rot[1], 1.0f);
                    if ((step = tunagi_anime(w->model, step, 2, 3, 0.2f, 0.2f, 0xC5, (u32) jijiDemoJijiTunagiAnim, 0xC5,
                                             (u32) jijiDemoJijiAnim)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    if (jijiDemoFrame >= 0x13BB) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], jijiDemoYaeX,
                                                          jijiDemoYaeZ, w->model->rot[1], 1.0f);
                    }
                    break;
                case 17:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (jijiDemoFrame == 0x15F4) {
                        w->state++;
                        step = 0;
                    }
                    if (jijiDemoFrame >= 0x13BB) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], jijiDemoYaeX,
                                                          jijiDemoYaeZ, w->model->rot[1], 1.0f);
                    }
                    break;
                case 18:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], jijiDemoGoeX,
                                                      jijiDemoGoeZ, w->model->rot[1], 1.0f);
                    if ((step = tunagi_anime(w->model, step, 2, 2, 0.4f, 0.4f, 0xC5, (u32) jijiDemoJijiTunagiAnim, 0xC5,
                                             (u32) jijiDemoJijiState1Anim)) == -1) {
                        w->state++;
                        step = 0;
                        scRunEnd();
                        scRunStart(0xDE, 0, 0);
                        speed = 0.3f;
                        jijiDemoTurnX = jijiDemoGoeX;
                        jijiDemoTurnZ = jijiDemoGoeZ;
                    }
                    break;
                case 19:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], jijiDemoTurnX,
                                                      jijiDemoTurnZ, w->model->rot[1], 1.0f);
                    demoAnimeStep(w->model, speed, 0);
                    switch (jijiDemoFrame) {
                        case 0x16BC:
                            scRunEnd();
                            scRunStart(0xDF, 0, 0);
                            jijiDemoTurnX = jijiDemoGoeX;
                            jijiDemoTurnZ = jijiDemoGoeZ;
                            break;
                        case 0x1838:
                            scRunEnd();
                            scRunStart(0xE0, 0, 0);
                            jijiDemoTurnX = jijiDemoEbiX;
                            jijiDemoTurnZ = jijiDemoEbiZ;
                            break;
                        case 0x1964:
                            jijiDemoTurnX = jijiDemoGoeX;
                            jijiDemoTurnZ = jijiDemoGoeZ;
                            scRunEnd();
                            w->state++;
                            break;
                    }
                    break;
                case 20:
                    if ((step = tunagi_anime(w->model, step, 2, 3, 0.2f, 0.2f, 0xC5, (u32) jijiDemoJijiTunagiAnim, 0xC5,
                                             (u32) jijiDemoJijiAnim)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 21:
                    demoAnimeStep(w->model, 0.2f, 0);
                    break;
            }
            jijiDemoJijiX = w->model->pos[0];
            jijiDemoJijiZ = w->model->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *jijiDemoJijiCreate(Obj *obj) {
    return tskCreate(obj, jijiDemoJijiProc, obj->file, NULL);
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
extern Gfx jijiDemoSmokeDl[];                       /* the smoke's display list */
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
extern u8 jijiDemoSmokeTex0[];
extern u8 jijiDemoSmokeTex1[];
extern u8 jijiDemoSmokeTex2[];
extern u8 jijiDemoSmokeTex3[];
extern u8 jijiDemoSmokeTex4[];
extern u8 jijiDemoSmokeTex6[];
/* This file's .data (0x08005A60): the characters' texture animations (demotool.c's table, as
 * tlb_0020_1's; nothing reads it), then the smoke's texture animation. */
AnimFrame jijiDemoCharaAnims[] = {
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
    { (u32) jijiDemoSmokeTex1, 8 },
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
    { (u32) jijiDemoSmokeTex1, 7 },
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
    { (u32) jijiDemoSmokeTex1, 200 },
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
AnimFrame jijiDemoSmokeAnim[] = { /* the smoke's texture animation */
    { (u32) jijiDemoSmokeTex0, 6 },
    { (u32) jijiDemoSmokeTex1, 6 },
    { (u32) jijiDemoSmokeTex2, 6 },
    { (u32) jijiDemoSmokeTex3, 6 },
    { (u32) jijiDemoSmokeTex4, 6 },
    { 0x03002D30, 8 },
    { (u32) jijiDemoSmokeTex6, 10 },
    { 0, 0 },
    { 0, 0 },
};

s32 kemuriProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    KemuriWork *w = (KemuriWork *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "KEMURI";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, jijiDemoSmokeDl, 0x123);
            fadeAnimStart(&w->anim, jijiDemoSmokeAnim, 0, &w->frame->segs[3], floadGetSetFile(0x123, 0));
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
            if (!(jijiDemoFrame & 1)) {
                fadeAnimStep(&w->anim);
            }
            if (jijiDemoFrame % w->period == 0) {
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

/* Starts a "KEMURI" smoke puff under the effects' parent at (x, y, z): scale, growth per frame,
 * rise per frame, and an alpha that loses fade every period frames. */
void kemuriCreate(f32 x, f32 y, f32 z, f32 scale, f32 grow, f32 rise, s32 alpha, s32 fade,
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
    tskCreate(gEffectTask, kemuriProc, gDmenuLastObj->file, &arg);
}

void jijiDemoChimneySmoke(void) {
    if (jijiDemoFrame % 40 == 0) {
        kemuriCreate(14.0f, 12.0f, 34.0f, 0.07f, 1.0f, 0.1f, 30, 1, 4);
    }
}
