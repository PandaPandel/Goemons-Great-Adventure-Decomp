#include "player.h"
#include "gfxmgr.h"
#include "mathutil.h"
#include "plswing.h"
#include "vram.h"

/* Swinging from a hook (overlay 9's third file). The character hangs from the point its damage
 * box hit, as a pendulum: the stick across pumps it, A lets go. Swung high enough it goes over
 * the top. Letting go throws the character with the swing's speed, or where the stick points. */

/* A swing's work (Player.work). */
typedef struct PlSwing {
    /* 0x00 */ f32 angle; /* from hanging straight down */
    /* 0x04 */ f32 speed; /* 15 steps a radian */
    /* 0x08 */ f32 energy;
    /* 0x0C */ s32 state; /* 1: swung over */
    /* 0x10 */ s32 still; /* the still pose is up */
    /* 0x14 */ ACollFrame *box; /* its attack box */
} PlSwing;

extern ACollFrame *pl_atk_root_frame; /* the player attacks' boxes */
u16 gPlSwingPumpSound[] = { 0x0397, 0x0000, 0x03B6, 0x03D3 }; /* per character: the pump sound */
u16 gPlSwingOverSound[] = { 0x038E, 0x036F, 0x03B7, 0x03BC }; /* per character: going over */
u16 gPlSwingLetGoVoice[] = { 0x0541, 0x0535, 0x054E, 0x055A }; /* per character: letting go */


/* The swing's default. Letting go throws the character: along the swing, or (swung over) the
 * way the stick points, 10 across and 10 up, 3 down or 5.5 up. */
void plSwingDefault(Player *p, s32 msg, s32 a, s32 b) {
    PlSwing *w;
    f32 s;
    f32 sy;
    f32 sx;
    f32 c;

    switch (msg) {
        case 0:
            break;
        case 1:
            return;
        case 2:
            w = (PlSwing *) p->work;
            if (!(p->flags & 0x20000)) {
                switch (w->state) {
                    case 0:
                        s = mathSin(w->angle);
                        c = mathCos(w->angle);
                        p->pushX = w->speed * -6.0f * c;
                        p->pushY = w->speed * 1.5f * s;
                        break;
                    case 1:
                        sx = plStickSpeedX(p);
                        sy = plStickSpeedY(p);
                        if (fabsf(sx) < 0.05f && fabsf(sy) < 0.05f) {
                            s = mathSin(w->angle);
                            c = mathCos(w->angle);
                            p->pushX = w->speed * -4.0f * c;
                            p->pushY = w->speed * s;
                        } else {
                            if (fabsf(sx) < 0.05f) {
                                p->pushX = 0.0f;
                            } else if (sx > 0.0f) {
                                p->pushX = 10.0f;
                            } else {
                                p->pushX = -10.0f;
                            }
                            if (fabsf(sy) < 0.05f) {
                                p->pushX *= 1.2f;
                                p->pushY = 5.5f;
                            } else if (sy > 0.0f) {
                                p->pushY = 10.0f;
                            } else {
                                p->pushY = -3.0f;
                            }
                        }
                        break;
                }
            }
            break;
    }
    plBaseDefault(p, msg, a, b);
}

/* A step of the swing: pumping with the stick across (ten times as hard with the swing), a
 * little damping, the pull down, and going over once the energy passes 12.5. The lean follows
 * the angle. A lets go: the next action is 0xD forwards, 0xC backwards.
 * The damping's two 1s are int literals: the game loads 1.0f once into a register for both. */
