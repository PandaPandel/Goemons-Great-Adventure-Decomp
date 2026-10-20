#include "tlb.h"

/* The "gyuki" enemy kind (archive file 170): an ox demon (gyuki) that only comes out at night
 * (19:00 to 5:00). It drops in from above (state 1 waits for a player within 100), lands with a
 * rumble and two puffs, turns to the nearest player and walks at it, jumps when it walks off a
 * ledge, and takes five points of damage; at dawn it fades out (state 10). Its animations are
 * a table (gyukiAnimeTable) and its state is the animation's number. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

/* An animation of the gyuki's table: the state with the same number plays it. */
typedef struct GyukiAnim {
    /* 0x00 */ u32 anim;
    /* 0x04 */ f32 speed; /* at one retrace a frame */
    /* 0x08 */ u8 loop; /* 1: loops (eneAnimLoop), else plays once */
} GyukiAnim; /* size 0x0C */

extern u8 gyukiAnim[];
Gfx gyukiFadeDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetColorDither(G_CD_DISABLE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH),
    gsDPSetRenderMode(AA_EN | Z_CMP | Z_UPD | IM_RD | CLR_ON_CVG | CVG_DST_WRAP | ZMODE_INTER | FORCE_BL | G_RM_PASS, AA_EN | Z_CMP | Z_UPD | IM_RD | CLR_ON_CVG | CVG_DST_WRAP | ZMODE_INTER | FORCE_BL | GBL_c2(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)),
    gsDPSetCombineLERP(NOISE, 0, TEXEL0, 0, 0, 0, 0, ENVIRONMENT, COMBINED, 0, SHADE, 0, 0, 0, 0, COMBINED),
    gsDPSetAlphaCompare(G_AC_DITHER),
    gsDPPipeSync(),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetColorDither(G_CD_DISABLE),
    gsSPEndDisplayList(),
}; /* its fade-out's texture setup (file 0xAA's copy) */
GyukiAnim gyukiAnimeTable[] = {
    { (u32) gyukiAnim, 1.0f, 0 },
    { (u32) gyukiAnim, 1.0f, 1 },
    { 0x03001A40, 0.0f, 0 },
    { 0x03001A40, 2.0f, 0 },
    { (u32) gyukiAnim, 1.0f, 1 },
    { 0x03000904, 1.0f, 1 },
    { 0x03000DF8, 1.0f, 0 },
    { 0x0300140C, 2.0f, 0 },
    { 0x03000904, 3.0f, 1 },
    { 0x03000DF8, 1.0f, 0 },
    { 0x03000DF8, 1.0f, 0 },
    { 0x03000DF8, 0.0f, 0 },
    { 0x03000DF8, 0.0f, 0 },
    { 0x00000000, 0.0f, 0 },
};

/* A gyuki's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Gyuki {
    /* 0x00 */ EneObj e;
    /* 0x4C */ f32 animSpeed; /* its animation's step at this frame rate */
    /* 0x50 */ u8 anim; /* its animation in the table, and its state */
    /* 0x51 */ u8 type; /* its placement's type, from eneSetup: 0 moves */
    /* 0x52 */ u8 hp;
    /* 0x53 */ s8 gravity; /* its fall's, in hundredths */
    /* 0x54 */ s8 dir; /* 0: the nearest player is on its way 1, 1: on the other side */
    /* 0x55 */ s8 target; /* the nearest player (-1: none) */
    /* 0x56 */ s8 flash; /* steps left of the hit flash, its body box off */
    /* 0x57 */ u8 drawMode; /* 0 tinted red when flashing, 1 dying, 2 fading at dawn */
    /* 0x58 */ u8 alpha;
    /* 0x59 */ u8 landed; /* eneLand's result last step */
} Gyuki;

#define GYUKI(obj) ((Gyuki *) FX(obj))
#define MODEL(w) ((AnimeFrame *) (w)->e.model)

/* A frame destroyed if it was made. A macro: the game's code for the first one is scheduled as
 * one source line (as1 puts the frmDestroy address's addiu, not w's, in the branch's delay
 * slot), as in tlb_0159. */
