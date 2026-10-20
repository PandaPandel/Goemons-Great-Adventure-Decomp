#ifndef FACE_H
#define FACE_H

#include "object.h"
#include "msgfrm.h"

s32 faceRun(Obj *obj, s16 msg, void *arg);
Obj *faceOpen(Obj *parent, s32 kind);
Obj *faceClose(Obj *face);
PicFrame *faceMakePic(Frame *parent, f32 x, f32 y, s32 kind);
void faceLoadFile(s32 kind);
void faceFreeFile(s32 kind);
void faceSetKind(Obj *face, s32 kind);

#endif
