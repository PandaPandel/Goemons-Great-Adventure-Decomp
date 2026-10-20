#include "enemy.h"
#include "game.h"
#include "demo.h"
#include "effect.h"
#include "enecommon.h"
#include "eneproc.h"
#include "gfxmgr.h"
#include "itex.h"
#include "particle.h"
#include "plmain.h"
#include "sndloc.h"
#include "stage.h"
#include "tmanager.h"

/* The demos (overlay 9): the players walked to goals at a speed or in a time
 * (demoSetSpeedPlRun, demoSetTimePlRun, for two or four players), set down and released, the
 * demo's talkers (demoTalkInit, demoTalk, demoPassMessage, demoFinish: a PersonObj), a death
 * in a demo, and the areas' flags kept while a demo plays. The debug prints are compiled out
 * (DPRINTF); IDO keeps their strings in .rodata, in source order. */

/* The game's debug print, compiled to nothing but its arguments (pakfiler.c's form). The
 * common.h PRINTF (an if (0)) moves registers in the two-player walks (demoSetSpeedPlRun,
 * demoSetTimePlRun); demoPlRunEndCheck needs it. */
#define DPRINTF

/* The 16 bytes of an area's flags at gSaveData + 0xA2 + area * 16. */
typedef struct DemoArea {
    u8 a[8];
    u8 b[8];
} DemoArea;

#define AREA(n) (((DemoArea *) gSaveData.areaFlags)[n])
#define SAVED_AREA(n) (((DemoArea *) gSaveCopy.areaFlags)[n])

extern Unk88208 gSaveCopy; /* gGameWork.save, named apart (undefined_syms.txt) */
extern u8 gDemoPlWalkDone[4]; /* by player: 1 once its demo walk is over */
extern Frame *ene_atk_root_frame; /* the enemies' attack boxes' parent */




/* demoSetSpeedPlRun: the first two players (jumping down first) walk to their goals at their
 * speeds. */
void demoSetSpeedPlRun(f32 x1, f32 y1, f32 z1, f32 speed1, f32 x2, f32 y2, f32 z2, f32 speed2) {
    s32 i;
    Vec3f goal;

    for (i = 0; i < 4; i++) {
        gDemoPlWalkDone[i] = 0;
        if (plIsInPlay(i) == 1) {
            plEnterDemo(gGameWork.players[i]);
            switch (i) {
                case 0:
                    goal.x = x1;
                    goal.y = y1;
                    goal.z = z1;
                    DPRINTF("demoSetSpeedPlRun: 1P speed[%f]\n", speed1);
                    plDemoWalkTo(gGameWork.players[i], goal, speed1);
                    break;
                case 1:
                    goal.x = x2;
                    goal.y = y2;
                    goal.z = z2;
                    DPRINTF("demoSetSpeedPlRun: 2P speed[%f]\n", speed2);
                    plDemoWalkTo(gGameWork.players[i], goal, speed2);
                    break;
            }
        }
    }
}

/* demoSetSpeedPlRun4P: the same for four players. */
void demoSetSpeedPlRun4P(f32 x1, f32 y1, f32 z1, f32 speed1, f32 x2, f32 y2, f32 z2, f32 speed2, f32 x3, f32 y3,
                            f32 z3, f32 speed3, f32 x4, f32 y4, f32 z4, f32 speed4) {
    s32 i;
    Vec3f goal;

    for (i = 0; i < 4; i++) {
        gDemoPlWalkDone[i] = 0;
        if (plIsInPlay(i) == 1) {
            plEnterDemo(gGameWork.players[i]);
            switch (i) {
                case 0:
                    goal.x = x1;
                    goal.y = y1;
                    goal.z = z1;
                    DPRINTF("demoSetSpeedPlRun4P: 1P speed[%f]\n", speed1);
                    plDemoWalkTo(gGameWork.players[i], goal, speed1);
                    break;
                case 1:
                    goal.x = x2;
                    goal.y = y2;
                    goal.z = z2;
                    DPRINTF("demoSetSpeedPlRun4P: 2P speed[%f]\n", speed2);
                    plDemoWalkTo(gGameWork.players[i], goal, speed2);
                    break;
                case 2:
                    goal.x = x3;
                    goal.y = y3;
                    goal.z = z3;
                    DPRINTF("demoSetSpeedPlRun4P: 3P speed[%f]\n", speed3);
                    plDemoWalkTo(gGameWork.players[i], goal, speed3);
                    break;
                case 3:
                    goal.x = x4;
                    goal.y = y4;
                    goal.z = z4;
                    DPRINTF("demoSetSpeedPlRun4P: 4P speed[%f]\n", speed4);
                    plDemoWalkTo(gGameWork.players[i], goal, speed4);
                    break;
            }
        }
    }
}

