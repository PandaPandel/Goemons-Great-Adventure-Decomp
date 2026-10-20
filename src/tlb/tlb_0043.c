#include "tlb.h"
Gfx bigBombUnusedDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetCombineLERP(ENVIRONMENT, COMBINED, ENV_ALPHA, COMBINED, TEXEL0, 0, SHADE, 0, ENVIRONMENT, COMBINED, ENV_ALPHA, COMBINED, TEXEL0, 0, SHADE, 0),
    gsDPSetEnvColor(0x00, 0xFF, 0xFF, 0xFF),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPPipeSync(),
    gsSPEndDisplayList(),
    gsDPNoOp(),
};

/* "BIGBOMB": a big explosion, two pol models (a core and a shell) that grow by 8% a step and
 * turn opposite ways, coloured with a prim and an env colour, until its time runs out. */

/* An enemy's work (EneObj) to 0x34, then its own. */
typedef struct BigBombWork {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ Frame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *core;
    /* 0x38 */ PolFrame *shell;
    /* 0x3C */ s32 size; /* eneSetup's value: 1-5 grow, 10 grows and fades */
    /* 0x40 */ s32 time;
    /* 0x44 */ u8 primR; /* named fields: as arrays, IDO sorts the colour words' ORs */
    /* 0x45 */ u8 primG;
    /* 0x46 */ u8 primB;
    /* 0x47 */ u8 envR;
    /* 0x48 */ u8 envG;
    /* 0x49 */ u8 envB;
    /* 0x4A */ u8 alpha;
} BigBombWork;

extern Gfx bigBombDl[];

/* The BIGBOMB task. Its value from eneSetup picks the start size (1-5, 3 smaller) or 10 (small
 * and fading by 10 a step).
 * Matched 2026-10-04; it was parked for a delay slot: in msg 1 and msg 3 the game's branch
 * delay slot holds the work pointer's "addiu t0,t0,0x20", where this C's slot took the hoisted
 * address's addiu instead. Writing the branch and the statement after it on one source line
 * (IDO_NOTES 324, kensukeMoneyProc's find) gives the game's order. */
s32 bigBombProc(Obj *obj, s16 msg, void *arg) {
    BigBombWork *w = (BigBombWork *) FX(obj);
    s32 pad[33];

    switch (msg) {
        case 2:
            obj->name = "BIGBOMB";
            PRINTF("!!!!! BIGBOMB BIGBOMB BIGBOMB !!!!!!!\n");
            w->size = eneSetup(obj, arg);
            w->core = enePolModelMake(obj, bigBombDl, NULL, 1);
            if (w->core == NULL) {
                return -1;
            }
            w->shell = enePolModelMake(obj, bigBombDl, NULL, 1);
            if (w->shell == NULL) {
                return -1;
            }
            w->primR = 0xFF;
            w->primG = 0xFF;
            w->primB = 0;
            w->envR = 0xFF;
            w->envG = 0;
            w->envB = 0;
            switch (w->size) {
                case 1:
                case 2:
                case 4:
                case 5:
                    w->core->scale[0] = w->core->scale[1] = w->core->scale[2] = 0.3000000119f;
                    w->shell->scale[0] = w->shell->scale[1] = w->shell->scale[2] = 0.2399999946f;
                    w->alpha = 0x80;
                    w->time = 100;
                    break;
                case 3:
                    w->core->scale[0] = w->core->scale[1] = w->core->scale[2] = 0.1000000015f;
                    w->shell->scale[0] = w->shell->scale[1] = w->shell->scale[2] = 0.07999999821f;
                    w->alpha = 0x80;
                    w->time = 100;
                    break;
                case 10:
                    w->core->scale[0] = w->core->scale[1] = w->core->scale[2] = 0.1000000015f;
                    w->shell->scale[0] = w->shell->scale[1] = w->shell->scale[2] = 0.07999999821f;
                    w->alpha = 0xFF;
                    w->time = 24;
                    break;
            }
            PRINTF("!!!!!!!! BOMB1 SIZE = %f, BOMB1 SIZE = %f !!!!\n", w->core->scale[0], w->shell->scale[0]);
            break;
        case 1:
            if (w->core != NULL) { frmDestroy(&w->core->f); } if (w->shell != NULL) {
                frmDestroy(&w->shell->f);
            }
            break;
        case 3:
            PRINTF("**** alpha = [%d] scale = [%.3f]\n", w->alpha, w->core->scale[0]);
            if (w->alpha >= 0x100) { w->alpha = 0xFF; } w->core->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gSPDisplayList(gGfxDlHead++, gDrawSetupCldSurfDl);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0,
                              ENVIRONMENT, 0, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0,
                              ENVIRONMENT, 0);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->primR, w->primG, w->primB, 0);
            gDPSetEnvColor(gGfxDlHead++, w->envR, w->envG, w->envB, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            w->shell->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gSPDisplayList(gGfxDlHead++, gDrawSetupCldSurfDl);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0,
                              ENVIRONMENT, 0, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0,
                              ENVIRONMENT, 0);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->primR, w->primG, w->primB, 0);
            gDPSetEnvColor(gGfxDlHead++, w->envR, w->envG, w->envB, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            switch (w->size) {
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                    w->core->scale[0] *= 1.08;
                    w->core->scale[1] *= 1.08;
                    w->core->scale[2] *= 1.08;
                    w->shell->scale[0] *= 1.08;
                    w->shell->scale[1] *= 1.08;
                    w->shell->scale[2] *= 1.08;
                    w->core->rot[1] += 0.5235987902f;
                    w->shell->rot[1] -= 0.5235987902f;
                    if (w->time-- <= 0) {
                        return -1;
                    }
                    break;
                case 10:
                    w->core->scale[0] *= 1.08;
                    w->core->scale[1] *= 1.08;
                    w->core->scale[2] *= 1.08;
                    w->shell->scale[0] *= 1.08;
                    w->shell->scale[1] *= 1.08;
                    w->shell->scale[2] *= 1.08;
                    w->core->rot[1] += 0.5235987902f;
                    w->shell->rot[1] -= 0.5235987902f;
                    if (w->alpha > 10) {
                        w->alpha -= 10;
                    } else {
                        w->alpha = 0;
                        return -1;
                    }
                    if (w->time-- <= 0) {
                        return -1;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
