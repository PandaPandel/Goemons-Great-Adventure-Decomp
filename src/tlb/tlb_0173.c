#include "tlb.h"

/* The "hellbat" enemy kind (archive file 173): a bat, of three types (its placement's). Type 0
 * hangs until a player comes within 100, then flies at it along the course, bobbing and
 * flapping, and falls when hit; type 1 and type 2 wait (for the debug R trigger) and swing out
 * along the course; type 2 turns to face the nearest player while it waits. A beaten bat
 * falls, lands, fades and is gone. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* fabs is IDO's intrinsic: an inline abs.d. */
#pragma intrinsic (fabs)

/* An animation table's entry: the animation, its speed at 60 steps a second, and whether it
 * loops. The entry's number is the bat's state. */
typedef struct HellbatAnim {
    /* 0x00 */ u32 anim;
    /* 0x04 */ f32 speed;
    /* 0x08 */ u8 loop;
} HellbatAnim;

/* A hellbat's work, at its task + 0x20: an enemy's (EneObj) up to 0x40, then its own. */
typedef struct Hellbat {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ MPolFrame *shadow;
    /* 0x40 */ f32 speed; /* its animation's, per frame */
    /* 0x44 */ f32 phase; /* its swing's angle */
    /* 0x48 */ f32 phase2; /* type 2's second angle (set, not read here) */
    /* 0x4C */ u8 type; /* its placement's type: 0, 1 or 2 */
    /* 0x4D */ u8 state; /* its animation table's entry */
    /* 0x4E */ u8 hits; /* hits it takes */
    /* 0x4F */ s8 gravity; /* its fall's, in hundredths */
    /* 0x50 */ u8 alpha; /* its env colour's */
    /* 0x51 */ u8 timer; /* the fade's steps */
    /* 0x52 */ s8 side; /* 0 or 1, the side of the course it flies to (-1: its own heading) */
    /* 0x53 */ s8 target; /* the nearest player (-1: none) */
    /* 0x54 */ u8 unused54; /* type 2: a random 10-30, never read */
    /* 0x55 */ u8 swingX; /* eneSetup's result, its low byte: the swing along the course */
    /* 0x56 */ u8 swingY; /* and its second: the swing up and down */
    /* 0x57 */ u8 wait; /* a random 0-15: the step its flap sounds on */
} Hellbat;

#define HB(obj) ((Hellbat *) FX(obj))

extern GameState gGameStateHb; /* gGameState, as type 0's state 1 reads it (undefined_syms.txt) */
extern u8 hellbatType0Anim[];
HellbatAnim hellbatAnimType0[] = {
    { (u32) hellbatType0Anim, 2.0f, 0 },
    { 0x030007A0, 0.0f, 0 },
    { (u32) hellbatType0Anim, 2.0f, 1 },
    { 0x030007A0, 0.0f, 0 },
    { 0x030007A0, 0.0f, 0 },
    { 0x030007A0, 0.0f, 0 },
    { 0x00000000, 0.0f, 0 },
}; /* type 0's animations */
HellbatAnim hellbatAnimType1[] = {
    { (u32) hellbatType0Anim, 1.2f, 0 },
    { 0x030007A0, 0.0f, 0 },
    { (u32) hellbatType0Anim, 1.2f, 1 },
    { (u32) hellbatType0Anim, 0.0f, 0 },
    { 0x00000000, 0.0f, 0 },
}; /* type 1's */
HellbatAnim hellbatAnimType2[] = {
    { (u32) hellbatType0Anim, 3.0f, 0 },
    { (u32) hellbatType0Anim, 3.0f, 1 },
    { (u32) hellbatType0Anim, 3.0f, 1 },
    { 0x030007A0, 0.0f, 0 },
    { (u32) hellbatType0Anim, 3.0f, 1 },
    { 0x030007A0, 0.0f, 0 },
    { 0x030007A0, 0.0f, 0 },
    { 0x030007A0, 0.0f, 0 },
    { 0x00000000, 0.0f, 0 },
}; /* type 2's */
extern u8 hellbatHalf0Dl[];
extern u8 hellbatHalf1Dl[];