/* demoSetTimePlRun: the first two players (in play and not plIsPoweredDown) walk to their goals
 * in the given times. */
void demoSetTimePlRun(f32 x1, f32 y1, f32 z1, f32 time1, f32 x2, f32 y2, f32 z2, f32 time2) {
    s32 i;
    f32 px1;
    f32 py1;
    f32 pz1;
    f32 px2;
    f32 py2;
    f32 pz2;
    s32 zone1;
    s32 zone2;
    f32 dist;
    f32 speed;
    Vec3f goal;
    s32 pad;

    demoGetPlPos(&px1, &py1, &pz1, &zone1, &px2, &py2, &pz2, &zone2);
    for (i = 0; i < 4; i++) {
        gDemoPlWalkDone[i] = 0;
        if (plIsInPlay(i) == 1 && plIsPoweredDown(i) == 0) {
            plEnterDemo(gGameWork.players[i]);
            switch (i) {
                case 0:
                    dist = sqrtf((x1 - px1) * (x1 - px1) + (y1 - py1) * (y1 - py1) + (z1 - pz1) * (z1 - pz1));
                    goal.x = x1;
                    goal.y = y1;
                    goal.z = z1;
                    speed = dist / time1;
                    DPRINTF("demoSetTimePlRun: 1P speed[%f]\n", speed);
                    plDemoWalkTo(gGameWork.players[i], goal, speed);
                    break;
                case 1:
                    dist = sqrtf((x2 - px2) * (x2 - px2) + (y2 - py2) * (y2 - py2) + (z2 - pz2) * (z2 - pz2));
                    goal.x = x2;
                    goal.y = y2;
                    goal.z = z2;
                    speed = dist / time2;
                    DPRINTF("demoSetTimePlRun: 2P speed[%f]\n", speed);
                    plDemoWalkTo(gGameWork.players[i], goal, speed);
                    break;
            }
        }
    }
}

/* demoSetTimePlRun4P: the same for four players. */
void demoSetTimePlRun4P(f32 x1, f32 y1, f32 z1, f32 time1, f32 x2, f32 y2, f32 z2, f32 time2, f32 x3, f32 y3,
                            f32 z3, f32 time3, f32 x4, f32 y4, f32 z4, f32 time4) {
    s32 i;
    f32 px1;
    f32 py1;
    f32 pz1;
    f32 px2;
    f32 py2;
    f32 pz2;
    f32 px3;
    f32 py3;
    f32 pz3;
    f32 px4;
    f32 py4;
    f32 pz4;
    s32 zone1;
    s32 zone2;
    s32 zone3;
    s32 zone4;
    f32 dist;
    f32 speed;
    s32 pad1;
    s32 pad2;
    Vec3f goal;
    s32 pad;

    demoGetPlPos4P(&px1, &py1, &pz1, &zone1, &px2, &py2, &pz2, &zone2, &px3, &py3, &pz3, &zone3, &px4, &py4,
                           &pz4, &zone4);
    for (i = 0; i < 4; i++) {
        gDemoPlWalkDone[i] = 0;
        if (plIsInPlay(i) == 1) {
            plEnterDemo(gGameWork.players[i]);
            switch (i) {
                case 0:
                    dist = sqrtf((x1 - px1) * (x1 - px1) + (y1 - py1) * (y1 - py1) + (z1 - pz1) * (z1 - pz1));
                    goal.x = x1;
                    goal.y = y1;
                    goal.z = z1;
                    speed = dist / time1;
                    DPRINTF("demoSetTimePlRun4P: 1P speed[%f]\n", speed);
                    plDemoWalkTo(gGameWork.players[i], goal, speed);
                    break;
                case 1:
                    dist = sqrtf((x2 - px2) * (x2 - px2) + (y2 - py2) * (y2 - py2) + (z2 - pz2) * (z2 - pz2));
                    goal.x = x2;
                    goal.y = y2;
                    goal.z = z2;
                    speed = dist / time2;
                    DPRINTF("demoSetTimePlRun4P: 2P speed[%f]\n", speed);
                    plDemoWalkTo(gGameWork.players[i], goal, speed);
                    break;
                case 2:
                    dist = sqrtf((x3 - px3) * (x3 - px3) + (y3 - py3) * (y3 - py3) + (z3 - pz3) * (z3 - pz3));
                    goal.x = x3;
                    goal.y = y3;
                    goal.z = z3;
                    speed = dist / time3;
                    DPRINTF("demoSetTimePlRun4P: 3P speed[%f]\n", speed);
                    plDemoWalkTo(gGameWork.players[i], goal, speed);
                    break;
                case 3:
                    dist = sqrtf((x4 - px4) * (x4 - px4) + (y4 - py4) * (y4 - py4) + (z4 - pz4) * (z4 - pz4));
                    goal.x = x4;
                    goal.y = y4;
                    goal.z = z4;
                    speed = dist / time4;
                    DPRINTF("demoSetTimePlRun4P: 4P speed[%f]\n", speed);
                    plDemoWalkTo(gGameWork.players[i], goal, speed);
                    break;
            }
        }
    }
}

