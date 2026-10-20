#include "tlb.h"
#include "tlb/tlb_0033.h"

extern s32 utaDemoFrame; /* the demo's frame count */
extern u8 demoPieceTex0[];
extern u8 demoPieceTex1[];
extern u8 demoPieceTex2[];
extern u8 demoPieceTex3[];
extern u8 demoPieceTex4[];
extern u8 demoPieceTex7[];
extern u8 demoPieceTex15[];
extern u8 demoPieceTex19[];
extern u8 demoPieceTex78[];
extern u8 demoFaceTex1[];
extern u8 demoFaceTex2[];
extern u8 utaSet0F3Tex0[];
extern u8 utaSet0F3Tex1[];
extern u8 utaDou2Tex0[];
extern u8 utaDou2Tex1[];

/* An anime model with an animated texture, run at a speed and shrinking. */
typedef struct AnimW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ Anim anim;
    /* 0x14 */ f32 speed;
    /* 0x18 */ f32 scale;
} AnimW;

extern s32 gDemoModelsDelete2; /* 1 deletes the demo's models */
extern u8 utaMach2ShrinkModelAnim[];
extern AnimFrame utaMach2ShrinkAnim[];

#define utaAnimeStep (*utaAnimeStep)

/* demotool.c's characters' animation table, a copy nothing reads; it starts tlb_0033_2's data. */
AnimFrame utaMach2CharaAnims[] = {
    { (u32) demoFaceTex2, 8 },
    { 0x00000000, 0 },
    { 0x04002100, 8 },
    { 0x04002940, 6 },
    { 0x04003180, 5 },
    { (u32) demoFaceTex1, 5 },
    { 0x04001080, 6 },
    { (u32) demoFaceTex1, 8 },
    { 0x04003180, 6 },
    { 0x04002940, 7 },
    { 0x04002100, 8 },
    { 0x00000000, 0 },
    { 0x04002520, 13 },
    { 0x04002D60, 13 },
    { 0x00000000, 0 },
    { 0x040035A0, 13 },
    { 0x00000000, 0 },
    { 0x03005350, 13 },
    { 0x00000000, 0 },
    { 0x03005770, 13 },
    { 0x00000000, 0 },
    { 0x03001570, 7 },
    { (u32) demoPieceTex0, 6 },
    { 0x03001570, 7 },
    { (u32) demoPieceTex2, 7 },
    { (u32) demoPieceTex1, 7 },
    { (u32) demoPieceTex0, 8 },
    { 0x03001570, 7 },
    { (u32) demoPieceTex2, 9 },
    { (u32) demoPieceTex3, 8 },
    { (u32) demoPieceTex4, 8 },
    { (u32) demoPieceTex1, 7 },
    { 0x03001570, 7 },
    { (u32) demoPieceTex2, 8 },
    { (u32) demoPieceTex1, 9 },
    { 0x03001570, 7 },
    { (u32) demoPieceTex0, 7 },
    { (u32) demoPieceTex2, 6 },
    { 0x03001570, 7 },
    { (u32) demoPieceTex0, 7 },
    { 0x03001570, 7 },
    { 0x00000000, 0 },
    { 0x03002E30, 7 },
    { 0x03001990, 6 },
    { 0x030021D0, 7 },
    { (u32) demoPieceTex7, 7 },
    { 0x03002E30, 7 },
    { 0x03001990, 8 },
    { 0x030021D0, 9 },
    { 0x030025F0, 8 },
    { 0x03002A10, 8 },
    { 0x03002E30, 7 },
    { (u32) demoPieceTex7, 7 },
    { 0x030021D0, 8 },
    { (u32) demoPieceTex7, 9 },
    { 0x03002E30, 7 },
    { 0x03001990, 7 },
    { 0x030021D0, 6 },
    { 0x03002E30, 7 },
    { 0x03001990, 7 },
    { 0x03002E30, 7 },
    { 0x00000000, 0 },
    { 0x030046F0, 7 },
    { 0x03003250, 6 },
    { 0x030046F0, 7 },
    { 0x03003A90, 7 },
    { 0x03003670, 7 },
    { 0x03003250, 8 },
    { 0x030046F0, 7 },
    { 0x03003A90, 9 },
    { (u32) demoPieceTex15, 8 },
    { 0x030042D0, 8 },
    { 0x030046F0, 7 },
    { 0x03003670, 7 },
    { 0x03003A90, 8 },
    { 0x030046F0, 7 },
    { 0x03003670, 9 },
    { 0x03003250, 7 },
    { 0x030046F0, 7 },
    { 0x03003A90, 6 },
    { 0x03003250, 7 },
    { 0x030046F0, 7 },
    { 0x00000000, 0 },
    { 0x03001570, 2 },
    { (u32) demoPieceTex3, 7 },
    { 0x03001570, 3 },
    { (u32) demoPieceTex1, 7 },
    { (u32) demoPieceTex2, 7 },
    { 0x03001570, 2 },
    { (u32) demoPieceTex0, 7 },
    { (u32) demoPieceTex2, 8 },
    { 0x00000000, 0 },
    { (u32) demoPieceTex4, 40 },
    { (u32) demoPieceTex2, 7 },
    { (u32) demoPieceTex1, 7 },
    { (u32) demoPieceTex0, 70 },
    { (u32) demoPieceTex4, 10 },
    { (u32) demoPieceTex4, 10 },
    { (u32) demoPieceTex0, 10 },
    { (u32) demoPieceTex3, 200 },
    { 0x00000000, 0 },
    { (u32) demoPieceTex19, 60 },
    { 0x03004B10, 7 },
    { (u32) demoPieceTex19, 10 },
    { 0x03004B10, 7 },
    { (u32) demoPieceTex19, 40 },
    { 0x03004B10, 7 },
    { (u32) demoPieceTex19, 40 },
    { 0x00000000, 0 },
    { (u32) utaSet0F3Tex0, 8 },
    { (u32) demoFaceTex1, 6 },
    { 0x04000C60, 5 },
    { 0x04001080, 5 },
    { 0x04001080, 8 },
    { 0x04000C60, 6 },
    { (u32) demoFaceTex1, 7 },
    { (u32) utaSet0F3Tex0, 8 },
    { 0x00000000, 0 },
    { 0x0300A5D0, 13 },
    { 0x00000000, 0 },
    { 0x040018C0, 13 },
    { 0x00000000, 0 },
    { 0x03007030, 2 },
    { 0x03006C10, 7 },
    { 0x03005FB0, 570 },
    { 0x00000000, 0 },
    { 0x03005B90, 7 },
    { 0x030063D0, 7 },
    { 0x03007030, 7 },
    { 0x03005FB0, 7 },
    { 0x03007030, 7 },
    { 0x03005B90, 7 },
    { 0x03007030, 7 },
    { 0x030063D0, 7 },
    { 0x030067F0, 7 },
    { 0x03006C10, 7 },
    { 0x03007030, 7 },
    { 0x03005FB0, 7 },
    { 0x030063D0, 7 },
    { 0x03005FB0, 7 },
    { 0x03007030, 7 },
    { 0x03005B90, 7 },
    { 0x030063D0, 7 },
    { 0x03007030, 7 },
    { 0x03005B90, 7 },
    { 0x03007030, 7 },
    { 0x00000000, 0 },
    { 0x03007450, 7 },
    { 0x03007C90, 7 },
    { 0x030088F0, 7 },
    { 0x03007870, 7 },
    { 0x030088F0, 7 },
    { 0x03007450, 7 },
    { 0x030088F0, 7 },
    { 0x03007C90, 7 },
    { 0x030080B0, 7 },
    { 0x030084D0, 7 },
    { 0x030088F0, 7 },
    { 0x03007870, 7 },
    { 0x03007C90, 7 },
    { 0x03007870, 7 },
    { 0x030088F0, 7 },
    { 0x03007450, 7 },
    { 0x03007C90, 7 },
    { 0x030088F0, 7 },
    { 0x03007450, 7 },
    { 0x030088F0, 7 },
    { 0x00000000, 0 },
    { 0x03008D10, 7 },
    { 0x03009550, 7 },
    { 0x0300A1B0, 7 },
    { 0x03009130, 7 },
    { 0x0300A1B0, 7 },
    { 0x03008D10, 7 },
    { 0x0300A1B0, 7 },
    { 0x03009550, 7 },
    { 0x03009970, 7 },
    { 0x03009D90, 7 },
    { 0x0300A1B0, 7 },
    { 0x03009130, 7 },
    { 0x03009550, 7 },
    { 0x03009130, 7 },
    { 0x0300A1B0, 7 },
    { 0x03008D10, 7 },
    { 0x03009550, 7 },
    { 0x0300A1B0, 7 },
    { 0x03008D10, 7 },
    { 0x0300A1B0, 7 },
    { 0x00000000, 0 },
    { 0x04000C60, 8 },
    { 0x04001080, 6 },
    { 0x04001CE0, 5 },
    { 0x04002520, 5 },
    { 0x04002520, 8 },
    { 0x04001CE0, 6 },
    { 0x04001080, 7 },
    { 0x04000C60, 8 },
    { 0x00000000, 0 },
    { 0x040018C0, 13 },
    { 0x00000000, 0 },
    { 0x040014A0, 13 },
    { 0x00000000, 0 },
    { 0x0300A9F0, 3 },
    { 0x0300B230, 4 },
    { 0x0300AE10, 4 },
    { 0x0300A9F0, 3 },
    { 0x0300B230, 4 },
    { 0x0300B650, 4 },
    { 0x0300BA70, 2 },
    { 0x0300AE10, 4 },
    { 0x0300B230, 4 },
    { 0x0300AE10, 4 },
    { 0x0300A9F0, 3 },
    { 0x0300B230, 4 },
    { 0x0300A9F0, 3 },
    { 0x00000000, 0 },
    { 0x0300C2B0, 3 },
    { 0x0300CAF0, 4 },
    { 0x0300C6D0, 4 },
    { 0x0300C2B0, 3 },
    { 0x0300CAF0, 4 },
    { 0x0300CF10, 4 },
    { 0x0300D330, 2 },
    { 0x0300C6D0, 4 },
    { 0x0300CAF0, 4 },
    { 0x0300C6D0, 4 },
    { 0x0300C2B0, 3 },
    { 0x0300CAF0, 4 },
    { 0x0300C2B0, 3 },
    { 0x00000000, 0 },
    { 0x0300DB70, 3 },
    { 0x0300E3B0, 4 },
    { 0x0300DF90, 4 },
    { 0x0300DB70, 3 },
    { 0x0300E3B0, 4 },
    { 0x0300E7D0, 4 },
    { 0x0300EBF0, 2 },
    { 0x0300DF90, 4 },
    { 0x0300E3B0, 4 },
    { 0x0300DF90, 4 },
    { 0x0300DB70, 3 },
    { 0x0300E3B0, 4 },
    { 0x0300DB70, 3 },
    { 0x00000000, 0 },
    { (u32) demoFaceTex1, 52 },
    { 0x04000C60, 1 },
    { 0x040014A0, 2 },
    { 0x04002100, 4 },
    { (u32) demoFaceTex1, 52 },
    { 0x04000C60, 1 },
    { 0x040014A0, 2 },
    { 0x04002100, 4 },
    { (u32) demoFaceTex1, 5 },
    { 0x04000C60, 1 },
    { 0x040014A0, 2 },
    { 0x04002100, 4 },
    { (u32) demoFaceTex1, 42 },
    { 0x00000000, 0 },
    { (u32) demoFaceTex1, 10 },
    { 0x04000C60, 10 },
    { 0x040014A0, 400 },
    { 0x040018C0, 3 },
    { 0x04001080, 3 },
    { (u32) demoFaceTex2, 3 },
    { 0x04001080, 3 },
    { 0x040018C0, 3 },
    { 0x04002520, 3 },
    { 0x04002520, 3 },
    { 0x00000000, 0 },
    { (u32) demoPieceTex78, 13 },
    { 0x00000000, 0 },
    { 0x030146B0, 13 },
    { 0x00000000, 0 },
    { 0x0300F850, 6 },
    { 0x03010CF0, 4 },
    { 0x03010090, 8 },
    { 0x03010CF0, 4 },
    { 0x0300FC70, 10 },
    { 0x03010CF0, 4 },
    { 0x0300F850, 12 },
    { 0x03010090, 8 },
    { 0x030104B0, 8 },
    { 0x03010CF0, 4 },
    { 0x030108D0, 8 },
    { 0x03010CF0, 3 },
    { 0x00000000, 0 },
    { 0x03011110, 4 },
    { 0x030125B0, 2 },
    { 0x03011950, 5 },
    { 0x030125B0, 2 },
    { 0x03011530, 6 },
    { 0x030125B0, 2 },
    { 0x03011110, 6 },
    { 0x03011950, 5 },
    { 0x03011D70, 5 },
    { 0x030125B0, 4 },
    { 0x03012190, 2 },
    { 0x030125B0, 2 },
    { 0x00000000, 0 },
    { 0x030125B0, 2 },
    { 0x03012190, 15 },
    { 0x03011950, 15 },
    { 0x03011530, 16 },
    { 0x03011950, 15 },
    { 0x03011530, 400 },
    { 0x00000000, 0 },
    { 0x030125B0, 2000 },
    { 0x00000000, 0 },
    { 0x030129D0, 4 },
    { 0x03013E70, 2 },
    { 0x03013210, 5 },
    { 0x03013E70, 2 },
    { 0x03012DF0, 6 },
    { 0x03013E70, 2 },
    { 0x030129D0, 6 },
    { 0x03013210, 5 },
    { 0x03013630, 5 },
    { 0x03013E70, 4 },
    { 0x03013A50, 2 },
    { 0x03013E70, 2 },
    { 0x00000000, 0 },
    { 0x03014AD0, 2000 },
    { 0x00000000, 0 },
};
/* A display list nothing reads. */
Gfx utaMach2UnusedDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetCombineLERP(ENVIRONMENT, TEXEL0, ENV_ALPHA, TEXEL0, TEXEL0, 0, SHADE, 0, ENVIRONMENT, TEXEL0, ENV_ALPHA, TEXEL0, TEXEL0, 0, SHADE, 0),
    gsDPSetEnvColor(0x00, 0xFF, 0xFF, 0xFF),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPPipeSync(),
    gsSPEndDisplayList(),
};
AnimFrame utaMach2ShrinkAnim[] = {
    { 0x03002F60, 1 },
    { 0, 0 },
};
AnimFrame utaSet0F3Anim[] = {
    { (u32) utaSet0F3Tex0, 15 },
    { (u32) utaSet0F3Tex1, 15 },
    { 0, 0 },
};
/* Read as AnimFrame (fadeAnimStart); its fifth word is not a whole frame. */
u32 utaDou2Anim[] = { (u32) utaDou2Tex0, 0x00070000, (u32) utaDou2Tex1, 0x000C0000, 0x00000080 };
s32 utaBisuUnusedFlag = 1;
Gfx utaFilterDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetEnvColor(0xFF, 0xFF, 0xFF, 0xC0),
    gsDPSetCombineLERP(0, 0, 0, SHADE, SHADE, 0, ENVIRONMENT, 0, 0, 0, 0, SHADE, SHADE, 0, ENVIRONMENT, 0),
    gsSPEndDisplayList(),
};
Gfx utaGhostDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_DISABLE),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetCombineLERP(0, 0, 0, TEXEL0, 0, 0, 0, ENVIRONMENT, 0, 0, 0, TEXEL0, 0, 0, 0, ENVIRONMENT),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetTextureFilter(G_TF_POINT),
    gsSPEndDisplayList(),
};

