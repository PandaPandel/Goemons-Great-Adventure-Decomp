#ifndef PLAYER_H
#define PLAYER_H

/* Overlay 9's player ("player", plMain_init). A player's task has a Player as its work (task
 * + 0x20). Its actions (the "PA") are handlers taking (p, msg, a, b): 0 enters one, 1 runs it
 * and picks the next action (plActChange), 2 leaves it; plActDefault is every message's
 * default. Every width below is the one the game's own loads and stores use. */

#include "object.h"
#include "acoll.h"

double sqrt(double x);
#pragma intrinsic (sqrt)

/* A frame-list animation's frame (plmodel.c's faces, demotool.c). */
typedef struct AnimFrame {
    /* 0x00 */ u32 addr;
    /* 0x04 */ s16 duration;
} AnimFrame;

/* An action's animation entry, 0x20 bytes (plmodel.c; the characters' tables). */
typedef struct PlActAnim {
    /* 0x00 */ u32 anim;
    /* 0x04 */ u32 attach; /* where a rider sits */
    /* 0x08 */ s32 events;
    /* 0x0C */ u32 zanzouAnim; /* the after-image's animation (zanzouStart reads it as ZanzouArg.anim) */
    /* 0x10 */ f32 step; /* below 0: backwards from the end */
    /* 0x14 */ s16 seg; /* segment 3's piece */
    /* 0x16 */ s8 atk; /* the attack box's piece, -1 none */
    /* 0x17 */ s8 zanzouPiece; /* another piece, -1 none */
    /* 0x18 */ s8 eventSeSet;
    /* 0x19 */ u8 unused19;
    /* 0x1A */ u16 flags; /* 1 loops, 8 keeps the face, 0x20 glows */
    /* 0x1C */ AnimFrame **face; /* the eyes' and mouth's frames */
} PlActAnim;

/* What a player is made from (0x40 bytes; plMakeInit fills one from the defaults). */
typedef struct PlInit {
    /* 0x00 */ s32 index;
    /* 0x04 */ s32 chara;
    /* 0x08 */ s32 action;
    /* 0x0C */ s32 defaultAction; /* the default action */
    /* 0x10 */ s32 zone;
    /* 0x14 */ f32 place;
    /* 0x18 */ f32 height;
    /* 0x1C */ f32 side;
    /* 0x20 */ f32 lean;
    /* 0x24 */ f32 facing;
    /* 0x28 */ f32 roll;
    /* 0x2C */ f32 topSpeed; /* the fastest along the course */
    /* 0x30 */ f32 maxFallSpeed; /* the fastest fall */
    /* 0x34 */ u32 flags; /* u32: gauge.c's join masks it (gaugeJoinPlayer) */
    /* 0x38 */ s32 flags2;
    /* 0x3C */ s32 flags3;
} PlInit;

