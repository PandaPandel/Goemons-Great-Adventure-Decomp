#include "player.h"
#include "enecommon.h"
#include "mathutil.h"
#include "pldefault.h"
#include "plmove.h"
#include "swwork.h"
#include "tmanager.h"

/* Every action's default message handler (overlay 9), after plBaseDefault: while walking, a
 * movable object walked into that answers "push" starts the push (action 0x25); a hit of kind
 * 0x65 is someone to talk to: B (or A as well, once item 10 is had and item 0xA3 is not)
 * pressed while facing it within 45 degrees asks it (message 8) and, if it agrees, starts the
 * talk (action 0xCA). */



void plActBaseDefault(Player *p, s32 msg, s32 a, s32 b) {
    plBaseDefault(p, msg, a, b);
}

void plActDefault(Player *p, s32 msg, s32 a, s32 b) {
    PlAsk ask;
    u16 pressed;
    u16 mask;
    Obj *obj;
    s32 pad;
    u16 prev;
    u16 now;

    switch (msg) {
        case 1:
            plFaceMoveDir(p, 0.5235988f);
            obj = (Obj *) p->wallObj;
            if (obj != NULL && plSendTagMsg(obj, 0x70757368, 0, 0) == 0x70757368 &&
                (p->action == 1 || p->action == 2 || p->action == 3 || p->action == 4)) {
                if (plActChange(p, 0x25)) {
                    return;
                }
            }
            break;
        case 3:
            if (p->hitKind == 0x65) {
                if (p->action == 0 || p->action == 1 || p->action == 2 || p->action == 3 || p->action == 4) {
                    now = plMoveGetButtons(p->index, 0);
                    prev = plMoveGetButtons(p->index, 1);
                    pressed = (plMoveGetButtons(p->index, 0) & prev) ^ now;
                    mask = 0x4000;
                    if (!swWorkTest(10) || swWorkTest(0xA3)) {
                        mask = 0xC000;
                    }
                    p->flags |= 0x1000;
                    if ((pressed & mask) && plIsFacingObj(p, (Obj *) p->hitBy)) {
                        ask.who = gGameWork.players[p->index];
                        ask.stage = 0;
                        if (tskSendMsg((Obj *) p->hitBy, 8, &ask)) {
                            p->flags |= 0x2000;
                            plActChange(p, 0xCA);
                            p->work[1] = p->hitBy;
                        }
                    }
                }
                break;
            }
            /* fallthrough */
        default:
            plBaseDefault(p, msg, a, b);
            break;
        case 4:
            break;
    }
}

/* Whether the player faces obj within 45 degrees. */
s32 plIsFacingObj(Player *p, Obj *obj) {
    Vec3f pos;

    eneGetWorldPos(obj, &pos);
    return (cosf(mathAtan2(pos.x - p->model->pos[0], pos.z - p->model->pos[2]) - p->model->rot[1]) >=
            0.7071067811865476)
               ? 1
               : 0;
}
