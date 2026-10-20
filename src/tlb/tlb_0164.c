#include "tlb.h"

/* The "hipeye" enemy kind (archive file 164), with its "unko" (kind 0x21F) and the unko's steam,
 * "yuge" (kind 0x220). A hipeye's placement type 1 backs up until it meets a wall (state 0),
 * plays its texture animation once, dropping an unko at the middle of it (state 1), and walks
 * away for four cycles of its walk (state 2), over and over; type 2 waits for a player within
 * 70 (state 3), drops one unko the same way (state 4) and then only walks in place (state 5).
 * State 1 tests the save's allClear and a time of day from 7:00 to 7:30 and prints which held
 * ("条件" 1 or 2), but drops the same unko either way. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 hipeyeYugeSteamDl[]; /* the steam's display list */
extern u8 etskHipeyeWalkAnim[]; /* the hipeye's walk */
extern u8 hipeyeUnkoAnim[]; /* the unko's animation (types 0 and 2) */
extern u8 hipeyeUnkoDl[]; /* the unko's display list (type 1) */
extern u8 etskHipeyeHalf0Dl[]; /* the hipeye's two halves, for a cut */
extern u8 etskHipeyeHalf1Dl[];

extern u8 hipeyeYugeTex0[];
extern u8 hipeyeYugeTex2[];
extern u8 hipeyeYugeTex5[];
AnimFrame hipeyeTexAnim[] = {
    { 0x03004B10, 3 },
    { 0x03005330, 3 },
    { 0x03005B50, 6 },
    { 0x03005330, 3 },
    { 0x03004B10, 3 },
    { 0, 0 },
}; /* the hipeye's textures (segment 3): five frames, the middle
                                           one held 6 steps */
AnimFrame hipeyeYugeAnim[] = {
    { (u32) hipeyeYugeTex0, 2 },
    { 0x03000D80, 2 },
    { (u32) hipeyeYugeTex2, 2 },
    { 0x03001D80, 2 },
    { 0x03002580, 2 },
    { (u32) hipeyeYugeTex5, 2 },
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
}; /* the steam's 16 textures */

/* A hipeye's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Hipeye {
    /* 0x00 */ EneObj e;
    /* 0x4C */ Anim anim; /* its textures, played once in states 1 and 4 */
    /* 0x5C */ u16 timer; /* steps to wait */
    /* 0x5E */ s16 count; /* type 1: walk cycles in state 2 (type 2 sets it to its z) */
} Hipeye;

#define HIPEYE(obj) ((Hipeye *) FX(obj))

/* An unko's work, at its task + 0x20. */
typedef struct HipeyeUnko {
    /* 0x00 */ EneObj e;
    /* 0x4C */ u16 timer; /* steps before it fades */
    /* 0x4E */ u8 alpha;
    /* 0x4F */ u8 type; /* its placement's: 0 falls, then can be stepped on; 1 falls and steams;
                         * 2 stays */
    /* 0x50 */ u8 steamed; /* type 1: its steam is out */
    /* 0x51 */ u8 splashed; /* it has fallen into water */
    /* 0x52 */ u8 squashed; /* type 0: stepped on */
} HipeyeUnko;

#define UNKO(obj) ((HipeyeUnko *) FX(obj))

/* eneproc.c defines it with five parameters; the unko passes a sixth (as eneDlEnvColorLit's). */
#define ENV_COLOR ((Gfx *(*)(Obj *, s32, s32, s32, s32, s32)) eneDlEnvColor)

/* The steam's work, at its task + 0x20. */
typedef struct HipeyeYuge {
    /* 0x00 */ EneObj e;
    /* 0x4C */ Anim anim; /* its pictures */
    /* 0x5C */ u8 alpha;
} HipeyeYuge;

#define YUGE(obj) ((HipeyeYuge *) FX(obj))

void hipeyeFunc1_1(Obj *o);
void hipeyeFunc1_2(Obj *o);
void hipeyeFunc1_3(Obj *o);
void hipeyeFunc2_1(Obj *o);
void hipeyeFunc2_2(Obj *o);
void hipeyeFunc2_3(Obj *o);

