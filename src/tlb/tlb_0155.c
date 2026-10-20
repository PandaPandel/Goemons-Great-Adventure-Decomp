#include "tlb.h"
/* The "utsubo" enemy kind (archive file 155): a moray eel, built as tlb_0087's Rkappa. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
/* tlb.h has eneAfterProc's far-call macro without its declaration. */

extern u8 etskUtsuboAnim[]; /* its animation */

/* An utsubo's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Utsubo {
    /* 0x00 */ EneObj e;
    /* 0x4C */ f32 angle; /* of its rise and fall (radians, 0.05 a frame) */
    /* 0x50 */ f32 homeX; /* its x when made */
} Utsubo;

#define UTSUBO(obj) ((Utsubo *) FX(obj))

void utsuboSwim(Obj *o);
void utsuboNop1(Obj *o);
void utsuboNop2(Obj *o);

/* "utsubo": a quarter-size animated model with two attack boxes (kind 120), one above the
 * other along its body, facing the course's angle 8; it remembers its x. Its steps are by state:
 * utsuboSwim (0), then two that do nothing. */
s32 Etsk_utsubo_main(Obj *o, s16 msg, void *arg) {
    Utsubo *w = UTSUBO(o);
    s32 pad;
    ACollFrame *atk;

    switch (msg) {
        case 2:
            o->name = "utsubo";
            eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewAnime(o, (u32) etskUtsuboAnim)) == NULL) {
                return -1;
            }
            ((AnimeFrame *) w->e.model)->scale[2] = 0.25f;
            ((AnimeFrame *) w->e.model)->scale[0] = ((AnimeFrame *) w->e.model)->scale[1] =
                ((AnimeFrame *) w->e.model)->scale[2];
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 70, 10, -40, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((atk = ATK_ADD_YZ(&w->e.atk->frame, w->e.model, 70, 10, -85, 0)) == NULL) {
                return -1;
            }
            atk->kind = 120;
            w->e.behave = 0;
            w->e.rot[1] = eneCourseAngle(o, 8);
            w->e.state = 0;
            w->homeX = w->e.x;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*utsuboSwim)(o);
                    break;
                case 1:
                    (*utsuboNop1)(o);
                    break;
                case 2:
                    (*utsuboNop2)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_utsubo_main: 死亡\n");
                return -1;
            }
            break;
        case 1:
            if (w->e.model != NULL) {
                frmDestroy(w->e.model);
            }
            if (w->e.shadow != NULL) {
                frmDestroy((Frame *) w->e.shadow);
            }
            if (w->e.atk != NULL) {
                frmDestroy(&w->e.atk->frame);
            }
            if (w->e.dmg != NULL) {
                frmDestroy(&w->e.dmg->frame);
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Main's sinf and libultra's cosf, called through their addresses as TLB code calls main. */
#define sinf (*sinf)
#define cosf (*cosf)

/* Its swim: the animation runs 0.7 a frame over 12 frames, and its speed along x follows the
 * stroke (out and back in four quarters, 1.3 a frame at most) while the cosine of its angle is
 * up; when it is down it stays at its first x. It rises and falls 90 about -70 with the sine,
 * and grows with its height (a quarter size, plus 0.12 of its height over 160).
 * Matched 2026-10-11; it was parked for w's spill slot (the second pad, below c). */
void utsuboSwim(Obj *o) {
    Utsubo *w = UTSUBO(o);
    s32 pad;
    f32 frame;
    f32 speed;
    f32 c;
    s32 pad2;

    ((AnimeFrame *) w->e.model)->frame += 0.7f;
    if (((AnimeFrame *) w->e.model)->frame > 12.0f) {
        ((AnimeFrame *) w->e.model)->frame -= 12.0f;
    }
    frame = ((AnimeFrame *) w->e.model)->frame; if (frame > 0.0f && frame <= 3.0f) {
        speed = (3.0 - frame) * 1.3;
    } else if (frame > 3.0f && frame <= 6.0f) {
        speed = (frame - 3.0) * -1.3;
    } else if (frame > 6.0f && frame <= 9.0f) {
        speed = (9.0 - frame) * -1.3;
    } else if (frame > 9.0f && frame <= 12.0f) {
        speed = (frame - 9.0) * 1.3;
    }
    c = cosf(w->angle);
    if (c > 0.0f) {
        w->e.vel[0] = c * speed;
    } else {
        w->e.vel[0] = 0.0f;
        w->e.x = w->homeX;
    }
    w->e.z = sinf(w->angle) * 90.0f + -70.0f;
    w->angle += 0.05;
    ((AnimeFrame *) w->e.model)->scale[2] = (w->e.z / 160.0f) * 0.12 + 0.25;
    ((AnimeFrame *) w->e.model)->scale[0] = ((AnimeFrame *) w->e.model)->scale[1] =
        ((AnimeFrame *) w->e.model)->scale[2];
}

void utsuboNop1(Obj *o) {
}

void utsuboNop2(Obj *o) {
}
