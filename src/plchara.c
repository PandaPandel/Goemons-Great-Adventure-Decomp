#include "player.h"
#include "gfxmgr.h"
#include "mathutil.h"
#include "plchara.h"

/* Two characters' own jumps (overlay 9's first file). Character 0 jumps 20 high. Character 1
 * drops with an attack box under it. Landing on flat ground squashes it and bounces it back
 * up 20 above where it started, stretching; landing on a slope slides it off. */

/* A bounce's work (Player.work). */
typedef struct PlBounce {
    /* 0x0 */ s32 done; /* the stretch has played */
    /* 0x4 */ s32 timer; /* before the drop */
    /* 0x8 */ ACollFrame *box; /* its attack box */
    /* 0xC */ s32 count; /* steps falling */
} PlBounce;

extern ACollFrame *pl_atk_root_frame; /* the player attacks' boxes */


/* Character 0's jump: 20 high, from where it stands. (The root is named: inline, IDO puts it
 * first in the add.) */
void plChara0Jump(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;
    f64 r;

    switch (msg) {
        case 0:
            /* (15/14 + sqrt((15/14)^2 + 20 * 120/14)) / 2: the speed that climbs 20 */
            r = sqrt(172.57652f);
            p->jumpPush = (1.0714285f + r) * 0.5 - p->velY;
            p->flags |= 0x4000;
            plSound(p, 0x547);
            break;
        case 1:
            plStickSpeed(p);
            if (plIsOnGround(p)) {
                next = 0;
            } else if (plIsInWater(p)) {
                next = 0x19;
            } else if (p->velY < 0.0f) {
                next = plPickAirAttack(p);
                if (next == -1) {
                    next = 0x15;
                }
            } else {
                next = plPickAirAttack(p);
            }
            plActChange(p, next);
            return;
        case 2:
            p->flags &= ~0x4000;
            break;
    }
    plActDefault(p, msg, a, b);
}

/* Character 1's bounce. (Two pad slots under next; state 3's fall test is written with an int
 * 0, so IDO does not share state 1's 0.0f in f0 with it.) */
void plChara1Bounce(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;
    s32 pad[2];
    PlBounce *w = (PlBounce *) p->work;
    f32 r;

    switch (msg) {
        case 0:
            p->flags |= 0x4000;
            w->timer = 5;
            w->box = NULL;
            if (p->flags & 0x10) {
                p->velX = 0.0f;
                p->pushX = 0.0f;
                p->velZ = 0.0f;
                p->pushZ = 0.0f;
            } else {
                p->pushX = -p->velX;
            }
            p->baseGravity = 0.0f;
            p->jumpPush = -p->velY;
            break;
        case 1:
            switch (p->state) {
                case 0:
                    if (--w->timer == 0) {
                        p->state = 1;
                        w->count = 0;
                        p->pushY = -6.0f;
                        p->baseGravity = -1.0714285f;
                        w->box = acollAtkAddYZ((Frame *) pl_atk_root_frame, &p->model->f, 0x50, 0,
                                                        0, 0);
                        if (w->box != NULL) {
                            plAttackBoxSet(p, w->box, 0x11);
                        }
                        plInvincibleSet(p, -1);
                    }
                    break;
                case 1:
                    if (plIsOnGround(p)) {
                        if (fabsf(p->slope) < 0.62831855f) {
                            plSetPhase(p, 0x80);
                            w->done = 0;
                            plAnimeSet(p, 20);
                            p->state = 2;
                            plSound(p, 0x244);
                        } else {
                            next = 0xF;
                            if (!(p->flags & 0x10)) {
                                p->pushX = mathCos(p->slope) * 0.5f;
                            }
                        }
                    } else if (p->velY >= 0.0f) {
                        next = 0xF;
                    }
                    if (++w->count >= 121) {
                        next = 0;
                    }
                    break;
                case 2:
                    if (plSquashStretch(p, 0x40, -1.0f, 0x200)) {
                        if (plIsOnGround(p)) {
                            p->state = 3;
                            r = sqrtf(1.147959113f - (p->jumpHeight + 20.0f) * -8.571428f);
                            p->jumpPush = (1.0714285f + r) * 0.5f - p->velY;
                            if (w->box != NULL) {
                                frmDestroy(&w->box->frame);
                            }
                            w->box = NULL;
                            plInvincibleSet(p, 15);
                        } else {
                            next = 0xF;
                        }
                    }
                    break;
                case 3:
                    plStickSpeed(p);
                    if (w->done == 0 && plSquashStretch(p, 0x50, 1.0f, 0x400)) {
                        w->done = 1;
                    }
                    if (plIsOnGround(p)) {
                        next = 0;
                    } else if (p->velY < 0) {
                        next = 0x16;
                    }
                    break;
            }
            plActChange(p, next);
            return;
        case 2:
            p->flags &= ~0x4000;
            plResetScale(p);
            p->baseGravity = -1.0714285f;
            if (w->box != NULL) {
                frmDestroy(&w->box->frame);
            }
            plInvincibleSet(p, 0);
            break;
    }
    plActDefault(p, msg, a, b);
}

