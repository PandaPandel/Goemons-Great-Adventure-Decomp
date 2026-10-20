#include "player.h"
#include "fade.h"
#include "file.h"
#include "gfxmgr.h"
#include "mathutil.h"
#include "plhoni.h"
#include "plmain.h"
#include "plpick.h"
#include "plshot.h"
#include "plsound.h"
#include "plutil.h"
#include "ride.h"
#include "tmanager.h"
#include "vram.h"

/* Character 1's thrown "honi" (overlay 9). A combo of up to three steps, each read from a table
 * of the frames it waits for: step 0 opens an attack box, steps 1-3 throw one honi each. A
 * honi flies from the player, falls, bounces on the ground and stops, can be stood on (a ride
 * frame), and breaks into five pieces when it is hit, after about three seconds, or 150 away
 * from its thrower. */

/* What a honi is made with. */
typedef struct HoniInit {
    /* 0x00 */ Player *p;
    /* 0x04 */ s32 i; /* 0-2 */
    /* 0x08 */ HoniTbl *tbl;
    /* 0x0C */ f32 *vel;
} HoniInit;

/* What a piece is made with. */
typedef struct HoniBitInit {
    /* 0x00 */ f32 pos[3];
    /* 0x0C */ s32 blink;
} HoniBitInit;

/* A piece of a broken honi. */
typedef struct PlHoniBit {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ s32 t;
    /* 0x08 */ s32 alpha;
    /* 0x0C */ f32 vel[3];
    /* 0x18 */ f32 spin[3];
    /* 0x24 */ s32 blink;
} PlHoniBit;

#define FX(obj) ((void *) ((u32) (obj) + sizeof(Obj)))
/* Character 1 keeps two u16s where the others keep a task: the honi out (a bit each) and a
 * time. */
#define PL_HONI(p) ((u16 *) &(p)->chain)

extern Frame *gRideTree1; /* the trees of ride frames */
extern Obj *gEffectTask; /* the effect manager */
extern Gfx *gGfxDlHead; /* the display-list buffer */
extern Frame *gFrmModelLayer;
extern Frame *gFrmLayer8;
extern s16 gFrmKindAnime; /* "anime" */
extern ACollFrame *pl_atk_root_frame; /* the player attacks' boxes */
extern ACollFrame *ene_dmg_root_frame; /* enemy damage */
/* plhoni.c's .data (0x801A7300-0x801A73E0). */
u32 gPlHoniAnimTable[3] = { 0x04000074, 0x040000B4, 0x040000F4 }; /* the honi animations */
u32 gPlHoniRideBoxTable[3] = { 0x04006E30, 0x04006EB0, 0x04006F30 }; /* its ride boxes */
u32 gPlHoniPieceAnimTable[2] = { 0x04000134, 0x04000174 }; /* the animations of the pieces */
HoniTbl gPlHoniComboTbl = {
    { 14.0f, 8.2f, 0.0f },
    { { 5.0f, 0.0f, 0.0f }, { 3.1f, 0.0f, 0.0f }, { 1.2f, 0.0f, 0.0f } },
    { 10, 30, 50, 70 },
};
HoniTbl gPlHoniStandComboTbl = {
    { 13.0f, 5.0f, 0.0f },
    { { 5.0f, 0.0f, 0.0f }, { 3.1f, 0.0f, 0.0f }, { 1.2f, 0.0f, 0.0f } },
    { 10, 30, 50, 70 },
};
HoniTbl gPlHoniSwimComboTbl = {
    { 14.0f, 10.0f, 0.0f },
    { { 5.0f, 0.0f, 0.0f }, { 3.1f, 0.0f, 0.0f }, { 1.2f, 0.0f, 0.0f } },
    { 10, 30, 50, 70 },
};



/* The combo's default: leaving closes the attack box. */
void plHoniActBase(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 1:
            break;
        case 2:
            frmDestroy((Frame *) p->work[0]);
            break;
    }
    plBaseDefault(p, msg, a, b);
}

void plHoniAct(Player *p, s32 msg, s32 a, s32 b) {
    s32 pad[6];
    HoniTbl tbl = gPlHoniComboTbl;

    switch (msg) {
        case 0:
            p->work[0] = 0;
            break;
        case 1:
            if (plActChange(p, plHoniComboStep(p, &tbl))) {
                return;
            }
            break;
    }
    plHoniActBase(p, msg, a, b);
}

