#include "object.h"
#include "cambg.h"
#include "camera.h"
#include "fade.h"
#include "file.h"
#include "fileres.h"
#include "gfxmgr.h"
#include "heap.h"
#include "mathutil.h"
#include "tmanager.h"

/* The "camBG" task: a background picture behind the stage (a frame of kind gFrmKindBg).
 * - It scrolls with the camera: sideways by how far the eye moves across the view and by the
 *   turn of its heading, up and down by the eye's height and the turn of its pitch.
 * - Its colours follow the time of day. The picture has four palettes (dawn, day, dusk, night),
 *   and the frame draws a blend of two of them.
 * - A change of program blends from the current colours to the new ones over 60 steps of the
 *   clock, through two palettes made on the heap.
 * A background is entry n of file 0xF, 32 bytes each. The frame is object.h's BGFrame. */

/* An entry of file 0xF. */
typedef struct BGEntry {
    /* 0x00 */ u16 file; /* the file holding the picture */
    /* 0x02 */ u16 tileCount;
    /* 0x04 */ u16 w;
    /* 0x06 */ u16 h;
    /* 0x08 */ u32 img1; /* addresses in the file */
    /* 0x0C */ u32 img2;
    /* 0x10 */ u32 pal[4];
} BGEntry; /* size 0x20 */

extern Frame *gFrmLayers[]; /* the frame pools */
extern s16 gFrmKindBg; /* the background kind */


#define CAMBG(obj) ((CamBGWork *) ((obj) + 1))

