#include "tlb.h"
#include "tlb/tlb_0026.h"
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
/* This file's .data (0x08005080): the characters' texture animations (demotool.c's table, as
 * tlb_0020_1's; nothing reads it), then Suzaku's. */
AnimFrame st2clearCharaAnims[] = {
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
AnimFrame st2clearSuzakuAnim[] = {
    { 0x03009E30, 10 },
    { 0x0300A250, 8 },
    { 0, 0 },
};

/* The file's .bss (0x08005FA0 on), laid out by first declaration: these globals in address
 * order, then the demo characters' statics in function order. The words nothing reads are
 * declared where they sit. */
s32 st2clearFrame;        /* the demo's frame count (tlb_0026_0 counts it) */
s32 st2clearUnused3[7];     /* unread */
s32 st2clearSuzakuShadowAlpha;        /* suzaku's shadow alpha */
s32 st2clearUnused4;        /* unread */
f32 st2clearSuzakuAnimSpeed;        /* suzaku's last animation speed */
s32 st2clearRoadAlpha;        /* the road's colour alpha */
s32 st2clearUnused5;        /* unread */
s32 st2clearUnused6;        /* unread */
f32 st2clearSasX;        /* Sasuke's x and z, each frame */
f32 st2clearSasZ;
f32 st2clearGoeX;        /* Goemon's x and z, each frame */
f32 st2clearGoeZ;
f32 st2clearEbiX;        /* Ebisumaru's x and z, each frame */
f32 st2clearEbiZ;
f32 st2clearYaeX;        /* Yae's x and z, each frame */
f32 st2clearYaeZ;
f32 st2clearTurnX;        /* the old man's x and z: the point the others turn toward */
f32 st2clearTurnZ;
s32 st2clearUnused7;        /* unread */
s32 st2clearUnused8;        /* unread */

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

extern Gfx st2clearShadowDl[]; /* the shadow */
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

/* "genoGOE" (Goemon, character 0, in the demo) at (-700, 1, 827) under the menu's last task,
 * the twin of tlb_0030_1's (its three statics, here in .bss); OBJMOVE's model 0, his shadow 0.3
 * above his feet. 23 steps on the demo's frame count, linking his actions (scene runs 0xB7,
 * 0xBC, 0xC0, 0xC2, 0xC3, 0xC7 and 0xC8). Case 6's literal 0.3 (a double) keeps the shadow's
 * 0.3f from being reused there. */
s32 st2clearGoeProc(Obj *obj, s16 msg, void *arg) {
    static s32 step;  /* .bss 0x0800600C: his demoPlTunagiAnime step */
    static f32 speed; /* .bss 0x08006010: unused here (genoGOE's fall speed) */
    static Player *p; /* .bss 0x08006014: his player */
    s32 pad[2];
    DemoPlW *w = (DemoPlW *) (obj + 1);
    PlInit init;

    DPRINTF("goecreate\n"); /* the file's rodata has it first, before the name */
    switch (msg) {
        case 2:
            obj->name = "genoGOE";
            plMakeInit(&init, 0, 0);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = -700.0f;
            init.height = 1.0f;
            init.side = 827.0f;
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
            demoPolSetDL(w->shadow, st2clearShadowDl, 0xBC);
            w->shadow->scale[2] = 0.07999999821f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->chara = 0;
            w->model->rot[1] = 3.1415927f;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -700.0f;
            w->model->pos[1] = 1.0f;
            w->model->pos[2] = 827.0f;
            w->model->rot[1] = 2.5132742f;
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
            w->shadow->pos[1] = w->model->pos[1] + 0.3f;
            st2clearGoeX = w->model->pos[0];
            st2clearGoeZ = w->model->pos[2];
            switch (w->state) {
                case 0:
                    w->model->scale[2] = 0.097f;
                    w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                    w->model->pos[0] = -700.0f;
                    w->model->pos[1] = 1.0f;
                    w->model->pos[2] = 827.0f;
                    w->model->rot[1] = 2.5132742f;
                    w->state++;
                    break;
                case 1:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame >= 0x12C) {
                        step = 0;
                        w->state++;
                    }
                    break;
                case 2:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.5f, 0xEB, 0xEC)) == -1) {
                        w->state++;
                        step = 0;
                        scRunEnd();
                        scRunStart(0xB7, 0, 0);
                    }
                    break;
                case 3:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.5f, 0);
                    if (st2clearFrame >= 0x2BC) {
                        w->state++;
                    }
                    break;
                case 4:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.5f, 0.2f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 5:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame >= 0x992) {
                        step = 0;
                        w->state++;
                    }
                    break;
                case 6:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 2, 0.3, 0.4f, 0x10A, 0x10B)) == -1) {
                        w->state++;
                        step = 0;
                        scRunEnd();
                        scRunStart(0xBC, 0, 0);
                    }
                    break;
                case 7:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], st2clearTurnX,
                                                      st2clearTurnZ, w->model->rot[1], 1.0f);
                    if (st2clearFrame >= 0xA78) {
                        w->state++;
                    }
                    break;
                case 8:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame >= 0xD34) {
                        step = 0;
                        w->state++;
                    }
                    break;
                case 9:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.5f, 0x10C, 0x10D)) == -1) {
                        w->state++;
                        step = 0;
                        scRunEnd();
                        scRunStart(0xC0, 0, 0);
                    }
                    break;
                case 10:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.5f, 0);
                    if (st2clearFrame >= 0xF5A) {
                        w->state++;
                        scRunEnd();
                    }
                    break;
                case 11:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.5f, 0.2f, 0x10B, 0x10B)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 12:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame >= 0x12F2) {
                        scRunEnd();
                        scRunStart(0xC2, 0, 0);
                        demoTexAnimK0(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 13:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame >= 0x13BA) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 14:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.5f, 0x10C, 0x10D)) == -1) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0xC3, 0, 0);
                    }
                    break;
                case 15:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.5f, 0);
                    if (st2clearFrame >= 0x14B4) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 16:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.5f, 0.2f, 0x10B, 0x10B)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 17:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame >= 0x1932) {
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK0(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->state++;
                        step = 0;
                    }
                    break;
                case 18:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame >= 0x1BEE) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 19:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.5f, 0xEB, 0xEC)) == -1) {
                        w->state++;
                        step = 0;
                        scRunEnd();
                        scRunStart(0xC7, 0, 0);
                    }
                    break;
                case 20:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.5f, 0);
                    if (st2clearFrame >= 0x1E46) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0xC8, 0, 0);
                    }
                    break;
                case 21:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.5f, 0);
                    if (st2clearFrame >= 0x206C) {
                        w->state++;
                        step = 0;
                        scRunEnd();
                    }
                    break;
                case 22:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.5f, 0.2f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 23:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *st2clearGoeCreate(Obj *obj) {
    return tskCreate(obj, st2clearGoeProc, obj->file, NULL);
}

