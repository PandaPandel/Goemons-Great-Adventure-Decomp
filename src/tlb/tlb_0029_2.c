#include "tlb.h"
#include "tlb/tlb_0029.h"

#define DPRINTF

/* Main's sinf and libultra's cosf, called through their addresses as TLB code calls main. */
#define sinf (*sinf)
#define cosf (*cosf)

#define ABSF(x) ((x) > 0.0f ? (x) : -(x))

extern u8 gDemoCamHold;
extern s32 gObjMoveSel;  /* OBJMOVE: the model chosen, -1 for none */
extern f32 gObjMovePosX; /* the chosen model's position and angle, as shown */
extern f32 gObjMovePosY;
extern f32 gObjMovePosZ;
extern f32 gObjMoveRot;
extern f32 gObjMoveDx; /* the moves this frame: x, y, z, angle */
extern f32 gObjMoveDy;
extern f32 gObjMoveDz;
extern f32 gObjMoveDrot;
extern s32 mach2DemoFrame; /* the file's frame counter */
extern Gfx mach2DemoShadowDl[];         /* a player's shadow's display list */

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

extern u8 mach2DemoSmokeTex0[];
extern u8 mach2DemoSmokeTex1[];
extern u8 mach2DemoSmokeTex2[];
extern u8 mach2DemoSmokeTex3[];
extern u8 mach2DemoSmokeTex4[];
extern u8 mach2DemoSmokeTex6[];
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
/* This file's .data (0x08004C10), in address order: the characters' texture animations
 * (demotool.c's table, as tlb_0020_1's; nothing reads it), Sasuke's x and z, then the others'. */
