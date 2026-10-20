#include "tlb.h"

/* The "wetwoman" enemy kind (archive file 90): a woman who turns to face the player, lunges
 * along x when one comes near and turns back at a wall. Its placement's type picks how it starts
 * (state): 1 standing (0), 2 walking (6), 3 tipped over (rot x 90 degrees) and raised 70, rising
 * when a player is within 80 (8), 4 tipped over, dropping when a player is within 120 along x
 * (11). A hit flashes it (the env colour) and knocks it back (5). */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 etskWetwomanAnim[];
extern u8 etskWetwomanWalkAnim[];
extern u8 etskWetwomanState13Anim[];
extern u8 etskWetwomanHalf0Dl[];
extern u8 etskWetwomanHalf1Dl[];

/* A wetwoman's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Wetwoman {
    /* 0x00 */ EneObj e;
    /* 0x4C */ u8 way; /* the way it faces along x: 1 or 4 (enePlDirX) */
    /* 0x4D */ u8 unused4D; /* written 4 at its start, never read */
    /* 0x4E */ s8 flash; /* frames left of the hit flash */
    /* 0x4F */ u8 prev; /* its state when it was hit */
} Wetwoman;

#define WET(obj) ((Wetwoman *) FX(obj))
#define MODEL(w) ((AnimeFrame *) (w)->e.model)

void wetwomanStand(Obj *o);
void wetwomanTurn(Obj *o);
void wetwomanWalk(Obj *o);
void wetwomanLunge(Obj *o);
void wetwomanWalkToWall(Obj *o);
void wetwomanKnockback(Obj *o);
void wetwomanChase(Obj *o);
void wetwomanChaseTurn(Obj *o);
void wetwomanTippedWait(Obj *o);
void wetwomanRise(Obj *o);
void wetwomanCrawl(Obj *o);
void wetwomanDropWait(Obj *o);
void wetwomanFall(Obj *o);
void wetwomanLandRise(Obj *o);

