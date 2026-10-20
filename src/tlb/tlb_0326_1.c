#include "tlb.h"
#include "tlb/tlb_0326.h"

extern Obj *gBattleTask; /* the battle's task (beffect.c) */

/* acollAtkAddYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);

/* "Ohuda"'s (bkasOhudaProc) argument: only what its starter sets. */
typedef struct OhudaArg {
    /* 0x00 */ u8 pad00[0x8];
    /* 0x08 */ s32 type;
    /* 0x0C */ f32 pos[3];
    /* 0x18 */ u8 pad18[0x4];
} OhudaArg;

/* "Ohuda"'s work. */
typedef struct OhudaW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ ACollFrame *atk;
    /* 0x08 */ u8 pad08[0x10];
    /* 0x18 */ f32 alpha;
} OhudaW;

extern u8 bkasOhudaType0Dl[]; /* the four charms' display lists (types 0-3), in file 0xF5 */
extern u8 bkasOhudaType1Dl[];
extern u8 bkasOhudaType2Dl[];
extern u8 bkasOhudaType3Dl[];

/* "Ohuda" (a paper charm, type 0-3) at the position given, tilted 45 degrees (the other way for
 * type 1). It unfolds from 0.01 to 0.15 high by 0.02 a frame and fades from 255 by 4 to black.
 * Its attack box (radius 5000, kind 0x50) is never live: each frame clears its live bit. It
 * ends at once when the battle ends. */
s32 bkasOhudaProc(Obj *obj, s16 msg, void *arg) {
    OhudaW *w = (OhudaW *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Ohuda";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                PRINTF("隊長,タスクの作成に失敗しました。\n");
                return -1;
            }
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 5000, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x50;
            if (((OhudaArg *) arg)->type == 0) {
                bossSetPolDL(w->frame, (Gfx *) bkasOhudaType0Dl, 0xF5);
            } else if (((OhudaArg *) arg)->type == 1) {
                bossSetPolDL(w->frame, (Gfx *) bkasOhudaType1Dl, 0xF5);
            } else if (((OhudaArg *) arg)->type == 2) {
                bossSetPolDL(w->frame, (Gfx *) bkasOhudaType2Dl, 0xF5);
            } else if (((OhudaArg *) arg)->type == 3) {
                bossSetPolDL(w->frame, (Gfx *) bkasOhudaType3Dl, 0xF5);
            }
            w->frame->pos[0] = ((OhudaArg *) arg)->pos[0];
            w->frame->pos[1] = ((OhudaArg *) arg)->pos[1];
            w->frame->pos[2] = ((OhudaArg *) arg)->pos[2];
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x20);
            if (((OhudaArg *) arg)->type == 0) {
                w->frame->rot[1] = 0.0f;
                w->frame->rot[2] = 0.7853982f;
            } else if (((OhudaArg *) arg)->type == 1) {
                w->frame->rot[1] = 0.0f;
                w->frame->rot[2] = -0.7853982f;
            } else {
                w->frame->rot[1] = 0.0f;
                w->frame->rot[2] = 0.7853982f;
            }
            w->frame->scale[2] = 0.15f;
            w->frame->scale[0] = w->frame->scale[2];
            w->frame->scale[1] = 0.01f;
            w->alpha = 255.0f;
            break;
        case 0:
            if (gBattleFlags & 0x10) {
                return -1;
            }
            if (gBattleFlags2 & 0x4000) {
                return -1;
            }
            w->frame->scale[1] += 0.02;
            if (w->frame->scale[1] > 0.15) {
                w->frame->scale[1] = 0.15f;
            }
            w->alpha -= 4.0f;
            if (w->alpha < 0.0f) {
                return -1;
            }
            w->atk->flags &= ~2;
            break;
        case 3:
            w->frame->dl = bossXluPrimEnvColorDL(0, 0, 0, w->alpha);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->atk->frame);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* An "Ohuda" of a type at x, y, z under the effect task. */
void bkasOhudaCreate(f32 x, f32 y, f32 z, s32 type) {
    OhudaArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.type = type;
    tskCreate(gEffectTask, bkasOhudaProc, gBattleTask->file, &arg);
}

/* The two "Sensu" (fan) tasks' argument: only what their starters set. */
typedef struct SensuArg {
    /* 0x00 */ u8 pad00[0x20];
    /* 0x20 */ s32 type;
    /* 0x24 */ f32 pos[3];
    /* 0x30 */ u8 pad30[0xC];
} SensuArg;

/* "Sensu"'s work. */
typedef struct SensuW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 vel[3]; /* BmoveX's and bossApplyVel2's (bmove.c) */
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ ACollFrame *atk;
    /* 0x18 */ ACollFrame *dmg;
    /* 0x1C */ u8 pad1C[0x4];
    /* 0x20 */ s32 type;
    /* 0x24 */ u8 pad24[0x10];
    /* 0x34 */ s32 state; /* 0: opening; 1: flying, spinning */
    /* 0x38 */ f32 distSq; /* from the boss */
} SensuW;

/* acollDmgAddYZ returns its box (main declares it void). */
typedef ACollFrame *(*DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                    s32 a);

extern CamView *gBattleCamView; /* the battle's camera (beffect.c) */
extern u8 bkasSensuAnim[]; /* the fan's animation, in file 0xF5 */

/* "Sensu" (a thrown fan): an animated fan from file 0xF5, twice life size, with an attack box
 * (radius 100, 50 up, kind 0x51) and a damage box (150 by 150, kind 0x52). It opens, then flies
 * at speed 15 toward a random point near the Impact, spinning. Hit by hit kinds 15, 1, 3, 5, 8
 * or 11 it bursts in a puff (with coins, and sound 0x228 unless the battle is ending, for kinds
 * 1, 3, 5, 8); hitting with its attack it bursts too. It ends 15000 from the boss, or when the
 * battle sets gBattleFlags2 0x4000 or 0x400. */
s32 bkasSensuProc(Obj *obj, s16 msg, void *arg) {
    SensuW *w = (SensuW *) FX(obj);
    s32 kind;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "Sensu";
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                PRINTF("隊長,タスクの作成に失敗しました。\n");
                return -1;
            }
            bossSetAnimAt(w->frame, bkasSensuAnim, 0xF5, 0.0f);
            bossSetFrameFlags2(w->frame, 0x20);
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 100, 50, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x51;
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->frame->f, 150, 150, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 0x52;
            w->frame->pos[0] = ((SensuArg *) arg)->pos[0];
            w->frame->pos[1] = ((SensuArg *) arg)->pos[1];
            w->frame->pos[2] = ((SensuArg *) arg)->pos[2];
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 2.0f;
            w->type = ((SensuArg *) arg)->type;
            BmoveX(obj, fadeRandRange(-150, 150) + gImpactPosX, fadeRandRange(-50, 50) + gImpactPosY, gImpactPosZ,
                   15.0f);
            w->state = 0;
            break;
        case 3:
            break;
        case 0:
            if (gBattleFlags2 & 0x4000) {
                return -1;
            }
            if (gBattleFlags2 & 0x400) {
                return -1;
            }
            if (acollDmgWasHit(w->dmg)
                && ((kind = w->dmg->hitKind) == 15 || kind == 1 || kind == 3 || kind == 5 || kind == 8
                    || kind == 11)) {
                if (kind == 1 || kind == 3 || kind == 5 || kind == 8) {
                    ryouCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
                }
                itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
                if (!(gBattleFlags & 0x10)) {
                    SoundLocation2(0x228, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                   300.0f, 15000.0f, gBattleCamView->fovy);
                }
                return -1;
            }
            if (acollAtkWasHit(w->atk) == 1) {
                itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
                return -1;
            }
            w->distSq = bossDistSq(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], gBossPosX, gBossPosY,
                                   gBossPosZ);
            if (w->distSq > 2.25e8f) {
                return -1;
            }
            switch (w->state) {
                case 0:
                    if (bossStepAnim(w->frame, 0.3f, 2)) {
                        w->state++;
                    }
                    break;
                case 1:
                    w->frame->rot[2] -= 0.08726646501940039;
                    bossApplyVel2(obj);
                    break;
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->atk->frame);
            frmDestroy(&w->dmg->frame);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "Sensu" of a type at x, y, z under the effect task. */
void bkasSensuCreate(f32 x, f32 y, f32 z, s32 type) {
    SensuArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.type = type;
    tskCreate(gEffectTask, bkasSensuProc, gBattleTask->file, &arg);
}

/* The create argument of the "hand" effect tasks (bkasHandProc, bkasHand2Proc). */
typedef struct HandArg {
    /* 0x00 */ u8 pad00[8];
    /* 0x08 */ s32 type;
    /* 0x0C */ u8 pad0C[4];
    /* 0x10 */ f32 pos[3];
    /* 0x1C */ u8 pad1C[4];
} HandArg;

/* The "hand" tasks' work. */
typedef struct HandW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ ACollFrame *atk; /* the slap's attack box */
    /* 0x08 */ s32 type; /* 0 or 1: which hand */
    /* 0x0C */ s32 time;
    /* 0x10 */ u8 pad10[0xC];
    /* 0x1C */ f32 alpha;
} HandW;

extern u8 bkasHandType0Dl[]; /* the hands' display lists (types 0 and 1), in file 0xF5 */
extern u8 bkasHandType1Dl[];
void bkasHand2Create(f32 x, f32 y, f32 z, s32 type);

/* "hand", the slap ("harite"): a hand (type 0 or 1) three times life size beside the boss, with
 * an attack box (radius 150, kind 0x53 or 0x54) that is live one frame in ten. It keeps its
 * place by the boss's heading and leaves a fading hand (bkasHand2Create) every fourth
 * frame. It ends when the boss says so (gBossFlags 0x8000) or the battle ends. */