/* Whether every player's demo walk is over ("おわったでー"): a player whose walk ended jumps
 * down once. */
s32 demoPlRunEndCheck(void) {
    s32 i;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i) == 1) {
            plIsDemoWalking(gGameWork.players[i]);
            if (plIsDemoWalking(gGameWork.players[i]) == 0 && gDemoPlWalkDone[i] == 0) {
                /* The common.h PRINTF (an if (0)), not DPRINTF: its dead block decides the
                 * registers (i in s1; with DPRINTF i takes s2). */
                PRINTF("おわったでー[%d]\n", i);
                plEnterDemo(gGameWork.players[i]);
                gDemoPlWalkDone[i] |= 1;
            }
        } else {
            gDemoPlWalkDone[i] |= 1;
        }
    }
    if ((gDemoPlWalkDone[0] & 1) && (gDemoPlWalkDone[1] & 1) && (gDemoPlWalkDone[2] & 1) && (gDemoPlWalkDone[3] & 1)) {
        return 1;
    }
    return 0;
}

/* The first two players' places on their courses (0 if not in play). Player 1 not being in
 * play zeroes player 2's (sic). */
void demoGetPlPos(f32 *x1, f32 *y1, f32 *z1, s32 *zone1, f32 *x2, f32 *y2, f32 *z2, s32 *zone2) {
    f32 x;
    f32 y;
    f32 z;
    s32 zone;
    s32 i;

    for (i = 0; i < 4; i++) {
        switch (i) {
            case 0:
                if (plIsInPlay(i) == 1) {
                    (*plAskPos)(gGameWork.players[i], &x, &y, &z, &zone);
                    *x1 = x;
                    *y1 = y;
                    *z1 = z;
                    *zone1 = zone;
                } else {
                    *x2 = 0.0f;
                    *y2 = 0.0f;
                    *z2 = 0.0f;
                    *zone2 = 0;
                }
                break;
            case 1:
                if (plIsInPlay(i) == 1) {
                    (*plAskPos)(gGameWork.players[i], &x, &y, &z, &zone);
                    *x2 = x;
                    *y2 = y;
                    *z2 = z;
                    *zone2 = zone;
                } else {
                    *x2 = 0.0f;
                    *y2 = 0.0f;
                    *z2 = 0.0f;
                    *zone2 = 0;
                }
                break;
        }
    }
}

