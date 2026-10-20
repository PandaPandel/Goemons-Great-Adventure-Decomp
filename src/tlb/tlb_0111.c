#include "tlb.h"

/* Main's sinf and cosf, called through their addresses as TLB code calls main. */
#define sinf (*sinf)
#define cosf (*cosf)

/* "gasya" (its proc gasyaProc): what its steps below touch. */
typedef struct Tlb111GasyaW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone; /* the course it is on */
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ AnimeFrame *body;
    /* 0x10 */ f32 x; /* along the course */
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ AnimeFrame *rise; /* the animation it rises by */
    /* 0x2C */ AnimeFrame *anim; /* its head */
    /* 0x30 */ ACollFrame *dmg; /* the head's damage box */
    /* 0x34 */ f32 aimPlace;
    /* 0x38 */ f32 unused38; /* written 1940.0f when it has risen, never read */
    /* 0x3C */ f32 aimSpeed;
    /* 0x40 */ u8 pad40[0x4];
    /* 0x44 */ f32 phase; /* its rocking walk's */
    /* 0x48 */ u8 hp; /* hits left, from 3 */
    /* 0x49 */ u8 sinceHit; /* frames since the last hit (0: none); 0x60 ends it */
    /* 0x4A */ u8 strides;
    /* 0x4B */ u8 pad4B[0x5];
    /* 0x50 */ f32 dustPlace; /* where the last dust was made, 0 for none yet */
    /* 0x54 */ void *cam; /* the AI camera it holds while the players walk in */
} Tlb111GasyaW;

typedef ACollFrame *(*DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                    s32 a);

/* Its steps, called far; gasyaBodyFollow near. */
void gasyaWait(Obj *o);
void gasyaPlayersWalkIn(Obj *o);
void gasyaRise(Obj *o);
void gasyaWalk(Obj *o);
void gasyaHalt(Obj *o);
void gasyaEnd(Obj *o);
void gasyaBodyFollow(Obj *o);

extern u8 gasyaBodyAnim[]; /* its body's, rise's and head's animations */
extern u8 gasyaRiseAnim[];
extern u8 gasyaHeadAnim[];

#define DPRINTF

s32 gasyaProc(Obj *obj, s16 msg, void *arg);

/* "gasya", the gasha machine Goemon's party meets at the gate: a body, a rising part and a head
 * (animated models of 1.0), the head with a damage box, placed on the course by the gate. Each
 * frame its state's step runs (it waits, the players walk in, it rises and rocks along); three
 * hits to the head end it (state 5), each flashing it white and replaying the head's flinch.
 * The flinch's 7.0 + 0.3 is added at run time in the game, so the 0.3 is a variable (speed).
 * sinceHit is read once into a local (since), tested and stepped through it. */
