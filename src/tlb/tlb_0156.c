#include "tlb.h"

/* The "boatskel" enemy kind (archive file 156): a skeleton rowing a boat that bobs on the
 * water. Three models: the boat (its EneObj model, bobbing on a sine of the frame count), the
 * skeleton's body (three animations in turn: states 0-2) and its oars (always looping). Near the
 * end of the body's third animation it throws a "head" (kind 0x211): a skull that flies, grows
 * as it leaves the boat's line and splashes once when it reaches the water. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

/* libultra's sinf (PR/gu.h declares it), called from the TLB through its address. */
#define sinf (*sinf)

#pragma intrinsic(fabs)

extern u8 boatskelRowBodyAnim[];
extern u8 etskBoatskelBodyAnim[];
extern u8 boatskelWindUpBodyAnim[];
extern u8 etskBoatskelOarsAnim[];
extern u8 etskBoatskelAnim[];
extern u8 boatskelHeadDl[];

/* A boatskel's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct BoatSkel {
    /* 0x00 */ EneObj e; /* its model is the boat */
    /* 0x4C */ AnimeFrame *body; /* the skeleton */
    /* 0x50 */ AnimeFrame *oars;
    /* 0x54 */ u8 type; /* its placement's type, from eneSetup */
} BoatSkel;

/* A thrown head's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct BoatSkelHead {
    /* 0x00 */ EneObj e;
    /* 0x4C */ s16 homeZ; /* where it was thrown from, across the course */
    /* 0x4E */ u8 type; /* its placement's type, from eneSetup (0: no growing) */
    /* 0x4F */ u8 splashed; /* its splash is made */
} BoatSkelHead;

#define BOATSKEL(obj) ((BoatSkel *) FX(obj))
#define HEAD(obj) ((BoatSkelHead *) FX(obj))
#define MODEL(w) ((AnimeFrame *) (w)->e.model)
#define POL(w) ((PolFrame *) (w)->e.model)

void boatskelRow(Obj *o);
void boatskelWindUp(Obj *o);
void boatskelThrow(Obj *o);

