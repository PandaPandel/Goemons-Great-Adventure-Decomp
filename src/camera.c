#include "camera.h"
#include "game.h"
#include "audio.h"
#include "cambg.h"
#include "fade.h"
#include "geography.h"
#include "gfxmgr.h"
#include "joypad.h"
#include "mathutil.h"
#include "sndloc.h"
#include "tmanager.h"
#include "vram.h"

/* A zone's rail: the place along it nearest a point, a point from a place, and the eye's
 * heading at a place. */
extern void *gFrmLayers[]; /* the frame pools */
extern s16 gFrmKindCamera; /* the pools' frame kinds: a view's, */
extern s16 gFrmKindLight; /* a light's, */
extern s16 gFrmKindAmbi;
extern s16 gFrmKindDlist; /* a display list's */
extern Gfx *gGfxDlHead; /* the display list being built */

/* The light's colour sets (overlay 6's .data): four colours each, or two or four sets of four
 * for camSunByHour2Sets and camSunByHour4Sets. */
/* camera.c's .data (0x801077A0-0x80107934), in ROM order. */
Obj *gCamAiObj = NULL; /* the camera manager */
Obj *gCamSunObj = NULL; /* the camera control */
Color3 gCamSunProg0Colors[16] = {
    { 0x30, 0x40, 0xE0 }, { 0xA0, 0xA0, 0xA0 }, { 0xA0, 0xA0, 0xA0 }, { 0xA0, 0xA0, 0xA0 },
    { 0x40, 0x60, 0xE0 }, { 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF },
    { 0xFF, 0x40, 0x20 }, { 0x80, 0x70, 0x10 }, { 0xFF, 0xA0, 0x60 }, { 0xC0, 0x58, 0x18 },
    { 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x40 }, { 0x80, 0x80, 0x80 }, { 0x00, 0x00, 0x00 },
};
Color3 gCamSunProg5Colors[16] = {
    { 0x00, 0x00, 0x00 }, { 0xF1, 0x86, 0x00 }, { 0xA0, 0xA0, 0xA0 }, { 0x78, 0x43, 0x00 },
    { 0x00, 0x00, 0x00 }, { 0xBD, 0x08, 0x00 }, { 0xFF, 0xFF, 0xFF }, { 0x5E, 0x04, 0x00 },
    { 0x00, 0x00, 0x00 }, { 0xA0, 0xB9, 0x27 }, { 0xFF, 0xA0, 0x60 }, { 0x50, 0x3E, 0x14 },
    { 0x00, 0x00, 0x00 }, { 0x54, 0x4E, 0x20 }, { 0x80, 0x80, 0x80 }, { 0x2A, 0x27, 0x10 },
};
Color3 gCamSunProg6Colors[16] = {
    { 0x70, 0x70, 0x70 }, { 0x50, 0x70, 0x50 }, { 0xA0, 0xA0, 0xA0 }, { 0x60, 0x70, 0x60 },
    { 0x60, 0x80, 0x60 }, { 0x50, 0x78, 0x40 }, { 0xFF, 0xFF, 0xFF }, { 0x58, 0x7C, 0x50 },
    { 0xB0, 0x60, 0x40 }, { 0x88, 0x60, 0x3C }, { 0xFF, 0xA0, 0x60 }, { 0x88, 0x6C, 0x3C },
    { 0x20, 0x30, 0x20 }, { 0x10, 0x10, 0x10 }, { 0x80, 0x80, 0x80 }, { 0x10, 0x10, 0x10 },
};
Color3 gCamSunProg7Colors[16] = {
    { 0xD0, 0xD8, 0xF0 }, { 0xD0, 0xD8, 0xF0 }, { 0xA0, 0xA0, 0xA0 }, { 0xD0, 0xD8, 0xF0 },
    { 0xD8, 0xE0, 0xFF }, { 0xD8, 0xE0, 0xFF }, { 0xFF, 0xFF, 0xFF }, { 0xD8, 0xE0, 0xFF },
    { 0xD0, 0xA8, 0x80 }, { 0xD0, 0xA8, 0x80 }, { 0xFF, 0xA0, 0x60 }, { 0xD0, 0xA8, 0x80 },
    { 0x18, 0x20, 0x40 }, { 0x18, 0x20, 0x40 }, { 0x80, 0x80, 0x80 }, { 0x18, 0x20, 0x40 },
};
Color3 gCamSunProg4Colors[8] = {
    { 0x30, 0x30, 0x70 }, { 0x30, 0x30, 0x70 }, { 0xA0, 0xA0, 0xFF }, { 0x30, 0x30, 0x70 },
    { 0x18, 0x18, 0x38 }, { 0x18, 0x18, 0x38 }, { 0x70, 0x70, 0xA0 }, { 0x18, 0x18, 0x38 },
};
Color3 gCamSunProg2Colors[4] = {
    { 0x00, 0x00, 0x00 }, { 0x40, 0x40, 0x40 }, { 0x80, 0x80, 0x80 }, { 0x00, 0x00, 0x00 },
};
Color3 gCamSunProg8Colors[4] = {
    { 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00 }, { 0x80, 0x80, 0x80 }, { 0x00, 0x00, 0x00 },
};
Color3 gCamSunProg9Colors[4] = {
    { 0x70, 0x44, 0x28 }, { 0x70, 0x44, 0x28 }, { 0xD0, 0xD0, 0xD0 }, { 0x70, 0x44, 0x28 },
};
Color3 gCamSunProg10Colors[16] = {
    { 0x3C, 0x50, 0xAA }, { 0xFF, 0xDC, 0xAA }, { 0xA0, 0xA0, 0xA0 }, { 0xA0, 0xA0, 0xA0 },
    { 0x64, 0xA0, 0xFF }, { 0xE6, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF },
    { 0xFF, 0x8C, 0x00 }, { 0xFF, 0x00, 0x00 }, { 0xFF, 0xA0, 0x60 }, { 0xC0, 0x58, 0x18 },
    { 0x00, 0x00, 0x00 }, { 0x00, 0x14, 0x64 }, { 0x80, 0x80, 0x80 }, { 0x00, 0x00, 0x00 },
};
Color3 gCamSunProg11Colors[16] = {
    { 0x3C, 0x50, 0xAA }, { 0xE1, 0xE1, 0xBE }, { 0xA0, 0xA0, 0xA0 }, { 0xA0, 0xA0, 0xA0 },
    { 0x00, 0x14, 0x73 }, { 0x3C, 0x87, 0xFF }, { 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF },
    { 0xFF, 0x00, 0x00 }, { 0xFF, 0x7D, 0x00 }, { 0xFF, 0xA0, 0x60 }, { 0xC0, 0x58, 0x18 },
    { 0x00, 0x00, 0x00 }, { 0x00, 0x14, 0x64 }, { 0x80, 0x80, 0x80 }, { 0x00, 0x00, 0x00 },
};
Color3 gCamSunProg12Colors[16] = {
    { 0x32, 0x5A, 0xFF }, { 0xFF, 0xFF, 0xB4 }, { 0xA0, 0xA0, 0xA0 }, { 0xA0, 0xA0, 0xA0 },
    { 0x00, 0x00, 0x64 }, { 0x73, 0xCD, 0xFF }, { 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF },
    { 0xFF, 0xBE, 0x0A }, { 0xFF, 0x28, 0x00 }, { 0xFF, 0xA0, 0x60 }, { 0xC0, 0x58, 0x18 },
    { 0x00, 0x00, 0x32 }, { 0x00, 0x32, 0x50 }, { 0x80, 0x80, 0x80 }, { 0x00, 0x00, 0x00 },
};
/* GameState's fadeStart and fadeEnd, which the sky and the light store by absolute address
 * even where they hold GameState's base in a register: the original names them apart. */
extern u16 gGameStateFadeStart;
extern u16 gGameStateFadeEnd;

/* The camera: an object of overlay 6 (its handler is dracam: 2 creates it, 1 frees it,
 * 0 runs it) whose view is gGameWork. */

/* One of the camera's view values toward its target by its step, landing on it when nearer
 * than a step (and stopping). */
#define CAM_MOVE(cur, target, step)             \
    if ((step) > 0.00001f) {                    \
        if (fabsf((target) - (cur)) < (step)) { \
            (cur) = (target);                   \
            (step) = 0.0f;                      \
        } else if ((target) < (cur)) {          \
            (cur) = (cur) - (step);             \
        } else {                                \
            (cur) = (cur) + (step);             \
        }                                       \
    }

/* The same step as the sky's colour fade (bgrad) writes it: "-=" and "+=" give cur the
 * lower float register and step the higher, as the game has them (CAM_MOVE swaps the two). */
#define FADE_MOVE(cur, target, step)            \
    if ((step) > 0.00001f) {                    \
        if (fabsf((target) - (cur)) < (step)) { \
            (cur) = (target);                   \
            (step) = 0.0f;                      \
        } else if ((target) < (cur)) {          \
            (cur) -= (step);                    \
        } else {                                \
            (cur) += (step);                    \
        }                                       \
    }

/* The camera "dracam". Message 2 creates it with a view frame from the pool (none left prints a
 * caution and fails), speed 20, every move stopped; its view is the game's (gGameWork). 1 frees
 * the frame. 0 moves the view's values toward their targets, and keeps the look-at point from
 * standing right above the eye. */
