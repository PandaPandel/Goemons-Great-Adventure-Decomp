#include "tlb.h"

/* The pick-ups (file 149): the gold coin ("koban"), the dumpling ("dango"), the cat ("neko"),
 * the gold cat ("goldneko"), the "korezo" and the "ooiri". */

/* The game's argument-only debug print: the string stays, no code. */
#define DPRINTF

/* acoll.c's definition is void but returns the box it adds (tlb.h follows the definitions):
 * called through this cast. */
#define acollAtkAddYZ_r ((ACollFrame * (*) (Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)

/* Each pick-up's model is its own file at segment 3 (file sets 0xF koban, 0x10 dango, 0x11 neko,
 * 0x12 goldneko: files 469-472), at the same offsets in each: one address, four assets, a name each. */
extern u8 etskKobanDl[];
extern u8 etskDangoDl[];
extern u8 etskNekoDl[];
extern u8 etskGoldnekoDl[];
extern u8 itemOoiriDl[];
extern u8 itemKorezoDl[];

extern u8 itemKobanTex0[];
extern u8 itemKobanTex1[];
extern u8 itemKobanTex2[];
extern u8 itemKobanTex4[];
extern u8 itemKobanTex5[];
extern u8 itemKobanTex6[];
extern u8 itemDangoTex0[];
extern u8 itemDangoTex1[];
extern u8 itemDangoTex2[];
extern u8 itemDangoTex4[];
extern u8 itemDangoTex5[];
extern u8 itemDangoTex6[];
extern u8 itemNekoTex0[];
extern u8 itemNekoTex1[];
extern u8 itemNekoTex2[];
extern u8 itemNekoTex4[];
extern u8 itemNekoTex5[];
extern u8 itemNekoTex6[];
extern u8 itemGoldnekoTex0[];
extern u8 itemGoldnekoTex1[];
extern u8 itemGoldnekoTex2[];
extern u8 itemGoldnekoTex4[];
extern u8 itemGoldnekoTex5[];
extern u8 itemGoldnekoTex6[];
AnimFrame itemKobanAnim[] = {
    { (u32) itemKobanTex0, 2 },
    { (u32) itemKobanTex1, 2 },
    { (u32) itemKobanTex2, 2 },
    { 0x03000700, 2 },
    { (u32) itemKobanTex4, 2 },
    { (u32) itemKobanTex5, 2 },
    { (u32) itemKobanTex6, 2 },
    { 0x03000F00, 2 },
    { 0x00000000, 0 },
};
AnimFrame itemDangoAnim[] = {
    { (u32) itemDangoTex0, 2 },
    { (u32) itemDangoTex1, 2 },
    { (u32) itemDangoTex2, 2 },
    { 0x03000700, 2 },
    { (u32) itemDangoTex4, 2 },
    { (u32) itemDangoTex5, 2 },
    { (u32) itemDangoTex6, 2 },
    { 0x03000F00, 2 },
    { 0x00000000, 0 },
};
AnimFrame itemNekoAnim[] = {
    { (u32) itemNekoTex0, 2 },
    { (u32) itemNekoTex1, 2 },
    { (u32) itemNekoTex2, 2 },
    { 0x03000700, 2 },
    { (u32) itemNekoTex4, 2 },
    { (u32) itemNekoTex5, 2 },
    { (u32) itemNekoTex6, 2 },
    { 0x03000F00, 2 },
    { 0x00000000, 0 },
};
AnimFrame itemGoldnekoAnim[] = {
    { (u32) itemGoldnekoTex0, 2 },
    { (u32) itemGoldnekoTex1, 2 },
    { (u32) itemGoldnekoTex2, 2 },
    { 0x03000700, 2 },
    { (u32) itemGoldnekoTex4, 2 },
    { (u32) itemGoldnekoTex5, 2 },
    { (u32) itemGoldnekoTex6, 2 },
    { 0x03000F00, 2 },
    { 0x00000000, 0 },
};
f32 itemKobanThrowVel[][3] = {
    { 0.0f, 4.5f, 0.0f },
    { 0.0f, 0.0f, 0.0f },
    { 0.45f, 5.5f, 0.0f },
    { 0.2f, 6.0f, 0.0f },
    { -0.45f, 5.5f, 0.0f },
    { -0.2f, 6.0f, 0.0f },
    { -0.5f, 5.5f, 0.0f },
    { 0.0f, 6.0f, 0.0f },
    { 0.5f, 5.5f, 0.0f },
    { -0.4f, 5.0f, 0.0f },
    { 0.4f, 5.0f, 0.0f },
    { -0.5f, 4.0f, 0.0f },
    { -0.3f, 5.0f, 0.0f },
    { -0.1f, 6.0f, 0.0f },
    { 0.1f, 6.0f, 0.0f },
    { 0.3f, 5.0f, 0.0f },
    { 0.5f, 4.0f, 0.0f },
    { -0.7f, 4.5f, 0.0f },
    { -0.5f, 5.0f, 0.0f },
    { -0.3f, 5.5f, 0.0f },
    { -0.15f, 6.0f, 0.0f },
    { 0.15f, 6.0f, 0.0f },
    { 0.3f, 5.5f, 0.0f },
    { 0.5f, 5.0f, 0.0f },
    { 0.7f, 4.5f, 0.0f },
};

