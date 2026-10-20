#include "tlb.h"

/* File 132: the big elefish ("b-ele", an octopus: "tako") of stages 42 and 48, and its manager
 * ("demoelefish", demoBigElefish_proc), which makes it, drops the pass (tegata, kind 0xF7) when it
 * dies, gives the pass to the player who touches it (eventTegataSdSet) and plays the clear. */

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* eneMakeEx's last argument, here a number for the made enemy. */
typedef u32 (*EneMakeFunc)(void *, void *, void *);

/* Two s16 in one word for eneMakeEx: lo low, hi high. Both casts are the game's: without the
 * (s16) on lo, the four throws' t-registers came out rotated (measured). */
#define PACK16(lo, hi) (((s16) (lo) & 0xFFFF) | (((s16) (hi) << 16) & 0xFFFF0000))

/* acollDmgAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*DmgAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height,
                                     s16 ox, s16 oy, s16 oz, s32 a);

/* The pass's work (kind 0xF7, another file's), as far as this file reads it. */
typedef struct PassW {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 dist;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 off;
    /* 0x1C */ f32 rotX;
    /* 0x20 */ f32 rotY;
    /* 0x24 */ f32 rotZ;
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *atk; /* touched by a player */
    /* 0x38 */ Obj *glow;
} PassW;

/* The manager's work, at its task + 0x20. */
typedef struct DemoEleW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ Obj *ele; /* the big elefish (kind 0x181) */
    /* 0x38 */ Obj *pass; /* the pass it drops (kind 0xF7) */
    /* 0x3C */ s32 timer;
    /* 0x40 */ u32 stage; /* eneSetup's: 1 stage 42, 2 stage 48 */
} DemoEleW;

/* The big elefish's work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct BEleW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u32 stage; /* eneSetup's: 1 stage 42, 2 stage 48 */
    /* 0x38 */ Frame *unused38; /* destroyed in message 1 if set, never made */
    /* 0x3C */ ACollFrame *dmg; /* its body */
    /* 0x40 */ ACollFrame *dmg2; /* a second box, 90 up: live while it is risen */
    /* 0x44 */ MPolFrame *shadow;
    /* 0x48 */ Obj *sub; /* kind 0x1EF */
    /* 0x4C */ Obj *sub2; /* kind 0x1EF, made when it rises */
    /* 0x50 */ s8 hp;
    /* 0x54 */ s32 way; /* to the nearest player: 1, 2, 4, 8 */
    /* 0x58 */ u8 count;
    /* 0x5C */ f32 push[3];
    /* 0x68 */ s32 timer;
} BEleW;

#define BELE(obj) ((BEleW *) FX(obj))

/* plGetModelPos as eventTegataSdSet called it: an int function (main's is void). */
typedef s32 (*GetPosIntFunc)(Obj *task, f32 *x, f32 *y, f32 *z);

s32 bigElePassTouched(Obj *o);
Obj *bigElePassToucher(Obj *o);
s32 bigElePlayerNear(Obj *o);
s32 bigElePlayerOnCourse(Obj *o, Obj *pl);
void bigEleSetSubY(Obj *o, f32 y);

extern u8 bigEleAnim[];
extern u8 bigEleThrowAnim[];
u32 bigEleAnimBase[] = { (u32) bigEleAnim }; /* the big elefish's own animation (eneNewAnime's): { 0x030001B0 } */
u32 bigEleAnimThrow[] = { (u32) bigEleThrowAnim, 0x00000000, 0x00000000 }; /* its throwing animation: { 0x03000390, 0, 0 } */
extern u8 bigEleAnim[];

/* "eventTegataSdSet": the pass is the player's; its jingle, and the player's own voice. */
void bigEleEventTegataSdSet(Obj *o, Obj *pl) {
    s32 pad[3];
    u16 se;
    Vec3f pos;

    sndPlay(0xB8);
    if (pl == NULL) {
        DPRINTF("&&&& eventTegataSdSet  pl D がない\n");
        return;
    }
    switch (plGetChara(pl)) {
        case 0:
            DPRINTF("&&&& SD_G_CLEAR1\n");
            se = 0x16F;
            break;
        case 1:
            DPRINTF("&&&& SD_E_CLEAR1\n");
            se = 0x16E;
            break;
        case 2:
            DPRINTF("&&&& SD_S_CLEAR1\n");
            se = 0x170;
            break;
        case 3:
            DPRINTF("&&&& SD_Y_CLEAR1\n");
            se = 0x171;
            break;
    }
    /* The game called it with no prototype, as an int function: its unused result in v0 puts
     * soundIncamCam's address in v1 (measured: a void call gives v0 and the temp a word low). */
    ((GetPosIntFunc) plGetModelPos)(pl, &pos.x, &pos.y, &pos.z);
    soundIncamCam(se, pos.x, pos.y, pos.z);
    soundIncamCam(0x233, pos.x, pos.y, pos.z);
}

