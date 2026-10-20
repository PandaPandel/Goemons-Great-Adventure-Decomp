#include "tlb.h"

/* The crying old man (archive file 89): "konaki" (子泣き爺, the tag 0xBBD2B5E3 子泣 it sends)
 * crouches at its placement and peeks up and down (state 0) until the camera comes within 70,
 * then hops along x (1). With a player within 50 at the end of a hop it ducks (5) and leaps at
 * that player (2), as far as the player is. When its attack lands it clings to that player (6),
 * riding the player's model until the player shakes it off (the tag 0xCEA5C3A6 離脱 "leave"),
 * leaves play, or takes an action with no attach point. A hit knocks it back (3); it gets up
 * (4) and hops again (1). */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)
/* model.c takes the frame as an s32; the callers pass the model's f32 frame (pldive.c). */
#define ANIM_NODE_POS ((void (*)(u32, f32, u32, f32 *, f32 *, f32 *)) mdlAnimNodePos)

/* tlb.h has eneAfterProc's far-call macro without its declaration. */
#define sinf (*sinf)
#define cosf (*cosf)

extern u8 etskKonakiAnim[];
extern u8 etskKonakiTagAnim[];
extern u8 etskKonakiHitAnim[];
extern u8 etskKonakiKnockbackAnim[];
extern u8 etskKonakiHalf0Dl[];
extern u8 etskKonakiHalf1Dl[];

/* A konaki's work, at its task + 0x20. */
typedef struct Konaki {
    /* 0x00 */ EneObj e;
    /* 0x4C */ s8 player; /* the player it clings to (-1: none) */
    /* 0x4D */ u8 raised; /* 1: risen to frame 3, going back down */
} Konaki;

#define KONAKI(obj) ((Konaki *) FX(obj))
#define MODEL(w) ((AnimeFrame *) (w)->e.model)

void konakiCrouch(Obj *o);
void konakiHop(Obj *o);
void konakiKnockback(Obj *o);
void konakiGetUp(Obj *o);
void konakiDuck(Obj *o);
s32 konakiCling(Obj *o);
void konakiLetGo(Obj *o);
s32 konakiSendTag(Obj *obj, s32 tag, s32 a, s32 b);

