#include "player.h"
#include "plqueue.h"

/* The player's queue of actions to take next (overlay 9): three at Player+0xC, -1 for none. */

void plQueueClear(Player *p) {
    s32 i;

    for (i = 0; i < 3; ) {
        p->queue[i++] = -1;
    }
}


/* Adds an action at the back; 0 when the queue is full. */
s32 plQueuePushBack(Player *p, s32 action) {
    s32 i;

    for (i = 0; i < 3; i++) {
        if (p->queue[i] == -1) {
            p->queue[i] = action;
            return 1;
        }
    }
    return 0;
}

/* Adds an action at the front, pushing out the last; 0 when that one was taken. */
s32 plQueuePushFront(Player *p, s32 action) {
    s32 ok;
    s32 i;

    ok = 1;
    if (p->queue[2] != -1) {
        ok = 0;
    }
    for (i = 2; i > 0; i--) {
        p->queue[i] = p->queue[i - 1];
    }
    p->queue[0] = action;
    return ok;
}

/* Takes the first action (-1 for none). */
s32 plQueuePop(Player *p) {
    s32 i;
    s32 action;

    for (i = 0; i < 3; i++) {
        action = p->queue[i];
        if (action != -1) {
            for (; i < 2; i++) {
                p->queue[i] = p->queue[i + 1];
            }
            p->queue[2] = -1;
            return action;
        }
    }
    return -1;
}
