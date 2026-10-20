#include "tlb.h"

/* The "shibag" enemy kind (archive file 163): one that waits until a player is within 70, turns
 * to face the course across or along, swings (its two attack boxes live from frames 4 and 8,
 * its damage box growing to 195 high), then at frame 6 of its throw puts out an "iwa" (a
 * rock, kind 0x21D) its way; it recovers for 80 steps and swings again. The iwa fades in,
 * then flies off its thrower's way, can be knocked back, and is gone after 120 steps. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 etskShibagAnim[];
extern u8 shibagSwingAnim[];
extern u8 shibagIwaDl[];
extern u8 etskShibagHalf0Dl[];
extern u8 etskShibagHalf1Dl[];

/* A shibag's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Shibag {
    /* 0x00 */ EneObj e;
    /* 0x4C */ Obj *iwa; /* the rock it last put out */
    /* 0x50 */ u8 wait; /* state 3: steps left before it swings again */
    /* 0x51 */ u8 way; /* 1, 2, 4 or 8: the way it faces and throws */
} Shibag;

/* An iwa's work: an enemy's and its own. */
typedef struct ShibagIwa {
    /* 0x00 */ EneObj e;
    /* 0x4C */ u8 way; /* its thrower's way, from eneSetup */
    /* 0x4D */ u8 alpha;
    /* 0x4E */ u8 age; /* steps flown */
    /* 0x4F */ u8 flying; /* 1 once thrown: it falls */
} ShibagIwa;

#define SHIBAG(obj) ((Shibag *) FX(obj))
#define IWA(obj) ((ShibagIwa *) FX(obj))

void shibagWait(Obj *o);
void shibagSwing(Obj *o);
void shibagThrow(Obj *o);
void shibagRecover(Obj *o);
void shibagIwaFadeIn(Obj *o);
void shibagIwaFly(Obj *o);

s32 Etsk_shibag_main(Obj *o, s16 msg, void *arg) {
    Shibag *w = SHIBAG(o);
    ACollFrame *box;
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "shibag";
            eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewAnime(o, (u32) etskShibagAnim)) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 40, 40, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((box = ATK_ADD_YZ(&w->e.atk->frame, w->e.model, 35, 140, 0, 0)) == NULL) {
                return -1;
            }
            box->kind = 120;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 50, 195, 0, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->e.shadow = (Obj *) shadowCreate()) == NULL) {
                return -1;
            }
            w->e.halves[0] = (u32) etskShibagHalf0Dl;
            w->e.halves[1] = (u32) etskShibagHalf1Dl;
            w->e.atk->flags &= ~2;
            ((ACollFrame *) w->e.atk->frame.child)->flags &= ~2;
            w->e.dmg->flags &= ~2;
            if (eneSetOnEarth(o) == -1) {
                return -1;
            }
            w->e.behave = 0xC10000;
            w->wait = 0;
            w->e.state = 0;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*shibagWait)(o);
                    break;
                case 1:
                    (*shibagSwing)(o);
                    break;
                case 2:
                    (*shibagThrow)(o);
                    break;
                case 3:
                    (*shibagRecover)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_shibag_main: 死亡\n");
                return -1;
            }
            break;
        case 1:
            if (w->e.model != NULL) { frmDestroy(w->e.model); }
            if (w->e.atk != NULL) { frmDestroy(&w->e.atk->frame); }
            if (w->e.dmg != NULL) { frmDestroy(&w->e.dmg->frame); }
            if (w->e.shadow != NULL) { frmDestroy((Frame *) w->e.shadow); }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* State 0: waits for a player within 70, then faces across the course toward its middle (or
 * along it, toward the nearest player, when on it) and swings. */
void shibagWait(Obj *o) {
    Shibag *w = SHIBAG(o);

    if (enePlInRangeCheck(o, 70.0f, 0)) {
        if (w->e.z < 0.0f) {
            w->way = 8;
            w->e.rot[1] = eneCourseAngle(o, w->way);
        } else if (0.0f < w->e.z) {
            w->way = 2;
            w->e.rot[1] = eneCourseAngle(o, w->way);
        } else {
            w->way = enePlDirX(o, 0);
            w->e.rot[1] = eneCourseAngle(o, 8);
        }
        w->e.state = 1;
    }
}

