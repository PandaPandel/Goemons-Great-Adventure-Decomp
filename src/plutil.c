#include "player.h"
#include "game.h"
#include "contpak.h"
#include "mathutil.h"
#include "plmain.h"
#include "plmodel.h"
#include "plqueue.h"
#include "plutil.h"
#include "polygon.h"
#include "vram.h"

/* The player's helpers (overlay 9): changing action (each action is a handler in a table, sent
 * 2 to leave, then 0 to enter), the queue's front end, replies to messages, the stick, speeds,
 * the animation's end, the partner, the two timed flags, the model's place and scale, vectors,
 * money and damage (with the Rumble Pak), and the player's size by form. */


#pragma intrinsic (fabs)

extern void (*gPlActFuncTable[])(Player *p, s32 msg, s32 a, s32 b); /* the actions */
/* plutil.c's .data (0x801A8520-0x801A8720). */
f32 gPlFormSizeX[4][5] = { /* per character and form: the width */
    { 10.0f, 20.0f, 10.0f, 10.0f, 20.0f },
    { 10.0f, 20.0f, 10.0f, 10.0f, 20.0f },
    { 10.0f, 20.0f, 10.0f, 10.0f, 20.0f },
    { 10.0f, 20.0f, 10.0f, 10.0f, 20.0f },
};
f32 gPlFormSizeY[4][5] = { /* the height */
    { 20.0f, 10.0f, 20.0f, 16.0f, 20.0f },
    { 18.0f, 10.0f, 18.0f, 16.0f, 20.0f },
    { 10.0f, 10.0f, 10.0f, 16.0f, 20.0f },
    { 20.0f, 10.0f, 20.0f, 16.0f, 20.0f },
};
f32 gPlFormSizeZ[4][5] = {
    { 10.0f, 10.0f, 10.0f, 10.0f, 10.0f },
    { 10.0f, 10.0f, 10.0f, 10.0f, 10.0f },
    { 10.0f, 10.0f, 10.0f, 10.0f, 10.0f },
    { 10.0f, 10.0f, 10.0f, 10.0f, 10.0f },
};
f32 gPlFormSwimDepth[4][5] = {
    { 18.0f, 10.0f, 11.8f, 16.0f, 4.0f },
    { 19.0f, 10.0f, 13.0f, 16.0f, 4.0f },
    { 16.0f, 10.0f, 10.0f, 16.0f, 4.0f },
    { 20.0f, 10.0f, 15.8f, 16.0f, 4.0f },
};
s16 gPlFormDmgRadius[4][5] = { /* the damage box radius */
    { 40, 80, 40, 40, 40 },
    { 40, 80, 40, 40, 40 },
    { 40, 80, 40, 40, 40 },
    { 40, 80, 40, 40, 40 },
};
s16 gPlFormDmgHeight[4][5] = { /* its height */
    { 160, 80, 160, 128, 160 },
    { 144, 80, 144, 128, 160 },
    { 120, 60, 120, 128, 40 },
    { 160, 80, 160, 128, 60 },
};
s16 gPlFormDmgOfsY[4][5] = { /* its offset up */
    { 0, 0, 0, 0, -80 },
    { 0, 0, 0, 0, -80 },
    { 0, 0, 0, 0, -20 },
    { 0, 0, 0, 0, -20 },
};
s32 gPlWeaponActionTable[5][3] = { /* by weapon level */
    { 0x34, 0x39, 0x3E }, { 0x35, 0x3A, 0x3F }, { 0x36, 0x3B, 0x40 }, { 0x37, 0x3C, 0x41 },
    { 0x38, 0x3D, 0x42 },
};
extern s32 gPlMsgReplies[]; /* per player: the reply to its last message */


s32 plActChange(Player *p, s32 next) {
    if (next == -1) {
        return 0;
    }
    if (next != p->action) {
        return plActSwitch(p, next);
    }
    return 0;
}

/* The same, once the action has run n steps. */
s32 plActChangeAfter(Player *p, s32 next, s32 n) {
    return (p->actionSteps >= n) ? plActChange(p, next) : 0;
}

s32 plActSwitch(Player *p, s32 next) {
    if (next == -1) {
        return 0;
    }
    plActSend(p, 2, 0, 0);
    plActAssign(p, next);
    plActSend(p, 0, 0, 0);
    return 1;
}

