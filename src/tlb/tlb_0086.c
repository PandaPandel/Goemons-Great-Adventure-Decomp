#include "tlb.h"

/* The "kappa" enemy kind (archive file 86), and "jenel", its generator: a jenel puts out a kappa
 * (kind 0xCE) on its first step and every 91 steps after, passing on its own type. A kappa's
 * placement type picks how it acts (its states, kappaRise on): 1 rises out of the water and
 * leaps across, biting, and is gone; 6 does the same over and over (states 12-15); 3 waits on
 * the bank for a player, leaps and walks off; 4 and 5 wait under the bank, jump out and hop
 * along it; 8 climbs to the course and runs; 9 chases the nearest player; 10 and 11 circle
 * their home. Types 1 and 6 splash as they fall back into the water. */

#define sinf (*sinf)
#define cosf (*cosf)

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 etskKappaType3Anim[];
extern u8 etskKappaType8Anim[];
extern u8 etskKappaType1Anim[];
extern u8 etskKappaHalf0Dl[];
extern u8 etskKappaHalf1Dl[];

/* A jenel's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Jenel {
    /* 0x00 */ EneObj e;
    /* 0x4C */ u8 pad4C[4];
    /* 0x50 */ s16 time; /* steps since the last kappa */
    /* 0x52 */ u8 type; /* its placement's type, passed on to each kappa */
} Jenel;

/* A kappa's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Kappa {
    /* 0x00 */ EneObj e;
    /* 0x4C */ s16 homeX; /* types 10 and 11: the centre of its circle */
    /* 0x4E */ s16 homeZ;
    /* 0x50 */ s16 homeY; /* the height it jumps from */
    /* 0x52 */ u8 wait; /* steps before its next move */
    /* 0x53 */ u8 type; /* its placement's type, from eneSetup */
    /* 0x54 */ f32 phase; /* an angle: its sway, or its place on the circle */
    /* 0x58 */ f32 speed;
    /* 0x5C */ u8 way; /* the way it walks (1 or 4) */
    /* 0x5D */ u8 splashed; /* its splash is made */
} Kappa;

#define JENEL(obj) ((Jenel *) FX(obj))
#define KAPPA(obj) ((Kappa *) FX(obj))

void kappaRise(Obj *o);
void kappaSwayJump(Obj *o);
void kappaJumpBite(Obj *o);
void kappaBankWait(Obj *o);
void kappaBankWalk(Obj *o);
void kappaBankLeap(Obj *o);
void kappaBankLeave(Obj *o);
void kappaUnderWait(Obj *o);
void kappaCrossJump(Obj *o);
void kappaLand(Obj *o);
void kappaWalk(Obj *o);
void kappaHop(Obj *o);
void kappaWaterRise(Obj *o);
void kappaWaterSway(Obj *o);
void kappaDuckJump(Obj *o);
void kappaJumpTurn(Obj *o);
void kappaClimb(Obj *o);
void kappaRun(Obj *o);
void kappaChase(Obj *o);
void kappaCircle(Obj *o);

