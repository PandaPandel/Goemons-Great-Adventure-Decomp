#include "tlb.h"

/* "doors": a door (21 kinds of model, by its placement's kind) that swings open when it is hit
 * and shut again when hit once more. Kinds 0-9 are drawn in another layer (gFrmLayer3). A door
 * with event 0x29 set and 0xB6 not is locked (step 3) until that changes. */

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF


/* A door's work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct DoorsW {
    /* 0x00 */ u8 state; /* 0 shut, 1 opening, 2 shutting, 3 locked */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *box; /* what is hit */
    /* 0x38 */ u8 flags; /* 1: drawn in gFrmLayer3's layer */
} DoorsW;

#define DOORS(obj) ((DoorsW *) FX(obj))

/* acollAtkAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy,
                                     s16 oz, s32 a);

extern Frame *gFrmLayer3;
extern u8 doorsKind0Anim[];
extern u8 doorsKind10Anim[];
extern u8 doorsKind13Anim[];
extern u8 doorsKind1Anim[];
extern u8 doorsKind2Anim[];
extern u8 doorsKind14Anim[];
extern u8 doorsKind3Anim[];
extern u8 doorsKind15Anim[];
extern u8 doorsKind4Anim[];
extern u8 doorsKind16Anim[];
extern u8 doorsKind5Anim[];
extern u8 doorsKind17Anim[];
extern u8 doorsKind6Anim[];
extern u8 doorsKind18Anim[];
extern u8 doorsKind7Anim[];
extern u8 doorsKind19Anim[];
extern u8 doorsKind8Anim[];
extern u8 doorsKind9Anim[];
extern u8 doorsKind20Anim[]; /* kinds 10-20 are file 529's (file set 0x47): doorsKind9Anim's address there */
extern u8 doorsKind11Anim[];

s32 doorsProc(Obj *o, s16 msg, void *arg) {
    DoorsW *w = DOORS(o);
    s32 pad[2];
    DoorsData *d;

    switch (msg) {
        case 2:
            o->name = "doors";
            d = (DoorsData *) stageAddrToRam(eneSetup(o, arg));
            switch (d->kind) {
                case 0:
                    w->model = eneNewAnime(o, (u32) doorsKind0Anim);
                    w->flags |= 1;
                    break;
                case 1:
                    w->model = eneNewAnime(o, (u32) doorsKind1Anim);
                    w->flags |= 1;
                    break;
                case 2:
                    w->flags |= 1;
                    w->model = eneNewAnime(o, (u32) doorsKind2Anim);
                    break;
                case 3:
                    w->flags |= 1;
                    w->model = eneNewAnime(o, (u32) doorsKind3Anim);
                    break;
                case 4:
                    w->flags |= 1;
                    w->model = eneNewAnime(o, (u32) doorsKind4Anim);
                    break;
                case 5:
                    w->flags |= 1;
                    w->model = eneNewAnime(o, (u32) doorsKind5Anim);
                    break;
                case 6:
                    w->flags |= 1;
                    w->model = eneNewAnime(o, (u32) doorsKind6Anim);
                    break;
                case 7:
                    w->flags |= 1;
                    w->model = eneNewAnime(o, (u32) doorsKind7Anim);
                    break;
                case 8:
                    w->flags |= 1;
                    w->model = eneNewAnime(o, (u32) doorsKind8Anim);
                    break;
                case 9:
                    w->flags |= 1;
                    w->model = eneNewAnime(o, (u32) doorsKind9Anim);
                    break;
                case 10:
                    w->model = eneNewAnime(o, (u32) doorsKind10Anim);
                    break;
                case 11:
                    w->model = eneNewAnime(o, (u32) doorsKind11Anim);
                    break;
                case 12:
                    w->model = eneNewAnime(o, (u32) doorsKind11Anim);
                    break;
                case 13:
                    w->model = eneNewAnime(o, (u32) doorsKind13Anim);
                    break;
                case 14:
                    w->model = eneNewAnime(o, (u32) doorsKind14Anim);
                    break;
                case 15:
                    w->model = eneNewAnime(o, (u32) doorsKind15Anim);
                    break;
                case 16:
                    w->model = eneNewAnime(o, (u32) doorsKind16Anim);
                    break;
                case 17:
                    w->model = eneNewAnime(o, (u32) doorsKind17Anim);
                    break;
                case 18:
                    w->model = eneNewAnime(o, (u32) doorsKind18Anim);
                    break;
                case 19:
                    w->model = eneNewAnime(o, (u32) doorsKind19Anim);
                    break;
                case 20:
                    w->model = eneNewAnime(o, (u32) doorsKind20Anim);
                    break;
                default:
                    DPRINTF("doors_proc: Warning: door kind Error!!\n");
                    w->model = eneNewAnime(o, (u32) doorsKind0Anim);
                    break;
            }
            if (w->model == NULL) {
                return -1;
            }
            if (w->flags & 1) {
                frmMove(&w->model->f, gFrmLayer3);
            }
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 1.0f;
            eneSetAngle(o, d->angle * 180.0f / 3.141592741f);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("doors1_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, (Frame *) w->model, 20, 0, 20, 0, 0);
            if (w->box == NULL) {
                return -1;
            }
            w->box->kind = 0x66;
            if (swWorkTest(0x29) && !swWorkTest(0xB6)) {
                w->state = 3;
            }
            DPRINTF("doors1_proc: ドアセットＯＫ\n");
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->box != NULL) {
                frmDestroy(&w->box->frame);
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (acollAtkWasHit(w->box) == 1) {
                        soundLocationCam(0x2BA, w->model->pos[0], w->model->pos[1], w->model->pos[2]);
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (acollAtkWasHit(w->box) == 1) {
                        eneAnimPlayOnce(o, w->model, 0.15f);
                    } else {
                        soundLocationCam(0x2BA, w->model->pos[0], w->model->pos[1], w->model->pos[2]);
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (acollAtkWasHit(w->box) != 1) {
                        eneAnimPlayOnce(o, w->model, -0.15f);
                    } else {
                        w->state = 0;
                    }
                    break;
                case 3:
                    if (swWorkTest(0xB6)) {
                        w->state = 0;
                    }
                    if (!swWorkTest(0x29)) {
                        w->state = 0;
                    }
                    break;
            }
            if (w->model != NULL) {
                eneSetModel(o, &w->model->f);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