s32 hellbatDeathAct(Obj *o, ACollFrame *box, u32 dl, u32 dl2);
s32 hellbatActType0(Obj *o);
s32 hellbatActType1(Obj *o);
s32 hellbatActType2(Obj *o);

/* Sets animation n of the table tab, from its first frame. */
void hellbatSetAnime(Obj *o, u8 n, HellbatAnim *tab) {
    Hellbat *w = HB(o);

    w->state = n;
    w->speed = tab[n].speed / gRetracesPerFrame;
    w->model->anim = tab[n].anim;
    w->model->frame = 0.0f;
}

/* Sets animation n of the table tab, from a random frame. */
void hellbatSetAnimeRand(Obj *o, u8 n, HellbatAnim *tab) {
    Hellbat *w = HB(o);
    s32 pad;

    hellbatSetAnime(o, n, tab);
    w->model->frame = fadeRandRange(0, polAnimeLastFrame(w->model));
}

void hellbatNop0(void) {
}

void hellbatNop1(void) {
}

/* Puts it back on its course and returns its heading there: mode 0 the heading less ang, 1 the
 * heading turned by ang and a half turn, -1 its own rotation, else the heading. */
f32 hellbatCourseHeading(Obj *o, f32 ang, s8 mode) {
    Hellbat *w = HB(o);
    f32 r;
    f32 x;
    f32 y;
    f32 z;
    s32 pad;

    w->x = vramCoursePointXZ(w->zone, w->x, w->z, &x, &z);
    r = vramCourseHeading(w->zone, w->x);
    if (mode == 0) {
        r -= ang;
    } else if (mode == 1) {
        r += ang + 3.141592741f;
    } else if (mode == -1) {
        r = w->rot[1];
    }
    return r;
}

/* The distance from it to player i (0 if i is not 0-3 or not in play). */
f32 hellbatPlDist(Obj *o, s32 i) {
    f32 dx;
    f32 dy;
    f32 x;
    f32 y;
    f32 z;
    Vec3f pos;

    if (i < 0 || i >= 4) {
        return 0.0f;
    }
    eneWorldPos(o, &pos);
    if (plIsInPlay(i)) {
        plGetModelPos(gGameWork.players[i], &x, &y, &z);
        dx = x - pos.x;
        dy = y - pos.y;
        return sqrtf(dx * dx + dy * dy + (z - pos.z) * (z - pos.z));
    }
    return 0.0f;
}

/* The nearest player in play (-1: none). */
s32 hellbatNearestPl(Obj *o) {
    f32 d;
    f32 min = 65536.0f;
    s32 i;
    f32 x;
    f32 y;
    f32 z;
    s32 pad1;
    s32 pad2;
    s32 near = -1;
    Vec3f pos;

    eneWorldPos(o, &pos);
    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plGetModelPos(gGameWork.players[i], &x, &y, &z);
            d = hellbatPlDist(o, i);
            if (d < min) {
                min = d;
                near = i;
            }
        }
    }
    return near;
}

/* The angle from it to player i, along the ground (0 if i is not 0-3 or not in play). */
f32 hellbatPlAngle(Obj *o, s32 i) {
    f32 dx;
    f32 dy;
    f32 x;
    f32 y;
    f32 z;
    Vec3f pos;

    if (i < 0 || i >= 4) {
        return 0.0f;
    }
    eneWorldPos(o, &pos);
    if (plIsInPlay(i)) {
        plGetModelPos(gGameWork.players[i], &x, &y, &z);
        return mathAtan2(x - pos.x, z - pos.z);
    }
    return 0.0f;
}

/* The kill's reward, for the player whose attack hit box (eneDeathAct's). */
void hellbatKillReward(Obj *o, ACollFrame *box, s32 many) {
    s32 i;
    s32 r = -1;
    Obj *by = (Obj *) box->hitPower;

    if (by != NULL) {
        for (i = 0; i < 4; i++) {
            if (by == gGameWork.players[i]) {
                r = PL(gGameWork.players[i])->index;
            }
        }
    }
    eneKillReward(o, r, many);
}