s32 plSwingStep(Player *p) {
    PlSwing *w = (PlSwing *) p->work;
    s32 next = -1;
    f32 pot;
    f32 pull;
    f32 push;
    f32 step;
    s32 pad[2];
    f32 c;

    switch (w->state) {
        case 0:
            push = -plStickSpeedX(p) * 0.1f;
            if (mathCos(w->angle) < 0.0f) {
                if (w->speed > 0.0f && push > 0.0f) {
                    push *= 10.0f;
                } else if (w->speed < 0.0f && push < 0.0f) {
                    push *= 10.0f;
                }
            } else {
                if (w->speed > 0.0f && push < 0.0f) {
                    push *= 10.0f;
                } else if (w->speed < 0.0f && push > 0.0f) {
                    push *= 10.0f;
                }
            }
            c = mathCos(w->angle);
            pull = mathSin(w->angle) * (p->gravity * 0.5f) + push * c;
            w->speed = w->speed * (1 - 0.03f / (w->energy * 0.4f + 1)) + pull;
            if (p->actionSteps == 0xF) {
                plSound(p, gPlSwingPumpSound[p->chara]);
            }
            break;
        case 1:
            w->speed *= 1.1f;
            break;
    }
    if (w->speed < -7.0f) {
        w->speed = -7.0f;
    } else if (w->speed > 7.0f) {
        w->speed = 7.0f;
    }
    step = w->speed / 15.0f;
    w->angle += step;
    pot = (1.0f - mathCos(w->angle)) * (-(p->gravity * 0.5f) * 15.0f);
    w->energy = w->speed * w->speed * 0.5f + pot;
    if (w->state != 1 && w->speed * w->speed * 0.5f + pot * 0.5f > 12.5f) {
        plSound(p, gPlSwingOverSound[p->chara]);
        w->state = 1;
    }
    if (p->facing < 1.5707962f) {
        p->lean = w->angle;
    } else {
        p->lean = -w->angle;
    }
    if (w->state == 0) {
        p->lean += -0.47123894f;
    }
    if (plSwingAPressed(p)) {
        p->flags &= ~0x20004;
        next = (w->speed * p->lean >= 0.0f) ? 0xD : 0xC;
        if (w->state == 1 && (plMoveStickY(p->index, 0) >= 41 ||
                              fabsf(plMoveStickX(p->index, 0)) > 40.0f)) {
            plSound(p, 0x219);
            plVoice(p, gPlSwingLetGoVoice[p->chara]);
        }
    }
    return next;
}

/* Turns the damage box's offset with the lean: 50 across, 100 down. */
void plSwingBoxOffset(Player *p) {
    f32 lean;

    if (mathCos(p->facing) > 0.0f) {
        lean = -p->lean;
    } else {
        lean = p->lean;
    }
    p->dmg->ofs[2] = -mathSin(lean) * 50.0f;
    p->dmg->ofs[1] = -mathCos(-p->lean) * 100.0f;
}