/* The hipeye's task. */
s32 Etsk_hipeye_main(Obj *o, s16 msg, void *arg) {
    Hipeye *w = HIPEYE(o);
    u32 file;
    u32 type;

    switch (msg) {
        case 2:
            o->name = "hipeye";
            type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneNewAnime(o, (u32) etskHipeyeWalkAnim)) == NULL) {
                return -1;
            }
            file = floadGetSetFile(0x1E, 0);
            fadeAnimStart(&w->anim, hipeyeTexAnim, 0, &((AnimeFrame *) w->e.model)->segs[3], file);
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 50, 80, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 100, 160, 0, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->e.shadow = (Obj *) shadowCreate()) == NULL) {
                return -1;
            }
            w->e.halves[0] = (u32) etskHipeyeHalf0Dl;
            w->e.halves[1] = (u32) etskHipeyeHalf1Dl;
            if (eneSetOnEarth(o) == -1) {
                return -1;
            }
            w->e.behave = 0xC10280;
            w->timer = 0;
            w->count = 0;
            if (type == 1) {
                w->e.state = 0;
            } else if (type == 2) {
                w->count = w->e.z;
                w->e.state = 3;
            }
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*hipeyeFunc1_1)(o);
                    break;
                case 1:
                    (*hipeyeFunc1_2)(o);
                    break;
                case 2:
                    (*hipeyeFunc1_3)(o);
                    break;
                case 3:
                    (*hipeyeFunc2_1)(o);
                    break;
                case 4:
                    (*hipeyeFunc2_2)(o);
                    break;
                case 5:
                    (*hipeyeFunc2_3)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_hipeye_main: 死亡\n");
                return -1;
            }
            break;
        case 1:
            if (w->e.model != NULL) { frmDestroy(w->e.model); }
            if (w->e.atk != NULL) { frmDestroy(&w->e.atk->frame); }
            if (w->e.dmg != NULL) { frmDestroy(&w->e.dmg->frame); }
            if (w->e.shadow != NULL) { frmDestroy((Frame *) w->e.shadow); }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* State 0 (type 1): after a wait, backs up (its speed falling to -0.7, its walk played forward)
 * until it meets a wall. */
void hipeyeFunc1_1(Obj *o) {
    Hipeye *w = HIPEYE(o);
    f32 angle;
    f32 step;

    if (w->timer - 1 >= 0) {
        w->timer--;
        return;
    }
    angle = eneCourseAngle(o, 4);
    w->e.rot[1] = angle + 0.3926990926f;
    w->e.vel[0] -= 0.1;
    if (w->e.vel[0] <= -0.7) {
        w->e.vel[0] = -0.7f;
    }
    step = w->e.vel[0] / -0.7;
    eneAnimLoop(o, (AnimeFrame *) w->e.model, step);
    if (w->e.hit & 4) {
        w->e.state = 1;
        w->e.vel[0] = 0.0f;
    }
}

/* State 1 (type 1): plays its textures once, dropping an unko (with its sound) at the middle
 * frame; then waits 10 (state 2). */
void hipeyeFunc1_2(Obj *o) {
    Hipeye *w = HIPEYE(o);
    u16 hour;
    s32 minute;
    s32 pad; /* "s32 time = gTimeOfDay;" or a float "frac" for the minutes here moves the code */

    if (fadeAnimStep(&w->anim)) {
        w->timer = 10;
        w->e.state = 2;
        w->e.vel[0] = 0.0f;
    }
    if (w->anim.tick == 3) {
        eneSetSd(o, 0x22F);
        if (gSaveData.allClear) {
            hour = gTimeOfDay >> 16;
            minute = (f32) (gTimeOfDay & 0xFFFF) / 65536.0f * 60.0f;
            if (hour == 7 && minute < 31) {
                PRINTF("Efunc_hipeye1_2(): 条件１\n");
                eneMakeEx(NULL, ENE_HIPEYE_UNKO_MAIN, w->e.zone, w->e.x - 6.0f, w->e.y + 13.0f, w->e.z, 2, NULL);
            } else {
                PRINTF("Efunc_hipeye1_2(): 条件２\n");
                eneMakeEx(NULL, ENE_HIPEYE_UNKO_MAIN, w->e.zone, w->e.x - 6.0f, w->e.y + 13.0f, w->e.z, 2, NULL);
            }
        } else {
            eneMakeEx(NULL, ENE_HIPEYE_UNKO_MAIN, w->e.zone, w->e.x - 6.0f, w->e.y + 13.0f, w->e.z, 2, NULL);
        }
    }
}

/* State 2 (type 1): after a wait, walks away (speeding up to 1.4, its walk played backward); at
 * the end of its fourth walk cycle, waits and backs up again (state 0). */
