#ifndef ENEMY_H
#define ENEMY_H

#include "player.h"
#include "enekind.h"

/* The enemies (overlay 9): enemy.c (the manager), enecommon.c and eneproc.c (the routines they
 * share). */

typedef struct FileSet {
    s16 file[4];
} FileSet;

/* A kind's entry in file 0xC's table. */
typedef struct EneKindInfo {
    /* 0x00 */ s32 (*proc)(Obj *, s16, void *);
    /* 0x04 */ u8 prio;
    /* 0x05 */ u8 flags;
    /* 0x06 */ s16 set;
    /* 0x08 */ s32 file;
    /* 0x0C */ u8 base;
    /* 0x0D */ u8 from;
    /* 0x0E */ u8 to;
    /* 0x0F */ u8 pad;
} EneKindInfo;

typedef struct EneKind {
    /* 0x00 */ s16 kind;
    /* 0x02 */ FileSet files;
    /* 0x0A */ u8 pad[2];
    /* 0x0C */ EneKindInfo info;
} EneKind;

/* What an enemy is made with (message 2's argument). */
typedef struct EneArg {
    /* 0x00 */ s16 zone;
    /* 0x04 */ f32 x;
    /* 0x08 */ f32 y;
    /* 0x0C */ f32 z;
    /* 0x10 */ EneKind *kind;
    /* 0x14 */ s16 mode; /* how c is: 1 called now, 2 returned, 3 returned mapped */
    /* 0x18 */ u32 (*c)(void *, void *, void *);
} EneArg;

/* An enemy's work, at its task + 0x20. The pieces an enemy breaks into ("kill_hahen" and
 * "futtobi_hahen", enecommon.c) are tasks with this work too: they move and land as an enemy
 * does (eneMove, eneLandOff) and keep their own counters where an enemy keeps its boxes, the same
 * words read as other types (kill_hahen lw/sw 0x34, futtobi_hahen lwc1/swc1 0x34; lbu 0x3C, 0x48). */
typedef struct EneObj {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone; /* the course (0: x and z are absolute) */
    /* 0x02 */ s16 spawn; /* its placement's number, from 1 (0: none) */
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind; /* the last hit's attack kind */
    /* 0x0A */ u8 hit; /* 1 on the ground, 2 and 4 the wall checks' results, 8 its attack hit,
                        * 0x10 hit this step */
    /* 0x0B */ s8 hp;
    /* 0x0C */ Frame *model;
    /* 0x10 */ f32 x; /* along the course */
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z; /* across it */
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ union {
        ACollFrame *atk; /* its attack boxes (a frame tree) */
        s32 time; /* a "kill_hahen" piece's frames to its burst */
        f32 h; /* a "futtobi_hahen" piece's height, for the walls */
    };
    /* 0x38 */ union {
        ACollFrame *dmg; /* its damage box */
        s32 count; /* "kill_hahen": bounces left; "futtobi_hahen": frames left */
    };
    /* lit and lit2 are the top bytes of shadow and behave, as on the N64 (lit2 = 1 is behave's
     * 0x01000000, which eneHitValidCheck tests); a little-endian build keeps them there. */
    /* 0x3C */ union {
        Obj *shadow;
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        struct {
            u8 litPad[sizeof(Obj *) - 1];
            u8 lit;
        };
#else
        u8 lit; /* "kill_hahen": bursts with a flash (and sound 0x522) */
#endif
    };
    /* 0x40 */ u32 halves[2]; /* its two halves' display lists, for a cut (eneDeathAct) */
    /* 0x48 */ union {
        u32 behave; /* what eneAfterProc (eneAfterProc) does for it: see there */
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        struct {
            u8 lit2Pad[3];
            u8 lit2;
        };
#else
        u8 lit2; /* "futtobi_hahen": bursts with a flash */
#endif
    };
} EneObj;

/* A townsperson's table entry (person.c; demo.c's talkers too). */
typedef struct PersonData {
    /* 0x00 */ s32 talk; /* its scenario (message) number */
    /* 0x04 */ u32 path; /* its walk: a list of commands */
    /* 0x08 */ u8 radiusHeight[2]; /* its radius and height */
    /* 0x0A */ u8 hours; /* when it is out: personHoursCheck */
    /* 0x0C */ f32 rotY;
    /* 0x10 */ u8 camera; /* 0: the talk camera looks at both; 1: it closes in */
} PersonData;

