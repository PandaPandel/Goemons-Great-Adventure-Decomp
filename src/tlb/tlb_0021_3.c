#include "tlb.h"
#include "tlb/tlb_0021.h"

/* Main's sinf, called through its address as TLB code calls main. */
#define sinf (*sinf)

extern s32 opdemo3Frame; /* the demo's frame count */
extern u8 gDemoCamHold;
extern s32 gDemoModelsDelete2; /* 1 deletes the demo's models */
extern s32 gObjMoveSel; /* OBJMOVE: the model chosen, -1 for none */
extern f32 gObjMovePosX; /* the chosen model's position and angle, as shown */
extern f32 gObjMovePosY;
extern f32 gObjMovePosZ;
extern f32 gObjMoveRot;
extern f32 gObjMoveDx; /* the moves this frame: x, y, z, angle */
extern f32 gObjMoveDy;
extern f32 gObjMoveDz;
extern f32 gObjMoveDrot;
extern u8 op3SparkleDl[]; /* a sparkle's display list */
extern u8 op3ShadowDl[]; /* a shadow's display list (file set 0xBC) */


/* The demo players' work ("OP3GOE", "OP3EBI"): the player task, its model, its shadow and the
 * face's two texture animations. */
typedef struct Tlb21DemoPlW {
    /* 0x00 */ Obj *pl;
    /* 0x04 */ AnimeFrame *model;
    /* 0x08 */ PolFrame *shadow;
    /* 0x0C */ Anim eyes;
    /* 0x1C */ Anim mouth;
    /* 0x2C */ s32 chara;
    /* 0x30 */ u8 pad30[4];
    /* 0x34 */ s32 state;
} Tlb21DemoPlW;

extern CamView *opDemoView; /* the demo's camera view */
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

/* The file's .data, in the game's order: the faces' animation frames, then each character's
 * published place and his statics, declared where the game declares them (IDO_NOTES 343). */
AnimFrame op3FaceAnimFrames[] = {
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
f32 op3JijiX = 5.0f; /* the old man's x and z */
f32 op3JijiZ = 5.0f;
f32 op3BisuX = 5.0f; /* Bismaru's x and z */
f32 op3BisuZ = 5.0f;
f32 op3BisuElegantX = 5.0f; /* Bismaru Elegant's x, z and y */
f32 op3BisuElegantZ = 5.0f;
f32 op3BisuElegantY = 5.0f;
f32 op3GoeX = 5.0f; /* Goemon's x and z, each frame */
f32 op3GoeZ = 5.0f;
f32 op3EbiX = 5.0f; /* Ebisumaru's x and z */
f32 op3EbiZ = 5.0f;
f32 opMachineHeight = 5.0f; /* the machine's height, for the others */
f32 op3Unused1 = 5.0f; /* two words nothing reads */
f32 op3Unused2 = 5.0f;

/* "OP3GOE" (Goemon, character 0, in the opening's third scene) at (-32, 0, -50) under the
 * menu's last task; OBJMOVE's model 0 moves him. His places, animations, joined animations and
 * face follow the demo's frame count (scene runs 0x181, 0x188 and 0x18E-0x190); at the end he
 * walks off to (-8, -340).
 * His step and sound flag are function statics (IDO_NOTES 343), initialised, so .data
 * (0x08012880-84); the flag is zeroed after init's place. */
s32 op3GoeProc(Obj *obj, s16 msg, void *arg) {
    static s32 op3GoeStep = 1; /* his demoPlTunagiAnime step */
    static s32 op3GoeWalkSnd = 0; /* 1 while his walk's sound plays */
    s32 step;
    s32 pad;
    Tlb21DemoPlW *w = (Tlb21DemoPlW *) (obj + 1);
    Player *p;
    PlInit init;

    switch (msg) {
        case 2:
            PRINTF("goecreate\n");
            obj->name = "OP3GOE";
            plMakeInit(&init, 0, 0);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = -32.0f;
            init.height = 0.0f;
            init.side = -50.0f;
            op3GoeWalkSnd = 0;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            PRINTF("デモモード突入\n");
            PRINTF("歩きモーション\n");
            plAnimeSet(PL(w->pl), 0xD7);
            PRINTF("*** p->model_buff=%x\n", p->model);
            w->model = p->model;
            demoTexAnimK0(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
            demoTexAnimK0(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
            PRINTF("goe_ani_flame -->%f\n", p->model->frame);
            PRINTF("ゴエモン作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, (Gfx *) op3ShadowDl, 0xBC);
            w->shadow->scale[2] = 0.12f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->chara = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 0.28f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -32.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = -50.0f;
            w->model->rot[1] = 0.0f;
            op3GoeStep = 0;
            PRINTF("ゴエモン作りました\n");
            PRINTF("ebicreate\n");
            break;
        case 1:
            tskDestroy(w->pl);
            frmDestroy((Frame *) w->model);
            frmDestroy((Frame *) w->shadow);
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
            if (opdemo3Frame == 0x1096) {
                return -1;
            }
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo3Frame >= 0x12D) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], op3BisuX,
                                                          op3BisuZ, w->model->rot[1], 1.0f);
                    }
                    if (opdemo3Frame == 0x1FC) {
                        w->model->pos[0] = 86.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -19.0f;
                        w->model->rot[1] = 1.466076612f;
                        w->state++;
                    }
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo3Frame == 0x260) {
                        w->state++;
                        plAnimeSet(PL(w->pl), 0xF9);
                        demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 2:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (demoAnimeStep(w->model, 0.2f, 2)) {
                        plAnimeSet(PL(w->pl), 0xFA);
                        demoTexAnimK0(&w->eyes, 3, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 3, &PL(w->pl)->model->segs[4]);
                        w->state++;
                        op3GoeStep = 0;
                        scRunEnd();
                        scRunStart(0x181, 0, 0);
                        sndPlay(0x337);
                    }
                    break;
                case 3:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo3Frame == 0x328) {
                        scRunEnd();
                        w->state++;
                        plAnimeSet(PL(w->pl), 0xF9);
                        w->model->frame = polAnimeLastFrame(w->model);
                    }
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
                case 4:
                    if (demoAnimeStep(w->model, 0.1f, 3)) {
                        plAnimeSet(PL(w->pl), 0xE9);
                        w->state++;
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo3Frame == 0x76C) {
                        w->model->pos[0] = 73.0f;
                        w->model->pos[1] = 1.0f;
                        w->model->pos[2] = -12.0f;
                        w->model->rot[1] = 1.692969441f;
                        plAnimeSet(PL(w->pl), 0xF9);
                        w->state++;
                    }
                    break;
                case 6:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (demoAnimeStep(w->model, 0.2f, 2)) {
                        plAnimeSet(PL(w->pl), 0xFA);
                        demoTexAnimK0(&w->eyes, 3, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 3, &PL(w->pl)->model->segs[4]);
                        w->state++;
                        op3GoeStep = 0;
                        scRunStart(0x188, 0, 0);
                        sndPlay(0x338);
                    }
                    break;
                case 7:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (opdemo3Frame >= 0x7D2) {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    if (opdemo3Frame == 0x80D) {
                        scRunEnd();
                        w->state++;
                        plAnimeSet(PL(w->pl), 0xF9);
                        w->model->frame = polAnimeLastFrame(w->model);
                    }
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
                case 8:
                    if (demoAnimeStep(w->model, 0.2f, 3)) {
                        plAnimeSet(PL(w->pl), 0xE9);
                        demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 9:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo3Frame == 0xB7F) {
                        w->model->pos[0] = 52.0f;
                        w->model->pos[1] = 2.0f;
                        w->model->pos[2] = -25.0f;
                        w->model->rot[1] = 0.6108652353f;
                        w->state++;
                    }
                    break;
                case 10:
                    demoAnimeStep(w->model, 0.2f, 0);
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], op3JijiX,
                                                      op3JijiZ, w->model->rot[1], 1.0f);
                    if (opdemo3Frame == 0xE0C) {
                        w->state++;
                        op3GoeStep = 0;
                    }
                    break;
                case 11:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], op3JijiX,
                                                      op3JijiZ, w->model->rot[1], 1.0f);
                    op3GoeStep = step = demoPlTunagiAnime(obj, 3, op3GoeStep, 2, 2, 0.4f, 0.4f,
                                                                   0xEB, 0xEC);
                    if (step == -1) {
                        w->state++;
                        op3GoeStep = 0;
                        scRunEnd();
                        scRunStart(0x18E, 0, 0);
                        sndPlay(0x339);
                    }
                    break;
                case 12:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (opdemo3Frame >= 0xEB7) {
                        demoAnimeStep(w->model, 0.1f, 0);
                    }
                    switch (opdemo3Frame) {
                        case 0xEB6:
                            scRunEnd();
                            scRunStart(0x18F, 0, 0);
                            break;
                        case 0xF38:
                            scRunEnd();
                            scRunStart(0x190, 0, 0);
                            break;
                        case 0xF60:
                            w->state++;
                            break;
                    }
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
                case 13:
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], op3EbiX,
                                                      op3EbiZ, w->model->rot[1], 4.0f);
                    if (opdemo3Frame == 0xF74) {
                        plAnimeSet(PL(w->pl), 0xDC);
                        demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 14:
                    demoWalkTo(w->model, 0.6f, 1.5f, -8.0f, -340.0f);
                    if (w->model->frame > 10.0f && op3GoeWalkSnd == 0) {
                        SoundLocation2(0x40B, opDemoView, w->model->pos[0], w->model->pos[1],
                                       w->model->pos[2], 30.0f, 6000.0f, opDemoView->fovy);
                        op3GoeWalkSnd = 1;
                    }
                    if (w->model->frame < 1.0 && op3GoeWalkSnd == 1) {
                        SoundLocation2(0x40B, opDemoView, w->model->pos[0], w->model->pos[1],
                                       w->model->pos[2], 30.0f, 6000.0f, opDemoView->fovy);
                        op3GoeWalkSnd = 0;
                    }
                    break;
            }
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = w->model->pos[1] + 0.3;
            op3GoeX = w->model->pos[0];
            op3GoeZ = w->model->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 op3GoeProc(Obj *obj, s16 msg, void *arg);

