#include "tlb.h"
#include "tlb/tlb_0030.h"

#define DPRINTF

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
extern Gfx st4clearShadowDl[]; /* a shadow's display list */

/* The demo player's work ("chaSAS", "chaYAE"): the player task, its model, its shadow, the
 * face's two texture animations, a frame count and a state. */
typedef struct DemoPlW {
    /* 0x00 */ Obj *pl;
    /* 0x04 */ AnimeFrame *model;
    /* 0x08 */ PolFrame *shadow;
    /* 0x0C */ Anim eyes;
    /* 0x1C */ Anim mouth;
    /* 0x2C */ s32 chara;
    /* 0x30 */ s32 timer;
    /* 0x34 */ s32 state;
} DemoPlW;

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
/* The file's .data (0x08005530-0x08005FBF), defined here in address order: the characters'
 * texture animations (demotool.c's table, as tlb_0020_1's; nothing reads it), the positions; the
 * four players' statics follow in their procs, and the Suzaku's texture animation after them. */
AnimFrame st4clearCharaAnims[] = {
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
f32 st4clearSasX = 1.0f; /* Sasuke's x and z, each frame */
f32 st4clearSasZ = 1.0f;
f32 st4clearGoeX = 1.0f; /* Goemon's x and z, each frame */
f32 st4clearGoeZ = 1.0f;
f32 st4clearEbiX = 1.0f; /* Ebisumaru's x and z, each frame */
f32 st4clearEbiZ = 1.0f;
s32 st4clearGourdPoseDone = 1; /* 0 while Goemon holds the gourd's pose, then 1 (hyotan reads it) */
f32 st4clearYaeX = 1.0f; /* Yae's x and z, each frame */
f32 st4clearYaeZ = 1.0f;
f32 st4clearSuzakuX = 1.0f; /* Suzaku's x and z, each frame */
f32 st4clearSuzakuZ = 1.0f;
f32 st4clearUnused3[2] = { 1.0f, 1.0f }; /* unused */

/* "genoGOE": Goemon (character 0) in the demo at (0, 298.45, 0) under the menu's last task;
 * OBJMOVE's model 0 moves him. At frame 0xA he falls (speed + 0.049 a frame) to y 100 (sound
 * 0x257, 0xE0), lands (0xD7), turns, links his animations with demoPlTunagiAnime (scene runs
 * 0x123 to 0x13C) and plays the gourd scene with hyotan through st4clearGourdPoseDone.
 * Matched 2026-10-09; it was parked only until the file's .data (its three statics) was C. */
s32 st4clearGoeProc(Obj *obj, s16 msg, void *arg);
s32 st4clearGoeProc(Obj *obj, s16 msg, void *arg) {
    static s32 step = -1;    /* 0x08005F7C: his demoPlTunagiAnime step */
    static f32 speed = 1.0f; /* 0x08005F80: his fall speed */
    static Player *p = (Player *) 0x01234567; /* 0x08005F84: his player */
    s32 pad[2];
    DemoPlW *w = (DemoPlW *) (obj + 1);
    PlInit init;

    DPRINTF("goecreate\n"); /* the game's rodata has it first, before the name */
    switch (msg) {
        case 2:
            obj->name = "genoGOE";
            plMakeInit(&init, 0, 0);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = 0.0f;
            init.height = 298.4500122f;
            init.side = 0.0f;
            speed = 0.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            PRINTF("デモモード突入\n");
            PRINTF("落下モーション\n");
            plAnimeSet(PL(w->pl), 0xDE);
            PRINTF("ani %x\n", 0xDE);
            w->model = p->model;
            PRINTF("*** p->model_buff=%x\n", p->model);
            PRINTF("ゴエモン作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, st4clearShadowDl, 0xBC);
            w->shadow->scale[2] = 0.07999999821f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->chara = 0;
            w->model->scale[2] = 0.09700000286f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 0.0f;
            w->model->pos[1] = 100.0f;
            w->model->pos[2] = 0.0f;
            w->model->rot[1] = -0.7853981853f;
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
            w->timer++;
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
            st4clearGoeX = w->model->pos[0];
            st4clearGoeZ = w->model->pos[2];
            switch (w->state) {
                case 0:
                    w->model->scale[2] = 0.09700000286f;
                    w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                    w->model->pos[0] = 0.0f;
                    w->model->pos[1] = 298.4500122f;
                    w->model->pos[2] = 0.0f;
                    w->model->rot[1] = -0.7853981853f;
                    step = 0;
                    speed = 0.0f;
                    if (w->timer >= 0xA) {
                        w->state++;
                    }
                    break;
                case 1:
                    demoAnimeStep(w->model, 1.0f, 2);
                    speed += 0.049;
                    w->model->pos[1] -= speed;
                    if (w->model->pos[1] <= 100.0) {
                        w->model->pos[1] = 100.0f;
                        sndPlay(0x257);
                        plAnimeSet(PL(w->pl), 0xE0);
                        w->state++;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->model, 0.25f, 2);
                    if (polAnimeLastFrame(w->model) == (s32) w->model->frame) {
                        plAnimeSet(PL(w->pl), 0xD7);
                        w->state++;
                    }
                    break;
                case 3:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (w->timer >= 0x1C2 && w->timer < 0x1E0) {
                        w->model->rot[1] -= 0.0261799395058201177;
                    }
                    if (w->timer == 0x1E0) {
                        w->model->pos[0] = -6.0f;
                        w->model->pos[1] = 100.0f;
                        w->model->pos[2] = 0.0f;
                        w->model->rot[1] = 4.729842186f;
                        w->state++;
                    }
                    break;
                case 4:
                    if (step != -1) {
                        step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.4f, 0xEB, 0xEC);
                        if (step == -1) {
                            scRunStart(0x123, 0, 0);
                        }
                    } else {
                        demoAnimeStep(w->model, 0.4f, 0);
                    }
                    if (w->timer == 0x370) {
                        scRunEnd();
                        w->state++;
                        step = 0;
                    }
                    break;
                case 5:
                    if (step != -1) {
                        step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.4f, 0.2f, 0xEB, 0xE9);
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    if (w->timer >= 0x62C) {
                        plAnimeSet(PL(w->pl), 0x108);
                        demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                        scRunEnd();
                        scRunStart(0x127, 0, 0);
                        w->state++;
                    }
                    break;
                case 6:
                    demoAnimeStep(w->model, 0.1f, 2);
                    if (w->timer == 0x64A) {
                        scRunEnd();
                    }
                    if (w->timer >= 0x6AE) {
                        scRunStart(0x128, 0, 0);
                        w->state++;
                    }
                    break;
                case 7:
                    if (w->timer >= 0x9A6) {
                        plAnimeSet(PL(w->pl), 0xD7);
                        scRunEnd();
                        scRunStart(0x12A, 0, 0);
                        w->state++;
                    }
                    break;
                case 8:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (w->timer >= 0xA6E) {
                        w->model->pos[0] = 0.0f;
                        w->model->rot[1] = 0.226892814f;
                        w->state++;
                    }
                    break;
                case 9:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (w->timer >= 0x2116) {
                        step = 0;
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 10:
                    if (step != -1) {
                        step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.4f, 0xEB, 0xEC);
                        if (step == -1) {
                            scRunStart(0x135, 0, 0);
                        }
                    } else if (w->timer < 0x23D2) {
                        demoAnimeStep(w->model, 0.4f, 0);
                    } else {
                        demoAnimeStep(w->model, 0.5f, 0);
                    }
                    if (w->timer == 0x23D2) {
                        scRunEnd();
                        scRunStart(0x136, 0, 0);
                    } else if (w->timer >= 0x262A) {
                        step = 0;
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 11:
                    if (step != -1) {
                        step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.4f, 0.2f, 0xEB, 0xE9);
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    if (w->timer == 0x27BA) {
                        plAnimeSet(PL(w->pl), 0x109);
                        demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                        w->model->rot[1] = 3.612831593f;
                        w->state++;
                    }
                    /* fallthrough */
                case 12:
                    demoAnimeStep(w->model, 0.4f, 2);
                    if (w->timer >= 0x27CE) {
                        step = 0;
                        scRunEnd();
                        scRunStart(0x138, 0, 0);
                        demoTexAnimK0(&w->eyes, 3, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 3, &PL(w->pl)->model->segs[4]);
                        sndPlay(0x196);
                        w->state++;
                    }
                    break;
                case 13:
                    demoAnimeStep(w->model, 0.4f, 2);
                    if (w->timer < 0x27DD) {
                        fadeAnimStep(&w->eyes);
                    }
                    fadeAnimStep(&w->mouth);
                    if (w->timer >= 0x28B4) {
                        plAnimeSet(PL(w->pl), 0xD7);
                        w->model->rot[1] = 0.226892814f;
                        step = -1;
                        w->state++;
                    }
                    break;
                case 14:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (w->timer == 0x2DC8) {
                        step = 0;
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 15:
                    if (demoAnimeStep(w->model, 0.2f, 2)) {
                        st4clearGourdPoseDone = 0;
                        plAnimeSet(PL(w->pl), 0xEB);
                        w->state++;
                    }
                    break;
                case 16:
                    if (demoAnimeStep(w->model, 0.4f, 2)) {
                        scRunStart(0x13B, 0, 0);
                        plAnimeSet(PL(w->pl), 0xEC);
                        w->state++;
                    }
                    break;
                case 17:
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (w->timer == 0x2F58) {
                        scRunEnd();
                        scRunStart(0x13C, 0, 0);
                        w->state++;
                    }
                    break;
                case 18:
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (w->timer == 0x30E8) {
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 19:
                    switch (step) {
                        case 0:
                            if (demoAnimeStep(w->model, 0.4f, 2) == 1) {
                                plAnimeSet(PL(w->pl), 0xEB);
                                w->model->frame = polAnimeLastFrame(w->model);
                                st4clearGourdPoseDone = 1;
                                step++;
                            }
                            break;
                        case 1:
                            if (demoAnimeStep(w->model, 0.2f, 3) == 1) {
                                plAnimeSet(PL(w->pl), 0xE9);
                                step++;
                            }
                            break;
                        case 2:
                            demoAnimeStep(w->model, 0.2f, 0);
                            break;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *st4clearGoeCreate(Obj *obj) {
    return tskCreate(obj, st4clearGoeProc, obj->file, NULL);
}

extern f32 st4clearEbiX; /* Ebisumaru's x and z, each frame */
extern f32 st4clearEbiZ;

/* Ebisumaru (character 1; named "genoGOE" like Goemon) in the demo at (-2, -100, -28) under the
 * menu's last task; OBJMOVE's model 1 moves him. At frame 0x89 he is lifted to y 298.45 and
 * falls (speed + 0.049 a frame) to y 100 (0xE0, sound 0x19B), lands (0xD7), steps to
 * (-34, 100, 31) at 0x1E0, and links his animations with demoPlTunagiAnime (scene runs 0x125
 * and 0x130).
 * Matched 2026-10-09; it was parked only until the file's .data (its two statics) was C. */
s32 st4clearEbiProc(Obj *obj, s16 msg, void *arg);
s32 st4clearEbiProc(Obj *obj, s16 msg, void *arg) {
    static s32 step = -1;    /* 0x08005F88: his demoPlTunagiAnime step */
    static f32 speed = 1.0f; /* 0x08005F8C: his fall speed */
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
            init.place = -2.0f;
            init.height = -100.0f;
            init.side = -28.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            PRINTF("デモモード突入\n");
            plAnimeSet(PL(w->pl), 0xDE);
            w->model = p->model;
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, st4clearShadowDl, 0xBC);
            w->shadow->scale[2] = 0.09000000358f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 1;
            w->state = 0;
            w->model->scale[2] = 0.09700000286f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -2.0f;
            w->model->pos[1] = -100.0f;
            w->model->pos[2] = -28.0f;
            w->model->rot[1] = -0.6981317401f;
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
            w->timer++;
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
            st4clearEbiX = w->model->pos[0];
            st4clearEbiZ = w->model->pos[2];
            switch (w->state) {
                case 0:
                    w->model->pos[0] = -2.0f;
                    w->model->pos[1] = -100.0f;
                    w->model->pos[2] = -28.0f;
                    w->model->rot[1] = -0.6981317401f;
                    step = 0;
                    speed = 0.0f;
                    if (w->timer >= 0x89) {
                        w->model->pos[1] = 298.4500122f;
                        w->state++;
                    }
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.5f, 2);
                    speed += 0.049;
                    w->model->pos[1] -= speed;
                    if (w->model->pos[1] <= 100.0) {
                        w->model->pos[1] = 100.0f;
                        plAnimeSet(PL(w->pl), 0xE0);
                        sndPlayPan(0x19B, 0x28);
                        w->state++;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->model, 0.1f, 2);
                    if (polAnimeLastFrame(w->model) == (s32) w->model->frame) {
                        plAnimeSet(PL(w->pl), 0xD7);
                        w->state++;
                    }
                    break;
                case 3:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (w->timer >= 0x1E0) {
                        w->model->pos[0] = -34.0f;
                        w->model->pos[1] = 100.0f;
                        w->model->pos[2] = 31.0f;
                        w->model->rot[1] = 2.129301786f;
                        w->state++;
                    }
                    break;
                case 4:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (w->timer >= 0x370) {
                        step = 0;
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 5:
                    if (step != -1) {
                        step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.5f, 0xEB, 0xEC);
                        if (step == -1) {
                            scRunStart(0x125, 0, 0);
                        }
                    } else {
                        demoAnimeStep(w->model, 0.5f, 0);
                        fadeAnimStep(&w->eyes);
                        fadeAnimStep(&w->mouth);
                    }
                    if (w->timer >= 0x474) {
                        scRunEnd();
                        w->state++;
                        step = 0;
                    }
                    break;
                case 6:
                    if (step != -1) {
                        step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.5f, 0.2f, 0xEB, 0xE9);
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    if (w->timer >= 0x62C) {
                        plAnimeSet(PL(w->pl), 0x108);
                        w->state++;
                        step = 0;
                    }
                    break;
                case 7:
                    demoAnimeStep(w->model, 0.1f, 2);
                    if (w->timer >= 0xA6E) {
                        w->model->rot[1] = 0.6283185482f;
                        plAnimeSet(PL(w->pl), 0xD7);
                        w->state++;
                    }
                    break;
                case 8:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (w->timer == 0x14FA) {
                        scRunEnd();
                        step = 0;
                        w->state++;
                    }
                    break;
                case 9:
                    if (step != -1) {
                        step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.5f, 0xEB, 0xEC);
                        if (step == -1) {
                            scRunStart(0x130, 0, 0);
                        }
                    } else {
                        demoAnimeStep(w->model, 0.5f, 0);
                        fadeAnimStep(&w->eyes);
                        fadeAnimStep(&w->mouth);
                    }
                    if (w->timer == 0x1752) {
                        scRunEnd();
                        step = 0;
                        w->state++;
                    }
                    break;
                case 10:
                    if (step != -1) {
                        step = demoPlTunagiAnime(obj, 2, step, 2, 3, 0.5f, 0.2f, 0xEB, 0xE9);
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *st4clearEbiCreate(Obj *obj) {
    return tskCreate(obj, st4clearEbiProc, obj->file, NULL);
}

extern f32 st4clearSasX; /* Sasuke's x and z, each frame */
extern f32 st4clearSasZ;

/* "chaSAS": Sasuke (character 2) in the demo at (-13, -100, -37) under the menu's last task;
 * OBJMOVE's model 2 moves him. At frame 0x6B he is lifted to y 298.45 and falls (speed + 0.049
 * a frame) to y 100 (0xE0, sound 0x19B), lands (0xD7), steps to (-26, 100, 18) at 0x1E0, turns
 * at 0xA6E, and from 0x1176 links his animations with demoPlTunagiAnime (scene run 0x12E).
 * Matched 2026-10-09; it was parked only until the file's .data (its two statics) was C. */
s32 st4clearSasProc(Obj *obj, s16 msg, void *arg);
s32 st4clearSasProc(Obj *obj, s16 msg, void *arg) {
    static s32 step = -1;   /* 0x08005F90: his demoPlTunagiAnime step */
    static f32 speed = 1.0f; /* 0x08005F94: his fall speed */
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
            init.place = -13.0f;
            init.height = -100.0f;
            init.side = -37.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            PRINTF("デモモード突入\n");
            plAnimeSet(PL(w->pl), 0xDE);
            w->model = p->model;
            PRINTF("サスケ作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, st4clearShadowDl, 0xBC);
            w->shadow->scale[2] = 0.06499999762f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 2;
            w->state = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 0.09700000286f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = -13.0f;
            w->model->pos[1] = -100.0f;
            w->model->pos[2] = -37.0f;
            w->model->rot[1] = -0.5235987902f;
            PRINTF("サスケ作りました\n");
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
            w->timer++;
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
            st4clearSasX = w->model->pos[0];
            st4clearSasZ = w->model->pos[2];
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = 0.3f;
            switch (w->state) {
                case 0:
                    w->model->pos[0] = -13.0f;
                    w->model->pos[1] = -100.0f;
                    w->model->pos[2] = -37.0f;
                    w->model->rot[1] = -0.5235987902f;
                    step = 0;
                    speed = 0.0f;
                    if (w->timer >= 0x6B) {
                        w->model->pos[1] = 298.4500122f;
                        w->state++;
                    }
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.5f, 2);
                    speed += 0.049;
                    w->model->pos[1] -= speed;
                    if (w->model->pos[1] <= 100.0) {
                        w->model->pos[1] = 100.0f;
                        plAnimeSet(PL(w->pl), 0xE0);
                        sndPlayPan(0x19B, 0x10);
                        w->state++;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->model, 0.15f, 2);
                    if (polAnimeLastFrame(w->model) == (s32) w->model->frame) {
                        plAnimeSet(PL(w->pl), 0xD7);
                        w->state++;
                    }
                    break;
                case 3:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (w->timer >= 0x1E0) {
                        w->model->pos[0] = -26.0f;
                        w->model->pos[1] = 100.0f;
                        w->model->pos[2] = 18.0f;
                        w->model->rot[1] = 2.007128716f;
                        w->state++;
                    }
                    break;
                case 4:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (w->timer >= 0x62C) {
                        plAnimeSet(PL(w->pl), 0x108);
                        w->state++;
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.3f, 2);
                    if (w->timer >= 0xA6E) {
                        w->model->rot[1] = 0.3141592741f;
                        plAnimeSet(PL(w->pl), 0xD7);
                        w->state++;
                    }
                    break;
                case 6:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (w->timer == 0x1176) {
                        scRunEnd();
                        w->state++;
                    }
                    break;
                case 7:
                    if (step != -1) {
                        step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.2f, 0.5f, 0xEB, 0xEC);
                        if (step == -1) {
                            scRunStart(0x12E, 0, 0);
                        }
                    } else {
                        demoAnimeStep(w->model, 0.5f, 0);
                    }
                    if (w->timer == 0x12A2) {
                        scRunEnd();
                        w->state++;
                        step = 0;
                    }
                    break;
                case 8:
                    if (step != -1) {
                        step = demoPlTunagiAnime(obj, 0, step, 2, 3, 0.5f, 0.2f, 0xEB, 0xE9);
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *st4clearSasCreate(Obj *obj) {
    return tskCreate(obj, st4clearSasProc, obj->file, NULL);
}

extern f32 st4clearYaeX; /* Yae's x and z, each frame */
extern f32 st4clearYaeZ;
/* "chaYAE": Yae (character 3) in the demo at (18, -100, 4) under the menu's last task; OBJMOVE's
 * model 3 moves her. At frame 0x75 she is lifted to y 298.45 and falls (speed + 0.049 a frame)
 * to y 100 (sound 0x19B), lands (0xE0, then 0xD7), and steps through her animations by the
 * frame count (0x1E0, 0x62C, 0xA6E, 0x12A2 with scene run 0x12F, 0x14FA).
 * Matched 2026-10-09; it was parked only until the file's .data (its two statics) was C. */
s32 st4clearYaeProc(Obj *obj, s16 msg, void *arg);
s32 st4clearYaeProc(Obj *obj, s16 msg, void *arg) {
    static s32 step = -1;    /* 0x08005F98: a demoPlTunagiAnime step, only reset */
    static f32 speed = 1.0f; /* 0x08005F9C: her fall speed */
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
            init.place = 18.0f;
            init.height = -100.0f;
            init.side = 4.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            PRINTF("デモモード突入\n");
            plAnimeSet(PL(w->pl), 0xDE);
            w->model = p->model;
            PRINTF("ヤエ作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, st4clearShadowDl, 0xBC);
            w->shadow->scale[2] = 0.06499999762f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 3;
            w->state = 0;
            w->model->scale[2] = 0.09700000286f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 18.0f;
            w->model->pos[1] = -100.0f;
            w->model->pos[2] = 4.0f;
            w->model->rot[1] = -1.082104206f;
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
            w->timer++;
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
            st4clearYaeX = w->model->pos[0];
            st4clearYaeZ = w->model->pos[2];
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = 0.3f;
            switch (w->state) {
                case 0:
                    w->model->pos[0] = 18.0f;
                    w->model->pos[1] = -100.0f;
                    w->model->pos[2] = 4.0f;
                    w->model->rot[1] = -1.082104206f;
                    step = 0;
                    speed = 0.0f;
                    if (w->timer >= 0x75) {
                        w->model->pos[1] = 298.4500122f;
                        w->state++;
                    }
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.5f, 2);
                    speed += 0.049;
                    w->model->pos[1] -= speed;
                    if (w->model->pos[1] <= 100.0) {
                        w->model->pos[1] = 100.0f;
                        sndPlayPan(0x19B, 0x50);
                        plAnimeSet(PL(w->pl), 0xE0);
                        w->state++;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->model, 0.15f, 2);
                    if (polAnimeLastFrame(w->model) == (s32) w->model->frame) {
                        plAnimeSet(PL(w->pl), 0xD7);
                        w->state++;
                    }
                    break;
                case 3:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (w->timer >= 0x1E0) {
                        w->model->pos[0] = -32.0f;
                        w->model->pos[1] = 100.0f;
                        w->model->pos[2] = 9.0f;
                        w->model->rot[1] = 1.745329261f;
                        w->state++;
                    }
                    break;
                case 4:
                    demoAnimeStep(w->model, 0.2f, 0);
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    if (w->timer >= 0x62C) {
                        plAnimeSet(PL(w->pl), 0x108);
                        w->state++;
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.1f, 2);
                    if (w->timer >= 0xA6E) {
                        w->model->pos[0] = -16.0f;
                        w->model->pos[1] = 100.0f;
                        w->model->pos[2] = 9.0f;
                        w->model->rot[1] = 0.4014257491f;
                        plAnimeSet(PL(w->pl), 0xD7);
                        w->state++;
                    }
                    break;
                case 6:
                    if (w->timer >= 0x12A2) {
                        if (demoAnimeStep(w->model, 0.2f, 2) == 1) {
                            scRunEnd();
                            plAnimeSet(PL(w->pl), 0x101);
                            scRunStart(0x12F, 0, 0);
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
                case 7:
                    if (w->timer >= 0x14FA) {
                        if (demoAnimeStep(w->model, 0.16f, 2) == 1) {
                            plAnimeSet(PL(w->pl), 0xD7);
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.16f, 0);
                    }
                    break;
                case 8:
                    demoAnimeStep(w->model, 0.2f, 0);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *st4clearYaeCreate(Obj *obj) {
    return tskCreate(obj, st4clearYaeProc, obj->file, NULL);
}

/* "suzaku" (its proc st4clearSuzakuProc): its model, its shadow, its texture animation, a
 * frame count and a state. */
typedef struct SuzakuW {
    /* 0x00 */ AnimeFrame *model;
    /* 0x04 */ PolFrame *shadow;
    /* 0x08 */ Anim anim;
    /* 0x18 */ s32 timer;
    /* 0x1C */ s32 state;
} SuzakuW;

extern u8 st4clearSuzakuModelAnim[]; /* Suzaku's animations (file 0xCD) */
extern u8 st4clearSuzakuAnim2[];
extern u8 st4clearSuzakuState7Anim[];
AnimFrame st4clearSuzakuAnim[] = { /* its texture animation */
    { 0x0300A250, 8 },
    { 0x03009E30, 10 },
    { 0, 0 },
};
extern s32 st4clearSuzakuShadowAlpha;                   /* its shadow's alpha */
s32 st4clearSuzakuStep = -1;           /* set to 0 in its state 0 */
f32 st4clearSuzakuSpeed = 1.0f;         /* unused */

/* "suzaku": Suzaku (file 0xCD) at (13, 100, 97); OBJMOVE's model 4. It plays its animations by
 * the frame count and starts scene runs 0x126 to 0x13A. */
s32 st4clearSuzakuProc(Obj *obj, s16 msg, void *arg) {
    SuzakuW *w = (SuzakuW *) (obj + 1);
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
            demoAnimeSet(w->model, (u32) st4clearSuzakuModelAnim, 0xCD, 0.0f);
            fadeAnimStart(&w->anim, st4clearSuzakuAnim, 1, &w->model->segs[3], floadGetSetFile(0xCD, 0));
            demoPolSetDL(w->shadow, st4clearShadowDl, 0xBC);
            w->shadow->scale[2] = 0.07999999821f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->model->scale[2] = 0.09700000286f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 13.0f;
            w->model->pos[1] = 100.0f;
            w->model->pos[2] = 97.0f;
            w->model->rot[1] = 3.490658522f;
            PRINTF("スザク作りました\n");
            break;
        case 1:
            frmDestroy(&w->model->f);
            frmDestroy(&w->shadow->f);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, st4clearSuzakuShadowAlpha);
            break;
        case 0:
            w->timer++;
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
                    w->model->pos[0] = 13.0f;
                    w->model->pos[1] = 100.0f;
                    w->model->pos[2] = 97.0f;
                    w->model->rot[1] = 3.490658522f;
                    st4clearSuzakuStep = 0;
                    w->state++;
                    break;
                case 1:
                    if (w->timer == 0x49C) {
                        demoAnimeStep(w->model, 0.2f, 0);
                        scRunEnd();
                        scRunStart(0x126, 0, 0);
                    } else if (w->timer >= 0x7B2) {
                        if (demoAnimeStep(w->model, 0.2f, 2) == 1) {
                            demoAnimeSet(w->model, (u32) st4clearSuzakuAnim2, 0xCD, 0.0f);
                            fadeAnimStart(&w->anim, st4clearSuzakuAnim, 1, &w->model->segs[3],
                                          floadGetSetFile(0xCD, 0));
                            scRunEnd();
                            scRunStart(0x129, 0, 0);
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
                case 2:
                    if (w->timer < 0x993) {
                        fadeAnimStep(&w->anim);
                    }
                    if (w->timer >= 0x992) {
                        if (demoAnimeStep(w->model, 0.2f, 2) == 1) {
                            demoAnimeSet(w->model, (u32) st4clearSuzakuModelAnim, 0xCD, 0.0f);
                            fadeAnimStart(&w->anim, st4clearSuzakuAnim, 1, &w->model->segs[3],
                                          floadGetSetFile(0xCD, 0));
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
                case 3:
                    if (w->timer >= 0xA6E) {
                        w->model->pos[0] = 7.0f;
                        w->model->pos[1] = 100.0f;
                        w->model->pos[2] = 72.0f;
                        w->model->rot[1] = 3.403392076f;
                        if (demoAnimeStep(w->model, 0.2f, 2) == 1) {
                            demoAnimeSet(w->model, (u32) st4clearSuzakuAnim2, 0xCD, 0.0f);
                            fadeAnimStart(&w->anim, st4clearSuzakuAnim, 1, &w->model->segs[3],
                                          floadGetSetFile(0xCD, 0));
                            scRunEnd();
                            scRunStart(0x12B, 0, 0);
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
                case 4:
                    fadeAnimStep(&w->anim);
                    if (w->timer == 0xCC6) {
                        scRunEnd();
                        scRunStart(0x12C, 0, 0);
                        demoAnimeStep(w->model, 0.2f, 0);
                    } else if (w->timer == 0xF82) {
                        scRunEnd();
                        scRunStart(0x12D, 0, 0);
                        demoAnimeStep(w->model, 0.2f, 0);
                    } else if (w->timer >= 0x1176) {
                        if (demoAnimeStep(w->model, 0.2f, 2) == 1) {
                            demoAnimeSet(w->model, (u32) st4clearSuzakuModelAnim, 0xCD, 0.0f);
                            fadeAnimStart(&w->anim, st4clearSuzakuAnim, 1, &w->model->segs[3],
                                          floadGetSetFile(0xCD, 0));
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
                case 5:
                    if (w->timer == 0x1752) {
                        w->model->pos[0] = 28.0f;
                        w->model->pos[1] = 100.0f;
                        w->model->pos[2] = 26.0f;
                        w->model->rot[1] = 3.926990986f;
                    }
                    if (w->timer >= 0x1752) {
                        fadeAnimStart(&w->anim, st4clearSuzakuAnim, 1, &w->model->segs[3],
                                      floadGetSetFile(0xCD, 0));
                        if (demoAnimeStep(w->model, 0.2f, 2) == 1) {
                            demoAnimeSet(w->model, (u32) st4clearSuzakuAnim2, 0xCD, 0.0f);
                            fadeAnimStart(&w->anim, st4clearSuzakuAnim, 1, &w->model->segs[3],
                                          floadGetSetFile(0xCD, 0));
                            scRunStart(0x131, 0, 0);
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
                case 6:
                    fadeAnimStep(&w->anim);
                    if (w->timer == 0x1946) {
                        scRunEnd();
                        scRunStart(0x132, 0, 0);
                        demoAnimeStep(w->model, 0.2f, 0);
                    } else if (w->timer == 0x1C02) {
                        scRunEnd();
                        scRunStart(0x133, 0, 0);
                        demoAnimeStep(w->model, 0.2f, 0);
                    } else if (w->timer == 0x1EBE) {
                        scRunEnd();
                        scRunStart(0x134, 0, 0);
                        demoAnimeStep(w->model, 0.2f, 0);
                    } else if (w->timer >= 0x210C) {
                        fadeAnimStart(&w->anim, st4clearSuzakuAnim, 1, &w->model->segs[3],
                                      floadGetSetFile(0xCD, 0));
                        if (demoAnimeStep(w->model, 0.2f, 2) == 1) {
                            demoAnimeSet(w->model, (u32) st4clearSuzakuModelAnim, 0xCD, 0.0f);
                            fadeAnimStart(&w->anim, st4clearSuzakuAnim, 1, &w->model->segs[3],
                                          floadGetSetFile(0xCD, 0));
                            w->state++;
                        }
                    } else if (w->timer < 0x1C02 || w->timer >= 0x1EBF) {
                        demoAnimeStep(w->model, 0.2f, 0);
                    } else {
                        demoAnimeStep(w->model, 0.3f, 0);
                    }
                    break;
                case 7:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (w->timer == 0x26A2) {
                        demoAnimeSet(w->model, (u32) st4clearSuzakuState7Anim, 0xCD, 0.0f);
                        fadeAnimStart(&w->anim, st4clearSuzakuAnim, 1, &w->model->segs[3],
                                      floadGetSetFile(0xCD, 0));
                        scRunStart(0x137, 0, 0);
                        w->state++;
                    }
                    break;
                case 8:
                    fadeAnimStep(&w->anim);
                    demoAnimeStep(w->model, 0.13f, 2);
                    if (w->timer == 0x27BA) {
                        fadeAnimStart(&w->anim, st4clearSuzakuAnim, 0, &w->model->segs[3],
                                      floadGetSetFile(0xCD, 0));
                        scRunEnd();
                        w->model->pos[1] = -100.0f;
                        w->state++;
                    }
                    break;
                case 9:
                    if (w->timer == 0x28B4) {
                        w->model->pos[0] = 7.0f;
                        w->model->pos[1] = 100.0f;
                        w->model->pos[2] = 72.0f;
                        w->model->rot[1] = 3.403392076f;
                        demoAnimeSet(w->model, (u32) st4clearSuzakuAnim2, 0xCD, 0.0f);
                        fadeAnimStart(&w->anim, st4clearSuzakuAnim, 1, &w->model->segs[3],
                                      floadGetSetFile(0xCD, 0));
                        scRunEnd();
                        scRunStart(0x139, 0, 0);
                        w->state++;
                    }
                    break;
                case 10:
                    fadeAnimStep(&w->anim);
                    if (w->timer == 0x2BD4) {
                        scRunEnd();
                        scRunStart(0x13A, 0, 0);
                    }
                    if (w->timer >= 0x2DBE) {
                        if (demoAnimeStep(w->model, 0.2f, 2) == 1) {
                            demoAnimeSet(w->model, (u32) st4clearSuzakuModelAnim, 0xCD, 0.0f);
                            fadeAnimStart(&w->anim, st4clearSuzakuAnim, 0, &w->model->segs[3],
                                          floadGetSetFile(0xCD, 0));
                            w->state++;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
                case 11:
                    demoAnimeStep(w->model, 0.2f, 0);
                    break;
            }
            st4clearSuzakuX = w->model->pos[0];
            st4clearSuzakuZ = w->model->pos[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *st4clearSuzakuCreate(Obj *obj) {
    return tskCreate(obj, st4clearSuzakuProc, obj->file, NULL);
}

/* "hyotan" (its proc st4clearHyotanProc): its model, a frame count and a state. */
typedef struct HyotanW {
    /* 0x00 */ AnimeFrame *model;
    /* 0x04 */ AnimeFrame *objMoveModel; /* never set: OBJMOVE's code reads its model here, as in the
                                   * players' work (DemoPlW) it was copied from */
    /* 0x08 */ u8 pad08[0x10];
    /* 0x18 */ s32 timer;
    /* 0x1C */ s32 state;
} HyotanW;

extern u8 st4clearHyotanAnim[]; /* the gourd model's animations (file 0xC4) */
extern u8 st4clearHyotanState1Anim[];
extern u8 st4clearHyotanState2Anim[];
extern u8 st4clearHyotanState3Anim[];
extern u8 st4clearHyotanState4Anim[];
extern s32 st4clearGourdPoseDone; /* set by genoGOE (st4clearGoeProc): 0, then 1 */

/* "hyotan": the gourd (file 0xC4) at (28, -200, 26); OBJMOVE's model 5. At frame 0x26A2 it is
 * at (28, 100, 26), at 0x26CA sound 0x19D, at 0x27BA it moves to (0, 100, 0); then its
 * animations follow st4clearGourdPoseDone. */
s32 st4clearHyotanProc(Obj *obj, s16 msg, void *arg) {
    HyotanW *w = (HyotanW *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "hyotan";
            w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->model == NULL) {
                return -1;
            }
            PRINTF("ひょうたん作ります\n");
            demoAnimeSet(w->model, (u32) st4clearHyotanAnim, 0xC4, 0.0f);
            w->state = 0;
            w->model->scale[2] = 0.09700000286f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->rot[1] = 0.0f;
            w->model->pos[0] = w->model->pos[2] = w->model->rot[1];
            w->model->pos[1] = -200.0f;
            w->model->pos[0] = 28.0f;
            w->model->pos[1] = -200.0f;
            w->model->pos[2] = 26.0f;
            w->model->rot[1] = 3.926990986f;
            PRINTF("ひょうたん作りました\n");
            break;
        case 1:
            frmDestroy((Frame *) w->model);
            break;
        case 3:
            break;
        case 0:
            w->timer++;
            w->model->f.flags |= 2;
            if (gObjMoveSel == 5) {
                w->objMoveModel->pos[0] += gObjMoveDx;
                w->objMoveModel->pos[1] += gObjMoveDy;
                w->objMoveModel->pos[2] += gObjMoveDz;
                w->objMoveModel->rot[1] += gObjMoveDrot;
                gObjMovePosX = w->objMoveModel->pos[0];
                gObjMovePosY = w->objMoveModel->pos[1];
                gObjMovePosZ = w->objMoveModel->pos[2];
                gObjMoveRot = w->objMoveModel->rot[1];
                if (gGameFrameCount % 20 >= 10) {
                    w->objMoveModel->f.flags |= 2;
                } else {
                    w->objMoveModel->f.flags &= ~2;
                }
            }
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            switch (w->state) {
                case 0:
                    if (w->timer == 0x26A2) {
                        w->model->pos[0] = 28.0f;
                        w->model->pos[1] = 100.0f;
                        w->model->pos[2] = 26.0f;
                        w->model->rot[1] = 3.926990986f;
                        w->state++;
                    }
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.13f, 2);
                    if (w->timer == 0x26CA) {
                        sndPlay(0x19D);
                    }
                    if (w->timer == 0x27BA) {
                        demoAnimeSet(w->model, (u32) st4clearHyotanState1Anim, 0xC4, 0.0f);
                        w->model->pos[0] = 0.0f;
                        w->model->pos[1] = 100.0f;
                        w->model->pos[2] = 0.0f;
                        w->model->rot[1] = 3.612831593f;
                        w->state++;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->model, 0.4f, 2);
                    if (w->timer == 0x28B4) {
                        demoAnimeSet(w->model, (u32) st4clearHyotanState2Anim, 0xC4, 0.0f);
                        w->model->rot[1] = 0.226892814f;
                        w->state++;
                    }
                    break;
                case 3:
                    demoAnimeStep(w->model, 0.2f, 0);
                    if (st4clearGourdPoseDone == 0) {
                        demoAnimeSet(w->model, (u32) st4clearHyotanState3Anim, 0xC4, 0.0f);
                        w->state++;
                    }
                    break;
                case 4:
                    if (demoAnimeStep(w->model, 0.4f, 2) == 1) {
                        demoAnimeSet(w->model, (u32) st4clearHyotanState4Anim, 0xC4, 0.0f);
                        w->state++;
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.4f, 0);
                    if (st4clearGourdPoseDone == 1) {
                        demoAnimeSet(w->model, (u32) st4clearHyotanState3Anim, 0xC4, 0.0f);
                        w->model->frame = polAnimeLastFrame(w->model);
                        w->state++;
                    }
                    break;
                case 6:
                    if (demoAnimeStep(w->model, 0.2f, 3) == 1) {
                        demoAnimeSet(w->model, (u32) st4clearHyotanState2Anim, 0xC4, 0.0f);
                        w->state++;
                    }
                    break;
                case 7:
                    demoAnimeStep(w->model, 0.2f, 0);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *st4clearHyotanCreate(Obj *obj) {
    return tskCreate(obj, st4clearHyotanProc, obj->file, NULL);
}
