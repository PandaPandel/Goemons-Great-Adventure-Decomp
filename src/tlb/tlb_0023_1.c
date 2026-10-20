#include "tlb.h"
#include "symbol_pad.h"
#include "tlb/tlb_0023.h"

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

extern s32 omiDemoFrame; /* the demo's frame count */
extern f32 omiDemoSasukeX; /* Sasuke's x and z, for the others */
extern f32 omiDemoSasukeZ;
extern f32 omiDemoGoe1X; /* the first scene's Goemon's x and z */
extern f32 omiDemoGoe1Z;
extern AnimFrame omiDemoOmi2FaceAnims[]; /* the second Omitchan's face animations */
extern AnimFrame omiDemoOmiAnim[];
extern f32 omiDemoOmiX; /* Omitchan's x and z, for the others */
extern f32 omiDemoOmiZ;
extern f32 omiDemoJijiX; /* the old man's x and z */
extern f32 omiDemoJijiZ;
extern f32 omiDemoMachineX; /* the machine's x and z */
extern f32 omiDemoMachineZ;
extern AnimFrame omiDemoOmiTexAnim[]; /* Omitchan's texture animation */
/* The file's .bss, in the game's order: the statics of the second scene's old man, Kurobei
 * and Omitchan come first, though their functions are late in the file: their symbol numbers
 * wrap past 256 (IDO_NOTES 337, SYMBOL_PAD). Then 4GOE's, genoGOE's and chaSAS's statics. */

extern u8 omiDemoShadowDl[];
extern u8 omiDemoKiranDl[]; /* the sparkle's display list (file set 0xC6: omiDemoShadowDl's address in another file) */
extern u8 omiDemoKiranGlowDl[]; /* its glow's */
extern u8 omiDemoOmiState4Anim[];
extern u8 omiDemoOmiState2Anim[];
extern u8 omiDemoJijiAnim[];
extern u8 omiDemoOmiState5Anim[];
extern u8 omiDemoMachineAnim[];
extern u8 omiDemoKuroState3Anim[];
extern u8 omiDemoShortOmiAnim[];
extern u8 omiDemoJijiTunagiAnim[];
extern u8 omiDemoJijiTunagiEndAnim[];
extern u8 omiDemoKuroAnim2[];
extern u8 omiDemoKuroAnim[];
extern u8 omiDemoJijiState0Anim[];
extern u8 omiDemoJijiState2Anim[];
extern u8 omiDemoJijiState3Anim[];

/* A village person's work ("jiji", "kuro"): the model and its shadow. */
typedef struct PersonW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ PolFrame *shadow;
    /* 0x08 */ u8 pad08[4];
    /* 0x0C */ s32 state;
} PersonW;

/* "omi"'s work: Omitchan's model, her shadow, and her face's texture animation. */
typedef struct OmiW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ PolFrame *shadow;
    /* 0x08 */ Anim anim;
    /* 0x18 */ u8 pad18[4];
    /* 0x1C */ s32 state;
} OmiW;

/* A demo player's work ("genoGOE"...): its player task, model and shadow, and two face
 * animations of one kind (demotool.c's DemoActor, with a shadow). */
typedef struct DemoGoeW {
    /* 0x00 */ Obj *player;
    /* 0x04 */ AnimeFrame *frame;
    /* 0x08 */ PolFrame *shadow;
    /* 0x0C */ Anim anim[2];
    /* 0x2C */ s32 kind;
    /* 0x30 */ u8 pad30[4];
    /* 0x34 */ s32 state;
} DemoGoeW;

extern f32 omiDemoGoeX; /* Goemon's x and z, for the others */
extern f32 omiDemoGoeZ;

void omiDemoKiranCreate(Obj *obj);
Obj *omiDemoMachineCreate(Obj *obj);
Obj *omiDemoBeamCreate(Obj *obj);

/* Main's sinf, called through its address as TLB code calls main. */
#define sinf (*sinf)

/* "4GOE": Goemon in the opening demo's first scene. At demo frame 0x14E he walks 300 toward the
 * back (scene run 0x73 at 0x158) until 0x1BC and stands at (-1, 1, -49); from 0x220 his joined
 * animations (scene run 0x74) until 0x306; at 0x63A he stands at (-16, 0, -57) with his face
 * animating (scene run 0x79 at 0x768 to 0x81C), starts the MACHINE at 0x830 and its beam at 0x952
 * (sound 0x1AC), turns toward the old man from 0xA7E, plays joined animations from 0xBAA (scene
 * run 0x7A to 0xC4A), stands at (4, 0, -48) at 0xF24 and from 0x12DA plays them once more (scene
 * runs 0x85, 0x86 at 0x137A, to 0x1442).
 * The first rot[1] is the double pi (the game loads pi twice, from two .rodata words), and
 * state 15's second demoAnimeStep is 0.2f, the game's constant. */