s32 kappaJenelProc(Obj *o, s16 msg, void *arg) {
    Jenel *w = JENEL(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "jenel";
            w->type = eneSetup(o, arg);
            w->time = 90;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            w->time++;
            if (w->time > 90) {
                eneMakeEx(NULL, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, w->e.zone, w->e.x, w->e.y, w->e.z, 2,
                          (u32 (*)(void *, void *, void *)) w->type);
                w->time = 0;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_kappa_j: 死亡\n");
                return -1;
            }
            break;
        case 1:
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Its model, an animated one. */
#define MODEL(w) ((AnimeFrame *) (w)->e.model)
/* Its second attack box (its bite), the first's child: live only while it bites. */
#define BITE(w) ((ACollFrame *) (w)->e.atk->frame.child)

s32 Etsk_kappa_main(Obj *o, s16 msg, void *arg) {
    Kappa *w = KAPPA(o);
    s32 pad;
    ACollFrame *box;
    Vec3f pos;
    Vec3f push;

    switch (msg) {
        case 2:
            o->name = "kappa";
            w->type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewAnime(o, 0)) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 40, 80, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((box = ATK_ADD_YZ((Frame *) w->e.atk, w->e.model, 30, 25, 180, 0)) == NULL) {
                return -1;
            }
            box->kind = 120;
            BITE(w)->flags &= ~2;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 65, 130, 0, 0, 0)) == NULL) {
                return -1;
            }
            if (w->type != 6) {
                if ((w->e.shadow = (Obj *) shadowCreate()) == NULL) {
                    return -1;
                }
            }
            if (w->type != 9 && w->type != 10 && w->type != 11 && eneSetOnEarth(o) == -1) {
                return -1;
            }
            w->e.halves[0] = (u32) etskKappaHalf0Dl;
            w->e.halves[1] = (u32) etskKappaHalf1Dl;
            if (w->type == 1) {
                MODEL(w)->anim = etskKappaType1Anim;
                w->e.rot[1] = eneCourseAngle(o, 8);
                w->homeY = w->e.y;
                w->e.state = 0;
                w->e.z -= 30.0f;
            } else if (w->type == 3) {
                MODEL(w)->anim = etskKappaType3Anim;
                w->e.behave = 0x404000;
                w->e.rot[1] = eneCourseAngle(o, 1);
                w->e.state = 3;
            } else if (w->type == 4) {
                w->e.behave = 0x404000;
                w->e.rot[1] = eneCourseAngle(o, 8);
                w->e.y -= 80.0f;
                w->e.state = 7;
                w->e.z -= 50.0f;
            } else if (w->type == 5) {
                w->e.behave = 0x404000;
                w->e.rot[1] = eneCourseAngle(o, 2);
                w->e.y -= 80.0f;
                w->e.state = 7;
                w->e.z += 50.0f;
            } else if (w->type == 6) {
                MODEL(w)->anim = etskKappaType1Anim;
                w->e.behave = 0x400000;
                w->e.rot[1] = eneCourseAngle(o, 8);
                w->homeY = w->e.y;
                w->e.state = 12;
            } else if (w->type == 8) {
                MODEL(w)->anim = etskKappaType8Anim;
                w->e.behave = 0x800;
                w->e.rot[1] = eneCourseAngle(o, 8);
                w->e.state = 16;
            } else if (w->type == 9) {
                MODEL(w)->anim = etskKappaType8Anim;
                w->e.behave = 0x404028;
                w->e.rot[1] = eneCourseAngle(o, eneNearestPlWay(o));
                w->e.state = 18;
            } else if (w->type == 10) {
                MODEL(w)->anim = etskKappaType8Anim;
                w->homeX = w->e.x;
                w->homeZ = w->e.z;
                w->phase = 0.0f;
                w->e.rot[1] = eneCourseAngle(o, 8);
                w->e.behave = 0x400000;
                w->e.state = 19;
            } else if (w->type == 11) {
                MODEL(w)->anim = etskKappaType8Anim;
                MODEL(w)->scale[0] = MODEL(w)->scale[1] = MODEL(w)->scale[2] = 0.05999999866f;
                w->homeX = w->e.x;
                w->homeZ = w->e.z;
                w->phase = -0.5235987902f;
                w->e.rot[1] = eneCourseAngle(o, 8);
                w->e.behave = 0x400000;
                w->e.state = 19;
            } else {
                w->wait = 0;
            }
            w->splashed = 0;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*kappaRise)(o);
                    break;
                case 1:
                    (*kappaSwayJump)(o);
                    break;
                case 2:
                    (*kappaJumpBite)(o);
                    break;
                case 3:
                    (*kappaBankWait)(o);
                    break;
                case 4:
                    (*kappaBankWalk)(o);
                    break;
                case 5:
                    (*kappaBankLeap)(o);
                    break;
                case 6:
                    (*kappaBankLeave)(o);
                    break;
                case 7:
                    (*kappaUnderWait)(o);
                    break;
                case 8:
                    (*kappaCrossJump)(o);
                    break;
                case 9:
                    (*kappaLand)(o);
                    break;
                case 10:
                    (*kappaWalk)(o);
                    break;
                case 11:
                    (*kappaHop)(o);
                    break;
                case 12:
                    (*kappaWaterRise)(o);
                    break;
                case 13:
                    (*kappaWaterSway)(o);
                    break;
                case 14:
                    (*kappaDuckJump)(o);
                    break;
                case 15:
                    (*kappaJumpTurn)(o);
                    break;
                case 16:
                    (*kappaClimb)(o);
                    break;
                case 17:
                    (*kappaRun)(o);
                    break;
                case 18:
                    (*kappaChase)(o);
                    break;
                case 19:
                    (*kappaCircle)(o);
                    break;
            }
            if ((u8 *) MODEL(w)->anim == etskKappaType1Anim) {
                if (MODEL(w)->frame >= 13.0 && MODEL(w)->frame < 15.0) {
                    BITE(w)->flags |= 2;
                } else {
                    BITE(w)->flags &= ~2;
                }
            } else {
                BITE(w)->flags &= ~2;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_kappa_main: 死亡\n");
                return -1;
            }
            if (w->type == 1 || w->type == 6) {
                MODEL(w)->pos[1] = w->e.y - 14.0f;
                if (w->e.vel[1] < 0.0f && w->splashed == 0) {
                    pos.x = MODEL(w)->pos[0];
                    pos.y = MODEL(w)->pos[1];
                    pos.z = MODEL(w)->pos[2];
                    if (rideTree2PointIn(&pos, &push) != 0) {
                        eneSetSd(o, 0x298);
                        effectSibukiCreateWhite(pos.x, pos.y, pos.z, 0.07500000298f, 0.07500000298f,
                                                0.004999999888f, 0.004999999888f);
                        w->splashed = 1;
                    }
                }
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

/* State 0 (types 1): rises out of the water to 9 above its home. */
void kappaRise(Obj *o) {
    Kappa *w = KAPPA(o);

    w->e.y += 3.0f;
    if (w->homeY + 9 <= w->e.y) {
        w->e.y = w->homeY + 9;
        w->wait = 8;
        w->e.state = 1;
        w->phase = 0.0f;
    }
}

/* State 1: sways, then jumps (up 4.5, across to z = 0 by the top). */
void kappaSwayJump(Obj *o) {
    Kappa *w = KAPPA(o);
    s32 pad;

    if (w->wait - 1 >= 0) {
        w->wait--;
        return;
    }
    w->phase += 0.5;
    if (w->phase >= 6.283185482f) {
        w->phase = 6.283185482f;
        MODEL(w)->anim = etskKappaType1Anim;
        MODEL(w)->frame = 0.0f;
        w->e.behave |= 0x800;
        w->e.vel[1] = 4.5f;
        w->e.vel[2] = -(w->e.z * 2) / (f32) ((s32) (w->e.vel[1] / 0.3 * 2) + 0.99);
        w->wait = 0;
        eneSetSd(o, 0x297);
        w->e.state = 2;
    }
    w->e.rot[1] = eneCourseAngle(o, 8) + sinf(w->phase) * 3.141592741f / 4;
}

/* State 2: in the air, biting at frames 13-15 (the main proc); past 0 down it is gone. */
void kappaJumpBite(Obj *o) {
    Kappa *w = KAPPA(o);
    f32 speed = 0.8f;
    s32 pad[7];

    if (MODEL(w)->frame < 14.0) {
        eneAnimPlayOnce(o, MODEL(w), speed);
        if (MODEL(w)->frame >= 11.0 && MODEL(w)->frame < 11.0 + speed) {
            eneSetSd(o, 0x226);
        }
    }
    if (w->e.y <= 0.0f) {
        w->e.behave |= 0x40000;
    }
}

/* State 3 (type 3): waits on the bank until a player is 20-70 away on the way 1 side. */
void kappaBankWait(Obj *o) {
    Kappa *w = KAPPA(o);
    s32 pad[2];

    eneAnimLoop(o, MODEL(w), 1.0f);
    if (!eneNearestPlInX(o, 20.0f) && eneNearestPlInX(o, 70.0f) && eneNearestPlWay(o) == 1) {
        w->wait = 0;
        w->e.state = 4;
        w->e.vel[0] = 1.5f;
    }
}

/* State 4: walks for 10 steps. */
void kappaBankWalk(Obj *o) {
    Kappa *w = KAPPA(o);

    eneAnimLoop(o, MODEL(w), 1.0f);
    if (++w->wait >= 10) {
        MODEL(w)->anim = etskKappaType1Anim;
        MODEL(w)->frame = 0.0f;
        w->e.state = 5;
        w->e.vel[0] = 0.0f;
    }
}

/* State 5: a jump forward along the bank (4 at frame 12, slowing past frame 16). */
void kappaBankLeap(Obj *o) {
    Kappa *w = KAPPA(o);
    f32 speed = 0.8f;
    s32 pad;

    eneAnimPlayOnce(o, MODEL(w), speed);
    if (MODEL(w)->frame >= 11.0 && MODEL(w)->frame < 11.0 + speed) {
        eneSetSd(o, 0x226);
    }
    if (MODEL(w)->frame >= 12.0 && MODEL(w)->frame < 12.0 + speed) {
        w->e.vel[0] = 4.0f;
    }
    if (MODEL(w)->frame > 16.0) {
        w->e.vel[0] -= 0.4;
        if (w->e.vel[0] <= 0.0f) {
            MODEL(w)->anim = etskKappaType3Anim;
            MODEL(w)->frame = 0.0f;
            w->wait = 0;
            w->e.state = 6;
            w->e.vel[0] = -2.0f;
        }
    }
}

/* State 6: walks back for 30 steps, then is gone. */
void kappaBankLeave(Obj *o) {
    Kappa *w = KAPPA(o);

    eneAnimLoop(o, MODEL(w), 2.5f);
    if (++w->wait >= 30) {
        w->e.behave |= 0x40000;
    }
}

/* State 7 (types 4 and 5): waits under the bank for a player within 85, then jumps out. */
void kappaUnderWait(Obj *o) {
    Kappa *w = KAPPA(o);

    if (eneNearestPlInX(o, 85.0f)) {
        MODEL(w)->anim = etskKappaType1Anim;
        MODEL(w)->frame = 0.0f;
        w->e.behave |= 0x800;
        w->e.vel[1] = 7.5f;
        w->e.vel[2] = (w->e.z >= 0.0f) ? -1.5 : 1.5;
        w->e.state = 8;
    }
}

/* State 8: in the air across the course (a splash sound at frame 4) until it is back on z = 0. */
void kappaCrossJump(Obj *o) {
    Kappa *w = KAPPA(o);
    f32 speed = 0.8f;
    AnimeFrame *m;

    if (MODEL(w)->frame < 14.0) {
        eneAnimPlayOnce(o, MODEL(w), speed);
        m = MODEL(w);
        if (m->frame >= 4.0 && m->frame < 4.0 + speed) {
            SoundLocation2(0x219, gGameWork.view, m->pos[0], m->pos[1], m->pos[2], 0.0f, 0.0f, 1.570796371f);
        }
    }
    if ((w->e.vel[2] > 0.0f && w->e.z >= 0.0f) || (w->e.vel[2] < 0.0f && w->e.z <= 0.0f)) {
        w->e.z = 0.0f;
        w->e.vel[2] = 0.0f;
        w->e.state = 9;
    }
}

/* State 9: lands, then faces the nearest player's way and walks (1.5) that way. */
void kappaLand(Obj *o) {
    Kappa *w = KAPPA(o);
    s32 pad;

    eneAnimPlayOnce(o, MODEL(w), 0.8f);
    if (w->e.hit & 1) {
        MODEL(w)->anim = etskKappaType3Anim;
        MODEL(w)->frame = 0.0f;
        w->e.vel[0] = ((w->way = eneNearestPlWay(o)) == 1) ? -1.5 : 1.5;
        w->wait = 15;
        w->e.state = 10;
    }
}

/* State 10: walks for 15 steps. */
void kappaWalk(Obj *o) {
    Kappa *w = KAPPA(o);
    s32 n;

    eneAnimLoop(o, MODEL(w), 1.0f);
    n = w->wait - 1;
    if (n >= 0) {
        w->wait = n;
        return;
    }
    MODEL(w)->anim = etskKappaType1Anim;
    MODEL(w)->frame = 0.0f;
    w->e.behave &= ~0x4000;
    w->e.state = 11;
    w->e.vel[0] = 0.0f;
}

/* State 11: hops along its way (4 at frame 12), turning round in the air; at the end of a hop
 * it faces the other way. */
void kappaHop(Obj *o) {
    Kappa *w = KAPPA(o);
    f32 speed = 0.8f;
    s32 pad;

    eneAnimPlayOnce(o, MODEL(w), speed);
    if (MODEL(w)->frame >= 11.0 && MODEL(w)->frame < 11.0 + speed) {
        eneSetSd(o, 0x226);
    }
    if (MODEL(w)->frame >= 12.0 && MODEL(w)->frame < 12.0 + speed) {
        w->e.vel[0] = (w->way == 4) ? -4.0 : 4.0;
    }
    if (MODEL(w)->frame <= 13.0) {
        w->e.rot[1] += 3.141592741f / (13.0f / speed);
    }
    if (MODEL(w)->frame >= 15.0) {
        if (w->e.vel[0] > 0.0f) {
            w->e.vel[0] -= 0.4;
            if (w->e.vel[0] <= 0.0f) {
                w->e.vel[0] = 0.0f;
                w->e.rot[1] = eneCourseAngle(o, 1);
                w->way = 4;
                MODEL(w)->frame = 0.0f;
            }
        } else if (w->e.vel[0] < 0.0f) {
            w->e.vel[0] += 0.4;
            if (w->e.vel[0] >= 0.0f) {
                w->e.vel[0] = 0.0f;
                w->e.rot[1] = eneCourseAngle(o, 4);
                w->way = 1;
                MODEL(w)->frame = 0.0f;
            }
        }
    }
}

/* State 12 (type 6): waits, then rises out of the water to 9 above its home. */
void kappaWaterRise(Obj *o) {
    Kappa *w = KAPPA(o);

    if (w->wait - 1 >= 0) {
        w->wait--;
        return;
    }
    w->e.behave &= ~0x800;
    w->e.y += 2.0f;
    if (w->homeY + 9 <= w->e.y) {
        w->e.y = w->homeY + 9;
        w->wait = 8;
        w->e.state = 13;
        w->phase = 0.0f;
    }
}

/* State 13: sways above the water until a player is within 60. */
void kappaWaterSway(Obj *o) {
    Kappa *w = KAPPA(o);
    s32 pad;

    if (w->wait - 1 >= 0) {
        w->wait--;
        return;
    }
    w->phase += 0.5;
    if (w->phase >= 6.283185482f) {
        w->wait = 5;
        w->phase -= 6.283185482f;
        if (eneNearestPlInX(o, 60.0f)) {
            w->e.state = 14;
        }
    }
    w->e.rot[1] = eneCourseAngle(o, 8) + sinf(w->phase) * 3.141592741f / 4;
}

/* State 14: ducks back to its home height, then jumps (up 5) toward the nearest player. */
void kappaDuckJump(Obj *o) {
    Kappa *w = KAPPA(o);
    s32 pad;

    if (w->wait - 1 >= 0) {
        w->wait--;
        return;
    }
    w->e.y -= 3.0f;
    if (w->e.y <= w->homeY) {
        MODEL(w)->anim = etskKappaType1Anim;
        MODEL(w)->frame = 0.0f;
        w->e.behave |= 0x800;
        w->e.y = w->homeY;
        w->e.vel[1] = 5.0f;
        eneSetSd(o, 0x297);
        if (eneNearestPlWay(o) == 4) {
            w->e.rot[1] = eneCourseAngle(o, 1);
        } else if (eneNearestPlWay(o) == 1) {
            w->e.rot[1] = eneCourseAngle(o, 4);
        }
        w->wait = 8;
        w->e.state = 15;
    }
}

/* State 15: in the air, turning round; back in the water it waits again (state 12). */
void kappaJumpTurn(Obj *o) {
    Kappa *w = KAPPA(o);
    f32 speed = 0.8f;

    if (w->wait - 1 >= 0) {
        w->wait--;
        return;
    }
    eneAnimPlayOnce(o, MODEL(w), speed);
    if (MODEL(w)->frame >= 11.0 && MODEL(w)->frame < 11.0 + speed) {
        eneSetSd(o, 0x226);
    }
    if (MODEL(w)->frame <= 13.0) {
        w->e.rot[1] += 3.141592741f / (13.0f / speed);
        w->splashed = 0;
    }
    if (w->e.hit & 1) {
        MODEL(w)->frame = 0.0f;
        w->e.rot[1] = eneCourseAngle(o, 8);
        w->wait = 15;
        w->e.state = 12;
    }
}

/* State 16 (type 8): comes up the bank to z = 0, then (once on the ground) runs. */
void kappaClimb(Obj *o) {
    Kappa *w = KAPPA(o);
    s32 pad[2];

    w->e.rot[1] = eneCourseAngle(o, 8);
    eneAnimLoop(o, MODEL(w), 2.0f);
    if (w->e.z < 0.0f) {
        w->e.vel[2] = 1.799999952f;
    } else {
        w->e.vel[2] = 0.0f;
        if (w->e.hit & 1) {
            w->e.state = 17;
        }
    }
}

/* State 17: runs, speeding up to 1.8 and curving. */
void kappaRun(Obj *o) {
    Kappa *w = KAPPA(o);

    eneAnimLoop(o, MODEL(w), 2.0f);
    if (w->speed < 1.79999999999999982) {
        w->speed += 0.0750000000000000111;
    }
    w->e.vel[0] = w->speed;
    w->e.rot[1] += w->speed / 10.0f;
}

/* State 18 (type 9): steers toward the nearest player (enePlDir), 0.05 a step up to 0.5. */
void kappaChase(Obj *o) {
    Kappa *w = KAPPA(o);
    s32 dir;

    eneAnimLoop(o, MODEL(w), 1.6f);
    dir = enePlDir(o, 0);
    if (dir & 0x10) {
        w->e.vel[1] += 0.05;
        if (w->e.vel[1] >= 0.5) {
            w->e.vel[1] = 0.5f;
        }
    } else if (dir & 0x20) {
        w->e.vel[1] -= 0.05;
        if (w->e.vel[1] <= -0.5) {
            w->e.vel[1] = -0.5f;
        }
    } else {
        w->e.vel[1] = 0.0f;
    }
    if (dir & 1) {
        w->e.vel[0] += 0.05;
        if (w->e.vel[0] >= 0.5) {
            w->e.vel[0] = 0.5f;
        }
    } else if (dir & 4) {
        w->e.vel[0] -= 0.05;
        if (w->e.vel[0] <= -0.5) {
            w->e.vel[0] = -0.5f;
        }
    } else {
        w->e.vel[0] = 0.0f;
    }
}

/* State 19 (types 10 and 11): circles its home, 30 out. */
void kappaCircle(Obj *o) {
    Kappa *w = KAPPA(o);
    s32 pad[2];

    eneAnimLoop(o, MODEL(w), 1.6f);
    w->e.x = cosf(w->phase) * 30.0f + w->homeX;
    w->e.z = sinf(w->phase) * 30.0f + w->homeZ;
    w->e.rot[1] -= 0.03;
    w->phase += 0.03;
}
