#include "player.h"
#include "plattack.h"
#include "plpick.h"
#include "plshot.h"
#include "plsound.h"
#include "plutil.h"

/* The attack actions (overlay 9): each fires the character's shot (plShotFire) or sub-weapon
 * (plSubWeaponFire) once its animation reaches that character's frame, pays for it, and from a
 * percent of the animation on lets the next action start. The throws from a swing (0x7E-0x87)
 * start from the hand for the swing's form. */

/* plattack.c's .data (0x801A8980-0x801A8BA0), in address order. */
s32 gPlShotCost[4] = { -1, -1, -2, -2 }; /* per character: what a shot costs */
s32 gPlSubWeaponCost[4] = { -3, -3, -5, -5 }; /* and a sub-weapon */
Vec3f gPlAttackHandPos[4][6] = { /* per character and form: where it starts */
    { { -1.0f, 10.0f, 2.0f }, { -1.0f, 10.0f, 4.0f }, { -6.0f, 3.0f, -6.0f },
      { -1.0f, 10.0f, 0.0f }, { -1.0f, 13.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
    { { -6.0f, 8.0f, 4.0f }, { -6.0f, 8.0f, 4.0f }, { -6.0f, 4.0f, 4.0f },
      { -6.0f, 15.0f, 1.0f }, { -6.0f, 15.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } },
    { { 0.0f, 8.0f, 6.0f }, { 0.0f, 8.0f, 6.0f }, { 0.0f, 3.0f, 2.0f },
      { 0.0f, 11.0f, 1.0f }, { 0.0f, 11.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } },
    { { -2.5f, 12.0f, 10.0f }, { -2.5f, 12.0f, 8.0f }, { -2.5f, 5.0f, 14.0f },
      { -2.5f, 18.0f, -2.0f }, { -2.5f, 18.0f, -2.0f }, { 0.0f, 0.0f, 0.0f } },
};
f32 gPlSubWeaponFrame[4][6] = { /* per character and form: the frame of the sub-weapon */
    { 2.0f, 1.5f, 4.5f, 3.5f, 3.0f, 0.0f },
    { 1.0f, 2.5f, 1.4f, 2.0f, 1.0f, 0.0f },
    { 4.0f, 4.0f, 10.0f, 4.0f, 4.0f, 0.0f },
    { 8.0f, 0.0f, 10.0f, 9.0f, 1.0f, 0.0f },
};
Vec3f gPlShotVel = { 0.0f, 0.0f, 5.0f }; /* the speed of the shot */
/* per character: the frame the shot leaves, by action */
f32 gPlShotFrameShot[4] = { 2.0f, 1.0f, 4.0f, 8.0f };
f32 gPlShotFrameShotAir[4] = { 1.5f, 2.5f, 4.0f, 0.0f };
f32 gPlShotFrameStand[4] = { 4.5f, 1.4f, 10.0f, 10.0f };
f32 gPlShotFrameFwd[4] = { 3.5f, 2.0f, 4.0f, 9.0f };
f32 gPlShotFrameFwdAir[4] = { 3.0f, 1.0f, 4.0f, 1.0f };
u8 gPlAttackEndPercent[4][6] = { /* per character and form: the percent the attack ends at */
    { 75, 100, 80, 70, 100, 100 },
    { 65, 100, 80, 70, 100, 100 },
    { 60, 100, 80, 60, 100, 100 },
    { 50, 100, 65, 55, 100, 100 },
};
Vec3f gPlSubWeaponVel = { 0.0f, 0.0f, 5.0f }; /* the speed of the sub-weapon */



/* An attack's start: the shot not yet out, the facing kept. */
void plAttackInit(Player *p, s32 msg, s32 a, s32 b) {
    plChargeRelease(p);
    p->state = 0;
    p->facing = p->targetFacing;
}

/* The attacks' default. */
void plAttackDefault(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            plAttackInit(p, msg, a, b);
            plAttackVoice(p);
            /* fallthrough */
        case 2:
        default:
            plBaseDefault(p, msg, a, b);
            break;
        case 1:
        case 4:
            break;
    }
}

/* Fires the shot once the animation reaches the character's frame; whether it did now. */
s32 plAttackFireShot(Player *p, f32 *frames) {
    switch (p->state) {
        case 0:
            if (frames[p->chara] <= p->model->frame) {
                plShotFire(p, &gPlAttackHandPos[p->chara][plAttackForm(p)],
                                       &gPlShotVel);
                plMoneyAdd(p, gPlShotCost[p->chara]);
                if (p->chara == 2 && p->action != 0x80) {
                    plSound(p, 0x21D);
                }
                p->state = 1;
                return 1;
            }
            break;
        case 1:
            break;
    }
    return 0;
}

void plAttackShot(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;
    s32 n;

    switch (msg) {
        case 0:
            if (p->chara == 3) {
                plAttackInit(p, msg, a, b);
                return;
            }
            break;
        case 1:
            if (plAttackFireShot(p, gPlShotFrameShot) && p->chara == 3) {
                plAttackVoice(p);
            }
            if (plAnimEnded(p)) {
                next = plPickWalk(p);
            }
            if (p->state == 1 && plAttackCanEnd(p)) {
                plStickSpeed(p);
                n = plPickGroundOrWalk(p);
                if (n != 0) {
                    next = n;
                }
                p->pushX = 0.0f;
            }
            plActChange(p, next);
            return;
    }
    plAttackDefault(p, msg, a, b);
}

void plAttackShotAir(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    if (msg == 1) {
        plStickSpeed(p);
        if (plAnimEnded(p)) {
            if (p->ground) {
                next = 0;
            } else {
                next = 0xF;
            }
        } else if (p->ground) {
            next = 0;
        }
        plAttackFireShot(p, gPlShotFrameShotAir);
        plActChange(p, next);
        return;
    }
    plAttackDefault(p, msg, a, b);
}

/* The attack on the run (size form 1): the stick across (0.4 of it) past 0.35 keeps running
 * (action 5). */
void plAttackShotStand(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;
    s32 n;
    f32 x;

    switch (msg) {
        case 0:
            plFormSizeSet(p, 1);
            if (p->chara == 2 || p->chara == 3) {
                plAttackInit(p, msg, a, b);
                return;
            }
            break;
        case 1:
            if (plAttackFireShot(p, gPlShotFrameStand) && (p->chara == 2 || p->chara == 3)) {
                plAttackVoice(p);
            }
            if (plAnimEnded(p)) {
                next = 8;
            }
            if (p->state == 1 && plAttackCanEnd(p)) {
                x = plStickSpeedX(p) * 0.4f;
                n = plPickAttackNoFwd(p);
                if (next == -1 && fabsf(x) > 0.35) {
                    n = 5;
                    p->pushX = x;
                }
                if (n != -1) {
                    next = n;
                }
            }
            plActChange(p, next);
            return;
        case 2:
            plFormSizeSet(p, 0);
            break;
    }
    plAttackDefault(p, msg, a, b);
}

void plAttackShotFwd(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;
    s32 n;

    switch (msg) {
        case 0:
            if (p->chara == 3) {
                plAttackInit(p, msg, a, b);
                return;
            }
            break;
        case 1:
            if (plAttackFireShot(p, gPlShotFrameFwd) && p->chara == 3) {
                plAttackVoice(p);
            }
            if (plAnimEnded(p)) {
                next = 0;
            }
            if (p->state == 1 && plAttackCanEnd(p)) {
                plStickSpeed(p);
                n = plPickGroundOrWalk(p);
                if (n != 0) {
                    next = n;
                }
            }
            plActChange(p, next);
            return;
    }
    plAttackDefault(p, msg, a, b);
}

void plAttackShotFwdAir(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    if (msg == 1) {
        plStickSpeed(p);
        plAttackFireShot(p, gPlShotFrameFwdAir);
        if (p->ground) {
            next = plPickLanding(p);
        } else if (plAnimEnded(p)) {
            next = plPickWalk(p);
        }
        plActChange(p, next);
        return;
    }
    plAttackDefault(p, msg, a, b);
}

/* The swing's form (0-4, 5 for none) from the action: 0x7E-0x82 and 0x83-0x87. */
s32 plAttackForm(Player *p) {
    s32 form;

    switch (p->action) {
        case 0x7E:
        case 0x83:
            form = 0;
            break;
        case 0x7F:
        case 0x84:
            form = 1;
            break;
        case 0x80:
        case 0x85:
            form = 2;
            break;
        case 0x81:
        case 0x86:
            form = 3;
            break;
        case 0x82:
        case 0x87:
            form = 4;
            break;
        default:
            form = 5;
            break;
    }
    return form;
}

/* Whether the attack is far enough through for the next action. */
s32 plAttackCanEnd(Player *p) {
    s32 pad[2];
    s32 pct = plAnimPercent(p);

    return (pct >= gPlAttackEndPercent[p->chara][plAttackForm(p)]) ? 1 : 0;
}

/* The sub-weapon's form (0-4, 5 for none): 0x83-0x87. */
s32 plAttackChargedForm(Player *p) {
    s32 form;

    switch (p->action) {
        case 0x83:
            form = 0;
            break;
        case 0x84:
            form = 1;
            break;
        case 0x85:
            form = 2;
            break;
        case 0x86:
            form = 3;
            break;
        case 0x87:
            form = 4;
            break;
        default:
            form = 5;
            break;
    }
    return form;
}

/* Whether the animation has reached the sub-weapon's frame. */
s32 plAttackAtChargedFrame(Player *p) {
    return (gPlSubWeaponFrame[p->chara][plAttackChargedForm(p)] <= p->model->frame) ? 1 : 0;
}

/* Fires the sub-weapon at its frame. */
void plAttackFireCharged(Player *p) {
    switch (p->state) {
        case 0:
            if (plAttackAtChargedFrame(p)) {
                plSubWeaponFire(p, &gPlAttackHandPos[p->chara][plAttackForm(p)],
                                       &gPlSubWeaponVel);
                plMoneyAdd(p, gPlSubWeaponCost[p->chara]);
                p->state = 1;
            }
            break;
        case 1:
            break;
    }
}

void plAttackCharged(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    switch (msg) {
        case 0:
            if (p->chara == 0 || p->chara == 3) {
                plAttackInit(p, msg, a, b);
                return;
            }
            break;
        case 1:
            if (plAnimEnded(p)) {
                next = plPickWalk(p);
            }
            plAttackFireCharged(p);
            plActChange(p, next);
            return;
    }
    plAttackDefault(p, msg, a, b);
}

void plAttackChargedAir(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    switch (msg) {
        case 0:
            if (p->chara == 0 || p->chara == 3) {
                plAttackInit(p, msg, a, b);
                return;
            }
            break;
        case 1:
            plStickSpeed(p);
            if (plAnimEnded(p)) {
                if (p->ground) {
                    next = 0;
                } else {
                    next = 0xF;
                }
            } else if (p->ground) {
                next = 0;
            }
            plAttackFireCharged(p);
            plActChange(p, next);
            return;
    }
    plAttackDefault(p, msg, a, b);
}

void plAttackChargedStand(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            if (p->chara == 0 || p->chara == 3) {
                plAttackInit(p, msg, a, b);
                return;
            }
            break;
        case 1:
            plAttackFireCharged(p);
            if (plAnimEnded(p)) {
                plActChange(p, 8);
                return;
            }
            return;
    }
    plAttackDefault(p, msg, a, b);
}

void plAttackChargedFwd(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            if (p->chara == 0 || p->chara == 3) {
                plAttackInit(p, msg, a, b);
                return;
            }
            break;
        case 1:
            plAttackFireCharged(p);
            if (plAnimEnded(p)) {
                plActChange(p, plPickWalk(p));
                return;
            }
            return;
    }
    plAttackDefault(p, msg, a, b);
}

void plAttackChargedFwdAir(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    switch (msg) {
        case 0:
            if (p->chara == 0 || p->chara == 3) {
                plAttackInit(p, msg, a, b);
                return;
            }
            break;
        case 1:
            plStickSpeed(p);
            plAttackFireCharged(p);
            if (p->ground) {
                next = plPickLanding(p);
            } else if (plAnimEnded(p)) {
                next = plPickWalk(p);
            }
            plActChange(p, next);
            return;
    }
    plAttackDefault(p, msg, a, b);
}
