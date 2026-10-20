#include "acoll.h"
#include "aniatk.h"
#include "gfxmgr.h"
#include "mathutil.h"

/* Animated attack chains (the "aniAtk " frame kind, 40 links of 0x2C).
 * - A chain follows a model through an animation: one link per joint, each with an attack
 *   box (acoll.c) on it.
 * - The animation's header word holds the joint count (bits 8-22) and the frame count
 *   (bits 0-7). Its second word points at the tracks' byte stream, its third at the joints
 *   (8 bytes each: a scale track and x, y, z offset tracks).
 * - A track value is found by walking the stream's runs to the frame, and is interpolated
 *   with the next frame (wrapping at the last).
 *   - A run below 0x78 is bytes under a high byte that it sets (0x60+), raises (0x40+) or
 *     lowers (0x20+).
 *   - A run below 0xF0 repeats one value, n + 2 times.
 *   - 0xF0+ is raw 16-bit values.
 *   - A count of 0 holds the value from there on.
 * - Keys above 0xA000 are constants (the key - 0xB000), and bit 15 marks a track without
 *   frames. */

extern s16 gFrmKindCore; /* the "null" kind (roots) */
extern s16 gFrmKindAniAtk; /* "aniAtk " */
extern Frame *gAniAtkRoot; /* the chains' root */
extern f32 gAniAtkJointX; /* the joint being read: its offset scale x, y, z, and its scale */
extern f32 gAniAtkJointY;
extern f32 gAniAtkJointZ;
extern f32 gAniAtkJointScale;
extern f32 gAniAtkOutScale; /* the result: its scale, and its place */
extern f32 gAniAtkOutX;
extern f32 gAniAtkOutY;
extern f32 gAniAtkOutZ;
extern u8 *gAniAtkStream; /* the track stream */
extern f32 gAniAtkNumFrames; /* the frame count */
extern f32 gAniAtkTime; /* the frame */

void *fileRamAddrAt(s32 file, u32 addr); /* FAKE: file.h is not included here: fileRamAddrAt: the game's code calls it as this file declares it; file.h has `u32 fileRamAddrAt(u32 file, u32 offset)` */
ACollFrame *_atkLinkAdd(Frame *parent, Frame *model, f32 x, f32 y, f32 z, f32 sx,
                                   f32 sy, f32 sz, f32 rx, f32 ry, f32 rz, u16 radius, s16 ox,
                                   s16 oy, s16 oz); /* FAKE: in no header: src/aniatk.c includes acoll.h already and used here as this file declares it (The number of arguments doesn't agree with the number in the declaration.); acoll.h has `ACollFrame *_atkLinkAdd(Frame *parent, Frame *model, f32 x, f32 y, f32 z, f32 sx, f32 sy, f32 sz, f32 rx, f32 ry, f32 rz, u16 radius, s16 ox, s16 oy, s16 oz, s32 unused)` */

/* Makes the kind and the chains' root. */
s32 aniAtkInit(void) {
    gFrmKindAniAtk = frmNewKind("aniAtk ", NULL, sizeof(AniAtkFrame), 40, NULL);
    if (gFrmKindAniAtk == -1) {
        return -1;
    }
    gAniAtkRoot = frmCreateRoot(gFrmKindCore);
    return 0;
}

/* A link after the first takes its parent's frame. */
void aniAtkSyncLinkTime(AniAtkFrame *f, s32 link) {
    f32 time;

    if (link != 0) {
        time = ((AniAtkFrame *) f->frame.parent)->time;
        f->time = time;
    }
}

/* A chain for model's animation (in file, at base): its links and their attack boxes, the
 * first box under parent. Returns the first box, or NULL (and deletes what was made). */
ACollFrame *aniAtkCreate(Frame *parent, Frame *model, u8 *anim, s32 file, s32 base) {
    f32 x, y, z;
    f32 sx, sy, sz;
    f32 rx, ry, rz;
    AniAtkFrame *firstLink = NULL;
    ACollFrame *firstBox = NULL;
    AniAtkFrame *link = (AniAtkFrame *) gAniAtkRoot;
    ACollFrame *box = (ACollFrame *) parent;
    ACollFrame *first;
    s32 i = 0;

    while (TRUE) {
        link = (AniAtkFrame *) frmCreate((Frame *) link, gFrmKindAniAtk);
        if (i == 0) {
            firstLink = link;
        }
        if (link == NULL) {
            if (firstLink != NULL) {
                frmDestroy((Frame *) firstLink);
            }
            if (firstBox != NULL) {
                frmDestroy((Frame *) firstBox);
            }
            return NULL;
        }
        link->model = model;
        link->anim = (u32) anim;
        link->file = file;
        link->link = i;
        link->base = base;
        aniAtkLinkTransform(link, i, 0.0f, &x, &y, &z, &rx, &ry, &rz, &sx, &sy, &sz);
        box = _atkLinkAdd((Frame *) box, (Frame *) link, x, y, z, sx, sy, sz, rx, ry, rz,
                                     1, 0, 0, 0);
        if (i == 0) {
            firstBox = box;
        }
        if (box == NULL) {
            if (firstLink != NULL) {
                frmDestroy((Frame *) firstLink);
            }
            if (firstBox != NULL) {
                frmDestroy((Frame *) firstBox);
            }
            return NULL;
        }
        if (i == 0) {
            first = box;
        }
        if (i + 1 < aniAtkJointCount(link)) {
            i++;
            continue;
        }
        return first;
    }
}

