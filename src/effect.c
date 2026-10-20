#include "object.h"
#include "effect.h"
#include "fade.h"
#include "fileload.h"
#include "gfxmgr.h"
#include "itex.h"
#include "particle.h"
#include "sndloc.h"
#include "tilemap.h"
#include "tmanager.h"

/* The effect manager ("effect", gEffectTask) and its effects, each a task under it drawing
 * "pol" frames:
 * - puffs (effectPuffProc): a textured, animated sprite that drifts, grows and fades;
 * - "kiran" (kiran): a flash;
 * - "Fspirit" (Fspirit): a spirit glow in two colours;
 * - "pompom" (pompom): a chain of small explosions that follows a point;
 * - "Star" (effectStarProc): a star;
 * - "sibuki" (sibuki): a splash of three rings.
 * The manager also makes the "Bg" frame kind (tilemap.c's layers). The handlers are still
 * assembly. */



/* A kind of puff. */
typedef struct FxKind {
    /* 0x00 */ char *name;
    /* 0x04 */ Gfx *dl;
    /* 0x08 */ s32 file; /* segment 3, and the animation's */
    /* 0x0C */ AnimFrame *frames;
    /* 0x10 */ u16 flags; /* the frame's */
} FxKind; /* size 0x14 */

/* A flash's work. */
typedef struct KiranWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[0xC];
    /* 0x10 */ s32 alpha;
    /* 0x14 */ s32 state; /* 0: brightening, 1: fading */
    /* 0x18 */ s32 rise;
    /* 0x1C */ s32 fade;
} KiranWork;

/* What a spirit glow is made with. */
typedef struct SpiritInit {
    /* 0x00 */ u8 pad00[0x8];
    /* 0x08 */ f32 pos[3];
    /* 0x14 */ f32 scale;
    /* 0x18 */ u8 pad18[0x8];
    /* 0x20 */ Color3 color;
    /* 0x23 */ Color3 color2;
} SpiritInit; /* size 0x28 */

/* A spirit glow's work. */
typedef struct SpiritWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 rise;
    /* 0x08 */ u8 pad08[0xC];
    /* 0x14 */ f32 size;
    /* 0x18 */ s32 time;
    /* 0x1C */ s32 alpha;
    /* 0x20 */ Color3 color;
    /* 0x23 */ Color3 color2;
    /* 0x26 */ u8 state; /* 0: growing, 1: rising and fading */
} SpiritWork;

/* An explosion chain ("pompom"), and what it is made with: every interval (give or take a
 * third) it makes an effect of its kind at a random point around pos, of a random size,
 * until its life runs out. */
typedef struct PompomInit {
    /* 0x00 */ f32 pos[3];
    /* 0x0C */ f32 spread[3];
    /* 0x18 */ f32 size;
    /* 0x1C */ f32 var; /* used only when below the size */
    /* 0x20 */ s32 interval;
    /* 0x24 */ s32 life;
    /* 0x28 */ s32 kind; /* 5-14: which effect */
    /* 0x2C */ s32 timer;
} PompomInit; /* size 0x30 */

/* A star's init. */
typedef struct StarInit {
    /* 0x00 */ u8 pad00[0x24];
    /* 0x24 */ f32 pos[3];
    /* 0x30 */ u8 pad30[0x14];
} StarInit; /* size 0x44 */

/* A star's work. */
typedef struct StarWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[0x38];
    /* 0x3C */ s32 alpha;
} StarWork;

/* A splash's work: three rings that widen and rise for a time, then widen and flatten. */
typedef struct SibukiWork {
    /* 0x00 */ PolFrame *ring[3];
    /* 0x0C */ s32 alpha;
    /* 0x10 */ f32 widenRise; /* the widening, rising */
    /* 0x14 */ f32 widenFall; /* and after */
    /* 0x18 */ f32 rise; /* the rise */
    /* 0x1C */ f32 fall; /* the fall */
    /* 0x20 */ s32 time;
    /* 0x24 */ u8 state;
    /* 0x25 */ Color3 color;
    /* 0x28 */ Color3 color2;
} SibukiWork;

