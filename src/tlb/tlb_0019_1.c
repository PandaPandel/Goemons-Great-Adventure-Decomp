#include "tlb.h"
#include "symbol_pad.h"
#include "tlb/tlb_0019.h"

/* Main's sinf and cosf, called through their addresses as TLB code calls main. */
#define sinf (*sinf)
#define cosf (*cosf)

/* The file's .bss (0x08007FB0) starts with WATER's eight statics, though his is the file's
 * last function: their symbol numbers pass 256 and wrap (IDO_NOTES 337, SYMBOL_PAD). */

/* "genoimp"'s work. */
typedef struct GenoimpW {
    /* 0x00 */ AnimeFrame *model;
    /* 0x04 */ s32 timer;
    /* 0x08 */ s32 state; /* 0: arriving; 1: there */
} GenoimpW;

extern s32 gdemoFrame; /* the demo's frame counter */
extern u8 gdemoImpactAnim[];          /* Impact's animation */
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
void gdemoValcanFire(Obj *obj, f32 angle);

/* "genoimp", Impact in the menu demo: its model (animation 0xBD, scale 13) at (1720, 250,
 * -5403). Each frame it can be moved by the object mover (blinking while it is), and unless
 * the demo camera holds it animates until frame 3900; from frame 11 of every 30 to frame 500
 * it fires a Valcan every 8th frame along its facing, which swings 45 degrees about -60 with
 * the frame. */
s32 gdemoGenoimpProc(Obj *obj, s16 msg, void *arg) {
    GenoimpW *w = (GenoimpW *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "genoimp";
            if ((w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime)) == NULL) {
                return -1;
            }
            demoAnimeSet(w->model, (u32) gdemoImpactAnim, 0xBD, 0);
            w->model->flags |= 0x20;
            w->model->scale[2] = 13.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 1720.0f;
            w->model->pos[1] = 250.0f;
            w->model->pos[2] = -5403.0f;
            w->timer = 0;
            w->state = 0;
            break;
        case 1:
            frmDestroy((Frame *) w->model);
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
            switch (w->state) {
                case 0:
                    if (gdemoFrame % 30 > 10 && !(gdemoFrame & 7) &&
                        gdemoFrame < 500) {
                        (*gdemoValcanFire)(obj, w->model->rot[1]);
                    }
                    if (gdemoFrame < 3900) {
                        demoAnimeStep(w->model, 0.2f, 0);
                    } else {
                        w->state = 1;
                    }
                    break;
                case 1:
                    break;
            }
            w->model->rot[1] =
                (sinf((gdemoFrame / 2 + 60) * 0.0174532930038800771) * 45.0f + -60.0f) *
                0.0174532930038800771;
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *gdemoGenoimpCreate(Obj *obj) {
    return tskCreate(obj, gdemoGenoimpProc, obj->file, NULL);
}

/* The "geno" demo characters' work: tlb_0029_2's DemoPlW with a frame timer at 0x30. */
typedef struct GenoPlW {
    /* 0x00 */ Obj *pl;
    /* 0x04 */ AnimeFrame *model;
    /* 0x08 */ PolFrame *shadow;
    /* 0x0C */ Anim eyes;
    /* 0x1C */ Anim mouth;
    /* 0x2C */ s32 chara;
    /* 0x30 */ s32 timer;
    /* 0x34 */ s32 state;
} GenoPlW;

#define DPRINTF

extern Gfx gdemoShadowDl[];         /* the shadow */
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
/* This file's .data (0x08006C50, IDO_NOTES 333 and 343), defined in the game's order: the
 * characters' texture animations (demotool.c's table, as tlb_0020_1's; nothing here reads it),
 * the light's direction index and its six x directions, the shadows' scales, the smokes' texture
 * animations, then the two flash counters, the lightnings' own statics (last). */
AnimFrame gdemoCharaAnims[] = {
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
s32 gdemoLightDirIndex = 1; /* which of the light's six directions */
s32 gdemoLightDirX[6] = { -30, 20, -20, 45, -40, 43 }; /* the light's directions (its x) */
f32 gdemoShadowLength = 5.0f; /* the shadow's length and angle */
f32 gdemoShadowAngle = 5.0f;

/* "genoGOE" (Goemon, character 0) in the castle demo, at (913, 93, -1664) under the menu's last
 * task; OBJMOVE's model 1 moves him. His long shadow (2.7 long) follows him 7 up. He walks in
 * until frame 0x58C, hears the news at 0x96B ("なんだとー！", scene run 0x6A), gets going
 * ("こうしちゃいられねー！", 0x6D) and sets off ("いくぜ！", 0x71). */
s32 gdemoGenoGoeProc(Obj *obj, s16 msg, void *arg) {
    static s32 step; /* .bss 0x08007FB8 */
    s32 pad[2];
    GenoPlW *w = (GenoPlW *) (obj + 1);
    Player *p;
    PlInit init;

    switch (msg) {
        case 2:
            obj->name = "genoGOE";
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, gdemoShadowDl, 0xBD);
            plMakeInit(&init, 0, 0);
            init.flags |= 0x90000040;
            init.zone = 0;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            w->model = p->model;
            w->chara = 0;
            plAnimeSet(PL(w->pl), 0xE7);
            demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
            demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
            w->model->flags |= 0x20;
            w->shadow->flags |= 0x20;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->shadow->scale[0] = w->shadow->scale[1];
            w->shadow->scale[2] = 2.7f;
            w->model->pos[0] = 913.0f;
            w->model->pos[1] = 93.0f;
            w->model->pos[2] = -1664.0f;
            w->model->rot[1] = 3.1415927f;
            w->shadow->rot[1] = 3.1415927f;
            w->timer = 0;
            w->state = 0;
            break;
        case 1:
            frmDestroy(&w->model->f);
            frmDestroy(&w->shadow->f);
            tskDestroy(w->pl);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, 0xD0);
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
            w->timer++;
            switch (w->state) {
                case 0:
                    if (gdemoFrame == 0x2DA) {
                        w->model->pos[0] = 830.0f;
                        w->model->pos[1] = 97.0f;
                        w->model->pos[2] = -1810.0f;
                    }
                    if (gdemoFrame >= 0x17D && gdemoFrame < 0x2DA) {
                        if (w->model->pos[0] < 886.0f) {
                            w->model->pos[0] -= 1.0f;
                        }
                    }
                    if (gdemoFrame < 0x58C) {
                        demoAnimeStep(w->model, 0.2f, 0);
                    } else {
                        w->state = 1;
                        DPRINTF("GOE_ANI_CHANGE_START\n");
                        plAnimeSet(PL(w->pl), 0xEA);
                        demoTexAnimK0(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        DPRINTF("GOE_ANI_CHANGE_END\n");
                    }
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.05f, 2);
                    if (gdemoFrame == 0x6D6) {
                        w->state = 2;
                        plAnimeSet(PL(w->pl), 0xE8);
                        demoTexAnimK0(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->model->pos[0] = 843.0f;
                        w->model->pos[1] = 96.0f;
                        w->model->pos[2] = -1767.0f;
                    }
                    break;
                case 2:
                    if (gdemoFrame >= 0x96B) {
                        if (demoAnimeStep(w->model, 0.2f, 2)) {
                            w->state = 3;
                            plAnimeSet(PL(w->pl), 0xEE);
                            demoTexAnimK0(&w->eyes, 3, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK0(&w->mouth, 3, &PL(w->pl)->model->segs[4]);
                        }
                    } else {
                        demoAnimeStep(w->model, 0.1f, 0);
                    }
                    break;
                case 3:
                    if (demoAnimeStep(w->model, 0.2f, 2)) {
                        w->state = 4;
                        plAnimeSet(PL(w->pl), 0xEF);
                        demoTexAnimK0(&w->eyes, 3, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 3, &PL(w->pl)->model->segs[4]);
                        DPRINTF("ゴエモン「なんだとー！」\n");
                        scRunEnd();
                        scRunStart(0x6A, 0, 0);
                    }
                    break;
                case 4:
                    demoAnimeStep(w->model, 0.6f, 0);
                    if (gdemoFrame == 0xA1E) {
                        step = 0;
                        w->state = 5;
                    }
                    break;
                case 5:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 3, 3, 0.2f, 0.2f, 0xEE, 0xE8)) == -1) {
                        DPRINTF("<goe>step6へ\n");
                        w->state = 6;
                        step = 0;
                    }
                    break;
                case 6:
                    if (gdemoFrame >= 0xD49) {
                        step = demoPlTunagiAnime(obj, 3, step, 2, 2, 0.4f, 0.4f, 0xEE, 0xEF);
                    } else {
                        demoAnimeStep(w->model, 0.1f, 0);
                    }
                    if (step == -1) {
                        w->state = 7;
                        DPRINTF("ゴエモン「こうしちゃいられねー！ 早く助けに行かなきゃ！」\n");
                        scRunEnd();
                        scRunStart(0x6D, 0, 0);
                        step = 0;
                        w->timer = 0;
                    }
                    break;
                case 7:
                    if (gdemoFrame >= 0xE89) {
                        if ((step = demoPlTunagiAnime(obj, 0, step, 3, 3, 0.3f, 0.3f, 0xEE, 0xE8)) == -1) {
                            DPRINTF("<goe>step8へ\n");
                            w->state = 8;
                            step = 0;
                        }
                    } else if (w->timer < 0x46) {
                        demoAnimeStep(w->model, 0.25f, 0);
                    } else {
                        demoAnimeStep(w->model, 0.6f, 0);
                    }
                    break;
                case 8:
                    if (gdemoFrame >= 0x11A9) {
                        if ((step = demoPlTunagiAnime(obj, 0, step, 2, 2, 0.4f, 0.4f, 0xED, 0xE9)) == -1) {
                            w->state = 9;
                            step = 0;
                        }
                    } else {
                        demoAnimeStep(w->model, 0.1f, 0);
                    }
                    break;
                case 9:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (gdemoFrame == 0x11E4) {
                        step = 0;
                        w->state = 10;
                    }
                    break;
                case 10:
                    if ((step = demoPlTunagiAnime(obj, 3, step, 2, 2, 0.2f, 0.2f, 0xEB, 0xEC)) == -1) {
                        w->state = 11;
                        step = 0;
                        w->timer = 0;
                        DPRINTF("そうとわかりゃ、いくぜ！\n");
                        scRunEnd();
                        scRunStart(0x71, 0, 0);
                    }
                    break;
                case 11:
                    if (gdemoFrame == 0x1298) {
                        step = 0;
                        w->state = 12;
                    }
                    if (w->timer >= 0x3D) {
                        demoAnimeStep(w->model, 0.5f, 0);
                    } else {
                        demoAnimeStep(w->model, 0.2f, 0);
                    }
                    break;
                case 12:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 3, 3, 0.4f, 0.3f, 0xEB, 0xE9)) == -1) {
                        w->state = 13;
                        step = 0;
                    }
                    break;
                case 13:
                    demoAnimeStep(w->model, 0.1f, 0);
                    break;
            }
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[1] = w->model->pos[1] + 7.0f;
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->scale[2] = gdemoShadowLength * 2.7;
            w->shadow->rot[1] = gdemoShadowAngle;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "genoEBI" (Ebisumaru, character 1) in the castle demo, at (1057, 93, -1832); OBJMOVE's model
 * 2. He walks in, then at 0xEA6 doubts the climb ("あんな高いとこ昇れまへんで！", scene run 0x6E)
 * turning to about 306 degrees and back to 275 (its "RY+"/"RY-" prints), and at the plan
 * agrees ("なるヘソ！", 0x70). */
