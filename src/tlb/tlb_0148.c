#include "tlb.h"

/* The octopus enemy (file 148): the octopus ("tako"), the ink it spits ("sumi") and the ball it
 * shoots ("tama"). */

#define sinf (*sinf)
#pragma intrinsic (fabs)

/* acoll.c's definitions are void but return the box they add (tlb.h follows the definitions):
 * called through these casts. */
#define acollAtkAddYZ_r ((ACollFrame * (*) (Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define acollDmgAddYZ_r ((ACollFrame * (*) (Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 takoTamaDl[];
extern u8 etskTakoAnim[];
extern u8 etskTakoTurnStartAnim[];
extern u8 takoSumiDl[];
extern u8 etskTakoHalf0Dl[];
extern u8 etskTakoHalf1Dl[];

extern u8 takoSumiTex0[];
extern u8 takoSumiTex1[];
extern u8 takoSumiTex2[];
extern u8 takoSumiTex3[];
extern u8 takoSumiTex4[];
extern u8 takoSumiTex6[];
AnimFrame takoSumiAnim[] = {
    { (u32) takoSumiTex0, 2 },
    { (u32) takoSumiTex1, 2 },
    { (u32) takoSumiTex2, 2 },
    { (u32) takoSumiTex3, 2 },
    { (u32) takoSumiTex4, 2 },
    { 0x03002D30, 2 },
    { (u32) takoSumiTex6, 2 },
    { 0x03003D30, 2 },
    { 0x03004530, 2 },
    { 0x03004D30, 2 },
    { 0x03005530, 2 },
    { 0x03005D30, 2 },
    { 0, 0 },
    { 0, 0 },
};

/* An octopus's work, at its task + 0x20: an enemy's (EneObj) up to 0x4C. */
typedef struct TakoObj {
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
    /* 0x4C */ s16 count; /* steps to wait */
    /* 0x4E */ u8 hops; /* hops made (every fourth one it stops) */
    /* 0x4F */ u8 way; /* its placement's kind: 1 still, 2 hopping, 3 floating */
    /* 0x50 */ f32 turnTo; /* the angle it turns to */
    /* 0x54 */ f32 turnStep;
    /* 0x58 */ u8 pad58[4];
    /* 0x5C */ s16 homeY; /* a floater's: where it bobs about */
    /* 0x5E */ s16 homeX;
} TakoObj;

/* An ink blot's work. */
typedef struct SumiObj {
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
    /* 0x5C */ s32 timer; /* steps it grows */
} SumiObj;

/* A ball's work. */
typedef struct TamaObj {
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
    /* 0x4C */ s16 count; /* steps it has flown */
} TamaObj;

#define TAKO(obj) ((TakoObj *) FX(obj))
#define SUMI(obj) ((SumiObj *) FX(obj))
#define TAMA(obj) ((TamaObj *) FX(obj))

void takoWait(Obj *o);
void takoTurnStart(Obj *o);
void takoSpit(Obj *o);
void takoSpitEnd(Obj *o);
void takoHop(Obj *o);
void takoHopEnd(Obj *o);
void takoShoot(Obj *o);
void takoShootEnd(Obj *o);
void takoFloatWatch(Obj *o);
void takoFloatBrake(Obj *o);
void takoFloatShoot(Obj *o);