s32 omiDemoGoeProc(Obj *obj, s16 msg, void *arg) {
    static s32 goeStep;      /* .bss 0x0800967C: his demoPlTunagiAnime step */
    static s32 reserved;      /* .bss 0x08009680: a word of the game's .bss that nothing reads */
    static Player *goePlayer; /* .bss 0x08009684: his player */
    DemoGoeW *w = (DemoGoeW *) (obj + 1);
    s32 pad[2];
    PlInit init;

    switch (msg) {
        case 2:
            PRINTF("goecreate\n");
            obj->name = "4GOE";
            plMakeInit(&init, 0, 0);
            init.flags |= 0x90000040;
            init.flags2 |= 8;
            init.zone = 0;
            init.place = -50.0f;
            init.height = 0.0f;
            init.side = 77.0f;
            w->player = plCreate(gDmenuLastObj, &init);
            goePlayer = PL(w->player);
            plEnterDemo(w->player);
            PRINTF("デモモード突入\n");
            PRINTF("歩きモーション\n");
            plAnimeSet(PL(w->player), 0xDC);
            w->frame = goePlayer->model;
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, (Gfx *) omiDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.08f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->kind = 0;
            w->frame->rot[1] = 3.141592653589793;
            PRINTF("*** p->model_buff=%x\n", goePlayer->model);
            PRINTF("ゴエモン作ります\n");
            w->frame->scale[2] = 0.097f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = -50.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 77.0f;
            w->frame->rot[1] = 3.141592741f;
            PRINTF("ゴエモン作りました\n");
            PRINTF("goecreate\n");
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            frmDestroy((Frame *) w->shadow);
            tskDestroy(w->player);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, 0x80);
            break;
        case 0:
            w->frame->f.flags |= 2;
            if (gObjMoveSel == 0) {
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
            w->shadow->pos[0] = w->frame->pos[0];
            w->shadow->pos[2] = w->frame->pos[2];
            w->shadow->pos[1] = 0.3f;
            omiDemoGoe1X = w->frame->pos[0];
            omiDemoGoe1Z = w->frame->pos[2];
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->frame, 0.3f, 0);
                    if (omiDemoFrame == 0x14E) {
                        w->state++;
                    }
                    break;
                case 1:
                    demoWalkTo(w->frame, 0.6f, 0.8f, w->frame->pos[0], w->frame->pos[2] - 300.0f);
                    if (omiDemoFrame == 0x158) {
                        scRunStart(0x73, 0, 0);
                    }
                    if (omiDemoFrame == 0x1BC) {
                        plAnimeSet(PL(w->player), 0xE9);
                        w->frame->pos[0] = -1.0f;
                        w->frame->pos[1] = 1.0f;
                        w->frame->pos[2] = -49.0f;
                        w->frame->rot[1] = -3.01941967f;
                        goeStep = 0;
                        w->state++;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->frame, 0.2f, 0);
                    if (omiDemoFrame == 0x220) {
                        w->state++;
                    }
                    break;
                case 3:
                    goeStep = demoPlTunagiAnime(obj, 2, goeStep, 2, 2, 0.4f, 0.4f, 0xEB, 0xEC);
                    if (goeStep == -1) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0x74, 0, 0);
                        goeStep = 0;
                    }
                    break;
                case 4:
                    fadeAnimStep(&w->anim[0]);
                    fadeAnimStep(&w->anim[1]);
                    demoAnimeStep(w->frame, 0.4f, 0);
                    if (omiDemoFrame == 0x306) {
                        w->state++;
                        goeStep = 0;
                    }
                    break;
                case 5:
                    goeStep = demoPlTunagiAnime(obj, 5, goeStep, 2, 3, 0.2f, 0.2f, 0xEB, 0xE9);
                    if (goeStep == -1) {
                        w->state++;
                    }
                    break;
                case 6:
                    demoAnimeStep(w->frame, 0.2f, 0);
                    if (omiDemoFrame == 0x63A) {
                        plAnimeSet(PL(w->player), 0xE7);
                        demoTexAnimK0(&w->anim[0], 2, &PL(w->player)->model->segs[3]);
                        demoTexAnimK0(&w->anim[1], 2, &PL(w->player)->model->segs[4]);
                        w->frame->pos[0] = -16.0f;
                        w->frame->pos[1] = 0.0f;
                        w->frame->pos[2] = -57.0f;
                        w->frame->rot[1] = 0.2094395161f;
                        w->state++;
                    }
                    break;
                case 7:
                    demoAnimeStep(w->frame, 0.2f, 0);
                    if (omiDemoFrame == 0x768) {
                        scRunEnd();
                        scRunStart(0x79, 0, 0);
                    }
                    if (omiDemoFrame == 0x81C) {
                        scRunEnd();
                    }
                    if (omiDemoFrame == 0x830) {
                        (*omiDemoMachineCreate)(obj);
                    }
                    if (omiDemoFrame == 0x952) {
                        (*omiDemoBeamCreate)(obj);
                        sndPlay(0x1AC);
                    }
                    if (omiDemoFrame == 0xA7E) {
                        plAnimeSet(PL(w->player), 0xE9);
                        demoTexAnimK0(&w->anim[0], 0, &PL(w->player)->model->segs[3]);
                        demoTexAnimK0(&w->anim[1], 0, &PL(w->player)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 8:
                    demoAnimeStep(w->frame, 0.2f, 0);
                    w->frame->rot[1] = demoTurnToward(w->frame->pos[0], w->frame->pos[2], omiDemoJijiX,
                                                      omiDemoJijiZ, w->frame->rot[1], 1.0f);
                    if (omiDemoFrame == 0xBAA) {
                        w->state++;
                        goeStep = 0;
                    }
                    break;
                case 9:
                    goeStep = demoPlTunagiAnime(obj, 2, goeStep, 2, 2, 0.4f, 0.4f, 0xEB, 0xEC);
                    if (goeStep == -1) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0x7A, 0, 0);
                        goeStep = 0;
                    }
                    break;
                case 10:
                    fadeAnimStep(&w->anim[0]);
                    fadeAnimStep(&w->anim[1]);
                    demoAnimeStep(w->frame, 0.3f, 0);
                    if (omiDemoFrame == 0xC4A) {
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 11:
                    goeStep = demoPlTunagiAnime(obj, 0, goeStep, 2, 3, 0.4f, 0.4f, 0xEB, 0xE9);
                    if (goeStep == -1) {
                        w->state++;
                        goeStep = 0;
                    }
                    break;
                case 12:
                    demoAnimeStep(w->frame, 0.3f, 0);
                    fadeAnimStep(&w->anim[0]);
                    fadeAnimStep(&w->anim[1]);
                    if (omiDemoFrame == 0xF24) {
                        w->frame->pos[0] = 4.0f;
                        w->frame->pos[1] = 0.0f;
                        w->frame->pos[2] = -48.0f;
                        w->frame->rot[1] = 5.934119701f;
                        w->state++;
                    }
                    break;
                case 13:
                    demoAnimeStep(w->frame, 0.3f, 0);
                    fadeAnimStep(&w->anim[0]);
                    fadeAnimStep(&w->anim[1]);
                    if (omiDemoFrame == 0x12DA) {
                        w->state++;
                    }
                    break;
                case 14:
                    goeStep = demoPlTunagiAnime(obj, 2, goeStep, 2, 2, 0.4f, 0.4f, 0xEB, 0xEC);
                    if (goeStep == -1) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0x85, 0, 0);
                        goeStep = 0;
                    }
                    break;
                case 15:
                    fadeAnimStep(&w->anim[0]);
                    fadeAnimStep(&w->anim[1]);
                    demoAnimeStep(w->frame, 0.4f, 0);
                    if (omiDemoFrame == 0x137A) {
                        scRunEnd();
                        scRunStart(0x86, 0, 0);
                    }
                    if (omiDemoFrame >= 0x137B) {
                        demoAnimeStep(w->frame, 0.2f, 0);
                    }
                    if (omiDemoFrame == 0x1442) {
                        w->state++;
                        goeStep = 0;
                        scRunEnd();
                    }
                    break;
                case 16:
                    goeStep = demoPlTunagiAnime(obj, 5, goeStep, 2, 3, 0.2f, 0.2f, 0xEB, 0xE9);
                    if (goeStep == -1) {
                        w->state++;
                    }
                    break;
                case 17:
                    fadeAnimStep(&w->anim[0]);
                    fadeAnimStep(&w->anim[1]);
                    demoAnimeStep(w->frame, 0.4f, 0);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 omiDemoGoeProc(Obj *obj, s16 msg, void *arg);

Obj *omiDemoGoeCreate(Obj *obj) {
    return tskCreate(obj, omiDemoGoeProc, obj->file, NULL);
}

/* "genoGOE": Goemon in the opening demo. At demo frame 0x14E he walks 300 toward the back until
 * 0x1BC and stands at (14, 0, -63); at 0x57C a sparkle (sound 0x1A9); then he faces the sparkle
 * (scene run 0x77), stands at (-21, 0, -30) at 0x63A with his face animating (scene runs 0x78 at
 * 0x676, to 0x766), turns toward the old man from 0xA7E, stands at (-11, 0, -53) at 0xF24, and
 * from 0xF38 plays his joined animations (scene run 0x81 to 0x1000).
 * His demoPlTunagiAnime step is a static of this function (0x08009688; IDO_NOTES 343): the
 * game forms its address at every use, which only a function static gets. The game's .rodata
 * has an unused 0.3f (0x080093A4) between case 8's 0.4f and case 9's 0.3f. */