Obj *op3GoeCreate(Obj *obj) {
    return tskCreate(obj, op3GoeProc, obj->file, NULL);
}


/* "OP3EBI" (Ebisumaru, character 1, in the opening's third scene) at (46, 0, -45) under the
 * menu's last task; OBJMOVE's model 1 moves him. His places, joined animations and face follow
 * the demo's frame count (scene runs 0x189 and 0x191); at the end he walks off to (-8, -340).
 * His step and sound flag are function statics (IDO_NOTES 343), initialised, so .data
 * (0x08012888-8C). The result is tested through a local step: the game tests v0. */
s32 op3EbiProc(Obj *obj, s16 msg, void *arg) {
    static s32 op3EbiStep = 1; /* his demoPlTunagiAnime step */
    static s32 op3EbiWalkSnd = 0; /* 1 while his walk's sound plays */
    s32 step;
    s32 pad;
    Tlb21DemoPlW *w = (Tlb21DemoPlW *) (obj + 1);
    Player *p;
    PlInit init;

    switch (msg) {
        case 2:
            obj->name = "OP3EBI";
            plMakeInit(&init, 0, 1);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = 46.0f;
            init.height = 0.0f;
            init.side = -45.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            PRINTF("デモモード突入\n");
            PRINTF("歩きモーション\n");
            plAnimeSet(PL(w->pl), 0xD7);
            PRINTF("ebi_ani_flame   -->%f\n", p->model->frame);
            PRINTF("エビス丸作ります\n");
            w->model = p->model;
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, (Gfx *) op3ShadowDl, 0xBC);
            w->shadow->scale[2] = 0.13f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 1;
            w->state = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 0.28f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 46.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = -45.0f;
            w->model->rot[1] = -0.226892814f;
            op3EbiStep = 0;
            op3EbiWalkSnd = 0;
            PRINTF("エビス丸作りました\n");
            break;
        case 1:
            tskDestroy(w->pl);
            frmDestroy((Frame *) w->model);
            frmDestroy((Frame *) w->shadow);
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
            if (opdemo3Frame == 0x1096) {
                return -1;
            }
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo3Frame == 0x1FC) {
                        w->model->pos[0] = 175.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -50.0f;
                        w->model->rot[1] = 0.9250245094f;
                        w->state++;
                    }
                    if (opdemo3Frame >= 0x12D) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], op3BisuX,
                                                          op3BisuZ, w->model->rot[1], 1.0f);
                    }
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo3Frame == 0x260) {
                        w->model->pos[0] = 151.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -39.0f;
                        w->model->rot[1] = 1.413716674f;
                        w->state++;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo3Frame == 0x76C) {
                        w->model->pos[0] = 47.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -57.0f;
                        w->model->rot[1] = 1.39626348f;
                        plAnimeSet(PL(w->pl), 0xD7);
                        w->state++;
                    }
                    break;
                case 3:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo3Frame == 0x7EF) {
                        w->state++;
                    }
                    break;
                case 4:
                    op3EbiStep = step = demoPlTunagiAnime(obj, 2, op3EbiStep, 2, 2, 0.4f, 0.2f,
                                                                   0xEB, 0xEC);
                    if (step == -1) {
                        w->state++;
                        op3EbiStep = 0;
                        scRunStart(0x189, 0, 0);
                        sndPlay(0x330);
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.5f, 0);
                    if (opdemo3Frame == 0x8C3) {
                        w->state++;
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 6:
                    op3EbiStep = step = demoPlTunagiAnime(obj, 0, op3EbiStep, 2, 3, 0.4f, 0.2f,
                                                                   0xEB, 0xE9);
                    if (step == -1) {
                        w->state++;
                        op3EbiStep = 0;
                    }
                    break;
                case 7:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo3Frame == 0xB7F) {
                        w->state++;
                    }
                    break;
                case 8:
                    demoAnimeStep(w->model, 0.2f, 0);
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], op3JijiX,
                                                      op3JijiZ, w->model->rot[1], 2.0f);
                    if (opdemo3Frame == 0xE0C) {
                        w->state++;
                    }
                    break;
                case 9:
                    demoAnimeStep(w->model, 0.2f, 0);
                    w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], op3GoeX,
                                                      op3GoeZ, w->model->rot[1], 0.5f);
                    if (opdemo3Frame >= 0xF75) {
                        w->model->rot[1] = demoTurnToward(w->model->pos[0], w->model->pos[2], op3GoeX,
                                                          op3GoeZ, w->model->rot[1], 2.0f);
                    }
                    if (opdemo3Frame == 0xF92) {
                        scRunEnd();
                        scRunStart(0x191, 0, 0);
                        sndPlay(0x331);
                    }
                    if (opdemo3Frame == 0xFBA) {
                        w->state++;
                        plAnimeSet(PL(w->pl), 0xDC);
                        demoTexAnimK1(&w->eyes, 2, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 2, &PL(w->pl)->model->segs[4]);
                    }
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
                case 10:
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    demoWalkTo(w->model, 0.6f, 1.5f, -8.0f, -340.0f);
                    if (w->model->frame > 7.0f && op3EbiWalkSnd == 0) {
                        SoundLocation2(0x40C, opDemoView, w->model->pos[0], w->model->pos[1],
                                       w->model->pos[2], 30.0f, 6000.0f, opDemoView->fovy);
                        op3EbiWalkSnd = 1;
                    }
                    if (w->model->frame < 1.0 && op3EbiWalkSnd == 1) {
                        SoundLocation2(0x40C, opDemoView, w->model->pos[0], w->model->pos[1],
                                       w->model->pos[2], 30.0f, 6000.0f, opDemoView->fovy);
                        op3EbiWalkSnd = 0;
                    }
                    if (opdemo3Frame == 0x105A) {
                        scRunEnd();
                    }
                    break;
            }
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = w->model->pos[1] + 0.3;
            op3EbiX = w->model->pos[0];
            op3EbiZ = w->model->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 op3EbiProc(Obj *obj, s16 msg, void *arg);