typedef struct Player {
    /* 0x000 */ s32 index; /* the player, 0-3 */
    /* 0x004 */ s32 chara; /* which character (zanzou.c reads it as the weapon) */
    /* 0x008 */ s32 action; /* the "PA" */
    /* 0x00C */ s32 queue[3]; /* the actions to take next (-1: none; plqueue.c) */
    /* 0x018 */ s32 defaultAction; /* the action to take when the queue is empty */
    /* 0x01C */ s32 prevAction; /* an action (the dive checks it for 0x4F and 0x50) */
    /* 0x020 */ s32 actionSteps; /* steps in this action */
    /* 0x024 */ s32 totalSteps; /* steps since the player was made */
    /* 0x028 */ s32 state; /* the action's own step */
    /* 0x02C */ s32 phase; /* an angle, 0-0x3FF (a u16 at 0x2E for the sine table) */
    /* 0x030 */ s32 work[6]; /* the action's own */
    /* 0x048 */ u16 buttons; /* buttons (A is tested) */
    /* 0x04A */ u16 held; /* the buttons pressed this step (plmove.c) */
    /* 0x04C */ s32 zone;
    /* 0x050 */ f32 place; /* along the zone's rail */
    /* 0x054 */ f32 height;
    /* 0x058 */ f32 side;
    /* 0x05C */ f32 velX;
    /* 0x060 */ f32 x; /* the place (where the chain's links start from) */
    /* 0x064 */ f32 velXHist[3]; /* PL_HIST's steps [2..4] of each row (below) */
    /* 0x070 */ f32 velY; /* below 0 while falling */
    /* 0x074 */ f32 y;
    /* 0x078 */ f32 velYHist[3];
    /* 0x084 */ f32 velZ;
    /* 0x088 */ f32 z;
    /* 0x08C */ f32 velZHist[3];
    /* 0x098 */ f32 pushX; /* a speed along the rail */
    /* 0x09C */ f32 lastPushX;
    /* 0x0A0 */ f32 pushXHist[3];
    /* 0x0AC */ f32 pushY; /* a speed up */
    /* 0x0B0 */ f32 lastPushY;
    /* 0x0B4 */ f32 pushYHist[3];
    /* 0x0C0 */ f32 pushZ;
    /* 0x0C4 */ f32 pushZHist[4]; /* steps [1..4] */
    /* 0x0D4 */ f32 jumpHeight;
    /* 0x0D8 */ f32 jumpPush;
    /* 0x0DC */ f32 baseGravity;
    /* 0x0E0 */ f32 baseDrag;
    /* 0x0E4 */ f32 gravity; /* twice the pull down */
    /* 0x0E8 */ f32 drag;
    /* 0x0EC */ f32 dragY;
    /* 0x0F0 */ f32 targetFacing;
    /* 0x0F4 */ f32 targetLean;
    /* 0x0F8 */ f32 lean; /* radians */
    /* 0x0FC */ f32 facing; /* radians; under pi/2 faces one way */
    /* 0x100 */ f32 roll;
    /* 0x104 */ f32 leanOffset;
    /* 0x108 */ f32 tilt; /* the demo's walk tilt */
    /* 0x10C */ f32 rollOffset;
    /* 0x110 */ f32 modelOffset[3]; /* added to the model's place */
    /* 0x11C */ f32 startSide;
    /* 0x120 */ f32 width; /* the player's width */
    /* 0x124 */ f32 bodyHeight; /* a height (the splash scales by it) */
    /* 0x128 */ f32 sizeZ; /* the size's third (plFormSizeSet) */
    /* 0x12C */ f32 swimDepth;
    /* 0x130 */ f32 probeDrop; /* the dive's depth, 10 */
    /* 0x134 */ s32 charge; /* a count; 31 or more starts the sparkle's spin */
    /* 0x138 */ s32 releasedCharge; /* the charge, when it was held long enough */
    /* 0x13C */ f32 goalX; /* the demo's place to walk to */
    /* 0x140 */ f32 goalY;
    /* 0x144 */ f32 goalZ;
    /* 0x148 */ f32 goalSpeed;
    /* 0x14C */ s32 goal; /* 1: walking to the place */
    /* 0x150 */ s32 flags; /* 0x10: ; 0x4000: in a special action; 0x20004: on a swing */
    /* 0x154 */ s32 flags2;
    /* 0x158 */ s32 flags3;
    /* 0x15C */ s32 rideHold; /* the piggyback's hold, counted down */
    /* 0x160 */ s32 invincibleTimer; /* with flag 0x80 (plInvincibleSet) */
    /* 0x164 */ s32 blinkTimer; /* with flag 0x100 (plBlinkSet) */
    /* 0x168 */ Obj *ride; /* what the player stands on */
    /* 0x16C */ s32 onigiriTimer;
    /* 0x170 */ s32 slowTimer; /* steps until flag 0x08000000 clears */
    /* 0x174 */ s32 partner; /* the piggyback partner, a Player * (ombuUStart, ombuDStart; plPartnerGet) */
    /* 0x178 */ s32 wallObj; /* the wall's object the player walked into */
    /* 0x17C */ s32 idle; /* the idle demo's steps left */
    /* 0x180 */ Obj *clinger;
    /* 0x184 */ Obj *onigiri; /* the rice ball thrown */
    /* 0x188 */ s32 hitKind;
    /* 0x18C */ s32 hitBy;
    /* 0x190 */ s16 damage;
    /* 0x192 */ s16 weaponDamage;
    /* 0x194 */ s32 pipsCooldown; /* steps before another power-up sounds */
    /* 0x198 */ s32 lifeCooldown;
    /* 0x19C */ s16 ceilingKind; /* the ground ahead, as plmove.c's PlGround */
    /* 0x19E */ s16 wallKind;
    /* 0x1A0 */ s16 ground; /* on the ground */
    /* 0x1A2 */ u8 groundSurface;
    /* 0x1A3 */ u8 unused1A3;
    /* 0x1A4 */ f32 slope; /* an angle, in radians */
    /* 0x1A8 */ u16 edgeFlags;
    /* 0x1AA */ u8 pad1AA[2];
    /* 0x1AC */ s32 waterObj; /* in water */
    /* 0x1B0 */ s32 prevWaterObj;
    /* 0x1B4 */ f32 surfacePos[3]; /* the water's surface, where the player meets it */
    /* 0x1C0 */ f32 toSurface[3]; /* and its normal */
    /* 0x1CC */ f32 waterDepth; /* how deep the player is */
    /* 0x1D0 */ f32 prevWaterDepth; /* and was, a step ago */
    /* 0x1D4 */ s16 waterSeWait; /* steps before unk1D6 sounds */
    /* 0x1D6 */ u16 waterSe; /* a sound to play */
    /* 0x1D8 */ s32 splashWait; /* steps before another splash */
    /* 0x1DC */ f32 jumpSpeed; /* a jump's speed */
    /* 0x1E0 */ f32 jumpCut;
    /* 0x1E4 */ f32 jumpHoldSteps;
    /* 0x1E8 */ f32 maxSpeed; /* the fastest along the course */
    /* 0x1EC */ f32 maxFall; /* the fastest fall */
    /* 0x1F0 */ u16 eventBits; /* the animation's events so far */
    /* 0x1F2 */ u16 eventAnimFlags; /* and, as they came: the animation's flags */
    /* 0x1F4 */ s32 eventList; /* its event list */
    /* 0x1F8 */ f32 eventFrame; /* its frame */
    /* 0x1FC */ f32 eventStep; /* its step */
    /* 0x200 */ s32 eventAction; /* the action */
    /* 0x204 */ u8 seSet;
    /* 0x205 */ u8 eventSeSet;
    /* 0x206 */ u8 pad206[2];
    /* 0x208 */ u32 seAltBits; /* 0x55555555, turned a bit each time the animation's sounds run */
    /* 0x20C */ AnimeFrame *model;
    /* 0x210 */ s32 animStatus; /* the animation's flags: 1 ended, 2 looped */
    /* 0x214 */ s32 loadedPiece; /* the model's segment 3 file, by action */
    /* 0x218 */ s32 atkBoxPart;
    /* 0x21C */ s32 zanzouPart;
    /* 0x220 */ f32 animStep; /* the animation's step */
    /* 0x224 */ f32 animStepAdd; /* added to it */
    /* 0x228 */ u16 animFlags; /* the animation's flags: 1 loops, 8 no Anims, 0x20 a glow */
    /* 0x22A */ u8 pad22A[2];
    /* 0x22C */ s32 animEvents; /* the animation's event list */
    /* 0x230 */ ACollFrame *box; /* the attack box */
    /* 0x234 */ ACollFrame *dmg; /* the damage box */
    /* 0x238 */ Obj *shadow; /* follows the model (shadowPut) */
    /* 0x23C */ s32 ripple; /* the water effects' task (plMoyaCreate) */
    /* 0x240 */ u8 anims[0x30]; /* three Anims (fade.c's): 0x240 and 0x250 the face, 0x260 */
    /* 0x270 */ s32 pieceBuf;
    /* 0x274 */ s32 modelFile; /* the model's file, in RAM */
    /* 0x278 */ s32 texFile; /* its textures' */
    /* 0x27C */ s32 atkBoxBuf;
    /* 0x280 */ s32 zanzouBuf;
    /* 0x284 */ Obj *chain; /* the chain's task ("PL鎖") */
    /* 0x288 */ Obj *cloud; /* character 0's ("goe雲") */
    /* 0x28C */ u8 pad28C[8];
    /* 0x294 */ Obj *bubble; /* character 3's ("yae泡") */
} Player;