s32 bkasHandProc(Obj *obj, s16 msg, void *arg) {
    s32 pad[2];
    HandW *w = (HandW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "hand";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 150, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->type = ((HandArg *) arg)->type;
            if (w->type == 0) {
                w->atk->kind = 0x53;
                bossSetPolDL(w->frame, (Gfx *) bkasHandType0Dl, 0xF5);
            } else if (w->type == 1) {
                w->atk->kind = 0x54;
                bossSetPolDL(w->frame, (Gfx *) bkasHandType1Dl, 0xF5);
            }
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x27);
            if (w->type == 0) {
                bossHeadingOffset(gBossYaw, -250.0f, 1240.0f, &w->frame->pos[0], &w->frame->pos[2]);
                w->frame->pos[1] = gBossPosY + 300.0f;
            } else if (w->type == 1) {
                bossHeadingOffset(gBossYaw, 50.0f, 1200.0f, &w->frame->pos[0], &w->frame->pos[2]);
                w->frame->pos[1] = gBossPosY + 400.0f;
            }
            w->frame->pos[0] += gBossPosX;
            w->frame->pos[2] += gBossPosZ;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 3.0f;
            w->type = ((HandArg *) arg)->type;
            w->time = 0;
            w->alpha = 255.0f;
            break;
        case 0:
            if (gBattleFlags & 0x10) {
                return -1;
            }
            if (gBattleFlags2 & 0x4000) {
                return -1;
            }
            if (!(w->time & 3)) {
                (*bkasHand2Create)(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], w->type);
            }
            if (w->type == 0) {
                bossHeadingOffset(gBossYaw, -250.0f, 1240.0f, &w->frame->pos[0], &w->frame->pos[2]);
            } else if (w->type == 1) {
                bossHeadingOffset(gBossYaw, 50.0f, 1240.0f, &w->frame->pos[0], &w->frame->pos[2]);
            }
            w->frame->pos[0] += gBossPosX;
            w->frame->pos[2] += gBossPosZ;
            if (gGameFrameCount % 10 == 0) {
                w->atk->flags |= 2;
            } else {
                w->atk->flags &= ~2;
            }
            if (gBossFlags & 0x8000) {
                PRINTF("はりて終了命令がでましたので消去します。\n");
                return -1;
            }
            w->time++;
            break;
        case 3:
            w->frame->dl = bossXluPrimEnvColorDL(w->alpha / 2, 0, 0, w->alpha);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->atk->frame);
            /* the game's .rodata keeps this print after case 0's; where it stood is not known */
            PRINTF("隊長,タスクの作成に失敗しました。\n");
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

extern Obj *gBattleTask; /* the battle's task (beffect.c) */

void bkasHandCreate(f32 x, f32 y, f32 z, s32 type) {
    HandArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.type = type;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bkasHandProc, gBattleTask->file, &arg);
}

/* "hand": a hand (type 0 or 1) three times life size, 50 in front of its position, that grows
 * by 3% a frame and fades from 128 by 10 a frame, red going to black. It ends at once when the
 * battle ends (gBattleFlags 0x10, gBattleFlags2 0x4000). */
s32 bkasHand2Proc(Obj *obj, s16 msg, void *arg) {
    HandW *w = (HandW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "hand";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                PRINTF("隊長,タスクの作成に失敗しました。\n");
                return -1;
            }
            if (((HandArg *) arg)->type == 0) {
                bossSetPolDL(w->frame, (Gfx *) bkasHandType0Dl, 0xF5);
            } else if (((HandArg *) arg)->type == 1) {
                bossSetPolDL(w->frame, (Gfx *) bkasHandType1Dl, 0xF5);
            }
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x27);
            w->frame->pos[0] = ((HandArg *) arg)->pos[0];
            w->frame->pos[1] = ((HandArg *) arg)->pos[1];
            w->frame->pos[2] = ((HandArg *) arg)->pos[2] - 50.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 3.0f;
            w->time = 0;
            w->alpha = 128.0f;
            break;
        case 0:
            if (gBattleFlags & 0x10) {
                return -1;
            }
            if (gBattleFlags2 & 0x4000) {
                return -1;
            }
            w->frame->scale[0] *= 1.03;
            w->frame->scale[1] *= 1.03;
            w->frame->scale[2] *= 1.03;
            w->alpha -= 10.0f;
            if (w->alpha < 0.0f) {
                return -1;
            }
            break;
        case 3:
            w->frame->dl = bossXluPrimEnvColorDL(w->alpha / 2, 0, 0, w->alpha);
            break;
        case 1:
            frmDestroy(&((HandW *) FX(obj))->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

void bkasHand2Create(f32 x, f32 y, f32 z, s32 type) {
    HandArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.type = type;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bkasHand2Proc, gBattleTask->file, &arg);
}

extern u8 bkasTakiTex[]; /* a 32x64 I4 texture, in file 0xF5 */
Gfx *bkasTakiDL(Gfx *gfx, u16 time);

/* A 32x64 4-bit texture loaded into TMEM at tmem for tile rtile, as gDPLoadMultiBlock_4b, with
 * its tile size at (uls, ult): one macro, so one source line to IDO, as tlb_0050.c's
 * gDPLoadScrollBlock (waterFallDL is these builders' twin). */
#define gDPLoadScrollBlock_4b(pkt, fmt, timg, tmem, rtile, uls, ult)                                 \
    {                                                                                                \
        gDPSetTextureImage(pkt, fmt, G_IM_SIZ_16b, 1, timg);                                         \
        gDPSetTile(pkt, fmt, G_IM_SIZ_16b, 0, tmem, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, 6,  \
                   G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD);                            \
        gDPLoadSync(pkt);                                                                            \
        gDPLoadBlock(pkt, G_TX_LOADTILE, 0, 0, ((32 * 64 + 3) >> 2) - 1, CALC_DXT_4b(32));           \
        gDPPipeSync(pkt);                                                                            \
        gDPSetTile(pkt, fmt, G_IM_SIZ_4b, 2, tmem, rtile, 0, G_TX_WRAP | G_TX_NOMIRROR, 6,           \
                   G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD);                            \
        gDPSetTileSize(pkt, rtile, uls, ult, (32 - 1) << 2, (64 - 1) << 2);                          \
    }

/* bkasWaterDL's twin for an I4 texture: bkasTakiTex twice, tile 0 at TMEM 0 from s 0
 * and tile 1 at 0x80 from s 10, both scrolling down, the second twice as fast (waterFallDL's
 * form, with the 4-bit words). Matched 2026-10-12: the first tile's s is 0, not 10 (the game's
 * "lui at,0xF200" ORs no s into the first SetTileSize; the second adds "ori at,at,0xA000"). */
Gfx *bkasTakiDL(Gfx *gfx, u16 time) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 128, 128, 128, 128);
    gDPLoadScrollBlock_4b(gfx++, G_IM_FMT_I, bkasTakiTex, 0, G_TX_RENDERTILE, 0, time);
    gDPLoadScrollBlock_4b(gfx++, G_IM_FMT_I, bkasTakiTex, 0x80, 1, 10, time * 2);
    gSPEndDisplayList(gfx++);
    return gfx;
}

extern u8 bkasWaterTex[]; /* the water's texture (32x64 CI4), in file 0xF5 */

/* The water's texture (32x64 CI4, loaded as gDPLoadMultiBlock_4b), twice: tile 0 at TMEM 0 from
 * s 0 and tile 1 at 0x80 from s 10, both scrolling down, the second twice as fast. Matched
 * 2026-10-12: the first tile's s is 0, and each load is the one gDPLoadScrollBlock_4b macro
 * (one source line), as its twin. */
Gfx *bkasWaterDL(Gfx *gfx, u16 time) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 128, 128, 128, 128);
    gDPLoadScrollBlock_4b(gfx++, G_IM_FMT_CI, bkasWaterTex, 0, G_TX_RENDERTILE, 0, time);
    gDPLoadScrollBlock_4b(gfx++, G_IM_FMT_CI, bkasWaterTex, 0x80, 1, 10, time * 2);
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* "water"'s work. */
typedef struct WaterW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[0x8];
    /* 0x0C */ s32 time;
} WaterW;

extern u8 bkasWaterDl[]; /* the water's display list, in file 0xF5 */
Gfx *bkasWaterDL(Gfx *gfx, u16 time);

/* "water": a polygon from file 0xF5 at the origin, its display list built each frame by
 * bkasWaterDL from its time. */
s32 bkasWaterProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    WaterW *w = (WaterW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "water";
            w->frame = (PolFrame *) frmCreate(gFrmLayers[6], gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bkasWaterDl, 0xF5);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 1.0f;
            w->frame->pos[0] = w->frame->pos[1] = w->frame->pos[2] = 0.0f;
            break;
        case 1:
            frmDestroy(&((WaterW *) FX(obj))->frame->f);
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gGfxDlHead = bkasWaterDL(gGfxDlHead, w->time);
            w->time++;
            break;
        case 0:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Taki"'s work. */
typedef struct TakiW {
    /* 0x00 */ PolFrame *frames[3];
    /* 0x0C */ s32 time;
} TakiW;

extern u8 bkasTakiDl0[]; /* the waterfall's three display lists, in file 0xF5 */
extern u8 bkasTakiDl1[];
extern u8 bkasTakiDl2[];

/* "Taki" (the waterfall): three polygons from file 0xF5 at the origin, each given a display list
 * from bkasTakiDL each frame. Each creation checks the first frame again, not the
 * one just made, as the game does. A pad word before w puts the cached call pointer at the
 * game's 0x20 (IDO_NOTES 348). */
s32 bkasTakiProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    TakiW *w = (TakiW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "Taki";
            w->frames[0] = (PolFrame *) frmCreate(gFrmLayers[6], gFrmKindPol);
            if (w->frames[0] == NULL) {
                return -1;
            }
            w->frames[1] = (PolFrame *) frmCreate(gFrmLayers[6], gFrmKindPol);
            if (w->frames[0] == NULL) {
                return -1;
            }
            w->frames[2] = (PolFrame *) frmCreate(gFrmLayers[6], gFrmKindPol);
            if (w->frames[0] == NULL) {
                return -1;
            }
            bossSetPolDL(w->frames[0], (Gfx *) bkasTakiDl0, 0xF5);
            bossSetPolDL(w->frames[1], (Gfx *) bkasTakiDl1, 0xF5);
            bossSetPolDL(w->frames[2], (Gfx *) bkasTakiDl2, 0xF5);
            w->frames[0]->scale[0] = w->frames[0]->scale[1] = w->frames[0]->scale[2] = 1.0f;
            w->frames[0]->pos[0] = w->frames[0]->pos[1] = w->frames[0]->pos[2] = 0.0f;
            w->frames[1]->scale[0] = w->frames[1]->scale[1] = w->frames[1]->scale[2] = 1.0f;
            w->frames[1]->pos[0] = w->frames[1]->pos[1] = w->frames[1]->pos[2] = 0.0f;
            w->frames[2]->scale[0] = w->frames[2]->scale[1] = w->frames[2]->scale[2] = 1.0f;
            w->frames[2]->pos[0] = w->frames[2]->pos[1] = w->frames[2]->pos[2] = 0.0f;
            break;
        case 1:
            frmDestroy(&w->frames[0]->f);
            frmDestroy(&w->frames[1]->f);
            frmDestroy(&w->frames[2]->f);
            break;
        case 3:
            w->frames[0]->dl = gGfxDlHead;
            gGfxDlHead = bkasTakiDL(gGfxDlHead, w->time);
            w->frames[1]->dl = gGfxDlHead;
            gGfxDlHead = bkasTakiDL(gGfxDlHead, w->time);
            w->frames[2]->dl = gGfxDlHead;
            gGfxDlHead = bkasTakiDL(gGfxDlHead, w->time);
            w->time--;
            break;
        case 0:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "kabukiB"'s (bkasKabukiBProc) argument: only what its starter sets. */
typedef struct KabukiBArg {
    /* 0x00 */ u8 pad00[0x18];
    /* 0x18 */ f32 pos[3];
    /* 0x24 */ u8 pad24[0x20];
    /* 0x44 */ s32 type;
} KabukiBArg;

#ifndef DPRINTF
#define DPRINTF
#endif

/* "kabukiB"'s work (kabukiDB's layout). */
typedef struct KabukiBW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[0x2C];
    /* 0x30 */ f32 scale;
    /* 0x34 */ u8 pad34[0x4];
    /* 0x38 */ s32 timer;
    /* 0x3C */ s32 alpha;
    /* 0x40 */ u32 state; /* 0: in front of the boss; 1: rising; 2: swelling, then bursting */
    /* 0x44 */ s32 type;  /* the burst's pattern: 0-4, or 9 */
} KabukiBW;

extern u8 bkasSparkDl[]; /* the spark's display list, in file 0x120 */
void bkasKabukiDBCreate(f32 x, f32 y, f32 z, f32 tx, f32 ty, f32 tz, f32 scale, s32 type);
void kballEffectCreate(f32 x, f32 y, f32 z, f32 scale, f32 radius);
void bkasKabukiDB2Create(f32 x, f32 y, f32 z, f32 a, f32 b, f32 c, f32 d, s32 n);

/* Seven kabukiDBs from the ball to (tx, ty) at the boss's depth (one of them type 1). */
#define KABUKIB_SHOT(tx, ty, type)                                                                      \
    (*bkasKabukiDBCreate)(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], tx, ty, gBossPosZ, 5.0f, \
                              type)

