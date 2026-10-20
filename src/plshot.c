#include "player.h"
#include "contpak.h"
#include "effect.h"
#include "enemy.h"
#include "fade.h"
#include "file.h"
#include "fileload.h"
#include "gfxmgr.h"
#include "itex.h"
#include "mathutil.h"
#include "particle.h"
#include "plattack.h"
#include "plmain.h"
#include "plpick.h"
#include "plshot.h"
#include "plsound.h"
#include "plutil.h"
#include "polygon.h"
#include "ride.h"
#include "tmanager.h"
#include "vram.h"

/* The player's shots and sub-weapons (overlay 9). Each is a task under the player's with its own
 * handler: the koban coin ("koban"), the shuriken ("shuriken"), the bomb ("サ爆弾") and the
 * charged bomb ("ため爆"), the bazooka ("bazooka") and its fragments, the dud bazooka ("dbaz"),
 * the kunai ("kunai"), the homing bazooka ("hbazoo") and shuriken ("hshuri"), the lock-on
 * ("btargM") and its markers ("btarge"), and the cloud ("onara") that bombs leave. */




/* The lock-on ("btargM", character 3's): up to three targets, nearest first. */
typedef struct PlLockOn {
    /* 0x00 */ Player *p;
    /* 0x04 */ PlTarget targets[3];
    /* 0x1C */ s32 mode; /* 0 aiming, 1 fired, 2 ending */
    /* 0x20 */ s32 timer; /* ending: the steps left */
    /* 0x24 */ s32 hold; /* aiming: the steps left */
} PlLockOn;

/* A lock-on marker ("btarge"), on its target. */
typedef struct PlMark {
    /* 0x00 */ u8 pad00[0x10];
    /* 0x10 */ AnimeFrame *model;
    /* 0x14 */ s32 unused14;
    /* 0x18 */ s32 index; /* the player's */
    /* 0x1C */ Obj *target;
    /* 0x20 */ s32 mode; /* 0 closing in, 1 locked, 2 fired, 3 fading */
    /* 0x24 */ s32 t;
    /* 0x28 */ s32 wait; /* locked: the steps before it fades */
    /* 0x2C */ s32 alpha;
} PlMark;

/* The kunai: its model's texture runs an animation. */
typedef struct PlKunai {
    /* 0x00 */ Vec3f vel;
    /* 0x0C */ f32 spin;
    /* 0x10 */ AnimeFrame *model;
    /* 0x14 */ ACollFrame *box;
    /* 0x18 */ s32 index;
    /* 0x1C */ s32 unused1C;
    /* 0x20 */ s32 unused20;
    /* 0x24 */ Anim anim;
} PlKunai;

/* The homing shots ("hbazoo", "hshuri"). */
typedef struct PlHoming {
    /* 0x00 */ Vec3f vel;
    /* 0x0C */ f32 spin;
    /* 0x10 */ AnimeFrame *model;
    /* 0x14 */ ACollFrame *box;
    /* 0x18 */ s32 index;
    /* 0x1C */ Obj *target;
    /* 0x20 */ Vec3f dir; /* toward the target, added to the speed each step */
    /* 0x2C */ s32 mode; /* 0 homing, 1 straight on (the shuriken looks for a target), 2 done */
    /* 0x30 */ s32 t;
} PlHoming;

/* The bazooka's shot ("bazooka", "dbaz"). */
typedef struct PlBazooka {
    /* 0x00 */ Vec3f vel;
    /* 0x0C */ f32 spin;
    /* 0x10 */ AnimeFrame *model;
    /* 0x14 */ ACollFrame *box;
    /* 0x18 */ s32 index;
    /* 0x1C */ s32 unused1C;
    /* 0x20 */ s32 t; /* steps; a puff of smoke every other one */
    /* 0x24 */ s32 kind; /* 1 bursts into eight, 2 is one of the eight */
    /* 0x28 */ s32 fromSwing;
} PlBazooka;

/* The cloud a bomb leaves ("onara"), and its kind's colour: 0xA red, 0xE blue, 0xD grey. */
typedef struct PlOnara {
    /* 0x00 */ PolFrame *model;
    /* 0x04 */ Anim anim;
    /* 0x14 */ s32 alpha;
    /* 0x18 */ ACollFrame *box;
    /* 0x1C */ s32 kind;
    /* 0x20 */ Vec3f vel;
} PlOnara;

#define FX(obj) ((void *) ((u32) (obj) + sizeof(Obj)))
#define SHOT(obj) ((PlShot *) FX(obj))
#define ONARA(obj) ((PlOnara *) FX(obj))

/* Character 2's lock-on: 0x284 is the lock-on's task, 0x288-0x290 the three targets. */
#define PL_TARGETS(p) ((Obj **) &(p)->cloud)

extern Frame *gFrmModelLayer; /* the frame the shots go under */
extern Frame *gFrmLayer8; /* and the effects */
extern Frame *gFrmEffectLayer;
extern s16 gFrmKindPol; /* "pol" */
extern s16 gFrmKindAnime; /* "anime" */
extern ACollFrame *pl_atk_root_frame; /* the player attacks' boxes */
extern u8 plShurikenAnim[]; /* the shuriken's animation */
extern u8 plBazookaMarkerAnim[]; /* the lock-on marker's ("btarge"), in the bazooka's file 0x1AF: the same address */
extern u8 plKobanAnim[]; /* the koban's */
extern u8 plBombAnim[]; /* the bomb's */
extern u8 plBazookaAnim[]; /* the bazooka's */
extern u8 plKobanGlowAnim[]; /* the koban's glow */
extern u8 plBazookaFragmentAnim[]; /* the bazooka's fragment */
extern u8 plKobanTrailAnim[]; /* the koban's trail */
extern u8 plKunaiAnim[]; /* the kunai's */
extern Obj *gEffectTask; /* the effect manager */
extern Gfx *gGfxDlHead; /* the display-list buffer */
extern s32 gGameFrameCount; /* the frame counter */
extern Vec3f gPlAttackHandPos[4][6]; /* per character and form: the charged bomb's hand */
extern AnimFrame gItexPuffAnim[]; /* the cloud's animation */
extern u8 plOnaraCloudDl[]; /* the cloud's list (file 0x123) */
extern Gfx gDrawSetupCldSurfDl[]; /* the markers' setup list */


/* plshot.c's .data (0x801A8930-0x801A8980), in address order: IDO lays .data out by each
 * variable's first declaration (the charged bomb's hands above are plattack.c's). */
s32 (*gPlShotProcTable[4])(Obj *, s16, void *) = {
    koban, shuriken, saBakudan, bazooka,
}; /* per character: its shot */
u16 gPlShotPrio[4] = { 2, 1, 1, 1 }; /* and the shot's priority */
s32 (*gPlSubWeaponFireTable[4])(Player *p, Vec3f *pos, Vec3f *vel) = {
    (s32 (*)(Player *, Vec3f *, Vec3f *)) plKobanChargedFire,
    plOnaraBlow,
    (s32 (*)(Player *, Vec3f *, Vec3f *)) plTameBakuFire,
    plHbazooFire,
}; /* its sub-weapon */
s32 gPlBombCloudKind[2] = { 10, 14 }; /* by charge: the cloud a bomb leaves */
u16 gPlBombCloudSound[2] = { 0x02C5, 0x0152 }; /* and its sound */
u16 gPlBombSetSound[2] = { 0x02C3, 0x02C4 }; /* the sound of a bomb set down */
f32 gPlBombCloudSize[2] = { 0.4f, 0.6f }; /* the cloud's size */
AnimFrame gPlKunaiFrames[] = { { 0x03001F60, 1 }, { 0, 0 } }; /* the kunai's animation */



/* The player's shot, by character. */
void plShotFire(Player *p, Vec3f *pos, Vec3f *vel) {
    PlShotArg arg;

    arg.p = p;
    arg.pos = pos;
    arg.vel = vel;
    arg.param = 0;
    arg.mode = 0;
    fadeCreateRetry(gGameWork.players[p->index], gPlShotProcTable[p->chara],
                           gPlShotPrio[p->chara], 0, &arg);
}

/* The player's sub-weapon, by character. */
void plSubWeaponFire(Player *p, Vec3f *pos, Vec3f *vel) {
    gPlSubWeaponFireTable[p->chara](p, pos, vel);
}

/* The charged koban. */
void plKobanChargedFire(Player *p, Vec3f *pos, Vec3f *vel) {
    PlShotArg arg;

    arg.p = p;
    arg.pos = pos;
    arg.vel = vel;
    arg.param = 1;
    arg.mode = 0;
    plSound(p, 0x134);
    fadeCreateRetry(gGameWork.players[p->index], koban, 2, 0, &arg);
}

