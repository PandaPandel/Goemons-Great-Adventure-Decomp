#include "tlb.h"
#include "tlb/tlb_0324.h"

extern Obj *gBattleTask; /* the battle's task (beffect.c) */

#define sinf (*sinf)
#define cosf (*cosf)

/* "TMarble"'s (btaiTMarbleProc) argument: only what its starter sets. */
typedef struct Tlb324Arg10980 {
    /* 0x00 */ u8 pad00[0x4];
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ u8 pad10[0x18];
    /* 0x28 */ f32 pos[3];
    /* 0x34 */ u8 pad34[0x28];
} Tlb324Arg10980;

/* "TMarble"'s work. */
typedef struct TMarbleW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3]; /* its throw, once it has settled */
    /* 0x10 */ f32 drift[3]; /* half its thrower's velocity */
    /* 0x1C */ f32 jitter[2]; /* x and y, dying away */
    /* 0x24 */ u8 pad24[0x8];
    /* 0x2C */ f32 y; /* its height when it settled */
    /* 0x30 */ u8 pad30[0x4];
    /* 0x34 */ f32 scale;
    /* 0x38 */ f32 dist; /* from the arena's middle */
    /* 0x3C */ f32 angle; /* of its throw */
    /* 0x40 */ ACollFrame *atk;
    /* 0x44 */ ACollFrame *dmg;
    /* 0x48 */ u8 pad48[0x4];
    /* 0x4C */ s32 alpha;
    /* 0x50 */ s32 unused50; /* set 0 in msg 2, never read */
    /* 0x54 */ f32 distSq; /* to the boss */
    /* 0x58 */ s8 kind; /* its model, 0-2 */
} TMarbleW;

typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                    s32 a);

/* tlb_0324_3's rand(-n..n) and tlb_0324_6's bubbles, called far. */
#define btaiRandSigned (*btaiRandSigned)
#define btaiTsbubbleMake (*btaiTsbubbleMake)

s32 tMarbleLastModel = 1; /* the last marble's model, 0-2 (this file's .data starts here) */
s32 btaiInkGone = 1; /* 1: the ink is gone (set by tlb_0324_6) */
extern CamView *gBattleCamView; /* the battle's camera (beffect.c) */
extern u8 btaiTMarble0Dl[]; /* the three marbles' display lists, in file 0xED */
extern u8 btaiTMarble1Dl[];
extern u8 btaiTMarble2Dl[];

s32 btaiTMarbleProc(Obj *obj, s16 msg, void *arg);

/* "TMarble", a marble the sea bream spits. Its model is the next of three in turn; its attack
 * and damage boxes take kinds 0x29 and 0x2A. It drifts with its thrower's velocity and a random
 * jitter, growing to twice its size, then flies at the Impact along its angle at a speed that
 * rises with the difficulty. Shot (kinds 1, 3, 5, 8 drop ryou) or hitting, it bursts; more than
 * 10,000 from the boss it ends. Its work pointer is set at its declaration (the game rebuilds it
 * from obj's home at each use), and speed is declared before it (IDO_NOTES 322). */