/* "kabukiB": the kabuki ball. It flickers 500 in front of the boss (size 15, sparks every 10
 * frames) for 120 frames, rises 8 a frame at twice the size until frame 240, then swells to three
 * times. At frame 250 it bursts in a white flash with sound 0x24D into seven kabukiDBs in one of
 * five patterns (its type, or at random), or for type 9 into bkasKabukiDB2Create's attack, and
 * fades. The type is stored, then tested through w (`w->type = arg->type; if (w->type != 9)`):
 * IDO forwards the store, and the value stays a temp, t3, as the game's. Matched 2026-10-12. */
s32 bkasKabukiBProc(Obj *obj, s16 msg, void *arg);
s32 bkasKabukiBProc(Obj *obj, s16 msg, void *arg) {
    KabukiBW *w = (KabukiBW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "kabukiB";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bkasSparkDl, 0x120);
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x22);
            w->scale = 15.0f;
            bossHeadingOffset(gBossYaw, 0.0f, 500.0f, &w->frame->pos[0], &w->frame->pos[2]);
            w->frame->pos[0] += gBossPosX;
            w->frame->pos[1] = gBossPosY;
            w->frame->pos[2] += gBossPosZ;
            beFlashPCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 30);
            DPRINTF("カブキ玉のタイプは[%d]です。\n", ((KabukiBArg *) arg)->type);
            w->type = ((KabukiBArg *) arg)->type;
            if (w->type != 9) {
                w->type = fadeRand(100) % 5;
                DPRINTF("カブキ玉ランダムタイプ:[%d]\n", w->type);
            }
            DPRINTF("クリエイトの段階ではタイプは[%d/%d]\n", w->type, ((KabukiBArg *) arg)->type);
            w->alpha = 255;
            w->timer = 0;
            w->state = 0;
            break;
        case 0:
            if (gBattleFlags & 0x10) {
                return -1;
            }
            if (gBattleFlags2 & 0x4000) {
                return -1;
            }
            switch (w->state) {
                case 0:
                    w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                        fadeRandRange(8, 12) / 10.0f * w->scale;
                    bossHeadingOffset(gBossYaw, 0.0f, 500.0f, &w->frame->pos[0], &w->frame->pos[2]);
                    w->frame->pos[0] += gBossPosX;
                    w->frame->pos[1] = gBossPosY;
                    w->frame->pos[2] += gBossPosZ;
                    if (w->timer % 10 == 0) {
                        (*kballEffectCreate)(w->frame->pos[0], w->frame->pos[1] + 200.0f, w->frame->pos[2],
                                                  3.0f, 800.0f);
                        (*kballEffectCreate)(w->frame->pos[0], w->frame->pos[1] + 200.0f, w->frame->pos[2],
                                                  2.0f, 800.0f);
                    }
                    if (w->timer >= 121) {
                        w->state++;
                    }
                    break;
                case 1:
                    w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                        fadeRandRange(8, 12) / 10.0f * (w->scale * 2);
                    w->frame->pos[1] += 8.0f;
                    if (w->timer >= 241) {
                        w->state++;
                    }
                    break;
                case 2:
                    w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                        fadeRandRange(8, 12) / 10.0f * (w->scale * 3.0f);
                    if (w->timer == 250) {
                        beFlashFCreate(255, 255, 255, 5);
                        effectKiranCreate(gBossPosX, gBossPosY, gBossPosZ - 500.0f);
                        sndPlay(0x24D);
                        if (w->type == 9) {
                            (*bkasKabukiDB2Create)(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 0.0f,
                                                      w->frame->pos[1], gBossPosZ, 5.0f, 0);
                        } else if (w->type == 0) {
                            KABUKIB_SHOT(-1500.0f, w->frame->pos[1] - 800.0f, 0);
                            KABUKIB_SHOT(-1000.0f, w->frame->pos[1] - 800.0f, 0);
                            KABUKIB_SHOT(-500.0f, w->frame->pos[1] - 800.0f, 0);
                            KABUKIB_SHOT(0.0f, w->frame->pos[1] - 800.0f, 1);
                            KABUKIB_SHOT(500.0f, w->frame->pos[1] - 800.0f, 0);
                            KABUKIB_SHOT(1000.0f, w->frame->pos[1] - 800.0f, 0);
                            KABUKIB_SHOT(1500.0f, w->frame->pos[1] - 800.0f, 0);
                        } else if (w->type == 1) {
                            KABUKIB_SHOT(0.0f, gBossPosY, 1);
                            KABUKIB_SHOT(0.0f, gBossPosY + 400.0f, 0);
                            KABUKIB_SHOT(0.0f, gBossPosY - 400.0f, 0);
                            KABUKIB_SHOT(800.0f, gBossPosY + 200.0f, 0);
                            KABUKIB_SHOT(-800.0f, gBossPosY + 200.0f, 0);
                            KABUKIB_SHOT(800.0f, gBossPosY - 200.0f, 0);
                            KABUKIB_SHOT(-800.0f, gBossPosY - 200.0f, 0);
                        } else if (w->type == 2) {
                            KABUKIB_SHOT(-1000.0f, gBossPosY + 400.0f, 0);
                            KABUKIB_SHOT(-1000.0f, gBossPosY - 400.0f, 0);
                            KABUKIB_SHOT(-500.0f, gBossPosY - 200.0f, 0);
                            KABUKIB_SHOT(0.0f, gBossPosY, 1);
                            KABUKIB_SHOT(500.0f, gBossPosY + 200.0f, 0);
                            KABUKIB_SHOT(1000.0f, gBossPosY + 400.0f, 0);
                            KABUKIB_SHOT(1000.0f, gBossPosY - 400.0f, 0);
                        } else if (w->type == 3) {
                            KABUKIB_SHOT(-1000.0f, gBossPosY - 400.0f, 0);
                            KABUKIB_SHOT(-1000.0f, gBossPosY + 400.0f, 0);
                            KABUKIB_SHOT(-500.0f, gBossPosY + 200.0f, 0);
                            KABUKIB_SHOT(0.0f, gBossPosY, 1);
                            KABUKIB_SHOT(500.0f, gBossPosY - 200.0f, 0);
                            KABUKIB_SHOT(1000.0f, gBossPosY - 400.0f, 0);
                            KABUKIB_SHOT(1000.0f, gBossPosY + 400.0f, 0);
                        } else if (w->type == 4) {
                            KABUKIB_SHOT(-1500.0f, gBossPosY - 300.0f, 0);
                            KABUKIB_SHOT(-1000.0f, gBossPosY + 300.0f, 0);
                            KABUKIB_SHOT(-500.0f, gBossPosY - 300.0f, 0);
                            KABUKIB_SHOT(0.0f, gBossPosY + 300.0f, 1);
                            KABUKIB_SHOT(500.0f, gBossPosY - 300.0f, 0);
                            KABUKIB_SHOT(1000.0f, gBossPosY + 300.0f, 0);
                            KABUKIB_SHOT(1500.0f, gBossPosY - 300.0f, 0);
                        }
                    }
                    w->alpha -= 5;
                    if (w->alpha < 0) {
                        DPRINTF("隊長,タスクの作成に失敗しました。\n");
                        DPRINTF("隊長,タスクの作成に失敗しました。\n");
                        return -1;
                    }
                    break;
                case 3:
                case 4:
                case 5:
                case 6:
                    break;
            }
            w->timer++;
            break;
        case 3:
            if (gGameFrameCount % 5 == 0) {
                w->frame->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 255, w->alpha);
            } else {
                w->frame->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 0, w->alpha);
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "kabukiB" of a type at x, y, z under the effect task. */
void bkasKabukiBCreate(f32 x, f32 y, f32 z, s32 type) {
    KabukiBArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.type = type;
    tskCreate(gEffectTask, bkasKabukiBProc, gBattleTask->file, &arg);
}

/* "kabukiDB"'s (bkasKabukiDBProc) argument: only what its starter sets. */
typedef struct KabukiDBArg {
    /* 0x00 */ u8 pad00[0x18];
    /* 0x18 */ f32 pos[3];
    /* 0x24 */ f32 target[3];
    /* 0x30 */ f32 scale;
    /* 0x34 */ u8 pad34[0x10];
    /* 0x44 */ s32 type;
} KabukiDBArg;

s32 bkasKabukiDBProc(Obj *obj, s16 msg, void *arg);

