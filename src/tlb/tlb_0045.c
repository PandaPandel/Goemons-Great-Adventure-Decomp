#include "tlb.h"

/* "bpole": a pole with four attack boxes up its length, hurting what touches it. */

/* An enemy's work (EneObj) to 0x34, then its own. */
typedef struct BpoleWork {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ Frame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ ACollFrame *box[4];
} BpoleWork;

/* acollAtkAddYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz,
                                    s32 a);

extern Frame *gFrmLayer3;
extern Gfx bpoleDl[];

/* The bpole task: its value from eneSetup turns it when it stands off the course (tlb_0048's
 * "edopole", gimEdopoleProc, is its twin without the turn). msg 0's loop is written in edopole's
 * form ("continue" on an empty box, then "== 1 && (Obj *) hitPower != NULL"). */
s32 bpoleProc(Obj *obj, s16 msg, void *arg) {
    BpoleWork *w = (BpoleWork *) FX(obj);
    s32 angle;
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "bpole";
            angle = eneSetup(obj, arg);
            w->pol = eneNewPol(obj, bpoleDl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayer3);
            if (w->zone) {
                w->pol->rot[1] = vramCourseHeading(w->zone, w->x) - 1.570796371f;
            } else {
                w->pol->rot[1] = angle * 3.141592741f / 512;
            }
            for (i = 0; i < 4; i++) {
                w->box[i] = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->pol->f, 40, 0,
                                                           i * 40 - 60, 0);
                if (w->box[i] == NULL) {
                    return 0;
                }
                w->box[i]->kind = 0x6A;
            }
            break;
        case 1:
            for (i = 0; i < 4; i++) {
                frmDestroy(&w->box[i]->frame);
            }
            frmDestroy(&w->pol->f);
            break;
        case 0:
            for (i = 0; i < 4; i++) {
                if (w->box[i] == NULL) {
                    continue;
                }
                if (acollAtkWasHit(w->box[i]) == 1 && (Obj *) w->box[i]->hitPower != NULL) {
                    tskSendMsg((Obj *) w->box[i]->hitPower, 7, obj);
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
