#include "tlb.h"

/* The "wanyu" enemy kind (archive file 161): a wanyudo, a burning wheel that swings like a
 * pendulum on a 40-unit arm, spinning one way and the other, with five "fire"s (kind 0x21A,
 * the second proc here) circling it. A player near it starts the swing; its model's animation
 * frame follows the player along x. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

/* libultra's sinf and cosf (PR/gu.h declares them), called from the TLB through their address. */
#define sinf (*sinf)
#define cosf (*cosf)

#pragma intrinsic(fabs)

extern u8 etskWanyuAnim[];
extern u8 etskWanyuWheelDl[];
extern u8 etskWanyuHalf0Dl[];
extern u8 etskWanyuHalf1Dl[];
extern u8 wanyuFireDl[];
extern u8 wanyuFireTex0[];
extern u8 wanyuFireTex2[];
extern u8 wanyuFireTex5[];
AnimFrame wanyuFireAnim[] = {
    { (u32) wanyuFireTex0, 2 },
    { 0x03000D80, 2 },
    { (u32) wanyuFireTex2, 2 },
    { 0x03001D80, 2 },
    { 0x03002580, 2 },
    { (u32) wanyuFireTex5, 2 },
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
    { 0, 0 },
};

/* A wanyu's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Wanyu {
    /* 0x00 */ EneObj e;
    /* 0x4C */ PolFrame *wheel; /* the burning wheel, spinning about x */
    /* 0x50 */ f32 spin; /* the wheel's step about x */
    /* 0x54 */ s8 dir; /* 1 or -1: the way spin changes */
    /* 0x55 */ u8 pl; /* the player that came near, 1-4 (0: none yet) */
    /* 0x56 */ u8 pad56[2];
    /* 0x58 */ s16 baseY; /* the swing's pivot */
    /* 0x5A */ s16 baseX;
    /* 0x5C */ s16 angle; /* the arm's, in milliradians (pi about 3142) */
    /* 0x5E */ s16 angVel; /* its step */
} Wanyu;

/* A fire's work: an enemy's, its texture animation and its place in the ring. */
typedef struct WanyuFire {
    /* 0x00 */ EneObj e;
    /* 0x4C */ Anim anim;
    /* 0x5C */ u8 index; /* from eneSetup: its place among the wheel's children */
} WanyuFire;

#define WANYU(obj) ((Wanyu *) FX(obj))
#define FIRE(obj) ((WanyuFire *) FX(obj))

void wanyuWait(Obj *o);
void wanyuSwingLeft(Obj *o);
void wanyuSwingRight(Obj *o);

