#include "player.h"
#include "enecommon.h"
#include "mathutil.h"
#include "plmain.h"
#include "plpick.h"
#include "pltalk.h"
#include "plutil.h"
#include "tmanager.h"
#include "vram.h"

/* The talk (overlay 9, action 0xCA; pldefault.c starts it): the player turns to face whoever it
 * talks to (work[1], the task), tells it so (message 8 with 2), then, each step, that it is
 * still there (8 with 3); while the partner answers, work[0] is 3, and once it stops answering
 * the talk lasts three more steps. Message 5 with 2 ends it. The other players are halted
 * (flag 0x40) for as long as it lasts. */



void plTalkBaseProc(Player *p, s32 msg, s32 a, s32 b) {
    PlAsk ask;
    s32 *w;

    switch (msg) {
        case 1:
            w = p->work;
            if (w) {
                ask.who = gGameWork.players[p->index];
                ask.stage = 3;
                if (tskSendMsg((Obj *) w[1], 8, &ask)) {
                    w[0] = 3;
                } else if (w[0] != 0) {
                    if (w[0] >= 4) {
                        w[0] = 3;
                    }
                    w[0]--;
                } else if (plActChange(p, 0)) {
                    PRINTF("TALK_BASE: timed out\n");
                    return;
                }
            }
            break;
        case 3:
            break;
        default:
            plBaseDefault(p, msg, a, b);
            break;
    }
}

void plTalkProc(Player *p, s32 msg, s32 a, s32 b) {
    s32 pad[2];
    Vec3f pos;
    PlAsk ask;
    s32 r;
    s32 *w;

    plOtherPlayer(p);
    switch (msg) {
        case 1:
            switch (p->state) {
                case 0:
                    p->state = 1;
                    eneGetWorldPos((Obj *) p->work[1], &pos);
                    p->targetFacing = mathAtan2(pos.x - p->model->pos[0], pos.z - p->model->pos[2]);
                    if (!(p->flags & 0x10)) {
                        p->targetFacing -= vramCourseHeading(p->zone, p->place);
                    } else {
                        p->facing = p->model->rot[1];
                    }
                    p->state = 1;
                    /* fallthrough */
                case 1:
                    r = plTurnTo(p, 0.7853982f);
                    if (p->flags & 0x10) {
                        p->model->rot[1] = p->facing;
                    }
                    if (r) {
                        PRINTF("pACT_talk(%d): send ENEPL_TALK_STANDBY\n", p->index);
                        ask.who = gGameWork.players[p->index];
                        ask.stage = 2;
                        tskSendMsg((Obj *) p->work[1], 8, &ask);
                        p->state = 2;
                    }
                    break;
                case 2:
                    break;
                case 3:
                    plActChange(p, 0);
                    break;
            }
            break;
        case 5:
            if (*(s32 *) a == 2) {
                p->state = 3;
            }
            return;
        case 0:
            w = p->work;
            plTalkHaltOthers(p);
            w[1] = 0;
            w[0] = 3;
            break;
        case 2:
            plTalkReleaseOthers(p);
            p->flags &= ~0x3000;
            break;
    }
    plTalkBaseProc(p, msg, a, b);
}

/* Halts (or lets go) every other player in play that is not flagged 0x10000000. */
void plTalkSetOthersHalt(Player *p, s32 halt) {
    s32 i;
    Player *q;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            q = PL(gGameWork.players[i]);
            if (i != p->index && !(q->flags & 0x10000000)) {
                if (halt) {
                    plHalt(gGameWork.players[i]);
                } else {
                    plContinue(gGameWork.players[i]);
                }
            }
        }
    }
}

void plTalkHaltOthers(Player *p) {
    if (!(p->flags & 0x40)) {
        PRINTF("plHaltOtherPlayers(%d)\n", p->index);
        plTalkSetOthersHalt(p, 1);
    } else {
        PRINTF("自分がHALTのときは禁止\n");
    }
}

void plTalkReleaseOthers(Player *p) {
    PRINTF("plContinueOtherPlayers(%d)\n", p->index);
    plTalkSetOthersHalt(p, 0);
    PRINTF("");
}
