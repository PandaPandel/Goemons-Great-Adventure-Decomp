#include "object.h"
#include "anim.h"
#include "camlight.h"
#include "gfxbuf.h"
#include "gfxmgr.h"
#include "mathutil.h"
#include "polygon.h"

/* The drawable frame kinds. Each one pushes a matrix, sets its segments, draws, and pops.
 * - "pol": a display list with a position, a rotation (x-y-z, radians, kept within two turns)
 *   and a scale. Flags 1/2/4 billboard it (it looks along the view, keeping only the other
 *   rotations), 8 copies its list in, 0x10 sets the camera's look-at (for reflections), 0x20
 *   reloads the lights after it.
 * - "anime": the same with an animated model (anim.c) at a frame.
 * - "mpol" / "mani": the same two with a whole matrix, multiplied by the parent's when the
 *   parent is one of these.
 * - "gpol" / "gani": lighter ones with eleven segments and no lights. A "gani" runs its
 *   animation by the global frame count, offset by its position. */

extern CamView *gCamDrawView; /* the camera being drawn */
extern f32 gCamDrawDirX; /* its view direction */
extern f32 gCamDrawDirY;
extern f32 gCamDrawDirZ;
extern LookAt *gCamDrawLookAt;
Vec3f gPolBillboardRot; /* the angles a billboarded model node turns by (overlay 6's .bss) */
extern f32 gWorldOriginX; /* the world origin */
extern f32 gWorldOriginY;
extern f32 gWorldOriginZ;
extern s16 gFrmKindMpol; /* the kinds */
extern s16 gFrmKindMani;
extern s16 gFrmKindGpol;
extern s16 gFrmKindGani;
extern s16 gFrmKindPol;
extern s16 gFrmKindAnime;
extern u32 gGameFrameCount; /* the frame count */
/* polygon.c's .data (0x80108670-0x8010898C), in ROM order. */
PolFrame gPolPolFrameInit = { /* each kind's new frame */
    { 0 }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, 0x0000, NULL, NULL,
    { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
};
AnimeFrame gPolAnimeFrameInit = {
    { 0 }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, 0x0000, 0.0f, NULL,
    0x00000000, { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
};
MPolFrame gPolMpolFrameInit = {
    { 0 },
    {
        {
            { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f },
        },
    },
    {
        {
            { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f, 0.0f },
        },
    },
    0x0000, NULL, NULL, { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
};
MAniFrame gPolManiFrameInit = {
    { 0 },
    {
        {
            { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f },
        },
    },
    {
        {
            { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f, 0.0f },
        },
    },
    0x0000, 0.0f, NULL, 0x00000000,
    { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
};
GPolFrame gPolGpolFrameInit = {
    { 0 },
    {
        {
            { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f },
        },
    },
    NULL,
    {
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
    },
};
GAniFrame gPolGaniFrameInit = {
    { 0 }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, 0x00000000,
    {
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
    },
};


/* Keeps an angle within two turns either way. */
#define WRAP_ANGLE(a)                \
    if ((a) > 4 * PI) {              \
        (a) -= 4 * PI;               \
    } else if ((a) < -4 * PI) {      \
        (a) += 4 * PI;               \
    }

/* Segments 3-8 from segs (0: left alone). */
Gfx *polSetSegments(Gfx *gfx, u32 *segs) {
    s32 i;

    for (i = 0; i < 6; i++) {
        if (segs[i] != 0) {
            gSPSegment(gfx++, i + 3, OS_K0_TO_PHYSICAL(segs[i]));
        }
    }
    return gfx;
}

/* An "mpol" frame's draw. */
Gfx *polDrawMpol(Gfx *gfx, MPolFrame *f) {
    MPolFrame *parent;
    Mtx *m;

    gfx = camlightLoadLights(gfx);
    m = gfxAllocMtx(1);
    parent = (MPolFrame *) f->f.parent;
    if (parent->f.kind == gFrmKindMpol || parent->f.kind == gFrmKindMani) {
        guMtxCatF(f->mtx.m, parent->world.m, f->world.m);
    } else {
        f->world = f->mtx;
        mathMtxPostTrans(f->world.m, -gWorldOriginX, -gWorldOriginY, -gWorldOriginZ);
    }
    mathMtxFToL(m, f->world.m);
    gSPMatrix(gfx++, K0_TO_PHYS(m), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_PUSH);
    gfx = polSetSegments(gfx, f->segs);
    if ((f->flags & 0x10) && gCamDrawView != NULL) {
        gSPLookAt(gfx++, gCamDrawLookAt);
    }
    if (f->dl != NULL) {
        if (f->flags & 8) {
            gfx = gfxCopyDisplayList(gfx, f->dl);
        } else {
            gSPDisplayList(gfx++, f->dl);
        }
    }
    if (f->dl2 != NULL) {
        gSPDisplayList(gfx++, f->dl2);
    }
    gSPPopMatrix(gfx++, G_MTX_MODELVIEW);
    if (f->flags & 0x20) {
        camlightForceReload();
    }
    return gfx;
}

/* A "gpol" frame's draw. "s32 pad;" first, as in the pol draw, gives the frame (m at 0x80, mf at
 * 0x40). The segment loop and the list read the frame through a copy of f (g), which keeps it in
 * s1 where f itself stays in its home. */
Gfx *polDrawGpol(Gfx *gfx, GPolFrame *f) {
    s32 pad;
    Mtx *m;
    MtxF mf;
    s32 i;
    GPolFrame *g;

    m = gfxAllocMtx(1);
    mf = f->mtx;
    mathMtxPostTrans(mf.m, -gWorldOriginX, -gWorldOriginY, -gWorldOriginZ);
    mathMtxFToL(m, mf.m);
    gSPMatrix(gfx++, K0_TO_PHYS(m), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_PUSH);
    g = f;
    for (i = 0; i < 11; i++) {
        if (g->segs[i] != 0) {
            gSPSegment(gfx++, i + 3, OS_K0_TO_PHYSICAL(g->segs[i]));
        }
    }
    if (g->dl != NULL) {
        gSPDisplayList(gfx++, g->dl);
    }
    gSPPopMatrix(gfx++, G_MTX_MODELVIEW);
    return gfx;
}

/* A "pol" frame's draw. */
Gfx *polDrawPol(Gfx *gfx, PolFrame *f) {
    s32 pad;
    Mtx *m;
    f32 mf[4][4];

    gfx = camlightLoadLights(gfx);
    m = gfxAllocMtx(1);
    WRAP_ANGLE(f->rot[0]);
    WRAP_ANGLE(f->rot[1]);
    WRAP_ANGLE(f->rot[2]);
    if ((f->flags & 7) && gCamDrawView != NULL) {
        mathMtxLookTransScale(mf, gCamDrawDirX, gCamDrawDirY, gCamDrawDirZ, f->pos[0] - gWorldOriginX,
                               f->pos[1] - gWorldOriginY, f->pos[2] - gWorldOriginZ, f->scale[0],
                               f->scale[1], f->scale[2]);
        if (!(f->flags & 1)) {
            mathMtxPreRotX(mf, f->rot[0]);
        }
        if (!(f->flags & 2)) {
            mathMtxPreRotY(mf, f->rot[1]);
        }
        if (!(f->flags & 4)) {
            mathMtxPreRotZ(mf, f->rot[2]);
        }
        mathMtxFToL(m, mf);
    } else {
        mathMtxTransRotScaleL(m, f->pos[0] - gWorldOriginX, f->pos[1] - gWorldOriginY,
                               f->pos[2] - gWorldOriginZ, f->rot[0], f->rot[1], f->rot[2],
                               f->scale[0], f->scale[1], f->scale[2]);
    }
    gSPMatrix(gfx++, K0_TO_PHYS(m), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_PUSH);
    gfx = polSetSegments(gfx, f->segs);
    if ((f->flags & 0x10) && gCamDrawView != NULL) {
        gSPLookAt(gfx++, gCamDrawLookAt);
    }
    if (f->dl != NULL) {
        if (f->flags & 8) {
            gfx = gfxCopyDisplayList(gfx, f->dl);
        } else {
            gSPDisplayList(gfx++, f->dl);
        }
    }
    if (f->dl2 != NULL) {
        gSPDisplayList(gfx++, f->dl2);
    }
    gSPPopMatrix(gfx++, G_MTX_MODELVIEW);
    if (f->flags & 0x20) {
        camlightForceReload();
    }
    return gfx;
}

/* A "gani" frame's draw: its animation's frame is the frame count plus its position, looped
 * over 128.
 * Matched 2026-10-04; it was parked for registers and frame layout, with polDrawMani and
 * polDrawAnime (IDO gave all three mpol's registers, f s1/gfx s0, where the game has f s0/gfx
 * s1): f read through a copy from the third angle's wrap on puts f in s0 and gfx in s1 as the
 * game has them. The billboard angles (gPolBillboardRot) are stored by absolute address in the
 * game; IDO keeps the address in a register for the whole function instead, until overlay 6's
 * .bss is split so polygon.c can define the Vec3f itself. */
Gfx *polDrawGani(Gfx *gfx, GAniFrame *f) {
    f32 mf[4][4];
    s32 i;
    Mtx *m;
    GAniFrame *g;
    s32 count;
    u32 *h;

    m = gfxAllocMtx(1);
    WRAP_ANGLE(f->rot[0]);
    WRAP_ANGLE(f->rot[1]);
    g = f;
    WRAP_ANGLE(g->rot[2]);
    mathMtxTransRotScaleL(m, g->pos[0] - gWorldOriginX, g->pos[1] - gWorldOriginY,
                           g->pos[2] - gWorldOriginZ, g->rot[0], g->rot[1], g->rot[2], g->scale[0],
                           g->scale[1], g->scale[2]);
    gSPMatrix(gfx++, K0_TO_PHYS(m), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_PUSH);
    for (i = 0; i < 11; i++) {
        if (g->segs[i] != 0) {
            gSPSegment(gfx++, i + 3, OS_K0_TO_PHYSICAL(g->segs[i]));
        }
    }
    if (g->anim != 0) {
        gPolBillboardRot.x = g->rot[0];
        gPolBillboardRot.y = g->rot[1];
        gPolBillboardRot.z = g->rot[2];
        h = (u32 *) gfxSegmentToRam(g->anim, g->segs);
        count = *h & 0xFF;
        animSeek(
            g->anim,
            (f32) count *
                (f32) ((s32) ((f32) gGameFrameCount + ((Vec3f *) g->pos)->x +
                              ((Vec3f *) g->pos)->y + ((Vec3f *) g->pos)->z) &
                       0x7F) /
                128,
            g->segs);
        gfx = polDrawAnimModel(gfx, g->anim, g->segs);
    }
    gSPPopMatrix(gfx++, G_MTX_MODELVIEW);
    return gfx;
}

/* A "mani" frame's draw.
 * Matched 2026-10-03: the mpol draw's C plus the anim block, with two FAKEs. (1) The
 * address-only use of &f->world after the segment call keeps the world temp alive past the segs
 * temp's birth, so the two get the game's separate slots (0x28, 0x24). (2) uopt colours by
 * priority (references over blocks spanned, IDO_NOTES 246) and the game gives f the first
 * saved register (s0) and gfx the second; here gfx outranks f. Seven empty blocks after f's last
 * use (the flags test), where gfx alone is live, lower gfx's priority alone and give the game's
 * f s0 / gfx s1 (six are not enough; FAKE uses of f in its blocks do not reach it). Measured
 * before the match (the line counts are check's differing lines):
 * the first form, 90 lines. Also tried, none of which keeps the two temps apart or
 * moves f to s0: the zeroes as a chain (x = y = z stores z first; z = y = x uses three float
 * registers); the 0C9C call, gSPMatrix and the 5D70 call on one line (still one temp slot); a
 * dead "parent = f" initializer and an empty "if (f) {}" before the parent read (f stays in s1).
 * With s0/s1 swapped, the game's mpol and mani are the same code up to the anim block. The world
 * temp takes the first temp slot in both (mpol 0x24 under six _g, mani 0x28 under five), so the
 * difference is the second temp: the game gives segs its own slot (0x24), and IDO here shares one.
 * 2026-10-03 (90 to 78, by hand): a FAKE address-only use of &f->world right after the
 * segment call keeps the world temp alive past the segs temp's birth, so the two get the game's
 * separate slots (0x28, 0x24); pad[2] then goes, and the frame and every slot are the game's
 * (check prints no stack line). Left: f s1 / gfx s0 where the game has f s0 / gfx s1. The
 * copies (best 75, but an instruction longer), empties (55) and splits (29) sweeps on this
 * form found nothing below 78. gfx as a new local set from camlightLoadLights's return (the
 * parameter renamed, so f is assigned first): 90, the local takes a slot (0x20).
 * 2026-10-03: the anime draw (polDrawAnime) matched with its copy g of f DECLARED FIRST;
 * the copies sweep here with each copy declared first as well as last (64 tries): the same 75 at
 * best (gfx through a copy, an instruction longer), so that lever does not carry here.
 * 2026-10-03 (scratch compiles, f's register read off the prologue): f stays s1 under FAKE
 * address-only uses of f ("&((u8 *) f)[(u32) f]", up to eight "&f->field" tests in one block),
 * the same for gfx, the world FAKE moved to any later block, a copy of f (into a new g, or into
 * the dead parent) at any point, gfx as a new local, "gSPMatrix(gfx, ..); 5D70(gfx + 1, ..)",
 * "if (f->anim)". Only 24 such uses over three blocks (code and frame change) put f in s0. A copy
 * of gfx from the segment call on ("g = gfx; g = 5D70(g, ..)") puts f in s0 but leaves the first
 * gfx in its home and g in a3: the game's gfx is one live range. FAKE empty blocks after f's last
 * use (the flags test) lower gfx's priority alone: five or six keep f in s1, seven (or eight, in
 * the if body or after it) give f s0 / gfx s1: MATCH (check, 148 lines). The 2-use test of f
 * above plus five empty blocks also flips it (worth about two blocks). Empty "if (G) {}" tests,
 * labels, nested do-whiles and other tail shapes (an early return, gotos) add no block; a
 * "register" f is ignored. Unparked, compiled as the build does: MATCH.
 * 2026-10-03 (fakes sweep): without the empty blocks, gfx as a new local DECLARED FIRST (the
 * parameter renamed dl) with the parent read inline, so the local takes the parent slot: the
 * frame is the game's (mix distance 0) but f stays s1, 90 lines. Both FAKEs stay.
 * 2026-10-04 (fakes round 2, read off uopt itself: -zdbug:5 priorities, -zdbug:3 code motion
 * sets; measured on a scratch copy). Both FAKEs stay; what the original had:
 * (1) The world FAKE. uopt gives spilled CSE temps their homes block by block as it re-emits
 * (findbbtemps, gettemp): a home is reused unless an expression holding it is in the block's
 * antout set (bb+0x15c: anticipated on every path from the block's end). &f->world (f + 0x54)
 * is last evaluated for the matrix conversion and is not anticipated after it, so the segs
 * temp (f + 0xA4) takes its home. With the FAKE it stays anticipated through the segment call
 * and the two get the game's homes (world 0x28, segs 0x24). So the original evaluated
 * &f->world (or f->world.m) again after the matrix conversion, on every path, in something
 * that leaves no instruction (as the empty test does). Measured: a segment-table local
 * ("segs = f->segs;" before the call, used by all three calls) with the parent read inline,
 * the local in the parent word: two homes with no FAKE and the game's frame, 12 lines (mix
 * distance 0), but reversed (world 0x24, segs 0x28): the inline parent read takes the first
 * home, world the second, segs reuses the first. The parent inline alone or with an unused
 * word does the same; segs set at the top with the parent kept, or a world pointer local
 * ("w = &f->world"), share one home again; a dead "if (0) call(f->world.m);" is gone before
 * code motion and extends nothing.
 * (2) The empties. Priority = references / units, units about blocks spanned / 4 + 1: f has
 * 15 references, gfx 22. Without the world FAKE f spans 24 blocks (15/7 = 2.14) and gfx 27
 * (22/8 = 2.75); the world FAKE adds two blocks to both (f 15/8 = 1.875). For the game's f s0
 * the original had, with no instruction left, either about 11 more blocks after the last
 * flags test where only gfx is live (14 here, against the world FAKE), or at least 20
 * references to f, or at most 16 to gfx. Each "do {} while (0);" is two blocks; a dead
 * "if (0) call(gfx);" one (ten give gfx 22/10, still above f). polygon.c has no debug strings
 * (its .rodata is the six kind names and the angle constants), so the tail was not
 * compiled-out PRINTFs with literals. */
Gfx *polDrawMani(Gfx *gfx, MAniFrame *f) {
    MAniFrame *parent;
    Mtx *m;

    gfx = camlightLoadLights(gfx);
    m = gfxAllocMtx(1);
    parent = (MAniFrame *) f->f.parent;
    if (parent->f.kind == gFrmKindMpol || parent->f.kind == gFrmKindMani) {
        guMtxCatF(f->mtx.m, parent->world.m, f->world.m);
    } else {
        f->world = f->mtx;
        mathMtxPostTrans(f->world.m, -gWorldOriginX, -gWorldOriginY, -gWorldOriginZ);
    }
    mathMtxFToL(m, f->world.m);
    gSPMatrix(gfx++, K0_TO_PHYS(m), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_PUSH);
    gfx = polSetSegments(gfx, f->segs);
    if (!&f->world) {} /* FAKE */
    if ((f->flags & 0x10) && gCamDrawView != NULL) {
        gSPLookAt(gfx++, gCamDrawLookAt);
    }
    if (f->dl != NULL) {
        if (f->flags & 8) {
            gfx = gfxCopyDisplayList(gfx, f->dl);
        } else {
            gSPDisplayList(gfx++, f->dl);
        }
    }
    if (f->anim != 0) {
        gPolBillboardRot.x = 0.0f;
        gPolBillboardRot.y = 0.0f;
        gPolBillboardRot.z = 0.0f;
        animSeek(f->anim, f->frame, f->segs);
        gfx = polDrawAnimModel(gfx, f->anim, f->segs);
    }
    gSPPopMatrix(gfx++, G_MTX_MODELVIEW);
    if (f->flags & 0x20) {
        camlightForceReload();
    }
    /* FAKE: OoT's `if (1) {}`, seven: empty blocks where gfx is live and f is not */
    if (1) {}
    if (1) {}
    if (1) {}
    if (1) {}
    if (1) {}
    if (1) {}
    if (1) {}
    return gfx;
}

/* An "anime" frame's draw. A billboarded one hands its model the angles it turned by.
 * Matched 2026-10-03: f read through a copy g from the third angle wrap on (the gani draw has the
 * same copy, IDO_NOTES 241), and g declared FIRST: its word is the frame's top slot (0x9C), which
 * puts the f->segs temp at the game's 0x3C (declared last, g took 0x54 and the temp 0x38).
 * History of the forms measured before (53 lines; see the end for the 50-line loop form):
 * only the stores to gPolBillboardRot differ (pad2 puts the f->segs temp at 0x3C, as in
 * the original). The original stores the else branch's three angles by absolute address,
 * x and y under one "lui at" (as1 shares it only for a symbol DEFINED in the file: this waits
 * for polygon.c's .bss to move into the C), and the billboard's x absolute with only y/z
 * through v0. IDO here keeps &gPolBillboardRot in v0 for all six. Defining it in the file (static,
 * global, or as f32[3]) did not change that; a (non-unrolled) loop in the else branch did make
 * the billboard's x absolute, so the original's else-branch stores are not uses of that
 * address CSE. Form not found yet.
 * Measured 2026-10-02: the
 * three gPolBillboardRot stores of the rotation as a loop over the pad local, "for (pad = 0; pad < 3;
 * pad++) ((f32 *) &gPolBillboardRot)[pad] = f->rot[pad];", 50 (from 53); with a FAKE block split
 * inside that loop, 53. gPolBillboardRot declared volatile (2026-10-03): 55, the base still in v0.
 * 2026-10-03, read off the game: the mani draw's three zero stores DO go through the v0 base
 * (its lui hoisted above the beql), so the Vec3f fits it; only stores of f->rot[i] (here and in
 * the gani draw) and the billboard's x (stored in the second atan call's delay slot) are
 * absolute. Measured: FAKE boundaries between the else's three stores, 53 (each store builds
 * its own v0 base, even alone); the same with the Vec3f DEFINED in the file, 53 (a struct's
 * fields always get the base here); three floats defined in the file with polDrawModelNode
 * reading "((Vec3f *) &gPolBillboardRot)->x" (713C still matches; this one 81, every store absolute,
 * the billboard's y/z too where the game has the base; mani 105; gani 119 from 124); the else's
 * copy as "for (i = 0; i < 3; i++) ((f32 *) &gPolBillboardRot)[i] = f->rot[i];" with its own i: 86
 * (kept rolled, a pointer walk to 0x8017324C, i takes a slot). The gani draw's fix (a copy of f
 * from the third wrap on, IDO_NOTES 241; sweep B's find, score 410 → 182): 63 in both the uniform
 * form and the find's own (the copy declared second, the billboard branch's first reads through
 * f), not kept.
 * 2026-10-03, with the Vec3f now DEFINED in polygon.c: the current C (straight stores in
 * both branches) 53, all six through v0. The else's stores as the loop over pad: 50, and the
 * billboard branch then matches the game (x absolute, y/z through v0): the else's straight
 * stores are what made IDO keep the address for the whole function. The loop stays rolled (a
 * walk to 0x8017324C); the else still needs three absolute stores. The else's stores through a
 * pointer local ("v = &gPolBillboardRot; v->x = .."; v in pad's place): 53, the same as straight.
 * The loop with a FAKE "do {} while (0);" or "if (0) {}" in its body: 50, still rolled.
 * "tools/permuter.py unroll": no step from limit 16 to 1280 (this build never unrolls the 3-trip
 * one-block loop, IDO_NOTES 44). A second name for the billboard's three stores
 * (gPolBillboardRot_bb: "#pragma weak" was ignored, cfe warning 780, so for the measurement only): 26,
 * 255 instructions, the billboard branch exact; the else's three straight stores (now the only
 * references to the name, one block) STILL go through v0, though the gani draw's same three
 * statements come out absolute. So it is not the cross-branch use alone. With that, the else's
 * loads through a copy ("g = f;" in pad's place, as the gani draw reads them): 26, no change;
 * "tools/permuter.py splits" (a boundary after every statement): 26 everywhere. The loop with a
 * dead second block in its body ("if (pad == 3) f->rot[pad] = 0.0f;", to see whether a
 * several-block 3-trip loop is unrolled completely here as a 4-trip one is): 184, not unrolled.
 * The loop, its index i in the first local's place (was "pad"; 50 either way), NOT kept (50
 * lines but mix distance 10 for the straight form's 3, and the walk is not the game's code;
 * only the else's three stores differ there: a rolled walk where the game
 * has "lui at; swc1 %lo(D)(at); swc1 %lo(D+4)(at); lui at; swc1 %lo(D+8)(at)"). Per IDO_NOTES 15
 * those per-element luis are an unrolled loop in the original; this build does not unroll a
 * 3-trip one-block loop (IDO_NOTES 44, the open unrolling difference), so the original's loop
 * cannot be reproduced here yet. The other way: straight stores whose name is not shared with
 * the billboard (an undefined_syms.txt name) still leave the else in v0 (the 26 above).
 * 2026-10-03: the game's else stores [0], [1], [2] in order, each from its own load (a
 * complete unroll of a 4-trip loop came out rotated in the game, [1] [2] [3] [0], IDO_NOTES 44).
 * The loop form compiled with IDO 5.3 (IDO=ido-5.3, measurement only): also rolled, 50, mix
 * distance 10, so the older compiler does not unroll it either. The loop form with uopt options
 * (IDO_EXTRA, measurement only): -Wo,-nomultibbunroll and -Wo,-loopunroll,8 the same 50;
 * -Wo,-loopunroll,3 and ,2 unroll it (mix distance 5, 55 lines) but not as the game: [0]
 * absolute, [1] and [2] through an index still computed (sll; addu base). Whether the game's
 * compiler unrolls a top-level 3-trip loop is open: the game's tiny 3-trip loops that stay
 * rolled (matched mathutil.c mathMatInv3, vanime.c vanimeMakeVtx) are all inner loops, and the
 * gani draw's straight stores give the same per-element luis, so the pattern proves neither.
 * 2026-10-03, the greedy mode (second pass; 53 to 22, mix distance 3 to 0): f read through a
 * copy g from the third angle wrap on (the gani draw has the same copy), pad2 dropped. */
Gfx *polDrawAnime(Gfx *gfx, AnimeFrame *f) {
    AnimeFrame *g;
    Mtx *m;
    f32 mf[4][4];

    gfx = camlightLoadLights(gfx);
    m = gfxAllocMtx(1);
    WRAP_ANGLE(f->rot[0]);
    WRAP_ANGLE(f->rot[1]);
    g = f;
    WRAP_ANGLE(g->rot[2]);
    if ((g->flags & 7) && gCamDrawView != NULL) {
        mathMtxLookTransScale(mf, gCamDrawDirX, gCamDrawDirY, gCamDrawDirZ, g->pos[0] - gWorldOriginX,
                               g->pos[1] - gWorldOriginY, g->pos[2] - gWorldOriginZ, g->scale[0],
                               g->scale[1], g->scale[2]);
        if (!(g->flags & 1)) {
            mathMtxPreRotX(mf, g->rot[0]);
        }
        if (!(g->flags & 2)) {
            mathMtxPreRotY(mf, g->rot[1]);
        }
        if (!(g->flags & 4)) {
            mathMtxPreRotZ(mf, g->rot[2]);
        }
        mathMtxFToL(m, mf);
        gPolBillboardRot.x = mathAtan2(mf[1][1], mf[2][1]) - PI / 2;
        gPolBillboardRot.y = mathAtan2(mf[2][0], mf[2][2]);
        gPolBillboardRot.z = 0.0f;
    } else {
        mathMtxTransRotScaleL(m, g->pos[0] - gWorldOriginX, g->pos[1] - gWorldOriginY,
                               g->pos[2] - gWorldOriginZ, g->rot[0], g->rot[1], g->rot[2],
                               g->scale[0], g->scale[1], g->scale[2]);
        gPolBillboardRot.x = g->rot[0];
        gPolBillboardRot.y = g->rot[1];
        gPolBillboardRot.z = g->rot[2];
    }
    gSPMatrix(gfx++, K0_TO_PHYS(m), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_PUSH);
    gfx = polSetSegments(gfx, g->segs);
    if ((g->flags & 0x10) && gCamDrawView != NULL) {
        gSPLookAt(gfx++, gCamDrawLookAt);
    }
    if (g->dl != NULL) {
        if (g->flags & 8) {
            gfx = gfxCopyDisplayList(gfx, g->dl);
        } else {
            gSPDisplayList(gfx++, g->dl);
        }
    }
    if (g->animAddr != 0) {
        animSeek(g->animAddr, g->frame, g->segs);
        gfx = polDrawAnimModel(gfx, g->animAddr, g->segs);
    }
    gSPPopMatrix(gfx++, G_MTX_MODELVIEW);
    if (g->flags & 0x20) {
        camlightForceReload();
    }
    return gfx;
}

/* The last frame of an "anime" frame's animation. */
s32 polAnimeLastFrame(AnimeFrame *f) {
    return (*(u32 *) gfxSegmentToRam(f->animAddr, f->segs) & 0xFF) - 1;
}

/* The last frame of an animation. */
s32 polAnimLastFrame(u32 anim, u32 *segs) {
    return (*(u32 *) gfxSegmentToRam(anim, segs) & 0xFF) - 1;
}

/* Draws an animation's model: its header's first word counts two lists to skip, then the
 * root nodes to draw. (The first loop's test is written "a > i": IDO leaves it rolled, with
 * slt; "i < a" is unrolled by 4. The header word in a local gives the bounds' copies.) */
Gfx *polDrawAnimModel(Gfx *gfx, u32 addr, u32 *segs) {
    u32 *h;
    u32 *p;
    s32 i;
    u32 w;
    s32 a;
    s32 b;
    s32 n;

    h = (u32 *) gfxSegmentToRam(addr, segs);
    if (h == NULL) {
        return gfx;
    }
    p = h + 2;
    w = h[0];
    a = (w >> 28) & 7;
    b = (w >> 24) & 0xF;
    n = (w >> 8) & 0x7FFF;
    for (i = 0; a > i; i++) {
        p++;
    }
    for (i = 0; i < b; i++) {
        p++;
    }
    for (i = 0; i < n; i++) {
        gfx = polDrawModelNode(gfx, (ModelNode *) gfxSegmentToRam(*p, segs));
        p++;
    }
    return gfx;
}

/* Draws a model node, its children, then its siblings. A node with bit 14 set in a rotation
 * track is billboarded on that axis. (The tracks' OR grouped as the game loads them; the
 * child and sibling as array steps.) */
Gfx *polDrawModelNode(Gfx *gfx, ModelNode *node) {
    f32 look[4][4];
    f32 pad[4][4];
    f32 rot[4][4];
    Mtx *m;
    f32 x;
    f32 y;
    f32 z;

    m = gfxAllocMtx(1);
    polModelNodeMtx(m, node);
    gSPMatrix(gfx++, K0_TO_PHYS(m), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    if (((node->rot[2] | (node->rot[1] | node->rot[0])) & 0x4000) && gCamDrawView != NULL) {
        x = (node->rot[0] & 0x4000) ? gPolBillboardRot.x : 0.0f;
        y = (node->rot[1] & 0x4000) ? gPolBillboardRot.y : 0.0f;
        z = (node->rot[2] & 0x4000) ? gPolBillboardRot.z : 0.0f;
        mathMtxRotInv(rot, x, y, z);
        x = ((node->rot[1] & 0x4000) || (node->rot[2] & 0x4000)) ? gCamDrawDirX : 0.0f;
        y = ((node->rot[2] & 0x4000) || (node->rot[0] & 0x4000)) ? gCamDrawDirY : 0.0f;
        z = ((node->rot[0] & 0x4000) || (node->rot[1] & 0x4000)) ? gCamDrawDirZ : 0.0f;
        mathMtxLookDir(look, x, y, z);
        guMtxCatF(look, rot, look);
        m = gfxAllocMtx(1);
        mathMtxFToL(m, look);
        gSPMatrix(gfx++, K0_TO_PHYS(m), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
        if (node->dl != 0) {
            gSPDisplayList(gfx++, node->dl);
        }
        gSPPopMatrix(gfx++, G_MTX_MODELVIEW);
    } else {
        if (node->dl != 0) {
            gSPDisplayList(gfx++, node->dl);
        }
    }
    if (node->child != 0) {
        gfx = polDrawModelNode(gfx, node + node->child);
    }
    gSPPopMatrix(gfx++, G_MTX_MODELVIEW);
    if (node->sibling != 0) {
        gfx = polDrawModelNode(gfx, node + node->sibling);
    }
    return gfx;
}

/* A model node's matrix at the animation's frame: rotation x-y-z, position (1/32 units) and
 * scale per row (1/256).
 * The locals are declared in the game's frame order (0xB8, read off its asm: each sine just
 * above its cosine); the zero column is one chain written inner-first, and sxsy then cxsy are
 * computed before mf[0][0], as the game's float registers show. */
void polModelNodeMtx(Mtx *m, ModelNode *node) {
    f32 sinX;
    f32 cosX;
    f32 sinY;
    f32 cosY;
    f32 sinZ;
    f32 cosZ;
    f32 sx;
    f32 sz;
    f32 cxsy;
    f32 sy;
    f32 sxsy;
    u16 rot[3];
    s32 pad0;
    f32 mf[4][4];
    f32 x;
    f32 y;
    f32 z;

    animGetRot(node, rot);
    sinX = mathSinCos(rot[0], &cosX);
    sinY = mathSinCos(rot[1], &cosY);
    sinZ = mathSinCos(rot[2], &cosZ);
    sxsy = sinX * sinY;
    cxsy = cosX * sinY;
    mf[0][0] = cosY * cosZ;
    mf[0][1] = cosY * sinZ;
    mf[0][2] = -sinY;
    mf[1][0] = sxsy * cosZ - cosX * sinZ;
    mf[1][1] = sxsy * sinZ + cosX * cosZ;
    mf[1][2] = sinX * cosY;
    mf[2][0] = cxsy * cosZ + sinX * sinZ;
    mf[2][1] = cxsy * sinZ - sinX * cosZ;
    mf[2][2] = cosX * cosY;
    x = animGetPos(node->pos[0]) * 0.03125f;
    mf[3][0] = x;
    y = animGetPos(node->pos[1]) * 0.03125f;
    mf[3][1] = y;
    z = animGetPos(node->pos[2]) * 0.03125f;
    mf[3][2] = z;
    mf[0][3] = mf[1][3] = mf[2][3] = 0.0f;
    mf[3][3] = 1.0f;
    sx = animGetScale(node->scale[0]) * 0.00390625f;
    sy = animGetScale(node->scale[1]) * 0.00390625f;
    sz = animGetScale(node->scale[2]) * 0.00390625f;
    if (sx != 1.0f) {
        mf[0][0] *= sx;
        mf[0][1] *= sx;
        mf[0][2] *= sx;
    }
    if (sy != 1.0f) {
        mf[1][0] *= sy;
        mf[1][1] *= sy;
        mf[1][2] *= sy;
    }
    if (sz != 1.0f) {
        mf[2][0] *= sz;
        mf[2][1] *= sz;
        mf[2][2] *= sz;
    }
    mathMtxFToL(m, mf);
}

/* Make the kinds, of count frames each. */
void polInitPolKind(s32 count) {
    gFrmKindPol = frmNewKind("pol", (FrameDraw) polDrawPol,
                                        sizeof(PolFrame), count, &gPolPolFrameInit);
}

void polInitAnimeKind(s32 count) {
    gFrmKindAnime = frmNewKind("anime", (FrameDraw) polDrawAnime,
                                        sizeof(AnimeFrame), count, &gPolAnimeFrameInit);
}

void polInitMpolKind(s32 count) {
    gFrmKindMpol = frmNewKind("mpol", (FrameDraw) polDrawMpol,
                                        sizeof(MPolFrame), count, &gPolMpolFrameInit);
}

void polInitManiKind(s32 count) {
    gFrmKindMani = frmNewKind("mani", (FrameDraw) polDrawMani,
                                        sizeof(MAniFrame), count, &gPolManiFrameInit);
}

void polInitGpolKind(s32 count) {
    gFrmKindGpol = frmNewKind("gpol", (FrameDraw) polDrawGpol,
                                        sizeof(GPolFrame), count, &gPolGpolFrameInit);
}

void polInitGaniKind(s32 count) {
    gFrmKindGani = frmNewKind("gani", (FrameDraw) polDrawGani,
                                        sizeof(GAniFrame), count, &gPolGaniFrameInit);
}