s32 dracam(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    CamWork *w = (CamWork *) ((u32) obj + sizeof(Obj));

    switch (msg) {
        case 2:
            obj->name = "dracam";
            w->view = frmCreate(gFrmLayers[0], gFrmKindCamera);
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
                frmDestroy(w->view);
                gGameWork.view = NULL;
            }
            break;
        case 0:
            CAM_MOVE(w->view->fovy, w->fovTarget, w->fovStep);
            CAM_MOVE(w->view->zFar, w->farTarget, w->farStep);
            CAM_MOVE(w->view->eye[0], w->eyeXTarget, w->eyeXStep);
            CAM_MOVE(w->view->eye[1], w->eyeYTarget, w->eyeYStep);
            CAM_MOVE(w->view->eye[2], w->eyeZTarget, w->eyeZStep);
            CAM_MOVE(w->view->at[0], w->atXTarget, w->atXStep);
            CAM_MOVE(w->view->at[1], w->atYTarget, w->atYStep);
            CAM_MOVE(w->view->at[2], w->atZTarget, w->atZStep);
            if (fabsf(w->view->at[0] - w->view->eye[0]) < 1.0f &&
                fabsf(w->view->at[2] - w->view->eye[2]) < 1.0f) {
                w->view->at[2] += 1.0f;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Which of the camera's moves are still going (a step above 0.00001): bit 6 view->fovy, bits
 * 0-2 the eye's x, y and z, and bits 3-5 the look-at's. */
s32 camGetMoving(Camera *cam) {
    s32 moving = 0;
    CamWork *w = (CamWork *) ((u32) cam + sizeof(Obj));

    if (cam->fovStep > 0.00001f) {
        moving = 0x40;
    }
    if (w->eyeXStep > 0.00001f) {
        moving |= 1;
    }
    if (w->eyeYStep > 0.00001f) {
        moving |= 2;
    }
    if (w->eyeZStep > 0.00001f) {
        moving |= 4;
    }
    if (w->atXStep > 0.00001f) {
        moving |= 8;
    }
    if (w->atYStep > 0.00001f) {
        moving |= 0x10;
    }
    if (w->atZStep > 0.00001f) {
        moving |= 0x20;
    }
    return moving;
}

/* Every move stopped. */
void camStop(Camera *cam) {
    cam->fovStep = 0.0f;
    cam->eyeXStep = 0.0f;
    cam->eyeYStep = 0.0f;
    cam->eyeZStep = 0.0f;
    cam->atXStep = 0.0f;
    cam->atYStep = 0.0f;
    cam->atZStep = 0.0f;
}

void camSetSpeed(Camera *cam, f32 speed) {
    cam->speed = speed;
}

/* view->fovy toward a target. */
void camMoveFov(Camera *cam, f32 target) {
    cam->fovTarget = target;
    cam->fovStep = fabsf(target - cam->view->fovy) / cam->speed;
}

/* view->fovy at once. */
void camSetFov(Camera *cam, f32 value) {
    cam->fovStep = 0.0f;
    cam->view->fovy = value;
}

/* view->zFar toward a target. */
void camMoveFar(Camera *cam, f32 target) {
    cam->farTarget = target;
    cam->farStep = fabsf(target - cam->view->zFar) / cam->speed;
}

/* The eye toward a target. */
void camMoveEye(Camera *cam, f32 x, f32 y, f32 z) {
    CamView *view = cam->view;
    f32 speed;

    cam->eyeXTarget = x;
    speed = cam->speed;
    cam->eyeXStep = fabsf(x - view->eye[0]) / speed;
    cam->eyeYTarget = y;
    cam->eyeYStep = fabsf(y - view->eye[1]) / speed;
    cam->eyeZTarget = z;
    cam->eyeZStep = fabsf(z - view->eye[2]) / speed;
}

void camStopXZ(Camera *cam) {
    cam->eyeXStep = 0.0f;
    cam->eyeZStep = 0.0f;
    cam->atXStep = 0.0f;
    cam->atZStep = 0.0f;
}

/* The eye toward a target, each axis only if its target changed (by more than 0.00001). */
void camMoveEyeIfChanged(Camera *cam, f32 x, f32 y, f32 z) {
    CamWork *w = (CamWork *) ((u32) cam + sizeof(Obj));

    if (fabsf(cam->eyeXTarget - x) > 0.00001f) {
        w->eyeXTarget = x;
        w->eyeXStep = fabsf(x - w->view->eye[0]) / w->speed;
    }
    if (fabsf(w->eyeYTarget - y) > 0.00001f) {
        w->eyeYTarget = y;
        w->eyeYStep = fabsf(y - w->view->eye[1]) / w->speed;
    }
    if (fabsf(w->eyeZTarget - z) > 0.00001f) {
        w->eyeZTarget = z;
        w->eyeZStep = fabsf(z - w->view->eye[2]) / w->speed;
    }
}

/* The look-at point toward a target. */
void camMoveAt(Camera *cam, f32 x, f32 y, f32 z) {
    CamView *view = cam->view;
    f32 speed;

    cam->atXTarget = x;
    speed = cam->speed;
    cam->atXStep = fabsf(x - view->at[0]) / speed;
    cam->atYTarget = y;
    cam->atYStep = fabsf(y - view->at[1]) / speed;
    cam->atZTarget = z;
    cam->atZStep = fabsf(z - view->at[2]) / speed;
}

/* The look-at point toward a target, each axis only if its target changed. */
void camMoveAtIfChanged(Camera *cam, f32 x, f32 y, f32 z) {
    CamWork *w = (CamWork *) ((u32) cam + sizeof(Obj));

    if (fabsf(cam->atXTarget - x) > 0.00001f) {
        w->atXTarget = x;
        w->atXStep = fabsf(x - w->view->at[0]) / w->speed;
    }
    if (fabsf(w->atYTarget - y) > 0.00001f) {
        w->atYTarget = y;
        w->atYStep = fabsf(y - w->view->at[1]) / w->speed;
    }
    if (fabsf(w->atZTarget - z) > 0.00001f) {
        w->atZTarget = z;
        w->atZStep = fabsf(z - w->view->at[2]) / w->speed;
    }
}

/* The look-at point's x and z toward targets. */
void camMoveAtXZ(Camera *cam, f32 x, f32 z) {
    CamView *view = cam->view;
    f32 speed;

    cam->atXTarget = x;
    cam->atZTarget = z;
    speed = cam->speed;
    cam->atXStep = fabsf(x - view->at[0]) / speed;
    cam->atZStep = fabsf(z - view->at[2]) / speed;
}

/* The look-at height toward a target. */
void camMoveAtY(Camera *cam, f32 target) {
    cam->atYTarget = target;
    cam->atYStep = fabsf(target - cam->view->at[1]) / cam->speed;
}

/* The eye at once. */
void camSetEye(Camera *cam, f32 x, f32 y, f32 z) {
    cam->eyeZStep = 0.0f;
    cam->eyeYStep = 0.0f;
    cam->eyeXStep = 0.0f;
    cam->view->eye[0] = x;
    cam->view->eye[1] = y;
    cam->view->eye[2] = z;
}

/* The eye's height at once. */
void camSetEyeY(Camera *cam, f32 y) {
    cam->eyeYStep = 0.0f;
    cam->view->eye[1] = y;
}

void camGetEye(Camera *cam, f32 *x, f32 *y, f32 *z) {
    *x = cam->view->eye[0];
    *y = cam->view->eye[1];
    *z = cam->view->eye[2];
}

/* The look-at point at once. */
void camSetAt(Camera *cam, f32 x, f32 y, f32 z) {
    cam->atZStep = 0.0f;
    cam->atYStep = 0.0f;
    cam->atXStep = 0.0f;
    cam->view->at[0] = x;
    cam->view->at[1] = y;
    cam->view->at[2] = z;
}

/* The look-at height at once. */
void camSetAtY(Camera *cam, f32 y) {
    cam->atYStep = 0.0f;
    cam->view->at[1] = y;
}

void camGetAt(Camera *cam, f32 *x, f32 *y, f32 *z) {
    *x = cam->view->at[0];
    *y = cam->view->at[1];
    *z = cam->view->at[2];
}

CamView *camGetView(Camera *cam) {
    return cam->view;
}

/* The camera at its targets at once. The work pointer is made through an integer, which IDO
 * does not fold into the offsets (IDO_NOTES rule 72). */
void camSnapToTargets(Camera *cam) {
    CamWork *w = (CamWork *) ((u32) cam + sizeof(Obj));

    camSetEye(cam, cam->eyeXTarget, cam->eyeYTarget, cam->eyeZTarget);
    camSetAt(cam, w->atXTarget, w->atYTarget, w->atZTarget);
}

/* The look-at target's offset from the look-at point, turned by the angle less a quarter turn
 * (mathVecRotateY turns a vector about y); its x. */
f32 camAtRemainSide(Camera *cam, f32 angle) {
    s32 pad[2];
    Vec3f v;

    v.x = cam->atXTarget - cam->view->at[0];
    v.y = 0.0f;
    v.z = cam->atZTarget - cam->view->at[2];
    mathVecRotateY(-(angle - 1.5707964f), &v.x, &v.y, &v.z);
    return v.x;
}

/* How far the look-at height has still to go. */
f32 camAtRemainY(Camera *cam) {
    return cam->atYTarget - cam->view->at[1];
}

/* The camera manager "AI Cam". Message 2 creates it with its camera ("dracam") under it: the
 * eye at (0, 50, 200), speed 8, the view's 40 degrees, 20 and 800, the light, and the rails
 * with limits as its follow mode. 1 frees it. 0 runs it: unless held (flag 4), it stops the
 * camera (flag 2) or runs its follow mode at speed 8. */
s32 AICam(Obj *obj, s16 msg, void *arg) {
    CamMgrWork *w = (CamMgrWork *) ((u32) obj + sizeof(Obj));

    switch (msg) {
        case 2:
            obj->name = "AI Cam";
            w->cam = (Camera *) tskCreate(obj, dracam, 0, 0);
            if (w->cam == NULL) {
                return -1;
            }
            camSetEye(w->cam, 0.0f, 50.0f, 200.0f);
            camSetSpeed(w->cam, 8.0f);
            w->view = camGetView(w->cam);
            w->view->zNear = 20.0f;
            w->view->zFar = 800.0f;
            w->view->fovy = 0.6981317401f;
            camSunCreate(obj);
            camSunSetView(w->view);
            geoSetView(w->view);
            w->pitch = 0x3A;
            w->hook = camAiFollowRailLimited;
            w->flags |= 1;
            w->dist = 160.0f;
            w->angle = 0.0f;
            break;
        case 1:
            geoSetView(NULL);
            gCamAiObj = NULL;
            break;
        case 0:
            if (!(w->flags & 4)) {
                if (w->flags & 2) {
                    camStop(w->cam);
                } else {
                    camSetSpeed(w->cam, 8.0f);
                    w->hook(w);
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The least and the greatest of the first n of four values, and their midpoint. */
f32 camMinMaxMid(f32 *min, f32 *max, s32 n, f32 a, f32 b, f32 c, f32 d) {
    s32 i;
    f32 v[4];
    f32 lo;
    f32 hi;

    v[0] = a;
    v[1] = b;
    v[2] = c;
    v[3] = d;
    lo = v[0];
    for (i = 1; i < n; i++) {
        if (v[i] < lo) {
            lo = v[i];
        }
    }
    hi = v[0];
    for (i = 1; i < n; i++) {
        if (hi < v[i]) {
            hi = v[i];
        }
    }
    *min = lo;
    *max = hi;
    return (lo + hi) / 2;
}

/* The midpoint of the first n values. */
f32 camMidOf(s32 n, f32 x, f32 y, f32 z, f32 w) {
    f32 a;
    f32 b;

    return camMinMaxMid(&a, &b, n, x, y, z, w);
}

/* The least of the first n values. */
f32 camMinOf(s32 n, f32 x, f32 y, f32 z, f32 w) {
    f32 a;
    f32 b;

    camMinMaxMid(&a, &b, n, x, y, z, w);
    return a;
}

/* The greatest of the first n values. */
f32 camMaxOf(s32 n, f32 x, f32 y, f32 z, f32 w) {
    f32 a;
    f32 b;

    camMinMaxMid(&a, &b, n, x, y, z, w);
    return b;
}

/* The player's pan of the view, then the follow point moved by it. While the manager runs
 * (flag 1), each follower's stick (/16) and D-pad (4) pan it, sideways if flags bit 0 and up
 * and down if bit 1, never past how far the point moved (in the view's frame). Left alone, the
 * pan first drops the look-at's own offset and stops the camera, then after 11 frames eases back
 * (0.95 a frame). It stays within 40, and sideways within the view's reach (fadeViewWidth,
 * plus 20) beyond the distance. */
void camAiPan(CamMgrWork *w, Vec3f *at, f32 dist, u16 flags) {
    f32 pad[2];
    Vec3f d;
    f32 panX;
    f32 panY;
    s32 i;
    s32 buttons;
    f32 f;
    Vec3f off;
    f32 lim;

    if (w->flags & 1) {
        d.x = at->x - w->pos.x;
        d.y = at->y - w->pos.y;
        d.z = at->z - w->pos.z;
        mathVecRotateY(-(w->angle - PI / 2), &d.x, &d.y, &d.z);
        panX = 0.0f;
        panY = 0.0f;
        for (i = 0; i < w->nReq; i++) {
            if (w->req[i].arg != 0) {
                panX += joyGetStickX(w->req[i].kind, 0) / 16;
                panY += joyGetStickY(w->req[i].kind, 0) / 16;
                buttons = joyGetButton(i, 0);
                if (buttons & 0x200) {
                    panX -= 4.0f;
                } else if (buttons & 0x100) {
                    panX += 4.0f;
                }
                if (buttons & 0x800) {
                    panY += 4.0f;
                } else if (buttons & 0x400) {
                    panY -= 4.0f;
                }
            }
        }
        if (fabsf(d.x) < fabsf(panX)) {
            panX = d.x;
        }
        if (fabsf(d.y) < fabsf(panY)) {
            panY = d.y;
        }
        if (!(flags & 1)) {
            panX = 0.0f;
        }
        if (!(flags & 2)) {
            panY = 0.0f;
        }
        if ((s32) panX == 0) {
            if (w->idleX == 0) {
                f = camAtRemainSide(w->cam, w->angle);
                if (w->panX * f < 0.0f) {
                    f = 0.0f;
                }
                w->panX -= f;
                camStop(w->cam);
            } else if (w->idleX >= 11) {
                w->panX *= 0.95f;
            }
            w->idleX++;
        } else {
            w->idleX = 0;
            w->panX += panX;
        }
        if ((s32) panY == 0) {
            if (w->idleY == 0) {
                f = camAtRemainY(w->cam);
                if (w->panY * f < 0.0f) {
                    f = 0.0f;
                }
                w->panY -= f;
                camStop(w->cam);
            } else if (w->idleY >= 11) {
                w->panY *= 0.95f;
            }
            w->idleY++;
        } else {
            w->idleY = 0;
            w->panY += panY;
        }
        if (w->panX < -40.0f) {
            w->panX = -40.0f;
        } else if (w->panX > 40.0f) {
            w->panX = 40.0f;
        }
        if (w->panY < -40.0f) {
            w->panY = -40.0f;
        } else if (w->panY > 40.0f) {
            w->panY = 40.0f;
        }
        f = fadeViewWidth(w->view) + 20.0f;
        lim = f - dist;
        if (f < dist) {
            lim = 0.0f;
        }
        if (lim < fabsf(w->panX)) {
            if (w->panX > 0.0f) {
                w->panX = lim;
            } else if (w->panX < 0.0f) {
                w->panX = -lim;
            }
        }
    }
    w->pos = *at;
    off.x = w->panX;
    off.y = w->panY;
    off.z = 0.0f;
    mathVecRotateY(w->angle - PI / 2, &off.x, &off.y, &off.z);
    at->x += off.x;
    at->y += off.y;
    at->z += off.z;
}

/* Looks at `at`, with the eye at the manager's distance from it: its pitch a byte (256 a full
 * turn) and its heading the angle less a quarter turn. */
void camAiLookAt(CamMgrWork *w, Vec3f *at) {
    camMoveAt(w->cam, at->x, at->y, at->z);
    camMoveEye(
        w->cam,
        w->dist * sinf((f32) w->pitch / 256 * 2 * PI) * sinf(w->angle - PI / 2) + at->x,
        cosf((f32) w->pitch / 256 * 2 * PI) * w->dist + at->y,
        w->dist * sinf((f32) w->pitch / 256 * 2 * PI) * cosf(w->angle - PI / 2) + at->z);
}

/* Follows objects on rails, as camAiFollowRail, within the zone's limits (the events). In one
 * zone, the view's height is 10 above the rail while their middle is less than 50 above it,
 * rises to their middle over the next 30, and follows the middle from 80. A limited place stops
 * the camera's moves (camStopXZ). */
void camAiFollowRailLimited(CamMgrWork *w) {
    CamTarget tgt[4];
    f32 h;
    f32 t;
    s32 i;
    f32 y;
    f32 min;
    f32 max;
    f32 dist;
    Vec3f at;
    s32 same;
    f32 dz;
    f32 diff;
    u16 moved;
    s16 zone;
    Vec3f pos[4];
    f32 dx;
    s32 pad[2];

    moved = TRUE;
    tgt[0].zone = tgt[0].t = tgt[0].side = tgt[0].y = 0.0f;
    for (i = 0; i < w->nReq; i++) {
        if (i != 0) {
            tgt[i] = tgt[0];
        }
        if ((Obj *) w->req[i].arg != NULL) {
            tskSendMsg((Obj *) w->req[i].arg, 5, &tgt[i]);
        }
    }
    same = TRUE;
    for (i = 1; i < w->nReq; i++) {
        if (tgt[i - 1].zone != tgt[i].zone) {
            same = FALSE;
            break;
        }
    }
    if (same == TRUE) {
        zone = tgt[0].zone;
        t = camMinMaxMid(&min, &max, w->nReq, tgt[0].t, tgt[1].t, tgt[2].t, tgt[3].t);
        for (i = 0; i < w->nEvt; i++) {
            if (zone == w->evt[i].zone) {
                if (t * w->evt[i].sign > w->evt[i].limit * w->evt[i].sign) {
                    moved = FALSE;
                    t = w->evt[i].limit;
                }
            }
        }
        y = camMidOf(w->nReq, tgt[0].y, tgt[1].y, tgt[2].y, tgt[3].y) + 10.0f;
        vramCoursePoint(zone, t, 0.0f, &at.x, &h, &at.z);
        diff = y - h;
        dist = fabsf(min - max);
        if (diff < 50.0f) {
            at.y = h + 10.0f;
        } else if (diff >= 50.0f && diff < 80.0f) {
            at.y = h + 10.0f + (diff - 50.0f) * 70.0f / 30.0f;
        } else {
            at.y = y;
        }
        w->angle = vramCourseHeading(zone, t);
    } else {
        pos[0].x = pos[0].y = pos[0].z = 0.0f;
        for (i = 0; i < w->nReq; i++) {
            if (i != 0) {
                pos[i] = pos[0];
            }
            vramCoursePointXZ(tgt[i].zone, tgt[i].t, 0.0f, &pos[i].x, &pos[i].z);
        }
        at.x = camMinMaxMid(&min, &max, w->nReq, pos[0].x, pos[1].x, pos[2].x, pos[3].x);
        dx = fabsf(min - max);
        at.y = camMinMaxMid(&min, &max, w->nReq, tgt[0].y, tgt[1].y, tgt[2].y, tgt[3].y) +
               10.0f;
        at.z = camMinMaxMid(&min, &max, w->nReq, pos[0].z, pos[1].z, pos[2].z, pos[3].z);
        dz = fabsf(min - max);
        dist = sqrtf(dx * dx + dz * dz);
        w->angle = vramCourseHeading(tgt[0].zone, tgt[0].t);
    }
    camAiPan(w, &at, dist, 1);
    camAiLookAt(w, &at);
    if (!(moved & 1)) {
        camStopXZ(w->cam);
    }
}

/* Follow mode 1, a free camera on controller 1: with C-right held, D-pad up and down bring the
 * eye nearer and further (by 2); otherwise they tilt it (the pitch, 0-0x7F) and left and right
 * turn it (the heading byte). It looks at the first followed object's point on its rail. */
void camAiFreeLook(CamMgrWork *w) {
    s32 buttons;
    CamTarget tgt;
    f32 x;
    f32 z;

    buttons = joyGetButton(0, 0);
    if (buttons & 1) {
        if (buttons & 0x800) {
            w->dist -= 2.0f;
        }
        if (buttons & 0x400) {
            w->dist += 2.0f;
        }
    } else {
        if (buttons & 0x800) {
            w->pitch--;
            if (w->pitch >= 0x80) {
                w->pitch = 0;
            }
        }
        if (buttons & 0x400) {
            w->pitch++;
            if (w->pitch >= 0x80) {
                w->pitch = 0x7F;
            }
        }
        if (buttons & 0x100) {
            w->heading++;
        }
        if (buttons & 0x200) {
            w->heading--;
        }
    }
    tgt.zone = tgt.t = tgt.side = tgt.y = 0.0f;
    if ((Obj *) w->req[0].arg != NULL) {
        tskSendMsg((Obj *) w->req[0].arg, 5, &tgt);
    }
    vramCoursePointXZ(tgt.zone, tgt.t, tgt.side, &x, &z);
    camMoveAt(w->cam, x, tgt.y, z);
    camMoveEye(
        w->cam,
        w->dist * sinf((f32) w->pitch / 256 * 2 * PI) * sinf((f32) w->heading / 256 * 2 * PI) + x,
        cosf((f32) w->pitch / 256 * 2 * PI) * w->dist + tgt.y,
        w->dist * sinf((f32) w->pitch / 256 * 2 * PI) * cosf((f32) w->heading / 256 * 2 * PI) + z);
}

/* Follow mode 2, a flying camera on controller 1: the eye at +0x38 moves with the stick along
 * the view's ground direction and sideways, or up and down with C-right held (and the D-pad
 * then sets the distance). Otherwise the D-pad tilts and turns the view, as mode 1. It looks
 * back from the eye along the pitch and heading. */
void camAiFly(CamMgrWork *w) {
    u16 buttons;
    s32 pad[7];
    f32 sx;
    f32 sy;
    f32 dx;
    f32 dz;
    f32 len;

    buttons = joyGetButton(0, 0);
    sx = joyGetStickX(0, 0) / 16;
    sy = joyGetStickY(0, 0) / 16;
    if (buttons & 1) {
        w->pos.y += sy / 2;
    } else {
        dx = w->view->at[0] - w->view->eye[0];
        dz = w->view->at[2] - w->view->eye[2];
        /* Written `(*sqrtf)`: IDO calls the library's sqrtf through t9 (`jalr`), where a plain
         * `sqrtf` is inline (IDO_NOTES rule 79). */
        len = (*sqrtf)(dx * dx + dz * dz);
        dx /= len;
        dz /= len;
        w->pos.x += dx * sy;
        w->pos.z += dz * sy;
        w->pos.x += -dz * sx;
        w->pos.z += dx * sx;
    }
    if (buttons & 1) {
        if (buttons & 0x800) {
            w->dist -= 2;
        }
        if (buttons & 0x400) {
            w->dist += 2;
        }
    } else {
        if (buttons & 0x800) {
            w->pitch--;
            if (w->pitch >= 0x80) {
                w->pitch = 0;
            }
        }
        if (buttons & 0x400) {
            w->pitch++;
            if (w->pitch >= 0x80) {
                w->pitch = 0x7F;
            }
        }
        if (buttons & 0x100) {
            w->heading++;
        }
        if (buttons & 0x200) {
            w->heading--;
        }
    }
    camMoveEye(w->cam, w->pos.x, w->pos.y, w->pos.z);
    camMoveAt(
        w->cam,
        w->pos.x - w->dist * sinf((f32) w->pitch / 256 * 2 * PI) * sinf((f32) w->heading / 256 * 2 * PI),
        w->pos.y - cosf((f32) w->pitch / 256 * 2 * PI) * w->dist,
        w->pos.z - w->dist * sinf((f32) w->pitch / 256 * 2 * PI) * cosf((f32) w->heading / 256 * 2 * PI));
}

/* Follows the middle of the objects (asked for their positions with message 4), 10 above it.
 * A ray down from 10 above the highest to 10 below the lowest finds the ground and its zone. The
 * place on the zone's rail nearest the middle is kept within the zone's limits (the events); a
 * limited place moves the middle onto the rail. The place gives the eye's heading. */
void camAiFollowGround(CamMgrWork *w) {
    Vec3f pos[4];
    s32 i;
    f32 t;
    s32 moved;
    CollTri *hit;
    Vec3f mid;
    s32 pad[3];
    Vec3f from;
    Vec3f dir;
    Vec3f at;
    f32 arg;
    s32 pad2[2];

    moved = TRUE;
    pos[0].x = pos[0].y = pos[0].z = 0.0f;
    for (i = 0; i < w->nReq; i++) {
        if (i != 0) {
            pos[i] = pos[0];
        }
        if ((Obj *) w->req[i].arg != NULL) {
            tskSendMsg((Obj *) w->req[i].arg, 4, &pos[i]);
        }
    }
    mid.x = camMidOf(w->nReq, pos[0].x, pos[1].x, pos[2].x, pos[3].x);
    mid.y = camMidOf(w->nReq, pos[0].y, pos[1].y, pos[2].y, pos[3].y) + 10.0f;
    mid.z = camMidOf(w->nReq, pos[0].z, pos[1].z, pos[2].z, pos[3].z);
    from.x = mid.x;
    from.y = camMaxOf(w->nReq, pos[0].y, pos[1].y, pos[2].y, pos[3].y) + 10.0f;
    from.z = mid.z;
    dir.x = 0.0f;
    dir.y = (camMinOf(w->nReq, pos[0].y, pos[1].y, pos[2].y, pos[3].y) - 10.0f) - from.y;
    dir.z = 0.0f;
    hit = vramRayGround(&from, &dir, &at, &arg);
    if (hit != NULL && hit->zone != 0) {
        w->zone = hit->zone;
    }
    t = vramNearestPlaceXZ(w->zone, mid.x, mid.z, &arg);
    for (i = 0; i < w->nEvt; i++) {
        if (w->zone == w->evt[i].zone) {
            if (t * w->evt[i].sign > w->evt[i].limit * w->evt[i].sign) {
                t = w->evt[i].limit;
                vramCoursePointXZ(w->zone, t, arg, &mid.x, &mid.z);
                moved = FALSE;
            }
        }
    }
    w->angle = vramCourseHeading(w->zone, t);
    camAiPan(w, &mid, 0.0f, moved);
    camAiLookAt(w, &mid);
}

/* Looks at the middle of the objects the camera follows (each asked for its position with
 * message 4), at the eye's height less 5, and moves the eye to its place. With events, the view
 * turns only while the middle stays within the angle of the direction to (unk44, unk48).
 * The eye's x and z are read inline: IDO keeps the x in a register and the z in a temporary
 * at 0x3C; named locals for them would each take a slot (frame 0xA0, not 0x98). */
void camAiFixedEye(CamMgrWork *w) {
    Vec3f pos[4];
    Vec3f mid;
    s32 i;
    f32 cosA;
    f32 dx;
    f32 dz;
    f32 ax;
    f32 az;

    pos[0].x = pos[0].y = pos[0].z = 0.0f;
    for (i = 0; i < w->nReq; i++) {
        if (i != 0) {
            pos[i] = pos[0];
        }
        if ((Obj *) w->req[i].arg != NULL) {
            tskSendMsg((Obj *) w->req[i].arg, 4, &pos[i]);
        }
    }
    mid.x = camMidOf(w->nReq, pos[0].x, pos[1].x, pos[2].x, pos[3].x);
    mid.y = camMidOf(w->nReq, pos[0].y, pos[1].y, pos[2].y, pos[3].y);
    mid.z = camMidOf(w->nReq, pos[0].z, pos[1].z, pos[2].z, pos[3].z);
    if (w->nEvt != 0) {
        dx = mid.x - w->view->eye[0];
        dz = mid.z - w->view->eye[2];
        ax = w->panX - w->view->eye[0];
        az = w->panY - w->view->eye[2];
        cosA = (ax * dx + az * dz) / sqrtf((ax * ax + az * az) * (dx * dx + dz * dz));
        if (cosA < mathCos(w->angle)) {
            return;
        }
    }
    camMoveAt(w->cam, mid.x, w->pos.y - 5.0f, mid.z);
    camMoveEye(w->cam, w->pos.x, w->pos.y, w->pos.z);
}

/* Follows objects on rails: each gives its zone and place (message 5). All in one zone, the
 * view goes to the middle place on the rail, 10 below their middle height or 40 below the rail,
 * whichever is lower, at the spread of their places as its distance. In different zones, it goes
 * to the middle of their rail points, 10 above their height, at the width of their spread. */
void camAiFollowRail(CamMgrWork *w) {
    CamTarget tgt[4];
    f32 h;
    f32 t;
    s32 i;
    f32 y;
    f32 min;
    f32 max;
    f32 dist;
    Vec3f at;
    s32 zones;
    f32 dx;
    f32 dz;
    Vec3f pos[4];
    f32 lim;
    s16 zone;

    tgt[0].zone =tgt[0].t = tgt[0].side = tgt[0].y = 0.0f;
    for (i = 0; i < w->nReq; i++) {
        if (i != 0) {
            tgt[i] = tgt[0];
        }
        if ((Obj *) w->req[i].arg != NULL) {
            tskSendMsg((Obj *) w->req[i].arg, 5, &tgt[i]);
        }
    }
    zones = 0;
    for (i = 0; i < w->nReq; i++) {
        zones |= tgt[i].zone;
    }
    if (zones == tgt[0].zone) {
        zone = tgt[0].zone;
        t = camMinMaxMid(&min, &max, w->nReq, tgt[0].t, tgt[1].t, tgt[2].t, tgt[3].t);
        y = camMidOf(w->nReq, tgt[0].y, tgt[1].y, tgt[2].y, tgt[3].y) - 10.0f;
        vramCoursePoint(zone, t, 0.0f, &at.x, &h, &at.z);
        lim = h - 40.0f;
        dist = fabsf(min - max);
        if (lim < y) {
            at.y = lim;
        } else {
            at.y = y;
        }
        w->angle = vramCourseHeading(zone, t);
    } else {
        pos[0].x = pos[0].y = pos[0].z = 0.0f;
        for (i = 0; i < w->nReq; i++) {
            if (i != 0) {
                pos[i] = pos[0];
            }
            vramCoursePointXZ(tgt[i].zone, tgt[i].t, 0.0f, &pos[i].x, &pos[i].z);
        }
        at.x = camMinMaxMid(&min, &max, w->nReq, pos[0].x, pos[1].x, pos[2].x, pos[3].x);
        dx = fabsf(min - max);
        at.y = camMinMaxMid(&min, &max, w->nReq, tgt[0].y, tgt[1].y, tgt[2].y, tgt[3].y) +
               10.0f;
        at.z = camMinMaxMid(&min, &max, w->nReq, pos[0].z, pos[1].z, pos[2].z, pos[3].z);
        dz = fabsf(min - max);
        dist = sqrtf(dx * dx + dz * dz);
        w->angle = vramCourseHeading(tgt[0].zone, tgt[0].t);
    }
    camAiPan(w, &at, dist, 3);
    camAiLookAt(w, &at);
}

/* Creates the camera manager under a parent, once. */
Obj *camAiCreate(Obj *parent) {
    if (gCamAiObj != NULL) {
        return NULL;
    }
    return gCamAiObj = tskCreateBlocks(parent, AICam, 2, 0, 0);
}

/* Sets the manager's follow mode: 0 (and any other) rails with limits, 1 camAiFreeLook, 2 the
 * eye kept where it is (camAiFly), 3 the ground's zone, 4 a fixed eye, 5 rails. */
void camAiSetMode(s32 mode) {
    CamMgrWork *w = (CamMgrWork *) (gCamAiObj + 1);

    if (gCamAiObj != NULL) {
        switch (mode) {
            case 0:
            default:
                w->mode = 0;
                w->hook = camAiFollowRailLimited;
                break;
            case 1:
                w->mode = 1;
                w->hook = camAiFreeLook;
                break;
            case 2:
                w->mode = 2;
                camGetEye(w->cam, &w->pos.x, &w->pos.y, &w->pos.z);
                w->hook = camAiFly;
                break;
            case 3:
                w->mode = 3;
                w->hook = camAiFollowGround;
                break;
            case 4:
                w->mode = 4;
                w->hook = camAiFixedEye;
                break;
            case 5:
                w->mode = 5;
                w->hook = camAiFollowRail;
                break;
        }
    }
}

void camAiSetStop(s32 on) {
    CamMgrWork *w = (CamMgrWork *) (gCamAiObj + 1);

    if (gCamAiObj != NULL) {
        if (on == TRUE) {
            w->flags |= 2;
            return;
        }
        w->flags &= ~2;
    }
}

void camAiSetHold(s32 on) {
    CamMgrWork *w = (CamMgrWork *) (gCamAiObj + 1);

    if (gCamAiObj != NULL) {
        if (on == TRUE) {
            w->flags |= 4;
            return;
        }
        w->flags &= ~4;
    }
}

CamView *camAiGetView(void) {
    CamMgrWork *w = (CamMgrWork *) (gCamAiObj + 1);

    if (gCamAiObj == NULL) {
        return NULL;
    }
    return w->view;
}

void *camAiGetCam(void) {
    CamMgrWork *w = (CamMgrWork *) (gCamAiObj + 1);

    if (gCamAiObj == NULL) {
        return NULL;
    }
    return w->cam;
}

void camAiGetAt(f32 *x, f32 *y, f32 *z) {
    CamMgrWork *w = (CamMgrWork *) (gCamAiObj + 1);

    if (gCamAiObj == NULL) {
        *x = 0.0f;
        *y = 0.0f;
        *z = 0.0f;
        return;
    }
    *x = w->view->at[0];
    *y = w->view->at[1];
    *z = w->view->at[2];
}

/* Queues a request for the manager (up to 4). */
void camAiAddLookat(s32 arg, s32 kind) {
    CamMgrWork *w = (CamMgrWork *) (gCamAiObj + 1);

    if (gCamAiObj != NULL && w->nReq < 4) {
        w->req[w->nReq].kind = kind;
        w->req[w->nReq].arg = arg;
        w->nReq++;
    }
}

/* aicamDeleteLookat: removes the manager's request (a followed object) for arg, closing the
 * gap. */
void aicamDeleteLookat(s32 arg) {
    CamMgrWork *w = (CamMgrWork *) (gCamAiObj + 1);
    s32 i;
    s32 j;

    if (gCamAiObj != NULL) {
        for (i = 0; i < w->nReq; i++) {
            if (arg == w->req[i].arg) {
                for (j = i; j < w->nReq - 1; j++) {
                    w->req[j] = w->req[j + 1];
                }
                w->req[j].kind = 0;
                w->req[j].arg = 0;
                w->nReq--;
                return;
            }
        }
        PRINTF("Warning: in aicamDeleteLookat !\n");
    }
}

/* Runs the manager's hook (flag 1 clear meanwhile), then applies it to the camera. */
void camAiSnap(void) {
    CamMgrWork *w = (CamMgrWork *) (gCamAiObj + 1);

    if (gCamAiObj != NULL) {
        w->flags &= ~1;
        w->hook(w);
        w->flags |= 1;
        camSnapToTargets(w->cam);
    }
}

void camAiSetFar(s16 v) {
    CamMgrWork *w = (CamMgrWork *) (gCamAiObj + 1);

    if (gCamAiObj != NULL) {
        w->view->zFar = v;
    }
}

void camAiSetPos(f32 x, f32 y, f32 z) {
    CamMgrWork *w = (CamMgrWork *) (gCamAiObj + 1);

    if (gCamAiObj != NULL) {
        w->pos.x = x;
        w->pos.y = y;
        w->pos.z = z;
    }
}

void camAiGetPan(f32 *out) {
    CamMgrWork *w = (CamMgrWork *) (gCamAiObj + 1);

    if (gCamAiObj != NULL) {
        out[0] = w->panX;
        out[1] = w->panY;
        out[2] = 0.0f;
    }
}

/* Queues an event for the manager (up to 12). The index read again inside the if (n) leaves
 * the sign in its home (sw a1,4(sp), then lw) and the zone copied to a3. */
void camAiAddLimit(s32 zone, s32 sign, s32 limit) {
    CamMgrWork *w = (CamMgrWork *) (gCamAiObj + 1);
    s32 n;

    if (gCamAiObj != NULL && w->nEvt < 12) {
        n = w->nEvt;
        w->evt[n].zone = zone;
        w->evt[n].sign = sign;
        w->evt[n].limit = limit;
        w->nEvt++;
    }
}

void camAiSetTurnLimit(f32 x, f32 y, f32 z) {
    CamMgrWork *w = (CamMgrWork *) (gCamAiObj + 1);

    if (gCamAiObj != NULL) {
        w->angle = z;
        w->panX = x;
        w->panY = y;
        w->nEvt = 1;
    }
}

/* The sky, "bgrad": a colour fade whose two colours are the frame's gradient fill (top and
 * bottom), rounded. Message 2 turns the gradient on over lines 0-240 at speed 10, 1 turns it off
 * and blacks the top, 0 moves both colours toward their targets.
 * Matched 2026-10-03 (130 lines to a match) on three facts, a fourth unfaked it. (1) The moves are written with
 * "cur -= step" / "cur += step" (FADE_MOVE): with "cur = cur - step" (CAM_MOVE) step took the
 * lower float register and cur the higher, the game's the other way round (130 to 11). (2) Case 2
 * is Artsun's shape (Artsun): the fade's first line is stored to the work and read back,
 * "fadeStart = w->topLine; fadeEnd = w->lines". ugen reloads both; as1 forwards the first reload
 * (no aliasing store between) but not the second (fadeStart's store may alias), so the first
 * spends t8 invisibly and lines comes back in t9. as1 forwards only an lhu: as an s16 (camera.h's
 * type until 2026-10-17, now u16: a kept FAKE), the lh stays (vi, 506). (3) Case 1 is three plain stores as in Artsun; its old
 * chain only spent the temporary case 2 now spends. (4) Case 2 clears the colours BEFORE it sets
 * the lines: as1 still puts the two halfword stores first (in the shadow of the mtc1s), and the
 * zero gets f2. Written with the lines first, the zero took f14 (192 lines) unless an empty
 * block boundary split the line stores from the clear (the fakes sweep, 2026-10-03). */
s32 bgrad(Obj *obj, s16 msg, void *arg) {
    ColorFadeWork *w = (ColorFadeWork *) ((u32) obj + sizeof(Obj));

    switch (msg) {
        case 2:
            obj->name = "bgrad";
            gGameState.fade = 1;
            w->cur1[2] = w->cur1[1] = w->cur1[0] = w->step1[2] = w->step1[1] = w->step1[0] =
                w->target1[2] = w->target1[1] = w->target1[0] = w->cur0[2] = w->cur0[1] =
                    w->cur0[0] = w->step0[2] = w->step0[1] = w->step0[0] = w->target0[2] =
                        w->target0[1] = w->target0[0] = 0.0f;
            w->topLine = 0;
            w->lines = 240;
            w->speed = 10.0f;
            gGameStateFadeStart = w->topLine;
            gGameStateFadeEnd = w->lines;
            break;
        case 1:
            gGameState.fade = 0;
            gGameState.fadeFrom.r = 0;
            gGameState.fadeFrom.g = 0;
            gGameState.fadeFrom.b = 0;
            break;
        case 0:
            FADE_MOVE(w->cur0[0], w->target0[0], w->step0[0]);
            FADE_MOVE(w->cur0[1], w->target0[1], w->step0[1]);
            FADE_MOVE(w->cur0[2], w->target0[2], w->step0[2]);
            FADE_MOVE(w->cur1[0], w->target1[0], w->step1[0]);
            FADE_MOVE(w->cur1[1], w->target1[1], w->step1[1]);
            FADE_MOVE(w->cur1[2], w->target1[2], w->step1[2]);
            gGameState.fadeFrom.r = w->cur0[0] + 0.5f;
            gGameState.fadeFrom.g = w->cur0[1] + 0.5f;
            gGameState.fadeFrom.b = w->cur0[2] + 0.5f;
            gGameState.fadeTo[0] = w->cur1[0] + 0.5f;
            gGameState.fadeTo[1] = w->cur1[1] + 0.5f;
            gGameState.fadeTo[2] = w->cur1[2] + 0.5f;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

void camBgradSetSpeed(ColorFade *f, f32 speed) {
    f->speed = speed;
}

/* The first colour toward a target. */
void camBgradMoveTop(ColorFade *f, u8 r, u8 g, u8 b) {
    f32 speed;

    f->target0[0] = r;
    f->target0[1] = g;
    f->target0[2] = b;
    speed = f->speed;
    f->step0[0] = fabsf(f->target0[0] - f->cur0[0]) / speed;
    f->step0[1] = fabsf(f->target0[1] - f->cur0[1]) / speed;
    f->step0[2] = fabsf(f->target0[2] - f->cur0[2]) / speed;
}

/* The second colour toward a target. */
void camBgradMoveBottom(ColorFade *f, u8 r, u8 g, u8 b) {
    f32 speed;

    f->target1[0] = r;
    f->target1[1] = g;
    f->target1[2] = b;
    speed = f->speed;
    f->step1[0] = fabsf(f->target1[0] - f->cur1[0]) / speed;
    f->step1[1] = fabsf(f->target1[1] - f->cur1[1]) / speed;
    f->step1[2] = fabsf(f->target1[2] - f->cur1[2]) / speed;
}

/* The first colour at once. */
void camBgradSetTop(ColorFade *f, u8 r, u8 g, u8 b) {
    f->step0[0] = 0.0f;
    f->step0[1] = 0.0f;
    f->step0[2] = 0.0f;
    f->cur0[0] = r;
    f->cur0[1] = g;
    f->cur0[2] = b;
}

/* The second colour at once. */
void camBgradSetBottom(ColorFade *f, u8 r, u8 g, u8 b) {
    f->step1[0] = 0.0f;
    f->step1[1] = 0.0f;
    f->step1[2] = 0.0f;
    f->cur1[0] = r;
    f->cur1[1] = g;
    f->cur1[2] = b;
}

/* The band of the screen drawn (the frame's fade lines), kept by the camera too. Both are read
 * back for the frame: the top one is forwarded from its store (its register, t6, is spent for
 * it), the bottom one reloaded after the store to the frame. */
void camBgradSetLines(Camera *cam, u16 top, u16 bottom) {
    cam->topLine = top;
    cam->bottomLine = bottom;
    gGameState.fadeStart = cam->topLine;
    gGameState.fadeEnd = cam->bottomLine;
}

/* The light, "Artsun". Message 2 takes its frames (a display list's and two lights') and sets
 * the sky's gradient over lines 20-100, fog from 994, and the four-set colours by the hour.
 * 1 frees them. 0, unless held (flag 1): advances the clock (unless stopped, flag 2) by a day in
 * daySeconds, runs its state and watcher, restarts its paired sounds every 16 frames, points
 * the sun along the view, and sets the sky (flag 4). 3 draws: the environment colour (also the
 * sun's), the fog's colour and range. */
s32 Artsun(Obj *obj, s16 msg, void *arg) {
    CamCtlWork *w = (CamCtlWork *) ((u32) obj + sizeof(Obj));
    LightFrame *frame;
    s32 i;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 s;
    s32 r;
    s32 g;
    s32 b;

    switch (msg) {
        case 2:
            obj->name = "Artsun";
            w->dlFrame = frmCreateTop(gFrmLayers[3], gFrmKindDlist);
            w->sun = frmCreate(gFrmLayers[0], gFrmKindLight);
            frame = frmCreate(w->sun, gFrmKindLight);
            frame->color[0] = 0;
            frame->color[1] = 0;
            frame->color[2] = 0;
            w->ambient = frmCreate(gFrmLayers[0], gFrmKindAmbi);
            w->topLine = 20;
            w->bottomLine = 100;
            gGameState.fade = 1;
            gGameStateFadeStart = w->topLine;
            gGameStateFadeEnd = w->bottomLine;
            w->fogNear = 0x3E2;
            w->dawnSound = 0x42F;
            w->state = camSunByHour4Sets;
            w->src = gCamSunProg0Colors;
            w->flags |= 4;
            break;
        case 1:
            gGameState.fade = 0;
            gGameState.fadeFrom.r = 0;
            gGameState.fadeFrom.g = 0;
            gGameState.fadeFrom.b = 0;
            if (w->dlFrame != NULL) {
                frmDestroy(w->dlFrame);
            }
            if (w->sun != NULL) {
                frmDestroy(w->sun);
            }
            if (w->ambient != NULL) {
                frmDestroy(w->ambient);
            }
            gCamSunObj = NULL;
            break;
        case 0:
            if (w->flags & 1) {
                break;
            }
            w->frames++;
            if (!(w->flags & 2)) {
                gGameWork.rate =
                    (24 << 16) / (gGameWork.daySeconds * 60 / gGameState.retracesPerFrame);
                gGameWork.time += gGameWork.rate;
                if (gGameWork.time >= (24 << 16)) {
                    gGameWork.time -= 24 << 16;
                }
            }
            if (w->state != NULL) {
                w->state(w);
            }
            if (w->watch != NULL) {
                w->watch(w);
            }
            if ((w->frames & 0xF) == 1) {
                for (i = 0; i < w->nPair; i++) {
                    if (sndGetVolume(w->pair[i].a, 0)) {
                        sndPlay(w->pair[i].b);
                    }
                }
            }
            if (w->view != NULL && w->sun != NULL) {
                dx = w->view->eye[0] - w->view->at[0];
                dy = w->view->eye[1] - w->view->at[1];
                dz = w->view->eye[2] - w->view->at[2];
                s = 127.999f / sqrtf(dx * dx + dy * dy + dz * dz);
                w->sun->dir[0] = dx * s;
                w->sun->dir[1] = dy * s;
                w->sun->dir[2] = dz * s;
            }
            if (w->flags & 4) {
                camSunApplySky(w);
            }
            break;
        case 3:
            w->dlFrame->dl = gGfxDlHead;
            r = BLEND(w, env, r);
            g = BLEND(w, env, g);
            b = BLEND(w, env, b);
            gDPSetEnvColor(gGfxDlHead++, r, g, b, 255);
            w->sun->color[0] = r;
            w->sun->color[1] = g;
            w->sun->color[2] = b;
            gDPPipeSync(gGfxDlHead++);
            gDPSetFogColor(gGfxDlHead++, BLEND(w, fog, r), BLEND(w, fog, g), BLEND(w, fog, b), 0);
            gSPFogPosition(gGfxDlHead++, w->fogNear, 1000);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The sky: the frame's gradient fill (top and bottom colours) blended by the level. */
void camSunApplySky(CamCtlWork *w) {
    gGameState.fadeFrom.r = BLEND(w, top, r);
    gGameState.fadeFrom.g = BLEND(w, top, g);
    gGameState.fadeFrom.b = BLEND(w, top, b);
    gGameState.fadeTo[0] = BLEND(w, bottom, r);
    gGameState.fadeTo[1] = BLEND(w, bottom, g);
    gGameState.fadeTo[2] = BLEND(w, bottom, b);
}

/* The light's four colours at a pair's [side], from its colour set n (src[4n] on). */
#define LIGHT_SET(w, side, n)                   \
    (w)->top[side] = (w)->src[(n) * 4];         \
    (w)->bottom[side] = (w)->src[(n) * 4 + 1];  \
    (w)->env[side] = (w)->src[(n) * 4 + 2];   \
    (w)->fog[side] = (w)->src[(n) * 4 + 3]

/* Sets the light's colours for the hour from its two sets (src: day, then night): day from
 * 7:00, dusk blending day to night from 16:00 to 20:00, night from 20:00, and dawn blending night
 * to day from 3:00 to 7:00. */
void camSunByHour2Sets(CamCtlWork *w) {
    s32 hour = gTimeOfDay / 0x10000;

    if (hour >= 7 && hour < 16) {
        w->level = 0;
        LIGHT_SET(w, 0, 0);
        return;
    }
    if (hour >= 16 && hour < 20) {
        w->level = (gTimeOfDay - (16 << 16)) / 4;
        LIGHT_SET(w, 0, 0);
        LIGHT_SET(w, 1, 1);
        return;
    }
    if (hour >= 20 || hour < 3) {
        w->level = 0;
        LIGHT_SET(w, 0, 1);
        return;
    }
    if (hour >= 3 && hour < 7) {
        w->level = (gTimeOfDay - (3 << 16)) / 4;
        LIGHT_SET(w, 0, 1);
        LIGHT_SET(w, 1, 0);
    }
}

/* As camSunByHour2Sets with four sets (src: dawn, day, evening, night): dawn to day from 5:00 to
 * 10:00, day to 16:00, day to evening to 18:00, evening to night to 20:00, night to 3:00, and
 * night to dawn to 5:00. */
void camSunByHour4Sets(CamCtlWork *w) {
    s32 hour = gTimeOfDay / 0x10000;

    if (hour >= 5 && hour < 10) {
        w->level = (gTimeOfDay - (5 << 16)) / 5;
        LIGHT_SET(w, 0, 0);
        LIGHT_SET(w, 1, 1);
        return;
    }
    if (hour >= 10 && hour < 16) {
        w->level = 0;
        LIGHT_SET(w, 0, 1);
        return;
    }
    if (hour >= 16 && hour < 18) {
        w->level = (gTimeOfDay - (16 << 16)) / 2;
        LIGHT_SET(w, 0, 1);
        LIGHT_SET(w, 1, 2);
        return;
    }
    if (hour >= 18 && hour < 20) {
        w->level = (gTimeOfDay - (18 << 16)) / 2;
        LIGHT_SET(w, 0, 2);
        LIGHT_SET(w, 1, 3);
        return;
    }
    if (hour >= 20 || hour < 3) {
        w->level = 0;
        LIGHT_SET(w, 0, 3);
        return;
    }
    if (hour >= 3 && hour < 5) {
        w->level = (gTimeOfDay - (3 << 16)) / 2;
        LIGHT_SET(w, 0, 3);
        LIGHT_SET(w, 1, 0);
    }
}

/* Takes its four colours from its source, the blend at its start. */
void camSunFixedSet(CamCtlWork *w) {
    w->level = 0;
    w->top[0] = w->src[0];
    w->bottom[0] = w->src[1];
    w->env[0] = w->src[2];
    w->fog[0] = w->src[3];
}

/* Stores the four blended colours in their pairs' [side]: 0 the blend's start, 1 its end. */
void camSunStoreBlend(CamCtlWork *w, s32 side) {
    w->top[side].r = BLEND(w, top, r);
    w->top[side].g = BLEND(w, top, g);
    w->top[side].b = BLEND(w, top, b);
    w->bottom[side].r = BLEND(w, bottom, r);
    w->bottom[side].g = BLEND(w, bottom, g);
    w->bottom[side].b = BLEND(w, bottom, b);
    w->env[side].r = BLEND(w, env, r);
    w->env[side].g = BLEND(w, env, g);
    w->env[side].b = BLEND(w, env, b);
    w->fog[side].r = BLEND(w, fog, r);
    w->fog[side].g = BLEND(w, fog, g);
    w->fog[side].b = BLEND(w, fog, b);
}

/* Runs the sky's next state as if `steps` of the clock had passed, toward the colours at src,
 * then puts the clock and the four colours back and the blend at its start. */
void camSunBeginBlend(CamCtlWork *w, s32 steps, Color3 *src) {
    Color3 c0;
    Color3 c1;
    Color3 c2;
    Color3 c3;

    camSunStoreBlend(w, FALSE);
    c0 = w->top[0];
    c1 = w->bottom[0];
    c2 = w->env[0];
    c3 = w->fog[0];
    w->start = gGameWork.time;
    w->end = steps * gGameWork.rate + gGameWork.time;
    if (w->end >= 24 << 16) {
        w->end -= 24 << 16;
    }
    gGameWork.time = w->end;
    w->src = src;
    w->next(w);
    gGameWork.time = w->start;
    camSunStoreBlend(w, TRUE);
    w->top[0] = c0;
    w->bottom[0] = c1;
    w->env[0] = c2;
    w->fog[0] = c3;
    w->level = 0;
}

/* The blend's state: the level runs 0-0x10000 as the time of day goes from start to end
 * (across midnight too), then the next state takes over. */
void camSunBlendState(CamCtlWork *w) {
    s32 now = gTimeOfDay;
    s32 span = w->end - w->start;
    s32 last = w->end;

    if (w->end < w->start) {
        now += 24 << 16;
        span += 24 << 16;
        last += 24 << 16;
    }
    if (now >= last) {
        w->state = w->next;
        return;
    }
    w->level = ((now - w->start) << 16) / span;
}

/* The light's watcher for an area with five tunes (camSunSetTunes4): each period starts the next
 * tune when the clock reaches its hour. Night to dawn at 2:30-4:00 (with no dawn tune, a
 * crossfade and the tune at +0x5E at 3:30-4:00), then day at 4:45, evening at 16:00, the fourth
 * at 18:00 and night at 18:30. The tune at +0x5C sounds once as the clock passes 4:45. */
void camSunTuneWatch(CamCtlWork *w) {
    s32 time;

    if (w->dawnSound != 0 && gGameWork.time >= 0x4C000 &&
        gGameWork.time < gGameWork.rate + 0x4C000) {
        sndPlay(w->dawnSound);
    }
    switch (w->period) {
        case 0:
            time = gTimeOfDay;
            if (w->tunes[0] != 0) {
                if (time >= 0x28000 && time < 0x40000) {
                    sndPlay(w->tunes[0]);
                    w->period = 1;
                }
            } else if (time >= 0x38000 && time < 0x40000) {
                sndCrossfade(0);
                sndPlay(w->altDawnTune);
                w->period = 1;
            }
            break;
        case 1:
            if (gGameWork.time >= 0x4C000 && gGameWork.time < 0x100000) {
                sndPlay(w->tunes[1]);
                w->period = 2;
            }
            break;
        case 2:
            if (gGameWork.time >= 0x100000 && gGameWork.time < 0x120000) {
                sndPlay(w->tunes[2]);
                w->period = 3;
            }
            break;
        case 3:
            if (w->tunes[3] != 0) {
                if (gGameWork.time >= 0x120000) {
                    sndPlay(w->tunes[3]);
                    w->period = 4;
                }
            } else {
                w->period = 4;
            }
            break;
        case 4:
            if (w->tunes[4] != 0) {
                if (gGameWork.time >= 0x128000) {
                    sndPlay(w->tunes[4]);
                    w->period = 0;
                }
            } else {
                w->period = 0;
            }
            break;
    }
}

/* Dawn and dusk: between 5:00 and 6:00 the music crossfades to its day version, between 18:00
 * and 19:00 back to its night one. */
void camSunDawnDuskWatch(CamCtlWork *w) {
    switch (w->period) {
        case FALSE:
            if (gTimeOfDay >= 5 << 16 && gTimeOfDay < 6 << 16) {
                sndCrossfade(1);
                w->period = TRUE;
                return;
            }
            return;
        case TRUE:
            if (gTimeOfDay >= 18 << 16 && gTimeOfDay < 19 << 16) {
                sndCrossfade(0);
                w->period = FALSE;
            }
            break;
    }
}

/* Copies its five values to the globals at 0x80088390; TRUE if none changed. */
s32 camSunCopyTunes(CamCtlWork *w) {
    s32 same = TRUE;
    s32 i;

    for (i = 0; i < 5; i++) {
        if (gGameWork.tunes[i] != w->tunes[i]) {
            same = FALSE;
        }
        gGameWork.tunes[i] = w->tunes[i];
    }
    return same;
}

void camSunPlayTune(void *unused, u16 id) {
    soundPlayAfterOne(id);
}

/* Creates gCamSunObj under a parent, once; returns it either way. */
Obj *camSunCreate(Obj *parent) {
    if (gCamSunObj != NULL) {
        return gCamSunObj;
    }
    return gCamSunObj = tskCreateBlocks(parent, Artsun, 2, 0, 0);
}

Obj *camSunDelete(void) {
    return tskDestroy(gCamSunObj);
}

/* Runs the light's state, then applies it; flag 1 marks it as run (or not). */
void camSunRefreshHold(s32 on) {
    CamCtlWork *w = (CamCtlWork *) (gCamSunObj + 1);

    if (gCamSunObj != NULL) {
        w->state(w);
        camSunApplySky(w);
        if (on == TRUE) {
            w->flags |= 1;
            return;
        }
        w->flags &= ~1;
    }
}

void camSunSetClockStop(s32 on) {
    CamCtlWork *w = (CamCtlWork *) (gCamSunObj + 1);

    if (gCamSunObj != NULL) {
        if (on == TRUE) {
            w->flags |= 2;
            return;
        }
        w->flags &= ~2;
    }
}

void camSunSetSkyOn(s32 on) {
    CamCtlWork *w = (CamCtlWork *) (gCamSunObj + 1);

    if (gCamSunObj != NULL) {
        if (on == TRUE) {
            w->flags |= 4;
            return;
        }
        w->flags &= ~4;
    }
}

LightFrame *camSunGetAmbient(void) {
    CamCtlWork *w = (CamCtlWork *) (gCamSunObj + 1);

    if (gCamSunObj == NULL) {
        return NULL;
    }
    return w->ambient;
}

void camSunSetColor(s32 r, s32 g, s32 b) {
    CamCtlWork *w = (CamCtlWork *) (gCamSunObj + 1);

    if (gCamSunObj != NULL) {
        w->env[0].r = r;
        w->env[0].g = g;
        w->env[0].b = b;
        w->env[1].r = r;
        w->env[1].g = g;
        w->env[1].b = b;
    }
}

void camSunSetFogColor(s32 r, s32 g, s32 b) {
    CamCtlWork *w = (CamCtlWork *) (gCamSunObj + 1);

    if (gCamSunObj != NULL) {
        w->fog[0].r = r;
        w->fog[0].g = g;
        w->fog[0].b = b;
        w->fog[1].r = r;
        w->fog[1].g = g;
        w->fog[1].b = b;
    }
}

void camSunSetFogNear(s32 v) {
    CamCtlWork *w = (CamCtlWork *) (gCamSunObj + 1);

    if (gCamSunObj != NULL) {
        w->fogNear = v;
    }
}

void camSunSetView(void *v) {
    CamCtlWork *w = (CamCtlWork *) (gCamSunObj + 1);

    if (gCamSunObj != NULL) {
        w->view = v;
    }
}

void camSunSetCamBG(void *v) {
    CamCtlWork *w = (CamCtlWork *) (gCamSunObj + 1);

    if (gCamSunObj != NULL) {
        w->camBg = v;
    }
}

/* 0x100, or gCamSunObj's level (/256) while it is in state camSunBlendState. */
s32 camSunBlendLevel(void) {
    CamCtlWork *w = (CamCtlWork *) (gCamSunObj + 1);

    if (gCamSunObj == NULL) {
        return 0x100;
    }
    if (w->state == camSunBlendState) {
        return w->level / 256;
    }
    return 0x100;
}

/* Sets the light's program for an area: fixed colours (8, 9), colours by the hour from two sets
 * (4) or four (5-7, 10-12), a blend over 60 steps into those (0) or into fixed ones (2), or none
 * (14). Then tells the view (cambgSetProgram). */
void camSunSetProgram(s32 kind) {
    CamCtlWork *w = (CamCtlWork *) (gCamSunObj + 1);

    if (gCamSunObj != NULL) {
        w->fogNear = 0x3E2;
        switch (kind) {
            case 14:
                w->state = NULL;
                break;
            case 0:
                w->state = camSunBlendState;
                w->next = camSunByHour4Sets;
                camSunBeginBlend(w, 60, gCamSunProg0Colors);
                break;
            case 2:
                w->state = camSunBlendState;
                w->next = camSunFixedSet;
                camSunBeginBlend(w, 60, gCamSunProg2Colors);
                break;
            case 8:
                w->state = camSunFixedSet;
                w->src = gCamSunProg8Colors;
                break;
            case 9:
                w->state = camSunFixedSet;
                w->src = gCamSunProg9Colors;
                break;
            case 4:
                w->state = camSunByHour2Sets;
                w->src = gCamSunProg4Colors;
                w->fogNear = 0x320;
                break;
            case 5:
                w->state = camSunByHour4Sets;
                w->src = gCamSunProg5Colors;
                break;
            case 6:
                w->state = camSunByHour4Sets;
                w->src = gCamSunProg6Colors;
                w->fogNear = 0x384;
                break;
            case 7:
                w->state = camSunByHour4Sets;
                w->src = gCamSunProg7Colors;
                w->fogNear = 0x35C;
                break;
            case 10:
                w->state = camSunByHour4Sets;
                w->src = gCamSunProg10Colors;
                w->fogNear = 0x3E4;
                break;
            case 11:
                w->state = camSunByHour4Sets;
                w->src = gCamSunProg11Colors;
                break;
            case 12:
                w->state = camSunByHour4Sets;
                w->src = gCamSunProg12Colors;
                break;
        }
        if (w->camBg != NULL) {
            cambgSetProgram(w->camBg, kind);
        }
    }
}

void camSunShowLights(s32 on) {
    CamCtlWork *w = (CamCtlWork *) (gCamSunObj + 1);

    if (gCamSunObj != NULL) {
        if (on == TRUE) {
            if (w->sun != NULL) {
                w->sun->f.flags |= 2;
            }
            if (w->ambient != NULL) {
                w->ambient->f.flags |= 2;
            }
        } else {
            if (w->sun != NULL) {
                w->sun->f.flags &= ~2;
            }
            if (w->ambient != NULL) {
                w->ambient->f.flags &= ~2;
            }
        }
    }
}

void camSunCallSetTunes4(u16 a, u16 b, u16 c, u16 d) {
    camSunSetTunes4(a, b, c, d);
}

/* No music watcher, and the tunes cleared (here and in gGameWork). */
void camSunClearTunes(void) {
    CamCtlWork *w = (CamCtlWork *) (gCamSunObj + 1);
    s32 i;

    if (gCamSunObj != NULL) {
        w->watch = NULL;
        for (i = 0; i < 5; i++) {
            gGameWork.tunes[i] = w->tunes[i] = 0;
        }
    }
}

/* Four tunes: dawn (2:30-4:00), day (to 16:00), evening (to 18:00) and night. The one for the
 * time starts unless they are the tunes already set; camSunTuneWatch switches them as the day
 * goes. (When the tunes are unchanged the game counts dawn to 4:45 and evening to 18:30, and
 * calls evening 4.) */
void camSunSetTunes4(u16 dawn, u16 day, u16 evening, u16 night) {
    CamCtlWork *w = (CamCtlWork *) (gCamSunObj + 1);

    if (gCamSunObj != NULL) {
        w->watch = camSunTuneWatch;
        w->tunes[0] = dawn;
        w->tunes[1] = day;
        w->tunes[2] = evening;
        w->tunes[3] = 0;
        w->tunes[4] = night;
        if (camSunCopyTunes(w) == TRUE) {
            if (gTimeOfDay >= 0x28000 && gTimeOfDay < 0x4C000) {
                w->period = 1;
                return;
            }
            if (gTimeOfDay >= 0x4C000 && gTimeOfDay < 0x100000) {
                w->period = 2;
                return;
            }
            if (gTimeOfDay >= 0x100000 && gTimeOfDay < 0x128000) {
                w->period = 4;
                return;
            }
            w->period = 0;
        } else {
            if (gTimeOfDay >= 0x28000 && gTimeOfDay < 0x40000) {
                camSunPlayTune(w, w->tunes[0]);
                w->period = 1;
                return;
            }
            if (gTimeOfDay >= 0x40000 && gTimeOfDay < 0x100000) {
                camSunPlayTune(w, w->tunes[1]);
                w->period = 2;
                return;
            }
            if (gTimeOfDay >= 0x100000 && gTimeOfDay < 0x120000) {
                camSunPlayTune(w, w->tunes[2]);
                w->period = 3;
                return;
            }
            if (gTimeOfDay >= 0x120000 || gTimeOfDay < 0x28000) {
                camSunPlayTune(w, w->tunes[4]);
                w->period = 0;
            }
        }
    }
}

/* The same with a tune kept apart (unk5E) and the night tune not stored: day, evening and night
 * play, from 3:30 (4:45 when the tunes are unchanged). */
void camSunSetTunes5(u16 other, u16 day, u16 evening, u16 third, u16 night) {
    CamCtlWork *w = (CamCtlWork *) (gCamSunObj + 1);

    if (gCamSunObj != NULL) {
        w->watch = camSunTuneWatch;
        w->altDawnTune = other;
        w->tunes[0] = 0;
        w->tunes[1] = day;
        w->tunes[2] = evening;
        w->tunes[3] = third;
        w->tunes[4] = 0;
        if (camSunCopyTunes(w) == TRUE) {
            if (gTimeOfDay >= 0x38000 && gTimeOfDay < 0x4C000) {
                w->period = 1;
                return;
            }
            if (gTimeOfDay >= 0x4C000 && gTimeOfDay < 0x100000) {
                w->period = 2;
                return;
            }
            if (gTimeOfDay >= 0x100000 && gTimeOfDay < 0x120000) {
                w->period = 3;
                return;
            }
            w->period = 0;
        } else {
            if (gTimeOfDay >= 0x38000 && gTimeOfDay < 0x100000) {
                camSunPlayTune(w, w->tunes[1]);
                w->period = 2;
                return;
            }
            if (gTimeOfDay >= 0x100000 && gTimeOfDay < 0x120000) {
                camSunPlayTune(w, w->tunes[2]);
                w->period = 3;
                return;
            }
            if (gTimeOfDay >= 0x120000 || gTimeOfDay < 0x40000) {
                camSunPlayTune(w, night);
                w->period = 0;
            }
        }
    }
}

/* The day and night music: sets the two tunes and starts the one for the time of day (5:00 to
 * 18:00 is day), unless they are the tunes already set; camSunDawnDuskWatch then switches at dawn and
 * dusk. */
void camSunSetTunes2(u16 day, u16 night) {
    CamCtlWork *w = (CamCtlWork *) (gCamSunObj + 1);

    if (gCamSunObj != NULL) {
        w->watch = camSunDawnDuskWatch;
        w->tunes[0] = 0;
        w->tunes[1] = day;
        w->tunes[2] = 0;
        w->tunes[3] = 0;
        w->tunes[4] = night;
        if (camSunCopyTunes(w) == TRUE) {
            if (gTimeOfDay >= 5 << 16 && gTimeOfDay < 18 << 16) {
                w->period = TRUE;
                return;
            }
            w->period = FALSE;
        } else {
            if (gTimeOfDay >= 5 << 16 && gTimeOfDay < 18 << 16) {
                camSunPlayTune(w, day);
                w->period = TRUE;
                return;
            }
            camSunPlayTune(w, night);
            w->period = FALSE;
        }
    }
}

void camSunSetDawnSound(u16 v) {
    CamCtlWork *w = (CamCtlWork *) (gCamSunObj + 1);

    if (gCamSunObj != NULL) {
        w->dawnSound = v;
    }
}

void camSunAddSoundPair(u16 a, u16 b) {
    CamCtlWork *w = (CamCtlWork *) (gCamSunObj + 1);

    if (gCamSunObj != NULL) {
        w->pair[w->nPair].a = a;
        w->pair[w->nPair].b = b;
        w->nPair++;
    }
}