/* A townsperson's work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct PersonObj {
    /* 0x00 */ u8 state; /* 0 walking, 1 turning to the player, 2 starting to talk, 3 talking,
                          * 4 done */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ Frame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PersonData *data;
    /* 0x38 */ ACollFrame *box; /* its talk box (kind 0x65) */
    /* 0x3C */ u8 flags; /* 1 talked to, 2 talking, 4 its animation backwards, 8 its animation
                         * ended, 0x10 once through, 0x40 an "anime" model */
    /* 0x3D */ u8 count;
    /* 0x3E */ u8 pad3E[2];
    /* 0x40 */ Obj *talker; /* the player talking to it */
    /* 0x44 */ u32 *anims; /* 0: standing, 1: walking, then its own */
    /* 0x48 */ u16 *path; /* the command it is at */
    /* 0x4C */ u8 step; /* frames into the command */
    /* 0x4D */ u8 pad4D[3];
    /* 0x50 */ Obj *shadow;
    /* 0x54 */ u8 mode; /* 0: its table in main RAM; 1: in its file */
    /* 0x55 */ u8 pad55[3];
    /* 0x58 */ f32 animSpeed;
    /* 0x5C */ s8 prev; /* gGameWork.partake while it talks */
} PersonObj;

/* A talk message's argument. */
typedef struct PersonTalk {
    /* 0x00 */ Obj *player;
    /* 0x04 */ s32 msg; /* 0 ENEPL_TALK_START, 2 ENEPL_TALK_STANDBY, 3 */
} PersonTalk;

#define FX(obj) ((void *) ((u32) (obj) + sizeof(Obj)))
#define ENE(obj) ((EneObj *) FX(obj))
#define PERSON(obj) ((PersonObj *) FX(obj))

/* A placement in the stage data. */
typedef struct EneDef {
    /* 0x00 */ s16 zone; /* the course (0: x and z are absolute) */
    /* 0x02 */ s16 x; /* the place along the course */
    /* 0x04 */ s16 y;
    /* 0x06 */ s16 z; /* the place across it */
    /* 0x08 */ s16 kind;
    /* 0x0A */ s16 mode;
    /* 0x0C */ s32 param;
} EneDef;

/* The data a placement's param points at, by its kind (eneSetup's value, a stage address);
 * PersonData (below) is the townspeople's. */

/* "connect" (ENE_CONNECT, tlb_0101.c connectProc): a stage exit. */
typedef struct ConnectData {
    /* 0x00 */ f32 radius; /* how near (a box) a player must be */
    /* 0x04 */ s32 stage; /* where it leads */
    /* 0x08 */ s32 angle;
    /* 0x0C */ f32 x; /* the spawn there */
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ f32 facing;
    /* 0x1C */ u16 tune; /* played as it fades (0: none) */
    /* 0x1E */ u8 run; /* 2 runs back, 4 stands (else runs on) */
    /* 0x1F */ u8 kind; /* 2: every player; 4: the first near, standing; 8: any player; else
                         * the first near */
} ConnectData;

/* An exit to the atlas (tlb_0100.c, kinds 248 and 249). */
typedef struct ExitData {
    /* 0x00 */ f32 range;
    /* 0x04 */ s32 area; /* the atlas's area and point it goes to */
    /* 0x08 */ s32 point;
    /* 0x0C */ s32 connect; /* -1: none */
    /* 0x10 */ u8 run; /* how the players leave: 4 frozen, 2 walking to -x, else to +x */
} ExitData;

/* "doors" (tlb_0106.c, kind 295 and 0x19B). */
typedef struct DoorsData {
    /* 0x00 */ f32 angle; /* in radians */
    /* 0x04 */ u8 kind; /* its model, 0-20 */
} DoorsData;

/* tanu (tlb_0099.c tanuProc, kind 243 through tanu_j and tanu_eff): "area %d point %d connect
 * [%d][%d][%d][%d] tegata %d". */
typedef struct Tlb99Data {
    /* 0x00 */ s32 area;
    /* 0x04 */ s32 point;
    /* 0x08 */ s32 connect[4]; /* -1: none */
    /* 0x18 */ s32 tegata;
} Tlb99Data;

/* "wsmaster2" and "wsound2" (tlb_0051.c, kinds 91 and 92): count is the number of words after
 * it, and each is read only when the count reaches it. */
