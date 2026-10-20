#include "object.h"
#include "anim.h"
#include "audio.h"
#include "file.h"
#include "mathutil.h"
#include "plmain.h"
#include "sndloc.h"
#include "tmanager.h"

/* Sounds in the world. A sound's volume falls from full at a near distance to nothing at a far
 * one from the camera's eye. Its pan is the sine of its angle off the view direction, scaled by
 * the sine of the field of view (64 either side of 0x40).
 * - "actpan" is a task that pans a sound after its parent's position (message 4).
 * - "suion" is a task for a sound at a fixed point, which deletes itself when the sound ends.
 * - The rest play one-shot sounds.
 * The "score data" is sound events keyed to an animation. A track's key of 0x800 or more
 * between the last frame and this one fires its sound. */

typedef struct ActPan {
    /* 0x00 */ u16 id;
    /* 0x02 */ u16 flags; /* 1 playing */
    /* 0x04 */ CamView *view;
    /* 0x08 */ f32 near;
    /* 0x0C */ f32 far;
    /* 0x10 */ f32 scale; /* divides the pan */
} ActPan; /* an "actpan" task's work */

typedef struct Suion {
    /* 0x00 */ u16 id;
    /* 0x02 */ u16 stopId; /* id | 0x8000 */
    /* 0x04 */ s16 timer;
    /* 0x08 */ CamView *view;
    /* 0x0C */ f32 near;
    /* 0x10 */ f32 far;
    /* 0x14 */ f32 scale;
    /* 0x18 */ f32 pos[3];
} Suion; /* a "suion" task's work */

extern G88338 gGameWork; /* its view is the game's camera */
extern u8 gSndStopAllFirst; /* send sound 1 before the next sound */


/* Plays a sound, after sound 1 if gSndStopAllFirst asks for it. */
void soundPlayAfterOne(u16 id) {
    if (gSndStopAllFirst != 0) {
        sndPlay(1);
        gSndStopAllFirst = 0;
    }
    sndPlay(id);
}

/* The pan of a point (x, z) for a view. */
s16 soundPanGet(CamView *view, f32 x, f32 z, f32 fov) {
    s32 pad[2];
    f32 dx;
    f32 dz;
    f32 vz;
    f32 vx;
    f32 s;

    dx = x - view->eye[0];
    dz = z - view->eye[2];
    vz = view->eye[2] - view->at[2];
    vx = view->at[0] - view->eye[0];
    s = (vz * dx + vx * dz) / sqrtf((vz * vz + vx * vx) * (dx * dx + dz * dz));
    return (s16) (s / mathSin(fov) * 64.0f) + 0x40;
}

/* 1 if the point is within 0.8 of the field of view. */
s32 soundInFovCheck(CamView *view, f32 x, f32 y, f32 z) {
    Vec3f d;
    Vec3f v;
    f32 c;

    d.x = x - view->eye[0];
    d.y = y - view->eye[1];
    d.z = z - view->eye[2];
    v.x = view->at[0] - view->eye[0];
    v.y = view->at[1] - view->eye[1];
    v.z = view->at[2] - view->eye[2];
    c = (v.x * d.x + v.y * d.y + v.z * d.z) /
        sqrtf((v.x * v.x + v.y * v.y + v.z * v.z) * (d.x * d.x + d.y * d.y + d.z * d.z));
    if (c < mathCos(view->fovy * 0.8f)) {
        return 0;
    }
    return 1;
}

