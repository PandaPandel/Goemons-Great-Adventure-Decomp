#include "tlb.h"

/* "girotin": a swinging guillotine blade. */

/* acollAtkAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy,
                                     s16 oz, s32 a);

#define sinf (*sinf)

extern Frame *gFrmLayer3;
extern u8 girotinAnim[];

/* The guillotine's work ("girotin"). */
typedef struct GirotinWork {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ Frame *frame;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ AnimeFrame *model;
    /* 0x38 */ f32 speed;  /* its swing's phase per step: a full turn per period */
    /* 0x3C */ s32 timer;
    /* 0x40 */ s32 period; /* steps per swing and back */
    /* 0x44 */ ACollFrame *box[5]; /* along the blade */
} GirotinWork;

/* An attack box of radius r at (x, y, z) from a model, of kind 120. */
ACollFrame *girotinAtkMake(Frame *model, f32 r, f32 x, f32 y, f32 z) {
    ACollFrame *box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, model, r, x, y, z, 0);

    if (box == NULL) {
        return NULL;
    }
    box->kind = 120;
    return box;
}

/* "girotin": its kind's value: the period in steps in the low half, in the high half the
 * quarter period it starts at (bits 0-1) and, from 4, the blade turned the other way.
 * The hit loop is written in edopole's form (tlb_0048's gimEdopoleProc: "continue" on an empty
 * box, then "== 1 && (Obj *) hitPower != NULL"); in other forms uopt hoists the 1 into s2. */
s32 girotinProc(Obj *o, s16 msg, void *arg) {
    GirotinWork *w = (GirotinWork *) FX(o);
    s32 pad;
    s32 i;

    switch (msg) {
        case 2:
            o->name = "girotin";
            i = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) girotinAnim);
            if (w->model == NULL) {
                return -1;
            }
            frmMove(&w->model->f, gFrmLayer3);
            w->model->rot[1] = vramCourseHeading(w->zone, w->x) - 1.570796371f - 0.1963495463f;
            w->box[0] = girotinAtkMake(&w->model->f, 50.0f, 0.0f, -600.0f, 0.0f);
            w->box[1] = girotinAtkMake(&w->model->f, 40.0f, 0.0f, -584.0f, 116.0f);
            w->box[2] = girotinAtkMake(&w->model->f, 40.0f, 0.0f, -584.0f, -116.0f);
            w->box[3] = girotinAtkMake(&w->model->f, 30.0f, 0.0f, -522.0f, 203.0f);
            w->box[4] = girotinAtkMake(&w->model->f, 30.0f, 0.0f, -522.0f, -203.0f);
            if ((i >> 16) >= 4) {
                w->model->rot[1] += -1.374446869f;
            }
            w->period = i & 0xFFFF;
            w->timer = gGameFrameCount;
            w->timer += (w->period / 4) * ((i >> 16) & 3);
            w->speed = 6.283185482f / w->period;
            break;
        case 1:
            for (i = 0; i < 5; i++) {
                frmDestroy(&w->box[i]->frame);
            }
            frmDestroy(&w->model->f);
            break;
        case 0:
            w->model->rot[0] = sinf(w->speed * w->timer) * 1.04719758f;
            if (++w->timer % (w->period / 2) == 0) {
                soundIncamCam(0x225, w->model->pos[0], w->model->pos[1], w->model->pos[2]);
            }
            for (i = 0; i < 5; i++) {
                if (w->box[i] == NULL) {
                    continue;
                }
                if (acollAtkWasHit(w->box[i]) == 1 && (Obj *) w->box[i]->hitPower != NULL) {
                    tskSendMsg((Obj *) w->box[i]->hitPower, 7, o);
                }
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