/* A coin's (dumpling's, cat's) work, at its task + 0x20: an enemy's (EneObj) up to 0x4C. */
typedef struct ItemObj {
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
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ MPolFrame *shadow;
    /* 0x40 */ u32 halves[2];
    /* 0x48 */ u32 behave;
    /* 0x4C */ Anim anim;
    /* 0x5C */ u8 timer; /* steps until it goes (it blinks below 70) */
    /* 0x5D */ u8 way; /* its placement's kind: how it is thrown out */
    /* 0x5E */ u8 shine; /* the glint's strength */
    /* 0x5F */ u8 glint; /* 0 none, 1 brightening, 2 fading */
} ItemObj;

/* A korezo's work. */
typedef struct KorezoObj {
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
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ MPolFrame *shadow;
    /* 0x40 */ u32 halves[2];
    /* 0x48 */ u32 behave;
    /* 0x4C */ u8 timer;
    /* 0x4D */ u8 way; /* 0: the player comes back facing 0, else facing pi */
} KorezoObj;

/* An ooiri's work. */
typedef struct OoiriObj {
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
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ MPolFrame *shadow;
    /* 0x40 */ u32 halves[2];
    /* 0x48 */ u32 behave;
    /* 0x4C */ s32 flag; /* its placement's: the switch it sets when taken (swWorkSet) */
    /* 0x50 */ u8 unused50;
    /* 0x51 */ u8 alpha;
    /* 0x52 */ s8 alphaStep;
} OoiriObj;

#define ITEM(obj) ((ItemObj *) FX(obj))
#define KOREZO(obj) ((KorezoObj *) FX(obj))
#define OOIRI(obj) ((OoiriObj *) FX(obj))

