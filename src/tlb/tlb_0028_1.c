#include "tlb.h"
#include "tlb/tlb_0028.h"

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
/* The file's .data (0x08004C00 on), laid out by first declaration: the characters' texture
 * animations (demotool.c's table, as tlb_0020_1's; nothing reads it), these globals in address
 * order, then the demo characters' statics in function order. */
AnimFrame st3clearCharaAnims[] = {
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
f32 st3clearSasX = 1.0f; /* Sasuke's x and z, each frame */
f32 st3clearSasZ = 1.0f;
f32 st3clearGoeX = 1.0f; /* Goemon's x and z, each frame */
f32 st3clearGoeZ = 1.0f;
f32 st3clearEbiX = 1.0f; /* Ebisumaru's x and z, each frame */
f32 st3clearEbiZ = 1.0f;
f32 st3clearYaeX = 1.0f; /* Yae's x and z, each frame */
f32 st3clearYaeZ = 1.0f;
f32 st3clearTurnX = 1.0f; /* the x and z she turns toward */
f32 st3clearTurnZ = 1.0f;
f32 st3clearUnused2 = 1.0f; /* unread */
f32 st3clearUnused3 = 1.0f; /* unread */

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

extern s32 st3clearFrame; /* the demo's frame count */
extern Gfx st3clearShadowDl[]; /* the shadow */
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

/* "genoGOE" (Goemon, character 0, in the demo) at (-150, 2, 205) under the menu's last task, the
 * twin of tlb_0030_1's (its three statics, here in .data); OBJMOVE's model 0, his shadow 0.3
 * above his feet. At frame 0x593 he is put at (-166, 2, 194); between 0x3C1 and 0x43D, and from
 * 0x1F29, he turns toward a point. His steps (0, then 10-13, 20-23, 30-33, 40-43, 50-53, 100)
 * link his actions with demoPlTunagiAnime on the demo's frame count (scenes 0xFD, 0x100, 0x10C,
 * 0x114 and 0x116 through st3clearScenarioStart).
 */
s32 st3clearGoeProc(Obj *obj, s16 msg, void *arg) {
    static s32 step = 1;            /* .data 0x08005648 (1): his demoPlTunagiAnime step */
    static f32 speed = 1.0f;        /* .data 0x0800564C (1.0f): genoGOE's fall speed, only stored */
    static Player *p = (Player *) 1; /* .data 0x08005650 (1): his player */
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
            init.place = -150.0f;
            init.height = 2.0f;
            init.side = 205.0f;
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
            demoPolSetDL(w->shadow, st3clearShadowDl, 0xBC);
            w->shadow->scale[2] = 0.07999999821f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->chara = 0;
            w->model->rot[1] = 3.1415927f;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -150.0f;
            w->model->pos[1] = 2.0f;
            w->model->pos[2] = 205.0f;
            w->model->rot[1] = 0.8028515f;
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
            w->shadow->pos[1] = w->model->pos[1] + 0.3;
            st3clearGoeX = w->model->pos[0];
            st3clearGoeZ = w->model->pos[2];
            if (st3clearFrame == 0x593) {
                w->model->pos[0] = -166.0f;
                w->model->pos[1] = 2.0f;
                w->model->pos[2] = 194.0f;
                w->model->rot[1] = -0.8028515f;
            }
            if (st3clearFrame >= 0x3C1 && st3clearFrame < 0x43E) {
                w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], st3clearTurnX,
                                                  st3clearTurnZ, w->model->rot[1], 2.0f);
            }
            if (st3clearFrame >= 0x1F29) {
                w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], -174.0f, 213.0f,
                                                  w->model->rot[1], 1.9f);
            }
            switch (w->state) {
                case 0:
                    w->model->scale[2] = 0.097f;
                    w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                    w->model->pos[0] = -150.0f;
                    w->model->pos[1] = 2.0f;
                    w->model->pos[2] = 205.0f;
                    w->model->rot[1] = 0.8028515f;
                    w->state = 10;
                    break;
                case 10:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (st3clearFrame == 0xF0) {
                        step = 0;
                        w->state++;
                    }
                    break;
                case 11:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.5f, 0xEB, 0xEC)) == -1) {
                        step = 0;
                        w->state++;
                        st3clearScenarioStart(0xFD);
                    }
                    break;
                case 12:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.5f, 0);
                    if (st3clearFrame == 0x1FE) {
                        scRunEnd();
                        step = 0;
                        w->state++;
                    }
                    break;
                case 13:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 3, 0.5f, 0.2f, 0xEB, 0xE9)) == -1) {
                        step = 0;
                        w->state = 20;
                    }
                    break;
                case 20:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (st3clearFrame == 0x3E4) {
                        step = 0;
                        w->state++;
                    }
                    break;
                case 21:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.5f, 0xEB, 0xEC)) == -1) {
                        step = 0;
                        w->state++;
                        st3clearScenarioStart(0x100);
                    }
                    break;
                case 22:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.6f, 0);
                    if (st3clearFrame == 0x470) {
                        step = 0;
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 23:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 3, 0.5f, 0.2f, 0xEB, 0xE9)) == -1) {
                        step = 0;
                        w->state = 30;
                        demoTexAnimK0(&w->eyes, 5, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 5, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 30:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st3clearFrame == 0x13F1) {
                        step = 0;
                        w->state++;
                    }
                    break;
                case 31:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.8f, 0xEB, 0xEC)) == -1) {
                        step = 0;
                        w->state++;
                        st3clearScenarioStart(0x10C);
                    }
                    break;
                case 32:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.8f, 0);
                    if (st3clearFrame == 0x14FF) {
                        scRunEnd();
                        step = 0;
                        w->state++;
                    }
                    break;
                case 33:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 3, 0.8f, 0.2f, 0xEB, 0xE9)) == -1) {
                        step = 0;
                        w->state = 40;
                        demoTexAnimK0(&w->eyes, 5, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 5, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 40:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (st3clearFrame == 0x1BAE) {
                        step = 0;
                        w->state++;
                    }
                    break;
                case 41:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.6f, 0xEB, 0xEC)) == -1) {
                        step = 0;
                        w->state++;
                        st3clearScenarioStart(0x114);
                    }
                    break;
                case 42:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.6f, 0);
                    if (st3clearFrame == 0x1D34) {
                        scRunEnd();
                        step = 0;
                        w->state++;
                    }
                    break;
                case 43:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 3, 0.6f, 0.2f, 0xEB, 0xE9)) == -1) {
                        step = 0;
                        w->state = 50;
                    }
                    break;
                case 50:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st3clearFrame == 0x1EEC) {
                        step = 0;
                        w->state++;
                    }
                    break;
                case 51:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.6f, 0xEB, 0xEC)) == -1) {
                        step = 0;
                        w->state++;
                        st3clearScenarioStart(0x116);
                    }
                    break;
                case 52:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.6f, 0);
                    if (st3clearFrame == 0x1FF0) {
                        scRunEnd();
                        step = 0;
                        w->state++;
                    }
                    break;
                case 53:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 3, 0.6f, 0.3f, 0xEB, 0xE9)) == -1) {
                        step = 0;
                        w->state = 100;
                    }
                    break;
                case 100:
                    demoAnimeStep(w->model, 0.3f, 0);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *st3clearGoeCreate(Obj *obj) {
    return tskCreate(obj, st3clearGoeProc, obj->file, NULL);
}


