#include "tlb.h"

/* The "bz_hani" enemy kind (archive file 84): tb_hani's twin (archive file 83). A haniwa that
 * waits for a player, then spits "tama" balls (kind 0x206, the second proc here) the way it
 * faces. Its placement's type 1 walks off after a volley; type 7 sways along x, out from where
 * it was placed and back. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 bzHaniTamaDl[];
extern u8 etskBzHaniCloseAnim[];
extern u8 etskBzHaniAnim[];
extern u8 etskBzHaniOpenAnim[];
extern u8 etskBzHaniHalf0Dl[];
extern u8 etskBzHaniHalf1Dl[];

/* A bz_hani's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct BzHani {
    /* 0x00 */ EneObj e;
    /* 0x4C */ s16 home; /* type 7: the x it was placed at, where its sway turns back */
    /* 0x4E */ s8 wait; /* frames before its next move */
    /* 0x4F */ u8 type; /* its placement's type, from eneSetup */
} BzHani;

/* A tama's work: an enemy's and its age. */
typedef struct BzTama {
    /* 0x00 */ EneObj e;
    /* 0x4C */ s16 age; /* past 100 frames it is gone */
} BzTama;

#define BZ(obj) ((BzHani *) FX(obj))
#define TAMA(obj) ((BzTama *) FX(obj))

void bzHaniWatch(Obj *o);
void bzHaniOpen(Obj *o);
void bzHaniShoot(Obj *o);
void bzHaniClose(Obj *o);
void bzHaniWalkIn(Obj *o);
void bzHaniWalk(Obj *o);
void bzHaniSwayBack(Obj *o);