/* A player's work from its task. */
#define PL(task) ((Player *) ((u32) (task) + sizeof(Obj)))

/* 0x5C-0xD3 are six histories of five steps each, [0] this step's: the speeds along the course,
 * up and across (0x5C, 0x70, 0x84), then the pushes the same ways (0x98, 0xAC, 0xC0). */
#define PL_HIST(p) ((f32 (*)[5]) &(p)->velX)

/* Character 1 reads 0x284 as two u16s: the honi's, and steps until it lets go. */
#define PL_HONI(p) ((u16 *) &(p)->chain)

/* Message 8's argument, to someone to talk to: who asks, and 0 to ask (pldefault.c), 2 once
 * turned to face it, 3 each step of the talk (pltalk.c). */
typedef struct PlAsk {
    /* 0x0 */ Obj *who;
    /* 0x4 */ s32 stage;
} PlAsk;

void plStickSpeed(Player *p);
s32 plPickAirAttack(Player *p);
s32 plPickAir(Player *p);
s32 plIsOnGround(Player *p);
s32 plIsInWater(Player *p);
s32 plWantsWeapon(Player *p);
s32 plWantsSubWeapon(Player *p);
s32 plStickForward(Player *p);
s32 plPickGroundOrWalk(Player *p);
void plFaceMoveDir(Player *p, f32 a);
void plLeanTo(Player *p, f32 a);
/* Sends obj a tagged message and returns its answer. The tags are two kanji in EUC-JP read as a
 * word: 0xBEC3CCC7 消滅 "vanish", 0xCEA5C3A6 離脱 "leave"; or ASCII: "push", "pull" (a movable
 * object answers "push" to "push"). */
