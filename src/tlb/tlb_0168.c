#include "tlb.h"

/* The "majo" enemy kind (archive file 168): a witch. Its placement's type picks its flight:
 * 1-4 wait for a player within 120, fly at it, stop, turn and climb or dive away (types 1 and 3
 * dive, 2 and 4 climb) while fading out; 5 and 6 wait for a player within 70, rise 40, turn
 * back along the course and fly off (5 to the left, 6 to the right), fading out too. Once
 * faded it is gone (behave 0x40000). It draws with its env colour's alpha. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define ATK_ADD_XYZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s16, s32)) acollAtkAddXYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

/* libultra's sinf (PR/gu.h declares it), called from the TLB through its address. */
#define sinf (*sinf)

/* fabs is IDO's intrinsic: an inline abs.d. */
#pragma intrinsic (fabs)

extern u8 etskMajoAnim0[];
extern u8 etskMajoAnim1[];
extern u8 etskMajoAnim2[];
extern u8 etskMajoAnim3[];
extern u8 etskMajoHalf0Dl[];
extern u8 etskMajoHalf1Dl[];

/* A majo's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Majo {
    /* 0x00 */ EneObj e;
    /* 0x4C */ s16 homeY; /* where it started to rise (state 4) */
    /* 0x4E */ u8 type; /* its placement's type, from eneSetup */
    /* 0x4F */ u8 way; /* the nearest player's way (eneNearestPlWay) */
    /* 0x50 */ u8 timer; /* steps before it starts to fade */
    /* 0x51 */ u8 alpha; /* its env colour's alpha: 255, fading by 32 */
} Majo;

#define MAJO(obj) ((Majo *) FX(obj))
#define MODEL(w) ((AnimeFrame *) (w)->e.model)
#define ARM(w) ((ACollFrame *) (w)->e.atk->frame.child)

void majoWait(Obj *o);
void majoSwoop(Obj *o);
void majoBrake(Obj *o);
void majoFlyAway(Obj *o);
void majoWaitRise(Obj *o);
void majoRise(Obj *o);
void majoCross(Obj *o);
void majoFlyOff(Obj *o);