void plSetPhase(Player *p, s32 phase) {
    p->phase = phase;
}

/* The model back to its size. */
void plResetScale(Player *p) {
    p->model->scale[0] = 0.1f;
    p->model->scale[1] = 0.1f;
    p->model->scale[2] = 0.1f;
}

/* Squashes (dir -1) or stretches (dir 1) the model by the sine of the phase, and moves the
 * phase on by step. 1 when it passes mark. */
s32 plSquashStretch(Player *p, s32 step, f32 dir, s32 mark) {
    f32 c;
    f32 d;
    s32 ret = 0;

    d = fabsf(mathSinCos(p->phase, &c)) * 0.05f * dir;
    p->model->scale[1] = 0.1f + d;
    p->model->scale[0] = p->model->scale[2] = 0.1f - d;
    if (p->phase < mark && p->phase + step >= mark) {
        ret = 1;
    }
    p->phase += step;
    p->phase &= 0x3FF;
    return ret;
}

/* The jump for the character; the others' go to action 0xE. */
void plCharaJump(Player *p, s32 msg, s32 a, s32 b) {
    switch (p->chara) {
        case 0:
            plChara0Jump(p, msg, a, b);
            return;
        case 1:
            plChara1Bounce(p, msg, a, b);
            return;
        default:
            plActChange(p, 0xE);
            plActDefault(p, msg, a, b);
            return;
    }
}

/* Character 0 takes plPickAir's action, but stays (-1) instead of going to 0xF while
 * unk070 >= -5; the others go to 0xF. (The else is hoisted above the chara test, leaving the
 * then-block's `b` to the next instruction with next = -1 in its slot.) */
void plChara0JumpFall(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;

    switch (msg) {
        case 1:
            if (p->chara == 0) {
                next = plPickAir(p);
                if (next == 0xF && p->velY >= -5.0f) {
                    next = -1;
                }
            } else {
                next = 0xF;
            }
            plActChange(p, next);
            break;
        case 0:
            p->flags |= 0x4000;
            break;
        case 2:
            p->flags &= ~0x4000;
            break;
    }
    plActDefault(p, msg, a, b);
}

/* Character 1 only. */
void plChara1BounceFall(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;

    if (p->chara != 1) {
        return;
    }
    switch (msg) {
        case 1:
            next = -1;
            plStickSpeed(p);
            if (plIsOnGround(p)) {
                next = 0x12;
            } else if (plWantsWeapon(p)) {
                if (plStickForward(p)) {
                    next = plWeaponAction(p, 4);
                } else {
                    next = plWeaponAction(p, 1);
                }
            } else if (plWantsSubWeapon(p)) {
                if (plStickForward(p)) {
                    next = 0x82;
                } else {
                    next = 0x7F;
                }
            } else if (plIsInWater(p)) {
                next = 0x19;
            }
            plActChange(p, next);
            break;
        case 0:
            p->flags |= 0x4000;
            break;
        case 2:
            p->flags &= ~0x4000;
            break;
    }
    plActDefault(p, msg, a, b);
}

/* 1 if the character's jump can start from what it is doing. */
s32 plCanCharaJump(Player *p) {
    switch (p->chara) {
        case 0:
            if (p->action == 0x10) {
                if (p->held & A_BUTTON) {
                    return TRUE;
                }
            } else if (p->action == 0xE && (p->held & A_BUTTON) && p->actionSteps >= 3) {
                return TRUE;
            }
            break;
        case 1:
            if (p->flags & 0x10) {
                if (!(p->held & Z_TRIG)) {
                    return FALSE;
                }
            } else if (plStickToSpeed(p, plMoveStickY(p->index, 0)) >= -1.8f) {
                return FALSE;
            }
            if (p->action == 0x10 || p->action == 0xF || (p->action == 0xE && p->actionSteps >= 3)) {
                return TRUE;
            }
            break;
    }
    return FALSE;
}