s32 gdemoGenoEbiProc(Obj *obj, s16 msg, void *arg) {
    static s32 step; /* .bss 0x08007FBC */
    static s32 turn; /* .bss 0x08007FC0: 0 turning right, 1 turning back */
    s32 pad[2];
    GenoPlW *w = (GenoPlW *) (obj + 1);
    Player *p;
    PlInit init;

    switch (msg) {
        case 2:
            obj->name = "genoEBI";
            plMakeInit(&init, 0, 1);
            init.flags |= 0x90000040;
            init.zone = 0;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            w->model = p->model;
            w->chara = 1;
            plAnimeSet(PL(w->pl), 0xE7);
            demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
            demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, gdemoShadowDl, 0xBD);
            w->model->flags |= 0x20;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->shadow->flags |= 0x20;
            w->shadow->scale[1] = 1.0f;
            w->shadow->scale[0] = w->shadow->scale[1];
            w->shadow->scale[2] = 2.7f;
            w->shadow->rot[1] = 3.1415927f;
            w->model->pos[0] = 1057.0f;
            w->model->pos[1] = 93.0f;
            w->model->pos[2] = -1832.0f;
            w->model->rot[1] = 3.1415927f;
            w->timer = 0;
            w->state = 0;
            break;
        case 1:
            frmDestroy(&w->model->f);
            tskDestroy(w->pl);
            frmDestroy(&w->shadow->f);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, 0xD0);
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
            w->timer++;
            switch (w->state) {
                case 0:
                    if (gdemoFrame == 0x2DA) {
                        w->model->pos[0] = 1000.0f;
                        w->model->pos[1] = 93.0f;
                        w->model->pos[2] = -1888.0f;
                    }
                    if (gdemoFrame < 0x58C) {
                        demoAnimeStep(w->model, 0.2f, 0);
                    } else {
                        w->state = 1;
                        plAnimeSet(PL(w->pl), 0xEA);
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.05f, 2);
                    if (gdemoFrame == 0x6D6) {
                        w->state = 2;
                        plAnimeSet(PL(w->pl), 0xE8);
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->model->rot[1] = 4.0840707f;
                        w->model->pos[0] = 1050.0f;
                        w->model->pos[1] = 98.0f;
                        w->model->pos[2] = -1884.0f;
                    }
                    break;
                case 2:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (gdemoFrame == 0xEA6) {
                        w->state = 3;
                        step = 0;
                    }
                    break;
                case 3:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 2, 0.3f, 0.3f, 0xED, 0xE9)) == -1) {
                        w->state = 4;
                        step = 0;
                        turn = 0;
                    }
                    break;
                case 4:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 2, 0.3f, 0.3f, 0xEB, 0xEC)) == -1) {
                        w->state = 5;
                        DPRINTF("でも、あんな高いとこ昇れまへんで！\n");
                        scRunEnd();
                        scRunStart(0x6E, 0, 0);
                        step = 0;
                        turn = 0;
                    }
                    if (w->model->rot[1] < 5.340707659187304) {
                        DPRINTF("RY+ \n");
                        w->model->rot[1] += 0.012217305102716054;
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (turn == 0) {
                        if (w->model->rot[1] < 5.340707659187304) {
                            DPRINTF("RY+ \n");
                            w->model->rot[1] += 0.01745329300388008;
                        } else {
                            turn = 1;
                        }
                    }
                    if (turn == 1) {
                        if (w->model->rot[1] > 4.799655576067021) {
                            DPRINTF("RY- \n");
                            w->model->rot[1] -= 0.00872664650194004;
                        }
                    }
                    if (gdemoFrame == 0xFBE) {
                        w->state = 6;
                        step = 0;
                    }
                    break;
                case 6:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 3, 3, 0.3f, 0.1f, 0xEB, 0xE9)) == -1) {
                        w->state = 7;
                        w->timer = 0;
                    }
                    break;
                case 7:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (gdemoFrame == 0x116C) {
                        w->state = 8;
                        step = 0;
                    }
                    break;
                case 8:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 2, 0.5f, 0.5f, 0xEB, 0xEC)) == -1) {
                        w->state = 9;
                        DPRINTF("なるヘソ！\n");
                        scRunEnd();
                        scRunStart(0x70, 0, 0);
                        turn = 0;
                    }
                    break;
                case 9:
                    demoAnimeStep(w->model, 0.6f, 0);
                    if (gdemoFrame == 0x11C6) {
                        w->state = 10;
                        step = 0;
                    }
                    break;
                case 10:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 3, 3, 0.3f, 0.3f, 0xEB, 0xE9)) == -1) {
                        w->state = 11;
                        w->timer = 0;
                    }
                    break;
                case 11:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (gdemoFrame == 0x1248) {
                        w->state = 12;
                        step = 0;
                    }
                    break;
                case 12:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 2, 2, 0.2f, 0.1f, 0xF0, 0xF1)) == -1) {
                        w->state = 13;
                        w->timer = 0;
                    }
                    break;
                case 13:
                    demoAnimeStep(w->model, 0.1f, 0);
                    break;
            }
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[1] = w->model->pos[1] + 7.0f;
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->scale[2] = gdemoShadowLength * 2.7;
            w->shadow->rot[1] = gdemoShadowAngle;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "genoSAS" (Sasuke, character 2) in the castle demo, at (654, 113, -1812); OBJMOVE's model 3.
 * He calms the others ("落ち着くでゴザル", scene run 0x68), then turns to about 110 degrees and
 * gives the plan ("天守閣まで登ってから、飛び移ればよいでゴザル。", 0x6F). */
