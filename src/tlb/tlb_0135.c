#include "tlb.h"

/* File 135: the karasu tengu (crow goblin) hunt. "Ktengu", the karasu tengu themselves; "kara_tama",
 * their spirit shot (kiaidan); "k_lady", the lady who asks the player to shoot them down;
 * "d_Karasu" ("demoKarasutengu_proc"), the hunt's manager; "ka_disp" ("karasuDisp_proc"), the count
 * of them on the screen (a cross, "batten", on each one shot down). File 126's tsuchikorobi hunt
 * is the same shape. */

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* A karasu tengu's work, at its task + 0x20: an enemy's (EneObj) up to 0x40. */
typedef struct KarasuW {
    /* 0x00 */ u8 state;
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
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ MPolFrame *shadow;
    /* 0x40 */ u8 target; /* the nearest player in play */
    /* 0x41 */ u8 step; /* the step in the state */
    /* 0x42 */ u8 unused42; /* written 0 at its start, never read */
    /* 0x43 */ u8 attack; /* the last attack: 2 the spirit shot, 3 the charge */
    /* 0x44 */ u8 life; /* hits it takes (5) */
    /* 0x45 */ u8 flapped; /* its wing sound played this flap */
    /* 0x46 */ s16 timer; /* ticks in the state */
    /* 0x48 */ f32 goal[3]; /* where the target player stood (plAskPos) */
    /* 0x54 */ f32 homeX; /* where it was placed: it keeps within 150 of it */
    /* 0x58 */ u8 id; /* which of the six (its bit in gGameWork.karasuShot) */
} KarasuW;

#define KARASU(obj) ((KarasuW *) FX(obj))

/* A spirit shot's work ("kara_tama"), at its task + 0x20: an enemy's and its own. */
typedef struct KarasuTamaW {
    /* 0x00 */ EneObj e;
    /* 0x4C */ u8 target; /* the player it flies at */
    /* 0x50 */ s32 alpha;
} KarasuTamaW;

#define KARASUTAMA(obj) ((KarasuTamaW *) FX(obj))

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them (file 84's form). */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

/* Degrees to radians, as the karasu tengu's tilt has it (a double). */
#define DEG2RAD 0.0174532930038800785

#define ABS(x) ((x) > 0 ? (x) : -(x))
/* How far a is from b, either way. */
#define DIST(a, b) ((b) < (a) ? (a) - (b) : -((a) - (b)))
#define SQ(x) ((x) * (x))

/* libultra's, through their addresses (code at 0x08000000 cannot jal them). */
#define sinf (*sinf)
#define cosf (*cosf)

/* The lady's work ("k_lady"), at its task + 0x20: a demo talker's (PersonObj) up to 0x44. */
typedef struct KarasuLadyW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 pad09;
    /* 0x0A */ u8 hit;
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PersonData *data;
    /* 0x38 */ ACollFrame *box; /* her talk box */
    /* 0x3C */ u8 flags3C;
    /* 0x3D */ u8 count;
    /* 0x3E */ u8 pad3E[2];
    /* 0x40 */ Obj *talker; /* the player talking to her */
    /* 0x44 */ MPolFrame *shadow;
    /* 0x48 */ Camera *cam;
    /* 0x4C */ Obj *pass; /* the pass (kind 0x1C3), made between her and the player */
    /* 0x50 */ s32 timer; /* ticks in the state */
    /* 0x54 */ u8 flags; /* 1: the sky's tunes change 300 ticks after the pass */
    /* 0x55 */ u8 tuneTimer;
} KarasuLadyW;

#define KARASULADY(obj) ((KarasuLadyW *) FX(obj))

/* acollAtkAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy,
                                     s16 oz, s32 a);

/* The hunt's manager's work ("demoKarasutengu_proc"), at its task + 0x20. */
typedef struct DemoKarasuW {
    /* 0x00 */ u8 state;
    /* 0x04 */ s32 timer; /* ticks in the state */
    /* 0x08 */ Obj *tengu; /* the karasu tengu (kind 0x1C2) */
    /* 0x0C */ Obj *disp; /* the count ("ka_disp") */
    /* 0x10 */ Camera *cam;
    /* 0x14 */ u8 pad14[4];
    /* 0x18 */ s8 partake; /* gGameWork.partake while scenario 0x298 runs */
} DemoKarasuW;

#define DEMOKARASU(obj) ((DemoKarasuW *) FX(obj))

/* The count's work ("karasuDisp_proc"), at its task + 0x20. */
typedef struct KarasuDispW {
    /* 0x00 */ CamView *cam; /* the count's camera (an orthographic one) */
    /* 0x04 */ AnimeFrame *faces[6]; /* the six karasu tengu */
    /* 0x1C */ PolFrame *battens[6]; /* a cross on each one shot down */
    /* 0x34 */ u8 n; /* the crosses made */
    /* 0x35 */ u8 pad35[3];
    /* 0x38 */ s32 timer;
} KarasuDispW;

#define KARASUDISP(obj) ((KarasuDispW *) FX(obj))

/* fadeSegToRam as this file calls it (file 126's form): the display list's address passed as a
 * pointer. */
typedef Gfx *(*SegToRamFunc)(Gfx *addr, u32 file);

extern u32 karasuTenguAnims[]; /* the karasu tengu's five animations */
extern Gfx karasuDispDl[]; /* the count's faces and crosses: their render modes */
extern Gfx karasuBattenDl[];
extern u8 karasuTenguAnim0[];
extern u8 karasuTamaDl[];
extern u32 karasuLadyAnims[]; /* the lady's animations: { 0x0300042C, karasuLadyAnim1, 0 } */
extern Unk88282 gSaveCopyPlayers[4]; /* the players' entries for a continue (stage.c) */

void karasuPassToPlayer(Obj *o, Obj *player);
extern Obj *karasuDispTask; /* the count's task ("ka_disp") */

/* The file's .data, in address order (its externs above stay as declarations). */
extern u8 karasuLadyAnim1[];
extern u8 karasuTenguHalf0Dl[]; /* the karasu tengu's fall (its death's two display lists) */
extern u8 karasuTenguHalf1Dl[];
Obj *karasuDispTask = (Obj *) 0x12345670; /* the count's task ("ka_disp") */
u32 karasuUnusedDl[] = { 0x00000000, 0xE7000000, 0x00000000, 0xD9FFFFFF, 0x00220404, 0xDF000000, 0x00000000 };
u32 karasuTenguAnims[] = { (u32) karasuTenguAnim0, (u32) karasuTenguAnim0, 0x03000E5C, 0x030015DC, 0x03001AEC }; /* the karasu tengu's five animations */
u32 karasuLadyAnims[] = { 0x0300042C, (u32) karasuLadyAnim1, 0x00000000 }; /* the lady's animations: { 0x0300042C, karasuLadyAnim1, 0 } */
Gfx karasuDispDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(CVG_DST_CLAMP | ZMODE_OPA | CVG_X_ALPHA | FORCE_BL | G_RM_PASS, CVG_DST_CLAMP | ZMODE_OPA | CVG_X_ALPHA | FORCE_BL | GBL_c2(G_BL_CLR_IN, G_BL_0, G_BL_CLR_IN, G_BL_1)),
    gsDPSetCombineLERP(PRIMITIVE, 0, SHADE, 0, 0, 0, 0, PRIMITIVE, PRIMITIVE, 0, SHADE, 0, 0, 0, 0, PRIMITIVE),
    gsDPPipeSync(),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsSPEndDisplayList(),
}; /* the count's faces and crosses: their render modes */

