#include "tlb.h"

/* The "kt_hani" enemy kind (archive file 82): a haniwa that walks along the course, waits for a
 * player, jumps, is squashed flat on landing and springs back. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

/* libultra's sinf (PR/gu.h declares it), called from the TLB through its address. */
#define sinf (*sinf)

extern u8 etskKtHaniAnim[];
extern u8 etskKtHaniState11Anim[];
extern u8 etskKtHaniHalf0Dl[];
extern u8 etskKtHaniHalf1Dl[];

/* A kt_hani's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct KtHani {
    /* 0x00 */ EneObj e;
    /* 0x4C */ f32 speed; /* its animation's step while it rolls (state 7) */
    /* 0x50 */ s16 homeX; /* where it bobs about (state 11) */
    /* 0x52 */ u8 type; /* its placement's type, from eneSetup */
    /* 0x53 */ u8 started; /* state 10: the camera came near */
    /* 0x54 */ u8 count;
    /* 0x55 */ u8 way; /* the nearest player's way (eneNearestPlWay) */
} KtHani;

#define KT(obj) ((KtHani *) FX(obj))

void ktHaniWait(Obj *o);
void ktHaniWalkIn(Obj *o);
void ktHaniWalk(Obj *o);
void ktHaniWaitJump(Obj *o);
void ktHaniSquash(Obj *o);
void ktHaniUnsquash(Obj *o);
void ktHaniRun(Obj *o);
void ktHaniPace(Obj *o);
void ktHaniLeap(Obj *o);
void ktHaniRunOff(Obj *o);
void ktHaniTopple(Obj *o);
void ktHaniSway(Obj *o);

s32 Etsk_kt_hani_main(Obj *o, s16 msg, void *arg) {
    KtHani *w = KT(o);
    ACollFrame *box;
    f32 r;
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "kt_hani";
            w->type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewAnime(o, (u32) etskKtHaniAnim)) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 42, 80, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((box = ATK_ADD_YZ(&w->e.atk->frame, w->e.model, 35, 100, 120, 0)) == NULL) {
                return -1;
            }
            box->kind = 120;
            ((ACollFrame *) w->e.atk->frame.child)->flags &= ~2;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 70, 160, 0, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->e.shadow = (Obj *) shadowCreate()) == NULL) {
                return -1;
            }
            w->e.halves[0] = (u32) etskKtHaniHalf0Dl;
            w->e.halves[1] = (u32) etskKtHaniHalf1Dl;
            w->count = 0;
            if (w->type == 1 || w->type == 5) {
                if (eneSetOnEarth(o) == -1) {
                    return -1;
                }
                w->e.behave = 0x404D00;
                if (w->e.z < 0.0f) {
                    w->e.rot[1] = eneCourseAngle(o, 8);
                    w->e.state = 0;
                } else if (w->e.z > 0.0f) {
                    w->e.rot[1] = eneCourseAngle(o, 2);
                    w->e.state = 0;
                } else {
                    w->e.rot[1] = eneCourseAngle(o, eneNearestPlWay(o));
                    w->e.state = 1;
                }
            } else if (w->type == 2) {
                w->e.behave = 0x400000;
                if (w->e.z < 0.0f) {
                    w->e.rot[1] = eneCourseAngle(o, 8);
                } else if (w->e.z > 0.0f) {
                    w->e.rot[1] = eneCourseAngle(o, 2);
                } else {
                    w->way = eneNearestPlWay(o);
                    if (w->way == 4) {
                        w->e.rot[1] = eneCourseAngle(o, 8) - 0.7853981853f;
                    } else if (w->way == 1) {
                        w->e.rot[1] = eneCourseAngle(o, 8) + 0.7853981853f;
                    }
                }
                w->e.state = 3;
            } else if (w->type == 3) {
                w->e.behave = 0x400000;
                w->speed = 0.5f;
                w->way = eneNearestPlWay(o);
                w->e.rot[1] = eneCourseAngle(o, w->way);
                if (w->way == 4) {
                    w->e.vel[0] = -0.5f;
                } else if (w->way == 1) {
                    w->e.vel[0] = 0.5f;
                }
                w->e.state = 7;
            } else if (w->type == 4) {
                w->e.behave = 0x400000;
                w->e.rot[1] = eneCourseAngle(o, 4);
                w->e.rot[0] = -0.7853981853f;
                w->started = 0;
                w->e.state = 10;
            } else if (w->type == 6) {
                w->e.behave = 0x420000;
                w->homeX = w->e.x;
                w->e.rot[1] = eneCourseAngle(o, 4);
                w->e.state = 11;
            } else if (w->type == 7) {
                w->e.behave = 0x400800;
                w->e.rot[1] = eneCourseAngle(o, eneNearestPlWay(o));
                w->e.rot[0] = -1.570796371f;
                w->started = 1;
                w->e.state = 10;
            }
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*ktHaniWait)(o);
                    break;
                case 1:
                    (*ktHaniWalkIn)(o);
                    break;
                case 2:
                    (*ktHaniWalk)(o);
                    break;
                case 3:
                    (*ktHaniWaitJump)(o);
                    break;
                case 4:
                    (*ktHaniSquash)(o);
                    break;
                case 5:
                    (*ktHaniUnsquash)(o);
                    break;
                case 6:
                    (*ktHaniRun)(o);
                    break;
                case 7:
                    (*ktHaniPace)(o);
                    break;
                case 8:
                    (*ktHaniLeap)(o);
                    break;
                case 9:
                    (*ktHaniRunOff)(o);
                    break;
                case 10:
                    (*ktHaniTopple)(o);
                    break;
                case 11:
                    (*ktHaniSway)(o);
                    break;
            }
            if (((AnimeFrame *) w->e.model)->anim == etskKtHaniState11Anim) {
                if ((((AnimeFrame *) w->e.model)->frame >= 3.0 && ((AnimeFrame *) w->e.model)->frame < 7.0)
                    || (((AnimeFrame *) w->e.model)->frame >= 13.0 && ((AnimeFrame *) w->e.model)->frame < 17.0)) {
                    ((ACollFrame *) w->e.atk->frame.child)->flags |= 2;
                } else {
                    ((ACollFrame *) w->e.atk->frame.child)->flags &= ~2;
                }
            }
            if (eneAfterProc(o) == -1) {
                /* "死亡" in Shift-JIS here, where the build writes EUC-JP: as bytes. */
                PRINTF("Etsk_kt_hani_main: \x8E\x80\x96\x53\n");
                return -1;
            }
            r = (w->e.rot[0] > 0.0f) ? w->e.rot[0] : -w->e.rot[0];
            ((AnimeFrame *) w->e.model)->pos[1] = r * 5.0f / 1.570796371f + w->e.y;
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

