#include "tlb.h"

/* File 146: "lantern", a paper-lantern ghost: at night it bobs and sways at its place (type 1)
 * or waits (type 2); a player near it makes it swing out at him and back; by day (5:00-19:00) it
 * rises away. Its flame is a second model that blinks while it is hurt. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* libultra's, through their addresses (code at 0x08000000 cannot jal them). */
#define sinf (*sinf)
#define cosf (*cosf)

extern u8 etskLanternAnim[];
extern u8 etskLanternFlameDl[];
extern u8 etskLanternHalf0Dl[];
extern u8 etskLanternHalf1Dl[];

/* A lantern's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Lantern {
    /* 0x00 */ EneObj e;
    /* 0x4C */ PolFrame *flame;
    /* 0x50 */ f32 phase; /* the sway's angle */
    /* 0x54 */ f32 lastX; /* x the step before */
    /* 0x58 */ s16 homeX; /* where it was placed */
    /* 0x5A */ s16 homeY;
    /* 0x5C */ u8 type;   /* its placement's (eneSetup): 1 bobbing, 2 waiting */
    /* 0x5D */ u8 dist;   /* to the player it swings at */
    /* 0x5E */ u8 way;    /* to the nearest player (eneNearestPlWay) */
    /* 0x5F */ u8 hurt;   /* steps it blinks after a hit; it waits meanwhile */
} Lantern;

#define LANTERN(obj) ((Lantern *) FX(obj))

/* Each state flaps at a local speed and sounds the flap when the animation passes frame 9 (the
 * speed a variable: "9.0 + speed" is added at run time; on separate lines, not a one-line macro:
 * IDO schedules by line). */

void lanternBob(Obj *o);
void lanternRiseSway(Obj *o);
void lanternWait(Obj *o);
void lanternSwing(Obj *o);
void lanternRise(Obj *o);

/* "lantern". */
s32 Etsk_lantern_main(Obj *o, s16 msg, void *arg) {
    Lantern *w = LANTERN(o);

    switch (msg) {
        case 2:
            o->name = "lantern";
            w->type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewAnime(o, (u32) etskLanternAnim)) == NULL) {
                return -1;
            }
            if ((w->flame = enePolModelMake(o, (Gfx *) etskLanternFlameDl, NULL, 1)) == NULL) {
                return -1;
            }
            floadGetSetRam((void **) w->flame->segs, 0x120);
            w->flame->flags |= 2;
            if ((w->e.shadow = (Obj *) shadowCreate()) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 45, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 80, 130, -65, 0, 0)) == NULL) {
                return -1;
            }
            w->e.halves[0] = (u32) etskLanternHalf0Dl;
            w->e.halves[1] = (u32) etskLanternHalf1Dl;
            w->e.hp = 0;
            w->e.behave = 0x400002;
            w->lastX = w->e.x;
            w->homeX = w->e.x;
            w->homeY = w->e.y;
            w->e.rot[1] = eneCourseAngle(o, 8);
            if (w->type == 1) {
                ((AnimeFrame *) w->e.model)->scale[0] = ((AnimeFrame *) w->e.model)->scale[1] =
                    ((AnimeFrame *) w->e.model)->scale[2] = 0.2f;
                w->way = eneNearestPlWay(o);
                if (w->way == 4) {
                    w->phase = 3.141592741f;
                } else {
                    w->phase = 0.0f;
                }
                w->e.state = 0;
            } else if (w->type == 2) {
                ((AnimeFrame *) w->e.model)->scale[0] = ((AnimeFrame *) w->e.model)->scale[1] =
                    ((AnimeFrame *) w->e.model)->scale[2] = 0.1199999973f;
                w->e.state = 2;
            }
            w->flame->scale[0] = w->flame->scale[1] = w->flame->scale[2] =
                ((AnimeFrame *) w->e.model)->scale[0] * 10.0f;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*lanternBob)(o);
                    break;
                case 1:
                    (*lanternRiseSway)(o);
                    break;
                case 2:
                    (*lanternWait)(o);
                    break;
                case 3:
                    (*lanternSwing)(o);
                    break;
                case 4:
                    (*lanternRise)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_lantern_main: 死亡\n");
                return -1;
            }
            w->flame->pos[0] = ((AnimeFrame *) w->e.model)->pos[0];
            w->flame->pos[1] = ((AnimeFrame *) w->e.model)->pos[1];
            w->flame->pos[2] = ((AnimeFrame *) w->e.model)->pos[2];
            if (w->e.hit & 0x10) {
                w->hurt = 12;
            }
            if (w->hurt - 1 > 0) {
                w->hurt = w->hurt - 1;
                w->e.dmg->flags &= ~2;
            } else {
                w->hurt = 0;
                w->e.dmg->flags |= 2;
            }
            if (w->flame != NULL) {
                if (w->hurt & 2) {
                    w->flame->dl2 = NULL;
                } else {
                    w->flame->dl2 = (Gfx *) etskLanternFlameDl;
                }
            }
            /* fallthrough */
        case 3:
            w->flame->dl = eneDlEnvPrimColor(o, 0xFF, 0xFF, 0x9F, 0xFF, 0xFF, 0, 0x5F);
            break;
        case 1:
            /* each test and its call on one line (IDO schedules by line; tlb_0084's form) */
            if (w->e.model != NULL) { frmDestroy(w->e.model); }
            if (w->flame != NULL) { frmDestroy(&w->flame->f); }
            if (w->e.atk != NULL) { frmDestroy(&w->e.atk->frame); }
            if (w->e.dmg != NULL) { frmDestroy(&w->e.dmg->frame); }
            if (w->e.shadow != NULL) { frmDestroy((Frame *) w->e.shadow); }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* State 0: bobs and sways at its place; by day it rises (1). */
