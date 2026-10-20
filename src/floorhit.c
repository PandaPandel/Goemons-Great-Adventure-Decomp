#include "object.h"
#include "floorhit.h"
#include "heap.h"
#include "vram.h"

/* Spheres against the stage's collision triangles (overlay 9): the moving floors' query
 * (pl3d.c, plmove.c) sweeps a few spheres along a step through the stage's collision grid and
 * pushes the step off each triangle it meets. The strings are the debug prints of the
 * triangle test ("pararell", "far plain", "VTX in", "TRI in", "line out", "AT LAST, no hit",
 * "Error: zero vector input in _dist_point_..."), compiled out to their strings. */

/* The collision data (as vram.c's). */
typedef struct Coll {
    /* 0x00 */ f32 origin[3];
    /* 0x0C */ s16 dims[3]; /* cells */
    /* 0x12 */ u8 pad12[2];
    /* 0x14 */ f32 cell[3]; /* a cell's size */
    /* 0x20 */ s32 file;
    /* 0x24 */ s32 count; /* triangles */
    /* 0x28 */ u16 *grid; /* a list's start for each cell (0xFFFF: none) */
    /* 0x2C */ u16 *lists; /* triangle numbers, each list ending in 0 */
    /* 0x30 */ CollTri *tris;
} Coll;

/* The VRAM task's work (as vram.c's CourseWork). */
typedef struct CourseWork {
    /* 0x00 */ Coll *coll;
    /* 0x04 */ u8 *tested; /* one byte a triangle, cleared for each sweep */
} CourseWork;

#define COURSE(obj) ((CourseWork *) ((u32) (obj) + sizeof(Obj)))

extern Obj *gVramTask; /* the VRAM task */

/* Sweeps the query's spheres along its step; the last triangle met (and the step, pushed off
 * it, in dir). */
CollTri *fhSweepSpheres(PlFloor *q) {
    Vec3f pos;
    PlSphere *s;
    CollTri *hit;
    CollTri *tri;

    s = q->buf;
    hit = NULL;
    if (s == NULL) {
        return NULL;
    }
    for (; s->r != 0.0f; s++) {
        pos.x = q->from.x + s->off.x;
        pos.y = q->from.y + s->off.y;
        pos.z = q->from.z + s->off.z;
        tri = fhSweepSphere(&pos, s->r, &q->dir, &q->push);
        if (tri != NULL) {
            hit = tri;
            q->dir = q->push;
        }
    }
    return hit;
}

/* Sweeps a sphere of radius r at pos by dir through the grid cells around where it ends; each
 * triangle met (once) pushes the end off it. The last triangle met, and the step (out). The
 * declarations are in the game's frame order (each read off its slot). */
CollTri *fhSweepSphere(Vec3f *pos, f32 r, Vec3f *dir, Vec3f *out) {
    CourseWork *w;
    s16 x0;
    s16 y0;
    s16 z0;
    s16 x1;
    s16 y1;
    s16 z1;
    s16 x;
    s16 y;
    s16 z;
    s16 nx;
    s16 ny;
    s16 nz;
    CollTri *hit;
    Vec3f lo; /* the box around the sphere */
    Vec3f hi;
    u16 *l;
    Vec3f to;
    Vec3f push;

    w = COURSE(gVramTask);
    hit = NULL;
    if (gVramTask == NULL || w->coll == NULL) {
        return NULL;
    }
    nx = w->coll->dims[0];
    ny = w->coll->dims[1];
    nz = w->coll->dims[2];
    heapMemClear(w->tested, w->coll->count);
    to.x = pos->x + dir->x;
    to.y = pos->y + dir->y;
    to.z = pos->z + dir->z;
    lo.x = to.x - r;
    lo.y = to.y - r;
    lo.z = to.z - r;
    x0 = (lo.x - w->coll->origin[0]) / w->coll->cell[0];
    y0 = (lo.y - w->coll->origin[1]) / w->coll->cell[1];
    z0 = (lo.z - w->coll->origin[2]) / w->coll->cell[2];
    hi.x = to.x + r;
    hi.y = to.y + r;
    hi.z = to.z + r;
    x1 = (hi.x - w->coll->origin[0]) / w->coll->cell[0];
    y1 = (hi.y - w->coll->origin[1]) / w->coll->cell[1];
    z1 = (hi.z - w->coll->origin[2]) / w->coll->cell[2];
    for (x = x0; x <= x1; x++) {
        for (y = y0; y <= y1; y++) {
            for (z = z0; z <= z1; z++) {
                if (x >= 0 && x < nx && y >= 0 && y < ny && z >= 0 && z < nz) {
                    u16 cell = w->coll->grid[x * ny * nz + y * nz + z];

                    if (cell == 0xFFFF) {
                        continue;
                    }
                    push.z = push.y = push.x = 0.0f;
                    for (l = &w->coll->lists[cell]; *l != 0; l++) {
                        if (w->tested[*l] == 0 &&
                            fhSphereVsTri(&w->coll->tris[*l], pos, dir, r, &to, &push) == 0) {
                            to.x += push.x;
                            to.y += push.y;
                            to.z += push.z;
                            hit = &w->coll->tris[*l];
                        }
                        w->tested[*l] = 1;
                    }
                }
            }
        }
    }
    out->x = to.x - pos->x;
    out->y = to.y - pos->y;
    out->z = to.z - pos->z;
    return hit;
}

