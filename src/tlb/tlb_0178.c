#include "tlb.h"

/* "bashi"'s work: an enemy's, with its own state from 0x34. */
typedef struct BashiW {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u8 step;
    /* 0x35 */ u8 pad35[0x3];
    /* 0x38 */ f32 scale;
    /* 0x3C */ u16 heading; /* 1024 a turn */
} BashiW;

extern u8 bashiAnim[]; /* its animation */

/* "bashi": an animated enemy whose placement value gives its scale (low half, tenths) and
 * heading (high half); it plays its animation once at half speed per retrace and goes. */
s32 bashiProc(Obj *o, s16 msg, void *arg) {
    BashiW *w = (BashiW *) FX(o);
    s32 v;

    switch (msg) {
        case 2:
            o->name = "bashi";
            v = eneSetup(o, arg);
            w->scale = (v & 0xFFFF) * 0.1f;
            w->heading = v >> 16;
            w->model = eneNewAnime(o, 0);
            if (w->model == NULL) {
                return -1;
            }
            break;
        case 0:
            eneMove((EneObj *) w);
            eneSetModelOnCourse(o, (Frame *) w->model);
            switch (w->step) {
                case 0:
                    w->model->anim = bashiAnim;
                    w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = w->scale;
                    w->rot[0] = 0.0f;
                    w->rot[1] = (f32) (u32) w->heading / 512 * 3.1415927f;
                    w->rot[2] = 0.0f;
                    w->step = 1;
                    break;
                case 1:
                    if (eneAnimPlayOnce(o, w->model, gRetracesPerFrame * 0.5f)) {
                        w->step = 2;
                    }
                    break;
                case 2:
                    return -1;
            }
            break;
        case 1:
            if (((BashiW *) FX(o))->model != NULL) {
                frmDestroy((Frame *) ((BashiW *) FX(o))->model);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "fireball"'s work: an enemy's, with its own from 0x34. */
typedef struct FireballW {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u8 primR;
    /* 0x35 */ u8 primG;
    /* 0x36 */ u8 primB;
    /* 0x37 */ u8 envR;
    /* 0x38 */ u8 envG;
    /* 0x39 */ u8 envB;
    /* 0x3A */ u8 pad3A[0x12];
    /* 0x4C */ ACollFrame *atk;
    /* 0x50 */ PolFrame *model2;
    /* 0x54 */ u8 state;
    /* 0x55 */ u8 alpha;
    /* 0x56 */ u8 type; /* eneSetup's */
} FireballW;

typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);

extern Gfx fireballDl[]; /* the ball */

/* "fireball": two pol models on the effect layer (set 0x120) with an attack box. It starts as a
 * yellow ball over red (0.1 and 0.08) with its box at radius 50 (kind 0x78) and sound 0x229;
 * then it grows by 0.005 a retrace, the two halves spinning 12 degrees a frame each way, and
 * fades by 8 (its box off under 33); at 0 it goes. Whoever its box hits gets message 7.
 * Its colours are named fields: as u8[3] arrays, gDPSetPrimColor/EnvColor load blue first. */
s32 fireballProc(Obj *o, s16 msg, void *arg) {
    FireballW *w = (FireballW *) FX(o);
    s32 hitter;

    switch (msg) {
        case 2:
            o->name = "fireball";
            w->type = eneSetup(o, arg);
            if ((w->model = eneNewPol(o, NULL)) == NULL) {
                return -1;
            }
            frmMove(&w->model->f, gFrmEffectLayer);
            if ((w->model2 = eneNewPol(o, NULL)) == NULL) {
                return -1;
            }
            frmMove(&w->model2->f, gFrmEffectLayer);
            floadLoadSet(0x120);
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 0, 0, 0, 0)) == NULL) {
                return -1;
            }
            break;
        case 0:
            if (acollAtkWasHit(w->atk) == 1 && (hitter = w->atk->hitPower) != 0) {
                tskSendMsg((Obj *) hitter, 7, o);
            }
            eneMove((EneObj *) w);
            eneSetModelOnCourse(o, &w->model->f);
            switch (w->state) {
                case 0:
                    w->model->dl2 = fireballDl;
                    w->model2->dl2 = fireballDl;
                    eneGetSegs(o, w->model->segs);
                    eneGetSegs(o, w->model2->segs);
                    w->primR = 255;
                    w->primG = 255;
                    w->primB = 0;
                    w->envR = 255;
                    w->envG = 0;
                    w->envB = 0;
                    w->model->scale[2] = 0.1f;
                    w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                    w->model2->scale[2] = 0.08f;
                    w->model2->scale[0] = w->model2->scale[1] = w->model2->scale[2];
                    w->alpha = 0xFF;
                    w->atk->radius = 50;
                    w->atk->kind = 0x78;
                    eneSetSd(o, 0x229);
                    w->state = 1;
                    break;
                case 1:
                    if (w->alpha - 8 < 8) {
                        w->alpha = 0;
                        w->state = 2;
                    } else {
                        w->alpha -= 8;
                    }
                    w->model->scale[0] += 0.005f * gRetracesPerFrame;
                    w->model->scale[1] = w->model->scale[2] = w->model2->scale[1] = w->model2->scale[2] =
                        w->model2->scale[0] = w->model->scale[0];
                    if (w->alpha < 33) {
                        w->atk->flags &= ~2;
                    }
                    w->model->rot[1] += 0.20943952f;
                    w->model2->rot[1] -= 0.20943952f;
                    break;
                case 2:
                    return -1;
            }
            break;
        case 3:
            if (w->alpha >= 0x100) { w->alpha = 0xFF; }
            w->model->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gSPDisplayList(gGfxDlHead++, gDrawSetupCldSurfDl);
            gDPSetCombine(gGfxDlHead++, 0x309A61, 0x5536FF7F);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->primR, w->primG, w->primB, 0);
            gDPSetEnvColor(gGfxDlHead++, w->envR, w->envG, w->envB, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            w->model2->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gSPDisplayList(gGfxDlHead++, gDrawSetupCldSurfDl);
            gDPSetCombine(gGfxDlHead++, 0x309A61, 0x5536FF7F);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->primR, w->primG, w->primB, 0);
            gDPSetEnvColor(gGfxDlHead++, w->envR, w->envG, w->envB, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            if (w->model != NULL) { frmDestroy(&w->model->f); }
            if (w->model2 != NULL) { frmDestroy(&w->model2->f); }
            if (w->atk != NULL) { frmDestroy(&w->atk->frame); }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