s32 Etsk_majo_main(Obj *o, s16 msg, void *arg) {
    Majo *w = MAJO(o);
    ACollFrame *a;

    switch (msg) {
        case 2:
            o->name = "majo";
            w->type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewAnime(o, (u32) etskMajoAnim1)) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 50, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((a = ATK_ADD_XYZ((Frame *) w->e.atk, w->e.model, 20, 120, 20, 0, 0)) == NULL) {
                return -1;
            }
            a->kind = 120;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 160, 140, -60, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->e.shadow = (Obj *) shadowCreate()) == NULL) {
                return -1;
            }
            w->e.halves[0] = (u32) etskMajoHalf0Dl;
            w->e.halves[1] = (u32) etskMajoHalf1Dl;
            w->alpha = 255;
            w->e.behave = 0x400000;
            switch (w->type) {
                case 1:
                case 2:
                    w->way = eneNearestPlWay(o);
                    if (w->way == 1) {
                        w->e.behave |= 0x40000;
                    }
                    w->e.state = 0;
                    break;
                case 3:
                case 4:
                    w->way = eneNearestPlWay(o);
                    if (w->way == 4) {
                        w->e.behave |= 0x40000;
                    }
                    w->e.state = 0;
                    break;
                case 5:
                case 6:
                    w->way = eneNearestPlWay(o);
                    w->e.state = 4;
                    break;
            }
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*majoWait)(o);
                    break;
                case 1:
                    (*majoSwoop)(o);
                    break;
                case 2:
                    (*majoBrake)(o);
                    break;
                case 3:
                    (*majoFlyAway)(o);
                    break;
                case 4:
                    (*majoWaitRise)(o);
                    break;
                case 5:
                    (*majoRise)(o);
                    break;
                case 6:
                    (*majoCross)(o);
                    break;
                case 7:
                    (*majoFlyOff)(o);
                    break;
            }
            if (w->alpha < 255) {
                frmMove(w->e.model, gFrmLayer8);
            }
            if (MODEL(w)->anim == etskMajoAnim0 || MODEL(w)->anim == etskMajoAnim1) {
                ARM(w)->ofs[0] = 0;
                ARM(w)->ofs[2] = 120;
            } else if (MODEL(w)->anim == etskMajoAnim2 || MODEL(w)->anim == etskMajoAnim3) {
                ARM(w)->ofs[0] = 120;
                ARM(w)->ofs[2] = 0;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_majo_main: 死亡\n");
                return -1;
            }
            MODEL(w)->pos[1] = (f64) sinf(gGameFrameCount / 10.0f) + w->e.y;
            /* fallthrough */
        case 3:
            MODEL(w)->dl = gGfxDlHead;
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
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
            if (w->e.shadow != NULL) {
                frmDestroy((Frame *) w->e.shadow);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* State 0: waits for a player within 120, then flies at it (1) from the end of its swoop. */
void majoWait(Obj *o) {
    Majo *w = MAJO(o);
    s32 pad;

    if (enePlInRangeCheck(o, 120.0f, 0)) {
        MODEL(w)->anim = etskMajoAnim1;
        MODEL(w)->frame = polAnimeLastFrame(MODEL(w));
        w->e.rot[1] = eneCourseAngle(o, w->way);
        w->e.vel[0] = (w->way == 4) ? -2.5 : 2.5;
        w->e.state = 1;
    }
}

/* State 1: flies at the player, tilted by its speed, until one is within 30 (2). */
void majoSwoop(Obj *o) {
    Majo *w = MAJO(o);
    s32 pad;

    w->e.rot[1] = eneCourseAngle(o, w->way);
    if (enePlInRangeCheck(o, 30.0f, 0)) {
        w->e.state = 2;
    }
    w->e.rot[0] = fabs(w->e.vel[0]) / 2.5 * 3.14159274101257324 / 10.0;
}

/* State 2: brakes; stopped, it plays the swoop backwards, then turns a quarter and climbs or
 * dives (3) for 20 steps before it fades. */
void majoBrake(Obj *o) {
    Majo *w = MAJO(o);
    s32 pad[2];

    w->e.rot[1] = eneCourseAngle(o, w->way);
    if (w->e.vel[0] != 0.0f) {
        eneAnimPlayOnce(o, MODEL(w), -0.8f);
        if (w->way == 4) {
            w->e.vel[0] += 0.4;
            if (w->e.vel[0] >= 0.0) {
                w->e.vel[0] = 0.0f;
            }
        } else if (w->way == 1) {
            w->e.vel[0] -= 0.4;
            if (w->e.vel[0] <= 0.0) {
                w->e.vel[0] = 0.0f;
            }
        }
    } else if (eneAnimPlayOnce(o, MODEL(w), -0.8f)) {
        MODEL(w)->anim = etskMajoAnim2;
        MODEL(w)->frame = 0.0f;
        w->timer = 20;
        w->e.rot[1] -= 1.570796371f;
        w->e.state = 3;
    }
    w->e.rot[0] = fabs(w->e.vel[0]) / 2.5 * 3.14159274101257324 / 10.0;
}

/* State 3: dives (types 1, 3) or climbs (2, 4) up to 3, pitched by the speed; when its timer
 * is out it fades by 32 a step and is gone once clear. */
void majoFlyAway(Obj *o) {
    Majo *w = MAJO(o);

    eneAnimLoop(o, MODEL(w), 0.5f);
    switch (w->type) {
        case 1:
        case 3:
            w->e.vel[2] -= 0.2;
            if (w->e.vel[2] <= -3.0) {
                w->e.vel[2] = -3.0f;
            }
            break;
        case 2:
        case 4:
            w->e.vel[2] += 0.2;
            if (w->e.vel[2] >= 3.0) {
                w->e.vel[2] = 3.0f;
            }
            break;
    }
    switch (w->type) {
        case 1:
        case 2:
            w->e.rot[0] = w->e.vel[2] / 3.0 * -3.14159274101257324 / 4;
            break;
        case 3:
        case 4:
            w->e.rot[0] = w->e.vel[2] / 3.0 * 3.14159274101257324 / 4;
            break;
    }
    if (w->timer - 1 >= 0) {
        w->timer = w->timer - 1;
    } else {
        w->timer = 0;
        if (w->alpha - 32 > 0) {
            w->alpha = w->alpha - 32;
        } else {
            w->alpha = 0;
            w->e.behave |= 0x40000;
        }
    }
}

/* State 4: waits for a player within 70, then rises (5) from where it stands. */
void majoWaitRise(Obj *o) {
    Majo *w = MAJO(o);
    s32 pad;

    if (enePlInRangeCheck(o, 70.0f, 0)) {
        MODEL(w)->anim = etskMajoAnim2;
        MODEL(w)->frame = 0.0f;
        w->homeY = w->e.y;
        w->e.vel[1] = 2.0f;
        w->e.rot[1] = eneCourseAngle(o, w->way) - 1.570796371f;
        w->e.state = 5;
    }
}

/* State 5: rises 40, slows to a stop, then turns to face along the course (6). */
void majoRise(Obj *o) {
    Majo *w = MAJO(o);
    s32 pad[2];
    f32 step;

    eneAnimLoop(o, MODEL(w), 1.0f);
    if (w->e.y - w->homeY >= 40.0f) {
        w->e.vel[1] -= 0.2;
        if (w->e.vel[1] <= 0.0f) {
            w->e.vel[1] = 0.0f;
            step = (w->way == 4) ? 0.2 : -0.2;
            if (!eneTurnRotYTo(o, eneCourseAngle(o, 8) - 1.570796371f, step)) {
                MODEL(w)->anim = etskMajoAnim3;
                MODEL(w)->frame = 0.0f;
                w->way = eneNearestPlWay(o);
                w->e.state = 6;
            }
        }
    }
}

/* State 6: speeds up across the course to 3; once past its middle it flies off (7). */
void majoCross(Obj *o) {
    Majo *w = MAJO(o);

    eneAnimPlayOnce(o, MODEL(w), 0.8f);
    w->e.vel[2] += 0.3;
    if (w->e.vel[2] >= 3.0) {
        w->e.vel[2] = 3.0f;
        if (w->e.z > 0.0f) {
            w->timer = 20;
            w->e.state = 7;
        }
    }
}

/* State 7: turns along the course (type 5 left, 6 right) at a total speed of 3, banked by it,
 * and fades out as in state 3. */
void majoFlyOff(Obj *o) {
    Majo *w = MAJO(o);
    s32 pad[2];

    eneAnimPlayOnce(o, MODEL(w), 0.8f);
    if (w->type == 5) {
        w->e.vel[0] -= 0.1;
        if (w->e.vel[0] <= -3.0) {
            w->e.vel[0] = -3.0f;
        }
    } else if (w->type == 6) {
        w->e.vel[0] += 0.1;
        if (w->e.vel[0] >= 3.0) {
            w->e.vel[0] = 3.0f;
        }
    }
    w->e.vel[2] = 3.0 - fabs(w->e.vel[0]);
    w->e.rot[1] = eneCourseAngle(o, 8) + mathAtan2(w->e.vel[0], w->e.vel[2]) - 1.570796371f;
    PRINTF("ry %f\n", w->e.rot[1]);
    w->e.rot[0] = w->e.vel[0] / 3.0 * -3.14159274101257324 / 4;
    if (w->timer - 1 >= 0) {
        w->timer = w->timer - 1;
    } else {
        w->timer = 0;
        if (w->alpha - 32 > 0) {
            w->alpha = w->alpha - 32;
        } else {
            w->alpha = 0;
            w->e.behave |= 0x40000;
        }
    }
}
