#include "tlb.h"

/* File 144: "echo", an enemy that only takes hits: a model with an attack box and a damage box
 * that goes off for 15 steps after each hit (with a sound) but for hits of kinds 0xA, 0xD and
 * 0xE. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

/* The echo's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Echo {
    /* 0x00 */ EneObj e;
    /* 0x4C */ s16 timer; /* steps its damage box stays off */
} Echo;

#define ECHO(obj) ((Echo *) FX(obj))

s32 Etsk_echo_main(Obj *o, s16 msg, void *arg) {
    Echo *w = ECHO(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "echo";
            eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewPol(o, NULL)) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 200, 100, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 104;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 220, 300, 300, 0, 0)) == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                PRINTF("Etsk_echo_main: 地面にセット失敗\n");
                return -1;
            }
            w->e.behave = 1;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            w->timer--;
            if (w->timer >= 0) {
                w->e.dmg->flags &= ~2;
            } else {
                w->e.dmg->flags |= 2;
                w->timer = 0;
            }
            if (w->e.hit & 0x10) {
                if (w->e.hitKind != 0xD && w->e.hitKind != 0xA && w->e.hitKind != 0xE) {
                    eneSetSd(o, 0x2C1);
                    w->timer = 15;
                }
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_echo_main: 死亡\n");
                return -1;
            }
            break;
        case 1:
            /* each test and its call on one line (IDO schedules by line; tlb_0084's form) */
            if (w->e.model != NULL) { frmDestroy(w->e.model); }
            if (w->e.atk != NULL) { frmDestroy(&w->e.atk->frame); }
            if (w->e.dmg != NULL) { frmDestroy(&w->e.dmg->frame); }
            PRINTF("");
            PRINTF("");
            PRINTF("");
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
