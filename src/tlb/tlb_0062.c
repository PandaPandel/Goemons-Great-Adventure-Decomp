#include "tlb.h"

extern Frame *gFrmLayer3;
extern u8 kpakoDl[];
extern u8 kpakoRide[];
extern u8 kpaoyaFaceATex[];
extern u8 kpaoyaFaceBTex[];
AnimFrame kpakoBlinkAnim[] = {
    { (u32) kpaoyaFaceATex, 60 },
    { (u32) kpaoyaFaceBTex, 4 },
    { (u32) kpaoyaFaceATex, 30 },
    { (u32) kpaoyaFaceBTex, 4 },
    { (u32) kpaoyaFaceATex, 6 },
    { (u32) kpaoyaFaceBTex, 4 },
    { 0, 0 },
    { 0, 0 },
};
extern u8 kpaFireballDl[];
extern u8 kpaoyaAnim[];
extern u8 kpaoyaActBeatenRide[];
extern u8 kpaoyaFaceATex[];
extern u8 kpaoyaFaceBTex[];

/* A "fireball"'s work (task + 0x20). */
typedef struct FireballWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ Anim anim; /* its texture's (gItexTimedYellowAnim) */
    /* 0x14 */ s32 alpha;
    /* 0x18 */ ACollFrame *atk;
    /* 0x1C */ s32 time; /* steps before it fades (16 a step) */
    /* 0x20 */ f32 vel[3]; /* vel[1] rises 0.02 a step */
} FireballWork;

#define FIREBALL(task) ((FireballWork *) ((u32) (task) + sizeof(Obj)))

/* A "kpaoya"'s work (task + 0x20): an enemy's (EneObj) up to 0x34. */
typedef struct KpaoyaObj {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp0;
    /* 0x0C */ Frame *unused0C;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ s32 (*act)(Obj *obj, struct KpaoyaObj *w); /* its state; nonzero: it ends */
    /* 0x38 */ AnimeFrame *model;
    /* 0x3C */ RideFrame *ride; /* made when it is beaten */
    /* 0x40 */ ACollFrame *dmg;
    /* 0x44 */ u8 flash; /* steps of white flashing after a hit */
    /* 0x45 */ u8 phase; /* within the state */
    /* 0x46 */ u16 count; /* steps in the phase */
    /* 0x48 */ s8 hp;
    /* 0x49 */ u8 glow; /* the red light before it breathes fire */
    /* 0x4A */ u16 count2; /* steps since it last rose */
    /* 0x4C */ s16 animEnd; /* the animation's last frame */
    /* 0x4E */ s16 animTime; /* steps to it, and to the move's end */
    /* 0x50 */ f32 animStep;
    /* 0x54 */ s16 tx; /* where it moves to (x, y) */
    /* 0x56 */ s16 ty;
    /* 0x58 */ f32 vx;
    /* 0x5C */ f32 vy;
} KpaoyaObj;

#define KPAOYA(task) ((KpaoyaObj *) ((u32) (task) + sizeof(Obj)))

s32 kpaFireballProc(Obj *obj, s16 msg, void *arg);
s32 kpaoyaActWait(Obj *obj, KpaoyaObj *w);
s32 kpaoyaActFight(Obj *obj, KpaoyaObj *w);
s32 kpaoyaActBeaten(Obj *obj, KpaoyaObj *w);
s32 kpaoyaActDone(Obj *obj, KpaoyaObj *w);

/* Message 6's argument (plmove.c's): a place on the course, moved by the object. */
typedef struct PlPlace {
    /* 0x0 */ s16 zone;
    /* 0x4 */ f32 place;
    /* 0x8 */ f32 side;
    /* 0xC */ f32 height;
} PlPlace;

/* A "kpako"'s work (task + 0x20): an enemy's (EneObj) up to 0x34. */
typedef struct KpakoObj {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ Frame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3]; /* vel[1]: its bob this step, passed to a rider */
    /* 0x34 */ PolFrame *frame;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ Anim anim;
    /* 0x4C */ f32 phase; /* the bob's, 2 degrees a step */
} KpakoObj;