Obj *op3EbiCreate(Obj *obj) {
    return tskCreate(obj, op3EbiProc, obj->file, NULL);
}

/* "jiji"'s work: the old man's model and his shadow. */
typedef struct Tlb21JijiW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ PolFrame *shadow;
    /* 0x08 */ s32 timer;
    /* 0x0C */ s32 state;
} Tlb21JijiW;

extern u8 op3JijiAnim[]; /* the old man's animations */
extern u8 op3JijiTunagiAnim[];
extern u8 op3JijiState8Anim[];
extern u8 op3JijiTunagi2Anim[];
extern u8 op3JijiState1Anim[];
extern u8 op3JijiState5Anim[];
/* "jiji" (the wise old man, "物知りじいさん作ります") at (7, 0, -12) with his shadow; OBJMOVE's
 * model 2 moves him. His animations, turns (toward Bismaru, then between two others) and the
 * scenes 0x17E-0x18D follow the demo's frame count. Publishes his x and z each frame.
 * His alpha and step are function statics (IDO_NOTES 343); state 6's demoAnimeSet takes an int
 * 0. "作りました" is printed last in create (as Ebisumaru's), and create's two zeros (pos[1],
 * rot[1]) are the int 0: a constant of its own (IDO_NOTES 365), so states 4 and 5 keep their
 * 0.0f in f0. */
s32 op3JijiProc(Obj *obj, s16 msg, void *arg) {
    static s32 op3JijiShadowAlpha = 0x80; /* his shadow's alpha */
    static s32 op3JijiStep = 1;    /* his tunagi_anime step */
    Tlb21JijiW *w = (Tlb21JijiW *) (obj + 1);
    s32 step;
    s32 pad;

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
            demoAnimeSet(w->frame, (u32) op3JijiState1Anim, 0xC5, 0);
            demoPolSetDL(w->shadow, (Gfx *) op3ShadowDl, 0xBC);
            w->shadow->scale[2] = 0.12f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            op3JijiStep = 0;
            w->frame->scale[2] = 0.28f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = 7.0f;
            w->frame->pos[1] = 0;
            w->frame->pos[2] = -12.0f;
            w->frame->rot[1] = 0;
            PRINTF("物知りじいさん作りました\n");
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            frmDestroy((Frame *) w->shadow);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, op3JijiShadowAlpha);
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
            if (opdemo3Frame == 0x1096) {
                return -1;
            }
            w->timer++;
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->frame, 0.1f, 0);
                    if (opdemo3Frame == 100) {
                        op3JijiStep = 0;
                        scRunEnd();
                        scRunStart(0x17E, 0, 0);
                        sndPlay(0x341);
                        w->state = 2;
                        op3JijiStep = 0;
                    }
                    break;
                case 1:
                    op3JijiStep = step = tunagi_anime(w->frame, op3JijiStep, 2, 2, 0.4f, 0.4f,
                                                              0xC5, (u32) op3JijiTunagi2Anim, 0xC5, (u32) op3JijiState1Anim);
                    if (step == -1) {
                        scRunEnd();
                        scRunStart(0x17E, 0, 0);
                        sndPlay(0x341);
                        w->state++;
                        op3JijiStep = 0;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->frame, 0.6f, 0);
                    if (opdemo3Frame == 0x1F4) {
                        scRunEnd();
                    }
                    if (opdemo3Frame >= 0x12D) {
                        w->frame->rot[1] = demoTurnToward(w->frame->pos[0], w->frame->pos[2], op3BisuX,
                                                          op3BisuZ, w->frame->rot[1], 1.0f);
                        w->state++;
                    }
                    break;
                case 3:
                    op3JijiStep = step = tunagi_anime(w->frame, op3JijiStep, 2, 3, 0.4f, 0.1f,
                                                              0xC5, (u32) op3JijiTunagi2Anim, 0xC5, (u32) op3JijiAnim);
                    if (step == -1) {
                        w->state++;
                        op3JijiStep = 0;
                    }
                    w->frame->rot[1] = demoTurnToward(w->frame->pos[0], w->frame->pos[2], op3BisuX,
                                                      op3BisuZ, w->frame->rot[1], 1.0f);
                case 4:
                    w->frame->rot[1] = demoTurnToward(w->frame->pos[0], w->frame->pos[2], op3BisuX,
                                                      op3BisuZ, w->frame->rot[1], 1.0f);
                    if (opdemo3Frame >= 0x1FD) {
                        w->frame->pos[0] = 86.0f;
                        w->frame->pos[1] = 0.0f;
                        w->frame->pos[2] = 51.0f;
                        w->frame->rot[1] = 1.954768777f;
                        demoAnimeSet(w->frame, (u32) op3JijiAnim, 0xC5, 0.0f);
                        w->state++;
                    }
                    break;
                case 5:
                    demoAnimeStep(w->frame, 0.1f, 0);
                    if (opdemo3Frame >= 0x76D) {
                        w->frame->pos[0] = 15.0f;
                        w->frame->pos[1] = 0.0f;
                        w->frame->pos[2] = 77.0f;
                        w->frame->rot[1] = 1.884955645f;
                        demoAnimeSet(w->frame, (u32) op3JijiState5Anim, 0xC5, 0.0f);
                        w->state++;
                    }
                    break;
                case 6:
                    demoAnimeStep(w->frame, 0.1f, 0);
                    if (opdemo3Frame == 0xB7F) {
                        demoAnimeSet(w->frame, (u32) op3JijiAnim, 0xC5, 0);
                        w->frame->pos[0] = 16.0f;
                        w->frame->pos[1] = 0.0f;
                        w->frame->pos[2] = 50.0f;
                        w->frame->rot[1] = 1.850049019f;
                        w->state++;
                    }
                    break;
                case 7:
                    demoAnimeStep(w->frame, 0.1f, 0);
                    w->frame->rot[1] = demoTurnToward(w->frame->pos[0], w->frame->pos[2],
                                                      (op3EbiX + op3GoeX) / 2,
                                                      (op3EbiZ + op3GoeZ) / 2,
                                                      w->frame->rot[1], 1.0f);
                    if (opdemo3Frame == 0xBC5) {
                        w->state++;
                        op3JijiStep = 0;
                    }
                    break;
                case 8:
                    w->frame->rot[1] = demoTurnToward(w->frame->pos[0], w->frame->pos[2],
                                                      (op3EbiX + op3GoeX) / 2,
                                                      (op3EbiZ + op3GoeZ) / 2,
                                                      w->frame->rot[1], 1.0f);
                    op3JijiStep = step = tunagi_anime(w->frame, op3JijiStep, 2, 2, 0.4f, 0.4f,
                                                              0xC5, (u32) op3JijiTunagiAnim, 0xC5, (u32) op3JijiState8Anim);
                    if (step == -1) {
                        scRunEnd();
                        scRunStart(0x18B, 0, 0);
                        sndPlay(0x342);
                        w->state++;
                        op3JijiStep = 0;
                    }
                    break;
                case 9:
                    demoAnimeStep(w->frame, 0.3f, 0);
                    if (opdemo3Frame >= 0xC8E) {
                        demoAnimeStep(w->frame, 0.2f, 0);
                    }
                    switch (opdemo3Frame) {
                        case 0xC8D:
                            scRunEnd();
                            scRunStart(0x18C, 0, 0);
                            break;
                        case 0xD37:
                            scRunEnd();
                            scRunStart(0x18D, 0, 0);
                            break;
                        case 0xDFF:
                            scRunEnd();
                            w->state++;
                            break;
                    }
                    break;
                case 10:
                    op3JijiStep = step = tunagi_anime(w->frame, op3JijiStep, 2, 3, 0.4f, 0.4f,
                                                              0xC5, (u32) op3JijiTunagiAnim, 0xC5, (u32) op3JijiAnim);
                    if (step == -1) {
                        w->state++;
                        op3JijiStep = 0;
                    }
                    break;
                case 11:
                    w->frame->rot[1] = demoTurnToward(w->frame->pos[0], w->frame->pos[2], op3GoeX,
                                                      op3GoeZ, w->frame->rot[1], 1.0f);
                    demoAnimeStep(w->frame, 0.2f, 0);
                    break;
            }
            op3JijiX = w->frame->pos[0];
            op3JijiZ = w->frame->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *op3JijiCreate(Obj *obj) {
    return tskCreate(obj, op3JijiProc, obj->file, NULL);
}