s32 Etsk_koban_main(Obj *o, s16 msg, void *arg) {
    ItemObj *e = ITEM(o);

    switch (msg) {
        case 2:
            o->name = "koban";
            e->way = eneSetup(o, arg);
            e->timer = 250;
            if ((e->model = enePolModelMake(o, (Gfx *) etskKobanDl, NULL, 0)) == NULL) {
                return -1;
            }
            floadGetSetRam((void **) e->model->segs, 0xF);
            e->model->flags |= 7;
            e->model->scale[2] = 0.17f;
            e->model->scale[0] = e->model->scale[1] = e->model->scale[2];
            fadeAnimStart(&e->anim, itemKobanAnim, 0, &e->model->segs[3], floadGetSetFile(0xF, 0));
            if ((e->shadow = shadowCreate()) == NULL) {
                return -1;
            }
            if ((e->atk = acollAtkAddYZ_r(ene_atk_root_frame, (Frame *) e->model, 0x23, 0x14, 0, 0)) == NULL) {
                return -1;
            }
            e->atk->kind = 0x6B;
            e->behave = 0x20802000;
            e->glint = 0;
            e->shine = 0;
            e->vel[0] = itemKobanThrowVel[e->way][0];
            e->vel[1] = itemKobanThrowVel[e->way][1];
            e->vel[2] = itemKobanThrowVel[e->way][2];
            if (e->way >= 2) {
                e->behave |= 0x100;
            }
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            fadeAnimStep(&e->anim);
            if (e->way != 1) {
                if (--e->timer == 0) {
                    e->behave |= 0x40000;
                }
                if (e->hit & 1) {
                    e->vel[1] *= -0.8;
                    if (((e->vel[1] > 0.0f) ? e->vel[1] : -e->vel[1]) < 0.25) {
                        e->vel[0] = 0.0f;
                    }
                }
            }
            if (e->glint == 1) {
                if (e->shine + 0x40 < 0xFF) {
                    e->shine += 0x40;
                } else {
                    e->shine = 0xFF;
                    e->glint = 2;
                }
            } else if (e->glint == 2) {
                if (e->shine - 0x40 > 0) {
                    e->shine -= 0x40;
                } else {
                    e->shine = 0;
                    e->glint = 0;
                }
            } else if (e->glint == 0) {
                if (e->anim.index == 7 && fadeRand(100) < 20) {
                    e->glint = 1;
                }
            }
            if (e->timer < 70) {
                if (gGameFrameCount & 1) {
                    e->model->dl2 = NULL;
                } else {
                    e->model->dl2 = (Gfx *) etskKobanDl;
                }
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_koban_main(): 小判消滅\n");
                return -1;
            }
            if (e->hit & 8) {
                PRINTF("Etsk_koban_main(): 小判ゲット\n");
                return -1;
            }
            shadowPut(e->shadow, e->model->pos[0], e->model->pos[1], e->model->pos[2], 0.6f, NULL);
            e->model->pos[1] += 0.5;
        case 3:
            e->model->dl = eneDlEnvColorLit(o, e->shine, 0xFF, 0xFF, 0xC0, 0);
            break;
        case 1:
            if (e->model != NULL) {
                frmDestroy((Frame *) e->model);
            }
            if (e->shadow != NULL) {
                frmDestroy((Frame *) e->shadow);
            }
            if (e->atk != NULL) {
                frmDestroy((Frame *) e->atk);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

s32 Etsk_dango_main(Obj *o, s16 msg, void *arg) {
    ItemObj *e = ITEM(o);

    switch (msg) {
        case 2:
            o->name = "dango";
            e->way = eneSetup(o, arg);
            e->timer = 250;
            if ((e->model = enePolModelMake(o, (Gfx *) etskDangoDl, NULL, 0)) == NULL) {
                return -1;
            }
            floadGetSetRam((void **) e->model->segs, 0x10);
            e->model->flags |= 7;
            e->model->scale[2] = 0.2f;
            e->model->scale[0] = e->model->scale[1] = e->model->scale[2];
            fadeAnimStart(&e->anim, itemDangoAnim, 0, &e->model->segs[3], floadGetSetFile(0x10, 0));
            if ((e->shadow = shadowCreate()) == NULL) {
                return -1;
            }
            if ((e->atk = acollAtkAddYZ_r(ene_atk_root_frame, (Frame *) e->model, 0x23, 0x14, 0, 0)) == NULL) {
                return -1;
            }
            e->atk->kind = 0x6C;
            e->behave = 0x20802000;
            e->glint = 0;
            e->shine = 0;
            if (e->way == 0) {
                e->vel[1] = 4.5f;
            }
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            fadeAnimStep(&e->anim);
            if (e->way == 0) {
                if (--e->timer == 0) {
                    e->behave |= 0x40000;
                }
                if (e->hit & 1) {
                    e->vel[1] *= -0.5;
                }
            }
            if (e->timer < 70) {
                if (gGameFrameCount & 1) {
                    e->model->dl2 = NULL;
                } else {
                    e->model->dl2 = (Gfx *) etskDangoDl;
                }
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_dango_main: 消滅\n");
                return -1;
            }
            if (e->hit & 8) {
                PRINTF("Etsk_dango_main: ゲット\n");
                return -1;
            }
            shadowPut(e->shadow, e->model->pos[0], e->model->pos[1], e->model->pos[2], 0.6f, NULL);
            e->model->pos[1] += 0.5;
        case 3:
            e->model->dl = eneDlEnvColorLit(o, e->shine, 0xFF, 0xFF, 0xC0, 0);
            break;
        case 1:
            if (e->model != NULL) {
                frmDestroy((Frame *) e->model);
            }
            if (e->shadow != NULL) {
                frmDestroy((Frame *) e->shadow);
            }
            if (e->atk != NULL) {
                frmDestroy((Frame *) e->atk);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

s32 Etsk_neko_main(Obj *o, s16 msg, void *arg) {
    ItemObj *e = ITEM(o);

    switch (msg) {
        case 2:
            o->name = "neko";
            e->way = eneSetup(o, arg);
            e->timer = 250;
            if ((e->model = enePolModelMake(o, (Gfx *) etskNekoDl, NULL, 0)) == NULL) {
                return -1;
            }
            floadGetSetRam((void **) e->model->segs, 0x11);
            e->model->flags |= 7;
            e->model->scale[2] = 0.18f;
            e->model->scale[0] = e->model->scale[1] = e->model->scale[2];
            fadeAnimStart(&e->anim, itemNekoAnim, 0, &e->model->segs[3], floadGetSetFile(0x11, 0));
            if ((e->shadow = shadowCreate()) == NULL) {
                return -1;
            }
            if ((e->atk = acollAtkAddYZ_r(ene_atk_root_frame, (Frame *) e->model, 0x23, 0x14, 0, 0)) == NULL) {
                return -1;
            }
            e->atk->kind = 0x6D;
            e->behave = 0x20000900;
            e->glint = 0;
            e->shine = 0;
            if (e->way == 0) {
                e->vel[1] = 3.0f;
                if (eneNearestPlWay(o) == 1) {
                    e->vel[0] = -0.8f;
                } else if (eneNearestPlWay(o) == 4) {
                    e->vel[0] = 0.8f;
                }
            }
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            fadeAnimStep(&e->anim);
            if (e->way == 0) {
                if (--e->timer == 0) {
                    e->behave |= 0x40000;
                }
                if (e->hit & 1) {
                    e->vel[1] = 3.0f;
                }
            }
            if (e->timer < 70) {
                if (gGameFrameCount & 1) {
                    e->model->dl2 = NULL;
                } else {
                    e->model->dl2 = (Gfx *) etskNekoDl;
                }
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_neko_main: 消滅\n");
                return -1;
            }
            if (e->hit & 8) {
                PRINTF("Etsk_neko_main: ゲット\n");
                return -1;
            }
            shadowPut(e->shadow, e->model->pos[0], e->model->pos[1], e->model->pos[2], 0.6f, NULL);
            e->model->pos[1] += 0.5;
        case 3:
            e->model->dl = eneDlEnvColorLit(o, e->shine, 0xFF, 0xFF, 0xC0, 0);
            break;
        case 1:
            if (e->model != NULL) {
                frmDestroy((Frame *) e->model);
            }
            if (e->shadow != NULL) {
                frmDestroy((Frame *) e->shadow);
            }
            if (e->atk != NULL) {
                frmDestroy((Frame *) e->atk);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

s32 Etsk_goldneko_main(Obj *o, s16 msg, void *arg) {
    ItemObj *e = ITEM(o);

    switch (msg) {
        case 2:
            o->name = "goldneko";
            e->way = eneSetup(o, arg);
            if ((e->model = eneNewPol(o, (Gfx *) etskGoldnekoDl)) == NULL) {
                return -1;
            }
            floadGetSetRam((void **) e->model->segs, 0x12);
            e->model->flags |= 7;
            e->model->scale[2] = 0.18f;
            e->model->scale[0] = e->model->scale[1] = e->model->scale[2];
            fadeAnimStart(&e->anim, itemGoldnekoAnim, 0, &e->model->segs[3], floadGetSetFile(0x12, 0));
            if ((e->shadow = shadowCreate()) == NULL) {
                return -1;
            }
            if ((e->atk = acollAtkAddYZ_r(ene_atk_root_frame, (Frame *) e->model, 0x23, 0x14, 0, 0)) == NULL) {
                return -1;
            }
            e->atk->kind = 0x73;
            e->behave = 0x900;
            e->shine = 0;
            break;
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            fadeAnimStep(&e->anim);
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_goldneko_main: 消滅\n");
                return -1;
            }
            if (e->hit & 8) {
                PRINTF("Etsk_goldneko_main: ゲット\n");
                return -1;
            }
            shadowPut(e->shadow, e->model->pos[0], e->model->pos[1], e->model->pos[2], 0.6f, NULL);
            break;
        case 3:
            e->model->dl = eneDlEnvColorLit(o, e->shine, 0xFF, 0xFF, 0xC0, 0);
            break;
        case 1:
            if (e->model != NULL) frmDestroy((Frame *) e->model);
            if (e->shadow != NULL) frmDestroy((Frame *) e->shadow);
            if (e->atk != NULL) frmDestroy((Frame *) e->atk);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

s32 itemKorezoMain(Obj *o, s16 msg, void *arg) {
    KorezoObj *e = KOREZO(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "korezo";
            e->way = eneSetup(o, arg);
            e->timer = 250;
            if ((e->model = eneNewPol(o, (Gfx *) itemKorezoDl)) == NULL) {
                return -1;
            }
            floadGetSetRam((void **) e->model->segs, 0xBA);
            e->model->flags |= 7;
            e->model->scale[2] = 0.1f;
            e->model->scale[0] = e->model->scale[1] = e->model->scale[2];
            if ((e->shadow = shadowCreate()) == NULL) {
                return -1;
            }
            if ((e->atk = acollAtkAddYZ_r(ene_atk_root_frame, (Frame *) e->model, 0x23, 0x14, 0, 0)) == NULL) {
                return -1;
            }
            e->atk->kind = 0x72;
            e->behave = 0x900;
            break;
        case 1:
            if (e->model != NULL) {
                frmDestroy((Frame *) e->model);
            }
            if (e->shadow != NULL) {
                frmDestroy((Frame *) e->shadow);
            }
            if (e->atk != NULL) {
                frmDestroy((Frame *) e->atk);
            }
            break;
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            if (eneAfterProc(o) == -1) {
                return -1;
            }
            if (e->hit & 8) {
                gGameWork.savedSpawn.on = 1;
                gGameWork.savedSpawn.angle = e->zone;
                gGameWork.savedSpawn.x = e->x;
                gGameWork.savedSpawn.y = e->y;
                gGameWork.savedSpawn.z = e->z;
                if (e->way == 0) {
                    gGameWork.savedSpawn.facing = 0.0f;
                } else {
                    gGameWork.savedSpawn.facing = 3.1415927f;
                }
                return -1;
            }
            shadowPut(e->shadow, e->model->pos[0], e->model->pos[1], e->model->pos[2], 0.6f, NULL);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The print is the argument-only DPRINTF: any PRINTF ("if (0) osSyncPrintf") anywhere in this
 * function costs the -1 of the two "== -1" tests its saved register (s0). */
s32 itemOoiriMain(Obj *o, s16 msg, void *arg) {
    OoiriObj *e = OOIRI(o);

    switch (msg) {
        case 2:
            o->name = "ooiri";
            e->flag = eneSetup(o, arg);
            if (swWorkTest(e->flag) || e->flag == 0) {
                DPRINTF("!!!!!!!!! ooiri clear !!!!!!!!!!\n");
                return -1;
            }
            if ((e->model = eneNewPol(o, (Gfx *) itemOoiriDl)) == NULL) {
                return -1;
            }
            floadGetSetRam((void **) e->model->segs, 0x3F);
            frmMove((Frame *) e->model, gFrmLayer2);
            e->model->scale[2] = 0.1f;
            e->model->scale[0] = e->model->scale[1] = e->model->scale[2];
            if ((e->shadow = shadowCreate()) == NULL) {
                return -1;
            }
            if ((e->atk = acollAtkAddYZ_r(ene_atk_root_frame, (Frame *) e->model, 0x23, 0x14, 0, 0)) == NULL) {
                return -1;
            }
            e->atk->kind = 0x74;
            e->alpha = 0;
            e->alphaStep = 8;
            break;
        case 1:
            if (e->model != NULL) frmDestroy((Frame *) e->model);
            if (e->shadow != NULL) frmDestroy((Frame *) e->shadow);
            if (e->atk != NULL) frmDestroy((Frame *) e->atk);
            break;
        case 3:
            e->model->dl = eneDlEnvColorLit(o, e->alpha, 0xFF, 0xFF, 0xFF, 1);
            break;
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            e->alpha += e->alphaStep;
            if (e->alpha == 0x40) {
                e->alphaStep = -8;
            }
            if (e->alpha == 0) {
                e->alphaStep = 8;
            }
            e->rot[1] += 0.06283185631f;
            if (e->rot[1] >= 6.283185482f) {
                e->rot[1] -= 6.283185482f;
            }
            if (eneAfterProc(o) == -1) {
                return -1;
            }
            if (e->hit & 8) {
                swWorkSet(e->flag);
                eneMakeEx(NULL, ENE_DEMO_KIRAKIRA, 0, e->model->pos[0], e->model->pos[1], e->model->pos[2], 0, NULL);
                return -1;
            }
            shadowPut(e->shadow, e->model->pos[0], e->model->pos[1], e->model->pos[2], 0.6f, NULL);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
