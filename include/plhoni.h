#ifndef PLHONI_H
#define PLHONI_H

#include "player.h"

/* A combo's table. */
typedef struct HoniTbl {
    /* 0x00 */ f32 pos[3]; /* the throw's start, from the model */
    /* 0x0C */ f32 vel[3][3]; /* each throw's speed */
    /* 0x30 */ s32 frame[4]; /* the frame each step waits for */
} HoniTbl;

/* A thrown honi. */
typedef struct PlHoni {
    /* 0x00 */ f32 vel[3];
    /* 0x0C */ f32 spin;
    /* 0x10 */ AnimeFrame *frame;
    /* 0x14 */ ACollFrame *box; /* its attack */
    /* 0x18 */ s32 index; /* the thrower */
    /* 0x1C */ ACollFrame *dmg; /* what hits it */
    /* 0x20 */ Frame *ride; /* what stands on it */
    /* 0x24 */ s32 t;
    /* 0x28 */ s32 state; /* 0 flying, 1 still, 2 blinking, 3 gone */
    /* 0x2C */ f32 w; /* its size: across, up, along */
    /* 0x30 */ f32 h;
    /* 0x34 */ f32 d;
    /* 0x38 */ f32 dx; /* the across half-size, turned */
    /* 0x3C */ f32 dz;
    /* 0x40 */ f32 dir;
    /* 0x44 */ f32 scale;
    /* 0x48 */ s32 kind; /* 0-2 */
    /* 0x4C */ s32 owner; /* the thrower */
} PlHoni;

void plHoniActBase(Player *p, s32 msg, s32 a, s32 b);
void plHoniAct(Player *p, s32 msg, s32 a, s32 b);
void plHoniActStand(Player *p, s32 msg, s32 a, s32 b);
void plHoniActSwim(Player *p, s32 msg, s32 a, s32 b);
s32 plHoniComboStep(Player *p, HoniTbl *tbl);
void plHoniThrow(Player *p, s32 i, HoniTbl *tbl, f32 *vel);
s32 honi(Obj *obj, s16 msg, void *arg);
s32 plHoniGroundCheck(PlHoni *w, f32 *vel);
s32 plHoniBitProc(Obj *obj, s16 msg, void *arg);

#endif