/* A "kabukiDB" from x, y, z toward a target, of a size and a type, under the effect task. */
void bkasKabukiDBCreate(f32 x, f32 y, f32 z, f32 tx, f32 ty, f32 tz, f32 scale, s32 type) {
    KabukiDBArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.target[0] = tx;
    arg.target[1] = ty;
    arg.target[2] = tz;
    arg.scale = scale;
    arg.type = type;
    tskCreate(gEffectTask, bkasKabukiDBProc, gBattleTask->file, &arg);
}

/* "kabukiDB"'s work. */
typedef struct KabukiDBW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3]; /* BmoveX's and bossApplyVel's (bmove.c) */
    /* 0x10 */ ACollFrame *atk;
    /* 0x14 */ ACollFrame *dmg;
    /* 0x18 */ f32 start[3];
    /* 0x24 */ f32 target[3];
    /* 0x30 */ f32 scale;
    /* 0x34 */ u8 pad34[0x4];
    /* 0x38 */ s32 timer;
    /* 0x3C */ s32 alpha;
    /* 0x40 */ u32 state; /* 0: flying to the target; 1: swelling; 2: flying home */
    /* 0x44 */ s32 type; /* 1: flashes red, else blue */
} KabukiDBW;

extern u8 bkasSparkDl[]; /* the spark's display list, in file 0x120 */

/* "kabukiDB": a flickering ball (file 0x120's spark, 0.5 to 1.5 times the given size) with an
 * attack box (radius 15, kind 0x56) and a damage box (16 by 32, 16 down, kind 0x57). It flies at
 * speed 40 to its target (500 further in z), swells to three times its size for 30 frames, then
 * flies at speed 30 to the centre and ends there. Hit by hit kinds 15, 1, 3, 5, 8 or 11 it
 * bursts in a puff (with coins for kinds 1, 3, 5, 8) and sound 0x290 unless the battle is
 * ending; landing its attack it bursts in a white flash. */
s32 bkasKabukiDBProc(Obj *obj, s16 msg, void *arg) {
    KabukiDBW *w = (KabukiDBW *) FX(obj);
    s32 kind;
    f32 s;

    switch (msg) {
        case 2:
            obj->name = "kabukiDB";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bkasSparkDl, 0x120);
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 15, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x56;
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->frame->f, 16, 32, -16, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 0x57;
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x22);
            w->scale = ((KabukiDBArg *) arg)->scale;
            w->start[0] = ((KabukiDBArg *) arg)->pos[0];
            w->start[1] = ((KabukiDBArg *) arg)->pos[1];
            w->start[2] = ((KabukiDBArg *) arg)->pos[2];
            w->target[0] = ((KabukiDBArg *) arg)->target[0];
            w->target[1] = ((KabukiDBArg *) arg)->target[1];
            w->target[2] = ((KabukiDBArg *) arg)->target[2] + 500.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                fadeRandRange(5, 15) / 10.0f * ((KabukiDBArg *) arg)->scale;
            w->frame->pos[0] = w->start[0];
            w->frame->pos[1] = w->start[1];
            w->frame->pos[2] = w->start[2];
            w->type = ((KabukiDBArg *) arg)->type;
            w->alpha = 255;
            w->timer = 0;
            w->state = 0;
            break;
        case 0:
            if (gBattleFlags & 0x10) {
                return -1;
            }
            if (gBattleFlags2 & 0x4000) {
                return -1;
            }
            if (acollDmgWasHit(w->dmg)
                && ((kind = w->dmg->hitKind) == 15 || kind == 1 || kind == 3 || kind == 5 || kind == 8
                    || kind == 11)) {
                itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                kind = w->dmg->hitKind;
                if (kind == 1 || kind == 3 || kind == 5 || kind == 8) {
                    ryouCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
                }
                if (!(gBattleFlags & 0x10)) {
                    SoundLocation2(0x290, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                   300.0f, 15000.0f, gBattleCamView->fovy);
                }
                return -1;
            }
            if (acollAtkWasHit(w->atk) == 1) {
                itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                beFlashFCreate(255, 255, 255, 180);
                return -1;
            }
            bossApplyVel(obj);
            switch (w->state) {
                case 0:
                    w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                        fadeRandRange(8, 12) / 10.0f * w->scale;
                    if (BmoveX(obj, w->target[0], w->target[1], w->target[2], 40.0f) == 0) {
                        w->timer = 0;
                        w->state++;
                    }
                    break;
                case 1:
                    w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                        fadeRandRange(8, 12) / 10.0f * (w->scale * 3.0f);
                    if (w->timer > 30) {
                        w->state++;
                    }
                    break;
                case 2:
                    w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                        fadeRandRange(8, 12) / 10.0f * (w->scale * 2);
                    if (BmoveX(obj, 0.0f, 0.0f, 0.0f, 30.0f) == 0) {
                        return -1;
                    }
                    break;
                case 3:
                    break;
                case 4:
                    break;
            }
            w->timer++;
            break;
        case 3:
            if (w->type == 1) {
                if (gGameFrameCount % 5 == 0) {
                    w->frame->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 255, w->alpha);
                } else {
                    w->frame->dl = bossPrimEnvColorDL(255, 255, 255, 255, 0, 0, w->alpha);
                }
            } else if (gGameFrameCount % 5 == 0) {
                w->frame->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 255, w->alpha);
            } else {
                w->frame->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 0, w->alpha);
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->atk->frame);
            frmDestroy(&w->dmg->frame);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "KballE"'s (bkasKballEProc) argument: only what its starter sets. */
typedef struct KballEArg {
    /* 0x00 */ u8 pad00[0x18];
    /* 0x18 */ f32 pos[3];
    /* 0x24 */ u8 pad24[0xC];
    /* 0x30 */ f32 scale;
    /* 0x34 */ f32 radius;
    /* 0x38 */ u8 pad38[0x10];
} KballEArg;

/* "KballE"'s work. */
typedef struct KballEW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3]; /* BmoveX's and bossApplyVel's (bmove.c) */
    /* 0x10 */ u8 pad10[8];
    /* 0x18 */ f32 center[3];
    /* 0x24 */ u8 pad24[0xC];
    /* 0x30 */ f32 scale;
    /* 0x34 */ u8 pad34[0x8];
    /* 0x3C */ s32 alpha;
} KballEW;

/* "KballE": a spark of the ball. It starts at a random point on a sphere of the given radius
 * around the given centre, 0.3 to 1.7 times the given size, flies back to the centre at speed
 * 10 and fades from 192 by 3 a frame, blinking blue every fifth frame. */
s32 bkasKballEProc(Obj *obj, s16 msg, void *arg) {
    KballEW *w = (KballEW *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "KballE";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                PRINTF("[kballEffectCreate]隊長,タスクの作成に失敗しました。\n");
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bkasSparkDl, 0x120);
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x22);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                fadeRandRange(3, 17) / 10.0f * ((KballEArg *) arg)->scale;
            w->scale = ((KballEArg *) arg)->scale;
            bossHeadingOffset(fadeRand(360) * 0.01745329300388008, 0.0f, ((KballEArg *) arg)->radius,
                              &w->frame->pos[0], &w->frame->pos[1]);
            bossHeadingOffset(fadeRand(360) * 0.01745329300388008, 0.0f, ((KballEArg *) arg)->radius,
                              &w->frame->pos[0], &w->frame->pos[2]);
            w->frame->pos[0] += ((KballEArg *) arg)->pos[0];
            w->frame->pos[1] += ((KballEArg *) arg)->pos[1];
            w->frame->pos[2] += ((KballEArg *) arg)->pos[2];
            w->alpha = 192;
            w->center[0] = ((KballEArg *) arg)->pos[0];
            w->center[1] = ((KballEArg *) arg)->pos[1];
            w->center[2] = ((KballEArg *) arg)->pos[2];
            break;
        case 0:
            bossApplyVel(obj);
            if (BmoveX(obj, w->center[0], w->center[1], w->center[2], 10.0f) == 0) {
                return -1;
            }
            w->alpha -= 3;
            if (w->alpha <= 0) {
                return -1;
            }
            break;
        case 3:
            if (gGameFrameCount % 5 == 0) {
                w->frame->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 255, w->alpha);
            } else {
                w->frame->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 0, w->alpha);
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "KballE" around x, y, z, of a size and at a radius, under the effect task. */
void kballEffectCreate(f32 x, f32 y, f32 z, f32 scale, f32 radius) {
    KballEArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = scale;
    arg.radius = radius;
    tskCreate(gEffectTask, bkasKballEProc, gBattleTask->file, &arg);
}

/* "wakemi"'s (bkasWakemiProc) argument: only what its starter sets. */
typedef struct WakemiArg {
    /* 0x00 */ u8 pad00[0x8];
    /* 0x08 */ f32 pos[3];
    /* 0x14 */ u8 pad14[0x14];
    /* 0x28 */ s32 type;
} WakemiArg;

/* "wakemi"'s work. */
typedef struct WakemiW {
    /* 0x00 */ AnimeFrame *body;
    /* 0x04 */ AnimeFrame *glow;
    /* 0x08 */ f32 pos[3]; /* z is from the boss */
    /* 0x14 */ u8 pad14[0x4];
    /* 0x18 */ s32 alpha;
    /* 0x1C */ u8 pad1C[0x4];
    /* 0x20 */ s32 timer;
    /* 0x24 */ s32 state; /* 0: fading in; 1: holding; 2: fading out */
    /* 0x28 */ s32 type; /* the colour: 0 grey, 1 blue, 2 red, 3 green */
} WakemiW;

extern u8 bkasWakemiBodyAnim[]; /* the two animations, in file 0xF5 */
extern u8 bkasWakemiGlowAnim[];

/* "wakemi": two animated models from file 0xF5, twice life size, in the type's colour, that keep
 * their place beside the boss (z from the boss) and turn toward the centre. They fade in by 5 to
 * 64 while the first animation plays once, hold 300 frames, then fade out by 1 a frame. They
 * end at once when the battle sets gBattleFlags2 0x400. */