void ktHaniWait(Obj *o) {
    KtHani *w = KT(o);

    eneAnimLoop(o, (AnimeFrame *) w->e.model, 1.0f);
    if (eneNearestPlInX(o, 70.0f)) {
        w->e.state = 1;
    }
}

/* The speeds are a ternary of doubles: as if/else, IDO folds each into a float load. */
void ktHaniWalkIn(Obj *o) {
    KtHani *w = KT(o);
    s32 pad[2];
    f32 speed;

    speed = (w->type == 1) ? 1.6 : 1.2;
    eneAnimLoop(o, (AnimeFrame *) w->e.model, speed * 1.1);
    if (w->e.z < 0.0f) {
        w->e.vel[2] = speed;
        if (w->e.z + speed >= 0.0f) {
            w->e.vel[2] = 0.0f;
            w->e.z = 0.0f;
        }
    } else if (w->e.z > 0.0f) {
        w->e.vel[2] = -speed;
        if (w->e.z <= -speed) {
            w->e.vel[2] = 0.0f;
            w->e.z = 0.0f;
        }
    } else {
        if (eneNearestPlWay(o) == 1) {
            w->e.vel[0] = speed;
        } else if (eneNearestPlWay(o) == 4) {
            w->e.vel[0] = -speed;
        }
        w->e.state = 2;
    }
}

void ktHaniWalk(Obj *o) {
    KtHani *w = KT(o);
    f32 speed;
    s32 pad[2];

    speed = (w->type == 1) ? 1.6 : 1.2;
    if (eneAnimLoop(o, (AnimeFrame *) w->e.model, speed * 1.1)) {
        if (w->type == 5) {
            w->count++;
            if (w->count >= 4) {
                w->count = 0;
            }
            if (w->count == 1) {
                ((AnimeFrame *) w->e.model)->anim = etskKtHaniState11Anim;
            } else {
                ((AnimeFrame *) w->e.model)->anim = etskKtHaniAnim;
            }
        }
    }
}

void ktHaniWaitJump(Obj *o) {
    KtHani *w = KT(o);

    eneAnimLoop(o, (AnimeFrame *) w->e.model, 2.0f);
    if (eneNearestPlInX(o, 90.0f)) {
        w->e.behave |= 0x800000;
        w->e.vel[1] = 3.0f;
        if (w->e.z < 0.0f) {
            w->e.vel[2] = 1.0f;
        } else if (w->e.z > 0.0f) {
            w->e.vel[2] = -1.0f;
        } else if (w->way == 4) {
            w->e.vel[0] = -1.0f;
        } else if (w->way == 1) {
            w->e.vel[0] = 1.0f;
        }
        w->e.state = 4;
    }
}

void ktHaniSquash(Obj *o) {
    KtHani *w = KT(o);

    if (w->e.hit & 1) {
        w->e.vel[0] = 0.0f;
        w->e.vel[2] = 0.0f;
        ((AnimeFrame *) w->e.model)->scale[1] -= 0.01;
        if (((AnimeFrame *) w->e.model)->scale[1] <= 0.07) {
            ((AnimeFrame *) w->e.model)->scale[1] = 0.07f;
            w->e.behave |= 0x4500;
            w->e.state = 5;
        }
        ((AnimeFrame *) w->e.model)->scale[0] = (0.1 - ((AnimeFrame *) w->e.model)->scale[1]) / 2 + 0.1;
        ((AnimeFrame *) w->e.model)->scale[2] = (0.1 - ((AnimeFrame *) w->e.model)->scale[1]) / 2 + 0.1;
    } else {
        eneAnimLoop(o, (AnimeFrame *) w->e.model, 2.0f);
    }
}

