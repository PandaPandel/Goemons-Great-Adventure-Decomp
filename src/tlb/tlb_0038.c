#include "tlb.h"

/* The gasya boss (file 38): its main task ("gasya_main"), body, hand, wall, flame, bomb, naniwa
 * and hasi tasks. Read in IDA (files/0038_08000000.bin.i64). */

/* The body's work ("gasya_body"): its proc keeps s0 = obj + 0x20. */
typedef struct GasyaBodyW {
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
    /* 0x34 */ AnimeFrame *hand; /* eneNewAnime2 (gasyaBodyHandAnim), the model the sparks follow */
    /* 0x38 */ AnimeFrame *anim; /* eneNewAnime2 (gasyaBodyAnim) */
    /* 0x3C */ ACollFrame *dmg; /* acollDmgAddYZ */
    /* 0x40 */ LightFrame *light; /* both models' */
    /* 0x44 */ Obj *bomb; /* kind 16, held then thrown (gasyaBombThrow) */
    /* 0x48 */ s32 timer;
    /* 0x4C */ s32 sound; /* a sound played once */
    /* 0x50 */ u8 pad50[0x4];
    /* 0x54 */ s32 hp; /* 12; it pauses at 9, 6 and 3 */
    /* 0x58 */ f32 rotGoal; /* the heading rot[1] turns to */
    /* 0x5C */ f32 aim; /* the heading anim's rot[1] turns to */
    /* 0x60 */ f32 baseX; /* where it bobs about */
    /* 0x64 */ f32 baseY;
    /* 0x68 */ f32 phase1; /* the bob's two angles */
    /* 0x6C */ f32 phase2;
    /* 0x70 */ f32 turn; /* its turn a frame when hit */
    /* 0x74 */ f32 vy; /* its fall's */
    /* 0x78 */ u8 fallen;
    /* 0x79 */ u8 tint; /* set by the body's proc (0 at its start), read by the hand: its tint */
    /* 0x7A */ u8 tintGoal;
    /* 0x7B */ u8 blink;
    /* 0x7C */ u8 bob; /* the bob's kind (0xFF: none) */
    /* 0x7D */ u8 track; /* 1: anim turns to aim */
    /* 0x7E */ u8 target; /* the player it aims at, from 1 (0: all of them) */
    /* 0x7F */ u8 attacks;
    /* 0x80 */ s32 wait;
} GasyaBodyW;

/* The hand's work ("gasya_hand"). */
typedef struct GasyaHandW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone; /* the course zone it makes enemies in */
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ AnimeFrame *anim; /* eneNewAnime */
    /* 0x38 */ AnimeFrame *point; /* where it makes them */
    /* 0x3C */ u8 pad3C[0x4];
    /* 0x40 */ u8 unused40; /* 0xFF or 0 from the body (gasyaHandSetHeld/ClearHeld), never read */
    /* 0x41 */ u8 pad41[0x7];
    /* 0x48 */ s32 timer;
    /* 0x4C */ s32 sound; /* 1: sound 0x2A2 to play, 2: sound 0x2A1 */
} GasyaHandW;

/* The main task's work ("gasya_main"). */
typedef struct GasyaMainW {
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
    /* 0x34 */ Obj *body;
    /* 0x38 */ Obj *hand;
    /* 0x3C */ Obj *wall;
    /* 0x40 */ Obj *piece1;
    /* 0x44 */ Obj *piece2;
    /* 0x48 */ s32 timer;
    /* 0x4C */ s32 sparkTimer;
    /* 0x50 */ s32 count;
    /* 0x54 */ s32 rainPeriod;
    /* 0x58 */ s32 rainTimer;
    /* 0x5C */ f32 windGoal;
    /* 0x60 */ f32 wind;
    /* 0x64 */ u8 rain; /* its kind; 0 for none */
    /* 0x65 */ u8 unused65; /* 1 from the fight's start to its end, never read */
    /* 0x66 */ u8 flashTimer; /* the sky's flash frames */
    /* 0x67 */ u8 pad67;
    /* 0x68 */ u8 ambient[3]; /* the sky's colour, kept while it flashes */
} GasyaMainW;

/* The enemy message 10 to gEneManager finds by this id (as tlb_0039's E61_ID). */
#define GASYA_ENE_ID 0xCF264

typedef struct GasyaEneW {
    /* 0x00 */ u8 unused00;
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
    /* 0x34 */ u8 pad34[8];
    /* 0x3C */ f32 speed;
} GasyaEneW;

/* A position returned by value. */
typedef struct GasyaPos {
    f32 v[3];
} GasyaPos;

#define GASYA_MAIN(o) ((GasyaMainW *) FX(o))
#define GASYA_BODY(o) ((GasyaBodyW *) FX(o))
#define GASYA_HAND(o) ((GasyaHandW *) FX(o))

u8 gasyaBodyGetState(Obj *body);
void gasyaBodyStart(Obj *body);
u8 gasyaHandGetState(Obj *hand);
void gasyaHandStart(Obj *hand);
s32 gasyaNaniwaCheckHand(Obj *naniwa, Obj *hand);

/* ptclSpawnSparksColor as the game calls it here, with a ninth word (1). */
typedef void (*SparksColor9Func)(f32 x, f32 y, f32 z, u8 r, u8 g, u8 b, f32 sx, f32 sy, s32 n);

/* "gasya_main", the gasha boss's director. Once the boss is beaten (save flag 0x80 at 0x107) it
 * only plays sound 0x105 while a player stands at the gate (course 2, place 1700-2250), and
 * unloads set 8. Otherwise: a player within 50 makes the AI camera hold on the gate, the players
 * walk in (1980, 1960, ...), the body, hand and wall rise (kinds 12, 13, 17), rain starts and
 * the sky turns; when the walk ends two more pieces come (kind 14); the hand slams, the body
 * roars (sound 0x18) and the players take control; the rain changes every 70 frames until the
 * body is done (25), and 20 frames later the camera lets go, the day tunes return, the save flag
 * is set and the boss's tasks go. Each frame the sky flashes while its count runs, the rain and
 * wind are drawn, and the wall goes when the hand says so.
 * 2026-10-12 (from real 396, a first draft): a switch on msg with an empty case 1 (the
 * game tests 0, 1, 2), w->state for every GASYA_MAIN(obj)->state and the second rain burst in the
 * first's order (281); w = GASYA_MAIN(obj) once at the top, not per branch (the game keeps w in s4
 * and rebuilds it from obj's home): 101, length 0; the frame's unused words as FAKE pads (28);
 * the light is an AmbFrame (its colour at 0x14); pad0 for the odd local count that puts an
 * alignment word under the temps (11); "for (i = 0, near = 0; ...)" and the same for n (7);
 * goal.x first: MATCH. The pads are words the game's frame has that no variable here uses. */
