#include "tlb.h"

/* File 37's tasks (strings, IDA files/0037_08000000.bin.i64): "ihead", "ihand", "ihandpoint",
 * "fire", "T_fire", "bomb", "dai", "sippo", "nobuko", "atoya". The works below hold only the
 * fields the small helpers read; each helper follows its task's proc in the file. */

typedef struct IheadW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model; /* eneNewPol */
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ LightFrame *light;
    /* 0x38 */ AmbFrame *amb;
    /* 0x3C */ f32 phase1; /* the two fade-line waves */
    /* 0x40 */ f32 phase2;
    /* 0x44 */ u8 pad44[0x8];
    /* 0x4C */ f32 eyeY; /* the fight camera's eye height and distance */
    /* 0x50 */ f32 eyeZ;
    /* 0x54 */ f32 wobble; /* the eye's shake (step 9), the quake's sound timer (step 11) */
    /* 0x58 */ s32 flags;
    /* 0x5C */ s32 timer;
} IheadW;

typedef struct IhandW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ AnimeFrame *anim; /* eneNewAnime */
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 velX;
    /* 0x2C */ f32 velY;
    /* 0x30 */ f32 velZ;
    /* 0x34 */ ACollFrame *atk; /* aniAtkCreate's */
    /* 0x38 */ s32 timer;
} IhandW;

typedef struct DaiW {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *pol; /* eneNewPol */
} DaiW;

typedef struct SippoW {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model; /* its tip */
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ f32 active;
    /* 0x38 */ f32 heading;
    /* 0x3C */ f32 spin;
    /* 0x40 */ f32 pos[3]; /* x and z from a course point */
    /* 0x4C */ f32 length; /* its length */
    /* 0x50 */ f32 phase1; /* its sway's two angles */
    /* 0x54 */ f32 phase2;
    /* 0x58 */ f32 sway1;
    /* 0x5C */ f32 sway2;
    /* 0x60 */ f32 baseY;
    /* 0x64 */ MPolFrame *shadow;
    /* 0x68 */ PolFrame *base;
    /* 0x6C */ PolFrame *hole;
    /* 0x70 */ ACollFrame *atk;
    /* 0x74 */ s32 count; /* segments */
    /* 0x78 */ PolFrame *segs[20];
    /* 0xC8 */ ACollFrame *segAtk[20];
} SippoW;

typedef struct NobukoW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ AnimeFrame *anim; /* eneNewAnime */
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3]; /* rot[1]: her heading, round the arena */
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ s32 ihead; /* the ihead task (kind 4) */
    /* 0x38 */ Obj *dai; /* kind 2, her stand */
    /* 0x3C */ Obj *sippo; /* kind 3, her tail */
    /* 0x40 */ MPolFrame *shadow;
    /* 0x44 */ ACollFrame *dmg; /* 100 by 420, kind 0x78 */
    /* 0x48 */ Obj *gauge; /* her life gauge (pipGauge*) */
    /* 0x4C */ Obj *helper; /* kind 10, which makes the fires (kinds 5 and 9) */
    /* 0x50 */ s32 wait;
    /* 0x54 */ s32 count;
    /* 0x58 */ u8 pad58[0x2];
    /* 0x5A */ u16 step; /* where she is in her attack script */
    /* 0x5C */ s32 timer; /* frames until the next step */
    /* 0x60 */ s32 life; /* 8 hits */
    /* 0x64 */ f32 daiPos[3];
    /* 0x70 */ f32 tailLen; /* the tail's (sippoSetLenSway) */
    /* 0x74 */ f32 tailSway1;
    /* 0x78 */ f32 tailSway2;
    /* 0x7C */ f32 spin; /* her turn a frame */
    /* 0x80 */ f32 spinAcc;
    /* 0x84 */ f32 goal; /* the heading she turns to (the camera's, iheadCamHeading) */
    /* 0x88 */ Obj *target; /* the player she goes for */
    /* 0x8C */ s32 player; /* the player she aims at (an index) */
    /* 0x90 */ u8 passed; /* the spin's sound */
    /* 0x91 */ u8 fire; /* which fire sound is next */
    /* 0x92 */ u8 shot; /* the big fire is made */
} NobukoW;

#define DPRINTF

#define IHEAD(o) ((IheadW *) FX(o))
#define IHAND(o) ((IhandW *) FX(o))
#define DAI(o) ((DaiW *) FX(o))
#define SIPPO(o) ((SippoW *) FX(o))
#define NOBUKO(o) ((NobukoW *) FX(o))

/* The course place 96 out at angle: the course (2 from pi on, else 1), the place along it and
 * the side. */
void nobukoRingPlace(f32 angle, u8 *course, f32 *place, f32 *side) {
    s32 pad;
    f32 z;

    if (angle >= 3.1415927f) {
        *course = 2;
    } else {
        *course = 1;
    }
    z = mathCos(angle) * 96.0;
    *place = vramNearestPlaceXZ(*course, mathSin(angle) * 96.0, z, side);
    DPRINTF("????? kaku = %f\n", angle);
}

extern Gfx iheadHeadDl[]; /* the head's model */
void nobukoShowGauge(Obj *o);

/* The ring of fires, 120 out at angle a, height y. */
#define IHEAD_FIRE(a, y) ptclSpawnFire(mathCos(a) * 120.0f, y, mathSin(a) * 120.0f, 0xFF)

/* "ihead": the boss's head, and the fight's director.
 * - It lights the arena (a red light, a dim ambient), circles it with 16 fires and holds the
 *   camera.
 * - The intro (steps 1-6, skipped by Start, or at once on a second visit):
 *   - the letterbox comes in with the players frozen at height 200;
 *   - sounds 0x257 and 0xB5;
 *   - the camera moves in two shots, then scene run 7.
 * - Step 9 is the fight: the camera's eye follows the players' centre from eyeZ out, shaking
 *   while flags is set.
 * - Steps 10-12 are its death (from iheadDie): a quake, then an explosion (0x3F) and a
 *   white fade, then demo 3.
 * - Every frame the fade lines wave. */