/* "MACH2": shrinks by 0.97 each frame. Without the FAKE boundary in case 2 the zero takes f0 and
 * 1.0 f2 (32 lines); tried an int "0" for the frame (43), a pos loop (50). */
s32 utaMach2ShrinkProc(Obj *obj, s16 msg, void *arg) {
    AnimW *w = (AnimW *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "MACH2";
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            demoAnimeSet(w->frame, (u32) utaMach2ShrinkModelAnim, 0xCF, 0);
            fadeAnimStart(&w->anim, utaMach2ShrinkAnim, 0, &w->frame->segs[3], floadGetSetFile(0x128, 0));
            w->frame->flags |= 0x30;
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 0.0f;
            if (1) {} /* FAKE: a block boundary; it gives 1.0 f0 and the zero f2 */
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale = 1.0f;
            w->speed = 0.76f; w->frame->frame = 0.0f;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 0:
            if (gDemoModelsDelete2) {
                return -1;
            }
            utaAnimeStep(w->frame, w->speed, 0);
            w->scale *= 0.97;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale;
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *utaMach2ShrinkCreate(Obj *obj) {
    return tskCreate(obj, utaMach2ShrinkProc, obj->file, NULL);
}

/* A task whose work is one pol frame. */
typedef struct PolW {
    /* 0x00 */ PolFrame *frame;
} PolW;

extern u8 utaHaikeiDl[];

/* "haikei" (the background): a pol model turning slowly about z. */
s32 utaHaikeiProc(Obj *obj, s16 msg, void *arg) {
    PolW *w = (PolW *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "haikei";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, (Gfx *) utaHaikeiDl, 0xCF);
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 0.0f;
            w->frame->scale[0] = 1.2f;
            w->frame->scale[1] = 1.2f;
            w->frame->scale[2] = 1.2f;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 3:
            w->frame->dl = demoShadeDL();
            break;
        case 0:
            if (gDemoModelsDelete2) {
                return -1;
            }
            w->frame->rot[2] -= 0.0174532930038800785;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *utaHaikeiCreate(Obj *obj) {
    return tskCreate(obj, utaHaikeiProc, obj->file, NULL);
}

/* "KIRAN"'s work: two sparkle frames growing together. */
typedef struct KiranW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ PolFrame *frame2;
    /* 0x08 */ u8 pad08[4];
    /* 0x0C */ f32 scale;
    /* 0x10 */ s32 timer;
    /* 0x14 */ s32 alpha;
    /* 0x18 */ s32 state;
} KiranW;

extern u8 utaKiranDl[];
extern u8 utaKiranGlowDl[];

/* "KIRAN": two sparkles at (0, 0, 100) growing by 1.2 a frame and brightening. */
s32 utaKiranPairProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    KiranW *w = (KiranW *) (obj + 1);
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "KIRAN";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, (Gfx *) utaKiranDl, 0xC6);
            w->frame->flags |= 7;
            w->frame2 = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame2 == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame2, (Gfx *) utaKiranGlowDl, 0xC6);
            w->frame2->flags |= 7;
            w->frame->pos[0] = w->frame2->pos[0] = 0.0f;
            w->frame->pos[1] = w->frame2->pos[1] = 0.0f;
            w->frame->pos[2] = w->frame2->pos[2] = 100.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale = 0.9f;
            w->frame2->scale[0] = w->frame2->scale[1] = w->frame2->scale[2] = w->scale;
            w->alpha = 0x80;
            w->timer = 0;
            w->state = 0;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            frmDestroy((Frame *) w->frame2);
            break;
        case 3:
            w->frame->dl = demoColorDL(255, 255, 255, 160, 160, 255, w->alpha);
            w->frame2->dl = demoColorDL(255, 255, 255, 160, 160, 255, w->alpha);
            break;
        case 0:
            if (gDemoModelsDelete2) {
                return -1;
            }
            w->timer++;
            w->scale *= 1.2;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale;
            w->frame2->scale[0] = w->frame2->scale[1] = w->frame2->scale[2] = w->scale;
            w->alpha++;
            if (w->alpha >= 0x100) {
                w->alpha = 0xFF;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *utaKiranPairCreate(Obj *obj) {
    return tskCreate(obj, utaKiranPairProc, obj->file, NULL);
}

/* "jimen"'s work. */
typedef struct JimenScrollW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ u8 pad04[0x10];
    /* 0x14 */ f32 speed;
    /* 0x18 */ u8 pad18[4];
    /* 0x1C */ s32 scroll; /* the texture's s offset, 0-31 */
    /* 0x20 */ u8 pad20[4];
    /* 0x24 */ s32 state;
} JimenScrollW;