s32 omiDemoEbiProc(Obj *obj, s16 msg, void *arg) {
    static s32 omiDemoEbiTunagiStep; /* his demoPlTunagiAnime step */
    DemoGoeW *w = (DemoGoeW *) (obj + 1);
    s32 pad[2];
    Player *p;
    PlInit init;

    switch (msg) {
        case 2:
            obj->name = "genoGOE";
            plMakeInit(&init, 0, 1);
            init.flags |= 0x90000040;
            init.flags2 |= 8;
            init.zone = 0;
            init.place = -33.0f;
            init.height = 0.0f;
            init.side = 80.0f;
            w->player = plCreate(gDmenuLastObj, &init);
            p = PL(w->player);
            plEnterDemo(w->player);
            PRINTF("デモモード突入\n");
            PRINTF("歩きモーション\n");
            plAnimeSet(PL(w->player), 0xDC);
            w->frame = p->model;
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, (Gfx *) omiDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.09f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->kind = 1;
            w->state = 0;
            w->frame->scale[2] = 0.097f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = -33.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 80.0f;
            w->frame->rot[1] = 3.141592741f;
            PRINTF("エビス丸作りました\n");
            PRINTF("goecreate\n");
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            frmDestroy((Frame *) w->shadow);
            tskDestroy(w->player);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, 0x80);
            break;
        case 0:
            w->frame->f.flags |= 2;
            if (gObjMoveSel == 1) {
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
            w->shadow->pos[0] = w->frame->pos[0];
            w->shadow->pos[2] = w->frame->pos[2];
            w->shadow->pos[1] = 0.3f;
            omiDemoGoeX = w->frame->pos[0];
            omiDemoGoeZ = w->frame->pos[2];
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->frame, 0.3f, 0);
                    if (omiDemoFrame == 0x14E) {
                        w->state++;
                    }
                    break;
                case 1:
                    demoWalkTo(w->frame, 0.6f, 0.48f, w->frame->pos[0], w->frame->pos[2] - 300.0f);
                    if (omiDemoFrame == 0x1BC) {
                        plAnimeSet(PL(w->player), 0xE9);
                        w->frame->pos[0] = 14.0f;
                        w->frame->pos[1] = 0.0f;
                        w->frame->pos[2] = -63.0f;
                        w->frame->rot[1] = 4.380776405f;
                        w->state++;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->frame, 0.3f, 0);
                    if (omiDemoFrame == 0x57C) {
                        sndPlay(0x1A9);
                        (*omiDemoKiranCreate)(obj);
                        w->state++;
                    }
                    break;
                case 3:
                    if (demoAnimeStep(w->frame, 0.3f, 2)) {
                        w->state++;
                        plAnimeSet(PL(w->player), 0xE9);
                        scRunEnd();
                        scRunStart(0x77, 0, 0);
                    }
                    break;
                case 4:
                    w->frame->rot[1] = demoTurnToward(w->frame->pos[0], w->frame->pos[2], -4.0f, -82.0f,
                                                      w->frame->rot[1], 1.0f);
                    if (omiDemoFrame == 0x63A) {
                        w->state++;
                        plAnimeSet(PL(w->player), 0xE7);
                        demoTexAnimK1(&w->anim[0], 2, &PL(w->player)->model->segs[3]);
                        demoTexAnimK1(&w->anim[1], 2, &PL(w->player)->model->segs[4]);
                        w->frame->pos[0] = -21.0f;
                        w->frame->pos[1] = 0.0f;
                        w->frame->pos[2] = -30.0f;
                        w->frame->rot[1] = 0.1570796371f;
                    }
                    break;
                case 5:
                    demoAnimeStep(w->frame, 0.3f, 0);
                    fadeAnimStep(&w->anim[0]);
                    fadeAnimStep(&w->anim[1]);
                    if (omiDemoFrame == 0x676) {
                        scRunEnd();
                        scRunStart(0x78, 0, 0);
                    }
                    if (omiDemoFrame == 0x766) {
                        scRunEnd();
                    }
                    if (omiDemoFrame == 0xA7E) {
                        plAnimeSet(PL(w->player), 0xE9);
                        w->state++;
                    }
                    break;
                case 6:
                    fadeAnimStep(&w->anim[0]);
                    fadeAnimStep(&w->anim[1]);
                    demoAnimeStep(w->frame, 0.2f, 0);
                    w->frame->rot[1] = demoTurnToward(w->frame->pos[0], w->frame->pos[2], omiDemoJijiX,
                                                      omiDemoJijiZ, w->frame->rot[1], 1.0f);
                    if (omiDemoFrame == 0xF24) {
                        w->frame->pos[0] = -11.0f;
                        w->frame->pos[1] = 0.0f;
                        w->frame->pos[2] = -53.0f;
                        w->frame->rot[1] = 6.667158127f;
                        w->state++;
                    }
                    break;
                case 7:
                    fadeAnimStep(&w->anim[0]);
                    fadeAnimStep(&w->anim[1]);
                    demoAnimeStep(w->frame, 0.2f, 0);
                    if (omiDemoFrame == 0xF38) {
                        w->state++;
                        omiDemoEbiTunagiStep = 0;
                    }
                    break;
                case 8:
                    omiDemoEbiTunagiStep = demoPlTunagiAnime(obj, 2, omiDemoEbiTunagiStep, 2, 2, 0.4f, 0.4f, 0xEB, 0xEC);
                    if (omiDemoEbiTunagiStep == -1) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0x81, 0, 0);
                        omiDemoEbiTunagiStep = 0;
                    }
                    break;
                case 9:
                    fadeAnimStep(&w->anim[0]);
                    fadeAnimStep(&w->anim[1]);
                    demoAnimeStep(w->frame, 0.3f, 0);
                    if (omiDemoFrame == 0x1000) {
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 10:
                    omiDemoEbiTunagiStep = demoPlTunagiAnime(obj, 0, omiDemoEbiTunagiStep, 2, 3, 0.4f, 0.4f, 0xEB, 0xE9);
                    if (omiDemoEbiTunagiStep == -1) {
                        w->state++;
                        omiDemoEbiTunagiStep = 0;
                    }
                    break;
                case 11:
                    demoAnimeStep(w->frame, 0.3f, 0);
                    fadeAnimStep(&w->anim[0]);
                    fadeAnimStep(&w->anim[1]);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 omiDemoEbiProc(Obj *obj, s16 msg, void *arg);

Obj *omiDemoEbiCreate(Obj *obj) {
    return tskCreate(obj, omiDemoEbiProc, obj->file, NULL);
}

/* "chaSAS": Sasuke in the opening demo. At demo frame 0x14E he runs 300 toward the back until
 * 0x1BC and stands at (-17, 0, -52); from 0x2E8 his joined animations (scene run 0x75) until
 * 0x3B0; he stands at (2, 0, -67) at 0x63A with his face animating, turns toward the old man from
 * 0xA7E, plays his joined animations again from 0xC5E (scene run 0x7B, to 0xD26), and stands at
 * (14, 0, -42) at 0xF24.
 * His step (0x0800968C) is a static of this function, as genoGOE's (IDO_NOTES 343). */
s32 omiDemoSasProc(Obj *obj, s16 msg, void *arg) {
    static s32 omiDemoSasTunagiStep; /* his demoPlTunagiAnime step */
    DemoGoeW *w = (DemoGoeW *) (obj + 1);
    s32 pad[2];
    Player *p;
    PlInit init;

    switch (msg) {
        case 2:
            obj->name = "chaSAS";
            plMakeInit(&init, 0, 2);
            init.flags |= 0x90000040;
            init.flags2 |= 8;
            init.zone = 0;
            init.place = -17.0f;
            init.height = 0.0f;
            init.side = -52.0f;
            w->player = plCreate(gDmenuLastObj, &init);
            p = PL(w->player);
            plEnterDemo(w->player);
            PRINTF("デモモード突入\n");
            PRINTF("走りモーション\n");
            plAnimeSet(PL(w->player), 0xDC);
            PRINTF("サスケ作ります\n");
            w->frame = p->model;
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, (Gfx *) omiDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.065f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->kind = 2;
            w->state = 0;
            w->frame->rot[1] = 0.0f;
            w->frame->scale[2] = 0.097f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = -62.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 76.0f;
            w->frame->rot[1] = -3.211405993f;
            PRINTF("サスケ作りました\n");
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            frmDestroy((Frame *) w->shadow);
            tskDestroy(w->player);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, 0x80);
            break;
        case 0:
            w->frame->f.flags |= 2;
            if (gObjMoveSel == 2) {
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
            omiDemoSasukeX = w->frame->pos[0];
            omiDemoSasukeZ = w->frame->pos[2];
            w->shadow->pos[0] = w->frame->pos[0];
            w->shadow->pos[2] = w->frame->pos[2];
            w->shadow->pos[1] = 0.3f;
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->frame, 0.3f, 0);
                    if (omiDemoFrame == 0x14E) {
                        w->state++;
                    }
                    break;
                case 1:
                    demoWalkTo(w->frame, 0.6f, 1.12f, w->frame->pos[0], w->frame->pos[2] - 300.0f);
                    if (omiDemoFrame == 0x1BC) {
                        plAnimeSet(PL(w->player), 0xE9);
                        w->frame->pos[0] = -17.0f;
                        w->frame->pos[1] = 0.0f;
                        w->frame->pos[2] = -52.0f;
                        w->frame->rot[1] = -3.909537554f;
                        w->state++;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->frame, 0.3f, 0);
                    if (omiDemoFrame == 0x2E8) {
                        w->state++;
                    }
                    break;
                case 3:
                    omiDemoSasTunagiStep = demoPlTunagiAnime(obj, 2, omiDemoSasTunagiStep, 2, 2, 0.4f, 0.4f, 0xEB, 0xEC);
                    if (omiDemoSasTunagiStep == -1) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0x75, 0, 0);
                        omiDemoSasTunagiStep = 0;
                    }
                    break;
                case 4:
                    fadeAnimStep(&w->anim[0]);
                    fadeAnimStep(&w->anim[1]);
                    demoAnimeStep(w->frame, 0.4f, 0);
                    if (omiDemoFrame == 0x3B0) {
                        w->state++;
                        omiDemoSasTunagiStep = 0;
                    }
                    break;
                case 5:
                    omiDemoSasTunagiStep = demoPlTunagiAnime(obj, 0, omiDemoSasTunagiStep, 2, 3, 0.2f, 0.2f, 0xEB, 0xE9);
                    if (omiDemoSasTunagiStep == -1) {
                        w->state++;
                    }
                    break;
                case 6:
                    demoAnimeStep(w->frame, 0.2f, 0);
                    if (omiDemoFrame == 0x63A) {
                        w->state++;
                        plAnimeSet(PL(w->player), 0xE7);
                        demoTexAnimK2(&w->anim[0], 2, &PL(w->player)->model->segs[3]);
                        demoTexAnimK2(&w->anim[1], 2, &PL(w->player)->model->segs[4]);
                        w->frame->pos[0] = 2.0f;
                        w->frame->pos[1] = 0.0f;
                        w->frame->pos[2] = -67.0f;
                        w->frame->rot[1] = 0.0f;
                    }
                    break;
                case 7:
                    demoAnimeStep(w->frame, 0.2f, 0);
                    if (omiDemoFrame == 0xA7E) {
                        plAnimeSet(PL(w->player), 0xE9);
                        w->state++;
                    }
                    break;
                case 8:
                    demoAnimeStep(w->frame, 0.2f, 0);
                    w->frame->rot[1] = demoTurnToward(w->frame->pos[0], w->frame->pos[2], omiDemoJijiX,
                                                      omiDemoJijiZ, w->frame->rot[1], 1.0f);
                    if (omiDemoFrame == 0xC5E) {
                        w->state++;
                    }
                    break;
                case 9:
                    omiDemoSasTunagiStep = demoPlTunagiAnime(obj, 2, omiDemoSasTunagiStep, 2, 2, 0.4f, 0.4f, 0xEB, 0xEC);
                    if (omiDemoSasTunagiStep == -1) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0x7B, 0, 0);
                        omiDemoSasTunagiStep = 0;
                    }
                    break;
                case 10:
                    fadeAnimStep(&w->anim[0]);
                    fadeAnimStep(&w->anim[1]);
                    demoAnimeStep(w->frame, 0.4f, 0);
                    if (omiDemoFrame == 0xD26) {
                        w->state++;
                        omiDemoSasTunagiStep = 0;
                    }
                    break;
                case 11:
                    omiDemoSasTunagiStep = demoPlTunagiAnime(obj, 0, omiDemoSasTunagiStep, 2, 3, 0.2f, 0.2f, 0xEB, 0xE9);
                    if (omiDemoSasTunagiStep == -1) {
                        w->state++;
                    }
                    break;
                case 12:
                    demoAnimeStep(w->frame, 0.2f, 0);
                    if (omiDemoFrame == 0xF24) {
                        w->frame->pos[0] = 14.0f;
                        w->frame->pos[1] = 0.0f;
                        w->frame->pos[2] = -42.0f;
                        w->frame->rot[1] = 5.39306736f;
                        w->state++;
                    }
                    break;
                case 13:
                    demoAnimeStep(w->frame, 0.2f, 0);
                    fadeAnimStep(&w->anim[0]);
                    fadeAnimStep(&w->anim[1]);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 omiDemoSasProc(Obj *obj, s16 msg, void *arg);