void plActAssign(Player *p, s32 next) {
    p->prevAction = p->action;
    p->action = next;
    p->actionSteps = 0;
    p->state = 0;
    p->phase = 0;
    plAnimeSetCurAction(p);
    plFaceAnimeStart(p);
}

s32 plAlwaysTrue(Player *p) {
    return 1;
}

/* The queue's next action, else the default. */
void plActFromQueue(Player *p) {
    s32 next = plActDequeue(p);

    plActChange(p, (next == -1) ? p->defaultAction : next);
}

s32 plActSetDefault(Player *p, s32 action) {
    if (action >= 0) {
        p->defaultAction = action;
    } else {
        PRINTF("paDefault: Illegal action %d. Ignored.\n", action);
        return -1;
    }
    return action;
}

s32 plActEnqueue(Player *p, s32 action) {
    return plQueuePushBack(p, action);
}

s32 plActEnqueueFront(Player *p, s32 action) {
    return plQueuePushFront(p, action);
}

s32 plActDequeue(Player *p) {
    return plQueuePop(p);
}

/* Sends the action's handler a message. */
void plActSend(Player *p, s32 msg, s32 a, s32 b) {
    gPlActFuncTable[p->action](p, msg, a, b);
}

/* Replies to a player's message (the player is passed as a message's word). */
void plMsgReply(s32 a, s32 reply) {
    gPlMsgReplies[((Player *) a)->index] = reply;
}

s32 plMsgReplyTake(Player *p) {
    s32 reply = gPlMsgReplies[p->index];

    gPlMsgReplies[p->index] = 0;
    return reply;
}

f32 plStickSpeedX(Player *p) {
    return plStickToSpeed(p, plMoveStickX(p->index, 0));
}

/* A stick value as a speed: under 5 is none; 1.2 times faster swimming with B held. */
f32 plStickToSpeed(Player *p, s32 v) {
    if (fabs(v) < 5.0) {
        v = 0;
    }
    if ((p->flags & 0x10) && (p->buttons & 0x4000) && plIsOnGround(p)) {
        return v * 0.03f * 1.2f;
    }
    return v * 0.03f;
}

f32 plStickSpeedY(Player *p) {
    return plStickToSpeed(p, plMoveStickY(p->index, 0));
}

/* The speed (z is declared first). */
f32 plSpeedLen(Player *p) {
    f32 z = p->velZ;
    f32 x = p->velX;
    f32 y = p->velY;

    return sqrtf(z * z + (x * x + y * y));
}

/* The push. */
f32 plPushLen(Player *p) {
    f32 z = p->pushZ;
    f32 x = p->pushX;
    f32 y = p->pushY;

    return sqrtf(z * z + (x * x + y * y));
}

/* Whether the animation has ended. */
s32 plAnimEnded(Player *p) {
    if (polAnimeLastFrame(p->model) <= p->model->frame || (p->animStatus & 1)) {
        return 1;
    }
    return 0;
}

/* How far through the animation, in percent. */
s32 plAnimPercent(Player *p) {
    return p->model->frame / polAnimeLastFrame(p->model) * 100.0f;
}

s32 plIsZero(s32 a) {
    if (a == 0) {
        return 1;
    }
    return 0;
}

/* The other player of the first two.
 * The load of the task sits inside the ternary (`(o = ...) != NULL ? PL(o) : NULL`): that keeps
 * the NULL preset in a0 at entry and the `jr ra; move v0,a0` exit. With `o` loaded in its own
 * statement the result goes through v1 on both exits; the index as a ternary inside the subscript
 * puts the lui before the index's `move v0,zero`. */
Player *plOtherPlayer(Player *p) {
    s32 i = (p->index == 0) ? 1 : 0;
    Obj *o;

    return ((o = gGameWork.players[i]) != NULL) ? PL(o) : NULL;
}

Player *plPartnerGet(Player *p) {
    return (Player *) p->partner;
}

void plInvincibleSet(Player *p, s32 n) {
    if (n == 0) {
        p->flags &= ~0x80;
    } else {
        p->flags |= 0x80;
    }
    p->invincibleTimer = n;
}

s32 plIsInvincible(Player *p) {
    return (p->flags & 0x80) ? 1 : 0;
}

void plBlinkSet(Player *p, s32 n) {
    if (n == 0) {
        p->flags &= ~0x100;
    } else {
        p->flags |= 0x100;
    }
    p->blinkTimer = n;
}