void plHoniActStand(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;
    s32 pad[5];
    HoniTbl tbl = gPlHoniStandComboTbl;
    s32 t;

    switch (msg) {
        case 0:
            p->work[0] = 0;
            break;
        case 1:
            t = plAnimPercent(p);
            if (p->state < 4) {
                next = plHoniComboStep(p, &tbl);
            } else if (t == 100) {
                next = 8;
            }
            if (plActChange(p, next)) {
                return;
            }
            break;
    }
    plHoniActBase(p, msg, a, b);
}

void plHoniActSwim(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;
    s32 pad[5];
    HoniTbl tbl = gPlHoniSwimComboTbl;
    s32 t;

    switch (msg) {
        case 0:
            p->work[0] = 0;
            break;
        case 1:
            t = plAnimPercent(p);
            if (p->state < 4) {
                next = plHoniComboStep(p, &tbl);
            } else if (t == 100) {
                next = plPickWalk(p);
            }
            if (plActChange(p, next)) {
                return;
            }
            break;
    }
    plHoniActBase(p, msg, a, b);
}

/* A combo step, by the action's frame: the attack box, then the three throws. */
s32 plHoniComboStep(Player *p, HoniTbl *tbl) {
    s32 next = -1;
    s32 t = plAnimPercent(p);
    ACollFrame **box;

    switch (p->state) {
        case 0:
            if (t >= tbl->frame[0]) {
                box = (ACollFrame **) p->work;
                if ((*box = acollAtkAddYZ((Frame *) pl_atk_root_frame, &p->model->f, 30,
                                                   tbl->pos[1] * 10.0f,
                                                   (tbl->pos[0] - 2.0f) * 10.0f, 0)) != NULL) {
                    plAttackBoxSet(p, *box, 0x16);
                }
                p->state = 1;
                plSound(p, 0x2BF);
            }
            break;
        case 1:
            if (t >= tbl->frame[1]) {
                plHoniThrow(p, 0, tbl, tbl->vel[0]);
                p->state = 2;
            }
            break;
        case 2:
            if (t >= tbl->frame[2]) {
                plHoniThrow(p, 1, tbl, tbl->vel[1]);
                p->state = 3;
            }
            break;
        case 3:
            if (t >= tbl->frame[3]) {
                plHoniThrow(p, 2, tbl, tbl->vel[2]);
                p->state = 4;
            }
            break;
        case 4:
            if (t == 100) {
                next = plPickWalk(p);
            }
            break;
    }
    return next;
}

/* Throws honi i (0-2). */
void plHoniThrow(Player *p, s32 i, HoniTbl *tbl, f32 *vel) {
    HoniInit init;

    init.p = p;
    init.i = i;
    init.tbl = tbl;
    init.vel = vel;
    PL_HONI(p)[0] |= 1 << (i % 3);
    PL_HONI(p)[1] = 180;
    fadeCreateRetry(gGameWork.players[p->index], honi, 1, 0x1A5,
                           &init);
}

