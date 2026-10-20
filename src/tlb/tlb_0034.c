#include "tlb.h"
#include "tlb/tlb_0034.h"

/* File 34: the ending ("end.c": "end親", "endmain", "endInit"). The parent task reads the four
 * controllers; its child "endmain" builds the scene (the sky, the camera "gcam", the light, the
 * four players, the sliding door "襖", the castle, the trees, the impact) and runs it. */

/* The game's debug print, argument-only: the strings stay in .rodata in source order. */
#define DPRINTF

/* The "endmain" task's work. */
typedef struct EndWork {
    /* 0x00 */ Obj *self;
    /* 0x04 */ Obj *sky; /* "bgrad" */
    /* 0x08 */ Obj *cam; /* "gcam" */
    /* 0x0C */ Obj *fade;
    /* 0x10 */ Obj *alig; /* the ambient light */
    /* 0x14 */ Obj *pl[4]; /* the four characters */
    /* 0x24 */ Obj *plTask[4]; /* each one's control, "pl制御" */
    /* 0x34 */ Obj *jiji; /* the old man, "じじい" */
    /* 0x38 */ Obj *mao; /* "マオ" */
    /* 0x3C */ Obj *bisu; /* "bisu" */
    /* 0x40 */ Obj *fusuma; /* the sliding door, "襖" */
    /* 0x44 */ Obj *room; /* "座敷" */
    /* 0x48 */ Obj *lab; /* "lab" */
    /* 0x4C */ Obj *mac2; /* "mac2" */
    /* 0x50 */ Obj *castle; /* "castle" */
    /* 0x54 */ Obj *impact[2]; /* "impact" */
    /* 0x5C */ Obj *ground; /* "城地面" */
    /* 0x60 */ Obj *gate; /* "城門" */
    /* 0x64 */ Obj *tree[2]; /* "tree" */
    /* 0x6C */ Obj *flash[3]; /* "b閃光" */
    /* 0x78 */ Obj *fball[3]; /* "Fball" */
    /* 0x84 */ LightFrame *light;
    /* 0x88 */ s32 flags; /* 2: stopped */
    /* 0x8C */ f32 boxTop; /* the letterbox's edges as it opens */
    /* 0x90 */ f32 boxBottom;
    /* 0x94 */ s32 state;
    /* 0x98 */ s32 timer;
    /* 0x9C */ s32 frames;
} EndWork; /* size 0xA0 */

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

/* An "anime" frame's animation player. */
typedef struct AnimPlay {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 speed; /* frames per tick */
    /* 0x08 */ f32 accel;
    /* 0x0C */ s32 flags; /* 2 loops */
    /* 0x10 */ s32 status; /* 1 and 2 at an end; 2 cleared while it runs */
} AnimPlay;