Obj *omiDemoSasCreate(Obj *obj) {
    return tskCreate(obj, omiDemoSasProc, obj->file, NULL);
}

/* The file's .data, in address order (its externs above stay as declarations). */
extern u8 demoPieceTex0[];
extern u8 demoPieceTex1[];
extern u8 demoPieceTex2[];
extern u8 demoPieceTex3[];
extern u8 demoPieceTex4[];
extern u8 demoPieceTex7[];
extern u8 demoPieceTex15[];
extern u8 demoPieceTex19[];
extern u8 omiDemoOmiFaceTex[];
extern u8 demoPieceTex78[];
extern u8 demoFaceTex0[];
extern u8 demoFaceTex1[];
extern u8 demoFaceTex2[];
/* (omiDemoFrame is tlb_0023_0's .data; this file's starts at 0x08008330 with the characters'
 * texture animations, demotool.c's table, which nothing reads) */
AnimFrame omiDemoCharaAnims[] = {
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
f32 omiDemoSasukeX = 5.0f; /* Sasuke's x and z, for the others */
f32 omiDemoSasukeZ = 5.0f;
f32 omiDemoGoe1X = 5.0f; /* the first scene's Goemon's x and z */
f32 omiDemoGoe1Z = 5.0f;
f32 omiDemoGoeX = 5.0f; /* Goemon's x and z, for the others */
f32 omiDemoGoeZ = 5.0f;
f32 omiDemoOmiX = 5.0f; /* Omitchan's x and z, for the others */
f32 omiDemoOmiZ = 5.0f;
f32 omiDemoJijiX = 5.0f; /* the old man's x and z */
f32 omiDemoJijiZ = 5.0f;
f32 omiDemoMachineX = 5.0f; /* the machine's x and z */
f32 omiDemoMachineZ = 5.0f;
u32 omiDemoUnused1[] = { 0x40A00000, 0x40A00000 };
/* The second "jiji" (the wise old man, "物知りじいさん作ります"): at (-28, -300, -2), hidden. At
 * demo frame 0x92A he stands at (0, 269, 206), small; at 0x9B6 he shows and from 0x9B7 sinks;
 * at 0xA7F he stands at (0, 46, -26) and later sinks again, his shadow darkening, to the ground;
 * from 0xD30 he talks (scene runs 0x7C, 0x7D at 0xE2A, 0x82 at 0x1000, 0x83 at 0x1078 and 0x84
 * at 0x11A4), turning toward the others.
 * His alpha and step are function statics, so a fresh lui reaches them at each use (IDO_NOTES
 * 343); they are the .data words at 0x08008D80/84, after the data defined above him. States 2
 * and 3 test the frame as "!(omiDemoFrame % 2)", which keeps IDO's whole signed remainder
 * (IDO_NOTES 367; "% 2 == 0" folds to one andi). */
s32 omiDemoJijiProc(Obj *obj, s16 msg, void *arg) {
    static s32 omiDemoJiji2ShadowAlpha = 1;
    static s32 omiDemoJiji2Step = 1;
    s32 pad;
    PersonW *w = (PersonW *) (obj + 1);
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "jiji";
            PRINTF("物知りじいさん作ります\n");
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoAnimeSet(w->frame, (u32) omiDemoJijiAnim, 0xC5, 0);
            demoPolSetDL(w->shadow, (Gfx *) omiDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.09f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->frame->scale[2] = 0.097f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = -28.0f;
            w->frame->pos[1] = -300.0f;
            w->frame->pos[2] = -2.0f;
            w->frame->rot[1] = -5.131268024f;
            w->frame->f.flags &= ~2;
            omiDemoJiji2ShadowAlpha = 0;
            PRINTF("物知りじいさん作りました\n");
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            frmDestroy((Frame *) w->shadow);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, omiDemoJiji2ShadowAlpha);
            break;
        case 0:
            w->shadow->pos[0] = w->frame->pos[0];
            w->shadow->pos[2] = w->frame->pos[2];
            w->shadow->pos[1] = 0.3f;
            omiDemoJijiX = w->frame->pos[0];
            omiDemoJijiZ = w->frame->pos[2];
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->frame, 0.3f, 0);
                    if (omiDemoFrame == 0x92A) {
                        demoAnimeSet(w->frame, (u32) omiDemoJijiState0Anim, 0xC5, 0);
                        w->frame->pos[0] = 0.0f;
                        w->frame->pos[1] = 269.0f;
                        w->frame->pos[2] = 206.0f;
                        w->frame->rot[1] = 3.141592741f;
                        w->frame->scale[2] = 0.05f;
                        w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                        w->state++;
                    }
                    break;
                case 1:
                    demoAnimeStep(w->frame, 0.1f, 0);
                    if (omiDemoFrame == 0x9B6) {
                        w->frame->f.flags |= 2;
                    }
                    if (omiDemoFrame >= 0x9B7) {
                        w->frame->pos[1] -= 0.5;
                    }
                    if (omiDemoFrame >= 0xA7F) {
                        w->frame->pos[0] = 0.0f;
                        w->frame->pos[1] = 46.0f;
                        w->frame->pos[2] = -26.0f;
                        w->frame->rot[1] = 3.141592741f;
                        w->frame->scale[2] = 0.097f;
                        w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                        w->state++;
                    }
                    break;
                case 2:
                    if (!(omiDemoFrame % 2)) {
                        omiDemoJiji2ShadowAlpha++;
                    }
                    if (omiDemoJiji2ShadowAlpha >= 0x100) {
                        omiDemoJiji2ShadowAlpha = 0xFF;
                    }
                    demoAnimeStep(w->frame, 0.1f, 0);
                    w->frame->pos[1] -= 0.4;
                    if (w->frame->pos[1] < 11.0f) {
                        demoAnimeSet(w->frame, (u32) omiDemoJijiState2Anim, 0xC5, 0);
                        w->state++;
                    }
                    break;
                case 3:
                    if (!(omiDemoFrame % 2)) {
                        omiDemoJiji2ShadowAlpha++;
                    }
                    if (omiDemoJiji2ShadowAlpha >= 0x100) {
                        omiDemoJiji2ShadowAlpha = 0xFF;
                    }
                    w->frame->pos[1] -= 0.4;
                    if (w->frame->pos[1] < -1.0f) {
                        w->frame->pos[1] = -1.0f;
                    }
                    if (demoAnimeStep(w->frame, 0.24f, 2)) {
                        demoAnimeSet(w->frame, (u32) omiDemoJijiState3Anim, 0xC5, 0);
                        w->state++;
                    }
                    break;
                case 4:
                    omiDemoJiji2ShadowAlpha++;
                    if (omiDemoJiji2ShadowAlpha >= 0x100) {
                        omiDemoJiji2ShadowAlpha = 0xFF;
                    }
                    if (demoAnimeStep(w->frame, 0.24f, 2)) {
                        demoAnimeSet(w->frame, (u32) omiDemoJijiAnim, 0xC5, 0);
                        w->state++;
                    }
                    break;
                case 5:
                    demoAnimeStep(w->frame, 0.3f, 0);
                    if (omiDemoFrame == 0xD30) {
                        w->state++;
                    }
                    break;
                case 6:
                    omiDemoJiji2Step = tunagi_anime(w->frame, omiDemoJiji2Step, 2, 2, 0.3f, 0.3f, 0xC5,
                                                       (u32) omiDemoJijiTunagiAnim, 0xC5, (u32) omiDemoJijiTunagiEndAnim);
                    if (omiDemoJiji2Step == -1) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0x7C, 0, 0);
                        omiDemoJiji2Step = 0;
                    }
                    break;
                case 7:
                    demoAnimeStep(w->frame, 0.4f, 0);
                    if (omiDemoFrame < 0xE2A) {
                        w->frame->rot[1] = demoTurnToward(w->frame->pos[0], w->frame->pos[2], omiDemoSasukeX,
                                                          omiDemoSasukeZ, w->frame->rot[1], 1.0f);
                    } else {
                        w->frame->rot[1] = demoTurnToward(w->frame->pos[0], w->frame->pos[2], omiDemoGoe1X,
                                                          omiDemoGoe1Z, w->frame->rot[1], 1.0f);
                    }
                    if (omiDemoFrame == 0xE2A) {
                        scRunEnd();
                        scRunStart(0x7D, 0, 0);
                    }
                    if (omiDemoFrame == 0xF24) {
                        demoAnimeSet(w->frame, (u32) omiDemoJijiAnim, 0xC5, 0);
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 8:
                    demoAnimeStep(w->frame, 0.4f, 0);
                    if (omiDemoFrame == 0x1000) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0x82, 0, 0);
                    }
                    break;
                case 9:
                    omiDemoJiji2Step = tunagi_anime(w->frame, omiDemoJiji2Step, 2, 2, 0.3f, 0.3f, 0xC5,
                                                       (u32) omiDemoJijiTunagiAnim, 0xC5, (u32) omiDemoJijiTunagiEndAnim);
                    if (omiDemoJiji2Step == -1) {
                        w->state++;
                        omiDemoJiji2Step = 0;
                    }
                    break;
                case 10:
                    demoAnimeStep(w->frame, 0.3f, 0);
                    if (omiDemoFrame < 0x1078) {
                        w->frame->rot[1] = demoTurnToward(w->frame->pos[0], w->frame->pos[2], omiDemoGoeX,
                                                          omiDemoGoeZ, w->frame->rot[1], 1.0f);
                    } else {
                        w->frame->rot[1] = demoTurnToward(w->frame->pos[0], w->frame->pos[2], omiDemoGoe1X,
                                                          omiDemoGoe1Z, w->frame->rot[1], 1.0f);
                    }
                    if (omiDemoFrame == 0x1078) {
                        scRunEnd();
                        scRunStart(0x83, 0, 0);
                    }
                    if (omiDemoFrame == 0x11A4) {
                        scRunEnd();
                        scRunStart(0x84, 0, 0);
                    }
                    if (omiDemoFrame >= 0x11A5) {
                        demoAnimeStep(w->frame, 0.3f, 0);
                    }
                    if (omiDemoFrame == 0x12D0) {
                        w->state++;
                    }
                    break;
                case 11:
                    omiDemoJiji2Step = tunagi_anime(w->frame, omiDemoJiji2Step, 2, 3, 0.3f, 0.3f, 0xC5,
                                                       (u32) omiDemoJijiTunagiAnim, 0xC5, (u32) omiDemoJijiAnim);
                    if (omiDemoJiji2Step == -1) {
                        w->state++;
                        omiDemoJiji2Step = 0;
                    }
                    break;
                case 12:
                    demoAnimeStep(w->frame, 0.2f, 0);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 omiDemoJijiProc(Obj *obj, s16 msg, void *arg);