s32 bkasWakemiProc(Obj *obj, s16 msg, void *arg) {
    WakemiW *w = (WakemiW *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "wakemi";
            w->body = (AnimeFrame *) frmCreate(gFrmLayer8, gFrmKindAnime);
            if (w->body == NULL) {
                PRINTF("[wakemiCreate]隊長,タスクの作成に失敗しました。\n");
                return -1;
            }
            w->glow = (AnimeFrame *) frmCreate(gFrmLayer8, gFrmKindAnime);
            if (w->glow == NULL) {
                return -1;
            }
            bossSetAnimAt(w->body, bkasWakemiBodyAnim, 0xF5, 0.0f);
            bossSetFrameFlags2(w->body, 0x20);
            bossSetAnimAt(w->glow, bkasWakemiGlowAnim, 0xF5, 0.0f);
            bossSetFrameFlags2(w->glow, 0x20);
            w->pos[0] = ((WakemiArg *) arg)->pos[0];
            w->pos[1] = ((WakemiArg *) arg)->pos[1];
            w->pos[2] = ((WakemiArg *) arg)->pos[2];
            w->type = ((WakemiArg *) arg)->type;
            w->body->pos[0] = ((WakemiArg *) arg)->pos[0];
            w->body->pos[1] = ((WakemiArg *) arg)->pos[1];
            w->body->pos[2] = ((WakemiArg *) arg)->pos[2] + gBossPosZ;
            w->body->scale[0] = w->body->scale[1] = w->body->scale[2] = 2.0f;
            w->glow->pos[0] = w->body->pos[0];
            w->glow->pos[1] = w->body->pos[1];
            w->glow->pos[2] = w->body->pos[2];
            w->glow->scale[0] = w->glow->scale[1] = w->glow->scale[2] = 2.0f;
            w->state = 0;
            w->alpha = 0;
            break;
        case 0:
            if (gBattleFlags2 & 0x400) {
                return -1;
            }
            w->body->pos[2] = w->pos[2] + gBossPosZ;
            w->glow->pos[0] = w->body->pos[0];
            w->glow->pos[1] = w->body->pos[1];
            w->glow->pos[2] = w->body->pos[2];
            bossTurnToward2(w->body, 0.0f, 0.0f, 10.0f);
            bossStepAnim(w->glow, 0.2f, 0);
            switch (w->state) {
                case 0:
                    w->alpha += 5;
                    if (w->alpha > 64) {
                        w->alpha = 64;
                    }
                    if (bossStepAnim(w->body, 0.2f, 2)) {
                        w->timer = 300;
                        w->state++;
                    }
                    break;
                case 1:
                    bossStepAnim(w->body, 0.2f, 0);
                    if (w->timer-- == 0) {
                        w->state++;
                    }
                    break;
                case 2:
                    bossStepAnim(w->body, 0.2f, 0);
                    w->alpha--;
                    if (w->alpha < 0) {
                        return -1;
                    }
                    break;
            }
            break;
        case 3:
            if (w->type == 0) {
                w->body->dl = bossCombineEnvColorDL(128, 128, 128, w->alpha);
                w->glow->dl = bossCombineEnvColorDL(128, 128, 128, w->alpha);
            } else if (w->type == 1) {
                w->body->dl = bossCombineEnvColorDL(0, 0, 128, w->alpha);
                w->glow->dl = bossCombineEnvColorDL(0, 0, 128, w->alpha);
            } else if (w->type == 2) {
                w->body->dl = bossCombineEnvColorDL(128, 0, 0, w->alpha);
                w->glow->dl = bossCombineEnvColorDL(128, 0, 0, w->alpha);
            } else if (w->type == 3) {
                w->body->dl = bossCombineEnvColorDL(0, 128, 0, w->alpha);
                w->glow->dl = bossCombineEnvColorDL(0, 128, 0, w->alpha);
            }
            break;
        case 1:
            frmDestroy(&w->body->f);
            frmDestroy(&w->glow->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "wakemi" of a type at x, y, z under the effect task. */
void wakemiCreate(f32 x, f32 y, f32 z, s32 type) {
    WakemiArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.type = type;
    tskCreate(gEffectTask, bkasWakemiProc, gBattleTask->file, &arg);
}

/* "Kstage"'s (bkasKstageProc) argument: only the position its starter sets. */
typedef struct KstageArg {
    /* 0x00 */ u8 pad00[0x8];
    /* 0x08 */ f32 pos[3];
    /* 0x14 */ u8 pad14[0x10];
} KstageArg;

/* "Kstage"'s work. */
typedef struct KstageW {
    /* 0x00 */ PolFrame *top;
    /* 0x04 */ PolFrame *bottom;
    /* 0x08 */ u8 pad08[0x10];
    /* 0x18 */ s32 alpha;
} KstageW;

extern u8 bkasKstageTopDl[]; /* the stage's two display lists, in file 0xF5 */
extern u8 bkasKstageBottomDl[];

/* "Kstage": a stage in two parts from file 0xF5, twice life size, at the position given, the
 * lower part 1000 below. Each frame the lower part follows the boss down (700 below it) but
 * never rises above the upper part. It ends on either pilot's start button, or when the battle
 * sets gBattleFlags2 8 or 0x400000. */
s32 bkasKstageProc(Obj *obj, s16 msg, void *arg) {
    KstageW *w = (KstageW *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Kstage";
            w->top = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->top == NULL) {
                PRINTF("[demoStageCreate]隊長,タスクの作成に失敗しました。\n");
                return -1;
            }
            w->bottom = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->bottom == NULL) {
                return -1;
            }
            bossSetPolDL(w->top, (Gfx *) bkasKstageTopDl, 0xF5);
            bossSetFrameFlags((AnimeFrame *) w->top, 0x20);
            w->top->scale[0] = w->top->scale[1] = w->top->scale[2] = 2.0f;
            bossSetPolDL(w->bottom, (Gfx *) bkasKstageBottomDl, 0xF5);
            bossSetFrameFlags((AnimeFrame *) w->bottom, 0x20);
            w->bottom->scale[0] = w->bottom->scale[1] = w->bottom->scale[2] = 2.0f;
            w->top->pos[0] = ((KstageArg *) arg)->pos[0];
            w->top->pos[1] = ((KstageArg *) arg)->pos[1];
            w->top->pos[2] = ((KstageArg *) arg)->pos[2];
            w->bottom->pos[0] = ((KstageArg *) arg)->pos[0];
            w->bottom->pos[1] = ((KstageArg *) arg)->pos[1] - 1000.0f;
            w->bottom->pos[2] = ((KstageArg *) arg)->pos[2];
            w->alpha = 255;
            break;
        case 3:
            break;
        case 1:
            frmDestroy(&w->top->f);
            frmDestroy(&w->bottom->f);
            break;
        case 0:
            if (joyGetHeldTime(gCockpitPort, gCockpitBtnStart) == 1
                || joyGetHeldTime(gBrotherPort, gBrotherBtnStart) == 1) {
                return -1;
            }
            if (gBattleFlags2 & 8) {
                return -1;
            }
            if (gBattleFlags2 & 0x400000) {
                return -1;
            }
            w->bottom->pos[1] = gBossPosY - 700.0f;
            if (w->top->pos[1] < w->bottom->pos[1]) {
                w->bottom->pos[1] = w->top->pos[1];
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "Kstage" at x, y, z under the effect task. */
void demoStageCreate(f32 x, f32 y, f32 z) {
    KstageArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bkasKstageProc, gBattleTask->file, &arg);
}

/* "Klift"'s (bkasKliftProc) argument: only what its starter sets. */
typedef struct KliftArg {
    /* 0x00 */ u8 pad00[0x8];
    /* 0x08 */ f32 pos[3];
    /* 0x14 */ u8 pad14[0xC];
    /* 0x20 */ s32 type;
} KliftArg;

/* "Klift"'s work. */
typedef struct KliftW {
    /* 0x00 */ PolFrame *beam;
    /* 0x04 */ PolFrame *pool; /* no display list of its own */
    /* 0x08 */ u8 pad08[0x10];
    /* 0x18 */ s32 alpha;
    /* 0x1C */ u8 pad1C[0x4];
    /* 0x20 */ s32 type;
} KliftW;

extern u8 bkasKliftLightDl[]; /* the light's display list, in file 0xF5 */

/* "Klift" (the source names it demoLight): a yellow light from file 0xF5 at the position given,
 * 20 x 10 x 20, turned half round for type 1, with a second polygon 40 times life size. It
 * fades from 255 by one every fourth frame and ends at 0, on either pilot's start button, or
 * when the battle sets gBattleFlags2 8 or 0x400000. */
s32 bkasKliftProc(Obj *obj, s16 msg, void *arg) {
    KliftW *w = (KliftW *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Klift";
            w->beam = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->beam == NULL) {
                PRINTF("[demoLightCreate]隊長,タスクの作成に失敗しました。\n");
                return -1;
            }
            w->pool = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->pool == NULL) {
                return -1;
            }
            w->type = ((KliftArg *) arg)->type;
            bossSetPolDL(w->beam, (Gfx *) bkasKliftLightDl, 0xF5);
            bossSetFrameFlags((AnimeFrame *) w->beam, 0x20);
            w->beam->scale[0] = 20.0f;
            w->beam->scale[1] = 10.0f;
            w->beam->scale[2] = 20.0f;
            bossSetFrameFlags((AnimeFrame *) w->pool, 0x20);
            w->pool->scale[0] = w->pool->scale[1] = w->pool->scale[2] = 40.0f;
            w->beam->pos[0] = ((KliftArg *) arg)->pos[0];
            w->beam->pos[1] = ((KliftArg *) arg)->pos[1];
            w->beam->pos[2] = ((KliftArg *) arg)->pos[2];
            w->pool->pos[0] = ((KliftArg *) arg)->pos[0];
            w->pool->pos[1] = ((KliftArg *) arg)->pos[1];
            w->pool->pos[2] = ((KliftArg *) arg)->pos[2];
            if (w->type == 1) {
                w->beam->rot[1] = 3.1415927f;
            }
            w->alpha = 255;
            break;
        case 1:
            frmDestroy(&w->beam->f);
            frmDestroy(&w->pool->f);
            break;
        case 0:
            if (!(gGameFrameCount & 3)) {
                w->alpha--;
            }
            if (w->alpha < 0) {
                w->alpha = 0;
                return -1;
            }
            if (joyGetHeldTime(gCockpitPort, gCockpitBtnStart) == 1
                || joyGetHeldTime(gBrotherPort, gBrotherBtnStart) == 1) {
                return -1;
            }
            if (gBattleFlags2 & 8) {
                return -1;
            }
            if (gBattleFlags2 & 0x400000) {
                return -1;
            }
            break;
        case 3:
            w->beam->dl = bossPrimEnvColorDL(255, 255, 0, 255, 255, 255, w->alpha);
            w->pool->dl = bossPrimEnvColorDL(255, 255, 0, 255, 255, 255, w->alpha);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "Klift" of a type at x, y, z under the effect task. */
void demoLightCreate(f32 x, f32 y, f32 z, s32 type) {
    KliftArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.type = type;
    tskCreate(gEffectTask, bkasKliftProc, gBattleTask->file, &arg);
}

/* "Kpaper"'s (bkasKpaperProc) argument: only the position its starter sets. */
typedef struct KpaperArg {
    /* 0x00 */ u8 pad00[0x4];
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ u8 pad10[0x38];
} KpaperArg;

/* "Kpaper"'s work. */
typedef struct KpaperW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ Vec3f base; /* where it was started */
    /* 0x10 */ f32 pivot[3]; /* the swing's centre */
    /* 0x1C */ f32 swing[3]; /* the point on the swing */
    /* 0x28 */ f32 phase;
    /* 0x2C */ f32 phaseStep;
    /* 0x30 */ f32 radius;
    /* 0x34 */ f32 angle;
    /* 0x38 */ u8 pad38[0x4];
    /* 0x3C */ s32 alpha;
    /* 0x40 */ s32 state;
} KpaperW;

#define cosf (*cosf)
#define sinf (*sinf)

extern u8 bkasKpaperDl[]; /* the paper's display list, in file 0xF5 */

/* "Kpaper" (the source names it demoPaper): a piece of paper from file 0xF5, half life size,
 * tumbling as it falls. It hangs from a pivot at a random radius (20-119) that sinks 2 a frame,
 * starting 30 degrees to one side or the other, and swings 60 degrees each way, faster for a
 * shorter radius. It ends 500 below the start, or when the battle sets gBattleFlags2 8. Its x
 * and z both add the swing's x, as the game does. The code's order is the game's; the float
 * registers differ from the position sums on (225 lines; base + (pivot + swing) is 237).
 * Sweep 2026-10-11: pad[11] (a word more, the same frame) puts the temp at the game's 0x20: 217.
 * 2026-10-12, 60 to 29 (one check each): base read as ((Vec3f *) w->base)->x (IDO_NOTES
 * 273, FAKE) fixes the outer add (84, inner then swapped); swing + pivot written swing first,
 * the sums the game's (78); the height read into a local y before the -500 test (29; one pad
 * word gave its slot). Left: y takes f0 and the 3-degree double f2, the game the reverse
 * (y declared first, "(y = ..) < -500" 29; the double through an f64 local 31); the state-0
 * if/else's then-arm jumps to a separate "b end; nop" join in the game, here both arms go to
 * the end (length -1; a ternary 62; an empty if on r after it emits a test, +2; a dead flag
 * set in the else and read by an empty if +4; PRINTF after it, do {} while (0) round it: 60
 * then; a probe: a nested switch, a goto, an empty "if (len & 1)" or
 * "if (len)" after it, a dead load or product, "if (0) g();" -- none gives the join); and the
 * game's case 1 and 2 "state = 3" temps are one t-register later (t8/t1 to t7/t8; joining
 * pivot[2] or phase to its line: no change).
 * 2026-10-12 (second try): "if (w->state) {}" after the state-0 if/else (FAKE, diagnostic): 29,
 * length +3, but it gives the game's join (then-arm "b join") and spends t7, so case 1's 3 takes
 * the game's t8 (case 2 t0, game t1): the game's join held a reload of state (lw t7) that
 * something later deleted (IDO_NOTES 274's as1 forwarding?). Its test stays here.
 * 2026-10-12 (third try): a bare "w->state;" after the if/else: 29, the same (uopt drops it). The
 * permuter (680 from its base) only moved case 2's state/phase stores or doubled a store.
 * Without y (the test on w->frame->pos[1]): 78, the double back in the game's f0 but the load in
 * f4 (game f2). f2 holds nothing else in the game's code, so y is a coloured variable that loses
 * f0 to the double: Chow's priority, y's must be lower. y reused in state 3 for swing[1]
 * ("y = sinf(..) * radius + pivot[1]; swing[1] = y;"): 29 to 26, y takes the game's f2 and the
 * double f0, but state 3's swing temps then shift (the game has no f2 there). Kept for now.
 * 2026-10-12 (fourth try): y (not reused in state 3) a block local of case 0 round its test: 29,
 * the same as at function level. In place of the reuse, a FAKE empty "if (y) {}" (it emits
 * nothing; shopRoomBGProc's lever) after the -500 test, at state 0's head or at state 3's head:
 * 24, the head the game's (y f2, the double f0); state 3's swing temps differ (radius f8, pivot
 * f4, "pivot + product": game f4, f6, "product + pivot"); at
 * state 3's end: 32 (y spilled, +7). A no-op (f32) cast on state 3's product: 26, the same.
 * Left: the join's "b; nop", cases 1/2's "state = 3" scheduled first, state 3's float temps.
 * 2026-10-12 (fifth try), 24 to 9: state 3's two swing sums written "pivot + cos * radius"
 * (IDO_NOTES 378 E8's operand order: the game's "product + pivot" registers and order) 13;
 * cases 1/2's "phase = ..; state = 3;" in that order (the game stores state last) 9. Left:
 * the join's "b; nop" and the integer temps: every state case spends one t more in the game
 * than here (case 1's 3 t8 not t7, case 2's t1 not t8, msg 3's t4 not t0). Tried at 9: a
 * "default: break;" right after case 0 (9, threaded the same); the y FAKE removed (14); the
 * "if (w->state) {}" diagnostic after case 0's if/else (13, length +3: the join, and case 1's
 * t8, but case 2 t0, msg 3 t1); reloads sweep (nothing better). Permuter from the 9 form
 * (--stack-diffs, 20 min, ~1,000 iterations, base 190): nothing better.
 * 2026-10-12 (sixth try), trace + ugen-edit: proven
 * target, real 0 (exits4.txt): ONE dead int load in a ugen t-register at the end of each of the
 * inner switch's four exits (case 0's if/else join, cases 1, 2, 3 before "b end"); as1 deletes
 * each, leaving case 0's "b join; join: b end; nop", and each later temp moves one t on
 * (cases 1/2's 3 t8/t1, msg 3's t4: the round-robin is t0-t7, t8, without t9). The join's load
 * alone (joinload.txt): real 6, the join right. So the game's C has something at every inner
 * break that ugen evaluates into a temp and discards (not "if (w->state) {}" after the inner
 * switch: 18, length +4, it is emitted).
 * 2026-10-12 (seventh try), 9 to MATCH (check: "MATCH except own section addresses"): the proof's
 * four dead t-values made with IDO_NOTES 284's never-true unsigned test. Read in trace's full.s
 * (ugen's listing): ugen emits "addu tN, s0, 1; bltu tN, 0, end" for the test and the object
 * has neither (as1 deletes the never-taken branch, then the dead addu), but the deleted branch
 * still ends an as1 block, so where it sits decides what as1 can schedule across it. Tries:
 * "(u32) w->alpha < 0" alone: 9 (no code reaches ugen: dropped before it). At the join,
 * "if ((u32) w->alpha + 1 < 0) {}" or "{ break; }": 6, length +2 (ugen inverts it to an
 * always-taken "bgeu" plus a dead "b", which as1 keeps; and the load + add spend two t's);
 * "if ((u32) w + 1 >= 0) break;" at the join: real 4, length 0 (bltu never taken, one t: the
 * join right, case 1's 3 in t8). The same ">= 0" form at cases 1/2/3's ends: 6, length +3
 * (the deleted branch splits the last store block from "b end": swc1 stays out of the slot).
 * "< 0" before the last store block of cases 1/2/3: 8 (spent before case 1's li 3: t0). The
 * fix is the t count, not the place: one t between case 1's li 3 and case 2's (case 2's head:
 * real 6 with case 3's and msg 3's heads, the t's all right; but at case 3's head the beql
 * loses its slot load and at msg 3's head the dispatch beq's slot changes). In case 3 there is
 * no block-safe point: the game schedules across every statement boundary (the phase/pivot
 * loads, the sinf arg above the swing[0] store, the angle load above the swing[1] store; the
 * test before the angle line: real 2, that lwc1 two lines late). Two tests "(u32) w + 1" in a
 * row are one CSE in v0 (no t spent). Case 3's sinf result through y with the test after the
 * call: t's right but swing[1]'s float temps swapped (6; written either order or with an
 * (f32) cast, the same: the variable form loads pivot first). Msg 3 split at its call (a
 * boundary right after a call moves nothing): "dl = bossCombineEnvColorDL(..); if ((u32)
 * w->alpha + 1 < 0) break; w->frame->dl = dl;": the code all right but a new Gfx * local adds
 * a frame word (pad[8]: real 2, sinf's spill at sp+36); through the existing int r: MATCH.
 * Trap: "trace" deletes its working files between runs, so the sixth try's exits4.txt,
 * joinload.txt and ugen_diff.txt are gone (their edit: "lw tN, 64($16)" before each exit). */
s32 bkasKpaperProc(Obj *obj, s16 msg, void *arg) {
    KpaperW *w = (KpaperW *) FX(obj);
    s32 pad[10]; /* FAKE: the pads sweep's */
    f32 y;
    s32 r;
    s32 len;

    switch (msg) {
        case 2:
            obj->name = "Kpaper";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                PRINTF("[demoPaperCreate]隊長,タスクの作成に失敗しました。\n");
                return -1;
            }
            if (gBattleFlags2 & 8) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bkasKpaperDl, 0xF5);
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x20);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 0.5f;
            w->base.x = ((KpaperArg *) arg)->pos[0];
            w->base.y = ((KpaperArg *) arg)->pos[1];
            w->base.z = ((KpaperArg *) arg)->pos[2];
            w->frame->pos[0] = ((KpaperArg *) arg)->pos[0];
            w->frame->pos[1] = ((KpaperArg *) arg)->pos[1];
            w->frame->pos[2] = ((KpaperArg *) arg)->pos[2];
            w->alpha = 255;
            w->state = 0;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (gBattleFlags2 & 8) {
                return -1;
            }
            w->frame->rot[0] += 0.052359879011640235;
            w->frame->rot[1] += 0.052359879011640235;
            w->frame->pos[0] = w->swing[0] + w->pivot[0] + w->base.x;
            w->frame->pos[1] = w->swing[1] + w->pivot[1] + w->base.y;
            w->frame->pos[2] = w->swing[0] + w->pivot[2] + w->base.z;
            y = w->frame->pos[1];
            if (y < -500.0f) {
                return -1;
            }
            if (y) { /* FAKE: an empty test: y's range reaches the switch, so the 3-degree double
                      * outranks it for f0 as in the game (IDO_NOTES 378 E5's empty test) */
            }
            switch (w->state) {
                case 0:
                    r = fadeRand(100);
                    len = r + 20;
                    w->radius = len;
                    w->phaseStep = 6.2831855f / (r * 10 + 240);
                    if (len & 1) {
                        w->state = 1;
                    } else {
                        w->state = 2;
                    }
                    /* FAKE: never false (unsigned) -- ugen spends t7 on w + 1 and its
                     * never-taken "bltu" over the break; as1 deletes both, leaving the
                     * game's "b join; join: b end; nop" (IDO_NOTES 284) */
                    if ((u32) w + 1 >= 0) {
                        break;
                    }
                    break;
                case 1:
                    w->pivot[0] = w->swing[0] - cosf(-0.5235988f) * w->radius;
                    w->pivot[1] = w->swing[1] - sinf(-0.5235988f) * w->radius;
                    w->pivot[2] = w->swing[2];
                    w->phase = 1.5707964f;
                    w->state = 3;
                    break;
                case 2:
                    /* FAKE: never true; spends one ugen t-register (case 2's 3 takes the
                     * game's t1), as1 deletes the code (IDO_NOTES 284) */
                    if ((u32) w + 1 < 0) {
                        break;
                    }
                    w->pivot[0] = w->swing[0] - cosf(-2.6179938f) * w->radius;
                    w->pivot[1] = w->swing[1] - sinf(-2.6179938f) * w->radius;
                    w->pivot[2] = w->swing[2];
                    w->phase = -1.5707964f;
                    w->state = 3;
                    break;
                case 3:
                    w->pivot[1] -= 2.0f;
                    w->phase += w->phaseStep;
                    w->angle = sinf(w->phase) * 1.0471976f - 1.5707964f;
                    w->swing[0] = w->pivot[0] + cosf(w->angle) * w->radius;
                    w->swing[1] = w->pivot[1] + sinf(w->angle) * w->radius;
                    w->angle += 0.7853982f;
                    break;
            }
            break;
        case 3:
            /* FAKE: the result through r and a never-true test after the call: the load and
             * add spend two t-registers (the frame load takes the game's t4) and as1 deletes
             * them; a boundary right after the call moves nothing (IDO_NOTES 284). r, not
             * a Gfx * local: a new pointer local cost a frame word. */
            r = (s32) bossCombineEnvColorDL(0, 0, 0, w->alpha);
            if ((u32) w->alpha + 1 < 0) {
                break;
            }
            w->frame->dl = (Gfx *) r;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 bkasKpaperProc(Obj *obj, s16 msg, void *arg);

/* A "Kpaper" at x, y, z under the effect task. */
void demoPaperCreate(f32 x, f32 y, f32 z) {
    KpaperArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bkasKpaperProc, gBattleTask->file, &arg);
}