/* The same for four players (the same slip for player 1). */
void demoGetPlPos4P(f32 *x1, f32 *y1, f32 *z1, s32 *zone1, f32 *x2, f32 *y2, f32 *z2, s32 *zone2,
                            f32 *x3, f32 *y3, f32 *z3, s32 *zone3, f32 *x4, f32 *y4, f32 *z4, s32 *zone4) {
    f32 x;
    f32 y;
    f32 z;
    s32 zone;
    s32 i;

    for (i = 0; i < 4; i++) {
        switch (i) {
            case 0:
                if (plIsInPlay(i) == 1) {
                    (*plAskPos)(gGameWork.players[i], &x, &y, &z, &zone);
                    *x1 = x;
                    *y1 = y;
                    *z1 = z;
                    *zone1 = zone;
                } else {
                    *x2 = 0.0f;
                    *y2 = 0.0f;
                    *z2 = 0.0f;
                    *zone2 = 0;
                }
                break;
            case 1:
                if (plIsInPlay(i) == 1) {
                    (*plAskPos)(gGameWork.players[i], &x, &y, &z, &zone);
                    *x2 = x;
                    *y2 = y;
                    *z2 = z;
                    *zone2 = zone;
                } else {
                    *x2 = 0.0f;
                    *y2 = 0.0f;
                    *z2 = 0.0f;
                    *zone2 = 0;
                }
                break;
            case 2:
                if (plIsInPlay(i) == 1) {
                    (*plAskPos)(gGameWork.players[i], &x, &y, &z, &zone);
                    *x3 = x;
                    *y3 = y;
                    *z3 = z;
                    *zone3 = zone;
                } else {
                    *x3 = 0.0f;
                    *y3 = 0.0f;
                    *z3 = 0.0f;
                    *zone3 = 0;
                }
                break;
            case 3:
                if (plIsInPlay(i) == 1) {
                    (*plAskPos)(gGameWork.players[i], &x, &y, &z, &zone);
                    *x4 = x;
                    *y4 = y;
                    *z4 = z;
                    *zone4 = zone;
                } else {
                    *x4 = 0.0f;
                    *y4 = 0.0f;
                    *z4 = 0.0f;
                    *zone4 = 0;
                }
                break;
        }
    }
}

/* Sets the players down for a demo: each at its course (zone) and place, with its tilt; a
 * player given zone 0 jumps down and is put at the place in the world instead. Each player's
 * five stores are on ONE line: IDO schedules by line, and that order of the loads needs it. */
void demoSetPlPos(s32 zone1, f32 x1, f32 y1, f32 z1, f32 r1, s32 zone2, f32 x2, f32 y2, f32 z2, f32 r2,
                            s32 zone3, f32 x3, f32 y3, f32 z3, f32 r3, s32 zone4, f32 x4, f32 y4, f32 z4, f32 r4) {
    Vec3f pos[4];
    s32 zone[4];
    f32 r[4];
    s32 i;
    Player *p;

    pos[0].x = x1; pos[0].y = y1; pos[0].z = z1; r[0] = r1; zone[0] = zone1;
    pos[1].x = x2; pos[1].y = y2; pos[1].z = z2; r[1] = r2; zone[1] = zone2;
    pos[2].x = x3; pos[2].y = y3; pos[2].z = z3; r[2] = r3; zone[2] = zone3;
    pos[3].x = x4; pos[3].y = y4; pos[3].z = z4; r[3] = r4; zone[3] = zone4;
    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i) == 1) {
            if (zone[i] != 0) {
                plSetPos(gGameWork.players[i], zone[i], &pos[i]);
                p = PL(gGameWork.players[i]);
                p->tilt = r[i];
            } else {
                plEnterDemo(gGameWork.players[i]);
                p = PL(gGameWork.players[i]);
                p->flags |= 0x10000000;
                p->place = pos[i].x;
                p->height = pos[i].y;
                p->side = pos[i].z;
                p->model->pos[0] = pos[i].x;
                p->model->pos[1] = pos[i].y;
                p->model->pos[2] = pos[i].z;
                p->model->rot[1] = r[i];
            }
        }
    }
}

/* Every player in play jumps down ("デモ解除"). */
void demoPlStandAll(void) {
    s32 i;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i) == 1) {
            plEnterDemo(gGameWork.players[i]);
            DPRINTF("[%d] P デモ解除\n", i);
        }
    }
}

/* Every player in play to action 0xE2. */
void demoPlFreezeAll(void) {
    s32 i;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i) == 1) {
            plSetAction(gGameWork.players[i], 0xE2);
        }
    }
}

/* Every player in play to action 0, flag 0x10000000 off. */
void demoPlReleaseAll(void) {
    s32 i;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i) == 1) {
            plSetAction(gGameWork.players[i], 0);
            PL(gGameWork.players[i])->flags &= ~0x10000000;
        }
    }
}

