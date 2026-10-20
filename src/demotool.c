#include "common.h"
#include "object.h"
#include "player.h"
#include "game.h"
#include "camera.h"
#include "ascii.h"
#include "contpak.h"
#include "demotool.h"
#include "fade.h"
#include "file.h"
#include "fileload.h"
#include "gfxmgr.h"
#include "joypad.h"
#include "mathutil.h"
#include "model.h"
#include "polygon.h"
#include "tmanager.h"

/* The demo scripts' tools (overlay 9): texture animations for four kinds, frame stepping,
 * gfxReplace, and the debug tasks "model", "CAMchk", "JOYchk", "OBJMOVE" and "democam". */

extern Gfx *gGfxDlHead; /* the display-list buffer */

/* demotool.c's .data (0x801B45D0-0x801B5148, then IDO's 16-byte alignment), in address order:
 * the four kinds' texture animations (segment 4 the face, segment 3 the model's pieces), the
 * debug tasks' steps, and the display lists. No code names two of them (the frame list at
 * 0x801B4868 and the display list at 0x801B5088). */
AnimFrame gDemoGoemonTexAnim7[] = {
    { 0x04000840, 8 }, { 0, 0 },
};
AnimFrame gDemoGoemonTexAnim0[] = {
    { 0x04002100, 8 }, { 0x04002940, 6 }, { 0x04003180, 5 }, { 0x04000420, 5 }, { 0x04001080, 6 },
    { 0x04000420, 8 }, { 0x04003180, 6 }, { 0x04002940, 7 }, { 0x04002100, 8 }, { 0, 0 },
};
AnimFrame gDemoGoemonTexAnim1[] = {
    { 0x04002520, 13 }, { 0x04002D60, 13 }, { 0, 0 },
};
AnimFrame gDemoGoemonTexAnim8[] = {
    { 0x040035A0, 13 }, { 0, 0 },
};
AnimFrame gDemoGoemonTexAnim9[] = {
    { 0x03005350, 13 }, { 0, 0 },
};
AnimFrame gDemoGoemonTexAnim10[] = {
    { 0x03005770, 13 }, { 0, 0 },
};
AnimFrame gDemoGoemonTexAnim2[] = {
    { 0x03001570, 7 }, { 0x030000D0, 6 }, { 0x03001570, 7 }, { 0x03000910, 7 }, { 0x030004F0, 7 },
    { 0x030000D0, 8 }, { 0x03001570, 7 }, { 0x03000910, 9 }, { 0x03000D30, 8 }, { 0x03001150, 8 },
    { 0x030004F0, 7 }, { 0x03001570, 7 }, { 0x03000910, 8 }, { 0x030004F0, 9 }, { 0x03001570, 7 },
    { 0x030000D0, 7 }, { 0x03000910, 6 }, { 0x03001570, 7 }, { 0x030000D0, 7 }, { 0x03001570, 7 },
    { 0, 0 },
};
AnimFrame gDemoGoemonTexAnim3[] = {
    { 0x03002E30, 7 }, { 0x03001990, 6 }, { 0x030021D0, 7 }, { 0x03001DB0, 7 }, { 0x03002E30, 7 },
    { 0x03001990, 8 }, { 0x030021D0, 9 }, { 0x030025F0, 8 }, { 0x03002A10, 8 }, { 0x03002E30, 7 },
    { 0x03001DB0, 7 }, { 0x030021D0, 8 }, { 0x03001DB0, 9 }, { 0x03002E30, 7 }, { 0x03001990, 7 },
    { 0x030021D0, 6 }, { 0x03002E30, 7 }, { 0x03001990, 7 }, { 0x03002E30, 7 }, { 0, 0 },
};
AnimFrame gDemoGoemonTexAnim4[] = {
    { 0x030046F0, 7 }, { 0x03003250, 6 }, { 0x030046F0, 7 }, { 0x03003A90, 7 }, { 0x03003670, 7 },
    { 0x03003250, 8 }, { 0x030046F0, 7 }, { 0x03003A90, 9 }, { 0x03003EB0, 8 }, { 0x030042D0, 8 },
    { 0x030046F0, 7 }, { 0x03003670, 7 }, { 0x03003A90, 8 }, { 0x030046F0, 7 }, { 0x03003670, 9 },
    { 0x03003250, 7 }, { 0x030046F0, 7 }, { 0x03003A90, 6 }, { 0x03003250, 7 }, { 0x030046F0, 7 },
    { 0, 0 },
};
AnimFrame gDemoGoemonTexAnimUnused[] = {
    { 0x03001570, 2 }, { 0x03000D30, 7 }, { 0x03001570, 3 }, { 0x030004F0, 7 }, { 0x03000910, 7 },
    { 0x03001570, 2 }, { 0x030000D0, 7 }, { 0x03000910, 8 }, { 0, 0 },
};
AnimFrame gDemoGoemonTexAnim6[] = {
    { 0x03001150, 40 }, { 0x03000910, 7 }, { 0x030004F0, 7 }, { 0x030000D0, 70 },
    { 0x03001150, 10 }, { 0x03001150, 10 }, { 0x030000D0, 10 }, { 0x03000D30, 200 }, { 0, 0 },
};
AnimFrame gDemoGoemonTexAnim5[] = {
    { 0x03004F30, 60 }, { 0x03004B10, 7 }, { 0x03004F30, 10 }, { 0x03004B10, 7 },
    { 0x03004F30, 40 }, { 0x03004B10, 7 }, { 0x03004F30, 40 }, { 0, 0 },
};
AnimFrame gDemoEbisumaruTexAnim0[] = {
    { 0x04000000, 8 }, { 0x04000420, 6 }, { 0x04000C60, 5 }, { 0x04001080, 5 }, { 0x04001080, 8 },
    { 0x04000C60, 6 }, { 0x04000420, 7 }, { 0x04000000, 8 }, { 0, 0 },
};
AnimFrame gDemoEbisumaruTexAnim1[] = {
    { 0x0300A5D0, 13 }, { 0, 0 },
};
AnimFrame gDemoEbisumaruTexAnim6[] = {
    { 0x040018C0, 13 }, { 0, 0 },
};
AnimFrame gDemoEbisumaruTexAnim5[] = {
    { 0x03007030, 2 }, { 0x03006C10, 7 }, { 0x03005FB0, 570 }, { 0, 0 },
};
AnimFrame gDemoEbisumaruTexAnim2[] = {
    { 0x03005B90, 7 }, { 0x030063D0, 7 }, { 0x03007030, 7 }, { 0x03005FB0, 7 }, { 0x03007030, 7 },
    { 0x03005B90, 7 }, { 0x03007030, 7 }, { 0x030063D0, 7 }, { 0x030067F0, 7 }, { 0x03006C10, 7 },
    { 0x03007030, 7 }, { 0x03005FB0, 7 }, { 0x030063D0, 7 }, { 0x03005FB0, 7 }, { 0x03007030, 7 },
    { 0x03005B90, 7 }, { 0x030063D0, 7 }, { 0x03007030, 7 }, { 0x03005B90, 7 }, { 0x03007030, 7 },
    { 0, 0 },
};
AnimFrame gDemoEbisumaruTexAnim3[] = {
    { 0x03007450, 7 }, { 0x03007C90, 7 }, { 0x030088F0, 7 }, { 0x03007870, 7 }, { 0x030088F0, 7 },
    { 0x03007450, 7 }, { 0x030088F0, 7 }, { 0x03007C90, 7 }, { 0x030080B0, 7 }, { 0x030084D0, 7 },
    { 0x030088F0, 7 }, { 0x03007870, 7 }, { 0x03007C90, 7 }, { 0x03007870, 7 }, { 0x030088F0, 7 },
    { 0x03007450, 7 }, { 0x03007C90, 7 }, { 0x030088F0, 7 }, { 0x03007450, 7 }, { 0x030088F0, 7 },
    { 0, 0 },
};
AnimFrame gDemoEbisumaruTexAnim4[] = {
    { 0x03008D10, 7 }, { 0x03009550, 7 }, { 0x0300A1B0, 7 }, { 0x03009130, 7 }, { 0x0300A1B0, 7 },
    { 0x03008D10, 7 }, { 0x0300A1B0, 7 }, { 0x03009550, 7 }, { 0x03009970, 7 }, { 0x03009D90, 7 },
    { 0x0300A1B0, 7 }, { 0x03009130, 7 }, { 0x03009550, 7 }, { 0x03009130, 7 }, { 0x0300A1B0, 7 },
    { 0x03008D10, 7 }, { 0x03009550, 7 }, { 0x0300A1B0, 7 }, { 0x03008D10, 7 }, { 0x0300A1B0, 7 },
    { 0, 0 },
};
AnimFrame gDemoSasukeTexAnim0[] = {
    { 0x04000C60, 8 }, { 0x04001080, 6 }, { 0x04001CE0, 5 }, { 0x04002520, 5 }, { 0x04002520, 8 },
    { 0x04001CE0, 6 }, { 0x04001080, 7 }, { 0x04000C60, 8 }, { 0, 0 },
};
AnimFrame gDemoSasukeTexAnim1[] = {
    { 0x040018C0, 13 }, { 0, 0 },
};
AnimFrame gDemoSasukeTexAnim5[] = {
    { 0x040014A0, 13 }, { 0, 0 },
};
AnimFrame gDemoSasukeTexAnim2[] = {
    { 0x0300A9F0, 3 }, { 0x0300B230, 4 }, { 0x0300AE10, 4 }, { 0x0300A9F0, 3 }, { 0x0300B230, 4 },
    { 0x0300B650, 4 }, { 0x0300BA70, 2 }, { 0x0300AE10, 4 }, { 0x0300B230, 4 }, { 0x0300AE10, 4 },
    { 0x0300A9F0, 3 }, { 0x0300B230, 4 }, { 0x0300A9F0, 3 }, { 0, 0 },
};
AnimFrame gDemoSasukeTexAnim3[] = {
    { 0x0300C2B0, 3 }, { 0x0300CAF0, 4 }, { 0x0300C6D0, 4 }, { 0x0300C2B0, 3 }, { 0x0300CAF0, 4 },
    { 0x0300CF10, 4 }, { 0x0300D330, 2 }, { 0x0300C6D0, 4 }, { 0x0300CAF0, 4 }, { 0x0300C6D0, 4 },
    { 0x0300C2B0, 3 }, { 0x0300CAF0, 4 }, { 0x0300C2B0, 3 }, { 0, 0 },
};
AnimFrame gDemoSasukeTexAnim4[] = {
    { 0x0300DB70, 3 }, { 0x0300E3B0, 4 }, { 0x0300DF90, 4 }, { 0x0300DB70, 3 }, { 0x0300E3B0, 4 },
    { 0x0300E7D0, 4 }, { 0x0300EBF0, 2 }, { 0x0300DF90, 4 }, { 0x0300E3B0, 4 }, { 0x0300DF90, 4 },
    { 0x0300DB70, 3 }, { 0x0300E3B0, 4 }, { 0x0300DB70, 3 }, { 0, 0 },
};
AnimFrame gDemoYaeTexAnim0[] = {
    { 0x04000420, 52 }, { 0x04000C60, 1 }, { 0x040014A0, 2 }, { 0x04002100, 4 },
    { 0x04000420, 52 }, { 0x04000C60, 1 }, { 0x040014A0, 2 }, { 0x04002100, 4 }, { 0x04000420, 5 },
    { 0x04000C60, 1 }, { 0x040014A0, 2 }, { 0x04002100, 4 }, { 0x04000420, 42 }, { 0, 0 },
};
AnimFrame gDemoYaeTexAnim8[] = {
    { 0x04000420, 10 }, { 0x04000C60, 10 }, { 0x040014A0, 400 },
};
AnimFrame gDemoYaeTexAnim5[] = {
    { 0x040018C0, 3 }, { 0x04001080, 3 }, { 0x04000840, 3 }, { 0x04001080, 3 }, { 0x040018C0, 3 },
    { 0x04002520, 3 }, { 0x04002520, 3 }, { 0, 0 },
};
AnimFrame gDemoYaeTexAnim1[] = {
    { 0x03014290, 13 }, { 0, 0 },
};
AnimFrame gDemoYaeTexAnim9[] = {
    { 0x030146B0, 13 }, { 0, 0 },
};
AnimFrame gDemoYaeTexAnim2[] = {
    { 0x0300F850, 6 }, { 0x03010CF0, 4 }, { 0x03010090, 8 }, { 0x03010CF0, 4 }, { 0x0300FC70, 10 },
    { 0x03010CF0, 4 }, { 0x0300F850, 12 }, { 0x03010090, 8 }, { 0x030104B0, 8 }, { 0x03010CF0, 4 },
    { 0x030108D0, 8 }, { 0x03010CF0, 3 }, { 0, 0 },
};
AnimFrame gDemoYaeTexAnim3[] = {
    { 0x03011110, 4 }, { 0x030125B0, 2 }, { 0x03011950, 5 }, { 0x030125B0, 2 }, { 0x03011530, 6 },
    { 0x030125B0, 2 }, { 0x03011110, 6 }, { 0x03011950, 5 }, { 0x03011D70, 5 }, { 0x030125B0, 4 },
    { 0x03012190, 2 }, { 0x030125B0, 2 }, { 0, 0 },
};
AnimFrame gDemoYaeTexAnim6[] = {
    { 0x030125B0, 2 }, { 0x03012190, 15 }, { 0x03011950, 15 }, { 0x03011530, 16 },
    { 0x03011950, 15 }, { 0x03011530, 400 }, { 0, 0 },
};
AnimFrame gDemoYaeTexAnim7[] = {
    { 0x030125B0, 2000 }, { 0, 0 },
};
AnimFrame gDemoYaeTexAnim4[] = {
    { 0x030129D0, 4 }, { 0x03013E70, 2 }, { 0x03013210, 5 }, { 0x03013E70, 2 }, { 0x03012DF0, 6 },
    { 0x03013E70, 2 }, { 0x030129D0, 6 }, { 0x03013210, 5 }, { 0x03013630, 5 }, { 0x03013E70, 4 },
    { 0x03013A50, 2 }, { 0x03013E70, 2 }, { 0, 0 },
};
AnimFrame gDemoYaeTexAnim10[] = {
    { 0x03014AD0, 2000 }, { 0, 0 },
};
f32 gObjMoveStep = 1.0f; /* the step of OBJMOVE */
f32 gCamChkStep = 1.0f; /* the step of CAMchk */
f32 gCamChkMarkerSize = 1.0f; /* the size of its marker */

