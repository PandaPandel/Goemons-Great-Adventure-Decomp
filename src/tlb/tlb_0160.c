#include "tlb.h"

/* The "neckbite" enemy kind (archive file 160): a head on a neck that sways along x about where
 * it was placed (a sine of its swing, 22.5 either side), leaning with its speed; when the swing
 * is past a quarter turn either way it opens up and spits a "tama" (a ball, kind 0x218, this
 * file's second task) towards the way it faces. The ball flies at 3 along x and dies on a wall,
 * the ground or any hit but kinds 10, 13 and 14. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

/* libultra's sinf (PR/gu.h declares it), called from the TLB through its address. */
#define sinf (*sinf)

#pragma intrinsic(fabs)

extern u8 etskNeckbiteAnim[];
extern u8 etskNeckbiteSwayAnim[];
extern u8 etskNeckbiteHalf0Dl[];
extern u8 etskNeckbiteHalf1Dl[];
extern u8 neckbiteTamaDl[];

/* A neckbite's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Neckbite {
    /* 0x00 */ EneObj e;
    /* 0x4C */ f32 swing; /* the sway's phase, -pi to pi */
    /* 0x50 */ f32 swingStep; /* the phase's step: 0.06 in the middle quarter turns, 0.09 out */
    /* 0x54 */ f32 prevX; /* x last step */
    /* 0x58 */ s16 homeX; /* where it was placed */
    /* 0x5A */ u8 unused5A;
    /* 0x5B */ u8 way; /* the way it faces along x: 1 or 4 (eneNearestPlWay) */
} Neckbite;

#define NECK(obj) ((Neckbite *) FX(obj))
#define MODEL(w) ((AnimeFrame *) (w)->e.model)

/* A frame destroyed if it was made: one source line (as1 schedules the first one's delay slot
 * by it, as in tlb_0159). */
#define FRM_DESTROY(f) if ((f) != NULL) { frmDestroy((Frame *) (f)); }

/* eneMakeEx's last argument carries the new enemy's placement type (eneSetup returns it). Through
 * an s32: a u8 cast straight to the pointer made IDO copy the way into a0 for the else-if. */
#define TYPE_ARG(t) ((u32 (*)(void *, void *, void *)) (s32) (t))

void neckbiteSway(Obj *o);
void neckbiteWaitLoop(Obj *o);
void neckbiteOpen(Obj *o);
void neckbiteSpit(Obj *o);
void neckbiteStop(Obj *o);
void neckbiteStopped(Obj *o);

s32 Etsk_neckbite_main(Obj *o, s16 msg, void *arg) {
    Neckbite *w = NECK(o);
    s32 pad[2];
    f32 dx;
    f32 lean;

    switch (msg) {
        case 2:
            o->name = "neckbite";
            eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewAnime(o, (u32) etskNeckbiteAnim)) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 45, 80, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 80, 240, 30, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->e.shadow = (Obj *) shadowCreate()) == NULL) {
                return -1;
            }
            w->e.halves[0] = (u32) etskNeckbiteHalf0Dl;
            w->e.halves[1] = (u32) etskNeckbiteHalf1Dl;
            w->e.behave = 0x400000;
            w->way = eneNearestPlWay(o);
            w->swing = 0.0f;
            w->homeX = w->e.x;
            w->prevX = w->homeX;
            w->e.rot[1] = eneCourseAngle(o, w->way);
            w->e.state = 0;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            /* Here, not after the way's store in case 2: there it kept w live across
             * eneHitValidCheck (a spill the game does not have). */
            PRINTF("atk_dir %x\n", w->way);
            if (w->way == 4) {
                w->e.x = w->homeX - sinf(w->swing) * 22.5;
            } else if (w->way == 1) {
                w->e.x = sinf(w->swing) * 22.5 + w->homeX;
            }
            dx = w->e.x - w->prevX;
            if (-1.570796371f <= w->swing && w->swing < 1.570796371f) {
                w->swingStep = 0.06f;
                w->e.rot[0] = fabs(dx) * 0.5 * 3.14159274101257324 / 8;
                lean = 0.0f;
            } else {
                w->swingStep = 0.09f;
                lean = fabs(dx) * 0.5 * 3.14159274101257324 / 4;
                w->e.rot[0] = 0.0f;
            }
            w->swing += w->swingStep;
            if (3.141592741f < w->swing) {
                w->swing -= 6.283185482f;
            }
            if (w->way == 4) {
                w->e.rot[1] = eneCourseAngle(o, 4) + lean;
            } else if (w->way == 1) {
                w->e.rot[1] = eneCourseAngle(o, 1) - lean;
            }
            w->prevX = w->e.x;
            switch (w->e.state) {
                case 0:
                    (*neckbiteSway)(o);
                    break;
                case 1:
                    (*neckbiteWaitLoop)(o);
                    break;
                case 2:
                    (*neckbiteOpen)(o);
                    break;
                case 3:
                    (*neckbiteSpit)(o);
                    break;
                case 4:
                    (*neckbiteStop)(o);
                    break;
                case 5:
                    (*neckbiteStopped)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_neckbite_main: 死亡\n");
                return -1;
            }
            break;
        case 1:
            FRM_DESTROY(w->e.model);
            FRM_DESTROY(w->e.atk);
            FRM_DESTROY(w->e.dmg);
            FRM_DESTROY(w->e.shadow);
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* State 0: sways; when a loop of its animation ends past a quarter turn either way, it opens
 * up (3). */
