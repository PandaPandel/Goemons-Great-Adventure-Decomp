#include "tlb.h"
#include "tlb/tlb_0031.h"

/* Main's sinf, called through its address as TLB code calls main. */
#define sinf (*sinf)

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
extern Gfx demo22ShadowDl[]; /* a shadow's display list */

/* The demo player's work ("genoGOE", "chaSAS", "chaYAE"): the player task, its model, its
 * shadow, the face's two texture animations, a frame count and a state. */
typedef struct GenoW {
    /* 0x00 */ Obj *pl;
    /* 0x04 */ AnimeFrame *model;
    /* 0x08 */ PolFrame *shadow;
    /* 0x0C */ Anim eyes;
    /* 0x1C */ Anim mouth;
    /* 0x2C */ s32 chara;
    /* 0x30 */ s32 timer;
    /* 0x34 */ s32 state;
} GenoW;

extern f32 demo22GoeX;     /* Goemon's x and z, each frame */
extern f32 demo22GoeZ;
extern Player *demo22GoePlayer; /* Goemon's player */

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
/* This file's .data (0x08007D60): the characters' texture animations (demotool.c's table, as
 * tlb_0020_1's; nothing reads it), then the positions. */
AnimFrame demo22CharaAnims[] = {
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
f32 demo22SasX = 1.0f; /* Sasuke's x and z, each frame */
f32 demo22SasZ = 1.0f;
f32 demo22GoeX = 1.0f;     /* Goemon's x and z, each frame */
f32 demo22GoeZ = 1.0f;
f32 demo22EbiX = 1.0f; /* Ebisumaru's x and z, each frame */
f32 demo22EbiZ = 1.0f;
f32 demo22YaeX = 1.0f; /* Yae's x and z, each frame */
f32 demo22YaeZ = 1.0f;
u32 demo22Unused2[] = { 0x3F800000, 0x3F800000 };
f32 demo22SuzakuX = 1.0f;       /* suzaku's x, as last placed */
f32 demo22SuzakuZ = 1.0f;       /* suzaku's z, as last placed */
f32 demo22LastPlacedX = 1.0f;         /* its x, as last placed */
f32 demo22LastPlacedZ = 1.0f;         /* its z, as last placed */
u32 demo22Unused3[] = { 0x3F800000, 0x3F800000 };
s32 demo22OniAnimStart = 1;         /* the frame the oni's last animation started */
/* "genoGOE" (Goemon, character 0, in the demo) at (-120, 152, 2600) under the menu's last task;
 * OBJMOVE's model 0 moves him. He plays his animations 0x10C/0x10D/0x105 by the frame count,
 * starting scene runs 0x13E, 0x148, 0x14A, 0x14D and 0x14F; at 0x44C he steps to z 1810. */
s32 demo22GoeProc(Obj *obj, s16 msg, void *arg);
s32 demo22GoeProc(Obj *obj, s16 msg, void *arg) {
    static s32 demo22GoeStep = -1;
    Obj *pl;
    s32 pad;
    GenoW *w = (GenoW *) (obj + 1);
    PlInit init;

    switch (msg) {
        case 2:
            PRINTF("goecreate\n");
            obj->name = "genoGOE";
            plMakeInit(&init, 0, 0);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = -120.0f;
            init.height = 152.0f;
            init.side = 2600.0f;
            w->pl = pl = plCreate(gDmenuLastObj, &init);
            demo22GoePlayer = PL(pl);
            plEnterDemo(pl);
            PRINTF("デモモード突入\n");
            plAnimeSet(PL(w->pl), 0x105);
            w->model = demo22GoePlayer->model;
            PRINTF("*** p->model_buff=%x\n", w->model);
            PRINTF("ゴエモン作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, demo22ShadowDl, 0xBC);
            w->shadow->scale[2] = 0.08f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->timer = 0;
            w->state = 0;
            w->chara = 0;
            w->model->rot[1] = 3.141592741;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -120.0f;
            w->model->pos[1] = 152.0f;
            w->model->pos[2] = 2600.0f;
            w->model->rot[1] = 3.141592741f;
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
            w->timer++;
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = w->model->pos[1];
            demo22GoeX = w->model->pos[0];
            demo22GoeZ = w->model->pos[2];
            switch (w->state) {
                case 0:
                    w->model->scale[2] = 1.0f;
                    w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                    w->model->pos[0] = -120.0f;
                    w->model->pos[1] = 152.0f;
                    w->model->pos[2] = 2600.0f;
                    w->model->rot[1] = 3.141592741f;
                    w->state++;
                    break;
                case 1:
                    if (w->timer >= 0x1A4) {
                        if (demoAnimeStep(w->model, 0.2f, 2) == 1) {
                            scRunStart(0x13E, 0, 0);
                            plAnimeSet(PL(w->pl), 0x10C);
                            demoTexAnimK0(&w->eyes, 3, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK0(&w->mouth, 3, &PL(w->pl)->model->segs[4]);
                            w->state++;
                            demo22GoeStep = 0;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
                case 2:
                    if (demo22GoeStep != -1) {
                        if (demoAnimeStep(w->model, 0.5f, 2) == 1) {
                            plAnimeSet(PL(w->pl), 0x10D);
                            demo22GoeStep = -1;
                        }
                    } else if (w->timer >= 0x30C) {
                        if (demoAnimeStep(w->model, 0.5f, 2) == 1) {
                            plAnimeSet(PL(w->pl), 0x10C);
                            w->model->frame = polAnimeLastFrame(w->model);
                            scRunEnd();
                            w->state++;
                            demo22GoeStep = 0;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.5f, 0);
                    }
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
                case 3:
                    if (demo22GoeStep != -1) {
                        if (demoAnimeStep(w->model, 0.5f, 3) == 1) {
                            plAnimeSet(PL(w->pl), 0x105);
                            demo22GoeStep = -1;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    if (w->timer == 0x44C) {
                        w->model->pos[0] = -120.0f;
                        w->model->pos[1] = 152.0f;
                        w->model->pos[2] = 1810.0f;
                        w->model->rot[1] = 3.141592741f;
                        w->state++;
                    }
                    break;
                case 4:
                    if (w->timer >= 0x1018) {
                        if (demoAnimeStep(w->model, 0.2f, 2) == 1) {
                            scRunStart(0x148, 0, 0);
                            plAnimeSet(PL(w->pl), 0x10C);
                            w->state++;
                            demo22GoeStep = 0;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
                case 5:
                    if (demo22GoeStep != -1) {
                        if (demoAnimeStep(w->model, 0.5f, 2) == 1) {
                            plAnimeSet(PL(w->pl), 0x10D);
                            demo22GoeStep = -1;
                        }
                    } else if (w->timer >= 0x1180) {
                        if (demoAnimeStep(w->model, 0.5f, 2) == 1) {
                            plAnimeSet(PL(w->pl), 0x10C);
                            w->model->frame = polAnimeLastFrame(w->model);
                            scRunEnd();
                            w->state++;
                            demo22GoeStep = 0;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.5f, 0);
                        fadeAnimStep(&w->eyes);
                        fadeAnimStep(&w->mouth);
                    }
                    break;
                case 6:
                    switch (demo22GoeStep) {
                        case 0:
                            if (demoAnimeStep(w->model, 0.5f, 3) == 1) {
                                plAnimeSet(PL(w->pl), 0x114);
                                demo22GoeStep++;
                            }
                            break;
                        case 1:
                            if (demoAnimeStep(w->model, 0.1f, 2) == 1) {
                                plAnimeSet(PL(w->pl), 0x106);
                                demo22GoeStep++;
                            }
                            break;
                        case 2:
                            if (demoAnimeStep(w->model, 0.15f, 2) == 1) {
                                plAnimeSet(PL(w->pl), 0x105);
                                demo22GoeStep++;
                            }
                            break;
                        case 3:
                            demoAnimeStep(w->model, 0.2f, 0);
                            break;
                    }
                    if (w->timer == 0x16BC) {
                        plAnimeSet(PL(w->pl), 0x105);
                        demoTexAnimK0(&w->eyes, 7, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 7, &PL(w->pl)->model->segs[4]);
                        scRunStart(0x14A, 0, 0);
                        w->state++;
                        demo22GoeStep = 0;
                    }
                    break;
                case 7:
                    if (w->timer == 0x1782) {
                        scRunEnd();
                    }
                    if (w->timer >= 0x1C34) {
                        if (demoAnimeStep(w->model, 0.2f, 2) == 1) {
                            scRunStart(0x14D, 0, 0);
                            plAnimeSet(PL(w->pl), 0x10C);
                            demoTexAnimK0(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK0(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                            w->state++;
                            demo22GoeStep = 0;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
                case 8:
                    if (demo22GoeStep != -1) {
                        if (demoAnimeStep(w->model, 0.5f, 2) == 1) {
                            plAnimeSet(PL(w->pl), 0x10D);
                            demo22GoeStep = -1;
                        }
                    } else if (w->timer >= 0x1DB0) {
                        if (demoAnimeStep(w->model, 0.5f, 2) == 1) {
                            plAnimeSet(PL(w->pl), 0x10C);
                            w->model->frame = polAnimeLastFrame(w->model);
                            demo22GoeStep = 0;
                            scRunEnd();
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.5f, 0);
                    }
                    break;
                case 9:
                    if (demo22GoeStep != -1) {
                        if (demoAnimeStep(w->model, 0.5f, 3) == 1) {
                            plAnimeSet(PL(w->pl), 0x105);
                            demo22GoeStep = -1;
                        }
                    } else if (w->timer >= 0x1F54) {
                        if (demoAnimeStep(w->model, 0.2f, 2) == 1) {
                            scRunStart(0x14F, 0, 0);
                            plAnimeSet(PL(w->pl), 0x10C);
                            w->state++;
                            demo22GoeStep = 0;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
                case 10:
                    if (demo22GoeStep != -1) {
                        if (demoAnimeStep(w->model, 0.5f, 2) == 1) {
                            plAnimeSet(PL(w->pl), 0x10D);
                            demo22GoeStep = -1;
                        }
                    } else if (w->timer >= 0x20E4) {
                        if (demoAnimeStep(w->model, 0.5f, 2) == 1) {
                            plAnimeSet(PL(w->pl), 0x10C);
                            w->model->frame = polAnimeLastFrame(w->model);
                            demo22GoeStep = 0;
                            scRunEnd();
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.5f, 0);
                        fadeAnimStep(&w->eyes);
                        fadeAnimStep(&w->mouth);
                    }
                    break;
                case 11:
                    if (demo22GoeStep != -1) {
                        if (demoAnimeStep(w->model, 0.5f, 3) == 1) {
                            plAnimeSet(PL(w->pl), 0x105);
                            demo22GoeStep = -1;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *demo22GoeCreate(Obj *obj) {
    return tskCreate(obj, demo22GoeProc, obj->file, NULL);
}

extern f32 demo22EbiX; /* Ebisumaru's x and z, each frame */
extern f32 demo22EbiZ;

/* "genoGOE" (Ebisumaru, character 1, in the demo) at (-340, 152, 2490) under the menu's last
 * task; OBJMOVE's model 1 moves him. At frame 0x44C he steps to z 1810; his eyes rest from
 * 0x16BC to 0x1784 (texture animation 4). */
s32 demo22EbiProc(Obj *obj, s16 msg, void *arg) {
    s32 pad[2];
    GenoW *w = (GenoW *) (obj + 1);
    Player *p;
    PlInit init;

    switch (msg) {
        case 2:
            obj->name = "genoGOE";
            plMakeInit(&init, 0, 1);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = -340.0f;
            init.height = 152.0f;
            init.side = 2490.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            PRINTF("デモモード突入\n");
            plAnimeSet(PL(w->pl), 0x105);
            demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
            demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
            w->model = p->model;
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, demo22ShadowDl, 0xBC);
            w->shadow->scale[2] = 0.09f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 1;
            w->timer = 0;
            w->state = 0;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -340.0f;
            w->model->pos[1] = 152.0f;
            w->model->pos[2] = 2490.0f;
            w->model->rot[1] = 3.141592741f;
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
            demo22EbiX = w->model->pos[0];
            demo22EbiZ = w->model->pos[2];
            w->timer++;
            switch (w->state) {
                case 0:
                    w->model->pos[0] = -340.0f;
                    w->model->pos[1] = 152.0f;
                    w->model->pos[2] = 2490.0f;
                    w->model->rot[1] = 3.141592741f;
                    scRunEnd();
                    w->state++;
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (w->timer < 0x16BC || w->timer >= 0x1784) {
                        fadeAnimStep(&w->eyes);
                    }
                    fadeAnimStep(&w->mouth);
                    if (w->timer == 0x44C) {
                        w->model->pos[0] = -340.0f;
                        w->model->pos[1] = 152.0f;
                        w->model->pos[2] = 1810.0f;
                        w->model->rot[1] = 3.141592741f;
                    }
                    if (w->timer == 0x16BC) {
                        demoTexAnimK1(&w->eyes, 4, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 4, &PL(w->pl)->model->segs[4]);
                    } else if (w->timer == 0x1784) {
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *demo22EbiCreate(Obj *obj) {
    return tskCreate(obj, demo22EbiProc, obj->file, NULL);
}

extern f32 demo22SasX; /* Sasuke's x and z, each frame */
extern f32 demo22SasZ;
extern s32 demo22SasAnimEnd;           /* -1 once Sasuke's one-shot animation has ended */

/* "chaSAS": Sasuke (character 2) in the demo at (300, 152, 2490) under the menu's last task;
 * OBJMOVE's model 2 moves him. At frame 0x44C he steps to z 1810; at 0xE88 to x 170 with his
 * animation 0x10C (scene run 0x147), then 0x10D until 0x100E; back at x 300 at 0x1784. */
s32 demo22SasProc(Obj *obj, s16 msg, void *arg) {
    s32 pad[2];
    GenoW *w = (GenoW *) (obj + 1);
    Player *p;
    PlInit init;

    switch (msg) {
        case 2:
            obj->name = "chaSAS";
            plMakeInit(&init, 0, 2);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = 300.0f;
            init.height = 152.0f;
            init.side = 2490.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            PRINTF("デモモード突入\n");
            plAnimeSet(PL(w->pl), 0x105);
            w->model = p->model;
            PRINTF("サスケ作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, demo22ShadowDl, 0xBC);
            w->shadow->scale[2] = 0.065f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 2;
            w->timer = 0;
            w->state = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 300.0f;
            w->model->pos[1] = 152.0f;
            w->model->pos[2] = 2490.0f;
            w->model->rot[1] = 3.141592741f;
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
            demo22SasX = w->model->pos[0];
            demo22SasZ = w->model->pos[2];
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = 0.3f;
            w->timer++;
            switch (w->state) {
                case 0:
                    w->model->pos[0] = 300.0f;
                    w->model->pos[1] = 152.0f;
                    w->model->pos[2] = 2490.0f;
                    w->model->rot[1] = 3.141592741f;
                    w->state++;
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (w->timer == 0x44C) {
                        w->model->pos[0] = 300.0f;
                        w->model->pos[1] = 152.0f;
                        w->model->pos[2] = 1810.0f;
                        w->model->rot[1] = 3.141592741f;
                        w->state++;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (w->timer == 0xE88) {
                        w->model->pos[0] = 170.0f;
                    }
                    if (w->timer == 0xE88) {
                        scRunStart(0x147, 0, 0);
                        plAnimeSet(PL(w->pl), 0x10C);
                        demoTexAnimK2(&w->eyes, 3, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 3, &PL(w->pl)->model->segs[4]);
                        demo22SasAnimEnd = 0;
                        w->state++;
                    }
                    break;
                case 3:
                    if (demo22SasAnimEnd != -1) {
                        if (demoAnimeStep(w->model, 0.5f, 2) == 1) {
                            plAnimeSet(PL(w->pl), 0x10D);
                            demo22SasAnimEnd = -1;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.5f, 0);
                        fadeAnimStep(&w->eyes);
                        fadeAnimStep(&w->mouth);
                    }
                    if (w->timer >= 0x100E) {
                        plAnimeSet(PL(w->pl), 0x10C);
                        w->model->frame = polAnimeLastFrame(w->model);
                        scRunEnd();
                        demo22SasAnimEnd = 0;
                        w->state++;
                    }
                    break;
                case 4:
                    if (demo22SasAnimEnd != -1) {
                        if (demoAnimeStep(w->model, 0.5f, 3) == 1) {
                            plAnimeSet(PL(w->pl), 0x105);
                            demoTexAnimK2(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK2(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                            demo22SasAnimEnd = -1;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                        if (w->timer < 0x16BC || w->timer >= 0x1784) {
                            fadeAnimStep(&w->eyes);
                        }
                        fadeAnimStep(&w->mouth);
                    }
                    if (w->timer == 0x16BC) {
                        demoTexAnimK2(&w->eyes, 4, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 4, &PL(w->pl)->model->segs[4]);
                    } else if (w->timer == 0x1784) {
                        demoTexAnimK2(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    if (w->timer == 0x1784) {
                        w->model->pos[0] = 300.0f;
                    }
                    if (w->timer == 0x1DC4) {
                        w->model->pos[0] = 170.0f;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *demo22SasCreate(Obj *obj) {
    return tskCreate(obj, demo22SasProc, obj->file, NULL);
}

extern f32 demo22YaeX; /* Yae's x and z, each frame */
extern f32 demo22YaeZ;

/* "chaYAE": Yae (character 3) in the demo at (120, 152, 2600) under the menu's last task;
 * OBJMOVE's model 3 moves her. At frame 0x44C she steps to z 1810, then moves on x by the frame
 * count; from 0x1DC4 her animation 0x10D (scene run 0x14E), from 0x1F2C back to 0x105. */
s32 demo22YaeProc(Obj *obj, s16 msg, void *arg) {
    s32 pad[2];
    GenoW *w = (GenoW *) (obj + 1);
    Player *p;
    PlInit init;

    switch (msg) {
        case 2:
            obj->name = "chaYAE";
            plMakeInit(&init, 0, 3);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = 120.0f;
            init.height = 152.0f;
            init.side = 2600.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            PRINTF("デモモード突入\n");
            plAnimeSet(PL(w->pl), 0x105);
            w->model = p->model;
            PRINTF("ヤエ作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, demo22ShadowDl, 0xBC);
            w->shadow->scale[2] = 0.065f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 3;
            w->timer = 0;
            w->state = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 120.0f;
            w->model->pos[1] = 152.0f;
            w->model->pos[2] = 2600.0f;
            w->model->rot[1] = 3.141592741f;
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
            demo22YaeX = w->model->pos[0];
            demo22YaeZ = w->model->pos[2];
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = 0.3f;
            w->timer++;
            switch (w->state) {
                case 0:
                    w->state++;
                    w->model->pos[0] = 120.0f;
                    w->model->pos[1] = 152.0f;
                    w->model->pos[2] = 2600.0f;
                    w->model->rot[1] = 3.141592741f;
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (w->timer == 0x44C) {
                        w->model->pos[0] = 120.0f;
                        w->model->pos[1] = 152.0f;
                        w->model->pos[2] = 1810.0f;
                        w->model->rot[1] = 3.141592741f;
                        w->state++;
                    }
                    break;
                case 2:
                    if (w->timer == 0xE88) {
                        w->model->pos[0] = 50.0f;
                    }
                    if (w->timer == 0x1784) {
                        w->model->pos[0] = 120.0f;
                    }
                    if (w->timer == 0x1DC4) {
                        w->model->pos[0] = 50.0f;
                    }
                    if (w->timer >= 0x1DC4) {
                        if (demoAnimeStep(w->model, 0.3f, 2) == 1) {
                            plAnimeSet(PL(w->pl), 0x10D);
                            scRunStart(0x14E, 0, 0);
                            demoTexAnimK3(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK3(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                        if (w->timer < 0x16BC || w->timer >= 0x1784) {
                            fadeAnimStep(&w->eyes);
                        }
                        fadeAnimStep(&w->mouth);
                    }
                    if (w->timer == 0x16BC) {
                        demoTexAnimK3(&w->eyes, 4, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 4, &PL(w->pl)->model->segs[4]);
                    } else if (w->timer == 0x1784) {
                        demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 3:
                    if (w->timer >= 0x1F2C) {
                        if (demoAnimeStep(w->model, 0.3f, 2) == 1) {
                            plAnimeSet(PL(w->pl), 0x105);
                            demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                            scRunEnd();
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.3f, 0);
                        fadeAnimStep(&w->eyes);
                        fadeAnimStep(&w->mouth);
                    }
                    break;
                case 4:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *demo22YaeCreate(Obj *obj) {
    return tskCreate(obj, demo22YaeProc, obj->file, NULL);
}

/* "suzaku" (its proc demo22SuzakuProc): its model, a shadow, a texture animation, a frame
 * count and a state. */
typedef struct SuzakuW {
    /* 0x00 */ AnimeFrame *model;
    /* 0x04 */ PolFrame *shadow;
    /* 0x08 */ Anim anim;
    /* 0x18 */ s32 timer;
    /* 0x1C */ s32 state;
} SuzakuW;

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
extern s32 demo22SuzakuShadowAlpha;                 /* suzaku's shadow's alpha */
extern u8 demo22SuzakuModelAnim[];                /* suzaku's animation */
extern Gfx demo22ShadowDl[];               /* a shadow's display list */
extern AnimFrame demo22SuzakuAnim[]; /* suzaku's texture animation */
extern f32 demo22SuzakuX;       /* suzaku's x, as last placed */
extern f32 demo22SuzakuZ;       /* suzaku's z, as last placed */

s32 demo22SuzakuProc(Obj *obj, s16 msg, void *arg);

/* "suzaku": Suzaku's model, moved by OBJMOVE as model 4; from frame 3020 it comes down from
 * 1800 to 1170 and then hovers there. */
s32 demo22SuzakuProc(Obj *obj, s16 msg, void *arg) {
    SuzakuW *w = (SuzakuW *) (obj + 1);
    s32 file;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "suzaku";
            PRINTF("スザク作ります\n");
            w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->model == NULL) {
                return -1;
            }
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoAnimeSet(w->model, (u32) demo22SuzakuModelAnim, 0xCD, 0.0f);
            file = floadGetSetFile(0xCD, 0);
            fadeAnimStart(&w->anim, demo22SuzakuAnim, 0, &w->model->segs[3], file);
            demoPolSetDL(w->shadow, demo22ShadowDl, 0xBC);
            w->shadow->scale[2] = 0.08f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->timer = 0;
            w->state = 0;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -200.0f;
            w->model->pos[1] = 1800.0f;
            w->model->pos[2] = 100.0f;
            w->model->rot[1] = 0.0f;
            PRINTF("スザク作りました\n");
            break;
        case 1:
            frmDestroy((Frame *) w->model);
            frmDestroy((Frame *) w->shadow);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, demo22SuzakuShadowAlpha);
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
            w->timer++;
            switch (w->state) {
                case 0:
                    w->model->pos[0] = -200.0f;
                    w->model->pos[1] = 0.0f;
                    w->model->pos[2] = 100.0f;
                    w->model->rot[1] = 0.0f;
                    w->state++;
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (w->timer >= 0xBCC) {
                        w->model->pos[1] = 1800.0f;
                        w->state++;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (1170.0f < w->model->pos[1]) {
                        w->model->pos[1] -= 10.0f;
                    } else {
                        w->state++;
                    }
                    break;
                case 3:
                    demoAnimeStep(w->model, 0.3f, 0);
                    w->model->pos[1] = 1170.0f - sinf((w->timer % 120) * 3.0f * 0.0174532930038800785) * 20.0f;
                    if (w->timer == 0xE88) {
                        w->model->pos[0] = -400.0f;
                    }
                    if (w->timer == 0x16BC) {
                        w->model->pos[0] = -200.0f;
                    }
                    break;
            }
            demo22SuzakuX = w->model->pos[0];
            demo22SuzakuZ = w->model->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Starts a "suzaku" under obj. */
Obj *demo22SuzakuCreate(Obj *obj) {
    return tskCreate(obj, demo22SuzakuProc, obj->file, NULL);
}

/* "dochuki" (its proc demo22DochukiProc): the oni's model, its glow, its texture animation,
 * a frame count and a state. */
typedef struct DochukiW {
    /* 0x00 */ AnimeFrame *model;
    /* 0x04 */ PolFrame *glow;
    /* 0x08 */ Anim anim;
    /* 0x18 */ s32 timer;
    /* 0x1C */ s32 state;
} DochukiW;

extern u8 demo22DochukiAnim[]; /* the oni's animations (file 0x29) */
extern u8 demo22DochukiAnim2[];
extern u8 demo22DochukiState5Anim[];
extern u8 demo22DochukiState6Anim[];
extern u8 demo22DochukiState7Anim[];
extern u8 demo22DochukiState9Anim[];
extern u8 demo22DochukiState10Anim[];
extern u8 demo22DochukiState11Anim[];
extern u8 demo22DochukiAnim3[];
extern AnimFrame demo22OniAnim[]; /* the oni's texture animations */
extern s32 demo22DochukiAnimEnd;                   /* -1 once the oni's one-shot animation has ended */
extern Gfx demo22SparkleDl[];                 /* a sparkle's display list (file 0x120) */
extern s32 demo22OniAnimStart;         /* the frame the oni's last animation started */
extern f32 demo22LastPlacedX;         /* its x, as last placed */
extern f32 demo22LastPlacedZ;         /* its z, as last placed */

/* "dochuki": the Douchuu oni (file 0x29) at (0, 860, 240) with a glow, moved by OBJMOVE as
 * model 5. It steps through its animations by the frame count (scene runs 0x13F to 0x14C), the
 * glow flies off at 0x974; from state 15 the oni shrinks away and the glow fades. */
s32 demo22DochukiProc(Obj *obj, s16 msg, void *arg) {
    DochukiW *w = (DochukiW *) (obj + 1);
    s32 file;

    switch (msg) {
        case 2:
            obj->name = "dochuki";
            PRINTF("ドウチュウ鬼作ります\n");
            w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->model == NULL) {
                return -1;
            }
            w->glow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->glow == NULL) {
                return -1;
            }
            demoAnimeSet(w->model, (u32) demo22DochukiAnim, 0x29, 0.0f);
            file = floadGetSetFile(0x29, 0);
            fadeAnimStart(&w->anim, demo22OniAnim, 1, &w->model->segs[3], file);
            demoPolSetDL(w->glow, demo22SparkleDl, 0x120);
            w->glow->scale[2] = 0.5f;
            w->glow->scale[0] = w->glow->scale[1] = w->glow->scale[2];
            w->glow->pos[0] = 0.0f;
            w->glow->pos[1] = 1060.0f;
            w->glow->pos[2] = 240.0f;
            w->timer = 0;
            w->state = 0;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 0.0f;
            w->model->pos[1] = 860.0f;
            w->model->pos[2] = 240.0f;
            w->model->rot[1] = 0.0f;
            PRINTF("ドウチュウ鬼作りました\n");
            break;
        case 1:
            frmDestroy((Frame *) w->model);
            frmDestroy((Frame *) w->glow);
            break;
        case 3:
            w->glow->dl = demoColorDL(255, 255, 255, 0, 200, 80, 160);
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
            w->timer++;
            switch (w->state) {
                case 0:
                    w->model->pos[0] = 0.0f;
                    w->model->pos[1] = 860.0f;
                    w->model->pos[2] = 240.0f;
                    w->model->rot[1] = 0.0f;
                    w->state++;
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (w->timer >= 0x514) {
                        demoAnimeSet(w->model, (u32) demo22DochukiAnim2, 0x29, 0.0f);
                        fadeAnimStart(&w->anim, demo22OniAnim, 1, &w->model->segs[3], floadGetSetFile(0x29, 0));
                        scRunStart(0x13F, 0, 0);
                        w->state++;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->anim);
                    if (w->timer >= 0x5D2) {
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 3:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (w->timer >= 0x76C) {
                        demoAnimeSet(w->model, (u32) demo22DochukiAnim2, 0x29, 0.0f);
                        fadeAnimStart(&w->anim, demo22OniAnim, 1, &w->model->segs[3], floadGetSetFile(0x29, 0));
                        scRunStart(0x141, 0, 0);
                        w->state++;
                    }
                    break;
                case 4:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->anim);
                    if (w->timer >= 0x82A) {
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (w->timer == 0x8FC) {
                        scRunStart(0x143, 0, 0);
                    }
                    if (w->timer >= 0x938) {
                        demoAnimeSet(w->model, (u32) demo22DochukiState5Anim, 0x29, 0.0f);
                        fadeAnimStart(&w->anim, demo22OniAnim, 1, &w->model->segs[3], floadGetSetFile(0x29, 0));
                        w->state++;
                    }
                    break;
                case 6:
                    demoAnimeStep(w->model, 0.1f, 2);
                    if (w->timer == 0x974) {
                        w->glow->pos[0] = 110.0f;
                        w->glow->pos[1] = 1130.0f;
                        w->glow->pos[2] = 220.0f;
                        w->glow->scale[2] = 4.0f;
                        w->glow->scale[0] = w->glow->scale[1] = w->glow->scale[2];
                        sndPlay(0x198);
                    }
                    if (2.0f < w->glow->scale[2]) {
                        w->glow->pos[0] += 20.0f;
                        w->glow->pos[1] -= 10.0f;
                        if (300.0f < w->glow->pos[0]) {
                            w->glow->scale[2] = 2.0f;
                            w->glow->scale[0] = w->glow->scale[1] = w->glow->scale[2];
                        }
                    }
                    if (w->timer == 0x974) {
                        w->glow->pos[1] = 0.0f;
                    }
                    if (w->timer == 0x96A) {
                        scRunEnd();
                    }
                    if (w->timer >= 0x9D8) {
                        demoAnimeSet(w->model, (u32) demo22DochukiState6Anim, 0x29, 0.0f);
                        fadeAnimStart(&w->anim, demo22OniAnim, 1, &w->model->segs[3], floadGetSetFile(0x29, 0));
                        scRunStart(0x145, 0, 0);
                        demo22DochukiAnimEnd = 0;
                        w->state++;
                    }
                    break;
                case 7:
                    if (demo22DochukiAnimEnd != -1) {
                        if (demoAnimeStep(w->model, 0.05f, 2) == 1) {
                            demoAnimeSet(w->model, (u32) demo22DochukiState7Anim, 0x29, 0.0f);
                            fadeAnimStart(&w->anim, demo22OniAnim, 1, &w->model->segs[3], floadGetSetFile(0x29, 0));
                            demo22DochukiAnimEnd = -1;
                        }
                    } else if (w->timer >= 0xC30) {
                        if (demoAnimeStep(w->model, 0.05f, 2) == 1) {
                            demoAnimeSet(w->model, (u32) demo22DochukiState7Anim, 0x29, 0.0f);
                            fadeAnimStart(&w->anim, demo22OniAnim, 1, &w->model->segs[3], floadGetSetFile(0x29, 0));
                            scRunStart(0x146, 0, 0);
                            demo22DochukiAnimEnd = 0;
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.05f, 0);
                    }
                    if (w->timer < 0xBCC) {
                        fadeAnimStep(&w->anim);
                    }
                    if (w->timer == 0xBCC) {
                        scRunEnd();
                    }
                    break;
                case 8:
                    if (w->timer == 0xE7E) {
                        scRunEnd();
                    }
                    if (w->timer >= 0xE74) {
                        if (demoAnimeStep(w->model, 0.1f, 2) == 1) {
                            demoAnimeSet(w->model, (u32) demo22DochukiState7Anim, 0x29, 0.0f);
                            fadeAnimStart(&w->anim, demo22OniAnim, 1, &w->model->segs[3], floadGetSetFile(0x29, 0));
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.1f, 0);
                        fadeAnimStep(&w->anim);
                    }
                    break;
                case 9:
                    if (w->timer >= 0x12D4) {
                        if (demoAnimeStep(w->model, 0.1f, 2) == 1) {
                            demoAnimeSet(w->model, (u32) demo22DochukiState9Anim, 0x29, 0.0f);
                            fadeAnimStart(&w->anim, demo22OniAnim, 1, &w->model->segs[3], floadGetSetFile(0x29, 0));
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.1f, 0);
                    }
                    break;
                case 10:
                    demoAnimeStep(w->model, 0.1f, 2);
                    if (w->timer >= 0x1339) {
                        demoAnimeSet(w->model, (u32) demo22DochukiState10Anim, 0x29, 0.0f);
                        fadeAnimStart(&w->anim, demo22OniAnim, 0, &w->model->segs[3], floadGetSetFile(0x29, 0));
                        scRunStart(0x149, 0, 0);
                        w->state++;
                    }
                    break;
                case 11:
                    if (w->timer >= 0x15F5) {
                        if (demoAnimeStep(w->model, 0.2f, 2) == 1) {
                            scRunEnd();
                            demoAnimeSet(w->model, (u32) demo22DochukiState11Anim, 0x29, 0.0f);
                            fadeAnimStart(&w->anim, demo22OniAnim, 0, &w->model->segs[3], floadGetSetFile(0x29, 0));
                            demo22OniAnimStart = w->timer;
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                        fadeAnimStep(&w->anim);
                    }
                    break;
                case 12:
                    demoAnimeStep(w->model, 0.2f, 2);
                    if (w->timer == 0x160B) {
                        sndPlay(0x19C);
                        fadeAnimStart(&w->anim, demo22OniAnim, 1, &w->model->segs[3], floadGetSetFile(0x29, 0));
                    } else if (w->timer == demo22OniAnimStart + 42) {
                        fadeAnimStart(&w->anim, demo22OniAnim, 0, &w->model->segs[3], floadGetSetFile(0x29, 0));
                    }
                    if (w->timer >= 0x16BD) {
                        demoAnimeSet(w->model, (u32) demo22DochukiAnim3, 0x29, 0.0f);
                        fadeAnimStart(&w->anim, demo22OniAnim, 0, &w->model->segs[3], floadGetSetFile(0x29, 0));
                        w->state++;
                    }
                    break;
                case 13:
                    if (w->timer >= 0x1784) {
                        if (demoAnimeStep(w->model, 0.2f, 2) == 1) {
                            demoAnimeSet(w->model, (u32) demo22DochukiState7Anim, 0x29, 0.0f);
                            fadeAnimStart(&w->anim, demo22OniAnim, 1, &w->model->segs[3], floadGetSetFile(0x29, 0));
                            scRunStart(0x14B, 0, 0);
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
                case 14:
                    if (w->timer == 0x1C20) {
                        scRunEnd();
                    } else if (w->timer == 0x1AA4) {
                        scRunEnd();
                        scRunStart(0x14C, 0, 0);
                    }
                    if (w->timer >= 0x1C20) {
                        if (demoAnimeStep(w->model, 0.2f, 2) == 1) {
                            demoAnimeSet(w->model, (u32) demo22DochukiAnim3, 0x29, 0.0f);
                            fadeAnimStart(&w->anim, demo22OniAnim, 0, &w->model->segs[3], floadGetSetFile(0x29, 0));
                            sndPlay(0x199);
                            w->glow->pos[0] = 0.0f;
                            w->glow->pos[1] = 1060.0f;
                            w->glow->pos[2] = 340.0f;
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                        fadeAnimStep(&w->anim);
                    }
                    break;
                case 15:
                    demoAnimeStep(w->model, 0.2f, 0);
                    w->model->scale[2] -= 0.01;
                    w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                    w->model->pos[1] += 1.0;
                    if (w->model->scale[2] < 0.02) {
                        w->state++;
                    }
                    w->glow->scale[2] += 0.4;
                    w->glow->scale[0] = w->glow->scale[2];
                    w->glow->scale[1] += 0.8;
                    break;
                case 16:
                    w->model->pos[1] = 0.0f;
                    if (w->glow->scale[1] >= 2.0) {
                        w->glow->scale[2] -= 0.5;
                        w->glow->scale[0] = w->glow->scale[2];
                        w->glow->scale[1] -= 1.0;
                    } else {
                        w->glow->pos[1] = 0.0f;
                    }
                    break;
            }
            demo22LastPlacedX = w->model->pos[0];
            demo22LastPlacedZ = w->model->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *demo22DochukiCreate(Obj *obj) {
    return tskCreate(obj, demo22DochukiProc, obj->file, NULL);
}

/* "bismaru" (its proc demo22BismaruProc): its model, a shadow frame, its glow, a frame
 * count, a state and its rise speed. */
typedef struct BismaruW {
    /* 0x00 */ AnimeFrame *model;
    /* 0x04 */ PolFrame *shadow;
    /* 0x08 */ PolFrame *glow;
    /* 0x0C */ u8 pad0C[0x10];
    /* 0x1C */ s32 timer;
    /* 0x20 */ s32 state;
    /* 0x24 */ f32 vel; /* its rise speed when it flies off */
} BismaruW;

extern u8 demo22BismaruAnim[];  /* Bismaru's animation */
extern Gfx demo22SparkleDl[]; /* a sparkle's display list (file 0x120) */
extern f32 demo22LastPlacedX; /* its x, as last placed */
extern f32 demo22LastPlacedZ; /* its z, as last placed */

/* "bismaru": Bismaru's model at (310, 860, 130) with a glow, moved by OBJMOVE as model 6; it steps
 * its animation and starts scene runs 0x140, 0x142 and 0x144 by the frame count, then flies off
 * upward, shrinking, until it is gone. */
s32 demo22BismaruProc(Obj *obj, s16 msg, void *arg) {
    BismaruW *w = (BismaruW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "bismaru";
            PRINTF("ビスマル作ります\n");
            w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->model == NULL) {
                return -1;
            }
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            w->glow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->glow == NULL) {
                return -1;
            }
            demoAnimeSet(w->model, (u32) demo22BismaruAnim, 0xC3, 0.0f);
            demoPolSetDL(w->glow, demo22SparkleDl, 0x120);
            w->glow->scale[2] = 1.0f;
            w->glow->scale[0] = w->glow->scale[1] = w->glow->scale[2];
            w->glow->pos[0] = 310.0f;
            w->glow->pos[1] = 960.0f;
            w->glow->pos[2] = 130.0f;
            w->timer = 0;
            w->state = 0;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 310.0f;
            w->model->pos[1] = 860.0f;
            w->model->pos[2] = 130.0f;
            w->model->rot[1] = 0.0f;
            PRINTF("ビスマル作りました\n");
            break;
        case 1:
            frmDestroy((Frame *) w->model);
            frmDestroy((Frame *) w->shadow);
            frmDestroy((Frame *) w->glow);
            break;
        case 3:
            w->glow->dl = demoColorDL(255, 255, 255, 0, 200, 80, 160);
            break;
        case 0:
            w->model->f.flags |= 2;
            if (gObjMoveSel == 6) {
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
            w->timer++;
            switch (w->state) {
                case 0:
                    w->model->pos[0] = 310.0f;
                    w->model->pos[1] = 860.0f;
                    w->model->pos[2] = 130.0f;
                    w->model->rot[1] = 0.0f;
                    w->state++;
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (w->timer == 0x5DC) {
                        scRunStart(0x140, 0, 0);
                        w->state++;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->model, 0.7f, 0);
                    if (w->timer == 0x758) {
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 3:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (w->timer == 0x834) {
                        scRunStart(0x142, 0, 0);
                        w->state++;
                    }
                    break;
                case 4:
                    demoAnimeStep(w->model, 0.6f, 0);
                    if (w->timer == 0x8F2) {
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (w->timer == 0x974) {
                        scRunStart(0x144, 0, 0);
                        w->vel = 19.6f;
                        w->state++;
                    }
                    break;
                case 6:
                    w->model->pos[1] += w->vel;
                    w->vel -= 0.49;
                    w->model->pos[0] += 10.0f;
                    w->model->scale[2] -= 0.01;
                    w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                    w->glow->pos[0] = w->model->pos[0];
                    w->glow->pos[1] = 100.0f * w->model->scale[1] + w->model->pos[1];
                    w->glow->pos[2] = 180.0f;
                    w->glow->scale[0] = w->glow->scale[1] = w->glow->scale[2] =
                        15.0f * w->model->scale[0] + sinf(w->timer % 36 * 10 * 0.0174532930038800785) * 0.3;
                    if (w->timer == 0x9CE) {
                        scRunEnd();
                    }
                    if (w->model->scale[0] <= 0.05) {
                        w->state++;
                    } else if (w->model->pos[1] < 152.0f) {
                        w->state++;
                    }
                    break;
                case 7:
                    w->model->pos[1] = -200.0f;
                    if (w->timer == 0x9CE) {
                        scRunEnd();
                    }
                    break;
            }
            demo22LastPlacedX = w->model->pos[0];
            demo22LastPlacedZ = w->model->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *demo22BismaruCreate(Obj *obj) {
    return tskCreate(obj, demo22BismaruProc, obj->file, NULL);
}

/* "gate" (its proc demo22GateProc): its model and a shadow, a frame count and a state. */
typedef struct GateW {
    /* 0x00 */ AnimeFrame *model;
    /* 0x04 */ PolFrame *shadow;
    /* 0x08 */ u8 pad08[0x10];
    /* 0x18 */ s32 timer;
    /* 0x1C */ s32 state;
} GateW;

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
extern s32 demo22ShadowAlpha;           /* the shadows' alpha */
extern u8 demo22GateAnim[];          /* the gate's animation */
extern f32 demo22LastPlacedX; /* the gate's x, as last placed */
extern f32 demo22LastPlacedZ; /* the gate's z, as last placed */

/* "gate": the gate's model at (0, 292, 3600), moved by OBJMOVE as model 7; it plays sound 0x19A
 * once, then opens. */
s32 demo22GateProc(Obj *obj, s16 msg, void *arg) {
    GateW *w = (GateW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "gate";
            w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->model == NULL) {
                return -1;
            }
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoAnimeSet(w->model, (u32) demo22GateAnim, 0xCB, 0.0f);
            w->state = 0;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 0.0f;
            w->model->pos[1] = 292.0f;
            w->model->pos[2] = 3600.0f;
            w->model->rot[1] = 0.0f;
            break;
        case 1:
            frmDestroy((Frame *) w->model);
            frmDestroy((Frame *) w->shadow);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, demo22ShadowAlpha);
            break;
        case 0:
            w->model->f.flags |= 2;
            if (gObjMoveSel == 7) {
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
            w->timer++;
            switch (w->state) {
                case 0:
                    w->model->pos[0] = 0.0f;
                    w->model->pos[1] = 292.0f;
                    w->model->pos[2] = 3600.0f;
                    w->model->rot[1] = 0.0f;
                    sndPlay(0x19A);
                    w->state++;
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.01f, 2);
                    break;
            }
            demo22LastPlacedX = w->model->pos[0];
            demo22LastPlacedZ = w->model->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Starts a "gate" under obj. */
Obj *demo22GateCreate(Obj *obj) {
    Obj *o;

    PRINTF("とびら作ります\n");
    o = tskCreate(obj, demo22GateProc, obj->file, NULL);
    PRINTF("とびら作りました\n");
    return o;
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
extern Gfx demo22SmokeDl[];                       /* the smoke's display list */
/* The jiji demo's frame count (tlb_0022_0, 0x08005A50): this smoke code is tlb_0022's
 * (kemuriProc, jijiDemoChimneySmoke) and kept its address; in this file 0x08005A50 is a word of
 * demo22BismaruProc's code, not demo22Frame (0x08007D40). */
extern s32 jijiDemoFrame;
extern AnimFrame demo22SmokeAnim[];       /* the smoke's texture animation */

s32 demo22KemuriProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    KemuriWork *w = (KemuriWork *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "KEMURI";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, demo22SmokeDl, 0x123);
            fadeAnimStart(&w->anim, demo22SmokeAnim, 0, &w->frame->segs[3], floadGetSetFile(0x123, 0));
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

void demo22KemuriCreate(f32 x, f32 y, f32 z, f32 scale, f32 grow, f32 rise, s32 alpha, s32 fade,
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
    tskCreate(gEffectTask, demo22KemuriProc, gDmenuLastObj->file, &arg);
}

/* Every 40 frames, a puff of smoke at (14, 12, 34). */
void demo22KemuriPuff(void) {
    if (jijiDemoFrame % 40 == 0) {
        demo22KemuriCreate(14.0f, 12.0f, 34.0f, 0.07f, 1.0f, 0.1f, 30, 1, 4);
    }
}

/* "TORCH" (its proc demo22TorchProc): its flame frame, texture animation and two
 * colours. */
typedef struct TorchW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ Anim anim;
    /* 0x14 */ u8 prim[3];
    /* 0x17 */ u8 env[3];
    /* 0x1A */ u8 alpha; /* the colours' alpha */
} TorchW;

extern Gfx demo22TorchFlameDl[];                   /* the flame's display list (file 0x125) */
extern AnimFrame demo22FlameAnim[];   /* the flame's texture animation */

/* "TORCH": a flame (file 0x125) with an animated texture, in its two colours; it stands still
 * while gDemoCamHold. */
s32 demo22TorchProc(Obj *obj, s16 msg, void *arg) {
    TorchW *w = (TorchW *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "TORCH";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, demo22TorchFlameDl, 0x125);
            fadeAnimStart(&w->anim, demo22FlameAnim, 0, &w->frame->segs[3], floadGetSetFile(0x125, 0));
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
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "TORCH" flame at x, y, z, scaled s, in two colours. */
void demo22TorchCreate(Obj *obj, f32 x, f32 y, f32 z, u8 pr, u8 pg, u8 pb, u8 er, u8 eg, u8 eb, f32 s,
                            u8 n) {
    Obj *o = tskCreate(obj, demo22TorchProc, obj->file, NULL);

    if (o != NULL) {
        ((TorchW *) FX(o))->frame->pos[0] = x;
        ((TorchW *) FX(o))->frame->pos[1] = y;
        ((TorchW *) FX(o))->frame->pos[2] = z;
        ((TorchW *) FX(o))->frame->scale[0] = ((TorchW *) FX(o))->frame->scale[1] = ((TorchW *) FX(o))->frame->scale[2] = s;
        ((TorchW *) FX(o))->frame->flags |= 0x22;
        ((TorchW *) FX(o))->alpha = n;
        ((TorchW *) FX(o))->prim[0] = pr;
        ((TorchW *) FX(o))->prim[1] = pg;
        ((TorchW *) FX(o))->prim[2] = pb;
        ((TorchW *) FX(o))->env[0] = er;
        ((TorchW *) FX(o))->env[1] = eg;
        ((TorchW *) FX(o))->env[2] = eb;
    }
}

/* The "ONARA" (fart cloud) task's work (as tlb_0032_1's): what its starter sets. */
typedef struct OnaraW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ Anim anim;
    /* 0x14 */ u8 pad14[0x10];
    /* 0x24 */ f32 rise; /* its rise each step */
    /* 0x28 */ f32 grow;  /* its scale's factor each step */
    /* 0x2C */ s32 n;     /* its alpha */
    /* 0x30 */ s32 fade; /* the alpha's fall each step */
} OnaraW;

#define ONARA(o) ((OnaraW *) FX(o))

extern AnimFrame demo22CloudAnim[]; /* the cloud's texture animation */

/* "ONARA": a fart cloud (the smoke's display list, file 0x123) that grows, rises and fades. */
s32 demo22OnaraProc(Obj *obj, s16 msg, void *arg) {
    OnaraW *w = (OnaraW *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "ONARA";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, demo22SmokeDl, 0x123);
            fadeAnimStart(&w->anim, demo22CloudAnim, 0, &w->frame->segs[3], floadGetSetFile(0x123, 0));
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = demoColorDL(200, 160, 40, 200, 160, 40, w->n);
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

/* An "ONARA" (demo22OnaraProc) under obj at x, y, z, scaled s, growing by 1.1 (fade
 * 10). */
void demo22OnaraCreate(Obj *obj, f32 x, f32 y, f32 z, s32 n, f32 s) {
    Obj *o;

    PRINTF("おなら作ります。\n");
    o = tskCreate(obj, demo22OnaraProc, obj->file, NULL);
    if (o != NULL) {
        PRINTF("おなら作りました。\n");
        ONARA(o)->frame->flags |= 0x27;
        ONARA(o)->frame->scale[0] = ONARA(o)->frame->scale[1] = ONARA(o)->frame->scale[2] = s;
        ONARA(o)->frame->pos[0] = x;
        ONARA(o)->frame->pos[1] = y;
        ONARA(o)->frame->pos[2] = z;
        ONARA(o)->n = n;
        ONARA(o)->fade = 10;
        ONARA(o)->rise = 0.0f;
        ONARA(o)->grow = 1.1f;
    }
}

/* "hyotan" (its proc demo22HyotanProc): its model, the gourd's frame, a frame count and a
 * state. */
typedef struct HyotanW {
    /* 0x00 */ AnimeFrame *model;
    /* 0x04 */ PolFrame *gourd;
    /* 0x08 */ u8 pad08[0x10];
    /* 0x18 */ s32 timer;
    /* 0x1C */ s32 state;
} HyotanW;

void demo22PieceCreate(Obj *obj, s32 v);

extern u8 demo22HyotanState3Anim[];          /* the gourd model's animations */
extern u8 demo22HyotanState4Anim[];
extern Gfx demo22GourdDl[];         /* the gourd's display list (file 0xC4) */
extern s32 demo22OniAnimStart; /* the frame its burst waits for (1: none yet) */
extern u8 demo22HyotanSndOnce;  /* 1: play sound 0x196 once */

/* The file's .data, in address order (its externs above stay as declarations). */
extern u8 demo22SmokeTex0[];
extern u8 demo22FlameTex0[];
extern u8 demo22SmokeTex1[];
extern u8 demo22SmokeTex2[];
extern u8 demo22FlameTex2[];
extern u8 demo22SmokeTex3[];
extern u8 demo22SmokeTex4[];
extern u8 demo22FlameTex5[];
extern u8 demo22SmokeTex6[];
extern u8 demo22OniTex0[];
extern u8 demo22OniTex1[];
u32 demo22GoeSpeed = 0xBF800000;
Player *demo22GoePlayer = (Player *) 0x01234567; /* Goemon's player */
u32 demo22SuzakuAnimUnused[] = { 0x03009E30, 0x00080000, 0x0300A250, 0x000A0000, 0x00000000, 0x00000000 };
AnimFrame demo22SuzakuAnim[] = {
    { 0x0300A670, 8 },
    { 0x00000000, 0 },
}; /* suzaku's texture animation */
AnimFrame demo22OniAnim[] = {
    { (u32) demo22OniTex0, 8 },
    { (u32) demo22OniTex1, 10 },
    { 0x00000000, 0 },
}; /* the oni's texture animations */
AnimFrame demo22SmokeAnim[] = {
    { (u32) demo22SmokeTex0, 6 },
    { (u32) demo22SmokeTex1, 6 },
    { (u32) demo22SmokeTex2, 6 },
    { (u32) demo22SmokeTex3, 6 },
    { (u32) demo22SmokeTex4, 6 },
    { 0x03002D30, 8 },
    { (u32) demo22SmokeTex6, 10 },
    { 0x00000000, 0 },
};       /* the smoke's texture animation */
AnimFrame demo22FlameAnim[] = {
    { (u32) demo22FlameTex0, 2 },
    { 0x03000D80, 2 },
    { (u32) demo22FlameTex2, 2 },
    { 0x03001D80, 2 },
    { 0x03002580, 2 },
    { (u32) demo22FlameTex5, 2 },
    { 0x03003580, 2 },
    { 0x03003D80, 2 },
    { 0x03004580, 2 },
    { 0x03004D80, 2 },
    { 0x03005580, 2 },
    { 0x03005D80, 2 },
    { 0x03006580, 2 },
    { 0x03006D80, 2 },
    { 0x03007580, 2 },
    { 0x03007D80, 2 },
    { 0x00000000, 0 },
};   /* the flame's texture animation */
AnimFrame demo22CloudAnim[] = {
    { (u32) demo22SmokeTex0, 6 },
    { (u32) demo22SmokeTex1, 6 },
    { (u32) demo22SmokeTex2, 6 },
    { (u32) demo22SmokeTex3, 6 },
    { (u32) demo22SmokeTex4, 6 },
    { 0x03002D30, 8 },
    { (u32) demo22SmokeTex6, 10 },
    { 0x00000000, 0 },
}; /* the cloud's texture animation */
u8 demo22HyotanSndOnce = 0x01;  /* 1: play sound 0x196 once */
extern u8 demo22HyotanSndOnceClr; /* the same flag, as state 3 clears it (undefined_syms.txt) */

/* "hyotan": the gourd (file 0xC4); OBJMOVE's model 8 moves both its frames. From frame 0x11D6 it
 * flies in, lands at (0, 428, 960) and slides to z 240, then bursts into 13 "piece"s and an
 * "ONARA" 40 frames after demo22OniAnimStart. The sound flag's clear is written under a second
 * name (undefined_syms.txt): the game forms its address again for the store. */
s32 demo22HyotanProc(Obj *obj, s16 msg, void *arg);
s32 demo22HyotanProc(Obj *obj, s16 msg, void *arg) {
    HyotanW *w = (HyotanW *) (obj + 1);
    f32 scale;

    switch (msg) {
        case 2:
            obj->name = "hyotan";
            w->gourd = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->gourd == NULL) {
                return -1;
            }
            demoPolSetDL(w->gourd, demo22GourdDl, 0xC4);
            w->gourd->scale[1] = 1.0f;
            scale = w->gourd->scale[1];
            w->gourd->scale[2] = scale;
            w->gourd->scale[0] = scale;
            w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->model == NULL) {
                return -1;
            }
            demoAnimeSet(w->model, (u32) demo22HyotanState3Anim, 0xC4, 0.0f);
            w->state = 0;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->rot[1] = 0.0f;
            w->model->pos[0] = w->model->pos[2] = w->model->rot[1];
            w->model->pos[1] = -200.0f;
            w->gourd->pos[2] = 0.0f;
            w->gourd->pos[0] = w->gourd->pos[1] = w->gourd->pos[2];
            break;
        case 1:
            frmDestroy((Frame *) w->model);
            frmDestroy((Frame *) w->gourd);
            break;
        case 3:
            break;
        case 0:
            w->timer++;
            w->model->f.flags |= 2;
            w->gourd->f.flags |= 2;
            if (gObjMoveSel == 8) {
                w->model->pos[0] += gObjMoveDx;
                w->model->pos[1] += gObjMoveDy;
                w->model->pos[2] += gObjMoveDz;
                w->model->rot[1] += gObjMoveDrot;
                gObjMovePosX = w->model->pos[0];
                gObjMovePosY = w->model->pos[1];
                gObjMovePosZ = w->model->pos[2];
                gObjMoveRot = w->model->rot[1];
                if ((u32) gGameState.frameCount % 20 >= 10) {
                    w->model->f.flags |= 2;
                } else {
                    w->model->f.flags &= ~2;
                }
                w->gourd->pos[0] += gObjMoveDx;
                w->gourd->pos[1] += gObjMoveDy;
                w->gourd->pos[2] += gObjMoveDz;
                w->gourd->rot[1] += gObjMoveDrot;
                gObjMovePosX = w->gourd->pos[0];
                gObjMovePosY = w->gourd->pos[1];
                gObjMovePosZ = w->gourd->pos[2];
                gObjMoveRot = w->gourd->rot[1];
                if ((u32) gGameState.frameCount % 20 >= 10) {
                    w->gourd->f.flags |= 2;
                } else {
                    w->gourd->f.flags &= ~2;
                }
            }
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            switch (w->state) {
                case 0:
                    if (w->timer == 0x11D6) {
                        w->gourd->pos[0] = -170.0f;
                        w->gourd->pos[1] = -140.0f;
                        w->gourd->pos[2] = 1800.0f;
                        sndPlay(0x19D);
                        w->state++;
                    }
                    break;
                case 1:
                    w->gourd->pos[1] += 2.0f;
                    w->gourd->pos[2] -= 2.0f;
                    if (w->timer >= 0x11DF) {
                        w->gourd->pos[1] += 8.0f;
                        w->gourd->pos[2] -= 8.0f;
                    }
                    if (w->timer >= 0x12B6 || w->gourd->pos[2] < 0.0f) {
                        w->state++;
                    }
                    break;
                case 2:
                    if (w->timer >= 0x12D5) {
                        w->gourd->pos[1] = 0.0f;
                        w->model->pos[0] = 0.0f;
                        w->model->pos[1] = 428.0f;
                        w->model->pos[2] = 960.0f;
                        w->state++;
                    }
                    break;
                case 3:
                    if (240.0f != w->model->pos[2]) {
                        w->model->pos[1] += 6.0f;
                        w->model->pos[2] -= 10.0f;
                    } else {
                        if (w->timer >= 0x133B) {
                            demoAnimeSet(w->model, (u32) demo22HyotanState3Anim, 0xC4, 0.0f);
                            w->state++;
                        }
                        if (demo22HyotanSndOnce) {
                            sndPlay(0x196);
                            demo22HyotanSndOnceClr = 0;
                        }
                    }
                    break;
                case 4:
                    if (demo22OniAnimStart == 1) {
                        demoAnimeStep(w->model, 0.2f, 0);
                    } else if (demo22OniAnimStart == w->timer) {
                        demoAnimeSet(w->model, (u32) demo22HyotanState4Anim, 0xC4, 0.0f);
                    } else {
                        demoAnimeStep(w->model, 0.2f, 2);
                    }
                    if (demo22OniAnimStart + 40 < w->timer && demo22OniAnimStart != 1) {
                        w->model->pos[1] = 0.0f;
                        (*demo22PieceCreate)(obj, 0);
                        (*demo22PieceCreate)(obj, 1);
                        (*demo22PieceCreate)(obj, 2);
                        (*demo22PieceCreate)(obj, 2);
                        (*demo22PieceCreate)(obj, 2);
                        (*demo22PieceCreate)(obj, 2);
                        (*demo22PieceCreate)(obj, 3);
                        (*demo22PieceCreate)(obj, 3);
                        (*demo22PieceCreate)(obj, 3);
                        (*demo22PieceCreate)(obj, 3);
                        (*demo22PieceCreate)(obj, 3);
                        (*demo22PieceCreate)(obj, 3);
                        (*demo22PieceCreate)(obj, 3);
                        (*demo22OnaraCreate)(obj, 90.0f, 1240.0f, 340.0f, 0xC0, 1.0f);
                        w->state++;
                    }
                    break;
                case 5:
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Starts a "hyotan" under obj (as the gate's starter, demo22GateCreate). */
Obj *demo22HyotanCreate(Obj *obj) {
    Obj *o;

    PRINTF("ひょうたん作ります\n");
    o = tskCreate(obj, demo22HyotanProc, obj->file, NULL);
    PRINTF("ひょうたん作りました\n");
    return o;
}

/* "piece" (its proc demo22PieceProc): a flying fragment's frame, its frame count, state,
 * kind (what its starter sets), its move and its spin each frame. */
typedef struct PieceW {
    /* 0x00 */ u8 pad00[4];
    /* 0x04 */ PolFrame *frame;
    /* 0x08 */ u8 pad08[0x10];
    /* 0x18 */ s32 timer;
    /* 0x1C */ s32 state;
    /* 0x20 */ s32 kind; /* the kind: bit 0 picks the display list, 0 and 1 fixed flights */
    /* 0x24 */ f32 vel[3];
    /* 0x30 */ f32 spin[3];
} PieceW;

extern Gfx demo22Piece1Dl[]; /* the fragments' display lists (file 0xC4) */
extern Gfx demo22Piece2Dl[];

/* "piece": a fragment (file 0xC4) thrown from (90, 1240, 340) that spins and flies for 300
 * frames: kinds 0 and 1 on set paths, the others at random; OBJMOVE's model 8 moves it. */
s32 demo22PieceProc(Obj *obj, s16 msg, void *arg) {
    PieceW *w = (PieceW *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "piece";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            if (w->kind & 1) {
                demoPolSetDL(w->frame, demo22Piece1Dl, 0xC4);
            } else {
                demoPolSetDL(w->frame, demo22Piece2Dl, 0xC4);
            }
            w->frame->scale[1] = 1.0f;
            w->frame->scale[0] = w->frame->scale[2] = w->frame->scale[1];
            w->state = 0;
            w->frame->pos[0] = 90.0f;
            w->frame->pos[1] = 1240.0f;
            w->frame->pos[2] = 340.0f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            break;
        case 0:
            w->timer++;
            w->frame->f.flags |= 2;
            if (gObjMoveSel == 8) {
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
            switch (w->state) {
                case 0:
                    switch (w->kind) {
                        case 0:
                            w->spin[1] = w->spin[0] = 0.05235987902f;
                            w->vel[0] = 1.0f;
                            w->spin[2] = 0.0f;
                            w->vel[1] = 3.0f;
                            break;
                        case 1:
                            w->spin[0] = 0.0f;
                            w->spin[1] -= 0.034906586007760157;
                            w->spin[2] += 0.0523598790116402354;
                            w->vel[0] -= 1.0f;
                            w->vel[1] -= 3.0f;
                            w->vel[2] += 1.0f;
                            break;
                        default:
                            w->spin[0] = (fadeRand(2) + 1) * (fadeRand(2) ? 1 : -1) * 0.0174532930038800785;
                            w->spin[1] = (fadeRand(3) + 1) * (fadeRand(2) ? 1 : -1) * 0.0174532930038800785;
                            w->spin[2] = (fadeRand(2) + 1) * (fadeRand(2) ? 1 : -1) * 0.0174532930038800785;
                            w->vel[0] = (fadeRand(5) + 1) * (fadeRand(2) ? 1 : -1);
                            w->vel[1] = (fadeRand(3) + 1) * (fadeRand(2) ? 1 : -1);
                            w->vel[2] = (fadeRand(1) + 1) * (fadeRand(2) ? 1 : -1);
                            break;
                    }
                    w->state++;
                case 1:
                    w->frame->rot[0] += w->spin[0];
                    w->frame->rot[1] += w->spin[1];
                    w->frame->rot[2] += w->spin[2];
                    w->frame->pos[0] += w->vel[0];
                    w->frame->pos[1] += w->vel[1];
                    w->frame->pos[2] += w->vel[2];
                    if (w->timer >= 300) {
                        return -1;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Starts a "piece" under obj with v in its work. */
void demo22PieceCreate(Obj *obj, s32 v) {
    Obj *o;

    PRINTF("破片作ります\n");
    o = tskCreate(obj, demo22PieceProc, obj->file, NULL);
    if (o != NULL) {
        PRINTF("破片作りました\n");
        ((PieceW *) (o + 1))->kind = v;
    }
}