/* "eneFallLimit": 1 once it is below -600 or above 1000. */
s32 hellbatFallLimit(Obj *o) {
    Hellbat *w = HB(o);

    if (w->y < -600.0f) {
        PRINTF("eneFallLimit(-600):\n");
        return 1;
    }
    /* written constant first: "w->y > 1000.0f" shares the first test's load of y */
    if (1000.0f < w->y) {
        PRINTF("eneFallLimit(+1000):\n");
        return 1;
    }
    return 0;
}

/* "hellbat": the kind's proc. */
s32 hellbatProc(Obj *o, s16 msg, void *arg) {
    Hellbat *w = HB(o);
    s32 a;
    Obj *by;
    s32 r;

    switch (msg) {
        case 2:
            o->name = "hellbat";
            a = eneSetup(o, arg);
            w->swingX = a & 0xFF;
            w->swingY = (a >> 8) & 0xFF;
            w->type = (f32) (a >> 16);
            w->state = 0;
            w->gravity = 0;
            w->timer = 0;
            w->target = 0;
            w->hits = 1;
            w->alpha = 255;
            w->wait = fadeRandRange(0, 15);
            if ((w->model = eneNewAnime(o, 0)) == NULL) {
                return -1;
            }
            if ((w->shadow = shadowCreate()) == NULL) {
                return -1;
            }
            if ((w->atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, &w->model->f, 0, 0, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, &w->model->f, 0, 0, 0, 0, 0)) == NULL) {
                return -1;
            }
            break;
        case 0:
            if (hellbatFallLimit(o)) {
                return -1;
            }
            if (acollAtkWasHit(w->atk) == 1) {
                by = (Obj *) w->atk->hitPower;
                if (by != NULL) {
                    tskSendMsg(by, 7, o);
                }
            }
            switch (w->type) {
                case 0:
                    r = hellbatActType0(o);
                    break;
                case 1:
                    r = hellbatActType1(o);
                    break;
                case 2:
                    r = hellbatActType2(o);
                    break;
            }
            if (r != 0) {
                return r;
            }
            break;
        case 3:
            w->model->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombine(gGfxDlHead++, 0x409A81, 0xFF37FFFF);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Its death by the hit on box (eneDeathAct's, with the halves' lists dl and dl2): cut in two
 * or blown off (then it is gone: -1), else a burst of smoke (0, and it is drawn on layer 8). */
s32 hellbatDeathAct(Obj *o, ACollFrame *box, u32 dl, u32 dl2) {
    Hellbat *w = HB(o);
    s32 pad;
    s32 reward = 0;
    s32 gone = 0;

    switch (box->hitKind) {
        case 4:
        case 5:
        case 6:
            soundIncamCam(0x21F, box->hitPos[0], box->hitPos[1], box->hitPos[2]);
            if (dl != 0 && dl2 != 0) {
                eneMakeKillHahen(o, box, dl, dl2);
            }
            reward = 1;
            gone = 1;
            break;
        case 18:
        case 19:
        case 20:
            soundIncamCam(0x22D, box->hitPos[0], box->hitPos[1], box->hitPos[2]);
            eneBlowOff(o, box);
            reward = 1;
            gone = 1;
            break;
        case 1:
        case 2:
        case 3:
        case 17:
        case 24:
        case 28:
            reward = 1;
        default:
            eneSetSd2(o, 0x229, 0x22A, 0);
            effectBurst(w->model->pos[0], w->model->pos[1] + 5.0f, w->model->pos[2]);
            break;
    }
    sndPlay(0x852E);
    gGameWork.kills++;
    PRINTF("hellbat: %d 人斬り\n", gGameWork.kills);
    if (reward) {
        hellbatKillReward(o, box, 0);
    }
    if (gone) {
        w->model->f.flags &= ~2;
        return -1;
    }
    frmMove(&w->model->f, gFrmLayer8);
    return 0;
}

/* Type 0: hangs (1) until a player is within 100, then flies at it (2); hit, it falls (3),
 * lands and fades (4), and is gone (5). State 1 reads gGameState under a second name
 * (undefined_syms.txt): uopt otherwise makes one address candidate for states 1 and 3. The
 * speed is a ternary inside the product; the frame (0x40) needs d and one pad. */
s32 hellbatActType0(Obj *o) {
    Hellbat *w = HB(o);
    s32 r;
    f32 d;
    s32 pad;

    eneFall(o, w->gravity * 0.01f * gRetracesPerFrame);
    eneMove((EneObj *) w);
    eneSetModelOnCourse(o, &w->model->f);
    if (hellbatAnimType0[w->state].loop == 1) {
        eneAnimLoop(o, w->model, w->speed);
    } else {
        eneAnimPlayOnce(o, w->model, w->speed);
    }
    if (w->shadow != NULL) {
        shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.3f, NULL);
    }
    if (acollDmgWasHit(w->dmg) == 1 && --w->hits == 0) {
        hellbatSetAnime(o, 3, hellbatAnimType0);
        w->gravity = 25;
        w->vel[0] = 0.0f;
        w->vel[1] = 4.5f;
        w->dmg->flags &= ~2;
        w->atk->flags &= ~2;
        if (hellbatDeathAct(o, w->dmg, (u32) hellbatHalf0Dl, (u32) hellbatHalf1Dl)) {
            PRINTF("*** hellbat:dead ***\n");
            hellbatSetAnime(o, 5, hellbatAnimType0);
        }
    }
    switch (w->state) {
        case 0:
            w->atk->radius = 70;
            w->atk->ofs[1] = 20;
            w->atk->kind = 120;
            w->dmg->radius = 80;
            w->dmg->height = 150;
            w->dmg->ofs[1] = -50;
            w->model->scale[2] = 0.1f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            if (eneNearestPlWay(o) == 1) {
                w->side = 0;
            } else {
                w->side = 1;
            }
            w->rot[1] = hellbatCourseHeading(o, 0.5235987902f, w->side);
            hellbatSetAnime(o, 1, hellbatAnimType0);
        case 1:
            if ((u32) gGameStateHb.frameCount % (30 / gGameStateHb.retracesPerFrame) == w->wait) {
                eneSetSd(o, 0x52E);
            }
            w->target = hellbatNearestPl(o);
            if (w->target < 0) {
                break;
            }
            d = hellbatPlDist(o, w->target);
            if (d < 100.0f) {
                PRINTF("*** increase ***\n");
                hellbatSetAnimeRand(o, 2, hellbatAnimType0);
            }
            PRINTF("**** dist[%d][%f][%f] ****\n", w->target, d, 100.0f);
            break;
        case 2:
            if (gGameFrameCount % (30 / gRetracesPerFrame) == w->wait) {
                eneSetSd(o, 0x52E);
            }
            w->rot[1] = hellbatCourseHeading(o, 0.5235987902f, w->side);
            w->phase += 6.0f * gRetracesPerFrame * 0.01745329238f;
            w->vel[1] = w->swingY * (0.1f * gRetracesPerFrame) * mathSin(w->phase);
            w->vel[0] = (w->side ? gRetracesPerFrame * -0.1f : gRetracesPerFrame * 0.1f) * w->swingX *
                        (fabs(mathCos(w->phase - 1.570796371f)) + 0.5);
            w->speed = (mathCos(w->phase) + 1.0f) * gRetracesPerFrame;
            break;
        case 3:
            sndPlay(0x852E);
            r = eneLandOff((EneObj *) w, 10.0f, -5.0f);
            w->rot[1] += 15.0f * gGameState.retracesPerFrame * 0.01745329238f;
            if (r) {
                w->gravity = 0;
                w->vel[2] = 0.0f;
                w->vel[0] = 0.0f;
                w->vel[1] = 0.0f;
                w->timer = 40 / gGameState.retracesPerFrame;
                w->side = -1;
                w->model->scale[0] *= 0.7;
                hellbatSetAnime(o, 4, hellbatAnimType0);
            }
            break;
        case 4:
            sndPlay(0x852E);
            if (--w->timer == 0) {
                hellbatSetAnime(o, 5, hellbatAnimType0);
            }
            if (w->timer < 40 / gRetracesPerFrame && w->shadow != NULL) {
                w->shadow = (MPolFrame *) frmDestroy(&w->shadow->f);
            }
            w->alpha -= (u8) (255 / (40 / gRetracesPerFrame));
            w->model->scale[1] -= 0.1f / (40 / gRetracesPerFrame) * 0.5f;
            break;
        case 5:
            sndPlay(0x852E);
            eneReleaseSpawn(w->spawn);
            return -1;
    }
    return 0;
}

