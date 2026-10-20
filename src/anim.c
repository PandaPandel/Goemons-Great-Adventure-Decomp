#include "object.h"
#include "anim.h"
#include "gfxbuf.h"
#include "mathutil.h"

/* Animations: each track of a model node (ModelNode's scale, rotation and position) is a
 * 16-bit value. Above 0xA000 it is a constant (less 0xB000). Otherwise it is the start of the
 * track's keys in the animation's data, one key a frame; with bit 15 set, only the first key is
 * used. Values between frames are interpolated.
 *
 * The keys are compressed, a byte stream of runs (animDecodeKeys). An op byte below 0x78 is a
 * run of (op & 0x1F) low bytes. An op byte from 0x78 up to 0xF0 is one low byte repeated
 * ((op - 0x78) & 0x1F) + 2 times. For both, the op's range also changes the high byte shared by
 * the values: a new one follows the op, or it goes up by 1, down by 1, or stays. An op byte of
 * 0xF0 or more is (op & 0xF) whole 16-bit values. A count of 0 runs to the end. */

typedef struct AnimHeader {
    /* 0x0 */ u32 frames; /* in the low byte */
    /* 0x4 */ u32 keys; /* a segmented address */
} AnimHeader;

extern f32 gAnimFrame; /* the frame being shown */
extern f32 gAnimFrameCount; /* the frame count */
extern u8 *gAnimKeys; /* the keys */


/* Seeks an animation at a frame (a fraction between two). */
void animSeek(u32 addr, f32 frame, u32 *segs) {
    AnimHeader *h = (AnimHeader *) gfxSegmentToRam(addr, segs);

    gAnimFrame = frame;
    gAnimFrameCount = h->frames & 0xFF;
    gAnimKeys = (u8 *) gfxSegmentToRam(h->keys, segs);
}

/* A track's value at the frame: a scale. The last frame runs back to the first. */
u16 animGetScale(u16 v) {
    u16 a;
    u16 b;
    f32 t;
    s32 key;

    v &= 0xBFFF;
    if (v > 0xA000) {
        return v - 0xB000;
    }
    if (v & 0x8000) {
        v &= 0x7FFF;
        t = 0.0f;
        key = 0;
    } else {
        key = gAnimFrame;
        t = gAnimFrame - key;
    }
    if (fabsf(t) < 0.0001) {
        animDecodeKeys(v, -1, key, NULL, &a);
        return a;
    }
    if (gAnimFrameCount - 1.0f <= key) {
        animDecodeKeys(v, 0, key, &b, &a);
    } else {
        animDecodeKeys(v, key, key + 1, &a, &b);
    }
    return a + (s32) ((b - a) * t);
}

/* The same for a signed track: a position. (Its test for a whole frame is ==, where the
 * others have <.) */
s16 animGetPos(u16 v) {
    s16 a;
    s16 b;
    f32 t;
    s32 key;

    v &= 0xBFFF;
    if (v > 0xA000) {
        return v - 0xB000;
    }
    if (v & 0x8000) {
        v &= 0x7FFF;
        t = 0.0f;
        key = 0;
    } else {
        key = gAnimFrame;
        t = gAnimFrame - key;
    }
    if (fabsf(t) == 0.0001) {
        animDecodeKeys(v, -1, key, NULL, (u16 *) &a);
        return a;
    }
    if (gAnimFrameCount - 1.0f <= key) {
        animDecodeKeys(v, 0, key, (u16 *) &b, (u16 *) &a);
    } else {
        animDecodeKeys(v, key, key + 1, (u16 *) &a, (u16 *) &b);
    }
    return a + (s16) ((b - a) * t);
}

/* A node's rotation at the frame, into rot. Each angle turns the short way; when x would turn
 * between 0x199 and 0x268, the angles are flipped first (mathRotAltEuler). */
