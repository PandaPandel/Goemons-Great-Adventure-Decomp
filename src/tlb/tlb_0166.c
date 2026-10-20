#include "tlb.h"

/* "biwa" (its proc Etsk_biwa_main): what its first helper touches. */
typedef struct BiwaW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 unused09;
    /* 0x0A */ u8 hit; /* EneObj's */
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ AnimeFrame *anim;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vx;
    /* 0x2C */ f32 vy;
    /* 0x30 */ f32 vz;
    /* 0x34 */ ACollFrame *atk; /* EneObj's, from here to behave */
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ MPolFrame *shadow;
    /* 0x40 */ u32 halves[2];
    /* 0x48 */ u32 behave;
    /* 0x4C */ u8 pad4C[0x4];
    /* 0x50 */ s16 baseX;
    /* 0x52 */ s16 baseY; /* the height it bobs about */
    /* 0x54 */ u8 partWait; /* frames to the next part */
    /* 0x55 */ u8 parts; /* parts made */
    /* 0x56 */ u8 partFlew; /* set by a part as it flies off */
} BiwaW;

typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                    s32 a);

void biwaWait(Obj *o);
void biwaMakeParts(Obj *o);
void biwaSlide(Obj *o);
void biwaSlideStop(Obj *o);
extern f32 biwaSpiderPlaces[][3];
extern u8 etskBiwaAnim[]; /* the biwa's animation */
extern Gfx etskBiwaHalf0Dl[]; /* its two halves, for a cut */
extern Gfx etskBiwaHalf1Dl[];

#define DPRINTF

/* "biwa", the lute: on its start it makes its three spiders ("kumo", kind 0x223) at their places
 * about it, its attack box (radius 45, 80 up, kind 0x78), its damage box (80 by 180) and its
 * shadow, keeps its place as a base and faces along the course; then it runs its state's step
 * (waiting, making its parts, and the two after). */
s32 Etsk_biwa_main(Obj *o, s16 msg, void *arg) {
    BiwaW *w = (BiwaW *) FX(o);
    s32 i;
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "biwa";
            eneSetup(o, arg);
            if ((w->anim = eneNewAnime(o, (u32) etskBiwaAnim)) == NULL) {
                return -1;
            }
            for (i = 0; i < 3; i++) {
                eneMakeEx(o, ENE_ETSK_BIWA_KUMO, w->zone, biwaSpiderPlaces[i][0] + w->x, biwaSpiderPlaces[i][1] + w->y,
                          biwaSpiderPlaces[i][2] + w->z, 2, (u32 (*)(void *, void *, void *)) i);
            }
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->anim->f, 45, 80, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 0x78;
            if ((w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->anim->f, 80, 180, 0, 0, 0)) ==
                NULL) {
                return -1;
            }
            if ((w->shadow = shadowCreate()) == NULL) {
                return -1;
            }
            w->halves[0] = (u32) etskBiwaHalf0Dl;
            w->halves[1] = (u32) etskBiwaHalf1Dl;
            w->behave = 0x400000;
            w->partFlew = 0;
            w->baseX = w->x;
            w->baseY = w->y;
            w->rot[1] = eneCourseAngle(o, 8);
            w->state = 0;
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (((BiwaW *) FX(o))->state) {
                case 0:
                    (*biwaWait)(o);
                    break;
                case 1:
                    (*biwaMakeParts)(o);
                    break;
                case 2:
                    (*biwaSlide)(o);
                    break;
                case 3:
                    (*biwaSlideStop)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                DPRINTF("Etsk_biwa_main: 死亡\n");
                return -1;
            }
            break;
        case 1:
            if (w->anim != NULL) {
                frmDestroy((Frame *) w->anim);
            }
            if (w->atk != NULL) {
                frmDestroy((Frame *) w->atk);
            }
            if (w->dmg != NULL) {
                frmDestroy((Frame *) w->dmg);
            }
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Loops its animation at half speed; once a player is within 120, state 1. */
void biwaWait(Obj *o) {
    BiwaW *w = (BiwaW *) FX(o);

    eneAnimLoop(o, w->anim, 0.5f);
    if (enePlInRangeCheck(o, 120.0f, 0)) {
        /* state stored last: IDO then builds w (addiu v0,v0,0x20) after the byte stores, as the
         * game does */
        w->parts = 0;
        w->partWait = 0;
        w->vx = -0.4f;
        w->state = 1;
    }
}