Gfx gDemoColorSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                          G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetCombineLERP(PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0,
                       PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsDPPipeSync(),
    gsDPSetBlendColor(0, 0, 0, 1),
    gsSPEndDisplayList(),
};

Gfx gDemoModelXluDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(0xFFFF),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetEnvColor(255, 255, 255, 160),
    gsDPSetCombineLERP(0, 0, 0, SHADE, SHADE, 0, ENVIRONMENT, 0, 0, 0, 0, SHADE, SHADE, 0,
                       ENVIRONMENT, 0),
    gsSPEndDisplayList(),
};

Gfx gDemoEnvShadeXluDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(0xFFFF),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetCombineLERP(ENVIRONMENT, 0, 0, SHADE, ENVIRONMENT, 0, 0, SHADE, ENVIRONMENT, 0, 0,
                       SHADE, ENVIRONMENT, 0, 0, SHADE),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsDPPipeSync(),
    gsDPSetBlendColor(0, 0, 0, 1),
    gsSPEndDisplayList(),
};

Gfx gDemoShadeSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(0xFFFF),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_TEX_EDGE, G_RM_NOOP2),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPEndDisplayList(),
};

Gfx gDemoModulateSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(0xFFFF),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_TEX_EDGE, G_RM_NOOP2),
    gsDPSetCombineMode(G_CC_MODULATEIDECALA, G_CC_MODULATEIDECALA),
    gsSPEndDisplayList(),
};

