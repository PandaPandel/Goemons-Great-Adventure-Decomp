#include "tlb.h"

/* The "deadhand" enemy kind (archive file 174): a hand that rises out of the ground. Type 0
 * waits for a player within 130, rises, leans toward it and sways, and can be hit twice (the
 * first hit sinks it for a while); it is also a ride (a platform) that drags a player standing
 * on it along the course toward its middle, slowing that player. Type 1 only grows out of the
 * ground and leans toward the nearest player. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* An animation table's entry: the animation, its speed at 60 steps a second, and whether it
 * loops. The entry's number is the hand's state. */
typedef struct DeadhandAnim {
    /* 0x00 */ u32 anim;
    /* 0x04 */ f32 speed;
    /* 0x08 */ u8 loop;
} DeadhandAnim;

/* Where a player stands, as a ride's message 6 passes it (tlb_0069's). */
typedef struct PlPlace {
    /* 0x0 */ s16 zone;
    /* 0x4 */ f32 place;
    /* 0x8 */ f32 side;
    /* 0xC */ f32 height;
} PlPlace;

/* A deadhand's work, at its task + 0x20: an enemy's (EneObj) up to 0x34, then its own. */
typedef struct Deadhand {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u8 type; /* its placement's type: 0 or 1 */
    /* 0x38 */ u8 pad38[4];
    /* 0x3C */ ACollFrame *dmg;
    /* 0x40 */ PolFrame *pol; /* the ride's frame, on the model */
    /* 0x44 */ RideFrame *ride;
    /* 0x48 */ u8 state; /* its animation table's entry */
    /* 0x4C */ f32 speed; /* its animation's, per frame */
    /* 0x50 */ u8 hp; /* type 0: 2 (255 for type 1) */
    /* 0x51 */ s8 unused51; /* written -1 at its start, never read */
    /* 0x52 */ s8 target; /* the nearest player (-1: none) */
    /* 0x53 */ u8 timer;
    /* 0x54 */ f32 phase; /* its sway's angle */
} Deadhand;

#define DH(obj) ((Deadhand *) FX(obj))

extern u8 deadhandType0Anim[];
RideData deadhandRideData[] = { { 1, 141.50972f, { 0 }, { { 0x0000, { 100.0f, 15.0f, 100.0f }, { 0.0f, -10.0f, 0.0f } } } } }; /* the ride's data */
DeadhandAnim deadhandAnimType0[] = {
    { (u32) deadhandType0Anim, 0.0f, 0 },
    { (u32) deadhandType0Anim, 0.4f, 1 },
    { (u32) deadhandType0Anim, 0.8f, 1 },
    { (u32) deadhandType0Anim, 1.0f, 1 },
    { (u32) deadhandType0Anim, 0.0f, 0 },
    { 0x0300056C, 0.5f, 0 },
    { 0x0300056C, 0.0f, 0 },
    { 0x0300056C, -0.5f, 0 },
    { (u32) deadhandType0Anim, 0.0f, 0 },
    { 0x00000000, 0.0f, 0 },
}; /* type 0's animations */
DeadhandAnim deadhandAnimType1[] = {
    { (u32) deadhandType0Anim, 0.0f, 0 },
    { (u32) deadhandType0Anim, 0.4f, 1 },
    { (u32) deadhandType0Anim, 0.8f, 1 },
    { (u32) deadhandType0Anim, 1.0f, 1 },
    { 0x00000000, 0.0f, 0 },
}; /* type 1's */

s32 deadhandActType0(Obj *o);
s32 deadhandActType1(Obj *o);

/* Sets animation n of the table tab, from its first frame. */
void deadhandSetAnime(Obj *o, u8 n, DeadhandAnim *tab) {
    Deadhand *w = DH(o);

    w->state = n;
    w->speed = tab[n].speed / gRetracesPerFrame;
    w->model->anim = tab[n].anim;
    w->model->frame = 0.0f;
}

