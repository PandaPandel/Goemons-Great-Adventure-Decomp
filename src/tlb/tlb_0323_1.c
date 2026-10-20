#include "tlb.h"
#include "tlb/tlb_0323.h"

extern Obj *gBattleTask; /* the battle's task (beffect.c) */
extern CamView *gBattleCamView; /* the battle's camera (beffect.c) */
extern u8 bisuPlateDl[]; /* the plate's display list, in file 0xF4 */
extern u8 bisuBVaniDl[]; /* the vertex-animated model's display list, in file 0x2CA */
extern u8 bisuBVaniVAnime[]; /* its vertex animation, in file 0x2CA */
extern u8 bisuBVaniVAnimeStep[]; /* the same, as the steps name it (undefined_syms.txt) */

/* acollAtkAddYZ and acollDmgAddYZ return their box (main declares them void). */
typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                    s32 a);

#define cosf (*cosf)
#define sinf (*sinf)

#define DPRINTF

/* "Hbeam"'s work. */
typedef struct HbeamW {
    /* 0x00 */ PolFrame *frame; /* the heart */
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ u8 pad10[0x4];
    /* 0x14 */ PolFrame *glow; /* turning behind it */
    /* 0x18 */ ACollFrame *atk;
    /* 0x1C */ ACollFrame *dmg;
    /* 0x20 */ u8 pad20[0x4];
    /* 0x24 */ s32 alpha;
    /* 0x28 */ f32 distSq; /* from the boss */
} HbeamW;

void bisuHbeamECreate(f32 x, f32 y, f32 z);
void bisuHbeamPCreate(f32 x, f32 y, f32 z, f32 v);
extern u8 bisuHbeamFlashDl[]; /* the heart's display list, in file 0xF4 */
extern u8 bisuHbeamShardDl[]; /* the glow's display list, in file 0xF4 */

/* "Hbeam", the boss's heart beam: a heart (file 0xF4) five times life size with a glow turning
 * behind it, an attack box (radius 30, kind 0x15) and a damage box (30 by 50, kind 0x16). It
 * starts 200 in front of the boss (raised 200 with no arg) and heads for the impact point at 12,
 * re-aiming at 8 every 60 frames; it bobs as it goes and sparkles every 15 frames. Its sound
 * alternates between 0x2C9 and 0x2CA. Hit by kinds 15, 1, 3, 5, 8 or 11 it bursts (sound
 * 0x284; the last four drop money); touching a player it bursts (sound 0x228 unless the battle's
 * flag 0x10). It is gone 5000 from the boss, or when the battle ends it (flag 0x400). */
