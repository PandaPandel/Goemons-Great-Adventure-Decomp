#include "tlb.h"

/* The "Rkappa" enemy kind (archive file 87): a kappa that waits until the camera is near, then
 * swims along x one way and the other, turning back after a few strokes. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 etskRkappaAnim[];
extern u8 etskRkappaHalf0Dl[];
extern u8 etskRkappaHalf1Dl[];

/* An Rkappa's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Rkappa {
    /* 0x00 */ EneObj e;
    /* 0x4C */ u8 pad4C[2];
    /* 0x4E */ u8 way; /* the way to the nearest player (eneNearestPlWay) */
    /* 0x4F */ u8 strokes; /* animation loops since it last turned */
} Rkappa;

#define RKAPPA(obj) ((Rkappa *) FX(obj))

void rkappaWait(Obj *o);
void rkappaSwim(Obj *o);
void rkappaDash(Obj *o);

s32 Etsk_Rkappa_main(Obj *o, s16 msg, void *arg) {
    Rkappa *w = RKAPPA(o);
    s32 pad;
    s32 type;

    switch (msg) {
        case 2:
            o->name = "Rkappa";
            type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewAnime(o, (u32) etskRkappaAnim)) == NULL) {
                return -1;
            }
            if ((w->e.shadow = (Obj *) shadowCreate()) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 45, 80, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 70, 160, 0, 0, 0)) == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                PRINTF("Etsk_Rkappa_j: 地面にセット失敗\n");
                return -1;
            }
            w->e.halves[0] = (u32) etskRkappaHalf0Dl;
            w->e.halves[1] = (u32) etskRkappaHalf1Dl;
            if (type == 1) {
                w->e.behave = 0xC00503;
                ((AnimeFrame *) w->e.model)->scale[2] = 0.15f;
                ((AnimeFrame *) w->e.model)->scale[0] = ((AnimeFrame *) w->e.model)->scale[1] =
                    ((AnimeFrame *) w->e.model)->scale[2];
                ((AnimeFrame *) w->e.model)->frame = 14.0f;
                w->way = eneNearestPlWay(o);
                w->e.rot[1] = eneCourseAngle(o, w->way);
                w->e.state = 0;
            }
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*rkappaWait)(o);
                    break;
                case 1:
                    (*rkappaSwim)(o);
                    break;
                case 2:
                    (*rkappaDash)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_Rkappa_main: 死亡\n");
                return -1;
            }
            if (w->e.hit & 0x10) {
                w->e.behave &= ~1;
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

void rkappaWait(Obj *o) {
    Rkappa *w = RKAPPA(o);

    if (eneCamNearCheck(o, 40.0f)) {
        if (w->way == 4) {
            w->e.vel[0] = -0.4f;
        } else if (w->way == 1) {
            w->e.vel[0] = 0.4f;
        }
        w->e.state = 1;
    }
}

void rkappaSwim(Obj *o) {
    Rkappa *w = RKAPPA(o);
    f32 speed = RKAPPA(o)->e.vel[0] / -1.5 / 1.1;

    PRINTF("vx %f, ani_speed %f\n", RKAPPA(o)->e.vel[0], speed);
    if (eneAnimLoop(o, (AnimeFrame *) RKAPPA(o)->e.model, speed)) {
        if (++w->strokes == 2) {
            w->strokes = 0;
            w->e.state = 2;
            w->e.vel[0] *= -5.0f;
        }
    }
}

void rkappaDash(Obj *o) {
    Rkappa *w = RKAPPA(o);
    f32 speed = RKAPPA(o)->e.vel[0] / -1.5 / 1.1;

    PRINTF("vx %f, ani_speed %f\n", RKAPPA(o)->e.vel[0], speed);
    if (eneAnimLoop(o, (AnimeFrame *) RKAPPA(o)->e.model, speed)) {
        if (++w->strokes == 3) {
            w->e.vel[0] *= -1.0f;
        } else if (w->strokes == 6) {
            w->strokes = 0;
            w->e.state = 1;
            w->e.vel[0] /= -5.0f;
        }
    }
}