#define FRM_DESTROY(f) if ((f) != NULL) { frmDestroy((Frame *) (f)); }

s32 gyukiFallLimit(Obj *o);
s32 gyukiMain(Obj *o);

/* Sets animation n of the table: its step, scaled to the frame rate, from its first frame. */
void gyukiSetAnime(Obj *o, u8 n, GyukiAnim *tab) {
    Gyuki *w = GYUKI(o);

    w->anim = n;
    w->animSpeed = tab[n].speed / gRetracesPerFrame;
    MODEL(w)->anim = tab[n].anim;
    MODEL(w)->frame = 0.0f;
}

void gyukiNop(void) {
}

void gyukiNop2(void) {
}

void gyukiNop3(void) {
}

/* Puts it on its course at its distance along it and returns the course's heading there:
 * mode 0 turned by -ofs, 1 by ofs + pi, -1 its own heading (rot y), else the course's. */
f32 gyukiCourseAngle(Obj *o, f32 ofs, s8 mode) {
    Gyuki *w = GYUKI(o);
    f32 a;
    f32 x;
    s32 pad;
    f32 z;
    s32 pad2;

    w->e.x = vramCoursePointXZ(w->e.zone, w->e.x, w->e.z, &x, &z);
    a = vramCourseHeading(w->e.zone, w->e.x);
    if (mode == 0) {
        a = a - ofs;
    } else if (mode == 1) {
        a += ofs + 3.141592741f;
    } else if (mode == -1) {
        a = w->e.rot[1];
    }
    return a;
}

/* The distance from it to player i (0 when i is out of range or not in play). */
f32 gyukiPlDist(Obj *o, s32 i) {
    s32 pad[2];
    f32 x;
    f32 y;
    f32 z;
    Vec3f e;

    if (i < 0 || i >= 4) {
        return 0.0f;
    }
    eneWorldPos(o, &e);
    if (plIsInPlay(i)) {
        plGetModelPos(gGameWork.players[i], &x, &y, &z);
        return sqrtf((x - e.x) * (x - e.x) + (y - e.y) * (y - e.y) + (z - e.z) * (z - e.z));
    }
    return 0.0f;
}

/* The nearest player in play (-1: none). */
s32 gyukiNearestPl(Obj *o) {
    s32 i;
    f32 min;
    f32 d;
    f32 x;
    f32 y;
    f32 z;
    s32 pad[2];
    s32 nearest;
    Vec3f e;

    min = 65536.0f;
    nearest = -1;
    eneWorldPos(o, &e);
    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plGetModelPos(gGameWork.players[i], &x, &y, &z);
            d = gyukiPlDist(o, i);
            if (d < min) {
                min = d;
                nearest = i;
            }
        }
    }
    return nearest;
}

/* Its kill's reward, to the player whose attack hit its box last. */
void gyukiKillReward(Obj *o, ACollFrame *box, s32 many) {
    s32 pl = -1;
    s32 i;

    if ((Obj *) box->hitPower != NULL) {
        for (i = 0; i < 4; i++) {
            if ((Obj *) box->hitPower == gGameWork.players[i]) {
                pl = PL(gGameWork.players[i])->index;
            }
        }
    }
    eneKillReward(o, pl, many);
}

/* 1 when it has fallen below -600 or risen above 1000. */
s32 gyukiFallLimit(Obj *o) {
    Gyuki *w = GYUKI(o);

    if (w->e.y < -600.0f) {
        PRINTF("eneFallLimit(-600):\n");
        return 1;
    }
    if (w->e.y > 1000.0f) {
        PRINTF("eneFallLimit(+1000):\n");
        return 1;
    }
    return 0;
}