s32 iheadProc(Obj *o, s16 msg, void *arg) {
    s32 pad0[2];
    Camera *cam = camAiGetCam();
    s32 pad[2];
    Vec3f p;
    s32 pad2[3];
    s32 zone;
    IheadW *w = IHEAD(o);
    Player *pl;
    s32 i;
    s32 n;

    switch (msg) {
        case 2:
            o->name = "ihead";
            eneSetup(o, arg);
            w->model = eneNewPol(o, iheadHeadDl);
            w->light = (LightFrame *) frmCreate(gFrmLayers[0], gFrmKindLight);
            w->light->color[0] = 0xFF;
            w->light->color[1] = 0x80;
            w->light->color[2] = 0x80;
            w->light->dir[0] = 100;
            w->light->dir[1] = 100;
            w->light->dir[2] = 100;
            w->amb = (AmbFrame *) frmCreate(gFrmLayers[0], gFrmKindAmbi);
            w->amb->color[0] = 30;
            w->amb->color[1] = 30;
            w->amb->color[2] = 30;
            w->phase1 = 0.0f;
            w->phase2 = 0.0f;
            gGameState.fade = 1;
            gGameState.fadeStart = 0x30;
            gGameState.fadeEnd = 0x80;
            camAiSetHold(1);
            camSetAt(cam, 9.0f, 162.0f, 73.0f);
            camSetEye(cam, 30.0f, 157.0f, 27.0f);
            w->eyeY = 175.0f;
            w->eyeZ = 220.0f;
            for (i = 0; i < 4; i++) {
                if (plIsInPlay(i)) {
                    plSetAction(gGameWork.players[i], 0xD7);
                    pl = PL(gGameWork.players[i]);
                    pl->tilt = 1.5707964f;
                }
            }
            IHEAD_FIRE(0.0f, 180.0f);
            IHEAD_FIRE(0.3926991f, 190.0f);
            IHEAD_FIRE(0.7853982f, 180.0f);
            IHEAD_FIRE(1.1780972f, 190.0f);
            IHEAD_FIRE(1.5707964f, 180.0f);
            IHEAD_FIRE(1.9634955f, 190.0f);
            IHEAD_FIRE(2.3561945f, 180.0f);
            IHEAD_FIRE(2.7488937f, 190.0f);
            IHEAD_FIRE(3.1415927f, 180.0f);
            IHEAD_FIRE(3.5342917f, 190.0f);
            IHEAD_FIRE(3.926991f, 180.0f);
            IHEAD_FIRE(4.31969f, 190.0f);
            IHEAD_FIRE(4.712389f, 180.0f);
            IHEAD_FIRE(5.105088f, 190.0f);
            IHEAD_FIRE(5.4977875f, 180.0f);
            IHEAD_FIRE(5.8904862f, 190.0f);
            w->timer = 60;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->state = 0;
            gGameWork.scenarioRunning = 2;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->light != NULL) {
                frmDestroy(&w->light->f);
            }
            if (w->amb != NULL) {
                frmDestroy(&w->amb->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            if (w->state > 0 && w->state < 7 && fadeStartPressed()) {
                sndPlay(7);
                w->state = 7;
            }
            switch (w->state) {
                case 0:
                    if (eneCounterGet(0)) {
                        w->state = 7;
                        eneCounterIncrement(0);
                    } else {
                        eneCounterIncrement(0);
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (fadeMoveLetterbox(45, 185, 1)) {
                        if (w->timer == 0x3B) {
                            sndPlay(0x257);
                        }
                        if (w->timer == 0x36) {
                            for (i = 0, n = 0; i < 4; i++) {
                                if (plIsInPlay(i)) {
                                    n++;
                                }
                            }
                            DPRINTF("!!!!! ppp = %d", n);
                            DPRINTF("@@@@@@@@@\n");
                            if (n >= 2) {
                                sndPlay(0x257);
                            }
                        }
                        if (w->timer == 0x32) {
                            sndPlay(0xB5);
                        }
                        if (w->timer-- <= 0) {
                            w->timer = 100;
                            camSetSpeed(cam, 100.0f);
                            camMoveFov(cam, 0.87266463f);
                            camMoveAt(cam, 9.0f, 170.0f, 73.0f);
                            camMoveEye(cam, 27.0f, 151.0f, 145.0f);
                            w->state = 2;
                        }
                    } else {
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                plAskPos(gGameWork.players[i], &p.x, &p.y, &p.z, &zone);
                                p.y = 200.0f;
                                plSetPos(gGameWork.players[i], zone, &p);
                            }
                        }
                    }
                    break;
                case 2:
                    if (w->timer-- <= 0) {
                        w->timer = 40;
                        w->state = 3;
                    }
                    break;
                case 3:
                    if (w->timer-- <= 0) {
                        w->timer = 60;
                        camSetSpeed(cam, 60.0f);
                        camMoveFov(cam, 0.69813174f);
                        camMoveAt(cam, 0.0f, 210.0f, 0.0f);
                        camMoveEye(cam, 47.0f, 200.0f, 65.0f);
                        w->state = 4;
                    }
                    break;
                case 4:
                    if (w->timer-- <= 0) {
                        scRunStart(7, 0, 0);
                        w->timer = 120;
                        w->state = 5;
                    }
                    break;
                case 5:
                    if (w->timer-- <= 0) {
                        camSetSpeed(cam, 45.0f);
                        camMoveFov(cam, 0.69813174f);
                        camMoveAt(cam, 0.0f, 170.0f, 0.0f);
                        camMoveEye(cam, 0.0f, w->eyeY, w->eyeZ);
                        w->state = 6;
                    }
                    gGameWork.scenarioRunning = 2;
                    break;
                case 6:
                    gGameWork.scenarioRunning = 2;
                    if (fadeMoveLetterbox(0, 240, 2)) {
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                plSetAction(gGameWork.players[i], 0);
                                pl = PL(gGameWork.players[i]);
                                pl->tilt = 0.0f;
                            }
                        }
                        gGameWork.scenarioRunning = 1;
                        w->state = 9;
                        sndPlay(0x98);
                        (*nobukoShowGauge)(o->parent);
                    }
                    break;
                case 7:
                    camSetFov(cam, 0.69813174f);
                    camSetAt(cam, 0.0f, 170.0f, 0.0f);
                    camSetEye(cam, 0.0f, w->eyeY, w->eyeZ);
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            plSetAction(gGameWork.players[i], 0);
                            pl = PL(gGameWork.players[i]);
                            pl->tilt = 0.0f;
                        }
                    }
                    w->state = 8;
                    /* fallthrough */
                case 8:
                    fadeSetLetterbox(320, 0, 0, 240);
                    gGameWork.scenarioRunning = 1;
                    w->state = 9;
                    scRunEnd();
                    sndPlay(0x98);
                    (*nobukoShowGauge)(o->parent);
                    eneRumbleStopAll();
                    break;
                case 9: {
                    Vec3f d;
                    f32 x;
                    f32 y;
                    f32 z;
                    f32 out;
                    f32 shake;

                    n = 0;
                    d.x = d.y = d.z = 0.0f;
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            plGetModelPos(gGameWork.players[i], &x, &y, &z);
                            n++;
                            d.x += x;
                            d.z += z;
                        }
                    }
                    out = sqrtf(d.x * d.x + d.z * d.z);
                    if (n == 0) {
                        n = 1;
                    }
                    out = 50.0f - out / (n * 2);
                    if (out >= 20.0) {
                        out = 20.0f;
                    }
                    d.x /= n;
                    d.z /= n;
                    (*guNormalize)(&d.x, &d.y, &d.z);
                    w->light->dir[0] = d.x * 100.0f;
                    w->light->dir[1] = 100;
                    w->light->dir[2] = d.z * 100.0f;
                    d.x *= w->eyeZ + out;
                    d.z *= w->eyeZ + out;
                    camSetSpeed(cam, 4.0f);
                    if (w->flags) {
                        w->wobble += 60.0f;
                        if (w->wobble >= 1080.0f) {
                            w->flags ^= 1;
                        }
                        shake = (*sinf)(w->wobble / 180.0 * 3.1415927f) * 4.0f;
                    } else {
                        shake = 0.0f;
                        w->wobble = 0.0f;
                    }
                    camMoveEye(cam, d.x, w->eyeY + out / 2 + shake, d.z);
                    break;
                }
                case 10:
                    gGameWork.scenarioRunning = 2;
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            plSetAction(gGameWork.players[i], 0xD7);
                        }
                    }
                    camSetAt(cam, 0.0f, 200.0f, 0.0f);
                    camSetEye(cam, -40.0f, 170.0f, 80.0f);
                    camSetFov(cam, 1.0471976f);
                    camSetSpeed(cam, 200.0f);
                    camMoveAt(cam, 0.0f, 210.0f, 0.0f);
                    camMoveEye(cam, 20.0f, 222.0f, 65.0f);
                    camMoveFov(cam, 0.95993114f);
                    w->light->dir[0] = 100;
                    w->light->dir[1] = 100;
                    w->light->dir[2] = 100;
                    sndPlay(9);
                    w->timer = 240;
                    eneRumbleAll(0xBBBBBBBB, 240);
                    w->state = 11;
                    w->wobble = 0.0f;
                    /* fallthrough */
                case 11:
                    fadeMoveLetterbox(45, 185, 1);
                    if (w->timer-- == 0) {
                        w->timer = 70;
                        eneRumbleAll(-1, 70);
                        sndPlay(0x11D);
                        eneMakeEx(o, ENE_BIG_BOMB, 0, 0.0f, 200.0f, 0.0f, 2, (u32 (*)(void *, void *, void *)) 1);
                        fadeSetAll(w->timer, 0.0f, 255.0f, 255, 255, 255);
                        w->state = 12;
                    }
                    if (w->wobble-- <= 0.0f) {
                        w->wobble = fadeRand(2) + 5;
                        sndPlay(0x513);
                    }
                    break;
                case 12:
                    if (w->timer-- == -10) {
                        w->state = 13;
                        eneRumbleStopAll();
                        eneCounterSetFlag(0);
                        dmenuQueueDemo(3);
                    }
                    break;
                case 14:
                    w->timer = 80;
                    w->state = 15;
                    /* fallthrough */
                case 15:
                    if (w->timer-- == 0) {
                        w->timer = 60;
                        w->state = 16;
                    }
                    break;
                case 16:
                    if (w->timer-- == 0) {
                        sndPlay(7);
                    }
                    if (fadeMoveLetterbox(120, 120, 3)) {
                        w->state = 17;
                    }
                    break;
                case 17:
                    break;
            }
            eneSetModel(o, w->model);
            w->phase1 += 0.06283186f;
            if (w->phase1 >= 6.2831855f) {
                w->phase1 -= 6.2831855f;
            }
            w->phase2 += 0.0418879f;
            if (w->phase2 >= 6.2831855f) {
                w->phase2 -= 6.2831855f;
            }
            gGameState.fadeStart = mathSin(w->phase1) * 16.0f + 48.0f;
            gGameState.fadeEnd = mathSin(w->phase2) * 16.0f + 128.0f;
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void iheadDie(Obj *ihead) {
    IHEAD(ihead)->state = 10;
}

void iheadCloseLetterbox(Obj *ihead) {
    IHEAD(ihead)->state = 14;
}

void iheadSetShake(Obj *ihead, s32 flags) {
    IHEAD(ihead)->flags |= flags;
}

/* The camera's heading: the angle of its eye in x and z (tlb_0039's twin's C: the camera in a
 * local puts x, y, z where the game has them). */
f32 iheadCamHeading(void) {
    Camera *cam;
    f32 z;
    f32 y;
    f32 x;

    cam = camAiGetCam();
    camGetEye(cam, &x, &y, &z);
    return mathAtan2(x, z);
}

void ihandpointGetPos(Obj *o, f32 *x, f32 *y, f32 *z);
extern u8 ihandHandAnim[]; /* the hand's animation */
extern u8 ihandAttackAtkChain[]; /* its attack animation */

