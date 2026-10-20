#include "tlb.h"

/* The togeita (a spiked board that slides in and out): two attack boxes on a pol model. */

typedef struct TogeitaWork {
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
    /* 0x38 */ ACollFrame *box[2];
    /* 0x40 */ f32 base; /* where it swings from */
    /* 0x44 */ f32 ofs; /* how far out it is */
    /* 0x48 */ u8 mode; /* 0 out, 1 wait, 2 in, 3 wait */
    /* 0x4A */ s16 timer;
} TogeitaWork;

/* acollAtkAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy,
                                     s16 oz, s32 a);

extern Frame *gFrmLayer3;
extern Gfx togeitaDl[];

ACollFrame *togeitaAtkMake(Frame *model, f32 x, f32 y, f32 z) {
    ACollFrame *box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, model, 80, x, y, z, 0);

    if (box == NULL) {
        return NULL;
    }
    box->kind = 120;
    return box;
}

/* The up-and-down togeita: out 20 above its place and back, waiting 30 steps at each end.
 * Message 2's value from eneSetup (its starting offset) is held in the loop index i: IDO keeps
 * it in i's home (0x3C) as the game does; a separate local gets s0. */
s32 togeitaUpDownProc(Obj *o, s16 msg, void *arg) {
    TogeitaWork *w = (TogeitaWork *) FX(o);
    Obj *by;
    s32 i;

    switch (msg) {
        case 2:
            o->name = "togeita";
            i = eneSetup(o, arg);
            w->pol = eneNewPol(o, togeitaDl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayer3);
            w->box[0] = togeitaAtkMake(&w->pol->f, 0.0f, 0.0f, -80.0f);
            w->box[1] = togeitaAtkMake(&w->pol->f, 0.0f, 0.0f, 80.0f);
            w->rot[1] = vramCourseHeading(w->zone, w->x) - 3.141592741f;
            w->base = w->y;
            w->ofs = i;
            break;
        case 1:
            for (i = 0; i < 2; i++) {
                frmDestroy(&w->box[i]->frame);
            }
            frmDestroy(&w->pol->f);
            break;
        case 0:
            w->timer++;
            switch (w->mode) {
                case 0:
                    w->ofs += 2.0f;
                    if (w->ofs >= 20.0f) {
                        w->timer = 0;
                        w->mode = 1;
                    }
                    break;
                case 1:
                    if (w->timer >= 31) {
                        w->mode = 2;
                    }
                    break;
                case 2:
                    w->ofs -= 2.0f;
                    if (w->ofs <= -20.0f) {
                        w->timer = 0;
                        w->mode = 3;
                    }
                    break;
                case 3:
                    if (w->timer >= 31) {
                        w->mode = 0;
                    }
                    break;
            }
            w->y = w->base + w->ofs;
            for (i = 0; i < 2; i++) {
                if (w->box[i] != NULL && acollAtkWasHit(w->box[i]) == 1) {
                    by = (Obj *) w->box[i]->hitPower;
                    if (by != NULL) {
                        tskSendMsg(by, 7, o);
                    }
                }
            }
            eneSetModelOnCourse(o, &w->pol->f);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The sideways togeita: lying flat, out 30 along the course and back. */
s32 togeitaSideProc(Obj *o, s16 msg, void *arg) {
    TogeitaWork *w = (TogeitaWork *) FX(o);
    Obj *by;
    s32 i;

    switch (msg) {
        case 2:
            o->name = "togeita";
            i = eneSetup(o, arg);
            w->pol = eneNewPol(o, togeitaDl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayer3);
            w->box[0] = togeitaAtkMake(&w->pol->f, 0.0f, 0.0f, -80.0f);
            w->box[1] = togeitaAtkMake(&w->pol->f, 0.0f, 0.0f, 80.0f);
            w->rot[0] = 1.570796371f;
            w->rot[1] = vramCourseHeading(w->zone, w->x) - 3.141592741f;
            w->base = w->x;
            w->ofs = i;
            break;
        case 1:
            for (i = 0; i < 2; i++) {
                frmDestroy(&w->box[i]->frame);
            }
            frmDestroy(&w->pol->f);
            break;
        case 0:
            w->timer++;
            switch (w->mode) {
                case 0:
                    w->ofs += 2.0f;
                    if (w->ofs >= 30.0f) {
                        w->timer = 0;
                        w->mode = 1;
                    }
                    break;
                case 1:
                    if (w->timer >= 31) {
                        w->mode = 2;
                    }
                    break;
                case 2:
                    w->ofs -= 2.0f;
                    if (w->ofs <= -30.0f) {
                        w->timer = 0;
                        w->mode = 3;
                    }
                    break;
                case 3:
                    if (w->timer >= 31) {
                        w->mode = 0;
                    }
                    break;
            }
            w->x = w->base + w->ofs;
            for (i = 0; i < 2; i++) {
                if (w->box[i] != NULL && acollAtkWasHit(w->box[i]) == 1) {
                    by = (Obj *) w->box[i]->hitPower;
                    if (by != NULL) {
                        tskSendMsg(by, 7, o);
                    }
                }
            }
            eneSetModelOnCourse(o, &w->pol->f);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