s32 bisuHbeamProc(Obj *obj, s16 msg, void *arg) {
    HbeamW *w = (HbeamW *) FX(obj);
    s32 kind;

    switch (msg) {
        case 2:
            obj->name = "Hbeam";
            if ((w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                return -1;
            }
            if ((w->glow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bisuHbeamFlashDl, 0xF4);
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 30, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 0x15;
            if ((w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->frame->f, 30, 50, -25, 0, 0)) ==
                NULL) {
                return -1;
            }
            w->dmg->kind = 0x16;
            bossSetPolDL(w->glow, (Gfx *) bisuHbeamShardDl, 0xF4);
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x27);
            bossHeadingOffset(gBossYaw, 0.0f, 200.0f, &w->frame->pos[0], &w->frame->pos[2]);
            w->frame->pos[0] += gBossPosX;
            w->frame->pos[2] += gBossPosZ;
            if (arg == NULL) {
                w->frame->pos[1] = gBossPosY + 200.0f;
            } else {
                w->frame->pos[1] = gBossPosY;
            }
            w->frame->scale[2] = 5.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->glow->scale[2] = 5.0f;
            w->glow->scale[0] = w->glow->scale[1] = w->glow->scale[2];
            BmoveX(obj, gImpactPosX, gImpactPosY, gImpactPosZ, 12.0f);
            w->alpha = 0xD0;
            if (!(gBattleSndToggles & 0x10)) {
                DPRINTF("ハートビーム:SD_POWA1をコールします。[%d]\n", 0x2C9);
                SoundLocation2(0x2C9, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
                gBattleSndToggles |= 0x10;
            } else {
                DPRINTF("ハートビーム:SD_POWA2をコールします。[%d]\n", 0x2CA);
                SoundLocation2(0x2CA, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
                gBattleSndToggles &= ~0x10;
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->glow->f);
            frmDestroy(&w->atk->frame);
            frmDestroy(&w->dmg->frame);
            break;
        case 0:
            if (gBattleFlags2 & 0x400) {
                return -1;
            }
            w->glow->pos[0] = w->frame->pos[0];
            w->glow->pos[1] = w->frame->pos[1];
            w->glow->pos[2] = w->frame->pos[2];
            w->glow->rot[1] += 0.13962634403104063;
            bossBobAccel(obj, 0.3f, 5.0f);
            if (gGameFrameCount % 15 == 0) {
                (*bisuHbeamECreate)(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2]);
            }
            if (gGameFrameCount % 60 == 0) {
                BmoveX(obj, gImpactPosX, gImpactPosY, gImpactPosZ, 8.0f);
            }
            if (acollDmgWasHit(w->dmg)
                && ((kind = w->dmg->hitKind) == 15 || kind == 1 || kind == 3 || kind == 5 || kind == 8
                    || kind == 11)) {
                itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                (*bisuHbeamPCreate)(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 4.0f);
                (*bisuHbeamPCreate)(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 4.0f);
                (*bisuHbeamPCreate)(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 4.0f);
                SoundLocation2(0x284, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
                kind = w->dmg->hitKind;
                if (kind == 1 || kind == 3 || kind == 5 || kind == 8) {
                    ryouCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
                }
                return -1;
            }
            if (acollAtkWasHit(w->atk) == 1) {
                itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                (*bisuHbeamPCreate)(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 4.0f);
                (*bisuHbeamPCreate)(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 4.0f);
                (*bisuHbeamPCreate)(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 4.0f);
                if (!(gBattleFlags & 0x10)) {
                    SoundLocation2(0x228, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                   300.0f, 15000.0f, gBattleCamView->fovy);
                }
                return -1;
            }
            w->distSq = bossDistSq(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], gBossPosX, gBossPosY,
                                   gBossPosZ);
            if (w->distSq > 2.5e7f) {
                return -1;
            }
            bossApplyVel(obj);
            break;
        case 3:
            w->frame->dl = bossPrimEnvColorDL(255, 255, 255, 255, 0x33, 0x7A, w->alpha);
            w->glow->dl = bossXluPrimEnvColorDL(0, 0, 0, 0x80);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "HbeamE"'s (bisuHbeamEProc) argument: only the position its starter sets. */
typedef struct HbeamEArg {
    /* 0x00 */ u8 pad00[0x8];
    /* 0x08 */ f32 pos[3];
    /* 0x14 */ u8 pad14[0x4];
} HbeamEArg;

s32 bisuHbeamEProc(Obj *obj, s16 msg, void *arg);

/* An "HbeamE" at x, y, z under the effect task. */
void bisuHbeamECreate(f32 x, f32 y, f32 z) {
    HbeamEArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bisuHbeamEProc, gBattleTask->file, &arg);
}

/* "HbeamE"'s work. */
typedef struct HbeamEW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[0x10];
    /* 0x14 */ s32 alpha;
} HbeamEW;

extern u8 bisuHbeamFlashDl[]; /* the flash's display list, in file 0xF4 */

/* "HbeamE" (the beam's end): a flash from file 0xF4 five times life size at the position
 * given, white on pink, that shrinks by 0.1 a frame until it is gone, fading from 128 by 2. It
 * ends at once on gBattleFlags2 0x400 or when the battle ends. */
s32 bisuHbeamEProc(Obj *obj, s16 msg, void *arg) {
    HbeamEW *w = (HbeamEW *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "HbeamE";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->pos[0] = ((HbeamEArg *) arg)->pos[0];
            w->frame->pos[1] = ((HbeamEArg *) arg)->pos[1];
            w->frame->pos[2] = ((HbeamEArg *) arg)->pos[2];
            bossSetPolDL(w->frame, (Gfx *) bisuHbeamFlashDl, 0xF4);
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x27);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 5.0f;
            w->alpha = 128;
            break;
        case 0:
            w->frame->scale[0] -= 0.1;
            w->frame->scale[1] -= 0.1;
            w->frame->scale[2] -= 0.1;
            if (w->frame->scale[0] < 0.0f) {
                return -1;
            }
            w->alpha -= 2;
            if (gBattleFlags2 & 0x400) {
                return -1;
            }
            if (gBattleFlags & 0x10) {
                return -1;
            }
            break;
        case 3:
            w->frame->dl = bossPrimEnvColorDL(255, 255, 255, 255, 0x33, 0x7A, w->alpha);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The starters below put x, y, z and one more value in their task's argument. */

typedef struct HbeamPArg {
    /* 0x00 */ u8 pad00[0x14];
    /* 0x14 */ f32 pos[3];
    /* 0x20 */ f32 scale;
    /* 0x24 */ u8 pad24[0x8];
} HbeamPArg;

s32 bisuHbeamPProc(Obj *obj, s16 msg, void *arg);

/* An "HbeamP" at x, y, z under the effect task. */
void bisuHbeamPCreate(f32 x, f32 y, f32 z, f32 v) {
    HbeamPArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = v;
    tskCreate(gEffectTask, bisuHbeamPProc, gBattleTask->file, &arg);
}

/* "HbeamP"'s work. */
typedef struct HbeamPW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ f32 fall; /* added to the fall speed each frame */
    /* 0x14 */ u8 pad14[0x10];
    /* 0x24 */ s32 alpha;
    /* 0x28 */ u8 spin; /* 1: one way, else the other */
    /* 0x29 */ u8 step; /* 0: thrown; 1, 2: bouncing; 3: falling through */
} HbeamPW;

extern u8 bisuHbeamShardDl[]; /* the shard's display list, in file 0xF4 */

/* "HbeamP" (the beam's shards): a shard from file 0xF4, 0.7 to 1.2 times the size given, at
 * the position given, spinning 7 degrees a frame one way or the other. It is thrown up at
 * random (20-30 up, up to 10 aside), falls ever faster (to 40), bounces twice off y -500 at 0.9
 * of its speed and ends there the third time, fading by 4 a frame in a flickering random
 * colour. */
s32 bisuHbeamPProc(Obj *obj, s16 msg, void *arg) {
    HbeamPW *w = (HbeamPW *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "HbeamP";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bisuHbeamShardDl, 0xF4);
            w->frame->pos[0] = ((HbeamPArg *) arg)->pos[0];
            w->frame->pos[1] = ((HbeamPArg *) arg)->pos[1];
            w->frame->pos[2] = ((HbeamPArg *) arg)->pos[2];
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                fadeRandRange(70, 120) / 100.0f * ((HbeamPArg *) arg)->scale;
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x20);
            w->alpha = 255;
            w->step = 0;
            if (fadeRand(100) > 50) {
                w->spin = 1;
            } else {
                w->spin = 0;
            }
            break;
        case 0:
            if (gBattleFlags2 & 0x400) {
                return -1;
            }
            if (w->spin == 1) {
                w->frame->rot[2] += 0.122173056f;
            } else {
                w->frame->rot[2] -= 0.122173056f;
            }
            w->alpha -= 4;
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            if (w->step == 0) {
                w->vel[0] = fadeRandRange(-10, 10);
                w->vel[1] = fadeRandRange(20, 30);
                w->vel[2] = fadeRandRange(-10, 10);
                w->fall = 0.0f;
                w->step++;
            } else if (w->step == 1) {
                if (w->frame->pos[1] <= -500.0f) {
                    w->vel[1] = -(w->vel[1] * 0.9);
                    w->step++;
                }
            } else if (w->step == 2) {
                if (w->frame->pos[1] <= -500.0f) {
                    w->vel[1] = -(w->vel[1] * 0.9);
                    w->step++;
                }
            } else if (w->step == 3 && w->frame->pos[1] <= -500.0f) {
                return -1;
            }
            w->fall += 0.2;
            w->vel[1] -= w->fall;
            if (w->vel[1] < -40.0f) {
                w->vel[1] = -40.0f;
            }
            bossApplyVel(obj);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = bossXluPrimEnvColorDL(fadeRand(200), fadeRand(200), fadeRand(200), 128);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

typedef struct NeedleArg {
    /* 0x00 */ u8 pad00[0x20];
    /* 0x20 */ f32 pos[3];
    /* 0x2C */ f32 scale;
    /* 0x30 */ u8 pad30[0x10];
} NeedleArg;

s32 bisuNeedleProc(Obj *obj, s16 msg, void *arg);

/* A "Needle" at x, y, z under the effect task. */
void bisuNeedleCreate(f32 x, f32 y, f32 z, f32 v) {
    NeedleArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = v;
    tskCreate(gEffectTask, bisuNeedleProc, gBattleTask->file, &arg);
}

/* "Needle"'s work. */
typedef struct NeedleW {
    /* 0x00 */ PolFrame *frame; /* its place and turn */
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ MPolFrame *model; /* drawn by the matrix built from the frame */
    /* 0x14 */ ACollFrame *atk;
    /* 0x18 */ ACollFrame *dmg;
    /* 0x1C */ u8 pad1C[0x14];
    /* 0x30 */ f32 distSq; /* from the boss */
    /* 0x34 */ s32 time;
    /* 0x38 */ s32 flash; /* set by a vulcan hit: drawn red for a frame */
    /* 0x3C */ u8 aimed;
} NeedleW;

extern u8 bisuNeedleDl[]; /* the needle's display list, in file 0xF4 */

/* "Needle": a needle from file 0xF4 at the position and size given, with an attack box (radius
 * 40, kind 0x1B) and a damage box (30 by 40, 20 down, kind 0x1C) (sound 0x21C). It flies at
 * speed 11 toward a point 300 from the boss at the angle of the frame count, turned to its flight
 * and spinning; at frame 30 it re-aims at speed 15 at a random point near the Impact. Hit kinds
 * 1, 3, 5, 8 and 11 shoot it down (a flash, coins, sound 0x228 unless the battle is ending); the
 * vulcan (15) only flashes it red. Landing its attack it is gone in a flash; it ends 8000 from
 * the boss or on gBattleFlags2 0x400. */
s32 bisuNeedleProc(Obj *obj, s16 msg, void *arg) {
    NeedleW *w = (NeedleW *) FX(obj);
    s32 pad;
    f32 a;
    f32 x;
    f32 y;
    s32 kind;

    switch (msg) {
        case 2:
            obj->name = "Needle";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->model = (MPolFrame *) frmCreate(gFrmModelLayer, gFrmKindMpol);
            if (w->model == NULL) {
                return -1;
            }
            w->model->dl2 = (Gfx *) bisuNeedleDl;
            floadGetSetRam((void **) w->model->segs, 0xF4);
            bossSetMpolFlags(w->model, 0x20);
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 40, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x1B;
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->frame->f, 30, 40, -20, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 0x1C;
            w->frame->pos[0] = ((NeedleArg *) arg)->pos[0];
            w->frame->pos[1] = ((NeedleArg *) arg)->pos[1];
            w->frame->pos[2] = ((NeedleArg *) arg)->pos[2];
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = ((NeedleArg *) arg)->scale;
            a = (gGameFrameCount % 360) * 0.01745329300388008;
            x = cosf(a) * 300.0f + gBossPosX;
            y = sinf(a) * 300.0f + gBossPosY;
            BmoveX(obj, x, y, gBossPosZ, 11.0f);
            SoundLocation2(0x21C, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                           15000.0f, gBattleCamView->fovy);
            w->time = 0;
            w->flash = 0;
            break;
        case 0:
            if (gBattleFlags2 & 0x400) {
                return -1;
            }
            w->time++;
            w->frame->rot[2] += 0.08726646501940039;
            w->frame->rot[1] = mathAtan2(w->vel[0], w->vel[2]);
            w->frame->rot[0] = mathAtan2(w->vel[1], sqrtf(w->vel[2] * w->vel[2] + w->vel[0] * w->vel[0]));
            if (w->aimed == 0 && w->time == 30) {
                BmoveX(obj, fadeRandRange(-150, 150) + gImpactPosX, fadeRandRange(-50, 50) + gImpactPosY, gImpactPosZ,
                       15.0f);
                w->aimed++;
            }
            if (acollDmgWasHit(w->dmg)) {
                kind = w->dmg->hitKind;
                if (kind == 1 || kind == 3 || kind == 5 || kind == 8 || kind == 11) {
                    itexFlash(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                    if (!(gBattleFlags & 0x10)) {
                        SoundLocation2(0x228, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                       300.0f, 15000.0f, gBattleCamView->fovy);
                    }
                    ryouCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
                    return -1;
                }
                if (kind == 15) {
                    PRINTF("バルカンに当たったけど無視。\n");
                    w->flash = 1;
                }
            }
            if (acollAtkWasHit(w->atk) == 1) {
                itexFlash(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                SoundLocation2(0x228, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
                return -1;
            }
            w->distSq = bossDistSq(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], gBossPosX, gBossPosY,
                                   gBossPosZ);
            if (w->distSq > 6.4e7f) {
                return -1;
            }
            bossApplyVel(obj);
            break;
        case 3:
            mathMtxIdent(w->model->mtx.m);
            mathMtxScaleRot(w->model->mtx.m, w->frame->scale[0]);
            mathMtxPreRotY(w->model->mtx.m, w->frame->rot[1]);
            mathMtxPreRotX(w->model->mtx.m, w->frame->rot[0]);
            mathMtxPreRotZ(w->model->mtx.m, w->frame->rot[2]);
            mathMtxPostTrans(w->model->mtx.m, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2]);
            if (w->flash) {
                w->model->dl = bossPrimEnvColorDL(255, 255, 255, 255, 0, 0, 255);
                w->flash = 0;
            } else {
                w->model->dl = bossPrimEnvColorDL(255, 255, 255, 0, 0, 0, 255);
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->model->f);
            frmDestroy(&w->atk->frame);
            frmDestroy(&w->dmg->frame);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

typedef struct RoseArg {
    /* 0x00 */ u8 pad00[0x4];
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ f32 scale;
    /* 0x14 */ u8 pad14[0xC];
} RoseArg;

s32 bisuRoseProc(Obj *obj, s16 msg, void *arg);

/* A "Rose" at x, y, z, of a size, under the effect task. */
void bisuRoseCreate(f32 x, f32 y, f32 z, f32 scale) {
    RoseArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = scale;
    tskCreate(gEffectTask, bisuRoseProc, gBattleTask->file, &arg);
}

/* "Rose"'s work. */
typedef struct RoseW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ u8 pad04[0x10];
    /* 0x14 */ s32 alpha;
    /* 0x18 */ s32 state; /* 0: blooming; 1: holding; 2: fading */
    /* 0x1C */ s32 timer;
} RoseW;

extern u8 bisuRoseAnim[]; /* the rose's animation, in file 0xF4 */

/* "Rose": an animated rose from file 0xF4 at the position and size given (sound 0x179). It
 * fades in by 1 to 192 while it blooms, holds 240 frames, then fades out by 1 (sound 0x51A after
 * 60 frames). It fades at once when the battle ends, and ends on gBattleFlags2 0x400. */
s32 bisuRoseProc(Obj *obj, s16 msg, void *arg) {
    RoseW *w = (RoseW *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Rose";
            w->frame = (AnimeFrame *) frmCreate(gFrmLayer8, gFrmKindAnime);
            if (w->frame == NULL) {
                PRINTF("隊長,タスクの作成に失敗しました。\n");
                return -1;
            }
            bossSetAnimAt(w->frame, bisuRoseAnim, 0xF4, 0.0f);
            bossSetFrameFlags2(w->frame, 0x20);
            w->frame->pos[0] = ((RoseArg *) arg)->pos[0];
            w->frame->pos[1] = ((RoseArg *) arg)->pos[1];
            w->frame->pos[2] = ((RoseArg *) arg)->pos[2];
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = ((RoseArg *) arg)->scale;
            w->alpha = 0;
            sndPlay(0x179);
            break;
        case 0:
            if (w->state == 0) {
                if (++w->alpha > 192) {
                    w->alpha = 192;
                }
                if (bossStepAnim(w->frame, 0.08f, 2)) {
                    w->state = 1;
                    w->timer = 240;
                }
            } else if (w->state == 1) {
                if (--w->timer <= 0) {
                    w->timer = 60;
                    w->state = 2;
                }
            } else if (w->state == 2) {
                if (--w->timer == 0) {
                    sndPlay(0x51A);
                }
                if (--w->alpha < 0) {
                    w->alpha = 0;
                    return -1;
                }
            }
            if (gBattleFlags2 & 0x400) {
                return -1;
            }
            if (gBattleFlags & 0x10) {
                w->state = 2;
            }
            break;
        case 3:
            w->frame->dl = bossXluPrimEnvColorDL(0, 0, 0, w->alpha);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The argument of the "Hpunch" task (bisuHpunchProc). */
typedef struct HpunchArg {
    /* 0x00 */ u8 pad00[8];
    /* 0x08 */ s32 type; /* 0-4: picks the model and the spin */
    /* 0x0C */ u8 pad0C[4];
    /* 0x10 */ f32 pos[3];
    /* 0x1C */ u8 pad1C[4];
} HpunchArg; /* size 0x20 */

s32 bisuHpunchProc(Obj *obj, s16 msg, void *arg);

/* Starts an "Hpunch" effect task of the given type at a point, under the effect task, in the
 * battle task's file (as beffect.c's beFlashPCreate). */
void bisuHpunchCreate(f32 x, f32 y, f32 z, s32 type) {
    HpunchArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.type = type;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bisuHpunchProc, gBattleTask->file, &arg);
}

/* "Hpunch"'s work (the second Hpunch's, bisuHpunch2Proc, below). */
typedef struct HpunchW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ ACollFrame *atk;
    /* 0x08 */ u8 pad08[0x4];
    /* 0x0C */ s32 time;
    /* 0x10 */ u8 pad10[0xC];
    /* 0x1C */ f32 alpha;
} HpunchW;

extern u8 bisuHpunchType0Dl[]; /* the punch for types 0-2, in file 0xF4 */
extern u8 bisuHpunchOtherDl[]; /* and for the others */

/* "Hpunch": a punch from file 0xF4 at half life size, its model and turn by its type (types 0
 * and 3 straight on, 1 and 4 turned 45 degrees one way, the rest the other), with an attack box
 * (radius 2000, kind 0x17) that is never live. It shrinks by 0.002 a frame, shakes up and down
 * by 10 every two frames and fades from 255 by 4, red to black. The second Hpunch
 * (bisuHpunch2Proc) is its twin. The argument is read through a typed local set in case 2, and
 * a one-byte pad sets its spill slot. */
s32 bisuHpunchProc(Obj *obj, s16 msg, void *arg) {
    HpunchW *w = (HpunchW *) FX(obj);
    HpunchArg *a;
    s8 pad;

    switch (msg) {
        case 2:
            obj->name = "Hpunch";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 2000, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            a = (HpunchArg *) arg;
            w->atk->kind = 0x17;
            if (a->type == 0 || a->type == 1 || a->type == 2) {
                bossSetPolDL(w->frame, (Gfx *) bisuHpunchType0Dl, 0xF4);
            } else {
                bossSetPolDL(w->frame, (Gfx *) bisuHpunchOtherDl, 0xF4);
            }
            w->frame->pos[0] = a->pos[0];
            w->frame->pos[1] = a->pos[1];
            w->frame->pos[2] = a->pos[2];
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x20);
            if (a->type == 0 || a->type == 3) {
                w->frame->rot[1] = 0.0f;
            } else if (a->type == 1 || a->type == 4) {
                w->frame->rot[1] = -0.7853982f;
            } else {
                w->frame->rot[1] = 0.7853982f;
            }
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 0.5f;
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
            w->frame->scale[0] -= 0.002;
            w->frame->scale[1] -= 0.002;
            w->frame->scale[2] -= 0.002;
            if (w->time % 4 >= 2) {
                w->frame->pos[1] += 10.0f;
            } else {
                w->frame->pos[1] -= 10.0f;
            }
            w->alpha -= 4.0f;
            if (w->alpha < 0.0f) {
                return -1;
            }
            w->atk->flags &= ~2;
            w->time++;
            break;
        case 3:
            w->frame->dl = bossXluPrimEnvColorDL(w->alpha / 2, 0, 0, w->alpha);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->atk->frame);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Lbarri"'s work. */
typedef struct LbarriW {
    /* 0x00 */ PolFrame *frame; /* the barrier */
    /* 0x04 */ PolFrame *hearts[6];
    /* 0x1C */ Frame *unused1C; /* freed in msg 1, never made */
    /* 0x20 */ Frame *unused20; /* freed in msg 1, never made */
    /* 0x24 */ s32 alpha;
    /* 0x28 */ f32 angle; /* the hearts' */
} LbarriW;

extern u8 bisuLbarriDl[]; /* the barrier's display list, in file 0xF4 */

/* "Lbarri", the boss's love barrier: a barrier (file 0xF4) 400 in front of the boss, growing to
 * 11 by 0.02 a frame, with six hearts circling it 300 out, 60 degrees apart, turning 5 degrees a
 * frame and growing to 3; red, yellow, green, blue, violet and magenta, three of them sparkling
 * in turn every 5 frames. Unless the boss holds it (gBossFlags 0x200) it fades by 10 a frame
 * from 0xB0 and is gone at 0. The hearts' creation checks test the first three hearts again. */
s32 bisuLbarriProc(Obj *obj, s16 msg, void *arg) {
    LbarriW *w = (LbarriW *) FX(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "Lbarri";
            if ((w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                return -1;
            }
            if ((w->hearts[0] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                return -1;
            }
            if ((w->hearts[1] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                return -1;
            }
            if ((w->hearts[2] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                return -1;
            }
            w->hearts[3] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->hearts[0] == NULL) {
                return -1;
            }
            w->hearts[4] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->hearts[1] == NULL) {
                return -1;
            }
            w->hearts[5] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->hearts[2] == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bisuLbarriDl, 0xF4);
            bossSetPolDL(w->hearts[0], (Gfx *) bisuHbeamFlashDl, 0xF4);
            bossSetPolDL(w->hearts[1], (Gfx *) bisuHbeamFlashDl, 0xF4);
            bossSetPolDL(w->hearts[2], (Gfx *) bisuHbeamFlashDl, 0xF4);
            bossSetPolDL(w->hearts[3], (Gfx *) bisuHbeamFlashDl, 0xF4);
            bossSetPolDL(w->hearts[4], (Gfx *) bisuHbeamFlashDl, 0xF4);
            bossSetPolDL(w->hearts[5], (Gfx *) bisuHbeamFlashDl, 0xF4);
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x27);
            bossSetFrameFlags((AnimeFrame *) w->hearts[0], 0x27);
            bossSetFrameFlags((AnimeFrame *) w->hearts[1], 0x27);
            bossSetFrameFlags((AnimeFrame *) w->hearts[2], 0x27);
            bossSetFrameFlags((AnimeFrame *) w->hearts[3], 0x27);
            bossSetFrameFlags((AnimeFrame *) w->hearts[4], 0x27);
            bossSetFrameFlags((AnimeFrame *) w->hearts[5], 0x27);
            bossHeadingOffset(gBossYaw, 0.0f, 300.0f, &w->frame->pos[0], &w->frame->pos[2]);
            w->frame->pos[0] += gBossPosX;
            w->frame->pos[1] = gBossPosY;
            w->frame->pos[2] += gBossPosZ;
            w->frame->scale[2] = 0.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->hearts[0]->scale[2] = 0.0f;
            w->hearts[0]->scale[0] = w->hearts[0]->scale[1] = w->hearts[0]->scale[2];
            w->hearts[1]->scale[2] = 0.0f;
            w->hearts[1]->scale[0] = w->hearts[1]->scale[1] = w->hearts[1]->scale[2];
            w->hearts[2]->scale[2] = 0.0f;
            w->hearts[2]->scale[0] = w->hearts[2]->scale[1] = w->hearts[2]->scale[2];
            w->hearts[3]->scale[2] = 0.0f;
            w->hearts[3]->scale[0] = w->hearts[3]->scale[1] = w->hearts[3]->scale[2];
            w->hearts[4]->scale[2] = 0.0f;
            w->hearts[4]->scale[0] = w->hearts[4]->scale[1] = w->hearts[4]->scale[2];
            w->hearts[5]->scale[2] = 0.0f;
            w->hearts[5]->scale[0] = w->hearts[5]->scale[1] = w->hearts[5]->scale[2];
            w->alpha = 0xB0;
            w->angle = 0.0f;
            break;
        case 1:
            DPRINTF("ラブバリアー消滅します。[%d]\n", w->alpha);
            frmDestroy(&w->frame->f);
            frmDestroy(&w->hearts[0]->f);
            frmDestroy(&w->hearts[1]->f);
            frmDestroy(&w->hearts[2]->f);
            frmDestroy(&w->hearts[3]->f);
            frmDestroy(&w->hearts[4]->f);
            frmDestroy(&w->hearts[5]->f);
            frmDestroy(w->unused1C);
            frmDestroy(w->unused20);
            break;
        case 0:
            w->frame->scale[2] += 0.02;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->hearts[0]->scale[2] += 0.02;
            w->hearts[0]->scale[0] = w->hearts[0]->scale[1] = w->hearts[0]->scale[2];
            w->hearts[1]->scale[2] += 0.02;
            w->hearts[1]->scale[0] = w->hearts[1]->scale[1] = w->hearts[1]->scale[2];
            w->hearts[2]->scale[2] += 0.02;
            w->hearts[2]->scale[0] = w->hearts[2]->scale[1] = w->hearts[2]->scale[2];
            w->hearts[3]->scale[2] += 0.02;
            w->hearts[3]->scale[0] = w->hearts[3]->scale[1] = w->hearts[3]->scale[2];
            w->hearts[4]->scale[2] += 0.02;
            w->hearts[4]->scale[0] = w->hearts[4]->scale[1] = w->hearts[4]->scale[2];
            w->hearts[5]->scale[2] += 0.02;
            w->hearts[5]->scale[0] = w->hearts[5]->scale[1] = w->hearts[5]->scale[2];
            if (w->frame->scale[0] > 11.0) {
                w->frame->scale[2] = 11.0f;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            }
            if (w->hearts[0]->scale[0] > 3.0) {
                w->hearts[0]->scale[2] = 3.0f;
                w->hearts[0]->scale[0] = w->hearts[0]->scale[1] = w->hearts[0]->scale[2];
            }
            if (w->hearts[1]->scale[0] > 3.0) {
                w->hearts[1]->scale[2] = 3.0f;
                w->hearts[1]->scale[0] = w->hearts[1]->scale[1] = w->hearts[1]->scale[2];
            }
            if (w->hearts[2]->scale[0] > 3.0) {
                w->hearts[2]->scale[2] = 3.0f;
                w->hearts[2]->scale[0] = w->hearts[2]->scale[1] = w->hearts[2]->scale[2];
            }
            if (w->hearts[3]->scale[0] > 3.0) {
                w->hearts[3]->scale[2] = 3.0f;
                w->hearts[3]->scale[0] = w->hearts[3]->scale[1] = w->hearts[3]->scale[2];
            }
            if (w->hearts[4]->scale[0] > 3.0) {
                w->hearts[4]->scale[2] = 3.0f;
                w->hearts[4]->scale[0] = w->hearts[4]->scale[1] = w->hearts[4]->scale[2];
            }
            if (w->hearts[5]->scale[0] > 3.0) {
                w->hearts[5]->scale[2] = 3.0f;
                w->hearts[5]->scale[0] = w->hearts[5]->scale[1] = w->hearts[5]->scale[2];
            }
            bossHeadingOffset(gBossYaw, 0.0f, 400.0f, &w->frame->pos[0], &w->frame->pos[2]);
            w->frame->pos[0] += gBossPosX;
            w->frame->pos[1] = gBossPosY;
            w->frame->pos[2] += gBossPosZ;
            w->angle += 0.0872664650194003855;
            bossHeadingOffset(w->angle, 0.0f, 300.0f, &w->hearts[0]->pos[0], &w->hearts[0]->pos[1]);
            w->hearts[0]->pos[0] += w->frame->pos[0];
            w->hearts[0]->pos[1] += w->frame->pos[1];
            w->hearts[0]->pos[2] = w->frame->pos[2];
            bossHeadingOffset(w->angle + 2.09439516046560925, 0.0f, 300.0f, &w->hearts[1]->pos[0],
                              &w->hearts[1]->pos[1]);
            w->hearts[1]->pos[0] += w->frame->pos[0];
            w->hearts[1]->pos[1] += w->frame->pos[1];
            w->hearts[1]->pos[2] = w->frame->pos[2];
            bossHeadingOffset(w->angle + 4.1887903209312185, 0.0f, 300.0f, &w->hearts[2]->pos[0],
                              &w->hearts[2]->pos[1]);
            w->hearts[2]->pos[0] += w->frame->pos[0];
            w->hearts[2]->pos[1] += w->frame->pos[1];
            w->hearts[2]->pos[2] = w->frame->pos[2];
            bossHeadingOffset(w->angle + 1.04719758023280463, 0.0f, 300.0f, &w->hearts[3]->pos[0],
                              &w->hearts[3]->pos[1]);
            w->hearts[3]->pos[0] += w->frame->pos[0];
            w->hearts[3]->pos[1] += w->frame->pos[1];
            w->hearts[3]->pos[2] = w->frame->pos[2];
            bossHeadingOffset(w->angle + 3.14159274069841432, 0.0f, 300.0f, &w->hearts[4]->pos[0],
                              &w->hearts[4]->pos[1]);
            w->hearts[4]->pos[0] += w->frame->pos[0];
            w->hearts[4]->pos[1] += w->frame->pos[1];
            w->hearts[4]->pos[2] = w->frame->pos[2];
            bossHeadingOffset(w->angle + 5.23598790116402313, 0.0f, 300.0f, &w->hearts[5]->pos[0],
                              &w->hearts[5]->pos[1]);
            w->hearts[5]->pos[0] += w->frame->pos[0];
            w->hearts[5]->pos[1] += w->frame->pos[1];
            w->hearts[5]->pos[2] = w->frame->pos[2];
            if (gGameFrameCount % 15 == 0) {
                (*bisuHbeamECreate)(w->hearts[0]->pos[0], w->hearts[0]->pos[1], w->hearts[0]->pos[2]);
            }
            if (gGameFrameCount % 15 == 5) {
                (*bisuHbeamECreate)(w->hearts[2]->pos[0], w->hearts[2]->pos[1], w->hearts[2]->pos[2]);
            }
            if (gGameFrameCount % 15 == 10) {
                (*bisuHbeamECreate)(w->hearts[4]->pos[0], w->hearts[4]->pos[1], w->hearts[4]->pos[2]);
            }
            if (!(gBossFlags & 0x200)) {
                w->alpha -= 10;
                if (w->alpha <= 0) {
                    return -1;
                }
            }
            break;
        case 3:
            w->frame->dl = bossPrimEnvColorDL(255, 255, 255, 255, 0x33, 0x7A, w->alpha);
            w->hearts[0]->dl = bossPrimEnvColorDL(255, 255, 255, 255, 0, 0, w->alpha);
            w->hearts[1]->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 0, w->alpha);
            w->hearts[2]->dl = bossPrimEnvColorDL(255, 255, 255, 0, 255, 0, w->alpha);
            w->hearts[3]->dl = bossPrimEnvColorDL(255, 255, 255, 0, 0, 255, w->alpha);
            w->hearts[4]->dl = bossPrimEnvColorDL(255, 255, 255, 0x80, 0, 255, w->alpha);
            w->hearts[5]->dl = bossPrimEnvColorDL(255, 255, 255, 255, 0, 255, w->alpha);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "plate"'s work. */
typedef struct PlateW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ s32 timer;
    /* 0x14 */ s32 state; /* 0-2: falling and bouncing; 3: resting */
} PlateW;

/* "plate": the boss's plate from file 0xF4, twice life size, dropped from 2500 above the boss
 * with a puff every fifth frame. It lands 75 above the boss (sound 0x178), bounces up at 20,
 * then at 10, settles, and is gone 20 frames later. It ends at once on gBattleFlags2 0x400 or
 * gBattleFlags 0x400. */
s32 bisuPlateProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    PlateW *w = (PlateW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "plate";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bisuPlateDl, 0xF4);
            w->frame->pos[0] = gBossPosX;
            w->frame->pos[1] = gBossPosY + 2500.0f;
            w->frame->pos[2] = gBossPosZ;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 2.0f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (gBattleFlags2 & 0x400) {
                return -1;
            }
            if (gBattleFlags & 0x400) {
                return -1;
            }
            bossApplyVel(obj);
            if (gGameFrameCount % 5 == 0) {
                itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
            }
            switch (w->state) {
                case 0:
                    w->vel[1] -= 1.5;
                    if (w->frame->pos[1] < gBossPosY + 75.0f) {
                        w->frame->pos[1] = gBossPosY + 75.0f;
                        w->vel[1] = 20.0f;
                        SoundLocation2(0x178, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                       300.0f, 15000.0f, gBattleCamView->fovy);
                        w->state++;
                    }
                    break;
                case 1:
                    w->vel[1] -= 1.5;
                    if (w->frame->pos[1] < gBossPosY + 75.0f) {
                        w->frame->pos[1] = gBossPosY + 75.0f;
                        w->state++;
                        w->vel[1] = 10.0f;
                    }
                    break;
                case 2:
                    w->vel[1] -= 2.0f;
                    if (w->frame->pos[1] < gBossPosY + 75.0f) {
                        w->frame->pos[1] = gBossPosY + 75.0f;
                        w->timer = 20;
                        w->state++;
                        w->vel[1] = 0.0f;
                    }
                    break;
                case 3:
                    if (--w->timer == 0) {
                        return -1;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "bVani"'s work. */
typedef struct BVaniW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ s32 timer;
    /* 0x08 */ s32 step; /* 0: waiting; 1: turning over; 2: holding */
} BVaniW;

/* "bVani": the vertex-animated model of file 0x2CA, twice life size, at the boss. After 49
 * frames it turns over by 8 degrees a frame to upside down, holds 100 frames and ends. Its
 * vertices are built and stepped when it is drawn. It ends at once on gBattleFlags2 0x400 or
 * 0x2000000, or gBrotherFlags 0x4000. The steps name the animation bisuBVaniVAnimeStep: under one
 * name IDO keeps the address on the stack between the two calls (IDO_NOTES 298). */
s32 bisuBVaniProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    BVaniW *w = (BVaniW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "bVani";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                PRINTF("隊長,タスクの作成に失敗しました。\n");
                return -1;
            }
            w->frame->dl2 = (Gfx *) bisuBVaniDl;
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x20);
            w->frame->segs[0] = fileRamAddr(0x2CA);
            w->frame->pos[0] = gBossPosX;
            w->frame->pos[1] = gBossPosY;
            w->frame->pos[2] = gBossPosZ;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 2.0f;
            w->frame->segs[5] = (u32) gGfxDlHead;
            gGfxDlHead = (Gfx *) vanimeMakeVtx((MVtx *) gGfxDlHead, (u32) bisuBVaniVAnime, 0x2CA);
            w->timer = 49;
            w->step = 0;
            break;
        case 1:
            gVanimeFrame = 0.0f;
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (gBattleFlags2 & 0x400) {
                return -1;
            }
            if (gBrotherFlags & 0x4000) {
                return -1;
            }
            if (gBattleFlags2 & 0x2000000) {
                return -1;
            }
            switch (w->step) {
                case 0:
                    if (--w->timer == 0) {
                        w->step++;
                    }
                    break;
                case 1:
                    w->frame->rot[2] += 0.13962634403104063;
                    if (w->frame->rot[2] > 3.1415927f) {
                        w->frame->rot[2] = 3.1415927f;
                        w->timer = 100;
                        w->step++;
                    }
                    break;
                case 2:
                    if (--w->timer == 0) {
                        return -1;
                    }
                    break;
            }
            break;
        case 3:
            if (w->step == 0) {
                w->frame->segs[5] = (u32) gGfxDlHead;
                gGfxDlHead = (Gfx *) vanimeMakeVtx((MVtx *) gGfxDlHead, (u32) bisuBVaniVAnime, 0x2CA);
                vanimeStep((u32) bisuBVaniVAnimeStep, 0x2CA, 0, 0.02f, 0.0f);
            } else if (w->step == 1) {
                w->frame->segs[5] = (u32) gGfxDlHead;
                gGfxDlHead = (Gfx *) vanimeMakeVtx((MVtx *) gGfxDlHead, (u32) bisuBVaniVAnime, 0x2CA);
                vanimeStep((u32) bisuBVaniVAnimeStep, 0x2CA, 0, 0.02f, 0.0f);
            } else if (w->step == 2) {
                w->frame->segs[5] = (u32) gGfxDlHead;
                gGfxDlHead = (Gfx *) vanimeMakeVtx((MVtx *) gGfxDlHead, (u32) bisuBVaniVAnime, 0x2CA);
                vanimeStep((u32) bisuBVaniVAnimeStep, 0x2CA, 0, 0.02f, 0.0f);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 bisuHpunch2Proc(Obj *obj, s16 msg, void *arg);

void bisuHpunch2Create(f32 x, f32 y, f32 z, s32 type) {
    HpunchArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.type = type;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bisuHpunch2Proc, gBattleTask->file, &arg);
}

/* The second "Hpunch"'s work. */
typedef struct Hpunch2W {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ ACollFrame *atk;
    /* 0x08 */ u8 pad08[0x4];
    /* 0x0C */ s32 time;
    /* 0x10 */ u8 pad10[0xC];
    /* 0x1C */ f32 alpha;
} Hpunch2W;

extern u8 bisuHpunch2Dl[]; /* the punch's display list, in file 0xF4 */

/* The second "Hpunch": a punch from file 0xF4 at 0.12 of life size, turned 45 degrees one way
 * or the other by its type (sound 0x285 unless the battle is ending), with an attack box
 * (radius 5000, kind 0x19) that is never live. It grows by 0.002 a frame, shakes up and down by
 * 10 every two frames and fades from 255 by 4, purple to black. */
s32 bisuHpunch2Proc(Obj *obj, s16 msg, void *arg) {
    Hpunch2W *w = (Hpunch2W *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Hpunch";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 5000, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x19;
            bossSetPolDL(w->frame, (Gfx *) bisuHpunch2Dl, 0xF4);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 0.12f;
            w->frame->pos[0] = ((HpunchArg *) arg)->pos[0];
            w->frame->pos[1] = ((HpunchArg *) arg)->pos[1];
            w->frame->pos[2] = ((HpunchArg *) arg)->pos[2];
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x27);
            if (((HpunchArg *) arg)->type == 0) {
                w->frame->rot[1] = 0.0f;
            } else if (((HpunchArg *) arg)->type == 1) {
                w->frame->rot[1] = -0.7853982f;
            } else {
                w->frame->rot[1] = 0.7853982f;
            }
            /* one source line, as the game's scheduling has it */
            if (!(gBattleFlags & 0x10)) { sndPlay(0x285); } w->time = 0; w->alpha = 255.0f;
            break;
        case 0:
            if (gBattleFlags & 0x10) {
                return -1;
            }
            if (gBattleFlags2 & 0x400) {
                return -1;
            }
            if (gBattleFlags2 & 0x4000) {
                return -1;
            }
            w->frame->scale[0] += 0.002;
            w->frame->scale[1] += 0.002;
            w->frame->scale[2] += 0.002;
            if (w->time % 4 >= 2) {
                w->frame->pos[1] += 10.0f;
            } else {
                w->frame->pos[1] -= 10.0f;
            }
            w->alpha -= 4.0f;
            if (w->alpha < 0.0f) {
                return -1;
            }
            w->atk->flags &= ~2;
            w->time++;
            break;
        case 3:
            w->frame->dl = bossXluPrimEnvColorDL(w->alpha / 2, 0, w->alpha / 2, w->alpha);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->atk->frame);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}


typedef struct BboostArg {
    /* 0x00 */ u8 pad00[0x4];
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ f32 unused10; /* the starter's 4th argument (100.0f at both calls); the task never reads it */
    /* 0x14 */ u8 pad14[0x4];
} BboostArg;

/* "Bboost"'s work. */
typedef struct BboostW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[0xC];
    /* 0x10 */ f32 height; /* above the floor at -1000 */
    /* 0x14 */ s32 alpha;
} BboostW;

extern u8 bisuBboostDl[]; /* the boost's display list, in file 0xF4 */

/* "Bboost": a column of thrust from file 0xF4 standing on the floor (y -1000) under the
 * position given, its size the height above the floor over 1000. It widens by 5% and grows by
 * 10% a frame, spinning 30 degrees, and fades from 128 by 8. */
s32 bisuBboostProc(Obj *obj, s16 msg, void *arg) {
    BboostW *w = (BboostW *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Bboost";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bisuBboostDl, 0xF4);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] =
                (f64) ((((BboostArg *) arg)->pos[1] - -1000.0f) / 1000.0f);
            w->frame->pos[0] = ((BboostArg *) arg)->pos[0];
            w->frame->pos[1] = -1000.0f;
            w->frame->pos[2] = ((BboostArg *) arg)->pos[2];
            w->height = ((BboostArg *) arg)->pos[1] - -1000.0f;
            w->alpha = 128;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->alpha -= 8;
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            w->frame->scale[0] *= 1.05;
            w->frame->scale[1] *= 1.1;
            w->frame->scale[2] *= 1.05;
            w->frame->rot[1] += 0.5235987901164023;
            break;
        case 3:
            w->frame->dl = bossXluPrimEnvColorDL(0, 0, 0, w->alpha);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "Bboost" at x, y, z under the effect task. */
void bisuBboostCreate(f32 x, f32 y, f32 z, f32 v) {
    BboostArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.unused10 = v;
    tskCreate(gEffectTask, bisuBboostProc, gBattleTask->file, &arg);
}

/* "bVDemo"'s work. */
typedef struct BVDemoW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ s32 timer;
    /* 0x08 */ s32 step; /* 0: start; 1: spinning; 2: shrinking */
} BVDemoW;


/* "bVDemo": a vertex-animated model from file 0x2CA, 1.7 times life size, 30 above the boss.
 * Each frame it builds its vertices into the display list (segment 8) and steps the
 * animation. It spins 10 degrees a frame for 120 frames, then shrinks until it is gone. The
 * steps name the animation bisuBVaniVAnimeStep: under one name IDO keeps the address in s0
 * (IDO_NOTES 298). */
s32 bisuBVDemoProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    BVDemoW *w = (BVDemoW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "bVDemo";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = (Gfx *) bisuBVaniDl;
            w->frame->segs[0] = fileRamAddr(0x2CA);
            w->frame->pos[0] = gBossPosX;
            w->frame->pos[1] = gBossPosY + 30.0f;
            w->frame->pos[2] = gBossPosZ;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 1.7f;
            w->frame->segs[5] = (u32) gGfxDlHead;
            gGfxDlHead = (Gfx *) vanimeMakeVtx((MVtx *) gGfxDlHead, (u32) bisuBVaniVAnime, 0x2CA);
            w->timer = 120;
            break;
        case 1:
            gVanimeFrame = 0.0f;
            frmDestroy(&w->frame->f);
            break;
        case 0:
            switch (w->step) {
                case 0:
                    w->frame->segs[5] = (u32) gGfxDlHead;
                    gGfxDlHead = (Gfx *) vanimeMakeVtx((MVtx *) gGfxDlHead, (u32) bisuBVaniVAnime, 0x2CA);
                    vanimeStep((u32) bisuBVaniVAnimeStep, 0x2CA, 2, 0.01f, 0.0f);
                    w->step++;
                    break;
                case 1:
                    w->frame->rot[1] += 0.17453293003880077;
                    if (--w->timer == 0) {
                        w->step++;
                    }
                    w->frame->segs[5] = (u32) gGfxDlHead;
                    gGfxDlHead = (Gfx *) vanimeMakeVtx((MVtx *) gGfxDlHead, (u32) bisuBVaniVAnime, 0x2CA);
                    vanimeStep((u32) bisuBVaniVAnimeStep, 0x2CA, 0, 0.01f, 0.0f);
                    break;
                case 2:
                    w->frame->scale[0] -= 0.1;
                    w->frame->scale[1] -= 0.05;
                    w->frame->scale[2] -= 0.1;
                    if (w->frame->scale[0] < 0.0f) {
                        return -1;
                    }
                    w->frame->segs[5] = (u32) gGfxDlHead;
                    gGfxDlHead = (Gfx *) vanimeMakeVtx((MVtx *) gGfxDlHead, (u32) bisuBVaniVAnime, 0x2CA);
                    vanimeStep((u32) bisuBVaniVAnimeStep, 0x2CA, 0, 0.01f, 0.0f);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "BplateD"'s work. */
typedef struct BplateDW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3]; /* bossApplyVel's (bmove.c), never set here */
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ s32 hold; /* while set it does not spin or shrink */
} BplateDW;

/* "BplateD": a plate from file 0xF4, twice life size and upside down, 80 below the boss. It
 * moves by its velocity and, unless held, spins 10 degrees a frame and shrinks by 0.01 until
 * it is gone. */
s32 bisuBplateDProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    BplateDW *w = (BplateDW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "BplateD";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bisuPlateDl, 0xF4);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 2.0f;
            w->frame->pos[0] = gBossPosX;
            w->frame->pos[1] = gBossPosY - 80.0f;
            w->frame->pos[2] = gBossPosZ;
            w->frame->rot[2] = 3.1415927f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            bossApplyVel(obj);
            if (w->hold == 0) {
                w->frame->rot[1] += 0.17453293003880077;
                w->frame->scale[0] -= 0.01;
                w->frame->scale[1] -= 0.01;
                w->frame->scale[2] -= 0.01;
                if (w->frame->scale[0] < 0.0f) {
                    w->frame->scale[0] = 0.0f;
                    w->frame->scale[1] = 0.0f;
                    w->frame->scale[2] = 0.0f;
                    return -1;
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "BroseD"'s work. */
typedef struct BroseDW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ s32 timer;
} BroseDW;

extern u8 bisuBroseDDl[]; /* the rose's display list, in file 0xF4 */

/* "BroseD": a rose from file 0xF4, a tenth of life size, thrown from 200 beside the boss toward
 * x -400 at speed 7 and up at 30. It falls slowly for 40 frames, then hard, shedding three stars
 * every third frame, and is gone after 180 frames. The stars' x and y are position + random
 * and z random + position, as the game's float registers show. */
s32 bisuBroseDProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    BroseDW *w = (BroseDW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "BroseD";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bisuBroseDDl, 0xF4);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 0.1f;
            w->frame->pos[0] = gBossPosX + 200.0f;
            w->frame->pos[1] = gBossPosY + 200.0f;
            w->frame->pos[2] = gBossPosZ + 200.0f;
            w->frame->rot[0] = 0.7853982f;
            w->frame->rot[2] = -0.7853982f;
            BmoveX(obj, -400.0f, 0.0f, 0.0f, 7.0f);
            w->timer = 180;
            w->vel[1] = 30.0f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->vel[1] -= 0.01;
            if (w->timer > 140) {
                w->vel[1] -= 0.99;
            }
            if (w->timer == 0) {
                return -1;
            }
            bossApplyVel(obj);
            if (gGameFrameCount % 6 == 0) {
                beStarCreate(w->frame->pos[0] + fadeRandRange(-100, 100), w->frame->pos[1] + fadeRandRange(-100, 100),
                             w->frame->pos[2] + fadeRandRange(-100, 100));
                beStarCreate(w->frame->pos[0] + fadeRandRange(-100, 100), w->frame->pos[1] + fadeRandRange(-100, 100),
                             w->frame->pos[2] + fadeRandRange(-100, 100));
                beStarCreate(w->frame->pos[0] + fadeRandRange(-100, 100), w->frame->pos[1] + fadeRandRange(-100, 100),
                             w->frame->pos[2] + fadeRandRange(-100, 100));
            }
            if (gGameFrameCount % 6 == 3) {
                beStarCreate(w->frame->pos[0] + fadeRandRange(-100, 100), w->frame->pos[1] + fadeRandRange(-100, 100),
                             w->frame->pos[2] + fadeRandRange(-100, 100));
                beStarCreate(w->frame->pos[0] + fadeRandRange(-100, 100), w->frame->pos[1] + fadeRandRange(-100, 100),
                             w->frame->pos[2] + fadeRandRange(-100, 100));
                beStarCreate(w->frame->pos[0] + fadeRandRange(-100, 100), w->frame->pos[1] + fadeRandRange(-100, 100),
                             w->frame->pos[2] + fadeRandRange(-100, 100));
            }
            w->timer--;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Bappcam", this boss's entrance: a letterboxed demo on the battle's camera. It glides in
 * (sounds 1 and 0x105), shakes between random points (sounds 0x10E, 0x111), closes in on the
 * boss at frame 615, sparkles above it at 700 (sound 0x126), pulls back at 900 (sounds 0x127 and
 * 0x101), rises with the boss at 1160 and 1300 (the second gives the battle's flags back), cuts
 * behind the Impact at 1450 and follows the Impact's view, swaying, until frame 1520.
 * gBattleFlags2 0x8000 ends it at once. Its shake's two branches are the same in the game. */
s32 bisuBappcamProc(Obj *obj, s16 msg, void *arg) {
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
            gBattleFlags2 |= 0x100;
            camSetFov(w->cam, 0.5235988f);
            camSetSpeed(w->cam, 300.0f);
            camSetEye(w->cam, 2000.0f, -800.0f, 0.0f);
            camSetAt(w->cam, 0.0f, -700.0f, -2000.0f);
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
            switch (w->step) {
                case 0:
                    if (w->timer == 0) {
                        camMoveEye(w->cam, -1000.0f, -800.0f, -1000.0f);
                        sndPlay(1);
                        sndPlay(0x105);
                        w->step++;
                    }
                    break;
                case 1:
                    fadeMoveLetterbox(50, 190, 1);
                    if (w->timer % 10 < 5) {
                        camMoveAt(w->cam, fadeRandRange(-1200, -800), fadeRandRange(-1000, -600), -1000.0f);
                    } else {
                        camMoveAt(w->cam, fadeRandRange(-1200, -800), fadeRandRange(-1000, -600), -1000.0f);
                    }
                    if (w->timer == 250) {
                        sndPlay(0x10E);
                    }
                    if (w->timer == 600) {
                        sndPlay(0x111);
                    }
                    if (w->timer == 615) {
                        camMoveFov(w->cam, 0.17453294f);
                        camSetSpeed(w->cam, 10.0f);
                        camMoveAt(w->cam, gBossPosX, gBossPosY, gBossPosZ);
                        w->step++;
                    }
                    break;
                case 2:
                    camMoveAt(w->cam, gBossPosX, gBossPosY, gBossPosZ);
                    if (w->timer == 700) {
                        effectKiranCreate(gBossPosX, gBossPosY + 500.0f, gBossPosZ);
                        sndPlay(0x126);
                    }
                    if (w->timer == 900) {
                        camSetSpeed(w->cam, 60.0f);
                        camMoveFov(w->cam, 0.87266463f);
                        camMoveEye(w->cam, 1000.0f, gBossPosY - 300.0f, -1000.0f);
                        camMoveAt(w->cam, 0.0f, gBossPosY + 100.0f, -2000.0f);
                        w->step++;
                    }
                    break;
                case 3:
                    if (w->timer == 920) {
                        sndPlay(0x127);
                    }
                    if (w->timer == 1130) {
                        sndPlay(0x101);
                    }
                    if (w->timer == 1160) {
                        camSetSpeed(w->cam, 40.0f);
                        camMoveFov(w->cam, 0.87266463f);
                        camMoveEye(w->cam, 0.0f, gBossPosY + 50.0f, 0.0f);
                        camMoveAt(w->cam, 0.0f, gBossPosY + 150.0f, -2000.0f);
                        w->step++;
                    }
                    break;
                case 4:
                    DPRINTF("[Sstep4]\n");
                    if (w->timer == 1300) {
                        camSetSpeed(w->cam, 30.0f);
                        camMoveFov(w->cam, 0.34906587f);
                        camMoveEye(w->cam, 0.0f, gBossPosY + 200.0f, 0.0f);
                        camMoveAt(w->cam, 0.0f, gBossPosY + 300.0f, -2000.0f);
                        gBattleFlags &= ~0x10000000;
                        gBattleFlags &= ~0x1000000;
                        w->step++;
                    }
                    break;
                case 5:
                    DPRINTF("[Sstep5]\n");
                    camMoveEye(w->cam, 1000.0f, gBossPosY - 350.0f, -800.0f);
                    camMoveAt(w->cam, 0.0f, gBossPosY + 300.0f, -2000.0f);
                    if (w->timer == 1450) {
                        camSetSpeed(w->cam, 10.0f);
                        camSetFov(w->cam, 0.34906587f);
                        camSetEye(w->cam, 0.0f, 0.0f, 0.0f);
                        camSetAt(w->cam, 0.0f, 0.0f, -300.0f);
                        gBattleFlags2 &= ~0x100;
                        gBrotherFlags &= ~0x10000;
                        w->step++;
                    }
                    break;
                case 6:
                    DPRINTF("[Sstep6]\n");
                    fadeOpenLetterbox();
                    camMoveFov(w->cam, 0.95993114f);
                    camMoveEye(w->cam, 0.0f, sinf(gGameFrameCount * 0.0174532930038800771) * 3.0f, 0.0f);
                    camMoveAt(w->cam, gCockpitLookAtX, gCockpitLookAtY, gCockpitLookAtZ);
                    if (w->timer == 1520) {
                        return -1;
                    }
                    break;
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Heffect"'s (bisuHeffectProc) argument: only the position its starter sets. */
typedef struct HeffectArg {
    /* 0x00 */ u8 pad00[0x24];
    /* 0x24 */ f32 pos[3];
    /* 0x30 */ u8 pad30[0x14];
} HeffectArg;

/* "Heffect"'s work. */
typedef struct HeffectW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[0x38];
    /* 0x3C */ s32 alpha;
    /* 0x40 */ u8 pad40[0x2];
    /* 0x42 */ u8 step; /* 0: squashing; 1: stretching up; 2: stretching and fading */
} HeffectW;

extern u8 bisuHeffectDl[]; /* the effect's display list, in file 0xEE */

/* "Heffect": a column from file 0xEE (4 x 8 x 4) at the position given, spinning 8 degrees a
 * frame. It squashes flat (widening by 0.4 a frame), then stretches up by 0.8 a frame to 12
 * times wide, then goes on stretching while it fades from 160 by 4, white on magenta. */
s32 bisuHeffectProc(Obj *obj, s16 msg, void *arg) {
    HeffectW *w = (HeffectW *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Heffect";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bisuHeffectDl, 0xEE);
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x20);
            w->frame->scale[0] = 4.0f;
            w->frame->scale[1] = 8.0f;
            w->frame->scale[2] = 4.0f;
            w->frame->pos[0] = ((HeffectArg *) arg)->pos[0];
            w->frame->pos[1] = ((HeffectArg *) arg)->pos[1];
            w->frame->pos[2] = ((HeffectArg *) arg)->pos[2];
            w->alpha = 160;
            w->step = 0;
            break;
        case 0:
            switch (w->step) {
                case 0:
                    w->frame->rot[1] += 0.13962634403104063;
                    w->frame->scale[0] += 0.4;
                    w->frame->scale[1] -= 0.4;
                    w->frame->scale[2] += 0.4;
                    if (w->frame->scale[1] < 0.1) {
                        w->step++;
                    }
                    break;
                case 1:
                    w->frame->rot[1] += 0.13962634403104063;
                    w->frame->scale[0] += 0.2;
                    w->frame->scale[1] += 0.8;
                    w->frame->scale[2] += 0.2;
                    if (w->frame->scale[0] > 12.0) {
                        w->step++;
                    }
                    break;
                case 2:
                    w->frame->rot[1] += 0.13962634403104063;
                    w->frame->scale[0] += 0.2;
                    w->frame->scale[1] += 0.8;
                    w->frame->scale[2] += 0.2;
                    w->alpha -= 4;
                    if (w->alpha <= 0) {
                        w->alpha = 0;
                        return -1;
                    }
                    break;
            }
            break;
        case 3:
            w->frame->dl = bossPrimEnvColorDL(255, 255, 255, 255, 0, 255, w->alpha);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* An "Heffect" at x, y, z under the effect task. */
void bisuHeffectCreate(f32 x, f32 y, f32 z) {
    HeffectArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bisuHeffectProc, gBattleTask->file, &arg);
}

typedef struct HringArg {
    /* 0x00 */ u8 pad00[0x24];
    /* 0x24 */ f32 pos[3];
    /* 0x30 */ u8 pad30[0x14];
} HringArg;

/* "Hring"'s work. */
typedef struct HringW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[0x38];
    /* 0x3C */ s32 alpha;
} HringW;

extern u8 bisuHringDl[]; /* the ring's display list, in file 0xEE */

/* "Hring": a ring from file 0xEE, 0.8 times life size, lying flat at the position given. It
 * rises 50 a frame and fades from 224 by 5, white on green. */
s32 bisuHringProc(Obj *obj, s16 msg, void *arg) {
    HringW *w = (HringW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "Hring";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bisuHringDl, 0xEE);
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x20);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 0.8f;
            w->frame->pos[0] = ((HringArg *) arg)->pos[0];
            w->frame->pos[1] = ((HringArg *) arg)->pos[1];
            w->frame->pos[2] = ((HringArg *) arg)->pos[2];
            w->frame->rot[0] = -1.5707964f;
            w->alpha = 224;
            break;
        case 0:
            w->alpha -= 5;
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            w->frame->pos[1] += 50.0f;
            break;
        case 3:
            w->frame->dl = bossPrimEnvColorDL(255, 255, 255, 0, 255, 0, w->alpha);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* An "Hring" at x, y, z under the effect task. */
void bisuHringCreate(f32 x, f32 y, f32 z) {
    HringArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bisuHringProc, gBattleTask->file, &arg);
}
