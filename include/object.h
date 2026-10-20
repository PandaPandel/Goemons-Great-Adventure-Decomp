#ifndef OBJECT_H
#define OBJECT_H

/* Overlay 6's objects and frames, shared by its files. */

#include "common.h"

/* Overlay 6 has sqrtf inline (sqrt.s); main calls the library's. PR/gu.h makes it an intrinsic
 * only from libultra 2.0K. */
#pragma intrinsic (sqrtf)

/* An overlay-6 object: a task of the task manager (tmanager.c), in one or more 0x80-byte blocks
 * of its pool; its work area follows the header (obj + 1). Objects are named ("dracam", "AI
 * Cam", "bgrad", "Artsun", "Fade"). A handler takes (obj, message, arg): 2 creates, 1 frees, 0
 * runs, 3 draws; the rest go to tskDefaultProc, which passes 0, 3 and 10 on to the children. A
 * handler that returns -1 has its task deleted. */
typedef struct Obj {
    /* 0x00 */ u16 flags; /* 1 in use, 2 asleep (sent no messages) */
    /* 0x02 */ u8 blocks; /* the pool blocks it spans */
    /* 0x04 */ char *name;
    /* 0x08 */ struct Obj *parent;
    /* 0x0C */ struct Obj *child; /* the first */
    /* 0x10 */ struct Obj *prev; /* siblings */
    /* 0x14 */ struct Obj *next;
    /* 0x18 */ u32 file; /* the file the handler is in, TLB-mapped for each call */
    /* 0x1C */ s32 (*proc)(struct Obj *, s16, void *);
} Obj; /* size 0x20 */

typedef struct TaskBlock {
    /* 0x00 */ Obj obj;
    /* 0x20 */ u8 work[0x60];
} TaskBlock; /* size 0x80 */

/* A frame of the graphic manager (gfxmgr.c): a node in the tree it walks to build each frame's
 * display list, taken from its kind's pool (frmCreate and the others). Kinds add their own
 * fields after this header; a "core" frame is the header alone. */
typedef struct Frame {
    /* 0x00 */ u16 flags; /* 1 in use, 2 its kind's draw handler runs */
    /* 0x02 */ s16 kind;
    /* 0x04 */ struct Frame *parent; /* the previous one on a free list */
    /* 0x08 */ struct Frame *child; /* the first; the next one on a free list */
    /* 0x0C */ struct Frame *prev; /* siblings */
    /* 0x10 */ struct Frame *next;
} Frame; /* size 0x14 */

typedef struct DLFrame {
    /* 0x00 */ Frame f;
    /* 0x14 */ u16 dlFlags; /* 8: the list is copied in, not called */
    /* 0x18 */ Gfx *dl;
} DLFrame; /* size 0x1C: the "dlist " kind */

/* A "pol" frame (polygon.c): display lists with a position, a rotation and a scale. */
typedef struct PolFrame {
    /* 0x00 */ Frame f;
    /* 0x14 */ f32 pos[3];
    /* 0x20 */ f32 rot[3];
    /* 0x2C */ f32 scale[3];
    /* 0x38 */ u16 flags;
    /* 0x3C */ Gfx *dl;
    /* 0x40 */ Gfx *dl2;
    /* 0x44 */ u32 segs[6]; /* segments 3-8 */
} PolFrame; /* size 0x5C: "pol" */

/* An "anime" frame (polygon.c): the same with an animated model at a frame (the player's model,
 * and the effects that follow it). The animation is a segment address: polygon.c reads it as
 * one (an integer test, then animSeek and polDrawAnimModel), every other file as a pointer
 * (set to and compared with an animation's symbol). The two names keep both readings. */
typedef struct AnimeFrame {
    /* 0x00 */ Frame f;
    /* 0x14 */ f32 pos[3];
    /* 0x20 */ f32 rot[3];
    /* 0x2C */ f32 scale[3];
    /* 0x38 */ u16 flags;
    /* 0x3C */ f32 frame; /* the animation's */
    /* 0x40 */ Gfx *dl;
    /* 0x44 */ union {
        void *anim;
        u32 animAddr;
    };
    /* 0x48 */ u32 segs[6];
} AnimeFrame; /* size 0x60: "anime" */

