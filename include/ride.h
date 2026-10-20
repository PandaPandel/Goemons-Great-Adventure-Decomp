#ifndef RIDE_H
#define RIDE_H

#include "object.h"
#include "geography.h"

/* A box (0x68 bytes). */
typedef struct RideBox {
    /* 0x00 */ u16 flags; /* 1, 2, 4: its x, y or z faces are open */
    /* 0x04 */ f32 half[3];
    /* 0x10 */ f32 pos[3];
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ MtxF inv;
} RideBox;

/* A set of boxes, as a model's data holds it. */
typedef struct RideData {
    /* 0x00 */ s16 count;
    /* 0x04 */ f32 radius;
    /* 0x08 */ u8 pad8[4];
    /* 0x0C */ RideBox boxes[1];
} RideData;

/* A "ride" frame (0x78 bytes). */
typedef struct RideFrame {
    /* 0x00 */ Frame f;
    /* 0x14 */ s32 id;
    /* 0x18 */ Frame *target; /* the model it moves with */
    /* 0x1C */ s16 count;
    /* 0x20 */ f32 radius;
    /* 0x24 */ RideBox *boxes;
    /* 0x28 */ f32 reach; /* the radius times the model's largest scale */
    /* 0x2C */ f32 center[3];
    /* 0x38 */ MtxF inv;
} RideFrame;

s32 Ride(Obj *obj, s16 msg, void *arg);
void rideFollowModel(RideFrame *r);
void rideUpdateTree(Frame *f);
void rideInitBoxMtx(RideData *d);
s32 rideBoxStop(RideBox *b, Vec3f *from, Vec3f *to, Vec3f *move, f32 *tp);
f32 rideFaceStopT(f32 a, f32 b, f32 c, f32 da, f32 db, f32 dc, f32 ha, f32 hb, f32 hc);
s32 rideBoxPushOut(RideBox *b, Vec3f *p, Vec3f *push);
s32 rideBoxOnto(RideBox *b, Vec3f *from, Vec3f *to, Vec3f *move, f32 *tp);
f32 rideFaceOntoPos(f32 a, f32 b, f32 c, f32 da, f32 db, f32 dc, f32 ha, f32 hb, f32 hc, f32 *tp);
s32 rideToWorld(Frame *t, Vec3f *v);
s32 rideTreePointIn(Frame *f, Vec3f *p, Vec3f *push, s32 skip);
void rideClearHit(void);
void rideTreeMoveHit(Frame *f, Vec3f *from, Vec3f *to, f32 len, s32 skip);
s32 rideAskMsg7(Obj *obj);
s32 rideTree2PointIn(Vec3f *p, Vec3f *push);
s32 ridePointInTurned(Vec3f *p, Vec3f *push, f32 angle, s32 skip);
s32 rideMoveOnto(Vec3f *p, Vec3f *d, Vec3f *out, s32 skip);
s32 rideMoveStop(Vec3f *p, Vec3f *d, Vec3f *out, s32 skip);
s32 rideMoveOntoTurned(Vec3f *p, Vec3f *d, Vec3f *out, f32 angle, s32 skip);
RideFrame *rideCreate(Frame *parent, s32 id, Frame *target, RideData *data);
void rideModelMtx(Frame *t, Mtx *m);
Gfx *rideDraw(Gfx *gfx, Frame *frame);

#endif
