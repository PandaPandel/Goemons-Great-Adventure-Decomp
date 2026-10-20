#ifndef CAMERA_H
#define CAMERA_H

/* The camera (src/camera.c): an object of overlay 6 whose view is gGameWork. Its setters move
 * a view value toward a target over time: they store the target and a per-tick step, the
 * distance divided by the camera's speed. */

#include "object.h"

struct CamView;
struct Camera;

/* gCamAiObj's work area: the camera manager. */
typedef struct CamMgrWork {
    /* 0x00 */ struct Camera *cam;
    /* 0x04 */ struct CamView *view;
    /* 0x08 */ u8 pad8[0x4];
    /* 0x0C */ f32 dist; /* the eye's distance from the look-at point (160 at first) */
    /* 0x10 */ u8 heading; /* the free camera's (mode 1), 256 a full turn */
    /* 0x11 */ u8 pitch; /* the eye's, 256 a full turn (0x3A at first) */
    /* 0x12 */ u8 flags; /* flags: 1 while its hook runs, 2 the camera stopped, 4 held */
    /* 0x13 */ u8 nReq;
    /* 0x14 */ struct {
        u8 kind; /* the follower's controller (camAiPan) */
        u8 pad[3];
        s32 arg;
    } req[4];
    /* 0x34 */ void (*hook)(); /* the follow mode's function (camAiSetMode) */
    /* 0x38 */ Vec3f pos; /* the last follow point (camAiPan); the fixed eye (mode 4) */
    /* 0x44 */ f32 panX; /* the pan: sideways, then up (camAiPan); with a fixed eye, a */
    /* 0x48 */ f32 panY; /* point (x, z) the view may turn from by at most the angle */
    /* 0x4C */ f32 angle; /* the eye's heading on rails (the rail's, vramCourseHeading); the angle
                           * limit with a fixed eye (camAiFixedEye) */
    /* 0x50 */ u8 mode; /* the follow mode, 0-5 */
    /* 0x51 */ u8 zone; /* from the ground under the followed objects (camAiFollowGround) */
    /* 0x52 */ u16 idleX; /* frames since the pan last moved sideways */
    /* 0x54 */ u16 idleY; /* and up or down */
    /* 0x56 */ u8 nEvt;
    /* 0x57 */ u8 unused57;
    /* 0x58 */ struct {
        u8 zone;
        s8 sign; /* 1: the place on the rail at most limit; -1: at least */
        s16 limit;
    } evt[12]; /* limits on the zone's rail */
} CamMgrWork;

/* What a followed object answers to message 5: its zone and its place on the zone's rail. */
typedef struct CamTarget {
    /* 0x00 */ s16 zone;
    /* 0x04 */ f32 t; /* the place along the rail */
    /* 0x08 */ f32 side;
    /* 0x0C */ f32 y;
} CamTarget;

/* gCamSunObj's work area: the day and night light. Its colours blend by a level that runs from
 * a start to an end time of day (camSunBlendState). */
typedef struct CamCtlWork {
    /* 0x00 */ void (*state)(); /* the state it runs (called with the work area twice) */
    /* 0x04 */ void (*next)(); /* the state after this one */
    /* 0x08 */ void (*watch)(); /* run each frame too: the dawn/dusk music (camSunDawnDuskWatch) */
    /* 0x0C */ struct DLFrame *dlFrame; /* holds the light's display list (Artsun) */
    /* 0x10 */ struct LightFrame *sun; /* the sun: its direction and colour */
    /* 0x14 */ struct LightFrame *ambient;
    /* 0x18 */ struct CamView *view; /* the camera's (camSunSetView): the sun shines along it */
    /* 0x1C */ void *camBg; /* told the light's program (cambgSetProgram) */
    /* 0x20 */ s32 level; /* 0-0x10000 through the blend */
    /* 0x24 */ s32 start; /* times of day, as gTimeOfDay */
    /* 0x28 */ s32 end;
    /* 0x2C */ s16 topLine; /* the sky's gradient from this line (20) */
    /* 0x2E */ s16 bottomLine; /* to this (100) */
    /* 0x30 */ u8 flags; /* flags */
    /* 0x31 */ u8 period; /* the music's time of day: with two tunes 1 day, 0 night; with
                            * four, 1 dawn, 2 day, 3 evening, 0 night */
    /* 0x32 */ u8 pad32[0x2];
    /* 0x34 */ Color3 *src; /* four colours */
    /* Four colour pairs, each blended from [0] to [1] by the level: */
    /* 0x38 */ Color3 top[2];    /* the sky's top */
    /* 0x3E */ Color3 bottom[2]; /* the sky's bottom */
    /* 0x44 */ Color3 fog[2]; /* the fog's colour */
    /* 0x4A */ Color3 env[2]; /* the environment colour and the sun's */
    /* 0x50 */ u16 fogNear; /* the fog's near end (the far is 1000) */
    /* 0x52 */ u16 tunes[5]; /* the tunes: dawn, day, evening, -, night (to gGameWork) */
    /* 0x5C */ u16 dawnSound;
    /* 0x5E */ u16 altDawnTune;
    /* 0x60 */ u16 frames; /* counted while it runs */
    /* 0x62 */ u8 nPair;
    /* 0x63 */ u8 unused63;
    /* 0x64 */ struct {
        u16 a; /* while this sound plays, */
        u16 b; /* this one is started every 16 frames */
    } pair[1]; /* the count is not known */
} CamCtlWork;