void karasuFadeOut(f32 speed);
void karasuTenguSetAnim(Obj *o, s32 n, s32 last);
u8 karasuIsKilled(u8 id);
void karasuSetKilled(u8 id);
void karasuDeathAct(Obj *o, ACollFrame *box, u32 dl, u32 dl2);
void karasuClearKilled(void);
Obj *karasuDispMake(Obj *obj);
void karasuBattenAdd(Obj *o);

/* The screen fades to black at the given speed. The first call is on the brace's line: IDO
 * schedules by line, and only there does "sw ra" come after the call's address, as the game has
 * it (file 126's tsuchiFadeOut and 127's beatFadeOut, the same code). */
void karasuFadeOut(f32 speed) { fadeSetSpeed(speed);
    fadeSetColor(0, 0, 0);
    fadeSetTarget(255.0f);
}

typedef struct PassW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *pol;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *glow;
    /* 0x38 */ u8 rgb[3];
    /* 0x3B */ u8 pad3B;
    /* 0x3C */ s16 alpha;
    /* 0x3E */ u8 pad3E[2];
    /* 0x40 */ s32 timer;
    /* 0x44 */ Obj *player; /* the one it goes to */
} PassW;

/* The pass goes to the player (state 3). */
void karasuPassToPlayer(Obj *o, Obj *player) {
    PassW *w = (PassW *) FX(o);

    w->state = 3;
    w->player = player;
    w->timer = 0;
}

void karasuEventTegataSdSet(void) {
    DPRINTF("&&&& eventTegataSdSet  pl D がないn");
}

void karasuEventClearSd(void) {
    DPRINTF("&&&& SD_G_CLEAR1n");
    DPRINTF("&&&& SD_E_CLEAR1n");
    DPRINTF("&&&& SD_S_CLEAR1n");
    DPRINTF("&&&& SD_Y_CLEAR1n");
}

/* "Ktengu": a karasu tengu (id 0-5 from its placement). Made while the hunt is on and it has not
 * been shot down. It hovers over the nearest player, keeping within 150 of where it was placed,
 * and every 40 ticks attacks by turns: a spirit shot (state 2: kind 0x193 toward the player) or a
 * charge (state 3: it dives at where the player stood). Hit (state 4) it is knocked back and
 * flashes; the fifth hit shoots it down, crosses it out on the count and marks it. It tilts with
 * its moves (the second switch).
 * Matched 2026-10-16; it was parked for which of two as1 schedule tie-breaks ran first (the
 * place reload against s1's address at 347, the 60.0f load against the charge test's two place
 * loads at 1185); reading as1's last tie-break (the lower source .loc line goes first) and
 * writing both tests through file 97's DIST macro on one line (2026-10-17) closed it. */