/* "genoGOE" (the game's name; it is Ebisumaru, character 1, and prints "EBI STEP") at
 * (-689, 1, 804) under the menu's last task; OBJMOVE's model 1, his shadow 0.3 above his feet.
 * Twelve steps on the demo's frame count (scene runs 0xB8 and 0xC6); at 0x1932 he is put back
 * where he started. */
s32 st2clearEbiProc(Obj *obj, s16 msg, void *arg) {
    static s32 tunagiStep;  /* .bss 0x08006018 */
    static s32 reserved1;   /* .bss 0x0800601C-0x08006024: three words of the game's .bss */
    static s32 reserved2;   /* that nothing reads (scalars: IDO 8-aligns a .bss array) */
    static s32 reserved3;
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
            init.place = -689.0f;
            init.height = 1.0f;
            init.side = 804.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            DPRINTF("歩きモーション\n");
            plAnimeSet(PL(w->pl), 0xE9);
            DPRINTF("EBI STEP --->%d\n", w->state);
            w->model = p->model;
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, st2clearShadowDl, 0xBC);
            w->shadow->scale[2] = 0.09f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 1;
            w->state = 0;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -689.0f;
            w->model->pos[1] = 1.0f;
            w->model->pos[2] = 804.0f;
            w->model->rot[1] = 0.0f;
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
            w->shadow->pos[1] = w->model->pos[1] + 0.3f;
            st2clearEbiX = w->model->pos[0];
            st2clearEbiZ = w->model->pos[2];
            switch (w->state) {
                case 0:
                    w->model->pos[0] = -689.0f;
                    w->model->pos[1] = 1.0f;
                    w->model->pos[2] = 804.0f;
                    w->model->rot[1] = 0.0f;
                    demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                    demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    w->state++;
                    break;
                case 1:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame >= 0x2BC) {
                        tunagiStep = 0;
                        w->state++;
                    }
                    break;
                case 2:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 2, tunagiStep, 2, 2, 0.2f, 0.5f, 0xEB, 0xEC)) == -1) {
                        w->state++;
                        tunagiStep = 0;
                        scRunEnd();
                        scRunStart(0xB8, 0, 0);
                    }
                    break;
                case 3:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.5f, 0);
                    if (st2clearFrame >= 0x44C) {
                        w->state++;
                    }
                    break;
                case 4:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 0, tunagiStep, 2, 3, 0.5f, 0.2f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 5:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame >= 0x9C4) {
                        w->state++;
                    }
                    break;
                case 6:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], st2clearTurnX,
                                                      st2clearTurnZ, w->model->rot[1], 1.0f);
                    tunagiStep = demoPlTunagiAnime(obj, 0, tunagiStep, 2, 2, 0.2f, 0.2f, 0x10A, 0x10B);
                    if (tunagiStep == -1 && st2clearFrame >= 0xA78) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 7:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame == 0x1932) {
                        w->model->pos[0] = -689.0f;
                        w->model->pos[1] = 1.0f;
                        w->model->pos[2] = 804.0f;
                        w->model->rot[1] = 0.0f;
                        w->state++;
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 8:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame >= 0x1996) {
                        w->state++;
                    }
                    break;
                case 9:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 2, tunagiStep, 2, 2, 0.2f, 0.5f, 0xEB, 0xEC)) == -1) {
                        w->state++;
                        tunagiStep = 0;
                        scRunEnd();
                        scRunStart(0xC6, 0, 0);
                    }
                    break;
                case 10:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.5f, 0);
                    if (st2clearFrame >= 0x1BBC) {
                        w->state++;
                    }
                    break;
                case 11:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 0, tunagiStep, 2, 3, 0.5f, 0.2f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                    }
                    break;
                case 12:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *st2clearEbiCreate(Obj *obj) {
    return tskCreate(obj, st2clearEbiProc, obj->file, NULL);
}

