#include "player.h"
#include "enemy.h"
#include "fade.h"
#include "file.h"
#include "gfxmgr.h"
#include "plbase.h"
#include "plchain.h"
#include "pllimit.h"
#include "plmain.h"
#include "plshot.h"
#include "plutil.h"
#include "tmanager.h"
#include "vram.h"

/* The chain ("PL鎖", overlay 9). Its actions throw a chain task (0x1A0 bytes, under the player's
 * task) and wait for it: message 6 with the chain's tag says it has caught something. */

/* What the chain task is made with. */
typedef struct PlChainInit {
    /* 0x00 */ Player *p;
    /* 0x04 */ Vec3f *a; /* where it starts, off the model (plPlaceTurned pos) */
    /* 0x08 */ Vec3f *b; /* its speed (plPlaceTurned vel) */
    /* 0x0C */ s32 unusedC; /* set to 0, never read */
    /* 0x10 */ s32 unused10; /* likewise */
    /* 0x14 */ s32 unused14;
} PlChainInit;

#define CHAIN_TAG 0xBABFBBE0 /* message 6's tag from the chain: "鎖死" in EUC-JP */

/* A link of the chain. */
typedef struct ChainLink {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ ACollFrame *box;
    /* 0x08 */ Vec3f vel; /* the step this frame */
    /* 0x14 */ Vec3f move;
    /* 0x20 */ f32 unused20; /* set to 0, never read */
} ChainLink; /* size 0x24 */

/* The chain task's work (0x1A0 bytes with the header). */
typedef struct ChainWork {
    /* 0x00 */ ChainLink links[6]; /* the hand's end last */
    /* 0xD8 */ AnimeFrame *tip;
    /* 0xDC */ Vec3f base;
    /* 0xE8 */ ACollFrame *hook;
    /* 0xEC */ Obj *target; /* what it caught */
    /* 0xF0 */ s32 caught;
    /* 0xF4 */ s32 player;
    /* 0xF8 */ s32 time;
    /* 0xFC */ u32 state; /* 0 out, 2 struck, 3 back, 5 home, 6 holding */
} ChainWork;

extern u8 plChainLinkAnim[]; /* the link's and the tip's animations */
extern u8 plChainFirstLinkAnim[];
extern u8 plChainTipAnim[];
extern Frame *gFrmModelLayer;
extern s16 gFrmKindAnime; /* the "anime" kind */
extern Frame *pl_atk_root_frame; /* the attack boxes' parent */


Vec3f gPlChainStartPos = { 0.0f, 4.0f, 16.0f }; /* the start: 4 up, 16 ahead of the model */
Vec3f gPlChainVel = { 0.0f, 0.0f, 22.5f }; /* the speed: 22.5 ahead */
Vec3f gPlChainStartPos62 = { 0.0f, 2.0f, 16.0f }; /* the start for action 0x62: 2 up */

/* Throws the chain. */
Obj *plChainThrow(Player *p) {
    PlChainInit init;

    init.p = p;
    init.a = &gPlChainStartPos;
    if (p->action == 0x62) {
        init.a = &gPlChainStartPos62;
    }
    init.b = &gPlChainVel;
    init.unusedC = 0;
    init.unused10 = 0;
    return p->chain = fadeCreateRetry(gGameWork.players[p->index],
                                             plChainProc, 3, 0x1A0, &init);
}

/* The chain actions' default. */
void plChainDefault(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 1:
            return;
        case 3:
            if (plIsInvincible(p) && p->hitKind >= 0x76) {
                return;
            }
            if (p->chain != NULL) {
                plSendTagMsg(p->chain, 0xBEC3CCC7 /* 消滅 */, 0, 0);
            }
            break;
    }
    plBaseDefault(p, msg, a, b);
}

void plChainStart5C(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            p->chain = NULL;
            p->facing = p->targetFacing;
            plSound(p, 0x20D);
            plSound(p, 0x20B);
            break;
        case 1:
            if (plAnimEnded(p)) {
                plActChange(p, 0x5E);
            }
            break;
    }
    plChainDefault(p, msg, a, b);
}

/* The explicit "default: break;" keeps p in its stack home, as the game does: the extra block
 * lowers p's priority. Without it IDO puts p in s0. plChainWait64 is its twin. */