void plFramePosCopy(AnimeFrame *a, AnimeFrame *b) {
    a->pos[0] = b->pos[0];
    a->pos[1] = b->pos[1];
    a->pos[2] = b->pos[2];
}

void plModelPosCopy(Player *a, Player *b) {
    plFramePosCopy(a->model, b->model);
}

void plModelOffsetSet(Player *p, f32 x, f32 y, f32 z) {
    p->modelOffset[0] = x;
    p->modelOffset[1] = y;
    p->modelOffset[2] = z;
}

void plModelOffsetZStartSide(Player *p) {
    p->modelOffset[2] = p->startSide;
}

/* The steps a jump takes to come down by h. */
f32 plJumpSpeed(f32 h) {
    return (sqrt(1.1479591f - -8.571428f * h) + 1.0714285f) * 0.5;
}

/* The push's size: across the ground when swimming, else along the course. */
f32 plPushSize(Player *p) {
    f32 r;

    if (p->flags & 0x10) {
        r = sqrtf(p->pushZ * p->pushZ + p->pushX * p->pushX);
    } else {
        r = fabsf(p->pushX);
    }
    return r;
}

void plFrameScaleSet(AnimeFrame *f, f32 s) {
    f->scale[0] = f->scale[1] = f->scale[2] = s;
}

void plFrameScaleMul(AnimeFrame *f, f32 s) {
    f->scale[0] *= s;
    f->scale[1] *= s;
    f->scale[2] *= s;
}

void plFramePosAdd(AnimeFrame *f, Vec3f *v) {
    f->pos[0] += v->x;
    f->pos[1] += v->y;
    f->pos[2] += v->z;
}

void plVecScaleSelf(Vec3f *v, f32 s) {
    v->x *= s;
    v->y *= s;
    v->z *= s;
}

s32 plWeaponAction(Player *p, s32 n) {
    return gPlWeaponActionTable[n][gSaveData.players[p->index].weaponLevel];
}

/* Z held counts up the charge (to 30). */
void plChargeCount(Player *p) {
    if ((p->buttons & 0x2000) && !(p->flags & 0x3020)) {
        if (p->charge < 30) {
            p->charge++;
        }
    }
}

void plChargeRelease(Player *p) {
    if (p->charge >= 30) {
        p->releasedCharge = p->charge;
    }
    p->charge = 0;
}

/* Adds to the player's money; 0 (and none left) if it would go below 0. */
s32 plMoneyAdd(Player *p, s32 n) {
    s32 v = gSaveData.players[p->index].money + n;

    if (v < 0) {
        gSaveData.players[p->index].money = 0;
        return 0;
    }
    gSaveData.players[p->index].money += n;
    return 1;
}

/* Turns v by the course's heading plus a. */
Vec3f *plVecTurnByCourse(Player *p, f32 a, Vec3f *v, Vec3f *out) {
    f32 h = vramCourseHeading(p->zone, p->place) + a;

    out->x = mathSin(h) * v->x - mathCos(h) * v->z;
    out->y = v->y;
    out->z = mathCos(h) * v->x + mathSin(h) * v->z;
    return out;
}

void *plCourseRayCast(s32 zone, f32 *place, Vec3f *d) {
    Vec3f from;
    Vec3f dir;
    Vec3f at;
    f32 t;

    vramCoursePointXZ(zone, place[0], place[2], &from.x, &from.z);
    dir.x = d->x;
    dir.y = d->y;
    dir.z = d->z;
    mathVecRotateY(vramCourseHeading(zone, place[0]) - 1.5707963f, &dir.x, &dir.y,
                           &dir.z);
    return vramRayGround(&from, &dir, &at, &t);
}

f32 plVecDist(Vec3f *a, Vec3f *b) {
    f32 x = a->x - b->x;
    f32 y = a->y - b->y;
    f32 z = a->z - b->z;

    return sqrtf(x * x + y * y + z * z);
}

f32 plVecLen(Vec3f *v) {
    return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
}

f32 plLen2D(f32 a, f32 b) {
    return sqrtf(a * a + b * b);
}

void plVecClamp(Vec3f *v, f32 max) {
    f32 len = plVecLen(v);
    f32 k;

    if (max < len) {
        k = max / len;
        v->x *= k;
        v->y *= k;
        v->z *= k;
    }
}

