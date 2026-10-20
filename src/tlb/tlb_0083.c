#include "tlb.h"

/* The "tb_hani" enemy kind (archive file 83): a haniwa that guards its front, waits for a
 * player, then spits "tama" balls (kind 0xDA, the second proc here) the way it faces. A hit
 * from the side it faces while its guard is up knocks it back instead of killing it. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 tbHaniTamaDl[];
extern u8 etskTbHaniCloseAnim[];
extern u8 etskTbHaniState6Anim[];
extern u8 etskTbHaniAnim[];
extern u8 etskTbHaniOpenAnim[];
extern u8 etskTbHaniHalf0Dl[];
extern u8 etskTbHaniHalf1Dl[];

/* A tb_hani's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct TbHani {
    /* 0x00 */ EneObj e;
    /* 0x4C */ s8 wait; /* frames before its next move */
    /* 0x4D */ u8 type; /* its placement's type, from eneSetup */
    /* 0x4E */ u8 guard; /* hits from the way it faces knock it back (state 6) */
} TbHani;

/* A tama's work: an enemy's and its age. */
typedef struct TbTama {
    /* 0x00 */ EneObj e;
    /* 0x4C */ s16 age; /* past 100 frames it is gone */
} TbTama;

#define TB(obj) ((TbHani *) FX(obj))
#define TAMA(obj) ((TbTama *) FX(obj))

void tbHaniWait(Obj *o);
void tbHaniOpen(Obj *o);
void tbHaniSpit(Obj *o);
void tbHaniClose(Obj *o);
void tbHaniWalkIn(Obj *o);
void tbHaniWalk(Obj *o);
void tbHaniKnockBack(Obj *o);

s32 Etsk_tb_hani_main(Obj *o, s16 msg, void *arg) {
    TbHani *w = TB(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "tb_hani";
            w->type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewAnime(o, (u32) etskTbHaniAnim)) == NULL) {
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
            if (eneSetOnEarth(o) == -1) {
                return -1;
            }
            w->e.halves[0] = (u32) etskTbHaniHalf0Dl;
            w->e.halves[1] = (u32) etskTbHaniHalf1Dl;
            if (w->e.z < 0.0f) {
                w->e.rot[1] = eneCourseAngle(o, 8);
            } else if (w->e.z > 0.0f) {
                w->e.rot[1] = eneCourseAngle(o, 2);
            } else {
                w->e.rot[1] = eneCourseAngle(o, eneNearestPlWay(o));
            }
            w->e.state = 0;
            if (w->type == 1) {
                w->e.behave = 0x404D01;
            } else if (w->type == 2) {
                w->e.behave = 0x400801;
            } else if (w->type == 3) {
                w->e.behave = 0x400801;
                w->e.rot[1] = eneCourseAngle(o, 4);
            } else if (w->type == 4) {
                w->e.behave = 0x400801;
                w->e.rot[1] = eneCourseAngle(o, 1);
            } else if (w->type == 5) {
                w->e.behave = 0x400801;
            } else if (w->type == 6) {
                w->e.behave = 0x801;
                w->e.rot[1] = eneCourseAngle(o, 4);
            } else if (w->type == 7) {
                w->e.behave = 0x400801;
                w->e.rot[1] = eneCourseAngle(o, 4);
            }
            w->guard = 1;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*tbHaniWait)(o);
                    break;
                case 1:
                    (*tbHaniOpen)(o);
                    break;
                case 2:
                    (*tbHaniSpit)(o);
                    break;
                case 3:
                    (*tbHaniClose)(o);
                    break;
                case 4:
                    (*tbHaniWalkIn)(o);
                    break;
                case 5:
                    (*tbHaniWalk)(o);
                    break;
                case 6:
                    (*tbHaniKnockBack)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_tb_hani_main: 死亡\n");
                return -1;
            }
            if (w->e.hit & 0x10) {
                if (w->e.dmg->hitKind == 13 || w->e.dmg->hitKind == 10 || w->e.dmg->hitKind == 14
                    || w->e.dmg->hitKind == 125) {
                    w->e.behave |= 0x80000;
                } else if (w->guard != 0 && eneNearestPlWay(o) == eneFaceWay(o)) {
                    eneSetSd(o, 0x2AF);
                    ((AnimeFrame *) w->e.model)->anim = etskTbHaniState6Anim;
                    ((AnimeFrame *) w->e.model)->frame = 1.0f;
                    w->e.behave &= ~0x4500;
                    w->e.behave |= 0x280;
                    if (eneFaceWay(o) == 4) {
                        w->e.vel[0] = 1.0f;
                    } else if (eneFaceWay(o) == 1) {
                        w->e.vel[0] = -1.0f;
                    }
                    w->e.state = 6;
                } else {
                    w->e.behave |= 0x80000;
                }
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

void tbHaniWait(Obj *o) {
    TbHani *w = TB(o);

    if (eneNearestPlInX(o, 80.0f)) {
        w->wait = 0;
        w->e.state = 1;
    }
}

void tbHaniOpen(Obj *o) {
    TbHani *w = TB(o);
    s32 pad[2];

    if (w->wait > 0) {
        w->wait--;
    } else {
        if (eneAnimPlayOnce(o, (AnimeFrame *) w->e.model, 0.3f) == 1) {
            if (w->type == 5 || w->type == 7) {
                w->wait = 15;
                w->e.state = 3;
            } else {
                ((AnimeFrame *) w->e.model)->anim = etskTbHaniOpenAnim;
                ((AnimeFrame *) w->e.model)->frame = 0.0f;
                w->wait = 15;
                w->e.state = 2;
            }
        }
        if (((AnimeFrame *) w->e.model)->frame >= 1.0) {
            w->guard = 0;
        }
    }
}

void tbHaniSpit(Obj *o) {
    TbHani *w = TB(o);
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
                eneMakeEx(NULL, ENE_TB_HANI_TAMA_MAIN, w->e.zone, w->e.x + 14.0f, w->e.y + 11.5, w->e.z, 2,
                          (u32 (*)(void *, void *, void *)) way);
                break;
            case 2:
                vramCoursePointXZ(w->e.zone, w->e.x + 7.5, w->e.z - 14.0f, &x, &z);
                itexPuffWhite(x, ((AnimeFrame *) w->e.model)->pos[1] + 11.5, z, 0.06f);
                eneMakeEx(NULL, ENE_TB_HANI_TAMA_MAIN, w->e.zone, w->e.x + 7.5, w->e.y + 11.5, w->e.z - 14.0f, 2,
                          (u32 (*)(void *, void *, void *)) way);
                break;
            case 4:
                vramCoursePointXZ(w->e.zone, w->e.x - 14.0f, w->e.z - 7.5, &x, &z);
                itexPuffWhite(x, ((AnimeFrame *) w->e.model)->pos[1] + 11.5, z, 0.06f);
                eneMakeEx(NULL, ENE_TB_HANI_TAMA_MAIN, w->e.zone, w->e.x - 14.0f, w->e.y + 11.5, w->e.z, 2,
                          (u32 (*)(void *, void *, void *)) way);
                break;
            case 8:
                vramCoursePointXZ(w->e.zone, w->e.x - 7.5, w->e.z + 14.0f, &x, &z);
                itexPuffWhite(x, ((AnimeFrame *) w->e.model)->pos[1] + 11.5, z, 0.06f);
                eneMakeEx(NULL, ENE_TB_HANI_TAMA_MAIN, w->e.zone, w->e.x - 7.5, w->e.y + 11.5, w->e.z + 14.0f, 2,
                          (u32 (*)(void *, void *, void *)) way);
                break;
        }
    }
    if (eneAnimPlayOnce(o, (AnimeFrame *) w->e.model, speed) == 1) {
        ((AnimeFrame *) w->e.model)->anim = etskTbHaniAnim;
        ((AnimeFrame *) w->e.model)->frame = polAnimeLastFrame((AnimeFrame *) w->e.model);
        w->e.state = 3;
    }
}