/* "kpako": a platform you can ride, bobbing up and down 0.1 in a sine, animated. eneSetup's
 * data is its turn (1024 a circle). */
s32 kpakoProc(Obj *obj, s16 msg, void *arg) {
    KpakoObj *w = (KpakoObj *) ((u32) obj + sizeof(Obj));
    s32 pad;
    s32 data;
    s32 pad2[5]; /* five words below data that nothing touches (the game's frame, 0x58) */

    switch (msg) {
        case 2:
            obj->name = "kpako";
            data = eneSetup(obj, arg);
            if ((w->frame = eneNewPol(obj, (Gfx *) kpakoDl)) == NULL) {
                return -1;
            }
            frmMove(&w->frame->f, gFrmLayer3);
            fadeAnimStart(&w->anim, (AnimFrame *) fadeSegToRam(kpakoBlinkAnim, obj->file), 0,
                          &w->frame->segs[3], eneGetFile(obj, 0));
            if ((w->ride = rideCreate(gRideTree1, (s32) obj, &w->frame->f,
                                      (RideData *) eneFileAddr(obj, kpakoRide, 0))) == NULL) {
                return -1;
            }
            w->rot[1] = data * PI / 512;
            break;
        case 1:
            frmDestroy(&w->ride->f);
            frmDestroy(&w->frame->f);
            break;
        case 6:
            ((PlPlace *) arg)->height += w->vel[1];
            break;
        case 0:
            fadeAnimStep(&w->anim);
            w->phase += 0.034906585f;
            w->vel[1] = mathSin(w->phase) * 0.1;
            w->y += w->vel[1];
            eneSetModelOnCourse(obj, &w->frame->f);
            eneStepHookPost(obj);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "fireball": a growing yellow fireball ("pol" frame, animated texture) that flies on its
 * velocity, rising, and fades after its time; message 7 to whatever it hits. */
s32 kpaFireballProc(Obj *obj, s16 msg, void *arg) {
    FireballWork *w = FIREBALL(obj);
    s32 n; /* the texture file's index in case 2, the time left in case 0 */

    switch (msg) {
        case 2:
            obj->name = "fireball";
            if ((w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                return -1;
            }
            w->frame->dl2 = (Gfx *) kpaFireballDl;
            floadGetSetRam((void **) w->frame->segs, 0x125);
            n = floadGetSetFile(0x125, 0);
            fadeAnimStart(&w->anim, gItexTimedYellowAnim, 0, &w->frame->segs[3], n);
            w->frame->flags |= 7;
            w->frame->scale[2] = 0.3f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->alpha = 255;
            if ((w->atk = ((ACollFrame * (*) (Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)(
                     &ene_atk_root_frame->frame, &w->frame->f, 20, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 0x78;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->atk->frame);
            break;
        case 0:
            fadeAnimStep(&w->anim);
            w->vel[1] += 0.02;
            w->frame->pos[0] += w->vel[0];
            w->frame->pos[1] += w->vel[1];
            w->frame->pos[2] += w->vel[2];
            w->frame->scale[0] *= 1.008f;
            w->frame->scale[1] *= 1.008f;
            w->frame->scale[2] *= 1.008f;
            n = --w->time;
            if (n < 0) {
                w->alpha -= 16;
                if (w->alpha < 0) {
                    return -1;
                }
            }
            if (w->atk != NULL && acollAtkWasHit(w->atk) == 1 && (Obj *) w->atk->hitPower != NULL) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, obj);
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombine(gGfxDlHead++, 0x30D261, 0x55A6FF7F);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 128, 0);
            gDPSetEnvColor(gGfxDlHead++, 255, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Makes a fireball under parent at (x, y, z) with velocity (vx, vy, vz), fading after time. */
Obj *kpaFireballMake(Obj *parent, f32 x, f32 y, f32 z, f32 vx, f32 vy, f32 vz, f32 time) {
    Obj *t = tskCreate(parent, kpaFireballProc, parent->file, NULL);

    if (t != NULL) {
        FIREBALL(t)->frame->pos[0] = x;
        FIREBALL(t)->frame->pos[1] = y;
        FIREBALL(t)->frame->pos[2] = z;
        FIREBALL(t)->vel[0] = vx;
        FIREBALL(t)->vel[1] = vy;
        FIREBALL(t)->vel[2] = vz;
        FIREBALL(t)->time = time;
    }
    return t;
}

/* The kpaoya's next animation and move take n steps. */
void kpaoyaSetMoveTime(Obj *obj, s16 n) {
    KPAOYA(obj)->animTime = n;
}

/* Its animation from frame 0 to 6 (rising). */
void kpaoyaAnimRise(Obj *obj) {
    KPAOYA(obj)->model->frame = 0.0f;
    KPAOYA(obj)->animEnd = 6;
    KPAOYA(obj)->animStep = (KPAOYA(obj)->animEnd - KPAOYA(obj)->model->frame) / KPAOYA(obj)->animTime;
}

/* Its animation from frame 6 to 11 (sinking). */
void kpaoyaAnimSink(Obj *obj) {
    KPAOYA(obj)->model->frame = 6.0f;
    KPAOYA(obj)->animEnd = 11;
    KPAOYA(obj)->animStep = (KPAOYA(obj)->animEnd - KPAOYA(obj)->model->frame) / KPAOYA(obj)->animTime;
}

/* Its face texture: the first (kpaoyaFaceATex) or the second (kpaoyaFaceBTex), as kpako's blink. */
void kpaoyaFaceA(Obj *obj) {
    KPAOYA(obj)->model->segs[3] = eneFileAddr(obj, kpaoyaFaceATex, 0);
}

void kpaoyaFaceB(Obj *obj) {
    KPAOYA(obj)->model->segs[3] = eneFileAddr(obj, kpaoyaFaceBTex, 0);
}

/* Moves it to (x, y) on its course in its animTime steps. */
void kpaoyaMoveTo(Obj *obj, s16 x, s16 y) {
    KPAOYA(obj)->tx = x;
    KPAOYA(obj)->ty = y;
    KPAOYA(obj)->vx = (KPAOYA(obj)->tx - KPAOYA(obj)->x) / KPAOYA(obj)->animTime;
    KPAOYA(obj)->vy = (KPAOYA(obj)->ty - KPAOYA(obj)->y) / KPAOYA(obj)->animTime;
}

/* "kpaoya": a boss that rises from the water when a player passes 1940, breathes fireballs, and
 * sinks; five hits beat it, and it stays as a platform (a ride). Runs its state (act) each step,
 * then its move and animation; message 3 lights it red (glow) or white (a hit's flash). */
s32 kpaoyaProc(Obj *obj, s16 msg, void *arg) {
    KpaoyaObj *w = KPAOYA(obj);
    s32 pad[2];
    s32 data;
    s32 pad2[3];

    switch (msg) {
        case 2:
            obj->name = "kpaoya";
            data = eneSetup(obj, arg);
            if ((w->model = eneNewAnime(obj, (u32) kpaoyaAnim)) == NULL) {
                return -1;
            }
            frmMove(&w->model->f, gFrmLayer3);
            w->rot[1] = data * PI / 512;
            w->dmg = ((ACollFrame * (*) (Frame *, Frame *, u16, u16, s16, s16, s16, s32)) acollDmgAddXYZ)(
                &ene_dmg_root_frame->frame, &w->model->f, 500, 1000, 0, -500, 0, 0);
            w->animTime = 16;
            w->act = kpaoyaActWait;
            kpaoyaFaceB(obj);
            w->hp = 5;
            if (gGameWork.kpaoyaBeaten != 0) {
                w->act = kpaoyaActDone;
            }
            break;
        case 1:
            frmDestroy(&w->dmg->frame);
            frmDestroy(&w->ride->f);
            frmDestroy(&w->model->f);
            break;
        case 6:
            if (arg != NULL) {
                /* FAKE: empty, as in the game (it tests arg and branches past nothing) */
            }
            break;
        case 0:
            if (w->act != NULL && w->act(obj, w) != 0) {
                return -1;
            }
            if (w->vx != 0.0f) {
                if (fabsf(w->tx - w->x) < fabsf(w->vx)) {
                    w->x = w->tx;
                    w->vx = 0.0f;
                } else {
                    w->x += w->vx;
                }
            }
            if (w->vy != 0.0f) {
                if (fabsf(w->ty - w->y) < fabsf(w->vy)) {
                    w->y = w->ty;
                    w->vy = 0.0f;
                } else {
                    w->y += w->vy;
                }
            }
            w->model->frame += w->animStep;
            if (w->animEnd < w->model->frame) {
                w->model->frame = w->animEnd;
            }
            if (w->flash != 0) {
                w->flash--;
            }
            eneSetModelOnCourse(obj, &w->model->f);
            eneStepHookPost(obj);
            break;
        case 3:
            if (w->glow) {
                if (w->model->f.parent != gFrmLayers[2]) {
                    frmMove(&w->model->f, gFrmLayers[2]);
                }
                w->model->dl = eneDlEnvColorLit(obj, 0x60, w->glow, 0, 0, 1);
            } else if (w->flash & 2) {
                if (w->model->f.parent != gFrmLayers[2]) {
                    frmMove(&w->model->f, gFrmLayers[2]);
                }
                w->model->dl = eneDlEnvColorLit(obj, 0x80, 0xFF, 0xFF, 0xFF, 0);
            } else {
                if (w->model->f.parent != gFrmLayers[3]) {
                    frmMove(&w->model->f, gFrmLayers[3]);
                }
                w->model->dl = NULL;
            }
            break;
        default:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Its first state: it waits under the water at 2084 on course 1 until a player is past 1940, then
 * rises (phase 1) and opens its eyes (phase 2), then fights (kpaoyaActFight). */
s32 kpaoyaActWait(Obj *obj, KpaoyaObj *w) {
    s32 i;
    s32 zone;
    f32 pos[3];

    w->count++;
    switch (w->phase) {
        case 0:
            w->zone = 1;
            w->x = 2084.0f;
            w->y = 26.0f;
            w->z = 0.0f;
            for (i = 0; i < 4; i++) {
                if (plIsInPlay(i)) {
                    plAskPos(gGameWork.players[i], &pos[0], &pos[1], &pos[2], &zone);
                    if (pos[0] > 1940.0f) {
                        w->count = 0;
                        w->phase = 1;
                        break;
                    }
                }
            }
            break;
        case 1:
            if (w->count == 40) {
                kpaoyaFaceA(obj);
                kpaoyaSetMoveTime(obj, 30);
                kpaoyaMoveTo(obj, 0x824, 40);
                w->count = 0;
                w->phase = 2;
            }
            break;
        case 2:
            if (w->count == 20) {
                kpaoyaSetMoveTime(obj, 30);
                kpaoyaAnimRise(obj);
            } else if (w->count == 60) {
                kpaoyaSetMoveTime(obj, 30);
                kpaoyaAnimSink(obj);
            } else if (w->count == 90) {
                w->phase = 0;
                w->count = 0;
                w->act = kpaoyaActFight;
            }
            break;
    }
    return 0;
}

/* The fight: up (0) with its damage box live for 90 steps, then rise (2), breathe a fireball
 * every 8 steps (3), sink (4); a hit (1) ducks it. At no hp it is beaten (kpaoyaActBeaten). */
s32 kpaoyaActFight(Obj *obj, KpaoyaObj *w) {
    f32 p[3];
    f32 v[3];

    w->count++;
    w->count2++;
    switch (w->phase) {
        case 0:
            w->dmg->flags |= 2;
            if (w->count2 >= 91) {
                w->count = 0;
                w->phase = 2;
            }
            break;
        case 2:
            w->dmg->flags &= ~2;
            w->glow += 8;
            if (w->count == 1) {
                kpaoyaSetMoveTime(obj, 30);
                kpaoyaFaceA(obj);
                kpaoyaAnimRise(obj);
                kpaoyaMoveTo(obj, 0x824, 40);
                return 0;
            }
            if (w->count == 30) {
                w->glow = 255;
                w->count = 0;
                w->phase = 3;
                sndPlay(0x2A5);
            }
            return 0;
        case 3:
            if ((w->count & 7) == 1) {
                p[0] = w->model->scale[0] * 0;
                p[1] = w->model->scale[1] * -67.0f;
                p[2] = w->model->scale[2] * 231.0f;
                mathVecRotateY(w->model->rot[1], &p[0], &p[1], &p[2]);
                p[0] += w->model->pos[0];
                p[1] += w->model->pos[1];
                p[2] += w->model->pos[2];
                v[0] = 0.0f;
                v[1] = -0.3f;
                v[2] = -1.5f;
                mathVecRotateY(vramCourseHeading(w->zone, w->x), &v[0], &v[1], &v[2]);
                kpaFireballMake(obj, p[0], p[1], p[2], v[0], v[1], v[2], 64.0f);
            }
            if (w->count >= 91) {
                w->count = 0;
                w->phase = 4;
            }
            return 0;
        case 4:
            w->glow -= 8;
            if (w->count == 1) {
                kpaoyaSetMoveTime(obj, 30);
                kpaoyaFaceA(obj);
                kpaoyaAnimSink(obj);
                kpaoyaMoveTo(obj, 0x824, 40);
                return 0;
            }
            if (w->count == 30) {
                w->count2 = 0;
                w->glow = 0;
                w->count = 0;
                w->phase = 0;
            }
            return 0;
        case 1:
            w->dmg->flags &= ~2;
            if (w->count == 1) {
                kpaoyaSetMoveTime(obj, 30);
                kpaoyaFaceB(obj);
                kpaoyaAnimRise(obj);
                kpaoyaMoveTo(obj, 0x838, 40);
                return 0;
            }
            if (w->count == 40) {
                kpaoyaAnimSink(obj);
                kpaoyaMoveTo(obj, 0x824, 40);
                return 0;
            }
            if (w->count == 70) {
                kpaoyaFaceA(obj);
                w->count = 0;
                w->phase = 0;
            }
            return 0;
    }
    if (w->dmg != NULL && acollDmgWasHit(w->dmg) == 1) {
        eneHitSound(w->dmg);
        w->hp--;
        w->flash = 12;
        w->count = 0;
        w->phase = 1;
    }
    if (w->hp <= 0) {
        w->phase = 0;
        w->count = 0;
        w->act = kpaoyaActBeaten;
        return 0;
    }
    return 0;
}

/* Beaten: marks it (gGameWork + 0xA0), sinks to 20 below the water as a ride, with its sound. */
s32 kpaoyaActBeaten(Obj *obj, KpaoyaObj *w) {
    switch (w->phase) {
        case 0:
            gGameWork.kpaoyaBeaten = 1;
            w->glow = 0;
            kpaoyaSetMoveTime(obj, 60);
            kpaoyaFaceB(obj);
            kpaoyaAnimSink(obj);
            kpaoyaMoveTo(obj, 0x824, -20);
            if (w->ride == NULL) {
                w->ride = rideCreate(gRideTree1, (s32) obj, &w->model->f,
                                     (RideData *) eneFileAddr(obj, kpaoyaActBeatenRide, 0));
            }
            SoundLocation2(0x2CC, gGameWork.view, w->model->pos[0], w->model->pos[1], w->model->pos[2], 0.0f,
                           0.0f, 1.5707964f);
            w->count = 0;
            w->phase = 1;
            break;
        case 1:
            kpaoyaMoveTo(obj, 0x824, -20);
            if (fabsf(w->vx) < 0.1f && fabsf(w->vy) < 0.1f) {
                sndPlay(0x82CC);
                w->phase = 2;
            }
            break;
        case 2:
            break;
    }
    return 0;
}

/* Already beaten (gGameWork + 0xA0 set): straight to the sunk platform. */
s32 kpaoyaActDone(Obj *obj, KpaoyaObj *w) {
    switch (w->phase) {
        case 0:
            w->glow = 0;
            kpaoyaSetMoveTime(obj, 1);
            kpaoyaFaceB(obj);
            kpaoyaAnimSink(obj);
            kpaoyaMoveTo(obj, 0x824, -20);
            if (w->ride == NULL) {
                w->ride = rideCreate(gRideTree1, (s32) obj, &w->model->f,
                                     (RideData *) eneFileAddr(obj, kpaoyaActBeatenRide, 0));
            }
            w->phase = 1;
            break;
        case 1:
            break;
    }
    return 0;
}
