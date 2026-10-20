#include "tlb.h"
#include "tlb/tlb_0020.h"

/* A demo character's work (tlb_0022_1's DemoPlW). */
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

/* The demo characters' x and z, each frame, which the others turn toward (this file's .data,
 * all 5.0f at boot; still in the data .s, so extern here). */
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
extern u8 chademoSmokeTex0[];
extern u8 chademoSmokeTex1[];
extern u8 chademoSmokeTex2[];
extern u8 chademoSmokeTex3[];
extern u8 chademoSmokeTex4[];
extern u8 chademoSmokeTex6[];
AnimFrame chademoCharaAnims[] = {
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
    { (u32) chademoSmokeTex1, 8 },
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
    { (u32) chademoSmokeTex1, 7 },
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
    { (u32) chademoSmokeTex1, 200 },
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
f32 chademoSasX = 5.0f; /* .data: Sasuke's x and z (chademoSasProc writes them) */
f32 chademoSasZ = 5.0f;
f32 chademoGoeX = 5.0f; /* .data: Goemon's x and z (chademoGoeProc) */
f32 chademoGoeZ = 5.0f;
f32 chademoEbiX = 5.0f; /* .data: Ebisumaru's x and z (chademoEbiProc) */
f32 chademoEbiZ = 5.0f;
f32 chademoEbiOrSasX = 5.0f; /* .data: Ebisumaru's or Sasuke's x and z in some steps */
f32 chademoEbiOrSasZ = 5.0f;
extern s32 chademoTick;
extern u8 gDemoCamHold;
extern Gfx chademoShadowDl[];
extern s32 gObjMoveSel;
extern f32 gObjMovePosX;
extern f32 gObjMovePosY;
extern f32 gObjMovePosZ;
extern f32 gObjMoveRot;
extern f32 gObjMoveDx;
extern f32 gObjMoveDy;
extern f32 gObjMoveDz;
extern f32 gObjMoveDrot;

/* "genoGOE" (Goemon, character 0) in the tea house demo: OBJMOVE's model 0 moves him, his shadow
 * follows him. His "goecreate" print, before the switch, is the file's first string. */
s32 chademoGoeProc(Obj *obj, s16 msg, void *arg) {
    static s32 step = 1; /* .data 0x08005D78 */
    s32 pad[2];
    DemoPlW *w = (DemoPlW *) (obj + 1);
    Player *p;
    PlInit init;

    DPRINTF("goecreate\n");
    switch (msg) {
        case 2:
            obj->name = "genoGOE";
            plMakeInit(&init, 0, 0);
            init.flags |= 0x90000040;
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
            DPRINTF("*** p->model_buff=%x\n");
            DPRINTF("立ち止まります\n");
            w->model = p->model;
            DPRINTF("ゴエモン作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, chademoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.08f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->chara = 0;
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
            tskDestroy(w->pl);
            frmDestroy(&w->model->f);
            frmDestroy(&w->shadow->f);
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
            chademoGoeX = w->model->pos[0];
            chademoGoeZ = w->model->pos[2];
            switch (w->state) {
                case 0:
                    if (demoWalkTo(w->model, 0.2f, 0.4f, 21.0f, 60.0f)) {
                        w->state = 1;
                        plAnimeSet(PL(w->pl), 0xD7);
                    }
                    break;
                case 1:
                    if (chademoTick > 220) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], chademoSasX,
                                                          chademoSasZ, w->model->rot[1], 1.0f);
                    } else if (w->model->rot[1] < 3.82227116784973697) {
                        w->model->rot[1] += 0.00698131720155203191;
                    }
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (chademoTick == 535) {
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK0(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->model->scale[2] = 0.097f;
                        w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                        w->model->pos[0] = 15.0f;
                        w->model->pos[1] = 4.0f;
                        w->model->pos[2] = -3.0f;
                        w->model->rot[1] = 10.594149f;
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (chademoTick < 858) {
                        demoAnimeStep(w->model, 0.2f, 0);
                        fadeAnimStep(&w->eyes);
                        fadeAnimStep(&w->mouth);
                    } else if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.2f, 0xEB, 0xEC)) == -1) {
                        w->state = 3;
                        step = 0;
                        scRunStart(0x54, 0, 0);
                    }
                    break;
                case 3:
                    if (chademoTick == 1058) {
                        scRunEnd();
                        w->state = 4;
                    }
                    demoAnimeStep(w->model, 0.5f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
                case 4:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.2f, 0.2f, 0xEB, 0xE9)) == -1) {
                        w->state = 5;
                        step = 0;
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (chademoTick == 1798) {
                        w->model->pos[0] = 24.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 24.0f;
                        w->model->rot[1] = 9.96583f;
                        w->state = 6;
                    }
                    break;
                case 6:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], chademoEbiOrSasX,
                                                      chademoEbiOrSasZ, w->model->rot[1], 0.5f);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (chademoTick == 3248) {
                        w->model->pos[0] = 15.0f;
                        w->model->pos[1] = 4.0f;
                        w->model->pos[2] = -3.0f;
                        w->model->rot[1] = 9.721484f;
                        w->state = 7;
                    }
                    break;
                case 7:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (chademoTick < 3488) {
                        demoAnimeStep(w->model, 0.2f, 0);
                    } else {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], chademoEbiX,
                                                          chademoEbiZ, w->model->rot[1], 1.0f);
                        if ((step = demoPlTunagiAnime(obj, 1, step, 2, 2, 0.4f, 0.2f, 0xF2, 0xF3)) == -1) {
                            w->state = 8;
                            step = 0;
                            scRunEnd();
                            scRunStart(0x5D, 0, 0);
                        }
                    }
                    break;
                case 8:
                    if (chademoTick == 3808) {
                        scRunEnd();
                        w->state = 9;
                    }
                    demoAnimeStep(w->model, 0.5f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
                case 9:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], chademoSasX,
                                                      chademoSasZ, w->model->rot[1], 1.0f);
                    if ((step = demoPlTunagiAnime(obj, 1, step, 2, 3, 0.4f, 0.2f, 0xF2, 0xE9)) == -1) {
                        w->state = 10;
                        step = 0;
                        scRunEnd();
                    }
                    break;
                case 10:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (chademoTick == 4548) {
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                        w->model->pos[0] = 24.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 24.0f;
                        w->model->rot[1] = 9.96583f;
                        w->state++;
                    }
                    break;
                case 11:
                    if (chademoTick < 4568) {
                        demoAnimeStep(w->model, 0.2f, 0);
                    } else if ((step = demoPlTunagiAnime(obj, 3, step, 2, 2, 0.2f, 0.2f, 0xEB, 0xEC)) == -1) {
                        w->state++;
                        step = 0;
                        scRunStart(0x60, 0, 0);
                    }
                    break;
                case 12:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (chademoTick == 5098) {
                        scRunEnd();
                        w->state++;
                    }
                    if (chademoTick == 4768) {
                        scRunEnd();
                        scRunStart(0x61, 0, 0);
                    }
                    if (chademoTick > 4768) {
                        demoAnimeStep(w->model, 0.3f, 0);
                    }
                    demoAnimeStep(w->model, 0.3f, 0);
                    break;
                case 13:
                    if ((step = demoPlTunagiAnime(obj, 1, step, 2, 3, 0.2f, 0.2f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 14:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *chademoGoeCreate(Obj *obj) {
    return tskCreate(obj, chademoGoeProc, obj->file, NULL);
}

/* "genoGOE" (the name is Goemon's; this is Ebisumaru, character 1) in the tea house demo:
 * OBJMOVE's model 1 moves him, his shadow follows him. */
s32 chademoEbiProc(Obj *obj, s16 msg, void *arg) {
    static s32 step; /* .bss 0x08006260 */
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
            init.place = 21.0f;
            init.height = 0.0f;
            init.side = 99.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            DPRINTF("歩きモーション\n");
            plAnimeSet(PL(w->pl), 0xDA);
            DPRINTF("立ち止まります\n");
            w->model = p->model;
            DPRINTF("エビス丸作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, chademoShadowDl, 0xBC);
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
            tskDestroy(w->pl);
            frmDestroy(&w->model->f);
            frmDestroy(&w->shadow->f);
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
            chademoEbiX = w->model->pos[0];
            chademoEbiZ = w->model->pos[2];
            switch (w->state) {
                case 0:
                    if (chademoTick > 70 && demoWalkTo(w->model, 0.3f, 0.4f, 42.0f, 57.0f)) {
                        w->state = 1;
                        plAnimeSet(PL(w->pl), 0xD7);
                    }
                    break;
                case 1:
                    if (chademoTick > 220) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], chademoSasX,
                                                          chademoSasZ, w->model->rot[1], 1.0f);
                    } else if (w->model->rot[1] < 3.66519153081481663) {
                        w->model->rot[1] += 0.0174532930038800785;
                    }
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (chademoTick == 535) {
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->model->scale[2] = 0.097f;
                        w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                        w->model->pos[0] = 25.0f;
                        w->model->pos[1] = 2.0f;
                        w->model->pos[2] = -13.0f;
                        w->model->rot[1] = 4.8345623f;
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (chademoTick < 1048) {
                        demoAnimeStep(w->model, 0.2f, 0);
                    } else if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.2f, 0xEB, 0xEC)) == -1) {
                        w->state = 3;
                        step = 0;
                        scRunStart(0x55, 0, 0);
                    }
                    break;
                case 3:
                    if (chademoTick == 1268) {
                        scRunEnd();
                        w->state = 4;
                    }
                    demoAnimeStep(w->model, 0.5f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
                case 4:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.2f, 0.2f, 0xEB, 0xE9)) == -1) {
                        w->state = 5;
                        step = 0;
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (chademoTick == 1798) {
                        w->model->pos[0] = 42.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 14.0f;
                        w->model->rot[1] = 4.2586036f;
                        w->state = 6;
                    }
                    break;
                case 6:
                    chademoEbiOrSasX = w->model->pos[0];
                    chademoEbiOrSasZ = w->model->pos[2];
                    if (chademoTick < 1808) {
                        demoAnimeStep(w->model, 0.2f, 0);
                    } else if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.2f, 0xEB, 0xEC)) == -1) {
                        w->state = 7;
                        step = 0;
                        scRunStart(0x57, 0, 0);
                    }
                    break;
                case 7:
                    if (chademoTick == 2028) {
                        scRunEnd();
                        scRunStart(0x58, 0, 0);
                    }
                    if (chademoTick == 2338) {
                        scRunEnd();
                        w->state = 8;
                    }
                    demoAnimeStep(w->model, 0.3f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (chademoTick > 2028) {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
                case 8:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.2f, 0.2f, 0xEB, 0xE9)) == -1) {
                        w->state = 9;
                        step = 0;
                    }
                    break;
                case 9:
                    if (chademoTick == 3008) {
                        scRunStart(0x5B, 0, 0);
                    }
                    if (chademoTick < 3008) {
                        demoAnimeStep(w->model, 0.2f, 0);
                    } else if ((step = demoPlTunagiAnime(obj, 3, step, 2, 3, 0.2f, 0.1f, 0xED, 0xE8)) == -1) {
                        w->state = 10;
                        step = 0;
                    }
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
                case 10:
                    if (chademoTick == 3248) {
                        scRunEnd();
                        plAnimeSet(PL(w->pl), 0xF4);
                        demoTexAnimK1(&w->eyes, 3, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 3, &PL(w->pl)->model->segs[4]);
                        scRunStart(0x5C, 0, 0);
                        w->state = 11;
                        w->model->pos[0] = 25.0f;
                        w->model->pos[1] = 2.0f;
                        w->model->pos[2] = -13.0f;
                        w->model->rot[1] = 6.178466f;
                    }
                    demoAnimeStep(w->model, 0.1f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
                case 11:
                    if (step == 0) {
                        if (demoAnimeStep(w->model, 0.2f, 2)) {
                            step = 1;
                            plAnimeSet(PL(w->pl), 0xF5);
                            demoTexAnimK1(&w->eyes, 3, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK1(&w->mouth, 3, &PL(w->pl)->model->segs[4]);
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    if (chademoTick == 3808) {
                        scRunEnd();
                        w->state = 12;
                        step = 0;
                    }
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
                case 12:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], chademoSasX,
                                                      chademoSasZ, w->model->rot[1], 0.5f);
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.2f, 0.1f, 0xF4, 0xE9)) == -1) {
                        w->state = 13;
                        step = 0;
                    }
                    break;
                case 13:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (chademoTick == 4548) {
                        scRunEnd();
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->model->pos[0] = 42.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 14.0f;
                        w->model->rot[1] = 4.2586036f;
                        w->state++;
                    }
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
                case 14:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *chademoEbiCreate(Obj *obj) {
    return tskCreate(obj, chademoEbiProc, obj->file, NULL);
}

