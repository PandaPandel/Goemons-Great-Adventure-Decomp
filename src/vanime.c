#include "common.h"
#include "object.h"
#include "fade.h"
#include "vanime.h"

/* "vanime" (overlay 10): vertex animations. A file holds a header (vertex count, keyframe
 * count, the texture coordinates and each keyframe's vertices); the functions here build a
 * vertex list between two keyframes at a float frame, and step the frame (once, back, to and
 * fro). One set keeps the frame in globals, the other in a task's work. */

typedef struct VAnime {
    /* 0x00 */ s16 count; /* vertices */
    /* 0x02 */ s16 frames; /* keyframes */
    /* 0x04 */ u32 tcs; /* each vertex's s, t (s16 each) */
    /* 0x08 */ u32 keys[5]; /* each keyframe's vertices (VKey) */
} VAnime; /* size 0x1C */

typedef struct VKey {
    /* 0x0 */ s16 pos[3];
    /* 0x6 */ s8 col[3];
} VKey; /* size 0xA */

/* A task's vertex animation: its frame, direction and the vertex being made. */
typedef struct VAnimeWork {
    /* 0x00 */ f32 frame;
    /* 0x04 */ s32 back; /* 1 while it plays backward */
    /* 0x08 */ s32 i;
} VAnimeWork;

#define VANIME(task) ((VAnimeWork *) ((u32) (task) + sizeof(Obj)))

s32 gVanimeVtxIndex = 0; /* the vertex being made */
f32 gVanimeFrame = 0.0f; /* the frame */
s32 gVanimeBackward = 0; /* 1 while it plays backward */

/* The vertex list at the frame gVanimeFrame, from the animation at addr in file, into v (which
 * needs one vertex more as scratch). Returns the end of the list.
 * The (s16)/(s8) casts give the game's schedule (nops and all). The old values in named locals
 * (x, y, z, w; x and y again for cn[1], cn[2]) give its addu order, result first (inline, all
 * six addu take the old value first). The original has no separate texture-coordinate pointer:
 * k reads the s, t (pos[0], pos[1] sit at offsets 0 and 2); with an "s16 *tc" the frame is 8
 * bytes too big. j is declared above h: the game has 8 bytes of locals above the header. */
MVtx *vanimeMakeVtx(MVtx *v, u32 addr, u32 file) {
    VKey *k;
    s32 j;
    VAnime h;
    f32 t;
    s16 x, y, z, w;

    h = *(VAnime *) fadeSegToRam(addr, file);
    for (gVanimeVtxIndex = 0; gVanimeVtxIndex < h.count; gVanimeVtxIndex++) {
        k = fadeSegToRam(h.tcs + gVanimeVtxIndex * 4, file);
        v->tc[0] = k->pos[0];
        v->tc[1] = k->pos[1];
        k = fadeSegToRam(h.keys[(s32) gVanimeFrame] + gVanimeVtxIndex * sizeof(VKey), file);
        for (j = 0; j < 3; j++) {
            v->ob[j] = k->pos[j];
            v->cn[j] = k->col[j];
        }
        v++;
        k = fadeSegToRam(h.keys[(s32) gVanimeFrame + 1] + gVanimeVtxIndex * sizeof(VKey),
                                   file);
        for (j = 0; j < 3; j++) {
            v->ob[j] = k->pos[j];
            v->cn[j] = k->col[j];
        }
        t = gVanimeFrame;
        while (t > 1.0f) {
            t -= 1.0f;
        }
        x = v[-1].ob[0]; v[-1].ob[0] = x + (s16) ((v->ob[0] - x) * t);
        y = v[-1].ob[1]; v[-1].ob[1] = y + (s16) ((v->ob[1] - y) * t);
        z = v[-1].ob[2]; v[-1].ob[2] = z + (s16) ((v->ob[2] - z) * t);
        w = v[-1].cn[0]; v[-1].cn[0] = w + (s8) ((v->cn[0] - w) * t);
        x = v[-1].cn[1]; v[-1].cn[1] = x + (s8) ((v->cn[1] - x) * t);
        y = v[-1].cn[2]; v[-1].cn[2] = y + (s8) ((v->cn[2] - y) * t);
    }
    return v;
}

/* Steps gVanimeFrame by speed: mode 0 once to the end (or to `end`), 1 back to the start, 2 to
 * and fro, 6 sets it to `end`. 1 when it reached an end. */