/* The player in play with character chara (4: none). */
s32 demoFindPlByChara(s32 chara) {
    s32 i;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i) && plGetChara(gGameWork.players[i]) == chara) {
            return i;
        }
    }
    return 4;
}

/* demoTalkInit: a demo talker's table entry (none with 0): its heading, and its talk box's
 * kind 0x65. */
void demoTalkInit(Obj *o, u32 data) {
    PersonObj *p = PERSON(o);
    PersonData *d;

    if (data == 0) {
        p->data = NULL;
        return;
    }
    d = (PersonData *) stageAddrToRam(data);
    p->data = d;
    DPRINTF("demoTalkInit: talkNo[%d] r[%d] h[%d] ry[%f]\n", d->talk, d->radiusHeight[0], d->radiusHeight[1], d->rotY);
    p->rot[1] = d->rotY;
    if (p->box != NULL) {
        p->box->kind = 0x65;
    }
}

/* demoPassMessage: a hit on its talk box passes message 7 to the hitter; 1 if so. */
s32 demoPassMessage(Obj *o) {
    Obj *by;

    if ((*acollAtkWasHit)(PERSON(o)->box) == 1) {
        by = (Obj *) PERSON(o)->box->hitPower;
        if (by != NULL) {
            (*tskSendMsg)(by, 7, o);
            return 1;
        }
        DPRINTF("demoPassMessage: Please set playerD at damage frame. Please use taskSet\n");
    }
    return 0;
}

/* demoTalk: the talk messages, as personTalk's but for a demo talker. */
s32 demoTalk(Obj *o, PersonTalk *t) {
    PersonObj *p = PERSON(o);

    switch (t->msg) {
        case 0:
            DPRINTF("demoTalk: ENEPL_TALK_START を受け取ったぜ！\n");
            p->talker = t->player;
            if (!(p->flags & 2)) {
                p->flags |= 2;
                return 1;
            }
            return 0;
        case 3:
            p->count = 0;
            if (p->flags & 2) {
                return 1;
            }
            return 0;
        case 2:
            DPRINTF("personTalk: ENEPL_TALK_STANDBY を受け取ったぜ！\n");
            p->flags |= 1;
            break;
    }
    return 1;
}

/* demoFinish: the talker is sent message 8, the talk is over and the talk box is on again.
 * The else holding the compiled-out print keeps p in a register across the call. */
void demoFinish(Obj *o) {
    PersonObj *p = PERSON(o);
    s32 pad;
    struct {
        Obj *o;
        s32 one;
    } arg;

    arg.o = o;
    arg.one = 1;
    if (p->talker != NULL) {
        (*tskSendMsg)(p->talker, 8, &arg);
    } else {
        DPRINTF("demoFinish: Please set playerD at damage frame. Please use taskSet\n");
    }
    p->flags &= ~2;
    p->flags &= ~1;
    p->box->flags |= 4;
    eneSetForward(o, 0);
}

/* Whether its talk is done (talked to and a hit passed on); a talk left waiting ends after 4
 * frames. */
s32 demoTalkEndCheck(Obj *o) {
    PersonObj *p = PERSON(o);

    if ((*demoPassMessage)(o) != 0 && (PERSON(o)->flags & 1)) {
        return 1;
    }
    if (p->flags & 2) {
        if (++p->count >= 4) {
            DPRINTF("personAct: time out....\n");
            p->flags &= ~2;
        }
    }
    return 0;
}

/* Its talk box's kind to 0x69. */
void demoSetBoxKind69(Obj *o) {
    PersonObj *p = PERSON(o);

    if (PERSON(o)->box != NULL) {
        p->box->kind = 0x69;
    }
}

/* Whether its talk box was hit this step. */
s32 demoBoxHitCheck(Obj *o) {
    if ((*acollAtkWasHit)(PERSON(o)->box) == 1) {
        return 1;
    }
    return 0;
}

/* A demo talker's model (a "pol" frame) and talk box (radius r); -1 if there is no model. */
s32 demoTalkerMake(Obj *o, u8 r) {
    PersonObj *p = PERSON(o);
    s32 pad;

    p->model = (Frame *) (*eneNewPol)(o, NULL);
    if (p->model == NULL) {
        return -1;
    }
    p->box = (*acollAtkAddYZ)(ene_atk_root_frame, p->model, r, 0, 0, 0);
    p->box->kind = 0x65;
    return 0;
}