/* A sphere of radius r ending at to (moving from pos by dir) against triangle t: 0 if it
 * touches (push: along the normal, out by r less its distance to the plane), 1 if the
 * triangle is degenerate or, farther than r from the plane, the move does not cross it
 * (vramRayTriangle), 3 if it misses. The normal is made on first use (n[0] over 10: not yet).
 * It touches when a corner is within r, when its foot on the plane is inside the triangle, or
 * when an edge is within r at a place along the edge.
 * MATCHED 2026-10-03 (see the end of this note). The history:
 * NON_MATCHING (991 lines; 1243 for the first draft), 2026-10-02. Read off the game: the
 * triangle's s16 fields are used as they are, so the determinants are integer multiplies
 * (multu, subu, cvt.s.w), "-t->e1[2]" is an integer negation, and the third edge call takes
 * integer sums and differences; the converted values are compiler temps (0x54-0x8C), the
 * same slots here. The corners a (0x124) and b (0x118) are Vec3f in memory; the third corner
 * is never stored (inline here; as a Vec3f or three scalars before dA: 1085). With these
 * locals in this order the frame is the game's 0x148. Left: the game keeps the third corner's
 * sums in a's registers and reloads a for dA; this C keeps a in registers and reloads it for
 * the third corner. Corner arrays (f32 a[3]) bring in f20/f22 (1326); operand order in the
 * corner sums is IDO's own (no change). The two late tests are if/else in the game (the
 * then-part jumps over the else to the push): an empty then for "inside", and "nx = t->n[0]"
 * against "return 3"; the branches are now the game's, one for one (990). The splits
 * sweep's best (a FAKE boundary before tC, 931) leaves this C 21 instructions short (18 now):
 * not kept.
 * 2026-10-03, read off the slots: from at (0xD4 in the game, 0xD8 here) down, every local is 4
 * lower in the game, so it declares one word more above at. The game stores the foot's z
 * (n[2] * k + pz, before "- v[2]") to 0xEC and never reloads it, and stores u (not v) to 0xFC
 * in the first two projections (u then also in f2), never reloading it either. Measured: the
 * foot first and "q -= v" after (1134); "v = u = -1.0f" (the game's else sets u, then v from
 * it; 990, no change). The permuter's old best (score 19353 from 21495: "qx = px - (a.x +
 * e2[0])" before dA, used as one factor of dC): 1116 in the real compile. Sweep B's find on
 * the faithful base (score 21495 → 19009), a dead "b.y = a.y + t->e1[1];" before the push: 995.
 * The pads sweep: fC declared before fB, 988. The greedy mode (2026-10-03, 988 to 916, mix
 * distance 18 to 16): declarations moved and FAKE empty tests of dA and dC.
 * Slots read off the game (2026-10-03): with these declarations dB is 0x104 (the game
 * spills dB there), u 0xFC and qx 0xEC, so the game's 0xEC store (the foot's z) is a q home.
 * The foot first and "q -= v" after, declared qz, qy, qx (qz at 0xEC): 1008 lines but mix
 * distance 14 (16 as written), a hill; not kept.
 * 2026-10-03 (second pass; 916 lines, mix 16 at start): the game converts and sums e2
 * (the third corner, cx/cy/cz in f2/f12/f14, the registers a's values sat in) BEFORE dA, and
 * keeps e2's ints in a2/a3/t0 to the determinants. Three scalars cx/cy/cz after b, in Vec3f
 * c's old words: 952 lines, mix 12 (a then lost its stores: IDO propagated a.x into dA).
 * With that, a and b as f32[3] arrays: 1323, mix 59, frame 0x138 (arrays sit elsewhere).
 * b and c as "t->v[0] + t->e1[0]" (a read only in dA): 1380, mix 88 (integer adds).
 * A FAKE "do {} while (0);" between cz and dA: 1086, mix 18; a and b are then read from
 * their homes for dA and dB as in the game, but c is spilled to its words (0x10C-0x114) and
 * the v conversions stay in registers. Not kept.
 * The file's debug strings (pararell, far plain, inter, VTX in, near, TRI in, line out, t1t2t3,
 * line IN, AT LAST: in .rodata order) put back as dead PRINTFs at their places, the vertex test
 * as "if (dA <= r || ...) { VTX in } else": 952, mix 12, no change (kept: the game's prints).
 * unfakes on this base: without "if (dC) {}" 982, without "if (dA) {}" 1162 (both kept).
 * b and c from "(f32) t->v[i] + t->e1[i]" (fewer reads of a): 1039, mix 17 (the v and e1
 * conversions interleave, a's stores sink).
 * FAKE "if (cx) {} if (cy) {} if (cz) {}" after cz (more uses for c): 1143, mix 15 (a real
 * compare is emitted). The moves sweep on this base: best 950 (the fC differences reordered),
 * noise, not kept.
 * Not kept (the lead's keep rule: the lines must not rise): cx/cy/cz before dA, 952 at mix 12,
 * a hill for later; the C below is the inline third corner in dC (916, mix 16), with the prints.
 * Read off the game (2026-10-03): before its call the game spills t->n[0] to 0x90, the top
 * of its compiler-temp area, and after it reloads "lwc1 0x14(a0); swc1 0x90; lwc1 0x90": the
 * original has no nx variable, only t->n[0] (PRE'd). Written so (every nx read as t->n[0], its
 * assignments gone): that opening comes out as the game's, instruction for instruction. With a
 * pad word in nx's place, t->n[0] at 0x90 and the locals at the game's slots, the converted
 * temps 4 low (0x50.. for 0x54..): 928, mix 16. Without the pad, the temps at the game's
 * 0x54-0x8C but t->n[0] at 0x94 and the locals 4 high: 934. Not kept (916); the hill to build
 * on, with the game's two more temp words (0x4C, 0x50) still to explain.
 * Five parallel attempts, 2026-10-03, each on its own copy (every try kept for reference).
 * The kept one: the third edge call's sums read as named fields, "TV(t)->x + TE1(t)->x"
 * (CollVec): the game's "addu t8,t7,v1" (v first), 916 to 910. The whole edges section as
 * fields gives the game's edge code (z, y, x loads; "wz * e"; "(x + y) + z") at 1053, mix 17:
 * the float rotation is set upstream by the u/v section. The e2/v register swap is
 * function-wide (e2's whole live range), not the edges' own. HILL (a0): the two extra temps
 * explained: 0x50/0x4C/0x48 hold dC's x, y, z differences, spilled because the game keeps the
 * third corner c in f2/f12/f14 through dA..dC. A "running corner" with no nx gives that corner
 * code as the game's (each conversion stored and reloaded into f2/f12/f14, a stored and
 * reloaded for dA, c added in place, c's homes untouched): "c.x = t->v[0]; ...; a.x = c.x;
 * ...; b.x = c.x + t->e1[0]; ...; c.x += t->e2[0]; ..." then dA, dB, dC: 1037, mix 9. With a
 * FAKE pad after qx (declared qz, qy, qx) and the foot first then "q -= v", the temp area is the
 * game's word for word (t->n[0] 0x90, conversions 0x54-0x8C, 0x50/0x4C/0x48, 0x44): 1066, mix
 * 12. Left there: the float rotation at the top (f10 for f6) and the foot block's registers
 * (the game: px f0, n1 f2, n2 f12, k f14, qx/qy f16/f18, qz's first value to 0xEC). The
 * running-corner form is the base for the next pass or greedy run.
 * What is left, read off the game: the game colours c (f2/f12/f14) across dA..dC and px in f4
 * across all three distances, and SPLITS a: a.x sits in f2 for b.x and cx, is stored to its
 * home and reloaded for dA; b is read from its home for dB (no spill temp at 0x30). This C
 * colours a whole (no stores to 0x124-0x12C) and spills c to 0x10C-0x114. So in the game a's
 * reads are not copy-propagated (IDO_NOTES 265: a local with two definitions is not) and
 * there is a uopt block boundary between cz and dA (a and b then come from their homes,
 * measured above) that as1 still schedules across. The game's e2 ints sit in a2/a3/t0 and the
 * tA section's v ints in t1/t2/t3; this C has them the other way round.
 * This pass, on this copy: the C below is round 1's running-corner HILL (no nx, the
 * running corner, qz/qy/qx + pad, the foot first then "q -= v") with the KEPT TV/TE1 sums in
 * dC's call: 1060, mix 12 (1066 without the fields). Tries on it:
 *   b from "c.x + TE1(t)->x" (fields): 1060, mix 12, the same code (not kept).
 *   b from "a.x + t->e1[0]": 1060, mix 12, the same code (a.x forwarded; not kept).
 *   FAKE "do {} while (0);" between c.z and dA: 1070, mix 22 (c leaves f2/f12/f14; not kept).
 * Read off the side-by-side (norm.awk, side.txt here): the game stores b and then reads b from
 * its home for dB; this C keeps b.x/b.y in f6/f10 and spills them to 0x30/0x34. The game adds
 * "c.x + cvt" (c first, f2 + f10); this C "cvt + c" (f8 + f2), in every form above.
 *   KEPT: the corner's copies interleaved, "c.x = t->v[0]; a.x = c.x; c.y = t->v[1]; a.y =
 *   c.y; ...": 1047, mix 11 (each a store now follows its conversion, as in the game).
 *   FAKE "do {} while (0);" between b.z and "c.x +=": 1122, mix 18 (b is then reloaded from
 *   its home as in the game, but the conversions go straight into f0/f16/f18; not kept).
 * cc -S (ugen) on this copy: uopt forwards b's stored sums into dB (ugen keeps f6/f10/f8 and
 * spills b.x/b.y to 0x30/0x34); the game reloads b from 0x118-0x120. Those kept b values are
 * also why px is reloaded for each of dA, dB, dC here, where the game keeps it in f4.
 *   b reused as tB's w vector (a second definition of b): 1047, mix 11, the same code (the
 *   forwarding is local to the block; not kept).
 * One compile of many forms (vx/: body.inc with #if V == n, vars.c includes it per V; vx/run.sh
 * scores each against the game, normalized; V0 = this C gave the check's own count, 883):
 *   b as "t->e1[0] + c.x" (V1), "a.x = c.x = t->v[0]" chains (V2), "Vec3f q" for qz/qy/qx (V13):
 *   the same code. b through "Vec3f *pb = &b" (V3), "a = c;" (V6), "b = a; b.x += .." (V7): far
 *   worse (frames 0x140/0x138). FAKE boundary between "q = n * k + p" and "q -= v" (V14 with
 *   Vec3f q, V16 scalars): worse (919, 922). Without "if (dC) {}" only: 927; without "if (dA)
 *   {}" only: 937; WITHOUT BOTH (V10): 871.
 *   KEPT: both FAKE empties ("if (dC) {}", "if (dA) {}") out: 1031, mix 11 (px now f6 at the top
 *   as in the game; 10.0f f8 for f4, py f4 for f10, pz f10 for f8).
 * vx/score.sh folds ugen's float temps (f4 f6 f8 f10) to one name: a "structural" count that
 * the FIFO temp queue (IDO_NOTES 274) does not move (this C 251).
 *   b with two definitions, "b.x = a.x; b.y = a.y; b.z = a.z; b.x += t->e1[0]; ..." (or "b.x =
 *   c.x; ... b.x += .."): b is no longer forwarded into dB, it is read from its home, and px
 *   stays in one temp across dA, dB, dC: the corner and distance code is the game's
 *   instruction for instruction, temps aside (structural 195 from 251; normalized 893, worse,
 *   all FIFO). Per coordinate "b.x = c.x; b.x += .." breaks it (1116).
 *   On that: FAKE boundaries at each foot statement, "v = u = -1.0f", the natural "a = v; b =
 *   a; b += e1; c = a; c += e2" (270): all worse structurally. The foot as "h = n.p - d; k = -h
 *   / nn" with h (or nn) in the pad's word, or k reused for the numerator: normalized 805-812
 *   but structural 242-262 (k reused gives px f0 and k f14 as the game, numerator in f14).
 *   The len's normal read as named fields, "TN(t)->x * TN(t)->x + ..." (Vec3f view of n):
 *   n[1] f2 and n[2] f12 as in the game (top and foot), structural 171, normalized 557.
 *   (dist, k, q or the push through fields or operands swapped: nothing.)
 *   KEPT: two-definition b + TN len: 675 lines, mix 7.
 * On that (vx): the foot's k written into another local. det: structural 110; len: 112;
 * u: 88; V: 66 (dA..dC, tA..tC, wx..wz, inv: no change, 171). With v as the step ("v = -(n.p
 * - d) / (n.n); qx = t->n[0] * v + px; ..."; k left declared, unused) the foot and the
 * projections are the game's: px in f0, n1 f2, n2 f12, the step f14, qx f16, qy f18, qz's first
 * value stored to 0xEC, its second in f2, u stored to 0xFC and moved to f2, v f14.
 *   HILL: that form, checked: 1059 lines but MIX DISTANCE 1 and the frame and slots the game's
 *   (no stack line). Not kept (lines). What is left there is ugen's float temp queue: every
 *   f4/f6/f8/f10 is permuted against the game's from the first instruction (the game's queue at
 *   entry, read off by FIFO simulation, is f6 f10 f8 f4; this form's f4 f8 f10 f6), plus a few
 *   multu operand orders (e2/v ints) and schedule differences that follow from the registers.
 *   FAKE "if (push->x) {}" / "if (push->z) {}" at the end: no change at the top.
 * THE QUEUE, measured (vx/fifo.awk simulates ugen's FIFO over a "cc -S" listing; it reproduces
 * every float temp of this function): ugen generates the function TWICE, and the real pass
 * starts with the queue the first pass left at the function's end; the first pass starts from
 * f4 f6 f8 f10. (The hill's code run from f4 f6 f8 f10 ends at f4 f8 f10 f6, exactly the
 * queue its prologue shows.) So the entry registers are set by every temp release in the
 * function, and a release follows the asm operand order: every commutative operand order
 * IDO sorted (array elements) moves the queue. Each fix came from where the game-to-this-C
 * temp mapping first changed:
 *   dist's sum "(n0 px + n1 py) + n2 pz" was emitted n2 pz first: TN fields in dist (755).
 *   the foot's denominator likewise: TN fields in the step's numerator and denominator.
 *   the projections' "-e1z qx + e1x qz" was summed e1x qz first, and the dets' multu operands
 *   swapped: TE1/TE2 fields in the dets and in u, v (637, structural 23).
 *   tA's "wz * e1z" was "e1z * wz" (and the edge loads x, y, z for the game's z, y, x): TV/TE1/
 *   TE2 fields in tA, tB, tC (the calls' arguments stay arrays; as fields: no help).
 * MATCH (check: "MATCH except own section addresses") with all of that on the hill: the C
 * below. k is left declared and unused (its word is in the game's frame).
 * Fakes sweep, 2026-10-03: the pad's word is ratio. The file's "  ratio[%f]\n" (0xC4, after
 * AT LAST and before the error of _dist_point_to_line in .rodata, so at the end of this function)
 * is a dead print of r - dist before the push; ratio as the push's factor is a live variable
 * and takes the pad's word (MATCH as before, no FAKE left in this function). */