/* Etsk_konaki_main */
s32 Etsk_konaki_main(Obj *o, s16 msg, void *arg) {
    Konaki *w = KONAKI(o);
    s32 pad;
    s32 type;
    s32 i;

    switch (msg) {
        case 2:
            o->name = "konaki";
            type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewAnime(o, (u32) etskKonakiAnim)) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 50, 80, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 0x7F;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 70, 160, 0, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->e.shadow = (Obj *) shadowCreate()) == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                return -1;
            }
            w->e.halves[0] = (u32) etskKonakiHalf0Dl;
            w->e.halves[1] = (u32) etskKonakiHalf1Dl;
            w->e.behave = 0x404002;
            w->raised = 0;
            w->e.hp = 1;
            if (type == 1) {
                w->e.rot[1] = eneCourseAngle(o, eneNearestPlWay(o));
                w->e.state = 0;
            } else if (type == 2) {
                w->e.rot[1] = eneCourseAngle(o, 8);
                w->e.state = 0;
            }
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*konakiCrouch)(o);
                    break;
                case 1:
                case 2:
                    (*konakiHop)(o);
                    break;
                case 3:
                    (*konakiKnockback)(o);
                    break;
                case 4:
                    (*konakiGetUp)(o);
                    break;
                case 5:
                    (*konakiDuck)(o);
                    break;
                case 6:
                    (*konakiCling)(o);
                    break;
            }
            if (w->e.state != 6) {
                if (eneAfterProc(o) == -1) {
                    PRINTF("Etsk_konaki_main: 死亡\n");
                    return -1;
                }
            } else {
                shadowPut((MPolFrame *) w->e.shadow, MODEL(w)->pos[0], MODEL(w)->pos[1], MODEL(w)->pos[2],
                          1.0f, NULL);
            }
            if (w->e.hit & 0x10) {
                w->e.atk->flags &= ~2;
                w->e.dmg->flags &= ~2;
                MODEL(w)->anim = etskKonakiHitAnim;
                MODEL(w)->frame = 0.0f;
                w->e.behave &= ~0x4000;
                w->e.rot[1] = eneCourseAngle(o, 8);
                w->e.vel[0] = eneKnockbackSpeed(o, w->e.dmg, 1.5f);
                w->e.state = 3;
                w->e.vel[1] = 3.0f;
            }
            if (w->e.hit & 8) {
                w->player = -1;
                for (i = 0; i < 4; i++) {
                    if (w->e.atk->hitPower == (s32) gGameWork.players[i]) {
                        w->player = i;
                    }
                }
                if (w->player >= 0 && plIsInPlay(w->player) &&
                    konakiSendTag(gGameWork.players[w->player], 0xBBD2B5E3 /* 子泣 */, (s32) o, 0)) {
                    w->e.atk->flags &= ~2;
                    /* Compared and stored as a pointer (the anim field is a void *): an address
                     * spelled as a pointer is built at each use, as the game builds it again. */
                    if (MODEL(w)->anim != etskKonakiTagAnim) {
                        eneSetSd2(o, 0x2ED, 0, 0);
                        MODEL(w)->anim = etskKonakiTagAnim;
                        MODEL(w)->frame = 0.0f;
                    }
                    w->e.state = 6;
                    w->e.vel[0] = w->e.vel[1] = w->e.vel[2] = 0.0f;
                }
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
            if (w->e.shadow != NULL) {
                frmDestroy((Frame *) w->e.shadow);
            }
            break;
        case 9:
            if (*(s32 *) arg == 0xCEA5C3A6 /* 離脱 */) {
                konakiLetGo(o);
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* konakiCrouch: state 0, peeks up to frame 3 and back down; the camera near at either end
 * starts the hopping (state 1). */
void konakiCrouch(Obj *o) {
    Konaki *w = KONAKI(o);
    f32 step = 0.4f;

    if (w->raised == 0) {
        eneAnimPlayOnce(o, MODEL(w), step);
        if (MODEL(w)->frame >= 3.0) {
            if (eneCamNearCheck(o, 70.0f)) {
                MODEL(w)->frame = 0.0f;
                w->e.state = 1;
            } else {
                MODEL(w)->frame = 3.0f;
                w->raised = 1;
            }
        }
    } else {
        eneAnimPlayOnce(o, MODEL(w), -step);
        if (MODEL(w)->frame == 0.0) {
            if (eneCamNearCheck(o, 70.0f)) {
                w->e.state = 1;
            } else {
                w->raised = 0;
            }
        }
    }
}

/* konakiHop: states 1 and 2, hops (a sound and a jump at frame 3 of each loop); state 2 jumps
 * toward the nearest player, as far as it is. Each loop ends in state 5 with a player within
 * 50, else 1. */
void konakiHop(Obj *o) {
    Konaki *w = KONAKI(o);
    s32 pad;
    f32 step = (w->e.state == 2) ? 0.4 : 0.6;
    f32 speed;
    s32 dir;
    s32 pad2;

    if (eneAnimLoop(o, MODEL(w), step)) {
        if (enePlInRangeCheck(o, 50.0f, 0)) {
            w->e.state = 5;
        } else {
            w->e.state = 1;
        }
    }
    if (MODEL(w)->frame > 3.0 && MODEL(w)->frame <= 3.0 + step) {
        if (w->e.hit & 1) {
            eneSetSd2(o, 0x239, 0, 0);
            if (w->e.state == 2) {
                w->e.vel[1] = 4.0f;
                speed = enePlDist(o, 0, 1);
                speed = speed / (9.0 / step);
            } else {
                w->e.vel[1] = 4.0f;
                speed = 1.0f;
            }
            dir = enePlDirX(o, 0);
            if (dir == 4) {
                w->e.vel[0] = -speed;
            } else {
                w->e.vel[0] = speed;
            }
        }
    }
    if (!(MODEL(w)->frame > 3.0 && MODEL(w)->frame <= 11.0)) {
        w->e.vel[0] = 0.0f;
    }
    if (MODEL(w)->frame > 11.0 && !(w->e.hit & 1)) {
        MODEL(w)->frame = 11.0f;
    }
    eneFall(o, 8.0 / (9.0 / step));
}

/* konakiKnockback: state 3, flies back until on the ground, then gets up (state 4). */
void konakiKnockback(Obj *o) {
    Konaki *w = KONAKI(o);

    if (eneAnimPlayOnce(o, MODEL(w), 0.8f)) {
        if (w->e.hit & 1) {
            MODEL(w)->anim = etskKonakiKnockbackAnim;
            MODEL(w)->frame = 0.0f;
            w->e.state = 4;
            w->e.vel[0] = 0.0f;
        }
    }
    eneFall(o, 0.35f);
}

/* konakiGetUp: state 4, gets up; its boxes live again, it hops (state 1). */
void konakiGetUp(Obj *o) {
    Konaki *w = KONAKI(o);

    if (eneAnimPlayOnce(o, MODEL(w), 0.8f)) {
        MODEL(w)->anim = etskKonakiAnim;
        MODEL(w)->frame = 0.0f;
        w->e.atk->flags |= 2;
        w->e.dmg->flags |= 2;
        w->e.behave |= 0x4000;
        w->e.state = 1;
    }
}

/* konakiDuck: state 5, rises to frame 3 and back down, then leaps at the player (state 2). */
void konakiDuck(Obj *o) {
    Konaki *w = KONAKI(o);
    s32 pad[3];

    if (w->raised == 0) {
        eneAnimPlayOnce(o, MODEL(w), 0.4f);
        if (MODEL(w)->frame >= 3.0) {
            MODEL(w)->frame = 3.0f;
            w->raised = 1;
        }
    } else {
        eneAnimPlayOnce(o, MODEL(w), -0.4f);
        if (MODEL(w)->frame == 0.0) {
            w->raised = 0;
            w->e.state = 2;
        }
    }
}

/* konakiCling: state 6, rides the player's attach point (in tenths) turned by the player's
 * yaw; lets go (konakiLetGo) when the player is out of play or its action has none. */
s32 konakiCling(Obj *o) {
    Konaki *w = KONAKI(o);
    s32 pad[2];
    f32 x;
    f32 y;
    f32 z;
    s32 zone;
    PlActAnim *anim;
    f32 s;

    if (!plIsInPlay(w->player)) {
        konakiLetGo(o);
        return 0;
    }
    anim = plActAnimeGet(PL(gGameWork.players[w->player]));
    if (anim->attach == 0) {
        konakiLetGo(o);
        return 0;
    }
    eneAnimLoop(o, MODEL(w), 0.4f);
    ANIM_NODE_POS(anim->attach, PL(gGameWork.players[w->player])->model->frame,
                  PL(gGameWork.players[w->player])->model->segs[0], &x, &y, &z);
    x /= 10.0f;
    y /= 10.0f;
    z /= 10.0f;
    s = sinf(PL(gGameWork.players[w->player])->model->rot[1]);
    MODEL(w)->pos[0] = PL(gGameWork.players[w->player])->model->pos[0] +
                       (cosf(PL(gGameWork.players[w->player])->model->rot[1]) * x + z * s);
    s = cosf(PL(gGameWork.players[w->player])->model->rot[1]);
    MODEL(w)->pos[2] = PL(gGameWork.players[w->player])->model->pos[2] +
                       (z * s - sinf(PL(gGameWork.players[w->player])->model->rot[1]) * x);
    MODEL(w)->pos[1] = PL(gGameWork.players[w->player])->model->pos[1] + y;
    MODEL(w)->rot[1] = PL(gGameWork.players[w->player])->model->rot[1];
    plAskPos(gGameWork.players[w->player], &w->e.x, &w->e.y, &w->e.z, &zone);
    w->e.zone = zone;
    w->e.rot[1] = plGetModelYaw(gGameWork.players[w->player]);
    return 1;
}

/* konakiLetGo: drops off the player onto the course below its model, knocked back (state 3). */
void konakiLetGo(Obj *o) {
    Konaki *w = KONAKI(o);
    s32 pad;

    w->e.dmg->flags &= ~2;
    MODEL(w)->anim = etskKonakiHitAnim;
    MODEL(w)->frame = 0.0f;
    w->e.x = vramNearestPlaceXZ(w->e.zone, MODEL(w)->pos[0], MODEL(w)->pos[2], &w->e.z);
    w->e.y = MODEL(w)->pos[1];
    w->e.rot[1] = eneCourseAngle(o, 8);
    w->e.state = 3;
    w->e.vel[1] = 3.0f;
    w->e.vel[0] = 0.0f;
}

/* konakiSendTag: plSendTagMsg's copy (plpick.c). */
s32 konakiSendTag(Obj *obj, s32 tag, s32 a, s32 b) {
    s32 msg[3];

    msg[0] = tag;
    msg[1] = a;
    msg[2] = b;
    return tskSendMsg(obj, 9, msg);
}