/* A colour pair's channel c, blended from pair[0] to pair[1] by the light's level, rounded. */
#define BLEND(w, pair, c) \
    ((((w)->pair[1].c - (w)->pair[0].c) * (w)->level + (w)->level / 2) / 0x10000 + (w)->pair[0].c)

extern Obj *gCamAiObj;
extern Obj *gCamSunObj;
extern s32 gTimeOfDay; /* the time of day: hours as 16.16, wrapping at 24 << 16 */

/* The camera task ("dracam"): its Obj header, then its work (CamWork below). */
typedef struct Camera {
    /* 0x00 */ Obj hdr;
    /* 0x20 */ CamView *view;
    /* 0x24 */ f32 speed; /* a move's length divides by this */
    /* 0x28 */ f32 fovTarget; /* view->fovy's target */
    /* 0x2C */ f32 fovStep; /* and step */
    /* 0x30 */ f32 farTarget; /* view->zFar's target */
    /* 0x34 */ f32 farStep; /* and step */
    /* 0x38 */ f32 eyeXTarget;
    /* 0x3C */ f32 eyeXStep;
    /* 0x40 */ f32 eyeZTarget;
    /* 0x44 */ f32 eyeZStep;
    /* 0x48 */ f32 eyeYTarget;
    /* 0x4C */ f32 eyeYStep; /* eye.y's step */
    /* 0x50 */ f32 atXTarget;
    /* 0x54 */ f32 atXStep;
    /* 0x58 */ f32 atZTarget;
    /* 0x5C */ f32 atZStep;
    /* 0x60 */ f32 atYTarget; /* at.y's target */
    /* 0x64 */ f32 atYStep; /* and step */
    /* 0x68 */ f32 unused68;
    /* 0x6C */ u16 topLine;
    /* 0x6E */ u16 bottomLine;
} Camera;

/* A colour fade: two colours, each moving from its current value toward a target by a step (the
 * distance over the speed). */
typedef struct ColorFade {
    /* 0x00 */ Obj hdr;
    /* 0x20 */ f32 speed;
    /* 0x24 */ f32 target0[3];
    /* 0x30 */ f32 step0[3];
    /* 0x3C */ f32 cur0[3];
    /* 0x48 */ f32 target1[3];
    /* 0x54 */ f32 step1[3];
    /* 0x60 */ f32 cur1[3];
} ColorFade;

/* ColorFade's fields past its header, as its work area: the sky object "bgrad"
 * (bgrad), whose two colours are the frame's gradient fill. */
typedef struct ColorFadeWork {
    /* 0x00 */ f32 speed;
    /* 0x04 */ f32 target0[3]; /* the top colour's */
    /* 0x10 */ f32 step0[3];
    /* 0x1C */ f32 cur0[3];
    /* 0x28 */ f32 target1[3]; /* the bottom colour's */
    /* 0x34 */ f32 step1[3];
    /* 0x40 */ f32 cur1[3];
    /* 0x4C */ u16 topLine;
    /* 0x4E */ u16 lines; /* the fade's last line (240) */
} ColorFadeWork;

/* Camera's fields past its header, as its work area (obj + 1): eyeXTarget (0x18) is Camera's (0x38). */
typedef struct CamWork {
    /* 0x00 */ CamView *view;
    /* 0x04 */ f32 speed;
    /* 0x08 */ f32 fovTarget; /* view->fovy's target */
    /* 0x0C */ f32 fovStep; /* and step */
    /* 0x10 */ f32 farTarget; /* view->zFar's target */
    /* 0x14 */ f32 farStep; /* and step */
    /* 0x18 */ f32 eyeXTarget; /* the eye's target */
    /* 0x1C */ f32 eyeXStep;
    /* 0x20 */ f32 eyeZTarget;
    /* 0x24 */ f32 eyeZStep;
    /* 0x28 */ f32 eyeYTarget;
    /* 0x2C */ f32 eyeYStep;
    /* 0x30 */ f32 atXTarget; /* the look-at point's target */
    /* 0x34 */ f32 atXStep;
    /* 0x38 */ f32 atZTarget;
    /* 0x3C */ f32 atZStep;
    /* 0x40 */ f32 atYTarget;
    /* 0x44 */ f32 atYStep;
} CamWork;