/* Sets animation n of the table tab, from a random frame. */
void deadhandSetAnimeRand(Obj *o, u8 n, DeadhandAnim *tab) {
    Deadhand *w = DH(o);
    s32 pad;

    deadhandSetAnime(o, n, tab);
    w->model->frame = fadeRandRange(0, polAnimeLastFrame(w->model));
}

void deadhandNop0(void) {
}

/* Sets animation n of the table tab, from the frame given. */
void deadhandSetAnimeAt(Obj *o, u8 n, DeadhandAnim *tab, f32 frame) {
    deadhandSetAnime(o, n, tab);
    DH(o)->model->frame = frame;
}

/* The distance from it to player i (0 if i is not 0-3 or not in play). */
f32 deadhandPlDist(Obj *o, s32 i) {
    f32 dx;
    f32 dy;
    f32 x;
    f32 y;
    f32 z;
    Vec3f pos;

    if (i < 0 || i >= 4) {
        return 0.0f;
    }
    eneWorldPos(o, &pos);
    if (plIsInPlay(i)) {
        plGetModelPos(gGameWork.players[i], &x, &y, &z);
        dx = x - pos.x;
        dy = y - pos.y;
        return sqrtf(dx * dx + dy * dy + (z - pos.z) * (z - pos.z));
    }
    return 0.0f;
}

/* The nearest player in play (-1: none). */
s32 deadhandNearestPl(Obj *o) {
    f32 d;
    f32 min = 65536.0f;
    s32 i;
    f32 x;
    f32 y;
    f32 z;
    s32 pad1;
    s32 pad2;
    s32 near = -1;
    Vec3f pos;

    eneWorldPos(o, &pos);
    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plGetModelPos(gGameWork.players[i], &x, &y, &z);
            d = deadhandPlDist(o, i);
            if (d < min) {
                min = d;
                near = i;
            }
        }
    }
    return near;
}

/* The angle from it to player i, along the ground (0 if i is not 0-3 or not in play). */
f32 deadhandPlAngle(Obj *o, s32 i) {
    f32 dx;
    f32 dy;
    f32 x;
    f32 y;
    f32 z;
    Vec3f pos;

    if (i < 0 || i >= 4) {
        return 0.0f;
    }
    eneWorldPos(o, &pos);
    if (plIsInPlay(i)) {
        plGetModelPos(gGameWork.players[i], &x, &y, &z);
        return mathAtan2(x - pos.x, z - pos.z);
    }
    return 0.0f;
}

void deadhandNop1(void) {
}

/* "deadhand": the kind's proc. Type 0 is a ride: message 6 drags the player standing on it
 * along the course toward the hand's middle and down, and slows the player at that side. */