s32 gdemoGenoSasProc(Obj *obj, s16 msg, void *arg) {
    static s32 step; /* .bss 0x08007FC4 */
    s32 pad[2];
    GenoPlW *w = (GenoPlW *) (obj + 1);
    Player *p;
    PlInit init;

    switch (msg) {
        case 2:
            obj->name = "genoSAS";
            plMakeInit(&init, 0, 2);
            init.flags |= 0x90000040;
            init.zone = 0;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            w->model = p->model;
            w->chara = 2;
            plAnimeSet(PL(w->pl), 0xE7);
            demoTexAnimK2(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
            demoTexAnimK2(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, gdemoShadowDl, 0xBD);
            w->model->flags |= 0x20;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->shadow->scale[0] = w->shadow->scale[1];
            w->shadow->scale[2] = 2.4f;
            w->model->pos[0] = 654.0f;
            w->model->pos[1] = 113.0f;
            w->model->pos[2] = -1812.0f;
            w->model->rot[1] = 3.1415927f;
            w->shadow->rot[1] = 3.1415927f;
            w->timer = 0;
            w->state = 0;
            break;
        case 1:
            frmDestroy(&w->model->f);
            frmDestroy(&w->shadow->f);
            tskDestroy(w->pl);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, 0xD0);
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
            switch (w->state) {
                case 0:
                    if (gdemoFrame == 0x2DA) {
                        w->model->pos[0] = 661.0f;
                        w->model->pos[1] = 95.0f;
                        w->model->pos[2] = -1889.0f;
                    }
                    if (gdemoFrame < 0x58C) {
                        demoAnimeStep(w->model, 0.2f, 0);
                    } else {
                        w->state = 1;
                        plAnimeSet(PL(w->pl), 0xEA);
                        demoTexAnimK2(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 1:
                    demoAnimeStep(w->model, 0.05f, 2);
                    if (gdemoFrame == 0x6D6) {
                        w->state = 2;
                        plAnimeSet(PL(w->pl), 0xE8);
                        demoTexAnimK2(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->model->rot[1] = 2.8972466f;
                        w->model->pos[0] = 664.0f;
                        w->model->pos[1] = 99.0f;
                        w->model->pos[2] = -1812.0f;
                        step = 0;
                    }
                    break;
                case 2:
                    if (gdemoFrame >= 0x74F) {
                        if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.4f, 0.4f, 0xEE, 0xEF)) == -1) {
                            w->state = 3;
                            DPRINTF("サスケ:「落ち着くでゴザル」\n");
                            scRunEnd();
                            scRunStart(0x68, 0, 0);
                        }
                    } else {
                        demoAnimeStep(w->model, 0.4f, 0);
                    }
                    break;
                case 3:
                    demoAnimeStep(w->model, 0.5f, 0);
                    if (gdemoFrame == 0x816) {
                        w->state = 4;
                        step = 0;
                    }
                    break;
                case 4:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 3, 3, 0.4f, 0.4f, 0xEE, 0xE8)) == -1) {
                        w->state = 5;
                        step = 0;
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (gdemoFrame == 0xFB4) {
                        w->state = 6;
                    }
                    break;
                case 6:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.4f, 0.4f, 0xED, 0xE9)) == -1) {
                        w->state = 7;
                        step = 0;
                    }
                    if (w->model->rot[1] > 1.9198622304268087) {
                        w->model->rot[1] -= 0.01745329300388008;
                    }
                    break;
                case 7:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.4f, 0.4f, 0xEB, 0xEC)) == -1) {
                        w->state = 8;
                        step = 0;
                        DPRINTF("天守閣まで登ってから、飛び移ればよいでゴザル。\n");
                        scRunEnd();
                        scRunStart(0x6F, 0, 0);
                    }
                    if (w->model->rot[1] > 1.9198622304268087) {
                        w->model->rot[1] -= 0.03490658600776016;
                    }
                    break;
                case 8:
                    demoAnimeStep(w->model, 0.3f, 0);
                    if (gdemoFrame == 0x1158) {
                        w->state = 9;
                        step = 0;
                    }
                    fadeAnimStep(&w->eyes);
                    fadeAnimStep(&w->mouth);
                    break;
                case 9:
                    if ((step = demoPlTunagiAnime(obj, 0, step, 3, 3, 0.2f, 0.2f, 0xEB, 0xE9)) == -1) {
                        w->state = 10;
                        step = 0;
                    }
                    break;
                case 10:
                    demoAnimeStep(w->model, 0.1f, 0);
                    if (gdemoFrame == 0x1220) {
                        w->state = 11;
                        step = 0;
                    }
                    break;
                case 11:
                    if ((step = demoPlTunagiAnime(obj, 2, step, 2, 2, 0.1f, 0.1f, 0xF0, 0xF1)) == -1) {
                        w->state = 12;
                        w->timer = 0;
                    }
                    break;
                case 12:
                    demoAnimeStep(w->model, 0.1f, 0);
                    break;
            }
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[1] = w->model->pos[1] + 8.0f;
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->scale[2] = gdemoShadowLength * 2.4;
            w->shadow->rot[1] = gdemoShadowAngle;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "genoKURO"'s work. */