void tbHaniClose(Obj *o) {
    TbHani *w = TB(o);
    s32 pad[2];

    if (w->wait > 0) {
        w->wait--;
    } else {
        if (eneAnimPlayOnce(o, (AnimeFrame *) w->e.model, -0.6f) == 1) {
            if (w->type == 1) {
                ((AnimeFrame *) w->e.model)->anim = etskTbHaniCloseAnim;
                ((AnimeFrame *) w->e.model)->frame = 0.0f;
                w->e.state = 4;
            } else {
                ((AnimeFrame *) w->e.model)->anim = etskTbHaniAnim;
                ((AnimeFrame *) w->e.model)->frame = 0.0f;
                w->wait = 50;
                w->e.state = 1;
            }
        }
        if (((AnimeFrame *) w->e.model)->frame <= 1.0) {
            w->guard = 1;
        }
    }
}

void tbHaniWalkIn(Obj *o) {
    TbHani *w = TB(o);
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

void tbHaniWalk(Obj *o) {
    TbHani *w = TB(o);

    eneAnimLoop(o, (AnimeFrame *) w->e.model, 1.0f);
}

void tbHaniKnockBack(Obj *o) {
    TbHani *w = TB(o);
    s32 pad[2];

    if (w->e.vel[0] > 0.0f) {
        w->e.vel[0] -= 0.25;
        if (w->e.vel[0] <= 0.0f) {
            w->e.vel[0] = 0.0f;
        }
    } else if (w->e.vel[0] < 0.0f) {
        w->e.vel[0] += 0.25;
        if (w->e.vel[0] <= 0.0f) {
            w->e.vel[0] = 0.0f;
        }
    }
    eneAnimPlayOnce(o, (AnimeFrame *) w->e.model, 0.5f);
    if (w->e.vel[0] == 0.0) {
        ((AnimeFrame *) w->e.model)->anim = etskTbHaniAnim;
        ((AnimeFrame *) w->e.model)->frame = 0.0f;
        w->e.behave &= ~0x280;
        w->e.behave |= 0x4500;
        w->wait = 0;
        w->e.state = 1;
    }
}

/* "tama": tb_hani's ball (file 84's bzHaniTamaProc to the instruction). Message 1's tests are
 * each on one line with their call: written over three lines, as1 fills the first test's delay
 * slot with frmDestroy's address instead of w's "addiu v1,v1,0x20". */
s32 tbHaniTamaMain(Obj *o, s16 msg, void *arg) {
    TbTama *w = TAMA(o);
    s32 pad;
    s32 way;
    Vec3f v;

    switch (msg) {
        case 2:
            o->name = "tama";
            way = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewPol(o, (Gfx *) tbHaniTamaDl)) == NULL) {
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
                PRINTF("Etsk_tb_hani_tama: 死亡\n");
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