/* "ihand", the boss's iron hand: an animation at scale 2 with an animated attack box. It rises
 * from -320 to 170 at its point (260 over from it), arming its box (kind 0x78) at the top; then
 * (state 1) it slams down to frame 9 with sound 0x109 and a rumble, waits 40 frames and rises
 * back (state 2), or sinks away by 10 a frame (state 3 to 4). Whoever its box hits gets
 * message 7. */
s32 ihandProc(Obj *o, s16 msg, void *arg) {
    IhandW *w = IHAND(o);
    s32 hitter;
    f32 z;
    f32 y;
    f32 x;

    switch (msg) {
        case 2:
            o->name = "ihand";
            eneSetup(o, arg);
            if ((w->anim = eneNewAnime(o, (u32) ihandHandAnim)) == NULL) {
                return -1;
            }
            if ((w->atk = aniAtkCreate(&ene_atk_root_frame->frame, &w->anim->f, (u32) ihandAttackAtkChain, eneGetFile(o, 0),
                                       0)) == NULL) {
                return -1;
            }
            w->anim->scale[2] = 2.0f;
            w->anim->scale[0] = w->anim->scale[1] = w->anim->scale[2];
            w->x = 260.0f;
            w->y = -320.0f;
            w->z = 0.0f;
            w->velY = 10.0f;
            break;
        case 1:
            if (w->anim != NULL) {
                frmDestroy(&w->anim->f);
            }
            if (w->atk != NULL) {
                aniAtkDelete(w->atk);
            }
            break;
        case 0:
            eneStepHookPre(o);
            if (w->atk != NULL) {
                ((AniAtkFrame *) w->atk->model)->time = w->anim->frame;
                if (acollAtkWasHit(w->atk) == 1 && (hitter = w->atk->hitPower) != 0) {
                    tskSendMsg((Obj *) hitter, 7, o);
                }
            }
            switch (w->state) {
                case 0:
                    (*ihandpointGetPos)(o->parent, &x, &y, &z);
                    w->x = x + 260.0f;
                    w->y += w->velY;
                    w->z = z;
                    if (w->y >= 170.0f) {
                        w->atk->kind = 0x78;
                        w->y = 170.0f;
                        w->velY = 0.0f;
                    }
                    break;
                case 1:
                    eneAnimPlayOnce(o, w->anim, 0.2f);
                    if (w->anim->frame >= 9.0) {
                        w->anim->frame = 9.0f;
                        sndPlay(0x109);
                        eneRumbleAll(-1, 30);
                        w->timer = 40;
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (--w->timer <= 0 && eneAnimPlayOnce(o, w->anim, -0.4f)) {
                        w->state = 0;
                    }
                    break;
                case 3:
                    w->y -= 10.0f;
                    if (w->y <= -320.0f) {
                        w->state = 4;
                    }
                    break;
                case 4:
                    break;
            }
            eneSetModel(o, &w->anim->f);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void ihandSlam(Obj *ihand) {
    IHAND(ihand)->state = 1;
}

s32 ihandIsDown(Obj *o) {
    if (ENE(o)->state == 2) {
        return 1;
    }
    return 0;
}

s32 ihandIsUp(Obj *ihand) {
    if (IHAND(ihand)->state == 0) {
        return 1;
    }
    return 0;
}

s32 ihandIsGone(Obj *ihand) {
    if (IHAND(ihand)->state == 4) {
        return 1;
    }
    return 0;
}

void ihandSink(Obj *ihand) {
    IHAND(ihand)->state = 3;
}

/* The hand's animation frame. */
f32 ihandGetFrame(Obj *ihand) {
    return IHAND(ihand)->anim->frame;
}

/* "ihandpoint"'s work: the iron hand's target marker. */
typedef struct IhandpointW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone; /* its course */
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *frame; /* the marker */
    /* 0x10 */ f32 place; /* along the course */
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 side;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *ring; /* its pulsing ring */
    /* 0x38 */ MPolFrame *shadow; /* the hand's shadow */
    /* 0x3C */ Obj *hand; /* the "ihand" */
    /* 0x40 */ s32 target; /* the player it follows, from 1 */
    /* 0x44 */ s32 timer;
    /* 0x48 */ s32 slams; /* left */
    /* 0x4C */ f32 angle; /* its bob's, in degrees */
    /* 0x50 */ f32 size; /* the shadow's */
} IhandpointW;

void ihandSlam(Obj *ihand);
s32 ihandIsDown(Obj *o);
s32 ihandIsUp(Obj *ihand);
s32 ihandIsGone(Obj *ihand);
void ihandSink(Obj *ihand);
f32 ihandGetFrame(Obj *ihand);
s32 nobukoGetIhead(Obj *nobuko);
extern Gfx ihandpointMarkerDl[]; /* the marker */
extern Gfx ihandpointRingDl[]; /* its ring */

/* Main's sinf, called through its address as TLB code calls main. */
#define sinf (*sinf)

/* "ihandpoint", the iron hand's target: it makes the hand ("ihand", kind 8) at (260, -320, 0)
 * and two pol models (0.2) at height 155, with the hand's shadow, and follows a random player
 * in play along the course (10 a frame, turning back at 100 and 450). After its timer (100),
 * within 10 of the player it calls the slam (sound 0x2A7). While the hand slams (state 1) it
 * bobs; when the hand is back up (2) it starts over at another player, or after its three
 * slams lets the hand sink and goes with it (3). The shadow grows with the hand's frame.
 * Matched 2026-10-11; it was parked for a spill-slot order: uopt gives the work pointer's
 * expression (o + 0x20) and eneNewPol's/eneSetModelOnCourse's/sinf's call addresses the SAME
 * spill slot only when w's expression comes after all three call addresses in uopt's bit
 * order, which the dead store marked FAKE below forces. */
s32 ihandpointProc(Obj *o, s16 msg, void *arg) {
    IhandpointW *w;
    f32 dx;
    f32 sx;
    f32 sz;
    f32 x;
    f32 y;
    f32 z;

    /* FAKE: a dead store (dx is set again before every read); it makes eneNewPol's address the
     * first saved expression (uopt bit order), so the call addresses' spill slot is above w's,
     * as in the game. dx, not a new local, so the homes stay the game's 44 bytes. */
    dx = (s32) eneNewPol;
    w = (IhandpointW *) FX(o);

    switch (msg) {
        case 2:
            o->name = "ihandpoint";
            eneSetup(o, arg);
            w->hand = eneMakeUnder(o, ENE_IHAND, w->zone, 260.0f, -320.0f, 0.0f);
            if ((w->frame = eneNewPol(o, ihandpointMarkerDl)) == NULL) {
                return -1;
            }
            if ((w->ring = eneNewPol(o, ihandpointRingDl)) == NULL) {
                return -1;
            }
            w->frame->flags |= 7;
            w->frame->scale[2] = 0.2f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->ring->flags |= 7;
            w->ring->scale[2] = 0.2f;
            w->ring->scale[0] = w->ring->scale[1] = w->ring->scale[2];
            w->timer = 100;
            w->shadow = shadowCreate();
            w->size = 1.0f;
            w->target = eneRandomPlayerIndex();
            w->slams = 3;
            sndPlay(0x2A6);
            break;
        case 1:
            if (w->frame != NULL) { frmDestroy(&w->frame->f); }
            if (w->ring != NULL) { frmDestroy(&w->ring->f); }
            if (w->shadow != NULL) { frmDestroy((Frame *) w->shadow); }
            break;
        case 0:
            eneStepHookPre(o);
            if (w->hand == NULL) {
                DPRINTF("NONNONONONONONONONONO!!\n");
                return -1;
            }
            switch (w->state) {
                case 0:
                    w->place += 10.0f;
                    w->y = 155.0f;
                    w->side = 0.0f;
                    if (w->place >= 450.0f || w->place <= 100.0f) {
                        if (w->zone == 1) {
                            w->zone = 2;
                        } else {
                            w->zone = 1;
                        }
                        w->place = vramNearestPlaceXZ(w->zone, w->frame->pos[0], w->frame->pos[2], &w->side);
                    }
                    eneSetModelOnCourse(o, &w->frame->f);
                    eneSetModelOnCourse(o, &w->ring->f);
                    w->ring->pos[1] += 15.0f;
                    if (!plIsInPlay(w->target)) {
                        if ((w->target = eneRandomPlayerIndex()) == 0) {
                            return -1;
                        }
                    }
                    plGetModelPos(gGameWork.players[w->target], &x, &y, &z);
                    if (--w->timer <= 0) {
                        f32 dy;
                        f32 dz;

                        dx = x - w->frame->pos[0];
                        dy = y - w->frame->pos[1];
                        dz = z - w->frame->pos[2];
                        if (sqrtf(dx * dx + dy * dy + dz * dz) <= 10.0) {
                            sndPlay(0x2A7);
                            w->timer = 40;
                            w->slams--;
                            (*ihandSlam)(w->hand);
                            w->state = 1;
                        }
                    }
                    break;
                case 1:
                    w->angle += 30.0;
                    if (w->angle >= 360.0) {
                        w->angle -= 360.0;
                    }
                    w->frame->pos[1] = sinf(w->angle / 180.0 * 3.1415927f) * 2.5 + w->y;
                    /* dx reused as the ring's size (the game keeps it in f2, dx's register) */
                    dx = sinf(w->angle / 180.0 * 3.1415927f) * 0.05 + 0.2;
                    w->ring->scale[2] = dx;
                    w->ring->scale[1] = dx;
                    w->ring->scale[0] = dx;
                    if (ihandIsDown(w->hand)) {
                        iheadSetShake((Obj *) nobukoGetIhead(o->parent->parent), 1);
                        w->state = 2;
                    }
                    break;
                case 2:
                    w->angle = 0.0f;
                    w->frame->pos[1] = w->y;
                    w->ring->scale[2] = 0.2f;
                    w->ring->scale[0] = w->ring->scale[1] = w->ring->scale[2];
                    w->timer = 100;
                    if (ihandIsUp(w->hand)) {
                        if (w->slams <= 0) {
                            if (w->frame != NULL) {
                                w->frame = (PolFrame *) frmDestroy(&w->frame->f);
                            }
                            if (w->ring != NULL) {
                                w->ring = (PolFrame *) frmDestroy(&w->ring->f);
                            }
                            if (w->shadow != NULL) {
                                w->shadow = (MPolFrame *) frmDestroy((Frame *) w->shadow);
                            }
                            ihandSink(w->hand);
                            w->state = 3;
                        } else {
                            w->target = eneRandomPlayerIndex();
                            sndPlay(0x2A6);
                            w->state = 0;
                        }
                    }
                    break;
                case 3:
                    if (ihandIsGone(w->hand)) {
                        return -1;
                    }
                    break;
            }
            if (w->hand != NULL) {
                w->size = ihandGetFrame(w->hand) * 1.5;
                if (w->size <= 1.0) {
                    w->size = 1.0f;
                }
                vramCoursePointXZ(w->zone, w->place, w->side, &sx, &sz);
                shadowPut(w->shadow, sx, w->y, sz, w->size, NULL);
            }
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Its position, or (260, -320, 0) without a frame. */
void ihandpointGetPos(Obj *o, f32 *x, f32 *y, f32 *z) {
    IhandpointW *w = (IhandpointW *) FX(o);

    if (w->frame != NULL) {
        *x = w->frame->pos[0];
        *y = w->frame->pos[1];
        *z = w->frame->pos[2];
    } else {
        *x = 260.0f;
        *y = -320.0f;
        *z = 0.0f;
    }
}

/* "fire"'s work: an enemy's, then its own counters. */
typedef struct FireW {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ Frame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 velX;
    /* 0x2C */ f32 velY;
    /* 0x30 */ f32 velZ;
    /* 0x34 */ s32 count; /* frames since its last T_fire */
    /* 0x38 */ s32 timer; /* frames left once it is out of the ring */
} FireW;

Obj *tFireCreate(Obj *obj, f32 x, f32 y, f32 z);

/* "fire": thrown outward at 3 a frame (toward twice its place), it falls (0.2) to a floor at
 * 160 and leaves a "T_fire" every third frame; once 120 from the middle it goes after 40
 * frames. Message 4 reads its position into arg. */
s32 fireProc(Obj *o, s16 msg, void *arg) {
    FireW *w = (FireW *) FX(o);

    switch (msg) {
        case 2:
            o->name = "fire";
            eneSetup(o, arg);
            eneSetVelTowardXZ(o, 3.0f, w->x * 2, w->z * 2);
            w->timer = 40;
            w->velY = 0.0f;
            break;
        case 4:
            ((f32 *) arg)[0] = w->x;
            ((f32 *) arg)[1] = w->y;
            ((f32 *) arg)[2] = w->z;
            break;
        case 1:
            if (((FireW *) FX(o))->model != NULL) {
                frmDestroy(((FireW *) FX(o))->model);
            }
            break;
        case 0:
            eneStepHookPre(o);
            if (sqrtf(w->x * w->x + w->z * w->z) > 120.0) {
                if (--w->timer <= 0) {
                    return -1;
                }
            } else {
                eneFall(o, 0.2f);
                eneMove((EneObj *) w);
                if (w->y <= 160.0) {
                    w->y = 160.0f;
                }
                if (w->count++ >= 2) {
                    PRINTF("!!!! x = %f, y = %f !!!\n", w->x, w->y);
                    tFireCreate(o, w->x, w->y, w->z);
                    w->count = 0;
                }
            }
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "T_fire"'s work. */
typedef struct TFireW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ ACollFrame *atk;
    /* 0x08 */ Anim anim; /* its flicker */
    /* 0x18 */ s32 alpha;
    /* 0x1C */ u8 pad1C[0x4];
    /* 0x20 */ u8 state;
} TFireW;

extern u8 tFireFlameDl[]; /* a flame */

/* acollAtkAddYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);

/* "T_fire", a flame left by "fire": a yellow flickering flame (file 0x125) with an attack box
 * (radius 20, kind 0x78). It grows by 0.05 a frame to 0.4 while fading in, then shrinks by 0.02
 * while fading out, losing its box under 0.2 and going under 0.1; it rises 0.1 a frame. Whoever
 * its box hits gets message 7.
 * The hitter is tested as a pointer, as kpaFireball does; as an int IDO loads it into v0. */
s32 tFireProc(Obj *obj, s16 msg, void *arg) {
    TFireW *w = (TFireW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "T_fire";
            if ((w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                return -1;
            }
            w->frame->dl2 = (Gfx *) tFireFlameDl;
            floadGetSetRam((void **) w->frame->segs, 0x125);
            fadeAnimStart(&w->anim, gItexTimedYellowAnim, 0, &w->frame->segs[3], floadGetSetFile(0x125, 0));
            w->frame->flags |= 7;
            w->frame->scale[2] = 0.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->alpha = 0xFF;
            w->state = 0;
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 20, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 0x78;
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            break;
        case 0:
            fadeAnimStep(&w->anim);
            switch (w->state) {
                case 0:
                    w->frame->scale[0] += 0.05f;
                    w->frame->scale[1] += 0.05f;
                    w->frame->scale[2] += 0.05f;
                    w->alpha += 31.875;
                    if (w->alpha >= 255.0) {
                        w->alpha = 0xFF;
                    }
                    if (w->frame->scale[0] >= 0.4) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    w->frame->scale[0] -= 0.02f;
                    w->frame->scale[1] -= 0.02f;
                    w->frame->scale[2] -= 0.02f;
                    w->alpha -= 6.375;
                    if (w->frame->scale[0] <= 0.2 && w->atk != NULL) {
                        w->atk = (ACollFrame *) frmDestroy(&w->atk->frame);
                    }
                    if (w->frame->scale[0] <= 0.1 || w->alpha <= 0) {
                        return -1;
                    }
                    break;
            }
            w->frame->pos[1] += 0.1f;
            if (w->atk != NULL && acollAtkWasHit(w->atk) == 1 && (Obj *) w->atk->hitPower != NULL) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, obj);
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombine(gGfxDlHead++, 0x30D261, 0x55A6FF7F);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 255, 0);
            gDPSetEnvColor(gGfxDlHead++, 255, 255, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 tFireProc(Obj *obj, s16 msg, void *arg);

/* A "T_fire" at x, y, z under obj. */
Obj *tFireCreate(Obj *obj, f32 x, f32 y, f32 z) {
    Obj *o = tskCreate(obj, tFireProc, obj->file, NULL);

    if (o != NULL) {
        ((TFireW *) FX(o))->frame->pos[0] = x;
        ((TFireW *) FX(o))->frame->pos[1] = y;
        ((TFireW *) FX(o))->frame->pos[2] = z;
    }
    return o;
}

/* "bomb"'s work (an EneObj's head). */
typedef struct BombW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x; /* along the course */
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vx;
    /* 0x2C */ f32 vy;
    /* 0x30 */ f32 vz;
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ u8 pad3C[0x4];
    /* 0x40 */ s32 timer;
    /* 0x44 */ f32 phase; /* its wobble's */
    /* 0x48 */ f32 gravity; /* while knocked away */
} BombW;

typedef ACollFrame *(*BombDmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                        s32 a);

extern Gfx bombDl[]; /* the bomb */
u8 atoyaToggle(Obj *o);

/* "bomb", dropped by the boss's hand: a pol model (0.2) with an attack box (20 by 20, kind 0x78,
 * off at first) and a damage box (25 by 50, kind 0x79), made with sound 0x231. It falls at 4 to
 * height 280, takes a random place 96 out on the course, waits 40 frames, turns its box on and
 * wobbles down 1 a frame. At 149 it explodes (state 4): yellow sparks and sound 0x229 if the hand
 * (atoyaToggle) says so, else 0x22A. Its own kind on its box makes it vanish in blue sparks
 * (sound 0x11B); hitting a player, it sends message 7 and explodes. A player's hit (not kinds 13
 * and 0x32) knocks it toward the centre in an arc for 60 frames as a player's attack (kind 0x32);
 * kind 13 explodes it. */
s32 bombProc(Obj *obj, s16 msg, void *arg) {
    BombW *w = (BombW *) FX(obj);
    s32 pad;
    CamView *view;

    switch (msg) {
        case 2:
            obj->name = "bomb";
            eneSetup(obj, arg);
            if ((w->model = eneNewPol(obj, bombDl)) == NULL) {
                return -1;
            }
            w->vy = 4.0f;
            w->model->scale[2] = 0.2f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->flags |= 7;
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 20, 20, 0, 0)) == NULL) {
                return -1;
            }
            if (w->atk != NULL) {
                w->atk->flags &= ~2;
                w->atk->kind = 0x78;
            }
            if ((w->dmg = ((BombDmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->model->f, 25, 50, 0, 0, 0)) ==
                NULL) {
                return -1;
            }
            if (w->dmg != NULL) {
                w->dmg->kind = 0x79;
            }
            eneSetModel(obj, &w->model->f);
            SoundLocation2(0x231, gGameWork.view, w->model->pos[0], w->model->pos[1] + 5.0f, w->model->pos[2], 0.0f,
                           1000.0f, gGameWork.view->fovy);
            break;
        case 1:
            if (w->model != NULL) { frmDestroy(&w->model->f); }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            break;
        case 0:
            eneStepHookPre(obj);
            if (w->atk != NULL && acollAtkWasHit(w->atk) == 1) {
                if (w->atk->hitKind != 0x79) {
                    if (w->atk->hitKind == 0x78) {
                        w->state = 3;
                    } else {
                        tskSendMsg((Obj *) w->atk->hitPower, 7, obj);
                        w->state = 4;
                    }
                }
            }
            if (w->dmg != NULL && acollDmgWasHit(w->dmg) == 1) {
                if (w->dmg->hitKind == 13) {
                    w->state = 4;
                } else if (w->dmg->hitKind != 0x32) {
                    eneHitSound(w->dmg);
                    w->state = 5;
                }
            }
            switch (w->state) {
                case 0:
                    eneMove((EneObj *) w);
                    if (w->y >= 280.0f) {
                        nobukoRingPlace(2.0 * (fadeRand(360) / 360.0) * 3.14159274101257324, &w->zone, &w->x,
                                               &w->z);
                        w->state = 1;
                        w->timer = 40;
                        w->z = 0.0f;
                    } else {
                        eneSetModel(obj, &w->model->f);
                    }
                    break;
                case 1:
                    if (w->timer-- == 0) {
                        w->state = 2;
                        if (w->atk != NULL) {
                            w->atk->flags |= 2;
                        }
                    }
                    eneSetModelOnCourse(obj, &w->model->f);
                    break;
                case 2:
                    w->phase += 0.62831855f;
                    if (w->phase >= 6.2831855f) {
                        w->phase -= 6.2831855f;
                    }
                    w->x += (f64) mathSin(w->phase);
                    w->z = 0.0f;
                    w->y -= 1.0;
                    if (w->y <= 149.0) {
                        w->y = 155.0f;
                        w->model->dl2 = NULL;
                        w->state = 4;
                    }
                    eneSetModelOnCourse(obj, &w->model->f);
                    break;
                case 3:
                    sndPlay(0x11B);
                    itexSparkleBlue(w->model->pos[0], w->model->pos[1] + 5.0f, w->model->pos[2], 0.2f);
                    ptclSpawnSparksFall(w->model->pos[0], w->model->pos[1] + 5.0f, w->model->pos[2]);
                    return -1;
                case 4:
                    view = gGameWork.view;
                    itexSparkleYellow(w->model->pos[0], w->model->pos[1] + 5.0f, w->model->pos[2], 0.1f);
                    ptclSpawnSparksFall(w->model->pos[0], w->model->pos[1] + 5.0f, w->model->pos[2]);
                    if (atoyaToggle(obj->parent)) {
                        SoundLocation2(0x229, view, w->model->pos[0], w->model->pos[1] + 5.0f, w->model->pos[2], 150.0f,
                                       800.0f, view->fovy);
                        return -1;
                    }
                    SoundLocation2(0x22A, view, w->model->pos[0], w->model->pos[1] + 5.0f, w->model->pos[2], 150.0f,
                                   800.0f, view->fovy);
                    return -1;
                case 5:
                    w->x = w->model->pos[0];
                    w->y = w->model->pos[1];
                    w->z = w->model->pos[2];
                    w->vy = (220.0 - w->model->pos[1]) / 10.0;
                    if (w->vy >= 0.0f) {
                        w->gravity = -w->vy / 20.0;
                    } else {
                        w->gravity = w->vy / 40.0;
                    }
                    w->vx = (0.0f - w->model->pos[0]) / 40.0;
                    w->vz = (0.0f - w->model->pos[2]) / 40.0;
                    if (w->atk != NULL) {
                        frmMove(&w->atk->frame, (Frame *) pl_atk_root_frame);
                    }
                    if (w->atk != NULL) {
                        w->atk->kind = 0x32;
                    }
                    w->timer = 60;
                    w->state = 6;
                    eneSetModel(obj, &w->model->f);
                case 6:
                    if (w->timer-- <= 0) {
                        return -1;
                    }
                    w->vy += w->gravity;
                    eneMove((EneObj *) w);
                    if (w->y <= 149.0) {
                        w->state = 4;
                        w->y = 155.0f;
                    }
                    eneSetModel(obj, &w->model->f);
                    break;
            }
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

extern Gfx daiDl[]; /* the dai's model */

/* "dai": a stand, an enemy model at 0.15 scale turned 270 degrees, set on the ground. The
 * twin of tlb_0131's hanabiHiroba_main. */
s32 daiProc(Obj *o, s16 msg, void *arg) {
    s32 pad;
    EneObj *w = ENE(o);

    switch (msg) {
        case 2:
            o->name = "dai";
            eneSetup(o, arg);
            w->model = (Frame *) eneNewPol(o, daiDl);
            if (w->model == NULL) {
                return -1;
            }
            w->rot[1] = 4.712389f;
            ((PolFrame *) w->model)->scale[2] = 0.15f;
            ((PolFrame *) w->model)->scale[0] = ((PolFrame *) w->model)->scale[1] =
                ((PolFrame *) w->model)->scale[2];
            eneSetOnEarth(o);
            eneSetModelOnCourse(o, w->model);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(w->model);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The dai's position. */
void daiGetPos(Obj *dai, f32 *x, f32 *y, f32 *z) {
    *x = DAI(dai)->pol->pos[0];
    *y = DAI(dai)->pol->pos[1];
    *z = DAI(dai)->pol->pos[2];
}

/* A point off the sippo (its proc sippoProc's helper): `up` and `out` are angles, the
 * point is `len * cos(up)` above its height less 4, and `len2 * sin(out) + off` behind it
 * (its heading turned by pi). */
void sippoCurvePoint(Obj *o, f32 up, f32 out, f32 len, f32 len2, f32 off, f32 *x, f32 *y,
                            f32 *z) {
    f32 r;
    SippoW *w = SIPPO(o);

    *y = (w->pos[1] - 4.0) + mathCos(up) * len;
    r =mathSin(out) * len2 + off;
    *x = w->pos[0] + mathSin(w->heading + 3.1415927f) * r;
    *z = w->pos[2] + mathCos(w->heading + 3.1415927f) * r;
}

extern Gfx sippoTailTipDl[]; /* the tail's tip */
extern Gfx sippoBaseDl[]; /* its base */
extern Gfx sippoHoleDl[]; /* the hole */
extern Gfx sippoDl[]; /* a segment */

/* "sippo", the boss's tail: a tip (0.2) with a shadow and an attack box (15 by 15, kind 0x78),
 * standing on a base (0.8, layer 4) over a hole (layer 8). The tip sways on two angles, 6 and 9
 * degrees a frame (sippoCurvePoint). Each frame it rebuilds its body: a segment (0.2, its own
 * attack box) for each 8 of its length, up to 19, placed down the curve 36 and 48 degrees behind
 * per segment, their sway shrinking toward the base. Whatever a box hits gets message 7. The
 * hole shows in green, its alpha turning with the tail once it is over 25 long. */
s32 sippoProc(Obj *obj, s16 msg, void *arg) {
    SippoW *w = SIPPO(obj);
    s32 i;
    s32 j;
    f32 r;
    f32 step;
    f32 a;

    switch (msg) {
        case 2:
            obj->name = "sippo";
            eneSetup(obj, arg);
            if ((w->model = eneNewPol(obj, sippoTailTipDl)) == NULL) {
                return -1;
            }
            w->model->flags |= 7;
            w->model->scale[2] = 0.2f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->shadow = shadowCreate();
            if ((w->base = eneNewPol(obj, sippoBaseDl)) == NULL) {
                return -1;
            }
            frmMove(&w->base->f, gFrmLayers[4]);
            w->base->scale[2] = 0.8f;
            w->base->scale[0] = w->base->scale[1] = w->base->scale[2];
            w->base->pos[0] = w->pos[0];
            w->base->pos[1] = w->baseY;
            w->base->pos[2] = w->pos[2];
            if ((w->hole = eneNewPol(obj, sippoHoleDl)) == NULL) {
                return -1;
            }
            frmMove(&w->hole->f, gFrmLayers[8]);
            w->base->scale[2] = 0.1f;
            w->hole->scale[0] = w->base->scale[1] = w->base->scale[2];
            w->hole->pos[0] = w->pos[0];
            w->hole->pos[1] = w->pos[1] - 4.0;
            w->hole->pos[2] = w->pos[2];
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 15, 15, 0, 0)) == NULL) {
                return -1;
            }
            if (w->atk != NULL) {
                w->atk->kind = 0x78;
            }
            w->count = 3;
            break;
        case 3:
            w->base->dl = gGfxDlHead;
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 0, 0, 0, 0x80);
            gSPEndDisplayList(gGfxDlHead++);
            if (w->length > 25.0f) {
                a = w->spin * 128.0 * 40.0 / 6.28318548202514648;
            } else {
                a = 0.0f;
            }
            w->hole->dl = gGfxDlHead;
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 0x80, 0xFF, 0x80, (u8) a);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            if (w->base != NULL) {
                frmDestroy(&w->base->f);
            }
            if (w->hole != NULL) {
                frmDestroy(&w->hole->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            for (i = 0; i < 20; i++) {
                if (w->segs[i] != NULL) {
                    frmDestroy(&w->segs[i]->f);
                }
                if (w->segAtk[i] != NULL) {
                    frmDestroy(&w->segAtk[i]->frame);
                }
            }
            break;
        case 0:
            eneStepHookPre(obj);
            if (w->atk != NULL && acollAtkWasHit(w->atk) == 1) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, obj);
            }
            w->phase1 += 0.104719758f;
            if (w->phase1 >= 6.2831855f) {
                w->phase1 -= 6.2831855f;
            }
            w->phase2 += 0.1570796371f;
            if (w->phase2 >= 6.2831855f) {
                w->phase2 -= 6.2831855f;
            }
            sippoCurvePoint(obj, w->phase1, w->phase2, w->sway1, w->sway2, w->length, &w->x, &w->y, &w->z);
            eneSetModel(obj, &w->model->f);
            shadowPut(w->shadow, w->x, w->y, w->z, 2.0f, NULL);
            w->count = w->length / 8;
            if (w->count >= 21) {
                w->count = 19;
            }
            step = w->length / (w->count + 1);
            for (i = 0; i < 20; i++) {
                if (w->segs[i] != NULL) {
                    frmDestroy(&w->segs[i]->f);
                    w->segs[i] = NULL;
                }
                if (w->segAtk[i] != NULL) {
                    if (acollAtkWasHit(w->segAtk[i]) == 1) {
                        tskSendMsg((Obj *) w->segAtk[i]->hitPower, 7, obj);
                    }
                    frmDestroy(&w->segAtk[i]->frame);
                    w->segAtk[i] = NULL;
                }
            }
            for (i = 0, j = 1; i < w->count; i++, j++) {
                if ((w->segs[i] = eneNewPol(obj, sippoDl)) != NULL) {
                    if ((w->segAtk[i] = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->segs[i]->f, 10, 10, 0,
                                                                       0)) != NULL) {
                        w->segAtk[i]->kind = 0x78;
                    }
                    w->segs[i]->flags |= 7;
                    w->segs[i]->scale[2] = 0.2f;
                    w->segs[i]->scale[0] = w->segs[i]->scale[1] = w->segs[i]->scale[2];
                    r = (w->length - step * j) / w->length;
                    sippoCurvePoint(obj, w->phase1 - 0.628318548202514648 * j,
                                           w->phase2 - 0.837758064270019531 * j, w->sway1 * r, w->sway2 * r,
                                           w->length - step * j, &w->segs[i]->pos[0], &w->segs[i]->pos[1],
                                           &w->segs[i]->pos[2]);
                }
            }
            w->base->scale[2] = w->length / 10.0;
            w->base->rot[1] = w->heading + 3.1415927f;
            w->base->pos[0] = w->pos[0];
            w->base->pos[1] = w->baseY + 0.1;
            w->base->pos[2] = w->pos[2];
            w->hole->scale[0] = w->hole->scale[2] = w->length / 10.0;
            w->hole->rot[1] = w->heading;
            w->hole->pos[0] = w->pos[0];
            w->hole->pos[1] = w->pos[1] - 4.0;
            w->hole->pos[2] = w->pos[2];
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

void sippoSetActive(Obj *sippo, s32 n) {
    SIPPO(sippo)->active = n;
}

void sippoSetLenSway(Obj *sippo, f32 a, f32 b, f32 c) {
    SIPPO(sippo)->length = a;
    SIPPO(sippo)->sway1 = b;
    SIPPO(sippo)->sway2 = c;
}

/* Places sippo at course point dist (offset off) of course n, at height y. */
void sippoPlace(Obj *sippo, f32 a, f32 b, u8 n, f32 dist, f32 y, f32 off, f32 d) {
    SippoW *w = SIPPO(sippo);

    vramCoursePointXZ(n, dist, off, &SIPPO(sippo)->pos[0], &SIPPO(sippo)->pos[2]);
    w->pos[1] = y;
    w->heading = a;
    w->spin = b;
    w->baseY = d;
}

extern u8 nobukoStandingAnim[]; /* nobuko's animations: standing, */
extern u8 nobukoSmallFiresAnim[]; /* the small fires, */
extern u8 nobukoBigFireAnim[]; /* the big fire, */
extern u8 nobukoCallAnim[]; /* the call, */
extern u8 nobukoSpinAnim[]; /* the spin, */
extern u8 nobukoHitAnim[]; /* hit, */
extern u8 nobukoBeatenAnim[]; /* and beaten */
void iheadDie(Obj *ihead);
void iheadCloseLetterbox(Obj *ihead);
f32 iheadCamHeading(void);
void nobukoScript(Obj *o);

/* "nobuko", the boss (anim 0.15) on her dai (kind 2) with her tail (sippo, kind 3), the ihead
 * (kind 4), a shadow, a damage box (100 by 420, kind 0x78), a life gauge (8 hits) and a helper
 * (kind 10) that makes her fires. Her script (nobukoScript) picks the states:
 * - 2/3 (and 31/32 after a hit): she turns to the camera's heading and spins after it;
 * - 4-7 and 8-12: she turns to a player and throws small fires (kind 6, sounds 0x29F/0x2A0
 *   in turn), 8-12 ten times;
 * - 13-17 and 18-19: the big fire (kind 5 from the helper), 10 or 4 times;
 * - 20/21: a call (kind 9 at 134, 160); 22-27: the spin, the tail stretching to 200 (sound
 *   0x113 each time she passes the camera), then back.
 * A hit (28/29, sound 0x314) takes a point; at none (33-37) she is beaten: sound 0x82A6, the
 * ihead dies (iheadDie), pompoms, sound 0x10E, the helper goes.
 * Matched 2026-10-11; it was parked for register colouring: case 3's `diff` conflicted with
 * w->rot[1]'s f2 in cases 4/24 (the fire's z takes f2 in cases 6/10/15 instead, spilled at each
 * sndPlay), so |diff| in cases 3 and 4 is a ternary in the test, as the twin gasyaBodyProc
 * writes it (tlb_0038.c:943); case 4's mathAtan2 spills to 0x60 and reloads into goal, declared
 * last. The fire coordinates are `pos + scale * (trig * K)` (tlb_0042.c:2073's spelling). */
s32 nobukoProc(Obj *o, s16 msg, void *arg);
s32 nobukoProc(Obj *o, s16 msg, void *arg) {
    NobukoW *w = NOBUKO(o);
    f32 x;
    f32 y;
    f32 z;
    f32 diff;
    f32 gap;
    f32 px;
    f32 py;
    f32 pz;
    f32 goal;

    switch (msg) {
        case 2:
            o->name = "nobuko";
            eneSetup(o, arg);
            /* nobukoStandingAnim passed and stored as a pointer here and at its three other sites (the
             * anim field is a void *; kabuki's lever, IDO_NOTES 378): IDO then keeps it as one
             * address constant and builds it in a1 at every site, as the game does. */
            w->anim = eneNewAnime(o, nobukoStandingAnim);
            w->state = 0;
            w->anim->scale[2] = 0.15f;
            w->anim->scale[0] = w->anim->scale[1] = w->anim->scale[2];
            w->ihead = (s32) eneMakeUnder(o, ENE_IHEAD, 1, 0.0f, 0.0f, 0.0f);
            w->shadow = shadowCreate();
            w->dai = eneMakeUnder(o, ENE_DAI, w->zone, w->x, w->y, w->z);
            (*daiGetPos)(w->dai, &w->daiPos[0], &w->daiPos[1], &w->daiPos[2]);
            w->sippo = eneMakeUnder(o, ENE_SIPPO, w->zone, w->x + 20.0f, w->y, w->z);
            (*sippoSetActive)(w->sippo, 1);
            (*sippoPlace)(w->sippo, w->rot[1], w->spin, w->zone, w->x, w->y, w->z, w->daiPos[1]);
            w->tailSway1 = 6.0f;
            w->tailSway2 = 2.0f;
            w->tailLen = 25.0f;
            w->spin = 0.0f;
            (*sippoSetLenSway)(w->sippo, w->tailLen, w->tailSway1, w->tailSway2);
            if ((w->dmg = ((BombDmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->anim->f, 100, 420, 0, 0, 0)) ==
                NULL) {
                return -1;
            }
            w->dmg->kind = 0x78;
            w->gauge = tskCreate(o, pipGauge_proc, 0, NULL);
            eneCounterReward(0, 1, 250.0f, 150.0f, 0.0f, 2, 250.0f, 150.0f, 0.0f);
            w->helper = eneMakeUnder(o, ENE_ATOYA, 0, 0.0f, 0.0f, 0.0f);
            w->life = 8;
            w->step = 0xFFFF;
            break;
        case 1:
            (*sippoSetActive)(w->sippo, 0);
            if (w->anim != NULL) {
                frmDestroy(&w->anim->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            break;
        case 0:
            eneStepHookPre(o);
            if (acollDmgWasHit(w->dmg) == 1 && w->state < 28) {
                w->life--;
                pipGaugeSetTarget(w->gauge, w->life * 50 / 8);
                if (w->life >= 0) {
                    w->state = 28;
                } else {
                    w->life = 0;
                }
            }
            switch (w->state) {
                case 0:
                    w->anim->anim = nobukoStandingAnim;
                    w->anim->frame = 0.0f;
                    w->timer = 60;
                    w->state = 1;
                case 1:
                    eneAnimLoop(o, w->anim, 0.7f);
                    break;
                case 2:
                case 31:
                    w->tailSway1 = 6.0f;
                    w->tailSway2 = 2.0f;
                    w->tailLen = 25.0f;
                    if (w->anim->anim != nobukoStandingAnim) {
                        w->anim->anim = nobukoStandingAnim;
                        w->anim->frame = 0.0f;
                    }
                    w->state++;
                case 3:
                case 32:
                    goal = iheadCamHeading();
                    /* z holds the turn left (the game keeps it in the fire's z's register: one
                     * variable) */
                    z = goal - w->goal;
                    if (z > 3.1415927f) {
                        w->goal += 6.2831855f;
                        z = goal - w->goal;
                    } else if (z < -3.1415927f) {
                        w->goal -= 6.2831855f;
                        z = goal - w->goal;
                    }
                    if ((w->goal < goal ? z : -z) < 0.20943952f) {
                        w->goal = goal;
                    } else if (goal < w->goal) {
                        w->goal -= 0.20943952f;
                    } else {
                        w->goal += 0.20943952f;
                    }
                    if (w->goal >= 6.2831855f) {
                        w->goal -= 6.2831855f;
                    }
                    if (w->goal < 0.0f) {
                        w->goal += 6.2831855f;
                    }
                    if (w->goal - w->rot[1] > 3.1415927f) {
                        w->rot[1] += 6.2831855f;
                    } else if (w->goal - w->rot[1] < -3.1415927f) {
                        w->rot[1] -= 6.2831855f;
                    }
                    if (w->goal < w->rot[1]) {
                        if (w->spin >= 0.0f && w->spinAcc >= 0.0f) {
                            w->spin = 0.2f;
                        }
                        w->spinAcc = -0.02f;
                    }
                    if (w->rot[1] <= w->goal) {
                        if (w->spin < 0.0f && w->spinAcc < 0.0f) {
                            w->spin = -0.2f;
                        }
                        w->spinAcc = 0.02f;
                    }
                    w->spin += w->spinAcc;
                    if (w->spin >= 0.5) {
                        w->spin = 0.5f;
                    }
                    if (w->spin <= -0.5) {
                        w->spin = -0.5f;
                    }
                    w->rot[1] += w->spin;
                    if (w->rot[1] >= 6.2831855f) {
                        w->rot[1] -= 6.2831855f;
                    }
                    if (w->rot[1] < 0.0f) {
                        w->rot[1] += 6.2831855f;
                    }
                    eneAnimLoop(o, w->anim, 0.8f);
                    nobukoScript(o);
                    break;
                case 4:
                case 8:
                    if (!plIsInPlay(w->player)) {
                        if ((w->player = eneRandomPlayerIndex()) == 0) {
                            return -1;
                        }
                    }
                    plGetModelPos(gGameWork.players[w->player], &px, &py, &pz);
                    goal = mathAtan2(px, pz);
                    goal += (f32) (fadeRand(80) - 40) / 100.0;
                    diff = goal - w->rot[1];
                    if (diff > 3.1415927f) {
                        w->rot[1] += 6.2831855f;
                        diff = goal - w->rot[1];
                    } else if (diff < -3.1415927f) {
                        w->rot[1] -= 6.2831855f;
                        diff = goal - w->rot[1];
                    }
                    if ((w->rot[1] < goal ? diff : -diff) < 0.20943951606750488) {
                        w->rot[1] = goal;
                        w->count = 0;
                        w->state++;
                    } else if (goal < w->rot[1]) {
                        w->rot[1] -= 0.20943951606750488;
                    } else {
                        w->rot[1] += 0.20943951606750488;
                    }
                    eneAnimLoop(o, w->anim, 0.8f);
                    break;
                case 5:
                case 9:
                    w->anim->anim = nobukoSmallFiresAnim;
                    w->anim->frame = 0.0f;
                    w->state++;
                case 6:
                case 10:
                    eneAnimPlayOnce(o, w->anim, 1.5f);
                    if (w->anim->frame >= 10.0) {
                        w->anim->frame = 10.0f;
                        w->wait = 10;
                        w->state++;
                        x = w->anim->pos[0] + w->anim->scale[0] * (mathSin(w->anim->rot[1]) * 80.0);
                        y = w->anim->pos[1] + w->anim->scale[1] * 40.0;
                        z = w->anim->pos[2] + w->anim->scale[2] * (mathCos(w->anim->rot[1]) * 80.0);
                        if (!w->fire) {
                            w->fire = 1;
                            DPRINTF("PPPPPPPPP SD_FIRE3_1 !!!!!!!!!!!!!!!!!\n");
                            sndPlay(0x29F);
                        } else {
                            w->fire = 0;
                            DPRINTF("QQQQQQQQQ SD_FIRE3_2 !!!!!!!!!!!!!!!!!\n");
                            sndPlay(0x2A0);
                        }
                        eneMakeUnder(o, ENE_FIRE, w->zone, x, y, z);
                    }
                    break;
                case 7:
                    if (w->wait-- == 0) {
                        w->wait = 0;
                        if (eneAnimPlayOnce(o, w->anim, 1.0f)) {
                            w->state = 2;
                        }
                    }
                    break;
                case 11:
                    w->wait -= 2;
                    if (w->wait == 0) {
                        w->wait = 2;
                        if (eneAnimPlayOnce(o, w->anim, 1.0f)) {
                            if (++w->count >= 10) {
                                w->state = 2;
                            } else {
                                w->wait = 4;
                                w->state = 12;
                            }
                        }
                    }
                    break;
                case 12:
                    w->rot[1] -= 0.2617993950843811;
                    if (w->wait-- == 0) {
                        w->state = 9;
                    }
                    break;
                case 13:
                case 18:
                    w->anim->anim = nobukoBigFireAnim;
                    w->anim->frame = 0.0f;
                    w->state++;
                case 14:
                case 19:
                    eneAnimPlayOnce(o, w->anim, 0.7f);
                    if (w->anim->frame >= 10.0) {
                        w->anim->frame = 10.0f;
                        w->count = 10;
                        if (w->state == 19) {
                            w->count = 4;
                        }
                        w->shot = 0;
                        w->state = 15;
                    }
                    break;
                case 15:
                    if (!w->shot) {
                        w->shot = 1;
                        x = w->anim->pos[0] + w->anim->scale[0] * (mathSin(w->anim->rot[1]) * 22.0);
                        y = w->anim->pos[1] + w->anim->scale[1] * 220.0;
                        z = w->anim->pos[2] + w->anim->scale[2] * (mathCos(w->anim->rot[1]) * 22.0);
                        eneMakeUnder(w->helper, ENE_BOMB, w->zone, x, y, z);
                    }
                    eneAnimPlayOnce(o, w->anim, 1.0f);
                    if (w->anim->frame >= 14.0) {
                        w->anim->frame = 14.0f;
                        w->wait = 3;
                        w->state = 16;
                    }
                    break;
                case 16:
                    if (w->wait-- <= 0) {
                        if (--w->count <= 0) {
                            w->wait = 20;
                            w->state = 17;
                        } else {
                            w->anim->frame = 10.0f;
                            w->shot = 0;
                            w->state = 15;
                        }
                    }
                    break;
                case 17:
                    if (w->wait-- <= 0) {
                        if (eneAnimPlayOnce(o, w->anim, 0.3f)) {
                            w->state = 2;
                        }
                    }
                    break;
                case 20:
                    w->anim->anim = nobukoCallAnim;
                    w->anim->frame = 0.0f;
                    w->state = 21;
                case 21:
                    if (eneAnimPlayOnce(o, w->anim, 0.4f)) {
                        w->state = 2;
                        eneMakeUnder(w->helper, ENE_IHANDPOINT, w->zone, 134.0f, 160.0f, 0.0f);
                    }
                    break;
                case 22:
                    w->anim->anim = nobukoSpinAnim;
                    w->anim->frame = 0.0f;
                    w->state = 23;
                case 23:
                    w->tailSway1 -= 0.3;
                    if (w->tailSway1 <= 0.0001) {
                        w->tailSway1 = 0.0f;
                    }
                    w->tailSway2 -= 0.1;
                    if (w->tailSway2 <= 0.0001) {
                        w->tailSway2 = 0.0f;
                    }
                    w->tailLen = 25.0f;
                    (*sippoSetLenSway)(w->sippo, w->tailLen, w->tailSway1, w->tailSway2);
                    if (eneAnimPlayOnce(o, w->anim, 0.7f) && w->tailSway1 == 0.0f && w->tailSway2 == 0.0f) {
                        w->wait = 250;
                        w->state = 24;
                        w->spin = 0.0f;
                    }
                    break;
                case 24:
                    w->tailLen += 2.0;
                    if (w->tailLen >= 200.0f) {
                        w->tailLen = 200.0f;
                    }
                    (*sippoSetLenSway)(w->sippo, w->tailLen, w->tailSway1, w->tailSway2);
                    if (w->tailLen >= 50.0f) {
                        goal = iheadCamHeading() - 0.7853982f;
                        gap = w->rot[1] - goal;
                        if (gap > 3.1415927f) {
                            goal += 6.2831855f;
                        } else if (gap < -3.1415927f) {
                            goal -= 6.2831855f;
                        }
                        if (goal <= w->rot[1] && w->passed == 0) {
                            w->passed = 1;
                        } else if (w->rot[1] <= goal && w->passed == 1) {
                            w->passed = 0;
                            sndPlay(0x113);
                        }
                    }
                    w->spin += 0.0052359877f;
                    if (w->spin > 0.10471976f) {
                        w->spin = 0.10471976f;
                        if (w->wait-- <= 0) {
                            w->state = 25;
                        }
                    }
                    w->rot[1] += w->spin;
                    if (w->rot[1] > 6.2831855f) {
                        w->rot[1] -= 6.2831855f;
                    }
                    break;
                case 25:
                    w->tailLen -= 4.0;
                    if (w->tailLen <= 25.0f) {
                        w->tailLen = 25.0f;
                    }
                    (*sippoSetLenSway)(w->sippo, w->tailLen, w->tailSway1, w->tailSway2);
                    w->spin -= 0.015707964f;
                    if (w->spin < 0.06283186f) {
                        w->spin = 0.06283186f;
                        w->state = 26;
                    }
                    w->rot[1] += w->spin;
                    if (w->rot[1] > 6.2831855f) {
                        w->rot[1] -= 6.2831855f;
                    }
                    break;
                case 26:
                    w->tailLen -= 4.0;
                    if (w->tailLen <= 25.0f) {
                        w->tailLen = 25.0f;
                    }
                    w->tailSway1 += 0.15;
                    if (w->tailSway1 >= 6.0f) {
                        w->tailSway1 = 6.0f;
                    }
                    w->tailSway2 += 0.05;
                    if (w->tailSway2 >= 2.0f) {
                        w->tailSway2 = 2.0f;
                    }
                    (*sippoSetLenSway)(w->sippo, w->tailLen, w->tailSway1, w->tailSway2);
                    w->rot[1] += w->spin;
                    if (w->rot[1] > 6.2831855f) {
                        w->rot[1] -= 6.2831855f;
                    }
                    if (w->tailLen == 25.0f && w->tailSway1 == 6.0f && w->tailSway2 == 2.0f) {
                        w->state = 27;
                    }
                    break;
                case 27:
                    if (eneAnimPlayOnce(o, w->anim, -0.4f)) {
                        w->state = 2;
                    }
                    w->rot[1] += w->spin;
                    if (w->rot[1] > 6.2831855f) {
                        w->rot[1] -= 6.2831855f;
                    }
                    break;
                case 28:
                    w->anim->anim = nobukoHitAnim;
                    w->anim->frame = 0.0f;
                    sndPlay(0x314);
                    w->state = 29;
                case 29:
                    w->tailLen -= 10.0;
                    if (w->tailLen <= 25.0f) {
                        w->tailLen = 25.0f;
                    }
                    if (eneAnimPlayOnce(o, w->anim, 0.2f) && w->tailLen == 25.0f) {
                        if (w->life <= 0) {
                            w->wait = 20;
                            w->state = 33;
                        } else {
                            w->state = 2;
                        }
                    }
                    break;
                case 30:
                    iheadCloseLetterbox((Obj *) w->ihead);
                    w->timer = 7000;
                    w->state = 31;
                    break;
                case 33:
                    w->tailSway1 = 6.0f;
                    w->tailSway2 = 2.0f;
                    w->tailLen = 25.0f;
                    w->rot[1] = 0.0f;
                    w->anim->anim = nobukoHitAnim;
                    w->anim->frame = 0.0f;
                    w->wait = 20;
                    w->count = 0;
                    sndPlay(0x82A6);
                    iheadDie((Obj *) w->ihead);
                    eneEndPlayersPowerDown();
                    effectPompomCreate(0.0f, 190.0f, 0.0f, 20.0f, 25.0f, 25.0f, 10, 8000, 2, 0.1f, 0.03f);
                    sndPlay(0x10E);
                    tskDestroy(w->helper);
                    w->state = 34;
                case 34:
                    eneAnimPlayOnce(o, w->anim, 0.07f);
                    if (w->anim->frame >= 3.0) {
                        w->anim->frame = 3.0f;
                        w->wait = 40;
                        w->state = 35;
                    }
                    break;
                case 35:
                    if (--w->wait <= 0 && eneAnimPlayOnce(o, w->anim, 0.07f)) {
                        w->state = 36;
                    }
                case 36:
                    w->anim->anim = nobukoBeatenAnim;
                    w->anim->frame = polAnimeLastFrame(w->anim);
                    w->state = 37;
                case 37:
                    if (eneAnimPlayOnce(o, w->anim, -0.08f)) {
                        w->wait = 80;
                        w->state = 38;
                    }
                    break;
                case 38:
                    break;
            }
            (*sippoSetLenSway)(w->sippo, w->tailLen, w->tailSway1, w->tailSway2);
            eneSetModelOnCourse(o, &w->anim->f);
            (*sippoPlace)(w->sippo, w->rot[1], w->spin, w->zone, w->x, w->y, w->z, w->daiPos[1]);
            shadowPut(w->shadow, w->anim->pos[0], w->anim->pos[1], w->anim->pos[2], 3.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Shows nobuko's gauge, fills it to 50, state 2. */
void nobukoShowGauge(Obj *nobuko) {
    NobukoW *w = NOBUKO(nobuko);

    pipGaugeOn(NOBUKO(nobuko)->gauge);
    pipGaugeSetTarget(w->gauge, 50);
    w->state = 2;
}

s32 nobukoGetIhead(Obj *nobuko) {
    return NOBUKO(nobuko)->ihead;
}

/* Nobuko's attack script: she picks a player at random each frame, and when her timer runs out
 * takes the next step, each an attack state and the frames until the step after; the last
 * waits 500 frames and starts the script again. */
void nobukoScript(Obj *o) {
    NobukoW *w = NOBUKO(o);

    w->target = eneRandomPlayer();
    if (w->timer-- <= 0) {
        switch (++w->step) {
            case 0:
                w->state = 13; w->timer = 20;
                break;
            case 1:
                w->state = 8; w->timer = 40;
                break;
            case 2:
                w->state = 18; w->timer = 4;
                break;
            case 3:
                w->state = 22; w->timer = 40;
                break;
            case 4:
                w->state = 13; w->timer = 10;
                break;
            case 6:
                w->state = 4; w->timer = 10;
                break;
            case 7:
                w->state = 4; w->timer = 10;
                break;
            case 8:
                w->state = 4; w->timer = 10;
                break;
            case 9:
                w->state = 4; w->timer = 10;
                break;
            case 10:
                w->state = 4; w->timer = 40;
                break;
            case 11:
                w->state = 18; w->timer = 4;
                break;
            case 12:
                w->state = 20; w->timer = 500;
                w->step = 0xFFFF;
                break;
        }
    }
}

/* "atoya": an enemy that only sets up and frees its model. The work pointer local is what
 * keeps obj in a3 and msg in its home, as the game does (IDO_NOTES 309). */
s32 atoyaProc(Obj *o, s16 msg, void *arg) {
    EneObj *w = ENE(o);

    switch (msg) {
        case 2:
            o->name = "atoya";
            eneSetup(o, arg);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(w->model);
            }
            break;
        case 0:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "atoya" (its proc atoyaProc): a flag the helper below flips. */
typedef struct AtoyaW {
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
    /* 0x34 */ u8 on;
} AtoyaW;

/* Flips atoya's flag and returns it. */
u8 atoyaToggle(Obj *o) {
    AtoyaW *w = (AtoyaW *) FX(o);

    if (w->on) {
        w->on = 0;
    } else {
        w->on = 1;
    }
    return w->on;
}
