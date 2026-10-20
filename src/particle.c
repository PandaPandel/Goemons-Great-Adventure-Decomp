#include "object.h"
#include "camera.h"
#include "fade.h"
#include "fileload.h"
#include "gfxmgr.h"
#include "heap.h"
#include "mathutil.h"
#include "particle.h"
#include "tmanager.h"

/* Particle effects drawn in software, tasks under the effect manager (effect.c). Each frame
 * a handler clears a 64x64 4-bit intensity texture in the display-list buffer, plots its
 * particles into it, and draws it on one quad (gPtclQuadVtx) in a colour:
 * - "particle": sparks that fly out and fade, some falling;
 * - "fire_ko": small flames that rise and flicker;
 * - "ame": rain, streaks drawn along the wind the camera sees;
 * - "bhanabi": a firework, a rising trail of "pol" frames that bursts. */

/* A particle: a position and a speed in texels (the centre is 0), and a brightness whose top
 * four bits are what is plotted. */
typedef struct Particle {
    /* 0x0 */ s8 x;
    /* 0x1 */ s8 y;
    /* 0x2 */ s8 vx;
    /* 0x3 */ s8 vy; /* in quarter texels */
    /* 0x4 */ u8 bright;
} Particle;

typedef struct ParticleWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 flags; /* 1: they fall; 2: filtered */
    /* 0x05 */ u8 time;
    /* 0x06 */ Color3 color;
    /* 0x09 */ Particle p[16];
} ParticleWork;

/* A flame: a position in half texels, a rising speed, a heat added to its brightness. */
typedef struct Flame {
    /* 0x0 */ u8 x;
    /* 0x1 */ u8 y;
    /* 0x2 */ s8 rise;
    /* 0x3 */ s8 heat;
    /* 0x4 */ u8 bright;
} Flame;

typedef struct FireWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 time; /* 0xFF: until stopped */
    /* 0x05 */ u8 flags; /* 2: filtered */
    /* 0x06 */ Color3 color;
    /* 0x09 */ Flame p[16];
} FireWork;

/* A raindrop's streak. */
typedef struct Drop {
    /* 0x0 */ u8 x;
    /* 0x1 */ u8 y;
    /* 0x2 */ u8 len;
    /* 0x3 */ u8 bright;
} Drop;

typedef struct RainWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 speed; /* along the camera's view */
    /* 0x08 */ f32 fall;
    /* 0x0C */ f32 unused0C; /* ptclSpawnRain's c, never read */
    /* 0x10 */ u8 count;
    /* 0x11 */ u8 time; /* 0xFF: until stopped */
    /* 0x12 */ Color3 color;
    /* 0x15 */ Drop d[6];
} RainWork;

/* A firework ("bhanabi"): a flash, a core and a trail of 8 frames that fade from the colour
 * to the end colour. It is made with a copy of this. */
typedef struct HanabiWork {
    /* 0x00 */ PolFrame *frame; /* the flash */
    /* 0x04 */ PolFrame *core;
    /* 0x08 */ Frame *trail; /* a "dlist " frame over the trail's "pol" frames */
    /* 0x0C */ s16 alpha; /* the trail's */
    /* 0x0E */ s8 fade;
    /* 0x0F */ s8 fade2; /* the fade after the burst */
    /* 0x10 */ s8 grow; /* the core's alpha step */
    /* 0x12 */ s16 count;
    /* 0x14 */ Color3 end;
    /* 0x17 */ Color3 color;
    /* 0x1C */ f32 rise;
    /* 0x20 */ Vec3f off;
    /* 0x2C */ f32 sx;
    /* 0x30 */ f32 sy;
    /* 0x34 */ f32 rot;
    /* 0x38 */ s16 unused38;
    /* 0x3A */ s16 time; /* to the burst */
    /* 0x3C */ s16 alpha2; /* the core's */
    /* 0x3E */ s16 flash;
    /* 0x40 */ f32 flashGrow;
    /* 0x44 */ Vec3f pos;
} HanabiWork; /* size 0x50 */

