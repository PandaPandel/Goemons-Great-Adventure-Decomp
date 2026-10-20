#include "tlb.h"

/* File 147: "elefish", an electric fish that swims to and fro along the course (type 2) or waits
 * across it (type 3) and, a player near, charges and lets four sparks fly
 * ("elefish_lightning", kind 0x1F7, its own kind: "Etsk_mimi_tama"), then chases him. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 etskElefishAnim[];
extern u8 elefishSwimAnim[];
extern u8 etskElefishHalf0Dl[];
extern u8 etskElefishHalf1Dl[];
extern u8 elefishLightningDl[];
extern u8 elefishLightningGlowDl[];

/* An elefish's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Elefish {
    /* 0x00 */ EneObj e;
    /* 0x4C */ s32 way;       /* along the course 1 or 4, across it 2 or 8 */
    /* 0x50 */ s32 charged;   /* 1: its sparks are out (state 1 winds down) */
    /* 0x54 */ f32 animSpeed; /* state 1's: up to 2 while it charges, down to 0.5 */
    /* 0x58 */ f32 speed;
} Elefish;

#define ELEFISH(obj) ((Elefish *) FX(obj))

/* A spark's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct EleSpark {
    /* 0x00 */ EneObj e;
    /* 0x4C */ PolFrame *glow;
} EleSpark;

#define ELESPARK(obj) ((EleSpark *) FX(obj))

void elefishSwim(Obj *o);
void elefishCharge(Obj *o);
void elefishWait(Obj *o);
void elefishChase(Obj *o);

/* "elefish". */
s32 Etsk_elefish_main(Obj *o, s16 msg, void *arg) {
    Elefish *w = ELEFISH(o);
    s32 pad;
    s32 type;

    switch (msg) {
        case 2:
            o->name = "elefish";
            type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewAnime(o, (u32) etskElefishAnim)) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 35, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 80, 70, -35, 0, 0)) == NULL) {
                return -1;
            }
            w->e.halves[0] = (u32) etskElefishHalf0Dl;
            w->e.halves[1] = (u32) etskElefishHalf1Dl;
            if (type == 2) {
                w->e.behave = 0x404030;
                w->way = eneNearestPlWay(o);
                w->e.state = 0;
            } else if (type == 3) {
                w->e.behave = 0x400020;
                if (w->e.z < 0.0f) {
                    w->way = 8;
                } else if (0.0f < w->e.z) {
                    w->way = 2;
                } else {
                    w->way = eneNearestPlWay(o);
                }
                w->e.state = 2;
            }
            w->e.rot[1] = eneCourseAngle(o, w->way);
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*elefishSwim)(o);
                    break;
                case 1:
                    (*elefishCharge)(o);
                    break;
                case 2:
                    (*elefishWait)(o);
                    break;
                case 3:
                    (*elefishChase)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_elefish_main: 死亡\n");
                return -1;
            }
            break;
        case 1:
            /* each test and its call on one line (IDO schedules by line; tlb_0084's form) */
            if (w->e.model != NULL) { frmDestroy(w->e.model); }
            if (w->e.atk != NULL) { frmDestroy(&w->e.atk->frame); }
            if (w->e.dmg != NULL) { frmDestroy(&w->e.dmg->frame); }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* State 0: swims to and fro, slowing; a player within 15 turns it round, one within 45 makes it
 * charge (1). */
void elefishSwim(Obj *o) {
    Elefish *w = ELEFISH(o);
    s32 pad[2];

    if (0.0f < w->e.vel[0]) {
        w->way = 1;
    } else if (w->e.vel[0] < 0.0f) {
        w->way = 4;
    }
    if (w->speed - 0.05 > 0.05) {
        w->speed = w->speed - 0.05;
    } else if (eneNearestPlInX(o, 15.0f)) {
        w->speed = 2.0f;
        if (w->way == 1) {
            w->way = 4;
        } else {
            w->way = 1;
        }
    } else if (eneNearestPlInX(o, 45.0f)) {
        ((AnimeFrame *) w->e.model)->anim = elefishSwimAnim;
        ((AnimeFrame *) w->e.model)->frame = 0.0f;
        w->e.state = 1;
    } else {
        w->speed = 2.0f;
    }
    if (w->way == 1) {
        w->e.vel[0] = w->speed;
    } else {
        w->e.vel[0] = -w->speed;
    }
    eneAnimLoop(o, (AnimeFrame *) w->e.model, w->speed + 1.0f);
}

/* State 1: charges (its animation speeding up to 2), lets four sparks fly, winds down and swims
 * on (0). */
void elefishCharge(Obj *o) {
    Elefish *w = ELEFISH(o);
    s32 i;
    Obj *spark;

    if (w->charged == 0) {
        if ((w->animSpeed += 0.05) < 2.0) {
            w->animSpeed += 0.05;
        } else {
            w->animSpeed = 2.0f;
            w->charged = 1;
            for (i = 1; i < 5; i++) {
                spark = eneMakeEx(NULL, ENE_ELEFISH_LIGHTNING, w->e.zone, w->e.x, w->e.y, w->e.z, 2,
                                  (u32 (*)(void *, void *, void *)) i);
            }
            if (spark != NULL) {
                eneSetSd(o, 0x157);
            }
        }
        eneAnimLoop(o, (AnimeFrame *) w->e.model, w->animSpeed);
    } else if ((w->animSpeed -= 0.05) > 0.5) {
        w->animSpeed -= 0.05;
        eneAnimLoop(o, (AnimeFrame *) w->e.model, w->animSpeed);
    } else {
        w->animSpeed = 0.5f;
        if (eneAnimPlayOnce(o, (AnimeFrame *) w->e.model, w->animSpeed)) {
            ((AnimeFrame *) w->e.model)->anim = etskElefishAnim;
            ((AnimeFrame *) w->e.model)->frame = 0.0f;
            w->e.state = 0;
            w->charged = 0;
            w->speed = 2.0f;
        }
    }
}

