#include "tlb.h"

/* The "kijim" enemy kind (archive file 162): a kijimuna (a tree sprite) that waits until a
 * player is within 80, then hops along x toward or away from the players for 150 steps,
 * turning back when one gets within 25 on its side or falls 50 behind, and is gone after. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 etskKijimAnim[];
extern u8 etskKijimHalf0Dl[];
extern u8 etskKijimHalf1Dl[];

/* A kijim's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Kijim {
    /* 0x00 */ EneObj e;
    /* 0x4C */ u8 type; /* its placement's type, from eneSetup */
    /* 0x4D */ u8 way; /* 1 or 4: the way it hops */
    /* 0x4E */ u8 time; /* steps of hopping left */
} Kijim;

#define KIJIM(obj) ((Kijim *) FX(obj))

void kijimWait(Obj *o);
void kijimLand(Obj *o);
void kijimHop(Obj *o);

s32 Etsk_kijim_main(Obj *o, s16 msg, void *arg) {
    Kijim *w = KIJIM(o);
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "kijim";
            w->type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewAnime(o, (u32) etskKijimAnim)) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 20, 50, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 60, 120, 0, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->e.shadow = (Obj *) shadowCreate()) == NULL) {
                return -1;
            }
            w->e.halves[0] = (u32) etskKijimHalf0Dl;
            w->e.halves[1] = (u32) etskKijimHalf1Dl;
            if (eneSetOnEarth(o) == -1) {
                return -1;
            }
            w->e.y += 90.0f;
            w->e.behave = 0x400000;
            w->e.rot[1] = eneCourseAngle(o, 8);
            w->e.state = 0;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*kijimWait)(o);
                    break;
                case 1:
                    (*kijimLand)(o);
                    break;
                case 2:
                    (*kijimHop)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_kijim_main: 死亡\n");
                return -1;
            }
            break;
        case 1:
            if (w->e.model != NULL) { frmDestroy(w->e.model); }
            if (w->e.atk != NULL) { frmDestroy(&w->e.atk->frame); }
            if (w->e.dmg != NULL) { frmDestroy(&w->e.dmg->frame); }
            if (w->e.shadow != NULL) { frmDestroy((Frame *) w->e.shadow); }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* State 0: waits for a player within 80. */
void kijimWait(Obj *o) {
    Kijim *w = KIJIM(o);

    if (enePlInRangeCheck(o, 80.0f, 0)) {
        w->e.behave |= 0x800;
        w->e.state = 1;
    }
}

/* State 1: once on the ground, hops off away from the nearest player for 150 steps. */
void kijimLand(Obj *o) {
    s32 pad;
    Kijim *w = KIJIM(o);

    if (w->e.hit & 1) {
        w->e.behave &= ~0x800;
        w->way = enePlDirX(o, 0);
        w->time = 150;
        w->e.state = 2;
    }
}

/* State 2: hops; at each hop's start it turns back if a player on its way is within 25, or one
 * behind it is past 50. */
void kijimHop(Obj *o) {
    Kijim *w = KIJIM(o);
    f32 speed = 0.3f;
    s32 pad[2];
    s32 way;

    if (((AnimeFrame *) w->e.model)->frame >= 0.0 && ((AnimeFrame *) w->e.model)->frame < (f64) speed) {
        way = enePlDirX(o, 0);
        if (way == w->way && enePlInRangeCheck(o, 25.0f, 0)) {
            if (w->way == 4) { w->way = 1; } else { w->way = 4; }
        } else if (way != w->way && !enePlInRangeCheck(o, 50.0f, 0)) {
            if (w->way == 4) { w->way = 1; } else { w->way = 4; }
        }
    }
    if (w->time - 1 >= 0) {
        w->time = w->time - 1;
    } else {
        w->e.behave |= 0x10000000;
    }
    eneAnimLoop(o, (AnimeFrame *) w->e.model, speed);
    if (((AnimeFrame *) w->e.model)->frame > 1.0 && ((AnimeFrame *) w->e.model)->frame <= 1.0 + speed) {
        eneSetSd(o, 0x23C);
        if (w->e.hit & 1) {
            w->e.vel[1] = 6.0f;
        }
    }
    if (((AnimeFrame *) w->e.model)->frame > 1.0 && ((AnimeFrame *) w->e.model)->frame <= 5.0) {
        w->e.vel[0] = (w->way == 4) ? -2.0 : 2.0;
    } else {
        w->e.vel[0] = 0.0f;
    }
    if (((AnimeFrame *) w->e.model)->frame > 5.0 && !(w->e.hit & 1)) {
        ((AnimeFrame *) w->e.model)->frame = 5.0f;
    }
    eneFall(o, 12.0 / (4.0 / speed));
    PRINTF("time %f, y %f, vy %f, vx %f, 0x%x\n", ((AnimeFrame *) w->e.model)->frame, w->e.y, w->e.vel[1],
           w->e.vel[0], w->e.behave);
    if (w->way == 4) {
        eneTurnRotYTo(o, eneCourseAngle(o, 8) - 0.7853981853f, -0.1f);
    } else {
        eneTurnRotYTo(o, eneCourseAngle(o, 8) + 0.7853981853f, 0.1f);
    }
}