void hipeyeFunc1_3(Obj *o) {
    Hipeye *w = HIPEYE(o);
    f32 angle;
    f32 step;
    s32 looped;

    if (w->timer - 1 >= 0) {
        w->timer--;
        return;
    }
    w->e.vel[0] += 0.2;
    if (w->e.vel[0] >= 1.4) {
        w->e.vel[0] = 1.4f;
    }
    angle = eneCourseAngle(o, 4);
    w->e.rot[1] = angle + 0.3926990926f;
    step = w->e.vel[0] / -0.7;
    looped = eneAnimLoop(o, (AnimeFrame *) w->e.model, step);
    if (looped) {
        if (w->count++ >= 3) {
            w->count = 0;
            w->timer = 10;
            w->e.state = 0;
            w->e.vel[0] = 0.0f;
        }
    }
}

/* State 3 (type 2): waits for a player within 70. */
void hipeyeFunc2_1(Obj *o) {
    if (enePlInRangeCheck(o, 70.0f, 0)) {
        HIPEYE(o)->e.state = 4;
    }
}

/* State 4 (type 2): plays its textures once, dropping an unko at the middle frame; then waits 10
 * (state 5). */
void hipeyeFunc2_2(Obj *o) {
    Hipeye *w = HIPEYE(o);

    if (fadeAnimStep(&w->anim)) {
        w->timer = 10;
        w->e.state = 5;
        w->e.vel[0] = 0.0f;
    }
    if (w->anim.tick == 3) {
        eneMakeEx(NULL, ENE_HIPEYE_UNKO_MAIN, w->e.zone, w->e.x, w->e.y + 13.0f, w->e.z + 6.0f, 2, NULL);
    }
}

/* State 5 (type 2): after the wait, plays its walk backward, speeding up to 1.4 (the speed kept in
 * vel[2], not vel[0] as in state 2). */
void hipeyeFunc2_3(Obj *o) {
    Hipeye *w = HIPEYE(o);
    f32 step;

    if (w->timer - 1 >= 0) {
        w->timer--;
        return;
    }
    w->e.vel[2] += 0.2;
    if (w->e.vel[2] >= 1.4) {
        w->e.vel[2] = 1.4f;
    }
    step = w->e.vel[2] / -0.7;
    eneAnimLoop(o, (AnimeFrame *) w->e.model, step);
}

/* "unko": what a hipeye drops (kind 0x21F). Types 0 and 1 are thrown up and away (back along the
 * course, or across it where z is below 0), growing and tilting along their path, and land;
 * type 0 can then be stepped on (squashed flat, with a print of its sound), type 1 flattens and
 * puts out its steam; after 120 steps type 0 fades out and type 1 shrinks away. Type 2 only
 * plays its animation. One that falls into water splashes. */
