#ifndef PLSHOT_H
#define PLSHOT_H

#include "player.h"

/* What a shot is made with (message 2's argument). */
typedef struct PlShotArg {
    /* 0x00 */ Player *p;
    /* 0x04 */ Vec3f *pos; /* where it starts, from the player's model */
    /* 0x08 */ Vec3f *vel;
    /* 0x0C */ s32 param;
    /* 0x10 */ s32 mode;
    /* 0x14 */ f32 tilt; /* the kunai's tilt */
} PlShotArg;

/* Every shot's work starts so. */
typedef struct PlShot {
    /* 0x00 */ Vec3f vel;
    /* 0x0C */ f32 spin; /* added to the model's frame each step */
    /* 0x10 */ AnimeFrame *model;
    /* 0x14 */ ACollFrame *box;
    /* 0x18 */ s32 index; /* the player's */
} PlShot;

/* The koban coin ("koban"). */
typedef struct PlKoban {
    /* 0x00 */ Vec3f vel;
    /* 0x0C */ f32 spin;
    /* 0x10 */ AnimeFrame *model;
    /* 0x14 */ ACollFrame *box;
    /* 0x18 */ s32 index;
    /* 0x1C */ f32 mtx[4][4]; /* turned the way it flies, for the sparkles */
    /* 0x5C */ AnimeFrame *glow; /* charged */
    /* 0x60 */ PlShotArg arg; /* unk0C: charged; unk10: 1 or 2, how the trail turns */
    /* 0x78 */ AnimeFrame *trail[5]; /* charged */
    /* 0x8C */ s16 alpha[5]; /* and their fade */
} PlKoban;

/* A lock-on target, and how far it is. */
typedef struct PlTarget {
    /* 0x00 */ f32 dist;
    /* 0x04 */ Obj *obj;
} PlTarget;

/* The bomb ("サ爆弾") and the charged bomb ("ため爆"). */
typedef struct PlBomb {
    /* 0x00 */ Vec3f vel;
    /* 0x0C */ f32 spin;
    /* 0x10 */ AnimeFrame *model;
    /* 0x14 */ s32 unused14;
    /* 0x18 */ s32 index;
    /* 0x1C */ ACollFrame *box;
    /* 0x20 */ s32 mode; /* 0 thrown, 1 thrown from a swing, 2 set down */
    /* 0x24 */ s32 timer; /* set down: the steps until it goes off */
    /* 0x28 */ s32 landed;
    /* 0x2C */ s32 size; /* 1, 2 charged */
    /* 0x30 */ s32 inWater;
    /* 0x34 */ s32 wasInWater;
} PlBomb;

void plShotFire(Player *p, Vec3f *pos, Vec3f *vel);
void plSubWeaponFire(Player *p, Vec3f *pos, Vec3f *vel);
void plKobanChargedFire(Player *p, Vec3f *pos, Vec3f *vel);
s32 plOnaraBlow(Player *p, Vec3f *pos, Vec3f *vel);
void plTameBakuFire(Player *p, Vec3f *pos, Vec3f *vel);
s32 plHbazooFire(Player *p, Vec3f *pos, Vec3f *vel);
s32 koban(Obj *obj, s16 msg, void *arg);
void plKobanTrail(PlKoban *w);
s32 shuriken(Obj *obj, s16 msg, void *arg);
s32 saBakudan(Obj *obj, s16 msg, void *arg);
s32 plBombStep(PlBomb *w, s32 charged);
f32 plRandHundredths(s32 n);
s32 bazooka(Obj *obj, s16 msg, void *arg);
s32 onara(Obj *obj, s16 msg, void *arg);
Obj *plOnaraCreate(f32 x, f32 y, f32 z, f32 size, s32 kind);
void plOnaraSetVel(Obj *o, f32 x, f32 y, f32 z);
s32 tameBaku(Obj *obj, s16 msg, void *arg);
void plDbazFire(Player *p);
s32 dbaz(Obj *obj, s16 msg, void *arg);
void plKunaiFire(Player *p, Vec3f *pos, Vec3f *vel);
s32 kunai(Obj *obj, s16 msg, void *arg);
s32 btargM(Obj *obj, s16 msg, void *arg);
s32 plTargetFind(Player *p, PlTarget *t, s32 n);
s32 plTargetFindNew(Player *p, PlTarget *have, s32 nHave, PlTarget *t, s32 n);
s32 plTargetHas(PlTarget *t, s32 n, Obj *obj);
s32 plTargetInReach(Player *p, Obj *obj, f32 *dist);
s32 plTargetWouldFit(PlTarget *t, s32 n, f32 dist, Obj *obj);
s32 plTargetInsert(PlTarget *t, s32 n, f32 dist, Obj *obj);
void plTargetRemove(PlTarget *t, s32 n, Obj *obj);
s32 btarge(Obj *obj, s16 msg, void *arg);
s32 hbazoo(Obj *obj, s16 msg, void *arg);
s32 hshuri(Obj *obj, s16 msg, void *arg);
s32 plShotHitGround(PlShot *w, f32 *at);
f32 plShotDistFromPlayer(Obj *task, PlShot *w);
void plShotSpin(PlShot *w);
void plAnimeStepLoop(AnimeFrame *m, f32 step);
void plShotPlaceAtHand(Player *p, PlShot *w, f32 speed);
void plShotPlaceLean(Player *p, PlShot *w, f32 speed);
void plShotPlace(Player *p, PlShot *w, Vec3f *pos, Vec3f *vel);
void plPlaceTurned(Player *p, AnimeFrame *m, Vec3f *outVel, Vec3f *pos, Vec3f *vel);

#endif