extern FxKind gEffectPuffTable[]; /* the puffs */
extern Frame *gFrmLayer8; /* the frame puffs go under (gFrmLayers[8]) */
extern s16 gFrmKindPol; /* "pol" */
extern Gfx *gGfxDlHead; /* the display-list buffer */
extern Gfx gKiranSetupDl[]; /* the flash's draw setup */
extern Gfx kiranFlashDl[]; /* the flash, in file 0x120 */
extern Gfx effectSpiritGlowDl[]; /* the spirit glow, in file 0x126 */
extern Gfx effectStarDl[]; /* the star, in file set 0x124 (file 770): the same address, another file */
extern Gfx sibukiRing1Dl[]; /* the splash's rings, in file 0x127 */
extern Gfx sibukiRing2Dl[];
extern Gfx sibukiRing3Dl[];
extern Gfx gDrawSetupPrimEnvTexBlendDl[]; /* the splash's draw setup */
extern G88338 gGameWork;
/* effect.c's .data (0x8010C1C0-0x8010C290), in ROM order. */
extern AnimFrame gItexPuffAnim[]; /* the animations of itex.c */
extern AnimFrame gItexGlowAnim[];
extern AnimFrame gItexBombAnim[];
/* Every "Bg" frame starts as a copy of this: opaque (alpha 0x100), wrapping both ways. */
BGFrame gBgFrameTemplate = { { 0 }, 0, 0, 0.0f, 0.0f, 0, 0, 0, 0, NULL, NULL, 0, 0, 0, 0x100, 1, 1 };
Obj *gEffectTask = NULL; /* the manager */
s16 gFrmKindBg = 0; /* "Bg" */


#define FX(obj) ((void *) ((u32) (obj) + sizeof(Obj)))