/* Deletes a chain by its first box. */
s32 aniAtkDelete(ACollFrame *box) {
    if (box->model != NULL) {
        frmDestroy(box->model);
    }
    if (box != NULL) {
        frmDestroy((Frame *) box);
    }
    return 0;
}

/* A chain's box at its index. */
ACollFrame *aniAtkGetBox(ACollFrame *box) {
    s32 i;
    ACollFrame *f = box;

    for (i = 1; i <= box->index; i++) {
        f = (ACollFrame *) f->frame.child;
    }
    return f;
}

/* The joint count, and the frame count. */
s32 aniAtkJointCount(AniAtkFrame *f) {
    u32 *h;
    u32 w;

    if (f->file != 0) {
        h = fileRamAddrAt(f->file, f->anim);
    } else {
        h = (u32 *) f->anim;
    }
    w = *h;
    return (w >> 8) & 0x7FFF;
}

s32 aniAtkFrameCount(AniAtkFrame *f) {
    u32 *h;
    u32 w;

    if (f->file != 0) {
        h = fileRamAddrAt(f->file, f->anim);
    } else {
        h = (u32 *) f->anim;
    }
    w = *h;
    gAniAtkNumFrames = w & 0xFF;
    return gAniAtkNumFrames;
}

/* The frame count of a box's chain. */
s32 aniAtkBoxFrameCount(ACollFrame *box) {
    AniAtkFrame *f = (AniAtkFrame *) box->model;
    u32 *h;
    u32 w;
    f32 n;

    if (f->file != 0) {
        h = fileRamAddrAt(f->file, f->anim);
    } else {
        h = (u32 *) f->anim;
    }
    w = *h;
    n = w & 0xFF;
    return n;
}

/* The track stream, and the joints. */
u8 *aniAtkTrackStream(AniAtkFrame *f, u32 base) {
    u32 *h;

    if (f->file != 0) {
        h = fileRamAddrAt(f->file, f->anim);
    } else {
        h = (u32 *) f->anim;
    }
    h++;
    if (f->file != 0) {
        gAniAtkStream = fileRamAddrAt(f->file, *h);
    } else {
        gAniAtkStream = (u8 *) ((*h & 0xFFFFFF) + base);
    }
    return gAniAtkStream;
}

u16 *aniAtkJoints(AniAtkFrame *f, u32 base) {
    u32 *h;
    u16 *joints;

    if (f->file != 0) {
        h = fileRamAddrAt(f->file, f->anim);
    } else {
        h = (u32 *) f->anim;
    }
    h += 2;
    if (f->file != 0) {
        joints = fileRamAddrAt(f->file, *h);
    } else {
        joints = (u16 *) ((*h & 0xFFFFFF) + base);
    }
    return joints;
}

/* A link's place, rotation and scale at a frame: the model's, plus the joint's offset turned
 * with it, the joint's scale. */
void aniAtkLinkTransform(AniAtkFrame *f, u8 link, f32 time, f32 *x, f32 *y, f32 *z, f32 *rx,
                            f32 *ry, f32 *rz, f32 *sx, f32 *sy, f32 *sz) {
    s32 pad[2];
    u16 *joints;
    Frame *model;
    f32 mx, my, mz;
    f32 msx, msy, msz;
    f32 mrx, mry, mrz;

    aniAtkJointCount(f);
    joints = aniAtkJoints(f, f->base);
    model = f->model;
    gAniAtkStream = aniAtkTrackStream(f, f->base);
    gAniAtkNumFrames = aniAtkFrameCount(f);
    gAniAtkTime = time;
    joints = &joints[link * 4];
    aniAtkReadJoint(joints);
    _FrameToPara(model, &mx, &my, &mz, &mrx, &mry, &mrz, &msx, &msy, &msz);
    aniAtkPlaceJoint(mx, my, mz, msx, msy, msz, mrx, mry, mrz);
    *x = gAniAtkOutX;
    *y = gAniAtkOutY;
    *z = gAniAtkOutZ;
    *rx = mrx;
    *ry = mry;
    *rz = mrz;
    *sx = gAniAtkOutScale;
    *sy = gAniAtkOutScale;
    *sz = gAniAtkOutScale;
}

/* Reads a joint's four tracks at the frame. */
void aniAtkReadJoint(u16 *joint) {
    gAniAtkJointScale = (f32) aniAtkTrackValueU(joint[0]) * (1.0 / 256);
    gAniAtkJointX = (f32) aniAtkTrackValueS(joint[1]) * (1.0 / 32);
    gAniAtkJointY = (f32) aniAtkTrackValueS(joint[2]) * (1.0 / 32);
    gAniAtkJointZ = (f32) aniAtkTrackValueS(joint[3]) * (1.0 / 32);
}