s32 gasyaProc(Obj *obj, s16 msg, void *arg) {
    f32 speed;
    Tlb111GasyaW *w = (Tlb111GasyaW *) FX(obj);
    s32 since; /* frames since the last hit */

    switch (msg) {
        case 2:
            obj->name = "gasya";
            eneSetup(obj, arg);
            if ((w->body = eneAnimeModelMake(obj, (u32) gasyaBodyAnim, NULL, 0)) == NULL) {
                return -1;
            }
            w->body->scale[2] = 1.0f;
            w->body->scale[0] = w->body->scale[1] = w->body->scale[2];
            if ((w->rise = eneAnimeModelMake(obj, (u32) gasyaRiseAnim, NULL, 0)) == NULL) {
                return -1;
            }
            w->rise->scale[2] = 1.0f;
            w->rise->scale[0] = w->rise->scale[1] = w->rise->scale[2];
            if ((w->anim = eneAnimeModelMake(obj, (u32) gasyaHeadAnim, NULL, 0)) == NULL) {
                return -1;
            }
            w->anim->scale[2] = 1.0f;
            w->anim->scale[0] = w->anim->scale[1] = w->anim->scale[2];
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->anim->f, 26, 22, 0, 0, 0);
            vramCoursePointXZ(w->zone, w->x, w->z - 125.0f, &w->rise->pos[0], &w->rise->pos[2]);
            w->rise->pos[1] = 80.0f;
            w->rise->rot[1] = eneCourseAngle(obj, 8);
            DPRINTF("plDemoBusyP %d\n", w->state);
            vramCoursePointXZ(w->zone, w->x + 35.0f, w->z, &w->anim->pos[0], &w->anim->pos[2]);
            w->anim->pos[1] = 156.0f;
            w->anim->rot[1] = eneCourseAngle(obj, 8);
            DPRINTF("HEAD_Ani->ry %f", w->anim->rot[1]);
            w->body->rot[1] = eneCourseAngle(obj, 8);
            w->hp = 3;
            w->strides = 1;
            w->state = 0;
        case 0:
            switch (w->state) {
                case 0:
                    (*gasyaWait)(obj);
                    break;
                case 1:
                    (*gasyaPlayersWalkIn)(obj);
                    break;
                case 2:
                    (*gasyaRise)(obj);
                    break;
                case 3:
                    (*gasyaWalk)(obj);
                    break;
                case 4:
                    (*gasyaHalt)(obj);
                    break;
                case 5:
                    (*gasyaEnd)(obj);
                    break;
            }
            gasyaBodyFollow(obj);
            since = w->sinceHit;
            if (since == 0x60) {
                return -1;
            }
            if (since != 0) {
                w->sinceHit = since + 1;
                if (w->hp != 0) {
                    speed = 0.3f;
                    if (eneAnimPlayOnce(obj, w->anim, speed)) {
                        w->anim->frame = 0.0f;
                        w->sinceHit = 0;
                        w->dmg->flags |= 2;
                    }
                    if (w->anim->frame >= 7.0f && w->anim->frame < 7.0f + speed) {
                        w->anim->frame += 6.0f;
                    }
                }
            }
            if (acollDmgWasHit(w->dmg) == 1) {
                w->dmg->flags &= ~2;
                w->sinceHit = 1;
                if (--w->hp == 0) {
                    w->state = 5;
                }
            }
        case 3:
            if (w->sinceHit != 0 && (w->sinceHit & 2)) {
                w->anim->dl = eneDlEnvColorLit(obj, 255, 255, 255, 255, 0);
            } else {
                w->anim->dl = eneDlEnvColorLit(obj, -w->hp * 6 + 18, 255, 0, 0, 0);
            }
            w->body->dl = eneDlEnvColorLit(obj, -w->hp * 6 + 18, 255, 0, 0, 0);
            w->rise->dl = eneDlEnvColorLit(obj, -w->hp * 6 + 18, 255, 0, 0, 0);
            break;
        case 1:
            if (w->body != NULL) {
                frmDestroy(&w->body->f);
            }
            if (w->rise != NULL) {
                frmDestroy(&w->rise->f);
            }
            if (w->anim != NULL) {
                frmDestroy(&w->anim->f);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Waits until a player reaches x 1740; then players 1 and 2 walk to the gate (1980 and 1960,
 * height 156) and the AI camera holds (state 1). w is declared first with one pad word after it,
 * which puts the compiler temps at the game's 0x24 (IDO_NOTES 348). */
void gasyaWait(Obj *o) {
    Tlb111GasyaW *w;
    s32 pad;
    s32 zone0;
    s32 zone1;
    f32 x0;
    f32 y0;
    f32 z0;
    f32 x1;
    f32 y1;
    f32 z1;
    Vec3f goal0;
    Vec3f goal1;

    if (plIsInPlay(0)) {
        plAskPos(gGameWork.players[0], &x0, &y0, &z0, &zone0);
    }
    if (plIsInPlay(1)) {
        plAskPos(gGameWork.players[1], &x1, &y1, &z1, &zone1);
    }
    if (x0 >= 1740.0f || x1 >= 1740.0f) {
        if (plIsInPlay(0)) {
            plSetAction(gGameWork.players[0], 0xDC);
            goal0.x = 1980.0f;
            goal0.y = 156.0f;
            goal0.z = 0.0f;
            plDemoWalkTo(gGameWork.players[0], goal0, 2.0f);
        }
        if (plIsInPlay(1)) {
            plSetAction(gGameWork.players[1], 0xDC);
            goal1.x = 1960.0f;
            goal1.y = 156.0f;
            goal1.z = 0.0f;
            plDemoWalkTo(gGameWork.players[1], goal1, 2.0f);
        }
        w = (Tlb111GasyaW *) FX(o);
        w->cam = camAiGetCam();
        camAiSetHold(1);
        w->state = 1;
    }
}

#define GASYA(o) ((Tlb111GasyaW *) FX(o))

/* The AI camera looks on from the course's place 1970 (off 180, height 200) while the players
 * walk to the gate. With player 1 in play, it aims at his place on the course (height 188),
 * once its aim is short of 1980; with only player 2, at him, no further than 1980. When the
 * one it follows has stopped walking, he takes action 0xD7, the sun runs program 2 and the
 * state is 2. */
void gasyaPlayersWalkIn(Obj *o) {
    Tlb111GasyaW *w = GASYA(o);
    s32 pad0;
    f32 x;
    f32 y;
    f32 z;
    f32 ax;
    f32 ay;
    f32 az;
    f32 px;
    f32 py;
    f32 pz;
    s32 pad[3];
    s32 zone;
    s32 pad2[6];

    vramCoursePointXZ(2, 1970.0f, 180.0f, &x, &z);
    camSetSpeed(w->cam, 80.0f);
    camMoveEye(w->cam, x, 200.0f, z);
    if (plIsInPlay(0)) {
        camGetAt(w->cam, &x, &y, &z);
        if (vramNearestPlaceXZ(2, x, z, &az) < 1980.0f) {
            plAskPos(gGameWork.players[0], &px, &py, &pz, &zone);
            vramCoursePointXZ(zone, px, pz, &ax, &az);
            camSetSpeed(w->cam, 10.0f);
            camMoveAt(w->cam, ax, 188.0f, az);
        }
        if (plIsDemoWalking(gGameWork.players[0]) == 1) {
            return;
        }
        plSetAction(gGameWork.players[0], 0xD7);
    } else if (plIsInPlay(1)) {
        plAskPos(gGameWork.players[1], &ax, &ay, &az, &zone);
        camMoveAt(w->cam, ax < 1980.0f ? ax : 1980.0f, 188.0f, az);
        if (plIsDemoWalking(gGameWork.players[1]) == 1) {
            return;
        }
        plSetAction(gGameWork.players[1], 0xD7);
    }
    plIsDemoWalking(gGameWork.players[0]);
    camSunSetProgram(2);
    w->state = 2;
}

extern u8 gasyaBodyRiseAnim[]; /* animations in its segment */
extern u8 gasyaHeadRiseAnim[];

/* Plays its animation at 0.2; near its end it plays its rise at 0.3, lifting `rise` 2 a frame
 * from 80 for the first 19 frames with the body in step. When the rise ends both get new
 * animations, it goes to state 3 at 1910, 1940, and player 1 is put back to action 0. The
 * last stores' order (the head's frame, aimPlace, aimSpeed, unused38, state) is the game's schedule. */
void gasyaRise(Obj *o) {
    Tlb111GasyaW *w = (Tlb111GasyaW *) FX(o);
    s32 pad[14];
    s32 done;

    eneAnimPlayOnce(o, w->anim, 0.2f);
    if (polAnimeLastFrame(w->anim) - 1.0 <= w->anim->frame) {
        done = eneAnimPlayOnce(o, w->rise, 0.3f);
        if (w->rise->frame <= 19.0) {
            w->rise->pos[1] = w->rise->frame * 2 + 80.0f;
        }
        w->body->frame = w->rise->frame;
        if (done) {
            w->body->anim = gasyaBodyRiseAnim;
            w->body->frame = 0.0f;
            w->anim->anim = gasyaHeadRiseAnim;
            w->anim->frame = 0.0f;
            w->aimPlace = 1910.0f;
            w->aimSpeed = 0.0f;
            w->unused38 = 1940.0f;
            w->state = 3;
            plSetAction(gGameWork.players[0], 0);
        }
    }
}

extern u8 gasyaBodyWalkAnim[]; /* its walk */
extern u8 gasyaRiseWalkAnim[];

/* Walks along the course toward player 1: its place aimPlace moves by aimSpeed, which builds up by 0.1
 * a frame to 1.5 either way and eases off (90%) within 15 of him. It faces a point 30 on along
 * the course and rocks (0.1 of cos phase, less 0.1), the phase going on by 0.1 a frame; each
 * time the phase passes 90 degrees it counts a step (strides). On every 6th step it stops (state
 * 4, new animations); on even steps it plays its walk at 0.2 (a step more at its end) and, on
 * frame 1, raises a puff (kind 0x136) at its nearest place on its course, 20 below it.
 * Matched 2026-10-16; it was parked for which value held f12 (place) and which f2 (px); every
 * place read routed through w (not a local), with an empty else ending the if-chain (sm64's
 * bowser.inc.c:670 form) in place of the FAKE tests, gave the game's colouring. */
void gasyaWalk(Obj *o) {
    s32 pad0[1]; /* FAKE: frame word; with y and near, the game's slots */
    f32 y;
    f32 near;
    s32 zone;
    f32 px;
    f32 py;
    f32 pz;
    f32 x;
    f32 z;
    Tlb111GasyaW *w;
    f32 place; /* unused: the game's frame has its word */
    f32 side;

    w = (Tlb111GasyaW *) FX(o);
    plAskPos(gGameWork.players[0], &px, &py, &pz, &zone);
    if (w->aimPlace < px) {
        if (((w->aimPlace < px) ? px - w->aimPlace : -(px - w->aimPlace)) < 15.0f) {
            w->aimSpeed *= 0.9;
            w->aimPlace = w->aimPlace + w->aimSpeed;
        } else {
            w->aimPlace = w->aimPlace + w->aimSpeed;
            if (w->aimSpeed < 1.5) {
                w->aimSpeed += 0.1;
            }
        }
    } else if (px < w->aimPlace) {
        if (((w->aimPlace < px) ? px - w->aimPlace : -(px - w->aimPlace)) < 15.0f) {
            w->aimSpeed *= 0.9;
            w->aimPlace = w->aimPlace + w->aimSpeed;
        } else {
            w->aimPlace = w->aimPlace + w->aimSpeed;
            if (w->aimSpeed > -1.5) {
                w->aimSpeed -= 0.1;
            }
        }
    } else {
    }
    vramCoursePointXZ(zone, w->aimPlace + 30.0f, 0.0f, &x, &z);
    w->body->rot[1] = mathAtan2(x - w->body->pos[0], z - w->body->pos[2]);
    w->phase += 0.1;
    if (w->phase >= 6.2831855f) {
        w->phase -= 6.2831855f;
    }
    if (w->phase >= 1.5707964f && w->phase < 1.5707964f + 0.1) {
        w->strides++;
    }
    w->body->rot[2] = cosf(w->phase) * 0.1 + -0.1;
    if (w->strides % 6 == 0) {
        w->body->anim = gasyaBodyWalkAnim;
        w->body->frame = 0.0f;
        w->rise->anim = gasyaRiseWalkAnim;
        w->rise->frame = 0.0f;
        w->state = 4;
        w->dustPlace = 0.0f;
        return;
    }
    if (!(w->strides % 2)) {
        if (eneAnimPlayOnce(o, w->body, 0.2f)) {
            w->body->frame = 0.0f;
            w->strides++;
        }
        if (w->body->frame >= 1.0 && w->body->frame < 1.2) {
            near = vramNearestPlaceXZ(w->zone, w->body->pos[0], w->body->pos[2], &side);
            y = w->body->pos[1] - 20.0f;
            eneMakeEx(NULL, ENE_GASYA_FIRE, w->zone, near, y, side, 0, NULL);
        }
    }
}

/* Turns the body to 1.36 (0.04 a frame) and plays the rise at 0.2 with the body in step; when
 * it ends the body takes its first animation, the state is 3 at 1910 and strides counts up.
 * Every 4th frame of the body's frames 12-27 a dust (kind 0x137) rises at the nearest place on
 * its course, 20 below it, carrying how far the place moved and the height (both x100).
 * Matched 2026-10-16; it was parked for a missing float range (eneMakeEx's x copy); copying the
 * call's result to x right after the call (gasyaBodyCalcAim's precedent for one result held in
 * two named variables) let the colouring give x, px and z their registers as the game does. */
void gasyaHalt(Obj *o) {
    Tlb111GasyaW *w = (Tlb111GasyaW *) FX(o);
    s32 done;
    s32 pad0; /* FAKE: frame word; with done, the game's frame 0x68 */
    f32 place;
    f32 x;
    f32 side;
    s32 pad2;
    f32 y;
    f32 z;
    f32 dx;
    f32 speed = 0.2f;

    if (w->body->rot[1] > 1.36) {
        w->body->rot[1] -= speed / 5.0f;
        if (w->body->rot[1] < 1.36) {
            w->body->rot[1] = 1.36f;
        }
    } else if (w->body->rot[1] < 1.36) {
        w->body->rot[1] += speed / 5.0f;
        if (w->body->rot[1] > 1.36) {
            w->body->rot[1] = 1.36f;
        }
    }
    done = eneAnimPlayOnce(o, w->rise, speed);
    w->body->frame = w->rise->frame;
    if (done) {
        w->body->anim = gasyaBodyRiseAnim;
        w->body->frame = 0.0f;
        w->aimPlace = 1910.0f;
        w->state = 3;
        w->aimSpeed = 0.0f;
        w->strides++;
    }
    if (!(w->body->frame < 12.0f) && !(w->body->frame > 27.0f) && !(gGameFrameCount & 3)) {
        place = vramNearestPlaceXZ(w->zone, w->body->pos[0], w->body->pos[2], &side);
        x = place;
        z = side;
        y = w->body->pos[1] - 20.0f;
        if (w->dustPlace != 0.0f) {
            dx = w->dustPlace - place;
        } else {
            dx = 0.0f;
        }
        /* FAKE: the height read as a Vec3f field (IDO_NOTES 198, as eneproc.c's VEL): as the
         * array element pos[1] IDO sorts the OR's operands and packs the height's half first */
        eneMakeEx(NULL, ENE_GASYA_FIRE2, w->zone, x, y, z, 2,
                  (void *) (((s16) (dx * 100.0f) & 0xFFFF) | (((s16) (((Vec3f *) w->body->pos)->y * 100.0f) << 16) & 0xFFFF0000)));
        w->dustPlace = place;
    }
}

/* Loops its animation (0.4 a frame) between frames 7 and 13; at 0x60 it lets the camera go. */
void gasyaEnd(Obj *o) {
    Tlb111GasyaW *w = (Tlb111GasyaW *) FX(o);

    eneAnimLoop(o, ((Tlb111GasyaW *) FX(o))->anim, 0.4f);
    if (w->anim->frame >= 13.0) {
        w->anim->frame -= 6.0;
    }
    if (w->sinceHit == 0x60) {
        camAiSetHold(0);
    }
}

/* "fire"'s work (its proc gasyaFireProc): EneObj's position and velocity, then two
 * flame models each with a texture animation. */
typedef struct Tlb111FireW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *frame;
    /* 0x10 */ f32 x; /* along the course */
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ Anim anim;
    /* 0x44 */ PolFrame *frame2;
    /* 0x48 */ Anim anim2;
    /* 0x58 */ ACollFrame *atk;
    /* 0x5C */ u8 pad5C[0x2];
    /* 0x5E */ u16 alpha; /* from 0x4FF, by 0x20 a frame once it lands */
} Tlb111FireW;

typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);

