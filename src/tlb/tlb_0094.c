#include "tlb.h"

/* The "turube" enemy kind (archive file 94), a tsurube (a well bucket that drops on a rope), and
 * "turube_j", its generator: a turube_j puts out a turube (kind 0xE0) every 60, 80 or 90 steps
 * by its placement's type, or (type 8) one once, when a player first comes near. */

/* A turube_j's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct TurubeJ {
    /* 0x00 */ EneObj e;
    /* 0x4C */ s16 time; /* steps since the last turube */
    /* 0x4E */ u8 type; /* its placement's type, passed on to each turube */
    /* 0x4F */ u8 done; /* type 8: its turube is made */
} TurubeJ;

#define TURUBEJ(obj) ((TurubeJ *) FX(obj))

s32 turubeJProc(Obj *o, s16 msg, void *arg) {
    TurubeJ *w = TURUBEJ(o);
    s32 type;

    switch (msg) {
        case 2:
            o->name = "turube_j";
            w->type = eneSetup(o, arg);
            type = w->type;
            if (type == 1 || type == 2 || type == 3 || type == 8) {
                if (eneSetOnEarth(o) == -1) {
                    PRINTF("地面にセット失敗してんねやんか\n");
                    return -1;
                }
                w->e.behave = 0x800;
            }
            break;
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            type = w->type;
            switch (type) {
                case 1:
                    if (w->time % 90 == 0) {
                        eneMakeEx(NULL, ENE_TURUBE, w->e.zone, w->e.x, w->e.y, w->e.z, 2,
                                  (u32 (*)(void *, void *, void *)) type);
                    }
                    w->time++;
                    if (w->time >= 9000) {
                        w->time = 0;
                    }
                    break;
                case 2:
                    if (w->time % 90 == 0) {
                        eneMakeEx(NULL, ENE_TURUBE, w->e.zone, w->e.x, w->e.y, w->e.z, 2,
                                  (u32 (*)(void *, void *, void *)) type);
                    }
                    w->time++;
                    if (w->time >= 9000) {
                        w->time = 0;
                    }
                    break;
                case 3:
                    if (w->time % 60 == 0) {
                        eneMakeEx(NULL, ENE_TURUBE, w->e.zone, w->e.x, w->e.y, w->e.z, 2,
                                  (u32 (*)(void *, void *, void *)) type);
                    }
                    w->time++;
                    if (w->time >= 6000) {
                        w->time = 0;
                    }
                    break;
                case 6:
                    if (w->time % 80 == 0) {
                        eneMakeEx(NULL, ENE_TURUBE, w->e.zone, w->e.x, w->e.y, w->e.z, 2,
                                  (u32 (*)(void *, void *, void *)) type);
                    }
                    w->time++;
                    if (w->time >= 8000) {
                        w->time = 0;
                    }
                    break;
                case 7:
                    if (w->time % 60 == 0) {
                        eneMakeEx(NULL, ENE_TURUBE, w->e.zone, w->e.x, w->e.y, w->e.z, 2,
                                  (u32 (*)(void *, void *, void *)) type);
                    }
                    w->time++;
                    if (w->time >= 6000) {
                        w->time = 0;
                    }
                    break;
                case 8:
                    if (!w->done && !eneCamNearCheck(o, 125.0f) && eneNearestPlWay(o) == 1) {
                        eneMakeEx(NULL, ENE_TURUBE, w->e.zone, w->e.x, w->e.y, w->e.z, 2,
                                  (u32 (*)(void *, void *, void *)) w->type);
                        w->done = 1;
                    }
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_turube_main: 死亡\n");
                return -1;
            }
            break;
        case 1:
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 turubeDl[];

/* A turube's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Turube {
    /* 0x00 */ EneObj e;
    /* 0x4C */ MPolFrame *mpol; /* a second copy of its model, drawn by its own matrix */
    /* 0x50 */ u8 type; /* its placement's type, from eneSetup */
    /* 0x51 */ u8 airborne;
    /* 0x52 */ s16 alpha; /* its fade-in, 0-0xFF */
    /* 0x54 */ f32 speed;
    /* 0x58 */ s16 fadeTimer;
    /* 0x5A */ u8 turned; /* 1: the copy is turned a quarter round */
} Turube;

#define TURUBE(obj) ((Turube *) FX(obj))
#define POL(w) ((PolFrame *) (w)->e.model)
/* Its radius: it rolls, turning by the distance over it. Its scale is read as a Vec3f: IDO sorts
 * a commutative operation's operands when one is an array element, and the game's order is the
 * source's. */
#define RADIUS(w) (10.0f * (((Vec3f *) POL(w)->scale)->x * 10.0f))

void turubeActRollIn(Obj *o);
void turubeActRoll(Obj *o);
void turubeActRollFadeIn(Obj *o);
void turubeActMoveToA(Obj *o);
void turubeActFadeOut(Obj *o);
void turubeActMoveToB(Obj *o);
void turubeActSpeedUpFadeOut(Obj *o);
void turubeActRollFadeOut(Obj *o);

s32 turubeProc(Obj *o, s16 msg, void *arg) {
    Turube *w = TURUBE(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "turube";
            PRINTF("Etsk_turube( )\n");
            w->type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) enePolModelAlpha(o, (Gfx *) turubeDl, 0)) == NULL) {
                return -1;
            }
            w->e.model->flags &= ~2;
            if ((w->mpol = (MPolFrame *) frmCreate(gFrmLayer8, gFrmKindMpol)) == NULL) {
                return -1;
            }
            w->mpol->dl2 = (Gfx *) turubeDl;
            eneGetSegs(o, w->mpol->segs);
            w->turned = 1;
            if ((w->e.shadow = (Obj *) shadowCreate()) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 70, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) uda_dmg_root_frame, w->e.model, 100, 200, -100, 0, 0)) == NULL) {
                return -1;
            }
            w->e.dmg->flags &= ~8;
            w->airborne = 0;
            w->fadeTimer = 0;
            if (w->type == 1 || w->type == 2) {
                w->turned = 0;
                w->e.behave = 0x801;
                w->alpha = 0;
                w->e.state = 0;
                w->speed = 0.0f;
            } else if (w->type == 3) {
                w->e.behave = 0x801;
                w->alpha = 0;
                w->e.state = 2;
            } else if (w->type == 6) {
                w->e.behave = 0x1101;
                w->alpha = 0xFF;
                w->e.state = 3;
            } else if (w->type == 7) {
                w->e.behave = 0x1101;
                w->alpha = 0xFF;
                w->e.state = 5;
            } else if (w->type == 8) {
                w->e.behave = 0x801;
                w->alpha = 0xFF;
                w->e.state = 7;
            }
            break;
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*turubeActRollIn)(o);
                    break;
                case 1:
                    (*turubeActRoll)(o);
                    break;
                case 2:
                    (*turubeActRollFadeIn)(o);
                    break;
                case 3:
                    (*turubeActMoveToA)(o);
                    break;
                case 4:
                    (*turubeActFadeOut)(o);
                    break;
                case 5:
                    (*turubeActMoveToB)(o);
                    break;
                case 6:
                    (*turubeActSpeedUpFadeOut)(o);
                    break;
                case 7:
                    (*turubeActRollFadeOut)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_turube_main: 死亡\n");
                return -1;
            }
            POL(w)->pos[1] = w->e.y + RADIUS(w);
            mathMtxIdent(w->mpol->mtx.m);
            if (w->turned == 1) {
                mathMtxPostRotY(w->mpol->mtx.m, 1.570796371f);
            }
            mathMtxPostRotX(w->mpol->mtx.m, -POL(w)->rot[0]);
            mathMtxPostRotY(w->mpol->mtx.m, POL(w)->rot[1]);
            mathMtxPostRotZ(w->mpol->mtx.m, POL(w)->rot[2]);
            mathMtxPostTrans(w->mpol->mtx.m, POL(w)->pos[0], POL(w)->pos[1], POL(w)->pos[2]);
            mathMtxScaleRotX(w->mpol->mtx.m, POL(w)->scale[0]);
            mathMtxScaleRotY(w->mpol->mtx.m, POL(w)->scale[1]);
            mathMtxScaleRotZ(w->mpol->mtx.m, POL(w)->scale[2]);
            break;
        case 3:
            w->mpol->dl = gGfxDlHead;
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            frmDestroy(&w->mpol->f);
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
    }
    return tskDefaultProc(o, msg, arg);
}