typedef struct KuroW {
    /* 0x00 */ AnimeFrame *model;
    /* 0x04 */ PolFrame *shadow;
    /* 0x08 */ s32 timer;
    /* 0x0C */ s32 state;
} KuroW;

extern u8 gdemoKurobeiAnim[]; /* Kurobei's animations (file 0xBF) */
extern u8 gdemoKurobeiState0Anim[];
extern u8 gdemoKurobeiState2Anim[];
extern u8 gdemoKurobeiState1Anim[];
extern u8 gdemoKurobeiState5Anim[];
extern u8 gdemoKurobeiState6Anim[];
extern u8 gdemoKurobeiState3Anim[];
extern u8 gdemoKurobeiState4Anim[];
void gdemoKemuriFlatCreate(f32 x, f32 y, f32 z);

/* "genoKURO", Kurobei (file 0xBF) in the castle demo. He falls in from (1079, 775, -2463) in 64
 * frames toward (832, 126, -2065), spinning for 34 frames, while his shadow slides in and fades
 * up; landing below 128 he raises dust (gdemoKemuriFlatCreate) and panics ("クロベー慌てる"). At 0x6D6 he
 * stands at (871, 126, -1962) and tells the news (scene runs 0x67, 0x72, 0x69, 0x6B, 0x6C). */
s32 gdemoGenoKuroProc(Obj *obj, s16 msg, void *arg) {
    static f32 vx;       /* .bss 0x08007FC8: his fall, a frame */
    static f32 vy;
    static f32 vz;
    static f32 spin;     /* .bss 0x08007FD4 */
    static f32 shadowVz; /* .bss 0x08007FD8 */
    static s32 alpha;    /* .bss 0x08007FDC: the shadow's */
    KuroW *w = (KuroW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "genoKURO";
            w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->model == NULL) {
                return -1;
            }
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoAnimeSet(w->model, (u32) gdemoKurobeiAnim, 0xBF, 0.0f);
            demoPolSetDL(w->shadow, gdemoShadowDl, 0xBD);
            w->model->flags |= 0x20;
            w->model->scale[2] = 1.2f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 1079.0f;
            w->model->pos[1] = 775.0f;
            w->model->pos[2] = -2463.0f;
            w->shadow->pos[0] = 832.0f;
            w->shadow->pos[1] = 120.0f;
            w->shadow->pos[2] = -1850.0f;
            w->shadow->scale[2] = 2.0f;
            w->shadow->scale[0] = 0.7f;
            w->shadow->scale[1] = 0.7f;
            w->model->rot[1] = 0.0f;
            vy = -(w->model->pos[1] - 126.0f) / 64;
            vx = -(w->model->pos[0] - 832.0f) / 64;
            vz = -(w->model->pos[2] + 2065.0f) / 64;
            spin = 0.36651915f;
            shadowVz = -(w->shadow->pos[2] + 2065.0f) / 30.0f;
            w->timer = 0;
            w->state = 0;
            alpha = 0;
            break;
        case 1:
            frmDestroy(&w->model->f);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, alpha);
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
                    w->model->pos[0] += vx;
                    w->model->pos[1] += vy;
                    w->model->pos[2] += vz;
                    if (w->timer < 0x22) {
                        w->model->rot[0] += spin;
                    } else {
                        w->model->rot[0] = 0.0f;
                        w->shadow->pos[2] += shadowVz;
                        if (alpha < 0xD0) {
                            alpha += 6;
                        }
                        demoAnimeStep(w->model, 0.15f, 2);
                    }
                    if (w->timer == 0x22) {
                        demoAnimeSet(w->model, (u32) gdemoKurobeiState0Anim, 0xBF, 0.0f);
                    }
                    if (w->model->pos[1] < 128.0f) {
                        w->model->pos[1] = 126.0f;
                        w->model->rot[0] = 0.0f;
                        w->state = 1;
                        (*gdemoKemuriFlatCreate)(w->model->pos[0], w->model->pos[1] - 35.0f, w->model->pos[2]);
                        DPRINTF("クロベー慌てる\n");
                    }
                    break;
                case 1:
                    if (demoAnimeStep(w->model, 0.11f, 2)) {
                        demoAnimeSet(w->model, (u32) gdemoKurobeiState1Anim, 0xBF, 0.0f);
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (demoAnimeStep(w->model, 0.1f, 2)) {
                        demoAnimeSet(w->model, (u32) gdemoKurobeiState2Anim, 0xBF, 0.0f);
                        w->state = 3;
                    }
                    break;
                case 3:
                    if (gdemoFrame == 0x6D6) {
                        w->state = 4;
                        demoAnimeSet(w->model, (u32) gdemoKurobeiState3Anim, 0xBF, 0.0f);
                        DPRINTF("クロベー「イイイインパクトトト・・おおおおみつさんんん・・・・。」\n");
                        scRunEnd();
                        scRunStart(0x67, 0, 0);
                        w->model->pos[0] = 871.0f;
                        w->model->pos[1] = 126.0f;
                        w->model->pos[2] = -1962.0f;
                        w->shadow->pos[0] = w->model->pos[0];
                        w->shadow->pos[1] = 77.0f;
                        w->shadow->pos[2] = w->model->pos[2];
                        w->shadow->scale[1] = 0.8f;
                        w->shadow->scale[0] = w->shadow->scale[1];
                        w->shadow->scale[2] = 1.5f;
                        w->shadow->rot[1] = 3.1415927f;
                        w->shadow->rot[0] = 0.12217305f;
                        demoPolSetDL(w->shadow, gdemoShadowDl, 0xBD);
                    } else {
                        demoAnimeStep(w->model, 0.1f, 0);
                    }
                    break;
                case 4:
                    if (gdemoFrame >= 0x817) {
                        if (demoAnimeStep(w->model, 0.2f, 2)) {
                            w->state = 5;
                            demoAnimeSet(w->model, (u32) gdemoKurobeiState4Anim, 0xBF, 0.0f);
                            DPRINTF("クロベー落ち着く\n");
                            scRunEnd();
                            scRunStart(0x72, 0, 0);
                        }
                    } else {
                        demoAnimeStep(w->model, 0.7f, 0);
                    }
                    break;
                case 5:
                    if (demoAnimeStep(w->model, 0.1f, 2)) {
                        w->state = 6;
                        demoAnimeSet(w->model, (u32) gdemoKurobeiState5Anim, 0xBF, 0.0f);
                    }
                    break;
                case 6:
                    if (demoAnimeStep(w->model, 0.15f, 2)) {
                        demoAnimeSet(w->model, (u32) gdemoKurobeiState6Anim, 0xBF, 0.0f);
                        w->state = 7;
                        DPRINTF("クロベー:「ビスマルにおみつさんが連れ去られました。」%d\n", gdemoFrame);
                        scRunEnd();
                        scRunStart(0x69, 0, 0);
                    }
                    break;
                case 7:
                    if (gdemoFrame == 0x9DA) {
                        w->state = 8;
                    }
                    demoAnimeStep(w->model, 0.65f, 0);
                    break;
                case 8:
                    if (gdemoFrame == 0xA3C) {
                        w->state = 9;
                        DPRINTF("クロベー「さらにインパクトさんの頭に変な装置を付けて操っている様なのです」\n");
                        scRunEnd();
                        scRunStart(0x6B, 0, 0);
                    }
                    demoAnimeStep(w->model, 0.1f, 0);
                    break;
                case 9:
                    if (gdemoFrame == 0xBCC) {
                        w->state = 10;
                    }
                    demoAnimeStep(w->model, 0.5f, 0);
                    break;
                case 10:
                    if (gdemoFrame == 0xBFE) {
                        w->state = 11;
                        DPRINTF("クロベー「おみつさんはその装置の中に閉じこめられていると聞きました。」\n");
                        scRunEnd();
                        scRunStart(0x6C, 0, 0);
                    }
                    demoAnimeStep(w->model, 0.1f, 0);
                    break;
                case 11:
                    if (gdemoFrame == 0xD2A) {
                        w->state = 12;
                    }
                    demoAnimeStep(w->model, 0.6f, 0);
                    break;
                case 12:
                    demoAnimeStep(w->model, 0.13f, 0);
                    break;
            }
            w->shadow->scale[2] = gdemoShadowLength * 1.5;
            w->shadow->rot[1] = gdemoShadowAngle;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 gdemoGenoGoeProc(Obj *obj, s16 msg, void *arg);
