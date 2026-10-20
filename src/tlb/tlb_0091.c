#include "tlb.h"

/* The "hyoro" enemy kind (archive file 91): a floating thing (a polygon model with a glowing
 * underside, its texture an animated set) that bobs up and down on a sine and drifts along x
 * toward the nearest player, turning back every three bob cycles. */

/* tlb.h does not declare sinf: libultra's, called far. */
#define sinf (*sinf)

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 hyoroUnderDl[];
extern u8 hyoroDl[];

/* Its texture's animation: 16 pictures, 2 frames each (fadeAnimStart). */
extern u8 hyoroTex0[];
extern u8 hyoroTex2[];
extern u8 hyoroTex5[];
AnimFrame hyoroTexAnim[] = {
    { (u32) hyoroTex0, 2 },
    { 0x03000D80, 2 },
    { (u32) hyoroTex2, 2 },
    { 0x03001D80, 2 },
    { 0x03002580, 2 },
    { (u32) hyoroTex5, 2 },
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

/* A hyoro's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Hyoro {
    /* 0x00 */ EneObj e;
    /* 0x4C */ Anim anim; /* its texture's animation */
    /* 0x5C */ s16 baseY; /* the height it bobs about */
    /* 0x5E */ s16 phase; /* the bob's phase, in tenths of a radian */
} Hyoro;

#define HYORO(obj) ((Hyoro *) FX(obj))
#define POL(w) ((PolFrame *) (w)->e.model)

void hyoroStart(Obj *o);
void hyoroFloat(Obj *o);

#define DPRINTF

s32 hyoroProc(Obj *o, s16 msg, void *arg) {
    Hyoro *w = HYORO(o);
    PolFrame *under;

    switch (msg) {
        case 2:
            o->name = "hyoro";
            eneSetup(o, arg);
            if ((w->e.model = (Frame *) enePolModelMake(o, (Gfx *) hyoroDl, NULL, 1)) == NULL) {
                return -1;
            }
            floadGetSetRam((void **) POL(w)->segs, 0x125);
            POL(w)->scale[2] = 0.25f;
            POL(w)->scale[0] = POL(w)->scale[1] = POL(w)->scale[2];
            POL(w)->flags |= 2;
            fadeAnimStart(&w->anim, hyoroTexAnim, 0, &POL(w)->segs[3], floadGetSetFile(0x125, 0));
            if ((under = enePolModelUnder(o, (Gfx *) hyoroUnderDl, w->e.model)) == NULL) {
                return -1;
            }
            under->scale[2] = 0.25f;
            under->scale[0] = under->scale[1] = under->scale[2];
            under->flags |= 2;
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 20, 50, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 40, 100, 15, 0, 0)) == NULL) {
                return -1;
            }
            w->e.behave = 0x419000;
            w->phase = 93;
            w->e.state = 0;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            fadeAnimStep(&w->anim);
            switch (w->e.state) {
                case 0:
                    (*hyoroStart)(o);
                    break;
                case 1:
                    (*hyoroFloat)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                DPRINTF("Etsk_hyoro_main: 死亡\n");
                return -1;
            }
            ((PolFrame *) POL(w)->f.child)->pos[0] = POL(w)->pos[0];
            ((PolFrame *) POL(w)->f.child)->pos[1] = POL(w)->pos[1];
            ((PolFrame *) POL(w)->f.child)->pos[2] = POL(w)->pos[2];
            /* fallthrough */
        case 3:
            POL(w)->dl = eneDlEnvPrimColor(o, 0xFF, 0xBF, 0x3F, 0x7F, 0, 0, 0xFF);
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

/* State 0: drifts toward the nearest player and starts bobbing from where it is. */
void hyoroStart(Obj *o) {
    Hyoro *w = HYORO(o);
    s32 way = eneNearestPlWay(o);

    if (way == 4) {
        w->e.vel[0] = -0.6f;
    } else if (way == 1) {
        w->e.vel[0] = 0.6f;
    }
    w->baseY = w->e.y;
    w->e.state = 1;
}

/* State 1: bobs 6 up and down; every three cycles (6 pi) it turns back along x. */
void hyoroFloat(Obj *o) {
    Hyoro *w = HYORO(o);

    w->e.y = sinf(w->phase / 10.0f) * 6.0f + w->baseY;
    w->phase++;
    if (w->phase / 10.0f > 18.84955597f) {
        w->phase -= 188;
        w->e.vel[0] *= -1.0f;
    }
}