/* State 0 (types 1 and 2): it rolls in along z, fading in, until it lands, then turns to roll
 * along x (state 1). */
void turubeActRollIn(Obj *o) {
    Turube *w;
    s32 pad;

    w = TURUBE(o);
    w->e.rot[1] = eneCourseAngle(o, 8);
    if (w->e.z < 0.0f) {
        w->e.vel[2] = 1.799999952f;
    } else {
        w->e.vel[2] = 0.0f;
        if (w->e.hit & 1) {
            eneSetSd(o, 0x104);
            effectSibukiCreateWhite(POL(w)->pos[0], w->e.y, POL(w)->pos[2], 0.1000000015f, 0.1000000015f,
                                    0.004999999888f, 0.009999999776f);
            w->turned = 1;
            w->e.rot[1] = eneCourseAngle(o, 4);
            w->e.state = 1;
        }
    }
    w->e.rot[0] = w->e.z / RADIUS(w);
    w->alpha += 0x10;
    if (w->alpha >= 0x100) {
        w->alpha = 0xFF;
        frmMove(&w->mpol->f, gFrmModelLayer);
    }
}

/* State 1: it rolls along x, speeding up to 1.8, its way by its type, splashing when it lands. */
void turubeActRoll(Obj *o) {
    Turube *w;
    s32 pad;

    w = TURUBE(o);
    w->e.rot[1] = eneCourseAngle(o, 4);
    if (w->speed < 1.8) {
        w->speed += 0.0750000000000000111;
    }
    if (w->type == 1) {
        w->e.vel[0] = -w->speed;
    } else if (w->type == 2) {
        w->e.vel[0] = w->speed;
    }
    if (w->e.hit & 1) {
        if (w->airborne) {
            eneSetSd(o, 0x104);
            w->airborne = 0;
            effectSibukiCreateWhite(POL(w)->pos[0], w->e.y, POL(w)->pos[2], 0.1000000015f, 0.1000000015f,
                                    0.004999999888f, 0.009999999776f);
        }
    } else {
        w->airborne = 1;
    }
    w->e.rot[0] += -w->e.vel[0] / RADIUS(w);
}

