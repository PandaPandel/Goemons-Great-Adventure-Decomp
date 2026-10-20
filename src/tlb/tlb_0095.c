#include "tlb.h"

/* The "noru_mark" kind (archive file 95): a marker that turns on its spot, with a glow over it
 * that swells to full size and breathes. */

extern u8 noruMarkDl[];
extern u8 noruMarkGlowDl[];
Gfx noruMarkSetupDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPPipelineMode(G_PM_NPRIMITIVE),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetCombineKey(G_CK_NONE),
    gsDPSetTextureLOD(G_TL_TILE),
    gsDPSetTextureDetail(G_TD_CLAMP),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsDPSetTextureLUT(G_TT_NONE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_TEX_EDGE, G_RM_AA_TEX_EDGE2),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsSPEndDisplayList(),
    gsDPNoOp(),
}; /* the display list drawn before its models (fadeSegToRam) */

/* A noru_mark's work, at its task + 0x20: an enemy's (EneObj) up to 0x34, then its own. */
typedef struct NoruMark {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ PolFrame *model; /* the mark */
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *glow; /* the glow over it */
    /* 0x38 */ s16 alpha;
    /* 0x3A */ u8 unused3A;
    /* 0x3B */ u8 r;
    /* 0x3C */ u8 g;
    /* 0x3D */ u8 b;
    /* 0x40 */ s32 type; /* its placement's type, from eneSetup */
    /* 0x44 */ u16 time; /* steps since it was made */
} NoruMark;

#define NORUMARK(obj) ((NoruMark *) FX(obj))

PolFrame *noruMarkNewPol(Obj *o, Gfx *dl2);

s32 noruMarkProc(Obj *o, s16 msg, void *arg) {
    NoruMark *w = NORUMARK(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "noru_mark";
            w->type = eneSetup(o, arg);
            if ((w->model = (*noruMarkNewPol)(o, (Gfx *) noruMarkDl)) == NULL) {
                return -1;
            }
            w->model->flags |= 0x25;
            w->model->scale[2] = 0.150000006f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            if ((w->glow = (*noruMarkNewPol)(o, (Gfx *) noruMarkGlowDl)) == NULL) {
                return -1;
            }
            w->glow->scale[2] = 0.150000006f;
            w->glow->scale[0] = w->glow->scale[1] = w->glow->scale[2];
            w->glow->scale[0] = 0.0f;
            w->glow->flags |= 0x27;
            w->r = w->g = w->b = 0xFF;
            w->alpha = 0xFF;
            w->time = 0;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->glow != NULL) {
                frmDestroy(&w->glow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            w->glow->scale[0] += (f32) w->time / 100.0;
            if (w->glow->scale[0] > 0.149999999999999994) {
                w->glow->scale[0] = 0.150000006f;
            }
            if (w->time % 20 < 10) {
                w->glow->scale[1] += 0.00300000000000000006;
            } else {
                w->glow->scale[1] -= 0.00300000000000000006;
            }
            w->time++;
            w->rot[1] += 0.174532930038800771;
            eneMove((EneObj *) w);
            eneSetModelOnCourse(o, &w->model->f);
            w->glow->pos[0] = w->model->pos[0];
            w->glow->pos[1] = w->model->pos[1] + 15.0f;
            w->glow->pos[2] = w->model->pos[2];
            eneStepHookPost(o);
            break;
        case 3:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Makes one of its models: a pol frame on the effect layer at its place, a tenth size, drawn
 * after noruMarkSetupDl, with its kind's first three files as segments. */
PolFrame *noruMarkNewPol(Obj *o, Gfx *dl2) {
    EneObj *e = ENE(o);
    PolFrame *m;
    EneKind *k = e->kind;
    s32 pad[2];

    if ((m = (PolFrame *) frmCreate(gFrmEffectLayer, gFrmKindPol)) != NULL) {
        vramCoursePointXZ(e->zone, e->x, e->z, &m->pos[0], &m->pos[2]);
        m->pos[1] = e->y;
        m->scale[0] = m->scale[1] = m->scale[2] = 0.1000000015f;
        m->dl = (Gfx *) fadeSegToRam(noruMarkSetupDl, o->file);
        m->dl2 = dl2;
        m->segs[0] = fileRamAddr(k->files.file[0]);
        m->segs[1] = fileRamAddr(k->files.file[1]);
        m->segs[2] = fileRamAddr(k->files.file[2]);
    }
    return m;
}