s32 gasyaMainProc(Obj *obj, s16 msg, void *arg);
s32 gasyaMainProc(Obj *obj, s16 msg, void *arg) {
    s32 pad0; /* FAKE */
    Camera *cam = camAiGetCam();
    GasyaMainW *w = GASYA_MAIN(obj);
    f32 z;
    f32 y;
    f32 x;
    s32 zone;
    s32 pad1; /* FAKE */
    s32 near;
    Vec3f goal;
    f32 ex;
    f32 ez;
    s32 i;
    s32 pad2; /* FAKE */
    s32 n;
    s32 pad3[5]; /* FAKE */
    f32 ax;
    f32 ay;
    f32 az;
    s32 pad4; /* FAKE */
    f32 sx;
    f32 sz;
    AmbFrame *amb;

    switch (msg) {
    case 2:
        obj->name = "gasya_main";
        eneSetup(obj, arg);
        w->state = 0;
        w->rain = 0;
        w->rainTimer = 30;
        w->rainPeriod = 30;
        w->windGoal = 0.0f;
        break;
    case 1:
        break;
    case 0:
        if (gSaveData.stageFlags[0x5D] & 0x80) {
            for (i = 0, near = 0; i < 4; i++) {
                if (plIsInPlay(i)) {
                    plAskPos(gGameWork.players[i], &x, &y, &z, &zone);
                    if (zone == 2 && x >= 1700.0f && x <= 2250.0f) {
                        near = 1;
                    }
                }
            }
            switch (w->state) {
                case 0:
                    if (near) {
                        sndPlay(0x105);
                        floadFreeSet(8);
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (!near) {
                        sndPlay(0x8105);
                        w->state = 0;
                    }
                    break;
                case 2:
                    if (!near) {
                        w->state = 0;
                    }
                    break;
            }
        } else {
            switch (w->state) {
                case 0:
                    if (eneNearestPlayerDist(obj) < 50.0) {
                        w->state =1;
                        camAiSetHold(1);
                        camSetSpeed(cam, 80.0f);
                        vramCoursePointXZ(2, 1970.0f, 0.0f, &ex, &ez);
                        camMoveAt(cam, ex, 188.0f, ez);
                        vramCoursePointXZ(2, 1970.0f, 180.0f, &ex, &ez);
                        camMoveEye(cam, ex, 200.0f, ez);
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                plSetAction(gGameWork.players[i], 0xDC);
                                goal.x = 1980 - i * 20;
                                goal.y = 156.0f;
                                goal.z = 0.0f;
                                plDemoWalkTo(gGameWork.players[i], goal, 2.0f);
                            }
                        }
                        w->state =1;
                        camSunSetProgram(2);
                        camSunClearTunes();
                        w->unused65 = 1;
                        w->rain = 1;
                        w->rainPeriod = 2;
                        w->wind = -2.0f;
                        w->windGoal = -2.0f;
                        w->body = eneMakeUnder(obj, ENE_GASYA_BODY, w->zone, 2000.0f, 0.0f, -110.0f);
                        w->hand = eneMakeUnder(obj, ENE_GASYA_HAND, w->zone, 2030.0f, 155.0f, -30.0f);
                        w->wall = eneMakeUnder(obj, ENE_GASYA_NANIWA, w->zone, 2030.0f, 155.0f, 0.0f);
                    }
                    break;
                case 1:
                    for (i = 0, n = 0; i < 4; i++) {
                        if (plIsInPlay(i) && plIsDemoWalking(gGameWork.players[i])) {
                            n++;
                        }
                    }
                    if (n == 0) {
                        w->state =2;
                        w->rain = 2;
                        w->rainPeriod = 1;
                        w->windGoal = -4.0f;
                        w->piece1 = eneMakeUnder(obj, ENE_GASYA_WALL, w->zone, 1880.0f, 140.0f, 0.0f);
                        w->piece2 = eneMakeUnder(obj, ENE_GASYA_WALL, w->zone, 2025.0f, 140.0f, 0.0f);
                        camSunSetClockStop(1);
                    }
                    break;
                case 2:
                    gasyaHandStart(w->hand);
                    w->state =3;
                    break;
                case 3:
                    if (gasyaHandGetState(w->hand) == 2) {
                        gasyaBodyStart(w->body);
                        sndPlay(0x18);
                        w->state =4;
                    }
                    break;
                case 4:
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            plSetAction(gGameWork.players[i], 0);
                        }
                    }
                    w->state =5;
                    break;
                case 5:
                    w->rain = 3;
                    w->rainPeriod = 1;
                    if (w->count++ >= 70) {
                        w->count = 0;
                        if (w->windGoal == -2.0) {
                            w->windGoal = -6.0f;
                        } else {
                            w->windGoal = -2.0f;
                        }
                    }
                    if (gasyaBodyGetState(w->body) == 25) {
                        w->state =6;
                    }
                    break;
                case 6:
                    w->rain = 0;
                    w->timer = 20;
                    camSunSetClockStop(0);
                    camSunSetProgram(0);
                    w->state =7;
                case 7:
                    if (w->timer-- <= 0) {
                        w->unused65 = 0;
                        camAiSetHold(0);
                        camSunSetTunes5(0x518, 0x36, 0x37, 0x38, 0x39);
                        gSaveData.stageFlags[0x5D] |= 0x80;
                        tskDestroyChildren(obj);
                        w->state =2;
                        while (gfxGetPendingSwaps()) {
                        }
                        floadFreeSet(8);
                    }
                    break;
            }
            if ((amb = (AmbFrame *) camSunGetAmbient()) != NULL) {
                if (w->flashTimer != 0) {
                    if (--w->flashTimer != 0) {
                        camSunSetColor(0, 0, 0);
                        amb->color[0] = 0;
                        amb->color[1] = 0;
                        amb->color[2] = 0;
                    } else {
                        amb->color[0] = w->ambient[0];
                        amb->color[1] = w->ambient[1];
                        amb->color[2] = w->ambient[2];
                    }
                } else {
                    w->ambient[0] = amb->color[0];
                    w->ambient[1] = amb->color[1];
                    w->ambient[2] = amb->color[2];
                }
            }
            if (w->rain != 0) {
                if (w->sparkTimer-- <= 0) {
                    w->sparkTimer = 30;
                    vramCoursePointXZ(2, fadeRand(80) + 1910, 0.0f, &sx, &sz);
                    ((SparksColor9Func) ptclSpawnSparksColor)(sx, 156.0f, sz, 0xC0, 0xC0, 0xFF, 0.5f, 1.0f, 1);
                }
                if (w->wind < w->windGoal) {
                    w->wind += 0.05;
                }
                if (w->windGoal < w->wind) {
                    w->wind -= 0.05;
                }
                if (w->rainTimer-- <= 0) {
                    w->rainTimer = w->rainPeriod;
                    camGetAt(cam, &ax, &ay, &az);
                    ax += fadeRand(140) - 70;
                    ay = 250.0f;
                    az += fadeRand(140) - 70;
                    ptclSpawnRain(ax, ay, az, w->wind, -12.0f, 0.0f, fadeRand(10) + 50, 6, w->rain, 8);
                    camGetAt(cam, &ax, &ay, &az);
                    ax += fadeRand(140) - 70;
                    ay = 250.0f;
                    az += fadeRand(140) - 70;
                    ptclSpawnRain(ax, ay, az, w->wind, -13.0f, 0.0f, fadeRand(10) + 50, 5, w->rain, 7);
                }
            }
            if (w->wall != NULL && gasyaNaniwaCheckHand(w->wall, w->hand)) {
                w->wall = NULL;
            }
        }
        break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A sparkle at the course's point 1970 along (200 to the side), then main's byte 0x66 = 5. */
void gasyaMainFlash(Obj *o) {
    s32 pad;
    f32 x;
    f32 z;
    s32 pad2;

    vramCoursePointXZ(2, 1970.0f, -200.0f, &x, &z);
    effectKiranCreateSized(x, 170.0f, z, 100.0f, 0x80, 12);
    GASYA_MAIN(o)->flashTimer = 5;
}

extern u8 gasyaBodyHandAnim[]; /* the hand's animations */
extern u8 gasyaBodyHandState3Anim[];
extern u8 gasyaBodyHandState19Anim[];
extern u8 gasyaBodyAnim[]; /* the body's */
extern u8 gasyaBodyState3Anim[];
extern u8 gasyaBodyState12Anim[];
extern u8 gasyaBodyState19Anim[];
void gasyaBodyPointFollow(Obj *o);
void gasyaBodySetDmgOfs(Obj *o);
f32 gasyaBodyCalcAim(Obj *body);
Obj *gasyaBodyMakeEnemies(Obj *o);
void gasyaHandSetHeld(Obj *hand);
void gasyaHandClearHeld(Obj *hand);
void gasyaHandSwing(Obj *hand);
void gasyaHandWithdraw(Obj *hand);
void gasyaBombThrow(Obj *o, u8 target);
void gasyaHasiSetSpeed(f32 v);

/* acollDmgAddYZ returns its box (main declares it void). */
typedef ACollFrame *(*BodyDmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                        s32 a);

/* "gasya_body", the gasha boss: two lit models (the hand gasyaBodyHandAnim and anim gasyaBodyAnim) under a
 * light, a damage box (13 by 60) and 12 hit points; message 3 tints both (blinking while hit).
 * It rises (1, sounds 0x2B9 and 0x129) and then loops: it throws a bomb (kind 16) at a player
 * three times (4-10, it sinks and rises between), then slams with the hand (11-16,
 * gasyaBodyMakeEnemies making enemies). A hit (17/18) turns it about; at 9, 6 and 3 points it
 * rears (19/20), at 0 it falls (22-24: pompoms, sounds 0x11B, 0x10E, 0x810E, rumbling) and is
 * done (25). Each frame it turns anim to aim, rot[1] to rotGoal, bobs about baseX/baseY by its
 * bob's kind and moves its tint 4 a frame to tintGoal.
 * Matched 2026-10-16; it was parked for the register of the hit's old hit points (a0 in the
 * game). The hit reads hp into old; the tint's step reuses hp and old. */
