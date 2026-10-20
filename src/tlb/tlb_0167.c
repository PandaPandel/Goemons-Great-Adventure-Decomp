#include "tlb.h"

/* The "yoroi" enemy kind (archive file 167): a suit of armour. Its placement's type 1 walks
 * back and forth within 60 of where the camera first came near and recoils when hit (four
 * hits); type 2 stands unseen with every box off until a player is within 50, then fades in,
 * strikes (its third attack box is live from frame 6 to 8 of the swing) and fades out again;
 * types 3 and 4 stand still, boxes off, facing way 4 or 8. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 etskYoroiWalkAnim[]; /* its walk */
extern u8 etskYoroiSwingAnim[]; /* its swing */
extern u8 etskYoroiRecoilAnim[]; /* its recoil */
extern u8 etskYoroiHalf0Dl[];
extern u8 etskYoroiHalf1Dl[];

/* A yoroi's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Yoroi {
    /* 0x00 */ EneObj e;
    /* 0x4C */ f32 speed; /* its walk's, up to 1 */
    /* 0x50 */ u8 type; /* its placement's type, from eneSetup */
    /* 0x51 */ u8 way; /* 1 or 4: the way it walks */
    /* 0x52 */ u8 alpha; /* types 2-4: how far it has faded in */
    /* 0x53 */ s8 flash; /* steps of the flash after a hit */
    /* 0x54 */ u8 unused54; /* written 0 at its start, never read */
    /* 0x56 */ s16 homeX; /* where the camera first came near: the middle of its walk */
} Yoroi;

#define YOROI(obj) ((Yoroi *) FX(obj))

void yoroiWait(Obj *o);
void yoroiWalk(Obj *o);
void yoroiIdle(Obj *o);
void yoroiHide(Obj *o);
void yoroiAppear(Obj *o);
void yoroiVanish(Obj *o);
void yoroiStand(Obj *o);
void yoroiRecoil(Obj *o);

s32 Etsk_yoroi_main(Obj *o, s16 msg, void *arg) {
    Yoroi *w = YOROI(o);
    ACollFrame *box;
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "yoroi";
            w->type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneAnimeModelMake(o, (u32) etskYoroiSwingAnim, NULL, 0)) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 50, 100, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((box = ATK_ADD_YZ(&w->e.atk->frame, w->e.model, 40, 250, 0, 0)) == NULL) {
                return -1;
            }
            box->kind = 120;
            if ((box = ATK_ADD_YZ(&w->e.atk->frame, w->e.model, 60, 140, 250, 0)) == NULL) {
                return -1;
            }
            box->kind = 120;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 75, 300, 0, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->e.shadow = (Obj *) shadowCreate()) == NULL) {
                return -1;
            }
            w->e.halves[0] = (u32) etskYoroiHalf0Dl;
            w->e.halves[1] = (u32) etskYoroiHalf1Dl;
            if (eneSetOnEarth(o) == -1) {
                return -1;
            }
            w->e.behave = 0x8400A80;
            w->e.hp = 4;
            w->flash = 0;
            w->alpha = 0;
            w->unused54 = 0;
            if (w->type == 1) {
                ((AnimeFrame *) w->e.model)->anim = etskYoroiWalkAnim;
                ((ACollFrame *) w->e.atk->frame.child->next)->flags &= ~2;
                w->way = eneNearestPlWay(o);
                w->e.rot[1] = eneCourseAngle(o, w->way);
                w->e.state = 0;
            } else if (w->type == 2) {
                w->e.atk->flags &= ~2;
                ((ACollFrame *) w->e.atk->frame.child)->flags &= ~2;
                ((ACollFrame *) w->e.atk->frame.child->next)->flags &= ~2;
                w->e.dmg->flags &= ~2;
                if (w->e.z < 0.0f) {
                    w->e.rot[1] = eneCourseAngle(o, 8);
                } else {
                    w->e.rot[1] = eneCourseAngle(o, 2);
                }
                w->e.state = 3;
            } else if (w->type == 3) {
                w->e.atk->flags &= ~2;
                ((ACollFrame *) w->e.atk->frame.child)->flags &= ~2;
                ((ACollFrame *) w->e.atk->frame.child->next)->flags &= ~2;
                w->e.dmg->flags &= ~2;
                w->e.rot[1] = eneCourseAngle(o, 4);
                w->e.state = 6;
            } else if (w->type == 4) {
                w->e.atk->flags &= ~2;
                ((ACollFrame *) w->e.atk->frame.child)->flags &= ~2;
                ((ACollFrame *) w->e.atk->frame.child->next)->flags &= ~2;
                w->e.dmg->flags &= ~2;
                w->e.rot[1] = eneCourseAngle(o, 8);
                w->e.state = 6;
            }
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*yoroiWait)(o);
                    break;
                case 1:
                    (*yoroiWalk)(o);
                    break;
                case 2:
                    (*yoroiIdle)(o);
                    break;
                case 3:
                    (*yoroiHide)(o);
                    break;
                case 4:
                    (*yoroiAppear)(o);
                    break;
                case 5:
                    (*yoroiVanish)(o);
                    break;
                case 6:
                    (*yoroiStand)(o);
                    break;
                case 7:
                    (*yoroiRecoil)(o);
                    break;
            }
            if (((AnimeFrame *) w->e.model)->anim == etskYoroiSwingAnim) {
                if (((AnimeFrame *) w->e.model)->frame >= 6.0 && ((AnimeFrame *) w->e.model)->frame < 8.0) {
                    ((ACollFrame *) w->e.atk->frame.child->next)->flags |= 2;
                } else {
                    ((ACollFrame *) w->e.atk->frame.child->next)->flags &= ~2;
                }
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_yoroi_main: 死亡\n");
                return -1;
            }
            if (w->type == 1) {
                if (w->e.hit & 0x10) {
                    eneSetSd2(o, 0x2EC, 0, 0);
                    w->flash = 16;
                    ((AnimeFrame *) w->e.model)->anim = etskYoroiRecoilAnim;
                    ((AnimeFrame *) w->e.model)->frame = 2.0f;
                    w->e.state = 7;
                    w->e.vel[0] = 0.0f;
                }
                if (--w->flash > 0) {
                    w->e.dmg->flags &= ~2;
                } else {
                    w->flash = 0;
                    w->e.dmg->flags |= 2;
                }
            }
            /* fallthrough */
        case 3:
            if (w->flash != 0) {
                ((AnimeFrame *) w->e.model)->dl = eneDlEnvColorLit(o, (w->flash & 2) ? 0x80 : 0, 0xFF, 0xFF, 0xFF,
                                                                   (w->flash & 2) ? 0 : 1);
            } else {
                ((AnimeFrame *) w->e.model)->dl = eneDlEnvColorLit(o, w->alpha, 0xFF, 0x7F, 0x4F, 1);
            }
            break;
        case 1:
            if (w->e.model != NULL) { frmDestroy(w->e.model); }
            if (w->e.atk != NULL) { frmDestroy(&w->e.atk->frame); }
            if (w->e.dmg != NULL) { frmDestroy(&w->e.dmg->frame); }
            if (w->e.shadow != NULL) { frmDestroy((Frame *) w->e.shadow); }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* State 0: waits for the camera within 120; where it is then is the middle of its walk. */
