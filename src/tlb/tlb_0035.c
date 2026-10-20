#include "tlb.h"
#include "tlb/tlb_0035.h"

/* File 35: the game over screen ("end.c": "gov親", "goverMain", "goverInit"). The parent task
 * reads the first two controllers; its child "govmain" builds the scene (the sky, a camera
 * "gcam", the light, the oni on his stand, two spotlights, the "Continue? / Yes / No" pictures)
 * and runs the choice: continue goes back to the atlas (or the title), no goes to the title. */

/* The game's debug print, argument-only: the strings stay in .rodata in source order and no
 * code is left (the repo's PRINTF, "if (0)", moves goverInit's registers and frame). */
#define DPRINTF

/* The parent task "gov親"'s work. */
typedef struct GovParentWork {
    /* 0x00 */ s32 wait; /* frames before a released button counts */
    /* 0x04 */ s32 released; /* A or Start released on either controller */
} GovParentWork;

/* The "govmain" task's work. */
typedef struct GovWork {
    /* 0x00 */ Obj *self;
    /* 0x04 */ Obj *sky; /* "bgrad" */
    /* 0x08 */ Obj *cam; /* "gcam" */
    /* 0x0C */ Obj *fade;
    /* 0x10 */ Obj *alig; /* the ambient light */
    /* 0x14 */ Obj *oni; /* the oni */
    /* 0x18 */ Obj *dai; /* his stand */
    /* 0x1C */ Obj *oni2[2]; /* two more oni after "continue" */
    /* 0x24 */ Obj *dai2[2];
    /* 0x2C */ Obj *dtex[3]; /* the pictures: the question, yes, no */
    /* 0x38 */ Obj *spot[2]; /* the spotlights */
    /* 0x40 */ LightFrame *light;
    /* 0x44 */ s32 flags; /* 2: stopped */
    /* 0x48 */ s32 choice; /* 0 yes, 1 no */
    /* 0x4C */ s32 port; /* the controller that chose */
    /* 0x50 */ u32 tex[5];
    /* 0x64 */ u8 pad64[4];
    /* 0x68 */ s32 state;
    /* 0x6C */ s32 timer;
    /* 0x70 */ s32 frames;
} GovWork; /* size 0x74 */

/* The camera "gcam": main's dracam (camera.c) with a mode word. */
typedef struct GcamWork {
    /* 0x00 */ CamView *view;
    /* 0x04 */ f32 speed;
    /* 0x08 */ f32 fovTarget;
    /* 0x0C */ f32 fovStep;
    /* 0x10 */ f32 farTarget;
    /* 0x14 */ f32 farStep;
    /* 0x18 */ f32 eyeXTarget;
    /* 0x1C */ f32 eyeXStep;
    /* 0x20 */ f32 eyeZTarget;
    /* 0x24 */ f32 eyeZStep;
    /* 0x28 */ f32 eyeYTarget;
    /* 0x2C */ f32 eyeYStep;
    /* 0x30 */ f32 atXTarget;
    /* 0x34 */ f32 atXStep;
    /* 0x38 */ f32 atZTarget;
    /* 0x3C */ f32 atZStep;
    /* 0x40 */ f32 atYTarget;
    /* 0x44 */ f32 atYStep;
    /* 0x48 */ s32 mode; /* message 9's argument, 0 or 4 */
} GcamWork;

/* An "anime" frame's animation player (the oni's). */
typedef struct AnimPlay {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 speed; /* frames per tick */
    /* 0x08 */ f32 accel;
    /* 0x0C */ s32 flags; /* 2 loops, 4 one frame longer, 8 twice as fast */
    /* 0x10 */ s32 status; /* 1 and 2 at an end; 2 cleared while it runs */
} AnimPlay;

/* The oni's work ("oni"). */
typedef struct OniWork {
    /* 0x00 */ AnimPlay anim;
    /* 0x14 */ f32 spin; /* added to rot[1] each tick */
    /* 0x18 */ s32 prev; /* the state before this one */
    /* 0x1C */ s32 state;
    /* 0x20 */ s32 step; /* within the state */
    /* 0x24 */ f32 rate; /* the animations' speed */
    /* 0x28 */ f32 vy;
    /* 0x2C */ s32 count; /* dances finished (state 1) */
    /* 0x30 */ Obj *sd; /* "sdtemp" (state 2) */
} OniWork;