/* The sums take the literal (z + 1.6f): through vel[2], an array element, IDO sorts the add's
 * operands. */
void ktHaniUnsquash(Obj *o) {
    KtHani *w = KT(o);
    s32 pad[2];

    if (((AnimeFrame *) w->e.model)->scale[1] < 0.1) {
        ((AnimeFrame *) w->e.model)->scale[1] += 0.01;
        if (((AnimeFrame *) w->e.model)->scale[1] >= 0.1) {
            ((AnimeFrame *) w->e.model)->scale[1] = 0.1f;
        }
        ((AnimeFrame *) w->e.model)->scale[0] = (0.1 - ((AnimeFrame *) w->e.model)->scale[1]) / 2 + 0.1;
        ((AnimeFrame *) w->e.model)->scale[2] = (0.1 - ((AnimeFrame *) w->e.model)->scale[1]) / 2 + 0.1;
    } else {
        eneAnimLoop(o, (AnimeFrame *) w->e.model, 2.0f);
        if (w->e.z < 0.0f) {
            w->e.vel[2] = 1.6f;
            if (w->e.z + 1.6f >= 0.0f) {
                w->e.vel[2] = 0.0f;
                w->e.z = 0.0f;
            }
        } else if (w->e.z > 0.0f) {
            w->e.vel[2] = -1.6f;
            if (w->e.z + -1.6f <= 0.0f) {
                w->e.vel[2] = 0.0f;
                w->e.z = 0.0f;
            }
        } else {
            if (eneNearestPlWay(o) == 1) {
                w->e.vel[0] = 1.6f;
            } else if (eneNearestPlWay(o) == 4) {
                w->e.vel[0] = -1.6f;
            }
            w->e.state = 6;
        }
    }
}

void ktHaniRun(Obj *o) {
    KtHani *w = KT(o);

    eneAnimLoop(o, (AnimeFrame *) w->e.model, 2.0f);
}

/* The velocities are set before the behaviour and state: the game loads their floats first. */
void ktHaniPace(Obj *o) {
    KtHani *w = KT(o);
    s32 pad;

    if (eneAnimLoop(o, (AnimeFrame *) w->e.model, w->speed)) {
        w->count++;
    }
    if (w->count >= 3) {
        w->count = 0;
        w->speed *= -1.0f;
        w->e.vel[0] *= -1.0f;
    }
    if (eneNearestPlWay(o) != w->way) {
        if (!eneNearestPlInX(o, 20.0f)) {
            w->e.vel[1] = 4.0f;
            w->e.vel[2] = -2.0f;
            w->e.vel[0] = 0.0f;
            w->e.behave |= 0x804400;
            w->e.state = 8;
        }
    }
}

/* w set at its declaration stays one register from the join on (assigned at the join, IDO
 * folds it); vel[0] before state keeps the 9's li in its block. */
void ktHaniLeap(Obj *o) {
    KtHani *w = KT(o);
    s32 pad;

    if (eneFaceWay(o) == 4) {
        eneTurnRotYTo(o, eneCourseAngle(o, 8), 0.1f);
    } else if (eneFaceWay(o) == 1) {
        eneTurnRotYTo(o, eneCourseAngle(o, 8), -0.1f);
    }
    if (w->e.z <= 0.0f) {
        w->e.z = 0.0f;
        w->e.vel[2] = 0.0f;
    }
    if (w->e.hit & 1) {
        w->e.vel[0] = 1.6f;
        w->e.state = 9;
    }
}

void ktHaniRunOff(Obj *o) {
    KtHani *w = KT(o);

    eneAnimLoop(o, (AnimeFrame *) w->e.model, 2.0f);
}

void ktHaniTopple(Obj *o) {
    KtHani *w = KT(o);
    s32 pad[2];

    if (w->started == 0) {
        if (eneCamNearCheck(o, 50.0f)) {
            w->started = 1;
            w->e.behave |= 0x800000;
        }
    } else {
        eneAnimLoop(o, (AnimeFrame *) w->e.model, 1.5f);
        if (w->e.hit & 1) {
            w->e.rot[0] -= 0.5;
            if (w->e.rot[0] <= -1.570796371f) {
                w->e.rot[0] = -1.570796371f;
            }
        }
    }
}

void ktHaniSway(Obj *o) {
    KtHani *w = KT(o);

    eneAnimLoop(o, (AnimeFrame *) w->e.model, 1.25f);
    w->e.x = sinf(gGameFrameCount / 20.0f) * 20.0f / 2 + w->homeX;
}
