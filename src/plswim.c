#include "player.h"
#include "game.h"
#include "plbox.h"
#include "plpick.h"
#include "plshot.h"
#include "plsound.h"
#include "plswim.h"
#include "plutil.h"

/* Swimming (overlay 9): the swim actions' default (entering sets the swimming size, form 2,
 * and flag 0x8000; leaving undoes it), the strokes, the attacks in water (whose sub-weapons
 * cost money), and the swim action picker. */

/* plswim.c's .data (0x801A8870-0x801A8930), in address order. */
Vec3f gPlSwimShotPos[4][2] = { /* per character and kind: where a shot starts */
    { { -2.0f, 10.0f, 2.0f }, { -2.0f, 10.0f, 0.0f } },
    { { -6.0f, 8.0f, 4.0f }, { -6.0f, 15.0f, 1.0f } },
    { { 0.0f, 8.0f, 6.0f }, { 0.0f, 11.0f, 1.0f } },
    { { -2.5f, 12.0f, 10.0f }, { -2.5f, 18.0f, -2.0f } },
};
Vec3f gPlSwimShotVel[2] = { { 0.0f, 0.0f, 5.0f }, { 0.0f, 5.0f, 0.0f } }; /* per kind: its speed */
u8 gPlSwimShotEndPercent[4][2] = { { 75, 70 }, { 65, 70 }, { 60, 60 }, { 40, 45 } }; /* per character and kind: the percent the attack ends at */
f32 gPlSwimShotFrame[4] = { 2.0f, 1.0f, 4.0f, 8.0f }; /* per character: the frame the first shot leaves */
f32 gPlSwimUpShotFrame[4] = { 3.5f, 2.0f, 4.0f, 9.0f }; /* and the second */
s32 gPlSwimAttackTable[2][3] = { { 0x1C, 0x1D, 0x1E }, { 0x1F, 0x20, 0x21 } }; /* the attacks, by charge and weapon level */
extern s32 gPlShotCost[4]; /* per character: what a shot costs */



void plSwimBase(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 1:
            plFaceMoveDir(p, 0.5235988f);
            p->animStepAdd = plPushSize(p) * 0.4f;
            return;
        case 0:
            plFormSizeSet(p, 2);
            p->flags |= 0x8000;
            break;
        case 2:
            plFormSizeSet(p, 0);
            p->flags &= ~0x8000;
            break;
    }
    plBaseDefault(p, msg, a, b);
}

void plSwimPa(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;

    if (msg == 1) {
        plStickSpeed(p);
        next = plSwimPick(p);
        if (next == -1) {
            if (!plSwimPushing(p)) {
                next = 0x1B;
            }
        }
        if (plActChange(p, next)) {
            return;
        }
    }
    plSwimBase(p, msg, a, b);
}

void plSwimIdlePa(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;

    if (msg == 1) {
        plStickSpeed(p);
        next = plSwimPick(p);
        if (next == -1) {
            if (plSwimPushing(p)) {
                next = 0x1A;
            }
        }
        if (plActChange(p, next)) {
            return;
        }
    }
    plSwimBase(p, msg, a, b);
}

void plSwimStartPa(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;

    if (msg == 1) {
        plStickSpeed(p);
        next = plSwimPick(p);
        if (next == -1) {
            if (plAnimEnded(p)) {
                next = 0x18;
            }
        }
        if (plActChange(p, next)) {
            return;
        }
    }
    plSwimBase(p, msg, a, b);
}

void plSwimStopPa(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;

    if (msg == 1) {
        plStickSpeed(p);
        next = plSwimPick(p);
        if (next == -1) {
            if (plAnimEnded(p)) {
                next = 0x19;
            }
        }
        if (plActChange(p, next)) {
            return;
        }
    }
    plSwimBase(p, msg, a, b);
}

/* An attack in water (n: the attack's kind): the swimmer turns the way it moves, the stroke
 * slows to a tenth, and from halfway through the next action may start. */
