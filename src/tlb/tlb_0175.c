#include "tlb.h"

/* The "nuppe" enemy kind (archive file 175, kind 0x230): a blob that sits until a player comes
 * within 70, then turns to the nearest player and creeps at it; a hit knocks it back and it
 * gathers itself (states 3 and 4), its last life sends it hopping at the players (state 5),
 * and it goes home when it strays more than 200 from where it was placed or leaves its course
 * (state 6). It takes three points of damage. Its animations are a table
 * (nuppeAnimTable) and its state is the animation's number. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

#pragma intrinsic(fabs)

/* An animation of the nuppe's table: the state with the same number plays it. */
typedef struct NuppeAnim {
    /* 0x00 */ u32 anim;
    /* 0x04 */ f32 speed; /* at one retrace a frame */
    /* 0x08 */ u8 loop; /* 1: loops (eneAnimLoop), else plays once */
} NuppeAnim; /* size 0x0C */

/* A nuppe's work, at its task + 0x20: an enemy's (EneObj) up to 0x40, then its own. */
typedef struct Nuppe {
    /* 0x00 */ u8 unused00;
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
    /* 0x40 */ f32 animSpeed; /* its animation's step at this frame rate */
    /* 0x44 */ f32 prevVel; /* vel[0] last step (state 5 leans with the change) */
    /* 0x48 */ f32 homeX; /* where it was placed, along its course (eneTeritoryCheck) */
    /* 0x4C */ u8 homeZone; /* and its course */
    /* 0x4D */ u8 type; /* its placement's type, from eneSetup: 0 moves */
    /* 0x4E */ u8 anim; /* its animation in the table, and its state */
    /* 0x4F */ u8 life; /* damage it takes */
    /* 0x50 */ s8 gravity; /* its fall's, in hundredths */
    /* 0x51 */ u8 wait; /* steps gathering itself (state 4) */
    /* 0x52 */ s8 dir; /* 0: the nearest player is on its way 1, 1: on the other side */
    /* 0x53 */ s8 target; /* the nearest player (-1: none) */
    /* 0x54 */ s8 flash; /* steps left of the hit flash */
    /* 0x55 */ u8 landed; /* eneLand's result last step */
    /* 0x56 */ s8 hitSe; /* steps before a blocked hit sounds again */
    /* 0x57 */ u8 alpha; /* its red tint's, rising while it hops */
} Nuppe;

#define NUPPE(obj) ((Nuppe *) FX(obj))

Gfx nuppeDrawDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetColorDither(G_CD_DISABLE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_TEX_EDGE2),
    gsDPSetCombineLERP(TEXEL0, 0, SHADE, 0, 0, 0, 0, TEXEL0, ENVIRONMENT, COMBINED, ENV_ALPHA, COMBINED, 0, 0, 0, COMBINED),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsSPEndDisplayList(),
}; /* its draw's render setup */
NuppeAnim nuppeAnimTable[] = {
    { 0x0300055C, 2.0f, 0 },
    { 0x0300055C, 0.0f, 0 },
    { 0x0300055C, 2.0f, 1 },
    { 0x03000AA8, 0.5f, 1 },
    { 0x03000AA8, 0.0f, 1 },
    { 0x030008E8, 3.4f, 1 },
    { 0x0300055C, 2.0f, 1 },
    { 0x0300055C, 0.0f, 0 },
    { 0x00000000, 0.0f, 0 },
    { 0x00000000, 0.0f, 0 },
};
extern u8 nuppeHalf0Dl[];
extern u8 nuppeHalf1Dl[];

s32 nuppeMain(Obj *o);

/* Sets animation n of the table: its step, scaled to the frame rate, from its first frame. */
void nuppeSetAnim(Obj *o, u8 n, NuppeAnim *tab) {
    Nuppe *w = NUPPE(o);

    w->anim = n;
    w->animSpeed = tab[n].speed / gRetracesPerFrame;
    w->model->anim = tab[n].anim;
    w->model->frame = 0.0f;
}

void nuppeNop1(void) {
}

/* Sets animation n of the table, at its last frame. */
void nuppeSetAnimEnd(Obj *o, u8 n, NuppeAnim *tab) {
    Nuppe *w = NUPPE(o);
    s32 pad;

    nuppeSetAnim(o, n, tab);
    w->model->frame = polAnimeLastFrame(w->model);
}