void animGetRot(ModelNode *node, u16 *rot) {
    s32 i;
    s32 key;
    f32 t;
    s32 key0;
    f32 frac;
    u16 next[3];
    u16 v[3];
    s16 d;
    s32 pad;

    v[0] = node->rot[0];
    v[1] = node->rot[1];
    v[2] = node->rot[2];
    key0 = gAnimFrame;
    frac = gAnimFrame - key0;
    for (i = 0; i < 3; i++) {
        key = key0;
        t = frac;
        v[i] &= 0xBFFF;
        if (v[i] > 0xA000) {
            rot[i] = next[i] = v[i] - 0xB000;
        } else {
            if (v[i] & 0x8000) {
                v[i] &= 0x7FFF;
                key = 0; t = 0.0f;
            }
            if (fabsf(t) < 0.0001) {
                animDecodeKeys(v[i], -1, key, NULL, &rot[i]);
                next[i] = rot[i];
            } else if (gAnimFrameCount - 1.0f <= key) {
                animDecodeKeys(v[i], 0, key, &next[i], &rot[i]);
            } else {
                animDecodeKeys(v[i], key, key + 1, &rot[i], &next[i]);
            }
        }
    }
    if (frac != 0.0f) {
        for (i = 0; i < 3; i++) {
            if (rot[i] != next[i]) {
                d = (next[i] - rot[i]) & 0x3FF;
                if (i == 0 && d >= 0x199 && d < 0x268) {
                    mathRotAltEuler(&rot[0], &rot[1], &rot[2]);
                    d = (next[0] - rot[0]) & 0x3FF;
                }
                while (d > 0x200) {
                    d -= 0x400;
                }
                rot[i] = (u16) (rot[i] + (s32) (d * frac)) & 0x3FF;
            }
        }
    }
}

/* Decodes a track from pos: the value of key k0 into *out0 (if k0 >= 0) and of key k1 into
 * *out1. After a run of whole values, the shared high byte is the last value's. */
void animDecodeKeys(u16 pos, s32 k0, s32 k1, u16 *out0, u16 *out1) {
    u8 op;
    u8 hi;
    u16 n;
    s32 len;

    hi = 0;
    while (TRUE) {
        op = gAnimKeys[pos];
        pos++;
        if (op < 0x78) {
            if (op >= 0x60) {
                hi = gAnimKeys[pos++];
            } else if (op >= 0x40) {
                hi++;
            } else if (op >= 0x20) {
                hi--;
            }
            n = op & 0x1F;
            if (k0 >= 0) {
                if (!n || k0 < n) {
                    *out0 = gAnimKeys[pos + k0] + (hi << 8);
                    k0 = -1;
                } else {
                    k0 -= n;
                }
            }
            if (!n || k1 < n) {
                *out1 = gAnimKeys[pos + k1] + (hi << 8);
                return;
            }
            k1 -= n;
            pos += n;
        } else if (op < 0xF0) {
            len = (op - 0x78) & 0x1F;
            if (op >= 0xD8) {
                hi = gAnimKeys[pos++];
            } else if (op >= 0xB8) {
                hi++;
            } else if (op >= 0x98) {
                hi--;
            }
            n = len;
            if (n != 0) {
                n += 2;
            }
            if (k0 >= 0) {
                if (!n || k0 < n) {
                    *out0 = gAnimKeys[pos] + (hi << 8);
                    k0 = -1;
                } else {
                    k0 -= n;
                }
            }
            if (!n || k1 < n) {
                *out1 = gAnimKeys[pos] + (hi << 8);
                return;
            }
            k1 -= n;
            pos++;
        } else {
            n = op & 0xF;
            if (k0 >= 0) {
                if (!n || k0 < n) {
                    *out0 = (gAnimKeys[pos + k0 * 2] << 8) + gAnimKeys[pos + k0 * 2 + 1];
                    k0 = -1;
                } else {
                    k0 -= n;
                }
            }
            if (!n || k1 < n) {
                *out1 = (gAnimKeys[pos + k1 * 2] << 8) + gAnimKeys[pos + k1 * 2 + 1];
                return;
            }
            k1 -= n;
            hi = gAnimKeys[pos + n * 2 - 2];
            pos += n * 2;
        }
    }
}