extern u8 bkasSensuRAnim[]; /* the second fan's animation, in file 0xF5 */

/* "SensuR" (the second fan): as "Sensu" (bkasSensuProc) but life size, turned by its
 * type in steps of 45 degrees, with its damage box (200 by 200) 300 up. It flies at speed 5 to
 * 50 below the Impact, turning 2 degrees a frame, and its puff is at its tip, 400 along its
 * turn. It ends at z 500, or when the battle ends or sets gBattleFlags2 0x400. */
s32 bkasSensuRProc(Obj *obj, s16 msg, void *arg) {
    SensuW *w = (SensuW *) FX(obj);
    s32 kind;
    f32 dx;
    f32 dy;

    switch (msg) {
        case 2:
            obj->name = "SensuR";
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                PRINTF("隊長,タスクの作成に失敗しました。\n");
                return -1;
            }
            bossSetAnimAt(w->frame, bkasSensuRAnim, 0xF5, 0.0f);
            bossSetFrameFlags2(w->frame, 0x20);
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 100, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x51;
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->frame->f, 200, 200, 300, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 0x52;
            w->frame->pos[0] = ((SensuArg *) arg)->pos[0];
            w->frame->pos[1] = ((SensuArg *) arg)->pos[1];
            w->frame->pos[2] = ((SensuArg *) arg)->pos[2];
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 1.0f;
            w->type = ((SensuArg *) arg)->type;
            if (w->type == 0) {
                w->frame->rot[2] = 0.0f;
            }
            if (w->type == 1) {
                w->frame->rot[2] = 0.7853982f;
            }
            if (w->type == 2) {
                w->frame->rot[2] = 1.5707964f;
            }
            if (w->type == 3) {
                w->frame->rot[2] = 2.3561945f;
            }
            if (w->type == 4) {
                w->frame->rot[2] = 3.1415927f;
            }
            if (w->type == 5) {
                w->frame->rot[2] = 3.926991f;
            }
            if (w->type == 6) {
                w->frame->rot[2] = 4.712389f;
            }
            if (w->type == 7) {
                w->frame->rot[2] = 5.4977875f;
            }
            BmoveX(obj, gImpactPosX, gImpactPosY - 50.0f, gImpactPosZ, 5.0f);
            w->state = 0;
            break;
        case 3:
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->atk->frame);
            frmDestroy(&w->dmg->frame);
            break;
        case 0:
            if (gBattleFlags & 0x10) {
                return -1;
            }
            if (gBattleFlags2 & 0x4000) {
                return -1;
            }
            if (gBattleFlags2 & 0x400) {
                return -1;
            }
            if (acollDmgWasHit(w->dmg)
                && ((kind = w->dmg->hitKind) == 15 || kind == 1 || kind == 3 || kind == 5 || kind == 8
                    || kind == 11)) {
                if (kind == 1 || kind == 3 || kind == 5 || kind == 8) {
                    ryouCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
                }
                bossHeadingOffset(w->frame->rot[2], 0.0f, -400.0f, &dx, &dy);
                itexPuff(w->frame->pos[0] + dx, w->frame->pos[1] - dy, w->frame->pos[2], 1.0f);
                if (!(gBattleFlags & 0x10)) {
                    SoundLocation2(0x228, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                   300.0f, 15000.0f, gBattleCamView->fovy);
                }
                return -1;
            }
            if (acollAtkWasHit(w->atk) == 1) {
                bossHeadingOffset(w->frame->rot[2], 0.0f, -400.0f, &dx, &dy);
                itexPuff(w->frame->pos[0] + dx, w->frame->pos[1] - dy, w->frame->pos[2], 1.0f);
                return -1;
            }
            if (w->frame->pos[2] > 500.0f) {
                return -1;
            }
            switch (w->state) {
                case 0:
                    if (bossStepAnim(w->frame, 0.3f, 2)) {
                        w->state++;
                    }
                    break;
                case 1:
                    w->frame->rot[2] -= 0.03490658600776016;
                    bossApplyVel2(obj);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "SensuR" (bkasSensuRProc) of a type at x, y, z under the effect task. */
void bkasSensuRCreate(f32 x, f32 y, f32 z, s32 type) {
    SensuArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.type = type;
    tskCreate(gEffectTask, bkasSensuRProc, gBattleTask->file, &arg);
}

/* The second "kabukiDB"'s (bkasKabukiDB2Proc) argument: only what its starter sets. */
typedef struct KabukiDB2Arg {
    /* 0x00 */ u8 pad00[0x18];
    /* 0x18 */ f32 pos[3];
    /* 0x24 */ f32 target[3];
    /* 0x30 */ f32 scale; /* as kabukiDB's; this task sets its own (20) and never reads it */
    /* 0x34 */ u8 pad34[0x20];
    /* 0x54 */ s32 type;
} KabukiDB2Arg;

/* The second "kabukiDB"'s work. */
typedef struct KabukiDB2W {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3]; /* bossApplyVel's (bmove.c) */
    /* 0x10 */ ACollFrame *atk;
    /* 0x14 */ ACollFrame *dmg;
    /* 0x18 */ f32 start[3];
    /* 0x24 */ f32 target[3];
    /* 0x30 */ f32 scale;
    /* 0x34 */ u8 pad34[0x4];
    /* 0x38 */ s32 armorBroken; /* set once either of the Impact's armours is gone */
    /* 0x3C */ s32 life;
    /* 0x40 */ s32 noHit; /* frames the boxes stay off */
    /* 0x44 */ s32 time;
    /* 0x48 */ s32 alpha;
    /* 0x4C */ u32 step; /* 0: flying to the centre; 3: stopped; 6: to the boss; 7: hit */
    /* 0x50 */ s32 speed;
    /* 0x54 */ s32 type; /* 1: flashes red, else blue */
} KabukiDB2W;