void nuppeNop2(void) {
}

/* Puts it back on its course and returns its heading there: mode 0 the heading less ang, 1 the
 * heading turned by ang and a half turn, -1 its own rotation, else the heading. */
f32 nuppeCourseAngle(Obj *o, f32 ang, s8 mode) {
    Nuppe *w = NUPPE(o);
    f32 h;
    f32 x;
    f32 y;
    f32 z;
    f32 r;

    w->x = vramCoursePointXZ(w->zone, w->x, w->z, &x, &z);
    r = h = vramCourseHeading(w->zone, w->x);
    if (mode == 0) {
        r = h - ang;
    } else if (mode == 1) {
        r += ang + 3.141592741f;
    } else if (mode == -1) {
        r = w->rot[1];
    }
    return r;
}

/* The distance from it to player i (0 if i is not 0-3 or not in play). */
f32 nuppePlDist(Obj *o, s32 i) {
    f32 dx;
    f32 dy;
    f32 x;
    f32 y;
    f32 z;
    Vec3f pos;

    if (i < 0 || i >= 4) {
        return 0.0f;
    }
    eneWorldPos(o, &pos);
    if (plIsInPlay(i)) {
        plGetModelPos(gGameWork.players[i], &x, &y, &z);
        dx = x - pos.x;
        dy = y - pos.y;
        return sqrtf(dx * dx + dy * dy + (z - pos.z) * (z - pos.z));
    }
    return 0.0f;
}

/* The nearest player in play (-1: none). */
s32 nuppeNearestPl(Obj *o) {
    s32 pad[3];
    f32 x;
    f32 y;
    f32 z;
    s32 i;
    f32 d;
    s32 near;
    Vec3f pos;
    f32 min;

    min = 65536.0f;
    near = -1;
    eneWorldPos(o, &pos);
    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plGetModelPos(gGameWork.players[i], &x, &y, &z);
            d = nuppePlDist(o, i);
            if (d < min) {
                min = d;
                near = i;
            }
        }
    }
    return near;
}

void nuppeNop3(void) {
}

/* "eneFallLimit": 1 once it is below -600 or above 1000. */
s32 nuppeFallLimit(Obj *o) {
    Nuppe *w = NUPPE(o);

    if (w->y < -600.0f) {
        PRINTF("eneFallLimit(-600):\n");
        return 1;
    }
    if (1000.0f < w->y) {
        PRINTF("eneFallLimit(+1000):\n");
        return 1;
    }
    return 0;
}

/* "eneTeritoryCheck": 1 if it is on another course than *zone (which it then takes, with its
 * place in *x) or farther than range from *x along it. */
s32 nuppeTeritoryCheck(Obj *o, f32 *x, u8 *zone, f32 range) {
    Nuppe *w = NUPPE(o);

    if (w->zone != *zone) {
        PRINTF("eneTeritoryCheck: new course\n");
        *x = w->x;
        *zone = w->zone;
        return 1;
    }
    if (fabs(w->x - *x) > range) {
        PRINTF("eneTeritoryCheck: out of range\n");
        return 1;
    }
    return 0;
}

