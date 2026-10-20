#include "tlb.h"

/* The haniwa enemies (file 81): the haniwa ("hani"), its small kind ("kohani"), the rolling
 * haniwa ("Rhani"), and the fire it leaves ("fire"). */

#define sinf (*sinf)
#define cosf (*cosf)

/* acoll.c's definitions are void but return the box they add (tlb.h follows the definitions):
 * called through these casts. */
#define acollAtkAddYZ_r ((ACollFrame * (*) (Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define acollDmgAddYZ_r ((ACollFrame * (*) (Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 haniFireDl[];
extern u8 etskHaniAnim[];
extern u8 etskHaniHalf0Dl[];
extern u8 etskHaniHalf1Dl[];

extern u8 haniFireTex0[];
extern u8 haniFireTex2[];
extern u8 haniFireTex5[];
AnimFrame haniFireAnim[] = {
    { (u32) haniFireTex0, 2 },
    { 0x03000D80, 2 },
    { (u32) haniFireTex2, 2 },
    { 0x03001D80, 2 },
    { 0x03002580, 2 },
    { (u32) haniFireTex5, 2 },
    { 0x03003580, 2 },
    { 0x03003D80, 2 },
    { 0x03004580, 2 },
    { 0x03004D80, 2 },
    { 0x03005580, 2 },
    { 0x03005D80, 2 },
    { 0x03006580, 2 },
    { 0x03006D80, 2 },
    { 0x03007580, 2 },
    { 0x03007D80, 2 },
    { 0, 0 },
    { 0, 0 },
};

/* A haniwa's work, at its task + 0x20: an enemy's (EneObj) up to 0x4C. */
typedef struct HaniObj {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ AnimeFrame *model;
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
    /* 0x4C */ s16 count4C; /* kohani's step count */
    /* 0x4E */ u8 pad4E[2];
    /* 0x50 */ s16 count; /* hani's step count; Rhani: its way (u8) */
    /* 0x52 */ s16 homeX; /* hani: where it bobs about */
} HaniObj;

/* A rolling haniwa's work. */
typedef struct RhaniObj {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ AnimeFrame *model;
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
    /* 0x4C */ u8 pad4C[4];
    /* 0x50 */ u8 way; /* the way it rolls: 1 or 4 (eneNearestPlWay) */
    /* 0x51 */ u8 pad51[2];
    /* 0x53 */ u8 timer; /* steps it cannot be hit after a hit */
} RhaniObj;

/* A fire's work. */
typedef struct FireObj {
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
    /* 0x5C */ u8 alpha;
} FireObj;

#define HANI(obj) ((HaniObj *) FX(obj))
#define RHANI(obj) ((RhaniObj *) FX(obj))
#define FIRE(obj) ((FireObj *) FX(obj))

void haniWaitCam(Obj *o);
void haniToCourse(Obj *o);
void haniWalk(Obj *o);
void haniStand(Obj *o);
void haniWaitNear(Obj *o);
void haniShrink(Obj *o);
void haniGrow(Obj *o);
void haniGrowWalk(Obj *o);
void haniAppear(Obj *o);
void haniAppearWait(Obj *o);
void haniAppearWalk(Obj *o);
void haniFireWalk(Obj *o);
void haniSway(Obj *o);
void haniKoStand(Obj *o);
void haniKoHop(Obj *o);
void haniRWait(Obj *o);
void haniRRoll(Obj *o);
void haniRNone(Obj *o);
void haniRKnockback(Obj *o);