extern u8 biwaFireTex0[];
extern u8 biwaFireTex2[];
extern u8 biwaFireTex5[];
AnimFrame biwaFireAnim[] = {
    { (u32) biwaFireTex0, 2 },
    { 0x03000D80, 2 },
    { (u32) biwaFireTex2, 2 },
    { 0x03001D80, 2 },
    { 0x03002580, 2 },
    { (u32) biwaFireTex5, 2 },
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
};
f32 biwaPartPlaces[][3] = { -15.0f, 1e+01f, 0.0f, 0.0f, 2e+01f, 0.0f, 15.0f, 1e+01f, 0.0f }; /* the parts' places on the biwa */
f32 biwaSpiderPlaces[][3] = { -6.0f, 0.0f, 0.0f, 6.0f, 0.0f, 0.0f, 0.0f, -2.0f, 5.0f }; /* the spiders' places */
s32 Etsk_biwa_fire(Obj *obj, s16 msg, void *arg);

#define sinf (*sinf)

/* The biwa's step: loops its animation at half speed. With a player within 70 it makes its
 * three parts (enemy 0x224, numbered) one every 5 frames at their places. When a part flies off
 * it lets all of its parts go to the enemies' base (sound 0x138). It bobs 4 about its height,
 * and against a wall it takes behave bits 0x220000.
 * eneMakeEx's last argument is cast (s32), and sinf's argument is computed into a local first. */
void biwaMakeParts(Obj *o) {
    BiwaW *w = (BiwaW *) FX(o);
    f32 phase;
    s32 wait;
    Obj *part;

    eneAnimLoop(o, ((BiwaW *) FX(o))->anim, 0.5f);
    if (enePlInRangeCheck(o, 70.0f, 0)) {
        wait = w->partWait - 1;
        if (wait >= 0) {
            w->partWait = wait;
        } else {
            if (w->parts < 3) {
                eneMakeEx(o, ENE_ETSK_BIWA_FIRE, w->zone, biwaPartPlaces[w->parts][0] + w->x,
                          biwaPartPlaces[w->parts][1] + w->y, biwaPartPlaces[w->parts][2] + w->z, 2,
                          (u32 (*)(void *, void *, void *)) (s32) w->parts);
                w->parts++;
            }
            w->partWait = 4;
        }
    }
    if (w->partFlew) {
        eneSetSd2(o, 0x138, 0, 0);
        while ((part = tskFind(o, Etsk_biwa_fire, o->file)) != NULL) {
            tskUnlink(part);
            tskLink(part, eneGetBase(1));
        }
        w->partFlew = 0;
    }
    phase = gGameFrameCount / 10.0f;
    w->y = sinf(phase) * 4.0f + w->baseY;
    eneWallCheck(o, 10.0f, 15.0f);
    if (w->hit & 6) {
        w->behave |= 0x220000;
    }
}

/* Loops its animation; with a player within 20 it slides faster (by 0.6 a frame, to at most
 * 2), with none within 45 it slows by 0.2 to a stop. It leans into the slide: its heading is
 * the course's, less half its speed times 60 degrees. */
void biwaSlide(Obj *o) {
    EneObj *w = ENE(o);
    s32 pad;

    eneAnimLoop(o, ((BiwaW *) FX(o))->anim, 1.0f);
    if (enePlInRangeCheck(o, 20.0f, 0)) {
        if (w->vel[0] + 0.6 < 2.0) {
            w->vel[0] += 0.6;
        } else {
            w->vel[0] = 2.0f;
        }
    } else if (!enePlInRangeCheck(o, 45.0f, 0)) {
        if (w->vel[0] > 0.2) {
            w->vel[0] -= 0.2;
        } else {
            w->vel[0] = 0.0f;
        }
    }
    w->rot[1] = eneCourseAngle(o, 8) - w->vel[0] * 0.5 * 1.04719758033752441;
}

/* Slows its slide by 0.2 a frame to a stop, and leans into it: its heading is the course's,
 * less half its speed times 45 degrees. */
void biwaSlideStop(Obj *o) {
    EneObj *w = ENE(o);
    s32 pad;

    if (ENE(o)->vel[0] > 0.2) {
        w->vel[0] -= 0.2;
    } else {
        w->vel[0] = 0.0f;
    }
    w->rot[1] = eneCourseAngle(o, 8) - w->vel[0] * 0.5 * 0.785398185253143311;
}

