#include "tlb.h"

/* File 158: "konaki" (the crying old man yokai, konaki-jiji: placed as a model that bobs or as one
 * of two still pictures) and the "blood" it is drawn with. Its death print still says "hyoro". */

#define sinf (*sinf)

extern u8 konakiType1Anim[]; /* type 1: its animation */
extern u8 konakiType2PictureDl[]; /* type 2: its picture */
extern u8 konakiType3PictureDl[]; /* type 3: its picture */
extern u8 konakiBloodDl[]; /* the blood */

/* The konaki's work, at its task + 0x20: an enemy's (EneObj) up to 0x4C. */
typedef struct Konaki {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ Frame *model; /* type 1 an AnimeFrame, types 2 and 3 a PolFrame */
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ MPolFrame *shadow;
    /* 0x40 */ u32 halves[2];
    /* 0x48 */ u32 behave;
    /* 0x4C */ u8 type; /* its placement's type */
    /* 0x4D */ u8 pad4D[3];
    /* 0x50 */ f32 phase; /* type 1: its bob's angle */
    /* 0x54 */ f32 baseY; /* type 1: the height it bobs below */
} Konaki;

/* The blood's work. */
typedef struct KonakiBlood {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
} KonakiBlood;

#define KONAKI(obj) ((Konaki *) FX(obj))
#define KONAKIBLOOD(obj) ((KonakiBlood *) FX(obj))

s32 konakiMain(Obj *o, s16 msg, void *arg) {
    Konaki *w = KONAKI(o);

    switch (msg) {
        case 2:
            o->name = "konaki";
            w->type = eneSetup(o, arg);
            if (w->type == 1) {
                if ((w->model = (Frame *) eneNewAnime(o, (u32) konakiType1Anim)) == NULL) {
                    return -1;
                }
                w->rot[0] = -1.570796371f;
                w->rot[1] = eneCourseAngle(o, 1) + 0.7853981853f;
                w->baseY = w->y;
                w->phase = 0.0f;
            } else if (w->type == 2) {
                if ((w->model = (Frame *) eneNewPol(o, (Gfx *) konakiType2PictureDl)) == NULL) {
                    return -1;
                }
                w->rot[0] = -0.7853981853f;
                w->rot[1] = eneCourseAngle(o, 4) - 0.5235987902f;
            } else if (w->type == 3) {
                if ((w->model = (Frame *) eneNewPol(o, (Gfx *) konakiType3PictureDl)) == NULL) {
                    return -1;
                }
                w->y -= 2.0;
                w->rot[0] = -0.7853981853f;
                w->rot[1] = eneCourseAngle(o, 4) + 0.5235987902f;
            }
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            if (w->type == 1) {
                w->y = w->baseY - (f64) sinf(w->phase);
                w->phase += 0.1;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_hyoro_main: 死亡\n");
                return -1;
            }
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(w->model);
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

s32 konakiBloodMain(Obj *o, s16 msg, void *arg) {
    KonakiBlood *w = KONAKIBLOOD(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "blood";
            eneSetup(o, arg);
            if ((w->model = enePolModelMake(o, (Gfx *) konakiBloodDl, NULL, 1)) == NULL) {
                return -1;
            }
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 0.6f;
            w->rot[0] = -1.570796371f;
            w->y += 1.5;
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_konaki_blood: 死亡\n");
                return -1;
            }
        case 3:
            w->model->dl = eneDlEnvPrimColor(o, 0x7F, 0, 0, 0x7F, 0, 0, 0xFF);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy((Frame *) w->model);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
