#include "object.h"
#include "anim.h"
#include "dma.h"
#include "file.h"
#include "mathutil.h"
#include "model.h"

/* Models in files: a part's data is loaded into RAM and read through segmented offsets (the low
 * 24 bits) from its base. A model's header holds (+8) the offset of its nodes (ModelNode, in
 * object.h). */

/* The node i nodes on from node (a child or sibling index). */
#define NODE_AT(node, i) ((ModelNode *) ((u8 *) (node) + (i) * sizeof(ModelNode)))

extern u8 gFileLoaderBusy; /* the file loader is busy */

/* Loads part n of a file (table: the parts' segmented offsets) to dest; returns the base that
 * the part's offsets are from. */
s32 mdlLoadPart(u16 file, u32 *table, s32 n, u8 *dest) {
    u8 loading;
    u32 off;
    u32 rom;
    s32 size;

    if (file == 0 || table == NULL || dest == NULL) {
        return 0;
    }
    loading = gFileLoaderBusy;
    off = table[n] & 0xFFFFFF;
    rom = fileRomStart(file) + off;
    if (table[n + 1] == 0) {
        size = fileRomEnd(file) - fileRomStart(file) - off;
    } else {
        size = table[n + 1] - table[n];
    }
    gFileLoaderBusy = 1;
    dmaReadRom(rom, dest, size);
    gFileLoaderBusy = loading;
    return (s32) dest - off;
}

/* Swaps display lists in a model (at segmented addr from base): each node's from[i] becomes
 * to[i]. */
s32 mdlSwapDLs(u32 addr, u8 *base, u32 *from, u32 *to) {
    s32 n;
    ModelNode *node;
    s32 i;

    if (addr == 0 || base == NULL || from == NULL || to == NULL) {
        return 0;
    }
    node = (ModelNode *) ((*(u32 *) ((u32) base + (addr & 0xFFFFFF) + 8) & 0xFFFFFF) + base);
    n = mdlListCount(to);
    for (i = 0; from[i] != 0; i++) {
        if (node->dl == from[i] && i < n) {
            node->dl = to[i];
        }
    }
    if (node->child != 0) {
        mdlNodeSwapDLs(NODE_AT(node, node->child), from, to);
    }
    if (node->sibling != 0) {
        mdlNodeSwapDLs(NODE_AT(node, node->sibling), from, to);
    }
    return 1;
}

/* A model's first drawn node's position (in 1/32 units) at an animation's frame. */
void mdlAnimNodePos(u32 addr, f32 frame, u8 *base, f32 *x, f32 *y, f32 *z) {
    ModelNode *node = mdlFirstDrawnNode(addr, frame, base);
    u8 *b = base;

    animSeek(addr, frame, &b);
    if (x != NULL) {
        *x = animGetPos(node->pos[0]) * 0.03125f;
    }
    if (y != NULL) {
        *y = animGetPos(node->pos[1]) * 0.03125f;
    }
    if (z != NULL) {
        *z = animGetPos(node->pos[2]) * 0.03125f;
    }
}

/* And its rotation (512 units a half turn), in radians. */
void mdlAnimNodeRot(u32 addr, f32 frame, u8 *base, f32 *rx, f32 *ry, f32 *rz) {
    ModelNode *node = mdlFirstDrawnNode(addr, frame, base);
    u8 *b = base;
    u16 rot[3];

    animSeek(addr, frame, &b);
    animGetRot(node, rot);
    if (rx != NULL) {
        *rx = rot[0] * PI / 512;
    }
    if (ry != NULL) {
        *ry = rot[1] * PI / 512;
    }
    if (rz != NULL) {
        *rz = rot[2] * PI / 512;
    }
}

/* A model's first drawn node's matrix at an animation's frame: its position (1/32), rotation
 * (512 a half turn) and scale (1/256). The last scale is called for before the arithmetic and
 * kept in a u16 local, which sits in rot's alignment padding (a u32 local would grow the frame
 * by 8; the call inline in the arguments costs two compiler temporaries). */
void mdlAnimNodeMtx(u32 addr, f32 frame, u8 **base, f32 out[4][4]) {
    ModelNode *node;
    u16 sz;
    u16 rot[3];
    struct {
        s16 pos[3];
        u16 scale[3];
    } t;

    node = mdlFirstDrawnNode(addr, frame, *base);
    animSeek(addr, frame, base);
    animGetRot(node, rot);
    t.pos[0] = animGetPos(node->pos[0]);
    t.pos[1] = animGetPos(node->pos[1]);
    t.pos[2] = animGetPos(node->pos[2]);
    t.scale[0] = animGetScale(node->scale[0]);
    t.scale[1] = animGetScale(node->scale[1]);
    sz = animGetScale(node->scale[2]);
    mathMtxTransRotScale(out, t.pos[0] * 0.03125f, t.pos[1] * 0.03125f, t.pos[2] * 0.03125f,
                           rot[0] * PI / 512, rot[1] * PI / 512, rot[2] * PI / 512,
                           t.scale[0] * 0.00390625f, t.scale[1] * 0.00390625f,
                           sz * 0.00390625f);
}

/* How many entries a list has before its 0. */
s32 mdlListCount(u32 *list) {
    s32 n = 0;

    while (*list != 0) {
        n++;
        list++;
    }
    return n;
}

/* Swaps a node's display list (and its children's and siblings') from one list to the other. */
s32 mdlNodeSwapDLs(ModelNode *node, u32 *from, u32 *to) {
    s32 n = mdlListCount(to);
    s32 i;

    if (node == NULL) {
        return 0;
    }
    for (i = 0; from[i] != 0; i++) {
        if ((node->dl & 0x0FFFFFFF) == from[i] && i < n) {
            node->dl = to[i];
        }
    }
    if (node->child != 0) {
        mdlNodeSwapDLs(NODE_AT(node, node->child), from, to);
    }
    if (node->sibling != 0) {
        mdlNodeSwapDLs(NODE_AT(node, node->sibling), from, to);
    }
    return 1;
}

/* A model's first node with a display list. */
ModelNode *mdlFirstDrawnNode(u32 addr, f32 unused, u8 *base) {
    ModelNode *node =
        (ModelNode *) ((*(u32 *) (((addr + 8) & 0xFFFFFF) + base) & 0xFFFFFF) + base);

    while (node->dl == 0) {
        node += node->child;
    }
    return node;
}