/* "bisu"'s work: Bismaru's model. */
typedef struct Tlb21BisuW {
    /* 0x00 */ AnimeFrame *model;
    /* 0x04 */ f32 speed;
    /* 0x08 */ s32 timer;
    /* 0x0C */ s32 state;
} Tlb21BisuW;

extern u8 op3BismaruAnim[]; /* Bismaru's animation */
s32 op3BisuUnusedAlpha = 0x80; /* a word nothing reads (an alpha, as jiji's) */
s32 op3BisuUnusedFlag = 1;
/* A display list nothing points at: pipe sync, cycle and texture modes, geometry, combine. */
Gfx op3UnusedDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetColorDither(G_CD_DISABLE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_TEX_EDGE2),
    gsDPSetCombineLERP(ENVIRONMENT, TEXEL0, ENV_ALPHA, TEXEL0, 0, 0, 0, TEXEL0, COMBINED, 0, SHADE, 0, 0, 0, 0, COMBINED),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsSPEndDisplayList(),
};
extern f32 op3BisuElegantY; /* Bismaru Elegant's y */

/* "bisu" ("ビスマル", Bismaru) at (302, 125, 5); OBJMOVE's model 3 moves him. His animation and
 * the scenes 0x17F-0x18A follow the demo's frame count: at 0x559 he moves to (-517, 1788, -1273),
 * from 0x8EC he rises 1.5 a frame and from 0xA18 rides at Bismaru Elegant's height. */
