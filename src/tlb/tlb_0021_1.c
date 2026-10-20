#include "tlb.h"
#include "tlb/tlb_0021.h"

/* Main's sinf and cosf, called through their addresses as TLB code calls main. */
#define sinf (*sinf)
#define cosf (*cosf)


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

extern s32 opdemo1Frame; /* OPDEMO1's frame counter (tlb_0021_0) */
extern f32 op1GoeX; /* Goemon's x, y and z (tlb_0021_0's .data) */
extern f32 op1GoeY;
extern f32 op1GoeZ;
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
AnimFrame op1CharaAnims[] = {
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
f32 op1Unused1 = 5.0f; /* unread (0x08011378, after the table's last frame) */
f32 op1Unused2 = 5.0f; /* unread */
/* The speakers' x and z (all 5.0 at first), ahead of their steps by first declaration. */
f32 opSpeakerGoeX = 5.0f; /* Goemon's x and z, again */
f32 opSpeakerGoeZ = 5.0f;
f32 opSpeakerEbiX = 5.0f; /* Ebisumaru's x and z */
f32 opSpeakerEbiZ = 5.0f;
f32 op1Unused3 = 5.0f; /* unread */
f32 op1Unused4 = 5.0f; /* unread */
extern Gfx op1ShadowDl[];         /* the shadow */
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

/* "OP1GOE": Goemon in the opening (OPDEMO1), under its task at (30, 3, -700), small (0.17).
 * He walks in from frame 0x191, drifting 0.2 a frame in z; at 0x370 scene run 0x168 starts with
 * sound 0x332 and he links his actions; at 0x686 he stands at (30, 3, -611); at 0x6A4 scene
 * run 0x16A and sound 0x333; at 0x794 he turns and walks off toward (3, -759). At 0x9EC he is
 * gone. */
s32 op1GoeProc(Obj *obj, s16 msg, void *arg) {
    static s32 step = 1; /* .data 0x08011398 */
    static Player *p;    /* .bss 0x08013770, the file's only .bss */
    s32 pad[2];
    DemoPlW *w = (DemoPlW *) (obj + 1);
    PlInit init;

    DPRINTF("goecreate\n"); /* the file's rodata has it first, before the name */
    switch (msg) {
        case 2:
            obj->name = "OP1GOE";
            plMakeInit(&init, 0, 0);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = 30.0f;
            init.height = 3.0f;
            init.side = -700.0f;
            w->pl = plCreate(obj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            DPRINTF("歩きモーション\n");
            plAnimeSet(PL(w->pl), 0xF6);
            demoTexAnimK0(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
            demoTexAnimK0(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
            w->model = p->model;
            DPRINTF("*** p->model_buff=%x\n", p->model);
            DPRINTF("ゴエモン作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, op1ShadowDl, 0xBC);
            w->shadow->scale[2] = 0.12f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->chara = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 0.17f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 30.0f;
            w->model->pos[1] = 3.0f;
            w->model->pos[2] = -700.0f;
            DPRINTF("ゴエモン作りました\n");
            break;
        case 1:
            tskDestroy(w->pl);
            frmDestroy(&w->model->f);
            frmDestroy(&w->shadow->f);
            DPRINTF("ebi\n");
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
            if (opdemo1Frame == 0x9EC) {
                return -1;
            }
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (opdemo1Frame >= 0x191) {
                        w->state++;
                    }
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.3f, 0);
                    w->model->pos[2] += 0.2;
                    if (opdemo1Frame == 0x370) {
                        step = 0;
                        w->state++;
                        scRunStart(0x168, 0, 0);
                        sndPlay(0x332);
                    }
                    break;
                case 2:
                    w->model->pos[2] += 0.2;
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.4f, 0.4f, 0xF7, 0xF6)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 3:
                    demoAnimeStep(w->model, 0.3f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    w->model->pos[2] += 0.2;
                    if (opdemo1Frame == 0x46A) {
                        step = 0;
                        w->state = 6;
                        demoTexAnimK0(&w->eyes, 5, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 5, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 4:
                    w->model->pos[2] += 0.2;
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if ((step = demoPlTunagiAnime(obj, 5, step, 2, 2, 0.4f, 0.4f, 0xF7, 0xF6)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.3f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    w->model->pos[2] += 0.2;
                    if (opdemo1Frame >= 0x46B) {
                        step = 0;
                        w->state++;
                        demoTexAnimK0(&w->eyes, 5, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 5, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 6:
                    demoAnimeStep(w->model, 0.3f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    w->model->pos[2] += 0.2;
                    if (opdemo1Frame == 0x686) {
                        w->state++;
                        w->model->pos[0] = 30.0f;
                        w->model->pos[1] = 3.0f;
                        w->model->pos[2] = -611.0f;
                        w->model->rot[1] = -0.13962634f;
                        plAnimeSet(PL(w->pl), 0xF8);
                        demoTexAnimK0(&w->eyes, 5, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 5, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 7:
                    if (opdemo1Frame == 0x6A4) {
                        scRunStart(0x16A, 0, 0);
                        demoTexAnimK0(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                        sndPlay(0x333);
                    }
                    if (opdemo1Frame >= 0x6A5) {
                        demoAnimeStep(w->model, 0.1f, 2);
                        fadeAnimStep(&w->eyes);
                        fadeAnimStep(&w->mouth);
                    }
                    if (opdemo1Frame == 0x726) {
                        demoTexAnimK0(&w->eyes, 5, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 5, &PL(w->pl)->model->segs[4]);
                    }
                    if (opdemo1Frame == 0x794) {
                        plAnimeSet(PL(w->pl), 0xF6);
                        demoTexAnimK0(&w->eyes, 5, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 5, &PL(w->pl)->model->segs[4]);
                        w->model->pos[0] = 3.0f;
                        w->model->pos[1] = -28.0f;
                        w->model->pos[2] = -611.0f;
                        w->model->rot[1] = -9.442231f;
                        w->state++;
                    }
                    break;
                case 8:
                    demoWalkTo(w->model, 0.3f, 0.3f, 3.0f, -759.0f);
                    break;
            }
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = w->model->pos[1] + 0.3;
            opSpeakerGoeX = op1GoeX = w->model->pos[0];
            opSpeakerGoeZ = op1GoeZ = w->model->pos[2];
            op1GoeY = w->model->pos[1];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *op1GoeCreate(Obj *obj) {
    return tskCreate(obj, op1GoeProc, obj->file, NULL);
}


/* "OP1EBI": Ebisumaru in the opening (OPDEMO1), under its task at (3, 3, -700), small (0.17).
 * He walks in from frame 0x191, drifting 0.2 a frame in z; at 0x482 scene run 0x169 with sound
 * 0x329; at 0x686 he stands at (1, 3, -611); at 0x71C scene run 0x16B with sound 0x32A; at
 * 0x794 he turns and walks off toward (31, -759). At 0x9EC he is gone. */
s32 op1EbiProc(Obj *obj, s16 msg, void *arg) {
    static s32 step = 1; /* .data 0x0801139C */
    s32 pad[2];
    DemoPlW *w = (DemoPlW *) (obj + 1);
    Player *p;
    PlInit init;

    switch (msg) {
        case 2:
            obj->name = "OP1EBI";
            plMakeInit(&init, 0, 1);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = 3.0f;
            init.height = 3.0f;
            init.side = -700.0f;
            w->pl = plCreate(obj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            DPRINTF("歩きモーション\n");
            plAnimeSet(PL(w->pl), 0xF6);
            DPRINTF("テクスチャアニメ初期化\n");
            DPRINTF("EBIstep---->%d\r", w->state);
            w->model = p->model;
            DPRINTF("エビス丸作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, op1ShadowDl, 0xBC);
            w->shadow->scale[2] = 0.13f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 1;
            w->state = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 0.17f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 3.0f;
            w->model->pos[1] = 3.0f;
            w->model->pos[2] = -700.0f;
            DPRINTF("エビス丸作りました\n");
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
            if (opdemo1Frame == 0x9EC) {
                return -1;
            }
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (opdemo1Frame >= 0x191) {
                        w->state = 3;
                    }
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.3f, 0);
                    w->model->pos[2] += 0.2;
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (opdemo1Frame == 0x384) {
                        w->state++;
                    }
                    break;
                case 2:
                    w->model->pos[2] += 0.2;
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 2, 0.4f, 0.4f, 0xF7, 0xF6)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 3:
                    demoAnimeStep(w->model, 0.3f, 0);
                    w->model->pos[2] += 0.2;
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (opdemo1Frame == 0x482) {
                        w->state++;
                        scRunEnd();
                        scRunStart(0x169, 0, 0);
                        sndPlay(0x329);
                    }
                    break;
                case 4:
                    w->model->pos[2] += 0.2;
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.4f, 0.4f, 0xF7, 0xF6)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.3f, 0);
                    w->model->pos[2] += 0.2;
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (opdemo1Frame == 0x5D6) {
                        w->state = 7;
                    }
                    break;
                case 6:
                    w->model->pos[2] += 0.2;
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.4f, 0.4f, 0xF7, 0xF6)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 7:
                    demoAnimeStep(w->model, 0.3f, 0);
                    w->model->pos[2] += 0.2;
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (opdemo1Frame == 0x5E0) {
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        scRunEnd();
                    }
                    if (opdemo1Frame == 0x686) {
                        plAnimeSet(PL(w->pl), 0xD7);
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->model->pos[0] = 1.0f;
                        w->model->pos[1] = 3.0f;
                        w->model->pos[2] = -611.0f;
                        w->model->rot[1] = 0.12217305f;
                        w->state++;
                        step = 0;
                    }
                    break;
                case 8:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.4f, 0.2f, 0xEA, 0xE7)) == -1) {
                        w->state++;
                        step = 0;
                    }
                    break;
                case 9:
                    demoAnimeStep(w->model, 0.3f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (opdemo1Frame == 0x71C) {
                        scRunEnd();
                        scRunStart(0x16B, 0, 0);
                        sndPlay(0x32A);
                        demoTexAnimK1(&w->eyes, 5, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 5, &PL(w->pl)->model->segs[4]);
                    }
                    if (opdemo1Frame == 0x758) {
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    if (opdemo1Frame == 0x794) {
                        w->model->pos[0] = 31.0f;
                        w->model->pos[1] = -28.0f;
                        w->model->pos[2] = -611.0f;
                        w->model->rot[1] = 9.424778f;
                        plAnimeSet(PL(w->pl), 0xF6);
                        w->state++;
                    }
                    break;
                case 10:
                    scRunEnd();
                    demoWalkTo(w->model, 0.3f, 0.3f, 31.0f, -759.0f);
                    break;
            }
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = w->model->pos[1] + 0.3;
            opSpeakerEbiX = w->model->pos[0];
            opSpeakerEbiZ = w->model->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 op1EbiProc(Obj *obj, s16 msg, void *arg);

Obj *op1EbiCreate(Obj *obj) {
    return tskCreate(obj, op1EbiProc, obj->file, NULL);
}

/* "lensflare" (its proc opLensflareProc): its frame, its alpha and the alpha's step. */
typedef struct LensflareW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ s32 alpha;
    /* 0x08 */ s32 alphaStep; /* the alpha's step */
} LensflareW;

extern s32 gDemoModelsDelete2;
extern s32 gDemoForcedFade;
extern s32 opdemo1Frame; /* OPDEMO1's frame counter (tlb_0021_0) */
extern u8 opLensflareAnim[]; /* the lens flare's animation, in file 0x13B */

s32 opLensflareProc(Obj *obj, s16 msg, void *arg);

/* "lensflare", the opening demo's: tlb_0017_2's (its twin, parked there too) at 0.7 and fading
 * by 4 a step. It sits where the view line meets the plane halfway between the eye and its own
 * position, faces the eye, and dims the screen by how straight the eye looks at it unless the
 * demo forces the fade. It ends at frame 2540 of the demo, when the demo's models go, or when
 * its alpha runs out; below 200 it flickers, more below 100. Parked as its twin is: 497 lines
 * (mix 7), the game spilling its position loads and temps to slots this C keeps in registers.
 * 2026-10-12 (from 246): dx, dy, dz computed in that order (the game's stores 0x80,
 * 0x84, 0x88 go in that order): 244, kept. Then ez, ey, ex declared (the game homes ey at 0x90
 * and ez at 0x94): 244, no change (here they are never spilled). px/py/pz and the dot product
 * as locals, pad dropped: frame 0xB8 (266). Left: the game holds ey/ez in f12/f14 and spills
 * them to their homes for mathAtan2's arguments, w to 0x40 and mathAtan2's address to 0x3C;
 * vx/vy/vz in f2/f16/f18 (this C f18/f14/f10).
 * 2026-10-12 (second try), from 244: w set at its declaration (the twin's lever, IDO_NOTES 378)
 * and the three per-case assignments gone: 179 (frame 8 over); pad[2] gone: 161 (case 1 through
 * w or FX(obj): the same); the frame an AnimeFrame (the game reads flags at 0 and dl at 0x40,
 * as the twin's; PolFrame put them at 0x38/0x3C), its casts gone: 154. Left: case 0's floats
 * (the game: ex f0, pos[0..2] spilled to 0x34/0x2C/0x30 as soon as loaded, vx/vy/vz in
 * f2/f16/f18; this C: ex f2, pos[1]/pos[2] kept in f0/f16, vy/vz spilled to their homes).
 * The twin's lever of the same day (a FAKE block boundary before vx: 137 to 94 there) gives
 * this function the twin's colouring (ex f0, ey f2, ez f16), not this game's (ex f0, vx f2, ey
 * f12, ez f14, vy f16, vz f18): with pad[2] (the boundary takes the frame to 0x98) 154. Other
 * places, one check each: before dx 154 (no change); between vx and vy 162; after vz, pad[2]
 * 152 (ey f2, ez f16 again). All taken back.
 * match_function.py splits (pad[2] in, 140 places): best one boundary after "dx = ..": real
 * 145 with pad[2] (kept, FAKE): the game's coloured set (ex f0 and ey, ez, vx, vy, vz in f2,
 * f12-f18, positions spilled), but permuted (here ey f2, ez f16, vx f18, vy f12, vz f14).
 * Splits' next best: eight boundaries after q[1] (270 lines). A second sweep from 145: best
 * one more after t (298 to 294 lines), not taken. The twin's later finds: (0, dz) in t's
 * divisor 149; the fade level without the store back to c 145 (no change). Both taken back.
 * 2026-10-12, match_function.py trace (145): every case-0 float variable is 3.0 and
 *   col.txt colours them in range order ex, ey, ez, vx, vy, vz: ex f0, ey f2, ez f16 (f12/f14
 *   forbidden: its range reaches block 21, mathAtan2's arguments), vx f18, vy f12, vz f14; dx,
 *   dy, dz split and uncoloured. The game's set is ex f0, vx f2, ey f12, ez f14, vy f16, vz f18,
 *   which fits the same order with ey and ez forbidden f2 instead of f12/f14 (unverified: their ranges end
 *   before the mathAtan2 block; the game stores them to their homes 0x90/0x94 there).
 * 2026-10-15 (145): the twin's slot lever (uopt.c f_spilltemps: the words under the homes
 *   are uopt spill slots, given first-fit in expression-bit order; pad[2] as one word moved the
 *   twin's slots onto the game's) here: slots 0x2C-0x3C (game 0x2C-0x40, the game's extra one
 *   being w's own spill at 0x40: there w is not coloured), real 151. Taken back: the slots
 *   follow the colouring, which is what differs here.
 * 2026-10-16 (last-percent push; start 145; IDA on 0021's .i64, 080060CC). The twin's
 *   view lever (its note: every position read as "((Vec3f *) w->frame->pos)->x/y/z", term 3
 *   "add.s f6,f14(ez),f8(pos2)" at 0x080062D4 is source order): 149. With it this C's case 0 is
 *   the game's instruction for instruction but for registers: here the twin's colouring (ex f0,
 *   ey f2, ez f16, vx f18, vy f12, vz f14), the game's ex f0, vx f2, ey f12, ez f14, vy f16, vz
 *   f18, ey/ez stored to their homes 0x90/0x94 (0x08006298) and reloaded for q[1]/q[2].
 *   Read in n64decomp/ido's decompiled uopt (uoptreg2.c globalcolor, compute_save): ranges are
 *   picked by adjsave, ties by bit; a range splits when adjsave x count <= its best cost. The
 *   game's set is vx, vy, vz coloured BEFORE ey/ez (vx f2: f0 forbidden by mathAtan2's result;
 *   vy f16, vz f18: f12/f14 forbidden through block 21), then ex f0, then ey/ez find only
 *   callee-saved registers, split off block 21 (their stores and reloads) and take f12/f14, then
 *   the pos loads find nothing. Here all of ex..vz and the pos loads 297/300 are 3.0 (6 refs over
 *   count 2; 3 refs in one block), so it needs adjsave(v) > adjsave(ey, ez) >= adjsave(pos).
 *   Measured on the view form (one check each, all taken back): no FAKE with pad[2] 166, without
 *   pad 147 (block 18 then holds ex..t); the FAKE before vx 159; three boundaries after dx 175
 *   (v's first with the game's f2/f16/f18, but e's at 2.0 fall below the pos loads, which take
 *   f0/f12); six after dx and one after vz 256 (v's f2/f16/f18; k 2.0 takes f0, pos 1.5 take
 *   f12/f14, e's split to nothing); "ez, ey, ex" declared (the game's homes ey 0x90, ez 0x94)
 *   149, no change; the eye loads before the demo tests 176; "vx = pos; vx -= ex;" 180 (block
 *   limit moved); empty "if (vx) {}" tests 223 (+16, float compares). Any boundary inside the pos
 *   loads' span is inside the v's span too, so no layout of these statements gives v > e >= pos.
 *   force cannot pin a split; force ey 26, ez 27, vx 25, vy 28, vz 29 (CONFLICT on ey/ez): 137.
 * 2026-10-16 (second pass, start 145, end 145; IDA 0021 .i64 080060CC; n64decomp/ido src/uopt read).
 *   Length -4 named: the game's ey/ez home stores swc1 f12,0x90 / swc1 f14,0x94 (0x08006298) and
 *   reloads lwc1 0x90 / 0x94 for q[1]/q[2] (0x08006360, 0x08006380), i.e. ey/ez split off the
 *   q block; no other unpaired instruction. They come only with the game's colouring.
 *   Mechanism, read and measured (trace each):
 *   - Block limit (uoptinput.c getop 1658-1707; uoptmain.c: calls and labels also end blocks): a load
 *     of a variable counts unless it was assigned in the block from a load-free value (then it
 *     is substituted). From the eye loads to the end of k this C counts 24 (cam 6, w 6, e 9, v 3),
 *     so without the FAKE the block ends after k.
 *   - split (uoptreg2.c 1573, addadjacents 1098): ey's piece starts at its def's block and takes
 *     the next block only if fewer than regsleft of its interferences are new there. With any
 *     boundary between ey's def and t, block 20 is refused (FAKE after dx, FAKE before vx): the
 *     block-20 piece gets a new late bit and the pos loads (3.0, bits 297/300) take f12/f14.
 *   - Diagnostic (all FAKE, not kept), proving the target: no FAKE, w assigned after the demo
 *     tests (w's 6 loads substituted, 19 refs at k, block = eye loads..t), and dead stores
 *     "vx = 0.0f; vy = 0.0f; vz = 0.0f;" at the head of case 0 (v's bits 45/48/51, before ex 64):
 *     colouring exactly the game's (vx f2, vy f16, vz f18, ex f0, ey split {18} f12, ez f14, pos
 *     loads uncoloured), real 241. The rest differs because a w assigned in the block is folded
 *     (getop 1744: uadd + constant) into "lw 32(obj)" for w->frame, where the game has "lw 0(a4)".
 *   So the game needs (1) one block from ey's def through t with w NOT substituted, which this
 *   C's references cannot fit (21 even from ey's def), and (2) the v's ahead of ey/ez in bit
 *   order (all six tie at 3.0; globalcolor keeps bit order on a tie). Neither natural form found.
 *   Tries, one check each, all taken back: view form 149; Vec3f e (homes x<y<z, as the game's
 *   ey 0x90 < ez 0x94) 149, no change; f32 e[3] 179 (kept in memory); "vx = pos; ... vx -= ex"
 *   with the v's first 201 (v bits first, but their ranges start in block 18: 2.67);
 *   "vx = vy = vz = 0.0f" 244 (cfe chain temps); the same as three stores, FAKE after dx 246;
 *   no FAKE 166 (block ends after k, v's 2.0); d's from cam->eye directly 208 (eye CSEs 6.0 take
 *   f0/f2/f12); w after the tests, view, no FAKE 276; + dead v stores 241 (the diagnostic);
 *   FAKE before vx + dead v stores 246 (ey's piece {18,19} takes f2); w as (obj + 1) and as
 *   ((u8 *) obj + sizeof(Obj)) 241, the same fold.
 * 2026-10-16 (third pass, start 145): MATCH ("except own section addresses", 384 lines). IDA 0021
 *   .i64 080060CC read first; uoptinput.c getop 1658-1707 and uoptreg2.c globalcolor 2045-2080
 *   (n64decomp/ido). Requirement (1), the block: getop counts only Ulod of an isvar; trace's
 *   pri.txt shows every global here is uilod(lda) (gGameWork.view bit 45, opdemo1Frame 48), so
 *   eye/at read through gGameWork.view cost nothing, where cam's six loads count. Tries (real):
 *   - "cam = gGameWork.view;" after the demo tests, FAKE do-while gone: 177 (frame 0xA8), and
 *     pad[2] gone 159: cam's value is an ilod of count 1, so its loads still count (getop
 *     1678-1683); the block still ends after k.
 *   - no cam, eye/at as gGameWork.view->eye/at: 173. Trace: one block from the eye loads through
 *     t (18 counted by the end of k), w not substituted; colouring still the twin's (bits ex 52
 *     < ey 56 < ez 60 < vx 79).
 *   - + "vx = 0.0f; vy = 0.0f; vz = 0.0f;" at the head of case 0 (the second pass's diagnostic): 148,
 *     length +0, colouring exactly the game's (vx f2, vy f16, vz f18, ex f0, ey f12, ez f14 with
 *     their home stores); frame 0x98 (game 0xA0).
 *   - + "CamView *cam;" after w (the game's never-written word at -8, as the sibling's cam):
 *     frame 0xA0, 94. ez, ey, ex declared (the game's homes ey 0x90, ez 0x94): 93.
 *   - the sibling's Vec3f view on all nine position reads: 2 (left: c * 2 - 1 kept in c, f14,
 *     where the game has a temp f0). The sibling's fade level "(c * 2 - 1.0f) * (c * 2 - 1.0f)":
 *     MATCH.
 *   - cam set and tested at the head of case 0 (no unused local): MATCH, kept.
 *   - the dead stores as vx/vy/vz's initialisers at their declaration: MATCH, kept (FAKE; the
 *     original's form of the bit order is unknown).
 * 2026-10-17 (certain pass): the Vec3f view of pos gone, k written z term
 *   first (IDO reverses a sum whose terms all hold array elements), as the sibling's: MATCH, object
 *   identical. */
s32 opLensflareProc(Obj *obj, s16 msg, void *arg) {
    LensflareW *w = (LensflareW *) FX(obj);
    CamView *cam;
    f32 ez;
    f32 ey;
    f32 ex;
    f32 dz;
    f32 dy;
    f32 dx;
    f32 q[3];
    /* The zero initialisers are dead, but they put vx, vy, vz ahead of ex, ey, ez for uopt, so
     * the v's get their registers first, as in the game */
    f32 vx = 0.0f;
    f32 vy = 0.0f;
    f32 vz = 0.0f;
    f32 k;
    f32 t;
    f32 c;

    switch (msg) {
        case 2:
            obj->name = "lensflare";
            if ((w->frame = (AnimeFrame *) frmCreate(gFrmLayer8, gFrmKindAnime)) == NULL) {
                return -1;
            }
            w->frame->anim = opLensflareAnim;
            floadGetSetRam((void **) w->frame->segs, 0x13B);
            w->frame->scale[2] = 0.7f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->alpha = 255;
            w->alphaStep = -4;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            cam = gGameWork.view;
            if (cam == NULL) {
                break;
            }
            if (opdemo1Frame == 2540) {
                return -1;
            }
            if (gDemoModelsDelete2) {
                return -1;
            }
            /* Read through gGameWork.view, not cam, so that uopt's block runs from the eye loads
             * through t, as the game's does */
            ex = gGameWork.view->eye[0];
            ey = gGameWork.view->eye[1];
            ez = gGameWork.view->eye[2];
            dx = gGameWork.view->at[0] - ex;
            dy = gGameWork.view->at[1] - ey;
            dz = gGameWork.view->at[2] - ez;
            vx = w->frame->pos[0] - ex;
            vy = w->frame->pos[1] - ey;
            vz = w->frame->pos[2] - ez;
            k = vz * (ez + w->frame->pos[2]) / 2 +
                (vy * (ey + w->frame->pos[1]) / 2 + (w->frame->pos[0] + ex) * vx / 2);
            t = (k - vx * ex - vy * ey - vz * ez) / (vx * dx + vy * dy + vz * dz);
            q[0] = dx * t + ex - w->frame->pos[0];
            q[1] = dy * t + ey - w->frame->pos[1];
            q[2] = dz * t + ez - w->frame->pos[2];
            w->frame->rot[1] = mathAtan2(q[0], q[2]);
            mathVecRotateY(-w->frame->rot[1], &q[0], &q[1], &q[2]);
            w->frame->rot[0] = mathAtan2(q[2], q[1]) - 1.5707964f;
            w->frame->rot[2] = 0.0f;
            c = (vx * dx + vy * dy + vz * dz) / sqrtf((vx * vx + vy * vy + vz * vz) * (dx * dx + dy * dy + dz * dz));
            if (!gDemoForcedFade) {
                fadeSetColor(255, 255, 255);
            }
            if (c > 0.5 && !gDemoForcedFade) {
                fadeSetLevel((c * 2 - 1.0f) * (c * 2 - 1.0f) * 0.8 * w->alpha);
            } else {
                fadeSetLevel(0.0f);
            }
            w->alpha += w->alphaStep;
            if (w->alpha < 0) {
                w->alpha = 0;
                return -1;
            }
            if (w->alpha > 255) {
                w->alpha = 255;
            }
            w->frame->f.flags |= 2;
            if (w->alpha < 200 && (w->alpha % 14 < 3 || w->alpha % 20 < 3)) {
                w->frame->f.flags &= ~2;
            }
            if (w->alpha < 100 && (w->alpha % 5 < 2 || w->alpha % 10 < 3)) {
                w->frame->f.flags &= ~2;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombine(gGfxDlHead++, 0xFF9BFF, 0xFF36FF7F);
            gDPSetEnvColor(gGfxDlHead++, 255, 255, 255, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "lensflare" at x, y, z under obj. */
Obj *opLensflareCreate(Obj *obj, f32 x, f32 y, f32 z) {
    Obj *o = tskCreate(obj, opLensflareProc, obj->file, NULL);

    if (o != NULL) {
        ((LensflareW *) FX(o))->frame->pos[0] = x;
        ((LensflareW *) FX(o))->frame->pos[1] = y;
        ((LensflareW *) FX(o))->frame->pos[2] = z;
    }
    return o;
}

void opLensflareSetFadeStep(Obj *o, s32 v) {
    ((LensflareW *) FX(o))->alphaStep = v;
}

/* "gate"'s work. */
typedef struct GateW {
    /* 0x00 */ AnimeFrame *frame;
} GateW;

extern f32 op1GoeX; /* tlb_0021_0's (the overlay's .data starts with that file's) */
extern f32 op1GoeY;
extern s32 opdemo1Frame; /* the demo's frame counter */
extern s32 gDemoModelsDelete2;
extern u8 opGateAnim[]; /* the gate's animation */

/* "gate": the gate's animated model at (22, -31, -788), file 0xC1. It opens (sound 0x1BD at
 * demo frame 2080, its animation at 0.06 from 2081) and goes at 2540 or when the demo deletes
 * its models. */
s32 opGateProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    GateW *w = (GateW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "gate";
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->anim = opGateAnim;
            floadGetSetRam((void **) w->frame->segs, 0xC1);
            w->frame->scale[2] = 1.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = 22.0f;
            w->frame->pos[1] = -31.0f;
            w->frame->pos[2] = -788.0f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (opdemo1Frame == 2540) {
                return -1;
            }
            if (gDemoModelsDelete2) {
                return -1;
            }
            if (opdemo1Frame == 2080) {
                sndPlay(0x1BD);
            }
            if (opdemo1Frame > 2080) {
                demoAnimeStep(((GateW *) FX(obj))->frame, 0.06f, 2);
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *opGateCreate(Obj *obj) {
    return tskCreate(obj, opGateProc, obj->file, NULL);
}

/* The third "gate"'s work: eight ripples. */
typedef struct GateRippleW {
    /* 0x00 */ PolFrame *frames[8];
} GateRippleW;

#define ABS(x) ((x) > 0.0f ? (x) : -(x))

extern Gfx opGateRippleDl[]; /* a ripple */
/* The file's last .data; IDO pads it to 16 (0x080113D8). tlb_0021_2's .data starts at 0x080113E0
 * with its copy of the characters' texture animations. */
Gfx opGateEffectDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPEndDisplayList(),
}; /* its setup */

/* The third "gate": eight ripples (file 0xC0) on layer 8 at random places (within 50 across, up
 * to 700 back), their widths |sin| and depths |cos| + 1 of the demo frame (each 30 frames on from
 * the last); from frame 880 the depths ease (0.02 a frame) toward 0.3 |cos| + 0.3. It goes at
 * 2540 or when the demo deletes its models. */
s32 opGateRipplesProc(Obj *obj, s16 msg, void *arg) {
    GateRippleW *w;
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "gate";
            w = (GateRippleW *) FX(obj);
            for (i = 0; i < 8; i++) {
                if ((w->frames[i] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                    return -1;
                }
                w->frames[i]->dl2 = opGateRippleDl;
                floadGetSetRam((void **) w->frames[i]->segs, 0xC0);
                w->frames[i]->scale[2] = 1.0f;
                w->frames[i]->scale[0] = w->frames[i]->scale[1] = w->frames[i]->scale[2];
                w->frames[i]->pos[0] = fadeRandRange(-50, 50);
                w->frames[i]->pos[1] = 0.0f;
                w->frames[i]->pos[2] = -fadeRand(700);
                /* the table passed as a pointer (fadeSegToRam takes a void *): the game forms its
                 * address at each call */
                w->frames[i]->dl = (Gfx *) fadeSegToRam(opGateEffectDl, obj->file);
            }
            break;
        case 1:
            w = (GateRippleW *) FX(obj);
            for (i = 0; i < 8; i++) {
                frmDestroy(&w->frames[i]->f);
            }
            break;
        case 0:
            if (opdemo1Frame == 2540) {
                return -1;
            }
            if (gDemoModelsDelete2) {
                return -1;
            }
            w = (GateRippleW *) FX(obj);
            if (opdemo1Frame < 880) {
                for (i = 0; i < 8; i++) {
                    w->frames[i]->scale[0] = ABS(sinf((i * 30 + opdemo1Frame) * 0.0174532930038800771));
                    w->frames[i]->scale[1] = ABS(cosf((i * 30 + opdemo1Frame) * 0.0174532930038800771)) + 1.0f;
                }
            } else {
                for (i = 0; i < 8; i++) {
                    w->frames[i]->scale[0] = ABS(sinf((i * 30 + opdemo1Frame) * 0.0174532930038800771));
                    if (w->frames[i]->scale[1] <
                        ABS(cosf((i * 30 + opdemo1Frame) * 0.0174532930038800771) * 0.3f) + 0.3f) {
                        w->frames[i]->scale[1] += 0.02;
                    } else {
                        w->frames[i]->scale[1] -= 0.02;
                    }
                }
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 opGateRipplesProc(Obj *obj, s16 msg, void *arg);

Obj *opGateRipplesCreate(Obj *obj) {
    return tskCreate(obj, opGateRipplesProc, obj->file, NULL);
}

/* The second "gate"'s work: two glows. */
typedef struct GateGlowW {
    /* 0x00 */ PolFrame *frames[2];
    /* 0x08 */ f32 x; /* their place, random about (0, -330) */
    /* 0x0C */ f32 z;
    /* 0x10 */ s32 alpha;
} GateGlowW;

extern Gfx opGateGlowDl[]; /* a glow */

/* The second "gate": two glows (file 0xC0) on layer 8, at a random place within 30 of (0, -330),
 * swaying 20 about it out of step (30 frames apart) and pulsing 96 +- 20 in alpha; it goes at
 * 2540 or when the demo deletes its models. The angle's "* 2" is an int (as "* 2.0" IDO makes an
 * add.d), and the cases are in the game's order (2, 3, 1, 0: its jump targets). */
s32 opGateGlowsProc(Obj *obj, s16 msg, void *arg) {
    GateGlowW *w;
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "gate";
            w = (GateGlowW *) FX(obj);
            for (i = 0; i < 2; i++) {
                if ((w->frames[i] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                    return -1;
                }
                w->frames[i]->dl2 = opGateGlowDl;
                floadGetSetRam((void **) w->frames[i]->segs, 0xC0);
                w->frames[i]->scale[2] = 1.0f;
                w->frames[i]->scale[0] = w->frames[i]->scale[1] = w->frames[i]->scale[2];
                w->frames[i]->pos[0] = w->x = fadeRandRange(-30, 30);
                w->frames[i]->pos[1] = 0.01f;
                w->frames[i]->pos[2] = w->z = fadeRandRange(-30, 30) + -330.0f;
                w->frames[i]->dl = (Gfx *) fadeSegToRam(opGateEffectDl, obj->file);
            }
            break;
        case 3:
            w = (GateGlowW *) FX(obj);
            for (i = 0; i < 2; i++) {
                w->frames[i]->dl = demoColorDL(2, 2, 1, 1, 2, 1, w->alpha + i * 40);
            }
            break;
        case 1:
            w = (GateGlowW *) FX(obj);
            for (i = 0; i < 2; i++) {
                frmDestroy(&w->frames[i]->f);
            }
            break;
        case 0:
            if (opdemo1Frame == 2540) {
                return -1;
            }
            w = (GateGlowW *) FX(obj);
            if (gDemoModelsDelete2) {
                return -1;
            }
            for (i = 0; i < 2; i++) {
                w->frames[i]->pos[0] =
                    ABS(sinf((i * 30 + opdemo1Frame) * 0.0174532930038800771 * 2) * 20.0f) + w->x;
                w->frames[i]->pos[2] =
                    ABS(cosf((i * 30 + opdemo1Frame) * 0.0174532930038800771 * 2) * 20.0f) + (w->z + 1.0f);
                w->alpha = sinf((i * 30 + opdemo1Frame) * 0.0174532930038800771 * 4.0) * 20.0f + 96.0f;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 opGateGlowsProc(Obj *obj, s16 msg, void *arg);

Obj *opGateGlowsCreate(Obj *obj) {
    return tskCreate(obj, opGateGlowsProc, obj->file, NULL);
}
