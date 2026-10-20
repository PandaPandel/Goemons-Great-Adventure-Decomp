#include "tlb.h"

/* The "s_stone" enemy kind (archive file 151): a stone that waits for a player within 120, then
 * rolls along x at -1.8, turning by the distance over its radius; on course 1 it fades out and
 * dies before x 1490. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 etskSStoneDl[];

/* An s_stone's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct SStone {
    /* 0x00 */ EneObj e;
    /* 0x4C */ u8 pad4C[2];
    /* 0x4E */ s16 alpha;
} SStone;

#define SSTONE(obj) ((SStone *) FX(obj))
#define POL(w) ((PolFrame *) (w)->e.model)
/* Its radius: 13 times its scale's ten. Its scale is read as a Vec3f: IDO sorts a commutative
 * operation's operands when one is an array element. */
#define RADIUS(w) (13.0f * (((Vec3f *) POL(w)->scale)->x * 10.0f))

void sStoneWait(Obj *o);
void sStoneRoll(Obj *o);

s32 Etsk_s_stone_main(Obj *o, s16 msg, void *arg) {
    SStone *w = SSTONE(o);

    switch (msg) {
        case 2:
            o->name = "s_stone";
            eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewPol(o, (Gfx *) etskSStoneDl)) == NULL) {
                return -1;
            }
            POL(w)->scale[2] = 0.07f;
            POL(w)->scale[0] = POL(w)->scale[1] = POL(w)->scale[2];
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 100, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 130, 260, -130, 0, 0)) == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                PRINTF("Etsk_s_stone: 地面にセット失敗\n");
                return -1;
            }
            w->e.behave = 0x400801;
            if (enePlDirX(o, 0) != 4) {
                w->e.behave |= 0x40000;
            }
            w->alpha = 0xFF;
            w->e.state = 0;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*sStoneWait)(o);
                    break;
                case 1:
                    (*sStoneRoll)(o);
                    break;
            }
            if (w->alpha < 0xFF) {
                frmMove(w->e.model, gFrmLayer8);
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_s_stone_main: 死亡\n");
                return -1;
            }
            w->e.rot[1] = eneCourseAngle(o, 4);
            w->e.rot[0] += -w->e.vel[0] / RADIUS(w);
            POL(w)->pos[1] = w->e.y + RADIUS(w);
            /* fallthrough */
        case 3:
            POL(w)->dl = gGfxDlHead;
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
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

/* State 0: it waits for a player within 120, then rolls (state 1). */
void sStoneWait(Obj *o) {
    SStone *w = SSTONE(o);

    if (eneNearestPlInR(o, 120.0f) != 0) {
        w->e.state = 1;
    }
}

/* State 1: it rolls along x at -1.8; on course 1, before x 1490, it fades out and dies. */
void sStoneRoll(Obj *o) {
    SStone *w = SSTONE(o);

    w->e.vel[0] = -1.799999952f;
    if (w->e.zone == 1 && w->e.x < 1490.0f) {
        w->alpha -= 0x20;
        if (w->alpha <= 0) {
            w->alpha = 0;
            w->e.behave |= 0x40000;
        }
    }
}