extern Gfx gasyaFireFlame0Dl[]; /* its two flames */
extern Gfx gasyaFireFlame1Dl[];
extern Gfx gasyaFireLandedDl[]; /* the landed flame */
extern u8 gasyaLandedFlameTex0[];
extern u8 gasyaLandedFlameTex2[];
extern u8 gasyaLandedFlameTex5[];
extern u8 gasyaFireBurstTex3[];
extern u8 gasyaFireBurstTex10[];
AnimFrame gasyaFireAnim[] = {
    { (u32) gasyaLandedFlameTex0, 2 },
    { 0x03000D80, 2 },
    { (u32) gasyaLandedFlameTex2, 2 },
    { 0x03001D80, 2 },
    { 0x03002580, 2 },
    { (u32) gasyaLandedFlameTex5, 2 },
    { 0x03003580, 2 },
    { 0x03003D80, 2 },
    { 0x03004580, 2 },
    { 0x03004D80, 2 },
    { 0x03005580, 2 },
    { 0x03005D80, 2 },
    { 0x03006580, 2 },
    { 0x03006D80, 2 },
    { 0x03007580, 2 },
    { 0x03007D80, 2 },
    { 0, 0 },
}; /* their texture animation (file 0x125) */

/* "fire", the gasha machine's fireball: two flame models of 0.2 (file 0x125) with an attack box
 * against the players (kind 0x7B, its radius 80 times the scale), thrown at speed 2 toward the
 * first player's head (state 1). Landed (z 0 or more), the second flame takes its landed model
 * and grows to 0.2 by 0.025 while both fade by 0x20 a frame; under 0x20 it is gone (state 2).
 * Hitting a player, it sends message 7 to the player's task. The texture file is read into a
 * local before each fadeAnimStart: passed inline it costs a compiler temporary, which put
 * plAskPos's locals one word high (0x5C-0x68 for the game's 0x58-0x64). */
