#include "object.h"
#include "game.h"
#include "contpak.h"
#include "fade.h"
#include "file.h"
#include "gfxmgr.h"
#include "heap.h"
#include "joypad.h"
#include "mathutil.h"
#include "tmanager.h"

/* The screen fade, "Fade" (gFadeTask): a level (0 clear) moving toward a target, in a colour,
 * and a letterbox: the picture between two lines, top and bottom. */

typedef struct FadeWork {
    /* 0x00 */ void *frame;
    /* 0x04 */ f32 speed; /* frames to the target */
    /* 0x08 */ f32 level;
    /* 0x0C */ f32 target;
    /* 0x10 */ f32 step;
    /* 0x14 */ u8 r;
    /* 0x15 */ u8 g;
    /* 0x16 */ u8 b;
    /* 0x18 */ s32 width; /* 320 */
    /* 0x1C */ s32 top; /* the letterbox's lines */
    /* 0x20 */ s32 bottomLeft;
    /* 0x24 */ s32 bottom;
} FadeWork;

/* fade.c's .data (0x80107940-0x80107944), in ROM order. */
Obj *gFadeTask = NULL;
extern s32 gRandSeed; /* the random seed */

extern Vp gCamDefaultVp; /* the default viewport */
extern struct { /* an Obj (object.h), as far as this file reads it */
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ Obj *child; /* the first */
} *gEffectTask;

/* The save file on the Controller Pak (src/contpak.c): three slots of 0x200 bytes, each holding
 * the block at 0x80088208, and a header of 0x100 bytes holding the 16-byte settings at
 * 0x800881F8. */
typedef struct Settings {
    /* 0x00 */ s32 crcHi; /* the header's first u64 is its CRC over 0x10 bytes (contpak.c): 0 */
    /* 0x04 */ s32 crc;   /* and its low word */
    /* 0x08 */ s8 language; /* 0 English, 1 French, 2 German (tlb_0018_3) */
    /* 0x09 */ s8 soundMode; /* 0 sends sound command 5, 1 command 6 (tlb_0018_4) */
    /* 0x0A */ u8 pad0A[2];
    /* 0x0C */ s32 unused0C;
} Settings;

typedef union SaveBuf {
    Unk88208 save;
    u8 raw[0x200];
} SaveBuf;

typedef union HeaderBuf {
    Settings settings;
    u8 raw[0x100];
} HeaderBuf;

extern Settings gSaveSettings;


extern void *gFrmLayers[]; /* the frame pools */
extern s16 gFrmKindDlist; /* a display list's frame kind */
extern Gfx *gGfxDlHead; /* the display list being built */

#define FADE() ((FadeWork *) ((u32) gFadeTask + sizeof(Obj)))

/* The fade, "Fade". Message 2 takes a display-list frame and opens the letterbox (0-240); 1
 * frees it; 0 moves the level toward the target (0-255); 3 draws: the whole screen in the
 * colour with the level as its alpha (from 1), the letterbox's black bars, and a clear line
 * along each bar's edge. */