/* The honi ("honi"). */
s32 honi(Obj *obj, s16 msg, void *arg) {
    PlHoni *w = FX(obj);
    s32 pad0;
    s32 pad1;
    Player *p;
    f32 off[3];
    f32 dir;
    f32 side;
    s32 pad2[3];
    Vec3f vel;
    s32 hit;
    s32 pad3;
    HoniBitInit bit;
    s32 i;
    s32 pad4;

    switch (msg) {
        case 2:
            p = ((HoniInit *) arg)->p;
            obj->name = "honi";
            if ((w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime)) ==
                NULL) {
                return -1;
            }
            w->index = p->index;
            w->kind = ((HoniInit *) arg)->i % 3;
            w->frame->anim = gPlHoniAnimTable[w->kind];
            if ((w->ride = rideCreate(
                     gRideTree1, (s32) obj, &w->frame->f,
                     fadeSegToRam(gPlHoniRideBoxTable[w->kind], 0x1A5))) == NULL) {
                return -1;
            }
            w->frame->segs[1] = fileRamAddr(0x1A5);
            dir = vramCourseHeading(p->zone, p->place);
            plVecTurnByCourse(p, p->targetFacing, ((HoniInit *) arg)->tbl->pos, off);
            w->frame->pos[0] = off[0] + p->model->pos[0];
            w->frame->pos[1] = off[1] + p->model->pos[1];
            w->frame->pos[2] = off[2] + p->model->pos[2];
            plVecTurnByCourse(p, p->targetFacing, ((HoniInit *) arg)->vel, w->vel);
            w->frame->rot[1] = dir - 1.5707963f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 0.1f;
            w->spin = 0.0f;
            w->frame->flags |= 0x20;
            w->h = 200.0f;
            w->d = 200.0f;
            w->w = 150.0f;
            side = (mathCos(p->targetFacing) > 0.0f) ? 1.0f : -1.0f;
            w->dx = mathSin(dir) * w->w * side;
            w->dz = mathCos(dir) * w->d * side;
            w->dir = dir;
            w->t = 0;
            w->scale = 0.010000001f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale;
            w->owner = p->index;
            if ((w->box = acollAtkAddYZ((Frame *) pl_atk_root_frame, &w->frame->f, 80,
                                                 w->h * 0.5f, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->dmg = acollDmgAddYZ((Frame *) ene_dmg_root_frame, &w->frame->f, 80, 80, 0,
                                                 0, 0)) == NULL) {
                return -1;
            }
            w->state = 0;
            plAttackBoxSet(p, w->box, 0x15);
            w->dmg->kind = 0x66;
            w->dmg->power = (s32) obj;
            break;
        case 0:
            switch (w->state) {
                case 0:
                    break;
                case 1:
                    if (w->t++ >= 120) {
                        w->state = 2;
                        w->t = 0;
                    }
                    break;
                case 2:
                    if (w->t & 1) {
                        w->frame->f.flags &= ~2;
                    } else {
                        w->frame->f.flags |= 2;
                    }
                    if (w->t++ >= 61) {
                        w->state = 3;
                        w->frame->f.flags |= 2;
                    }
                    break;
                case 3:
                    plSoundAtModel(w->frame, 0x15A, w->index);
                    return -1;
            }
            if (w->scale < 0.1f) {
                w->scale += 0.02f;
                if (w->scale > 0.1f) {
                    w->scale = 0.1f;
                }
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale;
            }
            if (acollAtkWasHit(w->box) == 1 && w->box->hitKind != 0x66) {
                plSoundAtModel(w->frame, 0x15A, w->index);
                return -1;
            }
            if (acollDmgWasHit(w->dmg) == 1 && w->dmg->hitKind != 0x15 &&
                w->dmg->hitKind != 0x11 && w->dmg->hitKind != 0x16) {
                plSoundAtModel(w->frame, 0x15A, w->index);
                return -1;
            }
            vel = *(Vec3f *) w->vel;
            hit = plHoniGroundCheck(w, &vel.x);
            w->frame->pos[0] += vel.x;
            w->frame->pos[1] += vel.y;
            w->frame->pos[2] += vel.z;
            w->vel[0] *= 0.95f;
            w->vel[2] *= 0.95f;
            w->vel[1] += -0.2f;
            if (hit & 2) {
                w->vel[2] = w->vel[0] = 0.0f;
                if (w->state == 0) {
                    w->state = 1;
                }
            }
            if (hit & 1) {
                w->vel[1] = 0.0f;
                w->vel[0] *= 0.75f;
                w->vel[2] *= 0.75f;
                if (w->state == 0) {
                    w->state = 1;
                }
            }
            if (plShotDistFromPlayer(gGameWork.players[w->index], w) > 150.0f) {
                return -1;
            }
            break;
        case 1:
            bit.pos[0] = w->frame->pos[0];
            bit.pos[1] = w->frame->pos[1];
            bit.pos[2] = w->frame->pos[2];
            if (w->state >= 2) {
                bit.blink = TRUE;
            } else {
                bit.blink = FALSE;
            }
            frmDestroy(&w->box->frame);
            frmDestroy(&w->dmg->frame);
            frmDestroy(&w->frame->f);
            frmDestroy(w->ride);
            for (i = 0; i < 5; i++) {
                fadeCreateRetry(gEffectTask, plHoniBitProc, 1, 0x1A5, &bit);
            }
            if (plIsInPlay(w->owner)) {
                p = PL(gGameWork.players[w->owner]);
                if (p->chara == 1) {
                    PL_HONI(p)[0] &= ~(1 << w->kind);
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Casts the honi's move at the ground: three rays down (across its width) for the fall, the
 * highest ground wins (1), and one along the move for walls (2). vel is cut to what fits.
 * The game's slots (c 0xC4 ... hit[] 0x3C) put the four register-only scalars between the
 * arrays, in the words 0x8C/0x88/0x80/0x58, and one unused word at 0x48. */
s32 plHoniGroundCheck(PlHoni *w, f32 *vel) {
    Vec3f c;
    Vec3f a;
    Vec3f b;
    Vec3f wall;
    Vec3f dir;
    f32 k2;
    s32 i;
    s32 best;
    s32 flags;
    f32 bestK;
    Vec3f at[3];
    Vec3f *bestAt;
    f32 k[3];
    s32 pad;
    s32 hit[3];

    c.x = w->frame->pos[0];
    c.y = w->frame->pos[1] + w->h * 0.5f * w->scale;
    c.z = w->frame->pos[2];
    a.x = w->dx * 0.5f * w->scale + c.x;
    a.y = c.y;
    a.z = w->dz * 0.5f * w->scale + c.z;
    b.x = c.x - w->dx * 0.5f * w->scale;
    b.y = c.y;
    b.z = c.z - w->dz * 0.5f * w->scale;
    dir.x = 0.0f;
    flags = 0;
    dir.y = vel[1] - w->h * 0.5f * w->scale;
    dir.z = 0.0f;
    hit[0] = (s32) vramRayGround(&a, &dir, &at[0], &k[0]);
    hit[1] = (s32) vramRayGround(&c, &dir, &at[1], &k[1]);
    hit[2] = (s32) vramRayGround(&b, &dir, &at[2], &k[2]);
    best = hit[0];
    bestAt = &at[0];
    bestK = best ? k[0] : 1.0f;
    for (i = 1; i < 3; i++) {
        if (hit[i] != 0 && (best == 0 || bestAt->y < at[i].y)) {
            best = hit[i];
            bestAt = &at[i];
            bestK = k[i];
        }
    }
    if (best) {
        flags = 1;
        vel[1] *= bestK;
    }
    dir.x = vel[0] + w->dx * 0.5f * w->scale;
    dir.y = vel[1] - w->h * 0.5f * w->scale;
    dir.z = vel[2] + w->dz * 0.5f * w->scale;
    if (vramRayGround(&c, &dir, &wall, &k2)) {
        flags |= 2;
        vel[0] *= k2;
        vel[2] *= k2;
    }
    return flags;
}

/* A piece of a broken honi: it flies off spinning, falls 0.3 a step and fades; it blinks from
 * step 30 (at once if the honi was already blinking) and goes at 60. */
s32 plHoniBitProc(Obj *obj, s16 msg, void *arg) {
    PlHoniBit *w = FX(obj);

    switch (msg) {
        case 2:
            if ((w->frame = (AnimeFrame *) frmCreate(gFrmLayer8, gFrmKindAnime)) ==
                NULL) {
                return -1;
            }
            w->frame->anim = gPlHoniPieceAnimTable[fadeRand(-1) & 1];
            w->frame->segs[1] = fileRamAddr(0x1A5);
            w->frame->pos[0] = ((HoniBitInit *) arg)->pos[0];
            w->frame->pos[1] = ((HoniBitInit *) arg)->pos[1];
            w->frame->pos[2] = ((HoniBitInit *) arg)->pos[2];
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                (fadeRand(20) + 30) * 0.01f;
            w->frame->flags |= 0x20;
            w->blink = ((HoniBitInit *) arg)->blink;
            if (w->blink != 0) {
                w->t = 30;
            } else {
                w->t = 0;
            }
            w->alpha = 255;
            w->vel[0] = (fadeRand(30) - 15) * 0.05f;
            w->vel[1] = (fadeRand(10) + 8) * 0.2f;
            w->vel[2] = (fadeRand(30) - 15) * 0.05f;
            w->spin[0] = (fadeRand(78) - 39.26990891f) * 0.005f;
            w->spin[1] = (fadeRand(78) - 39.26990891f) * 0.005f;
            w->spin[2] = (fadeRand(78) - 39.26990891f) * 0.005f;
            break;
        case 0:
            w->frame->pos[0] += w->vel[0];
            w->frame->pos[1] += w->vel[1];
            w->frame->pos[2] += w->vel[2];
            w->frame->rot[0] += w->spin[0];
            w->frame->rot[1] += w->spin[1];
            w->frame->rot[2] += w->spin[2];
            w->vel[1] -= 0.3f;
            if (w->blink) {
                if (w->t & 1) {
                    w->frame->f.flags &= ~2;
                } else {
                    w->frame->f.flags |= 2;
                }
            }
            if (++w->t >= 61) {
                return -1;
            }
            if (w->t >= 31) {
                w->blink = TRUE;
            }
            if (w->alpha != 0) {
                w->alpha -= 10;
                if (w->alpha < 0) {
                    w->alpha = 0;
                }
            }
            break;
        case 3:
            if (w->frame != NULL) {
                w->frame->dl = gGfxDlHead;
                gDPPipeSync(gGfxDlHead++);
                gDPSetCycleType(gGfxDlHead++, G_CYC_1CYCLE);
                gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
                gSPEndDisplayList(gGfxDlHead++);
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