s32 btaiTMarbleProc(Obj *obj, s16 msg, void *arg) {
    f32 speed;
    TMarbleW *w = FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "TMarble";
            if ((w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol)) == NULL) {
                return -1;
            }
            if (++tMarbleLastModel >= 3) {
                tMarbleLastModel = 0;
            }
            switch (tMarbleLastModel) {
                case 0:
                    bossSetPolDL(w->frame, (Gfx *) btaiTMarble0Dl, 0xED);
                    break;
                case 1:
                    bossSetPolDL(w->frame, (Gfx *) btaiTMarble1Dl, 0xED);
                    break;
                case 2:
                    bossSetPolDL(w->frame, (Gfx *) btaiTMarble2Dl, 0xED);
                    break;
            }
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 40, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 0x29;
            if ((w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->frame->f, 50, 100, -50, 0, 0))
                == NULL) {
                return -1;
            }
            w->dmg->kind = 0x2A;
            w->frame->flags |= 0x20;
            w->frame->flags |= 7;
            w->frame->pos[0] = ((Tlb324Arg10980 *) arg)->pos[0];
            w->frame->pos[1] = ((Tlb324Arg10980 *) arg)->pos[1];
            w->frame->pos[2] = ((Tlb324Arg10980 *) arg)->pos[2];
            w->drift[0] = ((Tlb324Arg10980 *) arg)->vel[0];
            w->drift[1] = ((Tlb324Arg10980 *) arg)->vel[1];
            w->drift[2] = ((Tlb324Arg10980 *) arg)->vel[2];
            w->jitter[0] = btaiRandSigned(36);
            w->jitter[1] = btaiRandSigned(36);
            w->kind = tMarbleLastModel;
            w->angle = mathAtan2(-w->frame->pos[0], -w->frame->pos[2]);
            speed = (gBossPhase * 2 + 34) * 0.7;
            w->vel[2] = -cosf(w->angle + 3.1415927f) * speed;
            w->vel[0] = -sinf(w->angle + 3.1415927f) * speed;
            w->frame->pos[0] += w->vel[0] * 30.0f;
            w->frame->pos[2] += w->vel[2] * 30.0f;
            w->dist = sqrtf(w->frame->pos[2] * w->frame->pos[2] + w->frame->pos[2] * w->frame->pos[2]);
            w->scale = 0.5f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale;
            w->alpha = 255;
            w->unused50 = 0;
            SoundLocation2(0x230, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 500.0f,
                           13000.0f, gBattleCamView->fovy);
            break;
        case 1:
            if (w->frame != NULL) { frmDestroy(&w->frame->f); }
            if (w->atk != NULL) { frmDestroy(&w->atk->frame); }
            if (w->dmg != NULL) { frmDestroy(&w->dmg->frame); }
            break;
        case 0:
            if (btaiInkGone) {
                return -1;
            }
            if (acollDmgWasHit(w->dmg)) {
                itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 0.7f);
                if (w->dmg->hitKind == 1 || w->dmg->hitKind == 3 || w->dmg->hitKind == 5 || w->dmg->hitKind == 8) {
                    ryouCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
                }
                if (!(gBattleFlags & 0x10)) {
                    SoundLocation2(0x228, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1000.0f,
                                   13000.0f, gBattleCamView->fovy);
                }
                return -1;
            }
            if (acollAtkWasHit(w->atk) == 1) {
                itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 0.7f);
                btaiTsbubbleMake(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2]);
                SoundLocation2(0x228, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1000.0f,
                               13000.0f, gBattleCamView->fovy);
                return -1;
            }
            if ((w->distSq = bossDistSq(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], gBossPosX, gBossPosY,
                                        gBossPosZ)) > 1e8f) {
                return -1;
            }
            if (w->scale < 2.0) {
                w->scale += 0.05;
            }
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale;
            if (((w->jitter[0] > 0.0f) ? w->jitter[0] : -w->jitter[0]) > 0.1
                || ((w->jitter[1] > 0.0f) ? w->jitter[1] : -w->jitter[1]) > 0.1) {
                w->jitter[0] *= 0.85;
                w->jitter[1] *= 0.85;
                w->frame->pos[0] += w->jitter[0] + w->drift[0];
                w->frame->pos[1] += w->jitter[1] + w->drift[1];
                w->frame->pos[2] += w->drift[2];
                w->dist = sqrtf(w->frame->pos[0] * w->frame->pos[0] + w->frame->pos[2] * w->frame->pos[2]);
                w->angle = mathAtan2(btaiRandSigned(30) - w->frame->pos[0],
                                     btaiRandSigned(30) - w->frame->pos[2]);
                w->y = w->frame->pos[1];
            } else {
                w->vel[1] = (-w->y - 20.0f) / (w->dist / (gBossPhase * 2 + 34));
                w->vel[2] = -cosf(w->angle + 3.1415927f) * (gBossPhase * 2 + 34);
                w->vel[0] = -sinf(w->angle + 3.1415927f) * (gBossPhase * 2 + 34);
                w->frame->pos[0] += w->vel[0];
                w->frame->pos[1] += w->vel[1];
                w->frame->pos[2] += w->vel[2];
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The caller's work (the two procs that call this pass their frame's position): its frame, then
 * a velocity. */