/* What demoCamChkOpen passes to the "CAMchk" task. */
typedef struct CamChkArg {
    /* 0x00 */ u8 pad0[0x14];
    /* 0x14 */ f32 eye[3];
    /* 0x20 */ f32 at[3];
    /* 0x2C */ f32 fov; /* degrees */
} CamChkArg;

/* A demo actor's work: a player, a model, and two texture animations of one kind (0-3). */
typedef struct DemoActor {
    /* 0x00 */ Obj *player;
    /* 0x04 */ AnimeFrame *frame;
    /* 0x08 */ u8 pad8[4];
    /* 0x0C */ Anim anim[2];
    /* 0x2C */ s32 kind;
} DemoActor;

#define DEMOACT(obj) ((DemoActor *) ((u32) (obj) + sizeof(Obj)))

/* What a "model" task is made with. */
typedef struct ModelArg {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ f32 scale[3];
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ s32 set; /* the file set for its segments */
    /* 0x2C */ Gfx *dl;
    /* 0x30 */ u8 pad30[0xC];
    /* 0x3C */ s32 kind; /* 0 or 1: under the world; 2: under the other root. 1 and 2 add a
                          * display list when drawn */
} ModelArg;

/* A "model" task's work. */
typedef struct ModelWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad4[0x2C];
    /* 0x30 */ AsciiFrame *mpos; /* the checking model's lines */
    /* 0x34 */ AsciiFrame *scale;
    /* 0x38 */ u8 pad38[4];
    /* 0x3C */ s32 kind;
} ModelWork;

#define MODEL(task) ((ModelWork *) ((u32) (task) + sizeof(Obj)))

/* The "CAMchk" task's work: a camera to look through and a marker at its look-at point. */
typedef struct CamChk {
    /* 0x00 */ Camera *cam; /* its task */
    /* 0x04 */ PolFrame *marker;
    /* 0x08 */ AsciiFrame *pos; /* the lines showing the eye and the look-at point */
    /* 0x0C */ AsciiFrame *ipos;
    /* 0x10 */ CamView *view;
    /* 0x14 */ f32 eye[3];
    /* 0x20 */ f32 at[3];
    /* 0x2C */ u8 pad2C[4];
    /* 0x30 */ s32 timer;
    /* 0x34 */ f32 size; /* the marker's, pulsing */
} CamChk;

#define CAMCHK(task) ((CamChk *) ((u32) (task) + sizeof(Obj)))

extern Gfx camchkMarkerDl[]; /* the marker's display list, in file set 0xED */
extern u8 gDemoCamHold; /* 1 while a forced fade is on (the demo camera stands still) */
extern s8 gCamChkActive; /* 1 while CAMchk is open */


/* One of the camera's view values toward its target by its step: camera.c's, but the threshold
 * here is a double. */
#define CAM_MOVE(cur, target, step)             \
    if ((step) > 0.00001) {                     \
        if (fabsf((target) - (cur)) < (step)) { \
            (cur) = (target);                   \
            (step) = 0.0f;                      \
        } else if ((target) < (cur)) {          \
            (cur) = (cur) - (step);             \
        } else {                                \
            (cur) = (cur) + (step);             \
        }                                       \
    }

extern Obj *gDmenuLastObj; /* the DEMO SELECT menu (dmenu.c) */
extern Frame *gFrmTextRoot; /* the text frames' root */
extern Frame *gFrmLayers;
extern Frame *gFrmModelLayer; /* the world's root */
extern Frame *gFrmLayer8;
extern Frame *gFrmEffectLayer;
extern s16 gFrmKindPol; /* the "pol" kind */
extern s16 gFrmKindCamera;
extern s32 gDemoModelsDelete; /* 1 deletes the demo's models */
extern s32 gDemoModelsDelete2; /* and so does 1 here */
extern s32 gDemoForcedFade; /* bit 0: a forced fade is on */
extern f32 gCamChkEyeX; /* CAMchk's eye and look-at */
extern f32 gCamChkEyeY;
extern f32 gCamChkEyeZ;
extern f32 gCamChkAtX;
extern f32 gCamChkAtY;
extern f32 gCamChkAtZ;
extern s32 gJoyChkUp; /* JOYchk: controller 2 holds up */
extern s32 gJoyChkDown; /* down */
extern s32 gJoyChkRight; /* right */
extern s32 gJoyChkLeft; /* left */
extern s32 gJoyChkA; /* A */
extern s32 gJoyChkB; /* B */
extern s32 gJoyChkR; /* R */
extern s32 gJoyChkL; /* L */
extern s32 gJoyChkZ; /* Z */
extern s32 gObjMoveSel; /* OBJMOVE: the model chosen, -1 for none */
extern s32 gObjMoveCount; /* the number of models */
extern f32 gObjMovePosX; /* the chosen model's position and angle, as shown */
extern f32 gObjMovePosY;
extern f32 gObjMovePosZ;
extern f32 gObjMoveRot;
extern f32 gObjMoveDx; /* the moves this frame: x, y, z, angle */
extern f32 gObjMoveDy;
extern f32 gObjMoveDz;
extern f32 gObjMoveDrot;


/* A display list (returned) that draws with gDemoColorSetupDl's setup in a primitive colour and an
 * environment colour. */