void lanternBob(Obj *o) {
    Lantern *w = LANTERN(o);
    f32 speed = 1.0f;
    s32 hour;
    s32 pad;

    w->e.y = sinf(gGameFrameCount / 5.0f) * 3.0 + w->homeY;
    if (w->hurt == 0) {
        eneAnimLoop(o, (AnimeFrame *) w->e.model, speed);
        if (((AnimeFrame *) w->e.model)->frame >= 9.0 && ((AnimeFrame *) w->e.model)->frame < 9.0 + speed) {
            eneSetSd2(o, 0x2CB, 0, 0);
        }
        w->e.x = sinf(w->phase) * 40.0 + w->homeX;
        w->phase += 0.05;
        hour = gGameWork.time / 0x10000;
        if (hour < 19 && hour >= 5) {
            w->e.state = 1;
        }
    }
}

/* State 1: rises away, swaying. */
void lanternRiseSway(Obj *o) {
    Lantern *w = LANTERN(o);
    f32 speed = 1.0f;

    if (w->hurt == 0) {
        w->e.vel[1] += 0.2;
        if (w->e.vel[1] >= 2.0) {
            w->e.vel[1] = 2.0f;
        }
        if (w->e.y - w->homeY > 120.0f) {
            w->e.behave |= 0x40000;
        }
        w->e.rot[1] = eneCourseAngle(o, 8);
        eneAnimLoop(o, (AnimeFrame *) w->e.model, speed);
        if (((AnimeFrame *) w->e.model)->frame >= 9.0 && ((AnimeFrame *) w->e.model)->frame < 9.0 + speed) {
            eneSetSd2(o, 0x2CB, 0, 0);
        }
        w->e.x = sinf(w->phase) * 40.0 + w->homeX;
        w->phase += 0.05;
    }
}

/* State 2: turned aside from the nearest player, it waits at its place for one within 90; then
 * it swings at him (3). By day it rises (4). */