/* The model's place plus the joint's offset, scaled and turned, and the scale. */
void aniAtkPlaceJoint(f32 x, f32 y, f32 z, f32 sx, f32 sy, f32 sz, f32 rx, f32 ry, f32 rz) {
    f32 px, py, pz;

    px = gAniAtkJointX * sx;
    py = gAniAtkJointY * sy;
    pz = gAniAtkJointZ * sz;
    mathVecRotate(rx, ry, rz, &px, &py, &pz);
    px += x;
    py += y;
    pz += z;
    gAniAtkOutScale = gAniAtkJointScale * sx;
    gAniAtkOutX = px;
    gAniAtkOutY = py;
    gAniAtkOutZ = pz;
}

/* A track's value at the frame (unsigned), interpolated with the next. */
u16 aniAtkTrackValueU(u16 key) {
    u16 prev;
    u16 next;
    s32 i;
    f32 frac;
    s32 pad[2];

    key &= ~0x4000;
    if (key >= 0xA001) {
        return key - 0xB000;
    }
    if (key & 0x8000) {
        key &= 0x7FFF;
        frac = 0.0f;
        i = 0;
    } else {
        i = gAniAtkTime;
        frac = gAniAtkTime - i;
    }
    if (frac == 0.0f) {
        return aniAtkWalkTrack(key, -1, NULL, i);
    }
    if (gAniAtkNumFrames - 1.0f <= i) {
        prev = aniAtkWalkTrack(key, 0, &next, i);
    } else {
        next = aniAtkWalkTrack(key, i, &prev, i + 1);
    }
    return prev + (u32) ((next - prev) * frac);
}

/* The same, signed. */
s16 aniAtkTrackValueS(u16 key) {
    s16 prev;
    s16 next;
    s32 i;
    f32 frac;
    s32 pad[2];

    key &= ~0x4000;
    if (key >= 0xA001) {
        return key - 0xB000;
    }
    if (key & 0x8000) {
        key &= 0x7FFF;
        frac = 0.0f;
        i = 0;
    } else {
        i = gAniAtkTime;
        frac = gAniAtkTime - i;
    }
    if (frac == 0.0f) {
        return aniAtkWalkTrack(key, -1, NULL, i);
    }
    if (gAniAtkNumFrames - 1.0f <= i) {
        prev = aniAtkWalkTrack(key, 0, (u16 *) &next, i);
    } else {
        next = aniAtkWalkTrack(key, i, (u16 *) &prev, i + 1);
    }
    return prev + (s16) ((next - prev) * frac);
}

/* Walks a track from pos: returns the value at index i2, and puts the one at i1 (if not -1)
 * in out1. */
u16 aniAtkWalkTrack(u16 pos, s32 i1, u16 *out1, s32 i2) {
    u8 op;
    u8 hi = 0;
    u16 n;

    while (TRUE) {
        op = gAniAtkStream[pos];
        pos++;
        if (op < 0x78) {
            if (op >= 0x60) {
                hi = gAniAtkStream[pos++];
            } else if (op >= 0x40) {
                hi++;
            } else if (op >= 0x20) {
                hi--;
            }
            if (i1 >= 0) {
                n = op & 0x1F;
                if (!n || i1 < n) {
                    *out1 = gAniAtkStream[pos + i1] + (hi << 8);
                    i1 = -1;
                } else {
                    i1 -= n;
                }
            }
            n = op & 0x1F;
            if (!n || i2 < n) {
                return gAniAtkStream[pos + i2] + (hi << 8);
            }
            i2 -= n;
            pos += n;
        } else if (op < 0xF0) {
            if (op >= 0xD8) {
                hi = gAniAtkStream[pos++];
            } else if (op >= 0xB8) {
                hi++;
            } else if (op >= 0x98) {
                hi--;
            }
            n = (op - 0x78) & 0x1F;
            if (n) {
                n += 2;
            }
            if (i1 >= 0) {
                if (!n || i1 < n) {
                    *out1 = gAniAtkStream[pos] + (hi << 8);
                    i1 = -1;
                } else {
                    i1 -= n;
                }
            }
            if (!n || i2 < n) {
                return gAniAtkStream[pos] + (hi << 8);
            }
            i2 -= n;
            pos++;
        } else {
            if (i1 >= 0) {
                n = op & 0xF;
                if (!n || i1 < n) {
                    *out1 = (gAniAtkStream[pos + i1 * 2] << 8) + gAniAtkStream[pos + i1 * 2 + 1];
                    i1 = -1;
                } else {
                    i1 -= n;
                }
            }
            n = op & 0xF;
            if (!n || i2 < n) {
                return (gAniAtkStream[pos + i2 * 2] << 8) + gAniAtkStream[pos + i2 * 2 + 1];
            }
            i2 -= n;
            hi = gAniAtkStream[pos + n * 2 - 2];
            pos += n * 2;
        }
    }
}
