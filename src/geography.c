#include "object.h"
#include "game.h"
#include "cache.h"
#include "file.h"
#include "fileres.h"
#include "geography.h"
#include "gfxbuf.h"
#include "gfxmgr.h"
#include "heap.h"
#include "mathutil.h"
#include "polygon.h"
#include "stage.h"
#include "tmanager.h"

/* The "Geography" task: the stage's scenery ("地形マネージャー", the terrain manager).
 * - A geography is a grid of cells over the stage. Each run of cells lists placements: an
 *   object (a display list and up to eleven files for its segments) with a position, a
 *   rotation (1024 a turn) and a scale (256 = 1).
 * - Every 8 frames it finds the cells inside the camera's view, clipped to a box ahead of the
 *   eye (appear_in_cam, "appear_in_cam"). A frame later it makes a frame for each visible
 *   placement (geoMakeFrames), loading the files through a cache of recently used ones.
 * - Setting up a stage (geoLoadStage) loads its geography, its object file (keeping only the
 *   objects its placements use) and its textures. */

extern Frame *gFrmLayers[]; /* the frame pools */
extern s16 gFrmKindMpol;
extern s16 gFrmKindDlist; /* the "dlist " kind */
/* geography.c's .data (0x801090F0-0x801094B4), in ROM order. */
u16 gGeoTextureFiles[448] = { /* 448 entries: the last few are zero */
    0x0000, 0x081B, 0x033D, 0x033E, 0x0339, 0x033A, 0x0343, 0x04D3, 0x032A, 0x0329, 0x0335, 0x0336,
    0x0337, 0x0344, 0x0346, 0x0348, 0x032C, 0x0325, 0x0338, 0x0324, 0x032B, 0x034A, 0x0341, 0x0326,
    0x0327, 0x0328, 0x0347, 0x0332, 0x0350, 0x034B, 0x0333, 0x034C, 0x0345, 0x0342, 0x0331, 0x0349,
    0x032F, 0x034F, 0x0334, 0x0330, 0x032E, 0x032D, 0x033F, 0x033B, 0x0340, 0x033C, 0x034D, 0x034E,
    0x0561, 0x058C, 0x0560, 0x058D, 0x0592, 0x056C, 0x0819, 0x057C, 0x058A, 0x0991, 0x080E, 0x099C,
    0x056E, 0x056B, 0x0572, 0x0575, 0x0581, 0x0589, 0x058E, 0x057D, 0x080C, 0x058F, 0x057E, 0x0570,
    0x058B, 0x0580, 0x057F, 0x0577, 0x0571, 0x0568, 0x0587, 0x0578, 0x056F, 0x057B, 0x0563, 0x0564,
    0x0565, 0x0566, 0x0573, 0x0574, 0x0584, 0x0576, 0x0567, 0x0579, 0x0585, 0x0569, 0x056A, 0x0588,
    0x057A, 0x0590, 0x0591, 0x0582, 0x056D, 0x03D6, 0x03CF, 0x03CA, 0x03B9, 0x03B2, 0x03CB, 0x03B4,
    0x03C8, 0x03B5, 0x03BA, 0x03B7, 0x03BF, 0x03B8, 0x03C1, 0x03C0, 0x03B3, 0x03B6, 0x03CC, 0x03BC,
    0x03BB, 0x03D4, 0x03C2, 0x03D1, 0x03C3, 0x03C7, 0x03D5, 0x03C6, 0x03C5, 0x03D7, 0x03C4, 0x0995,
    0x0992, 0x03CD, 0x0996, 0x0993, 0x0994, 0x0997, 0x069C, 0x069B, 0x069F, 0x06A0, 0x06A5, 0x0719,
    0x0692, 0x0690, 0x0693, 0x0694, 0x069A, 0x069E, 0x069D, 0x06A2, 0x0696, 0x06A6, 0x0699, 0x06A3,
    0x06A4, 0x0691, 0x0695, 0x06A1, 0x0698, 0x0697, 0x069E, 0x069D, 0x06A3, 0x0695, 0x0698, 0x08C1,
    0x03D3, 0x08AD, 0x08BD, 0x08A6, 0x08A2, 0x08A4, 0x08AA, 0x08C5, 0x08B6, 0x08B7, 0x08B8, 0x08B9,
    0x08B5, 0x08A8, 0x09A3, 0x0895, 0x08AC, 0x089A, 0x08A5, 0x08BE, 0x0899, 0x08A3, 0x08AB, 0x08B2,
    0x08B4, 0x089F, 0x08A0, 0x08AE, 0x0897, 0x08A7, 0x08C2, 0x08A1, 0x08BF, 0x089B, 0x0721, 0x0717,
    0x070E, 0x071B, 0x070A, 0x0720, 0x0708, 0x0709, 0x071D, 0x071E, 0x071C, 0x0722, 0x070F, 0x0710,
    0x03B1, 0x03D0, 0x03BE, 0x03C9, 0x03CE, 0x03BD, 0x0586, 0x0583, 0x0714, 0x0715, 0x071A, 0x061D,
    0x0712, 0x071F, 0x070D, 0x070B, 0x0523, 0x0521, 0x0718, 0x0894, 0x08B0, 0x0896, 0x099A, 0x0999,
    0x099B, 0x0998, 0x0990, 0x098F, 0x0459, 0x0458, 0x0463, 0x0465, 0x045B, 0x046D, 0x046E, 0x045A,
    0x0471, 0x046F, 0x03D2, 0x04D4, 0x0711, 0x0716, 0x070C, 0x0454, 0x0455, 0x045C, 0x045D, 0x045E,
    0x046C, 0x0461, 0x0462, 0x08AF, 0x08C3, 0x08C0, 0x08BA, 0x08BB, 0x0464, 0x0456, 0x0457, 0x0470,
    0x045F, 0x0468, 0x0460, 0x0466, 0x0467, 0x04D7, 0x04DF, 0x04D6, 0x04D0, 0x04DE, 0x04E0, 0x04E1,
    0x0562, 0x0593, 0x0594, 0x04DA, 0x04D1, 0x04DB, 0x04D2, 0x0469, 0x046A, 0x046B, 0x04DC, 0x04DD,
    0x04D8, 0x0804, 0x0818, 0x0817, 0x0805, 0x0807, 0x0806, 0x0816, 0x0814, 0x0808, 0x0815, 0x0803,
    0x051F, 0x0801, 0x080B, 0x0802, 0x080A, 0x0809, 0x089C, 0x08B1, 0x089E, 0x0898, 0x09A1, 0x099F,
    0x09A0, 0x099D, 0x099E, 0x080D, 0x07FF, 0x0518, 0x0524, 0x0526, 0x0522, 0x051A, 0x0519, 0x051E,
    0x051B, 0x051D, 0x08C4, 0x078E, 0x078C, 0x0794, 0x0798, 0x0783, 0x077F, 0x0780, 0x0791, 0x0520,
    0x0800, 0x0781, 0x078B, 0x077E, 0x078B, 0x081A, 0x081C, 0x0810, 0x0812, 0x0811, 0x0813, 0x081D,
    0x080F, 0x0820, 0x081F, 0x0516, 0x0517, 0x081E, 0x0797, 0x0786, 0x0793, 0x0790, 0x0782, 0x0785,
    0x0792, 0x0784, 0x0795, 0x0796, 0x0789, 0x0787, 0x0788, 0x078A, 0x078D, 0x09A7, 0x09A9, 0x09A6,
    0x09A2, 0x0612, 0x0630, 0x0631, 0x0616, 0x0636, 0x0634, 0x0635, 0x0637, 0x0632, 0x0633, 0x0626,
    0x0619, 0x062C, 0x0638, 0x0613, 0x08B3, 0x078F, 0x08A9, 0x0525, 0x051C, 0x09A5, 0x09A4, 0x09A8,
    0x09AA, 0x0713, 0x09AB, 0x089D, 0x088A, 0x088C, 0x088D, 0x088B, 0x0615, 0x0617, 0x0614, 0x0627,
    0x061B, 0x0620, 0x0623, 0x0624, 0x061A, 0x0618, 0x0621, 0x061F, 0x061C, 0x08BC, 0x0000, 0x0629,
    0x062D, 0x0625, 0x0628, 0x0622, 0x062A, 0x062B, 0x061E, 0x062E, 0x062F, 0x04D9, 0x04D5, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000,
};
Obj *gGeographyObj = NULL; /* the Geography task */
Gfx gGeoSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetCombineMode(G_CC_MODULATEIA, G_CC_MODULATEIA),
    gsSPEndDisplayList(),
};
u32 gGeoFileRam = 0x00000000; /* the geography file's RAM */
extern s32 gGameFrameCount; /* the frame count (GameState's, by its own name) */