s32 hipeyeUnkoMain(Obj *o, s16 msg, void *arg) {
    HipeyeUnko *w = UNKO(o);
    s32 oy;
    Vec3f v;
    Vec3f p;
    Vec3f push;

    switch (msg) {
        case 2:
            o->name = "unko";
            w->type = eneSetup(o, arg);
            if (w->type == 0) {
                if ((w->e.model = (Frame *) eneNewAnime(o, (u32) hipeyeUnkoAnim)) == NULL) {
                    return -1;
                }
                ((AnimeFrame *) w->e.model)->scale[2] = 0.1f;
                ((AnimeFrame *) w->e.model)->scale[0] = ((AnimeFrame *) w->e.model)->scale[1] =
                    ((AnimeFrame *) w->e.model)->scale[2];
                ((AnimeFrame *) w->e.model)->frame = 3.0f;
                if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 30, 55, 0, 0, 0)) == NULL) {
                    return -1;
                }
                w->e.dmg->flags &= ~2;
                w->e.behave = 0x803;
            } else if (w->type == 1) {
                if ((w->e.model = (Frame *) enePolModelMake(o, (Gfx *) hipeyeUnkoDl, NULL, 0)) == NULL) {
                    return -1;
                }
                ((PolFrame *) w->e.model)->scale[2] = 0.1f;
                ((PolFrame *) w->e.model)->scale[0] = ((PolFrame *) w->e.model)->scale[1] =
                    ((PolFrame *) w->e.model)->scale[2];
                w->e.behave = 0x800;
            } else if (w->type == 2) {
                if ((w->e.model = (Frame *) eneNewAnime(o, (u32) hipeyeUnkoAnim)) == NULL) {
                    return -1;
                }
                ((AnimeFrame *) w->e.model)->scale[2] = 0.2f;
                ((AnimeFrame *) w->e.model)->scale[0] = ((AnimeFrame *) w->e.model)->scale[1] =
                    ((AnimeFrame *) w->e.model)->scale[2];
                w->e.behave = 0x800;
            }
            if (w->type == 1) {
                oy = 0;
            } else {
                oy = 15;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 15, oy, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 123;
            w->timer = 120;
            w->steamed = 0;
            w->splashed = 0;
            w->squashed = 0;
            w->alpha = 0xFF;
            if (w->type == 0 || w->type == 1) {
                w->e.vel[1] = 2.5f;
                if (w->e.z < 0.0f) {
                    w->e.vel[2] = 6.0f;
                    w->e.rot[1] = eneCourseAngle(o, 4) + 1.570796371f;
                } else {
                    w->e.vel[0] = -6.0f;
                    w->e.rot[1] = eneCourseAngle(o, 4);
                }
            }
            v.x = w->e.vel[0];
            v.y = w->e.vel[1];
            v.z = w->e.vel[2];
            *(Vec3f *) &w->e.atk->info = v;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            if (w->e.hit & 1) {
                w->e.vel[0] = 0.0f;
                w->e.vel[2] = 0.0f;
                w->e.rot[0] = 0.0f;
                if (w->type == 0) {
                    if (w->e.hit & 0x10) {
                        eneSetSd(o, 0x2ED);
                        PRINTF("ちゅぽ\n");
                        w->squashed = 1;
                    }
                    if (w->squashed) {
                        w->e.dmg->flags &= ~2;
                    } else {
                        w->e.dmg->flags |= 2;
                    }
                    if (w->squashed) {
                        if (((AnimeFrame *) w->e.model)->scale[1] > 0.08) {
                            ((AnimeFrame *) w->e.model)->scale[1] -= 0.03;
                            ((AnimeFrame *) w->e.model)->scale[0] += 0.03;
                            ((AnimeFrame *) w->e.model)->scale[2] = ((AnimeFrame *) w->e.model)->scale[0];
                        } else {
                            w->timer = 0;
                        }
                    }
                }
                if (w->type == 1) {
                    if (((PolFrame *) w->e.model)->scale[1] > 0.08) {
                        ((PolFrame *) w->e.model)->scale[1] -= 0.03;
                        ((PolFrame *) w->e.model)->scale[0] += 0.03;
                        ((PolFrame *) w->e.model)->scale[2] = ((PolFrame *) w->e.model)->scale[0];
                    }
                    if (!w->steamed && !w->splashed) {
                        eneMakeEx(o, ENE_HIPEYE_YUGE_MAIN, w->e.zone, w->e.x, w->e.y + 2.0f, w->e.z, 0, NULL);
                        w->steamed = 1;
                    }
                } else {
                    eneAnimeLoop(o, (AnimeFrame *) w->e.model, 0.8f);
                    if ((((AnimeFrame *) w->e.model)->frame >= 0.0 && ((AnimeFrame *) w->e.model)->frame < 0.8) ||
                        (((AnimeFrame *) w->e.model)->frame >= 6.0 && ((AnimeFrame *) w->e.model)->frame < 6.8)) {
                        if (w->alpha >= 0x80) {
                            eneSetSd2(o, 0x304, 0, 0);
                        }
                    }
                }
            } else {
                if (w->type == 0) {
                    ((AnimeFrame *) w->e.model)->scale[0] += 0.015;
                    if (((AnimeFrame *) w->e.model)->scale[0] >= 0.2) {
                        ((AnimeFrame *) w->e.model)->scale[0] = 0.2f;
                    }
                    ((AnimeFrame *) w->e.model)->scale[1] = ((AnimeFrame *) w->e.model)->scale[2] =
                        ((AnimeFrame *) w->e.model)->scale[0];
                } else if (w->type == 1) {
                    ((PolFrame *) w->e.model)->scale[0] += 0.015;
                    if (((PolFrame *) w->e.model)->scale[0] >= 0.2) {
                        ((PolFrame *) w->e.model)->scale[0] = 0.2f;
                    }
                    ((PolFrame *) w->e.model)->scale[1] = ((PolFrame *) w->e.model)->scale[2] =
                        ((PolFrame *) w->e.model)->scale[0];
                }
                if (w->e.vel[0] != 0.0f) {
                    w->e.vel[0] *= 0.9;
                    w->e.rot[0] = mathAtan2(fabsf(w->e.vel[0]), w->e.vel[1]) + 3.141592741f;
                } else if (w->e.vel[2] != 0.0f) {
                    w->e.vel[2] *= 0.9;
                    w->e.rot[0] = mathAtan2(fabsf(w->e.vel[2]), w->e.vel[1]) + 3.141592741f;
                }
            }
            if (w->timer - 1 >= 0) {
                if (w->type != 2) {
                    w->timer = w->timer - 1;
                }
            } else if (w->type == 0) {
                frmMove(w->e.model, gFrmLayer8);
                if (w->alpha - 32 > 0) {
                    w->alpha -= 32;
                } else {
                    w->alpha = 0;
                    w->e.behave |= 0x40000;
                }
                if (w->alpha < 0x80) {
                    w->e.atk->flags &= ~2;
                }
            } else if (w->type == 1) {
                if (((PolFrame *) w->e.model)->scale[1] > 0.01) {
                    ((PolFrame *) w->e.model)->scale[1] -= 0.01;
                    ((PolFrame *) w->e.model)->scale[0] -= 0.005;
                    ((PolFrame *) w->e.model)->scale[2] = ((PolFrame *) w->e.model)->scale[0];
                } else {
                    w->e.behave |= 0x40000;
                }
                if (((PolFrame *) w->e.model)->scale[0] < 0.4) {
                    w->e.atk->flags &= ~2;
                }
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_hipeye_unko: 死亡\n");
                return -1;
            }
            if (!w->splashed) {
                p.x = ((AnimeFrame *) w->e.model)->pos[0];
                p.y = ((AnimeFrame *) w->e.model)->pos[1];
                p.z = ((AnimeFrame *) w->e.model)->pos[2];
                if (rideTree2PointIn(&p, &push)) {
                    eneSetSd(o, 0x298);
                    effectSibukiCreateWhite(p.x, p.y, p.z, 0.05f, 0.05f, 0.005f, 0.005f);
                    w->splashed = 1;
                }
            }
            /* fallthrough */
        case 3:
            if (w->type == 1) {
                ((PolFrame *) w->e.model)->dl = ENV_COLOR(o, 0xFF, 0xFF, 0xBF, 0x1F, 1);
            } else {
                ((AnimeFrame *) w->e.model)->dl = gGfxDlHead;
                gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
                gSPEndDisplayList(gGfxDlHead++);
            }
            break;
        case 1:
            if (w->e.model != NULL) { frmDestroy(w->e.model); }
            if (w->e.atk != NULL) { frmDestroy(&w->e.atk->frame); }
            if (w->e.dmg != NULL) { frmDestroy(&w->e.dmg->frame); }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "yuge": the steam over an unko, a picture of 16 frames that fades in to 96. */
s32 hipeyeYugeMain(Obj *o, s16 msg, void *arg) {
    HipeyeYuge *w = YUGE(o);

    switch (msg) {
        case 2:
            o->name = "yuge";
            eneSetup(o, arg);
            if ((w->e.model = (Frame *) enePolModelMake(o, (Gfx *) hipeyeYugeSteamDl, NULL, 1)) == NULL) {
                return -1;
            }
            floadGetSetRam((void **) ((PolFrame *) w->e.model)->segs, 0x125);
            ((PolFrame *) w->e.model)->scale[0] = 0.15f;
            ((PolFrame *) w->e.model)->scale[1] = 0.1f;
            ((PolFrame *) w->e.model)->flags = 2;
            fadeAnimStart(&w->anim, hipeyeYugeAnim, 0, &((PolFrame *) w->e.model)->segs[3],
                          floadGetSetFile(0x125, 0));
            w->alpha = 0;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            fadeAnimStep(&w->anim);
            if (w->alpha + 4 <= 96) {
                w->alpha += 4;
            } else {
                w->alpha = 96;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_hipeye_yuge: 死亡\n");
                return -1;
            }
            /* fallthrough */
        case 3:
            ((PolFrame *) w->e.model)->dl = eneDlEnvPrimColor(o, 0xFF, 0xBF, 0x7F, 0xFF, 0xFF, 0xFF, w->alpha);
            break;
        case 1:
            if (w->e.model != NULL) { frmDestroy(w->e.model); }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
