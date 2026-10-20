#include "tlb.h"
f32 ene20BSpawnOffsets[5][3] = { /* the pieces' places when it breaks */
    { 12.0f, 15.0f, 0.0f }, { 18.0f, -5.0f, 0.0f }, { 0.0f, -20.0f, 0.0f }, { -18.0f, -5.0f, 0.0f },
    { -12.0f, 15.0f, 0.0f },
};

/* "l_stone"'s work (its proc Etsk_l_stone_main): EneObj's layout, with its own fields. */
typedef struct LStoneW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 unused09;
    /* 0x0A */ u8 flags; /* 1: on the ground */
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ PolFrame *frame;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3]; /* its roll, its heading */
    /* 0x28 */ f32 vx;
    /* 0x2C */ f32 vy;
    /* 0x30 */ f32 vz;
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ Obj *shadow;
    /* 0x40 */ u8 pad40[0x8];
    /* 0x48 */ u32 behave; /* EneObj's */
    /* 0x4C */ PolFrame *top; /* the frame a player rides, on top of it */
    /* 0x50 */ RideFrame *ride;
    /* 0x54 */ f32 lastRoll; /* its roll when it last touched the ground */
    /* 0x58 */ f32 speed;
    /* 0x5C */ u8 near; /* a player came within 10 */
    /* 0x5D */ u8 way; /* eneNearestPlWay */
    /* 0x5E */ u8 unused5E; /* set 1 when it starts to roll, never read */
} LStoneW;

typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                    s32 a);

/* Its steps, called far. */
void lStoneWait(Obj *o);
void lStoneFall(Obj *o);
void lStoneRoll(Obj *o);

extern Gfx etskLStoneDl[]; /* the stone */
extern u8 etskLStoneRide[]; /* its ride data */

#define DPRINTF

s32 Etsk_l_stone_main(Obj *o, s16 msg, void *arg);

/* "l_stone", a rolling stone a player can ride: a model of 0.08 with an attack box against the
 * players (kind 0x7D) and a damage box, a ride frame on top and a shadow. While it is off the
 * ground it falls at 0.3 times its roll since it last touched it, over a quarter turn. It rolls
 * by its speed over its radius, facing along the course; the ride frame follows it 15 above.
 * Matched 2026-10-12; it was parked for the height's operand order. The height's scale is read
 * as a field, ((Vec3f *) scale)->x: IDO sorts a commutative operation's operands when one is an
 * array element, not a struct field. */
