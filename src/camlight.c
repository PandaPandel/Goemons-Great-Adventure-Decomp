#include "object.h"
#include "camlight.h"
#include "fade.h"
#include "gfxbuf.h"
#include "gfxmgr.h"

/* The frame kinds "camera", "light" and "ambi". A camera frame sets the viewport, the
 * projection (perspective or ortho) and the view, with guLookAtHilite. Light frames fill a
 * Lights7 in the display-list buffer, and camlightLoadLights loads it when it has grown. */

extern CamView *gCamDrawView; /* the camera being drawn */
extern f32 gCamDrawDirX; /* its view direction, at - eye */
extern f32 gCamDrawDirY;
extern f32 gCamDrawDirZ;
extern LookAt *gCamDrawLookAt; /* its look-at */
extern Lights7 *gLightsCur; /* this frame's lights */
extern s16 gLightsNumSet; /* how many lights are set */
extern s16 gLightsNumLoaded; /* how many are loaded */
extern f32 gWorldOriginX; /* the world's origin, taken off the eye and target */
extern f32 gWorldOriginY;
extern f32 gWorldOriginZ;
extern s16 gFrmKindCamera; /* the "camera" kind */
extern s16 gFrmKindLight; /* the "light" kind */
extern s16 gFrmKindAmbi; /* the "ambi" kind */
extern Gfx *gGfxDlHead; /* the display-list buffer */
/* camlight.c's .data (0x80108990-0x80108AC4), in ROM order. */
Vp gCamDefaultVp = { /* the default viewport */
    { { 640, 480, 511, 0 }, { 640, 480, 511, 0 } },
};
Lights7 gLightsInit = { 0 }; /* the lights at the start of a frame */
CamView gCamFrameInit = { /* a new camera frame */
    { 0 }, { 0.0f, 0.0f, 400.0f }, { 0.0f, 0.0f, 0.0f }, 0.0f, 0.34906587f, 20.0f, 5000.0f,
    { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 32, 32, 0x00, &gCamDefaultVp, 3,
    { 0, 0, 0, 0 }, 0.0f, 0.0f, 320.0f, 240.0f,
};
LightFrame gLightFrameInit = { /* a new light frame */
    { 0 }, 0x00, { 100, 100, 100 }, { 0xFF, 0xFF, 0xFF },
};
AmbFrame gAmbiFrameInit = { { 0 }, { 0x60, 0x60, 0x60 } }; /* a new ambient frame */


/* No camera yet this frame; the view direction is (1, 1, 1). */
void camlightResetCamera(void) {
    gCamDrawView = NULL;
    gCamDrawDirX = gCamDrawDirY = gCamDrawDirZ = 1.0f;
}

/* A camera frame's draw: the viewport, the projection and the view. */
Gfx *camlightCameraDraw(Gfx *gfx, CamView *cam) {
    Vp *vp;
    u16 perspNorm;
    Mtx *proj;
    Mtx *view;
    Mtx *ident;
    s32 pad;
    LookAt *lookAt;
    Hilite *hilite;

    proj = gfxAllocMtx(1);
    view = gfxAllocMtx(1);
    ident = gfxAllocMtx(1);
    lookAt = gfxAllocLookAt(1);
    hilite = (Hilite *) gfxAllocVp(1);
    gCamDrawView = cam;
    gCamDrawDirX = cam->at[0] - cam->eye[0];
    gCamDrawDirY = cam->at[1] - cam->eye[1];
    gCamDrawDirZ = cam->at[2] - cam->eye[2];
    gCamDrawLookAt = lookAt;
    if (cam->vp != NULL) {
        vp = (Vp *) gfxAllocVtx(1);
        *vp = *cam->vp;
        gSPViewport(gfx++, vp);
    } else {
        gSPViewport(gfx++, &gCamDefaultVp);
    }
    if (!(cam->ortho & 1)) {
        guPerspective(proj, &perspNorm, cam->fovy / (2 * PI) * 360.0f, 4.0f / 3.0f,
                           cam->zNear, cam->zFar, 1.0f);
        gSPPerspNormalize(gfx++, perspNorm);
    } else {
        guOrtho(proj, cam->left, cam->right, cam->bottom, cam->top, cam->zNear,
                           cam->zFar, 1.0f);
    }
    gSPMatrix(gfx++, OS_K0_TO_PHYSICAL(proj), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    if (!(cam->ortho & 1)) {
        guLookAtHilite(view, lookAt, hilite, cam->eye[0] - gWorldOriginX,
                           cam->eye[1] - gWorldOriginY, cam->eye[2] - gWorldOriginZ,
                           cam->at[0] - gWorldOriginX, cam->at[1] - gWorldOriginY,
                           cam->at[2] - gWorldOriginZ, sinf(cam->roll), cosf(cam->roll), 0.0f,
                           cam->hilite1[0], cam->hilite1[1], cam->hilite1[2], cam->hilite2[0],
                           cam->hilite2[1], cam->hilite2[2], cam->hiliteW, cam->hiliteH);
        gSPMatrix(gfx++, OS_K0_TO_PHYSICAL(view), G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
    }
    guMtxIdent(ident);
    gSPMatrix(gfx++, K0_TO_PHYS(ident), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    if (cam->clip != 0) {
        gMoveWd(gfx++, G_MW_CLIP, G_MWO_CLIP_RNX, cam->clip);
        gMoveWd(gfx++, G_MW_CLIP, G_MWO_CLIP_RNY, cam->clip);
        gMoveWd(gfx++, G_MW_CLIP, G_MWO_CLIP_RPX, 0x10000 - cam->clip);
        gMoveWd(gfx++, G_MW_CLIP, G_MWO_CLIP_RPY, 0x10000 - cam->clip);
    }
    if (cam->scissor.ulx | cam->scissor.uly | cam->scissor.lrx |
        cam->scissor.lry) {
        gDPSetScissor(gfx++, G_SC_NON_INTERLACE, cam->scissor.ulx, cam->scissor.uly,
                      cam->scissor.lrx, cam->scissor.lry);
    }
    return gfx;
}

/* Starts a frame's lights from gLightsInit, none set. */
void camlightStartLights(void) {
    gLightsNumSet = gLightsNumLoaded = 0;
    gLightsCur = gfxAllocLights7(1);
    *gLightsCur = gLightsInit;
}

/* Makes camlightLoadLights load the lights again. */
void camlightForceReload(void) {
    gLightsNumLoaded = 0;
}

/* A light frame's draw: its colour and direction into light number f->lightNum (1-7), or the next
 * one for 0. The Frame parameter is cast into a LightFrame local, as in gfxmgr.c's frmDlistDraw:
 * read through the local, the count's store stays out of &gLightsNumSet's register (uopt gives it
 * the test and the else arm only, and the index takes v1), as in the game. */
Gfx *camlightLightDraw(Gfx *gfx, Frame *frame) {
    LightFrame *f = (LightFrame *) frame;
    s32 i;

    fadeLightNormalize(f);
    if (f->lightNum != 0) {
        i = f->lightNum - 1;
        gLightsCur->l[i].l.col[0] = f->color[0];
        gLightsCur->l[i].l.col[1] = f->color[1];
        gLightsCur->l[i].l.col[2] = f->color[2];
        gLightsCur->l[i].l.pad1 = 0;
        gLightsCur->l[i].l.colc[0] = f->color[0];
        gLightsCur->l[i].l.colc[1] = f->color[1];
        gLightsCur->l[i].l.colc[2] = f->color[2];
        gLightsCur->l[i].l.pad2 = 0;
        gLightsCur->l[i].l.dir[0] = f->dir[0];
        gLightsCur->l[i].l.dir[1] = f->dir[1];
        gLightsCur->l[i].l.dir[2] = f->dir[2];
        gLightsCur->l[i].l.pad3 = 0;
        if (gLightsNumSet < f->lightNum) {
            gLightsNumSet = f->lightNum;
        }
        gLightsNumLoaded = 0;
    } else {
        gLightsCur->l[gLightsNumSet].l.col[0] = f->color[0];
        gLightsCur->l[gLightsNumSet].l.col[1] = f->color[1];
        gLightsCur->l[gLightsNumSet].l.col[2] = f->color[2];
        gLightsCur->l[gLightsNumSet].l.pad1 = 0;
        gLightsCur->l[gLightsNumSet].l.colc[0] = f->color[0];
        gLightsCur->l[gLightsNumSet].l.colc[1] = f->color[1];
        gLightsCur->l[gLightsNumSet].l.colc[2] = f->color[2];
        gLightsCur->l[gLightsNumSet].l.pad2 = 0;
        gLightsCur->l[gLightsNumSet].l.dir[0] = f->dir[0];
        gLightsCur->l[gLightsNumSet].l.dir[1] = f->dir[1];
        gLightsCur->l[gLightsNumSet].l.dir[2] = f->dir[2];
        gLightsCur->l[gLightsNumSet].l.pad3 = 0;
        gLightsNumSet++;
    }
    return gfx;
}

/* Loads the lights, copied into the display-list buffer, if more are set than loaded. */
Gfx *camlightLoadLights(Gfx *gfx) {
    s32 i;

    if (gLightsNumLoaded < gLightsNumSet) {
        gDPPipeSync(gfx++);
        gSPNumLights(gfx++, gLightsNumSet);
        for (i = 0; i < gLightsNumSet; i++) {
            *(Light *) gGfxDlHead = gLightsCur->l[i];
            gSPLight(gfx++, gGfxDlHead, i + 1);
            gGfxDlHead = (Gfx *) ((Light *) gGfxDlHead + 1);
        }
        *(Ambient *) gGfxDlHead = gLightsCur->a;
        gSPLight(gfx++, gGfxDlHead, gLightsNumSet + 1);
        gGfxDlHead = (Gfx *) ((Ambient *) gGfxDlHead + 1);
    }
    gLightsNumLoaded = gLightsNumSet;
    return gfx;
}

/* An ambient frame's draw: its colour. */
Gfx *camlightAmbiDraw(Gfx *gfx, AmbFrame *f) {
    gLightsCur->a.l.col[0] = gLightsCur->a.l.colc[0] = f->color[0];
    gLightsCur->a.l.col[1] = gLightsCur->a.l.colc[1] = f->color[1];
    gLightsCur->a.l.col[2] = gLightsCur->a.l.colc[2] = f->color[2];
    return gfx;
}

/* Makes the "camera" kind, of count frames. */
void camlightMakeCameraKind(s32 count) {
    gFrmKindCamera = frmNewKind("camera", (FrameDraw) camlightCameraDraw,
                                        sizeof(CamView), count, &gCamFrameInit);
}

/* The "light" kind. */
void camlightMakeLightKind(s32 count) {
    gFrmKindLight = frmNewKind("light", (FrameDraw) camlightLightDraw,
                                        sizeof(LightFrame), count, &gLightFrameInit);
}

/* The "ambi" kind. */
void camlightMakeAmbiKind(s32 count) {
    gFrmKindAmbi = frmNewKind("ambi", (FrameDraw) camlightAmbiDraw,
                                        sizeof(AmbFrame), count, &gAmbiFrameInit);
}
