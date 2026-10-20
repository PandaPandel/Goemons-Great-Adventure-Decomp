#include "tlb.h"
#include "tlb/tlb_0021.h"


/* A demo character's work (tlb_0025_1's DemoPlW). */
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

extern Gfx op2ShadowDl[]; /* the shadow */
extern s32 opdemo2Frame; /* the demo's frame counter */
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
/* The file's .data (0x080113E0 on), laid out by first declaration: the characters' texture
 * animations (demotool.c's table, as op1CharaAnims and op3FaceAnimFrames; nothing reads it), these
 * globals in address order, then the characters' statics in function order (OP2GOE's step,
 * OP2EBI's, jiji's alpha and step). The characters' positions, each frame, are all 5.0 at first. */
AnimFrame op2CharaAnims[] = {
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
f32 op2JijiX = 5.0f; /* jiji's x and z */
f32 op2JijiZ = 5.0f;
f32 op2GoeX = 5.0f; /* Goemon's x and z */
f32 op2GoeZ = 5.0f;
f32 op2EbiX = 5.0f; /* Ebisumaru's x and z */
f32 op2EbiZ = 5.0f;
f32 op2Unused2 = 5.0f; /* unread */
f32 op2Unused3 = 5.0f; /* unread */

/* "OP2GOE": Goemon in the opening demo (OPDEMO2), at (4, 0, -37) under the menu's last task;
 * OBJMOVE's model 0, his shadow 0.3 above his feet. 15 steps on the demo's frame count (scene
 * runs 0x16C, 0x173, 0x17C); at 0x1392 he goes. */
s32 op2GoeProc(Obj *obj, s16 msg, void *arg);
s32 op2GoeProc(Obj *obj, s16 msg, void *arg) {
    static s32 step = 1; /* .data 0x08011E18: his demoPlTunagiAnime step */
    s32 pad[2];
    DemoPlW *w = (DemoPlW *) (obj + 1);
    Player *p;
    PlInit init;

    DPRINTF("goecreate\n");
    switch (msg) {
        case 2:
            obj->name = "OP2GOE";
            plMakeInit(&init, 0, 0);
            init.flags |= 0x90000040;
            step = 0;
            init.zone = 0;
            init.place = 4.0f;
            init.height = 0.0f;
            init.side = -37.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            DPRINTF("歩きモーション\n");
            plAnimeSet(PL(w->pl), 0xD7);
            w->model = p->model;
            demoTexAnimK0(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
            demoTexAnimK0(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
            DPRINTF("*** p->model_buff=%x\n", p->model);
            DPRINTF("ゴエモン作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, op2ShadowDl, 0xBC);
            w->shadow->scale[2] = 0.12f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->chara = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 0.14f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 4.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = -37.0f;
            w->model->rot[1] = -0.17453294f;
            DPRINTF("ゴエモン作りました\n");
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
            if (opdemo2Frame == 0x1392) {
                return -1;
            }
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo2Frame == 0x12A) {
                        w->model->pos[0] = 4.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -37.0f;
                        w->model->rot[1] = -0.17453294f;
                        w->state++;
                        plAnimeSet(PL(w->pl), 0xEC);
                        demoTexAnimK0(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo2Frame == 0x152) {
                        scRunStart(0x16C, 0, 0);
                        sndPlay(0x334);
                    }
                    if (opdemo2Frame > 0x152) {
                        fadeAnimStep(&w->eyes);
                        fadeAnimStep(&w->mouth);
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    if (opdemo2Frame == 0x1F2) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 2:
                    if ((step = demoPlTunagiAnime(obj, 5, step, 2, 3, 0.2f, 0.2f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 3:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (opdemo2Frame == 0x2CF) {
                        w->model->pos[0] = -18.0f;
                        w->model->pos[1] = -4.0f;
                        w->model->pos[2] = -33.0f;
                        w->model->rot[1] = -0.22689281f;
                        w->state++;
                        demoTexAnimK0(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 4:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (opdemo2Frame == 0x5B3) {
                        w->model->pos[0] = -13.0f;
                        w->model->pos[1] = 1.0f;
                        w->model->pos[2] = -122.0f;
                        w->model->rot[1] = 0.017453292f;
                        w->state++;
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (opdemo2Frame == 0x879) {
                        w->model->pos[0] = -13.0f;
                        w->model->pos[1] = 1.0f;
                        w->model->pos[2] = -103.0f;
                        w->model->rot[1] = 0.0f;
                        plAnimeSet(PL(w->pl), 0xF6);
                        demoTexAnimK0(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 6:
                    demoWalkTo(w->model, 0.15f, 0.15f, -13.0f, 0.0f);
                    if (opdemo2Frame == 0x919) {
                        w->model->pos[0] = -39.0f;
                        w->model->pos[1] = 1.0f;
                        w->model->pos[2] = -28.0f;
                        w->model->rot[1] = 0.13962634f;
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK0(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 7:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (opdemo2Frame == 0xA4F) {
                        w->model->pos[0] = -31.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -34.0f;
                        w->model->rot[1] = 0.15707964f;
                        plAnimeSet(PL(w->pl), 0xEB);
                        demoTexAnimK0(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                        scRunEnd();
                        scRunStart(0x173, 0, 0);
                        sndPlay(0x335);
                        w->state++;
                    }
                    break;
                case 8:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (demoAnimeStep(w->model, 0.14f, 2)) {
                        plAnimeSet(PL(w->pl), 0xEC);
                        demoTexAnimK0(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 9:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (opdemo2Frame == 0xB2B) {
                        w->state++;
                        demoTexAnimK0(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 10:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.3f, 0.3f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 11:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo2Frame == 0x1086) {
                        w->model->pos[0] = -39.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -37.0f;
                        w->model->rot[1] = 0.15707964f;
                        w->state++;
                    }
                    break;
                case 12:
                    if (opdemo2Frame == 0x120C) {
                        plAnimeSet(PL(w->pl), 0xFB);
                        w->state++;
                    }
                    break;
                case 13:
                    if (demoAnimeStep(w->model, 0.05f, 2)) {
                        plAnimeSet(PL(w->pl), 0xFC);
                        w->state++;
                    }
                    break;
                case 14:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo2Frame == 0x12EC) {
                        sndPlay(0x1C1);
                    }
                    if (opdemo2Frame == 0x1310) {
                        scRunEnd();
                        scRunStart(0x17C, 0, 0);
                        sndPlay(0x336);
                    }
                    break;
            }
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = w->model->pos[1] + 0.3;
            op2GoeX = w->model->pos[0];
            op2GoeZ = w->model->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *op2GoeCreate(Obj *obj) {
    return tskCreate(obj, op2GoeProc, obj->file, NULL);
}

/* "OP2EBI": Ebisumaru in the opening demo (OPDEMO2), character 1, at (59, 0, -30) under the
 * menu's last task; OBJMOVE's model 1, his shadow 0.3 above his feet. 22 steps on the demo's
 * frame count: he turns toward Goemon and then the old man (scene runs 0x16D, 0x171, 0x174,
 * 0x178, 0x17D); at 0x1392 he goes. */
s32 op2EbiProc(Obj *obj, s16 msg, void *arg);
s32 op2EbiProc(Obj *obj, s16 msg, void *arg) {
    static s32 step = 1; /* .data 0x08011E1C: his demoPlTunagiAnime step */
    s32 pad[2];
    DemoPlW *w = (DemoPlW *) (obj + 1);
    Player *p;
    PlInit init;

    DPRINTF("ebicreate\n");
    switch (msg) {
        case 2:
            obj->name = "OP2EBI";
            plMakeInit(&init, 0, 1);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = 59.0f;
            init.height = 0.0f;
            init.side = -30.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            DPRINTF("歩きモーション\n");
            plAnimeSet(PL(w->pl), 0xD7);
            w->model = p->model;
            DPRINTF("エビス丸作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, op2ShadowDl, 0xBC);
            w->shadow->scale[2] = 0.13f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 1;
            w->state = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 0.14f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 59.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = -30.0f;
            w->model->rot[1] = -0.22689281f;
            DPRINTF("エビス丸作りました\n");
            step = 0;
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
            if (opdemo2Frame == 0x1392) {
                return -1;
            }
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo2Frame == 0x12A) {
                        w->model->pos[0] = 27.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -33.0f;
                        w->model->rot[1] = -0.41887903f;
                        w->state++;
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo2Frame == 0x1F2) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 2:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.4f, 0.3f, 0xEB, 0xEC)) == -1) {
                        w->state++;
                        step = 0;
                        demoTexAnimK1(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                        scRunEnd();
                        scRunStart(0x16D, 0, 0);
                        sndPlay(0x32B);
                    }
                    break;
                case 3:
                    demoAnimeStep(w->model, 0.5f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (opdemo2Frame == 0x2CE) {
                        w->state++;
                    }
                    break;
                case 4:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.4f, 0.4f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                        step = 0;
                        scRunEnd();
                    }
                    if (opdemo2Frame == 0x2CF) {
                        plAnimeSet(PL(w->pl), 0xE9);
                        w->model->pos[0] = 7.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -32.0f;
                        w->model->rot[1] = -0.41887903f;
                        w->state += 2;
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (opdemo2Frame >= 0x2D0) {
                        w->model->pos[0] = 7.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -32.0f;
                        w->model->rot[1] = -0.41887903f;
                        w->state++;
                    }
                    break;
                case 6:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (opdemo2Frame == 0x5B3) {
                        w->model->pos[0] = 26.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -134.0f;
                        w->model->rot[1] = -0.10471976f;
                        w->state++;
                    }
                    break;
                case 7:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (opdemo2Frame == 0x879) {
                        w->model->pos[0] = 16.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -102.0f;
                        w->model->rot[1] = -0.41887903f;
                        plAnimeSet(PL(w->pl), 0xF6);
                        demoTexAnimK1(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                        scRunEnd();
                        scRunStart(0x171, 0, 0);
                        sndPlay(0x32C);
                        w->state++;
                    }
                    break;
                case 8:
                    demoWalkTo(w->model, 0.17f, 0.17f, 16.0f, 0.0f);
                    if (opdemo2Frame == 0x8FB) {
                        scRunEnd();
                    }
                    if (opdemo2Frame < 0x8FB) {
                        fadeAnimStep(&w->eyes);
                    }
                    fadeAnimStep(&w->mouth);
                    if (opdemo2Frame == 0x919) {
                        w->model->pos[0] = 1.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -26.0f;
                        w->model->rot[1] = -0.22689281f;
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 9:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo2Frame == 0xA4F) {
                        w->model->pos[0] = -8.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -31.0f;
                        w->model->rot[1] = -0.593412f;
                        w->state++;
                    }
                    break;
                case 10:
                    demoAnimeStep(w->model, 0.2f, 0);
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], op2GoeX,
                                                      op2GoeZ + 13.0f, w->model->rot[1], 1.0f);
                    if (opdemo2Frame == 0xB21) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0x174, 0, 0);
                        sndPlay(0x32D);
                    }
                    break;
                case 11:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], op2JijiX,
                                                      op2JijiZ, w->model->rot[1], 1.0f);
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.4f, 0.4f, 0xEB, 0xEC)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 12:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], op2JijiX,
                                                      op2JijiZ, w->model->rot[1], 1.0f);
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (opdemo2Frame >= 0xB54) {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    if (opdemo2Frame == 0xC11) {
                        w->state++;
                        scRunEnd();
                    }
                    break;
                case 13:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.4f, 0.4f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 14:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo2Frame == 0xF6E) {
                        w->state++;
                    }
                    break;
                case 15:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.6f, 0.6f, 0xEB, 0xEC)) == -1) {
                        w->state++;
                        step = 0;
                        scRunStart(0x178, 0, 0);
                        sndPlay(0x32E);
                    }
                    break;
                case 16:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (opdemo2Frame >= 0x100F) {
                        demoAnimeStep(w->model, 0.3f, 0);
                    }
                    if (opdemo2Frame == 0x107C) {
                        w->state++;
                        scRunEnd();
                    }
                    break;
                case 17:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.4f, 0.4f, 0xEB, 0xE9)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 18:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo2Frame >= 0x1087) {
                        w->model->pos[0] = -8.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -33.0f;
                        w->model->rot[1] = -6.0388393f;
                        w->state++;
                    }
                    break;
                case 19:
                    if (opdemo2Frame == 0x120C) {
                        plAnimeSet(PL(w->pl), 0xFB);
                        w->state++;
                    }
                    break;
                case 20:
                    if (demoAnimeStep(w->model, 0.15f, 2)) {
                        plAnimeSet(PL(w->pl), 0xFC);
                        w->state++;
                    }
                    break;
                case 21:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo2Frame == 0x1310) {
                        scRunEnd();
                        scRunStart(0x17D, 0, 0);
                        sndPlay(0x32F);
                    }
                    break;
            }
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = w->model->pos[1] + 0.3;
            op2EbiX = w->model->pos[0];
            op2EbiZ = w->model->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *op2EbiCreate(Obj *obj) {
    return tskCreate(obj, op2EbiProc, obj->file, NULL);
}

/* "jiji"'s work (tlb_0028_1's JijiW). */
typedef struct JijiW {
    /* 0x00 */ AnimeFrame *model;
    /* 0x04 */ PolFrame *shadow;
    /* 0x08 */ s32 timer; /* counts his frames; nothing reads it */
    /* 0x0C */ s32 state;
} JijiW;

/* Main's sinf, called through its address as TLB code calls main. */
#define sinf (*sinf)

extern u8 op2JijiAnim[]; /* the old man's animations (set 0xC5) */
extern u8 op2JijiState3Anim[];
extern u8 op2JijiTunagiAnim[];
extern u8 op2JijiState1Anim[];
extern u8 op2JijiState5Anim[];
extern u8 op2JijiState17Anim[];
extern u8 op2JijiState7Anim[];
extern u8 op2JijiState7EndAnim[];
extern u8 op2JijiState16Anim[];

/* "jiji": the wise old man (set 0xC5) in the opening demo (OPDEMO2) at (34, 0, 16); OBJMOVE's
 * model 2. 19 steps on the demo's frame count: he links idle and talk with tunagi_anime, turns
 * toward Ebisumaru, Goemon or the point between them (scene runs 0x16E-0x170, 0x172, 0x175-0x177,
 * 0x179-0x17B), and at the end sways his head by a sine of the frame count; at 0x1392 he goes.
 * Each frame his x and z are where Ebisumaru turns. The midpoints divide by the int 2 (2.0f
 * becomes a mul by 0.5), and step 16's turn speed is the int 2: as 2.0f it is loaded after 13.0
 * and the float registers of steps 16-18 shift. */
s32 op2JijiProc(Obj *obj, s16 msg, void *arg);
s32 op2JijiProc(Obj *obj, s16 msg, void *arg) {
    static s32 alpha = 128; /* .data 0x08011E20: his shadow's alpha */
    static s32 step = 1;    /* .data 0x08011E24: his tunagi_anime step (and step 6's flag) */
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
            demoAnimeSet(w->model, (u32) op2JijiAnim, 0xC5, 0.0f);
            demoPolSetDL(w->shadow, op2ShadowDl, 0xBC);
            w->shadow->scale[2] = 0.12f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->model->scale[2] = 0.14f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 34.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = 16.0f;
            w->model->rot[1] = -2.984513f;
            DPRINTF("物知りじいさん作りました\n");
            step = 0;
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
            if (opdemo2Frame == 0x1392) {
                return -1;
            }
            w->timer++;
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo2Frame == 0x2CF) {
                        w->model->pos[0] = -4.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -5.0f;
                        w->model->rot[1] = -3.0543263f;
                        w->state++;
                        scRunEnd();
                        scRunStart(0x16E, 0, 0);
                        sndPlay(0x33A);
                    }
                    break;
                case 1:
                    if ((step = tunagi_anime(w->model, step, 2, 2, 0.6f, 0.8f, 0xC5, (u32) op2JijiTunagiAnim, 0xC5,
                                             (u32) op2JijiState1Anim)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (opdemo2Frame < 0x347) {
                        demoAnimeStep(w->model, 0.2f, 0);
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], op2EbiX,
                                                          op2EbiZ, w->model->rot[1], 1.0f);
                    } else if (opdemo2Frame < 0x3D3) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], op2GoeX,
                                                          op2GoeZ, w->model->rot[1], 1.0f);
                    } else {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2],
                                                          (op2EbiX + op2GoeX) / 2,
                                                          (op2EbiZ + op2GoeZ) / 2,
                                                          w->model->rot[1], 1.0f);
                    }
                    if (opdemo2Frame == 0x347) {
                        scRunEnd();
                        scRunStart(0x16F, 0, 0);
                    }
                    if (opdemo2Frame == 0x491) {
                        w->state++;
                    }
                    break;
                case 3:
                    if ((step = tunagi_anime(w->model, step, 2, 3, 0.4f, 0.4f, 0xC5, (u32) op2JijiTunagiAnim, 0xC5,
                                             (u32) op2JijiState3Anim)) == -1) {
                        w->state++;
                        step = 0;
                        scRunEnd();
                    }
                    break;
                case 4:
                    demoAnimeStep(w->model, 0.4f, 0);
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], w->model->pos[0],
                                                      w->model->pos[2] + 20.0f, w->model->rot[1], 3.0f);
                    if (opdemo2Frame >= 0x514) {
                        w->state++;
                    }
                    break;
                case 5:
                    demoWalkTo(w->model, 0.4f, 0.3f, w->model->pos[0], w->model->pos[2] + 50.0f);
                    if (opdemo2Frame == 0x5B3) {
                        w->model->pos[0] = 4.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -108.0f;
                        w->model->rot[1] = 6.1610126f;
                        demoAnimeSet(w->model, (u32) op2JijiState5Anim, 0xC5, 0.0f);
                        w->state++;
                    }
                    break;
                case 6:
                    if (opdemo2Frame >= 0x5C8) {
                        if (step == 0) {
                            if (demoAnimeStep(w->model, 0.4f, 2)) {
                                step = 1;
                            }
                        } else {
                            demoAnimeStep(w->model, 0.1f, 3);
                        }
                    }
                    if (opdemo2Frame == 0x67B) {
                        w->model->pos[0] = 13.0f;
                        w->model->pos[1] = 1.0f;
                        w->model->pos[2] = -13.0f;
                        w->model->rot[1] = 3.5430186f;
                        demoAnimeSet(w->model, (u32) op2JijiAnim, 0xC5, 0.0f);
                        w->state++;
                        step = 0;
                    }
                    break;
                case 7:
                    if ((step = tunagi_anime(w->model, step, 2, 2, 0.4f, 0.25f, 0xC5, (u32) op2JijiState7Anim, 0xC5,
                                             (u32) op2JijiState7EndAnim)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    if (opdemo2Frame == 0x6C1) {
                        scRunStart(0x170, 0, 0);
                        sndPlay(0x33B);
                    }
                    break;
                case 8:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo2Frame == 0x919) {
                        w->model->pos[0] = -23.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 16.0f;
                        w->model->rot[1] = 3.1415927f;
                        w->state++;
                        demoAnimeSet(w->model, (u32) op2JijiAnim, 0xC5, 0.0f);
                        scRunEnd();
                        scRunStart(0x172, 0, 0);
                        sndPlay(0x33C);
                    }
                    break;
                case 9:
                    if ((step = tunagi_anime(w->model, step, 2, 2, 0.8f, 0.6f, 0xC5, (u32) op2JijiTunagiAnim, 0xC5,
                                             (u32) op2JijiState1Anim)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 10:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (opdemo2Frame < 0x97D) {
                        demoAnimeStep(w->model, 0.2f, 0);
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], op2EbiX,
                                                          op2EbiZ, w->model->rot[1], 1.0f);
                    } else if (opdemo2Frame < 0x9E1) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], op2GoeX,
                                                          op2GoeZ, w->model->rot[1], 1.0f);
                    } else {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2],
                                                          (op2EbiX + op2GoeX) / 2,
                                                          (op2EbiZ + op2GoeZ) / 2,
                                                          w->model->rot[1], 1.0f);
                    }
                    if (opdemo2Frame == 0xA27) {
                        w->state++;
                    }
                    break;
                case 11:
                    if ((step = tunagi_anime(w->model, step, 2, 3, 0.4f, 0.4f, 0xC5, (u32) op2JijiTunagiAnim, 0xC5,
                                             (u32) op2JijiAnim)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 12:
                    if (opdemo2Frame == 0xC2F) {
                        scRunStart(0x175, 0, 0);
                        sndPlay(0x33D);
                    }
                    if (opdemo2Frame > 0xC2F) {
                        if ((step = tunagi_anime(w->model, step, 2, 2, 0.4f, 0.3f, 0xC5, (u32) op2JijiTunagiAnim, 0xC5,
                                                 (u32) op2JijiState1Anim)) == -1) {
                            w->state++;
                            step = 0;
                            scRunEnd();
                            scRunStart(0x176, 0, 0);
                        }
                    }
                    break;
                case 13:
                    if (opdemo2Frame < 0xDAB) {
                        demoAnimeStep(w->model, 0.3f, 0);
                    } else {
                        demoAnimeStep(w->model, 0.04f, 0);
                    }
                    if (opdemo2Frame >= 0xE10) {
                        demoAnimeStep(w->model, 0.5f, 0);
                    }
                    if (opdemo2Frame == 0xE0F) {
                        scRunEnd();
                        scRunStart(0x177, 0, 0);
                    }
                    if (opdemo2Frame == 0xF6D) {
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 14:
                    if ((step = tunagi_anime(w->model, step, 2, 3, 0.4f, 0.4f, 0xC5, (u32) op2JijiTunagiAnim, 0xC5,
                                             (u32) op2JijiAnim)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 15:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (opdemo2Frame == 0x1086) {
                        w->model->pos[0] = -31.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 9.0f;
                        w->model->rot[1] = 3.1415927f;
                        scRunStart(0x179, 0, 0);
                        sndPlay(0x33E);
                    }
                    if (opdemo2Frame == 0x114E) {
                        demoAnimeSet(w->model, (u32) op2JijiAnim, 0xC5, 0.0f);
                        w->state++;
                    }
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], w->model->pos[0],
                                                      w->model->pos[2] + 13.0f, w->model->rot[1], 0.1f);
                    break;
                case 16:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], w->model->pos[0],
                                                      w->model->pos[2] + 13.0f, w->model->rot[1], 2);
                    if (demoAnimeStep(w->model, 0.4f, 2)) {
                        demoAnimeSet(w->model, (u32) op2JijiState16Anim, 0xC5, 0.0f);
                        w->state++;
                    }
                    break;
                case 17:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], w->model->pos[0],
                                                      w->model->pos[2] + 13.0f, w->model->rot[1], 3.0f);
                    if (opdemo2Frame == 0x11C6) {
                        scRunEnd();
                        scRunStart(0x17A, 0, 0);
                        sndPlay(0x33F);
                    }
                    if (opdemo2Frame == 0x120C) {
                        scRunEnd();
                    }
                    demoAnimeStep(w->model, 0.18f, 2);
                    if (opdemo2Frame == 0x120C) {
                        demoAnimeSet(w->model, (u32) op2JijiState17Anim, 0xC5, 0.0f);
                        w->state++;
                    }
                    break;
                case 18:
                    if (opdemo2Frame == 0x1270) {
                        scRunEnd();
                        scRunStart(0x17B, 0, 0);
                        sndPlay(0x340);
                    }
                    if (opdemo2Frame > 0x1270) {
                        demoAnimeStep(w->model, 0.3f, 0);
                    }
                    demoAnimeStep(w->model, 0.04f, 0);
                    w->model->rot[1] =
                        sinf(opdemo2Frame * 2 * 0.0174532930038800771) * 30.0f * 0.0174532930038800771;
                    break;
            }
            op2JijiX = w->model->pos[0];
            op2JijiZ = w->model->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *op2JijiCreate(Obj *obj) {
    return tskCreate(obj, op2JijiProc, obj->file, NULL);
}

