#include "object.h"
#include "anim.h"
#include "file.h"
#include "fileload.h"
#include "gfxmgr.h"
#include "heap.h"
#include "tmanager.h"
#include "vram.h"
#include "zanzou.h"

/* Weapon afterimages ("zanzou"): the trail a player's weapon leaves as it swings. The swing's
 * animation gives the weapon's tip and base each frame; each is fitted with a cubic spline,
 * and the trail is drawn as a textured triangle strip between the two curves, in the weapon
 * level's colour. When the swing ends, a copy ("z_cp") stays behind and fades. */

/* A player's weapon state in main (10 bytes each). */
typedef struct ZPlayerStat {
    /* 0x0 */ u8 pad0[3];
    /* 0x3 */ s8 wpLv;
    /* 0x4 */ u8 pad4[6];
} ZPlayerStat;

/* An afterimage's work. */
typedef struct ZanzouWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ Vtx *vtx[2]; /* drawn and filled in turn */
    /* 0x0C */ f32 t[2]; /* how far each track has come along its splines, 0 to 1 (base, tip) */
    /* 0x14 */ f32 driftX;
    /* 0x18 */ f32 driftZ;
    /* 0x1C */ f32 limit;
    /* 0x20 */ f32 *pt[6]; /* the knots: the tip's x, y, z, then the base's */
    /* 0x38 */ f32 last[3]; /* the player's position last step */
    /* 0x44 */ f32 lastYaw;
    /* 0x48 */ f32 step;
    /* 0x4C */ u8 *tex;
    /* 0x50 */ u8 trackLen;
    /* 0x51 */ u8 steps;
    /* 0x52 */ u8 player;
    /* 0x53 */ u8 weapon;
    /* 0x54 */ u8 wpLv;
    /* 0x55 */ u8 knots;
    /* 0x56 */ u8 verts;
    /* 0x57 */ Color3 color;
    /* 0x5A */ u8 noCopy;
    /* 0x5B */ u8 turnSlow;
    /* 0x5C */ s8 buf;
    /* 0x5D */ u8 piggyback;
} ZanzouWork;

/* A trail left behind to fade ("z_cp"). */
typedef struct ZCopyWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ Vtx *vtx;
    /* 0x08 */ u8 pad08[0xC];
    /* 0x14 */ s32 alpha;
    /* 0x18 */ u8 *tex;
    /* 0x1C */ u8 unused1C;
    /* 0x1D */ u8 verts;
    /* 0x1E */ Color3 color;
} ZCopyWork;

/* A joint record in a swing's animation (object.h's ModelNode). */
typedef struct ZJoint {
    /* 0x00 */ u8 pad00[0x12]; /* ModelNode's dl, child, sibling, scale[3], rot[3] */
    /* 0x12 */ u16 pos[3]; /* keys */
} ZJoint; /* size 0x18 */

extern G88338 gGameWork;
extern ZPlayerStat gPlayerStats[];
extern Obj *gEffectTask; /* the effect manager */
extern Gfx *gGfxDlHead; /* the display-list buffer */
extern Frame *gFrmEffectLayer; /* the frame effects go under */
extern s16 gFrmKindPol; /* "pol" */
/* zanzou.c's .data (0x8010C6B0-0x8010C754), in ROM order. */
Color3 gZanzouColorTable[6] = { /* the trail's colour, by weapon level */
    { 0x60, 0x60, 0x60 }, { 0x40, 0x40, 0x80 }, { 0x60, 0x60, 0x30 }, { 0x80, 0x80, 0x80 },
    { 0x60, 0x60, 0xA0 }, { 0x80, 0x80, 0x30 },
};
f32 gZanzouStepTable[4][4] = { /* the spline step, by weapon and swing */
    { 0.15f, 0.25f, 0.15f, 0.095f }, { 0.17f, 0.15f, 0.2f, 0.12f }, { 0.23f, 0.16f, 0.0f, 0.0f },
    { 0.22f, 0.12f, 0.13f, 0.24f },
};
u8 gZanzouTrackLenTable[4][4] = { { 3, 2, 3, 3 }, { 3, 1, 3, 3 }, { 3, 1, 0, 0 }, { 3, 1, 3, 3 } };
f32 gZanzouLimitTable[4][4] = { /* the trail's length */
    { 0.7f, 1.0f, 0.7f, 0.5f }, { 0.6f, 0.8f, 1.0f, 0.5f }, { 0.8f, 0.9f, 1.0f, 1.0f },
    { 0.9f, 0.8f, 1.0f, 1.0f },
};
extern u8 zanzouTrailTex[]; /* the trail's texture, in file 0x306 */

void plGetModelPos(Obj *player, f32 *x, f32 *y, f32 *z); /* FAKE: plmain.h is not included here: see plIsInPlay */ /* the weapon's position */
s32 plIsInPlay(u8 i); /* FAKE: plmain.h is not included here: plIsInPlay: the game's code calls it as this file declares it; plmain.h has `s32 plIsInPlay(s32 i)` */ /* whether a player is in play */
void plAskPos(Obj *player, f32 *dist, f32 *b, f32 *c, s32 *zone); /* FAKE: plmain.h is not included here: see plIsInPlay */ /* where on the course */

#define FX(obj) ((void *) ((u32) (obj) + sizeof(Obj)))

/* Starts a player's afterimage for a swing, unless the player is doing actions 0xB6-0xBE.
 * If the manager is full, it frees its first task and tries again, twice at most. */
