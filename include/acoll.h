#ifndef ACOLL_H
#define ACOLL_H

#include "object.h"

/* Attack and damage boxes (acoll.c) and the animated attack chains they can follow
 * (aniatk.c). */

/* Three words a box carries and hands to what it hits. */
typedef struct ACollInfo {
    /* 0x0 */ s32 a;
    /* 0x4 */ s32 b;
    /* 0x8 */ s32 c;
} ACollInfo;

typedef struct ACollFrame {
    /* 0x00 */ Frame frame;
    /* 0x14 */ u16 radius;
    /* 0x16 */ u16 height; /* 0: a sphere */
    /* 0x18 */ f32 pos[3];
    /* 0x24 */ f32 scale[3];
    /* 0x30 */ f32 rot[3];
    /* 0x3C */ s16 ofs[3];
    /* 0x44 */ Frame *model;
    /* 0x48 */ s32 power;
    /* 0x4C */ ACollInfo info;
    /* 0x58 */ u8 flags; /* 1: hit this step; 2: live; 4: keep hits; 8: turned offset; 0x10: sphere test */
    /* 0x59 */ u8 kind;
    /* 0x5A */ u8 index; /* in its chain */
    /* 0x5C */ s32 hitPower; /* the last hit's (the other box's) */
    /* 0x60 */ u8 hitKind;
    /* 0x61 */ u8 lastHitKind;
    /* 0x64 */ f32 hitPos[3];
    /* 0x70 */ ACollInfo hitInfo;
    /* 0x7C */ f32 hitRotY;
    /* 0x80 */ f32 *push; /* damage boxes: a place to push out of attacks */
} ACollFrame; /* size 0x80 ("atk "), 0x84 ("dmg ") */

/* An animated attack chain's link ("aniAtk "). */
typedef struct AniAtkFrame {
    /* 0x00 */ Frame frame;
    /* 0x14 */ Frame *model; /* the model the chain is animated from */
    /* 0x18 */ u32 anim; /* the animation (a 0x08xxxxxx address in file, or a pointer) */
    /* 0x1C */ s32 file;
    /* 0x20 */ u8 link; /* its joint; also how many links up the chain begins */
    /* 0x24 */ f32 time;
    /* 0x28 */ u32 base; /* where the animation's file is, for its offsets */
} AniAtkFrame; /* size 0x2C */

s32 acollLinkInit(void);
s32 acollLinkInitCounts(s32 atk, s32 dmg);
s16 _atkLinkInit(s32 count);
s16 _dmgLinkInit(s32 count);
void acollManagerDelete(void);
void acollAtkPlaceAll(ACollFrame *root);
void acollDmgPlaceAll(ACollFrame *root);
void acollAtkHitsToEnd(ACollFrame *root);
void acollDmgHitsToEnd(ACollFrame *root);
void acollAtkPlace(ACollFrame *f);
void acollDmgPlace(ACollFrame *f);
void acollCheckAll(ACollFrame *atks, ACollFrame *dmgs, s32 depth);
void acollCheckAtk(ACollFrame *atk, ACollFrame *dmgs, s32 depth);
void acollCheckPair(ACollFrame *atk, ACollFrame *dmg);
s32 acollHitSimple(f32 r, f32 h, f32 dx, f32 dy, f32 dz, f32 r2);
void acollAtkClearHits(ACollFrame *root);
void acollDmgClearHits(ACollFrame *root);
void acollAtkRecordHit(ACollFrame *f, ACollFrame *other);
void acollDmgRecordHit(ACollFrame *f, ACollFrame *other);
ACollFrame *_dmgLinkAdd(Frame *parent, Frame *model, f32 x, f32 y, f32 z, f32 sx, f32 sy, f32 sz, f32 rx, f32 ry, f32 rz, u16 radius, u16 height, s16 ox, s16 oy, s16 oz, s32 unused);
void _FrameToPara(Frame *model, f32 *x, f32 *y, f32 *z, f32 *rx, f32 *ry, f32 *rz, f32 *sx, f32 *sy, f32 *sz);
void acollTurnOffset(f32 x, f32 y, f32 z, f32 rx, f32 ry, f32 rz, f32 sx, f32 sy, f32 sz, s16 ox, s16 oy, s16 oz, f32 *outx, f32 *outy, f32 *outz);
void acollBoxPos(f32 x, f32 y, f32 z, f32 rx, f32 ry, f32 rz, f32 sx, f32 sy, f32 sz, s16 ox, s16 oy, s16 oz, f32 *outx, f32 *outy, f32 *outz, u8 turned);
s32 acollHitTest(f32 r, f32 h, f32 dx, f32 dy, f32 dz, f32 r2);
s32 acollSliceInSpan(f32 c, f32 r, f32 y, f32 h, f32 lo);
void acollRelPos(ACollFrame *atk, ACollFrame *dmg, f32 *dx, f32 *dy, f32 *dz);
void acollSetPushPoint(ACollFrame *dmg, f32 dx, f32 dy, f32 dz);
ACollFrame *acollAtkAddYZ(Frame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);
void acollAtkAddXYZ(Frame *parent, Frame *model, u16 radius, s16 ox, s16 oy, s16 oz, s32 a);
ACollFrame *acollDmgAddYZ(Frame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz, s32 a);
void acollDmgAddXYZ(Frame *parent, Frame *model, u16 radius, u16 height, s16 ox, s16 oy, s16 oz, s32 a);
s32 acollAtkWasHit(ACollFrame *f);
s32 acollDmgWasHit(ACollFrame *f);

#endif