Gfx *demoColorDL(s32 r, s32 g, s32 b, s32 er, s32 eg, s32 eb, s32 ea) {
    Gfx *dl = gGfxDlHead;

    gSPDisplayList(gGfxDlHead++, gDemoColorSetupDl);
    gDPSetPrimColor(gGfxDlHead++, 0, 0, r, g, b, 255);
    gDPSetEnvColor(gGfxDlHead++, er, eg, eb, ea);
    gSPEndDisplayList(gGfxDlHead++);
    return dl;
}

/* A display list (returned) calling gDemoShadeSetupDl. */
Gfx *demoShadeDL(void) {
    Gfx *dl = gGfxDlHead;

    gSPDisplayList(gGfxDlHead++, gDemoShadeSetupDl);
    gSPEndDisplayList(gGfxDlHead++);
    return dl;
}

/* A display list (returned) calling gDemoModulateSetupDl. */
Gfx *demoModulateDL(void) {
    Gfx *dl = gGfxDlHead;

    gSPDisplayList(gGfxDlHead++, gDemoModulateSetupDl);
    gSPEndDisplayList(gGfxDlHead++);
    return dl;
}

/* Texture animation `which` of kind 0x1B3 (file 0x1B3 or 0x28D) into *out. */
void demoTexAnimK0(Anim *a, s32 which, u32 *out) {
    switch (which) {
        case 0:
            (*fadeAnimStart)(a, gDemoGoemonTexAnim0, 0, out, 0x1B3);
            break;
        case 1:
            (*fadeAnimStart)(a, gDemoGoemonTexAnim1, 0, out, 0x1B3);
            break;
        case 2:
            (*fadeAnimStart)(a, gDemoGoemonTexAnim2, 0, out, 0x28D);
            break;
        case 3:
            (*fadeAnimStart)(a, gDemoGoemonTexAnim3, 0, out, 0x28D);
            break;
        case 4:
            (*fadeAnimStart)(a, gDemoGoemonTexAnim4, 0, out, 0x28D);
            break;
        case 5:
            (*fadeAnimStart)(a, gDemoGoemonTexAnim5, 0, out, 0x28D);
            break;
        case 6:
            (*fadeAnimStart)(a, gDemoGoemonTexAnim6, 0, out, 0x28D);
            break;
        case 7:
            (*fadeAnimStart)(a, gDemoGoemonTexAnim7, 0, out, 0x1B3);
            break;
        case 8:
            (*fadeAnimStart)(a, gDemoGoemonTexAnim8, 0, out, 0x1B3);
            break;
        case 9:
            (*fadeAnimStart)(a, gDemoGoemonTexAnim9, 0, out, 0x28D);
            break;
        case 10:
            (*fadeAnimStart)(a, gDemoGoemonTexAnim10, 0, out, 0x28D);
            break;
    }
}

/* The same for kind 0x1B4. */
void demoTexAnimK1(Anim *a, s32 which, u32 *out) {
    switch (which) {
        case 0:
            (*fadeAnimStart)(a, gDemoEbisumaruTexAnim0, 0, out, 0x1B4);
            break;
        case 1:
            (*fadeAnimStart)(a, gDemoEbisumaruTexAnim1, 0, out, 0x1B4);
            break;
        case 2:
            (*fadeAnimStart)(a, gDemoEbisumaruTexAnim2, 0, out, 0x28D);
            break;
        case 3:
            (*fadeAnimStart)(a, gDemoEbisumaruTexAnim3, 0, out, 0x28D);
            break;
        case 4:
            (*fadeAnimStart)(a, gDemoEbisumaruTexAnim4, 0, out, 0x28D);
            break;
        case 5:
            (*fadeAnimStart)(a, gDemoEbisumaruTexAnim5, 0, out, 0x28D);
            break;
        case 6:
            (*fadeAnimStart)(a, gDemoEbisumaruTexAnim6, 0, out, 0x1B4);
            break;
    }
}

/* The same for kind 0x1B5. */
void demoTexAnimK2(Anim *a, s32 which, u32 *out) {
    switch (which) {
        case 0:
            (*fadeAnimStart)(a, gDemoSasukeTexAnim0, 0, out, 0x1B5);
            break;
        case 1:
            (*fadeAnimStart)(a, gDemoSasukeTexAnim1, 0, out, 0x1B5);
            break;
        case 2:
            (*fadeAnimStart)(a, gDemoSasukeTexAnim2, 0, out, 0x28D);
            break;
        case 3:
            (*fadeAnimStart)(a, gDemoSasukeTexAnim3, 0, out, 0x28D);
            break;
        case 4:
            (*fadeAnimStart)(a, gDemoSasukeTexAnim4, 0, out, 0x28D);
            break;
        case 5:
            (*fadeAnimStart)(a, gDemoSasukeTexAnim5, 0, out, 0x1B5);
            break;
    }
}

/* The same for kind 0x1B6. */
void demoTexAnimK3(Anim *a, s32 which, u32 *out) {
    switch (which) {
        case 0:
            (*fadeAnimStart)(a, gDemoYaeTexAnim0, 0, out, 0x1B6);
            break;
        case 7:
            (*fadeAnimStart)(a, gDemoYaeTexAnim7, 0, out, 0x28D);
            break;
        case 1:
            (*fadeAnimStart)(a, gDemoYaeTexAnim1, 0, out, 0x28D);
            break;
        case 2:
            (*fadeAnimStart)(a, gDemoYaeTexAnim2, 0, out, 0x28D);
            break;
        case 3:
            (*fadeAnimStart)(a, gDemoYaeTexAnim3, 0, out, 0x28D);
            break;
        case 6:
            (*fadeAnimStart)(a, gDemoYaeTexAnim6, 0, out, 0x28D);
            break;
        case 4:
            (*fadeAnimStart)(a, gDemoYaeTexAnim4, 0, out, 0x28D);
            break;
        case 5:
            (*fadeAnimStart)(a, gDemoYaeTexAnim5, 0, out, 0x1B6);
            break;
        case 8:
            (*fadeAnimStart)(a, gDemoYaeTexAnim8, 0, out, 0x1B6);
            break;
        case 9:
            (*fadeAnimStart)(a, gDemoYaeTexAnim9, 0, out, 0x28D);
            break;
        case 10:
            (*fadeAnimStart)(a, gDemoYaeTexAnim10, 0, out, 0x28D);
            break;
    }
}

/* A "pol" frame's second display list and its segments (file set n). */
void demoPolSetDL(PolFrame *f, Gfx *dl, s32 n) {
    f->dl2 = dl;
    floadGetSetRam((void **) f->segs, n);
}

void demoNop(void) {
}

/* An "anime" frame's animation, its segments (file set n), and its frame at a fraction of the
 * way through. */
void demoAnimeSet(AnimeFrame *f, u32 anim, s32 n, f32 at) {
    f->anim = anim;
    floadGetSetRam((void **) f->segs, n);
    f->frame = polAnimeLastFrame(f) * at;
}

/* Steps an "anime" frame's animation by speed: mode 0 loops forward, 2 stops at the end, 1 loops
 * backward, 3 stops at the start. 1 when it wrapped or stopped. */