extern s16 gFrmKindGpol; /* the "gpol" kind */
extern s16 gFrmKindGani; /* the "gani" kind */


#define GEO(obj) ((GeoWork *) ((u32) (obj) + sizeof(Obj)))

/* The handler. It runs the view test on frames 1 (of 8) and the frame making on 2 and 3. */
s32 Geography(Obj *obj, s16 msg, void *arg) {
    GeoWork *self = GEO(obj);
    GeoWork *w = self;
    s32 file;
    s32 i;
    u8 *data;

    switch (msg) {
        case 2:
            if (gGeographyObj != NULL) {
                return -1;
            }
            gGeographyObj = obj;
            obj->name = "Geography";
            w->root = frmCreate(gFrmLayers[3], gFrmKindMpol);
            w->dlf = (DLFrame *) frmCreateNext(gFrmLayers[9], gFrmKindDlist);
            if (w->root == NULL || w->dlf == NULL) {
                return -1;
            }
            w->dlf->dl = gGeoSetupDl;
            w->scale = 0.1f;
            if (gGameWork.geoCacheSize != 0) {
                w->cacheSize = gGameWork.geoCacheSize;
                gGameWork.geoCacheSize = 0;
            } else {
                w->cacheSize = 30;
            }
            w->box[0] = 600;
            w->box[1] = 300;
            w->box[2] = 600;
            w->ahead = 200;
            PRINTF("地形マネージャーをインストールします \n");
            break;
        case 1:
            gGeographyObj = NULL;
            frmDestroy(w->root);
            frmDestroy((Frame *) w->dlf);
            while (gfxGetPendingSwaps() != 0) {
            }
            if (w->cache != NULL) {
                for (i = 0; i < *(s32 *) self->cache; i++) {
                    file = cacheGetRecent(w->cache, &data, i);
                    if (file != 0) {
                        resDrop(file);
                    }
                }
                cacheDelete(w->cache);
            }
            if (w->objs != NULL) {
                heapFree(w->objs);
            }
            if (w->visible != NULL) {
                heapFree(w->visible);
            }
            break;
        case 0:
            if (w->geo != NULL && w->objs != NULL && w->cache != NULL) {
                switch (gGameFrameCount & 7) {
                    case 1:
                        appear_in_cam(w);
                        break;
                    case 2:
                        geoMakeFrames(w);
                        break;
                    case 3:
                        geoMakeFrames(w);
                        break;
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* out = x, y and z times the first three rows of m. */
void geoRotateVec(Vec3f *out, MtxF *m, f32 x, f32 y, f32 z) {
    out->x = m->m[0][0] * x + m->m[1][0] * y + m->m[2][0] * z;
    out->y = m->m[0][1] * x + m->m[1][1] * y + m->m[2][1] * z;
    out->z = m->m[0][2] * x + m->m[1][2] * y + m->m[2][2] * z;
}

/* A point's signed distance from a plane (normal, then its offset). */
f32 geoPlaneDist(f32 *plane, f32 x, f32 y, f32 z) {
    f32 a = plane[0];
    f32 b = plane[1];
    f32 c = plane[2];

    return (a * x + b * y + c * z - plane[3]) / sqrtf(a * a + b * b + c * c);
}

/* "appear_in_cam": which placements are visible. Mode 1 shows all and mode 2 none. Otherwise a
 * box of cells (w->box, by the cell size) around a point w->ahead along the view is tested:
 * a cell corner on the inner side of the view's four side planes (0.8 of the field of view
 * each way, turned by the camera's roll) and of its far plane marks the eight cells around it,
 * and every placement listed for a marked cell becomes visible.
 * MATCHED 2026-10-03: the last difference (dimY/dimZ's spill "sh s3,0x160; sh s4,0x15E" right
 * after the visible-clear call, where the game has it in the scan's preheader after "blez nx;
 * sw zero,x") went when guAlignF read w->view straight instead of through the local view: one
 * live range fewer beside dimY/dimZ at the top, and their register region then reaches over
 * the scan's guard block as in the game (the scratch probes below had shown that block alone
 * decided it). view keeps its word in the frame: it holds the view for the NULL test only
 * (set and tested in the switch, the same code as testing w->view). The fakes sweep
 * (2026-10-03): view, an unused FAKE until then, is that test's copy, and the unused FAKE
 * halfword pad3 is mode, the switch's copy of w->mode (a u8: the same frame word); both give
 * the game's code with no FAKE left. Not that word: an s16 or s32 copy of w->ahead for the
 * call (532 lines: the copy takes v0 and the code is 2 shorter). The prints of the functions
 * below (once a FAKE block at this function's end, to keep their strings after this one's in
 * .rodata) are in their own functions now: geoMakeFrames's two at its end, the texture print
 * inside geoRelocate's loop (before the loop it moves geoRelocate's registers, 27
 * lines); .rodata is the game's bytes either way (string order is source order). The
 * "(z + z0) * 1" FAKE is gone (fakes round 2, 2026-10-04): the scan reads the cell through dx,
 * dy and dz again, "dx = x + x0; dy = y + y0; dz = z + z0;" then "dimY * dx * dimZ + dy * dimZ
 * + dz", the game's code with no FAKE. uopt reassociates an add chain of one type: written in
 * place, "A + (z + z0)" becomes "(A + z) + z0" ("cell = A; cell += z + z0;" 36 lines, "(z +
 * z0) + (A)" 96; cc -O1 keeps the grouping, so it is uopt; IDO_NOTES 33). A sum read through a
 * variable is a leaf when uopt builds the chain, and copy propagation puts it back in place
 * afterwards, so the game's single add stays. In scratch cc -S, "dz = z + z0;" alone and
 * "(u32) (z + z0)" (a type change in the chain) also give the same code. History while it was
 * parked, in order, newest last.
 * Old notes: the flow and every local's place relative to the frame's
 * top are the game's; the game keeps the loop counters x and y in their homes, w in fp, and
 * has 40 bytes more of compiler temporaries (a 0x210 frame, 0x1E8 here). 2026-10-02: the game
 * has 18 multiplies to this C's 14: it stores the box's sizes twice (nx, ny to 0x11C/0x118 and
 * nx, ny, nz to 0xCC/0xC8/0xC4), and the last four marks (z + 1) read ny from the second set,
 * so their (x - x0) * ny * nz products are not shared with the first four. Control flow read
 * off the game: a cell outside a plane jumps past the marks to the z step (no "k == 5" test:
 * a goto), and the run search tests at its top with a jump back ("while (TRUE) { if (cell <
 * run[1]) break; ...}"; a while or a goto loop is rotated). With those the branches are the
 * game's but for the x/y loop feet (bne with the counter's store in the delay slot: the game
 * keeps x and y in their homes) and one beqz (875 lines, register shifts; 872 before).
 * The splits sweep's best (a FAKE boundary after the third plane's offset, 806) leaves this C
 * 44 instructions short (41 now) and the frame 0x1F0: not kept. Nor "continue" for an empty
 * grid cell (889) or an empty run (885).
 * 2026-10-03 (875 to 784, the permuter on a faithful base): the last five marks index with an
 * s16 copy of y (FAKE), so their products are not shared with the first three's. Its other
 * change, guAlignF reading w->view straight: 875 alone, 788 with the copy (not kept).
 * Its own print ("メモリが足りません appear_in_cam", which a FAKE print block at its end also
 * held for the assembly build then) now sits in the out-of-memory branch: no change (784). x and y
 * address-taken (a dead print of &x, &y, a probe for their homes): 1229, so not that.
 * 2026-10-03, sweep B's find on the faithful base (score 24994 → 13528), IDO_NOTES 241: w read
 * through a copy g from the third plane's offset on: 784 → 638 (the same instruction mix:
 * registers and schedule move). Its guAlignF on w->view straight, with g: 642 (not kept).
 * The copies sweep on top: w itself a local copy of the argument, set first: 632. The joins
 * sweep: nx and ny on one line, 630 (this C 659 of the game's 665, mix distance 24). The
 * inlines sweep: the second "view = g->view;" read in place in its three uses, 620. Against
 * the game (mix distance 24): two more multu/mflo and addu, five more nops, three fewer lwc1
 * there. The cell's three coordinates named before the plane loop: 774 (the same mix).
 * 2026-10-03, the greedy mode (620 to 376, mix distance 24 to 20): FAKE empty tests of arg, bz
 * and z and one block boundary after the grid's cell store.
 * screen (2026-10-03, the calls now compared by address: guAlignF is guAlignF, no call
 * difference): the game has 2 multu and 1 lh more, 1 lw and 1 sw fewer, its branch 14 a bnez
 * where this C has bnezl. So the game shares fewer of the eight corner marks' products (each
 * mark is (x - x0 + i) * ny * nz + (y - y0 + j) * nz + (z - z0 + k), in the order i, j, then k
 * fastest-first) and reloads one s16 (yy, the permuter's FAKE copy, is an s16 here).
 * Read off the game: at ny's definition it stores the value TWICE, "sw t6,0xC8; sw
 * t6,0x118" (0x118 ny's home, 0xC8 a second home). The first four marks multiply by the 0x118
 * copy; the z+1 marks reload the 0xC8 one and RECOMPUTE (x - x0) * ny * nz and (x - x0 + 1) *
 * ny * nz (the 2 multu more); the visibility scan's y loop reads 0xC8 for its entry test and its
 * x * ny product and 0x118 for its bottom test. So the game has ny's value in two names from
 * its definition on (a chain "ny2 = ny = y1 - y0 + 1", or a temp uopt keeps beside ny), and
 * the z+1 marks and the scan use the second: the lead, in place of the FAKE yy copy. Its s16
 * locals at 0x15E/0x160 are stored before the scan and reloaded in it (the lh more).
 * The two sets read off the game: nx 0x11C and ny 0x118 sit with the other locals (x0..z1 at
 * 0x140..0x12C, x and y at 0x128/0x124), nz is s7 (its home never stored); 0xCC/0xC8/0xC4 hold
 * x1-x0+1, y1-y0+1, z1-z0+1 again. Measured 2026-10-03, both in place of the FAKE yy:
 * a second named set "nx2 = nx = ...", used by the z+1 marks and the scan: 1070 (mix 40,
 * frame 0x238); the expressions written out there instead ("(y1 - y0 + 1)" in the z+1 marks,
 * "x * (y1 - y0 + 1) * (z1 - z0 + 1)" in the scan): 1041 (mix 33, frame 0x220). Not kept.
 * 2026-10-03 (scratch compiles of whole variants, diff lines by diff(1): 346 for the 376
 * C): the planes' third products written "eye[2] * planes[k][2]" and "planes[4][2] * (eye[2] -
 * planes[4][2])" (IDO emitted the third product's operands reversed from the source): 250, the
 * plane offsets now the game's to the instruction. Kept.
 * Then (check: 376 to 197, mix 20 to 15): (1) the box sizes are nx, ny, nz themselves, reused
 * later for the counts (the game has bz and nz both in s7: one variable; separate bx/by/bz, 319
 * against 179 in the scratch count), their three words kept as a FAKE pad (removed: 500); (2) the
 * order x0, y0, z0, then x1, y1, z1 (the x0 clamp's reload and slt land where the game has them:
 * every instruction up to the loops is the game's); (3) the FAKE empty tests of bz and z taken
 * out; (4) "(u32) run[0] != 0xFFFF": a constant of another type, so the game's second 0xFFFF
 * register (t1 beside a1) and no copy of run[0]. Measured in the scratch count, not kept: x1
 * right after x0 (389), assign-in-expression chains (389), the clamp as "dimX <= x0" (318) or
 * nested (393); the scan's z as k or i (264-283); "run[0] != 0xFFFFU" (241 on the old base),
 * "*p != 0xFFFFU" (241), "(u16) -1" (271). Kept as the line-best but NOT the game's mix: the
 * permuter's yy copy. The marks written as the game computes them (m1-m4 and m8 with ny; m5-m7
 * with "(y1 - y0 + 1)", the 0xC8 temp, no yy): the game's instruction mix to one nop, but 259
 * against 179 here (registers: v0/v1 and a2/a3 swapped, the z+1 partial sums in s0/s4/s5). Still
 * differing: (a) the game reloads dimY/dimZ into s3/s4 at the marks loop's exit and spills them
 * in the scan's preheader (this C spills before the x guard and never reloads); FAKE "if (dimY)
 * {} if (dimZ) {}" between the loops gives the game's mix (the spill lands before the call);
 * (b) the cell's "(z + z0)" is one add in the game; IDO reassociates it here ("(z + z0) * 1",
 * FAKE, or "(u32) (z + z0)" keep it, but shift the t-register rotation after it by one).
 * A FAKE block boundary after the visible-clear call (the splits sweep's find, on the
 * mix-correct form below): 197 to 183 (mix 15). The mix-correct form, measured by check on a
 * scratch copy: the game's marks (no yy), FAKE "if (dimY) {} if (dimZ) {}" after the marks loop,
 * "(z + z0) * 1" (FAKE), boundaries at the z loop's head and after the call: 224 lines, this C 665
 * long (the game's length), mix distance 4; without the boundaries 331. What it still lacks:
 * the marks' CSE registers (the game: v0 z-z0, v1 (y-y0)*nz, a2 y-y0, a3 grid+X0, the z+1
 * marks' FULL addresses in s0/s4/s5; this C: v0 and v1 swapped, a2 and a3 swapped, the partial
 * sums in s0/s4/s5), y kept in v1 across the y loop's foot (the z-loop boundary loses it), and
 * t8/t9 named the other way round in the scan. Tried for those, no gain: every mark's term order
 * (y first 245, z first 640, the z+1 terms first 353), "(z - z0 + 1)" / "(z + 1 - z0)" /
 * "(1 + y1 - y0)" forms (all the same code), FAKE address tests of z - z0 or y - y0 (721+),
 * marks as a 2-trip loop over k (IDO keeps the loop: 700), the scan's z run from z0 (335+).
 * THE C BELOW (2026-10-03): the game's instructions exactly (check: 246 lines, this C 665
 * long, MIX DISTANCE 0; the base to sweep from, as bmove.c 17CDF8's twin form was). The first
 * mark written "X + (z - z0) + Y" (the rest "X + Y + Z") gives v0 = z - z0 and v1 = (y - y0) * nz
 * as in the game (scratch count 251 to 218); without the z loop's head boundary (it cost y's
 * v1 at the y loop's foot: one sw more, a bne for a bnel) the mix is 0. The line-best form is
 * still the permuter's yy one: the first
 * three marks with y, the last five with "yy = y;" (an s16 copy, FAKE) in place of the
 * (y1 - y0 + 1) products, a FAKE boundary after the second mark, no dimY/dimZ tests, no "* 1":
 * 183 lines but mix 15 (two multu short: it can never match). What still differs here: the
 * marks block's schedule and registers (the game computes y - y0 first, into a2; here a2 is
 * grid + X0 and a3 y - y0; the z+1 marks' full addresses in s0/s4/s5 there, partial sums here),
 * the y loop's foot, and the t-register rotation after it (t8/t9 the other way round in the
 * scan). Next: the permuter or greedy on this form, not on the yy one.
 * FOUND (2026-10-03; check 246 to 40, mix 0): the marks read three locals "dx = x - x0;
 * dy = y - y0; dz = z - z0;" set right before them, and every mark uses ny (no (y1 - y0 + 1), no
 * yy): the three words the game leaves after nz are these three (never stored: uopt keeps them
 * in a0, a2, v0), and copy propagation puts the game's 0xC8 copy of ny into the z+1 marks by
 * itself. A dead "cell = (z - z0) + (y - y0);" before the marks had pointed the way (y - y0 and
 * z - z0 first, the game's registers).
 * Then 40 to 12: the dimY/dimZ tests after the marks loop are not needed with the
 * boundary after the call; run[0] read into a local ("size = run[0]; if ((u32) size !=
 * 0xFFFF)", size being free there): beql a3,t1 as the game has it ("(u32) run[0]" straight gave
 * t1,a3); the "(z + z0) * 1" FAKE now lines the t-registers up with the game. nx, ny and nz on
 * ONE line (the game's nz temp store before nx's home store; on two lines or three, nx's comes
 * first). Left: the scan's dimY/dimZ spill (sh s3, sh s4) sits right after the visible-clear
 * call (at the FAKE boundary) where the game has it in the scan's preheader, after "blez nx; sw
 * zero,x". Forms measured for that (scratch, build-path compiles): the boundary after "start =
 * 0" (the same), after "run = ..." (the spill straddles the blez), inside the x loop (78), the
 * x loop as "if (nx > 0) { x = 0; do {..} while (++x < nx); }" with no boundary (the spill
 * before the blez), with a boundary before "x = 0" (after the blez, but the run load takes the
 * delay slot and x = 0 follows the spill), "x = 0" ahead of that if (the reload is lost: 66), the
 * scan's z as k, i, dx, dz (84-96), FAKE tests of dimY/dimZ before or after the scan (code).
 * Then: the first four planes' offsets are one loop over k (IDO unrolls it fully), and
 * with it the FAKE "if (arg) {}" and the copy g are gone (w everywhere): the same code, two FAKEs
 * fewer. A block-scoped scan x takes a new frame slot (0x218): not that.
 * For the spill, also measured (no gain): a label alone (no boundary: 68), "goto l; l:;" and
 * "while (0) {}" (the same as the boundary), the visible-clear in a "do { } while (0)" macro
 * (the same), the greedy splits sweep for a second boundary (none helps), "z + z0" held in a
 * local (IDO strength-reduces the cell: 634 long), the scan loop "for (x = 0; nx > x; ..)",
 * "x != nx", "++x" (no reload). Whenever "x = 0" is in the scan's guard block (where the game
 * has it, in the blez delay slot) and no boundary precedes it, the reload at the marks exit is
 * lost; with "if (nx > 0) { boundary; x = 0; do .. }" the spill lands in the preheader as in
 * the game but the run load takes the delay slot and x = 0 follows the spill.
 * 2026-10-03 (batch, check 10 at the start): a FAKE address-only "if (!&gGeoTextureFiles[dimY
 * + dimZ]) {}" after "run = ..." (to put dimY/dimZ live in the guard block): 197 (674 long, it
 * leaves code). The boundary after the call taken out (re-measured): 68 (663 long; both spills
 * in the marks guard and its preheader, li s2/addiu s1 pushed into the preheader, no reload at
 * the marks exit). So the block "start = 0; run = ...; x = 0; nx > 0" is what never joins
 * dimY/dimZ's register region here; in the game it does (reload at the marks exit, spill in
 * the scan preheader, as for the marks loop: spill in its preheader). The scan's z as a new
 * s32 zs in pad3's place (the marks' z then dead between the loops): 32, the spill unmoved.
 * The moves sweep (118 moves): nothing under 10. Scratch compiles (a scratch copy,
 * body.inc built with -DV=n, only the spill's place read): the scan's guard block ("start = 0;
 * run = ...; x = 0; nx > 0") joins dimY/dimZ's register region, spill after the blez as in the
 * game, whenever it does NOT hold all three of those: x = 0 before the call (spill right, but
 * the sw zero then precedes the call and the run load takes the delay slot: t2 for t1 too),
 * or, as probes that change behaviour, start or run left uninitialised. With all three in it,
 * every form tried leaves it out: x = 0 first, "start = x = 0", "x = start = 0" (also in the
 * for), comma inits with and without the boundary, "x = start", the three on one line, start
 * as u16 (2 longer), the scan's x as a new local (x then in a register), FAKE address-only
 * uses of dimY/dimZ after the clamps or after the call (no code, no change), of x after the
 * call (folded, or code), volatile x or y (all different). A second boundary before the for:
 * the block "x = 0; nx > 0" alone is still left out ("sw zero; sh s4; blez; sh s3": as many
 * lines). Then guAlignF on w->view straight (view unused): the game's code, spill and all. */
void appear_in_cam(GeoWork *arg) {
    f32 planes[5][4];
    u8 *grid;
    s32 k;
    Vec3f dir;
    MtxF m;
    s32 pad[2];
    s16 dimX;
    s16 dimY;
    s16 dimZ;
    f32 ox;
    f32 oy;
    f32 oz;
    f32 cx;
    f32 cy;
    f32 cz;
    s32 x0;
    s32 y0;
    s32 z0;
    s32 x1;
    s32 y1;
    s32 z1;
    s32 x;
    s32 y;
    s32 z;
    s32 nx;
    s32 ny;
    s32 nz;
    s32 dx;
    s32 dy;
    s32 dz;
    s32 size;
    s32 cell;
    s32 start;
    u16 *run;
    u16 *p;
    f32 c;
    f32 s;
    Geo *geo;
    CamView *view;
    s32 i;
    u8 mode;
    GeoWork *w;

    w = arg;
    mode = w->mode;
    switch (mode) {
        default:
            view = w->view;
            if (view == NULL) {
                return;
            }
            break;
        case 1:
            for (i = 0; i < w->geo->count; i++) {
                w->visible[i] = 1;
            }
            return;
        case 2:
            heapMemClear(w->visible, w->geo->count);
            return;
    }
            geo = w->geo;
            dimX = geo->dims[0];
            dimY = geo->dims[1];
            dimZ = geo->dims[2];
            ox = geo->origin[0] * w->scale;
            oy = geo->origin[1] * w->scale;
            oz = geo->origin[2] * w->scale;
            cx = geo->cell[0] * w->scale;
            cy = geo->cell[1] * w->scale;
            cz = geo->cell[2] * w->scale;
            guAlignF(m.m, w->view->roll, w->view->eye[0] - w->view->at[0], w->view->eye[1] - w->view->at[1],
                     w->view->eye[2] - w->view->at[2]);
            c = cosf(w->view->fovy * 0.8f);
            s = sinf(w->view->fovy * 0.8f);
            geoRotateVec((Vec3f *) planes[0], &m, 0.0f, -c, s);
            geoRotateVec((Vec3f *) planes[1], &m, 0.0f, c, s);
            geoRotateVec((Vec3f *) planes[2], &m, c, 0.0f, s);
            geoRotateVec((Vec3f *) planes[3], &m, -c, 0.0f, s);
            geoRotateVec((Vec3f *) planes[4], &m, 0.0f, 0.0f, -w->view->zFar);
            for (k = 0; k < 4; k++) {
                planes[k][3] = planes[k][0] * w->view->eye[0] + planes[k][1] * w->view->eye[1] +
                               w->view->eye[2] * planes[k][2];
            }
            planes[4][3] = planes[4][0] * (w->view->eye[0] - planes[4][0]) +
                           planes[4][1] * (w->view->eye[1] - planes[4][1]) +
                           planes[4][2] * (w->view->eye[2] - planes[4][2]);
            dir.x = w->view->at[0] - w->view->eye[0];
            dir.y = w->view->at[1] - w->view->eye[1];
            dir.z = w->view->at[2] - w->view->eye[2];
            mathVecNormalizeB(&dir);
            mathVecScale(&dir, w->ahead);
            nx = w->box[0] / cx + 1.0f;
            ny = w->box[1] / cy + 1.0f;
            nz = w->box[2] / cz + 1.0f;
            x0 = (w->view->at[0] + dir.x - ox) / cx - nx / 2;
            y0 = (w->view->at[1] + dir.y - oy) / cy - ny / 2;
            z0 = (w->view->at[2] + dir.z - oz) / cz - nz / 2;
            x1 = x0 + nx;
            y1 = y0 + ny;
            z1 = z0 + nz;
            if (x0 < 0) {
                x0 = 0;
            } else if (x0 >= dimX) {
                x0 = dimX - 1;
            }
            if (y0 < 0) {
                y0 = 0;
            } else if (y0 >= dimY) {
                y0 = dimY - 1;
            }
            if (z0 < 0) {
                z0 = 0;
            } else if (z0 >= dimZ) {
                z0 = dimZ - 1;
            }
            if (x1 < 0) {
                x1 = 0;
            } else if (x1 >= dimX) {
                x1 = dimX - 1;
            }
            if (y1 < 0) {
                y1 = 0;
            } else if (y1 >= dimY) {
                y1 = dimY - 1;
            }
            if (z1 < 0) {
                z1 = 0;
            } else if (z1 >= dimZ) {
                z1 = dimZ - 1;
            }
            nx = x1 - x0 + 1; ny = y1 - y0 + 1; nz = z1 - z0 + 1;
            size = nx * ny * nz;
            grid = heapMallocTop(size);
            if (grid == NULL) {
                PRINTF("メモリが足りません appear_in_cam \n");
                return;
            }
            heapMemClear(grid, size);
            for (x = x0; x < x1; x++) {
                for (y = y0; y < y1; y++) {
                    for (z = z0; z < z1; z++) {
                        for (k = 0; k < 5; k++) {
                            if (geoPlaneDist(planes[k], ox + (x + 1) * cx, oy + (y + 1) * cy,
                                                       oz + (z + 1) * cz) < 0.0f) {
                                goto outside;
                            }
                        }
                        dx = x - x0;
                        dy = y - y0;
                        dz = z - z0;
                        grid[dx * ny * nz + dy * nz + dz] = 1;
                        grid[(dx + 1) * ny * nz + dy * nz + dz] = 1;
                        grid[dx * ny * nz + (dy + 1) * nz + dz] = 1;
                        grid[(dx + 1) * ny * nz + (dy + 1) * nz + dz] = 1;
                        grid[dx * ny * nz + dy * nz + dz + 1] = 1;
                        grid[(dx + 1) * ny * nz + dy * nz + dz + 1] = 1;
                        grid[dx * ny * nz + (dy + 1) * nz + dz + 1] = 1;
                        grid[(dx + 1) * ny * nz + (dy + 1) * nz + dz + 1] = 1;
                    outside:;
                    }
                }
            }
            heapMemClear(w->visible, w->geo->count);
            start = 0;
            run = w->geo->runs;
            for (x = 0; x < nx; x++) {
                for (y = 0; y < ny; y++) {
                    for (z = 0; z < nz; z++) {
                        if (grid[x * ny * nz + y * nz + z] != 0) {
                            dx = x + x0;
                            dy = y + y0;
                            dz = z + z0;
                            cell = dimY * dx * dimZ + dy * dimZ + dz;
                            if (cell < start) {
                                start = 0;
                                run = w->geo->runs;
                            }
                            while (TRUE) {
                                if (cell < run[1]) {
                                    break;
                                }
                                start = run[1];
                                run += 2;
                            }
                            size = run[0];
                            if ((u32) size != 0xFFFF) {
                                for (p = &w->geo->lists[size]; *p != 0xFFFF; p++) {
                                    w->visible[*p] = 1;
                                }
                            }
                        }
                    }
                }
            }
            heapFree(grid);
}

/* Make a frame for each visible placement, after freeing the last ones: a "gpol" (kinds 0, 2)
 * or a "gani" (1, 3), under the root (0, 1) or the list frame (2, 3), placed and scaled by the
 * task's scale, with its object's display list. Its first file comes through the cache, locked
 * for the framebuffer being drawn (no free slot: the frame is dropped), the rest by their RAM
 * addresses.
 * (The frame is the game's 0xA0 with data declared last and two pad words under it. The first
 * file is tested in place and copied to a u32 local inside the if: so the test's load is v0,
 * kept in s0, as the game does; a u16 local adds an andi.) */
void geoMakeFrames(GeoWork *w) {
    GeoPlace *pl;
    GeoObj *obj;
    Frame *f;
    u32 *segs;
    GPolFrame *g;
    GAniFrame *a;
    u32 file;
    s32 old;
    s32 missing;
    s32 i;
    s32 j;
    u8 *data;
    s32 pad[2];

    if (w->visible != NULL) {
        frmDestroyChildren(w->root);
        frmDestroyChildren((Frame *) w->dlf);
        cacheClearLockBit(w->cache, gGameState.fbIndex + 1);
        cacheSetLockBit(w->cache, gGameState.fbIndex + 1);
        missing = 0;
        for (i = 0; i < w->geo->count; i++) {
            if (w->visible[i] == 0) {
                continue;
            }
            pl = &w->geo->places[i];
            obj = pl->obj;
            if (obj->kind == 0) {
                g = (GPolFrame *) frmCreate(w->root, gFrmKindGpol);
                if (g == NULL) {
                    continue;
                }
                geoPlaceMtx(&g->mtx, pl);
                mathMtxScale(&g->mtx, w->scale);
                segs = g->segs;
                f = &g->f;
                g->dl = obj->dl;
            } else if (obj->kind == 1) {
                a = (GAniFrame *) frmCreate(w->root, gFrmKindGani);
                if (a == NULL) {
                    continue;
                }
                a->pos[0] = pl->pos[0] * w->scale;
                a->pos[1] = pl->pos[1] * w->scale;
                a->pos[2] = pl->pos[2] * w->scale;
                a->rot[0] = pl->rot[0] * 3.1415927f / 512;
                a->rot[1] = pl->rot[1] * 3.1415927f / 512;
                a->rot[2] = pl->rot[2] * 3.1415927f / 512;
                a->scale[0] = (f32) pl->scale[0] / 256 * w->scale;
                a->scale[1] = (f32) pl->scale[1] / 256 * w->scale;
                a->scale[2] = (f32) pl->scale[2] / 256 * w->scale;
                segs = a->segs;
                f = &a->f;
                a->anim = (u32) obj->dl;
            } else if (obj->kind == 2) {
                g = (GPolFrame *) frmCreate((Frame *) w->dlf, gFrmKindGpol);
                if (g == NULL) {
                    continue;
                }
                geoPlaceMtx(&g->mtx, pl);
                mathMtxScale(&g->mtx, w->scale);
                segs = g->segs;
                f = &g->f;
                g->dl = obj->dl;
            } else if (obj->kind == 3) {
                a = (GAniFrame *) frmCreate((Frame *) w->dlf, gFrmKindGani);
                if (a == NULL) {
                    continue;
                }
                a->pos[0] = pl->pos[0] * w->scale;
                a->pos[1] = pl->pos[1] * w->scale;
                a->pos[2] = pl->pos[2] * w->scale;
                a->rot[0] = pl->rot[0] * 3.1415927f / 512;
                a->rot[1] = pl->rot[1] * 3.1415927f / 512;
                a->rot[2] = pl->rot[2] * 3.1415927f / 512;
                a->scale[0] = (f32) pl->scale[0] / 256 * w->scale;
                a->scale[1] = (f32) pl->scale[1] / 256 * w->scale;
                a->scale[2] = (f32) pl->scale[2] / 256 * w->scale;
                segs = a->segs;
                f = &a->f;
                a->anim = (u32) obj->dl;
            } else {
                continue;
            }
            if (obj->files[0] != 0) {
                file = obj->files[0];
                old = cacheGetSlot(w->cache, &data, file);
                if (data == NULL) {
                    frmDestroy(f);
                    missing++;
                    continue;
                }
                if (old != file) {
                    if (old != 0) {
                        resDrop(old);
                    }
                    fileLoadNow(file, data);
                }
                segs[0] = (u32) data;
            }
            for (j = 1; j < 11; j++) {
                if (obj->files[j] != 0) {
                    segs[j] = fileRamAddr(obj->files[j]);
                }
            }
        }
        if (missing != 0) {
            PRINTF("geography: フリーのキャッシュ領域が %d 個足りません\n", missing);
        } else {
            PRINTF("geoDisp OK!! \n");
        }
    }
}

/* A placement's matrix: rotation (x, then y, then z), scale by rows, position.
 * The scale is compared and converted straight from the u16 field: that puts the held 0x100
 * first (`beq v1,v0`) and gives the unsigned fix-up (IDO_NOTES 128, 129). */
void geoPlaceMtx(MtxF *m, GeoPlace *p) {
    f32 sx;
    f32 cx;
    f32 sy;
    f32 cy;
    f32 sz;
    f32 cz;
    f32 f;
    f32 sxsy;
    f32 cxsy;

    sx = mathSinCos(p->rot[0], &cx);
    sy = mathSinCos(p->rot[1], &cy);
    sz = mathSinCos(p->rot[2], &cz);
    cxsy = cx * sy;
    m->m[0][0] = cy * cz;
    m->m[0][1] = cy * sz;
    m->m[0][2] = -sy;
    sxsy = sx * sy;
    m->m[1][0] = sxsy * cz - cx * sz;
    m->m[1][1] = sxsy * sz + cx * cz;
    m->m[1][2] = sx * cy;
    m->m[2][0] = cxsy * cz + sx * sz;
    m->m[2][1] = cxsy * sz - sx * cz;
    m->m[2][2] = cx * cy;
    m->m[3][0] = p->pos[0];
    m->m[3][1] = p->pos[1];
    m->m[3][2] = p->pos[2];
    m->m[0][3] = m->m[1][3] = m->m[2][3] = 0.0f;
    m->m[3][3] = 1.0f;
    if (p->scale[0] != 0x100) {
        f = (f32) p->scale[0] / 256;
        m->m[0][0] *= f;
        m->m[0][1] *= f;
        m->m[0][2] *= f;
    }
    if (p->scale[1] != 0x100) {
        f = (f32) p->scale[1] / 256;
        m->m[1][0] *= f;
        m->m[1][1] *= f;
        m->m[1][2] *= f;
    }
    if (p->scale[2] != 0x100) {
        f = (f32) p->scale[2] / 256;
        m->m[2][0] *= f;
        m->m[2][1] *= f;
        m->m[2][2] *= f;
    }
}

/* The box ahead of the eye that the view test keeps to. */
void geoSetViewBox(s32 ahead, s32 x, s32 y, s32 z) {
    GeoWork *w = GEO(gGeographyObj);

    if (gGeographyObj != NULL) {
        w->box[0] = x;
        w->box[1] = y;
        w->box[2] = z;
        w->ahead = ahead;
    }
}

/* The camera it tests against. */
void geoSetView(CamView *view) {
    GeoWork *w = GEO(gGeographyObj);

    if (gGeographyObj != NULL) {
        w->view = view;
    }
}

CamView *geoGetView(void) {
    GeoWork *w = GEO(gGeographyObj);

    if (gGeographyObj == NULL) {
        return NULL;
    }
    return w->view;
}

/* The number of files the cache keeps (from the next stage on). */
void geoSetCacheSize(s32 n) {
    GeoWork *w = GEO(gGeographyObj);

    if (gGeographyObj != NULL) {
        w->cacheSize = n;
    }
}

/* Loads stage n's geography, its objects and textures, and makes the cache and the visible
 * list for them. */
void geoLoadStage(s32 n) {
    GeoWork *w = GEO(gGeographyObj);
    s32 pad;
    StageData d;

    if (gGeographyObj != NULL) {
        stageReadData(&d, n);
        w->geo = geoRelocate((Geo *) d.geo, d.file);
        w->objs = legoMakeMold(d.objFile, w->geo);
        if (w->cache != NULL) {
            cacheDelete(w->cache);
        }
        w->cache = cacheCreate(w->cacheSize,
                                          geoMaxFileSize(w->objs, w->geo->objCount), 0);
        if (w->visible != NULL) {
            heapFree(w->visible);
        }
        w->visible = heapMalloc(w->geo->count);
    }
}

/* Runs the view test and the frame making now. */
void geoRefresh(void) {
    GeoWork *w = GEO(gGeographyObj);

    if (gGeographyObj != NULL && w->geo != NULL && w->objs != NULL && w->cache != NULL) {
        appear_in_cam(w);
        geoMakeFrames(w);
    }
}

/* Sets the view test's mode and runs it now. */
void geoSetMode(s32 mode) {
    GeoWork *w = GEO(gGeographyObj);

    if (gGeographyObj != NULL) {
        w->mode = mode;
        geoRefresh();
    }
}

/* A segmented address in the geography file, in RAM (0: NULL). */
void *geoSegToRam(u32 addr) {
    if (addr != 0) {
        return (void *) ((addr & 0xFFFFFF) + gGeoFileRam);
    }
    return NULL;
}

/* The geography at addr in file: its tables made RAM addresses, and its textures loaded. */
Geo *geoRelocate(Geo *geo, s32 file) {
    u16 *t;

    gGeoFileRam = fileRamAddr(file);
    geo = geoSegToRam((u32) geo);
    geo->runs = geoSegToRam((u32) geo->runs);
    geo->lists = geoSegToRam((u32) geo->lists);
    geo->places = geoSegToRam((u32) geo->places);
    geo->textures = geoSegToRam((u32) geo->textures);
    geo->file = file;
    for (t = geo->textures; *t != 0; t++) {
        PRINTF("地形テクスチャをロードします\n");
        fileLoadNow(gGeoTextureFiles[*t], NULL);
    }
    return geo;
}

/* Loads the object file and keeps only the objects the placements use (of up to 1000),
 * pointing each placement at its copy. "legoMakeMold" by its own prints.
 * - The first loop's if/else with the compiled-out print in the then-branch is what lets IDO
 *   replace the counter by the byte offset (count * 24 recomputed at the test); a plain if keeps
 *   the counter.
 * - pad: the game's frame is 0x40 with objs spilled at 0x24, four unexplained words above. */
GeoObj *legoMakeMold(s32 file, Geo *geo) {
    s32 pad[4];
    u16 *used;
    GeoObj *src = NULL;
    GeoObj *objs;
    s32 i;
    s32 n;
    u32 no;

    used = heapMallocTop(1000 * sizeof(u16));
    if (used != NULL) {
        heapMemClear(used, 1000 * sizeof(u16));
        for (i = 0; i < geo->count; i++) {
            if ((u32) geo->places[i].obj >= 1000) {
                PRINTF("Error: mold No. over in legoMakeMold!\n");
            } else {
                used[(u32) geo->places[i].obj] = 1;
            }
        }
        src = heapMallocTop(fileRamSize(file));
        if (src != NULL) {
            fileLoadNow(file, src);
            geo->objCount = 0;
            for (i = 0; i < 1000; i++) {
                if (used[i]) {
                    geo->objCount++;
                }
            }
            objs = heapMalloc(geo->objCount * sizeof(GeoObj));
            if (objs != NULL) {
                for (n = 0, i = 0; i < 1000; i++) {
                    if (used[i]) {
                        used[i] = n;
                        objs[n] = src[i];
                        n++;
                    }
                }
                resDrop(file);
                heapFree(src);
                for (i = 0; i < geo->count; i++) {
                    no = (u32) geo->places[i].obj;
                    geo->places[i].obj = &objs[used[no]];
                }
                heapFree(used);
                return objs;
            }
        }
    }
    PRINTF("legoMakeMold: メモリーが足りません \n");
    if (used != NULL) {
        heapFree(used);
    }
    if (src != NULL) {
        heapFree(src);
    }
    return NULL;
}

/* The largest main file among n objects (the cache's slot size). */
s32 geoMaxFileSize(GeoObj *objs, s32 n) {
    s32 i;
    s32 max = 0;
    s32 size;

    for (i = 0; i < n; i++) {
        size = fileRamSize(objs[i].files[0]);
        if (max < size) {
            max = size;
        }
    }
    return max;
}