/* "chaSAS" (Sasuke, character 2, in the demo) at (-671, 1, 829) under the menu's last task;
 * OBJMOVE's model 2, his shadow 0.3 above his feet. Twelve steps on the demo's frame count,
 * linking his actions (scene runs 0xB9 and 0xC4); at 0x1932 he is put back where he started. */
s32 st2clearSasProc(Obj *obj, s16 msg, void *arg) {
    static s32 tunagiStep; /* .bss 0x08006028 */
    static s32 reserved;   /* .bss 0x0800602C: a word of the game's .bss that nothing reads */
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
            init.place = -671.0f;
            init.height = 1.0f;
            init.side = 829.0f;
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
            demoPolSetDL(w->shadow, st2clearShadowDl, 0xBC);
            w->shadow->scale[2] = 0.065f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 2;
            w->state = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -671.0f;
            w->model->pos[1] = 1.0f;
            w->model->pos[2] = 829.0f;
            w->model->rot[1] = 3.5430186f;
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
            st2clearSasX = w->model->pos[0];
            st2clearSasZ = w->model->pos[2];
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = w->model->pos[1] + 0.3f;
            switch (w->state) {
                case 0:
                    w->model->pos[0] = -671.0f;
                    w->model->pos[1] = 1.0f;
                    w->model->pos[2] = 829.0f;
                    w->model->rot[1] = 3.5430186f;
                    w->state++;
                    break;
                case 1:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame >= 0x44C) {
                        tunagiStep = 0;
                        w->state++;
                    }
                    break;
                case 2:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 2, tunagiStep, 2, 2, 0.2f, 0.5f, 0xEB, 0xEC)) == -1) {
                        w->state++;
                        tunagiStep = 0;
                        scRunEnd();
                        scRunStart(0xB9, 0, 0);
                    }
                    break;
                case 3:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.5f, 0);
                    if (st2clearFrame >= 0x5DC) {
                        w->state++;
                    }
                    break;
                case 4:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 0, tunagiStep, 2, 3, 0.5f, 0.2f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 5:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame >= 0x9C4) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 6:
                    tunagiStep = demoPlTunagiAnime(obj, 0, tunagiStep, 2, 2, 0.2f, 0.2f, 0x10A, 0x10B);
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], st2clearTurnX,
                                                      st2clearTurnZ, w->model->rot[1], 1.0f);
                    if (tunagiStep == -1 && st2clearFrame >= 0xA78) {
                        tunagiStep = 0;
                        w->state++;
                    }
                    break;
                case 7:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame >= 0x154A) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 8:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 2, tunagiStep, 2, 2, 0.2f, 0.5f, 0x10C, 0x10D)) == -1) {
                        w->state++;
                        tunagiStep = 0;
                        scRunEnd();
                        scRunStart(0xC4, 0, 0);
                    }
                    break;
                case 9:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.5f, 0);
                    if (st2clearFrame >= 0x16DA) {
                        w->state++;
                    }
                    break;
                case 10:
                    if ((tunagiStep = demoPlTunagiAnime(obj, 0, tunagiStep, 2, 3, 0.5f, 0.2f, 0x10B, 0x10B)) == -1) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 11:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame == 0x1932) {
                        w->model->pos[0] = -671.0f;
                        w->model->pos[1] = 1.0f;
                        w->model->pos[2] = 829.0f;
                        w->model->rot[1] = 3.5430186f;
                        w->state++;
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK2(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 12:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *st2clearSasCreate(Obj *obj) {
    return tskCreate(obj, st2clearSasProc, obj->file, NULL);
}

/* "chaYAE" (Yae, character 3, in the demo) at (-668, 1, 805) under the menu's last task, the
 * twin of tlb_0029_2's: OBJMOVE's model 3 moves her, and her shadow follows her 0.3 above her
 * feet. Twelve steps on the demo's frame count: she stands; at 1500 she plays action 0x101
 * (faces 2) and scene 0xBA starts; at 2000 she goes back to 0xD7; at 2500 she turns toward a
 * point and links actions 0x10A and 0x10B; at 5850 action 0x10D and scene 0xC5; at 6350 action
 * 0x10B; at 6450 she is put back where she started.
 * Her tunagi step is a static of this function, in this file's .bss at 0x08006030 (the
 * game reaches it with a fresh lui at every use, IDO_NOTES 38). `0.3` (not 0.3f) in
 * demoAnimeStep keeps the shadow's 0.3f from being reused in case 4. */
s32 st2clearYaeProc(Obj *obj, s16 msg, void *arg) {
    static s32 tunagiStep;
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
            init.place = -668.0f;
            init.height = 1.0f;
            init.side = 805.0f;
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
            demoPolSetDL(w->shadow, st2clearShadowDl, 0xBC);
            w->shadow->scale[2] = 0.065f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 3;
            w->state = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -668.0f;
            w->model->pos[1] = 1.0f;
            w->model->pos[2] = 805.0f;
            w->model->rot[1] = -0.7679448724f;
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
            st2clearYaeX = w->model->pos[0];
            st2clearYaeZ = w->model->pos[2];
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = w->model->pos[1] + 0.3f;
            switch (w->state) {
                case 0:
                    w->state++;
                    w->model->pos[0] = -668.0f;
                    w->model->pos[1] = 1.0f;
                    w->model->pos[2] = 805.0f;
                    w->model->rot[1] = -0.7679448724f;
                    break;
                case 1:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame >= 1500) {
                        w->state++;
                    }
                    break;
                case 2:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (demoAnimeStep(w->model, 0.3, 2)) {
                        plAnimeSet(PL(w->pl), 0x101);
                        demoTexAnimK3(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                        w->state++;
                        scRunEnd();
                        scRunStart(0xBA, 0, 0);
                    }
                    break;
                case 3:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame >= 2000) {
                        w->state++;
                    }
                    break;
                case 4:
                    if (demoAnimeStep(w->model, 0.3, 2)) {
                        plAnimeSet(PL(w->pl), 0xD7);
                        demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        scRunEnd();
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 5:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame >= 2500) {
                        tunagiStep = 0;
                        w->state++;
                    }
                    break;
                case 6:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], st2clearTurnX, st2clearTurnZ,
                                                      w->model->rot[1], 1.0f);
                    tunagiStep = demoPlTunagiAnime(obj, 0, tunagiStep, 2, 2, 0.3f, 0.2f, 0x10A, 0x10B);
                    if (tunagiStep == -1 && st2clearFrame >= 2680) {
                        w->state++;
                        tunagiStep = 0;
                    }
                    break;
                case 7:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame >= 5850) {
                        w->state++;
                    }
                    break;
                case 8:
                    if (demoAnimeStep(w->model, 0.3, 2)) {
                        plAnimeSet(PL(w->pl), 0x10D);
                        demoTexAnimK3(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                        w->state++;
                        scRunEnd();
                        scRunStart(0xC5, 0, 0);
                    }
                    break;
                case 9:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (st2clearFrame >= 6350) {
                        if (demoAnimeStep(w->model, 0.3, 2)) {
                            plAnimeSet(PL(w->pl), 0x10B);
                            demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                            w->state++;
                            scRunEnd();
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
                case 10:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame >= 6450) {
                        w->model->pos[0] = -668.0f;
                        w->model->pos[1] = 1.0f;
                        w->model->pos[2] = 805.0f;
                        w->model->rot[1] = -0.7679448724f;
                        w->state++;
                        plAnimeSet(PL(w->pl), 0xD7);
                        demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 11:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *st2clearYaeCreate(Obj *obj) {
    return tskCreate(obj, st2clearYaeProc, obj->file, NULL);
}

/* "suzaku"'s work (tlb_0030_1's SuzakuW). */
typedef struct SuzakuW {
    /* 0x00 */ AnimeFrame *model;
    /* 0x04 */ PolFrame *shadow;
    /* 0x08 */ Anim anim;
    /* 0x18 */ s32 timer;
    /* 0x1C */ s32 state;
} SuzakuW;

extern u8 st2clearSuzakuState8Anim[]; /* the old man's animations (file 0xCD) */
extern u8 st2clearSuzakuState3Anim[];
extern u8 st2clearSuzakuState9Anim[];
extern u8 st2clearSuzakuState10Anim[];

/* "suzaku": the wise old man (file 0xCD) at (-685, 102, 706); OBJMOVE's model 4. He plays his
 * animations by the demo's frame count and starts scene runs 0xBB, 0xBD and 0xC1; at the end he
 * vanishes in a puff, 100 up, with sound 0x19E.
 * Its pointers colour v1 as the game's only with case 4's FAKE (see there). */
s32 st2clearSuzakuProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    SuzakuW *w = (SuzakuW *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "suzaku";
            DPRINTF("物知りじいさん作ります\n");
            w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->model == NULL) {
                return -1;
            }
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoAnimeSet(w->model, (u32) st2clearSuzakuState8Anim, 0xCD, 0.0f);
            fadeAnimStart(&w->anim, st2clearSuzakuAnim, 0, &w->model->segs[3], floadGetSetFile(0xCD, 0));
            demoPolSetDL(w->shadow, st2clearShadowDl, 0xBC);
            w->shadow->scale[2] = 0.07999999821f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->model->scale[2] = 0.09700000286f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -685.0f;
            w->model->pos[1] = 102.0f;
            w->model->pos[2] = 706.0f;
            w->model->rot[1] = -0.174532935f;
            DPRINTF("物知りじいさん作りました\n");
            break;
        case 1:
            frmDestroy(&w->model->f);
            frmDestroy(&w->shadow->f);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, st2clearSuzakuShadowAlpha);
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
                    w->model->pos[0] = -685.0f;
                    w->model->pos[1] = 102.0f;
                    w->model->pos[2] = 706.0f;
                    w->model->rot[1] = -0.174532935f;
                    w->state++;
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame >= 0x834) {
                        scRunEnd();
                        scRunStart(0xBB, 0, 0);
                        w->state++;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame >= 0xA78) {
                        w->state++;
                    }
                    break;
                case 3:
                    if (demoAnimeStep(w->model, 0.2f, 2)) {
                        demoAnimeSet(w->model, (u32) st2clearSuzakuState3Anim, 0xCD, 0.0f);
                        fadeAnimStart(&w->anim, st2clearSuzakuAnim, 1, &w->model->segs[3], floadGetSetFile(0xCD, 0));
                        w->state++;
                        scRunEnd();
                        scRunStart(0xBD, 0, 0);
                    }
                    break;
                case 4:
                    fadeAnimStep(&w->anim);
                    demoAnimeStep(w->model, 0.2f, 0);
                    /* FAKE (the permuter's find): two dead reads of w->model, which colour the
                     * model and shadow pointers v1 as the game does. Probably a compiled-out
                     * debug check; a guarded DPRINTF does not reproduce it. */
                    if (w->model) {}
                    if (w->model) {}
                    if (st2clearFrame >= 0xCD0) {
                        w->state++;
                    }
                    break;
                case 5:
                    fadeAnimStart(&w->anim, st2clearSuzakuAnim, 0, &w->model->segs[3], floadGetSetFile(0xCD, 0));
                    scRunEnd();
                    w->state++;
                    break;
                case 6:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st2clearFrame >= 0xFBE) {
                        w->state++;
                    }
                    break;
                case 7:
                    fadeAnimStart(&w->anim, st2clearSuzakuAnim, 1, &w->model->segs[3], floadGetSetFile(0xCD, 0));
                    w->state++;
                    scRunEnd();
                    scRunStart(0xC1, 0, 0);
                    break;
                case 8:
                    fadeAnimStep(&w->anim);
                    if (st2clearFrame >= 0x11E4) {
                        if (demoAnimeStep(w->model, 0.3f, 2)) {
                            demoAnimeSet(w->model, (u32) st2clearSuzakuState8Anim, 0xCD, 0.0f);
                            fadeAnimStart(&w->anim, st2clearSuzakuAnim, 0, &w->model->segs[3],
                                          floadGetSetFile(0xCD, 0));
                            scRunEnd();
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
                case 9:
                    if (demoAnimeStep(w->model, 0.2f, 2)) {
                        demoAnimeSet(w->model, (u32) st2clearSuzakuState9Anim, 0xCD, 0.0f);
                        fadeAnimStart(&w->anim, st2clearSuzakuAnim, 0, &w->model->segs[3], floadGetSetFile(0xCD, 0));
                        w->state++;
                    }
                    break;
                case 10:
                    if (demoAnimeStep(w->model, 0.1f, 2)) {
                        demoAnimeSet(w->model, (u32) st2clearSuzakuState10Anim, 0xCD, 0.0f);
                        fadeAnimStart(&w->anim, st2clearSuzakuAnim, 0, &w->model->segs[3], floadGetSetFile(0xCD, 0));
                        w->state++;
                    }
                    break;
                case 11:
                    if (demoAnimeStep(w->model, 0.4f, 2)) {
                        w->state++;
                        scRunEnd();
                        itexPuff(w->model->pos[0], w->model->pos[1] + 10.0f, w->model->pos[2] + 10.0f, 0.2f);
                        w->model->pos[1] += 100.0f;
                        sndPlay(0x19E);
                    }
                    break;
                case 12:
                    demoAnimeStep(w->model, st2clearSuzakuAnimSpeed, 0);
                    break;
            }
            st2clearTurnX = w->model->pos[0];
            st2clearTurnZ = w->model->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *st2clearSuzakuCreate(Obj *obj) {
    return tskCreate(obj, st2clearSuzakuProc, obj->file, NULL);
}

/* "test"'s work. */
typedef struct RoadTestW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ PolFrame *pol;
    /* 0x08 */ u8 pad08[0x14];
    /* 0x1C */ s32 state;
} RoadTestW;