s32 Etsk_tako_main(Obj *o, s16 msg, void *arg) {
    TakoObj *e = TAKO(o);
    s32 pad;
    s32 dir;

    switch (msg) {
        case 2:
            o->name = "tako";
            e->way = eneSetup(o, arg);
            if ((e->model = eneNewAnime(o, (u32) etskTakoAnim)) == NULL) {
                return -1;
            }
            if ((e->shadow = shadowCreate()) == NULL) {
                return -1;
            }
            if ((e->atk = acollAtkAddYZ_r(ene_atk_root_frame, (Frame *) e->model, 0x32, 0x50, 0, 0)) == NULL) {
                return -1;
            }
            e->atk->kind = 0x78;
            if ((e->dmg = acollDmgAddYZ_r(ene_dmg_root_frame, (Frame *) e->model, 0x46, 0xA0, 0, 0, 0)) == NULL) {
                return -1;
            }
            e->halves[0] = (u32) etskTakoHalf0Dl;
            e->halves[1] = (u32) etskTakoHalf1Dl;
            if (e->way == 1) {
                e->behave = 0x800;
                if (eneSetOnEarth(o) == -1) {
                    return -1;
                }
                e->rot[1] = eneCourseAngle(o, 8);
                e->state = 0;
            } else if (e->way == 2) {
                e->behave = 0x404900;
                if (eneSetOnEarth(o) == -1) {
                    return -1;
                }
                e->rot[1] = eneCourseAngle(o, eneNearestPlWay(o));
                e->state = 4;
            } else if (e->way == 3) {
                e->behave = 0x400000;
                dir = enePlDirX(o, 0);
                if (dir != 4) {
                    e->behave |= 0x40000;
                }
                e->homeY = e->y;
                e->homeX = e->x;
                e->vel[0] = -2.5f;
                e->rot[1] = eneCourseAngle(o, dir);
                e->state = 8;
            }
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (e->state) {
                case 0:
                    (*takoWait)(o);
                    break;
                case 1:
                    (*takoTurnStart)(o);
                    break;
                case 2:
                    (*takoSpit)(o);
                    break;
                case 3:
                    (*takoSpitEnd)(o);
                    break;
                case 4:
                    (*takoHop)(o);
                    break;
                case 5:
                    (*takoHopEnd)(o);
                    break;
                case 6:
                    (*takoShoot)(o);
                    break;
                case 7:
                    (*takoShootEnd)(o);
                    break;
                case 8:
                    (*takoFloatWatch)(o);
                    break;
                case 9:
                    (*takoFloatBrake)(o);
                    break;
                case 10:
                    (*takoFloatShoot)(o);
                    break;
            }
            if (e->way == 3) {
                e->y = sinf(gGameFrameCount / 5.0f) + (f64) e->homeY;
                e->rot[0] = e->vel[0] / 2.5 * -3.1415927f / 3.0;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_tako_main: 死亡\n");
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

void takoWait(Obj *o) {
    TakoObj *e = TAKO(o);

    if (eneNearestPlInX(o, 70.0f)) {
        e->count = 0;
        e->state = 1;
    }
}

void takoTurnStart(Obj *o) {
    TakoObj *e = TAKO(o);
    s32 pad[2];
    s32 dir;
    s32 face;
    f32 jump;

    e->count--;
    if (e->count < 0 && eneAnimPlayOnce(o, e->model, 0.8f) == 1) {
        e->model->frame = 0.0f;
        e->model->anim = etskTakoTurnStartAnim;
        dir = eneNearestPlWay(o);
        face = eneFaceWay(o);
        if (face != dir) {
            jump = 2.0f;
            e->vel[1] = jump;
            if (face == 8) {
                if (dir == 1) {
                    e->turnStep = 1.5707964f / (e->vel[1] * jump / 0.3);
                } else {
                    e->turnStep = -(1.5707964f / (e->vel[1] * jump / 0.3));
                }
            } else {
                if (dir == 1) {
                    e->turnStep = 3.1415927f / (e->vel[1] * jump / 0.3);
                } else {
                    e->turnStep = -(3.1415927f / (e->vel[1] * jump / 0.3));
                }
            }
            e->turnTo = eneCourseAngle(o, dir);
        } else {
            e->turnTo = e->rot[1];
        }
        e->state = 2;
    }
}

void takoSpit(Obj *o) {
    TakoObj *e = TAKO(o);
    s16 face;
    f32 step;
    s16 shot;

    eneTurnRotYTo(o, e->turnTo, e->turnStep);
    if (e->hit & 1) {
        step = 0.3f;
        if (eneAnimPlayOnce(o, e->model, step) == 1) {
            e->model->anim = etskTakoAnim;
            e->model->frame = polAnimeLastFrame(e->model);
            e->state = 3;
        }
        if (5.0 < e->model->frame && e->model->frame <= 8.0 + step) {
            if (5.0 < e->model->frame && e->model->frame <= 5.0 + step) {
                eneSetSd(o, 0x258);
                shot = 1;
            } else {
                shot = 0;
            }
            face = eneFaceWay(o);
            switch (face) {
                case 1:
                    eneMakeEx(NULL, ENE_TAKO_SUMI_MAIN, e->zone, e->x + 4.0f, e->y + 9.0f, e->z, 2,
                              (u32 (*)(void *, void *, void *)) ((face & 0xFFFF) | ((shot << 16) & 0xFFFF0000)));
                    break;
                case 4:
                    eneMakeEx(NULL, ENE_TAKO_SUMI_MAIN, e->zone, e->x - 4.0f, e->y + 9.0f, e->z, 2,
                              (u32 (*)(void *, void *, void *)) ((face & 0xFFFF) | ((shot << 16) & 0xFFFF0000)));
                    break;
                case 8:
                    eneMakeEx(NULL, ENE_TAKO_SUMI_MAIN, e->zone, e->x, e->y + 9.0f, e->z + 4.0f, 2,
                              (u32 (*)(void *, void *, void *)) ((face & 0xFFFF) | ((shot << 16) & 0xFFFF0000)));
                    break;
                case 2:
                    eneMakeEx(NULL, ENE_TAKO_SUMI_MAIN, e->zone, e->x, e->y + 9.0f, e->z - 4.0f, 2,
                              (u32 (*)(void *, void *, void *)) ((face & 0xFFFF) | ((shot << 16) & 0xFFFF0000)));
                    break;
            }
        }
    }
}

void takoSpitEnd(Obj *o) {
    TakoObj *e = TAKO(o);

    if (eneAnimPlayOnce(o, e->model, -1.0f) == 1) {
        e->count = 45;
        e->state = 1;
    }
}

void takoHop(Obj *o) {
    TakoObj *e = TAKO(o);
    s32 pad;

    if (e->hit & 1) {
        if (++e->hops % 4) {
            eneSetSd(o, 0x239);
            e->vel[1] = 3.0f;
            if (eneNearestPlWay(o) == 1) {
                e->vel[0] = 0.3f;
            } else {
                e->vel[0] = -0.3f;
            }
        } else {
            e->state = 5;
            e->vel[0] = 0.0f;
        }
    }
}

void takoHopEnd(Obj *o) {
    TakoObj *e = TAKO(o);

    if (eneAnimPlayOnce(o, e->model, 0.8f) == 1) {
        e->model->anim = etskTakoTurnStartAnim;
        e->model->frame = 0.0f;
        e->state = 6;
    }
}

void takoShoot(Obj *o) {
    TakoObj *e = TAKO(o);
    s32 pad;
    s32 face;
    f32 step;

    step = 0.3f;
    if (eneAnimPlayOnce(o, e->model, step) == 1) {
        e->model->anim = etskTakoAnim;
        e->model->frame = polAnimeLastFrame(e->model);
        e->state = 7;
    }
    if (5.0 < e->model->frame && e->model->frame <= 5.0 + step) {
        eneSetSd(o, 0x22F);
        face = eneFaceWay(o);
        switch (face) {
            case 1:
                eneMakeEx(NULL, ENE_TAKO_TAMA_MAIN, e->zone, e->x + 4.0f, e->y + 9.0f, e->z, 2,
                          (u32 (*)(void *, void *, void *)) face);
                break;
            case 4:
                eneMakeEx(NULL, ENE_TAKO_TAMA_MAIN, e->zone, e->x - 4.0f, e->y + 9.0f, e->z, 2,
                          (u32 (*)(void *, void *, void *)) face);
                break;
            case 8:
                eneMakeEx(NULL, ENE_TAKO_TAMA_MAIN, e->zone, e->x, e->y + 9.0f, e->z + 4.0f, 2,
                          (u32 (*)(void *, void *, void *)) face);
                break;
            case 2:
                eneMakeEx(NULL, ENE_TAKO_TAMA_MAIN, e->zone, e->x, e->y + 9.0f, e->z - 4.0f, 2,
                          (u32 (*)(void *, void *, void *)) face);
                break;
        }
    }
}

void takoShootEnd(Obj *o) {
    TakoObj *e = TAKO(o);

    if (eneAnimPlayOnce(o, e->model, -1.0f) == 1) {
        e->state = 4;
    }
}

void takoFloatWatch(Obj *o) {
    TakoObj *e = TAKO(o);

    if (enePlInRangeCheck(o, 50.0f, 0) || fabs(e->x - e->homeX) >= 120.0) {
        e->state = 9;
    }
}

void takoFloatBrake(Obj *o) {
    TakoObj *e = TAKO(o);
    s32 pad[2];

    e->vel[0] += 0.3;
    if (e->vel[0] >= 0.0) {
        e->vel[0] = 0.0f;
        if (eneAnimPlayOnce(o, e->model, 1.0f) != 0) {
            e->model->anim = etskTakoTurnStartAnim;
            e->model->frame = 0.0f;
            e->state = 10;
        }
    }
}

void takoFloatShoot(Obj *o) {
    TakoObj *e = TAKO(o);
    s32 pad[2];
    f32 step;

    step = 0.3f;
    if (eneAnimPlayOnce(o, e->model, step) == 1) {
        e->vel[0] += 0.5;
        if (e->vel[0] >= 3.0) {
            e->vel[0] = 3.0f;
        }
    }
    if (5.0 < e->model->frame && e->model->frame <= 5.0 + step) {
        eneSetSd(o, 0x22F);
        eneMakeEx(NULL, ENE_TAKO_TAMA_MAIN, e->zone, e->x - 4.0f, e->y + 9.0f, e->z, 2, (u32 (*)(void *, void *, void *)) 4);
    }
}

s32 takoSumiMain(Obj *o, s16 msg, void *arg) {
    SumiObj *e = SUMI(o);
    s32 r;
    s32 pad;
    s16 flag;
    s16 way;
    s32 pad2[4];
    f32 v[3];

    switch (msg) {
        case 2:
            o->name = "sumi";
            r = eneSetup(o, arg);
            way = r & 0xFFFF;
            flag = (r >> 16) & 0xFFFF;
            PRINTF("type %x, atk_dir %x, atr %x\n", r, way, flag);
            e->timer = 15;
            if ((e->model = enePolModelMake(o, (Gfx *) takoSumiDl, NULL, 1)) == NULL) {
                return -1;
            }
            floadGetSetRam((void **) e->model->segs, 0x123);
            e->model->flags |= 7;
            e->model->scale[2] = 0.05f;
            e->model->scale[0] = e->model->scale[1] = e->model->scale[2];
            fadeAnimStart(&e->anim, takoSumiAnim, 0, &e->model->segs[3], floadGetSetFile(0x123, 0));
            if (flag != 0) {
                if ((e->atk = acollAtkAddYZ_r(ene_atk_root_frame, (Frame *) e->model,
                                              e->model->scale[0] * 10.0f * 55.0f, 0, 0, 0)) == NULL) {
                    return -1;
                }
                e->atk->kind = 0x7B;
            }
            switch (way) {
                case 1:
                    e->vel[0] = 1.5f;
                    break;
                case 4:
                    e->vel[0] = -1.5f;
                    break;
                case 8:
                    e->vel[2] = 1.5f;
                    break;
                case 2:
                    e->vel[2] = -1.5f;
                    break;
            }
            if (e->atk != NULL) {
                v[0] = e->vel[0];
                v[1] = e->vel[1];
                v[2] = e->vel[2];
                e->atk->info = *(ACollInfo *) v;
            }
            e->behave = 0;
            e->state = 1;
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (e->state) {
                case 1:
                    e->timer--;
                    if (e->timer <= 0) {
                        e->state = 2;
                    }
                    e->model->scale[0] *= 1.05;
                    e->model->scale[1] *= 1.05;
                    e->model->scale[2] *= 1.05;
                    break;
                case 2:
                    e->vel[0] *= 0.9;
                    e->vel[2] *= 0.9;
                    if (e->model->scale[0] < 0.35) {
                        e->model->scale[0] *= 1.1;
                        e->model->scale[1] *= 1.1;
                        e->model->scale[2] *= 1.1;
                    } else if (fadeAnimStep(&e->anim) != 0) {
                        return -1;
                    }
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_tako_sumi: 死亡\n");
                return -1;
            }
        case 3:
            e->model->dl = eneDlEnvPrimColor(o, 0x20, 0, 0x20, 0, 0, 0, 0xFF);
            break;
        case 1:
            if (e->model != NULL) {
                frmDestroy((Frame *) e->model);
            }
            if (e->atk != NULL) {
                frmDestroy((Frame *) e->atk);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

s32 takoTamaMain(Obj *o, s16 msg, void *arg) {
    TamaObj *e = TAMA(o);
    s32 pad;
    s32 way;
    f32 v[3];

    switch (msg) {
        case 2:
            o->name = "tama";
            way = eneSetup(o, arg);
            if ((e->model = eneNewPol(o, (Gfx *) takoTamaDl)) == NULL) {
                return -1;
            }
            if ((e->atk = acollAtkAddYZ_r(ene_atk_root_frame, (Frame *) e->model, 0x14, 0, 0, 0)) == NULL) {
                return -1;
            }
            e->atk->kind = 0x7B;
            e->model->flags |= 7;
            e->model->scale[2] = 0.1f;
            e->model->scale[0] = e->model->scale[1] = e->model->scale[2];
            e->count = 0;
            e->behave = 4;
            if (way == 4) {
                e->vel[0] = -3.0f;
            } else if (way == 1) {
                e->vel[0] = 3.0f;
            } else if (way == 8) {
                e->vel[2] = 3.0f;
            } else if (way == 2) {
                e->vel[2] = -3.0f;
            }
            v[0] = e->vel[0];
            v[1] = e->vel[1];
            v[2] = e->vel[2];
            e->atk->info = *(ACollInfo *) v;
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            if (e->hit & 0xE) {
                e->behave |= 0x4000000;
            }
            if (e->count++ > 100) {
                e->behave |= 0x10000000;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_tako_tama: 死亡\n");
                return -1;
            }
            break;
        case 1:
            if (e->model != NULL) frmDestroy((Frame *) e->model);
            if (e->atk != NULL) frmDestroy((Frame *) e->atk);
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
