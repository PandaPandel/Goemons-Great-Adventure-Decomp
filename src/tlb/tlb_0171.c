#include "tlb.h"

/* The "pot" object kind (archive file 171): a pot standing on the ground that can be ridden
 * (stood on); a hit breaks it, scattering eight coins (kind 0x1FB) and its "pot_fragment"
 * (kind 0x22C), three shards that fly apart, spinning, for two seconds. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

/* A frame destroyed if it was made. A macro: the game's code for the first one is scheduled as
 * one source line (as1 puts the frmDestroy address's addiu, not w's, in the branch's delay
 * slot), as in tlb_0159. */
#define FRM_DESTROY(f) if ((f) != NULL) { frmDestroy((Frame *) (f)); }

extern u8 potDl[];
RideData potRideData[] = { { 1, 114.45523f, { 0 }, { { 0x0000, { 50.0f, 45.0f, 50.0f }, { 0.0f, 45.0f, 0.0f } } } } }; /* its ride's box (file 0xAB's copy) */
u32 potFragmentDls[] = { 0x03000BE8, 0x030008D8, 0x030008D8 }; /* the shards' display lists */

/* A pot's work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct Pot {
    /* 0x00 */ u8 state; /* 0 set up, 1 standing, 2 broken */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ PolFrame *pol;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *dmg;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ u8 type; /* its placement's type, from eneSetup: 0 stands */
} Pot;

/* A pot_fragment's work, at its task + 0x20: three shards. */
typedef struct PotFragment {
    /* 0x00 */ u8 state; /* 0 thrown, 1 flying */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *shard[3];
    /* 0x40 */ s8 vx[3]; /* in hundredths */
    /* 0x44 */ f32 vy[3];
    /* 0x50 */ s8 vz[3]; /* in hundredths */
    /* 0x53 */ u8 timer;
} PotFragment;

#define POT(obj) ((Pot *) FX(obj))
#define POT_FRAGMENT(obj) ((PotFragment *) FX(obj))

s32 potMain(Obj *o);

/* 1 when it has fallen below -600 or risen above 1000 (and prints which). */
s32 potFallLimit(Obj *o) {
    Pot *w = POT(o);

    if (w->y < -600.0f) {
        PRINTF("eneFallLimit(-600):\n");
        return 1;
    }
    if (w->y > 1000.0f) {
        PRINTF("eneFallLimit(+1000):\n");
        return 1;
    }
    return 0;
}

/* The pot's task. Types 1 and 2 do nothing, and the game then tests ret unset (as gyuki's
 * proc, tlb_0170): its home (the second local's, 0x34) is loaded in the switch's head.
 * Matched 2026-10-12 with types 1, 2 and default under one label group: written as
 * separate cases, or with a FAKE "ret = r" before the switch, IDO loads ret on each edge or
 * before the compare (39-49 differing lines: "ret = r" before, in or around the switch, the
 * cases reordered, a dead print, "ret = ret;", "if (ret) {}", separate "default: break;"). The
 * pad is the first local (ret's home must be the second word). */