Obj *omiDemoJijiCreate(Obj *obj) {
    return tskCreate(obj, omiDemoJijiProc, obj->file, NULL);
}

/* "jiji" (the wise old man, "物知りじいさん作ります"): stands at (0, 0, -26) with his shadow
 * under him; OBJMOVE's model 5 moves him. He turns to talk (state 0; scene run 0x7E, then 0x7F at
 * demo frame 0x104), talks until 0x208 (1), turns back (2) and idles (3). Publishes his x and z
 * each frame.
 * His step is a function static: the game forms its address at each use (lui at), which only a
 * function static gets (IDO_NOTES 343). His statics are the file's first .bss: their numbers
 * wrap past 256 (IDO_NOTES 337, include/symbol_pad.h). */
s32 omiDemoShortJijiProc(Obj *obj, s16 msg, void *arg) {
    static s32 omiDemoJijiShadowAlpha; /* .bss 0x08009640: his shadow's alpha */
    static s32 omiDemoJijiStep;        /* .bss 0x08009644: his tunagi_anime step */
    static s32 reserved0;              /* .bss 0x08009648-0x08009654: four words nothing reads */
    static s32 reserved1;
    static s32 reserved2;
    static s32 reserved3;
    s32 pad;
    PersonW *w = (PersonW *) (obj + 1);
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "jiji";
            PRINTF("物知りじいさん作ります\n");
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoAnimeSet(w->frame, (u32) omiDemoJijiAnim, 0xC5, 0);
            demoPolSetDL(w->shadow, (Gfx *) omiDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.08f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->frame->scale[2] = 0.097f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = -26.0f;
            w->frame->rot[1] = 3.508111954f;
            w->frame->f.flags &= ~2;
            PRINTF("物知りじいさん作りました\n");
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            frmDestroy((Frame *) w->shadow);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, omiDemoJijiShadowAlpha);
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
            omiDemoJijiX = w->frame->pos[0];
            omiDemoJijiZ = w->frame->pos[2];
            switch (w->state) {
                case 0:
                    omiDemoJijiStep = tunagi_anime(w->frame, omiDemoJijiStep, 2, 2, 0.3f, 0.3f, 0xC5,
                                                       (u32) omiDemoJijiTunagiAnim, 0xC5, (u32) omiDemoJijiTunagiEndAnim);
                    if (omiDemoJijiStep == -1) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0x7E, 0, 0);
                        omiDemoJijiStep = 0;
                    }
                    break;
                case 1:
                    demoAnimeStep(w->frame, 0.3f, 0);
                    if (omiDemoFrame == 0x104) {
                        scRunEnd();
                        scRunStart(0x7F, 0, 0);
                    }
                    if (omiDemoFrame >= 0x105) {
                        demoAnimeStep(w->frame, 0.3f, 0);
                    }
                    if (omiDemoFrame == 0x208) {
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 2:
                    omiDemoJijiStep = tunagi_anime(w->frame, omiDemoJijiStep, 2, 3, 0.3f, 0.3f, 0xC5,
                                                       (u32) omiDemoJijiTunagiAnim, 0xC5, (u32) omiDemoJijiAnim);
                    if (omiDemoJijiStep == -1) {
                        w->state++;
                        omiDemoJijiStep = 0;
                    }
                    break;
                case 3:
                    demoAnimeStep(w->frame, 0.3f, 0);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 omiDemoShortJijiProc(Obj *obj, s16 msg, void *arg);

Obj *omiDemoShortJijiCreate(Obj *obj) {
    return tskCreate(obj, omiDemoShortJijiProc, obj->file, NULL);
}

/* The second "kuro" (Kurobei, "クロベー作ります"): starts at (-15, 3, 76) with his shadow under
 * him; OBJMOVE's model 4 moves him. At demo frame 0x14E he walks 300 toward the camera's back
 * until 0x1BC, at 0x63A he stands at (-40, 3, -57) in his second animation, at 0xA7E he takes his
 * first again and turns toward the old man. */
s32 omiDemoKuroProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    PersonW *w = (PersonW *) (obj + 1);
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "kuro";
            PRINTF("クロベー作ります\n");
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoAnimeSet(w->frame, (u32) omiDemoKuroAnim, 0xBF, 0);
            demoPolSetDL(w->shadow, (Gfx *) omiDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.06f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->frame->scale[2] = 0.097f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = -15.0f;
            w->frame->pos[1] = 3.0f;
            w->frame->pos[2] = 76.0f;
            w->frame->rot[1] = -3.141592741f;
            PRINTF("クロベー作りました\n");
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            frmDestroy((Frame *) w->shadow);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, 0x80);
            break;
        case 0:
            w->frame->f.flags |= 2;
            if (gObjMoveSel == 4) {
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
            w->shadow->pos[0] = w->frame->pos[0];
            w->shadow->pos[2] = w->frame->pos[2];
            w->shadow->pos[1] = 0.3f;
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->frame, 0.3f, 0);
                    if (omiDemoFrame == 0x14E) {
                        w->state++;
                    }
                    break;
                case 1:
                    demoWalkTo(w->frame, 0.84f, 0.72f, w->frame->pos[0], w->frame->pos[2] - 300.0f);
                    if (omiDemoFrame == 0x1BC) {
                        w->state++;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->frame, 0.2f, 0);
                    if (omiDemoFrame == 0x63A) {
                        demoAnimeSet(w->frame, (u32) omiDemoKuroAnim2, 0xBF, 0);
                        w->state++;
                        w->frame->pos[0] = -40.0f;
                        w->frame->pos[1] = 3.0f;
                        w->frame->pos[2] = -57.0f;
                        w->frame->rot[1] = 0.3316125572f;
                    }
                    break;
                case 3:
                    demoAnimeStep(w->frame, 0.2f, 0);
                    if (omiDemoFrame == 0xA7E) {
                        demoAnimeSet(w->frame, (u32) omiDemoKuroState3Anim, 0xBF, 0);
                        w->state++;
                    }
                    break;
                case 4:
                    demoAnimeStep(w->frame, 0.2f, 0);
                    w->frame->rot[1] = demoTurnToward(w->frame->pos[0], w->frame->pos[2], omiDemoJijiX,
                                                      omiDemoJijiZ, w->frame->rot[1], 1.0f);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *omiDemoKuroCreate(Obj *obj) {
    return tskCreate(obj, omiDemoKuroProc, obj->file, NULL);
}

/* "kuro" (Kurobei, "クロベー作ります"): stands at (-26, 3, -15) with his shadow under him;
 * OBJMOVE's model 4 moves him. At demo frame 0x29E he finishes his animation, starts the second
 * and scene run 0x80, then turns toward Omitchan, animating twice as fast from demo frame 0x33F. */
s32 omiDemoShortKuroProc(Obj *obj, s16 msg, void *arg) {
    static s32 omiDemoKurobeiShadowAlpha; /* .bss 0x08009658: his shadow's alpha */
    static s32 reserved0; /* .bss 0x0800965C-0x0800966C: five words nothing reads */
    static s32 reserved1;
    static s32 reserved2;
    static s32 reserved3;
    static s32 reserved4;
    s32 pad;
    PersonW *w = (PersonW *) (obj + 1);
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "kuro";
            PRINTF("クロベー作ります\n");
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoAnimeSet(w->frame, (u32) omiDemoKuroState3Anim, 0xBF, 0);
            demoPolSetDL(w->shadow, (Gfx *) omiDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.08f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->frame->scale[2] = 0.097f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = -26.0f;
            w->frame->pos[1] = 3.0f;
            w->frame->pos[2] = -15.0f;
            w->frame->rot[1] = 3.682644844f;
            PRINTF("クロベー作りました\n");
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            frmDestroy((Frame *) w->shadow);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, omiDemoKurobeiShadowAlpha);
            break;
        case 0:
            w->frame->f.flags |= 2;
            if (gObjMoveSel == 4) {
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
            w->shadow->pos[0] = w->frame->pos[0];
            w->shadow->pos[2] = w->frame->pos[2];
            w->shadow->pos[1] = 0.3f;
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->frame, 0.3f, 0);
                    if (omiDemoFrame == 0x29E) {
                        w->state++;
                    }
                    break;
                case 1:
                    if (demoAnimeStep(w->frame, 0.3f, 2)) {
                        demoAnimeSet(w->frame, (u32) omiDemoKuroAnim2, 0xBF, 0);
                        scRunEnd();
                        scRunStart(0x80, 0, 0);
                        w->state++;
                    }
                    w->frame->rot[1] = demoTurnToward(w->frame->pos[0], w->frame->pos[2], omiDemoOmiX,
                                                      omiDemoOmiZ, w->frame->rot[1], 1.0f);
                    break;
                case 2:
                    w->frame->rot[1] = demoTurnToward(w->frame->pos[0], w->frame->pos[2], omiDemoOmiX,
                                                      omiDemoOmiZ, w->frame->rot[1], 1.0f);
                    demoAnimeStep(w->frame, 0.3f, 0);
                    if (omiDemoFrame >= 0x33F) {
                        demoAnimeStep(w->frame, 0.3f, 0);
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *omiDemoShortKuroCreate(Obj *obj) {
    return tskCreate(obj, omiDemoShortKuroProc, obj->file, NULL);
}

/* The second "omi" (Omitchan, "おみっちゃん作ります"): stands at (3, -1, -64) with her shadow;
 * OBJMOVE's model 3 moves her. She sways her head until demo frame 0xCC, then faces a point 6 to
 * the side of another person, finishes her animation at 0x3B0 (scene run 0x76), faces them until
 * 0x554, plays her second animation, stands at (24, -1, -46) at 0x63A, turns toward the old man at
 * 0xA7E and stands at (61, -1, 154) at 0xF24. */
s32 omiDemoOmiProc(Obj *obj, s16 msg, void *arg) {
    OmiW *w = (OmiW *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "omi";
            PRINTF("おみっちゃん作ります\n");
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoAnimeSet(w->frame, (u32) omiDemoOmiState4Anim, 0xC7, 0);
            demoPolSetDL(w->shadow, (Gfx *) omiDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.065f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->frame->scale[2] = 0.097f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = 3.0f;
            w->frame->pos[1] = -1.0f;
            w->frame->pos[2] = -64.0f;
            w->frame->rot[1] = 0.0f;
            fadeAnimStart(&w->anim, omiDemoOmi2FaceAnims, 0, &w->frame->segs[3], floadGetSetFile(0xC7, 0));
            PRINTF("おみっちゃん作りました\n");
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            frmDestroy((Frame *) w->shadow);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, 0x80);
            break;
        case 0:
            w->frame->f.flags |= 2;
            if (gObjMoveSel == 3) {
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
            w->shadow->pos[0] = w->frame->pos[0];
            w->shadow->pos[2] = w->frame->pos[2];
            w->shadow->pos[1] = 0.3f;
            switch (w->state) {
                case 0:
                    w->state++;
                    break;
                case 1:
                    demoAnimeStep(w->frame, 0.1f, 0);
                    if (omiDemoFrame < 0xCC) {
                        w->frame->rot[1] = sinf(omiDemoFrame * 0.0174532930038800785) * 20.0f * 0.0174532930038800785;
                    } else {
                        w->frame->rot[1] = demoTurnToward(w->frame->pos[0], w->frame->pos[2], omiDemoGoe1X + 6.0f,
                                                          omiDemoGoe1Z, w->frame->rot[1], 1.0f);
                    }
                    if (omiDemoFrame == 0x3B0) {
                        w->state++;
                    }
                    break;
                case 2:
                    if (demoAnimeStep(w->frame, 0.2f, 2)) {
                        w->state++;
                        demoAnimeSet(w->frame, (u32) omiDemoOmiState2Anim, 0xC7, 0);
                        fadeAnimStart(&w->anim, omiDemoOmiAnim, 0, &w->frame->segs[3], floadGetSetFile(0xC7, 0));
                        scRunEnd();
                        scRunStart(0x76, 0, 0);
                    }
                    break;
                case 3:
                    if (omiDemoFrame < 0x428) {
                        w->frame->rot[1] = demoTurnToward(w->frame->pos[0], w->frame->pos[2], omiDemoSasukeX,
                                                          omiDemoSasukeZ, w->frame->rot[1], 1.0f);
                    } else {
                        w->frame->rot[1] = demoTurnToward(w->frame->pos[0], w->frame->pos[2], omiDemoGoe1X + 6.0f,
                                                          omiDemoGoe1Z, w->frame->rot[1], 1.0f);
                    }
                    demoAnimeStep(w->frame, 0.2f, 0);
                    fadeAnimStep(&w->anim);
                    if (omiDemoFrame == 0x554) {
                        fadeAnimStart(&w->anim, omiDemoOmi2FaceAnims, 0, &w->frame->segs[3], floadGetSetFile(0xC7, 0));
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 4:
                    fadeAnimStep(&w->anim);
                    if (demoAnimeStep(w->frame, 0.2f, 3)) {
                        demoAnimeSet(w->frame, (u32) omiDemoOmiState4Anim, 0xC7, 0);
                        fadeAnimStart(&w->anim, omiDemoOmi2FaceAnims, 0, &w->frame->segs[3], floadGetSetFile(0xC7, 0));
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 5:
                    demoAnimeStep(w->frame, 0.2f, 0);
                    if (omiDemoFrame == 0x63A) {
                        w->state++;
                        demoAnimeSet(w->frame, (u32) omiDemoOmiState5Anim, 0xC7, 0);
                        fadeAnimStart(&w->anim, omiDemoOmi2FaceAnims, 0, &w->frame->segs[3], floadGetSetFile(0xC7, 0));
                        w->frame->pos[0] = 24.0f;
                        w->frame->pos[1] = -1.0f;
                        w->frame->pos[2] = -46.0f;
                        w->frame->rot[1] = 6.108652592f;
                    }
                    break;
                case 6:
                    demoAnimeStep(w->frame, 0.2f, 0);
                    if (omiDemoFrame == 0xA7E) {
                        w->state++;
                    }
                    break;
                case 7:
                    demoAnimeStep(w->frame, 0.2f, 0);
                    w->frame->rot[1] = demoTurnToward(w->frame->pos[0], w->frame->pos[2], omiDemoJijiX,
                                                      omiDemoJijiZ, w->frame->rot[1], 1.0f);
                    if (omiDemoFrame == 0xF24) {
                        w->frame->pos[0] = 61.0f;
                        w->frame->pos[1] = -1.0f;
                        w->frame->pos[2] = 154.0f;
                        w->frame->rot[1] = 3.455751896f;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *omiDemoOmiCreate(Obj *obj) {
    return tskCreate(obj, omiDemoOmiProc, obj->file, NULL);
}

/* "omi" (Omitchan, "おみっちゃん作ります"): stands at (-20, -1, -34) with her shadow under her;
 * OBJMOVE's model 3 moves her. At demo frame 0x1E0 she finishes her animation and starts the
 * second (sound 0x457). Publishes her x and z each frame. */
s32 omiDemoShortOmiProc(Obj *obj, s16 msg, void *arg) {
    static s32 omiDemoOmiShadowAlpha; /* .bss 0x08009670: her shadow's alpha */
    static s32 secondAnimWord;        /* .bss 0x08009674: zeroed when she starts her second animation */
    static s32 reserved;              /* .bss 0x08009678: a word nothing reads */
    OmiW *w = (OmiW *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "omi";
            PRINTF("おみっちゃん作ります\n");
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoAnimeSet(w->frame, (u32) omiDemoOmiState5Anim, 0xC7, 0);
            demoPolSetDL(w->shadow, (Gfx *) omiDemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.08f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->frame->scale[2] = 0.097f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = -20.0f;
            w->frame->pos[1] = -1.0f;
            w->frame->pos[2] = -34.0f;
            w->frame->rot[1] = 3.176499367f;
            fadeAnimStart(&w->anim, omiDemoOmiTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xC7, 0));
            PRINTF("おみっちゃん作りました\n");
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            frmDestroy((Frame *) w->shadow);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, omiDemoOmiShadowAlpha);
            break;
        case 0:
            w->frame->f.flags |= 2;
            if (gObjMoveSel == 3) {
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
            w->shadow->pos[0] = w->frame->pos[0];
            w->shadow->pos[2] = w->frame->pos[2];
            w->shadow->pos[1] = 0.3f;
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->frame, 0.1f, 0);
                    if (omiDemoFrame == 0x1E0) {
                        w->state++;
                        secondAnimWord = 0;
                    }
                    break;
                case 1:
                    if (demoAnimeStep(w->frame, 0.2f, 2)) {
                        w->state++;
                        demoAnimeSet(w->frame, (u32) omiDemoShortOmiAnim, 0xC7, 0);
                        fadeAnimStart(&w->anim, omiDemoOmiTexAnim, 1, &w->frame->segs[3], floadGetSetFile(0xC7, 0));
                        sndPlay(0x457);
                    }
                    break;
                case 2:
                    fadeAnimStep(&w->anim);
                    demoAnimeStep(w->frame, 0.15f, 2);
                    break;
            }
            break;
    }
    omiDemoOmiX = w->frame->pos[0];
    omiDemoOmiZ = w->frame->pos[2];
    return tskDefaultProc(obj, msg, arg);
}

Obj *omiDemoShortOmiCreate(Obj *obj) {
    return tskCreate(obj, omiDemoShortOmiProc, obj->file, NULL);
}

/* "kiran"'s (omiDemoKiranProc) argument: only the position its starter sets. */
typedef struct KiranArg {
    /* 0x00 */ u8 pad00[0x8];
    /* 0x08 */ f32 pos[3];
    /* 0x14 */ u8 pad14[0x8];
} KiranArg;

/* "kiran"'s work: a sparkle and its glow. */
typedef struct KiranW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ PolFrame *frame2; /* drawn at a sixth of the alpha, a quarter of the width */
    /* 0x08 */ u8 pad08[0xC];
    /* 0x14 */ s32 alpha;
    /* 0x18 */ s32 state; /* 0 brightens fast, 1 slowly, 2 fades */
} KiranW;

/* "kiran": a sparkle at the argument's position that stretches in x as it brightens to 255, then
 * fades, shrinking, and deletes itself; all go at demo frame 0x63A. */
s32 omiDemoKiranProc(Obj *obj, s16 msg, void *arg) {
    KiranW *w = (KiranW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "kiran";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame2 = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame2 == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, (Gfx *) omiDemoKiranDl, 0xC6);
            demoPolSetDL(w->frame2, (Gfx *) omiDemoKiranGlowDl, 0xC6);
            w->frame->flags |= 7;
            w->frame2->flags |= 7;
            w->frame->scale[2] = 0.07f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame2->scale[2] = 0.07f;
            w->frame2->scale[0] = w->frame2->scale[1] = w->frame2->scale[2];
            w->frame->pos[0] = ((KiranArg *) arg)->pos[0];
            w->frame->pos[1] = ((KiranArg *) arg)->pos[1];
            w->frame->pos[2] = ((KiranArg *) arg)->pos[2];
            w->frame2->pos[0] = ((KiranArg *) arg)->pos[0];
            w->frame2->pos[1] = ((KiranArg *) arg)->pos[1];
            w->frame2->pos[2] = ((KiranArg *) arg)->pos[2];
            w->alpha = 0x40;
            w->state = 0;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            frmDestroy((Frame *) w->frame2);
            break;
        case 3:
            w->frame->dl = demoColorDL(255, 255, 255, 222, 100, 100, w->alpha);
            w->frame2->dl = demoColorDL(255, 255, 255, 222, 100, 100, w->alpha / 6);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->alpha += 3;
                    if (w->alpha > 224) {
                        w->state = 1;
                        w->alpha = 224;
                    } else {
                        w->frame->scale[0] += 0.08;
                        w->frame->scale[1] = w->frame->scale[0];
                        w->frame2->scale[0] = w->frame->scale[0] / 4;
                        w->frame2->scale[1] = w->frame2->scale[0];
                    }
                    break;
                case 1:
                    w->alpha += 1;
                    if (w->alpha >= 0x100) {
                        w->alpha = 0xFF;
                        w->state = 2;
                    } else {
                        w->frame->scale[0] += 0.004;
                        w->frame->scale[1] = w->frame->scale[0];
                        w->frame2->scale[0] = w->frame->scale[0] / 4;
                        w->frame2->scale[1] = w->frame2->scale[0];
                    }
                    break;
                case 2:
                    w->alpha -= 1;
                    if (w->alpha <= 0) {
                        w->alpha = 0;
                        return -1;
                    }
                    if (w->frame->scale[0] > 0.01) {
                        w->frame->scale[0] -= 0.001;
                        w->frame->scale[1] = w->frame->scale[0];
                        w->frame2->scale[0] = w->frame->scale[0] / 4;
                        w->frame2->scale[1] = w->frame2->scale[0];
                    }
                    break;
            }
            if (omiDemoFrame == 0x63A) {
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "kiran" (sparkle) at (-4, 23, -82) under obj. */
void omiDemoKiranCreate(Obj *obj) {
    KiranArg arg;

    arg.pos[0] = -4.0f;
    arg.pos[1] = 23.0f;
    arg.pos[2] = -82.0f;
    tskCreate(obj, omiDemoKiranProc, obj->file, &arg);
}

/* "MACHINE"'s work: its anime model, and its speed toward the camera. */
typedef struct MachineW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 animSpeed;
    /* 0x08 */ f32 speed;
} MachineW;

/* "MACHINE": comes in from z 1212 to 260, slowing by 0.94 a frame past it; OBJMOVE's model 6
 * moves it (blinking while chosen). Publishes its x and z each frame. */
s32 omiDemoMachineProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    MachineW *w = (MachineW *) (obj + 1);
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "MACHINE";
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            demoAnimeSet(w->frame, (u32) omiDemoMachineAnim, 0x13A, 0);
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 300.0f;
            w->frame->pos[2] = 1212.0f;
            w->frame->scale[0] = 1.6f;
            w->frame->scale[1] = 1.6f;
            w->frame->scale[2] = 1.6f;
            w->frame->rot[1] = 3.1415927f;
            w->frame->rot[0] = -1.62315619f;
            w->speed = (240.0f - w->frame->pos[2]) / 260.0f;
            w->animSpeed = 0.3f;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 3:
            w->frame->dl = demoShadeDL();
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
            if (w->frame->pos[2] < 260.0f) {
                w->speed *= 0.94;
            }
            w->frame->pos[2] += w->speed;
            omiDemoMachineX = w->frame->pos[0];
            omiDemoMachineZ = w->frame->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 omiDemoMachineProc(Obj *obj, s16 msg, void *arg);

Obj *omiDemoMachineCreate(Obj *obj) {
    return tskCreate(obj, omiDemoMachineProc, obj->file, NULL);
}

/* "beam"'s work: two beam columns and two rings. */
typedef struct BeamW {
    /* 0x00 */ PolFrame *frames[4]; /* the beams (0, 1) and the rings (2, 3) */
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ f32 scale;
    /* 0x18 */ s32 timer;
} BeamW;

extern u8 omiDemoBeam1Dl[];
extern u8 omiDemoBeam2Dl[];
extern u8 omiDemoBeamSparkleDl[];
extern Gfx omiDemoBeamDl[]; /* the beams' display lists */
extern Gfx omiDemoBeamFrame1Dl[];

u32 omiDemoUnused2 = 0x3F800000;
AnimFrame omiDemoOmi2FaceAnims[] = {
    { (u32) omiDemoOmiFaceTex, 100 },
    { 0x00000000, 0 },
}; /* the second Omitchan's face animations */
AnimFrame omiDemoOmiTexAnim[] = {
    { (u32) omiDemoOmiFaceTex, 1 },
    { 0x03007280, 7 },
    { 0x03006200, 1000 },
    { 0x00000000, 0 },
}; /* Omitchan's texture animation */
AnimFrame omiDemoOmiAnim[] = { { (u32) omiDemoOmiFaceTex, 5 }, { 0x030059C0, 5 }, { 0x03005DE0, 7 }, { 0x03007280, 2 }, { 0x030076A0, 2 }, { 0x03007280, 2 }, { 0x030059C0, 5 }, { (u32) omiDemoOmiFaceTex, 5 }, { (u32) omiDemoOmiFaceTex, 2 }, { 0x030059C0, 3 }, { 0x03005DE0, 4 }, { 0x030059C0, 2 }, { (u32) omiDemoOmiFaceTex, 3 }, { 0x03007280, 2 }, { 0x030076A0, 2 }, { 0x03007280, 2 }, { (u32) omiDemoOmiFaceTex, 2 }, { 0x030059C0, 3 }, { 0x03005DE0, 4 }, { 0x030059C0, 2 }, { (u32) omiDemoOmiFaceTex, 3 }, { (u32) omiDemoOmiFaceTex, 5 }, { 0x030059C0, 5 }, { 0x03005DE0, 7 }, { 0x030059C0, 5 }, { (u32) omiDemoOmiFaceTex, 5 }, { (u32) omiDemoOmiFaceTex, 2 }, { (u32) omiDemoOmiFaceTex, 5 }, { 0x030059C0, 5 }, { 0x03005DE0, 7 }, { 0x030059C0, 5 }, { (u32) omiDemoOmiFaceTex, 5 }, { (u32) omiDemoOmiFaceTex, 2 }, { 0x00000000, 0 } };
Gfx omiDemoBeamDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetEnvColor(0xFF, 0xFF, 0xFF, 0xA0),
    gsDPSetCombineLERP(0, 0, 0, SHADE, SHADE, 0, ENVIRONMENT, 0, 0, 0, 0, SHADE, SHADE, 0, ENVIRONMENT, 0),
    gsSPEndDisplayList(),
}; /* the beams' display lists */
Gfx omiDemoBeamFrame1Dl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetEnvColor(0xFF, 0xFF, 0xFF, 0x30),
    gsDPSetCombineLERP(0, 0, 0, SHADE, SHADE, 0, ENVIRONMENT, 0, 0, 0, 0, SHADE, SHADE, 0, ENVIRONMENT, 0),
    gsSPEndDisplayList(),
};

/* "beam": a beam of light at the machine's x and z, 270 high, that widens for 60 frames while its
 * rings pulse; at demo frame 0xA7E it stands at (0, -26) and at 0xB0A it narrows and deletes
 * itself. */
s32 omiDemoBeamProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    BeamW *w = (BeamW *) (obj + 1);
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "beam";
            w->frames[0] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frames[0] == NULL) {
                return -1;
            }
            w->frames[1] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frames[1] == NULL) {
                return -1;
            }
            w->frames[2] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frames[2] == NULL) {
                return -1;
            }
            w->frames[3] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frames[3] == NULL) {
                return -1;
            }
            w->frames[0]->dl2 = (Gfx *) omiDemoBeam1Dl;
            floadGetSetRam((void **) w->frames[0]->segs, 0x13B);
            w->frames[1]->dl2 = (Gfx *) omiDemoBeam2Dl;
            floadGetSetRam((void **) w->frames[0]->segs, 0x13B);
            demoPolSetDL(w->frames[2], (Gfx *) omiDemoBeamSparkleDl, 0x120);
            demoPolSetDL(w->frames[3], (Gfx *) omiDemoBeamSparkleDl, 0x120);
            w->frames[0]->scale[0] = w->frames[0]->scale[2] = w->scale = 0.01f;
            w->frames[1]->scale[0] = w->frames[1]->scale[2] = w->scale;
            w->frames[0]->scale[1] = 1.0f;
            w->frames[1]->scale[1] = 2.0f;
            w->frames[0]->pos[0] = omiDemoMachineX;
            w->frames[0]->pos[1] = 270.0f;
            w->frames[0]->pos[2] = omiDemoMachineZ;
            w->frames[1]->pos[0] = w->frames[0]->pos[0];
            w->frames[1]->pos[1] = w->frames[0]->pos[1];
            w->frames[1]->pos[2] = w->frames[0]->pos[2];
            w->frames[2]->pos[0] = w->frames[3]->pos[0] = w->frames[0]->pos[0];
            w->frames[2]->pos[2] = w->frames[3]->pos[2] = w->frames[0]->pos[2];
            w->frames[3]->pos[1] = 270.0f;
            w->frames[2]->pos[1] = w->frames[3]->pos[1];
            w->frames[0]->dl = (Gfx *) fadeSegToRam(omiDemoBeamDl, obj->file);
            w->frames[1]->dl = (Gfx *) fadeSegToRam(omiDemoBeamFrame1Dl, obj->file);
            w->timer = 0;
            break;
        case 3:
            w->frames[2]->dl = demoColorDL(255, 255, 255, 0, 200, 80, 160);
            w->frames[3]->dl = demoColorDL(255, 255, 255, 0, 60, 100, 160);
            break;
        case 1:
            frmDestroy((Frame *) w->frames[0]);
            frmDestroy((Frame *) w->frames[1]);
            frmDestroy((Frame *) w->frames[2]);
            frmDestroy((Frame *) w->frames[3]);
            break;
        case 0:
            w->timer++;
            if (omiDemoFrame < 0xA7E) {
                if (w->timer < 60) {
                    w->scale += 0.002;
                }
                w->frames[0]->scale[0] = w->frames[0]->scale[2] = w->scale * 0.2f;
                w->frames[1]->scale[0] = w->frames[1]->scale[2] = w->scale * 0.2f;
                w->frames[2]->scale[0] = w->frames[2]->scale[1] =
                    sinf(omiDemoFrame * 10 * 0.0174532930038800785) * 0.3 + w->scale * 3.0f;
                w->frames[3]->scale[0] = w->frames[3]->scale[1] =
                    sinf((omiDemoFrame * 10 + 180) * 0.0174532930038800785) * 0.3 + w->scale * 3.0f;
                w->frames[0]->scale[1] = 4.0f;
                w->frames[1]->scale[1] = 4.0f;
                w->frames[2]->scale[2] = 1.0f;
                w->frames[3]->scale[2] = 1.0f;
                w->frames[0]->rot[1] += 0.0174532930038800785;
                w->frames[1]->rot[1] += 0.0174532930038800785;
                w->frames[2]->rot[0] = 1.570796371f;
                w->frames[3]->rot[0] = 1.570796371f;
                w->frames[0]->rot[1] += 0.034906586007760157;
                w->frames[1]->rot[1] += 0.034906586007760157;
            } else if (omiDemoFrame < 0xB0A) {
                w->frames[1]->pos[0] = 0.0f;
                w->frames[0]->pos[0] = w->frames[1]->pos[0];
                w->frames[1]->pos[2] = -26.0f;
                w->frames[0]->pos[2] = w->frames[1]->pos[2];
                w->frames[0]->scale[0] = w->frames[0]->scale[2] = w->scale * 0.3f;
                w->frames[1]->scale[0] = w->frames[1]->scale[2] = w->scale * 0.3f;
                w->frames[0]->scale[1] = 4.0f;
                w->frames[1]->scale[1] = 4.0f;
                w->frames[0]->rot[1] += 0.0174532930038800785;
                w->frames[1]->rot[1] += 0.0174532930038800785;
                w->frames[0]->rot[1] += 0.034906586007760157;
                w->frames[1]->rot[1] += 0.034906586007760157;
            } else {
                w->frames[0]->scale[0] *= 0.97;
                w->frames[0]->scale[2] *= 0.97;
                w->frames[1]->scale[0] *= 0.97;
                w->frames[1]->scale[2] *= 0.97;
                w->frames[0]->rot[1] += 0.034906586007760157;
                w->frames[1]->rot[1] += 0.034906586007760157;
                if (w->frames[0]->scale[0] < 0.0001) {
                    return -1;
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *omiDemoBeamCreate(Obj *obj) {
    return tskCreate(obj, omiDemoBeamProc, obj->file, NULL);
}