s32 dracam(Obj *obj, s16 msg, void *arg);
s32 camGetMoving(Camera *cam);
void camStop(Camera *cam);
void camSetSpeed(Camera *cam, f32 speed);
void camMoveFov(Camera *cam, f32 target);
void camSetFov(Camera *cam, f32 value);
void camMoveFar(Camera *cam, f32 target);
void camMoveEye(Camera *cam, f32 x, f32 y, f32 z);
void camStopXZ(Camera *cam);
void camMoveEyeIfChanged(Camera *cam, f32 x, f32 y, f32 z);
void camMoveAt(Camera *cam, f32 x, f32 y, f32 z);
void camMoveAtIfChanged(Camera *cam, f32 x, f32 y, f32 z);
void camMoveAtXZ(Camera *cam, f32 x, f32 z);
void camMoveAtY(Camera *cam, f32 target);
void camSetEye(Camera *cam, f32 x, f32 y, f32 z);
void camSetEyeY(Camera *cam, f32 y);
void camGetEye(Camera *cam, f32 *x, f32 *y, f32 *z);
void camSetAt(Camera *cam, f32 x, f32 y, f32 z);
void camSetAtY(Camera *cam, f32 y);
void camGetAt(Camera *cam, f32 *x, f32 *y, f32 *z);
CamView *camGetView(Camera *cam);
void camSnapToTargets(Camera *cam);
f32 camAtRemainSide(Camera *cam, f32 angle);
f32 camAtRemainY(Camera *cam);
s32 AICam(Obj *obj, s16 msg, void *arg);
f32 camMinMaxMid(f32 *min, f32 *max, s32 n, f32 a, f32 b, f32 c, f32 d);
f32 camMidOf(s32 n, f32 x, f32 y, f32 z, f32 w);
f32 camMinOf(s32 n, f32 x, f32 y, f32 z, f32 w);
f32 camMaxOf(s32 n, f32 x, f32 y, f32 z, f32 w);
void camAiPan(CamMgrWork *w, Vec3f *at, f32 dist, u16 flags);
void camAiLookAt(CamMgrWork *w, Vec3f *at);
void camAiFollowRailLimited(CamMgrWork *w);
void camAiFreeLook(CamMgrWork *w);
void camAiFly(CamMgrWork *w);
void camAiFollowGround(CamMgrWork *w);
void camAiFixedEye(CamMgrWork *w);
void camAiFollowRail(CamMgrWork *w);
Obj *camAiCreate(Obj *parent);
void camAiSetMode(s32 mode);
void camAiSetStop(s32 on);
void camAiSetHold(s32 on);
CamView *camAiGetView(void);
void *camAiGetCam(void);
void camAiGetAt(f32 *x, f32 *y, f32 *z);
void camAiAddLookat(s32 arg, s32 kind);
void aicamDeleteLookat(s32 arg);
void camAiSnap(void);
void camAiSetFar(s16 v);
void camAiSetPos(f32 x, f32 y, f32 z);
void camAiGetPan(f32 *out);
void camAiAddLimit(s32 zone, s32 sign, s32 limit);
void camAiSetTurnLimit(f32 x, f32 y, f32 z);
s32 bgrad(Obj *obj, s16 msg, void *arg);
void camBgradSetSpeed(ColorFade *f, f32 speed);
void camBgradMoveTop(ColorFade *f, u8 r, u8 g, u8 b);
void camBgradMoveBottom(ColorFade *f, u8 r, u8 g, u8 b);
void camBgradSetTop(ColorFade *f, u8 r, u8 g, u8 b);
void camBgradSetBottom(ColorFade *f, u8 r, u8 g, u8 b);
void camBgradSetLines(Camera *cam, u16 top, u16 bottom);
s32 Artsun(Obj *obj, s16 msg, void *arg);
void camSunApplySky(CamCtlWork *w);
void camSunByHour2Sets(CamCtlWork *w);
void camSunByHour4Sets(CamCtlWork *w);
void camSunFixedSet(CamCtlWork *w);
void camSunStoreBlend(CamCtlWork *w, s32 side);
void camSunBeginBlend(CamCtlWork *w, s32 steps, Color3 *src);
void camSunBlendState(CamCtlWork *w);
void camSunTuneWatch(CamCtlWork *w);
void camSunDawnDuskWatch(CamCtlWork *w);
s32 camSunCopyTunes(CamCtlWork *w);
void camSunPlayTune(void *unused, u16 id);
Obj *camSunCreate(Obj *parent);
Obj *camSunDelete(void);
void camSunRefreshHold(s32 on);
void camSunSetClockStop(s32 on);
void camSunSetSkyOn(s32 on);
LightFrame *camSunGetAmbient(void);
void camSunSetColor(s32 r, s32 g, s32 b);
void camSunSetFogColor(s32 r, s32 g, s32 b);
void camSunSetFogNear(s32 v);
void camSunSetView(void *v);
void camSunSetCamBG(void *v);
s32 camSunBlendLevel(void);
void camSunSetProgram(s32 kind);
void camSunShowLights(s32 on);
void camSunCallSetTunes4(u16 a, u16 b, u16 c, u16 d);
void camSunClearTunes(void);
void camSunSetTunes4(u16 dawn, u16 day, u16 evening, u16 night);
void camSunSetTunes5(u16 other, u16 day, u16 evening, u16 third, u16 night);
void camSunSetTunes2(u16 day, u16 night);
void camSunSetDawnSound(u16 v);
void camSunAddSoundPair(u16 a, u16 b);

#endif
