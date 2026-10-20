#include "tlb.h"
#include "tlb/tlb_0017.h"

/* Draws a screen-sized RGBA16 picture in 64x30 tiles, darkened to black by alpha. Each tile's
 * left and top are set in the inner loop and its other edges written from them, as the game has. */
Gfx *effGhostDrawImage(Gfx *gfx, void *img, s32 alpha) {
    s32 i;
    s32 j;
    s32 top;
    s32 left;

    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 0, 0, 0, alpha);
    for (j = 0; j < 8; j++) {
        for (i = 0; i < 5; i++) {
            left = i * 64;
            top = j * 30;
            gDPPipeSync(gfx++);
            gDPLoadTextureTile(gfx++, img, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, SCREEN_HEIGHT, left,
                               top, left + 63, top + 29, 0, G_TX_CLAMP, G_TX_CLAMP,
                               G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gSPTextureRectangle(gfx++, left << 2, top << 2, (left + 64) << 2, (top + 30) << 2,
                                G_TX_RENDERTILE, left << 5, top << 5, 1 << 10, 1 << 10);
        }
    }
    return gfx;
}

extern Gfx effItexQuadDl[]; /* the itex quad, in file 0x125 */

/* This file's "itex": a "pol" frame stuck to its parent task at an offset, its texture stepping
 * through the timed yellow animation (file 0x125), fading as it rises. */
typedef struct ItexWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ Anim anim;
    /* 0x14 */ s32 alpha;
    /* 0x18 */ s32 fade; /* added to alpha each step; gone at 0 */
    /* 0x1C */ f32 ofs[3]; /* from the parent's position */
    /* 0x28 */ f32 rise; /* added to ofs[2] each step */
} ItexWork;

#define ITEX(obj) ((ItexWork *) FX(obj))

