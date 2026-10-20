#include "tlb.h"

/* The hopping umbrella ghost (archive file 88): "kasa_j" waits at its placement and, once a
 * player is within 100, makes one "kasa" (kind 0xDC) there with its own type. A kasa drops in
 * opening (state 0), lands (1), then by its type hops left (1), faster left (3), in place (4) or
 * right (5) (state 2), or swings to and fro along x (type 2, state 3). */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

/* tlb.h has eneAfterProc's far-call macro without its declaration. */
#define sinf (*sinf)

extern u8 etskKasaType1Anim[];
extern u8 etskKasaDropAnim[];
extern u8 etskKasaType3Anim[];
extern u8 etskKasaType2Anim[];
extern u8 etskKasaHalf0Dl[];
extern u8 etskKasaHalf1Dl[];

/* A kasa_j's work, at its task + 0x20. */
typedef struct KasaJ {
    /* 0x00 */ EneObj e;
    /* 0x4C */ u8 pad4C[4];
    /* 0x50 */ s8 made; /* its kasa is made */
    /* 0x51 */ u8 type; /* its placement's type, passed on to the kasa */
} KasaJ;

/* A kasa's work, at its task + 0x20. */
typedef struct Kasa {
    /* 0x00 */ EneObj e;
    /* 0x4C */ u16 count; /* frames hopping along (types 1, 3 and 5) */
    /* 0x4E */ s8 dir; /* the swing's way: 1 or -1 */
    /* 0x4F */ s8 type; /* its placement's type */
    /* 0x50 */ f32 homeX;
    /* 0x54 */ f32 swingX;
    /* 0x58 */ f32 phase; /* the swing's */
} Kasa;

#define KASAJ(obj) ((KasaJ *) FX(obj))
#define KASA(obj) ((Kasa *) FX(obj))
#define MODEL(w) ((AnimeFrame *) (w)->e.model)

void kasaDrop(Obj *o); /* kasaDrop */
void kasaLand(Obj *o); /* kasaLand */
void kasaHop(Obj *o); /* kasaHop */
void kasaSwing(Obj *o); /* kasaSwing */