extern u8 goverOniAnim[]; /* the oni's animations (file 0x12D) */
extern u8 goverOniAnim2[];
extern u8 goverOniAnim3[];
extern u8 goverOniAnim4[];
extern u8 goverOniNoStep6Anim[];
extern u8 goverOniNoAnim[];
extern u8 goverOniContinueStep6Anim[];
extern u8 goverOniContinueStep7Anim[];
extern u8 goverOniState3Anim[];

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
AnimFrame goverCharaAnims[] = {
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
Obj *gGoverParent = (Obj *) 0x12345670; /* the parent task */
Obj *gGoverMainTask = (Obj *) 0x12345670; /* govmain */
u16 gGoverButton[4] = { 1234, 1234, 1234, 1234 }; /* the controllers' buttons */
u16 gGoverTrigger[4] = { 1234, 1234, 1234, 1234 }; /* pressed this frame */
u16 gGoverButtonOld[4] = { 0x04D2, 0x04D2, 0x04D2, 0x04D2 }; /* last frame's */
s32 gGoverA48 = 305419888;
s32 gGoverContinueVoices[4] = { 0x13F, 0x13E, 0x140, 0x141 }; /* each character's "continue" voice */
s32 gGoverNoVoices[4] = { 0x143, 0x142, 0x144, 0x145 }; /* and its "no" */
extern Obj *goverCam;

s32 goverMainProc(Obj *obj, s16 msg, void *arg);
void goverMain(Obj *obj);
void goverInit(Obj *obj);
void goverResetGlobals(Obj *obj);
s32 gcam(Obj *obj, s16 msg, void *arg);
void gcamMove(GcamWork *w);
s32 goverIsReleased(void);
void goverSetState(Obj *obj, s32 state);
void goverAnimStart(AnimPlay *a, u32 anim, s32 set, f32 speed, s32 flags);
void goverAnimStep(AnimPlay *a);
s32 goverAnimEnded(AnimPlay *a);
f32 goverAnimLeft(AnimPlay *a);
void goverFadeIn(s32 speed);
void goverFadeOut(s32 speed);
s32 goverOniProc(Obj *obj, s16 msg, void *arg);
void goverOniSetAnim(Obj *obj, u32 anim, f32 speed, s32 flags);
void goverOniContinue(Obj *obj);
void goverOniNo(Obj *obj);
Obj *goverSdTempCreate(Obj *obj);
Obj *goverOniCreate(Obj *obj);
void goverOniSetPos(Obj *obj, f32 x, f32 y, f32 z);
void goverOniSetState(Obj *obj, s32 state);
s32 goverOniGetState(Obj *obj);
s32 goverOniGetStep(Obj *obj);
Obj *goverDaiCreate(Obj *obj);
void goverDaiSetState(Obj *obj, s32 state);
void goverDaiSetPos(Obj *obj, f32 x, f32 y, f32 z);
Obj *goverDtexCreate(Obj *obj, s32 top);
void goverDtexSetTex(Obj *obj, u32 tex0, u32 tex1, s32 w, s32 h);
void goverDtexSetPos(Obj *obj, f32 x, f32 y);
void goverDtexSetPage(Obj *obj, s32 page);
void goverDtexSetState(Obj *obj, s32 state);
Obj *goverSpotCreate(Obj *obj);
void goverSpotSetSwing(Obj *obj, f32 phase, f32 period, f32 swing);
void goverSpotSetPos(Obj *obj, f32 x, f32 y, f32 z);
Obj *goverAligCreate(Obj *obj);
Obj *goverDelaySnd(Obj *obj, s32 time, s32 id);

/* A player's entry in main's table at 0x80088280 (10 bytes each), from its character on; the
 * same entry as include/game.h's Unk88282, with its names. */
typedef struct PlayerSlot {
    /* 0x00 */ s8 chara; /* gPlayer1Chara: -1 for none */
    /* 0x01 */ s8 lives;
    /* 0x02 */ s8 life;
    /* 0x03 */ s8 armor;
    /* 0x04 */ s16 money;
    /* 0x06 */ s8 unused6;
    /* 0x07 */ s8 weaponLevel;
    /* 0x08 */ s8 food;
    /* 0x09 */ u8 unused9;
} PlayerSlot;

u32 gGoverPictures[5] = { 0x0300FC00, 0x0300B100, 0x0300C480, 0x0300D800, 0x0300EA00 }; /* the pictures (segment 3 of file 0x310) */

s32 goverParentProc(Obj *obj, s16 msg, void *arg) {
    GovParentWork *w = (GovParentWork *) (obj + 1);
    s32 i;
    u16 old;
    u16 now;

    switch (msg) {
        case 2:
            obj->name = "gov親";
            gGoverParent = obj;
            gGoverMainTask = tskCreateBlocks(obj, goverMainProc, 2, obj->file, NULL);
            for (i = 0; i < 2; i++) { gGoverButton[i] = gGoverTrigger[i] = gGoverButtonOld[i] = 0; }
            w->released = 0;
            w->wait = 10;
            break;
        case 0:
            for (i = 0; i < 2; i++) {
                gGoverButtonOld[i] = gGoverButton[i];
                /* The game's call returns an int here (its result is masked to a u16). */
                gGoverButton[i] = ((s32 (*)(s32, s32)) joyGetButton)(i, 0);
                gGoverTrigger[i] = gGoverButton[i] ^ (gGoverButtonOld[i] & gGoverButton[i]);
                if (w->wait == 0) {
                    old = gGoverButtonOld[i] & 0x9000;
                    now = gGoverButton[i] & 0x9000;
                    if (old && !now) {
                        w->released = 1;
                    }
                } else {
                    w->wait--;
                }
            }
            break;
        case 1:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 goverIsReleased(void) {
    GovParentWork *w = (GovParentWork *) (gGoverParent + 1);

    return w->released;
}

s32 goverMainProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    GovWork *w = (GovWork *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "govmain";
            goverInit(obj);
            break;
        case 0:
            if (!(w->flags & 2)) {
                goverMain(obj);
                w->frames++;
            }
            break;
        case 1:
            frmDestroy((Frame *) w->light);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* govmain's states: 0 builds the scene, 1 the choice (either controller: the stick or the
 * d-pad moves it, A or Start takes it), 2 "continue" (the oni falls, the voice of the
 * chooser's character), 3 "no", 5 waits for the oni and brings the others, 6 fades out, 9 goes
 * on to the atlas (or the title), 4 and 7 fade out to the title. */
void goverMain(Obj *obj) {
    GovWork *w = (GovWork *) (obj + 1);
    s32 i;
    s32 x;
    s32 best;
    s32 pad;
    s32 decided;

    switch (w->state) {
        case 0:
            floadLoadSet(0x12D);
            goverFadeOut(0);
            goverFadeIn(30);
            camSetEye((Camera *) w->cam, 0.0f, 27.0f, 79.0f);
            camSetAt((Camera *) w->cam, 0.0f, 3.5f, -80.0f);
            w->oni = goverOniCreate(obj);
            goverOniSetState(w->oni, 0);
            w->dai = goverDaiCreate(obj);
            w->spot[0] = goverSpotCreate(obj);
            goverSpotSetSwing(w->spot[0], 1.5707963f, 110.0f, 0.62831855f);
            goverSpotSetPos(w->spot[0], 23.0f, -23.0f, 5.0f);
            w->spot[1] = goverSpotCreate(obj);
            goverSpotSetSwing(w->spot[1], 0.0f, 90.0f, 0.62831855f);
            goverSpotSetPos(w->spot[1], -23.0f, -23.0f, 5.0f);
            w->dtex[0] = goverDtexCreate(obj, 0);
            w->dtex[1] = goverDtexCreate(obj, 0);
            w->dtex[2] = goverDtexCreate(obj, 0);
            for (i = 0; i < 5; i++) {
                w->tex[i] = fadeSegToRam(gGoverPictures[i], 0x310);
            }
            goverDtexSetTex(w->dtex[0], w->tex[0], w->tex[0], 288, 64);
            goverDtexSetPos(w->dtex[0], 16.0f, 10.0f);
            goverDtexSetTex(w->dtex[1], w->tex[1], w->tex[2], 104, 24);
            goverDtexSetPos(w->dtex[1], 36.0f, 128.0f);
            goverDtexSetTex(w->dtex[2], w->tex[3], w->tex[4], 96, 24);
            goverDtexSetPos(w->dtex[2], 192.0f, 128.0f);
            goverDtexSetPage(w->dtex[2], 1);
            w->choice = 0;
            w->port = 0;
            w->timer = 60;
            w->state = 1;
            break;
        case 1:
            best = 0;
            decided = 0;
            for (i = 0; i < 2; i++) {
                x = joyGetStickX(i, 0);
                if (gGoverTrigger[i] & 0x200) {
                    x = -80;
                } else if (gGoverTrigger[i] & 0x100) {
                    x = 80;
                }
                if (fabsf(best) < fabsf(x)) {
                    best = x;
                }
                if (gGoverTrigger[i] & 0x9000) {
                    decided = 1;
                    w->port = i;
                    DPRINTF("goverMain(): selected by controller %d (0-3)\n", i);
                }
            }
            if (best > 50) {
                if (w->choice == 0) {
                    w->choice = 1;
                    goverDtexSetPage(w->dtex[1], 1);
                    goverDtexSetPage(w->dtex[2], 0);
                    sndPlay(0x119);
                }
            } else if (best < -50) {
                if (w->choice == 1) {
                    w->choice = 0;
                    goverDtexSetPage(w->dtex[1], 0);
                    goverDtexSetPage(w->dtex[2], 1);
                    sndPlay(0x119);
                }
            }
            if (decided) {
                if (w->choice == 0) { w->state = 2; } else { w->state = 3; }
            }
            break;
        case 2:
            goverOniSetState(w->oni, 1);
            goverDtexSetState(w->dtex[1], 0);
            w->state = 5;
            if (((PlayerSlot *) &gPlayer1Chara)[w->port].chara != -1) {
                DPRINTF("goverMain(%d): kind:%d\n", w->port, gGameWork.charas[w->port]);
                if (gGameWork.charas[w->port] < 4) {
                    sndPlay((u16) gGoverContinueVoices[gGameWork.charas[w->port]]);
                }
            }
            break;
        case 3:
            goverOniSetState(w->oni, 2);
            goverDtexSetState(w->dtex[2], 0);
            w->state = 10;
            if (((PlayerSlot *) &gPlayer1Chara)[w->port].chara != -1) {
                DPRINTF("goverMain(%d): kind:%d\n", w->port, gGameWork.charas[w->port]);
                if (gGameWork.charas[w->port] < 4) {
                    sndPlay((u16) gGoverNoVoices[gGameWork.charas[w->port]]);
                }
            }
            break;
        case 4:
            goverFadeOut(30);
            sndPlay(10);
            w->timer = 30;
            w->state = 7;
            break;
        case 7:
            if (--w->timer == 0) {
                stageGotoTitle();
            }
            break;
        case 5:
            if (goverOniGetState(w->oni) == 1 && goverOniGetStep(w->oni) == 6) {
                goverDtexSetState(w->dtex[1], 1);
                goverDtexSetState(w->dtex[2], 1);
                w->oni2[0] = goverOniCreate(obj);
                w->oni2[1] = goverOniCreate(obj);
                w->dai2[0] = goverDaiCreate(obj);
                w->dai2[1] = goverDaiCreate(obj);
                goverOniSetPos(w->oni2[0], -25.0f, 40.0f, -32.0f);
                goverOniSetPos(w->oni2[1], 25.0f, 40.0f, -32.0f);
                goverDaiSetPos(w->dai2[0], -25.0f, -20.0f, -32.0f);
                goverDaiSetPos(w->dai2[1], 25.0f, -20.0f, -32.0f);
                goverOniSetState(w->oni2[0], 3);
                goverOniSetState(w->oni2[1], 3);
                goverDaiSetState(w->dai2[0], 0);
                goverDaiSetState(w->dai2[1], 0);
                w->state = 8;
            }
            break;
        case 6:
            goverFadeOut(30);
            sndPlay(10);
            w->state = 9;
            break;
        case 9:
            if (--w->timer == 0) {
                if ((swWorkTest(0x30) && !swWorkTest(0xB4)) || (swWorkTest(7) && !swWorkTest(0x9B))) {
                    stageGotoAtlasFrom(NULL, gGameWork.area, gGameWork.areaStage);
                } else {
                    stageGotoTitleArg1();
                }
                w->state = 10;
            }
            break;
        case 10:
            break;
    }
}

void goverSetState(Obj *obj, s32 state) {
    ((GovWork *) (obj + 1))->state = state;
}

void goverInit(Obj *obj) {
    GovWork *w = (GovWork *) (obj + 1);
    s32 i;
    LightFrame *l;

    DPRINTF("goverInit\n");
    /* The clear stores one byte (the game's bug). A goto loop stays rolled (a for loop is
     * unrolled, or with a FAKE "if (0) {}" swaps s1 and s2); on one line, as a macro would be,
     * its store comes first. */
    i = 0; clear: *(u8 *) w = 0; i += 4; if (i < (s32) sizeof(GovWork)) { goto clear; }
    w->self = obj;
    goverResetGlobals(obj);
    w->flags = 1;
    w->state = 0;
    w->timer = 0;
    w->frames = 0;
    tskCreate(obj, attack_Proc, 0, NULL);
    tskCreate(obj, effect, 0, NULL);
    w->sky = tskCreate(obj, bgrad, 0, NULL);
    camBgradSetLines((Camera *) w->sky, 150, 240);
    camBgradMoveTop((ColorFade *) w->sky, 0, 0, 0);
    camBgradMoveBottom((ColorFade *) w->sky, 64, 64, 64);
    w->fade = fadeCreate(obj);
    tskCreate(obj, scParentProc, 0, NULL);
    w->cam = tskCreate(obj, gcam, obj->file, NULL);
    goverCam = w->cam;
    camSetFov((Camera *) w->cam, 0.69813174f);
    camSetEye((Camera *) w->cam, 0.0f, 100.0f, 100.0f);
    camSetAt((Camera *) w->cam, 0.0f, 0.0f, 0.0f);
    l = w->light = (LightFrame *) frmCreate(gFrmLayers[0], gFrmKindLight);
    l->dir[0] = 30;
    l->dir[1] = -100;
    l->dir[2] = 30;
    l->color[0] = 230;
    l->color[1] = 230;
    l->color[2] = 230;
    fadeLightNormalize(l);
    w->alig = goverAligCreate(obj);
    sndPlay(1);
    goverDelaySnd(obj, 8, 0x92);
    DPRINTF("endInit end\n");
}

void goverResetGlobals(Obj *obj) {
    goverCam = NULL;
    gGoverA48 = 0;
    gRetracesPerFrame = 1;
}

void goverNop0C3C(void) {
}

void goverUnusedStub1(void) {
}

/* The camera "gcam": main's dracam with message 9 setting its mode. */
s32 gcam(Obj *obj, s16 msg, void *arg) {
    s32 pad[2];
    GcamWork *w = (GcamWork *) ((u32) obj + sizeof(Obj));

    switch (msg) {
        case 2:
            obj->name = "gcam";
            w->view = (CamView *) frmCreate(gFrmLayers[0], gFrmKindCamera);
            w->mode = 0;
            if (w->view == NULL) {
                PRINTF("end.c: gcam: error\n");
                return -1;
            }
            camSetSpeed((Camera *) obj, 20.0f);
            camStop((Camera *) obj);
            gGameWork.view = w->view;
            break;
        case 0:
            gcamMove(w);
            break;
        case 9:
            switch (*(s32 *) arg) {
                case 0:
                    w->mode = 0;
                    break;
                case 4:
                    w->mode = 4;
                    break;
            }
            break;
        case 1:
            frmDestroy((Frame *) w->view);
            gGameWork.view = NULL;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

void goverUnusedStub2(void) {
}

void goverUnusedStub3(void) {
}

/* gcam's moves: main's dracam's, the eye's three written with a signed step. */
void gcamMove(GcamWork *w) {
    CamView *view = w->view;

    if (w->fovStep > 0.00001f) {
        if (fabsf(w->fovTarget - w->view->fovy) < w->fovStep) {
            w->view->fovy = w->fovTarget;
            w->fovStep = 0.0f;
        } else if (w->fovTarget < w->view->fovy) {
            view->fovy = view->fovy - w->fovStep;
        } else {
            view->fovy = view->fovy + w->fovStep;
        }
    }
    if (w->eyeXStep > 0.00001f) {
        if (fabsf(w->eyeXTarget - w->view->eye[0]) < w->eyeXStep) {
            view->eye[0] = w->eyeXTarget;
            w->eyeXStep = 0.0f;
        } else {
            view->eye[0] += (w->eyeXTarget < view->eye[0]) ? -w->eyeXStep : w->eyeXStep;
        }
    }
    if (w->eyeYStep > 0.00001f) {
        if (fabsf(w->eyeYTarget - view->eye[1]) < w->eyeYStep) {
            view->eye[1] = w->eyeYTarget;
            w->eyeYStep = 0.0f;
        } else {
            view->eye[1] += (w->eyeYTarget < view->eye[1]) ? -w->eyeYStep : w->eyeYStep;
        }
    }
    if (w->eyeZStep > 0.00001f) {
        if (fabsf(w->eyeZTarget - view->eye[2]) < w->eyeZStep) {
            view->eye[2] = w->eyeZTarget;
            w->eyeZStep = 0.0f;
        } else {
            view->eye[2] += (w->eyeZTarget < view->eye[2]) ? -w->eyeZStep : w->eyeZStep;
        }
    }
    if (w->atXStep > 0.00001f) {
        if (fabsf(w->atXTarget - view->at[0]) < w->atXStep) {
            view->at[0] = w->atXTarget;
            w->atXStep = 0.0f;
        } else if (w->atXTarget < view->at[0]) {
            view->at[0] = view->at[0] - w->atXStep;
        } else {
            view->at[0] = view->at[0] + w->atXStep;
        }
    }
    if (w->atYStep > 0.00001f) {
        if (fabsf(w->atYTarget - view->at[1]) < w->atYStep) {
            view->at[1] = w->atYTarget;
            w->atYStep = 0.0f;
        } else if (w->atYTarget < view->at[1]) {
            view->at[1] = view->at[1] - w->atYStep;
        } else {
            view->at[1] = view->at[1] + w->atYStep;
        }
    }
    if (w->atZStep > 0.00001f) {
        if (fabsf(w->atZTarget - view->at[2]) < w->atZStep) {
            view->at[2] = w->atZTarget;
            w->atZStep = 0.0f;
        } else if (w->atZTarget < view->at[2]) {
            view->at[2] = view->at[2] - w->atZStep;
        } else {
            view->at[2] = view->at[2] + w->atZStep;
        }
    }
    if (fabsf(view->at[0] - view->eye[0]) < 1.0f && fabsf(view->at[2] - view->eye[2]) < 1.0f) {
        view->at[2] += 1.0f;
    }
}

/* Starts an animation (file set `set`) at a speed, from its end if the speed is negative. */
void goverAnimStart(AnimPlay *a, u32 anim, s32 set, f32 speed, s32 flags) {
    a->frame->anim = anim;
    floadGetSetRam((void **) a->frame->segs, set);
    a->speed = speed;
    a->accel = 0.0f;
    if (speed < 0.0f) {
        a->frame->frame = polAnimeLastFrame(a->frame);
    } else {
        a->frame->frame = 0;
    }
    a->status = 0;
    a->flags = flags;
}

/* Steps an animation: at either end it loops (flag 2) or stops there, and sets status 3. */
void goverAnimStep(AnimPlay *a) {
    f32 next = a->speed + a->accel;
    AnimeFrame *f = a->frame;
    s32 last;

    if (a->speed * next <= 0.0f) {
        a->accel = 0.0f;
        a->speed = 0.0f;
    } else {
        a->speed = next;
    }
    if (a->flags & 8) {
        f->frame += a->speed + a->speed;
    } else {
        f->frame += a->speed * 0.5f;
    }
    last = polAnimeLastFrame(f);
    if (a->flags & 4) {
        last = last + 1.0f;
    }
    if (last <= f->frame) {
        if (a->flags & 2) {
            f->frame = f->frame - last;
        } else {
            f->frame = last;
        }
        a->status |= 3;
    } else if (f->frame <= 0.0f) {
        if (a->flags & 2) {
            f->frame = f->frame + last;
        } else {
            f->frame = 0.0f;
        }
        a->status |= 3;
    } else {
        a->status &= ~2;
    }
}

void goverUnusedStub4(void) {
}

/* Whether the animation has reached an end. */
s32 goverAnimEnded(AnimPlay *a) {
    return (a->status & 2) != 0;
}

/* The frames left in the animation. */
f32 goverAnimLeft(AnimPlay *a) {
    return polAnimeLastFrame(a->frame) - a->frame->frame;
}

void goverUnusedStub5(void) {
}

void goverUnusedStub6(void) {
}

void goverUnusedStub7(void) {
}

void goverUnusedStub8(void) {
}

/* Fades in from black. */
void goverFadeIn(s32 speed) {
    fadeSetAll(speed, 255.0f, 0.0f, 0, 0, 0);
}

/* Fades out to black. */
void goverFadeOut(s32 speed) {
    fadeSetAll(speed, 0.0f, 255.0f, 0, 0, 0);
}

/* The oni ("oni"): an anime frame of file 0x12D. States 4-7 dance in a loop, 1 and 2 are the
 * answers ("continue": he cheers; "no": he laughs), 3 falls him from above onto his stand. */
s32 goverOniProc(Obj *obj, s16 msg, void *arg) {
    OniWork *w = (OniWork *) (obj + 1);
    AnimeFrame *fr;
    s32 pad[2];
    f32 f;

    switch (msg) {
        case 2:
            obj->name = "oni";
            w->anim.frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->anim.frame == NULL) {
                return -1;
            }
            goverAnimStart(&w->anim, (u32) goverOniAnim, 0x12D, 0.0f, 1);
            fr = w->anim.frame;
            fr->scale[0] = 0.1f;
            fr->scale[1] = 0.1f;
            fr->scale[2] = 0.1f;
            fr->pos[0] = 0.0f;
            fr->pos[1] = 0.0f;
            fr->pos[2] = 0.0f;
            w->prev = 9;
            w->state = 9;
            w->spin = 0.0f;
            w->rate = 0.3f;
            w->sd = NULL;
            break;
        case 1:
            frmDestroy((Frame *) w->anim.frame);
            break;
        case 0:
            goverAnimStep(&w->anim);
            switch (w->state) {
                case 0:
                    goverOniSetAnim(obj, (u32) goverOniAnim, w->rate, 1);
                    w->spin = 0.015707964f;
                    goverOniSetState(obj, 4);
                    break;
                case 4:
                    f = goverAnimLeft(&w->anim);
                    if (f < 1.0f) {
                        goverOniSetAnim(obj, (u32) goverOniAnim2, w->rate, 1);
                        w->anim.frame->frame = 1.0f - f;
                        goverOniSetState(obj, 5);
                    }
                    break;
                case 5:
                    f = goverAnimLeft(&w->anim);
                    if (f < 1.0f) {
                        goverOniSetAnim(obj, (u32) goverOniAnim3, w->rate, 1);
                        w->anim.frame->frame = 1.0f - f;
                        goverOniSetState(obj, 6);
                    }
                    break;
                case 6:
                    f = goverAnimLeft(&w->anim);
                    if (f < 1.0f) {
                        goverOniSetAnim(obj, (u32) goverOniAnim4, w->rate, 1);
                        w->anim.frame->frame = 1.0f - f;
                        goverOniSetState(obj, 7);
                    }
                    break;
                case 7:
                    f = goverAnimLeft(&w->anim);
                    if (f < 1.0f) {
                        goverOniSetAnim(obj, (u32) goverOniAnim, w->rate, 1);
                        w->anim.frame->frame = 1.0f - f;
                        goverOniSetState(obj, 4);
                    }
                    break;
                case 1:
                    goverOniContinue(obj);
                    break;
                case 2:
                    goverOniNo(obj);
                    break;
                case 3:
                    w->vy = -1.0f;
                    goverOniSetAnim(obj, (u32) goverOniState3Anim, 0.0f, 1);
                    goverOniSetState(obj, 8);
                    break;
                case 8:
                    w->anim.frame->pos[1] += w->vy;
                    if (w->anim.frame->pos[1] < 0.0f) {
                        w->anim.frame->pos[1] = 0.0f;
                        w->rate = 0.6f;
                        goverOniSetState(obj, 0);
                    }
                    break;
                case 9:
                    break;
            }
            w->anim.frame->rot[1] += w->spin;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *goverOniCreate(Obj *obj) {
    return tskCreate(obj, goverOniProc, obj->file, NULL);
}

void goverOniSetPos(Obj *obj, f32 x, f32 y, f32 z) {
    AnimeFrame *f = ((OniWork *) (obj + 1))->anim.frame;

    f->pos[0] = x;
    f->pos[1] = y;
    f->pos[2] = z;
}

void goverOniSetAnim(Obj *obj, u32 anim, f32 speed, s32 flags) {
    goverAnimStart((AnimPlay *) (obj + 1), anim, 0x12D, speed, flags);
}

void goverOniSetState(Obj *obj, s32 state) {
    OniWork *w = (OniWork *) (obj + 1);

    w->prev = w->state;
    w->state = state;
    w->step = 0;
}

s32 goverOniGetState(Obj *obj) {
    return ((OniWork *) (obj + 1))->state;
}

s32 goverOniGetStep(Obj *obj) {
    return ((OniWork *) (obj + 1))->step;
}

void goverUnusedStub9(void) {
}

/* The oni's "continue" (state 1): from his dance into the cheer, five times over; A or Start
 * hurries it. Then govmain goes on (state 6). */
void goverOniContinue(Obj *obj) {
    OniWork *w = (OniWork *) (obj + 1);
    s32 pad[2];
    f32 f;
    s32 i;

    w->anim.flags &= ~8;
    switch (w->step) {
        case 0:
            w->rate = 0.3f;
            w->count = 0;
            switch (w->prev) {
                case 0:
                    w->step = 1;
                    break;
                case 4:
                    w->step = 2;
                    break;
                case 5:
                    w->step = 3;
                    break;
                case 6:
                    w->step = 4;
                    break;
                case 7:
                    w->step = 5;
                    break;
                default:
                    w->step = 1;
                    break;
            }
            break;
        case 1:
            goverOniSetAnim(obj, (u32) goverOniAnim, w->rate, 1);
            w->step = 2;
            break;
        case 2:
            f = goverAnimLeft(&w->anim);
            if (f < 1.0f) {
                goverOniSetAnim(obj, (u32) goverOniAnim2, w->rate, 1);
                w->anim.frame->frame = 1.0f - f;
                w->step = 3;
            }
            break;
        case 3:
            f = goverAnimLeft(&w->anim);
            if (f < 1.0f) {
                goverOniSetAnim(obj, (u32) goverOniAnim3, w->rate, 1);
                w->anim.frame->frame = 1.0f - f;
                w->step = 4;
            }
            break;
        case 4:
            f = goverAnimLeft(&w->anim);
            if (f < 1.0f) {
                goverOniSetAnim(obj, (u32) goverOniAnim4, w->rate, 1);
                w->anim.frame->frame = 1.0f - f;
                w->step = 5;
            }
            break;
        case 5:
            f = goverAnimLeft(&w->anim);
            if (f < 1.0f) {
                goverOniSetAnim(obj, (u32) goverOniAnim, w->rate, 1);
                w->anim.frame->frame = 1.0f - f;
                w->step = 6;
            }
            break;
        case 6:
            goverOniSetAnim(obj, (u32) goverOniContinueStep6Anim, 0.6f, 1);
            w->step = 7;
            break;
        case 7:
            if (goverAnimEnded(&w->anim)) {
                goverOniSetAnim(obj, (u32) goverOniContinueStep7Anim, 1.0f, 6);
                w->step = 8;
            }
            break;
        case 8:
            if (goverAnimEnded(&w->anim)) {
                if (++w->count == 5) {
                    goverSetState(gGoverMainTask, 6);
                }
            }
            break;
        case 9:
            break;
    }
    if (w->rate < 0.8f) {
        w->rate += 0.01f;
        if (w->rate > 0.8f) {
            w->rate = 0.8f;
        }
        w->anim.speed = w->rate;
    }
    if (goverIsReleased()) {
        for (i = 0; i < 2; i++) {
            if (gGoverButton[i] & 0x9000) {
                w->anim.flags |= 8;
            }
        }
    }
}

/* The oni's "no" (state 2): from his dance into the laugh, the music speeding up ("sdtemp")
 * until it ends; then govmain fades to the title (state 4). */
void goverOniNo(Obj *obj) {
    OniWork *w = (OniWork *) (obj + 1);
    s32 pad[3];
    f32 f;
    s32 i;

    w->anim.flags &= ~8;
    switch (w->step) {
        case 0:
            w->sd = goverSdTempCreate(obj);
            w->rate = 0.3f;
            switch (w->prev) {
                case 0:
                    w->step = 1;
                    break;
                case 4:
                    w->step = 2;
                    break;
                case 5:
                    w->step = 3;
                    break;
                case 6:
                    w->step = 4;
                    break;
                case 7:
                    w->step = 5;
                    break;
                default:
                    w->step = 1;
                    break;
            }
            break;
        case 1:
            goverOniSetAnim(obj, (u32) goverOniAnim, w->rate, 1);
            w->step = 2;
            break;
        case 2:
            f = goverAnimLeft(&w->anim);
            if (f < 1.0f) {
                goverOniSetAnim(obj, (u32) goverOniAnim2, w->rate, 1);
                w->anim.frame->frame = 1.0f - f;
                w->step = 3;
            }
            break;
        case 3:
            f = goverAnimLeft(&w->anim);
            if (f < 1.0f) {
                goverOniSetAnim(obj, (u32) goverOniAnim3, w->rate, 1);
                w->anim.frame->frame = 1.0f - f;
                w->step = 4;
            }
            break;
        case 4:
            f = goverAnimLeft(&w->anim);
            if (f < 1.0f) {
                goverOniSetAnim(obj, (u32) goverOniAnim4, w->rate, 1);
                w->anim.frame->frame = 1.0f - f;
                w->step = 5;
            }
            break;
        case 5:
            f = goverAnimLeft(&w->anim);
            if (f < 1.0f) {
                goverOniSetAnim(obj, (u32) goverOniAnim, w->rate, 1);
                w->anim.frame->frame = 1.0f - f;
                w->step = 6;
            }
            break;
        case 6:
            goverOniSetAnim(obj, (u32) goverOniNoStep6Anim, 0.3f, 1);
            sndPlay(0x102);
            w->sd = tskDestroy(w->sd);
            w->step = 7;
            break;
        case 7:
            if (goverAnimEnded(&w->anim)) {
                goverOniSetAnim(obj, (u32) goverOniNoAnim, 0.3f, 1);
                w->step = 8;
            }
            break;
        case 8:
            if (goverAnimEnded(&w->anim)) {
                w->step = 9;
            }
            break;
        case 9:
            goverSetState(gGoverMainTask, 4);
            w->step = 12;
            break;
        case 11:
            goverOniSetAnim(obj, (u32) goverOniNoAnim, 0.0f, 1);
            w->anim.frame->frame = polAnimeLastFrame(w->anim.frame);
            w->step = 9;
            break;
        case 12:
            break;
    }
    if (w->rate < 0.8f) {
        w->rate += 0.01f;
        if (w->rate > 0.8f) {
            w->rate = 0.8f;
        }
        w->anim.speed = w->rate;
    }
    if (w->state != 12 && goverIsReleased()) {
        for (i = 0; i < 2; i++) {
            if (gGoverButton[i] & 0x9000) {
                w->anim.flags |= 8;
            }
        }
    }
}

/* "sdtemp": the tempo-up sound every 36 frames while the oni laughs. */
typedef struct SdTempWork {
    /* 0x00 */ s32 wait;
} SdTempWork;

s32 goverSdTempProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    SdTempWork *w = (SdTempWork *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "sdtemp";
            w->wait = 10;
            break;
        case 0:
            if (--w->wait <= 0) {
                sndPlay(11);
                DPRINTF("SD_TEMPOUP_SET\n");
                w->wait = 36;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *goverSdTempCreate(Obj *obj) {
    return tskCreate(obj, goverSdTempProc, obj->file, NULL);
}

/* The oni's stand ("台"): a pol frame of file 0x12D, turning; state 0 raises it from below. */
typedef struct DaiWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ s32 state; /* 0 starts the rise, 1 rises, 2 stays */
} DaiWork;

extern u8 goverDaiDl[]; /* the stand's display list */

s32 goverDaiProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    DaiWork *w = (DaiWork *) (obj + 1);
    PolFrame *fr;

    switch (msg) {
        case 2:
            obj->name = "台";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            fr = w->frame;
            demoPolSetDL(fr, (Gfx *) goverDaiDl, 0x12D);
            fr->scale[0] = 0.012f;
            fr->scale[1] = 0.012f;
            fr->scale[2] = 0.012f;
            fr->pos[2] = fr->pos[1] = fr->pos[0] = 0.0f;
            w->state = 2;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 0:
            fr = w->frame;
            switch (w->state) {
                case 0:
                    w->state = 1;
                    break;
                case 1:
                    fr->pos[1] += 0.5f;
                    if (fr->pos[1] >= 0.0f) {
                        fr->pos[1] = 0.0f;
                        w->state = 2;
                    }
                    break;
                case 2:
                    break;
            }
            fr->rot[1] += 0.015707964f;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *goverDaiCreate(Obj *obj) {
    return tskCreate(obj, goverDaiProc, obj->file, NULL);
}

void goverDaiSetState(Obj *obj, s32 state) {
    ((DaiWork *) (obj + 1))->state = state;
}

void goverDaiSetPos(Obj *obj, f32 x, f32 y, f32 z) {
    PolFrame *f = ((DaiWork *) (obj + 1))->frame;

    f->pos[0] = x;
    f->pos[1] = y;
    f->pos[2] = z;
}

/* Draws an RGBA16 picture a row at a time (each row a 1-texel-high block load), bottom row
 * first, from (x, y). */
Gfx *goverDrawRows(Gfx *gfx, f32 x, f32 y, s32 width, s32 rows, u32 img) {
    s32 i;
    s32 step = width * 2;

    y += rows - 1;
    gDPPipeSync(gfx++);
    for (i = 0; i < rows; i++) {
        gDPLoadTextureBlock(gfx++, img, G_IM_FMT_RGBA, G_IM_SIZ_16b, width, 1, 0, G_TX_CLAMP, G_TX_CLAMP,
                            G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        gSPTextureRectangle(gfx++, (s32) (x * 4.0f), (s32) (y * 4.0f), (s32) ((width + x) * 4.0f),
                            (s32) ((y + 1.0f) * 4.0f), G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
        img += step;
        y -= 1.0f;
    }
    return gfx;
}

/* The "dtex" task's work: a picture (two pages, flipped each frame while it flashes). */
typedef struct DtexWork {
    /* 0x00 */ DLFrame *frame;
    /* 0x04 */ f32 x;
    /* 0x08 */ f32 y;
    /* 0x0C */ u32 tex[2];
    /* 0x14 */ s32 page;
    /* 0x18 */ s32 w;
    /* 0x1C */ s32 h;
    /* 0x20 */ s32 alpha;
    /* 0x24 */ s32 state; /* 0 flashes, 1 flashes and fades out, 2 stays */
} DtexWork;

Gfx gGoverPicSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetTextureFilter(G_TF_POINT),
    gsDPSetCombineLERP(0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0),
    gsDPPipeSync(),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsSPEndDisplayList(),
    gsDPNoOp(),
}; /* the pictures' setup */

/* "dtex" (as the staff roll's, file 36): with a non-NULL argument it goes on layer 2. */
s32 goverDtexProc(Obj *obj, s16 msg, void *arg) {
    s32 pad[6];
    DtexWork *w = (DtexWork *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "dtex";
            w->frame = (DLFrame *) frmCreate(arg != NULL ? gFrmLayer2 : gFrmLayers[0], gFrmKindDlist);
            if (w->frame == NULL) {
                return -1;
            }
            w->page = 0;
            w->alpha = 0xFF;
            w->state = 2;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->page ^= 1;
                    break;
                case 1:
                    w->page ^= 1;
                    w->alpha -= 5;
                    if (w->alpha <= 0) {
                        w->alpha = 0;
                        w->state = 2;
                    }
                    break;
                case 2:
                    break;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, fadeSegToRam(gGoverPicSetupDl, 0x23));
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            if (w->alpha >= 0xFF) {
                gDPSetRenderMode(gGfxDlHead++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
            } else {
                gDPSetRenderMode(gGfxDlHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
            }
            gGfxDlHead = goverDrawRows(gGfxDlHead, w->x, w->y, w->w, w->h, w->tex[w->page]);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *goverDtexCreate(Obj *obj, s32 top) {
    return tskCreate(obj, goverDtexProc, obj->file, (void *) top);
}

void goverDtexSetTex(Obj *obj, u32 tex0, u32 tex1, s32 w, s32 h) {
    DtexWork *d = (DtexWork *) (obj + 1);

    d->tex[0] = tex0;
    d->tex[1] = tex1;
    d->w = w;
    d->h = h;
}

void goverDtexSetPos(Obj *obj, f32 x, f32 y) {
    DtexWork *d = (DtexWork *) (obj + 1);

    d->x = x;
    d->y = y;
}

void goverDtexSetPage(Obj *obj, s32 page) {
    ((DtexWork *) (obj + 1))->page = page;
}

void goverDtexSetState(Obj *obj, s32 state) {
    ((DtexWork *) (obj + 1))->state = state;
}

/* A spotlight ("SPOT"): a pol frame of file 0x12D swinging about z, drawn translucent in its
 * prim and env colours. */
typedef struct SpotWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad4[4];
    /* 0x08 */ s32 primR;
    /* 0x0C */ s32 primG;
    /* 0x10 */ s32 primB;
    /* 0x14 */ s32 envR;
    /* 0x18 */ s32 envG;
    /* 0x1C */ s32 envB;
    /* 0x20 */ s32 alpha;
    /* 0x24 */ f32 t;
    /* 0x28 */ f32 step;
    /* 0x2C */ f32 base;
    /* 0x30 */ f32 swing;
} SpotWork;

extern u8 goverSpotDl[]; /* the spotlight's display list */

s32 goverSpotProc(Obj *obj, s16 msg, void *arg) {
    SpotWork *w = (SpotWork *) (obj + 1);
    s32 pad;
    PolFrame *f;
    PolFrame *fr;

    switch (msg) {
        case 2:
            obj->name = "SPOT";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            f = w->frame;
            demoPolSetDL(f, (Gfx *) goverSpotDl, 0x12D);
            f->scale[0] = 0.015f;
            f->scale[1] = 0.015f;
            f->scale[2] = 0.015f;
            f->pos[0] = 0.0f;
            f->pos[1] = 0.0f;
            f->pos[2] = 0.0f;
            w->primR = 200;
            w->primG = 80;
            w->primB = 80;
            w->envR = 200;
            w->envG = 80;
            w->envB = 80;
            w->alpha = 160;
            w->t = 0.0f;
            w->step = 0.0f;
            w->base = 0.0f;
            w->swing = 0.7853982f;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 0:
            fr = w->frame;
            w->t += w->step;
            fr->rot[2] = mathSin(w->t) * w->swing + w->base;
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCycleType(gGfxDlHead++, G_CYC_1CYCLE);
            gDPSetRenderMode(gGfxDlHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->primR, w->primG, w->primB, w->alpha);
            gDPSetEnvColor(gGfxDlHead++, w->envR, w->envG, w->envB, w->alpha);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0,
                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *goverSpotCreate(Obj *obj) {
    return tskCreate(obj, goverSpotProc, obj->file, NULL);
}

/* The swing: about `base`, a half turn each `period` frames, `swing` either way. */
void goverSpotSetSwing(Obj *obj, f32 base, f32 period, f32 swing) {
    SpotWork *w = (SpotWork *) (obj + 1);

    w->base = base;
    w->step = 3.1415927f / period;
    w->swing = swing;
}

void goverSpotSetPos(Obj *obj, f32 x, f32 y, f32 z) {
    PolFrame *f = ((SpotWork *) (obj + 1))->frame;

    f->pos[0] = x;
    f->pos[1] = y;
    f->pos[2] = z;
}

/* The ambient light ("alig"): its colour moves by a step each frame, clamped to 0-255. State 0
 * sets grey 128, 1 darkens, 2 brightens, 3 holds. */
typedef struct AligWork {
    /* 0x00 */ AmbFrame *frame;
    /* 0x04 */ s32 state;
    /* 0x08 */ s32 r;
    /* 0x0C */ s32 g;
    /* 0x10 */ s32 b;
    /* 0x14 */ s32 dr;
    /* 0x18 */ s32 dg;
    /* 0x1C */ s32 db;
} AligWork;

#define CLAMP255(x) ((x) >= 0xFF ? 0xFF : (x) <= 0 ? 0 : (x))

s32 goverAligProc(Obj *obj, s16 msg, void *arg) {
    AligWork *w = (AligWork *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "alig";
            w->frame = (AmbFrame *) frmCreate(gFrmLayers[0], gFrmKindAmbi);
            w->dr = w->dg = w->db = 0;
            w->r = w->g = w->b = 60;
            w->frame->color[0] = w->r;
            w->frame->color[1] = w->g;
            w->frame->color[2] = w->b;
            w->state = 3;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->r = w->g = w->b = 0x80;
                    w->dr = w->dg = w->db = 0;
                    w->state = 3;
                    break;
                case 1:
                    w->dr = w->dg = w->db = -2;
                    break;
                case 2:
                    w->dr = w->dg = w->db = 2;
                    break;
                case 3:
                case 4:
                    break;
            }
            w->r += w->dr;
            w->g += w->dg;
            w->b += w->db;
            w->frame->color[0] = w->r = CLAMP255(w->r);
            w->frame->color[1] = w->g = CLAMP255(w->g);
            w->frame->color[2] = w->b = CLAMP255(w->b);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *goverAligCreate(Obj *obj) {
    return tskCreate(obj, goverAligProc, obj->file, NULL);
}

void goverUnusedStub10(void) {
}

/* A sound after a delay: plays `id` after `time` frames, then ends. */
typedef struct DelaySndWork {
    /* 0x00 */ s32 time;
    /* 0x04 */ s32 id;
} DelaySndWork;

s32 goverDelaySndProc(Obj *obj, s16 msg, void *arg) {
    DelaySndWork *w = (DelaySndWork *) (obj + 1);

    if (msg == 0) {
        if (w->time-- <= 0) {
            sndPlay(w->id);
            return -1;
        }
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *goverDelaySnd(Obj *obj, s32 time, s32 id) {
    Obj *t = tskCreate(obj, goverDelaySndProc, obj->file, NULL);

    if (t != NULL) {
        ((DelaySndWork *) (t + 1))->time = time;
        ((DelaySndWork *) (t + 1))->id = id;
    }
    return t;
}