/* "chaSAS" (Sasuke, character 2) in the tea house demo: OBJMOVE's model 2 moves him, his shadow
 * follows him. In step 13 he sways (rot[0]) by a swing that shrinks by 0.98 every other frame.
 * His swing is the double 0.3 ("0.3f" lets IDO share step 12's 0.3 with the shadow's).
 * FAKE: demoTexAnimK2 is called as returning an int (as an undeclared function is, in the
 * game's C; IDO_NOTES 369): each pair's first call then leaves a result in v0, so the address
 * kept across it takes v1, as the game has. Called as void, the address took v0. */
s32 chademoSasProc(Obj *obj, s16 msg, void *arg);
#undef demoTexAnimK2
#define demoTexAnimK2 (*(s32 (*)(Anim *, s32, u32 *)) & demoTexAnimK2)
s32 chademoSasProc(Obj *obj, s16 msg, void *arg) {
    static s32 step;  /* .bss 0x08006264 */
    static f32 swing; /* .bss 0x08006268, in degrees */
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
            init.place = 28.0f;
            init.height = 0.0f;
            init.side = -12.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            DPRINTF("走りモーション\n");
            plAnimeSet(PL(w->pl), 0xDB);
            DPRINTF("立ち止まります\n");
            w->model = p->model;
            DPRINTF("サスケ作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, chademoShadowDl, 0xBC);
            w->shadow->scale[2] = 0.065f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 2;
            w->state = 0;
            w->model->pos[0] = 28.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = -12.0f;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            DPRINTF("サスケ作りました\n");
            break;
        case 1:
            tskDestroy(w->pl);
            frmDestroy(&w->model->f);
            frmDestroy(&w->shadow->f);
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
            chademoSasX = w->model->pos[0];
            chademoSasZ = w->model->pos[2];
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = 0.3f;
            switch (w->state) {
                case 0:
                    if (chademoTick > 220 && demoWalkTo(w->model, 0.4f, 0.5f, 29.0f, 37.0f)) {
                        w->state = 1;
                        plAnimeSet(PL(w->pl), 0xD7);
                    }
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (w->model->rot[1] < 3.68264482381869662) {
                        w->model->rot[1] += 0.0174532930038800785;
                    }
                    if (chademoTick == 535) {
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK2(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->model->scale[2] = 0.097f;
                        w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                        w->model->pos[0] = -5.0f;
                        w->model->pos[1] = 4.0f;
                        w->model->pos[2] = -14.0f;
                        w->model->rot[1] = 7.5921826f;
                        w->state = 2;
                        step = 0;
                    }
                    break;
                case 2:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.2f, 0xEB, 0xEC)) == -1) {
                        w->state = 3;
                        step = 0;
                        scRunStart(0x53, 0, 0);
                    }
                    break;
                case 3:
                    if (chademoTick == 868) {
                        scRunEnd();
                        w->state = 4;
                    }
                    demoAnimeStep(w->model, 0.7f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
                case 4:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.2f, 0.2f, 0xEB, 0xE9)) == -1) {
                        w->state = 5;
                        step = 0;
                    }
                    break;
                case 5:
                    if (chademoTick < 1258) {
                        demoAnimeStep(w->model, 0.2f, 0);
                    } else if ((step = demoPlTunagiAnime(obj, 3, step, 2, 2, 0.2f, 0.2f, 0xEB, 0xEC)) == -1) {
                        w->state = 6;
                        step = 0;
                        scRunStart(0x56, 0, 0);
                    }
                    break;
                case 6:
                    if (chademoTick == 1738) {
                        scRunEnd();
                        w->state = 7;
                    }
                    demoAnimeStep(w->model, 0.5f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
                case 7:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.5f, 0.4f, 0xEB, 0xE9)) == -1) {
                        w->state = 8;
                        step = 0;
                    }
                    break;
                case 8:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (chademoTick == 1798) {
                        w->model->pos[0] = 20.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 3.0f;
                        w->model->rot[1] = 6.9987707f;
                        w->state = 9;
                    }
                    break;
                case 9:
                    chademoEbiOrSasX = w->model->pos[0];
                    chademoEbiOrSasZ = w->model->pos[2];
                    if (chademoTick < 2358) {
                        demoAnimeStep(w->model, 0.2f, 0);
                    } else if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.2f, 0xEB, 0xEC)) == -1) {
                        w->state = 10;
                        step = 0;
                        scRunStart(0x59, 0, 0);
                    }
                    break;
                case 10:
                    if (chademoTick < 2708) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], chademoEbiX,
                                                          chademoEbiZ, w->model->rot[1], 0.5f);
                    } else {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], chademoGoeX,
                                                          chademoGoeZ, w->model->rot[1], 0.5f);
                        demoAnimeStep(w->model, 0.3f, 0);
                    }
                    if (chademoTick == 2708) {
                        scRunEnd();
                        scRunStart(0x5A, 0, 0);
                    }
                    if (chademoTick == 2998) {
                        scRunEnd();
                        w->state = 11;
                    }
                    demoAnimeStep(w->model, 0.34f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
                case 11:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.2f, 0.2f, 0xEB, 0xE9)) == -1) {
                        w->state = 12;
                        step = 0;
                    }
                    break;
                case 12:
                    if (chademoTick == 3248) {
                        w->model->pos[0] = -5.0f;
                        w->model->pos[1] = 4.0f;
                        w->model->pos[2] = -14.0f;
                        w->model->rot[1] = 7.6619954f;
                        w->state = 13;
                        swing = 0.3;
                    }
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
                case 13:
                    if (chademoTick < 3408) {
                        demoAnimeStep(w->model, 0.2f, 0);
                    } else {
                        w->model->rot[0] = swing * 0.0174532930038800785;
                        if (!(chademoTick & 1)) {
                            swing = swing * -0.98;
                        }
                    }
                    if (chademoTick == 3408) {
                        demoTexAnimK2(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                    }
                    if (chademoTick == 3808) {
                        scRunEnd();
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK2(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->model->pos[0] = -5.0f;
                        w->model->pos[1] = 4.0f;
                        w->model->pos[2] = -14.0f;
                        w->model->rot[1] = 7.5921826f;
                        w->state = 14;
                        w->model->rot[0] = 0.0f;
                        w->model->rot[2] = w->model->rot[0];
                    }
                    break;
                case 14:
                    chademoEbiOrSasX = w->model->pos[0];
                    chademoEbiOrSasZ = w->model->pos[2];
                    if (chademoTick < 3828) {
                        demoAnimeStep(w->model, 0.2f, 0);
                        fadeAnimStep(&w->eyes);
                        fadeAnimStep(&w->mouth);
                    } else if ((step = demoPlTunagiAnime(obj, 3, step, 2, 2, 0.2f, 0.2f, 0xEB, 0xEC)) == -1) {
                        w->state = 15;
                        step = 0;
                        scRunStart(0x5E, 0, 0);
                    }
                    break;
                case 15:
                    if (chademoTick < 4228) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], chademoEbiX,
                                                          chademoEbiZ, w->model->rot[1], 0.5f);
                    } else {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], chademoGoeX,
                                                          chademoGoeZ, w->model->rot[1], 0.5f);
                        demoAnimeStep(w->model, 0.3f, 0);
                    }
                    if (chademoTick == 4228) {
                        scRunEnd();
                        scRunStart(0x5F, 0, 0);
                    }
                    if (chademoTick == 4528) {
                        scRunEnd();
                        w->state = 16;
                    }
                    demoAnimeStep(w->model, 0.34f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
                case 16:
                    demoAnimeStep(w->model, 0.2f, 2);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (chademoTick == 4548) {
                        scRunEnd();
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK2(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                        w->model->pos[0] = 20.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 3.0f;
                        w->model->rot[1] = 6.9987707f;
                        w->state = 17;
                    }
                    break;
                case 17:
                    chademoEbiOrSasX = w->model->pos[0];
                    chademoEbiOrSasZ = w->model->pos[2];
                    if (chademoTick < 5088) {
                        demoAnimeStep(w->model, 0.2f, 0);
                        fadeAnimStep(&w->eyes);
                        fadeAnimStep(&w->mouth);
                    } else if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.2f, 0xEB, 0xEC)) == -1) {
                        w->state = 18;
                        step = 0;
                        scRunStart(0x62, 0, 0);
                    }
                    break;
                case 18:
                    if (chademoTick < 5228) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], chademoEbiX,
                                                          chademoEbiZ, w->model->rot[1], 0.5f);
                    } else {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], chademoGoeX,
                                                          chademoGoeZ, w->model->rot[1], 0.5f);
                        demoAnimeStep(w->model, 0.3f, 0);
                    }
                    if (chademoTick == 5328) {
                        scRunEnd();
                        w->state = 19;
                    }
                    if (chademoTick == 5228) {
                        demoTexAnimK2(&w->eyes, 3, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 3, &PL(w->pl)->model->segs[4]);
                    }
                    demoAnimeStep(w->model, 0.34f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
                case 19:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.2f, 0.2f, 0xEB, 0xE9)) == -1) {
                        w->state = 20;
                        step = 0;
                    }
                    break;
                case 20:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