s32 Etsk_l_stone_main(Obj *o, s16 msg, void *arg) {
    LStoneW *w = (LStoneW *) FX(o);

    switch (msg) {
        case 2:
            o->name = "l_stone";
            eneSetup(o, arg);
            w->top = eneNewPol(o, NULL);
            if ((w->frame = eneNewPol(o, etskLStoneDl)) == NULL) {
                return -1;
            }
            w->frame->scale[2] = 0.08f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(uda_atk_root_frame, &w->frame->f, 320, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 0x7D;
            if ((w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->frame->f, 400, 800, -400, 0, 0))
                == NULL) {
                return -1;
            }
            if ((w->ride = rideCreate(gRideTree1, (s32) o, &w->top->f,
                                      (RideData *) eneFileAddr(o, etskLStoneRide, 0))) == NULL) {
                return -1;
            }
            if ((w->shadow = (Obj *) shadowCreate()) == NULL) {
                return -1;
            }
            w->behave = 0x400001;
            w->state = 0;
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->state) {
                case 0:
                    (*lStoneWait)(o);
                    break;
                case 1:
                    (*lStoneFall)(o);
                    break;
                case 2:
                    (*lStoneRoll)(o);
                    break;
            }
            if (w->flags & 1) {
                eneFall(o, 0.3f);
            } else {
                eneFall(o, ((w->rot[0] > w->lastRoll) ? w->rot[0] - w->lastRoll : -(w->rot[0] - w->lastRoll))
                               / 1.5707964f * 0.3);
            }
            w->rot[1] = eneCourseAngle(o, 4);
            w->rot[0] += -w->vx / (40.0f * (w->frame->scale[0] * 10.0f));
            if (eneAfterProc(o) == -1) {
                DPRINTF("Etsk_l_stone_main: 死亡\n");
                return -1;
            }
            if (w->flags & 1) {
                w->lastRoll = w->rot[0];
            }
            w->frame->pos[1] = w->y + 40.0f * (((Vec3f *) w->frame->scale)->x * 10.0f);
            w->top->pos[0] = w->frame->pos[0];
            w->top->pos[1] = w->frame->pos[1] + 15.0f;
            w->top->pos[2] = w->frame->pos[2];
            w->top->rot[0] = w->frame->rot[0];
            w->top->rot[1] = w->frame->rot[1];
            w->top->rot[2] = w->frame->rot[2];
            DPRINTF("C %d, x %f\n", w->state, w->x);
            break;
        case 3:
            break;
        case 1:
            /* The first destroy on one line (the game's style) */
            if (w->top != NULL) { frmDestroy(&w->top->f); }
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            if (w->atk != NULL) {
                frmDestroy((Frame *) w->atk);
            }
            if (w->dmg != NULL) {
                frmDestroy((Frame *) w->dmg);
            }
            if (w->ride != NULL) {
                frmDestroy((Frame *) w->ride);
            }
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Marks a player within 10; once no player is within 40 after that, faces the nearest one's
 * way and goes to state 1. */
void lStoneWait(Obj *o) {
    s32 pad;
    LStoneW *w = (LStoneW *) FX(o);

    if (eneNearestPlInR(o, 10.0f)) {
        ((LStoneW *) FX(o))->near = 1;
    }
    if (!enePlInRangeCheck(o, 40.0f, 0)) {
        if (w->near) {
            w->way = eneNearestPlWay(o);
            w->unused5E = 1;
            w->state = 1;
        }
    }
}

/* With flag 1: sound 0x10B at its frame, state 2. Then it falls (0.5). */
void lStoneFall(Obj *o) {
    s32 pad;
    LStoneW *w = (LStoneW *) FX(o);

    if (((LStoneW *) FX(o))->flags & 1) {
        SoundLocation2(0x10B, gGameWork.view, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                       0.0f, 0.0f, 1.5707964f);
        w->state = 2;
    }
    eneFall(o, 0.5f);
}

/* Rolls: its speed grows by 0.1 to 1.3, toward its way (4: back); a rumble every 12 frames.
 * At the end of its run on courses 1, 2, 3 and 5 it breaks (sound 0x181) into five pieces
 * (kind 0x20B) about it, raised by its size, and is done.
 * Matched 2026-10-12; it was parked for the pieces' height's operand order. As in
 * Etsk_l_stone_main, the scale is read as a field, ((Vec3f *) scale)->x. */
void lStoneRoll(Obj *o) {
    LStoneW *w = (LStoneW *) FX(o);
    s32 broken = FALSE;
    s32 i;

    if (w->speed + 0.1 < 1.3) {
        w->speed += 0.1;
    } else {
        w->speed = 1.3f;
    }
    if (w->way == 4) {
        w->vx = -w->speed;
    } else {
        w->vx = w->speed;
    }
    if (gGameFrameCount % 12 == 0) {
        eneSetSd2(o, 0x2B0, 0, 2);
    }
    if (w->vx > 0.0f) {
        if (w->zone == 1 && w->x >= 4668.0f) {
            broken = TRUE;
        }
        if (w->zone == 2 && w->x >= 997.0f) {
            broken = TRUE;
        }
        if (w->zone == 5 && w->x >= 1730.0f) {
            broken = TRUE;
        }
    }
    if (w->vx < 0.0f) {
        if (w->zone == 1 && w->x <= 3423.0f) {
            broken = TRUE;
        }
        if (w->zone == 2 && w->x <= 118.0f) {
            broken = TRUE;
        }
        if (w->zone == 3 && w->x <= 666.0f) {
            broken = TRUE;
        }
    }
    if (broken) {
        eneSetSd2(o, 0x181, 0, 0);
        for (i = 0; i < 5; i++) {
            eneMakeEx(NULL, ENE_L_STONE_BROKE, w->zone, ene20BSpawnOffsets[i][0] + w->x,
                      ene20BSpawnOffsets[i][1] + w->y + 40.0f * (((Vec3f *) w->frame->scale)->x * 10.0f),
                      ene20BSpawnOffsets[i][2] + w->z, 2, (u32 (*)(void *, void *, void *)) i);
        }
        w->behave |= 0x40000;
    }
}

/* "broke": a piece of the broken stone (its kind, 0-4, from eneSetup). */
typedef struct BrokeW {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *frame;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3]; /* x and y used here */
    /* 0x34 */ u8 pad34[0x14];
    /* 0x48 */ u32 behave; /* EneObj's */
    /* 0x4C */ u8 type;
    /* 0x4D */ u8 alpha;
} BrokeW;

extern Gfx lStoneBrokePieceDl[]; /* a piece */

#define DPRINTF

/* "broke", a piece of the broken stone: a model of 0.08, turned a fifth of a turn per kind (from
 * a tenth), facing along the course, flying off its kind's way while it spins and fades by 8 a
 * frame; at 0 it is gone (behave 0x40000). */
s32 lStoneBrokeProc(Obj *o, s16 msg, void *arg) {
    s32 pad;
    BrokeW *w = (BrokeW *) FX(o);

    switch (msg) {
        case 2:
            o->name = "broke";
            w->type = eneSetup(o, arg);
            if ((w->frame = enePolModelAlpha(o, lStoneBrokePieceDl, 0xFF)) == NULL) {
                return -1;
            }
            w->frame->scale[2] = 0.08f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->behave = 0x1800;
            w->rot[0] = w->type * 1.2566371f + 0.62831855f;
            w->rot[1] = eneCourseAngle(o, 1);
            switch (w->type) {
                case 0:
                    w->vel[0] = 0.5f;
                    w->vel[1] = 0.8f;
                    break;
                case 1:
                    w->vel[0] = 0.8f;
                    w->vel[1] = 0.5f;
                    break;
                case 2:
                    break;
                case 3:
                    w->vel[0] = -0.8f;
                    w->vel[1] = 0.5f;
                    break;
                case 4:
                    w->vel[0] = -0.5f;
                    w->vel[1] = 1.0f;
                    break;
            }
            w->alpha = 0xFF;
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            if (w->alpha - 8 > 0) {
                w->alpha -= 8;
            } else {
                w->behave |= 0x40000;
            }
            switch (w->type) {
                case 0:
                    w->rot[0] += 0.3;
                    w->rot[1] += 0.1;
                    break;
                case 1:
                    w->rot[1] += 0.3;
                    w->rot[0] += 0.1;
                    break;
                case 2:
                    w->rot[1] += 0.1;
                    break;
                case 3:
                    w->rot[1] += -0.1;
                    w->rot[0] += -0.2;
                    break;
                case 4:
                    w->rot[1] += -0.2;
                    break;
            }
            if (eneAfterProc(o) == -1) {
                DPRINTF("Etsk_konuri_main: 敵殺します\n");
                return -1;
            }
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy((Frame *) w->frame);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