void plChainWait5E(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            if (plChainThrow(p) == NULL) {
                plActAssign(p, 0x5D);
            }
            break;
        case 1:
            break;
        case 6:
            if (*(s32 *) a == CHAIN_TAG) {
                plActAssign(p, 0x5D);
                p->chain = NULL;
                plMsgReply(b, 1);
                return;
            }
            break;
        default:
            break;
    }
    plChainDefault(p, msg, a, b);
}

void plChainEnd5D(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;
    s32 t;

    if (msg == 1) {
        t = plAnimPercent(p);
        if (t > 32) {
            plStickSpeed(p);
            next = plPickGroundOrWalk(p);
            if (next == 0) {
                next = -1;
            }
            if (next == -1 && t == 100) {
                next = 0;
            }
        }
        plActChange(p, next);
    }
    plChainDefault(p, msg, a, b);
}

void plChainStart5F(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            p->chain = NULL;
            p->facing = p->targetFacing;
            plSound(p, 0x20D);
            plSound(p, 0x20B);
            break;
        case 1:
            plStickSpeed(p);
            if (plAnimEnded(p)) {
                plActChange(p, 0x61);
            }
            break;
    }
    plChainDefault(p, msg, a, b);
}

void plChainWait61(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            if (plChainThrow(p) == NULL) {
                plActAssign(p, 0x5D);
            }
            break;
        case 1:
            if (!plIsOnGround(p)) {
                plStickSpeed(p);
                if (plPushSize(p) >= 0.05f) {
                    if (p->pushX >= 0.0f) {
                        p->targetFacing = 0.0f;
                    } else {
                        p->targetFacing = 3.1415927f;
                    }
                }
                plFaceMoveDir(p, 1.5707964f);
            }
            break;
        case 6:
            if (*(s32 *) a == CHAIN_TAG) {
                plActAssign(p, 0x60);
                p->chain = NULL;
                plMsgReply(b, 1);
                return;
            }
            break;
    }
    plChainDefault(p, msg, a, b);
}

void plChainEnd60(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;
    s32 t;

    if (msg == 1) {
        t = plAnimPercent(p);
        if (t > 32) {
            plStickSpeed(p);
            next = plPickGroundOrWalk(p);
            if (next == 0) {
                next = -1;
            }
            if (next == -1 && t == 100) {
                next = 0;
            }
        }
        plActChange(p, next);
    }
    plChainDefault(p, msg, a, b);
}

void plChainStart62(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            p->chain = NULL;
            p->facing = p->targetFacing;
            plSound(p, 0x20D);
            plSound(p, 0x20B);
            break;
        case 1:
            if (plAnimEnded(p)) {
                plActChange(p, 0x64);
            }
            break;
    }
    plChainDefault(p, msg, a, b);
}

/* (As plChainWait5E: the explicit default keeps p in its home.) */
void plChainWait64(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            if (plChainThrow(p) == NULL) {
                plActAssign(p, 0x5D);
            }
            break;
        case 1:
            break;
        case 6:
            if (*(s32 *) a == CHAIN_TAG) {
                plActAssign(p, 0x63);
                p->chain = NULL;
                plMsgReply(b, 1);
                return;
            }
            break;
        default:
            break;
    }
    plChainDefault(p, msg, a, b);
}

void plChainEnd63(Player *p, s32 msg, s32 a, s32 b) {
    s32 pad;
    s32 t;

    if (msg == 1) {
        t = plAnimPercent(p);
        if (t > 32) {
            if (plBaseStandNext(p)) {
                return;
            }
            if (t >= 100) {
                plActChange(p, 8);
                return;
            }
        }
    }
    plChainDefault(p, msg, a, b);
}

/* The chain task ("PL鎖"): six links strung from the player's hand toward init->b, each with
 * an attack box (the first of kind 0x19), and a tip, with a hook box on the player's model.
 * - Out (0): the links fly on their speeds; one striking the ground turns them back (2).
 * - Back (3): each link is drawn toward the one nearer the hand; the first home ends it.
 * - Something caught (message 9, "D2C46678") holds it (6) while the button 0x10 is held: the
 *   links are strung from it to the hand. Let go, the catch is told "expl".
 * - It ends when its player leaves play or is in state 0x200, or on "消滅" (BEC3CCC7).
 * u0, u4-u6 stand for unused locals that fill the game's gaps between the others.
 * - The game's two strings past "PL鎖": "%.2f\n" is the dot product's print (in a named
 *   f32 dot), and "base:..fix:.." is printed AFTER the main loop.
 * - "link->move.y = 0;" as an int: the game builds a fresh mtc1 zero there, not fs0's 0.0f.
 * - Case 2's call results are stored first and read back ("link->frame = f(..); f =
 *   link->frame;", box, tip), as the game does.
 * - Case 0's player is in its own variable pl. */