/* The second "kabukiDB", the "true kabuki ball" (真カブキ弾, as its prints call it): the first's
 * spark, size 20, with the same boxes. It flies to the centre at speed 10 and bursts there in a
 * small white flash. It takes ten hits: each of hit kinds 1, 3, 5, 8 costs one, sends it back
 * 2000 in z, faster by 5, with its boxes off for 5 frames; hit kind 11 (the Impact's cannon)
 * bursts it at once, and landing its attack bursts it in a white flash. Once either of the
 * Impact's armours is gone it stops. The game keeps a test of the hit kind against 1 around a
 * dead print (a beql to the next instruction): an empty if folds it, so a dead flag set in the
 * print's arm and read by an empty if keeps it (FAKE; a probe, `l`, the only
 * one of six bodies that kept it). Matched 2026-10-12 from real 24 with that, kind
 * reused for the noHit test (its load takes kind's v0, as the game's), and case 7's speed
 * bump before its two stores (its temps are numbered first).
 * A kept FAKE (2026-10-17): the flag is set by the test and its print is under "if (flag)" (the
 * same string, the same .rodata place): MATCH, object identical, in place of the FAKE dead
 * flag in the print's arm and its empty reader. */
s32 bkasKabukiDB2Proc(Obj *obj, s16 msg, void *arg) {
    KabukiDB2W *w = (KabukiDB2W *) FX(obj);
    s32 kind;
    s32 flag; /* a hit other than kind 1: its print */

    switch (msg) {
        case 2:
            obj->name = "kabukiDB";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bkasSparkDl, 0x120);
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 15, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x56;
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->frame->f, 16, 32, -16, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 0x57;
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x22);
            w->scale = 20.0f;
            w->start[0] = ((KabukiDB2Arg *) arg)->pos[0];
            w->start[1] = ((KabukiDB2Arg *) arg)->pos[1];
            w->start[2] = ((KabukiDB2Arg *) arg)->pos[2];
            w->target[0] = ((KabukiDB2Arg *) arg)->target[0];
            w->target[1] = ((KabukiDB2Arg *) arg)->target[1];
            w->target[2] = ((KabukiDB2Arg *) arg)->target[2];
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = fadeRandRange(5, 15) / 10.0f * w->scale;
            w->frame->pos[0] = w->start[0];
            w->frame->pos[1] = w->start[1];
            w->frame->pos[2] = w->start[2];
            w->type = ((KabukiDB2Arg *) arg)->type;
            w->alpha = 255;
            w->time = 0;
            w->life = 10;
            w->speed = 10;
            w->step = 0;
            break;
        case 0:
            if (gBattleFlags & 0x10) {
                return -1;
            }
            if (gBattleFlags2 & 0x4000) {
                return -1;
            }
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = fadeRandRange(9, 11) / 10.0f * w->scale;
            w->time++;
            bossApplyVel(obj);
            if (--w->noHit <= 0) {
                w->atk->flags |= 2;
                w->dmg->flags |= 2;
                w->noHit = 0;
            }
            if (w->armorBroken == 0 && (gImpactArmor <= 0 || gImpactArmor2 <= 0)) {
                w->armorBroken = 1;
                w->step = 3;
            }
            if ((kind = w->noHit) != 0) {
                PRINTF("無敵中です\n");
            } else {
                PRINTF("弾体力[%d]\n", w->life);
                if (acollAtkWasHit(w->atk) == 1) {
                    itexPuff(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 1.0f);
                    beFlashFCreate(255, 255, 255, 240);
                    PRINTF("粉砕しました。\n");
                    return -1;
                }
                if (acollDmgWasHit(w->dmg)) {
                    kind = w->dmg->hitKind;
                    if (kind == 11) {
                        PRINTF("めが砲にあたった。\n");
                        itexPuff(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 1.0f);
                        w->atk->flags &= ~2;
                        w->dmg->flags &= ~2;
                        return -1;
                    }
                    if (kind == 1 || kind == 3 || kind == 5 || kind == 8) {
                        itexPuff(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 1.0f);
                        if (!(gBattleFlags & 0x10)) {
                            SoundLocation2(0x290, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                           w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                        }
                        flag = 0;
                        if ((kind = w->dmg->hitKind) != 1) {
                            flag = 1;
                        }
                        if (flag) {
                            PRINTF("粉砕されました。\n");
                        }
                        w->atk->flags &= ~2;
                        w->dmg->flags &= ~2;
                        w->life--;
                        w->step = 7;
                    }
                }
            }
            if (w->life <= 0) {
                itexPuff(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 2.0f);
                return -1;
            }
            switch (w->step) {
                case 0:
                    if (BmoveX(obj, 0.0f, 0.0f, 0.0f, w->speed) == 0) {
                        itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                        beFlashFCreate(255, 255, 255, 30);
                        PRINTF("目標に到達しました。\n");
                        return -1;
                    }
                    break;
                case 6:
                    if (BmoveX(obj, gBossPosX, gBossPosY, gBossPosZ, w->speed) == 0) {
                        return -1;
                    }
                    break;
                case 7:
                    w->frame->pos[2] -= 2000.0f;
                    w->speed += 5;
                    w->noHit = 5;
                    w->step = 0;
                    break;
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                default:
                    PRINTF("真カブキ弾ステップ[%d]\n", w->step);
                    break;
            }
            break;
        case 3:
            if (w->type == 1) {
                if (gGameFrameCount % 5 == 0) {
                    w->frame->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 255, w->alpha);
                } else {
                    w->frame->dl = bossPrimEnvColorDL(255, 255, 255, 255, 0, 0, w->alpha);
                }
            } else if (gGameFrameCount % 5 == 0) {
                w->frame->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 255, w->alpha);
            } else {
                w->frame->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 0, w->alpha);
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->atk->frame);
            frmDestroy(&w->dmg->frame);
            PRINTF("隊長,タスクの作成に失敗しました。\n");
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 bkasKabukiDB2Proc(Obj *obj, s16 msg, void *arg);

