#include "tlb.h"

/* The "jelly" enemy kind (archive file 153): a jelly that bounces in place, an "anime" model
 * tinted by its placement's type (1 violet, 2 blue, 3 yellow, else grey). */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 etskJellyAnim[];

/* A jelly's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Jelly {
    /* 0x00 */ EneObj e;
    /* 0x4C */ f32 floor; /* the y it was placed at: below it, it bounces again */
    /* 0x50 */ u8 type; /* its placement's type, from eneSetup: its colour */
    /* 0x51 */ u8 bouncing; /* its animation runs (down to frame 7) */
} Jelly;

#define JELLY(obj) ((Jelly *) FX(obj))
#define ANIME(w) ((AnimeFrame *) (w)->e.model)

void jellyBounce(Obj *o);

s32 Etsk_jelly_main(Obj *o, s16 msg, void *arg) {
    Jelly *w = JELLY(o);
    s32 r;
    s32 g;
    s32 b;

    switch (msg) {
        case 2:
            o->name = "jelly";
            w->type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneAnimeModelMake(o, (u32) etskJellyAnim, NULL, 1)) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 125, 125, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 200, 250, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->e.behave = 1;
            w->e.rot[1] = eneCourseAngle(o, 8);
            w->floor = w->e.y;
            ANIME(w)->frame = 7.0f;
            w->e.state = 0;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*jellyBounce)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_jelly_main: 死亡\n");
                return -1;
            }
            /* fallthrough */
        case 3:
            switch (w->type) {
                default:
                    r = 0x3F;
                    g = 0x3F;
                    b = 0x3F;
                    break;
                case 1:
                    r = 0xBF;
                    g = 0;
                    b = 0xFF;
                    break;
                case 2:
                    r = 0x3F;
                    g = 0x3F;
                    b = 0xFF;
                    break;
                case 3:
                    r = 0xFF;
                    g = 0xFF;
                    b = 0x3F;
                    break;
            }
            ANIME(w)->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, SHADE, 0,
                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, SHADE, 0);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 255, 0);
            gDPSetEnvColor(gGfxDlHead++, r, g, b, 255);
            gSPEndDisplayList(gGfxDlHead++);
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

/* State 0: it bounces. While its animation runs, at frame 4 it jumps (0.5 up); at frame 7 it
 * stops. It falls at up to 0.2, and below its floor it bounces again. */
void jellyBounce(Obj *o) {
    Jelly *w = JELLY(o);
    f32 step = 0.1f; /* a literal folds "4.0 - step" into a double constant */

    if (w->bouncing) {
        ANIME(w)->frame -= step;
        if (ANIME(w)->frame < 0.0f) {
            ANIME(w)->frame += 11.0f;
        }
        if (ANIME(w)->frame <= 4.0 && ANIME(w)->frame > 4.0 - step) {
            w->e.vel[1] = 0.5f;
        }
        if (ANIME(w)->frame <= 7.0 && ANIME(w)->frame > 7.0 - step) {
            w->bouncing = 0;
        }
    }
    w->e.vel[1] -= 0.01;
    if (w->e.vel[1] <= -0.2) {
        w->e.vel[1] = -0.2f;
    }
    if (w->e.y < w->floor) {
        w->bouncing = 1;
    }
}