s32 fhSphereVsTri(CollTri *t, Vec3f *pos, Vec3f *dir, f32 r, Vec3f *to, Vec3f *push) {
    f32 px;
    f32 py;
    f32 pz;
    f32 len;
    f32 k; /* unused: a word of the frame in the game (the step is v) */
    f32 dist;
    Vec3f a;
    Vec3f b;
    Vec3f c;
    f32 dA;
    f32 dB;
    f32 dC;
    f32 u;
    f32 v;
    f32 det;
    f32 inv;
    f32 qz;
    f32 qy;
    f32 qx;
    f32 ratio;
    Vec3f at;
    f32 tt;
    Vec3f fA;
    Vec3f fB;
    Vec3f fC;
    f32 wx;
    f32 wy;
    f32 wz;
    f32 tA;
    f32 tB;
    f32 tC;

    px = to->x;
    py = to->y;
    pz = to->z;
    if (t->n.x > 10.0f) {
        vramMakeTriNormal(t);
        PRINTF(">>\tadj[%f][%f][%f]\n", t->n.x, t->n.y, t->n.z);
    }
    len = sqrtf(t->n.x * t->n.x + t->n.y * t->n.y + t->n.z * t->n.z);
    if (fabsf(len) < 0.0001f) {
        PRINTF("pararell\n");
        return 1;
    }
    dist = fabsf(t->n.x * px + t->n.y * py + t->n.z * pz - t->d) / len;
    if (r < dist) {
        PRINTF("far plain \n");
        if (vramRayTriangle(t, pos, dir, &at, &tt)) {
            return 1;
        }
        PRINTF("inter ***\n");
    }
    c.x = t->v.x;
    a.x = c.x;
    c.y = t->v.y;
    a.y = c.y;
    c.z = t->v.z;
    a.z = c.z;
    b.x = a.x;
    b.y = a.y;
    b.z = a.z;
    b.x += t->e1.x;
    b.y += t->e1.y;
    b.z += t->e1.z;
    c.x += t->e2.x;
    c.y += t->e2.y;
    c.z += t->e2.z;
    dA = sqrtf((px - a.x) * (px - a.x) + (py - a.y) * (py - a.y) + (pz - a.z) * (pz - a.z));
    dB = sqrtf((px - b.x) * (px - b.x) + (py - b.y) * (py - b.y) + (pz - b.z) * (pz - b.z));
    dC = sqrtf((px - c.x) * (px - c.x) + (py - c.y) * (py - c.y) + (pz - c.z) * (pz - c.z));
    if (dA <= r || dB <= r || dC <= r) {
        PRINTF("VTX in ********");
    } else {
        /* The foot on the plane, from the corner (v holds the step along the normal). */
        v = -(t->n.x * px + t->n.y * py + t->n.z * pz - t->d) /
            (t->n.x * t->n.x + t->n.y * t->n.y + t->n.z * t->n.z);
        qx = t->n.x * v + px;
        qy = t->n.y * v + py;
        qz = t->n.z * v + pz;
        qx -= t->v.x;
        qy -= t->v.y;
        qz -= t->v.z;
        if (qx * qx + qy * qy + qz * qz < 9.999999e-9f) {
            PRINTF("near ****\n");
            return 0;
        }
        /* Where it is in the triangle's edges, from the first flat projection that works. */
        if ((det = t->e1.x * t->e2.z - t->e2.x * t->e1.z) != 0.0f) {
            inv = 1.0f / det;
            u = (t->e2.z * qx - t->e2.x * qz) * inv;
            v = (-t->e1.z * qx + t->e1.x * qz) * inv;
        } else if ((det = t->e1.z * t->e2.y - t->e2.z * t->e1.y) != 0.0f) {
            inv = 1.0f / det;
            u = (t->e2.y * qz - t->e2.z * qy) * inv;
            v = (-t->e1.y * qz + t->e1.z * qy) * inv;
        } else if ((det = t->e1.x * t->e2.y - t->e2.x * t->e1.y) != 0.0f) {
            inv = 1.0f / det;
            u = (t->e2.y * qx - t->e2.x * qy) * inv;
            v = (-t->e1.y * qx + t->e1.x * qy) * inv;
        } else {
            u = v = -1.0f;
        }
        if (u >= 0.0f && v >= 0.0f && u + v <= 1.0f) {
            PRINTF("TRI in ********");
        } else {
            /* The edges. */
            dA = _dist_point_to_line(px, py, pz, t->e1.x, t->e1.y, t->e1.z, t->v.x, t->v.y,
                                        t->v.z, &fA);
            dB = _dist_point_to_line(px, py, pz, t->e2.x, t->e2.y, t->e2.z, t->v.x, t->v.y,
                                        t->v.z, &fB);
            dC = _dist_point_to_line(px, py, pz, t->e1.x - t->e2.x, t->e1.y - t->e2.y,
                                        t->e1.z - t->e2.z, t->v.x + t->e1.x, t->v.y + t->e1.y,
                                        t->v.z + t->e1.z, &fC);
            if (r < dA && r < dB && r < dC) {
                PRINTF("line out ********\n");
                return 3;
            }
            wx = fA.x - px;
            wy = fA.y - py;
            wz = fA.z - pz;
            tA = (wx * wx + wy * wy + wz * wz - wx * t->v.x - wy * t->v.y - wz * t->v.z) /
                 (wx * t->e1.x + wy * t->e1.y + wz * t->e1.z);
            wx = fB.x - px;
            wy = fB.y - py;
            wz = fB.z - pz;
            tB = (wx * wx + wy * wy + wz * wz - wx * t->v.x - wy * t->v.y - wz * t->v.z) /
                 (wx * t->e2.x + wy * t->e2.y + wz * t->e2.z);
            wx = fC.x - px;
            wy = fC.y - py;
            wz = fC.z - pz;
            tC = (wx * wx + wy * wy + wz * wz - wx * (t->v.x + t->e1.x) - wy * (t->v.y + t->e1.y) -
                  wz * (t->v.z + t->e1.z)) /
                 (wx * (t->e2.x - t->e1.x) + wy * (t->e2.y - t->e1.y) + wz * (t->e2.z - t->e1.z));
            PRINTF(" t1[%f]t2[%f]t3[%f]\n", tA, tB, tC);
            if ((dA <= r && tA >= 0.0f && tA <= 1.0f) || (dB <= r && tB >= 0.0f && tB <= 1.0f) ||
                (dC <= r && tC >= 0.0f && tC <= 1.0f)) {
                PRINTF(" line IN !!! ******\n");
            } else {
                PRINTF("AT LAST, no hit ********\n");
                return 3;
            }
        }
    }
    ratio = r - dist;
    PRINTF("  ratio[%f]\n", ratio);
    push->x = t->n.x * ratio;
    push->y = t->n.y * ratio;
    push->z = t->n.z * ratio;
    return 0;
}