typedef struct Tlb324MoverW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3];
} Tlb324MoverW;

/* A btaiTMarbleProc task under o at x, y + 680, z, with half o's velocity. */
void btaiTMarbleCreate(Obj *o, f32 x, f32 y, f32 z) {
    Tlb324Arg10980 arg;
    s32 pad[2];

    arg.pos[0] = x;
    arg.pos[1] = y - -600.0f + 80.0f;
    arg.pos[2] = z;
    arg.vel[0] = ((Tlb324MoverW *) FX(o))->vel[0] / 2;
    arg.vel[1] = ((Tlb324MoverW *) FX(o))->vel[1] / 2;
    arg.vel[2] = ((Tlb324MoverW *) FX(o))->vel[2] / 2;
    tskCreate(o, btaiTMarbleProc, gBattleTask->file, &arg);
}

/* "Ddmybomb"'s (btaiDdmybombProc) argument: only what its starter sets. */
typedef struct DdmybombArg {
    /* 0x00 */ u8 pad00[0x1C];
    /* 0x1C */ f32 pos[3];
    /* 0x28 */ f32 radius; /* its radius */
    /* 0x2C */ u8 pad2C[0xC];
    /* 0x38 */ s32 timer; /* its frames */
    /* 0x3C */ s32 unused3C; /* the starter's 7th argument (10 at every call), only copied to w->unused3C */
    /* 0x40 */ s32 kind; /* its attack kind */
} DdmybombArg;

/* "Ddmybomb"'s work. */
typedef struct DdmybombW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3]; /* x and z */
    /* 0x10 */ u8 pad10[0x18];
    /* 0x28 */ f32 radius;
    /* 0x2C */ f32 angle;
    /* 0x30 */ ACollFrame *atk;
    /* 0x34 */ u8 pad34[0x4];
    /* 0x38 */ s32 timer;
    /* 0x3C */ s32 unused3C; /* set from the argument's 0x3C (10) in msg 2, never read */
    /* 0x40 */ s32 kind;
} DdmybombW;


extern s32 btaiHitStun;
extern s32 btaiInkGone; /* 1: the ink is gone (tlb_0324_6) */
extern CamView *gBattleCamView; /* the battle's camera (beffect.c) */

#define DPRINTF

/* "Ddmybomb", an invisible bomb: an attack box of its radius at its place (on the ground),
 * flying at 200 a frame straight away from the middle. When it hits, a puff and its kind's
 * sound (0x27E, 0x27F or 0x228, panned) and it is gone; otherwise it is gone after its frames,
 * or when the battle stops. */