/* A demo talker's end: its model and talk box. */
void demoTalkerFree(Obj *o) {
    PersonObj *p = PERSON(o);

    if (PERSON(o)->model != NULL) {
        (*frmDestroy)(p->model);
    }
    if (p->box != NULL) {
        (*frmDestroy)((Frame *) p->box);
    }
}

/* A death in a demo, by the hit's kind (as eneDeathAct; with no halves' lists, the smoke,
 * which here also bursts and flashes); with reward, 0x1FC for a player with 1 or 2 pips, else
 * 0x1FB. Its locals in the game's slots (the pads), e set at its declaration (so it is kept, at
 * 0x30, not remade from o at the end), and the hit's attacker read once into by. */
void demoEneDeathAct(Obj *o, ACollFrame *box, u32 dl, u32 dl2, s32 reward) {
    u8 kind = box->hitKind;
    s8 pips;
    s32 pad[2];
    f32 x;
    f32 y;
    f32 z;
    f32 s;
    Obj *by;
    s32 give = 0;
    EneObj *e = ENE(o);

    if (dl == 0 || dl2 == 0) {
        kind = 7;
    }
    x =((PolFrame *) e->model)->pos[0];
    y = ((PolFrame *) e->model)->pos[1];
    z = ((PolFrame *) e->model)->pos[2];
    s = ((PolFrame *) e->model)->scale[0];
    switch (kind) {
        case 4:
        case 5:
        case 6:
            if (dl != 0 && dl2 != 0) {
                soundIncamCam(0x21F, box->hitPos[0], box->hitPos[1], box->hitPos[2]);
                (*eneMakeKillHahen)(o, box, dl, dl2);
            }
            if (reward) {
                give = 1;
            }
            break;
        case 18:
        case 19:
        case 20:
            soundIncamCam(0x22D, box->hitPos[0], box->hitPos[1], box->hitPos[2]);
            (*eneBlowOff)(o, box);
            if (reward) {
                give = 1;
            }
            break;
        case 1:
        case 2:
        case 3:
        case 17:
        case 24:
        case 28:
            if (reward) {
                give = 1;
            }
        default:
            (*eneSetSd2)(o, 0x229, 0x22A, 0);
            itexSparkleBlue(x, y + 5.0f, z, s);
            ptclSpawnSparksFall(x, y, z);
            effectFspiritCreate(x, y, z, s, 0xE0, 0xE0, 0xE0, 0x10, 0x10, 0x10);
            break;
    }
    by = (Obj *) box->hitPower;
    if (by != NULL && give) {
        pips = gSaveData.players[plGetIndex(by)].life;
        if (pips > 0 && pips < 3) {
            eneMakeEx(NULL, ENE_ETSK_DANGO_MAIN, e->zone, e->x, e->y, 0.0f, 0, NULL);
            return;
        }
        eneMakeEx(NULL, ENE_ETSK_KOBAN_MAIN, e->zone, e->x, e->y, 0.0f, 0, NULL);
    }
}

/* Keeps the six areas' flags while a demo plays. */
void demoAreaFlagSave(void) {
    s32 i;

    for (i = 0; i < 6; i++) {
        SAVED_AREA(i) = AREA(i);
    }
}

/* And puts them back. */
void demoAreaFlagRestore(void) {
    s32 i;

    for (i = 0; i < 6; i++) {
        AREA(i) = SAVED_AREA(i);
    }
}

/* Sets bit 7 of all of area n's flags. */
void demoAreaFlagSetBit7(s32 n) {
    s32 i;

    for (i = 0; i < 8; i++) {
        AREA(n).b[i] |= 0x80;
        AREA(n).a[i] |= 0x80;
    }
}

/* The enemy's model's place. */
void demoGetEneModelPos(Obj *o, Vec3f *out) {
    out->x = ((PolFrame *) ENE(o)->model)->pos[0];
    out->y = ((PolFrame *) ENE(o)->model)->pos[1];
    out->z = ((PolFrame *) ENE(o)->model)->pos[2];
}

/* Whether the enemy is in state 2. */
s32 demoEneState2Check(Obj *o) {
    if (ENE(o)->state == 2) {
        return 1;
    }
    return 0;
}