/* "kumo"'s work: EneObj's start, then its own. */
typedef struct KumoW {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u8 pad34[0x18];
    /* 0x4C */ u8 alpha;
    /* 0x4D */ u8 type; /* eneSetup's: which of the three places */
} KumoW;

extern Gfx etskBiwaKumoDl[]; /* the spider */

/* "kumo", a spider on the biwa: a model of a tenth, at its place (kind) about the biwa, its
 * scale bobbing 0.11 +- 0.01 (each spider 20 frames apart), drawn white over grey. */
s32 Etsk_biwa_kumo(Obj *o, s16 msg, void *arg) {
    KumoW *w = (KumoW *) FX(o);
    s32 pad;
    f32 phase;

    switch (msg) {
        case 2:
            o->name = "kumo";
            w->type = eneSetup(o, arg);
            if ((w->model = enePolModelMake(o, etskBiwaKumoDl, NULL, 1)) == NULL) {
                return -1;
            }
            w->model->scale[2] = 0.1f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->flags = 2;
            w->alpha = 0xFF;
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            w->x = biwaSpiderPlaces[w->type][0] + ((KumoW *) FX(o->parent))->x;
            w->y = biwaSpiderPlaces[w->type][1] + ((KumoW *) FX(o->parent))->y;
            w->z = biwaSpiderPlaces[w->type][2] + ((KumoW *) FX(o->parent))->z;
            phase = (w->type * 20 + gGameFrameCount) / 10.0f;
            w->model->scale[0] = sinf(phase) * 0.01 + 0.11;
            w->model->scale[1] = w->model->scale[0];
            if (eneAfterProc(o) == -1) {
                DPRINTF("Etsk_biwa_kumo: 死亡\n");
                return -1;
            }
        case 3:
            w->model->dl = eneDlEnvPrimColor(o, 0xFF, 0xFF, 0xFF, 0x7F, 0x7F, 0x7F, w->alpha);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy((Frame *) w->model);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* A biwa part's work (the proc Etsk_biwa_fire steps it): an enemy's, with its slot. */
typedef struct BiwaPartW {
    /* 0x00 */ EneObj e;
    /* 0x4C */ Anim anim; /* its flame */
    /* 0x5C */ u8 fade; /* its drawn alpha, in by 32 a frame */
    /* 0x5D */ u8 slot; /* its place on the parent, in biwaPartPlaces */
    /* 0x5E */ u8 alpha; /* its colour's blend, yellow to red */
} BiwaPartW;

void biwaFireWait(Obj *o);
void biwaFireLaunch(Obj *o);
void biwaFireFly(Obj *o);
extern Gfx etskBiwaFireDl[]; /* a flame */
extern AnimFrame biwaFireAnim[]; /* its flicker */

/* From a toward b as t goes 0 to 256. */
#define LERP(a, b, t) ((((b) - (a)) / 256) * (t) + (a))

/* "fire", a flame part of the biwa (its slot from eneSetup): a model (file 0x125) of a tenth,
 * flickering. It waits on its place on the biwa (state 4), fades in and flies off (5, 6),
 * bobbing 1 (each part 20 frames apart); hit, it is gone. Its colours run from yellow over pale
 * blue to red over black as its blend goes up.
 * Matched 2026-10-16; it was parked for its colours' registers. As in the game, the lerps' ends
 * are initialised variables, not constants, the colours are u8 variables set before the call,
 * and the prim's red and green start from a literal 0.0f. */
s32 Etsk_biwa_fire(Obj *o, s16 msg, void *arg) {
    BiwaPartW *w = (BiwaPartW *) FX(o);
    f32 s0 = 191.0f, s1 = 255.0f, s2 = 191.0f, s5 = 255.0f;
    f32 e0 = 255.0f, e1 = 191.0f, e2 = 63.0f, e3 = 127.0f, e4 = 0.0f, e5 = 0.0f;
    u8 er, eg, eb; /* the env colour */
    u8 pr, pg, pb; /* the prim colour */
    s32 pad[9]; /* FAKE: the game's frame (0xA0), greedy's pads */

    switch (msg) {
        case 2:
            o->name = "fire";
            w->slot = eneSetup(o, arg);
            if ((w->e.model = (Frame *) enePolModelMake(o, etskBiwaFireDl, NULL, 1)) == NULL) {
                return -1;
            }
            floadGetSetRam((void **) ((PolFrame *) w->e.model)->segs, 0x125);
            ((PolFrame *) w->e.model)->scale[2] = 0.1f;
            ((PolFrame *) w->e.model)->scale[0] = ((PolFrame *) w->e.model)->scale[1] =
                ((PolFrame *) w->e.model)->scale[2];
            ((PolFrame *) w->e.model)->flags = 2;
            fadeAnimStart(&w->anim, biwaFireAnim, 0, &((PolFrame *) w->e.model)->segs[3],
                          floadGetSetFile(0x125, 0));
            w->e.behave = 4;
            w->fade = 0;
            w->alpha = 0;
            w->e.state = 4;
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 4:
                    ((s32 (*)(Obj *o)) biwaFireWait)(o);
                    break;
                case 5:
                    ((s32 (*)(Obj *o)) biwaFireLaunch)(o);
                    break;
                case 6:
                    ((s32 (*)(Obj *o)) biwaFireFly)(o);
                    break;
            }
            fadeAnimStep(&w->anim);
            if (w->fade + 32 < 255) {
                w->fade += 32;
            } else {
                w->fade = 255;
            }
            if (w->e.hit & 0xE) {
                w->e.behave |= 0x4000000;
            }
            if (eneAfterProc(o) == -1) {
                DPRINTF("Etsk_biwa_fire: 死亡\n");
                return -1;
            }
            /* the bob, amplitude 1.0 (a double, as kumo's "sinf(phase) * 0.01 + 0.11"): the game
             * adds in double (cvt.d.s both, add.d) and uopt folds the "* 1.0" */
            ((PolFrame *) w->e.model)->pos[1] = sinf((w->slot * 20 + gGameFrameCount) / 5.0f) * 1.0 + w->e.y;
        case 3:
            er = (u8) LERP(s0, e0, w->alpha);
            eg = (u8) LERP(s1, e1, w->alpha);
            eb = (u8) LERP(s2, e2, w->alpha);
            pr = (u8) LERP(0.0f, e3, w->alpha);
            pg = (u8) LERP(0.0f, e4, w->alpha);
            pb = (u8) LERP(s5, e5, w->alpha);
            ((PolFrame *) w->e.model)->dl = eneDlEnvPrimColor(o, er, eg, eb, pr, pg, pb, w->fade);
            break;
        case 1:
            if (w->e.model != NULL) {
                frmDestroy(w->e.model);
            }
            if (w->e.atk != NULL) {
                frmDestroy(&w->e.atk->frame);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern f32 biwaPartPlaces[][3]; /* the parts' places on the biwa */

/* A biwa part's step: it goes to state 5 once a player is within 20 of its parent, and keeps
 * its place on the parent by its slot. */
void biwaFireWait(Obj *o) {
    BiwaPartW *w = (BiwaPartW *) FX(o);

    if (enePlInRangeCheck(o->parent, 20.0f, 0)) {
        w->e.state = 5;
    }
    w->e.x = biwaPartPlaces[w->slot][0] + ENE(o->parent)->x;
    w->e.y = biwaPartPlaces[w->slot][1] + ENE(o->parent)->y;
    w->e.z = biwaPartPlaces[w->slot][2] + ENE(o->parent)->z;
}

/* A biwa part fading in by 16 a frame. At full alpha it gets an attack box (kind 0x7B, radius
 * 30) and flies off away from the nearest player (1.8 a frame, sinking 1.8), marks its parent
 * and goes to state 6. It keeps its place on the parent by its slot. */
void biwaFireLaunch(Obj *o) {
    BiwaPartW *w = (BiwaPartW *) FX(o);
    s32 pad[2];

    if (((BiwaPartW *) FX(o))->alpha + 16 < 255) {
        w->alpha += 16;
    } else {
        w->alpha = 255;
        if (w->e.atk == NULL) {
            w->e.atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, w->e.model, 30, 0, 0, 0);
            if (w->e.atk != NULL) {
                w->e.atk->kind = 0x7B;
            }
        }
        w->e.vel[0] = (eneNearestPlWay(o->parent) == 4) ? -1.8 : 1.8;
        w->e.vel[1] = -1.8f;
        ((BiwaW *) FX(o->parent))->partFlew = 1;
        w->e.state = 6;
    }
    w->e.x = biwaPartPlaces[w->slot][0] + ENE(o->parent)->x;
    w->e.y = biwaPartPlaces[w->slot][1] + ENE(o->parent)->y;
    w->e.z = biwaPartPlaces[w->slot][2] + ENE(o->parent)->z;
}

/* State 6: nothing (the part flies on its speed). */
void biwaFireFly(Obj *o) {
}