/* "nuppe": the kind's proc. */
s32 nuppeProc(Obj *o, s16 msg, void *arg) {
    Nuppe *w = NUPPE(o);
    s32 pad;
    s32 r;

    switch (msg) {
        case 2:
            o->name = "nuppe";
            w->type = eneSetup(o, arg);
            w->life = 3;
            w->gravity = 30;
            w->homeX = w->x;
            w->homeZone = w->zone;
            if ((w->model = eneNewAnime(o, 0)) == NULL) {
                return -1;
            }
            frmMove(&w->model->f, gFrmLayer2);
            if ((w->shadow = shadowCreate()) == NULL) {
                return -1;
            }
            if ((w->atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, &w->model->f, 0, 0, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, &w->model->f, 0, 0, 0, 0, 0)) == NULL) {
                return -1;
            }
            break;
        case 0:
            if (nuppeFallLimit(o)) {
                return -1;
            }
            if (acollAtkWasHit(w->atk) == 1 && (Obj *) w->atk->hitPower != NULL) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, o);
            }
            /* Only type 0 has a main; r is unset for the others (the game's "lw v1" of its unset
             * home, "move v1,v0" after the call). The default's edge keeps r in a register at the
             * join. */
            switch (w->type) {
                case 0:
                    r = nuppeMain(o);
                    break;
                default:
                    break;
            }
            if (r != 0) {
                return r;
            }
            break;
        case 3:
            w->model->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, fileRamAddrAt(0xAF, (u32) nuppeDrawDl));
            if (w->flash == 0 && (w->anim == 4 || w->anim == 5)) {
                if (w->alpha + 6 >= 0x70) {
                    w->alpha = 0x70;
                } else {
                    w->alpha += 6;
                }
                gDPSetEnvColor(gGfxDlHead++, 0xFF, 0, 0, w->alpha);
            } else {
                gDPSetEnvColor(gGfxDlHead++, 0xFF, 0xFF, 0xFF, (w->flash & 2) ? 0xA0 : 0);
            }
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* A step: falls, moves, lands, animates, takes hits (dies at 0 life, state 7), goes home when
 * it strays (state 6), then runs its state. */