s32 Etsk_wetwoman_main(Obj *o, s16 msg, void *arg) {
    Wetwoman *w = WET(o);
    ACollFrame *a;
    s32 pad;
    s32 type;

    switch (msg) {
        case 2:
            o->name = "wetwoman";
            type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneAnimeModelMake(o, (u32) etskWetwomanAnim, NULL, 0)) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 50, 200, 125, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((a = ATK_ADD_YZ((Frame *) w->e.atk, w->e.model, 50, 50, 50, 0)) == NULL) {
                return -1;
            }
            a->kind = 120;
            if ((a = ATK_ADD_YZ((Frame *) w->e.atk, w->e.model, 30, 15, -120, 0)) == NULL) {
                return -1;
            }
            a->kind = 120;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 125, 250, 0, 65, 0)) == NULL) {
                return -1;
            }
            w->e.halves[0] = (u32) etskWetwomanHalf0Dl;
            w->e.halves[1] = (u32) etskWetwomanHalf1Dl;
            w->unused4D = 4;
            w->flash = 0;
            if (type == 1) {
                if (eneSetOnEarth(o) == -1) {
                    return -1;
                }
                w->e.behave = 0x8C00282;
                w->e.hp = 3;
                w->e.state = 0;
            } else if (type == 2) {
                MODEL(w)->scale[2] = 0.06f;
                MODEL(w)->scale[0] = MODEL(w)->scale[1] = MODEL(w)->scale[2];
                w->e.behave = 0x400A80;
                w->way = enePlDirX(o, 0);
                w->e.vel[0] = (w->way == 4) ? -0.6 : 0.6;
                w->e.rot[1] = eneCourseAngle(o, w->way);
                w->e.state = 6;
            } else if (type == 3) {
                MODEL(w)->scale[2] = 0.06f;
                MODEL(w)->scale[0] = MODEL(w)->scale[1] = MODEL(w)->scale[2];
                w->e.behave = 0x400280;
                w->e.y += 70.0f;
                w->e.rot[0] = 1.570796371f;
                w->e.rot[1] = eneCourseAngle(o, 8);
                w->e.state = 8;
            } else if (type == 4) {
                MODEL(w)->scale[2] = 0.06f;
                MODEL(w)->scale[0] = MODEL(w)->scale[1] = MODEL(w)->scale[2];
                w->e.behave = 0x400280;
                w->e.rot[0] = 1.570796371f;
                w->e.rot[1] = eneCourseAngle(o, 8);
                w->e.state = 11;
            }
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*wetwomanStand)(o);
                    break;
                case 1:
                    (*wetwomanTurn)(o);
                    break;
                case 2:
                    (*wetwomanWalk)(o);
                    break;
                case 3:
                    (*wetwomanLunge)(o);
                    break;
                case 4:
                    (*wetwomanWalkToWall)(o);
                    break;
                case 5:
                    (*wetwomanKnockback)(o);
                    break;
                case 6:
                    (*wetwomanChase)(o);
                    break;
                case 7:
                    (*wetwomanChaseTurn)(o);
                    break;
                case 8:
                    (*wetwomanTippedWait)(o);
                    break;
                case 9:
                    (*wetwomanRise)(o);
                    break;
                case 10:
                    (*wetwomanCrawl)(o);
                    break;
                case 11:
                    (*wetwomanDropWait)(o);
                    break;
                case 12:
                    (*wetwomanFall)(o);
                    break;
                case 13:
                    (*wetwomanLandRise)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_wetwoman_main: 死亡\n");
                return -1;
            }
            if (w->e.hit & 0x10) {
                MODEL(w)->anim = etskWetwomanState13Anim;
                MODEL(w)->frame = 0.0f;
                w->e.dmg->flags &= ~2;
                w->flash = 15;
                w->e.vel[0] = eneKnockbackSpeed(o, w->e.dmg, 4.0f);
                w->prev = w->e.state;
                w->e.state = 5;
            }
            /* fallthrough */
        case 3:
            if (w->flash != 0) {
                MODEL(w)->dl = eneDlEnvColorLit(o, (w->flash & 2) ? 0x80 : 0, 0xFF, 0xFF, 0xFF,
                                                (w->flash & 2) ? 0 : 1);
            } else {
                MODEL(w)->dl = eneDlEnvColorLit(o, 0, 0xFF, 0xFF, 0xFF, 1);
            }
            break;
        case 1:
            if (w->e.model != NULL) {
                frmDestroy(w->e.model);
            }
            if (w->e.atk != NULL) {
                frmDestroy(&w->e.atk->frame);
            }
            if (w->e.dmg != NULL) {
                frmDestroy(&w->e.dmg->frame);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* State 0: stands until a player is within 70, then turns to it. */
void wetwomanStand(Obj *o) {
    Wetwoman *w = WET(o);

    eneAnimeLoopLen(o, MODEL(WET(o)), 0.6f, 18.0f);
    if (enePlInRangeCheck(o, 70.0f, 0)) {
        w->way = enePlDirX(o, 0);
        w->e.state = 1;
    }
}

/* State 1: turns to face its way; the closer to the course's angle, the faster it walks. */
void wetwomanTurn(Obj *o) {
    Wetwoman *w = WET(o);
    s32 pad[3];
    s32 done;
    f32 d;

    eneAnimeLoopLen(o, MODEL(WET(o)), 0.6f, 18.0f);
    done = eneTurnRotYTo(o, eneCourseAngle(o, w->way), (w->way == 4) ? -0.1 : 0.1);
    d = (w->e.rot[1] < eneCourseAngle(o, 8)) ? eneCourseAngle(o, 8) - w->e.rot[1]
                                             : -(eneCourseAngle(o, 8) - w->e.rot[1]);
    if (w->way == 4) {
        w->e.vel[0] = -(f32) (d / 1.570796371f * 1.0);
    } else {
        w->e.vel[0] = d / 1.570796371f * 1.0;
    }
    if (!done) {
        w->e.state = 2;
    }
}

/* State 2: walks; when a loop ends it lunges if the player is still ahead, else turns. */
void wetwomanWalk(Obj *o) {
    Wetwoman *w = WET(o);
    s32 pad[2];
    s32 dir = enePlDirX(o, 0);

    if (eneAnimeLoopLen(o, MODEL(w), 0.6f, 18.0f)) {
        if (dir == w->way) {
            MODEL(w)->anim = etskWetwomanWalkAnim;
            MODEL(w)->frame = 0.0f;
            w->e.state = 3;
            w->e.vel[0] = 0.0f;
        } else {
            w->e.state = 4;
        }
    }
}

/* State 3: the lunge: fast from frame 8, stopped from frame 10. */
void wetwomanLunge(Obj *o) {
    Wetwoman *w = WET(o);
    s32 end = eneAnimLoop(o, MODEL(WET(o)), 0.4f);

    if (MODEL(w)->frame >= 8.0) {
        w->e.vel[0] = (w->way == 4) ? -5.0 : 5.0;
    }
    if (MODEL(w)->frame >= 10.0) {
        w->e.vel[0] = 0.0f;
    }
    if (end) {
        MODEL(w)->anim = etskWetwomanAnim;
        MODEL(w)->frame = 0.0f;
        w->e.vel[0] = (w->way == 4) ? -1.0 : 1.0;
        w->e.state = 4;
    }
}

/* State 4: walks on until it hits a wall, then turns back. */
void wetwomanWalkToWall(Obj *o) {
    Wetwoman *w = WET(o);

    eneAnimeLoopLen(o, MODEL(WET(o)), 0.6f, 18.0f);
    if (w->e.hit & 6) {
        if (w->way == 4) {
            w->way = 1;
        } else {
            w->way = 4;
        }
        w->e.vel[0] = 0.0f;
        w->e.state = 1;
    }
}

/* State 5: knocked back; slows down, and when the flash is over it turns to the player. */
void wetwomanKnockback(Obj *o) {
    Wetwoman *w = WET(o);

    if (WET(o)->e.vel[0] < 0.0f) {
        w->e.vel[0] += 0.285714285714285698;
    } else if (w->e.vel[0] > 0.0f) {
        w->e.vel[0] -= 0.285714285714285698;
    }
    eneAnimLoop(o, MODEL(w), (polAnimeLastFrame(MODEL(w)) + 1) / 15.0f);
    w->flash--;
    if (w->flash <= 0) {
        w->flash = 0;
        w->e.dmg->flags |= 2;
        MODEL(w)->anim = etskWetwomanAnim;
        MODEL(w)->frame = 0.0f;
        w->way = enePlDirX(o, 0);
        w->e.state = 1;
    }
}

/* State 6: walks (attacking) until it hits a wall, then turns back. */
void wetwomanChase(Obj *o) {
    Wetwoman *w = WET(o);

    WET(o)->e.behave |= 0x4000;
    eneAnimeLoopLen(o, MODEL(WET(o)), 1.0f, 18.0f);
    if (w->e.hit & 6) {
        w->e.behave &= ~0x4000;
        if (w->way == 4) {
            w->way = 1;
        } else {
            w->way = 4;
        }
        w->e.state = 7;
    }
}

/* State 7: turns back, walking at 0.9 of state 1's pace. */
void wetwomanChaseTurn(Obj *o) {
    Wetwoman *w = WET(o);
    s32 pad[4];
    s32 done;
    f32 d;

    eneAnimeLoopLen(o, MODEL(WET(o)), 1.0f, 18.0f);
    done = eneTurnRotYTo(o, eneCourseAngle(o, w->way), (w->way == 4) ? -0.1 : 0.1);
    d = (w->e.rot[1] < eneCourseAngle(o, 8)) ? eneCourseAngle(o, 8) - w->e.rot[1]
                                             : -(eneCourseAngle(o, 8) - w->e.rot[1]);
    if (w->way == 4) {
        w->e.vel[0] = -(f32) (d / 1.570796371f * 0.9);
    } else {
        w->e.vel[0] = d / 1.570796371f * 0.9;
    }
    if (!done) {
        w->e.state = 6;
    }
}

/* State 8: lies until a player is within 80, then gets up. */
void wetwomanTippedWait(Obj *o) {
    Wetwoman *w = WET(o);
    s32 pad[2];

    eneAnimeLoopLen(o, MODEL(WET(o)), 0.8f, 18.0f);
    if (enePlInRangeCheck(o, 80.0f, 0)) {
        w->e.behave |= 0x800000;
        w->way = enePlDirX(o, 0);
        w->e.state = 9;
    }
}

/* State 9: rises (rot x back to 0); once on the ground, turns to its way. */
void wetwomanRise(Obj *o) {
    Wetwoman *w = WET(o);
    s32 pad[3];
    f32 d;
    s32 done;

    w->e.rot[0] -= 0.08;
    if (w->e.rot[0] < 0.0f) {
        w->e.rot[0] = 0.0f;
    }
    eneAnimeLoopLen(o, MODEL(w), 0.8f, 18.0f);
    if (w->e.hit & 1) {
        done = eneTurnRotYTo(o, eneCourseAngle(o, w->way), (w->way == 4) ? -0.1 : 0.1);
        if (w->e.rot[1] < eneCourseAngle(o, 8)) {
            d = eneCourseAngle(o, 8) - w->e.rot[1];
        } else {
            d = -(eneCourseAngle(o, 8) - w->e.rot[1]);
        }
        if (w->way == 4) {
            w->e.vel[0] = -(f32) (d / 1.570796371f * 0.6);
        } else {
            w->e.vel[0] = d / 1.570796371f * 0.6;
        }
        if (!done) {
            w->e.state = 10;
        }
    }
}

/* State 10: walks (attacking) until it hits a wall, then turns back to state 9. */
void wetwomanCrawl(Obj *o) {
    Wetwoman *w = WET(o);

    WET(o)->e.behave |= 0x4000;
    eneAnimeLoopLen(o, MODEL(WET(o)), 0.8f, 18.0f);
    if (w->e.hit & 6) {
        w->e.behave &= ~0x4000;
        if (w->way == 4) {
            w->way = 1;
        } else {
            w->way = 4;
        }
        w->e.state = 9;
    }
}

/* State 11: lies until a player is within 120 along x, then jumps down. */
void wetwomanDropWait(Obj *o) {
    Wetwoman *w = WET(o);

    eneAnimeLoopLen(o, MODEL(WET(o)), 0.8f, 18.0f);
    if (eneNearestPlInX(o, 120.0f)) {
        w->way = enePlDirX(o, 0);
        w->e.state = 12;
        w->e.vel[1] = -0.5f;
    }
}

/* State 12: falls until it lands. */
void wetwomanFall(Obj *o) {
    Wetwoman *w = WET(o);

    eneAnimeLoopLen(o, MODEL(WET(o)), 0.8f, 18.0f);
    if (w->e.hit & 1) {
        w->e.state = 13;
    }
}

/* State 13: rises (rot x back to 0), then turns to its way. */
void wetwomanLandRise(Obj *o) {
    Wetwoman *w = WET(o);
    s32 pad[4];
    s32 done;
    f32 d;

    eneAnimeLoopLen(o, MODEL(WET(o)), 0.8f, 18.0f);
    w->e.rot[0] -= 0.08;
    if (w->e.rot[0] <= 0.0f) {
        w->e.rot[0] = 0.0f;
        done = eneTurnRotYTo(o, eneCourseAngle(o, w->way), (w->way == 4) ? -0.1 : 0.1);
        d = (w->e.rot[1] < eneCourseAngle(o, 8)) ? eneCourseAngle(o, 8) - w->e.rot[1]
                                                 : -(eneCourseAngle(o, 8) - w->e.rot[1]);
        if (w->way == 4) {
            w->e.vel[0] = -(f32) (d / 1.570796371f * 0.6);
        } else {
            w->e.vel[0] = d / 1.570796371f * 0.6;
        }
        if (!done) {
            w->e.state = 10;
        }
    }
}
