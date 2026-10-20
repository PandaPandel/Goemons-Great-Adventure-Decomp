#include "tlb.h"

/* File 143: a pass ("tegata") won by standing still. The stand ("ts_oya") makes four hit boxes
 * ("ts_atari", kind 0x1DC), one per player; when the players stand still on theirs long enough
 * (">>>  syugyou", training) a pass ("tega_f", kind 0x1DD) falls, glowing, and the one who stood
 * there gets it (scenario 0x20, flag 0xC4). */

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* The stand's work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct OyaW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ Obj *atari[4]; /* the players' hit boxes (kind 0x1DC) */
    /* 0x44 */ Obj *pass;     /* the falling pass (kind 0x1DD) */
    /* 0x48 */ u8 pad48[4];
    /* 0x4C */ s32 timer; /* ticks in the state */
    /* 0x50 */ s32 count; /* still steps (0 when anyone moves) */
    /* 0x54 */ u8 flags;  /* 1: the sky's tunes change 300 ticks after the pass */
    /* 0x55 */ u8 tuneTimer;
    /* 0x58 */ s32 chara; /* the last still player's character: the pass's */
} OyaW;

#define OYA(obj) ((OyaW *) FX(obj))

/* A hit box's work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct AtariW {
    /* 0x00 */ u8 state; /* 1: stopped */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *box;
    /* 0x38 */ s32 timer;
    /* 0x3C */ s32 player; /* its player's index (its maker's value) */
    /* 0x40 */ u8 flags;   /* 1: its player on it, still; 2: on it, moving or pressing */
} AtariW;

#define ATARI(obj) ((AtariW *) FX(obj))

/* The pass's work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct TegaW {
    /* 0x00 */ u8 state; /* 0 falling, 1 landed */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *glow; /* its glow, gone when it lands */
    /* 0x38 */ u8 r;           /* the glow's colour, new every 10 ticks */
    /* 0x39 */ u8 g;
    /* 0x3A */ u8 b;
    /* 0x3B */ u8 pad3B;
    /* 0x3C */ s16 alpha;
    /* 0x40 */ s32 timer; /* ticks in the state */
    /* 0x44 */ f32 limit; /* the height it lands at */
    /* 0x48 */ s32 chara; /* the winner's character (its maker's value) */
} TegaW;

#define TEGA(obj) ((TegaW *) FX(obj))

/* acollAtkAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy,
                                     s16 oz, s32 a);

extern Gfx tegaFDl[];
extern Gfx tegaFGlowDl[];

s32 tsPlIsBusy(Obj *pl);
s32 tsAtariIsStill(Obj *o);
s32 tsAtariIsMoving(Obj *o);
void tsAtariStop(Obj *o);
s32 tegaFIsLanded(Obj *o);

void tegataStub0(void) {
}

void tegataStub1(void) {
}

void eventTegataSdSet(void) {
    DPRINTF("&&&& eventTegataSdSet  pl D がないn");
}

void eventTegataSdClear1(void) {
    DPRINTF("&&&& SD_G_CLEAR1n");
    DPRINTF("&&&& SD_E_CLEAR1n");
    DPRINTF("&&&& SD_S_CLEAR1n");
    DPRINTF("&&&& SD_Y_CLEAR1n");
}

/* "ts_oya": the stand. Each part has its own loop counter: case 2 j, state 0 i, state 3 k (both
 * its loops). The game shows it in its registers (IDA, 0143's sub_8000020): case 2's counter is
 * v1, stored to its home 0x68 (the fourth declared word) around eneMakeEx while the &atari[j]
 * walker takes s0 and eneMakeEx's address s1; state 3's first walker is v0 spilled at 0x3C, a
 * range apart from state 0's walker (s1); state 3's first loop keeps slti. pad[3] is the game's
 * frame: three unused words right above pos (0x4C), as in its twin nekobusStopProc (tlb_0133,
 * the same "ts_oya" work, IDA 0133's sub_8001484), whose locals are i, pl, pad[3], pos.
 * Matched 2026-10-16 from real 85 (the parked form: j shared by case 2 and state 3's
 * second loop, i elsewhere, four x8 FAKE do-while stacks, one in case 2 and three in state 3):
 *   - The four FAKE stacks removed: 46, frame 0x68, s0-s3 as the game. Trace on the 85 form: the
 *     stacks' empty blocks diluted the constant 1's and w's priorities (2.22, 3.0) below j's (4.6),
 *     so j took s2 and the 1 a fifth register, s4. Without them the 1 is 6.67 and takes s2.
 *   - s32 pad[4] between j and pos (the game's pos at 0x4C, its counter's home at 0x68): 27,
 *     frame 0x78.
 *   - State 3's first loop on its own k (the game's walker there is not state 0's): 21; its walker
 *     is then v0 spilled at 0x3C, as the game. k takes one pad word (pad[3]).
 *   - State 3's second loop on k too (the game's slti in the first loop): MATCH. j is then case 2's
 *     alone and takes v1 around the call.
 *   Measured and taken back: state 3's first loop on j 182; case 2 on j with state 3's second
 *   loop on i 179; state 3's second loop on k with its first on i 30. */
