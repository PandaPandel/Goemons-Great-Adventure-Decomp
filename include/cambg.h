#ifndef CAMBG_H
#define CAMBG_H

#include "object.h"

/* The task's work. */
typedef struct CamBGWork {
    /* 0x00 */ BGFrame *frame;
    /* 0x04 */ u16 *pal[4]; /* dawn, day, dusk, night */
    /* 0x14 */ f32 scale; /* 100 / the picture's distance */
    /* 0x18 */ CamView *view; /* the game's camera */
    /* 0x1C */ Vec3f eye; /* its eye at the last step (copied whole: lw/sw) */
    /* 0x28 */ f32 yaw; /* and its heading */
    /* 0x2C */ f32 pitch;
    /* 0x30 */ u8 pad30[2];
    /* 0x32 */ u16 flags; /* 1 scrolls sideways, 2 up and down */
    /* 0x34 */ u8 pad34[0x8];
    /* 0x3C */ void (*proc)(struct CamBGWork *w); /* sets the colours each step */
    /* 0x40 */ void (*target)(struct CamBGWork *w); /* the program being blended into */
    /* 0x44 */ s32 from; /* the blend's start and end times */
    /* 0x48 */ s32 to;
    /* 0x4C */ u16 *buf; /* the blend's two palettes (0x400 bytes on the heap) */
} CamBGWork;

s32 camBG(Obj *obj, s16 msg, void *arg);
void cambgPalByHour(CamBGWork *w);
void cambgPalNight(CamBGWork *w);
void cambgBakeBlend(CamBGWork *w, s32 buf);
void cambgStartBlend(CamBGWork *w, s32 steps);
void cambgBlendStep(CamBGWork *w);
Obj *cambgCreate(Obj *parent, s32 n, f32 dist);
void cambgMove(Obj *obj, s32 dx, s32 dy);
void cambgScrollX(Obj *obj, s32 on);
void cambgScrollY(Obj *obj, s32 on);
void cambgShow(Obj *obj);
void cambgHide(Obj *obj);
void cambgSetProgram(Obj *obj, s32 mode);
void cambgSetAlpha(Obj *obj, u16 v);
void cambgLoad(Obj *obj, s32 n);

#endif