/* The handler. Each step it sets the colours and scrolls the picture with the camera. */
s32 camBG(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    CamBGWork *w;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 nx;
    f32 t;
    f32 px;
    f32 pz;
    f32 d;
    f32 a;
    f32 da;
    s32 pad2;

    w = CAMBG(obj);
    switch (msg) {
        case 2:
            obj->name = "camBG";
            w->frame = (BGFrame *) frmCreate(gFrmLayers[0], gFrmKindBg);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->wrapX = 1;
            w->frame->wrapY = 0;
            w->view = gGameWork.view;
            if (w->view == NULL) {
                PRINTF("gw.camera が存在しないため cambg が作れません\n");
                return -1;
            }
            w->eye = *(Vec3f *) w->view->eye;
            dx = w->view->at[0] - w->view->eye[0];
            dy = w->view->at[1] - w->view->eye[1];
            dz = w->view->at[2] - w->view->eye[2];
            w->yaw = mathAtan2(dx, dz);
            w->pitch = mathAtan2(dy, sqrtf(dx * dx + dz * dz));
            w->proc = cambgPalByHour;
            camSunSetCamBG(obj);
            w->flags |= 3;
            break;
        case 1:
            if (w->buf != NULL) {
                w->buf = heapFree(w->buf);
            }
            frmDestroy((Frame *) w->frame);
            break;
        case 0:
            w->proc(w);
            dx = w->view->at[0] - w->view->eye[0];
            dy = w->view->at[1] - w->view->eye[1];
            dz = w->view->at[2] - w->view->eye[2];
            if (w->flags & 1) {
                nx = -dx;
                t = (-(dx * (w->view->eye[2] - w->eye.z)) + (w->view->eye[0] - w->eye.x) * dz) *
                    (1.0f / (dz * dz - dx * nx));
                px = dz * t;
                pz = nx * t;
                d = sqrtf(px * px + pz * pz) * w->scale;
                if (t < 0.0f) {
                    w->frame->x += d;
                } else {
                    w->frame->x -= d;
                }
                a = mathAtan2(dx, dz);
                da = a - w->yaw;
                if (da < -3.1415927f) {
                    da += 6.2831855f;
                } else if (da > 3.1415927f) {
                    da -= 6.2831855f;
                }
                w->frame->x -= da * 200.0f;
                w->yaw = a;
            }
            if (w->flags & 2) {
                w->frame->y -= (w->view->eye[1] - w->eye.y) * w->scale;
                a = mathAtan2(dy, sqrtf(dx * dx + dz * dz));
                da = a - w->pitch;
                if (da < -3.1415927f) {
                    da += 6.2831855f;
                } else if (da > 3.1415927f) {
                    da -= 6.2831855f;
                }
                w->frame->y -= da * 200.0f;
                w->pitch = a;
            }
            w->eye = *(Vec3f *) w->view->eye;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The colours by the hour: dawn 3-5 (from night), morning 5-10 (to day), day 10-16, evening
 * 16-18 (to dusk) and 18-20 (to night), night 20-3. */
void cambgPalByHour(CamBGWork *w) {
    s32 hour = gGameWork.time / 0x10000;

    if (hour >= 5 && hour < 10) {
        w->frame->palA = w->pal[0];
        w->frame->palB = w->pal[1];
        w->frame->blend = (gGameWork.time - (5 << 16)) * 33 / (5 << 16);
    } else if (hour >= 10 && hour < 16) {
        w->frame->palA = w->pal[1];
        w->frame->palB = w->pal[1];
        w->frame->blend = 0;
    } else if (hour >= 16 && hour < 18) {
        w->frame->palA = w->pal[1];
        w->frame->palB = w->pal[2];
        w->frame->blend = (gGameWork.time - (16 << 16)) * 33 / (2 << 16);
    } else if (hour >= 18 && hour < 20) {
        w->frame->palA = w->pal[2];
        w->frame->palB = w->pal[3];
        w->frame->blend = (gGameWork.time - (18 << 16)) * 33 / (2 << 16);
    } else if (hour >= 20 || hour < 3) {
        w->frame->palA = w->pal[3];
        w->frame->palB = w->pal[3];
        w->frame->blend = 0;
    } else if (hour >= 3 && hour < 5) {
        w->frame->palA = w->pal[3];
        w->frame->palB = w->pal[0];
        w->frame->blend = (gGameWork.time - (3 << 16)) * 33 / (2 << 16);
    }
}

/* Night's colours, always. */
void cambgPalNight(CamBGWork *w) {
    w->frame->palA = w->pal[3];
    w->frame->palB = w->pal[3];
    w->frame->blend = 0;
}

/* Makes the frame's blend into palette `buf` (0 or 1) of the work's pair. Red is masked with
 * 0xF100, not 0xF800, so its lowest bit is lost.
 * The entries are read into s32 locals: "a & 1" on a local stays last in the OR, where
 * "palA[i] & 1" (a load) is put first. r's dead "= 0" makes it appear before b and a, so it
 * takes v0 as in the game. */
void cambgBakeBlend(CamBGWork *w, s32 buf) {
    u16 *palA = w->frame->palA;
    u16 *palB = w->frame->palB;
    s32 i;
    s32 a;
    s32 b;
    s32 r = 0;
    s32 g;
    s32 bl;

    for (i = 0; i < 256; i++) {
        b = PAL_GET(palB, i);
        a = PAL_GET(palA, i);
        r = (((b & 0xF100) >> 11) - ((a & 0xF100) >> 11)) * w->frame->blend / 32 + ((a & 0xF100) >> 11);
        g = (((b & 0x7C0) >> 6) - ((a & 0x7C0) >> 6)) * w->frame->blend / 32 + ((a & 0x7C0) >> 6);
        bl = (((b & 0x3E) >> 1) - ((a & 0x3E) >> 1)) * w->frame->blend / 32 + ((a & 0x3E) >> 1);
        PAL_SET(w->buf, (buf << 8) + i, r << 11 | g << 6 | bl << 1 | (a & 1));
    }
}

/* Starts a blend into w->target's colours over `steps` steps of the clock: palette 0 is now's
 * colours, palette 1 the target's at the end time. */
void cambgStartBlend(CamBGWork *w, s32 steps) {
    if (w->buf != NULL || (w->buf = heapMalloc(0x400)) != NULL) {
        cambgBakeBlend(w, 0);
        w->from = gGameWork.time;
        w->to = steps * gGameWork.rate + gGameWork.time;
        gGameWork.time = w->to;
        w->target(w);
        gGameWork.time = w->from;
        cambgBakeBlend(w, 1);
    }
}

/* A blend's step; at its end the palettes are freed and the target takes over. */
void cambgBlendStep(CamBGWork *w) {
    if (gGameWork.time >= w->to || w->buf == NULL) {
        if (w->buf != NULL) {
            w->buf = heapFree(w->buf);
        }
        w->proc = w->target;
        w->proc(w);
        return;
    }
    w->frame->palA = w->buf;
    w->frame->palB = w->buf + 256;
    w->frame->blend = (gGameWork.time - w->from) * 33 / (w->to - w->from);
}

/* Makes a camBG task under parent with background n at a distance (scrolling 100 / dist). */
Obj *cambgCreate(Obj *parent, s32 n, f32 dist) {
    Obj *obj = tskCreate(parent, camBG, 0, 0);

    if (obj != NULL) {
        CAMBG(obj)->scale = 100.0f / dist;
        cambgLoad(obj, n);
    }
    return obj;
}

/* Moves the picture by (dx, dy). */
void cambgMove(Obj *obj, s32 dx, s32 dy) {
    CamBGWork *w = CAMBG(obj);

    if (obj != NULL && w->frame != NULL) {
        w->frame->x += dx;
        w->frame->y += dy;
    }
}

/* Sideways scrolling on (1) or off. */
void cambgScrollX(Obj *obj, s32 on) {
    CamBGWork *w;

    if (obj != NULL) {
        w = CAMBG(obj);
        if (on == 1) {
            w->flags |= 1;
        } else {
            w->flags &= ~1;
        }
    }
}

/* Up and down scrolling on (1) or off. */
void cambgScrollY(Obj *obj, s32 on) {
    CamBGWork *w;

    if (obj != NULL) {
        w = CAMBG(obj);
        if (on == 1) {
            w->flags |= 2;
        } else {
            w->flags &= ~2;
        }
    }
}

/* Shows the picture. */
void cambgShow(Obj *obj) {
    if (obj != NULL && CAMBG(obj)->frame != NULL) {
        CAMBG(obj)->frame->f.flags |= 2;
    }
}

/* Hides it. */
void cambgHide(Obj *obj) {
    if (obj != NULL && CAMBG(obj)->frame != NULL) {
        CAMBG(obj)->frame->f.flags &= ~2;
    }
}

/* The colour program: 0 by the hour, 2 night; either blended into over 60 steps. */
void cambgSetProgram(Obj *obj, s32 mode) {
    CamBGWork *w;

    if (obj != NULL) {
        w = CAMBG(obj);
        switch (mode) {
            case 0:
                w->proc = cambgBlendStep;
                w->target = cambgPalByHour;
                cambgStartBlend(w, 60);
                break;
            case 2:
                w->proc = cambgBlendStep;
                w->target = cambgPalNight;
                cambgStartBlend(w, 60);
                break;
        }
    }
}

void cambgSetAlpha(Obj *obj, u16 v) {
    if (obj != NULL && CAMBG(obj)->frame != NULL) {
        CAMBG(obj)->frame->alpha = v;
    }
}

/* Loads background n: its file, its four palettes and its two pictures. */
void cambgLoad(Obj *obj, s32 n) {
    BGEntry *table;
    BGEntry *e;
    CamBGWork *w;
    s32 i;

    table = heapMallocTopPage(fileRamSize(0xF));
    if (table != NULL) {
        fileLoadNow(0xF, table);
        fileLoadNow(table[n].file, NULL);
        e = &table[n];
        w = CAMBG(obj);
        for (i = 0; i < 4;) {
            w->pal[i] = (u16 *) fadeSegToRam(e->pal[i], e->file);
            i++;
        }
        w->frame->w = e->w;
        w->frame->h = e->h;
        w->frame->tileCount = e->tileCount;
        w->frame->img1 = fadeSegToRam(e->img1, e->file);
        w->frame->img2 = fadeSegToRam(e->img2, e->file);
        w->frame->palA = w->pal[0];
        w->frame->palB = w->pal[0];
        resDrop(0xF);
        heapFree(table);
        return;
    }
    PRINTF("<cambgLoad> メモリーが足りません \n");
    if (table != NULL) {
        heapFree(table);
    }
}
