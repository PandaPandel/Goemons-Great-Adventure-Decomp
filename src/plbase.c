#include "player.h"
#include "ombupl.h"
#include "plbase.h"
#include "plpick.h"

/* The player's base ("plBase", overlay 9): standing, walking and running, and the piggyback
 * request (message 4, "PM_OMBU"). Another player asks with a = 0: the answer is no while the
 * player is busy (flag 0x10), else yes, and the player climbs on. a = 1 ("GETOFF") is refused
 * here. */


void plBase(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            p->facing = p->targetFacing;
            plFormSizeSet(p, 1);
            p->flags |= 0x100000;
            break;
        case 2:
            plFormSizeSet(p, 0);
            p->flags &= ~0x100000;
            break;
        case 4:
            switch (a) {
                case 0:
                    PRINTF("plBase: caught PM_OMBU (sender:%d)\n", b);
                    PRINTF("FUSE_BASE: caught PM_OMBU ");
                    if (p->flags & 0x10) {
                        PRINTF(" rejected\n");
                        plMsgReply(b, 0);
                        return;
                    }
                    PRINTF(" accept\n");
                    plMsgReply(b, 1);
                    plActChange(p, ombuUStart(p, b));
                    return;
                case 1:
                    PRINTF("plBase: PM_OMBU: 不正な GETOFF メッセージを受け取りました\n");
                    plMsgReply(b, 0);
                    break;
                default:
                    PRINTF("plBase: OMBU: invalid param\n");
                    break;
            }
            break;
    }
    plBaseDefault(p, msg, a, b);
}

/* The stick moves the player: across and up while flag 0x10 is set, else across only. */
void plBaseStickSpeed(Player *p) {
    if (p->flags & 0x10) {
        p->pushZ = plStickSpeedX(p) * 0.4f;
        p->pushX = plStickSpeedY(p) * 0.4f;
    } else {
        p->pushX = plStickSpeedX(p) * 0.4f;
    }
}

/* Standing: walks off above 0.35 (action 5), else stands (8). */
s32 plBaseStandNext(Player *p) {
    s32 next;
    s32 v;

    plBaseStickSpeed(p);
    if (plPushSize(p) > 0.35) {
        next = 5;
    } else {
        next = 8;
        p->pushX = p->pushZ = 0.0f;
    }
    if (plWantsJump(p) && plNoGroundAhead(p)) {
        next = 0xC2;
    } else if ((v = plPickAttackNoFwd(p)) != -1) {
        next = v;
    } else if (!plWantsTurnBack(p) && plNoGroundAhead(p)) {
        next = 7;
    }
    return plActChange(p, next);
}

/* Slowing down (0.9 a step); A picks the next action. */
s32 plBaseSlowDown(Player *p) {
    s32 next;

    plStickSpeed(p);
    p->pushX *= 0.9f;
    p->pushZ *= 0.9f;
    if (!(p->buttons & A_BUTTON)) {
        next = 0;
    } else {
        next = plPickGround(p);
        if (next == -1 || next == 0xE || next == 6) {
            next = plBaseWalkAction(p);
        }
    }
    return plActChange(p, next);
}

/* The walk's action by speed (0.05, 0.7, 1.2 and 1.7); -1 if it is the one already on. */
s32 plBaseWalkAction(Player *p) {
    f32 speed = plPushSize(p);
    s32 next;

    if (speed < 0.05f) {
        next = 0xC2;
    } else if (speed < 0.7f) {
        next = 0xC3;
    } else if (speed < 1.2f) {
        next = 0xC4;
    } else if (speed < 1.7f) {
        next = 0xC5;
    } else {
        next = 0xC6;
    }
    return (next == p->action) ? -1 : next;
}