s32 plSendTagMsg(Obj *obj, s32 tag, s32 a, s32 b);
void plAnimeSet(Player *p, s32 n); /* an animation */
void plBaseDefault(Player *p, s32 msg, s32 a, s32 b);
s32 plMoveStickX(s32 index, s32 a);
s32 plMoveStickY(s32 index, s32 a);
s32 plActChange(Player *p, s32 next); /* the next action; -1 keeps this one; nonzero
                                                   * if it changed */
void plActAssign(Player *p, s32 next);
void plMsgReply(s32 a, s32 b);
f32 plStickSpeedX(Player *p); /* the stick, across */
f32 plStickToSpeed(Player *p, s32 a);
f32 plStickSpeedY(Player *p); /* the stick, up */
s32 plAnimEnded(Player *p);
s32 plAnimPercent(Player *p);
s32 plIsInvincible(Player *p);
f32 plPushSize(Player *p);
void plInvincibleSet(Player *p, s32 a);
s32 plWeaponAction(Player *p, s32 a);
void plFormSizeSet(Player *p, s32 a);
void plAttackBoxSet(Player *p, ACollFrame *box, u8 kind);
void plVoice(Player *p, u16 se); /* a voice */
void plSound(Player *p, u16 se); /* a sound */
void plActDefault(Player *p, s32 msg, s32 a, s32 b); /* every action's default */

#endif
