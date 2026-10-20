#include "tlb.h"

/* "index": a still "anime" model, its heading and its animation frame from the placement. */
typedef struct IndexObj {
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
    /* 0x34 */ AnimeFrame *anime;
} IndexObj;

extern u8 indexAnim[];

s32 indexProc(Obj *obj, s16 msg, void *arg) {
    IndexObj *w = (IndexObj *) FX(obj);
    s32 pad;
    s32 data;

    switch (msg) {
        case 2:
            obj->name = "index";
            data = eneSetup(obj, (EneArg *) arg);
            w->anime = eneNewAnime(obj, (u32) indexAnim);
            if (w->anime == NULL) {
                return -1;
            }
            w->anime->rot[1] = (f32) (data >> 16) * 3.141592741f / 512;
            w->anime->frame = (f32) (data & 0xFF) / 2;
            break;
        case 1:
            frmDestroy(&w->anime->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