s32 gdemoGenoEbiProc(Obj *obj, s16 msg, void *arg);
s32 gdemoGenoSasProc(Obj *obj, s16 msg, void *arg);

/* Starts "genoGOE", "genoEBI" and "genoSAS" (Goemon, Ebisumaru, Sasuke) under obj. */
void gdemoGenoTrioCreate(Obj *obj) {
    tskCreate(obj, gdemoGenoGoeProc, obj->file, NULL);
    tskCreate(obj, gdemoGenoEbiProc, obj->file, NULL);
    tskCreate(obj, gdemoGenoSasProc, obj->file, NULL);
}

/* The two "KEMURI" (smoke) tasks' argument: only the position their starters set. */
typedef struct KemuriArg {
    /* 0x00 */ u8 pad00[0x14];
    /* 0x14 */ f32 pos[3];
    /* 0x20 */ u8 pad20[0x10];
} KemuriArg;

/* "KEMURI"'s work: its frame, texture animation, velocity and time. */
typedef struct KemuriW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ Anim anim;
    /* 0x14 */ u8 pad14[0xC];
    /* 0x20 */ f32 vel[3];
    /* 0x2C */ s32 time; /* also the alpha */
} KemuriW;

extern u8 gDemoCamHold;
extern s32 gDemoModelsDelete;
extern u8 gdemoSmokeDl[];               /* the smoke's display list, in file 0x123 */
extern u8 gdemoSmokeTex0[];
extern u8 gdemoSmokeTex1[];
extern u8 gdemoSmokeTex2[];
extern u8 gdemoSmokeTex3[];
extern u8 gdemoSmokeTex4[];
extern u8 gdemoSmokeTex6[];
AnimFrame gdemoKemuriAnim[] = { /* its texture animation */
    { (u32) gdemoSmokeTex0, 8 },  { (u32) gdemoSmokeTex1, 8 },  { (u32) gdemoSmokeTex2, 8 },  { (u32) gdemoSmokeTex3, 8 },
    { (u32) gdemoSmokeTex4, 8 },  { 0x03002D30, 10 },      { (u32) gdemoSmokeTex6, 12 }, { 0x03003D30, 13 },
    { 0x03004530, 14 },      { 0x03004D30, 16 },      { 0x03005530, 20 },      { 0x03005D30, 24 },
    { 0, 0 },
};

/* "KEMURI": a smoke puff from file 0x123 (tlb_0324_6's sand dust, sunaEfProc) at a random
 * scale of 6 to 10; it grows by 0.1 a frame and rises ever faster, up to 24 a
 * frame, fading over 240 frames. It stands still while the demo's camera holds. */