/* Type 1: waits (1) for the R trigger, then swings along the course (2); hit, it bursts into
 * kind 0x22E (3) and is gone.
 * Matched 2026-10-16; it was parked for which range held v0 over state 2's base read
 * (&gGameState, against mathCos's address); an empty test of gGameState.retracesPerFrame
 * (a FAKE, kept) gives the base one more read in the block, outranking mathCos's, as the game's
 * colouring needs. */
s32 hellbatActType1(Obj *o) {
    Hellbat *w = HB(o);
    s32 pad1;
    s32 pad2;
    s32 pad3;

    eneFall(o, w->gravity * 0.01f * gRetracesPerFrame);
    eneMove((EneObj *) w);
    eneSetModelOnCourse(o, &w->model->f);
    if (hellbatAnimType1[w->state].loop == 1) {
        eneAnimLoop(o, w->model, w->speed);
    } else {
        /* FAKE: an empty test of the frame rate (the permuter's find was an empty if/else, a
         * permuter pass named perm_refer_to_var_else; the plain test is the same object). It puts one more read
         * of gGameState in this block, so the base range covers it and v0 goes as in the game
         * (IDO_NOTES 283). */
        if (gGameState.retracesPerFrame) {}
        eneAnimPlayOnce(o, w->model, w->speed);
    }
    if (w->shadow != NULL) {
        shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.3f, NULL);
    }
    if (acollDmgWasHit(w->dmg) == 1 && --w->hits == 0) {
        hellbatSetAnime(o, 3, hellbatAnimType1);
        w->vel[2] = 0.0f;
        w->vel[1] = 0.0f;
        w->vel[0] = 0.0f;
    }
    switch (w->state) {
        case 0:
            w->atk->radius = 30;
            w->atk->ofs[1] = 20;
            w->atk->kind = 120;
            w->dmg->radius = 50;
            w->dmg->height = 100;
            w->dmg->ofs[1] = -20;
            w->model->scale[2] = 0.17f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            if (eneNearestPlWay(o) == 1) {
                w->side = 0;
            } else {
                w->side = 1;
            }
            w->rot[1] = hellbatCourseHeading(o, 0.5235987902f, w->side);
            hellbatSetAnime(o, 1, hellbatAnimType1);
        case 1:
            if (joyGetPadHist(0, 0)->button & R_TRIG) {
                w->vel[0] = w->side ? gRetracesPerFrame * -0.3f : gRetracesPerFrame * 0.3f;
                w->phase = fadeRandRange(0, 180) * 0.01745329238f;
                hellbatSetAnime(o, 2, hellbatAnimType1);
            }
            break;
        case 2:
            w->phase += 6.0f * gGameState.retracesPerFrame * 0.01745329238f;
            w->vel[0] = (w->side ? gGameState.retracesPerFrame * -0.2f : gGameState.retracesPerFrame * 0.2f) *
                        (fabs(mathCos(w->phase - 1.570796371f)) + 1.0);
            w->speed = (mathCos(w->phase) + 1.0f) * (0.5f * gGameState.retracesPerFrame);
            break;
        case 3:
            eneMakeEx(NULL, ENE_HELLBAT, w->zone, w->x, w->y, w->z, 2, (u32 (*)(void *, void *, void *)) 2);
            eneReleaseSpawn(w->spawn);
            return -1;
    }
    return 0;
}