s32 potProc(Obj *o, s16 msg, void *arg) {
    Pot *w = POT(o);
    s32 pad;
    s32 ret;

    switch (msg) {
        case 2:
            o->name = "pot";
            w->type = eneSetup(o, arg);
            if ((w->pol = eneNewPol(o, NULL)) == NULL) {
                return -1;
            }
            if ((w->dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, &w->pol->f, 0, 0, 0, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->ride = rideCreate(gRideTree1, (s32) o, &w->pol->f,
                                      (RideData *) fileRamAddrAt(0xAB, (u32) potRideData))) == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                PRINTF("pot: cannot set on ground\n");
                return -1;
            }
            /* fallthrough */
        case 0:
            if (potFallLimit(o)) {
                return -1;
            }
            switch (w->type) {
                case 0:
                    ret = potMain(o);
                    break;
                case 1:
                case 2:
                default:
                    break;
            }
            if (ret != 0) {
                return ret;
            }
            break;
        case 1:
            FRM_DESTROY(w->pol);
            FRM_DESTROY(w->dmg);
            FRM_DESTROY(w->ride);
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* A step: falls, lands; set up (0), it stands (1) until hit, then drops eight coins, loses its
 * box and its ride, and breaks (2): its fragments are made and it is gone. */
s32 potMain(Obj *o) {
    Pot *w;
    s32 i;

    eneFall(o, gRetracesPerFrame * 0.3f);
    w = POT(o);
    eneMove((EneObj *) w);
    if (eneLand((EneObj *) w, 9.0f)) {
        w->vel[1] = 0.0f;
    }
    switch (w->state) {
        case 0:
            w->dmg->radius = 40;
            w->dmg->height = 80;
            w->dmg->flags |= 2;
            w->pol->dl2 = (Gfx *) potDl;
            w->pol->scale[2] = 0.15f;
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
            w->state = 1;
            /* fallthrough */
        case 1:
            if (acollDmgWasHit(w->dmg) == 1) {
                for (i = 17; i < 25; i++) {
                    eneMakeEx(NULL, ENE_ETSK_KOBAN_MAIN, w->zone, w->x, w->y + 2.0f, w->z, 2,
                              (u32 (*)(void *, void *, void *)) i);
                }
                w->dmg->flags &= ~2;
                frmDestroy((Frame *) w->ride);
                eneSetSd(o, 0x248);
                w->state = 2;
            }
            break;
        case 2:
            eneMakeEx(NULL, ENE_POT_FRAGMENT, w->zone, w->x, w->y + 15.0f, w->z, 2, NULL);
            return -1;
    }
    if (w->pol != NULL) {
        eneSetModelOnCourse(o, &w->pol->f);
    }
    return 0;
}

/* The "pot_fragment" kind: three shards thrown apart at random (state 0), then flying and
 * spinning under gravity (1); gone after 120 retraces. */
s32 potFragmentProc(Obj *o, s16 msg, void *arg) {
    PotFragment *w = POT_FRAGMENT(o);
    s32 i;
    f32 a;
    f32 speed;
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "pot_fragment";
            eneSetup(o, arg);
            for (i = 0; i < 3; i++) {
                if ((w->shard[i] = eneNewPol(o, NULL)) == NULL) {
                    return -1;
                }
                w->shard[i]->dl2 = (Gfx *) potFragmentDls[i];
                w->shard[i]->scale[2] = 0.15f;
                w->shard[i]->scale[0] = w->shard[i]->scale[1] = w->shard[i]->scale[2];
            }
            /* fallthrough */
        case 0:
            if (w->state == 0) {
                a = fadeRandRange(-90, 90) * 0.01745329238f;
                for (i = 0; i < 3; i++) {
                    speed = fadeRandRange(40, 60) * gGameState.retracesPerFrame;
                    w->vz[i] = mathCos(a) * speed;
                    w->vx[i] = mathSin(a) * speed;
                    a += fadeRandRange(90, 120) * 0.01745329238f;
                    w->vy[i] = fadeRandRange(10, 15) * 0.1f * gGameState.retracesPerFrame;
                    w->shard[i]->rot[0] = fadeRandRange(-90, 90) * 0.01745329238f;
                    w->shard[i]->rot[1] = fadeRandRange(-90, 90) * 0.01745329238f;
                }
                w->state = 1;
            } else if (w->state == 1) {
                for (i = 0; i < 3; i++) {
                    w->vy[i] += -0.2f * gGameState.retracesPerFrame;
                    w->shard[i]->pos[0] += w->vx[i] * 0.01f;
                    w->shard[i]->pos[1] += w->vy[i];
                    w->shard[i]->pos[2] += w->vz[i] * 0.01f;
                    w->shard[i]->rot[0] +=
                        16.0f * gGameState.retracesPerFrame * 0.01745329238f * ((i & 1) ? 1.0f : -1.0f);
                    w->shard[i]->rot[1] +=
                        -10.0f * gGameState.retracesPerFrame * 0.01745329238f * ((i & 1) ? 1.0f : -1.0f);
                }
            }
            if (++w->timer >= 120 / gGameState.retracesPerFrame) {
                return -1;
            }
            break;
        case 1:
            for (i = 0; i < 3; i++) {
                if (w->shard[i] != NULL) {
                    frmDestroy(&w->shard[i]->f);
                }
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
