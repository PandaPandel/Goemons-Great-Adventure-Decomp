#ifndef VANIME_H
#define VANIME_H

#include "common.h"
#include "object.h"

typedef struct MVtx {
    /* 0x0 */ s16 ob[3];
    /* 0x6 */ u16 flag;
    /* 0x8 */ s16 tc[2];
    /* 0xC */ s8 cn[4];
} MVtx; /* size 0x10, a Vtx */

MVtx *vanimeMakeVtx(MVtx *v, u32 addr, u32 file);
s32 vanimeStep(u32 addr, u32 file, s32 mode, f32 speed, f32 end);
MVtx *vanimeMakeVtxTask(Obj *obj, MVtx *vtx, u32 addr, u32 file);
s32 vanime_act(Obj *obj, u32 addr, u32 file, s32 mode, f32 speed, f32 end);
MVtx *vanimeMakeVtxPair(MVtx *a, MVtx *b, u32 addr, u32 file);

#endif