s32 btaiDdmybombProc(Obj *obj, s16 msg, void *arg) {
    DdmybombW *w = (DdmybombW *) FX(obj);
    s16 pan;
    s32 pad[3];

    switch (msg) {
        case 2:
            w->kind = ((DdmybombArg *) arg)->kind;
            obj->name = "Ddmybomb";
            if ((w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol)) == NULL) {
                return -1;
            }
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f,
                                                         ((DdmybombArg *) arg)->radius, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = w->kind;
            w->frame->pos[0] = ((DdmybombArg *) arg)->pos[0];
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = ((DdmybombArg *) arg)->pos[2];
            w->angle = mathAtan2(-w->frame->pos[0], -w->frame->pos[2]);
            w->vel[2] = -cosf(w->angle + 3.1415927f) * 200.0f;
            w->vel[0] = -sinf(w->angle + 3.1415927f) * 200.0f;
            w->frame->scale[2] = 2.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->timer = ((DdmybombArg *) arg)->timer + 1;
            w->unused3C = ((DdmybombArg *) arg)->unused3C;
            w->radius = ((DdmybombArg *) arg)->radius;
            DPRINTF("とうめい爆弾 %d %d %d\n", w->timer, w->unused3C, w->kind);
            break;
        case 1:
            DPRINTF("とうめい爆弾消滅\n");
            frmDestroy(&w->frame->f);
            frmDestroy((Frame *) w->atk);
            break;
        case 0:
            if (btaiInkGone != 0 || btaiHitStun != 0) {
                return -1;
            }
            w->frame->pos[0] += w->vel[0];
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] += w->vel[2];
            if (acollAtkWasHit(w->atk) == 1) {
                DPRINTF("あたり！\n");
                itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 3.0f);
                pan = soundPanGet(gBattleCamView, w->frame->pos[0], w->frame->pos[2], gBattleCamView->fovy);
                switch (w->kind) {
                    case 0x34:
                        sndRequest(0x27E, pan, 0);
                        break;
                    case 0x32:
                        sndRequest(0x27F, pan, 0);
                        break;
                    case 0x2D:
                        sndRequest(0x27F, pan, 0);
                        break;
                    case 0x35:
                        sndRequest(0x228, pan, 0);
                        break;
                }
                return -1;
            }
            if (--w->timer < 0) {
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "Ddmybomb" at x, y, z under obj, in the battle task's file. */
void btaiDdmybombCreate(Obj *obj, f32 x, f32 y, f32 z, f32 v, s32 a, s32 b, s32 c) {
    DdmybombArg arg;
    s32 pad[3];

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.radius = v;
    arg.timer = a;
    arg.unused3C = b;
    arg.kind = c;
    tskCreate(obj, btaiDdmybombProc, gBattleTask->file, &arg);
}

/* "IKA_BOMB"'s (btaiIkaBombProc, a squid torpedo) argument: only what its starter sets. */
typedef struct IkaBombArg {
    /* 0x00 */ u8 pad00[0x10];
    /* 0x10 */ f32 vel[2];
    /* 0x18 */ u8 pad18[0x4];
    /* 0x1C */ f32 pos[3];
    /* 0x28 */ u8 pad28[0x28];
} IkaBombArg;

/* "IKA_BOMB"'s work. */
typedef struct IkaBombW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 vel[3]; /* its dive */
    /* 0x10 */ f32 drift[3]; /* its launch, dying away */
    /* 0x1C */ u8 pad1C[0x4];
    /* 0x20 */ f32 y; /* its height when it turned */
    /* 0x24 */ u8 pad24[0x4];
    /* 0x28 */ f32 scale;
    /* 0x2C */ f32 dist; /* from the arena's middle */
    /* 0x30 */ f32 angle; /* of its dive */
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ u8 pad3C[0x4];
    /* 0x40 */ s32 unused40; /* set 0 in msg 2, never read */
    /* 0x44 */ u8 state;
    /* 0x48 */ s32 timer;
    /* 0x4C */ u8 hp; /* propeller hits left */
    /* 0x4D */ u8 flash; /* red flash frames left */
} IkaBombW;

/* tlb_0324_4's and tlb_0324_6's, called far. */
#define btaiTurnTo (*btaiTurnTo)
#define btaiIkasumiMake (*btaiIkasumiMake)

extern Gfx gBossLitTexEnvDl[];
extern u8 btaiIkaBombAnim[]; /* the torpedo's animations, in file 0xF9 */
extern u8 btaiIkaBombState1Anim[];

s32 btaiIkaBombProc(Obj *obj, s16 msg, void *arg);

/* "IKA_BOMB", a squid torpedo the sea bream launches. It flies up and out, slowing, then dives
 * at the Impact (state 1), skims the water tilting up (2) and leaps with a splash (3); its
 * animation's end ends it (4). Shot with kinds 1, 3, 5 or 8 it bursts and drops ryou; the
 * propeller (kinds 15, 16) takes four hits, each flashing it red. As TMarble: w at its
 * declaration, the scale chain from w->scale, the distance before the scale; its zeros are 0.0f
 * (IDO_NOTES 326) and the splash toggle is its static. */