/* A BG map cell (BGFrame.img1; the BG tile pictures' maps): the tile in the low 14 bits, and the
 * tile mirrored in S (bit 15) or T (bit 14), as bgDraw reads it. */
#define BG_CELL_TILE 0x3FFF
#define BG_CELL_MIRROR_S 0x8000
#define BG_CELL_MIRROR_T 0x4000

/* A palette entry (RGBA5551), kept in the N64's byte order, as the RDP's TLUT load reads it. The
 * N64 build reads and writes it as the game does; under AVOID_UB it is taken a byte at a time,
 * high byte first, so a little-endian build sees the same colour. */
#ifndef AVOID_UB
#define PAL_GET(pal, i) ((pal)[i])
#define PAL_SET(pal, i, v) ((pal)[i] = (v))
#else
#define PAL_GET(pal, i) ((u16) ((((u8 *) (pal))[2 * (i)] << 8) | ((u8 *) (pal))[2 * (i) + 1]))
#define PAL_SET(pal, i, v) (((u8 *) (pal))[2 * (i)] = (u16) (v) >> 8, ((u8 *) (pal))[2 * (i) + 1] = (u8) (v))
#endif

/* A background frame (the "Bg" kind, gFrmKindBg): a picture drawn behind everything, scrolled
 * (camBG in cambg.c; "Bbg" in impact.c). */
typedef struct BGFrame {
    /* 0x00 */ Frame f;
    /* 0x14 */ u16 offX; /* where on the screen it starts (0: by the scroll) */
    /* 0x16 */ u16 offY;
    /* 0x18 */ f32 x; /* the scroll */
    /* 0x1C */ f32 y;
    /* 0x20 */ u16 w; /* in tiles */
    /* 0x22 */ u16 h;
    /* 0x24 */ u32 img1; /* the map: a u16 cell per tile (BG_CELL_*) */
    /* 0x28 */ u32 img2; /* the tiles' textures (CI8, 16x16 each) */
    /* 0x2C */ u16 *palA; /* the blend's two palettes (RGBA5551, 256 colours) */
    /* 0x30 */ u16 *palB;
    /* 0x34 */ u16 tileCount; /* the tiles' count */
    /* 0x36 */ u8 blend; /* palB's share, 0-32 */
    /* 0x37 */ u8 unused37;
    /* 0x38 */ u16 alpha; /* the alpha; 256 and up: opaque */
    /* 0x3A */ u8 wrapX; /* wraps across */
    /* 0x3B */ u8 wrapY; /* wraps down */
    /* 0x3C */ f32 vx; /* the scroll's speed */
    /* 0x40 */ f32 vy;
} BGFrame;

typedef struct LightFrame {
    /* 0x00 */ Frame f;
    /* 0x14 */ u8 lightNum;
    /* 0x15 */ s8 dir[3]; /* a light's direction */
    /* 0x18 */ u8 color[3]; /* and colour */
} LightFrame;

/* An "ambi" frame: the ambient light's colour (camlight.c). */
typedef struct AmbFrame {
    /* 0x00 */ Frame f;
    /* 0x14 */ u8 color[3];
} AmbFrame; /* size 0x18 */

typedef struct Color3 {
    u8 r, g, b;
} Color3;

/* A camera frame's scissor (all 0: left alone). The game reads it as four named fields: as an
 * s16[4], IDO sorts the operands of camlightCameraDraw's two OR chains (IDO_NOTES 198). */
typedef struct CamScissor {
    s16 ulx, uly, lrx, lry;
} CamScissor;

/* A "camera" frame (camlight.c, drawn by camlightCameraDraw): the game's is gGameWork's. */
typedef struct CamView {
    /* 0x00 */ Frame f;
    /* 0x14 */ f32 eye[3];
    /* 0x20 */ f32 at[3];
    /* 0x2C */ f32 roll; /* turns the up vector (fadeViewMatrix) */
    /* 0x30 */ f32 fovy; /* the field of view, in radians (40 degrees at first) */
    /* 0x34 */ f32 zNear; /* the near plane (20 at first) */
    /* 0x38 */ f32 zFar; /* the far plane (800 at first) */
    /* 0x3C */ f32 hilite1[3]; /* guLookAtHilite's two light directions */
    /* 0x48 */ f32 hilite2[3];
    /* 0x54 */ s8 hiliteW; /* and its texture size */
    /* 0x55 */ s8 hiliteH;
    /* 0x56 */ u8 ortho; /* bit 0: guOrtho with left..bottom instead of guPerspective */
    /* 0x58 */ Vp *vp; /* its viewport (NULL: the default, gCamDefaultVp) */
    /* 0x5C */ s32 clip; /* the clip ratio's negative side (0: left alone) */
    /* 0x60 */ CamScissor scissor;
    /* 0x68 */ f32 left;
    /* 0x6C */ f32 top;
    /* 0x70 */ f32 right;
    /* 0x74 */ f32 bottom;
} CamView; /* size 0x78 */