s32 op3BisuProc(Obj *obj, s16 msg, void *arg) {
    Tlb21BisuW *w = (Tlb21BisuW *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "bisu";
            PRINTF("ビスマル作ります\n");
            w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->model == NULL) {
                return -1;
            }
            demoAnimeSet(w->model, (u32) op3BismaruAnim, 0xC3, 0);
            w->state = 0;
            w->model->scale[2] = 0.28f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 302.0f;
            w->model->pos[1] = 125.0f;
            w->model->pos[2] = 5.0f;
            w->model->rot[1] = -1.466076612f;
            w->speed = 0.0f;
            op3BisuUnusedFlag = 0;
            PRINTF("ビスマル作りました\n");
            break;
        case 1:
            frmDestroy((Frame *) w->model);
            break;
        case 3:
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
            if (opdemo3Frame == 0x1096) {
                return -1;
            }
            w->timer++;
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (opdemo3Frame == 0x12C) {
                        scRunEnd();
                        scRunStart(0x17F, 0, 0);
                        sndPlay(0x326);
                        w->state++;
                    }
                    break;
                case 1:
                    switch (opdemo3Frame) {
                        case 0x1F4:
                            scRunEnd();
                            scRunStart(0x180, 0, 0);
                            break;
                        case 0x329:
                            scRunEnd();
                            sndPlay(0x327);
                            scRunStart(0x182, 0, 0);
                            break;
                        case 0x3DD:
                            scRunEnd();
                            scRunStart(0x183, 0, 0);
                            break;
                        case 0x487:
                            scRunEnd();
                            scRunStart(0x184, 0, 0);
                            break;
                        case 0x545:
                            scRunEnd();
                            break;
                        case 0x559:
                            w->state++;
                            w->model->pos[0] = -517.0f;
                            w->model->pos[1] = 1788.0f;
                            w->model->pos[2] = -1273.0f;
                            w->model->rot[1] = -2.426007748f;
                            break;
                    }
                    if (opdemo3Frame < 0x1F4) {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    demoAnimeStep(w->model, 0.2f, 0);
                    break;
                case 2:
                    demoAnimeStep(w->model, 0.2f, 0);
                    switch (opdemo3Frame) {
                        case 0x577:
                            scRunEnd();
                            scRunStart(0x185, 0, 0);
                            break;
                        case 0x60D:
                            scRunEnd();
                            scRunStart(0x186, 0, 0);
                            break;
                        case 0x6A5:
                            scRunEnd();
                            scRunStart(0x187, 0, 0);
                            break;
                        case 0x76B:
                            scRunEnd();
                            w->state++;
                            break;
                    }
                    break;
                case 3:
                    if (opdemo3Frame == 0x8EB) {
                        scRunEnd();
                        scRunStart(0x18A, 0, 0);
                        sndPlay(0x328);
                    }
                    if (opdemo3Frame >= 0x8EC) {
                        w->model->pos[1] += 1.5;
                    }
                    if (opdemo3Frame == 0x9B3) {
                        scRunEnd();
                    }
                    break;
            }
            if (opdemo3Frame >= 0xA18) {
                w->model->pos[1] = op3BisuElegantY;
            }
            if (opdemo3Frame == 0xB7F) {
                return -1;
            }
            op3BisuX = w->model->pos[0];
            op3BisuZ = w->model->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Starts a op3BisuProc task under obj. */
Obj *op3BisuCreate(Obj *obj) {
    return tskCreate(obj, op3BisuProc, obj->file, NULL);
}

void op3UnusedStub(void) {
}

/* The second "bisu"'s work: Bismaru Elegant's model, rising. */
typedef struct Tlb21BisuEW {
    /* 0x00 */ AnimeFrame *model;
    /* 0x04 */ f32 rise; /* added to y each frame */
    /* 0x08 */ s32 timer;
    /* 0x0C */ s32 state;
} Tlb21BisuEW;

extern u8 op3BismaruElegantAnim[]; /* Bismaru Elegant's animation */
s32 op3BisuElegantUnusedFlag = 1;
s32 op3BisuElegantTakeoffSndPlayed = 1; /* 1 once his take-off sound has played */
extern f32 op3BisuElegantX; /* his x, z and y (op3BisuElegantY), for the others */
extern f32 op3BisuElegantZ;

/* "bisu" ("ビスマルエレガント", Bismaru Elegant): an anime model at (-603, 2791, -1183) that from
 * demo frame 0xAE0 rises faster and faster (30 a frame past 5, with a sound); OBJMOVE's model 4
 * moves it. Publishes its position each frame and deletes itself at 0xB7F. */
s32 op3BisuElegantProc(Obj *obj, s16 msg, void *arg) {
    Tlb21BisuEW *w = (Tlb21BisuEW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "bisu";
            PRINTF("ビスマルエレガント作ります\n");
            w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->model == NULL) {
                return -1;
            }
            demoAnimeSet(w->model, (u32) op3BismaruElegantAnim, 0xF4, 0);
            w->state = 0;
            w->model->scale[2] = 5.4f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -603.0f;
            w->model->pos[1] = 2791.0f;
            w->model->pos[2] = -1183.0f;
            w->model->rot[1] = -5.323254585f;
            w->rise = 0.0f;
            op3BisuElegantTakeoffSndPlayed = 0;
            op3BisuElegantUnusedFlag = 0;
            PRINTF("ビスマルエレガント作りました\n");
            break;
        case 1:
            frmDestroy((Frame *) w->model);
            break;
        case 3:
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
            if (opdemo3Frame == 0x1096) {
                return -1;
            }
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            w->timer++;
            if (opdemo3Frame >= 0xAE0) {
                w->rise += 0.05;
                if (w->rise > 5.0) {
                    w->rise = 30.0f;
                }
                if (w->rise > 5.0 && op3BisuElegantTakeoffSndPlayed == 0) {
                    sndPlay(0x1BC);
                    op3BisuElegantTakeoffSndPlayed = 1;
                }
            }
            w->model->pos[1] += w->rise;
            op3BisuElegantX = w->model->pos[0];
            op3BisuElegantY = w->model->pos[1];
            op3BisuElegantZ = w->model->pos[2];
            if (opdemo3Frame == 0xB7F) {
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *op3BisuElegantCreate(Obj *obj) {
    return tskCreate(obj, op3BisuElegantProc, obj->file, NULL);
}

/* "MACHINE"'s work: its anime model. */
typedef struct Tlb21MachineW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 speed;
} Tlb21MachineW;

extern u8 op3MachineAnim[];
extern f32 opMachineHeight; /* the machine's height, for the others */

/* "MACHINE": an anime model at (-517, 1626, -1266) turning slowly about y; it rises 1.5 a frame
 * from demo frame 0x8EC, rides at Bismaru Elegant's height from 0xA18 and deletes itself at 0xB7F;
 * OBJMOVE's model 5 moves it. Publishes its height each frame. */
s32 op3MachineProc(Obj *obj, s16 msg, void *arg) {
    Tlb21MachineW *w = (Tlb21MachineW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "MACHINE";
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            demoAnimeSet(w->frame, (u32) op3MachineAnim, 0xC2, 0);
            w->frame->pos[0] = -517.0f;
            w->frame->pos[1] = 1626.0f;
            w->frame->pos[2] = -1266.0f;
            w->frame->scale[0] = 1.6f;
            w->frame->scale[1] = 1.6f;
            w->frame->scale[2] = 1.6f;
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
            if (opdemo3Frame == 0x1096) {
                return -1;
            }
            w->frame->rot[1] += 0.00872664650194003924;
            if (opdemo3Frame >= 0x8EC) {
                w->frame->pos[1] += 1.5;
            }
            if (opdemo3Frame >= 0xA18) {
                w->frame->pos[1] = op3BisuElegantY;
            }
            if (opdemo3Frame == 0xB7F) {
                return -1;
            }
            opMachineHeight = w->frame->pos[1];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *op3MachineCreate(Obj *obj) {
    return tskCreate(obj, op3MachineProc, obj->file, NULL);
}

/* "beam"'s work: two beam columns and two rings. */
typedef struct Tlb21BeamW {
    /* 0x00 */ PolFrame *frames[4]; /* the beams (0, 1) and the rings (2, 3) */
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ f32 scale;
} Tlb21BeamW;

extern Gfx op3Beam1Dl[]; /* the beam's two looks, in file 0x13B */
extern Gfx op3Beam2Dl[];
/* The beams' display lists. */
Gfx op3BeamFrame0Dl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPEndDisplayList(),
};
Gfx op3BeamFrame1Dl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetEnvColor(0xFF, 0xFF, 0xFF, 0xA0),
    gsDPSetCombineLERP(0, 0, 0, SHADE, SHADE, 0, ENVIRONMENT, 0, 0, 0, 0, SHADE, SHADE, 0, ENVIRONMENT, 0),
    gsSPEndDisplayList(),
};
extern s32 op3BeamSndPlayed; /* 1 once the beam's sound has played */
extern s32 op3BeamSndPlayedClear; /* the same flag, as case 2 clears it and case 0 sets it */
extern s32 op3BeamSndPlayedSet;

/* "beam": a beam of light over the machine; until demo frame 0x559 only the first column shows,
 * turning (everything shows before 0x14A); then all four at (-517, 2100, -1273) with pulsing
 * rings; once the machine rises past 2100 it plays a sound and narrows, and deletes itself when
 * thin. The sound flag is written under two second names (undefined_syms.txt): the game forms
 * its address at each use, where one name stays in v1 (IDO_NOTES 298). */
s32 op3BeamProc(Obj *obj, s16 msg, void *arg) {
    Tlb21BeamW *w = (Tlb21BeamW *) (obj + 1);
    s32 pad[2];

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
            w->frames[0]->dl2 = op3Beam1Dl;
            floadGetSetRam((void **) w->frames[0]->segs, 0x13B);
            w->frames[1]->dl2 = op3Beam2Dl;
            floadGetSetRam((void **) w->frames[0]->segs, 0x13B);
            demoPolSetDL(w->frames[2], (Gfx *) op3SparkleDl, 0x120);
            demoPolSetDL(w->frames[3], (Gfx *) op3SparkleDl, 0x120);
            w->frames[0]->scale[2] = 0.1f;
            w->frames[0]->scale[0] = w->frames[0]->scale[2];
            w->frames[1]->scale[2] = 0.1f;
            w->frames[1]->scale[0] = w->frames[1]->scale[2];
            w->frames[0]->scale[1] = 1.0f;
            w->frames[1]->scale[1] = 2.0f;
            w->frames[0]->pos[0] = 302.0f;
            w->frames[0]->pos[1] = 200.0f;
            w->frames[0]->pos[2] = 5.0f;
            w->frames[1]->pos[0] = 302.0f;
            w->frames[1]->pos[1] = 300.0f;
            w->frames[1]->pos[2] = 5.0f;
            w->frames[0]->dl = (Gfx *) fadeSegToRam(op3BeamFrame0Dl, obj->file);
            w->frames[1]->dl = (Gfx *) fadeSegToRam(op3BeamFrame1Dl, obj->file);
            w->scale = 1.0f;
            op3BeamSndPlayedClear = 0;
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
            if (opdemo3Frame == 0x1096) {
                return -1;
            }
            if (opdemo3Frame < 0x559) {
                w->frames[1]->f.flags &= ~2;
                w->frames[2]->f.flags &= ~2;
                w->frames[3]->f.flags &= ~2;
                w->frames[0]->f.flags |= 2;
                w->frames[0]->rot[1] += 0.0174532930038800785;
                w->frames[1]->rot[1] += 0.0174532930038800785;
                if (opdemo3Frame < 0x14A) {
                    w->frames[1]->f.flags |= 2;
                    w->frames[2]->f.flags |= 2;
                    w->frames[3]->f.flags |= 2;
                }
            } else {
                w->frames[1]->f.flags |= 2;
                w->frames[0]->f.flags |= 2;
                w->frames[2]->f.flags |= 2;
                w->frames[3]->f.flags |= 2;
                w->frames[0]->pos[0] = -517.0f;
                w->frames[0]->pos[1] = 2100.0f;
                w->frames[0]->pos[2] = -1273.0f;
                w->frames[1]->pos[0] = -517.0f;
                w->frames[1]->pos[1] = 2100.0f;
                w->frames[1]->pos[2] = -1273.0f;
                w->frames[3]->pos[0] = -517.0f;
                w->frames[2]->pos[0] = w->frames[3]->pos[0];
                w->frames[3]->pos[1] = 2028.0f;
                w->frames[2]->pos[1] = w->frames[3]->pos[1];
                w->frames[3]->pos[2] = -1273.0f;
                w->frames[2]->pos[2] = w->frames[3]->pos[2];
                w->frames[0]->scale[0] = w->frames[0]->scale[2] = w->scale * 0.2f;
                w->frames[1]->scale[0] = w->frames[1]->scale[2] = w->scale * 0.2f;
                w->frames[2]->scale[0] = w->frames[2]->scale[1] =
                    sinf(opdemo3Frame * 10 * 0.0174532930038800785) * 1.8 + w->scale * 10.0f;
                w->frames[3]->scale[0] = w->frames[3]->scale[1] =
                    sinf((opdemo3Frame * 10 + 180) * 0.0174532930038800785) * 1.7 + w->scale * 10.0f;
                w->frames[0]->scale[1] = 4.0f;
                w->frames[1]->scale[1] = 4.0f;
                w->frames[2]->scale[2] = 1.0f;
                w->frames[3]->scale[2] = 1.0f;
                w->frames[0]->rot[1] += 0.0174532930038800785;
                w->frames[1]->rot[1] += 0.0174532930038800785;
                w->frames[2]->rot[0] = 1.570796371f;
                w->frames[3]->rot[0] = 1.570796371f;
                if (opMachineHeight > 2100.0f && op3BeamSndPlayed == 0) {
                    op3BeamSndPlayedSet = 1;
                    sndPlay(0x1BB);
                }
                if (opMachineHeight > 2100.0f) {
                    w->scale *= 0.97;
                }
                if (w->scale < 0.04) {
                    return -1;
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 op3BeamProc(Obj *obj, s16 msg, void *arg);

Obj *op3BeamCreate(Obj *obj) {
    return tskCreate(obj, op3BeamProc, obj->file, NULL);
}

/* The second "beam"'s work: four sparkle frames. */
typedef struct Tlb21Beam2W {
    /* 0x00 */ PolFrame *frames[4]; /* two pairs, at the two ends of the machine */
    /* 0x10 */ s32 alpha;
} Tlb21Beam2W;

/* "beam": four sparkles that pulse at two points by the machine; from demo frame 0xA17 (a sound)
 * they brighten to 255, and at 0xB7F the task deletes itself. */
s32 op3BeamSparkleProc(Obj *obj, s16 msg, void *arg) {
    Tlb21Beam2W *w = (Tlb21Beam2W *) (obj + 1);
    s32 pad[2];

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
            demoPolSetDL(w->frames[0], (Gfx *) op3SparkleDl, 0x120);
            demoPolSetDL(w->frames[1], (Gfx *) op3SparkleDl, 0x120);
            demoPolSetDL(w->frames[2], (Gfx *) op3SparkleDl, 0x120);
            demoPolSetDL(w->frames[3], (Gfx *) op3SparkleDl, 0x120);
            w->alpha = 0x80;
            break;
        case 3:
            w->frames[0]->dl = demoColorDL(255, 255, 255, 0, 200, 80, w->alpha);
            w->frames[1]->dl = demoColorDL(255, 255, 255, 0, 60, 100, w->alpha);
            w->frames[2]->dl = demoColorDL(255, 255, 255, 0, 200, 80, w->alpha);
            w->frames[3]->dl = demoColorDL(255, 255, 255, 0, 60, 100, w->alpha);
            break;
        case 1:
            frmDestroy((Frame *) w->frames[0]);
            frmDestroy((Frame *) w->frames[1]);
            frmDestroy((Frame *) w->frames[2]);
            frmDestroy((Frame *) w->frames[3]);
            break;
        case 0:
            if (gDemoModelsDelete2) {
                return -1;
            }
            if (opdemo3Frame == 0x1096) {
                return -1;
            }
            w->frames[0]->pos[0] = w->frames[1]->pos[0] = op3BisuElegantX - 99.0f;
            w->frames[0]->pos[1] = w->frames[1]->pos[1] = op3BisuElegantY - 763.0f;
            w->frames[0]->pos[2] = w->frames[1]->pos[2] = op3BisuElegantZ - 62.0f;
            w->frames[2]->pos[0] = w->frames[3]->pos[0] = op3BisuElegantX + 106.0f;
            w->frames[2]->pos[1] = w->frames[3]->pos[1] = op3BisuElegantY - 763.0f;
            w->frames[2]->pos[2] = w->frames[3]->pos[2] = op3BisuElegantZ + 42.0f;
            w->frames[0]->scale[0] = w->frames[0]->scale[1] = w->frames[2]->scale[0] = w->frames[2]->scale[1] =
                sinf(opdemo3Frame * 30 * 0.0174532930038800785) * 1.8 + 10.0;
            w->frames[1]->scale[0] = w->frames[1]->scale[1] = w->frames[3]->scale[0] = w->frames[3]->scale[1] =
                sinf((opdemo3Frame * 30 + 180) * 0.0174532930038800785) * 1.7 + 10.0;
            w->frames[3]->rot[0] = 1.570796371f;
            w->frames[0]->rot[0] = w->frames[1]->rot[0] = w->frames[2]->rot[0] = w->frames[3]->rot[0];
            if (opdemo3Frame == 0xA17) {
                sndPlay(0x256);
            }
            if (opdemo3Frame >= 0xA18) {
                w->alpha += 2;
                if (w->alpha >= 0x100) {
                    w->alpha = 0xFF;
                }
            }
            if (opdemo3Frame == 0xB7F) {
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *op3BeamSparkleCreate(Obj *obj) {
    return tskCreate(obj, op3BeamSparkleProc, obj->file, NULL);
}

/* "BARA"'s work: twelve sparkle frames. */
typedef struct Tlb21BaraW {
    /* 0x00 */ PolFrame *frames[12];
} Tlb21BaraW;

extern u8 op3BaraDl[];

Obj *op3KiranCreate(Obj *obj);
Obj *op3KiranDriftCreate(Obj *obj);
Obj *op3KiranDriftLowCreate(Obj *obj);

/* "BARA" (roses): twelve sparkles around (279, 157-167, -6 to 16), turning about z; it starts
 * "kiran" sparkles every 5, 10 and 6 demo frames (and twenty at once when made), and deletes
 * itself at demo frame 0x1FD. */
s32 op3BaraProc(Obj *obj, s16 msg, void *arg) {
    s32 i;
    Tlb21BaraW *w = (Tlb21BaraW *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "BARA";
            for (i = 0; i < 12; i++) {
                w->frames[i] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
                if (w->frames[i] == NULL) {
                    return -1;
                }
                demoPolSetDL(w->frames[i], (Gfx *) op3BaraDl, 0xC3);
                w->frames[i]->flags |= 2;
                if (i < 4) {
                    w->frames[i]->scale[2] = 0.25f;
                    w->frames[i]->scale[0] = w->frames[i]->scale[1] = w->frames[i]->scale[2];
                } else {
                    w->frames[i]->scale[2] = 0.14f;
                    w->frames[i]->scale[0] = w->frames[i]->scale[1] = w->frames[i]->scale[2];
                }
                w->frames[i]->pos[0] = 279.0f;
            }
            w->frames[0]->pos[1] = 157.0f;
            w->frames[0]->pos[2] = -6.0f;
            w->frames[1]->pos[1] = 157.0f;
            w->frames[1]->pos[2] = 16.0f;
            w->frames[2]->pos[1] = 167.0f;
            w->frames[2]->pos[2] = -6.0f;
            w->frames[3]->pos[1] = 167.0f;
            w->frames[3]->pos[2] = 16.0f;
            w->frames[4]->pos[1] = 167.0f;
            w->frames[4]->pos[2] = -2.0f;
            w->frames[5]->pos[1] = 157.0f;
            w->frames[5]->pos[2] = -2.0f;
            w->frames[6]->pos[1] = 167.0f;
            w->frames[6]->pos[2] = 11.0f;
            w->frames[7]->pos[1] = 157.0f;
            w->frames[7]->pos[2] = 11.0f;
            w->frames[8]->pos[1] = 161.0f;
            w->frames[8]->pos[2] = -6.0f;
            w->frames[9]->pos[1] = 163.0f;
            w->frames[9]->pos[2] = -6.0f;
            w->frames[10]->pos[1] = 161.0f;
            w->frames[10]->pos[2] = 16.0f;
            w->frames[11]->pos[1] = 163.0f;
            w->frames[11]->pos[2] = 16.0f;
            for (i = 0; i < 10; i++) {
                (*op3KiranDriftCreate)(obj);
                (*op3KiranDriftLowCreate)(obj);
            }
            break;
        case 1:
            for (i = 0; i < 12; i++) {
                frmDestroy((Frame *) w->frames[i]);
            }
            break;
        case 3:
            for (i = 0; i < 12; i++) {
                w->frames[i]->dl = gGfxDlHead;
                gDPPipeSync(gGfxDlHead++);
                gDPSetPrimColor(gGfxDlHead++, 0, 0, 0, 0, 0, 0);
                gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 255);
                gSPEndDisplayList(gGfxDlHead++);
            }
            break;
        case 0:
            if (opdemo3Frame == 0x1096) {
                return -1;
            }
            if (gDemoModelsDelete2) {
                return -1;
            }
            if (opdemo3Frame % 5 == 0) {
                (*op3KiranCreate)(obj);
            }
            if (opdemo3Frame % 10 == 0) {
                (*op3KiranDriftCreate)(obj);
            }
            if (opdemo3Frame % 6 == 0) {
                (*op3KiranDriftLowCreate)(obj);
            }
            if (opdemo3Frame >= 0x1FD) {
                return -1;
            }
            for (i = 0; i < 12; i++) {
                w->frames[i]->rot[2] += 0.0174532930038800785;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *op3BaraCreate(Obj *obj) {
    return tskCreate(obj, op3BaraProc, obj->file, NULL);
}

/* "FBALL"'s work: two sparkle frames. */
typedef struct Tlb21FballW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ PolFrame *frame2;
    /* 0x08 */ s32 alpha;
    /* 0x0C */ s32 unused0C; /* written 0 at its start, never read */
} Tlb21FballW;

/* "FBALL" (a fireball): two sparkles at (278, 156), drifting in z, fading from demo frame 0x1FD;
 * OBJMOVE's model 6 moves it. */
s32 op3FballProc(Obj *obj, s16 msg, void *arg) {
    Tlb21FballW *w = (Tlb21FballW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "FBALL";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, (Gfx *) op3SparkleDl, 0x120);
            w->frame->flags |= 7;
            w->frame2 = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame2 == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame2, (Gfx *) op3SparkleDl, 0x120);
            w->frame2->flags |= 7;
            w->frame->pos[0] = 278.0f;
            w->frame->pos[1] = 156.0f;
            w->frame->pos[2] = 15.0f;
            w->frame2->pos[0] = 278.0f;
            w->frame2->pos[1] = 156.0f;
            w->frame2->pos[2] = 2.0f;
            w->frame->scale[2] = 1.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame2->scale[2] = 0.7f;
            w->frame2->scale[0] = w->frame2->scale[1] = w->frame2->scale[2];
            w->alpha = 0x80;
            w->unused0C = 0;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            frmDestroy((Frame *) w->frame2);
            break;
        case 3:
            w->frame->dl = demoColorDL(255, 255, 255, 255, 255, 255, w->alpha);
            w->frame2->dl = demoColorDL(255, 255, 255, 255, 255, 255, w->alpha);
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
            if (opdemo3Frame == 0x1096) {
                return -1;
            }
            if (gDemoModelsDelete2) {
                return -1;
            }
            w->frame->pos[2] += 0.2;
            w->frame2->pos[2] += 0.1;
            if (opdemo3Frame >= 0x1FD) {
                return -1;
            }
            if (opdemo3Frame >= 0x1FD) {
                w->alpha -= 2;
            }
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *op3FballCreate(Obj *obj) {
    return tskCreate(obj, op3FballProc, obj->file, NULL);
}


/* The two "kiran" (sparkle) tasks' argument: only the position their starters set. */
typedef struct Tlb21KiranArg {
    /* 0x00 */ u8 pad00[0x8];
    /* 0x08 */ f32 pos[3];
    /* 0x14 */ u8 pad14[0x8];
} Tlb21KiranArg;

extern u8 op3KiranDl[]; /* the sparkle's display list (file set 0xC6: op3ShadowDl's address in another file) */
extern u8 op3KiranGlowDl[]; /* the sparkle's glow's display list */

/* The first "kiran"'s work: a sparkle and its glow. */
typedef struct Tlb21Kiran1W {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ PolFrame *frame2; /* drawn at a sixth of the alpha, a quarter of the width */
    /* 0x08 */ u8 pad08[0xC];
    /* 0x14 */ s32 alpha;
    /* 0x18 */ s32 state; /* 0 brightens fast, 1 slowly, 2 fades */
} Tlb21Kiran1W;

/* "kiran": a sparkle at the argument's position that stretches in x as it brightens to 255, then
 * fades, shrinking, and deletes itself. */
s32 op3KiranProc(Obj *obj, s16 msg, void *arg) {
    Tlb21Kiran1W *w = (Tlb21Kiran1W *) (obj + 1);
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
            demoPolSetDL(w->frame, (Gfx *) op3KiranDl, 0xC6);
            demoPolSetDL(w->frame2, (Gfx *) op3KiranGlowDl, 0xC6);
            w->frame->flags |= 7;
            w->frame2->flags |= 7;
            w->frame->scale[2] = 0.04f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame2->scale[2] = 0.04f;
            w->frame2->scale[0] = w->frame2->scale[1] = w->frame2->scale[2];
            w->frame->pos[0] = ((Tlb21KiranArg *) arg)->pos[0];
            w->frame->pos[1] = ((Tlb21KiranArg *) arg)->pos[1];
            w->frame->pos[2] = ((Tlb21KiranArg *) arg)->pos[2];
            w->frame2->pos[0] = ((Tlb21KiranArg *) arg)->pos[0];
            w->frame2->pos[1] = ((Tlb21KiranArg *) arg)->pos[1];
            w->frame2->pos[2] = ((Tlb21KiranArg *) arg)->pos[2];
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
                    w->alpha += 32;
                    if (w->alpha > 224) {
                        w->state = 1;
                        w->alpha = 224;
                    } else {
                        w->frame->scale[0] += 0.2;
                        w->frame->scale[1] = w->frame->scale[0];
                        w->frame2->scale[0] = w->frame->scale[0] / 4;
                        w->frame2->scale[1] = w->frame2->scale[0];
                    }
                    break;
                case 1:
                    w->alpha += 2;
                    if (w->alpha >= 0x100) {
                        w->alpha = 0xFF;
                        w->state = 2;
                    } else {
                        w->frame->scale[0] += 0.02;
                        w->frame->scale[1] = w->frame->scale[0];
                        w->frame2->scale[0] = w->frame->scale[0] / 4;
                        w->frame2->scale[1] = w->frame2->scale[0];
                    }
                    break;
                case 2:
                    w->alpha -= 40;
                    if (w->alpha <= 0) {
                        w->alpha = 0;
                        return -1;
                    }
                    if (w->frame->scale[0] > 0.01) {
                        w->frame->scale[0] -= 0.02;
                        w->frame->scale[1] = w->frame->scale[0];
                        w->frame2->scale[0] = w->frame->scale[0] / 4;
                        w->frame2->scale[1] = w->frame2->scale[0];
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "kiran" (op3KiranProc) at x 280, a random height (157.0 to 167.0) and depth
 * (-6.0 to 16.0) under obj. */
Obj *op3KiranCreate(Obj *obj) {
    Tlb21KiranArg arg;

    arg.pos[0] = 280.0f;
    arg.pos[1] = fadeRandRange(1570, 1670) / 10.0f;
    arg.pos[2] = fadeRandRange(-60, 160) / 10.0f;
    return tskCreate(obj, op3KiranProc, obj->file, &arg);
}

/* The second "kiran"'s work. */
typedef struct Tlb21Kiran2W {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[0x10];
    /* 0x14 */ s32 alpha;
    /* 0x18 */ s32 state; /* 0 brightens, 1 fades */
} Tlb21Kiran2W;

/* "kiran": a sparkle drifting in z that brightens to 128, then fades and deletes itself. */
s32 op3KiranDriftProc(Obj *obj, s16 msg, void *arg) {
    Tlb21Kiran2W *w = (Tlb21Kiran2W *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "kiran";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, (Gfx *) op3SparkleDl, 0x120);
            w->frame->flags |= 7;
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = ((Tlb21KiranArg *) arg)->pos[0];
            w->frame->pos[1] = ((Tlb21KiranArg *) arg)->pos[1];
            w->frame->pos[2] = ((Tlb21KiranArg *) arg)->pos[2];
            w->alpha = 0x40;
            w->state = 0;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 3:
            w->frame->dl = demoColorDL(255, 255, 255, 222, 100, 100, w->alpha);
            break;
        case 0:
            w->frame->pos[2] += 0.04;
            switch (w->state) {
                case 0:
                    w->alpha += 12;
                    if (w->alpha > 128) {
                        w->state = 1;
                        w->alpha = 128;
                    }
                    break;
                case 1:
                    w->alpha -= 6;
                    if (w->alpha <= 40) {
                        w->alpha = 0;
                        return -1;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The other "kiran" (op3KiranDriftProc), heights 157.0 to 160.0. */
Obj *op3KiranDriftCreate(Obj *obj) {
    Tlb21KiranArg arg;

    arg.pos[0] = 280.0f;
    arg.pos[1] = fadeRandRange(1570, 1600) / 10.0f;
    arg.pos[2] = fadeRandRange(-60, 160) / 10.0f;
    return tskCreate(obj, op3KiranDriftProc, obj->file, &arg);
}

/* The other "kiran", heights 157.0 to 159.0. */
Obj *op3KiranDriftLowCreate(Obj *obj) {
    Tlb21KiranArg arg;

    arg.pos[0] = 280.0f;
    arg.pos[1] = fadeRandRange(1570, 1590) / 10.0f;
    arg.pos[2] = fadeRandRange(-60, 160) / 10.0f;
    return tskCreate(obj, op3KiranDriftProc, obj->file, &arg);
}