/* The grey cloud, blown ahead of the player. */
s32 plOnaraBlow(Player *p, Vec3f *pos, Vec3f *vel) {
    f32 heading;
    f32 x;
    f32 y;
    f32 z;
    Obj *o;

    plSound(p, 0x218);
    plSound(p, 0x564);
    heading = vramCourseHeading(p->zone, p->place);
    z = mathCos(p->targetFacing) * p->width;
    y = 0.0f;
    x = 0.0f;
    mathVecRotateY(heading, &x, &y, &z);
    o = plOnaraCreate(p->model->pos[0] + x, p->model->pos[1] + 10.0f, p->model->pos[2] + z, 0.62f,
                               0xD);
    if (o != NULL) {
        plOnaraSetVel(o, x * 0.1f, 0.0f, z * 0.1f);
    }
    return 0;
}

/* The charged bomb. */
void plTameBakuFire(Player *p, Vec3f *pos, Vec3f *vel) {
    if (p->action != 0x80) {
        plSound(p, 0x21D);
    }
    fadeCreateRetry(gGameWork.players[p->index], tameBaku, 1, 0, p);
}

/* The homing bazookas: one at each locked-on target, or straight on for the ones not locked. */
s32 plHbazooFire(Player *p, Vec3f *pos, Vec3f *vel) {
    PlShotArg arg;
    s32 i;

    arg.p = p;
    arg.pos = pos;
    arg.vel = vel;
    contPakRumbleStart(p->index, 0xF5000000, 10);
    if (p->chain != NULL) {
        plSendTagMsg(p->chain, 0xC8AFBCCD, 0, 0);
    }
    for (i = 0; i < 3; i++) {
        if (PL_TARGETS(p)[i] != NULL) {
            arg.param = (s32) PL_TARGETS(p)[i];
            arg.mode = 0;
        } else {
            arg.mode = 1;
            arg.param = 0;
        }
        fadeCreateRetry(gGameWork.players[p->index], hbazoo, 1, 0, &arg);
        PL_TARGETS(p)[i] = NULL;
    }
    plSound(p, 0x566);
    return 0;
}

/* The koban coin ("koban"): from a swing (0x81, 0x82, 0x86, 0x87, 0x23) it keeps the player's
 * speed and goes up at 5, flat. Plain, it sparkles every other step; charged, it glows and
 * leaves a trail. */