/* Where the players start in a stage (stage.c); 1 in `on` overrides the stage's own. */
typedef struct Spawn {
    /* 0x00 */ u8 on;
    /* 0x02 */ s16 angle;
    /* 0x04 */ f32 x;
    /* 0x08 */ f32 y;
    /* 0x0C */ f32 z;
    /* 0x10 */ f32 facing;
    /* 0x14 */ s32 action; /* 0 or more: the players' state */
} Spawn; /* size 0x18 */

/* The game's block at 0x80088338, as far as known. +0x19 is gContPakRumbleSuspended (set while the game
 * resets) and +0x4C is the time of day (gTimeOfDay). */
typedef struct G88338 {
    /* 0x00 */ CamView *view; /* the camera's */
    /* 0x04 */ Obj *gameProcTask; /* the task the next stage's task goes under */
    /* 0x08 */ s32 frames; /* frames since the stage began */
    /* 0x0C */ s16 stage;
    /* 0x0E */ s8 area;
    /* 0x0F */ s8 areaStage;
    /* 0x10 */ s8 diary; /* the Controller Pak save in use, -1 for none (sccheck.c) */
    /* 0x11 */ s8 partake;
    /* 0x12 */ s8 scenarioRunning; /* 2: pausing is off */
    /* 0x13 */ s8 mission;
    /* 0x14 */ s8 geoCacheSize; /* the next Geography's cache size, then cleared (0: 30) */
    /* 0x15 */ s8 demoPlaying; /* set while a demo plays (dkeeper, the staff roll): no rumble (contpak.c) */
    /* 0x16 */ u8 fadeColor[3]; /* a fade colour (gameproc.c's scene change, titleStepIntro) */
    /* 0x19 */ s8 rumbleSuspended; /* gContPakRumbleSuspended: set while the game resets, no rumble */
    /* 0x1A */ u8 pad1A[2];
    /* 0x1C */ Spawn spawn;
    /* 0x34 */ Spawn savedSpawn;
    /* 0x4C */ s32 time; /* the time of day (also named gTimeOfDay) */
    /* 0x50 */ s32 rate; /* the clock's step, as the sky's preview counts it */
    /* 0x54 */ s32 daySeconds; /* a day's length in seconds (Artsun) */
    /* 0x58 */ u16 tunes[5]; /* copied from the light (camSunCopyTunes) */
    /* 0x62 */ u8 pad62[0x2];
    /* 0x64 */ Obj *plManagerTask; /* the players' parent task ("pl親", plmain.c) */
    /* 0x68 */ Obj *players[4]; /* the four players' tasks (gauge.c) */
    /* 0x78 */ u8 charas[4];
    /* 0x7C */ u8 tokageSet; /* 1 while the lizard man ("tokage") is out (tlb_0136) */
    /* 0x7D */ u8 pad7D[3];
    /* 0x80 */ u16 flags;
    /* 0x82 */ u16 nextOverlay; /* a file game_proc loads before the next task (fileLoadOverlay) */
    /* 0x84 */ u8 pad84[4];
    /* 0x88 */ Obj *nextParent; /* where the next task goes */
    /* 0x8C */ void *next; /* the next task's handler (or a TLB address in a file) */
    /* 0x90 */ s32 nextBlocks; /* passed to the next task's creation (tskCreateBlocks's n) */
    /* 0x94 */ u16 nextFile; /* the file holding it */
    /* 0x96 */ u8 pad96[2];
    /* 0x98 */ s32 nextArg;
    /* 0x9C */ u8 entryFrom; /* where the next stage was entered from: on the map, the player who leaves by
                              * the Impact, whose line comes first after a boss stage (tlb_0016_0); in castle
                              * 4, the room came from (0: 4-4, 1: 4-5), picking 4-6's places (tlb_0244) */
    /* 0x9D */ u8 tunePart; /* a multi-room stage's part whose tune has started (lbu/sb only) */
    /* 0x9E */ u8 sndStopAllFirst; /* lbu in plmain.c plMain_init */
    /* 0x9F */ u8 restartSdCount;
    /* 0xA0 */ u8 kpaoyaBeaten; /* set when "kpaoya" is beaten (tlb_0062) */
    /* 0xA1 */ u8 titleDemo; /* which demo the title plays next, mod 3 (tlb_0018_4) */
    /* 0xA2 */ s8 losses; /* boss battles lost in a row (bmove.c); the third ends the game */
    /* 0xA3 */ u8 kinokoCount; /* the mushroom hunt's mushrooms picked (tlb_0128; sccheck.c) */
    /* 0xA4 */ u8 kinokoTaken[4]; /* a bit per mushroom id 1-30 */
    /* 0xA8 */ u8 kinoko; /* bits 1-7: the mushrooms found (sccheck.c) */
    /* 0xA9 */ u8 jizouCorrect; /* hats on the true jizou; 7 or more is enough (tlb_0130; sccheck.c) */
    /* 0xAA */ u8 kasaCount; /* hats put on any jizou */
    /* 0xAB */ u8 jizouKasa[2]; /* the jizou with their hats on, a bit each by the jizou's id */
    /* 0xAD */ u8 padAD;
    /* 0xAE */ u8 hanabiShows; /* the fireworks apprentice's shows since the last right answer */
    /* 0xAF */ u8 hanabiCue[9]; /* the fireworks the player lit, in order */
    /* 0xB8 */ u8 hanabiCueNo; /* and how many ("キューNo") */
    /* 0xB9 */ u8 hanabiQNo; /* the question (sequence) number: the game's own "gw.hanabiQNo" */
    /* 0xBA */ u8 okeBroken; /* a bit per tub broken (tlb_0134) */
    /* 0xBB */ u8 padBB;
    /* 0xBC */ u8 goalHour[4]; /* the hide-and-seek's goal, an hour and a minute of the clock: words the */
    /* 0xC0 */ u8 goalMin[4];  /* game writes with sw, read through tlb_0134's views */
    /* 0xC4 */ u8 kanbanBroken; /* a bit per signboard broken (tlb_0134) */
    /* 0xC5 */ u8 hiddenId[6]; /* the six children's hiding places: red, black, blue, green, pink, yellow */
    /* 0xCB */ u8 karasuShot; /* a bit per karasu tengu shot down (tlb_0135); bits 1-6 counted (sccheck.c) */
    /* 0xCC */ s32 savedDaySeconds; /* daySeconds kept while a timed mission runs (lw/sw) */
    /* 0xD0 */ u16 backStage; /* where stage 0x8D (and the inn) sends the players back to */
    /* 0xD2 */ s16 backAngle;
    /* 0xD4 */ f32 backPos[3];
    /* 0xE0 */ s32 kills; /* enemies beaten (eneproc.c); odd, their second death sounds */
    /* 0xE4 */ u8 padE4[4];
    /* 0xE8 */ u8 save[0x130]; /* gSaveData's areas kept while a demo plays (demo.c) */
} G88338;
extern G88338 gGameWork;

/* A model's node (model.c): a display list (0: none), the relative indices of the first child
 * and the next sibling, and its transform. The scale, rotation and position are animation
 * tracks: the start of each one's keys in the animation's data (anim.c), or a constant. */
typedef struct ModelNode {
    /* 0x00 */ u32 dl;
    /* 0x04 */ s8 child;
    /* 0x05 */ s8 sibling;
    /* 0x06 */ u16 scale[3]; /* 1/256 units */
    /* 0x0C */ u16 rot[3]; /* 1024 a turn */
    /* 0x12 */ s16 pos[3];
} ModelNode; /* size 0x18 */

/* A path through points, walked by mathSplinePoint from 0 to 1. A cardinal spline has one more
 * control point at each end. */
typedef struct Spline {
    /* 0x0 */ u8 kind; /* 0 straight lines, 1 a cardinal spline */
    /* 0x2 */ s16 n;   /* the points walked through */
    /* 0x4 */ f32 tension;
    /* 0x8 */ Vec3f *points;
} Spline;

#endif