/* Type 2: turns to the nearest player (2); with the R trigger (3) it swings along the course
 * and up and down (4); hit, it falls (5), lands and fades (6), and is gone (7). */
s32 hellbatActType2(Obj *o) {
    Hellbat *w = HB(o);
    s32 r;
    s32 pad1;
    s32 pad2;
    s32 pad3;

    eneFall(o, w->gravity * 0.01f * gRetracesPerFrame);
    eneMove((EneObj *) w);
    eneSetModelOnCourse(o, &w->model->f);
    if (hellbatAnimType2[w->state].loop == 1) {
        eneAnimLoop(o, w->model, w->speed);
    } else {
        eneAnimPlayOnce(o, w->model, w->speed);
    }
    if (w->shadow != NULL) {
        shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.3f, NULL);
    }
    if (acollDmgWasHit(w->dmg) == 1 && --w->hits == 0) {
        hellbatSetAnime(o, 5, hellbatAnimType2);
        w->gravity = 25;
        w->dmg->flags &= ~2;
        w->atk->flags &= ~2;
        hellbatKillReward(o, w->dmg, 0);
        frmMove(&w->model->f, gFrmLayer8);
    }
    switch (w->state) {
        case 0:
            w->atk->radius = 30;
            w->atk->ofs[1] = 20;
            w->atk->kind = 120;
            w->dmg->radius = 50;
            w->dmg->height = 100;
            w->dmg->ofs[1] = -20;
            w->model->scale[2] = 0.06f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->target = hellbatNearestPl(o);
            hellbatSetAnime(o, 1, hellbatAnimType2);
        case 1:
            w->unused54 = fadeRandRange(10, 30);
            w->phase = fadeRandRange(-180, 180) * 0.01745329238f;
            w->phase2 = fadeRandRange(0, 90) * 0.01745329238f;
            hellbatSetAnime(o, 2, hellbatAnimType2);
            break;
        case 2:
            w->side = -1;
            w->rot[1] = hellbatPlAngle(o, w->target);
            break;
        case 3:
            if (joyGetPadHist(0, 0)->button & R_TRIG) {
                w->vel[0] = w->side ? gRetracesPerFrame * -0.3f : gRetracesPerFrame * 0.3f;
                w->phase = fadeRandRange(0, 180) * 0.01745329238f;
                hellbatSetAnime(o, 4, hellbatAnimType2);
            }
            break;
        case 4:
            w->phase += 10.0f * gRetracesPerFrame * 0.01745329238f;
            w->vel[1] = 0.3f * gGameState.retracesPerFrame * mathSin(w->phase);
            w->vel[0] = (w->side ? gGameState.retracesPerFrame * -0.5f : gGameState.retracesPerFrame * 0.5f) *
                        (fabs(mathCos(w->phase - 1.570796371f)) + 0.5);
            w->speed = (mathCos(w->phase) + 1.0f) * (gRetracesPerFrame * 2.0f);
            break;
        case 5:
            r = eneLandOff((EneObj *) w, 6.0f, -5.0f);
            w->rot[1] += 15.0f * gGameState.retracesPerFrame * 0.01745329238f;
            if (r) {
                w->gravity = 0;
                w->vel[2] = 0.0f;
                w->vel[0] = 0.0f;
                w->vel[1] = 0.0f;
                w->timer = 40 / gGameState.retracesPerFrame;
                w->side = -1;
                w->model->scale[0] *= 0.7;
                hellbatSetAnime(o, 6, hellbatAnimType2);
            }
            break;
        case 6:
            if (--w->timer == 0) {
                hellbatSetAnime(o, 7, hellbatAnimType2);
            }
            if (w->timer < 40 / gRetracesPerFrame && w->shadow != NULL) {
                w->shadow = (MPolFrame *) frmDestroy(&w->shadow->f);
            }
            w->alpha -= (u8) (255 / (40 / gRetracesPerFrame));
            w->model->scale[1] -= 0.1f / (40 / gRetracesPerFrame) * 0.5f;
            break;
        case 7:
            PRINTF("*** hellbat:type2:dead ***\n");
            eneReleaseSpawn(w->spawn);
            return -1;
    }
    return 0;
}