s32 Etsk_boatskel_main(Obj *o, s16 msg, void *arg) {
    BoatSkel *w = BOATSKEL(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "boatskel";
            w->type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewAnime(o, (u32) etskBoatskelAnim)) == NULL) {
                return -1;
            }
            if ((w->body = eneNewAnime(o, (u32) etskBoatskelBodyAnim)) == NULL) {
                return -1;
            }
            if ((w->oars = eneNewAnime(o, (u32) etskBoatskelOarsAnim)) == NULL) {
                return -1;
            }
            w->e.behave = 0x400000;
            if (w->type == 2) {
                w->e.rot[1] = eneCourseAngle(o, 8);
            } else {
                w->e.rot[1] = eneCourseAngle(o, 4);
            }
            w->e.state = 0;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*boatskelRow)(o);
                    break;
                case 1:
                    (*boatskelWindUp)(o);
                    break;
                case 2:
                    (*boatskelThrow)(o);
                    break;
            }
            w->e.rot[1] = eneCourseAngle(o, 4);
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_boatskel_main: 死亡\n");
                return -1;
            }
            MODEL(w)->pos[1] = sinf((f32) gGameFrameCount / 7.5) * 0.8 + w->e.y;
            w->body->pos[0] = w->oars->pos[0] = MODEL(w)->pos[0];
            w->body->pos[1] = w->oars->pos[1] = MODEL(w)->pos[1];
            w->body->pos[2] = w->oars->pos[2] = MODEL(w)->pos[2];
            w->body->rot[1] = w->oars->rot[1] = MODEL(w)->rot[1];
            if (w->type == 3) {
                MODEL(w)->rot[1] = w->e.rot[1] + 3.141592741f;
            }
            break;
        case 1:
            if (w->e.model != NULL) {
                frmDestroy(w->e.model);
            }
            if (w->body != NULL) {
                frmDestroy(&w->body->f);
            }
            if (w->oars != NULL) {
                frmDestroy(&w->oars->f);
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void boatskelRow(Obj *o) {
    BoatSkel *w = BOATSKEL(o);
    s32 pad[2];

    if (w->type == 3) {
        w->e.vel[0] = 1.0f;
        w->e.vel[1] = -0.1f;
    }
    if (eneAnimPlayOnce(o, w->body, 0.5f)) {
        w->body->anim = boatskelRowBodyAnim;
        w->body->frame = 0.0f;
        w->e.state = 1;
    }
    eneAnimLoop(o, w->oars, 0.5f);
}

void boatskelWindUp(Obj *o) {
    BoatSkel *w = BOATSKEL(o);

    if (eneAnimPlayOnce(o, w->body, 0.5f)) {
        w->body->anim = boatskelWindUpBodyAnim;
        w->body->frame = 0.0f;
        w->e.state = 2;
    }
    eneAnimLoop(o, w->oars, 0.5f);
}

/* The step is a local: as a literal, "5.0 + 0.5f" folds, where the game converts the 0.5f and
 * adds. */
void boatskelThrow(Obj *o) {
    BoatSkel *w = BOATSKEL(o);
    f32 step = 0.5f;
    s32 pad;

    if (eneAnimPlayOnce(o, w->body, step)) {
        w->body->anim = etskBoatskelBodyAnim;
        w->body->frame = 0.0f;
        w->e.state = 0;
    }
    if (w->body->frame > 5.0 && w->body->frame <= 5.0 + step) {
        eneMakeEx(o, ENE_BOATSKEL_HEAD_MAIN, w->e.zone, w->e.x - 9.0f, w->e.y + 28.0f, w->e.z, 2,
                  (u32 (*)(void *, void *, void *)) w->type);
    }
    eneAnimLoop(o, w->oars, step);
}

/* The head. The game keeps w in no saved register (v1, spilled around each call): uopt gives
 * it s0 unless the function has enough blocks, so the block count is the game's own. The hit
 * kinds that do nothing are an empty first branch (written as nested ifs with !=, w took s0),
 * and the print of "" (its string follows the death print's) is one block more.
 * The frees are one line each, as in kijim (archive file 162): the addiu of w in the first
 * test's delay slot. */
s32 boatskelHeadMain(Obj *o, s16 msg, void *arg) {
    BoatSkelHead *w = HEAD(o);
    s32 pad;
    Vec3f p;
    Vec3f push;

    switch (msg) {
        case 2:
            o->name = "head";
            w->type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewPol(o, (Gfx *) boatskelHeadDl)) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 30, 30, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 123;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 40, 80, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->e.behave = 0x800003;
            w->e.rot[1] = eneCourseAngle(o, 4);
            w->homeZ = w->e.z;
            w->splashed = 0;
            if (w->type == 0) {
                POL(w)->scale[2] = 0.12f;
                POL(w)->scale[0] = POL(w)->scale[1] = POL(w)->scale[2];
            } else {
                w->e.vel[2] = w->e.vel[1] = 3.0f;
            }
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            if (w->type != 0) {
                POL(w)->scale[0] = fabs(w->e.z - w->homeZ) / 55.0 * 0.05 + 0.1;
                POL(w)->scale[1] = POL(w)->scale[2] = POL(w)->scale[0];
                if (w->e.hit & 9) {
                    w->e.behave |= 0x4000000;
                }
            }
            if (w->e.hit & 0x10) {
                if (w->e.hitKind == 10 || w->e.hitKind == 13 || w->e.hitKind == 14) {
                    /* these attack kinds do nothing to it */
                } else if (w->e.hitKind == 21) {
                    w->e.behave |= 0x4000000;
                } else {
                    w->e.behave |= 0x10000000;
                }
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_boatskel_head: 死亡\n");
                return -1;
            }
            if (w->splashed == 0) {
                p.x = POL(w)->pos[0];
                p.y = POL(w)->pos[1];
                p.z = POL(w)->pos[2];
                if (rideTree2PointIn(&p, &push)) {
                    eneSetSd(o, 0x298);
                    effectSibukiCreateWhite(p.x, p.y, p.z, 0.05999999866f, 0.05999999866f, 0.004999999888f,
                                            0.004999999888f);
                    w->splashed = 1;
                }
            }
            break;
        case 1:
            if (w->e.model != NULL) { frmDestroy(w->e.model); }
            if (w->e.atk != NULL) { frmDestroy(&w->e.atk->frame); }
            if (w->e.dmg != NULL) { frmDestroy(&w->e.dmg->frame); }
            break;
        case 3:
            break;
    }
    PRINTF("");
    return tskDefaultProc(o, msg, arg);
}