s32 deadhandProc(Obj *o, s16 msg, void *arg) {
    Deadhand *w = DH(o);
    s32 data;
    s32 i;
    f32 d;
    f32 ang;
    f32 x;
    f32 y;
    f32 z;
    f32 pos[3];
    s32 pad;
    s32 zone;
    s32 r;

    switch (msg) {
        case 2:
            o->name = "deadhand";
            data = eneSetup(o, arg);
            w->type = data & 0xFFFF;
            if (w->type == 1) {
                w->rot[1] = (data >> 16) * 0.01745329238f;
            }
            if (w->type == 0) {
                w->hp = 2;
            } else {
                w->hp = 255;
            }
            w->target = -1;
            w->unused51 = -1;
            if ((w->model = eneNewAnime(o, 0)) == NULL) {
                return -1;
            }
            if (w->type != 0) {
                break;
            }
            if ((w->dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, &w->model->f, 0, 0, 0, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->pol = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol)) == NULL) {
                return -1;
            }
            if ((w->ride = rideCreate(gRideTree1, (s32) o, &w->pol->f,
                                      (RideData *) fileRamAddrAt(0xAE, (u32) deadhandRideData))) == NULL) {
                return -1;
            }
            break;
        case 6:
            if (w->type == 0 && arg != NULL) {
                ang = vramCourseHeading(((PlPlace *) arg)->zone, ((PlPlace *) arg)->place) - 1.570796371f;
                vramCoursePointXZ(((PlPlace *) arg)->zone, ((PlPlace *) arg)->place, 0.0f, &x, &z);
                x -= w->model->pos[0];
                y = ((PlPlace *) arg)->height - w->model->pos[1];
                z -= w->model->pos[2];
                mathVecRotateY(-ang, &x, &y, &z);
                d = gRetracesPerFrame * 0.150000006f;
                if (d < x) {
                    ((PlPlace *) arg)->place -= d;
                    ((PlPlace *) arg)->height -= 0.5f;
                } else if (x < -d) {
                    /* one line: on two, the 0.5f's mtc1 comes after the first load */
                    ((PlPlace *) arg)->place += d; ((PlPlace *) arg)->height -= 0.5f;
                } else {
                    ((PlPlace *) arg)->height -= 0.5f;
                }
                for (i = 0; i < 4; i++) {
                    if (plIsInPlay(i)) {
                        plAskPos(gGameWork.players[i], &pos[0], &pos[1], &pos[2], &zone);
                        if (fabsf(((PlPlace *) arg)->side - pos[2]) < 2.0f) {
                            plSlowDown(i);
                            w->target = i;
                            break;
                        }
                    }
                }
            }
            break;
        case 0:
            switch (w->type) {
                case 0:
                    r = deadhandActType0(o);
                    break;
                case 1:
                    r = deadhandActType1(o);
                    break;
            }
            if (r != 0) {
                return r;
            }
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            if (w->ride != NULL) {
                frmDestroy(&w->ride->f);
            }
            break;
        case 3: /* nothing: four cases over 0-6 give a compare chain, five the game's jump table
                 * (its words 3-5 all go to tskDefaultProc) */
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Type 0: waits (1) for a player within 130, rises (3: leans toward it and sways); a hit sinks
 * it (4, back up after 30 steps), the second (5) lays it down for 120 steps (6) and it rises
 * again (7). */
s32 deadhandActType0(Obj *o) {
    Deadhand *w = DH(o);
    s32 pad;
    s32 done;

    eneMove((EneObj *) w);
    eneSetModelOnCourse(o, &w->model->f);
    if (deadhandAnimType0[w->state].loop == 1) {
        done = eneAnimLoop(o, w->model, w->speed);
    } else {
        done = eneAnimPlayOnce(o, w->model, w->speed);
    }
    if (acollDmgWasHit(w->dmg) == 1) {
        w->hp--;
        eneHitSoundHeavy(w->dmg);
        if (w->hp == 0) {
            w->timer = 0;
            w->dmg->flags &= ~2;
            frmUnlink(&w->ride->f);
            deadhandSetAnime(o, 5, deadhandAnimType0);
        } else {
            w->timer = 0;
            w->dmg->flags &= ~2;
            frmUnlink(&w->ride->f);
            deadhandSetAnime(o, 4, deadhandAnimType0);
        }
    }
    switch (w->state) {
        case 0:
            w->dmg->radius = 35;
            w->dmg->height = 100;
            w->dmg->ofs[1] = -10;
            w->dmg->flags &= ~2;
            w->model->scale[2] = 0.174999997f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->rot[1] = fadeRandRange(0, 180) * 0.01745329238f;
            w->pol->pos[0] = w->model->pos[0];
            w->pol->pos[1] = w->model->pos[1];
            w->pol->pos[2] = w->model->pos[2];
            w->pol->scale[0] = w->model->scale[0];
            w->pol->scale[1] = w->model->scale[1];
            w->pol->scale[2] = w->model->scale[2];
            w->pol->rot[0] = w->model->rot[0];
            w->pol->rot[1] = w->model->rot[1];
            w->pol->rot[2] = w->model->rot[2];
            deadhandSetAnimeRand(o, 1, deadhandAnimType0);
        case 1:
            w->target = deadhandNearestPl(o);
            if (w->target < 0) {
                break;
            }
            if (deadhandPlDist(o, w->target) < 130.0f) {
                w->dmg->flags |= 2;
                deadhandSetAnime(o, 3, deadhandAnimType0);
            }
            break;
        case 3:
            eneTurnToward(o, deadhandPlAngle(o, w->target), gRetracesPerFrame * 0.06981316954f);
            w->rot[0] = 1.570796371f - mathAtan2(20.0f, deadhandPlDist(o, w->target));
            if (0.6108652353f < w->rot[0]) {
                w->rot[0] = 0.6108652353f;
            }
            w->phase += 6.0f;
            w->rot[0] += 0.08726646006f * mathSin(w->phase);
            break;
        case 4:
            if (++w->timer == 30) {
                w->timer = 0;
                w->dmg->flags |= 2;
                frmLink(&w->ride->f, gRideTree1);
                deadhandSetAnime(o, 2, deadhandAnimType0);
            }
            break;
        case 5:
            if ((1.570796371f - w->rot[0]) / 1.570796371f * polAnimeLastFrame(w->model) <= w->model->frame) {
                w->timer = 0;
                deadhandSetAnimeAt(o, 6, deadhandAnimType0, w->model->frame);
            }
            break;
        case 6:
            if (++w->timer >= 120 / gRetracesPerFrame) {
                deadhandSetAnimeAt(o, 7, deadhandAnimType0, w->model->frame);
            }
            break;
        case 7:
            if (done) {
                w->timer = 0;
                w->hp = 2;
                frmLink(&w->ride->f, gRideTree1);
                w->dmg->flags |= 2;
                deadhandSetAnime(o, 3, deadhandAnimType0);
            }
            break;
        case 8:
            PRINTF("*** deadhand:dead ***\n");
            eneReleaseSpawn(w->spawn);
            return -1;
        default: /* an empty block in o's live range keeps o in its home, as the game has it
                  * (without, o takes s1) */
            break;
    }
    return 0;
}

/* Type 1: waits (1) for a player within 130, grows out of the ground (2) and leans toward the
 * nearest player, swaying (3). */
s32 deadhandActType1(Obj *o) {
    Deadhand *w = DH(o);

    eneMove((EneObj *) w);
    eneSetModelOnCourse(o, &w->model->f);
    if (deadhandAnimType1[w->state].loop == 1) {
        eneAnimLoop(o, w->model, w->speed);
    } else {
        eneAnimPlayOnce(o, w->model, w->speed);
    }
    switch (w->state) {
        case 0:
            w->model->scale[2] = 0.174999997f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            deadhandSetAnimeRand(o, 1, deadhandAnimType1);
        case 1:
            w->target = deadhandNearestPl(o);
            if (w->target < 0) {
                break;
            }
            if (deadhandPlDist(o, w->target) < 130.0f) {
                deadhandSetAnimeRand(o, 2, deadhandAnimType1);
            }
            break;
        case 2:
            if (w->model->scale[0] + 0.00800000038f * gGameState.retracesPerFrame < 0.174999997f) {
                w->model->scale[0] += 0.00800000038f * gGameState.retracesPerFrame;
                w->model->scale[1] += 0.00800000038f * gGameState.retracesPerFrame;
                w->model->scale[2] += 0.00800000038f * gGameState.retracesPerFrame;
            } else {
                w->model->scale[0] = 0.174999997f;
                w->model->scale[1] = 0.174999997f;
                w->model->scale[2] = 0.174999997f;
                deadhandSetAnimeRand(o, 3, deadhandAnimType1);
            }
            break;
        case 3:
            eneTurnToward(o, deadhandPlAngle(o, w->target),
                          gGameState.retracesPerFrame * 0.06981316954f);
            w->rot[0] = 1.570796371f - mathAtan2(20.0f, deadhandPlDist(o, w->target));
            if (0.6108652353f < w->rot[0]) {
                w->rot[0] = 0.6108652353f;
            }
            w->phase += 6.0f;
            w->rot[0] += 0.08726646006f * mathSin(w->phase);
            break;
    }
    return 0;
}
