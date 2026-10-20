#include "tlb.h"

/* File 145: "beto", a night ghost (Betobeto-san) that fades in near the camera and comes after
 * the players, by its type walking, sliding, hopping when a player near it presses A, circling or
 * swaying; hit, it is knocked back and bounces; by day (5:00-19:00) it turns, fades out and is
 * gone (its placement released). */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

/* libultra's, through its address (code at 0x08000000 cannot jal it). */
#define sinf (*sinf)

extern u8 etskBetoAnim[];

/* A beto's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Beto {
    /* 0x00 */ EneObj e;
    /* 0x4C */ s16 homeX; /* x where it was placed */
    /* 0x4E */ u8 type;   /* its placement's (eneSetup): 1-5 how it moves */
    /* 0x4F */ u8 alpha;  /* its model's and shadow's */
    /* 0x50 */ u8 way;    /* to the nearest player (eneNearestPlWay) */
    /* 0x51 */ u8 unused51; /* written 0 at its start, never read */
    /* 0x52 */ u8 unused52; /* written 0 at its start, never read */
    /* 0x53 */ s8 fade;   /* the alpha's step while it pulses (betoPulseAlpha) */
} Beto;

#define BETO(obj) ((Beto *) FX(obj))

void betoPulseAlpha(Obj *o, s32 step);

/* "beto".
 * Matched 2026-10-11 (IDO_NOTES 369): soundIncamCam is called as int-returning (it returns
 * SoundIncam2's value: sndloc.c and tlb.h declare it s32, 2026-10-17), so its dropped result
 * holds v0 and the model pointer takes v1. The slide speeds are a ternary of doubles (an if/else
 * folds -1.2 / 1.2 to floats); the sway divides by the int 4 (a "/ 4.0f" becomes a multiply);
 * the pads (three words above speed, two below) are the game's frame. */