void yoroiWait(Obj *o) {
    Yoroi *w = YOROI(o);

    if (eneCamNearCheck(o, 120.0f)) {
        w->homeX = w->e.x;
        w->e.state = 1;
    }
}

/* State 1: walks, speeding up to 1, turning back 60 from the middle. */
void yoroiWalk(Obj *o) {
    s32 pad;
    Yoroi *w = YOROI(o);
    f32 step;
    f32 d;

    w->speed += 0.05;
    if (w->speed >= 1.0) {
        w->speed = 1.0f;
    }
    if (w->way == 4) {
        w->e.vel[0] = -w->speed;
    } else {
        w->e.vel[0] = w->speed;
    }
    if (eneFaceWay(o) == w->way) {
        step = w->speed * 0.4;
    } else {
        step = -w->speed * 0.4;
    }
    eneAnimLoop(o, (AnimeFrame *) w->e.model, step);
    d = w->homeX - w->e.x;
    if (w->way == 4) {
        if (d >= 60.0f) {
            w->way = 1;
            w->speed = 0.0f;
        }
    } else if (d <= -60.0f) {
        w->way = 4;
        w->speed = 0.0f;
    }
}

/* State 2: nothing. */
void yoroiIdle(Obj *o) {
}

/* State 3 (types 2-4, hidden): waits for a player within 50. */
void yoroiHide(Obj *o) {
    if (enePlInRangeCheck(o, 50.0f, 0)) {
        YOROI(o)->e.state = 4;
    }
}

/* State 4: fades in, then strikes (with its sound at frame 5). */
void yoroiAppear(Obj *o) {
    Yoroi *w = YOROI(o);
    f32 speed = 0.3f;
    s32 pad;

    if (w->alpha + 32 < 255) {
        w->alpha += 32;
        return;
    }
    w->alpha = 255;
    if (eneAnimPlayOnce(o, (AnimeFrame *) w->e.model, speed)) {
        w->e.state = 5;
    }
    if (((AnimeFrame *) w->e.model)->frame >= 5.0 && ((AnimeFrame *) w->e.model)->frame < 5.0 + speed) {
        eneSetSd2(o, 0x2F1, 0, 0);
    }
}

/* State 5: fades out, and hides again. */
void yoroiVanish(Obj *o) {
    Yoroi *w = YOROI(o);

    if (w->alpha - 8 > 0) {
        w->alpha -= 8;
        return;
    }
    w->alpha = 0;
    ((AnimeFrame *) w->e.model)->frame = 0.0f;
    w->e.state = 3;
}

/* State 6: nothing. */
void yoroiStand(Obj *o) {
}

/* State 7: recoils from a hit, knocked back away from the way it faces, then walks again. */
void yoroiRecoil(Obj *o) {
    Yoroi *w = YOROI(o);
    f32 speed = 1.0;
    s32 pad;

    w->e.vel[0] = speed * -16.0f / 10.0f;
    if (eneFaceWay(o) == 4) {
        w->e.vel[0] *= -1.0f;
    }
    if (eneAnimPlayOnce(o, (AnimeFrame *) w->e.model, speed)) {
        ((AnimeFrame *) w->e.model)->anim = etskYoroiWalkAnim;
        ((AnimeFrame *) w->e.model)->frame = 0.0f;
        w->e.state = 1;
        w->speed = 0.0f;
    }
}