s32 Etsk_hani_main(Obj *o, s16 msg, void *arg) {
    HaniObj *e = HANI(o);
    s32 pad;
    u32 way;

    switch (msg) {
        case 2:
            o->name = "hani";
            way = eneSetup(o, arg);
            if ((e->model = eneNewAnime(o, (u32) etskHaniAnim)) == NULL) {
                return -1;
            }
            if ((e->atk = acollAtkAddYZ_r(ene_atk_root_frame, (Frame *) e->model, 0x2A, 0x50, 0, 0)) == NULL) {
                return -1;
            }
            e->atk->kind = 0x78;
            if ((e->dmg = acollDmgAddYZ_r(ene_dmg_root_frame, (Frame *) e->model, 0x46, 0xA0, 0, 0, 0)) == NULL) {
                return -1;
            }
            if ((e->shadow = shadowCreate()) == NULL) {
                return -1;
            }
            if (way != 6 && eneSetOnEarth(o) == -1) {
                return -1;
            }
            e->halves[0] = (u32) etskHaniHalf0Dl;
            e->halves[1] = (u32) etskHaniHalf1Dl;
            if (way == 1) {
                e->behave = 0xC04500;
                if (e->z < 0.0f) {
                    e->rot[1] = eneCourseAngle(o, 8);
                    e->state = 0;
                } else if (0.0f < e->z) {
                    e->rot[1] = eneCourseAngle(o, 2);
                    e->state = 0;
                } else {
                    e->rot[1] = eneCourseAngle(o, eneNearestPlWay(o));
                    e->state = 1;
                }
            } else if (way == 2) {
                e->behave = 0xC00000;
                e->rot[1] = eneCourseAngle(o, eneNearestPlWay(o));
                e->state = 3;
            } else if (way == 3) {
                e->behave = 0xC04500;
                e->rot[1] = eneCourseAngle(o, 8);
                e->state = 4;
            } else if (way == 4) {
                e->model->anim = 0;
                e->behave = 0xC04500;
                e->rot[1] = eneCourseAngle(o, 8);
                e->state = 8;
            } else if (way == 5) {
                e->behave = 0xC04500;
                e->rot[1] = eneCourseAngle(o, 8);
                if (eneNearestPlWay(o) == 1) {
                    e->vel[0] = 1.8f;
                } else if (eneNearestPlWay(o) == 4) {
                    e->vel[0] = -1.8f;
                }
                e->count = 15;
                e->state = 11;
            } else if (way == 6) {
                e->behave = 0x420000;
                e->homeX = e->x;
                e->rot[1] = eneCourseAngle(o, 1);
                e->state = 12;
            }
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (e->state) {
                case 0:
                    (*haniWaitCam)(o);
                    break;
                case 1:
                    (*haniToCourse)(o);
                    break;
                case 2:
                    (*haniWalk)(o);
                    break;
                case 3:
                    (*haniStand)(o);
                    break;
                case 4:
                    (*haniWaitNear)(o);
                    break;
                case 5:
                    (*haniShrink)(o);
                    break;
                case 6:
                    (*haniGrow)(o);
                    break;
                case 7:
                    (*haniGrowWalk)(o);
                    break;
                case 8:
                    (*haniAppear)(o);
                    break;
                case 9:
                    (*haniAppearWait)(o);
                    break;
                case 10:
                    (*haniAppearWalk)(o);
                    break;
                case 11:
                    (*haniFireWalk)(o);
                    break;
                case 12:
                    (*haniSway)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_hani_main: 死亡\n");
                return -1;
            }
            break;
        case 1:
            if (e->model != NULL) {
                frmDestroy((Frame *) e->model);
            }
            if (e->atk != NULL) {
                frmDestroy((Frame *) e->atk);
            }
            if (e->dmg != NULL) {
                frmDestroy((Frame *) e->dmg);
            }
            if (e->shadow != NULL) {
                frmDestroy((Frame *) e->shadow);
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void haniWaitCam(Obj *o) {
    eneAnimLoop(o, HANI(o)->model, 1.25f);
    if (eneCamNearCheck(o, 70.0f)) {
        HANI(o)->state = 1;
    }
}

void haniToCourse(Obj *o) {
    HaniObj *e = HANI(o);
    f32 z;
    f32 step;

    eneAnimLoop(o, e->model, 1.25f);
    z = e->z;
    if (z < 0.0f) {
        step = 1.0f;
        e->vel[2] = step;
        if (z + step >= 0.0f) {
            e->vel[2] = 0.0f;
            e->z = 0.0f;
        }
    } else if (z > 0.0f) {
        step = -1.0f;
        e->vel[2] = step;
        if (z + step <= 0.0f) {
            e->vel[2] = 0.0f;
            e->z = 0.0f;
        }
    } else {
        if (eneNearestPlWay(o) == 1) {
            e->vel[0] = 1.0f;
        } else if (eneNearestPlWay(o) == 4) {
            e->vel[0] = -1.0f;
        }
        e->state = 2;
    }
}

void haniWalk(Obj *o) {
    eneAnimLoop(o, HANI(o)->model, 1.25f);
}

void haniStand(Obj *o) {
    eneAnimLoop(o, HANI(o)->model, 1.25f);
}

void haniWaitNear(Obj *o) {
    eneAnimLoop(o, HANI(o)->model, 1.25f);
    if (eneNearestPlInX(o, 65.0f)) {
        HANI(o)->state = 5;
    }
}

void haniShrink(Obj *o) {
    HaniObj *e = HANI(o);

    if (0.01 < e->model->scale[0]) {
        e->model->scale[0] *= 0.6;
    } else {
        e->model->anim = 0;
        e->count = 0;
        e->state = 6;
        e->z = 0.0f;
    }
    e->model->scale[2] = e->model->scale[0];
}

void haniGrow(Obj *o) {
    HaniObj *e = HANI(o);
    s32 pad;

    e->count++;
    if (e->count >= 7) {
        e->model->anim = etskHaniAnim;
        if (e->model->scale[0] < 0.1) {
            e->model->scale[0] = e->model->scale[0] / 0.6;
        } else {
            e->model->scale[0] = 0.1f;
            if (eneNearestPlWay(o) == 1) {
                e->vel[0] = 1.0f;
            } else if (eneNearestPlWay(o) == 4) {
                e->vel[0] = -1.0f;
            }
            e->state = 7;
        }
        e->model->scale[2] = e->model->scale[0];
    }
}

void haniGrowWalk(Obj *o) {
    eneAnimLoop(o, HANI(o)->model, 1.25f);
}

void haniAppear(Obj *o) {
    HaniObj *e = HANI(o);
    f32 z;
    f32 x;

    if (eneNearestPlInX(o, 80.0f)) {
        x = e->model->pos[0] + sinf(e->rot[1]) * 10.0f;
        z = e->model->pos[2] + cosf(e->rot[1]) * 10.0f;
        itexPuffWhite(x, e->model->pos[1] + 5.0f, z, 0.1f);
        e->model->anim = etskHaniAnim;
        e->state = 9;
    }
}

void haniAppearWait(Obj *o) {
    HaniObj *e = HANI(o);
    s32 pad[2];

    eneAnimLoop(o, e->model, 1.25f);
    e->count++;
    if (e->count >= 11) {
        if (eneNearestPlWay(o) == 1) {
            e->vel[0] = 1.0f;
        } else if (eneNearestPlWay(o) == 4) {
            e->vel[0] = -1.0f;
        }
        e->state = 10;
    }
}

void haniAppearWalk(Obj *o) {
    eneAnimLoop(o, HANI(o)->model, 1.25f);
}

void haniFireWalk(Obj *o) {
    HaniObj *e = HANI(o);
    f32 c;
    f32 s;
    s32 pad[3];

    eneAnimLoop(o, e->model, ((e->vel[0] > 0.0f) ? e->vel[0] : -e->vel[0]) * 1.1);
    e->count++;
    if (e->count >= 31) {
        e->count = 0;
        e->vel[0] *= -1.0f;
    }
    s = sinf(e->rot[1] - eneCourseAngle(o, 8) + 3.1415927f) * 4.0f;
    c = cosf(e->rot[1] - eneCourseAngle(o, 8) + 3.1415927f) * 4.0f;
    eneMakeEx(o, ENE_HANI_FIRE_HOURS_ANY, e->zone, e->x + s, e->y + 5.0f, e->z + c, 0, NULL);
    if (!(gGameFrameCount & 7)) {
        eneSetSd(o, 0x459);
    }
}

void haniSway(Obj *o) {
    HaniObj *e = HANI(o);

    eneAnimLoop(o, e->model, 1.25f);
    e->x = sinf(gGameFrameCount / 20.0f) * 20.0f / 2 + e->homeX;
}

s32 haniFireProc(Obj *o, s16 msg, void *arg) {
    FireObj *e = FIRE(o);

    switch (msg) {
        case 2:
            o->name = "fire";
            eneSetup(o, arg);
            if ((e->model = enePolModelMake(o, (Gfx *) haniFireDl, NULL, 1)) == NULL) {
                return -1;
            }
            floadGetSetRam((void **) e->model->segs, 0x125);
            e->model->flags = 3;
            fadeAnimStart(&e->anim, haniFireAnim, 0, &e->model->segs[3], floadGetSetFile(0x125, 0));
            e->alpha = 0xFF;
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            fadeAnimStep(&e->anim);
            e->model->scale[0] *= 1.2;
            e->model->scale[1] = e->model->scale[2] = e->model->scale[0];
            e->vel[1] = 1.0f;
            e->alpha -= 0x20;
            if (e->alpha < 0x60) {
                e->behave |= 0x40000;
                if (!(gGameFrameCount & 3)) {
                    itexPuffGrey(e->model->pos[0], e->model->pos[1], e->model->pos[2], 0.08f);
                }
            }
            if (eneAfterProc(o) == -1) {
                return -1;
            }
        case 3:
            e->model->dl = eneDlEnvPrimColor(o, 0xFF, 0xFF, 0x7F, 0xFF, 0, 0, e->alpha);
            break;
        case 1:
            if (e->model != NULL) {
                frmDestroy((Frame *) e->model);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Message 1's tests are one line each ("if (x) frmDestroy(x);"): written over three lines,
 * as1 put the work pointer's addiu before frmDestroy's instead of in the beqz's delay slot. */
s32 Etsk_kohani_main(Obj *o, s16 msg, void *arg) {
    HaniObj *e = HANI(o);
    s32 pad;
    s32 way;
    s32 dir;

    switch (msg) {
        case 2:
            o->name = "kohani";
            way = eneSetup(o, arg);
            if ((e->model = eneNewAnime(o, (u32) etskHaniAnim)) == NULL) {
                return -1;
            }
            e->model->scale[2] = 0.06f;
            e->model->scale[0] = e->model->scale[1] = e->model->scale[2];
            if ((e->shadow = shadowCreate()) == NULL) {
                return -1;
            }
            if ((e->atk = acollAtkAddYZ_r(ene_atk_root_frame, (Frame *) e->model, 0x2A, 0x50, 0, 0)) == NULL) {
                return -1;
            }
            e->atk->kind = 0x78;
            if ((e->dmg = acollDmgAddYZ_r(ene_dmg_root_frame, (Frame *) e->model, 0x54, 0xC0, 0, 0, 0)) == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                PRINTF("Etsk_kohani_j: 地面にセット失敗\n");
                return -1;
            }
            e->halves[0] = (u32) etskHaniHalf0Dl;
            e->halves[1] = (u32) etskHaniHalf1Dl;
            if (way == 1) {
                e->behave = 0xC04500;
                dir = eneNearestPlWay(o);
                if (dir == 4) {
                    e->vel[0] = -1.0f;
                } else if (dir == 1) {
                    e->vel[0] = 1.0f;
                }
                e->state = 0;
            } else if (way == 2) {
                e->behave = 0xC04100;
                e->state = 1;
                e->vel[0] = -1.0f;
            }
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (e->state) {
                case 0:
                    (*haniKoStand)(o);
                    break;
                case 1:
                    (*haniKoHop)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_kohani_main: 死亡\n");
                return -1;
            }
            break;
        case 1:
            if (e->model != NULL) frmDestroy((Frame *) e->model);
            if (e->shadow != NULL) frmDestroy((Frame *) e->shadow);
            if (e->atk != NULL) frmDestroy((Frame *) e->atk);
            if (e->dmg != NULL) frmDestroy((Frame *) e->dmg);
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void haniKoStand(Obj *o) {
    eneAnimLoop(o, HANI(o)->model, 2.5f);
}

void haniKoHop(Obj *o) {
    HaniObj *e = HANI(o);
    s32 pad[2];

    eneAnimLoop(o, e->model, 2.5f);
    if (fadeRand(100) < 2 && (e->hit & 1)) {
        eneSetSd(o, 0x23C);
        e->vel[1] = 4.5f;
    }
    e->count4C++;
    if (e->count4C % 100 == 0) {
        e->vel[0] *= -1.0f;
    }
}

s32 Etsk_Rhani_main(Obj *o, s16 msg, void *arg) {
    RhaniObj *e = RHANI(o);

    switch (msg) {
        case 2:
            o->name = "Rhani";
            eneSetup(o, arg);
            if ((e->model = eneNewAnime(o, (u32) etskHaniAnim)) == NULL) {
                return -1;
            }
            if ((e->shadow = shadowCreate()) == NULL) {
                return -1;
            }
            if ((e->atk = acollAtkAddYZ_r(ene_atk_root_frame, (Frame *) e->model, 0x2A, 0x50, 0, 0)) == NULL) {
                return -1;
            }
            e->atk->kind = 0x78;
            if ((e->dmg = acollDmgAddYZ_r(ene_dmg_root_frame, (Frame *) e->model, 0x46, 0xA0, 0, 0, 0)) == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                PRINTF("Etsk_Rhani_j: 地面にセット失敗\n");
                return -1;
            }
            e->halves[0] = (u32) etskHaniHalf0Dl;
            e->halves[1] = (u32) etskHaniHalf1Dl;
            e->timer = 0;
            e->behave = 0xC04302;
            e->model->frame = 14.0f;
            e->way = eneNearestPlWay(o);
            e->rot[1] = eneCourseAngle(o, 8);
            e->hp = 1;
            e->state = 0;
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (e->state) {
                case 0:
                    (*haniRWait)(o);
                    break;
                case 1:
                    (*haniRRoll)(o);
                    break;
                case 2:
                    (*haniRNone)(o);
                    break;
                case 3:
                    (*haniRKnockback)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_Rhani_main: 死亡\n");
                return -1;
            }
            if (e->hit & 0x10) {
                e->timer = 12;
                e->vel[0] = eneKnockbackSpeed(o, e->dmg, 4.0f);
                e->state = 3;
            }
            if (e->timer - 1 >= 0) {
                e->dmg->flags &= ~2;
                e->behave &= ~0x4500;
                e->behave |= 0x280;
                e->timer--;
            } else {
                e->dmg->flags |= 2;
                e->timer = 0;
                e->behave &= ~0x280;
                e->behave |= 0x4500;
            }
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
            if (e->dmg != NULL) {
                frmDestroy((Frame *) e->dmg);
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void haniRWait(Obj *o) {
    RhaniObj *e = RHANI(o);

    if (eneCamNearCheck(o, 60.0f)) {
        if (e->way == 4) {
            e->vel[0] = -1.0f;
        } else if (e->way == 1) {
            e->vel[0] = 1.0f;
        }
        e->state = 1;
    }
}

/* Matched 2026-10-04; it was parked for registers: the work pointer and the direction took v0/v1
 * swapped from the game's, and the final multiply's operands were (speed, dir) for the game's
 * (dir, speed). No dir local: the sign is an inline ternary in the multiply itself,
 * "scale * 6.0 * ((vel < 0) ? -1 : 1)", which gives the game's operand order; pad[2] below it
 * fills the game's frame (0x30). */
void haniRRoll(Obj *o) {
    RhaniObj *e = RHANI(o);
    s32 pad[2]; /* FAKE: stands for 8 bytes of the game's frame no local of this C fills */

    if (e->model->scale[0] + 0.0075 < 0.15) {
        e->model->scale[0] += 0.0075;
    } else {
        e->model->scale[0] = 0.15f;
    }
    e->model->scale[1] = e->model->scale[2] = e->model->scale[0];
    e->vel[0] = e->model->scale[0] * 6.0 * ((e->vel[0] < 0.0f) ? -1 : 1);
    if (eneAnimLoop(o, e->model, 1.0f)) {
    }
    e->way = (e->vel[0] < 0.0f) ? 4 : 1;
}

void haniRNone(Obj *o) {
}

void haniRKnockback(Obj *o) {
    RhaniObj *e = RHANI(o);

    if (!(gGameFrameCount & 3)) {
        itexPuffWhite(e->model->pos[0], e->model->pos[1], e->model->pos[2], 0.05f);
    }
    if (e->vel[0] < 0.0f) {
        e->vel[0] += 0.36363636363636365;
    } else if (e->vel[0] > 0.0f) {
        e->vel[0] -= 0.36363636363636365;
    }
    if (e->timer == 0) {
        e->vel[0] = (e->way == 4) ? -1.0 : 1.0;
        e->state = 1;
    }
}