/* "demoelefish" (demoBigElefish_proc): the big elefish's manager. Stage 42 (flag 0xF: its
 * elefish dead; 0x9C: its pass taken) or 48 (0x10, 0x9D): makes the elefish, waits for it to
 * die, drops the pass, gives it to the player who touches it, then the clear (scenario 0x20)
 * and the stage's tunes. */
s32 demoBigElefish_proc(Obj *o, s16 msg, void *arg) {
    DemoEleW *w = (DemoEleW *) FX(o);
    s32 pad;
    Vec3f pos;
    s32 pad2;

    switch (msg) {
        case 2:
            o->name = "demoelefish";
            w->stage = eneSetup(o, arg);
            if (w->stage == 1 && swWorkTest(0xF)) {
                DPRINTF("demoBigElefish_proc: 42面 ＆ elefish1 死んでます\n");
                if (swWorkTest(0x9C)) {
                    DPRINTF("demoBigElefish_proc: 42面 ＆ elefish1 手形ゲットしているので死ね\n");
                    return -1;
                }
                DPRINTF("demoBigElefish_proc: 48面 ＆ elefish2 復活\n");
                swWorkReset(0xF);
            }
            if (w->stage == 2 && swWorkTest(0x10)) {
                DPRINTF("demoBigElefish_proc: 48面 ＆ elefish2 死んでます\n");
                if (swWorkTest(0x9D)) {
                    DPRINTF("demoBigElefish_proc: 48面 ＆ elefish2 手形ゲットしているので死ね\n");
                    return -1;
                }
                DPRINTF("demoBigElefish_proc: 48面 ＆ elefish2 復活\n");
                swWorkReset(0x10);
            }
            if (w->stage == 1) {
                DPRINTF("demoBigElefish_proc: 42面セット\n");
                w->state = 0;
            } else if (w->stage == 2) {
                DPRINTF("demoBigElefish_proc: 48面セット\n");
                w->state = 1;
            }
            break;
        case 1:
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    DPRINTF("demoBigElefish_proc: 42面 エレフィッシュセット\n");
                    w->ele = eneMakeEx(NULL, ENE_BIG_ELE, 1, 86.0f, 5.0f, 0.0f, 2, (EneMakeFunc) w->stage);
                    w->state = 2;
                    break;
                case 1:
                    DPRINTF("demoBigElefish_proc: 48面 エレフィッシュセット\n");
                    w->ele = eneMakeEx(NULL, ENE_BIG_ELE, 1, 259.0f, -582.0f, 0.0f, 2, (EneMakeFunc) w->stage);
                    w->state = 3;
                    break;
                case 2:
                    if (swWorkTest(0xF)) {
                        DPRINTF("demoBigElefish_proc: elefish1 死亡\n");
                        w->state = 4;
                    }
                    break;
                case 3:
                    if (swWorkTest(0x10)) {
                        DPRINTF("demoBigElefish_proc: elefish2 死亡\n");
                        w->state = 4;
                    }
                    break;
                case 4:
                    DPRINTF("demoBigElefish_proc: 手形落します\n");
                    if (w->stage == 1) {
                        w->pass = eneMakeEx(NULL, ENE_EPRG_TANU, 1, 86.0f, 5.0f, 0.0f, 0, NULL);
                    } else if (w->stage == 2) {
                        w->pass = eneMakeEx(NULL, ENE_EPRG_TANU, 1, 259.0f, -582.0f, 0.0f, 0, NULL);
                    } else {
                        DPRINTF("demoBigElefish_proc: Error: おかしいぞよDROP\n");
                    }
                    shimEneSetLowFlags(w->pass, 3);
                    w->state = 5;
                    break;
                case 5:
                    if (w->pass != NULL && bigElePassTouched(w->pass)) {
                        demoGetEneModelPos(w->pass, &pos);
                        ptclSpawnSparksFall(pos.x, pos.y, pos.z);
                        eneMakeEx(NULL, ENE_DEMO_KIRAKIRA, 0, pos.x, pos.y, pos.z, 0, NULL);
                        bigEleEventTegataSdSet(w->pass, bigElePassToucher(w->pass));
                        w->pass = tskDestroy(w->pass);
                        if (w->stage == 1) {
                            DPRINTF("demoBigElefish_proc: TEGATA2_2A ゲットだぜ\n");
                            swWorkSet(0x9C);
                        } else if (w->stage == 2) {
                            DPRINTF("demoBigElefish_proc: TEGATA3_2A ゲットだぜ\n");
                            swWorkSet(0x9D);
                        } else {
                            DPRINTF("demoBigElefish_proc: Error: おかしいぞよEND\n");
                            return -1;
                        }
                        scTegataAddArea(1);
                        w->timer = 0;
                        w->state = 6;
                    }
                    break;
                case 6:
                    if (w->timer == 60 / gRetracesPerFrame) {
                        scRunStart2(0x20, 0, 0);
                    }
                    if (w->timer == 300 / gRetracesPerFrame) {
                        camSunClearTunes();
                        if (w->stage == 1) {
                            camSunSetTunes2(0x43, 0x42);
                        } else if (w->stage == 2) {
                            camSunSetTunes2(0x4B, 0x4A);
                        }
                        sndSetReverb(1, 0xF);
                        camSunAddSoundPair(0x28D, 0x520);
                        camSunAddSoundPair(0x12F, 0x520);
                        return -1;
                    }
                    break;
            }
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The pass touched by a player: sparks, its glow gone. */
s32 bigElePassTouched(Obj *o) {
    PassW *w = (PassW *) FX(o);
    s32 pad;

    if (acollAtkWasHit(w->atk) == 1) {
        ptclSpawnSparksFall(w->model->pos[0], w->model->pos[1], w->model->pos[2]);
        w->glow = tskDestroy(w->glow);
        return 1;
    }
    return 0;
}

/* The player who touched the pass. */
Obj *bigElePassToucher(Obj *o) {
    return (Obj *) ((PassW *) FX(o))->atk->hitPower;
}

/* "b-ele": the big elefish, an octopus in the water. Hit on its body, it shakes; at less than 16
 * hit points it rises (a second piece, kind 0x1EF), turns to the nearest player and throws (kind
 * 0x1F9 on its way), sinks back; hit on its head with none left it dies (flag 0xF or 0x10). */
s32 bigEleProc(Obj *o, s16 msg, void *arg) {
    BEleW *w = BELE(o);
    s32 pad;
    s16 first;
    f32 x;
    f32 y;
    f32 z;
    f32 size;

    switch (msg) {
        case 2:
            o->name = "b-ele";
            w->stage = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) bigEleAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 0.3f;
            w->dmg = ((DmgAddXYZFunc) acollDmgAddXYZ)(ene_dmg_root_frame, &w->model->f, 55, 90, 0, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg2 = ((DmgAddXYZFunc) acollDmgAddXYZ)(ene_dmg_root_frame, &w->model->f, 40, 60, 0, 90, 0, 0);
            if (w->dmg2 == NULL) {
                return -1;
            }
            w->dmg2->flags &= ~2;
            w->hp = 25;
            w->shadow = shadowCreate();
            w->sub = eneMakeEx(gEneManager->child->next->next->next, ENE_DEMO_JIZOU_FILE_138, 0, w->model->pos[0],
                               w->model->pos[1], w->model->pos[2], 2, (EneMakeFunc) 0x1E14);
            if (w->sub != NULL) {
                shimEneSetLowFlags(w->sub, 3);
            }
            w->dmg->push = w->push;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->unused38 != NULL) {
                frmDestroy(w->unused38);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            if (w->dmg2 != NULL) {
                frmDestroy(&w->dmg2->frame);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            if (w->sub != NULL) {
                tskDestroy(w->sub);
            }
            if (w->sub2 != NULL) {
                tskDestroy(w->sub2);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (acollDmgWasHit(w->dmg) == 1) {
                        w->hp--;
                        DPRINTF("あたったHP[%d]\n", w->hp);
                        if (w->dmg->hitKind != 0) {
                            switch (w->dmg->hitKind) {
                                case 4:
                                case 5:
                                case 6:
                                case 23:
                                    eneSetSd2(o, 0x220, 0, 0);
                                    break;
                                case 18:
                                case 19:
                                case 20:
                                    eneSetSd2(o, 0x22D, 0, 0);
                                    break;
                                case 1:
                                default:
                                    eneSetSd2(o, 0x22C, 0, 0);
                                    break;
                            }
                        }
                        eneMakeHitSpark(0, w->dmg->push[0], w->dmg->push[1], w->dmg->push[2], 3, w->rot[1]);
                        w->timer = 0;
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (w->timer == 1 && w->dmg->hitPower != 0) {
                        fadeRumbleAll(0x66600000, 3);
                    }
                    if (w->timer < 2) {
                        w->rot[2] += 0.03f;
                    } else if (w->timer < 4) {
                        w->rot[2] -= 0.03f;
                    } else {
                        if (w->count++ >= 5) {
                            if (w->hp < 16) {
                                w->state = 2;
                            } else {
                                w->state = 0;
                            }
                            w->count = 0;
                        }
                        w->rot[2] = 0.0f;
                        w->timer = 0;
                    }
                    break;
                case 2:
                    if (w->timer == 1) {
                        w->sub2 = eneMakeEx(gEneManager->child->next->next->next, ENE_DEMO_JIZOU_FILE_138, 0, w->model->pos[0],
                                            w->model->pos[1] + 18.0f, w->model->pos[2], 2, (EneMakeFunc) 0x1001E14);
                        w->dmg2->push = w->push;
                        w->dmg->flags &= ~2;
                    }
                    if (eneAnimPlayOnce(o, w->model, 0.3f)) {
                        w->dmg2->flags |= 2;
                        w->timer = 0;
                        w->state = 3;
                    }
                    break;
                case 3:
                    if (w->timer == 1) {
                        w->way = eneNearestPlWay(o);
                    }
                    if (eneTurnToAngle(o, eneCourseAngle(o, w->way) * 360.0f / 6.283185482f, 10.0f)) {
                        w->model->anim = bigEleAnimThrow[0];
                        w->model->frame = 0.0f;
                        w->state = 4;
                    }
                    break;
                case 4:
                    if (eneAnimPlayOnce(o, w->model, 0.3f) == 1) {
                        w->model->anim = bigEleAnimBase[0];
                        w->model->frame = polAnimeLastFrame(w->model);
                        w->timer = 0;
                        w->state = 5;
                        break;
                    }
                    if (w->model->frame > 5.0 && w->model->frame <= 8.3) {
                        if (w->model->frame > 5.0 && w->model->frame <= 5.3) {
                            first = 1;
                        } else {
                            first = 0;
                        }
                        soundLocationCam(0x258, w->model->pos[0], w->model->pos[1], w->model->pos[2]);
                        switch (w->way) {
                            case 1:
                                eneMakeEx(NULL, ENE_TAKO_SUMI_MAIN, w->zone, w->x + 15.0f, w->y + 28.0f, w->z, 2,
                                          (EneMakeFunc) PACK16(w->way, first));
                                break;
                            case 4:
                                eneMakeEx(NULL, ENE_TAKO_SUMI_MAIN, w->zone, w->x - 15.0f, w->y + 28.0f, w->z, 2,
                                          (EneMakeFunc) PACK16(w->way, first));
                                break;
                            case 8:
                                eneMakeEx(NULL, ENE_TAKO_SUMI_MAIN, w->zone, w->x, w->y + 28.0f, w->z + 4.0f, 2,
                                          (EneMakeFunc) PACK16(w->way, first));
                                break;
                            case 2:
                                eneMakeEx(NULL, ENE_TAKO_SUMI_MAIN, w->zone, w->x, w->y + 28.0f, w->z - 4.0f, 2,
                                          (EneMakeFunc) PACK16(w->way, first));
                                break;
                        }
                    }
                    break;
                case 5:
                    if (w->timer >= 120 / gRetracesPerFrame) {
                        w->timer = 0;
                        w->state = 3;
                    }
                    break;
                case 6:
                    if (w->timer == 1) {
                        w->model->anim = bigEleAnimBase[0];
                        w->model->frame = polAnimeLastFrame(w->model);
                        eneMakeHitSpark(0, w->dmg->push[0], w->dmg->push[1], w->dmg->push[2], 3, 0.0f);
                        fadeRumbleAll(0x66443322, 30);
                    }
                    eneAnimPlayOnce(o, w->model, -0.8f);
                    if (w->sub2 != NULL) {
                        bigEleSetSubY(w->sub2, w->model->pos[1]);
                    }
                    DPRINTF("TAKO_TURN[%d]\n", polAnimeLastFrame(w->model));
                    DPRINTF("TAKO [%d]\n", w->state);
                    if (w->model->frame <= polAnimeLastFrame(w->model) - 8.0) {
                        w->state = 7;
                    }
                    break;
                case 7:
                    if (eneAnimPlayOnce(o, w->model, 0.8f)) {
                        if (w->sub2 != NULL) {
                            bigEleSetSubY(w->sub2, w->model->pos[1] + 18.0f);
                        }
                        w->state = 3;
                    }
                    break;
                case 8:
                    if (w->stage == 1) {
                        swWorkSet(0xF);
                    } else {
                        swWorkSet(0x10);
                    }
                    x = w->model->pos[0];
                    y = w->model->pos[1];
                    z = w->model->pos[2];
                    size = w->model->scale[0];
                    soundIncamCam(0x293, x, y, z);
                    itexSparkleBlue(x, y + 5.0f, z, size);
                    ptclSpawnSparksFall(x, y, z);
                    effectFspiritCreate(x, y, z, size, 0xE0, 0xE0, 0xE0, 0x10, 0x10, 0x10);
                    soundIncamCam(0x11B, w->model->pos[0], w->model->pos[1], w->model->pos[2]);
                    return -1;
                case 9:
                    if (w->timer == 1) {
                        w->model->anim = bigEleAnimBase[0];
                        w->model->frame = polAnimeLastFrame(w->model);
                        w->dmg2->flags &= ~2;
                    }
                    if (eneAnimPlayOnce(o, w->model, -0.8f)) {
                        DPRINTF("-------  タコ隠れる\n");
                        w->timer = 0;
                        w->state = 10;
                    }
                    break;
                case 10:
                    if (acollDmgWasHit(w->dmg) == 1) {
                        if (w->dmg->hitKind != 0) {
                            switch (w->dmg->hitKind) {
                                case 4:
                                case 5:
                                case 6:
                                case 23:
                                    eneSetSd2(o, 0x220, 0, 0);
                                    break;
                                case 18:
                                case 19:
                                case 20:
                                    eneSetSd2(o, 0x22D, 0, 0);
                                    break;
                                case 1:
                                default:
                                    eneSetSd2(o, 0x22C, 0, 0);
                                    break;
                            }
                        }
                        w->timer = 0;
                        w->state = 11;
                    }
                    if (!bigElePlayerNear(o) && eneAnimPlayOnce(o, w->model, 0.8f)) {
                        w->dmg2->flags |= 2;
                        w->state = 3;
                    }
                    break;
                case 11:
                    if (w->timer == 1) {
                        if (w->dmg->hitPower != 0) {
                            fadeRumbleAll(0x66600000, 3);
                        }
                        eneMakeHitSpark(w->zone, w->x, w->y + 10.0f, w->z, 2, w->rot[1]);
                    }
                    if (w->timer < 2) {
                        w->rot[2] += 0.03f;
                    } else if (w->timer < 4) {
                        w->rot[2] -= 0.03f;
                    } else {
                        if (w->count++ >= 5) {
                            w->state = 10;
                            w->count = 0;
                        }
                        w->rot[2] = 0.0f;
                        w->timer = 0;
                    }
                    break;
            }
            if (w->state != 6 && w->state != 7) {
                if (acollDmgWasHit(w->dmg2) == 1) {
                    w->hp--;
                    DPRINTF("あたったHP[%d]\n", w->hp);
                    if (w->hp <= 0) {
                        w->timer = 0;
                        w->state = 8;
                    } else {
                        if (w->dmg->hitKind != 0) {
                            switch (w->dmg->hitKind) {
                                case 4:
                                case 5:
                                case 6:
                                case 23:
                                    eneSetSd2(o, 0x220, 0, 0);
                                    break;
                                case 1:
                                default:
                                    eneSetSd2(o, 0x22C, 0, 0);
                                    break;
                            }
                        }
                        w->timer = 0;
                        w->state = 6;
                    }
                }
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 3.0f, NULL);
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* A player in play at the elefish's place (bigElePlayerOnCourse). */
s32 bigElePlayerNear(Obj *o) {
    s32 i;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i) && bigElePlayerOnCourse(o, gGameWork.players[i])) {
            return 1;
        }
    }
    return 0;
}

/* The player on the elefish's course and within 15 of it along the course. */
s32 bigElePlayerOnCourse(Obj *o, Obj *pl) {
    BEleW *w = BELE(o);
    Vec3f pos;
    s32 zone;

    plAskPos(pl, &pos.x, &pos.y, &pos.z, &zone);
    DPRINTF("my crs [%d] pl crs[%d]\n", w->zone, zone);
    if (zone == w->zone && w->x - 15.0f < pos.x && pos.x < w->x + 15.0f) {
        return 1;
    }
    return 0;
}

/* The second piece's height (kind 0x1EF's y). */
void bigEleSetSubY(Obj *o, f32 y) {
    ((EneObj *) FX(o))->y = y;
}