s32 Etsk_beto_main(Obj *o, s16 msg, void *arg) {
    Beto *w = BETO(o);
    s32 pad[3];
    f32 speed = 1.0f;
    s32 hour;
    s32 n;
    f32 angle;
    s32 pad2[2];

    switch (msg) {
        case 2:
            o->name = "beto";
            w->type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneAnimeModelAlpha(o, (u32) etskBetoAnim, 0)) == NULL) {
                return -1;
            }
            if ((w->e.shadow = (Obj *) shadowCreate()) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 50, 80, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 120;
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 80, 160, 0, 0, 0)) == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                PRINTF("Etsk_beto: 地面にセット失敗\n");
                return -1;
            }
            w->e.behave = 0x418902;
            w->e.hp = 2;
            w->unused52 = 0;
            w->unused51 = 0;
            w->e.rot[1] = eneCourseAngle(o, 8);
            w->way = eneNearestPlWay(o);
            w->alpha = 0x80;
            w->e.atk->flags |= 2;
            w->e.dmg->flags |= 2;
            w->homeX = w->e.x;
            switch (w->type) {
                case 1:
                default:
                    w->e.state = 2;
                    break;
                case 2:
                    w->e.state = 4;
                    break;
                case 3:
                    w->e.state = 8;
                    break;
                case 4:
                    w->e.state = 7;
                    break;
                case 5:
                    w->e.state = 6;
                    break;
                case 6:
                    w->e.state = 0;
                    w->type = 1;
                    break;
                case 7:
                    w->e.state = 0;
                    w->type = 2;
                    break;
                case 8:
                    w->e.state = 0;
                    w->type = 4;
                    break;
            }
            break;
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            hour = gGameWork.time / 0x10000;
            if (hour < 19 && hour >= 5) {
                if (w->e.state < 11) {
                    w->e.state = 11;
                }
            }
            switch (w->e.state) {
                case 0:
                    speed = 0.2f;
                    w->alpha = 0x40;
                    w->way = eneNearestPlWay(o);
                    if (eneCamNearCheck(o, 40.0f)) {
                        w->e.state = 1;
                    }
                    break;
                case 1:
                    w->e.behave &= ~0x4000;
                    w->alpha += 8;
                    if (w->alpha >= 0x80) {
                        w->alpha = 0x80;
                        if (w->type == 1) {
                            w->e.state = 2;
                        } else if (w->type == 2) {
                            w->e.state = 4;
                        } else if (w->type == 4) {
                            w->e.state = 7;
                        } else {
                            w->e.state = 2;
                        }
                    }
                    if (w->alpha >= 0x40) {
                        eneTurnToward(o, eneCourseAngle(o, w->way), PI / 50);
                    }
                    break;
                case 2:
                    w->e.behave |= 0x4400;
                    w->alpha = 0x80;
                    w->e.vel[0] = (w->way == 4) ? -1.2 : 1.2;
                    w->e.state = 3;
                    break;
                case 4:
                    w->e.behave |= 0x4000;
                    w->alpha = 0x80;
                    if (w->way == 4) {
                        w->e.vel[0] = -2.0f;
                    } else {
                        w->e.vel[0] = 2.0f;
                    }
                    w->e.state = 5;
                    /* fallthrough */
                case 5:
                    speed = 2.0f;
                    w->e.rot[0] = 0.3141592741f;
                    break;
                case 6:
                    speed = 0.6999999881f;
                    betoPulseAlpha(o, 8);
                    w->way = eneNearestPlWay(o);
                    eneTurnToward(o, eneCourseAngle(o, w->way), PI / 30);
                    if ((n = eneNearestPl(o)) != 0) {
                        if (enePlInRangeCheck(o, 40.0f, n - 1)) {
                            if (joyGetHeldTime(n - 1, A_BUTTON)) {
                                if (w->e.hit & 1) {
                                    soundIncamCam(0x239, ((AnimeFrame *) w->e.model)->pos[0],
                                                  ((AnimeFrame *) w->e.model)->pos[1],
                                                  ((AnimeFrame *) w->e.model)->pos[2]);
                                    w->e.vel[1] = 5.5f;
                                    itexPuffWhite(((AnimeFrame *) w->e.model)->pos[0],
                                                  ((AnimeFrame *) w->e.model)->pos[1],
                                                  ((AnimeFrame *) w->e.model)->pos[2], 0.04f);
                                }
                            }
                        }
                    }
                    break;
                case 7:
                    speed = 1.5f;
                    w->e.rot[1] += 0.104719758f;
                    betoPulseAlpha(o, 5);
                    if (w->e.hit & 1) {
                        soundIncamCam(0x239, ((AnimeFrame *) w->e.model)->pos[0],
                                      ((AnimeFrame *) w->e.model)->pos[1], ((AnimeFrame *) w->e.model)->pos[2]);
                        w->e.vel[1] = 5.0f;
                        itexPuffWhite(((AnimeFrame *) w->e.model)->pos[0], ((AnimeFrame *) w->e.model)->pos[1],
                                      ((AnimeFrame *) w->e.model)->pos[2], 0.04f);
                    }
                    break;
                case 8:
                    speed = 0.6999999881f;
                    betoPulseAlpha(o, 6);
                    w->e.rot[1] = eneCourseAngle(o, 8) + sinf(gGameFrameCount / 5.0f) * 3.141592741f / 4;
                    break;
                case 9:
                    w->e.behave &= ~0x4400;
                    speed = 0.0f;
                    w->e.vel[0] = eneKnockbackSpeed(o, w->e.dmg, 1.0f);
                    w->alpha = 0xF0;
                    w->e.hit &= ~1;
                    w->e.state = 10;
                    w->e.vel[1] = 4.0f;
                    /* fallthrough */
                case 10:
                    eneAnimLoop(o, (AnimeFrame *) w->e.model, 1.0f);
                    if (w->e.vel[1] < 0.0f) {
                        w->e.dmg->flags |= 2;
                    }
                    if (w->e.hit & 1) {
                        w->e.dmg->flags |= 2;
                        if (w->e.vel[1] < 0.4) {
                            w->e.vel[1] = 0.0f;
                            w->e.vel[0] = 0.0f;
                            if (w->type == 1) {
                                w->e.state = 2;
                            } else if (w->type == 2) {
                                w->e.state = 4;
                            } else if (w->type == 3) {
                                w->e.state = 8;
                            } else if (w->type == 4) {
                                w->e.state = 7;
                            } else if (w->type == 5) {
                                w->e.state = 6;
                            } else {
                                w->e.state = 2;
                            }
                        } else {
                            w->e.vel[1] *= -0.5;
                            w->e.vel[0] *= 0.9;
                            w->e.hit &= ~1;
                        }
                    }
                    break;
                case 11:
                    angle = eneCourseAngle(o, 8);
                    speed = 0.0f;
                    eneTurnToward(o, angle, PI / 40);
                    w->alpha += 8;
                    if (w->alpha >= 0xF0) {
                        w->alpha = 0xF0;
                        w->e.state = 12;
                    }
                    break;
                case 12:
                    eneTurnToward(o, eneCourseAngle(o, 8), PI / 40);
                    w->alpha -= 8;
                    if (w->alpha <= 0) {
                        eneReleaseSpawn(w->e.spawn);
                        eneClearSpawnWait(w->e.spawn);
                        return -1;
                    }
                    break;
            }
            eneAnimLoop(o, (AnimeFrame *) w->e.model, speed);
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_beto_main: 死亡\n");
                return -1;
            }
            if (w->e.hit & 0x10) {
                w->e.hit &= ~0x10;
                w->e.dmg->flags &= ~2;
                w->e.state = 9;
            }
            break;
        case 3:
            ((AnimeFrame *) w->e.model)->dl = gGfxDlHead;
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            ((MPolFrame *) w->e.shadow)->dl = gGfxDlHead;
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            if (w->e.model != NULL) {
                frmDestroy(w->e.model);
            }
            if (w->e.shadow != NULL) {
                frmDestroy((Frame *) w->e.shadow);
            }
            if (w->e.atk != NULL) {
                frmDestroy(&w->e.atk->frame);
            }
            if (w->e.dmg != NULL) {
                frmDestroy(&w->e.dmg->frame);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Pulses the alpha between 0x40 and 0x80 by step. */
void betoPulseAlpha(Obj *o, s32 step) {
    Beto *w = BETO(o);

    if (w->fade == 0) {
        w->fade = step;
    }
    if (w->alpha >= 0x80) {
        w->alpha = 0x80;
        w->fade = -fabsf(step);
    }
    if (w->alpha <= 0x40) {
        w->alpha = 0x40;
        w->fade = fabsf(step);
    }
    w->alpha += w->fade;
}