/* The "actpan" task's handler. */
s32 actpan_proc(Obj *obj, s16 msg, void *arg) {
    ActPan *w = (ActPan *) ((u32) obj + sizeof(Obj));
    s32 pad[2];
    s16 vol;
    s16 pan;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 t;
    f32 vz;
    f32 vx;
    Vec3f pos;
    s32 pad2;

    switch (msg) {
        case 1:
            if (w->flags & 1) {
                sndPlay(w->id | 0x8000);
            }
            break;
        case 2:
            obj->name = "actpan";
            w = (ActPan *) ((u32) obj + sizeof(Obj));
            if (arg != NULL) {
                *w = *(ActPan *) arg;
            }
            sndPlay(w->id);
            w->flags |= 1;
            /* fallthrough */
        case 0:
            pos.z = 0.0f;
            pos.y = 0.0f;
            pos.x = 0.0f;
            w = (ActPan *) ((u32) obj + sizeof(Obj));
            tskSendMsg(obj->parent, 4, &pos);
            dx = pos.x - w->view->eye[0];
            dy = pos.y - w->view->eye[1];
            dz = pos.z - w->view->eye[2];
            t = (sqrtf(dx * dx + dy * dy + dz * dz) - w->near) / (w->far - w->near);
            if (t > 1.0) {
                t = 1.0f;
            }
            vz = w->view->eye[2] - w->view->at[2];
            vx = w->view->at[0] - w->view->eye[0];
            pan = (vz * dx + vx * dz) / sqrtf((vz * vz + vx * vx) * (dx * dx + dz * dz)) /
                  w->scale * 64.0f;
            vol = (s16) ((1.0f - t) * 384.0f) - 0x100;
            sndSetParam(w->id, 1, vol);
            sndSetParam(w->id, 2, pan);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Marks an "actpan" task's sound as playing, or not. */
void soundActpanFlagSet(Obj *obj, s32 on) {
    ActPan *w = (ActPan *) ((u32) obj + sizeof(Obj));

    if (on == 1) {
        w->flags |= 1;
        return;
    }
    w->flags &= ~1;
}

Obj *soundActpanCreateCam(Obj *parent, u16 id) {
    if (gGameWork.view == NULL) {
        return NULL;
    }
    return soundActpanCreate(parent, id, gGameWork.view, gGameWork.view->zNear,
                                  gGameWork.view->zFar, gGameWork.view->fovy);
}

/* An "actpan" task under parent. A sound id with bit 15 clear is not started while it plays. */
Obj *soundActpanCreate(Obj *parent, u16 id, CamView *view, f32 near, f32 far, f32 fov) {
    ActPan a;

    if (id == 0) {
        return NULL;
    }
    if (!(id & 0x8000)) {
        if ((*sndGetVolume)(id, 0) != 0) {
            return NULL;
        }
    }
    a.id = id & ~0x8000;
    a.far = far;
    a.view = view;
    a.near = near;
    if (far < 0.0f) {
        a.far = 0.0f;
    }
    a.scale = 1.0f;
    return tskCreate(parent, actpan_proc, 0, &a);
}

/* The "suion" task's handler: a sound at a point. It deletes itself once the sound has ended
 * (after 9 frames) or when it starts too far away.
 * Matched 2026-10-03 (56 lines at the start). Case 0's colouring is the game's only
 * when vz and vx rank below dx, dz and the two eye loads: then dx and dz (used in both cases)
 * take f2 and f12, the eye[0] and eye[2] loads f14 and f16, vz f18, and vx, the sixth value for
 * five registers, none (stored to its home 0x44 and reloaded, as the game does). uopt ranks
 * them so when case 0's block ends between their definitions and their uses in the call. Its
 * limit of about 20 reads per block (IDO_NOTES 285) ends it there with no code once case 0
 * also computes dy, as case 2 does (dead here: the pan is flat). vz and vx then span two
 * blocks: priority 4 to 2 in uopt's own trace. Without
 * dy one block holds them all and vz and vx outrank the rest (114 lines). The camera is the
 * named local view, read once per case; it also gives the frame its word under vol, which puts
 * the far/vol temp at the game's 0x20 (without that word the temp sits at 0x24: 12 lines).
 * 2026-10-04 (fakes round 2): this form replaced a FAKE dead test "s = sqrtf(w->near); if (s
 * == 0.0f) {}" in that place (IDO_NOTES 253: a block boundary with no code) and its local s.
 * Measured on the way: view without case 0's dy 33; dy with s left unused (no view) MATCH, but
 * s is then a pad; s as the pan's denominator (SoundLocation2's form) or as its sine
 * (soundPanGet's form) is propagated away and takes no word (12, temp at 0x24); case 2's
 * distance inline as actpan_proc writes it (no dist) 12. Earlier: case 2's distance named dist
 * holds its word; a named pan for case 0's argument (s16, s32 or f32) takes none; vx as "f32
 * vx[1]" (the two sums' operands swapped) 58; a do-while boundary inside vz's live range 48;
 * Vec3f d, v 114; vz and vx inline 84; t reused for vz 70; a dead print after a denominator s
 * spills s (46). */
s32 suion_proc(Obj *obj, s16 msg, void *arg) {
    Suion *w = (Suion *) ((u32) obj + sizeof(Obj));
    f32 dx;
    f32 dy;
    f32 dz;
    f32 vx;
    f32 t;
    f32 vz;
    s16 vol;
    CamView *view;
    f32 dist;

    switch (msg) {
        case 1:
            sndPlay(w->stopId);
            break;
        case 2:
            obj->name = "suion";
            if (arg != NULL) {
                *w = *(Suion *) arg;
            }
            if (w->far != 0.0f) {
                view = w->view;
                dx = w->pos[0] - view->eye[0];
                dy = w->pos[1] - view->eye[1];
                dz = w->pos[2] - view->eye[2];
                dist = sqrtf(dx * dx + dy * dy + dz * dz);
                t = (dist - w->near) / (w->far - w->near);
                if (t > 1.0) {
                    t = 1.0f;
                }
                vol = (s16) ((1.0f - t) * 512.0f) - 0x100;
                if (vol < -0x200) {
                    return -1;
                }
                (*sndPlay)(w->id);
                (*sndSetParam)(w->id, 1, vol);
            } else {
                (*sndPlay)(w->id);
            }
            w->timer = 0;
            break;
        case 0:
            if (w->timer >= 9 && (*sndGetVolume)(w->id, 0) == 0) {
                return -1;
            }
            w->timer++;
            view = w->view;
            dx = w->pos[0] - view->eye[0];
            dy = w->pos[1] - view->eye[1];
            dz = w->pos[2] - view->eye[2];
            vz = view->eye[2] - view->at[2];
            vx = view->at[0] - view->eye[0];
            (*sndSetParam)(w->id, 2,
                                  (vz * dx + vx * dz) /
                                      sqrtf((vz * vz + vx * vx) * (dx * dx + dz * dz)) / w->scale *
                                      64.0f);
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* Changes an "actpan" task's sound. An old-style definition: the u16 comes in as an int, is
 * stored to its home and used from the register (sw a1,4(sp)). */
void soundSuionStopSet(obj, id)
Obj *obj;
u16 id;
{
    ((Suion *) ((u32) obj + sizeof(Obj)))->stopId = id;
}

Obj *soundSuionCreateCam(Obj *parent, u16 id, f32 x, f32 y, f32 z) {
    return soundSuionCreate(parent, id, gGameWork.view, x, y, z, 0.0f, 300.0f);
}

/* A "suion" task under parent. */
Obj *soundSuionCreate(Obj *parent, u16 id, CamView *view, f32 x, f32 y, f32 z, f32 near,
                            f32 far) {
    Suion a;

    if (id == 0) {
        return NULL;
    }
    if (view == NULL) {
        return NULL;
    }
    if (!(id & 0x8000)) {
        if ((*sndGetVolume)(id, 0) != 0) {
            return NULL;
        }
    }
    id &= ~0x8000;
    a.id = id;
    a.stopId = id | 0x8000;
    a.view = view;
    a.far = far;
    a.pos[0] = x;
    a.pos[1] = y;
    a.pos[2] = z;
    a.near = near;
    if (far < 0.0f) {
        a.far = 0.0f;
    }
    a.scale = 1.0f;
    return (*tskCreate)(parent, suion_proc, parent->file, &a);
}

/* A one-shot sound at (x, y, z) for the game's camera. */
void soundLocationCam(u16 id, f32 x, f32 y, f32 z) {
    SoundLocation2(id, gGameWork.view, x, y, z, gGameWork.view->zNear,
                           gGameWork.view->zFar, gGameWork.view->fovy);
}

/* A one-shot sound at (x, y, z), panned, and quieter with distance unless far is 0.
 * Matched 2026-10-02 (193 lines at the start): the slots in this declaration order (t, vol,
 * pan, dist, s, d, v: the game's 0x58 frame; dist is the distance, register-only as in
 * SoundIncam2, in the word a FAKE unused float held), the dead divide-by-zero print after s
 * (its string is this function's), "(1 - t)" with an int 1 (the permuter's), and near itself
 * taken to its absolute value as the first statement (the game: "abs.s f0,f14; mov.s
 * f14,f0" among the first loads; a separate n, or the param reassigned after d.x, gave 51). */
void SoundLocation2(u16 id, CamView *view, f32 x, f32 y, f32 z, f32 near, f32 far,
                            f32 fov) {
    f32 t;
    s16 vol;
    s16 pan;
    f32 dist;
    f32 s;
    Vec3f d;
    Vec3f v;

    near = fabsf(near);
    d.x = x - view->eye[0];
    d.y = y - view->eye[1];
    d.z = z - view->eye[2];
    if (far == 0.0f) {
        vol = 0;
    } else {
        dist = sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);
        t = (dist - near) / (far - near);
        if (t > 1.0) {
            t = 1.0f;
        } else if (t < 0.0f) {
            t = 0.0f;
        }
        vol = (1 - t) * 256.0f;
        if (vol <= 0) {
            return;
        }
    }
    v.x = view->eye[2] - view->at[2];
    v.z = view->at[0] - view->eye[0];
    s = sqrtf((v.x * v.x + v.z * v.z) * (d.x * d.x + d.z * d.z));
    if (s == 0.0f) {
        PRINTF("SoundLocation: 0除算しました \n");
    }
    pan = (s16) ((v.x * d.x + v.z * d.z) / s / mathSin(fov) * 64.0f) + 0x40;
    if (pan <= 0) {
        pan = 1;
    } else if (pan >= 0x80) {
        pan = 0x7F;
    }
    sndRequest(id, pan, vol);
}

s32 soundIncamCam(u16 id, f32 x, f32 y, f32 z) {
    return SoundIncam2(id, gGameWork.view, x, y, z, gGameWork.view->zNear,
                       gGameWork.view->zFar);
}

/* A one-shot sound at (x, y, z), played only between near and far and within 0.8 of the field
 * of view; 1 if played. */
s32 SoundIncam2(u16 id, CamView *view, f32 x, f32 y, f32 z, f32 near, f32 far) {
    s16 pan;
    f32 dist;
    f32 cs;
    f32 s;
    f32 c;
    Vec3f d;
    Vec3f v;

    d.x = x - view->eye[0];
    d.y = y - view->eye[1];
    d.z = z - view->eye[2];
    dist = sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);
    if (dist < near || far < dist) {
        return 0;
    }
    s = mathSinCos((s16) (fabsf(view->fovy) * 0.8f / PI * 512.0f), &c);
    v.x = view->at[0] - view->eye[0];
    v.y = view->at[1] - view->eye[1];
    v.z = view->at[2] - view->eye[2];
    cs = (v.x * d.x + v.y * d.y + v.z * d.z) /
         sqrtf((v.x * v.x + v.y * v.y + v.z * v.z) * (d.x * d.x + d.y * d.y + d.z * d.z));
    if (cs < c) {
        return 0;
    }
    v.x = view->eye[2] - view->at[2];
    v.z = view->at[0] - view->eye[0];
    pan = (s16) ((v.x * d.x + v.z * d.z) / sqrtf((v.x * v.x + v.z * v.z) * (d.x * d.x + d.z * d.z)) /
                 s * 64.0f) +
          0x40;
    if (pan <= 0) {
        pan = 1;
    } else if (pan >= 0x80) {
        pan = 0x7F;
    }
    sndPlayPan(id, pan);
    return 1;
}

/* Plays the score data's sounds that fire at this frame: up to four events, positioned at pos
 * if it is given. Returns a bit per event that fired, or -1. The test is a bitwise | of the two
 * conditions (sltiu, sltiu, or), which also loads the ids at entry into saved registers. */
s32 soundScorePlay(u32 addr, u16 file, f32 frame, f32 step, s32 loop, u16 id0, u16 id1,
                           u16 id2, u16 id3, PosFrame *pos) {
    s32 pad;
    s32 bits;
    ScoreEvent ev[4];
    s32 i;

    if ((addr == 0) | ((id0 | id1 | id2 | id3) == 0)) {
        return -1;
    }
    bits = 0;
    for (i = 0; i < 4; i++) {
        ev[i].hit = 0;
    }
    ev[0].id = id0;
    ev[1].id = id1;
    ev[2].id = id2;
    ev[3].id = id3;
    soundScoreRead(addr, file, ev, frame, step, loop);
    for (i = 0; i < 4; i++) {
        if (ev[i].hit != 0) {
            if (pos != NULL) {
                SoundLocation2(ev[i].id, gGameWork.view, pos->pos[0], pos->pos[1],
                                       pos->pos[2], 0.0f, 0.0f, gGameWork.view->fovy);
            } else {
                sndPlay(ev[i].id);
            }
        }
        bits |= ev[i].hit << i;
    }
    return bits;
}

/* Which of the score data's four events fire at this frame, a bit each; -1 for none. */
s32 soundScoreCheck(s32 addr, u16 file, f32 frame, f32 step, s32 loop, AnimeFrame *unused) {
    s32 pad;
    s32 bits;
    ScoreEvent ev[4];
    s32 i;

    if (addr == 0) {
        return -1;
    }
    for (i = 0; i < 4; i++) {
        ev[i].hit = 0;
    }
    ev[0].id = 0;
    ev[1].id = 0;
    ev[2].id = 0;
    ev[3].id = 0;
    soundScoreRead(addr, file, ev, frame, step, loop);
    bits = 0;
    for (i = 0; i < 4; i++) {
        bits |= ev[i].hit << i;
    }
    return bits;
}

/* Fills ev from the score data at addr (in file): its header counts its tracks (at most 4),
 * then two lists. Each track is an animation track (anim.c) at frame. The parameter itself
 * walks the data (s0); the original address is a copy taken after the first test (base, the
 * first local: its slot 0x84 is where the game keeps it, stored in the next test's delay slot).
 * The dead print for too many tracks is the game's string after SoundLocation2's. */
void soundScoreRead(u32 addr, u16 file, ScoreEvent *ev, f32 frame, f32 step, s32 loop) {
    u32 base;
    u32 w;
    u16 *tracks;
    u32 segs[10]; /* segments 3-12 */
    s32 n;
    s32 i;

    if ((addr == 0) | (ev == NULL)) {
        return;
    }
    base = addr;
    if (file != 0) {
        addr = fileRamAddrAt(file, addr);
    }
    addr &= 0x8FFFFFFE;
    w = *(u32 *) addr;
    if (w == 0) {
        return;
    }
    n = (w >> 8) & 0x7FFF;
    if (n >= 5) {
        PRINTF("Error:Score data model Over\n");
        return;
    }
    addr += 4;
    fileRamAddrAt(file, *(u32 *) addr);
    addr += 4;
    tracks = (u16 *) fileRamAddrAt(file, *(u32 *) addr);
    segs[((base >> 24) & 0xF) - 3] = fileRamAddr(file);
    animSeek(base, frame, segs);
    for (i = 0; i < n; i++) {
        if (*tracks & 0x8000) {
            ev->hit = 0;
        } else {
            ev->hit = soundScoreKeyCheck(*tracks, frame, step, loop, w & 0xFF);
        }
        tracks++;
        ev++;
    }
}

/* The score data's key test. Matched 2026-10-03 (from 155 lines); the
 * steps, in IDO_NOTES terms: a ternary return (157); the locals in the game's slot order (d,
 * now, a, b); the loop "(s32) now >= k" with no end variable (the game converts now twice, from
 * f18 for the entry test and from its home, hoisted and spilled across the call, for the bottom
 * test); one float d for |frame - step|, frame - step and |last - d| (the game's frame 0x50);
 * a FAKE volatile on a (done's load of a is ugen's t8, not uopt's v0; no struct, array or
 * pointer form found does it); and last the loop as a "for" with k++ in its increment: written
 * "k++;" after the call in a while body, ugen emits the call's caller-save reload of v0 before
 * the add and as1 keeps that order, while the for's increment gives the game's "addiu s0;
 * lw v0,0x34" (the while form with k++ after the if, or the call and k++ on one line: not).
 * HISTORY (the score data): float registers, spills and loop shapes. 2026-10-02 (155 to
 * 153): the end is a ternary return (the game's "b next; li v1,1", IDO_NOTES 157), and the
 * locals in the game's slot order (now 0x48 above a 0x46 and b 0x44, one word above now, three
 * between b and end at 0x34). Left: the game truncates frame into now at entry and homes it
 * (swc1 0x48, reloaded for end); here uopt converts frame again near each use and never homes
 * now, and the frame is 0x58 for the game's 0x50 (two more temp words).
 * 2026-10-03, read off the game: now lives in f18 from entry to the end (now == last, the
 * "(s32) now" argument, now - last) and in its home, which only "end = now" reloads; frame
 * stays in f12 and step in f14 throughout (frame - step into f8 once, for prev and the
 * "prev < last" test). So frame is still live where IDO here re-truncates it: the game's now
 * cannot be a value uopt sees as "(s32) frame" at those points. Not yet tried: a now that is
 * not frame's truncation in uopt's eyes (assigned through a call-free but opaque path).
 * The copies sweep's find (a through a copy from the "now - last" test on, 144) is jumped over
 * by the three "goto done": the return would read an unset copy (rejected; the mode now skips
 * positions a goto and its label straddle). The greedy mode (153 to 131, mix distance 13 to 11):
 * a declared after the floats, and a FAKE "if (now) {}" before the first test.
 * 2026-10-03 (131 to 95, mix distance 11 to 3; screen: one trunc.w.s more in the game): the
 * game converts now twice for the loop, from its register for the entry test and from its home
 * (hoisted, spilled at 0x34 across the call) for the bottom test: the loop is "while ((s32) now
 * >= k)", no end variable. The FAKE "if (now) {}" then does nothing: removed. The locals in the
 * game's order from its stack map (d, now, a, b, then the register ones; a 0x46, b 0x44, now
 * 0x48). Left: every local and the spill 8 high (frame 0x58 for 0x50: one compiler-temp word
 * more under the locals), and the game reloads a at done ("lhu t8,0x46" in the delay slots).
 * The second ternary as if/else: no change (95). The permuter on this form (734 to 666): now
 * read through a copy for "now - last == 1.0f" (FAKE): 93, the spill at the game's 0x34 (the copy
 * is a named local, so a compiler temp goes); declared after k, as here (declared second: also 93,
 * but above now). Left: a, b and now 8 high: four register-only words under them where the game
 * has three, so one of last, prev, k is no variable of its own in the game. d as the copy: 95.
 * 2026-10-03 (from screen: the game has one lhu more and one slti less): the game's done
 * block loads a itself into a t-temp ("lhu t8,0x46" first, copied into each branch's slot); this
 * C keeps a's load in v0 across blocks (the loop=1 test's "lhu v0" reaches done, whose bnezl
 * then skips done's load and puts slti in its slot). Diagnostic, a out of that test ("b >=
 * 0x123"): the frame stays 0x58, so that cross-block value is not the extra word.
 * 2026-10-03, "tools/permuter.py drops" (each one-line statement removed in turn): only "last =
 * (s32) prev;" removed gives the game's frame 0x50 with a and b at the game's 0x44/0x46 (one
 * other, the loop's call, also does, by killing b). The code from entry through the else
 * branch is the game's register for register (prev f16, last f0, now f18, k s0; last is an f32:
 * "k = last" is a trunc.w.s of f0); only the reserved words differ: under b the game has three
 * (last, prev, k), this C five (last, prev, k, the FAKE cur, and one temp). So the temp that
 * goes with last's definition (and cur's word) are the two words; a form that defines last
 * without that temp is the lead.
 * 2026-10-03 (from the 93 form): the FAKE cur taken out: 95, spill at 0x38 (with cur
 * 0x34): the temps sit right BELOW the locals, top-down, so cur only pushed them one word down
 * and filled the rounding word (11 words round to 0x58 as 12 do). The game: one word above now,
 * spill at 0x34 = 0x50 - 4 x 5 locals - 2 words, so either 5 local words and the same 5 temps,
 * or 6 locals with the temp above the spill gone. d also holding "frames - 1.0f" (temp gone?):
 * no change (95, same stack). k as the parameter loop: the game's frame and every slot, but
 * "move s0,a3" at entry (118): so 5 local words, one fewer named local. prev merged into d (one
 * float for |frame - step|, frame - step, then |last - prev|): 69, the game's frame and slots.
 * Left: the done block loads a into v0 (uopt's register: the loop=1 compare then loads a first,
 * into v0, and its bnezl skips done's load) where the game's done loads it into ugen's next
 * temp, t8; every t-register after is one off. a and b as one array u16 key[2]: the same 69.
 * Diagnostic, the loop=1 compare on "b >= 0x123": done still loads into v0 (92), so it is not
 * the compare's load carried over: done's own load of a is a uopt temp here and not in the
 * game. a declared volatile (FAKE, &a cast back to u16 *): 10, mix distance 0, done's lhu t8
 * and every t-register as the game; 8 of the 10 are this C's own .rodata relocations, the
 * other two the loop's "lw v0,0x34" above "addiu s0,s0,1" (the game: addiu first). The loop
 * as "if (c) do {..} while (c);" (the bottom test on a later line): the same 10. "while (k <=
 * (s32) now)": the same 10. The joins and splits sweeps: nothing better (a split inside the
 * else before the loop: 54; after "if (b >= a) {": 32). So the C below differs from the game
 * only in that one pair's order; its volatile is FAKE: the game's form that keeps done's load
 * of a out of uopt's registers is not found yet (and may be what orders the pair).
 * 2026-10-03 (from the 10 form; the pair: game "lhu t6; lhu t7; addiu s0; lw v0,0x34",
 * this C "lw v0,0x34" before "addiu s0"): the call and "k++;" on one line: the same 10. The
 * loop as "for (; (s32) now >= k; k++)": MATCH.
 * 2026-10-04 (fakes round 2; uopt's own trace, k0, k1, k3 with
 * pri.sh): what the volatile stands for. Without it, uopt makes done's conversion of a to int
 * (the expression "ucvt a") a live range of its own, priority 2, coloured v0: computed at the
 * end of every block that jumps to done (PRE insertions 5 against 2 with the volatile) and
 * shared with the loop=1 test "b >= a". With that test off a ("b >= 0x123") the range stays,
 * computed in all four blocks before done (6 insertions; 90 lines). The game has no such
 * range: done converts its own load of a (ugen's t8), and the loop=1 test loads a again. So
 * in the original, a at done was not a value uopt tracks as a variable: a volatile local, or
 * memory uopt does not track (a plain local, an array element or a struct field at a constant
 * offset are all tracked: measured, 69). A cast view at done ("*(u16 *) &a >= 0x800") is
 * folded back to a: 67, as with no volatile. No natural form found. */
/* 1 if the track has a key of 0x800 or more between the last frame (frame - step) and this
 * one; looping animations wrap. */
s32 soundScoreKeyCheck(u16 track, f32 frame, f32 step, s32 loop, f32 frames) {
    f32 d;
    f32 now = (s32) frame;
    volatile u16 a = 0; /* FAKE: keeps the load of a at done a ugen temp (t8), as in the game */
    u16 b = 0;
    f32 last;
    s32 k;

    if (frame != step) {
        d = (step < frame) ? frame - step : -(frame - step);
        if (d < 1e-07 || frame < step) {
            animDecodeKeys(track, -1, 0, NULL, (u16 *) &a);
            goto done;
        }
    }
    d = frame - step;
    last = (s32) d;
    if (now == last) {
        return 0;
    }
    if (frames - 1.0f <= now) {
        if (loop) {
            animDecodeKeys(track, -1, frames - 1.0f, NULL, (u16 *) &a);
            animDecodeKeys(track, -1, 0, NULL, &b);
            if (b >= a) {
                return 0;
            }
            goto done;
        }
        animDecodeKeys(track, -1, frames - 1.0f, NULL, (u16 *) &a);
        goto done;
    }
    if (now - last == 1.0f) {
        if (step < frame) {
            animDecodeKeys(track, -1, now, NULL, (u16 *) &a);
        }
    } else {
        d = (d < last) ? last - d : -(last - d);
        if (d < 1e-05) {
            k = last;
        } else {
            k = (s32) last + 1;
        }
        for (; (s32) now >= k; k++) {
            animDecodeKeys(track, -1, k, NULL, &b);
            if (b >= a) {
                a = b;
            }
        }
    }
done:
    return (a >= 0x800) ? 1 : 0;
}

/* 1 if no player's (0-3) test in the stage overlay is set. */
s32 soundNoPlPoweredDown(void) {
    s32 i;

    for (i = 0; i < 4; i++) {
        if (plIsPoweredDown(i) != 0) {
            return 0;
        }
    }
    return 1;
}
