#ifndef POLYGON_H
#define POLYGON_H

#include "object.h"
#include "geography.h"

/* A "mpol" frame: display lists drawn with a whole matrix of their own. */
typedef struct MPolFrame {
    /* 0x00 */ Frame f;
    /* 0x14 */ MtxF mtx;
    /* 0x54 */ MtxF world; /* mtx times the parent's, less the world origin */
    /* 0x94 */ u16 flags;
    /* 0x98 */ Gfx *dl;
    /* 0x9C */ Gfx *dl2;
    /* 0xA0 */ u32 segs[6];
} MPolFrame; /* size 0xB8: "mpol" */

typedef struct MAniFrame {
    /* 0x00 */ Frame f;
    /* 0x14 */ MtxF mtx;
    /* 0x54 */ MtxF world;
    /* 0x94 */ u16 flags;
    /* 0x98 */ f32 frame;
    /* 0x9C */ Gfx *dl;
    /* 0xA0 */ u32 anim;
    /* 0xA4 */ u32 segs[6];
} MAniFrame; /* size 0xBC: "mani" */

typedef struct GPolFrame {
    /* 0x00 */ Frame f;
    /* 0x14 */ MtxF mtx;
    /* 0x54 */ Gfx *dl;
    /* 0x58 */ u32 segs[11]; /* segments 3-13 */
} GPolFrame; /* size 0x84: "gpol" */

typedef struct GAniFrame {
    /* 0x00 */ Frame f;
    /* 0x14 */ f32 pos[3];
    /* 0x20 */ f32 rot[3];
    /* 0x2C */ f32 scale[3];
    /* 0x38 */ u32 anim;
    /* 0x3C */ u32 segs[11];
} GAniFrame; /* size 0x68: "gani" */

Gfx *polSetSegments(Gfx *gfx, u32 *segs);
Gfx *polDrawMpol(Gfx *gfx, MPolFrame *f);
Gfx *polDrawGpol(Gfx *gfx, GPolFrame *f);
Gfx *polDrawPol(Gfx *gfx, PolFrame *f);
Gfx *polDrawGani(Gfx *gfx, GAniFrame *f);
Gfx *polDrawMani(Gfx *gfx, MAniFrame *f);
Gfx *polDrawAnime(Gfx *gfx, AnimeFrame *f);
s32 polAnimeLastFrame(AnimeFrame *f);
s32 polAnimLastFrame(u32 anim, u32 *segs);
Gfx *polDrawAnimModel(Gfx *gfx, u32 addr, u32 *segs);
Gfx *polDrawModelNode(Gfx *gfx, ModelNode *node);
void polModelNodeMtx(Mtx *m, ModelNode *node);
void polInitPolKind(s32 count);
void polInitAnimeKind(s32 count);
void polInitMpolKind(s32 count);
void polInitManiKind(s32 count);
void polInitGpolKind(s32 count);
void polInitGaniKind(s32 count);

#endif