s32 gdemoKemuriProc(Obj *obj, s16 msg, void *arg) {
    KemuriW *w = (KemuriW *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "KEMURI";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, (Gfx *) gdemoSmokeDl, 0x123);
            fadeAnimStart(&w->anim, gdemoKemuriAnim, 0, &w->frame->segs[3], floadGetSetFile(0x123, 0));
            w->frame->flags |= 7;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = (fadeRandRange(-20, 20) + 80.0f) / 10.0f;
            w->frame->pos[0] = ((KemuriArg *) arg)->pos[0];
            w->frame->pos[1] = ((KemuriArg *) arg)->pos[1];
            w->frame->pos[2] = ((KemuriArg *) arg)->pos[2];
            w->time = 240;
            w->vel[0] = 0.0f;
            w->vel[1] = 0.0f;
            w->vel[2] = 0.0f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = demoColorDL(0x60, 0x10, 5, 0x28, 1, 1, w->time);
            break;
        case 0:
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            if (gDemoModelsDelete == 1) {
                return -1;
            }
            fadeAnimStep(&w->anim);
            w->time--;
            if (w->time <= 0) {
                w->time = 0;
                return -1;
            }
            w->frame->scale[0] += 0.1;
            w->frame->scale[1] += 0.1;
            w->frame->scale[2] += 0.1;
            w->vel[1] += 0.13;
            if (24.0f < w->vel[1]) {
                w->vel[1] = 24.0f;
            }
            w->frame->pos[0] += w->vel[0];
            w->frame->pos[1] += w->vel[1];
            w->frame->pos[2] += w->vel[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 gdemoKemuriProc(Obj *obj, s16 msg, void *arg);

/* A "KEMURI" (gdemoKemuriProc) at x, y, z under the menu's last task. */
void gdemoKemuriCreate(f32 x, f32 y, f32 z) {
    KemuriArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    tskCreate(gDmenuLastObj, gdemoKemuriProc, gDmenuLastObj->file, &arg);
}

AnimFrame gdemoKemuriFlatAnim[] = { /* the second smoke's texture animation */
    { 0x03002D30, 8 },  { (u32) gdemoSmokeTex6, 10 }, { 0x03003D30, 11 }, { 0x03004530, 12 },
    { 0x03004D30, 14 }, { 0x03005530, 18 },      { 0x03005D30, 24 }, { 0, 0 },
};

/* The other "KEMURI": the smoke laid flat (turned -90 degrees about x), widening by 0.05 a frame
 * and thickening by 0.003, without drifting, fading over 144 frames in another colour. */
s32 gdemoKemuriFlatProc(Obj *obj, s16 msg, void *arg) {
    KemuriW *w = (KemuriW *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "KEMURI";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, (Gfx *) gdemoSmokeDl, 0x123);
            fadeAnimStart(&w->anim, gdemoKemuriFlatAnim, 0, &w->frame->segs[3], floadGetSetFile(0x123, 0));
            w->frame->flags |= 0x20;
            w->frame->scale[2] = 1.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = ((KemuriArg *) arg)->pos[0];
            w->frame->pos[1] = ((KemuriArg *) arg)->pos[1];
            w->frame->pos[2] = ((KemuriArg *) arg)->pos[2];
            w->vel[0] = 0.0f;
            w->vel[1] = 0.0f;
            w->vel[2] = 0.0f;
            w->frame->flags |= 7;
            w->time = 0x90;
            w->frame->rot[0] = -1.5707964f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = demoColorDL(0xD0, 0x90, 0x60, 0x50, 0x3D, 0x1E, w->time);
            break;
        case 0:
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            if (gDemoModelsDelete == 1) {
                return -1;
            }
            fadeAnimStep(&w->anim);
            w->time--;
            if (w->time <= 0) {
                w->time = 0;
                return -1;
            }
            w->frame->scale[0] += 0.05;
            w->frame->scale[1] += 0.003;
            w->frame->scale[2] += 0.003;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The other "KEMURI" (gdemoKemuriFlatProc) at x, y, z under the menu's last task. */
void gdemoKemuriFlatCreate(f32 x, f32 y, f32 z) {
    KemuriArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    tskCreate(gDmenuLastObj, gdemoKemuriFlatProc, gDmenuLastObj->file, &arg);
}

/* "dValcan"'s argument (gdemoValcanProc): only what its starter sets. */
typedef struct ValcanArg {
    /* 0x00 */ u8 pad00[0x4];
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ f32 pos[3];
    /* 0x1C */ f32 angle;
    /* 0x20 */ u8 pad20[0x8];
} ValcanArg;

/* "dValcan"'s work. */
typedef struct ValcanW {
    /* 0x00 */ PolFrame *pol;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ u8 pad10[0x14];
    /* 0x24 */ s32 timer;
} ValcanW;

extern u8 gDemoCamHold;
extern Gfx gdemoValcanShellDl[]; /* the shell's display list */

/* "dValcan" (a cannon shell of the menu demo): its display list at 0.4 scale, flying by its
 * velocity and rolling 15 degrees a frame for 60 frames, then gone. It stands still while the
 * demo's camera holds. */
s32 gdemoValcanProc(Obj *obj, s16 msg, void *arg) {
    ValcanW *w;
    s32 pad[4];

    switch (msg) {
        case 2:
            obj->name = "dValcan";
            PRINTF("SD_BEAM\n");
            w = (ValcanW *) FX(obj);
            w->pol = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->pol == NULL) {
                return -1;
            }
            demoPolSetDL(w->pol, gdemoValcanShellDl, 0xBE);
            w->pol->flags |= 2;
            w->pol->scale[2] = 0.4f;
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
            w->pol->pos[0] = ((ValcanArg *) arg)->pos[0];
            w->pol->pos[1] = ((ValcanArg *) arg)->pos[1];
            w->pol->pos[2] = ((ValcanArg *) arg)->pos[2];
            w->vel[0] = ((ValcanArg *) arg)->vel[0];
            w->vel[1] = ((ValcanArg *) arg)->vel[1];
            w->vel[2] = ((ValcanArg *) arg)->vel[2];
            w->timer = 0;
            break;
        case 1:
            frmDestroy((Frame *) ((ValcanW *) FX(obj))->pol);
            break;
        case 0:
            w = (ValcanW *) FX(obj);
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            if (++w->timer > 60) {
                return -1;
            }
            w->pol->rot[2] += 0.261799395058201156;
            w->pol->pos[0] += w->vel[0];
            w->pol->pos[1] += w->vel[1];
            w->pol->pos[2] += w->vel[2];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

extern CamView *gGdemoCamView; /* the menu demo's camera view */

s32 gdemoValcanProc(Obj *obj, s16 msg, void *arg);

/* A "dValcan" under the menu's last task: it flies at 60 a frame along `angle` less 5 degrees,
 * starting six frames' flight out from obj at height 2382, with sound 0x237 there. 2382 is
 * passed as the literal, and pos[1] is set between pos[0] and pos[2]. The game has a dead
 * "addiu v0,v0,0x20" (obj's work address) after both folded loads: the genoimp's work pointer,
 * read twice through its own type (2026-10-17; before, the permuter's FAKE offset local,
 * CHANGELOG (720)). Forms that did not: a PolFrame **w local, unused, read once, or assigned
 * inside the first sum; (obj + 1) for FX. */
void gdemoValcanFire(Obj *obj, f32 angle) {
    ValcanArg arg;
    GenoimpW *w = (GenoimpW *) FX(obj);
    s32 pad;

    arg.vel[0] = sinf(angle - 0.0872664650194003855) * 60.0f;
    arg.vel[2] = cosf(angle - 0.0872664650194003855) * 60.0f;
    arg.vel[1] = 0.0f;
    arg.pos[0] = arg.vel[0] * 6.0f + w->model->pos[0];
    arg.pos[1] = 2382.0f;
    arg.pos[2] = arg.vel[2] * 6.0f + w->model->pos[2];
    SoundLocation2(0x237, gGdemoCamView, arg.pos[0], 2382.0f, arg.pos[2], 100.0f, 8000.0f,
                   gGdemoCamView->fovy);
    arg.angle = angle;
    tskCreate(gDmenuLastObj, gdemoValcanProc, gDmenuLastObj->file, &arg);
}

/* The lightning's work: the ambient and directional lights it flashes, and its clock. */
typedef struct FlashW {
    /* 0x00 */ u8 pad00[0x4];
    /* 0x04 */ AmbFrame *amb;
    /* 0x08 */ u8 pad08[0x4];
    /* 0x0C */ LightFrame *light;
    /* 0x10 */ u8 pad10[0x18];
    /* 0x28 */ s32 timer;
} FlashW;

#define FLASH(obj) ((FlashW *) FX(obj))

extern f32 gdemoShadowLength;    /* the shadows' height scale */
extern f32 gdemoShadowAngle;
/* (its flash count gdemoLightningAFlashes is its own static) */

/* A flash of lightning: in the last 14 frames of each second, every other four frames, and in
 * the last four of every ten, the flash count goes up, the light turns to its next direction and
 * the shadows' height scale jumps (1.12, 1.17); between flashes it falls 0.03 a frame to 1. Odd
 * flashes light in pale gold, even ones in orange. The `= 6` (both branches of gdemoLightningB
 * have it too) is the game's. The flash count is a function static (IDO_NOTES 343). */
void gdemoLightningA(Obj *obj) {
    static s32 gdemoLightningAFlashes = 0;
    FlashW *w = FLASH(obj);

    if (FLASH(obj)->timer % 60 >= 46 && w->timer % 8 >= 4) {
        gdemoLightningAFlashes++;
        if (++gdemoLightDirIndex == 6) {
            gdemoLightDirIndex = 0;
        }
        gdemoShadowLength = 1.12f;
    } else if (w->timer % 10 >= 6) {
        gdemoLightningAFlashes++;
        /* `=` for `==`, as the game has it: the direction always goes back to the first. */
        if (gdemoLightDirIndex = 6) {
            gdemoLightDirIndex = 0;
        }
        gdemoShadowLength = 1.17f;
    }
    w->light->dir[0] = gdemoLightDirX[gdemoLightDirIndex];
    gdemoShadowAngle = 3.1415927f;
    if (!(gdemoLightningAFlashes & 1)) {
        w->light->color[0] = 0xB0;
        w->light->color[1] = 0x70;
        w->light->color[2] = 0x20;
        w->amb->color[0] = 0x40;
        w->amb->color[1] = 0x20;
        w->amb->color[2] = 0x10;
    } else {
        w->light->color[0] = 0xA0;
        w->light->color[1] = 0x65;
        w->light->color[2] = 0x15;
        w->amb->color[0] = 0x40;
        w->amb->color[1] = 0x20;
        w->amb->color[2] = 0x10;
    }
    if (gdemoShadowLength > 1.0) {
        gdemoShadowLength -= 0.03;
    }
}

/* (its flash count gdemoLightningBFlashes is its own static) */

/* gdemoLightningA's other lightning, with its own count, scales 1.12 and 1.2, and
 * brighter colours: odd flashes warm white, even ones pale gold. Both branches have the
 * game's `= 6`. Its flash count is a function static, as gdemoLightningA's. */
void gdemoLightningB(Obj *obj) {
    static s32 gdemoLightningBFlashes = 0;
    FlashW *w = FLASH(obj);

    if (FLASH(obj)->timer % 60 >= 46 && w->timer % 8 >= 4) {
        gdemoLightningBFlashes++;
        if (gdemoLightDirIndex = 6) {
            gdemoLightDirIndex = 0;
        }
        gdemoShadowLength = 1.12f;
    } else if (w->timer % 10 >= 6) {
        gdemoLightningBFlashes++;
        if (gdemoLightDirIndex = 6) {
            gdemoLightDirIndex = 0;
        }
        gdemoShadowLength = 1.2f;
    }
    w->light->dir[0] = gdemoLightDirX[gdemoLightDirIndex];
    gdemoShadowAngle = 3.1415927f;
    if (!(gdemoLightningBFlashes & 1)) {
        w->light->color[0] = 0xF0;
        w->light->color[1] = 0x80;
        w->light->color[2] = 0x50;
        w->amb->color[0] = 0x55;
        w->amb->color[1] = 0x30;
        w->amb->color[2] = 0x10;
    } else {
        w->light->color[0] = 0xE0;
        w->light->color[1] = 0x75;
        w->light->color[2] = 0x45;
        w->amb->color[0] = 0x55;
        w->amb->color[1] = 0x30;
        w->amb->color[2] = 0x10;
    }
    if (gdemoShadowLength > 1.0) {
        gdemoShadowLength -= 0.03;
    }
}

/* "shadow"'s (gdemoShadowProc) argument: only what its starter sets. */
typedef struct ShadowArg {
    /* 0x00 */ u8 pad00[0x4];
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ f32 scale[3];
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ s32 set;    /* its display list's file */
    /* 0x2C */ Gfx *dl;     /* its display list */
    /* 0x30 */ u8 pad30[0x8];
} ShadowArg;

/* "shadow"'s work. */
typedef struct ShadowW {
    /* 0x00 */ PolFrame *pol;
    /* 0x04 */ u8 pad04[0xC];
    /* 0x10 */ f32 scale[3];
} ShadowW;

extern s32 gDemoModelsDelete;

/* "shadow": a display list at the starter's position, scale and rotation, drawn in black; its
 * height is its own scale times gdemoShadowLength. It ends when the demo deletes its models. */
s32 gdemoShadowProc(Obj *obj, s16 msg, void *arg) {
    ShadowW *w;
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "shadow";
            w = (ShadowW *) FX(obj);
            w->pol = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->pol == NULL) {
                return -1;
            }
            demoPolSetDL(w->pol, ((ShadowArg *) arg)->dl, ((ShadowArg *) arg)->set);
            w->pol->flags |= 0x20;
            w->pol->scale[0] = w->scale[0] = ((ShadowArg *) arg)->scale[0];
            w->pol->scale[1] = w->scale[1] = ((ShadowArg *) arg)->scale[1];
            w->pol->scale[2] = w->scale[2] = ((ShadowArg *) arg)->scale[2];
            w->pol->pos[0] = ((ShadowArg *) arg)->pos[0];
            w->pol->pos[1] = ((ShadowArg *) arg)->pos[1];
            w->pol->pos[2] = ((ShadowArg *) arg)->pos[2];
            w->pol->rot[0] = ((ShadowArg *) arg)->rot[0];
            w->pol->rot[1] = ((ShadowArg *) arg)->rot[1];
            w->pol->rot[2] = ((ShadowArg *) arg)->rot[2];
            break;
        case 1:
            frmDestroy((Frame *) ((ShadowW *) FX(obj))->pol);
            break;
        case 3:
            ((ShadowW *) FX(obj))->pol->dl = demoColorDL(1, 1, 1, 1, 1, 1, 0xFF);
            break;
        case 0:
            w = (ShadowW *) FX(obj);
            if (gDemoModelsDelete == 1) {
                PRINTF("モデルを消しました\n");
                return -1;
            }
            w->pol->scale[2] = w->scale[2] * gdemoShadowLength;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "shadow" at x, y, z under obj. */
void gdemoShadowCreate(Obj *obj, f32 x, f32 y, f32 z, f32 a, f32 b, f32 c, f32 d, f32 e, f32 f,
                            s32 m, Gfx *dl) {
    ShadowArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale[0] = a;
    arg.scale[1] = b;
    arg.scale[2] = c;
    arg.rot[0] = d;
    arg.rot[1] = e;
    arg.rot[2] = f;
    arg.set = m;
    arg.dl = dl;
    tskCreate(obj, gdemoShadowProc, obj->file, &arg);
}

extern s32 gdemoFrame; /* the demo's frame counter */

/* The demo's scenarios by frame: 0x63 from 820 to 1000, 0x64 from 1010 to 1252, 0x65 from 1262
 * to 1462, 0x66 from 1580 to 1730; one more ends at 3720. */
void gdemoScenarioStep(void) {
    switch (gdemoFrame) {
        case 820:
            PRINTF("ゴエモン:「イ、インパクト！？」\n");
            PRINTF("エビス丸:「いったいどないしはったんやろ？」\n");
            PRINTF("サスケ  :「これはひどいでゴザル」\n");
            PRINTF("クロベー:「ゴ、ゴエモンさん！」\n");
            scRunStart(0x63, 0, 0);
            break;
        case 1000:
            scRunEnd();
            break;
        case 1010:
            scRunStart(0x64, 0, 0);
            break;
        case 1252:
            scRunEnd();
            break;
        case 1262:
            scRunStart(0x65, 0, 0);
            break;
        case 1462:
            scRunEnd();
            break;
        case 1580:
            scRunStart(0x66, 0, 0);
            break;
        case 1730:
            scRunEnd();
            break;
        case 3720:
            scRunEnd();
            break;
    }
}

/* "WATER"'s work: its four layers and their two alphas. */
typedef struct WaterW {
    /* 0x00 */ PolFrame *layer[4];
    /* 0x10 */ s32 alpha[2];
} WaterW;

extern Gfx gdemoWaterLayerDl[]; /* the layers' polygon */
extern u8 gdemoWaterTex[];  /* the water's texture, I4 64x32 */

/* An I4 64x32 texture into TMEM for the render tile, its tile size at (s, t) (in quarter
 * texels): gDPLoadTextureBlock_4b with a scrolling gDPSetTileSize. */
#define gDPLoadWaterBlock(pkt, timg, s, t)                                                              \
    {                                                                                                  \
        gDPSetTextureImage(pkt, G_IM_FMT_I, G_IM_SIZ_16b, 1, timg);                                    \
        gDPSetTile(pkt, G_IM_FMT_I, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, 5, \
                   G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD);                              \
        gDPLoadSync(pkt);                                                                              \
        gDPLoadBlock(pkt, G_TX_LOADTILE, 0, 0, 511, 0x200);                                            \
        gDPPipeSync(pkt);                                                                              \
        gDPSetTile(pkt, G_IM_FMT_I, G_IM_SIZ_4b, 4, 0, G_TX_RENDERTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, 5, \
                   G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD);                              \
        gDPSetTileSize(pkt, G_TX_RENDERTILE, s, t, ((s) + 63) << 2, ((t) + 31) << 2);                  \
    }

/* "WATER" (水面): the castle demo's water, four layers 10 wide at z -4000 turned a quarter, two
 * at the surface and two 100 below. Each layer's texture scrolls in t on its own beat (every 4,
 * 6, 5 and 3 frames); the even layers are tinted orange (prim 0xA08010, env 0xA08020 at alpha
 * 0x90), the odd ones dark (env alpha 0xD0).
 * Its scroll bytes are the file's first .bss (0x08007FB0-0x08007FB7), though it is the last
 * function: their symbol numbers wrap past 256 (IDO_NOTES 337; include/symbol_pad.h). The init chains are written "t0 = s0 = 0" (t first): the game keeps each layer's s
 * in a register and reloads its t after the w0 store. */
s32 gdemoWaterProc(Obj *obj, s16 msg, void *arg) {
    static u8 s0; /* .bss 0x08007FB0: the layers' scroll (s, t) */
    static u8 t0;
    static u8 s1;
    static u8 t1;
    static u8 s2;
    static u8 t2;
    static u8 s3;
    static u8 t3;
    WaterW *w = (WaterW *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "WATER";
            DPRINTF("水面\n");
            w->layer[0] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->layer[0] == NULL) {
                return -1;
            }
            demoPolSetDL(w->layer[0], gdemoWaterLayerDl, 0xBD);
            w->layer[1] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->layer[1] == NULL) {
                return -1;
            }
            demoPolSetDL(w->layer[1], gdemoWaterLayerDl, 0xBD);
            w->layer[2] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->layer[2] == NULL) {
                return -1;
            }
            demoPolSetDL(w->layer[2], gdemoWaterLayerDl, 0xBD);
            w->layer[3] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->layer[3] == NULL) {
                return -1;
            }
            demoPolSetDL(w->layer[3], gdemoWaterLayerDl, 0xBD);
            w->layer[0]->flags |= 0x20;
            w->layer[1]->flags |= 0x20;
            w->layer[2]->flags |= 0x20;
            w->layer[3]->flags |= 0x20;
            w->layer[0]->scale[2] = 10.0f;
            w->layer[0]->scale[1] = w->layer[0]->scale[2];
            w->layer[0]->scale[0] = 10.0f;
            w->layer[1]->scale[2] = 10.0f;
            w->layer[1]->scale[1] = w->layer[1]->scale[2];
            w->layer[1]->scale[0] = 10.0f;
            w->layer[2]->scale[2] = 10.0f;
            w->layer[2]->scale[1] = w->layer[2]->scale[2];
            w->layer[2]->scale[0] = 10.0f;
            w->layer[3]->scale[2] = 10.0f;
            w->layer[3]->scale[1] = w->layer[3]->scale[2];
            w->layer[3]->scale[0] = 10.0f;
            w->layer[0]->pos[0] = w->layer[2]->pos[0] = 0.0f;
            w->layer[0]->pos[1] = 0.0f;
            w->layer[2]->pos[1] = -1.0f;
            w->layer[0]->pos[2] = w->layer[2]->pos[2] = -4000.0f;
            w->layer[1]->pos[0] = w->layer[3]->pos[0] = 0.0f;
            w->layer[1]->pos[1] = -100.0f;
            w->layer[3]->pos[1] = -101.0f;
            w->layer[1]->pos[2] = w->layer[3]->pos[2] = -4000.0f;
            w->layer[0]->rot[1] = -1.5707964f;
            w->layer[1]->rot[1] = -1.5707964f;
            w->layer[2]->rot[1] = -1.5707964f;
            w->layer[3]->rot[1] = -1.5707964f;
            w->alpha[0] = 0x90;
            w->alpha[1] = 0xD0;
            t0 = s0 = 0;
            t1 = s1 = 0;
            t2 = s2 = 0;
            t3 = s3 = 0;
            break;
        case 1:
            frmDestroy(&w->layer[0]->f);
            frmDestroy(&w->layer[1]->f);
            frmDestroy(&w->layer[2]->f);
            frmDestroy(&w->layer[3]->f);
            break;
        case 3:
            w->layer[0]->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetTexturePersp(gGfxDlHead++, G_TP_PERSP);
            gSPClearGeometryMode(gGfxDlHead++, 0x3F0605);
            gSPSetGeometryMode(gGfxDlHead++, 0x200405);
            gDPSetCombine(gGfxDlHead++, 0x309A61, 0x5536FF7F);
            gDPSetAlphaCompare(gGfxDlHead++, G_AC_THRESHOLD);
            gDPPipeSync(gGfxDlHead++);
            gDPSetBlendColor(gGfxDlHead++, 0, 0, 0, 1);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 0xA0, 0x80, 0x10, 0xFF);
            gDPSetEnvColor(gGfxDlHead++, 0xA0, 0x80, 0x20, w->alpha[0]);
            gDPLoadWaterBlock(gGfxDlHead++, gdemoWaterTex, s0, t0);
            gSPEndDisplayList(gGfxDlHead++);
            w->layer[1]->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetTexturePersp(gGfxDlHead++, G_TP_PERSP);
            gSPClearGeometryMode(gGfxDlHead++, 0x3F0605);
            gSPSetGeometryMode(gGfxDlHead++, 0x200405);
            gDPSetCombine(gGfxDlHead++, 0x309A61, 0x5536FF7F);
            gDPSetAlphaCompare(gGfxDlHead++, G_AC_THRESHOLD);
            gDPPipeSync(gGfxDlHead++);
            gDPSetBlendColor(gGfxDlHead++, 0, 0, 0, 1);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 1, 1, 1, 0xFF);
            gDPSetEnvColor(gGfxDlHead++, 1, 1, 1, w->alpha[1]);
            gDPLoadWaterBlock(gGfxDlHead++, gdemoWaterTex, s1, t1);
            gSPEndDisplayList(gGfxDlHead++);
            w->layer[2]->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetTexturePersp(gGfxDlHead++, G_TP_PERSP);
            gSPClearGeometryMode(gGfxDlHead++, 0x3F0605);
            gSPSetGeometryMode(gGfxDlHead++, 0x200405);
            gDPSetCombine(gGfxDlHead++, 0x309A61, 0x5536FF7F);
            gDPSetAlphaCompare(gGfxDlHead++, G_AC_THRESHOLD);
            gDPPipeSync(gGfxDlHead++);
            gDPSetBlendColor(gGfxDlHead++, 0, 0, 0, 1);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 0xA0, 0x80, 0x10, 0xFF);
            gDPSetEnvColor(gGfxDlHead++, 0xA0, 0x80, 0x20, w->alpha[0]);
            gDPLoadWaterBlock(gGfxDlHead++, gdemoWaterTex, s2, t2);
            gSPEndDisplayList(gGfxDlHead++);
            w->layer[3]->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetTexturePersp(gGfxDlHead++, G_TP_PERSP);
            gSPClearGeometryMode(gGfxDlHead++, 0x3F0605);
            gSPSetGeometryMode(gGfxDlHead++, 0x200405);
            gDPSetCombine(gGfxDlHead++, 0x309A61, 0x5536FF7F);
            gDPSetAlphaCompare(gGfxDlHead++, G_AC_THRESHOLD);
            gDPPipeSync(gGfxDlHead++);
            gDPSetBlendColor(gGfxDlHead++, 0, 0, 0, 1);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 1, 1, 1, 0xFF);
            gDPSetEnvColor(gGfxDlHead++, 1, 1, 1, w->alpha[1]);
            gDPLoadWaterBlock(gGfxDlHead++, gdemoWaterTex, s3, t3);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            if (!(gGameFrameCount & 3)) {
                t0++;
            }
            if (gGameFrameCount % 6 == 0) {
                t1++;
            }
            if (gGameFrameCount % 5 == 0) {
                t2++;
            }
            if (gGameFrameCount % 3 == 0) {
                t3++;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *gdemoWaterCreate(Obj *obj) {
    return tskCreate(obj, gdemoWaterProc, obj->file, NULL);
}