s32 gasyaBodyProc(Obj *o, s16 msg, void *arg);
s32 gasyaBodyProc(Obj *o, s16 msg, void *arg) {
    GasyaBodyW *w = GASYA_BODY(o);
    s32 t;
    s32 hp;
    f32 d;
    s32 old;
    s32 pad[2]; /* FAKE: frame words (one was the turn size, now a ?: expression) */

    switch (msg) {
        case 2:
            o->name = "gasya_body";
            eneSetup(o, arg);
            w->light = (LightFrame *) frmCreate(gFrmLayer2, gFrmKindLight);
            w->hand = eneNewAnime2(o, (u32) gasyaBodyHandAnim, gDrawSetupLitTexEdgeDl);
            w->anim = eneNewAnime2(o, (u32) gasyaBodyAnim, gDrawSetupLitTexEdgeDl);
            if (w->light == NULL || w->hand == NULL || w->anim == NULL) {
                return -1;
            }
            w->light->lightNum = 1;
            w->light->color[0] = 0xC0;
            w->light->color[1] = 0xC0;
            w->light->color[2] = 0xC0;
            w->light->dir[0] = 0;
            w->light->dir[1] = -127;
            w->light->dir[2] = 0;
            frmMove(&w->hand->f, &w->light->f);
            w->hand->scale[2] = 1.0f;
            w->hand->scale[0] = w->hand->scale[1] = w->hand->scale[2];
            w->rot[1] = w->rotGoal = eneCourseAngle(o, 8);
            frmMove(&w->anim->f, &w->light->f);
            w->anim->scale[2] = 1.0f;
            w->anim->scale[0] = w->anim->scale[1] = w->anim->scale[2];
            w->anim->rot[1] = w->aim = eneCourseAngle(o, 8);
            w->dmg = ((BodyDmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->anim->f, 13, 60, -30, 0, 0);
            w->tint = w->tintGoal = 0;
            w->bob = 0xFF;
            w->track = 1;
            w->hp = 12;
            w->target = 0;
            break;
        case 3:
            if (w->blink & 2) {
                w->anim->dl = gGfxDlHead;
                gSPDisplayList(gGfxDlHead++, gDrawSetupEnvTintTexDl);
                gDPSetEnvColor(gGfxDlHead++, 0xC0, 0xC0, 0xC0, 0xA0);
                gDPSetRenderMode(gGfxDlHead++, 0x552038, 0);
                gSPEndDisplayList(gGfxDlHead++);
            } else {
                w->anim->dl = gGfxDlHead;
                gSPDisplayList(gGfxDlHead++, gDrawSetupEnvTintTexShadeDl);
                gDPSetEnvColor(gGfxDlHead++, 0xFF, 0, 0, w->tint);
                gDPSetRenderMode(gGfxDlHead++, 0xC192038, 0);
                gSPEndDisplayList(gGfxDlHead++);
            }
            w->hand->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, gDrawSetupEnvTintTexShadeDl);
            gDPSetEnvColor(gGfxDlHead++, 0xFF, 0, 0, w->tint);
            gDPSetRenderMode(gGfxDlHead++, 0xC192038, 0);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            if (w->light != NULL) {
                frmDestroy(&w->light->f);
            }
            if (w->hand != NULL) {
                frmDestroy(&w->hand->f);
            }
            if (w->anim != NULL) {
                frmDestroy(&w->anim->f);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            break;
        case 0:
            if (acollDmgWasHit(w->dmg) == 1 && w->state >= 3 && w->state < 17) {
                t = shimAtkKindCheck(w->dmg);
                old = w->hp;
                w->hp = old - t;
                if (old >= 10) {
                    if (w->hp < 10) {
                        w->hp = 9;
                    }
                } else if (old >= 7) {
                    if (w->hp < 7) {
                        w->hp = 6;
                    }
                } else if (old >= 4) {
                    if (w->hp < 4) {
                        w->hp = 3;
                    }
                } else if (w->hp <= 0) {
                    w->hp = 0;
                }
                w->state = 17;
                eneHitSound(w->dmg);
            }
            switch (w->state) {
                case 0:
                    w->sound = 0;
                    break;
                case 1:
                    w->aim = gasyaBodyCalcAim(o);
                    eneAnimPlayOnce(o, w->hand, 0.5f);
                    eneAnimPlayOnce(o, w->anim, 0.5f);
                    /* FAKE: an empty test of hp (permuter, focused run on the split form); one more
                     * use of hp lowers its priority, so uopt colours tint's range, then hp, then old */
                    if (hp) {
                    } else {
                    }
                    if (w->anim->frame == 23.0) {
                        sndPlay(0x2B9);
                        eneRumbleAll(0x13579ADF, 30);
                    }
                    w->y += 2.0833333333333335;
                    if (w->y >= 125.0) {
                        w->y = 125.0f;
                    }
                    if (polAnimeLastFrame(w->hand) - 7 <= w->hand->frame && w->sound == 0) {
                        w->sound = 1;
                        sndPlay(0x129);
                    }
                    if (w->hand->frame == polAnimeLastFrame(w->hand) && w->anim->frame == polAnimeLastFrame(w->anim) &&
                        w->y == 125.0) {
                        gasyaMainFlash(o->parent);
                        w->wait = 80;
                        w->state = 3;
                        w->baseX = w->x;
                        w->baseY = w->y;
                    }
                    break;
                case 3:
                    w->hand->anim = gasyaBodyHandState3Anim;
                    w->hand->frame = 0.0f;
                    w->anim->anim = gasyaBodyState3Anim;
                    w->anim->frame = 0.0f;
                    w->timer = 40;
                    w->target = 0;
                    gasyaHandClearHeld(o->next);
                    w->bob = 0;
                    w->track = 1;
                    w->tintGoal = 0;
                    w->blink = 0;
                    w->dmg->flags |= 2;
                    w->state = 4;
                case 4:
                    if (w->timer-- < 0 && eneAnimPlayOnce(o, w->anim, 0.2f)) {
                        w->timer = 40;
                        w->anim->frame = 0.0f;
                    }
                    w->rotGoal = eneCourseAngle(o, 8) + 0.09817477f;
                    w->aim = gasyaBodyCalcAim(o);
                    if (w->wait-- <= 0) {
                        switch (w->attacks) {
                            case 0:
                                w->state = 5;
                                w->wait = 40;
                                w->attacks++;
                                break;
                            case 1:
                                w->state = 5;
                                w->wait = 40;
                                w->attacks++;
                                break;
                            case 2:
                                w->state = 5;
                                w->wait = 80;
                                w->attacks++;
                                break;
                            default:
                                w->state = 11;
                                w->wait = 100;
                                w->attacks = 0;
                                break;
                        }
                    }
                    break;
                case 5:
                    gasyaHandSetHeld(o->next);
                    w->track = 1;
                    w->target = eneRandomPlayerIndex() + 1;
                    w->dmg->flags &= ~2;
                    w->state = 6;
                case 6:
                    w->aim = gasyaBodyCalcAim(o);
                    if (w->anim->frame == 0.0f || eneAnimPlayOnce(o, w->anim, 0.2f)) {
                        w->tintGoal = 0x30;
                        w->anim->frame = 0.0f;
                        w->state = 7;
                    }
                    break;
                case 7:
                    w->aim = gasyaBodyCalcAim(o);
                    eneAnimPlayOnce(o, w->anim, 0.4f);
                    if (w->anim->frame >= 3.0f) {
                        if ((w->bomb = eneMakeUnder(o, ENE_GASYA_BOMB, w->zone, w->anim->pos[0], w->anim->pos[1], w->anim->pos[2])) ==
                            NULL) {
                            w->state = 10;
                        }
                        w->anim->frame = 3.0f;
                        w->timer = 30;
                        w->state = 8;
                    }
                    break;
                case 8:
                    w->aim = gasyaBodyCalcAim(o);
                    if (w->timer-- <= 0) {
                        gasyaBombThrow(w->bomb, w->target);
                        w->tintGoal = 0;
                        w->state = 9;
                    }
                    break;
                case 9:
                    w->z -= 5.0f;
                    if (w->z <= -120.0f) {
                        w->state = 10;
                    }
                    break;
                case 10:
                    if (w->z++ >= -110.0f) {
                        w->z = -110.0f;
                        if (eneAnimPlayOnce(o, w->anim, 0.1f)) {
                            w->state = 3;
                        }
                    }
                    break;
                case 11:
                    w->hand->anim = gasyaBodyHandState3Anim;
                    w->hand->frame = 0.0f;
                    w->tintGoal = 0x60;
                    w->state = 12;
                    gasyaHandSetHeld(o->next);
                    w->track = 1;
                    w->aim = eneCourseAngle(o, 8);
                    w->dmg->flags &= ~2;
                case 12:
                    eneAnimPlayOnce(o, w->anim, 0.5f);
                    if (w->tint == w->tintGoal) {
                        w->anim->anim = gasyaBodyState12Anim;
                        w->anim->frame = 0.0f;
                        w->bob = 1;
                        w->timer = 4;
                        w->sound = 0;
                        w->state = 13;
                    }
                    break;
                case 13:
                    if (w->hand->frame < 12.0f) {
                        eneAnimPlayOnce(o, w->hand, 0.4f);
                        eneAnimPlayOnce(o, w->anim, 0.4f);
                    } else {
                        w->sound = 1;
                        w->state = 14;
                    }
                    break;
                case 14:
                    eneAnimPlayOnce(o, w->hand, 0.18f);
                    eneAnimPlayOnce(o, w->anim, 0.18f);
                    if (w->hand->frame <= 26.0f) {
                        gasyaBodyMakeEnemies(o);
                    }
                    if (polAnimeLastFrame(w->hand) <= w->hand->frame && polAnimeLastFrame(w->anim) <= w->anim->frame) {
                        if (w->hp < 7) {
                            w->bob = 2;
                            w->timer = 20;
                            w->state = 15;
                        } else {
                            w->state = 3;
                        }
                    }
                    break;
                case 15:
                    if (w->timer-- <= 0) {
                        w->sound = 2;
                        w->timer = 4;
                        w->state = 16;
                    }
                    break;
                case 16:
                    eneAnimPlayOnce(o, w->hand, -0.18f);
                    eneAnimPlayOnce(o, w->anim, -0.18f);
                    gasyaBodyMakeEnemies(o);
                    if (w->hand->frame == 0.0f && w->anim->frame == 0.0f) {
                        w->state = 3;
                    }
                    break;
                case 17:
                    w->aim = w->anim->rot[1];
                    if (mathAtan2(w->dmg->hitPos[0] - w->anim->pos[0], w->dmg->hitPos[2] - w->anim->pos[2]) <=
                        w->aim - 0.5061454971631368) {
                        w->turn = 0.13962635f;
                    } else {
                        w->turn = -0.13962635f;
                    }
                    w->bob = 0;
                    w->track = 0;
                    w->tintGoal = 0;
                    w->state = 18;
                case 18:
                    w->blink++;
                    if (w->aim < w->anim->rot[1]) {
                        w->turn -= 0.010471976f;
                    }
                    if (w->anim->rot[1] < w->aim) {
                        w->turn += 0.010471976f;
                    }
                    eneAnimPlayOnce(o, w->anim, 0.2f);
                    if (w->aim < w->anim->rot[1]) {
                        d =w->anim->rot[1] - w->aim;
                    } else {
                        d = -(w->anim->rot[1] - w->aim);
                    }
                    if (d / 2 < (w->turn > 0.0f ? w->turn : -w->turn)) {
                        if (w->turn < 0.0f) {
                            w->turn += 0.012217305f;
                            if (w->turn >= 0.0f) {
                                w->turn = 0.0f;
                            }
                        } else {
                            w->turn -= 0.012217305f;
                            if (w->turn <= 0.0f) {
                                w->turn = 0.0f;
                            }
                        }
                        if (w->turn == 0.0f) {
                            w->anim->rot[1] = w->aim;
                            w->track = 1;
                            w->blink = 0;
                            hp = w->hp;
                            if (hp == 0) {
                                w->state = 22;
                            } else if (hp == 3 || hp == 6 || hp == 9) {
                                w->state = 19;
                            } else {
                                w->state = 3;
                            }
                        }
                    }
                    w->anim->rot[1] += w->turn;
                    break;
                case 19:
                    w->hand->anim = gasyaBodyHandState19Anim;
                    w->hand->frame = 0.0f;
                    w->anim->anim = gasyaBodyState19Anim;
                    w->anim->frame = 0.0f;
                    gasyaHandSwing(o->next);
                    w->aim = eneCourseAngle(o, 8);
                    w->state = 20;
                case 20:
                    eneAnimPlayOnce(o, w->hand, 0.1f);
                    eneAnimPlayOnce(o, w->anim, 0.1f);
                    if (w->hand->frame == polAnimeLastFrame(w->hand) && w->anim->frame == polAnimeLastFrame(w->anim)) {
                        w->blink = 0;
                        w->state = 11;
                    }
                    break;
                case 22:
                    w->hand->anim = gasyaBodyHandState19Anim;
                    w->hand->frame = 0.0f;
                    w->anim->anim = gasyaBodyState19Anim;
                    w->anim->frame = 0.0f;
                    gasyaHandSwing(o->next);
                    gasyaHandSetHeld(o->next);
                    w->aim = eneCourseAngle(o, 8);
                    w->bob = 3;
                    w->timer = 35;
                    w->state = 23;
                case 23:
                    eneAnimPlayOnce(o, w->hand, 0.05f);
                    eneAnimPlayOnce(o, w->anim, 0.05f);
                    if (w->timer-- == 0) {
                        sndPlay(0x2B9);
                        eneRumbleAll(0x13579ADF, 50);
                    }
                    gasyaHandSetHeld(o->next);
                    if (w->hand->frame >= 4.0f && w->anim->frame >= 4.0f) {
                        w->hand->frame = 4.0f;
                        w->anim->frame = 4.0f;
                        w->bob = 0xFF;
                        w->phase1 = 1.5707964f;
                        gasyaHandWithdraw(o->next);
                        effectPompomCreate(w->hand->pos[0], w->hand->pos[1] + 20.0f, w->hand->pos[2], 35.0f, 80.0f,
                                           35.0f, 10, 100, 1, 0.2f, 0.08f);
                        itexSparkleYellow(w->anim->pos[0], w->anim->pos[1], w->anim->pos[2], 0.3f);
                        w->state = 24;
                        w->timer = 7;
                        w->sound = 0;
                        sndPlay(0x11B);
                        sndPlay(0x10E);
                        eneRumbleAll(-1, 50);
                    }
                    break;
                case 24:
                    eneAnimPlayOnce(o, w->anim, 0.05f);
                    w->vy -= 0.4f;
                    w->anim->pos[1] += w->vy;
                    w->anim->pos[2] -= 0.3f;
                    if (!w->fallen) {
                        w->anim->rot[2] += 0.05235988274f;
                        if (w->anim->pos[1] < 160.0f) {
                            w->fallen = 1;
                            w->vy = 5.0f;
                            eneRumbleAll(-1, 100);
                            gasyaHasiSetSpeed(2.0f);
                        }
                    } else {
                        w->anim->rot[0] += 0.05235988274f;
                    }
                    if (w->anim->pos[1] <= 120.0f) {
                        w->anim->pos[1] = 120.0f;
                        w->anim->pos[2] = 10.0f;
                    }
                    eneAnimPlayOnce(o, w->hand, 0.07f);
                    w->phase1 += 3.1415927f;
                    w->x = mathSin(w->phase1) + w->baseX;
                    w->y -= 0.8f;
                    if (w->timer-- <= 0) {
                        w->timer = fadeRand(2) + 5;
                        sndPlay(0x513);
                    }
                    if (w->y < 15.0f && w->sound == 0) {
                        w->sound = 1;
                        sndPlay(8);
                    }
                    if (w->y < -10.0) {
                        sndPlay(0x810E);
                        w->state = 25;
                    }
                    break;
                case 25:
                    eneRumbleStopAll();
                    break;
            }
            eneSetModelOnCourse(o, &w->hand->f);
            if (w->state < 24) {
                gasyaBodyPointFollow(o);
            }
            if (w->track == 1) {
                if ((w->anim->rot[1] < w->aim ? w->aim - w->anim->rot[1] : -(w->aim - w->anim->rot[1])) <
                    0.02094395f) {
                    w->anim->rot[1] = w->aim;
                } else if (w->anim->rot[1] < w->aim) {
                    w->anim->rot[1] += 0.02094395f;
                } else if (w->aim < w->anim->rot[1]) {
                    w->anim->rot[1] -= 0.02094395f;
                }
            }
            gasyaBodySetDmgOfs(o);
            if ((w->rot[1] < w->rotGoal ? w->rotGoal - w->rot[1] : -(w->rotGoal - w->rot[1])) <= 0.0026179939f) {
                w->rot[1] = w->rotGoal;
            } else if (w->rot[1] < w->rotGoal) {
                w->rot[1] += 0.0026179939f;
            } else if (w->rotGoal < w->rot[1]) {
                w->rot[1] -= 0.0026179939f;
            }
            if (w->bob != 0xFF) {
                if (w->bob == 0) {
                    w->phase1 += 0.02617994137f;
                    if (w->phase1 > 6.2831855f) {
                        w->phase1 -= 6.2831855f;
                    }
                    w->phase2 += 0.08726647f;
                    if (w->phase2 > 6.2831855f) {
                        w->phase2 -= 6.2831855f;
                    }
                } else if (w->bob == 1) {
                    if (w->phase1 < 1.5707964f) {
                        w->phase1 += 6.2831855f;
                    }
                    w->phase1 -= 1.5707964f;
                    if (w->phase1 <= 3.1415927f) {
                        w->phase1 -= 0.17453294f;
                        if (w->phase1 <= 0.0f) {
                            w->phase1 = 0.0f;
                        }
                    } else {
                        w->phase1 += 0.13962635f;
                        if (w->phase1 >= 6.2831855f) {
                            w->phase1 = 0.0f;
                        }
                    }
                    w->phase1 += 1.5707964f;
                    w->phase2 += 0.08726647f;
                    if (w->phase2 > 6.2831855f) {
                        w->phase2 -= 6.2831855f;
                    }
                } else if (w->bob == 2) {
                    if (w->phase1 > 4.712389f) {
                        w->phase1 -= 6.2831855f;
                    }
                    w->phase1 += 1.5707964f;
                    if (w->phase1 <= 3.1415927f) {
                        w->phase1 -= 0.17453294f;
                        if (w->phase1 <= 0.0f) {
                            w->phase1 = 0.0f;
                        }
                    } else {
                        w->phase1 += 0.13962635f;
                        if (w->phase1 >= 6.2831855f) {
                            w->phase1 = 0.0f;
                        }
                    }
                    w->phase1 -= 1.5707964f;
                    w->phase2 += 0.08726647f;
                    if (w->phase2 > 6.2831855f) {
                        w->phase2 -= 6.2831855f;
                    }
                } else if (w->bob == 3) {
                    w->phase1 += 0.13962635f;
                    if (w->phase1 > 6.2831855f) {
                        w->phase1 = 6.2831855f;
                    }
                    w->phase2 += 0.08726647f;
                    if (w->phase2 > 6.2831855f) {
                        w->phase2 = 6.2831855f;
                    }
                }
                w->x = mathSin(w->phase1) * 15.0f + w->baseX;
                w->y = mathSin(w->phase2) * 3.0f + w->baseY;
            }
            hp = w->tint - w->tintGoal;
            old = hp > 0 ? hp : -hp;
            if (old < 5) {
                w->tint = w->tintGoal;
            } else if (w->tintGoal < w->tint) {
                w->tint -= 4;
            } else if (w->tint < w->tintGoal) {
                w->tint += 4;
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The body's state (main waits for 2). */
u8 gasyaBodyGetState(Obj *body) {
    return GASYA_BODY(body)->state;
}

/* Main starts the body (state 1). */
void gasyaBodyStart(Obj *body) {
    GASYA_BODY(body)->state = 1;
}

/* The hand reads the body's byte 0x79. */
u8 gasyaBodyGetTint(Obj *body) {
    return GASYA_BODY(body)->tint;
}

/* Main's sinf and cosf, called through their addresses as TLB code calls main. */
#define sinf (*sinf)
#define cosf (*cosf)

/* mdlAnimNodePos with its frame as the f32 it is (tlb.h declares it s32, as model.c defines
 * it). */
typedef void (*NodePosFunc)(u32 addr, f32 frame, u32 base, f32 *x, f32 *y, f32 *z);

extern u8 gasyaBodyHandAnim[]; /* the hand's three animations, */
extern u8 gasyaBodyHandState3Anim[];
extern u8 gasyaBodyHandState19Anim[];
extern u8 gasyaBodyHandAttach[]; /* and each one's node its point hangs from */
extern u8 gasyaBodyHandState3Attach[];
extern u8 gasyaBodyHandState19Attach[];

/* Keeps the hand's point at its node of the hand's animation (the node by which animation is
 * on), turned with the hand's heading (tlb_0111's gasyaBodyFollow does the same for gasya's
 * body). Each turned coordinate is written as the anim's position plus the turn, with the
 * product whose call comes first on the left. */
void gasyaBodyPointFollow(Obj *o) {
    GasyaHandW *w = (GasyaHandW *) FX(o);
    f32 x;
    f32 y;
    f32 z;

    if (GASYA_HAND(o)->anim->anim == gasyaBodyHandAnim) {
        ((NodePosFunc) mdlAnimNodePos)((u32) gasyaBodyHandAttach, w->anim->frame, eneFileAddr(o, 0, 0), &x, &y, &z);
    } else if (w->anim->anim == gasyaBodyHandState3Anim) {
        ((NodePosFunc) mdlAnimNodePos)((u32) gasyaBodyHandState3Attach, w->anim->frame, eneFileAddr(o, 0, 0), &x, &y, &z);
    } else if (w->anim->anim == gasyaBodyHandState19Anim) {
        ((NodePosFunc) mdlAnimNodePos)((u32) gasyaBodyHandState19Attach, w->anim->frame, eneFileAddr(o, 0, 0), &x, &y, &z);
    }
    w->point->pos[0] = w->anim->pos[0] + (z * sinf(w->anim->rot[1]) + cosf(w->anim->rot[1]) * x);
    w->point->pos[2] = w->anim->pos[2] + (z * cosf(w->anim->rot[1]) - sinf(w->anim->rot[1]) * x);
    w->point->pos[1] = y + w->anim->pos[1];
}

/* The body's damage box, 35 out at 331 degrees.
 * Matched 2026-10-11; it was parked for w's spill slot (the game's 0x1C), which pad[5] gives. */
void gasyaBodySetDmgOfs(Obj *o) {
    GasyaBodyW *w = GASYA_BODY(o);
    s32 pad[5];

    w->dmg->ofs[0] = mathSin(5.77704f) * 35.0;
    w->dmg->ofs[2] = mathCos(5.77704f) * 35.0;
}

/* The body's aim: halfway from the course's heading to the players (their middle, or its target
 * player if it has one still in play), plus 29 degrees (0.506 rad).
 * 86 lines, frame 0x80 as the game's. Still different: the sums sit in f20/f22 the other way
 * round (declaring them in either order changes nothing); the positions are at 0x54-0x5C in
 * the game (0x6C-0x74 here); the work pointer (s0) is rebuilt in the else.
 * Sweep 2 2026-10-11: every local keeps a slot even in a register, so the order sets the homes:
 * "w; sumX; sumZ; n; course; i; aim; pad; px; py; pz" puts every slot at the game's: 86 to 66
 * (sumZ before sumX: 66). Left: the sums' f20/f22 and the work pointer.
 * 2026-10-11: a FAKE "if (w != NULL) {}" after the else's w (IDO_NOTES 344) builds w as the game
 * (addiu s0 after the target's load): 66 to 58. Left: body kept in a1 (the game reloads it from
 * its home in the else) and the sums' registers; the adds swapped, 58.
 * 2026-10-11: the result built in course (course = (aim - course) / 2 + course; course +=
 * 0.506; return course: the game's add.s/cvt into f2 then mov.s f0): 25 -> 20; "sumX = sumZ =
 * 0.0f" (sumX zeroed first in f22, as the game): 14. Left: body kept in a1 and spilled late
 * (the game spills a0 at entry and reloads it into s0 in the else, w = s0 + 32 after the
 * target's load). Not better: no FAKE (22); no w in the else, GASYA_BODY(body) throughout (22,
 * the target CSE'd from the first test); the sum in aim (40).
 * 2026-10-12 (real 12): the zeroing and the first test on one line 12; the test through
 * `*&body` (folded) 12. The game never gives body a register (home store at entry, every later
 * use a reload, the else's into s0 = w); here uopt keeps it in a1 until each branch spills it.
 * 2026-10-12 (second try): the else's test through GASYA_BODY(body) (FAKE kept) put the home store at
 * entry but CSE'd the target (20); no FAKE, w->target (20). MATCH: w = GASYA_BODY(body) once at
 * the top, the first test through w->target, no FAKE, no w in the else. */
f32 gasyaBodyCalcAim(Obj *body) {
    GasyaBodyW *w;
    f32 sumX;
    f32 sumZ;
    s32 n = 0;
    f32 course;
    s32 i;
    f32 aim;
    s32 pad;
    f32 px;
    f32 py;
    f32 pz;

    w = GASYA_BODY(body);
    sumX = sumZ = 0.0f;
    if (w->target == 0) {
        for (i = 0; i < 4; i++) {
            if (plIsInPlay(i)) {
                plGetModelPos(gGameWork.players[i], &px, &py, &pz);
                n++;
                sumX += px;
                sumZ += pz;
            }
        }
    } else {
        if (plIsInPlay(w->target - 1)) {
            plGetModelPos(gGameWork.players[w->target - 1], &px, &py, &pz);
            sumX = px;
            sumZ = pz;
            n = 1;
        } else {
            w->target = 0;
        }
    }
    course = eneCourseAngle(body, 8);
    if (n == 0) {
        aim = course;
    } else {
        aim = mathAtan2(sumX / n - GASYA_BODY(body)->anim->pos[0], sumZ / n - GASYA_BODY(body)->anim->pos[2]);
    }
    course = (aim - course) / 2 + course;
    course += 0.506145497163136837;
    return course;
}

/* The body's heading less 29 degrees (0.5061454971631368 rad); the bomb and gasyaBombThrow aim by it. */
f32 gasyaBodyThrowHeading(Obj *body) {
    return GASYA_BODY(body)->anim->rot[1] - 0.5061454971631368;
}

/* The body's position. */
GasyaPos gasyaBodyGetPos(Obj *body) {
    s32 pad;
    GasyaPos p;

    p.v[0] = GASYA_BODY(body)->anim->pos[0];
    p.v[1] = GASYA_BODY(body)->anim->pos[1];
    p.v[2] = GASYA_BODY(body)->anim->pos[2];
    return p;
}

/* The hand, between frames 15 and 27 of its animation: plays its pending sound, and every fifth
 * frame makes an enemy of kind 15 at the course place nearest its point, 20 below it. Returns
 * the enemy made, or NULL. The call's x and y are float locals (x, y, then side, made): as
 * compiler temps they would put the locals 2 words higher than the game's. */
Obj *gasyaBodyMakeEnemies(Obj *o) {
    GasyaHandW *w = GASYA_HAND(o);
    f32 y;
    f32 x;
    f32 side;
    Obj *made = NULL;

    if (GASYA_HAND(o)->anim->frame >= 15.0f && w->anim->frame <= 27.0f) {
        if (w->sound == 1) {
            w->sound = 0;
            sndPlay(0x2A2);
        }
        if (w->sound == 2) {
            w->sound = 0;
            sndPlay(0x2A1);
        }
        if (w->timer-- <= 0) {
            w->timer = 4;
            x = vramNearestPlaceXZ(w->zone, w->point->pos[0], w->point->pos[2], &side);
            y = w->point->pos[1] - 20.0f;
            made = eneMakeUnder(o, ENE_GASYA_FLAME, w->zone, x, y, side);
        }
    }
    return made;
}

/* "gasya_hand"'s work: an enemy's place, then its own. */
typedef struct GasyaSlamW {
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
    /* 0x34 */ AnimeFrame *anim;
    /* 0x38 */ s32 timer;
    /* 0x3C */ u8 pad3C[0x4];
    /* 0x40 */ u8 held; /* 1: holding at the end of its swing */
} GasyaSlamW;

extern u8 gasyaHandState3Anim[]; /* the hand's animations */
extern u8 gasyaHandState5Anim[];
void gasyaHasiSetSpeed(f32 v);

/* "gasya_hand", the gasya's slamming hand on layer 2, red, its brightness the body's. It waits
 * 30 frames, then (state 1) slams: sound 8 when the wait runs out, the swing at 0.2 to frame 9
 * and then at 1.0 while it comes forward 3 a frame; at the end it rumbles and sets the bridge
 * swinging fast (gasyaHasiSetSpeed 2), holds 60 frames and (states 3 to 4) sways at its swing's end. It can be made to swing
 * (5, 6, shaking 1), lift (7) and sink away (8, 9). */
s32 gasyaHandProc(Obj *o, s16 msg, void *arg) {
    s32 pad;
    GasyaSlamW *w = (GasyaSlamW *) FX(o);

    switch (msg) {
        case 2:
            o->name = "gasya_hand";
            eneSetup(o, arg);
            if ((w->anim = eneNewAnime(o, (u32) gasyaHandState3Anim)) == NULL) {
                return -1;
            }
            frmMove(&w->anim->f, gFrmLayer2);
            w->anim->scale[2] = 1.0f;
            w->anim->scale[0] = w->anim->scale[1] = w->anim->scale[2];
            w->rot[1] = eneCourseAngle(o, 8);
            eneSetModelOnCourse(o, &w->anim->f);
            w->state = 0;
            break;
        case 3:
            w->anim->dl = eneDlEnvColorLit(o, gasyaBodyGetTint(o->prev), 255, 0, 0, 1);
            break;
        case 1:
            if (w->anim != NULL) {
                frmDestroy(&w->anim->f);
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->timer = 30;
                    break;
                case 1:
                    if (w->timer-- == 0) {
                        sndPlay(8);
                    }
                    if (w->anim->frame >= 9.0f) {
                        eneAnimPlayOnce(o, w->anim, 1.0f);
                        w->z -= -3.0f;
                        if (w->z >= 0.0f) {
                            w->z = 0.0f;
                        }
                    } else {
                        eneAnimPlayOnce(o, w->anim, 0.2f);
                    }
                    if (w->anim->frame == polAnimeLastFrame(w->anim) && w->z == 0.0f) {
                        w->timer = 60;
                        eneRumbleAll(-1, 20);
                        gasyaHasiSetSpeed(2.0f);
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (w->timer-- <= 0) {
                        w->state = 3;
                    }
                    break;
                case 3:
                    w->anim->anim = gasyaHandState3Anim;
                    w->anim->frame = polAnimeLastFrame(w->anim);
                    w->held = 0;
                    w->state = 4;
                case 4:
                    if (w->held) {
                        if (eneAnimPlayOnce(o, w->anim, 0.3f) && w->held == 1) {
                            w->held = 0;
                        }
                    } else {
                        eneAnimPlayOnce(o, w->anim, -0.05f);
                        if (w->anim->frame <= polAnimeLastFrame(w->anim) - 2.0) {
                            w->anim->frame = polAnimeLastFrame(w->anim) - 2.0;
                            w->held = 1;
                        }
                    }
                    break;
                case 5:
                    if (eneAnimPlayOnce(o, w->anim, 0.5f)) {
                        w->anim->anim = gasyaHandState5Anim;
                        w->anim->frame = 0.0f;
                        w->held = 0;
                        w->state = 6;
                    }
                    break;
                case 6:
                    gasyaHasiSetSpeed(1.0f);
                    if (eneAnimPlayOnce(o, w->anim, 0.3f)) {
                        w->state = 3;
                    }
                    break;
                case 7:
                    eneAnimPlayOnce(o, w->anim, -0.1f);
                    if (w->anim->frame <= 14.0) {
                        w->anim->frame = 14.0f;
                        w->state = 8;
                    }
                    break;
                case 8:
                    if (w->y-- < 50.0) {
                        w->state = 9;
                        w->anim->anim = 0;
                        w->timer = 60;
                    }
                    break;
                case 9:
                    if (w->timer--) {
                        w->state = 10;
                        w->anim->anim = 0;
                    }
                    break;
                case 10:
                    break;
            }
            eneSetModelOnCourse(o, &w->anim->f);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The hand's state (main waits for 2). */
u8 gasyaHandGetState(Obj *hand) {
    return GASYA_HAND(hand)->state;
}

/* Main starts the hand (state 1). */
void gasyaHandStart(Obj *hand) {
    GASYA_HAND(hand)->state = 1;
}

/* The body sets the hand's byte 0x40 to 0xFF (the hand may be gone). */
void gasyaHandSetHeld(Obj *hand) {
    if (hand != NULL) {
        GASYA_HAND(hand)->unused40 = 0xFF;
    }
}

/* The body clears the hand's byte 0x40 (the hand may be gone). */
void gasyaHandClearHeld(Obj *hand) {
    if (hand != NULL) {
        GASYA_HAND(hand)->unused40 = 0;
    }
}

/* The body sets the hand's state 5. */
void gasyaHandSwing(Obj *hand) {
    GASYA_HAND(hand)->state = 5;
}

/* The body sets the hand's state 7. */
void gasyaHandWithdraw(Obj *hand) {
    GASYA_HAND(hand)->state = 7;
}

/* The hand's animation frame (naniwa reads it). */
f32 gasyaHandGetFrame(Obj *hand) {
    return GASYA_HAND(hand)->anim->frame;
}

/* "gasya_wall"'s work. */
typedef struct GasyaWallW {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rotX;
    /* 0x20 */ f32 heading;
    /* 0x24 */ f32 rotZ;
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *model;
    /* 0x38 */ RideFrame *ride;
} GasyaWallW;

extern u8 gasyaWallRide[]; /* the wall's collision (ride) data */

/* "gasya_wall": a wall model at 0.1 scale, turned to the course and set on it, that players
 * can stand on (a ride in gRideTree1). */
s32 gasyaWallProc(Obj *obj, s16 msg, void *arg) {
    GasyaWallW *w = (GasyaWallW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "gasya_wall";
            eneSetup(obj, arg);
            w->model = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[2] = 0.1f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->flags |= 0x20;
            w->heading = eneCourseAngle(obj, 8);
            eneSetModelOnCourse(obj, &w->model->f);
            w->ride = rideCreate(gRideTree1, (s32) obj, &w->model->f,
                                 (RideData *) eneFileAddr(obj, gasyaWallRide, 0));
            if (w->ride == NULL) {
                return -1;
            }
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->ride != NULL) {
                frmDestroy((Frame *) w->ride);
            }
            break;
        case 0:
            eneSetModelOnCourse(obj, &((GasyaWallW *) FX(obj))->model->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "g_flame"'s work: an enemy's place and speed, then its own. */
typedef struct GFlameW {
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
    /* 0x34 */ PolFrame *model;
    /* 0x38 */ Anim anim; /* its flicker */
    /* 0x48 */ ACollFrame *atk;
    /* 0x4C */ s32 timer;
} GFlameW;

typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);

extern u8 gasyaFlameDl[]; /* a flame */
void gasyaHasiSetSpeed(f32 v);

/* "g_flame", a flame the gasya spits: a yellow flickering flame (file 0x125, 0.3) 5 out from its
 * place, with an attack box (radius 15, kind 0x78), flying at 2 a frame toward (2010, 165, 0)
 * and growing 2.5% a frame. On reaching height 165 it slows (x by 4, y by 3, z by 2), rumbles
 * the controllers and sets the bridge swinging fast (gasyaHasiSetSpeed), then dies down and goes
 * 45 frames after it was made. Whoever its box hits gets message 7.
 * The aim is written dx, dy, dz (the game loads x first), with the timer's clear after vel[2]. */
s32 gasyaFlameProc(Obj *obj, s16 msg, void *arg) {
    s32 pad[3];
    s32 hitter;
    GFlameW *w = (GFlameW *) FX(obj);
    f32 dx;
    f32 dy;
    f32 dz;
    f32 len;

    switch (msg) {
        case 2:
            obj->name = "g_flame";
            eneSetup(obj, arg);
            if ((w->model = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                return -1;
            }
            w->model->dl2 = (Gfx *) gasyaFlameDl;
            floadGetSetRam((void **) w->model->segs, 0x125);
            fadeAnimStart(&w->anim, gItexTimedYellowAnim, 0, &w->model->segs[3], floadGetSetFile(0x125, 0));
            w->model->flags |= 7;
            w->model->scale[2] = 0.3f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->z += 5.0f;
            eneSetModelOnCourse(obj, &w->model->f);
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 15, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 0x78;
            dx = w->x - 2010.0;
            dy = 165.0 - w->y;
            dz = 0.0f - w->z;
            len = sqrtf(dx * dx + dy * dy + dz * dz);
            w->vel[0] = 2.0 * (dx / len);
            w->vel[1] = 2.0 * (dy / len);
            w->vel[2] = 2.0 * (dz / len);
            w->timer = 0;
            break;
        case 3:
            w->model->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombine(gGfxDlHead++, 0x309A61, 0x5536FF7F);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 63, 0);
            gDPSetEnvColor(gGfxDlHead++, 159, 0, 0, 255);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            break;
        case 0:
            if (w->atk != NULL && acollAtkWasHit(w->atk) == 1 && (hitter = w->atk->hitPower) != 0) {
                tskSendMsg((Obj *) hitter, 7, obj);
            }
            fadeAnimStep(&w->anim);
            w->timer++;
            switch (w->state) {
                case 0:
                    w->model->scale[0] *= 1.025;
                    w->model->scale[1] *= 1.025;
                    w->model->scale[2] *= 1.025;
                    if (w->y <= 165.0) {
                        w->vel[0] *= 0.25;
                        w->vel[1] /= 3.0;
                        w->vel[2] *= 0.5;
                        eneRumbleAll(0xBBBBBBBB, 5);
                        gasyaHasiSetSpeed(1.0f);
                        w->state = 1;
                    }
                    break;
                case 1:
                    w->vel[1] *= 0.98;
                    w->vel[2] *= 0.98;
                    if (w->timer >= 45) {
                        return -1;
                    }
                    break;
            }
            eneMove((EneObj *) w);
            eneSetModelOnCourse(obj, &w->model->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "gasya_bomb"'s work: an enemy's place and speed, then its own (g_flame's layout). */
typedef struct GBombW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 pad01[0xF];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ u8 pad1C[0xC];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *model;
    /* 0x38 */ Anim anim; /* its flicker, then its blast */
    /* 0x48 */ ACollFrame *atk;
    /* 0x4C */ s32 alpha;
} GBombW;

/* "gasya_bomb": a yellow flickering ball (file 0x125) held 25 out along the body's aim, 28 below
 * it, growing by 0.03 a frame to 0.45 and fading in. Thrown (state 1, gasyaBombThrow), it
 * flies until height 160, where it blasts: the bomb animation (file 0x11F, 0.4) at 147, its box
 * armed (kind 0x78, radius 20), the bridge set swinging fast, sound 0x228 and a rumble; it fades by 8
 * and goes with the blast. Whoever its box hits gets message 7.
 * Matched 2026-10-12; it was parked for the stack places of aim and the body's position. */
s32 gasyaBombProc(Obj *obj, s16 msg, void *arg) {
    GBombW *w = (GBombW *) FX(obj);
    s32 file;

    switch (msg) {
        case 2:
            obj->name = "gasya_bomb";
            if ((w->model = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                return -1;
            }
            w->model->dl2 = (Gfx *) gasyaFlameDl;
            floadGetSetRam((void **) w->model->segs, 0x125);
            file = floadGetSetFile(0x125, 0);
            fadeAnimStart(&w->anim, gItexTimedYellowAnim, 0, &w->model->segs[3], file);
            w->model->flags |= 7;
            w->model->scale[2] = 0.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->alpha = 0xFF;
            w->state = 0;
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 20, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 0x78;
            w->atk->flags &= ~2;
            break;
        case 1:
            /* FAKE: the first test and the next on one line (IDO_NOTES 347) */
            if (w->model != NULL) { frmDestroy(&w->model->f); } if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            break;
        case 3:
            w->model->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombine(gGfxDlHead++, 0x30D261, 0x55A6FF7F);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 63, 0);
            gDPSetEnvColor(gGfxDlHead++, 159, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            if (w->atk != NULL && acollAtkWasHit(w->atk) == 1 && (Obj *) w->atk->hitPower != NULL) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, obj);
            }
            fadeAnimStep(&w->anim);
            switch (w->state) {
                case 0: {
                    f32 aim;
                    GasyaPos p;

                    aim = gasyaBodyThrowHeading(obj->parent);
                    p = gasyaBodyGetPos(obj->parent);
                    w->x = p.v[0] + mathSin(aim) * 25.0;
                    w->y = p.v[1] - 28.0f;
                    w->z = p.v[2] + mathCos(aim) * 25.0;
                    w->model->scale[0] += 0.03f;
                    w->model->scale[1] += 0.03f;
                    w->model->scale[2] += 0.03f;
                    w->alpha += 31.875;
                    if (w->alpha >= 255.0) {
                        w->alpha = 0xFF;
                    }
                    if (w->model->scale[0] >= 0.45) {
                        w->model->scale[2] = 0.45f;
                        w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                    }
                    break;
                }
                case 1:
                    eneMove((EneObj *) w);
                    if (w->y <= 160.0) {
                        w->y = 147.0f;
                        w->model->dl2 = (Gfx *) gasyaFlameDl;
                        floadGetSetRam((void **) w->model->segs, 0x11F);
                        file = floadGetSetFile(0x11F, 0);
                        fadeAnimStart(&w->anim, gItexBombAnim, 0, &w->model->segs[3], file);
                        w->model->scale[2] = 0.4f;
                        w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                        w->atk->flags |= 2;
                        w->atk->ofs[1] = 20;
                        gasyaHasiSetSpeed(1.0f);
                        soundIncamCam(0x228, w->x, w->y, w->z);
                        eneRumbleAll(0xBBBBBBBB, 10);
                        w->state = 2;
                    }
                    break;
                case 2:
                    w->alpha -= 8;
                    if (fadeAnimStep(&w->anim)) {
                        return -1;
                    }
                    break;
            }
            eneSetModel(obj, &w->model->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Launches a bomb (state 1) with sound 0x138: with no target, along the body's aim (2 a frame
 * across, rising to height 160 over 15 frames); at player `target` (from 1), at 3 a frame
 * toward the course point under that player at height 160. */
void gasyaBombThrow(Obj *o, u8 target) {
    EneObj *w = ENE(o);
    s32 pad;
    f32 aim;
    f32 place;
    f32 height;
    f32 side;
    s32 zone;

    if (target == 0) {
        aim = gasyaBodyThrowHeading(o->parent);
        w->vel[0] = sinf(aim) * 2.0;
        w->vel[1] = (160.0 - w->y) / 15.0;
        w->vel[2] = cosf(aim) * 2.0;
        w->state = 1;
    } else {
        plAskPos(gGameWork.players[target - 1], &place, &height, &side, &zone);
        vramCoursePointXZ(zone, place, side, &place, &side);
        eneSetVelToward(o, 3.0f, place, 160.0f, side);
        w->state = 1;
    }
    soundIncamCam(0x138, w->x, w->y, w->z);
}

/* "gasya_naniwa"'s work. */
typedef struct NaniwaW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rotX;
    /* 0x20 */ f32 heading;
    /* 0x24 */ f32 rotZ;
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ AnimeFrame *anim;
    /* 0x38 */ MPolFrame *shadow;
} NaniwaW;

extern u8 gasyaNaniwaAnim[]; /* its animation */

/* "gasya_naniwa": an animated model at 0.15 scale with a shadow, turned to the course. In state
 * 1 it bursts (a yellow sparkle and falling sparks 10 above it, sound 0x11B) and goes. */
s32 gasyaNaniwaProc(Obj *o, s16 msg, void *arg) {
    NaniwaW *w = (NaniwaW *) FX(o);

    switch (msg) {
        case 2:
            o->name = "gasya_naniwa";
            eneSetup(o, arg);
            w->anim = eneNewAnime(o, (u32) gasyaNaniwaAnim);
            if (w->anim == NULL) {
                return -1;
            }
            w->anim->scale[2] = 0.15f;
            w->anim->scale[0] = w->anim->scale[1] = w->anim->scale[2];
            w->heading = eneCourseAngle(o, 4);
            w->shadow = shadowCreate();
            break;
        case 1:
            if (w->anim != NULL) {
                frmDestroy((Frame *) w->anim);
            }
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    eneAnimLoop(o, w->anim, 0.5f);
                    eneSetModelOnCourse(o, (Frame *) w->anim);
                    break;
                case 1:
                    itexSparkleYellow(w->anim->pos[0], w->anim->pos[1] + 10.0f, w->anim->pos[2], 0.3f);
                    ptclSpawnSparksFall(w->anim->pos[0], w->anim->pos[1] + 10.0f, w->anim->pos[2]);
                    soundIncamCam(0x11B, w->anim->pos[0], w->anim->pos[1] + 10.0f, w->anim->pos[2]);
                    return -1;
            }
            shadowPut(w->shadow, w->anim->pos[0], w->anim->pos[1], w->anim->pos[2], 2.0f, NULL);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The naniwa waits for the hand's animation to reach frame 16, then goes to state 1. */
s32 gasyaNaniwaCheckHand(Obj *naniwa, Obj *hand) {
    if (gasyaHandGetFrame(hand) >= 16.0) {
        *(u8 *) FX(naniwa) = 1;
        return 1;
    }
    return 0;
}

/* "gasya_hasi"'s work. */
typedef struct GasyaHasiW {
    /* 0x00 */ u8 unused00;
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
    /* 0x34 */ AnimeFrame *a;
    /* 0x38 */ AnimeFrame *b;
    /* 0x3C */ f32 speed;
} GasyaHasiW;

Gfx gasyaHasiDl[] = {
    gsSPDisplayList(gDrawSetupLitOpaDl),
    gsDPSetRenderMode(G_RM_RA_ZB_OPA_SURF, G_RM_RA_ZB_OPA_SURF2),
    gsSPEndDisplayList(),
    gsDPNoOp(),
}; /* the bridge's display list */
extern Gfx gasyaHasiDlB[];
extern Gfx gasyaHasiDlA[];
extern u8 gasyaHasiAnim0[];           /* its two animations */
extern u8 gasyaHasiAnim1[];

/* "gasya_hasi", the gasya's bridge at (585, 149.5, 731): two models of 0.252 sharing a display
 * list. While a player is within 400 they animate, at a speed easing down to 0.1 (0.02 a frame);
 * otherwise they stand. Message 10 with 0xCF264 answers with the task.
 * FAKE: the display list has a second name at each use (gasyaHasiDlB, gasyaHasiDlA in undefined_syms.txt) so the
 * address is formed again, not spilled (IDO_NOTES 298). */
s32 gasyaHasiProc(Obj *obj, s16 msg, void *arg) {
    GasyaHasiW *w = (GasyaHasiW *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "gasya_hasi";
            eneSetup(obj, arg);
            w->x = 585.0f;
            w->y = 149.5f;
            w->z = 731.0f;
            w->zone = 0;
            w->b = eneNewAnime2(obj, (u32) gasyaHasiAnim1, (Gfx *) fadeSegToRam(gasyaHasiDlB, obj->file));
            w->b->scale[2] = 0.252f;
            w->b->scale[0] = w->b->scale[1] = w->b->scale[2];
            eneSetModel(obj, &w->b->f);
            w->a = eneNewAnime2(obj, (u32) gasyaHasiAnim0, (Gfx *) fadeSegToRam(gasyaHasiDlA, obj->file));
            w->a->scale[2] = 0.252f;
            w->a->scale[0] = w->a->scale[1] = w->a->scale[2];
            eneSetModel(obj, &w->a->f);
            w->speed = 0.1f;
            break;
        case 10:
            if ((s32) arg == 0xCF264) {
                return (s32) obj;
            }
            break;
        case 1:
            if (w->a != NULL) {
                frmDestroy(&w->a->f);
            }
            if (w->b != NULL) {
                frmDestroy(&w->b->f);
            }
            break;
        case 0:
            w->a->anim = 0;
            w->b->anim = 0;
            if (eneNearestPlayerDist(obj) < 400.0) {
                w->a->anim = gasyaHasiAnim0;
                w->b->anim = gasyaHasiAnim1;
            }
            if (w->speed > 0.1) {
                w->speed -= 0.02;
            } else {
                w->speed = 0.1f;
            }
            if (w->a->anim != 0) {
                eneAnimeLoop(obj, w->a, w->speed);
                eneAnimeLoop(obj, w->b, w->speed);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Sets the bridge's animation speed (GasyaHasiW's speed, at 0x3C; the bridge answers message 10
 * with GASYA_ENE_ID), if it is there: it swings faster, then eases back to 0.1. */
void gasyaHasiSetSpeed(f32 v) {
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) GASYA_ENE_ID);

    if (o != NULL) {
        ((GasyaEneW *) (o + 1))->speed = v;
    }
}