extern u8 st2clearRoadAnim[];   /* the road's animation */

/* "test" (道テスト, a road test): the road's animation of file 0xCE at the origin, stepped at
 * half speed from its second frame; its pol frame is drawn in near-black at st2clearRoadAlpha. */
#define ROADTEST(obj) ((RoadTestW *) FX(obj))

/* Matched 2026-10-12. The history, when parked: 62 lines, all in case 1 and the frame: the game computes w before frmDestroy's address and
 * keeps both on the stack across the first call (0x48 frame); here w folds into s0's offsets.
 * w at its declaration homes obj instead (133 lines); w through the macro at each use, 98.
 * 2026-10-11: case 1's empty if (IDO_NOTES 311) and pad[2] (the frame's 8 bytes) -> real 7,
 * left in case 0: the switch load and demoAnimeStep's w->frame still fold (0x3C/0x20(s0)).
 * Measured there: the same empty if (real 67: obj leaves s0), an if/else chain (41),
 * (RoadTestW *)(obj + 1) for FX (no change).
 * 2026-10-12: case 0's w set in a do-while(0): 65 (length +3); in the switch's expression: 5,
 * no change.
 * 2026-10-12: case 0's "if (w != NULL) {}" after its inner switch: 7; case 0's w as
 * "(u32) ROADTEST(obj) ^ 0" (IDO_NOTES 353): 66.
 * 2026-10-12 (second pass): case 0's work as its own block local (RoadTestW *w0): 18; "reloads" and
 * "empties" sweeps: nothing below 5. This C already builds w in v1 in case 0; only the switch's
 * load (0x3C(s0)) and demoAnimeStep's w->frame (0x20(s0)) read through obj before it.
 * 2026-10-12 (third try): the switch as "(*w).state" (IDO_NOTES 353's second spelling): 5, no
 * change. Both pos/rot clears as 4-chains, pad dropped: 43 (the chain changes the 0.0f code).
 * Cases 1 and 0 with block-local w (each block local reserves a word: they are the game's two
 * frame words) and pad[2] dropped: 5, the same frame. Then case 1's FAKE dropped: 15. Then
 * case 0's FAKE empty if added in its block: MATCH. */