/* kasaJProc */
s32 kasaJProc(Obj *o, s16 msg, void *arg) {
    KasaJ *w;
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "kasa_j";
            w = KASAJ(o);
            w->type = eneSetup(o, arg);
            w->made = 0;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            w = KASAJ(o);
            if (enePlInRangeCheck(o, 100.0f, 0) && w->made == 0) {
                eneMakeEx(NULL, ENE_ETSK_KASA_MAIN_HOURS_ANY, w->e.zone, w->e.x, w->e.y, w->e.z, 2,
                          (u32 (*)(void *, void *, void *)) w->type);
                w->made = 1;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_kasa_j: 死亡\n");
                return -1;
            }
            break;
        case 1:
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Etsk_kasa_main */
s32 Etsk_kasa_main(Obj *o, s16 msg, void *arg) {
    Kasa *w = KASA(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "kasa";
            w->type = eneSetup(o, arg);
            if (w->type == 1 || w->type == 5) {
                w->e.model = (Frame *) eneNewAnime(o, (u32) etskKasaType1Anim);
            } else if (w->type == 2) {
                w->e.model = (Frame *) eneNewAnime(o, (u32) etskKasaType2Anim);
            } else if (w->type == 3 || w->type == 4) {
                w->e.model = (Frame *) eneNewAnime(o, (u32) etskKasaType3Anim);
            }
            if (w->e.model == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 40, 80, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 75, 240, 0, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->e.shadow = (Obj *) shadowCreate()) == NULL) {
                return -1;
            }
            itexPuffWhite(MODEL(w)->pos[0], MODEL(w)->pos[1], MODEL(w)->pos[2], 0.1f);
            w->e.halves[0] = (u32) etskKasaHalf0Dl;
            w->e.halves[1] = (u32) etskKasaHalf1Dl;
            w->e.behave = 0x4000;
            w->e.rot[1] = eneCourseAngle(o, 4);
            w->homeX = w->e.x;
            w->phase = 0.0f;
            if (w->type == 1) {
                w->e.state = 0;
            } else if (w->type == 2) {
                w->dir = 1;
                w->e.state = 3;
            } else if (w->type == 3) {
                w->e.state = 2;
            } else if (w->type == 5) {
                w->e.rot[1] = eneCourseAngle(o, 1);
                w->e.state = 0;
            }
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (KASA(o)->e.state) {
                case 0:
                    (*kasaDrop)(o);
                    break;
                case 1:
                    (*kasaLand)(o);
                    break;
                case 2:
                    (*kasaHop)(o);
                    break;
                case 3:
                    (*kasaSwing)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_kasa_main: 死亡\n");
                return -1;
            }
            break;
        case 1:
            if (w->e.model != NULL) {
                frmDestroy(w->e.model);
            }
            if (w->e.shadow != NULL) {
                frmDestroy((Frame *) w->e.shadow);
            }
            if (w->e.atk != NULL) {
                frmDestroy(&w->e.atk->frame);
            }
            if (w->e.dmg != NULL) {
                frmDestroy(&w->e.dmg->frame);
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* kasaDrop: state 0, drops 50 while its opening runs to frame 9. */
void kasaDrop(Obj *o) {
    Kasa *w = KASA(o);
    f32 step = 0.4f;

    w->e.vel[1] = -50.0f / (9.0f / step);
    MODEL(w)->frame += step;
    if (MODEL(w)->frame >= 9.0) {
        w->e.state = 1;
        MODEL(w)->anim = etskKasaDropAnim;
        MODEL(w)->frame = 0.5f;
    }
}

/* kasaLand: state 1, plays the landing once, then hops (state 2). */
void kasaLand(Obj *o) {
    Kasa *w = KASA(o);

    if (eneAnimPlayOnce(o, MODEL(w), 0.5f) == 1) {
        w->e.state = 2;
        MODEL(w)->anim = etskKasaType3Anim;
        MODEL(w)->frame = 0.0f;
        w->count = 0;
    }
}

/* kasaHop: state 2, hops (a sound and a jump at frame 1 of each loop), moving by its type
 * while in the air; after 176 frames of moving hops it is let go (behave 0x100000). */
void kasaHop(Obj *o) {
    Kasa *w = KASA(o);
    f32 step = 0.75f;
    s32 pad[2];

    eneAnimLoop(o, MODEL(w), step);
    if (MODEL(w)->frame > 1.0 && MODEL(w)->frame <= 1.0 + step) {
        eneSetSd(o, 0x239);
        if (w->e.hit & 1) {
            w->e.vel[1] = 4.0f;
        }
    }
    /* The two (f64) 12.0f: as a plain 12.0 IDO shares one register for the 12.0 of these
     * tests and the fall's; the game builds each 12.0 afresh. */
    if (MODEL(w)->frame > 1.0 && MODEL(w)->frame <= (f64) 12.0f) {
        if (w->type == 1) {
            w->e.vel[0] = -1.0f;
        } else if (w->type == 3) {
            w->e.vel[0] = -1.3f;
        } else if (w->type == 4) {
            w->e.vel[0] = 0.0f;
        } else if (w->type == 5) {
            w->e.vel[0] = 1.0f;
        }
    } else {
        if (w->type == 1 || w->type == 3) {
            w->e.vel[0] = -0.1f;
        } else if (w->type == 5) {
            w->e.vel[0] = 0.1f;
        }
    }
    if (MODEL(w)->frame > (f64) 12.0f && !(w->e.hit & 1)) {
        MODEL(w)->frame = 12.0f;
    }
    if (w->type == 1 || w->type == 3 || w->type == 5) {
        if (++w->count >= 176) {
            w->e.behave |= 0x100000;
        }
    }
    eneFall(o, 8.0 / (12.0 / step));
}

/* kasaSwing: state 3 (type 2), swings to and fro along x about its home, its animation
 * played forward and back. */
void kasaSwing(Obj *o) {
    Kasa *w = KASA(o);
    f32 speed = w->dir * 0.8;
    s32 pad;

    if (eneAnimPlayOnce(o, MODEL(w), speed) == 1) {
        w->dir = -w->dir;
    }
    w->e.vel[1] = -0.5f;
    w->swingX = w->homeX - sinf(w->phase) * 20.0f;
    w->e.x = w->swingX - (MODEL(w)->frame - 4.5) * 0.8;
    w->phase += 0.075;
    if (w->e.hit & 1) {
        w->e.behave |= 0x100000;
    }
}