s32 karasuTenguProc(Obj *o, s16 msg, void *arg) {
    f32 px;
    f32 py;
    f32 pz;
    KarasuW *w = KARASU(o);
    s32 i;
    f32 place;
    f32 height;
    f32 side;
    f32 mx;
    f32 dx;
    f32 mz;
    f32 dz;
    f32 d;
    f32 lim;
    f32 m;
    f32 tx;
    f32 ad;
    f32 tz;
    f32 dist;
    s32 pad5[2];
    s32 zone;
    s32 pad6[2];
    f64 a;
    f64 b;
    s32 pad7[3];
    f32 qx;
    f32 qy;
    f32 qz;
    f32 place2;
    f32 height2;
    f32 side2;
    s32 best;
    s32 pad8;

    switch (msg) {
        case 2:
            o->name = "Ktengu";
            if (!swWorkTest(0x17)) {
                PRINTF(">>>>   KARASU_MISSION フラグがたっていない\n");
                return -1;
            }
            if (swWorkTest(0xAE)) {
                PRINTF(">>>>    手形もらってるで\n");
                return -1;
            }
            w->id = eneSetup(o, arg);
            PRINTF("カラステング発生 id=[%d]\n", w->id);
            if (karasuIsKilled(w->id)) {
                PRINTF("既に殺されている id=[%d]\n", w->id);
                return -1;
            }
            w->model = eneNewAnime(o, karasuTenguAnims[0]);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[2] = 0.6499999762f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, &w->model->f, 9, 20, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x78;
            w->dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, &w->model->f, 13, 35, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->shadow = shadowCreate();
            w->unused42 = 0;
            w->target = eneNearestPl(o);
            w->state = 0;
            w->timer = 0;
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            w->step = 0;
            w->attack = 2;
            w->life = 5;
            w->goal[0] = w->x;
            w->homeX = w->x;
            w->goal[1] = w->y;
            w->goal[2] = w->z;
            PRINTF(">>>>  からす天狗(id=[%d])セットＯＫ\n", w->id);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 3:
            w->model->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 0, 0, 0, 0);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 0);
            gDPSetCombineMode(gGfxDlHead++, G_CC_MODULATEIDECALA, G_CC_MODULATEIDECALA);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            eneStepHookPre(o);
            w->timer++;
            dist = 1000000.0f;
            for (i = 0; i < 4; i++) {
                if (plIsInPlay(i)) {
                    plGetModelPos(gGameWork.players[i], &px, &py, &pz);
                    d = sqrtf(SQ(pz - w->model->pos[2]) + SQ(px - w->model->pos[0]));
                    if (d < dist) {
                        dist = d;
                        tx = px;
                        tz = pz;
                        w->target = i;
                        plAskPos(gGameWork.players[w->target], &place, &height, &side, &zone);
                    }
                }
            }
            if (acollDmgWasHit(w->dmg) == 1) {
                PRINTF("ダメージを受けました 1\n");
                w->step = 0;
                w->state = 4;
                w->timer = 0;
                w->dmg->flags &= ~2;
            }
            if (w->homeX + 150.0f < w->x) {
                PRINTF("[X+]stx %d\n", w->state);
                w->x = w->homeX + 150.0f;
            }
            if (w->x < w->homeX - 150.0f) {
                PRINTF("[X-]\n");
                w->x = w->homeX - 150.0f;
            }
            if (w->x < 0.0f) {
                w->x = 1.0f;
            }
            switch (w->state) {
                case 0:
                    eneTurnToPoint(o, tx, tz, 6.0f);
                    if (gGameFrameCount % 30 == 0) {
                        eneSetSd(o, 0x515);
                    }
                    if (DIST(w->x, place) < 15.0f) {
                        if (eneNearestPlWay(o) == 4) {
                            w->vel[0] += 0.07;
                            if (10.0f < w->vel[0]) {
                                w->vel[0] = 10.0f;
                            }
                        } else {
                            w->vel[0] -= 0.07;
                            if (w->vel[0] < -10.0f) {
                                w->vel[0] = -10.0f;
                            }
                        }
                    } else {
                        w->vel[0] *= 0.97;
                    }
                    if (place < w->x) {
                        ad = w->x - place;
                    } else {
                        ad = -(w->x - place);
                    }
                    if (55.0f < ad) {
                        if (eneNearestPlWay(o) == 4) {
                            w->vel[0] -= 0.07;
                            if (w->vel[0] < -10.0f) {
                                w->vel[0] = -10.0f;
                            }
                        } else {
                            w->vel[0] += 0.07;
                            if (10.0f < w->vel[0]) {
                                w->vel[0] = 10.0f;
                            }
                        }
                    } else {
                        w->vel[0] *= 0.97;
                    }
                    if (w->y < height + 15.0f - 6.0f) {
                        w->vel[1] += 0.05;
                        if (w->vel[1] > 0.5) {
                            w->vel[1] = 0.5f;
                        }
                    } else if (height + 15.0f + 6.0f < w->y) {
                        w->vel[1] -= 0.05;
                        if (w->vel[1] < -0.5) {
                            w->vel[1] = -0.5f;
                        }
                    } else {
                        w->vel[1] *= 0.93;
                    }
                    if (dist < 12.0f) {
                        if (eneNearestPlWay(o) == 4) {
                            w->vel[2] += 0.3;
                        } else {
                            w->vel[2] -= 0.3;
                        }
                        if (4.0f < w->vel[2]) {
                            w->vel[2] = 4.0f;
                        }
                        if (w->vel[2] < -4.0f) {
                            w->vel[2] = -4.0f;
                        }
                        eneAnimLoop(o, w->model, 0.3f);
                    } else {
                        w->vel[2] *= 0.95;
                    }
                    w->z *= 0.97;
                    w->goal[0] = place;
                    w->goal[1] = height;
                    w->goal[2] = side;
                    eneAnimLoop(o, w->model, 0.8f);
                    if (w->timer == 40) {
                        if (place < w->homeX + 150.0f && w->homeX - 150.0f < place) {
                            if (w->attack == 3) {
                                PRINTF("<karasu>気合弾\n");
                                w->state = 2;
                                w->attack = 2;
                            } else {
                                PRINTF("<karasu>突っ込み\n");
                                w->state = 3;
                                w->attack = 3;
                            }
                            w->timer = 0;
                            w->step = 0;
                        }
                    }
                    break;
                case 2:
                    switch (w->step) {
                        case 0:
                            eneTurnToPoint(o, tx, tz, 6.0f);
                            eneAnimLoop(o, w->model, 0.4f);
                            w->z *= 0.90; /* FAKE: 0.9, spelled apart (IDO_NOTES 365) */
                            if (place < w->x) {
                                ad = w->x - place;
                            } else {
                                ad = -(w->x - place);
                            }
                            if (ad < 30.0f) {
                                if (eneNearestPlWay(o) == 4) {
                                    w->vel[0] += 0.07;
                                    if (10.0f < w->vel[0]) {
                                        w->vel[0] = 10.0f;
                                    }
                                } else {
                                    w->vel[0] -= 0.07;
                                    if (w->vel[0] < -10.0f) {
                                        w->vel[0] = -10.0f;
                                    }
                                }
                            } else {
                                w->vel[0] *= 0.97;
                            }
                            w->vel[1] = 0.0f;
                            if (w->y < height + 35.0f - 6.0f) {
                                w->vel[1] = 0.200000003f;
                            }
                            if (height + 35.0f + 6.0f < w->y) {
                                w->vel[1] = -0.200000003f;
                            }
                            if (w->timer > 30) {
                                w->step++;
                                (*karasuTenguSetAnim)(o, 2, 0);
                                w->goal[0] = place;
                                w->goal[1] = height;
                                w->goal[2] = side;
                            }
                            w->z *= 0.95;
                            break;
                        case 1:
                            eneTurnToPoint(o, tx, tz, 6.0f);
                            w->model->frame += 0.7;
                            w->vel[0] *= 0.95;
                            w->vel[1] *= 0.95;
                            if (polAnimeLastFrame(w->model) < w->model->frame) {
                                if (place < w->x) {
                                    w->vel[0] = 1.0f;
                                } else {
                                    w->vel[0] = -1.0f;
                                }
                                w->vel[1] = 0.400000006f;
                                PRINTF("発射します\n");
                                w->model->frame = polAnimeLastFrame(w->model);
                                w->step++;
                                eneMakeEx(NULL, ENE_KARASU_TAMA, w->zone, w->x, w->y + 11.5, w->z, 2,
                                          (u32 (*)(void *, void *, void *)) w->target);
                                PRINTF("気合弾打ちました\n");
                            }
                            break;
                        case 2:
                            eneTurnToPoint(o, tx, tz, 4.0f);
                            w->model->frame -= 0.06;
                            if (w->model->frame < 0.0f) {
                                w->model->frame = 0.0f;
                            }
                            w->vel[0] *= 0.96;
                            w->vel[1] *= 0.96;
                            if (w->vel[0] > 0) {
                                ad = w->vel[0];
                            } else {
                                ad = -w->vel[0];
                            }
                            if (ad < 0.1) {
                                PRINTF("気合弾おわります\n");
                                (*karasuTenguSetAnim)(o, 0, 0);
                                w->state = 0;
                                w->step = 0;
                                w->timer = 0;
                            }
                            break;
                    }
                    break;
                case 3:
                    if (w->homeX + 150.0f - 30.0f < place && place < w->homeX - 150.0f + 30.0f) {
                        w->step = 2;
                    }
                    if (w->homeX + 150.0f - 20.0f < w->x && w->x < w->homeX - 150.0f + 20.0f) {
                        w->step = 2;
                    }
                    if (zone != w->zone) {
                        w->step = 2;
                    }
                    switch (w->step) {
                        case 0:
                            eneTurnToPoint(o, tx, tz, 3.0f);
                            eneAnimLoop(o, w->model, 0.4f);
                            w->z *= 0.90; /* FAKE: 0.9, spelled apart (IDO_NOTES 365) */
                            if (place < w->x) {
                                ad = w->x - place;
                            } else {
                                ad = -(w->x - place);
                            }
                            if (ad < 60.0f) {
                                if (eneNearestPlWay(o) == 4) {
                                    w->vel[0] += 0.07;
                                    if (10.0f < w->vel[0]) {
                                        w->vel[0] = 10.0f;
                                    }
                                } else {
                                    w->vel[0] -= 0.07;
                                    if (w->vel[0] < -10.0f) {
                                        w->vel[0] = -10.0f;
                                    }
                                }
                            } else {
                                w->vel[0] *= 0.97;
                            }
                            if (place < w->x) {
                                ad = w->x - place;
                            } else {
                                ad = -(w->x - place);
                            }
                            if (100.0f < ad) {
                                if (eneNearestPlWay(o) == 4) {
                                    w->vel[0] -= 0.07;
                                    if (w->vel[0] < -10.0f) {
                                        w->vel[0] = -10.0f;
                                    }
                                } else {
                                    w->vel[0] += 0.07;
                                    if (10.0f < w->vel[0]) {
                                        w->vel[0] = 10.0f;
                                    }
                                }
                            } else {
                                w->vel[0] *= 0.97;
                            }
                            w->vel[1] = 0.0f;
                            if (w->y < height + 32.0f - 6.0f) {
                                w->vel[1] = 0.200000003f;
                            }
                            if (height + 32.0f + 6.0f < w->y) {
                                w->vel[1] = -0.200000003f;
                            }
                            if (place < w->x) {
                                ad = w->x - place;
                            } else {
                                /* FAKE: one source line (as1 orders hoisted instructions by .loc line, see the note) */
                                ad = -(w->x - place); } if (ad < 100.0f) { if (place < w->x) { m = w->x - place; } else { m = -(w->x - place); } if (60.0f < m) {
                                    PRINTF("加速します\n");
                                    w->step++;
                                    (*karasuTenguSetAnim)(o, 3, 0);
                                    w->goal[0] = place;
                                    w->goal[1] = height;
                                    w->goal[2] = side;
                                    w->dmg->flags &= ~2;
                                }
                            }
                            w->z *= 0.95;
                            break;
                        case 1:
                            w->atk->ofs[0] = -sinf(w->model->rot[1]) * 50.0f;
                            w->atk->ofs[2] = -cosf(w->model->rot[1]) * 50.0f;
                            if (w->x < w->goal[0]) {
                                w->vel[0] += 0.4;
                            } else {
                                w->vel[0] -= 0.4;
                            }
                            if (w->y < w->goal[1]) {
                                w->vel[1] += 0.2;
                            } else {
                                w->vel[1] -= 0.2;
                            }
                            if (20.0f < w->vel[0]) {
                                w->vel[0] = 20.0f;
                            }
                            if (w->vel[0] < -20.0f) {
                                w->vel[0] = -20.0f;
                            }
                            if (3.0f < w->vel[1]) {
                                w->vel[1] = 3.0f;
                            }
                            if (w->vel[1] < -3.0f) {
                                w->vel[1] = -3.0f;
                            }
                            if (w->model->frame < 10.0f) {
                                w->model->frame += 0.8;
                                if (10.0f < w->model->frame) {
                                    w->model->frame = 10.0f;
                                }
                            }
                            if (w->goal[0] < w->x) {
                                ad = w->x - w->goal[0];
                            } else {
                                ad = -(w->x - w->goal[0]);
                            }
                            if (ad < 12.0f) {
                                w->step++;
                                w->vel[1] *= -0.9;
                                w->atk->ofs[0] = 0;
                                w->atk->ofs[2] = 0;
                                w->dmg->flags |= 2;
                                PRINTF("攻撃終了モーション\n");
                            }
                            w->z *= 0.90; /* FAKE spelling (IDO_NOTES 365), as state 2's: a ugen load */
                            break;
                        case 2:
                            eneTurnToPoint(o, tx, tz, 3.0f);
                            w->vel[0] *= 0.85;
                            w->vel[1] *= 0.95;
                            w->model->frame -= 0.2;
                            if (w->model->frame < 1.0f) {
                                PRINTF("突っ込み攻撃終わります\n");
                                w->timer = 0;
                                w->step = 0;
                                w->state = 0;
                                (*karasuTenguSetAnim)(o, 0, 0);
                            }
                            break;
                    }
                    break;
                case 4:
                    eneTurnToPoint(o, tx, tz, 3.0f);
                    switch (w->step) {
                        case 0:
                            PRINTF("ダメージモーション\n");
                            dist = 1000000.0f;
                            (*karasuTenguSetAnim)(o, 4, 0);
                            w->step++;
                            best = -1;
                            for (i = 0; i < 4; i++) {
                                if (plIsInPlay(i)) {
                                    plGetModelPos(gGameWork.players[i], &qx, &qy, &qz);
                                    if (sqrtf(SQ(qz - w->atk->hitPos[0]) + SQ(qx - w->atk->hitPos[0])) < dist) {
                                        dist = sqrtf(SQ(qz - w->atk->hitPos[0]) + SQ(qx - w->atk->hitPos[0]));
                                        best = i;
                                    }
                                }
                            }
                            if (best == -1) {
                                PRINTF("だれに当てられたか分かりません\n");
                                w->vel[2] = -3.0f;
                            } else {
                                plAskPos(gGameWork.players[best], &place2, &height2, &side2, &zone);
                                if (place2 < w->x) {
                                    PRINTF("**** >左からの攻撃\n");
                                    w->vel[2] = -3.0f;
                                    w->vel[0] = 3.0f;
                                } else {
                                    PRINTF("**** >右からの攻撃\n");
                                    w->vel[0] = -3.0f;
                                    w->vel[2] = 3.0f;
                                }
                            }
                            PRINTF("のこりHP --> %d", w->life - 1);
                            if (--w->life == 0) {
                                w->step = 2;
                                w->vel[1] = 0.0f;
                                shimEneDeathAct(o, w->dmg, (u32) karasuTenguHalf0Dl, (u32) karasuTenguHalf1Dl);
                                karasuBattenAdd(karasuDispTask);
                                karasuSetKilled(w->id);
                                w->atk->flags |= 0x10;
                                return -1;
                            }
                            eneMakeHitSpark(w->zone, w->x, w->y + 10.0f, w->z, 2, w->rot[1]);
                            eneHitSoundHeavy(w->dmg);
                            break;
                        case 1:
                            eneAnimLoop(o, w->model, 0.2f);
                            w->vel[0] *= 0.9;
                            w->vel[2] *= 0.9;
                            w->vel[1] *= 0.8;
                            if (w->timer % 4 >= 2) {
                                w->model->f.flags |= 2;
                            } else {
                                w->model->f.flags &= ~2;
                            }
                            if (w->vel[0] > 0) {
                                ad = w->vel[0];
                            } else {
                                ad = -w->vel[0];
                            }
                            if (ad < 0.3) {
                                if (w->timer > 30) {
                                    PRINTF("NORMALへ\n");
                                    w->model->f.flags |= 2;
                                    w->state = 0;
                                    (*karasuTenguSetAnim)(o, 0, 0);
                                    w->step = 0;
                                    w->timer = 0;
                                    w->dmg->flags |= 2;
                                }
                            }
                            break;
                        case 2:
                            eneAnimLoop(o, w->model, 0.2f);
                            if (w->timer % 4 >= 2) {
                                w->model->f.flags |= 2;
                            } else {
                                w->model->f.flags &= ~2;
                            }
                            w->vel[1] -= 0.02;
                            w->vel[0] *= 0.9;
                            w->vel[2] *= 0.9;
                            if (w->vel[1] < -2.0) {
                                w->vel[1] = -2.0f;
                            }
                            if (ABS(w->vel[0]) < 0.3) {
                                PRINTF("死にます\n");
                                itexPuffWhite(w->model->pos[0], w->model->pos[1] + 20.0f, w->model->pos[2], 0.1f);
                                karasuDeathAct(o, w->dmg, (u32) karasuTenguHalf0Dl, (u32) karasuTenguHalf1Dl);
                                karasuBattenAdd(karasuDispTask);
                                karasuSetKilled(w->id);
                                w->atk->flags |= 0x10;
                                return -1;
                            }
                            break;
                    }
                    break;
            }
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            mx = w->model->pos[0];
            mz = w->model->pos[2];
            eneMove((EneObj *) w);
            eneWallCheck(o, 10.0f, 5.0f);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            if (w->model->anim == karasuTenguAnim0) {
                if (2.4 < w->model->frame) {
                    if (w->flapped == 0) {
                        eneSetSd(o, 0x2CB);
                        w->flapped = 1;
                    }
                }
                if (w->model->frame < 3.0) {
                    w->flapped = 0;
                }
            }
            switch (w->state) {
                case 3:
                    w->model->rot[0] *= 0.99;
                    w->model->rot[2] *= 0.99;
                    break;
                case 0:
                    dx = w->model->pos[0] - mx;
                    dz = w->model->pos[2] - mz;
                    lim = 45.0f;
                    w->model->rot[0] = dz * 30.0f * DEG2RAD;
                    w->model->rot[2] = -dx * 30.0f * DEG2RAD;
                    if (lim * DEG2RAD < w->model->rot[0]) {
                        w->model->rot[0] = lim * DEG2RAD;
                    }
                    if (w->model->rot[0] < -lim * DEG2RAD) {
                        w->model->rot[0] = -lim * DEG2RAD;
                    }
                    if (lim * DEG2RAD < w->model->rot[2]) {
                        w->model->rot[2] = lim * DEG2RAD;
                    }
                    if (w->model->rot[2] < -lim * DEG2RAD) {
                        w->model->rot[2] = -lim * DEG2RAD;
                    }
                    if (w->model->rot[0] / DEG2RAD > 0) {
                        a = w->model->rot[0] / DEG2RAD;
                    } else {
                        a = -(w->model->rot[0] / DEG2RAD);
                    }
                    if (w->model->rot[2] / DEG2RAD > 0) {
                        b = w->model->rot[2] / DEG2RAD;
                    } else {
                        b = -(w->model->rot[2] / DEG2RAD);
                    }
                    m = lim / (b + a) * lim;
                    if (m * DEG2RAD < w->model->rot[0]) {
                        w->model->rot[0] = m * DEG2RAD;
                    }
                    if (w->model->rot[0] < -m * DEG2RAD) {
                        w->model->rot[0] = -m * DEG2RAD;
                    }
                    if (m * DEG2RAD < w->model->rot[2]) {
                        w->model->rot[2] = m * DEG2RAD;
                    }
                    if (w->model->rot[2] < -m * DEG2RAD) {
                        w->model->rot[2] = -m * DEG2RAD;
                    }
                    break;
                case 4:
                    dx = w->model->pos[0] - mx;
                    dz = w->model->pos[2] - mz;
                    lim = 45.0f;
                    w->model->rot[0] = dz * 30.0f * DEG2RAD;
                    w->model->rot[2] = -dx * 30.0f * DEG2RAD;
                    if (lim * DEG2RAD < w->model->rot[0]) {
                        w->model->rot[0] = lim * DEG2RAD;
                    }
                    if (w->model->rot[0] < -lim * DEG2RAD) {
                        w->model->rot[0] = -lim * DEG2RAD;
                    }
                    if (lim * DEG2RAD < w->model->rot[2]) {
                        w->model->rot[2] = lim * DEG2RAD;
                    }
                    if (w->model->rot[2] < -lim * DEG2RAD) {
                        w->model->rot[2] = -lim * DEG2RAD;
                    }
                    if (w->model->rot[0] / DEG2RAD > 0) {
                        a = w->model->rot[0] / DEG2RAD;
                    } else {
                        a = -(w->model->rot[0] / DEG2RAD);
                    }
                    if (w->model->rot[2] / DEG2RAD > 0) {
                        b = w->model->rot[2] / DEG2RAD;
                    } else {
                        b = -(w->model->rot[2] / DEG2RAD);
                    }
                    m = lim / (b + a) * lim;
                    if (m * DEG2RAD < w->model->rot[0]) {
                        w->model->rot[0] = m * DEG2RAD;
                    }
                    if (w->model->rot[0] < -m * DEG2RAD) {
                        w->model->rot[0] = -m * DEG2RAD;
                    }
                    if (m * DEG2RAD < w->model->rot[2]) {
                        w->model->rot[2] = m * DEG2RAD;
                    }
                    if (w->model->rot[2] < -m * DEG2RAD) {
                        w->model->rot[2] = -m * DEG2RAD;
                    }
                    break;
                case 2:
                    dx = -w->model->pos[0] - mx;
                    dz = -w->model->pos[2] - mz;
                    lim = 22.0f;
                    w->model->rot[0] = dz * 30.0f * DEG2RAD;
                    w->model->rot[2] = -dx * 30.0f * DEG2RAD;
                    if (lim * DEG2RAD < w->model->rot[0]) {
                        w->model->rot[0] = lim * DEG2RAD;
                    }
                    if (w->model->rot[0] < -lim * DEG2RAD) {
                        w->model->rot[0] = -lim * DEG2RAD;
                    }
                    if (lim * DEG2RAD < w->model->rot[2]) {
                        w->model->rot[2] = lim * DEG2RAD;
                    }
                    if (w->model->rot[2] < -lim * DEG2RAD) {
                        w->model->rot[2] = -lim * DEG2RAD;
                    }
                    if (w->model->rot[0] / DEG2RAD > 0) {
                        a = w->model->rot[0] / DEG2RAD;
                    } else {
                        a = -(w->model->rot[0] / DEG2RAD);
                    }
                    if (w->model->rot[2] / DEG2RAD > 0) {
                        b = w->model->rot[2] / DEG2RAD;
                    } else {
                        b = -(w->model->rot[2] / DEG2RAD);
                    }
                    m = lim / (b + a) * lim;
                    if (m * DEG2RAD < w->model->rot[0]) {
                        w->model->rot[0] = m * DEG2RAD;
                    }
                    if (w->model->rot[0] < -m * DEG2RAD) {
                        w->model->rot[0] = -m * DEG2RAD;
                    }
                    if (m * DEG2RAD < w->model->rot[2]) {
                        w->model->rot[2] = m * DEG2RAD;
                    }
                    if (w->model->rot[2] < -m * DEG2RAD) {
                        w->model->rot[2] = -m * DEG2RAD;
                    }
                    break;
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 2.0f, NULL);
            if (acollAtkWasHit(w->atk) == 1) {
                if ((Obj *) w->atk->hitPower != NULL) {
                    PRINTF("eneAfterProc(): 攻撃ヒット\n");
                    tskSendMsg((Obj *) w->atk->hitPower, 7, o);
                }
            }
            PRINTF("MAX_TIME->%f\n", dist);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The karasu tengu's animation n (karasuTenguAnims), from its first frame or, with last 1, its
 * last. */