/* A pol frame task's work (the stands, the castle, the trees). */
typedef struct DaiWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ s32 state; /* 0 starts the rise, 1 rises, 2 stays */
} DaiWork;

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
AnimFrame endCharaAnims[] = {
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
Obj *endParentTask = (Obj *) 0x12345670; /* the parent task */
Obj *endMainTask = (Obj *) 0x12345670; /* endmain */
u16 endPadButtons[4] = { 1234, 1234, 1234, 1234 }; /* the controllers' buttons */
u16 endPadPressed[4] = { 1234, 1234, 1234, 1234 }; /* pressed this frame */
Obj *endScTask = (Obj *) 0x12345670; /* the "sc制御" task running */
extern Obj *endInitObj; /* endInit's object */
extern Obj *endCam; /* gcam */

s32 endMainProc(Obj *obj, s16 msg, void *arg);
void endMainStep(Obj *obj);
void endInit(Obj *obj);
void endInitGlobals(Obj *obj);
s32 endGcamProc(Obj *obj, s16 msg, void *arg);
void endGcamMove(GcamWork *w);
Obj *endAligCreate(Obj *obj);
Obj *endPlCtlCreate(Obj *obj);

/* "end親": reads the four controllers. The clear is two do-while loops, the buttons' on j and
 * the presses' on i: the game stores the four buttons first, each array under one lui. */
s32 endParentProc(Obj *obj, s16 msg, void *arg) {
    s32 i;
    s32 j;
    u16 old;

    switch (msg) {
        case 2:
            obj->name = "end親";
            endParentTask = obj;
            endMainTask = tskCreateBlocks(obj, endMainProc, 2, 0, NULL);
            j = 0;
            do {
                endPadButtons[j] = 0;
            } while (++j < 4);
            i = 0;
            do {
                endPadPressed[i] = 0;
            } while (++i < 4);
            break;
        case 0:
            for (i = 0; i < 4; i++) {
                old = endPadButtons[i];
                endPadButtons[i] = ((s32 (*)(s32, s32)) joyGetButton)(i, 0);
                endPadPressed[i] = endPadButtons[i] ^ (endPadButtons[i] & old);
            }
            break;
        case 1:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "endmain". */
s32 endMainProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    EndWork *w = (EndWork *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "endmain";
            DPRINTF("INIT\n");
            endInit(obj);
            DPRINTF("INIT end\n");
            break;
        case 0:
            if (!(w->flags & 2)) {
                endMainStep(obj);
                w->frames++;
            }
            break;
        case 1:
            frmDestroy((Frame *) w->light);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

void endPlSetPos(Obj *pl, f32 x, f32 y, f32 z);
void endPlSetRot(Obj *pl, f32 angle);
void endLightSetDir(LightFrame *l, f32 *dir);
void endFadeIn(s32 speed);
void endFadeOut(s32 speed);
Obj *endFusumaCreate(Obj *obj);
void endFusumaOpen(Obj *obj);
void endFusumaOpenDelayed(Obj *obj);
Obj *endJijiCreate(Obj *obj);
void endJijiSetPos(Obj *obj, f32 x, f32 y, f32 z);
void endJijiSetRot(Obj *obj, f32 angle);
void endJijiSetState(Obj *obj, s32 state);
void endJijiHide(Obj *obj);
void endJijiShow(Obj *obj);
Obj *endMaoCreate(Obj *obj);
void endMaoSetPos(Obj *obj, f32 x, f32 y, f32 z);
void endMaoSetRot(Obj *obj, f32 angle);
void endMaoSetState(Obj *obj, s32 state);
Obj *endBisuCreate(Obj *obj);
void endBisuSetPos(Obj *obj, f32 x, f32 y, f32 z);
void endBisuSetRot(Obj *obj, f32 angle);
void endBisuSetState(Obj *obj, s32 state);
void endAligSetState(Obj *obj, s32 state);
Obj *endZashikiCreate(Obj *obj);
Obj *endLabCreate(Obj *obj);
Obj *endMac2Create(Obj *obj);
void endMac2SetState(Obj *obj, s32 state);
void endPlCtlSetState(Obj *obj, s32 state);
void endPlCtlWalkTo(Obj *obj, f32 x, f32 y, f32 z, f32 speed);
void endPlCtlStopAnime(Obj *obj);
void endPlCtlFaceStart(Obj *obj, s32 time, s32 mode, s32 eyes, s32 mouth);
Obj *endFlashCreate(Obj *obj);
void endFlashSetState(Obj *obj, s32 state);
void endFlashSetPos(Obj *obj, f32 x, f32 y, f32 z);
void endFlashSetRot(Obj *obj, f32 x, f32 y, f32 z);
void endFlashSetSpeed(Obj *obj, f32 speed);
Obj *endFballCreate(Obj *obj);
void endFballSetPos(Obj *obj, f32 x, f32 y, f32 z);
Obj *endCastleCreate(Obj *obj);
Obj *endCastleGroundCreate(Obj *obj);
Obj *endCastleGateCreate(Obj *obj);
Obj *endImpactCreate(Obj *obj);
void endImpactSetState(Obj *obj, s32 state);
void endImpactSetPos(Obj *obj, f32 x, f32 y, f32 z);
Obj *endTreeCreate(Obj *obj);
void endTreeSetPos(Obj *obj, f32 x, f32 y, f32 z);
Obj *edScSet(Obj *obj, s32 id, s32 time);
void edScSetTime(Obj *obj, s32 time);
Obj *edSdSet(s32 id, s32 time);

#define PL_HIDDEN 0x02000000 /* Player flags: not drawn */

/* endmain's states: the ending's scenes, one cut after another, each waiting `timer` frames. */
void endMainStep(Obj *obj) {
    s32 pad[2];
    Player *p0;
    Player *p1;
    Player *p2;
    Player *p3;
    EndWork *w = (EndWork *) (obj + 1);
    s32 pad2[3];
    f32 dir[3];
    LightFrame *l;

    if (plIsInPlay(0)) {
        p0 = PL(w->pl[0]);
        p1 = PL(w->pl[1]);
        p2 = PL(w->pl[2]);
        p3 = PL(w->pl[3]);
    }
    switch (w->state) {
        case 0:
            floadLoadSet(0xC0);
            floadLoadSet(0xC2);
            floadLoadSet(0xC1);
            floadLoadSet(0xC5);
            floadLoadSet(0x12B);
            floadLoadSet(0x12A);
            floadLoadSet(0x12C);
            floadLoadSet(0xEE);
            floadLoadSet(0xBC);
            floadLoadSet(0xBD);
            floadLoadSet(0x120);
            plAnimeSet(p0, 0x125);
            plAnimeSet(p1, 0x125);
            plAnimeSet(p2, 0);
            plAnimeSet(p3, 0);
            p1->flags |= PL_HIDDEN;
            p2->flags |= PL_HIDDEN;
            p3->flags |= PL_HIDDEN;
            w->fusuma = endFusumaCreate(obj);
            endFadeOut(0);
            endFadeIn(30);
            camSetEye((Camera *) w->cam, 0.0f, 10.0f, -34.0f);
            camSetAt((Camera *) w->cam, 0.0f, 10.0f, -193.0f);
            w->timer = 60;
            sndPlay(1);
            edSdSet(0x72, 8);
            w->state = 1;
            break;
        case 1:
            if (w->timer == 10) {
                plAnimeSet(p0, 0x12D);
            }
            if (--w->timer == 0) {
                camSetEye((Camera *) w->cam, 0.0f, 10.0f, -34.0f);
                camSetAt((Camera *) w->cam, 0.0f, 10.0f, -193.0f);
                endFusumaOpen(w->fusuma);
                edScSet(obj, 0x192, 0);
                edSdSet(0x34C, 8);
                endPlSetPos(w->pl[0], 0.0f, 0.0f, -75.0f);
                endPlSetRot(w->pl[0], 0.0f);
                endPlCtlFaceStart(w->plTask[0], 75, 2, 2, 2);
                edSdSet(0x18A, 0);
                w->state = 3;
            }
            break;
        case 2:
            break;
        case 3:
            if (plAnimEnded(p0)) {
                plAnimeSet(p0, 0x12C);
                w->state = 4;
                w->timer = 120;
            }
            break;
        case 4:
            if (--w->timer == 0) {
                endFadeOut(30);
                edScSetTime(endScTask, 15);
                w->timer = 30;
                w->state = 5;
            }
            break;
        case 5:
            if (--w->timer == 0) {
                scRunEnd();
                w->fusuma = tskDestroy(w->fusuma);
                w->state = 6;
            }
            break;
        case 6:
            endFadeIn(30);
            w->timer = 30;
            camSetEye((Camera *) w->cam, 5.8f, 10.0f, 13.1f);
            camSetAt((Camera *) w->cam, -69.0f, 6.0f, -128.0f);
            w->room = endZashikiCreate(obj);
            p1->flags &= ~PL_HIDDEN;
            p2->flags &= ~PL_HIDDEN;
            p3->flags &= ~PL_HIDDEN;
            endPlSetPos(w->pl[0], -41.0f, 0.0f, -37.0f);
            endPlSetRot(w->pl[0], 0.0f);
            plAnimeSet(p0, 0);
            plAnimeSet(p1, 0xEC);
            endPlCtlFaceStart(w->plTask[1], 150, 1, 2, 2);
            endPlSetPos(w->pl[1], -28.0f, 0.0f, -41.0f);
            endPlSetRot(w->pl[1], 0.0f);
            endPlSetPos(w->pl[2], -14.0f, 0.0f, -40.0f);
            endPlSetRot(w->pl[2], 0.0f);
            endPlSetPos(w->pl[3], -3.0f, 0.0f, -40.0f);
            endPlSetRot(w->pl[3], 0.0f);
            edScSet(obj, 0x193, 0);
            edSdSet(0x347, 15);
            w->timer = 180;
            w->state = 7;
            break;
        case 7:
            if (--w->timer == 0) {
                w->timer = 60;
                edScSetTime(endScTask, 30);
                w->state = 8;
            }
            break;
        case 8:
            if (--w->timer == 0) {
                w->state = 9;
            }
            break;
        case 9:
            p0->flags |= PL_HIDDEN;
            p1->flags |= PL_HIDDEN;
            camSetEye((Camera *) w->cam, 21.2f, 15.5f, -79.5f);
            camSetAt((Camera *) w->cam, -25.0f, 7.6f, 73.4f);
            plAnimeSet(p1, 0);
            w->jiji = endJijiCreate(obj);
            edScSet(obj, 0x194, 0);
            edSdSet(0x351, 30);
            endJijiSetPos(w->jiji, 11.1f, 0.0f, 0.5f);
            endJijiSetRot(w->jiji, 3.2724924f);
            endJijiSetState(w->jiji, 1);
            w->state = 10;
            w->timer = 180;
            break;
        case 10:
            if (--w->timer == 0) {
                camSetEye((Camera *) w->cam, 16.0f, 16.5f, -37.8f);
                camSetAt((Camera *) w->cam, -13.0f, 8.0f, 131.4f);
                p2->flags |= PL_HIDDEN;
                p3->flags |= PL_HIDDEN;
                w->state = 11;
                w->timer = 120;
            }
            break;
        case 11:
            if (--w->timer == 0) {
                camSetEye((Camera *) w->cam, 5.8f, 10.0f, 13.1f);
                camSetAt((Camera *) w->cam, -69.0f, 6.0f, -128.0f);
                edScSet(obj, 0x195, 0);
                edSdSet(0x358, 15);
                plAnimeSet(p2, 0xEC);
                endPlCtlFaceStart(w->plTask[2], 90, 1, 2, 2);
                p0->flags &= ~PL_HIDDEN;
                p1->flags &= ~PL_HIDDEN;
                p2->flags &= ~PL_HIDDEN;
                p3->flags &= ~PL_HIDDEN;
                w->state = 12;
                w->timer = 180;
            }
            break;
        case 12:
            if (--w->timer == 0) {
                endPlCtlSetState(w->plTask[2], 1);
                endPlCtlSetState(w->plTask[3], 0);
                endPlCtlFaceStart(w->plTask[3], 270, 1, 2, 2);
                edScSet(obj, 0x196, 0);
                edSdSet(0x35C, 12);
                w->state = 13;
                w->timer = 360;
            }
            break;
        case 13:
            if (w->timer == 120) {
                endPlCtlSetState(w->plTask[3], 1);
            }
            if (--w->timer == 0) {
                camSetEye((Camera *) w->cam, 16.0f, 16.5f, -37.8f);
                camSetAt((Camera *) w->cam, -13.0f, 8.0f, 131.4f);
                p0->flags |= PL_HIDDEN;
                p1->flags |= PL_HIDDEN;
                p2->flags |= PL_HIDDEN;
                p3->flags |= PL_HIDDEN;
                endJijiSetState(w->jiji, 2);
                edScSet(obj, 0x197, 0);
                edSdSet(0x352, 0);
                w->state = 14;
                w->timer = 120;
            }
            break;
        case 14:
            if (--w->timer == 0) {
                w->timer = 438;
                w->state = 15;
            }
            break;
        case 15:
            if (--w->timer == 0) {
                edScSet(obj, 0x198, 0);
                edSdSet(0x353, 0);
                endJijiSetState(w->jiji, 3);
                w->timer = 120;
                w->state = 16;
            }
            break;
        case 16:
            if (--w->timer == 0) {
                camSetEye((Camera *) w->cam, 21.0f, 8.5f, -21.6f);
                camSetAt((Camera *) w->cam, -128.0f, 20.0f, -79.0f);
                camSetFov((Camera *) w->cam, 0.47996557f);
                plAnimeSet(p0, 0);
                plAnimeSet(p1, 0);
                plAnimeSet(p2, 0);
                plAnimeSet(p3, 0);
                endPlSetPos(w->pl[1], -28.0f, 0.0f, -42.0f);
                endPlCtlFaceStart(w->plTask[0], -1, 1, 7, 7);
                endPlCtlFaceStart(w->plTask[1], -1, 1, 6, 6);
                endPlCtlFaceStart(w->plTask[2], -1, 1, 1, 1);
                endPlCtlFaceStart(w->plTask[3], -1, 1, 1, 1);
                p0->flags &= ~PL_HIDDEN;
                p1->flags &= ~PL_HIDDEN;
                p2->flags &= ~PL_HIDDEN;
                p3->flags &= ~PL_HIDDEN;
                endJijiHide(w->jiji);
                edScSet(obj, 0x199, 0);
                edSdSet(0x343, 0);
                edSdSet(0x35B, 0);
                edSdSet(0x35F, 0);
                w->timer = 54;
                w->state = 17;
            }
            break;
        case 17:
            if (--w->timer == 0) {
                edScSet(obj, 0x19A, 0);
                edSdSet(0x34D, 0);
                endPlCtlFaceStart(w->plTask[0], 40, 1, 2, 2);
                w->timer = 120;
                w->state = 18;
            }
            break;
        case 18:
            if (--w->timer == 0) {
                endFadeOut(30);
                w->timer = 30;
                w->state = 20;
            }
            break;
        case 19:
            floadLoadSet(0xC0);
            floadLoadSet(0xC2);
            floadLoadSet(0xC1);
            floadLoadSet(0xC5);
            floadLoadSet(0x12B);
            floadLoadSet(0x12A);
            floadLoadSet(0x12C);
            floadLoadSet(0xEE);
            floadLoadSet(0xBC);
            floadLoadSet(0xBD);
            floadLoadSet(0x120);
            endPlSetPos(w->pl[0], -41.0f, 0.0f, -37.0f);
            endPlSetRot(w->pl[0], 0.0f);
            endPlSetPos(w->pl[1], -28.0f, 0.0f, -42.0f);
            endPlSetRot(w->pl[1], 0.0f);
            endPlSetPos(w->pl[2], -14.0f, 0.0f, -40.0f);
            endPlSetRot(w->pl[2], 0.0f);
            endPlSetPos(w->pl[3], -3.0f, 0.0f, -40.0f);
            endPlSetRot(w->pl[3], 0.0f);
            w->jiji = endJijiCreate(obj);
            w->room = endZashikiCreate(obj);
            endFadeOut(0);
            w->state = 20;
            w->timer = 1;
            break;
        case 20:
            if (--w->timer == 0) {
                endFadeIn(30);
                camSetEye((Camera *) w->cam, -28.2f, 42.5f, -2.7f);
                camSetAt((Camera *) w->cam, 29.0f, -23.3f, -150.9f);
                camSetFov((Camera *) w->cam, 0.69813174f);
                w->room = tskDestroy(w->room);
                w->fusuma = endFusumaCreate(obj);
                endFusumaOpenDelayed(w->fusuma);
                p0->flags |= PL_HIDDEN;
                p1->flags |= PL_HIDDEN;
                p2->flags |= PL_HIDDEN;
                p3->flags |= PL_HIDDEN;
                endJijiShow(w->jiji);
                endJijiSetPos(w->jiji, 0.0f, 0.0f, -80.0f);
                endJijiSetRot(w->jiji, 0.0f);
                endJijiSetState(w->jiji, 4);
                sndPlay(0x73);
                edSdSet(0x1BE, 30);
                w->timer = 120;
                w->state = 21;
            }
            break;
        case 21:
            if (--w->timer == 0) {
                endFadeOut(30);
                w->timer = 60;
                w->state = 22;
            }
            break;
        case 22:
            if (--w->timer == 0) {
                endFadeIn(30);
                w->fusuma = tskDestroy(w->fusuma);
                w->lab = endLabCreate(obj);
                w->mac2 = endMac2Create(obj);
                camSetEye((Camera *) w->cam, 0.0f, 15.0f, -11.0f);
                camSetAt((Camera *) w->cam, 0.0f, 120.0f, 107.0f);
                camSetSpeed((Camera *) w->cam, 240.0f);
                camMoveEye((Camera *) w->cam, 0.0f, 15.0f, -53.0f);
                camMoveAt((Camera *) w->cam, 0.0f, 26.0f, 106.0f);
                camSetFov((Camera *) w->cam, 0.69813174f);
                endJijiSetPos(w->jiji, 14.0f, 0.0f, 10.0f);
                endJijiSetRot(w->jiji, 3.2672563f);
                endJijiSetState(w->jiji, 5);
                edScSet(obj, 0x19B, 0);
                edSdSet(0x354, 0);
                w->timer = 240;
                w->state = 23;
            }
            break;
        case 23:
            if (--w->timer == 0) {
                w->timer = 120;
                w->state = 24;
            }
            break;
        case 24:
            if (--w->timer == 0) {
                camSetEye((Camera *) w->cam, -25.9f, 19.0f, 0.0f);
                camSetAt((Camera *) w->cam, -172.0f, -20.0f, -52.3f);
                camSetFov((Camera *) w->cam, 0.57595867f);
                p0->flags &= ~PL_HIDDEN;
                endJijiHide(w->jiji);
                endPlSetPos(w->pl[0], -52.0f, 0.0f, -10.0f);
                endPlSetRot(w->pl[0], 0.7853982f);
                endPlCtlFaceStart(w->plTask[0], 175, 1, 4, 4);
                endPlSetPos(w->pl[1], -42.5f, 0.0f, -21.0f);
                endPlSetRot(w->pl[1], 0.7853982f);
                endPlSetPos(w->pl[2], -32.0f, 0.0f, -30.0f);
                endPlSetRot(w->pl[2], 0.7853982f);
                endPlSetPos(w->pl[3], -24.0f, 0.0f, -38.0f);
                endPlSetRot(w->pl[3], 0.7853982f);
                edScSet(obj, 0x19C, 0);
                edSdSet(0x34E, 30);
                w->timer = 210;
                w->state = 25;
            }
            break;
        case 25:
            if (--w->timer == 0) {
                camSetEye((Camera *) w->cam, 16.8f, 21.0f, -9.2f);
                camSetAt((Camera *) w->cam, -158.0f, -15.7f, -67.1f);
                camSetFov((Camera *) w->cam, 0.69813174f);
                p1->flags &= ~PL_HIDDEN;
                p2->flags &= ~PL_HIDDEN;
                p3->flags &= ~PL_HIDDEN;
                endPlCtlFaceStart(w->plTask[2], 150, 1, 4, 4);
                edScSet(obj, 0x19D, 0);
                edSdSet(0x359, 15);
                w->timer = 156;
                w->state = 26;
            }
            break;
        case 26:
            if (--w->timer == 0) {
                endPlCtlSetState(w->plTask[1], 0);
                endPlCtlFaceStart(w->plTask[1], 90, 1, 2, 2);
                edScSet(obj, 0x19E, 0);
                edSdSet(0x348, 42);
                w->timer = 282;
                w->state = 27;
            }
            break;
        case 27:
            if (--w->timer == 0) {
                endFadeOut(45);
                w->timer = 78;
                w->state = 28;
            }
            break;
        case 28:
            if (--w->timer == 0) {
                endFadeIn(45);
                camSetEye((Camera *) w->cam, 0.0f, 14.5f, -32.1f);
                camSetAt((Camera *) w->cam, 0.0f, 30.0f, 105.6f);
                camSetSpeed((Camera *) w->cam, 240.0f);
                camMoveAt((Camera *) w->cam, 0.0f, -19.0f, 105.6f);
                w->mao = endMaoCreate(obj);
                endMaoSetPos(w->mao, 5.0f, 0.0f, 5.0f);
                endMaoSetRot(w->mao, 0.5340708f);
                endJijiShow(w->jiji);
                endJijiSetState(w->jiji, 6);
                p0->flags &= ~PL_HIDDEN;
                p1->flags &= ~PL_HIDDEN;
                p2->flags &= ~PL_HIDDEN;
                p3->flags &= ~PL_HIDDEN;
                edScSet(obj, 0x19F, 0);
                edSdSet(0x355, 0);
                w->timer = 60;
                w->state = 29;
            }
            break;
        case 29:
            if (--w->timer == 0) {
                w->timer = 144;
                w->state = 30;
            }
            break;
        case 30:
        case 31:
            if (--w->timer == 0) {
                camSetEye((Camera *) w->cam, 1.9f, 14.5f, -5.5f);
                camSetAt((Camera *) w->cam, -140.0f, -9.0f, -76.6f);
                plAnimeSet(p1, 1);
                endPlCtlFaceStart(w->plTask[1], 120, 1, 2, 2);
                p1->animStep = 0.9f;
                p1->flags2 |= 1;
                endPlCtlWalkTo(w->plTask[1], -12.0f, 0.0f, 8.0f, 0.25f);
                edScSet(obj, 0x1A0, 0);
                edSdSet(0x349, 20);
                w->timer = 150;
                w->state = 32;
            }
            break;
        case 32:
            if (--w->timer == 0) {
                endFadeOut(20);
                w->timer = 20;
                w->state = 33;
            }
            break;
        case 33:
            if (--w->timer == 0) {
                endFadeIn(20);
                camSetEye((Camera *) w->cam, -30.2f, 15.0f, -8.7f);
                camSetAt((Camera *) w->cam, 102.5f, 7.1f, 80.1f);
                endPlCtlSetState(w->plTask[1], 2);
                p1->flags2 &= ~1;
                endPlCtlFaceStart(w->plTask[1], 60, 1, 2, 2);
                endPlSetPos(w->pl[1], 4.5f, 0.0f, 6.5f);
                endPlSetRot(w->pl[1], 4.3353977f);
                endMaoSetState(w->mao, 1);
                endMaoSetPos(w->mao, p1->model->pos[0], p1->model->pos[1], p1->model->pos[2]);
                endMaoSetRot(w->mao, 4.3353977f);
                endJijiSetPos(w->jiji, 12.7f, 0.0f, -4.0f);
                endJijiSetRot(w->jiji, 4.8380527f);
                endJijiSetState(w->jiji, 7);
                edScSet(obj, 0x1A1, 0);
                w->timer = 60;
                w->state = 34;
            }
            break;
        case 34:
            if (--w->timer == 0) {
                w->timer = 120;
                w->state = 35;
            }
            break;
        case 35:
            if (--w->timer == 0) {
                camSetEye((Camera *) w->cam, 11.7f, 16.5f, -7.9f);
                camSetAt((Camera *) w->cam, -131.4f, -7.0f, -75.6f);
                endPlCtlFaceStart(w->plTask[0], 60, 1, 3, 3);
                endPlSetPos(w->pl[0], -42.5f, 0.0f, -20.0f);
                endPlSetRot(w->pl[0], 1.0367256f);
                endPlSetPos(w->pl[2], -32.0f, 0.0f, -30.0f);
                endPlSetPos(w->pl[3], -22.0f, 0.0f, -37.0f);
                endPlSetRot(w->pl[3], 0.6597344f);
                edScSet(obj, 0x1A2, 0);
                edSdSet(0x34F, 0);
                w->timer = 120;
                w->state = 36;
            }
            break;
        case 36:
            if (--w->timer == 0) {
                endPlCtlFaceStart(w->plTask[2], 90, 1, 4, 4);
                edScSet(obj, 0x1A3, 0);
                edSdSet(0x35A, 0);
                w->timer = 120;
                w->state = 37;
            }
            break;
        case 37:
            if (--w->timer == 0) {
                endPlCtlFaceStart(w->plTask[3], 90, 2, 4, 4);
                edScSet(obj, 0x1A4, 0);
                edSdSet(0x35D, 0);
                w->timer = 150;
                w->state = 38;
            }
            break;
        case 38:
            if (--w->timer == 0) {
                camSetEye((Camera *) w->cam, -13.9f, 10.5f, -27.3f);
                camSetAt((Camera *) w->cam, 32.6f, 10.5f, 125.9f);
                endPlCtlSetState(w->plTask[1], 3);
                endMaoSetState(w->mao, 2);
                endPlSetRot(w->pl[1], 3.5185838f);
                endMaoSetRot(w->mao, 3.5185838f);
                endJijiSetPos(w->jiji, 15.1f, 0.0f, -9.0f);
                edScSet(obj, 0x1A5, 0);
                edSdSet(0x34A, 0);
                w->timer = 120;
                w->state = 39;
            }
            break;
        case 39:
            if (--w->timer == 0) {
                w->timer = 120;
                w->state = 40;
            }
            break;
        case 40:
            if (--w->timer == 0) {
                camSetEye((Camera *) w->cam, -12.3f, 13.0f, -15.2f);
                camSetAt((Camera *) w->cam, 142.2f, 28.7f, 23.5f);
                camSetFov((Camera *) w->cam, 0.5235988f);
                endJijiSetState(w->jiji, 8);
                edScSet(obj, 0x1A6, 0);
                edSdSet(0x356, 0);
                w->timer = 120;
                w->state = 41;
            }
            break;
        case 41:
            if (--w->timer == 0) {
                endFadeOut(50);
                edScSetTime(endScTask, 180);
                w->timer = 60;
                w->state = 43;
            }
            break;
        case 42:
            floadLoadSet(0xC0);
            floadLoadSet(0xC2);
            floadLoadSet(0xC1);
            floadLoadSet(0xC5);
            floadLoadSet(0x12B);
            floadLoadSet(0x12A);
            floadLoadSet(0x12C);
            floadLoadSet(0xEE);
            floadLoadSet(0xBC);
            floadLoadSet(0xBD);
            floadLoadSet(0x120);
            endPlSetPos(w->pl[0], -41.0f, 0.0f, -37.0f);
            endPlSetRot(w->pl[0], 0.0f);
            endPlSetPos(w->pl[1], -28.0f, 0.0f, -42.0f);
            endPlSetRot(w->pl[1], 0.0f);
            endPlSetPos(w->pl[2], -14.0f, 0.0f, -40.0f);
            endPlSetRot(w->pl[2], 0.0f);
            endPlSetPos(w->pl[3], -3.0f, 0.0f, -40.0f);
            endPlSetRot(w->pl[3], 0.0f);
            w->jiji = endJijiCreate(obj);
            w->mao = endMaoCreate(obj);
            w->lab = endLabCreate(obj);
            w->mac2 = endMac2Create(obj);
            endPlSetPos(w->pl[0], -42.5f, 0.0f, -20.0f);
            endPlSetRot(w->pl[0], 1.0367256f);
            endPlSetPos(w->pl[1], 4.5f, 0.0f, 6.5f);
            endPlSetRot(w->pl[1], 3.5185838f);
            endPlSetPos(w->pl[2], -32.0f, 0.0f, -30.0f);
            endPlSetRot(w->pl[2], 0.7853982f);
            endPlSetPos(w->pl[3], -22.0f, 0.0f, -37.0f);
            endPlSetRot(w->pl[3], 0.6597344f);
            endMaoSetPos(w->mao, p1->model->pos[0], p1->model->pos[1], p1->model->pos[2]);
            endMaoSetRot(w->mao, 3.5185838f);
            endMaoSetState(w->mao, 2);
            endPlCtlSetState(w->plTask[1], 3);
            endJijiSetPos(w->jiji, 15.1f, 0.0f, -9.0f);
            endJijiSetState(w->jiji, 8);
            endFadeOut(0);
            w->state = 43;
            w->timer = 120;
            break;
        case 43:
            if (--w->timer == 0) {
                endFadeIn(50);
                camSetEye((Camera *) w->cam, -10.3f, 37.0f, -116.0f);
                camSetAt((Camera *) w->cam, -10.3f, 5.8f, 41.0f);
                camSetFov((Camera *) w->cam, 0.69813174f);
                endJijiSetState(w->jiji, 9);
                endPlCtlStopAnime(w->plTask[0]);
                endPlCtlStopAnime(w->plTask[1]);
                endPlCtlStopAnime(w->plTask[2]);
                endPlCtlStopAnime(w->plTask[3]);
                endMaoSetState(w->mao, 4);
                edSdSet(0x189, 0);
                w->state = 44;
                w->timer = 60;
            }
            break;
        case 44:
            if (--w->timer == 0) {
                w->flash[0] = endFlashCreate(obj);
                endFlashSetPos(w->flash[0], 0.0f, 20.0f, 35.0f);
                endFlashSetState(w->flash[0], 0);
                w->state = 45;
                w->timer = 30;
            }
            break;
        case 45:
            if (--w->timer == 0) {
                w->flash[1] = endFlashCreate(obj);
                endFlashSetPos(w->flash[1], 0.0f, 20.0f, 35.0f);
                endFlashSetState(w->flash[1], 0);
                endFlashSetSpeed(w->flash[1], 0.4f);
                endFlashSetRot(w->flash[1], 1.5707963f, 1.5707963f, 1.5707963f);
                w->state = 46;
                w->timer = 10;
            }
            break;
        case 46:
            if (--w->timer == 0) {
                w->flash[2] = endFlashCreate(obj);
                endFlashSetPos(w->flash[2], 0.0f, 20.0f, 35.0f);
                endFlashSetState(w->flash[2], 0);
                endFlashSetSpeed(w->flash[2], 0.15f);
                endFlashSetRot(w->flash[1], -1.5707963f, -1.5707963f, 1.5707963f);
                w->state = 47;
                w->timer = 30;
            }
            break;
        case 47:
            if (--w->timer == 0) {
                endAligSetState(w->alig, 1);
                w->state = 48;
                w->timer = 120;
            }
            break;
        case 48:
            if (--w->timer == 0) {
                fadeSetAll(90.0f, 0.0f, 255.0f, 255, 255, 255);
                edSdSet(0x18B, 0);
                w->state = 49;
                w->timer = 120;
            }
            break;
        case 49:
            if (--w->timer == 0) {
                fadeSetAll(90.0f, 255.0f, 0.0f, 255, 255, 255);
                w->flash[0] = tskDestroy(w->flash[0]);
                w->flash[1] = tskDestroy(w->flash[1]);
                w->flash[2] = tskDestroy(w->flash[2]);
                endAligSetState(w->alig, 0);
                camSetEye((Camera *) w->cam, 0.0f, 58.5f, -3.3f);
                camSetAt((Camera *) w->cam, 0.0f, 60.4f, 35.0f);
                camSetFov((Camera *) w->cam, 0.5235988f);
                camSetSpeed((Camera *) w->cam, 240.0f);
                camMoveEye((Camera *) w->cam, 0.0f, 45.0f, -25.0f);
                w->bisu = endBisuCreate(obj);
                endBisuSetState(w->bisu, 0);
                endBisuSetPos(w->bisu, 0.0f, 42.0f, 35.0f);
                endBisuSetRot(w->bisu, 3.1415927f);
                endMac2SetState(w->mac2, 0);
                plAnimeSet(p0, 0);
                plAnimeSet(p2, 0);
                plAnimeSet(p3, 0);
                endJijiSetState(w->jiji, 0);
                endPlCtlSetState(w->plTask[1], 4);
                endPlSetRot(w->pl[1], 0.37699112f);
                endMaoSetState(w->mao, 3);
                endMaoSetRot(w->mao, 0.37699112f);
                edScSet(obj, 0x1A7, 0);
                edSdSet(0x344, 0);
                sndPlay(0x74);
                w->state = 50;
                w->timer = 204;
            }
            break;
        case 50:
            if (--w->timer == 0) {
                camSetEye((Camera *) w->cam, -6.0f, 7.5f, -124.0f);
                camSetAt((Camera *) w->cam, -6.0f, 31.0f, 34.3f);
                camSetFov((Camera *) w->cam, 0.69813174f);
                edScSet(obj, 0x1A8, 0);
                edSdSet(0x350, 20);
                w->state = 51;
                w->timer = 114;
            }
            break;
        case 51:
            if (--w->timer == 0) {
                camSetEye((Camera *) w->cam, 2.5f, 15.5f, -15.1f);
                camSetAt((Camera *) w->cam, -130.6f, -16.9f, -109.1f);
                endPlCtlSetState(w->plTask[3], 5);
                edScSet(obj, 0x1A9, 0);
                edSdSet(0x35E, 15);
                w->state = 52;
                w->timer = 120;
            }
            break;
        case 52:
            if (--w->timer == 0) {
                camSetEye((Camera *) w->cam, 27.9f, 40.5f, -15.0f);
                camSetAt((Camera *) w->cam, -56.8f, 81.8f, 126.5f);
                endBisuSetState(w->bisu, 1);
                edScSet(obj, 0x1AA, 0);
                edSdSet(0x345, 0);
                w->state = 53;
                w->timer = 300;
            }
            break;
        case 53:
            if (--w->timer == 0) {
                endBisuSetState(w->bisu, 2);
                edScSet(obj, 0x1AB, 0);
                edSdSet(0x346, 0);
                w->state = 54;
                w->timer = 52;
            }
            break;
        case 54:
            if (--w->timer == 0) {
                camSetEye((Camera *) w->cam, -36.2f, 11.0f, 1.1f);
                camSetAt((Camera *) w->cam, 125.2f, 11.0f, 54.4f);
                camSetFov((Camera *) w->cam, 0.5235988f);
                endPlCtlSetState(w->plTask[1], 4);
                endPlSetRot(w->pl[1], 0.37699112f);
                endMaoSetState(w->mao, 3);
                endMaoSetRot(w->mao, 0.37699112f);
                edScSet(obj, 0x1AC, 0);
                edSdSet(0x34B, 0);
                w->state = 55;
                w->timer = 120;
            }
            break;
        case 55:
            if (--w->timer == 0) {
                l = w->light;
                camSetEye((Camera *) w->cam, -3.5f, 11.0f, 14.3f);
                camSetAt((Camera *) w->cam, 102.8f, 34.5f, -103.0f);
                endJijiSetState(w->jiji, 8);
                endJijiSetRot(w->jiji, 0.0f);
                l->color[0] = 200;
                l->color[1] = 200;
                l->color[2] = 200;
                dir[0] = -1.0f;
                dir[1] = 0.0f;
                dir[2] = 4.0f;
                endLightSetDir(w->light, dir);
                w->flash[0] = endFlashCreate(obj);
                endFlashSetPos(w->flash[0], 0.0f, 20.0f, 35.0f);
                endFlashSetState(w->flash[0], 0);
                edScSet(obj, 0x1AD, 0);
                edSdSet(0x357, 60);
                w->state = 56;
                w->timer = 60;
            }
            break;
        case 56:
            if (--w->timer == 0) {
                w->state = 57;
                w->timer = 90;
            }
            break;
        case 57:
            if (--w->timer == 0) {
                camSetEye((Camera *) w->cam, 176.3f, 8.0f, 240.3f);
                camSetAt((Camera *) w->cam, 105.2f, 32.7f, 90.0f);
                camSetSpeed((Camera *) w->cam, 180.0f);
                camMoveEye((Camera *) w->cam, 202.4f, 8.0f, 295.5f);
                camSetFov((Camera *) w->cam, 0.69813174f);
                w->lab = tskDestroy(w->lab);
                w->mac2 = tskDestroy(w->mac2);
                w->flash[0] = tskDestroy(w->flash[0]);
                w->mao = tskDestroy(w->mao);
                w->jiji = tskDestroy(w->jiji);
                w->bisu = tskDestroy(w->bisu);
                gGameWork.plManagerTask = tskDestroy(gGameWork.plManagerTask);
                w->castle = endCastleCreate(obj);
                w->ground = endCastleGroundCreate(obj);
                w->gate = endCastleGateCreate(obj);
                w->tree[0] = endTreeCreate(obj);
                endTreeSetPos(w->tree[0], 65.0f, 0.0f, 0.0f);
                w->tree[1] = endTreeCreate(obj);
                endTreeSetPos(w->tree[1], 0.0f, 0.0f, -170.0f);
                w->impact[0] = endImpactCreate(obj);
                endImpactSetPos(w->impact[0], -106.0f, 0.0f, -153.0f);
                w->impact[1] = endImpactCreate(obj);
                endImpactSetPos(w->impact[1], -178.0f, 0.0f, -153.0f);
                endImpactSetState(w->impact[1], 0);
                endAligSetState(w->alig, 0);
                edSdSet(0x18C, 0);
                endFadeIn(0);
                w->state = 58;
                w->timer = 120;
            }
            break;
        case 58:
            if (--w->timer == 0) {
                w->flash[0] = endFlashCreate(obj);
                endFlashSetState(w->flash[0], 1);
                endFlashSetPos(w->flash[0], 0.0f, 75.0f, -90.0f);
                endAligSetState(w->alig, 1);
                w->timer = 60;
                w->state = 59;
            }
            break;
        case 59:
            if (w->timer == 40) {
                w->flash[2] = endFlashCreate(obj);
                endFlashSetState(w->flash[2], 1);
                endFlashSetPos(w->flash[2], 30.0f, 40.0f, -70.0f);
                endFlashSetSpeed(w->flash[2], 0.3f);
                endFlashSetRot(w->flash[2], 0.0f, 1.5707963f, 0.0f);
            }
            if (w->timer == 20) {
                w->flash[1] = endFlashCreate(obj);
                endFlashSetState(w->flash[1], 1);
                endFlashSetPos(w->flash[1], -20.0f, 50.0f, -80.0f);
                endFlashSetSpeed(w->flash[1], 0.4f);
                endFlashSetRot(w->flash[1], 0.0f, 1.5707963f, 0.0f);
            }
            if (--w->timer == 0) {
                fadeSetAll(90.0f, 0.0f, 255.0f, 255, 255, 255);
                w->timer = 60;
                w->state = 60;
            }
            break;
        case 60:
            if (w->timer == 50) {
                w->fball[0] = endFballCreate(obj);
                endFballSetPos(w->fball[0], -30.0f, 40.0f, -70.0f);
                edSdSet(0x11D, 20);
            }
            if (w->timer == 30) {
                w->fball[1] = endFballCreate(obj);
                endFballSetPos(w->fball[1], 10.0f, 50.0f, -65.0f);
            }
            if (w->timer == 10) {
                w->fball[2] = endFballCreate(obj);
                endFballSetPos(w->fball[2], 20.0f, 70.0f, -70.0f);
            }
            if (--w->timer == 0) {
                w->fball[0] = tskDestroy(w->fball[0]);
                w->fball[1] = tskDestroy(w->fball[1]);
                w->fball[2] = tskDestroy(w->fball[2]);
                w->flash[0] = tskDestroy(w->flash[0]);
                w->flash[1] = tskDestroy(w->flash[1]);
                w->flash[2] = tskDestroy(w->flash[2]);
                w->castle = tskDestroy(w->castle);
                w->ground = tskDestroy(w->ground);
                w->gate = tskDestroy(w->gate);
                w->tree[0] = tskDestroy(w->tree[0]);
                w->tree[1] = tskDestroy(w->tree[1]);
                w->impact[0] = tskDestroy(w->impact[0]);
                w->impact[1] = tskDestroy(w->impact[1]);
                endScTask = tskDestroy(endScTask);
                w->timer = 20;
                w->state = 61;
                w->boxTop = 45.0f;
                w->boxBottom = 185.0f;
            }
            break;
        case 61:
            if (--w->timer == 0) {
                w->timer = 100;
                w->state = 62;
            }
            break;
        case 62:
            fadeSetLetterbox(320, w->boxTop, 0, w->boxBottom);
            if (w->boxTop > 0.0f) {
                w->boxTop -= 0.5f;
            }
            if (w->boxBottom < 240.0f) {
                w->boxBottom += 0.5f;
            }
            if (--w->timer == 0) {
                stageGotoStaffRoll();
                w->state = 63;
            }
            break;
        case 63:
            break;
    }
    if (plIsInPlay(0)) {
        plAnimeStep(p0);
        plAnimeStep(p1);
        plAnimeStep(p2);
        plAnimeStep(p3);
    }
}

/* Builds the scene: the sky, the camera, the light, the four characters and their controls. */
void endInit(Obj *obj) {
    s32 pad;
    PlInit init[4];
    EndWork *w = (EndWork *) (obj + 1);
    s32 i;
    LightFrame *l;

    DPRINTF("endInit\n");
    /* The clear stores one byte (the game's bug, as in file 35). */
    for (i = 0; i < sizeof(EndWork); i++) {
        *(u8 *) w = 0;
    }
    w->self = obj;
    endInitGlobals(obj);
    w->flags = 1;
    w->state = 0;
    w->timer = 0;
    w->frames = 0;
    tskCreate(obj, attack_Proc, 0, NULL);
    tskCreate(obj, effect, 0, NULL);
    w->sky = tskCreate(obj, bgrad, 0, NULL);
    camBgradSetLines((Camera *) w->sky, 76, 240);
    camBgradMoveTop((ColorFade *) w->sky, 0x60, 0x60, 0xFF);
    camBgradMoveBottom((ColorFade *) w->sky, 0xE0, 0xE0, 0xFF);
    w->fade = fadeCreate(obj);
    fadeSetLetterbox(320, 45, 0, 185);
    tskCreate(obj, scParentProc, 0, NULL);
    w->cam = tskCreate(obj, endGcamProc, 0, NULL);
    endCam = w->cam;
    DPRINTF("endInit: %08x\n", w->cam);
    camSetFov((Camera *) w->cam, 0.69813174f);
    camSetEye((Camera *) w->cam, 0.0f, 100.0f, 100.0f);
    camSetAt((Camera *) w->cam, 0.0f, 0.0f, 0.0f);
    l = w->light = (LightFrame *) frmCreate(gFrmLayers[0], gFrmKindLight);
    l->dir[0] = 30;
    l->dir[1] = 100;
    l->dir[2] = 30;
    l->color[0] = 230;
    l->color[1] = 230;
    l->color[2] = 230;
    fadeLightNormalize(l);
    w->alig = endAligCreate(obj);
    for (i = 0; i < 4; i++) {
        plMakeInit(&init[i], 0, i);
        init[i].flags |= 0x91800050;
    }
    for (i = 0; i < 4; i++) {
        w->pl[i] = plCreate(obj, &init[i]);
        w->plTask[i] = endPlCtlCreate(w->pl[i]);
    }
    DPRINTF("endInit end\n");
}

void endInitGlobals(Obj *obj) {
    endInitObj = obj;
    endCam = NULL;
    endScTask = NULL;
    gRetracesPerFrame = 1;
}

void endUnusedStub1(void) {
}

void endUnusedStub2(void) {
}

/* The camera "gcam": main's dracam with message 9 setting its mode. */
s32 endGcamProc(Obj *obj, s16 msg, void *arg) {
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
            endGcamMove(w);
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

void endUnusedStub3(void) {
}

void endUnusedStub4(void) {
}

/* gcam's moves: main's dracam's, the eye's three written with a signed step. */
void endGcamMove(GcamWork *w) {
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

/* Puts a player's model at (x, y, z). */
void endPlSetPos(Obj *pl, f32 x, f32 y, f32 z) {
    AnimeFrame *m = PL(pl)->model;

    m->pos[0] = x;
    m->pos[1] = y;
    m->pos[2] = z;
}

/* Turns a player's model to face `angle`. */
void endPlSetRot(Obj *pl, f32 angle) {
    AnimeFrame *m = PL(pl)->model;

    m->rot[1] = angle;
}

/* Starts an animation (file set `set`) at a speed, from its end if the speed is negative. */
void endAnimStart(AnimPlay *a, u32 anim, s32 set, f32 speed, s32 flags) {
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
void endAnimStep(AnimPlay *a) {
    f32 next = a->speed + a->accel;
    AnimeFrame *f = a->frame;
    f32 last;

    if (a->speed * next <= 0.0f) {
        a->accel = 0.0f;
        a->speed = 0.0f;
    } else {
        a->speed = next;
    }
    f->frame += a->speed * 0.5f;
    last = polAnimeLastFrame(f);
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

/* How far the animation has run, in percent (at an end: 100 forwards, 0 backwards). */
s32 endAnimPercent(AnimPlay *a) {
    AnimeFrame *f = a->frame;

    if (a->status & 1) {
        return (a->speed >= 0.0f) ? 100 : 0;
    }
    return f->frame / polAnimeLastFrame(f) * 100.0f;
}

/* Whether the animation has reached an end. */
s32 endAnimEnded(AnimPlay *a) {
    return (a->status & 2) != 0;
}

/* Hides a frame (its draw handler stops). */
void endFrameHide(Frame **f) {
    (*f)->flags &= ~2;
}

/* Shows a frame. */
void endFrameShow(Frame **f) {
    (*f)->flags |= 2;
}

void endUnusedStub5(void) {
}

/* Points a light along `dir`. */
void endLightSetDir(LightFrame *l, f32 *dir) {
    l->dir[0] = dir[0];
    l->dir[1] = dir[1];
    l->dir[2] = dir[2];
    fadeLightNormalize(l);
}

/* Fades in from black. */
void endFadeIn(s32 speed) {
    fadeSetAll(speed, 255.0f, 0.0f, 0, 0, 0);
}

/* Fades out to black. */
void endFadeOut(s32 speed) {
    fadeSetAll(speed, 0.0f, 255.0f, 0, 0, 0);
}

/* The sliding door ("襖"): an anime frame of file 0xC1. State 1 opens it until a quarter way,
 * then slows it; 3 waits 30 frames and opens it the rest. */
typedef struct FusumaWork {
    /* 0x00 */ AnimPlay anim;
    /* 0x14 */ s32 state;
    /* 0x18 */ s32 timer;
} FusumaWork;

extern u8 endFusumaDoorAnim[]; /* the door's animation */

s32 endFusumaProc(Obj *obj, s16 msg, void *arg) {
    FusumaWork *w = (FusumaWork *) (obj + 1);
    AnimeFrame *f;
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "襖";
            w->anim.frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->anim.frame == NULL) {
                return -1;
            }
            endAnimStart(&w->anim, (u32) endFusumaDoorAnim, 0xC1, 0.0f, 1);
            f = w->anim.frame;
            f->scale[0] = 0.75f;
            f->scale[1] = 0.75f;
            f->scale[2] = 0.75f;
            f->pos[0] = 0.0f;
            f->pos[1] = 0.0f;
            f->pos[2] = 0.0f;
            break;
        case 1:
            frmDestroy((Frame *) w->anim.frame);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    break;
                case 1:
                    if (endAnimPercent(&w->anim) >= 25) {
                        w->anim.accel = -0.002f;
                        w->state = 2;
                    }
                    break;
                case 2:
                    break;
                case 3:
                    if (--w->timer == 0) {
                        w->anim.speed = 0.075f;
                        w->state = 4;
                    }
                    break;
                case 4:
                    break;
                case 5:
                    break;
            }
            endAnimStep(&w->anim);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *endFusumaCreate(Obj *obj) {
    return tskCreate(obj, endFusumaProc, obj->file, NULL);
}

/* Opens the door. */
void endFusumaOpen(Obj *obj) {
    FusumaWork *w = (FusumaWork *) (obj + 1);

    w->anim.speed = 0.05f;
    w->anim.frame->frame = 0.0f;
    w->anim.status = 0;
    w->anim.flags = 1;
    w->state = 1;
}

/* Opens the door after 30 frames. */
void endFusumaOpenDelayed(Obj *obj) {
    FusumaWork *w = (FusumaWork *) (obj + 1);

    w->anim.speed = 0.0f;
    w->anim.frame->frame = 0.0f;
    w->anim.status = 0;
    w->anim.flags = 1;
    w->timer = 30;
    w->state = 3;
}

/* The old man ("じじい"): an anime frame of file 0xC5. States 0-9 start his acts, 10-21 run
 * them, 22 idles. */
typedef struct JijiWork {
    /* 0x00 */ AnimPlay anim;
    /* 0x14 */ s32 state;
    /* 0x18 */ s32 timer;
} JijiWork;

extern u8 endJijiAnim[]; /* the old man's animations (file 0xC5) */
extern u8 endJijiState12Anim[];
extern u8 endJijiAnim2[];
extern u8 endJijiState19Anim[];
extern u8 endJijiState20Anim[];
extern u8 endJijiState16Anim[];
extern u8 endJijiState17Anim[];
extern u8 endJijiState1Anim[];
extern u8 endJijiState10Anim[];
extern u8 endJijiAnim3[];
extern u8 endJijiState11Anim[];
extern u8 endJijiState6Anim[];
extern u8 endJijiState18Anim[];
extern u8 endJijiState8Anim[];
extern u8 endJijiState21Anim[];

void endJijiAnimStart(Obj *obj, u32 anim, f32 speed, s32 flags);

s32 endJijiProc(Obj *obj, s16 msg, void *arg) {
    JijiWork *w = (JijiWork *) (obj + 1);
    AnimeFrame *f;

    switch (msg) {
        case 2:
            obj->name = "じじい";
            w->anim.frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->anim.frame == NULL) {
                return -1;
            }
            endAnimStart(&w->anim, (u32) endJijiAnim, 0xC5, 0.05f, 2);
            f = w->anim.frame;
            f->scale[0] = 0.1f;
            f->scale[1] = 0.1f;
            f->scale[2] = 0.1f;
            f->pos[0] = 0.0f;
            f->pos[1] = 0.0f;
            f->pos[2] = 0.0f;
            w->state = 23;
            break;
        case 1:
            frmDestroy((Frame *) w->anim.frame);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    endJijiAnimStart(obj, (u32) endJijiAnim, 0.7f, 2);
                    w->state = 22;
                    break;
                case 1:
                    endJijiAnimStart(obj, (u32) endJijiState1Anim, 0.7f, 1);
                    w->state = 10;
                    break;
                case 10:
                    if (endAnimPercent(&w->anim) >= 100) {
                        endJijiAnimStart(obj, (u32) endJijiState10Anim, 0.7f, 2);
                        w->state = 22;
                    }
                    break;
                case 2:
                    endJijiAnimStart(obj, (u32) endJijiAnim3, 0.7f, 1);
                    w->state = 11;
                    break;
                case 11:
                    if (endAnimPercent(&w->anim) >= 100) {
                        endJijiAnimStart(obj, (u32) endJijiState11Anim, 0.7f, 2);
                        w->state = 22;
                    }
                    break;
                case 3:
                    endJijiAnimStart(obj, (u32) endJijiAnim3, -0.8f, 1);
                    w->state = 12;
                    break;
                case 12:
                    w->anim.frame->rot[1] += 0.05235987902f;
                    if (endAnimEnded(&w->anim)) {
                        endJijiAnimStart(obj, (u32) endJijiState12Anim, 0.4f, 1);
                        w->timer = 60;
                        w->state = 13;
                    }
                    break;
                case 13:
                    w->anim.frame->rot[1] += 0.05235987902f;
                    if (mathSin(w->anim.frame->rot[1]) >= 0.0f) {
                        w->anim.frame->rot[1] = 0.0f;
                        w->state = 22;
                    }
                    break;
                case 4:
                    endJijiAnimStart(obj, (u32) endJijiAnim2, 0.67f, 1);
                    w->timer = 80;
                    w->state = 14;
                    break;
                case 14:
                    if (--w->timer == 0) {
                        w->state = 15;
                    }
                    break;
                case 15:
                    if (endAnimPercent(&w->anim) >= 100) {
                        endJijiAnimStart(obj, (u32) endJijiAnim2, -0.4f, 1);
                        w->state = 22;
                    }
                    break;
                case 5:
                    w->timer = 110;
                    w->state = 16;
                    break;
                case 16:
                    if (--w->timer == 0) {
                        endJijiAnimStart(obj, (u32) endJijiState16Anim, 0.75f, 1);
                        w->state = 17;
                    }
                    break;
                case 17:
                    if (endAnimPercent(&w->anim) >= 100) {
                        endJijiAnimStart(obj, (u32) endJijiState17Anim, 1.0f, 1);
                        w->state = 22;
                    }
                    break;
                case 6:
                    endJijiAnimStart(obj, (u32) endJijiState6Anim, 0.5f, 1);
                    w->state = 18;
                    break;
                case 18:
                    if (endAnimPercent(&w->anim) >= 100) {
                        endJijiAnimStart(obj, (u32) endJijiState18Anim, 0.5f, 2);
                        w->state = 22;
                    }
                    break;
                case 7:
                    endJijiAnimStart(obj, (u32) endJijiAnim, 0.3f, 2);
                    w->timer = 45;
                    w->state = 19;
                    break;
                case 19:
                    if (--w->timer == 0) {
                        endJijiAnimStart(obj, (u32) endJijiState19Anim, 0.6f, 1);
                        w->state = 20;
                    }
                    break;
                case 20:
                    if (endAnimPercent(&w->anim) >= 100) {
                        endJijiAnimStart(obj, (u32) endJijiState20Anim, 0.6f, 2);
                        w->state = 22;
                    }
                    break;
                case 8:
                    endJijiAnimStart(obj, (u32) endJijiState8Anim, 0.7f, 1);
                    w->state = 21;
                    break;
                case 21:
                    if (endAnimPercent(&w->anim) >= 100) {
                        endJijiAnimStart(obj, (u32) endJijiState21Anim, 0.75f, 2);
                        w->state = 22;
                    }
                    break;
                case 9:
                    w->anim.speed = 0.0f;
                    w->state = 22;
                    break;
                case 22:
                    break;
                case 23:
                    break;
            }
            endAnimStep(&w->anim);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *endJijiCreate(Obj *obj) {
    return tskCreate(obj, endJijiProc, obj->file, NULL);
}

void endJijiSetPos(Obj *obj, f32 x, f32 y, f32 z) {
    AnimeFrame *f = ((JijiWork *) (obj + 1))->anim.frame;

    f->pos[0] = x;
    f->pos[1] = y;
    f->pos[2] = z;
}

void endJijiSetRot(Obj *obj, f32 angle) {
    AnimeFrame *f = ((JijiWork *) (obj + 1))->anim.frame;

    f->rot[1] = angle;
}

void endJijiAnimStart(Obj *obj, u32 anim, f32 speed, s32 flags) {
    endAnimStart((AnimPlay *) (obj + 1), anim, 0xC5, speed, flags);
}

void endJijiSetState(Obj *obj, s32 state) {
    ((JijiWork *) (obj + 1))->state = state;
}

void endJijiHide(Obj *obj) {
    endFrameHide((Frame **) (obj + 1));
}

void endJijiShow(Obj *obj) {
    endFrameShow((Frame **) (obj + 1));
}

/* Mao ("マオ"): an anime frame of file 0x12A. States 0-4 start his acts, 5 and 6 run one, 7
 * idles. */
typedef struct MaoWork {
    /* 0x00 */ AnimPlay anim;
    /* 0x14 */ s32 state;
} MaoWork;

extern u8 endMaoState0Anim[]; /* Mao's animations (file 0x12A) */
extern u8 endMaoState1Anim[];
extern u8 endMaoState5Anim[];
extern u8 endMaoState6Anim[];
extern u8 endMaoState2Anim[];
extern u8 endMaoState3Anim[];

s32 endMaoProc(Obj *obj, s16 msg, void *arg) {
    MaoWork *w = (MaoWork *) (obj + 1);
    AnimeFrame *f;

    switch (msg) {
        case 2:
            obj->name = "マオ";
            w->anim.frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->anim.frame == NULL) {
                return -1;
            }
            endAnimStart(&w->anim, (u32) endMaoState0Anim, 0x12A, 0.5f, 2);
            f = w->anim.frame;
            f->scale[0] = 0.06f;
            f->scale[1] = 0.06f;
            f->scale[2] = 0.06f;
            f->pos[0] = 0.0f;
            f->pos[1] = 0.0f;
            f->pos[2] = 0.0f;
            w->state = 8;
            break;
        case 1:
            frmDestroy((Frame *) w->anim.frame);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    endAnimStart(&w->anim, (u32) endMaoState0Anim, 0x12A, 0.4f, 2);
                    w->state = 7;
                    break;
                case 1:
                    endAnimStart(&w->anim, (u32) endMaoState1Anim, 0x12A, 0.4f, 2);
                    w->state = 5;
                    break;
                case 5:
                    if (endAnimPercent(&w->anim) == 100) {
                        endAnimStart(&w->anim, (u32) endMaoState5Anim, 0x12A, 0.4f, 1);
                        w->state = 6;
                    }
                    break;
                case 6:
                    if (endAnimPercent(&w->anim) == 100) {
                        endAnimStart(&w->anim, (u32) endMaoState6Anim, 0x12A, 0.4f, 2);
                        w->state = 7;
                    }
                    break;
                case 2:
                    endAnimStart(&w->anim, (u32) endMaoState2Anim, 0x12A, 0.4f, 2);
                    w->state = 7;
                    break;
                case 4:
                    w->anim.speed = 0.0f;
                    w->state = 7;
                    break;
                case 3:
                    endAnimStart(&w->anim, (u32) endMaoState3Anim, 0x12A, 0.6f, 2);
                    w->state = 7;
                    break;
                case 7:
                    break;
                case 8:
                    break;
            }
            endAnimStep(&w->anim);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *endMaoCreate(Obj *obj) {
    return tskCreate(obj, endMaoProc, obj->file, NULL);
}

void endMaoSetPos(Obj *obj, f32 x, f32 y, f32 z) {
    AnimeFrame *f = ((MaoWork *) (obj + 1))->anim.frame;

    f->pos[0] = x;
    f->pos[1] = y;
    f->pos[2] = z;
}

void endMaoSetRot(Obj *obj, f32 angle) {
    AnimeFrame *f = ((MaoWork *) (obj + 1))->anim.frame;

    f->rot[1] = angle;
}

void endMaoSetState(Obj *obj, s32 state) {
    ((MaoWork *) (obj + 1))->state = state;
}

void endUnusedStub6(void) {
}

void endUnusedStub7(void) {
}

/* "bisu": an anime frame of file 0x12C. States 0-2 start her acts, 3 and 4 run them, 5
 * idles. */
typedef struct BisuWork {
    /* 0x00 */ AnimPlay anim;
    /* 0x14 */ s32 state;
} BisuWork;

extern u8 endBisuState0Anim[]; /* bisu's animations (file 0x12C) */
extern u8 endBisuAnim[];
extern u8 endBisuState1Anim[];
extern u8 endBisuState3Anim[];
extern u8 endBisuState2Anim[];
extern u8 endBisuState4Anim[];

s32 endBisuProc(Obj *obj, s16 msg, void *arg) {
    BisuWork *w = (BisuWork *) (obj + 1);
    AnimeFrame *f;

    switch (msg) {
        case 2:
            obj->name = "bisu";
            w->anim.frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->anim.frame == NULL) {
                return -1;
            }
            endAnimStart(&w->anim, (u32) endBisuAnim, 0x12C, 0.5f, 2);
            f = w->anim.frame;
            f->pos[0] = 0.0f;
            f->pos[1] = 0.0f;
            f->pos[2] = 0.0f;
            f->scale[0] = 0.14f;
            f->scale[1] = 0.14f;
            f->scale[2] = 0.14f;
            w->state = 6;
            break;
        case 1:
            frmDestroy((Frame *) w->anim.frame);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    endAnimStart(&w->anim, (u32) endBisuState0Anim, 0x12C, 0.8f, 2);
                    w->state = 5;
                    break;
                case 1:
                    endAnimStart(&w->anim, (u32) endBisuState1Anim, 0x12C, 0.8f, 1);
                    w->state = 3;
                    break;
                case 3:
                    if (endAnimPercent(&w->anim) >= 100) {
                        endAnimStart(&w->anim, (u32) endBisuState3Anim, 0x12C, 0.8f, 2);
                        w->state = 5;
                    }
                    break;
                case 2:
                    if (endAnimEnded(&w->anim)) {
                        endAnimStart(&w->anim, (u32) endBisuState2Anim, 0x12C, 0.8f, 1);
                        w->state = 4;
                    }
                    break;
                case 4:
                    if (endAnimPercent(&w->anim) >= 100) {
                        endAnimStart(&w->anim, (u32) endBisuState4Anim, 0x12C, 0.8f, 2);
                        w->state = 5;
                    }
                    break;
                case 5:
                    break;
                case 6:
                    break;
            }
            endAnimStep(&w->anim);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *endBisuCreate(Obj *obj) {
    return tskCreate(obj, endBisuProc, obj->file, NULL);
}

void endBisuSetPos(Obj *obj, f32 x, f32 y, f32 z) {
    AnimeFrame *f = ((BisuWork *) (obj + 1))->anim.frame;

    f->pos[0] = x;
    f->pos[1] = y;
    f->pos[2] = z;
}

void endBisuSetRot(Obj *obj, f32 angle) {
    AnimeFrame *f = ((BisuWork *) (obj + 1))->anim.frame;

    f->rot[1] = angle;
}

void endBisuSetState(Obj *obj, s32 state) {
    ((BisuWork *) (obj + 1))->state = state;
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

s32 endAligProc(Obj *obj, s16 msg, void *arg) {
    AligWork *w = (AligWork *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "alig";
            w->frame = (AmbFrame *) frmCreate(gFrmLayers[0], gFrmKindAmbi);
            w->dr = w->dg = w->db = 0;
            w->r = w->g = w->b = 0x80;
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

Obj *endAligCreate(Obj *obj) {
    return tskCreate(obj, endAligProc, obj->file, NULL);
}

void endAligSetState(Obj *obj, s32 state) {
    ((AligWork *) (obj + 1))->state = state;
}

extern u8 endZashikiRoomDl[]; /* the room's display list (file 0xC2) */

/* The room ("座敷"): a pol frame of file 0xC2. */
s32 endZashikiProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    DaiWork *w = (DaiWork *) (obj + 1);
    PolFrame *f;

    switch (msg) {
        case 2:
            obj->name = "座敷";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            f = w->frame;
            demoPolSetDL(f, (Gfx *) endZashikiRoomDl, 0xC2);
            f->pos[0] = 0.0f;
            f->pos[1] = 0.0f;
            f->pos[2] = 0.0f;
            f->scale[0] = 0.75f;
            f->scale[1] = 0.75f;
            f->scale[2] = 0.75f;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 0:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *endZashikiCreate(Obj *obj) {
    return tskCreate(obj, endZashikiProc, obj->file, NULL);
}

extern u8 endLabDl[]; /* the lab's display list (file 0xC2) */

/* The lab ("lab"): a pol frame of file 0xC2. */
s32 endLabProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    DaiWork *w = (DaiWork *) (obj + 1);
    PolFrame *f;

    switch (msg) {
        case 2:
            obj->name = "lab";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            f = w->frame;
            demoPolSetDL(f, (Gfx *) endLabDl, 0xC2);
            f->pos[0] = 0.0f;
            f->pos[1] = 0.0f;
            f->pos[2] = 0.0f;
            f->scale[0] = 0.75f;
            f->scale[1] = 0.75f;
            f->scale[2] = 0.75f;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 0:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *endLabCreate(Obj *obj) {
    return tskCreate(obj, endLabProc, obj->file, NULL);
}

/* "mac2": a pol frame (file 0x12B) and an anime frame (file 0xC2) at z 35, turned round. */
typedef struct Mac2Work {
    /* 0x00 */ AnimPlay anim;
    /* 0x14 */ PolFrame *pol;
    /* 0x18 */ s32 state; /* 0 jumps to the animation's end */
} Mac2Work;

extern u8 endMac2Dl[]; /* mac2's display list (file 0x12B) */
extern u8 endMac2Anim[]; /* its animation (file 0xC2) */

/* "mac2": a model (file 0x12B's display list) with an animated copy over it (file 0xC2), both
 * at scale 0.67, z 35, turned half round. The anim's scales are a double 0.67: written 0.67f
 * on both sides, the NULL check between them leaves IDO one more 0.67f in .rodata that
 * nothing loads. */
s32 endMac2Proc(Obj *obj, s16 msg, void *arg) {
    Mac2Work *w = (Mac2Work *) (obj + 1);
    AnimeFrame *f;
    PolFrame *p;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "mac2";
            w->pol = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->pol == NULL) {
                return -1;
            }
            p = w->pol;
            demoPolSetDL(p, (Gfx *) endMac2Dl, 0x12B);
            p->scale[0] = 0.67f;
            p->scale[1] = 0.67f;
            p->scale[2] = 0.67f;
            p->pos[0] = 0.0f;
            p->pos[1] = 0.0f;
            p->pos[2] = 35.0f;
            p->rot[1] = 3.1415927f;
            w->anim.frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->anim.frame == NULL) {
                return -1;
            }
            endAnimStart(&w->anim, (u32) endMac2Anim, 0xC2, 0.0f, 1);
            f = w->anim.frame;
            f->scale[0] = 0.67;
            f->scale[1] = 0.67;
            f->scale[2] = 0.67;
            f->pos[0] = 0.0f;
            f->pos[1] = 0;
            f->pos[2] = 35.0f;
            f->rot[1] = 3.1415927f;
            w->anim.speed = 0.0f;
            w->state = 1;
            break;
        case 1:
            frmDestroy((Frame *) w->pol);
            frmDestroy((Frame *) w->anim.frame);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->anim.frame->frame = polAnimeLastFrame(w->anim.frame);
                    w->state = 1;
                    break;
                case 1:
                    break;
            }
            endAnimStep(&w->anim);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *endMac2Create(Obj *obj) {
    return tskCreate(obj, endMac2Proc, obj->file, NULL);
}

void endMac2SetState(Obj *obj, s32 state) {
    ((Mac2Work *) (obj + 1))->state = state;
}

/* A player's control ("pl制御"), a child of the player: runs its acts (states 0-6 start
 * them, 7-13 run them, 15 idles), walks it to a point (6), and its face animations. */
typedef struct PlCtlWork {
    /* 0x00 */ Obj *pl;
    /* 0x04 */ s32 state;
    /* 0x08 */ f32 x; /* where it walks to */
    /* 0x0C */ f32 y;
    /* 0x10 */ f32 z;
    /* 0x14 */ f32 vx;
    /* 0x18 */ f32 vy;
    /* 0x1C */ f32 vz;
    /* 0x20 */ Anim eyes; /* the face's texture animations, segments 6 and 7 */
    /* 0x30 */ Anim mouth;
    /* 0x40 */ s32 count; /* frames of face left; -1 forever */
    /* 0x44 */ s32 timer;
    /* 0x48 */ s32 flags; /* 1 the face runs; 2 and 4 the face to go back to */
} PlCtlWork;

/* The characters' face animations (demoTexAnimK0-K3), by character. */
void (*endFaceAnimFuncs[])(Anim *a, s32 which, u32 *out) = { demoTexAnimK0, demoTexAnimK1, demoTexAnimK2, demoTexAnimK3 };
/* The face each character goes back to, by character and flag (2 or 4); -1 for none. */
s32 endFaceReturn[][2] = { { -1, 5 }, { -1, 0 }, { -1, 0 }, { -1, 0 }, { 0, 0 } };

void endPlCtlFaceStep(Obj *obj);
s32 endPlCtlProc(Obj *obj, s16 msg, void *arg);

/* MATCH (see the end). History: the player p was spilled (frame 0x28) where the game keeps it in s0 and the
 * work in s1 with no frame word at all (frame 0x20, obj left in its home). Measured: the work
 * read through obj (w below) and p a block local of case 0, 313 lines (mix 11, obj in s0);
 * w a local at its declaration and p set in case 0, 317 (mix 13; the same with p declared
 * first, p a block local, or dx/dz named); p set at its declaration 371; no locals at all (both
 * re-derived) 368, mix 46 (frame right, obj in s0).
 * 2026-10-12 (from real 129, the w macro form): the game sets w in case 0 AND in case
 * 2 (lw s1,0x20(sp); addiu s1,s1,0x20 in both) and keeps it in s1, p in s0, obj in its home.
 * w a local set in case 0 only, case 2 through casts (143); set in both cases (143); then FAKE
 * "if (w != NULL) {}" after case 0's w (125, kept: obj now homed and w in its own register,
 * case 0's head the game's code). Left: p in a2, spilled around each call, where the game gives
 * it s0. From 125, no change: p a function-level local, declared first, "register", an empty
 * "if (p != NULL) {}"; "results" (best 125); "splits" (nothing).
 * 2026-10-12 (second try), from 125: with the FAKE kept, no p local (PL(w->pl) at each use): 131,
 * length +24 (reloaded at each use).
 * 2026-10-12 (third try): the game's frame 0x20 has no local words at all (s0, s1, ra only); this
 * C's extra 8 is p's spill. p declared before w with no FAKE: 143 (obj back in s0).
 * 2026-10-12 (fourth try): case 6's distance with the z term first ("(pos[2] - z)^2 + (pos[0] -
 * x)^2"): its floats are the game's (x and pos[0] loaded first, f0 = dx), 125 to 123, kept. The
 * same FAKE "if (w != NULL) {}" in case 2: no change (125). Read from the objects: the game's case
 * 2 keeps w in s1 as case 0 does ("lw s1,0x20(sp); sw zero,0x68(s1); addiu s1,s1,0x20"), where
 * this C folds w into obj's offsets there; so in the game both cases' w are one register
 * variable, as if live across the whole switch.
 * Then w set at its declaration (no FAKE, case 2 through it): 123, the same, and case 2 now keeps
 * w in its register as the game (kept: the FAKE is gone). From there: p set at its declaration
 * too 283; p declared before w 123 (no change). Left: p in a2, spilled around the calls, w in
 * s0, where the game colours p first (s0) and w second (s1).
 * MATCH (2026-10-12), from 123: the permuter's best (score 325; its first finds 4688)
 * was state 0's
 * break as "do { break; } while (0)", i.e. state 0 falling into state 7: real 7, p in s0 and w in
 * s1 as the game, but the game has the break. So p must be live across state 0's plAnimeSet
 * call: an empty "if (p) {}" after "w->state = 7" did it but left a beqz (9); the same empty if
 * between the call and the state store emits nothing (6, FAKE). "if (p) {}" after the inner
 * switch instead: p in s1 and w in s0 (61; p live everywhere ranks below w); a PRINTF of p
 * there: no effect (if (0) code does not count). Then case 2's stores as flags, pl, state (the
 * game's ugen order: obj, parent, 15; as1 still stores state before pl): MATCH. */
s32 endPlCtlProc(Obj *obj, s16 msg, void *arg) {
    PlCtlWork *w = (PlCtlWork *) (obj + 1);
    Player *p;

    switch (msg) {
        case 0: {
            p = PL(w->pl);

            switch (w->state) {
                case 0:
                    plAnimeSet(p, 0xEB);
                    if (p) { /* FAKE: keeps p live across the call (the game colours p first, s0) */
                    }
                    w->state = 7;
                    break;
                case 7:
                    if (plAnimEnded(p)) {
                        plAnimeSet(p, 0xEC);
                        w->state = 15;
                    }
                    break;
                case 1:
                    if (p->animStatus & 2) {
                        plAnimeSet(p, 0xEB);
                        p->animStep = -p->animStep;
                        p->model->frame = polAnimeLastFrame(p->model);
                        w->state = 8;
                    }
                    break;
                case 8:
                    if (plAnimEnded(p)) {
                        plAnimeSet(p, 0);
                        w->state = 15;
                    }
                    break;
                case 2:
                    plAnimeSet(p, 0x132);
                    w->state = 9;
                    break;
                case 9:
                    if (plAnimEnded(p)) {
                        plAnimeSet(p, 0x131);
                        w->state = 15;
                    }
                    break;
                case 3:
                    plAnimeSet(p, 0x135);
                    w->timer = 30;
                    w->state = 10;
                    break;
                case 10:
                    if (--w->timer == 0) {
                        w->state = 11;
                    }
                    break;
                case 11:
                    if (p->animStatus & 2) {
                        plAnimeSet(p, 0x134);
                        w->state = 12;
                    }
                    break;
                case 12:
                    if (plAnimEnded(p)) {
                        plAnimeSet(p, 0x133);
                        w->state = 15;
                    }
                    break;
                case 4:
                    plAnimeSet(p, 0x136);
                    w->state = 15;
                    break;
                case 5:
                    plAnimeSet(p, 0x138);
                    w->state = 13;
                    break;
                case 13:
                    if (plAnimEnded(p)) {
                        plAnimeSet(p, 0x137);
                        w->state = 15;
                    }
                    break;
                case 6:
                    if ((*sqrtf)((p->model->pos[2] - w->z) * (p->model->pos[2] - w->z) +
                                 (p->model->pos[0] - w->x) * (p->model->pos[0] - w->x)) < 5.0f) {
                        w->state = 15;
                    } else {
                        p->model->pos[0] += w->vx;
                        p->model->pos[2] += w->vz;
                    }
                    break;
                case 14:
                    break;
                case 15:
                    break;
            }
            endPlCtlFaceStep(obj);
            break;
        }
        case 1:
            break;
        case 2:
            obj->name = "pl制御";
            PRINTF("dplayer: TM_CREATE: %08x\n", obj);
            w->flags = 0;
            w->pl = obj->parent;
            w->state = 15;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *endPlCtlCreate(Obj *obj) {
    return tskCreate(obj, endPlCtlProc, obj->file, NULL);
}

void endPlCtlSetState(Obj *obj, s32 state) {
    ((PlCtlWork *) (obj + 1))->state = state;
}

/* Walks the player to (x, y, z) at `speed`. */
void endPlCtlWalkTo(Obj *obj, f32 x, f32 y, f32 z, f32 speed) {
    PlCtlWork *w = (PlCtlWork *) (obj + 1);
    Player *p;
    s32 pad;
    f32 dx;
    f32 dz;
    f32 k;

    w->x = x;
    w->y = y;
    w->z = z;
    p = PL(w->pl);
    dx = x - p->model->pos[0];
    dz = z - p->model->pos[2];
    k = 1.0f / (*sqrtf)(dx * dx + dz * dz);
    w->state = 6;
    w->vx = dx * k * speed;
    w->vz = dz * k * speed;
    w->vy = 0.0f;
}

/* Stops the player's animation. */
void endPlCtlStopAnime(Obj *obj) {
    Player *p = PL(((PlCtlWork *) (obj + 1))->pl);

    p->animStep = 0.0f;
    p->animStepAdd = 0.0f;
}

/* Runs face animations `eyes` and `mouth` for `time` frames, then goes back to the face that
 * `mode` names (1 or 2). */
void endPlCtlFaceStart(Obj *obj, s32 time, s32 mode, s32 eyes, s32 mouth) {
    PlCtlWork *w = (PlCtlWork *) (obj + 1);
    s32 pad;
    Player *p = PL(w->pl);

    endFaceAnimFuncs[p->chara](&w->eyes, eyes, &p->model->segs[3]);
    endFaceAnimFuncs[p->chara](&w->mouth, mouth, &p->model->segs[4]);
    w->count = time;
    p->flags |= 0x1000000;
    w->flags |= 1;
    if (mode & 1) {
        w->flags |= 2;
    } else if (mode & 2) {
        w->flags |= 4;
    }
}

/* Ends the face animations. */
void endPlCtlFaceEnd(Obj *obj) {
    PlCtlWork *w = (PlCtlWork *) (obj + 1);

    w->flags &= ~7;
    PL(w->pl)->flags |= 0x1000000;
}

/* Steps the face animations; at their end, the face to go back to. */
void endPlCtlFaceStep(Obj *obj) {
    PlCtlWork *w = (PlCtlWork *) (obj + 1);
    s32 face;
    Player *p = PL(w->pl);
    s32 i;

    if ((w->flags & 1) && w->count > 0) {
        fadeAnimStep(&w->eyes);
        fadeAnimStep(&w->mouth);
        if (w->count != -1) {
            w->count--;
        }
        if (w->count == 0) {
            if (!(w->flags & 6)) {
                endPlCtlFaceEnd(obj);
            } else if (w->flags & 2) {
                i = 0;
            } else if (w->flags & 4) {
                i = 1;
            }
            face = endFaceReturn[p->chara][i];
            if (face != -1) {
                endPlCtlFaceStart(obj, -1, 0, face, face);
            } else {
                p->flags &= ~0x1000000;
            }
        }
    }
}

/* A flash ("b閃光"): an anime frame of file 0xEE on layer 8, drawn translucent in its prim and
 * env colours, its alpha moving by a step. States 0 and 1 start a flash, 2 and 3 run it, 4
 * idles. */
typedef struct FlashWork {
    /* 0x00 */ AnimPlay anim;
    /* 0x14 */ s32 state;
    /* 0x18 */ s32 alpha;
    /* 0x1C */ s32 step; /* added to alpha each frame, 0 at either end */
    /* 0x20 */ s32 primR;
    /* 0x24 */ s32 primG;
    /* 0x28 */ s32 primB;
    /* 0x2C */ s32 envR;
    /* 0x30 */ s32 envG;
    /* 0x34 */ s32 envB;
    /* 0x38 */ s32 timer;
} FlashWork;

extern u8 endFlashAnim[]; /* the flash's animations (file 0xEE) */
extern u8 endFlashLoopAnim[];

s32 endFlashProc(Obj *obj, s16 msg, void *arg) {
    FlashWork *w = (FlashWork *) (obj + 1);
    AnimeFrame *f;

    switch (msg) {
        case 2:
            obj->name = "b閃光";
            w->anim.frame = (AnimeFrame *) frmCreate(gFrmLayer8, gFrmKindAnime);
            if (w->anim.frame == NULL) {
                return -1;
            }
            endAnimStart(&w->anim, (u32) endFlashAnim, 0xEE, 0.0f, 1);
            f = w->anim.frame;
            f->scale[0] = f->scale[1] = f->scale[2] = 0.1f;
            f->pos[0] = f->pos[1] = f->pos[2] = 0.0f;
            f->rot[0] = f->rot[1] = f->rot[2] = 0.0f;
            w->alpha = 0;
            w->step = 0;
            w->primR = 255;
            w->primG = 255;
            w->primB = 255;
            w->envR = 255;
            w->envG = 255;
            w->envB = 255;
            w->state = 4;
            break;
        case 1:
            frmDestroy((Frame *) w->anim.frame);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    endAnimStart(&w->anim, (u32) endFlashAnim, 0xEE, 0.25f, 1);
                    f = w->anim.frame;
                    f->scale[0] = f->scale[1] = f->scale[2] = 0.25f;
                    w->alpha = 0;
                    w->step = 10;
                    w->state = 2;
                    break;
                case 2:
                    if (endAnimPercent(&w->anim) >= 100) {
                        endAnimStart(&w->anim, (u32) endFlashLoopAnim, 0xEE, 0.25f, 2);
                        w->timer = 60;
                        w->state = 4;
                    }
                    break;
                case 1:
                    endAnimStart(&w->anim, (u32) endFlashAnim, 0xEE, 0.35f, 1);
                    f = w->anim.frame;
                    f->scale[0] = f->scale[1] = f->scale[2] = 0.67f;
                    w->alpha = 0;
                    w->step = 10;
                    w->primR = 255;
                    w->primG = 255;
                    w->primB = 100;
                    w->state = 3;
                    break;
                case 3:
                    if (endAnimPercent(&w->anim) >= 100) {
                        endAnimStart(&w->anim, (u32) endFlashLoopAnim, 0xEE, 0.35f, 2);
                        w->timer = 60;
                        w->state = 4;
                    }
                    break;
                case 4:
                    break;
                case 5:
                    break;
            }
            endAnimStep(&w->anim);
            w->alpha += w->step;
            if (w->alpha >= 255) {
                w->step = 0;
                w->alpha = 255;
            } else if (w->alpha <= 0) {
                w->step = 0;
                w->alpha = 0;
            }
            break;
        case 3:
            w->anim.frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCycleType(gGfxDlHead++, G_CYC_1CYCLE);
            gDPSetRenderMode(gGfxDlHead++, G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2);
            gDPSetTexturePersp(gGfxDlHead++, G_TP_PERSP);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0,
                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0);
            gDPSetAlphaCompare(gGfxDlHead++, G_AC_THRESHOLD);
            gDPPipeSync(gGfxDlHead++);
            gDPSetBlendColor(gGfxDlHead++, 0, 0, 0, 1);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->primR, w->primG, w->primB, 255);
            gDPSetEnvColor(gGfxDlHead++, w->envR, w->envG, w->envB, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *endFlashCreate(Obj *obj) {
    return tskCreate(obj, endFlashProc, obj->file, NULL);
}

void endFlashSetState(Obj *obj, s32 state) {
    ((FlashWork *) (obj + 1))->state = state;
}

void endFlashSetPos(Obj *obj, f32 x, f32 y, f32 z) {
    AnimeFrame *f = ((FlashWork *) (obj + 1))->anim.frame;

    f->pos[0] = x;
    f->pos[1] = y;
    f->pos[2] = z;
}

void endFlashSetRot(Obj *obj, f32 x, f32 y, f32 z) {
    AnimeFrame *f = ((FlashWork *) (obj + 1))->anim.frame;

    f->rot[0] = x;
    f->rot[1] = y;
    f->rot[2] = z;
}

void endFlashSetSpeed(Obj *obj, f32 speed) {
    ((FlashWork *) (obj + 1))->anim.speed = speed;
}

/* A fireball ("Fball"): a pol frame of file 0x120 that grows by 10% a frame for 90 frames
 * (state 0 starts it), drawn translucent in its prim and env colours. */
typedef struct FballWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ s32 state;
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 grow;
    /* 0x10 */ s32 timer;
    /* 0x14 */ s32 alpha;
    /* 0x18 */ s32 step; /* added to alpha each frame */
    /* 0x1C */ s32 primR;
    /* 0x20 */ s32 primG;
    /* 0x24 */ s32 primB;
    /* 0x28 */ s32 envR;
    /* 0x2C */ s32 envG;
    /* 0x30 */ s32 envB;
} FballWork;

extern u8 endFballDl[]; /* the fireball's display list (file 0x120) */

s32 endFballProc(Obj *obj, s16 msg, void *arg);

s32 endFballProc(Obj *obj, s16 msg, void *arg) {
    PolFrame *fr;
    FballWork *w = (FballWork *) (obj + 1);
    PolFrame *f;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "Fball";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            f = w->frame;
            demoPolSetDL(f, (Gfx *) endFballDl, 0x120);
            f->scale[0] = 0.1f;
            f->scale[1] = 0.1f;
            f->scale[2] = 0.1f;
            f->pos[0] = 0.0f;
            f->pos[1] = 0.0f;
            f->pos[2] = 0.0f;
            w->alpha = 255;
            w->step = 0;
            w->primR = 255;
            w->primG = 255;
            w->primB = 0x80;
            w->envR = 255;
            w->envG = 0x9B;
            w->envB = 0x1C;
            w->state = 0;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 0:
            fr = w->frame;
            switch (w->state) {
                case 0:
                    w->grow = 1.1f;
                    w->state = 1;
                    w->timer = 90;
                    w->scale = 0.1f;
                    break;
                case 1:
                    w->scale *= w->grow;
                    if (--w->timer == 0) {
                        w->state = 2;
                    }
                    break;
                case 2:
                    break;
            }
            fr->scale[0] = fr->scale[1] = fr->scale[2] = w->scale;
            w->alpha += w->step;
            if (w->alpha < 0) {
                w->alpha = 0;
                w->step = 0;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCycleType(gGfxDlHead++, G_CYC_1CYCLE);
            gDPSetRenderMode(gGfxDlHead++, G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2);
            gDPSetTexturePersp(gGfxDlHead++, G_TP_PERSP);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0,
                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0);
            gDPSetAlphaCompare(gGfxDlHead++, G_AC_THRESHOLD);
            gDPPipeSync(gGfxDlHead++);
            gDPSetBlendColor(gGfxDlHead++, 0, 0, 0, 1);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->primR, w->primG, w->primB, 255);
            gDPSetEnvColor(gGfxDlHead++, w->envR, w->envG, w->envB, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *endFballCreate(Obj *obj) {
    return tskCreate(obj, endFballProc, obj->file, NULL);
}

void endUnusedStub8(void) {
}

void endFballSetPos(Obj *obj, f32 x, f32 y, f32 z) {
    PolFrame *f = ((FballWork *) (obj + 1))->frame;

    f->pos[0] = x;
    f->pos[1] = y;
    f->pos[2] = z;
}

extern u8 endCastleDl[]; /* the castle's display list (file 0xC1) */

/* The castle ("castle"): a pol frame of file 0xC1. */
s32 endCastleProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    DaiWork *w = (DaiWork *) (obj + 1);
    PolFrame *f;

    switch (msg) {
        case 2:
            obj->name = "castle";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            f = w->frame;
            demoPolSetDL(f, (Gfx *) endCastleDl, 0xC1);
            f->pos[0] = 0.0f;
            f->pos[1] = 0.0f;
            f->pos[2] = 0.0f;
            f->scale[0] = 0.1f;
            f->scale[1] = 0.1f;
            f->scale[2] = 0.1f;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 0:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *endCastleCreate(Obj *obj) {
    return tskCreate(obj, endCastleProc, obj->file, NULL);
}

extern u8 endCastleGroundDl[]; /* the castle ground's display list (file 0xC0) */

/* The castle ground ("城地面"): a pol frame of file 0xC0. */
s32 endCastleGroundProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    DaiWork *w = (DaiWork *) (obj + 1);
    PolFrame *f;

    switch (msg) {
        case 2:
            obj->name = "城地面";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            f = w->frame;
            demoPolSetDL(f, (Gfx *) endCastleGroundDl, 0xC0);
            f->pos[0] = 0.0f;
            f->pos[1] = 0.0f;
            f->pos[2] = 0.0f;
            f->scale[0] = 1.0f;
            f->scale[1] = 1.0f;
            f->scale[2] = 1.0f;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 0:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *endCastleGroundCreate(Obj *obj) {
    return tskCreate(obj, endCastleGroundProc, obj->file, NULL);
}

/* The castle gate ("城門"): an anime frame of file 0xC1. */
typedef struct GateWork {
    /* 0x00 */ AnimPlay anim;
} GateWork;

extern u8 endCastleGateAnim[]; /* the gate's animation (file 0xC1) */

s32 endCastleGateProc(Obj *obj, s16 msg, void *arg) {
    GateWork *w = (GateWork *) (obj + 1);
    AnimeFrame *f;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "城門";
            w->anim.frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->anim.frame == NULL) {
                return -1;
            }
            endAnimStart(&w->anim, (u32) endCastleGateAnim, 0xC1, 0.0f, 1);
            f = w->anim.frame;
            f->scale[0] = 0.2f;
            f->scale[1] = 0.2f;
            f->scale[2] = 0.2f;
            f->pos[0] = 0.0f;
            f->pos[1] = 0.0f;
            f->pos[2] = 0.0f;
            break;
        case 1:
            frmDestroy((Frame *) w->anim.frame);
            break;
        case 0:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *endCastleGateCreate(Obj *obj) {
    return tskCreate(obj, endCastleGateProc, obj->file, NULL);
}

/* The impact ("impact"): an anime frame of file 0xBD; state 0 plays its second animation. */
typedef struct ImpactWork {
    /* 0x00 */ AnimPlay anim;
    /* 0x14 */ s32 state;
} ImpactWork;

extern u8 endImpactAnim[]; /* the impact's animations (file 0xBD) */
extern u8 endImpactState0Anim[];

s32 endImpactProc(Obj *obj, s16 msg, void *arg) {
    ImpactWork *w = (ImpactWork *) (obj + 1);
    AnimeFrame *f;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "impact";
            w->anim.frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->anim.frame == NULL) {
                return -1;
            }
            endAnimStart(&w->anim, (u32) endImpactAnim, 0xBD, 0.0f, 1);
            f = w->anim.frame;
            f->scale[0] = 1.0f;
            f->scale[1] = 1.0f;
            f->scale[2] = 1.0f;
            f->pos[0] = 0.0f;
            f->pos[1] = 0.0f;
            f->pos[2] = 0.0f;
            w->state = 1;
            break;
        case 1:
            frmDestroy((Frame *) w->anim.frame);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    endAnimStart(&w->anim, (u32) endImpactState0Anim, 0xBD, 0.0f, 1);
                    w->state = 1;
                    break;
                case 1:
                    break;
            }
            endAnimStep(&w->anim);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *endImpactCreate(Obj *obj) {
    return tskCreate(obj, endImpactProc, obj->file, NULL);
}

void endImpactSetState(Obj *obj, s32 state) {
    ((ImpactWork *) (obj + 1))->state = state;
}

void endImpactSetPos(Obj *obj, f32 x, f32 y, f32 z) {
    AnimeFrame *f = ((ImpactWork *) (obj + 1))->anim.frame;

    f->pos[0] = x;
    f->pos[1] = y;
    f->pos[2] = z;
}

extern u8 endTreeDl[]; /* the trees' display list (file 0xC0) */

/* The trees ("tree"): a pol frame of file 0xC0. */
s32 endTreeProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    DaiWork *w = (DaiWork *) (obj + 1);
    PolFrame *f;

    switch (msg) {
        case 2:
            obj->name = "tree";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            f = w->frame;
            demoPolSetDL(f, (Gfx *) endTreeDl, 0xC0);
            f->pos[0] = 0.0f;
            f->pos[1] = 0.0f;
            f->pos[2] = 0.0f;
            f->scale[0] = 0.5f;
            f->scale[1] = 0.5f;
            f->scale[2] = 0.5f;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 0:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *endTreeCreate(Obj *obj) {
    return tskCreate(obj, endTreeProc, obj->file, NULL);
}

void endTreeSetPos(Obj *obj, f32 x, f32 y, f32 z) {
    PolFrame *f = ((DaiWork *) (obj + 1))->frame;

    f->pos[0] = x;
    f->pos[1] = y;
    f->pos[2] = z;
}

void endUnusedStub9(void) {
}

/* A task's arguments: a delay and a scenario or a sound. */
typedef struct EndTaskArgs {
    /* 0x00 */ s32 time;
    /* 0x04 */ s32 id;
} EndTaskArgs;

/* The scenario's control ("sc制御"): runs scenario `id`; with a time, ends after it. */
typedef struct ScCtlWork {
    /* 0x00 */ s32 time; /* frames left; 0 for no end */
} ScCtlWork;

s32 edSc(Obj *obj, s16 msg, void *arg) {
    ScCtlWork *w = (ScCtlWork *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "sc制御";
            w->time = ((EndTaskArgs *) arg)->time;
            scRunStart(((EndTaskArgs *) arg)->id, 0, 0);
            break;
        case 1:
            endScTask = NULL;
            scRunEnd();
            break;
        case 0:
            if (w->time != 0) {
                if (--w->time == 0) {
                    return -1;
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Runs scenario `id` (the one running ends), ending it after `time` frames if not 0. */
Obj *edScSet(Obj *obj, s32 id, s32 time) {
    EndTaskArgs args;

    args.time = time;
    args.id = id;
    if (endScTask != NULL) {
        tskDestroy(endScTask);
    }
    endScTask = tskCreate(obj, edSc, obj->file, &args);
    return endScTask;
}

void edScSetTime(Obj *obj, s32 time) {
    ((ScCtlWork *) (obj + 1))->time = time;
}

/* A sound after a delay ("sd制御"): plays `id` after `time` frames, then ends. */
typedef struct SdCtlWork {
    /* 0x00 */ s32 time;
    /* 0x04 */ s32 id;
} SdCtlWork;

s32 edSd(Obj *obj, s16 msg, void *arg) {
    SdCtlWork *w = (SdCtlWork *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "sd制御";
            w->time = ((EndTaskArgs *) arg)->time;
            w->id = ((EndTaskArgs *) arg)->id;
            break;
        case 0:
            if (w->time == 0) {
                PRINTF("edSd: %d\n", w->id);
                sndPlay(w->id);
            }
            if (--w->time < 0) {
                return -1;
            }
            break;
        case 1:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Plays sound `id` after `time` frames (a task of endmain). */
Obj *edSdSet(s32 id, s32 time) {
    EndTaskArgs args;
    s32 pad;
    Obj *main = endMainTask;

    if (main != NULL) {
        PRINTF("edSdSet: %08x\n", main);
        args.time = time;
        args.id = id;
        return tskCreate(main, edSd, main->file, &args);
    }
    return NULL;
}
