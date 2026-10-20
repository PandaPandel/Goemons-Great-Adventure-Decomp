#ifndef CAMLIGHT_H
#define CAMLIGHT_H

#include "object.h"

void camlightResetCamera(void);
Gfx *camlightCameraDraw(Gfx *gfx, CamView *cam);
void camlightStartLights(void);
void camlightForceReload(void);
Gfx *camlightLightDraw(Gfx *gfx, Frame *frame);
Gfx *camlightLoadLights(Gfx *gfx);
Gfx *camlightAmbiDraw(Gfx *gfx, AmbFrame *f);
void camlightMakeCameraKind(s32 count);
void camlightMakeLightKind(s32 count);
void camlightMakeAmbiKind(s32 count);

#endif