s32 vanimeStep(u32 addr, u32 file, s32 mode, f32 speed, f32 end) {
    VAnime h;

    h = *(VAnime *) fadeSegToRam(addr, file);
    if (gVanimeBackward == 0) {
        gVanimeFrame += speed;
    } else {
        gVanimeFrame -= speed;
    }
    if (mode == 0) {
        if (end != 0.0f) {
            if (end < gVanimeFrame) {
                gVanimeFrame = end;
                return 1;
            }
        } else if (h.frames - 1 <= gVanimeFrame) {
            gVanimeFrame = h.frames - 1.001;
            return 1;
        }
    } else if (mode == 1) {
        if (end != 0.0f) {
            if (end < gVanimeFrame) {
                gVanimeFrame = 0.0f;
                return 1;
            }
        } else if (h.frames - 1.001 < gVanimeFrame) {
            gVanimeFrame = 0.0f;
            return 1;
        }
    } else if (mode == 2) {
        if (end != 0.0f) {
            if (end < gVanimeFrame) {
                gVanimeBackward = 1;
                gVanimeFrame = end - 0.01;
                return 1;
            }
            if (gVanimeFrame < 0.0f) {
                gVanimeBackward = 0;
                gVanimeFrame = 0.01f;
                return 1;
            }
        } else {
            if (h.frames - 1 < gVanimeFrame) {
                gVanimeBackward = 1;
                gVanimeFrame = h.frames - 1.01;
                return 1;
            }
            if (gVanimeFrame < 0.0f) {
                gVanimeBackward = 0;
                gVanimeFrame = 0.01f;
                return 1;
            }
        }
    } else if (mode == 6) {
        gVanimeFrame = end;
    }
    return 0;
}

/* vanimeMakeVtx for a task: the frame is the task's own.
 * The list is walked through a local copy of the parameter, "v = vtx;" as the FIRST statement
 * (before the call), declared last (in the slot a pad held). That copy is what gives obj s5
 * (shared with the keys' stride 10) and v s0, and the game's prologue order: obj's copy first,
 * v's after "sw ra; sw s3", then "move a1,a3; jal; move a0,a2" (matched 2026-10-03). Without
 * the copy: obj s1 (shared with the copy loops' bound 3), 18 lines; with a FAKE
 * "if (addr == 0) {}" after the call instead, every register right but the prologue's order
 * (v's copy first, a0 before the jal), 6 lines. The copy AFTER the call: the parameter lives
 * across it in s5, obj in s1, and an extra "move s0,s5". VANIME(obj) inline instead of w: 57
 * (offsets fold to 0x28). */
MVtx *vanimeMakeVtxTask(Obj *obj, MVtx *vtx, u32 addr, u32 file) {
    VAnimeWork *w = VANIME(obj);
    VKey *k;
    s32 j;
    VAnime *p;
    VAnime h;
    f32 t;
    s16 x, y, z, q;
    MVtx *v;

    v = vtx;
    p = fadeSegToRam(addr, file);
    h = *p;
    for (w->i = 0; w->i < h.count; w->i++) {
        k = fadeSegToRam(h.tcs + w->i * 4, file);
        v->tc[0] = k->pos[0];
        v->tc[1] = k->pos[1];
        k = fadeSegToRam(h.keys[(s32) w->frame] + w->i * sizeof(VKey), file);
        for (j = 0; j < 3; j++) {
            v->ob[j] = k->pos[j];
            v->cn[j] = k->col[j];
        }
        v++;
        k = fadeSegToRam(h.keys[(s32) w->frame + 1] + w->i * sizeof(VKey), file);
        for (j = 0; j < 3; j++) {
            v->ob[j] = k->pos[j];
            v->cn[j] = k->col[j];
        }
        t = w->frame;
        while (t > 1.0f) {
            t -= 1.0f;
        }
        x = v[-1].ob[0]; v[-1].ob[0] = x + (s16) ((v->ob[0] - x) * t);
        y = v[-1].ob[1]; v[-1].ob[1] = y + (s16) ((v->ob[1] - y) * t);
        z = v[-1].ob[2]; v[-1].ob[2] = z + (s16) ((v->ob[2] - z) * t);
        q = v[-1].cn[0]; v[-1].cn[0] = q + (s8) ((v->cn[0] - q) * t);
        x = v[-1].cn[1]; v[-1].cn[1] = x + (s8) ((v->cn[1] - x) * t);
        y = v[-1].cn[2]; v[-1].cn[2] = y + (s8) ((v->cn[2] - y) * t);
    }
    return v;
}

