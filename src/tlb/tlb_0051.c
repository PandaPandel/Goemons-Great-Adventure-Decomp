#include "tlb.h"

/* Water sounds: a master per sound ("wsmaster", "wsmaster2") plays it at the loudest level its
 * sources ("wsound", "wsound2") ask for this frame, panned as that source asks; a "soundspot"
 * plays its sound from where it stands. */

/* A "wsmaster2" task's work (task + 0x20): an enemy's (EneObj) up to 0x34. */
typedef struct WsMasterObj {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ Frame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u16 sound;
    /* 0x36 */ s16 quiet; /* the level it rests at (-255: silent) */
    /* 0x38 */ s16 level; /* the loudest asked for this frame */
    /* 0x3A */ s16 pan;   /* that source's */
} WsMasterObj;

/* A "wsound2" source's work (task + 0x20). */
typedef struct WsoundObj {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ Frame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ Obj *master; /* its sound's "wsmaster2" */
    /* 0x38 */ f32 near;    /* full volume within (100) */
    /* 0x3C */ f32 far;     /* silent from (500) */
    /* 0x40 */ s32 volume;  /* the level at full volume (0) */
    /* 0x44 */ f32 panScale; /* (1) */
} WsoundObj;

/* A "soundspot"'s work (task + 0x20). */
typedef struct SoundSpotObj {
    /* 0x00 */ u8 unused00; /* an enemy's (EneObj) */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u16 sound;
} SoundSpotObj;

/* A "wsmaster" task's work (task + 0x20). */
typedef struct WsMaster1Obj {
    /* 0x00 */ u8 unused00; /* an enemy's (EneObj) */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u16 sound;
    /* 0x36 */ s16 count;      /* levels sent this frame */
    /* 0x38 */ s16 levels[16];
} WsMaster1Obj;

s32 wsMaster2Proc(Obj *obj, s16 msg, void *arg);
s32 wsMasterProc(Obj *obj, s16 msg, void *arg);

/* "wsmaster2": message 10 answers whether arg is its sound (with the task), message 7 asks for
 * a level (the low half of arg) and pan (the high half). */