/* The second "kabukiDB" at x, y, z under the effect task. */
void bkasKabukiDB2Create(f32 x, f32 y, f32 z, f32 a, f32 b, f32 c, f32 d, s32 n) {
    KabukiDB2Arg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.target[0] = a;
    arg.target[1] = b;
    arg.target[2] = c;
    arg.scale = d;
    arg.type = n;
    tskCreate(gEffectTask, bkasKabukiDB2Proc, gBattleTask->file, &arg);
}

/* "Bappcam", this boss's entrance: a letterboxed demo through timed shots. The camera starts
 * low beside the stage, follows the boss, then cuts between set shots at frames 1261-1601. At
 * 1601 it starts scenario 5 (the boss's introduction) and holds until 1781. Then it goes back
 * behind the Impact, opens the letterbox and ends at frame 1880, or at once on gBattleFlags2
 * 0x8000 (cleared). Step 10 changes its flags in the order of tlb_0327_1's Bappcam (BF, BF,
 * BF2, Brother, BF2).
 * Matched 2026-10-11; it was parked for a cached function pointer's slot (pad[10] puts it at
 * the game's 0x24) and step 10's flag order. */
s32 bkasBappcamProc(Obj *obj, s16 msg, void *arg) {
    BCam *w = BCAM(obj);
    s32 pad[10];

    switch (msg) {
        case 2:
            obj->name = "Bappcam";
            fadeCreate(gEffectTask);
            fadeSetParent(gFramePool10);
            w->cam = (Camera *) tskCreate(obj, dracam, 0, NULL);
            if (w->cam == NULL) {
                return -1;
            }
            camSetFov(w->cam, 0.6981319795f);
            camSetSpeed(w->cam, 60.0f);
            camSetEye(w->cam, 824.0f, -120.0f, 80.0f);
            camSetAt(w->cam, 360.0f, -428.0f, -1600.0f);
            camGetView(w->cam)->zFar = 20000.0f;
            gBattleFlags |= 0x10000000;
            gBattleFlags |= 0x1000000;
            gBrotherFlags |= 0x10000;
            fadeSetLetterbox(320, 120, 0, 120);
            w->on = 1;
            w->step = 0;
            w->timer = 0;
            break;
        case 1:
            frmDestroy(w->frame);
            frmDestroy(w->frames[0]);
            frmDestroy(w->frames[1]);
            frmDestroy(w->frames[2]);
            break;
        case 0:
            camGetView(w->cam);
            if (gBattleFlags2 & 0x8000) {
                gBattleFlags2 &= ~0x8000;
                return -1;
            }
            PRINTF("\rデモタイマーstep/time[%d/%d]", w->step, w->timer);
            switch (w->step) {
                case 0:
                    fadeMoveLetterbox(50, 190, 1);
                    if (w->timer > 800) {
                        w->step++;
                    }
                    break;
                case 1:
                    camMoveAt(w->cam, gBossPosX, gBossPosY, gBossPosZ);
                    if (w->timer > 1260) {
                        camSetSpeed(w->cam, 60.0f);
                        camMoveFov(w->cam, 0.4363319873f);
                        camMoveEye(w->cam, 550.0f, -402.0f, -2674.0f);
                        camMoveAt(w->cam, -66.0f, 140.0f, -3844.0f);
                        w->step++;
                    }
                    break;
                case 2:
                    if (w->timer > 1320) {
                        camSetSpeed(w->cam, 60.0f);
                        camMoveFov(w->cam, 0.2094389939f);
                        camMoveEye(w->cam, 2381.0f, -96.0f, -258.0f);
                        camMoveAt(w->cam, 1319.0f, 0.0f, -1600.0f);
                        w->step++;
                    }
                    break;
                case 3:
                    if (w->timer > 1380) {
                        camSetSpeed(w->cam, 30.0f);
                        camSetFov(w->cam, 0.1745319965f);
                        camSetEye(w->cam, 431.0f, 1554.0f, 872.0f);
                        camSetAt(w->cam, 169.0f, 60.0f, -1620.0f);
                        camMoveEye(w->cam, 431.0f, -66.0f, 872.0f);
                        w->step++;
                    }
                    break;
                case 4:
                    if (w->timer > 1540) {
                        camSetFov(w->cam, 0.6981319795f);
                        camSetEye(w->cam, 1020.0f, -808.0f, -2324.0f);
                        camSetAt(w->cam, 356.0f, -532.0f, -3623.0f);
                        w->step++;
                    }
                    break;
                case 5:
                    if (w->timer > 1555) {
                        camSetFov(w->cam, 0.5934110287f);
                        camSetEye(w->cam, 564.0f, -576.0f, -5776.0f);
                        camSetAt(w->cam, 10.0f, 580.0f, -1599.0f);
                        w->step++;
                    }
                    break;
                case 6:
                    if (w->timer > 1570) {
                        camSetFov(w->cam, 0.6981319795f);
                        camSetEye(w->cam, -904.0f, -12.0f, -2676.0f);
                        camSetAt(w->cam, 344.0f, 68.0f, -3603.0f);
                        w->step++;
                    }
                    break;
                case 7:
                    if (w->timer > 1585) {
                        camSetFov(w->cam, 1.4311709405f);
                        camSetEye(w->cam, -872.0f, -892.0f, -4384.0f);
                        camSetAt(w->cam, 2968.0f, 1028.0f, -1599.0f);
                        w->step++;
                    }
                    break;
                case 8:
                    if (w->timer > 1600) {
                        scRunStart(5, 0, 0);
                        camSetFov(w->cam, 0.1570789965f);
                        camSetEye(w->cam, -120.0f, 442.0f, 0.0f);
                        camSetAt(w->cam, -122.0f, 300.0f, -1600.0f);
                        w->step++;
                    }
                    break;
                case 9:
                    if (w->timer > 1780) {
                        w->step++;
                    }
                    break;
                case 10:
                    camSetSpeed(w->cam, 10.0f);
                    camSetFov(w->cam, 0.34906587f);
                    camSetEye(w->cam, 0.0f, 0.0f, 0.0f);
                    camSetAt(w->cam, 0.0f, 0.0f, -300.0f);
                    gBattleFlags &= ~0x10000000;
                    gBattleFlags &= ~0x1000000;
                    gBattleFlags2 &= ~0x100;
                    gBrotherFlags &= ~0x10000;
                    gBattleFlags2 |= 0x80000;
                    w->step++;
                    break;
                case 11:
                    fadeOpenLetterbox();
                    camMoveFov(w->cam, 0.87266463f);
                    camMoveEye(w->cam, 0.0f, sinf(gGameFrameCount * 0.0174532930038800785) * 3.0f, 0.0f);
                    camMoveAt(w->cam, gCockpitLookAtX, gCockpitLookAtY, gCockpitLookAtZ);
                    if (w->timer == 1880) {
                        return -1;
                    }
                    break;
                case 12:
                case 13:
                case 14:
                    break;
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