/* "FUSUMA"'s work. */
typedef struct FusumaW {
    /* 0x00 */ AnimeFrame *frame;
} FusumaW;

extern s32 opdemo2Frame; /* the demo's frame counter */
extern s32 gDemoModelsDelete2;
extern u8 op2SlidingDoorsAnim[]; /* the sliding doors' animation */

/* "FUSUMA": the sliding doors' animated model at the origin, file 0xC1. They slide (sound
 * 0x1BE at demo frame 1499, the animation at 0.06 from 1500) and go at 5010 or when the demo
 * deletes its models. */
s32 op2FusumaProc(Obj *obj, s16 msg, void *arg) {
    FusumaW *w = (FusumaW *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "FUSUMA";
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            demoAnimeSet(w->frame, (u32) op2SlidingDoorsAnim, 0xC1, 0.0f);
            w->frame->flags |= 0x20;
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 0.0f;
            w->frame->scale[0] = 1.0f;
            w->frame->scale[1] = 1.0f;
            w->frame->scale[2] = 1.0f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (gDemoModelsDelete2) {
                return -1;
            }
            if (opdemo2Frame == 1499) {
                sndPlay(0x1BE);
            }
            if (opdemo2Frame >= 1500) {
                demoAnimeStep(((FusumaW *) FX(obj))->frame, 0.06f, 2);
            }
            if (opdemo2Frame == 5010) {
                return -1;
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *op2FusumaCreate(Obj *obj) {
    return tskCreate(obj, op2FusumaProc, obj->file, NULL);
}

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
extern u8 op2MachineAnim[]; /* the machine's animation */

/* "MACH2"'s work: its anime model and its animation speed. */
typedef struct Mach2W {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 speed;
} Mach2W;

/* "MACH2": the machine (anime of file 0xC2) at (-23, 13, 67), turned half round; OBJMOVE's
 * model 3 moves it. At demo frame 0x1086 it moves to z 51, at 0x11C6 it starts (sound 0x1C0)
 * and animates at 0.02, from 0x120D at a speed rising by 0.04 to 0.6, looping its first ten
 * frames; at 0x1392 it goes. tlb_0021_3's "MACHINE" (op3MachineProc) is its twin. */
s32 op2Mach2Proc(Obj *obj, s16 msg, void *arg) {
    Mach2W *w = (Mach2W *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "MACH2";
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            demoAnimeSet(w->frame, (u32) op2MachineAnim, 0xC2, 0);
            w->frame->pos[0] = -23.0f;
            w->frame->pos[1] = 13.0f;
            w->frame->pos[2] = 67.0f;
            w->frame->scale[0] = 1.0f;
            w->frame->scale[1] = 1.0f;
            w->frame->scale[2] = 1.0f;
            w->frame->rot[1] = 3.1415927f;
            w->speed = 0.3f;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 3:
            w->frame->dl = demoShadeDL();
            break;
        case 0:
            if (gDemoModelsDelete2) {
                return -1;
            }
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
            if (opdemo2Frame == 0x1392) {
                return -1;
            }
            if (opdemo2Frame == 0x1086) {
                w->frame->pos[2] = 51.0f;
            }
            if (opdemo2Frame == 0x11C6) {
                sndPlay(0x1C0);
            }
            if (opdemo2Frame >= 0x11C7 && opdemo2Frame < 0x120C) {
                demoAnimeStep(w->frame, 0.02f, 2);
            }
            if (opdemo2Frame >= 0x120D) {
                if (w->speed < 0.6f) {
                    w->speed += 0.04f;
                }
                demoAnimeStep(w->frame, w->speed, 2);
                if (10.0f < w->frame->frame) {
                    w->frame->frame = 0.0f;
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *op2Mach2Create(Obj *obj) {
    return tskCreate(obj, op2Mach2Proc, obj->file, NULL);
}