/* State 2: waits across the course for a player within 50 (120 on the course's line), then
 * chases (3). */
void elefishWait(Obj *o) {
    Elefish *w = ELEFISH(o);
    s32 pad[2];

    eneAnimLoop(o, (AnimeFrame *) w->e.model, w->speed + 1.0f);
    if (w->e.z != 0.0f) {
        if (eneNearestPlInX(o, 50.0f)) {
            w->e.state = 3;
        }
    } else {
        if (eneNearestPlInX(o, 120.0f)) {
            w->e.state = 3;
        }
    }
}

/* State 3: chases, speeding up to 2.5, rising or sinking to the nearest player's height,
 * bubbling. */
void elefishChase(Obj *o) {
    Elefish *w = ELEFISH(o);
    s32 pad[2];
    s32 n;
    s32 zone;
    f32 place;
    f32 height;
    f32 side;

    if (w->e.hit & 0xE) {
        w->e.behave |= 0x4000000;
    }
    w->speed += 0.2;
    if (w->speed >= 2.5) {
        w->speed = 2.5f;
    }
    if (w->way == 8) {
        w->e.vel[2] = w->speed;
    } else if (w->way == 2) {
        w->e.vel[2] = -w->speed;
    } else {
        if (w->way == 1) {
            w->e.vel[0] = w->speed;
        } else {
            w->e.vel[0] = -w->speed;
        }
        if ((n = eneNearestPl(o)) != 0) {
            if (plIsInPlay(n - 1)) {
                plAskPos(gGameWork.players[n - 1], &place, &height, &side, &zone);
                if (height < w->e.y) {
                    w->e.y -= 0.1;
                } else if (w->e.y < height) {
                    w->e.y += 0.1;
                }
            }
        }
    }
    w->e.rot[1] = eneCourseAngle(o, w->way);
    eneAnimLoop(o, (AnimeFrame *) w->e.model, w->speed + 1.0f);
    if (gGameFrameCount & 1) {
        itexPuffWhite(((AnimeFrame *) w->e.model)->pos[0], ((AnimeFrame *) w->e.model)->pos[1],
                      ((AnimeFrame *) w->e.model)->pos[2], 0.03f);
    }
}

/* "elefish_lightning": a spark, flying off one of four ways, slowing, gone when it stops or
 * hits a wall. */
s32 elefishLightningProc(Obj *o, s16 msg, void *arg) {
    EleSpark *w = ELESPARK(o);
    s32 pad;
    s32 type;
    Vec3f v;

    switch (msg) {
        case 2:
            o->name = "elefish_lightning";
            type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewPol(o, (Gfx *) elefishLightningDl)) == NULL) {
                return -1;
            }
            ((PolFrame *) w->e.model)->flags |= 3;
            if ((w->glow = enePolModelMake(o, (Gfx *) elefishLightningGlowDl, NULL, 1)) == NULL) {
                return -1;
            }
            w->glow->flags |= 3;
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 25, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 123;
            switch (type) {
                case 1:
                    w->e.vel[0] = 3.0f;
                    w->e.vel[1] = 3.0f;
                    w->e.rot[2] -= 0.7853981853f;
                    break;
                case 2:
                    w->e.vel[1] = -3.0f;
                    w->e.vel[0] = 3.0f;
                    w->e.rot[2] += 0.7853981853f;
                    break;
                case 3:
                    w->e.vel[0] = -3.0f;
                    w->e.vel[1] = -3.0f;
                    w->e.rot[2] -= 0.7853981853f;
                    break;
                case 4:
                    w->e.vel[0] = -3.0f;
                    w->e.vel[1] = 3.0f;
                    w->e.rot[2] += 0.7853981853f;
                    break;
            }
            v.x = w->e.vel[0];
            v.y = w->e.vel[1];
            v.z = w->e.vel[2];
            w->e.atk->info = *(ACollInfo *) &v;
            w->e.behave = 4;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            if (gGameFrameCount & 1) {
                ((PolFrame *) w->e.model)->dl2 = NULL;
            } else {
                ((PolFrame *) w->e.model)->dl2 = (Gfx *) elefishLightningDl;
            }
            w->e.vel[0] *= 0.9;
            w->e.vel[1] *= 0.9;
            if (((w->e.vel[0] > 0.0f) ? w->e.vel[0] : -w->e.vel[0]) < 0.02 &&
                ((w->e.vel[0] > 0.0f) ? w->e.vel[0] : -w->e.vel[0]) < 0.02) {
                w->e.behave |= 0x40000;
            }
            if (w->e.hit & 6) {
                w->e.behave |= 0x40000;
            } else if (w->e.hit & 8) {
                w->e.behave |= 0x4000000;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_mimi_tama: 死亡\n");
                return -1;
            }
            if (w->glow != NULL) {
                eneSetModelOnCourse(o, &w->glow->f);
            }
            w->glow->pos[0] = ((PolFrame *) w->e.model)->pos[0];
            w->glow->pos[1] = ((PolFrame *) w->e.model)->pos[1];
            w->glow->pos[2] = ((PolFrame *) w->e.model)->pos[2];
            /* fallthrough */
        case 3:
            w->glow->dl = eneDlEnvPrimColor(o, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0, 0x3F);
            break;
        case 1:
            if (w->e.model != NULL) {
                frmDestroy(w->e.model);
            }
            if (w->glow != NULL) {
                frmDestroy(&w->glow->f);
            }
            if (w->e.atk != NULL) {
                frmDestroy(&w->e.atk->frame);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