s32 gyukiProc(Obj *o, s16 msg, void *arg) {
    Gyuki *w = GYUKI(o);
    s32 hour = gGameWork.time / 0x10000;
    s32 pad;

    switch (msg) {
        case 2:
            if (hour < 19 && hour >= 5) {
                return -1;
            }
            o->name = "gyuki";
            w->type = eneSetup(o, arg);
            w->gravity = 0;
            w->alpha = 255;
            if ((w->e.model = (Frame *) eneAnimeModelMake(o, 0, NULL, 1)) == NULL) {
                return -1;
            }
            frmMove(w->e.model, gFrmLayer2);
            if ((w->e.shadow = (Obj *) shadowCreate()) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 0, 0, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 0, 0, 0, 0, 0)) == NULL) {
                return -1;
            }
            break;
        case 0: {
            /* The game reads r unset when the type is not 0. Written as the ternary that keeps
             * it: IDO loads its home (lw v1) before the test and moves the result into v1; an
             * "if" without the else kept r in memory (sw/lw around the call). */
            s32 r;

            if (gyukiFallLimit(o)) {
                return -1;
            }
            if (acollAtkWasHit(w->e.atk) == 1 && (Obj *) w->e.atk->hitPower != NULL) {
                tskSendMsg((Obj *) w->e.atk->hitPower, 7, o);
            }
            r = (w->type == 0) ? gyukiMain(o) : r;
            if (r != 0) {
                return r;
            }
            break;
        }
        case 3:
            if (w->drawMode == 1) {
                MODEL(w)->dl = gGfxDlHead;
                gSPDisplayList(gGfxDlHead++, fileRamAddrAt(0xAA, (u32) gyukiFadeDl));
                gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
                gSPEndDisplayList(gGfxDlHead++);
            } else if (w->drawMode == 2) {
                MODEL(w)->dl = gGfxDlHead;
                gDPPipeSync(gGfxDlHead++);
                gDPSetCombineLERP(gGfxDlHead++, ENVIRONMENT, TEXEL0, ENV_ALPHA, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0,
                                  ENVIRONMENT, TEXEL0, ENV_ALPHA, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0);
                gDPSetEnvColor(gGfxDlHead++, 255, 255, 255, w->alpha);
                gSPEndDisplayList(gGfxDlHead++);
            } else {
                MODEL(w)->dl = gGfxDlHead;
                gSPDisplayList(gGfxDlHead++, gDrawSetupEnvTintTexShadeDl);
                gDPSetEnvColor(gGfxDlHead++, 255, 0, 0, (w->flash & 2) ? 160 : 0);
                gSPEndDisplayList(gGfxDlHead++);
            }
            break;
        case 1:
            FRM_DESTROY(w->e.model);
            FRM_DESTROY(w->e.atk);
            FRM_DESTROY(w->e.dmg);
            FRM_DESTROY(w->e.shadow);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* A step: falls, moves, lands, animates, takes hits (dies at 0 hp, state 9), fades out at dawn
 * (state 10), then runs its state. The states' zero speeds are int 0 and the others 0.0f: the
 * two are separate constants to uopt, and only the states' alone are held in f0 (made afresh,
 * "mtc1 zero,f0", at each); with the hit spark's 0.0f among them none was. */