s32 Etsk_bz_hani_main(Obj *o, s16 msg, void *arg) {
    BzHani *w = BZ(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "bz_hani";
            w->type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewAnime(o, (u32) etskBzHaniAnim)) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 42, 80, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 70, 160, 0, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->e.shadow = (Obj *) shadowCreate()) == NULL) {
                return -1;
            }
            if (w->type != 7 && eneSetOnEarth(o) == -1) {
                return -1;
            }
            w->e.halves[0] = (u32) etskBzHaniHalf0Dl;
            w->e.halves[1] = (u32) etskBzHaniHalf1Dl;
            if (w->e.z < 0.0f) {
                w->e.rot[1] = eneCourseAngle(o, 8);
            } else if (w->e.z > 0.0f) {
                w->e.rot[1] = eneCourseAngle(o, 2);
            } else {
                w->e.rot[1] = eneCourseAngle(o, eneNearestPlWay(o));
            }
            w->e.state = 0;
            if (w->type == 1) {
                w->e.behave = 0x404D00;
            } else if (w->type == 2) {
                w->e.behave = 0x400800;
            } else if (w->type == 3) {
                w->e.behave = 0x400800;
                w->e.rot[1] = eneCourseAngle(o, 4);
            } else if (w->type == 4) {
                w->e.behave = 0x400800;
                w->e.rot[1] = eneCourseAngle(o, 1);
            } else if (w->type == 5) {
                w->e.behave = 0x400800;
            } else if (w->type == 6) {
                w->e.behave = 0x800;
                w->e.rot[1] = eneCourseAngle(o, 4);
            } else if (w->type == 7) {
                w->e.behave = 0x20000;
                w->home = w->e.x;
                w->e.vel[0] = -0.4f;
                w->e.rot[1] = eneCourseAngle(o, 1);
                w->e.state = 1;
            }
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*bzHaniWatch)(o);
                    break;
                case 1:
                    (*bzHaniOpen)(o);
                    break;
                case 2:
                    (*bzHaniShoot)(o);
                    break;
                case 3:
                    (*bzHaniClose)(o);
                    break;
                case 4:
                    (*bzHaniWalkIn)(o);
                    break;
                case 5:
                    (*bzHaniWalk)(o);
                    break;
                case 6:
                    (*bzHaniSwayBack)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_bz_hani_main: 死亡\n");
                return -1;
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
            if (w->e.shadow != NULL) {
                frmDestroy((Frame *) w->e.shadow);
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void bzHaniWatch(Obj *o) {
    BzHani *w = BZ(o);

    if (eneNearestPlInX(o, 80.0f)) {
        w->wait = 0;
        w->e.state = 1;
    }
}

void bzHaniOpen(Obj *o) {
    BzHani *w = BZ(o);
    s32 pad[2];

    if (w->wait > 0) {
        w->wait--;
    } else if (eneAnimPlayOnce(o, (AnimeFrame *) w->e.model, 0.3f) == 1) {
        if (w->type == 5) {
            w->wait = 15;
            w->e.state = 3;
        } else {
            ((AnimeFrame *) w->e.model)->anim = etskBzHaniOpenAnim;
            ((AnimeFrame *) w->e.model)->frame = 0.0f;
            w->wait = 15;
            w->e.state = 2;
        }
    }
}

void bzHaniShoot(Obj *o) {
    BzHani *w = BZ(o);
    s32 pad;
    f32 speed;
    f32 x;
    f32 z;
    s32 way;

    if (w->wait > 0) {
        w->wait--;
        return;
    }
    if (((AnimeFrame *) w->e.model)->frame < 2.0) {
        speed = 0.5f;
    } else {
        speed = 0.2f;
    }
    if (((AnimeFrame *) w->e.model)->frame >= 0.0f && ((AnimeFrame *) w->e.model)->frame < speed) {
        eneSetSd(o, 0x22F);
        switch (way = eneFaceWay(o)) {
            case 1:
                vramCoursePointXZ(w->e.zone, w->e.x + 14.0f, w->e.z + 7.5, &x, &z);
                itexPuffWhite(x, ((AnimeFrame *) w->e.model)->pos[1] + 11.5, z, 0.06f);
                eneMakeEx(NULL, ENE_BZ_HANI_TAMA, w->e.zone, w->e.x + 14.0f, w->e.y + 11.5, w->e.z, 2,
                          (u32 (*)(void *, void *, void *)) way);
                break;
            case 2:
                vramCoursePointXZ(w->e.zone, w->e.x + 7.5, w->e.z - 14.0f, &x, &z);
                itexPuffWhite(x, ((AnimeFrame *) w->e.model)->pos[1] + 11.5, z, 0.06f);
                eneMakeEx(NULL, ENE_BZ_HANI_TAMA, w->e.zone, w->e.x + 7.5, w->e.y + 11.5, w->e.z - 14.0f, 2,
                          (u32 (*)(void *, void *, void *)) way);
                break;
            case 4:
                vramCoursePointXZ(w->e.zone, w->e.x - 14.0f, w->e.z - 7.5, &x, &z);
                itexPuffWhite(x, ((AnimeFrame *) w->e.model)->pos[1] + 11.5, z, 0.06f);
                eneMakeEx(NULL, ENE_BZ_HANI_TAMA, w->e.zone, w->e.x - 14.0f, w->e.y + 11.5, w->e.z, 2,
                          (u32 (*)(void *, void *, void *)) way);
                break;
            case 8:
                vramCoursePointXZ(w->e.zone, w->e.x - 7.5, w->e.z + 14.0f, &x, &z);
                itexPuffWhite(x, ((AnimeFrame *) w->e.model)->pos[1] + 11.5, z, 0.06f);
                eneMakeEx(NULL, ENE_BZ_HANI_TAMA, w->e.zone, w->e.x - 7.5, w->e.y + 11.5, w->e.z + 14.0f, 2,
                          (u32 (*)(void *, void *, void *)) way);
                break;
        }
    }
    if (eneAnimPlayOnce(o, (AnimeFrame *) w->e.model, speed) == 1) {
        ((AnimeFrame *) w->e.model)->anim = etskBzHaniAnim;
        ((AnimeFrame *) w->e.model)->frame = polAnimeLastFrame((AnimeFrame *) w->e.model);
        w->e.state = 3;
    }
}

void bzHaniClose(Obj *o) {
    BzHani *w = BZ(o);
    s32 pad[2];

    if (w->wait > 0) {
        w->wait--;
    } else if (eneAnimPlayOnce(o, (AnimeFrame *) w->e.model, -0.6f) == 1) {
        if (w->type == 1) {
            ((AnimeFrame *) w->e.model)->anim = etskBzHaniCloseAnim;
            ((AnimeFrame *) w->e.model)->frame = 0.0f;
            w->e.state = 4;
        } else if (w->type == 7) {
            ((AnimeFrame *) w->e.model)->anim = etskBzHaniCloseAnim;
            ((AnimeFrame *) w->e.model)->frame = 0.0f;
            w->e.state = 6;
            w->e.vel[0] = 0.25f;
        } else {
            ((AnimeFrame *) w->e.model)->anim = etskBzHaniAnim;
            ((AnimeFrame *) w->e.model)->frame = 0.0f;
            w->wait = 30;
            w->e.state = 1;
        }
    }
}

void bzHaniWalkIn(Obj *o) {
    BzHani *w = BZ(o);
    s32 pad[2];

    eneAnimLoop(o, (AnimeFrame *) w->e.model, 1.0f);
    if (w->e.z < 0.0f) {
        w->e.vel[2] = 0.8f;
        if (w->e.z + 0.8f > 0.0f) {
            w->e.vel[2] = 0.0f;
            w->e.z = 0.0f;
        }
    } else if (w->e.z > 0.0f) {
        w->e.vel[2] = -0.8f;
        if (w->e.z < -0.8f) {
            w->e.vel[2] = 0.0f;
            w->e.z = 0.0f;
        }
    } else {
        if (eneNearestPlWay(o) == 1) {
            w->e.vel[0] = 0.8f;
        } else if (eneNearestPlWay(o) == 4) {
            w->e.vel[0] = -0.8f;
        }
        w->e.state = 5;
    }
}

void bzHaniWalk(Obj *o) {
    BzHani *w = BZ(o);

    eneAnimLoop(o, (AnimeFrame *) w->e.model, 1.0f);
}

void bzHaniSwayBack(Obj *o) {
    BzHani *w = BZ(o);

    eneAnimLoop(o, (AnimeFrame *) w->e.model, 1.0f);
    if (w->e.x >= w->home) {
        ((AnimeFrame *) w->e.model)->anim = etskBzHaniAnim;
        ((AnimeFrame *) w->e.model)->frame = 0.0f;
        w->e.vel[0] = -0.4f;
        w->wait = 30;
        w->e.state = 1;
    }
}

/* tb_hani's tama (tlb_0083's tbHaniTamaMain) to the instruction. Message 1's tests are each on
 * one line with their call: written over three lines, as1 fills the first test's delay slot
 * with frmDestroy's address instead of w's "addiu v1,v1,0x20". */
s32 bzHaniTamaProc(Obj *o, s16 msg, void *arg) {
    BzTama *w = TAMA(o);
    s32 pad;
    s32 way;
    Vec3f v;

    switch (msg) {
        case 2:
            o->name = "tama";
            way = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewPol(o, (Gfx *) bzHaniTamaDl)) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 20, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 123;
            ((PolFrame *) w->e.model)->flags |= 7;
            ((PolFrame *) w->e.model)->scale[2] = 0.1f;
            ((PolFrame *) w->e.model)->scale[0] = ((PolFrame *) w->e.model)->scale[1] =
                ((PolFrame *) w->e.model)->scale[2];
            w->age = 0;
            w->e.behave = 4;
            if (way == 4) {
                w->e.vel[0] = -3.0f;
            } else if (way == 1) {
                w->e.vel[0] = 3.0f;
            } else if (way == 8) {
                w->e.vel[2] = 3.0f;
            } else if (way == 2) {
                w->e.vel[2] = -3.0f;
            }
            v.x = w->e.vel[0];
            v.y = w->e.vel[1];
            v.z = w->e.vel[2];
            *(Vec3f *) &w->e.atk->info = v;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            if (w->e.hit & 0xE) {
                w->e.behave |= 0x4000000;
            }
            if (w->age++ > 100) {
                w->e.behave |= 0x10000000;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_bz_hani_tama: 死亡\n");
                return -1;
            }
            break;
        case 1:
            if (w->e.model != NULL) { frmDestroy(w->e.model); }
            if (w->e.atk != NULL) { frmDestroy(&w->e.atk->frame); }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