s32 nuppeMain(Obj *o) {
    Nuppe *w = NUPPE(o);
    s32 n;
    s32 land;
    s32 end;
    f32 rate;
    f32 acc; /* the step's change of speed, then of lean */
    f32 tilt; /* the new lean */

    eneFall(o, w->gravity * 0.01f * gRetracesPerFrame);
    eneMove((EneObj *) w);
    land = eneLand((EneObj *) w, 10.0f);
    eneSetModelOnCourse(o, &w->model->f);
    if (nuppeAnimTable[w->anim].loop == 1) {
        end = eneAnimLoop(o, w->model, w->animSpeed);
    } else {
        end = eneAnimPlayOnce(o, w->model, w->animSpeed);
    }
    if (w->shadow != NULL) {
        shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.3f, NULL);
    }
    if (w->hitSe - 1 <= 0) {
        w->hitSe = 0;
    } else {
        w->hitSe--;
    }
    if (acollDmgWasHit(w->dmg) == 1) {
        if (w->flash != 0 || w->anim == 4 || w->anim == 3) {
            if (w->hitSe == 0) {
                eneHitSoundHeavy(w->dmg);
                w->hitSe = 15;
            }
        } else {
            n = shimAtkKindCheck(w->dmg);
            if (n >= w->life) {
                w->life = 0;
                w->flash = 0;
                w->dmg->flags &= ~2;
                w->atk->flags &= ~2;
                nuppeSetAnim(o, 7, nuppeAnimTable);
            } else {
                w->life -= n;
                eneHitSoundHeavy(w->dmg);
                w->flash = 24 / gRetracesPerFrame;
                if (w->life == 1) {
                    if (fabs(w->vel[0]) <= 1.5) {
                        if (w->dir == 0) {
                            w->vel[0] = -1.5f;
                        } else {
                            w->vel[0] = 1.5f;
                        }
                    }
                    w->vel[0] = eneKnockbackSpeed(o, w->dmg, w->vel[0]);
                    if (w->anim != 5) {
                        nuppeSetAnim(o, 5, nuppeAnimTable);
                    }
                } else {
                    if (eneNearestPlWay(o) == 1) {
                        w->vel[0] = gRetracesPerFrame * -1.0;
                    }
                    if (eneNearestPlWay(o) == 4) {
                        w->vel[0] = gRetracesPerFrame * 1.0;
                    }
                    nuppeSetAnim(o, 3, nuppeAnimTable);
                }
            }
        }
    }
    if (w->flash != 0) {
        w->flash--;
        if (w->flash <= 0) {
            w->flash = 0;
        }
    }
    if (w->anim != 6 && nuppeTeritoryCheck(o, &w->homeX, &w->homeZone, 200.0f)) {
        w->vel[0] = w->prevVel = w->rot[0] = 0.0f;
        nuppeSetAnim(o, 6, nuppeAnimTable);
    }
    switch (w->anim) {
        case 0:
            w->atk->radius = 70;
            w->atk->ofs[1] = 100;
            w->atk->kind = 120;
            w->dmg->radius = 90;
            w->dmg->height = 300;
            w->model->scale[2] = 0.1f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            if (eneNearestPlWay(o) == 1) {
                w->dir = 0;
            } else {
                w->dir = 1;
            }
            w->rot[1] = nuppeCourseAngle(o, -1.919862151f, w->dir);
            nuppeSetAnim(o, 1, nuppeAnimTable);
            break;
        case 1:
            w->target = nuppeNearestPl(o);
            if (w->target >= 0 && nuppePlDist(o, w->target) < 70.0f) {
                nuppeSetAnim(o, 2, nuppeAnimTable);
            }
            break;
        case 2:
            if (land) {
                w->vel[1] = 0;
            }
            if (eneNearestPlWay(o) == 1) {
                w->dir = 0;
            } else {
                w->dir = 1;
            }
            eneTurnToward(o, nuppeCourseAngle(o, 0.3490658402f, w->dir), gRetracesPerFrame * 0.06981316954f);
            if (w->dir == 0) {
                w->vel[0] = gRetracesPerFrame * 0.2;
            } else if (w->dir == 1) {
                w->vel[0] = gRetracesPerFrame * -0.2;
            }
            break;
        case 3:
            w->vel[0] *= 0.8500000238f;
            if (land) {
                w->vel[1] = 0;
            }
            if (land && end) {
                w->wait = 0;
                w->vel[0] = 0;
                nuppeSetAnimEnd(o, 4, nuppeAnimTable);
            }
            break;
        case 4:
            if (land) {
                w->vel[1] = 0;
            }
            w->wait++;
            if (land && 60 / gRetracesPerFrame < w->wait) {
                w->dmg->flags |= 2;
                if (w->life == 2) {
                    nuppeSetAnim(o, 5, nuppeAnimTable);
                } else {
                    nuppeSetAnim(o, 2, nuppeAnimTable);
                }
            }
            break;
        case 5:
            if (land) {
                w->vel[1] = 0;
            } else if (!land && w->landed) {
                w->vel[1] = 5.0f;
                eneSetSd(o, 0x239);
            }
            if (eneNearestPlWay(o) == 1) {
                w->dir = 0;
            } else {
                w->dir = 1;
            }
            eneTurnToward(o, nuppeCourseAngle(o, 0.3490658402f, w->dir), gRetracesPerFrame * 0.05235987902f);
            if (w->dir == 0) { rate = gRetracesPerFrame; acc = rate * 0.05000000075f; } else { rate = gRetracesPerFrame; acc = -(rate * 0.05000000075f); }
            if (!(fabs(w->vel[0] + acc) > 1.200000048f * rate)) {
                w->vel[0] += acc;
            }
            if (w->vel[0] * (w->vel[0] - w->prevVel) >= 0.0f) {
                acc = 0.05235987902f;
            } else {
                acc = -0.06981316954f;
            }
            tilt = w->rot[0] + acc;
            if (tilt <= 0.2617993951f && tilt >= -0.5235987902f) {
                w->rot[0] = tilt;
            }
            w->prevVel = w->vel[0];
            rate = gRetracesPerFrame;
            w->animSpeed = (1.0 - fabs(w->vel[0]) / (1.200000048f * rate)) * (1.200000048f * rate) + rate;
            break;
        case 6:
            if (land) {
                w->vel[1] = 0;
            }
            if (w->x < w->homeX) {
                w->dir = 0;
            } else {
                w->dir = 1;
            }
            eneTurnToward(o, nuppeCourseAngle(o, 0.3490658402f, w->dir), gRetracesPerFrame * 0.06981316954f);
            if (w->dir == 0) {
                w->vel[0] = gRetracesPerFrame * 0.2;
            } else if (w->dir == 1) {
                w->vel[0] = gRetracesPerFrame * -0.2;
            }
            if (fabs(w->x - w->homeX) <= 0.2) {
                w->vel[0] = 0;
                nuppeSetAnim(o, 1, nuppeAnimTable);
            }
            break;
        case 7:
            shimEneDeathAct(o, w->dmg, (u32) nuppeHalf0Dl, (u32) nuppeHalf1Dl);
            eneReleaseSpawn(w->spawn);
            PRINTF("*** nuppe:dead ***\n");
            return -1;
    }
    w->landed = land;
    return 0;
}