s32 wsMaster2Proc(Obj *obj, s16 msg, void *arg) {
    WsMasterObj *w = (WsMasterObj *) ((u32) obj + sizeof(Obj));
    s32 *data;
    s32 n;
    s32 pad[5];

    switch (msg) {
        case 2:
            obj->name = "wsmaster2";
            data = (s32 *) eneSetup(obj, arg);
            n = data[0];
            w->sound = data[1];
            if (n >= 6) {
                w->quiet = data[6];
            } else {
                w->quiet = -255;
            }
            w->level = w->quiet;
            w->pan = 0;
            break;
        case 1:
            sndPlay(w->sound | 0x8000);
            break;
        case 10:
            if ((u32) arg == w->sound) {
                return (s32) obj;
            }
            break;
        case 0:
            if (w->quiet >= w->level) {
                if (sndGetVolume(w->sound, 0) != 0) {
                    sndPlay(w->sound | 0x8000);
                }
            } else {
                if (sndGetVolume(w->sound, 0) == 0) {
                    sndPlay(w->sound);
                }
                sndSetParam(w->sound, 1, w->level);
                sndSetParam(w->sound, 2, w->pan);
            }
            w->level = w->quiet;
            w->pan = 0;
            break;
        case 7:
            if ((((s32) arg << 16) >> 16) >= w->level) {
                w->level = ((s32) arg << 16) >> 16;
                w->pan = (u32) arg >> 16;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Asks master (a "wsmaster2") for level and pan, if louder than what it has this frame. */
void wsMaster2Request(Obj *master, s16 level, s16 pan) {
    WsMasterObj *w = (WsMasterObj *) ((u32) master + sizeof(Obj));

    if (master->proc == wsMaster2Proc) {
        if (level >= w->level) {
            w->level = level;
            w->pan = pan;
        }
    }
}

/* "wsound2": a source of its master's sound (eneSetup's data[1], found by message 10 to the
 * enemy manager): each frame it asks the master for a level by the camera target's distance
 * (full within near, silent from far) and a pan by which side of the camera it is on. */
s32 wsSound2Proc(Obj *obj, s16 msg, void *arg) {
    WsoundObj *w = (WsoundObj *) ((u32) obj + sizeof(Obj));
    s32 *data;
    s32 n;
    CamView *v;
    f32 dy = 0.0f; /* dead initialisers; their order sets the float registers' */
    f32 ax = 0.0f;
    f32 dx = 0.0f;
    f32 dz = 0.0f;
    f32 r;
    s16 level;
    f32 len;
    f32 s;
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "wsound2";
            data = (s32 *) eneSetup(obj, arg);
            n = data[0];
            if (n >= 2) {
                w->near = data[2];
            } else {
                w->near = 100.0f;
            }
            if (n >= 3) {
                w->far = data[3];
            } else {
                w->far = 500.0f;
            }
            if (n >= 4) {
                w->volume = data[4];
            } else {
                w->volume = 0;
            }
            if (n >= 5) {
                w->panScale = data[5] / 100.0f;
            } else {
                w->panScale = 1.0f;
            }
            w->master = (Obj *) tskSendMsg(gEneManager, 10, (void *) data[1]);
            if (w->master == NULL) {
                return -1;
            }
            break;
        case 0:
            v = gGameWork.view;
            dx = w->x - v->at[0];
            dy = w->y - v->at[1];
            dz = w->z - v->at[2];
            ax = w->near;
            r = (sqrtf(dx * dx + dy * dy + dz * dz) - ax) / (w->far - ax);
            if (r > 1.0) {
                r = 1.0f;
            } else if (r < 0.0f) {
                r = 0.0f;
            }
            level = (s16) ((w->volume + 256) * (1.0f - r)) - 256;
            dy = v->eye[2] - v->at[2];
            ax = v->at[0] - v->eye[0];
            dx = w->x - v->eye[0];
            dz = w->z - v->eye[2];
            len = sqrtf((dy * dy + ax * ax) * (dx * dx + dz * dz));
            s = (dy * dx + ax * dz) / len;
            wsMaster2Request(w->master, level, s * 64 * w->panScale);
            break;
        case 1:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 wsDefaultProc(Obj *obj, s16 msg, void *arg) {
    return tskDefaultProc(obj, msg, arg);
}

/* "soundspot": plays its sound (eneSetup's data) while it lives. */
s32 wsSoundSpotProc(Obj *obj, s16 msg, void *arg) {
    SoundSpotObj *w = (SoundSpotObj *) ((u32) obj + sizeof(Obj));

    switch (msg) {
        case 0:
            break;
        case 2:
            obj->name = "soundspot";
            w->sound = eneSetup(obj, arg);
            sndPlay(w->sound);
            break;
        case 1:
            sndPlay(w->sound | 0x8000);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 wsDefaultProc2(Obj *obj, s16 msg, void *arg) {
    return tskDefaultProc(obj, msg, arg);
}

/* "wsmaster": plays its sound (eneSetup's data) at the loudest of the levels its sources sent
 * this frame (-0x90 if none louder). */
s32 wsMasterProc(Obj *obj, s16 msg, void *arg) {
    WsMaster1Obj *w = (WsMaster1Obj *) ((u32) obj + sizeof(Obj));
    s32 i;
    s32 pad;
    s16 max;

    switch (msg) {
        case 2:
            obj->name = "wsmaster";
            w->sound = eneSetup(obj, arg);
            sndPlay(w->sound);
            break;
        case 1:
            sndPlay(w->sound | 0x8000);
            break;
        case 0:
            max = -0x90;
            for (i = 0; i < w->count; i++) {
                if (max < w->levels[i]) {
                    max = w->levels[i];
                }
            }
            if (sndGetVolume(w->sound, 0) == 0) {
                sndPlay(w->sound);
            }
            sndSetParam(w->sound, 1, max);
            w->count = 0;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Sends master (a "wsmaster") a level for this frame (16 at most). */
void wsMasterRequest(Obj *master, s16 level) {
    WsMaster1Obj *w = (WsMaster1Obj *) ((u32) master + sizeof(Obj));

    if (master->proc == wsMasterProc) {
        if (w->count < 16) {
            w->levels[w->count] = level;
            w->count++;
        }
    }
}

/* "wsound": a source of the "wsmaster" in its file, asking it each frame for a level by the
 * camera target's distance (full within 150, silent from 350). */
/* 73 lines (mix distance 9): the float registers are now the game's (w->near f2 through n, dx
 * f12, dy f14, dz f16: the dead initialisers' order, as in wsSound2Proc); what differs is r,
 * which the game keeps in memory (its home sp+0x4C, the division's value also in a temp at
 * sp+0x24 for the "< 0" test; 3 instructions more), and the frame, 0x48 here for 0x58.
 * Measured: plain locals, 89; scratch compiles of r in two steps, a ternary clamp, "1.0 < r", the
 * distance in its own local: none keeps r in memory; "volatile f32 r" (FAKE) and pad[8], 98
 * (it reloads r after each store, which the game does not); pad[8] alone, 89 (frame 0x60);
 * initialisers n, dx, dy, dz = 0.0f with "n = w->near" after dz, 73; r's address taken (FAKE
 * "if (!&r) {}" after the clamp), 132. A HILL: r as an array, "f32 r[1];" declared right after
 * v (r[0] everywhere) with "s32 pad[4];" after dz, gives the game's frame 0x58, r at sp+0x4C and
 * mix distance 1 (116 instructions), but 79 lines: the game also parks the division's value in
 * a temp (sp+0x24) and reloads that for "< 0", where the array form reloads r[0]; so its w
 * spill is 0x24 for 0x28 and the float temps rotate one off. Without the pads, 101.
 * 2026-10-12: r as f64, real 50, length +1 (worse). The array form's frame says r lived in
 * memory as part of a local aggregate; which one is the open question (a probe next).
 * 2026-10-12 (real 33 to 21): the HILL with r assigned inside the first test,
 * "if ((r[0] = ...) > 1.0)": frame 0x58, the test's value kept for "< 0" (as the game), but in
 * f18 where the game parks it in a temp word (sp+0x24) because its f18 holds w->far (loaded
 * first, a uopt register); this C loads far later into a temp. Measured: a dead-initialised
 * "f = w->far" local after dz (pad[3]): f goes to its home, 34; plain r with the assignment in
 * the test: r in a register, 23.
 * 2026-10-12 (second try): a plain (not dead-initialised) "f32 f" after dz (pad[3]) with "f =
 * w->far;" after "n = w->near;": f gets a home (0x38), 34. Read of the game's slots: 0x20-0x27
 * (one dead word, then the division's parked value at 0x24) sit below case 2's w spill (0x28),
 * so they are front-end temps allocated before ugen's spills: an 8-aligned pair, the shape of
 * IDO's two words for a double chain (measured with a probe script). Untried: r's value
 * through a double-typed chain link.
 * Then (d): "r[0] = q = ..." chain, q tested: 21 (w spill to 0x24). w->far read into q first and
 * "q = r[0]" tested: 15 (far loaded early as the game; q colours first). q for far only, a new p
 * for the test ("p = r[0];"): 5, p in a home. "p = r[0] = ..." (a chain): the parked temp at
 * 0x24 as the game, frame 0x60 with pad[2] (14), pad[1] 4, no pad: MATCH. Plain r: frame 0x50
 * (18).
 * 2026-10-17 (a kept FAKE): the array's only effect was one more
 * word of cfe's local area (homes 44 against 40: the chain through an array element spends the
 * f32 chain word, IDO_NOTES 375/378); plain r with one pad word after p: MATCH, object identical. */
s32 wsSoundProc(Obj *obj, s16 msg, void *arg) {
    WsoundObj *w = (WsoundObj *) ((u32) obj + sizeof(Obj));
    CamView *v;
    f32 r;
    f32 n = 0.0f; /* dead initialisers; their order sets the float registers' */
    f32 dx = 0.0f;
    f32 dy = 0.0f;
    f32 dz = 0.0f;
    f32 q = 0.0f; /* the far edge (a dead initialiser, as n's) */
    f32 p; /* the ratio, tested; the chain "p = r = ..." parks it in a temp word as the game */
    s32 pad; /* FAKE: OoT's frame pad, the game's 11th word of locals (homes 44; without it 40) */

    switch (msg) {
        case 2:
            obj->name = "wsound";
            eneSetup(obj, arg);
            w->near = 150.0f;
            w->far = 350.0f;
            w->master = tskFind(gEneManager, wsMasterProc, obj->file);
            if (w->master == NULL) {
                return -1;
            }
        case 0:
            v = gGameWork.view;
            dx = w->x - v->at[0];
            dy = w->y - v->at[1];
            dz = w->z - v->at[2];
            n = w->near;
            q = w->far;
            p = r = (sqrtf(dx * dx + dy * dy + dz * dz) - n) / (q - n);
            if (p > 1.0) {
                r = 1.0f;
            } else if (p < 0.0f) {
                r = 0.0f;
            }
            wsMasterRequest(w->master, (s16) ((1.0f - r) * 256) - 256);
            break;
        case 1:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