AnimFrame mach2DemoCharaAnims[] = {
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
f32 mach2DemoSasX = 1.0f; /* Sasuke's x and z, each frame */
f32 mach2DemoSasZ = 1.0f;
f32 mach2DemoGoeX = 1.0f; /* Goemon's x and z, each frame */
f32 mach2DemoGoeZ = 1.0f;

/* "genoGOE" (Goemon, character 0, in the demo) at (10, 0, 56) under the menu's last task;
 * OBJMOVE's model 0 moves him. His steps follow the file's frame counter: five runs in and back
 * (actions 0xEB/0xEC in, 0xEB/0xE9 back) from 0x3F, 0x5EB, 0x812, 0xB85 and 0xFD2, each run in
 * starting a scene (0x118, 0x11B, 0x11D, 0x11F, 0x122) that ends before he runs back.
 * The run's step and his player are statics of this function, in this file's .bss
 * (0x08005C20, 0x08005C28): IDO forms their addresses afresh at each use, and a static whose
 * address is never taken does not alias w->pl, so plEnterDemo gets plCreate's v0 as in the
 * game. The whole .bss (the three characters' statics) is this file's. */
s32 mach2DemoGoeProc(Obj *obj, s16 msg, void *arg) {
    static s32 mach2DemoGoeRunStep;     /* the run's step (demoPlTunagiAnime) */
    static s32 mach2DemoGoeReserved;     /* reserved: the game's .bss has a word here that nothing reads */
    static Player *mach2DemoGoePlayer; /* his player */
    s32 pad[2];
    DemoPlW *w = (DemoPlW *) (obj + 1);
    PlInit init;

    switch (msg) {
        case 2:
            DPRINTF("goecreate\n");
            obj->name = "genoGOE";
            plMakeInit(&init, 0, 0);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = 10.0f;
            init.height = 0.0f;
            init.side = 56.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            mach2DemoGoePlayer = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            DPRINTF("歩きモーション\n");
            plAnimeSet(PL(w->pl), 0xE9);
            w->model = mach2DemoGoePlayer->model;
            DPRINTF("*** p->model_buff=%x\n", w->model);
            DPRINTF("ゴエモン作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, mach2DemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.08f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->chara = 0;
            w->model->rot[1] = 3.141592741f;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 10.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = 56.0f;
            w->model->rot[1] = 3.19395256f;
            demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
            demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
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
            mach2DemoGoeX = w->model->pos[0];
            mach2DemoGoeZ = w->model->pos[2];
            switch (w->state) {
                case 0:
                    w->model->scale[2] = 0.097f;
                    w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                    w->model->pos[0] = 10.0f;
                    w->model->pos[1] = 0.0f;
                    w->model->pos[2] = 56.0f;
                    w->model->rot[1] = 3.19395256f;
                    w->state++;
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.5f, 0);
                    w->state = 10;
                    break;
                case 10:
                    demoAnimeStep(w->model, 0.3f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (mach2DemoFrame == 0x3F) {
                        mach2DemoGoeRunStep = 0;
                        w->state++;
                    }
                    break;
                case 11:
                    mach2DemoGoeRunStep = demoPlTunagiAnime(obj, 3, mach2DemoGoeRunStep, 2, 2, 0.3f, 0.6f, 0xEB, 0xEC);
                    if (mach2DemoGoeRunStep == -1) {
                        mach2DemoGoeRunStep = 0;
                        w->state++;
                        mach2DemoScenarioStart(0x118);
                    }
                    break;
                case 12:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.6f, 0);
                    if (mach2DemoFrame == 0x11B) {
                        scRunEnd();
                        mach2DemoGoeRunStep = 0;
                        w->state++;
                    }
                    break;
                case 13:
                    mach2DemoGoeRunStep = demoPlTunagiAnime(obj, 3, mach2DemoGoeRunStep, 2, 3, 0.6f, 0.3f, 0xEB, 0xE9);
                    if (mach2DemoGoeRunStep == -1) {
                        mach2DemoGoeRunStep = 0;
                        w->state = 20;
                        demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 20:
                    demoAnimeStep(w->model, 0.3f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    w->model->rot[1] = 3.19395256f;
                    if (mach2DemoFrame == 0x5EB) {
                        mach2DemoGoeRunStep = 0;
                        w->state++;
                    }
                    break;
                case 21:
                    mach2DemoGoeRunStep = demoPlTunagiAnime(obj, 3, mach2DemoGoeRunStep, 2, 2, 0.3f, 0.5f, 0xEB, 0xEC);
                    if (mach2DemoGoeRunStep == -1) {
                        mach2DemoGoeRunStep = 0;
                        w->state++;
                        mach2DemoScenarioStart(0x11B);
                    }
                    break;
                case 22:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.5f, 0);
                    if (mach2DemoFrame == 0x6C7) {
                        scRunEnd();
                        mach2DemoGoeRunStep = 0;
                        w->state++;
                    }
                    break;
                case 23:
                    mach2DemoGoeRunStep = demoPlTunagiAnime(obj, 3, mach2DemoGoeRunStep, 2, 3, 0.5f, 0.3f, 0xEB, 0xE9);
                    if (mach2DemoGoeRunStep == -1) {
                        mach2DemoGoeRunStep = 0;
                        w->state = 30;
                        demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 30:
                    demoAnimeStep(w->model, 0.3f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (mach2DemoFrame == 0x812) {
                        mach2DemoGoeRunStep = 0;
                        mach2DemoScenarioStart(0x11D);
                        w->state++;
                    }
                    break;
                case 31:
                    mach2DemoGoeRunStep = demoPlTunagiAnime(obj, 2, mach2DemoGoeRunStep, 2, 2, 0.5f, 0.7f, 0xEB, 0xEC);
                    if (mach2DemoGoeRunStep == -1) {
                        mach2DemoGoeRunStep = 0;
                        w->state++;
                    }
                    break;
                case 32:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.7f, 0);
                    if (mach2DemoFrame == 0x860) {
                        scRunEnd();
                        mach2DemoGoeRunStep = 0;
                        w->state++;
                    }
                    break;
                case 33:
                    mach2DemoGoeRunStep = demoPlTunagiAnime(obj, 3, mach2DemoGoeRunStep, 2, 3, 0.5f, 0.5f, 0xEB, 0xE9);
                    if (mach2DemoGoeRunStep == -1) {
                        demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                        mach2DemoGoeRunStep = 0;
                        w->state = 40;
                    }
                    break;
                case 40:
                    demoAnimeStep(w->model, 0.3f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (mach2DemoFrame == 0x99B) {
                        demoTexAnimK0(&w->eyes, 7, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 7, &PL(w->pl)->model->segs[4]);
                    }
                    if (mach2DemoFrame == 0xAEF) {
                        demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                    }
                    if (mach2DemoFrame == 0xB85) {
                        mach2DemoGoeRunStep = 0;
                        w->state++;
                    }
                    break;
                case 41:
                    mach2DemoGoeRunStep = demoPlTunagiAnime(obj, 3, mach2DemoGoeRunStep, 2, 2, 0.3f, 0.6f, 0xEB, 0xEC);
                    if (mach2DemoGoeRunStep == -1) {
                        mach2DemoGoeRunStep = 0;
                        w->state++;
                        mach2DemoScenarioStart(0x11F);
                    }
                    break;
                case 42:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.6f, 0);
                    if (mach2DemoFrame == 0xC4D) {
                        scRunEnd();
                        mach2DemoGoeRunStep = 0;
                        w->state++;
                    }
                    break;
                case 43:
                    mach2DemoGoeRunStep = demoPlTunagiAnime(obj, 3, mach2DemoGoeRunStep, 2, 3, 0.6f, 0.4f, 0xEB, 0xE9);
                    if (mach2DemoGoeRunStep == -1) {
                        mach2DemoGoeRunStep = 0;
                        w->state = 50;
                        demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 50:
                    demoAnimeStep(w->model, 0.4f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (mach2DemoFrame == 0xFD2) {
                        mach2DemoGoeRunStep = 0;
                        w->state++;
                    }
                    break;
                case 51:
                    mach2DemoGoeRunStep = demoPlTunagiAnime(obj, 3, mach2DemoGoeRunStep, 2, 2, 0.4f, 0.6f, 0xEB, 0xEC);
                    if (mach2DemoGoeRunStep == -1) {
                        mach2DemoGoeRunStep = 0;
                        w->state++;
                        mach2DemoScenarioStart(0x122);
                    }
                    break;
                case 52:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.6f, 0);
                    if (mach2DemoFrame == 0x109A) {
                        scRunEnd();
                        mach2DemoGoeRunStep = 0;
                        w->state++;
                    }
                    break;
                case 53:
                    mach2DemoGoeRunStep = demoPlTunagiAnime(obj, 2, mach2DemoGoeRunStep, 2, 3, 0.6f, 0.3f, 0xEB, 0xE9);
                    if (mach2DemoGoeRunStep == -1) {
                        mach2DemoGoeRunStep = 0;
                        w->state = 60;
                        demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 60:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.3f, 0);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 mach2DemoGoeProc(Obj *obj, s16 msg, void *arg);

Obj *mach2DemoGoeCreate(Obj *obj) {
    return tskCreate(obj, mach2DemoGoeProc, obj->file, NULL);
}

f32 mach2DemoEbiX = 1.0f; /* Ebisumaru's x and z, each frame */
f32 mach2DemoEbiZ = 1.0f;

/* "genoGOE" (Ebisumaru, character 1, in the demo) at (25, 0, 60) under the menu's last task;
 * OBJMOVE's model 1 moves him. His steps follow the file's frame counter: a run in (actions
 * 0xEB/0xEC) from 0x72B and scene 0x11C when it ends, a run back from 0x7C1, a walk off from
 * 0x7D5 (sound 0x500 every 8 frames), then from 0x7FE a jump (sound 0x500 panned by his place,
 * sound 0x245 on each bounce), and at 0x99B he stands at (20, 0, 10).
 * The run's step is a static of this function (0x08005C2C, this file's .bss), so IDO forms
 * its address afresh at each use as the game does. */
s32 mach2DemoEbiProc(Obj *obj, s16 msg, void *arg) {
    static s32 mach2DemoEbiRunStep; /* the run's step (demoPlTunagiAnime) */
    static s32 mach2DemoEbiReserved[3]; /* reserved: 12 bytes of the game's .bss that nothing reads */
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
            init.place = 25.0f;
            init.height = 0.0f;
            init.side = 60.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            DPRINTF("歩きモーション\n");
            plAnimeSet(PL(w->pl), 0xE9);
            w->model = p->model;
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, mach2DemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.09f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 1;
            w->state = 0;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 25.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = 60.0f;
            w->model->rot[1] = 3.577924967f;
            demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
            demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
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
            mach2DemoEbiX = w->model->pos[0];
            mach2DemoEbiZ = w->model->pos[2];
            switch (w->state) {
                case 0:
                    w->model->pos[0] = 25.0f;
                    w->model->pos[1] = 0.0f;
                    w->model->pos[2] = 60.0f;
                    w->model->rot[1] = 3.577924967f;
                    scRunEnd();
                    w->state = 10;
                    break;
                case 10:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (mach2DemoFrame == 0x72B) {
                        mach2DemoEbiRunStep = 0;
                        w->state++;
                    }
                    break;
                case 11:
                    mach2DemoEbiRunStep = demoPlTunagiAnime(obj, 2, mach2DemoEbiRunStep, 2, 2, 0.2f, 0.7f, 0xEB, 0xEC);
                    if (mach2DemoEbiRunStep == -1) {
                        mach2DemoEbiRunStep = 0;
                        w->state++;
                        mach2DemoScenarioStart(0x11C);
                    }
                    break;
                case 12:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.7f, 0);
                    if (mach2DemoFrame >= 0x7C1) {
                        scRunEnd();
                        mach2DemoEbiRunStep = 0;
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 13:
                    demoAnimeStep(w->model, 0.7f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    mach2DemoEbiRunStep = demoPlTunagiAnime(obj, 2, mach2DemoEbiRunStep, 2, 3, 0.7f, 0.7f, 0xEB, 0xE9);
                    if (mach2DemoEbiRunStep == -1) {
                        plAnimeSet(PL(w->pl), 0xDA);
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        mach2DemoEbiRunStep = 0;
                        w->state = 20;
                    }
                    break;
                case 20:
                    demoAnimeStep(w->model, 0.7f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (mach2DemoFrame >= 0x7D5) {
                        w->model->pos[0] -= 0.125;
                        w->model->pos[2] -= 0.25;
                        if (!(mach2DemoFrame & 7)) {
                            sndPlay(0x500);
                        }
                    }
                    if (mach2DemoFrame >= 0x7FD) {
                        w->state = 30;
                    }
                    break;
                case 30:
                    demoAnimeStep(w->model, 0.6f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (mach2DemoFrame == 0x7FE) {
                        w->model->pos[0] = 20.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 30.0f;
                        w->model->rot[1] = 3.577924967f;
                    }
                    if (mach2DemoFrame >= 0x7FE && mach2DemoFrame < 0x83B) {
                        w->model->pos[0] += -0.333333333333333315;
                        w->model->pos[2] += -1.25;
                        if (!(mach2DemoFrame & 3)) {
                            sndRequest(0x500, w->model->pos[0] * 2 + 64.0f, w->model->pos[2] / 2 + 235.0f);
                        }
                    }
                    if (mach2DemoFrame >= 0x83A && mach2DemoFrame < 0x845) {
                        w->model->rot[1] += 0.270526041560141262;
                    }
                    if (mach2DemoFrame == 0x842) {
                        plAnimeSet(PL(w->pl), 0x14);
                    }
                    if (mach2DemoFrame >= 0x842 && mach2DemoFrame < 0x870) {
                        w->model->pos[2] += -0.555555582f;
                    }
                    if (mach2DemoFrame >= 0x842 && mach2DemoFrame < 0x861) {
                        w->model->pos[1] = sinf((mach2DemoFrame - 0x842) * 0.0523583333333333331) * 70.0f;
                    }
                    if (mach2DemoFrame >= 0x860 && mach2DemoFrame < 0x870) {
                        w->model->pos[1] =
                            cosf((mach2DemoFrame - 0x860) * 0.104716666666666666) * 18.0f + 52.0f;
                    }
                    if (mach2DemoFrame >= 0x870 && mach2DemoFrame < 0x88E) {
                        w->model->pos[1] =
                            sinf((mach2DemoFrame - 0x86F) * 3.14150000000000018 / 30.0) * 18.0f + 52.0f;
                    }
                    if (mach2DemoFrame == 0x86F) {
                        sndPlay(0x245);
                    }
                    if (mach2DemoFrame >= 0x88D) {
                        w->model->pos[1] =
                            ABSF(sinf((mach2DemoFrame - 0x88D) * 3.14150000000000018 / 20.0) * 12.0f) + 52.0f;
                        if ((mach2DemoFrame - 0x851) % 20 == 0 && mach2DemoFrame < 0x937) {
                            sndPlay(0x245);
                        }
                    }
                    if (mach2DemoFrame >= 0x996) {
                        w->state = 40;
                    }
                    break;
                case 40:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (mach2DemoFrame == 0x99B) {
                        plAnimeSet(PL(w->pl), 0xD7);
                        w->model->pos[0] = 20.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 10.0f;
                        w->model->rot[1] = 0.0f;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 mach2DemoEbiProc(Obj *obj, s16 msg, void *arg);

Obj *mach2DemoEbiCreate(Obj *obj) {
    return tskCreate(obj, mach2DemoEbiProc, obj->file, NULL);
}


/* "chaSAS" (Sasuke, character 2, in the demo) at (2, 0, 50) under the menu's last task;
 * OBJMOVE's model 2 moves him. His steps follow the file's frame counter: scene 0x119 at
 * 0x27A, a run in (actions 0xEB/0xEC) from 0x9F5 and scene 0x11E when it ends, the scene ends
 * at 0xABD, a run back (0xEB/0xE9).
 * The run's step is a static of this function (0x08005C3C, this file's .bss): IDO then forms
 * its address afresh at each use, as the game does (tlb_0027_1's chaSAS, dochukiDemoSasProc,
 * the same). */
s32 mach2DemoSasProc(Obj *obj, s16 msg, void *arg) {
    static s32 mach2DemoSasRunStep; /* the run's step (demoPlTunagiAnime) */
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
            init.place = 2.0f;
            init.height = 0.0f;
            init.side = 50.0f;
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
            demoPolSetDL(w->shadow, mach2DemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.065f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 2;
            w->state = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 2.0f;
            w->model->pos[1] = 2.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = 50.0f;
            w->model->rot[1] = 2.967059851f;
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
            mach2DemoSasX = w->model->pos[0];
            mach2DemoSasZ = w->model->pos[2];
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = 0.3f;
            switch (w->state) {
                case 0:
                    w->model->pos[0] = 2.0f;
                    w->model->pos[1] = 0.0f;
                    w->model->pos[2] = 50.0f;
                    w->model->rot[1] = 2.967059851f;
                    demoTexAnimK2(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                    demoTexAnimK2(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    w->state = 10;
                    break;
                case 10:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (mach2DemoFrame == 0x27A) {
                        mach2DemoScenarioStart(0x119);
                        w->state = 20;
                    }
                    break;
                case 20:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (mach2DemoFrame == 0x99B) {
                        demoTexAnimK2(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                    }
                    if (mach2DemoFrame == 0x9F5) {
                        mach2DemoSasRunStep = 0;
                        w->state++;
                    }
                    break;
                case 21:
                    mach2DemoSasRunStep = demoPlTunagiAnime(obj, 4, mach2DemoSasRunStep, 2, 2, 0.4f, 0.7f, 0xEB, 0xEC);
                    if (mach2DemoSasRunStep == -1) {
                        mach2DemoSasRunStep = 0;
                        w->state++;
                        mach2DemoScenarioStart(0x11E);
                    }
                    break;
                case 22:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.7f, 0);
                    if (mach2DemoFrame == 0xABD) {
                        scRunEnd();
                        mach2DemoSasRunStep = 0;
                        w->state++;
                    }
                    break;
                case 23:
                    mach2DemoSasRunStep = demoPlTunagiAnime(obj, 4, mach2DemoSasRunStep, 2, 3, 0.7f, 0.4f, 0xEB, 0xE9);
                    if (mach2DemoSasRunStep == -1) {
                        mach2DemoSasRunStep = 0;
                        w->state = 30;
                        demoTexAnimK2(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 30:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (mach2DemoFrame == 0xAF9) {
                        demoTexAnimK2(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    demoAnimeStep(w->model, 0.4f, 0);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 mach2DemoSasProc(Obj *obj, s16 msg, void *arg);

Obj *mach2DemoSasCreate(Obj *obj) {
    return tskCreate(obj, mach2DemoSasProc, obj->file, NULL);
}

f32 mach2DemoYaeX = 1.0f; /* Yae's x and z, each frame */
f32 mach2DemoYaeZ = 1.0f;
u32 mach2DemoUnused2[] = { 0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000 };

/* "chaYAE": Yae (character 3) in the demo at (-7, 0, 56) under the menu's last task; OBJMOVE's
 * model 3 moves her. She stands (animation 0xD7) and her face follows the file's frame counter:
 * scene 0x11A at 0x41E, the scene ends at 0x5AE; animation 0x101 and scene 0x121 at 0xE56, back
 * to 0xD7 at 0xFC8. */
s32 mach2DemoYaeProc(Obj *obj, s16 msg, void *arg) {
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
            init.place = -7.0f;
            init.height = 0.0f;
            init.side = 56.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            DPRINTF("立ちモーション\n");
            plAnimeSet(PL(w->pl), 0xD7);
            w->model = p->model;
            DPRINTF("ヤエ作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, mach2DemoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.065f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 3;
            w->state = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -7.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = 56.0f;
            w->model->rot[1] = 2.740166903f;
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
            mach2DemoYaeX = w->model->pos[0];
            mach2DemoYaeZ = w->model->pos[2];
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = 0.3f;
            switch (w->state) {
                case 0:
                    w->model->pos[0] = -7.0f;
                    w->model->pos[1] = 0.0f;
                    w->model->pos[2] = 56.0f;
                    w->model->rot[1] = 2.740166903f;
                    demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                    demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    w->state = 10;
                    break;
                case 10:
                    demoAnimeStep(w->model, 0.4f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (mach2DemoFrame == 0x41E) {
                        mach2DemoScenarioStart(0x11A);
                    }
                    if (mach2DemoFrame == 0x5AE) {
                        scRunEnd();
                        w->state = 20;
                    }
                    break;
                case 20:
                    demoAnimeStep(w->model, 0.4f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (mach2DemoFrame == 0x99B) {
                        demoTexAnimK3(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                    }
                    if (mach2DemoFrame == 0xAF4) {
                        demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    if (mach2DemoFrame == 0xE56) {
                        plAnimeSet(PL(w->pl), 0x101);
                        demoTexAnimK3(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                        mach2DemoScenarioStart(0x121);
                    }
                    if (mach2DemoFrame == 0xFC8) {
                        plAnimeSet(PL(w->pl), 0xD7);
                        demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        scRunEnd();
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *mach2DemoYaeCreate(Obj *obj) {
    return tskCreate(obj, mach2DemoYaeProc, obj->file, NULL);
}

extern u8 mach2DemoMachineAnim[]; /* the machine's animation */

/* "MACH2"'s work: its anime model and its animation speed. */
typedef struct Mach2W {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 speed;
} Mach2W;

/* "MACH2": the machine (anime of file 0xC2) at (0, 10, -80), scale 0.7, turned half round;
 * OBJMOVE's model 4 moves it. Unless the demo camera holds, its turn is reset to 0 each frame. */
s32 mach2DemoMachineProc(Obj *obj, s16 msg, void *arg) {
    Mach2W *w = (Mach2W *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "MACH2";
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            demoAnimeSet(w->frame, (u32) mach2DemoMachineAnim, 0xC2, 0.0f);
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 10.0f;
            w->frame->pos[2] = -80.0f;
            w->frame->rot[1] = 0.0f;
            w->frame->scale[0] = 0.7f;
            w->frame->scale[1] = 0.7f;
            w->frame->scale[2] = 0.7f;
            w->frame->rot[1] = 3.1415927f;
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
            w->frame->rot[1] = 0.0f;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *mach2DemoMachineCreate(Obj *obj) {
    DPRINTF("machine created\n");
    return tskCreate(obj, mach2DemoMachineProc, obj->file, NULL);
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
extern Gfx mach2DemoSmokeDl[];                       /* the smoke's display list */
extern s32 mach2DemoFrame;               /* the file's frame counter */
AnimFrame mach2DemoSmokeAnim[] = {
    { (u32) mach2DemoSmokeTex0, 6 },
    { (u32) mach2DemoSmokeTex1, 6 },
    { (u32) mach2DemoSmokeTex2, 6 },
    { (u32) mach2DemoSmokeTex3, 6 },
    { (u32) mach2DemoSmokeTex4, 6 },
    { 0x03002D30, 8 },
    { (u32) mach2DemoSmokeTex6, 10 },
    { 0, 0 },
    { 0, 0 },
};       /* the smoke's texture animation */

s32 mach2DemoKemuriProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    KemuriWork *w = (KemuriWork *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "KEMURI";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, mach2DemoSmokeDl, 0x123);
            fadeAnimStart(&w->anim, mach2DemoSmokeAnim, 0, &w->frame->segs[3], floadGetSetFile(0x123, 0));
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
            if (!(mach2DemoFrame & 1)) {
                fadeAnimStep(&w->anim);
            }
            if (mach2DemoFrame % w->period == 0) {
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

void mach2DemoKemuriCreate(f32 x, f32 y, f32 z, f32 scale, f32 grow, f32 rise, s32 alpha, s32 fade,
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
    tskCreate(gEffectTask, mach2DemoKemuriProc, gDmenuLastObj->file, &arg);
}

void mach2DemoEffectsStep(void) {
    if (mach2DemoFrame % 40 == 0 && mach2DemoFrame < 2189) {
        mach2DemoKemuriCreate(mach2DemoFrame % 13 - 6, mach2DemoFrame % 17 + 51,
                               mach2DemoFrame % 9 - 80, 0.5f, 1.015f, 0.1f, 120, 4, 4);
    }
    if (mach2DemoFrame == 2159) {
        effectPompomCreate(0.0f, 45.0f, -70.0f, 7.0f, 35.0f, 10.0f, 10, 210, 18, 0.45f, 0.35f);
        effectPompomCreate(0.0f, 55.0f, -70.0f, 5.0f, 10.0f, 5.0f, 10, 210, 80, 0.7f, 0.1f);
        effectPompomCreate(0.0f, 40.0f, -70.0f, 20.0f, 40.0f, 20.0f, 8, 210, 10, 0.4f, 0.3f);
        effectPompomCreate(0.0f, 40.0f, -70.0f, 15.0f, 30.0f, 10.0f, 9, 210, 15, 0.6f, 0.3f);
    }
}
