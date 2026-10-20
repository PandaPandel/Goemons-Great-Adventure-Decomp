#include "player.h"
#include "plpick.h"
#include "plpush.h"
#include "plsound.h"

/* Pushing and pulling (overlay 9): the player against an object that can be moved (Player+0x178,
 * found by the wall probes). Pushing tells the object "push" and pulling "pull", with the
 * player's speed (times 100); the actions 0x24-0x2A are the stances and steps. The way the
 * player faces gives the direction: 1 facing along the course (0), -1 facing back (pi). */



/* The push actions' default: every message but 1 goes to the player's default. */
void plPushDefault(Player *p, s32 msg, s32 a, s32 b) {
    if (msg == 0 || msg != 1) {
        plBaseDefault(p, msg, a, b);
    }
}

/* Pushes: the object gets "push" with the speed, at most 3.4 (none when moving back). */
void plPushSendPush(Player *p, f32 dir) {
    f32 v = p->pushX;
    s32 obj;

    if (v * dir < 0.0f) {
        v = 0.0f;
    } else if (fabsf(v) > 3.4f) {
        v = 3.4f * dir;
    }
    p->pushX = 1.2f * dir;
    if ((obj = p->wallObj) != 0) {
        plSendTagMsg((Obj *) obj, 0x70757368, v * 100.0f, 0);
    }
}

/* Pulls: "pull", the other way. */
void plPushSendPull(Player *p, f32 dir) {
    f32 v = p->pushX;
    s32 obj;

    if (v * dir > 0.0f) {
        v = 0.0f;
    } else if (fabsf(v) > 3.4f) {
        v = -3.4f * dir;
    }
    p->pushX = 1.2f * dir;
    if ((obj = p->wallObj) != 0) {
        plSendTagMsg((Obj *) obj, 0x70756C6C, v * 100.0f, 0);
    }
}

/* Pushing (B held keeps pushing; 0x27 when the push is going back fast). */
void plActPush(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;
    f32 dir;

    switch (msg) {
        case 1:
            dir = 1.0f - p->facing * 0.3183099f * 2;
            plStickSpeed(p);
            if (p->wallObj == 0) {
                plActChange(p, 0x26);
                return;
            }
            next = plPickGround(p);
            if (p->buttons & 0x4000) {
                next = (p->pushX * dir < -1.2f) ? 0x27 : 0x24;
                plPushSendPush(p, dir);
            } else if (p->pushX * dir < -1.2f) {
                next = plPickWalk(p);
            } else if (p->pushX * dir < 0.05f) {
                next = 0x26;
            } else {
                plPushSendPush(p, dir);
            }
            plActChange(p, next);
            return;
        case 0:
        case 2:
        default:
            plPushDefault(p, msg, a, b);
            return;
    }
}

/* Standing against the object: faces the way it moves on entering. */
void plActPushStand(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;
    f32 dir;

    switch (msg) {
        case 0:
            p->targetFacing = p->facing = (p->velX >= 0.0f) ? 0.0f : 3.1415927f;
            plPushSound(p);
            break;
        case 1:
            dir = 1.0f - p->facing * 0.3183099f * 2;
            plStickSpeed(p);
            next = plPickGround(p);
            if (p->buttons & 0x4000) {
                if (p->pushX * dir < -1.2f) {
                    next = 0x27;
                } else {
                    next = plAnimEnded(p) ? 0x24 : 0x25;
                }
                p->pushX = 1.7f * dir;
            } else if (next == -1) {
                if (plAnimEnded(p)) {
                    next = 0x24;
                } else if (p->pushX * dir < -1.2f) {
                    next = plPickWalk(p);
                } else if (p->pushX * dir < 0.05f) {
                    next = 0x26;
                }
            }
            if (!plActChange(p, next)) {
                plBaseDefault(p, msg, a, b);
            }
            return;
        case 2:
            break;
    }
    plPushDefault(p, msg, a, b);
}

void plActPushEnd(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;

    if (msg == 1) {
        plStickSpeed(p);
        next = plPickGround(p);
        if (next == -1) {
            next = plPickWalk(p);
            if (next == 0) {
                if (!plAnimEnded(p)) {
                    next = -1;
                }
            }
        }
        if (plActChange(p, next)) {
            return;
        }
    }
    plPushDefault(p, msg, a, b);
}

void plActPushToPull(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;
    f32 dir;

    if (msg == 1) {
        dir = 1.0f - p->facing * 0.3183099f * 2;
        plStickSpeed(p);
        next = plPickGround(p);
        if (p->buttons & 0x4000) {
            if (p->pushX * dir < -1.2f) {
                if (plAnimEnded(p)) {
                    next = 0x29;
                } else {
                    next = 0x27;
                }
            } else {
                next = 0x24;
            }
            p->pushX = 1.7f * dir;
        } else if (p->pushX * dir < 0.05f) {
            next = plPickWalk(p);
            if (next == 0) {
                next = 0x26;
            }
        }
        if (plActChange(p, next)) {
            return;
        }
    }
    plPushDefault(p, msg, a, b);
}

void plActPullToPush(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;
    f32 dir;

    if (msg == 1) {
        dir = 1.0f - p->facing * 0.3183099f * 2;
        plStickSpeed(p);
        plPickGround(p);
        if (p->buttons & 0x4000) {
            next = plAnimEnded(p) ? 0x24 : 0x28;
            p->pushX = 1.7f * dir;
        } else {
            next = plPickWalk(p);
            if (next == 0) {
                next = 0x2A;
            }
        }
        if (plActChange(p, next)) {
            return;
        }
    }
    plPushDefault(p, msg, a, b);
}

/* Pulling (B held). */
void plActPull(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;
    f32 dir;

    switch (msg) {
        case 1:
            dir = 1.0f - p->facing * 0.3183099f * 2;
            plStickSpeed(p);
            if (p->wallObj == 0) {
                plActChange(p, 0x2A);
                return;
            }
            plPickGround(p);
            if (p->buttons & 0x4000) {
                next = (p->pushX * dir > 1.2f) ? 0x28 : 0x29;
                plPushSendPull(p, dir);
            } else {
                next = plPickWalk(p);
                if (next == 0) {
                    next = 0x2A;
                }
            }
            plActChange(p, next);
            return;
        case 0:
        case 2:
        default:
            plPushDefault(p, msg, a, b);
            return;
    }
}

void plActPullEnd(Player *p, s32 msg, s32 a, s32 b) {
    s32 next;

    if (msg == 1) {
        plStickSpeed(p);
        next = plPickGround(p);
        if (next == -1) {
            if (plAnimEnded(p)) {
                next = plPickWalk(p);
            }
        }
        if (plActChange(p, next)) {
            return;
        }
    }
    plPushDefault(p, msg, a, b);
}