extern Frame *gFrmLayer3;
extern u8 utaJimenScrollAnim[];
extern u8 utaJimenScrollTex[]; /* a 32x64 RGBA16 texture */

/* "jimen" (the ground): an anime model on layer 3 whose 32x64 texture scrolls by 3 a frame until
 * demo frame 0x2BE; at 0x320 it starts animating, and at 0x3D4 it deletes itself. */
s32 utaJimenScrollProc(Obj *obj, s16 msg, void *arg) {
    JimenScrollW *w = (JimenScrollW *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "jimen";
            w->frame = (AnimeFrame *) frmCreate(gFrmLayer3, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            demoAnimeSet(w->frame, (u32) utaJimenScrollAnim, 0xD0, 0.0f);
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 0.0f;
            w->frame->scale[0] = 5.0f;
            w->frame->scale[1] = 5.0f;
            w->frame->scale[2] = 5.0f;
            w->speed = 0.2f;
            w->scroll = 0;
            w->state = 0;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetTextureImage(gGfxDlHead++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, utaJimenScrollTex);
            gDPSetTile(gGfxDlHead++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_MIRROR | G_TX_WRAP, 6, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD);
            gDPLoadSync(gGfxDlHead++);
            gDPLoadBlock(gGfxDlHead++, G_TX_LOADTILE, 0, 0, 32 * 64 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES));
            gDPPipeSync(gGfxDlHead++);
            gDPSetTile(gGfxDlHead++, G_IM_FMT_RGBA, G_IM_SIZ_16b, ((32 * G_IM_SIZ_16b_LINE_BYTES) + 7) >> 3, 0, G_TX_RENDERTILE, 0, G_TX_MIRROR | G_TX_WRAP, 6, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD);
            gDPSetTileSize(gGfxDlHead++, G_TX_RENDERTILE, w->scroll, 0, (32 - 1) << G_TEXTURE_IMAGE_FRAC, (64 - 1) << G_TEXTURE_IMAGE_FRAC);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            if (gDemoModelsDelete2) {
                return -1;
            }
            switch (w->state) {
                case 0:
                    if (utaDemoFrame < 0x2BE) {
                        w->scroll += 3;
                    }
                    if (w->scroll >= 32) {
                        w->scroll -= 32;
                    }
                    if (utaDemoFrame == 0x320) {
                        w->state++;
                    }
                    break;
                case 1:
                    demoAnimeStep(w->frame, 0.2f, 2);
                    if (utaDemoFrame == 0x3D4) {
                        PRINTF("地面消します\n");
                        return -1;
                    }
                    break;
                case 2:
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *utaJimenScrollCreate(Obj *obj) {
    return tskCreate(obj, utaJimenScrollProc, obj->file, NULL);
}

/* "BISU"'s work: as AnimW, with a state. */
typedef struct BisuAnimW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ Anim anim;
    /* 0x14 */ u8 pad14[4];
    /* 0x18 */ f32 scale;
    /* 0x1C */ u8 pad1C[8];
    /* 0x24 */ s32 state;
} BisuAnimW;

extern u8 utaBisuBigAnim[];
extern u8 utaBisuBigState1Anim[];
extern AnimFrame utaSet0F3Anim[];

/* "BISU" (Bismaru's big model): plays its first animation, splashes (sibuki) every 13 frames
 * from demo frame 0x2A3 to 0x2C4, switches to its second animation at 0x320 (scale 10), and
 * deletes itself at 0x3F3. */
s32 utaBisuBigProc(Obj *obj, s16 msg, void *arg) {
    BisuAnimW *w = (BisuAnimW *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "BISU";
            w->frame = (AnimeFrame *) frmCreate(gFrmLayer2, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            demoAnimeSet(w->frame, (u32) utaBisuBigAnim, 0xF4, 0);
            fadeAnimStart(&w->anim, utaSet0F3Anim, 0, &w->frame->segs[3], floadGetSetFile(0xF3, 0));
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 0.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale = 2.0f;
            w->state = 0;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, gDrawSetupEnvTintTexShadeDl);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 0);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            if (utaDemoFrame >= 0x2A3 && utaDemoFrame < 0x2C5 && utaDemoFrame % 13 == 0) {
                effectSibukiCreate(338.0f, -200.0f, 1642.0f, 3.0f, 2.0f, 0.01f, 0.01f, 0.01f, 0.01f, 0xD1, 0xCC, 0x93, 0xB7, 0xB2, 0xB2);
            }
            switch (w->state) {
                case 0:
                    if (demoAnimeStep(w->frame, 0.2f, 2)) {
                        w->state++;
                    }
                    break;
                case 1:
                    if (utaDemoFrame == 0x320) {
                        w->state++;
                        demoAnimeSet(w->frame, (u32) utaBisuBigState1Anim, 0xF4, 0);
                        w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale = 10.0f;
                        fadeAnimStart(&w->anim, utaSet0F3Anim, 0, &w->frame->segs[3], floadGetSetFile(0xF3, 0));
                    }
                    break;
                case 2:
                    demoAnimeStep(w->frame, 0.2f, 2);
                    if (utaDemoFrame == 0x3F3) {
                        return -1;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *utaBisuBigCreate(Obj *obj) {
    return tskCreate(obj, utaBisuBigProc, obj->file, NULL);
}

/* The three-machine "MACH2"'s work. */
typedef struct Mach3W {
    /* 0x00 */ AnimeFrame *frames[3];
    /* 0x0C */ Anim anims[3];
} Mach3W;

extern u8 utaMach2TrioAnim[];

/* "MACH2" (three machines): at demo frame 0x5F2 two of them are placed at x -1665 and 1665,
 * scaled by 6, and the third is hidden; deletes itself at 0x61A. */
s32 utaMach2TrioProc(Obj *obj, s16 msg, void *arg) {
    Mach3W *w = (Mach3W *) (obj + 1);
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "MACH2";
            for (i = 0; i < 3; i++) {
                w->frames[i] = (AnimeFrame *) frmCreate(gFrmLayers[2], gFrmKindAnime);
                if (w->frames[i] == NULL) {
                    return -1;
                }
                demoAnimeSet(w->frames[i], (u32) utaMach2TrioAnim, 0xF4, 0.0f);
                fadeAnimStart(&w->anims[i], utaSet0F3Anim, 0, &w->frames[i]->segs[3], floadGetSetFile(0xF3, 0));
                w->frames[i]->scale[0] = 1.0f;
                w->frames[i]->scale[1] = 1.0f;
                w->frames[i]->scale[2] = 1.0f;
            }
            w->frames[0]->pos[0] = 0.0f;
            w->frames[0]->pos[1] = 0.0f;
            w->frames[0]->pos[2] = 0.0f;
            w->frames[0]->rot[1] = 0.0f;
            w->frames[1]->pos[0] = -475.0f;
            w->frames[1]->pos[1] = 0.0f;
            w->frames[1]->pos[2] = 0.0f;
            w->frames[1]->rot[1] = 0.0f;
            w->frames[2]->pos[0] = 475.0f;
            w->frames[2]->pos[1] = 0.0f;
            w->frames[2]->pos[2] = 0.0f;
            w->frames[2]->rot[1] = 0.0f;
            break;
        case 1:
            for (i = 0; i < 3; i++) {
                frmDestroy((Frame *) w->frames[i]);
            }
            break;
        case 3:
            for (i = 0; i < 3; i++) {
                w->frames[i]->dl = gGfxDlHead;
                gSPDisplayList(gGfxDlHead++, gDrawSetupEnvTintTexShadeDl);
                gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 0);
                gSPEndDisplayList(gGfxDlHead++);
            }
            break;
        case 0:
            if (utaDemoFrame == 0x5F2) {
                for (i = 0; i < 3; i++) {
                    w->frames[i]->scale[0] = 6.0f;
                    w->frames[i]->scale[1] = 6.0f;
                    w->frames[i]->scale[2] = 6.0f;
                }
                w->frames[0]->pos[0] = -1665.0f;
                w->frames[0]->pos[1] = -18.10000038f;
                w->frames[0]->pos[2] = 433.0f;
                w->frames[0]->rot[0] = -0.8499753475f;
                w->frames[0]->rot[1] = 1.141445398f;
                w->frames[0]->rot[2] = -0.968657732f;
                w->frames[1]->pos[0] = 1665.0f;
                w->frames[1]->pos[1] = -18.10000038f;
                w->frames[1]->pos[2] = 433.0f;
                w->frames[1]->rot[0] = -0.8499753475f;
                w->frames[1]->rot[1] = -1.143190742f;
                w->frames[1]->rot[2] = 0.968657732f;
                w->frames[2]->f.flags &= ~2;
            }
            if (utaDemoFrame == 0x61A) {
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *utaMach2TrioCreate(Obj *obj) {
    return tskCreate(obj, utaMach2TrioProc, obj->file, NULL);
}

/* "DOU2"'s work. */
typedef struct Dou2W {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ AnimeFrame *frame1;
    /* 0x08 */ AnimeFrame *frame2;
    /* 0x0C */ Anim anim;
    /* 0x1C */ u8 pad1C[0x20];
    /* 0x3C */ u8 state;
} Dou2W;

extern u8 utaDou2ModelAnim[];
extern u8 utaDou2Frame1Anim[];
extern u8 utaDou2Frame2Anim[];

/* "DOU2": an anime model on layer 2 with an animated texture, and two models at (0, 320, 1590)
 * scaled by 12; the texture restarts at demo frame 0x62E, the two models animate from 0x683,
 * and it deletes itself at 0x6D8. */
s32 utaDou2Proc(Obj *obj, s16 msg, void *arg) {
    Dou2W *w = (Dou2W *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "DOU2";
            w->frame = (AnimeFrame *) frmCreate(gFrmLayer2, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            demoAnimeSet(w->frame, (u32) utaDou2ModelAnim, 0x29, 0);
            fadeAnimStart(&w->anim, (AnimFrame *) utaDou2Anim, 0, &w->frame->segs[3], floadGetSetFile(0x29, 0));
            w->frame1 = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            w->frame2 = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            demoAnimeSet(w->frame1, (u32) utaDou2Frame1Anim, 0x100, 0);
            demoAnimeSet(w->frame2, (u32) utaDou2Frame2Anim, 0x100, 0);
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 0.0f;
            w->frame1->pos[0] = w->frame2->pos[0] = 0.0f;
            w->frame1->pos[1] = w->frame2->pos[1] = 320.0f;
            w->frame1->pos[2] = w->frame2->pos[2] = 1590.0f;
            w->frame->scale[2] = 5.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame1->scale[2] = 12.0f;
            w->frame1->scale[0] = w->frame1->scale[1] = w->frame1->scale[2];
            w->frame2->scale[2] = 12.0f;
            w->frame2->scale[0] = w->frame2->scale[1] = w->frame2->scale[2];
            w->state = 0;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            frmDestroy((Frame *) w->frame1);
            frmDestroy((Frame *) w->frame2);
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, gDrawSetupEnvTintTexShadeDl);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 0);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (utaDemoFrame == 0x61A) {
                        w->state++;
                    }
                    break;
                case 1:
                    if (utaDemoFrame == 0x62E) {
                        fadeAnimStart(&w->anim, (AnimFrame *) utaDou2Anim, 1, &w->frame->segs[3], floadGetSetFile(0x29, 0));
                    }
                    if (demoAnimeStep(w->frame, 0.2f, 2)) {
                        fadeAnimStart(&w->anim, (AnimFrame *) utaDou2Anim, 1, &w->frame->segs[3], floadGetSetFile(0x29, 0));
                        w->state++;
                    }
                    break;
                case 2:
                    if (utaDemoFrame >= 0x683) {
                        demoAnimeStep(w->frame1, 0.2f, 2);
                        demoAnimeStep(w->frame2, 0.2f, 2);
                    }
                    if (utaDemoFrame == 0x6D8) {
                        return -1;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Starts a utaDou2Proc task under obj. */
Obj *utaDou2Create(Obj *obj) {
    return tskCreate(obj, utaDou2Proc, obj->file, NULL);
}

/* An empty function whose strings are an animated twin of "CUTCUT" (utaCutcutProc); its
 * body was compiled out, its strings stayed. FAKE: the strings as dead prints. */
void utaCutcutUnusedStub(void) {
    PRINTF("CUTCUT");
    PRINTF("EBIカット");
    PRINTF("YAEカット");
    PRINTF("GOEカット");
    PRINTF("GOEカットアニメ");
}

/* "CUTCUT"'s work: a pol frame and the cut it is on. */
typedef struct CutW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[0x14];
    /* 0x18 */ s32 state;
} CutW;

extern u8 utaCutcutDl[];
extern u8 utaCutcutState0Dl[];
extern u8 utaCutcutState1Dl[];
extern u8 utaCutcutState2Dl[];

/* "CUTCUT" (the cut-in model): changes its display list to Ebisumaru's, Yae's and Goemon's cut
 * at demo frames 0x406, 0x419 and 0x42D; deletes itself at 0x462. Each print comes first in its
 * if: after the state's increment it moved the state from a3 to v0. */
s32 utaCutcutProc(Obj *obj, s16 msg, void *arg) {
    CutW *w = (CutW *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "CUTCUT";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, (Gfx *) utaCutcutDl, 0xD0);
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 0.0f;
            w->frame->scale[2] = 1.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->state = 0;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (utaDemoFrame == 0x406) {
                        PRINTF("EBIカット");
                        w->state++;
                        demoPolSetDL(w->frame, (Gfx *) utaCutcutState0Dl, 0xD0);
                    }
                    break;
                case 1:
                    if (utaDemoFrame == 0x419) {
                        PRINTF("YAEカット");
                        w->state++;
                        demoPolSetDL(w->frame, (Gfx *) utaCutcutState1Dl, 0xD0);
                    }
                    break;
                case 2:
                    if (utaDemoFrame == 0x42D) {
                        PRINTF("GOEカット");
                        w->state++;
                        demoPolSetDL(w->frame, (Gfx *) utaCutcutState2Dl, 0xD0);
                    }
                    break;
                case 3:
                    if (utaDemoFrame == 0x462) {
                        PRINTF("GOEカットモデル");
                        w->state++;
                        return -1;
                    }
                    break;
                case 4:
                    if (utaDemoFrame == 0x488) {
                        return -1;
                    }
                    break;
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *utaCutcutCreate(Obj *obj) {
    return tskCreate(obj, utaCutcutProc, obj->file, NULL);
}

/* "bisu" ("ビスマル", Bismaru, Ebisumaru): an anime model. */
typedef struct BisuW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 speed;
    /* 0x08 */ u8 pad08[4];
    /* 0x0C */ s32 state;
} BisuW;

extern s32 utaBisuUnusedFlag;
extern u8 utaBisuAnim[];

s32 utaBisuProc(Obj *obj, s16 msg, void *arg) {
    BisuW *w = (BisuW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "bisu";
            PRINTF("ビスマル作ります\n");
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            demoAnimeSet(w->frame, (u32) utaBisuAnim, 0xC3, 0);
            w->state = 0;
            w->frame->scale[2] = 0.28f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = 302.0f;
            w->frame->pos[1] = 85.0f;
            w->frame->pos[2] = 5.0f;
            w->frame->rot[1] = -1.466076612f;
            w->speed = 0.0f;
            utaBisuUnusedFlag = 0;
            PRINTF("ビスマル作りました\n");
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 0:
            utaAnimeStep(w->frame, 0.2f, 0);
            w->frame->pos[1] += 10.0f;
            if (125.0f < w->frame->pos[1]) {
                w->frame->pos[1] = 125.0f;
            }
            if (utaDemoFrame == 0x708) {
                return -1;
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *utaBisuCreate(Obj *obj) {
    return tskCreate(obj, utaBisuProc, obj->file, NULL);
}

/* "BARA"'s work: twelve sparkle frames. */
typedef struct BaraW {
    /* 0x00 */ PolFrame *frames[12];
} BaraW;

extern u8 utaBaraDl[];

Obj *utaKiranStretchCreate(Obj *obj);
Obj *utaKiranDriftCreate(Obj *obj);
Obj *utaKiranDriftLowCreate(Obj *obj);

/* "BARA" (roses): twelve sparkles around (279, 157-167, -6 to 16), turning about z; it starts
 * "kiran" sparkles every 5, 10 and 6 demo frames (and twenty at once when made), and deletes
 * itself at 0x708. The starters return their task: declared void, the demo frame reloaded
 * after each call took v0 instead of v1. */
s32 utaBaraProc(Obj *obj, s16 msg, void *arg) {
    s32 i;
    BaraW *w = (BaraW *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "BARA";
            for (i = 0; i < 12; i++) {
                w->frames[i] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
                if (w->frames[i] == NULL) {
                    return -1;
                }
                demoPolSetDL(w->frames[i], (Gfx *) utaBaraDl, 0xC3);
                w->frames[i]->flags |= 2;
                if (i < 4) {
                    w->frames[i]->scale[2] = 0.25f;
                    w->frames[i]->scale[0] = w->frames[i]->scale[1] = w->frames[i]->scale[2];
                } else {
                    w->frames[i]->scale[2] = 0.14f;
                    w->frames[i]->scale[0] = w->frames[i]->scale[1] = w->frames[i]->scale[2];
                }
                w->frames[i]->pos[0] = 279.0f;
            }
            w->frames[0]->pos[1] = 157.0f;
            w->frames[0]->pos[2] = -6.0f;
            w->frames[1]->pos[1] = 157.0f;
            w->frames[1]->pos[2] = 16.0f;
            w->frames[2]->pos[1] = 167.0f;
            w->frames[2]->pos[2] = -6.0f;
            w->frames[3]->pos[1] = 167.0f;
            w->frames[3]->pos[2] = 16.0f;
            w->frames[4]->pos[1] = 167.0f;
            w->frames[4]->pos[2] = -2.0f;
            w->frames[5]->pos[1] = 157.0f;
            w->frames[5]->pos[2] = -2.0f;
            w->frames[6]->pos[1] = 167.0f;
            w->frames[6]->pos[2] = 11.0f;
            w->frames[7]->pos[1] = 157.0f;
            w->frames[7]->pos[2] = 11.0f;
            w->frames[8]->pos[1] = 161.0f;
            w->frames[8]->pos[2] = -6.0f;
            w->frames[9]->pos[1] = 163.0f;
            w->frames[9]->pos[2] = -6.0f;
            w->frames[10]->pos[1] = 161.0f;
            w->frames[10]->pos[2] = 16.0f;
            w->frames[11]->pos[1] = 163.0f;
            w->frames[11]->pos[2] = 16.0f;
            for (i = 0; i < 10; i++) {
                (*utaKiranDriftCreate)(obj);
                (*utaKiranDriftLowCreate)(obj);
            }
            break;
        case 1:
            for (i = 0; i < 12; i++) {
                frmDestroy((Frame *) w->frames[i]);
            }
            break;
        case 3:
            for (i = 0; i < 12; i++) {
                w->frames[i]->dl = gGfxDlHead;
                gDPPipeSync(gGfxDlHead++);
                gDPSetPrimColor(gGfxDlHead++, 0, 0, 0, 0, 0, 0);
                gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 255);
                gSPEndDisplayList(gGfxDlHead++);
            }
            break;
        case 0:
            if (utaDemoFrame % 5 == 0) {
                (*utaKiranStretchCreate)(obj);
            }
            if (utaDemoFrame % 10 == 0) {
                (*utaKiranDriftCreate)(obj);
            }
            if (utaDemoFrame % 6 == 0) {
                (*utaKiranDriftLowCreate)(obj);
            }
            if (utaDemoFrame == 0x708) {
                return -1;
            }
            for (i = 0; i < 12; i++) {
                w->frames[i]->rot[2] += 0.0174532930038800785;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *utaBaraCreate(Obj *obj) {
    return tskCreate(obj, utaBaraProc, obj->file, NULL);
}

/* "FBALL"'s work: two sparkle frames. */
typedef struct FballW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ PolFrame *frame2;
    /* 0x08 */ s32 alpha;
    /* 0x0C */ s32 unused0C; /* written 0 at its start, never read */
} FballW;

extern u8 utaFballDl[];

/* "FBALL" (a fireball): two sparkles at (278, 156), drifting in z, until demo frame 0x708. */
s32 utaFballProc(Obj *obj, s16 msg, void *arg) {
    FballW *w = (FballW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "FBALL";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, (Gfx *) utaFballDl, 0x120);
            w->frame->flags |= 7;
            w->frame2 = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame2 == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame2, (Gfx *) utaFballDl, 0x120);
            w->frame2->flags |= 7;
            w->frame->pos[0] = 278.0f;
            w->frame->pos[1] = 156.0f;
            w->frame->pos[2] = 15.0f;
            w->frame2->pos[0] = 278.0f;
            w->frame2->pos[1] = 156.0f;
            w->frame2->pos[2] = 2.0f;
            w->frame->scale[2] = 1.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame2->scale[2] = 0.7f;
            w->frame2->scale[0] = w->frame2->scale[1] = w->frame2->scale[2];
            w->alpha = 0x80;
            w->unused0C = 0;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            frmDestroy((Frame *) w->frame2);
            break;
        case 3:
            w->frame->dl = demoColorDL(255, 255, 255, 255, 255, 255, w->alpha);
            w->frame2->dl = demoColorDL(255, 255, 255, 255, 255, 255, w->alpha);
            break;
        case 0:
            w->frame->pos[2] += 0.2;
            w->frame2->pos[2] += 0.1;
            if (utaDemoFrame == 0x708) {
                return -1;
            }
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *utaFballCreate(Obj *obj) {
    return tskCreate(obj, utaFballProc, obj->file, NULL);
}

/* The two "kiran" (sparkle) tasks' argument: only the position their starters set. */
typedef struct Tlb33KiranArg {
    /* 0x00 */ u8 pad00[0x8];
    /* 0x08 */ f32 pos[3];
    /* 0x14 */ u8 pad14[0x8];
} Tlb33KiranArg;

/* The first "kiran"'s work: a sparkle and its glow. */
typedef struct Kiran1W {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ PolFrame *frame2; /* drawn at a sixth of the alpha, a quarter of the width */
    /* 0x08 */ u8 pad08[0xC];
    /* 0x14 */ s32 alpha;
    /* 0x18 */ s32 state; /* 0 brightens fast, 1 slowly, 2 fades */
} Kiran1W;

/* "kiran": a sparkle at the argument's position that stretches in x as it brightens to 255, then
 * fades, shrinking, and deletes itself. */
s32 utaKiranStretchProc(Obj *obj, s16 msg, void *arg) {
    Kiran1W *w = (Kiran1W *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "kiran";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame2 = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame2 == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, (Gfx *) utaKiranDl, 0xC6);
            demoPolSetDL(w->frame2, (Gfx *) utaKiranGlowDl, 0xC6);
            w->frame->flags |= 7;
            w->frame2->flags |= 7;
            w->frame->scale[2] = 0.04f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame2->scale[2] = 0.04f;
            w->frame2->scale[0] = w->frame2->scale[1] = w->frame2->scale[2];
            w->frame->pos[0] = ((Tlb33KiranArg *) arg)->pos[0];
            w->frame->pos[1] = ((Tlb33KiranArg *) arg)->pos[1];
            w->frame->pos[2] = ((Tlb33KiranArg *) arg)->pos[2];
            w->frame2->pos[0] = ((Tlb33KiranArg *) arg)->pos[0];
            w->frame2->pos[1] = ((Tlb33KiranArg *) arg)->pos[1];
            w->frame2->pos[2] = ((Tlb33KiranArg *) arg)->pos[2];
            w->alpha = 0x40;
            w->state = 0;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            frmDestroy((Frame *) w->frame2);
            break;
        case 3:
            w->frame->dl = demoColorDL(255, 255, 255, 222, 100, 100, w->alpha);
            w->frame2->dl = demoColorDL(255, 255, 255, 222, 100, 100, w->alpha / 6);
            break;
        case 0:
            if (utaDemoFrame == 0x708) {
                return -1;
            }
            switch (w->state) {
                case 0:
                    w->alpha += 32;
                    if (w->alpha > 224) {
                        w->state = 1;
                        w->alpha = 224;
                    } else {
                        w->frame->scale[0] += 0.2;
                        w->frame->scale[1] = w->frame->scale[0];
                        w->frame2->scale[0] = w->frame->scale[0] / 4;
                        w->frame2->scale[1] = w->frame2->scale[0];
                    }
                    break;
                case 1:
                    w->alpha += 2;
                    if (w->alpha >= 0x100) {
                        w->alpha = 0xFF;
                        w->state = 2;
                    } else {
                        w->frame->scale[0] += 0.02;
                        w->frame->scale[1] = w->frame->scale[0];
                        w->frame2->scale[0] = w->frame->scale[0] / 4;
                        w->frame2->scale[1] = w->frame2->scale[0];
                    }
                    break;
                case 2:
                    w->alpha -= 40;
                    if (w->alpha <= 0) {
                        w->alpha = 0;
                        return -1;
                    }
                    if (w->frame->scale[0] > 0.01) {
                        w->frame->scale[0] -= 0.02;
                        w->frame->scale[1] = w->frame->scale[0];
                        w->frame2->scale[0] = w->frame->scale[0] / 4;
                        w->frame2->scale[1] = w->frame2->scale[0];
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "kiran" (utaKiranStretchProc) at x 280, a random height (157.0 to 167.0) and depth
 * (-6.0 to 16.0) under obj. */
Obj *utaKiranStretchCreate(Obj *obj) {
    Tlb33KiranArg arg;

    arg.pos[0] = 280.0f;
    arg.pos[1] = fadeRandRange(1570, 1670) / 10.0f;
    arg.pos[2] = fadeRandRange(-60, 160) / 10.0f;
    return tskCreate(obj, utaKiranStretchProc, obj->file, &arg);
}

/* The second "kiran"'s work. */
typedef struct Kiran2W {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[0x10];
    /* 0x14 */ s32 alpha;
    /* 0x18 */ s32 state; /* 0 brightens, 1 fades */
} Kiran2W;

/* "kiran": a sparkle drifting in z that brightens to 128, then fades and deletes itself. */
s32 utaKiranDriftProc(Obj *obj, s16 msg, void *arg) {
    Kiran2W *w = (Kiran2W *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "kiran";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, (Gfx *) utaFballDl, 0x120);
            w->frame->flags |= 7;
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = ((Tlb33KiranArg *) arg)->pos[0];
            w->frame->pos[1] = ((Tlb33KiranArg *) arg)->pos[1];
            w->frame->pos[2] = ((Tlb33KiranArg *) arg)->pos[2];
            w->alpha = 0x40;
            w->state = 0;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 3:
            w->frame->dl = demoColorDL(255, 255, 255, 222, 100, 100, w->alpha);
            break;
        case 0:
            w->frame->pos[2] += 0.06;
            switch (w->state) {
                case 0:
                    w->alpha += 12;
                    if (w->alpha > 128) {
                        w->state = 1;
                        w->alpha = 128;
                    }
                    break;
                case 1:
                    w->alpha -= 6;
                    if (w->alpha <= 40) {
                        w->alpha = 0;
                        return -1;
                    }
                    break;
            }
            if (utaDemoFrame == 0x708) {
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The other "kiran" (utaKiranDriftProc), heights 157.0 to 160.0. */
Obj *utaKiranDriftCreate(Obj *obj) {
    Tlb33KiranArg arg;

    arg.pos[0] = 280.0f;
    arg.pos[1] = fadeRandRange(1570, 1600) / 10.0f;
    arg.pos[2] = fadeRandRange(-60, 160) / 10.0f;
    return tskCreate(obj, utaKiranDriftProc, obj->file, &arg);
}

/* The other "kiran", heights 157.0 to 159.0. */
Obj *utaKiranDriftLowCreate(Obj *obj) {
    Tlb33KiranArg arg;

    arg.pos[0] = 280.0f;
    arg.pos[1] = fadeRandRange(1570, 1590) / 10.0f;
    arg.pos[2] = fadeRandRange(-60, 160) / 10.0f;
    return tskCreate(obj, utaKiranDriftProc, obj->file, &arg);
}

/* "ZAKO4"'s work: a hidden leader frame and four enemies that follow it. */
typedef struct Zako4W {
    /* 0x00 */ PolFrame *frames[5];
} Zako4W;

extern u8 gDemoCamHold;
extern s32 gObjMoveSel; /* OBJMOVE: the model chosen, -1 for none */
extern f32 gObjMovePosX; /* the chosen model's position and angle, as shown */
extern f32 gObjMovePosY;
extern f32 gObjMovePosZ;
extern f32 gObjMoveRot;
extern f32 gObjMoveDx; /* the moves this frame: x, y, z, angle */
extern f32 gObjMoveDy;
extern f32 gObjMoveDz;
extern f32 gObjMoveDrot;
extern u8 utaZako4Frame1Dl[];
extern u8 utaZako4Frame2Dl[];
extern u8 utaZako4Frame3Dl[];
extern u8 utaZako4Frame4Dl[];

/* "ZAKO4": four enemies at (466, 0, -35) moving 2 a frame in -z; OBJMOVE's model 5 moves them
 * (the leader blinks while chosen). Deletes itself at demo frame 0x826. */
s32 utaZako4Proc(Obj *obj, s16 msg, void *arg) {
    s32 i;
    Zako4W *w = (Zako4W *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "ZAKO4";
            w->frames[0] = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frames[0] == NULL) {
                return -1;
            }
            for (i = 0; i < 4; i++) {
                w->frames[i + 1] = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
                if (w->frames[i + 1] == NULL) {
                    return -1;
                }
                w->frames[i + 1]->scale[0] = 1.0f;
                w->frames[i + 1]->scale[1] = 1.0f;
                w->frames[i + 1]->scale[2] = 1.0f;
                w->frames[i + 1]->pos[0] = 466.0f;
                w->frames[i + 1]->pos[1] = 0.0f;
                w->frames[i + 1]->pos[2] = -35.0f;
                w->frames[i + 1]->rot[1] = -1.39626348f;
            }
            w->frames[0]->pos[0] = w->frames[1]->pos[0];
            w->frames[0]->pos[1] = 0.0f;
            w->frames[0]->pos[2] = w->frames[1]->pos[2];
            w->frames[0]->rot[1] = w->frames[1]->rot[1];
            demoPolSetDL(w->frames[1], (Gfx *) utaZako4Frame1Dl, 0xFD);
            demoPolSetDL(w->frames[2], (Gfx *) utaZako4Frame2Dl, 0xFC);
            demoPolSetDL(w->frames[3], (Gfx *) utaZako4Frame3Dl, 0xFF);
            demoPolSetDL(w->frames[4], (Gfx *) utaZako4Frame4Dl, 0xFC);
            break;
        case 1:
            frmDestroy((Frame *) w->frames[0]);
            for (i = 0; i < 4; i++) {
                frmDestroy((Frame *) w->frames[i + 1]);
            }
            break;
        case 0:
            w->frames[0]->f.flags |= 2;
            if (gObjMoveSel == 5) {
                w->frames[0]->pos[0] += gObjMoveDx;
                w->frames[0]->pos[1] += gObjMoveDy;
                w->frames[0]->pos[2] += gObjMoveDz;
                w->frames[0]->rot[1] += gObjMoveDrot;
                gObjMovePosX = w->frames[0]->pos[0];
                gObjMovePosY = w->frames[0]->pos[1];
                gObjMovePosZ = w->frames[0]->pos[2];
                gObjMoveRot = w->frames[0]->rot[1];
                if (gGameFrameCount % 20 >= 10) {
                    w->frames[0]->f.flags |= 2;
                } else {
                    w->frames[0]->f.flags &= ~2;
                }
                for (i = 0; i < 4; i++) {
                    w->frames[i + 1]->pos[0] = w->frames[0]->pos[0];
                    w->frames[i + 1]->pos[1] = w->frames[0]->pos[1];
                    w->frames[i + 1]->pos[2] = w->frames[0]->pos[2];
                    w->frames[i + 1]->rot[1] = w->frames[0]->rot[1];
                }
            }
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            for (i = 0; i < 4; i++) {
                w->frames[i + 1]->pos[2] -= 2.0f;
                w->frames[0]->pos[2] = w->frames[1]->pos[2];
            }
            if (utaDemoFrame == 0x826) {
                return -1;
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *utaZako4Create(Obj *obj) {
    return tskCreate(obj, utaZako4Proc, obj->file, NULL);
}

extern u8 utaWaterTex[]; /* a 32x32 RGBA16 texture, in segment 4 */

/* Grey environment colour with alpha a, and one 32x32 RGBA16 texture in two tiles, scrolled by t
 * (tile 0 along both axes, tile 1 by (t + 10, -t)); as "water"'s in file 39. */
Gfx *utaWaterGfx(Gfx *gfx, u16 t, u8 a) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 128, 128, 128, a);
    gDPSetTextureImage(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, utaWaterTex); gDPSetTile(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD); gDPLoadSync(gfx++); gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES)); gDPPipeSync(gfx++); gDPSetTile(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, ((32 * G_IM_SIZ_16b_LINE_BYTES) + 7) >> 3, 0, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD); gDPSetTileSize(gfx++, G_TX_RENDERTILE, t, t, (32 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC);
    gDPSetTextureImage(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, utaWaterTex); gDPSetTile(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0x100, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD); gDPLoadSync(gfx++); gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES));
    gDPPipeSync(gfx++); gDPSetTile(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, ((32 * G_IM_SIZ_16b_LINE_BYTES) + 7) >> 3, 0x100, 1, 0, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD); gDPSetTileSize(gfx++, 1, t + 10, -t, (32 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC);
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* "water"'s work. */
typedef struct WaterW {
    /* 0x00 */ u8 pad00[0x20];
    /* 0x20 */ f32 rotY;
    /* 0x24 */ u8 pad24[0x10];
    /* 0x34 */ PolFrame *frame;
    /* 0x38 */ Frame *frame2;
    /* 0x3C */ s32 scroll; /* the texture's scroll (its low half, as a u16) */
    /* 0x40 */ u8 alpha;
    /* 0x41 */ u8 pad41[0x15];
    /* 0x56 */ u8 state;
} WaterW;

extern Gfx utaWaterDl[];

/* "water": a pol frame on layer 6 drawn with the scrolling texture of utaWaterGfx;
 * fades in from demo frame 0x826 and sinks from 0x8C1; deletes itself at 0x95D. */
s32 utaWaterProc(Obj *obj, s16 msg, void *arg) {
    WaterW *w = (WaterW *) (obj + 1);
    s32 pad[2];
    //! @bug ang is never set: the game reads whatever its stack word holds.
    s32 ang;
#ifdef AVOID_UB
    ang = 0;
#endif

    switch (msg) {
        case 2:
            obj->name = "water";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            frmMove((Frame *) w->frame, gFrmLayers[6]);
            w->frame->dl2 = utaWaterDl;
            floadGetSetRam((void **) w->frame->segs, 0x54);
            w->frame->scale[2] = 10.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->rotY = (ang >> 16) * 3.141592741f / 512;
            w->frame->pos[1] = -300.0f;
            w->alpha = 0;
            w->state = 0;
            break;
        case 1:
            if (w->frame2 != NULL) { frmDestroy(w->frame2); }
            if (w->frame != NULL) { frmDestroy((Frame *) w->frame); }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gGfxDlHead = utaWaterGfx(gGfxDlHead, w->scroll, w->alpha);
            if (!(utaDemoFrame % 2)) {
                w->scroll--;
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (utaDemoFrame == 0x826) {
                        w->state++;
                    }
                    break;
                case 1:
                    w->alpha++;
                    if (w->alpha > 100) {
                        w->alpha = 100;
                    }
                    if (utaDemoFrame >= 0x8C1) {
                        w->frame->pos[1] -= 43.0f;
                    }
                    if (utaDemoFrame == 0x95D) {
                        return -1;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *utaWaterCreate(Obj *obj) {
    return tskCreate(obj, utaWaterProc, obj->file, NULL);
}

/* "ZAKO10"'s work: ten anime frames. */
typedef struct Zako10W {
    /* 0x00 */ AnimeFrame *frames[10];
    /* 0x28 */ u8 unused28; /* written 0 at its start, never read */
} Zako10W;

extern u8 utaZako10Frame0Anim[];
extern u8 utaZako10Frame1Anim[];
extern u8 utaZako10Frame2Anim[];
extern u8 utaZako10Frame3Anim[];
extern u8 utaZako10Frame4Anim[]; /* file set 0xFF's: utaDou2Frame1Anim's address in another file */
extern u8 utaZako10Frame5Anim[];
extern u8 utaZako10Frame6Anim[];
extern u8 utaZako10Frame7Anim[];
extern u8 utaZako10Frame8Anim[];
extern u8 utaZako10Frame9Anim[];

/* "ZAKO10": ten small enemies (anime models) at height -800, drawn decal; those with an
 * animation fall towards 0 (y * 0.8 a frame), the first four always, the rest from demo frame
 * 0x96B. Deletes itself at 0x9B1. */
s32 utaZako10Proc(Obj *obj, s16 msg, void *arg) {
    s32 i;
    Zako10W *w = (Zako10W *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "ZAKO10";
            for (i = 0; i < 10; i++) {
                w->frames[i] = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
                if (w->frames[i] == NULL) {
                    return -1;
                }
                w->frames[i]->scale[0] = 1.0f;
                w->frames[i]->scale[1] = 1.0f;
                w->frames[i]->scale[2] = 1.0f;
                w->frames[i]->pos[0] = 0.0f;
                w->frames[i]->pos[1] = -800.0f;
                w->frames[i]->pos[2] = 0.0f;
                w->frames[i]->rot[1] = 0.0f;
            }
            PRINTF("MODEL0Set\n");
            demoAnimeSet(w->frames[0], (u32) utaZako10Frame0Anim, 0xFC, 0.0f);
            PRINTF("MODEL1Set\n");
            demoAnimeSet(w->frames[1], (u32) utaZako10Frame1Anim, 0x101, 0.0f);
            PRINTF("MODEL2Set\n");
            demoAnimeSet(w->frames[2], (u32) utaZako10Frame2Anim, 0x103, 0.0f);
            PRINTF("MODEL3Set\n");
            demoAnimeSet(w->frames[3], (u32) utaZako10Frame3Anim, 0x102, 0.0f);
            PRINTF("MODEL4Set\n");
            demoAnimeSet(w->frames[4], (u32) utaZako10Frame4Anim, 0xFF, 0.0f);
            PRINTF("MODEL5Set\n");
            demoAnimeSet(w->frames[5], (u32) utaZako10Frame5Anim, 0x104, 0.0f);
            demoAnimeSet(w->frames[6], (u32) utaZako10Frame6Anim, 0x107, 0.0f);
            PRINTF("MODEL7Set\n");
            demoAnimeSet(w->frames[7], (u32) utaZako10Frame7Anim, 0x105, 0.0f);
            PRINTF("MODEL8Set\n");
            demoAnimeSet(w->frames[8], (u32) utaZako10Frame8Anim, 0xFD, 0.0f);
            PRINTF("MODEL9Set\n");
            demoAnimeSet(w->frames[9], (u32) utaZako10Frame9Anim, 0xFE, 0.0f);
            w->unused28 = 0;
            break;
        case 1:
            for (i = 0; i < 10; i++) {
                frmDestroy((Frame *) w->frames[i]);
            }
            break;
        case 3:
            for (i = 0; i < 10; i++) {
                if (w->frames[i]->anim != 0) {
                    w->frames[i]->dl = gGfxDlHead;
                    gDPPipeSync(gGfxDlHead++);
                    gDPSetCombineMode(gGfxDlHead++, G_CC_DECALRGBA, G_CC_DECALRGBA);
                    gSPEndDisplayList(gGfxDlHead++);
                }
            }
            break;
        case 0:
            for (i = 0; i < 4; i++) {
                if (w->frames[i]->anim != 0) {
                    w->frames[i]->pos[1] *= 0.8;
                }
            }
            if (utaDemoFrame >= 0x96B) {
                for (i = 0; i < 10; i++) {
                    if (w->frames[i]->anim != 0) {
                        w->frames[i]->pos[1] *= 0.8;
                    }
                }
            }
            if (utaDemoFrame == 0x9B1) {
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *utaZako10Create(Obj *obj) {
    return tskCreate(obj, utaZako10Proc, obj->file, NULL);
}

/* "BOSS3"'s work: three anime models, each with its own alpha. */
typedef struct Boss3W {
    /* 0x00 */ AnimeFrame *frames[3];
    /* 0x0C */ s32 alpha[3];
    /* 0x18 */ u8 state;
} Boss3W;

extern u8 utaBoss3Frame0Anim[];
extern u8 utaBoss3Frame1Anim[];
extern u8 utaBoss3Frame2Anim[];

/* "BOSS3": three models that rise and fade one after another (the next one shown at demo frames
 * 0x9D1 and 0x9F6); all rise from 0xA01, and it deletes itself at 0xA56. */
s32 utaBoss3Proc(Obj *obj, s16 msg, void *arg) {
    s32 i;
    Boss3W *w = (Boss3W *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "BOSS3";
            for (i = 0; i < 3; i++) {
                w->frames[i] = (AnimeFrame *) frmCreate(gFrmLayer2, gFrmKindAnime);
                if (w->frames[i] == NULL) {
                    return -1;
                }
                w->alpha[i] = 0xFF;
                w->frames[i]->f.flags &= ~2;
            }
            w->frames[0]->scale[2] = 4.900000095f;
            w->frames[0]->scale[0] = w->frames[0]->scale[1] = w->frames[0]->scale[2];
            w->frames[0]->pos[0] = -358.0f;
            w->frames[0]->pos[1] = -240.0f;
            w->frames[0]->pos[2] = -1291.0f;
            w->frames[0]->rot[0] = 2.81347084f;
            w->frames[0]->rot[1] = 2.426007748f;
            w->frames[0]->rot[2] = 3.036872864f;
            w->frames[1]->scale[2] = 1.0f;
            w->frames[1]->scale[0] = w->frames[1]->scale[1] = w->frames[1]->scale[2];
            w->frames[1]->pos[0] = 0.0f;
            w->frames[1]->pos[1] = -20.0f;
            w->frames[1]->pos[2] = 0.0f;
            w->frames[1]->rot[0] = 0.0f;
            w->frames[1]->rot[1] = 0.0f;
            w->frames[1]->rot[2] = 0.0f;
            w->frames[2]->scale[2] = 1.0f;
            w->frames[2]->scale[0] = w->frames[2]->scale[1] = w->frames[2]->scale[2];
            w->frames[2]->pos[0] = -207.0f;
            w->frames[2]->pos[1] = 208.0f;
            w->frames[2]->pos[2] = 2534.0f;
            w->frames[2]->rot[0] = 0.0f;
            w->frames[2]->rot[1] = 3.141592741f;
            w->frames[2]->rot[2] = 0.0f;
            demoAnimeSet(w->frames[0], (u32) utaBoss3Frame0Anim, 0xD5, 0.0f);
            demoAnimeSet(w->frames[1], (u32) utaBoss3Frame1Anim, 0xD3, 0.0f);
            demoAnimeSet(w->frames[2], (u32) utaBoss3Frame2Anim, 0xD4, 0.0f);
            w->frames[0]->f.flags |= 2;
            w->state = 0;
            break;
        case 1:
            for (i = 0; i < 3; i++) {
                frmDestroy((Frame *) w->frames[i]);
            }
            break;
        case 3:
            for (i = 0; i < 3; i++) {
                w->frames[i]->dl = gGfxDlHead;
                gSPDisplayList(gGfxDlHead++, gDrawSetupEnvTintTexShadeDl);
                gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha[i]);
                gSPEndDisplayList(gGfxDlHead++);
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->alpha[0] -= 10;
                    w->frames[0]->pos[1] += 1.0;
                    if (utaDemoFrame == 0x9D1) {
                        w->frames[1]->f.flags |= 2;
                        w->state++;
                    }
                    break;
                case 1:
                    w->frames[0]->pos[1] += 0.8;
                    w->frames[1]->pos[1] += 0.5;
                    w->alpha[1] -= 10;
                    if (utaDemoFrame == 0x9F6) {
                        w->frames[2]->f.flags |= 2;
                        w->state++;
                    }
                    break;
                case 2:
                    w->alpha[2] -= 10;
                    if (utaDemoFrame >= 0xA01) {
                        w->frames[0]->pos[1] += 0.4;
                        w->frames[1]->pos[1] += 0.3;
                        w->frames[2]->pos[1] += 0.1;
                    }
                    if (utaDemoFrame == 0xA56) {
                        return -1;
                    }
                    break;
            }
            for (i = 0; i < 3; i++) {
                if (w->alpha[i] < 0) {
                    w->alpha[i] = 0;
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *utaBoss3Create(Obj *obj) {
    return tskCreate(obj, utaBoss3Proc, obj->file, NULL);
}

/* "filter": a pol frame on layer 8 whose alpha rises after demo frame 0x809. */
typedef struct FilterW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ s32 alpha;
    /* 0x08 */ s32 unused08; /* written 0 at its start, never read */
    /* 0x0C */ s32 unused0C; /* written 0 at its start, never read */
} FilterW;

extern u8 utaFilterModelDl[];
extern Gfx utaFilterDl[];

s32 utaFilterProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    FilterW *w = (FilterW *) (obj + 1);
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "filter";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            demoPolSetDL(w->frame, (Gfx *) utaFilterModelDl, 0xD2);
            w->alpha = 0xA0;
            w->frame->dl = (Gfx *) fadeSegToRam(utaFilterDl, obj->file);
            w->unused0C = 0;
            w->unused08 = 0;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 0:
            if (utaDemoFrame >= 0x809) {
                w->alpha += 10;
            }
            if (w->alpha >= 0x100) {
                w->alpha = 0xFF;
            }
            if (utaDemoFrame == 0x826) {
                return -1;
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *utaFilterCreate(Obj *obj) {
    return tskCreate(obj, utaFilterProc, obj->file, NULL);
}

/* "ZAKO14"'s work: fourteen anime frames. */
typedef struct Zako14W {
    /* 0x00 */ AnimeFrame *frames[14];
} Zako14W;

extern u8 utaZako14Frame0Anim[];
extern u8 utaZako14Frame1Anim[];
extern u8 utaZako14Frame2Anim[];
extern u8 utaZako14Frame3Anim[];
extern u8 utaZako14Frame4Anim[];
extern u8 utaZako14Frame5Anim[];
extern u8 utaZako14Frame6Anim[];
extern u8 utaZako14Frame7Anim[];
extern u8 utaZako14Frame9Anim[];
extern u8 utaZako14Frame10Anim[];
extern u8 utaZako14Frame11Anim[]; /* file set 0x106's: utaZako10Frame5Anim's address in another file */
extern u8 utaZako14Frame12Anim[];
extern u8 utaZako14Frame13Anim[];

/* "ZAKO14": fourteen small enemies (anime models), drawn decal; frame 8 is left without an
 * animation. Deletes itself at demo frame 0xA9B. */
s32 utaZako14Proc(Obj *obj, s16 msg, void *arg) {
    s32 i;
    s32 pad;
    Zako14W *w = (Zako14W *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "ZAKO14";
            for (i = 0; i < 14; i++) {
                w->frames[i] = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
                if (w->frames[i] == NULL) {
                    return -1;
                }
                w->frames[i]->scale[0] = 1.0f;
                w->frames[i]->scale[1] = 1.0f;
                w->frames[i]->scale[2] = 1.0f;
                w->frames[i]->pos[0] = 0.0f;
                w->frames[i]->pos[1] = 0.0f;
                w->frames[i]->pos[2] = 0.0f;
            }
            demoAnimeSet(w->frames[0], (u32) utaZako14Frame0Anim, 0xFC, 0.0f);
            demoAnimeSet(w->frames[1], (u32) utaZako14Frame1Anim, 0xFC, 0.0f);
            demoAnimeSet(w->frames[2], (u32) utaZako14Frame2Anim, 0xFC, 0.0f);
            demoAnimeSet(w->frames[3], (u32) utaZako14Frame3Anim, 0xFC, 0.0f);
            demoAnimeSet(w->frames[4], (u32) utaZako14Frame4Anim, 0xFC, 0.0f);
            demoAnimeSet(w->frames[5], (u32) utaZako14Frame5Anim, 0xFC, 0.0f);
            demoAnimeSet(w->frames[6], (u32) utaZako14Frame6Anim, 0xFF, 0.0f);
            demoAnimeSet(w->frames[7], (u32) utaZako14Frame7Anim, 0xFF, 0.0f);
            PRINTF("ZAKOb14");
            demoAnimeSet(w->frames[9], (u32) utaZako14Frame9Anim, 0xFE, 0.0f);
            demoAnimeSet(w->frames[10], (u32) utaZako14Frame10Anim, 0xFE, 0.0f);
            demoAnimeSet(w->frames[11], (u32) utaZako14Frame11Anim, 0x106, 0.0f);
            demoAnimeSet(w->frames[12], (u32) utaZako14Frame12Anim, 0x106, 0.0f);
            demoAnimeSet(w->frames[13], (u32) utaZako14Frame13Anim, 0x107, 0.0f);
            break;
        case 1:
            for (i = 0; i < 14; i++) {
                frmDestroy((Frame *) w->frames[i]);
            }
            break;
        case 3:
            for (i = 0; i < 14; i++) {
                if (w->frames[i] != NULL) {
                    w->frames[i]->dl = gGfxDlHead;
                    gDPPipeSync(gGfxDlHead++);
                    gDPSetCombineMode(gGfxDlHead++, G_CC_DECALRGBA, G_CC_DECALRGBA);
                    gSPEndDisplayList(gGfxDlHead++);
                }
            }
            break;
        case 0:
            for (i = 0; i < 14; i++) {
                if (w->frames[i]->anim != 0) {
                    demoAnimeStep(w->frames[i], 0.14f, 2);
                }
            }
            if (utaDemoFrame == 0xA9B) {
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

void utaUnusedStub3(void) {
}

s32 utaZako14Proc(Obj *obj, s16 msg, void *arg);

Obj *utaZako14Create(Obj *obj) {
    return tskCreate(obj, utaZako14Proc, obj->file, NULL);
}

/* Draws a screen-sized RGBA16 picture (the previous framebuffer) in 64x30 tiles, darkened to
 * black by alpha: "ghost"'s draw. The same instructions as file 17's effGhostDrawImage. */
Gfx *utaGhostDrawImage(Gfx *gfx, u16 *img, s32 alpha) {
    s32 i;
    s32 j;
    s32 top;
    s32 left;

    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 0, 0, 0, alpha);
    for (j = 0; j < 8; j++) {
        for (i = 0; i < 5; i++) {
            left = i * 64;
            top = j * 30;
            gDPPipeSync(gfx++);
            gDPLoadTextureTile(gfx++, img, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, SCREEN_HEIGHT, left,
                               top, left + 63, top + 29, 0, G_TX_CLAMP, G_TX_CLAMP,
                               G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gSPTextureRectangle(gfx++, left << 2, top << 2, (left + 64) << 2, (top + 30) << 2,
                                G_TX_RENDERTILE, left << 5, top << 5, 1 << 10, 1 << 10);
        }
    }
    return gfx;
}

/* "ghost" (its proc utaGhostProc): its display-list frame and the word its starter
 * sets. */
typedef struct GhostW {
    /* 0x00 */ DLFrame *frame;
    /* 0x04 */ s32 alpha;
} GhostW;

extern Gfx utaGhostDl[];

Gfx *utaGhostDrawImage(Gfx *gfx, u16 *fb, s32 v);

s32 utaGhostProc(Obj *obj, s16 msg, void *arg) {
    GhostW *w = (GhostW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "ghost";
            w->frame = (DLFrame *) frmCreate(gFrmLayers[0], gFrmKindDlist);
            if (w->frame == NULL) {
                return -1;
            }
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, fadeSegToRam(utaGhostDl, obj->file));
            gGfxDlHead = utaGhostDrawImage(gGfxDlHead, gfxGetPrevFramebuffer(), w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Starts a "ghost" under obj with v in its work. */
Obj *utaGhostCreate(Obj *obj, s32 v) {
    Obj *o = tskCreate(obj, utaGhostProc, obj->file, NULL);

    if (o != NULL) {
        ((GhostW *) (o + 1))->alpha = v;
    }
    return o;
}

/* "JIMEN" (the ground): an anime frame on layer 8. */
typedef struct JimenW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ u8 pad04[0x14];
    /* 0x18 */ f32 scale;
} JimenW;

extern u8 utaJimenShadeAnim[];

s32 utaJimenShadeProc(Obj *obj, s16 msg, void *arg) {
    JimenW *w = (JimenW *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "JIMEN";
            w->frame = (AnimeFrame *) frmCreate(gFrmLayer8, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            demoAnimeSet(w->frame, (u32) utaJimenShadeAnim, 0xCE, 0.0f);
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 0.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale = 5.0f;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 3:
            w->frame->dl = demoShadeDL();
            break;
        case 0:
            demoAnimeStep(w->frame, 0.14f, 2);
            if (utaDemoFrame == 0xA9B) {
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *utaJimenShadeCreate(Obj *obj) {
    return tskCreate(obj, utaJimenShadeProc, obj->file, NULL);
}