s32 Etsk_wanyu_main(Obj *o, s16 msg, void *arg) {
    Wanyu *w = WANYU(o);
    s32 pad;
    s32 i;
    f32 px;
    f32 py;
    f32 pz;
    f32 d;
    s32 pl;
    s32 pad2;
    s32 zone;

    switch (msg) {
        case 2:
            o->name = "wanyu";
            eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewAnime(o, (u32) etskWanyuAnim)) == NULL) {
                return -1;
            }
            if ((w->wheel = eneNewPol(o, (Gfx *) etskWanyuWheelDl)) == NULL) {
                return -1;
            }
            for (i = 0; i < 5; i++) {
                eneMakeEx(o, ENE_WANYU_FIRE_MAIN, w->e.zone, w->e.x, w->e.y, w->e.z, 2, (u32 (*)(void *, void *, void *)) i);
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 100, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 130, 260, -130, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->e.shadow = (Obj *) shadowCreate()) == NULL) {
                return -1;
            }
            w->e.halves[0] = (u32) etskWanyuHalf0Dl;
            w->e.halves[1] = (u32) etskWanyuHalf1Dl;
            w->e.behave = 0x400000;
            w->dir = 1;
            w->pl = 0;
            w->spin = 0.0f;
            if (eneNearestPlWay(o) == 4) {
                w->angle = 0x82E;
            } else {
                w->angle = 0x105C;
            }
            w->angVel = 0;
            w->baseX = w->e.x;
            w->e.x = sinf(w->angle / 1000.0f) * 40.0f + w->baseX;
            w->baseY = w->e.y - cosf(w->angle / 1000.0f) * 40.0f;
            w->e.rot[1] = eneCourseAngle(o, 8);
            w->e.state = 0;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*wanyuWait)(o);
                    break;
                case 1:
                    (*wanyuSwingLeft)(o);
                    break;
                case 2:
                    (*wanyuSwingRight)(o);
                    break;
            }
            if (fabs(w->spin) >= 0.3) {
                w->dir = -w->dir;
            }
            w->spin += 0.0025 * w->dir;
            w->wheel->rot[0] += w->spin;
            if (eneAfterProc(o) == -1) {
                if (w->e.hitKind == 0x12 || w->e.hitKind == 0x13 || w->e.hitKind == 0x14) {
                    eneMakeFuttobiHahen(o, w->e.dmg, (u32) w->wheel->dl2, 1, 0);
                }
                PRINTF("Etsk_wanyu_main: 死亡\n");
                return -1;
            }
            if (w->pl != 0) {
                pl = w->pl;
                if (plIsInPlay(pl)) {
                    plAskPos(gGameWork.players[w->pl - 1], &px, &py, &pz, &zone);
                    if (zone == w->e.zone) {
                        d = px - w->e.x;
                        if (fabsf(d) > 50.0f) {
                            d = ((d < 0.0f) ? -1 : 1) * 50;
                        }
                        ((AnimeFrame *) w->e.model)->frame = 11.0 - (f32) (d * 5.0 / 50.0);
                    }
                }
            }
            w->wheel->pos[0] = ((AnimeFrame *) w->e.model)->pos[0];
            w->wheel->pos[1] = ((AnimeFrame *) w->e.model)->pos[1];
            w->wheel->pos[2] = ((AnimeFrame *) w->e.model)->pos[2];
            w->wheel->rot[1] = ((AnimeFrame *) w->e.model)->rot[1] - 1.570796371f;
            break;
        case 1:
            if (w->e.model != NULL) {
                frmDestroy(w->e.model);
            }
            if (w->wheel != NULL) {
                frmDestroy(&w->wheel->f);
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

/* State 0: waits for a player within 80, then starts the swing from the end it hangs at. */
void wanyuWait(Obj *o) {
    Wanyu *w = WANYU(o);

    if ((w->pl = eneNearestPlInR(o, 80.0f)) != 0) {
        if (w->angle >= 0xC46) {
            w->e.state = 1;
        } else {
            w->e.state = 2;
        }
    }
}

/* State 1: the arm turns toward 0x105C (240 degrees, the -x end), up to 50 a step; past it, it
 * slows to a stop and turns back (state 2). */
void wanyuSwingLeft(Obj *o) {
    s32 pad;
    Wanyu *w = WANYU(o);

    w->angle += w->angVel;
    if (w->angle >= 0x105C) {
        w->angVel -= 3;
        if (w->angVel <= 0) {
            w->angVel = 0;
            w->e.state = 2;
        }
    } else {
        w->angVel += 3;
        if (w->angVel >= 50) {
            w->angVel = 50;
        }
    }
    w->e.x = sinf(w->angle / 1000.0f) * 40.0f + w->baseX;
    w->e.y = cosf(w->angle / 1000.0f) * 40.0f + w->baseY;
}

/* State 2: the mirror, toward 0x82E (120 degrees, the +x end). */
void wanyuSwingRight(Obj *o) {
    s32 pad;
    Wanyu *w = WANYU(o);

    w->angle += w->angVel;
    if (w->angle <= 0x82E) {
        w->angVel += 3;
        if (w->angVel >= 0) {
            w->angVel = 0;
            w->e.state = 1;
        }
    } else {
        w->angVel -= 3;
        if (w->angVel <= -50) {
            w->angVel = -50;
        }
    }
    w->e.x = sinf(w->angle / 1000.0f) * 40.0f + w->baseX;
    w->e.y = cosf(w->angle / 1000.0f) * 40.0f + w->baseY;
}

/* "fire": a flame that circles its wanyu at 12, evenly spaced among its siblings. */
s32 wanyuFireMain(Obj *o, s16 msg, void *arg) {
    WanyuFire *w = FIRE(o);
    f32 step;
    f32 rot;
    s32 pad[3];
    f32 r;
    f32 h;

    switch (msg) {
        case 2:
            o->name = "fire";
            w->index = eneSetup(o, arg);
            if ((w->e.model = (Frame *) enePolModelMake(o, (Gfx *) wanyuFireDl, NULL, 1)) == NULL) {
                return -1;
            }
            floadGetSetRam((void **) ((PolFrame *) w->e.model)->segs, 0x125);
            ((PolFrame *) w->e.model)->scale[2] = 0.1f;
            ((PolFrame *) w->e.model)->scale[0] = ((PolFrame *) w->e.model)->scale[1] =
                ((PolFrame *) w->e.model)->scale[2];
            ((PolFrame *) w->e.model)->flags = 2;
            fadeAnimStart(&w->anim, wanyuFireAnim, 0, &((PolFrame *) w->e.model)->segs[3],
                          floadGetSetFile(0x125, 0));
            break;
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            fadeAnimStep(&w->anim);
            step = 6.283185482f / tskCountChildren(o->parent) * w->index;
            rot = WANYU(o->parent)->wheel->rot[0];
            r = sinf(rot + step) * 12.0f;
            h = cosf(rot + step) * 12.0f;
            w->e.x = sinf(WANYU(o->parent)->e.rot[1] - eneCourseAngle(o, 1)) * r + WANYU(o->parent)->e.x;
            w->e.z = cosf(WANYU(o->parent)->e.rot[1] - eneCourseAngle(o, 1)) * r + WANYU(o->parent)->e.z;
            w->e.y = WANYU(o->parent)->e.y + h;
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_wanyu_fire: 死亡\n");
                return -1;
            }
            break;
        case 3:
            ((PolFrame *) w->e.model)->dl = eneDlEnvPrimColor(o, 255, 191, 63, 127, 0, 0, 255);
            break;
        case 1:
            if (w->e.model != NULL) {
                frmDestroy(w->e.model);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