void plSwimAttack(Player *p, s32 msg, s32 a, s32 b, s32 n) {
    s32 next;
    s32 pct;

    switch (msg) {
        case 0:
            p->flags |= 0x8000;
            plAttackSound(p);
            plBoxSetKind(p, n);
            plChargeRelease(p);
            break;
        case 1:
            next = -1;
            plStickSpeed(p);
            if (plPushSize(p) >= 0.05f) {
                if (p->pushX >= 0.0f) {
                    p->targetFacing = 0.0f;
                } else {
                    p->targetFacing = 3.1415927f;
                }
            }
            plFaceMoveDir(p, 1.5707964f);
            pct = plAnimPercent(p);
            if (pct >= 50) {
                next = plSwimPick(p);
                if (next == -1 && pct == 100) {
                    next = 0x19;
                }
            }
            plActSwitch(p, next);
            p->pushX *= 0.1f;
            p->pushZ *= 0.1f;
            return;
        case 2:
            plBoxDelete(p);
            break;
    }
    plSwimBase(p, msg, a, b);
}

void plSwimAttack0Pa(Player *p, s32 msg, s32 a, s32 b) {
    plSwimAttack(p, msg, a, b, 8);
}

void plSwimAttack1Pa(Player *p, s32 msg, s32 a, s32 b) {
    plSwimAttack(p, msg, a, b, 0x13);
}

void plSwimAttack2Pa(Player *p, s32 msg, s32 a, s32 b) {
    plSwimAttack(p, msg, a, b, 0x1E);
}

void plSwimUpAttack0Pa(Player *p, s32 msg, s32 a, s32 b) {
    plSwimAttack(p, msg, a, b, 9);
}

void plSwimUpAttack1Pa(Player *p, s32 msg, s32 a, s32 b) {
    plSwimAttack(p, msg, a, b, 0x14);
}

void plSwimUpAttack2Pa(Player *p, s32 msg, s32 a, s32 b) {
    plSwimAttack(p, msg, a, b, 0x1F);
}

/* A shot under water: at its frame, the sound, the shot (kind 0 or 1), and its cost. */
void plSwimShoot(Player *p, f32 *frames, s32 kind) {
    switch (p->state) {
        case 0:
            if (frames[p->chara] <= p->model->frame) {
                plAttackVoice(p);
                if (p->chara == 2) {
                    plSound(p, 0x21D);
                }
                plShotFire(p, &gPlSwimShotPos[p->chara][kind], &gPlSwimShotVel[kind]);
                plMoneyAdd(p, gPlShotCost[p->chara]);
                p->state = 1;
            }
            break;
        case 1:
            break;
    }
}

s32 plSwimShotEnded(Player *p, s32 kind) {
    return (plAnimPercent(p) >= gPlSwimShotEndPercent[p->chara][kind]) ? 1 : 0;
}

/* A shooting stroke: the next action once the shot is out. */
s32 plSwimShotNext(Player *p, s32 kind, f32 *frames) {
    s32 next = -1;
    s32 n;

    plSwimShoot(p, frames, kind);
    if (plAnimEnded(p)) {
        next = 0x19;
    } else if (plSwimShotEnded(p, kind)) {
        plStickSpeed(p);
        n = plSwimPick(p);
        if (n == -1 && plSwimPushing(p)) {
            next = 0x18;
        } else {
            next = n;
            p->pushX = 0.0f;
        }
    }
    return next;
}

void plSwimSubWeaponPa(Player *p, s32 msg, s32 a, s32 b) {
    if (msg != 1 || !plActChange(p, plSwimShotNext(p, 0, gPlSwimShotFrame))) {
        plSwimBase(p, msg, a, b);
    }
}

void plSwimUpSubWeaponPa(Player *p, s32 msg, s32 a, s32 b) {
    if (msg != 1 || !plActChange(p, plSwimShotNext(p, 1, gPlSwimUpShotFrame))) {
        plSwimBase(p, msg, a, b);
    }
}

/* The swim action picker (-1: none): an attack by the charge and weapon level, the sub-weapon,
 * or the next stroke. */
s32 plSwimPick(Player *p) {
    s32 next = -1;
    s32 n;

    if (plWantsWeapon(p)) {
        next = gPlSwimAttackTable[plStickForward(p) ? 1 : 0][gSaveData.players[p->index].weaponLevel];
    } else if (plWantsSubWeapon(p)) {
        if (plStickForward(p)) {
            next = 0x23;
        } else {
            next = 0x22;
        }
    } else if (plWantsJump(p)) {
        next = 0xE;
    } else if (!plIsInWater(p)) {
        if (plIsOnGround(p) && p->waterDepth < p->swimDepth) {
            next = plPickWalk(p);
        }
    } else {
        n = plPickOwnMove(p);
        if (n != -1) {
            next = n;
        }
    }
    return next;
}

s32 plSwimPushing(Player *p) {
    return (plPushSize(p) > 0.4f) ? 1 : 0;
}
