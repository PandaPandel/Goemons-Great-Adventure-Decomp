#include "tlb.h"
#include "tlb/tlb_0325.h"

extern Obj *gBattleTask; /* the battle's task (beffect.c) */

#define sinf (*sinf)
#define cosf (*cosf)

/* "Ddmybomb"'s (bfriDdmybombProc) argument: only what its starter sets. */
typedef struct DdmybombArg {
    /* 0x00 */ u8 pad00[0x1C];
    /* 0x1C */ f32 pos[3];
    /* 0x28 */ f32 radius; /* its radius */
    /* 0x2C */ u8 pad2C[0xC];
    /* 0x38 */ s32 timer; /* its frames */
    /* 0x3C */ s32 unused3C; /* the starter's 7th argument (10 at every call), only copied to w->unused3C */
    /* 0x40 */ s32 kind; /* its attack kind */
} DdmybombArg;

/* "Ddmybomb"'s work. */
typedef struct DdmybombW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3]; /* x and z */
    /* 0x10 */ u8 pad10[0x18];
    /* 0x28 */ f32 radius;
    /* 0x2C */ f32 angle;
    /* 0x30 */ ACollFrame *atk;
    /* 0x34 */ u8 pad34[0x4];
    /* 0x38 */ s32 timer;
    /* 0x3C */ s32 unused3C; /* set from the argument's 0x3C (10) in msg 2, never read */
    /* 0x40 */ s32 kind;
} DdmybombW;

typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);

extern s32 bfriCloudsGather;
s32 bfriFightOver = 1;
extern CamView *gBattleCamView; /* the battle's camera (beffect.c) */

#define DPRINTF

/* "Ddmybomb", this battle's invisible bomb: tlb_0324_5's at 150 a frame, with a flash of 6 on a
 * hit and its own kinds' sounds (0x27E, 0x27F or 0x228). Its destroys are NULL-checked, each on
 * one line: written over three lines, IDO schedules w's addiu out of the first test's delay
 * slot. */