s32 btaiIkaBombProc(Obj *obj, s16 msg, void *arg) {
    static s32 splash = 0; /* which of the two splash sounds is next */
    IkaBombW *w = FX(obj);
    s32 pad[3];

    switch (msg) {
        case 2:
            obj->name = "IKA_BOMB";
            if ((w->frame = (AnimeFrame *) frmCreate(gFrmLayer2, gFrmKindAnime)) == NULL) {
                return -1;
            }
            bossSetAnimAt(w->frame, btaiIkaBombAnim, 0xF9, 0.0f);
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 20, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 0x29;
            if ((w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->frame->f, 70, 100, -30, 0, 0))
                == NULL) {
                return -1;
            }
            w->dmg->kind = 0x2A;
            w->frame->pos[0] = ((IkaBombArg *) arg)->pos[0];
            w->frame->pos[1] = ((IkaBombArg *) arg)->pos[1];
            w->frame->pos[2] = ((IkaBombArg *) arg)->pos[2];
            w->drift[0] = ((IkaBombArg *) arg)->vel[0];
            w->drift[1] = ((IkaBombArg *) arg)->vel[1];
            w->drift[2] = 50.0f;
            w->angle = mathAtan2(-w->frame->pos[0], -w->frame->pos[2]);
            w->vel[2] = -cosf(w->angle + 3.1415927f) * 30.0f;
            w->vel[0] = -sinf(w->angle + 3.1415927f) * 30.0f;
            w->frame->pos[0] += w->vel[0] * 20.0f;
            w->frame->pos[2] += w->vel[2] * 20.0f;
            w->frame->rot[0] = 1.5707964f;
            w->dist = sqrtf(w->frame->pos[2] * w->frame->pos[2] + w->frame->pos[2] * w->frame->pos[2]);
            w->scale = 0.3f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale;
            w->unused40 = 0;
            w->state = 0;
            w->timer = 0;
            w->hp = 4;
            w->flash = 0;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->atk->frame);
            frmDestroy(&w->dmg->frame);
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, gBossLitTexEnvDl);
            if (w->flash) {
                gDPSetEnvColor(gGfxDlHead++, w->flash * 50, 0, 0, 255);
                w->flash--;
            } else {
                gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 0);
            }
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            if (btaiInkGone) {
                return -1;
            }
            if (acollDmgWasHit(w->dmg)) {
                if (w->dmg->hitKind == 1 || w->dmg->hitKind == 5 || w->dmg->hitKind == 8 || w->dmg->hitKind == 3) {
                    itexPuffGrey(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 3.0f);
                    btaiTsbubbleMake(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2]);
                    ryouCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
                    if (!(gBattleFlags & 0x10)) {
                        SoundLocation2(0x290, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                       0.0f, 15000.0f, gBattleCamView->fovy);
                    }
                    return -1;
                }
                if (w->dmg->hitKind == 15 || w->dmg->hitKind == 16) {
                    if (--w->hp <= 0) {
                        itexPuffGrey(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 3.0f);
                        btaiTsbubbleMake(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2]);
                        SoundLocation2(0x290, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                       0.0f, 15000.0f, gBattleCamView->fovy);
                        return -1;
                    }
                    DPRINTF("イカス魚雷:のこりHP ->%d\n", w->hp);
                    w->flash = 5;
                }
            }
            if (acollAtkWasHit(w->atk) == 1) {
                itexPuffGrey(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 3.0f);
                return -1;
            }
            if (bossDistSq(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], gBossPosX, gBossPosY, gBossPosZ)
                > 1e8f) {
                return -1;
            }
            if (w->scale < 1.0f) {
                w->scale += 0.04;
            }
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = w->scale;
            switch (w->state) {
                case 0:
                    if (sqrtf(w->drift[1] * w->drift[1] + w->drift[0] * w->drift[0]) < 0.4) {
                        w->state = 1;
                        w->vel[1] = (-w->y - 25.0f) / ((w->dist - 300.0f) / 30.0f);
                        w->vel[2] = -cosf(w->angle + 3.1415927f) * 30.0f;
                        w->vel[0] = -sinf(w->angle + 3.1415927f) * 30.0f;
                    }
                    w->drift[0] *= 0.95;
                    w->drift[1] *= 0.95;
                    w->drift[2] *= 0.8;
                    w->frame->pos[0] += w->drift[0];
                    w->frame->pos[1] += w->drift[1];
                    w->frame->pos[2] += w->drift[2];
                    w->dist = sqrtf(w->frame->pos[0] * w->frame->pos[0] + w->frame->pos[2] * w->frame->pos[2]);
                    w->angle = mathAtan2(btaiRandSigned(30) - w->frame->pos[0],
                                         btaiRandSigned(30) - w->frame->pos[2]);
                    w->y = w->frame->pos[1];
                    w->frame->rot[0] = 1.5707963705062866 - w->drift[1] * 0.0174532930038800785 * 6.0;
                    w->frame->rot[2] = -w->drift[0] * 0.0174532930038800785 * 6.0;
                    bossStepAnim(w->frame, 0.1f, 0);
                    break;
                case 1:
                    if (w->frame->pos[2] > -900.0f) {
                        w->state = 2;
                        bossSetAnimAt(w->frame, btaiIkaBombState1Anim, 0xF9, 0.0f);
                    }
                    w->frame->pos[0] += w->vel[0];
                    w->frame->pos[1] += w->vel[1];
                    w->frame->pos[2] += w->vel[2];
                    bossStepAnim(w->frame, 0.1f, 0);
                    break;
                case 2:
                    w->vel[0] *= 0.95;
                    w->vel[1] *= 0.95;
                    w->vel[2] *= 0.94;
                    if (w->frame->rot[0] > -0.3490658602346812) {
                        w->frame->rot[0] -= 0.06981317201552031;
                    } else {
                        w->state = 3;
                    }
                    btaiTurnTo(obj, 0.0f, 0.0f, 2.0f);
                    w->frame->pos[0] += w->vel[0];
                    w->frame->pos[1] += w->vel[1];
                    w->frame->pos[2] += w->vel[2];
                    break;
                case 3:
                    if (bossStepAnim(w->frame, 0.6f, 2) == 1) {
                        w->state = 4;
                    }
                    if (w->timer++ >= 7) {
                        if (w->timer == 8) {
                            btaiIkasumiMake(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 0.1f, 250);
                            if (splash == 0) {
                                sndPlay(0x2B2);
                                splash = 1;
                            } else {
                                sndPlay(0x2B3);
                                splash = 0;
                            }
                            w->vel[1] = 20.0f;
                            w->vel[2] = -10.0f;
                            w->vel[0] = w->frame->pos[0] * 0.05;
                        }
                        w->vel[1] += 1.0f;
                        w->vel[2] -= 1.0f;
                        w->vel[0] *= 1.06;
                        w->frame->rot[0] += 0.052359879011640235;
                    }
                    w->frame->pos[0] += w->vel[0];
                    w->frame->pos[1] += w->vel[1];
                    w->frame->pos[2] += w->vel[2];
                    break;
                case 4:
                    return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* An "IKA_BOMB" under obj at x, y + 700, z, launched at speed 24 at an angle (in degrees,
 * plus one), in the battle task's file. */
void btaiIkaBombCreate(Obj *obj, f32 x, f32 y, f32 z, f32 angle) {
    IkaBombArg arg;
    s32 pad[2];

    arg.pos[2] = z;
    arg.pos[0] = x;
    arg.pos[1] = y - -600.0f + 100.0f;
    arg.vel[1] = sinf((angle + 1.0f) * 0.0174532930038800785) * 24.0f;
    arg.vel[0] = cosf((angle + 1.0f) * 0.0174532930038800785) * 24.0f;
    tskCreate(obj, btaiIkaBombProc, gBattleTask->file, &arg);
}