s32 fadeProc(Obj *obj, s16 msg, void *arg) {
    FadeWork *w = (FadeWork *) ((u32) obj + sizeof(Obj));
    f32 d;

    switch (msg) {
        case 2:
            obj->name = "Fade";
            w->frame = frmCreate(gFrmLayers[10], gFrmKindDlist);
            w->width = 320;
            w->top = 0;
            w->bottomLeft = 0;
            w->bottom = 240;
            break;
        case 1:
            frmDestroy(w->frame);
            gFadeTask = NULL;
            break;
        case 0:
            if (w->step > 0.0001) {
                d = (w->level < w->target) ? w->target - w->level : -(w->target - w->level);
                if (d < w->step) {
                    w->level = w->target;
                    w->step = 0;
                } else if (w->target < w->level) {
                    w->level -= w->step;
                } else {
                    w->level += w->step;
                }
            }
            if (w->level > 255.0f) {
                w->level = 255.0f;
            } else if (w->level < 0.0f) {
                w->level = 0.0f;
            }
            break;
        case 3:
            ((DLFrame *) w->frame)->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCycleType(gGfxDlHead++, G_CYC_1CYCLE);
            gDPSetRenderMode(gGfxDlHead++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
            gDPSetCombineMode(gGfxDlHead++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
            gDPSetAlphaCompare(gGfxDlHead++, G_AC_NONE);
            if (w->level >= 1.0) {
                gDPSetPrimColor(gGfxDlHead++, 0, 0, w->r, w->g, w->b, (s32) w->level);
                gDPFillRectangle(gGfxDlHead++, 0, 0, 320, 240);
            }
            gDPSetRenderMode(gGfxDlHead++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 0, 0, 0, 255);
            gDPFillRectangle(gGfxDlHead++, 0, 0, w->width, w->top);
            gDPFillRectangle(gGfxDlHead++, w->bottomLeft, w->bottom, 320, 240);
            gDPSetRenderMode(gGfxDlHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 0, 0, 0, 0);
            gDPFillRectangle(gGfxDlHead++, 0, w->top, 320, w->top + 1);
            gDPFillRectangle(gGfxDlHead++, 0, w->bottom - 1, 320, w->bottom);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Creates the fade under a parent, once. */
Obj *fadeCreate(Obj *parent) {
    if (gFadeTask != NULL) {
        return gFadeTask;
    }
    return gFadeTask = tskCreate(parent, fadeProc, 0, 0);
}

void fadeSetSpeed(f32 speed) {
    FadeWork *w = FADE();

    if (gFadeTask != NULL) {
        w->speed = speed;
    }
}

void fadeSetLevel(f32 level) {
    FadeWork *w = FADE();

    if (gFadeTask != NULL) {
        w->level = level;
    }
}

void fadeSetColor(u8 r, u8 g, u8 b) {
    FadeWork *w = FADE();

    if (gFadeTask != NULL) {
        w->r = r;
        w->g = g;
        w->b = b;
    }
}

/* The level toward a target, a little faster than over speed frames. */
void fadeSetTarget(f32 target) {
    FadeWork *w = FADE();

    if (gFadeTask != NULL) {
        w->target = target;
        w->step = fabsf(target - w->level) / w->speed * 1.1f;
    }
}

void fadeSetLetterbox(s32 width, s32 top, s32 bottomLeft, s32 bottom) {
    FadeWork *w = FADE();

    if (gFadeTask != NULL) {
        w->width = width;
        w->top = top;
        w->bottomLeft = bottomLeft;
        w->bottom = bottom;
    }
}

/* Closes the letterbox a line at a time to line n (120 closes it at 124). 1 when there. */
s32 fadeCloseLetterbox(s32 n) {
    FadeWork *w = FADE();

    if (gFadeTask == NULL) {
        return 2;
    }
    if (n == 120) {
        n = 124;
    }
    w->width = 320;
    w->top++;
    w->bottomLeft = 0;
    w->bottom--;
    if (w->top >= n || 240 - n >= w->bottom) {
        w->top = n;
        w->bottom = 240 - n;
        return 1;
    }
    return 0;
}

/* Opens the letterbox a line at a time. 1 when open. */
s32 fadeOpenLetterbox(void) {
    FadeWork *w = FADE();

    if (gFadeTask == NULL) {
        return 2;
    }
    w->width = 320;
    w->top--;
    w->bottomLeft = 0;
    w->bottom++;
    if (w->top < 0 || w->bottom > 240) {
        w->top = 0;
        w->bottom = 240;
        return 1;
    }
    return 0;
}

/* Moves the letterbox's lines toward top and bottom by speed. 1 when both are there. */
s32 fadeMoveLetterbox(s32 top, s32 bottom, s32 speed) {
    FadeWork *w = FADE();
    s32 done = 0;

    if (gFadeTask == NULL) {
        return 2;
    }
    w->width = 320;
    w->bottomLeft = 0;
    if (top == w->top) {
        done = 1;
    } else if (w->top >= top) {
        w->top -= speed;
        if (top >= w->top) {
            w->top = top;
        }
    } else {
        w->top += speed;
        if (w->top >= top) {
            w->top = top;
        }
    }
    if (bottom == w->bottom) {
        done++;
    } else if (w->bottom >= bottom) {
        w->bottom -= speed;
        if (bottom >= w->bottom) {
            w->bottom = bottom;
        }
    } else {
        w->bottom += speed;
        if (w->bottom >= bottom) {
            w->bottom = bottom;
        }
    }
    if (done == 2) {
        return 1;
    }
    return 0;
}

void fadeSetParent(s32 arg) {
    FadeWork *w = FADE();

    if (gFadeTask != NULL) {
        frmMove(w->frame, arg);
    }
}

void fadeSetAll(f32 speed, f32 level, f32 target, u8 r, u8 g, u8 b) {
    fadeSetSpeed(speed);
    fadeSetLevel(level);
    fadeSetColor(r, g, b);
    fadeSetTarget(target);
}

void fadeGetColor(u8 *r, u8 *g, u8 *b) {
    FadeWork *w = FADE();

    *r = w->r;
    *g = w->g;
    *b = w->b;
}

/* A random number below n: the seed steps as seed * 23 + 2345 (mod 4535). */
s32 fadeRand(s32 n) {
    f32 r;

    gRandSeed = (gRandSeed * 23 + 2345) % 4535;
    r = gRandSeed * (1.0 / 4536) * n;
    return r;
}

/* A random number between lo and hi. */
f32 fadeRandRange(s32 lo, s32 hi) {
    s32 d = hi - lo;

    return fadeRand((d > 0) ? d : -d) + lo;
}

/* A segmented address (its low 24 bits an offset) in a file, in RAM. */
u32 fadeSegToRam(u32 addr, u32 file) {
    return fileRamAddr(file) + (addr & 0xFFFFFF);
}

/* Starts an animation at frame index: *out gets each frame's address, in file. */
void fadeAnimStart(Anim *a, AnimFrame *frames, s32 index, u32 *out, u32 file) {
    a->tick = 0;
    a->index = index;
    a->frames = frames;
    a->out = out;
    a->file = file;
    *a->out = fadeSegToRam(frames[index].addr, file);
}

/* Steps an animation a tick. 1 when it has wrapped to its first frame. */
s32 fadeAnimStep(Anim *a) {
    AnimFrame *frames = a->frames;

    if (frames == NULL) {
        return 0;
    }
    a->tick++;
    if (a->tick >= frames[a->index].duration) {
        a->index++;
        a->tick = 0;
        if (frames[a->index].addr != 0) {
            *a->out = fadeSegToRam(frames[a->index].addr, a->file);
        } else {
            a->index = 0;
            *a->out = fadeSegToRam(frames[0].addr, a->file);
            return 1;
        }
    }
    return 0;
}

/* An animation's frame at time t (looping). */
u32 fadeAnimFrameAt(Anim *a, s32 t) {
    AnimFrame *frames = a->frames;
    s32 total;
    s32 i;

    if (frames == NULL) {
        return 0;
    }
    total = 0;
    for (i = 0; frames[i].addr != 0; i++) {
        total += frames[i].duration;
    }
    t = t % total;
    t = t - frames[0].duration;
    i = 0;
    while (t >= 0) {
        i++;
        t -= frames[i].duration;
    }
    return fadeSegToRam(frames[i].addr, a->file);
}

/* A light's direction scaled to length 128. */
void fadeLightNormalize(LightFrame *f) {
    f32 x = f->dir[0];
    f32 y = f->dir[1];
    f32 z = f->dir[2];
    f32 s = 127.999f / sqrtf(x * x + y * y + z * z);

    f->dir[0] = x * s;
    f->dir[1] = y * s;
    f->dir[2] = z * s;
}

/* How wide a view sees at the look-at point: twice the distance times the tangent of half its
 * field of view (30000 without a view). */
f32 fadeViewWidth(CamView *v) {
    f32 s;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 c;

    if (v == NULL) {
        return 30000.0f;
    }
    s = mathSinCos((s32) (v->fovy / 2 / PI * 512.0f), &c);
    dx = v->at[0] - v->eye[0];
    dy = v->at[1] - v->eye[1];
    dz = v->at[2] - v->eye[2];
    return sqrtf(dx * dx + dy * dy + dz * dz) * (s / c) * 2;
}

/* How many of the four ports are in play. */
s32 fadeCountPlayers(void) {
    s32 n = 0;
    s32 i;

    for (i = 0; i < 4; i++) {
        if (gSaveData.players[i].chara != -1 && gSaveData.players[i].lives > 0) {
            n++;
        }
    }
    return n;
}

/* Whether a player in play presses Start (never while the fade is above 3). */
s32 fadeStartPressed(void) {
    FadeWork *w = FADE();
    s32 i;

    if (gFadeTask != NULL && w->level > 3.0) {
        return 0;
    }
    for (i = 0; i < 4; i++) {
        if (gSaveData.players[i].chara != -1 && gSaveData.players[i].lives > 0 &&
            (joyGetButton(i, 0) & START_BUTTON)) {
            return 1;
        }
    }
    return 0;
}

/* Rumbles every player in play (a pattern, one bit a frame, for a time). */
void fadeRumbleAll(u32 pattern, s32 time) {
    s32 i;

    for (i = 0; i < 4; i++) {
        if (gSaveData.players[i].chara != -1 && gSaveData.players[i].lives > 0) {
            contPakRumbleStart(i, pattern, time);
        }
    }
}

/* v in the frame of direction d (d along -z). 0 when d is vertical (then only turned). */
s32 fadeToViewSpace(Vec3f *v, Vec3f *out, Vec3f *d) {
    f32 r[3];
    f32 h;

    h = sqrtf(d->z * d->z + d->x * d->x);
    if (fabsf(h) < 0.00001f) {
        out->x = v->x;
        if (d->y > 0.0f) {
            out->y = -v->z;
            out->z = v->y;
        } else {
            out->y = v->z;
            out->z = -v->y;
        }
        return 0;
    }
    r[0] = (v->x * d->z - d->x * v->z) / h;
    r[1] = v->y * h + -(v->x * d->x + v->z * d->z) * d->y / h;
    r[2] = d->z * v->z + (v->x * d->x + v->y * d->y);
    out->x = r[0];
    out->y = r[1];
    out->z = r[2];
    return 1;
}

/* A point in a view's picture (-1 to 1 across its field of view); returns its depth scale,
 * negative behind the eye. The picture point is an array, not a Vec3f: with `out.z` IDO
 * copied the scale out of f2 into f12 for the divisions and returns. */
f32 fadeProjectView(CamView *view, f32 x, f32 y, f32 z, f32 *sx, f32 *sy) {
    Vec3f d;
    Vec3f p;
    f32 out[3];
    f32 f;

    d.x = view->eye[0] - view->at[0];
    d.y = view->eye[1] - view->at[1];
    d.z = view->eye[2] - view->at[2];
    p.x = x - view->eye[0];
    p.y = y - view->eye[1];
    p.z = z - view->eye[2];
    mathVecNormalize(&d);
    fadeToViewSpace(&p, (Vec3f *) out, &d);
    f = -out[2] * mathAtan(view->fovy / 2);
    if (f < 0.0f) {
        return f;
    }
    *sx = out[0] / f;
    *sy = out[1] / f;
    return f;
}

/* A point on the screen, through the camera's viewport (in quarter pixels, as a Vp holds). */
f32 fadeProjectScreen(f32 x, f32 y, f32 z, f32 *sx, f32 *sy) {
    Vp *vp = &gCamDefaultVp;
    CamView *view = gGameWork.view;
    f32 f;

    if (view == NULL) {
        return 0.0f;
    }
    f = fadeProjectView(view, x, y, z, sx, sy);
    if (f < 0.0f) {
        return f;
    }
    if (view->vp != NULL) {
        vp = view->vp;
    }
    *sx = (*sx * vp->vp.vscale[0] * 0.75f + vp->vp.vtrans[0]) * 0.25;
    *sy = (-*sy * vp->vp.vscale[1] + vp->vp.vtrans[1]) * 0.25;
    return f;
}

/* A view's projection and look-at matrix: its field of view (radians) at 4:3, near and far, and
 * its roll (+0x2C) turning the up vector. */
void fadeViewMatrix(f32 out[4][4], CamView *view) {
    u16 perspNorm;
    f32 lookat[4][4];
    f32 persp[4][4];

    guPerspectiveF(persp, &perspNorm, view->fovy / (PI * 2) * 360.0f, 4.0f / 3.0f, view->zNear,
                   view->zFar, 1.0f);
    guLookAtF(lookat, view->eye[0], view->eye[1], view->eye[2], view->at[0], view->at[1],
              view->at[2], sinf(view->roll), cosf(view->roll), 0.0f);
    guMtxCatF(lookat, persp, out);
}

/* Creates an object, freeing the one at gEffectTask's +0xC and trying again, up to prio more
 * times. */
Obj *fadeCreateRetry(Obj *parent, s32 (*handler)(Obj *, s16, void *), s32 prio, s32 a,
                            void *arg) {
    s32 i;
    Obj *obj;

    for (i = 0; i < prio + 1; i++) {
        if (i != 0 && gEffectTask != NULL && gEffectTask->child != NULL) {
            tskDestroy(gEffectTask->child);
        }
        obj = tskCreateBlocks(parent, handler, prio, a, arg);
        if (obj != NULL) {
            return obj;
        }
    }
    return NULL;
}

/* A new game's save: everything cleared, four players at 3, 3 and 100, the first in play, and
 * the first stage can be left. */
void fadeNewSave(Unk88208 *save) {
    s32 i;

    heapMemClear(save, sizeof(Unk88208));
    save->charaCount = 2;
    for (i = 0; i < 4; i++) {
        save->players[i].chara = -1;
        save->players[i].lives = 3;
        save->players[i].life = 3;
        save->players[i].money = 100;
    }
    save->players[0].chara = 0;
    save->areaFlags[0] |= 1;
    save->stageFlags[0] |= 1;
}

/* Probes the Controller Pak on port 1 and, if it answers, sets it up. 0 when done. */
s32 fadePakInit(void) {
    s32 ret;

    ret = (*contPakProbe)(0);
    if (ret != 0) {
        return ret;
    }
    return (*contPakCreateFile)(0);
}

/* Reads save slot 0-2 into save. 0 when done. */
s32 fadeReadSave(Unk88208 *save, s32 slot) {
    SaveBuf buf;
    s32 ret;

    if (slot < 0 || slot >= 3) {
        return 1;
    }
    ret = contPakProbe(0);
    if (ret != 0) {
        return ret;
    }
    ret = contPakReadSlot(buf.raw, slot, 0);
    if (ret != 0) {
        return ret;
    }
    *save = buf.save;
    return ret;
}

/* Writes save to slot 0-2, each player's values raised to their least (3, 3 and 100). */
s32 fadeWriteSave(Unk88208 *save, s32 slot) {
    SaveBuf buf;
    s32 ret;
    s32 i;

    if (slot < 0 || slot >= 3) {
        return 1;
    }
    buf.save = *save;
    for (i = 0; i < 4; i++) {
        buf.save.players[i].life = 3;
        if (buf.save.players[i].lives < 3) {
            buf.save.players[i].lives = 3;
        }
        if (buf.save.players[i].money < 100) {
            buf.save.players[i].money = 100;
        }
    }
    ret = contPakProbe(0);
    if (ret != 0) {
        return ret;
    }
    return contPakWriteSlot(buf.raw, slot, 0);
}

/* Clears the settings. */
void fadeClearSettings(void) {
    heapMemClear(&gSaveSettings, sizeof(Settings));
}

/* Reads the settings from the save file's header. 0 when done. */
s32 fadeReadSettings(void) {
    HeaderBuf buf;
    s32 ret;

    ret = contPakProbe(0);
    if (ret != 0) {
        return ret;
    }
    ret = contPakReadHeader(buf.raw, 0);
    if (ret != 0) {
        return ret;
    }
    gSaveSettings = buf.settings;
    return ret;
}

/* Writes the settings to the save file's header. 0 when done. */
s32 fadeWriteSettings(void) {
    HeaderBuf buf;
    s32 ret;

    buf.settings = gSaveSettings;
    ret = contPakProbe(0);
    if (ret != 0) {
        return ret;
    }
    return contPakWriteHeader(buf.raw, 0);
}