/* How far p is from the line through o along d (out: d times p - o, by axis); the game's
 * _dist_point_to_line (its zero-vector error print is the file's last string). The sum t is
 * written BEFORE the stores to out, the products inline in it and again in the stores (IDO
 * CSEs them): every operand load then comes before the first store through out, which IDO's
 * scheduler never lifts a param load above. z holds only the z product; IDO keeps it in a
 * compiler temp at 0x14, under z and t (the game's 0x20 frame). */
f32 _dist_point_to_line(f32 px, f32 py, f32 pz, f32 dx, f32 dy, f32 dz, f32 ox, f32 oy, f32 oz, Vec3f *out) {
    f32 z;
    f32 t;

    if (dx == 0 && dy == 0 && dz == 0) {
        PRINTF("Error: zero vector input in _dist_point_to_line\n");
        return 0.0f;
    }
    t = ((px - ox) * dx + dy * (py - oy) + dz * (pz - oz)) / (dx * dx + dy * dy + dz * dz);
    out->x = (px - ox) * dx;
    out->y = dy * (py - oy);
    z = dz * (pz - oz);
    out->z = z;
    return sqrtf((dx * t + ox - px) * (dx * t + ox - px) + (dy * t + oy - py) * (dy * t + oy - py) +
                 (dz * t + oz - pz) * (dz * t + oz - pz));
}
