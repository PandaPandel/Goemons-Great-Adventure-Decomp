#include "tlb.h"
#include "tlb/tlb_0036.h"

/* The staff roll's task (file 36, made by main's staffM at 0x08000000) and its "dtex" picture
 * task. */

/* The "staffmain" task's work. */
typedef struct StaffWork {
    /* 0x00 */ Obj *self;
    /* 0x04 */ u8 pad4[4];
    /* 0x08 */ Obj *fade;
    /* 0x0C */ u8 padC[0x18];
    /* 0x24 */ s32 state;
    /* 0x28 */ u8 pad28[4];
    /* 0x2C */ s32 frames;
} StaffWork;

/* The "dtex" task's work: a picture drawn in 64x64 I4 tiles, two pages, faded out. */
typedef struct DtexWork {
    /* 0x00 */ DLFrame *frame;
    /* 0x04 */ f32 x;
    /* 0x08 */ f32 y;
    /* 0x0C */ u32 tex[2];
    /* 0x14 */ s32 page;
    /* 0x18 */ s32 w;
    /* 0x1C */ s32 h;
    /* 0x20 */ s32 alpha;
    /* 0x24 */ s32 state;
} DtexWork;

u32 staffUnusedGlobals[] = { 0x12345670, 0x04D204D2, 0x04D204D2, 0x04D204D2, 0x04D204D2, 0x00000000 };
Gfx staffDtexDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetTextureFilter(G_TF_POINT),
    gsDPSetCombineLERP(0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0),
    gsDPPipeSync(),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsSPEndDisplayList(),
};

void staffInit(Obj *obj);
void staffStep(Obj *obj);
Gfx *staffDrawI4Tiles(Gfx *gfx, f32 x, f32 y, f32 scale, s32 w, s32 h, u32 tex);

/* The "staffmain" task: made with an arg of -1 it skips its init. */
s32 staffMain(Obj *obj, s16 msg, void *arg) {
    StaffWork *w = (StaffWork *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "staffmain";
            if ((s32) arg != -1) {
                PRINTF("requested\n");
                staffInit(obj);
            }
            break;
        case 0:
            staffStep(obj);
            w->frames++;
            break;
        case 1:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

void staffInit(Obj *obj) {
    StaffWork *w = (StaffWork *) (obj + 1);
    u8 *p = (u8 *) w;
    s32 i;

    PRINTF("staffInit\n");
    /* A clear of the work that never steps its pointer: only the first byte is cleared. */
    for (i = 0; i < sizeof(StaffWork); i++) {
        *p = 0;
    }
    w->self = obj;
    w->state = 0;
    w->frames = 0;
    w->fade = fadeCreate(obj);
    PRINTF("staffInit end\n");
}

/* Its step: at once to stage 0x100, with overlay 9. */
void staffStep(Obj *obj) {
    StaffWork *w = (StaffWork *) (obj + 1);

    switch (w->state) {
        case 0:
            PRINTF("INIT\n");
            stageGoto(obj->parent, 0x100);
            gGameWork.nextOverlay = 9;
            w->state = 1;
            break;
        case 1:
            break;
    }
}

/* An I4 picture of w x h in 64x64 tiles at (x, y), scaled: pakfiler.c's pakFilerDrawI4Tiles, word
 * for word. Ends the list without stepping past the end command. */
Gfx *staffDrawI4Tiles(Gfx *gfx, f32 x, f32 y, f32 scale, s32 w, s32 h, u32 tex) {
    s32 tw = 64;
    s32 row;
    s32 nx = (w + 63) / 64;
    s32 ny = (h + 63) / 64;
    s32 col;
    s32 s;
    s32 t;
    s32 sr;
    s32 tb;
    s32 th = 64;

    for (row = 0; row < ny; row++) {
        for (col = 0; col < nx; col++) {
            t = row * th;
            s = col * tw;
            sr = s + tw;
            if (w < sr) {
                sr = w;
            }
            tb = t + th;
            if (h < tb) {
                tb = h;
            }
            gDPPipeSync(gfx++);
            gDPLoadTextureTile_4b(gfx++, tex, G_IM_FMT_I, w, h, s, t, sr - 1, tb - 1, 0, G_TX_CLAMP,
                                  G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gSPTextureRectangle(gfx++, (s32) ((s * scale + x) * 4.0f),
                                (s32) ((y + t * scale) * 4.0f), (s32) ((sr * scale + x) * 4.0f),
                                (s32) ((y + tb * scale) * 4.0f), G_TX_RENDERTILE, s << 5, t << 5,
                                (s32) ((1.0f / scale) * 1024.0f), (s32) ((1.0f / scale) * 1024.0f));
        }
    }
    gSPEndDisplayList(gfx);
    return gfx;
}

/* The "dtex" task: a picture in two pages (w->tex), flipped every frame while state is 0 or 1;
 * in 1 it also fades out by 5 a frame, then stops (2). Arg non-NULL: drawn in gFrmLayer2. (The
 * work pointer at its declaration keeps obj in its home; the game's frame has six words more.) */
s32 staffDtex(Obj *obj, s16 msg, void *arg) {
    DtexWork *w = (DtexWork *) ((u32) obj + sizeof(Obj));
    s32 pad[6];

    switch (msg) {
        case 2:
            obj->name = "dtex";
            w->frame = (DLFrame *) frmCreate(arg != NULL ? gFrmLayer2 : gFrmLayers[0], gFrmKindDlist);
            if (w->frame == NULL) {
                return -1;
            }
            w->page = 0;
            w->alpha = 0xFF;
            w->state = 2;
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->page ^= 1;
                    break;
                case 1:
                    w->page ^= 1;
                    w->alpha -= 5;
                    if (w->alpha <= 0) {
                        w->alpha = 0;
                        w->state = 2;
                    }
                    break;
                case 2:
                    break;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, fadeSegToRam(staffDtexDl, 0x24));
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            if (w->alpha >= 0xFF) {
                gDPSetRenderMode(gGfxDlHead++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
            } else {
                gDPSetRenderMode(gGfxDlHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
            }
            gGfxDlHead = staffDrawI4Tiles(gGfxDlHead, w->x, w->y, 1.0f, w->w, w->h, w->tex[w->page]);
            gGfxDlHead++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Five empty functions nothing calls; the game's asm folds them into the one above. The
 * game's .rodata also ends in three "" strings after "dtex": zero words, which the section's
 * 16-byte padding gives as well. */
void staffEmpty1(void) {
}

void staffEmpty2(void) {
}

void staffEmpty3(void) {
}

void staffEmpty4(void) {
}

void staffEmpty5(void) {
}