s32 bfriDdmybombProc(Obj *obj, s16 msg, void *arg) {
    DdmybombW *w = (DdmybombW *) FX(obj);
    s16 pan;

    switch (msg) {
        case 2:
            w->kind = ((DdmybombArg *) arg)->kind;
            obj->name = "Ddmybomb";
            if ((w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol)) == NULL) {
                return -1;
            }
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f,
                                                         ((DdmybombArg *) arg)->radius, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = w->kind;
            w->frame->pos[0] = ((DdmybombArg *) arg)->pos[0];
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = ((DdmybombArg *) arg)->pos[2];
            w->angle = mathAtan2(-w->frame->pos[0], -w->frame->pos[2]);
            w->vel[2] = -cosf(w->angle + 3.1415927f) * 150.0f;
            w->vel[0] = -sinf(w->angle + 3.1415927f) * 150.0f;
            w->frame->scale[2] = 2.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->timer = ((DdmybombArg *) arg)->timer + 1;
            w->unused3C = ((DdmybombArg *) arg)->unused3C;
            w->radius = ((DdmybombArg *) arg)->radius;
            DPRINTF("とうめい爆弾 %d %d %d\n", w->timer, w->unused3C, w->kind);
            break;
        case 1:
            DPRINTF("とうめい爆弾消滅\n");
            if (w->frame != NULL) { frmDestroy(&w->frame->f); }
            if (w->atk != NULL) { frmDestroy((Frame *) w->atk); }
            break;
        case 0:
            if (bfriFightOver != 0 || bfriCloudsGather != 0) {
                return -1;
            }
            w->frame->pos[0] += w->vel[0];
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] += w->vel[2];
            if (acollAtkWasHit(w->atk) == 1) {
                DPRINTF("あたり！\n");
                itexFlash(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 6.0f);
                pan = soundPanGet(gBattleCamView, w->frame->pos[0], w->frame->pos[2], gBattleCamView->fovy);
                switch (w->kind) {
                    case 0x3E:
                        sndRequest(0x27E, pan, 0);
                        break;
                    case 0x40:
                        sndRequest(0x27F, pan, 0);
                        break;
                    case 0x3F:
                        sndRequest(0x27E, pan, 0);
                        break;
                    case 0x43:
                        sndRequest(0x228, pan, 0);
                        break;
                }
                return -1;
            }
            if (--w->timer < 0) {
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "Ddmybomb" at x, y, z under obj, in the battle task's file. */
void bfriDdmybombCreate(Obj *obj, f32 x, f32 y, f32 z, f32 v, s32 a, s32 b, s32 c) {
    DdmybombArg arg;
    s32 pad[3];

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.radius = v;
    arg.timer = a;
    arg.unused3C = b;
    arg.kind = c;
    tskCreate(obj, bfriDdmybombProc, gBattleTask->file, &arg);
}

/* "ELEC_BOMB"'s (bfriElecBombProc) argument: only the position its starter sets. */
typedef struct ElecBombArg {
    /* 0x00 */ u8 pad00[0x18];
    /* 0x18 */ f32 pos[3];
    /* 0x24 */ u8 pad24[0x18];
} ElecBombArg;

/* "ELEC_BOMB"'s work. */
typedef struct ElecBombW {
    /* 0x00 */ PolFrame *core;
    /* 0x04 */ PolFrame *ring;
    /* 0x08 */ PolFrame *glow;
    /* 0x0C */ f32 vel[3];
    /* 0x18 */ u8 pad18[0xC];
    /* 0x24 */ f32 angle; /* toward the middle */
    /* 0x28 */ f32 unused28; /* set 3.0f in msg 2, never read */
    /* 0x2C */ ACollFrame *atk;
    /* 0x30 */ ACollFrame *dmg;
    /* 0x34 */ s32 timer;
    /* 0x38 */ s32 unused38; /* set 0 in msg 2, never read */
} ElecBombW;

typedef ACollFrame *(*DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                    s32 a);

#define bfriRandSigned (*bfriRandSigned)
extern Gfx bfriElecBombGlowDl[]; /* a glow */
extern Gfx bfriElecBombDl[]; /* the bomb */
extern Gfx bfriElecBombRingDl[]; /* its ring */

/* "ELEC_BOMB", an electric bomb at its starter's place: a white core with a magenta ring and a
 * dark glow (scale 2, 2 and 7, flat), an attack box (radius 70, kind 0x41) and a damage box (50
 * by 100, kind 0x42). It drifts over 80 frames toward a random point near the middle, the
 * core spinning 50 degrees a frame, and goes after 96 frames, 10000 from the boss, or when the
 * battle stops. Hit by attack kinds 15, 16, 1, 11, 3, 8 or 5 it bursts (blue glow, sound 0x228;
 * for 1, 3, 5 and 8 it drops ryou); when its own box hits, a puff (and the sound outside
 * flag 0x10). */
s32 bfriElecBombProc(Obj *obj, s16 msg, void *arg) {
    ElecBombW *w = (ElecBombW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "ELEC_BOMB";
            if ((w->glow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                return -1;
            }
            if ((w->core = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                return -1;
            }
            if ((w->ring = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                return -1;
            }
            bossSetPolDL(w->ring, bfriElecBombRingDl, 0xFB);
            bossSetPolDL(w->core, bfriElecBombDl, 0xFB);
            bossSetPolDL(w->glow, bfriElecBombGlowDl, 0x120);
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->core->f, 70, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 0x41;
            if ((w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->core->f, 50, 100, -50, 0, 0)) ==
                NULL) {
                return -1;
            }
            w->dmg->kind = 0x42;
            w->core->flags |= 0x22;
            w->glow->flags |= 0x27;
            w->ring->flags |= 0x27;
            w->core->pos[0] = ((ElecBombArg *) arg)->pos[0];
            w->core->pos[1] = ((ElecBombArg *) arg)->pos[1];
            w->core->pos[2] = ((ElecBombArg *) arg)->pos[2];
            w->angle = mathAtan2(-w->core->pos[0], -w->core->pos[2]);
            w->timer = 0;
            w->unused38 = 0;
            w->unused28 = 3.0f;
            w->vel[0] = (bfriRandSigned(20) - w->core->pos[0]) / 80.0f;
            w->vel[1] = (bfriRandSigned(10) - w->core->pos[1]) / 80.0f;
            w->vel[2] = (bfriRandSigned(20) - w->core->pos[2]) / 80.0f;
            w->ring->scale[0] = 2.0f;
            w->ring->scale[2] = 0.0f;
            w->ring->scale[1] = 2.0f;
            w->glow->scale[0] = 7.0f;
            w->glow->scale[2] = 0.0f;
            w->glow->scale[1] = 7.0f;
            w->core->scale[0] = 2.0f;
            w->core->scale[2] = 0.0f;
            w->core->scale[1] = 2.0f;
            break;
        case 1:
            if (w->core != NULL) { frmDestroy(&w->core->f); }
            if (w->atk != NULL) { frmDestroy(&w->atk->frame); }
            if (w->dmg != NULL) { frmDestroy(&w->dmg->frame); }
            if (w->ring != NULL) { frmDestroy(&w->ring->f); }
            if (w->glow != NULL) { frmDestroy(&w->glow->f); }
            break;
        case 3:
            w->glow->dl = bossPrimEnvColorDL(0, 0, 0, 0, 0, 0, 0xF0);
            w->core->dl = bossPrimEnvColorDL(0xFF, 0xFF, 0xFF, 0x64, 0, 0x32, 0xFF);
            w->ring->dl = bossPrimEnvColorDL(0xDC, 0, 0x5A, 0x64, 0, 0x32, 0xFF);
            break;
        case 0:
            if (bfriFightOver != 0) {
                return -1;
            }
            if (acollDmgWasHit(w->dmg)
                && (w->dmg->hitKind == 15 || w->dmg->hitKind == 16 || w->dmg->hitKind == 1 || w->dmg->hitKind == 11
                    || w->dmg->hitKind == 3 || w->dmg->hitKind == 8 || w->dmg->hitKind == 5)) {
                itexGlowBlue(w->core->pos[0], w->core->pos[1], w->core->pos[2], 2.0f);
                SoundLocation2(0x228, gBattleCamView, w->core->pos[0], w->core->pos[1], w->core->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
                if (w->dmg->hitKind == 1 || w->dmg->hitKind == 3 || w->dmg->hitKind == 5 || w->dmg->hitKind == 8) {
                    ryouCreate(w->core->pos[0], w->core->pos[1], w->core->pos[2], 2.0f);
                }
                return -1;
            }
            if (acollAtkWasHit(w->atk) == 1) {
                effectPompomCreate(w->core->pos[0], w->core->pos[1], w->core->pos[2], 20.0f, 20.0f, 20.0f, 11, 10, 2,
                                   0.5f, 0.0f);
                if (!(gBattleFlags & 0x10)) {
                    SoundLocation2(0x228, gBattleCamView, w->core->pos[0], w->core->pos[1], w->core->pos[2], 300.0f,
                                   15000.0f, gBattleCamView->fovy);
                }
                return -1;
            }
            if (bossDistSq(w->core->pos[0], w->core->pos[1], w->core->pos[2], gBossPosX, gBossPosY, gBossPosZ) > 1e8f) {
                return -1;
            }
            if (++w->timer > 96) {
                w->timer = 96;
                return -1;
            }
            w->core->pos[0] += w->vel[0];
            w->core->pos[1] += w->vel[1];
            w->core->pos[2] += w->vel[2];
            w->ring->pos[0] = w->core->pos[0];
            w->ring->pos[1] = w->core->pos[1];
            w->ring->pos[2] = w->core->pos[2];
            w->glow->pos[0] = w->core->pos[0];
            w->glow->pos[1] = w->core->pos[1];
            w->glow->pos[2] = w->core->pos[2] - 30.0f;
            w->core->rot[2] += 0.872664650194004;
            if (w->core->rot[2] > 6.2831855f) {
                w->core->rot[2] -= 6.2831855f;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 bfriElecBombProc(Obj *obj, s16 msg, void *arg);

/* An "ELEC_BOMB" at x, y, z under obj, in the battle task's file. */
void bfriElecBombCreate(Obj *obj, f32 x, f32 y, f32 z) {
    ElecBombArg arg;
    s32 pad[3];

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    tskCreate(obj, bfriElecBombProc, gBattleTask->file, &arg);
}

/* "PuroBomb"'s work. */
typedef struct PuroBombW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ PolFrame *glow;
    /* 0x08 */ ACollFrame *atk;
    /* 0x0C */ ACollFrame *dmg;
    /* 0x10 */ f32 vel[3];
    /* 0x1C */ s32 timer;
    /* 0x20 */ s32 alpha;
    /* 0x24 */ u8 blowing; /* it has hit, or come close: it shakes and drops bombs */
    /* 0x25 */ u8 pad25[0x3];
    /* 0x28 */ f32 unused28; /* set 0.0f in msg 2, never read */
    /* 0x2C */ u8 pad2C[0x4];
    /* 0x30 */ f32 spin;
} PuroBombW;

s32 bfriPuroBombBlowing = 1; /* 1: a PuroBomb is blowing */
u32 bfriUnused2[] = { 0x00000000, 0x00000000 };
u32 bfriTornadoUnusedFlag = 0x00000001;
u32 bfriTornadoAlpha = 0x00000001;
u32 bfriPropellerFull = 0x00000001;
u32 bfriCloudsRun = 0x00000001;
extern Gfx bfriPuroBombDl[]; /* the bomb */

/* "PuroBomb", a propeller bomb from the boss's place: a model of a tenth with a red glow, an
 * attack box (radius 40, kind 0x44) and a damage box (130 by 200, kind 0x45), spinning 12
 * degrees a frame. It speeds up toward the players (1 a frame to 34), drawing in to the middle,
 * the glow brightening by 10 to 160. Hit by attack kinds 3, 8, 5 or 11 it bursts (blue glow,
 * ryou, sound 0x228); hit by the propeller (kind 1) it is knocked back (sound 0x150). Once its
 * box hits (sound 0x27E) or it is within 330, it hovers shaking and drops an invisible bomb
 * every 10 frames (sounds 0x417 and 0x418 in turn), going after 51 frames. */
s32 bfriPuroBombProc(Obj *obj, s16 msg, void *arg) {
    PuroBombW *w = (PuroBombW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "PuroBomb";
            if ((w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol)) == NULL) {
                return -1;
            }
            if ((w->glow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, bfriPuroBombDl, 0xFB);
            bossSetPolDL(w->glow, bfriElecBombGlowDl, 0x120);
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 40, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 0x44;
            if ((w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->frame->f, 130, 200, -200, 0, 0)) ==
                NULL) {
                return -1;
            }
            w->dmg->kind = 0x45;
            w->frame->flags |= 0x22;
            w->glow->flags |= 0x22;
            w->frame->pos[0] = w->glow->pos[0] = gBossPosX;
            w->frame->pos[1] = w->glow->pos[1] = gBossPosY;
            w->frame->pos[2] = w->glow->pos[2] = gBossPosZ;
            w->frame->scale[0] = 0.1f;
            w->frame->scale[2] = 0.1f;
            w->frame->scale[1] = 0.1f;
            w->glow->scale[0] = 10.0f;
            w->glow->scale[2] = 10.0f;
            w->glow->scale[1] = 10.0f;
            w->frame->rot[0] = 0.0f;
            w->frame->rot[2] = 0.0f;
            w->frame->rot[1] = 0.0f;
            w->timer = 0;
            w->alpha = 0;
            w->blowing = 0;
            w->spin = 0.0f;
            w->unused28 = 0.0f;
            w->vel[2] = 0.0f;
            w->vel[0] = 0.0f;
            w->vel[1] = 0.0f;
            break;
        case 1:
            if (w->frame != NULL) { frmDestroy(&w->frame->f); }
            if (w->glow != NULL) { frmDestroy(&w->glow->f); }
            if (w->atk != NULL) { frmDestroy(&w->atk->frame); }
            if (w->dmg != NULL) { frmDestroy(&w->dmg->frame); }
            bfriPuroBombBlowing = 0;
            break;
        case 3:
            w->glow->dl = bossPrimEnvColorDL(0xFA, 0, 0x32, 0x64, 0, 0x28, w->alpha);
            break;
        case 0:
            if (bfriFightOver != 0) {
                return -1;
            }
            if (acollDmgWasHit(w->dmg)) {
                if (w->dmg->hitKind == 3 || w->dmg->hitKind == 8 || w->dmg->hitKind == 5 || w->dmg->hitKind == 11) {
                    w->frame->pos[0] -= 30.0f;
                    itexGlowBlue(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 3.0f);
                    ryouCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
                    SoundLocation2(0x228, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                                   15000.0f, gBattleCamView->fovy);
                    return -1;
                }
                if (w->dmg->hitKind == 1) {
                    DPRINTF("プロペラが当たりました");
                    SoundLocation2(0x150, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                                   15000.0f, gBattleCamView->fovy);
                    w->frame->pos[0] -= 30.0f;
                    w->vel[0] = -w->vel[0] / 3.0f;
                    w->vel[2] = -w->vel[2] / 3.0f;
                }
            }
            if (acollAtkWasHit(w->atk) == 1 && w->blowing == 0) {
                SoundLocation2(0x27E, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
                w->blowing = 1;
            }
            if (w->frame->pos[2] > -330.0f) {
                w->blowing = 1;
            }
            if (w->blowing) {
                bfriPuroBombBlowing = 1;
                w->timer++;
                w->vel[2] = 0.0f;
                w->vel[1] = 0.0f;
                w->vel[0] = 0.0f;
                if (w->timer % 10 == 0) {
                    (*bfriDdmybombCreate)(obj, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f, 3, 10,
                                              0x44);
                    if (gBattleSndToggles & 2) {
                        sndPlay(0x417);
                        gBattleSndToggles &= ~2;
                    } else {
                        sndPlay(0x418);
                        gBattleSndToggles |= 2;
                    }
                }
                if (w->timer % 6 < 3) {
                    w->frame->pos[1] = -6.0f;
                } else {
                    w->frame->pos[1] = 6.0f;
                }
                if (w->timer >= 51) {
                    return -1;
                }
            } else {
                if (bossDistSq(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], gBossPosX, gBossPosY, gBossPosZ) >
                    1e8f) {
                    return -1;
                }
                w->vel[2] += 1.0f;
                if (w->vel[2] > 34.0f) {
                    w->vel[2] = 34.0f;
                }
                w->frame->pos[2] += w->vel[2];
                w->frame->pos[0] *= 0.98;
                w->frame->pos[1] *= 0.98;
                w->glow->pos[0] = w->frame->pos[0];
                w->glow->pos[1] = w->frame->pos[1];
                w->glow->pos[2] = w->frame->pos[2];
                w->alpha += 10;
                if (w->alpha > 160) {
                    w->alpha = 160;
                }
            }
            w->spin += 0.20943951604656094;
            if (w->spin > 6.2831855f) {
                w->spin -= 6.2831855f;
            }
            w->frame->rot[2] = w->spin;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 bfriPuroBombProc(Obj *obj, s16 msg, void *arg);

/* Starts a "PuroBomb" (bfriPuroBombProc) under the effect task. */
void bfriPuroBombCreate(void) {
    tskCreate(gEffectTask, bfriPuroBombProc, gBattleTask->file, NULL);
}