extern Frame *gFrmLayers[]; /* the frame pools */
extern Frame *gFrmEffectLayer; /* the frame effects go under (gFrmLayers[9]) */
extern s16 gFrmKindPol; /* "pol" */
extern s16 gFrmKindDlist; /* "dlist " */
extern Gfx hanabiCoreDl[]; /* the core, in file 0x123 */
extern u8 hanabiCoreTex[]; /* its texture */
extern Gfx hanabiFlashDl[]; /* the flash, in file 0x120 */
extern Gfx hanabiTrailDl[]; /* a trail frame, in file 0x120 */
extern Gfx *gGfxDlHead; /* the display-list buffer */
extern Obj *gEffectTask; /* the effect manager */
/* particle.c's .data (0x8010C5E0-0x8010C698), in ROM order. */
Gfx gPtclSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_CULL_BACK),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetTextureLOD(G_TL_TILE),
    gsDPSetTextureDetail(G_TD_CLAMP),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsDPSetTextureConvert(G_TC_FILT),
    gsDPSetCombineLERP(0, 0, 0, PRIMITIVE, PRIMITIVE, 0, TEXEL0, 0, 0, 0, 0, PRIMITIVE, PRIMITIVE, 0, TEXEL0, 0),
    gsDPSetRenderMode(G_RM_ZB_CLD_SURF, G_RM_ZB_CLD_SURF2),
    gsSPEndDisplayList(),
};
Vtx gPtclQuadVtx[4] = { /* the quad */
    { { { -20, -20, 0 }, 0x0000, { 0, 4096 }, { 0x00, 0x00, 0x7F, 0xFF } } },
    { { { 20, 20, 0 }, 0x0000, { 4096, 0 }, { 0x00, 0x00, 0x7F, 0xFF } } },
    { { { 20, -20, 0 }, 0x0000, { 4096, 4096 }, { 0x00, 0x00, 0x7F, 0xFF } } },
    { { { -20, 20, 0 }, 0x0000, { 0, 0 }, { 0x00, 0x00, 0x7F, 0xFF } } },
};


#define FX(obj) ((void *) ((u32) (obj) + sizeof(Obj)))

/* Takes size bytes from the display-list buffer: its pointer summed as a u32, as the allocators
 * in gfxbuf.c do it, in a statement macro (the usual do-while). */
#define GFX_ALLOC(p, size) \
    do { (p) = (void *) gGfxDlHead; gGfxDlHead = (Gfx *) ((u32) gGfxDlHead + (size)); } while (0)

/* The spark handler.
 * The y sum is written "y + (vy >> 2)", y first: the game loads vx, x and vy before the bright
 * store and y after it. GFX_ALLOC's do-while is a block boundary between buf's load and the
 * clear, which gives buf the game's saved register (s0) across the clear. */