s32 demoAnimeStep(AnimeFrame *f, f32 speed, s32 mode) {
    if (mode == 0 || mode == 2) {
        f->frame += speed;
        if (polAnimeLastFrame(f) < f->frame) {
            if (mode == 0) {
                f->frame -= polAnimeLastFrame(f);
            } else {
                f->frame = polAnimeLastFrame(f);
            }
            return 1;
        }
    } else if (mode == 1 || mode == 3) {
        f->frame -= speed;
        if (f->frame < 0.0f) {
            if (mode == 1) {
                f->frame += polAnimeLastFrame(f);
            } else {
                f->frame = 0.0f;
            }
            return 1;
        }
    }
    return 0;
}

/* gfxReplace: swaps the model's display lists (from[i] becomes to[i]), then points two more
 * segments at two files. */
void gfxReplace(AnimeFrame *f, u32 *from, u32 file1, u32 *to, u32 file2) {
    (*mdlSwapDLs)(f->anim, (u8 *) f->segs[0], from, to);
    f->segs[1] = (*fileRamAddr)(file1);
    PRINTF("*** L->ani->seg[2]=%x\n", f->segs[1]);
    f->segs[2] = (*fileRamAddr)(file2);
    PRINTF("*** L->ani->seg[3]=%x\n", f->segs[2]);
    PRINTF("*** gfxReplace end\n");
}

/* Turns angle `rot` toward the direction from (tx, tz) to (x, z), by at most step degrees. */
f32 demoTurnToward(f32 x, f32 z, f32 tx, f32 tz, f32 rot, f32 step) {
    f32 target = mathAtan2(x - tx, z - tz) + PI;
    f32 diff;
    f32 dist;

    if (rot < target) {
        diff = target - rot;
        dist = diff;
    } else {
        diff = target - rot;
        dist = -diff;
    }
    if (diff > PI) {
        rot += 6.2831855f;
    } else if (diff < -PI) {
        rot -= 6.2831855f;
    }
    if (dist < step * 0.0174532930038800785) {
        return target;
    }
    if (target < rot) {
        rot = rot - step * 0.0174532930038800785;
    } else {
        rot = rot + step * 0.0174532930038800785;
    }
    return rot;
}

/* "model": a "pol" frame placed, scaled and turned as the argument says, drawing its display
 * list; it deletes itself when gDemoModelsDelete or gDemoModelsDelete2 is 1. */