/* State 1: the swing; turning along the course when it faces along it. Its attack boxes go
 * live from frames 4 and 8, its damage box with a height growing with the frame. Then it
 * throws. The step is a local negated in the call (the game's neg.s and its two copies of
 * the constant); the angle, as a call nested in the arguments, cost a compiler temp. */
void shibagSwing(Obj *o) {
    Shibag *w = SHIBAG(o);
    f32 step = 0.1121997386f;
    f32 angle;
    s32 pad[2];
    s32 done;

    done = eneAnimPlayOnce(o, (AnimeFrame *) w->e.model, 1.0f);
    if (w->way == 4 || w->way == 1) {
        angle = eneCourseAngle(o, w->way);
        eneTurnRotYTo(o, angle, (w->way == 4) ? -step : step);
    }
    if (((AnimeFrame *) w->e.model)->frame >= 4.0f) {
        w->e.atk->flags |= 2;
    }
    if (((AnimeFrame *) w->e.model)->frame >= 8.0f) {
        ((ACollFrame *) w->e.atk->frame.child)->flags |= 2;
    }
    w->e.dmg->flags |= 2;
    w->e.dmg->height = ((AnimeFrame *) w->e.model)->frame * 195.0 / 14.0;
    if (done) {
        ((AnimeFrame *) w->e.model)->anim = shibagSwingAnim;
        ((AnimeFrame *) w->e.model)->frame = 0.0f;
        w->e.state = 2;
    }
}

/* State 2: the throw, slower from frame 6; at frame 6 it puts out an iwa its way (one at a
 * time) and holds the throw there until the iwa has faded in, then hands it to the enemy
 * base and plays on. */
void shibagThrow(Obj *o) {
    Shibag *w = SHIBAG(o);
    s32 pad;
    f32 speed = 0.5f;

    if (((AnimeFrame *) w->e.model)->frame >= 6.0) {
        speed /= 2;
    }
    if (((AnimeFrame *) w->e.model)->frame >= 6.0 && ((AnimeFrame *) w->e.model)->frame < 6.0 + speed) {
        if (tskCountChildren(o) == 0) {
            switch (w->way) {
                case 4:
                    w->iwa = eneMakeEx(o, ENE_SHIBAG_IWA_MAIN, w->e.zone, w->e.x + 10.0f, w->e.y + 20.0f, w->e.z, 2,
                                       (u32 (*)(void *, void *, void *)) (s32) w->way);
                    break;
                case 1:
                    w->iwa = eneMakeEx(o, ENE_SHIBAG_IWA_MAIN, w->e.zone, w->e.x - 10.0f, w->e.y + 20.0f, w->e.z, 2,
                                       (u32 (*)(void *, void *, void *)) (s32) w->way);
                    break;
                case 8:
                    w->iwa = eneMakeEx(o, ENE_SHIBAG_IWA_MAIN, w->e.zone, w->e.x, w->e.y + 20.0f, w->e.z - 10.0f, 2,
                                       (u32 (*)(void *, void *, void *)) (s32) w->way);
                    break;
                case 2:
                    w->iwa = eneMakeEx(o, ENE_SHIBAG_IWA_MAIN, w->e.zone, w->e.x, w->e.y + 20.0f, w->e.z + 10.0f, 2,
                                       (u32 (*)(void *, void *, void *)) (s32) w->way);
                    break;
                default:
                    break;
            }
        }
        if (IWA(w->iwa)->alpha < 0xFF) {
            return;
        }
        tskUnlink(w->iwa);
        tskLink(w->iwa, eneGetBase(1));
    }
    if (eneAnimPlayOnce(o, (AnimeFrame *) w->e.model, speed)) {
        ((AnimeFrame *) w->e.model)->anim = etskShibagAnim;
        ((AnimeFrame *) w->e.model)->frame = polAnimeLastFrame((AnimeFrame *) w->e.model);
        w->wait = 80;
        w->e.state = 3;
    }
}

/* State 3: plays the swing backward, its damage box off; then waits out its count, faces as
 * in state 0 and swings again. The attack boxes go off below frames 4 and 8. */