s32 effItexProc(Obj *obj, s16 msg, void *arg) {
    ItexWork *w = ITEX(obj);
    s32 pad[2];
    f32 pos[3];

    switch (msg) {
        case 2:
            obj->name = "itex";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = effItexQuadDl;
            floadGetSetRam((void **) w->frame->segs, 0x125);
            fadeAnimStart(&w->anim, gItexTimedYellowAnim, 0, &w->frame->segs[3],
                          floadGetSetFile(0x125, 0));
            w->frame->flags |= 7;
            w->frame->scale[2] = 0.3f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->alpha = 255;
            w->ofs[0] = 0.0f;
            w->ofs[1] = 0.0f;
            w->ofs[2] = 0.0f;
            w->rise = 0.0f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->alpha += w->fade;
            if (w->alpha <= 0) {
                return -1;
            }
            fadeAnimStep(&w->anim);
            pos[2] = 0.0f;
            pos[1] = 0.0f;
            pos[0] = 0.0f;
            tskSendMsg(obj->parent, 4, pos);
            w->frame->pos[0] = pos[0] + w->ofs[0];
            w->frame->pos[1] = pos[1] + w->ofs[1];
            w->frame->pos[2] = pos[2] + w->ofs[2];
            w->ofs[2] += w->rise;
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombine(gGfxDlHead++, 0x309A61, 0x5536FF7F);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 128, 0);
            gDPSetEnvColor(gGfxDlHead++, 255, 40, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* An itex at (x, y, z) from the parent. */
Obj *effItexMake(Obj *parent, f32 x, f32 y, f32 z) {
    Obj *o;

    if ((o = tskCreate(parent, effItexProc, parent->file, NULL)) != NULL) {
        ITEX(o)->ofs[0] = x;
        ITEX(o)->ofs[1] = y;
        ITEX(o)->ofs[2] = z;
        ITEX(o)->frame->scale[2] = 0.4f;
        ITEX(o)->frame->scale[0] = ITEX(o)->frame->scale[1] = ITEX(o)->frame->scale[2];
    }
    return o;
}

void effItexSetFade(Obj *obj, f32 fade) {
    ITEX(obj)->fade = fade;
}

/* An itex at (x, y, z) from the parent, fading and sinking. */
Obj *effItexMakeSinking(Obj *parent, f32 x, f32 y, f32 z) {
    Obj *o;

    if ((o = tskCreate(parent, effItexProc, parent->file, NULL)) != NULL) {
        ITEX(o)->ofs[0] = x;
        ITEX(o)->ofs[1] = y;
        ITEX(o)->ofs[2] = z;
        ITEX(o)->fade = -7;
        ITEX(o)->rise = -1.0f;
    }
    return o;
}

extern u8 effJetimpactAnim1[]; /* the jet impact's two animations, in file 0x13A */
extern u8 effJetimpactAnim2[];

/* "jetimpact": an "anime" frame moving at a set speed towards a target, its size moving too,
 * its animation playing to an end; while it has itexs at its sides it drops a sinking itex
 * from each every 8 steps. */
typedef struct JetimpactWork {
    /* 0x00 */ s32 kind; /* the animation (0 or not); how low the itexs sit */
    /* 0x04 */ AnimeFrame *frame;
    /* 0x08 */ f32 steps; /* how many steps a move takes */
    /* 0x0C */ f32 target[3];
    /* 0x18 */ f32 vel[3]; /* per step, on each axis */
    /* 0x24 */ f32 dir; /* the last move's heading */
    /* 0x28 */ f32 scale;
    /* 0x2C */ f32 scaleStep;
    /* 0x30 */ f32 scaleTarget;
    /* 0x34 */ u16 tick;
    /* 0x38 */ Obj *left; /* the itexs at its sides */
    /* 0x3C */ Obj *right;
    /* 0x40 */ f32 frameEnd;
    /* 0x44 */ f32 frameStep;
} JetimpactWork;

#define JETIMPACT(obj) ((JetimpactWork *) FX(obj))

s32 effJetimpactProc(Obj *obj, s16 msg, void *arg) {
    JetimpactWork *w = JETIMPACT(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "jetimpact";
            w->kind = (s32) arg;
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            if (w->kind == 0) {
                w->frame->anim = effJetimpactAnim1;
            } else {
                w->frame->anim = effJetimpactAnim2;
            }
            floadGetSetRam((void **) w->frame->segs, 0x13A);
            w->scale = 1.0f;
            w->frame->rot[0] = -1.5707964f;
            w->frameStep = 0.0f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 4:
            if (arg != NULL) {
                ((f32 *) arg)[0] = w->frame->pos[0];
                ((f32 *) arg)[1] = w->frame->pos[1];
                ((f32 *) arg)[2] = w->frame->pos[2];
            }
            break;
        case 0:
            w->tick++;
            if ((w->tick & 7) == 1) {
                if (w->left != NULL) {
                    effItexMakeSinking(obj, 15.0f, 0.0f, -75 - w->kind * 15);
                }
                if (w->right != NULL) {
                    effItexMakeSinking(obj, -15.0f, 0.0f, -75 - w->kind * 15);
                }
            }
            if (0.0001f < w->vel[0]) {
                if (fabsf(w->target[0] - w->frame->pos[0]) < w->vel[0]) {
                    w->frame->pos[0] = w->target[0];
                    w->vel[0] = 0.0f;
                } else if (w->target[0] < w->frame->pos[0]) {
                    w->frame->pos[0] -= w->vel[0];
                } else {
                    w->frame->pos[0] += w->vel[0];
                }
            }
            if (0.0001f < w->vel[1]) {
                if (fabsf(w->target[1] - w->frame->pos[1]) < w->vel[1]) {
                    w->frame->pos[1] = w->target[1];
                    w->vel[1] = 0.0f;
                } else if (w->target[1] < w->frame->pos[1]) {
                    w->frame->pos[1] -= w->vel[1];
                } else {
                    w->frame->pos[1] += w->vel[1];
                }
            }
            if (0.0001f < w->vel[2]) {
                if (fabsf(w->target[2] - w->frame->pos[2]) < w->vel[2]) {
                    w->frame->pos[2] = w->target[2];
                    w->vel[2] = 0.0f;
                } else if (w->target[2] < w->frame->pos[2]) {
                    w->frame->pos[2] -= w->vel[2];
                } else {
                    w->frame->pos[2] += w->vel[2];
                }
            }
            if (0.00001f < w->scaleStep) {
                if (fabsf(w->scaleTarget - w->scale) < w->scaleStep) {
                    w->scale = w->scaleTarget;
                    w->scaleStep = 0.0f;
                } else if (w->scaleTarget < w->scale) {
                    w->scale -= w->scaleStep;
                } else {
                    w->scale += w->scaleStep;
                }
            }
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale;
            w->frame->frame += w->frameStep;
            if (w->frameEnd < w->frame->frame) {
                w->frame->frame = w->frameEnd;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A jet impact of a kind at (x, y, z). */
Obj *effJetimpactMake(Obj *parent, s32 kind, f32 x, f32 y, f32 z) {
    Obj *o;

    if ((o = tskCreate(parent, effJetimpactProc, parent->file, (void *) kind)) != NULL) {
        JETIMPACT(o)->frame->pos[0] = x;
        JETIMPACT(o)->frame->pos[1] = y;
        JETIMPACT(o)->frame->pos[2] = z;
    }
    return o;
}

/* Puts an itex at each side. */
void effJetimpactAddItexs(Obj *obj) {
    JetimpactWork *w = JETIMPACT(obj);
    s32 pad;

    w->left = effItexMake(obj, 15.0f, 0.0f, -75 - w->kind * 15);
    w->right = effItexMake(obj, -15.0f, 0.0f, -75 - w->kind * 15);
}

/* Lets the side itexs fade out. */
void effJetimpactFadeItexs(Obj *obj) {
    JetimpactWork *w = JETIMPACT(obj);
    s32 pad;

    if (w->left != NULL) {
        effItexSetFade(w->left, -7.0f);
        w->left = NULL;
    }
    if (w->right != NULL) {
        effItexSetFade(w->right, -7.0f);
        w->right = NULL;
    }
}

void effJetimpactSetSteps(Obj *obj, s32 steps) {
    JETIMPACT(obj)->steps = steps;
}

void effJetimpactGetPos(Obj *obj, f32 *x, f32 *y, f32 *z) {
    *x = JETIMPACT(obj)->frame->pos[0];
    *y = JETIMPACT(obj)->frame->pos[1];
    *z = JETIMPACT(obj)->frame->pos[2];
}

void effJetimpactSetPos(Obj *obj, f32 x, f32 y, f32 z) {
    JETIMPACT(obj)->frame->pos[0] = x;
    JETIMPACT(obj)->frame->pos[1] = y;
    JETIMPACT(obj)->frame->pos[2] = z;
}

/* The move's target, reached in its steps on each axis; it heads that way. */
void effJetimpactMoveTo(Obj *obj, f32 x, f32 y, f32 z) {
    JetimpactWork *w = JETIMPACT(obj);

    w->target[0] = x;
    w->target[1] = y;
    w->target[2] = z;
    w->vel[0] = fabsf(x - w->frame->pos[0]) / w->steps;
    w->vel[1] = fabsf(y - w->frame->pos[1]) / w->steps;
    w->vel[2] = fabsf(z - w->frame->pos[2]) / w->steps;
    w->dir = mathAtan2(x - w->frame->pos[0], z - w->frame->pos[2]);
}

void effJetimpactStop(Obj *obj) {
    JETIMPACT(obj)->vel[2] = 0.0f;
    JETIMPACT(obj)->vel[1] = 0.0f;
    JETIMPACT(obj)->vel[0] = 0.0f;
}

void effJetimpactSetScale(Obj *obj, f32 scale) {
    JETIMPACT(obj)->scale = scale;
}

/* The size's target, reached in the move's steps. */
void effJetimpactScaleTo(Obj *obj, f32 scale) {
    JetimpactWork *w = JETIMPACT(obj);

    w->scaleTarget = scale;
    w->scaleStep = fabsf(scale - w->scale) / w->steps;
}

/* Plays the animation from start to end in the move's steps. */
void effJetimpactSetPlay(Obj *obj, f32 start, f32 end) {
    JetimpactWork *w = JETIMPACT(obj);

    w->frame->frame = start;
    w->frameEnd = end;
    w->frameStep = (end - start) / w->steps;
}

void effJetimpactSetTilt(Obj *obj, f32 tilt) {
    JETIMPACT(obj)->frame->rot[0] = tilt;
}

void effJetimpactHide(Obj *obj) {
    JETIMPACT(obj)->frame->f.flags &= ~2;
}

void effJetimpactShow(Obj *obj) {
    JETIMPACT(obj)->frame->f.flags |= 2;
}

extern Gfx effDrawSetupDl[]; /* the effects' draw setup (the kirari's, the tube's, ...) */
extern Gfx effKirariDl[]; /* the kirari, in file 0x13A */

/* "kirari": a sparkle, a "pol" frame whose size moves to a target in a set number of steps. */
typedef struct KirariWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 steps;
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 scaleStep;
    /* 0x10 */ f32 scaleTarget;
} KirariWork;

#define KIRARI(obj) ((KirariWork *) FX(obj))

s32 effKirariProc(Obj *obj, s16 msg, void *arg) {
    KirariWork *w = KIRARI(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "kirari";
            w->frame = (PolFrame *) frmCreate(gFrmEffectLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl = (Gfx *) fadeSegToRam(effDrawSetupDl, obj->file);
            w->frame->dl2 = effKirariDl;
            floadGetSetRam((void **) w->frame->segs, 0x13A);
            w->frame->flags |= 7;
            w->steps = 8.0f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (0.00001f < w->scaleStep) {
                if (fabsf(w->scaleTarget - w->scale) < w->scaleStep) {
                    w->scale = w->scaleTarget;
                    w->scaleStep = 0.0f;
                } else if (w->scaleTarget < w->scale) {
                    w->scale -= w->scaleStep;
                } else {
                    w->scale += w->scaleStep;
                }
            }
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "kirari" sparkle at (x, y, z). */
Obj *effKirariMake(Obj *parent, f32 x, f32 y, f32 z) {
    Obj *o;

    if ((o = tskCreate(parent, effKirariProc, parent->file, NULL)) != NULL) {
        KIRARI(o)->frame->pos[0] = x;
        KIRARI(o)->frame->pos[1] = y;
        KIRARI(o)->frame->pos[2] = z;
    }
    return o;
}

void effKirariSetSteps(Obj *obj, f32 steps) {
    KIRARI(obj)->steps = steps;
}

void effKirariSetScale(Obj *obj, f32 scale) {
    KIRARI(obj)->scale = scale;
}

/* The size's target, reached in the steps. */
void effKirariScaleTo(Obj *obj, f32 scale) {
    KirariWork *w = KIRARI(obj);

    w->scaleTarget = scale;
    w->scaleStep = fabsf(scale - w->scale) / w->steps;
}

extern Gfx effShowerDl[]; /* the shower's draw setup */
extern Gfx effShowerModelDl[]; /* the shower, in file 0x13A */

/* "shower": a "pol" frame turning a degree a step, its height moving to a target in a set
 * number of steps, its texture scrolling. */
typedef struct ShowerWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 steps;
    /* 0x08 */ f32 scale; /* the frame's scale[2] */
    /* 0x0C */ f32 scaleStep;
    /* 0x10 */ f32 scaleTarget;
    /* 0x14 */ s32 scroll; /* the texture's */
    /* 0x18 */ s32 scrollStep;
} ShowerWork;

#define SHOWER(obj) ((ShowerWork *) FX(obj))

s32 effShowerProc(Obj *obj, s16 msg, void *arg) {
    ShowerWork *w = SHOWER(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "shower";
            w->frame = (PolFrame *) frmCreate(gFrmEffectLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = effShowerModelDl;
            floadGetSetRam((void **) w->frame->segs, 0x13A);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (0.00001f < w->scaleStep) {
                if (fabsf(w->scaleTarget - w->scale) < w->scaleStep) {
                    w->scale = w->scaleTarget;
                    w->scaleStep = 0.0f;
                } else if (w->scaleTarget < w->scale) {
                    w->scale -= w->scaleStep;
                } else {
                    w->scale += w->scaleStep;
                }
            }
            w->frame->scale[2] = w->scale;
            w->frame->rot[2] -= 0.01745329238f;
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, fadeSegToRam(effShowerDl, obj->file));
            gGfxDlHead = effShowerLoadTex(gGfxDlHead, w->scroll);
            gSPEndDisplayList(gGfxDlHead++);
            w->scroll += w->scrollStep;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A shower at (x, y, z), turned to dir. */
Obj *effShowerMake(Obj *parent, f32 x, f32 y, f32 z, f32 dir) {
    Obj *o;

    if ((o = tskCreate(parent, effShowerProc, parent->file, NULL)) != NULL) {
        SHOWER(o)->frame->pos[0] = x;
        SHOWER(o)->frame->pos[1] = y;
        SHOWER(o)->frame->pos[2] = z;
        SHOWER(o)->frame->rot[1] = dir;
    }
    return o;
}

void effShowerSetSteps(Obj *obj, f32 steps) {
    SHOWER(obj)->steps = steps;
}

void effShowerSetScale(Obj *obj, f32 x, f32 y, f32 z) {
    SHOWER(obj)->scale = z;
    SHOWER(obj)->frame->scale[0] = x;
    SHOWER(obj)->frame->scale[1] = y;
    SHOWER(obj)->frame->scale[2] = z;
}

/* The height's target, reached in the steps. */
void effShowerScaleTo(Obj *obj, f32 scale) {
    ShowerWork *w = SHOWER(obj);

    w->scaleTarget = scale;
    w->scaleStep = fabsf(scale - w->scale) / w->steps;
}

void effShowerSetScroll(Obj *obj, s32 step) {
    SHOWER(obj)->scrollStep = step;
}

/* "lensflare": an "anime" frame (file 0x13B) set on the line from the camera through its
 * position, turned to face the camera, its alpha moving by a step. */
typedef struct LensflareWork {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ s32 alpha;
    /* 0x08 */ s32 alphaStep;
} LensflareWork;

#define LENSFLARE(obj) ((LensflareWork *) FX(obj))

extern u8 effLensflareAnim[]; /* the lens flare's animation, in file 0x13B */

/* Each step it sits where the view line meets the plane halfway between the eye and its own
 * position (square to that line), faces the eye, and dims the screen by how straight the eye
 * looks at it.
 * MATCHED 2026-10-16; the history below is how. Earlier: NON_MATCHING: 405 lines,
 * mix 5, the frame's slots now all the game's but its 0x44 (the
 * mathAtan2 address the game spills). The game's t-registers start at t3 (the switch's lh) and
 * it spills the frame's position loads (0x34-0x3C) in another order. Measured: d[3] as an array
 * with one dot-product local (435, mix 9, f20 saved); the dot product written twice (480, mix
 * 18: not shared across the calls while d is an array); dx/dy/dz scalars with the dot written
 * twice (465, mix 5, the game's 0x40 temp); pad[2] first (445), pad[2] last (405).
 * 2026-10-12, probes (this C copied, one part cut per variant):
 * the switch's first temporary moves with the rest of the function (no case 3: t1; no case 2:
 * t8; no math: t4; this C t6), so ugen's rotation reaches the dispatch after the case bodies
 * and the game's t3 says some body spends a different number of temporaries. One is case 3:
 * the game loads obj into s0 there (w's register), reads frame/alpha at 0x20/0x24(s0) and ends
 * with "addiu s0,s0,0x20"; this C reads obj into a temporary. A block boundary after case 3's
 * "w = LENSFLARE(obj);" (a goto label, or a FAKE "if (w) {}") gives the game's s0 form there
 * (and moves the first temporary to t5). In this file, the FAKE "if (w) {}" there: 222 to 202,
 * mix 5 to 3 (kept, FAKE). Left in case 3: the game fills the dispatch's delay slot with
 * gGfxDlHead's lui and loads s0 in the body; here s0's load takes the slot.
 * 2026-10-12, from 202: w set at its declaration ("LensflareWork *w =
 * LENSFLARE(obj);", one variable for every case, as endPlCtlProc's match) and the three per-case
 * assignments and the FAKE gone: 164; then pad[2] gone (the frame was 8 over): 137, frame 0xA8,
 * cases 1-3 and dx/dy/dz's homes (0x88-0x90) the game's. Left: case 0's float colouring and spills
 * (the game: ex f0, ey f2, ez f16; frame->pos[0..2] spilled to 0x3C/0x34/0x38 as soon as loaded;
 * vy/vz homes 0x74/0x70, k 0x6C; this C keeps pos[1], pos[2] in registers). The positions as
 * locals px/py/pz: 231 (frame 0xB8, kept in f16/f18/f0). Taken back.
 * 2026-10-12, from 137: the permuter (old base, ~15 min) found "if (1) { vy = .. }":
 * 139 (length right, registers not). Probes (the whole proc, one
 * change each): cutting the v.v length or k's position uses changes nothing; with no math calls
 * pos[2] spills too; `register` on vx/vy/vz nothing; an unreferenced label anywhere nothing. A
 * block boundary (a PRINTF or "do {} while (0);") right BEFORE vx gives the game's ex f0, ey
 * f2, ez f16, vx f18, vy f12 and the three position loads spilled as soon as loaded. In this
 * file: 137 to 133 (frame 0xA0), with pad[2] back 94, mix 0, length 0, every local's slot the
 * game's (kept, FAKE). The game's .rodata has no string after "lensflare", so it was no print.
 * The boundary before dx instead: 164; after vz: 154; vx-vz inside "if (1) {}": 154. Left
 * (94): from case 0's first float temporary on, ugen's f4/f6/f10 one step round (dx is f6 in the
 * game, f10 here), and the spill words: game pos[1] 0x34, pos[2] 0x38, pos[0] 0x3C, dot 0x40,
 * mathAtan2 0x44; here 0x30, 0x38, 0x34, 0x3C, 0x38 (one word fewer).
 * The permuter from 94 (~25 min): "q[1] = dy; q[1] = q[1] * t + ey - pos[1];" 34, length +2
 * (a store and reload of dy the game lacks), but every ugen float register from dx to q[0] then
 * the game's: so ugen's choices in that block depend on q[1]'s form later in it. Left with
 * it: k's two adds' operand order, c in f2 (game f0), the spill words. Not kept (length);
 * "(c * 2 - 1.0f)" written twice in fadeSetLevel: 94, no change.
 * Its other find: t's divisor as "vx * dx + vy * dy + vz * (0, dz)": 94 to 18 (kept, FAKE;
 * every ugen register through q[0] the game's). Not it: (pos[2] + ez) in k for (ez + pos[2]):
 * the same code; dx, dy, dz as an array d[3]: 220; "dz * vz" in the divisor: 94. The same FAKE in opLensflareProc: 145 to
 * 149. Left (18): the position spill words (game 0x3C/0x34/0x38, here 0x34/0x30/0x38), k's
 * z term "add.s f6,f16,f10" (here f10,f16) and two mul operand orders, the dot's spill 0x40
 * (here 0x3C), mathAtan2's 0x44 (here 0x38), c in f2 (game f0).
 * The permuter from 18: the fade level as "(c * 2 - 1.0f) * (c * 2 - 1.0f) * 0.8 * alpha" (no
 * store back to c): c in f0 as the game, 16 (kept; natural C). A third run (40 min) from 16
 * found only k merged into c: 18 (the game has k's own home, 0x6C). Left (16): the spill area
 * is one word short at its bottom (game unused 0x2C-0x30, here 0x2C), so the position words sit
 * a word low and in another order; k's z term's add and two muls take the other operand order
 * (the second one is the FAKE (0, dz)'s doing).
 * 2026-10-12: k's first term as "vx * (ex + pos[0])": 18 (term 1 was already the game's);
 * terms 2 and 3 in term 1's form "(pos + e) * v": 18 (term 2 then differs). The permuter from
 * this C, 40 minutes (~3200 iterations): nothing below its base.
 * 2026-10-12 (16, taken back): the frame test as "(w->frame = ..) == NULL" (an
 *   assignment in a test, a pointer-typed front-end word): no change. pad[2] gone: frame 0xA0,
 *   the spill words stay at 0x30-0x3C and the locals go 8 low (58): the spill words are counted
 *   from the bottom, so the game's extra one sits below them (0x2C-0x30 unused there, 0x2C here).
 *   An operand-order lever ("(f32) var + expr" puts the var first) on k's z term:
 *   "((f32) ez + pos[2])" 16, and with "(f32) vz *" too 16: no change.
 * 2026-10-12, match_function.py trace (16): the three position loads are uopt CSE
 *   ranges (uilod, 239/242/245) left uncoloured, so ugen stores each as loaded and reloads it.
 *   Their words here: pos[1] 0x30, pos[0] 0x34, pos[2] 0x38, the dot 0x3C, mathAtan2's address
 *   0x38 again (pos[2]'s word, free by then). The game: pos[1] 0x34, pos[2] 0x38, pos[0] 0x3C,
 *   the dot 0x40, mathAtan2 0x44: five words, none shared, all one word higher.
 * 2026-10-15 (16 to 14, kept): the words under the homes are uopt's spill slots
 *   (uopt.c f_spilltemps, 148313: expression bits in order, each takes the first slot of its
 *   size that no interfering temp holds, else a new one at the frame counter going down; it
 *   runs before globalcolor, so the work pointer's expression (bit 4, s0) holds slot 0 though
 *   never written). The homes were one word too many: cfe's homes end at -100 (-96 the switch's
 *   int word, -100 the scale chain's f32 word, -80..-92 the four gbi macros' _g block locals);
 *   pad[2] as one word: every slot one word up, the dot at the game's 0x40 and pos[1] at its
 *   0x34 (14). Left: pos[0] and pos[2] in each other's slot (game 0x3C/0x38) and mathAtan2's
 *   address sharing pos[2]'s slot (0x3C) where the game has it alone in slot 0 (0x44, with the
 *   work pointer's never-written slot): so in the game that address does not interfere with the
 *   work pointer's temp (first-fit shares slot 0), and pos[0]'s bit or interference puts it
 *   ahead of pos[2]. Not found which C form gives that.
 *   Measured 2026-10-15 with the patched uopt (vendor/uopt-spill, PROC 31: homes 96, frame
 *   112): w's uadd (bit 4) slot 0 -100 (0x44, empty); the dot's sum (118, blocks 19-23) and the
 *   w->frame load (221) slot 1 -104 (0x40); bit 139 (a ucvt, blocks 20-22) slot 2 -108 (0x3C);
 *   the pos load 239 slot 3 -112 (0x38). Bit 4 is in every block's eeavout (+0x15c), so 139
 *   cannot share slot 0. The game's frame is exactly this with 139 NOT interfering with w:
 *   139 in slot 0 (0x44, the game's mathAtan2 word), the dot 0x40, the pos load 0x3C (the
 *   game's pos[0]); pos[2] 0x38 and pos[1] 0x34 then follow. So, as in ihandpointProc (same
 *   reading, see its note), the game's work-pointer expression is absent from the eeavout of
 *   the blocks holding the call address.
 *   The scale chain written "[0] = [1] = [2] = 1.0f": the same code (16 then), no slot moved.
 * 2026-10-16 (bit order lever, ihandpointProc's/gimIheadProc's match; all taken back,
 *   start 14). Spill print (PROC 31): mathAtan2's address is ucvt bit 139, live in BB 20-22 with
 *   w's uadd (bit 4) and the dot sum (124, uadd{82}{1172}); the pos loads are uilod ranges 242
 *   (pos[0], kind-4 saved, slot 3 0x38), 245, 248 (uncoloured, no uopt slot; pos[2] lands in
 *   mathAtan2's slot, pos[1] at 0x34 below the counter). Tries: "c = (s32) mathAtan2;" (a dead
 *   store in an existing home) before "w = LENSFLARE(obj);": 14, mathAtan2 slot 0 = 0x44 (the
 *   game's), w slot 1 0x40 empty, the dot 0x3C, pos[0] 0x38, pos[2] 0x44: every other word one
 *   off. The same store after w (control): bit 4 then 8, mathAtan2 slot 1, 16. A second def of w
 *   in case 0 ("w = LENSFLARE(obj)", or "w = obj; w = LENSFLARE(w)"): folded, no change (14).
 *   w = obj at the top and "w = LENSFLARE(w)" in cases 2/0/3 (no saved uadd): slots exactly the
 *   game's (mathAtan2 0x44, dot 0x40, 0x3C) but the pos loads coloured, 185, length -2. "if
 *   (!&obj) {}" at the top (diagnostic): mathAtan2 SHARES w's slot 0 (bit 139 with bit 4), the
 *   game's reading, but obj in memory, 258, length -7. Why ordering cannot fit: mathAtan2
 *   interferes with the dot (BB 20-22) and w interferes with every saved bit, so with w saved
 *   and four slots (the frame) w must share a slot, i.e. not be in BB 20-22's eeavout. So here
 *   the lever is a kill of uadd(obj, 32) across the calls (obj altered or aliased), not bit
 *   order; ihandpointProc was ordering (its frame has the spare word for w).
 * 2026-10-16 (w's expression cut): all taken back, start 14. Bits now: w 4, dot 118,
 *   mathAtan2 139 (BB 17-19), w->frame 221, pos[0] 239. Tries (spill print each): "obj = (Obj
 *   *) (u32) obj;" before the first atan2: removed, 14, no change. FAKE "obj = (Obj *) ((u32) w
 *   - sizeof(Obj));" before the first atan2: bit 4 leaves BB 17+ and 139 SHARES slot 0 (the
 *   game's 0x44), but w then lives from the top (bit 4 in BB 0-3) and pos[0] is no longer
 *   saved: frame 0xA0, 268. The same after the first atan2 statement: bit 4 leaves only from BB
 *   18 on (so the kill needs to sit before the store of rot[1]), 282, s1 used. So a kill only
 *   moves bit 4 out of the blocks downstream of it; with uses after each call, the sharing
 *   needs a kill in each of 17-19, i.e. obj killed by the calls themselves (the "&obj"
 *   reading). w declared bare, "w = LENSFLARE(obj);" at the head of cases 2/0/3: 262 (pos[0]
 *   not saved, bit 4 still in 17-19); the same plus "if (!&obj) {}": 272 (no w slot, mathAtan2
 *   slot 1). LENSFLARE(obj)-> in the three atan2/rotY statements (w kept elsewhere): same sets,
 *   233 (s1 used). Case 0 in a block with its own "LensflareWork *w = LENSFLARE(obj);" (pad
 *   gone, homes 96): 241, pos[0] not saved. "Obj *self = obj;" in pad's place, "do {} while
 *   (0);", "w = LENSFLARE(self);": w's bit late (223, copy propagation) but still below the pos
 *   load (243), dot slot 0: 234. w = (LensflareWork *) obj at the top, "w = LENSFLARE(w);" at the
 *   head of cases 2/0/3 (the 185 form, case 2's after obj->name): 102, slots pos 0, dot 1,
 *   mathAtan2 2. Ordering reading: the game's slots need mathAtan2 < dot < pos < w (or no w
 *   slot); w's uadd is created in case 2, before case 0's pos loads, in every form that keeps
 *   it an expression.
 * 2026-10-16 (last-percent push; start 14). IDA (0017's .i64, 08003FF0): k's term 3 is
 *   "add.s f6,f16(ez),f10(pos2); mul.s f10,f14(vz),f6" (0x080041B8), vz * (ez + pos[2]) in
 *   source order; this C sorted it to (pos[2] + ez) * vz. Lever: IDO_NOTES 273/283 (an array
 *   element's commutative operands are sorted, a field keeps source order), as the twin's FAKE
 *   view (tlb_0326_1.c Kpaper) and tlb_0131.c. Tries, one check each:
 *   - term 3 alone as "((Vec3f *) w->frame->pos)->z": 119, length -2 (term 2 then sorted first);
 *   - all nine position reads as "((Vec3f *) w->frame->pos)->x/y/z" (vx..vz, k, q): 92, every
 *     operand order the game's, ugen's float temps one step round;
 *   - the same with the (0, dz) FAKE removed ("vz * dz", catalog 41: redo fakes after a fix):
 *     6, length 0, every register the game's. Left only the slot words: pos[0] 0x38 (game
 *     0x3C), pos[2] 0x3C (game 0x38), mathAtan2's address 0x3C (game 0x44). Not kept (the push
 *     keeps only a MATCH); this form is the start for the slots.
 *   Slot mechanism, read in n64decomp/ido's decompiled uopt (uoptreg2.c spilltemps, uopttemp.c
 *   findbbtemps/gettemp): the uncoloured pos[1]/pos[2] CSEs get their words at reemission from
 *   gettemp, the first slot whose expression is not in that block's region, else a new word.
 *   In the view form (uopt-spill, PROC 31): w 4 slot 0 (0x44), dot 118 slot 1, mathAtan2 139
 *   slot 2 (0x3C), pos[0] 239 slot 3 (0x38); block 16's region {4, 118, 239} leaves slot 2
 *   free, so pos[2] takes 0x3C and pos[1] a new 0x34. The game's words need slot 0 = {4, 139}
 *   with 4 in block 16's region but not in 17-19 (pos[2] then finds slots 0-2 busy: 0x38, then
 *   0x34), or no w slot with 139 in block 16's region. The region is the backward flow from
 *   each deleted occurrence (sink) to its source (uoptscm.c), and w is used after every call,
 *   so neither reading is reachable while w stays the uadd expression.
 *   - w = (LensflareWork *) obj, "w = LENSFLARE(w);" in cases 2/0/3, on the view form: 208,
 *     frame 0xA0 (w a variable: the pos loads are then numbered 80/85/90 among vx/vy/vz, pos[1]
 *     takes f14 and vz is left uncoloured). Taken back.
 * 2026-10-16 (the three slot words; 6 to 2, kept). Read: IDA on 0017 (08003FF0) and on
 *   0021 (opLensflareProc, 080060CC, the sibling's game): w's spill 0x40, mathAtan2's 0x3C, the
 *   dot 0x38, pos[0] 0x34, pos[2] 0x30, pos[1] 0x2C. So both games order the slots w, address,
 *   dot, pos[0], and the address's slot is taken in the position block. uopttemp.c gettemp and
 *   uoptemit.c emit_expr: a twice-used expression takes the first free slot when emitted and
 *   frees it after its last use, so in the position block w->frame (count 3, v1) takes a word
 *   first, pos[1] the next, and pos[2] gets w->frame's word back. The game's words then need:
 *   no pad (homes 92: w's never-written slot 0x48, as in getonProc/gimIheadProc), the address
 *   in slot 1 and busy there. Tries (real):
 *   - pad gone, "if (mathAtan2) {}" in the do-while's place: 8, every stack word the game's
 *     (spill print: w -96, address bit 77 -100 with region 14-19, dot -104, pos[0] -108); the
 *     address range then holds block 14, where cam is v0, so it took a0 (game v0).
 *   - the same test inside the do-while: 244; the do-while, then the test: 200 (spill print:
 *     address v0 and the words right, but two more blocks in ex/ey/ez's span: ex, ey, ez lose
 *     their registers); "do { if (mathAtan2) break; } while (0);": 200.
 *   - pad gone + "c = (s32) mathAtan2;" first (ihandpointProc's dead store): 53, frame 0xA0
 *     (address slot 1 but free in the position block: pos[2] took it, pos[1] 0x38); with the
 *     test also at the t/q boundary: 206. "while (mathAtan2) { break; }": 8 (test in block 14).
 *   - pad gone, "do {} while (mathAtan2 && 0);" (the test in the do-while's own block, v0 free,
 *     the block count unchanged): 2, kept. Left: the second call reloads the address into v0
 *     (game t9). Read in uoptreg2.c globalcolor: a range splits when adjsave x count (= its
 *     total save) <= its best cost; here the source block's test is one more use (save 4,
 *     cost 3: three call crossings), so the range stays whole in v0; in the original the
 *     address's source counted no use of its own (save 3 <= 3, split: [..call 1] v0, the rest
 *     uncoloured, t9).
 * 2026-10-16 (the t9 reload; 2 to MATCH). IDA read first: 0017 08003FF0 (li v0 after
 *   cam's last use, sw v0 0x44 before call 1, lw t9 0x44 before call 2) and the sibling 0021
 *   080060CC (the same: li v0 in the position block, lw t9 0x64 before its call 2). Read in
 *   n64decomp/ido's uopt: makelivranges counts a PRE source occurrence as store + load (2) and
 *   only a PRE insert as store alone (1), so no early source gives save 3; cupcosts adds one per
 *   call crossed (a lu after a call, a call block with a live successor, a reaching block after a
 *   call), so a use after the second call adds 2 to the cost and 1 to the save. Tries (real):
 *   - "do {} while (mathAtan2 && 0);" after the second atan2 statement: 171, length +2, but the
 *     split proven (the game's "lw t9" reload); the new block after call 2 moved the float
 *     reloads there and ugen spilled two more words (frame 0xB0).
 *   - "if (mathAtan2) {}" in the same place: 172, the same (test emits nothing, t9 right, the
 *     block after call 2 still split).
 *   - "if (mathAtan2) {}" at the fade if/else's join (floats dead there, a block start already):
 *     MATCH (check: "MATCH except own section addresses"; force none real 0). Spill print: the
 *     address range prio 1 (save 5), v0 cost 8, split ("77 split out 319"): 15-18 v0, the rest
 *     "not colored (-ve save)"; slots unchanged (w 0, address 1, dot 2, pos[0] 3). Kept, FAKE.
 * 2026-10-17 (certain pass): the Vec3f view of pos gone. With pos[k] array
 *   elements every term of k holds one, and IDO reverses such a sum (term 3 first); k written z
 *   term first, "vz * (..) / 2 + (vy * (..) / 2 + (..) * vx / 2)", as bmove.c's BmoveX and plshot.c
 *   write a distance from a frame's place, gives the game's order: MATCH, object identical. */
s32 effLensflareProc(Obj *obj, s16 msg, void *arg) {
    LensflareWork *w = LENSFLARE(obj);
    CamView *cam;
    f32 ex;
    f32 ey;
    f32 ez;
    f32 dz;
    f32 dy;
    f32 dx;
    f32 q[3];
    f32 vx;
    f32 vy;
    f32 vz;
    f32 k;
    f32 t;
    f32 c;

    switch (msg) {
        case 2:
            obj->name = "lensflare";
            w->frame = (AnimeFrame *) frmCreate(gFrmLayer8, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->anim = effLensflareAnim;
            ((s32 (*)(void **ram, s32 n)) floadGetSetRam)((void **) w->frame->segs, 0x13B);
            w->frame->scale[2] = 1.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->alpha = 255;
            break;
        case 1:
            frmDestroy(&LENSFLARE(obj)->frame->f);
            break;
        case 0:
            cam = gGameWork.view;
            if (cam == NULL) {
                break;
            }
            ex = gGameWork.view->eye[0];
            ey = gGameWork.view->eye[1];
            ez = gGameWork.view->eye[2];
            dx = gGameWork.view->at[0] - ex;
            dy = gGameWork.view->at[1] - ey;
            dz = gGameWork.view->at[2] - ez;
            vx = w->frame->pos[0] - ex;
            vy = w->frame->pos[1] - ey;
            vz = w->frame->pos[2] - ez;
            k = vz * (ez + w->frame->pos[2]) / 2 +
                (vy * (ey + w->frame->pos[1]) / 2 + (w->frame->pos[0] + ex) * vx / 2);
            t = (k - vx * ex - vy * ey - vz * ez) / (vx * dx + vy * dy + vz * dz);
            q[0] = dx * t + ex - w->frame->pos[0];
            q[1] = dy * t + ey - w->frame->pos[1];
            q[2] = dz * t + ez - w->frame->pos[2];
            w->frame->rot[1] = mathAtan2(q[0], q[2]);
            mathVecRotateY(-w->frame->rot[1], &q[0], &q[1], &q[2]);
            w->frame->rot[0] = mathAtan2(q[2], q[1]) - 1.5707964f;
            w->frame->rot[2] = 0.0f;
            c = (vx * dx + vy * dy + vz * dz) / sqrtf((vx * vx + vy * vy + vz * vz) * (dx * dx + dy * dy + dz * dz));
            fadeSetColor(255, 255, 255);
            if (c > 0.5) {
                fadeSetLevel((c * 2 - 1.0f) * (c * 2 - 1.0f) * 0.8 * w->alpha);
            } else {
                fadeSetLevel(0.0f);
            }
            w->alpha += w->alphaStep;
            if (w->alpha < 0) {
                w->alpha = 0;
            } else if (w->alpha > 255) {
                w->alpha = 255;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetEnvColor(gGfxDlHead++, 255, 255, 255, w->alpha);
            gDPSetCombine(gGfxDlHead++, 0xFF9BFF, 0xFF36FF7F);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A lens flare at (x, y, z). */
Obj *effLensflareMake(Obj *parent, f32 x, f32 y, f32 z) {
    Obj *o;

    if ((o = tskCreate(parent, effLensflareProc, parent->file, NULL)) != NULL) {
        LENSFLARE(o)->frame->pos[0] = x;
        LENSFLARE(o)->frame->pos[1] = y;
        LENSFLARE(o)->frame->pos[2] = z;
    }
    return o;
}

void effLensflareSetFade(Obj *obj, s32 step) {
    LENSFLARE(obj)->alphaStep = step;
}

extern Gfx effTractorDl[]; /* the tractor's draw setup */
extern Gfx effTractorBeam1Dl[]; /* the tractor beam's two looks, in file 0x13B */
extern Gfx effTractorBeam2Dl[];

/* "tractor": a "pol" frame whose scale moves to a target on each axis in a set number of
 * steps. */
typedef struct TractorWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 scaleTarget[3];
    /* 0x10 */ f32 scaleStep[3];
    /* 0x1C */ f32 steps;
} TractorWork;

#define TRACTOR(obj) ((TractorWork *) FX(obj))

s32 effTractorProc(Obj *obj, s16 msg, void *arg) {
    TractorWork *w = TRACTOR(obj);

    switch (msg) {
        case 2:
            obj->name = "tractor";
            w->frame = (PolFrame *) frmCreate(gFrmEffectLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl = (Gfx *) fadeSegToRam(effTractorDl, obj->file);
            if (arg == NULL) {
                w->frame->dl2 = effTractorBeam1Dl;
            } else {
                w->frame->dl2 = effTractorBeam2Dl;
            }
            floadGetSetRam((void **) w->frame->segs, 0x13B);
            w->steps = 8.0f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (0.00001 < w->scaleStep[0]) {
                if (fabsf(w->scaleTarget[0] - w->frame->scale[0]) < w->scaleStep[0]) {
                    w->frame->scale[0] = w->scaleTarget[0];
                    w->scaleStep[0] = 0.0f;
                } else if (w->scaleTarget[0] < w->frame->scale[0]) {
                    w->frame->scale[0] -= w->scaleStep[0];
                } else {
                    w->frame->scale[0] += w->scaleStep[0];
                }
            }
            if (0.00001 < w->scaleStep[1]) {
                if (fabsf(w->scaleTarget[1] - w->frame->scale[1]) < w->scaleStep[1]) {
                    w->frame->scale[1] = w->scaleTarget[1];
                    w->scaleStep[1] = 0.0f;
                } else if (w->scaleTarget[1] < w->frame->scale[1]) {
                    w->frame->scale[1] -= w->scaleStep[1];
                } else {
                    w->frame->scale[1] += w->scaleStep[1];
                }
            }
            if (0.00001 < w->scaleStep[2]) {
                if (fabsf(w->scaleTarget[2] - w->frame->scale[2]) < w->scaleStep[2]) {
                    w->frame->scale[2] = w->scaleTarget[2];
                    w->scaleStep[2] = 0.0f;
                } else if (w->scaleTarget[2] < w->frame->scale[2]) {
                    w->frame->scale[2] -= w->scaleStep[2];
                } else {
                    w->frame->scale[2] += w->scaleStep[2];
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A tractor beam of a look (0 or not) at (x, y, z). */
Obj *effTractorMake(Obj *parent, s32 look, f32 x, f32 y, f32 z) {
    Obj *o;

    if ((o = tskCreate(parent, effTractorProc, parent->file, (void *) look)) != NULL) {
        TRACTOR(o)->frame->pos[0] = x;
        TRACTOR(o)->frame->pos[1] = y;
        TRACTOR(o)->frame->pos[2] = z;
    }
    return o;
}

/* Points the beam at (x, y, z). */
void effTractorAimAt(Obj *obj, f32 x, f32 y, f32 z) {
    TractorWork *w = TRACTOR(obj);
    s32 pad;
    f32 d[3];

    d[0] = x - w->frame->pos[0];
    d[1] = y - w->frame->pos[1];
    d[2] = z - w->frame->pos[2];
    w->frame->rot[1] = mathAtan2(d[0], d[2]);
    mathVecRotateY(-w->frame->rot[1], &d[0], &d[1], &d[2]);
    w->frame->rot[0] = mathAtan2(d[2], d[1]) - 3.1415927f;
    w->frame->rot[2] = 0.0f;
}

void effTractorSetSteps(Obj *obj, f32 steps) {
    TRACTOR(obj)->steps = steps;
}

void effTractorSetScale(Obj *obj, f32 x, f32 y, f32 z) {
    TRACTOR(obj)->frame->scale[0] = x;
    TRACTOR(obj)->frame->scale[1] = y;
    TRACTOR(obj)->frame->scale[2] = z;
}

/* The scale's target, reached in the steps on each axis. */
void effTractorScaleTo(Obj *obj, f32 x, f32 y, f32 z) {
    TractorWork *w = TRACTOR(obj);

    w->scaleTarget[0] = x;
    w->scaleTarget[1] = y;
    w->scaleTarget[2] = z;
    w->scaleStep[0] = fabsf(x - w->frame->scale[0]) / w->steps;
    w->scaleStep[1] = fabsf(y - w->frame->scale[1]) / w->steps;
    w->scaleStep[2] = fabsf(z - w->frame->scale[2]) / w->steps;
}

extern Gfx effCloudDl[]; /* the cloud, in file 0x13A */

/* "cloud": a "pol" frame moving at a set speed towards a target, fading; gone when its alpha
 * runs out. */
typedef struct CloudWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 steps; /* how many steps a move takes */
    /* 0x08 */ f32 target[3];
    /* 0x14 */ f32 vel[3]; /* per step, on each axis */
    /* 0x20 */ s32 alpha;
    /* 0x24 */ s32 alphaStep;
} CloudWork;

#define CLOUD(obj) ((CloudWork *) FX(obj))

s32 effCloudProc(Obj *obj, s16 msg, void *arg) {
    CloudWork *w = CLOUD(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "cloud";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = effCloudDl;
            w->frame->flags = 7;
            floadGetSetRam((void **) w->frame->segs, 0x13A);
            w->frame->scale[2] = 10.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->alpha = 255;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (0.0001f < w->vel[0]) {
                if (fabsf(w->target[0] - w->frame->pos[0]) < w->vel[0]) {
                    w->frame->pos[0] = w->target[0];
                    w->vel[0] = 0.0f;
                } else if (w->target[0] < w->frame->pos[0]) {
                    w->frame->pos[0] -= w->vel[0];
                } else {
                    w->frame->pos[0] += w->vel[0];
                }
            }
            if (0.0001f < w->vel[1]) {
                if (fabsf(w->target[1] - w->frame->pos[1]) < w->vel[1]) {
                    w->frame->pos[1] = w->target[1];
                    w->vel[1] = 0.0f;
                } else if (w->target[1] < w->frame->pos[1]) {
                    w->frame->pos[1] -= w->vel[1];
                } else {
                    w->frame->pos[1] += w->vel[1];
                }
            }
            if (0.0001f < w->vel[2]) {
                if (fabsf(w->target[2] - w->frame->pos[2]) < w->vel[2]) {
                    w->frame->pos[2] = w->target[2];
                    w->vel[2] = 0.0f;
                } else if (w->target[2] < w->frame->pos[2]) {
                    w->frame->pos[2] -= w->vel[2];
                } else {
                    w->frame->pos[2] += w->vel[2];
                }
            }
            w->alpha += w->alphaStep;
            if (w->alpha < 0) {
                return -1;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetEnvColor(gGfxDlHead++, 255, 255, 255, w->alpha);
            gDPSetCombine(gGfxDlHead++, 0xFF9BFF, 0xFF36FF7F);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A cloud at (x, y, z). */
Obj *effCloudMake(Obj *parent, f32 x, f32 y, f32 z) {
    Obj *o;

    if ((o = tskCreate(parent, effCloudProc, parent->file, NULL)) != NULL) {
        CLOUD(o)->frame->pos[0] = x;
        CLOUD(o)->frame->pos[1] = y;
        CLOUD(o)->frame->pos[2] = z;
    }
    return o;
}

void effCloudSetScale(Obj *obj, f32 x, f32 y, f32 z) {
    CLOUD(obj)->frame->scale[0] = x;
    CLOUD(obj)->frame->scale[1] = y;
    CLOUD(obj)->frame->scale[2] = z;
}

/* Fades out over steps (the alpha's step rounded up), the move's steps too. */
void effCloudFadeOut(Obj *obj, s32 steps) {
    CloudWork *w = CLOUD(obj);

    w->steps = steps;
    w->alphaStep = -(w->alpha + steps - 1) / steps;
}

/* The move's target, reached in its steps on each axis. */
void effCloudMoveTo(Obj *obj, f32 x, f32 y, f32 z) {
    CloudWork *w = CLOUD(obj);

    w->target[0] = x;
    w->target[1] = y;
    w->target[2] = z;
    w->vel[0] = fabsf(x - w->frame->pos[0]) / w->steps;
    w->vel[1] = fabsf(y - w->frame->pos[1]) / w->steps;
    w->vel[2] = fabsf(z - w->frame->pos[2]) / w->steps;
}

/* "dhuman" (a demo human): a player in demo mode whose model moves at a set speed towards a
 * target, turns, grows and plays its animation by a step. */
typedef struct DhumanWork {
    /* 0x00 */ Obj *player;
    /* 0x04 */ AnimeFrame *model; /* the player's */
    /* 0x08 */ Frame *link; /* the model's parent while it is unlinked */
    /* 0x0C */ f32 steps; /* how many steps a move takes */
    /* 0x10 */ f32 target[3];
    /* 0x1C */ f32 vel[3]; /* per step, on each axis */
    /* 0x28 */ u8 pad28[4];
    /* 0x2C */ f32 turn; /* added to the model's rot[1] each step */
    /* 0x30 */ f32 scale;
    /* 0x34 */ f32 scaleStep;
    /* 0x38 */ f32 scaleTarget;
    /* 0x3C */ f32 animStep; /* added to the model's animation frame, up to its last */
} DhumanWork;

#define DHUMAN(obj) ((DhumanWork *) FX(obj))

s32 dhumanProc(Obj *obj, s16 msg, void *arg) {
    DhumanWork *w = DHUMAN(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "dhuman";
            w->scale = 0.1f;
            break;
        case 1:
            frmDestroy(&w->model->f);
            break;
        case 4:
            if (arg != NULL) {
                ((f32 *) arg)[0] = w->model->pos[0];
                ((f32 *) arg)[1] = w->model->pos[1];
                ((f32 *) arg)[2] = w->model->pos[2];
            }
            break;
        case 0:
            if (0.0001f < w->vel[0]) {
                if (fabsf(w->target[0] - w->model->pos[0]) < w->vel[0]) {
                    w->model->pos[0] = w->target[0];
                    w->vel[0] = 0.0f;
                } else if (w->target[0] < w->model->pos[0]) {
                    w->model->pos[0] -= w->vel[0];
                } else {
                    w->model->pos[0] += w->vel[0];
                }
            }
            if (0.0001f < w->vel[1]) {
                if (fabsf(w->target[1] - w->model->pos[1]) < w->vel[1]) {
                    w->model->pos[1] = w->target[1];
                    w->vel[1] = 0.0f;
                } else if (w->target[1] < w->model->pos[1]) {
                    w->model->pos[1] -= w->vel[1];
                } else {
                    w->model->pos[1] += w->vel[1];
                }
            }
            if (0.0001f < w->vel[2]) {
                if (fabsf(w->target[2] - w->model->pos[2]) < w->vel[2]) {
                    w->model->pos[2] = w->target[2];
                    w->vel[2] = 0.0f;
                } else if (w->target[2] < w->model->pos[2]) {
                    w->model->pos[2] -= w->vel[2];
                } else {
                    w->model->pos[2] += w->vel[2];
                }
            }
            if (0.00001f < w->scaleStep) {
                if (fabsf(w->scaleTarget - w->scale) < w->scaleStep) {
                    w->scale = w->scaleTarget;
                    w->scaleStep = 0.0f;
                } else if (w->scaleTarget < w->scale) {
                    w->scale -= w->scaleStep;
                } else {
                    w->scale += w->scaleStep;
                }
            }
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = w->scale;
            w->model->rot[1] += w->turn;
            if (0.0f < w->animStep) {
                w->model->frame += w->animStep;
                if (polAnimeLastFrame(w->model) < w->model->frame) {
                    w->model->frame = polAnimeLastFrame(w->model);
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A demo human: player index's chara at (x, y, z), standing in action 0xE4. */
Obj *dhumanMake(Obj *parent, s32 index, s32 chara, f32 x, f32 y, f32 z) {
    Obj *o;
    DhumanWork *w;
    s32 pad;
    Player *pl;
    PlInit init;

    if ((o = tskCreate(parent, dhumanProc, parent->file, NULL)) != NULL) {
        plMakeInit(&init, index, chara);
        init.zone = 0;
        init.flags |= 0x80000000;
        init.place = x;
        init.height = y;
        init.side = z;
        w = DHUMAN(o);
        w->player = plCreate(parent, &init);
        pl = PL(w->player);
        plEnterDemo(w->player);
        pl->flags |= 0x10000000;
        plSetAction(w->player, 0xE4);
        w->model = pl->model;
        if (pl->shadow != NULL) {
            pl->shadow->flags &= ~2;
        }
    }
    return o;
}

void dhumanSetSteps(Obj *obj, s32 steps) {
    if (obj != NULL) {
        DHUMAN(obj)->steps = steps;
    }
}

void dhumanGetPos(Obj *obj, f32 *x, f32 *y, f32 *z) {
    DhumanWork *w = DHUMAN(obj);

    if (obj != NULL) {
        *x = w->model->pos[0];
        *y = w->model->pos[1];
        *z = w->model->pos[2];
    }
}

void dhumanSetPos(Obj *obj, f32 x, f32 y, f32 z) {
    DhumanWork *w = DHUMAN(obj);

    if (obj != NULL) {
        w->model->pos[0] = x;
        w->model->pos[1] = y;
        w->model->pos[2] = z;
    }
}

void dhumanSetDir(Obj *obj, f32 dir) {
    if (obj != NULL) {
        DHUMAN(obj)->model->rot[1] = dir;
    }
}

/* The move's target, reached in its steps on each axis. */
void dhumanMoveTo(Obj *obj, f32 x, f32 y, f32 z) {
    DhumanWork *w = DHUMAN(obj);

    if (obj != NULL) {
        w->target[0] = x;
        w->target[1] = y;
        w->target[2] = z;
        w->vel[0] = fabsf(x - w->model->pos[0]) / w->steps;
        w->vel[1] = fabsf(y - w->model->pos[1]) / w->steps;
        w->vel[2] = fabsf(z - w->model->pos[2]) / w->steps;
    }
}

void dhumanStop(Obj *obj) {
    DhumanWork *w = DHUMAN(obj);

    if (obj != NULL) {
        w->vel[2] = 0.0f;
        w->vel[1] = 0.0f;
        w->vel[0] = 0.0f;
    }
}

void dhumanSetScale(Obj *obj, f32 scale) {
    if (obj != NULL) {
        DHUMAN(obj)->scale = scale;
    }
}

/* The size's target, reached in the move's steps. */
void dhumanScaleTo(Obj *obj, f32 scale) {
    DhumanWork *w = DHUMAN(obj);

    if (obj != NULL) {
        w->scaleTarget = scale;
        w->scaleStep = fabsf(scale - w->scale) / w->steps;
    }
}

/* Stands (action 0xE4), lying back. */
void dhumanLieBack(Obj *obj) {
    DhumanWork *w = DHUMAN(obj);
    s32 pad;

    if (obj != NULL) {
        if (w->player != NULL) {
            plSetAction(w->player, 0xE4);
        }
        w->animStep = 0.0f;
        w->model->rot[0] = 1.5707964f;
        w->turn = 0.0f;
    }
}

/* Action 0xE5, turned a quarter, spinning a degree a step. */
void dhumanSpin(Obj *obj) {
    DhumanWork *w = DHUMAN(obj);
    s32 pad;

    if (obj != NULL) {
        if (w->player != NULL) {
            plSetAction(w->player, 0xE5);
        }
        w->animStep = 0.0f;
        w->model->rot[0] = 0.0f;
        w->model->rot[1] = -1.5707964f;
        w->turn = 0.01745329238f;
    }
}

/* Action 0x13A. */
void dhumanSetAction13A(Obj *obj) {
    DhumanWork *w = DHUMAN(obj);
    s32 pad;

    if (obj != NULL) {
        if (w->player != NULL) {
            plSetAction(w->player, 0x13A);
        }
        w->animStep = 0.0f;
        w->model->rot[0] = 0.0f;
        w->turn = 0.0f;
    }
}

/* Halted, playing animation 0x11F. */
void dhumanPlayAnime11F(Obj *obj) {
    DhumanWork *w = DHUMAN(obj);
    s32 pad;

    if (obj != NULL) {
        if (w->player != NULL) {
            plHalt(w->player);
            plAnimeSet(PL(w->player), 0x11F);
        }
        w->animStep = 0.4f;
        w->model->rot[0] = 0.0f;
        w->turn = 0.0f;
    }
}

/* Halted, playing animation 0x120. */
void dhumanPlayAnime120(Obj *obj) {
    DhumanWork *w = DHUMAN(obj);
    s32 pad;

    if (obj != NULL) {
        if (w->player != NULL) {
            plHalt(w->player);
            plAnimeSet(PL(w->player), 0x120);
        }
        w->animStep = 0.4f;
        w->model->rot[0] = 0.0f;
        w->turn = 0.0f;
    }
}

/* Halted, playing animation 0x121. */
void dhumanPlayAnime121(Obj *obj) {
    DhumanWork *w = DHUMAN(obj);
    s32 pad;

    if (obj != NULL) {
        if (w->player != NULL) {
            plHalt(w->player);
            plAnimeSet(PL(w->player), 0x121);
        }
        w->animStep = 0.4f;
        w->model->rot[0] = 0.0f;
        w->turn = 0.0f;
    }
}

/* Takes the model out of the frame tree (hidden). */
void dhumanHide(Obj *obj) {
    DhumanWork *w = DHUMAN(obj);

    if (obj != NULL) {
        if (w->model->f.parent != NULL) {
            w->link = w->model->f.parent;
            frmUnlink(&w->model->f);
        }
    }
}

/* Puts the model back where it was. */
void dhumanShow(Obj *obj) {
    DhumanWork *w = DHUMAN(obj);

    if (obj != NULL) {
        if (w->link != NULL) {
            frmLink(&w->model->f, w->link);
        }
    }
}

extern Gfx effTubeSegment1Dl[]; /* two looks of a "pol" frame, in file 0x13A */
extern Gfx effTubeSegment2Dl[];

/* A "pol" frame of a look (0 or not) under parent at (x, y, z). */
void effTubeAddSegment(Frame *parent, s32 look, f32 x, f32 y, f32 z) {
    PolFrame *f;

    if ((f = (PolFrame *) frmCreate(parent, gFrmKindPol)) != NULL) {
        if (look == 0) {
            f->dl2 = effTubeSegment1Dl;
        } else {
            f->dl2 = effTubeSegment2Dl;
        }
        floadGetSetRam((void **) f->segs, 0x13A);
        f->scale[0] = f->scale[1] = f->scale[2] = 0.1f;
        f->pos[0] = x;
        f->pos[1] = y;
        f->pos[2] = z;
    }
}

extern Gfx effTubeRing1Dl[]; /* the tube's ring, two looks, in file 0x13B */
extern Gfx effTubeRing2Dl[];
extern Gfx effTubeGlow1Dl[]; /* the tube's glow, two looks, in file 0x13A */
extern Gfx effTubeGlow2Dl[];

/* "tube": a column ("pol" frame, with four more frames above and below it), a turning ring
 * around it and a glow moving up or down it to a target in a set number of steps. Made from a
 * copy of its own work (look and position). */
typedef struct TubeWork {
    /* 0x00 */ s32 look; /* 0 or not */
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ f32 steps;
    /* 0x14 */ PolFrame *base;
    /* 0x18 */ PolFrame *glow;
    /* 0x1C */ PolFrame *ring;
    /* 0x20 */ f32 glowTarget; /* the glow's height */
    /* 0x24 */ f32 glowStep;
} TubeWork;

#define TUBE(obj) ((TubeWork *) FX(obj))

s32 effTubeProc(Obj *obj, s16 msg, void *arg) {
    TubeWork *w = TUBE(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "tube";
            *w = *(TubeWork *) arg;
            w->base = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->base == NULL) {
                return -1;
            }
            if (w->look == 0) {
                w->base->dl2 = effTubeSegment1Dl;
            } else {
                w->base->dl2 = effTubeSegment2Dl;
            }
            floadGetSetRam((void **) w->base->segs, 0x13A);
            w->base->scale[2] = 0.1f;
            w->base->scale[0] = w->base->scale[1] = w->base->scale[2];
            w->base->pos[0] = w->pos[0];
            w->base->pos[1] = w->pos[1];
            w->base->pos[2] = w->pos[2];
            effTubeAddSegment(&w->base->f, w->look, w->pos[0], w->pos[1] + 250.0f, w->pos[2]);
            effTubeAddSegment(&w->base->f, w->look, w->pos[0], w->pos[1] - 250.0f, w->pos[2]);
            effTubeAddSegment(&w->base->f, w->look, w->pos[0], w->pos[1] + 500.0f, w->pos[2]);
            effTubeAddSegment(&w->base->f, w->look, w->pos[0], w->pos[1] - 500.0f, w->pos[2]);
            w->ring = (PolFrame *) frmCreate(gFrmEffectLayer, gFrmKindPol);
            if (w->ring == NULL) {
                return -1;
            }
            w->ring->dl = (Gfx *) fadeSegToRam(effDrawSetupDl, obj->file);
            if (w->look == 0) {
                w->ring->dl2 = effTubeRing1Dl;
            } else {
                w->ring->dl2 = effTubeRing2Dl;
            }
            floadGetSetRam((void **) w->ring->segs, 0x13B);
            w->ring->scale[2] = 0.1f;
            w->ring->scale[0] = w->ring->scale[2];
            w->ring->scale[1] = 2.5f;
            w->ring->pos[0] = w->pos[0];
            w->ring->pos[1] = w->pos[1];
            w->ring->pos[2] = w->pos[2];
            w->glow = (PolFrame *) frmCreate(gFrmEffectLayer, gFrmKindPol);
            if (w->glow == NULL) {
                return -1;
            }
            /* the setup list passed as a pointer at both calls (fadeSegToRam takes a void *):
             * IDO builds its address again here, as the game does, instead of keeping the ring's
             * (IDO_NOTES 378). */
            w->glow->dl = (Gfx *) fadeSegToRam(effDrawSetupDl, obj->file);
            if (w->look == 0) {
                w->glow->dl2 = effTubeGlow1Dl;
            } else {
                w->glow->dl2 = effTubeGlow2Dl;
            }
            floadGetSetRam((void **) w->glow->segs, 0x13A);
            w->glow->scale[2] = 0.1f;
            w->glow->scale[0] = w->glow->scale[1] = w->glow->scale[2];
            w->glow->pos[0] = w->pos[0];
            w->glow->pos[1] = w->pos[1];
            w->glow->pos[2] = w->pos[2];
            break;
        case 1:
            frmDestroy(&w->base->f);
            frmDestroy(&w->glow->f);
            frmDestroy(&w->ring->f);
            break;
        case 0:
            if (0.0001f < w->glowStep) {
                if (fabsf(w->glowTarget - w->glow->pos[1]) < w->glowStep) {
                    w->glow->pos[1] = w->glowTarget;
                    w->glowStep = 0.0f;
                } else if (w->glowTarget < w->glow->pos[1]) {
                    w->glow->pos[1] -= w->glowStep;
                } else {
                    w->glow->pos[1] += w->glowStep;
                }
            }
            w->ring->rot[1] += 0.01745329238f;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A tube of a look (0 or not) at (x, y, z). */
Obj *effTubeMake(Obj *parent, s32 look, f32 x, f32 y, f32 z) {
    TubeWork init;

    init.look = look;
    init.pos[0] = x;
    init.pos[1] = y;
    init.pos[2] = z;
    return tskCreate(parent, effTubeProc, parent->file, &init);
}

void effTubeSetSteps(Obj *obj, s32 steps) {
    TUBE(obj)->steps = steps;
}

void effTubeSetGlowY(Obj *obj, f32 y) {
    TUBE(obj)->glow->pos[1] = y;
}

/* The glow's target height, reached in the steps. */
void effTubeGlowTo(Obj *obj, f32 y) {
    TubeWork *w = TUBE(obj);

    w->glowTarget = y;
    w->glowStep = fabsf(y - w->glow->pos[1]) / w->steps;
}

extern Gfx effGhostDl[]; /* the ghost's draw setup */

/* "ghost": a "dlist" frame drawing the previous frame's picture over the screen, darkened by
 * its alpha (an afterimage). */
typedef struct GhostWork {
    /* 0x00 */ DLFrame *frame;
    /* 0x04 */ s32 alpha;
} GhostWork;

#define GHOST(obj) ((GhostWork *) FX(obj))

s32 effGhostProc(Obj *obj, s16 msg, void *arg) {
    GhostWork *w = GHOST(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "ghost";
            w->frame = (DLFrame *) frmCreate(gFrmLayers[0], gFrmKindDlist);
            if (w->frame == NULL) {
                return -1;
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, fadeSegToRam(effGhostDl, obj->file));
            gGfxDlHead = effGhostDrawImage(gGfxDlHead, gfxGetPrevFramebuffer(), w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* An afterimage of an alpha. */
Obj *effGhostMake(Obj *parent, s32 alpha) {
    Obj *o;

    if ((o = tskCreate(parent, effGhostProc, parent->file, NULL)) != NULL) {
        GHOST(o)->alpha = alpha;
    }
    return o;
}

extern u8 effDimpactKind0Anim[]; /* the dimpact's animations, two kinds, in file 0xEA */
extern u8 effDimpactKind1Anim[];
extern u8 effDimpactKind0Pose2Anim[]; /* its second pair */
extern u8 effDimpactKind1Pose2Anim[];

/* "dimpact": an "anime" frame (file 0xEA) moving at a set speed towards a target, turning, its
 * animation playing to an end. */
typedef struct DimpactWork {
    /* 0x00 */ s32 kind; /* which animations (0 or not) */
    /* 0x04 */ AnimeFrame *frame;
    /* 0x08 */ f32 steps; /* how many steps a move takes */
    /* 0x0C */ f32 target[3];
    /* 0x18 */ f32 vel[3]; /* per step, on each axis */
    /* 0x24 */ f32 turn; /* added to rot[1] each step */
    /* 0x28 */ f32 frameEnd;
    /* 0x2C */ f32 frameStep;
} DimpactWork;

#define DIMPACT(obj) ((DimpactWork *) FX(obj))

s32 effDimpactProc(Obj *obj, s16 msg, void *arg) {
    DimpactWork *w = DIMPACT(obj);

    switch (msg) {
        case 2:
            obj->name = "dimpact";
            w->kind = (s32) arg;
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            if (w->kind == 0) {
                w->frame->anim = effDimpactKind0Anim;
            } else {
                w->frame->anim = effDimpactKind1Anim;
            }
            floadGetSetRam((void **) w->frame->segs, 0xEA);
            w->frameStep = 0.0f;
            w->turn = 0.0f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (0.0001f < w->vel[0]) {
                if (fabsf(w->target[0] - w->frame->pos[0]) < w->vel[0]) {
                    w->frame->pos[0] = w->target[0];
                    w->vel[0] = 0.0f;
                } else if (w->target[0] < w->frame->pos[0]) {
                    w->frame->pos[0] -= w->vel[0];
                } else {
                    w->frame->pos[0] += w->vel[0];
                }
            }
            if (0.0001f < w->vel[1]) {
                if (fabsf(w->target[1] - w->frame->pos[1]) < w->vel[1]) {
                    w->frame->pos[1] = w->target[1];
                    w->vel[1] = 0.0f;
                } else if (w->target[1] < w->frame->pos[1]) {
                    w->frame->pos[1] -= w->vel[1];
                } else {
                    w->frame->pos[1] += w->vel[1];
                }
            }
            if (0.0001f < w->vel[2]) {
                if (fabsf(w->target[2] - w->frame->pos[2]) < w->vel[2]) {
                    w->frame->pos[2] = w->target[2];
                    w->vel[2] = 0.0f;
                } else if (w->target[2] < w->frame->pos[2]) {
                    w->frame->pos[2] -= w->vel[2];
                } else {
                    w->frame->pos[2] += w->vel[2];
                }
            }
            w->frame->frame += w->frameStep;
            if (w->frameEnd < w->frame->frame) {
                w->frame->frame = w->frameEnd;
            }
            w->frame->rot[1] += w->turn;
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombine(gGfxDlHead++, 0x127E24, 0xFFFFF3F9);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A dimpact of a kind at (x, y, z). */
Obj *effDimpactMake(Obj *parent, s32 kind, f32 x, f32 y, f32 z) {
    Obj *o;

    if ((o = tskCreate(parent, effDimpactProc, parent->file, (void *) kind)) != NULL) {
        DIMPACT(o)->frame->pos[0] = x;
        DIMPACT(o)->frame->pos[1] = y;
        DIMPACT(o)->frame->pos[2] = z;
    }
    return o;
}

void effDimpactSetSteps(Obj *obj, s32 steps) {
    DIMPACT(obj)->steps = steps;
}

void effDimpactGetPos(Obj *obj, f32 *x, f32 *y, f32 *z) {
    *x = DIMPACT(obj)->frame->pos[0];
    *y = DIMPACT(obj)->frame->pos[1];
    *z = DIMPACT(obj)->frame->pos[2];
}

void effDimpactSetPos(Obj *obj, f32 x, f32 y, f32 z) {
    DIMPACT(obj)->frame->pos[0] = x;
    DIMPACT(obj)->frame->pos[1] = y;
    DIMPACT(obj)->frame->pos[2] = z;
}

/* The move's target, reached in its steps on each axis. */
void effDimpactMoveTo(Obj *obj, f32 x, f32 y, f32 z) {
    DimpactWork *w = DIMPACT(obj);

    w->target[0] = x;
    w->target[1] = y;
    w->target[2] = z;
    w->vel[0] = fabsf(x - w->frame->pos[0]) / w->steps;
    w->vel[1] = fabsf(y - w->frame->pos[1]) / w->steps;
    w->vel[2] = fabsf(z - w->frame->pos[2]) / w->steps;
}

void effDimpactStop(Obj *obj) {
    DIMPACT(obj)->vel[2] = 0.0f;
    DIMPACT(obj)->vel[1] = 0.0f;
    DIMPACT(obj)->vel[0] = 0.0f;
}

/* Plays the first animation from start to end in the move's steps. */
void effDimpactSetPlay(Obj *obj, f32 start, f32 end) {
    DimpactWork *w = DIMPACT(obj);

    if (w->kind == 0) {
        w->frame->anim = effDimpactKind0Anim;
    } else {
        w->frame->anim = effDimpactKind1Anim;
    }
    w->frame->frame = start;
    w->frameEnd = end;
    w->frameStep = (end - start) / w->steps;
}

/* The second animation, held at its first frame. */
void effDimpactSetPose2(Obj *obj) {
    DimpactWork *w = DIMPACT(obj);

    if (w->kind == 0) {
        w->frame->anim = effDimpactKind0Pose2Anim;
    } else {
        w->frame->anim = effDimpactKind1Pose2Anim;
    }
    w->frame->frame = 0.0f;
    w->frameStep = 0.0f;
}

void effDimpactSetDir(Obj *obj, f32 dir) {
    DIMPACT(obj)->frame->rot[1] = dir;
}

void effDimpactSetTurn(Obj *obj, f32 turn) {
    DIMPACT(obj)->turn = turn;
}

/* A "pol" frame (file 0x13A) drawing dl under parent, at the screen's middle, turned round.
 * The turn is set before the three scales, so pi is loaded with 0.3f as in the game. */
PolFrame *effKirakiraLayerMake(Frame *parent, Gfx *dl) {
    PolFrame *f;

    if ((f = (PolFrame *) frmCreate(parent, gFrmKindPol)) != NULL) {
        f->dl2 = dl;
        floadGetSetRam((void **) f->segs, 0x13A);
        f->rot[1] = 3.1415927f;
        f->scale[0] = 0.3f;
        f->scale[1] = 0.3f;
        f->scale[2] = 0.3f;
        f->pos[0] = 160.0f;
        f->pos[1] = 120.0f;
    }
    return f;
}

extern Gfx effKirakiraLayer0Dl[]; /* the kirakira's three layers, in file 0x13A */
extern Gfx effKirakiraLayer1Dl[];
extern Gfx effKirakiraLayer2Dl[];

/* "kirakira" (glittering): a camera of its own (an orthographic one, before the others) drawing
 * three layers at the screen's middle, turning at different speeds. */
typedef struct KirakiraWork {
    /* 0x00 */ CamView *cam;
    /* 0x04 */ DLFrame *setup;
    /* 0x08 */ PolFrame *layers[3]; /* turning at 2, -2 and 1 degrees a step */
} KirakiraWork;

#define KIRAKIRA(obj) ((KirakiraWork *) FX(obj))

s32 effKirakiraProc(Obj *obj, s16 msg, void *arg) {
    KirakiraWork *w = KIRAKIRA(obj);

    switch (msg) {
        case 2:
            obj->name = "kirakira";
            w->cam = (CamView *) frmCreatePrev(gFrmLayers[0], gFrmKindCamera);
            if (w->cam == NULL) {
                return -1;
            }
            w->cam->ortho |= 1;
            w->cam->zNear = -200.0f;
            w->cam->zFar = 200.0f;
            w->setup = (DLFrame *) frmCreate(&w->cam->f, gFrmKindDlist);
            if (w->setup == NULL) {
                return -1;
            }
            w->setup->dl = (Gfx *) fadeSegToRam(effDrawSetupDl, obj->file);
            w->layers[2] = effKirakiraLayerMake(&w->setup->f, effKirakiraLayer2Dl);
            w->layers[1] = effKirakiraLayerMake(&w->setup->f, effKirakiraLayer1Dl);
            w->layers[0] = effKirakiraLayerMake(&w->setup->f, effKirakiraLayer0Dl);
            break;
        case 1:
            frmDestroy(&w->cam->f);
            break;
        case 0:
            w->layers[0]->rot[2] += 0.03490658477f;
            w->layers[1]->rot[2] -= 0.03490658477f;
            w->layers[2]->rot[2] += 0.01745329238f;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *effKirakiraMake(Obj *obj) {
    return tskCreate(obj, effKirakiraProc, obj->file, NULL);
}

extern Gfx effKbackTileDl[]; /* the kback's tile, in file 0x13A */
extern AnimFrame effKbackSparkleAnim[]; /* the kback sparkle's animation */

/* The file's .data, in address order (its externs above stay as declarations). */
extern u8 effKbackSparkleTex0[];
extern u8 effKbackSparkleTex1[];
extern u8 effKbackSparkleTex2[];
extern u8 effKbackSparkleTex3[];
extern u8 effKbackSparkleTex4[];
extern u8 effKbackSparkleTex6[];
Gfx effDrawSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPEndDisplayList(),
}; /* the effects' draw setup (the kirari's, the tube's, ...) */
Gfx effTractorDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetEnvColor(0xFF, 0xFF, 0xFF, 0xA0),
    gsDPSetCombineLERP(0, 0, 0, SHADE, SHADE, 0, ENVIRONMENT, 0, 0, 0, 0, SHADE, SHADE, 0, ENVIRONMENT, 0),
    gsSPEndDisplayList(),
    gsDPPipeSync(),
    gsDPSetEnvColor(0xFF, 0xFF, 0xFF, 0xFF),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0),
    gsSPEndDisplayList(),
}; /* the tractor's draw setup */
Gfx effShowerDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsDPPipelineMode(G_PM_NPRIMITIVE),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetCombineKey(G_CK_NONE),
    gsDPSetTextureLOD(G_TL_TILE),
    gsDPSetTextureDetail(G_TD_CLAMP),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsDPSetTextureLUT(G_TT_NONE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetCombineLERP(PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, COMBINED, 0, 0, 0, COMBINED),
    gsDPSetPrimColor(0, 0, 0xFF, 0xFF, 0xFF, 0xFF),
    gsDPSetEnvColor(0xFF, 0xFF, 0x00, 0xFF),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsSPEndDisplayList(),
}; /* the shower's draw setup */
Gfx effGhostDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_DISABLE),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetCombineLERP(0, 0, 0, TEXEL0, 0, 0, 0, ENVIRONMENT, 0, 0, 0, TEXEL0, 0, 0, 0, ENVIRONMENT),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetTextureFilter(G_TF_POINT),
    gsSPEndDisplayList(),
}; /* the ghost's draw setup */
AnimFrame effKbackSparkleAnim[] = { { (u32) effKbackSparkleTex0, 4 }, { (u32) effKbackSparkleTex1, 4 }, { (u32) effKbackSparkleTex2, 4 }, { (u32) effKbackSparkleTex3, 4 }, { (u32) effKbackSparkleTex4, 4 }, { 0x03002D30, 4 }, { (u32) effKbackSparkleTex6, 4 }, { 0x03003D30, 4 }, { 0x03004530, 4 }, { 0x03004D30, 4 }, { 0x03005530, 4 }, { 0x03005D30, 4 }, { 0x00000000, 0 } }; /* the kback sparkle's animation */
u32 effUnusedDls[] = { 0xE7000000, 0x00000000, 0xE3000A01, 0x00000000, 0xD9FF0000, 0x00000000, 0xD9FFFFFF, 0x00200405, 0xE200001C, 0x005049D8, 0xFCFFFFFF, 0xFFFE793C, 0xDF000000, 0x00000000, 0xE7000000, 0x00000000, 0xE3000A01, 0x00000000, 0xD9FF0000, 0x00000000, 0xD9FFFFFF, 0x00200405, 0xE200001C, 0x00443078, 0xFCFFFFFF, 0xFFFE793C, 0xDF000000, 0x00000000, 0x41523D71, 0x00000000, 0x3F333333, 0x41C80000, 0x00000000, 0x00000000, 0x406A3D71, 0x00000000, 0xC1400000, 0x00000000, 0x00000000, 0xC1C80000, 0x41D5999A, 0x00000000, 0xBFCCCCCD, 0x42480000, 0xC1B9999A, 0x41380000, 0xC00AE148, 0xC2480000 };

/* "kback" (a background): a camera of its own drawing 3 rows of 5 tiles over the screen (every
 * other row shifted half a tile), each turning. */
typedef struct KbackWork {
    /* 0x00 */ CamView *cam;
    /* 0x04 */ DLFrame *setup; /* the tiles' parent */
} KbackWork;

#define KBACK(obj) ((KbackWork *) FX(obj))

/* "kback": an orthographic camera with 3 rows of 5 tiles (file 0x13A), 75 apart, odd rows
 * shifted half a tile; each frame the tiles turn. Each tile's height y is computed right after
 * its scales: that puts its cvt.s.w in floadGetSetRam's delay slot, as the game does. */
s32 effKbackProc(Obj *obj, s16 msg, void *arg) {
    KbackWork *w = KBACK(obj);
    s32 pad[4];
    PolFrame *f;
    s32 i;
    s32 j;
    s32 y;

    switch (msg) {
        case 2:
            obj->name = "kback";
            w->cam = (CamView *) frmCreatePrev(gFrmLayers[0], gFrmKindCamera);
            if (w->cam == NULL) {
                return -1;
            }
            w->cam->ortho |= 1;
            w->cam->zNear = -200.0f;
            w->cam->zFar = 200.0f;
            w->setup = (DLFrame *) frmCreate(&w->cam->f, gFrmKindDlist);
            if (w->setup == NULL) {
                return -1;
            }
            w->setup->dl = (Gfx *) fadeSegToRam(effDrawSetupDl, obj->file);
            for (i = 0; i < 3; i++) {
                for (j = 0; j < 5; j++) {
                    if ((f = (PolFrame *) frmCreate(&w->setup->f, gFrmKindPol)) != NULL) {
                        f->scale[0] = f->scale[1] = f->scale[2] = 0.75f;
                        y = i * 75 + 40;
                        f->dl2 = effKbackTileDl;
                        floadGetSetRam((void **) f->segs, 0x13A);
                        f->rot[1] = 1.5707964f;
                        f->pos[1] = y;
                        f->pos[2] = 0.0f;
                        f->pos[0] = j * 75 + (i & 1) * 75 / 2;
                    }
                }
            }
            break;
        case 1:
            frmDestroy(&w->cam->f);
            break;
        case 0:
            for (f = (PolFrame *) w->setup->f.child; f != NULL; f = (PolFrame *) f->f.next) {
                f->rot[0] += 0.03490658477f;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *effKbackMake(Obj *obj) {
    return tskCreate(obj, effKbackProc, obj->file, NULL);
}

/* Hides the tiles (takes them out of the frame tree). */
void effKbackHide(Obj *obj) {
    KbackWork *w = KBACK(obj);

    if (&w->cam->f == w->setup->f.parent) {
        frmUnlink(&w->setup->f);
    }
}

/* Shows the tiles again. */
void effKbackShow(Obj *obj) {
    KbackWork *w = KBACK(obj);

    if (w->setup->f.parent == NULL) {
        frmLink(&w->setup->f, &w->cam->f);
    }
}

/* A white sparkle with a grey edge at (x, y, z), of a size, growing. */
void effKbackSparkleMake(Obj *obj, f32 x, f32 y, f32 z, f32 size) {
    ItexInit init;

    init = gItexTemplate;
    init.file = 0x123;
    init.frames = (AnimFrame *) fadeSegToRam(effKbackSparkleAnim, obj->file);
    init.flags = 3;
    init.prim.b = init.prim.g = init.prim.r = 255;
    init.env.b = init.env.g = init.env.r = 191;
    init.grow.x = init.grow.y = init.grow.z = 1.01f;
    init.scale[2] = size;
    init.scale[1] = size;
    init.scale[0] = size;
    init.pos[0] = x;
    init.pos[1] = y;
    init.pos[2] = z;
    tskCreate(gEffectTask, itex, 0, &init);
}

extern Gfx effHeartDl[]; /* the heart, in file 0xF4 */

/* "heartb" (a heart): a "pol" frame drifting at a set velocity, growing by a step and fading
 * in by 7 a step until its life runs out, then out; gone when its alpha runs out. */
typedef struct HeartbWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ s32 life; /* steps until it starts fading out */
    /* 0x08 */ f32 steps; /* how many steps a move takes */
    /* 0x0C */ f32 vel[3];
    /* 0x18 */ f32 scale;
    /* 0x1C */ f32 grow; /* added to scale each step */
    /* 0x20 */ s32 alpha;
    /* 0x24 */ s32 fade; /* added to alpha each step */
} HeartbWork;

#define HEARTB(obj) ((HeartbWork *) FX(obj))

s32 effHeartbProc(Obj *obj, s16 msg, void *arg) {
    HeartbWork *w = HEARTB(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "heartb";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            floadGetSetRam((void **) w->frame->segs, 0xF4);
            w->frame->dl2 = effHeartDl;
            w->frame->flags |= 7;
            w->life = 30;
            w->alpha = 0;
            w->fade = 7;
            w->vel[0] = 0.0f;
            w->vel[1] = 0.0f;
            w->vel[2] = 0.0f;
            w->grow = 0.0f;
            w->scale = 1.0f;
            w->steps = 60.0f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->life--;
            if (w->life < 0) {
                w->fade = -7;
            }
            w->frame->pos[0] += w->vel[0];
            w->frame->pos[1] += w->vel[1];
            w->frame->pos[2] += w->vel[2];
            w->scale += w->grow;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale;
            w->alpha += w->fade;
            if (w->alpha < 0) {
                return -1;
            }
            if (w->alpha > 255) {
                w->alpha = 255;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombine(gGfxDlHead++, 0x309A61, 0x5536FF7F);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 255, 0);
            gDPSetEnvColor(gGfxDlHead++, 255, 51, 122, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A heart at (x, y, z). */
Obj *effHeartbMake(Obj *parent, f32 x, f32 y, f32 z) {
    Obj *o;

    if ((o = tskCreate(parent, effHeartbProc, parent->file, NULL)) != NULL) {
        HEARTB(o)->frame->pos[0] = x;
        HEARTB(o)->frame->pos[1] = y;
        HEARTB(o)->frame->pos[2] = z;
    }
    return o;
}

void effHeartbSetSteps(Obj *obj, s32 steps) {
    HEARTB(obj)->steps = steps;
}

void effHeartbSetScale(Obj *obj, f32 scale) {
    HEARTB(obj)->scale = scale;
}

/* Grows to scale in the steps. */
void effHeartbScaleTo(Obj *obj, f32 scale) {
    HEARTB(obj)->grow = (scale - HEARTB(obj)->scale) / HEARTB(obj)->steps;
}

void effHeartbSetAlpha(Obj *obj, s32 alpha) {
    HEARTB(obj)->alpha = alpha;
}

/* Fades to alpha in the steps (the step rounded up). */
void effHeartbFadeTo(Obj *obj, s32 alpha) {
    s32 steps = HEARTB(obj)->steps;

    HEARTB(obj)->fade = (alpha - HEARTB(obj)->alpha + steps - 1) / steps;
}

/* Drifts to (x, y, z) in the steps. */
void effHeartbMoveTo(Obj *obj, f32 x, f32 y, f32 z) {
    HeartbWork *w = HEARTB(obj);

    w->vel[0] = (x - w->frame->pos[0]) / w->steps;
    w->vel[1] = (y - w->frame->pos[1]) / w->steps;
    w->vel[2] = (z - w->frame->pos[2]) / w->steps;
}

/* As effKirakiraLayerMake: a "pol" frame (file 0x13A) drawing dl under parent, at the screen's
 * middle, turned round. The turn is set before the scales, as in effKirakiraLayerMake. */
PolFrame *effKflowerLayerMake(Frame *parent, Gfx *dl) {
    PolFrame *f;

    if ((f = (PolFrame *) frmCreate(parent, gFrmKindPol)) != NULL) {
        f->dl2 = dl;
        floadGetSetRam((void **) f->segs, 0x13A);
        f->rot[1] = 3.1415927f;
        f->scale[0] = 0.3f;
        f->scale[1] = 0.3f;
        f->scale[2] = 0.3f;
        f->pos[0] = 160.0f;
        f->pos[1] = 120.0f;
    }
    return f;
}

extern Gfx effKflowerLayer0Dl[]; /* the kflower's four layers, in file 0x13A */
extern Gfx effKflowerLayer1Dl[];
extern Gfx effKflowerLayer2Dl[];
extern Gfx effKflowerLayer3Dl[];

/* "kflower" (a flower): as the kirakira, four layers turning at different speeds. */
typedef struct KflowerWork {
    /* 0x00 */ CamView *cam;
    /* 0x04 */ DLFrame *setup;
    /* 0x08 */ PolFrame *layers[4]; /* turning at -1, 2, -2 and 1 degrees a step */
} KflowerWork;

#define KFLOWER(obj) ((KflowerWork *) FX(obj))

s32 effKflowerProc(Obj *obj, s16 msg, void *arg) {
    KflowerWork *w = KFLOWER(obj);

    switch (msg) {
        case 2:
            obj->name = "kflower";
            w->cam = (CamView *) frmCreatePrev(gFrmLayers[0], gFrmKindCamera);
            if (w->cam == NULL) {
                return -1;
            }
            w->cam->ortho |= 1;
            w->cam->zNear = -200.0f;
            w->cam->zFar = 200.0f;
            w->setup = (DLFrame *) frmCreate(&w->cam->f, gFrmKindDlist);
            if (w->setup == NULL) {
                return -1;
            }
            w->setup->dl = (Gfx *) fadeSegToRam(effDrawSetupDl, obj->file);
            w->layers[3] = effKflowerLayerMake(&w->setup->f, effKflowerLayer3Dl);
            w->layers[2] = effKflowerLayerMake(&w->setup->f, effKflowerLayer2Dl);
            w->layers[1] = effKflowerLayerMake(&w->setup->f, effKflowerLayer1Dl);
            w->layers[0] = effKflowerLayerMake(&w->setup->f, effKflowerLayer0Dl);
            break;
        case 1:
            frmDestroy(&w->cam->f);
            break;
        case 0:
            w->layers[1]->rot[2] += 0.03490658477f;
            w->layers[2]->rot[2] -= 0.03490658477f;
            w->layers[3]->rot[2] += 0.01745329238f;
            w->layers[0]->rot[2] -= 0.01745329238f;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *effKflowerMake(Obj *obj) {
    return tskCreate(obj, effKflowerProc, obj->file, NULL);
}

extern u8 effGimAnim[]; /* the gim's animation, in file 0x13A */
extern Gfx effGimGlowDl[]; /* the gim's glow */

/* "gim" (a gimmick): an "anime" frame playing its animation to an end, and a glow. */
typedef struct GimWork {
    /* 0x00 */ AnimeFrame *model;
    /* 0x04 */ PolFrame *glow;
    /* 0x08 */ f32 frameStep;
    /* 0x0C */ f32 frameEnd;
} GimWork;

#define GIM(obj) ((GimWork *) FX(obj))

s32 effGimProc(Obj *obj, s16 msg, void *arg) {
    GimWork *w = GIM(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "gim";
            w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->model == NULL) {
                return -1;
            }
            w->model->anim = effGimAnim;
            floadGetSetRam((void **) w->model->segs, 0x13A);
            w->glow = (PolFrame *) frmCreate(gFrmEffectLayer, gFrmKindPol);
            if (w->glow == NULL) {
                return -1;
            }
            w->glow->dl = (Gfx *) fadeSegToRam(effDrawSetupDl, obj->file);
            w->glow->dl2 = effGimGlowDl;
            floadGetSetRam((void **) w->glow->segs, 0x13A);
            w->frameStep = 0.0f;
            w->frameEnd = 0.0f;
            break;
        case 1:
            frmDestroy(&w->model->f);
            frmDestroy(&w->glow->f);
            break;
        case 0:
            w->model->frame += w->frameStep;
            if (w->frameEnd < w->model->frame) {
                w->model->frame = w->frameEnd;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *effGimMake(Obj *obj) {
    return tskCreate(obj, effGimProc, obj->file, NULL);
}

/* Plays the animation from start to end in steps. */
void effGimSetPlay(Obj *obj, f32 steps, f32 start, f32 end) {
    GIM(obj)->model->frame = start;
    GIM(obj)->frameEnd = end;
    GIM(obj)->frameStep = (end - start) / steps;
}

extern u8 effGim2Anim[]; /* the gim2's two animations, in file 0x13A */
extern u8 effGim2SecondAnim[];
extern Gfx effGim2GlowDl[]; /* the gim2's glow */
extern u8 effGim2RiderNodeAttach; /* the node a rider stands on */

#define ANIM_NODE_POS ((void (*)(void *, f32, u8 *, f32 *, f32 *, f32 *)) mdlAnimNodePos)
#define ANIM_NODE_ROT ((void (*)(void *, f32, u8 *, f32 *, f32 *, f32 *)) mdlAnimNodeRot)

/* "gim2": two "anime" frames playing one animation together to an end, a glow, and a demo
 * human riding a node of the first. */
typedef struct Gim2Work {
    /* 0x00 */ AnimeFrame *model;
    /* 0x04 */ AnimeFrame *model2;
    /* 0x08 */ PolFrame *glow;
    /* 0x0C */ f32 frameStep;
    /* 0x10 */ f32 frameEnd;
    /* 0x14 */ Obj *rider; /* a dhuman */
} Gim2Work;

#define GIM2(obj) ((Gim2Work *) FX(obj))

void effGim2GetRiderPos(Obj *obj, f32 *x, f32 *y, f32 *z, f32 *dir);

s32 effGim2Proc(Obj *obj, s16 msg, void *arg) {
    Gim2Work *w = GIM2(obj);
    s32 pad;
    f32 x;
    f32 y;
    f32 z;
    f32 dir;

    switch (msg) {
        case 2:
            obj->name = "gim2";
            w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->model == NULL) {
                return -1;
            }
            w->model->anim = effGim2Anim;
            floadGetSetRam((void **) w->model->segs, 0x13A);
            w->model2 = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->model2 == NULL) {
                return -1;
            }
            w->model2->anim = effGim2SecondAnim;
            floadGetSetRam((void **) w->model2->segs, 0x13A);
            w->glow = (PolFrame *) frmCreate(gFrmEffectLayer, gFrmKindPol);
            if (w->glow == NULL) {
                return -1;
            }
            w->glow->dl = (Gfx *) fadeSegToRam(effDrawSetupDl, obj->file);
            w->glow->dl2 = effGim2GlowDl;
            floadGetSetRam((void **) w->glow->segs, 0x13A);
            w->frameStep = 0.0f;
            w->frameEnd = 0.0f;
            break;
        case 1:
            frmDestroy(&w->model->f);
            frmDestroy(&w->model2->f);
            frmDestroy(&w->glow->f);
            break;
        case 0:
            w->model->frame += w->frameStep;
            if (w->frameEnd < w->model->frame) {
                w->model->frame = w->frameEnd;
            }
            w->model2->frame = w->model->frame;
            if (w->rider != NULL) {
                effGim2GetRiderPos(obj, &x, &y, &z, &dir);
                dhumanSetPos(w->rider, x, y, z);
                dhumanSetDir(w->rider, dir - 1.5707964f);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *effGim2Make(Obj *obj) {
    return tskCreate(obj, effGim2Proc, obj->file, NULL);
}

/* Plays the animation from start to end in steps. */
void effGim2SetPlay(Obj *obj, f32 steps, f32 start, f32 end) {
    GIM2(obj)->model->frame = start;
    GIM2(obj)->frameEnd = end;
    GIM2(obj)->frameStep = (end - start) / steps;
}

/* Where the rider stands, and which way it faces. */
void effGim2GetRiderPos(Obj *obj, f32 *x, f32 *y, f32 *z, f32 *dir) {
    s32 pad[2];
    f32 rx;
    f32 rz;

    /* The node passed as a pointer at both calls: IDO builds its address again for the second,
     * as the game does. */
    ANIM_NODE_POS(&effGim2RiderNodeAttach, GIM2(obj)->model->frame, (u8 *) GIM2(obj)->model->segs[0], x, y, z);
    ANIM_NODE_ROT(&effGim2RiderNodeAttach, GIM2(obj)->model->frame, (u8 *) GIM2(obj)->model->segs[0], &rx, dir, &rz);
}

void effGim2SetRider(Obj *obj, Obj *rider) {
    GIM2(obj)->rider = rider;
}