void karasuTenguSetAnim(Obj *o, s32 n, s32 last) {
    KarasuW *w = KARASU(o);

    w->model->anim = karasuTenguAnims[n];
    polAnimeLastFrame(w->model);
    if (last == 1) {
        w->model->frame = polAnimeLastFrame(w->model);
    } else {
        w->model->frame = 0.0f;
    }
}

/* "kara_tama": a karasu tengu's spirit shot (kiaidan). It picks the farthest player in play,
 * flies at where that player stands (speed 4), its alpha rising by 30 a step, and is gone when
 * eneAfterProc says so. File 84's bzHaniTamaProc is its shape.
 * Matched 2026-10-15; it was parked for plIsInPlay's spilled address, one temp word off the
 * game's (0x44 for 0x4C). The distance is written out in sqrtf's argument, so px - x and pz - z
 * are each used twice and IDO gives each a home word under plIsInPlay's, as the game's frame has
 * them. pad3 is the two local words dx and dz took (the game's frame keeps them; nothing reads
 * them). */
s32 karasuTamaProc(Obj *o, s16 msg, void *arg) {
    KarasuTamaW *w = KARASUTAMA(o);
    s32 i;
    f32 dist;
    s32 pad3[2];
    f32 px;
    f32 py;
    f32 pz;
    f32 d;
    f32 a;
    f32 place;
    f32 height;
    f32 side;
    s32 pad[2];
    s32 zone;
    s32 pad2[2];
    Vec3f v;

    switch (msg) {
        case 2:
            o->name = "kara_tama";
            eneSetup(o, arg);
            dist = 0.0f;
            for (i = 0; i < 4; i++) {
                if (plIsInPlay(i)) {
                    plGetModelPos(gGameWork.players[i], &px, &py, &pz);
                    d = sqrtf((px - w->e.x) * (px - w->e.x) + (pz - w->e.z) * (pz - w->e.z));
                    if (dist < d) {
                        w->target = i;
                        dist = d;
                        plAskPos(gGameWork.players[w->target], &place, &height, &side, &zone);
                    }
                }
            }
            DPRINTF("気合弾作ります... target -->%d\n", w->target);
            w->e.model = (Frame *) enePolModelMake(o, (Gfx *) karasuTamaDl, NULL, 1);
            if (w->e.model == NULL) {
                return -1;
            }
            w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 8, 0, 0, 0);
            if (w->e.atk == NULL) {
                return -1;
            }
            w->e.atk->kind = 0x7B;
            ((PolFrame *) w->e.model)->flags |= 7;
            ((PolFrame *) w->e.model)->scale[2] = 0.5f;
            ((PolFrame *) w->e.model)->scale[0] = ((PolFrame *) w->e.model)->scale[1] =
                ((PolFrame *) w->e.model)->scale[2];
            w->alpha = 10;
            a = mathAtan2(w->e.x - place, w->e.y - height);
            w->e.vel[0] = -sinf(a) * 4.0f;
            w->e.vel[1] = -cosf(a) * 4.0f;
            v.x = w->e.vel[0];
            v.y = w->e.vel[1];
            v.z = w->e.vel[2];
            *(Vec3f *) &w->e.atk->info = v;
            eneSetSd(o, 0x21C);
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            if (w->e.hit & 0xF) {
                w->e.behave |= 0x4000000;
            }
            w->alpha += 30;
            if (w->alpha > 255) {
                w->alpha = 255;
            }
            if (eneAfterProc(o) == -1) {
                DPRINTF("Etsk_bz_hani_tama: 死亡\n");
                return -1;
            }
            /* fallthrough */
        case 3:
            ((PolFrame *) w->e.model)->dl = eneDlEnvPrimColor(o, 255, 255, 255, 191, 0, 255, w->alpha);
            break;
        case 1:
            if (w->e.model != NULL) {
                frmDestroy(w->e.model);
            }
            if (w->e.atk != NULL) {
                frmDestroy(&w->e.atk->frame);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "k_lady": the lady (her error print is a copy of kodomoRed_proc's). Away while the
 * hide-and-seek runs (0x13 without 0xAC). Talked to, she turns to the player and talks (0x266);
 * if the player takes on the hunt (0x94) the screen fades and the game goes to the atlas with
 * the hunt's stage open; back from it (0x92), she talks again, gives the pass between them
 * (kind 0x1C3, sparks and kind 0x1C1) when it is earned (0x93), the pass goes to the player
 * (scenario 0x20, then 0x268), and the sky's tunes change. File 126's tsuchiJisanProc is its
 * shape. */
s32 karasuLadyProc(Obj *o, s16 msg, void *arg) {
    KarasuLadyW *w = KARASULADY(o);
    Obj *player;
    u32 data;
    CamView *view;
    s32 pad;
    Vec3f at;
    s32 i;
    s32 pad2[2];
    f32 x;
    f32 y;
    f32 z;
    Vec3f spark;

    switch (msg) {
        case 2:
            o->name = "k_lady";
            if (swWorkTest(0x13)) {
                if (!swWorkTest(0xAC)) {
                    DPRINTF("****  カクレンボ中につき不在\n");
                    return -1;
                }
                DPRINTF("*** ???\n");
            }
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, karasuLadyAnims[0]);
            if (w->model == NULL) {
                return -1;
            }
            w->rot[1] = 0.2454369962f;
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("kodomoRed_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 100, 0, 0);
            demoTalkInit(o, data);
            w->shadow = shadowCreate();
            if (swWorkTest(0x92)) {
                swWorkReset(0x92);
                DPRINTF(">>>> イベントから帰って来た場合\n");
                gGameWork.scenarioRunning = 2;
                camSunSetClockStop(1);
                eneSetAngle(o, 30.0f);
                w->state = 5;
            }
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            demoTalkerFree(o);
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (demoTalkEndCheck(o)) {
                        demoCamZoom_CL(w->talker);
                        w->state = 1;
                    }
                    eneTurnToAngle(o, 200.0f, 4.0f);
                    break;
                case 1:
                    if (w->talker != NULL) {
                        tskSendMsg(w->talker, 4, &at);
                        if (eneTurnToPoint(o, at.x, at.z, 10.0f) == 1) {
                            w->state = 2;
                        }
                    }
                    break;
                case 2:
                    scRunStart(0x266, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    w->state = 3;
                    break;
                case 3:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x94)) {
                            swWorkReset(0x94);
                            gGameWork.scenarioRunning = 2;
                            w->state = 4;
                            w->timer = 0;
                        } else {
                            personTalkCamOff();
                            demoFinish(o);
                            w->state = 0;
                        }
                    }
                    break;
                case 4:
                    if (w->timer == 1) {
                        sndPlay(10);
                        karasuFadeOut(16 / gRetracesPerFrame);
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        view = camGetView(w->cam);
                        camSetSpeed(w->cam, 20 / gRetracesPerFrame);
                        camMoveAtY(w->cam, view->at[1] + 50.0f);
                        gGameWork.scenarioRunning = 2;
                        for (i = 0; i < 4; i++) {
                            gSaveCopyPlayers[i].chara = -1;
                        }
                    }
                    if (w->timer == 20 / gRetracesPerFrame) {
                        gGameWork.scenarioRunning = 0;
                        camAiSetHold(0);
                        demoAreaFlagSave();
                        demoAreaFlagSetBit7(3);
                        gSaveData.stageFlags[0x32] |= 1;
                        gSaveData.stageFlags[0x32] &= ~0x80;
                        gSaveData.stageFlags[0x2B] |= 1;
                        gSaveData.stageFlags[0x2B] &= ~0x80;
                        stageGotoAtlas(3, 3);
                    }
                    break;
                case 5:
                    if (w->timer == 1) {
                        demoPlStandAll();
                    }
                    if (w->timer == 8) {
                        if (scCountKarasuShot() < 6) {
                            camSunClearTunes();
                            camSunSetTunes4(0x89, 0x86, 0x87, 0x88);
                        }
                        demoCamZoom_CL(plCheck());
                        swWorkReset(0x92);
                        demoAreaFlagRestore();
                        scRunStart(0x266, 0, 0);
                        w->state = 6;
                    }
                    break;
                case 6:
                    if (!scIsRunning()) {
                        swWorkReset(0x92);
                        if (swWorkTest(0x93)) {
                            w->state = 7;
                            swWorkReset(0x93);
                        } else {
                            w->state = 10;
                        }
                    }
                    break;
                case 7:
                    player = plCheck();
                    if (player == NULL) {
                        return -1;
                    }
                    plGetModelPos(player, &x, &y, &z);
                    w->pass = eneMakeEx(NULL, ENE_DEMO_TEGATA, 0, (w->model->pos[0] + x) / 2, w->model->pos[1],
                                        (w->model->pos[2] + z) / 2, 0, NULL);
                    w->timer = 0;
                    w->state = 8;
                    break;
                case 8:
                    if (w->timer == 200 / gRetracesPerFrame) {
                        demoGetEneModelPos(w->pass, &spark);
                        ptclSpawnSparksFall(spark.x, spark.y, spark.z);
                        eneMakeEx(NULL, ENE_DEMO_KIRAKIRA, 0, spark.x, spark.y, spark.z, 0, NULL);
                        karasuPassToPlayer(w->pass, plCheck());
                        w->flags |= 1;
                    }
                    if (w->timer == 260 / gRetracesPerFrame) {
                        scRunStart(0x20, 0, 0);
                        w->state = 9;
                    }
                    break;
                case 9:
                    if (!scIsRunning()) {
                        scRunStart(0x268, 0, 0);
                        w->state = 10;
                    }
                    break;
                case 10:
                    if (!scIsRunning()) {
                        personTalkCamOff();
                        demoPlReleaseAll();
                        w->state = 0;
                    }
                    break;
            }
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            if (w->flags & 1) {
                if (300 / gRetracesPerFrame == w->tuneTimer++) {
                    w->flags &= ~1;
                    camSunSetTunes4(0x89, 0x86, 0x87, 0x88);
                }
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.6f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "demoKarasutengu_proc": the hunt's manager. Made only while the hunt is on (flag 0x17) and the
 * pass not yet given (0xAE). The first time (0x1B) the clock goes to 18:00. Once the players
 * stand it runs scenario 0x298 with the day made 1200 seconds long, then makes the karasu tengu
 * (kind 0x1C2) and the count. When six are shot down it runs scenario 0x21 and the players
 * watch; then (or at 6:00, or when the mission is over, 2) the screen fades, the day's length
 * comes back and the game goes back to the lady (spawn 0xEF) with flag 0x92 set. File 126's
 * demoTsuchi_proc is its shape. */
s32 demoKarasutengu_proc(Obj *o, s16 msg, void *arg) {
    DemoKarasuW *w = DEMOKARASU(o);
    s32 pad[4];
    s32 i;
    CamView *view;

    switch (msg) {
        case 2:
            o->name = "d_Karasu";
            if (!swWorkTest(0x17)) {
                DPRINTF("demoKarasutengu_proc: カラステング殲滅計画は開始されません\n");
                return -1;
            }
            if (swWorkTest(0xAE)) {
                DPRINTF("demoKarasutengu_proc: カラステング殲滅計画は開始されません 手形もらってるで\n");
                return -1;
            }
            if (!swWorkTest(0x1B)) {
                swWorkSet(0x1B);
                gGameWork.time = 0x120000;
                gGameWork.scenarioRunning = 2;
                w->state = 0;
            } else {
                w->state = 1;
            }
            floadLoadSet(0x83);
            gGameWork.mission = 1;
            DPRINTF("demoKarasu セット完了\n");
            break;
        case 1:
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (w->timer == 1) {
                        demoPlStandAll();
                    }
                    if (w->timer == 8) {
                        sndPlay(0xBA);
                        karasuClearKilled();
                        camSunSetClockStop(1);
                        gGameWork.savedDaySeconds = gGameWork.daySeconds;
                        gGameWork.daySeconds = 1200;
                        w->partake = gGameWork.partake;
                        gGameWork.partake = 1;
                        scRunStart2(0x298, 0, 0);
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (!scIsRunning()) {
                        gGameWork.partake = w->partake;
                        camSunSetClockStop(0);
                        demoPlReleaseAll();
                        w->tengu = eneMakeEx(NULL, ENE_DEMO_TIMER, 0, 0.0f, 0.0f, 0.0f, 2,
                                             (u32 (*)(void *, void *, void *)) 0x10006);
                        w->disp = karasuDispMake(o);
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (gGameWork.time >= 0x60000 && gGameWork.time < gGameWork.rate + 0x60000) {
                        w->timer = 0;
                        w->state = 5;
                    }
                    if (scCountKarasuShot() >= 6) {
                        scRunStart2(0x21, 0, 0);
                        gGameWork.partake = 1;
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i) == 1) {
                                plEnterDemo(gGameWork.players[i]);
                                PL(gGameWork.players[i])->flags |= 0x10000000;
                            }
                        }
                        camAiSetHold(1);
                        fadeSetColor(0, 0, 0);
                        fadeSetLevel(128.0f);
                        w->timer = 0;
                        w->state = 4;
                    }
                    break;
                case 4:
                    if (40 / gRetracesPerFrame < w->timer && !scIsRunning()) {
                        gGameWork.scenarioRunning = 2;
                        w->timer = 0;
                        w->state = 8;
                    }
                    break;
                case 5:
                    if (40 / gRetracesPerFrame < w->timer && !scIsRunning()) {
                        gGameWork.scenarioRunning = 2;
                        sndPlay(10);
                        w->timer = 0;
                        w->state = 8;
                    }
                    break;
                case 8:
                    if (w->timer == 1) {
                        for (i = 0; i < 4; i++) {
                            contPakInitRumble(i);
                            contPakRumbleRequestStop(i);
                        }
                        swWorkReset(0x1B);
                        karasuFadeOut(16 / gRetracesPerFrame);
                        w->tengu = tskDestroy(w->tengu);
                        w->disp = tskDestroy(w->disp);
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        view = camGetView(w->cam);
                        camSetSpeed(w->cam, 20 / gRetracesPerFrame);
                        camMoveAtY(w->cam, view->at[1] + 50.0f);
                        gGameWork.scenarioRunning = 2;
                        gGameWork.daySeconds = gGameWork.savedDaySeconds;
                        if (scCountKarasuShot() < 6) {
                            sndPlay(0x45F);
                        }
                    }
                    if (w->timer == 20 / gRetracesPerFrame) {
                        gGameWork.areaStage = 3;
                        gGameWork.scenarioRunning = 0;
                        gGameWork.mission = 0;
                        gGameWork.partake = 0;
                        swWorkSet(0x92);
                        stageGotoSpawn(gStageTask->parent, 0xEF, 0, 31.0f, 0.0f, 29.0f, 4.014257431f, 0xD7);
                    }
                    break;
            }
            if (gGameWork.mission == 2) {
                if (w->state != 8) {
                    DPRINTF("%%%%   event manager: PLAYER 死にました信号をうけとったぜ！\n");
                    gGameWork.scenarioRunning = 2;
                    sndPlay(10);
                    demoPlStandAll();
                    w->timer = 0;
                    w->state = 8;
                }
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Whether karasu tengu id has been shot down (its bit in gGameWork.karasuShot). */
u8 karasuIsKilled(u8 id) {
    return gGameWork.karasuShot & (u8) (1 << (u8) (id % 8));
}

/* Karasu tengu id has been shot down. */
void karasuSetKilled(u8 id) {
    gGameWork.karasuShot |= (u8) (1 << (u8) (id % 8));
}

void karasuDummy(void) {
}

/* None shot down yet. */
void karasuClearKilled(void) {
    gGameWork.karasuShot = 0;
}

/* "karasuDisp_proc": the count on the screen, six karasu tengu faces in a row under its own
 * camera (flapping), each crossed out (a "batten", grown in) when it is shot down. */
s32 karasuDisp_proc(Obj *o, s16 msg, void *arg) {
    KarasuDispW *w = KARASUDISP(o);
    /* One index per message: with one i for all four loops IDO colours the faces' and crosses'
     * loads as one value across the cases. */
    s32 i;
    s32 j;
    s32 k;
    s32 l;
    s32 last;
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "ka_disp";
            w->cam = (CamView *) frmCreate((Frame *) gFramePool10, gFrmKindCamera);
            if (w->cam == NULL) {
                DPRINTF(">>>>>  karasuDisp_proc cam つくれません\n");
                return -1;
            }
            w->cam->ortho |= 1;
            w->cam->zNear = -200.0f;
            w->cam->zFar = 200.0f;
            for (i = 0; i < 6; i++) {
                w->faces[i] = (AnimeFrame *) frmCreate(&w->cam->f, gFrmKindAnime);
                if (w->faces[i] == NULL) {
                    return -1;
                }
                w->faces[i]->dl = ((SegToRamFunc) fadeSegToRam)(karasuDispDl, o->file);
                /* Stored as a pointer: the game builds the address at each pass, where a u32
                 * ("(u32) karasuTenguAnim0") is a value IDO keeps in a saved register across the call
                 * before it (file 126's fadeSegToRam note). */
                *(u8 **) &w->faces[i]->anim = karasuTenguAnim0;
                w->faces[i]->flags |= 0x20;
                floadGetSetRam((void **) w->faces[i]->segs, 0x95);
                w->faces[i]->rot[0] = 3.141592741f;
                w->faces[i]->pos[0] = i * 18 + 84;
                w->faces[i]->pos[1] = 38.0f;
                w->faces[i]->scale[2] = 0.400000006f;
                w->faces[i]->scale[0] = w->faces[i]->scale[1] = w->faces[i]->scale[2];
            }
            for (i = 1; i < 7; i++) {
                if (karasuIsKilled(i)) {
                    karasuBattenAdd(o);
                }
            }
            DPRINTF("karasuDisp_proc: 撃墜したカラス数[%d]体\n", w->n);
            karasuDispTask = o;
            DPRINTF("karasuDisp_proc: セットＯＫ\n");
            break;
        case 1:
            for (j = 0; j < 6; j++) {
                if (w->faces[j] != NULL) {
                    frmDestroy(&w->faces[j]->f);
                }
                if (w->battens[j] != NULL) {
                    frmDestroy(&w->battens[j]->f);
                }
            }
            break;
        case 3:
            for (k = 0; k < 6; k++) {
                if (w->faces[k] != NULL) {
                    w->faces[k]->dl = eneDlEnvColorLit(o, 0, 0, 0, 0, 1);
                }
            }
            break;
        case 0:
            for (l = 0; l < 6; l++) {
                if (w->battens[l] != NULL) {
                    if (w->battens[l]->scale[0] < 0.05999999866f) {
                        w->battens[l]->scale[0] += 0.01999999955f;
                        w->battens[l]->scale[1] += 0.01999999955f;
                        w->battens[l]->scale[2] += 0.01999999955f;
                        last = polAnimeLastFrame(w->faces[l]);
                        w->faces[l]->frame += 0.1;
                        if (last < w->faces[l]->frame) {
                            w->faces[l]->frame = last;
                        }
                    } else {
                        w->battens[l]->scale[2] = 0.05999999866f;
                        w->battens[l]->scale[0] = w->battens[l]->scale[1] = w->battens[l]->scale[2];
                    }
                }
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

Obj *karasuDispMake(Obj *obj) {
    return tskCreate(obj, karasuDisp_proc, obj->file, NULL);
}

/* "battenAdd_proc": a cross over the next face of the count. */
void karasuBattenAdd(Obj *o) {
    KarasuDispW *w = KARASUDISP(o);
    s32 pad;

    w->battens[w->n] = (PolFrame *) frmCreate(&w->cam->f, gFrmKindPol);
    if (w->battens[w->n] == NULL) {
        DPRINTF("battenAdd_proc:Error: not enough frame memory\n");
    }
    w->battens[w->n]->dl = ((SegToRamFunc) fadeSegToRam)(karasuDispDl, o->file);
    w->battens[w->n]->dl2 = karasuBattenDl;
    floadGetSetRam((void **) w->battens[w->n]->segs, 0x83);
    w->battens[w->n]->rot[0] = 3.141592741f;
    w->battens[w->n]->pos[0] = w->n * 18 + 84;
    w->battens[w->n]->pos[1] = 33.0f;
    w->battens[w->n]->scale[2] = 0.009999999776f;
    w->battens[w->n]->scale[0] = w->battens[w->n]->scale[1] = w->battens[w->n]->scale[2];
    w->n++;
    DPRINTF("battenAdd_proc: batten set!\n");
}

/* A karasu tengu's death, by the hit's kind (demo.c's demoEneDeathAct with its own sounds and a
 * smaller spirit): with no halves' lists, the smoke. Then, by chance, an item: none 10 times in
 * 100, kind 0x1FC 20 times, else kind 0x1FB.
 * The game keeps e in its home (0x34) with no saved register at all; written plainly, IDO keeps
 * e in s0. Two empty block boundaries before the switch lower e's colouring priority below a
 * saved register's cost. */
void karasuDeathAct(Obj *o, ACollFrame *box, u32 dl, u32 dl2) {
    u8 kind = box->hitKind;
    s32 r;
    s32 pad;
    f32 x;
    f32 y;
    f32 z;
    f32 s;
    EneObj *e = ENE(o);

    if (dl == 0 || dl2 == 0) {
        kind = 7;
    }
    x = ((PolFrame *) e->model)->pos[0];
    y = ((PolFrame *) e->model)->pos[1];
    z = ((PolFrame *) e->model)->pos[2];
    s = ((PolFrame *) e->model)->scale[0];
    /* FAKE: two block boundaries (e stays in its home); OoT's `if (1) {}` spelling */
    if (1) {}
    if (1) {}
    switch (kind) {
        case 4:
        case 5:
        case 6:
            eneSetSd2(o, 0x21F, 0, 0);
            eneMakeKillHahen(o, box, dl, dl2);
            break;
        case 18:
        case 19:
        case 20:
            eneSetSd2(o, 0x22D, 0, 0);
            eneBlowOff(o, box);
            break;
        default:
            eneSetSd2(o, 0x229, 0x22A, 0);
            itexSparkleBlue(x, y + 5.0f, z, s * 0.1);
            ptclSpawnSparksFall(x, y, z);
            effectFspiritCreate(x, y, z, s * 0.1, 0xE0, 0xE0, 0xE0, 0x10, 0x10, 0x10);
            break;
    }
    r = fadeRand(100);
    if (r >= 10) {
        if (r < 30) {
            eneMakeEx(NULL, ENE_ETSK_DANGO_MAIN, e->zone, e->x, e->y, 0.0f, 0, NULL);
        } else {
            eneMakeEx(NULL, ENE_ETSK_KOBAN_MAIN, e->zone, e->x, e->y, 0.0f, 0, NULL);
        }
    }
}
