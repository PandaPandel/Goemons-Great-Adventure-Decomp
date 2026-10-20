#include "tlb.h"

/* The "improck" enemy kind (archive file 150): a rock (radius 12) that rolls along the course
 * and can be ridden, and "jenel", its generator, which puts out an improck (kind 0x204). */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 etskImprockDl[];
extern u8 etskImprockRide[];

/* A jenel's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Jenel {
    /* 0x00 */ EneObj e;
    /* 0x4C */ s16 type; /* its placement's type, from eneSetup, passed on to each improck */
    /* 0x4E */ s16 wait; /* steps before the next improck */
} Jenel;

/* An improck's work: an enemy's and its own. */
typedef struct Improck {
    /* 0x00 */ EneObj e;
    /* 0x4C */ PolFrame *pol; /* a frame that follows the model, the ride's and damage box's */
    /* 0x50 */ RideFrame *ride;
    /* 0x54 */ f32 spin; /* state 5: its turn per step */
    /* 0x58 */ s16 count; /* state 5: steps at full spin */
    /* 0x5A */ s16 alpha;
} Improck;

#define JENEL(obj) ((Jenel *) FX(obj))
#define IMPROCK(obj) ((Improck *) FX(obj))
#define POL(w) ((PolFrame *) (w)->e.model)

