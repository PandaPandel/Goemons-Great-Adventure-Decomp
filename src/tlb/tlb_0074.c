#include "tlb.h"

/* "udedokei": a clock whose hand turns with the time of day (gGameWork.time). */
typedef struct UdedokeiObj {
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
} UdedokeiObj;

extern Gfx udedokeiDl[];

s32 udedokeiProc(Obj *obj, s16 msg, void *arg) {
    UdedokeiObj *w = (UdedokeiObj *) FX(obj);
    s32 pad;
    s32 data;

    switch (msg) {
        case 2:
            obj->name = "udedokei";
            data = eneSetup(obj, (EneArg *) arg);
            w->pol = eneNewPol(obj, udedokeiDl);
            if (w->pol == NULL) {
                return -1;
            }
            w->pol->rot[1] = (f32) data * 3.141592741f / 512 - 1.570796371f;
            break;
        case 1:
            frmDestroy(&w->pol->f);
            break;
        case 0:
            w->pol->rot[0] = -(f32) gGameWork.time / 786432 * 2 * PI;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "kazan": a volcano, an "anime" model with a two-frame texture animation, puffing smoke. */
typedef struct KazanObj {
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
    /* 0x34 */ u16 count;
    /* 0x38 */ AnimeFrame *anime;
    /* 0x3C */ Anim anim;
} KazanObj;

extern Frame *gFrmLayer3;
extern u8 kazanAnim[];
extern u8 kazanTex0[];
AnimFrame kazanTexAnim[] = {
    { (u32) kazanTex0, 8 },
    { 0x03000A00, 8 },
    { 0, 0 },
    { 0, 0 },
};

s32 kazanProc(Obj *obj, s16 msg, void *arg) {
    KazanObj *w = (KazanObj *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "kazan";
            eneSetup(obj, (EneArg *) arg);
            w->anime = eneNewAnime(obj, (u32) kazanAnim);
            if (w->anime == NULL) {
                return -1;
            }
            w->anime->scale[2] = 1.0f;
            w->anime->scale[0] = w->anime->scale[1] = w->anime->scale[2];
            frmMove(&w->anime->f, gFrmLayer3);
            fadeAnimStart(&w->anim, (AnimFrame *) fadeSegToRam(kazanTexAnim, obj->file), 0,
                          &w->anime->segs[3], eneGetFile(obj, 0));
            break;
        case 1:
            frmDestroy(&w->anime->f);
            break;
        case 0:
            fadeAnimStep(&w->anim);
            if ((++w->count & 0xF) == 1) {
                itexPuff(w->x, w->y, w->z, 0.1f);
            }
            eneAnimLoop(obj, w->anime, 0.2f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