void neckbiteSway(Obj *o) {
    Neckbite *w = NECK(o);

    if (eneAnimeLoop(o, MODEL(w), 0.6f)) {
        if (w->swing < -1.570796371f || w->swing >= 1.570796371f) {
            MODEL(w)->anim = etskNeckbiteSwayAnim;
            MODEL(w)->frame = 0.0f;
            w->e.state = 3;
        }
    }
}

/* State 1: to state 2 at the end of a loop. */
void neckbiteWaitLoop(Obj *o) {
    Neckbite *w = NECK(o);

    if (eneAnimeLoop(o, MODEL(w), 0.6f)) {
        w->e.state = 2;
    }
}

/* State 2: opens up (3) at the end of a loop. */
void neckbiteOpen(Obj *o) {
    Neckbite *w = NECK(o);

    if (eneAnimeLoop(o, MODEL(w), 0.6f)) {
        MODEL(w)->anim = etskNeckbiteSwayAnim;
        MODEL(w)->frame = 0.0f;
        w->e.state = 3;
    }
}

/* State 3: the bite (slower from frame 9); at frame 9 it spits a ball, 8 ahead and 26 up; then
 * back to swaying (0). */
void neckbiteSpit(Obj *o) {
    Neckbite *w = NECK(o);
    s32 pad;
    f32 step = (MODEL(w)->frame < 9.0) ? 0.4f : 0.2f;

    if (eneAnimLoop(o, MODEL(w), step)) {
        MODEL(w)->anim = etskNeckbiteAnim;
        MODEL(w)->frame = 0.0f;
        w->e.state = 0;
    }
    if (9.0 < MODEL(w)->frame && MODEL(w)->frame <= 9.0 + step) {
        if (w->way == 4) {
            eneMakeEx(NULL, ENE_NECKBITE_TAMA_MAIN, w->e.zone, w->e.x - 8.0f, w->e.y + 26.0f, w->e.z, 2, TYPE_ARG(w->way));
        } else if (w->way == 1) {
            eneMakeEx(NULL, ENE_NECKBITE_TAMA_MAIN, w->e.zone, w->e.x + 8.0f, w->e.y + 26.0f, w->e.z, 2, TYPE_ARG(w->way));
        }
    }
}

/* State 4: one animation step, then 5. */
void neckbiteStop(Obj *o) {
    Neckbite *w = NECK(o);

    eneAnimLoop(o, MODEL(w), 0.3f);
    w->e.state = 5;
}

/* State 5: as 4. */
void neckbiteStopped(Obj *o) {
    Neckbite *w = NECK(o);

    eneAnimLoop(o, MODEL(w), 0.3f);
    w->e.state = 5;
}

/* The ball's task (kind 0x218): its placement's type is the way it flies (4: to -x). */
s32 neckbiteTamaMain(Obj *o, s16 msg, void *arg) {
    EneObj *w = (EneObj *) FX(o);
    s32 pad;
    s32 type;

    switch (msg) {
        case 2:
            o->name = "tama";
            type = eneSetup(o, arg);
            if ((w->model = (Frame *) eneNewPol(o, (Gfx *) neckbiteTamaDl)) == NULL) {
                return -1;
            }
            if ((w->atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->model, 30, 30, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 123;
            if ((w->dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->model, 40, 80, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->behave = 0x807;
            w->vel[0] = (type == 4) ? -3.0 : 3.0;
            break;
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            if (w->hit & 0xE) {
                w->behave |= 0x4000000;
            }
            if (w->hit & 0x10) {
                if (w->hitKind != 10 && w->hitKind != 13 && w->hitKind != 14) {
                    if (w->hitKind == 21) {
                        w->behave |= 0x4000000;
                    } else {
                        w->behave |= 0x10000000;
                    }
                }
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_neckbite_tama: 死亡\n");
                return -1;
            }
            break;
        case 1:
            FRM_DESTROY(w->model);
            FRM_DESTROY(w->atk);
            FRM_DESTROY(w->dmg);
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