s32 gasyaFireProc(Obj *obj, s16 msg, void *arg);
s32 gasyaFireProc(Obj *obj, s16 msg, void *arg) {
    Tlb111FireW *w = (Tlb111FireW *) FX(obj);
    s32 file;
    s32 zone;
    f32 x;
    f32 y;
    f32 z;

    switch (msg) {
        case 2:
            obj->name = "fire";
            eneSetup(obj, arg);
            w->alpha = 0x4FF;
            if ((w->frame = enePolModelMake(obj, gasyaFireFlame0Dl, NULL, 1)) == NULL) {
                return -1;
            }
            floadGetSetRam((void **) w->frame->segs, 0x125);
            w->frame->flags |= 7;
            w->frame->scale[2] = 0.2f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            file = floadGetSetFile(0x125, 0);
            fadeAnimStart(&w->anim, gasyaFireAnim, 0, &w->frame->segs[3], file);
            if ((w->frame2 = enePolModelMake(obj, gasyaFireFlame1Dl, NULL, 1)) == NULL) {
                return -1;
            }
            floadGetSetRam((void **) w->frame2->segs, 0x125);
            w->frame2->flags |= 7;
            w->frame2->scale[2] = 0.2f;
            w->frame2->scale[0] = w->frame2->scale[1] = w->frame2->scale[2];
            file = floadGetSetFile(0x125, 0);
            fadeAnimStart(&w->anim2, gasyaFireAnim, 0, &w->frame2->segs[3], file);
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f,
                                                         w->frame->scale[0] * 10.0f * 8.0f, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 0x7B;
            plAskPos(gGameWork.players[0], &x, &y, &z, &zone);
            eneSetVelToward(obj, 2.0f, x, w->frame->scale[0] * 10.0f * 4.0f + y, z);
            w->state = 1;
        case 0:
            switch (w->state) {
                case 1:
                    fadeAnimStep(&w->anim);
                    fadeAnimStep(&w->anim2);
                    if (w->z >= 0.0f) {
                        DPRINTF("テクスチャ変更\n");
                        w->frame2->dl2 = gasyaFireLandedDl;
                        w->frame2->scale[1] = 0.1f;
                        w->vel[0] = 0.0f;
                        w->vel[1] = 0.0f;
                        w->vel[2] = 0.0f;
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (w->frame2->scale[1] < 0.2) {
                        w->frame2->scale[1] += 0.025;
                    } else {
                        w->frame2->scale[1] = 0.2f;
                    }
                    fadeAnimStep(&w->anim);
                    fadeAnimStep(&w->anim2);
                    w->alpha -= 0x20;
                    DPRINTF("timer 0x%x, sy %f\n", w->alpha, w->frame2->scale[1]);
                    if (w->alpha < 0x20) {
                        return -1;
                    }
                    break;
            }
            w->x += w->vel[0];
            w->y += w->vel[1];
            w->z += w->vel[2];
            vramCoursePointXZ(w->zone, w->x, w->z, &w->frame->pos[0], &w->frame->pos[2]);
            w->frame->pos[1] = w->y;
            w->frame2->pos[0] = w->frame->pos[0];
            w->frame2->pos[1] = w->frame->pos[1];
            w->frame2->pos[2] = w->frame->pos[2] - 0.1;
            if (acollAtkWasHit(w->atk) == 1) {
                if ((Obj *) w->atk->hitPower != NULL) {
                    DPRINTF("eneAfterProc(): 攻撃ヒット\n");
                    tskSendMsg((Obj *) w->atk->hitPower, 7, obj);
                }
            }
        case 3:
            w->frame->dl = eneDlEnvPrimColor(obj, 255, 255, 63, 127, 0, 0, (w->alpha >= 256) ? 255 : w->alpha);
            w->frame->dl = eneDlEnvPrimColor(obj, 255, 255, 127, 255, 0, 0, (w->alpha >= 256) ? 255 : w->alpha);
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            if (w->frame2 != NULL) {
                frmDestroy(&w->frame2->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The second "fire"'s work (its proc gasyaFire2Proc): one flame model. */
typedef struct Tlb111Fire2W {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *frame;
    /* 0x10 */ f32 x; /* along the course */
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ Anim anim;
    /* 0x44 */ u8 pad44[0x14];
    /* 0x58 */ ACollFrame *atk;
    /* 0x5C */ u16 alpha;
    /* 0x5E */ u16 timer;
} Tlb111Fire2W;

AnimFrame gasyaFireBurstAnim[] = {
    { 0x030020D0, 2 },
    { 0x030028D0, 2 },
    { 0x030030D0, 2 },
    { (u32) gasyaFireBurstTex3, 2 },
    { 0x030040D0, 2 },
    { 0x030048D0, 2 },
    { 0x030050D0, 2 },
    { 0x030058D0, 2 },
    { 0x030060D0, 2 },
    { 0x030068D0, 2 },
    { (u32) gasyaFireBurstTex10, 2 },
    { 0x030078D0, 2 },
    { 0, 0 },
}; /* its burst's texture animation (file 0x11F) */

/* The second "fire": one flame model of 0.3 (file 0x125) with an attack box against the players
 * (kind 0x7B, radius 15), thrown at speed 3 toward the course's nearest place to the first
 * player, its arg's sa x5 short of it and ty - 45 high (state 1). It grows by 1.038 a frame;
 * at 21 frames it bursts (file 0x11F, 16 lower, its width x0.6, state 2) and slows by 0.98 a
 * frame, its attack box off from the burst's 4th frame, until the burst ends. Hitting a player,
 * it sends message 7 to the player's task. */
s32 gasyaFire2Proc(Obj *obj, s16 msg, void *arg) {
    s32 v;
    Tlb111Fire2W *w = (Tlb111Fire2W *) FX(obj);
    s32 zone;
    f32 x;
    f32 y;
    f32 z;
    f32 sa;
    f32 ty;

    switch (msg) {
        case 2:
            obj->name = "fire";
            v = eneSetup(obj, arg);
            sa = (f32) (v & 0xFFFF) / 100.0f;
            ty = (f32) ((v >> 16) & 0xFFFF) / 100.0f;
            DPRINTF("sa %f, ty %f\n", sa, ty);
            w->alpha = 0xFF;
            if ((w->frame = enePolModelAlpha(obj, gasyaFireFlame1Dl, w->alpha)) == NULL) {
                return -1;
            }
            floadGetSetRam((void **) w->frame->segs, 0x125);
            fadeAnimStart(&w->anim, gasyaFireAnim, 0, &w->frame->segs[3], floadGetSetFile(0x125, 0));
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 15, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 0x7B;
            w->frame->flags |= 7;
            w->frame->scale[2] = 0.3f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            plAskPos(gGameWork.players[0], &x, &y, &z, &zone);
            x = vramNearestPlaceXZ(w->zone, w->frame->pos[0], w->frame->pos[2], &z) - sa * 5.0f;
            eneSetVelToward(obj, 3.0f, x, ty - 45.0f, 0.0f);
            w->state = 1;
        case 0:
            switch (w->state) {
                case 1:
                    fadeAnimStep(&w->anim);
                    if (++w->timer >= 21) {
                        w->frame->dl2 = gasyaFireFlame1Dl;
                        floadGetSetRam((void **) w->frame->segs, 0x11F);
                        fadeAnimStart(&w->anim, gasyaFireBurstAnim, 0, &w->frame->segs[3], floadGetSetFile(0x11F, 0));
                        w->y -= 16.0f;
                        w->frame->scale[0] *= 0.6;
                        w->state = 2;
                    } else {
                        w->frame->scale[0] *= 1.038;
                        w->frame->scale[1] *= 1.038;
                        w->frame->scale[2] *= 1.038;
                        DPRINTF("sx %f\n", w->frame->scale[0]);
                    }
                    break;
                case 2:
                    w->vel[0] *= 0.98;
                    w->vel[1] *= 0.98;
                    w->vel[2] *= 0.98;
                    if (fadeAnimStep(&w->anim)) {
                        return -1;
                    }
                    if (w->anim.index >= 4) {
                        w->atk->flags &= ~2;
                    }
                    w->alpha -= 8;
                    break;
            }
            w->x += w->vel[0];
            w->y += w->vel[1];
            w->z += w->vel[2];
            vramCoursePointXZ(w->zone, w->x, w->z, &w->frame->pos[0], &w->frame->pos[2]);
            w->frame->pos[1] = w->y;
            w->frame->rot[0] = w->rot[0];
            w->frame->rot[1] = w->rot[1];
            w->frame->rot[2] = w->rot[2];
            if (acollAtkWasHit(w->atk) == 1) {
                if ((Obj *) w->atk->hitPower != NULL) {
                    DPRINTF("eneAfterProc(): 攻撃ヒット\n");
                    tskSendMsg((Obj *) w->atk->hitPower, 7, obj);
                }
            }
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0,
                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 63, 0);
            gDPSetEnvColor(gGfxDlHead++, 159, 0, 0, 255);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* mdlAnimNodePos with its frame as the f32 it is (tlb.h declares it s32, as model.c defines
 * it; ombupl.c and pldive.c declare it so too). */
typedef void (*NodePosFunc)(u32 addr, f32 frame, u32 base, f32 *x, f32 *y, f32 *z);

extern u8 gasyaRiseAnim[]; /* the rise's three animations, */
extern u8 gasyaRiseWalkAnim[];
extern u8 gasyaRiseAnim2[];
extern u8 gasyaRiseAttach[]; /* and each one's node the body hangs from */
extern u8 gasyaRiseWalkAttach[];
extern u8 gasyaRiseAttach2[];

/* Keeps the body at its node of the rise's animation (the node by which animation is on),
 * turned with the rise's heading. The file base is read into a local before each call (inline
 * as an argument it costs a compiler temporary, which put x, y, z one word high). IDO calls
 * the sums' functions left to right and emits `a + b` as b's register then a's, so the game's
 * source has the rise's position first and the products in the order the calls run. */
void gasyaBodyFollow(Obj *o) {
    Tlb111GasyaW *w = (Tlb111GasyaW *) FX(o);
    u32 base;
    f32 x;
    f32 y;
    f32 z;
    s32 pad;

    if (((Tlb111GasyaW *) FX(o))->rise->anim == gasyaRiseAnim) {
        base = eneFileAddr(o, 0, 0);
        ((NodePosFunc) mdlAnimNodePos)((u32) gasyaRiseAttach, w->rise->frame, base, &x, &y, &z);
    } else if (w->rise->anim == gasyaRiseWalkAnim) {
        base = eneFileAddr(o, 0, 0);
        ((NodePosFunc) mdlAnimNodePos)((u32) gasyaRiseWalkAttach, w->rise->frame, base, &x, &y, &z);
    } else if (w->rise->anim == gasyaRiseAnim2) {
        base = eneFileAddr(o, 0, 0);
        ((NodePosFunc) mdlAnimNodePos)((u32) gasyaRiseAttach2, w->rise->frame, base, &x, &y, &z);
    }
    w->body->pos[0] = w->rise->pos[0] + (z * sinf(w->rise->rot[1]) + cosf(w->rise->rot[1]) * x);
    w->body->pos[2] = w->rise->pos[2] + (z * cosf(w->rise->rot[1]) - sinf(w->rise->rot[1]) * x);
    w->body->pos[1] = y + w->rise->pos[1];
}