/* "genoGOE" (the game's name; it is Ebisumaru, character 1) at (-129, 2, 190) under the menu's
 * last task; OBJMOVE's model 1, his shadow 0.3 above his feet. At frame 0x593 he is put at
 * (-155, 2, 206); between 0x3DF and 0x43D, and from 0x1F51, he turns toward a point. His steps
 * (0, 10-13, 20-24, 100) link his actions on the demo's frame count (scenes 0x101, 0x108 and
 * 0x109 through st3clearScenarioStart).
 */
s32 st3clearEbiProc(Obj *obj, s16 msg, void *arg) {
    static s32 step = 1;         /* .data 0x08005654 (1): his demoPlTunagiAnime step */
    static f32 reserved1 = 1.0f; /* .data 0x08005658-0x08005660: three words nothing reads */
    static f32 reserved2 = 1.0f;
    static f32 reserved3 = 1.0f;
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
            init.place = -129.0f;
            init.height = 2.0f;
            init.side = 190.0f;
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
            demoPolSetDL(w->shadow, st3clearShadowDl, 0xBC);
            w->shadow->scale[2] = 0.09f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 1;
            w->state = 0;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -129.0f;
            w->model->pos[1] = 2.0f;
            w->model->pos[2] = 190.0f;
            w->model->rot[1] = -0.33161256f;
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
            w->shadow->pos[1] = w->model->pos[1] + 0.3;
            st3clearEbiX = w->model->pos[0];
            st3clearEbiZ = w->model->pos[2];
            if (st3clearFrame == 0x593) {
                w->model->pos[0] = -155.0f;
                w->model->pos[1] = 2.0f;
                w->model->pos[2] = 206.0f;
                w->model->rot[1] = -1.1170107f;
            }
            if (st3clearFrame >= 0x3DF && st3clearFrame < 0x43E) {
                w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], st3clearTurnX,
                                                  st3clearTurnZ, w->model->rot[1], 1.8f);
            }
            if (st3clearFrame >= 0x1F51) {
                w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], -174.0f, 213.0f,
                                                  w->model->rot[1], 1.4f);
            }
            switch (w->state) {
                case 0:
                    w->model->pos[0] = -129.0f;
                    w->model->pos[1] = 2.0f;
                    w->model->pos[2] = 190.0f;
                    w->model->rot[1] = -0.33161256f;
                    scRunEnd();
                    w->state = 10;
                    break;
                case 10:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (st3clearFrame == 0x484) {
                        step = 0;
                        w->state++;
                    }
                    break;
                case 11:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.5f, 0xEB, 0xEC)) == -1) {
                        step = 0;
                        w->state++;
                        st3clearScenarioStart(0x101);
                    }
                    break;
                case 12:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.5f, 0);
                    if (st3clearFrame == 0x574) {
                        step = 0;
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 13:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 3, 0.5f, 0.2f, 0xEB, 0xE9)) == -1) {
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        step = 0;
                        w->state = 20;
                    }
                    break;
                case 20:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st3clearFrame >= 0xE28) {
                        step = 0;
                        w->state++;
                    }
                    break;
                case 21:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.4f, 0xEB, 0xEC)) == -1) {
                        step = 0;
                        w->state++;
                        st3clearScenarioStart(0x108);
                    }
                    break;
                case 22:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (st3clearFrame == 0xEA0) {
                        st3clearScenarioStart(0x109);
                        step = 0;
                        w->state++;
                    }
                    break;
                case 23:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.7f, 0);
                    if (st3clearFrame == 0x1094) {
                        step = 0;
                        w->state++;
                    }
                    break;
                case 24:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 3, 0.7f, 0.2f, 0xEB, 0xE9)) == -1) {
                        step = 0;
                        w->state = 100;
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 100:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *st3clearEbiCreate(Obj *obj) {
    return tskCreate(obj, st3clearEbiProc, obj->file, NULL);
}


/* "chaSAS" (Sasuke, character 2, in the demo) at (-122, 2, 236) under the menu's last task;
 * OBJMOVE's model 2, his shadow 0.3 above his feet. At frame 0x593 he is put at (-158, 2, 222);
 * between 0x3B7 and 0x43D, and from 0x1F47, he turns toward a point. His steps (0, 10-13,
 * 20-23, 100) link his actions on the demo's frame count (scenes 0xFE and 0x107 through
 * st3clearScenarioStart).
 * A one-word pad: the game's frame has one spare word there. */
s32 st3clearSasProc(Obj *obj, s16 msg, void *arg) {
    static s32 step = 1;         /* .data 0x08005664 (1): his demoPlTunagiAnime step */
    static s32 reserved1 = 1;    /* .data 0x08005668-0x08005670: three words nothing reads */
    static f32 reserved2 = 1.0f;
    static s32 reserved3 = 1;
    s32 pad;
    DemoPlW *w = (DemoPlW *) (obj + 1);
    Player *p;
    PlInit init;

    switch (msg) {
        case 2:
            obj->name = "chaSAS";
            plMakeInit(&init, 0, 2);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = -122.0f;
            init.height = 2.0f;
            init.side = 236.0f;
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
            demoPolSetDL(w->shadow, st3clearShadowDl, 0xBC);
            w->shadow->scale[2] = 0.065f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 2;
            w->state = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -122.0f;
            w->model->pos[1] = 2.0f;
            w->model->pos[2] = 236.0f;
            w->model->rot[1] = -2.670354f;
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
            st3clearSasX = w->model->pos[0];
            st3clearSasZ = w->model->pos[2];
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = w->model->pos[1] + 0.3;
            if (st3clearFrame == 0x593) {
                w->model->pos[0] = -158.0f;
                w->model->pos[1] = 2.0f;
                w->model->pos[2] = 222.0f;
                w->model->rot[1] = -1.6406095f;
            }
            if (st3clearFrame >= 0x3B7 && st3clearFrame < 0x43E) {
                w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], st3clearTurnX,
                                                  st3clearTurnZ, w->model->rot[1], 1.8f);
            }
            if (st3clearFrame >= 0x1F47) {
                w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], -174.0f, 213.0f,
                                                  w->model->rot[1], 1.9f);
            }
            switch (w->state) {
                case 0:
                    w->model->pos[0] = -122.0f;
                    w->model->pos[1] = 2.0f;
                    w->model->pos[2] = 236.0f;
                    w->model->rot[1] = -2.670354f;
                    w->state = 10;
                    break;
                case 10:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (st3clearFrame == 0x208) {
                        step = 0;
                        w->state++;
                    }
                    break;
                case 11:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.3f, 0.7f, 0xEB, 0xEC)) == -1) {
                        step = 0;
                        w->state++;
                        st3clearScenarioStart(0xFE);
                    }
                    break;
                case 12:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.7f, 0);
                    if (st3clearFrame == 0x30C) {
                        scRunEnd();
                        step = 0;
                        w->state++;
                    }
                    break;
                case 13:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 3, 0.7f, 0.3f, 0xEB, 0xE9)) == -1) {
                        step = 0;
                        w->state = 20;
                        demoTexAnimK2(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 20:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (st3clearFrame == 0xD10) {
                        step = 0;
                        w->state++;
                    }
                    break;
                case 21:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.3f, 0.7f, 0xEB, 0xEC)) == -1) {
                        step = 0;
                        w->state++;
                        st3clearScenarioStart(0x107);
                    }
                    break;
                case 22:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.7f, 0);
                    if (st3clearFrame == 0xE14) {
                        scRunEnd();
                        step = 0;
                        w->state++;
                    }
                    break;
                case 23:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 3, 0.7f, 0.3f, 0xEB, 0xE9)) == -1) {
                        step = 0;
                        w->state = 100;
                        demoTexAnimK2(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 100:
                    demoAnimeStep(w->model, 0.3f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *st3clearSasCreate(Obj *obj) {
    return tskCreate(obj, st3clearSasProc, obj->file, NULL);
}

/* "chaYAE" (Yae, character 3, in the demo) at (-155, 2, 232) under the menu's last task, the
 * twin of tlb_0029_2's: OBJMOVE's model 3 moves her, and her shadow follows her, 0.3 above her
 * feet. At frame 0x593 she is put at (-161, 2, 232). Then she stands; at 0x8B8 she plays
 * action 0x101 with faces 2 and scene 0x104, at 0xA3E she goes back to 0xD7 and the scene ends;
 * between 0x3D5 and 0x43D, and from 0x1F5B, she turns toward a point. */
s32 st3clearYaeProc(Obj *obj, s16 msg, void *arg) {
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
            init.place = -155.0f;
            init.height = 2.0f;
            init.side = 232.0f;
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
            demoPolSetDL(w->shadow, st3clearShadowDl, 0xBC);
            w->shadow->scale[2] = 0.065f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 3;
            w->state = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -155.0f;
            w->model->pos[1] = 2.0f;
            w->model->pos[2] = 232.0f;
            w->model->rot[1] = 2.268928051f;
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
            st3clearYaeX = w->model->pos[0];
            st3clearYaeZ = w->model->pos[2];
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = w->model->pos[1] + 0.3;
            if (st3clearFrame == 0x593) {
                w->model->pos[0] = -161.0f;
                w->model->pos[1] = 2.0f;
                w->model->pos[2] = 232.0f;
                w->model->rot[1] = -1.91986227f;
            }
            switch (w->state) {
                case 0:
                    w->state++;
                    w->model->pos[0] = -155.0f;
                    w->model->pos[1] = 2.0f;
                    w->model->pos[2] = 232.0f;
                    w->model->rot[1] = 2.268928051f;
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (st3clearFrame == 0x8B8) {
                        plAnimeSet(PL(w->pl), 0x101);
                        demoTexAnimK3(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                        st3clearScenarioStart(0x104);
                    }
                    if (st3clearFrame == 0xA3E) {
                        plAnimeSet(PL(w->pl), 0xD7);
                        demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        scRunEnd();
                    }
                    if (st3clearFrame >= 0x3D5 && st3clearFrame < 0x43E) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], st3clearTurnX,
                                                          st3clearTurnZ, w->model->rot[1], 1.6f);
                    }
                    if (st3clearFrame >= 0x1F5B) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], -174.0f, 213.0f,
                                                          w->model->rot[1], 1.9f);
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *st3clearYaeCreate(Obj *obj) {
    return tskCreate(obj, st3clearYaeProc, obj->file, NULL);
}

/* "jiji"'s work (tlb_0024_1's TonoW). */
typedef struct JijiW {
    /* 0x00 */ AnimeFrame *model;
    /* 0x04 */ PolFrame *shadow;
    /* 0x08 */ s32 unused8;
    /* 0x0C */ s32 state;
} JijiW;

extern u8 st3clearJijiStartAnim[]; /* the old man's animations (set 0xC5) */
extern u8 st3clearJijiAnim[];
extern u8 st3clearJijiTunagiAnim[];
extern u8 st3clearJijiTunagiEndAnim[];

/* "jiji": the wise old man (set 0xC5) at (-260, 2, 199) under the menu's last task; OBJMOVE's
 * model 4. From frame 0x3EF to 0x591 he walks along x (0.1 a frame); at 0x593 he is put at
 * (-197, 2, 223) and plays st3clearJijiAnim. His steps (0, 10-11, 20-23, 30-33, 40-44, 50-54, 60-63,
 * 100) link idle and talk with tunagi_anime and start scenes 0xFF and 0x102-0x115 through
 * st3clearScenarioStart. Each frame his x and z are where the others turn.
 * The walk animation's address is read into anim before the frame-0x593 tests, which sets a1
 * early as the game does. His statics are the file's last .data. */
s32 st3clearJijiProc(Obj *obj, s16 msg, void *arg) {
    static s32 step = 1; /* his tunagi_anime step */
    static f32 unused = 1.0f; /* the word after it; nothing reads it */
    u8 *anim;
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
            demoAnimeSet(w->model, (u32) st3clearJijiStartAnim, 0xC5, 0.0f);
            demoPolSetDL(w->shadow, st3clearShadowDl, 0xBC);
            w->shadow->scale[2] = 0.07999999821f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->model->scale[2] = 0.09700000286f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -260.0f;
            w->model->pos[1] = 2.0f;
            w->model->pos[2] = 199.0f;
            w->model->rot[1] = 0.9948377f;
            DPRINTF("物知りじいさん作りました\n");
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
            if (st3clearFrame == 0x593) {
                w->model->pos[0] = -197.0f;
                w->model->pos[1] = 2.0f;
                w->model->pos[2] = 223.0f;
                w->model->rot[1] = 1.6406095f;
            }
            anim = st3clearJijiAnim;
            if (st3clearFrame >= 0x3EF && st3clearFrame < 0x592) {
                w->model->pos[0] += 0.1;
            }
            if (st3clearFrame == 0x593) {
                demoAnimeSet(w->model, (u32) anim, 0xC5, 0.0f);
            }
            switch (w->state) {
                case 0:
                    w->model->pos[0] = -260.0f;
                    w->model->pos[1] = 2.0f;
                    w->model->pos[2] = 199.0f;
                    w->model->rot[1] = 0.9948377f;
                    w->state = 10;
                    break;
                case 10:
                    if (st3clearFrame == 0x348) {
                        st3clearScenarioStart(0xFF);
                        w->state++;
                    }
                    break;
                case 11:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (st3clearFrame == 0x593) {
                        w->state = 20;
                        step = 0;
                    }
                    break;
                case 20:
                    if ((step = tunagi_anime(w->model, step, 2, 2, 0.3f, 0.3f, 0xC5, (u32) st3clearJijiTunagiAnim, 0xC5,
                                             (u32) st3clearJijiTunagiEndAnim)) == -1) {
                        w->state++;
                        step = 0;
                        st3clearScenarioStart(0x102);
                    }
                    break;
                case 21:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st3clearFrame == 0x714) {
                        st3clearScenarioStart(0x103);
                        step = 0;
                        w->state++;
                    }
                    break;
                case 22:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (st3clearFrame == 0x89A) {
                        scRunEnd();
                        step = 0;
                        w->state++;
                    }
                    break;
                case 23:
                    if ((step = tunagi_anime(w->model, step, 2, 3, 0.3f, 0.2f, 0xC5, (u32) st3clearJijiTunagiAnim, 0xC5,
                                             (u32) st3clearJijiAnim)) == -1) {
                        step = 0;
                        w->state = 30;
                    }
                    break;
                case 30:
                    if (st3clearFrame >= 0xA48) {
                        demoAnimeStep(w->model, 0.3f, 0);
                    } else {
                        demoAnimeStep(w->model, 0.1f, 0);
                    }
                    if (st3clearFrame == 0xA48) {
                        step = 0;
                        st3clearScenarioStart(0x105);
                    }
                    if (st3clearFrame == 0xB24) {
                        step = 0;
                        w->state++;
                    }
                    break;
                case 31:
                    if ((step = tunagi_anime(w->model, step, 2, 2, 0.4f, 0.3f, 0xC5, (u32) st3clearJijiTunagiAnim, 0xC5,
                                             (u32) st3clearJijiTunagiEndAnim)) == -1) {
                        step = 0;
                        w->state++;
                        st3clearScenarioStart(0x106);
                    }
                    break;
                case 32:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (st3clearFrame == 0xCAA) {
                        scRunEnd();
                        step = 0;
                        w->state++;
                    }
                    break;
                case 33:
                    if ((step = tunagi_anime(w->model, step, 2, 3, 0.4f, 0.1f, 0xC5, (u32) st3clearJijiTunagiAnim, 0xC5,
                                             (u32) st3clearJijiAnim)) == -1) {
                        step = 0;
                        w->state = 40;
                    }
                    break;
                case 40:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (st3clearFrame == 0x10A9) {
                        step = 0;
                        w->state++;
                    }
                    break;
                case 41:
                    if ((step = tunagi_anime(w->model, step, 2, 2, 0.4f, 0.3f, 0xC5, (u32) st3clearJijiTunagiAnim, 0xC5,
                                             (u32) st3clearJijiTunagiEndAnim)) == -1) {
                        step = 0;
                        w->state++;
                        st3clearScenarioStart(0x10A);
                    }
                    break;
                case 42:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (st3clearFrame == 0x124D) {
                        step = 0;
                        st3clearScenarioStart(0x10B);
                        w->state++;
                    }
                    break;
                case 43:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (st3clearFrame == 0x13E7) {
                        scRunEnd();
                        step = 0;
                        w->state++;
                    }
                    break;
                case 44:
                    if ((step = tunagi_anime(w->model, step, 2, 3, 0.3f, 0.2f, 0xC5, (u32) st3clearJijiTunagiAnim, 0xC5,
                                             (u32) st3clearJijiAnim)) == -1) {
                        step = 0;
                        w->state = 50;
                    }
                    break;
                case 50:
                    demoAnimeStep(w->model, 0.13f, 0);
                    if (st3clearFrame == 0x1509) {
                        step = 0;
                        w->state++;
                    }
                    break;
                case 51:
                    if ((step = tunagi_anime(w->model, step, 2, 2, 0.3f, 0.3f, 0xC5, (u32) st3clearJijiTunagiAnim, 0xC5,
                                             (u32) st3clearJijiTunagiEndAnim)) == -1) {
                        step = 0;
                        w->state++;
                        st3clearScenarioStart(0x10D);
                    }
                    break;
                case 52:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (st3clearFrame == 0x16AD) {
                        st3clearScenarioStart(0x10E);
                    }
                    if (st3clearFrame == 0x17ED) {
                        st3clearScenarioStart(0x10F);
                    }
                    if (st3clearFrame == 0x19F5) {
                        st3clearScenarioStart(0x110);
                        w->state++;
                    }
                    break;
                case 53:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (st3clearFrame == 0x1B49) {
                        st3clearScenarioStart(0x111);
                        w->state++;
                    }
                    if (st3clearFrame == 0x1C57) {
                        scRunEnd();
                        step = 0;
                        w->state++;
                    }
                    break;
                case 54:
                    if ((step = tunagi_anime(w->model, step, 2, 3, 0.3f, 0.2f, 0xC5, (u32) st3clearJijiTunagiAnim, 0xC5,
                                             (u32) st3clearJijiAnim)) == -1) {
                        step = 0;
                        w->state = 60;
                    }
                    break;
                case 60:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st3clearFrame == 0x1D48) {
                        step = 0;
                        w->state++;
                    }
                    break;
                case 61:
                    if ((step = tunagi_anime(w->model, step, 2, 2, 0.4f, 0.4f, 0xC5, (u32) st3clearJijiTunagiAnim, 0xC5,
                                             (u32) st3clearJijiTunagiEndAnim)) == -1) {
                        step = 0;
                        w->state++;
                        st3clearScenarioStart(0x115);
                    }
                    break;
                case 62:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (st3clearFrame == 0x1ED8) {
                        scRunEnd();
                        step = 0;
                        w->state++;
                    }
                    break;
                case 63:
                    if ((step = tunagi_anime(w->model, step, 2, 3, 0.3f, 0.2f, 0xC5, (u32) st3clearJijiTunagiAnim, 0xC5,
                                             (u32) st3clearJijiAnim)) == -1) {
                        step = 0;
                        w->state = 100;
                    }
                    break;
                case 100:
                    demoAnimeStep(w->model, 0.1f, 0);
                    break;
            }
            st3clearTurnX = w->model->pos[0];
            st3clearTurnZ = w->model->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *st3clearJijiCreate(Obj *obj) {
    return tskCreate(obj, st3clearJijiProc, obj->file, NULL);
}