s32 modelProc(Obj *obj, s16 msg, void *arg) {
    ModelWork *w = MODEL(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "model";
            switch (((ModelArg *) arg)->kind) {
                case 0:
                    w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
                    break;
                case 1:
                    w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
                    break;
                case 2:
                    w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
                    break;
            }
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, ((ModelArg *) arg)->dl, ((ModelArg *) arg)->set);
            w->frame->flags |= 0x20;
            w->frame->scale[0] = ((ModelArg *) arg)->scale[0];
            w->frame->scale[1] = ((ModelArg *) arg)->scale[1];
            w->frame->scale[2] = ((ModelArg *) arg)->scale[2];
            w->frame->pos[0] = ((ModelArg *) arg)->pos[0];
            w->frame->pos[1] = ((ModelArg *) arg)->pos[1];
            w->frame->pos[2] = ((ModelArg *) arg)->pos[2];
            w->frame->rot[0] = ((ModelArg *) arg)->rot[0];
            w->frame->rot[1] = ((ModelArg *) arg)->rot[1];
            w->frame->rot[2] = ((ModelArg *) arg)->rot[2];
            w->kind = ((ModelArg *) arg)->kind;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            PRINTF("モデルを消しました\n");
            break;
        case 3:
            switch (w->kind) {
                case 0:
                    break;
                case 1:
                    w->frame->dl = demoShadeDL();
                    break;
                case 2:
                    w->frame->dl = gDemoModelXluDl;
                    break;
            }
            break;
        case 0:
            if (gDemoModelsDelete == 1 || gDemoModelsDelete2 == 1) {
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The same, for checking a model: its position and scale are shown on two lines and move with
 * controller 4 (D-pad x and z, R and L y, the C buttons the scale); it blinks. The text root
 * gFrmTextRoot is read as element 10 of the root array at gFrmModelLayer: as its own global, IDO keeps
 * its address in s0 across the two line calls, where the game loads it with a lui for each. */
s32 modelChk(Obj *obj, s16 msg, void *arg) {
    ModelWork *w = MODEL(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "model";
            switch (((ModelArg *) arg)->kind) {
                case 0:
                    w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
                    break;
                case 1:
                    w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
                    break;
                case 2:
                    w->frame = (PolFrame *) frmCreate(gFrmEffectLayer, gFrmKindPol);
                    break;
            }
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, ((ModelArg *) arg)->dl, ((ModelArg *) arg)->set);
            w->frame->flags |= 0x20;
            w->frame->scale[0] = ((ModelArg *) arg)->scale[0];
            w->frame->scale[1] = ((ModelArg *) arg)->scale[1];
            w->frame->scale[2] = ((ModelArg *) arg)->scale[2];
            w->frame->pos[0] = ((ModelArg *) arg)->pos[0];
            w->frame->pos[1] = ((ModelArg *) arg)->pos[1];
            w->frame->pos[2] = ((ModelArg *) arg)->pos[2];
            w->frame->rot[0] = ((ModelArg *) arg)->rot[0];
            w->frame->rot[1] = ((ModelArg *) arg)->rot[1];
            w->frame->rot[2] = ((ModelArg *) arg)->rot[2];
            w->kind = ((ModelArg *) arg)->kind;
            w->mpos = asciiNew(((Frame **) &gFrmModelLayer)[10], 20, 50,
                                             "                                ");
            w->scale = asciiNew(((Frame **) &gFrmModelLayer)[10], 20, 60,
                                              "                                ");
            break;
        case 1:
            frmDestroy(&w->frame->f);
            PRINTF("モデルを消しました\n");
            break;
        case 3:
            switch (w->kind) {
                case 0:
                    break;
                case 1:
                    w->frame->dl = demoShadeDL();
                    break;
                case 2:
                    w->frame->dl = gDemoModelXluDl;
                    break;
            }
            break;
        case 0:
            if (gDemoModelsDelete == 1 || gDemoModelsDelete2 == 1) {
                return -1;
            }
            if ((u32) gGameState.frameCount % 10 >= 5) {
                w->frame->f.flags |= 2;
            } else {
                w->frame->f.flags &= ~2;
            }
            if (joyGetButton(3, 0) & U_JPAD) {
                w->frame->pos[2] = w->frame->pos[2] - 1.0f;
            }
            if (joyGetButton(3, 0) & D_JPAD) {
                w->frame->pos[2] += 1.0f;
            }
            if (joyGetButton(3, 0) & R_JPAD) {
                w->frame->pos[0] += 1.0f;
            }
            if (joyGetButton(3, 0) & L_JPAD) {
                w->frame->pos[0] -= 1.0f;
            }
            if (joyGetButton(3, 0) & R_TRIG) {
                w->frame->pos[1] += 1.0f;
            }
            if (joyGetButton(3, 0) & L_TRIG) {
                w->frame->pos[1] -= 1.0f;
            }
            if (joyGetButton(3, 0) & U_CBUTTONS) {
                w->frame->scale[2] -= 0.01;
            }
            if (joyGetButton(3, 0) & D_CBUTTONS) {
                w->frame->scale[2] += 0.01;
            }
            if (joyGetButton(3, 0) & R_CBUTTONS) {
                w->frame->scale[0] += 0.01;
            }
            if (joyGetButton(3, 0) & L_CBUTTONS) {
                w->frame->scale[0] -= 0.01;
            }
            sprintf(w->mpos->str, "MPOS   $ %5d %5d %5d ", (s32) w->frame->pos[0],
                               (s32) w->frame->pos[1], (s32) w->frame->pos[2]);
            sprintf(w->scale->str, "SCALE  $ %5d %5d %5d ",
                               (s32) (w->frame->scale[0] * 1000.0f),
                               (s32) (w->frame->scale[1] * 1000.0f),
                               (s32) (w->frame->scale[2] * 1000.0f));
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Makes a "model" task under parent (which 0), or a checking one (which 1). */
void demoModelCreate(Obj *parent, f32 x, f32 y, f32 z, f32 sx, f32 sy, f32 sz, f32 rx, f32 ry, f32 rz,
                            s32 set, Gfx *dl, s32 which, s32 kind) {
    ModelArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale[0] = sx;
    arg.scale[1] = sy;
    arg.scale[2] = sz;
    arg.rot[0] = rx;
    arg.rot[1] = ry;
    arg.rot[2] = rz;
    arg.set = set;
    arg.dl = dl;
    arg.kind = kind;
    switch (which) {
        case 0:
            tskCreate(parent, modelProc, parent->file, &arg);
            break;
        case 1:
            tskCreate(parent, modelChk, parent->file, &arg);
            break;
    }
}

/* "CAMchk": a camera at the given eye and look-at point with a pulsing marker at the look-at
 * point. Controller 1 moves them (D-pad, R and L the look-at point; the C buttons, A and B the
 * eye; with Z four times as far) and the two lines show them; Start closes it. */
s32 CAMchk(Obj *obj, s16 msg, void *arg) {
    CamChk *w = CAMCHK(obj);

    switch (msg) {
        case 2:
            obj->name = "CAMchk";
            PRINTF("チェックカメラを作りました\n");
            w->eye[0] = ((CamChkArg *) arg)->eye[0];
            w->eye[1] = ((CamChkArg *) arg)->eye[1];
            w->eye[2] = ((CamChkArg *) arg)->eye[2];
            w->at[0] = ((CamChkArg *) arg)->at[0];
            w->at[1] = ((CamChkArg *) arg)->at[1];
            w->at[2] = ((CamChkArg *) arg)->at[2];
            w->cam = (Camera *) tskCreate(obj, dracam, 0, NULL);
            camSetFov(w->cam, ((CamChkArg *) arg)->fov * 0.0174532930038800785);
            w->view = (*camGetView)(w->cam);
            w->view->zFar = 20000.0f;
            w->view->zNear = 10.0f;
            camSetEye(w->cam, w->eye[0], w->eye[1], w->eye[2]);
            camSetAt(w->cam, w->at[0], w->at[1], w->at[2]);
            w->marker = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->marker == NULL) {
                return -1;
            }
            w->marker->flags |= 7;
            demoPolSetDL(w->marker, camchkMarkerDl, 0xED);
            gCamChkActive = 1;
            w->timer = 0;
            w->pos = asciiNew(gFrmTextRoot, 20, 20, "                                ");
            w->ipos = asciiNew(gFrmTextRoot, 20, 30, "                                ");
            break;
        case 3:
            break;
        case 1:
            frmDestroy(&w->marker->f);
            frmDestroy(&w->pos->f);
            frmDestroy(&w->ipos->f);
            frmDestroy(&w->view->f);
            tskDestroy((Obj *) w->cam);
            gCamChkActive = 0;
            break;
        case 0:
            w->timer++;
            w->size = (sinf(w->timer * 0.0174532930038800785 * 10.0) * 0.5 + 1.0) *
                                gCamChkMarkerSize;
            w->marker->scale[0] = w->marker->scale[1] = w->marker->scale[2] =
                w->size;
            w->marker->pos[0] = w->at[0];
            w->marker->pos[1] = w->at[1];
            w->marker->pos[2] = w->at[2];
            if (joyGetButton(0, 0) & U_JPAD) {
                w->at[1] += gCamChkStep;
            }
            if (joyGetButton(0, 0) & D_JPAD) {
                w->at[1] -= gCamChkStep;
            }
            if (joyGetButton(0, 0) & R_JPAD) {
                w->at[0] += gCamChkStep;
            }
            if (joyGetButton(0, 0) & L_JPAD) {
                w->at[0] -= gCamChkStep;
            }
            if (joyGetButton(0, 0) & R_TRIG) {
                w->at[2] += gCamChkStep;
            }
            if (joyGetButton(0, 0) & L_TRIG) {
                w->at[2] -= gCamChkStep;
            }
            if (joyGetButton(0, 0) & U_CBUTTONS) {
                w->eye[1] += gCamChkStep;
            }
            if (joyGetButton(0, 0) & D_CBUTTONS) {
                w->eye[1] -= gCamChkStep;
            }
            if (joyGetButton(0, 0) & R_CBUTTONS) {
                w->eye[0] += gCamChkStep;
            }
            if (joyGetButton(0, 0) & L_CBUTTONS) {
                w->eye[0] -= gCamChkStep;
            }
            if (joyGetButton(0, 0) & A_BUTTON) {
                w->eye[2] += gCamChkStep;
            }
            if (joyGetButton(0, 0) & B_BUTTON) {
                w->eye[2] -= gCamChkStep;
            }
            if (joyGetButton(0, 0) & Z_TRIG) {
                if (joyGetButton(0, 0) & U_JPAD) {
                    w->at[1] += gCamChkStep * 4.0f;
                }
                if (joyGetButton(0, 0) & D_JPAD) {
                    w->at[1] -= gCamChkStep * 4.0f;
                }
                if (joyGetButton(0, 0) & R_JPAD) {
                    w->at[0] += gCamChkStep * 4.0f;
                }
                if (joyGetButton(0, 0) & L_JPAD) {
                    w->at[0] -= gCamChkStep * 4.0f;
                }
                if (joyGetButton(0, 0) & R_TRIG) {
                    w->at[2] += gCamChkStep * 4.0f;
                }
                if (joyGetButton(0, 0) & L_TRIG) {
                    w->at[2] -= gCamChkStep * 4.0f;
                }
                if (joyGetButton(0, 0) & U_CBUTTONS) {
                    w->eye[1] += gCamChkStep * 4.0f;
                }
                if (joyGetButton(0, 0) & D_CBUTTONS) {
                    w->eye[1] -= gCamChkStep * 4.0f;
                }
                if (joyGetButton(0, 0) & R_CBUTTONS) {
                    w->eye[0] += gCamChkStep * 4.0f;
                }
                if (joyGetButton(0, 0) & L_CBUTTONS) {
                    w->eye[0] -= gCamChkStep * 4.0f;
                }
                if (joyGetButton(0, 0) & A_BUTTON) {
                    w->eye[2] += gCamChkStep * 4.0f;
                }
                if (joyGetButton(0, 0) & B_BUTTON) {
                    w->eye[2] -= gCamChkStep * 4.0f;
                }
            }
            if (joyGetHeldTime(0, START_BUTTON) == 1 && w->timer >= 11) {
                return -1;
            }
            camSetEye(w->cam, w->eye[0], w->eye[1], w->eye[2]);
            camSetAt(w->cam, w->at[0], w->at[1], w->at[2]);
            sprintf(w->pos->str, "POS    $ %5d %5d %5d ", (s32) w->eye[0],
                               (s32) w->eye[1], (s32) w->eye[2]);
            sprintf(w->ipos->str, "IPOS   $ %5d %5d %5d ", (s32) w->at[0],
                               (s32) w->at[1], (s32) w->at[2]);
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* Opens "CAMchk" under the demo menu: a check camera at the saved eye and look-at, with a
 * field of view of fov degrees. */
void demoCamChkOpen(f32 fov) {
    s32 pad[2];
    CamChkArg arg;

    arg.eye[0] = gCamChkEyeX;
    arg.eye[1] = gCamChkEyeY;
    arg.eye[2] = gCamChkEyeZ;
    arg.at[0] = gCamChkAtX;
    arg.at[1] = gCamChkAtY;
    arg.at[2] = gCamChkAtZ;
    arg.fov = fov;
    tskCreate(gDmenuLastObj, CAMchk, gDmenuLastObj->file, &arg);
}

/* "JOYchk": each frame, which of controller 2's buttons are held (for OBJMOVE). */
s32 JOYchk(Obj *obj, s16 msg, void *arg) {
    switch (msg) {
        case 1:
        case 3:
            break;
        case 2:
            obj->name = "JOYchk";
            break;
        case 0:
            gJoyChkUp = 0;
            gJoyChkDown = 0;
            gJoyChkRight = 0;
            gJoyChkLeft = 0;
            gJoyChkR = 0;
            gJoyChkL = 0;
            gJoyChkA = 0;
            gJoyChkB = 0;
            gJoyChkZ = 0;
            if (joyGetButton(1, 0) & U_JPAD) {
                gJoyChkUp = 1;
            }
            if (joyGetButton(1, 0) & D_JPAD) {
                gJoyChkDown = 1;
            }
            if (joyGetButton(1, 0) & R_JPAD) {
                gJoyChkRight = 1;
            }
            if (joyGetButton(1, 0) & L_JPAD) {
                gJoyChkLeft = 1;
            }
            if (joyGetButton(1, 0) & R_TRIG) {
                gJoyChkR = 1;
            }
            if (joyGetButton(1, 0) & L_TRIG) {
                gJoyChkL = 1;
            }
            if (joyGetButton(1, 0) & Z_TRIG) {
                gJoyChkZ = 1;
            }
            if (joyGetButton(1, 0) & A_BUTTON) {
                gJoyChkA = 1;
            }
            if (joyGetButton(1, 0) & B_BUTTON) {
                gJoyChkB = 1;
            }
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* Opens "JOYchk" under the demo menu. */
void demoJoyChkOpen(void) {
    tskCreate(gDmenuLastObj, JOYchk, gDmenuLastObj->file, NULL);
}

/* "OBJMOVE": Start on controller 2 picks the next model (then none); the line shows the chosen
 * model's position and angle. The held buttons (JOYchk) give this frame's moves: the D-pad x
 * and y, R and L z, A and B the angle by a degree; with Z held, nine times as much. */
s32 OBJMOVE(Obj *obj, s16 msg, void *arg) {
    switch (msg) {
        case 3:
            break;
        case 2:
            obj->name = "OBJMOVE";
            gObjMovePosX = 0.0f;
            gObjMovePosY = 0.0f;
            gObjMovePosZ = 0.0f;
            *(AsciiFrame **) ((u32) obj + sizeof(Obj)) = asciiNew(
                gFrmTextRoot, 20, 40, "                                                             ");
            gObjMoveSel = -1;
            break;
        case 1:
            frmDestroy(&(*(AsciiFrame **) ((u32) obj + sizeof(Obj)))->f);
            break;
        case 0:
            if ((*joyGetHeldTime)(1, START_BUTTON) == 1) {
                gObjMoveSel++;
            }
            if (gObjMoveSel >= gObjMoveCount) {
                gObjMoveSel = -1;
            }
            if (gObjMoveSel >= 0) {
                sprintf((*(AsciiFrame **) ((u32) obj + sizeof(Obj)))->str,
                                   "MODEL%2d$ %5d %5d %5d R$ %3d", gObjMoveSel, (s32) gObjMovePosX, (s32) gObjMovePosY,
                                   (s32) gObjMovePosZ, (s32) (gObjMoveRot / 0.0174532930038800785));
            } else {
                sprintf((*(AsciiFrame **) ((u32) obj + sizeof(Obj)))->str,
                                   "                                                    ");
            }
            gObjMoveDx = gObjMoveDy = gObjMoveDz = gObjMoveDrot = 0.0f;
            if (gJoyChkUp) {
                gObjMoveDy = gObjMoveStep;
            }
            if (gJoyChkDown) {
                gObjMoveDy = -gObjMoveStep;
            }
            if (gJoyChkRight) {
                gObjMoveDx = gObjMoveStep;
            }
            if (gJoyChkLeft) {
                gObjMoveDx = -gObjMoveStep;
            }
            if (gJoyChkR) {
                gObjMoveDz = gObjMoveStep;
            }
            if (gJoyChkL) {
                gObjMoveDz = -gObjMoveStep;
            }
            if (gJoyChkA) {
                gObjMoveDrot = 0.017453292f;
            }
            if (gJoyChkB) {
                gObjMoveDrot = -0.017453292f;
            }
            if (gJoyChkZ) {
                if (gJoyChkUp) {
                    gObjMoveDy = gObjMoveStep * 9.0f;
                }
                if (gJoyChkDown) {
                    gObjMoveDy = -gObjMoveStep * 9.0f;
                }
                if (gJoyChkRight) {
                    gObjMoveDx = gObjMoveStep * 9.0f;
                }
                if (gJoyChkLeft) {
                    gObjMoveDx = -gObjMoveStep * 9.0f;
                }
                if (gJoyChkR) {
                    gObjMoveDz = gObjMoveStep * 9.0f;
                }
                if (gJoyChkL) {
                    gObjMoveDz = -gObjMoveStep * 9.0f;
                }
                if (gJoyChkA) {
                    gObjMoveDrot = 0.15707964f;
                }
                if (gJoyChkB) {
                    gObjMoveDrot = -0.15707964f;
                }
            }
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* Opens "OBJMOVE" under the demo menu. */
void demoObjMoveOpen(void) {
    tskCreate(gDmenuLastObj, OBJMOVE, gDmenuLastObj->file, NULL);
}

/* Plays an "anime" frame's animation, then a second one ("tunagi", the connecting motion).
 * step 0 plays the current animation
 * (mode1); at its end the frame takes anim1 (set1), from the start (mode2 2) or the end (3),
 * and step becomes 1. Step 1 plays that (mode2); at its end the frame takes anim2 (set2) and
 * step becomes -1. Returns the step. */
s32 tunagi_anime(AnimeFrame *f, s32 step, s32 mode1, s32 mode2, f32 speed1, f32 speed2, s32 set1,
                           u32 anim1, s32 set2, u32 anim2) {
    switch (step) {
        case 0:
            if (demoAnimeStep(f, speed1, mode1) != 0) {
                step = 1;
                PRINTF("<tunagi_anime>繋ぎモーションに切り替えます\n");
                demoAnimeSet(f, anim1, set1, 0.0f);
                switch (mode2) {
                    case 2:
                        f->frame = 0.0f;
                        break;
                    case 3:
                        f->frame = polAnimeLastFrame(f);
                        break;
                }
            }
            break;
        case 1:
            if (demoAnimeStep(f, speed2, mode2) != 0) {
                demoAnimeSet(f, anim2, set2, 0.0f);
                step = -1;
                PRINTF("<tunagi_anime>繋ぎモーション終わります\n");
            }
            break;
    }
    return step;
}

/* Texture animation `which` of kind 0-3 into *out. */
void demoTexAnim(s32 kind, Anim *a, s32 which, u32 *out) {
    switch (kind) {
        case 0:
            demoTexAnimK0(a, which, out);
            break;
        case 1:
            demoTexAnimK1(a, which, out);
            break;
        case 2:
            demoTexAnimK2(a, which, out);
            break;
        case 3:
            demoTexAnimK3(a, which, out);
            break;
    }
}

/* The same for a demo actor's player: its model plays on; at the end of each step the player
 * takes action1 (then action2), and its face's two texture animations become `which`
 * (segments 6 and 7 of its model). */
s32 demoPlTunagiAnime(Obj *obj, s32 which, s32 step, s32 mode1, s32 mode2, f32 speed1, f32 speed2,
                           s32 action1, s32 action2) {
    DemoActor *w = DEMOACT(obj);
    Player *p = PL(w->player);

    switch (step) {
        case 0:
            if (demoAnimeStep(p->model, speed1, mode1) != 0) {
                PRINTF("<tunagi_anime>繋ぎモーションに切り替えます\n");
                step = 1;
                (*plAnimeSet)(p, action1);
                demoTexAnim(w->kind, &w->anim[0], which, &PL(w->player)->model->segs[3]);
                demoTexAnim(w->kind, &w->anim[1], which, &PL(w->player)->model->segs[4]);
                switch (mode2) {
                    case 2:
                        w->frame->frame = 0.0f;
                        break;
                    case 3:
                        w->frame->frame = polAnimeLastFrame(w->frame);
                        break;
                }
            }
            break;
        case 1:
            if (demoAnimeStep(p->model, speed2, mode2) != 0) {
                (*plAnimeSet)(p, action2);
                demoTexAnim(w->kind, &w->anim[0], which, &PL(w->player)->model->segs[3]);
                demoTexAnim(w->kind, &w->anim[1], which, &PL(w->player)->model->segs[4]);
                step = -1;
                PRINTF("<tunagi_anime>繋ぎモーション終わります\n");
            }
            break;
    }
    return step;
}

/* "democam": the demos' camera, a copy of "dracam" (camera.c) that stands still while a forced
 * fade is on. */
s32 democam(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    CamWork *w = (CamWork *) ((u32) obj + sizeof(Obj));

    switch (msg) {
        case 2:
            obj->name = "democam";
            w->view = (CamView *) frmCreate(gFrmLayers, gFrmKindCamera);
            if (w->view == NULL) {
                PRINTF("Caution: カメラ用のフレームが不足しています in dracam\n");
                return -1;
            }
            camSetSpeed((Camera *) obj, 20.0f);
            camStop((Camera *) obj);
            gGameWork.view = w->view;
            break;
        case 1:
            if (w->view != NULL) {
                frmDestroy(&w->view->f);
                gGameWork.view = NULL;
            }
            break;
        case 0:
            if (gDemoCamHold == 1) {
                PRINTF("強制フェード中です。\n");
                return tskDefaultProc(obj, msg, arg);
            }
            CAM_MOVE(w->view->fovy, w->fovTarget, w->fovStep);
            CAM_MOVE(w->view->zFar, w->farTarget, w->farStep);
            CAM_MOVE(w->view->eye[0], w->eyeXTarget, w->eyeXStep);
            CAM_MOVE(w->view->eye[1], w->eyeYTarget, w->eyeYStep);
            CAM_MOVE(w->view->eye[2], w->eyeZTarget, w->eyeZStep);
            CAM_MOVE(w->view->at[0], w->atXTarget, w->atXStep);
            CAM_MOVE(w->view->at[1], w->atYTarget, w->atYStep);
            CAM_MOVE(w->view->at[2], w->atZTarget, w->atZStep);
            if (fabsf(w->view->at[0] - w->view->eye[0]) < 1.0f && fabsf(w->view->at[2] - w->view->eye[2]) < 1.0f) {
                w->view->at[2] += 1.0f;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Walks an "anime" frame toward (tx, tz): turns it by up to a degree a step, plays its
 * animation, and moves it speed along its heading. 1 once it is within reach (then it stands
 * on the spot). */
s32 demoWalkTo(AnimeFrame *f, f32 animSpeed, f32 speed, f32 tx, f32 tz) {
    f32 s;
    f32 c;

    f->rot[1] = demoTurnToward(f->pos[0], f->pos[2], tx, tz, f->rot[1], 1.0f);
    demoAnimeStep(f, animSpeed, 0);
    s = sinf(f->rot[1]) * speed;
    c = cosf(f->rot[1]);
    f->pos[0] += s;
    f->pos[2] += c * speed;
    if ((f->pos[2] - tz) * (f->pos[2] - tz) + (f->pos[0] - tx) * (f->pos[0] - tx) < speed * 3.0f) {
        f->pos[0] = tx;
        f->pos[2] = tz;
        return TRUE;
    }
    return FALSE;
}

/* Starts a fade toward a colour, unless a forced fade is on. */
void demoFadeStart(f32 speed, f32 level, f32 target, u8 r, u8 g, u8 b) {
    if (gDemoForcedFade == 0) {
        fadeSetSpeed(speed);
        fadeSetLevel(level);
        fadeSetColor(r, g, b);
        fadeSetTarget(target);
    }
}

/* A forced fade toward a colour: from `level` only if the colour changes. */
void demoFadeForce(f32 speed, f32 level, f32 target, u8 r, u8 g, u8 b) {
    u8 cr;
    u8 cg;
    u8 cb;

    gDemoForcedFade |= 1;
    fadeGetColor(&cr, &cg, &cb);
    fadeSetSpeed(speed);
    fadeSetColor(r, g, b);
    if (r != cr) {
        fadeSetLevel(level);
    }
    fadeSetTarget(target);
}

/* Rumbles every player in play's controller with a pattern for a time. */
void demoRumbleAll(u32 pattern, s32 time) {
    s32 i;

    for (i = 0; i < 4; i++) {
        if (gSaveData.players[i].chara != -1 && gSaveData.players[i].lives > 0) {
            (*contPakRumbleStart)(i, pattern, time);
        }
    }
}
