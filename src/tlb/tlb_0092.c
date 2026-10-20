#include "tlb.h"

/* The "hamag" enemy kind (archive file 92), a clam, and "hamag_j", its generator: a hamag_j puts
 * out a clam (kind 0xD1) every 150 steps until one of its clams is killed by a hit. A clam jumps
 * up out of the water, tips over as it falls and splashes back in. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 etskHamagAnim[];
extern u8 etskHamagHalf0Dl[];
extern u8 etskHamagHalf1Dl[];

/* A hamag_j's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct HamagJ {
    /* 0x00 */ EneObj e;
    /* 0x4C */ Obj *clam; /* the last clam it put out */
    /* 0x50 */ u16 time; /* steps since it started */
    /* 0x52 */ u8 on; /* 0 once one of its clams is killed by a hit */
    /* 0x53 */ u8 type; /* its placement's type, passed on to each clam */
} HamagJ;

/* A clam's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Hamag {
    /* 0x00 */ EneObj e;
    /* 0x4C */ f32 baseY; /* the height it jumps from (the water) */
    /* 0x50 */ u8 type; /* its placement's type, from eneSetup (2: it puffs smoke) */
    /* 0x51 */ u8 splashed; /* its splash is made */
} Hamag;

#define HAMAGJ(obj) ((HamagJ *) FX(obj))
#define HAMAG(obj) ((Hamag *) FX(obj))
#define MODEL(w) ((AnimeFrame *) (w)->e.model)

void hamagActJump(Obj *o);

s32 hamagJProc(Obj *o, s16 msg, void *arg) {
    HamagJ *w = HAMAGJ(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "hamag_j";
            w->type = eneSetup(o, arg);
            w->time = 0;
            w->on = 1;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            if (w->time % 150 == 0) {
                if (w->on) {
                    w->clam = eneMakeEx(o, ENE_ETSK_HAMAG_MAIN, w->e.zone, w->e.x, w->e.y, w->e.z, 2,
                                        (u32 (*)(void *, void *, void *)) w->type);
                }
            }
            w->time++;
            break;
        case 1:
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

s32 Etsk_hamag_main(Obj *o, s16 msg, void *arg) {
    Hamag *w = HAMAG(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "hamag";
            w->type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewAnime(o, (u32) etskHamagAnim)) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 50, 50, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 100, 140, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->e.halves[0] = (u32) etskHamagHalf0Dl;
            w->e.halves[1] = (u32) etskHamagHalf1Dl;
            w->e.behave = 0x1800;
            w->e.rot[1] = eneCourseAngle(o, 8);
            w->splashed = 0;
            w->e.state = 0;
            w->e.vel[1] = 9.0f;
            w->baseY = w->e.y;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            if (w->e.state == 0) {
                (*hamagActJump)(o);
            }
            if (eneAfterProc(o) == -1) {
                if (w->e.hit & 0x10) {
                    HAMAGJ(o->parent)->on = 0;
                }
                PRINTF("Etsk_hamag_main: 死亡\n");
                return -1;
            }
            break;
        case 1:
            if (w->e.model != NULL) {
                frmDestroy(w->e.model);
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

void hamagActJump(Obj *o) {
    Hamag *w = HAMAG(o);
    s32 pad;
    f32 step;
    Vec3f pos;
    Vec3f push;

    step = 0.0f;
    if (w->e.vel[1] >= -1.0f) {
        if (w->e.rot[1] < 1.570796371f) {
            w->e.rot[1] += 0.075;
        }
    } else {
        w->e.rot[1] -= 0.05;
    }
    if (w->e.vel[1] <= 4.0) {
        step = 1.0f;
    }
    eneAnimPlayOnce(o, MODEL(w), step);
    if (MODEL(w)->frame >= 10.0f && MODEL(w)->frame < 10.0f + step) {
        eneSetSd(o, 0x241);
    }
    if (w->type == 2 && !(gGameFrameCount & 1)) {
        itexPuffGrey(MODEL(w)->pos[0], MODEL(w)->pos[1], MODEL(w)->pos[2], 0.05f);
    }
    if (w->e.y < w->baseY) {
        w->e.behave |= 0x40000;
    }
    if (w->e.vel[1] < 0.0f && !w->splashed) {
        pos.x = MODEL(w)->pos[0];
        pos.y = MODEL(w)->pos[1];
        pos.z = MODEL(w)->pos[2];
        if (rideTree2PointIn(&pos, &push)) {
            eneSetSd(o, 0x298);
            effectSibukiCreateWhite(pos.x, pos.y, pos.z, 0.07500000298f, 0.07500000298f, 0.007499999832f,
                                    0.007499999832f);
            w->splashed = 1;
        }
    }
}