s32 st2clearRoadTestProc(Obj *obj, s16 msg, void *arg) {
    RoadTestW *w;

    switch (msg) {
        case 2:
            obj->name = "test";
            w = ROADTEST(obj);
            PRINTF("道テスト作ります\n");
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            demoAnimeSet(w->frame, (u32) st2clearRoadAnim, 0xCE, 0.0f);
            w->state = 0;
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 0.0f;
            w->frame->rot[1] = 0.0f;
            PRINTF("道テスト作りました\n");
            break;
        case 1: {
            RoadTestW *w = ROADTEST(obj);

            if (w != NULL) { /* FAKE: IDO_NOTES 311, keeps w in its own register (unfolded) */
            }
            frmDestroy((Frame *) w->frame);
            frmDestroy((Frame *) w->pol);
            break;
        }
        case 3:
            ROADTEST(obj)->pol->dl = demoColorDL(1, 1, 1, 1, 1, 1, st2clearRoadAlpha);
            break;
        case 0: {
            RoadTestW *w = ROADTEST(obj);

            if (w != NULL) { /* FAKE: as case 1's; with w a block local it keeps the switch's
                              * load and demoAnimeStep's off obj's offsets */
            }
            switch (w->state) {
                case 0:
                    w->frame->pos[0] = 0.0f;
                    w->frame->pos[1] = 0.0f;
                    w->frame->pos[2] = 0.0f;
                    w->frame->rot[1] = 0.0f;
                    w->state++;
                    break;
                case 1:
                    demoAnimeStep(w->frame, 0.5f, 0);
                    break;
            }
            break;
        }
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *st2clearRoadTestCreate(Obj *obj) {
    return tskCreate(obj, st2clearRoadTestProc, obj->file, NULL);
}
