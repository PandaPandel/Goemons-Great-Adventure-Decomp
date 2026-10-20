#include "tlb.h"

/* The "spike" object kind (archive file 172): a spike (a still model) with one attack box. Its
 * placement's eneSetup value packs the box: the low half its radius, the high half its attack
 * kind. A player its box hits is sent message 7. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)

/* A frame destroyed if it was made. A macro: the game's code for the first one is scheduled as
 * one source line, as in tlb_0159. */
#define FRM_DESTROY(f) if ((f) != NULL) { frmDestroy((Frame *) (f)); }

/* A spike's work, at its task + 0x20: an enemy's (EneObj). */
#define SPIKE(obj) ((EneObj *) FX(obj))

s32 spikeProc(Obj *o, s16 msg, void *arg) {
    EneObj *w = SPIKE(o);
    s32 r;
    s32 pad;
    f32 radius;
    u8 kind;

    switch (msg) {
        case 2:
            o->name = "spike";
            r = eneSetup(o, arg);
            radius = r & 0xFFFF;
            kind = r >> 16;
            if ((w->model = (Frame *) eneNewPol(o, NULL)) == NULL) {
                return -1;
            }
            if ((w->atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->model, radius, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = kind;
            /* Two empty prints: the file's .rodata has two "" after "spike". */
            PRINTF("");
            PRINTF("");
            break;
        case 0:
            if (acollAtkWasHit(w->atk) == 1 && (Obj *) w->atk->hitPower != NULL) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, o);
            }
            break;
        case 1:
            FRM_DESTROY(w->model);
            FRM_DESTROY(w->atk);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
