#ifndef GFXMGR_H
#define GFXMGR_H

#include "object.h"
#include "game.h"

typedef Gfx *(*FrameDraw)(Gfx *gfx, Frame *f);

s32 frmGraphicManagerProc(Obj *obj, s16 msg, void *arg);
void frmGraphicManagerRestart(void);
Gfx *frmDrawTree(Gfx *gfx, Frame *f);
Gfx *frmDlistDraw(Gfx *gfx, Frame *frame);
void frmReset(Frame *f, s32 kind);
s32 frmNewKind(char *name, FrameDraw draw, s32 size, s32 count, void *init);
void frmDeleteKind(s32 kind);
Frame *frmAlloc(s32 kind);
Frame *frmCreateRoot(s32 kind);
Frame *frmCreate(Frame *parent, s16 kind);
Frame *frmCreateNext(Frame *sib, s16 kind);
Frame *frmCreatePrev(Frame *sib, s16 kind);
Frame *frmCreateTop(Frame *parent, s16 kind);
void frmFree(Frame *f);
void frmUnlink(Frame *f);
void frmLink(Frame *f, Frame *parent);
void frmLinkNext(Frame *f, Frame *sib);
void frmLinkPrev(Frame *f, Frame *sib);
void frmMove(Frame *f, Frame *parent);
s32 frmRevive(Frame *f);
Frame *frmDestroy(Frame *f);
void frmDestroyChildren(Frame *f);
s32 frmGetIndex(Frame *f);
Frame *frmGetLastChild(Frame *f);
s32 frmCountChildren(Frame *f);
s32 frmCountFree(s32 kind);
s32 frmIsValid(Frame *f);

#endif
