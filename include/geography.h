#ifndef GEOGRAPHY_H
#define GEOGRAPHY_H

#include "object.h"
#include "game.h"
#include "cache.h"

/* A matrix of floats. */
typedef struct MtxF {
    f32 m[4][4];
} MtxF;

/* An object of the object file (0x1C bytes). */
typedef struct GeoObj {
    /* 0x00 */ Gfx *dl;
    /* 0x04 */ u8 kind; /* 0 "mpol", 1 "pol" under the root; 2, 3 the same under the list frame */
    /* 0x05 */ u8 unused5;
    /* 0x06 */ u16 files[11]; /* segments 3-13 */
} GeoObj;

/* A placement (0x18 bytes). */
typedef struct GeoPlace {
    /* 0x00 */ GeoObj *obj; /* an index into the object file until legoMakeMold */
    /* 0x04 */ s16 pos[3];
    /* 0x0A */ u16 rot[3]; /* 1024 a turn */
    /* 0x10 */ u16 scale[3]; /* 256 = 1 */
} GeoPlace;

/* A geography's header, at the start of its data. */
typedef struct Geo {
    /* 0x00 */ f32 origin[3];
    /* 0x0C */ s16 dims[3]; /* cells */
    /* 0x12 */ u8 pad12[2];
    /* 0x14 */ f32 cell[3]; /* a cell's size */
    /* 0x20 */ s32 count; /* placements */
    /* 0x24 */ s32 objCount; /* objects used (legoMakeMold) */
    /* 0x28 */ s32 file;
    /* 0x2C */ u16 *runs; /* (list, end) pairs over the cells */
    /* 0x30 */ u16 *lists; /* placement numbers, each list ending in 0xFFFF */
    /* 0x34 */ GeoPlace *places;
    /* 0x38 */ u16 *textures; /* numbers into gGeoTextureFiles, ending in 0 */
} Geo;

/* The task's work. */
typedef struct GeoWork {
    /* 0x00 */ CamView *view;
    /* 0x04 */ Frame *root;
    /* 0x08 */ DLFrame *dlf; /* its setup list, gGeoSetupDl */
    /* 0x0C */ Geo *geo;
    /* 0x10 */ GeoObj *objs;
    /* 0x14 */ struct Cache *cache;
    /* 0x18 */ f32 scale; /* 0.1 */
    /* 0x1C */ u8 pad1C[0xC];
    /* 0x28 */ u8 *visible; /* one byte a placement */
    /* 0x2C */ u8 pad2C[0x4];
    /* 0x30 */ u8 mode; /* 1: all visible, 2: none */
    /* 0x32 */ s16 cacheSize; /* files kept (30, or the game block's +0x14) */
    /* 0x34 */ s16 box[3]; /* the box ahead of the eye */
    /* 0x3A */ s16 ahead;
} GeoWork;

s32 Geography(Obj *obj, s16 msg, void *arg);
void geoRotateVec(Vec3f *out, MtxF *m, f32 x, f32 y, f32 z);
f32 geoPlaneDist(f32 *plane, f32 x, f32 y, f32 z);
void appear_in_cam(GeoWork *arg);
void geoMakeFrames(GeoWork *w);
void geoPlaceMtx(MtxF *m, GeoPlace *p);
void geoSetViewBox(s32 ahead, s32 x, s32 y, s32 z);
void geoSetView(CamView *view);
CamView *geoGetView(void);
void geoSetCacheSize(s32 n);
void geoLoadStage(s32 n);
void geoRefresh(void);
void geoSetMode(s32 mode);
void *geoSegToRam(u32 addr);
Geo *geoRelocate(Geo *geo, s32 file);
GeoObj *legoMakeMold(s32 file, Geo *geo);
s32 geoMaxFileSize(GeoObj *objs, s32 n);

#endif