typedef struct WsSound2Data {
    /* 0x00 */ s32 count;
    /* 0x04 */ s32 sound; /* the master's sound; a wsound2 finds its master by it (message 10) */
    /* 0x08 */ s32 near; /* wsound2: full volume within (100) */
    /* 0x0C */ s32 far; /* wsound2: silent from (500) */
    /* 0x10 */ s32 volume; /* wsound2: the level at full volume (0) */
    /* 0x14 */ s32 panScale; /* wsound2: its pan scale in hundredths (100) */
    /* 0x18 */ s32 quiet; /* wsmaster2: the level it rests at (-255) */
} WsSound2Data;

/* The lift master (tlb_0055.c, kind 119). */
typedef struct SeriMasterData {
    /* 0x00 */ u32 unused00; /* nothing reads it */
    /* 0x04 */ u32 snd; /* the lift sound */
    /* 0x08 */ u32 sndStep; /* the step sound */
} SeriMasterData;

/* The kkoma master (tlb_0060.c, kind 129). */
typedef struct KkMasterData {
    /* 0x00 */ s32 on; /* answers message 10 and plays sound 0x15B (nonzero) */
    /* 0x04 */ s32 home[3]; /* where its kkoma are made */
} KkMasterData;

/* A placement, as the manager keeps it. */
typedef struct EneSpawn {
    /* 0x00 */ u8 flags; /* 0-2: the reach class; 0x80 made */
    /* 0x01 */ u8 timer; /* steps before it may appear again */
    /* 0x02 */ u8 from;
    /* 0x03 */ u8 to;
    /* 0x04 */ f32 x;
    /* 0x08 */ f32 y;
    /* 0x0C */ f32 z;
    /* 0x10 */ EneDef *def;
} EneSpawn; /* size 0x14 */

/* The manager's work. */
typedef struct EneWork {
    /* 0x00 */ s32 count; /* placements */
    /* 0x04 */ EneSpawn *spawns;
    /* 0x08 */ s32 nKinds;
    /* 0x0C */ EneKind *kinds;
    /* 0x10 */ Obj *bases[5];
    /* 0x24 */ u8 paused;
    /* 0x26 */ s16 mode; /* 1: messages go to forward */
    /* 0x28 */ Obj *forward;
} EneWork;

s32 eneManagerProc(Obj *obj, s16 msg, void *arg);
Obj *eneGetBase(s32 i);
void eneMakeManager(Obj *parent);
void eneSpawnStart(void);
void eneEnableSpawn(s32 on);
void eneSpawnInReach(EneWork *w);
void eneReleaseSpawn(s32 n);
void eneClearSpawnWait(s32 n);
void eneDespawnFar(Obj *o, Vec3f *cam);
void eneDespawnFarAll(Obj *obj);
void eneMakeBenchOne(EneDef *a, s32 na);
void eneMakeBench(EneDef *a, s32 na, EneDef *b, s32 nb);
EneKind *eneFindKind(s32 kind);
s32 eneFillSpawns(EneWork *w, s32 n, EneDef *d, s32 nd);
void eneEatEnemyOne(EneDef *a, s32 na);
void geoEatEnemy(EneDef *a, s32 na, EneDef *b, s32 nb);
Obj *eneMake(s32 kind, s32 zone, f32 x, f32 y, f32 z);
Obj *eneMakeUnder(Obj *parent, s32 kind, s32 zone, f32 x, f32 y, f32 z);
Obj *eneMakeEx(Obj *parent, s32 kind, s32 zone, f32 x, f32 y, f32 z, s16 mode, u32 (*c)(void *, void *, void *));
Obj *eneMakeBaseTask(s32 i, s32 (*proc)(Obj *, s16, void *));
u32 eneSetup(Obj *o, EneArg *a);
AnimeFrame *eneNewAnime(); /* (Obj *o, anim), unprototyped: nobukoProc and kabukiProc pass the anim
                            * as a pointer; a void * parameter changes tlb_0048's code (measured) */
AnimeFrame *eneNewAnime2(Obj *o, u32 anim, Gfx *dl);
PolFrame *eneNewPol(Obj *o, Gfx *dl);
PolFrame *eneNewPol2(Obj *o, Gfx *dl2, Gfx *dl);
s32 eneGetFile(Obj *o, s32 n);
void eneGetSegs(Obj *o, u32 *segs);
s32 eneIsEnemy(Obj *o);
void eneSetMode(s32 mode);
void eneSetForward(Obj *forward, s32 mode);
void eneGetPos(Obj *o, Vec3f *out);

#endif