void shibagRecover(Obj *o) {
    Shibag *w = SHIBAG(o);

    if (eneAnimPlayOnce(o, (AnimeFrame *) w->e.model, -1.0f)) {
        w->e.dmg->flags &= ~2;
        if (w->wait - 1 >= 0) {
            w->wait = w->wait - 1;
        } else {
            if (w->e.z < 0.0f) {
                w->way = 8;
                w->e.rot[1] = eneCourseAngle(o, w->way);
            } else if (0.0f < w->e.z) {
                w->way = 2;
                w->e.rot[1] = eneCourseAngle(o, w->way);
            } else {
                w->way = enePlDirX(o, 0);
                w->e.rot[1] = eneCourseAngle(o, 8);
            }
            w->e.state = 1;
        }
    }
    if (((AnimeFrame *) w->e.model)->frame < 4.0f) {
        w->e.atk->flags &= ~2;
    }
    if (((AnimeFrame *) w->e.model)->frame < 8.0f) {
        ((ACollFrame *) w->e.atk->frame.child)->flags &= ~2;
    }
    w->e.dmg->height = ((AnimeFrame *) w->e.model)->frame * 195.0 / 14.0;
}

/* "iwa": a shibag's rock. */
s32 shibagIwaMain(Obj *o, s16 msg, void *arg) {
    ShibagIwa *w = IWA(o);
    s32 pad;
    Vec3f v;

    switch (msg) {
        case 2:
            o->name = "iwa";
            w->way = eneSetup(o, arg);
            if ((w->e.model = (Frame *) enePolModelAlpha(o, (Gfx *) shibagIwaDl, 0)) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 40, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 123;
            w->e.atk->flags &= ~2;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 50, 100, -50, 0, 0)) == NULL) {
                return -1;
            }
            w->e.dmg->flags &= ~2;
            v.x = w->e.vel[0];
            v.y = w->e.vel[1];
            v.z = w->e.vel[2];
            w->e.atk->info = *(ACollInfo *) &v;
            w->e.behave = 7;
            w->e.rot[1] = eneCourseAngle(o, 8);
            w->alpha = 0;
            w->age = 0;
            w->e.state = 4;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 4:
                    (*shibagIwaFadeIn)(o);
                    break;
                case 5:
                    (*shibagIwaFly)(o);
                    break;
            }
            if (w->e.hit & 0xE) {
                w->e.behave |= 0x4000000;
            }
            if (w->flying) {
                eneFall(o, 0.1f);
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_shibag_iwa: 死亡\n");
                return -1;
            }
            /* fallthrough */
        case 3:
            ((PolFrame *) w->e.model)->dl = gGfxDlHead;
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            if (w->e.model != NULL) { frmDestroy(w->e.model); }
            if (w->e.atk != NULL) { frmDestroy(&w->e.atk->frame); }
            if (w->e.dmg != NULL) { frmDestroy(&w->e.dmg->frame); }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Iwa state 4: fades in; once whole and at rest, its boxes go live and it flies off its
 * thrower's way. */
void shibagIwaFadeIn(Obj *o) {
    ShibagIwa *w = IWA(o);
    s32 pad;

    if (w->alpha + 32 < 0xFF) {
        w->alpha += 32;
    } else {
        w->alpha = 0xFF;
        frmMove(w->e.model, gFrmModelLayer);
    }
    if (w->alpha == 0xFF && w->e.vel[1] == 0.0f) {
        w->e.atk->flags |= 2;
        w->e.dmg->flags |= 2;
        w->flying = 1;
        w->e.vel[1] = 0.8f;
        switch (w->way) {
            case 4:
                w->e.vel[0] = -3.0f;
                break;
            case 1:
                w->e.vel[0] = 3.0f;
                break;
            case 8:
                w->e.vel[2] = 3.0f;
                break;
            case 2:
                w->e.vel[2] = -3.0f;
                break;
        }
        w->e.state = 5;
    }
}

/* Iwa state 5: flies; a hit knocks it back; gone after 120 steps. */
void shibagIwaFly(Obj *o) {
    ShibagIwa *w = IWA(o);
    s32 pad;

    if (acollDmgWasHit(w->e.dmg) == 1) {
        w->e.vel[0] = eneKnockbackSpeed(o, w->e.dmg, 5.0f);
        w->e.vel[1] = 2.0f;
        w->e.dmg->flags &= ~2;
    }
    if (w->age++ > 120) {
        w->e.behave |= 0x10000000;
    }
}