s32 koban(Obj *obj, s16 msg, void *arg) {
    PlKoban *w = FX(obj);
    Player *p;
    s32 i;
    s32 pad;
    Vec3f at;
    f32 s;
    f32 c;
    f32 x;
    f32 y;
    f32 z;

    switch (msg) {
        case 2:
            w->arg = *(PlShotArg *) arg;
            obj->name = "koban";
            p = ((PlShotArg *) arg)->p;
            if ((w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime)) == NULL) {
                return -1;
            }
            w->index = p->index;
            w->model->anim = plKobanAnim;
            w->model->segs[1] = fileRamAddr(0x1A0);
            plShotPlace(p, (PlShot *) w, ((PlShotArg *) arg)->pos, ((PlShotArg *) arg)->vel);
            plFrameScaleSet(w->model, 0.15f);
            w->spin = 0.2f;
            if (p->action == 0x81 || p->action == 0x82 || p->action == 0x86 || p->action == 0x87 ||
                p->action == 0x23) {
                w->vel.x = PL_HIST(p)[0][1];
                w->vel.y = 5.0f;
                w->vel.z = PL_HIST(p)[2][1];
                w->model->rot[0] = -1.5707963f;
            }
            if ((w->box = acollAtkAddYZ((Frame *) pl_atk_root_frame, (Frame *) w->model, 0x1E, 0, 0, 0)) ==
                NULL) {
                return -1;
            }
            plAttackBoxSet(p, w->box, ((PlShotArg *) arg)->param == 0 ? 7 : 8);
            mathMtxLookDir(w->mtx, w->vel.x, w->vel.y, w->vel.z);
            if (((PlShotArg *) arg)->param) {
                if ((w->glow = (AnimeFrame *) frmCreate(gFrmLayer8, gFrmKindAnime)) != NULL) {
                    w->glow->anim = plKobanGlowAnim;
                    w->glow->segs[1] = fileRamAddr(0x1A0);
                    plCopyAnimeFrame(w->glow, w->model);
                }
                for (i = 0; i < 5; i++) { w->trail[i] = NULL; }
                plSound(p, 0x565);
            } else {
                plSound(p, 0x20C);
            }
            break;
        case 0:
            if (!plIsInPlay(w->index)) {
                return -1;
            }
            plShotSpin((PlShot *) w);
            if (w->arg.param == 0) {
                if (gGameFrameCount & 1) {
                    s = mathSinCos((gGameFrameCount & 0x3FF) * 96, &c);
                    guMtxXFMF(w->mtx, c * 3.0f, s * 3.0f, 0.0f, &x, &y, &z);
                    effectStarCreate(w->model->pos[0] + x, w->model->pos[1] + y, w->model->pos[2] + z);
                }
            } else if (w->arg.param == 1) {
                plKobanTrail(w);
            }
            plFramePosAdd(w->model, &w->vel);
            if (w->glow != NULL) {
                plCopyAnimeFrame(w->glow, w->model);
                w->glow->rot[0] += 1.5707963f;
                if (w->arg.mode & 1) {
                    w->glow->rot[0] -= 1.5707963f;
                } else if (w->arg.mode & 2) {
                    w->glow->rot[0] -= 3.1415927f;
                }
            }
            if (acollAtkWasHit(w->box) == 1 || plShotHitGround((PlShot *) w, (f32 *) &at)) {
                return -1;
            }
            if (plShotDistFromPlayer(gGameWork.players[w->index], (PlShot *) w) > 110.0f) {
                return -1;
            }
            break;
        case 3:
            if (w->glow != NULL) {
                w->glow->dl = gGfxDlHead;
                gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0,
                                  ENVIRONMENT, 0, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0,
                                  0, ENVIRONMENT, 0);
                gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 254, 113, 0);
                gDPSetEnvColor(gGfxDlHead++, 226, 23, 2, 150);
                gSPEndDisplayList(gGfxDlHead++);
                for (i = 0; i < 5; i++) {
                    if (w->trail[i] != NULL) {
                        w->trail[i]->dl = gGfxDlHead;
                        gDPSetCombineLERP(gGfxDlHead++, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0,
                                          ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0);
                        gDPSetEnvColor(gGfxDlHead++, 255, 242 - w->alpha[i] * 3 / 10, 34, w->alpha[i]);
                        gSPEndDisplayList(gGfxDlHead++);
                    }
                }
            }
            break;
        case 1:
            frmDestroy((Frame *) w->box);
            frmDestroy((Frame *) w->model);
            if (w->arg.param == 1) {
                frmDestroy((Frame *) w->glow);
                for (i = 0; i < 5; i++) {
                    if (w->trail[i] != NULL) {
                        frmDestroy((Frame *) w->trail[i]);
                    }
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The charged koban's trail: every other step a copy of the coin, turned, that fades (15 a
 * step), shrinks, and drifts on at half the coin's speed. */
void plKobanTrail(PlKoban *w) {
    s32 i;
    AnimeFrame *f;

    if (gGameFrameCount & 1) {
        for (i = 0; i < 5; i++) {
            if (w->trail[i] == NULL) {
                if ((w->trail[i] = (AnimeFrame *) frmCreate(gFrmLayer8, gFrmKindAnime)) != NULL) {
                    w->trail[i]->anim = plKobanTrailAnim;
                    w->trail[i]->segs[1] = fileRamAddr(0x1A0);
                    plCopyAnimeFrame(w->trail[i], w->model);
                    w->trail[i]->rot[0] += 1.5707963f;
                    if (w->arg.mode & 1) {
                        w->trail[i]->rot[0] -= 1.5707963f;
                        w->trail[i]->rot[1] -= 0.31415927f;
                    } else if (w->arg.mode & 2) {
                        w->trail[i]->rot[1] -= 0.31415927f;
                    } else {
                        w->trail[i]->rot[1] += 0.31415927f;
                    }
                    w->alpha[i] = 200;
                }
                break;
            }
        }
    }
    for (i = 0; i < 5; i++) {
        if (w->trail[i] != NULL) {
            w->alpha[i] -= 15;
            if (w->alpha[i] <= 0) {
                frmDestroy((Frame *) w->trail[i]);
                w->trail[i] = NULL;
            } else {
                f = w->trail[i];
                plFrameScaleSet(f, 0.25f - (255 - w->alpha[i]) * 0.0005f);
                f->pos[0] += w->vel.x * 0.5f;
                f->pos[1] += w->vel.y * 0.5f;
                f->pos[2] += w->vel.z * 0.5f;
            }
        }
    }
}

/* The shuriken ("shuriken"): spins as it flies; on a swing (actions 0x81, 0x82) it keeps the
 * player's speed and goes up at 5. */
s32 shuriken(Obj *obj, s16 msg, void *arg) {
    PlShot *w = FX(obj);
    s32 pad;
    Player *p;
    s32 pad2;
    Vec3f at;

    switch (msg) {
        case 2:
            obj->name = "shuriken";
            p = ((PlShotArg *) arg)->p;
            if ((w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime)) == NULL) {
                return -1;
            }
            w->index = p->index;
            w->model->flags |= 0x20;
            w->model->anim = plShurikenAnim;
            w->model->segs[1] = fileRamAddr(0x1A5);
            plShotPlace(p, w, ((PlShotArg *) arg)->pos, ((PlShotArg *) arg)->vel);
            if (p->action == 0x81 || p->action == 0x82) {
                w->vel.x = PL_HIST(p)[0][1];
                w->vel.y = 5.0f;
                w->vel.z = PL_HIST(p)[2][1];
                mathVecRotateY(vramCourseHeading(p->zone, p->place) - 1.5707963f, &w->vel.x,
                                       &w->vel.y, &w->vel.z);
            }
            if ((w->box = acollAtkAddYZ((Frame *) pl_atk_root_frame, (Frame *) w->model, 0x32, 0, 0, 0)) ==
                NULL) {
                return -1;
            }
            plAttackBoxSet(p, w->box, 9);
            plSound(p, 0x210);
            break;
        case 0:
            if (!plIsInPlay(w->index)) {
                return -1;
            }
            plShotSpin(w);
            w->model->rot[0] += 0.3926991f;
            plFramePosAdd(w->model, &w->vel);
            if (acollAtkWasHit(w->box) == 1 || plShotHitGround(w, (f32 *) &at)) {
                return -1;
            }
            if (plShotDistFromPlayer(gGameWork.players[w->index], w) > 110.0f) {
                return -1;
            }
            break;
        case 1:
            frmDestroy((Frame *) w->box);
            frmDestroy((Frame *) w->model);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The bomb ("サ爆弾"): thrown (from a swing, 0x23/0x81/0x82: with the player's speed, up at 9;
 * 0x22/0x7E/0x7F: up at 6 and slower), or set down (0x80) to go off in 45 steps.
 * It matches only with the handler table gPlShotProcTable defined in this file: as1 fills
 * the message-1 branch's delay slot from the fall-through (the name's lui) when the function
 * has a jump table AND its address is taken in the same file. */
s32 saBakudan(Obj *obj, s16 msg, void *arg) {
    PlBomb *w = FX(obj);
    s32 pad;
    Player *p;
    s32 pad2;
    Vec3f pos;
    Vec3f push;

    switch (msg) {
        case 2:
            obj->name = "サ爆弾";
            p = ((PlShotArg *) arg)->p;
            if ((w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime)) == NULL) {
                return -1;
            }
            w->index = p->index;
            w->mode = 0;
            w->model->anim = plBombAnim;
            w->model->segs[1] = fileRamAddr(0x1AA);
            w->model->flags |= 0x20;
            plShotPlace(p, (PlShot *) w, ((PlShotArg *) arg)->pos, ((PlShotArg *) arg)->vel);
            w->size = 1;
            switch (p->action) {
                case 0x23:
                case 0x81:
                case 0x82:
                    w->vel.x = PL_HIST(p)[0][1];
                    w->vel.y = 9.0f;
                    w->vel.z = PL_HIST(p)[2][1];
                    mathVecRotateY(vramCourseHeading(p->zone, p->place) - 1.5707963f, &w->vel.x,
                                           &w->vel.y, &w->vel.z);
                    w->mode = 1;
                    break;
                case 0x80:
                    w->model->pos[0] += w->vel.x;
                    w->model->pos[2] += w->vel.z;
                    w->mode = 2;
                    w->timer = 45;
                    w->landed = 0;
                    w->vel.x = 0.0f;
                    w->vel.y = 0.0f;
                    w->vel.z = 0.0f;
                    break;
                case 0x22:
                case 0x7E:
                case 0x7F:
                    w->vel.x *= 0.75f;
                    w->vel.y = 6.0f;
                    w->vel.z *= 0.75f;
                    break;
            }
            if ((w->box = acollAtkAddYZ((Frame *) pl_atk_root_frame, (Frame *) w->model, w->size * 30, 0, 0,
                                                 0)) == NULL) {
                return -1;
            }
            plAttackBoxSet(p, w->box, 0xB);
            pos.x = w->model->pos[0];
            pos.y = w->model->pos[1];
            pos.z = w->model->pos[2];
            w->wasInWater = w->inWater = rideTree2PointIn(&pos, &push);
            break;
        case 0:
            if (plBombStep((PlBomb *) FX(obj), 0)) {
                return -1;
            }
            break;
        case 1:
            frmDestroy((Frame *) w->box);
            frmDestroy((Frame *) w->model);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A bomb's step (charged: 0 or 1). Thrown, it falls (1.07 a step, at most 8) and goes off on
 * the ground or anything it meets; from a swing, once it starts to fall. Set down, it settles
 * (with a sound) and goes off when its time runs out, or at once if it landed fast. Going off
 * makes a cloud of the bomb's size and kind with its sound; into water, a splash and it slows. */
s32 plBombStep(PlBomb *w, s32 charged) {
    Vec3f from;
    Vec3f at;
    f32 t;
    s32 ground;
    s32 hit;
    Vec3f pos;
    Vec3f push;

    if (!plIsInPlay(w->index)) {
        return -1;
    }
    plShotSpin((PlShot *) w);
    switch (w->mode) {
        case 1:
            if (w->vel.y < 0.0f) {
                plOnaraCreate(w->model->pos[0], w->model->pos[1], w->model->pos[2],
                                       gPlBombCloudSize[charged], gPlBombCloudKind[charged]);
                plSoundAtModel(w->model, gPlBombCloudSound[charged], w->index);
                return -1;
            }
            /* fallthrough */
        case 0:
            w->vel.y += -1.0714285f;
            if (w->vel.y < -8.0f) {
                w->vel.y = -8.0f;
            }
            if (w->inWater) {
                w->vel.x *= 0.3f;
                w->vel.y *= 0.725f;
            }
            from.x = w->model->pos[0];
            from.y = w->model->pos[1];
            from.z = w->model->pos[2];
            hit = rideMoveOnto(&from, &w->vel, &at, 0);
            if (hit) {
                at.x += from.x;
                ground = 0;
                at.y += from.y;
                at.z += from.z;
            } else {
                ground = vramRayGround(&from, &w->vel, &at, &t);
            }
            if (hit || ground) {
                plOnaraCreate(at.x, at.y, at.z, gPlBombCloudSize[charged],
                                       gPlBombCloudKind[charged]);
                plSoundAtModel(w->model, gPlBombCloudSound[charged], w->index);
                return -1;
            }
            break;
        case 2:
            if (--w->timer == 0) {
                plOnaraCreate(w->model->pos[0], w->model->pos[1], w->model->pos[2],
                                       gPlBombCloudSize[charged], gPlBombCloudKind[charged]);
                plSoundAtModel(w->model, gPlBombCloudSound[charged], w->index);
                return -1;
            }
            if (w->landed == 0) {
                w->vel.y += -1.0714285f;
                if (w->vel.y < -8.0f) {
                    w->vel.y = -8.0f;
                }
                if (w->inWater) {
                    w->vel.x *= 0.3f;
                    w->vel.z *= 0.3f;
                    w->vel.y *= 0.725f;
                }
                from.x = w->model->pos[0];
                from.y = w->model->pos[1];
                from.z = w->model->pos[2];
                w->vel.y -= w->size * 2.0f;
                if (rideMoveOnto(&from, &w->vel, &at, 0)) {
                    at.x += from.x;
                    at.y += from.y;
                    at.z += from.z;
                    plOnaraCreate(at.x, at.y, at.z, gPlBombCloudSize[charged],
                                           gPlBombCloudKind[charged]);
                    plSoundAtModel(w->model, gPlBombCloudSound[charged], w->index);
                    return -1;
                }
                if (vramRayGround(&from, &w->vel, &at, &t)) {
                    w->model->pos[0] = at.x;
                    w->model->pos[1] = w->size * 2.0f + at.y;
                    w->model->pos[2] = at.z;
                    if (fabsf(w->vel.y) > 7.5f) {
                        plOnaraCreate(at.x, at.y, at.z, gPlBombCloudSize[charged],
                                               gPlBombCloudKind[charged]);
                        plSoundAtModel(w->model, gPlBombCloudSound[charged], w->index);
                        return -1;
                    }
                    w->landed = 1;
                    w->vel.x = 0.0f;
                    w->vel.y = 0.0f;
                    w->vel.z = 0.0f;
                    plSoundAtModel(w->model, gPlBombSetSound[charged], 0);
                } else {
                    w->vel.y += w->size * 2.0f;
                }
            }
            break;
    }
    if (acollAtkWasHit(w->box) == 1) {
        plOnaraCreate(w->model->pos[0], w->model->pos[1], w->model->pos[2], 0.4f,
                               gPlBombCloudKind[charged]);
        plSoundAtModel(w->model, gPlBombCloudSound[charged], w->index);
        return -1;
    }
    plFramePosAdd(w->model, &w->vel);
    if (plShotDistFromPlayer(gGameWork.players[w->index], (PlShot *) w) > 110.0f) {
        return -1;
    }
    pos.x = w->model->pos[0];
    pos.y = w->model->pos[1];
    pos.z = w->model->pos[2];
    w->wasInWater = w->inWater;
    if ((w->inWater = rideTree2PointIn(&pos, &push)) && !w->wasInWater) {
        if (charged == 0) {
            effectSibukiCreateWhite(w->model->pos[0], w->model->pos[1] + push.y, w->model->pos[2], 0.002f,
                                   0.005f, 0.008f, 0.003f);
        } else {
            effectSibukiCreateWhite(w->model->pos[0], w->model->pos[1] + push.y, w->model->pos[2], 0.002f,
                                   0.0075f, 0.008f, 0.006f);
        }
        w->vel.x *= 0.5f;
        w->vel.z *= 0.5f;
        w->vel.y *= 0.75f;
    }
    return 0;
}

/* A random number up to n, in hundredths. */
f32 plRandHundredths(s32 n) {
    return fadeRand(n) * 0.01f;
}

/* The bazooka ("bazooka"): smoking every other step; kind 1 bursts into eight fragments (kind
 * 2: thrown about at random from where it burst, falling) when its box hits. */
s32 bazooka(Obj *obj, s16 msg, void *arg) {
    PlBazooka *w = FX(obj);
    s32 pad;
    Player *p;
    AnimeFrame *m;
    s32 pad2[2];
    AnimeFrame *f;
    Vec3f *start;
    Vec3f at;
    PlShotArg frag;
    Vec3f pos;
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "bazooka";
            p = ((PlShotArg *) arg)->p;
            if ((w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime)) == NULL) {
                return -1;
            }
            w->index = p->index;
            m = w->model;
            w->kind = ((PlShotArg *) arg)->param;
            if (w->kind == 2) {
                m->anim = plBazookaFragmentAnim;
            } else {
                m->anim = plBazookaAnim;
            }
            m->segs[1] = fileRamAddr(0x1AF);
            m->flags |= 0x20;
            if ((w->box = acollAtkAddYZ((Frame *) pl_atk_root_frame, (Frame *) m, 0x32, 0, 0, 0)) == NULL) {
                return -1;
            }
            if (w->kind != 2) {
                w->fromSwing = 0;
                plShotPlace(p, (PlShot *) w, ((PlShotArg *) arg)->pos, ((PlShotArg *) arg)->vel);
                if (p->action == 0x81 || p->action == 0x82 || p->action == 0xC0) {
                    w->fromSwing = 1;
                    w->vel.x = PL_HIST(p)[0][1];
                    w->vel.y = 5.0f;
                    w->vel.z = PL_HIST(p)[2][1];
                    mathVecRotateY(vramCourseHeading(p->zone, p->place) - 1.5707963f, &w->vel.x,
                                           &w->vel.y, &w->vel.z);
                }
                plAttackBoxSet(p, w->box, 0xC);
                plSound(p, 0x11E);
            } else {
                f = w->model;
                start = ((PlShotArg *) arg)->pos;
                plShotPlace(p, (PlShot *) w, start, ((PlShotArg *) arg)->vel);
                f->pos[0] = start->x;
                f->pos[1] = start->y;
                f->pos[2] = start->z;
                plFrameScaleSet(w->model, 0.08f);
                if (((PlShotArg *) arg)->mode == 0) {
                    w->vel.x = plRandHundredths(400) + ((PlShotArg *) arg)->vel->x - 2.0f;
                    w->vel.y = plRandHundredths(300) + 4.0f;
                    w->vel.z = plRandHundredths(400) + ((PlShotArg *) arg)->vel->z - 2.0f;
                    plVecScaleSelf(&w->vel, 0.67f);
                } else {
                    w->vel.x = plRandHundredths(250) + ((PlShotArg *) arg)->vel->x - 1.25f;
                    w->vel.y = plRandHundredths(250) + 4.0f;
                    w->vel.z = plRandHundredths(250) + ((PlShotArg *) arg)->vel->z - 1.25f;
                }
                plAttackBoxSet(p, w->box, 0xC);
            }
            break;
        case 0:
            if (!plIsInPlay(w->index)) {
                return -1;
            }
            plShotSpin((PlShot *) w);
            if (w->kind == 2) {
                w->vel.y += -0.53571427f;
                if (w->vel.y < -8.0f) {
                    w->vel.y = -8.0f;
                }
            }
            plFramePosAdd(w->model, &w->vel);
            if (acollAtkWasHit(w->box) == 1) {
                if (w->kind == 1 && plIsInPlay(w->index)) {
                    frag.p = PL(gGameWork.players[w->index]);
                    pos.x = w->model->pos[0];
                    pos.y = w->model->pos[1];
                    pos.z = w->model->pos[2];
                    frag.pos = &pos;
                    frag.vel = &w->vel;
                    frag.param = 2;
                    frag.mode = w->fromSwing;
                    for (i = 0; i < 8; i++) {
                        fadeCreateRetry(gGameWork.players[w->index], bazooka, 1, 0,
                                               &frag);
                    }
                }
                return -1;
            }
            if (plShotHitGround((PlShot *) w, (f32 *) &at)) {
                itexPuffYellow(w->model->pos[0], w->model->pos[1], w->model->pos[2], 0.04f);
                return -1;
            }
            if (plShotDistFromPlayer(gGameWork.players[w->index], (PlShot *) w) > 110.0f) {
                return -1;
            }
            if (w->kind != 2 && !(w->t++ & 1)) {
                itexPuffGrey(w->model->pos[0], w->model->pos[1], w->model->pos[2], 0.03f);
            }
            break;
        case 1:
            frmDestroy((Frame *) w->box);
            frmDestroy((Frame *) w->model);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The cloud ("onara"): an animated puff that grows and fades (11 a step). Grey (0xD) drifts
 * by its speed, slowing to 0.9 each step; red and blue rise, and are hidden in the first
 * view; its box hits with its kind. */
s32 onara(Obj *obj, s16 msg, void *arg) {
    PlOnara *w = FX(obj);

    switch (msg) {
        case 2:
            w->kind = (s32) arg;
            obj->name = "onara";
            if ((w->model = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                return -1;
            }
            w->model->dl2 = (Gfx *) plOnaraCloudDl;
            floadGetSetRam((void **) w->model->segs, 0x123);
            fadeAnimStart(&w->anim, gItexPuffAnim, 0, &w->model->segs[3],
                                   floadGetSetFile(0x123, 0));
            w->model->flags |= 3;
            w->model->rot[2] = fadeRand(360) * 3.1415927f / 180.0f;
            w->alpha = 255;
            if (w->kind == 0xA || w->kind == 0xE) {
                w->model->f.flags &= ~2;
                w->box = acollAtkAddYZ((Frame *) pl_atk_root_frame, (Frame *) w->model, 0x1D, 0, 0, 0);
            } else {
                w->box = acollAtkAddYZ((Frame *) pl_atk_root_frame, (Frame *) w->model, 0x21, 0, 0, 0);
            }
            if (w->box == NULL) {
                return -1;
            }
            w->vel.x = w->vel.y = w->vel.z = 0.0f;
            w->box->kind = (s32) arg;
            w->box->power = (s32) obj;
            break;
        case 1:
            frmDestroy((Frame *) w->model);
            frmDestroy((Frame *) w->box);
            break;
        case 0:
            if (fadeAnimStep(&w->anim)) {
                return -1;
            }
            if (w->kind == 0xD) {
                w->model->scale[0] *= 1.025f;
                w->model->scale[1] *= 1.025f;
                w->model->scale[2] *= 1.025f;
                w->model->pos[0] += w->vel.x;
                w->model->pos[1] += w->vel.y;
                w->model->pos[2] += w->vel.z;
                w->vel.x *= 0.9f;
                w->vel.y *= 0.9f;
                w->vel.z *= 0.9f;
            } else {
                w->model->scale[0] *= 1.02f;
                w->model->scale[1] *= 1.02f;
                w->model->scale[2] *= 1.02f;
                w->model->pos[1] += 0.2f;
            }
            w->alpha -= 11;
            if (w->alpha < 0) {
                w->alpha = 0;
            }
            break;
        case 3:
            w->model->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0,
                              ENVIRONMENT, 0, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0,
                              0, ENVIRONMENT, 0);
            if (w->kind == 0xD) {
                gDPSetPrimColor(gGfxDlHead++, 0, 0, 50, 160, 50, 0);
                gDPSetEnvColor(gGfxDlHead++, 15, 30, 15, w->alpha);
            } else {
                gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 220, 155, 0);
                gDPSetEnvColor(gGfxDlHead++, 255, 64, 64, w->alpha);
            }
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Makes a cloud at (x, y, z); kinds 0xA and 0xE light it red and blue. */
Obj *plOnaraCreate(f32 x, f32 y, f32 z, f32 size, s32 kind) {
    Obj *o;

    o = fadeCreateRetry(gEffectTask, onara, 1, 0, (void *) kind);
    if (o != NULL) {
        ONARA(o)->model->pos[0] = x;
        ONARA(o)->model->pos[1] = y;
        ONARA(o)->model->pos[2] = z;
        ONARA(o)->model->scale[2] = size;
        ONARA(o)->model->scale[0] = ONARA(o)->model->scale[1] = ONARA(o)->model->scale[2];
        if (kind == 0xA) {
            ptclSpawnHanabiSmall(x, y, z, size, 0xFF, 0, 0);
        } else if (kind == 0xE) {
            ptclSpawnHanabiSmall(x, y, z, size, 0x32, 0x32, 0xFF);
        }
    }
    return o;
}

void plOnaraSetVel(Obj *o, f32 x, f32 y, f32 z) {
    ONARA(o)->vel.x = x;
    ONARA(o)->vel.y = y;
    ONARA(o)->vel.z = z;
}

/* The charged bomb ("ため爆"): twice the size, from the player's hand (0x86/0x87 from a swing;
 * 0x85 set down; 0x83/0x84 up at 6 and slower). */
s32 tameBaku(Obj *obj, s16 msg, void *arg) {
    PlBomb *w = FX(obj);
    s32 pad[2];
    Vec3f pos;
    Vec3f push;

    switch (msg) {
        case 2:
            obj->name = "ため爆";
            if ((w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime)) == NULL) {
                return -1;
            }
            w->index = ((Player *) arg)->index;
            w->mode = 0;
            w->model->anim = plBombAnim;
            w->model->segs[1] = fileRamAddr(0x1AA);
            w->model->flags |= 0x20;
            plShotPlaceAtHand(arg, (PlShot *) w, 6.0f);
            w->size = 2;
            plFrameScaleSet(w->model, 0.2f);
            switch (((Player *) arg)->action) {
                case 0x86:
                case 0x87:
                    w->vel.x = PL_HIST((Player *) arg)[0][1];
                    w->vel.y = 9.0f;
                    w->vel.z = PL_HIST((Player *) arg)[2][1];
                    mathVecRotateY(vramCourseHeading(((Player *) arg)->zone, ((Player *) arg)->place) -
                                               1.5707963f,
                                           &w->vel.x, &w->vel.y, &w->vel.z);
                    w->mode = 1;
                    break;
                case 0x85:
                    w->model->pos[0] += w->vel.x;
                    w->model->pos[2] += w->vel.z;
                    w->mode = 2;
                    w->timer = 45;
                    w->landed = 0;
                    w->vel.x = 0.0f;
                    w->vel.y = 0.0f;
                    w->vel.z = 0.0f;
                    break;
                case 0x83:
                case 0x84:
                    w->vel.x *= 0.75f;
                    w->vel.y = 6.0f;
                    w->vel.z *= 0.75f;
                    break;
            }
            if ((w->box = acollAtkAddYZ((Frame *) pl_atk_root_frame, (Frame *) w->model, w->size * 15, 0, 0,
                                                 0)) == NULL) {
                return -1;
            }
            plAttackBoxSet(arg, w->box, 0xF);
            pos.x = w->model->pos[0];
            pos.y = w->model->pos[1];
            pos.z = w->model->pos[2];
            w->wasInWater = w->inWater = rideTree2PointIn(&pos, &push);
            break;
        case 0:
            if (plBombStep((PlBomb *) FX(obj), 1)) {
                return -1;
            }
            break;
        case 1:
            frmDestroy((Frame *) w->box);
            frmDestroy((Frame *) w->model);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The dud bazooka. */
void plDbazFire(Player *p) {
    fadeCreateRetry(gGameWork.players[p->index], dbaz, 1, 0, p);
}

/* The dud bazooka ("dbaz"): straight on, smoking; it bursts on the ground and fizzles out on
 * anything else or 110 from its player. */
s32 dbaz(Obj *obj, s16 msg, void *arg) {
    PlBazooka *w = FX(obj);
    s32 pad[2];
    Vec3f at;

    switch (msg) {
        case 2:
            obj->name = "dbaz";
            if ((w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime)) == NULL) {
                return -1;
            }
            w->index = ((Player *) arg)->index;
            w->model->anim = plBazookaAnim;
            w->model->segs[1] = fileRamAddr(0x1AF);
            plShotPlaceLean(arg, (PlShot *) w, 4.0f);
            if ((w->box = acollAtkAddYZ(pl_atk_root_frame, (Frame *) w->model, 0x32, 0, 0, 0)) == NULL) {
                return -1;
            }
            plAttackBoxSet(arg, w->box, 0xC);
            break;
        case 0:
            if (!plIsInPlay(w->index)) {
                return -1;
            }
            plFramePosAdd(w->model, &w->vel);
            if (acollAtkWasHit(w->box) == 1) {
                return -1;
            }
            if (plShotHitGround((PlShot *) w, (f32 *) &at)) {
                itexPuffYellow(w->model->pos[0], w->model->pos[1], w->model->pos[2], 0.04f);
                return -1;
            }
            if (plShotDistFromPlayer(gGameWork.players[w->index], (PlShot *) w) > 110.0f) {
                return -1;
            }
            if (!(w->t++ & 1)) {
                itexPuffWhite(w->model->pos[0], w->model->pos[1], w->model->pos[2], 0.03f);
            }
            break;
        case 1:
            frmDestroy((Frame *) w->box);
            frmDestroy((Frame *) w->model);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The kunai. */
void plKunaiFire(Player *p, Vec3f *pos, Vec3f *vel) {
    PlShotArg arg;

    arg.p = p;
    arg.pos = pos;
    arg.vel = vel;
    arg.param = 0;
    arg.mode = 0;
    fadeCreateRetry(gGameWork.players[p->index], kunai, 1, 0, &arg);
}

/* The kunai ("kunai"): tilted when thrown so (0.09 of the size, else 0.1). */
s32 kunai(Obj *obj, s16 msg, void *arg) {
    PlKunai *w = FX(obj);
    s32 pad[2];
    Player *p;
    Vec3f at;

    switch (msg) {
        case 2:
            obj->name = "kunai";
            p = ((PlShotArg *) arg)->p;
            if ((w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime)) == NULL) {
                return -1;
            }
            w->index = p->index;
            w->model->anim = plKunaiAnim;
            w->model->segs[1] = fileRamAddr(0x1AA);
            w->model->flags |= 0x30;
            fadeAnimStart(&w->anim, gPlKunaiFrames, 0, &w->model->segs[3], 0x306);
            plShotPlace(p, (PlShot *) w, ((PlShotArg *) arg)->pos, ((PlShotArg *) arg)->vel);
            if (((PlShotArg *) arg)->param) {
                w->model->rot[0] = -((PlShotArg *) arg)->tilt;
                plFrameScaleSet(w->model, 0.09f);
            } else {
                plFrameScaleSet(w->model, 0.1f);
            }
            if ((w->box = acollAtkAddYZ((Frame *) pl_atk_root_frame, (Frame *) w->model, 0x32, 0, 0, 0)) ==
                NULL) {
                return -1;
            }
            plAttackBoxSet(p, w->box, 0x17);
            break;
        case 0:
            plFramePosAdd(w->model, &w->vel);
            if (!plIsInPlay(w->index)) {
                return -1;
            }
            if (acollAtkWasHit(w->box) == 1) {
                return -1;
            }
            if (plShotHitGround((PlShot *) w, (f32 *) &at)) {
                itexPuffYellow(w->model->pos[0], w->model->pos[1], w->model->pos[2], 0.04f);
                return -1;
            }
            if (plShotDistFromPlayer(gGameWork.players[w->index], (PlShot *) w) > 110.0f) {
                return -1;
            }
            break;
        case 1:
            frmDestroy((Frame *) w->box);
            frmDestroy((Frame *) w->model);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The lock-on ("btargM"): while aiming (held on by 0xB3CEC7A7, else three steps) it keeps the
 * nearest three targets within 70 marked ("btarge"), telling the marks to hold; fired
 * (0xC8AFBCCD 発射), it hands the targets to the player (the homing shots) and tells the
 * marks; it ends 10 steps after, or once its marks are gone. A mark out of range
 * (0xB0E8B3B0) is dropped; 0xBEC3CCC7 (消滅) ends it and its marks.
 * The new targets' loop reuses i (that keeps the outer loop's slti test and the dead
 * "move v1,zero"); the index read goes through w->p (the stored arg, t8); the pads are the
 * game's frame (0xA0). */
s32 btargM(Obj *obj, s16 msg, void *arg) {
    PlLockOn *w = FX(obj);
    Obj *o;
    s32 i;
    s32 pad;
    s32 n;
    PlTarget *have;
    Vec3f pos;
    Vec3f me;
    PlTarget new[3];
    s32 pad2[2];

    switch (msg) {
        case 2:
            obj->name = "btargM";
            w->p = (Player *) arg;
            if (arg == NULL || !plIsInPlay(w->p->index)) {
                return -1;
            }
            n = plTargetFind(w->p, w->targets, 3);
            w->mode = 0;
            for (i = 0; i < n; i++) {
                fadeCreateRetry(obj, (s32 (*)(Obj *, s16, void *)) btarge, 1, 0,
                                       w->targets[i].obj);
            }
            w->hold = 3;
            break;
        case 0:
            if (!plIsInPlay(w->p->index) || w->p->chara != 3) {
                return -1;
            }
            switch (w->mode) {
                case 0:
                    for (o = obj->child; o != NULL; o = o->next) {
                        plSendTagMsg(o, 0xB3CEC7A7, 0, 0);
                    }
                    for (i = 0; i < 3; i++) {
                        if (w->targets[i].obj != NULL && eneIsEnemy(w->targets[i].obj)) {
                            eneGetPos(w->targets[i].obj, &pos);
                            plGetModelPos(gGameWork.players[w->p->index], &me.x, &me.y, &me.z);
                            w->targets[i].dist = plVecDist(&pos, &me);
                        }
                    }
                    for (i = 0; i < 3; i++) {
                        if (w->targets[i].obj == NULL) {
                            have = w->targets;
                            n = plTargetFindNew(w->p, have, 3, new, 3);
                            for (i = 0; i < n; i++) {
                                plTargetInsert(have, 3, new[i].dist, new[i].obj);
                                fadeCreateRetry(obj, (s32 (*)(Obj *, s16, void *)) btarge, 1,
                                                       0, new[i].obj);
                            }
                            break;
                        }
                    }
                    if (w->hold-- <= 0) {
                        w->timer = 10;
                        w->mode = 2;
                    }
                    break;
                case 1:
                    if (obj->child == NULL) {
                        w->timer = 0;
                        w->mode = 2;
                    }
                    break;
                case 2:
                    if (w->timer-- <= 0) {
                        return -1;
                    }
                    break;
            }
            break;
        case 9:
            switch (((s32 *) arg)[0]) {
                case 0xB0E8B3B0:
                    plTargetRemove(w->targets, 3, (Obj *) ((s32 *) arg)[1]);
                    break;
                case 0xC8AFBCCD:
                    for (i = 0; i < 3; i++) {
                        PL_TARGETS(w->p)[i] = w->targets[i].obj;
                    }
                    for (o = obj->child; o != NULL; o = o->next) {
                        plSendTagMsg(o, 0xC8AFBCCD, 0, 0);
                    }
                    w->mode = 1;
                    break;
                case 0xB3CEC7A7:
                    w->hold = 3;
                    break;
                case 0xBEC3CCC7:
                    for (o = obj->child; o != NULL; o = o->next) {
                        plSendTagMsg(o, 0xBEC3CCC7, 0, 0);
                    }
                    return -1;
            }
            break;
        case 1:
            w->p->chain = NULL;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The targets within reach of p, nearest first (at most n); how many there are. */
s32 plTargetFind(Player *p, PlTarget *t, s32 n) {
    Obj *mgr;
    Obj *o;
    s32 count;
    f32 dist;
    s32 i;

    mgr = eneGetBase(2);
    count = 0;
    for (i = 0; i < n; i++) {
        t[i].dist = 3.4028235e38f;
        t[i].obj = NULL;
    }
    if (mgr == NULL) {
        return 0;
    }
    for (o = mgr->child; o != NULL; o = o->next) {
        if (eneIsEnemy(o) && plTargetInReach(p, o, &dist) &&
            plTargetWouldFit(t, n, dist, o)) {
            count = plTargetInsert(t, n, dist, o);
        }
    }
    return count;
}

/* The same, leaving out the ones already in have. */
s32 plTargetFindNew(Player *p, PlTarget *have, s32 nHave, PlTarget *t, s32 n) {
    Obj *mgr;
    Obj *o;
    s32 count;
    f32 dist;
    s32 i;

    mgr = eneGetBase(2);
    count = 0;
    if (mgr == NULL) {
        return 0;
    }
    o = mgr->child;
    for (i = 0; i < n; i++) {
        t[i].dist = 3.4028235e38f;
        t[i].obj = NULL;
    }
    for (; o != NULL; o = o->next) {
        if (eneIsEnemy(o) && plTargetInReach(p, o, &dist) &&
            !plTargetHas(have, nHave, o) && plTargetWouldFit(t, n, dist, o)) {
            count = plTargetInsert(t, n, dist, o);
        }
    }
    return count;
}

/* Whether obj is in the list. */
s32 plTargetHas(PlTarget *t, s32 n, Obj *obj) {
    s32 i;

    for (i = 0; i < n; i++) {
        if (obj == t[i].obj) {
            return 1;
        }
    }
    return 0;
}

/* Whether obj is within 70 of p (and how far, in dist). The differences are written inline (named,
 * they take three slots); the sum as z + (y + x), which IDO compiles to the game's x, y, z loads
 * and (x + y) + z adds. */
s32 plTargetInReach(Player *p, Obj *obj, f32 *dist) {
    Vec3f pos;

    eneGetPos(obj, &pos);
    *dist = sqrtf((p->model->pos[2] - pos.z) * (p->model->pos[2] - pos.z) +
                  ((p->model->pos[1] - pos.y) * (p->model->pos[1] - pos.y) +
                   (p->model->pos[0] - pos.x) * (p->model->pos[0] - pos.x)));
    if (*dist < 70.0f) {
        return 1;
    } else {
        return 0;
    }
}

/* Whether a target at dist would make the list. */
s32 plTargetWouldFit(PlTarget *t, s32 n, f32 dist, Obj *obj) {
    s32 i;

    for (i = n - 1; i >= 0; i--) {
        if (dist < t[i].dist || t[i].obj == NULL) {
            return 1;
        }
    }
    return 0;
}

/* Puts obj in the list at its distance; returns how many the list holds. */
s32 plTargetInsert(PlTarget *t, s32 n, f32 dist, Obj *obj) {
    s32 i;
    s32 j;

    for (i = 0; i < n; i++) {
        if (dist < t[i].dist) {
            if (obj != t[i].obj) {
                for (j = n - 1; j > i; j--) {
                    t[j] = t[j - 1];
                }
                t[i].dist = dist;
                t[i].obj = obj;
            }
            break;
        }
    }
    for (; i < n; i++) {
        if (t[i].obj == NULL) {
            break;
        }
    }
    return i;
}

/* Takes obj out of the list. */
void plTargetRemove(PlTarget *t, s32 n, Obj *obj) {
    s32 i;

    for (i = 0; i < n; i++) {
        if (obj == t[i].obj) {
            for (; i < n - 1; i++) {
                t[i] = t[i + 1];
            }
            t[i].dist = 3.4028235e38f;
            t[i].obj = NULL;
            return;
        }
    }
}

/* A lock-on marker ("btarge"): it closes in on its target (shrinking to 0.09, ticking every
 * third step), locks with a sound, and fades (30 a step) once fired (tag 0xC8AFBCCD 発射) or
 * gone (0xBEC3CCC7 消滅), or three steps after the lock is dropped (0xB3CEC7A7). Out of
 * reach (90) or with its target gone, it tells the lock-on (0xB0E8B3B0) and ends.
 * The original reads msg from its home with lhu for the switch and with lh for the last call:
 * the plain `switch (msg)` on the s16 does both (2026-10-17, object identical; it replaced a
 * FAKE "*(u16 *) &msg", as on the TLB procs (measured); u16, s32, K&R and
 * volatile each gave andi, a spill, or sll/sra. pad2 makes the frame 0x70 (unused words below m).
 * It matches only with btargM (the lock-on, which passes this function's address when it
 * makes a mark) in C: saBakudan's as1 rule (a jump table AND the function's address taken
 * in the file's C) decides where case 9's first load goes. */
s32 btarge(Obj *obj, s16 msg, void *arg) {
    PlMark *w = FX(obj);
    s32 pad;
    Vec3f pos;
    AnimeFrame *m;
    s32 pad2[2];

    switch (msg) {
        case 2:
            w->target = (Obj *) arg;
            obj->name = "btarge";
            if (w->target == NULL) {
                return -1;
            }
            if ((w->model = (AnimeFrame *) frmCreate(gFrmEffectLayer, gFrmKindAnime)) == NULL) {
                return -1;
            }
            w->model->anim = plBazookaMarkerAnim;
            w->model->segs[1] = fileRamAddr(0x1AF);
            plFrameScaleSet(w->model, 0.15f);
            w->t = 0;
            w->mode = 0;
            w->wait = 3;
            w->alpha = 240;
            break;
        case 0:
            m = w->model;
            if (!plIsInPlay(w->index)) {
                return -1;
            }
            if (!eneIsEnemy(w->target)) {
                plSendTagMsg(obj->parent, 0xB0E8B3B0, (s32) w->target, 0);
                return -1;
            }
            eneGetPos(w->target, &pos);
            m->pos[0] = pos.x;
            m->pos[1] = pos.y;
            m->pos[2] = pos.z;
            if (plShotDistFromPlayer(obj, (PlShot *) w) > 90.0f) {
                plSendTagMsg(obj->parent, 0xB0E8B3B0, (s32) w->target, 0);
                return -1;
            }
            switch (w->mode) {
                case 0:
                    m->scale[0] -= 0.005f;
                    if (m->scale[0] < 0.09f) {
                        m->scale[0] = 0.09f;
                        w->mode = 1;
                        plSoundAtModel(m, 0x12C, 0);
                    } else if (w->t % 3 == 0) {
                        plSoundAtModel(m, 0x2C7, 0);
                    }
                    m->scale[1] = m->scale[2] = m->scale[0];
                    break;
                case 1:
                    if (w->wait-- == 0) {
                        w->mode = 3;
                    }
                    break;
                case 2:
                    w->mode = 3;
                    break;
                case 3:
                    w->alpha -= 30;
                    if (w->alpha <= 0) {
                        return -1;
                    }
                    break;
            }
            w->t++;
            break;
        case 3:
            w->model->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, gDrawSetupCldSurfDl);
            gDPPipeSync(gGfxDlHead++);
            gSPClearGeometryMode(gGfxDlHead++, G_ZBUFFER);
            gDPSetRenderMode(gGfxDlHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
            gDPSetCombineLERP(gGfxDlHead++, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0,
                              TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0);
            gDPSetEnvColor(gGfxDlHead++, 255, 255, 255, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 9:
            switch (((s32 *) arg)[0]) {
                case 0xB3CEC7A7:
                    w->wait = 3;
                    break;
                case 0xC8AFBCCD:
                    w->mode = 2;
                    break;
                case 0xBEC3CCC7:
                    w->mode = 3;
                    break;
            }
            break;
        case 1:
            frmDestroy((Frame *) w->model);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The homing bazooka ("hbazoo"): steers at its target (by 1 a step, at most 4 fast) for 300
 * steps or until the target is gone, then flies straight on; smoke every third step.
 * The steering is written inline, its sum as z + (y + x) (as plTargetInReach's distance): that
 * gives the game's x, y, z loads, adds and registers (k f2, the x difference f12) with no block
 * splits. The frame is the game's: pad words above p, three between at and k (where the
 * source had three float locals, perhaps the differences), and one below pos (the inline sum
 * takes one more compiler temp word than named differences did). */
s32 hbazoo(Obj *obj, s16 msg, void *arg) {
    PlHoming *w = FX(obj);
    s32 pad[2];
    Player *p;
    Vec3f at;
    s32 pad3[3];
    f32 k;
    Vec3f pos;
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "hbazoo";
            w->target = (Obj *) ((PlShotArg *) arg)->param;
            if ((w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime)) == NULL) {
                return -1;
            }
            p = ((PlShotArg *) arg)->p;
            w->index = p->index;
            w->model->anim = plBazookaAnim;
            w->model->segs[1] = fileRamAddr(0x1AF);
            w->box = acollAtkAddYZ((Frame *) pl_atk_root_frame, (Frame *) w->model, 0x32, 0, 0, 0);
            w->model->flags |= 0x20;
            if (w->box == NULL) {
                return -1;
            }
            plShotPlace(p, (PlShot *) w, ((PlShotArg *) arg)->pos, ((PlShotArg *) arg)->vel);
            if (p->action == 0x86 || p->action == 0x87) {
                w->vel.x = PL_HIST(p)[0][1];
                w->vel.y = 4.0f;
                w->vel.z = PL_HIST(p)[2][1];
                mathVecRotateY(vramCourseHeading(p->zone, p->place) - 1.5707963f, &w->vel.x,
                                       &w->vel.y, &w->vel.z);
            }
            w->mode = ((PlShotArg *) arg)->mode;
            w->t = 0;
            w->dir.x = 0.0f;
            w->dir.y = 0.0f;
            w->dir.z = 0.0f;
            plAttackBoxSet(p, w->box, 0x10);
            plSound(p, 0x11E);
            break;
        case 0:
            if (!plIsInPlay(w->index)) {
                return -1;
            }
            switch (w->mode) {
                case 0:
                    if (eneIsEnemy(w->target)) {
                        eneGetPos(w->target, &pos);
                        k = 1.0f / sqrtf((pos.z - w->model->pos[2]) * (pos.z - w->model->pos[2]) +
                                         ((pos.y - w->model->pos[1]) * (pos.y - w->model->pos[1]) +
                                          (pos.x - w->model->pos[0]) * (pos.x - w->model->pos[0])));
                        w->dir.x = (pos.x - w->model->pos[0]) * k;
                        w->dir.y = (pos.y - w->model->pos[1]) * k;
                        w->dir.z = (pos.z - w->model->pos[2]) * k;
                        if (w->t++ >= 301) {
                            w->mode = 1;
                        }
                    } else {
                        w->mode = 1;
                    }
                    break;
                case 1:
                    break;
            }
            w->vel.x += w->dir.x;
            w->vel.y += w->dir.y;
            w->vel.z += w->dir.z;
            plVecClamp(&w->vel, 4.0f);
            plFramePosAdd(w->model, &w->vel);
            if (acollAtkWasHit(w->box) == 1) {
                return -1;
            }
            if (plShotHitGround((PlShot *) w, (f32 *) &at)) {
                itexPuffYellow(w->model->pos[0], w->model->pos[1], w->model->pos[2], 0.04f);
                return -1;
            }
            if (plShotDistFromPlayer(gGameWork.players[w->index], (PlShot *) w) > 110.0f) {
                return -1;
            }
            if (w->t++ % 3 == 0) {
                itexPuffGrey(w->model->pos[0], w->model->pos[1], w->model->pos[2], 0.03f);
            }
            break;
        case 1:
            frmDestroy((Frame *) w->box);
            frmDestroy((Frame *) w->model);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The homing shuriken ("hshuri"): each step it looks for the nearest target within 70 and
 * steers at it (by 0.67 a step, at most 4 fast) for 450 steps; with none it flies straight on
 * and keeps looking.
 * The steering is hbazoo's (inline, the sum as z + (y + x)); the frame (0x88) puts
 * pad4 where the three float locals were and pad3 last (one word, for the inline sum's extra
 * compiler temp word); case 0 reads w->model through w (the game loads obj into s0, reads
 * 0x30 through it, then adds 0x20). In case 2, "w->mode = arg->mode;" comes BEFORE "w->t = 0;":
 * the store through w may alias arg, so with t first IDO kept the store above arg's load. */
s32 hshuri(Obj *obj, s16 msg, void *arg) {
    PlHoming *w = FX(obj);
    s32 pad;
    PlTarget t[1];
    s32 pad2;
    Player *p;
    Vec3f at;
    s32 pad4[3];
    f32 k;
    Vec3f pos;
    s32 pad3;

    switch (msg) {
        case 2:
            obj->name = "hshuri";
            if ((w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime)) == NULL) {
                return -1;
            }
            p = ((PlShotArg *) arg)->p;
            w->index = p->index;
            w->model->anim = plShurikenAnim;
            w->model->segs[1] = fileRamAddr(0x1A5);
            if ((w->box = acollAtkAddYZ((Frame *) pl_atk_root_frame, (Frame *) w->model, 0x32, 0, 0, 0)) ==
                NULL) {
                return -1;
            }
            w->model->flags |= 0x20;
            plShotPlace(p, (PlShot *) w, ((PlShotArg *) arg)->pos, ((PlShotArg *) arg)->vel);
            if (p->action == 0xC0) {
                w->vel.x = PL_HIST(p)[0][1];
                w->vel.y = 4.0f;
                w->vel.z = PL_HIST(p)[2][1];
                mathVecRotateY(vramCourseHeading(p->zone, p->place) - 1.5707963f, &w->vel.x,
                                       &w->vel.y, &w->vel.z);
            }
            w->mode = ((PlShotArg *) arg)->mode;
            w->t = 0;
            plAttackBoxSet(p, w->box, 9);
            w->target = NULL;
            w->mode = 1;
            w->dir.x = w->dir.y = w->dir.z = 0.0f;
            break;
        case 0:
            w->model->rot[0] += 0.3926991f;
            if (plIsInPlay(w->index)) {
                if (w->mode != 2) {
                    if (!plTargetFind(PL(gGameWork.players[w->index]), t, 1)) {
                        w->mode = 1;
                        w->target = NULL;
                    } else {
                        w->mode = 0;
                        w->target = t[0].obj;
                    }
                }
            } else {
                return -1;
            }
            {
                if (w->mode == 0) {
                    if (w->target != NULL && eneIsEnemy(w->target)) {
                        eneGetPos(w->target, &pos);
                        k = 1.0f /
                            sqrtf((pos.z - w->model->pos[2]) * (pos.z - w->model->pos[2]) +
                                  ((pos.y - w->model->pos[1]) * (pos.y - w->model->pos[1]) +
                                   (pos.x - w->model->pos[0]) * (pos.x - w->model->pos[0]))) *
                            0.67f;
                        w->dir.x = (pos.x - w->model->pos[0]) * k;
                        w->dir.y = (pos.y - w->model->pos[1]) * k;
                        w->dir.z = (pos.z - w->model->pos[2]) * k;
                        if (w->t++ >= 451) {
                            w->mode = 2;
                        }
                    } else {
                        w->mode = 1;
                    }
                }
                w->vel.x += w->dir.x;
                w->vel.y += w->dir.y;
                w->vel.z += w->dir.z;
                plVecClamp(&w->vel, 4.0f);
                plFramePosAdd(w->model, &w->vel);
                if (acollAtkWasHit(w->box) == 1) {
                    return -1;
                }
                if (plShotHitGround((PlShot *) w, (f32 *) &at)) {
                    itexPuffYellow(w->model->pos[0], w->model->pos[1], w->model->pos[2], 0.04f);
                    return -1;
                }
                if (plShotDistFromPlayer(gGameWork.players[w->index], (PlShot *) w) > 110.0f) {
                    return -1;
                }
            }
            break;
        case 1:
            frmDestroy((Frame *) w->box);
            frmDestroy((Frame *) w->model);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Whether the shot hit the ground on its way. */
s32 plShotHitGround(PlShot *w, f32 *at) {
    Vec3f pos;
    f32 t;
    s32 pad[2];

    pos.x = w->model->pos[0];
    pos.y = w->model->pos[1];
    pos.z = w->model->pos[2];
    return vramRayGround(&pos, &w->vel, (Vec3f *) at, &t);
}

/* How far the shot is from its player (the distance inline, z's square first, as
 * plTargetInReach's). */
f32 plShotDistFromPlayer(Obj *task, PlShot *w) {
    f32 x;
    f32 y;
    f32 z;

    plGetModelPos(gGameWork.players[w->index], &x, &y, &z);
    return sqrtf((w->model->pos[2] - z) * (w->model->pos[2] - z) +
                 ((w->model->pos[1] - y) * (w->model->pos[1] - y) +
                  (w->model->pos[0] - x) * (w->model->pos[0] - x)));
}

void plShotSpin(PlShot *w) {
    plAnimeStepLoop(w->model, w->spin);
}

/* Runs the model's animation by step, looping. */
void plAnimeStepLoop(AnimeFrame *m, f32 step) {
    f32 n;

    if (m != NULL) {
        m->frame += step;
        n = polAnimeLastFrame(m);
        if (n < m->frame) {
            m->frame -= n;
        }
    }
}

/* Puts the charged bomb at the player's hand for its form, thrown the way the player faces at
 * speed plus 0.3 of the player's own. */
void plShotPlaceAtHand(Player *p, PlShot *w, f32 speed) {
    f32 heading;
    s32 form;
    Vec3f at;
    f32 s;

    heading = vramCourseHeading(p->zone, p->place);
    form = plAttackForm(p);
    s = mathSin(p->targetFacing + heading);
    w->vel.x = (fabsf(PL_HIST(p)[0][1]) * 0.3f + speed) * s;
    w->vel.y = 0.0f;
    s = mathCos(p->targetFacing + heading);
    w->vel.z = (fabsf(PL_HIST(p)[0][1]) * 0.3f + speed) * s;
    plVecTurnByCourse(p, p->targetFacing, &gPlAttackHandPos[p->chara][form], &at);
    w->model->pos[0] = p->model->pos[0] + at.x;
    w->model->pos[1] = p->model->pos[1] + gPlAttackHandPos[p->chara][form].y;
    w->model->pos[2] = p->model->pos[2] + at.z;
    w->model->rot[1] = p->targetFacing + heading;
    plFrameScaleSet(w->model, 0.1f);
    w->spin = 0.0f;
}

/* Starts the shot from the player's model, at speed along the way the player faces and leans. */
void plShotPlaceLean(Player *p, PlShot *w, f32 speed) {
    f32 heading;
    f32 s;

    heading = vramCourseHeading(p->zone, p->place);
    s = mathSin(p->targetFacing + heading);
    w->vel.x = mathCos(p->lean) * s * speed;
    w->vel.y = -mathSin(p->lean) * speed;
    s = mathCos(p->targetFacing + heading);
    w->vel.z = mathCos(p->lean) * s * speed;
    w->model->pos[0] = p->model->pos[0] + w->vel.x;
    w->model->pos[1] = p->model->pos[1] + w->vel.y;
    w->model->pos[2] = p->model->pos[2] + w->vel.z;
    plFrameScaleSet(w->model, 0.1f);
    w->spin = 0.0f;
}

/* Puts the shot at pos from the player's model, turned the player's way, moving by vel. */
void plShotPlace(Player *p, PlShot *w, Vec3f *pos, Vec3f *vel) {
    Vec3f v;
    AnimeFrame *m;

    m = w->model;
    plPlaceTurned(p, m, &v, pos, vel);
    w->vel = v;
    plFrameScaleSet(w->model, 0.1f);
    m->scale[0] = m->scale[1] = m->scale[2] = 0.1f;
    w->spin = 0.0f;
}

void plPlaceTurned(Player *p, AnimeFrame *m, Vec3f *outVel, Vec3f *pos, Vec3f *vel) {
    f32 heading;
    Vec3f v;
    f32 ry;
    s32 pad[2];

    heading = vramCourseHeading(p->zone, p->place);
    ry = ((p->flags & 0x10) ? p->facing : p->targetFacing) + heading;
    v = *pos;
    mathVecRotate(p->lean, ry, p->roll, &v.x, &v.y, &v.z);
    *outVel = *vel;
    mathVecRotate(p->lean, ry, p->roll, &outVel->x, &outVel->y, &outVel->z);
    m->pos[0] = p->model->pos[0] + v.x;
    m->pos[1] = p->model->pos[1] + v.y;
    m->pos[2] = p->model->pos[2] + v.z;
    m->rot[0] = p->lean;
    m->rot[1] = ry;
    m->rot[2] = p->roll;
    plFrameScaleSet(m, 0.1f);
}