s32 gyukiMain(Obj *o) {
    Gyuki *w = GYUKI(o);
    s32 hour;
    s32 land;
    s32 end;
    s32 n;
    s32 dir;
    s32 pad;
    f32 p;
    f32 h;
    f32 s;
    s32 zone;

    eneFall(o, w->gravity * 0.01f * gRetracesPerFrame);
    eneMove(&w->e);
    land = eneLand(&w->e, 30.0f);
    eneSetModelOnCourse(o, w->e.model);
    if (gyukiAnimeTable[w->anim].loop == 1) {
        end = eneAnimLoop(o, MODEL(w), w->animSpeed);
    } else {
        end = eneAnimPlayOnce(o, MODEL(w), w->animSpeed);
    }
    if (w->e.shadow != NULL) {
        shadowPut((MPolFrame *) w->e.shadow, MODEL(w)->pos[0], MODEL(w)->pos[1], MODEL(w)->pos[2], 4.0f, NULL);
    }
    if (acollDmgWasHit(w->e.dmg) == 1) {
        n = shimAtkKindCheck(w->e.dmg);
        if (w->e.dmg->hitKind == 31) {
            n = 5;
        }
        eneMakeHitSpark(0, w->e.dmg->push[0], w->e.dmg->push[1], w->e.dmg->push[2], 3,
                        gyukiCourseAngle(o, 0.0f, w->dir));
        eneHitSoundHeavy(w->e.dmg);
        if (n >= w->hp) {
            w->hp = 0;
            w->e.dmg->flags &= ~2;
            w->e.atk->flags &= ~2;
            w->drawMode = 1;
            w->e.vel[0] = 0.0f;
            eneSetSd(o, 0x183);
            gyukiSetAnime(o, 9, gyukiAnimeTable);
        } else {
            w->hp = w->hp - n;
            w->flash = 24 / gRetracesPerFrame;
            if (eneNearestPlWay(o) == 1) {
                w->e.vel[0] = gRetracesPerFrame * -2.0;
            }
            if (eneNearestPlWay(o) == 4) {
                w->e.vel[0] = gRetracesPerFrame * 2.0;
            }
            gyukiSetAnime(o, 6, gyukiAnimeTable);
        }
    }
    if (w->flash != 0) {
        w->flash--;
        if (w->flash <= 0) {
            w->flash = 0;
            w->e.dmg->flags |= 2;
        } else {
            w->e.dmg->flags &= ~2;
        }
    }
    hour = gGameWork.time / 0x10000;
    if (hour < 19 && hour >= 5 && w->drawMode != 2) {
        w->hp = 0;
        w->e.dmg->flags &= ~2;
        w->e.atk->flags &= ~2;
        w->drawMode = 2;
        w->e.vel[0] = 0.0f;
        frmMove(w->e.model, gFrmLayer8);
        eneSetSd(o, 0x183);
        gyukiSetAnime(o, 10, gyukiAnimeTable);
    }
    switch (w->anim) {
        case 0:
            w->e.atk->radius = 200;
            w->e.atk->ofs[1] = 200;
            w->e.atk->ofs[2] = 100;
            w->e.atk->kind = 120;
            w->e.dmg->radius = 240;
            w->e.dmg->height = 350;
            w->e.dmg->ofs[2] = 80;
            w->e.dmg->flags &= ~2;
            w->e.dmg->push = (f32 *) w->e.halves;
            w->hp = 5;
            w->flash = 0;
            MODEL(w)->scale[2] = 0.1f;
            MODEL(w)->scale[0] = MODEL(w)->scale[1] = MODEL(w)->scale[2];
            if (eneNearestPlWay(o) == 1) {
                w->dir = 0;
            } else {
                w->dir = 1;
            }
            w->e.rot[1] = gyukiCourseAngle(o, 0.3490658402f, w->dir);
            gyukiSetAnime(o, 1, gyukiAnimeTable);
            break;
        case 1:
            w->e.vel[1] = 0;
            w->target = gyukiNearestPl(o);
            if (w->target >= 0 && gyukiPlDist(o, w->target) < 100.0f) {
                w->gravity = 30;
                gyukiSetAnime(o, 2, gyukiAnimeTable);
            }
            break;
        case 2:
            if (land) {
                w->e.vel[2] = w->e.vel[0] = w->e.vel[1] = 0;
                eneSetSd(o, 0x2EF);
                itexPuffGrey(MODEL(w)->pos[0] + 10.0f, MODEL(w)->pos[1] + 5.0f, MODEL(w)->pos[2], 0.1f);
                itexPuffGrey(MODEL(w)->pos[0] - 10.0f, MODEL(w)->pos[1] + 5.0f, MODEL(w)->pos[2], 0.1f);
                fadeRumbleAll(0xBBBBBBBB, 16);
                gyukiSetAnime(o, 3, gyukiAnimeTable);
            }
            break;
        case 3:
            w->e.vel[0] *= 0.75;
            if (land) {
                w->e.vel[2] = w->e.vel[0] = w->e.vel[1] = 0;
            }
            if (end) {
                w->e.dmg->flags |= 2;
                gyukiSetAnime(o, 4, gyukiAnimeTable);
            }
            break;
        case 4:
            if (land) {
                w->e.vel[1] = 0;
            }
            gyukiSetAnime(o, 5, gyukiAnimeTable);
            break;
        case 5:
            if (eneNearestPlWay(o) == 1) {
                dir = 0;
            } else {
                dir = 1;
            }
            if (land) {
                w->e.vel[1] = 0;
            } else if (!land && w->landed) {
                if (eneNearestPlWay(o) == 1) {
                    w->e.vel[0] = gRetracesPerFrame * 1.0;
                }
                if (eneNearestPlWay(o) == 4) {
                    w->e.vel[0] = gRetracesPerFrame * -1.0;
                }
                w->e.vel[1] = 4.5f;
                gyukiSetAnime(o, 2, gyukiAnimeTable);
                break;
            }
            if (dir != w->dir) {
                w->dir = dir;
                gyukiSetAnime(o, 8, gyukiAnimeTable);
                break;
            }
            w->dir = dir;
            eneTurnToward(o, gyukiCourseAngle(o, 0.5235987902f, w->dir), gRetracesPerFrame * 0.03490658477f);
            if (w->dir == 0) {
                w->e.vel[0] = gRetracesPerFrame * 0.5;
            } else if (w->dir == 1) {
                w->e.vel[0] = gRetracesPerFrame * -0.5;
            }
            break;
        case 6:
            w->e.vel[0] *= 0.9;
            if (land) {
                w->e.vel[1] = 0;
            }
            if (land && end) {
                gyukiSetAnime(o, 7, gyukiAnimeTable);
            }
            break;
        case 7:
            if (end) {
                if (eneNearestPlWay(o) == 1) {
                    w->e.vel[0] = gRetracesPerFrame * 1.5;
                }
                if (eneNearestPlWay(o) == 4) {
                    w->e.vel[0] = gRetracesPerFrame * -1.5;
                }
                w->e.vel[1] = gRetracesPerFrame * 3.0f;
                eneSetSd(o, 0x23A);
                gyukiSetAnime(o, 2, gyukiAnimeTable);
            }
            break;
        case 8:
            plAskPos(gGameWork.players[w->target], &p, &h, &s, &zone);
            if (eneNearestPlWay(o) == 1) {
                w->dir = 0;
            } else {
                w->dir = 1;
            }
            if (eneTurnToward(o, gyukiCourseAngle(o, 0.9599310756f, w->dir),
                              gRetracesPerFrame * 0.03490658477f)) {
                gyukiSetAnime(o, 4, gyukiAnimeTable);
            }
            break;
        case 9:
            if (w->alpha > 8) {
                w->alpha -= 8;
            } else if (end) {
                w->alpha = 0;
                gyukiSetAnime(o, 11, gyukiAnimeTable);
            }
            break;
        case 10:
            if (w->alpha > 8) {
                w->alpha -= 8;
            } else if (end) {
                w->alpha = 0;
                gyukiSetAnime(o, 12, gyukiAnimeTable);
            }
            break;
        case 11:
            eneMakeEx(NULL, ENE_BIG_BOMB, 0, MODEL(w)->pos[0], MODEL(w)->pos[1] + 10.0f, MODEL(w)->pos[2], 2,
                      (u32 (*)(void *, void *, void *)) 10);
            gyukiKillReward(o, w->e.dmg, 1);
            effectBurst(MODEL(w)->pos[0], MODEL(w)->pos[1] + 5.0f, MODEL(w)->pos[2]);
            gGameWork.kills++;
            PRINTF("gyuki: %d 人斬り\n", gGameWork.kills);
            eneSetSd2(o, 0x229, 0x22A, 0);
            eneReleaseSpawn(w->e.spawn);
            return -1;
        case 12:
            eneReleaseSpawn(w->e.spawn);
            return -1;
    }
    w->landed = land;
    return 0;
}