#undef demoTexAnimK2
#define demoTexAnimK2 (*demoTexAnimK2)

Obj *chademoSasCreate(Obj *obj) {
    return tskCreate(obj, chademoSasProc, obj->file, NULL);
}

/* "KEMURI" (smoke): a pol frame of file 0x123 with an animated texture, rising and growing
 * while its alpha fades by `fade` every `period` frames of the file's counter; it deletes
 * itself at alpha 0 or when gDemoModelsDelete is 1, and stands still while gDemoCamHold. */
typedef struct KemuriWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ Anim anim;
    /* 0x14 */ f32 pos[3];
    /* 0x20 */ f32 scale;
    /* 0x24 */ f32 rise;  /* added to the frame's height each frame */
    /* 0x28 */ f32 grow;  /* the frame's scale is multiplied by it each frame */
    /* 0x2C */ s32 alpha; /* the env colour's alpha */
    /* 0x30 */ s32 fade;  /* taken off alpha every period frames */
    /* 0x34 */ s32 period;
} KemuriWork; /* also the shape of its create argument */

extern u8 gDemoCamHold;
extern s32 gDemoModelsDelete;
extern Gfx chademoSmokeDl[];                       /* the smoke's display list */
extern s32 chademoTick;               /* the file's frame counter */
AnimFrame chademoKemuriAnim[] = { /* the smoke's texture animation */
    { (u32) chademoSmokeTex0, 6 },
    { (u32) chademoSmokeTex1, 6 },
    { (u32) chademoSmokeTex2, 6 },
    { (u32) chademoSmokeTex3, 6 },
    { (u32) chademoSmokeTex4, 6 },
    { 0x03002D30, 8 },
    { (u32) chademoSmokeTex6, 10 },
    { 0, 0 },
};