s32 plChainProc(Obj *obj, s16 msg, void *arg) {
    PlChainInit *init = arg;
    ChainWork *w = (ChainWork *) ((u32) obj + sizeof(Obj));
    f32 dot;
    Vec3f u0;
    Vec3f pos;
    Player *p;
    s32 i;
    Vec3f from;
    Vec3f at;
    f32 t;
    Vec3f u4;
    Vec3f u5;
    Vec3f u6;
    Vec3f hand;
    Player *pl;
    AnimeFrame tmp;
    Vec3f tvel;
    Vec3f prev;
    Vec3f tpos;
    Vec3f d;
    Vec3f prev2;
    Vec3f a;
    Vec3f cur;
    Vec3f b;
    ACollFrame *box;
    Vec3f rel;
    Vec3f next;
    ChainLink *link;
    AnimeFrame *f;

    switch (msg) {
        case 2:
            obj->name = "PL鎖";
            for (i = 0; i < 6; i++) {
                w->links[i].frame = NULL;
                w->links[i].box = NULL;
            }
            w->hook = NULL;
            w->target = NULL;
            w->caught = 0;
            w->player = init->p->index;
            p = PL(gGameWork.players[w->player]);
            for (i = 0; i < 6; i++) {
                link = &w->links[i];
                link->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
                f = link->frame;
                if (f == NULL) {
                    return -1;
                }
                f->anim = plChainLinkAnim;
                f->segs[1] = fileRamAddr(0x1A0);
                f->flags |= 0x20;
                link->box = acollAtkAddYZ(pl_atk_root_frame, &f->f, 30, 0, 0, 0);
                box = link->box;
                if (box == NULL) {
                    return -1;
                }
                if (i == 0) {
                    box->kind = 0x19;
                } else {
                    box->kind = 0x1A;
                }
                box->power = (s32) obj;
                link->move.x = 0.0f;
                link->move.y = 0.0f;
                link->move.z = 0.0f;
                pos.x = p->x + init->b->x * 0.16666667f * (6 - i);
                pos.y = p->y + init->b->y * 0.16666667f * (6 - i);
                pos.z = p->z + init->b->z * 0.16666667f * (6 - i);
                plPlaceTurned(PL(gGameWork.players[w->player]), f, &link->vel,
                                       init->a, &pos);
                if (i != 0) {
                    plFrameScaleSet(f, 0.075f);
                }
                link->unused20 = 0.0f;
            }
            w->links[0].frame->anim = plChainFirstLinkAnim;
            w->base = w->links[0].move;
            w->hook = acollAtkAddYZ(pl_atk_root_frame, &p->model->f, 50, 40, 120, 0);
            if (w->hook == NULL) {
                return -1;
            }
            plAttackBoxSet(p, box, 0x1A);
            w->tip = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            f = w->tip;
            if (f == NULL) {
                return -1;
            }
            f->anim = plChainTipAnim;
            f->segs[1] = fileRamAddr(0x1A0);
            f->flags |= 0x20;
            plFrameScaleSet(f, 0.075f);
            plFramePosCopy(f, w->links[0].frame);
            w->time = 0;
            w->state = 0;
            break;
        case 0:
            if (plIsInPlay(w->player) == 0) {
                return -1;
            }
            pl = PL(gGameWork.players[w->player]);
            if (pl->flags & 0x200) {
                return -1;
            }
            plPlaceTurned(pl, &tmp, &tvel, &gPlChainStartPos, &gPlChainVel);
            hand.x = tmp.pos[0];
            hand.y = tmp.pos[1];
            hand.z = tmp.pos[2];
            switch (w->state) {
                case 6:
                    if (!(pl->buttons & 0x10)) {
                        plSendTagMsg(w->target, 0x6578706C, 0, 0);
                        w->target = NULL;
                        w->state = 3;
                    } else if (w->target != NULL && eneIsEnemy(w->target) != 0) {
                        plSendTagMsg(w->target, 0x706F7369, (s32) &tpos, 0);
                        plVecSub(&d, &hand, &tpos);
                        plVecScale(&d, &d, 0.2f);
                        for (i = 0; i < 6; i++) {
                            f = w->links[i].frame;
                            f->pos[0] = i * d.x + tpos.x;
                            f->pos[1] = i * d.y + tpos.y;
                            f->pos[2] = i * d.z + tpos.z;
                        }
                        plFramePosCopy(w->tip, w->links[0].frame);
                    } else {
                        w->state = 3;
                    }
                    return tskDefaultProc(obj, msg, arg);
                case 0:
                    if (w->time >= 2) {
                        w->state = 3;
                    }
                    break;
                case 2:
                    w->state = 3;
                    break;
                case 3:
                    prev2 = hand;
                    for (i = 5; i >= 0; i--) {
                        link = &w->links[i];
                        f = link->frame;
                        cur.x = f->pos[0];
                        cur.y = f->pos[1];
                        cur.z = f->pos[2];
                        plVecSub(&a, &prev2, &cur);
                        plVecSub(&b, &hand, &cur);
                        plLimitSpeedXZ(&b, 3.0f);
                        link->move.x = pl->x + a.x * 0.85f + b.x * 0.6f;
                        link->move.y = pl->y * 0.5f + a.y * 0.8f + b.y * 0.3f;
                        link->move.z = pl->z + a.z * 0.85f + b.z * 0.6f;
                        prev2 = cur;
                    }
                    break;
                case 5:
                    plSound(pl, 0x20E);
                    return -1;
            }
            prev = hand;
            for (i = 5; i >= 0; i--) {
                link = &w->links[i];
                f = link->frame;
                if (w->state == 0) {
                    link->vel.x += link->move.x;
                    link->vel.y += link->move.y;
                    link->vel.z += link->move.z;
                    from.x = f->pos[0];
                    from.y = f->pos[1];
                    from.z = f->pos[2];
                    if (plChainCastRay(&from, &link->vel, &at, &t) != NULL) {
                        plVecScale(&link->vel, &link->vel, -0.1f);
                        link->move.y = 0;
                        w->state = 2;
                    }
                } else if (w->state == 3) {
                    link->vel = link->move;
                    plLimitSpeedXZ(&link->vel, 20.0f);
                    plLimitSpeedY(&link->vel, 20.0f);
                    from.x = f->pos[0];
                    from.y = f->pos[1];
                    from.z = f->pos[2];
                    plVecSub(&rel, &from, &hand);
                    plVecAdd(&next, &rel, &link->vel);
                    dot = plVecDot(&rel, &next);
                    PRINTF("%.2f\n", dot);
                    if (dot < 4.0f) {
                        if (i == 0) {
                            w->state = 5;
                        } else {
                            link->vel.z = 0.0f;
                            link->vel.y = 0.0f;
                            link->vel.x = 0.0f;
                            f->pos[0] = hand.x;
                            f->pos[1] = hand.y;
                            f->pos[2] = hand.z;
                        }
                    }
                }
                f->pos[0] += link->vel.x;
                f->pos[1] += link->vel.y;
                f->pos[2] += link->vel.z;
                prev.x = f->pos[0];
                prev.y = f->pos[1];
                prev.z = f->pos[2];
            }
            PRINTF("base:(%.1f,%.1f,%.1f), move:(%.1f,%.1f,%.1f), v:(%.1f,%.1f,%.1f), fix:(%.1f,%.1f,%.1f)\n",
                   w->base.x, w->base.y, w->base.z, link->move.x, link->move.y, link->move.z,
                   link->vel.x, link->vel.y, link->vel.z, f->pos[0], f->pos[1], f->pos[2]);
            plFramePosCopy(w->tip, w->links[0].frame);
            w->time++;
            break;
        case 1:
            for (i = 0; i < 6; i++) {
                frmDestroy(&w->links[i].frame->f);
                frmDestroy(&w->links[i].box->frame);
            }
            frmDestroy(&w->tip->f);
            frmDestroy(&w->hook->frame);
            if (plIsInPlay(w->player) != 0) {
                plSendTagMsg(gGameWork.players[w->player], CHAIN_TAG, 0, 0);
            }
            break;
        case 9:
            switch (*(s32 *) arg) {
                case 0xBEC3CCC7: /* 消滅 */
                    return -1;
                case 0xD2C46678:
                            w->target = ((Obj **) arg)[1];
                    if (w->caught == 0) {
                        w->state = 6;
                    }
                    w->caught = 1;
                    break;
                case 0xD2C4BBE0:
                            w->state = 3;
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

void *plChainCastRay(Vec3f *from, Vec3f *dir, Vec3f *at, f32 *arg) {
    return vramRayGround(from, dir, at, arg);
}

/* splat folds this uncalled empty function into the one before. */
void plChainEmpty(void) {
}
