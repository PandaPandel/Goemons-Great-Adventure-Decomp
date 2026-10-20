#include "tlb.h"

/* The "abura" enemy kind (archive file 165): one that walks along x, turning back at walls,
 * hurrying when a player is within 45; within 20 it stops and strikes (its second attack box
 * is live from frame 7 to 9 of the strike), then turns and walks on. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 etskAburaAnim[];
extern u8 etskAburaWalkAnim[];
extern u8 etskAburaHalf0Dl[];
extern u8 etskAburaHalf1Dl[];

/* An abura's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Abura {
    /* 0x00 */ EneObj e;
    /* 0x4C */ u8 way; /* 1 or 4: the way it walks */
} Abura;

#define ABURA(obj) ((Abura *) FX(obj))

void aburaWalk(Obj *o);
void aburaStrike(Obj *o);
void aburaCreep(Obj *o);

s32 Etsk_abura_main(Obj *o, s16 msg, void *arg) {
    Abura *w = ABURA(o);
    ACollFrame *box;
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "abura";
            eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewAnime(o, (u32) etskAburaAnim)) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 40, 80, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((box = ATK_ADD_YZ(&w->e.atk->frame, w->e.model, 30, 30, 150, 0)) == NULL) {
                return -1;
            }
            box->kind = 120;
            ((ACollFrame *) w->e.atk->frame.child)->flags &= ~2;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 60, 160, 0, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->e.shadow = (Obj *) shadowCreate()) == NULL) {
                return -1;
            }
            w->e.halves[0] = (u32) etskAburaHalf0Dl;
            w->e.halves[1] = (u32) etskAburaHalf1Dl;
            if (eneSetOnEarth(o) == -1) {
                return -1;
            }
            w->e.behave = 0xC04280;
            w->way = enePlDirX(o, 0);
            w->e.state = 0;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*aburaWalk)(o);
                    break;
                case 1:
                    (*aburaStrike)(o);
                    break;
                case 2:
                    (*aburaCreep)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_abura_main: 死亡\n");
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

/* State 0: walks, turning back at a wall; faster with a player within 45; strikes at 20. */
void aburaWalk(Obj *o) {
    s32 pad;
    Abura *w = ABURA(o);
    f32 speed = 1.0f;

    if (w->e.hit & 6) {
        if (w->way == 4) { w->way = 1; } else { w->way = 4; }
    }
    w->e.vel[0] = (w->way == 4) ? -0.4 : 0.4;
    if (enePlInRangeCheck(o, 45.0f, 0)) {
        speed *= 2.5;
        w->e.vel[0] *= 2.5;
    }
    if (eneAnimLoop(o, (AnimeFrame *) w->e.model, speed)) {
        if (enePlInRangeCheck(o, 20.0f, 0)) {
            ((AnimeFrame *) w->e.model)->anim = etskAburaWalkAnim;
            ((AnimeFrame *) w->e.model)->frame = 0.0f;
            w->e.state = 1;
            w->e.vel[0] = 0.0f;
        }
    }
}

/* State 1: the strike; its second attack box is live from frame 7 to 9. At the end it turns
 * and walks again. */
void aburaStrike(Obj *o) {
    Abura *w = ABURA(o);

    if (eneAnimPlayOnce(o, (AnimeFrame *) w->e.model, 0.5f)) {
        if (w->way == 4) { w->way = 1; } else { w->way = 4; }
        ((AnimeFrame *) w->e.model)->anim = etskAburaAnim;
        ((AnimeFrame *) w->e.model)->frame = 0.0f;
        w->e.state = 0;
    }
    if (((AnimeFrame *) w->e.model)->frame >= 7.0 && ((AnimeFrame *) w->e.model)->frame < 9.0) {
        ((ACollFrame *) w->e.atk->frame.child)->flags |= 2;
    } else {
        ((ACollFrame *) w->e.atk->frame.child)->flags &= ~2;
    }
}

/* State 2: walks slowly. */
void aburaCreep(Obj *o) {
    Abura *w = ABURA(o);

    w->e.vel[0] = (w->way == 4) ? -0.2 : 0.2;
}