f32 plVecDot(Vec3f *a, Vec3f *b) {
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

Vec3f *plVecNormalize(Vec3f *out, Vec3f *v) {
    f32 len = plVecLen(v);

    if (fabsf(len) < 0.0001f) {
        out->x = out->y = out->z = 0.0f;
    } else {
        plVecScale(out, v, 1.0f / len);
    }
    return out;
}

Vec3f *plVecAdd(Vec3f *out, Vec3f *a, Vec3f *b) {
    out->x = b->x + a->x;
    out->y = b->y + a->y;
    out->z = b->z + a->z;
    return out;
}

Vec3f *plVecSub(Vec3f *out, Vec3f *a, Vec3f *b) {
    out->x = a->x - b->x;
    out->y = a->y - b->y;
    out->z = a->z - b->z;
    return out;
}

Vec3f *plVecScale(Vec3f *out, Vec3f *v, f32 s) {
    out->x = v->x * s;
    out->y = v->y * s;
    out->z = v->z * s;
    return out;
}

/* Damage: the second row of pips goes first, then the first; a hit with the weapon drops its
 * level (-1 empties everything). The Rumble Pak shakes; 1 when nothing is left. */
s32 plDamage(Player *p, s32 hit, s32 wp) {
    s32 v = gSaveData.players[p->index].armor;
    s32 left = v - hit;

    if (wp == -1) {
        PRINTF("plDamage %d, %d\n", hit, wp);
        gSaveData.players[p->index].weaponLevel = 0;
    } else if (v == 0) {
        v = gSaveData.players[p->index].weaponLevel - wp;
        if (v <= 0) {
            gSaveData.players[p->index].weaponLevel = 0;
        } else {
            gSaveData.players[p->index].weaponLevel = v;
        }
    }
    if (wp == -1) {
        gSaveData.players[p->index].armor = 0;
        gSaveData.players[p->index].life = 0;
        contPakRumbleStart(p->index, -1, 15);
        return 1;
    }
    if (left < 0) {
        gSaveData.players[p->index].armor = 0;
    } else {
        gSaveData.players[p->index].armor = left;
        contPakRumbleStart(p->index, 0x88888888, 15);
        return 0;
    }
    v = gSaveData.players[p->index].life + left;
    if (v <= 0) {
        gSaveData.players[p->index].life = 0;
        contPakRumbleStart(p->index, -1, 15);
        return 1;
    }
    gSaveData.players[p->index].life = v;
    contPakRumbleStart(p->index, 0xAAAAAAAA, 15);
    return 0;
}

/* Ends the power-down (flag 0x200): back to the carried action if carried, with 120 steps of
 * both timed flags. */
void plRevive(Player *p) {
    s32 next = 0;

    p->flags &= ~0x200;
    if (p->flags & 0x10000) {
        if (p->chara == 3) {
            next = 0x47;
        } else if (p->chara == 2) {
            next = 0x56;
        }
    }
    plActSwitch(p, next);
    plInvincibleSet(p, 120);
    plBlinkSet(p, 120);
}

/* The player's size by form (the tables per character). */
void plFormSizeSet(Player *p, s32 form) {
    Vec3f size;

    plFormSizeGet(p, form, &size);
    p->width = size.x;
    p->bodyHeight = size.y;
    p->sizeZ = size.z;
    plFormDmgBoxSet(p, form);
    p->swimDepth = gPlFormSwimDepth[p->chara][form];
}

void plFormSizeGet(Player *p, s32 form, Vec3f *out) {
    out->x = gPlFormSizeX[p->chara][form];
    out->y = gPlFormSizeY[p->chara][form];
    out->z = gPlFormSizeZ[p->chara][form];
}

void plFormDmgBoxSet(Player *p, s32 form) {
    p->dmg->radius = gPlFormDmgRadius[p->chara][form];
    p->dmg->height = gPlFormDmgHeight[p->chara][form];
    p->dmg->ofs[1] = gPlFormDmgOfsY[p->chara][form];
}

/* Sets an attack box's kind and, for a player in play, its owner. */
void plAttackBoxSet(Player *p, ACollFrame *box, u8 kind) {
    box->kind = kind;
    if (p != NULL && plIsInPlay(p->index)) {
        box->power = (s32) gGameWork.players[p->index];
    }
}