/* State 2 (type 3): it rolls along x at -1.8, fading in. */
void turubeActRollFadeIn(Obj *o) {
    Turube *w;
    s32 pad;

    w = TURUBE(o);
    w->e.rot[1] = eneCourseAngle(o, 4);
    w->e.vel[0] = -1.799999952f;
    w->alpha += 0x10;
    if (w->alpha >= 0x100) {
        w->alpha = 0xFF;
        frmMove(&w->mpol->f, gFrmModelLayer);
    }
    w->e.rot[0] += -w->e.vel[0] / RADIUS(w);
}

/* State 3 (type 6): it rolls to (261, 329, -13), then on (state 4). */
void turubeActMoveToA(Obj *o) {
    Turube *w;
    s32 pad;

    w = TURUBE(o);
    w->e.rot[1] = eneCourseAngle(o, 4);
    if (eneMoveToPos3D(o, 261.0f, 329.0f, -13.0f, 2.299999952f)) {
        w->e.behave &= ~0x1000;
        w->e.behave |= 0x800;
        w->e.state = 4;
    }
    w->e.rot[0] += -w->e.vel[0] / RADIUS(w);
}

/* State 4: it rolls on, kept at z <= 0, and after 31 steps fades out and dies. */
void turubeActFadeOut(Obj *o) {
    Turube *w;
    s32 pad;

    w = TURUBE(o);
    if (w->e.z >= 0.0f) {
        w->e.z = 0.0f;
    }
    if (w->e.hit & 1) {
        if (w->airborne) {
            eneSetSd(o, 0x23D);
            w->airborne = 0;
        }
    } else {
        w->airborne = 1;
    }
    w->fadeTimer++;
    if (w->fadeTimer >= 31) {
        w->alpha -= 0x20;
        if (w->alpha <= 0x40) {
            w->e.atk->flags &= ~2;
            w->e.dmg->flags &= ~2;
        }
        if (w->alpha <= 0) {
            w->alpha = 0;
            w->e.behave |= 0x40000;
        }
    }
    w->e.rot[0] += -w->e.vel[0] / RADIUS(w);
}

/* State 5 (type 7): it rolls to (4089, 354, -14), then on (state 6). */
void turubeActMoveToB(Obj *o) {
    Turube *w;
    s32 pad;

    w = TURUBE(o);
    w->e.rot[1] = eneCourseAngle(o, 4);
    if (eneMoveToPos3D(o, 4089.0f, 354.0f, -14.0f, 2.0f)) {
        w->e.behave &= ~0x1000;
        w->e.behave |= 0x800;
        w->e.state = 6;
    }
    w->e.rot[0] += -w->e.vel[0] / RADIUS(w);
}

/* State 6: on the ground it speeds up to 2, and after 81 steps fades out and dies. */
void turubeActSpeedUpFadeOut(Obj *o) {
    Turube *w;
    s32 pad;

    w = TURUBE(o);
    w->e.rot[1] = eneCourseAngle(o, 4);
    if (w->e.z >= 0.0f) {
        w->e.z = 0.0f;
    }
    if (w->e.hit & 1) {
        if (w->speed == 0.0) {
            eneSetSd(o, 0x23D);
        }
        if (w->speed < 2.0) {
            w->speed += 0.149999999999999994;
        }
        w->e.vel[0] = w->speed;
    }
    w->fadeTimer++;
    if (w->fadeTimer >= 81) {
        w->alpha -= 0x20;
        if (w->alpha <= 0x40) {
            w->e.atk->flags &= ~2;
            w->e.dmg->flags &= ~2;
        }
        if (w->alpha <= 0) {
            w->alpha = 0;
            w->e.behave |= 0x40000;
        }
    }
    w->e.rot[0] += -w->e.vel[0] / RADIUS(w);
}

/* State 7 (type 8): it rolls along x at 1.8, and after 161 steps fades out and dies. */
void turubeActRollFadeOut(Obj *o) {
    Turube *w;
    s32 pad;

    w = TURUBE(o);
    w->e.rot[1] = eneCourseAngle(o, 4);
    w->e.vel[0] = 1.799999952f;
    if (w->e.hit & 1) {
        if (w->airborne) {
            eneSetSd(o, 0x23D);
            w->airborne = 0;
        }
    } else {
        w->airborne = 1;
    }
    w->fadeTimer++;
    if (w->fadeTimer >= 161) {
        w->alpha -= 0x20;
        if (w->alpha <= 0) {
            w->alpha = 0;
            w->e.behave |= 0x40000;
        }
    }
    w->e.rot[0] += -w->e.vel[0] / RADIUS(w);
}