void zanzouStart(s32 player, u32 file, ZanzouArg *arg) {
    ZanzouInit init;
    s32 n;
    f32 a;
    f32 b;
    f32 c;
    s32 pad[4];
    s32 action;

    init.anim = arg->anim;
    init.file = file;
    init.frames = arg->frames;
    init.player = player;
    if ((*plIsInPlay)(player) != 0) {
        zanzouPlayerWeapon((ZPlayer *) gGameWork.players[init.player]);
        zanzouPlayerWpLv((ZPlayer *) gGameWork.players[init.player]);
        action = zanzouPlayerAction((ZPlayer *) gGameWork.players[init.player]);
        if (action < 0xB6 || action >= 0xBF) {
            (*plAskPos)(gGameWork.players[init.player], &a, &b, &c, &n);
            if (n != 0 && init.anim != 0) {
                for (n = 0; n < 2 && tskCreateBlocks(gEffectTask,
                                                            zanzou, 1, 0,
                                                            &init) == NULL;
                     n++) {
                    if (gEffectTask != NULL && gEffectTask->child != NULL) {
                        tskDestroy(gEffectTask->child);
                    }
                }
            }
        } else {
            PRINTF("おんぶ攻撃をはじく\n");
        }
    }
}

/* The afterimage handler. Each step fills one vertex buffer while the other is drawn. */
s32 zanzou(Obj *obj, s16 msg, void *arg) {
    ZanzouWork *w = FX(obj);
    s32 b;
    s32 i;
    s32 k;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "zanzou";
            w->frame = (PolFrame *) frmCreate(gFrmEffectLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->flags |= 0x20;
            if (zanzouSetup(obj, arg) == 1) {
                return -1;
            }
            (*plGetModelPos)(gGameWork.players[w->player], &w->frame->pos[0],
                          &w->frame->pos[1], &w->frame->pos[2]);
            for (b = 0; b < 2; b++) {
                for (i = 0, k = 5; i < w->verts; i += 4) {
                    w->vtx[b][i].v.tc[0] = 0;
                    w->vtx[b][i].v.tc[1] = k << 6;
                    w->vtx[b][i + 2].v.tc[0] = 0;
                    w->vtx[b][i + 2].v.tc[1] = (k + 3) << 6;
                    k += 6;
                }
                for (i = 1, k = 5; i < w->verts; i += 4) {
                    w->vtx[b][i].v.tc[0] = 0x800;
                    w->vtx[b][i].v.tc[1] = k << 6;
                    w->vtx[b][i + 2].v.tc[0] = 0x800;
                    w->vtx[b][i + 2].v.tc[1] = (k + 3) << 6;
                    k += 6;
                }
            }
            w->steps = 0;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 0.1f;
            break;
        case 1:
            if (w->frame != NULL) {
                (*frmDestroy)(&w->frame->f);
            }
            if (w->vtx[0] != NULL) {
                heapFree(w->vtx[0]);
            }
            if (w->vtx[1] != NULL) {
                heapFree(w->vtx[1]);
            }
            if (w->pt[0] != NULL) {
                heapFree(w->pt[0]);
            }
            if (w->pt[1] != NULL) {
                heapFree(w->pt[1]);
            }
            if (w->pt[2] != NULL) {
                heapFree(w->pt[2]);
            }
            if (w->pt[3] != NULL) {
                heapFree(w->pt[3]);
            }
            if (w->pt[4] != NULL) {
                heapFree(w->pt[4]);
            }
            if (w->pt[5] != NULL) {
                heapFree(w->pt[5]);
            }
            break;
        case 3:
            w->frame->dl2 = gGfxDlHead;
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->color.r, w->color.g, w->color.b, 0xA0);
            gDPSetCombineLERP(gGfxDlHead++, 0, 0, 0, PRIMITIVE, PRIMITIVE, 0, TEXEL0, 0, 0, 0, 0,
                              PRIMITIVE, PRIMITIVE, 0, TEXEL0, 0);
            zanzouDrawStrip(w->vtx[!w->buf], w->verts, w->tex);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            if (zanzouStep(obj) == 1) {
                return -1;
            }
            w->buf = !w->buf;
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* Matched 2026-10-02 (633 lines at the start): "scale = 1.0f" as a local (the game multiplies by
 * it), n as a ternary, "default: break;" in the switch, the tip in two statements (the
 * permuter's), the header word in its own local "head" (the game keeps it in v1), the frame
 * count converted straight to u8 for the knots ("(u8) nframes - 1": the u8 conversion spends
 * the temporary the game skips; "(u32)" left every later t-register one low), and case 0x38
 * setting its width last, as the other cases do.
 * Sets an afterimage up from its swing. 1 if it is not to be drawn: too few frames, a
 * player action it does not follow, or no memory.
 * - The swing's header: the frame count in its low byte; its third word points past the
 *   joint records (24 bytes each). The tip is the joint n from the end (n is 3, or 6 for
 *   weapon 2), the base the joint two after.
 * - The trail's length, spline step and width come from per-weapon tables, by which of the
 *   five swings (0x34-0x38 after the weapon level's offset) it is.
 * The two dead prints are this function's: their strings follow "zanzou" in .rodata. They sat
 * in the handler as a FAKE while this function was assembly; here they change no code. */
s32 zanzouSetup(Obj *obj, ZanzouInit *init) {
    ZanzouWork *w = FX(obj);
    u32 *hdr;
    u32 head;
    ZJoint *tip;
    ZJoint *base;
    Color3 *c;
    f32 nframes;
    f32 t;
    f32 scale = 1.0f;
    s32 action;
    s32 v;
    s32 n;
    s32 i;

    w->player = init->player;
    w->weapon = zanzouPlayerWeapon((ZPlayer *) gGameWork.players[w->player]);
    w->wpLv = zanzouPlayerWpLv((ZPlayer *) gGameWork.players[w->player]);
    c = &gZanzouColorTable[w->wpLv];
    w->color.r = c->r;
    w->color.g = c->g;
    w->color.b = c->b;
    w->tex = zanzouTrailTex;
    w->frame->segs[0] = fileRamAddr(0x306);
    w->frame->segs[2] = init->file;
    hdr = (u32 *) ((init->anim & 0xFFFFFF) + init->file);
    head = hdr[0];
    nframes = head & 0xFF;
    w->knots = (u8) nframes - 1;
    if (w->knots < 5) {
        return 1;
    }
    action = zanzouPlayerAction((ZPlayer *) gGameWork.players[w->player]);
    if (action >= w->wpLv * 3 + 0xB6 && w->wpLv * 3 + 0xBE >= action) {
        return 1;
    }
    if (w->wpLv * 5 + 0x38 < action || action < w->wpLv * 5 + 0x34) {
        PRINTF("zanzou : PA unknown\n");
        return 1;
    }
    w->step = init->frames / (nframes - 1.0f);
    w->verts = 20;
    if (w->piggyback == 0) {
        v = action - w->wpLv * 5;
    } else {
        v = action - w->wpLv * 3;
    }
    switch (v) {
        case 0x34:
        case 0xB6:
            w->limit = gZanzouLimitTable[w->weapon][0];
            w->step = gZanzouStepTable[w->weapon][0];
            w->trackLen = gZanzouTrackLenTable[w->weapon][0];
            break;
        case 0x35:
            w->limit = gZanzouLimitTable[w->weapon][1];
            w->step = gZanzouStepTable[w->weapon][1];
            if (w->weapon == 0) {
                w->step = 0.13f;
            }
            w->trackLen = gZanzouTrackLenTable[w->weapon][1];
            break;
        case 0x36:
            w->limit = gZanzouLimitTable[w->weapon][2];
            w->step = gZanzouStepTable[w->weapon][2];
            w->trackLen = gZanzouTrackLenTable[w->weapon][2];
            break;
        case 0x37:
        case 0xB7:
        case 0xB8:
            w->limit = gZanzouLimitTable[w->weapon][3];
            w->step = gZanzouStepTable[w->weapon][3];
            w->trackLen = gZanzouTrackLenTable[w->weapon][3];
            break;
        case 0x38:
            w->limit = gZanzouLimitTable[w->weapon][3];
            w->step = gZanzouStepTable[w->weapon][3];
            w->trackLen = 1;
            break;
        default:
            PRINTF("zanzou : WP_LV unknown\n");
            break;
    }
    n = (w->weapon == 2) ? 6 : 3;
    if (zanzouAllocBuffers(obj, w->knots, w->verts) == 1) {
        return 1;
    }
    hdr += 2;
    tip = (ZJoint *) ((*hdr & 0xFFFFFF) + init->file);
    tip = tip - n;
    t = 0.0f;
    for (i = 0; i < w->knots; i++) {
        base = tip + 2;
        animSeek(init->anim, t, &w->frame->segs[0]);
        w->pt[0][i] = animGetPos(tip->pos[0]) / 32.0f * scale;
        w->pt[1][i] = animGetPos(tip->pos[1]) / 32.0f * scale;
        w->pt[2][i] = animGetPos(tip->pos[2]) / 32.0f * scale;
        w->pt[3][i] = animGetPos(base->pos[0]) / 32.0f;
        w->pt[4][i] = animGetPos(base->pos[1]) / 32.0f;
        w->pt[5][i] = animGetPos(base->pos[2]) / 32.0f;
        if (i > 0) {
            if (w->pt[0][i] == w->pt[0][i - 1] && w->pt[1][i] == w->pt[1][i - 1] &&
                w->pt[2][i] == w->pt[2][i - 1]) {
                w->pt[0][i] += 0.01;
            }
            if (w->pt[3][i] == w->pt[3][i - 1] && w->pt[4][i] == w->pt[4][i - 1] &&
                w->pt[5][i] == w->pt[5][i - 1]) {
                w->pt[3][i] += 0.01;
            }
        }
        t += 1.0;
    }
    if (w->weapon == 2) {
        w->pt[0][0] = w->pt[0][1] - 0.1;
        w->pt[1][0] = w->pt[1][1] - 0.1;
        w->pt[2][0] = w->pt[2][1] - 0.1;
        w->pt[3][0] = w->pt[3][1] - 0.1;
        w->pt[4][0] = w->pt[4][1] - 0.1;
        w->pt[5][0] = w->pt[5][1] - 0.1;
    }
    return 0;
}

/* Allocates and clears the knot arrays and the two vertex buffers. 1 if one is refused. */
s32 zanzouAllocBuffers(Obj *obj, s32 knots, s32 verts) {
    ZanzouWork *w = FX(obj);
    s32 pad;

    w->pt[0] = NULL;
    w->pt[1] = NULL;
    w->pt[2] = NULL;
    w->pt[3] = NULL;
    w->pt[4] = NULL;
    w->pt[5] = NULL;
    w->vtx[0] = NULL;
    w->vtx[1] = NULL;
    if ((w->pt[0] = heapMalloc(knots * 4)) == NULL) {
        return 1;
    }
    if ((w->pt[1] = heapMalloc(knots * 4)) == NULL) {
        return 1;
    }
    if ((w->pt[2] = heapMalloc(knots * 4)) == NULL) {
        return 1;
    }
    if ((w->pt[3] = heapMalloc(knots * 4)) == NULL) {
        return 1;
    }
    if ((w->pt[4] = heapMalloc(knots * 4)) == NULL) {
        return 1;
    }
    if ((w->pt[5] = heapMalloc(knots * 4)) == NULL) {
        return 1;
    }
    if ((w->vtx[0] = heapMalloc(verts * sizeof(Vtx))) == NULL) {
        return 1;
    }
    if ((w->vtx[1] = heapMalloc(verts * sizeof(Vtx))) == NULL) {
        return 1;
    }
    heapMemClear(w->pt[0], knots * 4);
    heapMemClear(w->pt[1], knots * 4);
    heapMemClear(w->pt[2], knots * 4);
    heapMemClear(w->pt[3], knots * 4);
    heapMemClear(w->pt[4], knots * 4);
    heapMemClear(w->pt[5], knots * 4);
    heapMemClear(w->vtx[0], verts * sizeof(Vtx));
    heapMemClear(w->vtx[1], verts * sizeof(Vtx));
    return 0;
}

/* A step of an afterimage. The knots are carried with the player: on the first step turned
 * to the course's heading, then moved by the player's movement (times 11) and turned by the
 * player's turn. The splines then grow by a step. 1 when the trail ends: its action is
 * over, it has gone its length (leaving a copy), or 31 steps. */
s32 zanzouStep(Obj *obj) {
    ZanzouWork *w = FX(obj);
    f32 h;
    s32 zone;
    s32 action;
    s32 i;
    f32 dist;
    f32 b;
    f32 c;
    f32 yaw;
    s32 first;
    f32 turn;
    f32 mf[4][4];
    f32 m2[4][4];
    f32 m1[4][4];
    Vec3f pos;
    f32 d[3];

    action = zanzouPlayerAction((ZPlayer *) gGameWork.players[w->player]);
    yaw = zanzouPlayerYaw((ZPlayer *) gGameWork.players[w->player]);
    (*plGetModelPos)(gGameWork.players[w->player], &pos.x, &pos.y, &pos.z);
    if (w->steps == 0) {
        w->lastYaw = yaw;
        (*plAskPos)(gGameWork.players[w->player], &dist, &b, &c, &zone);
        h = (*vramCourseHeading)(zone, dist);
        (*guRotateF)(mf, (yaw + h) * 57.295776f, 0.0f, 1.0f, 0.0f);
        for (i = 0; i < w->knots; i++) {
            (*guMtxXFMF)(mf, w->pt[0][i], w->pt[1][i], w->pt[2][i], &w->pt[0][i], &w->pt[1][i],
                         &w->pt[2][i]);
            (*guMtxXFMF)(mf, w->pt[3][i], w->pt[4][i], w->pt[5][i], &w->pt[3][i], &w->pt[4][i],
                         &w->pt[5][i]);
        }
        (*plGetModelPos)(gGameWork.players[w->player], &w->frame->pos[0], &w->frame->pos[1],
                      &w->frame->pos[2]);
        (*plGetModelPos)(gGameWork.players[w->player], &w->last[0], &w->last[1], &w->last[2]);
        w->noCopy = 0;
        w->turnSlow = 0;
        w->t[0] = 0.0f;
        w->t[1] = 0.0f;
        w->driftX = 0.0f;
        w->driftZ = 0.0f;
    }
    d[0] = pos.x - w->last[0];
    d[1] = pos.y - w->last[1];
    d[2] = pos.z - w->last[2];
    w->driftX += d[0];
    w->driftZ += d[2];
    turn = yaw - w->lastYaw;
    if (turn != 0.0) {
        if (w->turnSlow == 0) {
            w->step -= 0.1;
            w->turnSlow = 1;
        }
        (*guTranslateF)(m1, -w->driftX * 11.0f, 0.0f, -w->driftZ * 11.0f);
        (*guTranslateF)(m2, w->driftX * 11.0f, d[1] * 11.0f, w->driftZ * 11.0f);
        (*guRotateF)(mf, 57.295776f * turn, 0.0f, 1.0f, 0.0f);
        guMtxCatF(m1, mf, mf);
        guMtxCatF(mf, m2, m2);
        first = 3;
    } else {
        (*guTranslateF)(m2, d[0] * 11.0f, d[1] * 11.0f, d[2] * 11.0f);
        if (w->trackLen == gZanzouTrackLenTable[0][1] && d[1] < 0.0f) {
            first = 3;
        } else {
            first = 2;
        }
    }
    for (i = first; i < w->knots; i++) {
        (*guMtxXFMF)(m2, w->pt[0][i], w->pt[1][i], w->pt[2][i], &w->pt[0][i], &w->pt[1][i],
                     &w->pt[2][i]);
        (*guMtxXFMF)(m2, w->pt[3][i], w->pt[4][i], w->pt[5][i], &w->pt[3][i], &w->pt[4][i],
                     &w->pt[5][i]);
    }
    if (turn == 0.0f || ((turn > 0.0f) ? turn : -turn) > 6.0f) {
        if (w->turnSlow == 1) {
            w->step += 0.1;
            w->turnSlow = 2;
        }
    }
    if (zanzouLayTrack(obj, w->pt[0], w->pt[1], w->pt[2], 1, w->trackLen, turn) == 1) {
        return 1;
    }
    if (zanzouLayTrack(obj, w->pt[3], w->pt[4], w->pt[5], 0, w->trackLen, turn) == 1) {
        return 1;
    }
    w->lastYaw = zanzouPlayerYaw((ZPlayer *) gGameWork.players[w->player]);
    (*plGetModelPos)(gGameWork.players[w->player], &w->last[0], &w->last[1], &w->last[2]);
    if (w->piggyback != 0) {
        if (action < w->wpLv * 3 + 0xB6 || w->wpLv * 3 + 0xB8 < action) {
            return 1;
        }
    } else {
        if (action < w->wpLv * 5 + 0x34 || w->wpLv * 5 + 0x38 < action) {
            return 1;
        }
    }
    if (w->limit < w->t[0]) {
        if (w->noCopy == 0) {
            zanzouLeaveCopy(obj);
        }
        return 1;
    }
    if (++w->steps >= 31) {
        return 1;
    }
    return 0;
}

/* Draws the trail: n vertices as a strip of triangle pairs, textured with tex (64x32, IA 8-bit),
 * blended over what is there. The loop counts by the pair's last vertex (i < n). */
void zanzouDrawStrip(Vtx *vtx, s32 n, u8 *tex) {
    s32 i;

    gDPPipeSync(gGfxDlHead++);
    gDPSetCycleType(gGfxDlHead++, G_CYC_1CYCLE);
    gDPSetTexturePersp(gGfxDlHead++, G_TP_PERSP);
    gDPSetRenderMode(gGfxDlHead++, G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2);
    gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 255);
    gDPPipeSync(gGfxDlHead++);
    gDPSetBlendColor(gGfxDlHead++, 0, 0, 0, 1);
    gDPSetAlphaCompare(gGfxDlHead++, G_AC_THRESHOLD);
    gDPLoadTextureBlock(gGfxDlHead++, tex, G_IM_FMT_IA, G_IM_SIZ_8b, 64, 32, 0,
                        G_TX_MIRROR | G_TX_WRAP, G_TX_CLAMP, 5, 5, G_TX_NOLOD, G_TX_NOLOD);
    gDPSetTextureLOD(gGfxDlHead++, G_TL_TILE);
    gDPSetTextureDetail(gGfxDlHead++, G_TD_CLAMP);
    gDPSetTextureLUT(gGfxDlHead++, G_TT_NONE);
    gDPSetTextureFilter(gGfxDlHead++, G_TF_BILERP);
    gDPSetTextureConvert(gGfxDlHead++, G_TC_FILT);
    gSPClearGeometryMode(gGfxDlHead++, G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING |
                                           G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD);
    gSPSetGeometryMode(gGfxDlHead++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH);
    gSPTexture(gGfxDlHead++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON);
    osWritebackDCache(vtx, n * sizeof(Vtx));
    gSPVertex(gGfxDlHead++, vtx, n, 0);
    for (i = 3; i < n; i += 2) {
        gSP2Triangles(gGfxDlHead++, i - 3, i - 1, i - 2, 0, i - 2, i, i - 1, 0);
    }
    gSPEndDisplayList(gGfxDlHead++);
}

/* A player's facing, with its turn. */
f32 zanzouPlayerYaw(ZPlayer *p) {
    return p->yaw + p->yaw2;
}

s32 zanzouPlayerWeapon(ZPlayer *p) {
    return p->weapon;
}

s32 zanzouPlayerAction(ZPlayer *p) {
    return p->action;
}

void zanzouEmptyStub(void) {
}

/* A player's weapon level. */
s8 zanzouPlayerWpLv(ZPlayer *p) {
    return gPlayerStats[p->index].wpLv;
}

/* Lays one track of the strip (side 1: the tip's odd vertices; 0: the base's even ones) along
 * its knots' splines. At the first step every vertex sits at the start. After that each vertex
 * takes the one `len` pairs on from the other buffer, and the last pairs are sampled from the
 * spline between the old t and the new, t having moved on by the step. 1 when the base has
 * reached the end with a weapon out: a copy is left to fade.
 * Matched 2026-10-03: i holds the arrays' size before it is the vertex index (the game's s2 for
 * both), and u is a named local declared after z2a. A separate size local swapped s1/s2 with obj
 * and, with u, cost the frame a word (0xB8). With i as the size, no vp copy is needed and the
 * stack is the game's (0xB0). The end test is the plain "side == 0": uopt itself tests the
 * side * 4 it spilled for w->t[side] (2026-10-04: the unsigned "side * sizeof(f32)" FAKE that
 * stood here was needed only in the earlier forms; with i as the size the plain test matches).
 * The empty `if (1) {}` (a do-while(0) until 2026-10-17: the same blocks, object identical, a kept
 * FAKE) at the head of the return-1 path is a FAKE block boundary (without it,
 * 454 instructions for 451).
 * What it stands for (2026-10-04, uopt -zdbug:5): its two blocks lie in the live spans of ta,
 * x2a and xk/yk/zk and lower xk/yk/zk's priority (2.09 to 1.92) below ta's (2.0) and x2a's
 * (1.93), so ta, x2a and xk get s6, s7 and fp, as in the game; without it xk, yk, zk take them.
 * Anything that leaves a branch or a loop there with no code also matches: "if (0) {}", or
 * "while" for this "if" (its body always returns), or a do-while(0) around this path's sample
 * and its three stores (not also around the last sample: 511 cc -S lines off; nor all four,
 * as one sample macro would put them: 659). These
 * leave nothing and do not: an unused label, an assert under NDEBUG, an empty-expansion
 * DPRINTF (which also emits its string). The boundary just before the end test gives the
 * registers but a temp word more (frame 0xB8). So the original had, at the head of this path,
 * a statement that compiled to no code and no data but left a dead branch or an empty loop:
 * most likely a debug macro compiled out to "if (0) {}" or "do {} while (0)" that swallowed
 * its argument. It was not a PRINTF: that would leave a string, and the file's .rodata holds
 * only "spline make miss" (the failure branch's) between the init's prints and "z_cp".
 * How it was found (history, 669 lines to a match):
 * size/obj swap s1/s2 (the game: size then i in s2, obj then i * 16
 * in s1), and the end test: the game spills side * 4 (the index of w->t[side], computed after
 * in s1), and the end test: the game spills side * 4 (the index of w->t[side], computed after
 * the loops) at 0x78 and tests that; this C reads side from its home (one sw fewer, so every
 * later branch is 4 off). ta, x2a (s6/s7) and xk (fp), yk/zk read from their homes, the frame
 * 0xB0, as the game.
 * 2026-10-02 (194 to 67, the permuter's second find): "(side ^ 0) == 0" (FAKE; rule 70's form)
 * takes 8 off the frame. "!side", "side * 4 == 0" (folded back) and "&w->t[side] == w->t" (the
 * pointer spill is tested, not side * 4: 67) do not give the game's spill. "(side << 2) == 0"
 * (2026-10-03): not shared with the index's sll (one sll more, one sw fewer), frame +8, 194.
 * "(&w->t[side] - w->t) == 0": 227 (a divide's shift kept, mix distance 3).
 * 2026-10-03: the test UNSIGNED, "side * sizeof(f32) == 0" (IDO folds the signed product back to
 * side, but cannot fold an unsigned one): the game's code exactly, the index spilled at the
 * game's slot and tested (451 lines, mix distance 0), but the frame 0xB8 again (194: every
 * temp a word up, the locals two). "(side ^ 0) * sizeof(f32) == 0": 194, mix distance 2.
 * Its slots against the game's: an unused word at 0x68 under the temps (float 0x6C, pointer
 * 0x70, float 0x74, index 0x7C; the game 0x68/0x6C/0x70/0x78) and the frame rounded to 0xB8.
 * The plain "side == 0" has the same dead bottom temp, so it is not the unsigned test's. The
 * lead: the unsigned test with whatever takes that dead temp away (the (side ^ 0) form's 8).
 * Found (2026-10-03, the permuter on the unsigned form, 657 to 449): the third copy's index in
 * a new local, "j = i + n2; ... [!w->buf][j].v.ob[2]" (j declared second): the dead temp goes (a
 * named local's home takes its word), every temp slot is the game's (0x68/0x6C/0x70/0x78), mix
 * distance 0, 160 lines, but the locals 4 high and the frame 0xB8: one declared word too many.
 * size reused for the index instead: 194 (the temp back). j kept and w as a macro on FX(obj)
 * (no w local): the stack maps IDENTICAL to the game's, but 219 lines, mix distance 3 (IDO folds
 * obj + 0x20 into the offsets where the game keeps w in s0). So the game has j's word and no
 * word for w, with w still in s0: find what keeps obj + 0x20 in a register without a declared
 * w (or which other declared local the game lacks). Not n2: j kept and "len * 2" written at
 * n2's uses (no n2 local): 254, mix distance 6. j as a second loop variable for all three
 * copies ("i = side, j = side + n2; ...; i += 2, j += 2"): 403. The unused parameter turn
 * reused as u (no u local; j after size): 194, temps and locals moved up again. Note j's place
 * matters: second (after w) in the 160 form; with j second and turn as u: 192.
 * The permuter's next find (657 to 353) is closer: unsigned test, u declared second (after w),
 * and the SECOND copy read through a pointer copy, "vp = w->vtx; ... = vp[!w->buf][i +
 * n2].v.ob[1];" (Vtx **vp declared after ta): 112 lines, mix distance 0, every temp slot and
 * the spills at 0x84/0x90/0x94 the game's; only x, y, z 4 high (0xA4 for 0xA0) and the frame
 * 0xB8: one declared word too many, and w and u both above x where the game has one word. Its
 * block boundary before the last return: no change. turn as u in that form: 192. Not kept (67
 * lines here, the keep rule), but it is the closest code: start from it.
 * KEPT, as an exception to the keep rule (76 lines from 67, but mix distance 0 from 1): that
 * form with vp declared FIRST (locals stack up from the temps, so x, y, z drop to the game's
 * 0xA0-0xA8): the game's code, every temp and local slot; only the frame is 0xB8 for 0xB0, so
 * the parameters' homes read 8 high. The game has ONE declared word above x where this C has
 * three (vp, w, u). w as a macro there: 301, mix distance 3. Find the two words to drop. Not
 * size: written inline, 172 with mix distance 22 (w->knots reloaded after every call).
 * 2026-10-02 (669 to 194, the permuter's find, narrowed by hand): a block boundary at the head
 * of the return-1 path, the FAKE empty do-while. Before it the original's ta/x2a/xk lost their
 * registers to xk/yk/zk. Empty splits after the copy call, after free(x2a) or before
 * "return 1", nested ifs for the end test, and the spline print in the spline-failure branch
 * (its string sits between the init's prints and FA668's "z_cp") change nothing (669); a dead
 * PRINTF at the head of the path does the same as the FAKE, but no string is left for it.
 * "spline make miss" was dropped from the handler's FAKE prints at the unparking (it is here).
 * 2026-10-03: vp taken out (w->vtx at the second copy): 192 lines, mix distance 0, the s3
 * copy of w (the else loop's bottom test) STILL there, frame 0xB8 the same, but every temp a
 * word up (the dead word at 0x68 back). So vp does not make the s3 copy: it only trades the
 * dead bottom temp for a declared word. The real gap is TWO words: one dead temp the game does
 * not have, and one declared word too many above x (w, u: the game has one).
 * "tools/permuter.py drops" (2026-10-03): no single statement's removal gives the game's map.
 * The first-step call (0.0f) removed: frame 0xB0 but x, y, z 8 low (two temp words go with it,
 * not the words above x); the loop's call: 0xA8, 16 low; the copy loop's vp line: no move.
 * 2026-10-03 (76 to 24): vp dropped and u written as the unused float parameter turn: the
 * frame 0xB0 and every slot the game's, only size/obj in s1/s2 swapped. Then size dropped, i
 * holding the size, u named again after z2a: a match. Declaration order of size, i, ta, x2a,
 * and size written at its declaration did not move the s1/s2 swap. */
s32 zanzouLayTrack(Obj *obj, f32 *xk, f32 *yk, f32 *zk, s32 side, s32 len, f32 turn) {
    ZanzouWork *w = FX(obj);
    f32 x;
    f32 y;
    f32 z;
    f32 *ta;
    f32 *x2a = NULL;
    f32 *y2a = NULL;
    f32 *z2a = NULL;
    f32 u;
    s32 i;
    s32 n2;

    i = w->knots * sizeof(f32);
    if ((ta = heapMalloc(i)) == NULL) {
        goto end;
    }
    if ((x2a = heapMalloc(i)) == NULL) {
        goto end;
    }
    if ((y2a = heapMalloc(i)) == NULL) {
        goto end;
    }
    if ((z2a = heapMalloc(i)) == NULL) {
        goto end;
    }
    heapMemClear(ta, i);
    heapMemClear(x2a, i);
    heapMemClear(y2a, i);
    heapMemClear(z2a, i);
    if (zanzouSplineMake(ta, xk, yk, zk, x2a, y2a, z2a, w->knots) == 1) {
        PRINTF("spline make miss\n");
        goto end;
    }
    n2 = len * 2;
    if (w->steps == 0) {
        for (i = side; i < w->verts - n2; i += 2) {
            zanzouSplinePoint(0.0f, &x, &y, &z, ta, xk, yk, zk, x2a, y2a, z2a, w->knots);
            w->vtx[w->buf][i].v.ob[0] = x;
            w->vtx[w->buf][i].v.ob[1] = y;
            w->vtx[w->buf][i].v.ob[2] = z;
        }
    } else {
        for (i = side; i < w->verts - n2; i += 2) {
            w->vtx[w->buf][i].v.ob[0] = w->vtx[!w->buf][i + n2].v.ob[0];
            w->vtx[w->buf][i].v.ob[1] = w->vtx[!w->buf][i + n2].v.ob[1];
            w->vtx[w->buf][i].v.ob[2] = w->vtx[!w->buf][i + n2].v.ob[2];
        }
    }
    w->t[side] += w->step;
    if (w->t[side] > 1.0) {
        w->t[side] = 1.0f;
    }
    if (len >= 2) {
        u = w->t[side] - w->step + w->step / len;
        for (; i < w->verts - 2; i += 2) {
            if (u >= 1.0) {
                u = 1.0f;
            }
            zanzouSplinePoint(u, &x, &y, &z, ta, xk, yk, zk, x2a, y2a, z2a, w->knots);
            w->vtx[w->buf][i].v.ob[0] = x;
            w->vtx[w->buf][i].v.ob[1] = y;
            w->vtx[w->buf][i].v.ob[2] = z;
            u += w->step / len;
        }
    }
    if (w->t[side] == 1.0 && side == 0 && w->weapon != 0) {
        if (1) {} /* FAKE: OoT's `if (1) {}`, a block boundary at the head of this path (see the note) */
        zanzouSplinePoint(w->t[side], &x, &y, &z, ta, xk, yk, zk, x2a, y2a, z2a, w->knots);
        w->vtx[w->buf][i].v.ob[0] = x;
        w->vtx[w->buf][i].v.ob[1] = y;
        w->vtx[w->buf][i].v.ob[2] = z;
        zanzouLeaveCopy(obj);
        heapFree(ta);
        heapFree(x2a);
        heapFree(y2a);
        heapFree(z2a);
        return 1;
    }
    zanzouSplinePoint(w->t[side], &x, &y, &z, ta, xk, yk, zk, x2a, y2a, z2a, w->knots);
    w->vtx[w->buf][i].v.ob[0] = x;
    w->vtx[w->buf][i].v.ob[1] = y;
    w->vtx[w->buf][i].v.ob[2] = z;

end:
    if (ta != NULL) {
        heapFree(ta);
    }
    if (x2a != NULL) {
        heapFree(x2a);
    }
    if (y2a != NULL) {
        heapFree(y2a);
    }
    if (z2a != NULL) {
        heapFree(z2a);
    }
    return 0;
}

/* The spline coefficients y2 for the n knots (x, y), ends at 0, for zanzouSplineEval. The first
 * loop stores each slope one slot up, d[i + 1]; the doublings are int ("* 2": "* 2.0f" would
 * become an add), and the failed allocations go to the frees by goto, as in zanzouLayTrack. */
s32 zanzouSplineCoeffs(f32 *x, f32 *y, f32 *y2, s32 n) {
    f32 *d = NULL;
    f32 *h;
    f32 f;
    s32 i;

    if ((h = heapMalloc(n * sizeof(f32))) == NULL) {
        goto end;
    }
    if ((d = heapMalloc(n * sizeof(f32))) == NULL) {
        goto end;
    }
    heapMemClear(h, n * sizeof(f32));
    heapMemClear(d, n * sizeof(f32));
    y2[0] = 0.0f;
    y2[n - 1] = 0.0f;
    for (i = 0; i < n - 1; i++) {
        h[i] = x[i + 1] - x[i];
        d[i + 1] = (y[i + 1] - y[i]) / h[i];
    }
    y2[1] = (d[2] - d[1]) - h[0] * y2[0];
    d[1] = (x[2] - x[0]) * 2;
    for (i = 1; i < n - 2; i++) {
        f = h[i] / d[i];
        y2[i + 1] = (d[i + 2] - d[i + 1]) - y2[i] * f;
        d[i + 1] = 2 * (x[i + 2] - x[i]) - h[i] * f;
    }
    y2[n - 2] -= h[n - 2] * y2[n - 1];
    for (i = n - 2; i > 0; i--) {
        y2[i] = (y2[i] - h[i] * y2[i + 1]) / d[i];
    }
end:
    if (h != NULL) {
        heapFree(h);
    }
    if (d != NULL) {
        heapFree(d);
    }
    return 0;
}

/* A cubic spline through the n knots (xa, ya), with the coefficients y2a, at x. The int
 * literals 3 and 2 keep the game's mul.s by 3.0 and 2.0 (2.0f would fold to an add.s) and its
 * operand order. */
f32 zanzouSplineEval(f32 x, f32 *xa, f32 *ya, f32 *y2a, s32 n) {
    s32 lo = 0;
    s32 hi = n - 1;
    s32 k;
    f32 h;
    f32 d;

    while (lo < hi) {
        k = (lo + hi) / 2;
        if (xa[k] < x) {
            lo = k + 1;
        } else {
            hi = k;
        }
    }
    if (lo > 0) {
        lo--;
    }
    h = xa[lo + 1] - xa[lo];
    d = x - xa[lo];
    return (((y2a[lo + 1] - y2a[lo]) * d / h + y2a[lo] * 3) * d +
            ((ya[lo + 1] - ya[lo]) / h - (y2a[lo] * 2 + y2a[lo + 1]) * h)) *
               d +
           ya[lo];
}

/* Fits the three splines through n knots: t is the distance along them, from 0 to 1. 1 if a fit
 * fails. */
s32 zanzouSplineMake(f32 *ta, f32 *xa, f32 *ya, f32 *za, f32 *x2a, f32 *y2a, f32 *z2a,
                           s32 n) {
    s32 i;
    f32 dx;
    f32 dy;
    f32 dz;

    ta[0] = 0.0f;
    for (i = 1; i < n; i++) {
        dx = xa[i] - xa[i - 1];
        dy = ya[i] - ya[i - 1];
        dz = za[i] - za[i - 1];
        ta[i] = ta[i - 1] + sqrtf(dx * dx + dy * dy + dz * dz);
    }
    for (i = 1; i < n; i++) {
        ta[i] /= ta[n - 1];
    }
    if (zanzouSplineCoeffs(ta, xa, x2a, n) == 1) {
        return 1;
    }
    if (zanzouSplineCoeffs(ta, ya, y2a, n) == 1) {
        return 1;
    }
    if (zanzouSplineCoeffs(ta, za, z2a, n) == 1) {
        return 1;
    }
    return 0;
}

/* A point on the three splines at t. */
void zanzouSplinePoint(f32 t, f32 *x, f32 *y, f32 *z, f32 *ta, f32 *xa, f32 *ya, f32 *za,
                            f32 *x2a, f32 *y2a, f32 *z2a, s32 n) {
    *x = zanzouSplineEval(t, ta, xa, x2a, n);
    *y = zanzouSplineEval(t, ta, ya, y2a, n);
    *z = zanzouSplineEval(t, ta, za, z2a, n);
}

/* The copy's task: a trail left behind that fades by 40 a step, in the colour three weapon
 * levels on. It copies the trail's vertices (twice over) without their colours. */
s32 z_cp(Obj *obj, s16 msg, void *arg) {
    ZanzouWork *src = arg;
    s32 pad;
    ZCopyWork *w = FX(obj);
    s32 k;
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "z_cp";
            w->frame = (PolFrame *) frmCreate(gFrmEffectLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->flags |= 0x20;
            floadGetSetRam((void **) w->frame->segs, 0x128);
            w->frame->pos[0] = src->frame->pos[0];
            w->frame->pos[1] = src->frame->pos[1];
            w->frame->pos[2] = src->frame->pos[2];
            w->tex = src->tex;
            w->verts = src->verts;
            /* (D + 3)[lv], not D[lv + 3]: an int "+ 3" puts the switch's case 3 in v1. */
            w->color.r = (gZanzouColorTable + 3)[src->wpLv].r;
            w->color.g = (gZanzouColorTable + 3)[src->wpLv].g;
            w->color.b = (gZanzouColorTable + 3)[src->wpLv].b;
            w->vtx = heapMalloc(w->verts * sizeof(Vtx));
            if (w->vtx == NULL) {
                return -1;
            }
            heapMemClear(w->vtx, w->verts * sizeof(Vtx));
            for (k = 0; k < 2; k++) {
                for (i = 0; i < w->verts; i++) {
                    w->vtx[i].v.ob[0] = src->vtx[src->buf][i].v.ob[0];
                    w->vtx[i].v.ob[1] = src->vtx[src->buf][i].v.ob[1];
                    w->vtx[i].v.ob[2] = src->vtx[src->buf][i].v.ob[2];
                    w->vtx[i].v.flag = src->vtx[src->buf][i].v.flag;
                    w->vtx[i].v.tc[0] = src->vtx[src->buf][i].v.tc[0];
                    w->vtx[i].v.tc[1] = src->vtx[src->buf][i].v.tc[1];
                }
            }
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 0.1f;
            w->alpha = 0xFF;
            break;
        case 1:
            if (w->vtx != NULL) {
                heapFree(w->vtx);
            }
            if (w->frame != NULL) {
                (*frmDestroy)(&w->frame->f);
            }
            break;
        case 3:
            w->frame->dl2 = gGfxDlHead;
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->color.r, w->color.g, w->color.b,
                            (u8) w->alpha);
            gDPSetCombineLERP(gGfxDlHead++, 0, 0, 0, PRIMITIVE, PRIMITIVE, 0, TEXEL0, 0, 0, 0, 0,
                              PRIMITIVE, PRIMITIVE, 0, TEXEL0, 0);
            zanzouDrawStrip(w->vtx, w->verts, w->tex);
            break;
        case 0:
            if ((w->alpha -= 40) < 0) {
                return -1;
            }
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* Leaves a copy of an afterimage's trail behind, to fade. */
void zanzouLeaveCopy(Obj *obj) {
    tskCreate(gEffectTask, z_cp, 0, FX(obj));
}