s32 particle(Obj *obj, s16 msg, void *arg) {
    u8 *buf;
    ParticleWork *w = FX(obj);
    s32 i;
    s32 n;
    s32 x;
    s32 y;
    s32 k;

    switch (msg) {
        case 2:
            obj->name = "particle";
            w->frame = (PolFrame *) frmCreate(gFrmEffectLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl = gPtclSetupDl;
            w->frame->flags |= 7;
            w->time = 0x40;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            n = 0;
            if (w->time == 0) {
                return -1;
            }
            w->time--;
            for (i = 0; i < 16; i++) {
                if (w->p[i].bright >= 0x10) {
                    w->p[i].bright -= 4;
                    x = w->p[i].x + w->p[i].vx;
                    y = w->p[i].y + (w->p[i].vy >> 2);
                    if ((u16) (x + 0x80) >= 0x100 || (u16) (y + 0x80) >= 0x100) {
                        w->p[i].bright &= 0xF;
                    } else {
                        w->p[i].x = x;
                        w->p[i].y = y;
                        n++;
                        if (w->flags & 1) {
                            if (w->p[i].vy < 0x18) {
                                if ((w->p[i].bright & 3) + 1 == 4) {
                                    w->p[i].vy++;
                                } else {
                                    w->p[i].bright++;
                                }
                            }
                        }
                    }
                }
            }
            if (n == 0) {
                return -1;
            }
            break;
        case 3:
            GFX_ALLOC(buf, 0x800);
            heapMemClear(buf, 0x800);
            for (i = 0; i < 16; i++) {
                if (w->p[i].bright >= 0x10) {
                    k = (w->p[i].x >> 2) + ((w->p[i].y >> 2) << 6) + 0x820;
                    buf[k >> 1] |= (w->p[i].bright & 0xF0) >> ((k & 1) * 4);
                }
            }
            w->frame->dl2 = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPLoadTextureBlock_4b(gGfxDlHead++, buf, G_IM_FMT_I, 64, 64, 0, G_TX_CLAMP, G_TX_CLAMP,
                                   G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->color.r, w->color.g, w->color.b, 255);
            if (w->flags & 2) {
                gDPSetTextureFilter(gGfxDlHead++, G_TF_BILERP);
            } else {
                gDPSetTextureFilter(gGfxDlHead++, G_TF_POINT);
            }
            gSPVertex(gGfxDlHead++, gPtclQuadVtx, 4, 0);
            gSP2Triangles(gGfxDlHead++, 2, 1, 0, 0, 1, 3, 0, 0);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Sparks at (x, y, z), yellow. */
void ptclSpawnSparks(f32 x, f32 y, f32 z) {
    Obj *obj = tskCreate(gEffectTask, particle, 0, NULL);
    ParticleWork *w;
    s32 i;

    if (obj != NULL) {
        w = FX(obj);
        w->color.b = 0xA0;
        w->color.r = 0xFF;
        w->color.g = 0xFF;
        w->frame->pos[0] = x;
        w->frame->pos[1] = y;
        w->frame->pos[2] = z;
        for (i = 0; i < 16; i++) {
            w->p[i].vx = fadeRand(8) - 4;
            w->p[i].vy = fadeRand(32) - 16;
            w->p[i].bright = fadeRand(128) + 128;
        }
    }
}

/* Sparks at (x, y, z), yellow, that fall (the burst's, effect.c). */
void ptclSpawnSparksFall(f32 x, f32 y, f32 z) {
    Obj *obj = tskCreate(gEffectTask, particle, 0, NULL);
    ParticleWork *w;
    s32 i;

    if (obj != NULL) {
        w = FX(obj);
        w->flags |= 1;
        w->color.b = 0xA0;
        w->color.r = 0xFF;
        w->color.g = 0xFF;
        w->frame->pos[0] = x;
        w->frame->pos[1] = y;
        w->frame->pos[2] = z;
        for (i = 0; i < 16; i++) {
            w->p[i].vx = fadeRand(8) - 4;
            w->p[i].vy = fadeRand(32) - 20;
            w->p[i].bright = fadeRand(128) + 128;
        }
    }
}

/* White sparks at (x, y, z) that fall. */
void ptclSpawnSparksWhite(f32 x, f32 y, f32 z) {
    Obj *obj = tskCreate(gEffectTask, particle, 0, NULL);
    ParticleWork *w;
    s32 i;

    if (obj != NULL) {
        w = FX(obj);
        w->color.r = 0xFF;
        w->color.g = 0xFF;
        w->flags |= 1;
        w->color.b = 0xFF;
        w->frame->pos[0] = x;
        w->frame->pos[1] = y;
        w->frame->pos[2] = z;
        for (i = 0; i < 16; i++) {
            w->p[i].vx = fadeRand(8) - 4;
            w->p[i].vy = fadeRand(32) - 20;
            w->p[i].bright = fadeRand(128) + 128;
        }
    }
}

/* Sparks at (x, y, z) in a colour, on a quad of a size. (The flag tests set what they
 * test.) */
void ptclSpawnSparksColor(f32 x, f32 y, f32 z, u8 r, u8 g, u8 b, f32 sx, f32 sy) {
    Obj *obj = tskCreate(gEffectTask, particle, 0, NULL);
    ParticleWork *w;
    s32 i;

    if (obj != NULL) {
        w = FX(obj);
        if (w->flags & 1) {
            w->flags |= 1;
        }
        if (w->flags & 2) {
            w->flags |= 2;
        }
        w->color.r = r;
        w->color.g = g;
        w->color.b = b;
        w->frame->pos[0] = x;
        w->frame->pos[1] = y;
        w->frame->pos[2] = z;
        w->frame->scale[0] = sx;
        w->frame->scale[1] = sy;
        for (i = 0; i < 16; i++) {
            w->p[i].vx = fadeRand(8) - 4;
            if (w->flags & 1) {
                w->p[i].vy = fadeRand(32) - 20;
            } else {
                w->p[i].vy = fadeRand(32) - 16;
            }
            w->p[i].bright = fadeRand(128) + 128;
        }
    }
}

/* The flame handler. Everything happens as it draws: each flame rises by its speed and
 * brightens by its heat; at 0xC0 it starts to cool and slows by half; below 11 it is out. A
 * flame that is out or has left the top starts again at the bottom. (14 of the 16 are run.) */
s32 fire_ko(Obj *obj, s16 msg, void *arg) {
    FireWork *w = FX(obj);
    u8 *buf;
    s32 i;
    s32 n;
    s32 y;
    s32 k;

    switch (msg) {
        case 2:
            obj->name = "fire_ko";
            w->frame = (PolFrame *) frmCreate(gFrmEffectLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl = gPtclSetupDl;
            w->frame->flags |= 7;
            w->time = 0x40;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            if (w->time == 0) {
                return -1;
            }
            if (w->time != 0xFF) {
                w->time--;
            }
            GFX_ALLOC(buf, 0x800);
            heapMemClear(buf, 0x800);
            n = 0;
            for (i = 0; i < 14; i++) {
                k = (w->p[i].x >> 1) + ((w->p[i].y >> 1) << 6);
                buf[k >> 1] |= (w->p[i].bright & 0xF0) >> ((k & 1) * 4);
                w->p[i].bright += w->p[i].heat;
                if (w->p[i].bright >= 0xC0) {
                    w->p[i].heat = -10;
                    w->p[i].rise /= 2;
                }
                if (w->p[i].bright < 11) {
                    w->p[i].heat = 0;
                    w->p[i].bright = 0;
                }
                y = w->p[i].y - w->p[i].rise;
                if (y >= 0x80 || w->p[i].bright == 0) {
                    w->p[i].x = fadeRand(128);
                    w->p[i].y = fadeRand(32) + 0x60;
                    w->p[i].rise = fadeRandRange(2, 5);
                    w->p[i].heat = fadeRandRange(7, 10);
                    w->p[i].bright = 20;
                } else {
                    w->p[i].y = y;
                    k = (w->p[i].x >> 1) + ((w->p[i].y >> 1) << 6);
                    buf[k >> 1] |= (w->p[i].bright & 0xF0) >> ((k & 1) * 4);
                    n++;
                }
            }
            if (n == 0) {
                return -1;
            }
            w->frame->dl2 = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPLoadTextureBlock_4b(gGfxDlHead++, buf, G_IM_FMT_I, 64, 64, 0, G_TX_CLAMP, G_TX_CLAMP,
                                   G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->color.r, w->color.g, w->color.b, 255);
            if (w->flags & 2) {
                gDPSetTextureFilter(gGfxDlHead++, G_TF_BILERP);
            } else {
                gDPSetTextureFilter(gGfxDlHead++, G_TF_POINT);
            }
            gDPSetTextureFilter(gGfxDlHead++, G_TF_POINT);
            gSPVertex(gGfxDlHead++, gPtclQuadVtx, 4, 0);
            gSP2Triangles(gGfxDlHead++, 2, 1, 0, 0, 1, 3, 0, 0);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Flames at (x, y, z), orange, for a time (0xFF: until stopped). */
void ptclSpawnFire(f32 x, f32 y, f32 z, u8 time) {
    Obj *obj = tskCreate(gEffectTask, fire_ko, 0, NULL);
    FireWork *w;
    s32 i;

    if (obj != NULL) {
        w = FX(obj);
        w->color.r = 0xFF;
        w->color.g = 0x40;
        w->color.b = 0;
        w->frame->pos[0] = x;
        w->frame->pos[1] = y;
        w->frame->pos[2] = z;
        w->time = time;
        w->flags |= 2;
        for (i = 0; i < 16; i++) {
            w->p[i].x = fadeRand(128);
            w->p[i].y = fadeRand(128);
            w->p[i].rise = fadeRandRange(2, 5);
            w->p[i].heat = fadeRandRange(7, 10);
            w->p[i].bright = fadeRandRange(20, 192);
        }
    }
}

/* The rain handler. The frame drifts along the camera's view by the speed and falls. */
s32 ame(Obj *obj, s16 msg, void *arg) {
    RainWork *w = FX(obj);
    u8 *buf;
    s32 i;
    s32 pad[3];
    f32 atx;
    f32 aty;
    f32 atz;
    f32 eyex;
    f32 eyey;
    f32 eyez;
    f32 angle;
    void *cam;

    switch (msg) {
        case 2:
            obj->name = "ame";
            w->frame = (PolFrame *) frmCreate(gFrmEffectLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl = gPtclSetupDl;
            w->frame->flags |= 7;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (w->time == 0) {
                return -1;
            }
            if (w->time != 0xFF) {
                w->time--;
            }
            cam = (*camAiGetCam)();
            (*camGetAt)(cam, &atx, &aty, &atz);
            (*camGetEye)(cam, &eyex, &eyey, &eyez);
            angle = mathAtan2(atx - eyex, atz - eyez);
            w->frame->pos[0] += (*mathCos)(angle) * w->speed;
            w->frame->pos[2] += (*mathSin)(angle) * w->speed;
            w->frame->pos[1] += w->fall;
            break;
        case 3:
            GFX_ALLOC(buf, 0x800);
            heapMemClear(buf, 0x800);
            for (i = 0; i < w->count; i++) {
                ptclDrawRainStreak(buf, w->d[i].x, w->d[i].y, w->d[i].len, w->speed, w->fall,
                                       w->d[i].bright);
            }
            w->frame->dl2 = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPLoadTextureBlock_4b(gGfxDlHead++, buf, G_IM_FMT_I, 64, 64, 0, G_TX_CLAMP, G_TX_CLAMP,
                                   G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->color.r, w->color.g, w->color.b, 255);
            gDPSetTextureFilter(gGfxDlHead++, G_TF_BILERP);
            gSPVertex(gGfxDlHead++, gPtclQuadVtx, 4, 0);
            gSP2Triangles(gGfxDlHead++, 2, 1, 0, 0, 1, 3, 0, 0);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Plots a streak of len texels from (x, y), up the wind (dx, dy), fading from the
 * brightness (at most 15) to 0. It stops at the texture's edge. */
void ptclDrawRainStreak(u8 *buf, u8 x, u8 y, u8 len, f32 dx, f32 dy, u8 bright) {
    f32 fx;
    f32 fy;
    f32 fb;
    f32 d;
    s32 i;
    s32 k;

    if (x >= 64) {
        x = 63;
    }
    if (x <= 0) {
        x = 0;
    }
    if (y >= 64) {
        y = 63;
    }
    if (y <= 0) {
        y = 0;
    }
    if (bright > 16) {
        bright = 15;
    }
    fx = x;
    fy = y;
    fb = bright;
    d = sqrtf(dx * dx + dy * dy);
    for (i = 0; i < len; i++) {
        fx += -dx / d;
        if (fx > 63.0 || fx < 0.0) {
            return;
        }
        fy += dy / d;
        if (fy > 63.0 || fy < 0.0) {
            return;
        }
        fb -= (f32) bright / (f32) len;
        k = (s32) fx + ((s32) fy << 6);
        buf[k >> 1] |= ((s32) fb << 4) >> ((k & 1) * 4);
    }
}

/* Rain around (x, y, z): count streaks (at most 6) of a length and brightness, white, blown
 * at a speed and falling, for a time (0xFF: until stopped). The streak's random height is
 * taken as a u8 before len is added. */
void ptclSpawnRain(f32 x, f32 y, f32 z, f32 speed, f32 fall, f32 c, u8 len, u8 bright,
                            u8 count, u8 time) {
    Obj *obj = tskCreate(gEffectTask, ame, 0, NULL);
    u8 top;
    RainWork *w;
    s32 i;

    if (obj != NULL) {
        w = FX(obj);
        w->count = count;
        if (w->count >= 7) {
            w->count = 6;
        }
        w->time = time;
        w->color.r = 0xFF;
        w->color.g = 0xFF;
        w->color.b = 0xFF;
        w->frame->pos[0] = x;
        w->frame->pos[1] = y;
        w->frame->pos[2] = z;
        w->speed = speed;
        w->fall = fall;
        w->unused0C = c;
        for (i = 0; i < w->count; i++) {
            w->d[i].x = fadeRand(64);
            top = fadeRand(64 - len);
            w->d[i].y = top + len;
            w->d[i].len = len;
            w->d[i].bright = bright;
        }
    }
}

/* The firework handler. Each step the trail's first frame moves to its end, one offset past
 * the last; the flash grows for 4 steps; the core brightens for the last 30 steps before the
 * burst, then fades with the trail.
 * The core's roll divides by 512.0 (div.s), so "/ 512". Each case walks the frames with its
 * own pointer (f, t, prev), and the trail's colours go into r, g, b before the command: that
 * is the game's frame (next spilled at 0x34, w at 0x4C), its s3, and its hoist of
 * hanabiTrailDl. */
s32 bhanabi(Obj *obj, s16 msg, void *arg) {
    HanabiWork *w = FX(obj);
    PolFrame *f;
    PolFrame *t;
    s32 r;
    s32 g;
    s32 b;
    PolFrame *next;
    s32 i;
    s32 a;
    PolFrame *prev;

    switch (msg) {
        case 2:
            obj->name = "bhanabi";
            *w = *(HanabiWork *) arg;
            w->frame = (PolFrame *) frmCreate(gFrmLayers[9], gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            floadGetSetRam((void **) &w->frame->segs[0], 0x120);
            w->frame->dl2 = hanabiFlashDl;
            w->frame->pos[0] = w->pos.x;
            w->frame->pos[1] = w->pos.y;
            w->frame->pos[2] = w->pos.z;
            w->frame->scale[0] = w->frame->scale[1] = 0.0f;
            w->frame->flags |= 7;
            w->flash = 4;
            w->flashGrow = w->sx * w->time / w->flash;
            w->core = (PolFrame *) frmCreate(gFrmLayers[9], gFrmKindPol);
            if (w->core == NULL) {
                return -1;
            }
            floadGetSetRam((void **) &w->core->segs[0], 0x123);
            w->core->segs[3] = floadSegToRam((u32) hanabiCoreTex, 0x123, 0);
            w->core->dl2 = hanabiCoreDl;
            w->core->pos[0] = w->pos.x;
            w->core->pos[1] = w->pos.y - 0.005f * (w->time * w->time);
            w->core->pos[2] = w->pos.z;
            w->core->scale[0] = w->sx * w->time;
            w->core->scale[1] = w->sy * w->time;
            w->core->flags |= 3;
            w->core->rot[2] = fadeRand(0x400) * 3.1415927f / 512;
            w->trail = frmCreate(gFrmLayers[9], gFrmKindDlist);
            if (w->trail == NULL) {
                return -1;
            }
            ((DLFrame *) w->trail)->dl = gPtclSetupDl;
            w->count = 8;
            for (i = 0; i < w->count; i++) {
                f = (PolFrame *) frmCreate(w->trail, gFrmKindPol);
                if (f != NULL) {
                    f->dl2 = hanabiTrailDl;
                    floadGetSetRam((void **) &f->segs[0], 0x120);
                    f->flags |= 3;
                    f->pos[0] = w->pos.x;
                    f->pos[1] = w->pos.y;
                    f->pos[2] = w->pos.z;
                    f->scale[0] = 0.0f;
                    f->scale[1] = 0.0f;
                    f->rot[2] = w->core->rot[2];
                }
            }
            break;
        case 1:
            w->frame = (PolFrame *) frmDestroy(&w->frame->f);
            w->core = (PolFrame *) frmDestroy(&w->core->f);
            w->trail = frmDestroy(w->trail);
            break;
        case 0:
            w->count = frmCountChildren(w->trail);
            w->flash--;
            if (w->flash < 0) {
                w->frame = (PolFrame *) frmDestroy(&w->frame->f);
            }
            if (w->frame != NULL) {
                w->frame->scale[0] += w->flashGrow;
                w->frame->scale[1] += w->flashGrow;
            }
            w->time--;
            if (w->time < 0) {
                w->fade = w->fade2;
                w->alpha2 -= (w->grow + 1) / 2;
                if (w->alpha2 < 0) {
                    return -1;
                }
            } else if (w->time < 30) {
                w->alpha2 += w->grow;
            }
            w->alpha += w->fade;
            w->off.y += w->rise;
            if (w->alpha < 0) {
                w->alpha = 0;
            }
            f = (PolFrame *) w->trail->child;
            if (f != NULL) {
                frmMove(&f->f, w->trail);
                prev = (PolFrame *) f->f.prev;
                if (prev != NULL) {
                    f->pos[0] = prev->pos[0] + w->off.x;
                    f->pos[1] = prev->pos[1] + w->off.y;
                    f->pos[2] = prev->pos[2] + w->off.z;
                    f->scale[0] = prev->scale[0] + w->sx;
                    f->scale[1] = prev->scale[1] + w->sy;
                    f->rot[2] = prev->rot[2] + w->rot;
                }
            }
            break;
        case 3:
            if (w->frame != NULL) {
                w->frame->dl = gGfxDlHead;
                gSPDisplayList(gGfxDlHead++, gPtclSetupDl);
                gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 255, 255);
                gSPEndDisplayList(gGfxDlHead++);
            }
            if (w->core != NULL) {
                w->core->dl = gGfxDlHead;
                gSPDisplayList(gGfxDlHead++, gPtclSetupDl);
                gDPSetPrimColor(gGfxDlHead++, 0, 0, w->color.r, w->color.g, w->color.b, w->alpha2);
                gSPEndDisplayList(gGfxDlHead++);
            }
            i = 1;
            for (t = (PolFrame *) w->trail->child; t != NULL; t = next, i++) {
                next = (PolFrame *) t->f.next;
                a = w->alpha - (w->count - i) * 4;
                if (a < 0) {
                    frmDestroy(&t->f);
                } else {
                    r = (w->end.r - w->color.r) * i / w->count + w->color.r;
                    g = (w->end.g - w->color.g) * i / w->count + w->color.g;
                    b = (w->end.b - w->color.b) * i / w->count + w->color.b;
                    t->dl = gGfxDlHead;
                    gDPSetPrimColor(gGfxDlHead++, 0, 0, r, g, b, a);
                    gSPEndDisplayList(gGfxDlHead++);
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A firework at (x, y, z) of a size, in a colour, fading to white along its trail. */
void ptclSpawnHanabi(f32 x, f32 y, f32 z, f32 size, s32 r, s32 g, s32 b) {
    HanabiWork init;
    f32 s;

    heapMemClear(&init, sizeof(init));
    init.end.b = init.end.g = init.end.r = 0xFF;
    s = 0.02f * size;
    init.sx = s;
    init.sy = s;
    init.color.r = r;
    init.color.g = g;
    init.color.b = b;
    init.alpha = 0xFF;
    init.fade = 0;
    init.fade2 = -8;
    init.grow = 2;
    init.time = 40;
    init.off.x = init.off.y = init.off.z = 0.0f;
    init.rot = 0.0f;
    init.rise = -0.01f;
    init.pos.x = x;
    init.pos.y = y;
    init.pos.z = z;
    tskCreate(gEffectTask, bhanabi, 0, &init);
}

/* A small firework that bursts sooner, with falling sparks (particle's). */
void ptclSpawnHanabiSmall(f32 x, f32 y, f32 z, f32 size, s32 r, s32 g, s32 b) {
    HanabiWork init;
    f32 s;
    Obj *obj;

    heapMemClear(&init, sizeof(init));
    init.end.b = init.end.g = init.end.r = 0xFF;
    s = 0.05f * size;
    init.sx = s;
    init.sy = s;
    init.color.r = r;
    init.color.g = g;
    init.color.b = b;
    init.alpha = 0xFF;
    init.fade = 0;
    init.fade2 = -16;
    init.grow = 8;
    init.time = 24;
    init.off.x = init.off.y = init.off.z = 0.0f;
    init.rot = 0.0f;
    init.rise = -0.01f;
    init.pos.x = x;
    init.pos.y = y;
    init.pos.z = z;
    obj = tskCreate(gEffectTask, bhanabi, 0, &init);
    if (obj != NULL) {
        ((HanabiWork *) FX(obj))->time = 8;
    }
    ptclSpawnSparksFall(x, y, z);
}

/* A red firework. */
void ptclSpawnHanabiRed(f32 x, f32 y, f32 z, f32 size) {
    ptclSpawnHanabi(x, y, z, size, 0xFF, 0, 0);
}