/* The swing (an action). */
void plSwingHang(Player *p, s32 msg, s32 a, s32 b) {
    PlSwing *w = (PlSwing *) p->work;
    s32 next;
    f32 push;
    f32 arg;
    f32 place;
    f32 height;

    switch (msg) {
        case 1:
            next = plSwingStep(p);
            push = -plStickSpeedX(p) * 0.1f;
            plSwingBoxOffset(p);
            if (w->state == 1) {
                plAnimeSet(p, 11);
                w->still = 0;
            } else if (fabsf(push) < 0.05f) {
                if (++p->phase >= 11 && fabsf(mathSin(w->angle)) < 0.01f &&
                    fabsf(mathCos(w->angle)) >= 0.99f) {
                    if (w->still == 0) {
                        plAnimeSet(p, 9);
                        w->still = 1;
                    }
                } else {
                    goto swinging;
                }
            } else {
                p->phase = 0;
            swinging:
                plAnimeSet(p, 10);
                w->still = 0;
                p->animStep = 0.0f;
                if (p->facing < 1.5707962f) {
                    p->model->frame = (2.0f - mathSin(w->angle)) * 3.0f;
                } else {
                    p->model->frame = (2.0f - mathSin(-w->angle)) * 3.0f;
                }
            }
            plActChange(p, next);
            break;
        case 0:
            p->flags |= 0x20004;
            p->velY = 0.0f;
            p->velX = 0.0f;
            p->targetLean = 0.0f;
            p->facing = p->targetFacing;
            place = vramNearestPlaceXZ(p->zone, p->dmg->hitPos[0], p->dmg->hitPos[2], &arg);
            height = p->dmg->hitPos[1];
            w->angle = mathAtan2(p->place - place, p->height - height) + 3.1415927f;
            w->speed = mathSin(w->angle) * 3.0f;
            if (fabsf(w->speed) < 0.001f) {
                w->speed = 0.1f;
            }
            w->energy = 0.0f;
            p->place = place;
            p->height = height;
            p->phase = 0;
            w->state = 0;
            w->still = 0;
            w->box = acollAtkAddYZ((Frame *) pl_atk_root_frame, &p->model->f, 40, 0, 0, 0);
            if (w->box != NULL) {
                plAttackBoxSet(p, w->box, 1);
                w->box->ofs[1] = -60;
            }
            p->dmg->radius = 30;
            p->dmg->height = 100;
            p->dmg->flags &= ~8;
            plInvincibleSet(p, -1);
            plSound(p, 0x21E);
            break;
        case 2:
            plFormSizeSet(p, 0);
            p->dmg->ofs[0] = 0;
            p->dmg->ofs[1] = 0;
            p->dmg->ofs[2] = 0;
            p->dmg->flags |= 8;
            frmDestroy(&w->box->frame);
            plInvincibleSet(p, 0);
            break;
    }
    plSwingDefault(p, msg, a, b);
}

void plSwingAct0A(Player *p, s32 msg, s32 a, s32 b) {
}

void plSwingAct0B(Player *p, s32 msg, s32 a, s32 b) {
}

/* As plSwingBoxOffset, 200 down. */
void plSwingLetGoBoxOffset(Player *p) {
    f32 lean;

    if (mathCos(p->facing) > 0.0f) {
        lean = -p->lean;
    } else {
        lean = p->lean;
    }
    p->dmg->ofs[2] = -mathSin(lean) * 50.0f;
    p->dmg->ofs[1] = -mathCos(-p->lean) * 200.0f;
}

void plSwingLetGo(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    switch (msg) {
        case 1:
            if (plIsOnGround(p)) {
                next = 0x12;
            } else if (plIsInWater(p)) {
                next = 0;
            } else {
                plStickSpeed(p);
                if (p->velY > 0.0f) {
                    p->pushX *= 2.0f;
                    p->pushY = plStickSpeedY(p) * 0.2f;
                }
                plLeanTo(p, 0.1308997f);
            }
            plSwingLetGoBoxOffset(p);
            plActChange(p, next);
            return;
        case 0:
            p->flags |= 0x40008;
            p->dragY = 1.0f;
            p->drag = 1.0f;
            p->targetLean = 0.0f;
            p->dmg->flags &= ~8;
            break;
        case 2:
            p->flags &= ~0x40008;
            p->dragY = 1.0f;
            p->lean = 0.0f;
            p->drag = p->baseDrag;
            plFormSizeSet(p, 0);
            p->dmg->ofs[0] = 0;
            p->dmg->ofs[1] = 0;
            p->dmg->ofs[2] = 0;
            p->dmg->flags |= 8;
            break;
        case 3:
            if (p->actionSteps < 5 && p->hitKind == 0x6A) {
                return;
            }
            break;
    }
    plBaseDefault(p, msg, a, b);
}

void plSwingLetGoBack(Player *p, s32 msg, s32 a, s32 b) {
    plSwingLetGo(p, msg, a, b);
}

void plSwingLetGoFwd(Player *p, s32 msg, s32 a, s32 b) {
    plSwingLetGo(p, msg, a, b);
}

/* A pressed. */
s32 plSwingAPressed(Player *p) {
    return (p->held & A_BUTTON) ? TRUE : FALSE;
}