/* The manager: makes the "Bg" kind and loads nine effect files. */
s32 effect(Obj *obj, s16 msg, void *arg) {
    s32 pad[2];

    switch (msg) {
        case 0:
            break;
        case 2:
            gEffectTask = obj;
            obj->name = "effect";
            gFrmKindBg =
                frmNewKind("Bg", bgDraw, 0x44, 8, &gBgFrameTemplate);
            floadLoadSetToTop(0x11E);
            floadLoadSetToTop(0x120);
            floadLoadSetToTop(0x11F);
            floadLoadSetToTop(0x123);
            floadLoadSetToTop(0x126);
            floadLoadSetToTop(0x124);
            floadLoadSetToTop(0x127);
            floadLoadSetToTop(0x128);
            floadLoadSetToTop(0x122);
            break;
        case 1:
            if (gFrmKindBg != 0) {
                frmDeleteKind(gFrmKindBg);
                gFrmKindBg = 0;
            }
            gEffectTask = NULL;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The puff handler: it animates, drifts, grows and fades out, and ends with its animation. */
s32 effectPuffProc(Obj *obj, s16 msg, void *arg) {
    FxWork *init = arg;
    FxWork *w = FX(obj);

    switch (msg) {
        case 2:
            obj->name = gEffectPuffTable[init->kind].name;
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = gEffectPuffTable[init->kind].dl;
            floadGetSetRam((void **) &w->frame->segs[0], gEffectPuffTable[init->kind].file);
            fadeAnimStart(&w->anim, gEffectPuffTable[init->kind].frames, 0,
                                   &w->frame->segs[3],
                                   floadGetSetFile(gEffectPuffTable[init->kind].file, 0));
            w->frame->flags |= gEffectPuffTable[init->kind].flags;
            w->frame->pos[0] = init->pos[0];
            w->frame->pos[1] = init->pos[1];
            w->frame->pos[2] = init->pos[2];
            if (init->kind != 0) {
                w->frame->rot[2] = fadeRandRange(0, 359) * 0.0174532930038800785;
            }
            w->vel[0] = init->vel[0];
            w->vel[1] = init->vel[1];
            w->vel[2] = init->vel[2];
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = init->scale;
            w->grow = init->grow;
            w->alpha = init->alpha;
            w->fade = init->fade;
            w->r = init->r;
            w->g = init->g;
            w->b = init->b;
            w->unused40 = init->unused40;
            w->kind = init->kind;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            if (w->kind == 2) {
                gDPSetCombineLERP(gGfxDlHead++, 0, 0, 0, ENVIRONMENT, ENVIRONMENT, 0, TEXEL0, 0, 0,
                                  0, 0, ENVIRONMENT, ENVIRONMENT, 0, TEXEL0, 0);
            }
            gDPSetEnvColor(gGfxDlHead++, w->r, w->g, w->b, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            if (fadeAnimStep(&w->anim) != 0) {
                return -1;
            }
            w->frame->pos[0] += w->vel[0];
            w->frame->pos[1] += w->vel[1];
            w->frame->pos[2] += w->vel[2];
            w->frame->scale[0] += w->grow;
            w->frame->scale[1] += w->grow;
            w->frame->scale[2] += w->grow;
            w->alpha -= (w->fade > 0) ? w->fade : -w->fade;
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A puff at (x, y, z) of a kind: still, 0.3 growing by 0.02, fading by 10. */
void effectPuff(f32 x, f32 y, f32 z, u8 kind) {
    FxWork init;

    init.pos[0] = x;
    init.pos[1] = y;
    init.alpha = 0xFF;
    init.fade = 10;
    init.unused40 = init.r = init.g = init.b = 0;
    init.vel[0] = 0.0f;
    init.vel[1] = 0.0f;
    init.vel[2] = 0.0f;
    init.pos[2] = z;
    init.scale = 0.3f;
    init.grow = 0.02f;
    init.kind = kind;
    tskCreate(gEffectTask, effectPuffProc, 0, &init);
}

/* A puff rising at vy, of a size and growth. */
void effectPuffRise(f32 x, f32 y, f32 z, f32 scale, f32 grow, f32 vy, u8 kind) {
    FxWork init;

    init.pos[0] = x;
    init.pos[1] = y;
    init.alpha = 0xFF;
    init.fade = 10;
    init.unused40 = init.r = init.g = init.b = 0;
    init.vel[0] = 0.0f;
    init.vel[2] = 0.0f;
    init.pos[2] = z;
    init.vel[1] = vy;
    init.scale = scale;
    init.grow = grow;
    init.kind = kind;
    tskCreate(gEffectTask, effectPuffProc, 0, &init);
}

/* A puff made as given. */
void effectPuffMake(FxWork init) {
    tskCreate(gEffectTask, effectPuffProc, 0, &init);
}

/* A puff's defaults. */
void effectPuffDefaults(FxWork *init) {
    init->pos[0] = 0.0f;
    init->pos[1] = 0.0f;
    init->pos[2] = 0.0f;
    init->vel[0] = 0.0f;
    init->vel[1] = 0.0f;
    init->vel[2] = 0.0f;
    init->scale = 0.1f;
    init->grow = 0.01f;
    init->alpha = 0xFF;
    init->fade = 10;
    init->r = 0;
    init->g = 0;
    init->b = 0;
    init->unused40 = 0;
    init->kind = 2;
}

/* A burst: a sparkle (itex.c) above, particles (particle.c) and a white spirit glow. */
void effectBurst(f32 x, f32 y, f32 z) {
    itexSparkleBlue(x, y + 5.0f, z, 0.1f);
    ptclSpawnSparksFall(x, y, z);
    effectFspiritCreate(x, y, z, 0.1f, 0xE0, 0xE0, 0xE0, 0x10, 0x10, 0x10);
}

FxKind gEffectPuffTable[] = { /* the puffs */
    { "FbombL", (Gfx *) 0x03000040, 0x11F, gItexBombAnim, 0x23 },
    { "FspL", (Gfx *) 0x03000040, 0x121, gItexGlowAnim, 0x23 },
    { "Fsmoke2", (Gfx *) 0x030003A0, 0x123, gItexPuffAnim, 0x23 },
};
Gfx gKiranSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetCombineLERP(PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsSPEndDisplayList(),
};

/* The flash handler: white, its alpha rising by `rise` to 0xFF, then falling by `fade`. */
s32 kiran(Obj *obj, s16 msg, void *arg) {
    KiranWork *w = FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "kiran";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = kiranFlashDl;
            floadGetSetRam((void **) &w->frame->segs[0], 0x120);
            w->frame->flags |= 7;
            w->alpha = 1;
            w->state = 0;
            PRINTF("ぴか\n");
            break;
        case 1:
            (*frmDestroy)(&w->frame->f);
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, gKiranSetupDl);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 255, 255);
            gDPSetEnvColor(gGfxDlHead++, 255, 255, 255, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->alpha += w->rise;
                    if (w->alpha >= 0xFF) {
                        w->alpha = 0xFF;
                        w->state = 1;
                    }
                    break;
                case 1:
                    w->alpha -= w->fade;
                    if (w->alpha < 0) {
                        w->alpha = 0;
                        PRINTF("フラッシュを消しました\n");
                        return -1;
                    }
                    break;
            }
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* A flash at (x, y, z): 500 across, rising by 60, fading by 15. */
void effectKiranCreate(f32 x, f32 y, f32 z) {
    Obj *obj = tskCreate(gEffectTask, kiran, 0, NULL);

    if (obj != NULL) {
        KiranWork *w = FX(obj);

        w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 500.0f;
        w->frame->pos[0] = x;
        w->frame->pos[1] = y;
        w->frame->pos[2] = z;
        w->rise = 60;
        w->fade = 15;
    }
}

/* A flash of a size (depth 1), rising and fading at given rates. */
void effectKiranCreateSized(f32 x, f32 y, f32 z, f32 size, s32 rise, s32 fade) {
    Obj *obj = tskCreate(gEffectTask, kiran, 0, NULL);

    if (obj != NULL) {
        KiranWork *w = FX(obj);

        w->frame->scale[0] = w->frame->scale[1] = size;
        w->frame->scale[2] = 1.0f;
        w->frame->pos[0] = x;
        w->frame->pos[1] = y;
        w->frame->pos[2] = z;
        w->rise = rise;
        w->fade = fade;
    }
}

/* The spirit glow handler. It grows to its size while it brightens to 0xA0; two steps later
 * it plays sound 0x52C, then rises ever faster, stretches and fades.
 * The draw's alpha clamp is written on one line: only then does IDO put w's addiu, not the
 * &gGfxDlHead one, in the clamp branch's delay slot, as the original does (IDO_NOTES 12). */
s32 Fspirit(Obj *obj, s16 msg, void *arg) {
    SpiritInit *init = arg;
    SpiritWork *w = FX(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "Fspirit";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            floadGetSetRam((void **) &w->frame->segs[0], 0x126);
            w->frame->dl2 = effectSpiritGlowDl;
            w->frame->flags |= 7;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 0.0f;
            w->size = init->scale;
            w->frame->pos[0] = init->pos[0];
            w->frame->pos[1] = init->pos[1];
            w->frame->pos[2] = init->pos[2];
            w->rise = w->size * 5.0;
            w->color = init->color;
            w->color2 = init->color2;
            w->alpha = 0;
            w->state = 0;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->frame->scale[0] += w->size / 10.0f;
                    if (w->size < w->frame->scale[0]) {
                        w->frame->scale[0] = w->size;
                    }
                    w->frame->scale[1] = w->frame->scale[2] = w->frame->scale[0];
                    w->frame->pos[1] += w->rise;
                    w->alpha += 16;
                    if (w->alpha >= 0xA0) {
                        w->alpha = 0xA0;
                        w->time = 2;
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (w->time-- == 0) {
                        SoundLocation2(0x52C, gGameWork.view, w->frame->pos[0],
                                               w->frame->pos[1], w->frame->pos[2], 0.0f, 0.0f,
                                               gGameWork.view->fovy);
                    }
                    w->rise += 1.5 * w->size;
                    w->frame->pos[1] += w->rise;
                    w->frame->scale[1] += (w->size / 10.0f) * 1.5;
                    w->alpha -= 7;
                    if (w->alpha <= 0) {
                        w->alpha = 0;
                        return -1;
                    }
                    break;
            }
            break;
        case 3:
            if (w->alpha >= 0x100) w->alpha = 0xFF;
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0,
                              ENVIRONMENT, 0, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0,
                              0, ENVIRONMENT, 0);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->color.r, w->color.g, w->color.b, 0);
            gDPSetEnvColor(gGfxDlHead++, w->color2.r, w->color2.g, w->color2.b, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A spirit glow at (x, y, z) of a size, in two colours. */
void effectFspiritCreate(f32 x, f32 y, f32 z, f32 scale, u8 r, u8 g, u8 b, u8 r2, u8 g2, u8 b2) {
    SpiritInit init;

    init.pos[0] = x;
    init.pos[1] = y;
    init.pos[2] = z;
    init.color.r = r;
    init.color.g = g;
    init.color.b = b;
    init.color2.r = r2;
    init.color2.g = g2;
    init.color2.b = b2;
    init.scale = scale;
    tskCreate(gEffectTask, Fspirit, 0, &init);
}

/* The explosion chain handler. */
s32 pompom(Obj *obj, s16 msg, void *arg) {
    f32 x;
    f32 y;
    f32 z;
    f32 size;
    PompomInit *init = arg;
    PompomInit *w = FX(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "pompom";
            w->pos[0] = init->pos[0];
            w->pos[1] = init->pos[1];
            w->pos[2] = init->pos[2];
            w->spread[0] = init->spread[0];
            w->spread[1] = init->spread[1];
            w->spread[2] = init->spread[2];
            w->interval = init->interval;
            w->life = init->life;
            w->kind = init->kind;
            w->size = init->size;
            w->timer = w->interval;
            w->var = init->var;
            PRINTF("<-----ぽむぽむ爆発----->開始します\n");
            break;
        case 0:
            if (w->life-- <= 0) {
                PRINTF("<-----ぽむぽむ爆発----->終了します\n");
                return -1;
            }
            if (w->timer-- == 0) {
                w->timer = fadeRandRange(-w->interval / 3, w->interval / 3) + w->interval;
                x = w->pos[0] + fadeRandRange(-w->spread[0], w->spread[0]);
                y = w->pos[1] + fadeRandRange(-w->spread[1], w->spread[1]);
                z = w->pos[2] + fadeRandRange(-w->spread[2], w->spread[2]);
                if (w->var < w->size) {
                    size = fadeRandRange(-w->var, w->var) + w->size;
                } else {
                    size = w->size;
                }
                switch (w->kind) {
                    case 5:
                        effectBurst(x, y, z);
                        break;
                    case 6:
                        itexSparkle(x, y, z, size);
                        break;
                    case 7:
                        itexPuffYellow(x, y, z, size);
                        break;
                    case 8:
                        itexPuffGrey(x, y, z, size);
                        break;
                    case 9:
                        itexPuff(x, y, z, size);
                        break;
                    case 10:
                        itexSparkleYellow(x, y, z, size);
                        break;
                    case 11:
                        itexFlash(x, y, z, size);
                        break;
                    case 12:
                        itexGlowBlue(x, y, z, size);
                        break;
                    case 13:
                        itexSparkleBlue(x, y, z, size);
                        break;
                    case 14:
                        itexPuffWhite(x, y, z, size);
                        break;
                }
            }
            break;
        case 1:
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A chain of explosions ("pompom") of a kind around (x, y, z), within spread (sx, sy, sz):
 * one every interval steps, for life steps, of size give or take var. */
Obj *effectPompomCreate(f32 x, f32 y, f32 z, f32 sx, f32 sy, f32 sz, s32 kind, s32 life,
                            s32 interval, f32 size, f32 var) {
    PompomInit init;

    init.pos[0] = x;
    init.pos[1] = y;
    init.pos[2] = z;
    init.spread[0] = sx;
    init.spread[1] = sy;
    init.spread[2] = sz;
    init.size = size;
    init.interval = interval;
    init.life = life;
    init.kind = kind;
    init.var = var;
    return tskCreate(gEffectTask, pompom, 0, &init);
}

/* Moves a live explosion chain to (x, y, z). */
s32 effectPompomSetPos(Obj *obj, f32 x, f32 y, f32 z) {
    f32 *pos = FX(obj);

    if (!(obj->flags & 1)) {
        PRINTF("<pompom> Caution: このタスクは既に消去されています\n");
        return FALSE;
    }
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;
    return TRUE;
}

/* A star at (x, y, z). */
void effectStarCreate(f32 x, f32 y, f32 z) {
    StarInit init;

    init.pos[0] = x;
    init.pos[1] = y;
    init.pos[2] = z;
    tskCreate(gEffectTask, effectStarProc, 0, &init);
}

/* The star handler: a star of a random size and turn that shrinks and fades, twinkling in a
 * random colour. */
s32 effectStarProc(Obj *obj, s16 msg, void *arg) {
    StarWork *w = FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Star";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = effectStarDl;
            floadGetSetRam((void **) &w->frame->segs[0], 0x124);
            w->frame->flags |= 0x22;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                fadeRandRange(3, 17) / 10.0f * 0.4f;
            w->frame->pos[0] = ((StarInit *) arg)->pos[0];
            w->frame->pos[1] = ((StarInit *) arg)->pos[1];
            w->frame->pos[2] = ((StarInit *) arg)->pos[2];
            w->frame->rot[2] = fadeRand(360) * 0.0174532930038800785;
            w->alpha = 0xA0;
            break;
        case 0:
            w->frame->scale[0] *= 0.9f;
            w->frame->scale[1] *= 0.9f;
            w->frame->scale[2] *= 0.9f;
            w->alpha -= 10;
            if (w->alpha < 0) {
                return -1;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, gKiranSetupDl);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 255, 255);
            gDPSetEnvColor(gGfxDlHead++, (*fadeRand)(255),
                           (*fadeRand)(255), (*fadeRand)(255),
                           w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The splash handler. The three rings (file 0x127) share one draw list: env colour with the
 * alpha, prim colour. */
s32 sibuki(Obj *obj, s16 msg, void *arg) {
    SibukiWork *w = FX(obj);

    switch (msg) {
        case 2:
            obj->name = "sibuki";
            w->ring[0] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->ring[0] == NULL) {
                return -1;
            }
            w->ring[1] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->ring[1] == NULL) {
                return -1;
            }
            w->ring[2] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->ring[2] == NULL) {
                return -1;
            }
            floadGetSetRam((void **) &w->ring[0]->segs[0], 0x127);
            floadGetSetRam((void **) &w->ring[1]->segs[0], 0x127);
            floadGetSetRam((void **) &w->ring[2]->segs[0], 0x127);
            w->ring[0]->dl2 = sibukiRing1Dl;
            w->ring[1]->dl2 = sibukiRing2Dl;
            w->ring[2]->dl2 = sibukiRing3Dl;
            w->ring[0]->rot[1] = fadeRand(360) / 180.0f * 3.1415927f;
            w->ring[1]->rot[1] = fadeRand(360) / 180.0f * 3.1415927f;
            w->ring[2]->rot[1] = fadeRand(360) / 180.0f * 3.1415927f;
            w->alpha = 0xFF;
            w->state = 0;
            break;
        case 1:
            frmDestroy(&w->ring[0]->f);
            frmDestroy(&w->ring[1]->f);
            frmDestroy(&w->ring[2]->f);
            break;
        case 3:
            w->ring[0]->dl = gGfxDlHead;
            w->ring[1]->dl = gGfxDlHead;
            w->ring[2]->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, gDrawSetupPrimEnvTexBlendDl);
            gDPSetEnvColor(gGfxDlHead++, w->color.r, w->color.g, w->color.b, w->alpha);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->color2.r, w->color2.g, w->color2.b, 0);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->ring[0]->scale[2] += w->widenRise;
                    w->ring[0]->scale[0] = w->ring[0]->scale[2];
                    w->ring[1]->scale[2] += w->widenRise;
                    w->ring[1]->scale[0] = w->ring[1]->scale[2];
                    w->ring[2]->scale[2] += w->widenRise;
                    w->ring[2]->scale[0] = w->ring[2]->scale[2];
                    w->ring[0]->scale[1] += w->rise + w->rise;
                    w->ring[1]->scale[1] += 1.3f * w->rise;
                    w->ring[2]->scale[1] += w->rise;
                    if (w->time-- <= 0) {
                        w->state++;
                    }
                    break;
                case 1:
                    w->ring[0]->scale[2] += w->widenFall;
                    w->ring[0]->scale[0] = w->ring[0]->scale[2];
                    w->ring[1]->scale[2] += w->widenFall;
                    w->ring[1]->scale[0] = w->ring[1]->scale[2];
                    w->ring[2]->scale[2] += w->widenFall;
                    w->ring[2]->scale[0] = w->ring[2]->scale[2];
                    w->ring[0]->scale[1] -= 1.5f * w->fall;
                    w->ring[1]->scale[1] -= 1.2f * w->fall;
                    w->ring[2]->scale[1] -= w->fall;
                    if (w->ring[2]->scale[1] < 0.0001f) {
                        w->ring[2]->scale[1] = 0.0001f;
                    }
                    break;
            }
            w->alpha -= 8;
            if (w->alpha < 0) {
                return -1;
            }
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* A white splash: size, growth and two rates, twice each. */
void effectSibukiCreateWhite(f32 x, f32 y, f32 z, f32 size, f32 grow, f32 a, f32 b) {
    effectSibukiCreate(x, y, z, size, grow, a, a, b, b, 0xFF, 0xFF, 0xFF, 0, 0, 0xFF);
}

/* A splash at (x, y, z): three rings of size, 1.5 and 2 times it, in two colours.
 * Each ring's width goes through the local s first, the same way for all three rings: the
 * extra statements are what keep the ring loads and the four rate stores based on obj, with
 * w in its own register only for the colours and the time, as the original does. Without
 * them IDO bases the first ring load and the rate stores on w. */
Obj *effectSibukiCreate(f32 x, f32 y, f32 z, f32 size, f32 grow, f32 a, f32 b, f32 c,
                            f32 d, u8 r, u8 g, u8 bl, u8 r2, u8 g2, u8 b2) {
    Obj *obj = tskCreate(gEffectTask, sibuki, 0, NULL);

    if (obj) {
        SibukiWork *w = FX(obj);
        f32 s;

        w->ring[0]->pos[0] = w->ring[1]->pos[0] = w->ring[2]->pos[0] = x;
        w->ring[0]->pos[1] = w->ring[1]->pos[1] = w->ring[2]->pos[1] = y;
        w->ring[0]->pos[2] = w->ring[1]->pos[2] = w->ring[2]->pos[2] = z;
        s = size;
        w->ring[0]->scale[2] = s;
        w->ring[0]->scale[0] = s;
        w->ring[0]->scale[1] = grow;
        s = 1.5f * size;
        w->ring[1]->scale[2] = s;
        w->ring[1]->scale[0] = s;
        w->ring[1]->scale[1] = grow;
        s = 2.0f * size;
        w->ring[2]->scale[2] = s;
        w->ring[2]->scale[0] = s;
        w->ring[2]->scale[1] = grow;
        w->widenRise = a;
        w->widenFall = b;
        w->rise = c;
        w->fall = d;
        w->color.r = r;
        w->color.g = g;
        w->color.b = bl;
        w->color2.r = r2;
        w->color2.g = g2;
        w->color2.b = b2;
        w->time = 10;
    }
    return obj;
}