/* vanimeStep for a task. */
s32 vanime_act(Obj *obj, u32 addr, u32 file, s32 mode, f32 speed, f32 end) {
    VAnimeWork *w = VANIME(obj);
    s32 pad;
    VAnime h;

    h = *(VAnime *) fadeSegToRam(addr, file);
    if (w->back == 0) {
        w->frame += speed;
    } else {
        w->frame -= speed;
    }
    PRINTF("[vanime_act] L->vtimer[%f / %d]\n", w->frame, h.frames);
    if (mode == 0) {
        if (end != 0.0f) {
            if (end < w->frame) {
                w->frame = end;
                return 1;
            }
        } else if (h.frames - 1 <= w->frame) {
            w->frame = h.frames - 1.001;
            return 1;
        }
    } else if (mode == 1) {
        if (end != 0.0f) {
            if (end < w->frame) {
                w->frame = 0.0f;
                return 1;
            }
        } else if (h.frames - 1.001 < w->frame) {
            w->frame = 0.0f;
            return 1;
        }
    } else if (mode == 2) {
        if (end != 0.0f) {
            if (end < w->frame) {
                w->back = 1;
                w->frame = end - 0.01;
                return 1;
            }
            if (w->frame < 0.0f) {
                w->back = 0;
                w->frame = 0.01f;
                return 1;
            }
        } else {
            if (h.frames - 1 < w->frame) {
                w->back = 1;
                w->frame = h.frames - 1.001;
                return 1;
            }
            if (w->frame < 0.0f) {
                w->back = 0;
                w->frame = 0.01f;
                return 1;
            }
        }
    } else if (mode == 6) {
        w->frame = end;
    }
    return 0;
}

/* One vertex pair at the frame gVanimeFrame, for every vertex in turn: a is set from the frame's
 * key, b from the next, and a moves toward b only where all three coordinates differ. */
MVtx *vanimeMakeVtxPair(MVtx *a, MVtx *b, u32 addr, u32 file) {
    VKey *k;
    s32 pad1;
    VAnime h;
    f32 t;
    s32 pad[3];

    h = *(VAnime *) fadeSegToRam(addr, file);
    for (gVanimeVtxIndex = 0; gVanimeVtxIndex < h.count; gVanimeVtxIndex++) {
        k = fadeSegToRam(h.tcs + gVanimeVtxIndex * 4, file);
        a->tc[0] = k->pos[0];
        a->tc[1] = k->pos[1];
        k = fadeSegToRam(h.keys[(s32) gVanimeFrame] + gVanimeVtxIndex * sizeof(VKey), file);
        a->ob[0] = k->pos[0];
        a->cn[0] = k->col[0];
        a->ob[1] = k->pos[1];
        a->cn[1] = k->col[1];
        a->ob[2] = k->pos[2];
        a->cn[2] = k->col[2];
        k = fadeSegToRam(h.keys[(s32) gVanimeFrame + 1] + gVanimeVtxIndex * sizeof(VKey),
                                   file);
        b->ob[0] = k->pos[0];
        b->cn[0] = k->col[0];
        b->ob[1] = k->pos[1];
        b->cn[1] = k->col[1];
        b->ob[2] = k->pos[2];
        b->cn[2] = k->col[2];
        t = gVanimeFrame;
        while (t > 1.0f) {
            t -= 1.0f;
        }
        if (b->ob[0] != a->ob[0] && b->ob[1] != a->ob[1] && b->ob[2] != a->ob[2]) {
            a->ob[0] = (s16) ((b->ob[0] - a->ob[0]) * t) + a->ob[0];
            a->ob[1] = (s16) ((b->ob[1] - a->ob[1]) * t) + a->ob[1];
            a->ob[2] = (s16) ((b->ob[2] - a->ob[2]) * t) + a->ob[2];
            a->cn[0] = (s8) ((b->cn[0] - a->cn[0]) * t) + a->cn[0];
            a->cn[1] = (s8) ((b->cn[1] - a->cn[1]) * t) + a->cn[1];
            a->cn[2] = (s8) ((b->cn[2] - a->cn[2]) * t) + a->cn[2];
        }
    }
    return a;
}