s32 improckJProc(Obj *o, s16 msg, void *arg) {
    Jenel *w = JENEL(o);
    s32 pad;
    s32 hour;

    switch (msg) {
        case 2:
            o->name = "jenel";
            w->type = eneSetup(o, arg);
            w->wait = 0;
            if (w->type == 1) {
                if (eneSetOnEarth(o) == -1) {
                    return -1;
                }
                w->e.behave = 0x800;
            } else if (w->type == 2) {
                w->e.behave = 0;
            } else if (w->type == 3) {
                w->e.behave = 0;
            } else if (w->type == 10) {
                w->wait = fadeRand(10);
                w->e.behave = 0;
            }
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            if (w->type != 10) {
                if (w->wait-- <= 0 && eneNearestPlInX(o, 85.0f) != 0) {
                    w->wait = 90;
                    eneMakeEx(NULL, ENE_ETSK_IMPROCK_MAIN, w->e.zone, w->e.x, w->e.y, w->e.z, 2,
                              (u32 (*)(void *, void *, void *)) (s32) w->type);
                }
            } else if (w->wait-- <= 0) {
                hour = gGameWork.time / 0x10000;
                if (hour < 19 && hour >= 5) {
                    w->wait = fadeRand(10) + 70;
                } else {
                    w->wait = fadeRand(10) + 50;
                }
                eneMakeEx(NULL, ENE_ETSK_IMPROCK_MAIN, w->e.zone, w->e.x, w->e.y, w->e.z, 2, (u32 (*)(void *, void *, void *)) 2);
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_improck_j: 死亡\n");
                return -1;
            }
            break;
        case 1:
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void improckWait(Obj *o);
void improckFadeIn(Obj *o);
void improckFall(Obj *o);
void improckBounce(Obj *o);
void improckWaitHit(Obj *o);
void improckSpin(Obj *o);
void improckRollOff(Obj *o);

/* The ride is created and then tested: assigned inside the test it costs the frame 8 bytes. */
s32 Etsk_improck_main(Obj *o, s16 msg, void *arg) {
    Improck *w = IMPROCK(o);
    s32 pad;
    s32 type;

    switch (msg) {
        case 2:
            o->name = "improck";
            type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) enePolModelAlpha(o, (Gfx *) etskImprockDl, 0)) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) uda_atk_root_frame, w->e.model, 100, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 125;
            if ((w->pol = eneNewPol(o, NULL)) == NULL) {
                return -1;
            }
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) uda_dmg_root_frame, &w->pol->f, 125, 250, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->e.dmg->flags &= ~8;
            w->ride = rideCreate(gRideTree1, (s32) o, &w->pol->f, (RideData *) eneFileAddr(o, etskImprockRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            if ((w->e.shadow = (Obj *) shadowCreate()) == NULL) {
                return -1;
            }
            w->e.rot[1] = eneCourseAngle(o, 8);
            w->pol->rot[1] = eneCourseAngle(o, 4);
            if (type == 1) {
                w->e.z -= 90.0f;
                w->e.behave = 0x801;
                w->alpha = 0;
                w->e.state = 0;
            } else if (type == 2) {
                w->e.behave = 0x802001;
                w->alpha = 0xFF;
                w->e.state = 2;
            } else if (type == 3) {
                w->e.atk->flags &= ~2;
                w->e.rot[1] = eneCourseAngle(o, 4);
                w->pol->rot[1] = eneCourseAngle(o, 8);
                w->e.behave = 1;
                w->e.dmg->radius = 220;
                w->alpha = 0xFF;
                w->e.state = 4;
            }
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*improckWait)(o);
                    break;
                case 1:
                    (*improckFadeIn)(o);
                    break;
                case 2:
                    (*improckFall)(o);
                    break;
                case 3:
                    (*improckBounce)(o);
                    break;
                case 4:
                    (*improckWaitHit)(o);
                    break;
                case 5:
                    (*improckSpin)(o);
                    break;
                case 6:
                    (*improckRollOff)(o);
                    break;
            }
            if (w->alpha >= 0xFF) {
                frmMove(w->e.model, gFrmModelLayer);
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_improck_main: 死亡\n");
            }
            w->pol->pos[0] = POL(w)->pos[0];
            w->pol->pos[1] = POL(w)->pos[1];
            w->pol->pos[2] = POL(w)->pos[2];
            POL(w)->pos[1] = w->e.y + 12.0;
            /* fallthrough */
        case 3:
            POL(w)->dl = gGfxDlHead;
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            if (w->e.model != NULL) {
                frmDestroy(w->e.model);
            }
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
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
            if (w->ride != NULL) {
                frmDestroy(&w->ride->f);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* State 0: it waits for a player within 70 along x, then rolls in (state 1). */
void improckWait(Obj *o) {
    Improck *w = IMPROCK(o);

    if (eneNearestPlInX(o, 70.0f) != 0) {
        w->e.vel[2] = 3.0f;
        w->e.state = 1;
        w->e.rot[0] = w->e.z / 12.0;
    }
}

/* State 1: it rolls in along z, fading in, rumbling. */
void improckFadeIn(Obj *o) {
    Improck *w = IMPROCK(o);
    s32 pad;

    w->alpha += 0x10;
    if (w->alpha >= 0x100) {
        w->alpha = 0xFF;
        frmMove(w->e.model, gFrmModelLayer);
    }
    if (gGameFrameCount % 12 == 0) {
        eneSetSd2(o, 0x2B0, 0, 2);
    }
    w->e.rot[0] = w->e.z / 12.0;
}

/* State 2: it falls in along z; on the ground it bounces (state 3). */
void improckFall(Obj *o) {
    Improck *w = IMPROCK(o);

    w->e.vel[2] = 2.0f;
    if (w->e.hit & 1) {
        eneSetSd2(o, 0x23E, 0, 2);
        w->e.vel[1] *= -0.3;
        w->e.state = 3;
    }
}

/* State 3: it bounces, rumbling. */
void improckBounce(Obj *o) {
    Improck *w = IMPROCK(o);
    s32 pad;

    if (w->e.hit & 1) {
        w->e.vel[1] *= -0.3;
    }
    if (gGameFrameCount % 12 == 0) {
        eneSetSd2(o, 0x2B0, 0, 2);
    }
    w->e.rot[0] = w->e.z / 12.0;
}

/* State 4: it rolls along x; hit, its damage box shrinks; on the ground it spins up (state 5). */
void improckWaitHit(Obj *o) {
    Improck *w = IMPROCK(o);
    s32 pad;

    if (w->e.hit & 0x10) {
        w->e.behave |= 0x800000;
        w->e.dmg->radius = 125;
    }
    if (w->e.hit & 1) {
        w->e.atk->flags |= 2;
        eneSetSd2(o, 0x23E, 0, 2);
        w->count = 0;
        w->e.state = 5;
        w->spin = 0.0f;
    }
    w->e.rot[0] -= w->e.vel[0] / 12.0;
}

/* State 5: it spins up in place, puffing dust; after 15 steps at full spin it rolls off
 * (state 6). */
void improckSpin(Obj *o) {
    Improck *w = IMPROCK(o);
    s32 pad;
    s32 r;
    f32 x;
    f32 z;

    if (gGameFrameCount % 12 == 0) {
        eneSetSd2(o, 0x2B0, 0, 2);
    }
    w->spin += 0.02;
    if (w->spin >= 0.7853981853f) {
        w->spin = 1.570796371f;
        w->count++;
        if (w->count >= 15) {
            w->e.state = 6;
            w->e.vel[0] = 3.5f;
        }
    }
    if (gGameFrameCount % 10 == 0) {
        r = fadeRand(20);
        vramCoursePointXZ(w->e.zone, w->e.x - r, w->e.z, &x, &z);
        itexPuffWhite(x, w->e.y, z, r / 200.0f);
    }
    w->e.rot[0] -= w->spin;
}

/* State 6: it rolls on, rumbling. */
void improckRollOff(Obj *o) {
    Improck *w = IMPROCK(o);

    if (gGameFrameCount % 12 == 0) {
        eneSetSd2(o, 0x2B0, 0, 2);
    }
    w->e.rot[0] -= w->e.vel[0] / 12.0;
}