void lanternWait(Obj *o) {
    Lantern *w;
    s32 n;

    if (LANTERN(o)->hurt == 0) {
        w = LANTERN(o);
        w->way = eneNearestPlWay(o);
        if (w->way == 4) {
            if (eneTurnRotYTo(o, eneCourseAngle(o, 8) - 0.7853981853f, -0.3f) != 0) {
                return;
            }
        } else {
            if (eneTurnRotYTo(o, eneCourseAngle(o, 8) + 0.7853981853f, 0.3f) != 0) {
                return;
            }
        }
        w->e.x = w->homeX;
        w->e.y = w->homeY;
        if ((n = eneNearestPl(o)) != 0) {
            w->dist = enePlDist(o, n, 1);
            if (w->dist < 91) {
                w->dist = w->dist * 0.8;
                DPRINTF("after dist %d", w->dist);
                w->phase = -1.570796371f;
                w->e.state = 3;
            }
        }
        {
            f32 speed = 1.0f;
            s32 hour;

            eneAnimLoop(o, (AnimeFrame *) w->e.model, speed);
        if (((AnimeFrame *) w->e.model)->frame >= 9.0 && ((AnimeFrame *) w->e.model)->frame < 9.0 + speed) {
            eneSetSd2(o, 0x2CB, 0, 0);
        }
            hour = gGameWork.time / 0x10000;
            if (hour < 19 && hour >= 5) {
                w->e.state = 4;
            }
        }
        w->lastX = w->e.x;
    }
}

/* State 3: swings out at the player and back, tilting with its speed; then waits (2).
 * Matched 2026-10-12: the hurt test as an early return ("if (w->hurt != 0) return;")
 * instead of a block around the body; it frees ugen's t6 (o's reload) before the hurt byte's t7.
 * The difference x - lastX is a named local d (it takes fa0 and speed fa1, as the game's).
 * Pads: one word above speed and d below it give the frame and the sinf temp's slot. */
void lanternSwing(Obj *o) {
    Lantern *w = LANTERN(o);
    s32 pad;
    f32 speed;
    f32 d;

    DPRINTF("after dist %d", w->dist);
    if (w->hurt != 0) {
        return;
    }
    if (w->way == 4) {
        w->e.x = w->homeX - (sinf(w->phase) * w->dist / 2 + w->dist / 2);
    } else {
        w->e.x = sinf(w->phase) * w->dist / 2 + w->dist / 2 + w->homeX;
    }
    w->e.y = w->homeY - (sinf(w->phase) * 20.0f + 20.0f);
    if (w->phase >= -1.570796371f && w->phase < 1.570796371f) {
        w->phase += 0.12;
        speed = 2.0f;
        d = w->e.x - w->lastX;
        DPRINTF("vx %f\n", d);
        w->e.rot[0] = fabsf(d) / 4.3 * PI / 3;
    } else {
        w->phase += 0.1;
        w->e.y += fabsf(cosf(w->phase)) * 15.0f * w->dist / 90;
        speed = 1.0f;
        if (w->phase >= PI * 1.5) {
            w->e.state = 2;
        }
    }
    eneAnimLoop(o, (AnimeFrame *) w->e.model, speed);
    if (((AnimeFrame *) w->e.model)->frame >= 9.0 && ((AnimeFrame *) w->e.model)->frame < 9.0 + speed) {
        eneSetSd2(o, 0x2CB, 0, 0);
    }
    w->lastX = w->e.x;
}

/* State 4: rises away. */
void lanternRise(Obj *o) {
    Lantern *w = LANTERN(o);
    f32 speed = 1.0f;
    s32 pad;

    if (w->hurt == 0) {
        w->e.vel[1] += 0.2;
        if (w->e.vel[1] >= 2.0) {
            w->e.vel[1] = 2.0f;
        }
        if (w->e.y - w->homeY > 120.0f) {
            w->e.behave |= 0x40000;
        }
        eneAnimLoop(o, (AnimeFrame *) w->e.model, speed);
        if (((AnimeFrame *) w->e.model)->frame >= 9.0 && ((AnimeFrame *) w->e.model)->frame < 9.0 + speed) {
            eneSetSd2(o, 0x2CB, 0, 0);
        }
    }
}