s32 chademoKemuriProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    KemuriWork *w = (KemuriWork *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "KEMURI";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, chademoSmokeDl, 0x123);
            fadeAnimStart(&w->anim, chademoKemuriAnim, 0, &w->frame->segs[3], floadGetSetFile(0x123, 0));
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
            if (!(chademoTick & 1)) {
                fadeAnimStep(&w->anim);
            }
            if (chademoTick % w->period == 0) {
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

/* A puff of smoke at x, y, z under the menu's last task. */
void chademoKemuriCreate(f32 x, f32 y, f32 z, f32 scale, f32 grow, f32 rise, s32 alpha, s32 fade,
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
    tskCreate(gDmenuLastObj, chademoKemuriProc, gDmenuLastObj->file, &arg);
}

/* Every 40 frames a puff of smoke at 14, 12, 34 (rising 0.1, lasting 30) until frame 3808, then
 * smaller ones (rising 0.07, lasting 20). */
void chademoChimneySmoke(void) {
    if (chademoTick % 40 == 0) {
        if (chademoTick < 3808) {
            chademoKemuriCreate(14.0f, 12.0f, 34.0f, 0.07f, 1.0f, 0.1f, 30, 1, 4);
        } else {
            chademoKemuriCreate(14.0f, 12.0f, 34.0f, 0.07f, 1.0f, 0.07f, 20, 1, 4);
        }
    }
}

/* At frame 30 it starts scenario 0x52; at frame 445 it ends it. */
void chademoScenarioStep(void) {
    switch (chademoTick) {
        case 30:
            scRunStart(0x52, 0, 0);
            break;
        case 445:
            scRunEnd();
            break;
    }
}

#define DPRINTF

/* The tea house's people ("cha_baba", "musume"): a model and its shadow, a frame count and a
 * state (tlb_0031_1's GateW, the model and shadow 4 further on). */
typedef struct ChaPersonW {
    /* 0x00 */ u8 pad00[0x4];
    /* 0x04 */ AnimeFrame *model;
    /* 0x08 */ PolFrame *shadow;
    /* 0x0C */ u8 pad0C[0x24];
    /* 0x30 */ s32 timer;
    /* 0x34 */ s32 state;
} ChaPersonW;

extern s32 gObjMoveSel; /* OBJMOVE: the model chosen, -1 for none */
extern f32 gObjMovePosX; /* the chosen model's position and angle, as shown */
extern f32 gObjMovePosY;
extern f32 gObjMovePosZ;
extern f32 gObjMoveRot;
extern f32 gObjMoveDx; /* the moves this frame: x, y, z, angle */
extern f32 gObjMoveDy;
extern f32 gObjMoveDz;
extern f32 gObjMoveDrot;
extern Gfx chademoShadowDl[]; /* a shadow's display list */
extern u8 chademoBabaAnim[];  /* the old woman's animation */

/* "cha_baba": the tea house's old woman (anime of file 0x98) at (-4, 0, 60), turned 252
 * degrees; OBJMOVE's model 3 moves it. She walks to (-36, 64) and goes after 300 frames.
 * tlb_0031_1's "gate" (demo22GateProc) is its twin. */
s32 chademoBabaProc(Obj *obj, s16 msg, void *arg) {
    ChaPersonW *w = (ChaPersonW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "cha_baba";
            w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->model == NULL) {
                return -1;
            }
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, chademoShadowDl, 0xBC);
            demoAnimeSet(w->model, (u32) chademoBabaAnim, 0x98, 0.0f);
            w->model->flags |= 0x20;
            w->shadow->flags |= 0x20;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->shadow->scale[2] = 0.08f;
            w->shadow->scale[0] = w->shadow->scale[1] = w->shadow->scale[2];
            w->model->pos[0] = -4.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = 60.0f;
            w->model->rot[1] = 4.3982296f;
            w->timer = 0;
            w->state = 0;
            break;
        case 1:
            frmDestroy((Frame *) w->model);
            frmDestroy((Frame *) w->shadow);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, 0x80);
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
            w->timer++;
            demoWalkTo(w->model, 0.2f, 0.1f, -36.0f, 64.0f);
            if (w->timer > 300) {
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

extern u8 chademoMusumeAnim[]; /* the girl's animation */

/* "musume": the tea house's girl (anime of file 0x7D) at (6, 0, 29), turned 246 degrees;
 * OBJMOVE's model 4 moves her. She walks to (-3, -8) with her shadow under her, and goes at the
 * demo's frame 0x218. cha_baba's twin. */
s32 chademoMusumeProc(Obj *obj, s16 msg, void *arg) {
    ChaPersonW *w = (ChaPersonW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "musume";
            w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->model == NULL) {
                return -1;
            }
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, chademoShadowDl, 0xBC);
            demoAnimeSet(w->model, (u32) chademoMusumeAnim, 0x7D, 0.0f);
            w->model->flags |= 0x20;
            w->shadow->flags |= 0x20;
            w->model->scale[2] = 0.097f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->shadow->scale[2] = 0.065f;
            w->shadow->scale[0] = w->shadow->scale[1] = w->shadow->scale[2];
            w->model->pos[0] = 6.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = 29.0f;
            w->model->rot[1] = 4.29351f;
            w->timer = 0;
            w->state = 0;
            break;
        case 1:
            frmDestroy((Frame *) w->model);
            frmDestroy((Frame *) w->shadow);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, 0x80);
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
            demoWalkTo(w->model, 0.45f, 0.17f, -3.0f, -8.0f);
            if (chademoTick >= 0x218) {
                return -1;
            }
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[1] = 0.5f;
            w->shadow->pos[2] = w->model->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* An empty function nothing calls (splat folds it into the one above); the dead string of a
 * "shounin" (merchant) task that never was. */
void chademoShouninStub(void) {
    DPRINTF("shounin");
}

s32 chademoBabaProc(Obj *obj, s16 msg, void *arg);
s32 chademoMusumeProc(Obj *obj, s16 msg, void *arg);

/* Starts "cha_baba" and "musume" under obj. */
void chademoBabaMusumeCreate(Obj *obj) {
    tskCreate(obj, chademoBabaProc, obj->file, NULL);
    tskCreate(obj, chademoMusumeProc, obj->file, NULL);
}