s32 tsOyaProc(Obj *o, s16 msg, void *arg) {
    OyaW *w = OYA(o);
    s32 i;
    Player *p;
    s32 j;
    s32 k;
    s32 pad[3]; /* FAKE: frame words; the game's pos at 0x4C (the twin nekobusStopProc declares pad[3] above pos) */
    Vec3f pos;

    switch (msg) {
        case 2:
            o->name = "ts_oya";
            if (swWorkTest(0xC4)) {
                DPRINTF("***   TEGATA1_DERO もらってるので死ぬ\n");
                return -1;
            }
            eneSetup(o, arg);
            w->model = eneNewPol(o, NULL);
            if (w->model == NULL) {
                return -1;
            }
            for (j = 0; j < 4; j++) {
                w->atari[j] = eneMakeEx(o, ENE_TS_ATARI, 0, 3.0f, -119.0f, 0.0f, 2, (u32 (*)(void *, void *, void *)) j);
                if (w->atari[j] == NULL) {
                    return -1;
                }
            }
            DPRINTF(">>>  syugyou セーットッ！！\n");
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    for (i = 0; i < 4; i++) {
                        if (tsAtariIsStill(w->atari[i]) == 1) {
                            w->count++;
                            w->chara = plGetChara(gGameWork.players[i]);
                        }
                    }
                    for (i = 0; i < 4; i++) {
                        if (tsAtariIsMoving(w->atari[i]) == 1) {
                            DPRINTF(">>>  [%d]P のせいでタイマー０から(-_-メ)\n", i + 1);
                            w->count = 0;
                        }
                    }
                    if (w->count >= 1200 / gRetracesPerFrame) {
                        DPRINTF(">>>  手形ゲットしそう(^o^)/\n");
                        w->timer = 0;
                        w->state = 3;
                    }
                    break;
                case 3:
                    for (k = 0; k < 4; k++) {
                        tsAtariStop(w->atari[k]);
                    }
                    for (k = 0; k < 4; k++) {
                        if (plIsInPlay(k) == 1) {
                            if (tsAtariIsStill(w->atari[k]) == 1) {
                                plEnterDemo(gGameWork.players[k]);
                                p = PL(gGameWork.players[k]);
                                p->flags |= 0x10000000;
                                p->model->rot[1] = 0.0f;
                            }
                        }
                    }
                    demoPlStandAll();
                    w->timer = 0;
                    w->state = 4;
                    break;
                case 4:
                    w->pass = eneMakeEx(NULL, ENE_TEGA_F, 0, 1.0f, -36.0f, 13.0f, 2,
                                        (u32 (*)(void *, void *, void *)) w->chara);
                    sndPlay(0x159);
                    w->timer = 0;
                    w->state = 5;
                    break;
                case 5:
                    if (tegaFIsLanded(w->pass) == 1) {
                        demoGetEneModelPos(w->pass, &pos);
                        ptclSpawnSparksFall(pos.x, pos.y, pos.z);
                        eneMakeEx(NULL, ENE_DEMO_KIRAKIRA, 0, pos.x, pos.y, pos.z, 0, NULL);
                        w->flags |= 1;
                        w->timer = 0;
                        w->state = 6;
                    }
                    break;
                case 6:
                    if (w->timer >= 30 / gRetracesPerFrame) {
                        scRunStart(0x20, 0, 0);
                        scTegataAddArea(5);
                        swWorkSet(0xC4);
                        w->timer = 0;
                        w->state = 7;
                    }
                    break;
                case 7:
                    if (!scIsRunning()) {
                        demoPlReleaseAll();
                        w->timer = 0;
                        w->state = 8;
                    }
                    break;
                case 8:
                    DPRINTF("だああああああああああああああああ\n");
                    break;
            }
            if (w->flags & 1) {
                if (300 / gRetracesPerFrame == w->tuneTimer++) {
                    w->flags &= ~1;
                    camSunClearTunes();
                    camSunSetTunes4(0x30, 0x2D, 0x2E, 0x2F);
                    return -1;
                }
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "ts_atari": a player's hit box on the stand. Hit by its own player it notes whether he is still
 * (flag 1) or moving or pressing (flag 2); anyone else's hit, or none, clears both. */
s32 tsAtariProc(Obj *o, s16 msg, void *arg) {
    AtariW *w = ATARI(o);
    s32 pad[2];
    Obj *pl;

    switch (msg) {
        case 2:
            o->name = "ts_atari";
            w->player = eneSetup(o, arg);
            w->model = eneNewPol(o, NULL);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 100, 0, 0);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->box != NULL) {
                frmDestroy(&w->box->frame);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (acollAtkWasHit(w->box) == 1) {
                        pl = (Obj *) w->box->hitPower;
                        if (plGetIndex(pl) == w->player) {
                            DPRINTF(">>>  担当プレーヤー[%d]Pがあたった\n", w->player + 1);
                            w->flags |= 2;
                            if (tsPlIsBusy(pl) == 0) {
                                w->flags &= ~2;
                                w->flags |= 1;
                            }
                        } else {
                            DPRINTF(">>>  担当プレーヤー[%d]Pがあたらなくなった1\n", w->player + 1);
                            w->flags &= ~2;
                            w->flags &= ~1;
                        }
                    } else {
                        DPRINTF(">>>  担当プレーヤー [] Pがあたらなくなった2\n");
                        w->flags &= ~2;
                        w->flags &= ~1;
                    }
                    break;
                case 1:
                    break;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Whether a player is busy: pressing A, B or Z this step, or moving along or across. */
s32 tsPlIsBusy(Obj *pl) {
    s32 idx = plGetIndex(pl);
    Player *p;

    if (joyGetHeldTime(idx, A_BUTTON) == 1) {
        return 1;
    }
    if (joyGetHeldTime(idx, B_BUTTON) == 1) {
        return 1;
    }
    if (joyGetHeldTime(idx, Z_TRIG) == 1) {
        return 1;
    }
    p = PL(gGameWork.players[idx]);
    if (p->velX > 1.000000012e-07f || p->velX < -1.000000012e-07f) {
        return 1;
    }
    if (p->velZ > 1.000000012e-07f || p->velZ < -1.000000012e-07f) {
        return 1;
    }
    return 0;
}

/* Whether a hit box's player stands still on it. */
s32 tsAtariIsStill(Obj *o) {
    if (ATARI(o)->flags & 1) {
        return 1;
    }
    return 0;
}

/* Whether a hit box's player is on it, moving or pressing. */
s32 tsAtariIsMoving(Obj *o) {
    if (ATARI(o)->flags & 2) {
        return 1;
    }
    return 0;
}

/* Stops a hit box. */
void tsAtariStop(Obj *o) {
    ATARI(o)->state = 1;
}

/* "tega_f": the pass, falling from the stand with a glow that flickers and pulses, down to 60
 * below where it was made; landed, its glow goes, and the winner (its value: a character) and then
 * the other player cheer.
 * The other player's index is its own variable j; the pads give the game's frame; the random
 * offsets are written "field + call", the double compares "timer == K / rate", message 1's tests
 * on one line each.
 * soundIncamCam returns SoundIncam2's value, 1 if played (sndloc.c and tlb.h declare it s32,
 * 2026-10-17). Its dropped result in v0 is what gives the timer v1; called as void, v0
 * (IDO_NOTES 369). */
s32 tegaFProc(Obj *o, s16 msg, void *arg) {
    TegaW *w = TEGA(o);
    s32 i;
    Vec3f pos;
    s32 chara;
    s32 pad1;
    u16 id;
    Vec3f pos2;
    s32 j;
    s32 pad2;
    u16 id2;

    switch (msg) {
        case 2:
            o->name = "tega_f";
            w->chara = eneSetup(o, arg);
            DPRINTF("************ value[%d]\n", w->chara);
            w->model = eneNewPol(o, tegaFDl);
            if (w->model == NULL) {
                return -1;
            }
            w->model->flags |= 0x20;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 0.1f;
            w->glow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->glow == NULL) {
                return -1;
            }
            w->glow->dl2 = tegaFGlowDl;
            w->glow->flags |= 0x23;
            floadGetSetRam((void **) w->glow->segs, 0x120);
            w->glow->scale[0] = w->glow->scale[1] = w->glow->scale[2] = 0.5f;
            w->glow->pos[0] = w->model->pos[0];
            w->glow->pos[1] = w->model->pos[1];
            w->glow->pos[2] = w->model->pos[2];
            w->alpha = 0xA0;
            w->limit = w->model->pos[1] - 60.0f;
            DPRINTF("手形効果セットＯＫ tegeta[%f] 限界高度[%f]\n", w->model->pos[1], w->limit);
            break;
        case 3:
            if (w->glow != NULL) {
                w->glow->dl = eneDlEnvPrimColor(o, w->r, w->g, w->b, w->r, w->g, w->b, w->alpha);
            }
            break;
        case 1:
            /* each test and its call on one line (IDO schedules by line; tlb_0084's form) */
            if (w->model != NULL) { frmDestroy(&w->model->f); }
            if (w->glow != NULL) { frmDestroy(&w->glow->f); }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->glow->pos[1] = w->model->pos[1];
                    if (w->model->pos[1] <= w->limit) {
                        DPRINTF("限界高度以下になったか\n");
                        w->timer = 0;
                        w->state = 1;
                    }
                    if ((w->timer & 3) == 0) {
                        eneMakeEx(NULL, ENE_DEMO_KIRA, 0, w->model->pos[0] + fadeRandRange(1, 4), w->model->pos[1],
                                  w->model->pos[2] + fadeRandRange(1, 4), 0, NULL);
                    }
                    eneFall(o, 0.02f);
                    w->hit &= ~1;
                    if (eneLand((EneObj *) w, 10.0f) == 1) {
                        w->hit |= 1;
                        w->vel[1] = 0.0f;
                    }
                    break;
                case 1:
                    if (w->timer == 1) {
                        w->model->f.flags &= ~2;
                        if (w->glow != NULL) {
                            w->glow = (PolFrame *) frmDestroy(&w->glow->f);
                        }
                        sndPlay(0xB8);
                        soundIncamCam(0x233, w->model->pos[0], w->model->pos[1], w->model->pos[2]);
                    }
                    if (w->timer == 66.0 / gRetracesPerFrame) {
                        chara = w->chara;
                        i = demoFindPlByChara(chara);
                        switch (chara) {
                            case 0:
                                DPRINTF("&&&& tegataGet SD_G_CLEAR\n");
                                id = 0x16F;
                                break;
                            case 1:
                                DPRINTF("&&&& tegataGet  SD_E_CLEAR\n");
                                id = 0x16E;
                                break;
                            case 2:
                                DPRINTF("&&&& tegataGet  SD_S_CLEAR\n");
                                id = 0x170;
                                break;
                            case 3:
                                DPRINTF("&&&& tegataGet  SD_Y_CLEAR\n");
                                id = 0x171;
                                break;
                        }
                        plGetModelPos(gGameWork.players[i], &pos.x, &pos.y, &pos.z);
                        soundIncamCam(id, pos.x, pos.y, pos.z);
                    }
                    if (w->timer == 96.0 / gRetracesPerFrame) {
                        if (demoFindPlByChara(w->chara) == 0) {
                            j = 1;
                        } else {
                            j = 0;
                        }
                        if (gGameWork.players[j] == NULL) {
                            DPRINTF("&&&& tegataGet 他人はいない\n");
                            return -1;
                        }
                        switch (plGetChara(gGameWork.players[j])) {
                            case 0:
                                DPRINTF("&&&& tegataGet SD_G_CLEAR\n");
                                id2 = 0x16F;
                                break;
                            case 1:
                                DPRINTF("&&&& tegataGet  SD_E_CLEAR\n");
                                id2 = 0x16E;
                                break;
                            case 2:
                                DPRINTF("&&&& tegataGet  SD_S_CLEAR\n");
                                id2 = 0x170;
                                break;
                            case 3:
                                DPRINTF("&&&& tegataGet  SD_Y_CLEAR\n");
                                id2 = 0x171;
                                break;
                        }
                        plGetModelPos(gGameWork.players[j], &pos2.x, &pos2.y, &pos2.z);
                        soundIncamCam(id2, pos2.x, pos2.y, pos2.z);
                        return -1;
                    }
                    break;
            }
            if (w->timer % 10 == 0) {
                w->r = fadeRandRange(0x80, 0xFF);
                w->g = fadeRandRange(0x80, 0xFF);
                w->b = fadeRandRange(0x80, 0xFF);
            }
            if (w->glow != NULL) {
                if (!(w->timer & 1)) {
                    w->glow->scale[0] += 0.3;
                    w->glow->scale[1] += 0.3;
                    w->glow->scale[2] += 0.3;
                } else {
                    w->glow->scale[0] -= 0.3;
                    w->glow->scale[1] -= 0.3;
                    w->glow->scale[2] -= 0.3;
                }
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Whether the pass has landed. */
s32 tegaFIsLanded(Obj *o) {
    if (TEGA(o)->state == 1) {
        return 1;
    }
    return 0;
}
